##
 #######################################################################################################################
 #
 #  Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All Rights Reserved.
 #
 #  Permission is hereby granted, free of charge, to any person obtaining a copy
 #  of this software and associated documentation files (the "Software"), to deal
 #  in the Software without restriction, including without limitation the rights
 #  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 #  copies of the Software, and to permit persons to whom the Software is
 #  furnished to do so, subject to the following conditions:
 #
 #  The above copyright notice and this permission notice shall be included in all
 #  copies or substantial portions of the Software.
 #
 #  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 #  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 #  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 #  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 #  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 #  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 #  SOFTWARE.
 #
 #######################################################################################################################

include_guard()
cmake_minimum_required(VERSION 3.13)

set(SWD_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})
include(${CMAKE_CURRENT_LIST_DIR}/cmake/SwdHelper.cmake)

# It is expected that a client first include this file and then call swd_add_to_target for the
# target that needs SWD support.
#
# To enable specific GfxIps and ASICs, clients must set variables in the form of:
#    PREFIX_SWD_BUILD_XX
function(swd_add_to_target TARGET PREFIX)
    target_include_directories(${TARGET} PRIVATE ${SWD_SOURCE_DIR}/inc)

#if SWD_STRINGIFY
    swd_bp(${PREFIX}_SWD_STRINGIFY OFF)
    target_compile_definitions(${TARGET} PRIVATE SWD_STRINGIFY=$<BOOL:${${PREFIX}_SWD_STRINGIFY}>)
#endif

#if SWD_BUILD_PHX2
    swd_bp(${PREFIX}_SWD_BUILD_PHX2 OFF)
    target_compile_definitions(${TARGET} PRIVATE SWD_BUILD_PHX2=$<BOOL:${${PREFIX}_SWD_BUILD_PHX2}>)
#endif

#if SWD_BUILD_STRIX
    swd_bp(${PREFIX}_SWD_BUILD_STRIX OFF)
    target_compile_definitions(${TARGET} PRIVATE SWD_BUILD_STRIX=$<BOOL:${${PREFIX}_SWD_BUILD_STRIX}>)
#endif

#if SWD_BUILD_STRIX && SWD_BUILD_STRIX1
    swd_bp(${PREFIX}_SWD_BUILD_STRIX1 OFF DEPENDS_ON "${PREFIX}_SWD_BUILD_STRIX")
    target_compile_definitions(${TARGET} PRIVATE SWD_BUILD_STRIX1=$<BOOL:${${PREFIX}_SWD_BUILD_STRIX1}>)
#endif

#if SWD_BUILD_STRIX && SWD_BUILD_STRIX_HALO
    swd_bp(${PREFIX}_SWD_BUILD_STRIX_HALO OFF DEPENDS_ON "${PREFIX}_SWD_BUILD_STRIX")
    target_compile_definitions(${TARGET} PRIVATE SWD_BUILD_STRIX_HALO=$<BOOL:${${PREFIX}_SWD_BUILD_STRIX_HALO}>)
#endif

#if SWD_BUILD_GFX12
    swd_bp(${PREFIX}_SWD_BUILD_GFX12 OFF)
    target_compile_definitions(${TARGET} PRIVATE SWD_BUILD_GFX12=$<BOOL:${${PREFIX}_SWD_BUILD_GFX12}>)
#endif

#if SWD_BUILD_GFX12 && SWD_BUILD_NAVI4X
    swd_bp(${PREFIX}_SWD_BUILD_NAVI4X OFF DEPENDS_ON "${PREFIX}_SWD_BUILD_GFX12")
    target_compile_definitions(${TARGET} PRIVATE SWD_BUILD_NAVI4X=$<BOOL:${${PREFIX}_SWD_BUILD_NAVI4X}>)
#endif

#if SWD_BUILD_NAVI4X && SWD_BUILD_GFX12 && SWD_BUILD_NAVI48
    swd_bp(${PREFIX}_SWD_BUILD_NAVI48 OFF DEPENDS_ON "${PREFIX}_SWD_BUILD_NAVI4X;${PREFIX}_SWD_BUILD_GFX12")
    target_compile_definitions(${TARGET} PRIVATE SWD_BUILD_NAVI48=$<BOOL:${${PREFIX}_SWD_BUILD_NAVI48}>)
#endif

#if SWD_BUILD_NAVI4X && SWD_BUILD_GFX12 && SWD_BUILD_NAVI44
    swd_bp(${PREFIX}_SWD_BUILD_NAVI44 OFF DEPENDS_ON "${PREFIX}_SWD_BUILD_NAVI4X;${PREFIX}_SWD_BUILD_GFX12")
    target_compile_definitions(${TARGET} PRIVATE SWD_BUILD_NAVI44=$<BOOL:${${PREFIX}_SWD_BUILD_NAVI44}>)
#endif

    target_sources(${TARGET} PRIVATE ${SWD_SOURCE_DIR}/inc/g_gfx11SwWarDetection.h)
    set_source_files_properties(${SWD_SOURCE_DIR}/inc/g_gfx11SwWarDetection.h TARGET_DIRECTORY ${TARGET} PROPERTIES GENERATED ON)
#if SWD_BUILD_GFX12
    if(${PREFIX}_SWD_BUILD_GFX12)
        target_sources(${TARGET} PRIVATE ${SWD_SOURCE_DIR}/inc/g_gfx12SwWarDetection.h)
        set_source_files_properties(${SWD_SOURCE_DIR}/inc/g_gfx12SwWarDetection.h TARGET_DIRECTORY ${TARGET} PROPERTIES GENERATED ON)
    endif()
#endif

endfunction()
