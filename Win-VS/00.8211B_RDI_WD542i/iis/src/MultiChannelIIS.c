
/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    iis.c

Abstract:

    The routines of IIS.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"

#if MULTI_CHANNEL_VIDEO_REC

#include "board.h"
#include "task.h"
#include "iis.h"
#include "iisreg.h"
#include "iisapi.h"
#include <../inc/mars_controller/mars_dma.h>
#include "mp4api.h"
#include "asfapi.h"
#include "sysapi.h"
#include "aviapi.h"
#include "mpeg4api.h"
#include "sysapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "adcapi.h"
#include "osapi.h"
#include "timerapi.h"
#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
#include "ima_adpcm_api.h"
#endif
#include "GlobalVariable.h"

#if (HW_BOARD_OPTION == MR6730_AFN)
#include "..\..\ui\inc\MainFlow.h"
#endif

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

/* task and event related */
OS_STK      iisTaskStack0[IIS_TASK_STACK_SIZE]; /* Stack of task MultiChannelIISRecordTask() */
OS_STK      iisTaskStack1[IIS_TASK_STACK_SIZE]; /* Stack of task MultiChannelIISRecordTask() */
OS_STK      iisTaskStack2[IIS_TASK_STACK_SIZE]; /* Stack of task MultiChannelIISRecordTask() */
OS_STK      iisTaskStack3[IIS_TASK_STACK_SIZE]; /* Stack of task MultiChannelIISRecordTask() */

PVIDEO_CLIP_OPTION  pvcoIISRecDMA[DMA_ID_NUM];

#if AFN_DEBUG
u8      AudioTrigger[MULTI_CHANNEL_LOCAL_MAX];
#endif

/*
 *********************************************************************************************************
 * Extern Varaibel
 *********************************************************************************************************
 */

extern WAVEFORMAT iisPlayFormat;
extern WAVEFORMAT iisRecFormat;
#if NIC_SUPPORT
extern u8 EnableStreaming;
extern u8  LocalChannelSource; // ch0 source
#endif
extern const INT32U gDMAReqCmmd[DMA_REQ_MAX];
extern OS_EVENT *gRfiuAVCmpSemEvt[MAX_RFIU_UNIT];

extern  DMA_CFG_AUTO MarsdmaCfg_auto[DMA_REQ_MAX];

#if AFN_DEBUG
#if (AFN_DEBUG_MORE_CHECK)
extern u8      MultiIIS_ErrCnt[];//[MULTI_CHANNEL_LOCAL_MAX]
extern u8      MulChASF_Error[];//[MULTI_CHANNEL_LOCAL_MAX]
#endif
#endif

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

    Initialize multiple channel audio record task.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelIISRecordTaskCreate(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    int i;

    /* initialize sound buffer */
    for(i = 0; i < IIS_BUF_NUM; i++) {
        pVideoClipOption->iisSounBufMng[i].buffer = pVideoClipOption->iisSounBuf[i];
    }

    /* Create the semaphore */
    pVideoClipOption->iisTrgSemEvt      = OSSemCreate(IIS_BUF_NUM - 2); /* guarded for ping-pong buffer */
    pVideoClipOption->iisCmpSemEvt      = OSSemCreate(0);
   	//pVideoClipOption->iisplayCmpEvt     = OSSemCreate(0);
   	//pVideoClipOption->AudioRTPCmpSemEvt = OSSemCreate(0);
    //AudioRTPCmpSemEvt[pVideoClipOption->VideoChannelID] = OSSemCreate(0);
    pVideoClipOption->gIISPlayUseSem    = OSSemCreate(1);
    pVideoClipOption->gIISRecUseSem     = OSSemCreate(1);

    //pVideoClipOption->iisPlayDMACnt     = 0;
	pVideoClipOption->iisRecDMACnt      = 0;
	pVideoClipOption->IIS_Task_Stop     = 0;

    /* Create the task */
    DEBUG_IIS("Trace: MultiChannelIISRecordTaskCreate(%d) task creating...\n", pVideoClipOption->VideoChannelID);
    switch(pVideoClipOption->VideoChannelID)
    {
        case 0:
            pVideoClipOption->AudioChannelID    = 1;
            OSTaskCreate(MULTI_CH_IIS_REC_TASK, pVideoClipOption, MULTI_CH_IIS_TASK_STACK0, IIS_TASK_PRIORITY_UNIT0);
            break;
        case 1:
        #if IS_COMMAX_DOORPHONE // 無外部Amp的用這個, Mircro-phone in, ADC_IN3
            pVideoClipOption->AudioChannelID    = 0;
        #else
            pVideoClipOption->AudioChannelID    = 1;
        #endif
            OSTaskCreate(MULTI_CH_IIS_REC_TASK, pVideoClipOption, MULTI_CH_IIS_TASK_STACK1, IIS_TASK_PRIORITY_UNIT1);
            break;
        case 2:
            pVideoClipOption->AudioChannelID    = 0;
            OSTaskCreate(MULTI_CH_IIS_REC_TASK, pVideoClipOption, MULTI_CH_IIS_TASK_STACK2, IIS_TASK_PRIORITY_UNIT2);
            break;
        case 3:
            pVideoClipOption->AudioChannelID    = 0;
            OSTaskCreate(MULTI_CH_IIS_REC_TASK, pVideoClipOption, MULTI_CH_IIS_TASK_STACK3, IIS_TASK_PRIORITY_UNIT3);
            break;
        default:
            DEBUG_IIS("Error: MultiChannelIISRecordTaskCreate() don't support pVideoClipOption->VideoChannelID = %d\n", pVideoClipOption->VideoChannelID);
            return 0;
    }
    DEBUG_IIS("(%d) finish!!\n", pVideoClipOption->AudioChannelID);

    return 1;
}

/*

Routine Description:

    Destroy Multiple channel audio record task.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelIISRecordTaskDestroy(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    int     i;
    INT8U   err;

    DEBUG_IIS("MultiChannelIISRecordTaskDestroy(%d)\n", pVideoClipOption->VideoChannelID);

    MultiChannelIIsStopRec(pVideoClipOption);

    /* Delete the task */
    switch(pVideoClipOption->VideoChannelID)
    {
        case 0:
            if(OS_NO_ERR !=  OSTaskSuspend(IIS_TASK_PRIORITY_UNIT0))
                DEBUG_IIS("OSTaskSuspend(IIS_TASK_PRIORITY_UNIT0) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(IIS_TASK_PRIORITY_UNIT0))
                DEBUG_IIS("OSTaskDel(IIS_TASK_PRIORITY_UNIT0) error!!\n");
            break;
        case 1:
            if(OS_NO_ERR !=  OSTaskSuspend(IIS_TASK_PRIORITY_UNIT1))
                DEBUG_IIS("OSTaskSuspend(IIS_TASK_PRIORITY_UNIT1) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(IIS_TASK_PRIORITY_UNIT1))
                DEBUG_IIS("OSTaskDel(IIS_TASK_PRIORITY_UNIT1) error!!\n");
            break;
        case 2:
            if(OS_NO_ERR !=  OSTaskSuspend(IIS_TASK_PRIORITY_UNIT2))
                DEBUG_IIS("OSTaskSuspend(IIS_TASK_PRIORITY_UNIT2) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(IIS_TASK_PRIORITY_UNIT2))
                DEBUG_IIS("OSTaskDel(IIS_TASK_PRIORITY_UNIT2) error!!\n");
            break;
        case 3:
            if(OS_NO_ERR !=  OSTaskSuspend(IIS_TASK_PRIORITY_UNIT3))
                DEBUG_IIS("OSTaskSuspend(IIS_TASK_PRIORITY_UNIT3) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(IIS_TASK_PRIORITY_UNIT3))
                DEBUG_IIS("OSTaskDel(IIS_TASK_PRIORITY_UNIT3) error!!\n");
            break;
        default:
            DEBUG_IIS("Error: MultiChannelIISRecordTaskDestroy() don't support pVideoClipOption->VideoChannelID = %d", pVideoClipOption->VideoChannelID);
    }

    /* Delete the semaphore */
    pVideoClipOption->iisTrgSemEvt      = OSSemDel(pVideoClipOption->iisTrgSemEvt, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->iisTrgSemEvt)
        DEBUG_IIS("OSSemDel(pVideoClipOption->iisTrgSemEvt) error!!\n");

    pVideoClipOption->iisCmpSemEvt      = OSSemDel(pVideoClipOption->iisCmpSemEvt, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->iisCmpSemEvt)
        DEBUG_IIS("OSSemDel(pVideoClipOption->iisCmpSemEvt) error!!\n");

   	//pVideoClipOption->iisplayCmpEvt     = OSSemDel(pVideoClipOption->iisplayCmpEvt, OS_DEL_ALWAYS, &err);
    //if(pVideoClipOption->iisplayCmpEvt)
    //    DEBUG_IIS("OSSemDel(pVideoClipOption->iisplayCmpEvt) error!!\n");

   	//pVideoClipOption->VideoRTPCmpSemEvt = OSSemDel(pVideoClipOption->VideoRTPCmpSemEvt, OS_DEL_ALWAYS, &err);
    //if(pVideoClipOption->VideoRTPCmpSemEvt)
    //    DEBUG_IIS("OSSemDel(pVideoClipOption->VideoRTPCmpSemEvt) error!!\n");

   	pVideoClipOption->gIISPlayUseSem    = OSSemDel(pVideoClipOption->gIISPlayUseSem, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->gIISPlayUseSem)
        DEBUG_IIS("OSSemDel(pVideoClipOption->gIISPlayUseSem) error!!\n");

   	pVideoClipOption->gIISRecUseSem     = OSSemDel(pVideoClipOption->gIISRecUseSem, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->gIISRecUseSem)
        DEBUG_IIS("OSSemDel(pVideoClipOption->gIISRecUseSem) error!!\n");

    pVideoClipOption->IIS_Task_Go   = 0;    // 0: never run, 1: ever run

    return 1;
}

/*

Routine Description:

    The multiple channel audio record DMA setting.

Arguments:

    buf                 - Destination buffer address.
    siz                 - Buffer size.
    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    None.

*/

void MultiChanneliisSetNextRecDMA(u8* buf, u32 siz, VIDEO_CLIP_OPTION *pVideoClipOption)
{
	u32         ADCRecGrp;
    INT32U      uiDMAId, uiDMACmmd=0;
    REGDMA_CFG  RegDMACfg;
    REGDMA_CFG  *pRegDMACfg = (REGDMA_CFG*)&RegDMACfg;

	//DEBUG_IIS("MultiChanneliisSetNextRecDMA(%d)\n", pVideoClipOption->AudioChannelID);

	/* set read data dma */
  #if(AUDIO_OPTION == AUDIO_ADC_DAC)
    #if((CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B))
    ADCRecGrp = ((AdcCtrlReg & 0x0000003C)>>2); //Lsk : must call adcInit() to set adc record group.
    if(ADCRecGrp == 1)
    {
    	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
        pRegDMACfg->src = (u32)&(AdcRecData_G0);
    }
    else if(ADCRecGrp == 2)
    {
    	uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G1_RECORD]|DMA_BURST4;
        pRegDMACfg->src = (u32)&(AdcRecData_G1);
    }
    else if(ADCRecGrp == 4)
    {
    	uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G2_RECORD]|DMA_BURST4;
        pRegDMACfg->src = (u32)&(AdcRecData_G2);
    }
    else if(ADCRecGrp == 8)
    {
    	uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G3_RECORD]|DMA_BURST4;
        pRegDMACfg->src = (u32)&(AdcRecData_G3);
    }
	#elif((CHIP_OPTION == CHIP_A1016A))
    switch(pVideoClipOption->AudioChannelID)
    {
        case 0:
        	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
            pRegDMACfg->src = (u32)&(AdcRecData_G0);
            break;
        case 1:
        	uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G1_RECORD]|DMA_BURST4;
            pRegDMACfg->src = (u32)&(AdcRecData_G1);
            break;
        default:
            DEBUG_IIS("Error: MultiChanneliisSetNextRecDMA() fail, AudioChannelID = %d!!\n", pVideoClipOption->AudioChannelID);
            return;
    }
    #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) ) // A1020DIFF1016
	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pRegDMACfg->src = (u32)&(AdcRecData_G0);
    #else
   	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pRegDMACfg->src = (u32)&(AdcRecData);
    #endif
  #else
   	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pRegDMACfg->src = (u32)&(IisRxData);
  #endif

#if AFN_DEBUG
    AudioTrigger[pVideoClipOption->AudioChannelID]  = 1;
#endif
	pRegDMACfg->dst     = (u32)buf;
	pRegDMACfg->cnt     = (siz / 16);
    pRegDMACfg->cmmd    = uiDMACmmd;

	marsDMAConfig(pVideoClipOption->guiIISRecDMAId, pRegDMACfg);
}
void MultiChanneliisSetNextRecDMA_auto(u8* buf, u32 siz, VIDEO_CLIP_OPTION *pVideoClipOption)
{
    DMA_CFG_AUTO *pdmaCfg_auto;
    u32         ADCRecGrp;
    INT32U      uiDMAId, uiDMACmmd=0;
    REGDMA_CFG_AUTO  RegDMACfg_auto;
    REGDMA_CFG_AUTO  *pRegDMACfg_auto = (REGDMA_CFG_AUTO*)&RegDMACfg_auto;
    //DEBUG_IIS("MultiChanneliisSetNextRecDMA_auto(%d)\n", pVideoClipOption->AudioChannelID);
    isr_marsDMA_RecAuto(pVideoClipOption->guiIISRecDMAId,buf);
    pdmaCfg_auto= &MarsdmaCfg_auto[pVideoClipOption->guiIISRecDMAId];
	/* set read data dma */
  #if(AUDIO_OPTION == AUDIO_ADC_DAC)
    #if((CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B))
    ADCRecGrp = ((AdcCtrlReg & 0x0000003C)>>2); //Lsk : must call adcInit() to set adc record group.
    if(ADCRecGrp == 1)
    {
    	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
        pdmaCfg_auto->src = (u32)&(AdcRecData_G0);
    }
    else if(ADCRecGrp == 2)
    {
    	uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G1_RECORD]|DMA_BURST4;
        pdmaCfg_auto->src = (u32)&(AdcRecData_G1);
    }
    else if(ADCRecGrp == 4)
    {
    	uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G2_RECORD]|DMA_BURST4;
        pdmaCfg_auto->src = (u32)&(AdcRecData_G2);
    }
    else if(ADCRecGrp == 8)
    {
    	uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G3_RECORD]|DMA_BURST4;
        pdmaCfg_auto->src = (u32)&(AdcRecData_G3);
    }
	#elif((CHIP_OPTION == CHIP_A1016A))
    switch(pVideoClipOption->AudioChannelID)
    {
        case 0:
        	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
            pdmaCfg_auto->src = (u32)&(AdcRecData_G0);
            break;
        case 1:
        	uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G1_RECORD]|DMA_BURST4;
            pdmaCfg_auto->src = (u32)&(AdcRecData_G1);
            break;
        default:
            DEBUG_IIS("Error: MultiChanneliisSetNextRecDMA() fail, AudioChannelID = %d!!\n", pVideoClipOption->AudioChannelID);
            return;
    }
    #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pdmaCfg_auto->src = (u32)&(AdcRecData_G0);
    #else
   	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pdmaCfg_auto->src = (u32)&(AdcRecData);
    #endif
  #else
   	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pdmaCfg_auto->src = (u32)&(IisRxData);
  #endif

#if AFN_DEBUG
    AudioTrigger[pVideoClipOption->AudioChannelID]  = 1;
#endif

//	pdmaCfg_auto->dst     = (u32)buf;
//	pdmaCfg_auto->cnt     = (siz / 16);
//    pdmaCfg_auto->cmmd    = uiDMACmmd;
    
    pdmaCfg_auto->dst        = (u32)buf;
    pdmaCfg_auto->src_stride = 0;
    pdmaCfg_auto->dst_stride = IIS_RECORD_SIZE;
    pdmaCfg_auto->datacnt    = siz / 16; // 4 cycle(burst=1)* 4 bytes(word)
    pdmaCfg_auto->linecnt    = 1;
    pdmaCfg_auto->burst      = 1; /*CY 0917*/

//    if(pdmaCfg_auto->burst)
//        uiDMACmmd |= DMA_BURST4;

    uiDMACmmd |= DMA_AUTONONSTOP_EN;

    pRegDMACfg_auto->src     = pdmaCfg_auto->src;
    pRegDMACfg_auto->dst     = pdmaCfg_auto->dst;

    pdmaCfg_auto->src       += pdmaCfg_auto->src_stride;
    pdmaCfg_auto->dst       += pdmaCfg_auto->dst_stride;

    pRegDMACfg_auto->src_alt = pdmaCfg_auto->src;
    pRegDMACfg_auto->dst_alt = pdmaCfg_auto->dst;

    pRegDMACfg_auto->datacnt = pdmaCfg_auto->datacnt;
    pRegDMACfg_auto->linecnt = pdmaCfg_auto->linecnt;
    pRegDMACfg_auto->cmmd    = uiDMACmmd;

    marsDMAConfig_auto(pVideoClipOption->guiIISRecDMAId, pRegDMACfg_auto);
}


/*

Routine Description:

    The multiple channel audio record ISR.

Arguments:

    DMAId - The DMA id.

Return Value:

    None.

*/

void MultiChanneliisRecDMA_ISR(int DMAId)
{
#if (OS_CRITICAL_METHOD == 3)   /* Allocate storage for CPU status register           */
    unsigned int        cpu_sr = 0;   /* Prevent compiler warning                           */
#endif
    VIDEO_CLIP_OPTION   *pVideoClipOption;

    /* To confirm Audio payload playback finish*/
	#if FINE_TIME_STAMP
    s32                 TimeOffset;
	#endif

    pVideoClipOption    = pvcoIISRecDMA[DMAId];

#if AFN_DEBUG
    AudioTrigger[pVideoClipOption->AudioChannelID]  = 0;
#endif

	#if AUDIO_IN_TO_OUT
    //OSSemPost(pVideoClipOption->iisPlaybackSemEvt);
    #endif

	if(pVideoClipOption->guiIISRecDMAId == 0xFF)
    {
    #if AFN_DEBUG
        printf("Error in Ch%d MultiChanneliisRecDMA_ISR(): pVideoClipOption->guiIISRecDMAId = 0xff\n", pVideoClipOption->VideoChannelID);
    #endif
    	pVideoClipOption->gucIISRecDMAStarting = 0;
        return;
    }

    #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
    timerCountRead(2, (u32*) &TimeOffset);
    OS_ENTER_CRITICAL();
    pVideoClipOption->IISTime          += pVideoClipOption->IISTimeUnit;  // millisecond unit
    pVideoClipOption->IISTimeOffset     = TimeOffset >> 8;
    OS_EXIT_CRITICAL();
    #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
	timerCountRead(1, (u32*) &TimeOffset);
	OS_ENTER_CRITICAL();
	pVideoClipOption->IISTime          += pVideoClipOption->IISTimeUnit;  // millisecond unit
	pVideoClipOption->IISTimeOffset     = TimeOffset / 100;
	OS_EXIT_CRITICAL();
    #else
	OS_ENTER_CRITICAL();
	pVideoClipOption->IISTime          += pVideoClipOption->IISTimeUnit;  // millisecond unit
	OS_EXIT_CRITICAL();
    #endif

	pVideoClipOption->iisRecDMACnt++;
	if(pVideoClipOption->iisRecDMACnt % (IIS_CHUNK_SIZE / IIS_RECORD_SIZE)==0)
    {
	    Output_Sem();
	    OSSemPost(pVideoClipOption->iisCmpSemEvt);
	    OSSemPost(pVideoClipOption->iisplayCmpEvt);
    #if NIC_SUPPORT
      #if (HW_BOARD_OPTION == MR8211_ZINWELL)
        #if TUTK_SUPPORT
        #if 0
        if(P2PEnableStreaming[pVideoClipOption->VideoChannelID - 1])
            OSSemPost(P2PAudioCmpSemEvt[pVideoClipOption->VideoChannelID - 1]);
        #else
        if(pVideoClipOption->VideoChannelID == LocalChannelSource)
        {
            if(P2PEnableStreaming[0])
                OSSemPost(P2PAudioCmpSemEvt[0]);
        }
        #endif
        #endif
      #else
        #if TUTK_SUPPORT
        if(P2PEnableStreaming[0])
            OSSemPost(P2PAudioCmpSemEvt[0]);
        #endif

        if(EnableStreaming)
        	OSSemPost(AudioRTPCmpSemEvt[0]);
      #endif
    #endif

    #if RFIU_SUPPORT
         OSSemPost(gRfiuAVCmpSemEvt[0]);
    #endif
    }

    if(pVideoClipOption->gucIISRecDMACurrBufIdx == 15)
        pVideoClipOption->gucIISRecDMACurrBufIdx    = 0;
    else
        pVideoClipOption->gucIISRecDMACurrBufIdx++;

    //DEBUG_IIS("%d", gucIISPlayDMACurrBufIdx);
    if(pVideoClipOption->gucIISRecDMACurrBufIdx == pVideoClipOption->gucIISRecDMANextBufIdx)
    {
        //DEBUG_IIS("Prepare IIS addr for DMA slowly\n");
    #if AFN_DEBUG
        DEBUG_IIS("Error: Ch%d Prepare IIS addr for DMA slowly\n", pVideoClipOption->VideoChannelID);
		#if (AFN_DEBUG_MORE_CHECK>0)

		if(pVideoClipOption->AudioChannelID<MULTI_CHANNEL_LOCAL_MAX)
		{
			MultiIIS_ErrCnt[pVideoClipOption->AudioChannelID]++;
			DEBUG_IIS("MultiIIS_ErrCnt[%d]=%d\n", pVideoClipOption->AudioChannelID, MultiIIS_ErrCnt[pVideoClipOption->AudioChannelID]);		


			#if( HW_BOARD_OPTION == MR6730_AFN )
			if(MultiIIS_ErrCnt[pVideoClipOption->AudioChannelID]>MULCH_IIS_ERRCHK_THRES)
			{
			//remedy			
				
				DEBUG_IIS("terminate recording...\n");
				if((pVideoClipOption->VideoChannelID==VIDEO_CH1 && MACRO_UI_SET_STATUS_BIT_CHK((UI_SET_STATUS_BIT_CH1_ISREC))) \
				|| (pVideoClipOption->VideoChannelID==VIDEO_CH2 && MACRO_UI_SET_STATUS_BIT_CHK((UI_SET_STATUS_BIT_CH2_ISREC)))\
				   )	
				{
					
					MultiChannelAsfCaptureVideoStop(pVideoClipOption);	
				}
			}
			#endif	
		}

		#endif
    #endif
        pVideoClipOption->gucIISRecDMAStarting = 0;
        return;
    }
    if(pVideoClipOption->gucIISRecDMAStarting)
    {
    #if (Audio_mode == AUDIO_AUTO)
//        isr_marsDMA_RecAuto(pVideoClipOption->guiIISRecDMAId,(u8*)pVideoClipOption->gpIISRecDMANextBuf[pVideoClipOption->gucIISRecDMACurrBufIdx]);
        MultiChanneliisSetNextRecDMA_auto((u8*)pVideoClipOption->gpIISRecDMANextBuf[pVideoClipOption->gucIISRecDMACurrBufIdx], IIS_RECORD_SIZE, pVideoClipOption);
        iisStartRec();
    #else
        MultiChanneliisSetNextRecDMA((u8*)pVideoClipOption->gpIISRecDMANextBuf[pVideoClipOption->gucIISRecDMACurrBufIdx], IIS_RECORD_SIZE, pVideoClipOption);
        iisStartRec();
    #endif
    }
}


/*

Routine Description:

    The multiple channel IIS record task.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
void MultiChannelIISRecordTask(void* pData)
{
    u32*    pTime;
    u32*    pSize;
    u8*     pBuf;
    u8      err;
    s32     i, j, interval;
    u32     status;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    VIDEO_CLIP_OPTION   *pVideoClipOption;

    pVideoClipOption                = (VIDEO_CLIP_OPTION*)pData;
    interval                        = 0;
    pBuf                            = pBuf; /* avoid warning */
    pVideoClipOption->IIS_Task_Go   = 1;    // 0: never run, 1: ever run

	#if AFN_DEBUG
	#if (AFN_DEBUG_MORE_CHECK)
	if(pVideoClipOption->AudioChannelID<MULTI_CHANNEL_LOCAL_MAX)
	{
		MultiIIS_ErrCnt[pVideoClipOption->AudioChannelID]=0;
		DEBUG_IIS("reset MultiIIS_ErrCnt[%d]=%d\n", pVideoClipOption->AudioChannelID, MultiIIS_ErrCnt[pVideoClipOption->AudioChannelID]);
	}
	#endif
	#endif
	
    DEBUG_IIS("MultiChannelIISRecordTask(%d)\n", pVideoClipOption->VideoChannelID);

    while (1)
    {
    	pVideoClipOption->IIS_Task_Pend = 1;
        OSSemPend(pVideoClipOption->iisTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
        pVideoClipOption->IIS_Task_Pend = 0;

        if (err != OS_NO_ERR)
        {
            DEBUG_IIS("Error: iisTrgSemEvt is %d.\n", err);
        }

        pTime = &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngWriteIdx].time;
        pSize = &pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngWriteIdx].size;
        pBuf  = pVideoClipOption->iisSounBufMng[pVideoClipOption->iisSounBufMngWriteIdx].buffer;

        *pTime = IIS_CHUNK_TIME;
#if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MP4)
        *pSize = IIS_CHUNK_SIZE;
        *pTime  = (*pSize * 1000) / iisRecFormat.nAvgBytesPerSec;   //  sound length in millisecond unit
#elif (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_ASF)
        *pSize = IIS_CHUNK_SIZE;
        *pTime  = (*pSize * 1000) / iisRecFormat.nAvgBytesPerSec;   //  sound length in millisecond unit
#elif (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_AVI)
        *pSize  = IIS_CHUNK_SIZE;
        *pTime  = (*pSize * 1000) / iisRecFormat.nAvgBytesPerSec;   //  sound length in millisecond unit
#elif (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MOV)
        *pSize = IIS_CHUNK_SIZE;
	    *pTime  = (*pSize * 1000) / iisRecFormat.nAvgBytesPerSec;   //  sound length in millisecond unit
#endif

		if(pVideoClipOption->guiIISRecDMAId == 0xFF)
		{
		    marsDMAOpen(&pVideoClipOption->guiIISRecDMAId, MultiChanneliisRecDMA_ISR);
            pvcoIISRecDMA[pVideoClipOption->guiIISRecDMAId] = pVideoClipOption;
		    DEBUG_IIS("Ch%d guiIISRecDMAId = %d \r\n", pVideoClipOption->VideoChannelID, pVideoClipOption->guiIISRecDMAId);
		    pVideoClipOption->gucIISRecDMACurrBufIdx=0;
		    pVideoClipOption->gucIISRecDMANextBufIdx=0;
		}

        j   = *pSize / IIS_RECORD_SIZE;
        for(i = 0; i < j; i++)
        {
			pVideoClipOption->gpIISRecDMANextBuf[pVideoClipOption->gucIISRecDMANextBufIdx] = pBuf;
			pBuf   += IIS_RECORD_SIZE;
			/**********************************************
			*** 1. read more data in, and save pufIdx.  ***
			*** 2. Use DMA move to IIS in ISR function. ***
			*** 3. Sync ISR and read data.              ***
			**********************************************/
			if(pVideoClipOption->gucIISRecDMANextBufIdx == 15)
				pVideoClipOption->gucIISRecDMANextBufIdx=0;
			else
				pVideoClipOption->gucIISRecDMANextBufIdx++;

			if(pVideoClipOption->gucIISRecDMAStarting == 0)
            {
                pVideoClipOption->gucIISRecDMAStarting = 1;
            	//DEBUG_IIS("1.gucIISRecDMAStarting = %d \r\n", gucIISRecDMAStarting);
            #if (Audio_mode == AUDIO_AUTO)
                MultiChanneliisSetNextRecDMA_auto((u8*)pVideoClipOption->gpIISRecDMANextBuf[pVideoClipOption->gucIISRecDMACurrBufIdx], IIS_RECORD_SIZE, pVideoClipOption);
            #else
                MultiChanneliisSetNextRecDMA((u8*)pVideoClipOption->gpIISRecDMANextBuf[pVideoClipOption->gucIISRecDMACurrBufIdx], IIS_RECORD_SIZE, pVideoClipOption);
            #endif
                iisStartRec();
            }

            if(pVideoClipOption->gucIISRecDMACurrBufIdx == pVideoClipOption->gucIISRecDMANextBufIdx)
            {
                while((pVideoClipOption->gucIISRecDMACurrBufIdx == pVideoClipOption->gucIISRecDMANextBufIdx)&&(pVideoClipOption->gucIISRecDMAStarting)&&(!pVideoClipOption->IIS_Task_Stop))
                {
                	//DEBUG_IIS("#");
                	OSTimeDly(1);
                }
            }

        }
        if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL))
        {
            if((pVideoClipOption->WantChangeFile == 1) && (pVideoClipOption->GetLastAudio == 0))
            {
                OS_ENTER_CRITICAL();
                pVideoClipOption->LastAudio         = pVideoClipOption->iisSounBufMngWriteIdx;
                pVideoClipOption->GetLastAudio      = 1;
                OS_EXIT_CRITICAL();
            }
        }
        OS_ENTER_CRITICAL();
        pVideoClipOption->CurrentAudioSize     += *pSize;
        OS_EXIT_CRITICAL();

        pVideoClipOption->iisSounBufMngWriteIdx = (pVideoClipOption->iisSounBufMngWriteIdx + 1) %  IIS_BUF_NUM;
    }
}

/*

Routine Description:

    Stop record.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelIIsStopRec(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    volatile INT16U *pIISUseSemCnt = &pVideoClipOption->gIISRecUseSem->OSEventCnt;

    while(pVideoClipOption->iisTrgSemEvt->OSEventCnt)
    {
        OSSemAccept(pVideoClipOption->iisTrgSemEvt);
    }

    while(1)
    {
        if(*pIISUseSemCnt != 0)
            break;
		OSTimeDly(1);
    }
	OSTimeDly(3);
	if(pVideoClipOption->guiIISRecDMAId != 0xFF)
    {
        marsDMAClose(pVideoClipOption->guiIISRecDMAId);
        pVideoClipOption->guiIISRecDMAId = 0xFF;

    }
	pVideoClipOption->gucIISRecDMAStarting = 0;

    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) ||  (AUDIO_OPTION == AUDIO_IIS_ALC5621))
        IisCtrl &= ~IIS_RCV_ENA;
    #elif (AUDIO_OPTION == AUDIO_ADC_DAC)
        #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
            (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
            (CHIP_OPTION == CHIP_A1026A))
           #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
            AdcCtrlReg &= (~ADC_REC_G1);
            //AdcCtrlReg &= (~ADC_REC_CONV_ENA);
           #else
            switch(pVideoClipOption->AudioChannelID)
            {
                case 0:
                AdcCtrlReg &= (~ADC_REC_G0);
                //AdcCtrlReg &= (~ADC_REC_CONV_ENA);
                break;
                case 1:
                AdcCtrlReg &= (~ADC_REC_G1);
                //AdcCtrlReg &= (~ADC_REC_CONV_ENA);
                break;
            }
           #endif
        #else
        AdcCtrlReg &= ~ADC_RX_ENA;
        #endif
    #elif (AUDIO_OPTION == AUDIO_AC97_ALC203)
        Ac97ICC = 0x00000000;
        Ac97Ctrl &= (~AC97_RCV_ENA);
    #endif

    return 1;
}





#endif  // #if MULTI_CHANNEL_VIDEO_REC

