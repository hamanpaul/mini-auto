/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    mpeg4.c

Abstract:

    The routines of MPEG-4 encoder/decoder.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2005/08/26  David Tsai  Create  

*/

#include "general.h"
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
#include "Timerapi.h"
#include "rfiuapi.h"
#include "dcfapi.h"     //Toby 130306
#include "intapi.h"
#include "VideoCodecAPI.h"
#include "H264api.h"
#include "ciuapi.h"
#include "GlobalVariable.h"

#if PWIFI_SUPPORT
#include "pwifiapi.h"
#endif
/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

#if (HW_BOARD_OPTION == MR9160_TX_DB_BATCAM && Sensor_OPTION == Sensor_PO2210K_YUV601) //first 3 dark bad frames, VMD difference huge, so can't be reference(AE,AWB applying leads)
#define START_VIDEO_INDEX 3
#else
#define START_VIDEO_INDEX 2
#endif
/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
unsigned int asfVideoFrameCount; 
u32 VideoTimeStatistics=0;
u32 VideoBufMngWriteIdx;
u8 SetIVOP=0;

#if MULTI_STREAM_START_ON
  u32  mpeg4MultiStreamEnable=1; 
  u32  mpeg4MultiStreamStart=1;
#else
  u32  mpeg4MultiStreamEnable=0; 
  u32  mpeg4MultiStreamStart=0;
#endif

u32  IVOP_PERIOD = 60;

 
/*BJ 0530 S*/
u8  mpeg4SliceMask[4] = {0,1,3,7};
const u32 PutBitsMask[32]= {
        0x00000001, 0x00000003, 0x00000007, 0x0000000f,
        0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
        0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
        0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
        0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
        0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
        0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
        0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};
u32 HeaderBuf;
u32 mpeg4VopTimeInc=0;
u32 mpegflag;   /*Peter 1109 S*/
/*BJ 0530 E*/

#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
u8  Stuffing[9] = {
    0x00, 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f
};
#endif

/* task and event related */
OS_STK      mpeg4TaskStack[MPEG4_TASK_STACK_SIZE]; /* Stack of task mpeg4Task() */
OS_EVENT*   VideoTrgSemEvt;
OS_EVENT*   VideoCmpSemEvt;
OS_EVENT*   VideoRTPCmpSemEvt[MULTI_CHANNEL_MAX];
OS_EVENT*   VideoCpleSemEvt; /*BJ 0530 S*/

/* buffer management */
u32 VideoBufMngReadIdx;
/*Peter 1109 S*/
VIDEO_BUF_MNG VideoBufMng[VIDEO_BUF_NUM]; 
VIDEO_BUF_MNG *p_VideoBufMng;
/*BJ 0530 S*/
u32 CurrentVideoSize;

#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
    (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
VIDEO_BUF_MNG P2PVideoBufMng[10]; 
VIDEO_BUF_MNG P2PBusyVideoBufMng[10]; 
#else
VIDEO_BUF_MNG P2PVideoBufMng[VIDEO_BUF_NUM-10]; //Toby 130815
VIDEO_BUF_MNG P2PBusyVideoBufMng[10]; //Toby 130815
#endif

#if(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
extern u32 P2PVideoBufReadIdx[1];
extern int P2PTxBufFullness[1];
extern int P2PTxBitRate[1];
#endif

u8 video_double_field_flag=0;

int last_3s_bitrate[4] = {-1,-1,-1,-1}; //[1] last 1sec, [2]: last 2sec, [3]: last 3sec
/*Peter 1109 E*/
/* Peter 070108 S */

/* Peter 070108 E */



u8  *mpeg4RefBuf_Y, *mpeg4RefBuf_Cb, *mpeg4RefBuf_Cr, *mpeg4McoBuf_Y, *mpeg4McoBuf_Cb, *mpeg4McoBuf_Cr;
s32 mp4_avifrmcnt;
s32 mpeg4MBRef;
u32 Vop_Type;  /* Peter 0704 */
/*Peter 1109 S*/
u32 MPEG4_Mode;     // 0: record, 1: playback
u32 MPEG4_Status;
u32 MPEG4_Task_Go;  // 0: never run, 1: ever run
/*Peter 1109 E*/
u32 MPEG4_Error;

MP4_Option  Mp4Dec_opt;  /* Peter: 0707 */

/* playback related */

s64 Videodisplaytime[DISPLAY_BUF_NUM];

u8 dftMpeg4Quality      = 7;  //設定Video Clip Quality. Set Qp.
u8 mpeg4VideoRecQulity  = MPEG4_VIDEO_QUALITY_HIGH;
u8 VideoRecFrameRate    = MPEG4_VIDEO_FRAMERATE_30;
 
u8 Video_60fps_flag     =0;//Lucian: QVGA 60 fps, preview 只呈現30 fps. 特例處理
u8 show_flag = 0;

//u8 mpeg4IVOP_period=30;
/*BJ 0530 E*/
/* cytsai: for armulator only */
/* picture index */
u32 VideoPictureIndex;  /*BJ 0530 S*/   /*CY 0907*/
u32 VideoSmallPictureIndex;  /*BJ 0530 S*/   /*CY 0907*/
u32 NVOPCnt;
  #if(VIDEO_RESOLUTION_SEL== VIDEO_VGA_IN_VGA_OUT)
    u32 mpeg4Width = 640,  mpeg4Height = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_VGA_OUT)
    u32 mpeg4Width = 640,  mpeg4Height = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_FULLHD_IN_VGA_OUT)
    u32 mpeg4Width = 640,  mpeg4Height = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_1920x1440DECI3x3TOVGA)
    u32 mpeg4Width = 640,  mpeg4Height = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_HD_OUT)
    u32 mpeg4Width = 1280, mpeg4Height = 720;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_FULLHD_IN_HD_OUT)
    u32 mpeg4Width = 1280, mpeg4Height = 720;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_FULL_HD)
    u32 mpeg4Width = 1920, mpeg4Height = 1072;
  #else
    u32 mpeg4Width = 640,  mpeg4Height = 480;
  #endif
u32 Cal_FileTime_Start_Idx;
u8 TVout_Generate_Pause_Frame;
u8 ASF_set_interlace_flag;    


DEF_RATECONTROL_PARA mpRateControlParam;


/*
 *********************************************************************************************************
 * Extern Varaibel
 *********************************************************************************************************
 */
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];

extern int rfiuBatCamPIRTrig;
#if TX_PIRREC_SUPPORT 
extern u32 rfiuStopPIRRecReady;
#endif
extern u32 guiRFTimerID;

extern u32 sysVideoInSel;
extern u32 EventTrigger;  //用於Buffer moniting.
extern u32 asfVopCount;   //用於Buffer moniting.

extern s32 mp4_avifrmcnt, isu_avifrmcnt;
extern u32 IsuIndex;

extern u8* mpeg4outputbuf[3];
extern u8  sysCaptureVideoStop;
extern u32 unMp4FrameDuration[SIU_FRAME_DURATION_NUM]; /* mh@2006/11/22: for time stamp control */ /* Peter 070403 */
extern u32 isu_int_status;
extern DEF_RATECONTROL_PARA mpRateControlParam; //Lucian: For Rate Control

#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
extern u8 ResetPlayback;  			//Lsk 090401 : reset playback system 
#endif
extern u8 TvOutMode;
extern s32 mp4_avifrmcnt, isu_avifrmcnt;
#if DCF_WRITE_STATISTIC
extern u32 dcfWriteAccum;
extern u32 dcfMpegBufRemain;
#endif

extern u8 BandWidthControl;

#if NIC_SUPPORT
extern u8 EnableStreaming;
#endif
#if TUTK_SUPPORT
extern s8 P2PEnableStreaming[];
extern int gPlaybackWidth;
extern int gPlaybackHeight;
#endif
//extern u8  Start_MPEG4TimeStatistics;

OS_EVENT    *mpeg4ReadySemEvt;
extern OS_EVENT *gRfiuAVCmpSemEvt[MAX_RFIU_UNIT];

#if RF_TX_OPTIMIZE
extern unsigned int gRfiu_MpegEnc_Sta[MAX_RFIU_UNIT];
#endif

u8 filecon;
u8 splitmenu = 0; // 0: jpeg decode 滿屏(800*480), 1; jpeg decode 4分割 playback 畫面(400*480), 2: jpeg decode 滿屏(640*480)
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

extern void RCQ2_init(DEF_RATECONTROL_PARA *pRateControlParam);

/*BJ 0530 S*/
s32 mpeg4Output1Frame(u8* , s64* , u32* , u32*, u32*);
s32 mpeg4Coding1Frame(u8*, u32, s64*, u32*, u32, u32, u32*);
u32 mpeg4PutVOPHeader(u8*, u32, s64*, u32*,u32);
u32 mpeg4PutHeader(u8*, u8*, u8, u32);
/*BJ 0530 E*/
s32 rfiuTxRateControl_Start(int BitRate,
                                   int VideoInFrameRate,
                                   int SizeLevel);
s32 rfiuTxRateControl_GetSwitchBufferSize(
                                             int SizeLevel,
                                             int *pDrop_ext,
                                             int *pBitRateRatioPercent
                                        );
s32 rfiuTxRateControl_LiveMode(
                                             int TxBufFullness,
                                             int SizeLevel,
                                             unsigned int *pVideoCnt,
                                             int *pDrop_ext,
                                             int *pQP_ext2,
                                             int *pBitRateRatioPercent
                                        );

s32 rfiuTxRateControl_PIRRecMode(
                                             int VideoBufferSize,
                                             unsigned int *pVideoCnt,
                                             int *pDrop_ext,
                                             int *pQP_ext2,
                                             int *pBitRateRatioPercent
                                            );              

s32 mpeg4Decoding1Frame(MP4Dec_Bits* Bits, u32 BitsLen,u8 write2dispbuf_en );  /* Peter: 0707 */

int mpeg4ModifyTargetBitRate(int NewBitRate);


/* yc:0814 S */
void  Output_Sem(void);
/* yc:0814 E */


/*

Routine Description:

    Resume MPEG4 task.

Arguments:

    None.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mpeg4ResumeTask(void)
{
    /* Resume the task */
    //DEBUG_MP4("Trace: MPEG4 task resuming.\n");
  
    OSTaskResume(MPEG4_TASK_PRIORITY); 
    
    return 1;
}

/*

Routine Description:

    Suspend MPEG4 task.

Arguments:

    None.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mpeg4SuspendTask(void)
{
    /* Suspend the task */
    //DEBUG_MP4("Trace: MPEG4 task suspending.\n");
	MPEG4_Task_Go   = 0;    // 0: never run, 1: ever run  //Lsk 090622
    OSTaskSuspend(MPEG4_TASK_PRIORITY);
    
    return 1;
}

/*

Routine Description:

    Set video quality.

Arguments:

    quality - Video quality.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mpeg4SetVideoQuality(u8 quality)
{
    mpeg4VideoRecQulity = quality;
    return 1;
}


s32 mpeg4SetVideoFrameRate(u8 framerate)
{
    int i;

    VideoRecFrameRate   = framerate;

#if (MULTI_CHANNEL_VIDEO_REC)
    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        MultiChannelMpeg4SetVideoFrameRate(framerate, &VideoClipOption[i]);
    }
#endif
    
    return 1;
}

/*

Routine Description:

    Set video resolution.

Arguments:

    width - Video width.
    height - Video height.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mpeg4SetVideoResolution(u16 width, u16 height)
{
    u32 temp, mbNoSize, mbNo;       /*CY 0907*/
    u32 mbWidth, mbHeight;          /*CY 0907*/
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    u32 mpeg4VdPacketSize;
#endif
    
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif
    
    mpeg4Width = (u32) width;       /*CY 0907*/
    mpeg4Height = (u32) height;     /*CY 0907*/
    
    //0x0100
    mbWidth             = (u32) ((mpeg4Width + 15) >> 4);    /*CY 0907*/
    mbHeight            = (u32) ((mpeg4Height + 15) >> 4);   /*CY 0907*/
    mbNo = mbWidth * mbHeight;      /*CY 0907*/
    Mpeg4FrameSize      = (mbWidth << mbWidthShft) | 
                          (mbHeight << mbHeightShft) |
                          (mbNo << mbNoShft);   /*CY 0907*/
    
    // 0x0200
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
    mpeg4VdPacketSize = MPEG4_VDPACKET_SIZE;
    Mpeg4ErrResil = mpeg4VdPacketSize | MPEG4_RESY_ENA;
//    Mpeg4ErrResil = MPEG4_VDPACKET_SIZE | MPEG4_RESY_ENA;
#endif
    // 0x0204
    mbNoSize    = 0;           /*CY 0907*/ 
    temp = mbNo;            /*CY 0907*/
    while(temp > 0)
    {
        temp  >>= 1;
        mbNoSize++;     /*CY 0907*/
    }
    Mpeg4DecVidPkt  = mbNoSize;  /*CY 0907*/
    
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif

    return  1;
}

void mpeg4ConfigQualityFrameRate(int BitRateLevel)
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
    //DEBUG_MP4("\nBitLevel=%d\n",BitRateLevel);

    mpRateControlParam.max_Qp=RC_MAX_QP;                  //max Qp
	mpRateControlParam.min_Qp=RC_MIN_QP;                   //min Qp
    if( (mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
    {
        switch(VideoRecFrameRate)
        {
        case MPEG4_VIDEO_FRAMERATE_60:
            mpRateControlParam.enable_ratecontrol   = 1;
            mpRateControlParam.Framerate  = 60;               //Target Framerate

            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                mpRateControlParam.TargetBitrate    = 666 * 1000; // 2.99 Mb/sec
                mpRateControlParam.InitQP=6;
                mpRateControlParam.QP_I=6;
                mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                mpRateControlParam.TargetBitrate    = 566 * 1000; // 2.21 Mb/sec
                mpRateControlParam.InitQP=7;
                mpRateControlParam.QP_I=7;
                mpRateControlParam.QP_P=7;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                mpRateControlParam.TargetBitrate    = 333 * 1000; // 1.33 Mb/sec
                mpRateControlParam.InitQP=8;
                mpRateControlParam.QP_I=8;
                mpRateControlParam.QP_P=8;
            break;
            }
            
            
            break;
        case MPEG4_VIDEO_FRAMERATE_30:
            mpRateControlParam.enable_ratecontrol   = 1;
          #if ((Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601)) //TV-in
            if(sysTVinFormat == TV_IN_NTSC)
               mpRateControlParam.Framerate  = 30;               //Target Framerate
            else
               mpRateControlParam.Framerate  = 25;               //Target Framerate
          #else //Sensor-in
               mpRateControlParam.Framerate  = 30;  
          #endif

            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                mpRateControlParam.TargetBitrate    = 666 * 1000; // 2.99 Mb/sec
                mpRateControlParam.InitQP=6;
                mpRateControlParam.QP_I=6;
                mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                mpRateControlParam.TargetBitrate    = 566 * 1000; // 2.21 Mb/sec
                mpRateControlParam.InitQP=7;
                mpRateControlParam.QP_I=7;
                mpRateControlParam.QP_P=7;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                mpRateControlParam.TargetBitrate    = 333 * 1000; // 1.33 Mb/sec
                mpRateControlParam.InitQP=8;
                mpRateControlParam.QP_I=8;
                mpRateControlParam.QP_P=8;
            break;
            }
        break;

        case MPEG4_VIDEO_FRAMERATE_15:
            mpRateControlParam.enable_ratecontrol   = 1;
         #if ((Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601)) //TV-in
            if(sysTVinFormat == TV_IN_NTSC)
               mpRateControlParam.Framerate  = 15;               //Target Framerate
            else
               mpRateControlParam.Framerate  = 12;               //Target Framerate
         #else //Sensor-in
              mpRateControlParam.Framerate  = 15;
         #endif

            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                mpRateControlParam.TargetBitrate    = 266 * 1000;   // 1.01 Mb/sec
                mpRateControlParam.InitQP=6;
                mpRateControlParam.QP_I=6;
                mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                mpRateControlParam.TargetBitrate    = 200 * 1000;
                mpRateControlParam.InitQP=8;
                mpRateControlParam.QP_I=8;
                mpRateControlParam.QP_P=8;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                mpRateControlParam.TargetBitrate    = 133 * 1000;
                mpRateControlParam.InitQP=10;
                mpRateControlParam.QP_I=10;
                mpRateControlParam.QP_P=10;
            break;
        }
        break;

		case MPEG4_VIDEO_FRAMERATE_10:
			mpRateControlParam.enable_ratecontrol	= 0;
			switch(mpeg4VideoRecQulity)
			{
			case MPEG4_VIDEO_QUALITY_HIGH:
				dftMpeg4Quality = 7;
			break;
		
			case MPEG4_VIDEO_QUALITY_MEDIUM:
				dftMpeg4Quality = 12;
			break;
		
			case MPEG4_VIDEO_QUALITY_LOW:
				dftMpeg4Quality = 17;
			break;
		   }
		   break;
		
        case MPEG4_VIDEO_FRAMERATE_5:
            mpRateControlParam.enable_ratecontrol   = 0;
            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                dftMpeg4Quality = 7;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                dftMpeg4Quality = 12;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                dftMpeg4Quality = 17;
            break;
            }
        break;
        }

        switch(BitRateLevel)
        {
           case MPEG_BITRATE_LEVEL_100:
            break;

           case MPEG_BITRATE_LEVEL_80:
                mpRateControlParam.TargetBitrate=mpRateControlParam.TargetBitrate*80/100;
                mpRateControlParam.InitQP +=3;
                mpRateControlParam.QP_I   +=3;
                mpRateControlParam.QP_P   +=3;
	            mpRateControlParam.min_Qp =10;                   //min Qp
            break;

           case MPEG_BITRATE_LEVEL_60:
                mpRateControlParam.TargetBitrate=mpRateControlParam.TargetBitrate*60/100;
                mpRateControlParam.InitQP +=8;
                mpRateControlParam.QP_I   +=8;
                mpRateControlParam.QP_P   +=8;
                mpRateControlParam.min_Qp  =15;                   //min Qp
            break;
        }
    }
    else  //VGA size//
    {
        switch(VideoRecFrameRate)
        {
        case MPEG4_VIDEO_FRAMERATE_60:
            //Cannot support VGA 60 fps,Now
        case MPEG4_VIDEO_FRAMERATE_30:
            mpRateControlParam.enable_ratecontrol   = 1;
        #if ((Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601)) //TV-in
            if(sysTVinFormat == TV_IN_NTSC)
               mpRateControlParam.Framerate  = 30;               //Target Framerate
            else
               mpRateControlParam.Framerate  = 25;               //Target Framerate
        #else //sensor-in
              mpRateControlParam.Framerate  = 30; 
        #endif

            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                  mpRateControlParam.TargetBitrate    = 2000 * 1000; // 2.99 Mb/sec
                  mpRateControlParam.InitQP=6;
                  mpRateControlParam.QP_I=6;
                  mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                mpRateControlParam.TargetBitrate    = 1700 * 1000; // 2.21 Mb/sec
                mpRateControlParam.InitQP=8;
                mpRateControlParam.QP_I=8;
                mpRateControlParam.QP_P=8;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                mpRateControlParam.TargetBitrate    = 1000 * 1000; // 1.33 Mb/sec
                mpRateControlParam.InitQP=10;
                mpRateControlParam.QP_I=10;
                mpRateControlParam.QP_P=10;
            break;
            }
        break;

        case MPEG4_VIDEO_FRAMERATE_15:
            mpRateControlParam.enable_ratecontrol   = 1;
        #if ((Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601)) //TV-in
            if(sysTVinFormat == TV_IN_NTSC)
               mpRateControlParam.Framerate  = 15;               //Target Framerate
            else
               mpRateControlParam.Framerate  = 12;               //Target Framerate
        #else //sensor-in
            mpRateControlParam.Framerate  = 15;
        #endif

        #if(FPGA_BOARD_A1018_SERIES)
            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                mpRateControlParam.TargetBitrate    = 2000 * 1000;   
                mpRateControlParam.InitQP=6;
                mpRateControlParam.QP_I=6;
                mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                mpRateControlParam.TargetBitrate    = 1700 * 1000;
                mpRateControlParam.InitQP=8;
                mpRateControlParam.QP_I=8;
                mpRateControlParam.QP_P=8;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                mpRateControlParam.TargetBitrate    = 1000 * 1000;
                mpRateControlParam.InitQP=10;
                mpRateControlParam.QP_I=10;
                mpRateControlParam.QP_P=10;
            break;
            }        
         #else
            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                mpRateControlParam.TargetBitrate    = 800 * 1000;   // 1.01 Mb/sec
                mpRateControlParam.InitQP=6;
                mpRateControlParam.QP_I=6;
                mpRateControlParam.QP_P=6;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                mpRateControlParam.TargetBitrate    = 600 * 1000;
                mpRateControlParam.InitQP=8;
                mpRateControlParam.QP_I=8;
                mpRateControlParam.QP_P=8;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                mpRateControlParam.TargetBitrate    = 400 * 1000;
                mpRateControlParam.InitQP=10;
                mpRateControlParam.QP_I=10;
                mpRateControlParam.QP_P=10;
            break;
            }
         #endif   
        break;

         case MPEG4_VIDEO_FRAMERATE_10:
			 mpRateControlParam.enable_ratecontrol	 = 0;
			 switch(mpeg4VideoRecQulity)
			 {
			 case MPEG4_VIDEO_QUALITY_HIGH:
				 dftMpeg4Quality = 7;
			 break;
		 
			 case MPEG4_VIDEO_QUALITY_MEDIUM:
				 dftMpeg4Quality = 12;
			 break;
		 
			 case MPEG4_VIDEO_QUALITY_LOW:
				 dftMpeg4Quality = 17;
			 break;
		 	}
        	break;

        case MPEG4_VIDEO_FRAMERATE_5:
            mpRateControlParam.enable_ratecontrol   = 0;
            switch(mpeg4VideoRecQulity)
            {
            case MPEG4_VIDEO_QUALITY_HIGH:
                dftMpeg4Quality = 7;
            break;

            case MPEG4_VIDEO_QUALITY_MEDIUM:
                dftMpeg4Quality = 12;
            break;

            case MPEG4_VIDEO_QUALITY_LOW:
                dftMpeg4Quality = 17;
            break;
        }
        break;
        }

        switch(BitRateLevel)
        {
           case MPEG_BITRATE_LEVEL_100:
            break;

           case MPEG_BITRATE_LEVEL_80:
               mpRateControlParam.TargetBitrate=mpRateControlParam.TargetBitrate*80/100;
               mpRateControlParam.InitQP +=3;
               mpRateControlParam.QP_I   +=3;
               mpRateControlParam.QP_P   +=3;
               mpRateControlParam.min_Qp  =10;                   //min Qp
            break;

           case MPEG_BITRATE_LEVEL_60:
               mpRateControlParam.TargetBitrate=mpRateControlParam.TargetBitrate*60/100;
               mpRateControlParam.InitQP +=8;
               mpRateControlParam.QP_I   +=8;
               mpRateControlParam.QP_P   +=8;
               mpRateControlParam.min_Qp  =15;                   //min Qp
            break;
        }
    }

    //Lsk 090626
#if(USE_PROGRESSIVE_SENSOR == 1) //Use progressive-scan sensor like CMOS
    video_double_field_flag = 0;
#else //Use interlace-scan sensor like CCD
    if((mpeg4Width != 320) &&((VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_5) || (VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_10) || (VideoRecFrameRate == MPEG4_VIDEO_FRAMERATE_15 && mpeg4VideoRecQulity == MPEG4_VIDEO_QUALITY_LOW))) 
    {
        video_double_field_flag = 1; //Lucian: 低frame rate 用double field.
    }
  #if (DE_INTERLACE_SEL == DOUBLE_FIELD_ON) //For Cwell, 全部用double field.
    else if( mpeg4Width != 320 )
    {
      video_double_field_flag = 1;
    }
  #elif (DE_INTERLACE_SEL == DOUBLE_FIELD_OFF) //For Aurum, 全部不用double field.
    else if( mpeg4Width != 320 )
    {
      video_double_field_flag = 0;
    }
  #else
    else if( mpeg4Width != 320 ) //Auto mode: update by SAD in mpeg4 engine.
    {
       video_double_field_flag = 0;
    }
  #endif
    else //for QVGA
        video_double_field_flag = 0;
#endif  

    TVout_Generate_Pause_Frame = 0;
    #if (((Sensor_OPTION == Sensor_CCIR601)||(Sensor_OPTION == Sensor_CCIR656) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)) && ( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_D1) ) )
    {
        //Lsk can't sure current playback file quality and frame-rate(video_double_field_flag) setting
        if(sysVideoInSel == VIDEO_IN_TV)
           TVout_Generate_Pause_Frame = 1;
    }
    #endif
    
    ASF_set_interlace_flag = 0;
    #if (USE_PROGRESSIVE_SENSOR == 0)
    {
        if((video_double_field_flag==0)&&(mpeg4Width != 320))
            ASF_set_interlace_flag = 1;
    }
    #endif
    
    if(mpRateControlParam.enable_ratecontrol)
	   RCQ2_init(&mpRateControlParam); //Lsk 090608
}

/*

Routine Description:

    Set mpeg4 picture coding type.

Arguments:

    frame_idx - the frame idex in a coding sequence.
    Period  - the priod for coding an I frame.

Return Value:

    0 - P frame.
    1 - I frame.

*/
u32 DeterminePictureType(u32 frame_idx, u32 period)
{
    u32 diff;
    
    #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    #endif
    //------//

#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
    (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) ||\
    (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == DVP_RF_SELFTEST))

#else    
    #if(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_ASF)                               
    //calulate time by video time, to get exactly asf file time
    if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && SetIVOP)    
    {
        DEBUG_MP4("MPEG4 calulate time, start index = %d\n",VideoBufMngWriteIdx);        
        OS_ENTER_CRITICAL();
        Cal_FileTime_Start_Idx = VideoBufMngWriteIdx; 
        VideoTimeStatistics = 0;
        SetIVOP = 0;
        OS_EXIT_CRITICAL();
        asfVideoFrameCount += period;
        
        return FLAG_I_VOP;
    }        
    #elif(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_AVI) 
    if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) 
    {
        DEBUG_MP4("MPEG4 calulate time, start index = %d\n",VideoBufMngWriteIdx);        
        if((WantChangeFile == 1) && (GetLastVideo == 1) && (VideoBufMngWriteIdx == ((LastVideo + 1) % VIDEO_BUF_NUM))) 
        {
            asfVideoFrameCount += period;
            return FLAG_I_VOP;
        }
    }
    #endif
    
    if((asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && SetIVOP)
    {
        DEBUG_MP4("MPEG4 calulate time, start index = %d\n",VideoBufMngWriteIdx);
        
        OS_ENTER_CRITICAL();
        Cal_FileTime_Start_Idx = VideoBufMngWriteIdx; 
        VideoTimeStatistics = 0;
        SetIVOP = 0;
        OS_EXIT_CRITICAL();
        asfVideoFrameCount += period;
        return FLAG_I_VOP;
    }
#endif   
    /* I picture  */
    diff= (asfVideoFrameCount >= frame_idx) ? (asfVideoFrameCount-frame_idx) : (frame_idx-asfVideoFrameCount) ;
    if(diff > 1000)
    {
        DEBUG_MP4("Determine I frame overflow!(%d,%d)\n",asfVideoFrameCount,frame_idx);
        asfVideoFrameCount = frame_idx;
        return FLAG_P_VOP;
    }

    if(period == 0)
    {
        asfVideoFrameCount = frame_idx;
        return FLAG_I_VOP;
    }
    else if( frame_idx >= asfVideoFrameCount ) 
    {   
        asfVideoFrameCount += period;
        if(frame_idx >= asfVideoFrameCount)
        {
             DEBUG_MP4("FC overflow:%d,%d\n",frame_idx,asfVideoFrameCount);
        }
            
        return FLAG_I_VOP;
    }    
    else 
    { /* P picture */
        return FLAG_P_VOP;
    }

}

s32 mpeg4GetVideoResolution(int *pwidth, int *pheight)
{
   *pwidth =mpeg4Width;
   *pheight=mpeg4Height;

    return 1;
}

int mpeg4ModifyTargetBitRate(int NewBitRate)
{
     mpRateControlParam.RCQ2_config.bit_rate=NewBitRate;
     mpRateControlParam.RCQ2_config.target_rate=NewBitRate/mpRateControlParam.Framerate;
     
     return 1;
}

#if RFIU_SUPPORT
int rfiuGetTxBitRate(int SizeLevel)
{
    static int lastP2PTxBitRate = -1;
#if(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
    int TxBitRate = 0;
    int pbuf_used_num;
    if(sys9211TXWifiStat==MR9211_ENTER_WIFI){
		pbuf_used_num = check_pbuf_mem();
        if(rfiu_TX_WifiPower <=50 && pbuf_used_num <= 50){
            P2PTxBitRate[0] = 1701;
        }
        else if(rfiu_TX_WifiPower<=56 && pbuf_used_num <= 100){
            P2PTxBitRate[0] = 1100;
        }
        else if(rfiu_TX_WifiPower<=62 && pbuf_used_num <= 200){
            P2PTxBitRate[0] = 600;
        }
        else{
            P2PTxBitRate[0] = 300;
        }
    }
    else
        P2PTxBitRate[0] = 0;
    
    if((RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0)) && (sys9211TXWifiStat==MR9211_ENTER_WIFI)) // both RF and wifi connected
        TxBitRate = ((P2PTxBitRate[0])<(rfiuTXBitRate[0]))? (P2PTxBitRate[0]):(rfiuTXBitRate[0]);
    else if((sys9211TXWifiStat==MR9211_ENTER_WIFI)) // wifi connected only
        TxBitRate = P2PTxBitRate[0];
    else // RF connected only
        TxBitRate = rfiuTXBitRate[0];

    return TxBitRate;
    
#else
    return rfiuTXBitRate[0];
#endif

}


int rfiuGetTxBufFullness(){
#if(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
    int TxBufFullness = 0;
    static u8 continusHighBuf = 0;
    if(sys9211TXWifiStat==MR9211_ENTER_WIFI){
        if(VideoBufMng[P2PVideoBufReadIdx[0]].buffer < VideoBufMng[VideoBufMngWriteIdx].buffer){
            P2PTxBufFullness[0] = VideoBufMng[VideoBufMngWriteIdx].buffer - VideoBufMng[P2PVideoBufReadIdx[0]].buffer;
        }
        else{
            P2PTxBufFullness[0] = MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE - (VideoBufMng[P2PVideoBufReadIdx[0]].buffer - VideoBufMng[VideoBufMngWriteIdx].buffer);
    	}
    }
    else
        P2PTxBufFullness[0] = 0;
    
    P2PTxBufFullness[0] = P2PTxBufFullness[0] / 1000;

    if((RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0)) && (sys9211TXWifiStat==MR9211_ENTER_WIFI)) // both RF and wifi connected
        TxBufFullness = ((P2PTxBufFullness[0])>(rfiuTxBufFullness[0]))? (P2PTxBufFullness[0]):(rfiuTxBufFullness[0]);    
    if((sys9211TXWifiStat==MR9211_ENTER_WIFI)) // wifi connected only
        TxBufFullness = P2PTxBufFullness[0];
    else  // RF connected only
        TxBufFullness = rfiuTxBufFullness[0];

    return TxBufFullness;
    
#else
    return rfiuTxBufFullness[0];
#endif

}


#if (RATE_CONTROL_MODE == BALANCE_MODE || RATE_CONTROL_MODE == SMOOTHFRAME_MODE)
#define TX_BUFFCNTL_THR_IN_MS  500
s32 rfiuTxRateControl_LiveMode2(
                                             int TxBufFullness,
                                             unsigned long EncBitRate,
                                             unsigned int *pVideoCnt,
                                             int *pBitRateRatioPercent,
                                             s32 *DropExtraFrame
                                        )
{
    int BufTime;
    int EncByteRate;
    int MaxBufFullness;
    
    if( EncBitRate == 0 )
        return 0;

    EncByteRate = (EncBitRate/8);
	
    if( EncByteRate == 0 )
        return 0;	

    MaxBufFullness = rfiuGetTxBufFullness();
    
    BufTime = (MaxBufFullness *1000)/EncByteRate;
    if( RFIU_TX_MODE != rfiu_CheckTxTaskMode(RFI_UNIT_0)){
        *pBitRateRatioPercent = 0;
        return 0;
    }
    if(mpRateControlParam.enable_ratecontrol)
    {
        if( *pVideoCnt > 1)
        {
            *pVideoCnt=0;
            if(BufTime > TX_BUFFCNTL_THR_IN_MS+3000)
            {
               *pBitRateRatioPercent=10;
	         *DropExtraFrame = 5;
            }            
            else if(BufTime > TX_BUFFCNTL_THR_IN_MS+2400)
            {
               *pBitRateRatioPercent=20;
	         *DropExtraFrame = 3;
            }            
            else if(BufTime > TX_BUFFCNTL_THR_IN_MS+1800)
            {
               *pBitRateRatioPercent=30;
		   *DropExtraFrame = 1;
            }
            else if(BufTime > TX_BUFFCNTL_THR_IN_MS+1500)
            {
                 *pBitRateRatioPercent=40;
                 *DropExtraFrame = 0;
            }
            else if(BufTime > TX_BUFFCNTL_THR_IN_MS+1200)
            {
                 *pBitRateRatioPercent=50;
                 *DropExtraFrame = 0;
            }
            else if(BufTime > TX_BUFFCNTL_THR_IN_MS+900)
            {
                 *pBitRateRatioPercent=60;
                 *DropExtraFrame = 0;
            }
            else if(BufTime > TX_BUFFCNTL_THR_IN_MS+600)
            {
                 *pBitRateRatioPercent=70;
                 *DropExtraFrame = 0;
            }
            else if(BufTime > TX_BUFFCNTL_THR_IN_MS+300)
            {
                 *pBitRateRatioPercent=80;
                 *DropExtraFrame = 0;
            }
            else if(BufTime > TX_BUFFCNTL_THR_IN_MS)
            {
                  *pBitRateRatioPercent=90;
                  *DropExtraFrame = 0;
            }  
            else
            {
               *pBitRateRatioPercent=100;
               *DropExtraFrame = 0;			   
            }            
        }
    }
    return 0;
}

void rfiuTXMpeg4EncTask(void* pData)
{
    u32*        pFlag;
    s64*        pTime;
    u32*        pSize;
    u32*        pOffset;
    u8          *pBuf, *pReadBuf;
    u8          err;
    u32         NextIdx;
    u32         outLen;
    s32         i, DropFrame;
    int         SizeLevel;
    int         TxBufFullness;
    int         TotalDropFrame;
    int         DropFrameInterval, CntToDrop;
    int         Drop_sub;
    int         QP_OverInc;
#if RF_TX_OPTIMIZE      
    int         RFUnit;
#endif
    //Lucian: 多Tx時,要改為Global Array. 
    s32 Drop_ext=0, DropExtraFrame = 0;
    int  QP_ext2=0;
    u32  Mpeg4EncCnt = 0; //Lucina: for internal use: roundtype, reconstruct frame switch.
    u32  TXErrMonitor=0;
    u32  PrevTXRunCnt=0xffffffff;
    int  BitRateRatioPercent;
    //----//
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    unsigned int ByteCnt,ByteCnt_sub;
    unsigned long t1,t2,dt,RealBitRate,RealBitRate_sub;
    unsigned int VideoCnt;
    unsigned int AvgFrameSize;
    unsigned int EncFrameCnt,AvgFrameRate;
    int      QPsub_ext;
    unsigned int SubTargetBR;
    unsigned int EncodeTime;
    int VideoBufferSize;
    u8 RateControl_PIRRecOn, low_buffer_count;
    u32 max_qp_for_fullhd = RC_MAX_QP;
    unsigned long EncByteRate;
    int pre_rf_bitrate;
    int TxBitRate, MaxBufFullness = 0, wifi_offset = 0;
    //---------------------------//

#if RF_TX_OPTIMIZE  
    RFUnit= (int)pData;
    VideoBufMngWriteIdx=0;
    VideoPictureIndex  = 0;
    asfVideoFrameCount=0;
    gRfiu_MpegEnc_Sta[RFUnit]=RFI_MPEGENC_TASK_RUNNING;
#endif
    MPEG4_Task_Go   = 1;    // 0: never run, 1: ever run


    //====Set Initial Bit Rate====//
    mpRateControlParam.enable_ratecontrol = 0;

#if(SW_APPLICATION_OPTION  == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))	
    rfiuTXBitRate[0]=iconflag[UI_MENU_SETIDX_TX_LAST_BIT_RATE]*100; //Unit: Kbps, control at range 800~2000
#else
    rfiuTXBitRate[0] = 1700;
#endif

    pre_rf_bitrate = rfiuTXBitRate[0];
	
    if(mpeg4Width <= 360) //QVGA or CIF
    {
       SizeLevel=MPEG4_REC_QVGA;
    }
    else if(mpeg4Width<=720) //VGA or D1
    {
       SizeLevel=MPEG4_REC_QHD;
	 if(rfiuTXBitRate[0] > 1500)
	       mpRateControlParam.QP_P = 24;
	 else
	 	mpRateControlParam.QP_P = 30;
    }
    else if(mpeg4Width<=1440) //HD or 4D1
    {
       SizeLevel=MPEG4_REC_HD;
	 if(rfiuTXBitRate[0] > 1500)
	       mpRateControlParam.QP_P = 26;
	 else
	 	mpRateControlParam.QP_P = 32;	   
    }
    else if(mpeg4Width<=2200) //FHD
    {
       SizeLevel=MPEG4_REC_FULLHD;
	 if(rfiuTXBitRate[0] > 1500)
	       mpRateControlParam.QP_P = 29;
	 else
	 	mpRateControlParam.QP_P = 35;
    }
    else
    {
       SizeLevel=MPEG4_REC_QHD;
	 if(rfiuTXBitRate[0] > 1500)
	       mpRateControlParam.QP_P = 24;
	 else
	 	mpRateControlParam.QP_P = 30;
    }
    //====//
    rfiuTxBufFullness[0]=0;
    ByteCnt=0;
    ByteCnt_sub=0;
    DropFrame=0;
    QP_ext2=0;
    QPsub_ext=0;
    VideoCnt=0;
    EncFrameCnt=0;
    Drop_ext=0;
    EncodeTime=0;
    BitRateRatioPercent=100;
    RateControl_PIRRecOn = 1;
    DropFrameInterval = 0;
    mpRateControlParam.Int_prev_PMad = (8*4)*1024;  //FixRC
    AvgFrameSize=(rfiuTXBitRate[0]-140)*100/100*9/10/8/( rfiuVideoInFrameRate/(1+0) );

    timerCountRead(guiRFTimerID, &t1);

    DEBUG_MP4("\n-TXMpg4EncTsk-\n");
  #if RF_TX_OPTIMIZE 
      gRfiuUnitCntl[RFUnit].TX_MpegEnc_StopRdy=0;
  #endif
    while (1)
    {
    #if RF_TX_OPTIMIZE  
        if(gRfiuUnitCntl[RFUnit].TX_MpegEnc_Stop)
        {
           DEBUG_MP4("#");
           gRfiuUnitCntl[RFUnit].TX_MpegEnc_StopRdy=1;
           OSTimeDly(1);
           continue;
        }
        gRfiuUnitCntl[RFUnit].TX_MpegEnc_StopRdy=0;
    #else
    
        Video_Pend  = 1;
        OSSemPend(VideoTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
        Video_Pend  = 0;
    
        if (err != OS_NO_ERR)
        {
            DEBUG_MP4("Error: VideoTrgSemEvt is %d.\n", err);
            //return;
        }
    #endif
        
        if(mpeg4Width <= 360) //QVGA or CIF
	    {
	       SizeLevel=MPEG4_REC_QVGA;
	    }
	    else if(mpeg4Width<=720) //VGA or D1
	    {
	       SizeLevel=MPEG4_REC_QHD;
	    }
	    else if(mpeg4Width<=1440) //HD or 4D1
	    {
	       SizeLevel=MPEG4_REC_HD;
	    }
        else if(mpeg4Width<=2200) //FHD
	    {
	       SizeLevel=MPEG4_REC_FULLHD;
	    }
	    else
	    {
	       SizeLevel=MPEG4_REC_QHD;
	    }
        
        pBuf        = VideoBufMng[VideoBufMngWriteIdx].buffer;  /*BJ 0530 S*/
        pFlag       = &VideoBufMng[VideoBufMngWriteIdx].flag; 
        pTime       = &VideoBufMng[VideoBufMngWriteIdx].time; 
        pSize       = &VideoBufMng[VideoBufMngWriteIdx].size; 
        pOffset     = &VideoBufMng[VideoBufMngWriteIdx].offset; 

        if(RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))  
        {
               if(SizeLevel==MPEG4_REC_HD)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
               else if(SizeLevel==MPEG4_REC_FULLHD)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD/2);   // I: 1, P: 0
               else
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0

               if( rfiuRX_OpMode & RfIU_RX_OPMODE_FORCE_I )
               {
                   *pFlag=1;
                   rfiuRX_OpMode=rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);
                   DEBUG_MP4("\n-->Force I \n");
               }
               else if(EncFrameCnt == 0) // first encode frame must be I frame
                    *pFlag=1;
        }
     #if TX_PIRREC_SUPPORT
        else if(rfiuBatCamPIRTrig)
        {
               if(SizeLevel==MPEG4_REC_HD)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
               else if(SizeLevel==MPEG4_REC_FULLHD)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD/2);   // I: 1, P: 0
               else
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0

               if( rfiuRX_OpMode & RfIU_RX_OPMODE_FORCE_I )
               {
                   *pFlag=1;
                   rfiuRX_OpMode=rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);
                   DEBUG_MP4("\n-->Force I \n");
               }
               else if(EncFrameCnt == 0) // first encode frame must be I frame
                    *pFlag=1;
        }
     #endif
     #if(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
        //else if(sys9211TXWifiStat==MR9211_ENTER_WIFI) // when Link broken.
        else if(1)
        {
            if(rfiu_TX_P2pVideoQuality == RFWIFI_P2P_QUALITY_HIGH)
            {
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
            }
            else if(rfiu_TX_P2pVideoQuality == RFWIFI_P2P_QUALITY_MEDI)
            {
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
            }
            else //Qaulity Low
            {
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
            }
        }
     #endif     
        else
        {
            *pFlag      = DeterminePictureType(VideoPictureIndex, 0);   // I: 1, P: 0
             mpeg4MultiStreamStart=1;
        }
         
        Vop_Type    = (*pFlag)? I_VOP : P_VOP; // 0: I frame, 1: P frame
     
        if(Vop_Type == I_VOP) 
        {     // I frame
            if(mpeg4MultiStreamStart)
                mpeg4MultiStreamEnable=1;
            else
                mpeg4MultiStreamEnable=0;
            
            timerCountRead(guiRFTimerID, &t2);
             if(t1>t2)
              dt=t1-t2;
            else
              dt=(t1+TimerGetTimerCounter(TIMER_7))-t2;

            if(dt==0)
                dt=1;
            //DEBUG_MP4("dt=%d,Cnt=%d\n",dt,ByteCnt);
            RealBitRate = ((ByteCnt+64)*8*10/dt); //Lucian: Video + Header 
            AvgFrameRate=EncFrameCnt*10000/dt;

         #if MULTI_STREAM_SUPPORT  
             #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
                  (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || \
                  (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
                 Drop_sub=1;
                 QPsub_ext=0;
                 RealBitRate_sub = (ByteCnt_sub*8*10/dt); //Lucian: Video + Header
             #elif( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM))
                 if(mpeg4MultiStreamEnable)
                 {
                    if(AvgFrameRate >= (TARGET_FR_SUBSTREAM*2) )
                    {
                        Drop_sub= (AvgFrameRate+TARGET_FR_SUBSTREAM/2)/TARGET_FR_SUBSTREAM;
                    }
                    else if( AvgFrameRate >= (TARGET_FR_SUBSTREAM*3/2) )
                    {
                        Drop_sub=2;
                    }
                    else
                       Drop_sub=1;
                    
                    RealBitRate_sub = (ByteCnt_sub*8*10/dt); //Lucian: Video + Header
                    
                    if(gRfiuUnitCntl[0].RFpara.TX_SubStreamBRSel==RF_SUBSTREAM_BR_H)
                       SubTargetBR=TARGET_BR_SUBSTREAM_H;
                    else if(gRfiuUnitCntl[0].RFpara.TX_SubStreamBRSel==RF_SUBSTREAM_BR_M)
                       SubTargetBR=TARGET_BR_SUBSTREAM_M;
                    else
                       SubTargetBR=TARGET_BR_SUBSTREAM_L;
                    
                    if(RealBitRate_sub != 0)
                    {
                       if(mpRateControlParam.enable_ratecontrol)
                       {
                         if(RealBitRate_sub > SubTargetBR)
                            QPsub_ext ++;
                         else if(RealBitRate_sub < (SubTargetBR-15) )
                            QPsub_ext --;

                         if(QPsub_ext>5)
                            QPsub_ext = 5;
                         else if(QPsub_ext < -5)
                            QPsub_ext = -5;
                       }
                       else
                          QPsub_ext=0;
                    } 
                }
                else
                    Drop_sub=1;
            #endif
         #endif
            if( RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))
            {
                mpRateControlParam.QP_I = mpRateControlParam.QP_P + 4;
            }
         #if TX_PIRREC_SUPPORT   
            else if(rfiuBatCamPIRTrig)
            {
                mpRateControlParam.QP_I = mpRateControlParam.QP_P + 4;
            }
         #endif
            else
            {
                mpRateControlParam.QP_I = mpRateControlParam.QP_P + 4;
            }

            if(SizeLevel == MPEG4_REC_FULLHD){
                if(mpRateControlParam.QP_I > max_qp_for_fullhd)
                    mpRateControlParam.QP_I = max_qp_for_fullhd;
                else if(mpRateControlParam.QP_I < RC_MIN_QP_I)
                    mpRateControlParam.QP_I = RC_MIN_QP_I;                
            }
            else {
                if(mpRateControlParam.QP_I > RC_MAX_QP_I)
                    mpRateControlParam.QP_I = RC_MAX_QP_I;
                else if(mpRateControlParam.QP_I < RC_MIN_QP_I)
                    mpRateControlParam.QP_I = RC_MIN_QP_I;
            }
          
          
          #if MULTI_STREAM_SUPPORT     
            if(mpeg4MultiStreamEnable)
               DEBUG_MP4(" I:%d,%d,%d,%d,%d,%d,%d\n",AvgFrameRate,rfiuTxBufFullness[0],rfiuTXBitRate[0],RealBitRate,RealBitRate_sub, mpRateControlParam.QP_I, mpRateControlParam.QP_P);
            else
               DEBUG_MP4(" I:%d,%d,%d,%d,%d,%d\n",AvgFrameRate,rfiuTxBufFullness[0],rfiuTXBitRate[0],RealBitRate,mpRateControlParam.QP_I, mpRateControlParam.QP_P);
          #else
            DEBUG_MP4(" I:%d,%d,%d,%d,%d,%d\n",AvgFrameRate,rfiuTxBufFullness[0],rfiuTXBitRate[0],RealBitRate,mpRateControlParam.QP_I, mpRateControlParam.QP_P);
          #endif
          #if(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
            //DEBUG_MP4("WifiPow:%d, RF:%d, Wifi:%d, pbuf_used_num:%d, Wifi_ST:%d\n", rfiu_TX_WifiPower, rfiuTXBitRate[0], P2PTxBitRate[0], check_pbuf_mem(), sys9211TXWifiStat);
          #endif
            ByteCnt=0;
            ByteCnt_sub=0;
            //=====Lucian: 監視TX RF task是否當掉=====//
            if(Main_Init_Ready == 1)
            {
                if(gRfiuUnitCntl[0].RunCount == PrevTXRunCnt)
                    TXErrMonitor ++;
                else
                    TXErrMonitor=0;

                if(TXErrMonitor > 4)
                {
                   DEBUG_MP4("\n======== rfiu_Tx_Task_UnitX is Fatal Error! Reboot!======\n");
    			   sysForceWDTtoReboot();
                }
                PrevTXRunCnt=gRfiuUnitCntl[0].RunCount;
            }
            t1=t2;

            EncFrameCnt=0;
            VideoCnt=255;
            
        }
        else
        {   //P frame
            //-- 根據TxBuf fullness 來微調 Target bitrate--//
            //--------HD/Full HD--------//
           #if TX_PIRREC_SUPPORT 
             if(rfiuBatCamPIRTrig==0)
             {
                 rfiuTxRateControl_LiveMode2(
                                                  rfiuTxBufFullness[0],
                                                  RealBitRate,
                                                  &VideoCnt,
                                                  &BitRateRatioPercent,
                                                  &DropExtraFrame
                                           );
             }
             else
             {
                if(VideoBufMngReadIdx > 0)
		        {
                        rfiuTxRateControl_LiveMode2(
                                                  rfiuTxBufFullness[0],
                                                  RealBitRate,
                                                  &VideoCnt,
                                                  &BitRateRatioPercent,
                                                  &DropExtraFrame
                        );
                }
//                printf("[P frame] Buffer:%d, R/W:%d/%d, (R:%p  W:%p) (%d) drop:%d, QP_2:%d\n", rfiuTxBufFullness[0], VideoBufMngReadIdx, VideoBufMngWriteIdx, VideoBufMng[VideoBufMngReadIdx].buffer - VideoBuf, VideoBufMng[VideoBufMngWriteIdx].buffer - VideoBuf, VideoBufferSize, Drop_ext, QP_ext2);
             }
           #else
             rfiuTxRateControl_LiveMode2(
                                          rfiuTxBufFullness[0],
                                          RealBitRate,
                                          &VideoCnt,
                                          &BitRateRatioPercent,
                                          &DropExtraFrame
                                       );
           #endif
        }

        //--------Video rate control: Calculate QP-----//            
         if (Vop_Type == 0) //I frame
         {
             dftMpeg4Quality =mpRateControlParam.QP_I;
         }
         else //P frame
         {
             dftMpeg4Quality=mpRateControlParam.QP_P;
             video_double_field_flag = 0;
         }     
        //========================================//
        
     #if (MPEG4_FIELD_ENC_ENA && (CIU1_OPTION ==Sensor_CCIR656) )
   
     #elif (CIU1_BOB_REPLACE_MPEG_DF && (CHIP_OPTION != CHIP_A1016A) ) //Lucian: 不用 Mpeg4 double field,起動 ciu mob mode
       #if( CIU1_OPTION == Sensor_CCIR656 )
        if(video_double_field_flag)
        {
           CIU_1_CTL1= (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
           CIU_1_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
        }
        else
        {
           CIU_1_CTL1= (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656;
        } 
       #elif(CIU1_OPTION == Sensor_CCIR601)
        if(video_double_field_flag)
        {
           CIU_1_CTL1= (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
           CIU_1_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
        }
        else
        {
           CIU_1_CTL1= (CIU_1_CTL1 & (~0x30)) | CIU_MODE_601;
        } 
       #endif
     #endif

     #if(RF_TX_OPTIMIZE == 0)
        pReadBuf    = VideoBufMng[VideoBufMngReadIdx].buffer;   /* Peter 070130 */
        if(VideoPictureIndex != 0 && (pReadBuf >= pBuf)) 
        {
           #if DCF_WRITE_STATISTIC
              dcfMpegBufRemain=pReadBuf-pBuf;
           #endif
            while((pReadBuf >= pBuf) &&
                  ((pReadBuf - pBuf) < MPEG4_MIN_BUF_SIZE) && 
                  (VideoCmpSemEvt->OSEventCnt > 2) &&
                  (sysCaptureVideoStop == 0)) 
            {
                
                DEBUG_MP4("x");
                OSTimeDly(1);
                pReadBuf    = VideoBufMng[VideoBufMngReadIdx].buffer;
            }
        }
        else
        {
           dcfMpegBufRemain=MPEG4_MAX_BUF_SIZE-(pBuf-pReadBuf);
        }
     #endif        
        //---------- Video Encoding & Frame Rate control---------//
        *pTime=0;
    
        //======== Lucian: frame rate control =======//
//        TotalDropFrame=(DropFrame+Drop_ext);
//        DropFrameInterval=(DropFrame);

        
        //DEBUG_MP4("-->1\n");
        #if (MULTI_CHANNEL_SEL & 0x01)
        if(isuSemEvt->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! isuSemEvt=%d\n",isuSemEvt->OSEventCnt);
        }
        if(EncFrameCnt > 0){
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(isuSemEvt, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
        
            }
        }
        #elif (MULTI_CHANNEL_SEL & 0x02)
        if(ciuCapSemEvt_CH1->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! ciuCapSemEvt_CH1=%d\n",ciuCapSemEvt_CH1->OSEventCnt);
        }
        if(EncFrameCnt > 0){        
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH1, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
        
            }
        }
        #elif (MULTI_CHANNEL_SEL & 0x04)
        if(ciuCapSemEvt_CH2->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! ciuCapSemEvt_CH2=%d\n",ciuCapSemEvt_CH2->OSEventCnt);
        }
        if(EncFrameCnt > 0){
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH2, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
            }
        }
        #elif (MULTI_CHANNEL_SEL & 0x08)
        if(ciuCapSemEvt_CH3->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! ciuCapSemEvt_CH3=%d\n",ciuCapSemEvt_CH3->OSEventCnt);
        }
        if(EncFrameCnt > 0){
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH3, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
            }
        }
        #elif (MULTI_CHANNEL_SEL & 0x10)
        if(ciuCapSemEvt_CH4->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! ciuCapSemEvt_CH4=%d\n",ciuCapSemEvt_CH4->OSEventCnt);
        }
        if(EncFrameCnt > 0){
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH4, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
            }
        }
        #elif (MULTI_CHANNEL_SEL & 0x20)
        if(ciuCapSemEvt_CH5->OSEventCnt > 2)
        {
            DropFrame = 1;
            DEBUG_MP4("\n---->Warning! ciuCapSemEvt_CH5=%d\n",ciuCapSemEvt_CH5->OSEventCnt);
        }
        if(EncFrameCnt > 0){
	     if(DropExtraFrame > 0){
	            for(i = 0; i < (DropExtraFrame + DropFrame); i++){
	                OSSemPend(ciuCapSemEvt_CH5, 4 ,&err);
	                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
	                VideoPictureIndex++;
	                //VideoSmallPictureIndex++;
	                mp4_avifrmcnt++;
	                VideoCnt ++;
			   DropFrame = 0;
	            }
	     	}
		else
             {
                if(CntToDrop == 0 && DropFrameInterval!= 0)
                    CntToDrop = DropFrameInterval;
                
	            if((DropFrame) || (CntToDrop == 1))
	            {
	                OSSemPend(ciuCapSemEvt_CH5, 4 ,&err);
	                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
	                VideoPictureIndex++;
	                //VideoSmallPictureIndex++;
	                mp4_avifrmcnt++;
	                VideoCnt ++;
	                DropFrame = 0;
	            }
                if(CntToDrop > 0)
                    CntToDrop --;
		}
        }
        #endif

      //===================Encoding one Frame========================//
      //DEBUG_MP4("-->2\n");
      #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
        mpeg4Output1Frame(pBuf, pTime, pSize, &Mpeg4EncCnt, pOffset);
      #elif (VIDEO_CODEC_OPTION == H264_CODEC) 
        if(VideoPictureIndex>=START_VIDEO_INDEX)
        {
            video_info.StreamBuf = pBuf;
            video_info.FrameIdx  = VideoPictureIndex;  
            video_info.FrameTime = pTime;
            video_info.pSize     = pSize;  

            video_info.FrameType = *pFlag;
            video_info.Small_FrameType = *pFlag;
            
            H264Enc_cfg.qp = dftMpeg4Quality;
            H264Enc_cfg.slice_qp_delta =  H264Enc_cfg.qp - 26;
            #if MULTI_STREAM_SUPPORT
                if(  ((EncFrameCnt % Drop_sub) == 0) && mpeg4MultiStreamEnable )
                {
                    H264Enc_cfg.small_qp = dftMpeg4Quality ;    
                    H264Enc_cfg.small_qp += QPsub_ext;

                    if(mpRateControlParam.enable_ratecontrol)
                    {
                        if(H264Enc_cfg.small_qp > RC_MAX_QP_SUB)
                           H264Enc_cfg.small_qp=RC_MAX_QP_SUB;
                        else if(H264Enc_cfg.small_qp < RC_MIN_QP)
                           H264Enc_cfg.small_qp=RC_MIN_QP; 
                    }
                    else
                    {
                        if(H264Enc_cfg.small_qp > 45)
                           H264Enc_cfg.small_qp=45;
                    }
                        
                    H264Enc_cfg.small_slice_qp_delta = H264Enc_cfg.small_qp - 26;
                    
                    H264Enc_CompressOneFrame(&video_info,&H264Enc_cfg,1);   
                    VideoBufMng[VideoBufMngWriteIdx].offset = video_info.poffset; 
                    //VideoSmallPictureIndex++;
                 #if RF_TX_RATECTRL_DEBUG_ENA    
                    //DEBUG_MP4("SubQp=%d,%d\n",H264Enc_cfg.small_qp,(*pSize - *pOffset));
                 #endif
                }
                else
                {
                    H264Enc_CompressOneFrame(&video_info,&H264Enc_cfg,0);   // only big stream 
                    VideoBufMng[VideoBufMngWriteIdx].offset = video_info.poffset; 
                    //VideoSmallPictureIndex++;
                }

                EncFrameCnt ++;

            #else
                H264Enc_CompressOneFrame(&video_info,&H264Enc_cfg,0);   // only big stream 
                VideoBufMng[VideoBufMngWriteIdx].offset = video_info.poffset; 
                EncFrameCnt ++;
            #endif
        }
      else{
        #if (MULTI_CHANNEL_SEL & 0x01)
        OSSemPend(ciuCapSemEvt_CH1, 4 ,&err);
        #elif (MULTI_CHANNEL_SEL & 0x02)
        OSSemPend(ciuCapSemEvt_CH2, 4 ,&err);        
        #elif (MULTI_CHANNEL_SEL & 0x04)
        OSSemPend(ciuCapSemEvt_CH3, 4 ,&err);        
        #elif (MULTI_CHANNEL_SEL & 0x08)
        OSSemPend(ciuCapSemEvt_CH3, 4 ,&err);        
        #elif (MULTI_CHANNEL_SEL & 0x10)
        OSSemPend(ciuCapSemEvt_CH4, 4 ,&err);        
        #elif (MULTI_CHANNEL_SEL & 0x20)
        OSSemPend(ciuCapSemEvt_CH5, 4 ,&err);    
        #endif
        VideoPictureIndex++;
        continue;
      }
      #endif
        VideoCnt ++;
        //DEBUG_MP4("-->3\n");
        EncodeTime += *pTime;       
        //--------Rebuild RC model of Rate control ------//
        ByteCnt += *pSize;
     #if MULTI_STREAM_SUPPORT    
        ByteCnt_sub += (*pSize - *pOffset);
        //DEBUG_MP4("(%d,%d)",*pSize,(*pSize - *pOffset));
     #endif

	if(pre_rf_bitrate != rfiuTXBitRate[0]){
		if(last_3s_bitrate[3] < 0)
		 	rfiuTXBitRate[0] = pre_rf_bitrate;  //don't use the first 3 seconds bitrate
		else
			pre_rf_bitrate = rfiuTXBitRate[0];

		//enable rate control after 3 seconds
		if(last_3s_bitrate[3] >=0 && mpRateControlParam.enable_ratecontrol == 0)
			mpRateControlParam.enable_ratecontrol = 1;
		
		//save last bit rate
		last_3s_bitrate[3] = last_3s_bitrate[2];
		last_3s_bitrate[2] = last_3s_bitrate[1];
		last_3s_bitrate[1] = rfiuTXBitRate[0];
	}

        if(1) //must do the calculation of avg frame size, and adjust QP
        {
            //TotalSAD    = mpeg4Height*mpeg4Width*8;
            outLen      = *pSize; //Video size. 
            if( rfiuRX_OpMode & RFIU_RX_OPMODE_P2P)
            {
         	  if(rfiuRX_OpMode & RFIU_RX_OPMODE_P2P_MULTI)
         	  {
	                if( rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD)
	               {
	                    if(rfiuTXBitRate[0] > 1100)
	                    {
	                        rfiuTXBitRate[0] = 1100;
	                        pre_rf_bitrate = rfiuTXBitRate[0];
	                    }
	                }
	                else {
	                    if(rfiuTXBitRate[0] > 800)
	                    {
	                        rfiuTXBitRate[0] = 800;
	                        pre_rf_bitrate = rfiuTXBitRate[0];
	                    }
	                }
         	  }
	         else{
	                if( rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD){
	                    if(rfiuTXBitRate[0] > 2000)
	                    {
	                        rfiuTXBitRate[0] = 2000;
	                        pre_rf_bitrate = rfiuTXBitRate[0];
	                    }
	                }
	                else {
	                    if(rfiuTXBitRate[0] > 1500)
	                    {
	                        rfiuTXBitRate[0] = 1500;
	                        pre_rf_bitrate = rfiuTXBitRate[0];
	                    }
	                }
	         }
            }

            TxBitRate = rfiuGetTxBitRate(SizeLevel);
            
            if(TxBitRate > 130 && rfiuVideoInFrameRate > 1){
                if(DropFrameInterval > 1)
                    AvgFrameSize=(TxBitRate-130)*1024*BitRateRatioPercent/100*100/100/8/(rfiuVideoInFrameRate - rfiuVideoInFrameRate/DropFrameInterval);
                else
                {
                    AvgFrameSize=(TxBitRate-130)*1024*BitRateRatioPercent/100*100/100/8/(rfiuVideoInFrameRate);
                }    
            }
            
            if (Vop_Type ==I_VOP)  
            {
            //    if(mpRateControlParam.QP_I > mpRateControlParam.QP_P)
            //       QP_ext2= mpRateControlParam.QP_I-mpRateControlParam.QP_P;
            }
            else //P or B frame
            {
                mpRateControlParam.Avg_PMad=mpRateControlParam.Int_prev_PMad;

                if( (outLen) > (AvgFrameSize*2) )
                {
                    mpRateControlParam.QP_P +=3;
                }
                else if( (outLen) > (AvgFrameSize*15/10) )
                {
                    mpRateControlParam.QP_P +=2;
                }
                else if( (outLen) > AvgFrameSize)
                {
                    mpRateControlParam.QP_P +=1;
                }
                else if( (outLen) > (AvgFrameSize*9/10) )
                {
                }
                else{
                      mpRateControlParam.QP_P -= 1;
                }
            }
            
            {
                if(mpRateControlParam.QP_P > RC_MAX_QP)
                {
                    mpRateControlParam.QP_P = RC_MAX_QP;
                }
                else if(mpRateControlParam.QP_P < RC_MIN_QP)
                {
                    mpRateControlParam.QP_P = RC_MIN_QP;
                }
                
                if(mpRateControlParam.QP_P > START_DROP_FRAME_TRHESHOLD)
		  {
                    DropFrameInterval = (DROP_FRAME_INTERVAL_LEVEL + 2) - ( mpRateControlParam.QP_P - START_DROP_FRAME_TRHESHOLD );
                }
                else
                    DropFrameInterval = 0;
            }
        }
        
     #if RF_TX_RATECTRL_DEBUG_ENA
    if( RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))
        DEBUG_MP4("(%1d,%d, %2d,%2d,%2d,%2d,%d,%d)\n",dftMpeg4Quality,mpRateControlParam.QP_P, TxBufFullness,*pSize,AvgFrameSize,BitRateRatioPercent, DropExtraFrame, DropFrameInterval);
     #endif
        //-------------Video FIFO management: 計算下一個Video frame start address-------------//
        OS_ENTER_CRITICAL();
        CurrentVideoSize   += *pSize;
        OS_EXIT_CRITICAL();

            NextIdx = (VideoBufMngWriteIdx + 1) % VIDEO_BUF_NUM;
            pBuf    = (u8*)(((u32)pBuf + *pSize + 3) & ~3);
            
            if(pBuf > mpeg4VideBufEnd) 
            {
                DEBUG_MP4("VideoBuf overflow!!!!\n");
            }
            if(pBuf < (mpeg4VideBufEnd - MPEG4_MIN_BUF_SIZE))
                VideoBufMng[NextIdx].buffer = pBuf;
            else
                VideoBufMng[NextIdx].buffer = VideoBuf;
            VideoBufMngWriteIdx = NextIdx;
            
        VideoPictureIndex++;    

        //----------Sync with others Task-----------//
        OSSemPost(VideoCmpSemEvt);  
    #if NIC_SUPPORT
		if(EnableStreaming)
	        OSSemPost(VideoRTPCmpSemEvt[0]);  
	    #if TUTK_SUPPORT         
	        if(P2PEnableStreaming[0]) //Local_Record, Local_Playback
	            OSSemPost(P2PVideoCmpSemEvt[0]);  
	    #endif
    #endif
    #if RFIU_SUPPORT
         OSSemPost(gRfiuAVCmpSemEvt[0]);
    #endif

    }
}
#else

void rfiuTXMpeg4EncTask(void* pData)
{
    u32*        pFlag;
    s64*        pTime;
    u32*        pSize;
    u32*        pOffset;
    u8          *pBuf, *pReadBuf;
    u8          err;
    u32         NextIdx;
    u32         outLen;
    s32         i, DropFrame;
    int         SizeLevel;
    int         TxBufFullness;
    int         TotalDropFrame;
    int         Drop_sub;
#if RF_TX_OPTIMIZE      
    int         RFUnit;
#endif
    //Lucian: 多Tx時,要改為Global Array. 
    s32 Drop_ext=0;
    int  QP_ext2=0;
    u32  Mpeg4EncCnt = 0; //Lucina: for internal use: roundtype, reconstruct frame switch.
    u32  TXErrMonitor=0;
    u32  PrevTXRunCnt=0xffffffff;
    int  BitRateRatioPercent;
    //----//
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    unsigned int ByteCnt,ByteCnt_sub;
    unsigned long t1,t2,dt,RealBitRate,RealBitRate_sub;
    unsigned int VideoCnt;
    unsigned int AvgFrameSize;
    unsigned int EncFrameCnt,AvgFrameRate;
    int      QPsub_ext;
    unsigned int SubTargetBR;
    unsigned int EncodeTime;
    int VideoBufferSize;
    u8 RateControl_PIRRecOn, low_buffer_count;
    //---------------------------//

#if RF_TX_OPTIMIZE  
    RFUnit= (int)pData;
    VideoBufMngWriteIdx=0;
    VideoPictureIndex  = 0;
    asfVideoFrameCount=0;
    gRfiu_MpegEnc_Sta[RFUnit]=RFI_MPEGENC_TASK_RUNNING;
#endif
    MPEG4_Task_Go   = 1;    // 0: never run, 1: ever run


    //====Set Initial Bit Rate====//
    if(mpeg4Width <= 360) //QVGA or CIF
    {
       rfiuTXBitRate[0]=700; //Unit: Kbps
       SizeLevel=MPEG4_REC_QVGA;
    }
    else if(mpeg4Width<=720) //VGA or D1
    {
       rfiuTXBitRate[0]=1400; //Unit: Kbps
       SizeLevel=MPEG4_REC_QHD;
    }
    else if(mpeg4Width<=1440) //HD or 4D1
    {
       rfiuTXBitRate[0]=1700; //Unit: Kbps
       SizeLevel=MPEG4_REC_HD;
    }
    else if(mpeg4Width<=2200)
    {
       rfiuTXBitRate[0]=1700; //Unit: Kbps
       SizeLevel=MPEG4_REC_FULLHD;
    }
    else
    {
       rfiuTXBitRate[0]=2400; //Unit: Kbps
       SizeLevel=MPEG4_REC_HD;
    }
    //====//
    rfiuTxBufFullness[0]=0;
    ByteCnt=0;
    ByteCnt_sub=0;
    DropFrame=0;
    QP_ext2=0;
    QPsub_ext=0;
    VideoCnt=0;
    EncFrameCnt=0;
    Drop_ext=0;
    EncodeTime=0;
    BitRateRatioPercent=100;
    RateControl_PIRRecOn = 1;
    mpRateControlParam.Int_prev_PMad = (8*4)*1024;  //FixRC
    AvgFrameSize=(rfiuTXBitRate[0]-140)*100/100*9/10/8/( rfiuVideoInFrameRate/(1+0) );

    timerCountRead(guiRFTimerID, &t1);

    DEBUG_MP4("\n-TXMpg4EncTsk-\n");
  #if RF_TX_OPTIMIZE 
    gRfiuUnitCntl[RFUnit].TX_MpegEnc_StopRdy=0;
  #endif
    while (1)
    {
    #if RF_TX_OPTIMIZE  
        if(gRfiuUnitCntl[RFUnit].TX_MpegEnc_Stop)
        {
           DEBUG_MP4("#");
           gRfiuUnitCntl[RFUnit].TX_MpegEnc_StopRdy=1;
           OSTimeDly(1);
           continue;
        }
        gRfiuUnitCntl[RFUnit].TX_MpegEnc_StopRdy=0;
    #else
    
        Video_Pend  = 1;
        OSSemPend(VideoTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
        Video_Pend  = 0;
    
        if (err != OS_NO_ERR)
        {
            DEBUG_MP4("Error: VideoTrgSemEvt is %d.\n", err);
            //return;
        }
    #endif

        if(mpeg4Width <= 360) //QVGA or CIF
	    {
	       SizeLevel=MPEG4_REC_QVGA;
	    }
	    else if(mpeg4Width<=720) //VGA or D1
	    {
	       SizeLevel=MPEG4_REC_QHD;
	    }
	    else if(mpeg4Width<=1440) //HD or 4D1
	    {
	       SizeLevel=MPEG4_REC_HD;
	    }
        else if(mpeg4Width<=2200) //FHD
	    {
	       SizeLevel=MPEG4_REC_FULLHD;
	    }
	    else
	    {
	       SizeLevel=MPEG4_REC_QHD;
	    }
        
        pBuf        = VideoBufMng[VideoBufMngWriteIdx].buffer;  /*BJ 0530 S*/
        pFlag       = &VideoBufMng[VideoBufMngWriteIdx].flag; 
        pTime       = &VideoBufMng[VideoBufMngWriteIdx].time; 
        pSize       = &VideoBufMng[VideoBufMngWriteIdx].size; 
        pOffset     = &VideoBufMng[VideoBufMngWriteIdx].offset; 

        if(RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))  
        {
               if(SizeLevel==MPEG4_REC_HD)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
               else if(SizeLevel==MPEG4_REC_FULLHD)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD/2);   // I: 1, P: 0
               else
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0

               if( rfiuRX_OpMode & RfIU_RX_OPMODE_FORCE_I )
               {
                   *pFlag=1;
                   rfiuRX_OpMode=rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);
                   DEBUG_MP4("\n-->Force I \n");
               }
               else if(EncFrameCnt == 0) // first encode frame must be I frame
                    *pFlag=1;
        }
     #if TX_PIRREC_SUPPORT
        else if(rfiuBatCamPIRTrig)
        {
               if(SizeLevel==MPEG4_REC_HD)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
               else if(SizeLevel==MPEG4_REC_FULLHD)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD/2);   // I: 1, P: 0
               else
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0

               if( rfiuRX_OpMode & RfIU_RX_OPMODE_FORCE_I )
               {
                   *pFlag=1;
                   rfiuRX_OpMode=rfiuRX_OpMode & (~RfIU_RX_OPMODE_FORCE_I);
                   DEBUG_MP4("\n-->Force I \n");
               }
               else if(EncFrameCnt == 0) // first encode frame must be I frame
                    *pFlag=1;
        }
     #endif
        else
        {
            *pFlag      = DeterminePictureType(VideoPictureIndex, 0);   // I: 1, P: 0
             mpeg4MultiStreamStart=1;
        }
         
        Vop_Type    = (*pFlag)? I_VOP : P_VOP; // 0: I frame, 1: P frame
     
        if(Vop_Type == I_VOP) 
        {     // I frame
            if(mpeg4MultiStreamStart)
                mpeg4MultiStreamEnable=1;
            else
                mpeg4MultiStreamEnable=0;
            
            timerCountRead(guiRFTimerID, &t2);
             if(t1>t2)
              dt=t1-t2;
            else
              dt=(t1+TimerGetTimerCounter(TIMER_7))-t2;

            if(dt==0)
                dt=1;
            //DEBUG_MP4("dt=%d,Cnt=%d\n",dt,ByteCnt);
            RealBitRate = ((ByteCnt+64)*8*10/dt); //Lucian: Video + Header 
            AvgFrameRate=EncFrameCnt*10000/dt;

         #if MULTI_STREAM_SUPPORT  
             #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
                  (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
                  (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
                 Drop_sub=1;
                 QPsub_ext=0;
                 RealBitRate_sub = (ByteCnt_sub*8*10/dt); //Lucian: Video + Header
             #elif( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM))
                 if(mpeg4MultiStreamEnable)
                 {
                    if(AvgFrameRate >= (TARGET_FR_SUBSTREAM*2) )
                    {
                        Drop_sub= (AvgFrameRate+TARGET_FR_SUBSTREAM/2)/TARGET_FR_SUBSTREAM;
                    }
                    else if( AvgFrameRate >= (TARGET_FR_SUBSTREAM*3/2) )
                    {
                        Drop_sub=2;
                    }
                    else
                       Drop_sub=1;
                    
                    RealBitRate_sub = (ByteCnt_sub*8*10/dt); //Lucian: Video + Header
                    
                    if(gRfiuUnitCntl[0].RFpara.TX_SubStreamBRSel==RF_SUBSTREAM_BR_H)
                       SubTargetBR=TARGET_BR_SUBSTREAM_H;
                    else if(gRfiuUnitCntl[0].RFpara.TX_SubStreamBRSel==RF_SUBSTREAM_BR_M)
                       SubTargetBR=TARGET_BR_SUBSTREAM_M;
                    else
                       SubTargetBR=TARGET_BR_SUBSTREAM_L;
                    
                    if(RealBitRate_sub != 0)
                    {
                       if(mpRateControlParam.enable_ratecontrol)
                       {
                         if(RealBitRate_sub > SubTargetBR)
                            QPsub_ext ++;
                         else if(RealBitRate_sub < (SubTargetBR-15) )
                            QPsub_ext --;

                         if(QPsub_ext>5)
                            QPsub_ext = 5;
                         else if(QPsub_ext < -5)
                            QPsub_ext = -5;
                       }
                       else
                          QPsub_ext=0;
                    } 
                }
                else
                    Drop_sub=1;
            #endif
         #endif
            if( RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))
            {
                   DropFrame=rfiuTxRateControl_Start(rfiuTXBitRate[0],
                                                     rfiuVideoInFrameRate,
                                                     SizeLevel);
            }
         #if TX_PIRREC_SUPPORT   
            else if(rfiuBatCamPIRTrig)
            {
               rfiuTXBitRate[0]=1700;
               DropFrame=rfiuTxRateControl_Start(rfiuTXBitRate[0],
                                                 rfiuVideoInFrameRate,
                                                 SizeLevel);
            }
         #endif
            else
            {
            #if RF_TX_OPTIMIZE
               DropFrame=rfiuTxRateControl_Start(600,
                                                 rfiuVideoInFrameRate,
                                                 SizeLevel);
            #else
                DropFrame=rfiuTxRateControl_Start(150,
                                                  rfiuVideoInFrameRate,
                                                  SizeLevel);
            #endif
            }

          
          
          #if MULTI_STREAM_SUPPORT     
            if(mpeg4MultiStreamEnable)
               DEBUG_MP4(" I:%d,%d,%d,%d,%d,%d\n",AvgFrameRate,rfiuTxBufFullness[0],rfiuTXBitRate[0],RealBitRate,RealBitRate_sub,QPsub_ext);
            else
               DEBUG_MP4(" I:%d,%d,%d,%d\n",AvgFrameRate,rfiuTxBufFullness[0],rfiuTXBitRate[0],RealBitRate);
          #else
            DEBUG_MP4(" I:%d,%d,%d,%d\n",AvgFrameRate,rfiuTxBufFullness[0],rfiuTXBitRate[0],RealBitRate);
          #endif
            ByteCnt=0;
            ByteCnt_sub=0;
        #if 1
            //=====Lucian: 監視TX RF task是否當掉=====//
            if(Main_Init_Ready == 1)
            {
                if(gRfiuUnitCntl[0].RunCount == PrevTXRunCnt)
                    TXErrMonitor ++;
                else
                    TXErrMonitor=0;

                if(TXErrMonitor > 4)
                {
                   DEBUG_MP4("\n======== rfiu_Tx_Task_UnitX is Fatal Error! Reboot!======\n");
    			   sysForceWDTtoReboot();
                }
                PrevTXRunCnt=gRfiuUnitCntl[0].RunCount;
            }
        #endif
            t1=t2;

            EncFrameCnt=0;
            VideoCnt=255;
            
        }
        else
        {   //P frame
            //-- 根據TxBuf fullness 來微調 Target bitrate--//
            //--------HD/Full HD--------//
           #if TX_PIRREC_SUPPORT 
             if(rfiuBatCamPIRTrig==0)
             {
                 rfiuTxRateControl_LiveMode(
                                                  rfiuTxBufFullness[0],
                                                  SizeLevel,
                                                  &VideoCnt,
                                                  &Drop_ext,
                                                  &QP_ext2,
                                                  &BitRateRatioPercent
                                           );
             }
             else
             {
                // calculate the video buffer + TX buffer size
                if(VideoBufMng[VideoBufMngReadIdx].buffer < VideoBufMng[VideoBufMngWriteIdx].buffer)
                    VideoBufferSize = VideoBufMng[VideoBufMngWriteIdx].buffer - VideoBufMng[VideoBufMngReadIdx].buffer;
                else
                    VideoBufferSize = MPEG4_MAX_BUF_SIZE - (VideoBufMng[VideoBufMngReadIdx].buffer - VideoBufMng[VideoBufMngWriteIdx].buffer);
                VideoBufferSize = VideoBufferSize/1000 + rfiuTxBufFullness[0];                

                if(VideoBufMngReadIdx > 0){
#if TX_PIRREC_RATE_CONTROL
                    if((VideoBufferSize > rfiuTxRateControl_GetSwitchBufferSize(SizeLevel, &Drop_ext, &BitRateRatioPercent)) && RateControl_PIRRecOn){
                        low_buffer_count = 0;
                        rfiuTxRateControl_PIRRecMode(
                                                      VideoBufferSize,
                                                      &VideoCnt,
                                                      &Drop_ext,
                                                      &QP_ext2,
                                                      &BitRateRatioPercent
                        );
                    }
                    else
#endif
                    {
#if TX_PIRREC_RATE_CONTROL
                        low_buffer_count++;
                        if(low_buffer_count > 1)
                            RateControl_PIRRecOn = 0;
#endif
                        rfiuTxRateControl_LiveMode(
                                                  rfiuTxBufFullness[0],
                                                  SizeLevel,
                                                  &VideoCnt,
                                                  &Drop_ext,
                                                  &QP_ext2,
                                                  &BitRateRatioPercent
                        );
                    }
                }
//                printf("[P frame] Buffer:%d, R/W:%d/%d, (R:%p  W:%p) (%d) drop:%d, QP_2:%d\n", rfiuTxBufFullness[0], VideoBufMngReadIdx, VideoBufMngWriteIdx, VideoBufMng[VideoBufMngReadIdx].buffer - VideoBuf, VideoBufMng[VideoBufMngWriteIdx].buffer - VideoBuf, VideoBufferSize, Drop_ext, QP_ext2);
             }
           #else
             rfiuTxRateControl_LiveMode(
                                          rfiuTxBufFullness[0],
                                          SizeLevel,
                                          &VideoCnt,
                                          &Drop_ext,
                                          &QP_ext2,
                                          &BitRateRatioPercent
                                       );
           #endif
        }
                    
        //--------Video rate control: Calculate QP-----//            
        if(mpRateControlParam.enable_ratecontrol)
        {
             //DEBUG_MP4("--RCQ2_QuantAdjust---\n");
             if (Vop_Type == 0) //I frame
             {
                 dftMpeg4Quality =mpRateControlParam.QP_I;
             }
             else //P frame
             {
                 dftMpeg4Quality=mpRateControlParam.QP_P;
                 video_double_field_flag = 0;
             }

             if(Vop_Type == I_VOP)
             {
                if(dftMpeg4Quality>RC_MAX_QP_I)
                   dftMpeg4Quality=RC_MAX_QP_I;
             }
             else
             {
                dftMpeg4Quality += (QP_ext2);
                if(dftMpeg4Quality>RC_MAX_QP)
                   dftMpeg4Quality=RC_MAX_QP;
             }
             
             
             if(dftMpeg4Quality<RC_MIN_QP)
                dftMpeg4Quality=RC_MIN_QP;         
        }
        else
        {
        }
        //========================================//
        
     #if (MPEG4_FIELD_ENC_ENA && (CIU1_OPTION ==Sensor_CCIR656) )
   
     #elif (CIU1_BOB_REPLACE_MPEG_DF && (CHIP_OPTION != CHIP_A1016A) ) //Lucian: 不用 Mpeg4 double field,起動 ciu mob mode
       #if( CIU1_OPTION == Sensor_CCIR656 )
        if(video_double_field_flag)
        {
           CIU_1_CTL1= (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
           CIU_1_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
        }
        else
        {
           CIU_1_CTL1= (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656;
        } 
       #elif(CIU1_OPTION == Sensor_CCIR601)
        if(video_double_field_flag)
        {
           CIU_1_CTL1= (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
           CIU_1_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
        }
        else
        {
           CIU_1_CTL1= (CIU_1_CTL1 & (~0x30)) | CIU_MODE_601;
        } 
       #endif
     #endif

     #if(RF_TX_OPTIMIZE == 0)
        pReadBuf    = VideoBufMng[VideoBufMngReadIdx].buffer;   /* Peter 070130 */
        if(VideoPictureIndex != 0 && (pReadBuf >= pBuf)) 
        {
           #if DCF_WRITE_STATISTIC
              dcfMpegBufRemain=pReadBuf-pBuf;
           #endif
            while((pReadBuf >= pBuf) &&
                  ((pReadBuf - pBuf) < MPEG4_MIN_BUF_SIZE) && 
                  (VideoCmpSemEvt->OSEventCnt > 2) &&
                  (sysCaptureVideoStop == 0)) 
            {
                
                DEBUG_MP4("x");
                OSTimeDly(1);
                pReadBuf    = VideoBufMng[VideoBufMngReadIdx].buffer;
            }
        }
        else
        {
           dcfMpegBufRemain=MPEG4_MAX_BUF_SIZE-(pBuf-pReadBuf);
        }
     #endif        
        //---------- Video Encoding & Frame Rate control---------//
        *pTime=0;
    
        //======== Lucian: frame rate control =======//
        TotalDropFrame=(DropFrame+Drop_ext);
        //DEBUG_MP4("-->1\n");
        #if (MULTI_CHANNEL_SEL & 0x01)
        if(isuSemEvt->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! isuSemEvt=%d\n",isuSemEvt->OSEventCnt);
        }
        if(EncFrameCnt > 0){
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(isuSemEvt, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
        
            }
        }
        #elif (MULTI_CHANNEL_SEL & 0x02)
        if(ciuCapSemEvt_CH1->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! ciuCapSemEvt_CH1=%d\n",ciuCapSemEvt_CH1->OSEventCnt);
        }
        if(EncFrameCnt > 0){        
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH1, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
        
            }
        }
        #elif (MULTI_CHANNEL_SEL & 0x04)
        if(ciuCapSemEvt_CH2->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! ciuCapSemEvt_CH2=%d\n",ciuCapSemEvt_CH2->OSEventCnt);
        }
        if(EncFrameCnt > 0){
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH2, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
            }
        }
        #elif (MULTI_CHANNEL_SEL & 0x08)
        if(ciuCapSemEvt_CH3->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! ciuCapSemEvt_CH3=%d\n",ciuCapSemEvt_CH3->OSEventCnt);
        }
        if(EncFrameCnt > 0){
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH3, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
            }
        }
        #elif (MULTI_CHANNEL_SEL & 0x10)
        if(ciuCapSemEvt_CH4->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! ciuCapSemEvt_CH4=%d\n",ciuCapSemEvt_CH4->OSEventCnt);
        }
        if(EncFrameCnt > 0){
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH4, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
            }
        }
        #elif (MULTI_CHANNEL_SEL & 0x20)
        if(ciuCapSemEvt_CH5->OSEventCnt > 2)
        {
            TotalDropFrame +=1;
            DEBUG_MP4("\n---->Warning! ciuCapSemEvt_CH5=%d\n",ciuCapSemEvt_CH5->OSEventCnt);
        }
        if(EncFrameCnt > 0){
            for(i = 0; i < TotalDropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH5, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                //VideoSmallPictureIndex++;
                mp4_avifrmcnt++;
                VideoCnt ++;
            }
        }
        #endif

      //===================Encoding one Frame========================//
      //DEBUG_MP4("-->2\n");
      #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
        mpeg4Output1Frame(pBuf, pTime, pSize, &Mpeg4EncCnt, pOffset);
      #elif (VIDEO_CODEC_OPTION == H264_CODEC) 
        if(VideoPictureIndex>=START_VIDEO_INDEX)
        {
            video_info.StreamBuf = pBuf;
            video_info.FrameIdx  = VideoPictureIndex;  
            video_info.FrameTime = pTime;
            video_info.pSize     = pSize;  

            video_info.FrameType = *pFlag;
            video_info.Small_FrameType = *pFlag;
            
            H264Enc_cfg.qp = dftMpeg4Quality;
            H264Enc_cfg.slice_qp_delta =  H264Enc_cfg.qp - 26;
            #if MULTI_STREAM_SUPPORT
                if(  ((EncFrameCnt % Drop_sub) == 0) && mpeg4MultiStreamEnable )
                {
                    H264Enc_cfg.small_qp = dftMpeg4Quality ;    
                    H264Enc_cfg.small_qp += QPsub_ext;

                    if(mpRateControlParam.enable_ratecontrol)
                    {
                        if(H264Enc_cfg.small_qp > RC_MAX_QP_SUB)
                           H264Enc_cfg.small_qp=RC_MAX_QP_SUB;
                        else if(H264Enc_cfg.small_qp < RC_MIN_QP)
                           H264Enc_cfg.small_qp=RC_MIN_QP; 
                    }
                    else
                    {
                        if(H264Enc_cfg.small_qp > 45)
                           H264Enc_cfg.small_qp=45;
                    }
                        
                    H264Enc_cfg.small_slice_qp_delta = H264Enc_cfg.small_qp - 26;
                    
                    H264Enc_CompressOneFrame(&video_info,&H264Enc_cfg,1);   
                    VideoBufMng[VideoBufMngWriteIdx].offset = video_info.poffset; 
                    //VideoSmallPictureIndex++;
                 #if RF_TX_RATECTRL_DEBUG_ENA    
                    //DEBUG_MP4("SubQp=%d,%d\n",H264Enc_cfg.small_qp,(*pSize - *pOffset));
                 #endif
                }
                else
                {
                    H264Enc_CompressOneFrame(&video_info,&H264Enc_cfg,0);   // only big stream 
                    VideoBufMng[VideoBufMngWriteIdx].offset = video_info.poffset; 
                    //VideoSmallPictureIndex++;
                }

                EncFrameCnt ++;

            #else
                H264Enc_CompressOneFrame(&video_info,&H264Enc_cfg,0);   // only big stream 
                VideoBufMng[VideoBufMngWriteIdx].offset = video_info.poffset; 
                EncFrameCnt ++;
            #endif
        }
      else{
        #if (MULTI_CHANNEL_SEL & 0x01)
        OSSemPend(ciuCapSemEvt_CH1, 4 ,&err);
        #elif (MULTI_CHANNEL_SEL & 0x02)
        OSSemPend(ciuCapSemEvt_CH2, 4 ,&err);        
        #elif (MULTI_CHANNEL_SEL & 0x04)
        OSSemPend(ciuCapSemEvt_CH3, 4 ,&err);        
        #elif (MULTI_CHANNEL_SEL & 0x08)
        OSSemPend(ciuCapSemEvt_CH3, 4 ,&err);        
        #elif (MULTI_CHANNEL_SEL & 0x10)
        OSSemPend(ciuCapSemEvt_CH4, 4 ,&err);        
        #elif (MULTI_CHANNEL_SEL & 0x20)
        OSSemPend(ciuCapSemEvt_CH5, 4 ,&err);    
        #endif
        VideoPictureIndex++;
        continue;
      }
      #endif
        VideoCnt ++;
        //DEBUG_MP4("-->3\n");
        EncodeTime += *pTime;
     #if 0 //TX_PIRREC_SUPPORT
        if(rfiuBatCamPIRTrig)
        {
            if(EncodeTime > (rfiuBatCam_PIRRecDurationTime+3)*1000)
            {
                gRfiuUnitCntl[RFUnit].TX_MpegEnc_Stop=1;
            #if 1    
                switch(sysVideoInCHsel)
                {
                    case 0:
                        isuStop();
                        ipuStop();
                        siuStop();
                        break;

                    case 1:
                        ciu_1_Stop();
                        SYSClkDisable(SYS_CTL0_CIU_CKEN);
                        break;

                    case 2:
                        ciu_2_Stop();
                        SYSClkDisable(SYS_CTL0_CIU2_CKEN);
                        break;
                    case 3:
                        ciu_3_Stop();
                        break;

                    case 4:
                        ciu_4_Stop();
                        break;

                    case 5:
                        ciu_5_Stop();
                        ipuStop();
                        siuStop();
                        //SYSClkDisable(SYS_CTL0_SER_MCKEN);
                        //SYSClkDisable(SYS_CTL0_SIU_CKEN);
                        //SYSClkDisable(SYS_CTL0_ISU_CKEN);
                        break;
                }   
                //gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 ); //sensor reset
            #endif    
                rfiuStopPIRRecReady=1; // disable frame-check scheme.
                DEBUG_MP4("\n==REC Time up!==\n");
            }
        }
     #endif        
        //--------Rebuild RC model of Rate control ------//
        ByteCnt += *pSize;
     #if MULTI_STREAM_SUPPORT    
        ByteCnt_sub += (*pSize - *pOffset);
        //DEBUG_MP4("(%d,%d)",*pSize,(*pSize - *pOffset));
     #endif
        if(mpRateControlParam.enable_ratecontrol)
        {
            //TotalSAD    = mpeg4Height*mpeg4Width*8;
            outLen      = *pSize; //Video size. 
            AvgFrameSize=(rfiuTXBitRate[0]-130)*BitRateRatioPercent/100*100/100/8/( rfiuVideoInFrameRate/(1+DropFrame) );

            if (Vop_Type ==I_VOP)  
            {
                if(mpRateControlParam.QP_I > mpRateControlParam.QP_P)
                   QP_ext2= mpRateControlParam.QP_I-mpRateControlParam.QP_P;
            }
            else //P or B frame
            {
                mpRateControlParam.Avg_PMad=mpRateControlParam.Int_prev_PMad;

                if( (outLen >> 10) > (AvgFrameSize*2) )
                {
                     QP_ext2 +=3;
                }
                else if( (outLen >> 10) > (AvgFrameSize*15/10) )
                {
                     QP_ext2 +=2;
                }
                else if( (outLen >> 10) > AvgFrameSize)
                {
                     QP_ext2 +=1;
                }
                else if( (outLen >> 10) > (AvgFrameSize*9/10) )
                {
                
                }
                else
                {
                     QP_ext2 -=1;
                }


            }
            

            if(QP_ext2 > MAX_QP_P_EXT2)
                QP_ext2=MAX_QP_P_EXT2;
            else if(QP_ext2 < 0)
                QP_ext2=0; 
                
            
        }
        else
        {
            QP_ext2=0;
        }
        
     #if RF_TX_RATECTRL_DEBUG_ENA
        DEBUG_MP4("(%1d,%2d,%2d,%2d,%2d,%d)\n",TotalDropFrame,dftMpeg4Quality,TxBufFullness,*pSize>>10,QP_ext2,AvgFrameSize);
     #endif
        //-------------Video FIFO management: 計算下一個Video frame start address-------------//
        OS_ENTER_CRITICAL();
        CurrentVideoSize   += *pSize;
        OS_EXIT_CRITICAL();

            NextIdx = (VideoBufMngWriteIdx + 1) % VIDEO_BUF_NUM;
            pBuf    = (u8*)(((u32)pBuf + *pSize + 3) & ~3);
            
            if(pBuf > mpeg4VideBufEnd) 
            {
                DEBUG_MP4("VideoBuf overflow!!!!\n");
            }
            if(pBuf < (mpeg4VideBufEnd - MPEG4_MIN_BUF_SIZE))
                VideoBufMng[NextIdx].buffer = pBuf;
            else
                VideoBufMng[NextIdx].buffer = VideoBuf;
            VideoBufMngWriteIdx = NextIdx;
            
        VideoPictureIndex++;    

        //----------Sync with others Task-----------//
        OSSemPost(VideoCmpSemEvt);  
    #if NIC_SUPPORT
		if(EnableStreaming)
	        OSSemPost(VideoRTPCmpSemEvt[0]);  
	    #if TUTK_SUPPORT         
	        if(P2PEnableStreaming[0]) //Local_Record, Local_Playback
	            OSSemPost(P2PVideoCmpSemEvt[0]);  
	    #endif
    #endif
    #if RFIU_SUPPORT
         OSSemPost(gRfiuAVCmpSemEvt[0]);
    #endif

    }
}



s32 rfiuTxRateControl_Start(int BitRate,int VideoInFrameRate,int SizeLevel)
{
    s32 DropFrame=0;
//    static int Drop_Bitrate=0;
    static int Prev_BitRateLv=0xff;
    int Cur_BitRateLv;

    mpRateControlParam.max_Qp=RC_MAX_QP;                  //max Qp
	mpRateControlParam.min_Qp=RC_MIN_QP;                   //min Qp

    
    //============= QHD(20~24) ===============//
    if( SizeLevel == MPEG4_REC_QHD) //QHD size
    {
        if(VideoInFrameRate>=24)  // 24~30 fps
        {
           #if(FPGA_BOARD_A1018_SERIES)
            if(0)
            {

            }
           #else
            if( BitRate>1700) // 30 fps
            {  
               Cur_BitRateLv=0;
               DropFrame=0;
               mpRateControlParam.enable_ratecontrol   = 1;
               if(BitRate>2500)
               {
                  mpRateControlParam.InitQP=RC_MIN_QP;
                  mpRateControlParam.QP_I=28;
                  mpRateControlParam.QP_P=RC_MIN_QP;
               }
               else if(BitRate>2000)
               {
                  mpRateControlParam.InitQP=RC_MIN_QP;
                  mpRateControlParam.QP_I=28;
                  mpRateControlParam.QP_P=RC_MIN_QP;
               }
               else
               {
                  mpRateControlParam.InitQP=RC_MIN_QP;
                  mpRateControlParam.QP_I=29;
                  mpRateControlParam.QP_P=RC_MIN_QP;
               }
            }
           #endif   
            else if( BitRate>1000  ) // 15 fps
            {
               DropFrame=1;
               Cur_BitRateLv=1;
               mpRateControlParam.enable_ratecontrol   = 1;
               if(BitRate>1400)
               {
                  mpRateControlParam.InitQP=RC_MIN_QP;
                  mpRateControlParam.QP_I=28;
                  mpRateControlParam.QP_P=RC_MIN_QP;
               }
               else if(BitRate>1200)
               {
                  mpRateControlParam.InitQP=RC_MIN_QP;
                  mpRateControlParam.QP_I=29;
                  mpRateControlParam.QP_P=RC_MIN_QP;
               }
               else
               {
                  mpRateControlParam.InitQP=RC_MIN_QP;
                  mpRateControlParam.QP_I=29;
                  mpRateControlParam.QP_P=RC_MIN_QP;
               }
            }
            else if(BitRate>700)  // 10 fps
            {
               DropFrame=2;
               mpRateControlParam.enable_ratecontrol   = 0;
               dftMpeg4Quality = 30;
               video_double_field_flag = 1;
            }
            else if(BitRate>500)
            {
               DropFrame=5;       // 5 fps
               mpRateControlParam.enable_ratecontrol   = 0;
               dftMpeg4Quality = 32;
               video_double_field_flag = 1;
            }
            else
            {
               DropFrame=9;       // 3 fps
               mpRateControlParam.enable_ratecontrol   = 0;
               dftMpeg4Quality = 32;
               video_double_field_flag = 1;
            }
            mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
        }
        else
        {   //not implement now//
            DEBUG_MP4("Warning!! VGA,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
        }
        
    }
    //============= HD(20~24~27 fps) ===============//
    else if( SizeLevel == MPEG4_REC_HD) //HD size
    {
        video_double_field_flag = 0;
        #if (( HW_BOARD_OPTION == MR9100_TX_OPCOM_HD_USB) || ( HW_BOARD_OPTION == MR9100_TX_SKY_USB))
        if( BitRate>2300 ) // 30 fps
        #else
        if( BitRate>1900 ) // 30 fps
        #endif
        {
           Cur_BitRateLv=0;
        #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
             (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
             (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
           if( rfiuRX_OpMode & RFIU_RX_OPMODE_P2P)
           {
               if(rfiuRX_P2pVideoQuality==RF_P2PVdoQalt_LOW)
               {
                  DropFrame=0;
               }
               else
               {
                  DropFrame=1;
               }
           }
           else
              DropFrame=0;
        #else
           if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
           {
               DropFrame=0;
           }
           else //Quad mode
           {
               if(rfiuRX_CamOnOff_Num>3)
               {
                  DropFrame=3;
               }
               else if(rfiuRX_CamOnOff_Num>2)
                  DropFrame=2;
               else if(rfiuRX_CamOnOff_Num>1)
                  DropFrame=1;
               else
                  DropFrame=0;
           }
         #endif   
           mpRateControlParam.enable_ratecontrol   = 1;
           if(BitRate>3000)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=28;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else if(BitRate>2500)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=29;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else if(BitRate>2000)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=30;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=30;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
        }
        else if( (BitRate>1000) && ( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 ) && ( (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) ==0 )  ) // 15 fps
        {
        #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
             (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
             (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
           DropFrame=1;
        #else
           if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
              DropFrame=1;
           else
           {
               if(rfiuRX_CamOnOff_Num>3)
               {
                  DropFrame=3;
               }
               else if(rfiuRX_CamOnOff_Num>2)
                  DropFrame=2;
               else
                  DropFrame=1;
           }
         #endif
           Cur_BitRateLv=1;
           mpRateControlParam.enable_ratecontrol   = 1;
           if(BitRate>1400)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=28;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else if(BitRate>1200)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=29;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=30;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
        }
        else if(BitRate>750)  // 10 fps
        {
        #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
             (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
             (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
           DropFrame=2;
        #else
           if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
              DropFrame=2;
           else
           {
               if(rfiuRX_CamOnOff_Num>3)
               {
                  DropFrame=3;
               }
               else if(rfiuRX_CamOnOff_Num>2)
                  DropFrame=2;
               else
                  DropFrame=2;
           }
        #endif
           
           Cur_BitRateLv=2;
           mpRateControlParam.enable_ratecontrol   = 1;
           if(BitRate>900)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=30;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else if(BitRate>800)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=30;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=30;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
        }
        else if(BitRate>500)  // 6 fps
        {
           DropFrame=4;             
           Cur_BitRateLv=3;
           mpRateControlParam.enable_ratecontrol   = 1;
           mpRateControlParam.InitQP=RC_MIN_QP;
           mpRateControlParam.QP_I=30;
           mpRateControlParam.QP_P=RC_MIN_QP;
        }
        else if(BitRate>350)
        {
           DropFrame=7;       // 4 fps
           mpRateControlParam.enable_ratecontrol   = 0;
           dftMpeg4Quality = 30;
        }
        else if(BitRate>250)
        {
           DropFrame=9;       // 3 fps
           mpRateControlParam.enable_ratecontrol   = 0;
           dftMpeg4Quality = 32;
        }
        else if(BitRate>180)
        {
           DropFrame=14;       // 2 fps
           mpRateControlParam.enable_ratecontrol   = 0;
           dftMpeg4Quality = 32;
        }
        else if((BitRate>120))
        {
           DropFrame=29;       // 1 fps
           mpRateControlParam.enable_ratecontrol   = 0;
           dftMpeg4Quality = 32;
        }
        else if((BitRate>80))
        {
           DropFrame=29;       // 0.5 fps
           mpRateControlParam.enable_ratecontrol   = 0;
           dftMpeg4Quality = 34;
        }
        else
        {
           DropFrame=29;       // 0.25fps
           mpRateControlParam.enable_ratecontrol   = 0;
           dftMpeg4Quality = 38;
        }
        mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
    }
    //=============Full HD(10~12 fps) ===============//
    else if( SizeLevel == MPEG4_REC_FULLHD) //HD size
    {
        video_double_field_flag = 0;
                
        if( BitRate>1900 ) // 10 fps
        {
           Cur_BitRateLv=0;
        #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
             (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
             (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
           if( rfiuRX_OpMode & RFIU_RX_OPMODE_P2P)
           {
               if(rfiuRX_P2pVideoQuality==RF_P2PVdoQalt_LOW)
               {
                  DropFrame=0;
               }
               else
               {
                  DropFrame=1;
               }
           }
           else
              DropFrame=0;        
        #else
           if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
           {
               DropFrame=0;
           }
           else //Quad mode
           {
               if(rfiuRX_CamOnOff_Num>3)
               {
                  DropFrame=3;
               }
               else if(rfiuRX_CamOnOff_Num>2)
                  DropFrame=2;
               else if(rfiuRX_CamOnOff_Num>1)
                  DropFrame=1;
               else
                  DropFrame=0;
           }
         #endif   
           mpRateControlParam.enable_ratecontrol   = 1;
           if(BitRate>2500)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=31;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else if(BitRate>2000)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=32;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=33;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
        }
        else if( (BitRate>1000)   ) // 5 fps
        {
        #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
             (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
             (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
           DropFrame=1;
        #else
           if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
           {
               DropFrame=1;
           }
           else //Quad mode
           {
               if(rfiuRX_CamOnOff_Num>3)
               {
                  DropFrame=3;
               }
               else if(rfiuRX_CamOnOff_Num>2)
                  DropFrame=2;
               else if(rfiuRX_CamOnOff_Num>1)
                  DropFrame=1;
               else
                  DropFrame=1;
           }
        #endif
           Cur_BitRateLv=1;
           mpRateControlParam.enable_ratecontrol   = 1;
           if(BitRate>1400)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=31;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else if(BitRate>1200)
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=32;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
           else
           {
              mpRateControlParam.InitQP=RC_MIN_QP;
              mpRateControlParam.QP_I=33;
              mpRateControlParam.QP_P=RC_MIN_QP;
           }
        }
        else if(BitRate>700)  // 3 fps
        {
        #if( (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
             (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
             (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
           DropFrame=2;
        #else
           if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
              DropFrame=2;
           else
           {
               if(rfiuRX_CamOnOff_Num>3)
               {
                  DropFrame=3;
               }
               else if(rfiuRX_CamOnOff_Num>2)
                  DropFrame=2;              
               else
                  DropFrame=2;
           }
        #endif
           Cur_BitRateLv=2;
           mpRateControlParam.enable_ratecontrol   = 1;
           if(BitRate>900)
           {
              mpRateControlParam.InitQP=32;
              mpRateControlParam.QP_I=32;
              mpRateControlParam.QP_P=32;
           }
           else if(BitRate>800)
           {
              mpRateControlParam.InitQP=32;
              mpRateControlParam.QP_I=32;
              mpRateControlParam.QP_P=32;
           }
           else
           {
              mpRateControlParam.InitQP=32;
              mpRateControlParam.QP_I=32;
              mpRateControlParam.QP_P=32;
           }
        }
        else if(BitRate>500)
        {
           DropFrame=4;       // 2 fps
           mpRateControlParam.enable_ratecontrol   = 0;
           dftMpeg4Quality = 34;
        }
        else if(BitRate>300)
        {
           DropFrame=6;       // 1.5 fps
           mpRateControlParam.enable_ratecontrol   = 0;
           dftMpeg4Quality = 34;
        }
        else
        {
           DropFrame=9;       // 1 fps
           mpRateControlParam.enable_ratecontrol   = 0;
           dftMpeg4Quality = 34;
        }
        mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
    }
                
    TVout_Generate_Pause_Frame = 0;
    ASF_set_interlace_flag = 0;

    if(mpRateControlParam.enable_ratecontrol)
    {
       if(Prev_BitRateLv != Cur_BitRateLv)
       {
	      RCQ2_init(&mpRateControlParam); 
          DEBUG_MP4("==RCQ2_init==\n");
       }
	   Prev_BitRateLv=Cur_BitRateLv;
    }
    else
       Prev_BitRateLv=0xff; 
    
    return DropFrame;
}

s32 rfiuTxRateControl_LiveMode(
                                             int TxBufFullness,
                                             int SizeLevel,
                                             unsigned int *pVideoCnt,
                                             int *pDrop_ext,
                                             int *pQP_ext2,
                                             int *pBitRateRatioPercent
                                        )
{

    if( (SizeLevel == MPEG4_REC_FULLHD) )
    {
        if( RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))  
        {
            if(mpRateControlParam.enable_ratecontrol)
            {
                if( *pVideoCnt > 1)
                {    
                    *pVideoCnt=0;
                    if(TxBufFullness>TX_BUFFCNTL_THR_FHD+300)
                    {
                       *pBitRateRatioPercent=65;
                       *pDrop_ext = 6;
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD+275)
                    {
                       if( (*pDrop_ext >=6)  )
                       {
                         
                       }
                       else
                       {
                         *pBitRateRatioPercent=70;
                         *pDrop_ext = 5;
                       }            
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD+225)
                    {
                       if( (*pDrop_ext >=5)  )
                       {
                         
                       }
                       else
                       {
                         *pBitRateRatioPercent=75;
                         *pDrop_ext = 4;
                       }                            
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD+175)
                    {
                       if( (*pDrop_ext >=4)  )
                       {
                         
                       }
                       else
                       {
                         *pBitRateRatioPercent=80;
                         *pDrop_ext = 3;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD+120)
                    {
                       if(*pDrop_ext>=3)
                       {

                       }
                       else
                       {
                          *pBitRateRatioPercent=85;
                          *pDrop_ext = 2;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD+70)
                    {
                       if(*pDrop_ext>=2)
                       {

                       }
                       else
                       {
                           *pBitRateRatioPercent=90;
                           *pDrop_ext = 1;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD)
                    {
                       if(*pDrop_ext >= 1)
                       {
                       }
                       else
                       {
                          *pBitRateRatioPercent=90;
                          *pDrop_ext = 0;
                       }
                    }  
                    else
                    {
                       *pBitRateRatioPercent=100;
                       *pDrop_ext = 0;
                       
                    }

                }
            
            }
            else
            {
                *pQP_ext2=0;

                if( *pVideoCnt > 1)
                {    
                    *pVideoCnt=0;
                    if(TxBufFullness>TX_BUFFCNTL_THR_FHD_LOW+300)
                    {
                       *pDrop_ext = 9;
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD_LOW+275)
                    {
                       if( (*pDrop_ext >=9)  )
                       {
                         
                       }
                       else
                       {
                         *pDrop_ext = 8;
                       }            
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD_LOW+225)
                    {
                       if( (*pDrop_ext >=8)  )
                       {
                         
                       }
                       else
                       {
                         *pDrop_ext = 7;
                       }                            }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD_LOW+175)
                    {
                       if( (*pDrop_ext >=7)  )
                       {
                         
                       }
                       else
                       {
                         *pDrop_ext = 6;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD_LOW+120)
                    {
                       if(*pDrop_ext>=6)
                       {

                       }
                       else
                       {
                          *pDrop_ext = 5;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD_LOW+70)
                    {
                       if(*pDrop_ext>=5)
                       {

                       }
                       else
                       {
                           *pDrop_ext = 4;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_FHD_LOW)
                    {
                       if(*pDrop_ext >= 4)
                       {
                       }
                       else
                       {
                          *pDrop_ext = 2;
                       }
                    }  
                    else
                    {
                       *pDrop_ext = 0;
                    }

                }
            }
        }
        else
        {
           *pQP_ext2=0;
        }

    }
    else if( (SizeLevel == MPEG4_REC_HD) )
    {
        if( RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))  
        {
            if(mpRateControlParam.enable_ratecontrol)
            {
                if( *pVideoCnt > 1)
                {    
                    *pVideoCnt=0;
                    if(TxBufFullness>TX_BUFFCNTL_THR_HD+200)
                    {
                       *pBitRateRatioPercent=65;
                       *pDrop_ext = 6;
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD+170)
                    {
                       if( (*pDrop_ext >=6)  )
                       {
                         
                       }
                       else
                       {
                         *pBitRateRatioPercent=70;
                         *pDrop_ext = 5;
                       }            
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD+140)
                    {
                       if( (*pDrop_ext >=5)  )
                       {
                         
                       }
                       else
                       {
                         *pBitRateRatioPercent=75;
                         *pDrop_ext = 4;
                       }                            }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD+120)
                    {
                       if( (*pDrop_ext >=4)  )
                       {
                         
                       }
                       else
                       {
                         *pBitRateRatioPercent=80;
                         *pDrop_ext = 3;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD+100)
                    {
                       if(*pDrop_ext>=3)
                       {

                       }
                       else
                       {
                          *pBitRateRatioPercent=85;
                          *pDrop_ext = 2;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD+55)
                    {
                       if(*pDrop_ext>=2)
                       {

                       }
                       else
                       {
                           *pBitRateRatioPercent=90;
                           *pDrop_ext = 1;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD)
                    {
                       if(*pDrop_ext >= 1)
                       {
                       }
                       else
                       {
                          *pBitRateRatioPercent=90;
                          *pDrop_ext = 0;
                       }
                    }  
                    else
                    {
                       *pBitRateRatioPercent=100;
                       *pDrop_ext = 0;
                    }

                }
                //Drop_ext=0;
            }
            else
            {
                *pQP_ext2=0;

                if( *pVideoCnt > 1)
                {    
                    *pVideoCnt=0;
                    if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+200)
                    {
                       *pDrop_ext = 44;
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+170)
                    {
                       if( (*pDrop_ext >=44)  )
                       {
                         
                       }
                       else
                       {
                         *pDrop_ext = 36;
                       }            
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+140)
                    {
                       if( (*pDrop_ext >=36)  )
                       {
                         
                       }
                       else
                       {
                         *pDrop_ext = 28;
                       }                            }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+120)
                    {
                       if( (*pDrop_ext >=28)  )
                       {
                         
                       }
                       else
                       {
                         *pDrop_ext = 20;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+100)
                    {
                       if(*pDrop_ext>=20)
                       {

                       }
                       else
                       {
                          *pDrop_ext = 12;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+35)
                    {
                       if(*pDrop_ext>=12)
                       {

                       }
                       else
                       {
                           *pDrop_ext = 6;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW)
                    {
                       if(*pDrop_ext >= 6)
                       {
                       }
                       else
                       {
                          *pDrop_ext = 3;
                       }
                    }  
                    else
                    {
                       *pDrop_ext = 0;
                    }

                }
            }
        }
        else
        {
           *pQP_ext2=0;
        }

    }
    else //------------VGA,QVGA--------------//
    {
        if( RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))  
        {
            if(mpRateControlParam.enable_ratecontrol )
            {
                if( *pVideoCnt > 1)
                {    
                    *pVideoCnt=0;
                    if(TxBufFullness>TX_BUFFCNTL_THR_QHD+145)
                    {
                       *pBitRateRatioPercent=65;
                       *pDrop_ext = 6;
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_QHD+135)
                    {
                       if( (*pDrop_ext >=6)  )
                       {
                         
                       }
                       else
                       {
                         *pBitRateRatioPercent=70;
                         *pDrop_ext = 5;
                       }            
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_QHD+110)
                    {
                       if( (*pDrop_ext >=5)  )
                       {
                         
                       }
                       else
                       {
                         *pBitRateRatioPercent=75;
                         *pDrop_ext = 4;
                       }                            }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_QHD+85)
                    {
                       if( (*pDrop_ext >=4)  )
                       {
                         
                       }
                       else
                       {
                         *pBitRateRatioPercent=80;
                         *pDrop_ext = 3;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_QHD+45)
                    {
                       if(*pDrop_ext>=3)
                       {

                       }
                       else
                       {
                          *pBitRateRatioPercent=85;
                          *pDrop_ext = 2;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_QHD+20)
                    {
                       if(*pDrop_ext>=2)
                       {

                       }
                       else
                       {
                           *pBitRateRatioPercent=90;
                           *pDrop_ext = 1;
                       }
                    }
                    else if(TxBufFullness>TX_BUFFCNTL_THR_QHD)
                    {
                       if(*pDrop_ext >= 1)
                       {
                       }
                       else
                       {
                          *pBitRateRatioPercent=90;
                          *pDrop_ext = 0;
                       }
                    }  
                    else
                    {
                       *pBitRateRatioPercent=100;
                       *pDrop_ext = 0;
                    }
                    
                }
            
            }
            else
            {
                *pQP_ext2=0;
                if(TxBufFullness>150)
                   *pDrop_ext = 20;
                else if(TxBufFullness>120)
                   *pDrop_ext = 10;
                else if(TxBufFullness>100)
                   *pDrop_ext = 7;
                else if(TxBufFullness>70)
                   *pDrop_ext = 5;
                else if(TxBufFullness>50)
                   *pDrop_ext = 3;
                else if(TxBufFullness>30)
                   *pDrop_ext = 2;
                else if(TxBufFullness>20)
                   *pDrop_ext = 1;
            }
        }
        else
        {
           *pQP_ext2=0;
        }
    }

    return 0;
}
#if TX_PIRREC_RATE_CONTROL
s32 rfiuTxRateControl_GetSwitchBufferSize(
                                             int SizeLevel,
                                             int *pDrop_ext,
                                             int *pBitRateRatioPercent
                                        )
{

    if( (SizeLevel == MPEG4_REC_FULLHD) )
    {
        if(mpRateControlParam.enable_ratecontrol)
        {
            if(*pBitRateRatioPercent <= 65){
                return TX_BUFFCNTL_THR_FHD+300;
            }
            else if(*pBitRateRatioPercent==70){
                return TX_BUFFCNTL_THR_FHD+275;
            }
            else if(*pBitRateRatioPercent==75){
                return TX_BUFFCNTL_THR_FHD+225;
            }
            else if(*pBitRateRatioPercent==80){
                return TX_BUFFCNTL_THR_FHD+175;
            }
            else if(*pBitRateRatioPercent==85){
                return TX_BUFFCNTL_THR_FHD+120;
            }
            else if(*pBitRateRatioPercent==90){
                return TX_BUFFCNTL_THR_FHD+70;
            }
            else if(*pBitRateRatioPercent>=95){
                return TX_BUFFCNTL_THR_FHD;
            }                
        }
        else
        {
            if(*pDrop_ext >= 7){
                return TX_BUFFCNTL_THR_FHD_LOW+225;
            }
            else if(*pDrop_ext == 6){
                return TX_BUFFCNTL_THR_FHD_LOW+175;
            }
            else if(*pDrop_ext == 5){
                return TX_BUFFCNTL_THR_FHD_LOW+120;
            }
            else if(*pDrop_ext == 4){
                return TX_BUFFCNTL_THR_FHD_LOW+70;
            }
            else if(*pDrop_ext <= 3){
                return TX_BUFFCNTL_THR_FHD_LOW;
            }
        }
    }
    else if( (SizeLevel == MPEG4_REC_HD) )
    {
        if(mpRateControlParam.enable_ratecontrol)
        {
            if(*pBitRateRatioPercent <= 65){
                return TX_BUFFCNTL_THR_FHD+200;
            }
            else if(*pBitRateRatioPercent==70){
                return TX_BUFFCNTL_THR_FHD+170;
            }
            else if(*pBitRateRatioPercent==75){
                return TX_BUFFCNTL_THR_FHD+140;
            }
            else if(*pBitRateRatioPercent==80){
                return TX_BUFFCNTL_THR_FHD+120;
            }
            else if(*pBitRateRatioPercent==85){
                return TX_BUFFCNTL_THR_FHD+100;
            }
            else if(*pBitRateRatioPercent==90){
                return TX_BUFFCNTL_THR_FHD+55;
            }
            else if(*pBitRateRatioPercent>=95){
                return TX_BUFFCNTL_THR_FHD;
            }                                
        }
        else
        {
            if(*pDrop_ext>=6){
                TX_BUFFCNTL_THR_FHD_LOW+35;
            }
            else{
                TX_BUFFCNTL_THR_FHD_LOW;
            }
        }
    }
    else //------------VGA,QVGA--------------//
    {
        if(mpRateControlParam.enable_ratecontrol )
        {
            if(*pBitRateRatioPercent <= 65){
                return TX_BUFFCNTL_THR_QHD+145;
            }
            else if(*pBitRateRatioPercent == 70){
                return TX_BUFFCNTL_THR_QHD+135;
            }
            else if(*pBitRateRatioPercent == 75){
                return TX_BUFFCNTL_THR_QHD+110;
            }
            else if(*pBitRateRatioPercent == 80){
                return TX_BUFFCNTL_THR_QHD+85;
            }
            else if(*pBitRateRatioPercent == 85){
                return TX_BUFFCNTL_THR_QHD+45;
            }
            else if(*pBitRateRatioPercent == 90){
                return TX_BUFFCNTL_THR_QHD+20;
            }
            else if(*pBitRateRatioPercent >= 95){
                return TX_BUFFCNTL_THR_QHD;
            }
        }
        else
        {
            if(*pDrop_ext >= 7){
                return 100;
            }
            else if(*pDrop_ext >= 5){
                return 70;
            }
            else if(*pDrop_ext >= 3){
                return 50;
            }
            else if(*pDrop_ext >= 2){
                return 30;
            }
            else if(*pDrop_ext >= 1){
                return 20;
            }   
        }
    }

    return 0;
}

s32 rfiuTxRateControl_PIRRecMode(
                                             int VideoBufferSize,
                                             unsigned int *pVideoCnt,
                                             int *pDrop_ext,
                                             int *pQP_ext2,
                                             int *pBitRateRatioPercent
                                            )              
{
    static int PFramet1=0, PreVideoBufferSize = 0;

    int PFrameDt, PFramet2 = 0, BufferDecRate = 0;    
    if( *pVideoCnt > 1)
    {    
        *pVideoCnt=0;
        //get time
        timerCountRead(guiRFTimerID, &PFramet2);

        if(PFramet1 != 0){
            //calculate after 2 P frame, time diff
            if(PFramet1>PFramet2)
                PFrameDt=PFramet1-PFramet2;
            else
                PFrameDt=(PFramet1+TimerGetTimerCounter(TIMER_7))-PFramet2;
            
            if(PFrameDt > 0)
                BufferDecRate = (PreVideoBufferSize - VideoBufferSize)*10000/PFrameDt;

            if(BufferDecRate == 0)
                return;
            
            //printf("size:%d, Rate:%d, enable_ratecontrol:%d\n", VideoBufferSize, BufferDecRate, mpRateControlParam.enable_ratecontrol);

            if(BufferDecRate > 250){
               *pBitRateRatioPercent += 15;
               *pDrop_ext -= 3;
            }
            if(BufferDecRate > 200){
               *pBitRateRatioPercent += 10;
               *pDrop_ext -= 2;
            }
            else if(BufferDecRate > 150){
               *pBitRateRatioPercent += 5;
               *pDrop_ext -= 1;
}
            else if(BufferDecRate > 100){

            }
            else if(BufferDecRate > 50){
               *pBitRateRatioPercent -= 5; 
               *pDrop_ext += 1;
            }
            else if(BufferDecRate > 0){
               *pBitRateRatioPercent -= 10; 
               *pDrop_ext += 2;
            }
            else if(BufferDecRate < 0){
               *pBitRateRatioPercent -= 15;
               *pDrop_ext += 3;
            }

            if(*pBitRateRatioPercent < 65)
                *pBitRateRatioPercent = 65;
            if(*pBitRateRatioPercent > 100)
                *pBitRateRatioPercent = 100;    
            if(*pDrop_ext < 0)
               *pDrop_ext = 0;
            if(*pDrop_ext > 7)
               *pDrop_ext = 7;
        }
    
        timerCountRead(guiRFTimerID, &PFramet1);
        PreVideoBufferSize = VideoBufferSize;        
    }

    if(PFramet1 == 0){
        timerCountRead(guiRFTimerID, &PFramet1);
        PreVideoBufferSize = VideoBufferSize;
    }
    return 0;
}


#endif

#endif
#else
void rfiuTXMpeg4EncTask(void* pData)
{

}
#endif

#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
void mpeg4IntHandler(void)
{
}

s32 mpeg4Init(void)
{
}

u32 mpeg4DecodeVolHeader(MP4_Option *pMp4Dec_opt,u8* pVopBit, u32 BitsLen)  /* Peter: 0711 */
{
}

u32 rfiuMpeg4DecodeVOP(MP4_Option *pMp4Dec_opt, 
                      u8* pVopBit, 
                      u32 BitsLen, 
                      int RFUnit,
                      unsigned int Offset,
                      int DispMode,
                      int FieldDecEn)
{
}

s32 rfiuMpeg4EncodeVolHeader(u8* pHeader, u32* pHeaderSize, u32 Width, u32 Height,int Use_MPEG_Q)    
{
}

u32 mpeg4PutHeader(u8 *pBuf, u8 *CurrBit, u8 bitSize, u32 Value)
{
    u32 CodeWord;
    u8  packedSize;
    u32 addBytes;
    u32 CodeWord_Shft;
    
    if (bitSize == 0)
        return 0;

    addBytes        = 0;
    Value          &= PutBitsMask[bitSize-1];
    packedSize      = *CurrBit + bitSize;
    CodeWord        = Value +  (((u32) *pBuf)<<bitSize);
    CodeWord_Shft   = (32-packedSize);
    CodeWord      <<= CodeWord_Shft;
    while(packedSize >= 8)
    {
        *pBuf++     = (CodeWord & 0xff000000)>>24;
        CodeWord  <<= 8;
        packedSize -= 8;
        addBytes++;
    }
    if (packedSize !=0)
        *pBuf = (CodeWord & 0xff000000) >> (CodeWord_Shft + (addBytes << 3));
    *CurrBit = packedSize;

    return addBytes;
}

u32 mpeg4PutDummyVOPHeader(u32 Width,u32 Height,u8 *pBuf, u32 *byteno)
{
    u8  bitno;
    u32 temp;
    u8  bitsize;
    u32 bitpos;
    u8  i;
    u32 MB_cnt1,MB_cnt2;

    /*** P-VOP and all MB set skip mode, vista OS can playback smoothly ***/
    bitno = 0;
    // VOP start code
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 32, 0x000001b6);
        
    // Picture coding type
    //*pFlag: I(00),P(01),B(10),S(11)
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 2, 1);
    // Modulo time base
    mpeg4VopTimeInc    += VOP_TIME_INCREMENT; //Lsk : keep 30fps
    while(mpeg4VopTimeInc >= VOP_TIME_INCREMENT_RESOLUTION)
    {
      *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 1, 1);
      mpeg4VopTimeInc -=VOP_TIME_INCREMENT_RESOLUTION;
    }
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 1, 0); //???
    //marker bit
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 1, 1);  
    // vop time increment
    
    temp    = VOP_TIME_INCREMENT_RESOLUTION;
    bitsize = 0;
    do
    {
        temp >>= 1;
        bitsize++;
    } while(temp > 0);
    
    *byteno            += mpeg4PutHeader(pBuf+*byteno, &bitno, bitsize, mpeg4VopTimeInc);  //Lucian: 目前影音同步不會參考此一參數. 會用File format內的機制.    
    // marker bit
    *byteno            += mpeg4PutHeader(pBuf+*byteno, &bitno, 1, 1);

    //vop_coded
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 1, 1);

    
    //vop_rounding_type:: must set 0
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 1, 0);
    
    //intra_dc_vlc_thr//
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 3, 0);    

    //vop_quant (5 bits)
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 5, (u32) dftMpeg4Quality);
        
    //search range always is +/- 16.
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 3, 1);
      
    if(bitno)
        *byteno            += mpeg4PutHeader(pBuf+*byteno, &bitno, 8 - bitno, (u32)Stuffing[8 - bitno]);

    //VGA : All MB(640*480/16/16=1200) set skip mode
    MB_cnt1 = Width*Height/16/16/16;     /*CY 0907*/
    MB_cnt2 = (Width*Height/16/16)%16;     /*CY 0907*/ 

    for(i=0;i<MB_cnt1;i++)
    {
        *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 16, 0xffff);
    }
    if(MB_cnt2)
    {
        temp=1;
        for(i=1;i<MB_cnt2;i++)
        {
            temp = (temp << 1) | 0x00000001;
        }
        *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, MB_cnt2, temp);   
    }
    if(bitno)
        *byteno            += mpeg4PutHeader(pBuf+*byteno, &bitno, 8 - bitno, (u32)Stuffing[8 - bitno]);
    return 0;
}

s32 mpeg4Output1Frame(u8* pBuf, s64* pTime, u32* pSize, u32* Mpeg4EncCnt, u32* pOffset)
{
}
#endif


