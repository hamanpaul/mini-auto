/*

Copyright (c) 2012 Mars Semiconductor Corp.

Module Name:

    MultiChannelMPEG4.c

Abstract:

    The routines of multiple channel MPEG-4 encoder/decoder.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2012/05/08  Peter Hsu  Create  

*/

#include "general.h"

#if MULTI_CHANNEL_VIDEO_REC

#include "board.h"
#include "task.h"
#include "mp4api.h"
#include "asfapi.h"
#include "aviapi.h"    /* Peter 0704 */
#include "mpeg4api.h"
#include "mpeg4.h"
#include "mpeg4reg.h"
#include "isuapi.h"    /*Peter 1109 S*/
#include "gpioapi.h"
#include "siuapi.h"    /* Peter 070403 */
#include "sysapi.h"
#include "osapi.h"
#include "rtcapi.h"
#include "Mp4RateControl.h"
#include "i2capi.h"
#include "sysapi.h"
#include "iduapi.h"
#include "../idu/inc/idureg.h"
#include "uiapi.h"
#include "intapi.h"
#include "ciuapi.h"
#include "GlobalVariable.h"

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
OS_STK      mpeg4TaskStack0[MPEG4_TASK_STACK_SIZE]; /* Stack of task MultiChannelMPEG4EncoderTask() */
OS_STK      mpeg4TaskStack1[MPEG4_TASK_STACK_SIZE]; /* Stack of task MultiChannelMPEG4EncoderTask() */
OS_STK      mpeg4TaskStack2[MPEG4_TASK_STACK_SIZE]; /* Stack of task MultiChannelMPEG4EncoderTask() */
OS_STK      mpeg4TaskStack3[MPEG4_TASK_STACK_SIZE]; /* Stack of task MultiChannelMPEG4EncoderTask() */

u8          MPEG4_Curr_Channel_ID;


/*
 *********************************************************************************************************
 * Extern Varaibel
 *********************************************************************************************************
 */
extern OS_EVENT    *mpeg4ReadySemEvt;

extern u32 sysVideoInSel;
extern u32 EventTrigger;  //用於Buffer moniting.
extern u32 asfVopCount;   //用於Buffer moniting.

extern s32 mp4_avifrmcnt, isu_avifrmcnt;
extern u32 IsuIndex;

extern u8* mpeg4outputbuf[3];
extern u8  sysCaptureVideoStop;
extern u32 unMp4FrameDuration[SIU_FRAME_DURATION_NUM]; /* mh@2006/11/22: for time stamp control */ /* Peter 070403 */
extern u32 isu_int_status;

extern u32 VideoPictureIndex;
extern u8  TVout_Generate_Pause_Frame;

#if NIC_SUPPORT
extern u8 EnableStreaming;
extern u8  LocalChannelSource; // ch0 source
#endif
#if TUTK_SUPPORT
extern s8 P2PEnableStreaming[];
#endif

extern u32 FiqError;


/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

#if(VIDEO_CODEC_OPTION ==  MPEG4_CODEC)

/*

Routine Description:

    Initialize Multiple channel MPEG-4 encoder task.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelMPEG4EncoderTaskCreate(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    int i;

    /* initialize video buffer */
    for(i = 0; i < VIDEO_BUF_NUM; i++) {        
        pVideoClipOption->VideoBufMng[i].buffer = pVideoClipOption->VideoBuf;
    }

    // initialize rate control parameter
    memset(&pVideoClipOption->mpRateControlParam, 0, sizeof(DEF_RATECONTROL_PARA));
    RCQ2_init(&pVideoClipOption->mpRateControlParam);
    
    /* Create the semaphore */
    pVideoClipOption->VideoTrgSemEvt    = OSSemCreate(VIDEO_BUF_NUM - 2); /* guarded for ping-pong buffer */
    pVideoClipOption->VideoCmpSemEvt    = OSSemCreate(0);
   	pVideoClipOption->VideoRTPCmpSemEvt = OSSemCreate(0);
    pVideoClipOption->mpeg4Width        = pVideoClipOption->asfVopWidth;
    pVideoClipOption->mpeg4Height       = pVideoClipOption->asfVopHeight;
#if DUAL_MODE_DISP_SUPPORT
    pVideoClipOption->mpeg4Stride       = pVideoClipOption->mpeg4Width * 2;
    pVideoClipOption->pnbuf_size_y      = PNBUF_SIZE_Y * 2;
#else
    pVideoClipOption->mpeg4Stride       = pVideoClipOption->mpeg4Width;
    pVideoClipOption->pnbuf_size_y      = PNBUF_SIZE_Y;
#endif
    //pVideoClipOption->VideoCpleSemEvt   = OSSemCreate(0); /*BJ 0530 S*/
    MultiChannelMpeg4ConfigQualityFrameRate(MPEG_BITRATE_LEVEL_100, pVideoClipOption);
    
    /* Create the task */
    DEBUG_MP4("Trace: MultiChannelMPEG4EncoderTaskCreate(%d) task creating...\n", pVideoClipOption->VideoChannelID);
    switch(pVideoClipOption->VideoChannelID)
    {
        case 0:
            OSTaskCreate(MULTI_CH_MPEG4_ENC_TASK, pVideoClipOption, MULTI_CH_MPEG4_TASK_STACK0, MPEG4_TASK_PRIORITY_UNIT0); 
            break;
        case 1:
        //#if DUAL_MODE_DISP_SUPPORT
            pVideoClipOption->pnbuf_size_y      = ciu_1_pnbuf_size_y;
        //#endif
            OSTaskCreate(MULTI_CH_MPEG4_ENC_TASK, pVideoClipOption, MULTI_CH_MPEG4_TASK_STACK1, MPEG4_TASK_PRIORITY_UNIT1); 
            break;
        case 2:
        //#if DUAL_MODE_DISP_SUPPORT
            pVideoClipOption->pnbuf_size_y      = ciu_2_pnbuf_size_y;
        //#endif
            OSTaskCreate(MULTI_CH_MPEG4_ENC_TASK, pVideoClipOption, MULTI_CH_MPEG4_TASK_STACK2, MPEG4_TASK_PRIORITY_UNIT2); 
            break;
        case 3:
            OSTaskCreate(MULTI_CH_MPEG4_ENC_TASK, pVideoClipOption, MULTI_CH_MPEG4_TASK_STACK3, MPEG4_TASK_PRIORITY_UNIT3); 
            break;
        default:
            DEBUG_MP4("Error: MultiChannelMPEG4EncoderTaskCreate() don't support pVideoClipOption->VideoChannelID = %d\n", pVideoClipOption->VideoChannelID);
            return 0;
    }
        
    return 1;   
}

/*

Routine Description:

    Destroy Multiple channel MPEG-4 encoder task.

Arguments:

    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelMPEG4EncoderTaskDestroy(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    int     i;
    INT8U   err;

    DEBUG_MP4("MultiChannelMPEG4EncoderTaskDestroy(%d)\n", pVideoClipOption->VideoChannelID);
    /* Delete the task */
    switch(pVideoClipOption->VideoChannelID)
    {
        case 0:
            if(OS_NO_ERR !=  OSTaskSuspend(MPEG4_TASK_PRIORITY_UNIT0))
                DEBUG_MP4("OSTaskSuspend(MPEG4_TASK_PRIORITY_UNIT0) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(MPEG4_TASK_PRIORITY_UNIT0))
                DEBUG_MP4("OSTaskDel(MPEG4_TASK_PRIORITY_UNIT0) error!!\n");
            break;
        case 1:
            if(OS_NO_ERR !=  OSTaskSuspend(MPEG4_TASK_PRIORITY_UNIT1))
                DEBUG_MP4("OSTaskSuspend(MPEG4_TASK_PRIORITY_UNIT1) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(MPEG4_TASK_PRIORITY_UNIT1))
                DEBUG_MP4("OSTaskDel(MPEG4_TASK_PRIORITY_UNIT1) error!!\n");
            break;
        case 2:
            if(OS_NO_ERR !=  OSTaskSuspend(MPEG4_TASK_PRIORITY_UNIT2))
                DEBUG_MP4("OSTaskSuspend(MPEG4_TASK_PRIORITY_UNIT2) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(MPEG4_TASK_PRIORITY_UNIT2))
                DEBUG_MP4("OSTaskDel(MPEG4_TASK_PRIORITY_UNIT2) error!!\n");
            break;
        case 3:
            if(OS_NO_ERR !=  OSTaskSuspend(MPEG4_TASK_PRIORITY_UNIT3))
                DEBUG_MP4("OSTaskSuspend(MPEG4_TASK_PRIORITY_UNIT3) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(MPEG4_TASK_PRIORITY_UNIT3))
                DEBUG_MP4("OSTaskDel(MPEG4_TASK_PRIORITY_UNIT3) error!!\n");
            break;
        default:
            DEBUG_MP4("Error: MultiChannelMPEG4EncoderTaskDestroy() don't support pVideoClipOption->VideoChannelID = %d", pVideoClipOption->VideoChannelID);
    }
        
    /* Delete the semaphore */
    pVideoClipOption->VideoTrgSemEvt    = OSSemDel(pVideoClipOption->VideoTrgSemEvt, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->VideoTrgSemEvt)
        DEBUG_MP4("OSSemDel(pVideoClipOption->VideoTrgSemEvt) error!!\n");

    pVideoClipOption->VideoCmpSemEvt    = OSSemDel(pVideoClipOption->VideoCmpSemEvt, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->VideoCmpSemEvt)
        DEBUG_MP4("OSSemDel(pVideoClipOption->VideoCmpSemEvt) error!!\n");

   	pVideoClipOption->VideoRTPCmpSemEvt = OSSemDel(pVideoClipOption->VideoRTPCmpSemEvt, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->VideoRTPCmpSemEvt)
        DEBUG_MP4("OSSemDel(pVideoClipOption->VideoRTPCmpSemEvt) error!!\n");
    
    pVideoClipOption->MPEG4_Task_Go = 0;    // 0: never run, 1: ever run

    return 1;   
}

/*

Routine Description:

    The FIQ handler of MultiChannel MPEG-4 encoder/decoder.

Arguments:

    None.

Return Value:

    None.

*/
void MultiChannelMPEG4IntHandler(void)
{
#if(VIDEO_CODEC_OPTION == MPEG4_CODEC)
    u32 intStat         = Mpeg4IntStat;
    u32 temp;
    VIDEO_CLIP_OPTION   *pVideoClipOption;

    pVideoClipOption    = &VideoClipOption[MPEG4_Curr_Channel_ID];

    MPEG4_Status                    = intStat;
    pVideoClipOption->MPEG4_Status  = intStat;

    if (intStat & 0x00000001)   // encoder finish   
    {
        if (pVideoClipOption->mpegflag == 1)
        {
            OSSemPost(VideoCpleSemEvt);
            pVideoClipOption->mpegflag = 0;
        }
        pVideoClipOption->mp4_avifrmcnt++;
    } 
    else if(intStat & 0x00000006) 
    {   // decoder finish
        // for playback
    #if 0
        pVideoClipOption->mpegflag = 0;
        pVideoClipOption->VideoPictureIndex++;
        pVideoClipOption->VideoBufMngReadIdx  = (pVideoClipOption->VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
    #else
        VideoPictureIndex++;
        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
    #endif
        OSSemPost(VideoCpleSemEvt);
    } 
    else 
    {
        pVideoClipOption->mpegflag = 0;
        DEBUG_MP4("Ch%d MultiChannelMPEG4IntHandler(0x%08X,0x%08X,0x%08X,0x%08X) error!!!\n", MPEG4_Curr_Channel_ID, MPEG4_Status, intStat, IntFiqInput, IntIrqInput);
        //if((IntFiqInput & INT_FIQMASK_MPEG4) == 0)
        //    DEBUG_MP4("IntFiqInput(0x%08X) & INT_FIQMASK_MPEG4(0x%08X) == 0, something wrong!!!\n", IntFiqInput, INT_FIQMASK_MPEG4);
        FiqError    = 1;
    }
#endif
}


/*

Routine Description:

    Set multi channel mpeg4 picture coding type.

Arguments:

    frame_idx - the frame idex in a coding sequence.
    Period  - the priod for coding an I frame.

Return Value:

    0 - P frame.
    1 - I frame.

*/
#if 0
u32 MultiChannelDeterminePictureType(u32 frame_idx, u32 period)
{
    /* I picture  */
    if((period == 0) || ((frame_idx % period) == 0)) {        
        return FLAG_I_VOP;
    } else { /* P picture */
        return 0;
    }
}
#else
u32 MultiChannelDeterminePictureType(u32 frame_idx, u32 period, VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u32 diff;
    
    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif
    //------//
    
    #if(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_ASF)                               
    //calulate time by video time, to get exactly asf file time
    if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && pVideoClipOption->SetIVOP)    
    {
        DEBUG_MP4("Ch%d MPEG4 calulate time, start index = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngWriteIdx);        
        OS_ENTER_CRITICAL();
        pVideoClipOption->Cal_FileTime_Start_Idx    = pVideoClipOption->VideoBufMngWriteIdx; 
        pVideoClipOption->VideoTimeStatistics       = 0;
        pVideoClipOption->SetIVOP                   = 0;
        OS_EXIT_CRITICAL();
        pVideoClipOption->asfVideoFrameCount       += period;
        
        return FLAG_I_VOP;
    }        
    #elif(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_AVI) 
    if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) 
    {
        if((pVideoClipOption->WantChangeFile == 1) && (pVideoClipOption->GetLastVideo == 1) && (pVideoClipOption->VideoBufMngWriteIdx == ((pVideoClipOption->LastVideo + 1) % VIDEO_BUF_NUM))) 
        {
            pVideoClipOption->asfVideoFrameCount += period;
            return FLAG_I_VOP;
        }
    }
    #endif
    
    if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && pVideoClipOption->SetIVOP)
    {
        DEBUG_MP4("Ch%d MPEG4 calulate time, start index = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngWriteIdx);
        
        OS_ENTER_CRITICAL();
        pVideoClipOption->Cal_FileTime_Start_Idx    = pVideoClipOption->VideoBufMngWriteIdx; 
        pVideoClipOption->VideoTimeStatistics       = 0;
        pVideoClipOption->SetIVOP                   = 0;
        OS_EXIT_CRITICAL();
        pVideoClipOption->asfVideoFrameCount       += period;
        return FLAG_I_VOP;
    }
    
    /* I picture  */
    diff    = (pVideoClipOption->asfVideoFrameCount >= frame_idx) ? (pVideoClipOption->asfVideoFrameCount - frame_idx) : (frame_idx - pVideoClipOption->asfVideoFrameCount) ;
    if(diff > 1000)
    {
        DEBUG_MP4("Ch%d Determine I frame overflow!\n", pVideoClipOption->VideoChannelID);
        return FLAG_P_VOP;
    }

    if(period == 0)
    {
        pVideoClipOption->asfVideoFrameCount = frame_idx;
        return FLAG_I_VOP;
    }
    else if( frame_idx >= pVideoClipOption->asfVideoFrameCount ) 
    {   
        pVideoClipOption->asfVideoFrameCount += period;
        if(period >= pVideoClipOption->asfVideoFrameCount)
        {
             DEBUG_MP4("Ch%d FrameCount overflow!\n", pVideoClipOption->VideoChannelID);
        }
            
        return FLAG_I_VOP;
    }    
    else 
    { /* P picture */
        return FLAG_P_VOP;
    }
}
#endif

u32 MultiChannelMPEG4PutVOPHeader(u8 *pBuf, u32 pFlag, s64 *pTime, u32 *byteno,u32 FrameIdx, VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u8 bitno;
    u32 temp;
    u8  bitsize;
    u32 bitpos;
#if 0   
    u8 mpeg4VOPHeader[0x20] = 
    { 
        0x00, 0x00, 0x01, 0xB6,
        0x00, 0x00, 0x01, 0x20,
        0x00, 0xc8, 0x88, 0x80,
        0x0f, 0x50, 0xb0, 0x42,
        0x41, 0x41, 0x83,
    };
#endif      
    // This program will be wroten by a fix table
    bitno = 0;
    // VOP start code
    *byteno += mpeg4PutHeader(pBuf + *byteno, &bitno, 32, 0x000001b6);
        
    // Picture coding type
    //*pFlag: I(00),P(01),B(10),S(11)
    *byteno += mpeg4PutHeader(pBuf + *byteno, &bitno, 2, (u32) pFlag);
    // Modulo time base
    pVideoClipOption->mpeg4VopTimeInc    += (*pTime) * (VOP_TIME_INCREMENT_RESOLUTION/1000);
    while(pVideoClipOption->mpeg4VopTimeInc >= VOP_TIME_INCREMENT_RESOLUTION)
    {
        *byteno += mpeg4PutHeader(pBuf + *byteno, &bitno, 1, 1);
        pVideoClipOption->mpeg4VopTimeInc -= VOP_TIME_INCREMENT_RESOLUTION;
    }
    *byteno += mpeg4PutHeader(pBuf + *byteno, &bitno, 1, 0); //???
    //marker bit
    *byteno += mpeg4PutHeader(pBuf + *byteno, &bitno, 1, 1);  
    // vop time increment
    
    temp = VOP_TIME_INCREMENT_RESOLUTION;
        bitsize = 0;
    do
    {
        temp >>= 1;
        bitsize++;
    } while(temp > 0);
    
    
    *byteno            += mpeg4PutHeader(pBuf + *byteno, &bitno, bitsize, pVideoClipOption->mpeg4VopTimeInc);  //Lucian: 目前影音同步不會參考此一參數. 會用File format內的機制.
    // marker bit
    *byteno            += mpeg4PutHeader(pBuf + *byteno, &bitno, 1, 1);

    //vop_coded
    *byteno += mpeg4PutHeader(pBuf + *byteno, &bitno, 1, 1);
    
    //vop_rounding_type:: Only P frame
    if(pFlag == P_VOP)
    *byteno += mpeg4PutHeader(pBuf + *byteno, &bitno, 1, FrameIdx & 0x01);

    //intra_dc_vlc_thr//
    *byteno += mpeg4PutHeader(pBuf + *byteno, &bitno, 3, 0);    

    //vop_quant (5 bits)
    *byteno += mpeg4PutHeader(pBuf + *byteno, &bitno, 5, (u32) pVideoClipOption->dftMpeg4Quality);
        
    if (pFlag != I_VOP)
    {
        //search range always is +/- 16.
        *byteno += mpeg4PutHeader(pBuf + *byteno, &bitno, 3, 1);
    }       
    bitpos = ((*byteno & 0x03) << 3) + bitno;
    if (bitno != 0)
    {
        *(pBuf + *byteno) <<= (8 - bitno);  
    }
    *byteno &= 0xfc;

    return bitpos;
}

s32 MultiChannelMPEG4Coding1Frame(u8* FrameBuf, u32 FrameType, s64* FrameTime, u32* CmpSize, u32 FrameIdx, u32 mpeg4StartBits, VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u8  err;
    u32 mpeg4RTYPE, mpeg4Quality;
    u32 temp,i;
    u8  mpeg4RefSliceSize, mpeg4IMbRefXpos, mpeg4IMbRefYpos; 
    u32 mbWidth = (pVideoClipOption->mpeg4Width + 15) >> 4; /*CY 0907*/
    u32 mbNoSize, mbNo;       /*CY 0907*/
    u32 mbHeight;          /*CY 0907*/
    u32 mpeg4VdPacketSize;
    volatile INT16U *pCurISUSemCnt = &isuSemEvt->OSEventCnt;

    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    MPEG4_Curr_Channel_ID   = pVideoClipOption->VideoChannelID;

#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif
    
    // 0x0104
    mpeg4RTYPE      = FrameIdx & 0x01;
    mpeg4Quality    = (u32) pVideoClipOption->dftMpeg4Quality;
    Mpeg4MbParam    = (mpeg4Quality & 0x1f) | (mpeg4RTYPE << 8) | (FrameType << 16);

    //mpeg4SetVideoResolution(mpeg4Width, mpeg4Height);  
    //0x0100
    mbWidth             = (u32) ((pVideoClipOption->mpeg4Width + 15) >> 4);
    mbHeight            = (u32) ((pVideoClipOption->mpeg4Height + 15) >> 4);
    mbNo                = mbWidth * mbHeight;
    Mpeg4FrameSize      = (mbWidth << mbWidthShft) | 
                          (mbHeight << mbHeightShft) |
                          (mbNo << mbNoShft);
    Mpeg4SourceStride   = pVideoClipOption->mpeg4Stride;

    // 0x0200
    mpeg4VdPacketSize   = MPEG4_VDPACKET_SIZE;
    Mpeg4ErrResil       = mpeg4VdPacketSize | MPEG4_RESY_ENA;

    // 0x0204
    mbNoSize            = 0;
    temp                = mbNo;
    while(temp > 0)
    {
        temp  >>= 1;
        mbNoSize++;
    }
    Mpeg4DecVidPkt      = mbNoSize;
    //mpeg4SetVideoResolution(mpeg4Width, mpeg4Height);  
    // 0x0200 ~ 0x0204: move to set resolution
    /* 
    // 0x0200
    mpeg4VdPacketSize = MPEG4_VDPACKET_SIZE;
    Mpeg4ErrResil   = mpeg4VdPacketSize | MPEG4_RESY_DIS;
    //Mpeg4ErrResil   = mpeg4VdPacketSize |MPEG4_RESY_ENA;
    // 0x0204
    mpeg4MbNoSize   = 0;
    mpeg4MbNo1      = mpeg4MbNo;
    while(mpeg4MbNo1 > 0)
    {
        mpeg4MbNo1>>=1;
        mpeg4MbNoSize++;
    }
    Mpeg4DecVidPkt = mpeg4MbNoSize;
    */
    // 0x0300
    Mpeg4MeThresh1 = (MPEG4_INTRAINTER_THD & 0x03ff) | (MPEG4_ONEFOURMV_THD << 16);
    // 0x0304
    Mpeg4MeThresh2 = (MPEG4_BIASSAD_8 & 0x03ff) | (MPEG4_BIASSAD_16 << 16);
    // 0x0308 -- read only
    //Mpeg4SadSum
    // 0x030C
    if (FrameType == I_VOP)
        pVideoClipOption->mpeg4MBRef  = -1;
    else
        pVideoClipOption->mpeg4MBRef++;
    
    mpeg4RefSliceSize   = 3;
    mpeg4IMbRefXpos     = pVideoClipOption->mpeg4MBRef % mbWidth;
    mpeg4IMbRefYpos     = (pVideoClipOption->mpeg4MBRef / mbWidth) & mpeg4SliceMask[mpeg4RefSliceSize];
    Mpeg4IntraMbRefresh = (mpeg4IMbRefXpos) | (mpeg4IMbRefYpos << mpeg4IMbRefYpos_Shft) | (mpeg4RefSliceSize << mpeg4RefSliceSize_Shft) | (MPEG4_IMBREF_DIS);
    // 0x0400
    Mpeg4IntEna         = 0x01;
    // 0x0404
    //Mpeg4IntStat
    // 0x0500
    // bitstreaming start address
    Mpeg4StreamAddr     = (u32) FrameBuf;
    // 0x0504
    Mpeg4StreamStartBit = mpeg4StartBits; 
    *CmpSize           += (mpeg4StartBits>>3);
    // 0x0508
    temp                = (*FrameBuf << 24) + (*(FrameBuf + 1) << 16) + (*(FrameBuf + 2) << 8) + *(FrameBuf + 3);
    if (mpeg4StartBits == 0)
        Mpeg4StreamStartWord = 0;
    else
        Mpeg4StreamStartWord = temp & ~(PutBitsMask[31 - mpeg4StartBits]);

    //0x0510
    Mpeg4VopParam       = 0x01;
    
    //0x0600
    // pre-set all MV to 0xff
    memset( pVideoClipOption->mpeg4MVBuf, 0xff, MPEG4_MVBUF); 
    Mpeg4MvBufAddr = (u32) &(pVideoClipOption->mpeg4MVBuf[0]);
    //0x0604  // 0x0608
    // set input buffer
    //PNBuf_Y0, PNBuf_C0, PNBuf_Y1, PNBuf_C1, PNBuf_Y2, PNBuf_C2;

    switch (pVideoClipOption->VideoChannelID)
    {
        case 0:
            Mpeg4CurrRawYAddr   = (u32) PNBuf_Y[pVideoClipOption->VideoPictureIndex % 4];
            Mpeg4CurrRawCbAddr  = (u32) PNBuf_C[pVideoClipOption->VideoPictureIndex % 4];
            break;
        case 1:
            Mpeg4CurrRawYAddr   = (u32) PNBuf_sub1[pVideoClipOption->VideoPictureIndex % 4];
            Mpeg4CurrRawCbAddr  = (u32) (PNBuf_sub1[pVideoClipOption->VideoPictureIndex % 4] + pVideoClipOption->pnbuf_size_y);
            break;
        case 2:
            Mpeg4CurrRawYAddr   = (u32) PNBuf_sub2[pVideoClipOption->VideoPictureIndex % 4];
            Mpeg4CurrRawCbAddr  = (u32) (PNBuf_sub2[pVideoClipOption->VideoPictureIndex % 4] + pVideoClipOption->pnbuf_size_y);
            break;
        default:
            DEBUG_MP4("Encoder Error: pVideoClipOption->VideoChannelID is %d.\n", pVideoClipOption->VideoChannelID);
    }

    if (FrameIdx & 0x01)
    {
        pVideoClipOption->mpeg4RefBuf_Y   = pVideoClipOption->mpeg4NRefBuf_Y;
        pVideoClipOption->mpeg4RefBuf_Cb  = pVideoClipOption->mpeg4NRefBuf_Cb;
        pVideoClipOption->mpeg4RefBuf_Cr  = pVideoClipOption->mpeg4NRefBuf_Cr;
        pVideoClipOption->mpeg4McoBuf_Y   = pVideoClipOption->mpeg4PRefBuf_Y;
        pVideoClipOption->mpeg4McoBuf_Cb  = pVideoClipOption->mpeg4PRefBuf_Cb;
        pVideoClipOption->mpeg4McoBuf_Cr  = pVideoClipOption->mpeg4PRefBuf_Cr;
    }
    else
    {
        pVideoClipOption->mpeg4RefBuf_Y   = pVideoClipOption->mpeg4PRefBuf_Y;
        pVideoClipOption->mpeg4RefBuf_Cb  = pVideoClipOption->mpeg4PRefBuf_Cb;
        pVideoClipOption->mpeg4RefBuf_Cr  = pVideoClipOption->mpeg4PRefBuf_Cr;
        pVideoClipOption->mpeg4McoBuf_Y   = pVideoClipOption->mpeg4NRefBuf_Y;
        pVideoClipOption->mpeg4McoBuf_Cb  = pVideoClipOption->mpeg4NRefBuf_Cb;
        pVideoClipOption->mpeg4McoBuf_Cr  = pVideoClipOption->mpeg4NRefBuf_Cr;
    } 

    // 0x0610
    /* Peter 070108 S */
    Mpeg4CurrRecInYAddr     = (u32) &(pVideoClipOption->mpeg4McoBuf_Y[(pVideoClipOption->mpeg4Width + 32) * 16 + 16]);  
    // 0x0614
    Mpeg4CurrRecInCbAddr    = (u32) &(pVideoClipOption->mpeg4McoBuf_Cb[((pVideoClipOption->mpeg4Width >> 1) + 16) * 8 + 8]);
    // 0x0618
    Mpeg4CurrRecInCrAddr    = (u32) &(pVideoClipOption->mpeg4McoBuf_Cr[((pVideoClipOption->mpeg4Width >> 1) + 16) * 8 + 8]);
    // 0x061C
    Mpeg4CurrRecOutYAddr    = (u32) &(pVideoClipOption->mpeg4McoBuf_Y[0]);
    // 0x0620
    Mpeg4CurrRecOutCbAddr   = (u32) &(pVideoClipOption->mpeg4McoBuf_Cb[0]);
    // 0x0624
    Mpeg4CurrRecOutCrAddr   = (u32) &(pVideoClipOption->mpeg4McoBuf_Cr[0]);
    // 0x0628
    Mpeg4PrevRecInYAddr     = (u32) &(pVideoClipOption->mpeg4RefBuf_Y[(pVideoClipOption->mpeg4Width + 32) * 16 + 16]);
    // 0x062C
    Mpeg4PrevRecInCbAddr    = (u32) &(pVideoClipOption->mpeg4RefBuf_Cb[((pVideoClipOption->mpeg4Width >> 1) + 16) * 8 + 8]);
    // 0x0630
    Mpeg4PrevRecInCrAddr    = (u32) &(pVideoClipOption->mpeg4RefBuf_Cr[((pVideoClipOption->mpeg4Width >> 1) + 16) * 8 + 8]);
    // 0x0634
    Mpeg4PrevRecOutYAddr    = (u32) &(pVideoClipOption->mpeg4RefBuf_Y[0]);
    // 0x0638
    Mpeg4PrevRecOutCbAddr   = (u32) &(pVideoClipOption->mpeg4RefBuf_Cb[0]);
    // 0x063C
    Mpeg4PrevRecOutCrAddr   = (u32) &(pVideoClipOption->mpeg4RefBuf_Cr[0]);  
    /* Peter 070108 E */

    pVideoClipOption->mpegflag    = 1;

#if (CIU1_BOB_REPLACE_MPEG_DF  && ( (CIU1_OPTION == Sensor_CCIR656) || (CIU1_OPTION == Sensor_CCIR601) ))
   #if USE_MPEG_QUANTIZATION
    Mpeg4Ctrl   = MPEG4_QM_MPEG | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
   #else
    Mpeg4Ctrl   = MPEG4_QM_H263 | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
   #endif
#else
   #if USE_MPEG_QUANTIZATION
    //if((pVideoClipOption->video_double_field_flag) || (sysPIPMain == PIP_MAIN_NONE))
    if(pVideoClipOption->video_double_field_flag)
        Mpeg4Ctrl   = MPEG4_QM_MPEG | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_DB_FIELD | MPEG4_ENC_TRG;
    else
        Mpeg4Ctrl   = MPEG4_QM_MPEG | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
   #else
    //if((pVideoClipOption->video_double_field_flag) || (sysPIPMain == PIP_MAIN_NONE))
    if(pVideoClipOption->video_double_field_flag)
        Mpeg4Ctrl   = MPEG4_QM_H263 | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_DB_FIELD | MPEG4_ENC_TRG;
    else
        Mpeg4Ctrl   = MPEG4_QM_H263 | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
   #endif
#endif   
    OSSemPend(VideoCpleSemEvt, MPEG4_TIMEOUT, &err);
    
    if (err != OS_NO_ERR)
    {
        // reset MPEG-4 hardware
        SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100; 
        
        DEBUG_MP4("Encoder Error: VideoCpleSemEvt is %d.\n", err);
        DEBUG_MP4("VideoPictureIndex = %d\n", pVideoClipOption->VideoPictureIndex);
        pVideoClipOption->MPEG4_Error = 1;
    }


    // 0x050C
    *CmpSize += Mpeg4EncStreamSize;
    if((*CmpSize < 0x20) || (*CmpSize > MPEG4_MIN_BUF_SIZE))
    {
        DEBUG_MP4("Warning!! Mpeg4EncStreamSize == %d Bytes!!!\n", Mpeg4EncStreamSize);
        //if this message always appear, check TV decoder FID/vsync signal by Lsk experience.
    }

    //DEBUG_MP4("%d(%d) ", (*CmpSize)>>10,pVideoClipOption->dftMpeg4Quality);
    
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif  

    OSSemPost(mpeg4ReadySemEvt);    // release MPEG4 HW

    return 1;   
}

s32 MultiChannelMPEG4Output1Frame(u8* pBuf, s64* pTime, u32* pSize, u32* Mpeg4EncCnt, VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u8  err;
    u8  i;
    u32 bitpos; /*BJ 0530 S*/

    switch(pVideoClipOption->VideoChannelID)
    {
        case 0:
            OSSemPend(isuSemEvt, 30, &err);
            if (err != OS_NO_ERR)
            {
                pVideoClipOption->MPEG4_Error = 1;
                isuStop();
                ipuStop();
                siuStop();    
                
                DEBUG_MP4("Error: isuSemEvt(video capture mode) is %d.\n", err);
                DEBUG_MP4("isu_int_status = 0x%08x,0x%08x\n", isu_int_status,SYS_CTL0);

                //Reset SIU/IPU/ISU
                SYS_RSTCTL  = SYS_RSTCTL | 0x00000058;
                for(i=0;i<10;i++);
                SYS_RSTCTL  = SYS_RSTCTL & (~0x00000058); 
                return 1;
            }
            break;
        case 1:
            OSSemPend(ciuCapSemEvt_CH1, 30, &err);
            if (err != OS_NO_ERR)
            {
                pVideoClipOption->MPEG4_Error = 1;
                ciu_1_Stop();

                DEBUG_MP4("Error: ciuCapSemEvt_CH1(video capture mode) is %d.\n", err);

                //Reset CIU
                SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
                for(i=0;i<10;i++);
                SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
                return 1;
            }
            break;
        case 2:
            OSSemPend(ciuCapSemEvt_CH2, 30, &err);
            if (err != OS_NO_ERR)
            {
                pVideoClipOption->MPEG4_Error = 1;
                ciu_2_Stop();

                DEBUG_MP4("Error: ciuCapSemEvt_CH2(video capture mode) is %d.\n", err);

                //Reset CIU
                SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
                for(i=0;i<10;i++);
                SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
                return 1;
            }
            break;
        case 3:
            OSSemPend(ciuCapSemEvt_CH3, 30, &err);
            if (err != OS_NO_ERR)
            {
                pVideoClipOption->MPEG4_Error = 1;
                ciu_3_Stop();

                DEBUG_MP4("Error: ciuCapSemEvt_CH3(video capture mode) is %d.\n", err);

                //Reset CIU
                SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
                for(i=0;i<10;i++);
                SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
                return 1;
            }
            break;

        case 4:
            OSSemPend(ciuCapSemEvt_CH4, 30, &err);
            if (err != OS_NO_ERR)
            {
                pVideoClipOption->MPEG4_Error = 1;
                ciu_4_Stop();

                DEBUG_MP4("Error: ciuCapSemEvt_CH4(video capture mode) is %d.\n", err);

                //Reset CIU
                SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
                for(i=0;i<10;i++);
                SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
                return 1;
            }
            break;
    }
    
    *pSize  = 0;
    *pTime += pVideoClipOption->ISUFrameDuration[pVideoClipOption->VideoPictureIndex % ISU_FRAME_DURATION_NUM]; 

     
    bitpos  = MultiChannelMPEG4PutVOPHeader(pBuf, pVideoClipOption->Vop_Type, pTime, pSize, *Mpeg4EncCnt, pVideoClipOption);
    MultiChannelMPEG4Coding1Frame(pBuf+*pSize, pVideoClipOption->Vop_Type, pTime, pSize, *Mpeg4EncCnt, bitpos, pVideoClipOption);
    
    *Mpeg4EncCnt = (*Mpeg4EncCnt) + 1;

}

/*

Routine Description:

    The Multiple channel MPEG4 encoder task.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
void MultiChannelMPEG4EncoderTask(void* pData)
{
    u32*        pFlag;
    s64*        pTime;
    u32*        pSize;
    u8          *pBuf, *pReadBuf;
    u8          err;
    u32         NextIdx;
    //u32         Mpeg4EncCnt = 0; //Lucina: for internal use: roundtype, reconstruct frame switch.
    u32         doublefield_thr;
    s32         i, DropFrame;
    u32         TotalSAD;
    u32         outLen;

#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    VIDEO_CLIP_OPTION   *pVideoClipOption;

    pVideoClipOption    = (VIDEO_CLIP_OPTION*)pData;

    pVideoClipOption->MPEG4_Task_Go = 1;    // 0: never run, 1: ever run
    
    while (1)
    {
        OSSemPend(pVideoClipOption->VideoTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
    
        if (err != OS_NO_ERR)
        {
            DEBUG_MP4("Error: VideoTrgSemEvt is %d.\n", err);
        }
        
        pBuf                        = pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngWriteIdx].buffer;  /*BJ 0530 S*/
        pFlag                       = &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngWriteIdx].flag; 
        pTime                       = &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngWriteIdx].time; 
        pSize                       = &pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngWriteIdx].size; 
        *pFlag                      = MultiChannelDeterminePictureType(pVideoClipOption->VideoPictureIndex, IVOP_PERIOD, pVideoClipOption);   // I: 1, P: 0
        pVideoClipOption->Vop_Type  = (*pFlag)? 0 : 1; // 0: I frame, 1: P frame
        
        if(!pVideoClipOption->Vop_Type) {     // I frame
            DEBUG_MP4("Ch%dI ", pVideoClipOption->VideoChannelID);
        }
                    
        //--------Video rate control: Calculate QP-----//            
        if(pVideoClipOption->mpRateControlParam.enable_ratecontrol)
        {
            //DEBUG_MP4("--RCQ2_QuantAdjust---\n");
            if (pVideoClipOption->Vop_Type == 0) //I frame
            {
                pVideoClipOption->dftMpeg4Quality    = pVideoClipOption->mpRateControlParam.InitQP = RCQ2_QuantAdjust(&pVideoClipOption->mpRateControlParam, 0, 0);
            }
            else //P frame
            {
                pVideoClipOption->dftMpeg4Quality    = pVideoClipOption->mpRateControlParam.InitQP = RCQ2_QuantAdjust(&pVideoClipOption->mpRateControlParam,
                                                                                                                       pVideoClipOption->mpRateControlParam.Int_prev_PMad,
                                                                                                                       1);
            #if(USE_PROGRESSIVE_SENSOR == 1) //Use progressive-scan sensor like CMOS
                pVideoClipOption->video_double_field_flag = 0;
            #else //Use interlace-scan sensor like CCD
                if((pVideoClipOption->mpeg4Width != 320) &&
                    ((pVideoClipOption->VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_5) || 
                     (pVideoClipOption->VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_10) || 
                     (pVideoClipOption->VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_15 && mpeg4VideoRecQulity == MPEG4_VIDEO_QUALITY_LOW))) 
                {
                    pVideoClipOption->video_double_field_flag = 1; //Lucian: 低frame rate 用double field.
                }
              #if (DE_INTERLACE_SEL == DOUBLE_FIELD_ON) //For Cwell, 全部用double field.
                else if( pVideoClipOption->mpeg4Width != 320 )
                {
                    pVideoClipOption->video_double_field_flag = 1;
                }
              #elif (DE_INTERLACE_SEL == DOUBLE_FIELD_OFF) //For Aurum, 全部不用double field.
                else if( pVideoClipOption->mpeg4Width != 320 )
                {
                    pVideoClipOption->video_double_field_flag = 0;
                }
              #else     // (DE_INTERLACE_SEL == DOUBLE_FIELD_AUTO)
                else if( pVideoClipOption->mpeg4Width != 320 )
                {
               #if 0   // 用SAD判斷要不要做deinterlace不準,先不要用
                    //doublefield_thr   = pVideoClipOption->video_double_field_flag ? 160 : 140;
                    doublefield_thr   = pVideoClipOption->video_double_field_flag ? 20 : 40;
                    if( (pVideoClipOption->mpRateControlParam.Avg_PMad > doublefield_thr) || (pVideoClipOption->dftMpeg4Quality > 14) )
                    {
                        if(pVideoClipOption->video_double_field_flag == 0)
                        {
                            DEBUG_MP4("\nCh%d 1.video_double_field_flag = 1\n", pVideoClipOption->VideoChannelID);
                            DEBUG_MP4("Ch%d Avg_PMad                  = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->mpRateControlParam.Avg_PMad);
                            DEBUG_MP4("Ch%d dftMpeg4Quality           = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->dftMpeg4Quality);
                        }
                        pVideoClipOption->video_double_field_flag    = 1;
                        pVideoClipOption->double_field_cnt           = 0;
                    }
                    else
                    {
                    #if 1  //Lucian: 一但double field 啟動, 則維持30 frames.
                        if(pVideoClipOption->double_field_cnt < 30)
                        {
                            if(pVideoClipOption->video_double_field_flag == 0)
                            {
                                DEBUG_MP4("\nCh%d 2.video_double_field_flag = 1\n", pVideoClipOption->VideoChannelID);
                                DEBUG_MP4("Ch%d Avg_PMad                  = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->mpRateControlParam.Avg_PMad);
                                DEBUG_MP4("Ch%d dftMpeg4Quality           = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->dftMpeg4Quality);
                            }
                            pVideoClipOption->double_field_cnt++;
                            pVideoClipOption->video_double_field_flag = 1;
                        }
                        else
                        {
                            if(pVideoClipOption->video_double_field_flag == 1)
                            {
                                DEBUG_MP4("\nCh%d video_double_field_flag   = 0\n", pVideoClipOption->VideoChannelID);
                                DEBUG_MP4("Ch%d Avg_PMad                  = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->mpRateControlParam.Avg_PMad);
                                DEBUG_MP4("Ch%d dftMpeg4Quality           = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->dftMpeg4Quality);
                            }
                            pVideoClipOption->video_double_field_flag = 0;
                        }
                    #else
                        if(pVideoClipOption->dftMpeg4Quality > 9)
                            pVideoClipOption->video_double_field_flag = 1;
                        else
                        {
                            pVideoClipOption->video_double_field_flag = 0;
                        }
                    #endif
                    }
               #endif
                }
              #endif
                else //for QVGA
                    pVideoClipOption->video_double_field_flag = 0;
            #endif    
            }
            //if( (pVideoClipOption->dftMpeg4Quality>20-3) )
            //DEBUG_MP4("%d ",pVideoClipOption->dftMpeg4Quality);
             
        }

     #if (CIU1_BOB_REPLACE_MPEG_DF && (CHIP_OPTION > CHIP_A1016A) ) //Lucian: 不用 Mpeg4 double field,起動 ciu mob mode
        if(pVideoClipOption->video_double_field_flag)
        {
            switch(pVideoClipOption->VideoChannelID)
            {
        #if( CIU1_OPTION == Sensor_CCIR656 )
            case 1:
                CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                CIU_1_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
                break;
        #endif
        #if( CIU2_OPTION == Sensor_CCIR656 )
            case 2:
                CIU_2_CTL1  = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                CIU_2_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
                break;
        #endif
        #if( CIU3_OPTION == Sensor_CCIR656 )
            case 3:
                CIU_3_CTL1  = (CIU_3_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                CIU_3_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
                break;
        #endif
        #if( CIU4_OPTION == Sensor_CCIR656 )
            case 4:
                CIU_4_CTL1  = (CIU_4_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                CIU_4_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
                break;
        #endif
            }
        }
        else
        {
            switch(pVideoClipOption->VideoChannelID)
            {
        #if( CIU1_OPTION == Sensor_CCIR656 )
            case 1:
                CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656;
                break;
        #endif
        #if( CIU2_OPTION == Sensor_CCIR656 )
            case 2:
                CIU_2_CTL1  = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_656;
                break;
        #endif
        #if( CIU3_OPTION == Sensor_CCIR656 )
            case 3:
                CIU_3_CTL1  = (CIU_3_CTL1 & (~0x30)) | CIU_MODE_656;
                break;
        #endif
        #if( CIU4_OPTION == Sensor_CCIR656 )
            case 4:
                CIU_4_CTL1  = (CIU_4_CTL1 & (~0x30)) | CIU_MODE_656;
                break;
        #endif
            }
        } 
        if(pVideoClipOption->video_double_field_flag)
        {
            switch(pVideoClipOption->VideoChannelID)
            {
        #if( CIU1_OPTION == Sensor_CCIR601 )
            case 1:
                CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                CIU_1_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
                break;
        #endif
        #if( CIU2_OPTION == Sensor_CCIR601 )
            case 2:
                CIU_2_CTL1  = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                CIU_2_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
                break;
        #endif
        #if( CIU3_OPTION == Sensor_CCIR601 )
            case 3:
                CIU_3_CTL1  = (CIU_3_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                CIU_3_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
                break;
        #endif
        #if( CIU4_OPTION == Sensor_CCIR601 )
            case 4:
                CIU_4_CTL1  = (CIU_4_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                CIU_4_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
                break;
            }
        #endif
        }
        else
        {
            switch(pVideoClipOption->VideoChannelID)
            {
        #if( CIU1_OPTION == Sensor_CCIR601 )
            case 1:
                CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_601;
                break;
        #endif
        #if( CIU2_OPTION == Sensor_CCIR601 )
            case 2:
                CIU_2_CTL1  = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_601;
                break;
        #endif
        #if( CIU3_OPTION == Sensor_CCIR601 )
            case 3:
                CIU_3_CTL1  = (CIU_3_CTL1 & (~0x30)) | CIU_MODE_601;
                break;
        #endif
        #if( CIU4_OPTION == Sensor_CCIR601 )
            case 4:
                CIU_4_CTL1  = (CIU_4_CTL1 & (~0x30)) | CIU_MODE_601;
                break;
        #endif
            }
        } 
     #endif

        //------Video FIFO Management: 避免 Read/Write pointer overlay. ------//
        if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
            if((pVideoClipOption->WantChangeFile == 1) && (pVideoClipOption->GetLastVideo == 0) && (pVideoClipOption->GetLastAudio)) {
                OS_ENTER_CRITICAL();
                pVideoClipOption->LastVideo     = pVideoClipOption->VideoBufMngWriteIdx;
                pVideoClipOption->GetLastVideo  = 1;
                OS_EXIT_CRITICAL();
            }
        }

        pReadBuf    = pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer;   /* Peter 070130 */
        if((pVideoClipOption->VideoPictureIndex != 0) && (pReadBuf >= pBuf)) 
        {
            while((pReadBuf >= pBuf) &&
                  ((pReadBuf - pBuf) < MPEG4_MIN_BUF_SIZE) && 
                  (pVideoClipOption->VideoCmpSemEvt->OSEventCnt > 2) &&
                  (pVideoClipOption->sysCaptureVideoStop == 0)) 
            {
                
                DEBUG_MP4("x");
                OSTimeDly(1);
                pReadBuf    = pVideoClipOption->VideoBufMng[pVideoClipOption->VideoBufMngReadIdx].buffer;

                // GetLastVideo一直為0, 可能會造成dead lock, 所以要更新GetLastVideo
                if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
                    if((pVideoClipOption->WantChangeFile == 1) && (pVideoClipOption->GetLastVideo == 0) && (pVideoClipOption->GetLastAudio)) {
                        OS_ENTER_CRITICAL();
                        pVideoClipOption->LastVideo     = pVideoClipOption->VideoBufMngWriteIdx;
                        pVideoClipOption->GetLastVideo  = 1;
                        OS_EXIT_CRITICAL();
                    }
                }
            }
        }

        //---------- Video Encoding---------//
        *pTime  = 0;

    #if 1 //Lucian: frame rate control
        if(pVideoClipOption->VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_30)
        {
            DropFrame   = 0;
        }
        else if(pVideoClipOption->VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_15)
        {
            DropFrame   = 1;
        }
        else if(pVideoClipOption->VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_10)
        {
            DropFrame   = 2;
        }
        else if(pVideoClipOption->VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_5)
        {
            DropFrame   = 5;
        }

        for(i = 0; i < DropFrame; i++)
        {
            MultiChannelMpeg4DropFrame(pTime, pVideoClipOption);
        }
    #endif  //#if 1 //Lucian: frame rate control

        MultiChannelMPEG4Output1Frame(pBuf, pTime, pSize, &pVideoClipOption->Mpeg4EncCnt, pVideoClipOption);
    
        OS_ENTER_CRITICAL();
        pVideoClipOption->VideoTimeStatistics += *pTime;

        if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && (pVideoClipOption->VideoTimeStatistics >= (asfSectionTime * 1000)))            
        {
            pVideoClipOption->SetIVOP = 1;             
        }            
        if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && (pVideoClipOption->VideoTimeStatistics >= (asfRecTimeLen * 1000)))            
        {
            pVideoClipOption->SetIVOP = 1;             
        }
        OS_EXIT_CRITICAL();

        //--------Rebuild RC model of Rate control ------//
        if(pVideoClipOption->mpRateControlParam.enable_ratecontrol)
        {
            TotalSAD    = Mpeg4SadSum;
            outLen      = *pSize; //Video size. 
            if (pVideoClipOption->Vop_Type != I_VOP)
            {
                pVideoClipOption->mpRateControlParam.Int_prev_PMad = (TotalSAD * 4 / pVideoClipOption->mpeg4Height) * 1024 / pVideoClipOption->mpeg4Width;  //FixRC

                if( (pVideoClipOption->mpRateControlParam.PMad_cnt & 0x3) == 0 )  //Mod 4: 每四張統計一次MAD.
                {
                    pVideoClipOption->mpRateControlParam.Avg_PMad  = pVideoClipOption->mpRateControlParam.Sum_PMad / 4;
                    pVideoClipOption->mpRateControlParam.Sum_PMad  = 0;
                    //DEBUG_MP4("(%d) ",mpRateControlParam.Avg_PMad);
                }
                pVideoClipOption->mpRateControlParam.Sum_PMad += pVideoClipOption->mpRateControlParam.Int_prev_PMad;
                pVideoClipOption->mpRateControlParam.PMad_cnt ++;
            }
            //DEBUG_MP4("--RCQ2_Update2OrderModel---\n");   
            
            if(pVideoClipOption->mpRateControlParam.enable_ratecontrol)
            {
                RCQ2_Update2OrderModel(&pVideoClipOption->mpRateControlParam,
                                       outLen * 8,
                                       pVideoClipOption->Vop_Type);
            }

        }

        //-------------Video FIFO management: 計算下一個Video frame start address-------------//
        OS_ENTER_CRITICAL();
        pVideoClipOption->CurrentVideoSize   += *pSize;
		pVideoClipOption->CurrentBufferSize   += *pSize;
        OS_EXIT_CRITICAL();
    
        NextIdx = (pVideoClipOption->VideoBufMngWriteIdx + 1) % VIDEO_BUF_NUM;
        
        pBuf    = (u8*)(((u32)pBuf + *pSize + 3) & ~3);
        
        if(pBuf > pVideoClipOption->mpeg4VideBufEnd) 
        {
            DEBUG_MP4("Ch%d VideoBuf overflow!!!!\n", pVideoClipOption->VideoChannelID);
        }
        
        if(pBuf < (pVideoClipOption->mpeg4VideBufEnd - MPEG4_MIN_BUF_SIZE))
            pVideoClipOption->VideoBufMng[NextIdx].buffer   = pBuf;
        else
            pVideoClipOption->VideoBufMng[NextIdx].buffer   = pVideoClipOption->VideoBuf;
            
        pVideoClipOption->VideoBufMngWriteIdx   = NextIdx;
        pVideoClipOption->VideoPictureIndex++;    
        
        OSSemPost(pVideoClipOption->VideoCmpSemEvt);  
    #if NIC_SUPPORT
		if(EnableStreaming)
	        OSSemPost(VideoRTPCmpSemEvt[0]);  
	    #if TUTK_SUPPORT         
	        if(P2PEnableStreaming[0]) //Local_Record, Local_Playback
	            OSSemPost(P2PVideoCmpSemEvt[0]);  
	    #endif
    #endif
    }
}

/*

Routine Description:

    Multiple channel put VOL header. 

Arguments:

    pHeader             - Video header.
    pHeaderSize         - Video header size.
    pVideoClipOption    - Multiple channel video clip option.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 MultiChannelMpeg4EncodeVolHeader(VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u8 i;

#if USE_MPEG_QUANTIZATION   // quantization method choose MPEG method
    u8 VolHeader[0x13] =    
    { 
        0x00, 0x00, 0x01, 0x00,  
        0x00, 0x00, 0x01, 0x20,
        0x00, 0xc4, 0x88, 0xba,        
        0x98, 0x50, 0x00, 0x40,
        0x01, 0x49, 0x0f
    };

    
#else   // quantization method choose H263 method
    u8 VolHeader[0x13] =    
    { 
        0x00, 0x00, 0x01, 0x00,  
        0x00, 0x00, 0x01, 0x20,
        0x00, 0xc4, 0x88, 0xba,        
        0x98, 0x50, 0x00, 0x40,
        0x01, 0x44, 0x3f
    };
#endif

    VolHeader[0x0d] |= (u8)(pVideoClipOption->mpeg4Width >> 9); 
    VolHeader[0x0e] |= (u8)(pVideoClipOption->mpeg4Width >> 1);     
    VolHeader[0x0f] |= (u8)(pVideoClipOption->mpeg4Width << 7); 
    
    VolHeader[0x0f] |= (u8)(pVideoClipOption->mpeg4Height >> 7);
    VolHeader[0x10] |= (u8)(pVideoClipOption->mpeg4Height << 1);

    pVideoClipOption->asfVideHeaderSize = sizeof(VolHeader);
    memcpy(pVideoClipOption->asfVideHeader, (u8*)VolHeader, sizeof(VolHeader));

    pVideoClipOption->mpeg4VopTimeInc   = 0; //reset
    return 1;
}

void MultiChannelMpeg4ConfigQualityFrameRate(int BitRateLevel, VIDEO_CLIP_OPTION *pVideoClipOption)
{

    /* Lucian:
        Target Bitrate  (kbps)@30 fps; Lucian: 目前實際值 = 理論值*1.3

         理論值        |       實際值
         1.00M bps            1.33M bps
         1.50M bps            1.85M bps
         1.70M bps            2.21M bps
         2.00M bps            2.99M bps

         30/15 fps: 用rate control 控制QP.
         5     fps: 因時間上不連續. 則直接控制QP.
        
    */
    DEBUG_MP4("\nCh%d BitLevel=%d\n", pVideoClipOption->VideoChannelID, BitRateLevel);

    pVideoClipOption->mpRateControlParam.max_Qp=RC_MAX_QP;                  //max Qp
	pVideoClipOption->mpRateControlParam.min_Qp=RC_MIN_QP;                   //min Qp
    if( (pVideoClipOption->mpeg4Width == 320) || (pVideoClipOption->mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
    {
        switch(pVideoClipOption->VideoRecFrameRate)
        {
        case MPEG4_VIDEO_FRAMERATE_60:
            pVideoClipOption->mpRateControlParam.enable_ratecontrol   = 1;
            pVideoClipOption->mpRateControlParam.Framerate  = 60;               //Target Framerate

            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 666 * 1000; // 2.99 Mb/sec
                pVideoClipOption->mpRateControlParam.InitQP=6;
                pVideoClipOption->mpRateControlParam.QP_I=6;
                pVideoClipOption->mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 566 * 1000; // 2.21 Mb/sec
                pVideoClipOption->mpRateControlParam.InitQP=7;
                pVideoClipOption->mpRateControlParam.QP_I=7;
                pVideoClipOption->mpRateControlParam.QP_P=7;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 333 * 1000; // 1.33 Mb/sec
                pVideoClipOption->mpRateControlParam.InitQP=8;
                pVideoClipOption->mpRateControlParam.QP_I=8;
                pVideoClipOption->mpRateControlParam.QP_P=8;
            break;
            }
            
            
            break;
        case MPEG4_VIDEO_FRAMERATE_30:
            pVideoClipOption->mpRateControlParam.enable_ratecontrol   = 1;
          #if ((Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601)) //TV-in
            if(sysTVinFormat == TV_IN_NTSC)
               pVideoClipOption->mpRateControlParam.Framerate  = 30;               //Target Framerate
            else
               pVideoClipOption->mpRateControlParam.Framerate  = 25;               //Target Framerate
          #else //Sensor-in
               pVideoClipOption->mpRateControlParam.Framerate  = 30;  
          #endif

            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 666 * 1000; // 2.99 Mb/sec
                pVideoClipOption->mpRateControlParam.InitQP=6;
                pVideoClipOption->mpRateControlParam.QP_I=6;
                pVideoClipOption->mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 566 * 1000; // 2.21 Mb/sec
                pVideoClipOption->mpRateControlParam.InitQP=7;
                pVideoClipOption->mpRateControlParam.QP_I=7;
                pVideoClipOption->mpRateControlParam.QP_P=7;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 333 * 1000; // 1.33 Mb/sec
                pVideoClipOption->mpRateControlParam.InitQP=8;
                pVideoClipOption->mpRateControlParam.QP_I=8;
                pVideoClipOption->mpRateControlParam.QP_P=8;
            break;
            }
        break;

        case MPEG4_VIDEO_FRAMERATE_15:
            pVideoClipOption->mpRateControlParam.enable_ratecontrol   = 1;
         #if ((Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601)) //TV-in
            if(sysTVinFormat == TV_IN_NTSC)
               pVideoClipOption->mpRateControlParam.Framerate  = 15;               //Target Framerate
            else
               pVideoClipOption->mpRateControlParam.Framerate  = 12;               //Target Framerate
         #else //Sensor-in
              pVideoClipOption->mpRateControlParam.Framerate  = 15;
         #endif

            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 266 * 1000;   // 1.01 Mb/sec
                pVideoClipOption->mpRateControlParam.InitQP=6;
                pVideoClipOption->mpRateControlParam.QP_I=6;
                pVideoClipOption->mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 200 * 1000;
                pVideoClipOption->mpRateControlParam.InitQP=8;
                pVideoClipOption->mpRateControlParam.QP_I=8;
                pVideoClipOption->mpRateControlParam.QP_P=8;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 133 * 1000;
                pVideoClipOption->mpRateControlParam.InitQP=10;
                pVideoClipOption->mpRateControlParam.QP_I=10;
                pVideoClipOption->mpRateControlParam.QP_P=10;
            break;
        }
        break;

		case MPEG4_VIDEO_FRAMERATE_10:
			pVideoClipOption->mpRateControlParam.enable_ratecontrol	= 0;
			switch(mpeg4VideoRecQulity)
			{
			case MPEG4_VIDEO_QUALITY_HIGH:
				pVideoClipOption->dftMpeg4Quality = 7;
			break;
		
			case MPEG4_VIDEO_QUALITY_MEDIUM:
				pVideoClipOption->dftMpeg4Quality = 12;
			break;
		
			case MPEG4_VIDEO_QUALITY_LOW:
				pVideoClipOption->dftMpeg4Quality = 17;
			break;
		   }
		   break;
		
        case MPEG4_VIDEO_FRAMERATE_5:
            pVideoClipOption->mpRateControlParam.enable_ratecontrol   = 0;
            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                pVideoClipOption->dftMpeg4Quality = 7;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                pVideoClipOption->dftMpeg4Quality = 12;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                pVideoClipOption->dftMpeg4Quality = 17;
            break;
            }
        break;
        }

        switch(BitRateLevel)
        {
           case MPEG_BITRATE_LEVEL_100:
            break;

           case MPEG_BITRATE_LEVEL_80:
                pVideoClipOption->mpRateControlParam.TargetBitrate=pVideoClipOption->mpRateControlParam.TargetBitrate*80/100;
                pVideoClipOption->mpRateControlParam.InitQP +=3;
                pVideoClipOption->mpRateControlParam.QP_I   +=3;
                pVideoClipOption->mpRateControlParam.QP_P   +=3;
	            pVideoClipOption->mpRateControlParam.min_Qp =10;                   //min Qp
            break;

           case MPEG_BITRATE_LEVEL_60:
                pVideoClipOption->mpRateControlParam.TargetBitrate=pVideoClipOption->mpRateControlParam.TargetBitrate*60/100;
                pVideoClipOption->mpRateControlParam.InitQP +=8;
                pVideoClipOption->mpRateControlParam.QP_I   +=8;
                pVideoClipOption->mpRateControlParam.QP_P   +=8;
                pVideoClipOption->mpRateControlParam.min_Qp  =15;                   //min Qp
            break;
        }
    }
    else  //VGA size//
    {
        switch(pVideoClipOption->VideoRecFrameRate)
        {
        case MPEG4_VIDEO_FRAMERATE_60:
            //Cannot support VGA 60 fps,Now
        case MPEG4_VIDEO_FRAMERATE_30:
            pVideoClipOption->mpRateControlParam.enable_ratecontrol   = 1;
        #if ((Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601)) //TV-in
            if(sysTVinFormat == TV_IN_NTSC)
               pVideoClipOption->mpRateControlParam.Framerate  = 30;               //Target Framerate
            else
               pVideoClipOption->mpRateControlParam.Framerate  = 25;               //Target Framerate
        #else //sensor-in
              pVideoClipOption->mpRateControlParam.Framerate  = 30; 
        #endif

            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                  pVideoClipOption->mpRateControlParam.TargetBitrate    = 2000 * 1000; // 2.99 Mb/sec
                  pVideoClipOption->mpRateControlParam.InitQP=6;
                  pVideoClipOption->mpRateControlParam.QP_I=6;
                  pVideoClipOption->mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 1700 * 1000; // 2.21 Mb/sec
                pVideoClipOption->mpRateControlParam.InitQP=8;
                pVideoClipOption->mpRateControlParam.QP_I=8;
                pVideoClipOption->mpRateControlParam.QP_P=8;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 1000 * 1000; // 1.33 Mb/sec
                pVideoClipOption->mpRateControlParam.InitQP=10;
                pVideoClipOption->mpRateControlParam.QP_I=10;
                pVideoClipOption->mpRateControlParam.QP_P=10;
            break;
            }
        break;

        case MPEG4_VIDEO_FRAMERATE_15:
            pVideoClipOption->mpRateControlParam.enable_ratecontrol   = 1;
        #if ((Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601)) //TV-in
            if(sysTVinFormat == TV_IN_NTSC)
               pVideoClipOption->mpRateControlParam.Framerate  = 15;               //Target Framerate
            else
               pVideoClipOption->mpRateControlParam.Framerate  = 12;               //Target Framerate
        #else //sensor-in
            pVideoClipOption->mpRateControlParam.Framerate  = 15;
        #endif

            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 800 * 1000;   // 1.01 Mb/sec
                pVideoClipOption->mpRateControlParam.InitQP=6;
                pVideoClipOption->mpRateControlParam.QP_I=6;
                pVideoClipOption->mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 600 * 1000;
                pVideoClipOption->mpRateControlParam.InitQP=8;
                pVideoClipOption->mpRateControlParam.QP_I=8;
                pVideoClipOption->mpRateControlParam.QP_P=8;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                pVideoClipOption->mpRateControlParam.TargetBitrate    = 400 * 1000;
                pVideoClipOption->mpRateControlParam.InitQP=10;
                pVideoClipOption->mpRateControlParam.QP_I=10;
                pVideoClipOption->mpRateControlParam.QP_P=10;
            break;
            }
        break;

         case MPEG4_VIDEO_FRAMERATE_10:
			 pVideoClipOption->mpRateControlParam.enable_ratecontrol	 = 0;
			 switch(mpeg4VideoRecQulity)
			 {
			 case MPEG4_VIDEO_QUALITY_HIGH:
				 pVideoClipOption->dftMpeg4Quality = 7;
			 break;
		 
			 case MPEG4_VIDEO_QUALITY_MEDIUM:
				 pVideoClipOption->dftMpeg4Quality = 12;
			 break;
		 
			 case MPEG4_VIDEO_QUALITY_LOW:
				 pVideoClipOption->dftMpeg4Quality = 17;
			 break;
		 	}
        	break;

        case MPEG4_VIDEO_FRAMERATE_5:
            pVideoClipOption->mpRateControlParam.enable_ratecontrol   = 0;
            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                pVideoClipOption->dftMpeg4Quality = 7;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                pVideoClipOption->dftMpeg4Quality = 12;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                pVideoClipOption->dftMpeg4Quality = 17;
            break;
        }
        break;
        }

        switch(BitRateLevel)
        {
           case MPEG_BITRATE_LEVEL_100:
            break;

           case MPEG_BITRATE_LEVEL_80:
               pVideoClipOption->mpRateControlParam.TargetBitrate=pVideoClipOption->mpRateControlParam.TargetBitrate*80/100;
               pVideoClipOption->mpRateControlParam.InitQP +=3;
               pVideoClipOption->mpRateControlParam.QP_I   +=3;
               pVideoClipOption->mpRateControlParam.QP_P   +=3;
               pVideoClipOption->mpRateControlParam.min_Qp  =10;                   //min Qp
            break;

           case MPEG_BITRATE_LEVEL_60:
               pVideoClipOption->mpRateControlParam.TargetBitrate=pVideoClipOption->mpRateControlParam.TargetBitrate*60/100;
               pVideoClipOption->mpRateControlParam.InitQP +=8;
               pVideoClipOption->mpRateControlParam.QP_I   +=8;
               pVideoClipOption->mpRateControlParam.QP_P   +=8;
               pVideoClipOption->mpRateControlParam.min_Qp  =15;                   //min Qp
            break;
        }
    }

    //Lsk 090626
#if(USE_PROGRESSIVE_SENSOR == 1) //Use progressive-scan sensor like CMOS
    pVideoClipOption->video_double_field_flag = 0;
#else //Use interlace-scan sensor like CCD
    if((pVideoClipOption->mpeg4Width != 320) &&
        ((pVideoClipOption->VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_5) || 
         (pVideoClipOption->VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_10) || 
         (pVideoClipOption->VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_15 && mpeg4VideoRecQulity == MPEG4_VIDEO_QUALITY_LOW))) 
    {
        pVideoClipOption->video_double_field_flag = 1; //Lucian: 低frame rate 用double field.
    }
  #if (DE_INTERLACE_SEL == DOUBLE_FIELD_ON) //For Cwell, 全部用double field.
    else if( pVideoClipOption->mpeg4Width != 320 )
    {
      pVideoClipOption->video_double_field_flag = 1;
    }
  #elif (DE_INTERLACE_SEL == DOUBLE_FIELD_OFF) //For Aurum, 全部不用double field.
    else if( pVideoClipOption->mpeg4Width != 320 )
    {
      pVideoClipOption->video_double_field_flag = 0;
    }
  #else
    else if( pVideoClipOption->mpeg4Width != 320 ) //Auto mode: update by SAD in mpeg4 engine.
    {
       pVideoClipOption->video_double_field_flag = 0;
    }
  #endif
    else //for QVGA
        pVideoClipOption->video_double_field_flag = 0;
#endif  

    TVout_Generate_Pause_Frame = 0;
    #if (((Sensor_OPTION == Sensor_CCIR601)||(Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)) && ( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_D1) ) )
    {
        //Lsk can't sure current playback file quality and frame-rate(video_double_field_flag) setting
        if(sysVideoInSel == VIDEO_IN_TV)
           TVout_Generate_Pause_Frame = 1;
    }
    #endif
    
    pVideoClipOption->ASF_set_interlace_flag = 0;
    #if (USE_PROGRESSIVE_SENSOR == 0)
    {
        if((pVideoClipOption->video_double_field_flag==0)&&(pVideoClipOption->mpeg4Width != 320))
            pVideoClipOption->ASF_set_interlace_flag = 1;
    }
    #endif
    
    if(pVideoClipOption->mpRateControlParam.enable_ratecontrol)
	   RCQ2_init(&pVideoClipOption->mpRateControlParam);
}

int MultiChannelMpeg4ModifyTargetBitRate(int NewBitRate, VIDEO_CLIP_OPTION *pVideoClipOption)
{
     pVideoClipOption->mpRateControlParam.RCQ2_config.bit_rate      = NewBitRate;
     pVideoClipOption->mpRateControlParam.RCQ2_config.target_rate   = NewBitRate / pVideoClipOption->mpRateControlParam.Framerate;
     
     return 1;
}

#endif  // #if(VIDEO_CODEC_OPTION ==  MPEG4_CODEC)

s32 MultiChannelMpeg4SetVideoFrameRate(u8 framerate, VIDEO_CLIP_OPTION *pVideoClipOption)
{
    pVideoClipOption->VideoRecFrameRate = framerate;
    return 1;
}

void MultiChannelMpeg4DropFrame(s64* pTime, VIDEO_CLIP_OPTION *pVideoClipOption)
{
    u8          err;

    switch(pVideoClipOption->VideoChannelID)
    {
#if (MULTI_CHANNEL_SEL & 0x01)
    case 0:
        OSSemPend(isuSemEvt, 4 ,&err);
        if(OS_NO_ERR != err)
        {
            DEBUG_MP4("Ch%d Encoder Error: isuSemEvt is %d.\n", pVideoClipOption->VideoChannelID, err);
            DEBUG_MP4("Ch%d VideoPictureIndex = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoPictureIndex);
        }
        *pTime += pVideoClipOption->ISUFrameDuration[pVideoClipOption->VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
        pVideoClipOption->VideoPictureIndex++;
        pVideoClipOption->mp4_avifrmcnt++;
        break;
#endif
#if (MULTI_CHANNEL_SEL & 0x02)
    case 1:
        OSSemPend(ciuCapSemEvt_CH1, 4 ,&err);
        if(OS_NO_ERR != err)
        {
            DEBUG_MP4("Ch%d Encoder Error: ciuCapSemEvt_CH1 is %d.\n", pVideoClipOption->VideoChannelID, err);
            DEBUG_MP4("Ch%d VideoPictureIndex = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoPictureIndex);
        }
        *pTime += pVideoClipOption->ISUFrameDuration[pVideoClipOption->VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
        pVideoClipOption->VideoPictureIndex++;
        pVideoClipOption->mp4_avifrmcnt++;
        break;
#endif
#if (MULTI_CHANNEL_SEL & 0x04)
    case 2:
        OSSemPend(ciuCapSemEvt_CH2, 4 ,&err);
        if(OS_NO_ERR != err)
        {
            DEBUG_MP4("Ch%d Encoder Error: ciuCapSemEvt_CH2 is %d.\n", pVideoClipOption->VideoChannelID, err);
            DEBUG_MP4("Ch%d VideoPictureIndex = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoPictureIndex);
        }
        *pTime += pVideoClipOption->ISUFrameDuration[pVideoClipOption->VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
        pVideoClipOption->VideoPictureIndex++;
        pVideoClipOption->mp4_avifrmcnt++;
        break;
#endif
#if (MULTI_CHANNEL_SEL & 0x08)
    case 3:
        OSSemPend(ciuCapSemEvt_CH3, 4 ,&err);
        if(OS_NO_ERR != err)
        {
            DEBUG_MP4("Ch%d Encoder Error: ciuCapSemEvt_CH3 is %d.\n", pVideoClipOption->VideoChannelID, err);
            DEBUG_MP4("Ch%d VideoPictureIndex = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoPictureIndex);
        }
        *pTime += pVideoClipOption->ISUFrameDuration[pVideoClipOption->VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
        pVideoClipOption->VideoPictureIndex++;
        pVideoClipOption->mp4_avifrmcnt++;
        break;
#endif
#if (MULTI_CHANNEL_SEL & 0x10)
    case 4:
        OSSemPend(ciuCapSemEvt_CH4, 4 ,&err);
        if(OS_NO_ERR != err)
        {
            DEBUG_MP4("Ch%d Encoder Error: ciuCapSemEvt_CH4 is %d.\n", pVideoClipOption->VideoChannelID, err);
            DEBUG_MP4("Ch%d VideoPictureIndex = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoPictureIndex);
        }
        *pTime += pVideoClipOption->ISUFrameDuration[pVideoClipOption->VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
        pVideoClipOption->VideoPictureIndex++;
        pVideoClipOption->mp4_avifrmcnt++;
        break;
#endif
#if (MULTI_CHANNEL_SEL & 0x20)
    case 5:
        OSSemPend(ciuCapSemEvt_CH5, 4 ,&err);
        if(OS_NO_ERR != err)
        {
            DEBUG_MP4("Ch%d Encoder Error: ciuCapSemEvt_CH5 is %d.\n", pVideoClipOption->VideoChannelID, err);
            DEBUG_MP4("Ch%d VideoPictureIndex = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoPictureIndex);
        }
        *pTime += pVideoClipOption->ISUFrameDuration[pVideoClipOption->VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
        pVideoClipOption->VideoPictureIndex++;
        pVideoClipOption->mp4_avifrmcnt++;
        break;
#endif

    default:
        DEBUG_MP4("Error: Ch%d not support!!!", pVideoClipOption->VideoChannelID);
    }
}



#endif  // #if MULTI_CHANNEL_VIDEO_REC

