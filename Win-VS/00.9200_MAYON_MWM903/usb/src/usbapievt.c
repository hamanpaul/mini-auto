/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbapievt.c

Abstract:

   	USB API event handler.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"

#include "task.h"
#include "board.h"
#include "usb.h"
#include "usbreg.h"
#include "usbdev.h"
#include "usbcb.h"
#include "usbapievt.h"

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

void usbApiEvtTerminate(void);
void usbApiEvtSend(void);
void usbApiEvtRecv(void);
void usbApiEvtEpCancel(void);
void usbApiEvtEpReset(void);
void usbApiEvtEpStall(void);
void usbApiEvtEpClearStall(void);
void usbApiEvtRemoteWakeup(void);
void usbApiEvtQueryStatus(void);

s32 usbApiNotify(void);
s32 usbApiSendRecvNotify(u8);

extern void usbCbAttach(u32);
extern void usbCbDetach(u32);
extern void usbCbBusReset(u32);
extern void usbCbBusSuspend(u32);
extern void usbCbGetStatus(u32);
extern void usbCbClearFeature(u32);
extern void usbCbSetFeature(u32);
extern void usbCbSetAddress(u32);
extern void usbCbGetDescriptor(u32);
extern void usbCbSetDescriptor(u32);
extern void usbCbGetConfiguration(u32);
extern void usbCbSetConfiguration(u32);
extern void usbCbGetInterface(u32);
extern void usbCbSetInterface(u32);
extern void usbCbSynchFrame(u32);
extern void usbCbClassReq(u32);
extern void usbCbVendorReq(u32);

extern s32 usbEpCancel(u8);
extern s32 usbEpReset(u8);
extern s32 usbEpClearFifo(u8);
extern s32 usbEpStall(u8);
extern s32 usbEpClearStall(u8);
extern s32 usbRemoteWakeup(void);
extern s32 usbGetEpReadData(u8, u8*, u32*);
extern s32 usbEpReadData(u8, u8*, u32);
extern s32 usbEpWriteData(u8, u8*, u32);
extern s32 usbSetReadData(u32, u8*, u32);
extern s32 usbSetWriteData(u32, u8*, u32);
extern s32 usbSetReadDataDma(u32, u8*, u32);
extern s32 usbSetWriteDataDma(u32, u8*, u32);
extern s32 usbCheckDmaReady(void);

extern s32 usbGetEpReq(u8, USB_EP_REQ*);

extern s32 usbInitCtrlStatusStage(u8);
extern s32 usbSetCtrlIn(void);
extern s32 usbSetCtrlOut(void);
extern s32 usbSetBulkIn(void);
extern s32 usbSetBulkOut(void);
extern s32 usbSetIntrIn(void);
extern s32 usbSetIsoIn(void);

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
#define usbApiEvtDebugPrint 				printf

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

/* API event function */
void (*usbApiEvtFunc[])(void) =
    {
        usbApiEvtTerminate,		/* 0x00 - terminate */
        usbApiEvtSend,			/* 0x01 - send */
        usbApiEvtRecv,			/* 0x02 - receive */
        usbApiEvtEpCancel,		/* 0x03 - cancel */
        usbApiEvtEpReset,		/* 0x04 - endpoint reset */
        usbApiEvtEpStall,		/* 0x05 - endpoint stall */
        usbApiEvtEpClearStall,		/* 0x06 - endpoint clear stall */
        usbApiEvtRemoteWakeup,		/* 0x07 - remote wakeup */
        usbApiEvtQueryStatus,		/* 0x08 - query status */
    };

extern u32 usbEpMaxPktSize[];
extern u32 usbEpMaxDmaSize[];

extern OS_EVENT* usbSemApi[];
extern OS_EVENT* usbSemApiNotify[];

extern USB_API_EVT_SNAP usbApiCurEvt;

extern USB_DEV_SETTING usbDevSetting;

extern OS_EVENT* usbSemPrevSend;

//extern OS_EVENT* usbSemStatOK;
extern u8 usbScsiRead10;
extern u8 usbScsiWrite10;
extern u8 TheLastRecv;
extern u8 usbDmaRxCmpl;

extern OS_EVENT* usbSemPerfLastBlock;
extern OS_EVENT* usbSemPerfCritSec;
extern u8 ucbPerfLastBlock;


/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Terminate.

Arguments:

	None.

Return Value:

	None.

*/
void usbApiEvtTerminate(void)
{
    //DEBUG_USB("Trace: usbApiEvtTerminate()\n");

    /* notify the completion */
    usbApiNotify();
}

/*

Routine Description:

	Send.

Arguments:

	None.

Return Value:

	None.

*/
void usbApiEvtSend(void)
{
    u8 epNum = 0;
    USB_EP_REQ epReq;

    //DEBUG_USB("Trace: usbApiEvtSend()\n");

    epNum = usbApiCurEvt.epNum;

    /* check if endpoint number valid */
    if (epNum >= USB_EP_MAX)
    {
        DEBUG_USB("Error: Invalid endpoint number (%d).\n", epNum);
        return;
    }

    /* get endpoint request */
    if (usbGetEpReq(epNum, &epReq) == 0) /* error to get endpoint request */
    {
        DEBUG_USB("Error: Get endpoint request error.\n");
        return;
    }

    switch (epNum)
    {
    case USB_EP_CTRLIN:
        /* initialize dma of control in status stage */
        usbInitCtrlStatusStage(USB_EP_CTRLOUT);
        usbSetCtrlIn();
        break;

    case USB_EP_BULKIN:
        usbSetBulkIn();
        break;

    case USB_EP_INTRIN:
        usbSetIntrIn();
        break;

    case USB_EP_ISOIN:
        usbSetIsoIn();
        break;

    default:
        /* Error endpoint */
        break;
    }
}

/*

Routine Description:

	Receive.

Arguments:

	None.

Return Value:

	None.

*/
void usbApiEvtRecv(void)
{
    u8 epNum = 0;
    USB_EP_REQ epReq;

    //DEBUG_USB("Trace: usbApiEvtRecv()\n");

    epNum = usbApiCurEvt.epNum;

    /* check if endpoint number valid */
    if (epNum >= USB_EP_MAX)
    {
        DEBUG_USB("Error: Invalid endpoint number (%d).\n", epNum);
        return;
    }

    /* get endpoint request */
    if (usbGetEpReq(epNum, &epReq) == 0) /* error to get endpoint request */
    {
        DEBUG_USB("Error: Get endpoint request error.\n");
        return;
    }

    switch (epNum)
    {
    case USB_EP_CTRLOUT:
        /* initialize dma of control in status stage */
        usbInitCtrlStatusStage(USB_EP_CTRLIN);
        usbSetCtrlOut();
        break;

    case USB_EP_BULKOUT:
        usbSetBulkOut();
        break;

    default:
        /* Error endpoint */
        break;
    }
}

/*

Routine Description:

	Cancel.

Arguments:

	None.

Return Value:

	None.

*/
void usbApiEvtEpCancel(void)
{
    u8 epNum = 0;

    //DEBUG_USB("Trace: usbApiEvtEpCancel()\n");

    epNum = usbApiCurEvt.epNum;

    /* check if endpoint number valid */
    if (epNum >= USB_EP_MAX)
        return;

    /* cancel endpoint transfer */
    usbEpCancel(epNum);

    /* notify the completion */
    usbApiNotify();
}

/*

Routine Description:

	 Endpoint reset.

Arguments:

	None.

Return Value:

	None.

*/
void usbApiEvtEpReset(void)
{
    u8 epNum = 0;

    //DEBUG_USB("Trace: usbApiEvtEpReset()\n");

    epNum = usbApiCurEvt.epNum;

    /* check if endpoint number valid */
    if (epNum >= USB_EP_MAX)
        return;

    /* reset endpoint */
    usbEpReset(epNum);

    /* clear fifo of endpoint */
    usbEpClearFifo(epNum);

    /* notify the completion */
    usbApiNotify();
}

/*

Routine Description:

	Endpoint stall.

Arguments:

	None.

Return Value:

	None.

*/
void usbApiEvtEpStall(void)
{
    u8 epNum = 0;

    //DEBUG_USB("Trace: usbApiEvtEpStall()\n");

    epNum = usbApiCurEvt.epNum;

    /* check if endpoint number valid */
    if (epNum >= USB_EP_MAX)
        return;

    /* endpoint stall */
    usbEpStall(epNum);

    /* notify the completion */
    usbApiNotify();
}

/*

Routine Description:

	Endpoint clear stall.

Arguments:

	None.

Return Value:

	None.

*/
void usbApiEvtEpClearStall(void)
{
    u8 epNum = 0;

    //DEBUG_USB("Trace: usbApiEvtEpClearStall()\n");

    epNum = usbApiCurEvt.epNum;

    /* check if endpoint number valid */
    if (epNum >= USB_EP_MAX)
        return;

    /* endpoint stall */
    usbEpClearStall(epNum);

    /* notify the completion */
    usbApiNotify();
}

/*

Routine Description:

	Remote wakeup.

Arguments:

	None.

Return Value:

	None.

*/
void usbApiEvtRemoteWakeup(void)
{
    //DEBUG_USB("Trace: usbApiEvtRemoteWakeup()\n");

    /* remote wakeup */
    usbRemoteWakeup();

    /* notify the completion */
    usbApiNotify();
}

/*

Routine Description:

	Query status.

Arguments:

	None.

Return Value:

	None.

*/
void usbApiEvtQueryStatus(void)
{
    //DEBUG_USB("Trace: usbApiEvtQueryStatus()\n");

    /* TBD: update endpoint status if necesssay */

    /* notify the completion */
    usbApiNotify();
}

/*

Routine Description:

	Notify completion of an api.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiNotify(void)
{
    if (usbApiCurEvt.option & USB_API_OPT_NOTIFY)
    {
        /* release semaphore */
        OSSemPost(usbSemApiNotify[usbApiCurEvt.cause]);
    }

    return 1;
}

/*

Routine Description:

	Notify completion of an api.

Arguments:

	send - Send or receive.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiSendRecvNotify(u8 send)
{
    u8 ucLastBlock;
    u8 err;

    if (send)
    {
        if (usbScsiRead10 == 0)
            /* release semaphore */
            OSSemPost(usbSemApiNotify[USB_SEM_API_NOTIFY_SEND]);

        if (usbScsiRead10 == 1)
        {
            OSSemPost(usbSemPrevSend);

            OSSemPend(usbSemPerfCritSec, USB_TIMEOUT, &err);
            ucLastBlock = ucbPerfLastBlock;
            OSSemPost(usbSemPerfCritSec);

            if (ucLastBlock == 1)
                OSSemPost(usbSemPerfLastBlock);
        }

    }
    else
    {
        /* release semaphore */
        OSSemPost(usbSemApiNotify[USB_SEM_API_NOTIFY_RECV]);
    }

    return 1;
}

