/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ipuapi.h

Abstract:

   	The application interface of Image Processing Unit

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __IPU_API_H__
#define __IPU_API_H__

extern s32 ipuPreview(s8);
extern s32 ipuCapturePrimary(void);
extern s32 ipuCaptureVideo(void);/*BJ 0530 S*/

extern s32 ipuInit(void);
extern void ipuIntHandler(void);
extern void ipuTest(void);
extern s32 ipuGetOutputSize(u16*, u16*);
extern s32 ipuSetIOSize(u16, u16);
extern void ipuStop(void);
extern s32 ipuSetColorCorrMatrix(s16*);


#endif
