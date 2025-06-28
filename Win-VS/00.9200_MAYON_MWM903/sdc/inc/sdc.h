/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	sdc.h

Abstract:

   	The structures and constants of SD Card.

Environment:

    	ARM RealView Developer Suite

Revision History:

    2007/03/25  Ven-Chin Chen   Modify
	2005/08/26	David Tsai	Create

*/

#ifndef __SDC_H__
#define __SDC_H__


/*****************************************************************************/
/* Constant Definition			                                     */
/*****************************************************************************/

/*
 * Command Index
 */
#define SDC_CMD_GO_IDLE_STATE           0 /* Resets all cards to idle state */
#define SDC_CMD_ALL_SEND_CID            2 /* Asks any card to send the CID numbers on the CMD line */
#define SDC_CMD_SEND_RELATIVE_ADDR      3 /* Asks the card to publish a new relative address (RCA) */
#define SDC_CMD_SET_DSR                 4 /* Programs the DSR of all cards */
#define SDC_CMD_SWITCH_FUNC             6 /* Checks switchable function (mode 0) and switch card function (mode 1) */
#define SDC_CMD_SELECT_DESELECT_CARD    7 /* Toggles a card between the stand-by and transfer states or between
											the programming and disconnect states */
#if SD_SPEC_2DOT0
#define SDC_CMD_SEND_IF_COND            8 /* (VCC) FOR Spec 2.0 */
#endif
#define SDC_CMD_SEND_CSD                9 /* Addressed card sends its card-specific data (CSD) on the CMD line */
#define SDC_CMD_SEND_CID               10 /* Addressed card sends its card identification (CID) on CMD the line */
#define SDC_CMD_STOP_TRANSMISSION      12 /* Forces the card to stop transmission */
#define SDC_CMD_SEND_STATUS            13 /* Addressed card sends its status register */
#define SDC_CMD_GO_INACTIVE_STATE      15 /* Sets the card to inactive state */
#define SDC_CMD_SET_BLOCKLEN           16 /* Sets the block length (in bytes) for all following block commands
											(read, write, lock )*/ /* (VCC $) Non-used */
#define SDC_CMD_READ_SINGLE_BLOCK      17 /* Reads a block of the size selected by the SET_BLOCKLEN command */
#define SDC_CMD_READ_MULTIPLE_BLOCK    18 /* Continuously transfers data blocks from card to host until interrupted
    										by a STOP_TRANSMISSION command. */
#define SDC_CMD_WRITE_BLOCK            24 /* Writes a block of the size selected by the SET_BLOCKLEN command */
#define SDC_CMD_WRITE_MULTIPLE_BLOCK   25 /* Continuously writes blocks of data until a STOP_TRANSMISSION follows */
#define SDC_CMD_PROGRAM_CSD            27 /* Programming of the programmable bits of the CSD */
#define SDC_CMD_SET_WRITE_PROT         28 /* Sets the write protection bit of the addressed group */
#define SDC_CMD_CLR_WRITE_PROT         29 /* Clears the write protection bit of the addressed group */
#define SDC_CMD_SEND_WRITE_PROT        30 /* Asks the card to send the status of the write protection bits */
#define SDC_CMD_ERASE_WR_BLK_START     32 /* Sets the address of the first write block to be erased */
#define SDC_CMD_ERASE_WR_BLK_END       33 /* Sets the address of the last write block of the continuous range
    										to be erased */
#define SDC_CMD_ERASE                  38 /* Erases all previously selected write blocks */
#define SDC_CMD_LOCK_UNLOCK            42 /* Used to set/reset the password or lock/unlock the card */
#define SDC_CMD_APP_CMD                55 /* Indicates to the card that the next command is an application specific
    										command */
#define SDC_CMD_GEN_CMD                56 /* Transfer a data block to the card or to get a data block from the card
    										for general purpose / application specific commands */

/*
 * Application Command Index
 */
#define SDC_ACMD_SET_BUS_WIDTH             6 /* Defines the data bus width ('00'?1bit or '10'=4 bits bus)
    											to be used for data transfer */
#define SDC_ACMD_SD_STATUS                13 /* Send the SD Memory Card status */
#define SDC_ACMD_SEND_NUM_WR_BLOCKS       22 /* Send the number of the written (without errors) write blocks */
#define SDC_ACMD_SET_WR_BLK_ERASE_COUNT   23 /* Set the number of write blocks to be pre-erased before writing
        										(to be used for faster Multiple Block WR command). '1'=default
                        						(one wr block) */
#define SDC_ACMD_SD_SEND_OP_COND          41 /* Asks the accessed card to send its operating condition register (OCR)
                        						content in the response on the CMD line */
#define SDC_ACMD_SET_CLR_CARD_DETECT      42 /* Connect[1]/Disconnect[0] the 50KOhm pull-up resistor on CD/DAT3
                        						(pin 1) of the card */
#define SDC_ACMD_SEND_SCR                 51 /* Reads the SD Configuration Register (SCR) */

/*
 * Card Status
 */
#define SDC_CS_OUT_OF_RANGE        0x80000000
#define SDC_CS_ADDRESS_ERROR       0x40000000
#define SDC_CS_BLOCK_LEN_ERROR     0x20000000
#define SDC_CS_ERASE_SEQ_ERROR     0x10000000
#define SDC_CS_ERASE_PARAM         0x08000000
#define SDC_CS_WP_VIOLATION        0x04000000
#define SDC_CS_CARD_IS_LOCKED      0x02000000
#define SDC_CS_LOCK_UNLOCK_FAILED  0x01000000
#define SDC_CS_COM_CRC_ERROR       0x00800000
#define SDC_CS_ILLEGAL_COMMAND     0x00400000
#define SDC_CS_CARD_ECC_FAILED     0x00200000
#define SDC_CS_CC_ERROR            0x00100000
#define SDC_CS_ERROR               0x00080000
#define SDC_CS_CSD_OVERWRITE       0x00010000
#define SDC_CS_WP_ERASE_SKIP       0x00008000
#define SDC_CS_CARD_ECC_DISABLED   0x00004000
#define SDC_CS_ERASE_RESET         0x00002000

#define SDC_CS_CURRENT_STATE   0x00001e00
#define SDC_CS_STAT_IDLE       0x00000000
#define SDC_CS_STAT_READY      0x00000200
#define SDC_CS_STAT_IDENT      0x00000400
#define SDC_CS_STAT_STBY       0x00000600
#define SDC_CS_STAT_TRAN       0x00000800
#define SDC_CS_STAT_DATA       0x00000a00
#define SDC_CS_STAT_RCV        0x00000c00
#define SDC_CS_STAT_PRG        0x00000e00
#define SDC_CS_STAT_DIS        0x00001000

#define SDC_CS_READY_FOR_DATA   0x00000100
#define SDC_CS_APP_CMD          0x00000020
#define SDC_CS_AKE_SEQ_ERROR    0x00000008

/*
 * OCR Register
 */
#define SDC_OCR_POWER_UP_BUSY   0x80000000

/*
 * CSD Register
 */
/* CSD3 = 127 -  96 */
/* CSD2 =  95 -  64 */
#define SDC_CSD2_READ_BL_LEN_MASK   0x000f0000
#define SDC_CSD2_READ_BL_LEN_SHFT           16
#if SD_SPEC_2DOT0
#define SDC_CSD2_2Dot0_DEVICE_SIZE_MASK   0x0000003f
#define SDC_CSD2_2Dot0_DEVICE_SIZE_SHFT           16
#endif
#define SDC_CSD2_C_SIZE_MASK        0x000003ff
#define SDC_CSD2_C_SIZE_SHFT                 2
/* CSD1 =  63 -  32 */
#if SD_SPEC_2DOT0
#define SDC_CSD1_2Dot0_DEVICE_SIZE_MASK   0xffff0000
#define SDC_CSD1_2Dot0_DEVICE_SIZE_SHFT           16
#endif
#define SDC_CSD1_C_SIZE_MASK        0xc0000000
#define SDC_CSD1_C_SIZE_SHFT                30
#define SDC_CSD1_C_SIZE_MULT_MASK   0x00038000 /* [49:47] */
#define SDC_CSD1_C_SIZE_MULT_SHFT           15
/* CSD0 =  31 -   0 */

// Task Evt
#define SDC_DEV_MAX_INT_EVT 32

enum
{
    SDC_TASK_EVT_MOUNT = 0,
    SDC_TASK_EVT_UNMOUNT,
    SDC_TASK_EVT_UNDEF,
};


/*****************************************************************************/
/* Structure Definition			                                     */
/*****************************************************************************/

/*
 * Response Type
 */

typedef __packed struct _SDC_R1
{
    u8 dummy[2];
    u8 crc7;
    u32 card_status;
    u8 command_index;
}
SDC_R1;

typedef __packed struct _SDC_R2_CID
{
    u8 dummy[3];
    u8 crc7;
    u16 manufacture_date;
    u32 product_serial_number;
    u8 product_revision;
    u8 product_name[5];
    u16 oem_id;
    u8 manufacture_id;
    u8 reserved;
}
SDC_R2_CID;

/* Modify if avilable */
typedef __packed struct _SDC_R2_CSD
{
    u8 dummy[3]; /* Bit 159 - 136 */
    u32 csd[4]; /* csd[3]: Bit 135 - 104, csd[2]: Bit 103 - 72, csd[1]: Bit 71 - 40, csd[0]: Bit 39 - 8 */
    u8 reserved; /* Bit 7 - 0 */
}
SDC_R2_CSD;

typedef __packed struct _SDC_R3_OCR
{
    u8 dummy[2];
    u8 reserved2;
    u32 ocr;
    u8 reserved1;
}
SDC_R3_OCR;

typedef __packed struct _SDC_R6_RCA
{
    u8 dummy[2];
    u8 crc7;
    u16 card_status;
    u16 rca;
    u8 command_index;
}
SDC_R6_RCA;

#if SD_SPEC_2DOT0
typedef __packed struct _SDC_R7
{
    u8 dummy[2];
    u8 crc7;
    u8 check_pattern;
    u8 voltage_accepted;
    u8 reserved[2];
    u8 command_index;
}
SDC_R7;
#endif

typedef struct _SDC_INT_EVT
{
    u8 cause[SDC_DEV_MAX_INT_EVT];   // cause of interrupt event
    u8 idxSet;							// index of set event
    u8 idxGet;							// index of get event
}
SDC_INT_EVT;


typedef enum card_type
{
    SD_CARD = 1,
    MMC_CARD
} CARD_TYPE_E;


#endif
