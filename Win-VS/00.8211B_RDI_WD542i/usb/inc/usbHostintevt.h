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

#ifndef __USB_HOST_INT_EVT_H__
#define __USB_HOST_INT_EVT_H__

/* Constant */
#define USB_HOST_MAX_INT_EVT		32

/* Interrupt event cause index */
enum
{
    USB_HOST_INT_EVT_DEVICE_PLUG_IN = 0x00,	// 0x00 - attach event 
    USB_HOST_INT_EVT_DEVICE_PLUG_OUT,		// 0x01 - detach event 
    USB_HOST_INT_EVT_CHECK_DEVICE_SPEED,    // 0x02	- check device speed
    USB_HOST_INT_EVT_CTRL_TRANS_DONE,		// 0x03	- control transfer done
    USB_HOST_INT_EVT_SETUP,		            // 0x04	- setup packet */
    USB_HOST_INT_EVT_SETUP_OVERW,	        // 0x05	- setup packet overwrite 
    USB_HOST_INT_EVT_BULKIN,		        // 0x06 - bulk in data packet 
    USB_HOST_INT_EVT_BULKOUT,		        // 0x07	- bulk out data packet 
    USB_HOST_INT_EVT_RESET,		            // 0x08	- device reset event 
    USB_HOST_INT_EVT_SUSPEND,		        // 0x09	- device suspend event 
    USB_HOST_INT_EVT_RESUME,		        // 0x0a	- device resume event 
    USB_HOST_INT_EVT_SOF,		            // 0x0b	- start of frame packet 
    USB_HOST_INT_EVT_CTRLIN_NAK,		    // 0x0c	- control in nak packet 
    USB_HOST_INT_EVT_CTRLOUT_NAK,	        // 0x0d	- control out nak packet 
    USB_HOST_INT_EVT_BULKIN_NAK,		    // 0x0e	- bulk in nak packet 
    USB_HOST_INT_EVT_BULKOUT_NAK,	        // 0x0f	- bulk out nak packet 
    USB_HOST_INT_EVT_CTRLIN_STALL,	        // 0x10	- control in stall event 
    USB_HOST_INT_EVT_CTRLOUT_STALL,	        // 0x11	- control out stall event 
    USB_HOST_INT_EVT_INTRIN,		        // 0x12	- interrupt in data packet 
    USB_HOST_INT_EVT_INTRIN_NAK,		    // 0x13 - interrupt in nak packet 
    USB_HOST_INT_EVT_ISOIN,		            // 0x14 - isochronous in data packet 
    USB_HOST_INT_EVT_ISOIN_ZEROL,	        // 0x15 - isochronous in zero-length data packet 
    USB_HOST_INT_EVT_BULKIN_DMA_CMPL,	    // 0x16 - bulk in dma completion event 
    USB_HOST_INT_EVT_BULKOUT_DMA_CMPL,	    // 0x17 - bulk out dma completion event 
    USB_HOST_INT_EVT_ISOIN_DMA_CMPL,	    // 0x18 - isochronous in dma completion event 
    USB_HOST_INT_EVT_UNDEF		            // 0x19 and larger - undefined event 
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
