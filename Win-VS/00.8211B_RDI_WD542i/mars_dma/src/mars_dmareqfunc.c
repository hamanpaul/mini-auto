#include <general.h>
#include <osapi.h>
#include <mars_dma.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <board.h>

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A)) 
  DMA_CFG_AUTO MarsdmaCfg_auto[DMA_REQ_MAX];

extern void mcpu_ByteMemcpy(u8 *DstAddr, u8 *SrcAddr, unsigned int ByteCnt);
#endif

const INT32U gDMAReqCmmd[DMA_REQ_MAX] = {

// DMA_REQ_IIS_RECORD
#if(AUDIO_OPTION == AUDIO_ADC_DAC)        
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_ADCRX_REC),
#else
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_IIS_REC),
#endif

// DMA_REQ_IIS_PLAY
#if(CHIP_OPTION == CHIP_PA9001D)
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_IIS_PLAY|DMA_HIGH_PRI),
#else
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_IIS_PLAY),
#endif
#if ((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
// DMA_REQ_IIS2_RECORD
 #if(AUDIO_OPTION == AUDIO_ADC_DAC)
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_ADCRX_REC),
 #else
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_IIS2_REC),
 #endif
 
// DMA_REQ_IIS2_PLAY
 #if(CHIP_OPTION == CHIP_PA9001D)
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_IIS2_PLAY|DMA_HIGH_PRI),
 #else
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_IIS2_PLAY),
 #endif
#endif
#if ((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B))
// DMA_REQ_IIS3_RECORD
 #if(AUDIO_OPTION == AUDIO_ADC_DAC)
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_ADCRX_REC),
 #else
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_IIS3_REC),
 #endif
// DMA_REQ_IIS3_PLAY
 #if(CHIP_OPTION == CHIP_PA9001D)
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_IIS3_PLAY|DMA_HIGH_PRI),
 #else
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_IIS3_PLAY),
 #endif
// DMA_REQ_IIS4_RECORD
 #if(AUDIO_OPTION == AUDIO_ADC_DAC)
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_ADCRX_REC),
 #else
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_IIS4_REC),
 #endif
// DMA_REQ_IIS4_PLAY
 #if(CHIP_OPTION == CHIP_PA9001D)
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_IIS4_PLAY|DMA_HIGH_PRI),
 #else
     (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_IIS4_PLAY),
 #endif
#endif

// DMA_REQ_SD_READ
#if(CHIP_OPTION == CHIP_PA9001D)
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SD|DMA_BURST4|DMA_HIGH_PRI),
#else
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SD|DMA_BURST4),
#endif

// DMA_REQ_SD_WRITE
#if(CHIP_OPTION == CHIP_A1013A)
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_SD|DMA_BURST4|DMA_HIGH_PRI),
#else
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_SD|DMA_BURST4),
#endif

// DMA_REQ_SMC_READ
#if(CHIP_OPTION == CHIP_PA9001D)
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SMC|DMA_HIGH_PRI),
#else
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SMC),
#endif

// DMA_REQ_SMC_WRITE
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_SMC),


// DMA_REQ_USB_READ
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_USB),
// DMA_REQ_USB_WRITE
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_USB),
//#endif    
// DMA_REQ_SPI_READ
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SPI),
// DMA_REQ_SPI_READ_p
    (DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SPI),

// DMA_REQ_SPI_WRITE
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_SPI),
// DMA_REQ_SPI_WRITE_P
    (DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_SPI),

// DMA_REQ_MEM_COPY_BYTE
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_AHB|DMA_SRC_INC1|DMA_DST_INC1|DMA_DATA_BYTE|DMA_NONE),

// DMA_REQ_MEM_COPY_WORD
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_AHB|DMA_SRC_INC4|DMA_DST_INC4|DMA_DATA_WORD|DMA_NONE),

// DMA_REQ_MEM_SET_BYTE
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC1|DMA_DATA_BYTE|DMA_NONE),

// DMA_REQ_MEM_SET_WORD
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_NONE),
    
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
// DMA_REQ_CF_READ
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_CF|DMA_BURST4),
// DMA_REQ_CF_WRITE
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_CF|DMA_BURST4),

// DMA_REQ_GIP_READ
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_GPI),
// DMA_REQ_GIP_WRITE
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_GPI),
// DMA_REQ_SPI2_READ
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SPI_2),
// DMA_REQ_SPI2_READ_P
    (DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SPI_2),    
// DMA_REQ_SPI2_WRITE
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_SPI_2),
// DMA_REQ_SPI2_WRITE_P
    (DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_SPI_2),
// DMA_REQ_SPI3_READ
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SPI_3),
//DMA_REQ_SPI3_READ_P
    (DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SPI_3),
// DMA_REQ_SPI3_WRITE
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_SPI_3),
// DMA_REQ_SPI3_WRITE_P
    (DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_SPI_3),    
//DMA_REQ_ADC_G1_RECORD    
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_ADCRX_G1_REC),    
//DMA_REQ_ADC_G2_RECORD    
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_ADCRX_G2_REC),    
//DMA_REQ_ADC_G3_RECORD    
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_ADCRX_G3_REC),    
// DMA_REQ_SPI5_READ
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_APB|DMA_DST_AHB|DMA_SRC_INC0|DMA_DST_INC4|DMA_DATA_WORD|DMA_SPI_5),
// DMA_REQ_SPI5_WRITE
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_SPI_5),
//DMA_REQ_MEM_BLKCOPY_WORD_AUTO
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_AHB|DMA_SRC_INC4|DMA_DST_INC4|DMA_DATA_WORD|DMA_NONE | DMA_INT_ENA_AUTOFIN),
//DMA_REQ_MEM_BLKCOPY_BYTE_AUTO
    (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_AHB|DMA_SRC_INC1|DMA_DST_INC1|DMA_DATA_BYTE|DMA_NONE | DMA_INT_ENA_AUTOFIN),
#endif
};
extern const INT32U gDMARegSrcAddr[DMA_ID_NUM];
extern const INT32U gDMARegDstAddr[DMA_ID_NUM];
extern const INT32U gDMARegCycCnt[DMA_ID_NUM];
extern const INT32U gDMARegCmmd[DMA_ID_NUM];

INT32U marsDMAReq(INT32U uiReqId, DMA_CFG* pCfg)
{
    INT32U uiDMAId, uiDMACmmd=0;
    REGDMA_CFG  RegDMACfg;
    REGDMA_CFG  *pRegDMACfg = (REGDMA_CFG*)&RegDMACfg;
    
    marsDMAOpen(&uiDMAId, NULL);
    
    uiDMACmmd = gDMAReqCmmd[uiReqId];
    if(pCfg->burst)
        uiDMACmmd |= DMA_BURST4;
            
    pRegDMACfg->src = pCfg->src;
    pRegDMACfg->dst = pCfg->dst;
    pRegDMACfg->cnt = pCfg->cnt;
    pRegDMACfg->cmmd = uiDMACmmd;

#ifdef MMU_SUPPORT
    if (pCfg->dst < MEMORY_POOL_START_ADDR)
  #ifdef WRITE_BACK
    #ifdef COARSE_PAGE
        if (cehck_cache_by_address(pCfg->dst))
    #endif
            Test_Clean_Dcache();
  #else
    #ifdef COARSE_PAGE
        if (cehck_cache_by_address(pCfg->dst))
    #endif
            flush_dcache();     // We should flush Dcache before any DMA transfer
  #endif
#endif

    marsDMAConfig(uiDMAId, pRegDMACfg);
    return uiDMAId;
}

INT32U marsDMACheckReady(INT32U uiDMAId)
{
    INT32U status;
    
    status = marsDMAWaitForInt(uiDMAId);
    marsDMAClose(uiDMAId);
    return (status);    // 1->ok; 0->error
}
extern OS_FLAG_GRP  *gDMAFlagGrp;
extern INT32U gFlagDMAInt[DMA_ID_NUM]; 
extern INT32U gFlagDMAInt_AUTO[DMA_ID_NUM];
extern FP_INT gpDMAIsrTbl[DMA_ID_NUM];
extern INT32U gCurDMAIntStat[DMA_ID_NUM];
INT32U marsDMACheckReady_P(INT32U uiDMAId)
{
    INT32U status;
    INT8U err;
    INT32U i, CurDMAIntStat,uiDMAChkBit=0x03;
        volatile INT32U *pDMAIntSta;
    INT32U AutoDMAIntStat;
    unsigned int  cpu_sr = 0;
    int err_cnt=0;
    i=uiDMAId;
    OS_ENTER_CRITICAL();
    pDMAIntSta = (volatile INT32U *)REG_DMAINTSTAT;
    
    CurDMAIntStat = *pDMAIntSta;
    //DEBUG_DMA("0x%x ",CurDMAIntStat);
     status = (CurDMAIntStat >> (2*i))& 0x03;
     
	while(status==0)
	{
		if(err_cnt > 8000)
		{
			printf("\x1B[96m!\x1B[0m");
			break;
		}
		err_cnt++;
	    CurDMAIntStat |= *pDMAIntSta;
	    status = (CurDMAIntStat >> (2*i))& 0x03;
	}
	// DEBUG_DMA("status=%d id=%d",status,uiDMAId);
	CurDMAIntStat = CurDMAIntStat &(~(uiDMAChkBit<<(2*i)));
#if 1
 //for(i=0; i<DMA_ID_NUM; i++)
	for(i=0; i<2; i++)
	{
		//if (i==(uiDMAId)) continue;
		if(CurDMAIntStat & uiDMAChkBit)
		{
			//DEBUG_DMA("$%d",i);
			gCurDMAIntStat[i] = (CurDMAIntStat >> (2*i)) & 0x03;
			OSFlagPost(gDMAFlagGrp, gFlagDMAInt[i], OS_FLAG_SET, &err);
			if(gpDMAIsrTbl[i] != NULL)
				gpDMAIsrTbl[i](i);
		}
		uiDMAChkBit <<= 2;
	}

	if(CurDMAIntStat & 0x0ff0000)
	{
		AutoDMAIntStat=CurDMAIntStat >> 16;
		//for(i=0; i<DMA_ID_NUM; i++)
		for(i=0; i<2; i++)
		{
		    if(AutoDMAIntStat & 0x01)
		    {
		        OSFlagPost(gDMAFlagGrp, gFlagDMAInt_AUTO[i], OS_FLAG_SET, &err);
		        DEBUG_DMA("o");
		    }
		    AutoDMAIntStat >>=1;
		}
	}
#endif
      //DEBUG_DMA("0x%x ",CurDMAIntStat);
     //DEBUG_DMA("Wait DMA=%d Timeout :cycle=0x%X \n", uiDMAId, *(volatile INT32U *)gDMARegCycCnt[uiDMAId]);    
	OS_EXIT_CRITICAL();
    marsDMAClose(uiDMAId);
      
	if(status == DMA_INTSTAT_FINISH)
	{
		// DEBUG_DMA("uiDMAId%d 0x%x ",uiDMAId,CurDMAIntStat);
		return 1;    // ok
	}
	else
	{
		DEBUG_DMA("DMA =%d fail\n",uiDMAId);
		return 0;    // error
	}
        
   
}
void *memcpy_hw(void *dest, const void *src, unsigned int count)
{
    char     *dest1, *src1;
    INT32U   size1, uiDMAId,status;
    DMA_CFG dmaCfg;
    int i;

    dest1   = (char*)dest;
    src1    = (char*)src;

    if(count >= 0x80000000)
    {
        DEBUG_DMA("Error: memcpy_hw(0x%08x, 0x%08x, %d)\n", dest, src, count);
        return dest;
    }
    else if (count <= 768)
    {
        return memcpy(dest1, src1, count);
    }
    else //if(size1 != size2)
    {
    #if DMA_TEST  //just for test
       //DEBUG_DMA("(%d) ",count);
        size1=count & 0x3;
        if (size1)
        {
            memcpy(dest1, src1, size1);
            dest1  += size1;
            src1   += size1;
            count  -= size1;
        }
    
        dmaCfg.src   = (INT32U)src1;
        dmaCfg.dst   = (INT32U)dest1;
        dmaCfg.burst = 0;
        dmaCfg.cnt   = count / 4;

        uiDMAId = marsDMAReq(DMA_REQ_MEM_COPY_WORD, &dmaCfg);
     #if DMA_TEST
        DEBUG_DMA("uiDMAId = %d\n",uiDMAId);
     #endif
        status=marsDMACheckReady(uiDMAId);
        if(status == 0)
        {
            DEBUG_DMA("Warning! memcpy_hw is fail!\n");
            memcpy(dest1, src1, count);
        }
    #elif( ((CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
        (CHIP_OPTION == CHIP_A1026A)) && (DRAMCNTRL_SEL == DRAMCNTRL_DDR2) )
       //Lucian: 由於DDR2 controller 不支援 DMA_REQ_MEM_COPY_BYTE,A1013另外新增MCPU做memory copy.
       mcpu_ByteMemcpy((u8 *)dest, (u8 *)src,count);
       /*
       for(i=0;i<count;i++)
       {
           if(dest1[i] != src1[i])
           {
              DEBUG_DMA("-->Memcpy Compare error\n");
              break;
           }
       }
       */
    #else
        //DEBUG_DMA("(%d) ",count);
        size1=count & 0x3;
        if (size1)
        {
            memcpy(dest1, src1, size1);
            dest1  += size1;
            src1   += size1;
            count  -= size1;
        }
    
        dmaCfg.src   = (INT32U)src1;
        dmaCfg.dst   = (INT32U)dest1;
        dmaCfg.burst = 1;
        dmaCfg.cnt   = count / 4;

        uiDMAId = marsDMAReq(DMA_REQ_MEM_COPY_BYTE, &dmaCfg);
        status=marsDMACheckReady(uiDMAId);
        if(status == 0)
        {
            DEBUG_DMA("Warning! memcpy_hw is fail!\n");
            memcpy(dest1, src1, count);
        }
    #endif    
    }
    return dest;
}


//Lucian: 用於byte pattern
void *memset_hw(void *ori_dest, unsigned char dataVal, unsigned int ori_count)
{
    u8      *dest;
    u32     *dest1, *src1, uiDstAddr, count;
    INT32U   size1, uiDMAId, status, uiDataVal;
    DMA_CFG dmaCfg;
    u32 guiHWMemsetVal;
    u32 *gpuiHWMemsetVal;

    gpuiHWMemsetVal=(u32*)&guiHWMemsetVal;
    if(ori_count >= 0x80000000)
    {
        DEBUG_DMA("Error: memset_hw(0x%08x, 0x%02x, %d)\n", ori_dest, dataVal, ori_count);
        return ori_dest;
    }
    else if (ori_count <= 2048)
    {
        return memset(ori_dest, dataVal, ori_count);
    }

    
    dest  = (u8*)ori_dest;
    count = ori_count;
    
    uiDataVal = dataVal;
    *gpuiHWMemsetVal = ( uiDataVal|(uiDataVal<<8)|(uiDataVal<<16)|(uiDataVal<<24) );
    
    uiDstAddr = (u32)dest;
    if(uiDstAddr & 0x03)
    {
        size1 = 4-(uiDstAddr & 0x03);
        memset(dest, dataVal, size1);
        dest  += size1;
        count -= size1;
    }
    
    dest1   = (u32*)dest;
    src1    = (u32*)gpuiHWMemsetVal;
    
    dmaCfg.src   = (INT32U)src1;
    dmaCfg.dst   = (INT32U)dest1;
    dmaCfg.burst = 0;
    dmaCfg.cnt   = (count>>2);    // word

    uiDMAId = marsDMAReq(DMA_REQ_MEM_SET_WORD, &dmaCfg);
 #if DMA_TEST
    DEBUG_DMA("uiDMAId = %d\n",uiDMAId);
 #endif
    status=marsDMACheckReady(uiDMAId);
    
    if(status == 0)
    {
        DEBUG_DMA("Warning! memset_hw is fail!\n");
        memset(ori_dest, dataVal, ori_count);
        return ori_dest;
    }
    //DEBUG_DMA("DMA OK!\r\n");    
    
    if(count % 4)
    {
    	dest += ((count>>2)<<2);    // DMA moved
        memset(dest, dataVal, count%4);
    }
    
    return dest;
}


//Lucian: 用於word pattern
void *memset_hw_Word(void *ori_dest, unsigned int dataVal, int ori_count)
{
    u8      *dest;
    u32     *dest1, *src1, uiDstAddr, count;
    INT32U   size1, uiDMAId, status, uiDataVal;
    DMA_CFG dmaCfg;
    int i;
    u32 guiHWMemsetVal;
    u32 *gpuiHWMemsetVal;
    //---------------------------------------------//

    gpuiHWMemsetVal=(u32*)&guiHWMemsetVal;
    if(ori_count >= 0x80000000)
    {
        DEBUG_DMA("Error: memset_hw_Word(0x%08x, 0x%08x, %d)\n", ori_dest, dataVal, ori_count);
        return ori_dest;
    }
    else if (ori_count <= 2048)
    {
        dest1= (u32 *)ori_dest;
        for(i=0 ; i<ori_count ; i+=4)
        {
            *dest1 = dataVal;
            dest1++;
        }
        return dest1;
    }
    
    dest  = (u8*)ori_dest;
    count = (u32)ori_count;
    uiDstAddr = (u32)dest;    
    *gpuiHWMemsetVal = dataVal;
    
    if(uiDstAddr & 0x0F)
    {
        DEBUG_DMA("Warning! DMA address is not word-alignment!\n");
        size1 = 16-(uiDstAddr & 0x0F);
        memset(dest, dataVal & 0x0ff, size1);
        dest  += size1;
        count -= size1;
    }
    
    dest1   = (u32*)dest;
    src1    = (u32*)gpuiHWMemsetVal;
    
    dmaCfg.src   = (INT32U)src1;
    dmaCfg.dst   = (INT32U)dest1;
    dmaCfg.burst = 1;
    dmaCfg.cnt   = (count>>4);    // word

    uiDMAId = marsDMAReq(DMA_REQ_MEM_SET_WORD, &dmaCfg);
    status=marsDMACheckReady(uiDMAId);
    
    if(status == 0)
    {
        DEBUG_DMA("Warning! memset_hw is fail!\n");
        memset(ori_dest, dataVal, (u32)ori_count);
        return ori_dest;
    }
    //DEBUG_DMA("DMA OK!\r\n");    
    
    if(count % 16)
    {
    	dest += ((count>>4)<<4);    // DMA moved
        memset(dest, dataVal & 0x0ff, count%16);
    }
    
    return dest;
}

  //Note: datacnt must be 4x
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)|| (CHIP_OPTION == CHIP_A1026A))
    INT32U marsDMACheckReady_auto(INT32U uiDMAId)
    {
        INT32U status;
        
        status = marsDMAWaitForInt_auto(uiDMAId);
        marsDMAClose(uiDMAId);
        return (status);    // 1->ok; 0->error
    }
    
    void *memBlkCpyAuto_hw(unsigned char *dest, unsigned char *src, 
                                      unsigned int stride_dst, unsigned int stride_src, 
                                      unsigned int datacnt, unsigned int linecnt)
    {
         int i;
         unsigned char *pp,*qq;
         INT32U uiDMAId,status, uiDMACmmd=0;
         DMA_CFG_AUTO *pdmaCfg_auto;
         REGDMA_CFG_AUTO  RegDMACfg_auto;
         REGDMA_CFG_AUTO  *pRegDMACfg_auto = (REGDMA_CFG_AUTO*)&RegDMACfg_auto;


         if (datacnt <= 2048)
         {
            pp=dest;
            qq=src;
            for(i=0;i<linecnt;i++)
            {
              memcpy(pp,qq,datacnt);
              pp += stride_dst;
              qq += stride_src;
            }
         }
         else
         {
            marsDMAOpen(&uiDMAId, isr_marsDMAAuto);
         #if DMA_TEST
           DEBUG_DMA("uiDMAId = %d\n",uiDMAId);
         #endif
            pdmaCfg_auto= &MarsdmaCfg_auto[uiDMAId];
            
            pdmaCfg_auto->src       = (INT32U)src;
            pdmaCfg_auto->dst       = (INT32U)dest;
            pdmaCfg_auto->src_stride= stride_src;
            pdmaCfg_auto->dst_stride= stride_dst;

            pdmaCfg_auto->burst     = 0;
            pdmaCfg_auto->datacnt   = datacnt / 4;
            pdmaCfg_auto->linecnt   = linecnt;
        
            uiDMACmmd = gDMAReqCmmd[DMA_REQ_MEM_BLKCOPY_WORD_AUTO];
            if(pdmaCfg_auto->burst)
                uiDMACmmd |= DMA_BURST4;
                    
            pRegDMACfg_auto->src = pdmaCfg_auto->src;
            pRegDMACfg_auto->dst = pdmaCfg_auto->dst;

            pdmaCfg_auto->src += pdmaCfg_auto->src_stride;
            pdmaCfg_auto->dst += pdmaCfg_auto->dst_stride;

            pRegDMACfg_auto->src_alt= pdmaCfg_auto->src;
            pRegDMACfg_auto->dst_alt= pdmaCfg_auto->dst;
            
            pRegDMACfg_auto->datacnt = pdmaCfg_auto->datacnt;
            pRegDMACfg_auto->linecnt = pdmaCfg_auto->linecnt;
            pRegDMACfg_auto->cmmd = uiDMACmmd;

   #ifdef MMU_SUPPORT
            if (pRegDMACfg_auto->dst < MEMORY_POOL_START_ADDR)
       #ifdef WRITE_BACK
           #ifdef COARSE_PAGE
                if (cehck_cache_by_address(pRegDMACfg_auto->dst))
           #endif
                    Test_Clean_Dcache();
       #else
           #ifdef COARSE_PAGE
                if (cehck_cache_by_address(pRegDMACfg_auto->dst))
           #endif
                    //DEBUG_DMA("Destination: %x \n",pCfg->dst);
                    flush_dcache();     // We should flush Dcache before any DMA transfer
       #endif
   #endif

            marsDMAConfig_auto(uiDMAId, pRegDMACfg_auto);

            status=marsDMACheckReady_auto(uiDMAId);
            if(status == 0)
            {
                DEBUG_DMA("Error! memBlkCpyAuto_hw is fail!\n");
            }

         }


    }

    
#endif

#if DMA_TEST
extern unsigned char gDMALockStatus;
   #define DMATESTCNTMAX  65536
   
int marsDMA_Test()
{
     int i,j;
     unsigned char *pp,*qq;
     unsigned char mask[8]={0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f};

     //----Set PKbuf0: 0,1,2,3,4 ... 65535
     pp=PKBuf0;
     for(i=0;i<DMATESTCNTMAX;i++)
     {
        *pp=i;
        pp ++;
     }

     for(j=0;j<DMA_ID_NUM;j++)
     {
         DEBUG_DMA("=============Test DMA CH-%d==============\n",j);
         gDMALockStatus=0;
         gDMALockStatus |=mask[j];
         //---Memory copy test---//
         DEBUG_DMA("---memcpy_hw test---\n");
         memcpy_hw(PKBuf1, PKBuf0, DMATESTCNTMAX);
         pp=PKBuf0;
         qq=PKBuf1;
         for(i=0;i<DMATESTCNTMAX;i++)
         {
             if( *pp != *qq )
                return 0;    
             pp ++;
             qq ++;
         }
         DEBUG_DMA("---memcpy_hw pass---\n");

         //---Memory set test---//
         DEBUG_DMA("---memset_hw test:0x5a---\n");
         memset_hw(PKBuf1, 0x5a, DMATESTCNTMAX);
         pp=PKBuf1;
         for(i=0;i<DMATESTCNTMAX;i++)
         {
             if( *pp != 0x5a)
                return 0;    
             pp ++;
         }
         DEBUG_DMA("---memset_hw pass---\n");
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A))
         //----Memorycopy auto mode test---//
         DEBUG_DMA("---memBlkCpyAuto_hw test---\n");
         memBlkCpyAuto_hw(PKBuf1, PKBuf0,8192,8192,8192, DMATESTCNTMAX/8192);
         pp=PKBuf0;
         qq=PKBuf1;
         for(i=0;i<DMATESTCNTMAX;i++)
         {
             if( *pp != *qq )
             {
                DEBUG_DMA("Err! count=%d,src=0x%x,dst=0x%x\n",i,*pp,*qq);
                return 0;    
             }
             pp ++;
             qq ++;
         }
         DEBUG_DMA("---memBlkCpyAuto_hw pass---\n");
#endif
         //---Memory set test---//
         DEBUG_DMA("---memset_hw test:0xa5---\n");
         memset_hw(PKBuf1, 0xa5, DMATESTCNTMAX);
         pp=PKBuf1;
         for(i=0;i<DMATESTCNTMAX;i++)
         {
             if( *pp != 0xa5)
                return 0;    
             pp ++;
         }
         DEBUG_DMA("---memset_hw pass---\n");
     }

        
     gDMALockStatus=0;

     return 1;

}



#endif
