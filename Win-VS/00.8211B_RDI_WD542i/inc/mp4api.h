/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	mp4api.h

Abstract:

   	The application interface of the 3GPP/MP4 file.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __MP4_API_H__
#define __MP4_API_H__

/* function prototype */

extern s32 mp4CaptureVideo(s32);/*BJ 0530 S*/
extern s32 mp4SetVideoResolution(u16, u16);

extern s32 mp4ReadFile(void);

extern s32 mp4Init(void);
extern void mp4Test(void);

#endif
