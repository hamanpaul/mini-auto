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

#ifndef __USB_HOST_MSC_H__
#define __USB_HOST_MSC_H__

//SCSI Command

#define 	COMMAND_REQUEST_SENSE		    0x03
#define 	COMMAND_INQUIRY				    0x12
#define 	COMMAND_READ_FORMAT_CAPACITY	0x23
#define 	COMMAND_READ10				    0x28
#define 	COMMAND_WRITE10				    0x2A
#define 	COMMAND_UNKNOWN				    0xFF

//CBW
typedef __packed struct _USB_HOST_CBW
{
    u32	    Signature;
    u32     Tag;
    u32     Transfer_Len;
    u8      Flags;
    u8      Logic_Unit_Num;
    u8      CB_Len;
}
USB_HOST_CBW;


typedef __packed struct _USB_HOST_UFI_Command_Inquiry
{
    u8	    OP_Code;
    u8      UnitNum_EVPD;
    u8      Page_Code;
    u8      reserved1;
    u8      Alloc_Len;
    u8      reserved2;
    u16     reserved3;
    u32     reserved4;
    u32     reserved5;
}
USB_HOST_UFI_Command_Inquiry;

typedef __packed struct _USB_HOST_UFI_Command_Request_Sense
{
    u8	    OP_Code;
    u8      Logical_Unit_Num;
    u16     reserved1;
    u8      AllocLen;
    u8      reserved2;
    u16     reserved3;
    u32     Padding1;
    u32     Padding2;
}
USB_HOST_UFI_Command_Request_Sense;

typedef __packed struct _USB_HOST_UFI_Command_Read_Capacity
{
    u8	    OP_Code;
    u8      Logical_Unit_Num;
    u32     Logical_Blk_Addr;
    u32     reserved1;
    u32     reserved2;
    u16     reserved3;
}
USB_HOST_UFI_Command_Read_Capacity;

typedef __packed struct _USB_HOST_UFI_Command_Read10
{
    u8	    OP_Code;
    u8      Logical_Unit_Num;
    u32     Logical_BLK_Addr;
    u8      reserved1;
    u16     Tran_Len;
    u8      Control;
    u16     Padding1;
    u32     Padding2;
}
USB_HOST_UFI_Command_Read10;

#endif
