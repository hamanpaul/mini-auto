/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbcb.c

Abstract:

   	USB callback function.

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

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

void usbCbAttach(u32);
void usbCbDetach(u32);
void usbCbBusReset(u32);
void usbCbBusSuspend(u32);
void usbCbBusResume(u32);
void usbCbGetStatus(u32);
void usbCbClearFeature(u32);
void usbCbSetFeature(u32);
void usbCbSetAddress(u32);
void usbCbGetDescriptor(u32);
void usbCbSetDescriptor(u32);
void usbCbGetConfiguration(u32);
void usbCbSetConfiguration(u32);
void usbCbGetInterface(u32);
void usbCbSetInterface(u32);
void usbCbSynchFrame(u32);
void usbCbClassReq(u32);
void usbCbVendorReq(u32);

extern s32 usbSetDevAddr(u16);
extern s32 usbDeviceInit(void);
extern s32 usbDevReset(void);


/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* Callback function */
void (*usbCbFunc[])(u32) =
    {
        usbCbAttach,			/* 0x00 - attach */
        usbCbDetach,			/* 0x01 - detach */
        usbCbBusReset,			/* 0x02 - bus reset */
        usbCbBusSuspend,		/* 0x03 - bus suspend */
        usbCbBusResume,			/* 0x04 - bus resume */
        usbCbGetStatus,			/* 0x05 - GET_STATUS */
        usbCbClearFeature,		/* 0x06 - CLEAR_FEATURE */
        usbCbSetFeature,		/* 0x07 - SET_FEATURE */
        usbCbSetAddress,		/* 0x08 - SET_ADDRESS */
        usbCbGetDescriptor,		/* 0x09 - GET_DESCRIPTOR */
        usbCbSetDescriptor,		/* 0x0a - SET_DESCRIPTOR */
        usbCbGetConfiguration,		/* 0x0b - GET_CONFIGURATION */
        usbCbSetConfiguration,		/* 0x0c - SET_CONFIGURATION */
        usbCbGetInterface,		/* 0x0d - GET_INTERFACE */
        usbCbSetInterface,		/* 0x0e - SET_INTERFACE */
        usbCbSynchFrame,		/* 0x0f - SYNCH_FRAME */
        usbCbClassReq,			/* 0x10 - class request */
        usbCbVendorReq,			/* 0x11 - vendor request */
    };

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Attach.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbAttach(u32 param)
{}

/*

Routine Description:

	Detach.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbDetach(u32 param)
{}

/*

Routine Description:

	Bus reset.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbBusReset(u32 param)
{
    usbDeviceInit();
    usbDevReset();
}

/*

Routine Description:

	Bus suspend.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbBusSuspend(u32 param)
{}

/*

Routine Description:

	Bus resume.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbBusResume(u32 param)
{}

/*

Routine Description:

	GET_STATUS.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbGetStatus(u32 param)
{}

/*

Routine Description:

	CLEAR_FEATURE.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbClearFeature(u32 param)
{}

/*

Routine Description:

	SET_FEATURE.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbSetFeature(u32 param)
{}

/*

Routine Description:

	SET_ADDRESS.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbSetAddress(u32 param)
{
    USB_DEV_REQ* pDevReq = (USB_DEV_REQ*) param;
    u32 i;

    /* delay */
    for (i = 0; i < 0x10000; i++)
        i = i;

    usbSetDevAddr(pDevReq->wValue);
}

/*

Routine Description:

	GET_DESCRIPTOR.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbGetDescriptor(u32 param)
{}

/*

Routine Description:

	SET_DESCRIPTOR.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbSetDescriptor(u32 param)
{}

/*

Routine Description:

	GET_CONFIGURATION.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbGetConfiguration(u32 param)
{}

/*

Routine Description:

	SET_CONFIGURATION.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbSetConfiguration(u32 param)
{}

/*

Routine Description:

	GET_INTERFACE.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbGetInterface(u32 param)
{}

/*

Routine Description:

	SET_INTERFACE.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbSetInterface(u32 param)
{}

/*

Routine Description:

	SYNCH_FRAME.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbSynchFrame(u32 param)
{}

/*

Routine Description:

	Class request.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbClassReq(u32 param)
{}

/*

Routine Description:

	Vendor request.

Arguments:

	param - The parameter of this callback function.

Return Value:

	0 - Failure.
	1 - Success.

*/
void usbCbVendorReq(u32 param)
{}
