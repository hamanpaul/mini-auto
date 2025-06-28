/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbint.c

Abstract:

   	USB interrupt event handler.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"

#include "task.h"
#include "board.h"
#include "usb.h"
#include "usbdesc.h"
#include "usbreg.h"
#include "usbdev.h"
#include "usbcb.h"
#include "usbintevt.h"
#include "usbapi.h"

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

void usbIntEvtAttach(void);
void usbIntEvtDetach(void);
void usbIntEvtCtrlIn(void);
void usbIntEvtCtrlOut(void);
void usbIntEvtSetup(void);
void usbIntEvtSetupOverw(void);
void usbIntEvtBulkIn(void);
void usbIntEvtBulkOut(void);
void usbIntEvtReset(void);
void usbIntEvtSuspend(void);
void usbIntEvtResume(void);
void usbIntEvtSof(void);
void usbIntEvtCtrlInNak(void);
void usbIntEvtCtrlOutNak(void);
void usbIntEvtBulkInNak(void);
void usbIntEvtBulkOutNak(void);
void usbIntEvtCtrlInStall(void);
void usbIntEvtCtrlOutStall(void);
void usbIntEvtIntrIn(void);
void usbIntEvtIntrInNak(void);
void usbIntEvtIsoIn(void);
void usbIntEvtIsoInZeroL(void);
void usbIntEvtBulkInDmaCmpl(void);
void usbIntEvtBulkOutDmaCmpl(void);
void usbIntEvtIsoInDmaCmpl(void);

s32 usbDecDevReq(USB_DEV_REQ*);

void usbGetStatus(USB_DEV_REQ*);
void usbClearFeature(USB_DEV_REQ*);
void usbSetFeature(USB_DEV_REQ*);
void usbSetAddress(USB_DEV_REQ*);
void usbGetDescriptor(USB_DEV_REQ*);
void usbSetDescriptor(USB_DEV_REQ*);
void usbGetConfiguration(USB_DEV_REQ*);
void usbSetConfiguration(USB_DEV_REQ*);
void usbGetInterface(USB_DEV_REQ*);
void usbSetInterface(USB_DEV_REQ*);
void usbSynchFrame(USB_DEV_REQ*);
void usbClassReq(USB_DEV_REQ*);
void usbVendorReq(USB_DEV_REQ*);

s32 usbGetEpReq(u8, USB_EP_REQ*);
s32 usbSetEpReq(u8, USB_EP_REQ*);
s32 usbSetEpReqAbort(u8);

s32 usbEpNotify(u8, u8);

s32 usbInitCtrlStatusStage(u8);

s32 usbSetCtrlIn(void);
s32 usbSetCtrlOut(void);
s32 usbSetBulkIn(void);
s32 usbSetBulkOut(void);
s32 usbSetIntrIn(void);
s32 usbSetIsoIn(void);


/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
#define usbIntEvtDebugPrint 				printf

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

u8 ucSetAddressReq = FALSE;	/* init is not set address */
u8	USB_IN = 0;	/* a flag to indicate if USB is working or not */
u8	gucUsbPlugInStat = USB_PLUGIN_STAT_NONE;	/* a flag to indicate the usb plug-in status. */


/* Interrupt event function */
void (*usbIntEvtFunc[])(void) =
    {
        usbIntEvtAttach,		/* 0x00 - attach event */
        usbIntEvtDetach,		/* 0x01 - detach event */
        usbIntEvtCtrlIn,		/* 0x02	- control in data packet*/
        usbIntEvtCtrlOut,		/* 0x03	- control out data packet */
        usbIntEvtSetup,			/* 0x04	- setup packet */
        usbIntEvtSetupOverw,		/* 0x05	- setup packet overwrite */
        usbIntEvtBulkIn,		/* 0x06 - bulk in data packet */
        usbIntEvtBulkOut,		/* 0x07	- bulk out data packet */
        usbIntEvtReset,			/* 0x08	- device reset event */
        usbIntEvtSuspend,		/* 0x09	- device suspend event */
        usbIntEvtResume,		/* 0x0a	- device resume event */
        usbIntEvtSof,			/* 0x0b	- start of frame packet */
        usbIntEvtCtrlInNak,		/* 0x0c	- control in nak packet */
        usbIntEvtCtrlOutNak,		/* 0x0d	- control out nak packet */
        usbIntEvtBulkInNak,		/* 0x0e	- bulk in nak packet */
        usbIntEvtBulkOutNak,		/* 0x0f	- bulk out nak packet */
        usbIntEvtCtrlInStall,		/* 0x10	- control in stall event */
        usbIntEvtCtrlOutStall,		/* 0x11	- control out stall event */
        usbIntEvtIntrIn,		/* 0x12	- interrupt in data packet */
        usbIntEvtIntrInNak,		/* 0x13 - interrupt in nak packet */
        usbIntEvtIsoIn,			/* 0x14 - isochronous in data packet */
        usbIntEvtIsoInZeroL,		/* 0x15 - isochronous in zero-length data packet */
        usbIntEvtBulkInDmaCmpl,		/* 0x16 - bulk in dma completion event */
        usbIntEvtBulkOutDmaCmpl,	/* 0x17 - bulk out dma completion event */
        usbIntEvtIsoInDmaCmpl,		/* 0x18 - isochronous in dma completion event */
    };

__align(4) USB_DEV_REQ usbDevReq;
__align(4) USB_DEV_SETTING usbDevSetting;
u8 usbCtrlOutStatusStage = 0;
u8 usbCtrlInStatusStage = 0;

extern void (*usbCbFunc[])(u32);

extern u32 usbEpMaxPktSize[];
extern u32 usbEpMaxDmaSize[];

extern OS_EVENT* usbSemEp[];

extern USB_EP_REQ usbEpReq[];
extern USB_EP_REQ_RET usbEpReqRet[];

extern void (*usbCbFunc[])(u32);

extern s32 usbManageInit(void);

extern usbScsiRead10;

extern OS_EVENT* usbSemPrevSend;

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Attach.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbIntEvtAttach(void)
{
    //DEBUG_USB("Trace: usbInEvtAttach()\n");

    /* initialize management structure */
    usbManageInit();

    /* callback function */
    if (usbCbFunc[USB_CB_ATTACH] != NULL)
        (*usbCbFunc[USB_CB_ATTACH])((u32)0);
}

/*

Routine Description:

	Detach.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbIntEvtDetach(void)
{
    //DEBUG_USB("Trace: usbIntEvtDetach()\n");

    /* callback function */
    if (usbCbFunc[USB_CB_DETACH] != NULL)
        (*usbCbFunc[USB_CB_DETACH])((u32)0);
}

/*

Routine Description:

	Control in packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtCtrlIn(void)
{
    //DEBUG_USB("Trace: usbIntCtrlIn()\n");

    /* TBD: check if previous dma is ok */

    /* check if control in status stage */
    if (usbCtrlInStatusStage)
    {
        //DEBUG_USB("Trace: Control in status stage.\n");
        usbCtrlInStatusStage = 0;

        /* Reset enpoint 0 */
        usbEpReset(USB_EP_CTRLIN);

        return;
    }

    /* set next transfer */
    usbSetCtrlIn();

	/* a flag to indicate if USB is working or not */
//	USB_IN = 1;
	usbSetUsbPluginStat(USB_SITUATION_PC);
}

/*

Routine Description:

	Control out packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtCtrlOut(void)
{
    //DEBUG_USB("Trace: usbIntEvtCtrlOut()\n");

    /* TBD: check if previous dma is ok */

    /* check if control out status stage */
    if (usbCtrlOutStatusStage)
    {
        //DEBUG_USB("Trace: Control out status stage.\n");
        usbCtrlOutStatusStage = 0;

        /* Reset enpoint 0 */
        usbEpReset(USB_EP_CTRLIN);

        return;
    }

    /* set next transfer */
    usbSetCtrlOut();
}

/*

Routine Description:

	Setup packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtSetup(void)
{
    u32* pDevReq = (u32*)&usbDevReq;

    //DEBUG_USB("Trace: usbIntEvtSetup()\n");

    usbEpClearStall(USB_EP_CTRLIN);
    usbEpClearStall(USB_EP_CTRLOUT);
    usbEpReset(USB_EP_CTRLIN);

    usbGetDevReq(pDevReq);

    usbDecDevReq(&usbDevReq);
}

/*

Routine Description:

	Setup overwrite packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtSetupOverw(void)
{
    //DEBUG_USB("Trace: usbIntEvtSetupOverw()\n");

    /* Setup packet is over-written */

    return;
}

/*

Routine Description:

	Bulk in packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtBulkIn(void)
{
    //DEBUG_USB("Trace: usbIntEvtBulkIn()\n");

    /* Reset endpoint bulk in */
    usbEpReset(USB_EP_BULKIN);

    /* set next transfer */
    usbSetBulkIn();
}

/*

Routine Description:

	Bulk out packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtBulkOut(void)
{
    //DEBUG_USB("Trace: usbEvtIntBulkOut()\n");

    /* Reset endpoint bulk out */
    usbEpReset(USB_EP_BULKOUT);

    /* set next transfer */
    usbSetBulkOut();
}

/*

Routine Description:

	Reset.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtReset(void)
{
    //DEBUG_USB("Trace: usbIntEvtReset()\n");

    /* callback function */
    if (usbCbFunc[USB_CB_BUS_RESET] != NULL)
        (*usbCbFunc[USB_CB_BUS_RESET])((u32)0);
}

/*

Routine Description:

	Suspend.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtSuspend(void)
{
    //DEBUG_USB("Trace: usbIntEvtSuspend()\n");

    /* hardware suspend */
    usbDevSuspend();

    /* callback function */
    if (usbCbFunc[USB_CB_BUS_SUSPEND] != NULL)
        (*usbCbFunc[USB_CB_BUS_SUSPEND])((u32)0);
}

/*

Routine Description:

	Resume.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtResume(void)
{
    //DEBUG_USB("Trace: usbDebugEvtResume()\n");

    /* hardware resume */
    usbDevResume();

    /* callback function */
    if (usbCbFunc[USB_CB_BUS_RESUME] != NULL)
        (*usbCbFunc[USB_CB_BUS_RESUME])((u32)0);
}

/*

Routine Description:

	Start of frame packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtSof(void)
{
    //DEBUG_USB("Trace: usbIntEvtSof()\n");

    return;
}

/*

Routine Description:

	Control in nak packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtCtrlInNak(void)
{
    //DEBUG_USB("Trace: usbIntEvtCtrlInNak()\n");

    return;
}

/*

Routine Description:

	Control out nak packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtCtrlOutNak(void)
{
    //DEBUG_USB("Trace: usbIntEvtCtrlOutNak()\n");

    return;
}

/*

Routine Description:

	Bulk in nak packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtBulkInNak(void)
{
    //DEBUG_USB("Trace: usbIntEvtBulkInNak()\n");

    return;
}

/*

Routine Description:

	Bulk out nak packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtBulkOutNak(void)
{
    //DEBUG_USB("Trace: usbIntEvtBulkOutNak()\n");

    return;
}

/*

Routine Description:

	Control in stall.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtCtrlInStall(void)
{
    //DEBUG_USB("Trace: usbIntEvtCtrlInStall()\n");

    return;
}

/*

Routine Description:

	Control out stall.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtCtrlOutStall(void)
{
    //DEBUG_USB("Trace: usbIntEvtCtrlOutStall()\n");

    return;
}

/*

Routine Description:

	Interrupt in packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtIntrIn(void)
{
    //DEBUG_USB("Trace: usbIntEvtIntrIn()\n");

    /* TBD: check if previous dma is ok */

    /* set next transfer */
    usbSetIntrIn();
}

/*

Routine Description:

	Interrupt In nak packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtIntrInNak(void)
{
    //DEBUG_USB("Trace: usbIntEvtIntrInNak()\n");

    return;
}

/*

Routine Description:

	Isochronous in packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtIsoIn(void)
{
    //DEBUG_USB("Trace: usbIntEvtIsoIn()\n");

    /* TBD: check if previous dma is ok */

    /* set next transfer */
    usbSetIsoIn();
}

/*

Routine Description:

	Isochronous in zero-length packet.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtIsoInZeroL(void)
{
    //DEBUG_USB("Trace: usbIntEvtIsoInZeroL()\n");

    return;
}

/*

Routine Description:

	Bulk in dma completion.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtBulkInDmaCmpl(void)
{
    //DEBUG_USB("Trace: usbIntEvtBulkInDmaCmpl()\n");

    /* TBD: check if previous dma is ok */

    usbIntEvtBulkIn();

    return;
}

/*

Routine Description:

	Bulk out dma completion.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtBulkOutDmaCmpl(void)
{
    //DEBUG_USB("Trace: usbIntEvtBulkOutDmaCmpl()\n");

    /* TBD: check if previous dma is ok */

    return;
}

/*

Routine Description:

	Isochronous in dma completion.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntEvtIsoInDmaCmpl(void)
{
    //DEBUG_USB("Trace: usbIntEvtIsoInDmaCmpl()\n");

    /* TBD: check if previous dma is ok */

    return;
}

/*

Routine Description:

	Decode device request.

Arguments:

	pDevReq - The device request.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbDecDevReq(USB_DEV_REQ* pDevReq)
{
    u8 bmRequestType;
    u8 bRequest;
    u8 type;
    u8 stallFlag = FALSE;
    u8 callbackIndex = 0;
    u32 ret = 1;

    //DEBUG_USB("Trace: Setup packet:\n");
    //DEBUG_USB("\tbmRequestType = 0x%02x\n", pDevReq->bmRequestType);
    //DEBUG_USB("\tbRequest      = 0x%02x\n", pDevReq->bRequest);
    //DEBUG_USB("\twValue        = 0x%04x\n", pDevReq->wValue);
    //DEBUG_USB("\twIndex        = 0x%04x\n", pDevReq->wIndex);
    //DEBUG_USB("\twLength       = 0x%04x\n", pDevReq->wLength);

    /* get device request */
    bmRequestType = pDevReq->bmRequestType;
    bRequest      = pDevReq->bRequest;

    /* get request type */
    type = bmRequestType & USB_DEV_REQ_TYP_MASK;
    switch (type)
    {
    case USB_DEV_REQ_TYP_STANDARD:
        //DEBUG_USB("Trace: Standard request:\n");
        /* standard request */
        switch (bRequest)
        {
        case USB_RC_GET_STATUS:		/* GET_STATUS */
            //DEBUG_USB("\tGet status\n");
            usbGetStatus(pDevReq);
            callbackIndex = USB_CB_GET_STATUS;
            break;

        case USB_RC_CLEAR_FEATURE:	/* CLEAR_FEATURE */
            //DEBUG_USB("\tClear feature\n");
            usbClearFeature(pDevReq);
            callbackIndex = USB_CB_CLEAR_FEATURE;
            break;

        case USB_RC_SET_FEATURE:	/* SET_FEATURE */
            //DEBUG_USB("\tSet feature\n");
            usbSetFeature(pDevReq);
            callbackIndex = USB_CB_SET_FEATURE;
            break;

        case USB_RC_SET_ADDRESS:	/* SET_ADDERSS */
            //DEBUG_USB("\tSet address\n");
            usbSetAddress(pDevReq);
            callbackIndex = USB_CB_SET_ADDRESS;
            break;

        case USB_RC_GET_DESCRIPTOR:	/* GET_DESCRIPTOR */
            //DEBUG_USB("\tGet descriptor\n");
            usbGetDescriptor(pDevReq);
            callbackIndex = USB_CB_GET_DESCRIPTOR;
            break;

        case USB_RC_SET_DESCRIPTOR:	/* SET_DESCRIPTOR */
            //DEBUG_USB("\tSet descriptor\n");
            usbSetDescriptor(pDevReq);
            callbackIndex = USB_CB_SET_DESCRIPTOR;
            break;

        case USB_RC_GET_CONFIGURATION:	/* GET_CONFIGURATION */
            //DEBUG_USB("\tGet configuration\n");
            usbGetConfiguration(pDevReq);
            callbackIndex = USB_CB_GET_CONFIGURATION;
            break;

        case USB_RC_SET_CONFIGURATION:	/* SET_CONFIGURATION */
            //DEBUG_USB("\tSet configuration\n");
            usbSetConfiguration(pDevReq);
            callbackIndex = USB_CB_SET_CONFIGURATION;
            break;

        case USB_RC_GET_INTERFACE:	/* GET_INTERFACE */
            //DEBUG_USB("\tGet interface\n");
            usbGetInterface(pDevReq);
            callbackIndex = USB_CB_GET_INTERFACE;
            break;

        case USB_RC_SET_INTERFACE:	/* SET_INTERFACE */
            //DEBUG_USB("\tSet interface\n");
            usbSetInterface(pDevReq);
            callbackIndex = USB_CB_SET_INTERFACE;
            break;

        case USB_RC_SYNCH_FRAME:	/* SYNCH_FRAME */
            //DEBUG_USB("\tSynch frame\n");
            usbSynchFrame(pDevReq);
            callbackIndex = USB_CB_SYNCH_FRAME;
            break;

        default: 			/* Illegal request code */
            //DEBUG_USB("\tIllegal request code\n");
            ret = 0;
            stallFlag = TRUE;
            break;
        }
        if (!stallFlag)
        {
            /* callback function */
            if (usbCbFunc[callbackIndex] != NULL)
                (*usbCbFunc[callbackIndex])((u32)pDevReq);
        }
        break;

    case USB_DEV_REQ_TYP_CLASS:
        //DEBUG_USB("Trace: Class request\n");
        usbClassReq(pDevReq);
        break;

    case USB_DEV_REQ_TYP_VENDOR:		/* vendor request*/
        //DEBUG_USB("Trace: Vendor request\n");
        usbVendorReq(pDevReq);
        break;

    default:
        //DEBUG_USB("Trace: Unknown request\n");
        ret = 0;
        stallFlag = TRUE;
        break;
    }

    if (stallFlag)
    {	/* stall conrol in / out */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }

    return ret;
}

/*

Routine Description:

	GET_STATUS.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbGetStatus(USB_DEV_REQ* pDevReq)
{
    u16* pStat;
    u8 epNum = 0; /* endpoint number */
    u8 valid = 1; /* valid request or not */
    USB_EP_REQ epReq;	/* endpoint request */

    switch (pDevReq->bmRequestType)
    {
    case (USB_DEV_REQ_DIR_D2H | USB_DEV_REQ_REC_DEV):
                    pStat = &(usbDevSetting.devStat);
        break;

    case (USB_DEV_REQ_DIR_D2H | USB_DEV_REQ_REC_IF):
                    if (pDevReq->wIndex >= usbDevSetting.numIf) /* illegal interface */
                        valid = 0;
            else
                pStat = &(usbDevSetting.ifStat);
        break;

    case (USB_DEV_REQ_DIR_D2H | USB_DEV_REQ_REC_EP):
                    epNum = pDevReq->wIndex & 0x0f;
        if (epNum > USB_ENDPOINT_MAX)
            valid = 0;
        else
            pStat = &(usbDevSetting.epStat[epNum + 1]);
        break;

    default: /* illegal bmRequestType */
        valid = 0;
        break;
    }

    if (!valid)
{	/* invalid request */
        /* TBD: stop DMA and clear FIFO */
        /* stall conrol in / out */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }
    else
    {
        /* write status data */
        usbEpWriteData(USB_EP_CTRLIN, (u8*)pStat, 2);

        /* set endpoint request */
        memset((void *)&epReq, 0, sizeof(USB_EP_REQ));
        epReq.pData = NULL;
        epReq.reqSize = 0;
        epReq.reqOption = 0;
        usbSetEpReq(USB_EP_CTRLIN, &epReq);

        /* initialize dma of control out of status stage for control in data stage */
        usbInitCtrlStatusStage(USB_EP_CTRLOUT);
    }
}

/*

Routine Description:

	CLEAR_FEATURE.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbClearFeature(USB_DEV_REQ* pDevReq)
{
    u8 epNum = 0; /* endpoint number */
    u8 valid = 1; /* valid request or not */

    switch (pDevReq->bmRequestType)
    {
    case (USB_DEV_REQ_DIR_H2D | USB_DEV_REQ_REC_DEV):
            if (pDevReq->wValue == USB_FS_DEVICE_REMOTE_WAKEUP)
                usbDevSetting.devStat &= ~USB_DEV_STAT_REMOTE_WAKEUP;
            else
                valid = 0;
        break;

    case (USB_DEV_REQ_DIR_H2D | USB_DEV_REQ_REC_IF):
            if (pDevReq->wIndex >= usbDevSetting.numIf) /* illegal interface */
                valid = 0;
        break;

    case (USB_DEV_REQ_DIR_H2D | USB_DEV_REQ_REC_EP):
            if (pDevReq->wValue == USB_FS_ENDPOINT_HALT)
            {
                epNum = pDevReq->wIndex & 0x0f;
                if (epNum > USB_ENDPOINT_MAX)
                    valid = 0;
                else
                {
                    usbEpClearStall(epNum + 1);
                    if (epNum == 0)
                        usbEpClearStall(0);
                }
            }
            else
                valid = 0;
        break;

    default: /* illegal bmRequestType */
        valid = 0;
        break;
    }

    if (!valid)
    {	/* invalid request */
        /* TBD: stop DMA and clear FIFO */
        /* stall conrol in / out */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }
    else
    {
        /* initialize dma of control in status stage */
        usbInitCtrlStatusStage(USB_EP_CTRLIN);
    }
}

/*

Routine Description:

	SET_FEATURE.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbSetFeature(USB_DEV_REQ* pDevReq)
{
    u8 epNum = 0; /* endpoint number */
    u8 valid = 1; /* valid request or not */

    switch (pDevReq->bmRequestType)
    {
    case (USB_DEV_REQ_DIR_H2D | USB_DEV_REQ_REC_DEV):
                    if (pDevReq->wValue == USB_FS_DEVICE_REMOTE_WAKEUP)
                        usbDevSetting.devStat |= USB_DEV_STAT_REMOTE_WAKEUP;
            else
                valid = 0;
        break;

    case (USB_DEV_REQ_DIR_H2D | USB_DEV_REQ_REC_IF):
                    if (pDevReq->wIndex >= usbDevSetting.numIf) /* illegal interface */
                        valid = 0;
        break;

    case (USB_DEV_REQ_DIR_H2D | USB_DEV_REQ_REC_EP):
                    if (pDevReq->wValue == USB_FS_ENDPOINT_HALT)
            {
                epNum = pDevReq->wIndex & 0x0f;
                if (epNum > USB_ENDPOINT_MAX)
                    valid = 0;
                else
                {
                    usbEpStall(epNum + 1);
                    if (epNum == 0)
                        usbEpStall(0);
                }
            }
            else
                valid = 0;
        break;

    default: /* illegal bmRequestType */
        valid = 0;
        break;
    }

    if (!valid)
    {	/* invalid request */
        /* TBD: stop DMA and clear FIFO */
        /* stall conrol in / out */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }
    else
    {
        /* initialize dma of control in status stage */
        usbInitCtrlStatusStage(USB_EP_CTRLIN);
    }
}

/*

Routine Description:

	SET_ADDRESS.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbSetAddress(USB_DEV_REQ* pDevReq)
{
    /* save device address */
    usbDevSetting.devAddr = pDevReq->wValue;

    ucSetAddressReq = TRUE;		/* init is not set address */

    /* initialize dma of control in status stage */
    usbInitCtrlStatusStage(USB_EP_CTRLIN);
}

/*

Routine Description:

	GET_DESCRIPTOR.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbGetDescriptor(USB_DEV_REQ* pDevReq)
{
    u16 descType = 0;	/* descriptor type */
    u16 descIndex = 0;	/* descriptor index */
    u32 descAddr = 0;
    u8* pDescData = NULL; 	/* data pointer */
    u32 descSize = 0;   	/* data size */
    u8  valid = 1;		/* valid request or not */
    u32 dmaSize = 0;	/* dma size once */
    USB_EP_REQ epReq;	/* endpoint request */

    /* descriptor type and index */
    descType  = (pDevReq->wValue & 0xff00) >> 8;
    descIndex =  pDevReq->wValue & 0x00ff;

    switch (descType)
    {
    case USB_DESC_TYPE_DEVICE:
        //DEBUG_USB("\tGet device descriptor\n");
        /* get device descriptor */
        if (usbClassGetDevDesc(&descAddr, &descSize) == 0)
            valid = 0;
        else
            pDescData = (u8*) descAddr;
        break;

    case USB_DESC_TYPE_CONFIGURATION:
        //DEBUG_USB("\tGet configuration descriptor\n");
        if (usbClassGetCfgDesc(descIndex, &descAddr, &descSize) == 0)
            valid = 0;
        else
            pDescData = (u8*) descAddr;
        break;

    case USB_DESC_TYPE_STRING:
        //DEBUG_USB("\tGet string descriptor\n");
        if (usbClassGetStrDesc(descIndex, &descAddr, &descSize) == 0)
            valid = 0;
        else
            pDescData = (u8*) descAddr;
        break;

#if 0	/* for USB revision 2.0 only */

    case USB_DESC_TYPE_DEVICE_QUALIFIER:
        //DEBUG_USB("\tGet device qualifier descriptor\n");
        if (usbClassGetDevQualDesc(&descAddr, &descSize) == 0)
            valid = 0;
        else
            pDescData = (u8*) descAddr;
        break;

    case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
        //DEBUG_USB("\tGet other speed configuration descriptor\n");
        if (usbClassGetOtherSpeedCfgDesc(descIndex, &descAddr, &descSize) == 0)
            valid = 0;
        else
            pDescData = (u8*) descAddr;
        break;

#endif	/* for USB revision 2.0 only */
    default:
        //DEBUG_USB("\tGet unknown descriptor\n");
        valid = 0;
        break;
    }

    if (pDescData == NULL) /* check null descriptor */
        valid = 0;

    if (pDevReq->wLength == 0) /* check data size requested */
        valid = 0;

    if (!valid)
    {	/* invalid request */
        /* TBD: stop DMA and clear FIFO */
        /* stall conrol in / out */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }
    else
    {
        /* check data size requested is larger than descriptor size */
        descSize = pDevReq->wLength < descSize ? pDevReq->wLength : descSize;

        /* write descriptor up to dma size*/
        dmaSize = (usbEpMaxDmaSize[USB_EP_CTRLIN] < descSize) ? usbEpMaxDmaSize[USB_EP_CTRLIN] : descSize;
        usbEpWriteData(USB_EP_CTRLIN, pDescData, dmaSize);

        /* clear endpoint request */
        memset((void *)&epReq, 0, sizeof(USB_EP_REQ));

        /* one extra zero-packet needed when descriptor size is equal to multiple of packer size */
        if ((descSize % usbEpMaxPktSize[USB_EP_CTRLIN]) == 0)
            epReq.reqOption |= USB_EP_REQ_OPT_END_ZERO_PKT;

        /* set endpoint request */
        epReq.pData = pDescData;
        epReq.reqSize = descSize - dmaSize;
        epReq.svcSize = dmaSize;
        usbSetEpReq(USB_EP_CTRLIN, &epReq);

        /* initialize dma of control out of status stage for control in data stage */
        usbInitCtrlStatusStage(USB_EP_CTRLOUT);
    }
}

/*

Routine Description:

	SET_DESCRIPTOR.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbSetDescriptor(USB_DEV_REQ* pDevReq)
{
    /* invalid request */
    /* TBD: stop DMA and clear FIFO */
    /* stall conrol in / out */
    usbEpStall(USB_EP_CTRLIN);
    usbEpStall(USB_EP_CTRLOUT);
}

/*

Routine Description:

	GET_CONFIGURATION.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbGetConfiguration(USB_DEV_REQ* pDevReq)
{
    u8* pCfgNum;
    USB_EP_REQ epReq; /* endpoint request */

    pCfgNum = &(usbDevSetting.cfgNum);

    /* write status data */
    usbEpWriteData(USB_EP_CTRLIN, pCfgNum, 1);

    /* set endpoint request */
    memset((void *)&epReq, 0, sizeof(USB_EP_REQ));
    epReq.pData = NULL;
    epReq.reqSize = 0;
    epReq.reqOption = 0;
    usbSetEpReq(USB_EP_CTRLIN, &epReq);

    /* initialize dma of control out of status stage for control in data stage */
    usbInitCtrlStatusStage(USB_EP_CTRLOUT);
}

/*

Routine Description:

	SET_CONFIGURATION.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbSetConfiguration(USB_DEV_REQ* pDevReq)
{
    u8 i;
    u8 cfgNum;
    u8 valid = 1;

    for (i = 0; i < USB_EP_MAX; i++)
        usbEpClearStall(i); /* clear stall of each endpoint */

    cfgNum = (u8)pDevReq->wValue;
    if (cfgNum > usbDevSetting.numCfg)
        valid = 0; /* illegal configuration value */
    else
        usbDevSetting.cfgNum = cfgNum;

    if (!valid)
    {	/* invalid request */
        /* TBD: stop DMA and clear FIFO */
        /* stall conrol in / out */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }
    else
    {
        /* initialize dma of control in status stage */
        usbInitCtrlStatusStage(USB_EP_CTRLIN);
    }
}

/*

Routine Description:

	GET_INTERFACE.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbGetInterface(USB_DEV_REQ* pDevReq)
{
    u8 ifNum;
    u8* pAltSet;
    u8 valid;
    USB_EP_REQ epReq;	/* endpoint request */

    ifNum = (u8)pDevReq->wIndex;
    pAltSet = &(usbDevSetting.altSet[ifNum]);

    if (ifNum >= usbDevSetting.numIf)
        valid = 0; /* illegal interface number */

    if (!valid)
    {	/* invalid request */
        /* TBD: stop DMA and clear FIFO */
        /* stall conrol in / out */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }
    else
    {
        /* write status data */
        usbEpWriteData(USB_EP_CTRLIN, pAltSet, 1);

        /* set endpoint request */
        memset((void *)&epReq, 0, sizeof(USB_EP_REQ));
        epReq.pData = NULL;
        epReq.reqSize = 0;
        epReq.reqOption = 0;
        usbSetEpReq(USB_EP_CTRLIN, &epReq);

        /* initialize dma of control out of status stage for control in data stage */
        usbInitCtrlStatusStage(USB_EP_CTRLOUT);
    }
}

/*

Routine Description:

	SET_INTERFACE.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbSetInterface(USB_DEV_REQ* pDevReq)
{
    u8 i;
    u8 ifNum;
    u8 altSet;
    u8 valid;

    for (i = 0; i < USB_EP_MAX; i++)
        usbEpClearStall(i);	/* clear stall of each endpoint */

    ifNum = (u8)pDevReq->wIndex;
    altSet = (u8)pDevReq->wValue;

    if (ifNum >= usbDevSetting.numIf)
        valid = 0; /* illegal interface number */
    else
        usbDevSetting.altSet[ifNum] = altSet;

    if (!valid)
    {	/* invalid request */
        /* TBD: stop DMA and clear FIFO */
        /* stall conrol in / out */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }
    else
    {
        /* initialize dma of control in status stage */
        usbInitCtrlStatusStage(USB_EP_CTRLIN);
    }
}

/*

Routine Description:

	SYNCH_FRAME.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbSynchFrame(USB_DEV_REQ* pDevReq)
{
    /* invalid request */
    /* TBD: stop DMA and clear FIFO */
    /* stall conrol in / out */
    usbEpStall(USB_EP_CTRLIN);
    usbEpStall(USB_EP_CTRLOUT);
}

/*

Routine Description:

	Class request.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbClassReq(USB_DEV_REQ* pDevReq)
{
    if (usbCbFunc[USB_CB_CLASS_REQ] == NULL)
    {	/* callback function is not registered */
        /* invalid request */
        /* TBD: stop DMA and clear FIFO */
        /* stall conrol in / out */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }
    else
    {	/* callback function is registed */
        /* call the function*/
        (*usbCbFunc[USB_CB_CLASS_REQ])((u32)pDevReq);
    }
}

/*

Routine Description:

	Vendor request.

Arguments:

	pDevReq - The device request.

Return Value:

	None.

*/
void usbVendorReq(USB_DEV_REQ* pDevReq)
{
    if (usbCbFunc[USB_CB_VENDER_REQ] == NULL)
    {	/* callback function is not registered */
        /* invalid request */
        /* TBD: stop DMA and clear FIFO */
        /* stall conrol in / out */
        usbEpStall(USB_EP_CTRLIN);
        usbEpStall(USB_EP_CTRLOUT);
    }
    else
    {	/* callback function is registed */
        /* call the function*/
        (*usbCbFunc[USB_CB_VENDER_REQ])((u32)pDevReq);
    }
}

/*

Routine Description:

	Get endpoint request.

Arguments:

	epNum - Endpoint number.
	pReq - Endpoint rquest.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbGetEpReq(u8 epNum, USB_EP_REQ* pReq)
{
    u8 err;

    //DEBUG_USB("Trace: Get endpoint %d request.\n", epNum);

    /* check if endpoint number valid */
    if (epNum >= USB_EP_MAX)
    {
        DEBUG_USB("Error: Invalid endpoint number of %d.\n", epNum);
        return 0;
    }

    /* acquire semaphore */
    OSSemPend(usbSemEp[epNum], USB_EP_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemEp[%d] is %d.\n", epNum, err);
        return 0;
    }

    /* check endpoint request exist */
    //if (!(usbEpReq[epNum].svcStat & USB_EP_REQ_STAT_EXIST))
    //{
    //	DEBUG_USB("Error: Endpoint request is not existed.\n");
    //	/* release semaphore */
    //	OSSemPost(usbSemEp[epNum]);
    //	return 0;
    //}

    /* get endpoint request */
    memcpy(pReq, (void *)&usbEpReq[epNum], sizeof(USB_EP_REQ));

    //DEBUG_USB("pReq = 0x%08x, pReq->pData = 0x%08x, pReq->reqSize = 0x%08x\n", (u32)pReq, (u32)pReq->pData, (u32)pReq->reqSize);

    /* release semaphore */
    OSSemPost(usbSemEp[epNum]);

    return 1;
}

/*

Routine Description:

	Set endpoint request.

Arguments:

	epNum - Endpoint number.
	pReq - Endpoint request.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetEpReq(u8 epNum, USB_EP_REQ* pReq)
{
    u8	err;

    //DEBUG_USB("Trace: Set endpoint %d request.\n", epNum);

    /* check if endpoint number valid */
    if (epNum >= USB_EP_MAX)
        return 0;

    /* acquire semaphore */
    OSSemPend(usbSemEp[epNum], USB_EP_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemEp[%d] is %d.\n", epNum, err);
        return 0;
    }

    /* check endpoint request exist */
    //if (usbEpReq[epNum].svcStat & USB_EP_REQ_STAT_EXIST)
    //{
    //	DEBUG_USB("Error: Endpoint request is already existed.\n");
    //	/* release semaphore */
    //	OSSemPost(usbSemEp[epNum]);
    //	return 0;
    //}

    //DEBUG_USB("pReq = 0x%08x, pReq->pData = 0x%08x, pReq->reqSize = 0x%08x\n", (u32)pReq, (u32)pReq->pData, (u32)pReq->reqSize);

    /* set endpoint request */
    memcpy((void *)&usbEpReq[epNum], pReq, sizeof(USB_EP_REQ));
    //usbEpReq[epNum].svcStat |= USB_EP_REQ_STAT_EXIST;

    /* release semaphore */
    OSSemPost(usbSemEp[epNum]);

    return 1;
}

/*

Routine Description:

	Set endpoint request abort.

Arguments:

	epNum - Endpoint number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetEpReqAbort(u8 epNum)
{
    u8	err;

    /* check if endpoint number valid */
    if (epNum >= USB_EP_MAX)
        return 0;

    /* acquire semaphore */
    OSSemPend(usbSemEp[epNum], USB_EP_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemEp[%d] is %d.\n", epNum, err);
        return 0;
    }

    /* check endpoint request exist */
    //if (usbEpReq[epNum].svcStat & USB_EP_REQ_STAT_EXIST)
    //{
    //	/* release semaphore */
    //	OSSemPost(usbSemEp[epNum]);
    //	return 0;
    //}

    /* set endpoint request abort */
    usbEpReq[epNum].svcStat |= USB_EP_REQ_STAT_ABORT;

    /* release semaphore */
    OSSemPost(usbSemEp[epNum]);

    return 1;
}

/*

Routine Description:

	Notify completion of an endpoint.

Arguments:

	epNum - Endpoint number.
	send - Send or receive.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbEpNotify(u8 epNum, u8 send)
{
    u8 err;

    /* stop timeout if any */

    /* check if endpoint number valid */
    if (epNum >= USB_EP_MAX)
        return 0;

    /* acquire semaphore */
    OSSemPend(usbSemEp[epNum], USB_EP_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemEp[%d] is %d.\n", epNum, err);
        return 0;
    }

    if (usbEpReq[epNum].reqOption & USB_API_OPT_NOTIFY)
    {
        /* set return value */
        if (usbEpReqRet[epNum].pRetSize)
            *usbEpReqRet[epNum].pRetSize = usbEpReq[epNum].svcSize;

        if (usbEpReqRet[epNum].pRetStat)
            *usbEpReqRet[epNum].pRetStat = usbEpReq[epNum].svcStat;

        usbApiSendRecvNotify(send);
    }

    /* clear endpoint request */
    memset((void *)&usbEpReq[epNum], 0, sizeof(USB_EP_REQ));

    /* release semaphore */
    OSSemPost(usbSemEp[epNum]);

    return 1;
}

/*

Routine Description:

	Initialize control out of status stage for control in data stage.

Arguments:

	epNum - Endpoint number.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbInitCtrlStatusStage(u8 epNum)
{
    if (epNum == USB_EP_CTRLIN)
    {
        usbCtrlInStatusStage = 1;
        usbEpWriteData(USB_EP_CTRLIN, NULL, 0);

    }
    else /* if (epNum == USB_EP_CTRLOUT) */
    {
        usbCtrlOutStatusStage = 1;
        usbEpReadData(USB_EP_CTRLOUT, NULL, 0);
    }

    return 1;
}

/*

Routine Description:

	Set next control in.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetCtrlIn(void)
{
    USB_EP_REQ epReq;
    u32 sndSize;
    u8 sndEnd = 0;

    /* get endpoint request */
    if (usbGetEpReq(USB_EP_CTRLIN, &epReq) == 0) /* error to get endpoint request */
    {
        DEBUG_USB("Error: No request of control in endpoint.\n");
        return 0;
    }

    if (epReq.reqSize)
    {	/* data send continue */
        /* send data */
        sndSize = (usbEpMaxDmaSize[USB_EP_CTRLIN] < epReq.reqSize) ? usbEpMaxDmaSize[USB_EP_CTRLIN] : epReq.reqSize;
        usbEpWriteData(USB_EP_CTRLIN, epReq.pData+epReq.svcSize, sndSize);

        /* update endpoint request */
        epReq.reqSize -= sndSize;
        epReq.svcSize += sndSize;
    }
    else
    {	/* data send complete */
        if (epReq.reqOption & USB_EP_REQ_OPT_END_ZERO_PKT)
        {	/* send zero-length packet at the end of send */
            epReq.reqOption &= ~USB_EP_REQ_OPT_END_ZERO_PKT;
            usbEpWriteData(USB_EP_CTRLIN, NULL, 0);
        }
        else
        {	/* send end */
            epReq.svcStat = USB_EP_REQ_STAT_OK;
            sndEnd = 1;
        }
    }

    /* set endpoint request */
    usbSetEpReq(USB_EP_CTRLIN, &epReq);

    /* signal send end */
    if (sndEnd)
    {	/* notify result */
        usbEpNotify(USB_EP_CTRLIN, USB_API_SEND);
    }

    return 1;
}

/*

Routine Description:

	Set next control out.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetCtrlOut(void)
{
    USB_EP_REQ epReq;
    u32 rcvSize;
    u8 rcvEnd = 0;
    u8 firstRcv = 0;

    /* get endpoint request */
    if (usbGetEpReq(USB_EP_CTRLOUT, &epReq) == 0) /* error to get endpoint request */
    {
        DEBUG_USB("Error: No request of control out endpoint.\n");
        return 0;
    }

    /* check if first receive */
    if (epReq.reqOption & USB_EP_REQ_OPT_FIRST_RCV)
    {
        epReq.reqOption &= ~USB_EP_REQ_OPT_FIRST_RCV;
        firstRcv = 1;
    }

    /* update endpoint request */
    if (!firstRcv)
    {
        /* get receive size */
        usbGetEpReadData(USB_EP_CTRLOUT, epReq.pData+epReq.svcSize, &rcvSize);

        /* update receive size */
        epReq.reqSize -= rcvSize;
        epReq.svcSize += rcvSize;
    }

    if (epReq.reqSize)
    {	/* data receive may be continue */
        if ((!firstRcv) && (rcvSize < usbEpMaxDmaSize[USB_EP_CTRLOUT]))
        {	/* abort since received short packet */
            epReq.svcStat = USB_EP_REQ_STAT_ABORT;
            rcvEnd = 1;
        }
        else
        {	/* receive data */
            rcvSize = (usbEpMaxDmaSize[USB_EP_CTRLOUT] < epReq.reqSize) ? usbEpMaxDmaSize[USB_EP_CTRLOUT] : epReq.reqSize;
            usbEpReadData(USB_EP_CTRLOUT, epReq.pData+epReq.svcSize, rcvSize);
        }
    }
    else
    {	 /* data receive complete */
        if (epReq.reqOption & USB_EP_REQ_OPT_END_ZERO_PKT)
        {	/* receive zero-length packet at the end of receive */
            epReq.reqOption &= ~USB_EP_REQ_OPT_END_ZERO_PKT;
            usbEpReadData(USB_EP_CTRLOUT, NULL, 0);
        }
        else
        {	/* receive end */
            epReq.svcStat = USB_EP_REQ_STAT_OK;
            rcvEnd = 1;
        }
    }

    /* set endpoint request */
    usbSetEpReq(USB_EP_CTRLOUT, &epReq);

    /* signal receive end */
    if (rcvEnd)
    {	/* TBD: invalid data cache */
        /* notify result */
        usbEpNotify(USB_EP_CTRLOUT, USB_API_RECV);
        /* initialize dma of control in of status stage for control out data stage */
        usbInitCtrlStatusStage(USB_EP_CTRLIN);
    }

    return 1;
}

/*

Routine Description:

	Set next bulk in.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetBulkIn(void)
{
    USB_EP_REQ epReq;
    u32 sndSize;
    u8 sndEnd = 0;
    u8 err;

#if 0
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
#endif
    /* get endpoint request */
    if (usbGetEpReq(USB_EP_BULKIN, &epReq) == 0) /* error to get endpoint request */
    {
        DEBUG_USB("Error: No request of bulk in endpoint.\n");
        return 0;
    }

    if (epReq.reqSize)
    {	/* data send continue */
        if (epReq.svcStat & USB_EP_REQ_STAT_ABORT)
        {	/* abort send */
            DEBUG_USB("Trace: Abort bulk in endpoint.\n");
            epReq.reqSize = 0;
            epReq.reqOption &= ~USB_EP_REQ_OPT_END_ZERO_PKT;
            /* send null packet */
            usbEpWriteData(USB_EP_BULKIN, NULL, 0);
        }
        else
        {	/* continue send */
            /* send data */
            sndSize = (usbEpMaxDmaSize[USB_EP_BULKIN] < epReq.reqSize) ? usbEpMaxDmaSize[USB_EP_BULKIN] : epReq.reqSize;
            usbEpWriteData(USB_EP_BULKIN, epReq.pData+epReq.svcSize, sndSize);

            /* update endpoint request */
            epReq.reqSize -= sndSize;
            epReq.svcSize += sndSize;
        }
    }
    else
    {	/* data send complete */
        if (epReq.reqOption & USB_EP_REQ_OPT_END_ZERO_PKT)
        {	/* send zero-length packet at the end of send */
            epReq.reqOption &= ~USB_EP_REQ_OPT_END_ZERO_PKT;
            usbEpWriteData(USB_EP_BULKIN, NULL, 0);
        }
        else
        {	/* send end */
            epReq.svcStat = USB_EP_REQ_STAT_OK;
            sndEnd = 1;
        }
    }

    /* set endpoint request */
    usbSetEpReq(USB_EP_BULKIN, &epReq);

    /* signal send end */
    if (sndEnd)
    {	/* notify result */
        usbEpNotify(USB_EP_BULKIN, USB_API_SEND);
    }

    return 1;
}

/*

Routine Description:

	Set next bulk out.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetBulkOut(void)
{
    USB_EP_REQ epReq;
    u32 rcvSize;
    u8 rcvEnd = 0;
    u8 firstRcv = 0;

    /* get endpoint request */
    if (usbGetEpReq(USB_EP_BULKOUT, &epReq) == 0) /* error to get endpoint request */
    {
        DEBUG_USB("Error: No request of bulk out endpoint.\n");
        return 0;
    }

    /* check if first receive */
    if (epReq.reqOption & USB_EP_REQ_OPT_FIRST_RCV)
    {
        epReq.reqOption &= ~USB_EP_REQ_OPT_FIRST_RCV;
        firstRcv = 1;
    }

    /* update endpoint request */
    if (!firstRcv)
    {
        /* get receive size */
        usbGetEpReadData(USB_EP_BULKOUT, epReq.pData+epReq.svcSize, &rcvSize);

        /* update receive size */
        epReq.reqSize -= rcvSize;
        epReq.svcSize += rcvSize;
    }

    /* Clear fifo of enpoint bulk out */
    if ((UsbEp2CtrlStat & USB_CS_BULKOUT_FIFO_FULL) != USB_CS_BULKOUT_FIFO_FULL) /* If FIFO is full, do not clear it. bug fixed for nvidia */
//		usbEpClearFifo(USB_EP_BULKOUT);
        UsbEp2CtrlStat |= USB_CS_BULKOUT_FIFO_CLR;

    if (epReq.reqSize)
    {	/* data receive may be continue */
        if ((!firstRcv) && (rcvSize < usbEpMaxDmaSize[USB_EP_BULKOUT]))
        {	/* abort since received short packet */
            DEBUG_USB("Trace: Abort bulk out endpoint.\n");
            epReq.svcStat = USB_EP_REQ_STAT_ABORT;
            rcvEnd = 1;
        }
        else
        {	/* receive data */
            rcvSize = (usbEpMaxDmaSize[USB_EP_BULKOUT] < epReq.reqSize) ? usbEpMaxDmaSize[USB_EP_BULKOUT] : epReq.reqSize;
            usbEpReadData(USB_EP_BULKOUT, epReq.pData+epReq.svcSize, rcvSize);
        }
    }
    else
    {	 /* data receive complete */
        if (epReq.reqOption & USB_EP_REQ_OPT_END_ZERO_PKT)
        {	/* receive zero-length packet at the end of receive */
            epReq.reqOption &= ~USB_EP_REQ_OPT_END_ZERO_PKT;
            usbEpReadData(USB_EP_BULKOUT, NULL, 0);
        }
        else
        {	/* receive end */
            epReq.svcStat = USB_EP_REQ_STAT_OK;
            rcvEnd = 1;
        }
    }

    /* set endpoint request */
    usbSetEpReq(USB_EP_BULKOUT, &epReq);

    /* signal receive end */
    if (rcvEnd)
    {	/* TBD: invalid data cache */
        /* notify result */
        usbEpNotify(USB_EP_BULKOUT, USB_API_RECV);
    }

    return 1;
}

/*

Routine Description:

	Set next interrupt in.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetIntrIn(void)
{
    USB_EP_REQ epReq;
    u32 sndSize;
    u8 sndEnd = 0;

    /* get endpoint request */
    if (usbGetEpReq(USB_EP_INTRIN, &epReq) == 0) /* error to get endpoint request */
    {
        DEBUG_USB("Error: No request of interrupt in endpoint.\n");
        return 0;
    }

    if (epReq.reqSize)
    {	/* data send continue */
        if (epReq.svcStat & USB_EP_REQ_STAT_ABORT)
        {	/* abort send */
            epReq.reqSize = 0;
            epReq.reqOption &= ~USB_EP_REQ_OPT_END_ZERO_PKT;
            /* send null packet */
            usbEpWriteData(USB_EP_INTRIN, NULL, 0);
        }
        else
        {	/* continue send */
            /* send data */
            sndSize = (usbEpMaxDmaSize[USB_EP_INTRIN] < epReq.reqSize) ? usbEpMaxDmaSize[USB_EP_INTRIN] : epReq.reqSize;
            usbEpWriteData(USB_EP_INTRIN, epReq.pData+epReq.svcSize, sndSize);

            /* update endpoint request */
            epReq.reqSize -= sndSize;
            epReq.svcSize += sndSize;
        }
    }
    else
    {	/* data send complete */
        if (epReq.reqOption & USB_EP_REQ_OPT_END_ZERO_PKT)
        {	/* send zero-length packet at the end of send */
            epReq.reqOption &= ~USB_EP_REQ_OPT_END_ZERO_PKT;
            usbEpWriteData(USB_EP_INTRIN, NULL, 0);
        }
        else
        {	/* send end */
            epReq.svcStat = USB_EP_REQ_STAT_OK;
            sndEnd = 1;
        }
    }

    /* set endpoint request */
    usbSetEpReq(USB_EP_INTRIN, &epReq);

    /* signal send end */
    if (sndEnd)
    {	/* notify result */
        usbEpNotify(USB_EP_INTRIN, USB_API_SEND);
    }

    return 1;
}

/*

Routine Description:

	Set next isochronous in.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetIsoIn(void)
{
    USB_EP_REQ epReq;
    u32 sndSize;
    u8 sndEnd = 0;

    /* get endpoint request */
    if (usbGetEpReq(USB_EP_ISOIN, &epReq) == 0) /* error to get endpoint request */
    {
        DEBUG_USB("Error: No request of isochronous in endpoint.\n");
        return 0;
    }

    if (epReq.reqSize)
    {	/* data send continue */
        if (epReq.svcStat & USB_EP_REQ_STAT_ABORT)
        {	/* abort send */
            epReq.reqSize = 0;
            epReq.reqOption &= ~USB_EP_REQ_OPT_END_ZERO_PKT;
            /* send null packet */
            usbEpWriteData(USB_EP_ISOIN, NULL, 0);
        }
        else
        {	/* continue send */
            /* send data */
            sndSize = (usbEpMaxDmaSize[USB_EP_ISOIN] < epReq.reqSize) ? usbEpMaxDmaSize[USB_EP_ISOIN] : epReq.reqSize;
            usbEpWriteData(USB_EP_ISOIN, epReq.pData+epReq.svcSize, sndSize);

            /* update endpoint request */
            epReq.reqSize -= sndSize;
            epReq.svcSize += sndSize;
        }
    }
    else
    {	/* data send complete */
        if (epReq.reqOption & USB_EP_REQ_OPT_END_ZERO_PKT)
        {	/* send zero-length packet at the end of send */
            epReq.reqOption &= ~USB_EP_REQ_OPT_END_ZERO_PKT;
            usbEpWriteData(USB_EP_ISOIN, NULL, 0);
        }
        else
        {	/* send end */
            epReq.svcStat = USB_EP_REQ_STAT_OK;
            sndEnd = 1;
        }
    }

    /* set endpoint request */
    usbSetEpReq(USB_EP_ISOIN, &epReq);

    /* signal send end */
    if (sndEnd)
    {	/* notify result */
        usbEpNotify(USB_EP_ISOIN, USB_API_SEND);
    }

    return 1;
}
