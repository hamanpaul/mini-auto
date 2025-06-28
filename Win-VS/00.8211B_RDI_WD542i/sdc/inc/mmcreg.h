/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	mmcreg.h

Abstract:

   	The registers of MMC controller.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __MMC_REG_H__
#define __MMC_REG_H__

/*
 * SdcDmaTrig
 */

/*
 * SdcCfg
 */

/*
 * SdcCtrl0
 */
#define MMC_RSP_R1_R4_R5		0x00000000
#define MMC_RSP_R3			0x00000004
#define MMC_RSP_R2			0x00000008

#define MMC_CTRL0_CMD_RSP_R1		(SDC_CMD_RSP | MMC_RSP_R1_R4_R5)
#define MMC_CTRL0_CMD_RSP_R4		(SDC_CMD_RSP | MMC_RSP_R1_R4_R5)
#define MMC_CTRL0_CMD_RSP_R5		(SDC_CMD_RSP | MMC_RSP_R1_R4_R5)
#define MMC_CTRL0_CMD_RSP_R3		(SDC_CMD_RSP | MMC_RSP_R3)
#define MMC_CTRL0_CMD_RSP_R2		(SDC_CMD_RSP | MMC_RSP_R2)
#define MMC_CTRL0_CMD			(SDC_CMD)
#define MMC_CTRL0_CMD_RSP_R1_DAT_RD	(SDC_CMD_RSP_DAT_RD | MMC_RSP_R1_R4_R5 | SDC_RESET_FIFO | SDC_RCV_BLK_SZ_512)
#define MMC_CTRL0_CMD_RSP_R1_DAT_WR	(SDC_CMD_RSP_DAT_WR | MMC_RSP_R1_R4_R5 | SDC_RESET_FIFO | SDC_XMT_BLK_SZ_512)

/*
 * SdcCtrl1
 */

/*
 * SdcStat
 */

/*
 * SdcIntMask
 */

/*
 * SdcIntStat
 */


#endif
