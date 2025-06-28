/*

Copyright (c) 2005  Himax Technologies, Inc.

Module Name:

    CF.h

Abstract:

    The structures and constants of SD Card.

Environment:

        ARM RealView Developer Suite

Revision History:

    2007/03/25  Ven-Chin Chen   Create              

*/

#ifndef __CF_H__
#define __CF_H__

/*****************************************************************************/
/* Structure Definition                                              */
/*****************************************************************************/

/*
 * Device Register Task File
 */
typedef struct _CF_DEVICE_REGISTER
{
    u8 ucCommand;
    u8 ucCardDriveHead;
    u8 ucCylHigh;
    u8 ucCylLow;    
    u8 ucSecNum;    
    u8 ucSecCnt;        
    u8 ucFeature;            
} CF_DEVICE_REGISTER;

/*
 * HD header info format 
 */
typedef struct _HD_INFORMATION
{
    u16   temp1;        /* Word 0: General configuration bit-significant information */
    u16   cylinders;    /* Word 1: Number of logical cylinders */
    u16   temp2;        /* Word 2: Not Used or Reserved */ 
    u16   heads;        /* Word 3: Number of logical heads */ 
    u16   temp3[2];     /* Word 4-5: Not Used or Reserved */    
    u16   sectors;      /* Word 6: Number of logical sectors per logical track */
    u16   temp4[3];     /* Word 7-9: Not Used or Reserved */
    u8    ser_no[20];   /* Word 10-19: Serial number (20 ASCII characters) */
    u16   temp5[2];     /* Word 20-21: Not Used or Reserved */
    u16   ecc_num;      /* Word 22: obsolete */
    u8    firware_rev[8]; /* Word 23-26: Firmware revision (8 ASCII characters) */
    u8    model_num[40]; /* Word 27-46: Model number (40 ASCII characters) */
    u16   support_max_sector_no;   /* word 47: 01h-FFh = Maximum number of sectors that shall be 
                                      transferred per interrupt on READ/WRITE MULTIPLE commands */
    u16   temp6;                   /* word 48: Not Used or Reserved */
    u16   capability;              /* word 49: Capabilities */
    u16   capability2;             /* word 50: Capabilities 2 */
    u16   pio_mode;                /* word 51: PIO data transfer mode number */
    u16   temp7;                   /* word 52: Retired */
    u16   field_valid;             /* word 53: 1 = the fields reported in words x-y are valid */
    u16   cur_cylinders;           /* word 54: Number of current logical cylinders */
    u16   cur_heads;               /* word 55: Number of current logical heads */
    u16   cur_sectors;             /* word 56: Number of current logical sectors per track */
    u16   cur_capacity_in_sectors[2]; /* word 57-58: Current capacity in sectors */
    u16   current_max_sector_no;   /* (VCC $) word 59:  */    
    u32   lba_capacity;         /* word 60-61: Total number of user addressable sectors (LBA mode only) */
    u16   temp8;               /* word 62: Retired */
    u16   multiword_DMA_mode;   /* word 63: Multiword DMA mode 0-2 */
    u16   eide_pio_mode;           /* word 64: Advanced PIO modes supported */              
    u16   min_multiword_dma_tran_cycle_time_per_word; /* word 65: Minimum Multiword DMA transfer cycle time per word */
    u16   recommended_multiword_dma_tran_cycle_time_per_word; /* word 66: Manufacturer¡¦s recommended Multiword DMA transfer cycle time */    
    u16   min_PIO_tran_cycle_time_without_flow_control; /* word 67: Minimum PIO transfer cycle time without flow control */        
    u16   min_PIO_tran_cycle_time_with_IORDY_flow_control; /* word 68: Minimum PIO transfer cycle time with IORDY flow control */            
    u16   temp9[6];              /* word 69-74: Not Used or Reserved */
    u16   queue_depth;     /* word 75: Queue depth */    
    u16   temp10[4];              /* word 76-79: Not Used or Reserved */
    u16   major_ver_num;     /* word 80: Major version number */    
    u16   minor_ver_num;     /* word 81: Minor version number */    
    u16   command_set_support;     /* word 82: Command set supported */
    u16   command_set_support2;     /* word 83: Command sets supported 2 */
    u16   command_set_feature_support_ext;     /* word 84: Command set/feature supported extension */
    u16   command_set_feature_ena[2];     /* word 85-86: Command set/feature enabled */
    u16   command_set_feature_default;     /* word 87: Command set/feature default */
    u16   ultra_DMA_mode;     /* word 88: Ultra DMA mode */
    u16   security_erase_time;     /* word 89: Time required for security erase unit completion */
    u16   enhanced_security_erase_time;     /* word 90: Time required for Enhanced security erase completion */    
    u16   current_adv_pow_mana_val;     /* word 91: Current advanced power management value */        
    u16   temp11[35];     /* word 92-126: Not Used or Reserved */        
    u16   removable_media_status_notification_supp; /* word 127: Removable Media Status Notification feature set support */    
    u16   security_status; /* word 128: Security status */    
    u16   vendor_specific[31]; /* word 129-159: Security status */    
    u16   temp12[96];             /* word 160-255: Not Used or Reserved */    
} HD_INFORMATION;

typedef enum _TRANSFER_MODE_E
{ 
    PIO_MODE = 0 << 3, 
    DMA_MODE = 1 << 5, 
    UDMA_MODE = 1 << 6
} TRANSFER_MODE_E;
typedef enum _MODE_VALUE_E
{ MODE_0, MODE_1, MODE_2, MODE_3, MODE_4, MODE_5} MODE_VALUE_E;

#if 0
typedef enum _READ_OR_WRITE_CARD
{
	WRITE_CF_CARD,
	READ_CF_CARD
} READ_OR_WRITE_CARD_E;
#endif

/*****************************************************************************/
/* Constant Definition                                               */
/*****************************************************************************/

/*
 * Command Index for CF/ATA Device
 */
 
#define CF_CMD_IDENTIFY_DEV   0xec   /* IDENTIFY DEVICE */
#define CF_CMD_SET_MULTIPLE   0xc6   /* SET MULTIPLE MODE */
#define CF_CMD_SET_FEATURE    0xef   /* SET FEATURES */
#define CF_CMD_READ_SECTOR    0x20   /* READ SECTOR(S) */
#define CF_CMD_WRITE_SECTOR   0x30   /* WRITE SECTOR(S) */
#define CF_CMD_READ_MULTIPLE  0xc4   /* READ MULTIPLE */
#define CF_CMD_WRITE_MULTIPLE 0xc5   /* WRITE MULTIPLE */
#define CF_CMD_ERASE_SECTORS  0xc0   /* CFA ERASE SECTORS */

#define CF_CMD_READ_DMA       0xc8   /* READ DMA */
#define CF_CMD_WRITE_DMA      0xca   /* WRITE DMA */


#define CF_CMD_read_dma_ext    0x25   /* Resets all cards to idle state */

#define CF_CMD_write_dma_ext    0x35   /* Resets all cards to idle state */

#define CF_CMD_master_mode    0x40   /* LBA & Master mode */
// #define CF_CMD_master_mode    0xa0   /* CHS & Master mode */
#define CF_CMD_slave_mode    0x50   /* LBA & Slave mode */
// #define CF_CMD_master_mode    0xb0   /* CHS & Master mode */
#if 0
#define CF_CMD_lba_mode    0x40   /* Resets all cards to idle state */
#endif

/* Multi-block test */
#define CF_MULTI_BLK           16 
#define CF_MAX_SECTOR         256

/* 
   Layer 1 Functions in driver
*/
s32 cfIdentifyDrive(u8 ucDriveNum);
s32 cfReadSectors(u32 unStartSector, u32 unSectorCount, u8 *dataBuf);
s32 cfReadMultipleSectors(u32 unStartSector, u32 unSectorCount, u8 *dataBuf);
s32 cfWriteSectors(u32 unStartSector, u32 unSectorCount, u8 *dataBuf);
s32 cfWriteMultipleSectors(u32 unStartSector, u32 unSectorCount, u8 *dataBuf);
s32 cfReadMultipleSectorsDMA(u32 unStartSector, u32 unSectorCount, u8 *dataBuf);
s32 cfWriteMultipleSectorsDMA(u32 unStartSector, u32 unSectorCount, u8 *dataBuf);
s32 cfEraseSectors(u32 unStartSector, u8 unSectorCount);
s32 cfSetTransferMode(u8 ucTransferMode, u8 ucModeType);
s32 cfInit(void);
s32 cfMount(void);
u32 cfErrHandle(u32 ucErrorCode, u8 *ucOutputMessage);

#endif
