/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbdesc.h

Abstract:

   	The declarations of USB descriptors.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_DESC_H__
#define __USB_DESC_H__

#include "usbmsc.h"


/*---------------- mass storage class ----------------*/

/* Structure */

/* configuration */
typedef __packed struct _USB_MSC_CONFIGURATION_DESC
{
    USB_CFG_DESC	cfg;
    USB_IF_DESC	if0Alt0;
    USB_EP_DESC	if0Alt0Ep1;
    USB_EP_DESC	if0Alt0Ep2;
}
USB_MSC_CONFIGURATION_DESC;

/* Constant */

/* device descriptor */
#define USB_MSC_DEV_bLength			USB_DESC_LEN_DEVICE
/* Size of this descriptor */
#define USB_MSC_DEV_bDescriptorType		USB_DESC_TYPE_DEVICE
/* Device descriptor type */
#define USB_MSC_DEV_bcdUSB			USB_VERSION	/* USB Specification Release Number in BCD */
#define USB_MSC_DEV_bDeviceClass		0x00		/* Class code */
#define USB_MSC_DEV_bDeviceSubClass		0x00		/* Subclass code */
#define USB_MSC_DEV_bDeviceProtocol		0x00		/* Protocol code */
#define USB_MSC_DEV_bMaxPacketSize0		0x08		/* Max. packet size for endpoint zero */
#define USB_MSC_DEV_idVendor			USB_MSC_DEV_VENDOR_ID
/* Vendor ID */
#define USB_MSC_DEV_idProduct			USB_MSC_DEV_PRODUCT_ID
/* Product ID */
#define USB_MSC_DEV_bcdDevice			0x0000		/* Device release number in BCD */
#define USB_MSC_DEV_iManufacturer		0x01		/* Index of string descriptor of manufacturer */
#define USB_MSC_DEV_iProduct			0x02		/* Index of string descriptor of product */
#define USB_MSC_DEV_iSerialNumber		0x03		/* Index of string descriptor of serial number */
#define USB_MSC_DEV_bNumConfigurations		0x01		/* Number of configurations */

/* device qualifier descriptor - for USB rev. 2.0 only */
#define USB_MSC_DEV_QUAL_bLength		USB_DESC_LEN_DEVICE_QUALIFIER
/* Size of this descriptor */
#define USB_MSC_DEV_QUAL_bDescriptorType	USB_DESC_TYPE_DEVICE_QUALIFIER
/* Device qualifier descriptor type */
#define USB_MSC_DEV_QUAL_bcdUSB			0x0200		/* USB specification version number */
#define USB_MSC_DEV_QUAL_bDeviceClass		0x00		/* Class code */
#define USB_MSC_DEV_QUAL_bDeviceSubClass	0x00		/* Subclass code */
#define USB_MSC_DEV_QUAL_bDeviceProtocol	0x00		/* Protocol code */
#define USB_MSC_DEV_QUAL_bMaxPacketSize0	0x40		/* Max. packet size for endpoint zero */
#define USB_MSC_DEV_QUAL_bNumConfigurations	0x01		/* Number of other-speed configurations */
#define USB_MSC_DEV_QUAL_bReserved		0x00		/* Reserved */

/* interface 0 alternate setting 0 descriptor */
#define USB_MSC_IF0_ALT0_bLength		USB_DESC_LEN_INTERFACE
/* Size of this descriptor */
#define USB_MSC_IF0_ALT0_bDescriptorType	USB_DESC_TYPE_INTERFACE
/* Interface descriptor type */
#define USB_MSC_IF0_ALT0_bInterfaceNumber	0x00		/* Interface number */
#define USB_MSC_IF0_ALT0_bAlternateSetting	0x00		/* Alternate setting */
#define USB_MSC_IF0_ALT0_bNumEndpoints		0x02		/* Number of endpoints */
#define USB_MSC_IF0_ALT0_bInterfaceClass	USB_MSC_CLASS_MASS_STORAGE
/* Class code - mass storage class */
#define USB_MSC_IF0_ALT0_bInterfaceSubClass	USB_MSC_SUBCLASS_SCSI
/* Subclass code */
#define USB_MSC_IF0_ALT0_bInterfaceProtocol	USB_MSC_PROTOCOL_BO
/* Protocol code */
#define USB_MSC_IF0_ALT0_iInterface		0x00		/* Index of string descriptor of this interface */

/* interface 0 alternate setting 0 endpoint 1 descriptor */
#define USB_MSC_IF0_ALT0_EP1_bLength		USB_DESC_LEN_ENDPOINT
/* Size of this descriptor */
#define USB_MSC_IF0_ALT0_EP1_bDescriptorType	USB_DESC_TYPE_ENDPOINT
/* Endpoint descriptor type  */
#define USB_MSC_IF0_ALT0_EP1_bEndpointAddress	(USB_EP_IN | 0x01)	/* Endpoint address */
#define USB_MSC_IF0_ALT0_EP1_bmAttributes	USB_EP_ATTR_BULK	/* Endpoint attributes */
#define USB_MSC_IF0_ALT0_EP1_wMaxPacketSize	0x0040		/* Max. packet size */
#define USB_MSC_IF0_ALT0_EP1_bInterval		0x00		/* Interval for polling endpoint for data transfer */

/* interface 0 alternate setting 0 endpoint 2 descriptor */
#define USB_MSC_IF0_ALT0_EP2_bLength		USB_DESC_LEN_ENDPOINT
/* Size of this descriptor */
#define USB_MSC_IF0_ALT0_EP2_bDescriptorType	USB_DESC_TYPE_ENDPOINT
/* Endpoint descriptor type  */
#define USB_MSC_IF0_ALT0_EP2_bEndpointAddress	(USB_EP_OUT | 0x02)	/* Endpoint address */
#define USB_MSC_IF0_ALT0_EP2_bmAttributes	USB_EP_ATTR_BULK	/* Endpoint attributes */
#define USB_MSC_IF0_ALT0_EP2_wMaxPacketSize	0x0040		/* Max. packet size */
#define USB_MSC_IF0_ALT0_EP2_bInterval		0x00		/* Interval for polling endpoint for data transfer */

/* configuration descriptor */
#define USB_MSC_CFG_bLength			USB_DESC_LEN_CONFIGURATION
/* Size of this descriptor */
#define USB_MSC_CFG_bDescriptorType		USB_DESC_TYPE_CONFIGURATION
/* Configuration descriptor type */
#define USB_MSC_CFG_wTotalLength		USB_MSC_CFG_bLength + \
						USB_MSC_IF0_ALT0_bLength + \
						USB_MSC_IF0_ALT0_EP1_bLength + \
						USB_MSC_IF0_ALT0_EP2_bLength
/* Total length of this configuration */
#define USB_MSC_CFG_bNumInterfaces		0x01		/* Number of interfaces */
#define USB_MSC_CFG_bConfigurationValue		0x01		/* Value to select this configuration */
#define USB_MSC_CFG_iConfiguration		0x00		/* Index of string descriptor of this configuration */
#define USB_MSC_CFG_bmAttributes		0xc0		/* Configuration charateristics */
#define USB_MSC_CFG_MaxPower			0xfa		/* Max. power consumption */

/* other speed configuration descriptor - for USB rev. 2.0 only */
#define USB_MSC_OTHER_SPEED_CFG_bLength		USB_DESC_LEN_OTHER_SPEED_CONFIGURATION
/* Size of this descriptor */
#define USB_MSC_OTHER_SPEED_CFG_bDescriptorType USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION
/* Configuration descriptor type */
#define USB_MSC_OTHER_SPEED_CFG_wTotalLength	USB_MSC_CFG_bLength + \
						USB_MSC_IF0_ALT0_bLength + \
						USB_MSC_IF0_ALT0_EP1_bLength + \
						USB_MSC_IF0_ALT0_EP2_bLength
/* Total length of this configuration */
#define USB_MSC_OTHER_SPEED_CFG_bNumInterfaces	0x01		/* Number of interfaces */
#define USB_MSC_OTHER_SPEED_CFG_bConfigurationValue	0x01	/* Value to select this configuration */
#define USB_MSC_OTHER_SPEED_CFG_iConfiguration	0x00		/* Index of string descriptor of this configuration */
#define USB_MSC_OTHER_SPEED_CFG_bmAttributes	0xc0		/* Configuration charateristics */
#define USB_MSC_OTHER_SPEED_CFG_MaxPower	0xfa		/* Max. power consumption */

/*---------------- video class ----------------*/

/* Structure */

/* configuration */
typedef __packed struct _USB_VC_CONFIGURATION_DESC
{
    USB_CFG_DESC	cfg;
    USB_IF_DESC	if0Alt0;
    USB_EP_DESC	if0Alt0Ep4;
    USB_IF_DESC	if0Alt1;
    USB_EP_DESC	if0Alt1Ep4;
    USB_IF_DESC	if0Alt2;
    USB_EP_DESC	if0Alt2Ep4;
    USB_IF_DESC	if0Alt3;
    USB_EP_DESC	if0Alt3Ep4;
    USB_IF_DESC	if0Alt4;
    USB_EP_DESC	if0Alt4Ep4;
}
USB_VC_CONFIGURATION_DESC;

/* Constant */

/* device descriptor */
#define USB_VC_DEV_bLength			USB_DESC_LEN_DEVICE
/* Size of this descriptor */
#define USB_VC_DEV_bDescriptorType		USB_DESC_TYPE_DEVICE
/* Device descriptor type */
#define USB_VC_DEV_bcdUSB			0x0110		/* USB Specification Release Number in BCD */
#define USB_VC_DEV_bDeviceClass			0x00		/* Class code */
#define USB_VC_DEV_bDeviceSubClass		0x00		/* Subclass code */
#define USB_VC_DEV_bDeviceProtocol		0x00		/* Protocol code */
#define USB_VC_DEV_bMaxPacketSize0		0x40		/* Max. packet size for endpoint zero */
#define USB_VC_DEV_idVendor			USB_VC_DEV_VENDOR_ID
/* Vendor ID */
#define USB_VC_DEV_idProduct			USB_VC_DEV_PRODUCT_ID
/* Product ID */
#define USB_VC_DEV_bcdDevice			0x0000		/* Device release number in BCD */
#define USB_VC_DEV_iManufacturer		0x01		/* Index of string descriptor of manufacturer */
#define USB_VC_DEV_iProduct			0x02		/* Index of string descriptor of product */
#define USB_VC_DEV_iSerialNumber		0x03		/* Index of string descriptor of serial number */
#define USB_VC_DEV_bNumConfigurations		0x01		/* Number of configurations */

/* device qualifier descriptor - for USB rev. 2.0 only */
#define USB_VC_DEV_QUAL_bLength			USB_DESC_LEN_DEVICE_QUALIFIER
/* Size of this descriptor */
#define USB_VC_DEV_QUAL_bDescriptorType		USB_DESC_TYPE_DEVICE_QUALIFIER
/* Device qualifier descriptor type */
#define USB_VC_DEV_QUAL_bcdUSB			0x0200		/* USB specification version number */
#define USB_VC_DEV_QUAL_bDeviceClass		0x00		/* Class code */
#define USB_VC_DEV_QUAL_bDeviceSubClass		0x00		/* Subclass code */
#define USB_VC_DEV_QUAL_bDeviceProtocol		0x00		/* Protocol code */
#define USB_VC_DEV_QUAL_bMaxPacketSize0		0x40		/* Max. packet size for endpoint zero */
#define USB_VC_DEV_QUAL_bNumConfigurations	0x01		/* Number of other-speed configurations */
#define USB_VC_DEV_QUAL_bReserved		0x00		/* Reserved */

/* interface 0 alternate setting 0 descriptor */
#define USB_VC_IF0_ALT0_bLength			USB_DESC_LEN_INTERFACE
/* Size of this descriptor */
#define USB_VC_IF0_ALT0_bDescriptorType		USB_DESC_TYPE_INTERFACE
/* Interface descriptor type */
#define USB_VC_IF0_ALT0_bInterfaceNumber	0x00		/* Interface number */
#define USB_VC_IF0_ALT0_bAlternateSetting	0x00		/* Alternate setting */
#define USB_VC_IF0_ALT0_bNumEndpoints		0x01		/* Number of endpoints */
#define USB_VC_IF0_ALT0_bInterfaceClass		USB_VC_CLASS_VENDOR
/* Class code - vendor-specific class */
#define USB_VC_IF0_ALT0_bInterfaceSubClass	USB_VC_SUBCLASS_VENDOR
/* Subclass code */
#define USB_VC_IF0_ALT0_bInterfaceProtocol	USB_VC_PROTOCOL_VENDOR
/* Protocol code */
#define USB_VC_IF0_ALT0_iInterface		0x00		/* Index of string descriptor of this interface */

/* interface 0 alternate setting 0 endpoint 4 descriptor */
#define USB_VC_IF0_ALT0_EP4_bLength		USB_DESC_LEN_ENDPOINT
/* Size of this descriptor */
#define USB_VC_IF0_ALT0_EP4_bDescriptorType	USB_DESC_TYPE_ENDPOINT
/* Endpoint descriptor type  */
#define USB_VC_IF0_ALT0_EP4_bEndpointAddress	(USB_EP_IN | 0x04)	/* Endpoint address */
#define USB_VC_IF0_ALT0_EP4_bmAttributes	USB_EP_ATTR_ISO	/* Endpoint attributes */
#define USB_VC_IF0_ALT0_EP4_wMaxPacketSize	0x0000		/* Max. packet size */
#define USB_VC_IF0_ALT0_EP4_bInterval		0x00		/* Interval for polling endpoint for data transfer */

/* interface 0 alternate setting 1 descriptor */
#define USB_VC_IF0_ALT1_bLength			USB_DESC_LEN_INTERFACE
/* Size of this descriptor */
#define USB_VC_IF0_ALT1_bDescriptorType		USB_DESC_TYPE_INTERFACE
/* Interface descriptor type */
#define USB_VC_IF0_ALT1_bInterfaceNumber	0x00		/* Interface number */
#define USB_VC_IF0_ALT1_bAlternateSetting	0x01		/* Alternate setting */
#define USB_VC_IF0_ALT1_bNumEndpoints		0x01		/* Number of endpoints */
#define USB_VC_IF0_ALT1_bInterfaceClass		USB_VC_CLASS_VENDOR
/* Class code - vendor-specific class */
#define USB_VC_IF0_ALT1_bInterfaceSubClass	USB_VC_SUBCLASS_VENDOR
/* Subclass code */
#define USB_VC_IF0_ALT1_bInterfaceProtocol	USB_VC_PROTOCOL_VENDOR
/* Protocol code */
#define USB_VC_IF0_ALT1_iInterface		0x00		/* Index of string descriptor of this interface */

/* interface 0 alternate setting 1 endpoint 4 descriptor */
#define USB_VC_IF0_ALT1_EP4_bLength		USB_DESC_LEN_ENDPOINT
/* Size of this descriptor */
#define USB_VC_IF0_ALT1_EP4_bDescriptorType	USB_DESC_TYPE_ENDPOINT
/* Endpoint descriptor type  */
#define USB_VC_IF0_ALT1_EP4_bEndpointAddress	(USB_EP_IN | 0x04)	/* Endpoint address */
#define USB_VC_IF0_ALT1_EP4_bmAttributes	USB_EP_ATTR_ISO	/* Endpoint attributes */
#define USB_VC_IF0_ALT1_EP4_wMaxPacketSize	0x0140		/* Max. packet size */
#define USB_VC_IF0_ALT1_EP4_bInterval		0x00		/* Interval for polling endpoint for data transfer */

/* interface 0 alternate setting 2 descriptor */
#define USB_VC_IF0_ALT2_bLength			USB_DESC_LEN_INTERFACE
/* Size of this descriptor */
#define USB_VC_IF0_ALT2_bDescriptorType		USB_DESC_TYPE_INTERFACE
/* Interface descriptor type */
#define USB_VC_IF0_ALT2_bInterfaceNumber	0x00		/* Interface number */
#define USB_VC_IF0_ALT2_bAlternateSetting	0x02		/* Alternate setting */
#define USB_VC_IF0_ALT2_bNumEndpoints		0x01		/* Number of endpoints */
#define USB_VC_IF0_ALT2_bInterfaceClass		USB_VC_CLASS_VENDOR
/* Class code - vendor-specific class */
#define USB_VC_IF0_ALT2_bInterfaceSubClass	USB_VC_SUBCLASS_VENDOR
/* Subclass code */
#define USB_VC_IF0_ALT2_bInterfaceProtocol	USB_VC_PROTOCOL_VENDOR
/* Protocol code */
#define USB_VC_IF0_ALT2_iInterface		0x00		/* Index of string descriptor of this interface */

/* interface 0 alternate setting 2 endpoint 4 descriptor */
#define USB_VC_IF0_ALT2_EP4_bLength		USB_DESC_LEN_ENDPOINT
/* Size of this descriptor */
#define USB_VC_IF0_ALT2_EP4_bDescriptorType	USB_DESC_TYPE_ENDPOINT
/* Endpoint descriptor type  */
#define USB_VC_IF0_ALT2_EP4_bEndpointAddress	(USB_EP_IN | 0x04)	/* Endpoint address */
#define USB_VC_IF0_ALT2_EP4_bmAttributes	USB_EP_ATTR_ISO	/* Endpoint attributes */
#define USB_VC_IF0_ALT2_EP4_wMaxPacketSize	0x0280		/* Max. packet size */
#define USB_VC_IF0_ALT2_EP4_bInterval		0x00		/* Interval for polling endpoint for data transfer */

/* interface 0 alternate setting 3 descriptor */
#define USB_VC_IF0_ALT3_bLength			USB_DESC_LEN_INTERFACE
/* Size of this descriptor */
#define USB_VC_IF0_ALT3_bDescriptorType		USB_DESC_TYPE_INTERFACE
/* Interface descriptor type */
#define USB_VC_IF0_ALT3_bInterfaceNumber	0x00		/* Interface number */
#define USB_VC_IF0_ALT3_bAlternateSetting	0x03		/* Alternate setting */
#define USB_VC_IF0_ALT3_bNumEndpoints		0x01		/* Number of endpoints */
#define USB_VC_IF0_ALT3_bInterfaceClass		USB_VC_CLASS_VENDOR
/* Class code - vendor-specific class */
#define USB_VC_IF0_ALT3_bInterfaceSubClass	USB_VC_SUBCLASS_VENDOR
/* Subclass code */
#define USB_VC_IF0_ALT3_bInterfaceProtocol	USB_VC_PROTOCOL_VENDOR
/* Protocol code */
#define USB_VC_IF0_ALT3_iInterface		0x00		/* Index of string descriptor of this interface */

/* interface 0 alternate setting 3 endpoint 4 descriptor */
#define USB_VC_IF0_ALT3_EP4_bLength		USB_DESC_LEN_ENDPOINT
/* Size of this descriptor */
#define USB_VC_IF0_ALT3_EP4_bDescriptorType	USB_DESC_TYPE_ENDPOINT
/* Endpoint descriptor type  */
#define USB_VC_IF0_ALT3_EP4_bEndpointAddress	(USB_EP_IN | 0x04)	/* Endpoint address */
#define USB_VC_IF0_ALT3_EP4_bmAttributes	USB_EP_ATTR_ISO	/* Endpoint attributes */
#define USB_VC_IF0_ALT3_EP4_wMaxPacketSize	0x0300		/* Max. packet size */
#define USB_VC_IF0_ALT3_EP4_bInterval		0x00		/* Interval for polling endpoint for data transfer */

/* interface 0 alternate setting 4 descriptor */
#define USB_VC_IF0_ALT4_bLength			USB_DESC_LEN_INTERFACE
/* Size of this descriptor */
#define USB_VC_IF0_ALT4_bDescriptorType		USB_DESC_TYPE_INTERFACE
/* Interface descriptor type */
#define USB_VC_IF0_ALT4_bInterfaceNumber	0x00		/* Interface number */
#define USB_VC_IF0_ALT4_bAlternateSetting	0x04		/* Alternate setting */
#define USB_VC_IF0_ALT4_bNumEndpoints		0x01		/* Number of endpoints */
#define USB_VC_IF0_ALT4_bInterfaceClass		USB_VC_CLASS_VENDOR
/* Class code - vendor-specific class */
#define USB_VC_IF0_ALT4_bInterfaceSubClass	USB_VC_SUBCLASS_VENDOR
/* Subclass code */
#define USB_VC_IF0_ALT4_bInterfaceProtocol	USB_VC_PROTOCOL_VENDOR
/* Protocol code */
#define USB_VC_IF0_ALT4_iInterface		0x00		/* Index of string descriptor of this interface */

/* interface 0 alternate setting 4 endpoint 4 descriptor */
#define USB_VC_IF0_ALT4_EP4_bLength		0x07		/* Size of this descriptor */
#define USB_VC_IF0_ALT4_EP4_bDescriptorType	0x05		/* Endpoint descriptor type  */
#define USB_VC_IF0_ALT4_EP4_bEndpointAddress	(USB_EP_IN | 0x04)	/* Endpoint address */
#define USB_VC_IF0_ALT4_EP4_bmAttributes	USB_EP_ATTR_ISO	/* Endpoint attributes */
#define USB_VC_IF0_ALT4_EP4_wMaxPacketSize	0x03c0		/* Max. packet size */
#define USB_VC_IF0_ALT4_EP4_bInterval		0x00		/* Interval for polling endpoint for data transfer */

/* configuration descriptor */
#define USB_VC_CFG_bLength			0x09		/* Size of this descriptor */
#define USB_VC_CFG_bDescriptorType		0x02		/* Configuration descriptor type */
#define USB_VC_CFG_wTotalLength			USB_VC_CFG_bLength + \
						USB_VC_IF0_ALT0_bLength + \
						USB_VC_IF0_ALT0_EP4_bLength + \
						USB_VC_IF0_ALT1_bLength + \
						USB_VC_IF0_ALT1_EP4_bLength + \
						USB_VC_IF0_ALT2_bLength + \
						USB_VC_IF0_ALT2_EP4_bLength + \
						USB_VC_IF0_ALT3_bLength + \
						USB_VC_IF0_ALT3_EP4_bLength + \
						USB_VC_IF0_ALT4_bLength + \
						USB_VC_IF0_ALT4_EP4_bLength
/* Total length of this configuration */
#define USB_VC_CFG_bNumInterfaces		0x01		/* Number of interfaces */
#define USB_VC_CFG_bConfigurationValue		0x01		/* Value to select this configuration */
#define USB_VC_CFG_iConfiguration		0x00		/* Index of string descriptor of this configuration */
#define USB_VC_CFG_bmAttributes			0xc0		/* Configuration charateristics */
#define USB_VC_CFG_MaxPower			0xfa		/* Max. power consumption */

/* other speed configuration descriptor - for USB rev. 2.0 only */
#define USB_VC_OTHER_SPEED_CFG_bLength		0x09		/* Size of this descriptor */
#define USB_VC_OTHER_SPEED_CFG_bDescriptorType 	0x07		/* Configuration descriptor type */
#define USB_VC_OTHER_SPEED_CFG_wTotalLength	USB_VC_CFG_bLength + \
						USB_VC_IF0_ALT0_bLength + \
						USB_VC_IF0_ALT0_EP4_bLength + \
						USB_VC_IF0_ALT1_bLength + \
						USB_VC_IF0_ALT1_EP4_bLength + \
						USB_VC_IF0_ALT2_bLength + \
						USB_VC_IF0_ALT2_EP4_bLength + \
						USB_VC_IF0_ALT3_bLength + \
						USB_VC_IF0_ALT3_EP4_bLength + \
						USB_VC_IF0_ALT4_bLength + \
						USB_VC_IF0_ALT4_EP4_bLength
/* Total length of this configuration */
#define USB_VC_OTHER_SPEED_CFG_bNumInterfaces	0x01		/* Number of interfaces */
#define USB_VC_OTHER_SPEED_CFG_bConfigurationValue	0x01	/* Value to select this configuration */
#define USB_VC_OTHER_SPEED_CFG_iConfiguration	0x00		/* Index of string descriptor of this configuration */
#define USB_VC_OTHER_SPEED_CFG_bmAttributes	0xc0		/* Configuration charateristics */
#define USB_VC_OTHER_SPEED_CFG_MaxPower		0xfa		/* Max. power consumption */

/*-------- string descriptor --------*/

/* language id */
#define USB_STR0_bLength			0x04
#define USB_STR0_bDescriptorType		USB_DESC_TYPE_STRING
#define USB_STR0_wLANGID			0x09, 0x04

/* string of manufacturer */
#define USB_STR1_bLength			0x0a
#define USB_STR1_bDescriptorType		USB_DESC_TYPE_STRING
#define USB_STR1_bString			'M', 0x00, 'A', 0x00, 'R', 0x00, 'S', 0x00
/* string of product */
#define USB_STR2_bLength			0x0e
#define USB_STR2_bDescriptorType		USB_DESC_TYPE_STRING
#if (CHIP_OPTION == CHIP_PA9001D)
#define USB_STR2_bString			'M', 0x00, 'R', 0x00, '9', 0x00, '6', 0x00, '1', 0x00, '0', 0x00
#else
#define USB_STR2_bString			'M', 0x00, 'R', 0x00, '6', 0x00, '7', 0x00, '2', 0x00, '0', 0x00
#endif
/* string of serial number */
#define USB_STR3_bLength			0x0a
#define USB_STR3_bDescriptorType		USB_DESC_TYPE_STRING
#define USB_STR3_bString			'0', 0x00, '1', 0x00, '0', 0x00, '0', 0x00

#endif
