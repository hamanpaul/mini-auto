/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    isu.c

Abstract:

    The routines of Image Scaling Unit.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "board.h"
#include "isureg.h"
#include "isuapi.h"
#include "isu.h"

#include "siuapi.h" /*BJ 0530 S*/
#include "iduapi.h"
#include "jpegapi.h"
#include "mpeg4api.h"   /* Peter 070108 */
#include "timerapi.h"
#include "iisapi.h"
#include "asfapi.h"
#include "uiapi.h"
#include "rtcapi.h"
#if (ISU_OUT_BY_FID)
 #include "../siu/inc/siureg.h"
#endif

#if (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC)
 #include "../siu/inc/siureg.h"
#endif

#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif


#if ISU_OVERLAY_ENABLE
#if ISU_SUPPORT

#define SPEEDUP 1   // 1: generate font fast, 0: generate font slow
#if SPEEDUP
    int length1;
    static char szString1[MAX_OVERLAYSTR]  = "";
#endif

#endif
#endif

#define DEF_TOP_FIELD          1
#define DEF_BOT_FIELD          0
/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
ISU_IOSize  MSFSz;  //Main scale size
ISU_CRS     MSFCrs; //Main scale step
ISU_IOSize  SSFSz;  //Sub scale size
ISU_CRS     SSFCrs; //Sub scale step
s32 isu_avifrmcnt;
volatile s32 isu_idufrmcnt=0;
u32 IsuIndex;
u8 scaler2sd = 0;
ISU_Size isuPanel, isuPacket, isuSrcImg;
ISU_Size isuOut;/*,DstImg*/
u32 isu_int_status;
u32 ISUFrameDuration[ISU_FRAME_DURATION_NUM];
u32 isuFrameTime;
u32 SkipFrameDuration=0; //Lsk 090813
u8  FieldTypeID   = 0;  // 1: odd field, 0: even field
#if ISU_OVERLAY_ENABLE
     #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE 
        u8  isuGenerateScalarOverlayImage   = 1;
     #else
        u8  isuGenerateScalarOverlayImage   = 0;
     #endif
#endif


/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
#if MOTIONDETEC_ENA
extern u8  siuFrameCount;
extern u32  MD_period_Preview;  //Lucian: 設定幾個frame 後,做一次motion detection
extern u32  MD_period_Video;
extern OS_EVENT* siuSemEvt;
#endif

extern u32 sysVideoInSel;

extern u8 VideoRecFrameRate;
extern u8 siuOpMode;
extern u8 TvOutMode;
extern u32 mpeg4Width, mpeg4Height;
extern u32 sysTVinFormat;
#if ISU_OVERLAY_ENABLE
  extern char timeForRecord1[20];
  extern u32 *ScalarOverlayImage;
#endif


extern u8 sysReady2CaptureVideo;
#if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
extern u32 unMp4SkipFrameCount;
extern u8 avi_FrameSkip; /*BJ 0530 S*/
extern s32 mp4_avifrmcnt;
extern u32 siuFrameNumInMP4;
extern u32 siuSkipFrameRate;
#endif

extern u8  IsuOutAreaFlagOnTV;

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
void isuIntProcess_MPEGAVI(void);
void isuIntProcess_PREVIEW(void);
void isuIntProcess_CAP_PREVIEW(void);


void ResetisuPanel(void);
s32 isuScalar_D2D(
                     u8 *Srcbuf , u8 *Dstbuf ,
                     u16 sizeX_src, u16 sizeY_src,
                     u16 sizeX_dst, u16 sizeY_dst
                  );
void isuRst(void);
void isuStop(void);
u16 isuSize2Step(u16, u16, u16*);
s32 isuSetIOSize(u16, u16);
#if ISU_OVERLAY_ENABLE
void isuOverlayImgConfig(void);
s32 sysDrawTimeOnVideoClip(s32 dummy);
s32 GenerateOverlayImage(u32 *YUV422Image, char *szString, int MaxStrLen, int FontW, int FontH, int VideoW);
#endif

u32 X_offset = 80*2;  //Lsk 090707 : put QVGA int center, width*2
u32 Y_offset =  0;
extern void siuGetPreviewZoomWidthHeight(s32 zoomFactor,u16 *W, u16 *H);


/*
 *********************************************************************************************************
 * Extern Function prototype
 *********************************************************************************************************
 */
/*

Routine Description:

    Initialize the Image Scaling Unit.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
#if ((ISU_OVERLAY_ENABLE) && (CHIP_OPTION < CHIP_A1013A))

s32 GenerateOverlayImage(u32 *YUV422Image, char *szString, int MaxStrLen, int FontW, int FontH, int VideoW)
{
    u8 flag;

    //DEBUG_ISU("GenerateOverlayImage(0x%08x, %s, %d, %d, %d, %d)\n", YUV422Image, szString, MaxStrLen, FontW, FontH, VideoW);

    if(VideoW >= 640)
        flag = 1;
    else
        flag = 0;
#if (ISU_OVL_TYPE == ISU_OVL_TYPE_16Bit)
    return GenerateOverlayImage16Bits(YUV422Image, szString, MaxStrLen, FontW, FontH, VideoW, flag);
#elif (ISU_OVL_TYPE == ISU_OVL_TYPE_1Bit)
    return GenerateOverlayImage1Bits(YUV422Image, szString, MaxStrLen, FontW, FontH, VideoW, flag);
#elif (ISU_OVL_TYPE == ISU_OVL_TYPE_2Bit)
    return GenerateOverlayImage1Bits(YUV422Image, szString, MaxStrLen, FontW * 2, FontH, VideoW * 2, flag);
#endif
}

#if (ISU_OVL_TYPE == ISU_OVL_TYPE_16Bit)
s32 GenerateOverlayImage16Bits(u32 *YUV422Image, char *szString, int MaxStrLen, int FontW, int FontH, int VideoW, u8 ASCII_flag)
{
    int length, i, j, k, FontW_Word, VideoW_Word, x, y;
    u32 *source, *target, *YUV422Image1;
    u32 CharPerLine;
#if SPEEDUP
    int length1;
#endif

    length  = strlen(szString);

    for(i = 0; i < length; i++) {
        if(szString[i] > 126 || szString[i] < 32) {
            DEBUG_ISU("Error: The string\"%s\" not valid ASCII code!!!\n", szString);
            return 0;
        }
    }

    // draw font to overlay image
    FontW_Word      = FontW / 2;
    VideoW_Word     = VideoW / 2;
    YUV422Image1    = YUV422Image;
    CharPerLine     = VideoW / FontW;
    for(i = 0, x = 0, y = 0; i < length && i < MaxStrLen; i++, x++) {
        if(x == CharPerLine)
        {
            x               = 0;
            y++;
            YUV422Image1    = YUV422Image + VideoW_Word * FontH * y;
        }
        else if(x != 0)
        {
            YUV422Image1   += FontW_Word;
        }
#if SPEEDUP
        if(szString[i] != szString1[i])
#endif
        {
            if(!ASCII_flag ) //Lsk : QVGA frames
                source  = (u32*)QVGA_ASCII_Font[szString[i] - 32];
            else
                source  = (u32*)ASCII_Font[szString[i] - 32];
            target  = YUV422Image1;
            for(j = 0; j < FontH; j++) {
                memcpy(target, source, sizeof(u32) * FontW_Word);
                source += FontW_Word;
                target += VideoW_Word;
            }
        }
    }

#if SPEEDUP
    length1 = strlen(szString1);
    if(length < length1)
#endif
    {
        // clean remainder overlay image
        if(((length % CharPerLine) * FontW) < VideoW) {
            target  = YUV422Image1;
            k       = (VideoW - (length % CharPerLine) * FontW) * 2;
            for(j = 0; j < FontH; j++) {
                memset(target, 0, k);
                target += VideoW_Word;
            }
        }
    }

#if SPEEDUP
    strcpy(szString1, szString);
#endif

    return  1;
}
#endif  // #if (ISU_OVL_TYPE == ISU_OVL_TYPE_16Bit)

#if ((ISU_OVL_TYPE == ISU_OVL_TYPE_1Bit) || (ISU_OVL_TYPE == ISU_OVL_TYPE_2Bit))
s32 GenerateOverlayImage1Bits(u32 *YUV422Image, char *szString, int MaxStrLen, int FontW, int FontH, int VideoW, u8 ASCII_flag)
{
    int length, i, j, k, FontW_Byte, VideoW_Byte, x, y;
    u8  *source, *target, *YUV422Image1;
    u32 CharPerLine;
#if SPEEDUP
    int length1;
#endif

    length  = strlen(szString);

    for(i = 0; i < length; i++) {
        if(szString[i] > 126 || szString[i] < 32) {
            DEBUG_ISU("Error: The string\"%s\" not valid ASCII code!!!\n", szString);
            return 0;
        }
    }

    // draw font to overlay image
    FontW_Byte      = FontW / 8;
    VideoW_Byte     = (VideoW + 31) / 32 * 4;
    YUV422Image1    = (u8*)YUV422Image;
    CharPerLine     = VideoW / FontW;
    for(i = 0, x = 0, y = 0; i < length && i < MaxStrLen; i++, x++) {
        if(x == CharPerLine)
        {
            x               = 0;
            y++;
            YUV422Image1    = (u8*)YUV422Image + VideoW_Byte * FontH * y;
        }
        else if(x != 0)
        {
            YUV422Image1   += FontW_Byte;
        }
#if SPEEDUP
        if(szString[i] != szString1[i])
#endif
        {
            if(!ASCII_flag ) //Lsk : QVGA frames
                source  = (u8*)QVGA_ASCII_Font[szString[i] - 32];
            else
                source  = (u8*)ASCII_Font[szString[i] - 32];
            target  = YUV422Image1;
            for(j = 0; j < FontH; j++) {
                memcpy(target, source, sizeof(u8) * FontW_Byte);
                source += FontW_Byte;
                target += VideoW_Byte;
            }
        }
    }

#if SPEEDUP
    length1 = strlen(szString1);
    if(length < length1)
#endif
    {
        // clean remainder overlay image
        if(((length % CharPerLine) * FontW) < VideoW) {
            target  = YUV422Image1;
            k       = (VideoW_Byte - (length % CharPerLine) * FontW_Byte);
            for(j = 0; j < FontH; j++) {
                memset(target, 0, k);
                target += VideoW_Byte;
            }
        }
    }

#if SPEEDUP
    strcpy(szString1, szString);
#endif

    return  1;
}
#endif  // #if ((ISU_OVL_TYPE == ISU_OVL_TYPE_1Bit) || (ISU_OVL_TYPE == ISU_OVL_TYPE_2Bit))

#undef  SPEEDUP

#endif

s32 isuInit(void)
{
    isuSemEvt = OSSemCreate(0);

    return 1;
}

void ResetisuPanel(void)
    {
        if(sysTVOutOnFlag)
        {
          if(TvOutMode==SYS_TV_OUT_PAL)
          {
            #ifdef TV_WITH_PAL_FORCE2QVGA
             //isuPanel.h    = TVOUT_Y_NTSC/2;
             isuPanel.h    = 288; // 576/2=288
             isuPanel.w    = TVOUT_X/2;
            #else
             isuPanel.h    = 576;
             isuPanel.w    = TVOUT_X;
            #endif
          }
          else
          {
              isuPanel.h    = TVOUT_Y_NTSC;
              isuPanel.w    = TVOUT_X;
          }
        }
        else
        {
            isuPanel.h    = PANNEL_Y;
            isuPanel.w    = PANNEL_X;
        }
    }

    s32 isuSetIOSize(u16 width, u16 height)
    {
        isuSrcImg.h = height;
        isuSrcImg.w = width;
        return 1;
    }
/*Peter 1109 S*/
/*

Routine Description:

    The FIQ handler of Image Scaling Unit.

Arguments:

    None.

Return Value:

    None.

*/


#if ISU_SUPPORT
    /*****************************************
    Lsk 090813
    AV Sync when skip frame:
    1. SIU skip frame,不會更新isuFrameTime,故計算出來的ISUFrameDuration正確
    2. ISU skip frame,會更新isuFrameTime,故需累積SkipFrameDuration
    *****************************************/
    void isuIntHandler(void)
    {
        u32 intStat = IsuSCA_INTC;
    #if FINE_TIME_STAMP
        s32 IISTimeOffsetISU, TimeOffset;
        u32 IISTime1;
    #endif
        //=======================//
    #if DEBUG_ISU_INTR_USE_LED6
        static u32 count=0;
        gpioSetLevel(1, 6, count & 0x01);
        count ++;
        //gpioSetLevel(1, 6, 0x01);
    #endif

        isu_int_status  = intStat;
        if((isu_int_status & 0x00000001)==0)
        {
          DEBUG_ISU("\n#ISU Data Overflow:0x%x\n",isu_int_status);
          return; //if not frame complete ==> return
        }
    #if 1
        if(isu_int_status & 0x00000060)
        {
          DEBUG_ISU("\nISU PN Data Overflow:0x%x\n",isu_int_status);
        }
    #endif
        //-------------------------Capture Mode----------------------------//
        if( siuOpMode == SIUMODE_CAPTURE )
        {
            OSSemPost(isuSemEvt);
            isuStatus_OnRunning = 0;
        }
        else if( sysCameraMode == SYS_CAMERA_MODE_PREVIEW)
        {
            if (siuOpMode == SIUMODE_MPEGAVI)  //For Mpeg recording mode
            {
               isuIntProcess_MPEGAVI();
            }
            //-------------------------Preview Mode (Only VGA resolution)-------------------------------//
            else if(siuOpMode == SIUMODE_PREVIEW)
            {
               isuIntProcess_PREVIEW();
            }
            //-------------------------Capture Preview Mode (Only VGA resolution)-------------------------------//
            else if(siuOpMode == SIUMODE_CAP_RREVIEW)
            {
               isuIntProcess_CAP_PREVIEW();
            }
        }
        //-------------------------------- Playback Mode----------------------------------//
        else //For others mode
        {
            OSSemPost(isuSemEvt);
            IsuIndex++;
            isuStatus_OnRunning = 0;
        }

    }
    /*Peter 1109 E*/



#if ( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC)  )
    void isuIntProcess_MPEGAVI(void)
    {
      #if FINE_TIME_STAMP
          s32 IISTimeOffsetISU, TimeOffset;
          u32 IISTime1;
      #endif

      if( sysVideoInSel == VIDEO_IN_TV) //TV-in
      {
          if ( (FieldTypeID & 0x01) == DEF_TOP_FIELD)
          {
          }
          else
          {
           #if MULTI_CHANNEL_VIDEO_REC
                if(VideoClipOption[0].sysReady2CaptureVideo)
           #else
                if(sysReady2CaptureVideo)
           #endif
                {
                   #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                    timerCountRead(2, (u32*) &IISTimeOffsetISU);
                    IISTimeOffsetISU    = IISTimeOffsetISU >> 8;
                    if(IISTimeOffset >= IISTimeOffsetISU) {
                        TimeOffset  = IISTimeOffset - IISTimeOffsetISU;
                    } else {
                        TimeOffset  = IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetISU;
                    }
                    if(TimeOffset > IISTimeUnit) {
                        TimeOffset  = IISTimeUnit;
                    }
                    IISTime1    = IISTime + TimeOffset;
                    if(IISTime1 > isuFrameTime) {
                        ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = (IISTime1 - isuFrameTime) + SkipFrameDuration;  /* Peter 070403 */
                        isuFrameTime                                                = IISTime1;
                    }
        		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
        			timerCountRead(1, (u32*) &IISTimeOffsetISU);
                    IISTimeOffsetISU    = IISTimeOffsetISU / 100;
        			if(IISTimeOffset >= IISTimeOffsetISU) {
        				TimeOffset  = IISTimeOffset - IISTimeOffsetISU;
        			} else {
        			    TimeOffset  = IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetISU;
        			}
        			if(TimeOffset > IISTimeUnit) {
        				TimeOffset  = IISTimeUnit;
        			}
        			IISTime1    = IISTime + TimeOffset;
        			if(IISTime1 > isuFrameTime) {
        				ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = (IISTime1 - isuFrameTime) + SkipFrameDuration;  /* Peter 070403 */
                        isuFrameTime                                                = IISTime1;
                    }
                  #else   // only use IIS time to calculate frame time
                    if(IISTime > isuFrameTime) {
                        ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = (IISTime - isuFrameTime) + SkipFrameDuration;  /* Peter 070403 */
                        isuFrameTime                                                = IISTime;
                    }
                  #endif
                    else
                    {
                        ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = 1 + SkipFrameDuration;  /* Peter 070403 */
                        isuFrameTime++;
                    }
                }

                //---Frame rate Control---//
                if (
                #if MULTI_CHANNEL_VIDEO_REC
                       (VideoClipOption[0].sysReady2CaptureVideo==1) &&
                #else
                       (sysReady2CaptureVideo==1) &&
                #endif
                       ( ( (isu_avifrmcnt -avi_FrameSkip- mp4_avifrmcnt) >= (MAX_VIDEO_FRAME_BUFFER-1)) || (siuSkipFrameRate != 0))

        		   )
        		{
        		    avi_FrameSkip = 1;
        		    unMp4SkipFrameCount++;
                    SkipFrameDuration = ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM];
        		    DEBUG_ISU("@");
        		}
        		else
        		{
        		    SkipFrameDuration = 0;
        			avi_FrameSkip = 0;
        	        unMp4SkipFrameCount = 0;

                    isu_avifrmcnt++;
                    OSSemPost(isuSemEvt);
        		}
          }
        }
        else //sensor-in
        {
           #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
            timerCountRead(2, (u32*) &IISTimeOffsetISU);
            IISTimeOffsetISU    = IISTimeOffsetISU >> 8;
            if(IISTimeOffset >= IISTimeOffsetISU) {
                TimeOffset  = IISTimeOffset - IISTimeOffsetISU;
            } else {
                TimeOffset  = IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetISU;
            }
            if(TimeOffset > IISTimeUnit) {
                TimeOffset  = IISTimeUnit;
            }
            IISTime1    = IISTime + TimeOffset;
            if(IISTime1 > isuFrameTime) {
                ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = (IISTime1 - isuFrameTime) + SkipFrameDuration;
                isuFrameTime                                                = IISTime1;
            }
    	#elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
    	    timerCountRead(1, (u32*) &IISTimeOffsetISU);
            IISTimeOffsetISU    = IISTimeOffsetISU / 100;
            if(IISTimeOffset >= IISTimeOffsetISU) {
                TimeOffset  = IISTimeOffset - IISTimeOffsetISU;
            } else {
                TimeOffset  = IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetISU;
            }
            if(TimeOffset > IISTimeUnit) {
                TimeOffset  = IISTimeUnit;
            }
            IISTime1    = IISTime + TimeOffset;
            if(IISTime1 > isuFrameTime) {
                ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = (IISTime1 - isuFrameTime) + SkipFrameDuration; /* Peter 070403 */
                isuFrameTime                                                = IISTime1;
            }
        #else   // only use IIS time to calculate frame time
            if(IISTime > isuFrameTime) {
                ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = IISTime - isuFrameTime + SkipFrameDuration;  /* Peter 070403 */
                isuFrameTime                                                = IISTime;
            }
        #endif
            else {
                ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = 1 + SkipFrameDuration;  /* Peter 070403 */
                isuFrameTime++;
            }

            //---Frame rate Control---//
            if (
               #if MULTI_CHANNEL_VIDEO_REC
                   (VideoClipOption[0].sysReady2CaptureVideo==1) &&
               #else
                   (sysReady2CaptureVideo==1) &&
               #endif
                   ( ( (isu_avifrmcnt -avi_FrameSkip- mp4_avifrmcnt) >= (MAX_VIDEO_FRAME_BUFFER-1)) || (siuSkipFrameRate != 0))

    		   )
    		{
    		    avi_FrameSkip = 1;
    		    unMp4SkipFrameCount++;
                SkipFrameDuration = ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM];
    		    DEBUG_ISU("@");
    		}
    		else
    		{
    		    SkipFrameDuration = 0;
    			avi_FrameSkip = 0;
    	        unMp4SkipFrameCount = 0;

                isu_avifrmcnt++;
                isu_idufrmcnt++;
                OSSemPost(isuSemEvt);

    		}
        }

        isuStatus_OnRunning = 0;
    }
#else
    void isuIntProcess_MPEGAVI(void)
    {
      #if FINE_TIME_STAMP
          s32 IISTimeOffsetISU, TimeOffset;
          u32 IISTime1;
      #endif

      if( sysVideoInSel == VIDEO_IN_TV) //TV-in
      {
            #if( (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV) )

            #else //656
                    FieldTypeID=(SiuSyncStat>>31) & 0x01;
            #endif

            	    if ( (FieldTypeID & 0x01) == DEF_TOP_FIELD)
                    {   // odd field, top field.
                        if(sysTVOutOnFlag) //TV-out
                        {
                        #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_QVGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
                            #if DROP_BOTTOMFIELD
                                IsuSCA_FUN  |= (ISU_PKOUT_ENA);
                            #endif
                        #else
                            //0x0050
            	            IsuPK_IADDR0    = (((u32)PKBuf0 + isuPanel.w * 2) & ISU_OffAdr_Mask);
            	            IsuPK_IADDR1    = (((u32)PKBuf1 + isuPanel.w * 2) & ISU_OffAdr_Mask);
            	            IsuPK_IADDR2    = (((u32)PKBuf2 + isuPanel.w * 2) & ISU_OffAdr_Mask);
                        #endif
                        }
                        else //Pannel-out
                        {
                        #if ((LCM_OPTION == LCM_CCIR601_640x480P) || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
                            //0x0050
            	            IsuPK_IADDR0    = (((u32)PKBuf0 + isuPanel.w * 2) & ISU_OffAdr_Mask);
            	            IsuPK_IADDR1    = (((u32)PKBuf1 + isuPanel.w * 2) & ISU_OffAdr_Mask);
            	            IsuPK_IADDR2    = (((u32)PKBuf2 + isuPanel.w * 2) & ISU_OffAdr_Mask);
                        #else
                           #if DROP_BOTTOMFIELD
                            IsuSCA_FUN  |= (ISU_PKOUT_ENA);
                           #endif
                        #endif

                        }

                        if( (mpeg4Width == 320) || (mpeg4Width == 352) ) //QVGA
                        {
                            IsuSCA_FUN  &= (~ISU_PNOUT_ENA);
                        }
                        else
                        {
        	            //0x0060
                            IsuPN_YIADDR0   = (((u32)PNBuf_Y[isu_avifrmcnt & 0x03] + isuPacket.w)& ISU_OffAdr_Mask);
                            IsuPN_CIADDR0   = (((u32)PNBuf_C[isu_avifrmcnt & 0x03] + isuPacket.w)& ISU_OffAdr_Mask);
                        #if ISU_OVERLAY_ENABLE
                          #if(ISU_OVL_TYPE == ISU_OVL_TYPE_16Bit)
                            IsuOVL_IADDR    = (u32)ScalarOverlayImage + 1280;
                          #elif(ISU_OVL_TYPE == ISU_OVL_TYPE_1Bit)
                            IsuOVL_IADDR    = (u32)ScalarOverlayImage + 80;
                          #elif(ISU_OVL_TYPE == ISU_OVL_TYPE_2Bit)
                            IsuOVL_IADDR    = (u32)ScalarOverlayImage + 160;
                          #endif
                        #endif
                        }
            	    }
                    else
                    {   // even field, bottom field.
                        if(sysTVOutOnFlag) //TV-out
                        {
                        #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_QVGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
                            #if DROP_BOTTOMFIELD
                            IsuSCA_FUN  &= (~ISU_PKOUT_ENA);
                            #endif
                        #endif
            	            //0x0050
            	            IsuPK_IADDR0    = (((u32)PKBuf0) & ISU_OffAdr_Mask);
            	            IsuPK_IADDR1    = (((u32)PKBuf1) & ISU_OffAdr_Mask);
            	            IsuPK_IADDR2    = (((u32)PKBuf2) & ISU_OffAdr_Mask);

                        }
                        else //Pannel-out
                        {
                        #if ( (LCM_OPTION == LCM_CCIR601_640x480P)  || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
            	            //0x0050
            	            IsuPK_IADDR0    = (((u32)PKBuf0) & ISU_OffAdr_Mask);
            	            IsuPK_IADDR1    = (((u32)PKBuf1) & ISU_OffAdr_Mask);
            	            IsuPK_IADDR2    = (((u32)PKBuf2) & ISU_OffAdr_Mask);

                        #else
                            #if DROP_BOTTOMFIELD
                               IsuSCA_FUN  &= (~ISU_PKOUT_ENA);
                            #endif
                        #endif

                        if(sysVideoInCHsel == 0x00)
                        {
                            #if ISUCIU_PREVIEW_PNOUT
                                IduVidBuf0Addr=(((u32)PNBuf_Y[isu_avifrmcnt & 0x03])& ISU_OffAdr_Mask);
                                #if NEW_IDU_BRI
                                    BRI_IADDR_Y = IduVidBuf0Addr;
                                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                #endif
                            #else
                                IduWinCtrl = (IduWinCtrl & ~0x00003000) | ((isu_idufrmcnt % 3) << 12);
                                #if NEW_IDU_BRI
                                    switch( (isu_idufrmcnt) % 3)
                                    {
                                       case 0:
                                          BRI_IADDR_Y = IduVidBuf0Addr;
                                          BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                          break;
                                       case 1:
                                          BRI_IADDR_Y = IduVidBuf1Addr;
                                          BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                          break;
                                       case 2:
                                          BRI_IADDR_Y = IduVidBuf2Addr;
                                          BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                          break;
                                    }
                               #endif
                            #endif
                         }

                        #if((LCM_OPTION == LCM_HX8312)||(LCM_OPTION == LCM_TG200Q04)||(LCM_OPTION == LCM_TM024HDH29))  //on LCM, PlaybackMode is SINGLE mode, otherwise..
                            idu_switch();
                        #endif
                        }
                        isu_idufrmcnt++;
                        IsuSCA_MODE = (IsuSCA_MODE & ~0x3c00) |
                                      ((isu_idufrmcnt % 3) << 10);
                    #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                        timerCountRead(2, (u32*) &IISTimeOffsetISU);
                        IISTimeOffsetISU    = IISTimeOffsetISU >> 8;
                        if(IISTimeOffset >= IISTimeOffsetISU) {
                            TimeOffset  = IISTimeOffset - IISTimeOffsetISU;
                        } else {
                            TimeOffset  = IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetISU;
                        }
                        if(TimeOffset > IISTimeUnit) {
                            TimeOffset  = IISTimeUnit;
                        }
                        IISTime1    = IISTime + TimeOffset;
                        if(IISTime1 > isuFrameTime) {
                            ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = IISTime1 - isuFrameTime;  /* Peter 070403 */
                            isuFrameTime                                                = IISTime1;
                        }
        			#elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
        				timerCountRead(1, (u32*) &IISTimeOffsetISU);
                        IISTimeOffsetISU    = IISTimeOffsetISU / 100;
        				if(IISTimeOffset >= IISTimeOffsetISU) {
        					TimeOffset  = IISTimeOffset - IISTimeOffsetISU;
        				} else {
        				    TimeOffset  = IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetISU;
        				}
        				if(TimeOffset > IISTimeUnit) {
                            TimeOffset  = IISTimeUnit;
        				}
        				IISTime1    = IISTime + TimeOffset;
        				if(IISTime1 > isuFrameTime) {
        					ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = IISTime1 - isuFrameTime;  /* Peter 070403 */
                            isuFrameTime                                                = IISTime1;
                        }
                    #else   // only use IIS time to calculate frame time
                        if(IISTime > isuFrameTime) {
                            ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = IISTime - isuFrameTime;  /* Peter 070403 */
                            isuFrameTime                                                = IISTime;
                        }
                    #endif
                        else
                        {
                            ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = 1;  /* Peter 070403 */
                            isuFrameTime++;
                        }
                        isu_avifrmcnt++;

                        if((mpeg4Width == 320) || (mpeg4Width == 352)) //QVGA
                        {
                            IsuSCA_FUN  |= (ISU_PNOUT_ENA);
                        }
                        IsuPN_YIADDR0 = (((u32)PNBuf_Y[isu_avifrmcnt & 0x03])& ISU_OffAdr_Mask); //Lucian: 用四個frame buffer.
                        IsuPN_CIADDR0 = (((u32)PNBuf_C[isu_avifrmcnt & 0x03])& ISU_OffAdr_Mask);
                    #if ISU_OVERLAY_ENABLE
                        IsuOVL_IADDR    = (u32)ScalarOverlayImage;
                    #endif

                        OSSemPost(isuSemEvt);
                        isuStatus_OnRunning = 0;
                    }
        }
        else  //Sensor input..
        {
                #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                    timerCountRead(2, (u32*) &IISTimeOffsetISU);
                    IISTimeOffsetISU    = IISTimeOffsetISU >> 8;
                    if(IISTimeOffset >= IISTimeOffsetISU) {
                        TimeOffset  = IISTimeOffset - IISTimeOffsetISU;
                    } else {
                        TimeOffset  = IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetISU;
                    }
                    if(TimeOffset > IISTimeUnit) {
                        TimeOffset  = IISTimeUnit;
                    }
                    IISTime1    = IISTime + TimeOffset;
                    if(IISTime1 > isuFrameTime) {
                        ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = IISTime1 - isuFrameTime;
                        isuFrameTime                                                = IISTime1;
                    }
        		#elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
        			timerCountRead(1, (u32*) &IISTimeOffsetISU);
                    IISTimeOffsetISU    = IISTimeOffsetISU / 100;
                    if(IISTimeOffset >= IISTimeOffsetISU) {
                        TimeOffset  = IISTimeOffset - IISTimeOffsetISU;
                    } else {
                        TimeOffset  = IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetISU;
                    }
                    if(TimeOffset > IISTimeUnit) {
                        TimeOffset  = IISTimeUnit;
                    }
                    IISTime1    = IISTime + TimeOffset;
                    if(IISTime1 > isuFrameTime) {
                        ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = IISTime1 - isuFrameTime;  /* Peter 070403 */
                        isuFrameTime                                                = IISTime1;
                    }
                #else   // only use IIS time to calculate frame time
                    if(IISTime > isuFrameTime) {
                        ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = IISTime - isuFrameTime;  /* Peter 070403 */
                        isuFrameTime                                                = IISTime;
                    }
                #endif
                    else {
                        ISUFrameDuration[isu_avifrmcnt % ISU_FRAME_DURATION_NUM]    = 1;  /* Peter 070403 */
                        isuFrameTime++;
                    }


                    if(sysTVOutOnFlag)
                    {
                    }
                    else
                    {
                        if(sysVideoInCHsel == 0x00)
                        {
                            #if ISUCIU_PREVIEW_PNOUT
                               IduVidBuf0Addr=(((u32)PNBuf_Y[isu_avifrmcnt & 0x03])& ISU_OffAdr_Mask);
                               #if NEW_IDU_BRI
                                  BRI_IADDR_Y = IduVidBuf0Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                               #endif
                            #else
                               IduWinCtrl = (IduWinCtrl & ~0x00003000) | ((isu_idufrmcnt % 3) << 12);
                                #if NEW_IDU_BRI
                                switch( (isu_idufrmcnt) % 3)
                                {
                                   case 0:
                                      BRI_IADDR_Y = IduVidBuf0Addr;
                                      BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                      break;
                                   case 1:
                                      BRI_IADDR_Y = IduVidBuf1Addr;
                                      BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                      break;
                                   case 2:
                                      BRI_IADDR_Y = IduVidBuf2Addr;
                                      BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                      break;
                                }
                                #endif
                            #endif
                         }
                    }

                    isu_avifrmcnt++;
                #if((LCM_OPTION == LCM_HX8312)||(LCM_OPTION == LCM_TG200Q04)||(LCM_OPTION == LCM_TM024HDH29))  //on LCM, PlaybackMode is SINGLE mode, otherwise..
                    if(!sysTVOutOnFlag)
                       idu_switch();
                #endif
                    isu_idufrmcnt++;
                    IsuSCA_MODE = (IsuSCA_MODE & ~0x3c00) |
                                  ((isu_idufrmcnt % 3) << 10);
                    IsuPN_YIADDR0 = (((u32)PNBuf_Y[isu_avifrmcnt & 0x03])& ISU_OffAdr_Mask); //Lucian: 用四個frame buffer.
                    IsuPN_CIADDR0 = (((u32)PNBuf_C[isu_avifrmcnt & 0x03])& ISU_OffAdr_Mask);

                    OSSemPost(isuSemEvt);
                    isuStatus_OnRunning = 0;
         }
    }
#endif

    void isuIntProcess_PREVIEW(void)
    {
       if( sysVideoInSel == VIDEO_IN_TV) //TV-in
       {
          #if (ISU_OUT_BY_FID)
              //Do nothing...
          #else
              #if( (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV) )

              #else //656
                  FieldTypeID=(SiuSyncStat>>31) & 0x01;
              #endif

                if ( (FieldTypeID & 0x01) == DEF_TOP_FIELD)
                {    // odd field, Top field.
                    if(sysTVOutOnFlag) //TV-out
                    {
                    #if ( ISUCIU_PREVIEW_PNOUT  )
                       /*
                        IsuPN_YIADDR0 = (((u32)PKBuf0+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR0 = (((u32)PKBuf0+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_YIADDR1 = (((u32)PKBuf1+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR1 = (((u32)PKBuf1+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_YIADDR2 = (((u32)PKBuf2+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR2 = (((u32)PKBuf2+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                        */
                        IsuPN_YIADDR0   = (((u32)PNBuf_Y[isu_idufrmcnt & 0x03] + isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR0   = (((u32)PNBuf_C[isu_idufrmcnt & 0x03] + isuPanel.w)& ISU_OffAdr_Mask);
                    #else
                        IsuPK_IADDR0    = (((u32)PKBuf0 + isuPanel.w * 2) & ISU_OffAdr_Mask);
        	            IsuPK_IADDR1    = (((u32)PKBuf1 + isuPanel.w * 2) & ISU_OffAdr_Mask);
        	            IsuPK_IADDR2    = (((u32)PKBuf2 + isuPanel.w * 2) & ISU_OffAdr_Mask);
                    #endif
                    }
                    else //Pannel-out
                    {
                    #if ( (LCM_OPTION == LCM_CCIR601_640x480P) || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272)|| (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) || NEW_IDU_BRI ) //Top/Bottom field 組合輸出.
                       #if ( ISUCIU_PREVIEW_PNOUT  )
                        /*
                        IsuPN_YIADDR0 = (((u32)PKBuf0+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR0 = (((u32)PKBuf0+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_YIADDR1 = (((u32)PKBuf1+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR1 = (((u32)PKBuf1+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_YIADDR2 = (((u32)PKBuf2+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR2 = (((u32)PKBuf2+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                        */
                        IsuPN_YIADDR0   = (((u32)PNBuf_Y[isu_idufrmcnt & 0x03] + isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR0   = (((u32)PNBuf_C[isu_idufrmcnt & 0x03] + isuPanel.w)& ISU_OffAdr_Mask);
                       #else
                        IsuPK_IADDR0    = (((u32)PKBuf0 + isuPanel.w * 2) & ISU_OffAdr_Mask);
        	            IsuPK_IADDR1    = (((u32)PKBuf1 + isuPanel.w * 2) & ISU_OffAdr_Mask);
        	            IsuPK_IADDR2    = (((u32)PKBuf2 + isuPanel.w * 2) & ISU_OffAdr_Mask);
                       #endif
                    #else
                        #if DROP_BOTTOMFIELD
                          #if ( ISUCIU_PREVIEW_PNOUT  )
                             IsuSCA_FUN  |= (ISU_PNOUT_ENA);
                          #else
                             IsuSCA_FUN  |= (ISU_PKOUT_ENA);
                          #endif
                        #endif
                    #endif

                    }
        	    }
                else
                {   // even field, bottom field.
                    if(sysTVOutOnFlag) //TV-out
                    {

                    #if ( ISUCIU_PREVIEW_PNOUT  )
                        /*
                        IsuPN_YIADDR0 = (((u32)PKBuf0)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR0 = (((u32)PKBuf0+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                        IsuPN_YIADDR1 = (((u32)PKBuf1)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR1 = (((u32)PKBuf1+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                        IsuPN_YIADDR2 = (((u32)PKBuf2)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR2 = (((u32)PKBuf2+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                        */
                    #else
                        IsuPK_IADDR0    = (((u32)PKBuf0) & ISU_OffAdr_Mask);
                        IsuPK_IADDR1    = (((u32)PKBuf1) & ISU_OffAdr_Mask);
                        IsuPK_IADDR2    = (((u32)PKBuf2) & ISU_OffAdr_Mask);
                    #endif

                    }
                    else //Pannel-out
                    {
                          if(sysVideoInCHsel == 0x00)
                          {
                          #if ISUCIU_PREVIEW_PNOUT
                               IduWinCtrl = (IduWinCtrl & ~0x00003000);
                               IduVidBuf0Addr=(((u32)PNBuf_Y[isu_idufrmcnt & 0x03])& ISU_OffAdr_Mask);
                               #if NEW_IDU_BRI
                                BRI_IADDR_Y = IduVidBuf0Addr;
                                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                               #endif
                          #else
                               IduWinCtrl = (IduWinCtrl & ~0x00003000) | ((isu_idufrmcnt % 3) << 12);
                                #if NEW_IDU_BRI
                                switch( (isu_idufrmcnt) % 3)
                                {
                                   case 0:
                                      BRI_IADDR_Y = IduVidBuf0Addr;
                                      BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                      break;
                                   case 1:
                                      BRI_IADDR_Y = IduVidBuf1Addr;
                                      BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                      break;
                                   case 2:
                                      BRI_IADDR_Y = IduVidBuf2Addr;
                                      BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                      break;
                                }
                                #endif
                          #endif
                          }
                    #if ( (LCM_OPTION == LCM_CCIR601_640x480P) || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272)|| (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) || NEW_IDU_BRI ) //for big pannel, Two field
                       #if ( ISUCIU_PREVIEW_PNOUT  )
                          /*
                            IsuPN_YIADDR0 = (((u32)PKBuf0)& ISU_OffAdr_Mask);
                            IsuPN_CIADDR0 = (((u32)PKBuf0+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                            IsuPN_YIADDR1 = (((u32)PKBuf1)& ISU_OffAdr_Mask);
                            IsuPN_CIADDR1 = (((u32)PKBuf1+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                            IsuPN_YIADDR2 = (((u32)PKBuf2)& ISU_OffAdr_Mask);
                            IsuPN_CIADDR2 = (((u32)PKBuf2+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                           */
                       #else
                            IsuPK_IADDR0    = (((u32)PKBuf0) & ISU_OffAdr_Mask);
                            IsuPK_IADDR1    = (((u32)PKBuf1) & ISU_OffAdr_Mask);
                            IsuPK_IADDR2    = (((u32)PKBuf2) & ISU_OffAdr_Mask);
                       #endif
                    #else //for small pannel: only one field reserved
                        #if DROP_BOTTOMFIELD
                           #if ( ISUCIU_PREVIEW_PNOUT  )
                              IsuSCA_FUN  &= (~ISU_PNOUT_ENA);
                           #else
                              IsuSCA_FUN  &= (~ISU_PKOUT_ENA);
                           #endif
                        #endif
                    #endif

                    #if((LCM_OPTION == LCM_HX8312)||(LCM_OPTION == LCM_TG200Q04)||(LCM_OPTION == LCM_TM024HDH29))  //on LCM, PlaybackMode is SINGLE mode, otherwise..
                        idu_switch();
                    #endif
                    }

                    isu_idufrmcnt++;
                #if ( ISUCIU_PREVIEW_PNOUT  )
                    IsuSCA_MODE = (IsuSCA_MODE & ~ISU_PNWSEL_MASK);
                    IsuPN_YIADDR0   = (((u32)PNBuf_Y[isu_idufrmcnt & 0x03] )& ISU_OffAdr_Mask);
                    IsuPN_CIADDR0   = (((u32)PNBuf_C[isu_idufrmcnt & 0x03] )& ISU_OffAdr_Mask);
                #else
                    IsuSCA_MODE = (IsuSCA_MODE & ~ISU_PKWSEL_MASK) | ((isu_idufrmcnt % 3) << ISU_PKWSEL_SHFT);
                #endif

                }
            #endif
        }
        else //Sensor input
        {
           if(sysTVOutOnFlag) //TV-out
           {
           }
           else
           {
                   if(sysVideoInCHsel == 0x00)
                   {
                    #if ISUCIU_PREVIEW_PNOUT
                        IduWinCtrl = (IduWinCtrl & ~0x00003000);
                        IduVidBuf0Addr=(((u32)PNBuf_Y[isu_idufrmcnt & 0x03])& ISU_OffAdr_Mask);
                        #if NEW_IDU_BRI
                            BRI_IADDR_Y = IduVidBuf0Addr;
                            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                        #endif
                    #else
                        IduWinCtrl = (IduWinCtrl & ~0x00003000) | ((isu_idufrmcnt % 3) << 12);
                        #if NEW_IDU_BRI
                        switch( (isu_idufrmcnt) % 3)
                        {
                           case 0:
                              BRI_IADDR_Y = IduVidBuf0Addr;
                              BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                              break;
                           case 1:
                              BRI_IADDR_Y = IduVidBuf1Addr;
                              BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                              break;
                           case 2:
                              BRI_IADDR_Y = IduVidBuf2Addr;
                              BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                              break;
                        }
                        #endif
                    #endif
                    }
           }
        #if((LCM_OPTION == LCM_HX8312)||(LCM_OPTION == LCM_TG200Q04)||(LCM_OPTION == LCM_TM024HDH29)) //on LCM, PlaybackMode is SINGLE mode, otherwise..
            if(!sysTVOutOnFlag)
              idu_switch();
        #endif
            isu_idufrmcnt++;
        #if ( ISUCIU_PREVIEW_PNOUT )
            IsuSCA_MODE = (IsuSCA_MODE & ~ISU_PNWSEL_MASK);
            IsuPN_YIADDR0   = (((u32)PNBuf_Y[isu_idufrmcnt & 0x03] )& ISU_OffAdr_Mask);
            IsuPN_CIADDR0   = (((u32)PNBuf_C[isu_idufrmcnt & 0x03] )& ISU_OffAdr_Mask);
        #else
            IsuSCA_MODE = (IsuSCA_MODE & ~ISU_PKWSEL_MASK) | ((isu_idufrmcnt % 3) << ISU_PKWSEL_SHFT);
        #endif
        }
        isuStatus_OnRunning = 0;
    }

    void isuIntProcess_CAP_PREVIEW(void)
    {
       if( sysVideoInSel == VIDEO_IN_TV) //TV-in
       {
          #if( (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV) )

          #else //656
            FieldTypeID=(SiuSyncStat>>31) & 0x01;
          #endif
            if ( (FieldTypeID & 0x01) == DEF_TOP_FIELD)
            {    // odd field, Top field.
                isu_idufrmcnt = 1;

                IsuPK_IADDR0    = (((u32)PKBuf0 + isuPanel.w * 2) & ISU_OffAdr_Mask);
                IsuPK_IADDR1    = (((u32)PKBuf1 + isuPanel.w * 2) & ISU_OffAdr_Mask);
                IsuPK_IADDR2    = (((u32)PKBuf2 + isuPanel.w * 2) & ISU_OffAdr_Mask);
    	    }
            else
            {   // even field, bottom field.
                if(isu_idufrmcnt==1)
                    isu_idufrmcnt=2;
                else
                    isu_idufrmcnt=0;

                IsuPK_IADDR0    = (((u32)PKBuf0) & ISU_OffAdr_Mask);
                IsuPK_IADDR1    = (((u32)PKBuf1) & ISU_OffAdr_Mask);
                IsuPK_IADDR2    = (((u32)PKBuf2) & ISU_OffAdr_Mask);
            }
        }
        else //Sensor input
        {
            isu_idufrmcnt = 2;
        }
        isuStatus_OnRunning = 0;
    }


    void isuOutputAddrArrange_TV(void)
    {
      #if FINE_TIME_STAMP
        s32 IISTimeOffsetISU, TimeOffset;
        u32 IISTime1;
      #endif

        //-------------------------------AVI mode---------------------------//
        //Lucian: 解決錄影時(頻寬不足,造成ISR延遲)畫面跳動問題.
        if ( (siuOpMode == SIUMODE_MPEGAVI) && (sysCameraMode == SYS_CAMERA_MODE_PREVIEW) )
        {
          #if( (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV) )
          #else //656
              FieldTypeID=(SiuSyncStat>>31) & 0x01;
          #endif

    	    if ( (FieldTypeID & 0x01) == DEF_TOP_FIELD)
            {   // odd field, top field.

                if(sysTVOutOnFlag) //TV-out
                {
                #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_QVGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
                   #if DROP_BOTTOMFIELD
                     IsuSCA_FUN  |= (ISU_PKOUT_ENA);
                   #endif
                #else
                    //0x0050
    	            IsuPK_IADDR0    = (((u32)PKBuf0 + isuPanel.w * 2) & ISU_OffAdr_Mask);
    	            IsuPK_IADDR1    = (((u32)PKBuf1 + isuPanel.w * 2) & ISU_OffAdr_Mask);
    	            IsuPK_IADDR2    = (((u32)PKBuf2 + isuPanel.w * 2) & ISU_OffAdr_Mask);
                #endif
                }
                else //Pannel-out
                {
                #if( (LCM_OPTION == LCM_CCIR601_640x480P)|| (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272)|| NEW_IDU_BRI )
    	            IsuPK_IADDR0    = (((u32)PKBuf0 + isuPanel.w * 2) & ISU_OffAdr_Mask);
    	            IsuPK_IADDR1    = (((u32)PKBuf1 + isuPanel.w * 2) & ISU_OffAdr_Mask);
    	            IsuPK_IADDR2    = (((u32)PKBuf2 + isuPanel.w * 2) & ISU_OffAdr_Mask);
                #else
                   #if DROP_BOTTOMFIELD
                    IsuSCA_FUN  |= (ISU_PKOUT_ENA);
                   #endif
                #endif
                }

                //0x0060
                if((mpeg4Width == 320) || (mpeg4Width == 352) ) //QVGA
                {
                    IsuSCA_FUN  &= (~ISU_PNOUT_ENA);
                }
                else
                {
                    IsuPN_YIADDR0   = (((u32)PNBuf_Y[isu_avifrmcnt & 0x03] + isuPacket.w)& ISU_OffAdr_Mask);
                    IsuPN_CIADDR0   = (((u32)PNBuf_C[isu_avifrmcnt & 0x03] + isuPacket.w)& ISU_OffAdr_Mask);
                #if ISU_OVERLAY_ENABLE
                  #if(ISU_OVL_TYPE == ISU_OVL_TYPE_16Bit)
                    IsuOVL_IADDR    = (u32)ScalarOverlayImage + 1280;
                  #elif(ISU_OVL_TYPE == ISU_OVL_TYPE_1Bit)
                    IsuOVL_IADDR    = (u32)ScalarOverlayImage + 80;
                  #elif(ISU_OVL_TYPE == ISU_OVL_TYPE_2Bit)
                    IsuOVL_IADDR    = (u32)ScalarOverlayImage + 160;
                  #endif
                #endif
                }
    	    }
            else //bottom field
            {   // even field, bottom field.
                if(sysTVOutOnFlag) //TV-out
                {
                #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_QVGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
                  #if DROP_BOTTOMFIELD
                     IsuSCA_FUN  &= (~(ISU_PKOUT_ENA));
                  #endif
                #endif
    	            //0x0050
    	            IsuPK_IADDR0    = (((u32)PKBuf0) & ISU_OffAdr_Mask);
    	            IsuPK_IADDR1    = (((u32)PKBuf1) & ISU_OffAdr_Mask);
    	            IsuPK_IADDR2    = (((u32)PKBuf2) & ISU_OffAdr_Mask);

                }
                else //Pannel-out
                {
                #if( (LCM_OPTION == LCM_CCIR601_640x480P) || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272)|| NEW_IDU_BRI)
    	            //0x0050
    	            IsuPK_IADDR0    = (((u32)PKBuf0) & ISU_OffAdr_Mask);
    	            IsuPK_IADDR1    = (((u32)PKBuf1) & ISU_OffAdr_Mask);
    	            IsuPK_IADDR2    = (((u32)PKBuf2) & ISU_OffAdr_Mask);
                #else
                   #if DROP_BOTTOMFIELD
                    IsuSCA_FUN  &= (~(ISU_PKOUT_ENA));
                   #endif
                #endif

                    if(sysVideoInCHsel == 0x00)
                    {
                    #if ISUCIU_PREVIEW_PNOUT
                        IduVidBuf0Addr=(((u32)PNBuf_Y[isu_avifrmcnt & 0x03])& ISU_OffAdr_Mask);
                        #if NEW_IDU_BRI
                            BRI_IADDR_Y = IduVidBuf0Addr;
                            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                        #endif
                    #else
                        IduWinCtrl = (IduWinCtrl & ~0x00003000) | ((isu_idufrmcnt % 3) << 12);
                        #if NEW_IDU_BRI
                            switch( (isu_idufrmcnt) % 3)
                            {
                               case 0:
                                  BRI_IADDR_Y = IduVidBuf0Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                               case 1:
                                  BRI_IADDR_Y = IduVidBuf1Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                               case 2:
                                  BRI_IADDR_Y = IduVidBuf2Addr;
                                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                                  break;
                            }
                        #endif
                    #endif
                    }


                #if((LCM_OPTION == LCM_HX8312)||(LCM_OPTION == LCM_TG200Q04)||(LCM_OPTION == LCM_TM024HDH29))  //on LCM, PlaybackMode is SINGLE mode, otherwise..
                    idu_switch();
                #endif
                }

                isu_idufrmcnt++;
                IsuSCA_MODE = (IsuSCA_MODE & (~(ISU_PKWSEL_MASK|ISU_PNWSEL_MASK))) | ((isu_idufrmcnt % 3) << ISU_PKWSEL_SHFT);

               if((mpeg4Width == 320) || (mpeg4Width == 352) ) //QVGA
               {
                    IsuSCA_FUN  |= (ISU_PNOUT_ENA);
               }

               IsuPN_YIADDR0 = (((u32)PNBuf_Y[ (isu_avifrmcnt +1) & 0x03])& ISU_OffAdr_Mask); //Lucian: 用四個frame buffer.
               IsuPN_CIADDR0 = (((u32)PNBuf_C[ (isu_avifrmcnt +1) & 0x03])& ISU_OffAdr_Mask);

            #if ISU_OVERLAY_ENABLE
               IsuOVL_IADDR    = (u32)ScalarOverlayImage;
            #endif
            }
        }
        //----------Preview mode------------//
        else if (siuOpMode == SIUMODE_PREVIEW)
        {
          #if( (Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV) )
          #else //656
              FieldTypeID=(SiuSyncStat>>31) & 0x01;
          #endif
            if ( (FieldTypeID & 0x01) == DEF_TOP_FIELD)
            {    // odd field, Top field.
                if(sysTVOutOnFlag) //TV-out
                {
                #if ( ISUCIU_PREVIEW_PNOUT  )
                    /*
                    IsuPN_YIADDR0 = (((u32)PKBuf0+ isuPanel.w)& ISU_OffAdr_Mask);
                    IsuPN_CIADDR0 = (((u32)PKBuf0+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                    IsuPN_YIADDR1 = (((u32)PKBuf1+ isuPanel.w)& ISU_OffAdr_Mask);
                    IsuPN_CIADDR1 = (((u32)PKBuf1+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                    IsuPN_YIADDR2 = (((u32)PKBuf2+ isuPanel.w)& ISU_OffAdr_Mask);
                    IsuPN_CIADDR2 = (((u32)PKBuf2+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                    */
                    IsuPN_YIADDR0   = (((u32)PNBuf_Y[isu_idufrmcnt & 0x03] + isuPanel.w)& ISU_OffAdr_Mask);
                    IsuPN_CIADDR0   = (((u32)PNBuf_C[isu_idufrmcnt & 0x03] + isuPanel.w)& ISU_OffAdr_Mask);
                #else
                    IsuPK_IADDR0    = (((u32)PKBuf0 + isuPanel.w * 2) & ISU_OffAdr_Mask);
    	            IsuPK_IADDR1    = (((u32)PKBuf1 + isuPanel.w * 2) & ISU_OffAdr_Mask);
    	            IsuPK_IADDR2    = (((u32)PKBuf2 + isuPanel.w * 2) & ISU_OffAdr_Mask);
                #endif
                }
                else //Pannel-out
                {
                #if ( (LCM_OPTION == LCM_CCIR601_640x480P) || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272)|| (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) || NEW_IDU_BRI ) //Top/Bottom field 組合輸出.
                       #if ( ISUCIU_PREVIEW_PNOUT  )
                        /*
                        IsuPN_YIADDR0 = (((u32)PKBuf0+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR0 = (((u32)PKBuf0+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_YIADDR1 = (((u32)PKBuf1+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR1 = (((u32)PKBuf1+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_YIADDR2 = (((u32)PKBuf2+ isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR2 = (((u32)PKBuf2+PNBUF_SIZE_Y+ isuPanel.w)& ISU_OffAdr_Mask);
                        */
                        IsuPN_YIADDR0   = (((u32)PNBuf_Y[isu_idufrmcnt & 0x03] + isuPanel.w)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR0   = (((u32)PNBuf_C[isu_idufrmcnt & 0x03] + isuPanel.w)& ISU_OffAdr_Mask);
                       #else
                        IsuPK_IADDR0    = (((u32)PKBuf0 + isuPanel.w * 2) & ISU_OffAdr_Mask);
        	            IsuPK_IADDR1    = (((u32)PKBuf1 + isuPanel.w * 2) & ISU_OffAdr_Mask);
        	            IsuPK_IADDR2    = (((u32)PKBuf2 + isuPanel.w * 2) & ISU_OffAdr_Mask);
                       #endif
                #else
                    #if DROP_BOTTOMFIELD
                      #if ( ISUCIU_PREVIEW_PNOUT  )
                         IsuSCA_FUN  |= (ISU_PNOUT_ENA);
                      #else
                         IsuSCA_FUN  |= (ISU_PKOUT_ENA);
                      #endif
                    #endif
                #endif

                }
    	    }
            else
            {   // even field, bottom field.

                if(sysTVOutOnFlag) //TV-out
                {

                #if ( ISUCIU_PREVIEW_PNOUT  )
                    /*
                    IsuPN_YIADDR0 = (((u32)PKBuf0)& ISU_OffAdr_Mask);
                    IsuPN_CIADDR0 = (((u32)PKBuf0+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                    IsuPN_YIADDR1 = (((u32)PKBuf1)& ISU_OffAdr_Mask);
                    IsuPN_CIADDR1 = (((u32)PKBuf1+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                    IsuPN_YIADDR2 = (((u32)PKBuf2)& ISU_OffAdr_Mask);
                    IsuPN_CIADDR2 = (((u32)PKBuf2+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                    */
                #else
                    IsuPK_IADDR0    = (((u32)PKBuf0) & ISU_OffAdr_Mask);
                    IsuPK_IADDR1    = (((u32)PKBuf1) & ISU_OffAdr_Mask);
                    IsuPK_IADDR2    = (((u32)PKBuf2) & ISU_OffAdr_Mask);
                #endif
                }
                else //Pannel-out
                {
                #if ISUCIU_PREVIEW_PNOUT
                    IduWinCtrl = (IduWinCtrl & ~0x00003000);
                    IduVidBuf0Addr=(((u32)PNBuf_Y[isu_idufrmcnt & 0x03])& ISU_OffAdr_Mask);
                    #if NEW_IDU_BRI
                        BRI_IADDR_Y = IduVidBuf0Addr;
                        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                    #endif
                #else
                    IduWinCtrl = (IduWinCtrl & ~0x00003000) | ((isu_idufrmcnt % 3) << 12);
                    #if NEW_IDU_BRI
                    switch( (isu_idufrmcnt) % 3)
                    {
                       case 0:
                          BRI_IADDR_Y = IduVidBuf0Addr;
                          BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                          break;
                       case 1:
                          BRI_IADDR_Y = IduVidBuf1Addr;
                          BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                          break;
                       case 2:
                          BRI_IADDR_Y = IduVidBuf2Addr;
                          BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                          break;
                    }
                    #endif
                #endif

                #if ( (LCM_OPTION == LCM_CCIR601_640x480P) || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272)|| (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) || NEW_IDU_BRI) //for big pannel, Two field
                   #if ( ISUCIU_PREVIEW_PNOUT  )
                        /*
                        IsuPN_YIADDR0 = (((u32)PKBuf0)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR0 = (((u32)PKBuf0+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                        IsuPN_YIADDR1 = (((u32)PKBuf1)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR1 = (((u32)PKBuf1+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                        IsuPN_YIADDR2 = (((u32)PKBuf2)& ISU_OffAdr_Mask);
                        IsuPN_CIADDR2 = (((u32)PKBuf2+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
                        */
                   #else
                        IsuPK_IADDR0    = (((u32)PKBuf0) & ISU_OffAdr_Mask);
                        IsuPK_IADDR1    = (((u32)PKBuf1) & ISU_OffAdr_Mask);
                        IsuPK_IADDR2    = (((u32)PKBuf2) & ISU_OffAdr_Mask);
                   #endif
                #else //for small pannel: only one field reserved
                    #if DROP_BOTTOMFIELD
                       #if ( ISUCIU_PREVIEW_PNOUT  )
                          IsuSCA_FUN  &= (~ISU_PNOUT_ENA);
                       #else
                          IsuSCA_FUN  &= (~ISU_PKOUT_ENA);
                       #endif
                    #endif
                #endif

                #if((LCM_OPTION == LCM_HX8312)||(LCM_OPTION == LCM_TG200Q04)||(LCM_OPTION == LCM_TM024HDH29))  //on LCM, PlaybackMode is SINGLE mode, otherwise..
                    idu_switch();
                #endif
                }

                isu_idufrmcnt++;

            #if ( ISUCIU_PREVIEW_PNOUT  )
                IsuSCA_MODE = (IsuSCA_MODE & ~ISU_PNWSEL_MASK);
                IsuPN_YIADDR0   = (((u32)PNBuf_Y[isu_idufrmcnt & 0x03] )& ISU_OffAdr_Mask);
                IsuPN_CIADDR0   = (((u32)PNBuf_C[isu_idufrmcnt & 0x03] )& ISU_OffAdr_Mask);
            #else
                IsuSCA_MODE = (IsuSCA_MODE & ~ISU_PKWSEL_MASK) | ((isu_idufrmcnt % 3) << ISU_PKWSEL_SHFT);
            #endif
            }

        }
    }

    void isuOutputAddrArrange_Sensor(void)
    {
        //-------------------------------AVI mode---------------------------//
        //Lucian: 解決錄影時(頻寬不足,造成ISR延遲)畫面跳動問題.
        //printf("--a--\n");
        if ( (siuOpMode == SIUMODE_MPEGAVI) && (sysCameraMode == SYS_CAMERA_MODE_PREVIEW) )
        {
            #if((LCM_OPTION == LCM_HX8312)||(LCM_OPTION == LCM_TG200Q04)||(LCM_OPTION == LCM_TM024HDH29))  //on LCM, PlaybackMode is SINGLE mode, otherwise..
            if(!sysTVOutOnFlag)
                idu_switch();
            #endif

            IsuSCA_MODE = (IsuSCA_MODE & ~0x3c00) | (((isu_idufrmcnt+1) % 3) << 10);
            IsuPN_YIADDR0 = (((u32)PNBuf_Y[(isu_avifrmcnt+1) & 0x03])& ISU_OffAdr_Mask); //Lucian: 用四個frame buffer.
            IsuPN_CIADDR0 = (((u32)PNBuf_C[(isu_avifrmcnt+1) & 0x03])& ISU_OffAdr_Mask);

        }
    }


    s32 isuScUpZoom(s32 zoomFactor)
    {
       u32 W,H;

       getPreviewZoomSize(zoomFactor,&W,&H);

       IsuScUpInWindow   = (W <<ISU_SCUP_SRCWIN_W_SHFT) | (H <<ISU_SCUP_SRCWIN_H_SHFT);

       if( (isuSrcImg.w >= W) && (isuSrcImg.h >= H) )
         IsuScUpStartPos   = ( ((isuSrcImg.w-W)/2)<<ISU_SCUP_SRCSTART_X_SHFT ) | ( ((isuSrcImg.h-H)/2)<<ISU_SCUP_SRCSTART_Y_SHFT );
       else
         IsuScUpStartPos=0;
    }

    /*

    Routine Description:

        Preview.

    Arguments:

        zoomFacor - Zoom factor.

    Return Value:

        0 - Failure.
        1 - Success.

    */

    s32 isuPreview(s8 zoomFactor)
    {
        u16 phaseh, phasew;

        // Reset ISU module
        isuRst();

        // Set input image size
        MSFSz.inh = isuSrcImg.h;
        MSFSz.inw = isuSrcImg.w;

#if( SIU_SCUP_EN )
        IsuScUpInWindow   = (MSFSz.inw <<ISU_SCUP_SRCWIN_W_SHFT) | (MSFSz.inh <<ISU_SCUP_SRCWIN_H_SHFT);
        IsuScUpOutWindow  = (MSFSz.inw <<ISU_SCUP_OUTWIN_W_SHFT) | (MSFSz.inh <<ISU_SCUP_OUTWIN_H_SHFT);
        IsuScUpStartPos   = 0;
        IsuScUpFIFOCntr   = 0x20161610;
       #if ISUCIU_PREVIEW_PNOUT
        IsuScUpCntr       = ISU_SCUP_EN | (MSFSz.inw << ISU_SCUP_LINESIZE_SHFT) | ISU_SCUP_WAIT_PNY | ISU_SCUP_WAIT_PNC;
       #else
        IsuScUpCntr       = ISU_SCUP_EN | (MSFSz.inw << ISU_SCUP_LINESIZE_SHFT) | ISU_SCUP_WAIT_PK;
       #endif
#endif

        // Display image size (package output size)
        if(sysTVOutOnFlag) //TV-out
        {
               if(TvOutMode==SYS_TV_OUT_PAL) //PAL mode
               {
                    if( sysVideoInSel == VIDEO_IN_TV) //TV-in
                    {
                        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                        {
                           isuPanel.w    = 640;
                           isuPanel.h    = 480 / 2;
                        }
                        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) )
                        {
                           isuPanel.w    = 704;
                           isuPanel.h    = 576 / 2;
                        }
                    }
                    else //Sensor-in
                    {
                        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                        {
                            isuPanel.w    = 1280;
                            isuPanel.h    = 720;
                        }
                        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                        {
                            isuPanel.w    = 640;
                            isuPanel.h    = 480;
                        }
                        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
                        {
                            isuPanel.w    = 640;
                            isuPanel.h    = 352;
                        }
                        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
                        {
                            isuPanel.w    = 704;
                            isuPanel.h    = 480;
                        }
                        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
                        {
                            isuPanel.w    = 1920;
                            isuPanel.h    = 1080;
                        }
                        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
                        {
                            isuPanel.w    = 1600;
                            isuPanel.h    = 896;
                        }
                    }
                    IsuOutAreaFlagOnTV = FULL_TV_OUT;
               }
               else //NTSC mode
               {
                    if( sysVideoInSel == VIDEO_IN_TV) //TV-in
                    {
                        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                        {
                           isuPanel.w    = 640;
                           isuPanel.h    = 480 / 2;
                        }
                        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) )
                        {
                           isuPanel.w    = 704;
                           isuPanel.h    = 480 / 2;
                        }
                    }
                    else //Sensor-in
                    {
                        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                        {
                            isuPanel.w    = 1280;
                            isuPanel.h    = 720;
                        }
                        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                        {
                            isuPanel.w    = 640;
                            isuPanel.h    = 480;
                        }
                        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
                        {
                            isuPanel.w    = 640;
                            isuPanel.h    = 352;
                        }
                        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
                        {
                            isuPanel.w    = 704;
                            isuPanel.h    = 480;
                        }
                        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
                        {
                            isuPanel.w    = 1920;
                            isuPanel.h    = 1080;
                        }
                        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
                        {
                            isuPanel.w    = 1600;
                            isuPanel.h    = 896;
                        }
                    }
                    IsuOutAreaFlagOnTV = FULL_TV_OUT;
              }
        }
        else //Pannel-out
        {
            if( sysVideoInSel == VIDEO_IN_TV) //TV-in
            {
                if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                {
                   isuPanel.w    = 640;
                   isuPanel.h    = 480 / 2;
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) )
                {
                   isuPanel.w    = 704;
                   isuPanel.h    = 480 / 2;
                }
            }
            else //Sensor-in
            {
                if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                {
                    isuPanel.w    = 1280;
                    isuPanel.h    = 720;
                }
                else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                {
                    isuPanel.w    = 640;
                    isuPanel.h    = 480;
                }
                else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
                {
                    isuPanel.w    = 640;
                    isuPanel.h    = 352;
                }
                else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
                {
                    isuPanel.w    = 704;
                    isuPanel.h    = 480;
                }
                else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
                {
                    isuPanel.w    = 1920;
                    isuPanel.h    = 1080;
                }
                else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
                {
                    isuPanel.w    = 1600;
                    isuPanel.h    = 896;
                }
            }
        }
        /*BJ 0821 E*/

        // Output image size (panel output size)
        MSFSz.outw = isuPanel.w;
        MSFSz.outh = isuPanel.h;

        MSFCrs.stepw = isuSize2Step(isuPanel.w, isuSrcImg.w, &phasew);
        MSFCrs.steph = isuSize2Step(isuPanel.h, isuSrcImg.h, &phaseh);

        MSFCrs.phasew = phasew;
        MSFCrs.phaseh = phaseh;

        // 0x0004
        if(sysTVOutOnFlag)
        {
            IsuSCA_MODE   =
                        ISU_Manual_Mode |    // Lucian: Use manual mode for Preview mode
                        ISU_PROC_LBMode |    // Line buffer processing mode
                        ISU_SRC_IPU     |    // Source data from IPU
                        ISU_TRIPLE_Trip |    // Use triple frame buffer
                        ISU_BLKSEL_8L|
                        ISU_FBR_SEL_0|       //
                        ISU_PKW_SEL_0|
                        ISU_PNW_SEL_0;
        }
        else
        {
             IsuSCA_MODE  =
                        ISU_Manual_Mode |   // Lucian: Use manual mode for Preview mode
                        ISU_PROC_LBMode|    // Line buffer processing mode
                        ISU_SRC_IPU|        // Source data from IPU
                        ISU_TRIPLE_Trip|    // Use triple frame buffer
                        ISU_BLKSEL_8L|
        #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                        ISU_PKSRGB |
        #endif
                        ISU_FBR_SEL_0|      //
                        ISU_PKW_SEL_0|
                        ISU_PNW_SEL_0;

        }
        // 0x0008
        #if ( ISUCIU_PREVIEW_PNOUT )
        IsuSCA_FUN =    ISU_PKOUT_DISA |
                        ISU_PNOUT_ENA |
                        ISU_OVLEN_DISA |
                        ISU_OVLSEL_MAIN|
                        ISU_MSFEN_ENA|
                        ISU_SSFEN_DISA;
        #else
        IsuSCA_FUN =    ISU_PKOUT_ENA|
                        ISU_PNOUT_DISA|
                        ISU_OVLEN_DISA|
                        ISU_OVLSEL_MAIN|
                        ISU_MSFEN_ENA|
                        ISU_SSFEN_DISA;
        #endif
        // 0x000C
        IsuSCA_INTC =
                        ISU_FCINTE_ENA|
                        ISU_PKOINTE_DISA|
                        ISU_PYOINTE_DISA|
                        ISU_PCOINTE_DISA|
                        ISU_SRUINTE_DISA;

        //0x0020: 此時無作用.
        IsuSRC_SIZE = (MSFSz.inw<<ISU_SRC_W_SHFT) + (MSFSz.inh<<ISU_SRC_H_SHFT);

        //0x0024
        IsuPK_SIZE = (MSFSz.outw << ISU_PK_W_SHFT) + (MSFSz.outh << ISU_PK_H_SHFT);

        //0x0028
        IsuPN_SIZE = (MSFSz.outw << ISU_PN_W_SHFT) + (MSFSz.outh << ISU_PN_H_SHFT);;

        // 0x0030
        IsuMSF_STEP= (MSFCrs.stepw << ISU_MSTEPX_SHFT) + (MSFCrs.steph << ISU_MSTEPY_SHFT);

        //0x0034
        IsuMSF_PHA = (MSFCrs.phasew << ISU_MPHASEX_SHFT) + (MSFCrs.phaseh << ISU_MPHASEY_SHFT);

        //0x0040: 目前無作用.
        IsuSRC_IADDR0 = (u32)ipuDstBuf0;
        IsuSRC_IADDR1 = (u32)ipuDstBuf1;
        IsuSRC_IADDR2 = (u32)ipuDstBuf2;

        //0x0050
        IsuPK_IADDR0 = (((u32)PKBuf0)& ISU_OffAdr_Mask);
        IsuPK_IADDR1 = (((u32)PKBuf1)& ISU_OffAdr_Mask);
        IsuPK_IADDR2 = (((u32)PKBuf2)& ISU_OffAdr_Mask);

        //0x0060
        IsuPN_YIADDR0 = (((u32)PKBuf0)& ISU_OffAdr_Mask);
        IsuPN_CIADDR0 = (((u32)PKBuf0+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
        IsuPN_YIADDR1 = (((u32)PKBuf1)& ISU_OffAdr_Mask);
        IsuPN_CIADDR1 = (((u32)PKBuf1+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);
        IsuPN_YIADDR2 = (((u32)PKBuf2)& ISU_OffAdr_Mask);
        IsuPN_CIADDR2 = (((u32)PKBuf2+PNBUF_SIZE_Y)& ISU_OffAdr_Mask);

        //0x004C: 目前無作用
        IsuSRC_STRIDE = MSFSz.inw * 2;

        //0x005C(PK buffer)
        if(sysTVOutOnFlag) //TV-out
        {
           if( sysVideoInSel == VIDEO_IN_TV) //TV-in
           {
              IsuPK_STRIDE = (MSFSz.outw) * 2 * 2; //for interlace
           }
           else //Sensor-in
              IsuPK_STRIDE = (MSFSz.outw) * 2;
        }
        else //Pannel-out
        {
            if( sysVideoInSel == VIDEO_IN_TV) //TV-in
            {
                IsuPK_STRIDE = (MSFSz.outw) * 2 * 2;
            }
            else
                IsuPK_STRIDE = (MSFSz.outw) * 2;
        }

        //0x0078(PN buffer)
        if(sysTVOutOnFlag) //TV-out
        {
            if( sysVideoInSel == VIDEO_IN_TV) //TV-in
            {
                IsuPN_STRIDE = ((MSFSz.outw * 2) << ISU_PNCSTRIDE_SHFT) | ((MSFSz.outw * 2) << ISU_PNYSTRIDE_SHFT);
            }
            else //sensor-in
            {
                IsuPN_STRIDE = (MSFSz.outw<<ISU_PNCSTRIDE_SHFT) |(MSFSz.outw<<ISU_PNYSTRIDE_SHFT);
            }
        }
        else //Pannel out
        {
            if( sysVideoInSel == VIDEO_IN_TV) //TV-in
            {
                 IsuPN_STRIDE = ((MSFSz.outw * 2) << ISU_PNCSTRIDE_SHFT) | ((MSFSz.outw * 2) << ISU_PNYSTRIDE_SHFT);
            }
            else //sensor-in
            {
                IsuPN_STRIDE = (MSFSz.outw<<ISU_PNCSTRIDE_SHFT) |(MSFSz.outw<<ISU_PNYSTRIDE_SHFT);
            }
        }

        //Keep Burst Request for Packet Channel:Disable
        //Reason:sensor will be overflow, scaler will be underrun
        IsuFIFO_TH = 0x5377777f;  //bit3: improve performace @9003

        isuRst();
        isu_idufrmcnt=0;
        // 0x00000000
        IsuSCA_EN = ISU_ENA | ISU_Conti_ENA;

        return 1;
    }

    s32 isuCapturePreviewImg(s8 zoomFactor)
    {   //Lucian: Only Capture VGA(640x480) size.
        u16 phaseh, phasew;

        // Reset ISU module
        isuRst();

        // Set input image size
        MSFSz.inw = isuSrcImg.w;
        MSFSz.inh = isuSrcImg.h;

        if( sysVideoInSel == VIDEO_IN_TV) //TV-in
        {
            isuPanel.w    = 640;
            isuPanel.h    = 480 / 2;
        }
        else
        {
            isuPanel.w    = 640;
            isuPanel.h    = 480;
        }
        /*BJ 0821 E*/

        //Output image size (panel output size)
        MSFSz.outh = isuPanel.h;
        MSFSz.outw = isuPanel.w;

        MSFCrs.steph = isuSize2Step(isuPanel.h, isuSrcImg.h, &phaseh);
        MSFCrs.stepw = isuSize2Step(isuPanel.w, isuSrcImg.w, &phasew);

        MSFCrs.phaseh = phaseh;
        MSFCrs.phasew = phasew;

        // 0x0004
        IsuSCA_MODE   =
                        //ISU_Auto_Mode0|   // frame to frame processing for preview
                        ISU_Manual_Mode |    // Lucian: Use manual mode for Preview mode
                        ISU_PROC_LBMode|    // Line buffer processing mode
                        ISU_SRC_IPU|        // Source data from IPU
                        ISU_TRIPLE_Trip|    // Use triple frame buffer
                        ISU_BLKSEL_8L|
                        ISU_OVL_TYPE|
                        ISU_OVL_OPA|
                        ISU_FBR_SEL_0|      //
                        ISU_PKW_SEL_0|
                        ISU_PNW_SEL_0;

        // 0x0008
    	IsuSCA_FUN =	ISU_PKOUT_ENA|
    					ISU_PNOUT_DISA|
    					ISU_OVLEN_DISA|
    					ISU_OVLSEL_MAIN|
    					ISU_MSFEN_ENA|
    					ISU_SSFEN_DISA;
        // 0x000C
        IsuSCA_INTC =   ISU_FCINTE_ENA|
                        ISU_PKOINTE_DISA|   //Packet out FIFO overflow interrupt enable
                        ISU_PYOINTE_DISA|   //Planer_Y
                        ISU_PCOINTE_DISA|   //Planer_CbCr
                        ISU_SRUINTE_DISA;   //Source/overlay FIFO under-run
        //0x0020
        IsuSRC_SIZE = (MSFSz.inw<<ISU_SRC_W_SHFT) + (MSFSz.inh<<ISU_SRC_H_SHFT);

        //0x0024
        IsuPK_SIZE = (MSFSz.outw << ISU_PK_W_SHFT) + (MSFSz.outh << ISU_PK_H_SHFT);

        //0x0028
        IsuPN_SIZE = 0x00000000;

        // 0x0030
        //IsuMSF_STEP = 0x0a000a00; //0x09f409f4;
        IsuMSF_STEP= (MSFCrs.stepw << ISU_MSTEPX_SHFT) + (MSFCrs.steph << ISU_MSTEPY_SHFT);

        //0x0034
        IsuMSF_PHA = (MSFCrs.phasew << ISU_MPHASEX_SHFT) + (MSFCrs.phaseh << ISU_MPHASEY_SHFT);

        //0x0040
        IsuSRC_IADDR0 = (u32)ipuDstBuf0;
        IsuSRC_IADDR1 = (u32)ipuDstBuf1;
        IsuSRC_IADDR2 = (u32)ipuDstBuf2;

        //0x0050
        IsuPK_IADDR0 = (((u32)PKBuf0)& ISU_OffAdr_Mask);
        IsuPK_IADDR1 = (((u32)PKBuf1)& ISU_OffAdr_Mask);
        IsuPK_IADDR2 = (((u32)PKBuf2)& ISU_OffAdr_Mask);


        //0x004C
        IsuSRC_STRIDE = MSFSz.inw*2;
        //0x005C

        if( sysVideoInSel == VIDEO_IN_TV) //TV-in
            IsuPK_STRIDE = (MSFSz.outw) * 2 * 2; //for interlace
        else //Sensor-in
            IsuPK_STRIDE = (MSFSz.outw) * 2;


        //Keep Burst Request for Packet Channel:Disable
        //Reason:sensor will be overflow, scaler will be underrun
        IsuFIFO_TH = 0x5377777f;

        isuRst();
            isu_idufrmcnt=0;
        // 0x00000000
        IsuSCA_EN = ISU_ENA | ISU_Conti_ENA;

        return 1;
    }


     //Note: 只以PA9002D 用此一functon.(Lucian)
    s32 isuPreviewZoom(s8 zoomFactor)
    {
        u16 phaseh, phasew;

        // Reset ISU module
        isuRst();

        // Set input image size
        MSFSz.inh = isuSrcImg.h;
        MSFSz.inw = isuSrcImg.w;

        // Display image size (package output size)
        /*BJ 0821 S*/
        if(sysTVOutOnFlag) //TV-out
        {
               if(TvOutMode==SYS_TV_OUT_PAL) //PAL mode
               {
                #ifdef TV_WITH_PAL_FORCE2QVGA
                    isuPanel.h    = 288; // 576/2=288
                    isuPanel.w    = TVOUT_X/2;
                    IsuOutAreaFlagOnTV = HALF_TV_OUT;
                #else

                    if( sysVideoInSel == VIDEO_IN_TV) //TV-in
                    {
                        isuPanel.h    = TVOUT_Y_PAL / 2;
                        isuPanel.w    = TVOUT_X;
                    }
                    else //Sensor-in
                    {
                        isuPanel.h    = 480;
                        isuPanel.w    = TVOUT_X;
                    }
                    IsuOutAreaFlagOnTV = FULL_TV_OUT;

                #endif
               }
               else //NTSC mode
               {
                    if( sysVideoInSel == VIDEO_IN_TV) //TV-in
                    {
                        isuPanel.h    = TVOUT_Y_NTSC / 2;
                        isuPanel.w    = TVOUT_X;
                    }
                    else //Sensor-in
                    {
                        isuPanel.h    = TVOUT_Y_NTSC;
                        isuPanel.w    = TVOUT_X;
                    }
                    IsuOutAreaFlagOnTV = FULL_TV_OUT;
              }
        }
        else //Pannel-out
        {
            if( sysVideoInSel == VIDEO_IN_TV) //TV-in
            {
                #if( (LCM_OPTION == LCM_CCIR601_640x480P) || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
                    isuPanel.h    = PANNEL_Y / 2;
                    isuPanel.w    = PANNEL_X;
                #else
                    isuPanel.h    = PANNEL_Y;
                    isuPanel.w    = PANNEL_X;
                #endif
            }
            else //Sensor-in
            {
                    isuPanel.h    = PANNEL_Y;
                    isuPanel.w    = PANNEL_X;
            }
        }
        // Output image size (panel output size)
        MSFSz.outh = isuPanel.h;
        MSFSz.outw = isuPanel.w;

        MSFCrs.steph = isuSize2Step(isuPanel.h, isuSrcImg.h, &phaseh);
        MSFCrs.stepw = isuSize2Step(isuPanel.w, isuSrcImg.w, &phasew);

        MSFCrs.phaseh = phaseh;
        MSFCrs.phasew = phasew;

        // 0x0004
        if(sysTVOutOnFlag)
        {
            IsuSCA_MODE   =
                      //ISU_Auto_Mode0|   // frame to frame processing for preview
                        ISU_Manual_Mode |    // Lucian: Use manual mode for Preview mode
                        ISU_PROC_LBMode|    // Line buffer processing mode
                        ISU_SRC_IPU|        // Source data from IPU
                        ISU_TRIPLE_Trip|    // Use triple frame buffer
                        ISU_BLKSEL_8L|
                        ISU_FBR_SEL_0|      //
                        ISU_PKW_SEL_0|
                        ISU_PNW_SEL_0;
        }
        else
        {
        IsuSCA_MODE   =
                      //ISU_Auto_Mode0|   // frame to frame processing for preview
                        ISU_Manual_Mode |    // Lucian: Use manual mode for Preview mode
                        ISU_PROC_LBMode|    // Line buffer processing mode
                        ISU_SRC_IPU|        // Source data from IPU
                        ISU_TRIPLE_Trip|    // Use triple frame buffer
                        ISU_BLKSEL_8L|
                        ISU_FBR_SEL_0|      //
#if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                        ISU_PKSRGB |
#endif
                        ISU_PKW_SEL_0|
                        ISU_PNW_SEL_0;
        }
        IsuSCA_MODE = (IsuSCA_MODE & ~0x3c00) | ((isu_idufrmcnt % 3) << 10);
        // 0x0008
        IsuSCA_FUN =    ISU_PKOUT_ENA|
                        ISU_PNOUT_DISA|
                        ISU_OVLEN_DISA|
                        ISU_OVLSEL_MAIN|
                        ISU_MSFEN_ENA|
                        ISU_SSFEN_DISA;
        // 0x000C
        IsuSCA_INTC =   ISU_FCINTE_ENA|     //Lucian: Frame complete enable
                        ISU_PKOINTE_DISA|   //Packet out FIFO overflow interrupt enable
                        ISU_PYOINTE_DISA|   //Planer_Y
                        ISU_PCOINTE_DISA|   //Planer_CbCr
                        ISU_SRUINTE_DISA;   //Source/overlay FIFO under-run
        //0x0020
        IsuSRC_SIZE = (MSFSz.inw<<ISU_SRC_W_SHFT) + (MSFSz.inh<<ISU_SRC_H_SHFT);

        //0x0024
        IsuPK_SIZE = (MSFSz.outw << ISU_PK_W_SHFT) + (MSFSz.outh << ISU_PK_H_SHFT);

        //0x0028
        IsuPN_SIZE = 0x00000000;

        // 0x0030
        //IsuMSF_STEP = 0x0a000a00; //0x09f409f4;
        IsuMSF_STEP= (MSFCrs.stepw << ISU_MSTEPX_SHFT) + (MSFCrs.steph << ISU_MSTEPY_SHFT);

        //0x0034
        IsuMSF_PHA = (MSFCrs.phasew << ISU_MPHASEX_SHFT) + (MSFCrs.phaseh << ISU_MPHASEY_SHFT);

        //0x0040
        IsuSRC_IADDR0 = (u32)ipuDstBuf0;
        IsuSRC_IADDR1 = (u32)ipuDstBuf1;
        IsuSRC_IADDR2 = (u32)ipuDstBuf2;

        //0x0050
        if(sysTVOutOnFlag && TvOutMode==SYS_TV_OUT_PAL && isuPanel.h == 480) //PAL mode  //Lsk 090518 : put video frame in center
        {
            IsuPK_IADDR0 = (((u32)(PKBuf0 + 61440))& ISU_OffAdr_Mask);
            IsuPK_IADDR1 = (((u32)(PKBuf1 + 61440))& ISU_OffAdr_Mask);
            IsuPK_IADDR2 = (((u32)(PKBuf2 + 61440))& ISU_OffAdr_Mask);
        }else
        {
            IsuPK_IADDR0 = (((u32)PKBuf0)& ISU_OffAdr_Mask);
            IsuPK_IADDR1 = (((u32)PKBuf1)& ISU_OffAdr_Mask);
            IsuPK_IADDR2 = (((u32)PKBuf2)& ISU_OffAdr_Mask);
        }

        //0x004C
        IsuSRC_STRIDE = MSFSz.inw*2;

        //0x005C
        if(sysTVOutOnFlag) //TV-out
        {
            if( sysVideoInSel == VIDEO_IN_TV) //TV-in
            {
                  /*
                  #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_QVGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) ) //QVGA frames
                    IsuPK_STRIDE = (MSFSz.outw) * 2;
                  */
                  //#else
                    IsuPK_STRIDE = (MSFSz.outw) * 2 * 2; //for interlace
                  //#endif
            }
            else //Sensor-in
                  IsuPK_STRIDE = (MSFSz.outw) * 2;
        }
        else //Pannel-out
        {
        #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
            IsuPK_STRIDE = MSFSz.outw;
        #else
            if( sysVideoInSel == VIDEO_IN_TV) //TV-in
            {
              #if( (LCM_OPTION == LCM_CCIR601_640x480P) || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
                IsuPK_STRIDE = (MSFSz.outw) * 2 * 2;
              #else
                IsuPK_STRIDE = (MSFSz.outw) * 2;
              #endif
            }
            else
                IsuPK_STRIDE = (MSFSz.outw) * 2;
         #endif
        }

        IsuFIFO_TH = 0x5377777f;
        isuRst();
        // 0x00000000
        IsuSCA_EN = ISU_ENA | ISU_Conti_ENA;

        return 1;
    }


    u16 isuSize2Step(u16 dst_size, u16 src_size, u16 *phase)
    {
        u32 step;

        step  = (((u32) src_size)<<10) / (u32) dst_size;  //step = src/dst
        if(step > 0x1fff)
            step    = 0x1fff;
        *phase = 0;
        return (u16) step;
    }

    /*
    Routine Description:

        Capture primary.

    Arguments:

        None.

    Return Value:

        0 - Failure.
        1 - Success.

    */

    s32 isuCapture640x480_B2F(s32 zoomFactor) /*BJ 0530 S*/
    {
        u16 phaseh, phasew;
        u16 width, height;
        /* scale yuv data */

        isuRst();
        MSFSz.inh = isuSrcImg.h;       //sensor_validsize.imgSize.x-4
        MSFSz.inw = isuSrcImg.w;

        //isuGetImgOutResolution(&width, &height);

        MSFSz.outh = 480;//height;
        MSFSz.outw = 640;//width;

        MSFCrs.steph = isuSize2Step(MSFSz.outh, isuSrcImg.h, &phaseh);
        MSFCrs.stepw = isuSize2Step(MSFSz.outw, isuSrcImg.w, &phasew);

        MSFCrs.phaseh = phaseh;
        MSFCrs.phasew = phasew;

        IsuSCA_MODE   =ISU_Auto_Mode2|      // block to frame processing for preview
                                        ISU_PROC_2DMode|    // SDRAM buffer processing mode
                                        ISU_SRC_DRAM|       // Source data from DRAM
                                        ISU_TRIPLE_Trip|    // Use triple frame buffer
                                        ISU_BLKSEL_16L|     // Block Mode Lines Selection: 16 Lines
                                        ISU_FBR_SEL_0|      //
                                        ISU_PKW_SEL_0|
                                        ISU_PNW_SEL_0;

        IsuSCA_FUN = ISU_PKOUT_ENA|
                        ISU_PNOUT_DISA|
                        ISU_OVLEN_DISA|
                        ISU_OVLSEL_MAIN|
                        ISU_MSFEN_ENA|
                        ISU_SSFEN_DISA;
        // 0x000C
        IsuSCA_INTC =ISU_FCINTE_ENA|     //
                        ISU_PKOINTE_DISA|
                        ISU_PYOINTE_DISA|
                        ISU_PCOINTE_DISA|
                        ISU_SRUINTE_DISA;

        //0x0020
        IsuSRC_SIZE = (MSFSz.inw<<ISU_SRC_W_SHFT) + (MSFSz.inh<<ISU_SRC_H_SHFT);

        //0x0024
        IsuPK_SIZE = (MSFSz.outw << ISU_PK_W_SHFT) + (MSFSz.outh << ISU_PK_H_SHFT);

        //0x0028
        IsuPN_SIZE = 0x00000000;

        // 0x0030
        IsuMSF_STEP= (MSFCrs.stepw << ISU_MSTEPX_SHFT) + (MSFCrs.steph << ISU_MSTEPY_SHFT);

        //0x0034
        IsuMSF_PHA = (MSFCrs.phasew << ISU_MPHASEX_SHFT) + (MSFCrs.phaseh << ISU_MPHASEY_SHFT);

        IsuSRC_IADDR0 = (u32)ipuDstBuf0;
        IsuSRC_IADDR1 = (u32)ipuDstBuf1;
        IsuSRC_IADDR2 = (u32)ipuDstBuf2;
        IsuSRC_STRIDE = MSFSz.inw<<1;

        IsuPK_IADDR0 = (u32)PKBuf;
        IsuPK_IADDR1 = (u32)PKBuf1;
        IsuPK_IADDR2 = (u32)PKBuf2;
        IsuPK_STRIDE = MSFSz.outw<<1;

        IsuFIFO_TH = 0x5377777f;

        isuRst();

        // 0x00000000
        IsuSCA_EN = ISU_ENA | ISU_Single_ENA;

        return 1;
    }

    s32 isuCapturePrimary_B2F(s32 zoomFactor) /*BJ 0530 S*/
    {
        u16 phaseh, phasew;
        u16 width, height;
        /* scale yuv data */

        isuRst();
        MSFSz.inh = isuSrcImg.h;
        MSFSz.inw = isuSrcImg.w;

        isuGetImgOutResolution(&width, &height);

        MSFSz.outh = height;
        MSFSz.outw = width;

        MSFCrs.steph = isuSize2Step(MSFSz.outh, isuSrcImg.h, &phaseh);
        MSFCrs.stepw = isuSize2Step(MSFSz.outw, isuSrcImg.w, &phasew);

        MSFCrs.phaseh = phaseh;
        MSFCrs.phasew = phasew;

        IsuSCA_MODE   =ISU_Auto_Mode2|      // block to frame processing for preview
                                        ISU_PROC_2DMode|    // SDRAM buffer processing mode
                                        ISU_SRC_DRAM|       // Source data from DRAM
                                        ISU_TRIPLE_Trip|    // Use triple frame buffer
                                        ISU_BLKSEL_16L|     // Block Mode Lines Selection: 16 Lines
                                        ISU_FBR_SEL_0|      //
                                        ISU_PKW_SEL_0|
                                        ISU_PNW_SEL_0;

        IsuSCA_FUN = ISU_PKOUT_ENA|
                        ISU_PNOUT_DISA|
                        ISU_OVLEN_DISA|
                        ISU_OVLSEL_MAIN|
                        ISU_MSFEN_ENA|
                        ISU_SSFEN_DISA;
        // 0x000C
        IsuSCA_INTC =ISU_FCINTE_ENA|     //
                        ISU_PKOINTE_DISA|
                        ISU_PYOINTE_DISA|
                        ISU_PCOINTE_DISA|
                        ISU_SRUINTE_DISA;

        //0x0020
        IsuSRC_SIZE = (MSFSz.inw<<ISU_SRC_W_SHFT) + (MSFSz.inh<<ISU_SRC_H_SHFT);

        //0x0024
        IsuPK_SIZE = (MSFSz.outw << ISU_PK_W_SHFT) + (MSFSz.outh << ISU_PK_H_SHFT);

        //0x0028
        IsuPN_SIZE = 0x00000000;

        // 0x0030
        IsuMSF_STEP= (MSFCrs.stepw << ISU_MSTEPX_SHFT) + (MSFCrs.steph << ISU_MSTEPY_SHFT);

        //0x0034
        IsuMSF_PHA = (MSFCrs.phasew << ISU_MPHASEX_SHFT) + (MSFCrs.phaseh << ISU_MPHASEY_SHFT);

        IsuSRC_IADDR0 = (u32)ipuDstBuf0;
        IsuSRC_IADDR1 = (u32)ipuDstBuf1;
        IsuSRC_IADDR2 = (u32)ipuDstBuf2;
        IsuSRC_STRIDE = MSFSz.inw<<1;

        IsuPK_IADDR0 = (u32)PKBuf;
        IsuPK_IADDR1 = (u32)PKBuf1;
        IsuPK_IADDR2 = (u32)PKBuf2;
        IsuPK_STRIDE = MSFSz.outw<<1;

        IsuFIFO_TH = 0x5377777f;
        isuRst();

        // 0x00000000
        IsuSCA_EN = ISU_ENA | ISU_Single_ENA;

        return 1;
    }

    s32 isuCapturePrimary_B2B(s32 zoomFactor) /*BJ 0530 S*/
    {
        u16 phaseh, phasew;
        u16 width, height;
        /* scale yuv data */

        isuRst();
        MSFSz.inh = isuSrcImg.h;
        MSFSz.inw = isuSrcImg.w;

        isuGetImgOutResolution(&width, &height);

        MSFSz.outh = height;
        MSFSz.outw = width;

        MSFCrs.steph = isuSize2Step(MSFSz.outh, isuSrcImg.h, &phaseh);
        MSFCrs.stepw = isuSize2Step(MSFSz.outw, isuSrcImg.w, &phasew);

        MSFCrs.phaseh = phaseh;
        MSFCrs.phasew = phasew;

        IsuSCA_MODE   = ISU_Auto_Mode4|     // block to block processing for preview
                        ISU_PROC_2DMode|    // SDRAM buffer processing mode
                        ISU_SRC_DRAM|       // Source data from DRAM
                        ISU_TRIPLE_Trip|    // Use triple frame buffer
                        ISU_BLKSEL_16L|     // Block Mode Lines Selection: 16 Lines
                        ISU_FBR_SEL_0|      //
                        ISU_PKW_SEL_0|
                        ISU_PNW_SEL_0;


        IsuSCA_FUN =    ISU_PKOUT_ENA|
                        ISU_PNOUT_DISA|
                        ISU_OVLEN_DISA|
                        ISU_OVLSEL_MAIN|
                        ISU_MSFEN_ENA|
                        ISU_SSFEN_DISA;


        // 0x000C
        IsuSCA_INTC =   ISU_FCINTE_ENA|
                        ISU_PKOINTE_DISA|
                        ISU_PYOINTE_DISA|
                        ISU_PCOINTE_DISA|
                        ISU_SRUINTE_DISA;

        //0x0020
        IsuSRC_SIZE = (MSFSz.inw<<ISU_SRC_W_SHFT) + (MSFSz.inh<<ISU_SRC_H_SHFT);

        //0x0024
        IsuPK_SIZE = (MSFSz.outw << ISU_PK_W_SHFT) + (MSFSz.outh << ISU_PK_H_SHFT);

        //0x0028
        IsuPN_SIZE = 0x00000000;

        // 0x0030
        IsuMSF_STEP= (MSFCrs.stepw << ISU_MSTEPX_SHFT) + (MSFCrs.steph << ISU_MSTEPY_SHFT);

        //0x0034
        IsuMSF_PHA = (MSFCrs.phasew << ISU_MPHASEX_SHFT) + (MSFCrs.phaseh << ISU_MPHASEY_SHFT);

        IsuSRC_IADDR0 = (u32)ipuDstBuf0;
        IsuSRC_IADDR1 = (u32)ipuDstBuf1;
        IsuSRC_IADDR2 = (u32)ipuDstBuf2;
        IsuSRC_STRIDE = MSFSz.inw<<1;

        IsuPK_IADDR0 = (u32)PKBuf;
        IsuPK_IADDR1 = (u32)PKBuf1;
        IsuPK_IADDR2 = (u32)PKBuf2;
        IsuPK_STRIDE = MSFSz.outw<<1;

        IsuFIFO_TH = 0x5377777f;
        isuRst();

        // 0x00000000
        IsuSCA_EN = ISU_ENA | ISU_Single_ENA;

        return 1;
    }

    void isuSetImageOutResolution(u16 width, u16 height)
    {
        MSFSz.outh = height;
        MSFSz.outw = width;
    }


    /*BJ 0530 S*/
    s32 isuCaptureVideo(s32 zoomFactor)
    {
        isuFrameTime    = 0;
        SkipFrameDuration=0; //Lsk 090813
        // Reset ISU module
        isuRst();

        // Set input image size
        MSFSz.inh = isuSrcImg.h;
        MSFSz.inw = isuSrcImg.w;

#if( SIU_SCUP_EN )
        IsuScUpInWindow   = (MSFSz.inw <<ISU_SCUP_SRCWIN_W_SHFT) | (MSFSz.inh <<ISU_SCUP_SRCWIN_H_SHFT);
        IsuScUpOutWindow  = (MSFSz.inw <<ISU_SCUP_OUTWIN_W_SHFT) | (MSFSz.inh <<ISU_SCUP_OUTWIN_H_SHFT);
        IsuScUpStartPos   = 0;
        IsuScUpFIFOCntr   = 0x06080810;
       #if ISUCIU_PREVIEW_PNOUT
        IsuScUpCntr       = ISU_SCUP_EN | (MSFSz.inw << ISU_SCUP_LINESIZE_SHFT) | ISU_SCUP_WAIT_OVL  | ISU_SCUP_WAIT_PNY | ISU_SCUP_WAIT_PNC | ISU_SCUP_OVL_DUMY_0;
       #else
        IsuScUpCntr       = ISU_SCUP_EN | (MSFSz.inw << ISU_SCUP_LINESIZE_SHFT) | ISU_SCUP_WAIT_OVL | ISU_SCUP_WAIT_PK | ISU_SCUP_WAIT_PNY | ISU_SCUP_WAIT_PNC | ISU_SCUP_OVL_DUMY_0;
       #endif
#endif


        // Display image size (package output size)
        if(sysTVOutOnFlag) //TV out
        {
            if(TvOutMode==SYS_TV_OUT_PAL) //PAL mode
            {
                if( sysVideoInSel == VIDEO_IN_TV) //TV-in
                {
                    if(sysTVinFormat == TV_IN_PAL)
                        isuPanel.h    = TVOUT_Y_PAL/2;
                    else
                        isuPanel.h    = TVOUT_Y_NTSC/2;
                    isuPanel.w    = TVOUT_X;
                    IsuOutAreaFlagOnTV = FULL_TV_OUT;
                }
                else //sensor in
                {
                    isuPanel.h    = 480;
                    isuPanel.w    = TVOUT_X;
                    IsuOutAreaFlagOnTV = FULL_TV_OUT;
                }
            }
            else //NTSC
            {
                if( sysVideoInSel == VIDEO_IN_TV) //TV-in
                {
                    isuPanel.h    = TVOUT_Y_NTSC / 2;
                    isuPanel.w    = TVOUT_X;
                    IsuOutAreaFlagOnTV = FULL_TV_OUT;
                }
                else //sensor-in
                {
                    isuPanel.h    = TVOUT_Y_NTSC;
                    isuPanel.w    = TVOUT_X;
                    IsuOutAreaFlagOnTV = FULL_TV_OUT;
                }
            }
            // 0x0004
            IsuSCA_MODE=
                    //ISU_Auto_Mode0|       // block to frame processing for preview
                    ISU_Manual_Mode|        // block to frame processing for preview /*Peter 1109 S*/
                    ISU_PROC_LBMode|            // Line buffer processing mode
                    ISU_SRC_IPU|                // Source data from IPU
                    ISU_TRIPLE_Trip|        // Use triple frame buffer
                    ISU_BLKSEL_8L|
    				ISU_OVL_TYPE|
    				ISU_OVL_OPA|
                    ISU_FBR_SEL_0|             //
                    ISU_PKW_SEL_0|
                    ISU_PNW_SEL_0;


            // 0x0008
            IsuSCA_FUN =
                 #if ISUCIU_PREVIEW_PNOUT   //Lucian: 此時關掉PK out, 僅留PN out. Display 切為BRI mode. 可節省頻寬. valid in PA9003
                    ISU_PKOUT_DISA|
                 #else
                    ISU_PKOUT_ENA|
                 #endif
                 #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                    ISU_PNOUT_ENA|
                 #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
       				ISU_PNOUT_DISA|
                 #endif
            #if ISU_OVERLAY_ENABLE
                    ISU_OVLEN_ENA|
                #if (ISU_OVERLAY_ENABLE == 1)
                    ISU_OVLSEL_MAIN|
                #elif (ISU_OVERLAY_ENABLE == 2)
                    ISU_OVLSEL_BOTH|
                #endif
            #else
                    ISU_OVLEN_DISA|
                    ISU_OVLSEL_MAIN|
            #endif
                    ISU_MSFEN_ENA |
                 #if ISUCIU_PREVIEW_PNOUT
                    ISU_SSFEN_DISA;
                 #else
                    ISU_SSFEN_ENA;
                 #endif

           #if SIU_SCUP_EN
              IsuFIFO_TH = 0x5377777f;
           #else
              IsuFIFO_TH = 0x5377777f;
           #endif

        }
        else //Pannel out (only for small size pannel)
        {
             if( sysVideoInSel == VIDEO_IN_TV) //TV-in
             {
                    isuPanel.h    = PANNEL_Y / 2;
                    isuPanel.w    = PANNEL_X;
             }
             else
             {
                    isuPanel.h    = PANNEL_Y;
                    isuPanel.w    = PANNEL_X;
             }
            // 0x0004
            IsuSCA_MODE=
                    //ISU_Auto_Mode0|       // block to frame processing for preview
                    ISU_Manual_Mode|        // block to frame processing for preview /*Peter 1109 S*/
                    ISU_PROC_LBMode|        // Line buffer processing mode
                    ISU_SRC_IPU|            // Source data from IPU
                    ISU_TRIPLE_Trip|        // Use triple frame buffer
                    ISU_BLKSEL_8L|
    				ISU_OVL_TYPE|
    				ISU_OVL_OPA|
                    ISU_FBR_SEL_0|          //
                  #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                    ISU_PKSRGB |
                  #endif
                    ISU_PKW_SEL_0|
                    ISU_PNW_SEL_0;
            // 0x0008
            IsuSCA_FUN =
                  #if ISUCIU_PREVIEW_PNOUT
                    ISU_PKOUT_DISA |
                  #else
                    ISU_PKOUT_ENA |
                  #endif
                  #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                    ISU_PNOUT_ENA|
                  #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
       				ISU_PNOUT_DISA|
                  #endif
               #if ISU_OVERLAY_ENABLE
                    ISU_OVLEN_ENA|
                  #if (ISU_OVERLAY_ENABLE == 1)
                    ISU_OVLSEL_MAIN|
                  #elif (ISU_OVERLAY_ENABLE == 2)
                    ISU_OVLSEL_BOTH|
                  #endif
               #else
                    ISU_OVLEN_DISA|
                    ISU_OVLSEL_MAIN|
               #endif
                    ISU_MSFEN_ENA|
               #if ISUCIU_PREVIEW_PNOUT
                    ISU_SSFEN_DISA;
               #else
                    ISU_SSFEN_ENA;
               #endif

           #if SIU_SCUP_EN
              IsuFIFO_TH = 0x5377777f;
           #else
              IsuFIFO_TH = 0x5377777f; //Lucian: 避免Video clip時,LCD 上有雜點. 2008/2/25
           #endif

        }

         // Output image size (planer output size)
         if( sysVideoInSel == VIDEO_IN_TV) //TV-in
         {
            isuPacket.h = mpeg4Height / 2; /*CY 0907*/
         }
         else //Sensor-in
            isuPacket.h = mpeg4Height; /*CY 0907*/

        isuPacket.w = mpeg4Width;  /*CY 0907*/

        MSFSz.outh = isuPacket.h; //for mpeg4
        MSFSz.outw = isuPacket.w;

        SSFSz.outh  = isuPanel.h;  //for IDU/TV Display
        SSFSz.outw  = isuPanel.w;

    /*BJ 0821 S*/
        MSFCrs.steph    = isuSize2Step(isuPacket.h, MSFSz.inh, &(MSFCrs.phaseh));
        MSFCrs.stepw    = isuSize2Step(isuPacket.w, MSFSz.inw, &(MSFCrs.phasew));
    /*BJ 0821 E*/

    /*BJ 0821 S*/
        SSFCrs.steph    = isuSize2Step(SSFSz.outh, isuPacket.h, &(SSFCrs.phaseh));
        SSFCrs.stepw    = isuSize2Step(SSFSz.outw, isuPacket.w, &(SSFCrs.phasew));
    /*BJ 0821 E*/

        // 0x000C
        IsuSCA_INTC =
                        ISU_FCINTE_ENA|
                        ISU_PKOINTE_DISA|
                        ISU_PYOINTE_DISA|
                        ISU_PCOINTE_DISA|
                        ISU_SRUINTE_DISA;


        //0x0020
        IsuSRC_SIZE = (MSFSz.inw<<ISU_SRC_W_SHFT) + (MSFSz.inh<<ISU_SRC_H_SHFT);

        //0x0024
        IsuPK_SIZE  = (SSFSz.outw << ISU_PK_W_SHFT) + (SSFSz.outh << ISU_PK_H_SHFT);

        //0x0028
        IsuPN_SIZE  = (MSFSz.outw << ISU_PN_W_SHFT) + (MSFSz.outh << ISU_PN_H_SHFT);

        // 0x0030
        IsuMSF_STEP =(MSFCrs.stepw<<ISU_MSTEPX_SHFT) + (MSFCrs.steph<<ISU_MSTEPY_SHFT);

        //0x0034
        IsuMSF_PHA  = (MSFCrs.phasew << ISU_MPHASEX_SHFT) + (MSFCrs.phaseh << ISU_MPHASEY_SHFT);

        //0x0038
        IsuSSF_STEP =   (SSFCrs.stepw<<ISU_SSTEPX_SHFT) + (SSFCrs.steph<<ISU_SSTEPY_SHFT);

        //0x003C
        IsuSSF_PHA  = (MSFCrs.phasew << ISU_MPHASEX_SHFT) + (MSFCrs.phaseh << ISU_MPHASEY_SHFT);

        //0x0040
        IsuSRC_IADDR0 = (u32)ipuDstBuf0;
        IsuSRC_IADDR1 = (u32)ipuDstBuf1;
        IsuSRC_IADDR2 = (u32)ipuDstBuf2;

        //0x0050
        IsuPK_IADDR0 = (((u32)PKBuf0)& ISU_OffAdr_Mask);
        IsuPK_IADDR1 = (((u32)PKBuf1)& ISU_OffAdr_Mask);
    	IsuPK_IADDR2 = (((u32)PKBuf2)& ISU_OffAdr_Mask);
        //0x0060
        IsuPN_YIADDR0 = (((u32)PNBuf_Y0)& ISU_OffAdr_Mask);
        IsuPN_CIADDR0 = (((u32)PNBuf_C0)& ISU_OffAdr_Mask);
        IsuPN_YIADDR1 = (((u32)PNBuf_Y1)& ISU_OffAdr_Mask);
        IsuPN_CIADDR1 = (((u32)PNBuf_C1)& ISU_OffAdr_Mask);
        IsuPN_YIADDR2 = (((u32)PNBuf_Y2)& ISU_OffAdr_Mask);
        IsuPN_CIADDR2 = (((u32)PNBuf_C2)& ISU_OffAdr_Mask);

        //0x004C
        IsuSRC_STRIDE = MSFSz.inw*2;
        //0x005C
        if(sysTVOutOnFlag)//TV-out
        {
             if( sysVideoInSel == VIDEO_IN_TV) //TV-in
             {
                 IsuPK_STRIDE = (SSFSz.outw) * 2 * 2;
             }
             else //sensor-in                        //frame per sec
                 IsuPK_STRIDE = (SSFSz.outw) * 2;
        }
        else //Pannel-out
        {
             if( sysVideoInSel == VIDEO_IN_TV) //TV-in
             {
                 IsuPK_STRIDE = (isuPanel.w) * 2 * 2;
             }
             else //sensor-in
             {
                 IsuPK_STRIDE = (isuPanel.w) * 2;
             }
        }

        //0x0078
        if( sysVideoInSel == VIDEO_IN_TV) //TV-in
        {
            IsuPN_STRIDE = ((MSFSz.outw * 2) << ISU_PNCSTRIDE_SHFT) | ((MSFSz.outw * 2) << ISU_PNYSTRIDE_SHFT);
        }
        else
            IsuPN_STRIDE = (MSFSz.outw<<ISU_PNCSTRIDE_SHFT) |(MSFSz.outw<<ISU_PNYSTRIDE_SHFT);

#if ISU_OVERLAY_ENABLE
        isuOverlayImgConfig();
      #if SPEEDUP
        memset(szString1, 0, MAX_OVERLAYSTR);
      #endif
    	memset(ScalarOverlayImage, 0, 640 * 48 * 2);
      #if CDVR_LOG
        LogFileStart                = 0;
        LogFileIndex[LogFileStart]  = LogFileBuf;
        szLogFile                   = LogFileBuf;
      #endif
        sysDrawTimeOnVideoClip(1);
        isuGenerateScalarOverlayImage   = 1;
#endif

        isuRst();
        isu_avifrmcnt = 0;
        isu_idufrmcnt = 0; /*Peter 1113 S*/

        IsuSCA_EN = ISU_ENA | ISU_Conti_ENA;
        return 1;
    }

    void isuOverlayImgConfig()
    {
    #if ISU_OVERLAY_ENABLE

        if( sysVideoInSel == VIDEO_IN_TV) //TV-in
        {
            if( (mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
            {
                IsuOVL_IADDR    = (u32)ScalarOverlayImage;

            #if (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_12x12x95)
                IsuOVL_STRIDE   = 640;
                IsuOVL_WSP      =  83 | (223 << 16);
                IsuOVL_WEP      = 310 | (234 << 16);
            #elif (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_12x12x95_2BIT)
                    IsuOVL_STRIDE   = 80;
                    IsuOVL_WSP      = 83 | (223 << 16);
                    IsuOVL_WEP      =310 | (234 << 16);
                    IsuOVLPAL1      = 0x008080ff;
                    IsuOVLPAL2      = 0x00FF554C;
                    IsuOVLPAL3      = 0x00000000;
            #endif
            }
            else if (TvOutMode == SYS_TV_OUT_NTSC)
            {
                IsuOVL_IADDR    = (u32)ScalarOverlayImage;
            #if ((ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95) || (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_25) || (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DR900))
                IsuOVL_STRIDE   = 1280 * 2;
                IsuOVL_WSP      = 245 | (219 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95)
                IsuOVL_STRIDE   = 1280 * 2;
                IsuOVL_WSP      =  17 | (209 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x48x95)
                IsuOVL_STRIDE   = 1280 * 2;
                IsuOVL_WSP      =  17 | (205 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x36x95)
                IsuOVL_STRIDE   = 1280 * 2;
                IsuOVL_WSP      = 245 | (209 << 16);
                IsuOVL_WEP      = 624 | (225 << 16);
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_1BIT)
                IsuOVL_STRIDE   = 80 * 2;
                IsuOVL_WSP      =  17 | (209 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x008181FD;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_2BIT)
                IsuOVL_STRIDE   = 160 * 2;
                IsuOVL_WSP      =  17 | (209 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x00000000;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA)
                IsuOVL_STRIDE   = 160 * 2;
                IsuOVL_WSP      =  17 | (209 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x00000000;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_12x12x95_2BIT)
                IsuOVL_STRIDE   = 160 * 2;
                IsuOVL_WSP      =  17 | (209 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x00000000;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT || ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT || ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT_DR900 || ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT_WITH_LIGHT)
                IsuOVL_STRIDE   = 160 * 2;
                IsuOVL_WSP      =  17 | (209 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
                IsuOVLPAL1      = 0x008080ff;   // white
                IsuOVLPAL2      = 0x009400e1;   // yellow
                IsuOVLPAL3      = 0x00ff544c;   // red
            #endif
            }
            else    /*PAL*/
            {
                IsuOVL_IADDR    = (u32)ScalarOverlayImage;
            #if ((ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95) || (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_25) || (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DR900))
                IsuOVL_STRIDE   = 1280 * 2;
                IsuOVL_WSP      = 245 | (264 << 16);
                IsuOVL_WEP      = 624 | (273 << 16);
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95)
                IsuOVL_STRIDE   = 1280 * 2;
                IsuOVL_WSP      =  17 | (254 << 16);
                IsuOVL_WEP      = 624 | (273 << 16);
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x48x95)
                IsuOVL_STRIDE   = 1280 * 2;
                IsuOVL_WSP      =  17 | (254 << 16);
                IsuOVL_WEP      = 624 | (273 << 16);
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x36x95)
                IsuOVL_STRIDE   = 1280 * 2;
                IsuOVL_WSP      = 245 | (254 << 16);
                IsuOVL_WEP      = 624 | (270 << 16);
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_1BIT)
                IsuOVL_STRIDE   = 80 * 2;
                IsuOVL_WSP      =  17 | (254 << 16);
                IsuOVL_WEP      = 624 | (273 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x008181FD;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_2BIT)
                IsuOVL_STRIDE   = 160 * 2;
                IsuOVL_WSP      =  17 | (254 << 16);
                IsuOVL_WEP      = 624 | (273 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x008181FD;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA)
                IsuOVL_STRIDE   = 160 * 2;
                IsuOVL_WSP      =  17 | (254 << 16);
                IsuOVL_WEP      = 624 | (273 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x008181FD;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT || ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT || ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT_DR900|| ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT_WITH_LIGHT)
                IsuOVL_STRIDE   = 160 * 2;
                IsuOVL_WSP      =  17 | (267 << 16);
                IsuOVL_WEP      = 624 | (276 << 16);
                IsuOVLPAL1      = 0x008080ff;   // white
                IsuOVLPAL2      = 0x009400e1;   // yellow
                IsuOVLPAL3      = 0x00ff544c;   // red
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_12x12x95_2BIT)
                IsuOVL_STRIDE   = 160 * 2;
                IsuOVL_WSP      =  17 | (254 << 16);
                IsuOVL_WEP      = 624 | (273 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x008181FD;
            #endif
            }
        }
        else  //Sensor-in
        {
            //0x0080
            if( (mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
            {
                IsuOVL_IADDR    = (u32)ScalarOverlayImage;
            #if (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_12x12x95)
                IsuOVL_STRIDE   = 640;
                IsuOVL_WSP      =  83 | (223 << 16);
                IsuOVL_WEP      = 310 | (234 << 16);
            #elif (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_12x12x95_2BIT)
                IsuOVL_STRIDE   = 80;
                IsuOVL_WSP      = 83 | (223 << 16);
                IsuOVL_WEP      =310 | (234 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x00000000;
            #endif
            }
            else
            {
                IsuOVL_IADDR    = (u32)ScalarOverlayImage;
            #if ((ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95) || (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_25) || (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DR900))
                IsuOVL_STRIDE   = 1280;
                IsuOVL_WSP      = 245 | (437 << 16);        // for NEWKEN
                IsuOVL_WEP      = 624 | (456 << 16);     // for NEWKEN
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95)
                IsuOVL_STRIDE   = 1280;
                IsuOVL_WSP      = 17  | (417 << 16);        // for NEWKEN
                IsuOVL_WEP      = 624 | (456 << 16);     // for NEWKEN
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x48x95)
                IsuOVL_STRIDE   = 1280;
                IsuOVL_WSP      = 17  | (409 << 16);        // for NEWKEN
                IsuOVL_WEP      = 624 | (456 << 16);
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x36x95)
                IsuOVL_STRIDE   = 1280;
                IsuOVL_WSP      = 245 | (437 << 16);
                IsuOVL_WEP      = 624 | (456 << 16);     // for NEWKEN
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_1BIT)
                IsuOVL_STRIDE   = 80;
                IsuOVL_WSP      =  17 | (209 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x008181FD;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_2BIT)
                IsuOVL_STRIDE   = 160;
                IsuOVL_WSP      =  17 | (209 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x008181FD;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA)
                IsuOVL_STRIDE   = 160;
                IsuOVL_WSP      =  17 | (209 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x008181FD;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT || ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT || ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT_DR900|| ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT_WITH_LIGHT)
                IsuOVL_STRIDE   = 160;
                IsuOVL_WSP      = 245 | (437 << 16);
                IsuOVL_WEP      = 624 | (456 << 16);
                IsuOVLPAL1      = 0x008080ff;   // white
                IsuOVLPAL2      = 0x009400e1;   // yellow
                IsuOVLPAL3      = 0x00ff544c;   // red
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_12x12x95_2BIT)
                IsuOVL_STRIDE   = 160;
                IsuOVL_WSP      =  17 | (209 << 16);
                IsuOVL_WEP      = 624 | (228 << 16);
                IsuOVLPAL1      = 0x008080ff;
                IsuOVLPAL2      = 0x00FF554C;
                IsuOVLPAL3      = 0x008181FD;
            #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_8x16x95_2BIT)
                IsuOVL_STRIDE   = 160;
            #endif
            }
        }
    #endif
    }

     //Note: 只以PA9002D 用此一functon.(Lucian)
    s32 isuVideoZoom(s32 zoomFactor)
    {
        // Reset ISU module
        isuRst();

        // Set input image size
        MSFSz.inh = isuSrcImg.h;
        MSFSz.inw = isuSrcImg.w;

        if(sysTVOutOnFlag) //TV out
        {
            if(TvOutMode==SYS_TV_OUT_PAL) //PAL mode
            {
            #ifdef TV_WITH_PAL_FORCE2QVGA //Fixed bug of PA9001D PAL mode
                isuPanel.h    = 288;
                isuPanel.w    = TVOUT_X/2;
                IsuOutAreaFlagOnTV = HALF_TV_OUT;
            #else
                if( sysVideoInSel == VIDEO_IN_TV) //TV-in
                {
                   #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_QVGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )  //60 fps
                      if(sysTVinFormat == TV_IN_PAL)
                         isuPanel.h    = TVOUT_Y_PAL/2;
                      else
                         isuPanel.h    = TVOUT_Y_NTSC/2;
                      isuPanel.w    = TVOUT_X/2;
                      IsuOutAreaFlagOnTV = HALF_TV_OUT;
                   #else
                      if( (mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
                      {
                        if(sysTVinFormat == TV_IN_PAL)
                            isuPanel.h    = (TVOUT_Y_PAL/2);
                        else
                             isuPanel.h    = (TVOUT_Y_NTSC/2);
                        isuPanel.w    = TVOUT_X/2;
                        IsuOutAreaFlagOnTV = HALF_TV_OUT;
                      }
                      else
                      {
                        if(sysTVinFormat == TV_IN_PAL)
                            isuPanel.h    = TVOUT_Y_PAL/2;
                        else
                            isuPanel.h    = TVOUT_Y_NTSC/2;
                        isuPanel.w    = TVOUT_X;
                        IsuOutAreaFlagOnTV = FULL_TV_OUT;
                      }
                   #endif
                }
                else //sensor in
                {
                   #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_QVGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
                      isuPanel.h    = TVOUT_Y_PAL/2;
                      isuPanel.w    = TVOUT_X/2;
                      IsuOutAreaFlagOnTV = HALF_TV_OUT;
                   #else
                      if((mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
                      {
                        isuPanel.h    = 240;//TVOUT_Y_PAL/2;
                        isuPanel.w    = TVOUT_X/2;
                        IsuOutAreaFlagOnTV = HALF_TV_OUT;
                      }
                      else
                      {
                        isuPanel.h    = 480;
                        isuPanel.w    = TVOUT_X;
                        IsuOutAreaFlagOnTV = FULL_TV_OUT;
                      }
                   #endif
                }
            #endif
            }
            else //NTSC
            {
                if( sysVideoInSel == VIDEO_IN_TV) //TV-in
                {
                    #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_QVGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
                        isuPanel.h    = TVOUT_Y_NTSC / 2;
                        isuPanel.w    = TVOUT_X / 2;
                        IsuOutAreaFlagOnTV = HALF_TV_OUT;
                    #else
                        if( (mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
                        {
                            isuPanel.h    = (TVOUT_Y_NTSC / 2);
                            isuPanel.w    = TVOUT_X / 2;
                            IsuOutAreaFlagOnTV = HALF_TV_OUT;
                        }
                        else
                        {
                            isuPanel.h    = TVOUT_Y_NTSC / 2;
                            isuPanel.w    = TVOUT_X;
                            IsuOutAreaFlagOnTV = FULL_TV_OUT;
                        }
                    #endif
                }
                else //sensor-in
                {
                    #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_QVGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
                        isuPanel.h    = TVOUT_Y_NTSC/2;
                        isuPanel.w    = TVOUT_X/2;
                        IsuOutAreaFlagOnTV = HALF_TV_OUT;
                    #else
                        if( (mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
                        {
                            isuPanel.h    = TVOUT_Y_NTSC/2;
                            isuPanel.w    = TVOUT_X/2;
                            IsuOutAreaFlagOnTV = HALF_TV_OUT;
                        }
                        else
                        {
                            isuPanel.h    = TVOUT_Y_NTSC;
                            isuPanel.w    = TVOUT_X;
                            IsuOutAreaFlagOnTV = FULL_TV_OUT;
                        }
                    #endif
                }
            }
            // 0x0004
            IsuSCA_MODE=
                    //ISU_Auto_Mode0|       // block to frame processing for preview
                    ISU_Manual_Mode|        // block to frame processing for preview /*Peter 1109 S*/
                    ISU_PROC_LBMode|            // Line buffer processing mode
                    ISU_SRC_IPU|                // Source data from IPU
                    ISU_TRIPLE_Trip|        // Use triple frame buffer
                    ISU_BLKSEL_8L|
    				ISU_OVL_TYPE|
    				ISU_OVL_OPA|
                    ISU_FBR_SEL_0|             //
                    ISU_PKW_SEL_0|
                    ISU_PNW_SEL_0;


            // 0x0008
            IsuSCA_FUN =
                    ISU_PKOUT_ENA|
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                    ISU_PNOUT_ENA|
                    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
       				ISU_PNOUT_DISA|
                    #endif
            #if ISU_OVERLAY_ENABLE
                    ISU_OVLEN_ENA|
                #if (ISU_OVERLAY_ENABLE == 1)
                    ISU_OVLSEL_MAIN|
                #elif (ISU_OVERLAY_ENABLE == 2)
                    ISU_OVLSEL_BOTH|
                #endif
            #else
                    ISU_OVLEN_DISA|
                    ISU_OVLSEL_MAIN|
            #endif
                    ISU_MSFEN_ENA |
                    ISU_SSFEN_ENA;
           #if SIU_SCUP_EN
              IsuFIFO_TH = 0x5377777f;
           #else
              IsuFIFO_TH = 0x5377777f;
           #endif
        }
        else //Pannel out (only for small size pannel)
        {
             if( sysVideoInSel == VIDEO_IN_TV) //TV-in
             {
                #if( (LCM_OPTION == LCM_CCIR601_640x480P) || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
                    isuPanel.h    = PANNEL_Y / 2;
                    isuPanel.w    = PANNEL_X;
                #else
                    isuPanel.h    = PANNEL_Y;
                    isuPanel.w    = PANNEL_X;
                #endif
             }
             else
             {
                    isuPanel.h    = PANNEL_Y;
                    isuPanel.w    = PANNEL_X;
             }
            // 0x0004
            IsuSCA_MODE=
                    //ISU_Auto_Mode0|       // block to frame processing for preview
                    ISU_Manual_Mode|        // block to frame processing for preview /*Peter 1109 S*/
                    ISU_PROC_LBMode|        // Line buffer processing mode
                    ISU_SRC_IPU|            // Source data from IPU
                    ISU_TRIPLE_Trip|        // Use triple frame buffer
                    ISU_BLKSEL_8L|
    				ISU_OVL_TYPE|
    				ISU_OVL_OPA|
                    ISU_FBR_SEL_0|          //
                  #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                    ISU_PKSRGB |
                  #endif
                    ISU_PKW_SEL_0|
                    ISU_PNW_SEL_0;
            // 0x0008

            IsuSCA_FUN =
                    ISU_PKOUT_ENA|
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                    ISU_PNOUT_ENA|
                    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
       				ISU_PNOUT_DISA|
                    #endif
            #if ISU_OVERLAY_ENABLE
                    ISU_OVLEN_ENA|
                #if (ISU_OVERLAY_ENABLE == 1)
                    ISU_OVLSEL_MAIN|
                #elif (ISU_OVERLAY_ENABLE == 2)
                    ISU_OVLSEL_BOTH|
                #endif
            #else
                    ISU_OVLEN_DISA|
                    ISU_OVLSEL_MAIN|
            #endif
                    ISU_MSFEN_ENA|
                    ISU_SSFEN_ENA;
           #if SIU_SCUP_EN
              IsuFIFO_TH = 0x5377777f;
           #else
              IsuFIFO_TH = 0x5377777f; //Lucian: 避免Video clip時,LCD 上有雜點. 2008/2/25
           #endif
        }

        //Reassign Packet/Planar channel
        IduWinCtrl = (IduWinCtrl & ~0x00003000) | (( (isu_idufrmcnt-1) % 3) << 12);//Lucian: Re-assign idu display index, 20080704

        IsuSCA_MODE = (IsuSCA_MODE & ~0x3c00) |
                      ((isu_idufrmcnt % 3) << 10);
        IsuPN_YIADDR0 = (((u32)PNBuf_Y[isu_avifrmcnt % 4])& ISU_OffAdr_Mask);
        IsuPN_CIADDR0 = (((u32)PNBuf_C[isu_avifrmcnt % 4])& ISU_OffAdr_Mask);

        // Output image size (planer output size)
        if( sysVideoInSel == VIDEO_IN_TV) //TV-in
        {
            if( (mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frame
                isuPacket.h = mpeg4Height ; /*CY 0907*/
            else
                isuPacket.h = mpeg4Height / 2; /*CY 0907*/
        }
        else //Sensor-in
             isuPacket.h = mpeg4Height; /*CY 0907*/

        isuPacket.w = mpeg4Width;  /*CY 0907*/

        MSFSz.outh = isuPacket.h; //for mpeg4
        MSFSz.outw = isuPacket.w;

        if(sysTVOutOnFlag) //TV out
        {
            SSFSz.outh  = isuPanel.h;  //for IDU/TV Display
            SSFSz.outw  = isuPanel.w;
        }
        else
        {
            if( ((mpeg4Width == 320) || (mpeg4Width == 352)) && isuPanel.w > mpeg4Width) //Lsk 090703 : LCD pannel szie large than QVGA
            {
                SSFSz.outh  = mpeg4Height;  //for IDU/TV Display
                SSFSz.outw  = mpeg4Width;
            }
            else
            {
                SSFSz.outh  = isuPanel.h;  //for IDU/TV Display
                SSFSz.outw  = isuPanel.w;
            }
        }

    /*BJ 0821 S*/
        MSFCrs.steph = isuSize2Step(isuPacket.h, MSFSz.inh, &(MSFCrs.phaseh));
        MSFCrs.stepw = isuSize2Step(isuPacket.w, MSFSz.inw, &(MSFCrs.phasew));
    /*BJ 0821 E*/

    /*BJ 0821 S*/
        SSFCrs.steph = isuSize2Step(isuPanel.h, isuPacket.h, &(SSFCrs.phaseh));
        SSFCrs.stepw = isuSize2Step(isuPanel.w, isuPacket.w, &(SSFCrs.phasew));
    /*BJ 0821 E*/

        // 0x000C
        IsuSCA_INTC=
                        //ISU_FCINTE_DISA|  //Lucian: Fixed HW bug. 因Main path 未打開前, isu intrrupt 已開始發,影響 frame control.
                        ISU_FCINTE_ENA|
                        ISU_PKOINTE_DISA|
                        ISU_PYOINTE_DISA|
                        ISU_PCOINTE_DISA|
                        ISU_SRUINTE_DISA;

        //0x0020
        IsuSRC_SIZE = (MSFSz.inw<<ISU_SRC_W_SHFT) + (MSFSz.inh<<ISU_SRC_H_SHFT);

        //0x0024
        IsuPK_SIZE = (SSFSz.outw << ISU_PK_W_SHFT) + (SSFSz.outh << ISU_PK_H_SHFT);

        //0x0028
        IsuPN_SIZE = (MSFSz.outw << ISU_PN_W_SHFT) + (MSFSz.outh << ISU_PN_H_SHFT);

        // 0x0030
        IsuMSF_STEP =(MSFCrs.stepw<<ISU_MSTEPX_SHFT) + (MSFCrs.steph<<ISU_MSTEPY_SHFT);

        //0x0034
        IsuMSF_PHA = (MSFCrs.phasew << ISU_MPHASEX_SHFT) + (MSFCrs.phaseh << ISU_MPHASEY_SHFT);

        //0x0038
        IsuSSF_STEP =   (SSFCrs.stepw<<ISU_SSTEPX_SHFT) + (SSFCrs.steph<<ISU_SSTEPY_SHFT);

        //0x003C
        IsuSSF_PHA = (MSFCrs.phasew << ISU_MPHASEX_SHFT) + (MSFCrs.phaseh << ISU_MPHASEY_SHFT);

        //0x0040
        IsuSRC_IADDR0 = (u32)ipuDstBuf0;
        IsuSRC_IADDR1 = (u32)ipuDstBuf1;
        IsuSRC_IADDR2 = (u32)ipuDstBuf2;

        //0x0050
        if(sysTVOutOnFlag && TvOutMode==SYS_TV_OUT_PAL && isuPanel.h == 480) //PAL mode  //Lsk 090518 : put video frame in center
        {
       		IsuPK_IADDR0 = (((u32)(PKBuf0 + 61440))& ISU_OffAdr_Mask);
    	    IsuPK_IADDR1 = (((u32)(PKBuf1 + 61440))& ISU_OffAdr_Mask);
        	IsuPK_IADDR2 = (((u32)(PKBuf2 + 61440))& ISU_OffAdr_Mask);
        }
        else if(sysTVOutOnFlag && TvOutMode==SYS_TV_OUT_PAL && isuPanel.h == 240) //PAL mode and QVGA
        {
       		IsuPK_IADDR0 = (((u32)(PKBuf0 + 15360))& ISU_OffAdr_Mask);
    	    IsuPK_IADDR1 = (((u32)(PKBuf1 + 15360))& ISU_OffAdr_Mask);
        	IsuPK_IADDR2 = (((u32)(PKBuf2 + 15360))& ISU_OffAdr_Mask);
        }
        else
        {
       		IsuPK_IADDR0 = (((u32)PKBuf0)& ISU_OffAdr_Mask);
    	    IsuPK_IADDR1 = (((u32)PKBuf1)& ISU_OffAdr_Mask);
        	IsuPK_IADDR2 = (((u32)PKBuf2)& ISU_OffAdr_Mask);
        }

        //0x0060
        /* //Lucian: remove 080719
        IsuPN_YIADDR0 = (((u32)PNBuf_Y0)& ISU_OffAdr_Mask);
        IsuPN_CIADDR0 = (((u32)PNBuf_C0)& ISU_OffAdr_Mask);
        IsuPN_YIADDR1 = (((u32)PNBuf_Y1)& ISU_OffAdr_Mask);
        IsuPN_CIADDR1 = (((u32)PNBuf_C1)& ISU_OffAdr_Mask);
        IsuPN_YIADDR2 = (((u32)PNBuf_Y2)& ISU_OffAdr_Mask);
        IsuPN_CIADDR2 = (((u32)PNBuf_C2)& ISU_OffAdr_Mask);
        */
        //0x004C
        IsuSRC_STRIDE = MSFSz.inw*2;

        //0x005C
        if(sysTVOutOnFlag)//TV-out
        {
             if( sysVideoInSel == VIDEO_IN_TV) //TV-in
             {
                #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_QVGA) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) ) //QVGA frames
                    IsuPK_STRIDE = (SSFSz.outw) * 2;
                #else                            //field per sec
                    if( (mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
                        IsuPK_STRIDE = (SSFSz.outw) * 2;
                    else
                        IsuPK_STRIDE = (SSFSz.outw) * 2 * 2;
                #endif
             }
             else //sensor-in                        //frame per sec
                    IsuPK_STRIDE = (SSFSz.outw) * 2;
        }
        else //Pannel-out
        {
             if( sysVideoInSel == VIDEO_IN_TV) //TV-in
             {
                #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                    IsuPK_STRIDE = (isuPanel.w);
                #elif( (LCM_OPTION == LCM_CCIR601_640x480P) || (LCM_OPTION == LCM_HX8257_RGB666_480x272) ||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
                    IsuPK_STRIDE = (isuPanel.w) * 2 * 2;
                #else
                    IsuPK_STRIDE = (isuPanel.w) * 2;
                #endif
             }
             else //sensor-in
             {
                #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                    IsuPK_STRIDE = (isuPanel.w);
                #else
                    IsuPK_STRIDE = (isuPanel.w) * 2;
                #endif
             }

            if((mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
            {
                IsuPK_IADDR0 = (((u32)(PKBuf0 + X_offset + Y_offset*isuPanel.w))& ISU_OffAdr_Mask);  //Lsk 090707 : put QVGA in center
                IsuPK_IADDR1 = (((u32)(PKBuf1 + X_offset + Y_offset*isuPanel.w))& ISU_OffAdr_Mask);
                IsuPK_IADDR2 = (((u32)(PKBuf2 + X_offset + Y_offset*isuPanel.w))& ISU_OffAdr_Mask);
            }
        }

        //0x0078
        if( sysVideoInSel == VIDEO_IN_TV) //TV-in
        {
            if((mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
                IsuPN_STRIDE = (MSFSz.outw<<ISU_PNCSTRIDE_SHFT) |(MSFSz.outw<<ISU_PNYSTRIDE_SHFT);
            else
                IsuPN_STRIDE = ((MSFSz.outw * 2) << ISU_PNCSTRIDE_SHFT) | ((MSFSz.outw * 2) << ISU_PNYSTRIDE_SHFT);
        }
        else
            IsuPN_STRIDE = (MSFSz.outw<<ISU_PNCSTRIDE_SHFT) |(MSFSz.outw<<ISU_PNYSTRIDE_SHFT);

#if ISU_OVERLAY_ENABLE
        isuOverlayImgConfig();
        #if SPEEDUP
            memset(szString1, 0, MAX_OVERLAYSTR);
        #endif
    		memset(ScalarOverlayImage, 0, 640 * 48 * 2);
        #if CDVR_LOG
            LogFileStart                = 0;
            LogFileIndex[LogFileStart]  = LogFileBuf;
            szLogFile                   = LogFileBuf;
        #endif
            sysDrawTimeOnVideoClip(1);
#endif

        isuRst();

        IsuSCA_EN = ISU_ENA | ISU_Conti_ENA;
        return 1;
    }
    /*BJ 0530 E*/

    

    /*

    Routine Description:

        Capture thumbnail.

    Arguments:

        None.

    Return Value:

        0 - Failure.
        1 - Success.

    */
    s32 isuScalar2Thumbnail(s32 zoomFactor) /*BJ 0530 S*/
    {
        INT8U err;
        //========== first scaling down : PKBuf -->PKBuf0(320x240)
        if(sysTVOutOnFlag)
        {
              if(TvOutMode==SYS_TV_OUT_PAL)
              {
                #ifdef TV_WITH_PAL_FORCE2QVGA
                   isuScalar_D2D(PKBuf , PKBuf0 , MSFSz.outw , MSFSz.outh , (TVOUT_X/2), (TVOUT_Y_NTSC/2));
                #else
                   isuScalar_D2D(PKBuf , PKBuf0 , MSFSz.outw, MSFSz.outh , TVOUT_X , TVOUT_Y_NTSC);
                #endif
               }
               else
                 isuScalar_D2D(PKBuf , PKBuf0 , MSFSz.outw , MSFSz.outh , TVOUT_X , TVOUT_Y_NTSC);
        }
        else
        {
                 isuScalar_D2D(
                           PKBuf , PKBuf0 ,
                           MSFSz.outw, MSFSz.outh,
                           PANNEL_X, PANNEL_Y
                         );
        }

        //================ second scaling down: PKBuf0 -->PKBuf1
        if(sysTVOutOnFlag)
        {
              if(TvOutMode==SYS_TV_OUT_PAL)
              {
                #ifdef TV_WITH_PAL_FORCE2QVGA
                  //isuScalar_D2D(PKBuf0 , PKBuf1 , (TVOUT_X/2), 288 , 160 , 120);
                  isuScalar_D2D(PKBuf0 , PKBuf1 , (TVOUT_X/2), (TVOUT_Y_NTSC/2) , 160 , 120);
                #else
                  isuScalar_D2D(PKBuf0 , PKBuf1 , TVOUT_X, TVOUT_Y_NTSC , 160 , 120);
                #endif
              }
              else
                isuScalar_D2D(PKBuf0 , PKBuf1 , TVOUT_X, TVOUT_Y_NTSC , 160 , 120);
        }
        else
        {

                isuScalar_D2D(
                               PKBuf0 , PKBuf1 ,
                               PANNEL_X, PANNEL_Y,
                               160, 120);
        }
        return 1;
    }

    /*

    Routine Description:

        Reset the Image Scaling Unit.

    Arguments:

        None.

    Return Value:

        None.

    */
    void isuRst(void)
    {
        u16 i;

        IsuSCA_EN = ISU_NORMAL;
        for(i=0; i<10;i++){;}
        IsuSCA_EN = ISU_RESET;
        for(i=0; i<10;i++){;}
        IsuSCA_EN = ISU_NORMAL;
        for(i=0; i<10;i++){;}

        /*Peter 1109 S*/
        while(isuSemEvt->OSEventCnt > 0)
        {
            OSSemAccept(isuSemEvt);
        }
        /*Peter 1109 E*/
    }

    void isuStop(void)
    {
        IsuSCA_EN = ISU_DISA;
        IsuSCA_INTC=    ISU_FCINTE_DISA|    //Lucian: close interrupt @080716
                        ISU_PKOINTE_DISA|
                        ISU_PYOINTE_DISA|
                        ISU_PCOINTE_DISA|
                        ISU_SRUINTE_DISA;
    }

    /*BJ 0523 S*/
    /*BJ 0821 S*/
    s32 isuPlayback(u8 *Srcbuf , u8 *Dstbuf , u16 sizeX, u16 sizeY)
    {
        u8 err,EnSubScalarflag = 0;
        u32 StepX,StepY;
        u32 SubScalar_factor=0;
        u32 Disp_X,Disp_Y;

        #if DINAMICALLY_POWER_MANAGEMENT
                sysISU_enable();
        #endif

        if(sysTVOutOnFlag) //TV-out
        {
               if(TvOutMode==SYS_TV_OUT_PAL) //PAL mode
               {
                   
                    Disp_X=TVOUT_X;
                    Disp_Y=TVOUT_Y_PAL;

                    if((sizeX/TVOUT_X)>=8)
                    {
                   	  Disp_X = (TVOUT_X*2);
                   	  Disp_Y = (TVOUT_Y_PAL*2);
                   	  scaler2sd = 1;
                    }
               }
               else //NTSC mode
               {
                   
                       Disp_X=TVOUT_X;
                       Disp_Y=TVOUT_Y_NTSC;
                       if((sizeX/TVOUT_X)>=8)
                       {
                         Disp_X = (TVOUT_X*2);
                         Disp_Y = (TVOUT_Y_NTSC*2);
                         scaler2sd = 1;
                       }
               }
        }
        else
        {
               Disp_X=PANNEL_X;
               Disp_Y=PANNEL_Y;
               if((sizeX/PANNEL_X)>=8)
               {
                     Disp_X = TVOUT_X;
                     Disp_Y = TVOUT_Y_NTSC;
                     scaler2sd = 1;
               }
        }

        StepX = sizeX*1024/Disp_X;
        StepY = sizeY*1024/Disp_Y;

        EnSubScalarflag = 0;
        SubScalar_factor=0;

        isuRst();

        if(scaler2sd==1) //Lucian: 做兩階段scaling
        {
               IsuSCA_MODE =    ISU_Manual_Mode |
                                ISU_PROC_2DMode|
                                ISU_SRC_DRAM|
                                ISU_FBR_SEL_0|
                                ISU_PKW_SEL_0;

            IsuPK_STRIDE    =   Disp_X<<1;
        }
        else
        {
            IsuSCA_MODE =   ISU_Manual_Mode |
                            ISU_PROC_2DMode|
                            ISU_SRC_DRAM|
                            ISU_FBR_SEL_0|
                           #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                            ISU_PKSRGB |
                           #endif
                            ISU_PKW_SEL_0;

            #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                IsuPK_STRIDE    =   Disp_X;
            #else
            IsuPK_STRIDE    =   Disp_X<<1;
            #endif
        }

        IsuSCA_FUN  =   ISU_PKOUT_ENA|
                    ISU_MSFEN_ENA;

        IsuSCA_INTC =   ISU_FCINTE_ENA|
                    ISU_PKOINTE_DISA;


        IsuSRC_SIZE =   sizeX << ISU_SRC_W_SHFT |
                    sizeY << ISU_SRC_H_SHFT;

        IsuPK_SIZE  =   (Disp_X<< ISU_PK_W_SHFT) |
                    (Disp_Y<< ISU_PK_H_SHFT);

        IsuMSF_STEP =   ((sizeX*1024/Disp_X) << ISU_MSTEPX_SHFT)|
                    ((sizeY*1024/Disp_Y) << ISU_MSTEPY_SHFT);
        IsuMSF_PHA  =   0;


        IsuSRC_IADDR0   =   (u32)Srcbuf;
        IsuSRC_STRIDE   =   sizeX<<1;

        IsuPK_IADDR0    =   (u32)Dstbuf;


        IsuFIFO_TH      = 0x5377777f;

        isuRst();

        isuStatus_OnRunning= 1;/*BJ 0428*/
        IsuSCA_EN       =   ISU_ENA | ISU_Single_ENA;

        OSSemPend(isuSemEvt, ISU_TIMEOUT ,&err);
        if (err != OS_NO_ERR)
        {
            DEBUG_ISU("Error: isuSemEvt(playback mode) is %d.\n", err);
        }
        #if DINAMICALLY_POWER_MANAGEMENT
            sysISU_disable();
        #endif

        return 1;
    }
    /*BJ 0821 E*/
    /*BJ 0523 E*/

    s32 isuGenPauseFrame(u8 *Srcbuf , u8 *Dstbuf , u16 sizeX, u16 sizeY)
    {
        u32 StepX,StepY;
        u8 err;
        u32 Disp_X,Disp_Y;
        u16 phaseh, phasew;
        u8  Scale_half_Y;

        /*
           Lucian: 為節省頻寬, Playback Video 時,僅使用 half of Height (ex: 640x480 -->640x240) 當作source image
                   去做scaling.
        */

        Scale_half_Y= (sizeY > 360) ? 1: 0; //Lucian: height of source image 若大於360 lines,才做 half_scaling up;
        if(sysTVOutOnFlag) //TV-out
        {
            Disp_X = sizeX;
            Disp_Y = sizeY;
        
        }
        else //Pannel-out
        {
            Disp_X=PANNEL_X;
            Disp_Y=PANNEL_Y;
        }

    	#if (LCM_OPTION == LCM_CCIR601_640x480P)

        #else
           if(Scale_half_Y)
               sizeY >>= 1;
    	#endif

        isuRst();
        IsuSCA_MODE =       ISU_Manual_Mode |
                            ISU_PROC_2DMode|
                            ISU_SRC_DRAM|
                            ISU_FBR_SEL_0|
                         #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                            ISU_PKSRGB |
                         #endif
                            ISU_PKW_SEL_0;

        IsuSCA_FUN  =   ISU_PKOUT_ENA|
                        ISU_MSFEN_ENA;

        IsuSCA_INTC =   ISU_FCINTE_ENA|
                        ISU_PKOINTE_DISA;

        IsuSRC_SIZE =   sizeX << ISU_SRC_W_SHFT |
                        sizeY << ISU_SRC_H_SHFT;

        IsuPK_SIZE  =   (Disp_X<< ISU_PK_W_SHFT) |
                        (Disp_Y<< ISU_PK_H_SHFT);



        StepX       = isuSize2Step(Disp_X, sizeX, &phasew);
        StepY       = isuSize2Step(Disp_Y, sizeY, &phaseh);
        IsuMSF_STEP = (StepX << ISU_MSTEPX_SHFT) |
                                     (StepY << ISU_MSTEPY_SHFT);
        IsuMSF_PHA  = (phasew << ISU_MPHASEX_SHFT) + (phaseh << ISU_MPHASEY_SHFT);


        IsuSRC_IADDR0   =   (u32)Srcbuf;

        /*
           Lucian: 為節省頻寬, Playback Video 時,僅使用 half of Height (ex: 640x480 -->640x240) 當作source image
                   去做scaling.
        */
      #if (LCM_OPTION == LCM_CCIR601_640x480P)
        IsuSRC_STRIDE   = sizeX << 1;
      #else
        if(Scale_half_Y)
           IsuSRC_STRIDE   = sizeX << 2;
        else
           IsuSRC_STRIDE   = sizeX << 1;
      #endif


        IsuPK_IADDR0    =   (u32)Dstbuf;

        #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
            IsuPK_STRIDE    =   Disp_X;
        #else
            IsuPK_STRIDE    =   Disp_X<<1;
        #endif

        IsuFIFO_TH = 0x5377777f;

        isuRst();

        isuStatus_OnRunning    = 1;/*BJ 0428*/
        IsuSCA_EN           =   ISU_ENA | ISU_Single_ENA;

        return 1;
    }
    s32 isuPlayback_av(u8 *Srcbuf , u8 *Dstbuf , u16 sizeX, u16 sizeY)
    {
        u32 StepX,StepY;
        u8 err;
        u32 Disp_X,Disp_Y;
        u16 phaseh, phasew;
        u8  Scale_half_Y;

        /*
           Lucian: 為節省頻寬, Playback Video 時,僅使用 half of Height (ex: 640x480 -->640x240) 當作source image
                   去做scaling.
        */

        Scale_half_Y= (sizeY > 360) ? 1: 0; //Lucian: height of source image 若大於360 lines,才做 half_scaling up;
        if(sysTVOutOnFlag) //TV-out
        {
            if(TvOutMode == SYS_TV_OUT_PAL) //PAL mode
            {
                Disp_X  = TVOUT_X;
                Disp_Y  = TVOUT_Y_PAL;
            }
            else //NTSC mode
            {
                Disp_X  = TVOUT_X;
                Disp_Y  = TVOUT_Y_NTSC;
            }
        }
        else //Pannel-out
        {
               Disp_X=PANNEL_X;
               Disp_Y=PANNEL_Y;
        }

    	#if (LCM_OPTION == LCM_CCIR601_640x480P)

        #else
           if(Scale_half_Y)
               sizeY >>= 1;
    	#endif

        isuRst();
        IsuSCA_MODE =       ISU_Manual_Mode |
                            ISU_PROC_2DMode|
                            ISU_SRC_DRAM|
                            ISU_FBR_SEL_0|
                         #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
                            ISU_PKSRGB |
                         #endif
                            ISU_PKW_SEL_0;

        IsuSCA_FUN  =   ISU_PKOUT_ENA|
                        ISU_MSFEN_ENA;

        IsuSCA_INTC =   ISU_FCINTE_ENA|
                        ISU_PKOINTE_DISA;

        IsuSRC_SIZE =   sizeX << ISU_SRC_W_SHFT |
                        sizeY << ISU_SRC_H_SHFT;

        IsuPK_SIZE  =   (Disp_X<< ISU_PK_W_SHFT) |
                        (Disp_Y<< ISU_PK_H_SHFT);



        StepX       = isuSize2Step(Disp_X, sizeX, &phasew);
        StepY       = isuSize2Step(Disp_Y, sizeY, &phaseh);
        IsuMSF_STEP = (StepX << ISU_MSTEPX_SHFT) |
                                     (StepY << ISU_MSTEPY_SHFT);
        IsuMSF_PHA  = (phasew << ISU_MPHASEX_SHFT) + (phaseh << ISU_MPHASEY_SHFT);


        IsuSRC_IADDR0   =   (u32)Srcbuf;

        /*
           Lucian: 為節省頻寬, Playback Video 時,僅使用 half of Height (ex: 640x480 -->640x240) 當作source image
                   去做scaling.
        */
      #if (LCM_OPTION == LCM_CCIR601_640x480P)
        IsuSRC_STRIDE   = sizeX << 1;
      #else
        if(Scale_half_Y)
           IsuSRC_STRIDE   = sizeX << 2;
        else
           IsuSRC_STRIDE   = sizeX << 1;
      #endif


        IsuPK_IADDR0    =   (u32)Dstbuf;

        #if ((LCM_OPTION == LCM_HX8224_SRGB)||(LCM_OPTION == LCM_TD024THEB2_SRGB))
            IsuPK_STRIDE    =   Disp_X;
        #else
            IsuPK_STRIDE    =   Disp_X<<1;
        #endif

        IsuFIFO_TH = 0x5377777f;

        isuRst();

        isuStatus_OnRunning    = 1;/*BJ 0428*/
        IsuSCA_EN           =   ISU_ENA | ISU_Single_ENA;

        return 1;
    }
    /*Peter 1109 E*/
    s32 isuScalar_D2D(
                           u8 *Srcbuf , u8 *Dstbuf ,
                           u16 sizeX_src, u16 sizeY_src,
                           u16 sizeX_dst, u16 sizeY_dst
                          )
    {
        u8 err,EnSubScalarflag = 0;
        u32 StepX,StepY;

        if(sizeX_src == 0 || sizeY_src == 0 || sizeX_dst == 0 || sizeY_dst == 0)
        {
            DEBUG_ISU("Error: isuScalar_D2D(0x%08x, 0x%08x, %d, %d, %d, %d) have illegal parameter\n", Srcbuf, Dstbuf, sizeX_src, sizeY_src, sizeX_dst, sizeY_dst);
            return 0;
        }

        StepX = sizeX_src*1024/sizeX_dst;
        StepY = sizeY_src*1024/sizeY_dst;

        ///if(((StepX>0x800)||(StepY>0x800))) //Scalar down > 2x, for high scaling quality
            ///EnSubScalarflag = 1;

        isuRst();
        IsuSCA_MODE =   ISU_Manual_Mode |
                        ISU_PROC_2DMode|
                        ISU_SRC_DRAM|
                        ISU_FBR_SEL_0|
                        ISU_PKW_SEL_0;

        if(EnSubScalarflag == 0)
            IsuSCA_FUN  =   ISU_PKOUT_ENA|
                            ISU_MSFEN_ENA;
        else
            IsuSCA_FUN  =   ISU_PKOUT_ENA|
                            ISU_PNOUT_DISA|
                            ISU_SSFEN_ENA|
                            ISU_MSFEN_ENA;

        if(EnSubScalarflag== 0)
            IsuSCA_INTC =   ISU_FCINTE_ENA|
                            ISU_PKOINTE_DISA;
        else
            IsuSCA_INTC =   ISU_FCINTE_ENA|
                            ISU_PCOINTE_DISA|
                            ISU_PYOINTE_DISA;

        IsuSRC_SIZE =   sizeX_src << ISU_SRC_W_SHFT |
                        sizeY_src << ISU_SRC_H_SHFT;

        IsuPK_SIZE  =   (sizeX_dst<< ISU_PK_W_SHFT) |
                        (sizeY_dst<< ISU_PK_H_SHFT);

        if(EnSubScalarflag== 1)
            IsuPN_SIZE =    (sizeX_dst<< (ISU_PK_W_SHFT+1)) |
                            (sizeY_dst<< (ISU_PK_H_SHFT+1));

        if(EnSubScalarflag== 0)
        {
            IsuMSF_STEP =   ((sizeX_src*1024/sizeX_dst) << ISU_MSTEPX_SHFT)|
                            ((sizeY_src*1024/sizeY_dst) << ISU_MSTEPY_SHFT);
            IsuMSF_PHA  =   0;
        }
        else
        {
            IsuMSF_STEP =   ((sizeX_src*1024/(sizeX_dst<<1)) << ISU_MSTEPX_SHFT)|
                            ((sizeY_src*1024/(sizeY_dst<<1)) << ISU_MSTEPY_SHFT);
            IsuSSF_STEP =   (2048 << ISU_SSTEPX_SHFT)|
                            (2048 << ISU_SSTEPY_SHFT);
            IsuMSF_PHA  =   0;
            IsuSSF_PHA  =   0;
            IsuPN_STRIDE    =   ((sizeX_dst<<2) << ISU_PNYSTRIDE_SHFT)|
                                ((sizeX_dst<<1) << ISU_PNCSTRIDE_SHFT);
        }

        IsuSRC_IADDR0   =   (u32)Srcbuf;
        IsuSRC_STRIDE   =   sizeX_src<<1;

        IsuPK_IADDR0    =   (u32)Dstbuf;
        IsuPK_STRIDE    =   sizeX_dst<<1;

        IsuFIFO_TH      = 0x5377777f;

        isuRst();

        isuStatus_OnRunning= 1;/*BJ 0428*/
        IsuSCA_EN       =   ISU_ENA | ISU_Single_ENA;

        OSSemPend(isuSemEvt, ISU_TIMEOUT ,&err);
        if (err != OS_NO_ERR)
        {
            DEBUG_ISU("Error: isuSemEvt(isuScalar_D2D) is %d.\n", err);
        }

        return 1;
    }

    s32 isuScalar_D2D_SRGBout(
                           u8 *Srcbuf , u8 *Dstbuf ,
                           u16 sizeX_src, u16 sizeY_src,
                           u16 sizeX_dst, u16 sizeY_dst
                          )
    {
        u8 err,EnSubScalarflag = 0;
        u32 StepX,StepY;

        if(sizeX_src == 0 || sizeY_src == 0 || sizeX_dst == 0 || sizeY_dst == 0)
        {
            DEBUG_ISU("Error: isuScalar_D2D_SRGBout(0x%08x, 0x%08x, %d, %d, %d, %d) have illegal parameter\n", Srcbuf, Dstbuf, sizeX_src, sizeY_src, sizeX_dst, sizeY_dst);
            return 0;
        }

        StepX = sizeX_src*1024/sizeX_dst;
        StepY = sizeY_src*1024/sizeY_dst;

        isuRst();
        IsuSCA_MODE =  ISU_Manual_Mode |
                                    ISU_PROC_2DMode|
                                    ISU_SRC_DRAM|
                                    ISU_FBR_SEL_0|
                                    ISU_PKSRGB |
                                    ISU_PKW_SEL_0;

        if(EnSubScalarflag == 0)
            IsuSCA_FUN  =ISU_PKOUT_ENA|
                                    ISU_MSFEN_ENA;
        else
            IsuSCA_FUN  =ISU_PKOUT_ENA|
                                    ISU_PNOUT_DISA|
                                    ISU_SSFEN_ENA|
                                    ISU_MSFEN_ENA;

        if(EnSubScalarflag== 0)
            IsuSCA_INTC =ISU_FCINTE_ENA|
                                    ISU_PKOINTE_DISA;
        else
            IsuSCA_INTC =ISU_FCINTE_ENA|
                                    ISU_PCOINTE_DISA|
                                    ISU_PYOINTE_DISA;

        IsuSRC_SIZE =   sizeX_src << ISU_SRC_W_SHFT |
                                   sizeY_src << ISU_SRC_H_SHFT;

        IsuPK_SIZE  =    (sizeX_dst<< ISU_PK_W_SHFT) |
                                   (sizeY_dst<< ISU_PK_H_SHFT);

        if(EnSubScalarflag== 1)
            IsuPN_SIZE =   (sizeX_dst<< (ISU_PK_W_SHFT+1)) |
                                    (sizeY_dst<< (ISU_PK_H_SHFT+1));

        if(EnSubScalarflag== 0)
        {
            IsuMSF_STEP =   ((sizeX_src*1024/sizeX_dst) << ISU_MSTEPX_SHFT)|
                            ((sizeY_src*1024/sizeY_dst) << ISU_MSTEPY_SHFT);
            IsuMSF_PHA  =   0;
        }
        else
        {
            IsuMSF_STEP =   ((sizeX_src*1024/(sizeX_dst<<1)) << ISU_MSTEPX_SHFT)|
                            ((sizeY_src*1024/(sizeY_dst<<1)) << ISU_MSTEPY_SHFT);
            IsuSSF_STEP =   (2048 << ISU_SSTEPX_SHFT)|
                            (2048 << ISU_SSTEPY_SHFT);
            IsuMSF_PHA  =   0;
            IsuSSF_PHA  =   0;
            IsuPN_STRIDE    =   ((sizeX_dst<<2) << ISU_PNYSTRIDE_SHFT)|
                                ((sizeX_dst<<1) << ISU_PNCSTRIDE_SHFT);
        }

        IsuSRC_IADDR0   =   (u32)Srcbuf;
        IsuSRC_STRIDE   =   sizeX_src<<1;

        IsuPK_IADDR0    =   (u32)Dstbuf;

        IsuPK_STRIDE    =   sizeX_dst;

        IsuFIFO_TH      = 0x5377777f;

        isuRst();

            isuStatus_OnRunning= 1;/*BJ 0428*/
        IsuSCA_EN       =   ISU_ENA | ISU_Single_ENA;

        OSSemPend(isuSemEvt, ISU_TIMEOUT ,&err);
        if (err != OS_NO_ERR)
        {
            DEBUG_ISU("Error: isuSemEvt(isuScalar_D2D) is %d.\n", err);
        }

        return 1;
    }

    s32 isuScalar_D2D_Deblock(
                           u8 *Srcbuf , u8 *Dstbuf ,
                           u16 sizeX_src, u16 sizeY_src,
                           u16 sizeX_dst, u16 sizeY_dst
                          )
    {
        u8 err,EnSubScalarflag = 0;
        u32 StepX,StepY;

        if(sizeX_src == 0 || sizeY_src == 0 || sizeX_dst == 0 || sizeY_dst == 0)
        {
            DEBUG_ISU("Error: isuScalar_D2D_Deblock(0x%08x, 0x%08x, %d, %d, %d, %d) have illegal parameter\n", Srcbuf, Dstbuf, sizeX_src, sizeY_src, sizeX_dst, sizeY_dst);
            return 0;
        }

        StepX = sizeX_src*1024/sizeX_dst;
        StepY = sizeY_src*1024/sizeY_dst;

        //if(((StepX>0x800)||(StepY>0x800))) //Scalar down > 2x, for high scaling quality
            //EnSubScalarflag = 1;

        isuRst();
        IsuSCA_MODE =   ISU_Manual_Mode |
                        ISU_PROC_2DMode|
                        ISU_SRC_DRAM|
                        ISU_FBR_SEL_0|
                        ISU_PKW_SEL_0;

        if(EnSubScalarflag == 0)
            IsuSCA_FUN  =   ISU_PKOUT_ENA|
                            ISU_MSFEN_ENA;
        else
            IsuSCA_FUN  =   ISU_PKOUT_ENA|
                            ISU_PNOUT_DISA|
                            ISU_SSFEN_ENA|
                            ISU_MSFEN_ENA;

        if(EnSubScalarflag== 0)
            IsuSCA_INTC =   ISU_FCINTE_ENA|
                            ISU_PKOINTE_DISA;
        else
            IsuSCA_INTC =   ISU_FCINTE_ENA|
                            ISU_PCOINTE_DISA|
                            ISU_PYOINTE_DISA;

        IsuSRC_SIZE =   sizeX_src << ISU_SRC_W_SHFT |
                        sizeY_src << ISU_SRC_H_SHFT;

        IsuPK_SIZE  =   (sizeX_dst<< ISU_PK_W_SHFT) |
                        (sizeY_dst<< ISU_PK_H_SHFT);

        if(EnSubScalarflag== 1)
            IsuPN_SIZE =    (sizeX_dst<< (ISU_PK_W_SHFT+1)) |
                            (sizeY_dst<< (ISU_PK_H_SHFT+1));

        if(EnSubScalarflag== 0)
        {
            IsuMSF_STEP =   ((sizeX_src*1024/sizeX_dst) << ISU_MSTEPX_SHFT)|
                            ((sizeY_src*1024/sizeY_dst) << ISU_MSTEPY_SHFT);
            IsuMSF_PHA  =   0;
        }
        else
        {
            IsuMSF_STEP =   ((sizeX_src*1024/(sizeX_dst<<1)) << ISU_MSTEPX_SHFT)|
                            ((sizeY_src*1024/(sizeY_dst<<1)) << ISU_MSTEPY_SHFT);
            IsuSSF_STEP =   (2048 << ISU_SSTEPX_SHFT)|
                            (2048 << ISU_SSTEPY_SHFT);
            IsuMSF_PHA  =   0;
            IsuSSF_PHA  =   0;
            IsuPN_STRIDE    =   ((sizeX_dst<<2) << ISU_PNYSTRIDE_SHFT)|
                                ((sizeX_dst<<1) << ISU_PNCSTRIDE_SHFT);
        }

        IsuSRC_IADDR0   =   (u32)Srcbuf;
        IsuSRC_STRIDE   =   sizeX_src<<1;

        IsuPK_IADDR0    =   (u32)Dstbuf;
        IsuPK_STRIDE    =   sizeX_dst<<1;

        IsuFIFO_TH      = 0x5377777f;
        isuRst();

        isuStatus_OnRunning= 1;/*BJ 0428*/
        IsuSCA_EN       =   ISU_ENA | ISU_Single_ENA;

        /*
        OSSemPend(isuSemEvt, ISU_TIMEOUT ,&err);
        if (err != OS_NO_ERR)
        {
            DEBUG_ISU("Error: isuSemEvt(isuScalar_D2D) is %d.\n", err);
        }
        */

        return 1;
    }

    
#else
    void isuIntHandler(void)
    {}
    void isuIntProcess_MPEGAVI(void)
    {}

    void isuIntProcess_PREVIEW(void)
    {}

    void isuIntProcess_CAP_PREVIEW(void)
    {}


    void isuOutputAddrArrange_TV(void)
    {}

    void isuOutputAddrArrange_Sensor(void)
    {}


    s32 isuScUpZoom(s32 zoomFactor)
    {return 0;}

    /*

    Routine Description:

        Preview.

    Arguments:

        zoomFacor - Zoom factor.

    Return Value:

        0 - Failure.
        1 - Success.

    */

    s32 isuPreview(s8 zoomFactor)
    {
       return 0;
    }

    s32 isuCapturePreviewImg(s8 zoomFactor)
    {
       return 0;
    }


     //Note: 只以PA9002D 用此一functon.(Lucian)
    s32 isuPreviewZoom(s8 zoomFactor)
    {
       return 0;
    }


    u16 isuSize2Step(u16 dst_size, u16 src_size, u16 *phase)
    {
        return 0;
    }

    /*
    Routine Description:

        Capture primary.

    Arguments:

        None.

    Return Value:

        0 - Failure.
        1 - Success.

    */

    s32 isuCapture640x480_B2F(s32 zoomFactor) /*BJ 0530 S*/
    {
        return 0;
    }

    s32 isuCapturePrimary_B2F(s32 zoomFactor) /*BJ 0530 S*/
    {
        return 0;
    }

    s32 isuCapturePrimary_B2B(s32 zoomFactor) /*BJ 0530 S*/
    {
        return 0;
    }

    void isuSetImageOutResolution(u16 width, u16 height)
    {

    }


    /*BJ 0530 S*/
    s32 isuCaptureVideo(s32 zoomFactor)
    {
        return 0;
    }

    void isuOverlayImgConfig()
    {
    }

     //Note: 只以PA9002D 用此一functon.(Lucian)
    s32 isuVideoZoom(s32 zoomFactor)
    {
        return 0;
    }
    /*BJ 0530 E*/

    
    /*

    Routine Description:

        Capture thumbnail.

    Arguments:

        None.

    Return Value:

        0 - Failure.
        1 - Success.

    */
    s32 isuScalar2Thumbnail(s32 zoomFactor) /*BJ 0530 S*/
    {
         return 1;
    }

    /*

    Routine Description:

        Reset the Image Scaling Unit.

    Arguments:

        None.

    Return Value:

        None.

    */
    void isuRst(void)
    {

    }

    void isuStop(void)
    {

    }

    s32 isuPlayback(u8 *Srcbuf , u8 *Dstbuf , u16 sizeX, u16 sizeY)
    {
        return 1;
    }

    s32 isuGenPauseFrame(u8 *Srcbuf , u8 *Dstbuf , u16 sizeX, u16 sizeY)
    {
       return 1;
    }
    s32 isuPlayback_av(u8 *Srcbuf , u8 *Dstbuf , u16 sizeX, u16 sizeY)
    {
       return 1;
    }
    /*Peter 1109 E*/
    s32 isuScalar_D2D(
                           u8 *Srcbuf , u8 *Dstbuf ,
                           u16 sizeX_src, u16 sizeY_src,
                           u16 sizeX_dst, u16 sizeY_dst
                          )
    {
       return 1;
    }

    s32 isuScalar_D2D_SRGBout(
                           u8 *Srcbuf , u8 *Dstbuf ,
                           u16 sizeX_src, u16 sizeY_src,
                           u16 sizeX_dst, u16 sizeY_dst
                          )
    {

       return 1;
    }

    s32 isuScalar_D2D_Deblock(
                           u8 *Srcbuf , u8 *Dstbuf ,
                           u16 sizeX_src, u16 sizeY_src,
                           u16 sizeX_dst, u16 sizeY_dst
                          )
    {
       return 1;
    }

    
    
#endif
