/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * @file  palPipelineAbiUtils.h
 * @brief PAL Pipeline ABI utilities.
 ***********************************************************************************************************************
 */

#pragma once

#include "palPipelineAbi.h"
#include "palInlineFuncs.h"

#include <climits>

namespace Util
{
namespace Abi
{

/// Helper function to get the GFXIP version when given a machine type.
///
/// @param [in] machineType The machine type.
/// @param [out] pGfxIpMajorVer The major version.
/// @param [out] pGfxIpMinorVer The minor version.
/// @param [out] pGfxIpStepping The stepping.
inline void MachineTypeToGfxIpVersion(
    AmdGpuMachineType machineType,
    uint32* pGfxIpMajorVer,
    uint32* pGfxIpMinorVer,
    uint32* pGfxIpStepping)
{
    switch (machineType)
    {
#if PAL_CLIENT_INTERFACE_MAJOR_VERSION < 888
    case AmdGpuMachineType::Gfx600:
        *pGfxIpMajorVer = 6;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 0;
        break;
    case AmdGpuMachineType::Gfx601:
        *pGfxIpMajorVer = 6;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 1;
        break;
    case AmdGpuMachineType::Gfx602:
        *pGfxIpMajorVer = 6;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 2;
        break;
    case AmdGpuMachineType::Gfx700:
        *pGfxIpMajorVer = 7;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 0;
        break;
    case AmdGpuMachineType::Gfx701:
        *pGfxIpMajorVer = 7;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 1;
        break;
    case AmdGpuMachineType::Gfx702:
        *pGfxIpMajorVer = 7;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 2;
        break;
    case AmdGpuMachineType::Gfx703:
        *pGfxIpMajorVer = 7;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 3;
        break;
    case AmdGpuMachineType::Gfx704:
        *pGfxIpMajorVer = 7;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 4;
        break;
    case AmdGpuMachineType::Gfx705:
        *pGfxIpMajorVer = 7;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 5;
        break;
    case AmdGpuMachineType::Gfx800:
        *pGfxIpMajorVer = 8;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 0;
        break;
    case AmdGpuMachineType::Gfx801:
        *pGfxIpMajorVer = 8;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 1;
        break;
    case AmdGpuMachineType::Gfx802:
        *pGfxIpMajorVer = 8;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 2;
        break;
    case AmdGpuMachineType::Gfx803:
        *pGfxIpMajorVer = 8;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 3;
        break;
    case AmdGpuMachineType::Gfx805:
        *pGfxIpMajorVer = 8;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 5;
        break;
    case AmdGpuMachineType::Gfx810:
        *pGfxIpMajorVer = 8;
        *pGfxIpMinorVer = 1;
        *pGfxIpStepping = 0;
        break;
    case AmdGpuMachineType::Gfx900:
        *pGfxIpMajorVer = 9;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 0;
        break;
    case AmdGpuMachineType::Gfx902:
        *pGfxIpMajorVer = 9;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 2;
        break;
    case AmdGpuMachineType::Gfx904:
        *pGfxIpMajorVer = 9;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 4;
        break;
    case AmdGpuMachineType::Gfx906:
        *pGfxIpMajorVer = 9;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 6;
        break;
    case AmdGpuMachineType::Gfx909:
        *pGfxIpMajorVer = 9;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 9;
        break;
    case AmdGpuMachineType::Gfx90C:
        *pGfxIpMajorVer = 9;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 12;
        break;
#endif
    case AmdGpuMachineType::Gfx1010:
        *pGfxIpMajorVer = 10;
        *pGfxIpMinorVer = 1;
        *pGfxIpStepping = 0;
        break;
    case AmdGpuMachineType::Gfx1011:
        *pGfxIpMajorVer = 10;
        *pGfxIpMinorVer = 1;
        *pGfxIpStepping = 1;
        break;
    case AmdGpuMachineType::Gfx1012:
        *pGfxIpMajorVer = 10;
        *pGfxIpMinorVer = 1;
        *pGfxIpStepping = 2;
        break;
    case AmdGpuMachineType::Gfx1030:
        *pGfxIpMajorVer = 10;
        *pGfxIpMinorVer = 3;
        *pGfxIpStepping = 0;
        break;
    case AmdGpuMachineType::Gfx1031:
        *pGfxIpMajorVer = 10;
        *pGfxIpMinorVer = 3;
        *pGfxIpStepping = 1;
        break;
    case AmdGpuMachineType::Gfx1032:
        *pGfxIpMajorVer = 10;
        *pGfxIpMinorVer = 3;
        *pGfxIpStepping = 2;
        break;
    case AmdGpuMachineType::Gfx1034:
        *pGfxIpMajorVer = 10;
        *pGfxIpMinorVer = 3;
        *pGfxIpStepping = 4;
        break;
    case AmdGpuMachineType::Gfx1035:
        *pGfxIpMajorVer = 10;
        *pGfxIpMinorVer = 3;
        *pGfxIpStepping = 5;
        break;
    case AmdGpuMachineType::Gfx1036:
        *pGfxIpMajorVer = 10;
        *pGfxIpMinorVer = 3;
        *pGfxIpStepping = 6;
        break;
    case AmdGpuMachineType::Gfx1100:
        *pGfxIpMajorVer = 11;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 0;
        break;
    case AmdGpuMachineType::Gfx1101:
        *pGfxIpMajorVer = 11;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 1;
        break;
    case AmdGpuMachineType::Gfx1102:
        *pGfxIpMajorVer = 11;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 2;
        break;
    case AmdGpuMachineType::Gfx1103:
        *pGfxIpMajorVer = 11;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 3;
        break;
    case AmdGpuMachineType::Gfx1150:
        *pGfxIpMajorVer = 11;
        *pGfxIpMinorVer = 5;
        *pGfxIpStepping = 0;
        break;
#if PAL_BUILD_STRIX_HALO
    case AmdGpuMachineType::Gfx1151:
        *pGfxIpMajorVer = 11;
        *pGfxIpMinorVer = 5;
        *pGfxIpStepping = 1;
        break;
#endif
#if PAL_BUILD_NAVI44
    case AmdGpuMachineType::Gfx1200:
        *pGfxIpMajorVer = 12;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = GfxIpSteppingNavi44;
        break;
#endif
#if PAL_BUILD_NAVI48
    case AmdGpuMachineType::Gfx1201:
        *pGfxIpMajorVer = 12;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = GfxIpSteppingNavi48;
        break;
#endif
    default:
        // What is this?
        PAL_ASSERT_ALWAYS();
        *pGfxIpMajorVer = 0;
        *pGfxIpMinorVer = 0;
        *pGfxIpStepping = 0;
        break;
    }
}

/// Helper function to get the machine type when given a GFXIP version.
///
/// @param [in] gfxIpMajorVer The major version.
/// @param [in] gfxIpMinorVer The minor version.
/// @param [in] gfxIpStepping The stepping.
/// @param [out] pMachineType The machine type.
inline void GfxIpVersionToMachineType(
    uint32             gfxIpMajorVer,
    uint32             gfxIpMinorVer,
    uint32             gfxIpStepping,
    AmdGpuMachineType* pMachineType)
{
    switch (gfxIpMajorVer)
    {
#if PAL_CLIENT_INTERFACE_MAJOR_VERSION < 888
    case 6:
        switch (gfxIpStepping)
        {
        case GfxIpSteppingOland:
            *pMachineType = AmdGpuMachineType::Gfx602;
            break;
        default:
            *pMachineType = static_cast<AmdGpuMachineType>(static_cast<uint32>(AmdGpuMachineType::Gfx600) +
                                                                 gfxIpStepping);
            break;
        }
        break;
    case 7:
        switch (gfxIpStepping)
        {
        case GfxIpSteppingGodavari:
            *pMachineType = AmdGpuMachineType::Gfx705;
            break;
        default:
            *pMachineType = static_cast<AmdGpuMachineType>(static_cast<uint32>(AmdGpuMachineType::Gfx700) +
                                                                 gfxIpStepping);
            break;
        }
        break;
    case 8:
        switch (gfxIpMinorVer)
        {
        case 0:
            switch (gfxIpStepping)
            {
            case GfxIpSteppingTongaPro:
                *pMachineType = AmdGpuMachineType::Gfx805;
                break;
            default:
                *pMachineType =
                    static_cast<AmdGpuMachineType>(static_cast<uint32>(AmdGpuMachineType::Gfx801) + gfxIpStepping - 1);
                break;
            }
            break;
        case 1:
            *pMachineType = AmdGpuMachineType::Gfx810;
            break;
        default:
            PAL_ASSERT_ALWAYS();
        }
        break;
    case 9:
        switch (gfxIpStepping)
        {
        case GfxIpSteppingVega10:
            *pMachineType = AmdGpuMachineType::Gfx900;
            break;
        case GfxIpSteppingRaven:
            *pMachineType = AmdGpuMachineType::Gfx902;
            break;
        case GfxIpSteppingVega12:
            *pMachineType = AmdGpuMachineType::Gfx904;
            break;
        case GfxIpSteppingVega20:
            *pMachineType = AmdGpuMachineType::Gfx906;
            break;
        case GfxIpSteppingRaven2:
            *pMachineType = AmdGpuMachineType::Gfx909;
            break;
        case GfxIpSteppingRenoir:
            *pMachineType = AmdGpuMachineType::Gfx90C;
            break;
        }
        break;
#endif
    case 10:
        switch (gfxIpMinorVer)
        {
        case 1:
            switch (gfxIpStepping)
            {
            case GfxIpSteppingNavi10:
                *pMachineType = AmdGpuMachineType::Gfx1010;
                break;
            case GfxIpSteppingNavi12:
                *pMachineType = AmdGpuMachineType::Gfx1011;
                break;
            case GfxIpSteppingNavi14:
                *pMachineType = AmdGpuMachineType::Gfx1012;
                break;
            }
            break;
        case 3:
            switch (gfxIpStepping)
            {
            case GfxIpSteppingNavi21:
                *pMachineType = AmdGpuMachineType::Gfx1030;
                break;
            case GfxIpSteppingNavi22:
                *pMachineType = AmdGpuMachineType::Gfx1031;
                break;
            case GfxIpSteppingNavi23:
                *pMachineType = AmdGpuMachineType::Gfx1032;
                break;
            case GfxIpSteppingNavi24:
                *pMachineType = AmdGpuMachineType::Gfx1034;
                break;
            case GfxIpSteppingRembrandt:
                *pMachineType = AmdGpuMachineType::Gfx1035;
                break;
            case GfxIpSteppingRaphael:
                *pMachineType = AmdGpuMachineType::Gfx1036;
                break;
            default:
                PAL_ASSERT_ALWAYS();
            }
            break;
        }
        break;
    case 11:
        switch(gfxIpMinorVer)
        {
        case 0:
            switch (gfxIpStepping)
            {
            case GfxIpSteppingNavi31:
                *pMachineType = AmdGpuMachineType::Gfx1100;
                break;
            case GfxIpSteppingNavi32:
                *pMachineType = AmdGpuMachineType::Gfx1101;
                break;
            case GfxIpSteppingNavi33:
                *pMachineType = AmdGpuMachineType::Gfx1102;
                break;
            case GfxIpSteppingPhoenix:
                *pMachineType = AmdGpuMachineType::Gfx1103;
                break;
            default:
                PAL_ASSERT_ALWAYS();
                break;
            }
            break;
        case 5:
            switch (gfxIpStepping)
            {
            case GfxIpSteppingStrix:
                *pMachineType = AmdGpuMachineType::Gfx1150;
                break;
#if PAL_BUILD_STRIX_HALO
            case GfxIpSteppingStrixHalo:
                *pMachineType = AmdGpuMachineType::Gfx1151;
                break;
#endif
            default:
                PAL_ASSERT_ALWAYS();
                break;
            }
            break;
        }
        break;
#if PAL_BUILD_GFX12
    case 12:
        switch (gfxIpMinorVer)
        {
        case 0:
            switch (gfxIpStepping)
            {

#if PAL_BUILD_NAVI44
            case GfxIpSteppingNavi44:
                *pMachineType = AmdGpuMachineType::Gfx1200;
                break;
#endif

#if PAL_BUILD_NAVI48
            case GfxIpSteppingNavi48:
                *pMachineType = AmdGpuMachineType::Gfx1201;
                break;
#endif

            default:
                PAL_ASSERT_ALWAYS();
                break;
            }
            break;

        default:
            PAL_ASSERT_ALWAYS();
            break;
        }
        break;
#endif
    default:
        PAL_ASSERT_ALWAYS();
        break;
    }
}

/// Helper function to parse the Metadata section of a pipeline ELF.
///
/// @param [in] pReader            The message pack reader.
/// @param [in] pDesc              The content of the metadata note section.
/// @param [in] descSize           The length of the note section.
/// @param [in] versionType        the hash of the metadata version identifier
/// @param [out] pMetadataMajorVer The major metadata version.
/// @param [out] pMetadataMinorVer The minor metadata version.
///
/// @returns Success if successful, ErrorInvalidValue, ErrorUnknown or ErrorInvalidPipelineElf if
///          the metadata could not be parsed.
inline Result GetMetadataVersion(
    MsgPackReader* pReader,
    const void*    pDesc,
    uint32         descSize,
    uint32         versionType,
    uint32*        pMetadataMajorVer,
    uint32*        pMetadataMinorVer)
{
    // We need to retrieve version info from the msgpack blob.
    Result result = pReader->InitFromBuffer(pDesc, descSize);

    if ((result == Result::Success) && (pReader->Type() != CWP_ITEM_MAP))
    {
        result = Result::ErrorInvalidPipelineElf;
    }

    for (uint32 j = pReader->Get().as.map.size; ((result == Result::Success) && (j > 0)); --j)
    {
        result = pReader->Next(CWP_ITEM_STR);

        if (result == Result::Success)
        {
            const auto&  str     = pReader->Get().as.str;
            const uint32 keyHash = HashString(static_cast<const char*>(str.start), str.length);
            if (keyHash == versionType)
            {
                result = pReader->Next(CWP_ITEM_ARRAY);
                if ((result == Result::Success) && (pReader->Get().as.array.size >= 2))
                {
                    result = pReader->UnpackNext(pMetadataMajorVer);
                }
                if (result == Result::Success)
                {
                    result = pReader->UnpackNext(pMetadataMinorVer);
                }
                break;
            }
            else
            {
                // Ideally, the version is the first field written so we don't reach here.
                result = pReader->Skip(1);
            }
        }
    }
    return result;
}

} //Abi

namespace PalAbi
{

/// Helper function to parse the PalMetadata section of a pipeline ELF.
///
/// @param [in] pReader            The message pack reader.
/// @param [in] pDesc              The content of the metadata note section.
/// @param [in] descSize           The length of the note section.
/// @param [out] pMetadataMajorVer The major metadata version.
/// @param [out] pMetadataMinorVer The minor metadata version.
///
/// @returns Success if successful, ErrorInvalidValue, ErrorUnknown or ErrorInvalidPipelineElf if
///          the metadata could not be parsed.
inline Result GetPalMetadataVersion(
    MsgPackReader* pReader,
    const void*    pDesc,
    uint32         descSize,
    uint32*        pMetadataMajorVer,
    uint32*        pMetadataMinorVer)
{
    return Abi::GetMetadataVersion(pReader,
                                   pDesc,
                                   descSize,
                                   CompileTimeHashString(CodeObjectMetadataKey::Version),
                                   pMetadataMajorVer,
                                   pMetadataMinorVer);
}

/// Helper function to check if PalMetadata version is >= the specified version.
///
/// @param [in] metadata          Deserialized metadata.
/// @param [in] metadataMajorVer  Major version to check against.
/// @param [in] metadataMinorVer  Minor version to check against.
inline bool PalMetadataVersionAtLeast(
    const CodeObjectMetadata& metadata,
    uint32                    metadataMajorVer,
    uint32                    metadataMinorVer)
{
    return (metadata.version[0] > metadataMajorVer) ||
           ((metadata.version[0] == metadataMajorVer) && (metadata.version[1] >= metadataMinorVer));
}

/// Helper function to parse the PalMetadata section of a pipeline ELF.
///
/// @param [in] pReader          The message pack reader.
/// @param [in] pRawMetadata     The content of the metadata note section.
/// @param [in] metadataSize     The length of the note section.
/// @param [in] metadataMajorVer The major metadata version.
/// @param [in] metadataMinorVer The minor metadata version.
///
/// @returns Success if successful, ErrorInvalidValue, ErrorUnknown or ErrorUnsupportedPipelineElfAbiVersion if
///          the metadata could not be parsed.
inline Result DeserializeCodeObjectMetadata(
    MsgPackReader*         pReader,
    CodeObjectMetadata*    pMetadata,
    const void*            pRawMetadata,
    uint32                 metadataSize,
    uint32                 metadataMajorVer,
    uint32                 metadataMinorVer)
{
    Result result = Result::ErrorUnsupportedPipelineElfAbiVersion;
    if ((metadataMajorVer == PalAbi::PipelineMetadataMajorVersion) ||
        // Metadata version 3 is backwards compatible, although a new paradigm for our metadata
        // (hence the major increment).
        (metadataMajorVer == 2))
    {
        result = pReader->InitFromBuffer(pRawMetadata, metadataSize);
        uint32 registersOffset = UINT_MAX;

        if (result == Result::Success)
        {
            result = Metadata::DeserializeCodeObjectMetadata(pReader, pMetadata);
        }
    }

    return result;
}

} //Abi
} //Pal
