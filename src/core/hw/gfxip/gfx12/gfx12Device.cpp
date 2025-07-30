/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All Rights Reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 **********************************************************************************************************************/

#include "core/device.h"
#include "core/engine.h"
#include "core/hw/amdgpu_asic.h"
#include "core/hw/gfxip/gfx12/g_gfx12DataFormats.h"
#include "core/hw/gfxip/gfxCmdBuffer.h"
#include "core/hw/gfxip/archivePipeline.h"
#include "core/hw/gfxip/gfx12/gfx12BorderColorPalette.h"
#include "core/hw/gfxip/gfx12/gfx12Chip.h"
#include "core/hw/gfxip/gfx12/gfx12CmdStream.h"
#include "core/hw/gfxip/gfx12/gfx12CmdUploadRing.h"
#include "core/hw/gfxip/gfx12/gfx12ColorBlendState.h"
#include "core/hw/gfxip/gfx12/gfx12ColorTargetView.h"
#include "core/hw/gfxip/gfx12/gfx12ComputePipeline.h"
#include "core/hw/gfxip/gfx12/gfx12ComputeShaderLibrary.h"
#include "core/hw/gfxip/gfx12/gfx12DepthStencilState.h"
#include "core/hw/gfxip/gfx12/gfx12DepthStencilView.h"
#include "core/hw/gfxip/gfx12/gfx12Device.h"
#include "core/hw/gfxip/gfx12/gfx12FormatInfo.h"
#include "core/hw/gfxip/gfx12/gfx12GraphicsPipeline.h"
#include "core/hw/gfxip/gfx12/gfx12GraphicsShaderLibrary.h"
#include "core/hw/gfxip/gfx12/gfx12HybridGraphicsPipeline.h"
#include "core/hw/gfxip/gfx12/gfx12Image.h"
#include "core/hw/gfxip/gfx12/gfx12IndirectCmdGenerator.h"
#include "core/hw/gfxip/gfx12/gfx12MsaaState.h"
#include "core/hw/gfxip/gfx12/gfx12PerfCtrInfo.h"
#include "core/hw/gfxip/gfx12/gfx12PerfExperiment.h"
#include "core/hw/gfxip/gfx12/gfx12PipelineStatsQueryPool.h"
#include "core/hw/gfxip/gfx12/gfx12QueueContexts.h"
#include "core/hw/gfxip/gfx12/gfx12QueueRingBuffer.h"
#include "core/hw/gfxip/gfx12/gfx12UniversalCmdBuffer.h"
#include "core/hw/gfxip/gfx12/gfx12StreamoutStatsQueryPool.h"
#include "core/hw/gfxip/rpm/rpmUtil.h"
#include "core/hw/gfxip/sdma/gfx12/gfx12DmaCmdBuffer.h"
#include "palLiterals.h"
#include "palPipelineAbiReader.h"

using namespace Util;
using namespace Util::Literals;
using namespace Pal::Formats::Gfx12;

namespace Pal
{
namespace Gfx12
{

static_assert((uint32(Util::Abi::ZOrder::LateZ)           == LATE_Z)              &&
              (uint32(Util::Abi::ZOrder::EarlyZThenLateZ) == EARLY_Z_THEN_LATE_Z) &&
              (uint32(Util::Abi::ZOrder::ReZ)             == RE_Z)                &&
              (uint32(Util::Abi::ZOrder::EarlyZThenReZ)   == EARLY_Z_THEN_RE_Z),
              "ABI and HW enum values do not match!");

// =====================================================================================================================
// Determines the GFXIP level of a GPU supported by the Gfx12 hardware layer. The return value will be 0.0.0
// if the GPU is unsupported by this HWL.
// Only the major and minor ip versions are reported here. The stepping value will be updated later along with
// the other gpu-specific properties
IpTriple DetermineIpLevel(
    uint32 familyId, // Hardware Family ID.
    uint32 eRevId)   // Software Revision ID.
{
    IpTriple level = {};

    switch (familyId)
    {
    case FAMILY_NV4:
        level = { .major = 12, .minor = 0, .stepping = 0 };
        break;
    default:
        PAL_ASSERT_ALWAYS();
        break;
    }

    return level;
}

// =====================================================================================================================
// Gets the static format support info table for GFXIP 12 hardware.
const MergedFormatPropertiesTable* GetFormatPropertiesTable(
    GfxIpLevel gfxIpLevel)
{
    const MergedFormatPropertiesTable* pTable = nullptr;

    switch (gfxIpLevel)
    {
    case GfxIpLevel::GfxIp12:
        pTable = &Gfx12MergedFormatPropertiesTable;
        break;
    default:
        // What is this? Add future GFX12 steppings above.
        PAL_ASSERT_ALWAYS();
    }

    return pTable;
}

// =====================================================================================================================
size_t GetDeviceSize()
{
    return sizeof(Device);
}

// =====================================================================================================================
Result CreateDevice(
    Pal::Device*             pDevice,
    void*                    pPlacementAddr,
    DeviceInterfacePfnTable* pPfnTable,
    GfxDevice**              ppGfxDevice)
{
    PAL_ASSERT((pDevice != nullptr) && (pPlacementAddr != nullptr) && (ppGfxDevice != nullptr));

    Device* pGfxDevice = PAL_PLACEMENT_NEW(pPlacementAddr) Device(pDevice);

    Result result = pGfxDevice->EarlyInit();

    if (result == Result::Success)
    {
        (*ppGfxDevice) = pGfxDevice;

        pPfnTable->pfnCreateTypedBufViewSrds   = &Device::CreateTypedBufferViewSrds;
        pPfnTable->pfnCreateUntypedBufViewSrds = &Device::CreateUntypedBufferViewSrds;
        pPfnTable->pfnCreateImageViewSrds      = &Device::CreateImageViewSrds;
        pPfnTable->pfnCreateSamplerSrds        = &Device::CreateSamplerSrds;
        pPfnTable->pfnCreateBvhSrds            = &Device::CreateBvhSrds;
        pPfnTable->pfnDecodeBufferViewSrd      = &Device::DecodeBufferViewSrd;
        pPfnTable->pfnDecodeImageViewSrd       = &Device::DecodeImageViewSrd;
    }

    return result;
}

// =====================================================================================================================
// Creates a Gfx12 specific settings loader object
DevDriver::SettingsBase* CreateSettingsLoader(
    Pal::Device* pDevice)
{
    return PAL_NEW(SettingsLoader, pDevice->GetPlatform(), AllocInternal)(pDevice);
}

// =====================================================================================================================
// Initializes the GPU chip properties for a Device object, specifically for the Gfx12 hardware layer. Returns an error
// if an unsupported chip revision is detected.
void InitializeGpuChipProperties(
    const Platform*    pPlatform,
    GpuChipProperties* pInfo)
{
    pInfo->imageProperties.flags.u32All = 0;

    // Gfx12 core ASICs support all MSAA modes (up to S16F8)
    pInfo->imageProperties.msaaSupport      = MsaaAll;
    pInfo->imageProperties.maxMsaaFragments = 8;

    pInfo->imageProperties.tilingSupported[static_cast<uint32>(ImageTiling::Linear)]       = true;
    pInfo->imageProperties.tilingSupported[static_cast<uint32>(ImageTiling::Optimal)]      = true;

    // Standard swizzle modes are not supported for Gfx12
    pInfo->imageProperties.tilingSupported[static_cast<uint32>(ImageTiling::Standard64Kb)] = false;

    pInfo->imageProperties.flags.supportsAqbsStereoMode = 1;

    pInfo->gfxip.supportCaptureReplay    = 1;
    pInfo->gfxip.supportsHwVs            = 0;

    pInfo->gfxip.maxUserDataEntries      = MaxUserDataEntries;

    pInfo->gfxip.supportsVrs = 1;

    pInfo->gfxip.supportHsaAbi = 1;

    pInfo->gfxip.gfx9DataValid = 1;

    pInfo->imageProperties.maxImageDimension.width  = MaxImageWidth;
    pInfo->imageProperties.maxImageDimension.height = MaxImageHeight;
    pInfo->imageProperties.maxImageDimension.depth  = MaxImageDepth;
    pInfo->imageProperties.maxImageMipLevels        = MaxImageMipLevels;

    // The maximum amount of LDS space that can be shared by a group of threads (wave/ threadgroup) in bytes.
    pInfo->gfxip.ldsSizePerThreadGroup = 64_KiB;
    pInfo->gfxip.ldsSizePerCu          = 64_KiB;
    pInfo->gfxip.ldsGranularity        = LdsDwGranularity * sizeof(uint32);
    pInfo->gfxip.tcpSizeInBytes        = 16_KiB;
    pInfo->gfxip.maxLateAllocVsLimit   = 64;

    pInfo->gfxip.gl1cSizePerSa         = 0_KiB;  // Navi4x do not have GL1 cache.
    pInfo->gfxip.instCacheSizePerCu    = 32_KiB; // INST_CACHE_BANK_SIZE_KBYTES * INST_NUM_BANKS
    pInfo->gfxip.scalarCacheSizePerCu  = 16_KiB; // DATA_CACHE_BANK_SIZE_KBYTES * DATA_NUM_BANKS

    pInfo->gfxip.supportGl2Uncached      = 1;
    pInfo->gfxip.gl2UncachedCpuCoherency = (CoherCpu | CoherShader | CoherIndirectArgs | CoherIndexData |
                                            CoherQueueAtomic | CoherTimestamp | CoherStreamOut | CoherMemory |
                                            CoherCp | CoherSampleRate);

    pInfo->gfxip.maxGsOutputVert            = 256;
    pInfo->gfxip.maxGsTotalOutputComponents = 1024;
    pInfo->gfxip.maxGsInvocations           = 32;

    // Max supported by HW is 2^32-1 for all counters.  However limit Y and Z to keep total threads < 2^64 to avoid
    // potentially overflowing 64 bit counters in HW
    pInfo->gfxip.maxComputeThreadGroupCountX = UINT32_MAX;
    pInfo->gfxip.maxComputeThreadGroupCountY = UINT16_MAX;
    pInfo->gfxip.maxComputeThreadGroupCountZ = UINT16_MAX;

    pInfo->imageProperties.prtFeatures = PrtFeatures;
    pInfo->imageProperties.prtTileSize = PrtTileSize;

    pInfo->imageProperties.vrsTileSize.width  = 8;
    pInfo->imageProperties.vrsTileSize.height = 8;

    // When per-channel min/max filter operations are supported, make it clear that single channel always are as well.
    pInfo->gfx9.supportSingleChannelMinMaxFilter = 1;

    pInfo->gfx9.supports2BitSignedValues           = 1;
    pInfo->gfx9.supportConservativeRasterization   = 1;
    pInfo->gfx9.supportPrtBlendZeroMode            = 1;
    pInfo->gfx9.supportPrimitiveOrderedPs          = 1;
    pInfo->gfx9.supportImplicitPrimitiveShader     = 1;
    pInfo->gfx9.supportFp16Fetch                   = 1;
    pInfo->gfx9.support16BitInstructions           = 1;
    pInfo->gfx9.support64BitInstructions           = 1;
    pInfo->gfx9.supportBorderColorSwizzle          = 1;
    pInfo->gfx9.supportDoubleRate16BitInstructions = 1;
    pInfo->gfx9.supportImageViewMinLod             = 1;

    // Gfx12 has removed the register CB_COVERAGE_OUT_CONTROL.
    pInfo->gfx9.supportMsaaCoverageOut             = 0;

    // Support PrimitiveTopology::TwoDRectList for GfxIp9 and onwards.
    pInfo->gfx9.support2DRectList                  = 1;

    // All gfx9+ hardware can support subgroup/device clocks
    pInfo->gfx9.supportShaderSubgroupClock = 1;
    pInfo->gfx9.supportShaderDeviceClock   = 1;

    // Gfx12 doesn't need support for these PM4 packets.
    pInfo->gfx9.supportAddrOffsetDumpAndSetShPkt = 0;
    pInfo->gfx9.supportAddrOffsetSetSh256Pkt     = 0;

    pInfo->gfx9.supportPostDepthCoverage         = 1;

    //       11.264 : FP64 atomic operations are removed from GL2 in Gfx11, though atomic exch op is enabled
    pInfo->gfxip.supportFloat32BufferAtomics     = 1;
    pInfo->gfxip.supportFloat32ImageAtomics      = 1;
    pInfo->gfxip.supportFloat32ImageAtomicMinMax = 1;
    pInfo->gfxip.supportFloat32ImageAtomicAdd    = 1;
    pInfo->gfx9.supportFloat64Atomics            = 1;

    pInfo->gfx9.supportPatchTessDistribution     = 1;
    pInfo->gfx9.supportDonutTessDistribution     = 1;
    pInfo->gfx9.supportTrapezoidTessDistribution = 1;

    pInfo->gfx9.gfx10.supportedVrsRates = ((1 << static_cast<uint32>(Pal::VrsShadingRate::_16xSsaa)) |
                                           (1 << static_cast<uint32>(Pal::VrsShadingRate::_8xSsaa))  |
                                           (1 << static_cast<uint32>(Pal::VrsShadingRate::_4xSsaa))  |
                                           (1 << static_cast<uint32>(Pal::VrsShadingRate::_2xSsaa))  |
                                           (1 << static_cast<uint32>(Pal::VrsShadingRate::_1x1))     |
                                           (1 << static_cast<uint32>(Pal::VrsShadingRate::_1x2))     |
                                           (1 << static_cast<uint32>(Pal::VrsShadingRate::_2x1))     |
                                           (1 << static_cast<uint32>(Pal::VrsShadingRate::_2x2)));

    pInfo->gfx9.numShaderArrays         = 2;
    pInfo->gfx9.numSimdPerCu            = 2;
    pInfo->gfx9.numWavesPerSimd         = 16;
    pInfo->gfx9.nativeWavefrontSize     = 32;
    pInfo->gfx9.minWavefrontSize        = 32;
    pInfo->gfx9.maxWavefrontSize        = 64;
    pInfo->gfx9.numShaderVisibleSgprs   = MaxSgprsAvailable;
    pInfo->gfx9.numPhysicalSgprs        = pInfo->gfx9.numWavesPerSimd * 128;
    pInfo->gfx9.sgprAllocGranularity    = 128;
    pInfo->gfx9.minSgprAlloc            = pInfo->gfx9.sgprAllocGranularity;

    pInfo->gfx9.numPhysicalVgprs      = 1536;
    pInfo->gfx9.vgprAllocGranularity  = 24;
    pInfo->gfx9.minVgprAlloc          = pInfo->gfx9.vgprAllocGranularity;
    pInfo->gfxip.shaderPrefetchBytes  = 3 * ShaderICacheLineSize;
    pInfo->gfxip.supportsSwStrmout    = 1;
    pInfo->gfxip.supportsHwVs         = 0;

    pInfo->gfxip.support1dDispatchInterleave = 1;
    pInfo->gfxip.support2dDispatchInterleave = 1;

    pInfo->gfx9.gsVgtTableDepth         = 32;
    pInfo->gfx9.gsPrimBufferDepth       = 1792;
    pInfo->gfx9.doubleOffchipLdsBuffers = 1;

    pInfo->gfxip.vaRangeNumBits   = 48;
    pInfo->gfxip.hardwareContexts = 8;

    pInfo->gfx9.numScPerSe     = 1;
    pInfo->gfx9.numPackerPerSc = 4;

    pInfo->gfxip.soCtrlBufSize = SoCtrlBufSize;

    pInfo->srdSizes.typedBufferView   = sizeof(sq_buf_rsrc_t);
    pInfo->srdSizes.untypedBufferView = sizeof(sq_buf_rsrc_t);
    pInfo->srdSizes.imageView         = sizeof(sq_img_rsrc_t);
    pInfo->srdSizes.fmaskView         = 0;
    pInfo->srdSizes.sampler           = sizeof(sq_img_samp_t);
    pInfo->srdSizes.bvh               = sizeof(sq_bvh_rsrc_t);

    pInfo->nullSrds.pNullBufferView = &NullBufferView;
    pInfo->nullSrds.pNullImageView  = &NullImageView;
    pInfo->nullSrds.pNullFmaskView  = nullptr;
    pInfo->nullSrds.pNullSampler    = &NullSampler;

    // Setup anything specific to a given GFXIP level here

    // BVH used for ray-tracing is supported though.
    pInfo->gfx9.supportIntersectRayBarycentrics = 1;

    // For PS raw vertex attributes, unrelated to ray-tracing
    pInfo->gfx9.supportSortAgnosticBarycentrics = 1;

    pInfo->imageProperties.maxImageArraySize       = MaxImageArraySlices;
    pInfo->imageProperties.flags.supportDisplayDcc = 1;

    pInfo->gfx9.supportPerShaderStageWaveSize = 1;
    pInfo->gfx9.supportCustomWaveBreakSize    = 1;
    pInfo->gfx9.support1xMsaaSampleLocations  = 1;
    pInfo->gfx9.supportSpiPrefPriority        = 1;

    pInfo->gfx9.supportRayTraversalStack      = 1;
    pInfo->gfx9.supportPointerFlags           = 1;
    pInfo->gfx9.supportCooperativeMatrix      = 1;

    pInfo->gfx9.rayTracingIp = RayTracingIpLevel::RtIp3_1;

    pInfo->gfx9.supportFp16Dot2                = 1;
    pInfo->gfx9.supportInt8Dot                 = 1;
    pInfo->gfx9.supportInt4Dot                 = 1;
    pInfo->gfx9.supportMixedSignIntDot         = 1;
    pInfo->gfx9.supportSpp                     = 1;
    pInfo->gfx9.supportBFloat16                = 1;
    pInfo->gfx9.supportFloat8                  = 1;
    pInfo->gfx9.supportInt4                    = 1;
    pInfo->gfx9.supportCooperativeMatrix2      = 1;

    // GFX12-specific image properties go here
    pInfo->imageProperties.flags.supportsCornerSampling = 1;

    //  Gfx12 products don't support EQAA
    pInfo->imageProperties.msaaSupport = static_cast<MsaaFlags>(MsaaS1F1 | MsaaS2F2 | MsaaS4F4 | MsaaS8F8);

    // Per-chip properties:
#if PAL_BUILD_NAVI48
    if (AMDGPU_IS_NAVI48(pInfo->familyId, pInfo->eRevId))
    {
        uint32 stepping = Abi::GfxIpSteppingNavi48;

        pInfo->gfx9.rbPlus = 1;

        pInfo->gpuType                             = GpuType::Discrete;
        pInfo->revision                            = AsicRevision::Navi48;
        pInfo->gfxStepping                         = stepping;
        pInfo->gfxTriple.stepping                  = stepping;
        pInfo->gfx9.numShaderEngines               = 4;
        pInfo->gfx9.numSdpInterfaces               = 36;
        pInfo->gfx9.maxNumCuPerSh                  = 8;
        pInfo->gfx9.maxNumRbPerSe                  = 4;

        // The GL2C is the TCC.
        pInfo->gfx9.gfx10.numGl2a                  = 4;
        pInfo->gfx9.gfx10.numGl2c                  = 32;
        pInfo->gfx9.numTccBlocks                   = pInfo->gfx9.gfx10.numGl2c;

        pInfo->gfx9.gfx10.numWgpAboveSpi = 4; // GPU__GC__NUM_WGP0_PER_SA
        pInfo->gfx9.gfx10.numWgpBelowSpi = 0; // GPU__GC__NUM_WGP1_PER_SA

        pInfo->gfxip.mallSizeInBytes = 64_MiB;

        pInfo->gfxip.tccSizeInBytes = 8_MiB; // gl2c_total_cache_size_KB
    } 
    else if (AMDGPU_IS_NAVI44(pInfo->familyId, pInfo->eRevId))
    {
        uint32 stepping = Abi::GfxIpSteppingNavi44;

        pInfo->gfx9.rbPlus = 1;

        pInfo->gpuType                             = GpuType::Discrete;
        pInfo->revision                            = AsicRevision::Navi44;
        pInfo->gfxStepping                         = stepping;
        pInfo->gfxTriple.stepping                  = stepping;
        pInfo->gfx9.numShaderEngines               = 2;    // RX 9060 XT has 2 SEs
        pInfo->gfx9.numSdpInterfaces               = 32;   // Adjust for Navi44
        pInfo->gfx9.maxNumCuPerSh                  = 16;   // 32 CUs total / 2 SEs = 16 per SE
        pInfo->gfx9.maxNumRbPerSe                  = 4;    // Same as Navi48

        // The GL2C is the TCC - adjust cache config
        pInfo->gfx9.gfx10.numGl2a                  = 2;    // Fewer for mid-range
        pInfo->gfx9.gfx10.numGl2c                  = 16;   // Half of Navi48
        pInfo->gfx9.numTccBlocks                   = pInfo->gfx9.gfx10.numGl2c;

        pInfo->gfx9.gfx10.numWgpAboveSpi = 2;  // Adjust for smaller GPU
        pInfo->gfx9.gfx10.numWgpBelowSpi = 0;

        pInfo->gfxip.mallSizeInBytes = 32_MiB;  // Smaller cache
        pInfo->gfxip.tccSizeInBytes = 4_MiB;  // Half the cache size
    }
    else
#endif
    {
        PAL_ASSERT_ALWAYS_MSG("Unknown NV4 Revision %d", pInfo->eRevId);
    }

    pInfo->gfx9.numActiveShaderEngines = pInfo->gfx9.numShaderEngines;

    pInfo->gfxip.wgs.supported                = true;
    pInfo->gfxip.wgs.metadataAddrAlignment    = sizeof(uint64);
    pInfo->gfxip.wgs.instrCacheAddrAlignment  = 4_KiB;
    pInfo->gfxip.wgs.dataCacheAddrAlignment   = 64_KiB;

    // Nothing else should be set after this point.
}

// =====================================================================================================================
// Finalizes the GPU chip properties for a Device object, specifically for the Gfx12 hardware layer. Intended to be
// called after InitializeGpuChipProperties().
void FinalizeGpuChipProperties(
    const Pal::Device& device,
    GpuChipProperties* pInfo)
{

    // Setup some GPU properties which can be derived from other properties:

    // Total number of physical CU's (before harvesting)
    pInfo->gfx9.numPhysicalCus = (pInfo->gfx9.numShaderEngines *
                                  pInfo->gfx9.numShaderArrays  *
                                  pInfo->gfx9.maxNumCuPerSh);

    // GPU__GC__NUM_SE * GPU__GC__NUM_RB_PER_SE
    pInfo->gfx9.numTotalRbs = (pInfo->gfx9.numShaderEngines * pInfo->gfx9.maxNumRbPerSe);

    // Active RB counts will be overridden if any RBs are disabled.
    pInfo->gfx9.numActiveRbs     = pInfo->gfx9.numTotalRbs;
    pInfo->gfx9.activeNumRbPerSe = pInfo->gfx9.maxNumRbPerSe;

    // GPU__GC__NUM_SE
    pInfo->primsPerClock = pInfo->gfx9.numShaderEngines;

    // Loop over each shader array and shader engine to determine actual number of active CU's (total and per SA/SE).
    uint32 numActiveCus   = 0;
    uint32 numAlwaysOnCus = 0;
    for (uint32 se = 0; se < pInfo->gfx9.numShaderEngines; ++se)
    {
        bool seActive = false;
        for (uint32 sa = 0; sa < pInfo->gfx9.numShaderArrays; ++sa)
        {
            const uint32 cuActiveMask    = pInfo->gfx9.activeCuMask[se][sa];
            const uint32 cuActiveCount   = CountSetBits(cuActiveMask);
            numActiveCus += cuActiveCount;

            const uint32 cuAlwaysOnMask  = pInfo->gfx9.alwaysOnCuMask[se][sa];
            const uint32 cuAlwaysOnCount = CountSetBits(cuAlwaysOnMask);
            numAlwaysOnCus += cuAlwaysOnCount;

            pInfo->gfx9.numCuPerSh = Max(pInfo->gfx9.numCuPerSh, cuActiveCount);

            if (cuActiveCount != 0)
            {
                seActive = true;
            }
        }
        if (seActive)
        {
            pInfo->gfx9.activeSeMask |= (1 << se);
        }
    }

    pInfo->gfx9.numActiveShaderEngines = CountSetBits(pInfo->gfx9.activeSeMask);
    PAL_ASSERT((pInfo->gfx9.numCuPerSh > 0) && (pInfo->gfx9.numCuPerSh <= pInfo->gfx9.maxNumCuPerSh));
    pInfo->gfx9.numActiveCus = numActiveCus;
    pInfo->gfx9.numAlwaysOnCus = numAlwaysOnCus;
    PAL_ASSERT((pInfo->gfx9.numActiveCus > 0)   && (pInfo->gfx9.numActiveCus <= pInfo->gfx9.numPhysicalCus));
    PAL_ASSERT((pInfo->gfx9.numAlwaysOnCus > 0) && (pInfo->gfx9.numAlwaysOnCus <= pInfo->gfx9.numPhysicalCus));

    pInfo->gfx9.nativeWavefrontSize = 32;

    // We need to increase MaxNumRbs if this assert triggers.
    PAL_ASSERT(pInfo->gfx9.numTotalRbs <= MaxNumRbs);

    // Nothing else should be set after this point.
}

// =====================================================================================================================
// Initialize default values for the GPU engine properties.
void InitializeGpuEngineProperties(
    const GpuChipProperties&  chipProps,
    GpuEngineProperties*      pInfo)
{
    const GfxIpLevel gfxIpLevel = chipProps.gfxLevel;

    auto*const  pUniversal = &pInfo->perEngine[EngineTypeUniversal];

    // We support If/Else/While on the universal and compute queues; the command stream controls the max nesting depth.
    pUniversal->flags.timestampSupport                = 1;
    pUniversal->flags.borderColorPaletteSupport       = 1;
    pUniversal->flags.queryPredicationSupport         = 1;
    // Emulated by embedding a 64-bit predicate in the cmdbuf and copying from the 32-bit source.
    pUniversal->flags.memory32bPredicationEmulated    = 1;
    pUniversal->flags.memory64bPredicationSupport     = 1;
    pUniversal->flags.conditionalExecutionSupport     = 1;
    pUniversal->flags.loopExecutionSupport            = 1;
    pUniversal->flags.constantEngineSupport           = 0;
    pUniversal->flags.regMemAccessSupport             = 1;
    pUniversal->flags.indirectBufferSupport           = 1;
    pUniversal->flags.supportsMismatchedTileTokenCopy = 1;
    pUniversal->flags.supportsImageInitBarrier        = 1;
    pUniversal->flags.supportsImageInitPerSubresource = 1;
    pUniversal->flags.supportsUnmappedPrtPageAccess   = 1;
    pUniversal->flags.memory32bPredicationSupport     = 1;
    pUniversal->flags.supportsPws                     = 1;
    pUniversal->maxControlFlowNestingDepth            = CmdStream::CntlFlowNestingLimit;
    pUniversal->minTiledImageCopyAlignment.width      = 1;
    pUniversal->minTiledImageCopyAlignment.height     = 1;
    pUniversal->minTiledImageCopyAlignment.depth      = 1;
    pUniversal->minTiledImageMemCopyAlignment.width   = 1;
    pUniversal->minTiledImageMemCopyAlignment.height  = 1;
    pUniversal->minTiledImageMemCopyAlignment.depth   = 1;
    pUniversal->minLinearMemCopyAlignment.width       = 1;
    pUniversal->minLinearMemCopyAlignment.height      = 1;
    pUniversal->minLinearMemCopyAlignment.depth       = 1;
    pUniversal->minTimestampAlignment                 = 8; // The CP spec requires 8-byte alignment.
    pUniversal->queueSupport                          = SupportQueueTypeUniversal;

    auto*const pCompute = &pInfo->perEngine[EngineTypeCompute];

    pCompute->flags.timestampSupport                = 1;
    pCompute->flags.borderColorPaletteSupport       = 1;
    pCompute->flags.queryPredicationSupport         = 1;
    pCompute->flags.memory32bPredicationSupport     = 1;
    pCompute->flags.memory64bPredicationSupport     = 1;
    pCompute->flags.conditionalExecutionSupport     = 1;
    pCompute->flags.loopExecutionSupport            = 1;
    pCompute->flags.regMemAccessSupport             = 1;
    pCompute->flags.indirectBufferSupport           = 1;
    pCompute->flags.supportsMismatchedTileTokenCopy = 1;
    pCompute->flags.supportsImageInitBarrier        = 1;
    pCompute->flags.supportsImageInitPerSubresource = 1;
    pCompute->flags.supportsUnmappedPrtPageAccess   = 1;
    pCompute->maxControlFlowNestingDepth            = CmdStream::CntlFlowNestingLimit;
    pCompute->minTiledImageCopyAlignment.width      = 1;
    pCompute->minTiledImageCopyAlignment.height     = 1;
    pCompute->minTiledImageCopyAlignment.depth      = 1;
    pCompute->minTiledImageMemCopyAlignment.width   = 1;
    pCompute->minTiledImageMemCopyAlignment.height  = 1;
    pCompute->minTiledImageMemCopyAlignment.depth   = 1;
    pCompute->minLinearMemCopyAlignment.width       = 1;
    pCompute->minLinearMemCopyAlignment.height      = 1;
    pCompute->minLinearMemCopyAlignment.depth       = 1;
    pCompute->minTimestampAlignment                 = 8; // The CP spec requires 8-byte alignment.
    pCompute->queueSupport                          = SupportQueueTypeCompute;

    // SDMA engine is part of GFXIP for all Gfx12 hardware, so set that up here
    auto*const pDma = &pInfo->perEngine[EngineTypeDma];

    pDma->flags.timestampSupport               = 1;
    pDma->flags.memory32bPredicationSupport    = 0;
    pDma->flags.memory64bPredicationSupport    = 1;
    pDma->minTiledImageCopyAlignment.width     = 16;
    pDma->minTiledImageCopyAlignment.height    = 16;
    pDma->minTiledImageCopyAlignment.depth     = 8;
    pDma->minTiledImageMemCopyAlignment.width  = 1;
    pDma->minTiledImageMemCopyAlignment.height = 1;
    pDma->minTiledImageMemCopyAlignment.depth  = 1;
    pDma->minLinearMemCopyAlignment.width      = 4;
    pDma->minLinearMemCopyAlignment.height     = 1;
    pDma->minLinearMemCopyAlignment.depth      = 1;
    pDma->minTimestampAlignment                = 8;
    pDma->queueSupport                         = SupportQueueTypeDma;

    // Note that SDMA is technically part of GFXIP now.
    pInfo->perEngine[EngineTypeDma].flags.supportsImageInitBarrier        = 1;
    pInfo->perEngine[EngineTypeDma].flags.supportsMismatchedTileTokenCopy = 1;
    pInfo->perEngine[EngineTypeDma].flags.supportsUnmappedPrtPageAccess   = 1;
}

// =====================================================================================================================
void InitializePerfExperimentProperties(
    const GpuChipProperties&  chipProps,
    PerfExperimentProperties* pProperties)
{
    const PerfCounterInfo& perfCounterInfo = chipProps.gfx9.perfCounterInfo.gfx12Info;

    pProperties->features.u32All       = perfCounterInfo.features.u32All;
    pProperties->maxSqttSeBufferSize   = static_cast<size_t>(SqttMaximumBufferSize);
    pProperties->sqttSeBufferAlignment = static_cast<size_t>(SqttBufferAlignment);
    pProperties->shaderEngineCount     = chipProps.gfx9.numActiveShaderEngines;

    for (uint32 blockIdx = 0; blockIdx < static_cast<uint32>(GpuBlock::Count); blockIdx++)
    {
        const PerfCounterBlockInfo&  blockInfo = perfCounterInfo.block[blockIdx];
        GpuBlockPerfProperties*const pBlock    = &pProperties->blocks[blockIdx];

        pBlock->available = (blockInfo.distribution != PerfCounterDistribution::Unavailable);

        if (pBlock->available)
        {
            pBlock->instanceCount         = blockInfo.numInstances;
            pBlock->maxEventId            = blockInfo.maxEventId;
            pBlock->maxGlobalOnlyCounters = blockInfo.numGlobalOnlyCounters;
            pBlock->maxSpmCounters        = Max(blockInfo.num16BitSpmCounters, blockInfo.num32BitSpmCounters);
            pBlock->instanceGroupSize     = blockInfo.instanceGroupSize;

            if (blockIdx == static_cast<uint32>(GpuBlock::DfMall))
            {
                // For DF SPM, the max number of counters is equal to the number of global counters
                pBlock->maxSpmCounters = blockInfo.numGlobalOnlyCounters;
            }

            // Note that the current interface says the shared count includes all global counters. This seems
            // to be contradictory, how can something be shared and global-only? Regardless, we cannot change this
            // without a major interface change so we must compute the total number of global counters here.
            pBlock->maxGlobalSharedCounters = blockInfo.numGlobalSharedCounters + blockInfo.numGlobalOnlyCounters;
        }
    }
}

// =====================================================================================================================
// Apply internal heuristics to decide gpu memory compression
bool DefaultGpuMemoryCompression(
    const GpuMemoryCreateInfo& gpuMemCreateInfo,
    const Gfx12PalSettings&    settings,
    bool                       isCpuVisible,
    bool                       isClient)
{
    bool result = false;

    const uint32 distCompFlags = settings.distributedCompressionMask;

    if (isCpuVisible && (TestAnyFlagSet(distCompFlags, DistCompMemCpuVisible) == false))
    {
        result = false;
    }
    else if (gpuMemCreateInfo.pImage == nullptr)
    {
        const uint32 applicableMemFlags =
            (gpuMemCreateInfo.flags.crossAdapter      ? DistCompMemCrossAdapter      : 0) |
            (gpuMemCreateInfo.flags.interprocess      ? DistCompMemInterprocess      : 0) |
            (gpuMemCreateInfo.flags.presentable       ? DistCompMemPresentable       : 0) |
            (gpuMemCreateInfo.flags.privPrimary       ? DistCompMemPrivPrimary       : 0) |
            (gpuMemCreateInfo.flags.sharedViaNtHandle ? DistCompMemSharedViaNtHandle : 0) |
            (gpuMemCreateInfo.flags.flippable         ? DistCompMemFlippable         : 0) |
            (gpuMemCreateInfo.flags.shareable         ? DistCompMemShareable         : 0);

        result = isClient                                           &&
                 // Cannot enable DCC on TMZ allocations.
                 (gpuMemCreateInfo.flags.tmzProtected == 0)         &&
                 TestAllFlagsSet(distCompFlags, applicableMemFlags) &&
                 (gpuMemCreateInfo.size >= settings.compressBufferMemoryMinSize);
    }
    else
    {
        const Pal::Image*      pImage          = static_cast<Pal::Image*>(gpuMemCreateInfo.pImage);
        const ImageCreateInfo& imageCreateInfo = gpuMemCreateInfo.pImage->GetImageCreateInfo();
        const DisplayDccCaps&  displayDcc      = pImage->GetInternalCreateInfo().displayDcc;

        const uint32 applicableImgFlags =
            (pImage->IsPresentable()  ? DistCompImgPresentable  : 0) |
            (pImage->IsShared()       ? DistCompImgShared       : 0) |
            (displayDcc.enabled       ? DistCompImgDisplayable  : 0);

        if (pImage->IsTmz() || (gpuMemCreateInfo.flags.tmzProtected != 0))
        {
            // Cannot enable DCC on TMZ surfaces.
            result = false;
        }
        else if ((displayDcc.enabled == 0) && (gpuMemCreateInfo.flags.flippable || pImage->IsFlippable()))
        {
            // Can't enable compression on flippable surfaces if no DisplayDcc is supported.
            result = false;
        }
        else if (TestAllFlagsSet(distCompFlags, applicableImgFlags) == false)
        {
            result = false;
        }
        else if ((imageCreateInfo.compressionMode      != CompressionMode::ReadBypassWriteDisable) ||
                 (settings.enableCompressionReadBypass == false))
        {
            const ImageUsageFlags usage    = imageCreateInfo.usageFlags;
            const ChNumFormat     format   = imageCreateInfo.swizzledFormat.format;
            const bool            readOnly = (usage.shaderRead   == 1) &&
                                             (usage.shaderWrite  == 0) &&
                                             (usage.colorTarget  == 0) &&
                                             (usage.depthStencil == 0) &&
                                             (usage.videoDecoder == 0);
            float                 bpp      = Pal::Formats::BitsPerPixel(format);

            if (Pal::Formats::IsBlockCompressed(format))
            {
                Extent3d blockDim  = Pal::Formats::CompressedBlockDim(format);
                uint32   blockSize = blockDim.depth * blockDim.height * blockDim.width;
                bpp                = bpp / blockSize;
            }

            result = (usage.colorTarget  && (bpp >= settings.compressColorTargetImageMinBpp))        ||
                     (usage.depthStencil && (bpp >= settings.compressDsTargetImageMinBpp))           ||
                     (usage.shaderWrite  && (bpp >= settings.compressUavTargetImageMinBpp))          ||
                     (usage.videoDecoder && (bpp >= settings.compressVideoDecoderTargetImageMinBpp)) ||
                     (readOnly           && (bpp >= settings.compressReadOnlyImageMinBpp));
        }
    }

    return result;
}

// =====================================================================================================================
Device::Device(
    Pal::Device* pDevice)
    :
    GfxDevice(pDevice, &m_rsrcProcMgr),
    m_cmdUtil(*this),
    m_barrierMgr(this),
    m_rsrcProcMgr(this),
    m_samplePatternPalette{}
{
    memset(&m_vertexAttributesMem[0], 0, sizeof(m_vertexAttributesMem));
    memset(&m_primBufferMem[0], 0, sizeof(m_primBufferMem));
    memset(&m_posBufferMem[0], 0, sizeof(m_posBufferMem));
}

// =====================================================================================================================
Device::~Device()
{
}

// =====================================================================================================================
Result Device::EarlyInit()
{
    Result result = m_pipelineLoader.Init();

    if (result == Result::Success)
    {
        result = m_rsrcProcMgr.EarlyInit();
    }

    return result;
}

// =====================================================================================================================
Result Device::LateInit()
{
    const MutexAuto lock(&m_queueContextUpdateLock);

    // If this device has been used before it will need this state zeroed.
    m_queueContextUpdateCounter = 0;

    return Result::Success;
}

// =====================================================================================================================
// Performs extra initialization which needs to be done after the parent Device is finalized.
Result Device::Finalize()
{
    Result result = GfxDevice::Finalize();

    if (result == Result::Success)
    {
        result = m_rsrcProcMgr.LateInit();
    }

    if (result == Result::Success)
    {
        result = InitOcclusionResetMem();
    }

    if (result == Result::Success)
    {
        result = AllocateVertexOutputMem();
    }

    return result;
}

// =====================================================================================================================
// This must clean up all internal GPU memory allocations and all objects created after EarlyInit. Note that EarlyInit
// is called when the platform creates the device objects so the work it does must be preserved if we are to reuse
// this device object.
Result Device::Cleanup()
{
    Result result = Result::Success;

    static_assert((sizeof(m_vertexAttributesMem) == sizeof(m_primBufferMem)) &&
                  (sizeof(m_vertexAttributesMem) == sizeof(m_posBufferMem)));

    InternalMemMgr* const pMemMgr = m_pParent->MemMgr();

    for (uint32 i = 0; i < ArrayLen(m_vertexAttributesMem); i++)
    {
        if (m_vertexAttributesMem[i].IsBound())
        {
            result = CollapseResults(result, pMemMgr->FreeGpuMem(m_vertexAttributesMem[i].Memory(),
                                                                 m_vertexAttributesMem[i].Offset()));
            m_vertexAttributesMem[i].Update(nullptr, 0);
        }

        if (m_primBufferMem[i].IsBound())
        {
            result = CollapseResults(result, pMemMgr->FreeGpuMem(m_primBufferMem[i].Memory(),
                                                                 m_primBufferMem[i].Offset()));
            m_primBufferMem[i].Update(nullptr, 0);
        }

        if (m_posBufferMem[i].IsBound())
        {
            result = CollapseResults(result, pMemMgr->FreeGpuMem(m_posBufferMem[i].Memory(),
                                                                 m_posBufferMem[i].Offset()));
            m_posBufferMem[i].Update(nullptr, 0);
        }
    }

    // RsrcProcMgr::Cleanup must be called before GfxDevice::Cleanup because the ShaderCache object referenced by
    // RsrcProcMgr is owned by GfxDevice and gets reset on GfxDevice::Cleanup.
    m_pRsrcProcMgr->Cleanup();

    if ((result == Result::Success) && m_occlusionResetSrcMem.IsBound())
    {
        result = CollapseResults(result, pMemMgr->FreeGpuMem(m_occlusionResetSrcMem.Memory(),
                                                             m_occlusionResetSrcMem.Offset()));
        m_occlusionResetSrcMem.Update(nullptr, 0);
    }

    if (result == Result::Success)
    {
        result = GfxDevice::Cleanup();
    }

    return result;
}

// =====================================================================================================================
// Returns the GB_ADDR_CONFIG register associated with this device which contains all kinds of useful info.
regGB_ADDR_CONFIG Device::GetGbAddrConfig() const
{
    regGB_ADDR_CONFIG gbAddrConfig;

    gbAddrConfig.u32All = m_pParent->ChipProperties().gfx9.gbAddrConfig;

    return gbAddrConfig;
}

#if DEBUG
// =====================================================================================================================
// Useful helper function for debugging command buffers on the GPU. This adds a WAIT_REG_MEM command to the specified
// command buffer space which waits until the device's dummy memory location contains the provided 'number' value. This
// lets engineers temporarily hang the GPU so they can inspect hardware state and command buffer contents in WinDbg, and
// then when they're finished, they can "un-hang" the GPU by modifying the memory location being waited on to contain
// the provided value.
uint32* Device::TemporarilyHangTheGpu(
    EngineType engineType,
    uint32     number,
    uint32*    pCmdSpace
    ) const
{
    return (pCmdSpace + CmdUtil::BuildWaitRegMem(engineType,
                                                 mem_space__me_wait_reg_mem__memory_space,
                                                 function__me_wait_reg_mem__equal_to_the_reference_value,
                                                 engine_sel__me_wait_reg_mem__micro_engine,
                                                 m_debugStallGpuMem.GpuVirtAddr(),
                                                 number,
                                                 UINT_MAX,
                                                 pCmdSpace));
}
#endif

// =====================================================================================================================
// Engine object factory.  Gfx12 does not need HW-specific Engine implementations.
Result Device::CreateEngine(
    EngineType engineType,
    uint32     engineIndex,
    Engine**   ppEngine)
{
    Result  result = Result::ErrorOutOfMemory;
    Engine* pEngine = nullptr;

    PAL_ASSERT((engineType == EngineTypeUniversal) ||
               (engineType == EngineTypeCompute)   ||
               (engineType == EngineTypeDma));

    pEngine = PAL_NEW(Engine, GetPlatform(), AllocInternal)(*Parent(), engineType, engineIndex);

    if (pEngine != nullptr)
    {
        result = pEngine->Init();
    }

    if (result == Result::Success)
    {
        (*ppEngine) = pEngine;
    }
    else if (pEngine != nullptr)
    {
        PAL_DELETE(pEngine, GetPlatform());
    }

    return result;
}

// =====================================================================================================================
// Finalizes any chip properties which depend on settings being read.
void Device::FinalizeChipProperties(
    GpuChipProperties* pChipProperties
    ) const
{
    const PalSettings& settings = Parent()->Settings();
    const Gfx12PalSettings& gfx12Settings = GetGfx12Settings(Parent());

    GfxDevice::FinalizeChipProperties(pChipProperties);

    switch (settings.offchipLdsBufferSize)
    {
    case OffchipLdsBufferSize1024:
        pChipProperties->gfxip.offChipTessBufferSize = 1024 * sizeof(uint32);
        break;
    case OffchipLdsBufferSize2048:
        pChipProperties->gfxip.offChipTessBufferSize = 2048 * sizeof(uint32);
        break;
    case OffchipLdsBufferSize4096:
        pChipProperties->gfxip.offChipTessBufferSize = 4096 * sizeof(uint32);
        break;
    case OffchipLdsBufferSize8192:
        pChipProperties->gfxip.offChipTessBufferSize = 8192 * sizeof(uint32);
        break;
    default:
        PAL_NEVER_CALLED();
        break;
    }

    pChipProperties->gfxip.numOffchipTessBuffers     = settings.numOffchipLdsBuffers;

    pChipProperties->gfxip.tessFactorBufferSizePerSe = settings.tessFactorBufferSizePerSe;

    pChipProperties->gfx9.gfx10.supportVrsWithDsExports = true;

}

// =====================================================================================================================
Result Device::GetLinearImageAlignments(
    LinearImageAlignments* pAlignments
    ) const
{
    Result result = Result::Success;

    if (pAlignments == nullptr)
    {
        result = Result::ErrorInvalidPointer;
    }
    else if (pAlignments->maxElementSize == 0)
    {
        result = Result::ErrorInvalidValue;
    }
    else
    {
        // Pitch alignment is now 128 bytes. base address alignment remains at 256 bytes.
        pAlignments->baseAddress = 256;
        pAlignments->rowPitch    = 128;
        pAlignments->depthPitch  = 256;
    }

    return result;
}

// =====================================================================================================================
size_t Device::GetGfxQueueRingBufferSize() const
{
    PAL_NOT_IMPLEMENTED();

    return 0;
}

// =====================================================================================================================
Result Device::CreateGfxQueueRingBuffer(
    void* pPlacementAddr,
    GfxQueueRingBuffer** ppGfxQueueRb,
    GfxQueueRingBufferCreateInfo* pGfxQueueRingBufferCreateInfo)
{
    PAL_NOT_IMPLEMENTED();

    return Result::Unsupported;
}

// =====================================================================================================================
// Determines the size of the QueueContext object needed for GFX11 hardware. Only supports GFXIP queues: Universal,
// Compute and DMA.
size_t Device::GetQueueContextSize(
    const QueueCreateInfo& createInfo
    ) const
{
    size_t size = 0;

    switch (createInfo.queueType)
    {
    case QueueTypeUniversal:
        size = sizeof(UniversalQueueContext);
        break;
    case QueueTypeCompute:
        {
            size = sizeof(ComputeQueueContext);
        }
        break;
    case QueueTypeDma:
        size = sizeof(QueueContext);
        break;
    default:
        PAL_ASSERT_ALWAYS_MSG("Unexpected GFXIP queue type.");
        break;
    }

    return size;
}

// =====================================================================================================================
// Creates the QueueContext object for the specified Queue in preallocated memory. Only supports GFXIP queues:
// Universal, Compute and DMA.  The createInfo here is not the original createInfo passed by Pal client, it is a copy
// created by the queue's constructor that may be modified.
Result Device::CreateQueueContext(
    const QueueCreateInfo& createInfo,
    Engine*                pEngine,
    void*                  pPlacementAddr,
    Pal::QueueContext**    ppQueueContext)
{
    PAL_ASSERT((pPlacementAddr != nullptr) && (ppQueueContext != nullptr));

    Result result = Result::Success;

    const uint32 engineId = createInfo.engineIndex;
    switch (createInfo.queueType)
    {
    case QueueTypeUniversal:
        {
            UniversalQueueContext* pContext = PAL_PLACEMENT_NEW(pPlacementAddr) UniversalQueueContext(this);

            result = pContext->Init();

            if (result == Result::Success)
            {
                (*ppQueueContext) = pContext;
            }
            else
            {
                pContext->Destroy();
            }
        }
        break;
    case QueueTypeCompute:
        {
            {
                ComputeQueueContext* pContext =
                    PAL_PLACEMENT_NEW(pPlacementAddr) ComputeQueueContext(this, createInfo.tmzOnly);

                result = pContext->Init();

                if (result == Result::Success)
                {
                    (*ppQueueContext) = pContext;
                }
                else
                {
                    pContext->Destroy();
                }
            }
        }
    break;
    case QueueTypeDma:
        (*ppQueueContext) = PAL_PLACEMENT_NEW(pPlacementAddr) Pal::QueueContext(Parent());
        result = Result::Success;
        break;
    default:
        PAL_ASSERT_ALWAYS_MSG("Unexpected GFXIP queue type.");
        result = Result::ErrorUnavailable;
        break;
    }

    return result;
}

// =====================================================================================================================
// Creates a command stream and fills it with a single NOP command.  Useful in cases where a submission is required
// to communicate with the OS/KMD for a queue operation but there are no client command buffers to submit.
Result Device::CreateDummyCommandStream(
    EngineType       engineType,
    Pal::CmdStream** ppCmdStream
    ) const
{
    Result result = Result::ErrorOutOfMemory;

    Pal::CmdStream* pCmdStream =
        PAL_NEW(CmdStream, GetPlatform(), AllocInternal)(*this,
                                                         Parent()->InternalUntrackedCmdAllocator(),
                                                         engineType,
                                                         SubEngineType::Primary,
                                                         CmdStreamUsage::Workload,
                                                         false);

    if (pCmdStream != nullptr)
    {
        result = pCmdStream->Init();
    }

    if (result == Result::Success)
    {
        constexpr CmdStreamBeginFlags beginFlags = {};
        pCmdStream->Reset(nullptr, true);
        pCmdStream->Begin(beginFlags, nullptr);

        uint32* pCmdSpace = pCmdStream->ReserveCommands();
        if (engineType == EngineTypeDma)
        {
            pCmdSpace = DmaCmdBuffer::BuildNops(pCmdSpace, pCmdStream->GetSizeAlignDwords());
        }
        else
        {
            pCmdSpace += CmdUtil::BuildNop(1, pCmdSpace);
        }
        pCmdStream->CommitCommands(pCmdSpace);

        result = pCmdStream->End();
    }

    if (result == Result::Success)
    {
        (*ppCmdStream) = pCmdStream;
    }
    else
    {
        PAL_SAFE_DELETE(pCmdStream, GetPlatform());
    }

    return result;
}

// =====================================================================================================================
size_t Device::GetComputePipelineSize(
    const ComputePipelineCreateInfo& createInfo,
    Result*                          pResult
    ) const
{
    PAL_ASSERT((createInfo.pPipelineBinary != nullptr) && (createInfo.pipelineBinarySize != 0));

    if (pResult != nullptr)
    {
        (*pResult) = Result::Success;
    }

    return IsElf(createInfo) ? sizeof(ComputePipeline) : sizeof(ArchivePipeline);
}

// =====================================================================================================================
Result Device::CreateComputePipeline(
    const ComputePipelineCreateInfo& createInfo,
    void*                            pPlacementAddr,
    bool                             isInternal,
    IPipeline**                      ppPipeline)
{
    PAL_ASSERT(createInfo.pPipelineBinary != nullptr);
    PAL_ASSERT(pPlacementAddr != nullptr);

    Result result = Result::Success;
    Pal::Pipeline* pPipeline = nullptr;

    if (IsElf(createInfo))
    {
        AbiReader abiReader(GetPlatform(), {createInfo.pPipelineBinary, createInfo.pipelineBinarySize});
        result = abiReader.Init(createInfo.pKernelName);

        if (result == Result::Success)
        {
            MsgPackReader metadataReader;
            PalAbi::CodeObjectMetadata metadata = {};
            const uint8 abi = abiReader.GetOsAbi();
            if (abi == Abi::ElfOsAbiAmdgpuPal)
            {
                result = abiReader.GetMetadata(&metadataReader, &metadata);
            }

            auto* pComputePipeline = PAL_PLACEMENT_NEW(pPlacementAddr) ComputePipeline(this, isInternal);
            result    = pComputePipeline->Init(createInfo, abiReader, metadata, &metadataReader);
            pPipeline = pComputePipeline;
        }
    }
    else
    {
        auto* pArchive = PAL_PLACEMENT_NEW(pPlacementAddr) ArchivePipeline(this->Parent(), isInternal);
        result    = pArchive->Init(createInfo);
        pPipeline = pArchive;
    }

    if ((result != Result::Success) && (pPipeline != nullptr))
    {
        pPipeline->Destroy();
        pPipeline = nullptr;
    }

    *ppPipeline = pPipeline;

    return result;
}

// =====================================================================================================================
size_t Device::GetShaderLibrarySize(
    const ShaderLibraryCreateInfo& createInfo,
    Result*                        pResult) const
{
    if (pResult != nullptr)
    {
        (*pResult) = Result::Success;
    }

    static_assert(sizeof(GraphicsPipeline) <= sizeof(HybridGraphicsPipeline));

    return (createInfo.flags.isGraphics) ?
        sizeof(GraphicsShaderLibrary) + sizeof(HybridGraphicsPipeline) :
        sizeof(ComputeShaderLibrary);
}

// =====================================================================================================================
Result Device::CreateShaderLibrary(
    const ShaderLibraryCreateInfo&  createInfo,
    void*                           pPlacementAddr,
    bool                            isInternal,
    IShaderLibrary**                ppPipeline)
{
    PAL_ASSERT(createInfo.pCodeObject != nullptr);
    PAL_ASSERT(pPlacementAddr != nullptr);

    // Create shader lib
    Pal::ShaderLibrary* pShaderLib = nullptr;

    if (createInfo.flags.isGraphics)
    {
        pShaderLib = PAL_PLACEMENT_NEW(pPlacementAddr) GraphicsShaderLibrary(this);
    }
    else
    {
        pShaderLib = PAL_PLACEMENT_NEW(pPlacementAddr) ComputeShaderLibrary(this);
    }

    Result result = pShaderLib->InitializeCodeObject(createInfo);

    Span<const void> codeObj = {};
    if (result == Result::Success)
    {
        // Retrieve the code object from the shader library.
        // The AbiReader uses this pointer because the ShaderLibrary object may retain pointers to code object memory
        // via the AbiReader. This pointer will be valid for the lifetime of the ShaderLibrary object.
        // The client provided data may be deleted while the ShaderLibrary object is still in use.
        codeObj = pShaderLib->GetCodeObject();
    }

    if (codeObj.IsEmpty() == false)
    {
        AbiReader abiReader(GetPlatform(), codeObj);
        result = abiReader.Init();

        MsgPackReader              metadataReader;
        PalAbi::CodeObjectMetadata metadata;

        if (result == Result::Success)
        {
            result = abiReader.GetMetadata(&metadataReader, &metadata);
        }

        if (result == Result::Success)
        {
            result = pShaderLib->InitFromCodeObjectBinary(createInfo, abiReader, metadata, &metadataReader);
        }
    }

    if (result != Result::Success)
    {
        pShaderLib->Destroy();
        pShaderLib = nullptr;
    }

    *ppPipeline = pShaderLib;

    return result;
}

// =====================================================================================================================
size_t Device::GetGraphicsPipelineSize(
    const GraphicsPipelineCreateInfo& createInfo,
    bool                              isInternal,
    Result*                           pResult
    ) const
{
    PAL_ASSERT(((createInfo.pPipelineBinary   != nullptr) && (createInfo.pipelineBinarySize != 0)) ||
               ((createInfo.ppShaderLibraries != nullptr) && (createInfo.numShaderLibraries > 0)));

    if (pResult != nullptr)
    {
        (*pResult) = Result::Success;
    }

    static_assert(sizeof(GraphicsPipeline) <= sizeof(HybridGraphicsPipeline));

    return sizeof(HybridGraphicsPipeline);
}

// =====================================================================================================================
Result Device::CreateGraphicsPipeline(
    const GraphicsPipelineCreateInfo&         createInfo,
    const GraphicsPipelineInternalCreateInfo& internalInfo,
    void*                                     pPlacementAddr,
    bool                                      isInternal,
    IPipeline**                               ppPipeline)
{
    Result                      result          = Result::Success;
    AbiReader*                  pAbiReader      = nullptr;
    MsgPackReader*              pMetadataReader = nullptr;
    PalAbi::CodeObjectMetadata* pMetadata       = nullptr;
    bool                        hasTask         = false;

    alignas(AbiReader)                  uint8 abiReaderBuffer[sizeof(AbiReader)];
    alignas(MsgPackReader)              uint8 msgPackReaderBuffer[sizeof(MsgPackReader)];
    alignas(PalAbi::CodeObjectMetadata) uint8 metaDataBuffer[sizeof(PalAbi::CodeObjectMetadata)];

    if (createInfo.numShaderLibraries > 0)
    {
        for (uint32 i = 0; i < createInfo.numShaderLibraries; i++)
        {
            PAL_ASSERT(reinterpret_cast<const ShaderLibrary*>(createInfo.ppShaderLibraries[i])->IsGraphics());
            const auto* pLib = reinterpret_cast<const GraphicsShaderLibrary*>(createInfo.ppShaderLibraries[i]);
            if (Util::TestAnyFlagSet(pLib->GetApiShaderMask(), ApiShaderStageTask))
            {
                hasTask = true;
                break;
            }
        }
    }
    else
    {
        PAL_ASSERT(createInfo.pPipelineBinary != nullptr);
        PAL_ASSERT(pPlacementAddr != nullptr);
        const Span<const void> pipelineBinary = { createInfo.pPipelineBinary, createInfo.pipelineBinarySize };
        pAbiReader      = PAL_PLACEMENT_NEW(abiReaderBuffer)AbiReader(GetPlatform(), pipelineBinary);
        result          = pAbiReader->Init();
        pMetadataReader = PAL_PLACEMENT_NEW(msgPackReaderBuffer)MsgPackReader();
        pMetadata       = PAL_PLACEMENT_NEW(metaDataBuffer)PalAbi::CodeObjectMetadata{};

        if (result == Result::Success)
        {
            result = pAbiReader->GetMetadata(pMetadataReader, pMetadata);
        }

        if (result == Result::Success)
        {
            const auto& shaderMetadata = pMetadata->pipeline.shader[static_cast<uint32>(Abi::ApiShaderType::Task)];
            hasTask = ShaderHashIsNonzero({ shaderMetadata.apiShaderHash[0], shaderMetadata.apiShaderHash[1] });
        }
    }

    if (result == Result::Success)
    {
        if (hasTask)
        {
            PAL_PLACEMENT_NEW(pPlacementAddr) HybridGraphicsPipeline(this, isInternal);
        }
        else
        {
            PAL_PLACEMENT_NEW(pPlacementAddr) GraphicsPipeline(this, isInternal);
        }

        GraphicsPipeline* pPipeline = static_cast<GraphicsPipeline*>(pPlacementAddr);
        result = pPipeline->Init(createInfo, internalInfo, pAbiReader, pMetadata, pMetadataReader);

        if (result != Result::Success)
        {
            pPipeline->Destroy();
        }
        else
        {
            *ppPipeline = pPipeline;
        }
    }

    Util::Destructor(pAbiReader);
    Util::Destructor(pMetadata);
    Util::Destructor(pMetadataReader);

    return result;
}

// =====================================================================================================================
size_t Device::GetColorBlendStateSize(
    ) const
{
    return sizeof(ColorBlendState);
}

// =====================================================================================================================
Result Device::CreateColorBlendState(
    const ColorBlendStateCreateInfo& createInfo,
    void*                            pPlacementAddr,
    IColorBlendState**               ppColorBlendState
    ) const
{
    *ppColorBlendState = PAL_PLACEMENT_NEW(pPlacementAddr) ColorBlendState(*this, createInfo);

    return Result::Success;
}

// =====================================================================================================================
size_t Device::GetDepthStencilStateSize(
    ) const
{
    return sizeof(DepthStencilState);
}

// =====================================================================================================================
Result Device::CreateDepthStencilState(
    const DepthStencilStateCreateInfo& createInfo,
    void*                              pPlacementAddr,
    IDepthStencilState**               ppDepthStencilState
    ) const
{
    *ppDepthStencilState = PAL_PLACEMENT_NEW(pPlacementAddr) DepthStencilState(*this, createInfo);

    return Result::Success;
}

// =====================================================================================================================
size_t Device::GetMsaaStateSize(
    ) const
{
    return sizeof(MsaaState);
}

// =====================================================================================================================
Result Device::CreateMsaaState(
    const MsaaStateCreateInfo& createInfo,
    void*                      pPlacementAddr,
    IMsaaState**               ppMsaaState
    ) const
{
    *ppMsaaState = PAL_PLACEMENT_NEW(pPlacementAddr) MsaaState(*this, createInfo);

    return Result::Success;
}

// =====================================================================================================================
size_t Device::GetImageSize(
    const ImageCreateInfo& createInfo
    ) const
{
    return sizeof(Image);
}

// =====================================================================================================================
// Supposed to help improve the clone copy preference logic by sharing client tuning data with PAL.
bool Device::ImagePrefersCloneCopy(
    const ImageCreateInfo& createInfo
    ) const
{
    const ChNumFormat format    = createInfo.swizzledFormat.format;
    const Extent3d&   extent    = createInfo.extent;
    bool              cloneCopy = false;

    // Allow for some single sample D16 depth images.
    if ((createInfo.fragments == 1)               &&
        (createInfo.usageFlags.depthStencil != 0) &&
        (Formats::BitsPerPixel(format) == 16))
    {
        // For image size meets (W*H*ArraySize) <= (3840*2160*6).
        cloneCopy = ((uint64(extent.width) * extent.height * createInfo.arraySize) <= (3840 * 2160 * 6));
    }
    // Allow for some 8bpp MSAA images.
    else if ((createInfo.fragments > 1) && (Formats::BitsPerPixel(format) == 8))
    {
        // For image size meets (W*H*ArraySize*fragments) <= (3840*2160*1*4).
        cloneCopy = ((uint64(extent.width) * extent.height * createInfo.arraySize * createInfo.fragments) <=
                     (3840 * 2160 * 4));
    }

    return cloneCopy;
}

// =====================================================================================================================
// Creates a concrete Gfx12 GfxImage object
void Device::CreateImage(
    Pal::Image* pParentImage,
    ImageInfo*  pImageInfo,
    void*       pPlacementAddr,
    GfxImage**  ppImage
    ) const
{
    (*ppImage) = PAL_PLACEMENT_NEW(pPlacementAddr) Image(pParentImage, pImageInfo, *m_pParent);
}

// =====================================================================================================================
size_t Device::GetBorderColorPaletteSize(
    const BorderColorPaletteCreateInfo& createInfo,
    Result*                             pResult
    ) const
{
    if (pResult != nullptr)
    {
        *pResult = Result::Success;
    }

    return sizeof(BorderColorPalette);
}

// =====================================================================================================================
Result Device::CreateBorderColorPalette(
    const BorderColorPaletteCreateInfo& createInfo,
    void*                               pPlacementAddr,
    IBorderColorPalette**               ppBorderColorPalette
    ) const
{
    *ppBorderColorPalette = PAL_PLACEMENT_NEW(pPlacementAddr) BorderColorPalette(*this, createInfo);

    return Result::Success;
}

// =====================================================================================================================
size_t Device::GetQueryPoolSize(
    const QueryPoolCreateInfo& createInfo,
    Result*                    pResult
    ) const
{
    size_t queryPoolSize = 0;

    if (pResult != nullptr)
    {
        if (((createInfo.queryPoolType != QueryPoolType::Occlusion)       &&
             (createInfo.queryPoolType != QueryPoolType::PipelineStats)   &&
             (createInfo.queryPoolType != QueryPoolType::StreamoutStats)) ||
             (createInfo.numSlots == 0))
        {
            *pResult = Result::ErrorInvalidValue;
        }
        else
        {
            *pResult = Result::Success;
        }
    }

    if (createInfo.queryPoolType == QueryPoolType::Occlusion)
    {
        queryPoolSize = sizeof(OcclusionQueryPool);
    }
    else if (createInfo.queryPoolType == QueryPoolType::PipelineStats)
    {
        queryPoolSize = sizeof(PipelineStatsQueryPool);
    }
    else if (createInfo.queryPoolType == QueryPoolType::StreamoutStats)
    {
        queryPoolSize = sizeof(StreamoutStatsQueryPool);
    }
    else
    {
        PAL_ASSERT_ALWAYS();
    }

    return queryPoolSize;
}

// =====================================================================================================================
Result Device::CreateQueryPool(
    const QueryPoolCreateInfo& createInfo,
    void*                      pPlacementAddr,
    IQueryPool**               ppQueryPool
    ) const
{
    Result result = Result::Success;

    if (createInfo.queryPoolType == QueryPoolType::Occlusion)
    {
        *ppQueryPool = PAL_PLACEMENT_NEW(pPlacementAddr) OcclusionQueryPool(*this, createInfo);
    }
    else if (createInfo.queryPoolType == QueryPoolType::PipelineStats)
    {
        *ppQueryPool = PAL_PLACEMENT_NEW(pPlacementAddr) PipelineStatsQueryPool(*this, createInfo);
    }
    else if (createInfo.queryPoolType == QueryPoolType::StreamoutStats)
    {
        *ppQueryPool = PAL_PLACEMENT_NEW(pPlacementAddr) StreamoutStatsQueryPool(*this, createInfo);
    }
    else
    {
        PAL_NOT_IMPLEMENTED();

        result = Result::Unsupported;
    }

    return result;

}

// =====================================================================================================================
// As a performance optimization, we have a small piece of video memory which contains the reset values for each slot in
// an occlusion query pool. This initializes that memory for future use.
Result Device::InitOcclusionResetMem()
{
    Result result = Result::Success;

    const GpuChipProperties& chipProps = m_pParent->ChipProperties();

    // Initialize our copy of the reset data for a single query slot.
    memset(&m_occlusionSlotResetValues[0], 0, sizeof(m_occlusionSlotResetValues));

    PAL_ASSERT(chipProps.gfx9.numTotalRbs <= MaxNumRbs);

    // For GFX9+, rbs pack the results of active rbs in-order.
    for (uint32 rb = chipProps.gfx9.numActiveRbs; rb < chipProps.gfx9.numTotalRbs; rb++)
    {
        m_occlusionSlotResetValues[rb].begin.bits.valid = 1;
        m_occlusionSlotResetValues[rb].end.bits.valid   = 1;
    }

    const size_t slotSize = chipProps.gfx9.numTotalRbs * sizeof(OcclusionQueryResultPair);

    PAL_ALERT(slotSize > sizeof(m_occlusionSlotResetValues));

    // Use this VRAM as a source for the ResetOcclusionQueryPool.
    GpuMemoryCreateInfo srcMemCreateInfo = { };
    srcMemCreateInfo.alignment = sizeof(uint32);
    srcMemCreateInfo.size      = ResetOcclusionQueryPoolSrcSlots * slotSize;
    srcMemCreateInfo.priority  = GpuMemPriority::Normal;
    srcMemCreateInfo.heaps[0]  = GpuHeapLocal;
    srcMemCreateInfo.heaps[1]  = GpuHeapGartUswc;
    srcMemCreateInfo.heapCount = 2;

    GpuMemoryInternalCreateInfo internalInfo = { };
    internalInfo.flags.alwaysResident = 1;

    GpuMemory* pMemObj   = nullptr;
    gpusize    memOffset = 0;

    result = m_pParent->MemMgr()->AllocateGpuMem(srcMemCreateInfo, internalInfo, false, &pMemObj, &memOffset);

    uint8* pData = nullptr;
    if (result == Result::Success)
    {
        m_occlusionResetSrcMem.Update(pMemObj, memOffset);

        if ((m_pParent->GetPlatform() != nullptr) &&
            (m_pParent->GetPlatform()->GetGpuMemoryEventProvider() != nullptr))
        {
            ResourceDescriptionMiscInternal desc;
            desc.type = MiscInternalAllocType::OcclusionQueryResetData;

            ResourceCreateEventData createData = {};
            createData.type = ResourceType::MiscInternal;
            createData.pObj = &m_occlusionResetSrcMem;
            createData.pResourceDescData = &desc;
            createData.resourceDescSize = sizeof(ResourceDescriptionMiscInternal);

            m_pParent->GetPlatform()->GetGpuMemoryEventProvider()->LogGpuMemoryResourceCreateEvent(createData);

            GpuMemoryResourceBindEventData bindData = {};
            bindData.pGpuMemory = pMemObj;
            bindData.pObj = &m_occlusionResetSrcMem;
            bindData.offset = memOffset;
            bindData.requiredGpuMemSize = srcMemCreateInfo.size;
            m_pParent->GetPlatform()->GetGpuMemoryEventProvider()->LogGpuMemoryResourceBindEvent(bindData);

            Developer::BindGpuMemoryData callbackData = {};
            callbackData.pObj               = bindData.pObj;
            callbackData.requiredGpuMemSize = bindData.requiredGpuMemSize;
            callbackData.pGpuMemory         = bindData.pGpuMemory;
            callbackData.offset             = bindData.offset;
            callbackData.isSystemMemory     = bindData.isSystemMemory;
            m_pParent->DeveloperCb(Developer::CallbackType::BindGpuMemory, &callbackData);
        }

        result = m_occlusionResetSrcMem.Map(reinterpret_cast<void**>(&pData));
    }

    // Populate the buffer with occlusion query reset data.
    if (result == Result::Success)
    {
        for (uint32 slot = 0; slot < Pal::Device::OcclusionQueryDmaBufferSlots; ++slot)
        {
            memcpy(pData, m_occlusionSlotResetValues, slotSize);
            pData += slotSize;
        }

        result = m_occlusionResetSrcMem.Unmap();
    }

    return result;
}

// =====================================================================================================================
// Returns the size required for a Gfx12 command buffer object.
size_t Device::GetCmdBufferSize(
    const CmdBufferCreateInfo& createInfo
    ) const
{
    size_t size = 0;

    switch (createInfo.queueType)
    {
    case QueueTypeUniversal:
        size = sizeof(UniversalCmdBuffer);
        break;
    case QueueTypeCompute:
        size = sizeof(ComputeCmdBuffer);
        break;
    case QueueTypeDma:
        size = sizeof(DmaCmdBuffer);
        break;
    default:
        PAL_ASSERT_ALWAYS_MSG("Unexpected GFXIP queue type.");
        break;
    }

    return size;
}

// =====================================================================================================================
// Factory method to create the appropriate Gfx12 command buffer object.  This factory method is in the HWL to give it
// final control of the device config struct passed to the command buffer constructor.  This lets the HWL control
// feature capabilities, limits, settings, etc. that may vary per GFXIP/chip.
Result Device::CreateCmdBuffer(
    const CmdBufferCreateInfo& createInfo,
    void*                      pPlacementAddr,
    Pal::CmdBuffer**           ppCmdBuffer)
{
    Result result = Result::ErrorInvalidQueueType;

    switch (createInfo.queueType)
    {
    case QueueTypeUniversal:
        {
            UniversalCmdBufferDeviceConfig deviceConfig = { };

            // Let HW-independent layer initialize the device config struct as it is able based on generic panel
            // settings, OS-specific configuration, client-specific configuration, etc.
            InitUniversalCmdBufferDeviceConfig(&deviceConfig);

            // Override deviceConfig in any HW-specific way here (HW feature capabilities, etc.).

            *ppCmdBuffer = PAL_PLACEMENT_NEW(pPlacementAddr) UniversalCmdBuffer(*this, createInfo, deviceConfig);
            result = Result::Success;
        }
        break;
    case QueueTypeCompute:
        {
            ComputeCmdBufferDeviceConfig deviceConfig = {};

            InitComputeCmdBufferDeviceConfig(&deviceConfig);

            *ppCmdBuffer = PAL_PLACEMENT_NEW(pPlacementAddr) ComputeCmdBuffer(*this, createInfo, deviceConfig);
            result = Result::Success;
        }
        break;
    case QueueTypeDma:
        {
            *ppCmdBuffer = PAL_PLACEMENT_NEW(pPlacementAddr) DmaCmdBuffer(*m_pParent, createInfo);
            result = Result::Success;
        }
        break;
    default:
        PAL_ASSERT_ALWAYS_MSG("Unexpected GFXIP queue type.");
        break;
    }

    return result;
}

// =====================================================================================================================
Result Device::CreateCmdUploadRingInternal(
    const CmdUploadRingCreateInfo& createInfo,
    Pal::CmdUploadRing**           ppCmdUploadRing)
{
    return CmdUploadRing::CreateInternal(createInfo, this, ppCmdUploadRing);
}

// =====================================================================================================================
size_t Device::GetIndirectCmdGeneratorSize(
    const IndirectCmdGeneratorCreateInfo& createInfo,
    Result*                               pResult
    ) const
{
    if (pResult != nullptr)
    {
        *pResult = IndirectCmdGenerator::ValidateCreateInfo(createInfo);
    }

    return IndirectCmdGenerator::GetSize(createInfo);
}

// =====================================================================================================================
Result Device::CreateIndirectCmdGenerator(
    const IndirectCmdGeneratorCreateInfo& createInfo,
    void*                                 pPlacementAddr,
    IIndirectCmdGenerator**               ppGenerator
    ) const
{
    *ppGenerator = PAL_PLACEMENT_NEW(pPlacementAddr) IndirectCmdGenerator(*this, createInfo);

    return Result::Success;
}

// =====================================================================================================================
size_t Device::GetColorTargetViewSize(
    Result* pResult
    ) const
{
    if (pResult != nullptr)
    {
        (*pResult) = Result::Success;
    }

    return sizeof(ColorTargetView);
}

// =====================================================================================================================
Result Device::CreateColorTargetView(
    const ColorTargetViewCreateInfo&  createInfo,
    ColorTargetViewInternalCreateInfo internalInfo,
    void*                             pPlacementAddr,
    IColorTargetView**                ppColorTargetView
    ) const
{
    const uint32 viewId = m_nextColorTargetViewId++;

    (*ppColorTargetView) = PAL_PLACEMENT_NEW(pPlacementAddr) ColorTargetView(this, createInfo, internalInfo, viewId);

    return Result::Success;
}

// =====================================================================================================================
size_t Device::GetDepthStencilViewSize(
    Result* pResult
    ) const
{
    if (pResult != nullptr)
    {
        (*pResult) = Result::Success;
    }

    return sizeof(DepthStencilView);
}

// =====================================================================================================================
Result Device::CreateDepthStencilView(
    const DepthStencilViewCreateInfo&         createInfo,
    const DepthStencilViewInternalCreateInfo& internalInfo,
    void*                                     pPlacementAddr,
    IDepthStencilView**                       ppDepthStencilView
    ) const
{
    const uint32 viewId = m_nextDepthStencilViewId++;

    (*ppDepthStencilView) = PAL_PLACEMENT_NEW(pPlacementAddr) DepthStencilView(this, createInfo, internalInfo, viewId);

    return Result::Success;
}

// =====================================================================================================================
void Device::InitUniversalCmdBufferDeviceConfig(
    UniversalCmdBufferDeviceConfig* pDeviceConfig)
{
    const auto* const pPublicSettings  = Parent()->GetPublicSettings();
    auto              platformSettings = m_pParent->GetPlatform()->PlatformSettings();

    pDeviceConfig->tossPointMode                  = CoreSettings().tossPointMode;
    pDeviceConfig->disableBorderColorPaletteBinds = Settings().disableBorderColorPaletteBinds;
    pDeviceConfig->issueSqttMarkerEvent           = m_pParent->IssueSqttMarkerEvents();
    pDeviceConfig->describeDrawDispatch           = (pDeviceConfig->issueSqttMarkerEvent ||
                                                     platformSettings.cmdBufferLoggerConfig.embedDrawDispatchInfo ||
                                                     m_pParent->IssueCrashAnalysisMarkerEvents());
    pDeviceConfig->batchBreakOnNewPs              = Settings().batchBreakOnNewPixelShader;
    pDeviceConfig->pwsEnabled                     = Parent()->UsePws(EngineTypeUniversal);
    pDeviceConfig->pwsLateAcquirePointEnabled     = Parent()->UsePwsLateAcquirePoint(EngineTypeUniversal);
    pDeviceConfig->enableReleaseMemWaitCpDma      = EnableReleaseMemWaitCpDma();

    constexpr uint32 CcTagSize  = 1024;
    constexpr uint32 CcReadTags = 31;
    constexpr uint32 ZsTagSize  = 64;
    constexpr uint32 ZsNumTags  = 312;

    const GpuChipProperties& chipProps = Parent()->ChipProperties();

    uint32 totalNumRbs   = chipProps.gfx9.numActiveRbs;
    uint32 totalNumPipes = Util::Max(totalNumRbs, chipProps.gfx9.numSdpInterfaces);

    pDeviceConfig->pbb.colorBinSizeNumerator = ((CcReadTags * totalNumRbs / totalNumPipes) *
                                                (CcTagSize * totalNumPipes));

    pDeviceConfig->pbb.depthBinSizeNumerator  = ((ZsNumTags * totalNumRbs / totalNumPipes) *
                                                 (ZsTagSize * totalNumPipes));

    pDeviceConfig->pbb.minBinSize = { 128, 128 };
    pDeviceConfig->pbb.maxBinSize = { 512, 512 };

    pDeviceConfig->maxScissorSize    = MaxScissorSize;
    // VRS surface is calculated in tiles (8x8 pixels)
    pDeviceConfig->maxVrsRateCoord   = (MaxImageWidth / 8) - 1;

    pDeviceConfig->prefetchClampSize = CoreSettings().prefetchClampSize;

    pDeviceConfig->stateFilterFlags = Settings().gfx12RedundantStateFilter;

    pDeviceConfig->has32bitPredication =
        Parent()->EngineProperties().perEngine[EngineTypeUniversal].flags.memory32bPredicationSupport;

    pDeviceConfig->enablePreamblePipelineStats = Settings().enablePreamblePipelineStats;

#if PAL_DEVELOPER_BUILD
    pDeviceConfig->enablePm4Instrumentation    = GetPlatform()->PlatformSettings().pm4InstrumentorEnabled;
#endif

    pDeviceConfig->binningMaxPrimPerBatch = pPublicSettings->binningMaxPrimPerBatch;
    pDeviceConfig->customBatchBinSize     = pPublicSettings->customBatchBinSize;
    pDeviceConfig->binningMode            = pPublicSettings->binningMode;

    PAL_ASSERT(chipProps.gfx9.rbPlus == 1);///< All known GFX12 chips are RB+.
    pDeviceConfig->optimizeDepthOnlyFmt   = pPublicSettings->optDepthOnlyExportRate;

    pDeviceConfig->overrideCsDispatchPingPongMode = Settings().overrideCsDispatchPingPongMode;
    pDeviceConfig->temporalHintsIbRead            = Settings().gfx12TemporalHintsIbRead;

    // Initialize workarounds
    pDeviceConfig->workarounds.walkAlign64kScreenSpace    = Settings().waWalkAlign64kScreenSpace;
    pDeviceConfig->workarounds.drawOpaqueSqNonEvents      = Settings().waDrawOpaqueSqNonEvents;
    pDeviceConfig->workarounds.hiszEventBasedWar          = Settings().waHiZsBopTsEventAfterDraw;
    pDeviceConfig->workarounds.forceReZWhenHiZsDisabledWa = Settings().forceReZWhenHiZsDisabledWa;
    pDeviceConfig->workarounds.waDbForceStencilValid      = Settings().waDbForceStencilValid;

    pDeviceConfig->dispatchInterleaveSize2DMinX                = Settings().dispatchInterleaveSize2DMinX;
    pDeviceConfig->dispatchInterleaveSize2DMinY                = Settings().dispatchInterleaveSize2DMinY;
    pDeviceConfig->allow2dDispatchInterleaveOnIndirectDispatch = Settings().allow2dDispatchInterleaveOnIndirectDispatch;

    pDeviceConfig->cpPfpVersion = m_pParent->ChipProperties().pfpUcodeVersion;

    constexpr Gfx12DynamicCbTemporalHints DefaultClientDynamicCbTemporalHints =
    Gfx12DynamicCbTemporalHints(Gfx12DynamicCbTemporalHintsBlendReadsDest | Gfx12DynamicCbTemporalHintsReadAfterWrite);

    if ((Settings().gfx12DynamicCbTemporalHints == Gfx12DynamicCbTemporalHintsHonorClient) &&
        (Settings().gfx12TemporalHintsMrtRead   == Gfx12TemporalHintsReadHonorClient) &&
        (Settings().gfx12TemporalHintsMrtWrite  == Gfx12TemporalHintsWriteHonorClient))
    {
        switch (pPublicSettings->temporalHintsMrtBehavior)
        {
        case TemporalHintsDynamicRt:
            pDeviceConfig->dynCbTemporalHints         = DefaultClientDynamicCbTemporalHints;
            pDeviceConfig->gfx12TemporalHintsMrtRead  = Gfx12TemporalHintsReadNt;
            pDeviceConfig->gfx12TemporalHintsMrtWrite = Gfx12TemporalHintsWriteNt;
            break;
        case TemporalHintsStaticRt:
            pDeviceConfig->dynCbTemporalHints         = Gfx12DynamicCbTemporalHintsNone;
            pDeviceConfig->gfx12TemporalHintsMrtRead  = Gfx12TemporalHintsReadNtRt;
            pDeviceConfig->gfx12TemporalHintsMrtWrite = Gfx12TemporalHintsWriteNtRt;
            break;
        case TemporalHintsStaticNt:
            pDeviceConfig->dynCbTemporalHints         = Gfx12DynamicCbTemporalHintsNone;
            pDeviceConfig->gfx12TemporalHintsMrtRead  = Gfx12TemporalHintsReadNt;
            pDeviceConfig->gfx12TemporalHintsMrtWrite = Gfx12TemporalHintsWriteNt;
            break;
        default:
            PAL_NEVER_CALLED();
        }
    }
    else
    {
        PAL_ASSERT(Settings().gfx12DynamicCbTemporalHints != Gfx12DynamicCbTemporalHintsHonorClient);
        pDeviceConfig->dynCbTemporalHints =
            (Settings().gfx12DynamicCbTemporalHints == Gfx12DynamicCbTemporalHintsHonorClient) ?
            DefaultClientDynamicCbTemporalHints : Settings().gfx12DynamicCbTemporalHints;

        PAL_ASSERT(Settings().gfx12TemporalHintsMrtRead != Gfx12TemporalHintsReadHonorClient);
        pDeviceConfig->gfx12TemporalHintsMrtRead =
            (Settings().gfx12TemporalHintsMrtRead == Gfx12TemporalHintsReadHonorClient) ?
            Gfx12TemporalHintsReadNt : Settings().gfx12TemporalHintsMrtRead;

        PAL_ASSERT(Settings().gfx12TemporalHintsMrtWrite != Gfx12TemporalHintsWriteHonorClient);
        pDeviceConfig->gfx12TemporalHintsMrtWrite =
            (Settings().gfx12TemporalHintsMrtWrite == Gfx12TemporalHintsWriteHonorClient) ?
            Gfx12TemporalHintsWriteNt : Settings().gfx12TemporalHintsMrtWrite;
    }

    pDeviceConfig->gfx12TemporalHintsMrtReadBlendReadsDst  = Settings().gfx12TemporalHintsMrtReadBlendReadsDst;
    pDeviceConfig->gfx12TemporalHintsMrtWriteBlendReadsDst = Settings().gfx12TemporalHintsMrtWriteBlendReadsDst;
    pDeviceConfig->gfx12TemporalHintsMrtReadRaw            = Settings().gfx12TemporalHintsMrtReadRaw;
    pDeviceConfig->gfx12TemporalHintsMrtWriteRaw           = Settings().gfx12TemporalHintsMrtWriteRaw;
}

// =====================================================================================================================
// Initializes any HW-independent fields in a ComputeCmdBufferDeviceConfig struct.  This should include any
// required HW-independent panel settings, etc.
void Device::InitComputeCmdBufferDeviceConfig(
    ComputeCmdBufferDeviceConfig* pDeviceConfig)
{
    pDeviceConfig->prefetchClampSize = CoreSettings().prefetchClampSize;

    pDeviceConfig->disableBorderColorPaletteBinds = Settings().disableBorderColorPaletteBinds;
    pDeviceConfig->enablePreamblePipelineStats    = Settings().enablePreamblePipelineStats;
#if PAL_DEVELOPER_BUILD
    pDeviceConfig->enablePm4Instrumentation       = GetPlatform()->PlatformSettings().pm4InstrumentorEnabled;
#endif
    pDeviceConfig->issueSqttMarkerEvent           = m_pParent->IssueSqttMarkerEvents();
    pDeviceConfig->enableReleaseMemWaitCpDma      = EnableReleaseMemWaitCpDma();
}

// =====================================================================================================================
// Fills in the AddrLib create input fields based on chip specific properties. Note: at this point during init, settings
// have only been partially initialized. Only settings and member variables that are not impacted by validation or the
// client driver may be used.
Result Device::InitAddrLibCreateInput(
    ADDR_CREATE_FLAGS*   pCreateFlags,
    ADDR_REGISTER_VALUE* pRegValue
    ) const
{
    pRegValue->gbAddrConfig = m_pParent->ChipProperties().gfx9.gbAddrConfig;
    pCreateFlags->nonPower2MemConfig = (IsPowerOfTwo(m_pParent->MemoryProperties().vramBusBitWidth) == false);

    return Result::Success;
}

// =====================================================================================================================
Result Device::SetSamplePatternPalette(
    const SamplePatternPalette& palette)
{
    const MutexAuto lock(&m_queueContextUpdateLock);

    memcpy(const_cast<SamplePatternPalette*>(&m_samplePatternPalette), palette, sizeof(m_samplePatternPalette));

    // Increment counter to trigger later sample pattern palette update during submission
    m_queueContextUpdateCounter++;

    return Result::Success;
}

// =====================================================================================================================
void Device::GetSamplePatternPalette(
    SamplePatternPalette* pSamplePatternPalette)
{
    const MutexAuto lock(&m_queueContextUpdateLock);

    memcpy(pSamplePatternPalette,
        const_cast<const SamplePatternPalette*>(&m_samplePatternPalette),
        sizeof(m_samplePatternPalette));
}

// =====================================================================================================================
// Computes the CONTEXT_CONTROL value that should be used for universal engine submissions.  This will vary based on
// whether preemption is enabled or not, and the gfx ip level.  This exists as a helper function since there are cases
// where the command buffer may want to temporarily override the default value written by the queue context, and it
// needs to be able to restore it to the proper original value.
PM4_PFP_CONTEXT_CONTROL Device::GetContextControl() const
{
    PM4_PFP_CONTEXT_CONTROL contextControl = { };

    // Disable state shadowing by default if CP managed state shadowing isn't supported yet.
    // Note that there will be issue for write register via RMW packets if state shadowing is disabled.
    if (m_pParent->SupportStateShadowingByCpFw())
    {
        // Since PAL doesn't preserve GPU state across command buffer boundaries, we always need to enable loading
        // context and SH registers.
        contextControl.ordinal2.bitfields.update_load_enables    = 1;
        contextControl.ordinal2.bitfields.load_per_context_state = 1;
        contextControl.ordinal2.bitfields.load_cs_sh_regs        = 1;
        contextControl.ordinal2.bitfields.load_gfx_sh_regs       = 1;
        contextControl.ordinal2.bitfields.load_global_uconfig    = 1;

        // If state shadowing is enabled, then we enable shadowing and loading for all register types,
        // because if preempted the GPU state needs to be properly restored when the Queue resumes.
        // (Config registers are exempted because we don't write config registers in PAL.)
        contextControl.ordinal3.bitfields.update_shadow_enables    = 1;
        contextControl.ordinal3.bitfields.shadow_per_context_state = 1;
        contextControl.ordinal3.bitfields.shadow_cs_sh_regs        = 1;
        contextControl.ordinal3.bitfields.shadow_gfx_sh_regs       = 1;
        contextControl.ordinal3.bitfields.shadow_global_config     = 1;
        contextControl.ordinal3.bitfields.shadow_global_uconfig    = 1;
    }

    return contextControl;
}

// =====================================================================================================================
// Gfx12 helper function for patching a pipeline's shader internal SRD table.
void Device::PatchPipelineInternalSrdTable(
    void*       pDstSrdTable,   // Out: Patched SRD table in mapped GPU memory
    const void* pSrcSrdTable,   // In: Unpatched SRD table from ELF binary
    size_t      tableBytes,
    gpusize     dataGpuVirtAddr
    ) const
{
    // See Pipeline::PerformRelocationsAndUploadToGpuMemory() for more information.

    auto*const pSrcSrd = static_cast<const sq_buf_rsrc_t*>(pSrcSrdTable);
    auto*const pDstSrd = static_cast<sq_buf_rsrc_t*>(pDstSrdTable);

    for (uint32 i = 0; i < (tableBytes / sizeof(sq_buf_rsrc_t)); ++i)
    {
        sq_buf_rsrc_t srd = pSrcSrd[i];

        srd.base_address = srd.base_address + dataGpuVirtAddr;

        // Note: The entire unpatched SRD table has already been copied to GPU memory wholesale.  We just need to
        // modify the first quadword of the SRD to patch the addresses.
        memcpy((pDstSrd + i), &srd, sizeof(uint64));
    }
}

// =====================================================================================================================
gpusize Device::PrimBufferTotalMemSize() const
{
    const auto& chipProps = m_pParent->ChipProperties().gfx9;
    const auto& settings  = Settings();

    const uint32 numPrims = settings.primBufferRingSizing;
    PAL_ASSERT(numPrims > 0);
    PAL_ASSERT((numPrims & 1) == 0);
    PAL_ASSERT(numPrims <= MaxGePrimRingPrims);

    constexpr uint32 RingUnits = 32;

    // The equation for determining the total size allocation for the primitive buffer ring is:
    //     numPrims * 4bytes * numSes^2

    return Pow2Align(numPrims * sizeof(uint32) * chipProps.numShaderEngines * chipProps.numShaderEngines,
                     RingUnits);
}

// =====================================================================================================================
gpusize Device::PosBufferTotalMemSize() const
{
    const auto& chipProps = m_pParent->ChipProperties().gfx9;
    const auto& settings  = Settings();

    const uint32 numPositions = settings.posBufferRingSizing;
    PAL_ASSERT(numPositions > 0);
    PAL_ASSERT((numPositions & 1) == 0);
    PAL_ASSERT(numPositions <= MaxGePosRingPos);

    constexpr uint32 RingUnits = 32;

    // The equation for determining the total size allocation for the position buffer ring is:
    //     numPositions * 16bytes * numSes^2

    return Pow2Align(numPositions * sizeof(uint32) * 4 * chipProps.numShaderEngines * chipProps.numShaderEngines,
                     RingUnits);
}

// =====================================================================================================================
uint32 Device::GeomExportBufferMemSize(
    gpusize totalMemSize
    ) const
{
    const auto& chipProps = m_pParent->ChipProperties().gfx9;

    const uint32 memSize = (totalMemSize / (chipProps.numShaderEngines * chipProps.numShaderEngines)) >>
                           GeometryExportRingMemSizeShift;

    return RoundDownToMultiple(memSize, 2u);
}

// =====================================================================================================================
Result Device::AllocateVertexAttributesMem(
    bool isTmz)
{
    Util::MutexAuto lock(&m_vertexOutputMutex);
    Result result = Result::Success;

    if (m_vertexAttributesMem[isTmz].IsBound() == false)
    {
        // Create the attributes through memory ring buffer
        const auto&             chipProps = m_pParent->ChipProperties().gfx9;
        const Gfx12PalSettings& settings  = Settings();

        PAL_ASSERT(settings.gfx12VertexAttributesRingBufferSizePerSe != 0);

        GpuMemoryCreateInfo createInfo = { };
        createInfo.size                = settings.gfx12VertexAttributesRingBufferSizePerSe * chipProps.numShaderEngines;
        createInfo.alignment           = VertexAttributeRingAlignmentBytes;
        createInfo.priority            = GpuMemPriority::Normal;
        createInfo.heapAccess          = GpuHeapAccessCpuNoAccess;
        createInfo.flags.tmzProtected  = isTmz;

        GpuMemoryInternalCreateInfo internalInfo = {};
        internalInfo.flags.alwaysResident = 1;

        GpuMemory* pGpuMemory = nullptr;
        gpusize    memOffset  = 0;

        result = Parent()->MemMgr()->AllocateGpuMem(createInfo, internalInfo, 0, &pGpuMemory, &memOffset);
        if (result == Result::Success)
        {
            m_vertexAttributesMem[isTmz].Update(pGpuMemory, memOffset);
        }
    }

    return result;
}

// =====================================================================================================================
Result Device::AllocatePrimBufferMem(
    bool isTmz)
{
    Util::MutexAuto lock(&m_vertexOutputMutex);
    Result result = Result::Success;

    if (m_primBufferMem[isTmz].IsBound() == false)
    {
        // Create the primitive buffer ring.
        // The equation for determining the total size allocation for the primitive buffer ring is:
        //     numPrims * 4bytes * numSes^2
        GpuMemoryCreateInfo createInfo = { };
        createInfo.size                = PrimBufferTotalMemSize();
        createInfo.alignment           = VertexAttributeRingAlignmentBytes;
        createInfo.priority            = GpuMemPriority::Normal;
        createInfo.heapAccess          = GpuHeapAccessCpuNoAccess;
        createInfo.flags.tmzProtected  = isTmz;

        GpuMemory* pGpuMemory = nullptr;
        gpusize    memOffset  = 0;

        GpuMemoryInternalCreateInfo internalInfo = {};
        internalInfo.flags.alwaysResident = 1;

        if (result == Result::Success)
        {
            result = Parent()->MemMgr()->AllocateGpuMem(createInfo, internalInfo, 0, &pGpuMemory, &memOffset);
        }

        if (result == Result::Success)
        {
            m_primBufferMem[isTmz].Update(pGpuMemory, memOffset);
        }
    }

    return result;
}

// =====================================================================================================================
Result Device::AllocatePosBufferMem(
    bool isTmz)
{
    Util::MutexAuto lock(&m_vertexOutputMutex);
    Result result = Result::Success;

    if (m_posBufferMem[isTmz].IsBound() == false)
    {
        // Create the position buffer ring.
        // The equation for determining the total size allocation for the position buffer ring is:
        //     numPositions * 16bytes * numSes^2
        GpuMemoryCreateInfo createInfo = { };
        createInfo.size                = PosBufferTotalMemSize();
        createInfo.alignment           = VertexAttributeRingAlignmentBytes;
        createInfo.priority            = GpuMemPriority::Normal;
        createInfo.heapAccess          = GpuHeapAccessCpuNoAccess;
        createInfo.flags.tmzProtected  = isTmz;

        GpuMemory* pGpuMemory = nullptr;
        gpusize    memOffset  = 0;

        GpuMemoryInternalCreateInfo internalInfo = {};
        internalInfo.flags.alwaysResident = 1;

        if (result == Result::Success)
        {
            result = Parent()->MemMgr()->AllocateGpuMem(createInfo, internalInfo, 0, &pGpuMemory, &memOffset);
        }

        if (result == Result::Success)
        {
            m_posBufferMem[isTmz].Update(pGpuMemory, memOffset);
        }
    }

    return result;
}

// =====================================================================================================================
// Allocate the ring buffer for attributes through memory, primitive, and position buffers.
Result Device::AllocateVertexOutputMem()
{
    Result result = AllocateVertexAttributesMem(false);

    if (result == Result::Success)
    {
        result = AllocatePrimBufferMem(false);
    }

    if (result == Result::Success)
    {
        result = AllocatePosBufferMem(false);
    }

    return result;
}

// =====================================================================================================================
// Returns the image plane that corresponds to the supplied base address.
static uint32 DecodeImageViewSrdPlane(
    const Pal::Image& image,
    gpusize           srdBaseAddr,
    uint32            slice)
{
    uint32      plane           = 0;
    const auto& imageCreateInfo = image.GetImageCreateInfo();

    if (Formats::IsYuvPlanar(imageCreateInfo.swizzledFormat.format))
    {
        const auto*  pGfxImage = image.GetGfxImage();
        const auto&  imageInfo = image.GetImageInfo();

        // For Planar YUV, loop through each plane of the slice and compare the address with SRD to determine which
        // subresrouce this SRD represents.
        for (uint32 planeIdx = 0; (planeIdx < imageInfo.numPlanes); ++planeIdx)
        {
            const gpusize  planeBaseAddr = pGfxImage->GetPlaneBaseAddr(planeIdx, slice);
            const auto     subResAddr    = Get256BAddrLo(planeBaseAddr);

            if (srdBaseAddr == subResAddr)
            {
                plane      = planeIdx;
                break;
            }
        }
    }

    return plane;
}

// =====================================================================================================================
void PAL_STDCALL Device::DecodeBufferViewSrd(
    const IDevice*  pDevice,
    const void*     pBufferViewSrd,
    BufferViewInfo* pViewInfo)
{
    const Pal::Device* pPalDevice = static_cast<const Pal::Device*>(pDevice);
    const auto&        srd        = *(static_cast<const sq_buf_rsrc_t*>(pBufferViewSrd));

    // Verify that we have a buffer view SRD.
    PAL_ASSERT(srd.type == SQ_RSRC_BUF);

    // Reconstruct the buffer view info struct.
    pViewInfo->gpuAddr = static_cast<gpusize>(srd.base_address);
    pViewInfo->range   = srd.num_records;
    pViewInfo->stride  = srd.stride;

    if (pViewInfo->stride > 1)
    {
        pViewInfo->range *= pViewInfo->stride;
    }

    pViewInfo->swizzledFormat.format    = FmtFromHwBufFmt(static_cast<Chip::BUF_FMT>(srd.format));
    pViewInfo->swizzledFormat.swizzle.r = ChannelSwizzleFromHwSwizzle(static_cast<Chip::SQ_SEL_XYZW01>(srd.dst_sel_x));
    pViewInfo->swizzledFormat.swizzle.g = ChannelSwizzleFromHwSwizzle(static_cast<Chip::SQ_SEL_XYZW01>(srd.dst_sel_y));
    pViewInfo->swizzledFormat.swizzle.b = ChannelSwizzleFromHwSwizzle(static_cast<Chip::SQ_SEL_XYZW01>(srd.dst_sel_z));
    pViewInfo->swizzledFormat.swizzle.a = ChannelSwizzleFromHwSwizzle(static_cast<Chip::SQ_SEL_XYZW01>(srd.dst_sel_w));

    // Verify that we have a valid format.
    PAL_ASSERT(pViewInfo->swizzledFormat.format != ChNumFormat::Undefined);
}

// =====================================================================================================================
// GFX12-specific function for extracting the subresource range and format information from the supplied SRD and image
void PAL_STDCALL Device::DecodeImageViewSrd(
    const IDevice*   pDevice,
    const IImage*    pImage,
    const void*      pImageViewSrd,
    DecodedImageSrd* pDecodedInfo)
{
    const Pal::Image&  dstImage        = *static_cast<const Pal::Image*>(pImage);
    const Pal::Device* pPalDevice      = static_cast<const Pal::Device*>(pDevice);
    SubresRange*       pSubresRange    = &pDecodedInfo->subresRange;
    SwizzledFormat*    pSwizzledFormat = &pDecodedInfo->swizzledFormat;

    const sq_img_rsrc_t&   srd        = *static_cast<const sq_img_rsrc_t*>(pImageViewSrd);
    const ImageCreateInfo& createInfo = dstImage.GetImageCreateInfo();

    // Verify that we have an image view SRD.
    PAL_ASSERT((srd.type >= SQ_RSRC_IMG_1D) && (srd.type <= SQ_RSRC_IMG_2D_MSAA_ARRAY));

    const gpusize srdBaseAddr = srd.base_address;

    pSwizzledFormat->format    = FmtFromHwImgFmt(static_cast<Chip::IMG_FMT>(srd.format));
    pSwizzledFormat->swizzle.r = ChannelSwizzleFromHwSwizzle(static_cast<Chip::SQ_SEL_XYZW01>(srd.dst_sel_x));
    pSwizzledFormat->swizzle.g = ChannelSwizzleFromHwSwizzle(static_cast<Chip::SQ_SEL_XYZW01>(srd.dst_sel_y));
    pSwizzledFormat->swizzle.b = ChannelSwizzleFromHwSwizzle(static_cast<Chip::SQ_SEL_XYZW01>(srd.dst_sel_z));
    pSwizzledFormat->swizzle.a = ChannelSwizzleFromHwSwizzle(static_cast<Chip::SQ_SEL_XYZW01>(srd.dst_sel_w));

    // Verify that we have a valid format.
    PAL_ASSERT(pSwizzledFormat->format != ChNumFormat::Undefined);

    // Next, recover the original subresource range. We can't recover the exact range in all cases so we must assume
    // that it's looking at the color plane and that it's not block compressed.
    PAL_ASSERT(Formats::IsBlockCompressed(pSwizzledFormat->format) == false);

    const uint32 depth     = srd.depth;
    const uint32 baseArray = srd.base_array;

    // The PAL interface can not individually address the slices of a 3D resource.  "numSlices==1" is assumed to
    // mean all of them and we have to start from the first slice.
    if (createInfo.imageType == ImageType::Tex3d)
    {
        pSubresRange->numSlices              = 1;
        pSubresRange->startSubres.arraySlice = 0;

        // uav3d (previously known as array_pitch in older hardware)
        //     For 3D, bit 0 indicates SRV or UAV:
        //     0: SRV (base_array ignored, depth w.r.t. base map)
        //     1: UAV (base_array and depth are first and last layer in view, and w.r.t. mip level specified)
        const bool is3dUav = (srd.uav3d & 1) != 0;
        if (is3dUav)
        {
            const uint32 viewZBegin = baseArray;
            const uint32 viewZEnd   = depth + 1;
            const uint32 viewZCount = viewZEnd - viewZBegin;

            pDecodedInfo->zRange = { int32(viewZBegin), viewZCount };
        }
        else
        {
            const uint32 d = dstImage.SubresourceInfo(pSubresRange->startSubres)->extentTexels.depth;
            pDecodedInfo->zRange = { 0, d };
        }
    }
    else
    {
        pDecodedInfo->zRange = { 0, 1 };

        const bool   isYuvPlanar = Formats::IsYuvPlanar(createInfo.swizzledFormat.format);
        // Becuase of the way the HW needs to index YuvPlanar images, BASE_ARRAY is forced to 0, even if we
        // aren't indexing slice 0.  Additionally, numSlices must be 1 for any operation other than direct image loads.
        // When creating SRD, DEPTH == subresRange.startSubres.arraySlice + subresRange.numSlices - 1;
        // Since we know numSlices == 1, startSubres.arraySlice == DEPTH.
        if (isYuvPlanar)
        {
            PAL_ASSERT(baseArray == 0);
            pSubresRange->numSlices              = 1;
            pSubresRange->startSubres.arraySlice = depth;
        }
        else
        {
            pSubresRange->numSlices              = depth - baseArray + 1;
            pSubresRange->startSubres.arraySlice = baseArray;
        }
    }
    pSubresRange->startSubres.plane = DecodeImageViewSrdPlane(dstImage,
                                                              srdBaseAddr,
                                                              pSubresRange->startSubres.arraySlice);
    pSubresRange->numPlanes = 1;

    if (srd.type >= SQ_RSRC_IMG_2D_MSAA)
    {
        // MSAA textures cannot be mipmapped; the BASE_LEVEL and LAST_LEVEL fields indicate the texture's sample count.
        pSubresRange->startSubres.mipLevel = 0;
        pSubresRange->numMips              = 1;
    }
    else
    {
        pSubresRange->startSubres.mipLevel = srd.base_level;
        pSubresRange->numMips              = srd.last_level - srd.base_level + 1;
    }

    if ((pSubresRange->startSubres.mipLevel + pSubresRange->numMips) > createInfo.mipLevels)
    {
        // The only way that we should have an SRD that references non-existent mip-levels is with PRT+ residency
        // maps.  The Microsoft spec creates residency maps with the same number of mip levels as the parent image
        // which is unnecessary in our implementation.  Doing so wastes memory, so DX12 created only a single mip
        // level residency map (i.e, ignored the API request).
        //
        // Unfortunately, the SRD created here went through DX12's "CreateSamplerFeedbackUnorderedAccessView" entry
        // point (which in turn went into PAL's "Gfx10UpdateLinkedResourceViewSrd" function), so we have a hybrid SRD
        // here that references both the map image and the parent image and thus has the "wrong" number of mip levels.
        //
        // Fix up the SRD here to reference the "correct" number of mip levels owned by the image.
        PAL_ASSERT(createInfo.prtPlus.mapType == PrtMapType::Residency);

        pSubresRange->startSubres.mipLevel = 0;
        pSubresRange->numMips              = 1;
    }

    FixupDecodedSrdFormat(createInfo.swizzledFormat, pSwizzledFormat);
}

// =====================================================================================================================
size_t Device::GetPerfExperimentSize(
    const PerfExperimentCreateInfo& createInfo,
    Result*                         pResult
    ) const
{
    if (pResult != nullptr)
    {
        (*pResult) = Result::Success;
    }

    return sizeof(PerfExperiment);
}

// =====================================================================================================================
Result Device::CreatePerfExperiment(
    const PerfExperimentCreateInfo& createInfo,
    void*                           pPlacementAddr,
    IPerfExperiment**               ppPerfExperiment
    ) const
{
    PerfExperiment* pPerfExperiment = PAL_PLACEMENT_NEW(pPlacementAddr) PerfExperiment(this, createInfo);
    Result          result          = pPerfExperiment->Init();

    if (result == Result::Success)
    {
        (*ppPerfExperiment) = pPerfExperiment;
    }
    else
    {
        pPerfExperiment->Destroy();
    }

    return result;
}

// =====================================================================================================================
// Helper function to get final compression mode from view compression mode and image compression mode
const CompressionMode Device::GetImageViewCompressionMode(
    CompressionMode  viewCompressionMode,
    CompressionMode  imageCompressionMode,
    const GpuMemory* pGpuMem
    ) const
{
    CompressionMode finalCompressionMode = CompressionMode::ReadBypassWriteDisable;
    if (pGpuMem->MaybeCompressed())
    {
        switch (imageCompressionMode)
        {
        case CompressionMode::Default:
        case CompressionMode::ReadEnableWriteEnable:
            switch (viewCompressionMode)
            {
            case CompressionMode::Default:
            case CompressionMode::ReadEnableWriteEnable:
            case CompressionMode::ReadEnableWriteDisable:
                finalCompressionMode = viewCompressionMode;
                break;
            case CompressionMode::ReadBypassWriteDisable:
                PAL_ASSERT_ALWAYS(); // Should not use ReadBypassWriteDisable in this case
                finalCompressionMode = CompressionMode::ReadEnableWriteDisable;
                break;
            default:
                PAL_NEVER_CALLED();
            }
            break;
        case CompressionMode::ReadEnableWriteDisable:
            finalCompressionMode = CompressionMode::ReadEnableWriteDisable;
            break;
        case CompressionMode::ReadBypassWriteDisable:
            finalCompressionMode = CompressionMode::ReadBypassWriteDisable;
            break;
        default:
            PAL_NEVER_CALLED();
        }
    }
    if ((finalCompressionMode == CompressionMode::ReadBypassWriteDisable) &&
        (Settings().enableCompressionReadBypass == false))
    {
        finalCompressionMode = CompressionMode::ReadEnableWriteDisable;
    }
    return finalCompressionMode;
}

// =====================================================================================================================
bool Device::CompressGpuMemory(
    const GpuMemoryCreateInfo& gpuMemCreateInfo,
    bool                       isCpuVisible,
    bool                       isClient
    ) const
{
    bool result = false;
    switch (Settings().gpuMemoryCompression)
    {
    case Default:
        switch (gpuMemCreateInfo.compression)
        {
        case TriState::Default:
            result = DefaultGpuMemoryCompression(gpuMemCreateInfo, Settings(), isCpuVisible, isClient);
            break;
        case TriState::Enable:
            result = true;
            break;
        case TriState::Disable:
            result = false;
            break;
        default:
            PAL_NEVER_CALLED();
        }
        break;
    case ForceDefault:
        result = DefaultGpuMemoryCompression(gpuMemCreateInfo, Settings(), isCpuVisible, isClient);
        break;
    case ForceEnable:
        result = true;
        break;
    case ForceDisable:
        result = false;
        break;
    default:
        PAL_NEVER_CALLED();
    }

    return result;
}

// =====================================================================================================================
ClearMethod  Device::GetDefaultSlowClearMethod(
    const ImageCreateInfo& createInfo,
    const SwizzledFormat&  clearFormat
    ) const
{
    uint32 texelScale = 1;
    RpmUtil::GetRawFormat(clearFormat.format, &texelScale, nullptr);

    // Force clears of scaled formats to the compute engine
    return (texelScale > 1) ? ClearMethod::NormalCompute : ClearMethod::NormalGraphics;
}

// =====================================================================================================================
// Calculate the value for the various INST_PREF_SIZE fields.  Default behavior is to prefetch the entire shader.
uint32 Device::GetShaderPrefetchSize(
    gpusize  shaderSizeBytes
    ) const
{
    // Compute maximum prefetch size (in register units) based on available bits in register.
    // All the shader stages should have the same number of bits.
    constexpr gpusize MaxPrefetchSize = Gfx12::SPI_SHADER_PGM_RSRC4_PS__INST_PREF_SIZE_MASK >>
                                        Gfx12::SPI_SHADER_PGM_RSRC4_PS__INST_PREF_SIZE__SHIFT;
    static_assert(MaxPrefetchSize == (Gfx12::SPI_SHADER_PGM_RSRC4_GS__INST_PREF_SIZE_MASK >>
                                      Gfx12::SPI_SHADER_PGM_RSRC4_GS__INST_PREF_SIZE__SHIFT));
    static_assert(MaxPrefetchSize == (Gfx12::SPI_SHADER_PGM_RSRC4_HS__INST_PREF_SIZE_MASK >>
                                      Gfx12::SPI_SHADER_PGM_RSRC4_HS__INST_PREF_SIZE__SHIFT));
    static_assert(MaxPrefetchSize == (Gfx12::WGS_COMPUTE_PGM_RSRC3__INST_PREF_SIZE_MASK >>
                                      Gfx12::WGS_COMPUTE_PGM_RSRC3__INST_PREF_SIZE__SHIFT));

    constexpr gpusize CachelineSizeBytes = 128;

    // Don't prefetch more bytes than the panel setting allows
    const gpusize prefetchSizeBytes = Min(shaderSizeBytes,
                                          static_cast<gpusize>(Settings().shaderPrefetchSizeBytes));

    // Align to the nearest multiple of a cacheline
    const gpusize prefetchSizeAligned = Pow2Align(prefetchSizeBytes, CachelineSizeBytes);

    // Return in terms of register units (cachelines).  Don't allow a value larger than the register supports.
    const gpusize cacheLines = Min(MaxPrefetchSize, (prefetchSizeAligned / CachelineSizeBytes));
    return static_cast<uint32>(cacheLines);
}

} // Gfx12
} // Pal
