/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	Video4api.h

Abstract:

   	The application interface of the Video encoder/decoder.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/01	Lsk	Create	

*/

#ifndef __VIDEO_API_H__
#define __VIDEO_API_H__

#include "MPEG4api.h"
/* constant */

/*----------------- flag of frame-----------------*/
#define VIDEO_TYPE_I    0x00000001

#define VIDEO_QUALITY_HIGH    0
#define VIDEO_QUALITY_MEDIUM  1
#define VIDEO_QUALITY_LOW     2

#define VIDEO_FRAMERATE_30    0
#define VIDEO_FRAMERATE_15    1
#define VIDEO_FRAMERATE_5     2
#define VIDEO_FRAMERATE_10    3
#define VIDEO_FRAMERATE_60    4

#define VIDEO_N_VOP                 2

#define VIDEO_BITRATE_LEVEL_100      0
#define VIDEO_BITRATE_LEVEL_80       1
#define VIDEO_BITRATE_LEVEL_60       2

#define VIDEO_MIN_BUF_SIZE MPEG4_MIN_BUF_SIZE

/*----------------- type definition --------------*/
typedef struct _VIDEO_INFO
{   
    int  Height;
    int  Width;
    u32  FrameType;
    s64* FrameTime;
    u32  FrameIdx;    
    u32* pSize; 
    u32  BufStartBits;
	u8*  StreamBuf;
    u8   ResetFlag;
} VIDEO_INFO;

extern VIDEO_INFO video_info;        
/*----------------- variable ---------------------*/
/* task and event related */
extern OS_STK VideoTaskStack[]; 
extern OS_EVENT*   VideoTrgSemEvt;
extern OS_EVENT*   VideoCmpSemEvt;
extern OS_EVENT*   VideoCpleSemEvt; 

extern u8 Video_Task_Mode;     // 0: record, 1: playback
extern u8 Video_Task_Error;
extern u8 Video_Task_Pend;
extern u8 Video_Task_Go;       // 0: never run, 1: ever run

/* buffer management */
extern u32 VideoBufMngReadIdx;
extern u32 VideoBufMngWriteIdx;
extern u8* VideoBufEnd;
extern u8 *VideoNRefBuf_Y;
extern u8 *VideoNRefBuf_Cb;
extern u8 *VideoPRefBuf_Y;
extern u8 *VideoPRefBuf_Cb;
extern VIDEO_BUF_MNG VideoBufMng[]; //Lsk : for compiler pass
//extern VIDEO_BUF_MNG VideoBufMng[]; 
/* Container info*/
extern u32 VideoTimeStatistics;

extern s32 VideoTaskResume(void);
extern s32 VideoTaskSuspend(void);
extern s32 VideoCodecInit(void);

#endif
