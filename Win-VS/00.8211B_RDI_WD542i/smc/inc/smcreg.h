/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	smcreg.h

Abstract:

   	The registers of SMC and NAND gate flash controller.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __SMC_REG_H__
#define __SMC_REG_H__

/*CY 0601 S*/
/* SYS_CTL0 */
#define SYS_CTL0_SMC_CK_EN              0x00040000


/*CY 0601 E*/

/* SmcCfg */
#define SMC_CFG_READ			0x00000000
#define SMC_CFG_PAGE_PROGRAM		0x00000001
#define SMC_CFG_MULTI_PAGE_PROGRAM	0x00000002
#define SMC_CFG_BLOCK_ERASE		0x00000003
#define SMC_CFG_MULTI_BLOCK_ERASE	0x00000004
#define SMC_CFG_READ_ID1		0x00000005
#define SMC_CFG_READ_ID2		0x00000006
#define SMC_CFG_CMD_ONLY		0x00000007

#define SMC_CFG_ADDR_1CYCL		0x00000000
#define SMC_CFG_ADDR_2CYCL		0x00000008
#define SMC_CFG_ADDR_3CYCL		0x00000010
#define SMC_CFG_ADDR_4CYCL		0x00000018

#define SMC_CFG_NAND			0x00000000
#define SMC_CFG_SMC			0x00000020

#define SMC_CFG_WP_LO			0x00000000
#define SMC_CFG_WP_HI			0x00000040

#define SMC_CFG_ECC_ENA			0x00000080
#define SMC_CFG_ECC_DISA		0x00000000
/*xx3fffxx ~xx0000xx */
#define SMC_CFG_DATA_LEN_16B		0x00000400	/* 0x04  = 4 Word = (16 Byte / 4)  */
#define SMC_CFG_DATA_LEN_64B		0x00001000	/* 0x10  = 16 Word = (64  Byte / 4)  */
#define SMC_CFG_DATA_LEN_256B		0x00004400	/* 0x44  = 68 Word = (256+16) Byte / 4  */
#define SMC_CFG_DATA_LEN_512B		0x00008400	/* 0x84  = 132 Word = (512+16) Byte / 4  */
#define SMC_CFG_DATA_LEN_2048B		0x00021000	/* 0x210 = 528 Word = (2048+64) Byte / 4 */

#define SMC_CFG_DATA_LEN_4096B		0x00042000	/* 0x420 = 1056 Word = (4096+128) Byte / 4 */
#define SMC_CFG_DATA_LEN_8192B		0x00086D00	/* 0x86D = 2157 Word = (8192+436) Byte / 4 */

#define PAGE_MODE_2k				0x00000000
#define PAGE_MODE_4k				0x02000000
#define PAGE_MODE_8k				0x04000000

#define SMC_CFG_WRITE				0x00000001
#define SMC_CFG_CMD1				0x00000003
#define SMC_CFG_CMD2				0x00000004
#define SMC_CFG_CMD_STATUS		0x00000006

#define SMC_CFG_RS_CODES_EN			0x08000000
#define SMC_CFG_RST_ECC_STAT		0x10000000
#define SMC_CFG_ADDR_5CYCL_EN 	0x20000000
#define SMC_CFG_WAIT_BUSY_EN		0x40000000
#define SMC_CFG_ADV_EN				0x80000000




/* SmcTimeCtrl */

/* SmcCtrl */
#define SMC_CTRL_OP_START		0x00000001
#define SMC_CTRL_REDUN_CMD_TRIG	0x00000004

/* SmcCmd */
#define SMC_CMD_1ST_SHFT		0
#define SMC_CMD_2ND_SHFT		8
#define SMC_CMD_3RD_SHFT		16
#define SMC_CMD_4th_SHFT		24
/* SmcAddr */
#define SMC_ADDR_1ST_SHFT		0
#define SMC_ADDR_2ND_SHFT		8
#define SMC_ADDR_3RD_SHFT		16
#define SMC_ADDR_4TH_SHFT		24

/* SmcStat */
#define SMC_STAT_REG_MASK		0x000000ff

#define SMC_STAT_MAK_COD_MASK		0x0000ff00
#define SMC_STAT_DEV_COD_MASK		0x00ff0000

#define SMC_STAT_BUSY			0x00000000
#define SMC_STAT_READY			0x01000000

#define SMC_STAT_CARD_DETECT		0x02000000

#define SMC_STAT_ECC1_CORRECT		0x04000000
#define SMC_STAT_ECC2_CORRECT		0x08000000
#define SMC_STAT_ECC3_CORRECT		0x10000000
#define SMC_STAT_ECC4_CORRECT		0x20000000

#define SMC_STAT_FSM_STAT_IDLE			0x00000000
#define SMC_STAT_FSM_STAT_BUSY		0x80000000

/* SmcData */

/* SmcIntMask */
#define SMC_INT_OP_CMPL_ENA		0x00000001
#define SMC_INT_READ_CMPL_ENA		0x00000002
#define SMC_INT_ECC_ERR_ENA		0x00000004
#define SMC_INT_ECC_COMP_ENA		0x00000008

/* SmcIntStat */
#define SMC_INT_OP_CMPL			0x00000001
#define SMC_INT_READ_CMPL			0x00000002
#define SMC_INT_ECC_ERR			0x00000004
#define SMC_INT_ECC_COMP			0x00000008

/* SmcRedunA0 */
/* SmcRedunA1 */
/* SmcRedunA2 */
/* SmcRedunA3 */

/* SmcEcc1 */
/* SmcEcc2 */
/* SmcCheErrNum */
#define SMC_ErrNum_1ST_Mak		0x00000003
#define SMC_ErrNum_2ND_Mak		0x0000000C
#define SMC_ErrNum_3RD_Mak		0x00000030
#define SMC_ErrNum_4th_Mak		0x000000C0
#define SMC_ErrNum_5th_Mak		0x00000300
#define SMC_ErrNum_6th_Mak		0x00000C00
#define SMC_ErrNum_7th_Mak		0x00003000
#define SMC_ErrNum_8th_Mak		0x0000C000
#define SMC_ErrNum_9th_Mak		0x00030000
#define SMC_ErrNum_10th_Mak		0x000C0000	
#define SMC_ErrNum_11th_Mak		0x00300000	
#define SMC_ErrNum_12th_Mak		0x00C00000
#define SMC_ErrNum_13th_Mak		0x03000000	
#define SMC_ErrNum_14th_Mak		0x0C000000	
#define SMC_ErrNum_15th_Mak		0x30000000	
#define SMC_ErrNum_16th_Mak		0xC0000000
/* SmcCheErrAddr */
#endif
