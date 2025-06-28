/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	smc.h

Abstract:

   	The structures and constants of SMC and NAND gate flash protocol.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __SMC_H__
#define __SMC_H__


/*****************************************************************************/
/* Structure Definition			                                     */
/*****************************************************************************/

/* SMC Redundant Area */
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL) ||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
typedef __packed struct _SMC_REDUN_AREA
{

    u8	data_status;
    u8	block_status;
    u16	block_addr1;
    u32	user_data;
    u8	ecc2[3];
    u16	block_addr2;
    u8	ecc1[3];
}
SMC_REDUN_AREA;

#elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
typedef __packed struct _SMC_REDUN_AREA_ADV
{

    u8	block_status;		/* in Adv NAND, the 1st byte in redundant area indicates the block status of the flash block */
    u8	data_status;
    u16	block_addr1;
    u32 user_data1;
    u32	user_data[6];
    u8	ecc2[3];
    u8	extra_data0;
    u8	ecc1[3];
    u8	extra_data1;
    u8	ecc4[3];
    u8	extra_data2;
    u8	ecc3[3];
    u8	extra_data3;
    u8	ecc6[3];
    u8	extra_data4;
    u8	ecc5[3];
    u8	extra_data5;
    u8	ecc8[3];
    u8	extra_data6;
    u8	ecc7[3];
    u8	extra_data7;
}
SMC_REDUN_AREA_ADV;
#endif


/*****************************************************************************/
/* Constant Definition			                                     */
/*****************************************************************************/
/* SMC Command Sets */

/* SMC Command Sets */
#define SMC_CMD_READ_1ST				0x00
#define SMC_CMD_READ_2ND				0x30
#define SMC_CMD_READ_1A				0x00
#define SMC_CMD_READ_1B				0x01
#define SMC_CMD_READ_2C				0x50
#define SMC_CMD_WRITE_2C				0x50
#define SMC_CMD_READ_ID				0x90
#define SMC_CMD_RESET				0xff
#define SMC_CMD_PAGE_PROGRAM_1ST		0x80
#define SMC_CMD_PAGE_PROGRAM_2ND		0x10
#define SMC_CMD_MULTI_PAGE_PROGRAM_1ST		0x80
#define SMC_CMD_MULTI_PAGE_PROGRAM_2ND		0x11
/* Copy-back program no support page size 512 Bytes */
#define SMC_CMD_COPY_BACK_PROGRAM_1ST		0x85
#define SMC_CMD_COPY_BACK_PROGRAM_2ND		0x10
#define SMC_CMD_COPY_BACK_READ_1ST		0x00
#define SMC_CMD_COPY_BACK_READ_2ND		0x35
#define SMC_CMD_MULTI_COPY_BACK_PROGRAM_1ST	0x03
#define SMC_CMD_MULTI_COPY_BACK_PROGRAM_2ND	0x8a
#define SMC_CMD_MULTI_COPY_BACK_PROGRAM_3RD	0x11
#define SMC_CMD_BLOCK_ERASE_1ST			0x60
#define SMC_CMD_BLOCK_ERASE_2ND			0xd0
#define SMC_CMD_MULTI_BLOCK_ERASE_1ST		0x60
#define SMC_CMD_MULTI_BLOCK_ERASE_2ND		0xd0
#define SMC_CMD_READ_STATUS			0x70
#define SMC_CMD_READ_MULTI_STATUS		0x71
#define SMC_CMD_LOCK				0x2a
#define SMC_CMD_UNLOCK_1ST			0x23
#define SMC_CMD_UNLOCK_2ND			0x24
#define SMC_CMD_LOCK_TIGHT			0x2c
#define SMC_CMD_READ_BLOCK_LOCK_STATUS		0x7a
#define SMC_CMD_BLOCK_ERASE_1ST			0x60
#define SMC_CMD_BLOCK_ERASE_2ND			0xd0
#define SMC_CMD_READ_STATUS			0x70
#define SMC_CMD_READ_EDC_STATUS		0x78

/* SMC Status Register */
#define SMC_SR_PASS				0x00
#define SMC_SR_FAIL				0x01

/* SMC Operation Completeness */
#define SMC_NOT_CMPL				0
#define SMC_CMPL				1


/* SMC Operation Result */
#define SMC_OP_PASS		1
#define SMC_OP_ECC_ERROR		2
#define SMC_OP_ECC_COMP		3

/* SMC Operation Status */
#define	SMC_OP_STAT_FAIL		0
#define	SMC_OP_STAT_PASS		1
#define	SMC_OP_STAT_SR_FAIL	2
#define	SMC_OP_STAT_SR_PASS	3
#define	SMC_OP_STAT_EDC_FAIL	4
#define	SMC_OP_STAT_EDC_PASS	5


/*****************************************
* Functions Prototype Declaration
*****************************************/
s8 smcVerifyDefultValue(void);
s8 smcVerifyBlockErase(u32);
void smcSetRedunAreaTest(u16, u32);

#endif
