/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbapiex.c

Abstract:

   	USB API.

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
#include "usbapiex.h"
#include "usbapievt.h"

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 usbApiInit(void);
s32 usbApiTerminate(void);
s32 usbApiSend(USB_IO_REQ*);
s32 usbApiSendWait(USB_IO_REQ*);
s32 usbApiRecv(USB_IO_REQ*);
s32 usbApiRecvWait(USB_IO_REQ*);
s32 usbApiEpCancel(u8);
s32 usbApiEpAbort(u8);
s32 usbApiEpReset(u8);
s32 usbApiEpStall(u8, u8);
s32 usbApiRemoteWakeup(void);
s32 usbApiQueryStatus(u8, u16*);
s32 usbApiSetCallback(u8, void (*cbFunc)(u32));

extern s32 usbInit(void);

extern s32 usbSetEpReq(u8, USB_EP_REQ*);
extern s32 usbSetEpReqAbort(u8);

extern s32 usbSetApiEvt(USB_API_EVT_SNAP*);

extern OS_EVENT* usbSemEp[];
extern OS_EVENT* usbSemApi[];
extern OS_EVENT* usbSemApiNotify[];

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
#define usbApiDebugPrint 			printf

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

extern OS_EVENT* usbSemApi[];
extern OS_EVENT* usbSemApiNotify[];

extern USB_DEV_SETTING usbDevSetting;

extern void (*usbCbFunc[])(u32);

extern USB_EP_REQ_RET usbEpReqRet[];
extern u8 usbScsiRead10;
extern u8 usbScsiWrite10;
extern OS_EVENT* usbSemPrevSend;
/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Initialize.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiInit(void)
{
    return usbInit();
}

/*

Routine Description:

	Terminate.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiTerminate(void)
{
    USB_API_EVT_SNAP evt;
    u8 err;
    s32 stat = 1;

    /* acquire semaphore */
    OSSemPend(usbSemApi[USB_SEM_API_TERMINATE], USB_API_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApi[USB_SEM_API_TERMINATE] is %d.\n", err);
        return 0;
    }

    /* set event */
    evt.cause  = USB_API_EVT_TERMINATE;
    evt.epNum  = 0;
    evt.option |= USB_API_OPT_NOTIFY;
    usbSetApiEvt(&evt);

    /* Delete the task */
    //usbApiDebugPrint("Trace: USB tasks deleting.\n");
    OSTaskDel(USB_MSC_TASK_PRIORITY);

    /* acquire semaphore */
    OSSemPend(usbSemApiNotify[USB_SEM_API_NOTIFY_TERMINATE], USB_API_NOTIFY_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApiNotify[USB_SEM_API_NOTIFY_TERMINATE] is %d.\n", err);
        stat = 0;
    }

    /* release semaphore */
    OSSemPost(usbSemApi[USB_SEM_API_TERMINATE]);

    return stat;
}

/*

Routine Description:

	Send.

Arguments:

	pIoReq - IO request.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiSend(USB_IO_REQ* pIoReq)
{
    u8 epNum;
    USB_EP_REQ epReq; /* endpoint request */
    USB_API_EVT_SNAP evt;
    u8 err;

    /* check valid data pointer */
    if (pIoReq->pData == NULL)
        return 0;


    /* used for pipeline, to prevent bulk-in transfer racing */
    if (usbScsiRead10 == 1)
    {
        OSSemPend(usbSemPrevSend, USB_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            DEBUG_USB("Error: usbSemPrevSend is %d.\n", err);
            return 0;
        }

    }


    /* acquire semaphore */
    OSSemPend(usbSemApi[USB_SEM_API_SEND], USB_API_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApi[USB_SEM_API_SEND] is %d.\n", err);
        return 0;
    }

    /* TBD: check for correct connection and bus state */

    /* get endpoint number */
    epNum = pIoReq->epNum;

    /*---- set endpoint request ----*/
    /* set endpoint request */
    memset((void *)&epReq, 0, sizeof(USB_EP_REQ));
    epReq.pData = pIoReq->pData;
    epReq.reqSize = pIoReq->reqSize;
    epReq.reqOption = pIoReq->reqOption;
    /* set endpoint request return */
    usbEpReqRet[epNum].pRetSize = pIoReq->pRetSize;
    usbEpReqRet[epNum].pRetStat = pIoReq->pRetStat;
    /* send the request */
    usbSetEpReq(epNum, &epReq);

    /*---- set api event ----*/
    evt.cause  = USB_API_EVT_SEND;
    evt.epNum  = epNum;
    evt.option = 0; /* not notify */
    usbSetApiEvt(&evt);

    /* release semaphore */
    OSSemPost(usbSemApi[USB_SEM_API_SEND]);

    return 1;
}

/*

Routine Description:

	Send and wait till completion.

Arguments:

	pIoReq - IO request.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiSendWait(USB_IO_REQ* pIoReq)
{
    u8 err;

    /* check valid data pointer */
    if (pIoReq->pData == NULL)
        return 0;

    /* notify after completion */
    pIoReq->reqOption |= USB_EP_REQ_OPT_NOTIFY;

    /* send */
    if (usbApiSend(pIoReq) == 0)
    {
        DEBUG_USB("Error: [usbApiSendWait] API send error.\n");
        return 0;
    }

    if (usbScsiRead10 == 0)
    {
        /* wait for notify */
        OSSemPend(usbSemApiNotify[USB_SEM_API_NOTIFY_SEND], USB_API_NOTIFY_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            DEBUG_USB("Error: usbSemApiNotify[USB_SEM_API_NOTIFY_SEND] is %d.\n", err);
            return 0;
        }

    }

    return 1;
}

/*

Routine Description:

	Receive.

Arguments:

	pIoReq - IO request.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiRecv(USB_IO_REQ* pIoReq)
{
    u8 epNum;
    USB_EP_REQ epReq; /* endpoint request */
    USB_API_EVT_SNAP evt;
    u8 err;

    /* check valid data pointer */
    if (pIoReq->pData == NULL)
    {
        DEBUG_USB("Error: Data pointer of IO request is invalid.\n");
        return 0;
    }

    /* acquire semaphore */
    OSSemPend(usbSemApi[USB_SEM_API_RECV], USB_API_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApi[USB_SEM_API_RECV] is %d.\n", err);
        return 0;
    }

    /* TBD: check for correct connection and bus state */

    /* get endpoint number */
    epNum = pIoReq->epNum;

    /*---- set endpoint request ----*/
    /* set endpoint request */
    memset((void *)&epReq, 0, sizeof(USB_EP_REQ));
    epReq.pData = pIoReq->pData;
    epReq.reqSize = pIoReq->reqSize;
    epReq.reqOption = pIoReq->reqOption | USB_EP_REQ_OPT_FIRST_RCV; /* first time receive */
    /* set endpoint request return */
    usbEpReqRet[epNum].pRetSize = pIoReq->pRetSize;
    usbEpReqRet[epNum].pRetStat = pIoReq->pRetStat;
    /* send the request */
    usbSetEpReq(epNum, &epReq);

    /*---- set api event ----*/
    evt.cause  = USB_API_EVT_RECV;
    evt.epNum  = epNum;
    evt.option = 0; /* not notify */
    usbSetApiEvt(&evt);

    /* release semaphore */
    OSSemPost(usbSemApi[USB_SEM_API_RECV]);

    return 1;
}

/*

Routine Description:

	Receive and wait till completion.

Arguments:

	pIoReq - IO request.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiRecvWait(USB_IO_REQ* pIoReq)
{
    u8 err;

    /* check valid data pointer */
    if (pIoReq->pData == NULL)
    {
        DEBUG_USB("Error: Data pointer of IO request is invalid.\n");
        return 0;
    }

    /* notify after completion */
    pIoReq->reqOption |= USB_EP_REQ_OPT_NOTIFY;

    /* receive */
    if (usbApiRecv(pIoReq) == 0)
    {
        DEBUG_USB("Error: API receive error.\n");
        return 0;
    }

    /* wait for notify */
    OSSemPend(usbSemApiNotify[USB_SEM_API_NOTIFY_RECV], OS_IPC_WAIT_FOREVER, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApiNotify[USB_SEM_API_NOTIFY_RECV] is %d.\n", err);
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Cancel.

Arguments:

	epNum - Endpoint number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiEpCancel(u8 epNum)
{
    USB_API_EVT_SNAP evt;
    u8 err;
    s32 stat = 1;

    /* acquire semaphore */
    OSSemPend(usbSemApi[USB_SEM_API_EP_CANCEL], USB_API_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApi[USB_SEM_API_EP_CANCEL] is %d.\n", err);
        return 0;
    }

    /* set event */
    evt.cause  = USB_API_EVT_EP_CANCEL;
    evt.epNum  = epNum;
    evt.option |= USB_API_OPT_NOTIFY;
    usbSetApiEvt(&evt);

    /* acquire semaphore */
    OSSemPend(usbSemApiNotify[USB_SEM_API_NOTIFY_EP_CANCEL], USB_API_NOTIFY_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApiNotify[USB_SEM_API_NOTIFY_EP_CANCEL] is %d.\n", err);
        stat = 0;
    }

    /* release semaphore */
    OSSemPost(usbSemApi[USB_SEM_API_EP_CANCEL]);

    return stat;
}

/*

Routine Description:

	Endpoint abort.

Arguments:

	epNum - Endpoint number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiEpAbort(u8 epNum)
{
    usbSetEpReqAbort(epNum);

    return 1;
}

/*

Routine Description:

	 Endpoint reset.

Arguments:

	epNum - Endpoint number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiEpReset(u8 epNum)
{
    USB_API_EVT_SNAP evt;
    u8 err;
    s32 stat = 1;

    /* acquire semaphore */
    OSSemPend(usbSemApi[USB_SEM_API_EP_RESET], USB_API_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApi[USB_SEM_API_EP_RESET] is %d.\n", err);
        return 0;
    }

    /* set event */
    evt.cause  = USB_API_EVT_EP_RESET;
    evt.epNum  = epNum;
    evt.option |= USB_API_OPT_NOTIFY;
    usbSetApiEvt(&evt);


    /* acquire semaphore */
    OSSemPend(usbSemApiNotify[USB_SEM_API_NOTIFY_EP_RESET], USB_API_NOTIFY_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApiNotify[USB_SEM_API_NOTIFY_EP_RESET] is %d.\n", err);
        stat = 0;
    }

    /* release semaphore */
    OSSemPost(usbSemApi[USB_SEM_API_EP_RESET]);

    return stat;
}

/*

Routine Description:

	Endpoint stall.

Arguments:

	epNum - Endpoint number.
	stall - Stall or clear stall.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiEpStall(u8 epNum, u8 stall)
{
    USB_API_EVT_SNAP evt;
    u8 err;
    s32 stat = 1;

    /* acquire semaphore */
    OSSemPend(usbSemApi[USB_SEM_API_EP_STALL], USB_API_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApi[USB_SEM_API_EP_STALL] is %d.\n", err);
        return 0;
    }

    if (stall)
    {
        /* set event */
        evt.cause  = USB_API_EVT_EP_STALL;
        evt.epNum  = epNum;
        evt.option |= USB_API_OPT_NOTIFY;
        usbSetApiEvt(&evt);

        /* acquire semaphore */
        OSSemPend(usbSemApiNotify[USB_SEM_API_NOTIFY_EP_STALL], USB_API_NOTIFY_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            DEBUG_USB("Error: usbSemApiNotify[USB_SEM_API_NOTIFY_EP_STALL]] is %d.\n", err);
            stat = 0;
        }
    }
    else
    {
        /* set event */
        evt.cause  = USB_API_EVT_EP_CLEAR_STALL;
        evt.epNum  = epNum;
        evt.option |= USB_API_OPT_NOTIFY;
        usbSetApiEvt(&evt);

        /* acquire semaphore */
        OSSemPend(usbSemApiNotify[USB_SEM_API_NOTIFY_EP_CLEAR_STALL], USB_API_NOTIFY_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            DEBUG_USB("Error: usbSemApiNotify[USB_SEM_API_NOTIFY_EP_CLEAR_STALL]] is %d.\n", err);
            stat = 0;
        }
    }

    /* release semaphore */
    OSSemPost(usbSemApi[USB_SEM_API_EP_STALL]);

    return stat;
}

/*

Routine Description:

	Remote wakeup.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiRemoteWakeup(void)
{
    USB_API_EVT_SNAP evt;
    u8 err;
    s32 stat = 1;

    /* acquire semaphore */
    OSSemPend(usbSemApi[USB_SEM_API_REMOTE_WAKEUP], USB_API_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApi[USB_SEM_API_REMOTE_WAKEUP] is %d.\n", err);
        return 0;
    }

    /* set event */
    evt.cause  = USB_API_EVT_REMOTE_WAKEUP;
    evt.epNum  = 0;
    evt.option |= USB_API_OPT_NOTIFY;
    usbSetApiEvt(&evt);


    /* acquire semaphore */
    OSSemPend(usbSemApiNotify[USB_SEM_API_NOTIFY_REMOTE_WAKEUP], USB_API_NOTIFY_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApiNotify[USB_SEM_API_NOTIFY_REMOTE_WAKEUP] is %d.\n", err);
        stat = 0;
    }

    /* release semaphore */
    OSSemPost(usbSemApi[USB_SEM_API_REMOTE_WAKEUP]);

    return stat;
}

/*

Routine Description:

	Query status.

Arguments:

	epNum - Endpoint number.
	pStat - Endpoint status.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiQueryStatus(u8 epNum, u16* pStat)
{
    USB_API_EVT_SNAP evt;
    u8 err;
    s32 stat = 1;

    /* acquire semaphore */
    OSSemPend(usbSemApi[USB_SEM_API_QUERY_STATUS], USB_API_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApi[USB_SEM_API_QUERY_STATUS] is %d.\n", err);
        stat = 0;
    }

    /* set event */
    evt.cause  = USB_API_EVT_QUERY_STATUS;
    evt.epNum  = 0;
    evt.option |= USB_API_OPT_NOTIFY;
    usbSetApiEvt(&evt);

    /* acquire semaphore */
    OSSemPend(usbSemApiNotify[USB_SEM_API_NOTIFY_QUERY_STATUS], USB_API_NOTIFY_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApiNotify[USB_SEM_API_NOTIFY_QUERY_STATUS] is %d.\n", err);
        stat = 0;
    }
    else
    {
        *pStat = usbDevSetting.epStat[epNum];
    }

    /* release semaphore */
    OSSemPost(usbSemApi[USB_SEM_API_QUERY_STATUS]);

    return stat;
}

/*

Routine Description:

	Set callback function.

Arguments:

	cbIdx - Callback function index.
	cbFunc - Callcack function pointer.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbApiSetCallback(u8 cbIdx, void (*cbFunc)(u32))
{
    /* check valid of callback function index */
    if (cbIdx >= USB_CB_MAX)
        return 0;

    /* check valid of callback function pointer */
    if (cbFunc == NULL)
        return 0;

    /* register callback function */
    usbCbFunc[cbIdx] = cbFunc;

    return 1;
}

