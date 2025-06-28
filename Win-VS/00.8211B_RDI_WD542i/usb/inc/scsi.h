/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	scsi.h

Abstract:

   	The declarations of SCSI command.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __SCSI_H__
#define __SCSI_H__

/*---------------- Group 0 (000xxxxx) - six-byte commands ----------------*/
#define SCSI_CMD_TEST_UNIT_READY		0x00
#define SCSI_CMD_REZERO_UNIT			0x01
//#define SCSI_CMD_VENDOR_SPECIFIC		0x02
#define SCSI_CMD_REQUEST_SENSE			0x03
#define SCSI_CMD_FORMAT_UNIT			0x04
#define SCSI_CMD_READ_BLOCK_LIMITS              0x05
#define SCSI_CMD_REASSIGN_BLOCKS		0x07
#define SCSI_CMD_READ6				0x08
#define SCSI_CMD_WRITE6				0x0a
#define SCSI_CMD_SEEK6				0x0b
//#define SCSI_CMD_VENDOR_SPECIFIC		0x0c
//#define SCSI_CMD_VENDOR_SPECIFIC		0x0d
//#define SCSI_CMD_VENDOR_SPECIFIC		0x0e
#define SCSI_CMD_READ_REVERSE			0x0f
#define SCSI_CMD_WRITE_FILEMARKS		0x10
#define SCSI_CMD_SPACE				0x11
#define SCSI_CMD_INQUIRY			0x12
#define SCSI_CMD_VERIFY6			0x13
#define SCSI_CMD_RECOVER_BUFFERED_DATA		0x14
#define SCSI_CMD_MODE_SELECT6			0x15
#define SCSI_CMD_RESERVE			0x16
#define SCSI_CMD_RELEASE			0x17
#define SCSI_CMD_COPY				0x18
#define SCSI_CMD_MODE_SENSE6			0x1a
#define SCSI_CMD_START_STOP_UNIT		0x1b
#define SCSI_CMD_RECEIVE_DIAGNOSTIC_RESULTS	0x1c
#define SCSI_CMD_SEND_DIAGNOSTIC		0x1d
#define SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1e
//#define SCSI_CMD_VENDOR_SPECIFIC		0x1f

/*---------------- Group 1 (001xxxxx) - ten-byte commands ----------------*/
//#define SCSI_CMD_VENDOR_SPECIFIC		0x20
//#define SCSI_CMD_VENDOR_SPECIFIC		0x21
//#define SCSI_CMD_VENDOR_SPECIFIC		0x22
#define SCSI_CMD_READ_FORMAT_CAPACITIES		0x23
#define SCSI_CMD_SET_WINDOW			0x24
#define SCSI_CMD_READ_CAPACITY			0x25
//#define SCSI_CMD_VENDOR_SPECIFIC		0x26
//#define SCSI_CMD_VENDOR_SPECIFIC		0x27
#define SCSI_CMD_READ10				0x28
#define SCSI_CMD_READ_GENERATION		0x29
#define SCSI_CMD_WRITE10			0x2a
#define SCSI_CMD_SEEK10				0x2b
#define SCSI_CMD_ERASE10			0x2c
#define SCSI_CMD_READ_UPDATED_BLOCK		0x2d
#define SCSI_CMD_WRITE_AND_VERIFY10		0x2e
#define SCSI_CMD_VERIFY10			0x2f
#define SCSI_CMD_SEARCH_DATA_HIGH10		0x30
#define SCSI_CMD_SEARCH_DATA_EQUAL10		0x31
#define SCSI_CMD_SEARCH_DATA_LOW10		0x32
#define SCSI_CMD_SET_LIMITS10			0x33
#define SCSI_CMD_PREFETCH			0x34
#define SCSI_CMD_SYNCHRONIZE_CACHE		0x35
#define SCSI_CMD_LOCK_UNLOCK_CACHE		0x36
#define SCSI_CMD_READ_DEFECT_DATA10		0x37
#define SCSI_CMD_MEDIUM_SCAN			0x38
#define SCSI_CMD_COMPARE			0x39
#define SCSI_CMD_COPY_AND_VERIFY		0x3a
#define SCSI_CMD_WRITE_BUFFER			0x3b
#define SCSI_CMD_READ_BUFFER			0x3c
#define SCSI_CMD_UPDATE_BLOCK			0x3d
#define SCSI_CMD_READ_LONG			0x3e
#define SCSI_CMD_WRITE_LONG			0x3f

/*---------------- Group 2 (010xxxxx) - ten-byte commands ----------------*/
#define SCSI_CMD_CHANGE_DEFINITION		0x40
#define SCSI_CMD_WRITE_SAME			0x41
#define SCSI_CMD_READ_SUBCHANNEL		0x42
#define SCSI_CMD_READ_TOC			0x43
#define SCSI_CMD_READ_HEADER			0x44
#define SCSI_CMD_PLAY_AUDIO10			0x45
//#define SCSI_CMD_VENDOR_SPECIFIC		0x46
#define SCSI_CMD_PLAY_AUDIO_MSF			0x47
#define SCSI_CMD_PLAY_AUDIO_TRACK_INDEX		0x48
#define SCSI_CMD_PLAY_TRACK_RELATIVE		0x49
//#define SCSI_CMD_VENDOR_SPECIFIC		0x4a
#define SCSI_CMD_PAUSE_RESUME			0x4b
#define SCSI_CMD_LOG_SELECT			0x4c
#define SCSI_CMD_LOG_SENSE			0x4d
//#define SCSI_CMD_VENDOR_SPECIFIC		0x4e
//#define SCSI_CMD_VENDOR_SPECIFIC		0x4f
//#define SCSI_CMD_VENDOR_SPECIFIC		0x50
//#define SCSI_CMD_VENDOR_SPECIFIC		0x51
//#define SCSI_CMD_VENDOR_SPECIFIC		0x52
//#define SCSI_CMD_VENDOR_SPECIFIC		0x53
//#define SCSI_CMD_VENDOR_SPECIFIC		0x54
#define SCSI_CMD_MODE_SELECT10			0x55
//#define SCSI_CMD_VENDOR_SPECIFIC		0x56
//#define SCSI_CMD_VENDOR_SPECIFIC		0x57
//#define SCSI_CMD_VENDOR_SPECIFIC		0x58
//#define SCSI_CMD_VENDOR_SPECIFIC		0x59
#define SCSI_CMD_MODE_SENSE10			0x5a
//#define SCSI_CMD_VENDOR_SPECIFIC		0x5b
//#define SCSI_CMD_VENDOR_SPECIFIC		0x5c
//#define SCSI_CMD_VENDOR_SPECIFIC		0x5d
//#define SCSI_CMD_VENDOR_SPECIFIC		0x5e
//#define SCSI_CMD_VENDOR_SPECIFIC		0x5f

/*---------------- Group 3 (011xxxxx) - reserved ----------------*/

/*---------------- Group 4 (100xxxxx) - reserved ----------------*/

/*---------------- Group 5 (101xxxxx) - twelve-byte commands ----------------*/
#define SCSI_CMD_READ12				0xa8
#define SCSI_CMD_WRITE12			0xaa

/*---------------- Group 6 (110xxxxx) - vendor-specific ----------------*/

/*---------------- Group 7 (111xxxxx) - vendor-specific ----------------*/

/* Control byte of CDB */
#define SCSI_CDB_CONTROL_LINK			0x01
#define SCSI_CDB_CONTROL_FLAG			0x02

/*---------------- Group 0 (000xxxxx) - six-byte commands ----------------*/
/*-------- TEST UNIT READY --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_TEST_UNIT_READY
{
    u8	operationCode;
    __packed struct
    {
u8	reserved:
        5;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u8	reserved[3];
    u8	control;
}
SCSI_CDB_TEST_UNIT_READY;

/*-------- REQUEST SENSE --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_REQUEST_SENSE
{
    u8	operationCode;
    __packed struct
    {
u8	reserved:
        5;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u8	reserved[2];
    u8	allocationLength;
    u8	control;
}
SCSI_CDB_REQUEST_SENSE;

/*---- Sense data ----*/
#define SCSI_SENSE_KEY_SPECIFIC		3
typedef __packed struct _SCSI_SENSE_DATA
{
    __packed struct
    {
u8	errorCode:
        7;
u8	valid:
        1;
    }
    A0B;
    u8	segmentNumber;
    __packed struct
    {
u8	senseKey:
        4;
u8	reserved:
        1;
u8	incorrectLengthIndicator:
        1;
u8	endOfMedium:
        1;
u8	filemark:
        1;
    }
    A2B;
    u32	information;			/* bSwap32() */
    u8	additionalSenseLength;
    u32	commandSpecificInformation;	/* bSwap32() */
    u8	additionalSenseCode;
    u8	additionalSenseCodeQualifier;
    u8	fieldReplaceableUnitCode;
    u8	senseKeySpecific[SCSI_SENSE_KEY_SPECIFIC];
}
SCSI_SENSE_DATA;

/*---- Sense data constant ----*/

/* error code */
#define SCSI_SENSE_CURRENT_ERROR	0x70
#define SCSI_SENSE_DEFERRED_ERROR	0x71

/* additional sense length */
#define SCSI_SENSE_AD_LEN		0x0a

/* sense key */
#define SCSI_SK_NO_SENSE		0x00
#define SCSI_SK_RECOVER_ERR		0x01
#define SCSI_SK_NOT_READY               0x02
#define SCSI_SK_MEDIUM_ERR		0x03
#define SCSI_SK_HARDWARE_ERR		0x04
#define SCSI_SK_ILLEGAL_REQ		0x05
#define SCSI_SK_UNIT_ATTENTION		0x06
#define SCSI_SK_DATA_PROTECT		0x07
#define SCSI_SK_BLANK_CHECK		0x08
#define SCSI_SK_VENDOR_SPEC		0x09
#define SCSI_SK_COPY_ABORT		0x0a
#define SCSI_SK_ABORT_COMMAND		0x0b
#define SCSI_SK_EQUAL			0x0c
#define SCSI_SK_VOLUME_OVERFLOW         0x0d
#define SCSI_SK_MISCOMPARE              0x0e
#define SCSI_SK_RESERVED		0x0f

/* DESCRIPTION		ASC	ASCQ	*/
/* NO SENSE INFO	0x00	0x00	*/
#define SCSI_ASC_NO_SENSE_INFO		0x00
#define SCSI_ASCQ_NO_SENSE_INFO         0x00

/* WRITE FAULT		0x03	0x00	*/
#define SCSI_ASC_WRITE_FAULT		0x03
#define SCSI_ASCQ_WRITE_FAULT		0x00

/* NOT READY INIT REQ	0x04	0x02	*/
#define SCSI_ASC_NOT_READY_INIT_REQ	0x04
#define SCSI_ASCQ_NOT_READY_INIT_REQ	0x02

/* READ ERROR		0x11	0x00	*/
#define SCSI_ASC_READ_ERROR		0x11
#define SCSI_ASCQ_READ_ERROR		0x00

/* MISCOMPARE		0x1d	0x00	*/
#define SCSI_ASC_MISCOMPARE		0x1d
#define SCSI_ASCQ_MISCOMPARE		0x00

/* INVALID CDB		0x20	0x00	*/
#define SCSI_ASC_INVALID_CMD_OP		0x20
#define SCSI_ASCQ_INVALID_CMD_OP		0x00

/* INVALID CDB		0x24	0x00	*/
#define SCSI_ASC_INVALID_CDB		0x24
#define SCSI_ASCQ_INVALID_CDB		0x00

/* INVALID PARAM	0x26	0x02	*/
#define SCSI_ASC_INVALID_PARAM		0x26
#define SCSI_ASCQ_INVALID_PARAM		0x02

/* WRITE PROTECTED	0x27	0x00	*/
#define SCSI_ASC_WRITE_PROTECTED	0x27
#define SCSI_ASCQ_WRITE_PROTECTED	0x00

/* NOT READY TO READY	0x28	0x00	*/
#define SCSI_ASC_NOT_READY_TO_READY	0x28
#define SCSI_ASCQ_NOT_READY_TO_READY	0x00

/* MEDIUM NOT PRESENT	0x3a	0x00	*/
#define SCSI_ASC_MEDIUM_NOT_PRESENT	0x3a
#define SCSI_ASCQ_MEDIUM_NOT_PRESENT	0x00

/*-------- INQUIRY --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_INQUIRY
{
    u8	operationCode;
    __packed struct
    {
u8	enableVitalProductData:
        1;
u8	reserved:
        4;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u8	pageCode;
    u8	reserved;
    u8	allocationLength;
    u8	control;
}
SCSI_CDB_INQUIRY;

/*---- Standard INQUIRY data ----*/
typedef __packed struct _SCSI_STANDARD_INQUIRY_DATA
{
    __packed struct
    {
u8	peripheralDeviceType:
        5;
u8	peripheralQualifier:
        3;
    }
    A0B;
    __packed struct
    {
u8	deviceTypeModifier:
        7;
u8	removableMedium:
        1;
    }
    A1B;
    __packed struct
    {
u8	ansiApprovedVersion:
        3;
u8	ecmaVersion:
        3;
u8	isoVersion:
        2;
    }
    A2B;
    __packed struct
    {
u8	responseDataFormat:
        4;
u8	reserved:
        2;
u8	terminateIoProcess:
        1;
u8	asynchronousEventNotificationCapability:
        1;
    }
    A3B;
    u8	additionalLength;
    u8	reserved[2];
    __packed struct
    {
u8	softReset:
        1;
u8	commandQueue:
        1;
u8	reserved:
        1;
u8	linkedCommand:
        1;
u8	synchronousTransfer:
        1;
u8	wideBus16:
        1;
u8	wideBus32:
        1;
u8	relativeAddress:
        1;
    }
    A7B;
    u8	vendorIdentification[8];
    u8	productIdentification[16];
    u8	productRevisionLevel[4];
}
SCSI_STANDARD_INQUIRY_DATA;

/* peripheralDeviceType */
#define SCSI_DIRECT_ACCESS_DEVICE	0x00
#define SCSI_SEQUENTIAL_ACCESS_DEVICE	0x01
#define SCSI_PRINTER_DEVICE		0x02
#define SCSI_PROCESSOR_DEVICE		0x03
#define SCSI_WRITE_ONCE_DEVICE		0x04
#define SCSI_CDROM_DEVICE		0x05
#define SCSI_SCANNER_DEVICE		0x06
#define SCSI_OPTICAL_MEMORY_DEVICE	0x07
#define SCSI_MEDIUM_CHANGER_DEVICE	0x08
#define SCSI_COMMUNICATION_DEVICE	0x09

/* peripheralQualifier */
#define SCSI_PHYS_SUPP_CONN		0x00
#define SCSI_PHYS_SUPP_NOT_CONN		0x01
#define SCSI_PHYS_NOT_SUPP		0x03

/* isoVersion */
#define SCSI_ISO_VERSION		0x00

/* ecmaVersion */
#define SCSI_ECMA_VERSION		0x00

/* ansiApprovedVersion */
#define SCSC_ANSI_APPROVED_VERSION	0x00

/* responseDataFormat */
#define SCSI_ANSI_NONE			0x00
#define SCSI_ANSI_1			0x01
#define SCSI_ANSI_2			0x02

/* vendorIdentification[8] */
#define SCSI_VENDOR_ID			"MARS"

/* productIdentification[16] */
#define SCSI_PRODUCT_ID            	"USB MSC Device"

/* productRevisionLevel[4] */
#define SCSI_REVISION_LEVEL		"0100"

/*-- Vital Porduct Data --*/
/* ASCII implemented operating definition page*/
#define SCSI_VPD_ASCII_OPR_DEF_DESC_LENGTH	10
typedef __packed struct _SCSI_VPD_ASCII_IMPL_OPR_DEF_PAGE
{
    __packed struct
    {
u8	peripheralDeviceType:
        5;
u8	peripheralQualifier:
        3;
    }
    A0B;
    u8	pageCode;
    u8	reserved;
    u8	pageLength;
    u8	asciiOprDefDescLength;
    u8	asciiOprDefDescData[SCSI_VPD_ASCII_OPR_DEF_DESC_LENGTH];
}
SCSI_VPD_ASCII_IMPL_OPR_DEF_PAGE;

/* ASCII infomation page */
#define SCSI_VPD_ASCII_INFO_LENGTH		10
typedef __packed struct _SCSI_VPD_ASCII_INFO_PAGE
{
    __packed struct
    {
u8	peripheralDeviceType:
        5;
u8	peripheralQualifier:
        3;
    }
    A0B;
    u8	pageCode;
    u8	reserved;
    u8	pageLength;
    u8	asciiLength;
    u8	asciiInfo[SCSI_VPD_ASCII_INFO_LENGTH];
}
SCSI_VPD_ASCII_INFO_PAGE;

/* Implemented operating definition page */
#define	SCSI_VPD_SUPP_OPR_DEF_LIST_LENGTH	10
typedef __packed struct _SCSI_VPD_IMPL_OPR_DEF_PAGE
{
    __packed struct
    {
u8	peripheralDeviceType:
        5;
u8	peripheralQualifier:
        3;
    }
    A0B;
    u8	pageCode;
    u8	reserved;
    u8	pageLength;
    u8	currentOprDef;
    u8	defaultOprDef;
    u8	supportedOprDefList[SCSI_VPD_SUPP_OPR_DEF_LIST_LENGTH];
}
SCSI_VPD_IMPL_OPR_DEF_PAGE;

/* Supported vital product data page */
#define	SCSI_VPD_SUPP_PAGE_LIST_LENGTH		10
typedef __packed struct _SCSI_VPD_SUPP_DATA_PAGE
{
    __packed struct
    {
u8	peripheralDeviceType:
        5;
u8	peripheralQualifier:
        3;
    }
    A0B;
    u8	pageCode;
    u8	reserved;
    u8	pageLength;
    u8	supportedPageList[SCSI_VPD_SUPP_PAGE_LIST_LENGTH];
}
SCSI_VPD_SUPP_DATA_PAGE;

/* Unit serial number page */
#define	SCSI_VPD_UNIT_SERIAL_NUMBER_LENGTH	10
typedef __packed struct _SCSI_VPD_UNIT_SERIAL_NUMBER_PAGE
{
    __packed struct
    {
u8	peripheralDeviceType:
        5;
u8	peripheralQualifier:
        3;
    }
    A0B;
    u8	pageCode;
    u8	reserved;
    u8	pageLength;
    u8	productSerialNumber[SCSI_VPD_UNIT_SERIAL_NUMBER_LENGTH];
}
SCSI_VPD_UNIT_SERIAL_NUMBER_PAGE;

/*-------- MODE SELECT(6) --------*/
/*---- Command Descriptor Block ----*/
typedef struct _SCSI_CDB_MODE_SELECT6
{
    u8	operationCode;
    __packed struct
    {
u8	savePages:
        1;
u8	reserved:
        3;
u8	pageFormat:
        1;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u8	reserved[2];
    u8	parameterListLength;
    u8	control;
}
SCSI_CDB_MODE_SELECT6;

/*-------- MODE SENSE(6) --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_MODE_SENSE6
{
    u8	operationCode;
    __packed struct
    {
u8	reserved0:
        3;
u8	disableBlockDescriptors:
        1;
u8	reserved1:
        1;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    __packed struct
    {
u8	pageCode:
        6;
u8	pageControl:
        2;
    }
    A2B;
    u8	reserved;
    u8	allocationLength;
    u8	control;
}
SCSI_CDB_MODE_SENSE6;

/*-------- START STOP UNIT --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_START_STOP_UNIT
{
    u8	operationCode;
    __packed struct
    {
u8	immediate:
        1;
u8	reserved:
        4;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u8	reserved[2];
    __packed struct
    {
u8	start:
        1;
u8	loadEject:
        1;
u8	reserved:
        6;
    }
    A4B;
    u8	control;
}
SCSI_CDB_START_STOP_UNIT;

/*-------- PREVENT_MEDIUM REMOVAL --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_PREVENT_ALLOW_MEDIUM_REMOVAL
{
    u8	operationCode;
    __packed struct
    {
u8	reserved:
        5;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u8	reserved[2];
    __packed struct
    {
u8	prevent:
        1;
u8	reserved:
        7;
    }
    A4B;
    u8	control;
}
SCSI_CDB_PREVENT_ALLOW_MEDIUM_REMOVAL;

/*---------------- Group 1 (001xxxxx) - ten-byte commands ----------------*/
/*-------- READ FORMAT CAPACITIES --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_READ_FORMAT_CAPACITIES
{
    u8	operationCode;
    __packed struct
    {
u8	reserved:
        5;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u8	reserved[5];
    u16	allocationLength;	/* bSwap16() */
    u8	control;
}
SCSI_CDB_READ_FORMAT_CAPACITIES;

/*---- Capacity list header ----*/
typedef __packed struct _SCSI_CAPACITY_LIST_HEADER
{
    u8	reserved[3];
    u8	capabilityListLength;
}
SCSI_CAPACITY_LIST_HEADER;

/*---- Capacity descriptor ----*/
typedef __packed struct _SCSI_CAPACITY_DESC
{
    u32	numberOfBlock;		/* bSwap32() */
    __packed struct
    {
u8	descriptorCode:
        2;
u8	reserved:
        6;
    }
    A4B;
    u8	blockLength[3];
}
SCSI_CAPACITY_DESC;

/* numberOfBlock */
#define SCSI_DEF_NUMBER_OF_BLOCK       	0xffffffff

/* blockLength[3] */
#define SCSI_DEF_BLOCK_LENGTH		0x200

/* descriptorCode */
#define SCSI_DESC_CODE_NONE		0x00
#define SCSI_DESC_CODE_UNFORMATTED	0x01
#define SCSI_DESC_CODE_FORMATTED	0x02
#define SCSI_DESC_CODE_NO_CARTRIDGE	0x03

/*-------- READ CAPACITY --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_READ_CAPACITY
{
    u8	operationCode;
    __packed struct
    {
u8	relativeAddress:
        1;
u8	reserved:
        4;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u32	logicalBlockAddress;		/* bSwap32() */
    u8	reserved[2];
    __packed struct
    {
u8	partialMediumIndicator:
        1;
u8	reserved:
        7;
    }
    A8B;
    u8	control;
}
SCSI_CDB_READ_CAPACITY;

/*---- capacity data ----*/
typedef __packed struct _SCSI_READ_CAPACITY_DATA
{
    u32	returnLogicalBlockAddress;	/* bSwap32() */
    u32	blockLength;			/* bSwap32() */
}
SCSI_READ_CAPACITY_DATA;

/*-------- READ(10) --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_READ10
{
    u8	operationCode;
    __packed struct
    {
u8	relativeAddress:
        1;
u8	reserved:
        2;
u8	forceUnitAccess:
        1;
u8	disablePageOut:
        1;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u32	logicalBlockAddress;		/* bSwap32() */
    u8	reserved;
    u16	transferLength;			/* bSwap16() */
    u8	control;
}
SCSI_CDB_READ10;

/*-------- WRITE(10) --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_WRITE10
{
    u8	operationCode;
    __packed struct
    {
u8	relativeAddress:
        1;
u8	reserved:
        2;
u8	forceUnitAccess:
        1;
u8	disablePageOut:
        1;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u32	logicalBlockAddress;		/* bSwap32() */
    u8	reserved;
    u16	transferLength;			/* bSwap16() */
    u8	control;
}
SCSI_CDB_WRITE10;

/*-------- VERIFY10 --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_VERIFY10
{
    u8	operationCode;
    __packed struct
    {
u8	relativeAddress:
        1;
u8	byteCheck:
        1;
u8	reserved:
        2;
u8	disablePageOut:
        1;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    u32	logicalBlockAddress;		/* bSwap32() */
    u8	reserved;
    u16	transferLength;			/* bSwap16() */
    u8	control;
}
SCSI_CDB_VERIFY10;

/*---------------- Group 2 (010xxxxx) - ten-byte commands ----------------*/
/*-------- MODE SENSE(10) --------*/
/*---- Command Descriptor Block ----*/
typedef __packed struct _SCSI_CDB_MODE_SENSE10
{
    u8	operationCode;
    __packed struct
    {
u8	reserved0:
        3;
u8	disableBlockDescriptors:
        1;
u8	reserved1:
        1;
u8	logicalUnitNumber:
        3;
    }
    A1B;
    __packed struct
    {
u8	pageCode:
        6;
u8	pageControl:
        2;
    }
    A2B;
    u8	reserved[4];
    u16	allocationLength;		/* bSwap16() */
    u8	control;
}
SCSI_CDB_MODE_SENSE10;

/*---- Mode parameter header (6) ----*/
typedef __packed struct _SCSI_MODE_PARAM_HEADER6
{
    u8	modeDataLength;
    u8	mediumType;
    __packed struct
    {
u8	reserved0:
        4;
u8	disablePageOutForceUnitAccess:
        1;
u8	reserved1:
        2;
u8	writeProtect:
        1;
    }
    A2B;
    u8	deviceSpecificParam;
    u8	blockDescLength;
}
SCSI_MODE_PARAM_HEADER6;

/*---- Mode parameter header (10) ----*/
typedef __packed struct _SCSI_MODE_PARAM_HEADER10
{
    u16	modeDataLength;			/* bSwap16() */
    u8	mediumType;
    __packed struct
    {
u8	reserved0:
        4;
u8	disablePageOutForceUnitAccess:
        1;
u8	reserved1:
        2;
u8	writeProtect:
        1;
    }
    A3B;
    u8	reserved[2];
    u16	blockDescLength;		/* bSwap16() */
}
SCSI_MODE_PARAM_HEADER10;

/* mediumType */
#define SCSI_MEDIUM_TYPE_DEFAULT	0x00

/*---- Mode parameter block descriptor ----*/
#define SCSI_MODE_PARAM_BLOCK_DESC_COUNT	1       /* max count of mode parameter block descriptor */
typedef __packed struct _SCSI_MODE_PARAM_BLOCK_DESC
{
    u8	densityCode;
    u8	numberOfBlocks[3];
    u8	reserved;
    u8	blockLength[3];
}
SCSI_MODE_PARAM_BLOCK_DESC;

/*---- Mode page format (general) ----*/
#define SCSI_MODE_PAGE_COUNT			9       /* max count of mode page */
#define SCSI_MODE_PARAM_LENGTH			30
typedef __packed struct _SCSI_MODE_PAGE_FORMAT
{
    __packed struct
    {
u8	pageCode:
        6;
u8	reserved:
        1;
u8	parameterSavable:
        1;
    }
    A0B;
    u8	pageLength;
    u8	modeParam[SCSI_MODE_PARAM_LENGTH];
}
SCSI_MODE_PAGE_FORMAT;

/* pageCode */
#define SCSI_CONTROL_MODE_PAGE_CODE			0x0a
#define SCSI_DISCONN_RECONN_PAGE_CODE			0x02
#define SCSI_PERIPHERAL_DEVICE_PAGE_CODE		0x09
#define SCSI_CACHE_PAGE_CODE				0x08
#define SCSI_FORMAT_DEVICE_PAGE_CODE			0x03
#define SCSI_MEDIUM_TYPE_SUPPORTED_PAGE_CODE		0x0b
#define SCSI_NOTCH_PAGE_CODE				0x0c
#define SCSI_READ_WRITE_ERROR_RECOVERY_PAGE_CODE	0x01
#define SCSI_VERIFY_ERROR_RECOVERY_PAGE_CODE		0x07
#define SCSI_ALL_PAGE_CODE				0x3f

/* Control mode page */
typedef __packed struct _SCSI_CONTROL_MODE_PAGE
{
    __packed struct
    {
u8	pageCode:
        6;
u8	reserved:
        1;
u8	parameterSavable:
        1;
    }
    A0B;
    u8	pageLength;
    __packed struct
    {
u8	reportLogExceptionCondition:
        1;
u8	reserved:
        7;
    }
    A2B;
    __packed struct
    {
u8	disableQueue:
        1;
u8	queueErrorManagement:
        1;
u8	reserved:
        2;
u8	queueAlgorithmModifier:
        4;
    }
    A3B;
    __packed struct
    {
u8	errorAenPermission:
        1;
u8	unitAttentionAenPermission:
        1;
u8	readyAenPermission:
        1;
u8	reserved:
        4;
u8	enableExtendedContingentAlliance:
        1;
    }
    A4B;
    u8	reserved;
    u16	readyAenHoldoffPeriod;		/* bSwap16() */
}
SCSI_CONTROL_MODE_PAGE;

/* Disconnect-reconnect page */
typedef __packed  struct _SCSI_DISCONN_RECONN_PAGE
{
    __packed struct
    {
u8	pageCode:
        6;
u8	reserved:
        1;
u8	parameterSavable:
        1;
    }
    A0B;
    u8	pageLength;
    u8	bufferFullRatio;
    u8	bufferEmptyRatio;
    u16	busInactLimit;			/* bSwap16() */
    u16	disconnTimeLimit;		/* bSwap16() */
    u16	connTimeLimit;			/* bSwap16() */
    u16	maxBurstSize;			/* bSwap16() */
    __packed struct
    {
u8	dataTransferDisconnectControl:
        2;
u8	reserved:
        6;
    }
    A12B;
    u8	reserved[3];
}
SCSI_DISCONN_RECONN_PAGE;

/* dataTransferDisconnectControl */
#define SCSI_DISCONN_CTRL_NONE			0x00
#define SCSI_DISCONN_CTRL_UNTIL_ALL_DONE	0x01
#define SCSI_DISCONN_CTRL_UNTIL_CUR_DONE	0x03

/* Peripheral device page */
typedef __packed struct _SCSI_PERIPHERAL_DEVICE_PAGE
{
    __packed struct
    {
u8	pageCode:
        6;
u8	reserved:
        1;
u8	parameterSavable:
        1;
    }
    A0B;
    u8	pageLength;
    u16	interfaceIdentifier;    	/* bSwap16() */
    u8	reserved[4];
}
SCSI_PERIPHERAL_DEVICE_PAGE;

/* interfaceIdentifier */
#define SCSI_IF_ID_SCSI				0x0000
#define SCSI_IF_ID_SMI				0x0001
#define SCSI_IF_ID_ESDI				0x0002
#define SCSI_IF_ID_IPI2				0x0003
#define SCSI_IF_ID_IPI3				0x0004

/* Cache page */
typedef __packed struct _SCSI_CACHE_PAGE
{
    __packed struct
    {
u8	pageCode:
        6;
u8	reserved:
        1;
u8	parameterSavable:
        1;
    }
    A0B;
    u8	pageLength;
    __packed struct
    {
u8	readCacheDisable:
        1;
u8	multiplyFactor:
        1;
u8	writeCacheEnable:
        1;
u8	reserved:
        5;
    }
    A2B;
    __packed struct
    {
u8	writeRetentionPriority:
        4;
u8	demandReadRetentionPriority:
        4;
    }
    A3B;
    u16	disablePrefetchTransferLength;	/* bSwap16() */
    u16	minPrefetch;			/* bSwap16() */
    u16	maxPrefetch;			/* bSwap16() */
    u16	maxPrefetchCeiling;		/* bSwap16() */
}
SCSI_CACHE_PAGE;

/* Format device page */
typedef __packed struct _SCSI_FORMAT_DEVICE_PAGE
{
    __packed struct
    {
u8	pageCode:
        6;
u8	reserved:
        1;
u8	parameterSavable:
        1;
    }
    A0B;
    u8	pageLength;
    u16	trackPerZone;		/* bSwap16() */
    u16	altSectorPerZone;	/* bSwap16() */
    u16	altTrackPerZone;	/* bSwap16() */
    u16	altTrackPerLogicalUnit;	/* bSwap16() */
    u16	sectorPerTrack;		/* bSwap16() */
    u16	bytePerPhysicalSector;	/* bSwap16() */
    u16	interleave;		/* bSwap16() */
    u16	trackSkewFactor;	/* bSwap16() */
    u16	cylinderSkewFactor;	/* bSwap16() */
    __packed struct
    {
u8	reserved:
        4;
u8	surface:
        1;
u8	removable:
        1;
u8	hardSector:
        1;
u8	softSector:
        1;
    }
    A20B;
    u8	reserved[3];
}
SCSI_FORMAT_DEVICE_PAGE;

/* Medium types supported page */
typedef __packed struct _SCSI_MEDIUM_TYPE_SUPPORTED_PAGE
{
    __packed struct
    {
u8	pageCode:
        6;
u8	reserved:
        1;
u8	parameterSavable:
        1;
    }
    A0B;
    u8	pageLength;
    u8	reserved[2];
    u8	mediumTypeOneSupported;
    u8	mediumTypeTwoSupported;
    u8	mediumTypeThreeSupported;
    u8	mediumTypeFourSupported;
}
SCSI_MEDIUM_TYPE_SUPPORTED_PAGE;

/* mediumTypeXxxSupported */
#define SCSI_MEDIUM_TYPE_DEFAULT		0x00

/* Notch page */
typedef __packed struct _SCSI_NOTCH_PAGE
{
    __packed struct
    {
u8	pageCode:
        6;
u8	reserved:
        1;
u8	parameterSavable:
        1;
    }
    A0B;
    u8	pageLength;
    __packed struct
    {
u8	reserved:
        6;
u8	logicalPhysicalNotch:
        1;
u8	notchDrive:
        1;
    }
    A2B;
    u8	reserved;
    u16	maxNumberOfNotch;	/* bSwap16() */
    u16	activeNotch;		/* bSwap16() */
    u32	startBoundary;		/* bSwap32() */
    u32	endBoundary;		/* bSwap32() */
    u8	pageNotch[8];		/* bSwap64() */
}
SCSI_NOTCH_PAGE;

/* Read write error recovery page */
typedef __packed struct _SCSI_READ_WRITE_ERROR_RECOVERY_PAGE
{
    __packed struct
    {
u8	pageCode:
        6;
u8	reserved:
        1;
u8	parameterSavable:
        1;
    }
    A0B;
    u8	pageLength;
    __packed struct
    {
u8	disableCorrection:
        1;
u8	disableTransferOnError:
        1;
u8	postError:
        1;
u8	enableEarlyRecovery:
        1;
u8	readContinuous:
        1;
u8	transferBlock:
        1;
u8	autoReadReallocEnable:
        1;
u8	autoWriteReallocEnable:
        1;
    }
    A2B;
    u8	readRetryCount;
    u8	correctionSpan;
    u8	headOffsetCount;
    u8	dataStrobeOffsetCount;
    u8	reserved1;
    u8	writeRetryCount;
    u8	reserved2;
    u16	recoveryTimeLimit;		/* bSwap16() */
}
SCSI_READ_WRITE_ERROR_RECOVERY_PAGE;

/* Verify error recovery page */
typedef __packed struct _SCSI_VERIFY_ERROR_RECOVERY_PAGE
{
    __packed struct
    {
u8	pageCode:
        6;
u8	reserved:
        1;
u8	parameterSavable:
        1;
    }
    A0B;
    u8	pageLength;
    __packed struct
    {
u8	disableCorrection:
        1;
u8	disableTransferOnError:
        1;
u8	postError:
        1;
u8	enableEarlyRecovery:
        1;
u8	reserved:
        4;
    }
    A2B;
    u8	verifyRetryCount;
    u8	verifyCorrectionSpan;
    u8	reserved[5];
    u16	verifyRecoveryTimeLimit;	/* bSwap16() */
}
SCSI_VERIFY_ERROR_RECOVERY_PAGE;

#endif
