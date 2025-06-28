/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usb.c

Abstract:

   	USB routine.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"
#include "board.h"

#include "task.h"
#include "usb.h"
#include "usbreg.h"
#include "usbdev.h"
#include "usbapievt.h"
#include "usbintevt.h"
#include "usbapi.h"
#include "gpioapi.h"
#include "sdcapi.h"
#include "osapi.h"
#include "sysapi.h"

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

void usbTask(void*);

s32 usbManageInit(void);
s32 usbResourceInit(void);

s32 usbSetIntEvt(u8);
s32 usbGetIntEvt(u8*);
s32 usbSetApiEvt(USB_API_EVT_SNAP*);
s32 usbGetApiEvt(USB_API_EVT_SNAP*);
void usbDevEnaCtrl(u8);
s32 usbInit(void);
u8 usbSetUsbPluginStat(u8);
u8 usbSetInitUsb(void);
u8 usbUninst(void);



/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
#define usbDebugPrint 			DEBUG_UHOST

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

OS_STK usbTaskStack[USB_TASK_STACK_SIZE]; /* Stack of task usbTask() */

OS_EVENT* usbSemEvt; /* semaphore to synchronize event processing */
OS_EVENT* usbSemEp[USB_SEM_EP_MAX]; /* semaphore to synchronize endpoint access */
OS_EVENT* usbSemApi[USB_SEM_API_MAX]; /* semaphore to synchronize api event processing */
OS_EVENT* usbSemApiNotify[USB_SEM_API_NOTIFY_MAX]; /* semaphore to synchronize api event processing */

USB_INT_EVT usbIntEvt; /* Interrupt event queue */
USB_EP_REQ usbEpReq[USB_EP_MAX]; /* Endpoint request */
USB_EP_REQ_RET usbEpReqRet[USB_EP_MAX]; /* Endpoint request return */

USB_API_EVT usbApiEvt; /* API event queue */
USB_API_EVT_SNAP usbApiCurEvt; /* API current event */

extern u32 usbEpMaxPktSize[USB_EP_MAX];
extern u32 usbEpMaxDmaSize[USB_EP_MAX];

extern void (*usbIntEvtFunc[])(void);
extern void (*usbApiEvtFunc[])(void);

extern OS_EVENT* dmaSemSDTxfFin;

extern u8 ucSetAddressReq;
extern  USB_DEV_SETTING usbDevSetting;

extern OS_FLAG_GRP  *gSdUsbProcFlagGrp;

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Initialize USB.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbInit(void)
{
	u8	ucLevel;

    /* initialize management structure */
    usbManageInit();

    /* initialize resource */
    usbResourceInit();

    /* initialize device */
    usbDeviceInit();

    /* configure class */
    usbClassCfg();

    /* initialize class */
    usbClassInit();

    /* Create the task */
    //DEBUG_USB("Trace: USB task creating.\n");
    OSTaskCreate(USB_TASK, USB_TASK_PARAMETER, USB_TASK_STACK, USB_TASK_PRIORITY);


	/* set the usb plugin status */
	//gpioGetLevel(0, GPIO_CHECKBIT_USB, &ucLevel);
	if (ucLevel == 1)
		usbSetUsbPluginStat(USB_SITUATION_BOOT);
	else
		usbSetUsbPluginStat(USB_SITUATION_NONE);



    return 1;
}

/*

Routine Description:

	The USB driver task.

Arguments:

	pData - The task parameter.

Return Value:

	None.

*/
void usbTask(void* pData)
{
    u8 err;
    u8 cause;

    while (1)
    {
        if (usbGetIntEvt(&cause))
        {
            (*usbIntEvtFunc[cause])();
        }
        else if (usbGetApiEvt(&usbApiCurEvt))
        {
            (*usbApiEvtFunc[usbApiCurEvt.cause])();
        }
        else
        {
            OSSemPend(usbSemEvt, OS_IPC_WAIT_FOREVER, &err);
            if (err != OS_NO_ERR)
            {
                DEBUG_USB("Error: usbSemEvt is %d.\n", err);
                //return ;
            }
        }
    }
}

/*

Routine Description:

	The IRQ handler of USB.

Arguments:

	None.

Return Value:

	None.

*/
void usbIntHandler(void)
{
    u32 intStat = usbGetIntStat();

    if (intStat & USB_INT_STAT_CTRLIN)
    {
        /* control in data packet*/
        usbSetIntEvt(USB_INT_EVT_CTRLIN);

        if (ucSetAddressReq == TRUE)
        {
            UsbDevAddr = usbDevSetting.devAddr;
            ucSetAddressReq = FALSE;
        }

    }

    if (intStat & USB_INT_STAT_CTRLOUT)
    {
        /* control out data packet */
        usbSetIntEvt(USB_INT_EVT_CTRLOUT);
    }

    if (intStat & USB_INT_STAT_SETUP)
    {
        /* setup packet */
        usbSetIntEvt(USB_INT_EVT_SETUP);
    }

    if (intStat & USB_INT_STAT_SETUP_OVERW)
    {
        /* setup packet overwrite */
        usbSetIntEvt(USB_INT_EVT_SETUP_OVERW);
    }

    if (intStat & USB_INT_STAT_BULKIN)
    {
        /* bulk in data packet */
        usbSetIntEvt(USB_INT_EVT_BULKIN);
    }

    if (intStat & USB_INT_STAT_BULKOUT)
    {

        if (UsbEp2DataLen == 0x1F)
        {
            UsbIntEna &= ~ USB_INT_ENA_BULKOUT;
            UsbEp2CtrlStat &= ~ USB_CS_BULKOUT_ACK;
        }

        /* bulk out data packet */
        usbSetIntEvt(USB_INT_EVT_BULKOUT);
    }

    if (intStat & USB_INT_STAT_RESET)
    {
        /* device reset event */
        usbSetIntEvt(USB_INT_EVT_RESET);
    }

    if (intStat & USB_INT_STAT_SUSPEND)
    {
        /* device suspend event */
        usbSetIntEvt(USB_INT_EVT_SUSPEND);
    }

    if (intStat & USB_INT_STAT_RESUME)
    {
        /* device resume event */
        usbSetIntEvt(USB_INT_EVT_RESUME);
    }

    if (intStat & USB_INT_STAT_SOF)
    {
        /* start of frame packet */
        usbSetIntEvt(USB_INT_EVT_SOF);
    }

    if (intStat & USB_INT_STAT_CTRLIN_NAK)
    {
        /* control in nak packet */
        usbSetIntEvt(USB_INT_EVT_CTRLIN_NAK);
    }

    if (intStat & USB_INT_STAT_CTRLOUT_NAK)
    {
        /* control out nak packet */
        usbSetIntEvt(USB_INT_EVT_CTRLOUT_NAK);
    }

    if (intStat & USB_INT_STAT_BULKIN_NAK)
    {
        /* bulk in nak packet */
        usbSetIntEvt(USB_INT_EVT_BULKIN_NAK);
    }

    if (intStat & USB_INT_STAT_BULKOUT_NAK)
    {
        /* bulk out nak packet */
        usbSetIntEvt(USB_INT_EVT_BULKOUT_NAK);
    }

    if (intStat & USB_INT_STAT_CTRLIN_STALL)
    {
        /* control in stall event */
        usbSetIntEvt(USB_INT_EVT_CTRLIN_STALL);
    }

    if (intStat & USB_INT_STAT_CTRLOUT_STALL)
    {
        /* control out stall event */
        usbSetIntEvt(USB_INT_EVT_CTRLOUT_STALL);
    }

    if (intStat & USB_INT_STAT_INTRIN)
    {
        /* interrupt in data packet */
        usbSetIntEvt(USB_INT_EVT_INTRIN);
    }

    if (intStat & USB_INT_STAT_INTRIN_NAK)
    {
        /* interrupt in nak packet */
        usbSetIntEvt(USB_INT_EVT_INTRIN_NAK);
    }

    if (intStat & USB_INT_STAT_ISOIN)
    {
        /* isochronous in data packet */
        usbSetIntEvt(USB_INT_EVT_ISOIN);
    }

    if (intStat & USB_INT_STAT_ISOIN_ZEROL)
    {
        /* isochronous in zero-length data packet */
        usbSetIntEvt(USB_INT_EVT_ISOIN_ZEROL);
    }

    if (intStat & USB_INT_STAT_BULKIN_DMA_CMPL)
    {
        /* bulk in dma completion event */
        usbSetIntEvt(USB_INT_EVT_BULKIN_DMA_CMPL);
    }

    if (intStat & USB_INT_STAT_BULKOUT_DMA_CMPL)
    {
        /* bulk out dma completion event */
        usbSetIntEvt(USB_INT_EVT_BULKOUT_DMA_CMPL);
    }

    if (intStat & USB_INT_STAT_ISOIN_DMA_CMPL)
    {
        /* isochronous in dma completion event */
        usbSetIntEvt(USB_INT_EVT_ISOIN_DMA_CMPL);
    }
}

/* allen 20081120 S */
/*

Routine Description:

	USB device enable control by pull-high resistor.

Arguments:

	ucResCtrl - control level of pull-high resistor.

Return Value:

	None.

*/
void usbDevEnaCtrl(u8 ucResCtrl)
{
}
/* allen 20081120 E */

/*

Routine Description:

	USB manage initialization.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbManageInit(void)
{
    /* zero initialize structure */
    memset((void *)&usbIntEvt, 0, sizeof(USB_INT_EVT));
    memset((void *)&usbApiEvt, 0, sizeof(USB_API_EVT));
    memset((void *)&usbEpReq[0], 0, sizeof(USB_EP_REQ) * USB_EP_MAX);
    memset((void *)&usbEpReqRet[0], 0, sizeof(USB_EP_REQ_RET) * USB_EP_MAX);

    return 1;
}

/*

Routine Description:

	USB resource initialization.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbResourceInit(void)
{
    u8 i;

    /* Create the semaphore */
    usbSemEvt = OSSemCreate(0);

    for (i = 0; i < USB_SEM_EP_MAX; i++)
        usbSemEp[i] = OSSemCreate(1);

    for (i = 0; i < USB_SEM_API_MAX; i++)
        usbSemApi[i] = OSSemCreate(1);

    for (i = 0; i < USB_SEM_API_NOTIFY_MAX; i++)
        usbSemApiNotify[i] = OSSemCreate(0);


    return 1;
}


/*

Routine Description:

	USB resource un-initialization.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbResourceUnInit(void)
{
	u8	err;
	u8	i;

	/* del semaphore usbSemEvt */
	usbSemEvt=OSSemDel(usbSemEvt, OS_DEL_ALWAYS, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del usbSemEvt Failed = %d\n", err);
		return 0;
	}
	
	/* del semaphore usbSemEp */
	for (i = 0; i < USB_SEM_EP_MAX; i++)
    {
		usbSemEp[i]=OSSemDel(usbSemEp[i], OS_DEL_ALWAYS, &err);
		if (err != OS_NO_ERR)
		{
			DEBUG_USB("Del usbSemEp[%d] Failed = %d\n", i, err);
			return 0;
		}
	}
	
	/* del semaphore usbSemApi */
    for (i = 0; i < USB_SEM_API_MAX; i++)
    {
		usbSemApi[i]=OSSemDel(usbSemApi[i], OS_DEL_ALWAYS, &err);
		if (err != OS_NO_ERR)
		{
			DEBUG_USB("Del usbSemApi[%d] Failed = %d\n", i, err);
			return 0;
		}
	}

	/* del semaphore usbSemApiNotify */
    for (i = 0; i < USB_SEM_API_NOTIFY_MAX; i++)
    {
		usbSemApiNotify[i]=OSSemDel(usbSemApiNotify[i], OS_DEL_ALWAYS, &err);
		if (err != OS_NO_ERR)
		{
			DEBUG_USB("Del usbSemApiNotify[%d] Failed = %d\n", i, err);
			return 0;
		}
	}


	return 1;}

/*

Routine Description:

	Set interrupt event.

Arguments:

	cause - Cause of the event to set.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetIntEvt(u8 cause)
{
    /* check if cause is valid */
    if (cause >= USB_INT_EVT_UNDEF)
    {	/* cause out of range */
        return 0;
    }

    /* set the cause */
    usbIntEvt.cause[usbIntEvt.idxSet++] = cause;

    if (usbIntEvt.idxSet == USB_MAX_INT_EVT)
    {	/* wrap around the index */
        usbIntEvt.idxSet = 0;
    }

    /* check if event queue is full */
    if (usbIntEvt.idxSet == usbIntEvt.idxGet)
    {
        /* event queue is full */
        return 0;
    }

    /* signal event semaphore */
    OSSemPost(usbSemEvt);

    return 1;
}

/*

Routine Description:

	Get interrupt event.

Arguments:

	pCause - Cause of the event got.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbGetIntEvt(u8* pCause)
{
    /* check if event queue is empty */
    if (usbIntEvt.idxGet == usbIntEvt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    /* get the cause */
    *pCause = usbIntEvt.cause[usbIntEvt.idxGet++];

    if (usbIntEvt.idxGet == USB_MAX_INT_EVT)
    {	/* wrap around the index */
        usbIntEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (*pCause >= USB_INT_EVT_UNDEF)
    {	/* cause out of range */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Set API event.

Arguments:

	cause - Cause of the event to set.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbSetApiEvt(USB_API_EVT_SNAP* pEvt)
{
    u8 err;

    /* acquire api semaphore */
    OSSemPend(usbSemApi[USB_SEM_API], USB_API_REQ_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("Error: usbSemApi[USB_SEM_API] is %d.\n", err);
        return 0;
    }

    /* check if cause is valid */
    if (pEvt->cause >= USB_API_EVT_UNDEF)
    {	/* cause out of range */
        return 0;
    }

    /* set api event */
    usbApiEvt.cause[usbApiEvt.idxSet]  = pEvt->cause;
    usbApiEvt.epNum[usbApiEvt.idxSet]  = pEvt->epNum;
    usbApiEvt.option[usbApiEvt.idxSet] = pEvt->option;

    /* advance set index */
    usbApiEvt.idxSet++;
    if (usbApiEvt.idxSet == USB_MAX_API_EVT)
    {	/* wrap around the index */
        usbApiEvt.idxSet = 0;
    }

    /* check if event queue is full */
    if (usbApiEvt.idxSet == usbApiEvt.idxGet)
    {	/* event queue is full */
        return 0;
    }

    /* release api semaphore */
    OSSemPost(usbSemApi[USB_SEM_API]);

    /* signal event semaphore */
    OSSemPost(usbSemEvt);

    return 1;
}

/*

Routine Description:

	Get API event.

Arguments:

	pEvt - Event got.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 usbGetApiEvt(USB_API_EVT_SNAP* pEvt)
{
    /* check if event queue is empty */
    if (usbApiEvt.idxGet == usbApiEvt.idxSet)
    {
        /* event queue is empty */
        return 0;
    }

    /* get api event */
    pEvt->cause  = usbApiEvt.cause[usbApiEvt.idxGet];
    pEvt->epNum  = usbApiEvt.epNum[usbApiEvt.idxGet];
    pEvt->option = usbApiEvt.option[usbApiEvt.idxGet];

    /* advance get index */
    usbApiEvt.idxGet++;
    if (usbApiEvt.idxGet == USB_MAX_API_EVT)
    {	/* wrap around the index */
        usbApiEvt.idxGet = 0;
    }

    /* check if cause is valid */
    if (pEvt->cause >= USB_API_EVT_UNDEF)
    {	/* cause out of range */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Set usb plugin status.

Arguments:

	ucSituation - situation to set usb plug-in status.

Return Value:

	Usb plugin status.

*/
u8 usbSetUsbPluginStat(u8 ucSituation)
{
	u8	err;


    OSFlagPend(gSdUsbProcFlagGrp, FLAGUSB_SET_PLUGIN_STAT, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, USB_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_SYS("ERROR USB: OSFlagPend usbSetUsbPluginStat = %d!\n", err);
		return 0;
	}

	switch (ucSituation)
	{

		case USB_SITUATION_NONE:	/* Plug-out, reset the status */
			gucUsbPlugInStat = USB_SITUATION_NONE;
			break;

		case USB_SITUATION_BOOT:	/* boot */
			gucUsbPlugInStat = USB_PLUGIN_STAT_ONLY_POWER;
			break;

		case USB_SITUATION_PC: 		/* plug-in host */
			gucUsbPlugInStat &= ~USB_PLUGIN_STAT_ONLY_POWER;
			gucUsbPlugInStat |= USB_PLUGIN_STAT_PC;
			break;
			
		case USB_SITUATION_MASS_STORAGE: 		/* recognize mass storage */
			gucUsbPlugInStat &= ~USB_PLUGIN_STAT_ONLY_POWER;
			gucUsbPlugInStat |= (USB_PLUGIN_STAT_PC | USB_PLUGIN_STAT_MASS_STORAGE);
			break;
	}

	OSFlagPost(gSdUsbProcFlagGrp, FLAGUSB_SET_PLUGIN_STAT, OS_FLAG_SET, &err);

    return gucUsbPlugInStat;
}

/*

Routine Description:

	Set set and init usb.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 usbSetInitUsb(void)
{
	
	/* init usb task */
	DEBUG_UI("uiTask: usbInit()\n");
	sysUSB_enable();

	usbInit();

	sysUSB_disable();

	/* enable pull-high resistor */
    gpioCheckLevel_USB();
	
	return 1;

}


/*

Routine Description:

	Uninstall usb setting and release source occupied by usb.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 usbUninst(void)
{
	u8	err;

	DEBUG_USB("USB Uninst\n");

	/* del USB Task */
	err = OSTaskDel(USB_TASK_PRIORITY);
	if (err != OS_NO_ERR)
	{
		DEBUG_USB("Del USB_TASK_PRIORITY Failed = %d\n", err);
		return 0;
	}

	/* uninst Class resource */
	usbMscUnInit();

	/* uninst resource */
	usbResourceUnInit();

	return 1;
}

