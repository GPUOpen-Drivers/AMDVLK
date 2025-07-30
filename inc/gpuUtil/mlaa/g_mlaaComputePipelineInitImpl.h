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

#pragma once

#include "palDevice.h"
#include "palPipeline.h"
#include "palSysMemory.h"

#include "g_mlaaComputePipelineInit.h"
#include "g_mlaaComputePipelineBinaries.h"

namespace GpuUtil
{
namespace Mlaa
{

// =====================================================================================================================
// Creates all compute pipeline objects required by MlaaUtil.
template <typename Allocator>
Pal::Result CreateMlaaComputePipeline(
    Pal::IDevice*         pDevice,
    Allocator*            pAllocator,
    Pal::IPipeline**      pPipelineMem,
    const PipelineBinary* pTable,
    MlaaComputePipeline   pipelineType)
{
    Pal::Result result = Pal::Result::Success;

    Pal::ComputePipelineCreateInfo pipeInfo = { };

    pipeInfo = { };
    pipeInfo.pPipelineBinary      = pTable[static_cast<size_t>(pipelineType)].pBuffer;
    pipeInfo.pipelineBinarySize   = pTable[static_cast<size_t>(pipelineType)].size;
    pipeInfo.flags.clientInternal = 1;

    void* pMemory = PAL_MALLOC(pDevice->GetComputePipelineSize(pipeInfo, nullptr),
                               pAllocator,
                               Util::SystemAllocType::AllocInternal);
    if (pMemory != nullptr)
    {
        result = pDevice->CreateComputePipeline(
            pipeInfo,
            pMemory,
            &pPipelineMem[static_cast<size_t>(pipelineType)]);

        if (result != Pal::Result::Success)
        {
            // We need to explicitly free pMemory if an error occured because m_pPipeline won't be valid.
            PAL_SAFE_FREE(pMemory, pAllocator);
        }
    }
    else
    {
        result = Pal::Result::ErrorOutOfMemory;
    }

    return result;
}

// =====================================================================================================================
// Creates all compute pipeline objects required by MlaaUtil.
template <typename Allocator>
Pal::Result CreateMlaaComputePipelines(
    Pal::IDevice*    pDevice,
    Allocator*       pAllocator,
    Pal::IPipeline** pPipelineMem)
{
    Pal::Result result = Pal::Result::Success;

    Pal::DeviceProperties properties = {};
    pDevice->GetProperties(&properties);

    const PipelineBinary* pTable = nullptr;

    switch (Pal::uint32(properties.gfxTriple))
    {
    case Pal::IpTriple({ 10, 1, 0 }):
    case Pal::IpTriple({ 10, 1, 1 }):
    case Pal::IpTriple({ 10, 1, 2 }):
        pTable = mlaaComputeBinaryTable10_1_0;
        break;

    case Pal::IpTriple({ 10, 3, 0 }):
    case Pal::IpTriple({ 10, 3, 1 }):
    case Pal::IpTriple({ 10, 3, 2 }):
    case Pal::IpTriple({ 10, 3, 4 }):
    case Pal::IpTriple({ 10, 3, 5 }):
    case Pal::IpTriple({ 10, 3, 6 }):
        pTable = mlaaComputeBinaryTable10_3_0;
        break;

    case Pal::IpTriple({ 11, 0, 0 }):
    case Pal::IpTriple({ 11, 0, 1 }):
        pTable = mlaaComputeBinaryTable11_0_0;
        break;

    case Pal::IpTriple({ 11, 0, 2 }):
        pTable = mlaaComputeBinaryTable11_0_2;
        break;

    case Pal::IpTriple({ 11, 0, 3 }):
        pTable = mlaaComputeBinaryTable11_0_3;
        break;

    case Pal::IpTriple({ 11, 5, 0 }):
        pTable = mlaaComputeBinaryTable11_5_0;
        break;

#if  PAL_BUILD_STRIX_HALO
    case Pal::IpTriple({ 11, 5, 1 }):
        pTable = mlaaComputeBinaryTable11_5_1;
        break;
#endif

#if PAL_BUILD_GFX12 && PAL_BUILD_NAVI44
    case Pal::IpTriple({ 12, 0, 0 }):
        pTable = mlaaComputeBinaryTable12_0_0;
        break;
#endif

#if PAL_BUILD_GFX12 && PAL_BUILD_NAVI48
    case Pal::IpTriple({ 12, 0, 1 }):
        pTable = mlaaComputeBinaryTable12_0_1;
        break;
#endif

    default:
        result = Pal::Result::ErrorUnknown;
        PAL_NOT_IMPLEMENTED();
        break;
    }

    for (uint32 i = 0; ((result == Pal::Result::Success) && (i < static_cast<uint32>(MlaaComputePipeline::Count))); i++)
    {
        result = CreateMlaaComputePipeline(pDevice,
                                           pAllocator,
                                           pPipelineMem,
                                           pTable,
                                           static_cast<MlaaComputePipeline>(i));
    }

    return result;
}

} // Mlaa
} // GpuUtil
