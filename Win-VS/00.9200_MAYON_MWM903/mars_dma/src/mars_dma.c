#include <general.h>

#include "intapi.h"
#include <mars_int.h>
#include <mars_dma.h>
#include <stdio.h>
#include "board.h"
#include <IISapi.h>

#if(FPGA_BOARD_A1018_SERIES)
  #define DMA_TIMEOUT 60
#else
  #define DMA_TIMEOUT 40
#endif
void marsDMAIntHandler(void);

INT8U gDMALockStatus = 0;
OS_EVENT *gDMASem;
OS_EVENT *gDMAOpenSem;
OS_FLAG_GRP *gDMAFlagGrp;

FP_INT gpDMAIsrTbl[DMA_ID_NUM];
INT32U gCurDMAIntStat[DMA_ID_NUM];

__align(64) u32 gDMAHWMemsetVal[DMA_ID_NUM][16];
DMA_MemsetCacheBuffer gDMAMemsetValTbl[DMA_ID_NUM];

const INT32U gFlagDMAInt[DMA_ID_NUM]      = {FLAGDMA_INT_CH0, FLAGDMA_INT_CH1, FLAGDMA_INT_CH2, FLAGDMA_INT_CH3, FLAGDMA_INT_CH4, FLAGDMA_INT_CH5, FLAGDMA_INT_CH6, FLAGDMA_INT_CH7};
const INT32U gFlagDMAInt_AUTO[DMA_ID_NUM] = {FLAGDMA_AUTO_INT_CH0, FLAGDMA_AUTO_INT_CH1, FLAGDMA_AUTO_INT_CH2, FLAGDMA_AUTO_INT_CH3, FLAGDMA_AUTO_INT_CH4, FLAGDMA_AUTO_INT_CH5, FLAGDMA_AUTO_INT_CH6, FLAGDMA_AUTO_INT_CH7};

const INT32U gDMARegSrcAddr[DMA_ID_NUM] = {REG_DMACH0SRCADDR, REG_DMACH1SRCADDR, REG_DMACH2SRCADDR, REG_DMACH3SRCADDR, REG_DMACH4SRCADDR, REG_DMACH5SRCADDR, REG_DMACH6SRCADDR, REG_DMACH7SRCADDR};
const INT32U gDMARegDstAddr[DMA_ID_NUM] = {REG_DMACH0DSTADDR, REG_DMACH1DSTADDR, REG_DMACH2DSTADDR, REG_DMACH3DSTADDR, REG_DMACH4DSTADDR, REG_DMACH5DSTADDR, REG_DMACH6DSTADDR, REG_DMACH7DSTADDR};
const INT32U gDMARegCycCnt[DMA_ID_NUM]  = {REG_DMACH0CYCCNT, REG_DMACH1CYCCNT, REG_DMACH2CYCCNT, REG_DMACH3CYCCNT, REG_DMACH4CYCCNT, REG_DMACH5CYCCNT, REG_DMACH6CYCCNT, REG_DMACH7CYCCNT};
const INT32U gDMARegCmmd[DMA_ID_NUM]    = {REG_DMACH0CMD, REG_DMACH1CMD, REG_DMACH2CMD, REG_DMACH3CMD, REG_DMACH4CMD, REG_DMACH5CMD, REG_DMACH6CMD, REG_DMACH7CMD};

const INT32U gDMARegSrcAddrAlt[DMA_ID_NUM]={REG_DMACH0SRCADDRAlt,REG_DMACH1SRCADDRAlt,REG_DMACH2SRCADDRAlt,REG_DMACH3SRCADDRAlt,REG_DMACH4SRCADDRAlt,REG_DMACH5SRCADDRAlt,REG_DMACH6SRCADDRAlt,REG_DMACH7SRCADDRAlt};
const INT32U gDMARegDstAddrAlt[DMA_ID_NUM]={REG_DMACH0DSTADDRAlt,REG_DMACH1DSTADDRAlt,REG_DMACH2DSTADDRAlt,REG_DMACH3DSTADDRAlt,REG_DMACH4DSTADDRAlt,REG_DMACH5DSTADDRAlt,REG_DMACH6DSTADDRAlt,REG_DMACH7DSTADDRAlt};
const INT32U gDMARegAutoNum[DMA_ID_NUM]   ={REG_DMACH0AutoNum,REG_DMACH1AutoNum,REG_DMACH2AutoNum,REG_DMACH3AutoNum,REG_DMACH4AutoNum,REG_DMACH5AutoNum,REG_DMACH6AutoNum,REG_DMACH7AutoNum};


#ifdef  TCM_DMA_SUPPORT
#define TCM_DMA_TIMEOUT 20

OS_EVENT     *gTCM_DMASemReq;
OS_EVENT     *gTCM_DMASemFinish;
#endif

//---------Extern Variable----------//
extern  DMA_CFG_AUTO MarsdmaCfg_auto[DMA_REQ_MAX];

//----------------------------------//
void marsDMAInit(void)
{
    INT32U i;
    INT8U err;
    
    gDMASem     = OSSemCreate(DMA_ID_NUM);
    gDMAOpenSem = OSSemCreate(1);
    gDMAFlagGrp = OSFlagCreate(0x00000000, &err);
    
    //marsIntIRQEnable(INT_IRQMASK_DMA);
    marsIntIRQDefIsr(INT_IRQID_DMA, marsDMAIntHandler);
    
    for(i = 0; i < DMA_ID_NUM; i++)
    {
    	gpDMAIsrTbl[i] = NULL;
    	gDMAMemsetValTbl[i].DMAID = DMA_ID_UNMARK;
    	gDMAMemsetValTbl[i].CB = gDMAHWMemsetVal[i];
    }

#ifdef  TCM_DMA_SUPPORT
    TCM_DMAInit();
#endif

}

void marsDMAOpen(INT32U *pDMAId, FP_INT pEventHdl)
{
    INT8U err;
    INT32U i, uiDMALockBit;
    
    // get one semaphore
    OSSemPend(gDMASem, OS_IPC_WAIT_FOREVER, &err);
    
    if(gDMALockStatus == 0)
        marsIntIRQEnable(INT_IRQMASK_DMA);
    
    // get open semaphore
    OSSemPend(gDMAOpenSem, OS_IPC_WAIT_FOREVER, &err);    
    for(i=0; i<DMA_ID_NUM; i++)
    {
        uiDMALockBit = (0x01 << i);
        if((gDMALockStatus & uiDMALockBit) == 0)    // this DMA is not locked
        {
            gDMALockStatus |= uiDMALockBit;
            gpDMAIsrTbl[i] = pEventHdl;
            *pDMAId = i;            
            // release open semaphore
            OSSemPost(gDMAOpenSem);
            return;
        }
    }
    OSSemPost(gDMAOpenSem);
    DEBUG_DMA("marsDMAOpen ERROR !!");
    
}

void marsDMAClose(INT32U uiDMAId)
{
    INT32U uiDMALockBit = (0x01 << uiDMAId);
    INT8U err;
    
    if(uiDMAId >= DMA_ID_NUM)
        return;
    if((gDMALockStatus & uiDMALockBit) == 0)    // this DMA is not locked
        return;
    
    // get open semaphore
    OSSemPend(gDMAOpenSem, OS_IPC_WAIT_FOREVER, &err); 
    
    gDMALockStatus &= (~uiDMALockBit);
    gpDMAIsrTbl[uiDMAId] = NULL;
    
    // release semaphore
    OSSemPost(gDMASem);
    
    // release open semaphore
    OSSemPost(gDMAOpenSem);
            
    // for optimize
    //if(gDMALockStatus == 0)
    //    marsIntIRQDisable(INT_IRQMASK_DMA);
}

void marsDMACloseReleaseSource(INT32U uiDMAId)
{
    INT32U uiDMALockBit = (0x01 << uiDMAId);
    INT8U err;
    
    if(uiDMAId >= DMA_ID_NUM)
        return;
    if((gDMALockStatus & uiDMALockBit) == 0)    // this DMA is not locked
        return;
    
    // get open semaphore
    OSSemPend(gDMAOpenSem, OS_IPC_WAIT_FOREVER, &err); 
    
    gDMALockStatus &= (~uiDMALockBit);
    gpDMAIsrTbl[uiDMAId] = NULL;

    *(volatile INT32U *)gDMARegCmmd[uiDMAId] &= (~DMA_START);
    
    // release semaphore
    OSSemPost(gDMASem);
    
    // release open semaphore
    OSSemPost(gDMAOpenSem);
            
    // for optimize
    //if(gDMALockStatus == 0)
    //    marsIntIRQDisable(INT_IRQMASK_DMA);
}

void marsDMAIntHandler(void)
{
    INT8U err;
    INT32U i, CurDMAIntStat, uiDMAChkBit=0x03;
    volatile INT32U *pDMAIntSta;
    INT32U AutoDMAIntStat;
    
    pDMAIntSta = (volatile INT32U *)REG_DMAINTSTAT;
    CurDMAIntStat = *pDMAIntSta;
    //DEBUG_DMA("0x%x ",CurDMAIntStat);
    for(i=0; i<DMA_ID_NUM; i++)
    {
        if(CurDMAIntStat & uiDMAChkBit)
        {
            gCurDMAIntStat[i] = (CurDMAIntStat >> (2*i)) & 0x03;
            OSFlagPost(gDMAFlagGrp, gFlagDMAInt[i], OS_FLAG_SET, &err);
            if(gpDMAIsrTbl[i] != NULL)
                gpDMAIsrTbl[i](i);
        }
        uiDMAChkBit <<= 2;
    }
    
    
    //Auto-DMA-finish check 
    if(CurDMAIntStat & 0x0ff0000)
    {
        AutoDMAIntStat=CurDMAIntStat >> 16;
        for(i=0; i<DMA_ID_NUM; i++)
        {
            if(AutoDMAIntStat & 0x01)
            {
                OSFlagPost(gDMAFlagGrp, gFlagDMAInt_AUTO[i], OS_FLAG_SET, &err);
                //DEBUG_DMA("o");
            }
            AutoDMAIntStat >>=1;
        }
    }

#ifdef  TCM_DMA_SUPPORT
    TCM_DMAIntHandler();
#endif

}

INT32U marsDMAWaitForInt(INT32U uiDMAId)
{
    INT8U err;
    volatile INT32U *pDMARegCmmd;
    volatile INT32U *pDMARegCycle;
    
    if(uiDMAId >= DMA_ID_NUM)
        return 0;
    OSFlagPend(gDMAFlagGrp, gFlagDMAInt[uiDMAId], OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, DMA_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
    	DEBUG_DMA("Wait DMA=%d Timeout :cycle=0x%X \n", uiDMAId, *(volatile INT32U *)gDMARegCycCnt[uiDMAId]);
        DEBUG_DMA("0x%x,0x%x,0x%x,0x%x\n",*(volatile INT32U *)gDMARegSrcAddr[uiDMAId],*(volatile INT32U *)gDMARegDstAddr[uiDMAId],*(volatile INT32U *)gDMARegCycCnt[uiDMAId],*(volatile INT32U *)gDMARegCmmd[uiDMAId]);
        pDMARegCmmd = (volatile INT32U *)gDMARegCmmd[uiDMAId];
        pDMARegCycle= (volatile INT32U *)gDMARegCycCnt[uiDMAId];
        
        //*pDMARegCmmd &= (~DMA_START);
        *pDMARegCmmd=0;
        *pDMARegCycle =0;
        
        return 0;    // error
    }
    if(gCurDMAIntStat[uiDMAId] == DMA_INTSTAT_FINISH)
        return 1;    // ok
    else
        return 0;    // error
}   

int marsDMATerminate(INT32U uiDMAId)
{
    volatile INT32U *pDMARegCmmd;
    volatile INT32U *pDMARegCycle;

    if(uiDMAId >= DMA_ID_NUM)
        return 0;
    
    pDMARegCmmd = (volatile INT32U *)gDMARegCmmd[uiDMAId];
    pDMARegCycle= (volatile INT32U *)gDMARegCycCnt[uiDMAId];
        
    *pDMARegCmmd=0;
    *pDMARegCycle =0;        
}


INT32U marsDMAWaitForIntOrCancel(INT32U uiDMAId, INT16U Timeout, INT32U* CycleCnt)
{
    INT8U err;
    volatile INT32U *pDMARegCmmd;
    volatile INT32U *pDMARegCycle;
    
    if(uiDMAId >= DMA_ID_NUM)
        return 0;
    //OSFlagPend(gDMAFlagGrp, gFlagDMAInt[uiDMAId], OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    OSFlagPend(gDMAFlagGrp, gFlagDMAInt[uiDMAId]|DMA_CANCEL, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, Timeout, &err);
    if (err != OS_NO_ERR)
    {
    	DEBUG_DMA("Wait DMA=%d Timeout \r\n", uiDMAId);
    	pDMARegCmmd = (volatile INT32U *)gDMARegCmmd[uiDMAId];
        pDMARegCycle= (volatile INT32U *)gDMARegCycCnt[uiDMAId];
        
        //*pDMARegCmmd &= (~DMA_START);
        *pDMARegCmmd=0;
        *pDMARegCycle =0;
        
        return 0;    // timeout
    }
    if(gCurDMAIntStat[uiDMAId] == DMA_INTSTAT_FINISH)
    {
        *CycleCnt = *(volatile INT32U *)gDMARegCycCnt[uiDMAId];
        return 1;    // ok
    }
    else
    {
        *CycleCnt = *(volatile INT32U *)gDMARegCycCnt[uiDMAId];
        *(volatile INT32U *)gDMARegCmmd[uiDMAId] &= (~DMA_START);
        return 2;    // Cancel
    }
}   

INT32U marsDMACancel(void)
{
    INT8U err;
    
    OSFlagPost(gDMAFlagGrp, DMA_CANCEL, OS_FLAG_SET, &err);
    return 1;
}   

INT32U marsDMAClearCancelFlag(void)
{
    INT8U err;
    
    OSFlagPost(gDMAFlagGrp, DMA_CANCEL, OS_FLAG_CLR, &err);
    return 1;
}   

INT32S marsDMACheckCycleCnt(INT32U uiDMAId)
{
    volatile INT32U *pDMACycCnt;
    
    if(uiDMAId >= DMA_ID_NUM)
        return -1;
        
    pDMACycCnt = (volatile INT32U *)gDMARegCycCnt[uiDMAId];
    return (*pDMACycCnt);
}

INT32U marsDMAGetIntSta(INT32U uiDMAId)
{
    if(uiDMAId >= DMA_ID_NUM)
        return 0;
    return (gCurDMAIntStat[uiDMAId]);
}

INT32U marsDMAConfig(INT32U uiDMAId, REGDMA_CFG* pCfg)
{
    INT8U err;
    volatile INT32U *pDMARegSrcAddr, *pDMARegDstAddr, *pDMARegCycCnt, *pDMARegCmmd;
    
    if(uiDMAId >= DMA_ID_NUM)
        return 0;
    
    pDMARegSrcAddr = (volatile INT32U *)gDMARegSrcAddr[uiDMAId];
    pDMARegDstAddr = (volatile INT32U *)gDMARegDstAddr[uiDMAId];
    pDMARegCycCnt  = (volatile INT32U *)gDMARegCycCnt[uiDMAId];
    pDMARegCmmd    = (volatile INT32U *)gDMARegCmmd[uiDMAId];
    
    *pDMARegCmmd &= (~DMA_START);
    
    *pDMARegSrcAddr = pCfg->src;
    *pDMARegDstAddr = pCfg->dst;
    *pDMARegCycCnt  = pCfg->cnt;
    *pDMARegCmmd    = pCfg->cmmd;

    gCurDMAIntStat[uiDMAId] = 0;
    // clear DMA INT-flag occurs before
    OSFlagPost(gDMAFlagGrp, gFlagDMAInt[uiDMAId], OS_FLAG_CLR, &err);
    
    *pDMARegCmmd |= DMA_START;
    
    return 1;
}

INT32U marsDMAWaitForInt_auto(INT32U uiDMAId)
{
    INT8U err;
    volatile INT32U *pDMARegCmmd;
    volatile INT32U *pDMARegCycle;
    
    
    if(uiDMAId >= DMA_ID_NUM)
        return 0;
    
    OSFlagPend(gDMAFlagGrp, gFlagDMAInt_AUTO[uiDMAId], OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, DMA_TIMEOUT*10, &err);

        #if (FPGA_BOARD_A1018_SERIES)
            #if IIS_TEST
            *(volatile INT32U *)gDMARegCmmd[uiDMAId]&= (~DMA_AUTONONSTOP_EN); //Auto Run Non-Stop mode
            #endif
        #endif    
    
    if (err != OS_NO_ERR)
    {
    	DEBUG_DMA("Wait DMA=%d Timeout :cycle=0x%X \n", uiDMAId, *(volatile INT32U *)gDMARegCycCnt[uiDMAId]);
    	pDMARegCmmd = (volatile INT32U *)gDMARegCmmd[uiDMAId];
        pDMARegCycle= (volatile INT32U *)gDMARegCycCnt[uiDMAId];
        
        //*pDMARegCmmd &= ( ~(DMA_START | DMA_AUTOMODE_EN) );
        *pDMARegCmmd=0;
        *pDMARegCycle =0;
        
        return 0;    // error
    }
    if(gCurDMAIntStat[uiDMAId] == DMA_INTSTAT_FINISH)
        return 1;    // ok
    else
        return 0;    // error
}   

INT32U marsDMAConfig_auto(INT32U uiDMAId, REGDMA_CFG_AUTO* pCfg)
{
    INT8U err;
    volatile INT32U *pDMARegSrcAddr, *pDMARegDstAddr, *pDMARegCycCnt, *pDMARegCmmd, *pDMARegAutoNum;
    volatile INT32U *pDMARegSrcAddr_alt, *pDMARegDstAddr_alt;

    if(uiDMAId >= DMA_ID_NUM)
        return 0;
    
    pDMARegSrcAddr = (volatile INT32U *)gDMARegSrcAddr[uiDMAId];
    pDMARegDstAddr = (volatile INT32U *)gDMARegDstAddr[uiDMAId];
    pDMARegCycCnt  = (volatile INT32U *)gDMARegCycCnt[uiDMAId];
    pDMARegCmmd    = (volatile INT32U *)gDMARegCmmd[uiDMAId];
    pDMARegAutoNum = (volatile INT32U *)gDMARegAutoNum[uiDMAId];
    pDMARegSrcAddr_alt = (volatile INT32U *)gDMARegSrcAddrAlt[uiDMAId];
    pDMARegDstAddr_alt = (volatile INT32U *)gDMARegDstAddrAlt[uiDMAId];
    
    *pDMARegCmmd &= (~(DMA_START | DMA_AUTOMODE_EN) );
    
    *pDMARegSrcAddr = pCfg->src;
    *pDMARegDstAddr = pCfg->dst;
    *pDMARegSrcAddr_alt=pCfg->src_alt;
    *pDMARegDstAddr_alt=pCfg->dst_alt;
    *pDMARegCycCnt  = pCfg->datacnt;
    *pDMARegCmmd    = pCfg->cmmd;
    *pDMARegAutoNum = pCfg->linecnt;

    gCurDMAIntStat[uiDMAId] = 0;
    // clear DMA INT-flag occurs before
    OSFlagPost(gDMAFlagGrp, gFlagDMAInt[uiDMAId], OS_FLAG_CLR, &err);
    OSFlagPost(gDMAFlagGrp, gFlagDMAInt_AUTO[uiDMAId], OS_FLAG_CLR, &err);

    *pDMARegCmmd |= (DMA_START | DMA_AUTOMODE_EN);
    
    return 1;
}

void isr_marsDMAAuto(int DmaID)
{
     DMA_CFG_AUTO *pdmaCfg_auto;
     volatile INT32U *pDMARegSrcAddr_alt, *pDMARegDstAddr_alt;
     

     pdmaCfg_auto= &MarsdmaCfg_auto[DmaID];

     pdmaCfg_auto->src += pdmaCfg_auto->src_stride;
     pdmaCfg_auto->dst += pdmaCfg_auto->dst_stride;

     pDMARegSrcAddr_alt = (volatile INT32U *)gDMARegSrcAddrAlt[DmaID];
     pDMARegDstAddr_alt = (volatile INT32U *)gDMARegDstAddrAlt[DmaID];

     *pDMARegSrcAddr_alt=pdmaCfg_auto->src;
     *pDMARegDstAddr_alt=pdmaCfg_auto->dst;     
}

void isr_marsDMA_RecAuto(int DmaID,u8* buf)
{
     DMA_CFG_AUTO *pdmaCfg_auto;
     volatile INT32U *pDMARegSrcAddr_alt, *pDMARegDstAddr_alt;

     pdmaCfg_auto= &MarsdmaCfg_auto[DmaID];

     pdmaCfg_auto->src += pdmaCfg_auto->src_stride;
     //pdmaCfg_auto->dst += pdmaCfg_auto->dst_stride;
     pdmaCfg_auto->dst= (u32)buf;

     pDMARegSrcAddr_alt = (volatile INT32U *)gDMARegSrcAddrAlt[DmaID];
     pDMARegDstAddr_alt = (volatile INT32U *)gDMARegDstAddrAlt[DmaID];

     *pDMARegSrcAddr_alt=pdmaCfg_auto->src;
     *pDMARegDstAddr_alt=pdmaCfg_auto->dst;     
}
void isr_marsDMA_PlayAuto(int DmaID,u8* buf)
{
     DMA_CFG_AUTO *pdmaCfg_auto;
     volatile INT32U *pDMARegSrcAddr_alt, *pDMARegDstAddr_alt;

     pdmaCfg_auto= &MarsdmaCfg_auto[DmaID];

     //pdmaCfg_auto->src += pdmaCfg_auto->src_stride;
     pdmaCfg_auto->src = (u32)buf;
     pdmaCfg_auto->dst += pdmaCfg_auto->dst_stride;
     //pdmaCfg_auto->dst= (u32)buf;

     pDMARegSrcAddr_alt = (volatile INT32U *)gDMARegSrcAddrAlt[DmaID];
     pDMARegDstAddr_alt = (volatile INT32U *)gDMARegDstAddrAlt[DmaID];

     *pDMARegSrcAddr_alt=pdmaCfg_auto->src;
     *pDMARegDstAddr_alt=pdmaCfg_auto->dst;     
}


#ifdef  TCM_DMA_SUPPORT

#if TCM_DMA_TEST
void TCM_DMATest(void)
{
    u32     *pSrc, *pDes;
    u8      *pbDes;
    u16     *pwDes;
    u32     i, lenth;

    DEBUG_DMA("TCM_DMATest() begin...\n");
    pSrc            = (u32*)mpeg4VideBuf;
    pDes            = (u32*)TcmSramBase;
    pbDes           = (u8*) TcmSramBase;
    pwDes           = (u16*)TcmSramBase;
    lenth           = 8192;

    // cpu r/w testing
    *(pDes + 0)     = 0xaaaaaaaa;
    *(pDes + 1)     = 0x55555555;
    *(pbDes + 0)    = 0;
    *(pbDes + 1)    = 1;
    *(pbDes + 2)    = 2;
    *(pbDes + 3)    = 3;
    *(pwDes + 2)    = 4;
    *(pwDes + 3)    = 5;
    if((*pDes != 0x03020100) || (*(pDes + 1) != 0x00050004))
        DEBUG_DMA("TCM SRAM CPU r/w error!!! 0x%08x 0x%08x\n", *pDes, *(pDes + 1));
    
    for(i = 0; i < (lenth / 4); i++)
    {
        *(pSrc + i)  = i;
    }
    memset((u8*)TcmSramBase, 0x5a, lenth);
    DEBUG_DMA("TCM_DMA_SDRAM() testing...\n");
    TCM_DMA_SDRAM(pSrc, pDes, lenth);
    DEBUG_DMA("TCM_DMA_SDRAM() finish...\n");
    for(i = 0; i < (lenth / 4); i++)
    {
        if(*pDes != i)
            DEBUG_DMA("pDes address 0x%08x = 0x%08x != 0x%08x.\n", (u32)pDes, *pDes, i);
        pDes++;
    }
    DEBUG_DMA("TCM_DMATest() end\n");
}
#endif

void TCM_DMAInit(void)
{
    gTCM_DMASemReq      = OSSemCreate(1);
    gTCM_DMASemFinish   = OSSemCreate(0);
}

void TCM_Reset(void)
{
    TCM_CTRL   |= TCM_RST;
    TCM_CTRL   &= ~TCM_RST;
}

/*

Routine Description:

    Copy data from SDRAM to TCM SRAM.

Arguments:

    pSrc    - Source address of SDRAM, must be word align(multiple of 4).
    pDes    - Destination address of SRAM, must be word align(multiple of 4).
    lenth   - lendth in byte.

Return Value:

    0 - Failure.
    1 - Success.

*/
u32 TCM_DMA_SDRAM(void* pSrc, void* pDes, u32 lenth)
{
    u32     BurstCycle, BytesPerBurst, BytesAllBurst, remainder, i;
    INT8U   err;
    u8      *pSrc1, *pDes1;

    BytesPerBurst   = 16 * 4;
    BurstCycle      = lenth / BytesPerBurst;
    BytesAllBurst   = BurstCycle * BytesPerBurst;
    //DEBUG_DMA("OSSemPend(gTCM_DMASemReq)...");
    OSSemPend(gTCM_DMASemReq, TCM_DMA_TIMEOUT, &err);
    //DEBUG_DMA("end\n");
    if (err != OS_NO_ERR)
    {
        DEBUG_DMA("TCM_DMA_SDRAM() Error: gTCM_DMASemReq is %d.\n", err);
        return 0;
    }

    TCM_SRC_ADDR    = (u32)pSrc;
    TCM_DST_ADDR    = (u32)pDes;
    TCM_CYCLE       = BurstCycle;
    TCM_CTRL        = TCM_INT_EN | TCM_BURST_16 | TCM_SRC_INC | TCM_REQ_SEL_SDRAM;
    TCM_CTRL       |= TCM_DMA_TRIG;
    //DEBUG_DMA("OSSemPend(gTCM_DMASemFinish)...waiting\n");
    OSSemPend(gTCM_DMASemFinish, TCM_DMA_TIMEOUT, &err);
    DEBUG_DMA("OSSemPend(gTCM_DMASemFinish)...end\n");
    if (err != OS_NO_ERR)
    {
        DEBUG_DMA("TCM_DMA_SDRAM() Error: gTCM_DMASemFinish is %d.\n", err);
        TCM_Reset();
        OSSemPost(gTCM_DMASemReq);
        return 0;
    }

    pSrc1           = (u8*)pSrc + BytesAllBurst;
    pDes1           = (u8*)pDes + BytesAllBurst;
    remainder       = lenth - BytesAllBurst;
    memcpy(pDes, pSrc, remainder);

    //DEBUG_DMA("OSSemPost(gTCM_DMASemReq)...\n");
    OSSemPost(gTCM_DMASemReq);
    return 1;
}

void TCM_DMAIntHandler(void)
{
    u32 IntStatus;

    IntStatus   = TCM_CTRL;
    if(IntStatus & TCM_FINISH)
    {
        //DEBUG_DMA("TCM_DMAIntHandler() TCM_CTRL = 0x%08x\n", IntStatus);
        TCM_CTRL   &= ~TCM_FINISH;
        //DEBUG_DMA("OSSemPost(gTCM_DMASemFinish)\n");
        OSSemPost(gTCM_DMASemFinish);
    }
}

#endif



