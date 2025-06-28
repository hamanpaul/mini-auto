/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	usbmsc.h

Abstract:

   	The declarations of USB Mass Storage Class related.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __USB_MSC_H__
#define __USB_MSC_H__

/* Constant */

/*---- device descriptor ----*/

#if (CHIP_OPTION == CHIP_PA9001D)

/* vendor id */
#define USB_MSC_DEV_VENDOR_ID			0x7654
/* product id */
#define USB_MSC_DEV_PRODUCT_ID			0x3210

#elif (CHIP_OPTION == CHIP_PA9002D)

/* vendor id */
#define USB_MSC_DEV_VENDOR_ID			0x4D52	/* MR */
/* product id */
#define USB_MSC_DEV_PRODUCT_ID			0x4456	/* DV */

#else //PA9003
/* vendor id */
#define USB_MSC_DEV_VENDOR_ID			0x4D52	/* MR */
/* product id */
#define USB_MSC_DEV_PRODUCT_ID			0x4456	/* DV */
#endif

/* interface descriptor */

/* class code */
#define USB_MSC_CLASS_MASS_STORAGE		0x08

/* subclass code */
#define USB_MSC_SUBCLASS_RBC			0x01
#define USB_MSC_SUBCLASS_SFF8020I_MMC2		0x02
#define USB_MSC_SUBCLASS_QIC157			0x03
#define USB_MSC_SUBCLASS_UFI			0x04
#define USB_MSC_SUBCLASS_SFF8070I		0x05
#define USB_MSC_SUBCLASS_SCSI			0x06

/* protocol code */
#define USB_MSC_PROTOCOL_CBI_CCI		0x00
#define USB_MSC_PROTOCOL_CBI			0x01
#define USB_MSC_PROTOCOL_BO			0x50

/* Command Block Wrapper (CBW) */
#define USB_MSC_CBW_SIZE			0x1f
typedef __packed struct _USB_MSC_CBW
{
    u32	dCBWSignature;
    u32	dCBWTag;
    u32	dCBWDataTransferLength;
    u8	bmCBWFlags;
    u8	bCBWLUN;
    u8	bCBWCBLength;
    u8	CBWCB[16];
}
USB_MSC_CBW;

/* dCBWSignature */
#define USB_MSC_CBW_SIGNATURE			0x43425355	/* "CBSU" */

/* bmCBWFlags */
#define USB_MSC_CBW_DIR_OUT   			0x00
#define USB_MSC_CBW_DIR_IN    			0x80

/* Command Status Wrapper */
#define USB_MSC_CSW_SIZE			0x0d
typedef __packed struct _USB_MSC_CSW
{
    u32		dCSWSignature;
    u32		dCSWTag;
    u32		dCSWDataResidue;
    u8		bCSWStatus;
}
USB_MSC_CSW;

/* dCSWSignature */
#define USB_MSC_CSW_SIGNATURE			0x53425355	/* "SBSU" */

/* bCSWStatus */
#define USB_MSC_CSW_COMMAND_PASS		0x00
#define USB_MSC_CSW_COMMAND_FAIL		0x01
#define USB_MSC_CSW_PHASE_ERROR			0x02

/*---- class request ----*/

/* max. logical unit number */
#define USB_MSC_MAX_LUN				1

/* buffer count */
#define USB_MSC_BUF_COUNT			2	//2

/* buffer size */
#define USB_MSC_SECTOR_SIZE			512
#define USB_MSC_BUF_SIZE	        	(USB_MSC_SECTOR_SIZE * 32)

/* Bulk-Only Mass Storage Reset */
#define USB_MSC_RESET_bmRequestType		0x21
#define USB_MSC_RESET_bRequest			0xff
#define USB_MSC_RESET_wValue			0x0000
#define USB_MSC_RESET_wIndex			0x0000
#define USB_MSC_RESET_wLength			0x0000

/* Get Max LUN */
#define USB_MSC_GET_MAX_LUN_bmRequestType    	0xa1
#define USB_MSC_GET_MAX_LUN_bRequest    	0xfe
#define USB_MSC_GET_MAX_LUN_wValue		0x0000
#define USB_MSC_GET_MAX_LUN_wIndex		0x0000
#define USB_MSC_GET_MAX_LUN_wLength		0x0001

#endif
