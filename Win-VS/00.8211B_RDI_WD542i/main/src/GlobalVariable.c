/*

Copyright (c) 2012 Mars Semiconductor Corp.

Module Name:

    GlobalVariable.c

Abstract:

    The routines of global variable declaring.

Environment:

    ARM RealView Developer Suite

Revision History:
    
    2012/05/08  Peter Hsu  Create.

*/


#include "general.h"


#include "board.h"
#include "iisapi.h"
#include "mpeg4api.h"
#include "asfapi.h"
#include "GlobalVariable.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Global Variable
 *********************************************************************************************************
 */
u8  GMotionTrigger[MULTI_CHANNEL_MAX];  /*0: not trigger, 1 : trigger*/

#if MULTI_CHANNEL_VIDEO_REC
    VIDEO_CLIP_OPTION       VideoClipOption[MULTI_CHANNEL_MAX];
    VIDEO_CLIP_PARAMETER    VideoClipParameter[MULTI_CHANNEL_MAX];
  #if(MULTI_CHANNEL_LOCAL_MAX)
    VIDEO_BUF_MNG           MultiChannelVideoBufMng[MULTI_CHANNEL_LOCAL_MAX][VIDEO_BUF_NUM];
    IIS_BUF_MNG             MultiChanneliisSounBufMng[MULTI_CHANNEL_LOCAL_MAX][IIS_BUF_NUM];
  #else
    VIDEO_BUF_MNG           **MultiChannelVideoBufMng;
    IIS_BUF_MNG             **MultiChanneliisSounBufMng;
  #endif
    PVIDEO_CLIP_OPTION      pvcoRfiu[MAX_RFIU_UNIT];
  #if (BOOT_REC_ENABLE ==1)
    u32                     RfRxVideoRecordEnable   = (1 << (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX)) - 1; 
  #else
    u32                     RfRxVideoRecordEnable   = 0; 
  #endif
    OS_EVENT                *OverWriteDelReadySemEvt;

  #if ASF_MASS_WRITE
    __align(16) u8  MultiChannelAsfMassWriteData[MULTI_CHANNEL_MAX][ASF_MASS_WRITE_SIZE];
  #endif

  #if(G_SENSOR_DETECT)
    u8                  GSensorEvent;
  #endif

/*
 *********************************************************************************************************
 * External Variable Prototype
 *********************************************************************************************************
 */


/*****************************************************************************/
/* Function                                                  */
/*****************************************************************************/

void InitVideoClipOption(void)
{
    int i;
    
    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        //======== Video Clip Channel ==========================================================================================
        VideoClipOption[i].PackerTaskSemEvt         = OSSemCreate(1);

        //======== MPEG-4 ======================================================================================================
    
        VideoClipOption[i].video_double_field_flag  = 0;
        VideoClipOption[i].dftMpeg4Quality          = dftMpeg4Quality;    //³]©wVideo Clip Quality. Set Qp.
        //VideoClipOption[i].mpeg4VideoRecQulity      = MPEG4_VIDEO_QUALITY_HIGH;
        //VideoClipOption[i].mpeg4VideoRecFrameRate   = MPEG4_VIDEO_FRAMERATE_30;
        VideoClipOption[i].VideoRecFrameRate        = VideoRecFrameRate;

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
  #if(VIDEO_RESOLUTION_SEL== VIDEO_VGA_IN_VGA_OUT)
        VideoClipOption[i].mpeg4Width               = 640;
        VideoClipOption[i].mpeg4Height              = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_VGA_OUT)
        VideoClipOption[i].mpeg4Width               = 640;
        VideoClipOption[i].mpeg4Height              = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_1920x1440DECI3x3TOVGA)
        VideoClipOption[i].mpeg4Width               = 640;
        VideoClipOption[i].mpeg4Height              = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_HD_OUT)
        VideoClipOption[i].mpeg4Width               = 1280;
        VideoClipOption[i].mpeg4Height              = 720;
  #endif
#else
        VideoClipOption[i].mpeg4Width               = 640;
        VideoClipOption[i].mpeg4Height              = 480;
#endif
        VideoClipOption[i].double_field_cnt         = 30;

        //======== IIS ======================================================================================================
        VideoClipOption[i].AudioChannelID           = i;
        VideoClipOption[i].IISMode                  = 3;    // 0: record, 1: playback, 2: receive and playback audio in preview mode
        VideoClipOption[i].guiIISRecDMAId           = 0xFF;
        VideoClipOption[i].guiIISPlayDMAId          = 0xFF;

        //======== SYS ======================================================================================================
        VideoClipOption[i].sysCaptureVideoStop      = 1;

        //======== ASF ======================================================================================================
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
  #if(VIDEO_RESOLUTION_SEL== VIDEO_VGA_IN_VGA_OUT)
        VideoClipOption[i].asfVopWidth              = 640;
        VideoClipOption[i].asfVopHeight             = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_VGA_OUT)
        VideoClipOption[i].asfVopWidth              = 640;
        VideoClipOption[i].asfVopHeight             = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_HD_OUT)
        VideoClipOption[i].asfVopWidth              = 1280;
        VideoClipOption[i].asfVopHeight             = 720;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_1920x1440DECI3x3TOVGA)
        VideoClipOption[i].asfVopWidth              = 640;
        VideoClipOption[i].asfVopHeight             = 480;
  #endif
#else
        VideoClipOption[i].asfVopWidth              = 640;
        VideoClipOption[i].asfVopHeight             = 480;
#endif
        //VideoClipOption[i].curr_playback_speed      = 5;    //for UI level control
        //VideoClipOption[i].pre_playback_speed       = 5;    //for asf player level control  
        //VideoClipOption[i].video_playback_speed     = 5;    //for asf player level control  
        //VideoClipOption[i].asfSectionTime           = 30;   // section time per video captured file.
        //VideoClipOption[i].asfRecFileNum            = -1;   // less than 0: infinite, more than 0: video file number want to record?
        //VideoClipOption[i].asfRecTimeLen            = ASF_REC_TIME_LEN;     // Total recording time of event trigger per video, Second unit.
        //VideoClipOption[i].MotionlessRecTimeLen     = 10;                   //Max Motionless period
        //VideoClipOption[i].PreRecordTime            = 10;
        VideoClipOption[i].AV_TimeBase              = PREROLL; //For capture
        VideoClipOption[i].RTCseconds               = 0;
#if ASF_MASS_WRITE    /* Peter 070104 */
        VideoClipOption[i].asfMassWriteData         = (u8*)&MultiChannelAsfMassWriteData[i][0];
#endif

        //======== VideoClipParameter ======================================================================================================
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B)  || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
  #if(VIDEO_RESOLUTION_SEL== VIDEO_VGA_IN_VGA_OUT)
        VideoClipParameter[i].asfVopWidth           = 640;
        VideoClipParameter[i].asfVopHeight          = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_VGA_OUT)
        VideoClipParameter[i].asfVopWidth           = 640;
        VideoClipParameter[i].asfVopHeight          = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_HD_OUT)
        VideoClipParameter[i].asfVopWidth           = 1280;
        VideoClipParameter[i].asfVopHeight          = 720;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_1920x1440DECI3x3TOVGA)
        VideoClipParameter[i].asfVopWidth           = 640;
        VideoClipParameter[i].asfVopHeight          = 480;
  #endif
#else
        VideoClipParameter[i].asfVopWidth           = 640;
        VideoClipParameter[i].asfVopHeight          = 480;
#endif
        VideoClipParameter[i].sysCaptureVideoMode   = 0;
        VideoClipParameter[i].asfRecTimeLen         = ASF_REC_TIME_LEN;
    }
    OverWriteDelReadySemEvt = OSSemCreate(1);
}

#endif  // #if MULTI_CHANNEL_VIDEO_REC


