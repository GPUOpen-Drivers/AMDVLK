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

#pragma once

#include "core/image.h"
#include "core/internalMemMgr.h"
#include "core/privateScreen.h"
#include "core/hw/gfxip/gfxDevice.h"

#include "core/addrMgr/addrMgr.h"
#include "core/dmaUploadRing.h"
#include "palCmdAllocator.h"
#include "palDevice.h"
#include "palDeque.h"
#include "palEvent.h"
#include "palFile.h"
#include "palHashMap.h"
#include "palInlineFuncs.h"
#include "palIntrusiveList.h"
#include "palMutex.h"
#include "palPipeline.h"
#include "palSettingsFileMgr.h"
#include "palSysMemory.h"
#include "palTextWriter.h"
#include "palShaderLibrary.h"
#include "palLiterals.h"

#include "core/hw/amdgpu_asic.h"

#include "g_platformSettings.h"

#if PAL_BUILD_GFX

#include "core/hw/gfxip/gfx9/gfx9PerfCtrInfo.h"

#if PAL_BUILD_GFX12
#include "core/hw/gfxip/gfx12/gfx12PerfCtrInfo.h"
#endif

#endif

typedef void* ADDR_HANDLE;

namespace Util { enum class ValueType : uint32; }
namespace Util { namespace MetroHash { struct Hash; } }

namespace Pal
{
class  CmdAllocator;
class  CmdBuffer;
class  Fence;
class  GpuMemory;
class  Platform;
class  SettingsLoader;
class  Queue;
struct DeviceFinalizeInfo;
struct CmdBufferInternalCreateInfo;
struct GpuMemoryInternalCreateInfo;
struct PalSettings;

// Indicates to the address lib not to use tile index.
constexpr int32 TileIndexUnused = -1;
// Indicates to the address lib to use linear-general mode
constexpr int32 TileIndexLinearGeneral = -2;
// Indicates to the address lib that macro mode index is not necessary.
constexpr int32 TileIndexNoMacroIndex = -3;

// Minimum HW requirement is 32 bits for the flat address instruction with 32 bit ISA.
constexpr uint32 VaRangeLimitTo32bits = 32u;

// PAL requires the range of the device virtual address space to be atleast 36 bits.
constexpr uint32 MinVaRangeNumBits = 36u;

// PAL minimum fragment size for local memory allocations
constexpr gpusize PageSize = 0x1000u;

constexpr const char* SettingsFileName = VulkanSettingsFileName;

// Maximum number of excluded virtual address ranges.
constexpr size_t MaxExcludedVaRanges = 32;

// Enumerates the internal virtual address space partitions used for supporting multiple VA ranges.
enum class VaPartition : uint32
{
    Default,                // SEE: VaRange::Default
    DefaultBackup,          // Some platforms don't have enough virtual address space to use 4 GB sections for
                            // DescriptorTable and ShadowDescriptorTable, so we split the default VaRange into
                            // two pieces so that the other VA ranges will fit better.
    DescriptorTable,        // SEE: VaRange::DescriptorTable
    ShadowDescriptorTable,  // SEE: VaRange::ShadowDescriptorTable
    Svm,                    // SEE: VaRange::Svm
    CaptureReplay,          // SEE: VaRange::CaptureReplay
    Prt,                    // Some platforms require a specific VA range in order to properly setup HW for PRTs.
                            // To simplify client support, no corresponding VA range exists and this partition
                            // will be chosen instead of the default when required.
    Count,
};

// Tracks device-wide information for the hardware scheduler (HWS).
struct HwsContextInfo
{
    uint32 userQueueSize; // User queue size in bytes

    union
    {
        struct
        {
            uint32 numQueuesRealtime : 4;  // Realtime
            uint32 numQueuesHigh     : 4;  // High
            uint32 numQueuesMedium   : 4;  // Medium
            uint32 numQueuesNormal   : 4;  // Normal
            uint32 numQueuesLow      : 4;  // Idle
            uint32 maxTotalQueues    : 8;  // Max number of queues in a context
            uint32 reserved          : 4;
        };
        uint32 u32All;
    } bits;
};

// Indicates the number of available pipes for each engine type.
struct HwsPipesPerEngine
{
    union
    {
        struct
        {
            uint32 graphics  : 4;
            uint32 compute   : 6;
            uint32 dma       : 4;
            uint32 reserved  : 18;
        };
        uint32 u32All;
    };
};

// Indicates whether this engine instance can be used for gang submission workloads via a multi-queue.
struct GangSubmitEngineSupportFlags
{
    union
    {
        struct
        {
            uint32 graphics  : 1;
            uint32 compute   : 1;
            uint32 dma       : 1;
            uint32 reserved : 29;
        };
        uint32 u32All;
    };
};

struct UserModeSubmissionFlags
{
    union
    {
        struct
        {
            uint32 useSecondaryDoorbell : 1;  // Indicates if user mode submission requires a secondary
                                              // (aka aggregate) doorbell
            uint32 reserved             : 31;
        };
        uint32 u32All;
    };
};

struct HwsInfo
{
    union
    {
        struct
        {
            uint32 gfxHwsEnabled     : 1;  // flag graphic engine enablement using OS HWS (MES)
            uint32 computeHwsEnabled : 1;  // flag compute engine enablement using OS HWS (MES)
            uint32 dmaHwsEnabled     : 1;  // flag dma engine enablement using OS HWS (MES)
            uint32 vcnHwsEnabled     : 1;  // flag vcn engine enablement using OS HWS (MM HWS)
            uint32 reserved1         : 1;
            uint32 reserved          : 27;
        };
        uint32 osHwsEnableFlags;
    };

    HwsContextInfo               gfx;                    // Graphics HWS context info
    HwsContextInfo               compute;                // Compute HWS context info
    HwsContextInfo               sdma;                   // SDMA HWS context info
    HwsContextInfo               vcn;                    // VCN HWS context info
    uint32                       gdsSaveAreaSize;        // GDS save area size in bytes
    uint64                       engineOrdinalMask;      // Indicates which engines (by ordinal) support MES HWS
    uint64                       videoEngineOrdinalMask; // Indicates which video engines (by ordinal) support UMS HWS
    // Indicates whether this engine instance can be used for gang submission workloads via a multi-queue.
    GangSubmitEngineSupportFlags gangSubmitEngineFlags;
    UserModeSubmissionFlags      userModeSubmissionFlags;
    // Indicates the number of available pipes for each engine type.
    HwsPipesPerEngine            numOfPipesPerEngine;
    // Indicate which nodes support native fences (each bit represents a node ordinal)
    uint64                       nativeFenceNodeOrdinalMask;
    // Indicate which nodes support user mode submission (each bit represents a node ordinal)
    uint64                       userModeSubmissionNodeOrdinalMask;
};

// Additional flags that are kept with the IP levels
union HwIpLevelFlags
{
    struct {
        uint32 isSpoofed  : 1; // GPU is spoofed and OS props should be ignored.
        uint32 reserved   : 31;
    };
    uint32 u32All;
};

// Bundles the IP levels for all kinds of HW IP.
struct HwIpLevels
{
    IpTriple       gfx;
    VcnIpLevel     vcn;
    PspIpLevel     psp;
    HwIpLevelFlags flags;
};

// Bundles the sizes of the HW IP specific device classes.
struct HwIpDeviceSizes
{
    size_t gfx;
};

// Indicates a allocated virtual address range.
struct VaRangeInfo
{
    gpusize baseVirtAddr;    // Base address of the virtual address range
    gpusize size;            // Size of the virtual address range (in bytes)
};

// Everything PAL & its clients would ever need to know about a GPU's memory subsystem.
struct GpuMemoryProperties
{
    LocalMemoryType localMemoryType;
    uint32          memOpsPerClock;
    uint32          vramBusBitWidth;
    // For APU's, memory bandwidth is shared between the GPU and the CPU. This is the factor which total memory
    // bandwidth is reduced by to get the true GPU memory bandwidth.
    uint32          apuBandwidthFactor;

    gpusize nonLocalHeapSize;
    gpusize hbccSizeInBytes;    // Size of High Bandwidth Cache Controller (HBCC) memory segment.
                                // HBCC memory segment comprises of system and local video memory, where HW/KMD
                                // will ensure high performance by migrating pages accessed by hardware to local.
                                // This HBCC memory segment is only available on certain platforms.
    gpusize busAddressableMemSize;
    gpusize barSize;            // Total VRAM which can be accessed by the CPU.

    gpusize vaStart;        // Starting address of the GPU's virtual address space
    gpusize vaEnd;          // Ending address of the GPU's virtual address space

    // Some hardware has dynamically-sizeable virtual address ranges. For such GPU's, this is the initial ending
    // address of the virtual address space.
    gpusize vaInitialEnd;
    // NOTE: To prevent consuming all of a GPU's available global or submission memory reference entries for the
    // GPU's page table blocks, we limit the "usable" portion of the GPU address space to put an effective upper
    // bound on the number of page table blocks a given Device will require.
    gpusize vaUsableEnd;    // Ending address of the "usable" portion of the GPU's virtual address space
    gpusize vaStartPrt;     // Subset of the VA range for Partially Resident Textures (PRTs) when required by KMD

    gpusize pdeSize;            // Size (in bytes) of a page directory entry (PDE)
    gpusize pteSize;            // Size (in bytes) of a page table entry (PTE)
    gpusize spaceMappedPerPde;  // Amount of VA space mapped in a single PDE
    uint32  numPtbsPerGroup;    // Number of 4K page table blocks (PTBs) grouped into a single allocation
    gpusize fragmentSize;       // Size in bytes of a video memory fragment.
    uint32  uibVersion;         // Version number of the Unmap-info Buffer format

    gpusize realMemAllocGranularity;    // The addrs and sizes of "real" GPU memory objects must be aligned to this.
    gpusize virtualMemAllocGranularity; // The addrs and sizes of virtual GPU memory objects must be aligned to this.
    gpusize virtualMemPageSize;         // Size in bytes of a virtual GPU memory page.

    gpusize privateApertureBase; // Private memory base address for generic address space implementation.
    gpusize sharedApertureBase;  // Shared memory base address for generic address space implementation.

    gpusize gpuvmOffsetForSvm;   // Offset provided by KMD for 64bit Win10 SVM capable HW to access gpu va.

    gpusize dcnPrimarySurfaceVaStartAlign; // Starting VA alignment of primary surface provided by KMD
    gpusize dcnPrimarySurfaceVaSizeAlign;  // Physical size alignment of primary surface provided by KMD

    // BIG_PAGE is not supported for allocations < bigPageMinAlignment. If BIG_PAGE is supported then
    // allocations >= bigPageMinAlignment and < bigPageLargeAlignment must have their size, VA, and PA all aligned to
    // bigPageMinAlignment and for allocations > bigPageLargeAlignment aligned to bigPageLargeAlignment.
    // If KMD LargePage feature is disabled, bigPageLargeAlignment = bigPageMinAlignment.
    // If bigPageMinAlignment = 0, BIG_PAGE is not supported.
    gpusize bigPageLargeAlignment;
    gpusize bigPageMinAlignment;

    // ITERATE_256 is not supported for allocations < iterate256MinAlignment and in that case iterate256 bit must be
    // set to 1. If ITERATE_256 is supported then iterate256 bit is set to 0 and allocations >= iterate256MinAlignment
    // and < iterate256LargeAlignment must have their size, VA, and PA all aligned to iterate256MinAlignment and for
    // allocations > iterate256LargeAlignment aligned to iterate256LargeAlignment.
    // If KMD LargePage feature is disabled, iterate256LargeAlignment = iterate256MinAlignment.
    // If Iterate256MinAlignment is 0, iterate256 bit must always be set to 1 for the relevant depth buffers.
    gpusize iterate256LargeAlignment;
    gpusize iterate256MinAlignment;

    union
    {
        struct
        {
            uint32 intraSubmitMigration       :  1; // Indicates support for GPU memory migration during submissions.
            uint32 virtualRemappingSupport    :  1; // Indicates support for remapping virtual memory
            uint32 pinningSupport             :  1; // Indicates support for accessing pinned system memory
            uint32 supportPerSubmitMemRefs    :  1; // Indicates whether specifying memory references at Submit time
                                                    // is supported versus via the IDevice or IQueue AddReferences()
            uint32 ptbInNonLocal              :  1; // Indicates hardware support for PTB's being in non-local memory
            uint32 resizeableVaRange          :  1; // Indicates that the hardware's VA range is resizable
            uint32 adjustVmRangeEscapeSupport :  1; // Indicates that adjusting the VA range requires an Escape
            uint32 multipleVaRangeSupport     :  1; // Indicates the device has enough VA range to support multiple
                                                    // partitions of that range.
            uint32 defaultVaRangeSplit        :  1; // Indicates that the device is somewhat limited in VA range, and
                                                    // needs to split up the 'Default' VA range to accomodate this.
            uint32 globalGpuVaSupport         :  1; // Indicates support for GPU virtual addresses that are visible
                                                    // to all devices.
            uint32 svmSupport                 :  1; // Indicates support for SVM
            uint32 shadowDescVaSupport        :  1; // Indicates support to for shadow desc VA range.
            uint32 iommuv2Support             :  1; // Indicates support for IOMMUv2
            uint32 autoPrioritySupport        :  1; // Indiciates that the platform supports automatic allocation
                                                    // priority management.
            uint32 supportsTmz                :  1; // Indicates TMZ (or HSFB) protected memory is supported.
            uint32 supportsMall               :  1; // Indicates that this device supports the MALL.
            uint32 supportPageFaultInfo       :  1; // Indicates support for querying page fault information
#if PAL_BUILD_GFX12
            uint32 supportDistributedCompression    :  1; // Support for GFX12-style distributed compression
            uint32 supportDistributedCompressionCpu :  1; // The CPU can read and write distributed compression.
#else
            uint32 reserved1                  :  2;
#endif
            uint32 reserved                   : 13;
        };
        uint32 u32All;
    } flags;

    // Some areas of the GPU's virtual address range are reserved for system or future use. This list contains all
    // ranges of invalid GPU virtual addresses which cannot be assigned for use by a user-mode driver.
    size_t numExcludedVaRanges;
    VaRangeInfo excludedRange[MaxExcludedVaRanges];

    // These are the usable areas of the GPU's virtual address range. The whole range is partitioned into several
    // several smaller ranges designated for specific use cases (such as descriptor tables in Vulkan or D3D12).
    VaRangeInfo vaRange[static_cast<uint32>(VaPartition::Count)];

    struct
    {
        // Total size in bytes calculated with LargePageSize value from KMD. 4k*2^(LargePageSize)
        // LargePageSize 0 = 4K; 4 = 64K ; 9 = 2MB etc.
        gpusize largePageSizeInBytes;
        // Minimum size in bytes for which GPUVA\Size alignment needed. Calculated with MinSurfaceSizeForAlignment
        // from KMD = 4k*2^(MinSurfaceSizeForAlignment)
        gpusize minSurfaceSizeForAlignmentInBytes;
        // the starting GPUVA of the allocations must be aligned to the LargePageSize
        bool gpuVaAlignmentNeeded;
        // the size of the allocations must be aligned to the LargePageSize.
        // When set, HWVMINFO.fragment == LARGE_PAGE_FEATURES_FLAGS.LargePageSize
        bool sizeAlignmentNeeded;
    } largePageSupport;
};

// Everything PAL & its clients would ever need to know about a GPU's available engines.
struct GpuEngineProperties
{
    // Maximum number of user and internal memory references allowed with a single Queue submission.
    uint32 maxUserMemRefsPerSubmission;
    uint32 maxInternalRefsPerSubmission;

    struct
    {
        uint32   numAvailable;
        uint32   startAlign;                    // Alignment requirement for the starting address of command buffers.
        uint32   sizeAlignInDwords;             // Alignment requirement for the size of command buffers.
        uint32   maxControlFlowNestingDepth;    // Maximum depth of nested control-flow operations in command buffers.
        Extent3d minTiledImageCopyAlignment;    // Minimum alignments for X/Y/Z/Width/Height/Depth for
                                                // ICmdBuffer::CmdCopyImage() between optimally tiled images.
        Extent3d minTiledImageMemCopyAlignment; // Minimum alignments for X/Y/Z/Width/Height/Depth for
                                                // ICmdBuffer::CmdCopyImageToMemory() or
                                                // ICmdBuffer::CmdCopyMemoryToImage() for optimally tiled images.
        Extent3d minLinearMemCopyAlignment;     // Minimum alignments for X/Y/Z/Width/Height/Depth for
                                                // ICmdBuffer::CmdCopyTypedBuffer()
        uint32   minTimestampAlignment;         // If timestampSupport is set, this is the minimum address alignment in
                                                // bytes of the dstOffset argument to ICmdBuffer::CmdWriteTimestamp().
        uint32   queueSupport;                  // A mask of QueueTypeSupport flags indicating queue support.
        uint32   maxNumDedicatedCu;             // The maximum possible number of dedicated CUs per compute ring
        uint32   dedicatedCuGranularity;        // The granularity at which compute units can be dedicated to a queue.

        // Each of the following is used for mid-command-buffer preemption
        gpusize  fwShadowAreaSize;              // Size of the FW-driven shadow area for this engine, in bytes.
        gpusize  fwShadowAreaAlignment;         // Alignment of the FW-driven shadow area for this engine, in bytes.
        gpusize  contextSaveAreaSize;           // Size of the context-save-area for this engine, in bytes.
        gpusize  contextSaveAreaAlignment;      // Alignment of the context-save-area for this engine, in bytes.
        gpusize  gdsSaveAreaSize;               // Size of the GDS-save-area for this engine, in bytes.
        gpusize  gdsSaveAreaAlignment;          // Alignment of the GDS-save-area for this engine, in bytes.

        union
        {
            struct
            {
                uint32 physicalAddressingMode          :  1;
                uint32 mustBuildCmdBuffersInSystemMem  :  1;
                uint32 timestampSupport                :  1;
                uint32 borderColorPaletteSupport       :  1;
                uint32 queryPredicationSupport         :  1;
                uint32 memory64bPredicationSupport     :  1;
                uint32 memory32bPredicationSupport     :  1;
                uint32 conditionalExecutionSupport     :  1;
                uint32 loopExecutionSupport            :  1;
                uint32 constantEngineSupport           :  1;
                uint32 regMemAccessSupport             :  1;
                uint32 supportsMismatchedTileTokenCopy :  1;
                uint32 supportsImageInitBarrier        :  1;
                uint32 supportsImageInitPerSubresource :  1;
                uint32 indirectBufferSupport           :  1;
                uint32 supportVirtualMemoryRemap       :  1;
                uint32 supportsMidCmdBufPreemption     :  1;
                uint32 supportSvm                      :  1;
                uint32 mustUseSvmIfSupported           :  1;
                uint32 supportsTrackBusyChunks         :  1;
                uint32 supportsUnmappedPrtPageAccess   :  1;
                uint32 memory32bPredicationEmulated    :  1;
                uint32 supportsClearCopyMsaaDsDst      :  1;
                uint32 supportsPws                     :  1; // HW supports pixel wait sync plus.
                uint32 reserved                        :  8;
            };
            uint32 u32All;
        } flags;

        struct
        {
            union
            {
                struct
                {
                    uint32 exclusive                :  1;
                    uint32 mustUseDispatchTunneling :  1;
                    uint32 supportsMultiQueue       :  1;
                    uint32 hwsEnabled               :  1;
                    uint32 reserved                 : 28;
                };
                uint32 u32All;
            } flags;

            uint32 queuePrioritySupport;              // Mask of QueuePrioritySupport flags indicating which queue
                                                      // priority levels are supported.
            uint32 dispatchTunnelingPrioritySupport;  // Mask of QueuePrioritySupport flags indicating which queue
                                                      // priority levels support dispatch tunneling.
            uint32 maxFrontEndPipes;                  // Up to this number of IQueue objects can be consumed in
                                                      //  parallel by the front-end of this engine instance. It will
                                                      //  only be greater than 1 on hardware scheduled engine backed
                                                      //  by multiple hardware pipes/threads.
        } capabilities[MaxAvailableEngines];

        /// Specifies the suggested heap preference clients should use when creating an @ref ICmdAllocator that will
        /// allocate command space for this engine type.  These heap preferences should be specified in the allocHeap
        /// parameter of @ref CmdAllocatorCreateInfo.  Clients are free to ignore these defaults and use their own
        /// heap preferences, but may suffer a performance penalty.
        GpuHeap preferredCmdAllocHeaps[CmdAllocatorTypeCount];

        /// Indicate which queue supports per-command, per-submit, or per-queue TMZ based on the queue type.
        TmzSupportLevel tmzSupportLevel;
    } perEngine[EngineTypeCount];
};

// Everything PAL & its clients would ever need to know about a GPU's available queues.
struct GpuQueueProperties
{
    struct
    {
        union
        {
            struct
            {
                uint32 supportsSwapChainPresents  :  1;
                uint32 reserved744                :  1;
                uint32 supportSplitReleaseAcquire :  1;
                uint32 reserved                   : 29;
            };
            uint32 u32All;
        } flags;

         uint32 supportedDirectPresentModes; // A mask of PresentModeSupport flags indicating support for various
                                             // PresentModes when calling IQueue::PresentDirect().
    } perQueue[QueueTypeCount];

    // Maximum number of command streams allowed in a single Queue submission.
    uint32 maxNumCmdStreamsPerSubmit;
};

// Minimum number of command streams which need to be supported per Queue submission in order for PAL to function
// properly.
constexpr uint32 MinCmdStreamsPerSubmission = 4;

// Size of an instruction cache line (bytes).
constexpr gpusize ShaderICacheLineSize      = 64;

// Maximum number of VGPRs that any one shader can access.
constexpr uint32 MaxVgprPerShader = 256;

#if PAL_BUILD_GFX
constexpr uint32 Gfx9MaxShaderEngines = 6;  // GFX11 parts have six SE's
#endif

// Minimum PFP/MEC uCode version that indicates the device is running in RS64 mode
constexpr uint32 Gfx11Rs64MinPfpUcodeVersion = 300;
constexpr uint32 Gfx11Rs64MinMecUcodeVersion = 300;

// =====================================================================================================================
// Helper function to convert GfxIpLevel + stepping into a IpTriple
constexpr IpTriple CreateGfxTriple(
    GfxIpLevel gfxIpLevel,
    uint16     stepping)
{
    IpTriple retVal = {};

    switch (gfxIpLevel)
    {
#if PAL_BUILD_GFX
    case GfxIpLevel::GfxIp10_1:
        retVal = { .major = 10, .minor = 1, .stepping = stepping };
        break;
    case GfxIpLevel::GfxIp10_3:
        retVal = { .major = 10, .minor = 3, .stepping = stepping };
        break;
    case GfxIpLevel::GfxIp11_0:
        retVal = { .major = 11, .minor = 0, .stepping = stepping };
        break;
    case GfxIpLevel::GfxIp11_5:
        retVal = { .major = 11, .minor = 5, .stepping = stepping };
        break;
#if PAL_BUILD_GFX12
    case GfxIpLevel::GfxIp12:
        retVal = { .major = 12, .minor = 0, .stepping = stepping };
        break;
#endif
#endif
    default:
        PAL_ASSERT_ALWAYS();
        break;
    }

    return retVal;
}

// =====================================================================================================================
// Helper function to convert IpTriple into GfxIpLevel
constexpr GfxIpLevel IpTripleToGfxLevel(
    IpTriple ipTriple)
{
    GfxIpLevel retVal = GfxIpLevel::None;

    switch (ipTriple.major)
    {
#if PAL_BUILD_GFX
    case 10:
        switch (ipTriple.minor)
        {
        case 1:
            retVal = GfxIpLevel::GfxIp10_1;
            break;
        case 3:
            retVal = GfxIpLevel::GfxIp10_3;
            break;
        default:
            PAL_ASSERT_ALWAYS();
            break;
        }
        break;
    case 11:
        switch (ipTriple.minor)
        {
        case 0:
            retVal = GfxIpLevel::GfxIp11_0;
            break;
        case 5:
            retVal = GfxIpLevel::GfxIp11_5;
            break;
        default:
            PAL_ASSERT_ALWAYS();
            break;
        }
        break;
#if PAL_BUILD_GFX12
    case 12:
        switch (ipTriple.minor)
        {
        case 0:
            retVal = GfxIpLevel::GfxIp12;
            break;
        default:
            PAL_ASSERT_ALWAYS();
            break;
        }
        break;
#endif
#endif
    default:
        PAL_ASSERT_ALWAYS();
        break;
    }

    return retVal;
}

// Everything PAL & its clients would ever need to know about the actual GPU hardware.
struct GpuChipProperties
{
    uint32  gfxEngineId;    // Coarse-grain GFX engine ID (R800, SI, etc.).
    uint32  familyId;       // Hardware family ID.  Driver-defined identifier for a particular family of devices.
                            // E.g., FAMILY_NV3, etc. as defined in amdgpu_asic.h
    uint32  eRevId;         // Hardware revision ID.  Driver-defined identifier for a particular device and
                            // sub-revision in the hardware family designated by the familyId.
                            // See AMDGPU_NAVI31_RANGE, etc. as defined in amdgpu_asic.h.
    uint32  revisionId;     // PCI revision ID.  8-bit value as reported in the device structure in the PCI config
                            // space.  Identifies a revision of a specific PCI device ID.
    uint32  deviceId;       // PCI device ID.  16-bit value device ID as reported in the PCI config space.
    uint32  gpuIndex;       // Index of this GPU in whatever group of GPU's it belongs to.

    AsicRevision   revision;  // PAL specific ASIC revision identifier
    GpuType        gpuType;
    GfxIpLevel     gfxLevel;
    VcnIpLevel     vcnLevel;
    PspIpLevel     pspLevel;
    HwIpLevelFlags hwIpFlags;
    uint32         gfxStepping; // Stepping level of this GPU's GFX block.
    IpTriple       gfxTriple;

    uint16   gpuPerformanceCapacity; // GpuCapacity is the percentage (in fixed point [1, 65535]) of the
                                     // GPU's performance that can be used. 0 is invalid (not SRIOV).
    uint32   vcnUcodeVersion;                   // VCN Video encode firmware version
    uint32   vcnEncodeFwInterfaceVersionMajor;  // VCN Video encode firmware interface major version
    uint32   vcnEncodeFwInterfaceVersionMinor;  // VCN Video encode firmware interface minor version
    uint32   vcnFwVersionSubMinor;              // VCN Video firmware sub-minor version (revision)

    uint32 cpUcodeVersion;      // Command Processor feature version.
                                // Assume all blocks in the CP share the same feature version.
    uint32 pfpUcodeVersion;     // PrefetchProcessor Command Processor firmware version.
    uint32 meUcodeVersion;      // ME Command Processor firmware version.
    uint32 mecUcodeVersion;     // ME Compute Command Processor firmware version.

    struct
    {
        union
        {
            struct
            {
                // Images created on this device supports AQBS stereo mode, this AQBS stereo mode doesn't apply to the
                // array-based stereo feature supported by Presentable images.
                uint32 supportsAqbsStereoMode       :  1;

                // Images created on this device support being sampled with corner sampling.
                uint32 supportsCornerSampling       :  1;
                // Whether to support Display Dcc
                uint32 supportDisplayDcc            :  1;
                // Load bearing placeholder, do not touch.
                uint32 placeholder0                 :  1;
                // Reserved for future use.
                uint32 reserved                     : 28;
            };
            uint32 u32All;
        } flags;

        uint32                 minPitchAlignPixel;
        Extent3d               maxImageDimension;
        uint32                 maxImageArraySize;
        uint32                 maxImageMipLevels;
        PrtFeatureFlags        prtFeatures;
        gpusize                prtTileSize;
        MsaaFlags              msaaSupport;
        uint8                  maxMsaaFragments;
        uint8                  numSwizzleEqs;
        const SwizzleEquation* pSwizzleEqs;
        bool                   tilingSupported[static_cast<size_t>(ImageTiling::Count)];
        Extent2d               vrsTileSize;  // Pixel dimensions of a VRS tile.  0x0 indicates image-based shading
                                             // rate is not supported.
    } imageProperties;

    // GFXIP specific information which is shared by all GFXIP hardware layers.
#if PAL_BUILD_GFX
    struct
    {
        uint32 maxUserDataEntries;
        uint32 vaRangeNumBits;
        uint32 realTimeCuMask;
        uint32 maxThreadGroupSize;
        uint32 maxAsyncComputeThreadGroupSize;
        uint32 maxComputeThreadGroupCountX;          // Maximum number of compute thread groups to dispatch
        uint32 maxComputeThreadGroupCountY;
        uint32 maxComputeThreadGroupCountZ;
        uint32 hardwareContexts;
        uint32 ldsSizePerThreadGroup;                // Maximum LDS size available per thread group in bytes.
        uint32 ldsSizePerCu;                         // Maximum LDS size available per CU in KB.
        uint32 ldsGranularity;                       // LDS allocation granularity in bytes.
        uint32 numOffchipTessBuffers;
        uint32 offChipTessBufferSize;                // Size of each off-chip tessellation buffer in bytes.
        uint32 tessFactorBufferSizePerSe;            // Size of the tessellation-factor buffer per SE, in bytes.
        uint32 ceRamSize;                            // Maximum on-chip CE RAM size in bytes.
        uint32 maxPrimgroupSize;
        uint32 mallSizeInBytes;                      // Total size in bytes of MALL (Memory Attached Last Level - L3)
                                                     // cache in the device.
        uint32 tccSizeInBytes;                       // Total size in bytes of TCC (L2) in the device.
        uint32 tcpSizeInBytes;                       // Size in bytes of one TCP (L1). There is one TCP per CU.
        uint32 gl1cSizePerSa;                        // Size in bytes of GL1 cache per SA.
        uint32 instCacheSizePerCu;                   // Size in bytes of instruction cache per CU/WGP.
        uint32 scalarCacheSizePerCu;                 // Size in bytes of scalar cache per CU/WGP.
        uint32 maxLateAllocVsLimit;                  // Maximum number of VS waves that can be in flight without
                                                     // having param cache and position buffer space.
        uint32 shaderPrefetchBytes;                  // Number of bytes the SQ will prefetch, if any.

        uint32 gl2UncachedCpuCoherency;              // If supportGl2Uncached is set, then this is a bitmask of all
                                                     // CacheCoherencyUsageFlags that will be coherent with CPU
                                                     // reads/writes. Note that reporting CoherShader only means
                                                     // that GLC accesses will be CPU coherent.
                                                     // Note: Only valid if supportGl2Uncached is true.
        uint32 maxGsOutputVert;                      // Maximum number of GS vertices output.
        uint32 maxGsTotalOutputComponents;           // Maximum number of GS output components totally.
        uint32 maxGsInvocations;                     // Maximum number of GS prim instances, corresponding to geometry
                                                     // shader invocation in glsl.
        uint32 soCtrlBufSize;

        // Mask of active pixel packers. The mask is 128 bits wide, assuming a max of 32 SEs and a max of 4 pixel
        // packers (indicated by a single bit each) per physical SE (includes harvested SEs).
        uint32 activePixelPackerMask[ActivePixelPackerMaskDwords];

#if PAL_BUILD_GFX12
        // GFXIP information pertaining to SE-specific work graph scheduler (WGS) RS64 cores
        struct
        {
            bool supported;                             // WGS cores are supported on the gfxip
            uint32 metadataAddrAlignment;               // WGS metadata memory address alignment.
            uint32 instrCacheAddrAlignment;             // WGS firmware instruction cache required address
                                                        // alignment. Firmware instruction memory region should be
                                                        // aligned to this boundary.
            uint32 dataCacheAddrAlignment;              // WGS firmware data cache required address alignment.
                                                        // Firmware data memory should be aligned to this boundary.
        } wgs;
#endif

        struct
        {
            uint32 supportGl2Uncached               :  1; // Indicates support for the allocation of GPU L2
                                                          // un-cached memory. See gl2UncachedCpuCoherency
            uint32 supportsVrs                      :  1; // Indicates support for variable rate shading
            uint32 supportsSwStrmout                :  1; // Indicates support for software streamout
            uint32 supportsHwVs                     :  1; // Indicates hardware support for Vertex Shaders
            uint32 supportCaptureReplay             :  1; // Indicates support for Capture Replay
            uint32 supportHsaAbi                    :  1;
            uint32 supportAceOffload                :  1;
            uint32 supportStaticVmid                :  1; // Indicates support for static-VMID.
            uint32 supportFloat32BufferAtomics      :  1; // Indicates support for float32 buffer atomics
            uint32 supportFloat32ImageAtomics       :  1; // Indicates support for float32 image atomics
            uint32 supportFloat32BufferAtomicAdd    :  1; // Indicates support for float32 buffer atomics add op
            uint32 supportFloat32ImageAtomicAdd     :  1; // Indicates support for float32 image atomics add op
            uint32 supportFloat32ImageAtomicMinMax  :  1; // Indicates support for float32 image atomics min and max op
            uint32 supportFloat64BufferAtomicMinMax :  1; // Indicates support for float64 image atomics min and max op
            uint32 supportFloat64SharedAtomicMinMax :  1; // Indicates support for float64 shared atomics min and max op
            uint32 support1dDispatchInterleave      :  1; // Indicates support for 1D Dispatch Interleave
#if PAL_BUILD_GFX12
            uint32 support2dDispatchInterleave      :  1; // Indicates support for 2D Dispatch Interleave
#else
            uint32 placeholder1                     :  1;
#endif
            uint32 gfx9DataValid                    :  1;
            uint32 reserved                         : 14;
        };
    } gfxip;
#endif

    // GFX family specific data which may differ based on graphics IP level.
    union
    {
        // Hardware-specific information for GFXIP 9+ hardware.
        struct
        {
            uint32 backendDisableMask;
            uint32 gbAddrConfig;
            uint32 spiConfigCntl;
            uint32 paScTileSteeringOverride;
            uint32 numShaderEngines;        // Max Possible SEs on this GPU
            uint32 activeSeMask;
            uint32 numActiveShaderEngines;  // Number of Non-harvested SEs
            uint32 numShaderArrays;
            uint32 maxNumRbPerSe;
            uint32 activeNumRbPerSe;
            uint32 numScPerSe;              // Num Shader Complex per Shader Engine
            uint32 numPackerPerSc;
            uint32 nativeWavefrontSize;
            uint32 minWavefrontSize;        // The smallest supported wavefront size.
            uint32 maxWavefrontSize;        // All powers of two between the min size and max size are supported.
            uint32 numShaderVisibleSgprs;
            uint32 numPhysicalSgprs;
            uint32 minSgprAlloc;
            uint32 sgprAllocGranularity;
            uint32 numPhysicalVgprsPerSimd;
            uint32 numPhysicalVgprs;
            uint32 minVgprAlloc;
            uint32 vgprAllocGranularity;
            uint32 numCuPerSh;
            uint32 maxNumCuPerSh;
            uint32 numActiveCus;
            uint32 numAlwaysOnCus;
            uint32 numPhysicalCus;
            uint32 numTccBlocks;
            uint32 numSimdPerCu;
            uint32 numWavesPerSimd;
            uint32 numSqcBarriersPerCu;
            uint32 numActiveRbs;
            uint32 numTotalRbs;
            uint32 gsVgtTableDepth;
            uint32 gsPrimBufferDepth;
            uint32 maxGsWavesPerVgt;
            uint32 parameterCacheLines;
            uint32 sdmaDefaultRdL2Policy;
            uint32 sdmaDefaultWrL2Policy;
            bool   sdmaL2PolicyValid;
            uint32 activeCuMask  [Gfx9MaxShaderEngines] [MaxShaderArraysPerSe]; // Indexed using the physical SE
            uint32 alwaysOnCuMask[Gfx9MaxShaderEngines] [MaxShaderArraysPerSe]; // Indexed using the physical SE

            uint32 numSdpInterfaces;          // Number of Synchronous Data Port interfaces to memory.

            struct
            {
                uint32  numTcpPerSa;
                uint32  numWgpAboveSpi;
                uint32  numWgpBelowSpi;
                uint32  numGl2a;
                uint32  numGl2c;

                uint32  supportedVrsRates;          // Bitmask of VrsShadingRate enumerations indicating supported modes
                bool    supportVrsWithDsExports;    // If true, asic support coarse VRS rates
                                                    // when z or stencil exports are enabled

                uint32  minNumWgpPerSa;
                uint32  maxNumWgpPerSa;
                uint16  activeWgpMask  [Gfx9MaxShaderEngines] [MaxShaderArraysPerSe];   // Indexed using the physical SE
                uint16  alwaysOnWgpMask[Gfx9MaxShaderEngines] [MaxShaderArraysPerSe];   // Indexed using the physical SE
            } gfx10;

            struct
            {

                uint64 doubleOffchipLdsBuffers             :  1; // HW supports 2x number of offchip LDS buffers
                                                                 // per SE
                uint64 supportFp16Fetch                    :  1;
                uint64 supportFp16Dot2                     :  1;
                uint64 support16BitInstructions            :  1;
                uint64 support64BitInstructions            :  1;
                uint64 supportBorderColorSwizzle           :  1;
                uint64 supportFloat64Atomics               :  1;
                uint64 supportDoubleRate16BitInstructions  :  1;
                uint64 rbPlus                              :  1;
                uint64 supportConservativeRasterization    :  1;
                uint64 supportPrtBlendZeroMode             :  1;
                uint64 supports2BitSignedValues            :  1;
                uint64 supportPrimitiveOrderedPs           :  1;
                uint64 lbpwEnabled                         :  1; // Indicates Load Balance Per Watt is enabled
                uint64 supportPatchTessDistribution        :  1; // HW supports patch distribution mode.
                uint64 supportDonutTessDistribution        :  1; // HW supports donut distribution mode.
                uint64 supportTrapezoidTessDistribution    :  1; // HW supports trapezoidal distribution mode.
                uint64 supportAddrOffsetDumpAndSetShPkt    :  1; // Indicates support for DUMP_CONST_RAM_OFFSET
                                                                 // and SET_SH_REG_OFFSET indexed packet.
                uint64 supportAddrOffsetSetSh256Pkt        :  1; // Indicates support for SET_SH_REG_OFFSET_256B
                                                                 // indexed packet.
                uint64 supportImplicitPrimitiveShader      :  1;
                uint64 supportSpp                          :  1; // HW supports Shader Profiling for Power
                uint64 validPaScTileSteeringOverride       :  1; // Value of paScTileSteeringOverride is valid
                uint64 supportPerShaderStageWaveSize       :  1; // HW supports changing the wave size
                uint64 supportCustomWaveBreakSize          :  1;
                uint64 supportMsaaCoverageOut              :  1; // HW supports MSAA coverage samples
                uint64 supportPostDepthCoverage            :  1; // HW supports post depth coverage feature
                uint64 supportSpiPrefPriority              :  1;
                uint64 support1xMsaaSampleLocations        :  1; // HW supports 1xMSAA custom quad sample patterns
                uint64 eccProtectedGprs                    :  1; // Are VGPR's ECC-protected?
                uint64 overrideDefaultSpiConfigCntl        :  1; // KMD provides default value for SPI_CONFIG_CNTL.
                uint64 supportOutOfOrderPrimitives         :  1; // HW supports higher throughput for out of order
                uint64 supportIntersectRayBarycentrics     :  1; // HW supports the ray intersection mode which
                                                                 // returns triangle barycentrics.
                uint64 supportShaderSubgroupClock          :  1; // HW supports clock functions across subgroup.
                uint64 supportShaderDeviceClock            :  1; // HW supports clock functions across device.
                uint64 supportAlphaToOne                   :  1; // HW supports forcing alpha channel to one
                uint64 supportSingleChannelMinMaxFilter    :  1; // HW supports any min/max filter.
                uint64 supportSortAgnosticBarycentrics     :  1; // HW provides provoking vertex for custom interp
                uint64 supportMeshShader                   :  1;
                uint64 supportTaskShader                   :  1;
                uint64 supportMsFullRangeRtai              :  1; // HW supports full range render target array
                                                                 // for Mesh Shaders.
                uint64 supportRayTraversalStack            :  1;
                uint64 supportPointerFlags                 :  1;
                uint64 supportTextureGatherBiasLod         :  1; // HW supports SQ_IMAGE_GATHER4_L_O
                uint64 supportInt8Dot                      :  1; // HW supports a dot product 8bit.
                uint64 supportInt4Dot                      :  1; // HW supports a dot product 4bit.
                uint64 supportMixedSignIntDot              :  1; // HW supports a integer dot product with mixed sign
                                                                 // inputs.
                uint64 support2DRectList                   :  1; // HW supports PrimitiveTopology::TwoDRectList.
                uint64 supportImageViewMinLod              :  1; // Indicates image srd supports min_lod.
                uint64 stateShadowingByCpFw                :  1; // Indicates that state shadowing is done is CP FW.
                uint64 stateShadowingByCpFwUserAlloc       :  1; // FW state shadowing memory is allocated by PAL.
                uint64 supportCooperativeMatrix            :  1; // HW supports cooperative matrix
                uint64 placeholder6                        :  1;
                uint64 supportBFloat16                     :  1; // Indicates support for bfloat16
                uint64 supportFloat8                       :  1; // HW supports float 8-bit.
                uint64 supportInt4                         :  1; // HW supports integer 4-bit.
#if PAL_BUILD_GFX12
                uint64 supportCooperativeMatrix2           :  1; // HW supports Gfx12 extenson cooperative matrix.
#else
                uint64 placeholder7                        :  1;
#endif
                uint64 placeholder8 : 2;
                uint64 cwsrEnabled                         :  1; // Compute Wave Save/Restore is enabled
                uint64 reserved                            :  5;
            };

            RayTracingIpLevel        rayTracingIp;      //< HW RayTracing IP version

            UseExecuteIndirectPacket executeIndirectSupport; //< Specifies which CmdTypes of ExecuteIndirect are
                                                             //< supported by this FW version.
            union
            {
                Gfx9::PerfCounterInfo  gfx9Info;   // Gfx9 HWL Per block Perf Counter info
#if PAL_BUILD_GFX12
                Gfx12::PerfCounterInfo gfx12Info;  // Gfx12 HWL Per block Perf Counter info
#endif
            } perfCounterInfo;
        } gfx9;
    };

    // Maximum engine and memory clock speeds (MHz)
    uint32 maxEngineClock;  // Max Engine Clock reported is not valid until device is finalized in DX builds.
    uint32 maxMemoryClock;  // Max Memory Clock reported is not valid until device is finalized in DX builds.
    uint32 alusPerClock;
    uint32 pixelsPerClock;
    uint32 primsPerClock;
    uint32 texelsPerClock;
    uint64 gpuCounterFrequency;

    uint32 enginePerfRating;
    uint32 memoryPerfRating;

    // Below SRD sizes are all with bytes unit.
    struct
    {
        uint32 typedBufferView;
        uint32 untypedBufferView;
        uint32 imageView;
        uint32 fmaskView;
        uint32 sampler;
        uint32 bvh;
    } srdSizes;

    struct
    {
        const void* pNullBufferView;
        const void* pNullImageView;
        const void* pNullFmaskView;
        const void* pNullSampler;
    } nullSrds;

    uint32 pciDomainNumber;              // PCI domain number in the system for this GPU
    uint32 pciBusNumber;                 // PCI bus number in the system for this GPU
    uint32 pciDeviceNumber;              // PCI device number in the system for this GPU
    uint32 pciFunctionNumber;            // PCI function number in the system for this GPU
    bool   gpuConnectedViaThunderbolt;   // GPU connects to system through thunder bolt
    bool   requiresOnionAccess;          // Some APUs have issues with Garlic that can be worked around by using Onion
    bool   pcieAtomicOpsSupported;       // Pcie atomics support
    union
    {
        struct
        {
            uint32 p2pSupportEncode     : 1; // whether encoding HW can access FB memory of remote GPU in chain
            uint32 p2pSupportDecode     : 1; // whether decoding HW can access FB memory of remote GPU in chain
            uint32 p2pSupportTmz        : 1; // whether protected content can be transferred over P2P
            uint32 p2pCrossGPUCoherency : 1; // whether remote FB memory can be accessed without need for cache flush
            uint32 xgmiEnabled          : 1; // Whether XGMI is enabled
            uint32 reserved             : 27;
        };
        uint32 u32All;
    } p2pSupport;

};

// Helper function that calculates memory ops per clock for a given memory type.
uint32 MemoryOpsPerClock(LocalMemoryType memoryType);

// =====================================================================================================================
// Represents a client-configurable context for a particular physical GPU. Responsibilities include allocating GDS
// partitions. Also serves as a factory for other child objects, such as Command Buffers.
//
// Each HWIP block may require its own HW-specific Device object. To accommodate this need, the Device will contain an
// object for each HWIP block present in the associated Physical GPU (e.g., GFXIP, etc.). All of these objects
// share a single system-memory allocation. (See: Device::Init() for more details.)
class Device : public IDevice
{
public:
    static constexpr GpuHeap CmdBufInternalAllocHeap      = GpuHeap::GpuHeapGartCacheable;
    static constexpr uint32  CmdBufInternalAllocSize      = 128 * Util::OneKibibyte;
    static constexpr uint32  CmdBufInternalSuballocSize   = 8 * Util::OneKibibyte;
    static constexpr uint32  CmdBufMemReferenceLimit      = 16384;
    static constexpr uint32  InternalMemMgrAllocLimit     = 128;
    static constexpr uint32  GpuMemoryChunkGranularity    = 128 * Util::OneMebibyte;
    static constexpr uint32  MaxMemoryViewStride          = (1UL << 14) - 1;
    static constexpr uint32  CmdStreamReserveLimit        = 256;
    static constexpr uint32  PollInterval                 = 10;
    static constexpr uint32  OcclusionQueryDmaBufferSlots = 256;
    static constexpr uint32  OcclusionQueryDmaLowerBound  = 1024;
    static constexpr bool    CbSimpleFloatEnable          = true;

    virtual ~Device();
    virtual Result Cleanup() override;

    virtual Result EarlyInit(const HwIpLevels& ipLevels);
    virtual Result LateInit();

    virtual Result GetLinearImageAlignments(
        LinearImageAlignments* pAlignments) const override
    {
        return (m_pGfxDevice == nullptr) ? Result::ErrorUnavailable :
                m_pGfxDevice->GetLinearImageAlignments(pAlignments);
    }

    // NOTE: PAL internals can access the same information more directly via the MemoryProperties(), QueueProperties(),
    // or ChipProperties() getters.
    virtual Result GetProperties(
        DeviceProperties* pInfo) const override;

    virtual Result CheckExecutionState(
        PageFaultStatus* pPageFaultStatus) override;

    // NOTE: PAL internals can access the same information more directly via the HeapProperties() getter.
    virtual Result GetGpuMemoryHeapProperties(
        GpuMemoryHeapProperties info[GpuHeapCount]) const override;

    // NOTE: PAL internals can access the same information more directly via non-virtual functions in this class.
    virtual Result GetFormatProperties(
        MergedFormatPropertiesTable* pInfo
        ) const override;

    virtual Result GetPerfExperimentProperties(
        PerfExperimentProperties* pProperties) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result GetDefaultSamplePattern(
        uint32                 samples,
        MsaaQuadSamplePattern* pQuadSamplePattern) const override
    {
        return (m_pGfxDevice == nullptr) ? Result::ErrorUnavailable :
                m_pGfxDevice->GetDefaultSamplePattern(samples, pQuadSamplePattern);
    }

    // NOTE: Part of the public IDevice interface.
    virtual void BindTrapHandler(PipelineBindPoint pipelineType, IGpuMemory* pGpuMemory, gpusize offset) override
    {
        PAL_ASSERT(m_pGfxDevice != nullptr);
        m_pGfxDevice->BindTrapHandler(pipelineType, pGpuMemory, offset);
    }

    // NOTE: Part of the public IDevice interface.
    virtual void BindTrapBuffer(PipelineBindPoint pipelineType, IGpuMemory* pGpuMemory, gpusize offset) override
    {
        PAL_ASSERT(m_pGfxDevice != nullptr);
        m_pGfxDevice->BindTrapBuffer(pipelineType, pGpuMemory, offset);
    }

    // NOTE: Part of the public IDevice interface.
    virtual bool ReadSetting(
        const char*     pSettingName,
        SettingScope    settingScope,
        Util::ValueType valueType,
        void*           pValue,
        size_t          bufferSz = 0) const override;

    // This function is responsible for reading a specific settings from the OS appropriate source
    // (e.g. registry or config file)
    virtual bool ReadSetting(
        const char*          pSettingName,
        Util::ValueType      valueType,
        void*                pValue,
        InternalSettingScope settingType,
        size_t               bufferSz = 0) const;

    virtual PalPublicSettings* GetPublicSettings() override;
    virtual Result CommitSettingsAndInit() override;
    virtual Result Finalize(const DeviceFinalizeInfo& finalizeInfo) override;

    Result SplitSubresRanges(
        uint32              rangeCount,
        const SubresRange*  pRanges,
        uint32*             pSplitRangeCount,
        const SubresRange** ppSplitRanges,
        bool*               pMemAllocated) const;

    virtual Result QueryRawApplicationProfile(
        const wchar_t*           pFilename,
        const wchar_t*           pPathname,
        ApplicationProfileClient client,
        const char**             pOut) = 0;

    virtual Result EnableSppProfile(
        const wchar_t*           pFilename,
        const wchar_t*           pPathname) = 0;

    virtual Result SelectSppTable(
        uint32 pixelCount,
        uint32 msaaRate) const = 0;

    virtual Result SendGpuOpenCommand(
        void*  pData,
        size_t dataSize) { return Result::Unsupported; }

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetQueueSize(
        const QueueCreateInfo& createInfo,
        Result*                pResult) const override;

    // Helper method for determining the size of a Queue context object, in bytes.
    size_t QueueContextSize(const QueueCreateInfo& createInfo) const;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateQueue(
        const QueueCreateInfo& createInfo,
        void*                  pPlacementAddr,
        IQueue**               ppQueue) override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetMultiQueueSize(
        uint32                 queueCount,
        const QueueCreateInfo* pCreateInfo,
        Result*                pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateMultiQueue(
        uint32                 queueCount,
        const QueueCreateInfo* pCreateInfo,
        void*                  pPlacementAddr,
        IQueue**               ppQueue) override;

    // NOTE: Part of the public IDevice interface.
    virtual gpusize GetMaxGpuMemoryAlignment() const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result ResetFences(
        uint32              fenceCount,
        IFence*const*       ppFences) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result WaitForFences(
        uint32                   fenceCount,
        const IFence*const*      ppFences,
        bool                     waitAll,
        std::chrono::nanoseconds timeout) const override;

    // Queries the size of a GpuMemory object, in bytes.
    virtual size_t GpuMemoryObjectSize() const = 0;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetGpuMemorySize(
        const GpuMemoryCreateInfo& createInfo,
        Result*                    pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateGpuMemory(
        const GpuMemoryCreateInfo& createInfo,
        void*                      pPlacementAddr,
        IGpuMemory**               ppGpuMemory) override;

    Result CreateInternalGpuMemory(
        const GpuMemoryCreateInfo&         createInfo,
        const GpuMemoryInternalCreateInfo& internalInfo,
        GpuMemory**                        ppGpuMemory);

    Result CreateInternalGpuMemory(
        const GpuMemoryCreateInfo&         createInfo,
        const GpuMemoryInternalCreateInfo& internalInfo,
        void*                              pPlacementAddr,
        GpuMemory**                        ppGpuMemory);

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetPinnedGpuMemorySize(
        const PinnedGpuMemoryCreateInfo& createInfo,
        Result*                          pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreatePinnedGpuMemory(
        const PinnedGpuMemoryCreateInfo& createInfo,
        void*                            pPlacementAddr,
        IGpuMemory**                     ppGpuMemory) override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetSvmGpuMemorySize(
        const SvmGpuMemoryCreateInfo& createInfo,
        Result*                       pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateSvmGpuMemory(
        const SvmGpuMemoryCreateInfo& createInfo,
        void*                         pPlacementAddr,
        IGpuMemory**                  ppGpuMemory) override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetSharedGpuMemorySize(
        const GpuMemoryOpenInfo& openInfo,
        Result*                  pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetExternalSharedGpuMemorySize(
        Result* pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result OpenSharedGpuMemory(
        const GpuMemoryOpenInfo& openInfo,
        void*                    pPlacementAddr,
        IGpuMemory**             ppGpuMemory) override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetPeerGpuMemorySize(
        const PeerGpuMemoryOpenInfo& openInfo,
        Result*                      pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result OpenPeerGpuMemory(
        const PeerGpuMemoryOpenInfo& openInfo,
        void*                        pPlacementAddr,
        IGpuMemory**                 ppGpuMemory) override;

    virtual Result CreateInternalImage(
        const ImageCreateInfo&         createInfo,
        const ImageInternalCreateInfo& extCreateInfo,
        void*                          pPlacementAddr,
        Image**                        ppImage) = 0;

    // NOTE: Part of the public IDevice interface.
    virtual void GetPeerImageSizes(
        const PeerImageOpenInfo& openInfo,
        size_t*                  pPeerImageSize,
        size_t*                  pPeerGpuMemorySize,
        Result*                  pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result OpenPeerImage(
        const PeerImageOpenInfo& openInfo,
        void*                    pImagePlacementAddr,
        void*                    pGpuMemoryPlacementAddr,
        IImage**                 ppImage,
        IGpuMemory**             ppGpuMemory) override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetColorTargetViewSize(
        Result* pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateColorTargetView(
        const ColorTargetViewCreateInfo& createInfo,
        void*                            pPlacementAddr,
        IColorTargetView**               ppColorTargetView) const override;

    Result CreateInternalColorTargetView(
        const ColorTargetViewCreateInfo&  createInfo,
        ColorTargetViewInternalCreateInfo internalInfo,
        void*                             pPlacementAddr,
        IColorTargetView**                ppColorTargetView) const;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetDepthStencilViewSize(
        Result* pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateDepthStencilView(
        const DepthStencilViewCreateInfo& createInfo,
        void*                             pPlacementAddr,
        IDepthStencilView**               ppDepthStencilView) const override;

    Result CreateInternalDepthStencilView(
        const DepthStencilViewCreateInfo&         createInfo,
        const DepthStencilViewInternalCreateInfo& internalInfo,
        void*                                     pPlacementAddr,
        IDepthStencilView**                       ppDepthStencilView) const;

    // NOTE: Part of the public IDevice interface.
    virtual Result ValidateImageViewInfo(const ImageViewInfo& viewInfo) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result ValidateFmaskViewInfo(const FmaskViewInfo& info) const override;

    // NOTE: Part of the public IDevice interface.
    Result ValidateSamplerInfo(const SamplerInfo& info) const override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetBorderColorPaletteSize(
        const BorderColorPaletteCreateInfo& createInfo,
        Result*                             pResult) const override
        { return (m_pGfxDevice == nullptr) ? 0 : m_pGfxDevice->GetBorderColorPaletteSize(createInfo, pResult); }

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateBorderColorPalette(
        const BorderColorPaletteCreateInfo& createInfo,
        void*                               pPlacementAddr,
        IBorderColorPalette**               ppPalette) const override
    {
        return (m_pGfxDevice == nullptr) ? Result::ErrorUnavailable :
                m_pGfxDevice->CreateBorderColorPalette(createInfo, pPlacementAddr, ppPalette);
    }

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetComputePipelineSize(
        const ComputePipelineCreateInfo& createInfo,
        Result*                          pResult) const override
        { return (m_pGfxDevice == nullptr) ? 0 : m_pGfxDevice->GetComputePipelineSize(createInfo, pResult); }

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateComputePipeline(
        const ComputePipelineCreateInfo& createInfo,
        void*                            pPlacementAddr,
        IPipeline**                      ppPipeline) override
    {
        return (m_pGfxDevice == nullptr) ? Result::ErrorUnavailable :
                m_pGfxDevice->CreateComputePipeline(createInfo, pPlacementAddr,
                                                    createInfo.flags.clientInternal, ppPipeline);
    }

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetShaderLibrarySize(
        const ShaderLibraryCreateInfo& createInfo,
        Result*                        pResult) const override
    {
        return (m_pGfxDevice == nullptr) ? 0 : m_pGfxDevice->GetMaybeArchiveLibrarySize(createInfo, pResult);
    }

    virtual Result CreateShaderLibrary(
        const ShaderLibraryCreateInfo& createInfo,
        void*                          pPlacementAddr,
        IShaderLibrary**               ppLibrary) override
    {
        return (m_pGfxDevice == nullptr) ? Result::ErrorUnavailable :
                m_pGfxDevice->CreateMaybeArchiveLibrary(createInfo, pPlacementAddr,
                                                        createInfo.flags.clientInternal, ppLibrary);
    }

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetGraphicsPipelineSize(
        const GraphicsPipelineCreateInfo& createInfo,
        Result*                           pResult) const override
        { return (m_pGfxDevice == nullptr) ? 0 : m_pGfxDevice->GetGraphicsPipelineSize(createInfo, false, pResult); }

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateGraphicsPipeline(
        const GraphicsPipelineCreateInfo& createInfo,
        void*                             pPlacementAddr,
        IPipeline**                       ppPipeline) override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetMsaaStateSize() const override
    {
        return (m_pGfxDevice == nullptr) ? 0 : m_pGfxDevice->GetMsaaStateSize();
    }

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateMsaaState(
        const MsaaStateCreateInfo& createInfo,
        void*                      pPlacementAddr,
        IMsaaState**               ppMsaaState) const override
    {
        return (m_pGfxDevice == nullptr) ? Result::ErrorUnavailable :
                m_pGfxDevice->CreateMsaaState(createInfo, pPlacementAddr, ppMsaaState);
    }

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetColorBlendStateSize() const override
    {
        return (m_pGfxDevice == nullptr) ? 0 : m_pGfxDevice->GetColorBlendStateSize();
    }

    // NOTE: Part of the public IDevice interface.
    virtual bool CanEnableDualSourceBlend(
        const ColorBlendStateCreateInfo& createInfo) const override
    {
        return (m_pGfxDevice == nullptr) ? false : m_pGfxDevice->CanEnableDualSourceBlend(createInfo);
    }

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateColorBlendState(
        const ColorBlendStateCreateInfo& createInfo,
        void*                            pPlacementAddr,
        IColorBlendState**               ppColorBlendState) const override
    {
        return (m_pGfxDevice == nullptr) ? Result::ErrorUnavailable :
                m_pGfxDevice->CreateColorBlendState(createInfo, pPlacementAddr, ppColorBlendState);
    }

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetDepthStencilStateSize() const override
    {
        return (m_pGfxDevice == nullptr) ? 0 : m_pGfxDevice->GetDepthStencilStateSize();
    }

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateDepthStencilState(
        const DepthStencilStateCreateInfo& createInfo,
        void*                              pPlacementAddr,
        IDepthStencilState**               ppDepthStencilState) const override
    {
        return (m_pGfxDevice == nullptr) ? Result::ErrorUnavailable :
                m_pGfxDevice->CreateDepthStencilState(createInfo, pPlacementAddr, ppDepthStencilState);
    }

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetQueueSemaphoreSize(
        const QueueSemaphoreCreateInfo& createInfo,
        Result*                         pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateQueueSemaphore(
        const QueueSemaphoreCreateInfo& createInfo,
        void*                           pPlacementAddr,
        IQueueSemaphore**               ppQueueSemaphore) override;

    virtual bool IsNativeFenceSupported() const = 0;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetSharedQueueSemaphoreSize(
        const QueueSemaphoreOpenInfo& openInfo,
        Result*                       pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result OpenSharedQueueSemaphore(
        const QueueSemaphoreOpenInfo& openInfo,
        void*                         pPlacementAddr,
        IQueueSemaphore**             ppQueueSemaphore) override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetExternalSharedQueueSemaphoreSize(
        const ExternalQueueSemaphoreOpenInfo& openInfo,
        Result*                               pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result OpenExternalSharedQueueSemaphore(
        const ExternalQueueSemaphoreOpenInfo& openInfo,
        void*                                 pPlacementAddr,
        IQueueSemaphore**                     ppQueueSemaphore) override;

#if ( PAL_AMDGPU_BUILD)
    // NOTE: Part of the public IDevice interface.
    virtual Result QueryGpuMemoryBudgetInfo(
        GpuMemoryBudgetInfo* pInfo) override { return Result::Unsupported; }
#endif

    Result CreateInternalFence(
        const FenceCreateInfo& createInfo,
        Fence**                ppFence) const;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetGpuEventSize(
        const GpuEventCreateInfo& createInfo,
        Result*                   pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateGpuEvent(
        const GpuEventCreateInfo& createInfo,
        void*                     pPlacementAddr,
        IGpuEvent**               ppGpuEvent) override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetQueryPoolSize(
        const QueryPoolCreateInfo& createInfo,
        Result*                    pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateQueryPool(
        const QueryPoolCreateInfo& createInfo,
        void*                      pPlacementAddr,
        IQueryPool**               ppQueryPool) const override;

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetCmdAllocatorSize(
        const CmdAllocatorCreateInfo& createInfo,
        Result*                       pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateCmdAllocator(
        const CmdAllocatorCreateInfo& createInfo,
        void*                         pPlacementAddr,
        ICmdAllocator**               ppCmdAllocator) override;

    Result CreateInternalCmdAllocator(
        const CmdAllocatorCreateInfo& createInfo,
        CmdAllocator**                ppCmdAllocator);

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetCmdBufferSize(
        const CmdBufferCreateInfo& createInfo,
        Result*                    pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateCmdBuffer(
        const CmdBufferCreateInfo& createInfo,
        void*                      pPlacementAddr,
        ICmdBuffer**               ppCmdBuffer) override;

    Result CreateInternalCmdBuffer(
        const CmdBufferCreateInfo&         createInfo,
        const CmdBufferInternalCreateInfo& internalInfo,
        CmdBuffer**                        ppCmdBuffer);

    // NOTE: Part of the public IDevice interface.
    virtual size_t GetPerfExperimentSize(
        const PerfExperimentCreateInfo& createInfo,
        Result*                         pResult) const override
        { return (m_pGfxDevice == nullptr) ? 0 : m_pGfxDevice->GetPerfExperimentSize(createInfo, pResult); }

    // NOTE: Part of the public IDevice interface.
    virtual Result CreatePerfExperiment(
        const PerfExperimentCreateInfo& createInfo,
        void*                           pPlacementAddr,
        IPerfExperiment**               ppPerfExperiment) const override
    {
        return (m_pGfxDevice == nullptr) ? Result::ErrorUnavailable :
               m_pGfxDevice->CreatePerfExperiment(createInfo, pPlacementAddr, ppPerfExperiment);
    }

    virtual size_t GetIndirectCmdGeneratorSize(
        const IndirectCmdGeneratorCreateInfo& createInfo,
        Result*                               pResult) const override;

    virtual Result CreateIndirectCmdGenerator(
        const IndirectCmdGeneratorCreateInfo& createInfo,
        void*                                 pPlacementAddr,
        IIndirectCmdGenerator**               ppGenerator) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result GetPrivateScreens(
        uint32*          pNumScreens,
        IPrivateScreen** ppScreens) override;

    // NOTE: Part of the public IDevice interface.
    virtual void GetPrivateScreenImageSizes(
        const PrivateScreenImageCreateInfo& createInfo,
        size_t*                             pImageSize,
        size_t*                             pGpuMemorySize,
        Result*                             pResult) const override;

    // NOTE: Part of the public IDevice interface.
    virtual Result CreatePrivateScreenImage(
        const PrivateScreenImageCreateInfo& createInfo,
        void*                               pImagePlacementAddr,
        void*                               pGpuMemoryPlacementAddr,
        IImage**                            ppImage,
        IGpuMemory**                        ppGpuMemory) override;

    // NOTE: Part of the public IDevice interface.
    virtual Result SetSamplePatternPalette(
        const SamplePatternPalette& palette) override
    {
        return (m_pGfxDevice == nullptr) ? Result::ErrorUnavailable :
               m_pGfxDevice->SetSamplePatternPalette(palette);
    }

    // NOTE: Part of the public IDevice interface.
    virtual Result CreateVirtualDisplay(
        const VirtualDisplayInfo& virtualDisplayInfo,
        uint32*                   pScreenTargetId) override
        { return Result::Unsupported; }

    // NOTE: Part of the public IDevice interface.
    virtual Result DestroyVirtualDisplay(
        uint32     screenTargetId) override
        { return Result::Unsupported; }

    // NOTE: Part of the public IDevice interface.
    virtual Result GetVirtualDisplayProperties(
        uint32                    screenTargetId,
        VirtualDisplayProperties* pProperties) override
        { return Result::Unsupported; }

    // NOTE: Part of the public IDevice interface.
    virtual bool DetermineHwStereoRenderingSupported(
        const GraphicPipelineViewInstancingInfo& viewInstancingInfo) const override;

    // NOTE: Part of the public IDevice interface.
    virtual const char* GetCacheFilePath() const override
        { return m_cacheFilePath; }

    // NOTE: Part of the public IDevice interface.
    virtual const char* GetDebugFilePath() const override
        { return m_debugFilePath; }

    // NOTE: Part of the public IDevice interface.
    virtual Result SetStaticVmidMode(
        bool enable) override;

    virtual Result InitBusAddressableGpuMemory(
        IQueue*           pQueue,
        uint32            gpuMemCount,
        IGpuMemory*const* ppGpuMemList) override
    {
        return Result::Unsupported;
    }

    virtual Result AddGpuMemoryReferences(
        uint32              gpuMemRefCount,
        const GpuMemoryRef* pGpuMemoryRefs,
        IQueue*             pQueue,
        uint32              flags
        ) override;

    virtual Result RemoveGpuMemoryReferences(
        uint32            gpuMemoryCount,
        IGpuMemory*const* ppGpuMemory,
        IQueue*           pQueue
        ) override;

    virtual void GetReferencedMemoryTotals(
        gpusize  referencedGpuMemTotal[GpuHeapCount]) const override;

    static Result ValidateBindObjectMemoryInput(
        const IGpuMemory* pMemObject,
        gpusize           offset,
        gpusize           objMemSize,
        gpusize           objAlignment,
        bool              allowVirtualBinding = false);

    // Requests the device to reserve the GPU VA partition.
    virtual Result ReserveGpuVirtualAddress(VaPartition             vaPartition,
                                            gpusize                 vaStartAddress,
                                            gpusize                 vaSize,
                                            bool                    isVirtual,
                                            VirtualGpuMemAccessMode virtualAccessMode,
                                            gpusize*                pGpuVirtAddr) = 0;

    // Requests the device to free the GPU VA range.
    virtual Result FreeGpuVirtualAddress(gpusize vaStartAddress, gpusize vaSize) = 0;

    // Requests to reserve the GPU VA partition on all devices for SVM.
    virtual Result ReserveGpuVirtualAddressSvm(gpusize vaStartAddress,
                                               gpusize vaSize);

    // Requests the device to free the GPU VA range on all devices for SVM.
    virtual Result FreeGpuVirtualAddressSvm(gpusize vaStartAddress, gpusize vaSize);

    // Checks if this GPU is the master in a group of linked GPU's.
    virtual bool IsMasterGpu() const = 0;

    GfxDevice* GetGfxDevice() const { return m_pGfxDevice; }
    const AddrMgr* GetAddrMgr() const { return m_pAddrMgr;   }

    const GpuMemoryHeapProperties& HeapProperties(GpuHeap heap) const { return m_heapProperties[heap]; }
    gpusize HeapLogicalSize(GpuHeap heap) const { return HeapProperties(heap).logicalSize; }

    const GpuMemoryProperties& MemoryProperties() const { return m_memoryProperties; }
    const GpuEngineProperties& EngineProperties() const { return m_engineProperties; }
    const GpuQueueProperties&  QueueProperties()  const { return m_queueProperties;  }
    const GpuChipProperties&   ChipProperties()   const { return m_chipProperties;   }
    const HwsInfo& GetHwsInfo() const { return m_hwsInfo; }
    const PerfExperimentProperties& PerfProperties() const { return m_perfExperimentProperties; }
    const Extent3d& MaxImageDimension() const { return m_chipProperties.imageProperties.maxImageDimension; }

    InternalMemMgr* MemMgr() { return &m_memMgr; }

    // Returns the internal tracked command allocator except for engines that do not support tracking.
    CmdAllocator* InternalCmdAllocator(EngineType engineType) const
        { return m_pTrackedCmdAllocator; }

    // Returns the internal untracked command allocator for queue context specific use.
    CmdAllocator* InternalUntrackedCmdAllocator() const { return m_pUntrackedCmdAllocator; }

    const PalSettings& Settings() const;
    Util::MetroHash::Hash GetSettingsHash() const;

    Platform* GetPlatform() const;

    ADDR_HANDLE AddrLibHandle() const { return GetAddrMgr()->AddrLibHandle(); }

    uint32 MaxQueueSemaphoreCount() const { return m_maxSemaphoreCount; }

    // Helper method to index into the format support info table.
    FormatFeatureFlags FeatureSupportFlags(ChNumFormat format, ImageTiling tiling) const
    {
        return m_pFormatPropertiesTable->features[static_cast<uint32>(format)][tiling != ImageTiling::Linear];
    }

    bool SupportStateShadowingByCpFw() const
    {
        return m_chipProperties.gfx9.stateShadowingByCpFw;
    }

    bool SupportStateShadowingByCpFwUserAlloc() const
    {
        return m_chipProperties.gfx9.stateShadowingByCpFwUserAlloc;
    }

    bool SupportsStaticVmid() const
        { return m_chipProperties.gfxip.supportStaticVmid; }

    // Checks if a format/tiling-type pairing supports shader image-read operations.
    bool SupportsImageRead(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureImageShaderRead) != 0); }

    // Checks if a format/tiling-type pairing supports shader image-write operations.
    bool SupportsImageWrite(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureImageShaderWrite) != 0); }

    // Checks if a format/tiling-type pairing supports copy operations.
    bool SupportsCopy(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureCopy) != 0); }

    // Checks if a format/tiling-type pairing supports format conversion operations.
    bool SupportsFormatConversion(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureFormatConversion) != 0); }

    // Checks if a format/tiling-type pairing supports format conversion operations as the source image.
    bool SupportsFormatConversionSrc(ChNumFormat srcFormat, ImageTiling srcTiling) const
        { return ((FeatureSupportFlags(srcFormat, srcTiling) & FormatFeatureFormatConversionSrc) != 0); }

    // Checks if a format/tiling-type pairing supports format conversion operations as the destination image.
    bool SupportsFormatConversionDst(ChNumFormat dstFormat, ImageTiling dstTiling) const
        { return ((FeatureSupportFlags(dstFormat, dstTiling) & FormatFeatureFormatConversionDst) != 0); }

    // Checks if a format/tiling-type pairing supports shader read operations for buffer SRDs of this format.
    bool SupportsMemoryViewRead(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureMemoryShaderRead) != 0); }

    // Checks if a format/tiling-type pairing supports shader write operations for buffer SRDs of this format.
    bool SupportsMemoryViewWrite(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureMemoryShaderWrite) != 0); }

    // Checks if a format/tiling-type pairing supports color target operations.
    bool SupportsColor(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureColorTargetWrite) != 0); }

    // Checks if a format/tiling-type pairing supports color blend operations.
    bool SupportsBlend(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureColorTargetBlend) != 0); }

    // Checks if a format/tiling-type pairing supports depth target operations.
    bool SupportsDepth(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureDepthTarget) != 0); }

    // Checks if a format/tiling-type pairing supports stencil target operations.
    bool SupportsStencil(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureStencilTarget) != 0); }

    // Checks if a format/tiling-type pairing supports multisampled target operations.
    bool SupportsMsaa(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureMsaaTarget) != 0); }

    // Checks if a format/tiling-type pairing supports windowed-mode present operations.
    bool SupportsWindowedPresent(ChNumFormat format, ImageTiling tiling) const
        { return ((FeatureSupportFlags(format, tiling) & FormatFeatureWindowedPresent) != 0); }

    // Checks if a format is supported.
    bool SupportsFormat(ChNumFormat format) const
    {
        return ((FeatureSupportFlags(format, ImageTiling::Linear)  != 0) ||
                (FeatureSupportFlags(format, ImageTiling::Optimal) != 0));
    }

    virtual Result AddQueue(Queue* pQueue);
    void RemoveQueue(Queue* pQueue);

    Engine* GetEngine(EngineType type, uint32 engineIndex)
        { return m_pEngines[type][engineIndex]; }

    Result CreateEngine(EngineType engineType, uint32 engineIndex);

    CmdStream* GetDummyCommandStream(EngineType engineType)
        { return m_pDummyCommandStreams[engineType]; }

    // Override per-device settings as needed
    virtual void OverrideDefaultSettings(PalSettings* pSettings) const {}

    // Helper function to call client provided private screen destroy time callback.
    void PrivateScreenDestroyNotication(void* pOwner) const
        { m_finalizeInfo.privateScreenNotifyInfo.pfnOnDestroy(pOwner); }

    void DeveloperCb(Developer::CallbackType type, void* pCbData) const
        { m_pPlatform->DeveloperCb(m_deviceIndex, type, pCbData); }

    // Determines the start (inclusive) and end (exclusive) virtual addresses for the specified virtual address range.
    void VirtualAddressRange(VaPartition vaPartition, gpusize* pStartVirtAddr, gpusize* pEndVirtAddr) const;

    // Chooses a VA partition based on the given VaRange enum.
    VaPartition ChooseVaPartition(VaRange range, bool isVirtual) const;

    const SettingsLoader* const GetSettingsLoader() const { return m_pSettingsLoader; }

    virtual bool IsNull() const { return false; }

    bool IsUsingAutoPriorityForInternalAllocations() const
        { return m_memoryProperties.flags.autoPrioritySupport & m_finalizeInfo.flags.internalGpuMemAutoPriority; }

    bool IsPreemptionSupported(EngineType engineType) const
        { return m_engineProperties.perEngine[engineType].flags.supportsMidCmdBufPreemption; }

    bool IsConstantEngineSupported(EngineType engineType) const
        { return (m_engineProperties.perEngine[engineType].flags.constantEngineSupport != 0); }

    // Returns whether any pixel-wait-sync-plus feature can be enabled.
    bool UsePws(EngineType engineType) const;

    // Returns whether the pixel-wait-sync-plus late acquire point feature can be enabled.
    bool UsePwsLateAcquirePoint(EngineType engineType) const;

    const char* GetDumpDirName() const { return m_cmdBufDumpPath; }

    bool IsMultiVf() const;

#if PAL_ENABLE_PRINTS_ASSERTS
    bool IsCmdBufDumpEnabledViaHotkey() const { return m_cmdBufDumpEnabledViaHotkey; }
#else
    bool IsCmdBufDumpEnabledViaHotkey() const { return false; }
#endif
    uint32 GetFrameCount() const { return m_frameCnt; }
    void IncFrameCount();

    ImageTexOptLevel TexOptLevel() const { return m_texOptLevel; }

    void ApplyDevOverlay(const IImage& dstImage, CmdBuffer* pCmdBuffer) const;

    bool PhysicalEnginesAvailable() const { return m_flags.physicalEnginesAvailable; }

    static bool EngineSupportsCompute(EngineType  engineType);
    static bool EngineSupportsGraphics(EngineType  engineType);

    const BoundGpuMemory& GetDummyChunkMem() const { return m_dummyChunkMem; }

    bool DisableSwapChainAcquireBeforeSignalingClient() const { return m_disableSwapChainAcquireBeforeSignaling; }

    void SetHdrColorspaceFormat(ScreenColorSpace newFormat) { m_hdrColorspaceFormat = newFormat; }

    bool UsingHdrColorspaceFormat() const;

    const PalPublicSettings* GetPublicSettings() const
        { return static_cast<const PalPublicSettings*>(&m_publicSettings); }

    virtual bool ValidatePipelineUploadHeap(const GpuHeap& preferredHeap) const;

    // Add or subtract some memory from our per-heap totals. We refcount each added GPU memory object so it's safe
    // to add memory multiple times or subtract it multiple times.
    Result AddToReferencedMemoryTotals(
        uint32              gpuMemRefCount,
        const GpuMemoryRef* pGpuMemoryRefs);

    Result SubtractFromReferencedMemoryTotals(
        uint32            gpuMemoryCount,
        IGpuMemory*const* ppGpuMemory,
        bool              forceSubtract);

    bool    IsSpoofed() const { return m_chipProperties.hwIpFlags.isSpoofed != 0; }
    IfhMode GetIfhMode() const;

    // Helper for creating DmaUploadRing for PAL internal use.
    virtual Result CreateDmaUploadRing() = 0;

    Result AcquireRingSlot(UploadRingSlot* pSlotId);

    size_t UploadUsingEmbeddedData(
        UploadRingSlot  slotId,
        Pal::GpuMemory* pDst,
        gpusize         dstOffset,
        size_t          bytes,
        void**          ppEmbeddedData);

    Result SubmitDmaUploadRing(
        UploadRingSlot    slotId,
        UploadFenceToken* pCompletionFence,
        uint64            pagingFenceVal);

    Result WaitForPendingUpload(
        Pal::Queue* pWaiter,
        UploadFenceToken fenceValue);

    bool ShouldUploadUsingDma(GpuHeap pipelineHeapType) const;

    virtual bool IsHwEmulationEnabled() const { return false; }

    bool HasLargeBar() const;

    virtual bool KernelSupportCpuHostAperture() const { return false;}

    bool IssueSqttMarkerEvents() const;
    bool IssueCrashAnalysisMarkerEvents() const;
    bool EnablePerfCountersInPreamble() const;

    const FlglState& GetFlglState() const { return m_flglState; }

    virtual Result RegisterHipRuntimeState(const HipRuntimeSetup& runtimeState) const override
    {
        return Result::Unsupported;
    }

    virtual Result SetHipTrapHandler(
        const IGpuMemory* pTrapHandlerCode,
        gpusize           codeOffset,
        const IGpuMemory* pTrapHandlerMemory,
        gpusize           memoryOffset) const override
    {
        return Result::Unsupported;
    }

    virtual bool ImagePrefersCloneCopy(
        const ImageCreateInfo& createInfo) const override;

    void LogCodeObjectToDisk(
        Util::StringView<char> prefix,
        Util::StringView<char> name,
        PipelineHash           hash,
        bool                   isInternal,
        const void*            pCodeObject,
        size_t                 codeObjectLen) const;

    bool IsFinalized() const { return m_deviceFinalized; }
    size_t NumQueues() const { return m_queues.NumElements(); }
    uint32 AttachedScreenCount() const { return m_attachedScreenCount; }

    bool EnableDisplayDcc(const DisplayDccCaps& dccCaps, const SwizzledFormat& swizzledFormat) const;

    void InitOutputPaths();

protected:
    Device(
        Platform*              pPlatform,
        uint32                 deviceIndex,
        uint32                 attachedScreenCount,
        size_t                 deviceSize,
        const HwIpDeviceSizes& hwDeviceSizes,
        uint32                 maxSemaphoreCount);

    static bool DetermineGpuIpLevels(
        uint32            familyId,       // AMDGPU Family ID.
        uint32            eRevId,         // AMDGPU Revision ID.
        uint32            cpMicrocodeVersion,
        const Platform*   pPlatform,
        HwIpLevels*       pIpLevels);

    static void GetHwIpDeviceSizes(
        const HwIpLevels& ipLevels,
        HwIpDeviceSizes*  pHwDeviceSizes,
        size_t*           pAddrMgrSize);

    void InitPerformanceRatings();
    void InitMemoryHeapProperties();
    Result InitSettings();

    virtual Result OsEarlyInit() { return Result::Success; }
    virtual Result OsLateInit()  { return Result::Success; }

    virtual Result OsSetStaticVmidMode(
        bool enable) = 0;

    virtual void OsFinalizeSettings() { }

    // Responsible for setting up some of this GPU's queue properties which are based on settings.
    virtual void FinalizeQueueProperties() = 0;
    void FinalizeMemoryHeapProperties();

    virtual Result ProbeGpuVaRange(
        gpusize     vaStart,
        gpusize     vaSize,
        VaPartition vaPartition) const
        { return Result::Success; }

    Result FindGpuVaRangeReverse(
        gpusize*    pVaStart,
        gpusize     vaEnd,
        gpusize     vaSize,
        gpusize     vaAlignment,
        VaPartition vaParttion) const;

    Result FindGpuVaRange(
        gpusize*    pStartVaAddr,
        gpusize     vaEnd,
        gpusize     vaSize,
        gpusize     vaAlignment,
        VaPartition vaParttion,
        bool        reserveCpuVa = false) const;

    Result FixupUsableGpuVirtualAddressRange(uint32 vaRangeNumBits);

    // Queries the size of a Queue object, in bytes. This can return zero if a Queue type is unsupported.
    virtual size_t QueueObjectSize(
        const QueueCreateInfo& createInfo) const = 0;

    // Queries the size of a MultiQueue object, in bytes. This can return zero if a Queue type is unsupported.
    virtual size_t MultiQueueObjectSize(
        uint32                 queueCount,
        const QueueCreateInfo* pCreateInfo) const
    {
        return 0;
    }

    // Constructs a new Queue object in preallocated memory.
    virtual Queue* ConstructQueueObject(
        const QueueCreateInfo& createInfo,
        void*                  pPlacementAddr) = 0;

    // Constructs a new MutltiQueue object in preallocated memory.
    virtual Queue* ConstructMultiQueueObject(
        uint32                 queueCount,
        const QueueCreateInfo* pCreateInfo,
        void*                  pPlacementAddr)
    {
        return nullptr;
    }

    // Constructs a new CmdBuffer object in preallocated memory.
    Result ConstructCmdBuffer(
        const CmdBufferCreateInfo& createInfo,
        void*                      pPlacementAddr,
        CmdBuffer**                ppCmdBuffer) const;

    Result CreateCmdBufferHelper(
        const CmdBufferCreateInfo&         createInfo,
        const CmdBufferInternalCreateInfo& internalInfo,
        void*                              pPlacementAddr,
        CmdBuffer**                        ppCmdBuffer) const;

    // Constructs a new GpuMemory object in preallocated memory.
    virtual GpuMemory* ConstructGpuMemoryObject(
        void* pPlacementAddr) = 0;

    virtual Result EnumPrivateScreensInfo(
        uint32* pNumScreens) = 0;

    uint32 GetDeviceIndex() const
        { return m_deviceIndex; }

#if defined(__unix__)
    virtual void GetModifiersList(
        ChNumFormat format,
        uint32*     pModifierCount,
        uint64*     pModifiersList) const override {}
#endif

    Platform*      m_pPlatform;
    InternalMemMgr m_memMgr;

    // An array stores enumerated private screens info and only m_connectedPrivateScreens out of them are valid.
    PrivateScreenCreateInfo m_privateScreenInfo[MaxPrivateScreens];
    // Number of enumerated private screens, which means: first m_connectedPrivateScreens slots have valid information
    // in m_privateScreenInfo[] and m_connectedPrivateScreens out of MaxPrivateScreens slots are valid objects in
    // m_pPrivateScreens[].
    uint32          m_connectedPrivateScreens;
    // Number of emulated private screens, which will merged to m_connectedPrivateScreens after enumeration.
    uint32          m_emulatedPrivateScreens;
    // A counter starts from 0xFFFFFFFF to represent virtual target id or index for emulated private screen.
    uint32          m_emulatedTargetId;
    // An array stores currently valid private screen objects, note that this array is not necessarily ordered the same
    // as m_privateScreenInfo[] as this might be sparsely populated.
    PrivateScreen*  m_pPrivateScreens[MaxPrivateScreens];
    // An array stores emulated private screen objects.
    PrivateScreen*  m_pEmulatedPrivateScreens[MaxPrivateScreens];
    // Count of screen which are attached to the device.
    uint32          m_attachedScreenCount;

    DeviceFinalizeInfo m_finalizeInfo;
    GfxDevice*         m_pGfxDevice;

    GpuUtil::TextWriter<Platform>*     m_pTextWriter;
    uint32                             m_devDriverClientId;

    FlglState                          m_flglState;

    GpuMemoryProperties                m_memoryProperties;
    GpuEngineProperties                m_engineProperties;
    GpuQueueProperties                 m_queueProperties;
    GpuChipProperties                  m_chipProperties;
    HwsInfo                            m_hwsInfo;
    PerfExperimentProperties           m_perfExperimentProperties;
    const MergedFormatPropertiesTable* m_pFormatPropertiesTable;
    GpuMemoryHeapProperties            m_heapProperties[GpuHeapCount];

    Engine*       m_pEngines[EngineTypeCount][MaxAvailableEngines];

    CmdStream*    m_pDummyCommandStreams[EngineTypeCount];

    char  m_gpuName[MaxDeviceName];

    bool  m_deviceFinalized;    // Set if the client has ever call Finalize().

#if PAL_ENABLE_PRINTS_ASSERTS
    bool  m_settingsCommitted;           // Set if the client has ever called CommitSettingsAndInit().
    bool  m_cmdBufDumpEnabledViaHotkey;  // Command buffer dumping is enabled on the next frame
#endif

    const bool m_force32BitVaSpace;  // Forces 32 bit virtual address space

    // We must keep a list of queues that have been created so that we can ask them to remove retired memory references
    // when necessary
    Util::IntrusiveList<Queue> m_queues;
    Util::Mutex                m_queueLock; // Serializes all queue-list operations

    // Gpu memory that contains a special srd used for debugging unbound descriptors.
    BoundGpuMemory m_pageFaultDebugSrdMem;

    // Dummy memory used when DeviceLost happened.
    BoundGpuMemory m_dummyChunkMem;

    BigSoftwareReleaseInfo m_bigSoftwareRelease;   // Big Software (BigSW) Release Version

    VirtualDisplayCapabilities m_virtualDisplayCaps; // Virtual display capabilities

    struct
    {
        uint32 physicalEnginesAvailable :  1;  // Client enabled 1 or more physical engines during device finalization.
        uint32 reserved                 : 31;
    } m_flags;

    bool m_disableSwapChainAcquireBeforeSignaling;

    PalPublicSettings      m_publicSettings;
    SettingsLoader*        m_pSettingsLoader;

    Util::SettingsFileMgr<Platform>  m_settingsMgr;

    // Get*FilePath need to return a persistent storage
    char m_cacheFilePath[Util::MaxPathStrLen];
    char m_debugFilePath[Util::MaxPathStrLen];

    Util::Mutex    m_dmaUploadRingLock;
    DmaUploadRing* m_pDmaUploadRing;

    uint32         m_staticVmidRefCount;
    char m_cmdBufDumpPath[Util::MaxPathStrLen];

private:
    Result HwlEarlyInit();
    void   InitPageFaultDebugSrd();
    Result InitDummyChunkMem();
    Result CreateInternalCmdAllocators();

    Result SetupPublicSettingDefaults();

    Result CreateEngines(const DeviceFinalizeInfo& finalizeInfo);

    Result CreateDummyCommandStreams();

    std::chrono::nanoseconds GetTimeoutValueInNs(std::chrono::nanoseconds appTimeout) const;

    typedef Util::HashMap<IGpuMemory*, uint32, Pal::Platform>  MemoryRefMap;

    MemoryRefMap  m_referencedGpuMem;
    Util::Mutex   m_referencedGpuMemLock;
    gpusize       m_referencedGpuMemBytes[GpuHeapCount];

    AddrMgr*               m_pAddrMgr;
    CmdAllocator*          m_pTrackedCmdAllocator;
    CmdAllocator*          m_pUntrackedCmdAllocator;
    const uint32           m_deviceIndex;       // Unique index of this GPU compared to all other GPUs in the system.
    const size_t           m_deviceSize;
    const HwIpDeviceSizes  m_hwDeviceSizes;
    const uint32           m_maxSemaphoreCount; // The OS-specific GPU semaphore max signal count.
    volatile uint32        m_frameCnt;  // Device frame count
    ImageTexOptLevel       m_texOptLevel; // Client specified texture optimize level for internally-created views
    ScreenColorSpace       m_hdrColorspaceFormat;  // Current HDR Colorspace Format

    PAL_DISALLOW_DEFAULT_CTOR(Device);
    PAL_DISALLOW_COPY_AND_ASSIGN(Device);
};

#if PAL_BUILD_GFX
// Helper struct which contains the version numbers for the MEC, ME, and PFP firmwares.
struct CpFwVersions
{
    uint32 mec;
    uint32 me;
    uint32 pfp;
};

// Tests the MEC, ME, and PFP firmware version against minimums for all three and returns true if all three are
// at least the specified minimum.
extern bool TestCpFwVersions(
    const GpuChipProperties& chipProperties,
    const CpFwVersions&      versions);
#endif

// NOTE: Below are prototypes for several utility functions for each HWIP namespace in PAL. These functions are for
// determining what IP level of a particular HWIP block (GFXIP, etc.) a GPU supports, as well as initializing
// the GPU chip properties for a particular version of a HWIP block. Each HWIP namespace in PAL corresponds to one
// hardware layer for that group of IP levels. Each HWIP namespace must have the following function:
//
// IpLevel DetermineIpLevel(familyId, eRevId, ...);
// * This function is used to determine the IP level that the appropriate hardware layer decides the GPU device
//   has. For hardware which is not supported by a specific hardware layer, this function will return an IP level
//   indicating the HWIP block is absent. The "..." in the argument list is to indicate that some HWL's may require
//   additional parameters beyond the typical family & revision IDs to determine the supported IP level.
//
// Each GFXIP namespace must also have the following functions:
//
// const FormatPropertiesTable* GetFormatPropertiesTable();
// * This function is used to select the proper format properties table. These tables are static so the Device
//   class can save this pointer to a member variable in its constructor.
//
// Result InitializeGpuChipProperties(GpuChipProperties*);
// * This function is used to setup default values for the fields in the GpuChipProperties structure for certain
//   GFXIP levels. Some kernel-mode drivers provide explicit overrides for these values which will be overridden
//   in the OS-specific children of the Device class.
//
// void FinalizeGpuChipProperties(GpuChipProperties*);
// * This function is used to finalize any values of GpuChipProperties which can be derived from some of the other
//   fields initialized in InitializeGpuChipProperties() or overridden by the kernel-mode driver.
//
// void InitializePerfExperimentProperties(const GpuChipProperties&, PerfExperimentProperties*);
// * This function is used to setup default values for the fields in the PerfExperimentProperties structure for
//   certain GFXIP levels.

namespace Gfx9
{
// Determines the GFXIP level of an GFXIP 9+ GPU.
extern IpTriple DetermineIpLevel(
    uint32 familyId,    // Hardware Family ID.
    uint32 eRevId,      // Software Revision ID.
    uint32 microcodeVersion);

// Gets the static format properties table for GFXIP 9+ hardware.
extern const MergedFormatPropertiesTable* GetFormatPropertiesTable(
    GfxIpLevel                 gfxIpLevel,
    const PalPlatformSettings& settings);

extern void InitPerfCtrInfo(
    const Device& device,
    GpuChipProperties* pProps);

extern void InitializePerfExperimentProperties(
    const GpuChipProperties&  chipProps,
    PerfExperimentProperties* pProperties);

// Initialize default values for the GPU chip properties for GFXIP9+ hardware.
extern void InitializeGpuChipProperties(
    const Platform*    pPlatform,
    uint32             cpUcodeVersion,
    GpuChipProperties* pInfo);

// Finalize default values for the GPU chip properties for GFXIP9+ hardware.
extern void FinalizeGpuChipProperties(
    const Device&      device,
    GpuChipProperties* pInfo);

// Initialize default values for the GPU engine properties for GFXIP9+ hardware.
extern void InitializeGpuEngineProperties(
    const GpuChipProperties&  chipProps,
    GpuEngineProperties*      pInfo);
}

#if PAL_BUILD_GFX12
namespace Gfx12
{
// Determines the GFXIP level of an GFXIP GPU that supports Gfx12.
extern IpTriple DetermineIpLevel(
    uint32 familyId,    // Hardware Family ID.
    uint32 eRevId);     // Software Revision ID.

// Gets the static format properties table for GFX12 hardware.
extern const MergedFormatPropertiesTable* GetFormatPropertiesTable(
    GfxIpLevel gfxIpLevel);

// Initialize default values for the GPU chip properties for hardware that supports Gfx12.
extern void InitializeGpuChipProperties(
    const Platform*    pPlatform,
    GpuChipProperties* pInfo);

// Finalize default values for the GPU chip properties for hardware that supports Gfx12.
extern void FinalizeGpuChipProperties(
    const Device&      device,
    GpuChipProperties* pInfo);

// Initialize default values for the GPU engine properties for hardware that supports Gfx12.
extern void InitializeGpuEngineProperties(
    const GpuChipProperties&  chipProps,
    GpuEngineProperties*      pInfo);

extern void InitPerfCtrInfo(
    const Device& device,
    GpuChipProperties* pProps);

extern void InitializePerfExperimentProperties(
    const GpuChipProperties&  chipProps,
    PerfExperimentProperties* pProperties);
} // namespace Gfx12
#endif

// ASIC family and chip identification functions
#if PAL_BUILD_GFX12
constexpr bool IsGfx12(GfxIpLevel gfxLevel)
{
    return (gfxLevel == GfxIpLevel::GfxIp12);
}

inline bool IsGfx12(const Device& device)
{
    return IsGfx12(device.ChipProperties().gfxLevel);
}
#endif

#if PAL_BUILD_GFX12
constexpr bool IsGfx12Plus(GfxIpLevel gfxLevel)
{
    return  false
#if PAL_BUILD_GFX12
            || IsGfx12(gfxLevel)
#endif
    ;
}

inline bool IsGfx12Plus(const Device& device)
{
    return IsGfx12Plus(device.ChipProperties().gfxLevel);
}
#endif

constexpr bool IsGfx11(GfxIpLevel gfxLevel)
{
    return (gfxLevel == GfxIpLevel::GfxIp11_0)
           || (gfxLevel == GfxIpLevel::GfxIp11_5)
            ;
}

inline bool IsGfx11(const Device& device)
{
    return IsGfx11(device.ChipProperties().gfxLevel);
}

inline bool IsGfx11Plus(GfxIpLevel gfxLevel)
{
    return IsGfx11(gfxLevel)
#if PAL_BUILD_GFX12
           || IsGfx12(gfxLevel)
#endif
           ;
}

inline bool IsGfx11Plus(const Device& device)
{
    return IsGfx11(device.ChipProperties().gfxLevel)
#if PAL_BUILD_GFX12
           || IsGfx12(device.ChipProperties().gfxLevel)
#endif
           ;
}

constexpr bool IsGfx110(GfxIpLevel gfxLevel)
{
    return (gfxLevel == GfxIpLevel::GfxIp11_0);
}

inline bool IsGfx110(const Device& device)
{
    return IsGfx110(device.ChipProperties().gfxLevel);
}

inline bool IsNavi31(const Device& device)
{
    return AMDGPU_IS_NAVI31(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}

inline bool IsNavi32(const Device& device)
{
    return AMDGPU_IS_NAVI32(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}

inline bool IsNavi33(const Device& device)
{
    return AMDGPU_IS_NAVI33(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}

inline bool IsNavi3x(const Device& device)
{
    return (device.ChipProperties().familyId == FAMILY_NV3);
}

inline bool IsPhoenix1(const Device& device)
{
    return AMDGPU_IS_PHOENIX1(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}

inline bool IsPhoenix2(const Device& device)
{
    return AMDGPU_IS_PHOENIX2(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}

inline bool IsPhoenixFamily(const Device& device)
{
    return FAMILY_IS_PHX(device.ChipProperties().familyId);
}

#if PAL_BUILD_HAWK_POINT1
inline bool IsHawkPoint1(const Device& device)
{
    return AMDGPU_IS_HAWK_POINT1(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
#endif
#if PAL_BUILD_HAWK_POINT2
inline bool IsHawkPoint2(const Device& device)
{
    return AMDGPU_IS_HAWK_POINT2(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
#endif

#if PAL_BUILD_GFX12
inline bool IsNavi4x(const Device& device)
{
    return (device.ChipProperties().familyId == FAMILY_NV4);
}

#if PAL_BUILD_NAVI48
inline bool IsNavi48(const Device& device)
{
    return AMDGPU_IS_NAVI48(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
#endif

#if PAL_BUILD_NAVI44
inline bool IsNavi44(const Device& device)
{
    return AMDGPU_IS_NAVI44(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
#endif
#endif

constexpr bool IsGfx10(GfxIpLevel gfxLevel)
{
    return ((gfxLevel == GfxIpLevel::GfxIp10_1) || (gfxLevel == GfxIpLevel::GfxIp10_3));
}
inline bool IsGfx10(const Device& device)
{
    return IsGfx10(device.ChipProperties().gfxLevel);
}

constexpr bool IsGfx115(GfxIpLevel gfxLevel)
{
    return (gfxLevel == GfxIpLevel::GfxIp11_5);
}
inline bool IsGfx115(const Device& device)
{
    return IsGfx115(device.ChipProperties().gfxLevel);
}

inline bool IsStrixFamily(const Device& device)
{
    return FAMILY_IS_STX(device.ChipProperties().familyId);
}
inline bool IsStrix1(const Device& device)
{
    return AMDGPU_IS_STRIX1(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
#if PAL_BUILD_STRIX_HALO
inline bool IsStrixHalo(const Device& device)
{
    return AMDGPU_IS_STRIX_HALO(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
#endif

// Gfx10 / Navi1x
inline bool IsNavi10(const Device& device)
{
    return AMDGPU_IS_NAVI10(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
inline bool IsNavi12(const Device& device)
{
    return AMDGPU_IS_NAVI12(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
inline bool IsNavi14(const Device& device)
{
    return AMDGPU_IS_NAVI14(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
inline bool IsNavi1x(const Device& device)
{
    return (IsNavi10(device) || IsNavi12(device) || IsNavi14(device));
}

// Gfx10.3 / Navi2x
inline bool IsNavi21(const Device& device)
{
    return AMDGPU_IS_NAVI21(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
inline bool IsNavi22(const Device& device)
{
    return AMDGPU_IS_NAVI22(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
inline bool IsNavi23(const Device& device)
{
    return AMDGPU_IS_NAVI23(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
inline bool IsNavi24(const Device& device)
{
    return AMDGPU_IS_NAVI24(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
inline bool IsNavi2x(const Device& device)
{
    return (IsNavi21(device) || IsNavi22(device) || IsNavi23(device) || IsNavi24(device));
}
inline bool IsRembrandt(const Device& device)
{
    return AMDGPU_IS_REMBRANDT(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
inline bool IsRaphael(const Device& device)
{
    return AMDGPU_IS_RAPHAEL(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}
inline bool IsMendocino(const Device& device)
{
    return AMDGPU_IS_MENDOCINO(device.ChipProperties().familyId, device.ChipProperties().eRevId);
}

constexpr bool IsGfx103(GfxIpLevel gfxLevel)
{
    return (gfxLevel == GfxIpLevel::GfxIp10_3);
}
inline bool IsGfx103(const Device& device)
{
    return IsGfx103(device.ChipProperties().gfxLevel);
}
inline bool IsGfx103Plus(GfxIpLevel gfxLevel)
{
    return (gfxLevel > GfxIpLevel::GfxIp10_1);
}
inline bool IsGfx103Plus(const Device& device)
{
    return IsGfx103Plus(device.ChipProperties().gfxLevel);
}
constexpr bool IsGfx103PlusExclusive(GfxIpLevel gfxLevel)
{
    return (gfxLevel > GfxIpLevel::GfxIp10_1);
}
inline bool IsGfx103PlusExclusive(const Device& device)
{
    return IsGfx103PlusExclusive(device.ChipProperties().gfxLevel);
}
constexpr bool IsGfx103CorePlus(GfxIpLevel gfxLevel)
{
    return (gfxLevel >= GfxIpLevel::GfxIp10_3);
}
inline bool IsGfx103CorePlus(const Device& device)
{
    return (IsGfx103CorePlus(device.ChipProperties().gfxLevel));
}
constexpr bool IsGfx101(GfxIpLevel gfxLevel)
{
    return (gfxLevel == GfxIpLevel::GfxIp10_1);
}
inline bool IsGfx101(const Device& device)
{
    return IsGfx101(device.ChipProperties().gfxLevel);
}
inline bool IsGfx10Bard(const Device& device)
{
    return (false
            );
}

// The gfx9 HWL covers multiple GFXIPs, this gives us a way to distinguish between "gfx9 HW" and "gfx9 HWL".
constexpr bool IsGfx9Hwl(GfxIpLevel gfxLevel)
{
    return (IsGfx10(gfxLevel) || IsGfx11(gfxLevel));
}
inline bool IsGfx9Hwl(const Device& device)
{
    return IsGfx9Hwl(device.ChipProperties().gfxLevel);
}

} // Pal
