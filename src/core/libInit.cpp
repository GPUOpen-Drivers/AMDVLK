/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2014-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "core/platform.h"
#include "core/os/nullDevice/ndPlatform.h"
#include "core/os/nullDevice/ndDevice.h"

#if PAL_AMDGPU_BUILD
#include "core/os/amdgpu/amdgpuHeaders.h"
#endif

#include "core/layers/dbgOverlay/dbgOverlayPlatform.h"
#include "core/layers/gpuProfiler/gpuProfilerPlatform.h"
#include "core/layers/crashAnalysis/crashAnalysisPlatform.h"

#if PAL_DEVELOPER_BUILD
#include "core/layers/gpuDebug/gpuDebugPlatform.h"
#endif
#if PAL_DEVELOPER_BUILD
#include "core/layers/cmdBufferLogger/cmdBufferLoggerPlatform.h"
#endif
#if PAL_DEVELOPER_BUILD
#include "core/layers/interfaceLogger/interfaceLoggerPlatform.h"
#endif
#if PAL_DEVELOPER_BUILD
#include "core/layers/pm4Instrumentor/pm4InstrumentorPlatform.h"
#endif

#include "addrinterface.h"
#include "vaminterface.h"

#include "palInlineFuncs.h"

namespace Pal
{

// Static asserts to check that the client's AddrLib and VAM libraries are compatible with PAL.  If one of these
// asserts trips, then PAL will likely need an update in order to support a breaking change in one of these library's
// interfaces.

static_assert((ADDRLIB_VERSION_MAJOR == 9) || (ADDRLIB_VERSION_MAJOR == 10), "Unexpected AddrLib major version.");
static_assert(VAM_VERSION_MAJOR == 1, "Unexpected VAM major version.");

// Static asserts to ensure clients have defined PAL_CLIENT_INTERFACE_MAJOR_VERSION and that it falls in the supported
// range.
#ifndef PAL_CLIENT_INTERFACE_MAJOR_VERSION
    static_assert(false, "Client must define PAL_CLIENT_INTERFACE_MAJOR_VERSION.");
#else
    static_assert((PAL_CLIENT_INTERFACE_MAJOR_VERSION >= PAL_MINIMUM_INTERFACE_MAJOR_VERSION) &&
                  (PAL_CLIENT_INTERFACE_MAJOR_VERSION <= PAL_INTERFACE_MAJOR_VERSION),
                  "The specified PAL_CLIENT_INTERFACE_MAJOR_VERSION is not supported.");
#endif

// Static asserts to ensure clients have defined a supported GPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION
#ifndef GPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION
    static_assert(false, "Client must define GPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION.");
#else
    static_assert((GPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION >= PAL_MINIMUM_GPUOPEN_INTERFACE_MAJOR_VERSION),
                  "The specified GPUOPEN_CLIENT_INTERFACE_MAJOR_VERSION is not supported.");
#endif

constexpr uint32 GfxEngineGfx9 = CIASICIDGFXENGINE_ARCTICISLAND;

// Identification table for all GPUs that are supported
constexpr GpuInfo GpuInfoLookupTable[] =
{
    { AsicRevision::Navi10,     NullGpuId::Navi10,     GfxIpLevel::GfxIp10_1, FAMILY_NV,  NV_NAVI10_P, PRID_NV_NAVI10_00, GfxEngineGfx9, DEVICE_ID_NV_NAVI10_P_7310, "NAVI10:gfx1010" },
    { AsicRevision::Navi12,     NullGpuId::Navi12,     GfxIpLevel::GfxIp10_1, FAMILY_NV,  NV_NAVI12_P, PRID_NV_NAVI12_00, GfxEngineGfx9, DEVICE_ID_NV_NAVI12_P_7360, "NAVI12:gfx1011" },
    { AsicRevision::Navi14,     NullGpuId::Navi14,     GfxIpLevel::GfxIp10_1, FAMILY_NV,  NV_NAVI14_M, PRID_NV_NAVI14_00, GfxEngineGfx9, DEVICE_ID_NV_NAVI14_M_7340, "NAVI14:gfx1012" },
    { AsicRevision::Navi21,     NullGpuId::Navi21,     GfxIpLevel::GfxIp10_3, FAMILY_NV,  NV_NAVI21_P, PRID_NV_NAVI10_00, GfxEngineGfx9, DEVICE_ID_NV_NAVI10_P_7310, "NAVI21:gfx1030" },
    { AsicRevision::Navi22,     NullGpuId::Navi22,     GfxIpLevel::GfxIp10_3, FAMILY_NV,  NV_NAVI22_P, PRID_NV_NAVI10_00, GfxEngineGfx9, DEVICE_ID_NV_NAVI10_P_7310, "NAVI22:gfx1031" },
    { AsicRevision::Navi23,     NullGpuId::Navi23,     GfxIpLevel::GfxIp10_3, FAMILY_NV,  NV_NAVI23_P, PRID_NV_NAVI10_00, GfxEngineGfx9, DEVICE_ID_NV_NAVI10_P_7310, "NAVI23:gfx1032" },
    { AsicRevision::Navi24,     NullGpuId::Navi24,     GfxIpLevel::GfxIp10_3, FAMILY_NV,  NV_NAVI24_P, PRID_NV_NAVI10_00, GfxEngineGfx9, DEVICE_ID_NV_NAVI10_P_7310, "NAVI24:gfx1034" },
    { AsicRevision::Rembrandt,  NullGpuId::Rembrandt,  GfxIpLevel::GfxIp10_3, FAMILY_RMB, REMBRANDT_P, PRID_RMB_00,        GfxEngineGfx9, DEVICE_ID_RMB_1681,          "REMBRANDT:gfx1035" },
    { AsicRevision::Raphael,    NullGpuId::Raphael,    GfxIpLevel::GfxIp10_3, FAMILY_RPL, RAPHAEL_P,   PRID_RPL_00,        GfxEngineGfx9, DEVICE_ID_RPL_164E,          "RAPHAEL:gfx1036" },
    { AsicRevision::Navi31,     NullGpuId::Navi31,     GfxIpLevel::GfxIp11_0, FAMILY_NV3, NAVI31_P,    PRID_NV3_NAVI31_00, GfxEngineGfx9, DEVICE_ID_NV3_NAVI31_P_73BF, "NAVI31:gfx1100" },
    { AsicRevision::Navi32,     NullGpuId::Navi32,     GfxIpLevel::GfxIp11_0, FAMILY_NV3, NAVI32_P,    PRID_NV3_NAVI32_00, GfxEngineGfx9, DEVICE_ID_NV3_NAVI32_P_73DF, "NAVI32:gfx1101" },
    { AsicRevision::Navi33,     NullGpuId::Navi33,     GfxIpLevel::GfxIp11_0, FAMILY_NV3, NAVI33_P,    PRID_NV3_NAVI33_00, GfxEngineGfx9, DEVICE_ID_NV3_NAVI33_P_73F0, "NAVI33:gfx1102" },
    { AsicRevision::Phoenix1,   NullGpuId::Phoenix1,   GfxIpLevel::GfxIp11_0, FAMILY_PHX, PHOENIX1_P,  PRID_PHX_00,        GfxEngineGfx9, DEVICE_ID_PHX1_15BF,         "PHOENIX1:gfx1103" },
    { AsicRevision::Phoenix2,   NullGpuId::Phoenix2,   GfxIpLevel::GfxIp11_0, FAMILY_PHX, PHOENIX2_P,  PRID_PHX_00,        GfxEngineGfx9, DEVICE_ID_PHX2_15C8,         "PHOENIX2:gfx1103" },
    { AsicRevision::Strix1,     NullGpuId::Strix1,     GfxIpLevel::GfxIp11_5, FAMILY_STX, STRIX1_P,    PRID_STX_STRIX1_00, GfxEngineGfx9, DEVICE_ID_STX1_150E,         "STRIX1:gfx1150" },
#if PAL_BUILD_STRIX_HALO
    { AsicRevision::StrixHalo,  NullGpuId::StrixHalo, GfxIpLevel::GfxIp11_5, FAMILY_STX, STRIX_HALO_P, PRID_STX_STRIX_HALO_00, GfxEngineGfx9, DEVICE_ID_STXH_1586, "STRIX_HALO:gfx1151" },
#endif
#if PAL_BUILD_NAVI48
    { AsicRevision::Navi48,     NullGpuId::Navi48,    GfxIpLevel::GfxIp12,   FAMILY_NV4, NAVI48_P,     PRID_NV_NAVI48_00,      GfxEngineGfx9, DEVICE_ID_NAVI48_94, "NAVI48:gfx1201" },
#endif
#if PAL_BUILD_NAVI44
    { AsicRevision::Navi44,     NullGpuId::Navi44,    GfxIpLevel::GfxIp12,   FAMILY_NV4, NAVI44_P,     PRID_NV_NAVI44_00,      GfxEngineGfx9, DEVICE_ID_NAVI44_94, "NAVI44:gfx1200" },
#endif
};

// =====================================================================================================================
// Returns the size necessary to initialize a PAL Platform object.
size_t PAL_STDCALL GetPlatformSize()
{
    // The switch between the "real" and "null" device is determined at run-time.  We would never have both active
    // simultaneously.
#if PAL_BUILD_NULL_DEVICE
    size_t platformSize = Util::Max(Platform::GetSize(), NullDevice::Platform::GetSize());
#else
    size_t platformSize = Platform::GetSize();
#endif

    // We need to always assume that the all layers can be enabled.  Unfortunately, at this point, we have not yet read
    // the settings for the GPUs present so we do not know which layers will actually be enabled.

    platformSize += sizeof(DbgOverlay::Platform);
    platformSize += sizeof(GpuProfiler::Platform);
    platformSize += sizeof(CrashAnalysis::Platform);

#if PAL_DEVELOPER_BUILD
    platformSize += sizeof(InterfaceLogger::Platform);
#endif
#if PAL_DEVELOPER_BUILD
    platformSize += sizeof(GpuDebug::Platform);
#endif
#if PAL_DEVELOPER_BUILD
    platformSize += sizeof(CmdBufferLogger::Platform);
#endif
#if PAL_DEVELOPER_BUILD
    platformSize += sizeof(Pm4Instrumentor::Platform);
#endif

    return platformSize;
}

// =====================================================================================================================
// Initializes the PAL Platform object. This is the first call made by the client on startup, typically during process
// attach. See the public interface documentation for more detail.
Result PAL_STDCALL CreatePlatform(
    const PlatformCreateInfo&   createInfo,
    void*                       pPlacementAddr,
    IPlatform**                 ppPlatform)
{
    Result result = Result::Success;
    Util::AllocCallbacks allocCb = {};

    if ((createInfo.pAllocCb != nullptr) &&
        ((createInfo.pAllocCb->pfnAlloc == nullptr) || (createInfo.pAllocCb->pfnFree == nullptr)))
    {
        // If the client is specifying allocation callbacks, they must define both an alloc and free function pointer.
        result = Result::ErrorInvalidPointer;
    }
    else if ((pPlacementAddr == nullptr) || (createInfo.pSettingsPath == nullptr))
    {
        // The client must specify memory.
        result = Result::ErrorInvalidPointer;
    }
    else if (createInfo.pAllocCb == nullptr)
    {
        GetDefaultAllocCb(&allocCb);
    }
    else
    {
        allocCb = *createInfo.pAllocCb;
    }

    pPlacementAddr = Util::VoidPtrInc(pPlacementAddr, sizeof(DbgOverlay::Platform));
    pPlacementAddr = Util::VoidPtrInc(pPlacementAddr, sizeof(GpuProfiler::Platform));
    pPlacementAddr = Util::VoidPtrInc(pPlacementAddr, sizeof(CrashAnalysis::Platform));

    // NOTE: If a specific layer is being built we must always create a Platform decorator for that layer.
    //       This avoids a rather difficult issue where we need to place the IPlatform the client uses at the beginning
    //       of the memory they allocate (or we could have an issue when they go to free that memory). It is easier to
    //       just create the Platform decorator for every layer and make it the responsibility of the layer to
    //       understand when it is enabled or disabled.
#if PAL_DEVELOPER_BUILD
    pPlacementAddr = Util::VoidPtrInc(pPlacementAddr, sizeof(InterfaceLogger::Platform));
#endif
#if PAL_DEVELOPER_BUILD
    pPlacementAddr = Util::VoidPtrInc(pPlacementAddr, sizeof(GpuDebug::Platform));
#endif
#if PAL_DEVELOPER_BUILD
    pPlacementAddr = Util::VoidPtrInc(pPlacementAddr, sizeof(CmdBufferLogger::Platform));
#endif
#if PAL_DEVELOPER_BUILD
    pPlacementAddr = Util::VoidPtrInc(pPlacementAddr, sizeof(Pm4Instrumentor::Platform));
#endif

    Platform* pCorePlatform = nullptr;

    if (result == Result::Success)
    {
        result = Platform::Create(createInfo, allocCb, pPlacementAddr, &pCorePlatform);
    }

    IPlatform* pCurPlatform = pCorePlatform;

    if (result == Result::Success)
    {
        pPlacementAddr = Util::VoidPtrDec(pPlacementAddr, sizeof(GpuProfiler::Platform));
        pCurPlatform->SetClientData(pPlacementAddr);

        result = GpuProfiler::Platform::Create(createInfo,
                                               allocCb,
                                               pCurPlatform,
                                               pPlacementAddr,
                                               &pCurPlatform);
    }

    if (result == Result::Success)
    {
        pPlacementAddr = Util::VoidPtrDec(pPlacementAddr, sizeof(DbgOverlay::Platform));
        pCurPlatform->SetClientData(pPlacementAddr);

        result = DbgOverlay::Platform::Create(createInfo,
                                              allocCb,
                                              pCurPlatform,
                                              pCorePlatform->PlatformSettings().debugOverlayEnabled,
                                              pPlacementAddr,
                                              &pCurPlatform);
    }

#if PAL_DEVELOPER_BUILD
    if (result == Result::Success)
    {
        pPlacementAddr = Util::VoidPtrDec(pPlacementAddr, sizeof(Pm4Instrumentor::Platform));
        pCurPlatform->SetClientData(pPlacementAddr);

        result = Pm4Instrumentor::Platform::Create(createInfo,
                                                   allocCb,
                                                   pCurPlatform,
                                                   pCorePlatform->PlatformSettings().pm4InstrumentorEnabled,
                                                   pPlacementAddr,
                                                   &pCurPlatform);
    }
#endif

#if PAL_DEVELOPER_BUILD
    if (result == Result::Success)
    {
        pPlacementAddr = Util::VoidPtrDec(pPlacementAddr, sizeof(CmdBufferLogger::Platform));
        pCurPlatform->SetClientData(pPlacementAddr);

        result = CmdBufferLogger::Platform::Create(createInfo,
                                                   allocCb,
                                                   pCurPlatform,
                                                   pCorePlatform->PlatformSettings().cmdBufferLoggerEnabled,
                                                   pPlacementAddr,
                                                   &pCurPlatform);
    }
#endif

#if PAL_DEVELOPER_BUILD
    if (result == Result::Success)
    {
        pPlacementAddr = Util::VoidPtrDec(pPlacementAddr, sizeof(GpuDebug::Platform));
        pCurPlatform->SetClientData(pPlacementAddr);

        result = GpuDebug::Platform::Create(createInfo,
                                            allocCb,
                                            pCurPlatform,
                                            pCorePlatform->PlatformSettings().gpuDebugEnabled,
                                            pPlacementAddr,
                                            &pCurPlatform);
    }
#endif

#if PAL_DEVELOPER_BUILD
    if (result == Result::Success)
    {
        pPlacementAddr = Util::VoidPtrDec(pPlacementAddr, sizeof(InterfaceLogger::Platform));
        pCurPlatform->SetClientData(pPlacementAddr);

        result = InterfaceLogger::Platform::Create(createInfo,
                                                   allocCb,
                                                   pCurPlatform,
                                                   pCorePlatform->PlatformSettings().interfaceLoggerEnabled,
                                                   pPlacementAddr,
                                                   &pCurPlatform);
    }
#endif

    if (result == Result::Success)
    {
        pPlacementAddr = Util::VoidPtrDec(pPlacementAddr, sizeof(CrashAnalysis::Platform));
        pCurPlatform->SetClientData(pPlacementAddr);

        result = CrashAnalysis::Platform::Create(createInfo,
                                                 allocCb,
                                                 pCurPlatform,
                                                 pCorePlatform->IsCrashAnalysisModeEnabled(),
                                                 pPlacementAddr,
                                                 &pCurPlatform,
                                                 pCorePlatform->GetCrashAnalysisEventProvider());
    }

    if (result == Result::Success)
    {
        (*ppPlatform) = pCurPlatform;
    }

    return result;
}

// =====================================================================================================================
// If pNullGpuInfoArray is non-null, then on output it will be populated with the corresponding text name for each
// NULL GPU ID enumeration.  Otherwise, pNullGpuCount will be set to the maximum number of entries possible in the
// pNullGpuInfoArray structure.
Result PAL_STDCALL EnumerateNullDevices(
    uint32*       pNullGpuCount,
    NullGpuInfo*  pNullGpuInfoArray)
{
#if PAL_BUILD_NULL_DEVICE
    Result  result = Result::Success;

    if (pNullGpuCount != nullptr)
    {
        if (pNullGpuInfoArray == nullptr)
        {
            // This is a query for the max output array size necessary
            *pNullGpuCount = Util::ArrayLen32(GpuInfoLookupTable);
        }
        else
        {
            uint32 nullGpuCount = 0;
            const uint32 outputArrayCapacity = *pNullGpuCount;

            for (uint32 idx = 0; (idx < Util::ArrayLen32(GpuInfoLookupTable)) && (idx < outputArrayCapacity); ++idx)
            {
                const GpuInfo& curGpu = GpuInfoLookupTable[idx];

                if (curGpu.nullId != NullGpuId::Max)
                {
                    NullGpuInfo* pNullGpuInfo = &pNullGpuInfoArray[nullGpuCount++];
                    pNullGpuInfo->nullGpuId   = curGpu.nullId;
                    pNullGpuInfo->pGpuName    = curGpu.pGpuName;
                }
            }

            // Update the number of valid entries in the output array
            *pNullGpuCount = nullGpuCount;
        }
    }
    else
    {
        // No valid count info, can't continue
        result = Result::ErrorInvalidPointer;
    }
#else
    const Result result = Result::Unsupported;
#endif

    return result;
}

// =====================================================================================================================
// Provides the GpuInfo data for the specified NullGpuId.
Result PAL_STDCALL GetGpuInfoForNullGpuId(
    NullGpuId nullGpuId,
    GpuInfo* pGpuInfo)
{
    Result result = Result::NotFound;

    if (pGpuInfo == nullptr)
    {
        result = Result::ErrorInvalidPointer;
    }
    else if (nullGpuId == NullGpuId::Default)
    {
        // By convention we use the first device in the table as our default. It should be the oldest device we support.
        *pGpuInfo = GpuInfoLookupTable[0];
        result    = Result::Success;
    }
    else
    {
        for (uint32 idx = 0; idx < Util::ArrayLen(GpuInfoLookupTable); ++idx)
        {
            const GpuInfo& entry = GpuInfoLookupTable[idx];
            if (entry.nullId == nullGpuId)
            {
                *pGpuInfo = entry;
                result    = Result::Success;
                break;
            }
        }
    }

    return result;
}

// =====================================================================================================================
// Provides the GpuInfo data for the specified GPU name string.
Result PAL_STDCALL GetGpuInfoForName(
    const char* pGpuName,
    GpuInfo*    pGpuInfo)
{
    Result result = Result::NotFound;

    if ((pGpuName == nullptr) || (pGpuInfo == nullptr))
    {
        result = Result::ErrorInvalidPointer;
    }
    else
    {
        for (uint32 idx = 0; idx < Util::ArrayLen(GpuInfoLookupTable); ++idx)
        {
            const GpuInfo& entry = GpuInfoLookupTable[idx];

            if (entry.pGpuName == nullptr)
            {
                continue;
            }

            // Case insensitive comparison of each character. Until reaching the end of either string.
            // Or until finding the ':' delimeter in the table entry (should be of the form "gpuname:gfx###").
            bool found = true;
            for (uint32 i = 0; (entry.pGpuName[i] != '\0') && (entry.pGpuName[i] != ':'); ++i)
            {
                if (tolower(pGpuName[i]) != tolower(entry.pGpuName[i]))
                {
                    // Input string does not match or is too short
                    // (e.g. if "RAVEN2" came before "RAVEN" in the table, we should keep searching)
                    found = false;
                    break;
                }
            }

            if (found)
            {
                *pGpuInfo = entry;
                result    = Result::Success;
                break;
            }
        }
    }

    return result;
}

// =====================================================================================================================
// Provides the GpuInfo data for the specified hardware revision.
Result PAL_STDCALL GetGpuInfoForAsicRevision(
    AsicRevision asicRevision,
    GpuInfo*     pGpuInfo)
{
    Result result = Result::NotFound;

    if (pGpuInfo == nullptr)
    {
        result = Result::ErrorInvalidPointer;
    }
    else
    {
        for (uint32 idx = 0; idx < Util::ArrayLen(GpuInfoLookupTable); ++idx)
        {
            const GpuInfo& entry = GpuInfoLookupTable[idx];
            if (entry.asicRev == asicRevision)
            {
                *pGpuInfo = entry;
                result    = Result::Success;
                break;
            }
        }
    }

    return result;
}

} // Pal
