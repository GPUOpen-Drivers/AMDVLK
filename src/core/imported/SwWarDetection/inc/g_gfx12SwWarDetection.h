/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2019-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// WARNING!  WARNING!  WARNING!  WARNING!  WARNING!  WARNING!  WARNING!  WARNING!  WARNING!  WARNING!  WARNING!
//
// This code has been generated automatically. Do not hand-modify this code.
//
// WARNING!  WARNING!  WARNING!  WARNING!  WARNING!  WARNING!  WARNING! WARNING!  WARNING!  WARNING!  WARNING!
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __GFX12_SW_WAR_DETECTION_H__
#define __GFX12_SW_WAR_DETECTION_H__

#include <cstdint>
#include <cstdlib>

#if SWD_STRINGIFY
#include <cstring>
#endif

// Forward declarations:
union Gfx12SwWarDetection;

// =====================================================================================================================
// Determines the target major, minor, and stepping from the specified chip revision.
// Returns true if a target was found, false otherwise. If false, the values in pMajor, pMinor, and pStepping are
// invalid.
extern bool DetermineGfx12Target(
    uint32_t  familyId,
    uint32_t  eRevId,
    uint32_t* pMajor,
    uint32_t* pMinor,
    uint32_t* pStepping);

// =====================================================================================================================
// Main entrypoint. Returns, in place, which hardware bugs require software workarounds for the specified chip revision.
// Returns true if an ASIC was detected. False otherwise.
extern bool DetectGfx12SoftwareWorkaroundsByChip(
    uint32_t             familyId,
    uint32_t             eRevId,
    Gfx12SwWarDetection* pWorkarounds);

// =====================================================================================================================
// Main entrypoint. Returns, in place, which hardware bugs require software workarounds for the specified target based
// on major, minor, and stepping.
// Returns true if an ASIC was detected. False otherwise.
extern bool DetectGfx12SoftwareWorkaroundsByGfxIp(
    uint32_t             major,
    uint32_t             minor,
    uint32_t             stepping,
    Gfx12SwWarDetection* pWorkarounds);

// Number of workarounds that are represented in Gfx12SwWarDetection.
constexpr uint32_t Gfx12NumWorkarounds = 42;

// Number of DWORDs that make up the Gfx12SwWarDetection structure.
constexpr uint32_t Gfx12StructDwords = 2;

// Array of masks representing inactive workaround bits.
// It is expected that the client who consumes these headers will check the u32All of the structure
// against their own copy of the inactive mask. This one is provided as reference.
constexpr uint32_t Gfx12InactiveMask[] =
{
    0x00000000,
    0xfffffc00,
};

// Bitfield structure containing all workarounds active for the Gfx12 family.
union Gfx12SwWarDetection
{
    struct
    {
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t controlCp4xDDIDNotSupported_A_                                                                                                                 : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t shaderSqShaderSqcShaderSqgSQ_SQCAndSQGLegacyPerfCounterUsageIsBrokenWithNewGRBMArch__A_                                                        : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t sioSpiBci12_12412_125GLGWhenSpiGrpLaunchGuaranteeEnable_csGlgDisableIsSetAndGSTriggersGLG_UnexpectedSoftlockMaskIsSetOnHSShader_A_             : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t ppScIssueWithWALKALIGN8PRIMFITSST_1And64KScreenSpace_A_                                                                                        : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t ppScDBD16comp2ndDrawFailedWithFastSet_ZFrom0_50__A_                                                                                            : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t geoGeTessOnGESPIGsgrpMismatchDueToSEStateBeingOutOfSync_A_                                                                                     : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t controlImuIMUBusyNotConsideringRsmuFifoTransaction_A_                                                                                          : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t controlImuIMUBusyNotConsideringRsmuFifoTransaction_B_                                                                                          : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t textureTaGFX12RTTACanIncorrectlyCullProceduralNodes_A_                                                                                         : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t ppDbECRRTLFixForConservativeZPASSCounts_A_                                                                                                     : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

        uint32_t                                                                                                                                                : 1;

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t cmmGl2AtomicOpcodeCONDSUBNoRtnIsNotSupported_A_                                                                                                : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t ppDbudbOreoScoreBoard_udbOsbData_udbOsbdMonitor_ostSampleMaskMismatchOREOScoreboardStoresInvalidEWaveIDAndIncorrectlySetsRespectiveValidBit_A_ : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t cmmGl2GL2CPERFSELSECTORSPerformanceCounterRTLBug_A_                                                                                            : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t shaderSqShaderSqcsGetpcB64DoesntSignExtendTheResult_CausingSubsequentMemoryAccessToFail_A_                                                     : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t controlCpRS64GFX12CPGRTLBugInCpgParserRs64Block_A_                                                                                             : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t geoGeDRAWOPAQUERegUpdatesWithin5CyclesOnDifferentContextsCausesGEIssue_A_                                                                      : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t ppDbShaderSqSioSpiBciPixelWaitSyncPRECOLORModeLeadsToExportHang_A_                                                                             : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t cmmGl2RequestsToSameAddressNotOrderedForIOSpace_A_                                                                                             : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t geoGeGeoPaUpdateToPHMQProgrammingGuideRelatedToTheProgrammingOfPHRingRegisters_A_                                                              : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t ppDbMEMDIFFTOOLZSurfaceMismatchWithXorSwizzleBits_A_                                                                                           : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t textureTcpGfx12MainTCPHangsWhenSClauseHasTooManyInstrWithNoValidThreads_A_                                                                     : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

        uint32_t                                                                                                                                                : 1;

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t shaderSqGFX11GFX12TrapAfterInstructionIsSometimesNotReportedCorrectly_A_                                                                       : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t geoGeGeoPaPpScSioPcSioSpiBciSioSxBackPressureFromSCPC_SCSPICanCauseDeadlock_A_                                                                 : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t shaderSqSSLEEPVARAndSALLOCVGPRAliasWithCertainSQRegisterAddresses_A_                                                                           : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t gcDvIBHangAtASetkillInstructionWhenTheWaveIsInTrapAfterInstModeAndTrapOnEndIsSet_A_                                                            : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t shaderSqVALUSGPRReadFifosGatherMaskDoesNotUpdateCorrectly_A_                                                                                   : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t ppDbDataCorruptionDBFailedToMarkCacheValidForFastSetsTiles_A_                                                                                  : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

        uint32_t                                                                                                                                                : 1;

        uint32_t                                                                                                                                                : 1;

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t selectAllBlocksThatAreAffectedShaderSpsingleuseVdstFalsePositiveKillWhenVOPDInstructionIsPresentInTheSIMD_A_                                   : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

        uint32_t                                                                                                                                                : 1;

        uint32_t                                                                                                                                                : 1;

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t shaderSpInlineConstantsDoNotWorkForPseudoScalarTransF16Opcodes_A_                                                                              : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t shaderSqSQSHGlobalLoadTransposeWritesToOutOfRangeVGPR_A_                                                                                       : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t ppDbTSCEvictionTimeoutCanLeadToSCHangDueToHiZSCacheInflightCountCorruption_A_                                                                  : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_A_                                                                       : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_B_                                                                       : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_C_                                                                       : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t ppDbDBStencilCorruptionDueToMSAA_ZFASTNOOP_StencilFASTSET_A_                                                                                   : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
        uint32_t ppScIncorrectHiStencilUpdateEquationForSResultsOrCanLeadToImageCorruption__A_                                                                  : 1;
#else
        uint32_t                                                                                                                                                : 1;
#endif

        uint32_t reserved                                                                                                                                       : 22;
    };

    uint32_t u32All[Gfx12StructDwords];
};

static_assert(sizeof(Gfx12InactiveMask) == sizeof(Gfx12SwWarDetection),
              "Size of the inactive mask is different than the workaround structure.");

namespace swd_internal
{

#if SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
// =====================================================================================================================
void DetectNavi48A1Workarounds(
    Gfx12SwWarDetection* pWorkarounds)
{
    pWorkarounds->allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_A_                                                                       = 1;
    pWorkarounds->allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_B_                                                                       = 1;
    pWorkarounds->allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_C_                                                                       = 1;
    pWorkarounds->cmmGl2AtomicOpcodeCONDSUBNoRtnIsNotSupported_A_                                                                                                = 1;
    pWorkarounds->cmmGl2GL2CPERFSELSECTORSPerformanceCounterRTLBug_A_                                                                                            = 1;
    pWorkarounds->cmmGl2RequestsToSameAddressNotOrderedForIOSpace_A_                                                                                             = 1;
    pWorkarounds->controlCp4xDDIDNotSupported_A_                                                                                                                 = 1;
    pWorkarounds->controlCpRS64GFX12CPGRTLBugInCpgParserRs64Block_A_                                                                                             = 1;
    pWorkarounds->controlImuIMUBusyNotConsideringRsmuFifoTransaction_A_                                                                                          = 1;
    pWorkarounds->controlImuIMUBusyNotConsideringRsmuFifoTransaction_B_                                                                                          = 1;
    pWorkarounds->gcDvIBHangAtASetkillInstructionWhenTheWaveIsInTrapAfterInstModeAndTrapOnEndIsSet_A_                                                            = 1;
    pWorkarounds->geoGeDRAWOPAQUERegUpdatesWithin5CyclesOnDifferentContextsCausesGEIssue_A_                                                                      = 1;
    pWorkarounds->geoGeGeoPaPpScSioPcSioSpiBciSioSxBackPressureFromSCPC_SCSPICanCauseDeadlock_A_                                                                 = 1;
    pWorkarounds->geoGeGeoPaUpdateToPHMQProgrammingGuideRelatedToTheProgrammingOfPHRingRegisters_A_                                                              = 1;
    pWorkarounds->geoGeTessOnGESPIGsgrpMismatchDueToSEStateBeingOutOfSync_A_                                                                                     = 1;
    pWorkarounds->ppDbDBStencilCorruptionDueToMSAA_ZFASTNOOP_StencilFASTSET_A_                                                                                   = 1;
    pWorkarounds->ppDbDataCorruptionDBFailedToMarkCacheValidForFastSetsTiles_A_                                                                                  = 1;
    pWorkarounds->ppDbECRRTLFixForConservativeZPASSCounts_A_                                                                                                     = 1;
    pWorkarounds->ppDbMEMDIFFTOOLZSurfaceMismatchWithXorSwizzleBits_A_                                                                                           = 1;
    pWorkarounds->ppDbShaderSqSioSpiBciPixelWaitSyncPRECOLORModeLeadsToExportHang_A_                                                                             = 1;
    pWorkarounds->ppDbTSCEvictionTimeoutCanLeadToSCHangDueToHiZSCacheInflightCountCorruption_A_                                                                  = 1;
    pWorkarounds->ppDbudbOreoScoreBoard_udbOsbData_udbOsbdMonitor_ostSampleMaskMismatchOREOScoreboardStoresInvalidEWaveIDAndIncorrectlySetsRespectiveValidBit_A_ = 1;
    pWorkarounds->ppScDBD16comp2ndDrawFailedWithFastSet_ZFrom0_50__A_                                                                                            = 1;
    pWorkarounds->ppScIncorrectHiStencilUpdateEquationForSResultsOrCanLeadToImageCorruption__A_                                                                  = 1;
    pWorkarounds->ppScIssueWithWALKALIGN8PRIMFITSST_1And64KScreenSpace_A_                                                                                        = 1;
    pWorkarounds->selectAllBlocksThatAreAffectedShaderSpsingleuseVdstFalsePositiveKillWhenVOPDInstructionIsPresentInTheSIMD_A_                                   = 1;
    pWorkarounds->shaderSpInlineConstantsDoNotWorkForPseudoScalarTransF16Opcodes_A_                                                                              = 1;
    pWorkarounds->shaderSqGFX11GFX12TrapAfterInstructionIsSometimesNotReportedCorrectly_A_                                                                       = 1;
    pWorkarounds->shaderSqSQSHGlobalLoadTransposeWritesToOutOfRangeVGPR_A_                                                                                       = 1;
    pWorkarounds->shaderSqSSLEEPVARAndSALLOCVGPRAliasWithCertainSQRegisterAddresses_A_                                                                           = 1;
    pWorkarounds->shaderSqShaderSqcShaderSqgSQ_SQCAndSQGLegacyPerfCounterUsageIsBrokenWithNewGRBMArch__A_                                                        = 1;
    pWorkarounds->shaderSqShaderSqcsGetpcB64DoesntSignExtendTheResult_CausingSubsequentMemoryAccessToFail_A_                                                     = 1;
    pWorkarounds->shaderSqVALUSGPRReadFifosGatherMaskDoesNotUpdateCorrectly_A_                                                                                   = 1;
    pWorkarounds->sioSpiBci12_12412_125GLGWhenSpiGrpLaunchGuaranteeEnable_csGlgDisableIsSetAndGSTriggersGLG_UnexpectedSoftlockMaskIsSetOnHSShader_A_             = 1;
    pWorkarounds->textureTaGFX12RTTACanIncorrectlyCullProceduralNodes_A_                                                                                         = 1;
    pWorkarounds->textureTcpGfx12MainTCPHangsWhenSClauseHasTooManyInstrWithNoValidThreads_A_                                                                     = 1;
}
#endif

// =====================================================================================================================
// Perform overrides to the workaround structure.
static void Gfx12OverrideDefaults(
    Gfx12SwWarDetection* pWorkarounds)
{
    char* pOverride = getenv("SWD_GFX12_OVERRIDE");
    char* pMask     = getenv("SWD_GFX12_MASK");

    if ((pOverride != nullptr) && (pMask != nullptr))
    {
        char* pOverrideEnd = pOverride;
        char* pMaskEnd     = pMask;

        uint32_t i = 0;
        while ((pOverrideEnd != nullptr) && (pMaskEnd != nullptr) && (i < Gfx12StructDwords))
        {
            uint32_t overrideVal = strtol(pOverrideEnd, &pOverrideEnd, 16);
            uint32_t maskVal     = strtol(pMaskEnd,     &pMaskEnd,     16);
            pWorkarounds->u32All[i] = (pWorkarounds->u32All[i] & ~maskVal) | (maskVal & overrideVal);

            pOverrideEnd = (pOverrideEnd[0] != '\0') ? pOverrideEnd + 1 : nullptr;
            pMaskEnd     = (pMaskEnd[0]     != '\0') ? pMaskEnd     + 1 : nullptr;
            i++;
        }
    }
}

} // namespace swd_internal

#if SWD_STRINGIFY
// =====================================================================================================================
std::string StringifyActiveGfx12Workarounds(
    const Gfx12SwWarDetection& workarounds)
{
    std::string output = "Workarounds enabled for Gfx12:\n";

#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.controlCp4xDDIDNotSupported_A_ != 0)
    {
        output += " - controlCp4xDDIDNotSupported_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.shaderSqShaderSqcShaderSqgSQ_SQCAndSQGLegacyPerfCounterUsageIsBrokenWithNewGRBMArch__A_ != 0)
    {
        output += " - shaderSqShaderSqcShaderSqgSQ_SQCAndSQGLegacyPerfCounterUsageIsBrokenWithNewGRBMArch__A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.sioSpiBci12_12412_125GLGWhenSpiGrpLaunchGuaranteeEnable_csGlgDisableIsSetAndGSTriggersGLG_UnexpectedSoftlockMaskIsSetOnHSShader_A_ != 0)
    {
        output += " - sioSpiBci12_12412_125GLGWhenSpiGrpLaunchGuaranteeEnable_csGlgDisableIsSetAndGSTriggersGLG_UnexpectedSoftlockMaskIsSetOnHSShader_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.ppScIssueWithWALKALIGN8PRIMFITSST_1And64KScreenSpace_A_ != 0)
    {
        output += " - ppScIssueWithWALKALIGN8PRIMFITSST_1And64KScreenSpace_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.ppScDBD16comp2ndDrawFailedWithFastSet_ZFrom0_50__A_ != 0)
    {
        output += " - ppScDBD16comp2ndDrawFailedWithFastSet_ZFrom0_50__A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.geoGeTessOnGESPIGsgrpMismatchDueToSEStateBeingOutOfSync_A_ != 0)
    {
        output += " - geoGeTessOnGESPIGsgrpMismatchDueToSEStateBeingOutOfSync_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.controlImuIMUBusyNotConsideringRsmuFifoTransaction_A_ != 0)
    {
        output += " - controlImuIMUBusyNotConsideringRsmuFifoTransaction_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.controlImuIMUBusyNotConsideringRsmuFifoTransaction_B_ != 0)
    {
        output += " - controlImuIMUBusyNotConsideringRsmuFifoTransaction_B_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.textureTaGFX12RTTACanIncorrectlyCullProceduralNodes_A_ != 0)
    {
        output += " - textureTaGFX12RTTACanIncorrectlyCullProceduralNodes_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.ppDbECRRTLFixForConservativeZPASSCounts_A_ != 0)
    {
        output += " - ppDbECRRTLFixForConservativeZPASSCounts_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.cmmGl2AtomicOpcodeCONDSUBNoRtnIsNotSupported_A_ != 0)
    {
        output += " - cmmGl2AtomicOpcodeCONDSUBNoRtnIsNotSupported_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.ppDbudbOreoScoreBoard_udbOsbData_udbOsbdMonitor_ostSampleMaskMismatchOREOScoreboardStoresInvalidEWaveIDAndIncorrectlySetsRespectiveValidBit_A_ != 0)
    {
        output += " - ppDbudbOreoScoreBoard_udbOsbData_udbOsbdMonitor_ostSampleMaskMismatchOREOScoreboardStoresInvalidEWaveIDAndIncorrectlySetsRespectiveValidBit_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.cmmGl2GL2CPERFSELSECTORSPerformanceCounterRTLBug_A_ != 0)
    {
        output += " - cmmGl2GL2CPERFSELSECTORSPerformanceCounterRTLBug_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.shaderSqShaderSqcsGetpcB64DoesntSignExtendTheResult_CausingSubsequentMemoryAccessToFail_A_ != 0)
    {
        output += " - shaderSqShaderSqcsGetpcB64DoesntSignExtendTheResult_CausingSubsequentMemoryAccessToFail_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.controlCpRS64GFX12CPGRTLBugInCpgParserRs64Block_A_ != 0)
    {
        output += " - controlCpRS64GFX12CPGRTLBugInCpgParserRs64Block_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.geoGeDRAWOPAQUERegUpdatesWithin5CyclesOnDifferentContextsCausesGEIssue_A_ != 0)
    {
        output += " - geoGeDRAWOPAQUERegUpdatesWithin5CyclesOnDifferentContextsCausesGEIssue_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.ppDbShaderSqSioSpiBciPixelWaitSyncPRECOLORModeLeadsToExportHang_A_ != 0)
    {
        output += " - ppDbShaderSqSioSpiBciPixelWaitSyncPRECOLORModeLeadsToExportHang_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.cmmGl2RequestsToSameAddressNotOrderedForIOSpace_A_ != 0)
    {
        output += " - cmmGl2RequestsToSameAddressNotOrderedForIOSpace_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.geoGeGeoPaUpdateToPHMQProgrammingGuideRelatedToTheProgrammingOfPHRingRegisters_A_ != 0)
    {
        output += " - geoGeGeoPaUpdateToPHMQProgrammingGuideRelatedToTheProgrammingOfPHRingRegisters_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.ppDbMEMDIFFTOOLZSurfaceMismatchWithXorSwizzleBits_A_ != 0)
    {
        output += " - ppDbMEMDIFFTOOLZSurfaceMismatchWithXorSwizzleBits_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.textureTcpGfx12MainTCPHangsWhenSClauseHasTooManyInstrWithNoValidThreads_A_ != 0)
    {
        output += " - textureTcpGfx12MainTCPHangsWhenSClauseHasTooManyInstrWithNoValidThreads_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.shaderSqGFX11GFX12TrapAfterInstructionIsSometimesNotReportedCorrectly_A_ != 0)
    {
        output += " - shaderSqGFX11GFX12TrapAfterInstructionIsSometimesNotReportedCorrectly_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.geoGeGeoPaPpScSioPcSioSpiBciSioSxBackPressureFromSCPC_SCSPICanCauseDeadlock_A_ != 0)
    {
        output += " - geoGeGeoPaPpScSioPcSioSpiBciSioSxBackPressureFromSCPC_SCSPICanCauseDeadlock_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.shaderSqSSLEEPVARAndSALLOCVGPRAliasWithCertainSQRegisterAddresses_A_ != 0)
    {
        output += " - shaderSqSSLEEPVARAndSALLOCVGPRAliasWithCertainSQRegisterAddresses_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.gcDvIBHangAtASetkillInstructionWhenTheWaveIsInTrapAfterInstModeAndTrapOnEndIsSet_A_ != 0)
    {
        output += " - gcDvIBHangAtASetkillInstructionWhenTheWaveIsInTrapAfterInstModeAndTrapOnEndIsSet_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.shaderSqVALUSGPRReadFifosGatherMaskDoesNotUpdateCorrectly_A_ != 0)
    {
        output += " - shaderSqVALUSGPRReadFifosGatherMaskDoesNotUpdateCorrectly_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.ppDbDataCorruptionDBFailedToMarkCacheValidForFastSetsTiles_A_ != 0)
    {
        output += " - ppDbDataCorruptionDBFailedToMarkCacheValidForFastSetsTiles_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.selectAllBlocksThatAreAffectedShaderSpsingleuseVdstFalsePositiveKillWhenVOPDInstructionIsPresentInTheSIMD_A_ != 0)
    {
        output += " - selectAllBlocksThatAreAffectedShaderSpsingleuseVdstFalsePositiveKillWhenVOPDInstructionIsPresentInTheSIMD_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.shaderSpInlineConstantsDoNotWorkForPseudoScalarTransF16Opcodes_A_ != 0)
    {
        output += " - shaderSpInlineConstantsDoNotWorkForPseudoScalarTransF16Opcodes_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.shaderSqSQSHGlobalLoadTransposeWritesToOutOfRangeVGPR_A_ != 0)
    {
        output += " - shaderSqSQSHGlobalLoadTransposeWritesToOutOfRangeVGPR_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.ppDbTSCEvictionTimeoutCanLeadToSCHangDueToHiZSCacheInflightCountCorruption_A_ != 0)
    {
        output += " - ppDbTSCEvictionTimeoutCanLeadToSCHangDueToHiZSCacheInflightCountCorruption_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_A_ != 0)
    {
        output += " - allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_B_ != 0)
    {
        output += " - allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_B_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_C_ != 0)
    {
        output += " - allSubsystems4xSBufferLoadU16WithStridedBuffersReturns0InsteadOfValue_C_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.ppDbDBStencilCorruptionDueToMSAA_ZFASTNOOP_StencilFASTSET_A_ != 0)
    {
        output += " - ppDbDBStencilCorruptionDueToMSAA_ZFASTNOOP_StencilFASTSET_A_\n";
    }
#endif
#if   SWD_BUILD_NAVI48 || SWD_BUILD_NAVI44
    if (workarounds.ppScIncorrectHiStencilUpdateEquationForSResultsOrCanLeadToImageCorruption__A_ != 0)
    {
        output += " - ppScIncorrectHiStencilUpdateEquationForSResultsOrCanLeadToImageCorruption__A_\n";
    }
#endif

    return output;
}

// =====================================================================================================================
std::string StringifyActiveGfx12Workarounds(
    const uint32_t* pWorkarounds)
{
    Gfx12SwWarDetection workarounds = {};
    memcpy(&workarounds.u32All[0], pWorkarounds, Gfx12StructDwords * sizeof(uint32_t));
    return StringifyActiveGfx12Workarounds(workarounds);
}
#endif

// =====================================================================================================================
bool DetermineGfx12Target(
    uint32_t        familyId,
    uint32_t        eRevId,
    uint32_t*       pMajor,
    uint32_t*       pMinor,
    uint32_t*       pStepping)
{
    bool successful = false;

    if (false)
    {
    }
#if SWD_BUILD_NAVI4X
    else if (familyId == 152)
    {
        if (false)
        {
        }
#if SWD_BUILD_NAVI48
        else if ((0x51 <= eRevId) && (eRevId < 0xFF))
        {
            (*pMajor)    = 12;
            (*pMinor)    = 0;
            (*pStepping) = 1;
            successful   = true;
        }
#endif
#if SWD_BUILD_NAVI44
        else if ((0x01 <= eRevId) && (eRevId < 0x50))
        {
            (*pMajor)    = 12;
            (*pMinor)    = 0;
            (*pStepping) = 0;
            successful   = true;
        }
#endif
    }
#endif

    return successful;
}

// =====================================================================================================================
bool DetectGfx12SoftwareWorkaroundsByChip(
    uint32_t             familyId,
    uint32_t             eRevId,
    Gfx12SwWarDetection* pWorkarounds)
{
    bool successful = false;

    if (false)
    {
    }
#if SWD_BUILD_NAVI4X
    else if (familyId == 152)
    {
        if (false)
        {
        }
#if SWD_BUILD_NAVI48
        else if ((0x51 <= eRevId) && (eRevId < 0xFF))
        {
            swd_internal::DetectNavi48A1Workarounds(pWorkarounds);
            successful = true;
#endif
#if SWD_BUILD_NAVI44
        }
        else if ((0x01 <= eRevId) && (eRevId < 0x50))
        {
            swd_internal::DetectNavi48A1Workarounds(pWorkarounds);
            successful = true;
#endif
    }
}
#endif

    if (successful)
    {
        swd_internal::Gfx12OverrideDefaults(pWorkarounds);
    }

    return successful;
}

// =====================================================================================================================
bool DetectGfx12SoftwareWorkaroundsByGfxIp(
    uint32_t             major,
    uint32_t             minor,
    uint32_t             stepping,
    Gfx12SwWarDetection* pWorkarounds)
{
    bool successful = false;

    if (false)
    {
    }
#if SWD_BUILD_NAVI48
    else if ((major == 12) && (minor == 0) && (stepping == 1))
    {
        swd_internal::DetectNavi48A1Workarounds(pWorkarounds);
        successful = true;
    }
#endif
#if SWD_BUILD_NAVI44
    else if ((major == 12) && (minor == 0) && (stepping == 0))
    {
        swd_internal::DetectNavi48A1Workarounds(pWorkarounds);
        successful = true;
    }
#endif

    if (successful)
    {
        swd_internal::Gfx12OverrideDefaults(pWorkarounds);
    }

    return successful;
}

#endif
