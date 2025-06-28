/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbapiex.h

Abstract:

   	The declarations of USB API related.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_API_EX_H__
#define __USB_API_EX_H__

/* Structure */

/* I/O request */
typedef struct _USB_IO_REQ
{
    u8	epNum;		/* endpoint number */
    u8*	pData;		/* data requested */
    u32	reqSize; 	/* data size requested */
    u8	reqOption;	/* option requested */
    u32*	pRetSize;	/* data size to return */
    u8*	pRetStat;	/* status to return */
}
USB_IO_REQ;

/* Constant */

/* Function prototype */
extern s32 usbApiInit(void);
extern s32 usbApiTerminate(void);
extern s32 usbApiSend(USB_IO_REQ*);
extern s32 usbApiSendWait(USB_IO_REQ*);
extern s32 usbApiRecv(USB_IO_REQ*);
extern s32 usbApiRecvWait(USB_IO_REQ*);
extern s32 usbApiEpCancel(u8);
extern s32 usbApiEpAbort(u8);
extern s32 usbApiEpReset(u8);
extern s32 usbApiEpStall(u8, u8);
extern s32 usbApiQueryStatus(u8, u16*);
extern s32 usbApiSetCallback(u8, void (*cbFunc)(u32));

#endif
