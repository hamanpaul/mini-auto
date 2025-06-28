/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbcb.h

Abstract:

   	The declarations of USB callback function related.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_CB_H__
#define __USB_CB_H__

/* Constant */

/* Callback function index */
enum
{
    USB_CB_ATTACH = 0,		/* 0x00 - attach */
    USB_CB_DETACH,			/* 0x01 - detach */
    USB_CB_BUS_RESET,		/* 0x02 - bus reset */
    USB_CB_BUS_SUSPEND,		/* 0x03 - bus suspend */
    USB_CB_BUS_RESUME,		/* 0x04 - bus resume */
    USB_CB_GET_STATUS,		/* 0x05 - GET_STATUS */
    USB_CB_CLEAR_FEATURE,		/* 0x06 - CLEAR_FEATURE */
    USB_CB_SET_FEATURE,		/* 0x07 - SET_FEATURE */
    USB_CB_SET_ADDRESS,		/* 0x08 - SET_ADDRESS */
    USB_CB_GET_DESCRIPTOR,		/* 0x09 - GET_DESCRIPTOR */
    USB_CB_SET_DESCRIPTOR,		/* 0x0a - SET_DESCRIPTOR */
    USB_CB_GET_CONFIGURATION,	/* 0x0b - GET_CONFIGURATION */
    USB_CB_SET_CONFIGURATION,	/* 0x0c - SET_CONFIGURATION */
    USB_CB_GET_INTERFACE,		/* 0x0d - GET_INTERFACE */
    USB_CB_SET_INTERFACE,		/* 0x0e - SET_INTERFACE */
    USB_CB_SYNCH_FRAME,		/* 0x0f - SYNCH_FRAME */
    USB_CB_CLASS_REQ,		/* 0x10 - class request */
    USB_CB_VENDER_REQ,		/* 0x11 - vendor request */
    USB_CB_MAX			/* 0x12 - max allowed */
};

#endif
