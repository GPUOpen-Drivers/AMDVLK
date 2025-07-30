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

#ifndef _AMDGPU_ASIC_H
#define _AMDGPU_ASIC_H

#define ATI_VENDOR_ID 0x1002
#define AMD_VENDOR_ID 0x1022

// AMDGPU_VENDOR_IS_AMD(vendorId)
#define AMDGPU_VENDOR_IS_AMD(v) ((v == ATI_VENDOR_ID) || (v == AMD_VENDOR_ID))

#define FAMILY_UNKNOWN 0x00
#define FAMILY_NV      0x8F // 143 / Navi1x, Navi2x
#define FAMILY_NV3     0x91 // 145 / Navi3x
#define FAMILY_RMB     0x92 // 146 / Rembrandt
#define FAMILY_PHX     0x94 // 148 / Phoenix
#define FAMILY_RPL     0x95 // 149 / Raphael
#define FAMILY_STX     0x96 // 150 / Strix
#define FAMILY_MDN     0x97 // 151 / Mendocino
#if PAL_BUILD_GFX12
#define FAMILY_NV4     0x98 // 152 / Navi4x
#endif

// AMDGPU_FAMILY_IS(familyId, familyName)
#define FAMILY_IS(f, fn)     (f == FAMILY_##fn)

// Gfx10.x
#define FAMILY_IS_NV(f)      FAMILY_IS(f, NV)
#define FAMILY_IS_RMB(f)     FAMILY_IS(f, RMB)
#define FAMILY_IS_RPL(f)     FAMILY_IS(f, RPL)
#define FAMILY_IS_MDN(f)     FAMILY_IS(f, MDN)

// Gfx11.0
#define FAMILY_IS_NV3(f)     FAMILY_IS(f, NV3)
#define FAMILY_IS_PHX(f)     FAMILY_IS(f, PHX)

// Gfx11.5
#define FAMILY_IS_STX(f)     FAMILY_IS(f, STX)

#if PAL_BUILD_GFX12
// Gfx12
#define FAMILY_IS_NV4(f)     FAMILY_IS(f, NV4)
#endif

#define AMDGPU_UNKNOWN          0xFF

// Gfx10.1
#define AMDGPU_NAVI10_RANGE        0x01, 0x0A //  1 <= x < 10
#define AMDGPU_NAVI12_RANGE        0x0A, 0x14 // 10  <= x < 20
#define AMDGPU_NAVI14_RANGE        0x14, 0x28 // 20  <= x < 40

// Gfx10.3
#define AMDGPU_NAVI21_RANGE        0x28, 0x32 // 40  <= x < 50
#define AMDGPU_NAVI22_RANGE        0x32, 0x3C // 50  <= x < 60
#define AMDGPU_NAVI23_RANGE        0x3C, 0x46 // 60  <= x < 70
#define AMDGPU_NAVI24_RANGE        0x46, 0x50 // 70  <= x < 80
#define AMDGPU_REMBRANDT_RANGE     0x01, 0xff // 1  < 255
#define AMDGPU_RAPHAEL_RANGE       0x01, 0xff // 1  < 255
#define AMDGPU_MENDOCINO_RANGE     0x01, 0xff // 1  < 255

// Gfx11.0
#define AMDGPU_NAVI31_RANGE        0x01, 0x10 // 1 <= x < 16
#define AMDGPU_NAVI33_RANGE        0x10, 0x20 // 16 <= x < 32
#define AMDGPU_NAVI32_RANGE        0x20, 0xff // 32 <= x < 255
#define AMDGPU_PHOENIX1_RANGE      0x01, 0x80 // 1 <= x < 128
#define AMDGPU_PHOENIX2_RANGE      0x80, 0xC0 // 128 <= x < 192
#if PAL_BUILD_HAWK_POINT1
#define AMDGPU_HAWK_POINT1_RANGE   0xC0, 0xF0 // 192 <= x < 240
#endif
#if PAL_BUILD_HAWK_POINT2
#define AMDGPU_HAWK_POINT2_RANGE   0xF0, 0xFF // 240 <= x < 255
#endif

// Gfx11.5
#define AMDGPU_STRIX1_RANGE        0x01, 0x40 // 1  <= x < 64 (tentative)
#if PAL_BUILD_STRIX_HALO
#define AMDGPU_STRIX_HALO_RANGE    0xC0, 0xFF // 192 <= x < 255 (tentative)
#endif

// Gfx12
#if PAL_BUILD_NAVI48
#define AMDGPU_NAVI48_RANGE        0x50, 0xff
#endif
#if PAL_BUILD_NAVI44
#define AMDGPU_NAVI44_RANGE        0x20, 0x50 // 32 <= x < 80
#endif

#define AMDGPU_EXPAND_FIX(x) x
#define AMDGPU_RANGE_HELPER(val, min, max) ((val >= min) && (val < max))
#define AMDGPU_IN_RANGE(val, ...)   AMDGPU_EXPAND_FIX(AMDGPU_RANGE_HELPER(val, __VA_ARGS__))

// ASICREV_IS(eRevisionId, revisionName)
#define ASICREV_IS(r, rn)              AMDGPU_IN_RANGE(r, AMDGPU_##rn##_RANGE)

// Gfx10.1
#define ASICREV_IS_NAVI10(r)           ASICREV_IS(r, NAVI10)
#define ASICREV_IS_NAVI12(r)           ASICREV_IS(r, NAVI12)
#define ASICREV_IS_NAVI14(r)           ASICREV_IS(r, NAVI14)

// Gfx10.3
#define ASICREV_IS_NAVI21(r)           ASICREV_IS(r, NAVI21)
#define ASICREV_IS_NAVI22(r)           ASICREV_IS(r, NAVI22)
#define ASICREV_IS_NAVI23(r)           ASICREV_IS(r, NAVI23)
#define ASICREV_IS_NAVI24(r)           ASICREV_IS(r, NAVI24)
#define ASICREV_IS_REMBRANDT(r)        ASICREV_IS(r, REMBRANDT)
#define ASICREV_IS_RAPHAEL(r)          ASICREV_IS(r, RAPHAEL)
#define ASICREV_IS_MENDOCINO(r)        ASICREV_IS(r, MENDOCINO)

// Gfx11.0
#define ASICREV_IS_NAVI31(r)           ASICREV_IS(r, NAVI31)
#define ASICREV_IS_NAVI32(r)           ASICREV_IS(r, NAVI32)
#define ASICREV_IS_NAVI33(r)           ASICREV_IS(r, NAVI33)
#define ASICREV_IS_PHOENIX1(r)         ASICREV_IS(r, PHOENIX1)
#define ASICREV_IS_PHOENIX2(r)         ASICREV_IS(r, PHOENIX2)
#if PAL_BUILD_HAWK_POINT1
#define ASICREV_IS_HAWK_POINT1(r)      ASICREV_IS(r, HAWK_POINT1)
#endif
#if PAL_BUILD_HAWK_POINT2
#define ASICREV_IS_HAWK_POINT2(r)      ASICREV_IS(r, HAWK_POINT2)
#endif

// Gfx11.5
#define ASICREV_IS_STRIX1(r)           ASICREV_IS(r, STRIX1)
#if PAL_BUILD_STRIX_HALO
#define ASICREV_IS_STRIX_HALO(r)       ASICREV_IS(r, STRIX_HALO)
#endif

#if PAL_BUILD_NAVI48
#define ASICREV_IS_NAVI48(r)           ASICREV_IS(r, NAVI48)
#endif

// AMDGPU_IS(familyId, eRevisionId, familyName, revisionName)
#define AMDGPU_IS(f, r, fn, rn)    (FAMILY_IS(f, fn) && ASICREV_IS(r, rn))

// Gfx10.1
#define AMDGPU_IS_NAVI10(f, r)        AMDGPU_IS(f, r, NV, NAVI10)
#define AMDGPU_IS_NAVI12(f, r)        AMDGPU_IS(f, r, NV, NAVI12)
#define AMDGPU_IS_NAVI14(f, r)        AMDGPU_IS(f, r, NV, NAVI14)

// Gfx10.3
#define AMDGPU_IS_NAVI21(f, r)        AMDGPU_IS(f, r, NV, NAVI21)
#define AMDGPU_IS_NAVI22(f, r)        AMDGPU_IS(f, r, NV, NAVI22)
#define AMDGPU_IS_NAVI23(f, r)        AMDGPU_IS(f, r, NV, NAVI23)
#define AMDGPU_IS_NAVI24(f, r)        AMDGPU_IS(f, r, NV, NAVI24)
#define AMDGPU_IS_REMBRANDT(f, r)     AMDGPU_IS(f, r, RMB, REMBRANDT)
#define AMDGPU_IS_RAPHAEL(f, r)       AMDGPU_IS(f, r, RPL, RAPHAEL)
#define AMDGPU_IS_MENDOCINO(f, r)     AMDGPU_IS(f, r, MDN, MENDOCINO)

// Gfx11.0
#define AMDGPU_IS_NAVI31(f, r)        AMDGPU_IS(f, r, NV3, NAVI31)
#define AMDGPU_IS_NAVI32(f, r)        AMDGPU_IS(f, r, NV3, NAVI32)
#define AMDGPU_IS_NAVI33(f, r)        AMDGPU_IS(f, r, NV3, NAVI33)
#define AMDGPU_IS_PHOENIX1(f, r)      AMDGPU_IS(f, r, PHX, PHOENIX1)
#define AMDGPU_IS_PHOENIX2(f, r)      AMDGPU_IS(f, r, PHX, PHOENIX2)
#if PAL_BUILD_HAWK_POINT1
#define AMDGPU_IS_HAWK_POINT1(f, r)   AMDGPU_IS(f, r, PHX, HAWK_POINT1)
#endif
#if PAL_BUILD_HAWK_POINT2
#define AMDGPU_IS_HAWK_POINT2(f, r)   AMDGPU_IS(f, r, PHX, HAWK_POINT2)
#endif

// Gfx11.5
#define AMDGPU_IS_STRIX1(f,r)         AMDGPU_IS(f, r, STX, STRIX1)
#if PAL_BUILD_STRIX_HALO
#define AMDGPU_IS_STRIX_HALO(f,r)     AMDGPU_IS(f, r, STX, STRIX_HALO)
#endif

// Gfx12
#if PAL_BUILD_NAVI48
#define AMDGPU_IS_NAVI48(f, r)        AMDGPU_IS(f, r, NV4, NAVI48)
#endif
#if PAL_BUILD_NAVI44
#define AMDGPU_IS_NAVI44(f, r)        AMDGPU_IS(f, r, NV4, NAVI44)
#endif

// Device IDs
// Gfx10.1
#define DEVICE_ID_NV_NAVI10_P_7310      0x7310
#define DEVICE_ID_NV_NAVI12_P_7360      0x7360
#define DEVICE_ID_NV_NAVI14_M_7340      0x7340

// Gfx10.3
#define DEVICE_ID_RMB_1681              0x1681 // Rembrandt
#define DEVICE_ID_RPL_164E              0x164E // Raphael
#define DEVICE_ID_MDN_1506              0x1506 // Mendocino

// Gfx11.0
#define DEVICE_ID_NV3_NAVI31_P_73BF     0x73BF
#define DEVICE_ID_NV3_NAVI31_P_744C     0x744C
#define DEVICE_ID_NV32_7C               0x7C   // Navi32
#define DEVICE_ID_NV3_NAVI32_P_73DF     0x73DF
#define DEVICE_ID_NV3_NAVI33_P_73F0     0x73F0
#define DEVICE_ID_PHX1_15BF             0x15BF // Phoenix1
#define DEVICE_ID_PHX2_15C8             0x15C8 // Phoenix2
#if PAL_BUILD_HAWK_POINT1
#define DEVICE_ID_HP1_1900              0x1900 // HawkPoint1
#endif
#if PAL_BUILD_HAWK_POINT2
#define DEVICE_ID_HP2_1901              0x1901 // HawkPoint2
#endif

// Gf11.5
#define DEVICE_ID_STX1_150E             0x150E // Strix1
#if PAL_BUILD_STRIX_HALO
#define DEVICE_ID_STXH_1586             0x1586 // Strix Halo
#endif

#if PAL_BUILD_NAVI48
#define DEVICE_ID_NAVI48_94             0x94
#endif

#if PAL_BUILD_NAVI44
#define DEVICE_ID_NAVI44_94             0x94
#endif

// DEVICE_IS(deviceId, deviceName)
#define DEVICE_IS(d, dn) (d == DEVICE_ID_##dn)

// Gfx11.0
#define DEVICE_IS_NAVI31(d) (DEVICE_IS(d, NV3_NAVI31_P_744C) | DEVICE_IS(d, NV3_NAVI31_P_73BF))
#define DEVICE_IS_NAVI32(d) DEVICE_IS(d, NV3_NAVI32_P_73DF)

// Supported Revision IDs
// Gfx10.1
#define NV_NAVI10_P                  3
#define NV_NAVI12_P                 10
#define NV_NAVI14_M                 20

// Gfx10.3
#define NV_NAVI21_P                 40
#define NV_NAVI22_P                 50
#define NV_NAVI23_P                 60
#define NV_NAVI24_P                 70
#define REMBRANDT_P               0x20
#define RAPHAEL_P                 0x01
#define MENDOCINO_P               0x01

// Gfx11.0
#define NAVI31_P                  0x01
#define NAVI33_P                  0x10
#define NAVI32_P                  0x20
#define PHOENIX1_P                0x01
#define PHOENIX2_P                0x80

// Gfx11.5
#define STRIX1_P                  0x10
#if PAL_BUILD_STRIX_HALO
#define STRIX_HALO_P              0xC0
#endif

// Gfx12
#if PAL_BUILD_NAVI44
#define NAVI44_P                  0x50
#endif

#if PAL_BUILD_NAVI48
#define NAVI48_P                  0x51
#endif

// PRIDs
// Gfx10.1
#define PRID_NV_NAVI10_00           0x00
#define PRID_NV_NAVI12_00           0x00
#define PRID_NV_NAVI14_00           0x00

// Gfx10.3
#define PRID_RMB_00                 0x00
#define PRID_RPL_00                 0x00
#define PRID_MDN_00                 0x00

// Gfx11.0
#define PRID_NV3_NAVI31_00          0x00
#define PRID_NV3_NAVI31_18          0x18
#define PRID_NV3_NAVI31_19          0x19
#define PRID_NV3_NAVI31_C0          0xC0
#define PRID_NV3_NAVI31_C8          0xC8
#define PRID_NV3_NAVI31_CC          0xCC
#define PRID_NV3_NAVI31_CE          0xCE
#define PRID_NV3_NAVI31_CF          0xCF
#define PRID_NV3_NAVI31_D2          0xD2
#define PRID_NV3_NAVI31_D4          0xD4
#define PRID_NV3_NAVI31_D5          0xD5
#define PRID_NV3_NAVI31_D6          0xD6
#define PRID_NV3_NAVI31_D8          0xD8
#define PRID_NV3_NAVI31_D9          0xD9
#define PRID_NV3_NAVI31_DA          0xDA
#define PRID_NV3_NAVI31_DB          0xDB
#define PRID_NV3_NAVI31_E0          0xE0
#define PRID_NV3_NAVI31_EC          0xEC
#define PRID_NV3_NAVI31_EE          0xEE

#define PRID_NV3_NAVI32_00          0x00
#define PRID_NV3_NAVI32_80          0x80
#define PRID_NV3_NAVI32_88          0x88
#define PRID_NV3_NAVI32_89          0x89
#define PRID_NV3_NAVI32_98          0x98
#define PRID_NV3_NAVI32_99          0x99
#define PRID_NV3_NAVI32_9B          0x9B
#define PRID_NV3_NAVI32_BF          0xBF

#define PRID_NV3_NAVI33_00          0x00
#define PRID_NV3_NAVI33_3C          0x3C
#define PRID_NV3_NAVI33_3D          0x3D
#define PRID_NV3_NAVI33_3E          0x3E
#define PRID_NV3_NAVI33_3F          0x3F
#define PRID_NV3_NAVI33_C0          0xC0
#define PRID_NV3_NAVI33_C1          0xC1
#define PRID_NV3_NAVI33_C3          0xC3
#define PRID_NV3_NAVI33_C7          0xC7
#define PRID_NV3_NAVI33_CF          0xCF
#define PRID_NV3_NAVI33_F1          0xF1
#define PRID_NV3_NAVI33_F2          0xF2
#define PRID_NV3_NAVI33_F3          0xF3
#define PRID_NV3_NAVI33_F4          0xF4
#define PRID_NV3_NAVI33_F5          0xF5
#define PRID_NV3_NAVI33_F6          0xF6
#define PRID_NV3_NAVI33_F7          0xF7

#define PRID_PHX_00                 0x00   // Phoenix

// Gfx11.5
#define PRID_STX_STRIX1_00          0x00
#if PAL_BUILD_STRIX_HALO
#define PRID_STX_STRIX_HALO_00      0x00
#endif

// Gfx12
#if PAL_BUILD_NAVI48
#define PRID_NV_NAVI48_00      0x00
#endif

#if PAL_BUILD_NAVI44
#define PRID_NV_NAVI44_00      0x00
#endif

// VARIANT_IS(prid, variantName)
#define VARIANT_IS(v, vn) (v == PRID_##vn)

// Gfx11.0
#define VARIANT_IS_NAVI31_XTX(v) (VARIANT_IS(v, NV3_NAVI31_C8 ) | VARIANT_IS(v, NV3_NAVI31_D4))
#define VARIANT_IS_NAVI32_XL(v) VARIANT_IS(v, NV3_NAVI32_BF)

#endif
