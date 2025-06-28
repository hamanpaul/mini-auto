/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbreg.h

Abstract:

   	The registers of USB controller.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_HOSTREG_H__
#define __USB_HOSTREG_H__

/* UsbIntEna */
#define USB_INT_ENA_CTRLIN		0x00000001

/* Usb IntStat */
#define HOST_INT_STAT		    0x00000001
#define DEVICE_INT_STAT		    0x00000002
#define OTG_INT_STAT		    0x00000004
#define OTG_INT_ID_STAT         0x00000040
#define DEV_INT_EP0              0x00010000

/* Host IntStat */
#define PORT_CONNECTION		    0x00000001
#define PORT_RESET_DONE		    0x00000010
#define PORT_CONNECTION_RHS		0x00000010
#define DEVICE_SPEED_MASK		0x00000180

#define EP0_TRAN_DONE_INT		0x00000100
#define EP1_TRAN_DONE_INT		0x00000200
#define EP2_TRAN_DONE_INT		0x00000400
#define EP3_TRAN_DONE_INT		0x00000800

/* Host recgonize the speed for device */
#define NO_DEVICE		    0x00000000
#define FULL_SPEED		    0x00000001
#define LOW_SPEED		    0x00000002
#define HIGH_SPEED		    0x00000004


#endif
