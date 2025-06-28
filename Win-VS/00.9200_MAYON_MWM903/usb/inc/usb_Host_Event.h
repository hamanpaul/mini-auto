/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbintevt.h

Abstract:

   	The declarations of USB interrupt event related.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __FARADAY_USB_HOST_INT_EVT_H__
#define __FARADAY_USB_HOST_INT_EVT_H__

/* Constant */
#define USB_HOST_MAX_INT_EVT		32

/* Interrupt event cause index */
enum
{
    USB_HOST_INT_EVT_DEVICE_PLUG_IN = 0x00,	// 0x00 - attach event 
    USB_HOST_INT_EVT_CONTROL_IN,
    USB_HOST_INT_EVT_CONTROL_OUT, 
    USB_HOST_INT_EVT_SET_ADDRESS,
    USB_HOST_INT_EVT_GET_DEVICE_DESCRIPTOR,
    USB_HOST_INT_EVT_GET_CONFIG_DESCRIPTOR,
    USB_HOST_INT_EVT_GET_LANGUAGE_DESCRIPTOR,
    USB_HOST_INT_EVT_GET_CONFIG_DESCRIPTOR_ALL,
    USB_HOST_INT_EVT_SET_CONFIGURATION,
    USB_HOST_INT_EVT_SET_CLEAR_FEATURE,
    USB_HOST_INT_EVT_GET_MAX_LUN,
    USB_HOST_INT_EVT_BULK_IN_DATA,
    USB_HOST_INT_EVT_BULK_IN_STATUS,
    USB_HOST_INT_EVT_BULK_OUT,
    USB_HOST_INT_EVT_INQUIRY,
    USB_HOST_INT_EVT_READ_FORMAT_CAPACITY,
    USB_HOST_INT_EVT_REQUEST_SENSE,
    USB_HOST_INT_EVT_READ_10,
    USB_HOST_INT_EVT_WRITE_10,
    USB_HOST_INT_EVT_UNDEF		             
};

/* USB MSC event cause index */
enum
{
    USB_HOST_MSC_EVT_INIT = 0x00,	        // 0x00 - MSC init 
    USB_HOST_MSC_EVT_READ_10_SECTOR,		        // 0x01 - Read 10
    USB_HOST_MSC_EVT_READ_10_MULTI,		        // 0x02 - Read 10
    USB_HOST_MSC_EVT_WRITE_10_SECTOR,		        // 0x03 - write 10
    USB_HOST_MSC_EVT_WRITE_10_MULTI,                 // 0x04 - write 10 multi-sector
    USB_HOST_MSC_EVT_UNDEF		            // 0x19 and larger - undefined event 
};


#endif
