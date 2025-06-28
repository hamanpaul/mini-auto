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
#if MULTI_CHANNEL_SUPPORT
#include "ciuapi.h"
#include "GlobalVariable.h"
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

#if 1//FORCE_FPS
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

VIDEO_BUF_MNG P2PVideoBufMng[VIDEO_BUF_NUM-10]; //Toby 130815
VIDEO_BUF_MNG P2PBusyVideoBufMng[10]; //Toby 130815
u8 video_double_field_flag=0;
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
int pbuf_used_num=0;
 
u8 Video_60fps_flag     =0;//Lucian: QVGA 60 fps, preview 只呈現30 fps. 特例處理
u8 show_flag = 0;

//u8 mpeg4IVOP_period=30;
/*BJ 0530 E*/
/* cytsai: for armulator only */
/* picture index */
u32 VideoPictureIndex;  /*BJ 0530 S*/   /*CY 0907*/
u32 NVOPCnt;
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
  #if(VIDEO_RESOLUTION_SEL== VIDEO_VGA_IN_VGA_OUT)
    u32 mpeg4Width = 640,  mpeg4Height = 480;        /*CY 0907*/
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_VGA_OUT)
    u32 mpeg4Width = 640,  mpeg4Height = 480;        /*CY 0907*/
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_1920x1440DECI3x3TOVGA)
    u32 mpeg4Width = 640,  mpeg4Height = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_HD_OUT)
    u32 mpeg4Width = 1280, mpeg4Height = 720;        /*CY 0907*/
  #endif
#else
    u32 mpeg4Width = 640,  mpeg4Height = 480;        /*CY 0907*/  
#endif
u32 Cal_FileTime_Start_Idx;
u8 TVout_Generate_Pause_Frame;
u8 ASF_set_interlace_flag;    


DEF_RATECONTROL_PARA mpRateControlParam;

int RF_Bitrate_limit	= 1701;	
int RF_Bitrate_limit_H	= 1100;

/*
 *********************************************************************************************************
 * Extern Varaibel
 *********************************************************************************************************
 */
#if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
extern u8 sysEnZoom;
#endif
 
extern unsigned int asfVideoFrameCount;
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];

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

extern int P2PSentByteCnt;
extern int P2PTxBitRate[1];
extern int P2PTxBufFullness[1];
extern u32 P2PVideoBufReadIdx[1];
extern int pbuf_used_num; //Frame number in WIFI buffer 


#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
extern u8 ResetPlayback;  			//Lsk 090401 : reset playback system 
#endif
extern u8 TvOutMode;
extern u8      SetIVOP;
extern s32 mp4_avifrmcnt, isu_avifrmcnt;
#if DCF_WRITE_STATISTIC
extern u32 dcfWriteAccum;
extern u32 dcfMpegBufRemain;
#endif

extern u8 BandWidthControl;

extern u32 VideoTimeStatistics;
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
#if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
extern u8  nMotorTime;
#endif
#if USB2WIFI_SUPPORT
extern u32 USBVideoBufReadIdx[];
extern u32 USBAudioBufReadIdx[];
extern u8 Change_RSE;
extern u8 Reset_RES;
#endif

u8 filecon;
u8 splitmenu = 0; // 0: jpeg decode 滿屏(800*480), 1; jpeg decode 4分割 playback 畫面(400*480), 2: jpeg decode 滿屏(640*480)
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

extern void RCQ2_init(DEF_RATECONTROL_PARA *pRateControlParam);
extern int	check_pbuf_mem();

/*BJ 0530 S*/
s32 mpeg4Output1Frame(u8* , s64* , u32* , u32*, u32*);
s32 mpeg4Coding1Frame(u8*, u32, s64*, u32*, u32, u32, u32*);
u32 mpeg4PutVOPHeader(u8*, u32, s64*, u32*,u32);
u32 mpeg4PutHeader(u8*, u8*, u8, u32);
/*BJ 0530 E*/
s32 rfiuTxRateControl_Local(int BitRate,
                                   int BufFullness,
                                   int VideoInFrameRate,
                                   int SizeLevel,
                                   unsigned int RealBitRate);

s32 rfiuTxRateControl_P2P(int BitRate,
                                 int BufFullness,
                                 int VideoInFrameRate,
                                 int SizeLevel,
                                 int P2PVideoQuality,
                                 unsigned int RealBitRate);

s32 mpeg4Decoding1Frame(MP4Dec_Bits* Bits, u32 BitsLen,u8 write2dispbuf_en );  /* Peter: 0707 */

int mpeg4ModifyTargetBitRate(int NewBitRate);
int mpeg4ModifyQp(short Qp,int Type);


/* yc:0814 S */
void  Output_Sem(void);
/* yc:0814 E */

/*Peter 1109 S*/
/*

Routine Description:

    Initialize MPEG-4 encoder/decoder.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mpeg4Init(void)
{
    int i;  

    // MPEG-4 software reset
    SYS_RSTCTL = 0x00000100;
    for (i = 0; i < 256; i++);
    SYS_RSTCTL = 0;

    mpeg4ReadySemEvt    = OSSemCreate(1);
    
    for(i=0; i<MULTI_CHANNEL_MAX; i++)
        VideoRTPCmpSemEvt[i]   = OSSemCreate(0);
//#if MULTI_CHANNEL_VIDEO_REC
#if 0
    VideoCpleSemEvt     = OSSemCreate(0); /*BJ 0530 S*/    
#else

    /* initialize video buffer */
    for(i = 0; i < VIDEO_BUF_NUM; i++) {        
        VideoBufMng[i].buffer = VideoBuf;
    }

    // initialize rate control parameter
    memset(&mpRateControlParam,0,sizeof(DEF_RATECONTROL_PARA));
    RCQ2_init(&mpRateControlParam);
    
    /* Create the semaphore */
    VideoTrgSemEvt      = OSSemCreate(VIDEO_BUF_NUM - 2); /* guarded for ping-pong buffer */
    VideoCmpSemEvt      = OSSemCreate(0);   	
    VideoCpleSemEvt     = OSSemCreate(0); /*BJ 0530 S*/
    
    /* Create the task */
    OSTaskCreate(MPEG4_TASK, MPEG4_TASK_PARAMETER, MPEG4_TASK_STACK, MPEG4_TASK_PRIORITY); 
    mpeg4SuspendTask();

#endif

    return 1;   
}


/*Peter 1109 E*/

/*Peter 1109 S*/
/*

Routine Description:

    The FIQ handler of MPEG-4 encoder/decoder.

Arguments:

    None.

Return Value:

    None.

*/
void mpeg4IntHandler(void)
{
#if(VIDEO_CODEC_OPTION == MPEG4_CODEC)
    u32 intStat     = Mpeg4IntStat;
    u32 temp;

    MPEG4_Status    = intStat;
    if (intStat & 0x00000001)   // encoder finish   
    {
#if MPEG_DEBUG_ENA_LUCIAN
        gpioSetLevel(0, 1, 0);
#endif

        if (mpegflag == 1)
        {
            OSSemPost(VideoCpleSemEvt);
            mpegflag = 0;
            mp4_avifrmcnt++;
        }
        else if (mpegflag == 2)
        {
            OSSemPost(VideoCpleSemEvt);
            mpegflag = 0;
        }
        
    } 
    else if(intStat & 0x00000006) 
    {   // decoder finish
#if MPEG_DEBUG_ENA_LUCIAN
        gpioSetLevel(0, 1, 0);
#endif


        mpegflag = 0;
        VideoPictureIndex++;
        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
        OSSemPost(VideoCpleSemEvt);
    } 
    else 
    {
        mpegflag = 0;
        DEBUG_MP4("mpeg4IntHandler(0x%08X,0x%08X,0x%08X) error!!!\n", MPEG4_Status, intStat, IntFiqInput);
        if((IntFiqInput & INT_FIQMASK_MPEG4) == 0)
            DEBUG_MP4("IntFiqInput(0x%08X) & INT_FIQMASK_MPEG4(0x%08X) == 0, something wrong!!!\n", IntFiqInput, INT_FIQMASK_MPEG4);
    }
/*Peter 0707 E*/
#endif
}
/*Peter 1109 E*/

/*

Routine Description:

    The test routine of MPEG-4 encoder/decoder.

Arguments:

    None.

Return Value:

    None.

*/
void mpeg4Test(void)
{
    
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
    u32 mpeg4VdPacketSize;
    
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
    mpeg4VdPacketSize = MPEG4_VDPACKET_SIZE;
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    Mpeg4ErrResil = mpeg4VdPacketSize | MPEG4_RESY_ENA;
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


s32 mpeg4GetVideoResolution(int *pwidth, int *pheight)
{
   *pwidth =mpeg4Width;
   *pheight=mpeg4Height;
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

#if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)
    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        MultiChannelMpeg4SetVideoFrameRate(framerate, &VideoClipOption[i]);
    }
#endif
    
    return 1;
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
                #if(HW_BOARD_OPTION == JSY_DVRBOX)
                mpRateControlParam.TargetBitrate    =  28 * 1000;
                #else
                mpRateControlParam.TargetBitrate    = 133 * 1000;
                #endif
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
                #if(HW_BOARD_OPTION == SUNWAY_SDV)  //Lucian: 讓1GB 可錄一小時以上.
                  mpRateControlParam.TargetBitrate    = 1750 * 1000; // 2.33 Mb/sec
                  //mpRateControlParam.TargetBitrate    = 2000 * 1000; // 2.99 Mb/sec
                  mpRateControlParam.InitQP=7;
                  mpRateControlParam.QP_I=7;
                  mpRateControlParam.QP_P=7;
                #else
                  mpRateControlParam.TargetBitrate    = 2000 * 1000; // 2.99 Mb/sec
                  mpRateControlParam.InitQP=6;
                  mpRateControlParam.QP_I=6;
                  mpRateControlParam.QP_P=6;
                #endif
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

        #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
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

    Put VOL header. 

Arguments:

    pHeader - Video header.
    pHeaderSize - Video header size.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mpeg4EncodeVolHeader(u8* pHeader, u32* pHeaderSize)    /* Peter: 0711 */
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
    VolHeader[0x0d] |= (u8)(mpeg4Width >> 9); 
    VolHeader[0x0e] |= (u8)(mpeg4Width >> 1);     
    VolHeader[0x0f] |= (u8)(mpeg4Width << 7); 
    
    VolHeader[0x0f] |= (u8)(mpeg4Height >> 7);
    VolHeader[0x10] |= (u8)(mpeg4Height << 1);

    
    *pHeaderSize = sizeof(VolHeader);
    for (i = 0; i < *pHeaderSize; i++)
        *pHeader++ = VolHeader[i];  

    mpeg4VopTimeInc=0; //reset
    return 1;
}

/*BJ 0530 S*/
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
    
    /* I picture  */
    diff= (asfVideoFrameCount >= frame_idx) ? (asfVideoFrameCount-frame_idx) : (frame_idx-asfVideoFrameCount) ;
    if(diff > 1000)
    {
        DEBUG_MP4("Determine I frame overflow!\n");
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
        if(period >= asfVideoFrameCount)
        {
             DEBUG_MP4("FrameCount overflow!\n");
        }
            
        return FLAG_I_VOP;
    }    
    else 
    { /* P picture */
        return FLAG_P_VOP;
    }

}

/*Peter 1109 S*/
/*

Routine Description:

    The MPEG4 task.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
void mpeg4Task(void* pData)
{
    u32*        pFlag;
    s64*        pTime;
    s64         Time;
    u32*        pSize;
    u32*        pOffset;
    u32         bitpos; /*BJ 0530 S*/
    u8          *pBuf, *pReadBuf;
    u8          err;
    u32         NextIdx;
    u32         TotalSAD;
    u32         outLen;
    u8          write2dispbuf_en;
    s32         i, DropFrame;
    static u32  Mpeg4EncCnt = 0; //Lucina: for internal use: roundtype, reconstruct frame switch.
    //u32         Vop_Type;    /* Peter 0704 */
    u32         Vop_Result;
    u32         doublefield_thr;
    
    static  int Prev_BitRate_Level=MPEG_BITRATE_LEVEL_100;
    static  int double_field_cnt=30;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    u8 video_err = 0;
    u8 video_Playback = 0;

    MPEG4_Task_Go   = 1;    // 0: never run, 1: ever run
    
    while (1)
    {
        /*CY 0613 S*/
        Video_Pend  = 1;
        OSSemPend(VideoTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
        Video_Pend  = 0;

        /*CY 0613 E*/
        if (err != OS_NO_ERR)
        {
            DEBUG_MP4("Error: VideoTrgSemEvt is %d.\n", err);
            //return;
        }
        
        if(MPEG4_Mode) 
        {    // MPEG-4 playback
            pBuf        = VideoBufMng[VideoBufMngReadIdx].buffer;  /*BJ 0530 S*/
            pFlag       = &VideoBufMng[VideoBufMngReadIdx].flag;    
            Time        = VideoBufMng[VideoBufMngReadIdx].time; 
            pSize       = &VideoBufMng[VideoBufMngReadIdx].size;    
            Vop_Type    = (*pFlag)? 0 : 1;
            
            write2dispbuf_en    = 1;
            
			#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
			if(!ResetPlayback && (sysPlaybackForward == 0 ||  Vop_Type == I_VOP)) 
			#elif (AVSYNC == VIDEO_FOLLOW_AUDIO)
            if((sysPlaybackForward == SYS_PLAYBACK_FORWARD_X1) || 
               ((sysPlaybackForward != SYS_PLAYBACK_FORWARD_X1) && (Vop_Type == I_VOP))) 
			#endif
            {
                if(video_err == 1)
                {
                    if(VideoBufMng[VideoBufMngReadIdx].flag == 1)
                    {
                        if(video_Playback == 1)
                            sysPlaybackThumbnail = 1;
                    }
                    else
                    {
                        VideoPictureIndex++;
                        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                        OSSemPost(VideoCmpSemEvt);
                        continue;
                    }
                        
                }
                Vop_Result  = mpeg4DecodeVOP(pBuf, *pSize, write2dispbuf_en,0);

				Videodisplaytime[IsuIndex % DISPLAY_BUF_NUM]    = Time;
								
                if((video_err == 1) && (Vop_Result == 1))
                {
                    if(video_Playback == 1)
                    {
                        MainVideodisplaybuf_idx++;
                        VideoNextPresentTime = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
                    	iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
                        video_Playback = 0;
                    }
                    video_err = 0;
                }
                if(Vop_Result == 0)
                    video_err = 1;
                
                if(Vop_Result == MPEG4_N_VOP) {
                    OSSemPost(VideoCmpSemEvt);
                    continue;
                }
                //DEBUG_MP4("VideoPictureIndex = %d\n", VideoPictureIndex);
                if(write2dispbuf_en)//不啟動scaler engine. Mpeg decoding 直接decoding to display buffer.
                {
                    if(Video_60fps_flag)
                    {
                        if(show_flag)
                        {
                            isuStatus_OnRunning = 0;
                            IsuIndex++;
                            show_flag = 0;
                        }
                        else
                        {
                            show_flag = 1;
                        }
                    }
                    else
                    {
                        isuStatus_OnRunning = 0;
                        IsuIndex++;
                    }
                    
                }
                else
                {
                    // scaling MPEG-4 image to fit display format
                    if(IsuIndex != 0) 
                    {
                        
                        OSSemPend(isuSemEvt, 10 ,&err);
                     #if DINAMICALLY_POWER_MANAGEMENT
                        sysISU_disable();
                     #endif          
                        if (err != OS_NO_ERR) {
                            DEBUG_MP4("Error: isuSemEvt(playback mode) is %d.\n", err);
                        }
                    }
                }
                
                //DEBUG_MP4("IsuIndex = %d ,%d\n", IsuIndex,MainVideodisplaybuf_idx);
				#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
				#if (PLAYBACK_METHOD==PLAYBACK_IN_IIS_ISR)
				while(((IsuIndex >= (MainVideodisplaybuf_idx + DISPLAY_BUF_NUM - 3)) || (isuStatus_OnRunning == 1)) && !ResetPlayback && sysPlaybackVideoStop == 0) /*Peter 1113 S*/    //Lsk 090410 check it    //Lsk 090417 : avoid deadlock when press stop playback           
				#else
				while(((IsuIndex >= (MainVideodisplaybuf_idx + DISPLAY_BUF_NUM - 3)) || (isuStatus_OnRunning == 1)) && !ResetPlayback && sysPlaybackVideoStop == 0) /*Peter 1113 S*/    //Lsk 090410 check it    //Lsk 090417 : avoid deadlock when press stop playback           
				#endif
				#elif (AVSYNC == VIDEO_FOLLOW_AUDIO)
                while((IsuIndex >= (MainVideodisplaybuf_idx + DISPLAY_BUF_NUM - 2)) || (isuStatus_OnRunning == 1)) /*Peter 1113 S*/                   
				#endif
                {
                //    DEBUG_MP4("ISU waiting for display, IsuIndex = %d, MainVideodisplaybuf_idx = %d\n", IsuIndex, MainVideodisplaybuf_idx);
                    OSTimeDly(1);
                }

                
                if(!write2dispbuf_en)
                {
                #if DINAMICALLY_POWER_MANAGEMENT
                    sysISU_enable();
                #endif          
                    isuPlayback_av(mpeg4outputbuf[IsuIndex % 3],
                                   MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM], 
                                   Mp4Dec_opt.Width, 
                                   Mp4Dec_opt.Height);
                }

            } 
			#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
			else if(Vop_Type == P_VOP)
			#elif (AVSYNC == VIDEO_FOLLOW_AUDIO)
            else if((sysPlaybackForward != SYS_PLAYBACK_FORWARD_X1) && (Vop_Type == P_VOP)) 
			#endif
            {
                while((((VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM) == VideoBufMngWriteIdx && CloseFlag) && (sysPlaybackVideoStop == 0) && (ResetPlayback == 0)) //Lsk 090417 : avoid deadlock when press stop playback
                {
                    OSTimeDly(1);
                }
                VideoPictureIndex++;
                VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
            }
            if(video_err == 1)
            {
                video_Playback = sysPlaybackThumbnail;
                if(sysPlaybackThumbnail == 1)
                    sysPlaybackThumbnail = 0;
            }
            
        } 
        else //--------------------------- MPEG-4 capture Mode------------------------------------
        {  
            pBuf        = VideoBufMng[VideoBufMngWriteIdx].buffer;  /*BJ 0530 S*/
            pFlag       = &VideoBufMng[VideoBufMngWriteIdx].flag; 
            pTime       = &VideoBufMng[VideoBufMngWriteIdx].time; 
            pSize       = &VideoBufMng[VideoBufMngWriteIdx].size; 
            pOffset     = &VideoBufMng[VideoBufMngWriteIdx].offset;
            *pFlag      = DeterminePictureType(VideoPictureIndex, IVOP_PERIOD);   // I: 1, P: 0
            Vop_Type    = (*pFlag)? 0 : 1; // 0: I frame, 1: P frame
            
            if(!Vop_Type) 
            {     // I frame
            #if DCF_WRITE_STATISTIC
                DEBUG_MP4(" I%d,%d ",dcfWriteAccum,(dcfMpegBufRemain>>10));
                dcfWriteAccum=0;

                if( (asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && (EventTrigger==0) )
                {
                   //MPEG4 Encoder put bitstream in the SDRAM buufer.
                   //此時不做 buffer moniting
                }
                else
                {
                    if(asfVopCount > 300) //Lucian: 10 sec 後開始做 rate control.
                    {
                        if(dcfMpegBufRemain < MPEG4_MAX_BUF_SIZE*2/10)  // remain < 20%
                        {
                           if(Prev_BitRate_Level != MPEG_BITRATE_LEVEL_60)
                              mpeg4ConfigQualityFrameRate(MPEG_BITRATE_LEVEL_60);
                           Prev_BitRate_Level =MPEG_BITRATE_LEVEL_60;
                        }
                        else if(dcfMpegBufRemain < MPEG4_MAX_BUF_SIZE*4/10) //remain < 40%
                        {
                           if(Prev_BitRate_Level != MPEG_BITRATE_LEVEL_80)
                              mpeg4ConfigQualityFrameRate(MPEG_BITRATE_LEVEL_80);
                           Prev_BitRate_Level =MPEG_BITRATE_LEVEL_80;
                        }
                        else if(dcfMpegBufRemain > MPEG4_MAX_BUF_SIZE*7/10) //remain > 70%
                        {
                           if(Prev_BitRate_Level != MPEG_BITRATE_LEVEL_100)
                              mpeg4ConfigQualityFrameRate(MPEG_BITRATE_LEVEL_100);
                           Prev_BitRate_Level =MPEG_BITRATE_LEVEL_100;
                        }
                    }
                }
            #else
                DEBUG_MP4("I");
            #endif
            
            #if CDVR_LOG
                OSSemPend(LogFileSemEvt, 10, &err);
                LogFileIndex[LogFileCurrent]    = szLogFile;
                LogFileCurrent                  = (LogFileCurrent + 1) % LOG_INDEX_NUM;
                OSSemPost(LogFileSemEvt);
            #endif
            }
                        
            //--------Video rate control: Calculate QP-----//            
            if(mpRateControlParam.enable_ratecontrol)
            {
                 //DEBUG_MP4("--RCQ2_QuantAdjust---\n");
                 if (Vop_Type == 0) //I frame
                 {
                     dftMpeg4Quality    = mpRateControlParam.InitQP = RCQ2_QuantAdjust(&mpRateControlParam, 0, 0);
                 }
                 else //P frame
                 {
                     dftMpeg4Quality    = mpRateControlParam.InitQP = RCQ2_QuantAdjust(&mpRateControlParam,
                                                                                       mpRateControlParam.Int_prev_PMad,
                                                                                       1);
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
                        else if( mpeg4Width != 320 )
                        {
                          doublefield_thr=video_double_field_flag?  160:140;
                          if( (mpRateControlParam.Avg_PMad > doublefield_thr) || (dftMpeg4Quality>14) )
                          {
                               video_double_field_flag = 1;
                               double_field_cnt=0;
                          }
                          else
                          {
                          #if 1  //Lucian: 一但double field 啟動, 則維持30 frames.
                              if(double_field_cnt<30)
                              {
                                 double_field_cnt++;
                                 video_double_field_flag = 1;
                              }
                              else
                                 video_double_field_flag = 0;
                          #else
                              if(dftMpeg4Quality>9)
                                 video_double_field_flag = 1;
                              else
                              {
                                 video_double_field_flag = 0;
                              }
                          #endif
                          }
                        }
                      #endif
                        else //for QVGA
                            video_double_field_flag = 0;
                 #endif    
                 }
                 //if( (dftMpeg4Quality>20-3) )
                 //DEBUG_MP4("%d ",dftMpeg4Quality);
                 
            }

             #if (CIU1_BOB_REPLACE_MPEG_DF && (CHIP_OPTION > CHIP_A1016A) ) //Lucian: 不用 Mpeg4 double field,起動 ciu mob mode
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

            //------Video FIFO Management: 避免 Read/Write pointer overlay. ------//
            if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
                if((WantChangeFile == 1) && (GetLastVideo == 0) && (GetLastAudio)) {
                    OS_ENTER_CRITICAL();
                    LastVideo           = VideoBufMngWriteIdx;
                    GetLastVideo        = 1;
                    OS_EXIT_CRITICAL();

                #if CDVR_LOG
                    ChangeLogFileStartAddress();
                #endif
                }
            }

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

                    // GetLastVideo一直為0, 可能會造成dead lock, 所以要更新GetLastVideo
                    if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
                        if((WantChangeFile == 1) && (GetLastVideo == 0) && (GetLastAudio)) {
                            OS_ENTER_CRITICAL();
                            LastVideo           = VideoBufMngWriteIdx;
                            GetLastVideo        = 1;
                            OS_EXIT_CRITICAL();

                        #if CDVR_LOG
                            ChangeLogFileStartAddress();
                        #endif
                        }
                    }
                }
            }
        #if DCF_WRITE_STATISTIC
            else
            {
               dcfMpegBufRemain=MPEG4_MAX_BUF_SIZE-(pBuf-pReadBuf);
            }
        #endif
            //---------- Video Encoding---------//
            *pTime=0;

        #if 1 //Lucian: frame rate control
            if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_30)
            {
                DropFrame=0;
            }
            else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_15)
            {
                DropFrame=1;
            }
            else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_10)
            {
                DropFrame=2;
            }
            else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_5)
            {
                DropFrame=5;
            }

          #if MULTI_CHANNEL_SUPPORT
            #if (MULTI_CHANNEL_SEL & 0x01)
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(isuSemEvt, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
            #elif (MULTI_CHANNEL_SEL & 0x02)
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH1, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
            #elif (MULTI_CHANNEL_SEL & 0x04)
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH2, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
            #elif (MULTI_CHANNEL_SEL & 0x08)
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH3, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
            #elif (MULTI_CHANNEL_SEL & 0x10)
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH4, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
            #endif
          #else   // MULTI_CHANNEL_SUPPORT == 0
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(isuSemEvt, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
          #endif  // #if MULTI_CHANNEL_SUPPORT
        #endif  //#if 1 //Lucian: frame rate control
            #if(RF_DEMO_SETTING == 1)
                   dftMpeg4Quality = 5;
            #endif
            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
            mpeg4Output1Frame(pBuf, pTime, pSize, &Mpeg4EncCnt, pOffset);
            #elif (VIDEO_CODEC_OPTION == H264_CODEC) 
            video_info.StreamBuf = pBuf;
            video_info.FrameType = *pFlag;
            video_info.FrameIdx  = VideoPictureIndex;  
            video_info.FrameTime = pTime;
            video_info.pSize     = pSize;            

            H264Enc_CompressOneFrame(&video_info,&H264Enc_cfg);
            #endif
        
            OS_ENTER_CRITICAL();
            VideoTimeStatistics += *pTime;
            
            if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && (VideoTimeStatistics >= (asfSectionTime * 1000)))            
            {
                SetIVOP = 1;             
            }            
            if((asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && (VideoTimeStatistics >= (asfRecTimeLen * 1000)))            
            {
                SetIVOP = 1;             
            }
            OS_EXIT_CRITICAL();

            //--------Rebuild RC model of Rate control ------//
            if(mpRateControlParam.enable_ratecontrol)
            {
                TotalSAD    = Mpeg4SadSum;
                outLen      = *pSize; //Video size. 
                if (Vop_Type !=I_VOP)
                {
                    mpRateControlParam.Int_prev_PMad = (TotalSAD*4/mpeg4Height)*1024/mpeg4Width;  //FixRC

                    if( (mpRateControlParam.PMad_cnt & 0x3) == 0 )  //Mod 4: 每四張統計一次MAD.
                    {
                       mpRateControlParam.Avg_PMad = mpRateControlParam.Sum_PMad/4;
                       mpRateControlParam.Sum_PMad=0;
                       //DEBUG_MP4("(%d) ",mpRateControlParam.Avg_PMad);
                    }
                    mpRateControlParam.Sum_PMad += mpRateControlParam.Int_prev_PMad;
                    mpRateControlParam.PMad_cnt ++;
                }
                //DEBUG_MP4("--RCQ2_Update2OrderModel---\n");   
                
                if(mpRateControlParam.enable_ratecontrol)
                {
                    RCQ2_Update2OrderModel(&mpRateControlParam,
                                           outLen * 8,
                                           Vop_Type);
                }

            }
            
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
        }
        
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
/*Peter 1109 E*/

#define TX_BUFFCNTL_THR_HD_LOW   30

#define TX_BUFFCNTL_THR_VGA_LOW  15

extern u32 P2PVideoBufReadIdx[1];

//#define RF_Bitrate_limit		1701	
//#define RF_Bitrate_limit_H		1100

#define WIFI_POWER_THRESHOLD_LOW	45
#define WIFI_POWER_THRESHOLD_MID	52
#define WIFI_POWER_THRESHOLD_HIGH	59

// WIFI BUFFER MAX: 290
#define WIFI_BUFFER_THRESHOLD_LOW	50
#define WIFI_BUFFER_THRESHOLD_MID	100
#define WIFI_BUFFER_THRESHOLD_HIGH	200


#if RFIU_SUPPORT

int rfiuGetTxBitRate(int SizeLevel){
    static int lastP2PTxBitRate = -1;
#if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
    int TXBitRate = 0;
    static int LastLevel = -1;
    if(sys8211TXWifiStat==MR8211_ENTER_WIFI){
		pbuf_used_num = check_pbuf_mem();
        if(SizeLevel == MPEG4_REC_QVGA){
            if((rfiu_TX_WifiPower <= WIFI_POWER_THRESHOLD_LOW) && (pbuf_used_num <= WIFI_BUFFER_THRESHOLD_LOW) && (LastLevel <= 2)){
                P2PTxBitRate[0] = 1701;
                LastLevel = 1;
            }
            else if((rfiu_TX_WifiPower <= WIFI_POWER_THRESHOLD_MID) && (pbuf_used_num <= WIFI_BUFFER_THRESHOLD_MID) && (LastLevel <= 3)){
                P2PTxBitRate[0] = 1100;
                LastLevel = 2;
            }
            else if((rfiu_TX_WifiPower <= WIFI_POWER_THRESHOLD_HIGH) && (pbuf_used_num <= WIFI_BUFFER_THRESHOLD_HIGH) && (LastLevel <= 4)){
                P2PTxBitRate[0] = 600;
                LastLevel = 3;
            }
            else{
	            P2PTxBitRate[0] = 300;
	            LastLevel = 4;
            }
            	
        }		
        else if(SizeLevel == MPEG4_REC_VGA){
            if((rfiu_TX_WifiPower <= WIFI_POWER_THRESHOLD_LOW) && (pbuf_used_num <= WIFI_BUFFER_THRESHOLD_LOW) && (LastLevel <= 2)){
                P2PTxBitRate[0] = 1701;
                LastLevel = 1;
            }
            else if((rfiu_TX_WifiPower <= WIFI_POWER_THRESHOLD_MID) && (pbuf_used_num <= WIFI_BUFFER_THRESHOLD_MID) && (LastLevel <= 3)){
                P2PTxBitRate[0] = 1100;
                LastLevel = 2;
            }
            else if((rfiu_TX_WifiPower <= WIFI_POWER_THRESHOLD_HIGH) && (pbuf_used_num <= WIFI_BUFFER_THRESHOLD_HIGH) && (LastLevel <= 4)){
                P2PTxBitRate[0] = 600;
                LastLevel = 3;
            }
            else{
	            P2PTxBitRate[0] = 300;
	            LastLevel = 4;
            }
            	
        }
        else if (SizeLevel == MPEG4_REC_HD){
            if((rfiu_TX_WifiPower <= WIFI_POWER_THRESHOLD_LOW) && (pbuf_used_num <= WIFI_BUFFER_THRESHOLD_LOW) && (LastLevel <= 2)){
                P2PTxBitRate[0] = 1100;
                LastLevel = 1;
            }
            else if((rfiu_TX_WifiPower <= WIFI_POWER_THRESHOLD_MID) && (pbuf_used_num <= WIFI_BUFFER_THRESHOLD_MID) && (LastLevel <= 3)){
                P2PTxBitRate[0] = 800;
                LastLevel = 2;
            }
            else if((rfiu_TX_WifiPower <= WIFI_POWER_THRESHOLD_HIGH) && (pbuf_used_num <= WIFI_BUFFER_THRESHOLD_HIGH) && (LastLevel <= 4)){
                P2PTxBitRate[0] = 500;
                LastLevel = 3;
            }            
            else{
				P2PTxBitRate[0] = 300;
				LastLevel = 4;
            }
        }
    }
    
    if((RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0)) && (sys8211TXWifiStat==MR8211_ENTER_WIFI)) // both RF and wifi connected
        TXBitRate = ((P2PTxBitRate[0])<(rfiuTXBitRate[0]))? (P2PTxBitRate[0]):(rfiuTXBitRate[0]);
    else if((sys8211TXWifiStat==MR8211_ENTER_WIFI)) // wifi connected only
        TXBitRate = P2PTxBitRate[0];
    else // RF connected only
        TXBitRate = rfiuTXBitRate[0];

    return TXBitRate;
    
#else
    return rfiuTXBitRate[0];
#endif

}

int rfiuGetTxBufFullness(int *MaxBufFullness){
#if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
    int TxBufFullness = 0;
    static u8 continusHighBuf = 0;
    if(sys8211TXWifiStat==MR8211_ENTER_WIFI){
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
    
    if((RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0)) && (sys8211TXWifiStat==MR8211_ENTER_WIFI)) // both RF and wifi connected
        TxBufFullness = ((P2PTxBufFullness[0])>(rfiuTxBufFullness[0]))? (P2PTxBufFullness[0]):(rfiuTxBufFullness[0]);    
    if((sys8211TXWifiStat==MR8211_ENTER_WIFI)) // wifi connected only
        TxBufFullness = P2PTxBufFullness[0];
    else  // RF connected only
        TxBufFullness = rfiuTxBufFullness[0];

    return TxBufFullness;
    
#else
    return rfiuTxBufFullness[0];
#endif

}

void rfiuTXMpeg4EncTask(void* pData)
{
    u32*        pFlag;
    s64*        pTime;
    s64         Time;
    u32*        pSize;
    u32*        pOffset;
    u32         bitpos; /*BJ 0530 S*/
    u8          *pBuf, *pReadBuf;
    u8          err;
    u32         NextIdx;
    u32         TotalSAD;
    u32         outLen;
    u8          write2dispbuf_en;
    s32         i, DropFrame;
    u32         Vop_Result;
    u32         doublefield_thr;
    int         SizeLevel;
    int         TxBufFullness;
    int         Drop_ext2;
#if RF_TX_OPTIMIZE      
    int         RFUnit;
#endif

    //Lucian: 多Tx時,要改為Global Array. 
    int Prev_BitRate_Level=MPEG_BITRATE_LEVEL_100;
    int double_field_cnt=30;
    s32 Drop_ext=0;
    int  QP_ext=0;
    int  QP_ext2=0;
    u32  Mpeg4EncCnt = 0; //Lucina: for internal use: roundtype, reconstruct frame switch.
    u32  TXErrMonitor=0;
    u32  PrevTXRunCnt=0xffffffff;
#if (FORECE_MPEG_DROP_1_10_FPS )
    u32  TotalFrameCnt=0;
    u32  DropFrameCnt=0;
#endif    
    //----//
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
    unsigned int ByteCnt;
    unsigned int t1,t2,dt,RealBitRate;
    unsigned int VideoCnt;
    unsigned int AvgFrameSize;
    int TXBitRate, MaxBufFullness = 0, wifi_offset = 0;;
#if RF_TX_AUTOPOWER
    int RFPowerLv=0;
#endif
    //---------------------------//
#if RF_TX_OPTIMIZE  
    RFUnit= (int)pData;
    VideoBufMngWriteIdx=0;
    VideoPictureIndex  = 0;
    asfVideoFrameCount=0;
    gRfiu_MpegEnc_Sta[RFUnit]=RFI_MPEGENC_TASK_RUNNING;
#endif
  #if USB2WIFI_SUPPORT
    USBVideoBufReadIdx[0]  = 0; 
    Change_RSE = 0;
    Reset_RES = 0;
    USBAudioBufReadIdx[0] = (iisSounBufMngWriteIdx) % IIS_BUF_NUM; 
  #endif

  	P2PVideoBufReadIdx[0] = 0; //Sean: 20170623
    MPEG4_Task_Go   = 1;    // 0: never run, 1: ever run


    //====Set Initial Bit Rate====//
    if(mpeg4Width <= 360) //QVGA or CIF
    {
       rfiuTXBitRate[0]=700; //Unit: Kbps
       SizeLevel=MPEG4_REC_QVGA;
    }
    else if(mpeg4Width<=720) //VGA or D1
    {
       rfiuTXBitRate[0]=2400; //Unit: Kbps
       SizeLevel=MPEG4_REC_VGA;
    }
    else if(mpeg4Width<=1440) //HD or 4D1
    {
       rfiuTXBitRate[0]=2800; //Unit: Kbps
       SizeLevel=MPEG4_REC_HD;
    }
    else
    {
       rfiuTXBitRate[0]=2400; //Unit: Kbps
       SizeLevel=MPEG4_REC_VGA;
    }
    //====//
    
    rfiuTxBufFullness[0]=0;
    ByteCnt=0;
    DropFrame=0;
    QP_ext=0;
    QP_ext2=0;
    VideoCnt=0;

    timerCountRead(guiRFTimerID, &t1);

    DEBUG_MP4("-------rfiuTXMpeg4EncTask--------\n");
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
	       SizeLevel=MPEG4_REC_VGA;
	    }
	    else if(mpeg4Width<=1440) //HD or 4D1
	    {
	       SizeLevel=MPEG4_REC_HD;
	    }
	    else
	    {
	       SizeLevel=MPEG4_REC_VGA;
	    }
        
        pBuf        = VideoBufMng[VideoBufMngWriteIdx].buffer;  /*BJ 0530 S*/
        pFlag       = &VideoBufMng[VideoBufMngWriteIdx].flag; 
        pTime       = &VideoBufMng[VideoBufMngWriteIdx].time; 
        pSize       = &VideoBufMng[VideoBufMngWriteIdx].size; 
        pOffset     = &VideoBufMng[VideoBufMngWriteIdx].offset; 

        if(RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))  
        {
            if( rfiuRX_OpMode & RFIU_RX_OPMODE_P2P)
            {
               if(rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_HD_5_FPS)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
               else if(rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_HD_7_FPS)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0  
               else if(rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_VGA_30_FPS)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0   
               else if(rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_VGA_15_FPS)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
               else if(rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_VGA_10_FPS)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0   
               else if(rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_15_FPS)
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
               else if(rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_10_FPS)
                   *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
               else if(rfiuRX_P2pVideoQuality == RF_P2PVdoQalt_QVGA_7_FPS)
                   *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0    
               else
                  *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
            }
            else
            {
                 if(SizeLevel==MPEG4_REC_HD)
                 {
                 #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
                    *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
                 #else
                    *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD/2);   // I: 1, P: 0
                 #endif
                 }   
                 else
                 {
                 #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
                    if(sys8211TXWifiStat==MR8211_ENTER_WIFI)
                    {
                       *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
                    }
                    else
                    {
                       *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
                    }
                 #else
                    *pFlag      = DeterminePictureType(VideoPictureIndex, RFIU_IVOP_PERIOD);   // I: 1, P: 0
                 #endif
                 }   
            }
        }
     #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
        else if(sys8211TXWifiStat==MR8211_ENTER_WIFI) // when Link broken.
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
            *pFlag      = DeterminePictureType(VideoPictureIndex, 0);   // I: 1, P: 0
         
        Vop_Type    = (*pFlag)? I_VOP : P_VOP; // 0: I frame, 1: P frame

    
        if(Vop_Type == I_VOP) 
        {     // I frame
            dcfWriteAccum=0;
            timerCountRead(guiRFTimerID, &t2);
             if(t1>t2)
              dt=t1-t2;
            else
              dt=(t1+TimerGetTimerCounter(TIMER_7))-t2;

            if(dt==0)
                dt=1;
  
            RealBitRate = ((ByteCnt+64)*8*10/dt); //Lucian: Video + Header 

            TXBitRate = rfiuGetTxBitRate(SizeLevel);
            
            if( RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))
            {
                if( rfiuRX_OpMode & RFIU_RX_OPMODE_P2P)
                {
                   DropFrame=rfiuTxRateControl_P2P(rfiuTXBitRate[0],rfiuTxBufFullness[0],
                                                   rfiuVideoInFrameRate,SizeLevel,
                                                   rfiuRX_P2pVideoQuality,RealBitRate);
                }
                else
                {
                #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
                   if(sys8211TXWifiStat==MR8211_ENTER_WIFI)
                   {
                       if(rfiu_TX_P2pVideoQuality == RFWIFI_P2P_QUALITY_HIGH)
                       {
                          if(TXBitRate > RF_Bitrate_limit_H)
                              TXBitRate = RF_Bitrate_limit_H;
                       }
                       else
                       {
                          if(TXBitRate > RF_Bitrate_limit)
                              TXBitRate =RF_Bitrate_limit;
                       }

                   }
                   DropFrame=rfiuTxRateControl_Local(TXBitRate,
                                                     TxBufFullness,
                                                     rfiuVideoInFrameRate,
                                                     SizeLevel,
                                                     RealBitRate);
                #else
                   DropFrame=rfiuTxRateControl_Local(TXBitRate,
                                                     TxBufFullness,
                                                     rfiuVideoInFrameRate,
                                                     SizeLevel,
                                                     RealBitRate);
                #endif
                }
            #if RF_TX_AUTOPOWER
                if(TXBitRate>2000)
                {
                    RFPowerLv +=1;
                }
                else if(TXBitRate>1700)
                {
                }
                else
                {
                    RFPowerLv -=1;
                }

                if(RFPowerLv<0)
                    RFPowerLv=0;
                if(RFPowerLv>2)
                    RFPowerLv=2;

                DEBUG_MP4("Power LV=%d\n",RFPowerLv);
                
                OS_ENTER_CRITICAL();
                #if(RFIC_SEL == RFIC_A7130_4M)
                 switch(RFPowerLv)
                 {
                    case 0:
                       A7130_WriteReg_B1(0x2d, 0x37); // 17 dBm
                       break;
                    case 1:
                       A7130_WriteReg_B1(0x2d, 0x36); // 15 dBm
                       break;
                    case 2:
                       A7130_WriteReg_B1(0x2d, 0x35); // 13 dBm
                       break;                       
                 }
                #endif
                OS_EXIT_CRITICAL();
            #endif
            }
       #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
            else if(sys8211TXWifiStat==MR8211_ENTER_WIFI)  //when link broken
            {
                #if 0
                if(rfiu_TX_P2pVideoQuality == RFWIFI_P2P_QUALITY_HIGH)
                {
                    rfiuTXBitRate[0]=RF_Bitrate_limit_H;
                }
                else if(rfiu_TX_P2pVideoQuality == RFWIFI_P2P_QUALITY_MEDI)
                {
                    rfiuTXBitRate[0]=RF_Bitrate_limit;
                }
                else //Qaulity Low
                {
                    rfiuTXBitRate[0]=RF_Bitrate_limit;
                }  
                rfiuTxBufFullness[0]=0;
                #endif                
                DropFrame=rfiuTxRateControl_Local(TXBitRate,
                                                  TxBufFullness,
                                                  rfiuVideoInFrameRate,
                                                  SizeLevel,
                                                  RealBitRate);
            #if RF_TX_AUTOPOWER
                OS_ENTER_CRITICAL();
                #if(RFIC_SEL == RFIC_A7130_4M)
                    A7130_WriteReg_B1(0x2d, 0x37); // 17 dBm
                #endif
                OS_EXIT_CRITICAL();
            #endif
            }
       #endif
            else
            {
            #if RF_TX_OPTIMIZE      
                DropFrame=rfiuTxRateControl_Local(150,
                                                  0,
                                                  rfiuVideoInFrameRate,
                                                  SizeLevel,
                                                  RealBitRate);  
                
                    
            #else
                DropFrame=rfiuTxRateControl_Local(150,
                                                  0,
                                                  rfiuVideoInFrameRate,
                                                  SizeLevel,
                                                  RealBitRate); 
            #endif
            #if RF_TX_AUTOPOWER
                OS_ENTER_CRITICAL();
                #if(RFIC_SEL == RFIC_A7130_4M)
                    A7130_WriteReg_B1(0x2d, 0x37); // 17 dBm
                #endif
                OS_EXIT_CRITICAL();
            #endif
            
            }

            Drop_ext=0;
            
          #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)  
            if(sys8211TXWifiStat==MR8211_ENTER_WIFI)  
            {
                DEBUG_MP4("==WIFI:%d,%d==\n",rfiu_TX_WifiCHNum,rfiu_TX_WifiPower);
                if(rfiu_TX_P2pVideoQuality == RFWIFI_P2P_QUALITY_HIGH)
                {
                    if(TXBitRate > RF_Bitrate_limit_H)
                       TXBitRate = RF_Bitrate_limit_H;
                }
                else
                {
                    if(TXBitRate > RF_Bitrate_limit)
                       TXBitRate = RF_Bitrate_limit;
                }
                AvgFrameSize=(TXBitRate-128)/8/mpRateControlParam.Framerate;
            }
            else
               AvgFrameSize=(TXBitRate-128)/8/mpRateControlParam.Framerate;
          #else  
            AvgFrameSize=(TXBitRate-128)/8/mpRateControlParam.Framerate;
          #endif   
            DEBUG_MP4(" I:%d,%d,%d,(RF:%d,WIFI:%d),%d,%d\n",DropFrame,TxBufFullness,TXBitRate, rfiuTXBitRate[0], P2PTxBitRate[0] ,RealBitRate,ByteCnt);
            ByteCnt=0;
        #if 1
            //=====Lucian: 監視TX RF task是否當掉=====//
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
        #endif
            //QP_ext=0;
            QP_ext2=0;
            t1=t2;
            
            
        }
        else
        {   //P frame
            //-- 根據TxBuf fullness 來微調 Target bitrate--//
            //--------HD--------//
        #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)  
			if(sys8211TXWifiStat==MR8211_ENTER_WIFI){
	            if(VideoCnt>2){
					VideoCnt=0;
					TxBufFullness = rfiuGetTxBufFullness(&MaxBufFullness);
					pbuf_used_num = check_pbuf_mem();

					if(TxBufFullness>200 || pbuf_used_num > 200){
						Drop_ext = 20;
					}
					else if(TxBufFullness>150 || pbuf_used_num > 150){
						Drop_ext = 15;
					}
					else if(TxBufFullness>125 || pbuf_used_num > 125){
						Drop_ext = 10;
					}					
					else if(TxBufFullness>100 || pbuf_used_num > 100){
						Drop_ext = 5;
					}
					else if(TxBufFullness>75 || pbuf_used_num > 75){
						Drop_ext = 3;
					}						
					else if(TxBufFullness>50 || pbuf_used_num > 50){
						Drop_ext = 1;
					}					
					else 
						Drop_ext = 0;
	            }
	            //printf("TxBuf:%d, WifiBuf:%d, Drop:%d\n",TxBufFullness, pbuf_used_num, Drop_ext);
	        }
	        else
	    #endif
            if(SizeLevel == MPEG4_REC_HD)
            {
                if( RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))  
                {
                    TxBufFullness = rfiuGetTxBufFullness(&MaxBufFullness);
                    if(mpRateControlParam.enable_ratecontrol)
                    {
                       if(VideoCnt>2)
                       {
                            VideoCnt=0;
                            if(TxBufFullness>200)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                               mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *30/100);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                               mpeg4ModifyQp(10,P_VOP);
                            #endif
                               Drop_ext = 2;
                               QP_ext +=4;
                            }
                            else if(TxBufFullness>150)
                            {
                               if( Drop_ext >1 )
                               {
                               #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                  mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *30/100);
                               #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                  mpeg4ModifyQp(7,P_VOP);
                               #endif
                                  Drop_ext = 2;
                                  QP_ext +=3;
                               }
                               else
                               {
                               #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                  mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *40/100);
                               #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                  mpeg4ModifyQp(5,P_VOP);
                               #endif
                                  Drop_ext = 1;
                                  QP_ext +=3;
                               }
                            }
                            else if(TxBufFullness>100)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                               mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *55/100);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                               mpeg4ModifyQp(3,P_VOP);
                            #endif
                               Drop_ext = 1;
                               QP_ext+=3;
                            }
                            else if(TxBufFullness>70)
                            {
                               if( (Drop_ext >0)  )
                               {
                               #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                 mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *60/100);
                               #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                 mpeg4ModifyQp(3,P_VOP);
                               #endif
                                 Drop_ext = 1;
                                 QP_ext +=2;
                               }
                               else
                               {
                               #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                 mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *70/100);
                               #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                 mpeg4ModifyQp(2,P_VOP);
                               #endif
                                 Drop_ext = 0;
                                 QP_ext +=2;
                               }
                            }
                            else if(TxBufFullness>50)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                               mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *80/100);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                               mpeg4ModifyQp(2,P_VOP);
                            #endif
                               Drop_ext = 0;
                               QP_ext +=2;
                            }
                            else if(TxBufFullness>30)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                               mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *90/100);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                               mpeg4ModifyQp(1,P_VOP);
                            #endif
                               Drop_ext = 0;
                               QP_ext +=1;
                            }
                            else if(TxBufFullness>20)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                               mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *90/100);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                               mpeg4ModifyQp(1,P_VOP);
                            #endif
                               Drop_ext = 0;
                               QP_ext +=1;
                            }
                            else
                            {
                               Drop_ext = 0;
                               if(QP_ext>=2)
                                 QP_ext -=2;
                               else
                                 QP_ext=0;
                            }
                        #if RF_TX_SMOOTH_PLUS   
                            Drop_ext=0;
                            if(QP_ext>12)  
                                QP_ext=12;
                        #else
                          #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                            if(QP_ext>8)  //max to 8
                                QP_ext=8;
                          #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                            if(QP_ext>8)  //max to 8
                                QP_ext=8;
                          #endif
                        #endif
                        } 
                    
                    }
                    else
                    {
                        QP_ext=0;
                        QP_ext2=0;
                    #if 0    
                        if(TxBufFullness>250)
                           Drop_ext = 20;
                        else if(TxBufFullness>200)
                           Drop_ext = 10;
                        else if(TxBufFullness>150)
                           Drop_ext = 7;
                        else if(TxBufFullness>100)
                           Drop_ext = 5;
                        else if(TxBufFullness>70)
                           Drop_ext = 3;
                        else if(TxBufFullness>50)
                           Drop_ext = 2;
                        else if(TxBufFullness>30)
                           Drop_ext = 1;
                    #else
                        if( VideoCnt > 1)
                        {    
                            VideoCnt=0;
                            if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+200)
                            {
                               Drop_ext = 44;
                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+170)
                            {
                               if( (Drop_ext >=44)  )
                               {
                                 
                               }
                               else
                               {
                                 Drop_ext = 36;
                               }            
                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+140)
                            {
                               if( (Drop_ext >=36)  )
                               {
                                 
                               }
                               else
                               {
                                 Drop_ext = 28;
                               }                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+120)
                            {
                               if( (Drop_ext >=28)  )
                               {
                                 
                               }
                               else
                               {
                                 Drop_ext = 20;
                               }
                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+100)
                            {
                               if(Drop_ext>=20)
                               {

                               }
                               else
                               {
                                  Drop_ext = 12;
                               }
                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW+35)
                            {
                               if(Drop_ext>=12)
                               {

                               }
                               else
                               {
                                   Drop_ext = 6;
                               }
                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_HD_LOW)
                            {
                               if(Drop_ext >= 6)
                               {
                               }
                               else
                               {
                                  Drop_ext = 3;
                               }
                            }  
                            else
                            {
                               Drop_ext = 0;
                            }
                        }
                    #endif
                    }
                }
                else
                {
                   TxBufFullness = rfiuGetTxBufFullness(&MaxBufFullness);
                   QP_ext=0;
                   QP_ext2=0;
                }

            }
            else //------------VGA,QVGA--------------//
            {
                if( RFIU_TX_MODE == rfiu_CheckTxTaskMode(RFI_UNIT_0))  
                {
                    TxBufFullness = rfiuGetTxBufFullness(&MaxBufFullness);
                    if(mpRateControlParam.enable_ratecontrol)
                    {
                       if(VideoCnt>2)
                       {
                            VideoCnt=0;
                        
                            if(TxBufFullness> (150/(DropFrame+1)) )
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                               mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *30/100);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                               mpeg4ModifyQp(10,P_VOP);
                            #endif
                               Drop_ext = 2;
                               QP_ext +=4;
                            }
                            else if(TxBufFullness> (100/(DropFrame+1)) )
                            {
                               if( Drop_ext >1 )
                               {
                               #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                  mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *30/100);
                               #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                  mpeg4ModifyQp(7,P_VOP);
                               #endif
                                  Drop_ext = 2;
                                  QP_ext +=3;
                               }
                               else
                               {
                               #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                  mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *40/100);
                               #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                  mpeg4ModifyQp(5,P_VOP);
                               #endif
                                  Drop_ext = 1;
                                  QP_ext +=3;
                               }
                            }
                            else if(TxBufFullness> (70/(DropFrame+1)) )
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                               mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *55/100);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                               mpeg4ModifyQp(3,P_VOP);
                            #endif
                               Drop_ext = 1;
                               QP_ext+=3;
                            }
                            else if(TxBufFullness> (50/(DropFrame+1)) )
                            {
                               if( (Drop_ext >0)  )
                               {
                               #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                 mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *60/100);
                               #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                 mpeg4ModifyQp(3,P_VOP);
                               #endif
                                 Drop_ext = 1;
                                 QP_ext +=2;
                               }
                               else
                               {
                               #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                                 mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *70/100);
                               #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                                 mpeg4ModifyQp(2,P_VOP);
                               #endif
                                 Drop_ext = 0;
                                 QP_ext +=2;
                               }
                            }
                            else if(TxBufFullness> (36/(DropFrame+1)) )
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                               mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *80/100);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                               mpeg4ModifyQp(2,P_VOP);
                            #endif
                               Drop_ext = 0;
                               QP_ext +=2;
                            }
                            else if(TxBufFullness>15)
                            {
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                               mpeg4ModifyTargetBitRate(mpRateControlParam.TargetBitrate *90/100);
                            #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                               mpeg4ModifyQp(1,P_VOP);
                            #endif
                               Drop_ext = 0;
                               QP_ext +=1;
                            }
                            else
                            {
                               Drop_ext = 0;
                               if(QP_ext>=2)
                                 QP_ext -=2;
                               else
                                 QP_ext=0;
                            }
                        #if RF_TX_SMOOTH_PLUS   
                            Drop_ext=0;
                            if(QP_ext>12)  
                                QP_ext=12;
                        #else    
                          #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                            if(QP_ext>8)  //max to 8
                                QP_ext=8;
                          #elif (VIDEO_CODEC_OPTION == H264_CODEC)
                            if(QP_ext>8)  
                                QP_ext=8;
                          #endif
                        #endif
                        }
                    }
                    else
                    {
                        QP_ext=0;
                        QP_ext2=0;
                    #if 0    
                        if(TxBufFullness>150)
                           Drop_ext = 20;
                        else if(TxBufFullness>120)
                           Drop_ext = 10;
                        else if(TxBufFullness>100)
                           Drop_ext = 7;
                        else if(TxBufFullness>70)
                           Drop_ext = 5;
                        else if(TxBufFullness>50)
                           Drop_ext = 3;
                        else if(TxBufFullness>30)
                           Drop_ext = 2;
                        else if(TxBufFullness>15)
                           Drop_ext = 1;
                    #else
                        if( VideoCnt > 1)
                        {    
                            VideoCnt=0;
                            if(TxBufFullness>TX_BUFFCNTL_THR_VGA_LOW+200)
                            {
                               Drop_ext = 44;
                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_VGA_LOW+170)
                            {
                               if( (Drop_ext >=44)  )
                               {
                                 
                               }
                               else
                               {
                                 Drop_ext = 36;
                               }            
                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_VGA_LOW+140)
                            {
                               if( (Drop_ext >=36)  )
                               {
                                 
                               }
                               else
                               {
                                 Drop_ext = 28;
                               }                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_VGA_LOW+120)
                            {
                               if( (Drop_ext >=28)  )
                               {
                                 
                               }
                               else
                               {
                                 Drop_ext = 20;
                               }
                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_VGA_LOW+100)
                            {
                               if(Drop_ext>=20)
                               {

                               }
                               else
                               {
                                  Drop_ext = 12;
                               }
                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_VGA_LOW+35)
                            {
                               if(Drop_ext>=12)
                               {

                               }
                               else
                               {
                                   Drop_ext = 6;
                               }
                            }
                            else if(TxBufFullness>TX_BUFFCNTL_THR_VGA_LOW)
                            {
                               if(Drop_ext >= 6)
                               {
                               }
                               else
                               {
                                  Drop_ext = 3;
                               }
                            }  
                            else
                            {
                               Drop_ext = 0;
                            }
                        }
                    #endif
                    }
                }
                else
                {
                   QP_ext=0;
                   QP_ext2=0;
                   TxBufFullness = rfiuGetTxBufFullness(&MaxBufFullness);
                }
            }
        }
                    
        //--------Video rate control: Calculate QP-----//            
        if(mpRateControlParam.enable_ratecontrol)
        {
             //DEBUG_MP4("--RCQ2_QuantAdjust---\n");
             if (Vop_Type == 0) //I frame
             {
                 dftMpeg4Quality    = mpRateControlParam.InitQP = RCQ2_QuantAdjust(&mpRateControlParam, 0, 0);
             }
             else //P frame
             {
                dftMpeg4Quality    = mpRateControlParam.InitQP = RCQ2_QuantAdjust(&mpRateControlParam,mpRateControlParam.Int_prev_PMad,1);
             #if(USE_PROGRESSIVE_SENSOR == 1) //Use progressive-scan sensor like CMOS
                video_double_field_flag = 0;

             #elif (VIDEO_CODEC_OPTION == H264_CODEC) 
                video_double_field_flag = 0;
             #else //Use interlace-scan sensor like CCD
                #if (DE_INTERLACE_SEL == DOUBLE_FIELD_ON)
                    video_double_field_flag = 1;
                #elif(DE_INTERLACE_SEL == DOUBLE_FIELD_OFF)
                    video_double_field_flag = 0;
                #else
                    if( SizeLevel == MPEG4_REC_QVGA )
                    {
                        video_double_field_flag = 0;
                    }
                    else
                    {
                        doublefield_thr=video_double_field_flag?  160:140;
                        if( (mpRateControlParam.Avg_PMad > doublefield_thr) || (dftMpeg4Quality>14) )
                        {
                           video_double_field_flag = 1;
                           double_field_cnt=0;
                        }
                        else
                        {
                          //Lucian: 一但double field 啟動, 則維持30 frames.
                          if(double_field_cnt<30)
                          {
                             double_field_cnt++;
                             video_double_field_flag = 1;
                          }
                          else
                             video_double_field_flag = 0;
                        }
                    }
                #endif
             #endif    
             }
             dftMpeg4Quality +=(QP_ext+QP_ext2);
             //dftMpeg4Quality=7;
         #if RF_TX_SMOOTH_PLUS 
             if( (dftMpeg4Quality>31) && (TxBufFullness>150) )
                Drop_ext=2;
         #endif

         
         #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
             if(dftMpeg4Quality>RC_MAX_QP)
                dftMpeg4Quality=RC_MAX_QP;
         #elif (VIDEO_CODEC_OPTION == H264_CODEC)
             if(dftMpeg4Quality>RC_MAX_QP)
                dftMpeg4Quality=RC_MAX_QP;
         #endif
         
        }
        //========================================//

        #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
           if(nMotorTime>0)
              dftMpeg4Quality +=8;
           if(dftMpeg4Quality>RC_MAX_QP)
                dftMpeg4Quality=RC_MAX_QP;
        #endif

     #if (MPEG4_FIELD_ENC_ENA && (CIU1_OPTION ==Sensor_CCIR656) )
   
     #elif (CIU1_BOB_REPLACE_MPEG_DF && ((CHIP_OPTION != CHIP_A1016A)) ) //Lucian: 不用 Mpeg4 double field,起動 ciu mob mode
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
        //------Video FIFO Management: 避免 Read/Write pointer overlay. ------//
        if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
            if((WantChangeFile == 1) && (GetLastVideo == 0) && (GetLastAudio)) {
                OS_ENTER_CRITICAL();
                LastVideo           = VideoBufMngWriteIdx;
                GetLastVideo        = 1;
                OS_EXIT_CRITICAL();

            #if CDVR_LOG
                ChangeLogFileStartAddress();
            #endif
            }
        }

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

                // GetLastVideo一直為0, 可能會造成dead lock, 所以要更新GetLastVideo
                if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
                    if((WantChangeFile == 1) && (GetLastVideo == 0) && (GetLastAudio)) {
                        OS_ENTER_CRITICAL();
                        LastVideo           = VideoBufMngWriteIdx;
                        GetLastVideo        = 1;
                        OS_EXIT_CRITICAL();

                    #if CDVR_LOG
                        ChangeLogFileStartAddress();
                    #endif
                    }
                }
            }
        }
        else
        {
           dcfMpegBufRemain=MPEG4_MAX_BUF_SIZE-(pBuf-pReadBuf);
        }
      #endif
        //---------- Video Encoding & Frame Rate control---------//
        *pTime=0;
      #if (FORECE_MPEG_DROP_1_10_FPS )
        if(TotalFrameCnt > 8)
        {
            if(DropFrameCnt <= 1)
            {
               Drop_ext2=1;
               //DEBUG_MP4("DP ");
            }
            else
            {
               Drop_ext2=0;
            }

            TotalFrameCnt=0;
            DropFrameCnt=0;
        }
        else
        {
          Drop_ext2=0;  
        }
      #else
        Drop_ext2=0;    
      #endif    
    
    #if 1 //Lucian: frame rate control
      #if MULTI_CHANNEL_SUPPORT
        #if (MULTI_CHANNEL_SEL & 0x01)
        for(i = 0; i < (DropFrame+Drop_ext+Drop_ext2); i++)
        {
            OSSemPend(isuSemEvt, 4 ,&err);
            *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
            VideoPictureIndex++;
            mp4_avifrmcnt++;
          #if (FORECE_MPEG_DROP_1_10_FPS  )
            TotalFrameCnt ++;
            DropFrameCnt ++;
          #endif    
            VideoCnt ++;
    
        }
        #elif (MULTI_CHANNEL_SEL & 0x02)
        for(i = 0; i < (DropFrame+Drop_ext+Drop_ext2); i++)
        {
            OSSemPend(ciuCapSemEvt_CH1, 4 ,&err);
            *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
            VideoPictureIndex++;
            mp4_avifrmcnt++;
          #if (FORECE_MPEG_DROP_1_10_FPS )
            TotalFrameCnt ++;
            DropFrameCnt ++;
          #endif    
            VideoCnt ++;
    
        }
        #elif (MULTI_CHANNEL_SEL & 0x04)
        for(i = 0; i < (DropFrame+Drop_ext+Drop_ext2); i++)
        {
            OSSemPend(ciuCapSemEvt_CH2, 4 ,&err);
            *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
            VideoPictureIndex++;
            mp4_avifrmcnt++;
          #if (FORECE_MPEG_DROP_1_10_FPS  )
            TotalFrameCnt ++;
            DropFrameCnt ++;
          #endif
            VideoCnt ++;
        }
        #elif (MULTI_CHANNEL_SEL & 0x08)
        for(i = 0; i < (DropFrame+Drop_ext+Drop_ext2); i++)
        {
            OSSemPend(ciuCapSemEvt_CH3, 4 ,&err);
            *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
            VideoPictureIndex++;
            mp4_avifrmcnt++;
          #if (FORECE_MPEG_DROP_1_10_FPS )
            TotalFrameCnt ++;
            DropFrameCnt ++;
          #endif  
            VideoCnt ++;
        }
        #elif (MULTI_CHANNEL_SEL & 0x10)
        for(i = 0; i < (DropFrame+Drop_ext+Drop_ext2); i++)
        {
            OSSemPend(ciuCapSemEvt_CH4, 4 ,&err);
            *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
            VideoPictureIndex++;
            mp4_avifrmcnt++;
          #if (FORECE_MPEG_DROP_1_10_FPS)
            TotalFrameCnt ++;
            DropFrameCnt ++;
          #endif 
            VideoCnt ++;
        }
        #endif
      #else   // MULTI_CHANNEL_SUPPORT == 0
        for(i = 0; i < (DropFrame+Drop_ext+Drop_ext2); i++)
        {
            OSSemPend(isuSemEvt, 4 ,&err);
            *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
            VideoPictureIndex++;
            mp4_avifrmcnt++;
          #if (FORECE_MPEG_DROP_1_10_FPS)
            TotalFrameCnt ++;
            DropFrameCnt ++;
          #endif   
            VideoCnt ++;
        }
      #endif  // #if MULTI_CHANNEL_SUPPORT
    #endif  //#if 1 //Lucian: frame rate control

      #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
        mpeg4Output1Frame(pBuf, pTime, pSize, &Mpeg4EncCnt, pOffset);
      #elif (VIDEO_CODEC_OPTION == H264_CODEC) 
        video_info.StreamBuf = pBuf;
        video_info.FrameType = *pFlag;
        video_info.FrameIdx  = VideoPictureIndex;  

        video_info.FrameTime = pTime;
      
        video_info.pSize     = pSize;  

        H264Enc_cfg.qp=dftMpeg4Quality;
        H264Enc_cfg.slice_qp_delta = H264Enc_cfg.qp - 26; 

        H264Enc_CompressOneFrame(&video_info,&H264Enc_cfg);
      #endif
      #if (FORECE_MPEG_DROP_1_10_FPS)
        TotalFrameCnt ++;
      #endif 
        VideoCnt ++;
    
        OS_ENTER_CRITICAL();
        VideoTimeStatistics += *pTime;
        
        if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && (VideoTimeStatistics >= (asfSectionTime * 1000)))            
        {
            SetIVOP = 1;             
        }            
        if((asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && (VideoTimeStatistics >= (asfRecTimeLen * 1000)))            
        {
            SetIVOP = 1;             
        }
        OS_EXIT_CRITICAL();

        //--------Rebuild RC model of Rate control ------//
        ByteCnt += *pSize;
        //DEBUG_MP4("%d ",(*pSize)>>10);
        #if RF_TX_RATECTRL_DEBUG_ENA
            //DEBUG_MP4("(%d,%d,%d) ",TxBufFullness,dftMpeg4Quality,(*pSize)>>10);
        #endif

        if(mpRateControlParam.enable_ratecontrol)
        {
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
            TotalSAD    = Mpeg4SadSum;
            outLen      = *pSize; //Video size. 
            if (Vop_Type !=I_VOP)
            {
                mpRateControlParam.Int_prev_PMad = (TotalSAD*4/mpeg4Height)*1024/mpeg4Width;  //FixRC

                if( (mpRateControlParam.PMad_cnt & 0x3) == 0 )  //Mod 4: 每四張統計一次MAD.
                {
                   mpRateControlParam.Avg_PMad = mpRateControlParam.Sum_PMad/4;
                   mpRateControlParam.Sum_PMad=0;
                   //DEBUG_MP4("(%d) ",mpRateControlParam.Avg_PMad);
                }
                mpRateControlParam.Sum_PMad += mpRateControlParam.Int_prev_PMad;
                mpRateControlParam.PMad_cnt ++;
            }
            //DEBUG_MP4("--RCQ2_Update2OrderModel---\n");   
            
            if(mpRateControlParam.enable_ratecontrol)
            {
                RCQ2_Update2OrderModel(&mpRateControlParam,
                                       outLen * 8,
                                       Vop_Type);
            }
        #endif

            if( (outLen >> 10) > (AvgFrameSize*2) )
            {
                 QP_ext2 +=3;
            }
            else if( (outLen >> 10) > (AvgFrameSize+AvgFrameSize/2) )
            {
                 QP_ext2 +=2;
            }
            else if((outLen >> 10) > AvgFrameSize)
            {
                 QP_ext2 +=1;
            }
            else
                 QP_ext2 -=1;

            if(QP_ext2 > 4)
                QP_ext2=4;
            else if(QP_ext2 < 0)
                QP_ext2=0; 
        
        }
        else
        {
           QP_ext2=0;
        }
        
     #if RF_TX_RATECTRL_DEBUG_ENA
            DEBUG_MP4("(%d,%2d,%2d,%2d,%2d,%2d)\n",DropFrame+Drop_ext+Drop_ext2,dftMpeg4Quality,TxBufFullness,*pSize>>10,QP_ext,QP_ext2 );
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
  #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    s32 rfiuTxRateControl_Local(int BitRate,int BufFullness,int VideoInFrameRate,int SizeLevel,unsigned int RealBitRate)
    {
        s32 DropFrame=0;
        static int Drop_Bitrate=0;


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

        mpRateControlParam.max_Qp=RC_MAX_QP;                  //max Qp
    	mpRateControlParam.min_Qp=RC_MIN_QP;                   //min Qp
     #if RF_TX_LOWBITRATE_5FPS
        BitRate=600;
     #endif
        //=============QVGA ===============//
        if( SizeLevel == MPEG4_REC_QVGA) //QVGA size
        {
            video_double_field_flag = 0;
            if(VideoInFrameRate>=15)  // 15~30 fps
            {
            #if USE_MPEG_QUANTIZATION
                if(BitRate>600) // 30 fps
                {
                   DropFrame=0;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1000)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>800)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                }
                else if(BitRate>300) // 15 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>500)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>400)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                }
                else if(BitRate>200)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   mpRateControlParam.InitQP=8;
                   mpRateControlParam.QP_I=8;
                   mpRateControlParam.QP_P=8;
                }
                else if(BitRate>100)
                {
                   DropFrame=5;       // 5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 9;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 10;
                }
            #else
                if(BitRate>600) // 30 fps
                {
                   DropFrame=0;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1000)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>800)
                   {
                      mpRateControlParam.InitQP=7;
                      mpRateControlParam.QP_I=7;
                      mpRateControlParam.QP_P=7;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=9;
                      mpRateControlParam.QP_I=9;
                      mpRateControlParam.QP_P=9;
                   }
                }
                else if(BitRate>300) // 15 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>500)
                   {
                      mpRateControlParam.InitQP=7;
                      mpRateControlParam.QP_I=7;
                      mpRateControlParam.QP_P=7;
                   }
                   else if(BitRate>400)
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=10;
                      mpRateControlParam.QP_I=10;
                      mpRateControlParam.QP_P=10;
                   }
                }
                else if(BitRate>200)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   mpRateControlParam.InitQP=10;
                   mpRateControlParam.QP_I=10;
                   mpRateControlParam.QP_P=10;
                }
                else if(BitRate>100)
                {
                   DropFrame=5;       // 5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 9;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 9;
                }
            #endif
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 

                if(RealBitRate > BitRate)
                {
                    Drop_Bitrate +=10;
                    if(Drop_Bitrate > BitRate)
                        Drop_Bitrate=BitRate;
                }
                else
                {
                    Drop_Bitrate-=10;
                    if(Drop_Bitrate < 0)
                        Drop_Bitrate=0;
                }

                if(mpRateControlParam.enable_ratecontrol == 0)
                    Drop_Bitrate=0;
                
                BitRate -= Drop_Bitrate;
                if(BitRate<25)
                    BitRate=25;
                
                mpRateControlParam.TargetBitrate    = ((BitRate-200)*10/13) * 1000;
            }
            else
            {   //not implement now//
                DEBUG_MP4("Warning!! QVGA,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
            } 
        }
        //============= VGA ===============//
        else if( SizeLevel == MPEG4_REC_VGA) //VGA size
        {
		#if(SW_APPLICATION_OPTION ==  MR8211_RFCAM_TX1)
			if(sys8211TXWifiStat==MR8211_ENTER_WIFI){
	        	if(BitRate > 1700){
	  	        	  mpRateControlParam.enable_ratecontrol   = 0;
                      dftMpeg4Quality = 12;
                      DropFrame=0;
                      printf("QP: %d, Drop %d\n", dftMpeg4Quality, DropFrame);
                }
                else if( BitRate >= 1100){
					   mpRateControlParam.enable_ratecontrol   = 0;
					   dftMpeg4Quality = 16;
					   DropFrame=0;
					   printf("QP: %d, Drop %d\n", dftMpeg4Quality, DropFrame);
                }
                else if( BitRate >= 600 ){
					   mpRateControlParam.enable_ratecontrol   = 0;
					   dftMpeg4Quality = 24;
					   DropFrame=1;
					   printf("QP: %d, Drop %d\n", dftMpeg4Quality, DropFrame);
                }
                else{
					   mpRateControlParam.enable_ratecontrol   = 0;
					   dftMpeg4Quality = 28;                
					   DropFrame=2;
					   printf("QP: %d, Drop %d\n", dftMpeg4Quality, DropFrame);
                }                
	        }
	        else
	        #endif             
            if(VideoInFrameRate>=24)  // 24~30 fps
            {
            #if USE_MPEG_QUANTIZATION
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                if(0)
                {

                }
               #else
                #if(SW_APPLICATION_OPTION  == MR8120_RFCAM_TX1_6M)
                if( BitRate>1500 ) // 30 fps
                #else
                if( BitRate>1700 ) // 30 fps
                #endif
                {
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
                   {
                       DropFrame=0;
                   }
                   else //Quad mode
                   {
                       if(rfiuRX_CamOnOff_Num>2)
                       {
                          DropFrame=1;
                       }
                       else
                          DropFrame=0;
                   }

                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>2500)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>2000)
                   {
                      mpRateControlParam.InitQP=7;
                      mpRateControlParam.QP_I=7;
                      mpRateControlParam.QP_P=7;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                   
                }
               #endif   
                else if( BitRate>1000  ) // 15 fps
                {
             #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                   DropFrame=1;
             #else   
                   DropFrame=1;
             #endif
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>1200)
                   {
                      mpRateControlParam.InitQP=7;
                      mpRateControlParam.QP_I=7;
                      mpRateControlParam.QP_P=7;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                }
                else if(BitRate>700)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   mpRateControlParam.InitQP=8;
                   mpRateControlParam.QP_I=8;
                   mpRateControlParam.QP_P=8;
                }
                else if(BitRate>500)
                {
                   
                   mpRateControlParam.enable_ratecontrol   = 0;
                #if RF_TX_LOWBITRATE_5FPS
                   DropFrame=3;       // 7 fps
                #else
                   DropFrame=3;       // 7 fps 
                #endif
                   dftMpeg4Quality = 8; 
                   video_double_field_flag = 1;
                }
                else if(BitRate>300)
                {
                   mpRateControlParam.enable_ratecontrol   = 0;
                   DropFrame=6;       // 4 fps 
                   dftMpeg4Quality = 10; 
                   video_double_field_flag = 1;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 10;
                   video_double_field_flag = 1;
                }
            #else
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                if(0)
                {

                }
               #else
                #if(SW_APPLICATION_OPTION  == MR8120_RFCAM_TX1_6M)
                if( BitRate>1500 ) // 30 fps
                #else
                if( BitRate>1700) // 30 fps
                #endif
                {    
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
                   {
                       DropFrame=0;
                   }
                   else //Quad mode
                   {
                       if(rfiuRX_CamOnOff_Num>2)
                       {
                          DropFrame=1;
                       }
                       else
                          DropFrame=0;
                   }
                      
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>2500)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>2000)
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=10;
                      mpRateControlParam.QP_I=10;
                      mpRateControlParam.QP_P=10;
                   }
                }
               #endif   
                else if( BitRate>1000  ) // 15 fps
                {
            #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                   DropFrame=1;
            #else
                   DropFrame=1;
            #endif
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                      mpRateControlParam.InitQP=7;
                      mpRateControlParam.QP_I=7;
                      mpRateControlParam.QP_P=7;
                   }
                   else if(BitRate>1200)
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=10;
                      mpRateControlParam.QP_I=10;
                      mpRateControlParam.QP_P=10;
                   }
                }
                else if(BitRate>700)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   mpRateControlParam.InitQP=10;
                   mpRateControlParam.QP_I=10;
                   mpRateControlParam.QP_P=10;
                }
                else if(BitRate>500)
                {
                   mpRateControlParam.enable_ratecontrol   = 0;
                #if RF_TX_LOWBITRATE_5FPS
                   DropFrame=3;       // 7 fps
                #else
                   DropFrame=5;       // 5 fps
                #endif
                   dftMpeg4Quality = 8;
                   video_double_field_flag = 1;
                }
                else if(BitRate>300)
                {
                   mpRateControlParam.enable_ratecontrol   = 0;
                   DropFrame=7;       // 5 fps
                   dftMpeg4Quality = 9;
                   video_double_field_flag = 1;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 9;
                   video_double_field_flag = 1;
                }
            #endif        
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 

                if(RealBitRate > BitRate)
                {
                    Drop_Bitrate +=10;
                    if(Drop_Bitrate > BitRate)
                        Drop_Bitrate=BitRate;
                }
                else
                {
                    Drop_Bitrate-=10;
                    if(Drop_Bitrate < 0)
                        Drop_Bitrate=0;
                }

                if(mpRateControlParam.enable_ratecontrol == 0)
                    Drop_Bitrate=0;
                
                BitRate -= Drop_Bitrate;
                if(BitRate<100)
                    BitRate=100;
                
                mpRateControlParam.TargetBitrate    = ((BitRate-200)*10/12) * 1000;
            }
            else
            {   //not implement now//
                DEBUG_MP4("Warning!! VGA,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
            }
            
        }
        //============= HD ===============//
        else if( SizeLevel == MPEG4_REC_HD) //HD size
        {
            video_double_field_flag = 0;
		#if(SW_APPLICATION_OPTION ==  MR8211_RFCAM_TX1)
			if(sys8211TXWifiStat==MR8211_ENTER_WIFI){
	        	if(BitRate >= 1100){
	  	        	  mpRateControlParam.enable_ratecontrol   = 0;
                      dftMpeg4Quality = 18;
                      DropFrame=1;
                      printf("QP: %d, Drop %d\n", dftMpeg4Quality, DropFrame);
                }
                else if( BitRate >= 800){
					   mpRateControlParam.enable_ratecontrol   = 0;
					   dftMpeg4Quality = 24;
					   DropFrame=1;
                       printf("QP: %d, Drop %d\n", dftMpeg4Quality, DropFrame);
                }
                else if( BitRate >= 500 ){
					   mpRateControlParam.enable_ratecontrol   = 0;
					   dftMpeg4Quality = 28;
					   DropFrame=2;
                       printf("QP: %d, Drop %d\n", dftMpeg4Quality, DropFrame);				   
                }
                else{
					   mpRateControlParam.enable_ratecontrol   = 0;
					   dftMpeg4Quality = 31;                
					   DropFrame=3;
					   printf("QP: %d, Drop %d\n", dftMpeg4Quality, DropFrame);
                }                
	        }
	        else
        #endif            
            if(VideoInFrameRate>=24)  // 24~30 fps
            {   //not implement now//
                DEBUG_MP4("Warning!! HD,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
            }
            else
            {
            #if 0
                if( BitRate>1700 ) // 15 fps
                {
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
                   {
                   #if(SW_APPLICATION_OPTION ==  MR8211_RFCAM_TX1)
                       DropFrame=0; // 10 fps
                   #else
                       DropFrame=0;
                   #endif
                   }
                   else //Quad mode
                   {
                       if(rfiuRX_CamOnOff_Num>2)
                       {
                          DropFrame=2;
                       }
                       else if(rfiuRX_CamOnOff_Num==2)
                          DropFrame=1;
                       else
                          DropFrame=0;
                   }
                   
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>2500)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>2000)
                   {
                      mpRateControlParam.InitQP=7;
                      mpRateControlParam.QP_I=7;
                      mpRateControlParam.QP_P=7;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                }
                else if( (BitRate>1000) && ( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 ) && ( (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) ==0 )  ) // 7.5 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>1200)
                   {
                      mpRateControlParam.InitQP=7;
                      mpRateControlParam.QP_I=7;
                      mpRateControlParam.QP_P=7;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                }
                else if(BitRate>700)  // 5 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 6;
                }
                else if(BitRate>500)
                {
                   DropFrame=5;       // 2.5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 7;
                }
                else
                {
                   DropFrame=9;       // 1.5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 8;
                }
            #else
                if( BitRate>1700 ) // 15 fps
                {
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
                   {
                   #if(SW_APPLICATION_OPTION ==  MR8211_RFCAM_TX1)
                       if(sys8211TXWifiStat==MR8211_ENTER_WIFI)
                       {
                            if(sys8211TXWifiUserNum >= 2)
                            {
                                DropFrame=3;  //5 fps
                            }
                            else
                            {
                                DropFrame=1;  //10 fps
                            }
                       }
                       else
                       {
                           DropFrame=1;  //10 fps
                       }
                   #else
                       DropFrame=0;
                   #endif                   
                   }
                   else //Quad mode
                   {
                       if(rfiuRX_CamOnOff_Num>2)
                       {
                          DropFrame=2;
                       }
                       else if(rfiuRX_CamOnOff_Num==2)
                          DropFrame=1;
                       else
                          DropFrame=0;
                   }
                   
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>2500)
                   {
                   #if(SW_APPLICATION_OPTION ==  MR8211_RFCAM_TX1)
                       if(sys8211TXWifiStat==MR8211_ENTER_WIFI)
                       {
                           mpRateControlParam.InitQP=9;
                           mpRateControlParam.QP_I=9;
                           mpRateControlParam.QP_P=9;
                       }
                       else
                       {
                           mpRateControlParam.InitQP=7;
                           mpRateControlParam.QP_I=7;
                           mpRateControlParam.QP_P=7;
                       }
                   #else    
                      mpRateControlParam.InitQP=7;
                      mpRateControlParam.QP_I=7;
                      mpRateControlParam.QP_P=7;
                   #endif
                   }
                   else if(BitRate>2000)
                   {
                   #if(SW_APPLICATION_OPTION ==  MR8211_RFCAM_TX1)
                       if(sys8211TXWifiStat==MR8211_ENTER_WIFI)
                       {
                           mpRateControlParam.InitQP=10;
                           mpRateControlParam.QP_I=10;
                           mpRateControlParam.QP_P=10;
                       }
                       else
                       {
                           mpRateControlParam.InitQP=8;
                           mpRateControlParam.QP_I=8;
                           mpRateControlParam.QP_P=8;
                       }
                   #else
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   #endif
                   }
                   else
                   {
                      mpRateControlParam.InitQP=10;
                      mpRateControlParam.QP_I=10;
                      mpRateControlParam.QP_P=10;
                   }
                }
                else if( (BitRate>1000) && ( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 ) && ( (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) ==0 )  ) // 7.5 fps
                {
                #if(SW_APPLICATION_OPTION ==  MR8211_RFCAM_TX1)
                   DropFrame=2;
                #else
                   DropFrame=1;
                #endif
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                   #if(SW_APPLICATION_OPTION ==  MR8211_RFCAM_TX1)
                       if(sys8211TXWifiStat==MR8211_ENTER_WIFI)
                       {
                          mpRateControlParam.InitQP=10;
                          mpRateControlParam.QP_I=10;
                          mpRateControlParam.QP_P=10;
                       }
                       else
                       {
                          mpRateControlParam.InitQP=9;
                          mpRateControlParam.QP_I=9;
                          mpRateControlParam.QP_P=9;
                       }
                   #else    
                       mpRateControlParam.InitQP=9;
                       mpRateControlParam.QP_I=9;
                       mpRateControlParam.QP_P=9;
                   #endif
                   }
                   else if(BitRate>1200)
                   {
                   #if(SW_APPLICATION_OPTION ==  MR8211_RFCAM_TX1)
                       if(sys8211TXWifiStat==MR8211_ENTER_WIFI)
                       {
                           mpRateControlParam.InitQP=11;
                           mpRateControlParam.QP_I=11;
                           mpRateControlParam.QP_P=11;
                       }
                       else
                       {
                           mpRateControlParam.InitQP=10;
                           mpRateControlParam.QP_I=10;
                           mpRateControlParam.QP_P=10;
                       }
                   #else    
                       mpRateControlParam.InitQP=10;
                       mpRateControlParam.QP_I=10;
                       mpRateControlParam.QP_P=10;
                   #endif   
                   }
                   else
                   {
                      mpRateControlParam.InitQP=12;
                      mpRateControlParam.QP_I=12;
                      mpRateControlParam.QP_P=12;
                   }
                }
                else if(BitRate>700)  // 5 fps
                {
                #if(SW_APPLICATION_OPTION ==  MR8211_RFCAM_TX1)
                   DropFrame=3;
                #else
                   DropFrame=2;
                #endif
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 10;
                }
                else if(BitRate>500)
                {
                   DropFrame=5;       // 2.5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 11;
                }
                else
                {
                   DropFrame=9;       // 1.5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 12;
                }
            #endif        
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 

                if(RealBitRate > BitRate)
                {
                    Drop_Bitrate +=10;
                    if(Drop_Bitrate > BitRate)
                        Drop_Bitrate=BitRate;
                }
                else
                {
                    Drop_Bitrate-=10;
                    if(Drop_Bitrate < 0)
                        Drop_Bitrate=0;
                }

                if(mpRateControlParam.enable_ratecontrol == 0)
                    Drop_Bitrate=0;
                
                BitRate -= Drop_Bitrate;
                if(BitRate<300)
                    BitRate=300;
                
                mpRateControlParam.TargetBitrate    = ((BitRate-200)*10/12) * 1000;
            }
                    
        }

        //DEBUG_MP4("Drop_Bitrate=%d\n",Drop_Bitrate);

        //Lsk 090626
#if(USE_PROGRESSIVE_SENSOR == 1) //Use progressive-scan sensor like CMOS
        video_double_field_flag = 0;
#endif  

        TVout_Generate_Pause_Frame = 0;
        ASF_set_interlace_flag = 0;
        
        if(mpRateControlParam.enable_ratecontrol)
    	   RCQ2_init(&mpRateControlParam); //Lsk 090608

        return DropFrame;
    }


    s32 rfiuTxRateControl_P2P(int BitRate,
                                    int BufFullness,
                                    int VideoInFrameRate,
                                    int SizeLevel,
                                    int P2PVideoQuality,
                                    unsigned int RealBitRate)
    {
        s32 DropFrame=0;
        int TargetBitrate;
        static int Drop_Bitrate=0;
        //------------------------------//

        /* Lucian:
            Target Bitrate  (kbps)@30 fps; Lucian: 目前實際值 = 理論值*1.3

             理論值        |       實際值
             1.00M bps            1.33M bps
             1.50M bps            1.85M bps
             1.70M bps            2.21M bps
             2.00M bps            2.99M bps

             30/15 fps: 用rate control 控制QP.
             5     fps: 因時間上不連續. 則直接控制QP.


             1.720P  5 fps
             2.VGA  15 fps (1200 Kbps)
             3.QVGA 15 fps (400  Kbps)
             4.QVGA 10 fps (250  Kbps)
             5.QVGA 7.5 fps(100  Kbps)

            
        */

        mpRateControlParam.max_Qp=RC_MAX_QP;                  //max Qp
    	mpRateControlParam.min_Qp=RC_MIN_QP;                   //min Qp

        //=============QVGA ===============//
        if( SizeLevel == MPEG4_REC_QVGA) //QVGA size
        {
            video_double_field_flag = 0;
            if(VideoInFrameRate>=24)  // 24~30 fps
            {
            #if USE_MPEG_QUANTIZATION
                if(BitRate>500) // 30 fps
                {
                   DropFrame=0;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1000)
                   {
                      mpRateControlParam.InitQP=4;
                      mpRateControlParam.QP_I=4;
                      mpRateControlParam.QP_P=4;
                   }
                   else if(BitRate>800)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                }
                else if(BitRate>300) // 15 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>500)
                   {
                      mpRateControlParam.InitQP=4;
                      mpRateControlParam.QP_I=4;
                      mpRateControlParam.QP_P=4;
                   }
                   else if(BitRate>400)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                }
                else if(BitRate>200)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   dftMpeg4Quality = 5;
                }
                else if(BitRate>100)
                {
                   DropFrame=2;       // 10 fps
                   mpRateControlParam.enable_ratecontrol   = 1;
                   dftMpeg4Quality = 6;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 7;
                }
            #else
                if(BitRate>500) // 30 fps
                {
                   DropFrame=0;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1000)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>800)
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=10;
                      mpRateControlParam.QP_I=10;
                      mpRateControlParam.QP_P=10;
                   }
                }
                else if(BitRate>300) // 15 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>500)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>400)
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=10;
                      mpRateControlParam.QP_I=10;
                      mpRateControlParam.QP_P=10;
                   }
                }
                else if(BitRate>200)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   dftMpeg4Quality = 7;
                }
                else if(BitRate>100)
                {
                   DropFrame=2;       // 10 fps
                   mpRateControlParam.enable_ratecontrol   = 1;
                   dftMpeg4Quality = 8;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 9;
                }
            #endif
                //----------------------------------------------------------//
                if(P2PVideoQuality==RF_P2PVdoQalt_QVGA_15_FPS)
                {
                    TargetBitrate=400;
                    if(DropFrame<=1)
                       DropFrame=1;   //fixed to under 15 fps
                }
                else if(P2PVideoQuality==RF_P2PVdoQalt_QVGA_10_FPS)  
                {
                    TargetBitrate=250;
                    if(DropFrame<=2)
                       DropFrame=2;   //fixed to under 10 fps


                    if(BitRate>400)  // 10 fps
                    {
                       dftMpeg4Quality = 8;

                    }
                    else if(BitRate>200)
                    {
                       dftMpeg4Quality = 10;
                       DropFrame=9;
                    }
                    else
                    {
                       dftMpeg4Quality = 12;
                       DropFrame=14;
                    }
                    
                }
                else if(P2PVideoQuality==RF_P2PVdoQalt_QVGA_7_FPS)
                {
                    TargetBitrate=150;
                    if(DropFrame<=3)
                       DropFrame=3;   //fixed to under 7.5 fps


                    if(BitRate>200)  
                    {
                       dftMpeg4Quality = 14;
                    }
                    else if(BitRate>100)
                    {
                       dftMpeg4Quality = 16;
                       DropFrame=9;
                    }
                    else
                    {
                       dftMpeg4Quality = 18;
                       DropFrame=14;
                    }
                }
                else
                {
                   TargetBitrate=250;
                   if(DropFrame<=2)
                       DropFrame=2;   //fixed to under 10 fps
                }

                if(RealBitRate>TargetBitrate)
                {
                    Drop_Bitrate +=10;
                    if(Drop_Bitrate > TargetBitrate)
                        Drop_Bitrate=TargetBitrate;
                }
                else
                {
                    Drop_Bitrate-=10;
                    if(Drop_Bitrate < 0)
                        Drop_Bitrate=0;
                }
                TargetBitrate -= Drop_Bitrate;
                if(TargetBitrate<25)
                    TargetBitrate=25;
                
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
                mpRateControlParam.TargetBitrate    = (TargetBitrate*10/13) * 1000;  

                //DEBUG_MP4("QVGA Target bitrate=%d\n",TargetBitrate);
                    
            }
            else
            {   //not implement now//
                DEBUG_MP4("Warning!! QVGA,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
            } 
        }
        //============= VGA ===============//
        else if( SizeLevel == MPEG4_REC_VGA) //VGA size
        {
            if(VideoInFrameRate>=24)  // 24~30 fps
            {
            #if USE_MPEG_QUANTIZATION
                if( (P2PVideoQuality==RF_P2PVdoQalt_VGA_30_FPS) && (BitRate>1700) )
                {
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
                   {
                       DropFrame=0;
                   }
                   else //Quad mode
                   {
                       if(rfiuRX_CamOnOff_Num>1)
                       {
                          DropFrame=1;
                       }
                       else
                          DropFrame=0;
                   }
                   
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                      mpRateControlParam.InitQP=4;
                      mpRateControlParam.QP_I=4;
                      mpRateControlParam.QP_P=4;
                   }
                   else if(BitRate>1200)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                }
                else if( BitRate>700  ) // 15 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                      mpRateControlParam.InitQP=4;
                      mpRateControlParam.QP_I=4;
                      mpRateControlParam.QP_P=4;
                   }
                   else if(BitRate>1200)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                }
                else if(BitRate>400)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   dftMpeg4Quality = 5;
                   video_double_field_flag = 1;
                }
                else if(BitRate>300)
                {
                   DropFrame=5;       // 5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 6;
                   video_double_field_flag = 1;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 7;
                   video_double_field_flag = 1;
                }
            #else
                if( (P2PVideoQuality==RF_P2PVdoQalt_VGA_30_FPS) && (BitRate>1700) )
                {
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
                   {
                       DropFrame=0;
                   }
                   else //Quad mode
                   {
                       if(rfiuRX_CamOnOff_Num>2)
                       {
                          DropFrame=1;
                       }
                       else
                          DropFrame=0;
                   }
                   
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>1200)
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=10;
                      mpRateControlParam.QP_I=10;
                      mpRateControlParam.QP_P=10;
                   }
                }
                else  if( BitRate>700  ) // 15 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                      mpRateControlParam.InitQP=6;
                      mpRateControlParam.QP_I=6;
                      mpRateControlParam.QP_P=6;
                   }
                   else if(BitRate>1200)
                   {
                      mpRateControlParam.InitQP=8;
                      mpRateControlParam.QP_I=8;
                      mpRateControlParam.QP_P=8;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=10;
                      mpRateControlParam.QP_I=10;
                      mpRateControlParam.QP_P=10;
                   }
                }
                else if(BitRate>400)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   dftMpeg4Quality = 7;
                   video_double_field_flag = 1;
                }
                else if(BitRate>300)
                {
                   DropFrame=5;       // 5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 8;
                   video_double_field_flag = 1;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 9;
                   video_double_field_flag = 1;
                }
            #endif        
                //----------------------------------------------------------//
                if(P2PVideoQuality==RF_P2PVdoQalt_VGA_15_FPS)
                {
                    TargetBitrate=1000;
                    if(DropFrame<=1)
                       DropFrame=1;   //fixed to under 15 fps
                }
            #if 1
                else if(P2PVideoQuality==RF_P2PVdoQalt_VGA_30_FPS)
                {
                    TargetBitrate=2000;
                    DropFrame=0;   //fixed to under 30 fps
                }
            #endif
                else if(P2PVideoQuality==RF_P2PVdoQalt_VGA_10_FPS)  
                {
                    TargetBitrate=400;
                    if(DropFrame<=2)
                       DropFrame=2;   //fixed to under 10 fps

                    if(BitRate>400)  // 10 fps
                    {
                       dftMpeg4Quality = 8;

                    }
                    else if(BitRate>200)
                    {
                       dftMpeg4Quality = 10;
                       DropFrame=9;
                    }
                    else
                    {
                       dftMpeg4Quality = 12;
                       DropFrame=14;
                    }   

                }
                else
                { 
                   TargetBitrate=1000;
                   TargetBitrate=1000;
                   if(DropFrame<=1)
                       DropFrame=1;   //fixed to under 15 fps
                }

                if(RealBitRate>TargetBitrate)
                {
                    Drop_Bitrate +=10;
                    if(Drop_Bitrate > TargetBitrate)
                        Drop_Bitrate=TargetBitrate;
                }
                else
                {
                    Drop_Bitrate -=10;
                    if(Drop_Bitrate < 0)
                        Drop_Bitrate=0;
                }
                TargetBitrate -= Drop_Bitrate;
                if(TargetBitrate<100)
                    TargetBitrate=100;
                
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
                mpRateControlParam.TargetBitrate    = (TargetBitrate*10/12) * 1000; //Lucian: target to 1.2Mbps
            }
            else
            {   //not implement now//
                DEBUG_MP4("Warning!! VGA,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
            }
            
        }
        //============= HD ===============//
        else if( SizeLevel == MPEG4_REC_HD) //HD size
        {
            video_double_field_flag = 0;
            if(VideoInFrameRate>=24)  // 24~30 fps
            {   //not implement now//
                DEBUG_MP4("Warning!! HD,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
            }
            else
            {
            #if USE_MPEG_QUANTIZATION
                if( (P2PVideoQuality==RF_P2PVdoQalt_HD_7_FPS) && (BitRate>1700) )
                { 
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
                   {
                       DropFrame=1;
                   }
                   else //Quad mode
                   {
                       if(rfiuRX_CamOnOff_Num>2)
                       {
                          DropFrame=2;
                       }
                       else
                          DropFrame=1;
                   }
                   
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 6;
                }
                else if(BitRate>1200)  // 5 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 6;
                }
                else if(BitRate>1000)  // 5 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 7;
                }
                else if(BitRate>800)  // 4 fps
                {
                   DropFrame=3;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 8;
                }
                else if(BitRate>500)
                {
                   DropFrame=5;       // 2.5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 9;
                }
                else
                {
                   DropFrame=9;       // 1.5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 10;
                }
            #else
                if( (P2PVideoQuality==RF_P2PVdoQalt_HD_7_FPS) && (BitRate>1700) )
                { 
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
                   {
                       DropFrame=1;
                   }
                   else //Quad mode
                   {
                       if(rfiuRX_CamOnOff_Num>2)
                       {
                          DropFrame=2;
                       }
                       else
                          DropFrame=1;
                   }
                   
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 6;
                }
                else if(BitRate>1200)  // 5 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 8;
                }
                else if(BitRate>1000)  // 5 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 9;
                }
                else if(BitRate>800)  // 4 fps
                {
                   DropFrame=3;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 10;
                }
                else if(BitRate>500)
                {
                   DropFrame=5;       // 2.5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 11;
                }
                else
                {
                   DropFrame=9;       // 1.5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 12;
                }
            #endif 

                if(P2PVideoQuality==RF_P2PVdoQalt_HD_7_FPS)
                {
                    TargetBitrate=1700;
                    if(DropFrame<=1)
                       DropFrame=1;   //fixed to under 5 fps
                }
                else
                {
                    TargetBitrate=1200;
                    if(DropFrame<=2)
                       DropFrame=2;   //fixed to under 5 fps
                }
            
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
                mpRateControlParam.TargetBitrate    = ((TargetBitrate)*10/12) * 1000;
            }
                    
        }

        //Lsk 090626
#if(USE_PROGRESSIVE_SENSOR == 1) //Use progressive-scan sensor like CMOS
        video_double_field_flag = 0;
#endif  

        TVout_Generate_Pause_Frame = 0;
        ASF_set_interlace_flag = 0;
        
        if(mpRateControlParam.enable_ratecontrol)
    	   RCQ2_init(&mpRateControlParam); //Lsk 090608

        return DropFrame;
    }
   #elif (VIDEO_CODEC_OPTION == H264_CODEC)
    s32 rfiuTxRateControl_Local(int BitRate,int BufFullness,int VideoInFrameRate,int SizeLevel,unsigned int RealBitRate)
    {
        s32 DropFrame=0;
        static int Drop_Bitrate=0;

        mpRateControlParam.max_Qp=RC_MAX_QP;                  //max Qp
    	mpRateControlParam.min_Qp=RC_MIN_QP;                   //min Qp

        //=============QVGA ===============//
        if( SizeLevel == MPEG4_REC_QVGA) //QVGA size
        {
            video_double_field_flag=0;
            if(VideoInFrameRate>=24)  // 24~30 fps
            {
                if(BitRate>600) // 30 fps
                {
                   DropFrame=0;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1000)
                   {
                      mpRateControlParam.InitQP=30;
                      mpRateControlParam.QP_I=30;
                      mpRateControlParam.QP_P=30;
                   }
                   else if(BitRate>800)
                   {
                      mpRateControlParam.InitQP=35;
                      mpRateControlParam.QP_I=35;
                      mpRateControlParam.QP_P=35;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=40;
                      mpRateControlParam.QP_I=40;
                      mpRateControlParam.QP_P=40;
                   }
                }
                else if(BitRate>300) // 15 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>500)
                   {
                      mpRateControlParam.InitQP=35;
                      mpRateControlParam.QP_I=35;
                      mpRateControlParam.QP_P=35;
                   }
                   else if(BitRate>400)
                   {
                      mpRateControlParam.InitQP=40;
                      mpRateControlParam.QP_I=40;
                      mpRateControlParam.QP_P=40;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=45;
                      mpRateControlParam.QP_I=45;
                      mpRateControlParam.QP_P=45;
                   }
                }
                else if(BitRate>200)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 35;
                }
                else if(BitRate>100)
                {
                   DropFrame=5;       // 5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 40;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 45;
                }
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
            }
            else
            {   //not implement now//
                DEBUG_MP4("Warning!! QVGA,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
            } 
        }
        //============= VGA ===============//
        else if( SizeLevel == MPEG4_REC_VGA) //VGA size
        {
            if(VideoInFrameRate>=24)  // 24~30 fps
            {
               #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
                if(0)
                {

                }
               #else
                if( BitRate>1700) // 30 fps
                {    
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
                   {
                       DropFrame=0;
                   }
                   else //Quad mode
                   {
                       if(rfiuRX_CamOnOff_Num>2)
                       {
                          DropFrame=1;
                       }
                       else
                          DropFrame=0;
                   }
                      
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>2500)
                   {
                      mpRateControlParam.InitQP=30;
                      mpRateControlParam.QP_I=30;
                      mpRateControlParam.QP_P=30;
                   }
                   else if(BitRate>2000)
                   {
                      mpRateControlParam.InitQP=35;
                      mpRateControlParam.QP_I=35;
                      mpRateControlParam.QP_P=35;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=40;
                      mpRateControlParam.QP_I=40;
                      mpRateControlParam.QP_P=40;
                   }
                }
               #endif   
                else if( BitRate>1000  ) // 15 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                      mpRateControlParam.InitQP=35;
                      mpRateControlParam.QP_I=35;
                      mpRateControlParam.QP_P=35;
                   }
                   else if(BitRate>1200)
                   {
                      mpRateControlParam.InitQP=40;
                      mpRateControlParam.QP_I=40;
                      mpRateControlParam.QP_P=40;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=45;
                      mpRateControlParam.QP_I=45;
                      mpRateControlParam.QP_P=45;
                   }
                }
                else if(BitRate>700)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 35;
                   video_double_field_flag = 1;
                }
                else if(BitRate>500)
                {
                   DropFrame=5;       // 5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 40;
                   video_double_field_flag = 1;
                }
                else if(BitRate>300)
                {
                   DropFrame=7;       // 5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 40;
                   video_double_field_flag = 1;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 45;
                   video_double_field_flag = 1;
                }
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
            }
            else
            {   //not implement now//
                DEBUG_MP4("Warning!! VGA,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
            }
            
        }
        //============= HD ===============//
        else if( SizeLevel == MPEG4_REC_HD) //HD size
        {
            video_double_field_flag = 0;
                    
            if( BitRate>1700 ) // 15 fps
            {
               if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
               {
                   DropFrame=0;
               }
               else //Quad mode
               {
                   if(rfiuRX_CamOnOff_Num>2)
                   {
                      DropFrame=2;
                   }
                   else if(rfiuRX_CamOnOff_Num==2)
                      DropFrame=1;
                   else
                      DropFrame=0;
               }
               
               mpRateControlParam.enable_ratecontrol   = 1;
               if(BitRate>2500)
               {
                  mpRateControlParam.InitQP=30;
                  mpRateControlParam.QP_I=30;
                  mpRateControlParam.QP_P=30;
               }
               else if(BitRate>2000)
               {
                  mpRateControlParam.InitQP=35;
                  mpRateControlParam.QP_I=35;
                  mpRateControlParam.QP_P=35;
               }
               else
               {
                  mpRateControlParam.InitQP=40;
                  mpRateControlParam.QP_I=40;
                  mpRateControlParam.QP_P=40;
               }
            }
            else if( (BitRate>1000) && ( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 ) && ( (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) ==0 )  ) // 7.5 fps
            {
               DropFrame=1;
               mpRateControlParam.enable_ratecontrol   = 1;
               if(BitRate>1400)
               {
                  mpRateControlParam.InitQP=35;
                  mpRateControlParam.QP_I=35;
                  mpRateControlParam.QP_P=35;
               }
               else if(BitRate>1200)
               {
                  mpRateControlParam.InitQP=40;
                  mpRateControlParam.QP_I=40;
                  mpRateControlParam.QP_P=40;
               }
               else
               {
                  mpRateControlParam.InitQP=45;
                  mpRateControlParam.QP_I=45;
                  mpRateControlParam.QP_P=45;
               }
            }
            else if(BitRate>700)  // 5 fps
            {
               DropFrame=2;
               mpRateControlParam.enable_ratecontrol   = 0;
               dftMpeg4Quality = 35;
            }
            else if(BitRate>500)
            {
               DropFrame=5;       // 2.5 fps
               mpRateControlParam.enable_ratecontrol   = 0;
               dftMpeg4Quality = 40;
            }
            else
            {
               DropFrame=9;       // 1.5 fps
               mpRateControlParam.enable_ratecontrol   = 0;
               dftMpeg4Quality = 45;
            }
            mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
        }
                    

        TVout_Generate_Pause_Frame = 0;
        ASF_set_interlace_flag = 0;
        
        return DropFrame;
    }


    s32 rfiuTxRateControl_P2P(int BitRate,
                                    int BufFullness,
                                    int VideoInFrameRate,
                                    int SizeLevel,
                                    int P2PVideoQuality,
                                    unsigned int RealBitRate)
    {
        s32 DropFrame=0;
        int TargetBitrate;
        static int Drop_Bitrate=0;
        //------------------------------//
        mpRateControlParam.max_Qp=RC_MAX_QP;                  //max Qp
    	mpRateControlParam.min_Qp=RC_MIN_QP;                   //min Qp

        //=============QVGA ===============//
        if( SizeLevel == MPEG4_REC_QVGA) //QVGA size
        {
            video_double_field_flag = 0;
            if(VideoInFrameRate>=24)  // 24~30 fps
            {
            
                if(BitRate>500) // 30 fps
                {
                   DropFrame=0;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1000)
                   {
                      mpRateControlParam.InitQP=30;
                      mpRateControlParam.QP_I=30;
                      mpRateControlParam.QP_P=30;
                   }
                   else if(BitRate>800)
                   {
                      mpRateControlParam.InitQP=35;
                      mpRateControlParam.QP_I=35;
                      mpRateControlParam.QP_P=35;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=40;
                      mpRateControlParam.QP_I=40;
                      mpRateControlParam.QP_P=40;
                   }
                }
                else if(BitRate>300) // 15 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>500)
                   {
                      mpRateControlParam.InitQP=35;
                      mpRateControlParam.QP_I=35;
                      mpRateControlParam.QP_P=35;
                   }
                   else if(BitRate>400)
                   {
                      mpRateControlParam.InitQP=40;
                      mpRateControlParam.QP_I=40;
                      mpRateControlParam.QP_P=40;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=45;
                      mpRateControlParam.QP_I=45;
                      mpRateControlParam.QP_P=45;
                   }
                }
                else if(BitRate>200)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   dftMpeg4Quality = 35;
                }
                else if(BitRate>100)
                {
                   DropFrame=2;       // 10 fps
                   mpRateControlParam.enable_ratecontrol   = 1;
                   dftMpeg4Quality = 40;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 45;
                }
                //----------------------------------------------------------//
                if(P2PVideoQuality==RF_P2PVdoQalt_QVGA_15_FPS)
                {
                    TargetBitrate=400;
                    if(DropFrame<=1)
                       DropFrame=1;   //fixed to under 15 fps
                }
                else if(P2PVideoQuality==RF_P2PVdoQalt_QVGA_10_FPS)  
                {
                    TargetBitrate=250;
                    if(DropFrame<=2)
                       DropFrame=2;   //fixed to under 10 fps


                    if(BitRate>400)  // 10 fps
                    {
                       dftMpeg4Quality = 35;

                    }
                    else if(BitRate>200)
                    {
                       dftMpeg4Quality = 40;
                       DropFrame=9;
                    }
                    else
                    {
                       dftMpeg4Quality = 45;
                       DropFrame=14;
                    }
                    
                }
                else if(P2PVideoQuality==RF_P2PVdoQalt_QVGA_7_FPS)
                {
                    TargetBitrate=150;
                    if(DropFrame<=3)
                       DropFrame=3;   //fixed to under 7.5 fps


                    if(BitRate>200)  
                    {
                       dftMpeg4Quality = 40;
                    }
                    else if(BitRate>100)
                    {
                       dftMpeg4Quality = 45;
                       DropFrame=9;
                    }
                    else
                    {
                       dftMpeg4Quality = 45;
                       DropFrame=14;
                    }
                }
                else
                {
                   TargetBitrate=250;
                   if(DropFrame<=2)
                       DropFrame=2;   //fixed to under 10 fps
                }
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
            }
            else
            {   //not implement now//
                DEBUG_MP4("Warning!! QVGA,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
            } 
        }
        //============= VGA ===============//
        else if( SizeLevel == MPEG4_REC_VGA) //VGA size
        {
            if(VideoInFrameRate>=24)  // 24~30 fps
            {
                if( (P2PVideoQuality==RF_P2PVdoQalt_VGA_30_FPS) && (BitRate>1700) )
                {
                   if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) == 0 )  //Single mode
                   {
                       DropFrame=0;
                   }
                   else //Quad mode
                   {
                       if(rfiuRX_CamOnOff_Num>3)
                       {
                          DropFrame=1;
                       }
                       else
                          DropFrame=0;
                   }
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                      mpRateControlParam.InitQP=35;
                      mpRateControlParam.QP_I=35;
                      mpRateControlParam.QP_P=35;
                   }
                   else if(BitRate>1200)
                   {
                      mpRateControlParam.InitQP=40;
                      mpRateControlParam.QP_I=40;
                      mpRateControlParam.QP_P=40;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=45;
                      mpRateControlParam.QP_I=45;
                      mpRateControlParam.QP_P=45;
                   }
                }
                else if( BitRate>700  ) // 15 fps
                {
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 1;
                   if(BitRate>1400)
                   {
                      mpRateControlParam.InitQP=35;
                      mpRateControlParam.QP_I=35;
                      mpRateControlParam.QP_P=35;
                   }
                   else if(BitRate>1200)
                   {
                      mpRateControlParam.InitQP=40;
                      mpRateControlParam.QP_I=40;
                      mpRateControlParam.QP_P=40;
                   }
                   else
                   {
                      mpRateControlParam.InitQP=45;
                      mpRateControlParam.QP_I=45;
                      mpRateControlParam.QP_P=45;
                   }
                }
                else if(BitRate>400)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 35;
                   video_double_field_flag = 1;
                }
                else if(BitRate>300)
                {
                   DropFrame=5;       // 5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 40;
                   video_double_field_flag = 1;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 40;
                   video_double_field_flag = 1;
                }
                //----------------------------------------------------------//
                if(P2PVideoQuality==RF_P2PVdoQalt_VGA_15_FPS)
                {
                    TargetBitrate=1000;
                    if(DropFrame<=1)
                       DropFrame=1;   //fixed to under 15 fps
                }
                else if(P2PVideoQuality==RF_P2PVdoQalt_VGA_30_FPS)
                {
                    TargetBitrate=2000;
                    DropFrame=0;   //fixed to under 30 fps
                }
                else if(P2PVideoQuality==RF_P2PVdoQalt_VGA_10_FPS)  
                {
                    TargetBitrate=400;
                    if(DropFrame<=2)
                       DropFrame=2;   //fixed to under 10 fps

                    if(BitRate>400)  // 10 fps
                    {
                       dftMpeg4Quality = 35;

                    }
                    else if(BitRate>200)
                    {
                       dftMpeg4Quality = 40;
                       DropFrame=9;
                    }
                    else
                    {
                       dftMpeg4Quality = 45;
                       DropFrame=14;
                    }   

                }
                else
                { 
                   TargetBitrate=1000;
                   TargetBitrate=1000;
                   if(DropFrame<=1)
                       DropFrame=1;   //fixed to under 15 fps
                }
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
            }
            else
            {   //not implement now//
                DEBUG_MP4("Warning!! VGA,Frame rate is not approved:%d,%d",mpeg4Width,VideoInFrameRate);
            }
            
        }
        //============= HD ===============//
        else if( SizeLevel == MPEG4_REC_HD) //HD size
        {
                video_double_field_flag = 0;
                if( (P2PVideoQuality==RF_P2PVdoQalt_HD_7_FPS) && (BitRate>1700) )
                { 
                   DropFrame=1;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 35;
                }
                else if(BitRate>1200)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 35;
                }
                else if(BitRate>1000)  // 10 fps
                {
                   DropFrame=2;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 40;
                }
                else if(BitRate>800)  // 7.5 fps
                {
                   DropFrame=3;
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 40;
                }
                else if(BitRate>500)
                {
                   DropFrame=5;       // 5 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 40;
                }
                else
                {
                   DropFrame=9;       // 3 fps
                   mpRateControlParam.enable_ratecontrol   = 0;
                   dftMpeg4Quality = 45;
                }

                if(P2PVideoQuality==RF_P2PVdoQalt_HD_7_FPS)
                {
                    TargetBitrate=1700;
                    if(DropFrame<=1)
                       DropFrame=1;   //fixed to under 5 fps
                }
                else
                {
                    TargetBitrate=1200;
                    if(DropFrame<=2)
                       DropFrame=2;   //fixed to under 10 fps
                }
            
                mpRateControlParam.Framerate  = VideoInFrameRate/(DropFrame+1); 
        }
        TVout_Generate_Pause_Frame = 0;
        ASF_set_interlace_flag = 0;
              
        return DropFrame;
    }
   #endif

#else
void rfiuTXMpeg4EncTask(void* pData)
{

}
#endif
int mpeg4ModifyTargetBitRate(int NewBitRate)
{
     mpRateControlParam.RCQ2_config.bit_rate=NewBitRate;
     mpRateControlParam.RCQ2_config.target_rate=NewBitRate/mpRateControlParam.Framerate;
     
     return 1;
}

int mpeg4ModifyQp(short Qp,int Type)
{
     if(Type==I_VOP) //I frame
         mpRateControlParam.QP_I = mpRateControlParam.InitQP + Qp;
     else //P frame
         mpRateControlParam.QP_P = mpRateControlParam.InitQP + Qp;
     
     return 1;
}

/*BJ 0530 S*/   
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


u32 mpeg4PutVOPHeader(u8 *pBuf, u32 pFlag, s64 *pTime, u32 *byteno,u32 FrameIdx)
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
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 32, 0x000001b6);
        
    // Picture coding type
    //*pFlag: I(00),P(01),B(10),S(11)
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 2, (u32) pFlag);
    // Modulo time base
    mpeg4VopTimeInc    += (*pTime)*(VOP_TIME_INCREMENT_RESOLUTION/1000);
    while(mpeg4VopTimeInc >= VOP_TIME_INCREMENT_RESOLUTION)
    {
      *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 1, 1);
      mpeg4VopTimeInc -=VOP_TIME_INCREMENT_RESOLUTION;
    }
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 1, 0); //???
    //marker bit
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 1, 1);  
    // vop time increment
    
    temp = VOP_TIME_INCREMENT_RESOLUTION;
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
    
    //vop_rounding_type:: Only P frame
    if(pFlag == P_VOP)
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 1, FrameIdx&0x01);

    //intra_dc_vlc_thr//
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 3, 0);    

    //vop_quant (5 bits)
    *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 5, (u32) dftMpeg4Quality);
        
    if (pFlag !=I_VOP)
    {
        //search range always is +/- 16.
        *byteno += mpeg4PutHeader(pBuf+*byteno, &bitno, 3, 1);
    }       
    bitpos = ((*byteno & 0x03)<<3) + bitno;
    if (bitno !=0)
    {
        *(pBuf+*byteno) <<= (8-bitno);  
    }
    *byteno &= 0xfc;

    return bitpos;
}

#if 1//FORCE_FPS

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

#endif  // FORCE_FPS

s32 mpeg4Coding1Frame(u8* FrameBuf, u32 FrameType, s64* FrameTime, u32* CmpSize, u32 FrameIdx, u32 mpeg4StartBits,u32* pOffset)
{
    u8  err;
    u32 mpeg4RTYPE, mpeg4Quality;
    u32 temp,i;
    u8  mpeg4RefSliceSize, mpeg4IMbRefXpos, mpeg4IMbRefYpos; 
    u32 mbWidth = (mpeg4Width + 15) >> 4; /*CY 0907*/
    u32 mbNoSize, mbNo;       /*CY 0907*/
    u32 mbHeight;          /*CY 0907*/
    u32 mpeg4VdPacketSize;
    volatile INT16U *pCurISUSemCnt = &isuSemEvt->OSEventCnt;
    u8  *RefBuf_Y, *RefBuf_Cb, *RefBuf_Cr, *McoBuf_Y, *McoBuf_Cb, *McoBuf_Cr;

    
    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif
    SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
    for(i=0;i<5;i++); //delay
    SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);
    
    // 0x0104
    mpeg4RTYPE      = FrameIdx & 0x01;
    mpeg4Quality    = (u32) dftMpeg4Quality;
    Mpeg4MbParam    = (mpeg4Quality & 0x1f) | (mpeg4RTYPE << 8) | (FrameType << 16);

    //0x0100
#if (MPEG4_FIELD_ENC_ENA && (CIU1_OPTION ==Sensor_CCIR656) )
    mbWidth             = (u32) ((mpeg4Width + 15) >> 4);    /*CY 0907*/
    mbHeight            = (u32) ((mpeg4Height/2 + 15) >> 4);   /*CY 0907*/
    mbNo = mbWidth * mbHeight;      /*CY 0907*/
    Mpeg4FrameSize      = (mbWidth << mbWidthShft) | 
                          (mbHeight << mbHeightShft) |
                          (mbNo << mbNoShft);   /*CY 0907*/
    Mpeg4SourceStride   = mpeg4Width*2;
#else
    mbWidth             = (u32) ((mpeg4Width + 15) >> 4);    /*CY 0907*/
    mbHeight            = (u32) ((mpeg4Height + 15) >> 4);   /*CY 0907*/
    mbNo = mbWidth * mbHeight;      /*CY 0907*/
    Mpeg4FrameSize      = (mbWidth << mbWidthShft) | 
                          (mbHeight << mbHeightShft) |
                          (mbNo << mbNoShft);   /*CY 0907*/
    Mpeg4SourceStride   = mpeg4Width;
#endif
    // 0x0200
    mpeg4VdPacketSize = MPEG4_VDPACKET_SIZE;
    Mpeg4ErrResil = mpeg4VdPacketSize | MPEG4_RESY_ENA;

    // 0x0204
    mbNoSize    = 0;           /*CY 0907*/ 
    temp = mbNo;            /*CY 0907*/
    while(temp > 0)
    {
        temp  >>= 1;
        mbNoSize++;     /*CY 0907*/
    }
    Mpeg4DecVidPkt  = mbNoSize;  /*CY 0907*/
    // 0x0300
    Mpeg4MeThresh1 = (MPEG4_INTRAINTER_THD & 0x03ff) | (MPEG4_ONEFOURMV_THD << 16);
    // 0x0304
    Mpeg4MeThresh2 = (MPEG4_BIASSAD_8 & 0x03ff) | (MPEG4_BIASSAD_16 << 16);
    // 0x0308 -- read only
    if (FrameType == I_VOP)
            mpeg4MBRef  = -1;
    else
        mpeg4MBRef++;
    
    mpeg4RefSliceSize   = 3;
    mpeg4IMbRefXpos     = mpeg4MBRef % mbWidth;                    /*CY 0907*/
    mpeg4IMbRefYpos     = (mpeg4MBRef / mbWidth) & mpeg4SliceMask[mpeg4RefSliceSize];   /*CY 0907*/
    Mpeg4IntraMbRefresh = (mpeg4IMbRefXpos) | (mpeg4IMbRefYpos << mpeg4IMbRefYpos_Shft) | (mpeg4RefSliceSize << mpeg4RefSliceSize_Shft) | (MPEG4_IMBREF_DIS);
    // 0x0400
    Mpeg4IntEna         = 0x01;
    
    Mpeg4StreamAddr     = (u32) FrameBuf;
    // 0x0504
    Mpeg4StreamStartBit = mpeg4StartBits; 
    //*CmpSize           += (mpeg4StartBits>>3);
    // 0x0508
    temp                = (*FrameBuf << 24) + (*(FrameBuf + 1) << 16) + (*(FrameBuf + 2) << 8) + *(FrameBuf + 3);
    if (mpeg4StartBits == 0)
        Mpeg4StreamStartWord = 0;
    else
        Mpeg4StreamStartWord = temp & ~(PutBitsMask[31-mpeg4StartBits]);

    //0x0510
    Mpeg4VopParam       = 0x01;
    
    //0x0600
    // pre-set all MV to 0xff
    memset( mpeg4MVBuf, 0xff, MPEG4_MVBUF); 
    Mpeg4MvBufAddr = (u32) &(mpeg4MVBuf[0]);
    //0x0604  // 0x0608
    
#if MULTI_CHANNEL_SUPPORT
  #if (MULTI_CHANNEL_SEL & 0x01)
    Mpeg4CurrRawYAddr   = (u32) PNBuf_Y[VideoPictureIndex % 4];
    Mpeg4CurrRawCbAddr  = (u32) PNBuf_C[VideoPictureIndex % 4];
  #elif (MULTI_CHANNEL_SEL & 0x02)
    #if HW_DEINTERLACE_CIU1_ENA
    Mpeg4CurrRawYAddr   = (u32) PNBuf_sub1[(VideoPictureIndex-1) % 4];
    Mpeg4CurrRawCbAddr  = (u32) (PNBuf_sub1[(VideoPictureIndex-1) % 4] + PNBUF_SIZE_Y);
    #else
    Mpeg4CurrRawYAddr   = (u32) PNBuf_sub1[VideoPictureIndex % 4];
    Mpeg4CurrRawCbAddr  = (u32) (PNBuf_sub1[VideoPictureIndex % 4] + PNBUF_SIZE_Y);
    #endif
  #elif (MULTI_CHANNEL_SEL & 0x04)
    #if HW_DEINTERLACE_CIU2_ENA
    Mpeg4CurrRawYAddr   = (u32) PNBuf_sub2[(VideoPictureIndex-1) % 4];
    Mpeg4CurrRawCbAddr  = (u32) (PNBuf_sub2[(VideoPictureIndex-1) % 4] + PNBUF_SIZE_Y);
    #else
    Mpeg4CurrRawYAddr   = (u32) PNBuf_sub2[VideoPictureIndex % 4];
    Mpeg4CurrRawCbAddr  = (u32) (PNBuf_sub2[VideoPictureIndex % 4] + PNBUF_SIZE_Y);
    #endif
  #endif
#else   // MULTI_CHANNEL_SUPPORT == 0
    Mpeg4CurrRawYAddr   = (u32) PNBuf_Y[VideoPictureIndex % 4];
    Mpeg4CurrRawCbAddr  = (u32) PNBuf_C[VideoPictureIndex % 4];
#endif


        RefBuf_Y   = mpeg4PRefBuf_Y;
        RefBuf_Cb  = mpeg4PRefBuf_Cb;
        RefBuf_Cr  = mpeg4PRefBuf_Cr;
        
        McoBuf_Y   = mpeg4NRefBuf_Y;
        McoBuf_Cb  = mpeg4NRefBuf_Cb;
        McoBuf_Cr  = mpeg4NRefBuf_Cr;

        //Exchange address//
        mpeg4PRefBuf_Y  = McoBuf_Y;
        mpeg4PRefBuf_Cb = McoBuf_Cb;
        mpeg4PRefBuf_Cr = McoBuf_Cr;
        
        mpeg4NRefBuf_Y  = RefBuf_Y;
        mpeg4NRefBuf_Cb = RefBuf_Cb;
        mpeg4NRefBuf_Cr = RefBuf_Cr;


    // 0x0610
    /* Peter 070108 S */
    Mpeg4CurrRecInYAddr     = (u32) &(McoBuf_Y[(mpeg4Width + 32) * 16 + 16]);  
    // 0x0614
    Mpeg4CurrRecInCbAddr    = (u32) &(McoBuf_Cb[((mpeg4Width >> 1) + 16) * 8 + 8]);
    // 0x0618
    Mpeg4CurrRecInCrAddr    = (u32) &(McoBuf_Cr[((mpeg4Width >> 1) + 16) * 8 + 8]);
    // 0x061C
    Mpeg4CurrRecOutYAddr    = (u32) &(McoBuf_Y[0]);
    // 0x0620
    Mpeg4CurrRecOutCbAddr   = (u32) &(McoBuf_Cb[0]);
    // 0x0624
    Mpeg4CurrRecOutCrAddr   = (u32) &(McoBuf_Cr[0]);
    // 0x0628
    Mpeg4PrevRecInYAddr     = (u32) &(RefBuf_Y[(mpeg4Width + 32) * 16 + 16]);
    // 0x062C
    Mpeg4PrevRecInCbAddr    = (u32) &(RefBuf_Cb[((mpeg4Width >> 1) + 16) * 8 + 8]);
    // 0x0630
    Mpeg4PrevRecInCrAddr    = (u32) &(RefBuf_Cr[((mpeg4Width >> 1) + 16) * 8 + 8]);
    // 0x0634
    Mpeg4PrevRecOutYAddr    = (u32) &(RefBuf_Y[0]);
    // 0x0638
    Mpeg4PrevRecOutCbAddr   = (u32) &(RefBuf_Cb[0]);
    // 0x063C
    Mpeg4PrevRecOutCrAddr   = (u32) &(RefBuf_Cr[0]);  
    /* Peter 070108 E */
    mpegflag    = 1;

#if MPEG_DEBUG_ENA_LUCIAN
     gpioSetLevel(0, 1, 1);
#endif


#if (MPEG4_FIELD_ENC_ENA && (CIU1_OPTION ==Sensor_CCIR656) )
   #if USE_MPEG_QUANTIZATION
    Mpeg4Ctrl   = MPEG4_QM_MPEG | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
   #else
    Mpeg4Ctrl   = MPEG4_QM_H263 | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
   #endif
#else
  #if (CIU1_BOB_REPLACE_MPEG_DF && ( (CIU1_OPTION == Sensor_CCIR656) || (CIU1_OPTION == Sensor_CCIR601) ))
   #if USE_MPEG_QUANTIZATION
    Mpeg4Ctrl   = MPEG4_QM_MPEG | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
   #else
    Mpeg4Ctrl   = MPEG4_QM_H263 | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
   #endif
  #else
   #if USE_MPEG_QUANTIZATION
    if(video_double_field_flag)
        Mpeg4Ctrl   = MPEG4_QM_MPEG | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_DB_FIELD | MPEG4_ENC_TRG;
    else
        Mpeg4Ctrl   = MPEG4_QM_MPEG | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
   #else
    if(video_double_field_flag)
        Mpeg4Ctrl   = MPEG4_QM_H263 | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_DB_FIELD | MPEG4_ENC_TRG;
    else
        Mpeg4Ctrl   = MPEG4_QM_H263 | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
   #endif
  #endif
#endif
    //DEBUG_MP4("0x%x ",Mpeg4Ctrl);
    OSSemPend(VideoCpleSemEvt, MPEG4_TIMEOUT, &err);
    
    if (err != OS_NO_ERR)
    {
        // reset MPEG-4 hardware
        SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100; 
        
        DEBUG_MP4("Encoder Error: VideoCpleSemEvt is %d.\n", err);
        DEBUG_MP4("VideoPictureIndex = %d\n",VideoPictureIndex);
        MPEG4_Error = 1;
    }

#if (MPEG4_FIELD_ENC_ENA && (CIU1_OPTION ==Sensor_CCIR656) )
    // 0x0104
    mpeg4RTYPE      = FrameIdx & 0x01;
    mpeg4Quality    = (u32) dftMpeg4Quality;
    Mpeg4MbParam    = (mpeg4Quality & 0x1f) | (mpeg4RTYPE << 8) | (FrameType << 16);

    //0x0100
    mbWidth             = (u32) ((mpeg4Width + 15) >> 4);    /*CY 0907*/
    mbHeight            = (u32) ((mpeg4Height/2 + 15) >> 4);   /*CY 0907*/
    mbNo = mbWidth * mbHeight;      /*CY 0907*/
    Mpeg4FrameSize      = (mbWidth << mbWidthShft) | 
                          (mbHeight << mbHeightShft) |
                          (mbNo << mbNoShft);   /*CY 0907*/
    Mpeg4SourceStride   = mpeg4Width*2;

    // 0x0200
    mpeg4VdPacketSize = MPEG4_VDPACKET_SIZE;
    Mpeg4ErrResil = mpeg4VdPacketSize | MPEG4_RESY_ENA;

    // 0x0204
    mbNoSize    = 0;           /*CY 0907*/ 
    temp = mbNo;            /*CY 0907*/
    while(temp > 0)
    {
        temp  >>= 1;
        mbNoSize++;     /*CY 0907*/
    }
    Mpeg4DecVidPkt  = mbNoSize;  /*CY 0907*/
    // 0x0300
    Mpeg4MeThresh1 = (MPEG4_INTRAINTER_THD & 0x03ff) | (MPEG4_ONEFOURMV_THD << 16);
    // 0x0304
    Mpeg4MeThresh2 = (MPEG4_BIASSAD_8 & 0x03ff) | (MPEG4_BIASSAD_16 << 16);
    
    // 0x0308 -- read only
    
    // 0x0400
    Mpeg4IntEna         = 0x01;

    // 0x0504
    Mpeg4StreamAddr      = (u32) FrameBuf + ((Mpeg4EncStreamSize + (mpeg4StartBits>>3) +3) & (~0x03));
    
    Mpeg4StreamStartBit  = 0;    
    
    Mpeg4StreamStartWord = 0;
    
    //0x0510
    Mpeg4VopParam       = 0x01;
    
    //0x0600
    // pre-set all MV to 0xff
    memset( mpeg4MVBuf, 0xff, MPEG4_MVBUF); 
    Mpeg4MvBufAddr = (u32) &(mpeg4MVBuf[0]);
    //0x0604  // 0x0608
    
    // 0x050C
    *CmpSize += ((Mpeg4EncStreamSize + (mpeg4StartBits>>3) +3) & (~0x03));
    *pOffset = *CmpSize;

    //DEBUG_MP4("(%d,0x%x) ",*pOffset,Mpeg4StreamAddr);

    //0x0600
    // pre-set all MV to 0xff
    memset( mpeg4MVBuf, 0xff, MPEG4_MVBUF); 
    Mpeg4MvBufAddr = (u32) &(mpeg4MVBuf[0]);
    //0x0604  // 0x0608
    
#if MULTI_CHANNEL_SUPPORT
  #if (MULTI_CHANNEL_SEL & 0x01)
    Mpeg4CurrRawYAddr   = (u32) PNBuf_Y[VideoPictureIndex % 4] + mpeg4Width;
    Mpeg4CurrRawCbAddr  = (u32) PNBuf_C[VideoPictureIndex % 4] + mpeg4Width;
  #elif (MULTI_CHANNEL_SEL & 0x02)
    Mpeg4CurrRawYAddr   = (u32) PNBuf_sub1[VideoPictureIndex % 4] +  mpeg4Width;
    Mpeg4CurrRawCbAddr  = (u32) (PNBuf_sub1[VideoPictureIndex % 4] + PNBUF_SIZE_Y) +  mpeg4Width;
  #elif (MULTI_CHANNEL_SEL & 0x04)
    Mpeg4CurrRawYAddr   = (u32) PNBuf_sub2[VideoPictureIndex % 4] +  mpeg4Width;
    Mpeg4CurrRawCbAddr  = (u32) (PNBuf_sub2[VideoPictureIndex % 4] + PNBUF_SIZE_Y) +  mpeg4Width;
  #endif
#else   // MULTI_CHANNEL_SUPPORT == 0
    Mpeg4CurrRawYAddr   = (u32) PNBuf_Y[VideoPictureIndex % 4] + mpeg4Width;
    Mpeg4CurrRawCbAddr  = (u32) PNBuf_C[VideoPictureIndex % 4] + mpeg4Width;
#endif

    /*
    RefBuf_Y   = mpeg4PRefBuf_Y;
    RefBuf_Cb  = mpeg4PRefBuf_Cb;
    RefBuf_Cr  = mpeg4PRefBuf_Cr;
    
    McoBuf_Y   = mpeg4NRefBuf_Y;
    McoBuf_Cb  = mpeg4NRefBuf_Cb;
    McoBuf_Cr  = mpeg4NRefBuf_Cr;

    //Exchange address//
    mpeg4PRefBuf_Y  = McoBuf_Y;
    mpeg4PRefBuf_Cb = McoBuf_Cb;
    mpeg4PRefBuf_Cr = McoBuf_Cr;
    
    mpeg4NRefBuf_Y  = RefBuf_Y;
    mpeg4NRefBuf_Cb = RefBuf_Cb;
    mpeg4NRefBuf_Cr = RefBuf_Cr;
    */
    Mpeg4CurrRecInYAddr     += ((720 + 32) * (576 + 32));  
    // 0x0614
    Mpeg4CurrRecInCbAddr    += ((720/2 + 32) * (576/2 + 32));
    // 0x0618
    Mpeg4CurrRecInCrAddr    += ((720/2 + 32) * (576/2 + 32));
    // 0x061C
    Mpeg4CurrRecOutYAddr    += ((720 + 32) * (576 + 32));
    // 0x0620
    Mpeg4CurrRecOutCbAddr   += ((720/2 + 32) * (576/2 + 32));
    // 0x0624
    Mpeg4CurrRecOutCrAddr   += ((720/2 + 32) * (576/2 + 32));
    // 0x0628
    Mpeg4PrevRecInYAddr     += ((720 + 32) * (576 + 32));
    // 0x062C
    Mpeg4PrevRecInCbAddr    += ((720/2 + 32) * (576/2 + 32));
    // 0x0630
    Mpeg4PrevRecInCrAddr    += ((720/2 + 32) * (576/2 + 32));
    // 0x0634
    Mpeg4PrevRecOutYAddr    += ((720 + 32) * (576 + 32));
    // 0x0638
    Mpeg4PrevRecOutCbAddr   += ((720/2 + 32) * (576/2 + 32));
    // 0x063C
    Mpeg4PrevRecOutCrAddr   += ((720/2 + 32) * (576/2 + 32)); 

    mpegflag    = 2;
 #if USE_MPEG_QUANTIZATION
    Mpeg4Ctrl   = MPEG4_QM_MPEG | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
 #else
    Mpeg4Ctrl   = MPEG4_QM_H263 | MPEG4_VDI_YC | MPEG4_DBG_DISA | MPEG4_ENC_TRG;
 #endif

    OSSemPend(VideoCpleSemEvt, MPEG4_TIMEOUT, &err);
    
    if (err != OS_NO_ERR)
    {
        // reset MPEG-4 hardware
        SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100; 
        
        DEBUG_MP4("Encoder Error: VideoCpleSemEvt is %d.\n", err);
        DEBUG_MP4("VideoPictureIndex = %d\n",VideoPictureIndex);
        MPEG4_Error = 1;
    }

     //DEBUG_MP4("(%d,%d) ",*CmpSize >> 10,((Mpeg4EncStreamSize +3) & (~0x03))>> 10);
    *CmpSize += ((Mpeg4EncStreamSize +3) & (~0x03));

    //DEBUG_MP4("Size=%d\n",Mpeg4EncStreamSize);
#else
    // 0x050C
    *CmpSize += Mpeg4EncStreamSize + (mpeg4StartBits>>3);
    *pOffset = *CmpSize;  
    //DEBUG_MP4("(%d) ",*CmpSize >> 10);
#endif

    if((*CmpSize < 0x20) || (*CmpSize > MPEG4_MIN_BUF_SIZE))
    {
        DEBUG_MP4("Warning!! Mpeg4EncStreamSize == %d Bytes!!!\n", Mpeg4EncStreamSize);
    }

#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif  
   OSSemPost(mpeg4ReadySemEvt);    // release MPEG4 HW

    return 1;   
}

    
s32 mpeg4Output1Frame(u8* pBuf, s64* pTime, u32* pSize, u32* Mpeg4EncCnt, u32* pOffset)
{
    u8  err;
    u8  i;
    u32 bitpos; /*BJ 0530 S*/

#if MULTI_CHANNEL_SUPPORT
  #if (MULTI_CHANNEL_SEL & 0x01)
    OSSemPend(isuSemEvt, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
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
  #elif (MULTI_CHANNEL_SEL & 0x02)
    OSSemPend(ciuCapSemEvt_CH1, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        #if CiuLossSemEvtBUG
        #else
        ciu_1_Stop();
        #endif

        DEBUG_MP4("Error: ciuCapSemEvt_CH1(video capture mode) is %d.\n", err);

        //Reset CIU
        #if CiuLossSemEvtBUG
        #else
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        #endif
        return 1;
    }
  #elif (MULTI_CHANNEL_SEL & 0x04)
    OSSemPend(ciuCapSemEvt_CH2, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        ciu_2_Stop();

        DEBUG_MP4("Error: ciuCapSemEvt_CH2(video capture mode) is %d.\n", err);

        //Reset CIU
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        return 1;
    }
  #elif (MULTI_CHANNEL_SEL & 0x08)
    OSSemPend(ciuCapSemEvt_CH3, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        ciu_3_Stop();

        DEBUG_MP4("Error: ciuCapSemEvt_CH3(video capture mode) is %d.\n", err);

        //Reset CIU
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        return 1;
    }
  #elif (MULTI_CHANNEL_SEL & 0x10)
    OSSemPend(ciuCapSemEvt_CH4, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        ciu_4_Stop();

        DEBUG_MP4("Error: ciuCapSemEvt_CH4(video capture mode) is %d.\n", err);

        //Reset CIU
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        return 1;
    }  
  #endif
#else   // MULTI_CHANNEL_SUPPORT == 0
    OSSemPend(isuSemEvt, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
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
#endif  // #if MULTI_CHANNEL_SUPPORT
    
    *pSize  = 0;
    *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; 

     
    bitpos  = mpeg4PutVOPHeader(pBuf, Vop_Type, pTime, pSize,*Mpeg4EncCnt);
    mpeg4Coding1Frame(pBuf+*pSize, Vop_Type, pTime, pSize, *Mpeg4EncCnt, bitpos,pOffset);
    
    *Mpeg4EncCnt = (*Mpeg4EncCnt) + 1;

}
/*BJ 0530 E*/   
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

/*Peter 0707 S*/
//////////////////////////////////////////////////////////
//
// MPEG-4 decoder functions
//
//////////////////////////////////////////////////////////

#define _SWAP(a) ((a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3])

u32 ShowHeaderBits (s32 n, MP4Dec_Bits* Bits)
{
    u8  *v;
    s32 rbit;
    u32 a;
    u32 b;

    v       = Bits->rdptr;
    b       = _SWAP(v);   
    rbit    = 32 - Bits->Bigbitpos;
    if(rbit - n >= 0) {
        a   = (b & PutBitsMask[rbit - 1]) >> (rbit - n);
    } else {
        a   = (b & PutBitsMask[rbit - 1]) << (n - rbit);
        v  += 4;
        b   = _SWAP(v);
        a  |= (b >>  (32 - (n - rbit)));
    }
    return a;
}

void  FlushHeaderBits(s32 n, MP4Dec_Bits* Bits)
{

    Bits->Bigbitpos    += n;
    if(Bits->Bigbitpos >= 32) 
    {
        Bits->Read_bytecnt += 4;
        Bits->rdptr        += 4;
        Bits->Bigbitpos     = (Bits->Bigbitpos & 0x01f);
    }
}

/*

Routine Description:

    Get MPEG4 header value.

Arguments:

    pData - The task parameter.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
u32 mpeg4GetHeader(s32 n, MP4Dec_Bits* Bits)
{
  u32 l;

  l     = ShowHeaderBits(n, Bits);
  FlushHeaderBits(n, Bits);

  return l;
}

s32 FindMSB(int level)
{
    s32 MSB_mask    = 0x0001;
    s32 MSB         = 0;
    while(level >= MSB_mask) {
        MSB++;
        MSB_mask  <<= 1;
    }
    
    return MSB;
}

s32 ParseMPEG4Header(MP4_Option *pMp4Dec_opt,MP4Dec_Bits* Bits, s32 BitsLen)
{       
    u32 vol_ver_id;
    u32 start_code;
    u32 remainder;
    s32 modulo_time_base;
    u32 shape, quarter_pixel, data_partioned;

    //==========================================//
    /*
    Bits->Read_bytecnt = 0;
    Bits->Bigbitpos = 0;
    Bits->rdptr = buffer;
    */

    do
    {
        //Force bitstream byte alignment//
        remainder = (Bits->Bigbitpos & 0x07);
        if (remainder) 
            FlushHeaderBits(8 - remainder,Bits);
        start_code = ShowHeaderBits(32,Bits);
        //
        if (start_code == VISOBJSEQ_START_CODE)
        {
            FlushHeaderBits(32,Bits);                    // visual_object_sequence_start_code
            FlushHeaderBits(8,Bits);                 // profile_and_level_indication
        }
        else if (start_code == VISOBJSEQ_STOP_CODE)
        {
            FlushHeaderBits(32,Bits);                    // visual_object_sequence_stop_code
        }
        else if (start_code == VISOBJ_START_CODE)
        {
            FlushHeaderBits(32,Bits);                    // visual_object_start_code
            if (mpeg4GetHeader(1,Bits))             // is_visual_object_identified
            {
                vol_ver_id = mpeg4GetHeader(4,Bits);  // visual_object_ver_id
                FlushHeaderBits(3,Bits);             // visual_object_priority
            }
            else
            {
                vol_ver_id = 1;
            }

            if (ShowHeaderBits(4,Bits) != VISOBJ_TYPE_VIDEO)   // visual_object_type
            {
                DEBUG_MP4("visual_object_type != video\n");
                return 0;
            }
            FlushHeaderBits(4,Bits);

            // video_signal_type
            if (mpeg4GetHeader(1,Bits))         // video_signal_type
            {
                FlushHeaderBits(3,Bits);        // video_format
                FlushHeaderBits(1,Bits);        // video_range
                if (mpeg4GetHeader(1,Bits))     // color_description
                {
                    FlushHeaderBits(8,Bits);    // color_primaries
                    FlushHeaderBits(8,Bits);    // transfer_characteristics
                    FlushHeaderBits(8,Bits);    // matrix_coefficients
                }
            }
        }
        else if ((start_code & ~0x1f) == VIDOBJ_START_CODE)
        {
            FlushHeaderBits(32, Bits);          // video_object_start_code
        } 
        else if ((start_code & ~0xf) == VIDOBJLAY_START_CODE)
        {
            FlushHeaderBits(32, Bits);          // video_object_layer_start_code
            mpeg4GetHeader(1, Bits);            // random_accessible_vol

            // video_object_type_indication
            if (ShowHeaderBits(8,Bits) != VIDOBJLAY_TYPE_SIMPLE &&
                ShowHeaderBits(8,Bits) != VIDOBJLAY_TYPE_CORE &&
                ShowHeaderBits(8,Bits) != VIDOBJLAY_TYPE_MAIN &&
                ShowHeaderBits(8,Bits) != 0)     // BUGGY DIVX
            {
                DEBUG_MP4("video_object_type_indication not supported\n");
                return 0;
            }
            FlushHeaderBits(8, Bits);

            if (mpeg4GetHeader(1, Bits))                // is_object_layer_identifier
            {
                vol_ver_id  = mpeg4GetHeader(4, Bits);  // video_object_layer_verid
                FlushHeaderBits(3, Bits);               // video_object_layer_priority
            }
            else
            {
                vol_ver_id  = 1;
            }

            if (mpeg4GetHeader(4,Bits) == VIDOBJLAY_AR_EXTPAR)  // aspect_ratio_info
            {
                FlushHeaderBits(8,Bits);                        // par_width
                FlushHeaderBits(8,Bits);                        // par_height
            }

            if (mpeg4GetHeader(1, Bits))            // vol_control_parameters
            {
                FlushHeaderBits(2, Bits);           // chroma_format
                mpeg4GetHeader(1, Bits);            // low_delay
                if (mpeg4GetHeader(1, Bits))        // vbv_parameters
                {
                    FlushHeaderBits(15, Bits);      // first_half_bitrate
                    mpeg4GetHeader(  1, Bits);      // READ_MARKER();
                    FlushHeaderBits(15, Bits);      // latter_half_bitrate
                    mpeg4GetHeader(  1, Bits);      // READ_MARKER();
                    FlushHeaderBits(15, Bits);      // first_half_vbv_buffer_size
                    mpeg4GetHeader(  1, Bits);      // READ_MARKER();
                    FlushHeaderBits( 3, Bits);      // latter_half_vbv_buffer_size
                    FlushHeaderBits(11, Bits);      // first_half_vbv_occupancy
                    mpeg4GetHeader(  1, Bits);      // READ_MARKER();
                    FlushHeaderBits(15, Bits);      // latter_half_vbv_occupancy
                    mpeg4GetHeader(  1, Bits);      // READ_MARKER();
                
                }
            }

            shape   = mpeg4GetHeader(2,Bits);       // video_object_layer_shape
            
            if (shape != VIDOBJLAY_SHAPE_RECTANGULAR)
            {
                DEBUG_MP4("Error: not rectangular shape!!!\n");
                return 0;
            }

            mpeg4GetHeader(1,Bits);                 //READ_MARKER();

            pMp4Dec_opt->time_increment_resolution = mpeg4GetHeader(16,Bits); // vop_time_increment_resolution
            //Mp4Dec_opt.time_increment_resolution--;
            if (pMp4Dec_opt->time_increment_resolution > 0)
            {
                pMp4Dec_opt->fix_vop_timeincr_length = FindMSB(pMp4Dec_opt->time_increment_resolution - 1);
            }
            else
            {               
                pMp4Dec_opt->fix_vop_timeincr_length = 1;
            }

            mpeg4GetHeader(1,Bits); //READ_MARKER();

            if ( mpeg4GetHeader(1,Bits) )       // fixed_vop_rate
            {
                FlushHeaderBits(pMp4Dec_opt->fix_vop_timeincr_length, Bits);  // fix_vop_timeincr_length
            }

            if (shape != VIDOBJLAY_SHAPE_BINARY_ONLY)
            {

                if (shape == VIDOBJLAY_SHAPE_RECTANGULAR)
                {
                    mpeg4GetHeader(1,Bits); // marker
                    pMp4Dec_opt->Width    = mpeg4GetHeader(13,Bits);
                    mpeg4GetHeader(1,Bits); // marker
                    pMp4Dec_opt->Height   = mpeg4GetHeader(13,Bits);
                    mpeg4GetHeader(1,Bits); // marker
                    #if TUTK_SUPPORT
                    gPlaybackWidth=pMp4Dec_opt->Width;
                    gPlaybackHeight=pMp4Dec_opt->Height;
                    if((sysPlaybackVideoStart == 1) && (!sysTVOutOnFlag))
                    {
                        if(pMp4Dec_opt->Width == 320)
                            iduPlaybackMode(320,240,640);
                        else
                            iduPlaybackMode(640,480,640);
                    }

                    #else
                    if((sysPlaybackVideoStart == 1) && (!sysTVOutOnFlag))
                    {
                        if(pMp4Dec_opt->Width == 320)
                            iduPlaybackMode(320,240,640);
                        else
                            iduPlaybackMode(640,480,640);
                    }
                    #endif
                }

                if (mpeg4GetHeader(1,Bits)) // interlaced
                {
                    DEBUG_MP4("Don't support interlace!!!\n");
                }

                if (!mpeg4GetHeader(1,Bits) )                // obmc_disable
                {
                    DEBUG_MP4("Don't support OBMC!!!\n");
                }

                if ( mpeg4GetHeader( vol_ver_id == 1 ? 1 : 2 ,Bits)  )// sprite_enable
                {
                    DEBUG_MP4("Don't support sprite!!!\n");
                    return 0;
                }
            
                if (vol_ver_id != 1 && shape != VIDOBJLAY_SHAPE_RECTANGULAR)
                {
                    FlushHeaderBits(1, Bits);                       // sadct_disable
                }

                if (mpeg4GetHeader(1, Bits))                        // not_8_bit
                {
                    DEBUG_MP4("Don't support not_8_bit!!!\n");
                    mpeg4GetHeader(4, Bits);                        // quant_precision
                    FlushHeaderBits(4, Bits);                       // bits_per_pixel
                }

                pMp4Dec_opt->Quant_type = mpeg4GetHeader(1, Bits);    // quant_type
                
                if (pMp4Dec_opt->Quant_type)
                {
                    DEBUG_MP4("MPEG quantization mode!!!\n");
                    if (mpeg4GetHeader(1, Bits))    // load_intra_quant_mat
                    {
                        DEBUG_MP4("Don't support user define intra quantization table!!!\n");
                        return 0;
                    }

                    if (mpeg4GetHeader(1, Bits))    // load_inter_quant_mat
                    {
                        DEBUG_MP4("Don't support user define inter quantization table!!!\n");
                        return 0;
                    }
                }
                
                if (vol_ver_id != 1)
                {
                    quarter_pixel = mpeg4GetHeader(1, Bits);    // quarter_sampe
                    if (quarter_pixel)
                    {
                        DEBUG_MP4("Don't support quarter_sample!!!\n");
                        return 0;
                    }
                }
                else
                {
                    quarter_pixel = 0;
                }

                if (!mpeg4GetHeader(1, Bits))       // complexity_estimation_disable
                {
                    DEBUG_MP4("Don't support complexity_estimation header!!!\n");
                    return 0;
                }
                
                pMp4Dec_opt->resync_marker_disable    = mpeg4GetHeader(1,Bits); // resync_marker_disable
                data_partioned                      = mpeg4GetHeader(1,Bits);
                if (data_partioned) 
                {
                    mpeg4GetHeader(1, Bits);    // interlaced
                    DEBUG_MP4("Don't support data partitioning!!!\n");
                    return 0;
                }

                if (vol_ver_id != 1)
                {
                    if (mpeg4GetHeader(1, Bits))    // newpred_enable
                    {
                        DEBUG_MP4("Don't support newpred_enable!!!\n");
                        return 0;
                        //FlushHeaderBits(2,Bits);    // requested_upstream_message_type
                        //FlushHeaderBits(1,Bits);    // newpred_segment_type
                    }
                    if (mpeg4GetHeader(1, Bits))    // reduced_resolution_vop_enable
                    {
                        DEBUG_MP4("Don't support reduced_resolution_vop_enable!!!\n");
                        return 0;
                    }
                }
                
                if (mpeg4GetHeader(1,Bits))         // scalability
                {
                    DEBUG_MP4("Don't support scalability!!!\n");
                    return 0;
                }
            }
            else    // shape == BINARY_ONLY
            {
                DEBUG_MP4("Don't support binary only layer!!!\n");
                return 0;
            }
            return 1;  //Decoding VOL header is sucessful !
        }
        else if (start_code == GRPOFVOP_START_CODE)
        {
            FlushHeaderBits(32,Bits);          //group_vop_start_codes
            {
                int hours, minutes, seconds;
                hours   = hours;    /* avoid warning*/
                minutes = minutes;  /* avoid warning*/
                seconds = seconds;  /* avoid warning*/
                hours   = mpeg4GetHeader(5,Bits);      //time_code 18bits
                minutes = mpeg4GetHeader(6,Bits);
                mpeg4GetHeader(1,Bits); //READ_MARKER();
                seconds = mpeg4GetHeader(6,Bits);
            }
            FlushHeaderBits(1, Bits);   // closed_gov
            FlushHeaderBits(1, Bits);   // broken_link
        }
        else if (start_code == VOP_START_CODE)
        {
            FlushHeaderBits(32,Bits);                           // vop_start_code
            pMp4Dec_opt->PictureType = mpeg4GetHeader(2,Bits);    // vop_coding_type
            modulo_time_base = 0;
            while (mpeg4GetHeader(1,Bits) == 1) // modulo time base
            {
                modulo_time_base++;
            }
            mpeg4GetHeader(1,Bits); // marker bit

            pMp4Dec_opt->time_inc = mpeg4GetHeader(pMp4Dec_opt->fix_vop_timeincr_length, Bits); // vop_time_increment (1-16 bits)

            mpeg4GetHeader(1,Bits); // marker bit         

            if (!mpeg4GetHeader(1, Bits))   // vop_coded
            {
                return MPEG4_N_VOP;   //N_VOP;
            }

            if (pMp4Dec_opt->PictureType == P_VOP) {
                pMp4Dec_opt->RTYPE    = mpeg4GetHeader(1, Bits);
                //DEBUG_MP4("P");
            } else {
                pMp4Dec_opt->RTYPE    = 0;
                //DEBUG_MP4("I");
            }

            // intra_dc_vlc_threshold
            pMp4Dec_opt->intra_dc_vlc_thr = mpeg4GetHeader(3, Bits);

            pMp4Dec_opt->InitQP           = mpeg4GetHeader(5, Bits); // vop quant

            if (pMp4Dec_opt->PictureType != I_VOP) 
            {
                pMp4Dec_opt->fcode_for    = mpeg4GetHeader(3, Bits); 
                if(pMp4Dec_opt->fcode_for > 4)
                    DEBUG_MP4("Warning Search Range > 128 ! \n");
            }
            else
                pMp4Dec_opt->fcode_for    = 1;
            return 1;  //Decoding Frame header is sucessful !
        }
        else if (start_code == USERDATA_START_CODE)
        {
            //DEBUG_MP4("have user_data, ignore it!\n");
            FlushHeaderBits(32,Bits);        // user_data_start_code
        }
        else  // start_code == ?
        {
            if (ShowHeaderBits(24,Bits) == 0x000001)
            {
                DEBUG_MP4("*** WARNING: unknown start_code 0x%08X\n", ShowHeaderBits(32, Bits));
            }           
            FlushHeaderBits(8, Bits); 
        }
        
    }while(Bits->Read_bytecnt < BitsLen);

    DEBUG_MP4("*** WARNING: no vop_start_code found\n");

    return 0; /* ignore it */
}

u32 mpeg4DecodeVolHeader(MP4_Option *pMp4Dec_opt,u8* pVopBit, u32 BitsLen)  /* Peter: 0711 */
{
    MP4Dec_Bits Bitstream;
    u32         err;
    
    Bitstream.rdptr         = pVopBit;
    Bitstream.Read_bytecnt  = 0;
    Bitstream.Bigbitpos     = 0;
    
    err = ParseMPEG4Header(pMp4Dec_opt,&Bitstream, BitsLen);
    if(!err) {
        DEBUG_MP4("mpeg4DecodeVolHeader error!!!\n");  /* Peter 070108 */
    }
        
    return  err;
}

u32 mpeg4DecodeVOP(u8* pVopBit, u32 BitsLen, u8 write2dispbuf_en, u8 SmallSearch)
{
    MP4Dec_Bits Bitstream;
    u32         err;
    
    Bitstream.rdptr         = pVopBit;
    Bitstream.Read_bytecnt  = 0;
    Bitstream.Bigbitpos     = 0;

    if(SmallSearch)
       err = ParseMPEG4Header(&Mp4Dec_opt,&Bitstream, 256); //Lucian 2012/4/26
    else
       err = ParseMPEG4Header(&Mp4Dec_opt,&Bitstream, BitsLen);
    
    if(!err) {
        DEBUG_MP4("ParseMPEG4Header error!!!\n");
        VideoPictureIndex++;
        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
        MPEG4_Error             = 1;
        return  0;
    } else if(err == MPEG4_N_VOP) {
        //DEBUG_MP4("D");
        NVOPCnt++;//Lsk 090407
        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
        return  err;
    }
    err = mpeg4Decoding1Frame(&Bitstream, BitsLen,write2dispbuf_en);
    return  err;
}

s32 mpeg4Decoding1Frame(MP4Dec_Bits* Bits, u32 BitsLen,u8 write2dispbuf_en)
{
    u8  err;
    u32 mbNoSize;   /*CY 0907*/
    u32 pictWidth;  /*CY 0907*/
    u32 MbWidth, MbHeight, MbNum;
    u32 VideoOffset;
    
    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif

    // 0x0100
    MbWidth         = (Mp4Dec_opt.Width  + 15) >> 4;
    MbHeight        = (Mp4Dec_opt.Height + 15) >> 4;
    MbNum           = MbWidth * MbHeight;
    Mpeg4FrameSize  = MbWidth | (MbHeight << 8) | (MbNum << 16);

    // 0x0104
    Mpeg4MbParam    = (Mp4Dec_opt.InitQP & 0x01f) | (Mp4Dec_opt.PictureType << 16) | (Mp4Dec_opt.RTYPE << 8); 

    // 0x0200
    Mpeg4ErrResil   = (Mp4Dec_opt.resync_marker_disable) << 16;  //Error Resilience;

    // 0x0204
    mbNoSize   = FindMSB(MbNum - 1);                        /*CY 0907*/
    Mpeg4DecVidPkt  = mbNoSize | ((Mp4Dec_opt.fix_vop_timeincr_length - 1) << 8);   /*CY 0907*/

    // 0x0400
    Mpeg4IntEna     = 0x06;
    
    // 0x0500
    // bitstreaming start address
    Mpeg4StreamAddr = (u32)Bits->rdptr;
    

    // 0x0504
    Mpeg4StreamStartBit = Bits->Bigbitpos; 

    // 0x0510
    Mpeg4VopParam       = Mp4Dec_opt.fcode_for | (Mp4Dec_opt.intra_dc_vlc_thr << 8);
    
    // 0x0514
    Mpeg4DecStreamSize  = BitsLen - Bits->Read_bytecnt;

    // 0x0600
    // pre-set all MV to 0xff
    memset(mpeg4MVBuf, 0xff, MPEG4_MVBUF); 
    Mpeg4MvBufAddr = (u32) &(mpeg4MVBuf[0]);

    //0x0604  // 0x0608
    // set display buffer
    if(write2dispbuf_en)
    {
        if(sysTVOutOnFlag && (TvOutMode==SYS_TV_OUT_PAL) && (Mp4Dec_opt.Height == 480) ) //PAL mode  //Lsk 090518 : put video frame in center
        {
        	if(IsuIndex < DISPLAY_BUF_NUM)
        	{
        	#if 0
        		u32 i;
        		u32 *addr;
     			addr = (u32 *)(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] ); 
               	for(i=0 ; i<61440 ; i+=4)
               	{
                 	*addr = 0x80800000;
     	            addr++;
         	   	}
     
     			addr = (u32 *)(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + 675840); 
               	for(i=0 ; i<61440 ; i+=4)
               	{
                 	*addr = 0x80800000;
     	            addr++;
         	   	}
            #else
         	    memset_hw_Word(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM], 0x80800000, 61440);
         	    memset_hw_Word(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + 675840, 0x80800000, 61440);
            #endif
        	}
        	//Mpeg4CurrRawYAddr = (u32)(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + 61440);  // 640*(576-480)/2 = 61440    6720 no scale 20140110
        	Mpeg4CurrRawYAddr = (u32)(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM]);  // 20140110

        }
        else if (sysTVOutOnFlag && (TvOutMode==SYS_TV_OUT_PAL) && (Mp4Dec_opt.Height == 240) ) //PAL mode  //Lsk 090518 : put video frame in center
        {
        	if(IsuIndex < DISPLAY_BUF_NUM)
        	{
        	#if 0
        		u32 i;
        		u32 *addr;
     			addr = (u32 *)(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] ); 
               	for(i=0 ; i<15360 ; i+=4)
               	{
                 	*addr = 0x80800000;
     	            addr++;
         	   	}
     
     			addr = (u32 *)(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + 168960); 
               	for(i=0 ; i<15360 ; i+=4)
               	{
                 	*addr = 0x80800000;
     	            addr++;
         	   	}
            #else
         	    memset_hw_Word(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM], 0x80800000, 15360);
         	    memset_hw_Word(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + 168960, 0x80800000, 15360);
            #endif
        	}
        	Mpeg4CurrRawYAddr = (u32)(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + 15360);  // 640*(576-480)/2 = 61440
        }
 	    else
        {  
            if(Mp4Dec_opt.Height > RF_RX_2DISP_HEIGHT)
            {
                VideoOffset = RF_RX_2DISP_WIDTH * ((RF_RX_2DISP_HEIGHT - (Mp4Dec_opt.Height / 2)) / 2) * 2;
	        	if(IsuIndex < DISPLAY_BUF_NUM)
	        	{
	         	    memset_hw_Word(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM], 0x80800000, VideoOffset);
	         	    memset_hw_Word(MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + VideoOffset + Mp4Dec_opt.Width * Mp4Dec_opt.Height / 2 , 0x80800000, VideoOffset);
	        	}
            }
            else
            {
                VideoOffset = 0;
            }
            if(splitmenu==1)//playbacklist spite 
            {
                switch(filecon)
                {
                    case 0:
                        Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM]+ VideoOffset;
                        break;

                    case 1:
                        Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + 640 + VideoOffset;
                        break;

        		    case 2:
                        Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + RF_RX_DEC_WIDTH_spite*RF_RX_DEC_HEIGHT_spite+ VideoOffset;
        			    break;

        		    case 3:
                        Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + RF_RX_DEC_WIDTH_spite*RF_RX_DEC_HEIGHT_spite+640+ VideoOffset;
        			    break;	 
                }
            }
            else
 	   	        Mpeg4CurrRawYAddr   = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + VideoOffset;
        }
    }
    else
    {
        Mpeg4CurrRawYAddr = (u32) mpeg4outputbuf[VideoPictureIndex % 3];
    }        

#if 1
        mpeg4RefBuf_Y   = mpeg4PRefBuf_Y;
        mpeg4RefBuf_Cb  = mpeg4PRefBuf_Cb;
        mpeg4RefBuf_Cr  = mpeg4PRefBuf_Cr;
        
        mpeg4McoBuf_Y   = mpeg4NRefBuf_Y;
        mpeg4McoBuf_Cb  = mpeg4NRefBuf_Cb;
        mpeg4McoBuf_Cr  = mpeg4NRefBuf_Cr;
        
       //Exchange address//
        mpeg4PRefBuf_Y  = mpeg4McoBuf_Y;
        mpeg4PRefBuf_Cb = mpeg4McoBuf_Cb;
        mpeg4PRefBuf_Cr = mpeg4McoBuf_Cr;
        
        mpeg4NRefBuf_Y = mpeg4RefBuf_Y;
        mpeg4NRefBuf_Cb= mpeg4RefBuf_Cb;
        mpeg4NRefBuf_Cr= mpeg4RefBuf_Cr;
#else
    if (VideoPictureIndex & 0x01)
    {
        mpeg4RefBuf_Y   = mpeg4NRefBuf_Y;
        mpeg4RefBuf_Cb  = mpeg4NRefBuf_Cb;
        mpeg4RefBuf_Cr  = mpeg4NRefBuf_Cr;
        mpeg4McoBuf_Y   = mpeg4PRefBuf_Y;
        mpeg4McoBuf_Cb  = mpeg4PRefBuf_Cb;
        mpeg4McoBuf_Cr  = mpeg4PRefBuf_Cr;
    }
    else
    {
        mpeg4RefBuf_Y   = mpeg4PRefBuf_Y;
        mpeg4RefBuf_Cb  = mpeg4PRefBuf_Cb;
        mpeg4RefBuf_Cr  = mpeg4PRefBuf_Cr;
        mpeg4McoBuf_Y   = mpeg4NRefBuf_Y;
        mpeg4McoBuf_Cb  = mpeg4NRefBuf_Cb;
        mpeg4McoBuf_Cr  = mpeg4NRefBuf_Cr;
    } 
#endif
    /*CY 0907*/
    pictWidth = Mp4Dec_opt.Width;
    // 0x0610
    Mpeg4CurrRecInYAddr     = (u32) &(mpeg4McoBuf_Y[(pictWidth+32)*16 + 16]);   
    // 0x0614
    Mpeg4CurrRecInCbAddr    = (u32) &(mpeg4McoBuf_Cb[((pictWidth/2)+16)*8 + 8]);
    // 0x0618
    Mpeg4CurrRecInCrAddr    = (u32) &(mpeg4McoBuf_Cr[((pictWidth/2)+16)*8 + 8]);
    // 0x061C
    Mpeg4CurrRecOutYAddr    = (u32) &(mpeg4McoBuf_Y[0]);
    // 0x0620
    Mpeg4CurrRecOutCbAddr   = (u32) &(mpeg4McoBuf_Cb[0]);
    // 0x0624
    Mpeg4CurrRecOutCrAddr   = (u32) &(mpeg4McoBuf_Cr[0]);
    // 0x0628
    Mpeg4PrevRecInYAddr     = (u32) &(mpeg4RefBuf_Y[(pictWidth+32)*16 + 16]);
    // 0x062C
    Mpeg4PrevRecInCbAddr    = (u32) &(mpeg4RefBuf_Cb[((pictWidth/2)+16)*8 + 8]);
    // 0x0630
    Mpeg4PrevRecInCrAddr    = (u32) &(mpeg4RefBuf_Cr[((pictWidth/2)+16)*8 + 8]);
    // 0x0634
    Mpeg4PrevRecOutYAddr    = (u32) &(mpeg4RefBuf_Y[0]);
    // 0x0638
    Mpeg4PrevRecOutCbAddr   = (u32) &(mpeg4RefBuf_Cb[0]);
    // 0x063C
    Mpeg4PrevRecOutCrAddr   = (u32) &(mpeg4RefBuf_Cr[0]);   


    // 0x0000
    mpegflag    = 1;
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)|| (CHIP_OPTION == CHIP_A1026A))
    if(splitmenu==1) //playbacklist spite  
    {
        if(Mp4Dec_opt.Width >= 640)
        {
           Mpeg4SourceStride   = RF_RX_DEC_WIDTH_spite;
        #if MPEG4_ROT270_SUPPORT
           Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_ROTDS_ENA | MPEG4_ROT_ENA | MPEG4_ROT_270;
        #elif MPEG4_ROT90_SUPPORT
           Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_ROTDS_ENA | MPEG4_ROT_ENA | MPEG4_ROT_90;
        #else
           Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
        #endif
        }
        else
        {
           Mpeg4SourceStride   = RF_RX_DEC_WIDTH_spite;
        #if MPEG4_ROT270_SUPPORT
           Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_ROTDS_DISA | MPEG4_ROT_ENA | MPEG4_ROT_270;
        #elif MPEG4_ROT90_SUPPORT
           Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_ROTDS_DISA | MPEG4_ROT_ENA | MPEG4_ROT_90;
        #else
           Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_DISA;
        #endif
        }
    }
    else
    {
        if((Mp4Dec_opt.Width > 720) || (Mp4Dec_opt.Height > 576))   // decoder down sample output
        {
            Mpeg4SourceStride   = Mp4Dec_opt.Width / 2;
        #if MPEG4_ROT270_SUPPORT
            Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_ROTDS_ENA | MPEG4_ROT_ENA | MPEG4_ROT_270;
        #elif MPEG4_ROT90_SUPPORT
            Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_ROTDS_ENA | MPEG4_ROT_ENA | MPEG4_ROT_90;
        #else
            Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
        #endif
        } 
        else 
        {
            if((splitmenu ==1) ||(splitmenu ==2))
                Mpeg4SourceStride   = RF_RX_DEC_WIDTH_spite;
			else if((Mp4Dec_opt.Width == 320) && (Mp4Dec_opt.Height == 240)) //QVGA
				Mpeg4SourceStride   = Mp4Dec_opt.Width*2;
        	else
				Mpeg4SourceStride   = Mp4Dec_opt.Width;

        #if MPEG4_ROT270_SUPPORT
            Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_ROTDS_DISA | MPEG4_ROT_ENA | MPEG4_ROT_270;
        #elif MPEG4_ROT90_SUPPORT
            Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_ROTDS_DISA | MPEG4_ROT_ENA | MPEG4_ROT_90;
        #else
            Mpeg4Ctrl           = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
        #endif
        }
    }
#else
   #if MPEG4_ROT270_SUPPORT
      Mpeg4Ctrl   = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_ROTDS_DISA | MPEG4_ROT_ENA | MPEG4_ROT_270;
   #elif MPEG4_ROT90_SUPPORT
      Mpeg4Ctrl   = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_ROTDS_DISA | MPEG4_ROT_ENA | MPEG4_ROT_90;
   #else
      Mpeg4Ctrl   = (Mp4Dec_opt.Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
   #endif
#endif

#if MPEG_DEBUG_ENA_LUCIAN
    gpioSetLevel(0, 1, 1);
#endif



    OSSemPend(VideoCpleSemEvt, MPEG4_TIMEOUT, &err);
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif
    if (err != OS_NO_ERR)
    {
        // reset MPEG-4 hardware
        SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
        SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100;
        
        DEBUG_MP4("Decoder Error: VideoCpleSemEvt is %d.\n", err);
        DEBUG_MP4("VideoPictureIndex = %d\n", VideoPictureIndex);
        DEBUG_MP4("VideoBufMngReadIdx = %d\n", VideoBufMngReadIdx);
        DEBUG_MP4("VideoBufMngWriteIdx = %d\n", VideoBufMngWriteIdx);
        DEBUG_MP4("mp4_avifrmcnt = %d\n", mp4_avifrmcnt);
        DEBUG_MP4("mpeg4: VideoTrgSemEvt = %d\n", VideoTrgSemEvt->OSEventCnt);
        DEBUG_MP4("mpeg4: VideoCmpSemEvt = %d\n", VideoCmpSemEvt->OSEventCnt);
        mpegflag = 0;
        VideoPictureIndex++;
        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
        MPEG4_Error             = 1;
    }else
        MPEG4_Error             = 0;
    
    if(MPEG4_Status & 0x04) {
        DEBUG_MP4("MPEG-4 decode frame error!!!\n");
        DEBUG_MP4("VideoPictureIndex = %d\n", VideoPictureIndex);
        DEBUG_MP4("MPEG4_Status = %d\n", MPEG4_Status);
        MPEG4_Error             = 1;
    }

    OSSemPost(mpeg4ReadySemEvt);    // release MPEG4 HW
    if(MPEG4_Error)
        return  0;
    else
        return  1;   
}
/*Peter 0707 E*/

#if CDVR_LOG
void ChangeLogFileStartAddress(void)
{
    u8          err;

    //DEBUG_MP4("ChangeLogFileStartAddress()\n", err);
    
    // 讓Log檔與Video檔時間同步
    OSSemPend(LogFileSemEvt, 10, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_MP4("Encoder Error: LogFileSemEvt is %d.\n", err);
    }
#if (HW_BOARD_OPTION==ELEGANT_KFCDVR)
    {   // Record distance
        static u32  DistanceOld = 0;
        u32         Distance, Distance1;
        u8          szDistance[MAX_OVERLAYSTR];
        s32         StrLen;
        Distance1   = (u32)((s64)SpeedCounter1 * 256 * 1000 / (SpeedRPM * Speed_Pulse));
        Distance    = Distance1 - DistanceOld;
        DistanceOld = Distance1;
        sprintf (szDistance, "Distance: %7dm    %s", Distance, szVersion);
        strcpy(szLogFile, szDistance);
        StrLen      = strlen(szDistance);
        strcpy(szLogFile + StrLen, "\r\n");
        szLogFile  += StrLen + 2;
    }
#endif
    pLogFileEnd         = szLogFile;
    LogFileNextStart    = LogFileCurrent;
    LogFileCurrent      = (LogFileCurrent + 1) % LOG_INDEX_NUM;
    szLogFile           = (u8*)(((u32)szLogFile + 15) & ~15);   // fix 16 bytes boundry DMA bug
#if 0
    {
        static int      offset;
        offset++;
        if((offset & 3) == 0)
            offset++;
        offset         &= 15;
        szLogFile      += offset;        // 故意讓他發生跨 16 bytes boundry
    }
#endif
    if(szLogFile < (LogFileBufEnd - MAX_OVERLAYSTR * 2))
    {
        LogFileIndex[LogFileNextStart]  = szLogFile;
    }
    else
    {
        pLogFileMid                     = szLogFile;
        szLogFile                       = LogFileBuf;
        LogFileIndex[LogFileNextStart]  = LogFileBuf;
    }
    OSSemPost(LogFileSemEvt);
}
#endif


//-------------------RF-RX use -------------------------//
s32 rfiuMpeg4EncodeVolHeader(u8* pHeader, u32* pHeaderSize, u32 Width, u32 Height,int Use_MPEG_Q)    
{
    u8 i;
   
    u8 VolHeader_MPEG_Q[0x13] =    
    { 
        0x00, 0x00, 0x01, 0x00,  
        0x00, 0x00, 0x01, 0x20,
        0x00, 0xc4, 0x88, 0xba,        
        0x98, 0x50, 0x00, 0x40,
        0x01, 0x49, 0x0f
    };

    
    u8 VolHeader_263_Q[0x13] =    
    { 
        0x00, 0x00, 0x01, 0x00,  
        0x00, 0x00, 0x01, 0x20,
        0x00, 0xc4, 0x88, 0xba,        
        0x98, 0x50, 0x00, 0x40,
        0x01, 0x44, 0x3f
    };

    
    VolHeader_263_Q[0x0d] |= (u8)(Width >> 9); 
    VolHeader_263_Q[0x0e] |= (u8)(Width >> 1);     
    VolHeader_263_Q[0x0f] |= (u8)(Width << 7); 
    
    VolHeader_263_Q[0x0f] |= (u8)(Height >> 7);
    VolHeader_263_Q[0x10] |= (u8)(Height << 1);


    VolHeader_MPEG_Q[0x0d] |= (u8)(Width >> 9); 
    VolHeader_MPEG_Q[0x0e] |= (u8)(Width >> 1);     
    VolHeader_MPEG_Q[0x0f] |= (u8)(Width << 7); 
    
    VolHeader_MPEG_Q[0x0f] |= (u8)(Height >> 7);
    VolHeader_MPEG_Q[0x10] |= (u8)(Height << 1);

    if(Use_MPEG_Q)
    {
        *pHeaderSize = sizeof(VolHeader_MPEG_Q);
        for (i = 0; i < *pHeaderSize; i++)
            *pHeader++ = VolHeader_MPEG_Q[i];  
    }
    else
    {
        *pHeaderSize = sizeof(VolHeader_263_Q);
        for (i = 0; i < *pHeaderSize; i++)
            *pHeader++ = VolHeader_263_Q[i];  
    }
    
    return 1;
}


u32 rfiuMpeg4DecodeVOP(MP4_Option *pMp4Dec_opt, 
                      u8* pVopBit, 
                      u32 BitsLen, 
                      int RFUnit,
                      unsigned int Offset,
                      int DispMode,
                      int FieldDecEn)
{
    MP4Dec_Bits Bitstream;
    u32         err;
    u8 *BotFieldBits;
	
    //-------------------------//
    //DEBUG_MP4("offset1=%x\n",DispMode);    
    Bitstream.rdptr         = pVopBit;
    Bitstream.Read_bytecnt  = 0;
    Bitstream.Bigbitpos     = 0;

	#if VIDEO_STARTCODE_DEBUG_ENA
	if((*(pVopBit) == 0x00) && (*(pVopBit+1) == 0x00) 
        && (*(pVopBit+2) == 0x01) && (*(pVopBit+3) == 0xB6))
	{
	}
	else
	{
		DEBUG_ASF("Warning!!! LiveShow MPEG4 start code error - %d\n", VideoClipOption[RFUnit].VideoCmpSemEvt->OSEventCnt);
	}
	#endif

    err = ParseMPEG4Header(pMp4Dec_opt,&Bitstream, 256); //Lucian 2012/4/26
        
    if(!err) 
	{
        DEBUG_MP4("ParseMPEG4Header error!!!\n");
        pMp4Dec_opt->VideoPictureIndex++;
        //DEBUG_MP4("\n======== Mpeg4 Decoding Fatal Error! Reboot!======\n");
        //sysForceWDTtoReboot();
        DEBUG_MP4("\n======== Mpeg4 Decoding Error! Resync:%d!======\n",RFUnit);
        if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==0)
           sysback_RF_SetEvt(SYS_BACKRF_RFI_FORCERESYNC, RFUnit);
        return  0;
    } 
	else if(err == MPEG4_N_VOP) 
	{
        return  err;
    }

    BotFieldBits=pVopBit + Offset;
    err = rfiuMpeg4Decoding1Frame(pMp4Dec_opt,&Bitstream, 
                                  BitsLen,RFUnit,
                                  DispMode,BotFieldBits,
                                  FieldDecEn);

    if(!err)
    {
        //DEBUG_MP4("\n======== Mpeg4 Decoding Fatal Error! Reboot!======\n");
        //sysForceWDTtoReboot();
        DEBUG_MP4("\n======== Mpeg4 Decoding Error! Resync:%d!======\n",RFUnit);
        if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==0)
            sysback_RF_SetEvt(SYS_BACKRF_RFI_FORCERESYNC, RFUnit);
    }
    
    return  err;
}



s32 rfiuMpeg4Decoding1Frame(MP4_Option *pMp4Dec_opt,MP4Dec_Bits* Bits, 
	                                 u32 BitsLen,int RFUnit,
	                                 int DispMode,u8 *BotFieldBits,
	                                 int FieldDecEn)
{
    u8  err;
    u32 mbNoSize;   
    u32 pictWidth;  
    u32 MbWidth, MbHeight, MbNum;
	int i,Error;
	u8  *RefBuf_Y, *RefBuf_Cb, *RefBuf_Cr, *McoBuf_Y, *McoBuf_Cb, *McoBuf_Cr;
    u32 VideoOffset=0;
    int DecPos;
    static int MpegErrCnt=0;
    
    //------------------//
    Error=0;
    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif
    SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
    for(i=0;i<5;i++); //delay
    SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);

    // 0x0100
    if(FieldDecEn)
    {
       MbWidth         = (pMp4Dec_opt->Width  + 15) >> 4;
       MbHeight        = (pMp4Dec_opt->Height/2 + 15) >> 4;
       MbNum           = MbWidth * MbHeight;
    }
    else
    {
       MbWidth         = (pMp4Dec_opt->Width  + 15) >> 4;
       MbHeight        = (pMp4Dec_opt->Height + 15) >> 4;
       MbNum           = MbWidth * MbHeight;
    }
    Mpeg4FrameSize  = MbWidth | (MbHeight << 8) | (MbNum << 16);

    // 0x0104
    Mpeg4MbParam    = (pMp4Dec_opt->InitQP & 0x01f) | (pMp4Dec_opt->PictureType << 16) | (pMp4Dec_opt->RTYPE << 8); 

    // 0x0200
    Mpeg4ErrResil   = (pMp4Dec_opt->resync_marker_disable) << 16;  //Error Resilience;

    // 0x0204
    mbNoSize   = FindMSB(MbNum - 1);                        /*CY 0907*/
    Mpeg4DecVidPkt  = mbNoSize | ((pMp4Dec_opt->fix_vop_timeincr_length - 1) << 8);   /*CY 0907*/

    // 0x0400
    Mpeg4IntEna     = 0x06;
    
    // 0x0500
    // bitstreaming start address
    Mpeg4StreamAddr = (u32)Bits->rdptr;
    

    // 0x0504
    Mpeg4StreamStartBit = Bits->Bigbitpos; 

    // 0x0510
    Mpeg4VopParam       = pMp4Dec_opt->fcode_for | (pMp4Dec_opt->intra_dc_vlc_thr << 8);
    
    // 0x0514
    Mpeg4DecStreamSize  = BitsLen - Bits->Read_bytecnt;

    // 0x0600
    // pre-set all MV to 0xff
    memset(rfiuRxDecBuf[RFUnit].mpeg4MVBuf, 0xff, MPEG4_MVBUF); 
    Mpeg4MvBufAddr = (u32) &(rfiuRxDecBuf[RFUnit].mpeg4MVBuf[0]);

    //0x0604  // 0x0608   
    if(DispMode == RFIU_RX_DISP_MAIN) 
    {
       if(pMp4Dec_opt->Height >= 720) //For HD
       {
       #if RFRX_FULLSCR_HD_SINGLE
         VideoOffset=0;//RF_RX_2DISP_WIDTH*((RF_RX_2DISP_HEIGHT-(pMp4Dec_opt->Height/2))/2)*2;
       #else
         VideoOffset=RF_RX_2DISP_WIDTH*((RF_RX_2DISP_HEIGHT-(pMp4Dec_opt->Height/2))/2)*2;
       #endif
       }
       else if(pMp4Dec_opt->Height <= 400) //For QVGA
       {
         VideoOffset= 0;//(RF_RX_2DISP_WIDTH-pMp4Dec_opt->Width)/2*2 + RF_RX_2DISP_WIDTH*((RF_RX_2DISP_HEIGHT-pMp4Dec_opt->Height)/2)*2;
       }
       else //For VGA
       {
         VideoOffset=0;
       }
    #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
       if(sysEnZoom)
          Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + 0;
       else
          Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset;
    #else
       Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset;
    #endif
    }
    else if(DispMode == RFIU_RX_DISP_MASK)
    {
       if(pMp4Dec_opt->Height  >= 720)
       {
         VideoOffset= 800*((RF_RX_2DISP_HEIGHT-(pMp4Dec_opt->Height/2))/2)*2;
       }
       else if(pMp4Dec_opt->Height <= 400) //For QVGA
       {
         VideoOffset= (RF_RX_2DISP_WIDTH-pMp4Dec_opt->Width)/2*2 + 800*((RF_RX_2DISP_HEIGHT-pMp4Dec_opt->Height)/2)*2;
       }
       else //For VGA
       {
         VideoOffset=0;
       }

       Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + VideoOffset;

    }
	else if(DispMode == RFIU_RX_DISP_QUARD)
    {
       if(pMp4Dec_opt->Height >= 720)//For HD
       {
          VideoOffset= RF_RX_2DISP_WIDTH*2*2*((RF_RX_2DISP_HEIGHT-(pMp4Dec_opt->Height/2))/2);
       }
       else if(pMp4Dec_opt->Height <= 400) //For QVGA
       {
       #if UI_GRAPH_QVGA_ENABLE
          VideoOffset=0;
       #else
          if(rfiuRX_OpMode & RFIU_RX_OPMODE_P2P)
             VideoOffset=0;
          else
             VideoOffset=(RF_RX_2DISP_WIDTH-pMp4Dec_opt->Width)/2*2 + RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT-pMp4Dec_opt->Height)/2)*2;
       #endif
       }
       else //For VGA
       {
          VideoOffset=0;
       }
   #if RFRX_HALF_MODE_SUPPORT      
       if(rfiuRX_CamOnOff_Num <= 2)
       {
           if(rfiuRX_CamOnOff_Num==1)
           {
           #if UI_GRAPH_QVGA_ENABLE
               if(sysTVOutOnFlag)
                  Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT_TV/2);
               else
                  Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
           #else
               if( (pMp4Dec_opt->Height <= 400) && (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) ) //For QVGA
               {
                    if(sysTVOutOnFlag)
                       Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT_TV/2);
                    else
                       Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
               }
               else
               {
                    if(sysTVOutOnFlag)
                       Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT_TV;
                    else
                       Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT;
               }
           #endif     
           }
           else
           {
               if(rfiuRX_CamOnOff_Sta==0x03)
               {
                   if(RFUnit==0)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else if(rfiuRX_CamOnOff_Sta==0x05)
               {
                   if(RFUnit==0)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else if(rfiuRX_CamOnOff_Sta==0x09)
               {
                   if(RFUnit==0)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else if(rfiuRX_CamOnOff_Sta==0x06)
               {
                   if(RFUnit==1)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else if(rfiuRX_CamOnOff_Sta==0x0a)
               {
                   if(RFUnit==1)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else if(rfiuRX_CamOnOff_Sta==0x0c)
               {
                   if(RFUnit==2)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else
               {
                   DecPos=0;
               } 
               
            #if UI_GRAPH_QVGA_ENABLE
               switch(DecPos)
               {
                  case 0:
                     if(sysTVOutOnFlag)
                        Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT_TV/2);
                     else
                        Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
        			 break;

        		  case 1:
                     if(sysTVOutOnFlag)
                         Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2/2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT_TV/2);
                     else
                         Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2/2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
        			 break;
               }
            #else
               if( (pMp4Dec_opt->Height <= 400) && (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) ) //For QVGA
               {
                   switch(DecPos)
                   {
                      case 0:
                         if(sysTVOutOnFlag)
                            Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT_TV/2);
                         else
                            Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
            			 break;

            		  case 1:
                         if(sysTVOutOnFlag)
                            Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2/2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT_TV/2);
                         else
                            Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2/2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
            			 break;
                   }
               }
               else
               {
                   switch(DecPos)
                   {
                      case 0:
                         if(sysTVOutOnFlag)
                             Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT_TV;
                         else
                             Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT;
            			 break;

            		  case 1:
                         if(sysTVOutOnFlag)
                             Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT_TV;
                         else
                             Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT;
            			 break;
                   }
               }
            #endif
            }
       }
       else
    #endif
       {
       #if UI_GRAPH_QVGA_ENABLE
           switch(RFUnit)
           {
              case 0:
                 Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset;
    			 break;

    		  case 1:
                 Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + VideoOffset;
    			 break;

    		  case 2:
                 Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
    			 break;

    		  case 3:
                 Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
    			 break;	 
           }
           
       #else
           if( (pMp4Dec_opt->Height <= 400) && (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) ) //For QVGA
           {
               switch(RFUnit)
               {
                  case 0:
                     Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset;
        			 break;

        		  case 1:
                     Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + VideoOffset;
        			 break;

        		  case 2:
                     Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
        			 break;

        		  case 3:
                     Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
        			 break;	 
               }
           }
           else
           {
               switch(RFUnit)
               {
                  case 0:
                     Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0]+ VideoOffset;
        			 break;

        		  case 1:
                     Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2 + VideoOffset;
        			 break;

        		  case 2:
                     Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
        			 break;

        		  case 3:
                     Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2 + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
        			 break;	 
               }
           }
       #endif
       }
	}
	else if(DispMode == RFIU_RX_DISP_SUB1)
	{
   	   Mpeg4CurrRawYAddr = (u32)Sub1Videodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM];
	}
          
    RefBuf_Y   = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y;
    RefBuf_Cb  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cb;
    RefBuf_Cr  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr;
	
    McoBuf_Y   = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
    McoBuf_Cb  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
    McoBuf_Cr  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
 
    //Exchange address//
	rfiuRxDecBuf[0].mpeg4NRefBuf_Y =RefBuf_Y;
    rfiuRxDecBuf[0].mpeg4NRefBuf_Cb=RefBuf_Cb;
    rfiuRxDecBuf[0].mpeg4NRefBuf_Cr=RefBuf_Cr;

	rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y =McoBuf_Y;
    rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cb=McoBuf_Cb;
    rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr=McoBuf_Cr;

    /*CY 0907*/
    pictWidth = pMp4Dec_opt->Width;
    // 0x0610
    Mpeg4CurrRecInYAddr     = (u32) &(McoBuf_Y[(pictWidth+32)*16 + 16]);   
    // 0x0614
    Mpeg4CurrRecInCbAddr    = (u32) &(McoBuf_Cb[((pictWidth/2)+16)*8 + 8]);
    // 0x0618
    Mpeg4CurrRecInCrAddr    = (u32) &(McoBuf_Cr[((pictWidth/2)+16)*8 + 8]);
    // 0x061C
    Mpeg4CurrRecOutYAddr    = (u32) &(McoBuf_Y[0]);
    // 0x0620
    Mpeg4CurrRecOutCbAddr   = (u32) &(McoBuf_Cb[0]);
    // 0x0624
    Mpeg4CurrRecOutCrAddr   = (u32) &(McoBuf_Cr[0]);
    // 0x0628
    Mpeg4PrevRecInYAddr     = (u32) &(RefBuf_Y[(pictWidth+32)*16 + 16]);
    // 0x062C
    Mpeg4PrevRecInCbAddr    = (u32) &(RefBuf_Cb[((pictWidth/2)+16)*8 + 8]);
    // 0x0630
    Mpeg4PrevRecInCrAddr    = (u32) &(RefBuf_Cr[((pictWidth/2)+16)*8 + 8]);
    // 0x0634
    Mpeg4PrevRecOutYAddr    = (u32) &(RefBuf_Y[0]);
    // 0x0638
    Mpeg4PrevRecOutCbAddr   = (u32) &(RefBuf_Cb[0]);
    // 0x063C
    Mpeg4PrevRecOutCrAddr   = (u32) &(RefBuf_Cr[0]);   

    // 0x0000
	if( (DispMode == RFIU_RX_DISP_MAIN) )
    {
    #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
        if(pMp4Dec_opt->Height >= 720)  //HD
        {
           if(sysEnZoom)
           {
              Mpeg4SourceStride   = RF_RX_2DISP_WIDTH*2;
              Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_DISA;
           }
           else
           {
              Mpeg4SourceStride   = RF_RX_2DISP_WIDTH;
              Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
           }
		}
		else
		{
		   if(sysEnZoom)
		   {
               if(FieldDecEn)
                  Mpeg4SourceStride   = RF_RX_2DISP_WIDTH*2*2;
               else
                  Mpeg4SourceStride   = RF_RX_2DISP_WIDTH*2;
           }
           else
           {
    		   if(FieldDecEn)
                  Mpeg4SourceStride   = RF_RX_2DISP_WIDTH*2;
               else
                  Mpeg4SourceStride   = RF_RX_2DISP_WIDTH;
           }
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
	    }      
    #else
        if(pMp4Dec_opt->Height >= 720)  //HD
        {
           Mpeg4SourceStride   = RF_RX_2DISP_WIDTH;
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
		}
		else
		{
		   if(FieldDecEn)
              Mpeg4SourceStride   = RF_RX_2DISP_WIDTH*2;
           else
              Mpeg4SourceStride   = RF_RX_2DISP_WIDTH;
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
	    }    
    #endif
	}
    else if( (DispMode == RFIU_RX_DISP_SUB1))
    {
    #if TV_D1_OUT_FULL
           if(FieldDecEn)
              Mpeg4SourceStride   = 704*2;
           else
              Mpeg4SourceStride   = 704;
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
    #else
		   if(FieldDecEn)
              Mpeg4SourceStride   = 640*2;
           else
              Mpeg4SourceStride   = 640;
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
    #endif
    }
	else if(DispMode == RFIU_RX_DISP_QUARD)
    {
    #if UI_GRAPH_QVGA_ENABLE
        if(pMp4Dec_opt->Height>= 400)
        {
           Mpeg4SourceStride   = RF_RX_2DISP_WIDTH*2;
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
        }
        else
        {
           Mpeg4SourceStride   = RF_RX_2DISP_WIDTH*2;
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_DISA;
        }
    #else
        if(pMp4Dec_opt->Height>= 720)
        {
           Mpeg4SourceStride   = RF_RX_2DISP_WIDTH*2;
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
        }
        else
        {
           Mpeg4SourceStride   = RF_RX_2DISP_WIDTH*2;
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_DISA;
        }
    #endif
	}
    else if(DispMode == RFIU_RX_DISP_MASK)
    {
        if(pMp4Dec_opt->Height >= 720)  //HD
        {
           Mpeg4SourceStride   = 800;
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
		}
		else
		{
		   if(FieldDecEn)
              Mpeg4SourceStride   = 800*2;
           else
              Mpeg4SourceStride   = 800;
           Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
	    }    
    }

#if MPEG_DEBUG_ENA_LUCIAN
    gpioSetLevel(0, 1, 1);
#endif


    OSSemPend(VideoCpleSemEvt, MPEG4_TIMEOUT, &err);
    

    if (err != OS_NO_ERR)
    {
        // reset MPEG-4 hardware
        SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
		for(i=0;i<10;i++); //delay
        DEBUG_MP4("Decoder Error: VideoCpleSemEvt is %d.\n", err);
        SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);
        
        pMp4Dec_opt->VideoPictureIndex ++;
        Error = 1;

        if(MpegErrCnt > 10)
        {
            DEBUG_MP4("Mpeg4 fetal error! Force to Reboot!!\n");
			sysForceWDTtoReboot();
        }
        MpegErrCnt ++;
    }
	else
	{
		pMp4Dec_opt->VideoPictureIndex ++;
        MpegErrCnt=0;
	}
    
    if(MPEG4_Status & 0x04) 
    {
        // reset MPEG-4 hardware
        SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
		for(i=0;i<10;i++); //delay
        DEBUG_MP4("MPEG-4 decode frame error!!!\n");
        SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);

        Error = 1;
    }
    //-------Decoding Bottom field------//
    if(FieldDecEn)
    {
        MbWidth         = (pMp4Dec_opt->Width  + 15) >> 4;
        MbHeight        = (pMp4Dec_opt->Height/2 + 15) >> 4;
        MbNum           = MbWidth * MbHeight;

        Mpeg4FrameSize  = MbWidth | (MbHeight << 8) | (MbNum << 16);

        // 0x0104
        Mpeg4MbParam    = (pMp4Dec_opt->InitQP & 0x01f) | (pMp4Dec_opt->PictureType << 16) | (pMp4Dec_opt->RTYPE << 8); 

        // 0x0200
        Mpeg4ErrResil   = (pMp4Dec_opt->resync_marker_disable) << 16;  //Error Resilience;

        // 0x0204
        mbNoSize   = FindMSB(MbNum - 1);                        /*CY 0907*/
        Mpeg4DecVidPkt  = mbNoSize | ((pMp4Dec_opt->fix_vop_timeincr_length - 1) << 8);   /*CY 0907*/

        // 0x0400
        Mpeg4IntEna     = 0x06;
        
        // 0x0500
        // bitstreaming start address
        Mpeg4StreamAddr = (u32)BotFieldBits;        
        // 0x0504
        Mpeg4StreamStartBit = 0; 

        // 0x0510
        Mpeg4VopParam       = pMp4Dec_opt->fcode_for | (pMp4Dec_opt->intra_dc_vlc_thr << 8);
        
        // 0x0514
        Mpeg4DecStreamSize  = BitsLen;

        // 0x0600
        // pre-set all MV to 0xff
        memset(rfiuRxDecBuf[RFUnit].mpeg4MVBuf, 0xff, MPEG4_MVBUF); 
        Mpeg4MvBufAddr = (u32) &(rfiuRxDecBuf[RFUnit].mpeg4MVBuf[0]);

        Mpeg4CurrRawYAddr += (pMp4Dec_opt->Width*2);
        //Mpeg4CurrRawYAddr = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + pMp4Dec_opt->Width*2;

        /*
        RefBuf_Y   = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y;
        RefBuf_Cb  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cb;
        RefBuf_Cr  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr;
    	
        McoBuf_Y   = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
        McoBuf_Cb  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
        McoBuf_Cr  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
     
        //Exchange address//
    	rfiuRxDecBuf[0].mpeg4NRefBuf_Y =RefBuf_Y;
        rfiuRxDecBuf[0].mpeg4NRefBuf_Cb=RefBuf_Cb;
        rfiuRxDecBuf[0].mpeg4NRefBuf_Cr=RefBuf_Cr;

    	rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y =McoBuf_Y;
        rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cb=McoBuf_Cb;
        rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr=McoBuf_Cr;
        */
        pictWidth = pMp4Dec_opt->Width;
        // 0x0610
        Mpeg4CurrRecInYAddr     += ((720 + 32) * (576 + 32));   
        // 0x0614
        Mpeg4CurrRecInCbAddr    += ((720/2 + 32) * (576/2 + 32));
        // 0x0618
        Mpeg4CurrRecInCrAddr    += ((720/2 + 32) * (576/2 + 32));
        // 0x061C
        Mpeg4CurrRecOutYAddr    += ((720 + 32) * (576 + 32));
        // 0x0620
        Mpeg4CurrRecOutCbAddr   += ((720/2 + 32) * (576/2 + 32));
        // 0x0624
        Mpeg4CurrRecOutCrAddr   += ((720/2 + 32) * (576/2 + 32));
        // 0x0628
        Mpeg4PrevRecInYAddr     += ((720 + 32) * (576 + 32));
        // 0x062C
        Mpeg4PrevRecInCbAddr    += ((720/2 + 32) * (576/2 + 32));
        // 0x0630
        Mpeg4PrevRecInCrAddr    += ((720/2 + 32) * (576/2 + 32));
        // 0x0634
        Mpeg4PrevRecOutYAddr    += ((720 + 32) * (576 + 32));
        // 0x0638
        Mpeg4PrevRecOutCbAddr   += ((720/2 + 32) * (576/2 + 32));
        // 0x063C
        Mpeg4PrevRecOutCrAddr   += ((720/2 + 32) * (576/2 + 32));   

        if(DispMode == RFIU_RX_DISP_MASK)
        {
           Mpeg4SourceStride   = 800*2;
        }
        else
        {
           Mpeg4SourceStride   = RF_RX_2DISP_WIDTH*2;
        }
        Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;

        OSSemPend(VideoCpleSemEvt, MPEG4_TIMEOUT, &err);
    
        if (err != OS_NO_ERR)
        {
            // reset MPEG-4 hardware
            SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
    		for(i=0;i<10;i++); //delay
            DEBUG_MP4("Decoder Error: VideoCpleSemEvt is %d.\n", err);
            SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);
            
            Error = 1;
        }
    	
        if(MPEG4_Status & 0x04) 
        {
            // reset MPEG-4 hardware
            SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
    		for(i=0;i<10;i++); //delay
            DEBUG_MP4("MPEG-4 decode frame error!!!\n");
            SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);

            Error = 1;
        }
    }
	
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif    

    OSSemPost(mpeg4ReadySemEvt);    // release MPEG4 HW

    if(Error)
        return  MPEG4_ERROR;
    else
        return  MPEG4_NORMAL;   
	
}

