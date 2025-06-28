/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbmscfs.c

Abstract:

   	USB Mass Storage Class callback routine.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"

#include "task.h"
#include "board.h"
#include "usb.h"
#include "usbapiex.h"
#include "usbcb.h"
#include "usbmsc.h"
#include "usbdev.h"

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 usbMscInitCb(void);
void usbMscCbClassReq(u32);

s32 usbMscClassReqReset(void);
s32 usbMscClassReqGetMaxLun(void);

extern s32 usbEpStall(u8);

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
//#define usbMscCbDebugPrint 				printf
/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

extern u32 usbMscCurLun;

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Initialize callback function.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscInitCb(void)
{
    if (usbApiSetCallback(USB_CB_CLASS_REQ, usbMscCbClassReq) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

	Callback function of class request.

Arguments:

	pDevReq - Device request.

Return Value:

	None.

*/
void usbMscCbClassReq(u32 pDeviceRequest)
{
    USB_DEV_REQ *pDevReq = (USB_DEV_REQ*) pDeviceRequest;

    /* decode class request */
    if ((pDevReq->bmRequestType == USB_MSC_RESET_bmRequestType) &&
            (pDevReq->bRequest == USB_MSC_RESET_bRequest) &&
            (pDevReq->wValue == USB_MSC_RESET_wValue) &&
            (pDevReq->wLength == USB_MSC_RESET_wLength))
    {	/* Bulk-Only Mass Storage Reset */
        usbMscClassReqReset();
    }
    else if ((pDevReq->bmRequestType == USB_MSC_GET_MAX_LUN_bmRequestType) &&
             (pDevReq->bRequest == USB_MSC_GET_MAX_LUN_bRequest) &&
             (pDevReq->wValue == USB_MSC_GET_MAX_LUN_wValue) &&
             (pDevReq->wLength == USB_MSC_GET_MAX_LUN_wLength))
    {	/* Get Max LUN */
        usbMscClassReqGetMaxLun();
    }
    else
    {	/* invalid request */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }
}

/*

Routine Description:

	Class request of reset.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscClassReqReset(void)
{
    return(usbApiTerminate());
}

/*

Routine Description:

	Class request of get max lun.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbMscClassReqGetMaxLun(void)
{
    USB_IO_REQ ioReq;
    __align(4) u8 mscLun = USB_MSC_MAX_LUN;
    u32 size;
    u8 stat;

    memset((void*)&ioReq, 0, sizeof(USB_IO_REQ));
    ioReq.epNum    = USB_EP_CTRLIN;
    ioReq.pData    = &mscLun;
    ioReq.reqSize  = 1;
    ioReq.pRetSize = &size;
    ioReq.pRetStat = &stat;

    return(usbApiSend(&ioReq));
}

