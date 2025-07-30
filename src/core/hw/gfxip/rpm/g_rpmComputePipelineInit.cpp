/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "core/internalMemMgr.h"
#include "core/hw/gfxip/computePipeline.h"
#include "core/hw/gfxip/rpm/g_rpmComputePipelineInit.h"
#include "core/hw/gfxip/rpm/g_rpmComputePipelineBinaries.h"

using namespace Util;

namespace Pal
{

// =====================================================================================================================
// Helper function that returns a compute pipeline table for a given gfxIP
static const PipelineBinary*const GetRpmComputePipelineTable(
    const GpuChipProperties& properties)
{
    const PipelineBinary* pTable = nullptr;
    switch (uint32(properties.gfxTriple))
    {
    case Pal::IpTriple({ 10, 1, 0 }):
    case Pal::IpTriple({ 10, 1, 1 }):
    case Pal::IpTriple({ 10, 1, 2 }):
        pTable = rpmComputeBinaryTable10_1_0;
        break;

    case Pal::IpTriple({ 10, 3, 0 }):
    case Pal::IpTriple({ 10, 3, 1 }):
    case Pal::IpTriple({ 10, 3, 2 }):
    case Pal::IpTriple({ 10, 3, 4 }):
    case Pal::IpTriple({ 10, 3, 5 }):
        pTable = rpmComputeBinaryTable10_3_0;
        break;

    case Pal::IpTriple({ 10, 3, 6 }):
        pTable = rpmComputeBinaryTable10_3_6;
        break;

    case Pal::IpTriple({ 11, 0, 0 }):
    case Pal::IpTriple({ 11, 0, 1 }):
        pTable = rpmComputeBinaryTable11_0_0;
        break;

    case Pal::IpTriple({ 11, 0, 2 }):
        pTable = rpmComputeBinaryTable11_0_2;
        break;

    case Pal::IpTriple({ 11, 0, 3 }):
        pTable = rpmComputeBinaryTable11_0_3;
        break;

    case Pal::IpTriple({ 11, 5, 0 }):
        pTable = rpmComputeBinaryTable11_5_0;
        break;

#if  PAL_BUILD_STRIX_HALO
    case Pal::IpTriple({ 11, 5, 1 }):
        pTable = rpmComputeBinaryTable11_5_1;
        break;
#endif

#if PAL_BUILD_GFX12 && PAL_BUILD_NAVI48
    case Pal::IpTriple({ 12, 0, 1 }):
        pTable = rpmComputeBinaryTable12_0_1;
        break;
#endif

#if PAL_BUILD_GFX12 && PAL_BUILD_NAVI44
    case Pal::IpTriple({ 12, 0, 0 }):
        pTable = rpmComputeBinaryTable12_0_0;
        break;
#endif

    }

    return pTable;
}

// =====================================================================================================================
// Creates all compute pipeline objects required by RsrcProcMgr.
Result CreateRpmComputePipelines(
    GfxDevice*        pDevice,
    ComputePipeline** pPipelineMem)
{
    Result result = Result::Success;

    const GpuChipProperties& properties = pDevice->Parent()->ChipProperties();
    const PipelineBinary*const pTable   = GetRpmComputePipelineTable(properties);

    if (pTable == nullptr)
    {
        PAL_NOT_IMPLEMENTED();
        return Result::ErrorUnknown;
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ClearBuffer)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ClearBuffer);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ClearImage96Bpp)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ClearImage96Bpp);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ClearImage)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ClearImage);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ClearImageMsaaPlanar)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ClearImageMsaaPlanar);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ClearImageMsaaSampleMajor)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ClearImageMsaaSampleMajor);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyBufferByte)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyBufferByte);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyBufferDqword)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyBufferDqword);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyBufferDword)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyBufferDword);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImage2d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImage2d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImage2dMorton2x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImage2dMorton2x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImage2dMorton4x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImage2dMorton4x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImage2dMorton8x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImage2dMorton8x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImage2dms2x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImage2dms2x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImage2dms4x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImage2dms4x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImage2dms8x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImage2dms8x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImage2dShaderMipLevel)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImage2dShaderMipLevel);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImageGammaCorrect2d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImageGammaCorrect2d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImgToMem1d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImgToMem1d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImgToMem2d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImgToMem2d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImgToMem2dms2x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImgToMem2dms2x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImgToMem2dms4x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImgToMem2dms4x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImgToMem2dms8x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImgToMem2dms8x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyImgToMem3d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyImgToMem3d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyMemToImg1d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyMemToImg1d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyMemToImg2d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyMemToImg2d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyMemToImg2dms2x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyMemToImg2dms2x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyMemToImg2dms4x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyMemToImg2dms4x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyMemToImg2dms8x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyMemToImg2dms8x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyMemToImg3d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyMemToImg3d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyTypedBuffer1d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyTypedBuffer1d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyTypedBuffer2d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyTypedBuffer2d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::CopyTypedBuffer3d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::CopyTypedBuffer3d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ExpandMaskRam)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ExpandMaskRam);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ExpandMaskRamMs2x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ExpandMaskRamMs2x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ExpandMaskRamMs4x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ExpandMaskRamMs4x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ExpandMaskRamMs8x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ExpandMaskRamMs8x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::FastDepthClear)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::FastDepthClear);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::FastDepthExpClear)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::FastDepthExpClear);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::FastDepthStExpClear)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::FastDepthStExpClear);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::FillMem4xDword)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::FillMem4xDword);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        pipeInfo.interleaveSize = DispatchInterleaveSize::Disable;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::FillMemDword)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::FillMemDword);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        pipeInfo.interleaveSize = DispatchInterleaveSize::Disable;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::GenerateMipmaps)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::GenerateMipmaps);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::GenerateMipmapsLowp)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::GenerateMipmapsLowp);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::HtileCopyAndFixUp)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::HtileCopyAndFixUp);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::HtileSR4xUpdate)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::HtileSR4xUpdate);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::HtileSRUpdate)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::HtileSRUpdate);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskCopyImage)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskCopyImage);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskCopyImageOptimized)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskCopyImageOptimized);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskCopyImgToMem)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskCopyImgToMem);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskExpand2x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskExpand2x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskExpand4x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskExpand4x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskExpand8x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskExpand8x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve1xEqaa)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve1xEqaa);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve2x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve2x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve2xEqaa)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve2xEqaa);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve2xEqaaMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve2xEqaaMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve2xEqaaMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve2xEqaaMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve2xMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve2xMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve2xMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve2xMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve4x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve4x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve4xEqaa)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve4xEqaa);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve4xEqaaMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve4xEqaaMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve4xEqaaMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve4xEqaaMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve4xMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve4xMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve4xMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve4xMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve8x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve8x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve8xEqaa)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve8xEqaa);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve8xEqaaMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve8xEqaaMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve8xEqaaMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve8xEqaaMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve8xMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve8xMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskResolve8xMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskResolve8xMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaFmaskScaledCopy)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaFmaskScaledCopy);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolve2x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolve2x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolve2xMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolve2xMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolve2xMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolve2xMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolve4x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolve4x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolve4xMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolve4xMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolve4xMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolve4xMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolve8x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolve8x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolve8xMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolve8xMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolve8xMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolve8xMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolveStencil2xMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolveStencil2xMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolveStencil2xMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolveStencil2xMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolveStencil4xMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolveStencil4xMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolveStencil4xMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolveStencil4xMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolveStencil8xMax)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolveStencil8xMax);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaResolveStencil8xMin)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaResolveStencil8xMin);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::MsaaScaledCopyImage2d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::MsaaScaledCopyImage2d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ResolveOcclusionQuery)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ResolveOcclusionQuery);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ResolvePipelineStatsQuery)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ResolvePipelineStatsQuery);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ResolveStreamoutStatsQuery)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ResolveStreamoutStatsQuery);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::RgbToYuvPacked)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::RgbToYuvPacked);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::RgbToYuvPlanar)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::RgbToYuvPlanar);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ScaledCopyImage2d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ScaledCopyImage2d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ScaledCopyImage2dMorton2x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ScaledCopyImage2dMorton2x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ScaledCopyImage2dMorton4x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ScaledCopyImage2dMorton4x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ScaledCopyImage2dMorton8x)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ScaledCopyImage2dMorton8x);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ScaledCopyImage3d)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ScaledCopyImage3d);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::ScaledCopyTypedBufferToImg2D)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::ScaledCopyTypedBufferToImg2D);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::YuvIntToRgb)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::YuvIntToRgb);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::YuvToRgb)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::YuvToRgb);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx9EchoGlobalTable)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx9EchoGlobalTable);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        pipeInfo.interleaveSize = DispatchInterleaveSize::Disable;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10BuildDccLookupTable)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10BuildDccLookupTable);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10ClearDccComputeSetFirstPixel)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10ClearDccComputeSetFirstPixel);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10ClearDccComputeSetFirstPixelMsaa)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10ClearDccComputeSetFirstPixelMsaa);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10GenerateCmdDispatch)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10GenerateCmdDispatch);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10GenerateCmdDispatchTaskMesh)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10GenerateCmdDispatchTaskMesh);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10GenerateCmdDraw)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10GenerateCmdDraw);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10GfxDccToDisplayDcc)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10GfxDccToDisplayDcc);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10PrtPlusResolveResidencyMapDecode)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10PrtPlusResolveResidencyMapDecode);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10PrtPlusResolveResidencyMapEncode)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10PrtPlusResolveResidencyMapEncode);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10PrtPlusResolveSamplingStatusMap)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10PrtPlusResolveSamplingStatusMap);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx10VrsHtile)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx10VrsHtile);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx11GenerateCmdDispatchTaskMesh)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx11GenerateCmdDispatchTaskMesh);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }

#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48)
    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx12EchoGlobalTable)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx12EchoGlobalTable);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        pipeInfo.interleaveSize = DispatchInterleaveSize::Disable;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }
#endif

#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48)
    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx12FillMem128b)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx12FillMem128b);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        pipeInfo.interleaveSize = DispatchInterleaveSize::Disable;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }
#endif

#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48)
    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx12FillMem128bNoalloc)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx12FillMem128bNoalloc);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        pipeInfo.interleaveSize = DispatchInterleaveSize::Disable;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }
#endif

#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48)
    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx12ResolveOcclusionQuery)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx12ResolveOcclusionQuery);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }
#endif

#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48)
    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx12ResolvePipelineStatsQuery)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx12ResolvePipelineStatsQuery);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }
#endif

#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48)
    if ((result == Result::Success) && (pTable[uint32(RpmComputePipeline::Gfx12ResolveStreamoutStatsQuery)].pBuffer != nullptr))
    {
        constexpr uint32 Index = uint32(RpmComputePipeline::Gfx12ResolveStreamoutStatsQuery);

        ComputePipelineCreateInfo pipeInfo = { };
        pipeInfo.pPipelineBinary           = pTable[Index].pBuffer;
        pipeInfo.pipelineBinarySize        = pTable[Index].size;

        result = pDevice->CreateComputePipelineInternal(pipeInfo, &pPipelineMem[Index], AllocInternal);
    }
#endif

    return result;
}

} // Pal
