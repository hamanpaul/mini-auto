

#ifndef __USBDESCRIPTORS_H__
#define __USBDESCRIPTORS_H__

#include <general.h>

/*
 * communications class types
 *
 * c.f. CDC  USB Class Definitions for Communications Devices
 * c.f. WMCD USB CDC Subclass Specification for Wireless Mobile Communications Devices
 *
 */

#define CLASS_BCD_VERSION		0x0110

/* c.f. CDC 4.1 Table 14 */
#define COMMUNICATIONS_DEVICE_CLASS	0x02

/* c.f. CDC 4.2 Table 15 */
#define COMMUNICATIONS_INTERFACE_CLASS_CONTROL	0x02
#define COMMUNICATIONS_INTERFACE_CLASS_DATA		0x0A
#define COMMUNICATIONS_INTERFACE_CLASS_VENDOR	0x0FF

/* c.f. CDC 4.3 Table 16 */
#define COMMUNICATIONS_NO_SUBCLASS		0x00
#define COMMUNICATIONS_DLCM_SUBCLASS	0x01
#define COMMUNICATIONS_ACM_SUBCLASS		0x02
#define COMMUNICATIONS_TCM_SUBCLASS		0x03
#define COMMUNICATIONS_MCCM_SUBCLASS	0x04
#define COMMUNICATIONS_CCM_SUBCLASS		0x05
#define COMMUNICATIONS_ENCM_SUBCLASS	0x06
#define COMMUNICATIONS_ANCM_SUBCLASS	0x07

/* c.f. WMCD 5.1 */
#define COMMUNICATIONS_WHCM_SUBCLASS	0x08
#define COMMUNICATIONS_DMM_SUBCLASS		0x09
#define COMMUNICATIONS_MDLM_SUBCLASS	0x0a
#define COMMUNICATIONS_OBEX_SUBCLASS	0x0b

/* c.f. CDC 4.4 Table 17 */
#define COMMUNICATIONS_NO_PROTOCOL		0x00
#define COMMUNICATIONS_V25TER_PROTOCOL  0x01	/*Common AT Hayes compatible*/

/* c.f. CDC 4.5 Table 18 */
#define DATA_INTERFACE_CLASS 0x0a

/* c.f. CDC 4.6 No Table */
#define DATA_INTERFACE_SUBCLASS_NONE 0x00	/* No subclass pertinent */

/* c.f. CDC 4.7 Table 19 */
#define DATA_INTERFACE_PROTOCOL_NONE 0x00	/* No class protcol required */


/* c.f. CDC 5.2.3 Table 24 */
#define CS_INTERFACE    0x24
#define CS_ENDPOINT     0x25

/*
 * bDescriptorSubtypes
 *
 * c.f. CDC 5.2.3 Table 25
 * c.f. WMCD 5.3 Table 5.3
 */

#define USB_ST_HEADER   0x00
#define USB_ST_CMF      0x01
#define USB_ST_ACMF     0x02
#define USB_ST_DLMF     0x03
#define USB_ST_TRF      0x04
#define USB_ST_TCLF     0x05
#define USB_ST_UF       0x06
#define USB_ST_CSF      0x07
#define USB_ST_TOMF		0x08
#define USB_ST_USBTF    0x09
#define USB_ST_NCT      0x0a
#define USB_ST_PUF      0x0b
#define USB_ST_EUF      0x0c
#define USB_ST_MCMF     0x0d
#define USB_ST_CCMF     0x0e
#define USB_ST_ENF      0x0f
#define USB_ST_ATMNF    0x10

#define USB_ST_WHCM     0x11
#define USB_ST_MDLM     0x12
#define USB_ST_MDLMD    0x13
#define USB_ST_DMM      0x14
#define USB_ST_OBEX     0x15
#define USB_ST_CS       0x16
#define USB_ST_CSD      0x17
#define USB_ST_TCM      0x18

/* configuration modifiers */
#define BMATTRIBUTE_RESERVED		0x80
#define BMATTRIBUTE_SELF_POWERED	0x40

/*
 *
 * standard usb descriptor structures
 *
 */

/* All standard descriptors have these 2 fields in common */
__packed struct usb_descriptor_header
{
    unsigned char bLength;
    unsigned char bDescriptorType;
} ;

__packed struct usb_endpoint_descriptor
{
    u8  bLength;
    u8  bDescriptorType;	/* 0x5 */
    u8  bEndpointAddress;
    u8  bmAttributes;
    u16 wMaxPacketSize;
    u8  bInterval;
} ;

__packed struct usb_interface_descriptor
{
    u8 bLength;
    u8 bDescriptorType;	/* 0x04 */
    u8 bInterfaceNumber;
    u8 bAlternateSetting;
    u8 bNumEndpoints;
    u8 bInterfaceClass;
    u8 bInterfaceSubClass;
    u8 bInterfaceProtocol;
    u8 iInterface;
} ;

__packed struct usb_configuration_descriptor
{
    u8  bLength;
    u8  bDescriptorType;	/* 0x2 */
    u16 wTotalLength;
    u8  bNumInterfaces;
    u8  bConfigurationValue;
    u8  iConfiguration;
    u8  bmAttributes;
    u8  bMaxPower;
} ;

__packed struct usb_device_descriptor
{
    u8  bLength;
    u8  bDescriptorType;	/* 0x01 */
    u16 bcdUSB;
    u8  bDeviceClass;
    u8  bDeviceSubClass;
    u8  bDeviceProtocol;
    u8  bMaxPacketSize0;
    u16 idVendor;
    u16 idProduct;
    u16 bcdDevice;
    u8  iManufacturer;
    u8  iProduct;
    u8  iSerialNumber;
    u8  bNumConfigurations;
} ;

__packed struct usb_string_descriptor
{
    u8 bLength;
    u8 bDescriptorType;	/* 0x03 */
    u16 wData;
} ;

__packed struct usb_generic_descriptor
{
    u8 bLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
} ;


/*
 * communications class descriptor structures
 *
 * c.f. CDC 5.2 Table 25c
 */

__packed struct usb_class_function_descriptor
{
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
} ;

__packed struct usb_class_function_descriptor_generic
{
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bmCapabilities;
} ;

__packed struct usb_class_header_function_descriptor
{
    u8  bFunctionLength;
    u8  bDescriptorType;
    u8  bDescriptorSubtype;	/* 0x00 */
    u16 bcdCDC;
} ;

__packed struct usb_class_call_management_descriptor
{
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;	/* 0x01 */
    u8 bmCapabilities;
    u8 bDataInterface;
} ;

__packed struct usb_class_abstract_control_descriptor
{
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;	/* 0x02 */
    u8 bmCapabilities;
} ;

__packed struct usb_class_direct_line_descriptor
{
    u8 bFunctionLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;	/* 0x03 */
} ;

/*
 * HID class descriptor structures
 *
 * c.f. HID 6.2.1
 */

__packed struct usb_class_hid_descriptor
{
    u8  bLength;
    u8  bDescriptorType;
    u16 bcdHID;
    u8  bCountryCode;
    u8  bNumDescriptors;	/* 0x01 */
    u8  bDescriptorType0;
    u16 wDescriptorLength;
    /* optional descriptors are not supported. */
} ;

__packed struct usb_class_report_descriptor
{
    u8  bLength;	/* dummy */
    u8	bDescriptorType;
    u16	wLength;
    u8	bData;
} ;

/*
 *
 * descriptor union structures
 *
 */
__packed struct usb_descriptor
{
    __packed union
    {
        struct usb_generic_descriptor generic;
        struct usb_endpoint_descriptor endpoint;
        struct usb_interface_descriptor interface;
        struct usb_configuration_descriptor configuration;
        struct usb_device_descriptor device;
        struct usb_string_descriptor string;
    } descriptor;

} ;

__packed struct usb_class_descriptor
{
    __packed union
    {
        struct usb_class_function_descriptor function;
        struct usb_class_function_descriptor_generic generic;
        struct usb_class_header_function_descriptor header_function;
        struct usb_class_call_management_descriptor call_management;
        struct usb_class_abstract_control_descriptor abstract_control;
        struct usb_class_direct_line_descriptor direct_line;
        struct usb_class_hid_descriptor hid;
    } descriptor;

} ;

/********************************************************************************
 * This is customized area
 */

 /*
  *
  * for report descriptor record
  *
  */
enum usb_hid_usage_type
{
    //mouse
    BUTTON = 1,
    CONSTANT,   //both
    COORDINATE,
    WHEEL,
    AC_PAN,
    //keyboard
    KEYCODE,
    LED,
    CONTROLKEYCODE,
    REPORTID
};

struct hid_item
{
    unsigned format;
    u8 size;
    u8 type;
    u8 tag;
    union
    {
        u8 _u8;
        s8 _s8;
        u16 _u16;
        s16 _s16;
        u32 _u32;
        s32 _s32;
        u8 *longdata;
    } data;
} ;

__packed struct usb_hid_simple_rd_item
{
	/*
	 *	type: Main_Items(2 bits) + usb_hid_usage_type(6 bits)
	 *	Main_Items:
	 *	INPUT:		00
	 *	OUTPUT:		01
	 *	COLLECTION: 10
	 *	FEATURE:	11
	 */
    u8 type;
    u8 bMainItemType;
    u8 bReportSize;
    u8 bReportCount;
    u8 bReportID;
    s16	bUsageMinimum;
    s16	bUsageMaximum;
    s16	bLogicalMinimum;
    s16	bLogicalMaximum;
} ;

/*
 *
 * Hub staff
 *
 */
__packed struct usb_port_status
{
    unsigned short wPortStatus;
    unsigned short wPortChange;
} ;

__packed struct usb_hub_status
{
    unsigned short wHubStatus;
    unsigned short wHubChange;
} ;

/* Hub descriptor ? */
__packed struct usb_hub_descriptor
{
    u8  bLength;
    u8  bDescriptorType;
    u8  bNbrPorts;
    u16 wHubCharacteristics;
    u8  bPwrOn2PwrGood;
    u8  bHubContrCurrent;
    u8  DeviceRemovable[(USB_MAX_CHILDREN+1+7)/8];
    u8  PortPowerCtrlMask[(USB_MAX_CHILDREN+1+7)/8];
    /* DeviceRemovable and PortPwrCtrlMask want to be variable-length bitmaps that hold max 255 entries. (bit0 is ignored) */
} ;

struct usb_hub_device
{
    struct usb_device *pusb_dev;
    struct usb_hub_descriptor desc;
};

#ifdef DEBUG
static inline void print_device_descriptor(struct usb_device_descriptor *d)
{
    serial_printf("usb device descriptor \n");
    serial_printf("\tbLength %2.2x\n", d->bLength);
    serial_printf("\tbDescriptorType %2.2x\n", d->bDescriptorType);
    serial_printf("\tbcdUSB %4.4x\n", d->bcdUSB);
    serial_printf("\tbDeviceClass %2.2x\n", d->bDeviceClass);
    serial_printf("\tbDeviceSubClass %2.2x\n", d->bDeviceSubClass);
    serial_printf("\tbDeviceProtocol %2.2x\n", d->bDeviceProtocol);
    serial_printf("\tbMaxPacketSize0 %2.2x\n", d->bMaxPacketSize0);
    serial_printf("\tidVendor %4.4x\n", d->idVendor);
    serial_printf("\tidProduct %4.4x\n", d->idProduct);
    serial_printf("\tbcdDevice %4.4x\n", d->bcdDevice);
    serial_printf("\tiManufacturer %2.2x\n", d->iManufacturer);
    serial_printf("\tiProduct %2.2x\n", d->iProduct);
    serial_printf("\tiSerialNumber %2.2x\n", d->iSerialNumber);
    serial_printf("\tbNumConfigurations %2.2x\n", d->bNumConfigurations);
}
#else
/* stubs */
#define print_device_descriptor(d)
#endif /* DEBUG */

/*******************************************************************************/

#endif
