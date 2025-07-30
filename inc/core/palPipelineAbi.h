/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
/**
 ***********************************************************************************************************************
 * @file  palPipelineAbi.h
 * @brief PAL Pipeline ABI enums and structures defining the Pipeline ABI spec.
 ***********************************************************************************************************************
 */

#pragma once

#include "palInlineFuncs.h"
#include "palUtil.h"
#include "palElf.h"
#include <cstring>

namespace Util
{
namespace Abi
{

constexpr uint8  ElfOsAbiAmdgpuHsa = 64;        ///< ELFOSABI_AMDGPU_HSA
constexpr uint8  ElfOsAbiAmdgpuPal = 65;        ///< ELFOSABI_AMDGPU_PAL
constexpr uint8  ElfAbiVersionAmdgpuHsaV2 = 0;  ///< ELFABIVERSION_AMDGPU_HSA_V2
constexpr uint8  ElfAbiVersionAmdgpuHsaV3 = 1;  ///< ELFABIVERSION_AMDGPU_HSA_V3
constexpr uint8  ElfAbiVersionAmdgpuHsaV4 = 2;  ///< ELFABIVERSION_AMDGPU_HSA_V4
constexpr uint8  ElfAbiVersionAmdgpuHsaV5 = 3;  ///< ELFABIVERSION_AMDGPU_HSA_V5
constexpr uint8  ElfAbiVersionAmdgpuPal   = 0;  ///< ELFABIVERSION_AMDGPU_PAL

constexpr uint32 MetadataNoteType                = 32;   ///< NT_AMDGPU_METADATA
constexpr uint64 PipelineShaderBaseAddrAlignment = 256;  ///< Base address alignment for shader stage entry points on
                                                         ///  AMD GPUs.
constexpr uint64 DataMinBaseAddrAlignment        = 32;   ///< Minimum base address alignment for Data section.
constexpr uint64 RoDataMinBaseAddrAlignment      = 32;   ///< Minimum base address alignment for RoData section.

constexpr const char AmdGpuVendorName[]         = "AMD";     ///< Vendor name string.
constexpr const char AmdGpuArchName[]           = "AMDGPU";  ///< Architecture name string.

/// AmdGpuMachineType for the EF_AMDGPU_MACH selection mask in e_flags.
enum class AmdGpuMachineType : uint8
{
    GfxNone = 0x00,  ///< EF_AMDGPU_MACH_NONE
#if PAL_CLIENT_INTERFACE_MAJOR_VERSION < 888
    Gfx600  = 0x20,  ///< EF_AMDGPU_MACH_AMDGCN_GFX600
    Gfx601  = 0x21,  ///< EF_AMDGPU_MACH_AMDGCN_GFX601
    Gfx602  = 0x3a,  ///< EF_AMDGPU_MACH_AMDGCN_GFX602
    Gfx700  = 0x22,  ///< EF_AMDGPU_MACH_AMDGCN_GFX700
    Gfx701  = 0x23,  ///< EF_AMDGPU_MACH_AMDGCN_GFX701
    Gfx702  = 0x24,  ///< EF_AMDGPU_MACH_AMDGCN_GFX702
    Gfx703  = 0x25,  ///< EF_AMDGPU_MACH_AMDGCN_GFX703
    Gfx704  = 0x26,  ///< EF_AMDGPU_MACH_AMDGCN_GFX704
    Gfx705  = 0x3B,  ///< EF_AMDGPU_MACH_AMDGCN_GFX705
    Gfx800  = 0x27,  ///< EF_AMDGPU_MACH_AMDGCN_GFX800
    Gfx801  = 0x28,  ///< EF_AMDGPU_MACH_AMDGCN_GFX801
    Gfx802  = 0x29,  ///< EF_AMDGPU_MACH_AMDGCN_GFX802
    Gfx803  = 0x2a,  ///< EF_AMDGPU_MACH_AMDGCN_GFX803
    Gfx805  = 0x3c,  ///< EF_AMDGPU_MACH_AMDGCN_GFX805
    Gfx810  = 0x2b,  ///< EF_AMDGPU_MACH_AMDGCN_GFX810
    Gfx900  = 0x2c,  ///< EF_AMDGPU_MACH_AMDGCN_GFX900
    Gfx902  = 0x2d,  ///< EF_AMDGPU_MACH_AMDGCN_GFX902
    Gfx904  = 0x2e,  ///< EF_AMDGPU_MACH_AMDGCN_GFX904
    Gfx906  = 0x2f,  ///< EF_AMDGPU_MACH_AMDGCN_GFX906
    Gfx909  = 0x31,  ///< EF_AMDGPU_MACH_AMDGCN_GFX909
    Gfx90C  = 0x32,  ///< EF_AMDGPU_MACH_AMDGCN_GFX90C
#endif
    Gfx1010 = 0x33,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1010
    Gfx1011 = 0x34,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1011
    Gfx1012 = 0x35,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1012
    Gfx1030 = 0x36,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1030
    Gfx1031 = 0x37,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1031
    Gfx1032 = 0x38,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1032
    Gfx1034 = 0x3e,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1034
    Gfx1035 = 0x3d,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1035
    Gfx1036 = 0x45,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1036
    Gfx1100 = 0x41,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1100
    Gfx1101 = 0x46,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1101
    Gfx1102 = 0x47,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1102
    Gfx1103 = 0x44,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1103
    Gfx1150 = 0x43,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1150
#if PAL_BUILD_STRIX_HALO
    Gfx1151 = 0x4A,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1151
#endif
#if PAL_BUILD_NAVI44
    Gfx1200   = 0x4D,  ///< IDK
#endif
#if PAL_BUILD_NAVI48
    Gfx1201   = 0x4E,  ///< EF_AMDGPU_MACH_AMDGCN_GFX1200
#endif
};

/// AmdGpuFeatureV4Type for the feature selection mask bits in e_flags.
enum class AmdGpuFeatureV4Type : uint8
{
    Unsupported = 0x00, ///< EF_AMDGPU_FEATURE_*_UNSUPPORTED_V4
    Any         = 0x01, ///< EF_AMDGPU_FEATURE_*_ANY_V4
    Off         = 0x02, ///< EF_AMDGPU_FEATURE_*_OFF_V4
    On          = 0x03, ///< EF_AMDGPU_FEATURE_*_ON_V4
};

/// Enumerates the stepping values for each GPU supported by PAL (and by PAL's ABI).  There are many duplicates
/// in this list, because values are commonly re-used across different GFXIP major/minor versions.
enum GfxIpStepping : uint16
{
#if PAL_CLIENT_INTERFACE_MAJOR_VERSION < 888
    // GFXIP 6.0.x steppings:
    GfxIpSteppingTahiti    = 0,
    GfxIpSteppingPitcairn  = 1,
    GfxIpSteppingCapeVerde = 1,
    GfxIpSteppingOland     = 2,
    GfxIpSteppingHainan    = 2,

    // GFXIP 7.0.x steppings:
    GfxIpSteppingKaveri    = 0,
    GfxIpSteppingHawaiiPro = 1,
    GfxIpSteppingHawaii    = 2,
    GfxIpSteppingKalindi   = 3,
    GfxIpSteppingBonaire   = 4,
    GfxIpSteppingGodavari  = 5,

    // GFXIP 8.0.x steppings:
    GfxIpSteppingCarrizo  = 1,
    GfxIpSteppingIceland  = 2,
    GfxIpSteppingTonga    = 2,
    GfxIpSteppingFiji     = 3,
    GfxIpSteppingPolaris  = 3,
    GfxIpSteppingTongaPro = 5,

    // GFXIP 8.1.x steppings:
    GfxIpSteppingStoney = 0,

    // GFXIP 9.0.x steppings:
    GfxIpSteppingVega10 = 0,
    GfxIpSteppingRaven  = 2,
    GfxIpSteppingVega12 = 4,
    GfxIpSteppingVega20 = 6,
    GfxIpSteppingRaven2 = 9,
    GfxIpSteppingRenoir = 12,
#endif

    // GFXIP 10.1.x steppings:
    GfxIpSteppingNavi10        = 0,
    GfxIpSteppingNavi12        = 1,
    GfxIpSteppingNavi14        = 2,

    // GFXIP 10.3.x steppings:
    GfxIpSteppingNavi21        = 0,
    GfxIpSteppingNavi22        = 1,
    GfxIpSteppingNavi23        = 2,
    GfxIpSteppingNavi24        = 4,
    GfxIpSteppingRembrandt     = 5,
    GfxIpSteppingRaphael       = 6, // Also Mendocino

    // GFXIP 11.0.x steppings:
    GfxIpSteppingNavi31        = 0,
    GfxIpSteppingNavi32        = 1,
    GfxIpSteppingNavi33        = 2,
    GfxIpSteppingPhoenix       = 3,

    // GFXIP 11.5.x steppings:
    GfxIpSteppingStrix         = 0,
#if PAL_BUILD_STRIX_HALO
    GfxIpSteppingStrixHalo     = 1,
#endif

#if PAL_BUILD_NAVI44
    GfxIpSteppingNavi44        = 0,
#endif

#if PAL_BUILD_NAVI48
    GfxIpSteppingNavi48        = 1,
#endif
};

/// Name of the section where our pipeline binaries store the disassembly for all shader stages.
constexpr const char AmdGpuDisassemblyName[] = ".AMDGPU.disasm";

/// Name prefix of the section where our pipeline binaries store extra information e.g. LLVM IR.
constexpr const char AmdGpuCommentName[] = ".AMDGPU.comment.";

/// Name of the section where our pipeline binaries store AMDIL disassembly.
constexpr const char AmdGpuCommentAmdIlName[] = ".AMDGPU.comment.amdil";

/// Name of the section where our pipeline binaries store LLVMIR disassembly.
constexpr const char AmdGpuCommentLlvmIrName[] = ".AMDGPU.comment.llvmir";

/// String table of the Pipeline ABI symbols.
constexpr const char* PipelineAbiSymbolNameStrings[] =
{
    "unknown",
    "_amdgpu_ls_main",
    "_amdgpu_hs_main",
    "_amdgpu_es_main",
    "_amdgpu_gs_main",
    "_amdgpu_vs_main",
    "_amdgpu_ps_main",
    "_amdgpu_cs_main",
    "_amdgpu_ls_shdr_intrl_tbl",
    "_amdgpu_hs_shdr_intrl_tbl",
    "_amdgpu_es_shdr_intrl_tbl",
    "_amdgpu_gs_shdr_intrl_tbl",
    "_amdgpu_vs_shdr_intrl_tbl",
    "_amdgpu_ps_shdr_intrl_tbl",
    "_amdgpu_cs_shdr_intrl_tbl",
    "_amdgpu_ps_export_shader_shdr_intrl_tbl",
    "_amdgpu_ps_export_shader_dual_source_shdr_intrl_tbl",
    "_amdgpu_ls_disasm",
    "_amdgpu_hs_disasm",
    "_amdgpu_es_disasm",
    "_amdgpu_gs_disasm",
    "_amdgpu_vs_disasm",
    "_amdgpu_ps_disasm",
    "_amdgpu_cs_disasm",
    "_amdgpu_ps_export_shader_disasm",
    "_amdgpu_ps_export_shader_dual_source_disasm",
    "_amdgpu_ls_shdr_intrl_data",
    "_amdgpu_hs_shdr_intrl_data",
    "_amdgpu_es_shdr_intrl_data",
    "_amdgpu_gs_shdr_intrl_data",
    "_amdgpu_vs_shdr_intrl_data",
    "_amdgpu_ps_shdr_intrl_data",
    "_amdgpu_cs_shdr_intrl_data",
    "_amdgpu_pipeline_intrl_data",
    "_amdgpu_cs_amdil",
    "_amdgpu_task_amdil",
    "_amdgpu_vs_amdil",
    "_amdgpu_hs_amdil",
    "_amdgpu_ds_amdil",
    "_amdgpu_gs_amdil",
    "_amdgpu_mesh_amdil",
    "_amdgpu_ps_amdil",
    "_amdgpu_reserved38",
    "_amdgpu_reserved39",
    "_amdgpu_reserved40",
    "color_export_shader",
    "color_export_shader_dual_source",
};

/// Pipeline category.
enum PipelineType : uint32
{
    VsPs = 0,
    Gs,
    Cs,
    Ngg,
    Tess,
    GsTess,
    NggTess,
    Mesh,
    TaskMesh,
};

/// Helper enum which is used along with the @ref PipelineSymbolType and @ref PipelineMetadataType to
/// easily find a particular piece of metadata or symbol for any hardware shader stage.
/// @note: The order of these stages must match the order used for each stages' symbol type or metadata
/// type!
enum class HardwareStage : uint32
{
    Ls = 0, ///< Hardware LS stage
    Hs,     ///< Hardware hS stage
    Es,     ///< Hardware ES stage
    Gs,     ///< Hardware GS stage
    Vs,     ///< Hardware VS stage
    Ps,     ///< Hardware PS stage
    Cs,     ///< Hardware CS stage
    Count
};

/// HardwareStage enum to string conversion table.
constexpr const char* HardwareStageStrings[] =
{
    "LS",
    "HS",
    "ES",
    "GS",
    "VS",
    "PS",
    "CS",
    "INVALID",
};

static_assert(Util::ArrayLen32(HardwareStageStrings) == static_cast<uint32>(HardwareStage::Count) + 1,
              "HardwareStageStrings is not the same size as HardwareStage enum!");

/// Helper enum which is used along with the @ref GetMetadataHashForApiShader function to easily find
/// a metadata hash dword for a particular API shader type.
enum class ApiShaderType : uint32
{
    Cs = 0, ///< API compute shader
    Task,   ///< API task shader
    Vs,     ///< API vertex shader
    Hs,     ///< API hull shader
    Ds,     ///< API domain shader
    Gs,     ///< API geometry shader
    Mesh,   ///< API mesh shader
    Ps,     ///< API pixel shader
    Count
};

// Helper enum defined shader subType
enum class ApiShaderSubType : uint32
{
    Unknown = 0,
    Traversal,
    RayGeneration,
    Intersection,
    AnyHit,
    ClosestHit,
    Miss,
    Callable,
    LaunchKernel,           ///< Raytracing launch kernel
    Count
};

/// Used to represent hardware shader stage.
enum HardwareStageFlagBits : uint32
{
    HwShaderLs = (1 << uint32(HardwareStage::Ls)),
    HwShaderHs = (1 << uint32(HardwareStage::Hs)),
    HwShaderEs = (1 << uint32(HardwareStage::Es)),
    HwShaderGs = (1 << uint32(HardwareStage::Gs)),
    HwShaderVs = (1 << uint32(HardwareStage::Vs)),
    HwShaderPs = (1 << uint32(HardwareStage::Ps)),
    HwShaderCs = (1 << uint32(HardwareStage::Cs)),
};

/// Used along with the symbol name strings to identify the symbol type.
enum class PipelineSymbolType : uint32
{
    Unknown = 0,       ///< A custom symbol not defined by the Pipeline ABI.
    LsMainEntry,       ///< Hardware LS entry point.  Must be aligned to hardware requirements.
    HsMainEntry,       ///< Hardware HS entry point.  Must be aligned to hardware requirements.
    EsMainEntry,       ///< Hardware ES entry point.  Must be aligned to hardware requirements.
    GsMainEntry,       ///< Hardware GS entry point.  Must be aligned to hardware requirements.
    VsMainEntry,       ///< Hardware VS entry point.  Must be aligned to hardware requirements.
    PsMainEntry,       ///< Hardware PS entry point.  Must be aligned to hardware requirements.
    CsMainEntry,       ///< Hardware CS entry point.  Must be aligned to hardware requirements.
    LsShdrIntrlTblPtr, ///< LS shader internal table pointer.  Optional.  Described in Per-Shader Internal Table.
    HsShdrIntrlTblPtr, ///< HS shader internal table pointer.  Optional.  Described in Per-Shader Internal Table.
    EsShdrIntrlTblPtr, ///< ES shader internal table pointer.  Optional.  Described in Per-Shader Internal Table.
    GsShdrIntrlTblPtr, ///< GS shader internal table pointer.  Optional.  Described in Per-Shader Internal Table.
    VsShdrIntrlTblPtr, ///< VS shader internal table pointer.  Optional.  Described in Per-Shader Internal Table.
    PsShdrIntrlTblPtr, ///< PS shader internal table pointer.  Optional.  Described in Per-Shader Internal Table.
    CsShdrIntrlTblPtr, ///< CS shader internal table pointer.  Optional.  Described in Per-Shader Internal Table.
    PsExportShaderShdrIntrlTblPtr, ///< PS export shader internal table pointer.  Optional.  Described in Per-Shader Internal Table.
    PsExportShaderDualSourceShdrIntrlTblPtr, ///< PS export shader with dual source on internal table pointer.  Optional.  Described in Per-Shader Internal Table.
    LsDisassembly,     ///< Hardware LS disassembly.  Optional.  Associated with the .AMDGPU.disasm section.
    HsDisassembly,     ///< Hardware HS disassembly.  Optional.  Associated with the .AMDGPU.disasm section.
    EsDisassembly,     ///< Hardware ES disassembly.  Optional.  Associated with the .AMDGPU.disasm section.
    GsDisassembly,     ///< Hardware GS disassembly.  Optional.  Associated with the .AMDGPU.disasm section.
    VsDisassembly,     ///< Hardware VS disassembly.  Optional.  Associated with the .AMDGPU.disasm section.
    PsDisassembly,     ///< Hardware PS disassembly.  Optional.  Associated with the .AMDGPU.disasm section.
    CsDisassembly,     ///< Hardware CS disassembly.  Optional.  Associated with the .AMDGPU.disasm section.
    PsExportShaderDisassembly, ///< Hardware PS export shader disassembly.  Optional.  Associated with the .AMDGPU.disasm section.
    PsExportShaderDualSourceDisassembly, ///< Hardware PS export shader with dual source on disassembly.  Optional.  Associated with the .AMDGPU.disasm section.
    LsShdrIntrlData,   ///< LS shader internal data pointer.  Optional.
    HsShdrIntrlData,   ///< HS shader internal data pointer.  Optional.
    EsShdrIntrlData,   ///< ES shader internal data pointer.  Optional.
    GsShdrIntrlData,   ///< GS shader internal data pointer.  Optional.
    VsShdrIntrlData,   ///< VS shader internal data pointer.  Optional.
    PsShdrIntrlData,   ///< PS shader internal data pointer.  Optional.
    CsShdrIntrlData,   ///< CS shader internal data pointer.  Optional.
    PipelineIntrlData, ///< Cross-shader internal data pointer.  Optional.
    CsAmdIl,           ///< API CS shader AMDIL disassembly.  Optional.
                       ///  Associated with the .AMDGPU.comment.amdil section.
    TaskAmdIl,         ///< API Task shader AMDIL disassembly. Optional.
                       ///  Associated with the .AMDGPU.commd.amdil section.
    VsAmdIl,           ///< API VS shader AMDIL disassembly.  Optional.
                       ///  Associated with the .AMDGPU.comment.amdil section.
    HsAmdIl,           ///< API HS shader AMDIL disassembly.  Optional.
                       ///  Associated with the .AMDGPU.comment.amdil section.
    DsAmdIl,           ///< API DS shader AMDIL disassembly.  Optional.
                       ///  Associated with the .AMDGPU.comment.amdil section.
    GsAmdIl,           ///< API GS shader AMDIL disassembly.  Optional.
                       ///  Associated with the .AMDGPU.comment.amdil section.
    MeshAmdIl,         ///< API Mesh shader AMDIL disassembly. Optional.
                       ///  Associated with the .AMDGPU.commd.amdil section.
    PsAmdIl,           ///< API PS shader AMDIL disassembly.  Optional.
                       ///  Associated with the .AMDGPU.comment.amdil section.
    Reserved38,
    Reserved39,
    Reserved40,
    PsColorExportEntry,///< PS color export shader entry point. Optional.
    PsColorExportDualSourceEntry,///< PS color export shader with dual source on entry point. Optional.
    Count,

    ShaderMainEntry   = LsMainEntry,        ///< Shorthand for the first shader's entry point
    ShaderIntrlTblPtr = LsShdrIntrlTblPtr,  ///< Shorthand for the first shader's internal table pointer
    ShaderDisassembly = LsDisassembly,      ///< Shorthand for the first shader's disassembly string
    ShaderIntrlData   = LsShdrIntrlData,    ///< Shorthand for the first shader's internal data pointer
    ShaderAmdIl       = CsAmdIl,            ///< Shorthand for the first shader's AMDIL disassembly string
};

static_assert(static_cast<uint32>(PipelineSymbolType::Count) == sizeof(PipelineAbiSymbolNameStrings)/sizeof(char*),
              "PipelineSymbolType enum does not match PipelineAbiSymbolNameStrings.");

static_assert(static_cast<uint32>(HardwareStage::Count) <= (sizeof(uint8) * 8),
              "A mask of HardwareStage values will no longer fit into a uint8!");

/// This packed bitfield is used to correlate the @ref ApiShaderType enum with the @ref HardwareStage enum.
union ApiHwShaderMapping
{
    uint8 apiShaders[static_cast<uint32>(ApiShaderType::Count)];

    struct
    {
        uint32 u32Lo;   ///< Low 32 bits of this structure.
        uint32 u32Hi;   ///< High 32 bits of this structure.
    };

    uint64 u64All;      ///< Flags packed as 64-bit uint.
};

static_assert((sizeof(ApiHwShaderMapping) == sizeof(uint64)),
              "ApiHwShaderMapping is different in size than expected!");

/// This packed bitfield is used to set sample Info to register
union ApiSampleInfo
{
    struct
    {
        uint16 numSamples;       ///< Number of coverage samples
        uint16 samplePatternIdx; ///< Index into the currently bound MSAA sample pattern table
    };

    uint32 u32All;      ///< Flags packed as 32-bit uint.
};

/// This packed bitfield is used to set UserDataMapping::CompositeData
union ApiCompositeDataValue
{
    struct
    {
        uint32 primInfo           : 2; ///< Number of vertex per primitive
        uint32 numSamples         : 5; ///< Number of coverage samples
        uint32 dynamicSourceBlend : 1; ///< Whether to enable dynamic dual source blend.
        uint32 rasterStream       : 3; ///< Which vertex stream to rasterize. Reserved for future.
        uint32 reserved           : 21;
    };

    uint32 u32All; ///< Flags packed as 32-bit uint.
};

/// Helper function to get a pipeline symbol type for a specific hardware shader stage.
///
/// @param [in] symbolType Type of Pipeline Symbol to retrieve
/// @param [in] stage      Hardware shader stage of interest
///
/// @returns PipelineSymbolType enum associated with the base symbol type and hardware stage.
constexpr PipelineSymbolType GetSymbolForStage(
    PipelineSymbolType symbolType,
    HardwareStage      stage)
{
    return static_cast<PipelineSymbolType>(static_cast<uint32>(symbolType) + static_cast<uint32>(stage));
}

/// Helper function to get a pipeline symbol type for a specific API shader stage.
///
/// @param [in] symbolType Type of Pipeline Symbol to retrieve
/// @param [in] stage      API shader stage of interest
///
/// @returns PipelineSymbolType enum associated with the base symbol type and API stage.
constexpr PipelineSymbolType GetSymbolForStage(
    PipelineSymbolType symbolType,
    ApiShaderType      stage)
{
    return static_cast<PipelineSymbolType>(static_cast<uint32>(symbolType) + static_cast<uint32>(stage));
}

/// Helper function to get the symbol type when given a symbol name.
///
/// @param [in] pName The symbol name.
///
/// @returns The corresponding PipelineSymbolType.
inline PipelineSymbolType GetSymbolTypeFromName(const char* pName)
{
    PipelineSymbolType type = PipelineSymbolType::Unknown;
    for (uint32 i = 0; i < static_cast<uint32>(PipelineSymbolType::Count); i++)
    {
        if (strcmp(PipelineAbiSymbolNameStrings[i], pName) == 0)
        {
            type = static_cast<PipelineSymbolType>(i);
            break;
        }
    }

    return type;
}

/// User data entries can map to physical user data registers.  UserDataMapping describes the
/// content of the registers.
enum class UserDataMapping : uint32
{
    GlobalTable       = 0x10000000, ///< 32-bit pointer to GPU memory containing the global internal table.
    PerShaderTable    = 0x10000001, ///< 32-bit pointer to GPU memory containing the per-shader internal table.
    SpillTable        = 0x10000002, ///< 32-bit pointer to GPU memory containing the user data spill table.
    BaseVertex        = 0x10000003, ///< Vertex offset (32-bit unsigned integer). Not needed if the pipeline doesn't
                                    ///  reference the draw index in the vertex shader. Only supported by the first
                                    ///  stage in a graphics pipeline.
    BaseInstance      = 0x10000004, ///< Instance offset (32-bit unsigned integer). Only supported by the first stage in
                                    ///  a graphics pipeline.
    DrawIndex         = 0x10000005, ///< Draw index (32-bit unsigned integer). Only supported by the first stage in a
                                    ///  graphics pipeline.
    Workgroup         = 0x10000006, ///< Thread group count (32-bit unsigned integer). Low half of a 64-bit address of
                                    ///  a buffer containing the grid dimensions for a Compute dispatch operation. The
                                    ///  high half of the address is stored in the next sequential user-SGPR. Only
                                    ///  supported by compute pipelines.
    EsGsLdsSize       = 0x1000000A, ///< Indicates that PAL will program this user-SGPR to contain the amount of LDS
                                    ///  space used for the ES/GS pseudo-ring-buffer for passing data between shader
                                    ///  stages.
    ViewId            = 0x1000000B, ///< View id (32-bit unsigned integer) identifies a view of graphic
                                    ///  pipeline instancing.
    StreamOutTable    = 0x1000000C, ///< 32-bit pointer to GPU memory containing the stream out target SRD table.  This
                                    ///  can only appear for one shader stage per pipeline.
    PerShaderPerfData = 0x1000000D, ///< 32-bit pointer to GPU memory containing the per-shader performance data buffer.
    VertexBufferTable = 0x1000000F, ///< 32-bit pointer to GPU memory containing the vertex buffer SRD table.  This can
                                    ///  only appear for one shader stage per pipeline.
#if PAL_CLIENT_INTERFACE_MAJOR_VERSION < 904
    UavExportTable    = 0x10000010, ///< 32-bit pointer to GPU memory containing the UAV export SRD table.  This can
                                    ///  only appear for one shader stage per pipeline (PS). These replace color targets
                                    ///  and are completely separate from any UAVs used by the shader. This is optional,
                                    ///  and only used by the PS when UAV exports are used to replace color-target
                                    ///  exports to optimize specific shaders.
#endif
    NggCullingData    = 0x10000011, ///< 64-bit pointer to GPU memory containing the hardware register data needed by
                                    ///  some NGG pipelines to perform culling.  This value contains the address of the
                                    ///  first of two consecutive registers which provide the full GPU address.
    MeshTaskDispatchDims  = 0x10000012,  ///< Offset to three consecutive registers which indicate the number of
                                         ///  threadgroups dispatched in the X, Y, and Z dimensions.
    MeshTaskRingIndex     = 0x10000013,  ///< Index offset (32-bit unsigned integer). Indicates the index into the
                                         ///  Mesh/Task shader rings for the shader to consume.
    TaskDispatchIndex     = DrawIndex,   ///< Dispatch index (32-bit unsigned integer). Only supported by the first
                                         ///  stage (task shader stage) in a hybrid graphics pipeline.
    MeshPipeStatsBuf      = 0x10000014,  ///< 32-bit GPU virtual address of a buffer storing the shader-emulated mesh
                                         ///  pipeline stats query.
    StreamOutControlBuf   = 0x10000016, ///< 32-bit GPU virtual address to the streamout control buffer for GPUs that
                                        ///  use software-emulated streamout.
    EnPrimsNeededCnt      = 0x10000017,  ///< Address of userdata register that will be used to dynamically enable/disable
                                         ///  extra shader work for generated prim counts in PipelineStats queries
    SampleInfo            = 0x10000018,  ///< Sample Info, 16-bit numsamples + 16-bit Sample Pattern
    ColorExportAddr       = 0x10000020,  ///< 32-bit pointer to GPU memory containing the color export shader
    DynamicDualSrcBlendInfo = 0x10000022, ///< 32-bit dynamicDualSourceBlend info

    CompositeData           = 0x10000023, ///< The composite structure that includes sample info, DynamicDualSrcBlendInfo
                                          ///   and topology. It can be valid for various shader stages.

    // Range of values for a user data PAL metadata register to be resolved at pipeline create time in PAL.
    // PipelineLinkStart+N is initialized by PAL to the (low 32 bits of the) address of symbol _amdgpu_pipelineLinkN
    // in any ELF piece of the pipeline. For odd N, the symbol is optional; if it is not present, the register is
    // initialized to 0. For even N, the symbol is not optional; if it is not present, PAL pipeline creation fails
    // with an error.
    // The semantics of the various values of N are internal to the compiler implementation, as it generates both
    // the ELF defining the symbol and the ELF using the user data SGPR.
    PipelineLinkStart      = 0x10001000,
    PipelineLinkEnd        = 0x100010FF,

    NotMapped              = 0xFFFFFFFF,  ///< Register is not mapped to any user-data entry.

    /// @internal The following enum values are deprecated and only remain in the header file to avoid build errors.

    GdsRange          = 0x10000007, ///< GDS range (32-bit unsigned integer: gdsSizeInBytes | (gdsOffsetInBytes << 16)).
                                    ///  Only supported by compute pipelines.
    BaseIndex         = 0x10000008, ///< Index offset (32-bit unsigned integer). Only supported by the first stage in a
                                    ///  graphics pipeline.
    Log2IndexSize     = 0x10000009, ///< Base-2 logarithm of the size of each index buffer entry.
    IndirectTableLow  = 0x20000000, ///< Low range of 32-bit pointer to GPU memory containing the
                                    ///  address of the indirect user data table.
                                    ///  Subtract 0x20000000.
    IndirectTableHigh = 0x2FFFFFFF, ///< High range of 32-bit pointer to GPU memory containing the
                                    ///  address of the indirect user data table.
                                    ///  Subtract 0x20000000.
};

// Symbol name prefix for pipeline link symbol.
static constexpr char AmdGpuPipelineLinkPrefix[] = "_amdgpu_pipelineLink";

/// The ABI section type.  The Code (.text), and Data (.data)
/// sections are the main sections interacted with in the Pipeline ABI.
enum class AbiSectionType : uint32
{
    Undefined = 0, ///< An unassociated section
    Code,          ///< The code (.text) section containing executable machine code for all shader stages.
    Data,          ///< Data section
    Disassembly,   ///< Disassembly section
    AmdIl,         ///< AMDIL section
    LlvmIr         ///< LLVMIR section
};

/// These Relocation types are specific to the AMDGPU target machine architecture.
/// Relocation computation notation:
/// A   - The addend used to compute the value of the relocatable field. In Rel sections the addend
///       is obtained from the original value of the word being relocated. In Rela sections an
///       explicit field for a full-width addend is provided.
/// B   - The base address at which a shared object is loaded into memory during execution.
///       Generally, a shared object file is built with a base virtual address of 0.
///       However, the execution address of the shared object is different.
///       NOTE: As PAL does not know the base address until runtime this value
///             has to be externally provided when applying relocations.
/// G   - Represents the offset into the global offset table at which the
///       relocation entry's symbol will reside during execution.
/// GOT - Represents the address of the global offset table.
/// P   - The section offset or address of the storage unit being relocated, computed using r_offset.
/// S   - The value of the symbol whose index resides in the relocation entry.
/// Z   - The size of the symbol whose index resides in the relocation entry.
/// SEE: https://llvm.org/docs/AMDGPUUsage.html#relocation-records
/// for AMDGPU defined relocations.
enum class RelocationType : uint32
{
    None = 0,     ///< val: 0  | field: none   | calc: none
    Abs32Lo,      ///< val: 1  | field: word32 | calc: (S + A) & 0xFFFFFFFF
    Abs32Hi,      ///< val: 2  | field: word32 | calc: (S + A) >> 32
    Abs64,        ///< val: 3  | field: word64 | calc: S + A
    Rel32,        ///< val: 4  | field: word32 | calc: S + A - P
    Rel64,        ///< val: 5  | field: word64 | calc: S + A - P
    Abs32,        ///< val: 6  | field: word32 | calc: S + A
    GotPcRel,     ///< val: 7  | field: word32 | calc: G + GOT + A - P
    GotPcRel32Lo, ///< val: 8  | field: word32 | calc: (G + GOT + A - P) & 0xFFFFFFFF
    GotPcRel32Hi, ///< val: 9  | field: word32 | calc: (G + GOT + A - P) >> 32
    Rel32Lo,      ///< val: 10 | field: word32 | calc: (S + A - P) & 0xFFFFFFFF
    Rel32Hi,      ///< val: 11 | field: word32 | calc: (S + A - P) >> 32
    Rel16 = 14,   ///< val: 14 | field: word16 | calc: ((S + A - P) - 4) / 4
};

/// Contains only the relevant info for a pipeline symbol.
struct PipelineSymbolEntry
{
    PipelineSymbolType        type;
    Elf::SymbolTableEntryType entryType;
    AbiSectionType            sectionType;
    uint64                    value;
    uint64                    size;
};

/// Contains only the relevant info for a pipeline symbol.  E.g., a symbol whose name doesn't match any of the
/// predefined types in @ref PipelineSymbolType.
struct GenericSymbolEntry
{
    const char*               pName;
    Elf::SymbolTableEntryType entryType;
    AbiSectionType            sectionType;
    uint64                    value;
    uint64                    size;
};

/// The structure of the AMDGPU ELF e_flags header field.
union AmdGpuElfFlags
{
    struct
    {
        uint32 machineId      :  8;  ///< EF_AMDGPU_MACH
        uint32 xnackFeature   :  2;  ///< EF_AMDGPU_FEATURE_XNACK_V4
        uint32 sramEccFeature :  2;  ///< EF_AMDGPU_FEATURE_SRAMECC_V4
        uint32 reserved       : 20;
    };

    AmdGpuMachineType machineType;   ///< EF_AMDGPU_MACH

    uint32 u32All;                   ///< e_flags packed as a 32-bit unsigned integer.
};

static_assert(sizeof(AmdGpuMachineType) == 1, "AmdGpuMachineType enum underlying type is larger than expected.");

/// Maximum number of viewports.
constexpr uint32 MaxViewports = 16;

// Constant buffer used by the primitive shader when culling is enabled.
// Passes the currently set register state to the shader to control the culling algorithm.
struct PrimShaderCullingCb
{
    uint32 padding0;
    uint32 padding1;

    uint32 paClVteCntl;                         ///< Viewport transform control.
    uint32 paSuVtxCntl;                         ///< Controls for float to fixed vertex conversion.
    uint32 paClClipCntl;                        ///< Clip space controls.

    uint32 padding2;
    uint32 padding3;

    uint32 paSuScModeCntl;                      ///< Culling controls.

    uint32 paClGbHorzClipAdj;                   ///< Frustum horizontal adjacent culling control.
    uint32 paClGbHorzDiscAdj;                   ///< Frustum horizontal discard culling control.
    uint32 paClGbVertClipAdj;                   ///< Frustum vertical adjacent culling control.
    uint32 paClGbVertDiscAdj;                   ///< Frustum vertical discard culling control.

    uint32 padding4;

    struct Viewports
    {
        uint32 paClVportXScale;                 ///< Viewport transform scale for X.
        uint32 paClVportXOffset;                ///< Viewport transform offset for X.
        uint32 paClVportYScale;                 ///< Viewport transform scale for Y.
        uint32 paClVportYOffset;                ///< Viewport transform offset for Y.
        uint32 padding5;
        uint32 padding6;
    } viewports[MaxViewports];

    struct Scissors
    {
        uint32 padding7;
        uint32 padding8;
    } scissors[MaxViewports];

    uint32 padding9;
    uint32 padding10;
    uint32 padding11;

    uint32 enableConservativeRasterization;     ///< Conservative rasterization is enabled, disabled certain culling
                                                ///  algorithms.
};

/// Constant buffer used by primitive shader generation for per-submit register controls of culling.
struct PrimShaderPsoCb
{
    uint32 gsAddressLo;              ///< Low 32-bits of GS address used for a jump from ES.
    uint32 gsAddressHi;              ///< High 32-bits of GS address used for a jump from ES.
    uint32 paClVteCntl;              ///< Viewport transform control.
    uint32 paSuVtxCntl;              ///< Controls for float to fixed vertex conversion.
    uint32 paClClipCntl;             ///< Clip space controls.
    uint32 paScWindowOffset;         ///< Offset for vertices in screen space.
    uint32 paSuHardwareScreenOffset; ///< Offset for guardband.
    uint32 paSuScModeCntl;           ///< Culling controls.
    uint32 paClGbHorzClipAdj;        ///< Frustrum horizontal adjacent culling control.
    uint32 paClGbVertClipAdj;        ///< Frustrum vertical adjacent culling control.
    uint32 paClGbHorzDiscAdj;        ///< Frustrum horizontal discard culling control.
    uint32 paClGbVertDiscAdj;        ///< Frustrum vertical discard culling control.
    uint32 vgtPrimitiveType;         ///< Runtime handling of primitive type
};

/// Constant buffer used by primitive shader generation for per-submit register controls of viewport transform.
struct PrimShaderVportCb
{
    struct Controls
    {
        /// Viewport transform scale and offset for x, y, z components
        uint32 paClVportXscale;
        uint32 paClVportXoffset;
        uint32 paClVportYscale;
        uint32 paClVportYoffset;
        uint32 paClVportZscale;
        uint32 paClVportZoffset;
    } vportControls[MaxViewports];
};

/// Constant buffer used by primitive shader generation for per-submit register controls of bounding boxes.
struct PrimShaderScissorCb
{
    struct
    {
        /// Viewport scissor that defines a bounding box
        uint32 paScVportScissorTL;
        uint32 paScVportScissorBR;
    } scissorControls[MaxViewports];
};

/// Constant buffer used by the primitive shader generation for various render state not known until draw time
struct PrimShaderRenderCb
{
    uint32 primitiveRestartEnable;          ///< Enable resetting of a triangle strip using a special index.
    uint32 primitiveRestartIndex;           ///< Value used to determine if a primitive restart is triggered
    uint32 matchAllBits;                    ///< When comparing restart indices, this limits number of bits
    uint32 enableConservativeRasterization; ///< Conservative rasterization is enabled, triggering special logic
                                            ///  for culling.
};

/// This struct defines the expected layout in memory when 'contiguousCbs' is set
struct PrimShaderCbLayout
{
    PrimShaderPsoCb     pipelineStateCb;
    PrimShaderVportCb   viewportStateCb;
    PrimShaderScissorCb scissorStateCb;
    PrimShaderRenderCb  renderStateCb;
};

static_assert(sizeof(PrimShaderCullingCb) == sizeof(PrimShaderCbLayout),
    "Transition structure (PrimShaderCullingCb) is not the same size as original structure (PrimShaderCbLayout)!");

/// Point sprite override selection.
enum class PointSpriteSelect : uint32
{
    Zero,   ///< Select 0.0f.
    One,    ///< Select 1.0f.
    S,      ///< Select S component value.
    T,      ///< Select T component value.
    None,   ///< Keep interpolated result.
};

/// Geometry Shader output primitive type.
enum class GsOutPrimType : uint32
{
    PointList = 0, ///< A list of individual vertices that make up points.
    LineStrip,     ///< Each additional vertex after the first two makes a new line.
    TriStrip,      ///< Each additional vertex after the first three makes a new triangle.
    Rect2d,        ///< Each rect is the bounding box of an arbitrary 2D triangle.
    RectList,      ///< Each rect is three 2D axis-aligned rectangle vertices.
    Last,
};

/// Specifies how to populate the sample mask provided to pixel shaders.
enum class CoverageToShaderSel : uint32
{
    InputCoverage = 0,      ///< In over rasterization mode, replicate the overrast result to all detail samples of
                            ///  the pixel. In standard rasterization mode, leave the sample mask untouched.
    InputInnerCoverage,     ///< In under rasterization mode, replicate the underrast result to all detail samples
                            ///  of the pixel. If under rasterization is disabled output raw mask.
    InputDepthCoverage,     ///< The InputCoverage mask bitwise ANDed with the result of Early Depth/Stencil testing.
    Raw,                    ///< Output the scan converter's internal mask, unchanged.
};

/// Specifies how a shader instruction uses a constant buffer value
enum class CbConstUsageType : uint8
{
    LoopIter,
    Eq0Float,
    Lt0Float,
    Gt0Float,
    Eq0Int,
    Lt0Int,
    Gt0Int,
    Other
};

/// Defines the various methods for how tessellated patches can be distributed amongst the GPU's shader engines.
enum class TessDistributionMode : uint8
{
    NoDist = 0,   ///< Tessellated patches are not distributed amongst the shader engines.
    Patches,      ///< Whole tessellated patches are distributed.
    Donuts,       ///< Donut-shaped groups of tessellated geometry are distributed.
    Trapezoids,   ///< Trapezoid-shaped groups of tessellated geometry are distributed.
};

/// Indicates the type of Z testing.
enum class ZOrder : uint8
{
    LateZ = 0,
    EarlyZThenLateZ,
    ReZ,
    EarlyZThenReZ
};

 } //Abi
 namespace PalAbi
 {

 constexpr uint32 PipelineMetadataMajorVersion = 3;  ///< Pipeline Metadata Major Version
 constexpr uint32 PipelineMetadataMinorVersion = 6;  ///< Pipeline Metadata Minor Version

 constexpr uint32 PipelineMetadataBase = 0x10000000; ///< Deprecated - Pipeline Metadata base value to be OR'd with the
                                                     ///  PipelineMetadataEntry value when saving to ELF.

 } //PalAbi
 } //Pal
