/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	mmc.c

Abstract:

   	The routines of MMC.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"
#include "board.h"

#include "sdcapi.h"
#include "sdc.h"
#include "sdcreg.h"
#include "mmc.h"
#include "mmcreg.h"
#include "dmaapi.h"
#include "fsapi.h"	/*CY 0718*/

/*
 *********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */
/* MMC clock divisor */
#define MMC_CLOCK_DIVISOR0		0x00000040
#define MMC_CLOCK_DIVISOR1		0x00000002

/* MMC time out value */
#define MMC_TIMEOUT			20	/*CY 1023*/

/* MMC initial OCR */
#define MMC_INIT_RCA			0x01000000
#define MMC_INIT_OCR			0x00ff8000

/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */
u32 mmcRca;

extern s32 sdcMmc; /* cytsai */
extern u32 sdcTotalBlockCount; /* cytsai */

extern s32 dcfStorageSize;	/*CY 0718*/

/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */
/* Driver Function */
extern s32 sdcDetectCard(void);
extern s32 sdcSetCommand(u32, u32, u32);
extern s32 sdcCheckCommandResponseComplete(void);
extern s32 sdcCheckDmaReadComplete(void);
extern s32 sdcCheckDmaWriteComplete(void);
extern s32 sdcCheckReadError(void);
extern s32 sdcCheckWriteError(void);
extern s32 sdcGetResponseR1(SDC_R1*);
extern s32 sdcGetResponseR2Cid(SDC_R2_CID*);
extern s32 sdcGetResponseR2Csd(SDC_R2_CSD*);
extern s32 sdcGetResponseR3Ocr(SDC_R3_OCR*);
extern s32 sdcGetResponseR6Rca(SDC_R6_RCA*);
extern s32 sdcSetReadDataDma(u8*, u32);
extern s32 sdcSetWriteDataDma(u8*, u32);

s32 mmcGetResponseR4Fio(MMC_R4_FIO*);
s32 mmcGetResponseR5Irq(MMC_R5_IRQ*);

/* Middleware Function */
s32 mmcCardIdentificationMode(void);
s32 mmcGetCsd(void);
s32 mmcDataTransferMode(u32);
s32 mmcErase(u32, u32);
s32 mmcReadSingleBlock(u32, u8*);
s32 mmcReadMultipleBlock(u32, u32, u8*);
s32 mmcWriteSingleBlock(u32, u8*);
s32 mmcWriteMultipleBlock(u32, u32, u8*);
s32 mmcGetCardStatus(u32*);
s32 mmcCheckTransferState(void);

/*
 *********************************************************************************************************
 *                                                Driver Function
 *********************************************************************************************************
 */

/*

Routine Description:

	Get the response R4 of FIO.

Arguments:

	rspR4Fio - The response R4 of FIO.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcGetResponseR4Fio(MMC_R4_FIO* rspR4Fio)
{
    u32* rspPtr = (u32*) ((void*) rspR4Fio);

    *rspPtr++ = (SdcRsp2 << 24);
    *rspPtr = SdcRsp1;

    return 1;
}

/*

Routine Description:

	Get the response R5 of IRQ.

Arguments:

	rspR5Irq - The response R5 of IRQ.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcGetResponseR5Irq(MMC_R5_IRQ* rspR5Irq)
{
    u32* rspPtr = (u32*) ((void*) rspR5Irq);

    *rspPtr++ = (SdcRsp2 << 24);
    *rspPtr   = SdcRsp1;

    return 1;
}

/*
 *********************************************************************************************************
 *                                                Middleware Function
 *********************************************************************************************************
 */

/*

Routine Description:

	Card identification mode of MMC.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcCardIdentificationMode(void)
{
    s32 i;
    u32 temp = 0;
    SDC_R1 rspR1;
    SDC_R3_OCR rspR3Ocr;
    SDC_R2_CID rspR2Cid;

    /* MMC configuration */
    SdcCfg = MMC_CLOCK_DIVISOR0 | SDC_ENA | SDC_BUS_1BIT | SDC_MMC;

    /* initialization delay to supply ramp up time = max(1 msec, 74 clock cycles) */
    for (i = 0; i < 0xffff; i++)
        temp++;

    /* CMD0 */
    sdcSetCommand(MMC_CMD_GO_IDLE_STATE, 0, MMC_CTRL0_CMD);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;

    do
    {
        /* CMD1 */
        sdcSetCommand(MMC_CMD_SEND_OP_COND, MMC_INIT_OCR, MMC_CTRL0_CMD_RSP_R3);
        /* check command/response completion */
        if (sdcCheckCommandResponseComplete() != 1)
            return 0;
        /* get response */
        sdcGetResponseR3Ocr(&rspR3Ocr);
    }
    while ((rspR3Ocr.ocr & SDC_OCR_POWER_UP_BUSY) == 0);

    /* CMD2 */
    sdcSetCommand(MMC_CMD_ALL_SEND_CID, 0, MMC_CTRL0_CMD_RSP_R2);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR2Cid(&rspR2Cid);

    /* CMD3 */
    mmcRca = MMC_INIT_RCA;
    sdcSetCommand(MMC_CMD_SET_RELATIVE_ADDR, mmcRca, MMC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* get CSD */
    mmcGetCsd();

    return 1;
}

/*

Routine Description:

	Get CSD of MMC.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcGetCsd(void)
{
    SDC_R2_CSD rspR2Csd;
    u32 capacity, block_num, mult, block_len;

    /* CMD9 */
    sdcSetCommand(MMC_CMD_SEND_CSD, mmcRca, MMC_CTRL0_CMD_RSP_R2);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR2Csd(&rspR2Csd);

    /* calculate volume capacity */
    block_len = 1 << ((rspR2Csd.csd[2] & SDC_CSD2_READ_BL_LEN_MASK) >> SDC_CSD2_READ_BL_LEN_SHFT);
    mult = 1 << (((rspR2Csd.csd[1] & SDC_CSD1_C_SIZE_MULT_MASK) >> SDC_CSD1_C_SIZE_MULT_SHFT) + 2);
    block_num = ( ((rspR2Csd.csd[2] & SDC_CSD2_C_SIZE_MASK) << SDC_CSD2_C_SIZE_SHFT) +
                  ((rspR2Csd.csd[1] & SDC_CSD1_C_SIZE_MASK) >> SDC_CSD1_C_SIZE_SHFT) +
                  1 ) * mult;
    capacity = block_num * block_len;
    DEBUG_SDC("Trace: MMC capacity = %d bytes.\n", capacity);
    /*CY 0718*/
    /* select format size */
    if (capacity > (1000000000 - 256000000))	/* 256MB tolerance for 1GB */
    {
        dcfStorageSize = FS_MEDIA_MMC_1GB;
    }
    else if (capacity > (512000000 - 128000000))	/* 128MB tolerance for 512MB */
    {
        dcfStorageSize = FS_MEDIA_MMC_512MB;
    }
    else if (capacity > (256000000 - 64000000))	/* 64MB tolerance for 256MB */
    {
        dcfStorageSize = FS_MEDIA_MMC_256MB;
    }
    else if (capacity > (128000000 - 32000000))	/* 32MB tolerance for 128MB */
    {
        dcfStorageSize = FS_MEDIA_MMC_128MB;
    }
    else if (capacity > (64000000 - 16000000))	/* 16MB tolerance for 64MB */
    {
        dcfStorageSize = FS_MEDIA_MMC_64MB;
    }
    else if (capacity > (32000000 - 8000000))	/* 8MB tolerance for 32MB */
    {
        dcfStorageSize = FS_MEDIA_MMC_32MB;
    }
    else if (capacity > (16000000 - 4000000))	/* 4MB tolerance for 16MB */
    {
        dcfStorageSize = FS_MEDIA_MMC_16MB;
    }
    else
    {
        dcfStorageSize = 0;
    }
    /* save number of total block */
    sdcTotalBlockCount = block_num;	/* cytsai */
    /* check block length */
    if (block_len != MMC_BLK_LEN)
        return 0;

    return 1;
}

/*

Routine Description:

	Data transfer mode of MMC.

Arguments:

	busWidth - The width of data bus.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcDataTransferMode(u32 busWidth)
{
    SDC_R1 rspR1;

    /* CMD13 */
    sdcSetCommand(MMC_CMD_SEND_STATUS, mmcRca, MMC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* change from standby state to transfer state */
    if ((rspR1.card_status & MMC_CS_CURRENT_STATE) == MMC_CS_STAT_STBY)
    {
        /* CMD7 */
        sdcSetCommand(MMC_CMD_SELECT_DESELECT_CARD, mmcRca, MMC_CTRL0_CMD_RSP_R1);
        /* check command/response completion */
        if (sdcCheckCommandResponseComplete() != 1)
            return 0;
        /* get response */
        sdcGetResponseR1(&rspR1);
    }

    /* CMD13 */
    sdcSetCommand(SDC_CMD_SEND_STATUS, mmcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);
    /* check if in tran state */
    if ((rspR1.card_status & SDC_CS_CURRENT_STATE) != SDC_CS_STAT_TRAN)
        return 0;

    /* MMC configuration */
    SdcCfg = MMC_CLOCK_DIVISOR1 | SDC_ENA | SDC_BUS_1BIT | SDC_MMC;

    return 1;
}

/*

Routine Description:

	Erase blocks.

Arguments:

	startAddr - The start block address to erase.
	endAddr - The end block address to erase.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcErase(u32 startAddr, u32 endAddr)
{
    SDC_R1 rspR1;

    /* CMD32 */
    sdcSetCommand(MMC_CMD_TAG_SECTOR_START, startAddr, MMC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* CMD33 */
    sdcSetCommand(MMC_CMD_TAG_SECTOR_END, endAddr, MMC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* CMD38 */
    sdcSetCommand(MMC_CMD_ERASE, 0, MMC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);

    return 1;
}

/*

Routine Description:

	Read single block.

Arguments:

	addr - The block address to read.
	dataBuf - The buffer to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcReadSingleBlock(u32 addr, u8* dataBuf)
{
    SDC_R1 rspR1;

    /* CMD17 */
    sdcSetCommand(MMC_CMD_READ_SINGLE_BLOCK, addr, MMC_CTRL0_CMD_RSP_R1_DAT_RD);
    /* set read data dma */
    sdcSetReadDataDma(dataBuf, MMC_BLK_LEN);
    /* check command/response completion */
    //if (sdcCheckCommandResponseComplete() != 1) /* cytsai: the controller does not signal command/response completion */
    //	return 0;
    /* check dma completion */
    if (sdcCheckDmaReadComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);
    /* check read error */
    if (sdcCheckReadError() != 1)
        return 0;

    return 1;
}

/*

Routine Description:

	Read multiple blocks.

Arguments:

	count - The number of blocks.
	addr - The start block address of the multiple blocks.
	dataBuf - The buffer to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcReadMultipleBlock(u32 count, u32 addr, u8* dataBuf)
{
    SDC_R1 rspR1;
    u32 i;

    for (i = 0; i < count; i++)
    {
        /* CMD18 */
        sdcSetCommand(MMC_CMD_READ_MULTIPLE_BLOCK, addr, MMC_CTRL0_CMD_RSP_R1_DAT_RD);
        /* set read data dma */
        sdcSetReadDataDma(dataBuf, MMC_BLK_LEN);
        /* check command/response completion */
        //if (sdcCheckCommandResponseComplete() != 1) /* cytsai: the controller does not signal command/response completion */
        //	return 0;
        /* check dma complete */
        if (sdcCheckDmaReadComplete() != 1)
            return 0;
        /* get response */
        sdcGetResponseR1(&rspR1);
        /* check read error */
        if (sdcCheckReadError() != 1)
            return 0;

        /* advance pointer of data buffer */
        dataBuf += MMC_BLK_LEN;
    }

    /* CMD12 */
    sdcSetCommand(MMC_CMD_STOP_TRANSMISSION, 0, MMC_CTRL0_CMD_RSP_R1);
    /* check command response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);

    return 1;
}

/*

Routine Description:

	Write single block.

Arguments:

	addr - The block address to write.
	dataBuf - The buffer to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcWriteSingleBlock(u32 addr, u8* dataBuf)
{
    SDC_R1 rspR1;

    /* check write protect */
#if SDC_WRPROT_ENA    
    if (SdcStat & SDC_WRITE_PROTECT)
        return 0;
#endif

    /* CMD24 */
    sdcSetCommand(MMC_CMD_WRITE_BLOCK, addr, MMC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* write data block cycle */
    sdcSetCommand(MMC_CMD_WRITE_BLOCK, addr, MMC_CTRL0_CMD_RSP_R1_DAT_WR);
    /* set write data dma */
    sdcSetWriteDataDma(dataBuf, MMC_BLK_LEN);
    /* check dma complete */
    if (sdcCheckDmaWriteComplete() != 1)
        return 0;
    /* check write error */
    if (sdcCheckWriteError() != 1)
        return 0;

    return 1;
}

/*

Routine Description:

	Write multiple blocks.

Arguments:

	count - The number of blocks.
	addr - The start block address of multiple blocks.
	dataBuf - The buffer to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcWriteMultipleBlock(u32 count, u32 addr, u8* dataBuf)
{
    SDC_R1 rspR1;
    u32 i;

    /* check write protect */
#if SDC_WRPROT_ENA        
    if (SdcStat & SDC_WRITE_PROTECT)
        return 0;
#endif

    for (i = 0; i < count; i++)
    {
        /* CMD25 */
        sdcSetCommand(MMC_CMD_WRITE_MULTIPLE_BLOCK, addr, MMC_CTRL0_CMD_RSP_R1);
        /* check command/response completion */
        if (sdcCheckCommandResponseComplete() != 1)
            return 0;
        /* get response */
        sdcGetResponseR1(&rspR1);

        /* write data block cycle */
        sdcSetCommand(MMC_CMD_WRITE_MULTIPLE_BLOCK, addr, MMC_CTRL0_CMD_RSP_R1_DAT_WR);
        /* set write data dma */
        sdcSetWriteDataDma(dataBuf, MMC_BLK_LEN);
        /* check dma complete */
        if (sdcCheckDmaWriteComplete() != 1)
            return 0;
        /* check write error */
        if (sdcCheckWriteError() != 1)
            return 0;

        /* advance pointer of data buffer */
        dataBuf += MMC_BLK_LEN;
    }

    /* CMD12 */
    sdcSetCommand(MMC_CMD_STOP_TRANSMISSION, 0, MMC_CTRL0_CMD_RSP_R1);
    /* check command response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);

    return 1;
}

/*

Routine Description:

	Get card status.

Arguments:

	pStatus - Card status.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 mmcGetCardStatus(u32* pStatus)
{
    SDC_R1 rspR1;

    /* CMD13 */
    sdcSetCommand(MMC_CMD_SEND_STATUS, mmcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 0;
    /* get response */
    sdcGetResponseR1(&rspR1);
    *pStatus = rspR1.card_status;

    return 1;
}

/*

Routine Description:

	Check if card is in tranfer state.

Arguments:

	None.

Return Value:

	0 - Card not in transfer state.
	1 - Card in transfer state

*/
s32 mmcCheckTransferState(void)
{
    u32 status;

    if (mmcGetCardStatus(&status) == 0)
        return 0;

    if ((status & MMC_CS_CURRENT_STATE) != MMC_CS_STAT_TRAN)
        return 0;

    return 1;
}
