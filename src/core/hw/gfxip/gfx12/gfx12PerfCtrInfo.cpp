/*
 ***********************************************************************************************************************
 *
 *  Copyright (c) 2023-2025 Advanced Micro Devices, Inc. All Rights Reserved.
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
#include "core/hw/gfxip/gfx12/gfx12Chip.h"
#include "core/hw/gfxip/gfx12/gfx12PerfCtrInfo.h"

using namespace Util;

namespace Pal
{
namespace Gfx12
{

enum Gfx12SpmGlobalBlockSelect : uint32
{
    Gfx12SpmGlobalBlockSelectCpg     = 0,
    Gfx12SpmGlobalBlockSelectCpc     = 1,
    Gfx12SpmGlobalBlockSelectCpf     = 2,
    Gfx12SpmGlobalBlockSelectGds     = 3,
    Gfx12SpmGlobalBlockSelectGcr     = 4,
    Gfx12SpmGlobalBlockSelectPh      = 5,
    Gfx12SpmGlobalBlockSelectGe1     = 6,
    Gfx12SpmGlobalBlockSelectGl2a    = 7,
    Gfx12SpmGlobalBlockSelectGl2c    = 8,
    Gfx12SpmGlobalBlockSelectSdma    = 9,
    Gfx12SpmGlobalBlockSelectGus     = 10,
    Gfx12SpmGlobalBlockSelectEa      = 11,
    Gfx12SpmGlobalBlockSelectCha     = 12,
    Gfx12SpmGlobalBlockSelectChc     = 13,
    Gfx12SpmGlobalBlockSelectChcg    = 14,
    Gfx12SpmGlobalBlockSelectAtcl2   = 15,
    Gfx12SpmGlobalBlockSelectVml2    = 16,
    Gfx12SpmGlobalBlockSelectGe2Se   = 17,
    Gfx12SpmGlobalBlockSelectGe2Dist = 18,
    Gfx12SpmGlobalBlockSelectFfbm    = 19,
    Gfx12SpmGlobalBlockSelectCane    = 20,
    Gfx12SpmGlobalBlockSelectRspm    = 31,
};

enum Gfx12SpmSeBlockSelect : uint32
{
    Gfx12SpmSeBlockSelectCb    = 0,
    Gfx12SpmSeBlockSelectDb    = 1,
    Gfx12SpmSeBlockSelectPa    = 2,
    Gfx12SpmSeBlockSelectSx    = 3,
    Gfx12SpmSeBlockSelectSc    = 4,
    Gfx12SpmSeBlockSelectTa    = 5,
    Gfx12SpmSeBlockSelectTd    = 6,
    Gfx12SpmSeBlockSelectTcp   = 7,
    Gfx12SpmSeBlockSelectSpi   = 8,
    Gfx12SpmSeBlockSelectSqg   = 9,
    Gfx12SpmSeBlockSelectGl1a  = 10,
    Gfx12SpmSeBlockSelectRmi   = 11,
    Gfx12SpmSeBlockSelectGl1c  = 12,
    Gfx12SpmSeBlockSelectGl1cg = 13,
    Gfx12SpmSeBlockSelectCbr   = 14,
    Gfx12SpmSeBlockSelectDbr   = 15,
    Gfx12SpmSeBlockSelectGl1h  = 16,
    Gfx12SpmSeBlockSelectSqc   = 17,
    Gfx12SpmSeBlockSelectPc    = 18,
    Gfx12SpmSeBlockSelectEa    = 19,
    Gfx12SpmSeBlockSelectGe    = 20,
    Gfx12SpmSeBlockSelectGl2a  = 21,
    Gfx12SpmSeBlockSelectGl2c  = 22,
    Gfx12SpmSeBlockSelectWgs   = 23,
    Gfx12SpmSeBlockSelectGl1xa = 24,
    Gfx12SpmSeBlockSelectGl1xc = 25,
    Gfx12SpmSeBlockSelectUtcl1 = 26,
    Gfx12SpmSeBlockSelectSeRpm = 31,
};

// There's a terrifyingly large number of UMCCH registers. This macro makes UpdateUmcchBlockInfo much more sane.
#define SET_UMCCH_INSTANCE_REGS(Ns, Idx) \
    pInfo->umcchRegAddr[Idx].perfMonCtlClk = Ns::mmUMCCH##Idx##_PerfMonCtlClk; \
    pInfo->umcchRegAddr[Idx].perModule[0]  = { Ns::mmUMCCH##Idx##_PerfMonCtl1, 0,  Ns::mmUMCCH##Idx##_PerfMonCtr1_Lo,  Ns::mmUMCCH##Idx##_PerfMonCtr1_Hi  }; \
    pInfo->umcchRegAddr[Idx].perModule[1]  = { Ns::mmUMCCH##Idx##_PerfMonCtl2, 0,  Ns::mmUMCCH##Idx##_PerfMonCtr2_Lo,  Ns::mmUMCCH##Idx##_PerfMonCtr2_Hi  }; \
    pInfo->umcchRegAddr[Idx].perModule[2]  = { Ns::mmUMCCH##Idx##_PerfMonCtl3, 0,  Ns::mmUMCCH##Idx##_PerfMonCtr3_Lo,  Ns::mmUMCCH##Idx##_PerfMonCtr3_Hi  }; \
    pInfo->umcchRegAddr[Idx].perModule[3]  = { Ns::mmUMCCH##Idx##_PerfMonCtl4, 0,  Ns::mmUMCCH##Idx##_PerfMonCtr4_Lo,  Ns::mmUMCCH##Idx##_PerfMonCtr4_Hi  }; \
    pInfo->umcchRegAddr[Idx].perModule[4]  = { Ns::mmUMCCH##Idx##_PerfMonCtl5, 0,  Ns::mmUMCCH##Idx##_PerfMonCtr5_Lo,  Ns::mmUMCCH##Idx##_PerfMonCtr5_Hi  }; \
    pInfo->umcchRegAddr[Idx].perModule[5]  = { Ns::mmUMCCH##Idx##_PerfMonCtl6, 0,  Ns::mmUMCCH##Idx##_PerfMonCtr6_Lo,  Ns::mmUMCCH##Idx##_PerfMonCtr6_Hi  }; \
    pInfo->umcchRegAddr[Idx].perModule[6]  = { Ns::mmUMCCH##Idx##_PerfMonCtl7, 0,  Ns::mmUMCCH##Idx##_PerfMonCtr7_Lo,  Ns::mmUMCCH##Idx##_PerfMonCtr7_Hi  }; \
    pInfo->umcchRegAddr[Idx].perModule[7]  = { Ns::mmUMCCH##Idx##_PerfMonCtl8, 0,  Ns::mmUMCCH##Idx##_PerfMonCtr8_Lo,  Ns::mmUMCCH##Idx##_PerfMonCtr8_Hi  }; \
    pInfo->umcchRegAddr[Idx].perModule[8]  = { Ns::mmUMCCH##Idx##_PerfMonCtl9, 0,  Ns::mmUMCCH##Idx##_PerfMonCtr9_Lo,  Ns::mmUMCCH##Idx##_PerfMonCtr9_Hi  }; \
    pInfo->umcchRegAddr[Idx].perModule[9]  = { Ns::mmUMCCH##Idx##_PerfMonCtl10, 0, Ns::mmUMCCH##Idx##_PerfMonCtr10_Lo, Ns::mmUMCCH##Idx##_PerfMonCtr10_Hi }; \
    pInfo->umcchRegAddr[Idx].perModule[10] = { Ns::mmUMCCH##Idx##_PerfMonCtl11, 0, Ns::mmUMCCH##Idx##_PerfMonCtr11_Lo, Ns::mmUMCCH##Idx##_PerfMonCtr11_Hi }; \
    pInfo->umcchRegAddr[Idx].perModule[11] = { Ns::mmUMCCH##Idx##_PerfMonCtl12, 0, Ns::mmUMCCH##Idx##_PerfMonCtr12_Lo, Ns::mmUMCCH##Idx##_PerfMonCtr12_Hi };

typedef unsigned int MaxEventIds[MaxPerfCtrId];
constexpr MaxEventIds UnknownMaxEventIds = {};

// =====================================================================================================================
// Get an array with the maximum values of each perfcounter for this device
static const MaxEventIds& GetEventLimits(
    const Pal::Device& device)
{
    const MaxEventIds* pOut = nullptr;
    switch(device.ChipProperties().revision)
    {
#if PAL_BUILD_NAVI48
    case Pal::AsicRevision::Navi48:
        pOut = &Nv48MaxPerfEventIds;
        break;
#endif
#if PAL_BUILD_NAVI44
    case Pal::AsicRevision::Navi44:
        pOut = &Nv48MaxPerfEventIds;
        break;
#endif
    default:
        PAL_ASSERT_ALWAYS(); // What chip is this?
        pOut = &UnknownMaxEventIds;
    }
    return *pOut;
}

// =====================================================================================================================
// Initializes each block's basic hardware-defined information
// (distribution, numScopedInstances, numGenericSpmModules, etc.)
static void Gfx12InitBasicBlockInfo(
    const Pal::Device& device,
    GpuChipProperties* pProps)
{
    PerfCounterInfo*const pInfo   = &pProps->gfx9.perfCounterInfo.gfx12Info;
    const MaxEventIds&    maxIds  = GetEventLimits(device);
    const GfxIpLevel      gfxip   = device.ChipProperties().gfxLevel;
    const uint32          rbPerSa = pProps->gfx9.maxNumRbPerSe / pProps->gfx9.numShaderArrays;

    // Start by hard-coding hardware specific constants for each block.
    //
    // The distribution and numScopedInstances (per-distribution) are derived from our hardware architecture.
    // The generic module counts are determined by:
    //   1. Does the block follow the generic programming model as defined by the perf experiment code?
    //   2. If so, there's one SPM module for each SELECT/SELECT1 pair and one legacy module for the remaining SELECTs.
    // The number of SPM wires is a hardware constant baked into each ASIC's design. So are the SPM block selects.
    // The maximum event IDs are the largest values from the hardware perf_sel enums.
    // Finally, we hard-code the PERFCOUNTER# register addresses for each module.

    PerfCounterBlockInfo*const pCpf = &pInfo->block[static_cast<uint32>(GpuBlock::Cpf)];
    pCpf->distribution              = PerfCounterDistribution::GlobalBlock;
    pCpf->numScopedInstances        = 1;
    pCpf->numGenericSpmModules      = 1; // CPF_PERFCOUNTER0
    pCpf->numGenericLegacyModules   = 1; // CPF_PERFCOUNTER1
    pCpf->numSpmWires               = 2;
    pCpf->spmBlockSelect            = Gfx12SpmGlobalBlockSelectCpf;
    pCpf->maxEventId                = maxIds[CpfPerfcountSelId];

    pCpf->regAddr = { 0, {
        { mmCPF_PERFCOUNTER0_SELECT, mmCPF_PERFCOUNTER0_SELECT1, mmCPF_PERFCOUNTER0_LO, mmCPF_PERFCOUNTER0_HI },
        { mmCPF_PERFCOUNTER1_SELECT, 0,                          mmCPF_PERFCOUNTER1_LO, mmCPF_PERFCOUNTER1_HI },
    }};

    // There is only 1 PA instance per SE in gfx12.
    PerfCounterBlockInfo*const pPa = &pInfo->block[static_cast<uint32>(GpuBlock::Pa)];
    pPa->distribution              = PerfCounterDistribution::PerShaderEngine;
    pPa->numScopedInstances        = 1;
    pPa->numGenericSpmModules      = 4; // PA_SU_PERFCOUNTER0-3
    pPa->numGenericLegacyModules   = 0;
    pPa->numSpmWires               = 8;
    pPa->spmBlockSelect            = Gfx12SpmSeBlockSelectPa;
    pPa->maxEventId                = maxIds[SuPerfcntSelId];

    pPa->regAddr = { 0, {
        { mmPA_SU_PERFCOUNTER0_SELECT, mmPA_SU_PERFCOUNTER0_SELECT1, mmPA_SU_PERFCOUNTER0_LO, mmPA_SU_PERFCOUNTER0_HI },
        { mmPA_SU_PERFCOUNTER1_SELECT, mmPA_SU_PERFCOUNTER1_SELECT1, mmPA_SU_PERFCOUNTER1_LO, mmPA_SU_PERFCOUNTER1_HI },
        { mmPA_SU_PERFCOUNTER2_SELECT, mmPA_SU_PERFCOUNTER2_SELECT1, mmPA_SU_PERFCOUNTER2_LO, mmPA_SU_PERFCOUNTER2_HI },
        { mmPA_SU_PERFCOUNTER3_SELECT, mmPA_SU_PERFCOUNTER3_SELECT1, mmPA_SU_PERFCOUNTER3_LO, mmPA_SU_PERFCOUNTER3_HI },
    }};

    // In gfx12 SC is subdivided into SCF (SCT) and 2xSCB per SA. The sets of perf counters (PA_SC_PERFCOUNTER{0-7})
    // are instantiated in each of the two SCBs. In the hardware docs these are called packers, thus we're really
    // gathering perf counters from individual packer instances.
    PerfCounterBlockInfo*const pSc = &pInfo->block[static_cast<uint32>(GpuBlock::Sc)];
    pSc->distribution              = PerfCounterDistribution::PerShaderArray;
    pSc->numScopedInstances        = 2;
    pSc->numGenericSpmModules      = 1; // PA_SC_PERFCOUNTER0
    pSc->numGenericLegacyModules   = 7; // PA_SC_PERFCOUNTER1-7
    pSc->numSpmWires               = 2;
    pSc->spmBlockSelect            = Gfx12SpmSeBlockSelectSc;
    pSc->maxEventId                = maxIds[ScPerfcntSelId];

    pSc->regAddr = { 0, {
        { mmPA_SC_PERFCOUNTER0_SELECT, mmPA_SC_PERFCOUNTER0_SELECT1, mmPA_SC_PERFCOUNTER0_LO, mmPA_SC_PERFCOUNTER0_HI },
        { mmPA_SC_PERFCOUNTER1_SELECT, 0,                            mmPA_SC_PERFCOUNTER1_LO, mmPA_SC_PERFCOUNTER1_HI },
        { mmPA_SC_PERFCOUNTER2_SELECT, 0,                            mmPA_SC_PERFCOUNTER2_LO, mmPA_SC_PERFCOUNTER2_HI },
        { mmPA_SC_PERFCOUNTER3_SELECT, 0,                            mmPA_SC_PERFCOUNTER3_LO, mmPA_SC_PERFCOUNTER3_HI },
        { mmPA_SC_PERFCOUNTER4_SELECT, 0,                            mmPA_SC_PERFCOUNTER4_LO, mmPA_SC_PERFCOUNTER4_HI },
        { mmPA_SC_PERFCOUNTER5_SELECT, 0,                            mmPA_SC_PERFCOUNTER5_LO, mmPA_SC_PERFCOUNTER5_HI },
        { mmPA_SC_PERFCOUNTER6_SELECT, 0,                            mmPA_SC_PERFCOUNTER6_LO, mmPA_SC_PERFCOUNTER6_HI },
        { mmPA_SC_PERFCOUNTER7_SELECT, 0,                            mmPA_SC_PERFCOUNTER7_LO, mmPA_SC_PERFCOUNTER7_HI },
    }};

    PerfCounterBlockInfo*const pSpi = &pInfo->block[static_cast<uint32>(GpuBlock::Spi)];
    pSpi->distribution              = PerfCounterDistribution::PerShaderEngine;
    pSpi->numScopedInstances        = 1;
    pSpi->numGenericSpmModules      = 6; // SPI_PERFCOUNTER0-5
    pSpi->numGenericLegacyModules   = 0;
    pSpi->numSpmWires               = 12;
    pSpi->spmBlockSelect            = Gfx12SpmSeBlockSelectSpi;
    pSpi->maxEventId                = maxIds[SpiPerfcntSelId];

    pSpi->regAddr = { 0, {
        { mmSPI_PERFCOUNTER0_SELECT, mmSPI_PERFCOUNTER0_SELECT1, mmSPI_PERFCOUNTER0_LO, mmSPI_PERFCOUNTER0_HI },
        { mmSPI_PERFCOUNTER1_SELECT, mmSPI_PERFCOUNTER1_SELECT1, mmSPI_PERFCOUNTER1_LO, mmSPI_PERFCOUNTER1_HI },
        { mmSPI_PERFCOUNTER2_SELECT, mmSPI_PERFCOUNTER2_SELECT1, mmSPI_PERFCOUNTER2_LO, mmSPI_PERFCOUNTER2_HI },
        { mmSPI_PERFCOUNTER3_SELECT, mmSPI_PERFCOUNTER3_SELECT1, mmSPI_PERFCOUNTER3_LO, mmSPI_PERFCOUNTER3_HI },
        { mmSPI_PERFCOUNTER4_SELECT, mmSPI_PERFCOUNTER4_SELECT1, mmSPI_PERFCOUNTER4_LO, mmSPI_PERFCOUNTER4_HI },
        { mmSPI_PERFCOUNTER5_SELECT, mmSPI_PERFCOUNTER5_SELECT1, mmSPI_PERFCOUNTER5_LO, mmSPI_PERFCOUNTER5_HI },
    }};

    // There are changes to the SQ perf counters from previous chips, but basically it's been
    // reduced from 16 counters to 8 and the counters have been reduced to 32-bit counters.
    PerfCounterBlockInfo*const pSqWgp = &pInfo->block[static_cast<uint32>(GpuBlock::SqWgp)];
    pSqWgp->distribution              = PerfCounterDistribution::PerShaderArray;
    // maxNumWgpPerSa is the sum of gfx10.numWgpAboveSpi and gfx10.numWgpBelowSpi
    pSqWgp->numScopedInstances        = pProps->gfx9.gfx10.maxNumWgpPerSa;
    pSqWgp->num16BitSpmCounters       = 16;
    pSqWgp->num32BitSpmCounters       = 8;
    pSqWgp->numGlobalSharedCounters   = 8;
    pSqWgp->numGenericSpmModules      = 0;
    pSqWgp->numGenericLegacyModules   = 0;
    pSqWgp->numSpmWires               = 8;
    pSqWgp->spmBlockSelect            = Gfx12SpmSeBlockSelectSqc;
    pSqWgp->maxEventId                = maxIds[SqPerfSelId];

   // section 1.5
   // Legacy perfcounters use a pair of SPM counters,
   // so in legacy mode you can only use counters 0, 2, 4, 6, 8, 10, 12, 14.
    pSqWgp->regAddr = { 0, {
        { mmSQ_PERFCOUNTER0_SELECT,   0, mmSQ_PERFCOUNTER0_LO,  0 },
        { mmSQ_PERFCOUNTER1_SELECT,   0,                    0,  0 },
        { mmSQ_PERFCOUNTER2_SELECT,   0, mmSQ_PERFCOUNTER1_LO,  0 },
        { mmSQ_PERFCOUNTER3_SELECT,   0,                    0,  0 },
        { mmSQ_PERFCOUNTER4_SELECT,   0, mmSQ_PERFCOUNTER2_LO,  0 },
        { mmSQ_PERFCOUNTER5_SELECT,   0,                    0,  0 },
        { mmSQ_PERFCOUNTER6_SELECT,   0, mmSQ_PERFCOUNTER3_LO,  0 },
        { mmSQ_PERFCOUNTER7_SELECT,   0,                    0,  0 },
        { mmSQ_PERFCOUNTER8_SELECT,   0, mmSQ_PERFCOUNTER4_LO,  0 },
        { mmSQ_PERFCOUNTER9_SELECT,   0,                    0,  0 },
        { mmSQ_PERFCOUNTER10_SELECT,  0, mmSQ_PERFCOUNTER5_LO,  0 },
        { mmSQ_PERFCOUNTER11_SELECT,  0,                    0,  0 },
        { mmSQ_PERFCOUNTER12_SELECT,  0, mmSQ_PERFCOUNTER6_LO,  0 },
        { mmSQ_PERFCOUNTER13_SELECT,  0,                    0,  0 },
        { mmSQ_PERFCOUNTER14_SELECT,  0, mmSQ_PERFCOUNTER7_LO,  0 },
        { mmSQ_PERFCOUNTER15_SELECT,  0,                    0,  0 },
    } };

    PerfCounterBlockInfo* const pSq = &pInfo->block[static_cast<uint32>(GpuBlock::Sq)];
    pSq->distribution            = PerfCounterDistribution::PerShaderEngine;
    pSq->numScopedInstances      = 1;
    pSq->num16BitSpmCounters     = 0;
    pSq->num32BitSpmCounters     = 8; // Force since the counters must be used as 32bit
    pSq->numGenericSpmModules    = 8; // mmSQG_PERFCOUNTER0-7
    pSq->numGenericLegacyModules = 0;
    pSq->numSpmWires             = 8;
    pSq->spmBlockSelect          = Gfx12SpmSeBlockSelectSqg;
    pSq->maxEventId              = maxIds[SqgPerfSelId];

    pSq->regAddr = { 0, {
        { mmSQG_PERFCOUNTER0_SELECT, 0, mmSQG_PERFCOUNTER0_LO, mmSQG_PERFCOUNTER0_HI },
        { mmSQG_PERFCOUNTER1_SELECT, 0, mmSQG_PERFCOUNTER1_LO, mmSQG_PERFCOUNTER1_HI },
        { mmSQG_PERFCOUNTER2_SELECT, 0, mmSQG_PERFCOUNTER2_LO, mmSQG_PERFCOUNTER2_HI },
        { mmSQG_PERFCOUNTER3_SELECT, 0, mmSQG_PERFCOUNTER3_LO, mmSQG_PERFCOUNTER3_HI },
        { mmSQG_PERFCOUNTER4_SELECT, 0, mmSQG_PERFCOUNTER4_LO, mmSQG_PERFCOUNTER4_HI },
        { mmSQG_PERFCOUNTER5_SELECT, 0, mmSQG_PERFCOUNTER5_LO, mmSQG_PERFCOUNTER5_HI },
        { mmSQG_PERFCOUNTER6_SELECT, 0, mmSQG_PERFCOUNTER6_LO, mmSQG_PERFCOUNTER6_HI },
        { mmSQG_PERFCOUNTER7_SELECT, 0, mmSQG_PERFCOUNTER7_LO, mmSQG_PERFCOUNTER7_HI },
    } };

    // The SX not a single block and thus has per-SE and per-SA qualities. For example, the SX crossbar routes requests
    // between SAs so it lives in the SE. However, the "interesting bits" of the SX are split in half, one half in
    // each SA. Perfcounter requests are forwarded to one half of the SX using the SA index so for us it's per-SA.
    PerfCounterBlockInfo*const pSx = &pInfo->block[static_cast<uint32>(GpuBlock::Sx)];
    pSx->distribution              = PerfCounterDistribution::PerShaderArray;
    pSx->numScopedInstances        = 1;
    pSx->numGenericSpmModules      = 4; // SX_PERFCOUNTER0-3
    pSx->numGenericLegacyModules   = 0;
    pSx->numSpmWires               = 8;
    pSx->spmBlockSelect            = Gfx12SpmSeBlockSelectSx;
    pSx->maxEventId                = maxIds[SxPerfcounterValsId];

    pSx->regAddr = { 0, {
        { mmSX_PERFCOUNTER0_SELECT, mmSX_PERFCOUNTER0_SELECT1, mmSX_PERFCOUNTER0_LO, mmSX_PERFCOUNTER0_HI },
        { mmSX_PERFCOUNTER1_SELECT, mmSX_PERFCOUNTER1_SELECT1, mmSX_PERFCOUNTER1_LO, mmSX_PERFCOUNTER1_HI },
        { mmSX_PERFCOUNTER2_SELECT, mmSX_PERFCOUNTER2_SELECT1, mmSX_PERFCOUNTER2_LO, mmSX_PERFCOUNTER2_HI },
        { mmSX_PERFCOUNTER3_SELECT, mmSX_PERFCOUNTER3_SELECT1, mmSX_PERFCOUNTER3_LO, mmSX_PERFCOUNTER3_HI },
    }};

    PerfCounterBlockInfo*const pTa = &pInfo->block[static_cast<uint32>(GpuBlock::Ta)];
    pTa->distribution              = PerfCounterDistribution::PerShaderArray;
    pTa->numScopedInstances        = pProps->gfx9.numCuPerSh;
    pTa->numGenericSpmModules      = 1; // TA_PERFCOUNTER0
    pTa->numGenericLegacyModules   = 1; // TA_PERFCOUNTER1
    pTa->numSpmWires               = 2;
    pTa->spmBlockSelect            = Gfx12SpmSeBlockSelectTa;
    pTa->maxEventId                = maxIds[TaPerfcountSelId];

    pTa->regAddr = { 0, {
        { mmTA_PERFCOUNTER0_SELECT, mmTA_PERFCOUNTER0_SELECT1, mmTA_PERFCOUNTER0_LO, mmTA_PERFCOUNTER0_HI },
        { mmTA_PERFCOUNTER1_SELECT, 0,                         mmTA_PERFCOUNTER1_LO, mmTA_PERFCOUNTER1_HI },
    }};

    PerfCounterBlockInfo*const pTd = &pInfo->block[static_cast<uint32>(GpuBlock::Td)];
    pTd->distribution              = PerfCounterDistribution::PerShaderArray;
    pTd->numScopedInstances        = pProps->gfx9.numCuPerSh;
    pTd->numGenericSpmModules      = 1; // TD_PERFCOUNTER0
    pTd->numGenericLegacyModules   = 1; // TD_PERFCOUNTER1
    pTd->numSpmWires               = 2;
    pTd->spmBlockSelect            = Gfx12SpmSeBlockSelectTd;
    pTd->maxEventId                = maxIds[TdPerfcountSelId];

    pTd->regAddr = { 0, {
        { mmTD_PERFCOUNTER0_SELECT, mmTD_PERFCOUNTER0_SELECT1, mmTD_PERFCOUNTER0_LO, mmTD_PERFCOUNTER0_HI },
        { mmTD_PERFCOUNTER1_SELECT, 0,                         mmTD_PERFCOUNTER1_LO, mmTD_PERFCOUNTER1_HI },
    }};

    PerfCounterBlockInfo*const pTcp = &pInfo->block[static_cast<uint32>(GpuBlock::Tcp)];
    pTcp->distribution              = PerfCounterDistribution::PerShaderArray;
    pTcp->numScopedInstances        = pProps->gfx9.gfx10.numTcpPerSa;
    pTcp->numGenericSpmModules      = 2; // TCP_PERFCOUNTER0-1
    pTcp->numGenericLegacyModules   = 2; // TCP_PERFCOUNTER2-3
    pTcp->numSpmWires               = 4;
    pTcp->spmBlockSelect            = Gfx12SpmSeBlockSelectTcp;
    pTcp->maxEventId                = maxIds[TcpPerfcountSelectId];

    pTcp->regAddr = { 0, {
        { mmTCP_PERFCOUNTER0_SELECT, mmTCP_PERFCOUNTER0_SELECT1, mmTCP_PERFCOUNTER0_LO, mmTCP_PERFCOUNTER0_HI },
        { mmTCP_PERFCOUNTER1_SELECT, mmTCP_PERFCOUNTER1_SELECT1, mmTCP_PERFCOUNTER1_LO, mmTCP_PERFCOUNTER1_HI },
        { mmTCP_PERFCOUNTER2_SELECT, 0,                          mmTCP_PERFCOUNTER2_LO, mmTCP_PERFCOUNTER2_HI },
        { mmTCP_PERFCOUNTER3_SELECT, 0,                          mmTCP_PERFCOUNTER3_LO, mmTCP_PERFCOUNTER3_HI },
    }};

    PerfCounterBlockInfo*const pDb = &pInfo->block[static_cast<uint32>(GpuBlock::Db)];
    pDb->distribution              = PerfCounterDistribution::PerShaderArray;
    pDb->numScopedInstances        = rbPerSa;
    pDb->numGenericSpmModules      = 4; // DB_PERFCOUNTER0-3
    pDb->numGenericLegacyModules   = 0;
    pDb->numSpmWires               = 8;
    pDb->spmBlockSelect            = Gfx12SpmSeBlockSelectDb;
    pDb->maxEventId                = maxIds[PerfcounterValsId]; // Enum id for DB is not that clear enough

    pDb->regAddr = { 0, {
        { mmDB_PERFCOUNTER0_SELECT, mmDB_PERFCOUNTER0_SELECT1, mmDB_PERFCOUNTER0_LO, mmDB_PERFCOUNTER0_HI },
        { mmDB_PERFCOUNTER1_SELECT, mmDB_PERFCOUNTER1_SELECT1, mmDB_PERFCOUNTER1_LO, mmDB_PERFCOUNTER1_HI },
        { mmDB_PERFCOUNTER2_SELECT, mmDB_PERFCOUNTER2_SELECT1, mmDB_PERFCOUNTER2_LO, mmDB_PERFCOUNTER2_HI },
        { mmDB_PERFCOUNTER3_SELECT, mmDB_PERFCOUNTER3_SELECT1, mmDB_PERFCOUNTER3_LO, mmDB_PERFCOUNTER3_HI },
    }};

    PerfCounterBlockInfo*const pCb = &pInfo->block[static_cast<uint32>(GpuBlock::Cb)];
    pCb->distribution              = PerfCounterDistribution::PerShaderArray;
    pCb->numScopedInstances        = rbPerSa;
    pCb->numGenericSpmModules      = 1; // CB_PERFCOUNTER0
    pCb->numGenericLegacyModules   = 3; // CB_PERFCOUNTER1-3
    pCb->numSpmWires               = 2;
    pCb->spmBlockSelect            = Gfx12SpmSeBlockSelectCb;
    pCb->maxEventId                = maxIds[CBPerfSelId];

    pCb->regAddr = { 0, {
        { mmCB_PERFCOUNTER0_SELECT, mmCB_PERFCOUNTER0_SELECT1, mmCB_PERFCOUNTER0_LO, mmCB_PERFCOUNTER0_HI },
        { mmCB_PERFCOUNTER1_SELECT, 0,                         mmCB_PERFCOUNTER1_LO, mmCB_PERFCOUNTER1_HI },
        { mmCB_PERFCOUNTER2_SELECT, 0,                         mmCB_PERFCOUNTER2_LO, mmCB_PERFCOUNTER2_HI },
        { mmCB_PERFCOUNTER3_SELECT, 0,                         mmCB_PERFCOUNTER3_LO, mmCB_PERFCOUNTER3_HI },
    }};

    PerfCounterBlockInfo*const pGrbm = &pInfo->block[static_cast<uint32>(GpuBlock::Grbm)];
    pGrbm->distribution              = PerfCounterDistribution::GlobalBlock;
    pGrbm->numScopedInstances        = 1;
    pGrbm->numGenericSpmModules      = 0;
    pGrbm->numGenericLegacyModules   = 2; // GRBM_PERFCOUNTER0-1
    pGrbm->numSpmWires               = 0;
    pGrbm->maxEventId                = maxIds[GrbmPerfSelId];

    pGrbm->regAddr = { 0, {
        { mmGRBM_PERFCOUNTER0_SELECT, 0, mmGRBM_PERFCOUNTER0_LO, mmGRBM_PERFCOUNTER0_HI },
        { mmGRBM_PERFCOUNTER1_SELECT, 0, mmGRBM_PERFCOUNTER1_LO, mmGRBM_PERFCOUNTER1_HI },
    }};

    // The GRBMH block is one per SE, which allows for a reduction in wire count from the GRBM block in CPWD to SE.
    // It contain two performance counters to measure the performance of various blocks.
    PerfCounterBlockInfo*const pGrbmSe = &pInfo->block[static_cast<uint32>(GpuBlock::GrbmSe)];
    pGrbmSe->distribution              = PerfCounterDistribution::PerShaderEngine;
    pGrbmSe->numScopedInstances        = 1;
    pGrbmSe->numGenericSpmModules      = 0;
    pGrbmSe->numGenericLegacyModules   = 2; // GRBMH_PERFCOUNTER0-1
    pGrbmSe->numSpmWires               = 0;
    pGrbmSe->maxEventId                = maxIds[GrbmhPerfSelId];

    pGrbmSe->regAddr = { 0, {
        { mmGRBMH_PERFCOUNTER0_SELECT,  0, mmGRBMH_PERFCOUNTER0_LO,  mmGRBMH_PERFCOUNTER0_HI },
        { mmGRBMH_PERFCOUNTER1_SELECT,  0, mmGRBMH_PERFCOUNTER1_LO,  mmGRBMH_PERFCOUNTER1_HI },
    }};

    // The RLC's SELECT registers are non-standard because they lack PERF_MODE fields. This should be fine though
    // because we only use PERFMON_COUNTER_MODE_ACCUM which is zero. If we ever try to use a different mode the RLC
    // needs to be handled as a special case.
    static_assert(PERFMON_COUNTER_MODE_ACCUM == 0, "RLC legacy counters need special handling.");

    PerfCounterBlockInfo*const pRlc = &pInfo->block[static_cast<uint32>(GpuBlock::Rlc)];
    pRlc->distribution              = PerfCounterDistribution::GlobalBlock;
    pRlc->numScopedInstances        = 1;
    pRlc->numGenericSpmModules      = 0;
    pRlc->numGenericLegacyModules   = 2; // RLC_PERFCOUNTER0-1
    pRlc->numSpmWires               = 0;
    pRlc->maxEventId                = 6; // SERDES command write;

    pRlc->regAddr = { 0, {
        { mmRLC_PERFCOUNTER0_SELECT, 0, mmRLC_PERFCOUNTER0_LO, mmRLC_PERFCOUNTER0_HI },
        { mmRLC_PERFCOUNTER1_SELECT, 0, mmRLC_PERFCOUNTER1_LO, mmRLC_PERFCOUNTER1_HI },
    }};

    PerfCounterBlockInfo*const pDma = &pInfo->block[static_cast<uint32>(GpuBlock::Dma)];
    pDma->distribution              = PerfCounterDistribution::GlobalBlock;
    pDma->numScopedInstances        = device.EngineProperties().perEngine[EngineTypeDma].numAvailable;
    pDma->numGenericSpmModules      = 2; // SDMA#_PERFCOUNTER0-1
    pDma->numGenericLegacyModules   = 0;
    pDma->numSpmWires               = 4;
    pDma->spmBlockSelect            = Gfx12SpmGlobalBlockSelectSdma;
    pDma->maxEventId                = maxIds[SdmaPerfmonSelId];
    pDma->numScopedInstances        = Min(pDma->numScopedInstances, MaxSdmaInstances);

    pInfo->sdmaRegAddr[0][0] = { mmSDMA0_PERFCOUNTER0_SELECT, mmSDMA0_PERFCOUNTER0_SELECT1,
                                 mmSDMA0_PERFCOUNTER0_LO,     mmSDMA0_PERFCOUNTER0_HI };
    pInfo->sdmaRegAddr[0][1] = { mmSDMA0_PERFCOUNTER1_SELECT, mmSDMA0_PERFCOUNTER1_SELECT1,
                                 mmSDMA0_PERFCOUNTER1_LO,     mmSDMA0_PERFCOUNTER1_HI };

    pInfo->sdmaRegAddr[1][0] = { mmSDMA1_PERFCOUNTER0_SELECT, mmSDMA1_PERFCOUNTER0_SELECT1,
                                 mmSDMA1_PERFCOUNTER0_LO,     mmSDMA1_PERFCOUNTER0_HI };
    pInfo->sdmaRegAddr[1][1] = { mmSDMA1_PERFCOUNTER1_SELECT, mmSDMA1_PERFCOUNTER1_SELECT1,
                                 mmSDMA1_PERFCOUNTER1_LO,     mmSDMA1_PERFCOUNTER1_HI };

    PerfCounterBlockInfo*const pCpg = &pInfo->block[static_cast<uint32>(GpuBlock::Cpg)];
    pCpg->distribution              = PerfCounterDistribution::GlobalBlock;
    pCpg->numScopedInstances        = 1;
    pCpg->numGenericSpmModules      = 1; // CPG_PERFCOUNTER0
    pCpg->numGenericLegacyModules   = 1; // CPG_PERFCOUNTER1
    pCpg->numSpmWires               = 2;
    pCpg->spmBlockSelect            = Gfx12SpmGlobalBlockSelectCpg;
    pCpg->maxEventId                = maxIds[CpgPerfcountSelId];

    pCpg->regAddr = { 0, {
        { mmCPG_PERFCOUNTER0_SELECT, mmCPG_PERFCOUNTER0_SELECT1, mmCPG_PERFCOUNTER0_LO, mmCPG_PERFCOUNTER0_HI },
        { mmCPG_PERFCOUNTER1_SELECT, 0,                          mmCPG_PERFCOUNTER1_LO, mmCPG_PERFCOUNTER1_HI },
    }};

    PerfCounterBlockInfo*const pCpc = &pInfo->block[static_cast<uint32>(GpuBlock::Cpc)];
    pCpc->distribution              = PerfCounterDistribution::GlobalBlock;
    pCpc->numScopedInstances        = 1;
    pCpc->numGenericSpmModules      = 1; // CPC_PERFCOUNTER0
    pCpc->numGenericLegacyModules   = 1; // CPC_PERFCOUNTER1
    pCpc->numSpmWires               = 2;
    pCpc->spmBlockSelect            = Gfx12SpmGlobalBlockSelectCpc;
    pCpc->maxEventId                = maxIds[CpcPerfcountSelId];

    pCpc->regAddr = { 0, {
        { mmCPC_PERFCOUNTER0_SELECT, mmCPC_PERFCOUNTER0_SELECT1, mmCPC_PERFCOUNTER0_LO, mmCPC_PERFCOUNTER0_HI },
        { mmCPC_PERFCOUNTER1_SELECT, 0,                          mmCPC_PERFCOUNTER1_LO, mmCPC_PERFCOUNTER1_HI },
    }};

    // Also called the UTCL2.
    PerfCounterBlockInfo*const pMcVmL2 = &pInfo->block[static_cast<uint32>(GpuBlock::McVmL2)];
    pMcVmL2->distribution              = PerfCounterDistribution::GlobalBlock;
    pMcVmL2->numScopedInstances        = 1;
    pMcVmL2->numGenericSpmModules      = 2; // GCVML2_PERFCOUNTER2_0-1
    pMcVmL2->numGenericLegacyModules   = 8; // GCMC_VM_L2_PERFCOUNTER0-7
    pMcVmL2->numSpmWires               = 4;
    pMcVmL2->spmBlockSelect            = Gfx12SpmGlobalBlockSelectVml2;
    pMcVmL2->maxEventId                = maxIds[Gcvml2PerfSelId];
    pMcVmL2->isCfgStyle                = true;

    pMcVmL2->regAddr = { mmGCMC_VM_L2_PERFCOUNTER_RSLT_CNTL, {
        { mmGCMC_VM_L2_PERFCOUNTER0_CFG,  0,                               mmGCMC_VM_L2_PERFCOUNTER_LO, mmGCMC_VM_L2_PERFCOUNTER_HI },
        { mmGCMC_VM_L2_PERFCOUNTER1_CFG,  0,                               mmGCMC_VM_L2_PERFCOUNTER_LO, mmGCMC_VM_L2_PERFCOUNTER_HI },
        { mmGCMC_VM_L2_PERFCOUNTER2_CFG,  0,                               mmGCMC_VM_L2_PERFCOUNTER_LO, mmGCMC_VM_L2_PERFCOUNTER_HI },
        { mmGCMC_VM_L2_PERFCOUNTER3_CFG,  0,                               mmGCMC_VM_L2_PERFCOUNTER_LO, mmGCMC_VM_L2_PERFCOUNTER_HI },
        { mmGCMC_VM_L2_PERFCOUNTER4_CFG,  0,                               mmGCMC_VM_L2_PERFCOUNTER_LO, mmGCMC_VM_L2_PERFCOUNTER_HI },
        { mmGCMC_VM_L2_PERFCOUNTER5_CFG,  0,                               mmGCMC_VM_L2_PERFCOUNTER_LO, mmGCMC_VM_L2_PERFCOUNTER_HI },
        { mmGCMC_VM_L2_PERFCOUNTER6_CFG,  0,                               mmGCMC_VM_L2_PERFCOUNTER_LO, mmGCMC_VM_L2_PERFCOUNTER_HI },
        { mmGCMC_VM_L2_PERFCOUNTER7_CFG,  0,                               mmGCMC_VM_L2_PERFCOUNTER_LO, mmGCMC_VM_L2_PERFCOUNTER_HI },
        { mmGCVML2_PERFCOUNTER2_0_SELECT, mmGCVML2_PERFCOUNTER2_0_SELECT1, mmGCVML2_PERFCOUNTER2_0_LO,  mmGCVML2_PERFCOUNTER2_0_HI  },
        { mmGCVML2_PERFCOUNTER2_1_SELECT, mmGCVML2_PERFCOUNTER2_1_SELECT1, mmGCVML2_PERFCOUNTER2_1_LO,  mmGCVML2_PERFCOUNTER2_1_HI  }
    }};

    PerfCounterBlockInfo*const pEaCpwd = &pInfo->block[static_cast<uint32>(GpuBlock::EaCpwd)];
    pEaCpwd->distribution              = PerfCounterDistribution::GlobalBlock;
    pEaCpwd->numScopedInstances              = 1;  // One instance for CH interface to SDP
    pEaCpwd->numGenericSpmModules      = 1;  // GC_EA_CPWD_PERFCOUNTER0
    pEaCpwd->numGenericLegacyModules   = 1;  // GC_EA_CPWD_PERFCOUNTER1
    pEaCpwd->numSpmWires               = 2;
    pEaCpwd->spmBlockSelect            = Gfx12SpmGlobalBlockSelectEa;
    pEaCpwd->maxEventId                = maxIds[GcEaCpwdPerfcountSelId];

    pEaCpwd->regAddr = { 0, {
        { mmGC_EA_CPWD_PERFCOUNTER0_SELECT, mmGC_EA_CPWD_PERFCOUNTER0_SELECT1, mmGC_EA_CPWD_PERFCOUNTER0_LO, mmGC_EA_CPWD_PERFCOUNTER0_HI  },
        { mmGC_EA_CPWD_PERFCOUNTER1_SELECT, 0,                                 mmGC_EA_CPWD_PERFCOUNTER1_LO, mmGC_EA_CPWD_PERFCOUNTER1_HI  },
    }};

    PerfCounterBlockInfo* const pEaSe = &pInfo->block[static_cast<uint32>(GpuBlock::EaSe)];
    pEaSe->distribution             = PerfCounterDistribution::GlobalBlock; // While servicing SE, GL2C and EA are accessed globally
    pEaSe->numScopedInstances       = pProps->gfx9.gfx10.numGl2c; // One instance for each GL2C
    pEaSe->numGenericSpmModules     = 1;  // GC_EA_SE_PERFCOUNTER0
    pEaSe->numGenericLegacyModules  = 1;  // GC_EA_SE_PERFCOUNTER1
    pEaSe->numSpmWires              = 2;
    pEaSe->spmBlockSelect           = Gfx12SpmSeBlockSelectEa;
    pEaSe->maxEventId               = maxIds[GcEaSePerfcountSelId];

    pEaSe->regAddr = { 0, {
        { mmGC_EA_SE_PERFCOUNTER0_SELECT, mmGC_EA_SE_PERFCOUNTER0_SELECT1, mmGC_EA_SE_PERFCOUNTER0_LO, mmGC_EA_SE_PERFCOUNTER0_HI  },
        { mmGC_EA_SE_PERFCOUNTER1_SELECT, 0,                               mmGC_EA_SE_PERFCOUNTER1_LO, mmGC_EA_SE_PERFCOUNTER1_HI  },
    } };

    PerfCounterBlockInfo*const pRpb = &pInfo->block[static_cast<uint32>(GpuBlock::Rpb)];
    pRpb->distribution              = PerfCounterDistribution::GlobalBlock;
    pRpb->numScopedInstances        = 1;
    pRpb->numGenericSpmModules      = 0;
    pRpb->numGenericLegacyModules   = 4; // RPB_PERFCOUNTER0-3
    pRpb->numSpmWires               = 0;
    pRpb->maxEventId                = 63;
    pRpb->isCfgStyle                = true;

    pRpb->regAddr = { mmRPB_PERFCOUNTER_RSLT_CNTL, {
            { mmRPB_PERFCOUNTER0_CFG, 0, mmRPB_PERFCOUNTER_LO, mmRPB_PERFCOUNTER_HI },
            { mmRPB_PERFCOUNTER1_CFG, 0, mmRPB_PERFCOUNTER_LO, mmRPB_PERFCOUNTER_HI },
            { mmRPB_PERFCOUNTER2_CFG, 0, mmRPB_PERFCOUNTER_LO, mmRPB_PERFCOUNTER_HI },
            { mmRPB_PERFCOUNTER3_CFG, 0, mmRPB_PERFCOUNTER_LO, mmRPB_PERFCOUNTER_HI },
        }};

    PerfCounterBlockInfo*const pGe = &pInfo->block[static_cast<uint32>(GpuBlock::Ge)];
    pGe->distribution              = PerfCounterDistribution::GlobalBlock;
    pGe->numScopedInstances        = 1;
    pGe->numGenericSpmModules      = 4; // GE1_PERFCOUNTER0-3
    pGe->numGenericLegacyModules   = 0;
    pGe->numSpmWires               = 8;
    pGe->spmBlockSelect            = Gfx12SpmGlobalBlockSelectGe1;
    pGe->maxEventId                = maxIds[Ge1PerfcountSelectId];

    pGe->regAddr = { 0, {
        { mmGE1_PERFCOUNTER0_SELECT, mmGE1_PERFCOUNTER0_SELECT1, mmGE1_PERFCOUNTER0_LO, mmGE1_PERFCOUNTER0_HI },
        { mmGE1_PERFCOUNTER1_SELECT, mmGE1_PERFCOUNTER1_SELECT1, mmGE1_PERFCOUNTER1_LO, mmGE1_PERFCOUNTER1_HI },
        { mmGE1_PERFCOUNTER2_SELECT, mmGE1_PERFCOUNTER2_SELECT1, mmGE1_PERFCOUNTER2_LO, mmGE1_PERFCOUNTER2_HI },
        { mmGE1_PERFCOUNTER3_SELECT, mmGE1_PERFCOUNTER3_SELECT1, mmGE1_PERFCOUNTER3_LO, mmGE1_PERFCOUNTER3_HI },
    }};

    PerfCounterBlockInfo*const pGeDist = &pInfo->block[static_cast<uint32>(GpuBlock::GeDist)];
    pGeDist->distribution              = PerfCounterDistribution::GlobalBlock;
    pGeDist->numScopedInstances        = 1;
    pGeDist->numGenericSpmModules      = 4; // GE2_DIST_PERFCOUNTER0-3
    pGeDist->numGenericLegacyModules   = 0;
    pGeDist->numSpmWires               = 8;
    pGeDist->spmBlockSelect            = Gfx12SpmGlobalBlockSelectGe2Dist;
    pGeDist->maxEventId                = maxIds[Ge2DistPerfcountSelectId];

    pGeDist->regAddr = { 0, {
        { mmGE2_DIST_PERFCOUNTER0_SELECT, mmGE2_DIST_PERFCOUNTER0_SELECT1, mmGE2_DIST_PERFCOUNTER0_LO, mmGE2_DIST_PERFCOUNTER0_HI },
        { mmGE2_DIST_PERFCOUNTER1_SELECT, mmGE2_DIST_PERFCOUNTER1_SELECT1, mmGE2_DIST_PERFCOUNTER1_LO, mmGE2_DIST_PERFCOUNTER1_HI },
        { mmGE2_DIST_PERFCOUNTER2_SELECT, mmGE2_DIST_PERFCOUNTER2_SELECT1, mmGE2_DIST_PERFCOUNTER2_LO, mmGE2_DIST_PERFCOUNTER2_HI },
        { mmGE2_DIST_PERFCOUNTER3_SELECT, mmGE2_DIST_PERFCOUNTER3_SELECT1, mmGE2_DIST_PERFCOUNTER3_LO, mmGE2_DIST_PERFCOUNTER3_HI },
    }};

    // Yes, it is correct that this block is programmed per-SE but has a global SPM select. It is technically not
    // a real per-SE block because it doesn't live in the SEs but the global hardware still creates one GE2_SE
    // instance for each SE. It listens to GRBM_GFX_INDEX like a per-SE block but the SPM wires correctly hook
    // into the global mux interface.
    PerfCounterBlockInfo*const pGeSe = &pInfo->block[static_cast<uint32>(GpuBlock::GeSe)];
    pGeSe->distribution              = PerfCounterDistribution::PerShaderEngine;
    pGeSe->numScopedInstances        = 1;
    pGeSe->numGenericSpmModules      = 4; // GE2_SE_PERFCOUNTER0-3
    pGeSe->numGenericLegacyModules   = 0;
    pGeSe->numSpmWires               = 8;
    pGeSe->spmBlockSelect            = Gfx12SpmGlobalBlockSelectGe2Se;
    pGeSe->maxEventId                = maxIds[Ge2SePerfcountSelectId];

    pGeSe->regAddr = { 0, {
        { mmGE2_SE_PERFCOUNTER0_SELECT, mmGE2_SE_PERFCOUNTER0_SELECT1, mmGE2_SE_PERFCOUNTER0_LO, mmGE2_SE_PERFCOUNTER0_HI },
        { mmGE2_SE_PERFCOUNTER1_SELECT, mmGE2_SE_PERFCOUNTER1_SELECT1, mmGE2_SE_PERFCOUNTER1_LO, mmGE2_SE_PERFCOUNTER1_HI },
        { mmGE2_SE_PERFCOUNTER2_SELECT, mmGE2_SE_PERFCOUNTER2_SELECT1, mmGE2_SE_PERFCOUNTER2_LO, mmGE2_SE_PERFCOUNTER2_HI },
        { mmGE2_SE_PERFCOUNTER3_SELECT, mmGE2_SE_PERFCOUNTER3_SELECT1, mmGE2_SE_PERFCOUNTER3_LO, mmGE2_SE_PERFCOUNTER3_HI },
    }};

    //
    // The GL1 arbiter for SA (RB,TCP,SQC). The GL1 complex is per-SA by definition.
    PerfCounterBlockInfo*const pGl1a = &pInfo->block[static_cast<uint32>(GpuBlock::Gl1a)];
    pGl1a->distribution              = PerfCounterDistribution::PerShaderArray;
    pGl1a->numScopedInstances        = 1;
    pGl1a->numGenericSpmModules      = 4; // GL1A_PERFCOUNTER0-3
    pGl1a->numGenericLegacyModules   = 0;
    pGl1a->numSpmWires               = 8;
    pGl1a->spmBlockSelect            = Gfx12SpmSeBlockSelectGl1a;
    pGl1a->maxEventId                = maxIds[Gl1aPerfSelId];

    pGl1a->regAddr = { 0, {
        { mmGL1A_PERFCOUNTER0_SELECT, mmGL1A_PERFCOUNTER0_SELECT1, mmGL1A_PERFCOUNTER0_LO, mmGL1A_PERFCOUNTER0_HI },
        { mmGL1A_PERFCOUNTER1_SELECT, mmGL1A_PERFCOUNTER1_SELECT1, mmGL1A_PERFCOUNTER1_LO, mmGL1A_PERFCOUNTER1_HI },
        { mmGL1A_PERFCOUNTER2_SELECT, mmGL1A_PERFCOUNTER2_SELECT1, mmGL1A_PERFCOUNTER2_LO, mmGL1A_PERFCOUNTER2_HI },
        { mmGL1A_PERFCOUNTER3_SELECT, mmGL1A_PERFCOUNTER3_SELECT1, mmGL1A_PERFCOUNTER3_LO, mmGL1A_PERFCOUNTER3_HI },
    }};

    // The GL1 cache for SA. The GL1 in each SA(x) talks to 4 GL1C quadrants as before.
    PerfCounterBlockInfo*const pGl1c = &pInfo->block[static_cast<uint32>(GpuBlock::Gl1c)];
    pGl1c->distribution              = PerfCounterDistribution::PerShaderArray;
    pGl1c->numScopedInstances        = 4;
    pGl1c->numGenericSpmModules      = 4; // GL1C_PERFCOUNTER0-3
    pGl1c->numGenericLegacyModules   = 0;
    pGl1c->numSpmWires               = 8;
    pGl1c->spmBlockSelect            = Gfx12SpmSeBlockSelectGl1c;
    pGl1c->maxEventId                = maxIds[Gl1cPerfSelId];

    pGl1c->regAddr = { 0, {
        { mmGL1C_PERFCOUNTER0_SELECT, mmGL1C_PERFCOUNTER0_SELECT1, mmGL1C_PERFCOUNTER0_LO, mmGL1C_PERFCOUNTER0_HI },
        { mmGL1C_PERFCOUNTER1_SELECT, mmGL1C_PERFCOUNTER1_SELECT1, mmGL1C_PERFCOUNTER1_LO, mmGL1C_PERFCOUNTER1_HI },
        { mmGL1C_PERFCOUNTER2_SELECT, mmGL1C_PERFCOUNTER2_SELECT1, mmGL1C_PERFCOUNTER2_LO, mmGL1C_PERFCOUNTER2_HI },
        { mmGL1C_PERFCOUNTER3_SELECT, mmGL1C_PERFCOUNTER3_SELECT1, mmGL1C_PERFCOUNTER3_LO, mmGL1C_PERFCOUNTER3_HI },
    }};

    // The GL1 arbiter for SAx (all other gfx memory clients, like PC read, PA read/write, SC read/write and TT write).
    // Each SED is composed of 2 SAs and 1 SAx. There is a GL1 for each SA, SAx.
    PerfCounterBlockInfo*const pGl1xa = &pInfo->block[static_cast<uint32>(GpuBlock::Gl1xa)];
    pGl1xa->distribution              = PerfCounterDistribution::PerShaderEngine;
    pGl1xa->numScopedInstances        = 1;
    pGl1xa->numGenericSpmModules      = 4; // GL1XA_PERFCOUNTER0-3
    pGl1xa->numGenericLegacyModules   = 0;
    pGl1xa->numSpmWires               = 8;
    pGl1xa->spmBlockSelect            = Gfx12SpmSeBlockSelectGl1xa;
    pGl1xa->maxEventId                = maxIds[Gl1xaPerfSelId];

    pGl1xa->regAddr = { 0, {
        { mmGL1XA_PERFCOUNTER0_SELECT, mmGL1XA_PERFCOUNTER0_SELECT1, mmGL1XA_PERFCOUNTER0_LO, mmGL1XA_PERFCOUNTER0_HI },
        { mmGL1XA_PERFCOUNTER1_SELECT, mmGL1XA_PERFCOUNTER1_SELECT1, mmGL1XA_PERFCOUNTER1_LO, mmGL1XA_PERFCOUNTER1_HI },
        { mmGL1XA_PERFCOUNTER2_SELECT, mmGL1XA_PERFCOUNTER2_SELECT1, mmGL1XA_PERFCOUNTER2_LO, mmGL1XA_PERFCOUNTER2_HI },
        { mmGL1XA_PERFCOUNTER3_SELECT, mmGL1XA_PERFCOUNTER3_SELECT1, mmGL1XA_PERFCOUNTER3_LO, mmGL1XA_PERFCOUNTER3_HI },
    }};

    // The GL1 cache for SAx.
    PerfCounterBlockInfo*const pGl1xc = &pInfo->block[static_cast<uint32>(GpuBlock::Gl1xc)];
    pGl1xc->distribution              = PerfCounterDistribution::PerShaderEngine;
    pGl1xc->numScopedInstances        = 4;
    pGl1xc->numGenericSpmModules      = 4; // GL1C_PERFCOUNTER0-3
    pGl1xc->numGenericLegacyModules   = 0;
    pGl1xc->numSpmWires               = 8;
    pGl1xc->spmBlockSelect            = Gfx12SpmSeBlockSelectGl1xc;
    pGl1xc->maxEventId                = maxIds[Gl1xcPerfSelId];

    pGl1xc->regAddr = { 0, {
        { mmGL1XC_PERFCOUNTER0_SELECT, mmGL1XC_PERFCOUNTER0_SELECT1, mmGL1XC_PERFCOUNTER0_LO, mmGL1XC_PERFCOUNTER0_HI },
        { mmGL1XC_PERFCOUNTER1_SELECT, mmGL1XC_PERFCOUNTER1_SELECT1, mmGL1XC_PERFCOUNTER1_LO, mmGL1XC_PERFCOUNTER1_HI },
        { mmGL1XC_PERFCOUNTER2_SELECT, mmGL1XC_PERFCOUNTER2_SELECT1, mmGL1XC_PERFCOUNTER2_LO, mmGL1XC_PERFCOUNTER2_HI },
        { mmGL1XC_PERFCOUNTER3_SELECT, mmGL1XC_PERFCOUNTER3_SELECT1, mmGL1XC_PERFCOUNTER3_LO, mmGL1XC_PERFCOUNTER3_HI },
    }};

    // The GL2A (gl2 arbiter) block is typically broken down into four quadrants - we treat them as four instances.
    PerfCounterBlockInfo*const pGl2a = &pInfo->block[static_cast<uint32>(GpuBlock::Gl2a)];
    pGl2a->distribution              = PerfCounterDistribution::GlobalBlock;
    pGl2a->numScopedInstances        = pProps->gfx9.gfx10.numGl2a;
    pGl2a->numGenericSpmModules      = 4; // Gl2A_PERFCOUNTER0-3
    pGl2a->numGenericLegacyModules   = 0;
    pGl2a->numSpmWires               = 8;
    pGl2a->spmBlockSelect            = Gfx12SpmGlobalBlockSelectGl2a;
    pGl2a->maxEventId                = maxIds[Gl2aPerfSelId];

    pGl2a->regAddr = { 0, {
        { mmGL2A_PERFCOUNTER0_SELECT, mmGL2A_PERFCOUNTER0_SELECT1, mmGL2A_PERFCOUNTER0_LO, mmGL2A_PERFCOUNTER0_HI },
        { mmGL2A_PERFCOUNTER1_SELECT, mmGL2A_PERFCOUNTER1_SELECT1, mmGL2A_PERFCOUNTER1_LO, mmGL2A_PERFCOUNTER1_HI },
        { mmGL2A_PERFCOUNTER2_SELECT, mmGL2A_PERFCOUNTER2_SELECT1, mmGL2A_PERFCOUNTER2_LO, mmGL2A_PERFCOUNTER2_HI },
        { mmGL2A_PERFCOUNTER3_SELECT, mmGL2A_PERFCOUNTER3_SELECT1, mmGL2A_PERFCOUNTER3_LO, mmGL2A_PERFCOUNTER3_HI },
    }};

    PerfCounterBlockInfo*const pGl2c = &pInfo->block[static_cast<uint32>(GpuBlock::Gl2c)];
    pGl2c->distribution              = PerfCounterDistribution::GlobalBlock;
    pGl2c->numScopedInstances        = pProps->gfx9.gfx10.numGl2c; // This should be equal to the number of EAs.
    pGl2c->numGenericSpmModules      = 4; // Gl2C_PERFCOUNTER0-3
    pGl2c->numGenericLegacyModules   = 0;
    pGl2c->numSpmWires               = 8;
    pGl2c->spmBlockSelect            = Gfx12SpmGlobalBlockSelectGl2c;
    pGl2c->maxEventId                = maxIds[Gl2cPerfSelId];

    pGl2c->regAddr = { 0, {
        { mmGL2C_PERFCOUNTER0_SELECT, mmGL2C_PERFCOUNTER0_SELECT1, mmGL2C_PERFCOUNTER0_LO, mmGL2C_PERFCOUNTER0_HI },
        { mmGL2C_PERFCOUNTER1_SELECT, mmGL2C_PERFCOUNTER1_SELECT1, mmGL2C_PERFCOUNTER1_LO, mmGL2C_PERFCOUNTER1_HI },
        { mmGL2C_PERFCOUNTER2_SELECT, mmGL2C_PERFCOUNTER2_SELECT1, mmGL2C_PERFCOUNTER2_LO, mmGL2C_PERFCOUNTER2_HI },
        { mmGL2C_PERFCOUNTER3_SELECT, mmGL2C_PERFCOUNTER3_SELECT1, mmGL2C_PERFCOUNTER3_LO, mmGL2C_PERFCOUNTER3_HI },
    }};

    // The center hub arbiter (CHA). It's the global version of the GL1A and is used by global blocks.
    PerfCounterBlockInfo*const pCha = &pInfo->block[static_cast<uint32>(GpuBlock::Cha)];
    pCha->distribution              = PerfCounterDistribution::GlobalBlock;
    pCha->numScopedInstances        = 1;
    pCha->numGenericSpmModules      = 4; // CHA_PERFCOUNTER0-3
    pCha->numGenericLegacyModules   = 0;
    pCha->numSpmWires               = 8;
    pCha->spmBlockSelect            = Gfx12SpmGlobalBlockSelectCha;
    pCha->maxEventId                = maxIds[ChaPerfSelId];

    pCha->regAddr = { 0, {
        { mmCHA_PERFCOUNTER0_SELECT, mmCHA_PERFCOUNTER0_SELECT1, mmCHA_PERFCOUNTER0_LO, mmCHA_PERFCOUNTER0_HI },
        { mmCHA_PERFCOUNTER1_SELECT, mmCHA_PERFCOUNTER1_SELECT1, mmCHA_PERFCOUNTER1_LO, mmCHA_PERFCOUNTER1_HI },
        { mmCHA_PERFCOUNTER2_SELECT, mmCHA_PERFCOUNTER2_SELECT1, mmCHA_PERFCOUNTER2_LO, mmCHA_PERFCOUNTER2_HI },
        { mmCHA_PERFCOUNTER3_SELECT, mmCHA_PERFCOUNTER3_SELECT1, mmCHA_PERFCOUNTER3_LO, mmCHA_PERFCOUNTER3_HI },
    }};

    // The center hub buffer (CHC). It's the global version of the GL1C and is used by global blocks.
    PerfCounterBlockInfo*const pChc = &pInfo->block[static_cast<uint32>(GpuBlock::Chc)];
    pChc->distribution              = PerfCounterDistribution::GlobalBlock;
    pChc->numScopedInstances        = 4; // It also has four quadrants like the GL1C.
    pChc->numGenericSpmModules      = 4; // CHC_PERFCOUNTER0-3
    pChc->numGenericLegacyModules   = 0;
    pChc->numSpmWires               = 8;
    pChc->spmBlockSelect            = Gfx12SpmGlobalBlockSelectChc;
    pChc->maxEventId                = maxIds[ChcPerfSelId];

    pChc->regAddr = { 0, {
        { mmCHC_PERFCOUNTER0_SELECT, mmCHC_PERFCOUNTER0_SELECT1, mmCHC_PERFCOUNTER0_LO, mmCHC_PERFCOUNTER0_HI },
        { mmCHC_PERFCOUNTER1_SELECT, mmCHC_PERFCOUNTER1_SELECT1, mmCHC_PERFCOUNTER1_LO, mmCHC_PERFCOUNTER1_HI },
        { mmCHC_PERFCOUNTER2_SELECT, mmCHC_PERFCOUNTER2_SELECT1, mmCHC_PERFCOUNTER2_LO, mmCHC_PERFCOUNTER2_HI },
        { mmCHC_PERFCOUNTER3_SELECT, mmCHC_PERFCOUNTER3_SELECT1, mmCHC_PERFCOUNTER3_LO, mmCHC_PERFCOUNTER3_HI },
    }};

    // The global block that implements the graphics cache rinse feature.
    PerfCounterBlockInfo*const pGcr = &pInfo->block[static_cast<uint32>(GpuBlock::Gcr)];
    pGcr->distribution              = PerfCounterDistribution::GlobalBlock;
    pGcr->numScopedInstances        = 1;
    pGcr->numGenericSpmModules      = 2; // GCR_PERFCOUNTER0-1
    pGcr->numGenericLegacyModules   = 0;
    pGcr->numSpmWires               = 4;
    pGcr->spmBlockSelect            = Gfx12SpmGlobalBlockSelectGcr;
    pGcr->maxEventId                = maxIds[GCRPerfSelId];

    pGcr->regAddr = { 0, {
        { mmGCR_PERFCOUNTER0_SELECT, mmGCR_PERFCOUNTER0_SELECT1, mmGCR_PERFCOUNTER0_LO, mmGCR_PERFCOUNTER0_HI },
        { mmGCR_PERFCOUNTER1_SELECT, mmGCR_PERFCOUNTER1_SELECT1, mmGCR_PERFCOUNTER1_LO, mmGCR_PERFCOUNTER1_HI },
    }};

    // There are now two UTCL1s per SE. They receive requests from GL1C/SA01 and GL1XC/SAx respectively.
    // TCP, SQC, CB, and DB requests go to even instances and PA, PC, SC, TT, etc., requests go to odd instances.
    PerfCounterBlockInfo*const pUtcl1 = &pInfo->block[static_cast<uint32>(GpuBlock::UtcL1)];
    pUtcl1->distribution              = PerfCounterDistribution::PerShaderEngine;
    pUtcl1->numScopedInstances        = 2;
    pUtcl1->numGenericLegacyModules   = 4; // UTCL1_PERFCOUNTER0-3
    pUtcl1->numSpmWires               = 0;
    pUtcl1->numGenericSpmModules      = 0;
    pUtcl1->maxEventId                = maxIds[UTCL1PerfSelId];

    pUtcl1->regAddr = { 0, {
        { mmUTCL1_PERFCOUNTER0_SELECT, 0, mmUTCL1_PERFCOUNTER0_LO, mmUTCL1_PERFCOUNTER0_HI },
        { mmUTCL1_PERFCOUNTER1_SELECT, 0, mmUTCL1_PERFCOUNTER1_LO, mmUTCL1_PERFCOUNTER1_HI },
        { mmUTCL1_PERFCOUNTER2_SELECT, 0, mmUTCL1_PERFCOUNTER2_LO, mmUTCL1_PERFCOUNTER2_HI },
        { mmUTCL1_PERFCOUNTER3_SELECT, 0, mmUTCL1_PERFCOUNTER3_LO, mmUTCL1_PERFCOUNTER3_HI },
    }};

    // The Parameter Cache (PC) block, one per SE.
    PerfCounterBlockInfo*const pPc = &pInfo->block[static_cast<uint32>(GpuBlock::Pc)];
    pPc->distribution              = PerfCounterDistribution::PerShaderEngine;
    pPc->numScopedInstances        = 1;
    pPc->numGenericSpmModules      = 4; // PC_PERFCOUNTER0-3
    pPc->numGenericLegacyModules   = 0;
    pPc->numSpmWires               = 8;
    pPc->spmBlockSelect            = Gfx12SpmSeBlockSelectPc;
    pPc->maxEventId                = maxIds[PcPerfcntSelId];

    pPc->regAddr = { 0, {
        { mmPC_PERFCOUNTER0_SELECT, mmPC_PERFCOUNTER0_SELECT1, mmPC_PERFCOUNTER0_LO, mmPC_PERFCOUNTER0_HI },
        { mmPC_PERFCOUNTER1_SELECT, mmPC_PERFCOUNTER1_SELECT1, mmPC_PERFCOUNTER1_LO, mmPC_PERFCOUNTER1_HI },
        { mmPC_PERFCOUNTER2_SELECT, mmPC_PERFCOUNTER2_SELECT1, mmPC_PERFCOUNTER2_LO, mmPC_PERFCOUNTER2_HI },
        { mmPC_PERFCOUNTER3_SELECT, mmPC_PERFCOUNTER3_SELECT1, mmPC_PERFCOUNTER3_LO, mmPC_PERFCOUNTER3_HI },
    }};

    // The Work-Graph Scheduler (WGS). One Asynchronous Compute Engine (ACE) is instanced in each SED as this new block.
    PerfCounterBlockInfo*const pWgs = &pInfo->block[static_cast<uint32>(GpuBlock::Wgs)];
    pWgs->distribution              = PerfCounterDistribution::PerShaderEngine;
    pWgs->numScopedInstances        = 1;
    pWgs->numGenericSpmModules      = 1; // WGS_PERFCOUNTER0
    pWgs->numGenericLegacyModules   = 1; // WGS_PERFCOUNTER1
    pWgs->numSpmWires               = 2;
    pWgs->spmBlockSelect            = Gfx12SpmSeBlockSelectWgs;
    pWgs->maxEventId                = maxIds[WgsPerfcountSelId];

    pWgs->regAddr = { 0, {
        { mmWGS_PERFCOUNTER0_SELECT, mmWGS_PERFCOUNTER0_SELECT1, mmWGS_PERFCOUNTER0_LO, mmWGS_PERFCOUNTER0_HI },
        { mmWGS_PERFCOUNTER1_SELECT, 0,                          mmWGS_PERFCOUNTER1_LO, mmWGS_PERFCOUNTER1_HI },
    }};

    // RLC User data supplied "counters" that are updated by CP
    PerfCounterBlockInfo* const pRlcUser = &pInfo->block[static_cast<uint32>(GpuBlock::RlcUser)];
    pRlcUser->distribution        = PerfCounterDistribution::GlobalBlock;
    pRlcUser->numScopedInstances  = MaxRlcUserData;
    pRlcUser->num16BitSpmCounters = 0;
    pRlcUser->num32BitSpmCounters = 1; // This "counter" is forced 32bit
    pRlcUser->numSpmWires         = 4; // This is typically per instance, but for this block represents all "counters"
    pRlcUser->spmBlockSelect      = Gfx12SpmGlobalBlockSelectRspm;
    pRlcUser->maxEventId          = 0;

    // Because this block defines no generic legacy counters, the regAddr struct is not used for select or sample
    // programming and remains unpopulated
    pRlcUser->regAddr = { 0, {} };
}

// =====================================================================================================================
// Initializes the performance counter information for an adapter structure, specifically for the Gfx12 hardware layer.
void InitPerfCtrInfo(
    const Pal::Device& device,
    GpuChipProperties* pProps)
{
    // Something pretty terrible will probably happen if this isn't true.
    PAL_ASSERT(pProps->gfx9.numShaderEngines <= MaxShaderEngines);

    PerfCounterInfo*const pInfo = &pProps->gfx9.perfCounterInfo.gfx12Info;

    // The caller should already have zeroed this struct a long time ago but let's do it again just to be sure.
    // We depend very heavily on unsupported fields being zero by default.
    memset(pInfo, 0, sizeof(*pInfo));

    // Some fields require non-zero defaults.
    for (uint32 idx = 0; idx < static_cast<uint32>(GpuBlock::Count); idx++)
    {
        PerfCounterBlockInfo*const pBlock = &pInfo->block[idx];

        // The SPM block select requires a non-zero default. We use UINT32_MAX to indicate "invalid".
        pBlock->spmBlockSelect = UINT32_MAX;

        // Almost all blocks have per-instance counter hardware.
        pBlock->instanceGroupSize = 1;
    }

    // These features are supported by all ASICs.
    pInfo->features.counters         = 1;
    pInfo->features.threadTrace      = 1;
    pInfo->features.spmTrace         = 1;
    pInfo->features.supportPs1Events = 1;
    pInfo->features.dfSpmTrace       = 1;

    // Set the hardware specified per-block information (see the function for what exactly that means).
    // There's so much code to do this that it had to go in a helper function for each version.
    if (IsGfx12(pProps->gfxLevel))
    {
        Gfx12InitBasicBlockInfo(device, pProps);
    }

    // Using that information, infer the remaining per-block properties.
    for (uint32 idx = 0; idx < static_cast<uint32>(GpuBlock::Count); idx++)
    {
        PerfCounterBlockInfo*const pBlock = &pInfo->block[idx];

        if (pBlock->distribution != PerfCounterDistribution::Unavailable)
        {
            // Compute the total instance count.
            if (pBlock->distribution == PerfCounterDistribution::PerShaderArray)
            {
                pBlock->numInstances =
                    pBlock->numScopedInstances * pProps->gfx9.numActiveShaderEngines * pProps->gfx9.numShaderArrays;
            }
            else if (pBlock->distribution == PerfCounterDistribution::PerShaderEngine)
            {
                pBlock->numInstances = pBlock->numScopedInstances * pProps->gfx9.numActiveShaderEngines;
            }
            else
            {
                pBlock->numInstances = pBlock->numScopedInstances;
            }

            // If this triggers we need to increase MaxPerfModules.
            const uint32 totalGenericModules = pBlock->numGenericSpmModules + pBlock->numGenericLegacyModules;
            PAL_ASSERT(totalGenericModules <= MaxPerfModules);

            if (totalGenericModules > 0)
            {
                PAL_ASSERT((pBlock->numGlobalOnlyCounters == 0) && (pBlock->numGlobalSharedCounters == 0));

                // Unless explicitly set, each 64bit counter has 4 16bit counters and 2 32bit counters
                if ((pBlock->num16BitSpmCounters == 0) && (pBlock->num32BitSpmCounters == 0))
                {
                    pBlock->num16BitSpmCounters = pBlock->numGenericSpmModules * 4;
                    pBlock->num32BitSpmCounters = pBlock->numGenericSpmModules * 2;
                }
                pBlock->numGlobalOnlyCounters   = pBlock->numGenericLegacyModules;
                pBlock->numGlobalSharedCounters = pBlock->numGenericSpmModules;
            }

            // If some block has SPM counters it must have SPM wires and a SPM block select.
            PAL_ASSERT(((pBlock->num16BitSpmCounters == 0) && (pBlock->num32BitSpmCounters == 0)) ||
                       ((pBlock->numSpmWires > 0) && (pBlock->spmBlockSelect != UINT32_MAX)));
        }
    }

    // Verify that we didn't exceed any of our hard coded per-block constants.
    PAL_ASSERT(pInfo->block[static_cast<uint32>(GpuBlock::Dma)].numInstances         <= MaxSdmaInstances);
    PAL_ASSERT(pInfo->block[static_cast<uint32>(GpuBlock::Dma)].numGenericSpmModules <= MaxSdmaPerfModules);
    PAL_ASSERT(pInfo->block[static_cast<uint32>(GpuBlock::Umcch)].numInstances       <= MaxUmcchInstances);
}

} // Gfx12
} // Pal
