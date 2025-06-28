/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbapievt.h

Abstract:

   	The declarations of USB API event related.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_API_EVT_H__
#define __USB_API_EVT_H__

/* Constant */

/* API event cause index */
enum
{
    USB_API_EVT_TERMINATE = 0,	/* 0x00 - terminate */
    USB_API_EVT_SEND,		/* 0x01 - send */
    USB_API_EVT_RECV,		/* 0x02 - receive */
    USB_API_EVT_EP_CANCEL,		/* 0x03 - endpoint cancel */
    USB_API_EVT_EP_RESET,		/* 0x04 - endpoint reset */
    USB_API_EVT_EP_STALL,		/* 0x05 - endpoint stall */
    USB_API_EVT_EP_CLEAR_STALL,	/* 0x06 - endpoint clear stall */
    USB_API_EVT_REMOTE_WAKEUP,	/* 0x07 - remote wakeup */
    USB_API_EVT_QUERY_STATUS,	/* 0x08 - query status */
    USB_API_EVT_UNDEF		/* 0x09 and larger - undefined */
};

#endif
