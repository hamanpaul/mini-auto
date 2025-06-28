/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	sdcreg.c

Abstract:

   	The registers of SD Card registers.

Environment:

    	ARM RealView Developer Suite

Revision History:

    2007/03/25  Ven-Chin Chen   Modify
	2005/08/26	David Tsai	Create

*/

#ifndef __SDC_REG_H__
#define __SDC_REG_H__

/*CY 0601 S*/
/*
 * SYS_CTL0 0xD00B:0x0000
 */
#define SYS_CTL0_SD_CK_EN   0x00020000


/*CY 0601 E*/

/*
 * SdcDmaTrig 0x0020
 *
 */
/* SD request trigger level, only valid at data read phase (1~8 words) */
#define SDC_DMA_TRIG_1W   0x00000000
#define SDC_DMA_TRIG_2W   0x00000001
#define SDC_DMA_TRIG_3W   0x00000002
#define SDC_DMA_TRIG_4W   0x00000003
#define SDC_DMA_TRIG_5W   0x00000004
#define SDC_DMA_TRIG_6W   0x00000005
#define SDC_DMA_TRIG_7W   0x00000006
#define SDC_DMA_TRIG_8W   0x00000007

#define SDC_DMA_TRIG_04W4R 0x00000303
#define SDC_DMA_TRIG_08W4R 0x00000307
#define SDC_DMA_TRIG_12W4R 0x0000030B
#define SDC_DMA_TRIG_16W4R 0x0000030F
#define SDC_DMA_TRIG_16W16R	0x00000f0f 
#define SDC_DMA_TRIG_20W4R 0x00000313
#define SDC_DMA_TRIG_24W4R 0x00000317
#define SDC_DMA_TRIG_28W4R 0x0000031B

#define SDC_DMA_INC0	0x00000000
#define SDC_DMA_INC4	0x00010000
#define SDC_DMA_INC8	0x00020000
#define SDC_DMA_INC16	0x00030000

/*
 * SdcCfg 0x0024
 */
#define SDC_ENA        0x00001000 /* SD/MMC card controller enable */

#define SDC_BUS_1BIT   0x00000000 /* 1-bit data bus width */
#define SDC_BUS_4BIT   0x00002000 /* 4-bit data bus width */

#define SDC_MMC        0x00000000 /* MMC card */
#define SDC_SD         0x00004000 /* SD card */

#define SDC_CLK_INV    0x00008000

/*
 * SdcCtrl0 0x0024
 */
#define SDC_CMD_RSP          0x00000000                             /* Command with response cycle */
#define SDC_CMD              0x00000001                             /* Command without response cycle */
#define SDC_CMD_RSP_DAT_RD   0x00000002                             /* Read data block cycle */
#define SDC_CMD_RSP_DAT_WR   0x00000003                             /* Write data block cycle */

#define SDC_RSP_R1_R6_7        0x00000000                           /* 6 bytes with CRC check */
#define SDC_RSP_R3           0x00000004                             /* 6 bytes without CRC check */
#define SDC_RSP_R2           0x00000008                             /* 17 byte without CRC check */

#define SDC_RESET_FIFO       0x00000010                             /* write 1 to reset FIFO */

#define SDC_RCV_BLK_SZ_512   0x0001ff00                             /* Receive data block size = 512 Bytes */
#define SDC_XMT_BLK_SZ_512   0x1ff00000                             /* Transmit data block size = 512 Bytes */
#define SDC_RCV_BLK_SZ_SCR   		0x00000700	/*CY 0907*/
#define SDC_RCV_BLK_SZ_SWITCH		0x00003f00

#define SDC_CTRL0_CMD_RSP_R1          (SDC_CMD_RSP | SDC_RSP_R1_R6_7) /* 6 bytes with CRC check */
#define SDC_CTRL0_CMD_RSP_R6          (SDC_CMD_RSP | SDC_RSP_R1_R6_7) /* 6 bytes with CRC check */
#define SDC_CTRL0_CMD_RSP_R7          (SDC_CMD_RSP | SDC_RSP_R1_R6_7) /* 6 bytes with CRC check */
#define SDC_CTRL0_CMD_RSP_R3          (SDC_CMD_RSP | SDC_RSP_R3)    /* 6 bytes without CRC check */
#define SDC_CTRL0_CMD_RSP_R2          (SDC_CMD_RSP | SDC_RSP_R2)    /* 17 byte without CRC check */
#define SDC_CTRL0_CMD                 (SDC_CMD)
#define SDC_CTRL0_CMD_RSP_R1_DAT_RD   (SDC_CMD_RSP_DAT_RD | SDC_RSP_R1_R6_7 | SDC_RESET_FIFO | SDC_RCV_BLK_SZ_512)
#define SDC_CTRL0_CMD_RSP_R1_DAT_WR   (SDC_CMD_RSP_DAT_WR | SDC_RSP_R1_R6_7 | SDC_RESET_FIFO | SDC_XMT_BLK_SZ_512)
#define SDC_CTRL0_CMD_RSP_R1_SCR_RD	(SDC_CMD_RSP_DAT_RD | SDC_RSP_R1_R6_7 | SDC_RESET_FIFO | SDC_RCV_BLK_SZ_SCR)	/*CY 0907*/
#define SDC_CTRL0_CMD_RSP_R1_SWITCH_RD	(SDC_CMD_RSP_DAT_RD | SDC_RSP_R1_R6_7 | SDC_RESET_FIFO | SDC_RCV_BLK_SZ_SWITCH)	/*CY 0907*/


/*
 * SdcCtrl1 0x002C
 */
#define SDC_CMD_START   0x00000001 /* Write 1 to the bit to start data transmits.
it will be clear after the data transmit start. */

/*
 * SdcStat 0x0030
 */
#define SDC_BUSY            0x00000001 /* SD/MMC card busy */
#define SDC_WRITE_PROTECT   0x00000002 /* Status of write protect switch of SD card */
#define SDC_DETECT          0x00000004 /* SD/MMC card detection Indicator */

#define SDC_CRC_STAT_MASK   0x00000038 /* SD/MMC card write block data CRC status */
#define SDC_XMT_SUCC        0x00000010 /* Transfer success */
#define SDC_XMT_ERR         0x00000028 /* Transfer error */
#define SDC_PRG_ERR         0x00000038 /* Programming error */

#define SDC_FIFO_EMPTY      0x00000040 /* FIFO status */
#define SDC_CARD_READY      0x00000080 /* SD Card Ready status */

/*
 * SdcIntMask 0x0034
 */
#define SDC_CMD_RSP_CMPL_INT_ENA               0x00000001 /* Command/Response cycle complete */
#define SDC_DAT_RX_ERR_INT_ENA                 0x00000002 /* Data Rx CRC error */
#define SDC_RSP_RX_ERR_INT_ENA                 0x00000004 /* Response Rx CRC error */
#define SDC_DAT_TX_ERR_INT_ENA                 0x00000008 /* Data Tx error */
#define SDC_DMA_DAT_TX_ERR_INT_ENA             0x00000010 /* DMA data Tx error */
#define SDC_DMA_DAT_RX_ERR_INT_ENA             0x00000020 /* DMA data Rx error */
#define SDC_DAT_WRITE_TO_SD_CARD_INT_ENA       0x00000040 /* Data Write to SD card interrupt */
#define SDC_DAT_MUL_WRITE_TO_SD_CARD_INT_ENA   0x00000080 /* Data Write to SD card interrupt */
#define SDC_CARD_DETECT_INTR_ENA               0x00000700 /* Card detect intr enable*/

/*
 * SdcIntStat 0x0038
 */
#define SDC_CMD_RSP_CMPL               0x00000001 /* Command/Response cycle complete */
#define SDC_DAT_RX_CRC_ERR             0x00000002 /* Data Rx CRC Error */
#define SDC_RSP_RX_CRC_ERR             0x00000004 /* Response Rx CRC Error */
#define SDC_DAT_TX_ERR                 0x00000008 /* Data Tx error */
#define SDC_DMA_DAT_TX_ERR             0x00000010 /* DMA data Tx Error */
#define SDC_DMA_DAT_RX_ERR             0x00000020 /* DMA data Rx Error */
#define SDC_DAT_WRITE_TO_SD_CARD       0x00000040 /* Data Write to SD card interrupt */
#define SDC_DAT_MUL_WRITE_TO_SD_CARD   0x00000080 /* Data Write to SD card interrupt */
#define SDC_CARD_DETECT                0x00000100 /* SDC Card detect intr*/

/*
 *
 */
#define OUTPW(port, value)   (*((volatile s32 *) (port)) = value)
#define INPW(port)           (*((volatile s32 *) (port)))

#define SDC_LASTREGOFFSET   0x38                        /*  */
#define SDC_REGNUM          (SDC_LASTREGOFFSET / 4 + 1) /* The number of SD Controller registers. */

#endif
