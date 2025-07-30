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

#include "pal.h"

namespace Pal
{

class ComputePipeline;
class GfxDevice;

// RPM Compute Pipelines. Used to index into RsrcProcMgr::m_pComputePipelines array
enum class RpmComputePipeline : uint32
{
    ClearBuffer                           = 0,
    ClearImage96Bpp                       = 1,
    ClearImage                            = 2,
    ClearImageMsaaPlanar                  = 3,
    ClearImageMsaaSampleMajor             = 4,
    CopyBufferByte                        = 5,
    CopyBufferDqword                      = 6,
    CopyBufferDword                       = 7,
    CopyImage2d                           = 8,
    CopyImage2dMorton2x                   = 9,
    CopyImage2dMorton4x                   = 10,
    CopyImage2dMorton8x                   = 11,
    CopyImage2dms2x                       = 12,
    CopyImage2dms4x                       = 13,
    CopyImage2dms8x                       = 14,
    CopyImage2dShaderMipLevel             = 15,
    CopyImageGammaCorrect2d               = 16,
    CopyImgToMem1d                        = 17,
    CopyImgToMem2d                        = 18,
    CopyImgToMem2dms2x                    = 19,
    CopyImgToMem2dms4x                    = 20,
    CopyImgToMem2dms8x                    = 21,
    CopyImgToMem3d                        = 22,
    CopyMemToImg1d                        = 23,
    CopyMemToImg2d                        = 24,
    CopyMemToImg2dms2x                    = 25,
    CopyMemToImg2dms4x                    = 26,
    CopyMemToImg2dms8x                    = 27,
    CopyMemToImg3d                        = 28,
    CopyTypedBuffer1d                     = 29,
    CopyTypedBuffer2d                     = 30,
    CopyTypedBuffer3d                     = 31,
    ExpandMaskRam                         = 32,
    ExpandMaskRamMs2x                     = 33,
    ExpandMaskRamMs4x                     = 34,
    ExpandMaskRamMs8x                     = 35,
    FastDepthClear                        = 36,
    FastDepthExpClear                     = 37,
    FastDepthStExpClear                   = 38,
    FillMem4xDword                        = 39,
    FillMemDword                          = 40,
    GenerateMipmaps                       = 41,
    GenerateMipmapsLowp                   = 42,
    HtileCopyAndFixUp                     = 43,
    HtileSR4xUpdate                       = 44,
    HtileSRUpdate                         = 45,
    MsaaFmaskCopyImage                    = 46,
    MsaaFmaskCopyImageOptimized           = 47,
    MsaaFmaskCopyImgToMem                 = 48,
    MsaaFmaskExpand2x                     = 49,
    MsaaFmaskExpand4x                     = 50,
    MsaaFmaskExpand8x                     = 51,
    MsaaFmaskResolve1xEqaa                = 52,
    MsaaFmaskResolve2x                    = 53,
    MsaaFmaskResolve2xEqaa                = 54,
    MsaaFmaskResolve2xEqaaMax             = 55,
    MsaaFmaskResolve2xEqaaMin             = 56,
    MsaaFmaskResolve2xMax                 = 57,
    MsaaFmaskResolve2xMin                 = 58,
    MsaaFmaskResolve4x                    = 59,
    MsaaFmaskResolve4xEqaa                = 60,
    MsaaFmaskResolve4xEqaaMax             = 61,
    MsaaFmaskResolve4xEqaaMin             = 62,
    MsaaFmaskResolve4xMax                 = 63,
    MsaaFmaskResolve4xMin                 = 64,
    MsaaFmaskResolve8x                    = 65,
    MsaaFmaskResolve8xEqaa                = 66,
    MsaaFmaskResolve8xEqaaMax             = 67,
    MsaaFmaskResolve8xEqaaMin             = 68,
    MsaaFmaskResolve8xMax                 = 69,
    MsaaFmaskResolve8xMin                 = 70,
    MsaaFmaskScaledCopy                   = 71,
    MsaaResolve2x                         = 72,
    MsaaResolve2xMax                      = 73,
    MsaaResolve2xMin                      = 74,
    MsaaResolve4x                         = 75,
    MsaaResolve4xMax                      = 76,
    MsaaResolve4xMin                      = 77,
    MsaaResolve8x                         = 78,
    MsaaResolve8xMax                      = 79,
    MsaaResolve8xMin                      = 80,
    MsaaResolveStencil2xMax               = 81,
    MsaaResolveStencil2xMin               = 82,
    MsaaResolveStencil4xMax               = 83,
    MsaaResolveStencil4xMin               = 84,
    MsaaResolveStencil8xMax               = 85,
    MsaaResolveStencil8xMin               = 86,
    MsaaScaledCopyImage2d                 = 87,
    ResolveOcclusionQuery                 = 88,
    ResolvePipelineStatsQuery             = 89,
    ResolveStreamoutStatsQuery            = 90,
    RgbToYuvPacked                        = 91,
    RgbToYuvPlanar                        = 92,
    ScaledCopyImage2d                     = 93,
    ScaledCopyImage2dMorton2x             = 94,
    ScaledCopyImage2dMorton4x             = 95,
    ScaledCopyImage2dMorton8x             = 96,
    ScaledCopyImage3d                     = 97,
    ScaledCopyTypedBufferToImg2D          = 98,
    YuvIntToRgb                           = 99,
    YuvToRgb                              = 100,
    Gfx9EchoGlobalTable                   = 101,
    Gfx10BuildDccLookupTable              = 102,
    Gfx10ClearDccComputeSetFirstPixel     = 103,
    Gfx10ClearDccComputeSetFirstPixelMsaa = 104,
    Gfx10GenerateCmdDispatch              = 105,
    Gfx10GenerateCmdDispatchTaskMesh      = 106,
    Gfx10GenerateCmdDraw                  = 107,
    Gfx10GfxDccToDisplayDcc               = 108,
    Gfx10PrtPlusResolveResidencyMapDecode = 109,
    Gfx10PrtPlusResolveResidencyMapEncode = 110,
    Gfx10PrtPlusResolveSamplingStatusMap  = 111,
    Gfx10VrsHtile                         = 112,
    Gfx11GenerateCmdDispatchTaskMesh      = 113,
#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48 || PAL_BUILD_NAVI44)
    Gfx12EchoGlobalTable                  = 114,
#endif
#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48 || PAL_BUILD_NAVI44)
    Gfx12FillMem128b                      = 115,
#endif
#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48 || PAL_BUILD_NAVI44)
    Gfx12FillMem128bNoalloc               = 116,
#endif
#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48 || PAL_BUILD_NAVI44)
    Gfx12ResolveOcclusionQuery            = 117,
#endif
#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48 || PAL_BUILD_NAVI44)
    Gfx12ResolvePipelineStatsQuery        = 118,
#endif
#if PAL_BUILD_GFX12&& ((0 && 0) || (PAL_BUILD_NAVI48&& 0)  || PAL_BUILD_NAVI48 || PAL_BUILD_NAVI44)
    Gfx12ResolveStreamoutStatsQuery       = 119,
#endif
    Count
};

Result CreateRpmComputePipelines(GfxDevice* pDevice, ComputePipeline** pPipelineMem);

} // Pal
