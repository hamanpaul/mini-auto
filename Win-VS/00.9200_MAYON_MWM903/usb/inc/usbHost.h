/*

Copyright (c) 2008 Mars Semiconductor Corp.
Module Name:

	usbdev.h

Abstract:

   	The declarations of USB device.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_HOST_H__
#define __USB_HOST_H__

#define BIT0    0x00000001
#define BIT1    0x00000002
#define BIT2    0x00000004
#define BIT3    0x00000008
#define BIT4    0x00000010
#define BIT5    0x00000020
#define BIT6    0x00000040
#define BIT7    0x00000080
#define BIT8    0x00000100
#define BIT9    0x00000200
#define BIT10   0x00000400
#define BIT11   0x00000800
#define BIT12   0x00001000
#define BIT13   0x00002000
#define BIT14   0x00004000
#define BIT15   0x00008000
#define BIT16   0x00010000
#define BIT17   0x00020000
#define BIT18   0x00040000
#define BIT19   0x00080000
#define BIT20   0x00100000
#define BIT21   0x00200000
#define BIT22   0x00400000
#define BIT23   0x00800000
#define BIT24   0x01000000
#define BIT25   0x02000000
#define BIT26   0x04000000
#define BIT27   0x08000000
#define BIT28   0x10000000
#define BIT29   0x20000000
#define BIT30   0x40000000
#define BIT31   0x80000000

/*-------- Constant --------*/
#define USB_TIMEOUT		100

/*---- Interface number ----*/
#define USB_INTERFACE_MAX		0x02

/*---- Endpoint index ----*/
#define USB_HOST_EP_CTRLIN			0x00
#define USB_HOST_EP_CTRLOUT			0x01
#define USB_HOST_EP_BULKIN			0x02
#define USB_HOST_EP_BULKOUT			0x03
#define USB_HOST_EP_INTRIN			0x04
#define USB_HOST_EP_ISOIN			0x05
#define USB_HOST_EP_MAX			0x06

/*---- Endpoint number ----*/
#define USB_HOST_ENDPOINT_CTRL		0x00
#define USB_HOST_ENDPOINT_BULKIN		0x01
#define USB_HOST_ENDPOINT_BULKOUT		0x02
#define USB_HOST_ENDPOINT_INTRIN		0x03
#define USB_HOST_ENDPOINT_ISOIN		0x04
#define USB_HOST_ENDPOINT_MAX		0x05

/*---- Semaphore index ----*/
#define USB_HOST_SEM_EP_CTRLIN			0x00
#define USB_HOST_SEM_EP_CTRLOUT			0x01
#define USB_HOST_SEM_EP_BULKIN			0x02
#define USB_HOST_SEM_EP_BULKOUT			0x03
#define USB_HOST_SEM_EP_INTRIN			0x04
#define USB_HOST_SEM_EP_ISOIN			0x05
#define USB_HOST_SEM_EP				0x06
#define USB_HOST_SEM_EP_MAX				0x07

/*---- CLASS DEFINE ----*/
#define MASS_STORAGE                    0x08

/*---- CONTROL STAGE ----*/
#define SETUP_STAGE                     0x01
#define CONTROL_IN_STAGE                0x02
#define CONTROL_OUT_STAGE               0x03
#define CONTROL_DONE_STAGE              0x03

/*---- TRANSACTION STAGE ----*/
#define	COMMAND_STAGE					0x00
#define	DATA_STAGE						0x01
#define	STATUS_STAGE					0x02

/*---- bRequest COMMAND ----*/
#define HOST_SET_ADDRESS                0x05
#define HOST_GET_DSCRIPTOR              0x06
#define HOST_SET_CONFIGURATION          0x09
#define HOST_GET_MAX_LUN                 0xFE

/*---- Descriptor Types ----*/
#define DEVICE_DSCRIPTOR                0x0100
#define CONFIGURATION_DSCRIPTOR        0x0200
#define STRING_LANG_DSCRIPTOR          0x0300
#define INTERFACE_DSCRIPTOR            0x0400

/*---- Get Configuration Descriptor Types ----*/
#define ONLY_CONFIGURATION_DESC         0x01
#define ALL_CONFIGURATION_DESC          0x02

/* Enumeration Stage */
#define STAGE_GET_DEVECE_DESCRIPTOR		0x01
#define STAGE_SET_DEVECE_ADDRESS		0x02
#define STAGE_GET_DEVECE_DESCRIPTOR_2	0x03
#define STAGE_GET_CONFIG_DESCRIPTOR		0x04
#define STAGE_GET_LANGUAGE_DESCRIPTOR	0x05
#define STAGE_GET_CONFIG_DESCRIPTOR_ALL	0x06
#define STAGE_SET_CONFIGURATION			0x07
#define STAGE_GET_MAX_LUN				0x08
#define STAGE_ENUMERATION_DONE			0x09
#define STAGE_SET_CLEAR_FEATURE			0x0A


typedef struct _USB_HOST_SETTING
{
    u16		devAddr;			/* device address */
    u8		numCfg;				/* number of configuration */
    u8		numIf;				/* number of interface */
    u8		numEp;				/* number of endpoint */
    u8		cfgNum;				/* configuration number */
    u8   	ifNum;              /* interface number */
    u16		devStat;			/* device status */
    u16		ifStat;				/* interface status */
    u16		epStat[USB_HOST_ENDPOINT_MAX];	/* endpoint status */
}
USB_HOST_SETTING;

/* Interrupt event queue to signal usbTask() */
#define USB_MAX_INT_EVT		32			/* max. interrupt event queued */
typedef struct _USB_INT_EVT
{
    u8		cause[USB_MAX_INT_EVT];   	/* cause of interrupt event */
    u8	  	idxSet;                         /* index of set event */
    u8	  	idxGet;                         /* index of get event */
}
USB_INT_EVT;

/* API event queue to sigal usbTask() */
#define USB_MAX_API_EVT		32      		/* max. api event queued */
typedef struct _USB_HOST_API_EVT
{
    u8		cause[USB_MAX_API_EVT];		/* cause of api event */
    u8		epNum[USB_MAX_API_EVT];		/* endpoint number */
    u8		option[USB_MAX_API_EVT];	/* option */
    u8   		idxSet;                     	/* index of set event */
    u8		idxGet;                     	/* index of get event */
}
USB_HOST_API_EVT;

typedef struct _USB_API_EVT_SNAP
{
    u8		cause;				/* cause of api event */
    u8		epNum;				/* endpoint number */
    u8		option;				/* option */
}
USB_API_EVT_SNAP;

typedef __packed struct _USB_HOST_SETUP_REQ
{
    u8	    bmRequestType;
    u8      bRequest;
    u16     wValue;
    u16	    wIndex;
    u16	    wLength;
}
USB_HOST_SETUP_REQ;

//Device descriptor
typedef __packed struct _USB_HOST_DEV_DES
{
    u8	    bLength;
    u8      bDescriptorType;
    u16     bcdUSB;
    u8	    bDeviceClass;
    u8	    bDeviceSubClass;
    u8	    bDeviceProtocol;
    u8	    bMaxPacketSize0;
    u16     idVendor;
    u16     idProduct;
    u16     bcdDevice;
    u8      iManufacturer;
    u8      iProduct;
    u8      iSerialNumber;
    u8      bNumConfiguration;
}
USB_HOST_DEV_DES;

//Configuration descriptor
typedef __packed struct _USB_HOST_CONFIG_DES
{
    u8	    bLength;
    u8      bDescriptorType;
    u16     wTotalLength;
    u8      bNumInterface;
    u8      bConfigurationValue;
    u8      iConfiguration;
    u8      bmAttributes;
    u8      bMaxPower;
}
USB_HOST_CONFIG_DES;

//Interface descriptor
typedef __packed struct _USB_HOST_INF_DES
{
    u8	    bLength;
    u8      bDescriptorType;
    u8      bInterfaceNumber;    
    u8      bAlternateSetting;
    u8      bNumEndpoints;
    u8      bInterfaceClass;
    u8      bInterfaceSubClass;
    u8      bInterfaceProtocol;
    u8      iInterface;
}
USB_HOST_INF_DES;

//Endpoint descriptor
typedef __packed struct _USB_HOST_ENDPOINT_DES
{
    u8	    bLength;
    u8      bDescriptorType;
    u8      bEndpointAddress;
    u8      bmAttributes;
    u16     wMaxPacketSize;
    u8      bInterval;
}
USB_HOST_ENDPOINT_DES;

#endif
