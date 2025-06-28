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
#if 0
/*---------------- Mass Storage Class ----------------*/

/*-------- Device Descriptor --------*/
__align(4) USB_DEV_DESC usb_msc_dev_desc =
    {
        USB_MSC_DEV_bLength,			/* Size of this descriptor */
        USB_MSC_DEV_bDescriptorType,		/* Device descriptor type */
        USB_MSC_DEV_bcdUSB,			/* USB Specification Release Number in BCD */
        USB_MSC_DEV_bDeviceClass,		/* Class code */
        USB_MSC_DEV_bDeviceSubClass,		/* Subclass code */
        USB_MSC_DEV_bDeviceProtocol,		/* Protocol code */
        USB_MSC_DEV_bMaxPacketSize0,		/* Max. packet size for endpoint zero */
        USB_MSC_DEV_idVendor,			/* Vendor ID */
        USB_MSC_DEV_idProduct,			/* Product ID */
        USB_MSC_DEV_bcdDevice,			/* Device release number in BCD */
        USB_MSC_DEV_iManufacturer,		/* Index of string descriptor of manufacturer */
        USB_MSC_DEV_iProduct,			/* Index of string descriptor of product */
        USB_MSC_DEV_iSerialNumber,		/* Index of string descriptor of serial number */
        USB_MSC_DEV_bNumConfigurations,		/* Number of configurations */
    };

/*-------- Device Qualifier Descriptor --------*/
__align(4) USB_DEV_QUAL_DESC usb_msc_dev_qual_desc =
    {
        USB_MSC_DEV_QUAL_bLength,		/* Size of this descriptor */
        USB_MSC_DEV_QUAL_bDescriptorType,	/* Device qualifier descriptor type */
        USB_MSC_DEV_QUAL_bcdUSB,		/* USB specification version number */
        USB_MSC_DEV_QUAL_bDeviceClass,		/* Class code */
        USB_MSC_DEV_QUAL_bDeviceSubClass,	/* Subclass code */
        USB_MSC_DEV_QUAL_bDeviceProtocol,	/* Protocol code */
        USB_MSC_DEV_QUAL_bMaxPacketSize0,	/* Max. packet size for endpoint zero */
        USB_MSC_DEV_QUAL_bNumConfigurations,	/* Number of other-speed configurations */
        USB_MSC_DEV_QUAL_bReserved,		/* Reserved */
    };

/*-------- Configuration descriptor + Interface descriptor + Endpoint descriptor --------*/
__align(4) USB_MSC_CONFIGURATION_DESC usb_msc_configuration_desc =
    {
        /* cfg */
        {
            USB_MSC_CFG_bLength,			/* Size of this descriptor */
            USB_MSC_CFG_bDescriptorType,		/* Configuration descriptor type */
            USB_MSC_CFG_wTotalLength,		/* Total length of this configuration */
            USB_MSC_CFG_bNumInterfaces,		/* Number of interfaces */
            USB_MSC_CFG_bConfigurationValue,	/* Value to select this configuration */
            USB_MSC_CFG_iConfiguration,		/* Index of string descriptor of this configuration */
            USB_MSC_CFG_bmAttributes,		/* Configuration charateristics */
            USB_MSC_CFG_MaxPower,			/* Max. power consumption */
        },
        /* if0Alt0 */
        {
            USB_MSC_IF0_ALT0_bLength,		/* Size of this descriptor */
            USB_MSC_IF0_ALT0_bDescriptorType,	/* Interface descriptor type */
            USB_MSC_IF0_ALT0_bInterfaceNumber,	/* Interface number */
            USB_MSC_IF0_ALT0_bAlternateSetting,	/* Alternate setting */
            USB_MSC_IF0_ALT0_bNumEndpoints,		/* Number of endpoints */
            USB_MSC_IF0_ALT0_bInterfaceClass,	/* Class code - mass storage class */
            USB_MSC_IF0_ALT0_bInterfaceSubClass,	/* Subclass code */
            USB_MSC_IF0_ALT0_bInterfaceProtocol,	/* Protocol code */
            USB_MSC_IF0_ALT0_iInterface,		/* Index of string descriptor of this interface */
        },
        /* if0Alt0Ep1 */
        {
            USB_MSC_IF0_ALT0_EP1_bLength,		/* Size of this descriptor */
            USB_MSC_IF0_ALT0_EP1_bDescriptorType,	/* Endpoint descriptor type  */
            USB_MSC_IF0_ALT0_EP1_bEndpointAddress,	/* Endpoint address */
            USB_MSC_IF0_ALT0_EP1_bmAttributes,	/* Endpoint attributes */
            USB_MSC_IF0_ALT0_EP1_wMaxPacketSize,	/* Max. packet size */
            USB_MSC_IF0_ALT0_EP1_bInterval,		/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt0Ep2 */
        {
            USB_MSC_IF0_ALT0_EP2_bLength,		/* Size of this descriptor */
            USB_MSC_IF0_ALT0_EP2_bDescriptorType,	/* Endpoint descriptor type  */
            USB_MSC_IF0_ALT0_EP2_bEndpointAddress,	/* Endpoint address */
            USB_MSC_IF0_ALT0_EP2_bmAttributes,	/* Endpoint attributes */
            USB_MSC_IF0_ALT0_EP2_wMaxPacketSize,	/* Max. packet size */
            USB_MSC_IF0_ALT0_EP2_bInterval,		/* Interval for polling endpoint for data transfer */
        },
    };

/*---------------- Video Class ----------------*/

/*-------- Device Descriptor --------*/
__align(4) USB_DEV_DESC usb_vc_dev_desc =
    {
        USB_VC_DEV_bLength,			/* Size of this descriptor */
        USB_VC_DEV_bDescriptorType,		/* Device descriptor type */
        USB_VC_DEV_bcdUSB,			/* USB Specification Release Number in BCD */
        USB_VC_DEV_bDeviceClass,		/* Class code */
        USB_VC_DEV_bDeviceSubClass,		/* Subclass code */
        USB_VC_DEV_bDeviceProtocol,		/* Protocol code */
        USB_VC_DEV_bMaxPacketSize0,		/* Max. packet size for endpoint zero */
        USB_VC_DEV_idVendor,			/* Vendor ID */
        USB_VC_DEV_idProduct,			/* Product ID */
        USB_VC_DEV_bcdDevice,			/* Device release number in BCD */
        USB_VC_DEV_iManufacturer,		/* Index of string descriptor of manufacturer */
        USB_VC_DEV_iProduct,			/* Index of string descriptor of product */
        USB_VC_DEV_iSerialNumber,		/* Index of string descriptor of serial number */
        USB_VC_DEV_bNumConfigurations,		/* Number of configurations */
    };

/*-------- Device Qualifier Descriptor --------*/
__align(4) USB_DEV_QUAL_DESC usb_vc_dev_qual_desc =
    {
        USB_VC_DEV_QUAL_bLength,		/* Size of this descriptor */
        USB_VC_DEV_QUAL_bDescriptorType,	/* Device qualifier descriptor type */
        USB_VC_DEV_QUAL_bcdUSB,			/* USB specification version number */
        USB_VC_DEV_QUAL_bDeviceClass,		/* Class code */
        USB_VC_DEV_QUAL_bDeviceSubClass,	/* Subclass code */
        USB_VC_DEV_QUAL_bDeviceProtocol,	/* Protocol code */
        USB_VC_DEV_QUAL_bMaxPacketSize0,	/* Max. packet size for endpoint zero */
        USB_VC_DEV_QUAL_bNumConfigurations,	/* Number of other-speed configurations */
        USB_VC_DEV_QUAL_bReserved,		/* Reserved */
    };

/*-------- Configuration descriptor + Interface descriptor + Endpoint descriptor --------*/
__align(4) USB_VC_CONFIGURATION_DESC usb_vc_configuration_desc =
    {
        /* cfg */
        {
            USB_VC_CFG_bLength,			/* Size of this descriptor */
            USB_VC_CFG_bDescriptorType,		/* Configuration descriptor type */
            USB_VC_CFG_wTotalLength,		/* Total length of this configuration */
            USB_VC_CFG_bNumInterfaces,		/* Number of interfaces */
            USB_VC_CFG_bConfigurationValue,		/* Value to select this configuration */
            USB_VC_CFG_iConfiguration,		/* Index of string descriptor of this configuration */
            USB_VC_CFG_bmAttributes,		/* Configuration charateristics */
            USB_VC_CFG_MaxPower,			/* Max. power consumption */
        },
        /* if0Alt0 */
        {
            USB_VC_IF0_ALT0_bLength,		/* Size of this descriptor */
            USB_VC_IF0_ALT0_bDescriptorType,	/* Interface descriptor type */
            USB_VC_IF0_ALT0_bInterfaceNumber,	/* Interface number */
            USB_VC_IF0_ALT0_bAlternateSetting,	/* Alternate setting */
            USB_VC_IF0_ALT0_bNumEndpoints,		/* Number of endpoints */
            USB_VC_IF0_ALT0_bInterfaceClass,	/* Class code - vendor-specific class */
            USB_VC_IF0_ALT0_bInterfaceSubClass,	/* Subclass code */
            USB_VC_IF0_ALT0_bInterfaceProtocol,	/* Protocol code */
            USB_VC_IF0_ALT0_iInterface,		/* Index of string descriptor of this interface */
        },
        /* if0Alt0Ep4 */
        {
            USB_VC_IF0_ALT0_EP4_bLength,		/* Size of this descriptor */
            USB_VC_IF0_ALT0_EP4_bDescriptorType,	/* Endpoint descriptor type  */
            USB_VC_IF0_ALT0_EP4_bEndpointAddress,	/* Endpoint address */
            USB_VC_IF0_ALT0_EP4_bmAttributes,	/* Endpoint attributes */
            USB_VC_IF0_ALT0_EP4_wMaxPacketSize,	/* Max. packet size */
            USB_VC_IF0_ALT0_EP4_bInterval,		/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt1 */
        {
            USB_VC_IF0_ALT1_bLength,		/* Size of this descriptor */
            USB_VC_IF0_ALT1_bDescriptorType,	/* Interface descriptor type */
            USB_VC_IF0_ALT1_bInterfaceNumber,	/* Interface number */
            USB_VC_IF0_ALT1_bAlternateSetting,	/* Alternate setting */
            USB_VC_IF0_ALT1_bNumEndpoints,		/* Number of endpoints */
            USB_VC_IF0_ALT1_bInterfaceClass,	/* Class code - vendor-specific class */
            USB_VC_IF0_ALT1_bInterfaceSubClass,	/* Subclass code */
            USB_VC_IF0_ALT1_bInterfaceProtocol,	/* Protocol code */
            USB_VC_IF0_ALT1_iInterface,		/* Index of string descriptor of this interface */
        },
        /* if0Alt1Ep4 */
        {
            USB_VC_IF0_ALT1_EP4_bLength,		/* Size of this descriptor */
            USB_VC_IF0_ALT1_EP4_bDescriptorType,	/* Endpoint descriptor type  */
            USB_VC_IF0_ALT1_EP4_bEndpointAddress,	/* Endpoint address */
            USB_VC_IF0_ALT1_EP4_bmAttributes,	/* Endpoint attributes */
            USB_VC_IF0_ALT1_EP4_wMaxPacketSize,	/* Max. packet size */
            USB_VC_IF0_ALT1_EP4_bInterval,		/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt2 */
        {
            USB_VC_IF0_ALT2_bLength,		/* Size of this descriptor */
            USB_VC_IF0_ALT2_bDescriptorType,	/* Interface descriptor type */
            USB_VC_IF0_ALT2_bInterfaceNumber,	/* Interface number */
            USB_VC_IF0_ALT2_bAlternateSetting,	/* Alternate setting */
            USB_VC_IF0_ALT2_bNumEndpoints,		/* Number of endpoints */
            USB_VC_IF0_ALT2_bInterfaceClass,	/* Class code - vendor-specific class */
            USB_VC_IF0_ALT2_bInterfaceSubClass,	/* Subclass code */
            USB_VC_IF0_ALT2_bInterfaceProtocol,	/* Protocol code */
            USB_VC_IF0_ALT2_iInterface,		/* Index of string descriptor of this interface */
        },
        /* if0Alt2Ep4 */
        {
            USB_VC_IF0_ALT2_EP4_bLength,		/* Size of this descriptor */
            USB_VC_IF0_ALT2_EP4_bDescriptorType,	/* Endpoint descriptor type  */
            USB_VC_IF0_ALT2_EP4_bEndpointAddress,	/* Endpoint address */
            USB_VC_IF0_ALT2_EP4_bmAttributes,	/* Endpoint attributes */
            USB_VC_IF0_ALT2_EP4_wMaxPacketSize,	/* Max. packet size */
            USB_VC_IF0_ALT2_EP4_bInterval,		/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt3 */
        {
            USB_VC_IF0_ALT3_bLength,		/* Size of this descriptor */
            USB_VC_IF0_ALT3_bDescriptorType,	/* Interface descriptor type */
            USB_VC_IF0_ALT3_bInterfaceNumber,	/* Interface number */
            USB_VC_IF0_ALT3_bAlternateSetting,	/* Alternate setting */
            USB_VC_IF0_ALT3_bNumEndpoints,		/* Number of endpoints */
            USB_VC_IF0_ALT3_bInterfaceClass,	/* Class code - vendor-specific class */
            USB_VC_IF0_ALT3_bInterfaceSubClass,	/* Subclass code */
            USB_VC_IF0_ALT3_bInterfaceProtocol,	/* Protocol code */
            USB_VC_IF0_ALT3_iInterface,		/* Index of string descriptor of this interface */
        },
        /* if0Alt3Ep4 */
        {
            USB_VC_IF0_ALT3_EP4_bLength,		/* Size of this descriptor */
            USB_VC_IF0_ALT3_EP4_bDescriptorType,	/* Endpoint descriptor type  */
            USB_VC_IF0_ALT3_EP4_bEndpointAddress,	/* Endpoint address */
            USB_VC_IF0_ALT3_EP4_bmAttributes,	/* Endpoint attributes */
            USB_VC_IF0_ALT3_EP4_wMaxPacketSize,	/* Max. packet size */
            USB_VC_IF0_ALT3_EP4_bInterval,		/* Interval for polling endpoint for data transfer */
        },
        /* if0Alt4 */
        {
            USB_VC_IF0_ALT4_bLength,		/* Size of this descriptor */
            USB_VC_IF0_ALT4_bDescriptorType,	/* Interface descriptor type */
            USB_VC_IF0_ALT4_bInterfaceNumber,	/* Interface number */
            USB_VC_IF0_ALT4_bAlternateSetting,	/* Alternate setting */
            USB_VC_IF0_ALT4_bNumEndpoints,		/* Number of endpoints */
            USB_VC_IF0_ALT4_bInterfaceClass,	/* Class code - vendor-specific class */
            USB_VC_IF0_ALT4_bInterfaceSubClass,	/* Subclass code */
            USB_VC_IF0_ALT4_bInterfaceProtocol,	/* Protocol code */
            USB_VC_IF0_ALT4_iInterface,		/* Index of string descriptor of this interface */
        },
        /* if0Alt4Ep4 */
        {
            USB_VC_IF0_ALT4_EP4_bLength,		/* Size of this descriptor */
            USB_VC_IF0_ALT4_EP4_bDescriptorType,	/* Endpoint descriptor type  */
            USB_VC_IF0_ALT4_EP4_bEndpointAddress,	/* Endpoint address */
            USB_VC_IF0_ALT4_EP4_bmAttributes,	/* Endpoint attributes */
            USB_VC_IF0_ALT4_EP4_wMaxPacketSize,	/* Max. packet size */
            USB_VC_IF0_ALT4_EP4_bInterval,		/* Interval for polling endpoint for data transfer */
        },
    };
#endif
/*-------- String descriptor --------*/

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
