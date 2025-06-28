/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbcb.h

Abstract:

   	The declarations of USB callback function related.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_EHCI_H__
#define __USB_EHCI_H__

/* Definition */
#define NAK_COUNT   15
#define CONTROL_MAX_PKT_SIZE    64
#define BULK_MAX_PKT_SIZE       512
#define H_BIT                   0x00008000
#define DTC_BIT                 0x00004000
#define HIGH_SPEED              0x00002000
#define I_BIT                   0x00000080
#define SINGLE_TRANSACTION      0x40000000
#define	OUT_PID					0x00000000
#define	IN_PID					0x00000100
#define	SETUP_PID				0x00000200


/* QTD */
#define TOTAL_QTD_NUMBER		30
#define QTD_SIZE	            0x20 //(32bytes)
#define QTD_MEM_UNUSED			 0
#define QTD_MEM_USED			 1

/* Queue Head */
typedef __packed struct _USB_QUEUE_HEAD
{
    u32	    Horizontal_Link_Pointer ;
    u32     Endpoint_Characteristics_1 ;
    u32     Endpoint_Characteristics_2 ;
    u32     Current_qTD_Pointer ;
    u32     Next_qTD_Pointer ;
    u32     Alternate_Next_qTD_Pointer ;
    u32     qTD_Token ;
    u32     Buffer_Pointer_0 ;
    u32     Buffer_Pointer_1 ;
    u32     Buffer_Pointer_2 ;
    u32     Buffer_Pointer_3 ;
    u32     Buffer_Pointer_4 ;
}
USB_QUEUE_HEAD;

/* Queue Element Descriptor */
typedef __packed struct _USB_QTD
{
    u32     Next_qTD_Pointer ;
    u32     Alternate_Next_qTD_Pointer ;
    u32     qTD_Token ;
    u32     Buffer_Pointer_0 ;
    u32     Buffer_Pointer_1 ;
    u32     Buffer_Pointer_2 ;
    u32     Buffer_Pointer_3 ;
    u32     Buffer_Pointer_4 ;
}
USB_QTD;

#endif
