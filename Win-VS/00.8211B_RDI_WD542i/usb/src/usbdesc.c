/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbdesc.c

Abstract:

   	USB descriptors.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"
#include "usb.h"
#include "usbmsc.h"
#include "usbdesc.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

/* language id */
__align(4) u8 usb_str_desc0[] =
    {
        USB_STR0_bLength,		/* Size of this descriptor */
        USB_STR0_bDescriptorType,	/* String descriptor type */
        USB_STR0_wLANGID,		/* LANGID code */
    };

/* string of manufacturer */
__align(4) u8 usb_str_desc1[] =
    {
        USB_STR1_bLength,		/* Size of this descriptor */
        USB_STR1_bDescriptorType,	/* String descriptor type */
        USB_STR1_bString,		/* UNICODE encoded string */
    };

/* string of product */
__align(4) u8 usb_str_desc2[] =
    {
        USB_STR2_bLength,		/* Size of this descriptor */
        USB_STR2_bDescriptorType,	/* String descriptor type */
        USB_STR2_bString,		/* UNICODE encoded string */
    };

/* string of serial number */
__align(4) u8 usb_str_desc3[] =
    {
        USB_STR3_bLength,		/* Size of this descriptor */
        USB_STR3_bDescriptorType,	/* String descriptor type */
        USB_STR3_bString,		/* UNICODE encoded string */
    };
