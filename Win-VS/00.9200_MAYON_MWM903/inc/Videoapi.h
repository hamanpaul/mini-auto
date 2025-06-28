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

typedef struct _VIDEO_BUF_MNG
{
	u32	flag;
	u32	asfflag;
	s64	time;
	u32	size;
    u32 offset;  //Lucian: record the offset of  bottom filed bitstream start position.
	u8* buffer;	
} VIDEO_BUF_MNG;


        
/*----------------- variable ---------------------*/
extern s32 ResumeVideoTask(void);
extern s32 SuspendVideoTask(void);
#endif
