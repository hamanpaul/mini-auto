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

#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

#define USB_DEVICE_MAX_INT_EVT		32

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

typedef struct _USB_INT_EVT
{
    u8		cause[USB_DEVICE_MAX_INT_EVT];   	/* cause of interrupt event */
    u8	  	idxSet;                         /* index of set event */
    u8	  	idxGet;                         /* index of get event */
}
USB_INT_EVT;

//Device descriptor
typedef __packed struct _USB_DEVICE_DES
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
USB_DEVICE_DES;

//Configuration descriptor
typedef __packed struct _USB_CONFIGURATION_DES
{
    u8	    bLength;
    u8      bDescriptorType;
    u16     wTotalLength;
    u8	    bNumInterface;
    u8	    bConfigValue;
    u8	    iConfiguration;
    u8	    bmAttribute;
    u8     bMaxPower;
}
USB_CONFIGURATION_DES;

typedef __packed struct _USB_INTERFACE_DES
{
    u8	    bLength;
    u8      bDescriptorType;
    u8      bInterfaceNum;
    u8	    bAlternateSetting;
    u8	    bNumEndpoints;
    u8	    bInterfaceClass;
    u8	    bInterfaceSubClass;
    u8      bInterfaceProtocol;
    u8      iInterface;
}
USB_INTERFACE_DES;

typedef __packed struct _USB_ENDPOINT_DES
{
    u8	    bLength;
    u8      bDescriptorType;
    u8      bEndpointAddress;
    u8	    bmAttribute;
    u16	    wMaxPacketSize;
    u8      bInterval;
}
USB_ENDPOINT_DES;

typedef __packed struct _USB_LANGUAGE_DES
{
    u8	    bLength;
    u8      bDescriptorType;
    u16     wLangID;
}
USB_LANGUAGE_DES;

typedef __packed struct _USB_PRODUCT_DES
{
    u8	    bLength;
    u8      bDescriptorType;
    u32     Content0;
    u32     Content1;
    u32     Content2;
    u32     Content3;
    u32     Content4;
    u32     Content5;
}
USB_PRODUCT_DES;

typedef __packed struct _USB_SERIAL_DES
{
    u8	    bLength;
    u8      bDescriptorType;
    u32     Content0;
    u32     Content1;
    u32     Content2;
    u32     Content3;
    u32     Content4;
    u32     Content5;
    u32     Content6;
    u32     Content7;
    u32     Content8;
    u32     Content9;
    u32     Content10;
    u32     Content11;
}
USB_SERIAL_DES;

typedef __packed struct _USB_INQUIRY_RESPONSE
{
    u8	    DeviceType;
    u8      Removable;
    u8      Version;
    u8      Response_Format;
    u8      Additional_Length;
    u8      Reserve_0;
    u8      Reserve_1;
    u8      Reserve_2;
    u32     Vendor_Info0;
    u32     Vendor_Info1;
    u32     Product_Info0;
    u32     Product_Info1;
    u32     Product_Info2;
    u32     Product_Info3;
    u32     Product_Revision;
}
USB_INQUIRY_RESPONSE;

typedef __packed struct _USB_FORMAT_CAPACITY_RESPONSE
{
    u8      Reserve_0;
    u8      Reserve_1;
    u8      Reserve_2;
    u8      List_Length;
    u32     Num_Blocks;
    u8      Descriptor_Code;
    u8      Block_Len_0;
    u8      Block_Len_1;
    u8      Block_Len_2;
}
USB_FORMAT_CAPACITY_RESPONSE;

typedef __packed struct _USB_CAPACITY_RESPONSE
{
    u32     Logical_BLK_Address;
    u32     Block_Length;
}
USB_CAPACITY_RESPONSE;

typedef __packed struct _USB_MODE_SENSE_RESPONSE
{
    u16     Mode_Data_Len ;
    u8      Media_Type ;
    u8      WP;
    u8      Page_Code ;
    u8      Page_Length ;
    u16     Tansfer_Rate ;
    u8      Heads_Num ;
    u8      Sec_per_Track ;
    u16     Data_Bytes_per_Sec ;
    u16     Num_Cylinders ;
    u16     Reserve_0;
    u32     Reserve_1;
    u32     Reserve_2;
    u32     Reserve_3;
    u32     Reserve_4;
    u32     Reserve_5;
}
USB_MODE_SENSE_RESPONSE;

typedef __packed struct _USB_MSC_STATUS
{
    u32	    Signature;
    u32     Tag;
    u32     Data_Residue;
    u8      Status;
    
}
USB_MSC_STATUS;


enum
{
    USB_DEVICE_MSC_EVT_BULK_OUT = 0x00,	        // 0x00 - MSC init 
    USB_DEVICE_MSC_EVT_BULK_IN,		        // 0x01 - Read 10
    USB_DEVICE_MSC_EVT_UNDEF		            // 0x19 and larger - undefined event 
};


#endif
