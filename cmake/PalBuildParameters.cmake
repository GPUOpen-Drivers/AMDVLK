##
 #######################################################################################################################
 #
 #  Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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

# This file is meant to encapsulate all the variables that PAL's clients are intended to
# manipulate when customizing PAL for their purposes. Making it the first place cmake developers
# for DXCP, XGL, etc. will look to when they have build problems/ideas/confusion.
#
# If a client wishes to override a PAL variable, the recommended logic is:
#     if (NOT DEFINED PAL_*)
#         set(PAL_* <value>)
#     endif()
#
# If a client has their own build parameter that controls the PAL one, then the
# `if (NOT DEFINED PAL_*)` and `endif()` can be omitted.
#
# NOTE: It is not advised to write the PAL build parameter into the cache.
#
# If a client wishes to see the final value of a pal_client_bp in the client's CMake project, the client can use:
#    get_target_property(<output-var> pal <build-parameter-name>)
# to get the value that PAL uses. PAL's version of a build parameter may differ from a client's due to
# the dependency tree of that build parameter.

########################################################################################################################
# Start with any client-facing build parameters which the clients are REQUIRED to set.

# It's very convenient to have the PAL_CLIENT_INTERFACE_MAJOR_VERSION be defined before anything else
# And it needs to be defined hence the FATAL_ERROR.
pal_client_bp(PAL_CLIENT_INTERFACE_MAJOR_VERSION ""
    MODE "FATAL_ERROR"
    REQUIRED
)

if (DEFINED PAL_CLIENT_INTERFACE_MINOR_VERSION)
    message(AUTHOR_WARNING "Unneccessary to specify PAL_CLIENT_INTERFACE_MINOR_VERSION")
endif()

# Clients must define this variable so pal know what API it's facilitating
pal_client_bp(PAL_CLIENT ""
    MODE "FATAL_ERROR"
    REQUIRED
)

#####################################################################################
# Handle special case where we need PAL_CLIENT_* for dependencies in this file.

# Create a more convenient variable to avoid string comparisons.
set(PAL_CLIENT_${PAL_CLIENT} ON)

#####################################################################################

########################################################################################################################

########################################################################################################################
# Now provide any client-facing build parameters which the clients can ignore if desired.

pal_client_bp(PAL_BUILD_FORCE_ON OFF
    DESC
        "Forces all unset PAL_BUILD_* CMake variables to be ON, so long as the dependencies are met."
)

    set(PAL_BUILD_BRANCH "2420")

# See README.md for explanation of CORE/GPUUTIL functionality.
# NOTE: There is no PAL_BUILD_UTIL, that functionality isn't optional.
pal_client_bp(PAL_BUILD_CORE ON)
pal_client_bp(PAL_BUILD_GPUUTIL ON)

#if PAL_DEVELOPER_BUILD
pal_client_bp(PAL_DEVELOPER_BUILD OFF)
#endif

pal_deprecated_bp(PAL_BUILD_OSS ON "${PAL_CLIENT_INTERFACE_MAJOR_VERSION} LESS 888")
#if PAL_BUILD_OSS2_4
pal_deprecated_bp(PAL_BUILD_OSS2_4 ON "${PAL_CLIENT_INTERFACE_MAJOR_VERSION} LESS 888"
    DEPENDS_ON PAL_BUILD_OSS
)
#endif

#####################################################################################
# Handle special case where we need PAL_AMDGPU_BUILD for dependencies in this file.
# PAL_AMDGPU_BUILD is an internal build parameter, and as such is an exception in
# this file.

pal_internal_bp(PAL_AMDGPU_BUILD ON
    DEPENDS_ON
        UNIX
    DESC
        "Whether PAL is built with an amdgpu back-end?"
)

#####################################################################################

pal_client_bp(PAL_BUILD_DRI3    ON  DEPENDS_ON PAL_AMDGPU_BUILD)
pal_client_bp(PAL_BUILD_WAYLAND OFF DEPENDS_ON PAL_AMDGPU_BUILD)
pal_client_bp(PAL_DISPLAY_DCC   ON  DEPENDS_ON PAL_AMDGPU_BUILD)

# Build null device backend for offline compilation
pal_client_bp(PAL_BUILD_NULL_DEVICE ON
    DESC
        "Build null device backend for offline compilation"
)

#####################################################################################
# Specify GFXIP and ASIC enablements.
# The GFXIP and ASIC enablements should follow this hierarchy:
#     - Top-Level IP (for example, Gfxip)
#     |--- Hardware Layer (for example, Gfx9)
#     |------ Major IP Version (for example, Gfx10)
#     |--------- ASIC (for example, Navi21)
#
# Where possible, using whitespace below to dictate this hierarchy is preferred.
#
# If a major IP level is not required, it does not need to be specified and the ASICs
# in that major IP level are assumed to be controlled by their Hardware Layer
# enablement.
#
# In the event that a family of ASICs needs an enablement control to make coding
# easier, *sufficient need* must be proven during review in order to add a Family
# level to the hierarchy between Major IP Version and ASIC.
# For example, if all Navi2x ASICs need to be grouped together and *sufficient
# need* is proven, a PAL_BUILD_NAVI2X could be introduced to the hierarchy.
# NOTE: Family levels are considered internal build parameters and would be added to
#       PalOnlyBuildParameters.cmake.

pal_client_bp(PAL_BUILD_GFX ON
    DESC
        "Build PAL with Graphics support?"
)

    pal_client_bp(PAL_BUILD_GFX9 ON
        DEPENDS_ON
            PAL_BUILD_GFX
    )

        # PAL's GFX11 support is part of its GFX9 HWL so you need to enable both to get GFX11.
        pal_client_bp(PAL_BUILD_GFX11 ON
            DEPENDS_ON
                PAL_BUILD_GFX9
        )

            # Clients should directly set PAL_BUILD_GFX9 and PAL_BUILD_GFX11 in their cmakes to request support for
            # these GPUs. These used to be pal_client_bp calls but were simplified to pal_set_or calls to avoid
            # breaking clients that depend on the ASIC-specific variables automatically setting PAL_BUILD_GFX11.
            # Note: Explicitly not include DEPENDS_ON to ensure this support remains until it can be deprecated.
            pal_deprecated_bp(PAL_BUILD_PHOENIX2 ON "${PAL_CLIENT_INTERFACE_MAJOR_VERSION} LESS 888")
            pal_set_or(PAL_BUILD_GFX11 $<BOOL:${PAL_BUILD_PHOENIX2}>)
            pal_deprecated_bp(PAL_BUILD_STRIX1 ON "${PAL_CLIENT_INTERFACE_MAJOR_VERSION} LESS 917")
            pal_set_or(PAL_BUILD_GFX11 $<BOOL:${PAL_BUILD_STRIX1}>)

#if PAL_BUILD_HAWK_POINT1
            pal_client_bp(PAL_BUILD_HAWK_POINT1 ON
                DEPENDS_ON
                    PAL_BUILD_GFX11
            )
#endif PAL_BUILD_HAWK_POINT1
#if PAL_BUILD_HAWK_POINT2
            pal_client_bp(PAL_BUILD_HAWK_POINT2 ON
                DEPENDS_ON
                    PAL_BUILD_GFX11
            )
#endif
#if PAL_BUILD_STRIX_HALO
            pal_client_bp(PAL_BUILD_STRIX_HALO ON
                DEPENDS_ON
                    PAL_BUILD_GFX11
            )
#endif

#if PAL_BUILD_GFX12
    pal_client_bp(PAL_BUILD_GFX12 ON
        DEPENDS_ON
            PAL_BUILD_GFX
    )

#if PAL_BUILD_NAVI44
    pal_client_bp(PAL_BUILD_NAVI44 ON
        DEPENDS_ON
            PAL_BUILD_GFX12
    )

#if PAL_BUILD_NAVI48
        pal_client_bp(PAL_BUILD_NAVI48 ON
            DEPENDS_ON
                PAL_BUILD_GFX12
        )
#endif
#endif

#####################################################################################

#####################################################################################
# Miscellaneous enablements.

pal_client_bp(PAL_ENABLE_PRINTS_ASSERTS OFF
    DESC
        "Enable print assertions?"
)

pal_client_bp(PAL_ENABLE_PRINTS_ASSERTS_DEBUG ON
    DESC
        "Enable print assertions on debug builds?"
)

#if PAL_BUILD_RDF
pal_client_bp(PAL_BUILD_RDF ON)
#endif

pal_client_bp(PAL_BUILD_RPM_GFX_SHADERS ON
    DESC
        "This must always be enabled unless the client guarantees they do not use GFX/3D queues"
)

pal_client_bp(PAL_ENABLE_DEBUG_BREAK OFF
    DESC
        "Enable debug break?"
)

pal_client_bp(PAL_ENABLE_LOGGING OFF
    DESC
        "Enable debug logging?"
)

pal_client_bp(PAL_MEMTRACK OFF
    DESC
        "Enable PAL memory tracker?"
)

pal_client_bp(PAL_64BIT_ARCHIVE_FILE_FMT OFF
    DESC
        "DXCP requires 64-bit file archives to allow creation of files >4GB."
        "Vulkan requires 32-bit file archives for backwards compatibility."
        "Clients may choose your preference here. 32-bit by default."
)

pal_client_bp(PAL_PRECOMPILED_HEADERS OFF
    DESC
        "If set, enables precompiled headers to reduce compilation times."
)

#####################################################################################
