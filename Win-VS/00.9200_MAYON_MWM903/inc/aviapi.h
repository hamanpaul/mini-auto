/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	aviapi.h

Abstract:

   	The application interface of the AVI file.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/06/01	Peter Hsu	Create	

*/

#ifndef __AVI_API_H__
#define __AVI_API_H__

/* function prototype */

extern s32 aviCaptureVideo(s32, u32 Mode);/*Peter 0619 S*/
extern s32 aviSetVideoResolution(u16, u16);

extern s32 aviReadFile(void);

extern s32 aviInit(void);
extern void aviTest(void);
extern s32 aviCaptureVideoStop(void);
extern s32 aviPlaybackVideoStop(void);

extern s64 VideoNextPresentTime;
#endif
