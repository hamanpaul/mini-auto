/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	mmc.h

Abstract:

   	The structurs and constants of MMC.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __MMC_H__
#define __MMC_H__

/*****************************************************************************/
/* Structure Definition			                                     */
/*****************************************************************************/

/*
 * Response Type
 */

typedef __packed struct _MMC_R4_FIO
{
    u8	dummy[2];
    u8	crc7;
    u8	register_data;
    u8	register_addr;
    u16	rca;
    u8	command_index;
}
MMC_R4_FIO;

typedef __packed struct _MMC_R5_IRQ
{
    u8	dummy[2];
    u8	crc7;
    u16	rq_data;
    u16	rca;
    u8	command_index;
}
MMC_R5_IRQ;

/*****************************************************************************/
/* Constant Definition			                                     */
/*****************************************************************************/

/*
 * Command Index
 */

#define	MMC_CMD_GO_IDLE_STATE		0
#define MMC_CMD_SEND_OP_COND		1
#define MMC_CMD_ALL_SEND_CID		2
#define MMC_CMD_SET_RELATIVE_ADDR	3
#define MMC_CMD_SET_DSR			4
#define MMC_CMD_SELECT_DESELECT_CARD	7
#define	MMC_CMD_SEND_CSD		9
#define MMC_CMD_SEND_CID		10
#define MMC_CMD_READ_DAT_UNTIL_STOP	11
#define MMC_CMD_STOP_TRANSMISSION	12
#define MMC_CMD_SEND_STATUS		13
#define MMC_CMD_GO_INACTIVE_STATE	15
#define MMC_CMD_SET_BLOCKLEN		16
#define MMC_CMD_READ_SINGLE_BLOCK	17
#define MMC_CMD_READ_MULTIPLE_BLOCK	18
#define MMC_CMD_WRITE_DAT_UNTIL_STOP	20
#define MMC_CMD_WRITE_BLOCK		24
#define MMC_CMD_WRITE_MULTIPLE_BLOCK	25
#define MMC_CMD_PROGRAM_CID		26
#define MMC_CMD_PROGRAM_CSD		27
#define MMC_CMD_SET_WRITE_PROT		28
#define MMC_CMD_CLR_WRITE_PROT		29
#define MMC_CMD_SEND_WRITE_PROT		30
#define MMC_CMD_TAG_SECTOR_START	32
#define MMC_CMD_TAG_SECTOR_END		33
#define MMC_CMD_UNTAG_SECTOR		34
#define MMC_CMD_TAG_ERASE_GROUP_START	35
#define MMC_CMD_TAG_ERASE_GROUP_END	36
#define MMC_CMD_UNTAG_ERASE_GROUP	37
#define MMC_CMD_ERASE			38
#define MMC_CMD_FAST_IO			39
#define MMC_CMD_GO_IRQ_STATE		40
#define MMC_CMD_LOCK_UNLOCK		42
#define MMC_CMD_APP_CMD			55
#define MMC_CMD_GEN_CMD			56

/*
 * Card Status
 */
#define MMC_CS_OUT_OF_RANGE		0x80000000
#define MMC_CS_ADDRESS_ERROR		0x40000000
#define MMC_CS_BLOCK_LEN_ERROR		0x20000000
#define MMC_CS_ERASE_SEQ_ERROR		0x10000000
#define MMC_CS_ERASE_PARAM		0x08000000
#define MMC_CS_WP_VIOLATION		0x04000000
#define MMC_CS_CARD_IS_LOCKED		0x02000000
#define MMC_CS_LOCK_UNLOCK_FAILED	0x01000000
#define MMC_CS_COM_CRC_ERROR		0x00800000
#define MMC_CS_ILLEGAL_COMMAND		0x00400000
#define MMC_CS_CARD_ECC_FAILED		0x00200000
#define MMC_CS_CC_ERROR			0x00100000
#define MMC_CS_ERROR			0x00080000
#define MMC_CS_UNDERRUN			0x00040000
#define MMC_CS_OVERRUN			0x00020000
#define MMC_CS_CID_CSD_OVERWRITE	0x00010000
#define MMC_CS_WP_ERASE_SKIP		0x00008000
#define MMC_CS_CARD_ECC_DISABLED	0x00004000
#define MMC_CS_ERASE_RESET		0x00002000

#define MMC_CS_CURRENT_STATE		0x00001e00
#define MMC_CS_STAT_IDLE		0x00000000
#define MMC_CS_STAT_READY		0x00000200
#define MMC_CS_STAT_IDENT		0x00000400
#define MMC_CS_STAT_STBY		0x00000600
#define MMC_CS_STAT_TRAN		0x00000800
#define MMC_CS_STAT_DATA		0x00000a00
#define MMC_CS_STAT_RCV			0x00000c00
#define MMC_CS_STAT_PRG			0x00000e00
#define MMC_CS_STAT_DIS			0x00001000

#define MMC_CS_READY_FOR_DATA		0x00000100
#define MMC_CS_APP_CMD			0x00000020

#endif
