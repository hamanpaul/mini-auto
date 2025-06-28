/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usb.h

Abstract:

   	The declarations of USB protocol.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_H__
#define __USB_H__

/*---------------- Device Request ----------------*/

#define USB_DEV_REQ_LEN		8       /* Device request length */
typedef __packed struct _USB_DEV_REQ
{
    u8	bmRequestType;
    u8      bRequest;
    u16     wValue;
    u16	wIndex;
    u16	wLength;
}
USB_DEV_REQ;

/* bmRequestType */
#define USB_DEV_REQ_DIR_MASK		0x80	/* Data transfer direction */
#define USB_DEV_REQ_DIR_H2D		0x00	/* Host-to-device */
#define USB_DEV_REQ_DIR_D2H		0x80	/* Device-to-host */

#define USB_DEV_REQ_TYP_MASK		0x60	/* Type */
#define USB_DEV_REQ_TYP_STANDARD	0x00	/* Standard */
#define USB_DEV_REQ_TYP_CLASS		0x20	/* Class */
#define USB_DEV_REQ_TYP_VENDOR		0x40	/* Vendor */
#define USB_DEV_REQ_TYP_RESERVED	0x60	/* Reserved */

#define USB_DEV_REQ_REC_MASK		0x1f	/* Recipient */
#define USB_DEV_REQ_REC_DEV		0x00	/* Device */
#define USB_DEV_REQ_REC_IF		0x01	/* Interface */
#define USB_DEV_REQ_REC_EP		0x02	/* Endpoint */

/* bRequest - Request Code (RC) */
#define USB_RC_GET_STATUS		0x00	/* GET_STATUS */
#define USB_RC_CLEAR_FEATURE		0x01	/* CLEAR_FEATURE */
#define USB_RC_SET_FEATURE		0x03	/* SET_FEATURE */
#define USB_RC_SET_ADDRESS		0x05	/* SET_ADDRESS */
#define USB_RC_GET_DESCRIPTOR		0x06	/* GET_DESCRIPTOR */
#define USB_RC_SET_DESCRIPTOR		0x07	/* SET_DESCRIPTOR */
#define USB_RC_GET_CONFIGURATION	0x08	/* GET_CONFIGURATION */
#define USB_RC_SET_CONFIGURATION	0x09	/* SET_CONFIGURATION */
#define USB_RC_GET_INTERFACE		0x0a	/* GET_INTERFACE */
#define USB_RC_SET_INTERFACE		0x0b	/* SET_INTERFACE */
#define USB_RC_SYNCH_FRAME		0x0c	/* SYNCH_FRAME */

/* wValue - Feature selector (FS) */
#define USB_FS_ENDPOINT_HALT		0	/* Endpoint stall */
#define USB_FS_ENDPOINT_STALL		USB_FS_ENDPOINT_HALT
#define USB_FS_DEVICE_REMOTE_WAKEUP	1	/* Device remote wakeup */
#define USB_FS_TEST_MODE		2

/*---------------- Standard Descriptor ----------------*/

/* descriptor type */
#define	USB_DESC_TYPE_DEVICE			0x01
#define USB_DESC_TYPE_CONFIGURATION		0x02
#define USB_DESC_TYPE_STRING			0x03
#define USB_DESC_TYPE_INTERFACE			0x04
#define USB_DESC_TYPE_ENDPOINT			0x05
#define USB_DESC_TYPE_DEVICE_QUALIFIER		0x06
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION	0x07
#define USB_DESC_TYPE_INTERFACE_POWER		0x08

/* descriptor length */
#define	USB_DESC_LEN_DEVICE			0x12
#define USB_DESC_LEN_CONFIGURATION		0x09
#define USB_DESC_LEN_STRING			0x00		/* TBD */
#define USB_DESC_LEN_INTERFACE			0x09
#define USB_DESC_LEN_ENDPOINT			0x07
#define USB_DESC_LEN_DEVICE_QUALIFIER		0x0a
#define USB_DESC_LEN_OTHER_SPEED_CONFIGURATION	0x09
#define USB_DESC_LEN_INTERFACE_POWER		0x00		/* TBD */

/*---- Device descriptor ----*/
typedef __packed struct _USB_DEV_DESC
{
    u8	bLength;			/* Size of this descriptor */
    u8	bDescriptorType; 		/* Device descriptor type */
    u16	bcdUSB;				/* USB Specification Release Number in BCD */
    u8	bDeviceClass;			/* Class code */
    u8	bDeviceSubClass;		/* Subclass code */
    u8	bDeviceProtocol;		/* Protocol code */
    u8	bMaxPacketSize0;		/* Max. packet size for endpoint zero */
    u16	idVendor;			/* Vendor ID */
    u16	idProduct;			/* Product ID */
    u16	bcdDevice;			/* Device release number in BCD */
    u8	iManufacturer;			/* Index of string descriptor of manufacturer */
    u8	iProduct;			/* Index of string descriptor of product */
    u8	iSerialNumber;			/* Index of string descriptor of serial number */
    u8	bNumConfigurations;		/* Number of configurations */
}
USB_DEV_DESC;

/* bcdUSB */
#define USB_VERSION				0x0110		/* Version 1.1 */

/*---- Device qualifier descriptor - for USB rev. 2.0 only ----*/
typedef __packed struct _USB_DEV_QUAL_DESC
{
    u8	bLength;			/* Size of this descriptor */
    u8 	bDescriptorType;		/* Device qualifier descriptor type */
    u16	bcdUSB;				/* USB specification version number */
    u8	bDeviceClass;			/* Class code */
    u8	bDeviceSubClass;		/* Subclass code */
    u8	bDeviceProtocol;		/* Protocol code */
    u8	bMaxPacketSize0;		/* Max. packet size for endpoint zero */
    u8	bNumConfigurations;		/* Number of other-speed configurations */
    u8	bReserved;			/* Reserved */
}
USB_DEV_QUAL_DESC;

/*---- Configuration descriptor ----*/
typedef __packed struct _USB_CFG_DESC
{
    u8	bLength;                	/* Size of this descriptor */
    u8	bDescriptorType;		/* Configuration descriptor type */
    u16	wTotalLength;			/* Total length of this configuration */
    u8	bNumInterfaces;			/* Number of interfaces */
    u8	bConfigurationValue;		/* Value to select this configuration */
    u8	iConfiguration;			/* Index of string descriptor of this configuration */
    u8	bmAttributes;			/* Configuration charateristics */
    u8	MaxPower; 			/* Max. power consumption */
}
USB_CFG_DESC;

/* bmAttributes */
#define USB_CFG_ATTR_BUS_POWER       0x80	/* Bus-powered */
#define USB_CFG_ATTR_SELF_POWER      0x40	/* Self-powered */
#define USB_CFG_ATTR_REMOTE_WAKEUP   0x20	/* Remote wakeup */

/*---- Other speed configuration descriptor - for USB rev. 2.0 only ----*/
typedef __packed struct _USB_OTHER_SPEED_CFG_DESC
{
    u8	bLength;			/* size of this descriptor */
    u8	bDescriptorType;		/* Other speed configuration descriptor type */
    u16	wTotalLength;			/* Total length of this configuration */
    u8	bNumInterfaces;			/* Number of interfaces */
    u8	bConfigurationValue;		/* Value to select this configuration */
    u8	iConfiguration;			/* Index of string descriptor of this configuration */
    u8	bmAttributes;			/* Configuration charateristics */
    u8	bMaxPower;			/* Max. power consumption */
}
USB_OTHER_SPEED_CFG_DESC;

/*---- Intreface descriptor ----*/
typedef __packed struct _USB_IF_DESC
{
    u8	bLength;			/* Size of this descriptor */
    u8	bDescriptorType;		/* Interface descriptor type */
    u8	bInterfaceNumber;		/* Interface number */
    u8	bAlternateSetting;		/* Alternate setting */
    u8	bNumEndpoints;			/* Number of endpoints */
    u8	bInterfaceClass;		/* Class code */
    u8	bInterfaceSubClass;		/* Subclass code */
    u8	bInterfaceProtocol;		/* Protocol code */
    u8	iInterface;			/* Index of string descriptor of this interface */
}
USB_IF_DESC;

/*---- Endpoint descriptor ----*/
typedef __packed struct _USB_EP_DESC
{
    u8	bLength;			/* Size of this descriptor */
    u8	bDescriptorType;		/* Endpoint descriptor type  */
    u8	bEndpointAddress;		/* Endpoint address */
    u8	bmAttributes;			/* Endpoint attributes */
    u16	wMaxPacketSize;			/* Max. packet size */
    u8	bInterval;			/* Interval for polling endpoint for data transfer */
}
USB_EP_DESC;

/* bEndpointAddress */
#define USB_EP_OUT			0x00    /* Direction: OUT endpoint */
#define USB_EP_IN			0x80    /*            IN endpoint  */

/* bmAttributes */
#define USB_EP_ATTR_CTRL		0x00    /* Transfer type: Control     */
#define USB_EP_ATTR_ISO			0x01    /* 		  Isochronous */
#define USB_EP_ATTR_BULK		0x02    /*		  Bulk        */
#define USB_EP_ATTR_INTR		0x03    /* 		  Interrupt   */

/*---- String descriptor ----*/
typedef __packed struct _USB_STR_DESC
{
    u8	bLength;			/* Size of this descriptor */
    u8	bDescriptorType;		/* String descriptor type */
    u16	bString[127];			/* UNICODE string */
}
USB_STR_DESC;

/* Device status of Get Status */
#define USB_DEV_STAT_SELF_POWERED	0x0001
#define USB_DEV_STAT_REMOTE_WAKEUP	0x0002

#define USB_DEV_STAT_HALT		0x0001

#endif
