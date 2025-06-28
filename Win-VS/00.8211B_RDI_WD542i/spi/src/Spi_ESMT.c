/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	smc.c

Abstract:

	The routines of SMC and NAND gate flash.

Environment:

		ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"
#if (/*(FLASH_OPTION == FLASH_SERIAL_ESMT)||*/(FLASH_OPTION == FLASH_SERIAL_SST))
#include "board.h"
#include "osapi.h"
#include "spi.h"
#include "spireg.h"
//#include "dmaapi.h"
#include <../inc/mars_controller/mars_dma.h>
#include "fsapi.h"	/* cytsai: 0315 */
#include "rtcapi.h"
#include "dcfapi.h"
#include "stdio.h"
#include "gpioapi.h"
#include "spiapi.h"
#include "sysapi.h"



/*
 *********************************************************************************************************
 *												 CONSTANTS
 *********************************************************************************************************
 */

/* SPI time out value */
#define SPI_TIMEOUT				40
#define BUF_SIZE			256

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

__align(4) u8 	spiReadBuf[BUF_SIZE];
__align(4) u8 	spiWriteBuf[BUF_SIZE];

u8 	SpiInited = 0;
u32 	spiClkDiv;
u8 	cnt = 0;
u8	ucSpiTestResult[8] = {0};
u32	unAddrForTest;


u32 spiTotalSize;
u32	spiBlockSize;
u32	spiSectorSize;
u32	spiPageSize;
u8	spiManufID, spiDevID;


OS_EVENT* spiDmaEndSemEvt;
OS_EVENT* spiRXOvrSemEvt;
OS_EVENT* spiBusFinSemEvt;
OS_EVENT* spiTXEmptySemEvt;
OS_EVENT* spiRDSRSemEvt;
extern OS_EVENT* dmaSemForSPIRead;
u32 guiSPIFlashReadDMAId=0xFF, guiSPIFlashWriteDMAId=0xFF;
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Driver function
 *********************************************************************************************************
 */

/*
 ****************************
 * Funcstions For Verification
 ****************************
*/


/*

Routine Description:

	SPI Semaphore routine.

Arguments:

	ucSemFlg - Semaphore flag.
	ucCmdIdx -  index of commands.

Return Value:

	1 - success
	0 - failed

*/
s32 spiSemProcess(u8 ucSemFlg, u8 ucCmdIdx)
{
    u8 err;

    switch (ucSemFlg)
    {
    case SPI_SEM_FLG_BUSY:
        //OSSemPend(spiBusySemEvt, SPI_TIMEOUT, &err);
        break;

    case SPI_SEM_FLG_DMA_END:
        OSSemPend(spiDmaEndSemEvt, SPI_TIMEOUT, &err);
        break;

    case SPI_SEM_FLG_RX_OVR:
        OSSemPend(spiRXOvrSemEvt, SPI_TIMEOUT, &err);
        break;

    case SPI_SEM_FLG_BUS_FIN:
        OSSemPend(spiBusFinSemEvt, SPI_TIMEOUT, &err);
        break;

    case SPI_SEM_FLG_TX_EMPTY:
        OSSemPend(spiTXEmptySemEvt, SPI_TIMEOUT, &err);
        break;

    }


    if (err != OS_NO_ERR)
    {

        switch (ucCmdIdx)
        {
        case SPI_SEM_CMD_IDX_READID:
            DEBUG_SPI("Error: [spiREADID] SemEvt is %d.\n", err);
            break;

        case SPI_SEM_CMD_IDX_WRSR:
            DEBUG_SPI("Error: [spiWRSR] SemEvt is %d.\n", err);
            break;

        case SPI_SEM_CMD_IDX_WREN:
            DEBUG_SPI("Error: [spiWREN] SemEvt is %d.\n", err);
            break;

        case SPI_SEM_CMD_IDX_RDSR:
            DEBUG_SPI("Error: [spiRDSR] SemEvt is %d.\n", err);
            break;

        case SPI_SEM_CMD_IDX_SECTOR_ERASE:
            DEBUG_SPI("Error: [spiSECTOR_ERASE] SemEvt is %d.\n", err);
            break;

        case SPI_SEM_CMD_IDX_BLOCK_ERASE:
            DEBUG_SPI("Error: [spiBLOCK_ERASE] SemEvt is %d.\n", err);
            break;

        case SPI_SEM_CMD_IDX_CHIP_ERASE:
            DEBUG_SPI("Error: [spiCHIP_ERASE] SemEvt is %d.\n", err);
            break;

        case SPI_SEM_CMD_IDX_BYTE_PROGRAM:
            DEBUG_SPI("Error: [spiBYTE_PROGRAM] SemEvt is %d.\n", err);
            break;

        case SPI_SEM_CMD_IDX_AAI_PROGRAM:
            DEBUG_SPI("Error: [spiAAI_PROGRAM] SemEvt is %d.\n", err);
            break;

        }

        return 0;
    }

    return 1;

}


/*

Routine Description:

	The IRQ handler of SPI controller.

Arguments:

	None.

Return Value:

	None.

*/
void spiIntHandler(void)
{
    u32 spiIntStat = SpiStat;

    SpiStat &= 0x0000000C0;	/* write 1 to clear status */

    if (spiIntStat & SPI_SR_DMA_END)
    {
        SpiIntEn &= ~SPI_INT_DMA_END_EN;
        OSSemPost(spiDmaEndSemEvt);
    }

    if (spiIntStat & SPI_SR_RX_OVR)
    {
        SpiIntEn &= ~SPI_INT_RX_OVR_EN;
    }

    if (spiIntStat & SPI_SR_BUS_FIN)
    {
        SpiIntEn &= ~SPI_INT_BUS_FIN_EN;
        OSSemPost(spiBusFinSemEvt);
    }

    if (spiIntStat & SPI_SR_TX_EMPTY)
    {
        SpiIntEn &= ~SPI_INT_TX_EMPTY_EN;
    }



}
/*

Routine Description:

	Init SPI controller.

Arguments:

	None

Return Value:

	None

*/
void spiInit(void)
{

    if (SpiInited == 0)
    {
        SpiInited = 1;

#if(SYS_CPU_CLK_FREQ == 24000000)
        spiClkDiv = 0;

        DEBUG_SPI("System Freq = 24 MHz.\n");
        DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (24/2/(spiClkDiv +1)));

#elif(SYS_CPU_CLK_FREQ == 32000000)
        spiClkDiv = 0;

        DEBUG_SPI("System Freq = 32 MHz.\n");
        DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (32/2/(spiClkDiv +1)));


#elif(SYS_CPU_CLK_FREQ == 48000000)
        spiClkDiv = 0;

        DEBUG_SPI("System Freq = 48 MHz.\n");
        DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (48/2/(spiClkDiv +1)));

#elif(SYS_CPU_CLK_FREQ == 96000000)	/* for PA9003 */
        spiClkDiv = 0;
        DEBUG_SPI("System Freq = 96 MHz.\n");
        DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (96/2/(spiClkDiv +1)));

#elif(SYS_CPU_CLK_FREQ == 108000000)	/* for PA9003 */
        spiClkDiv = 0;
        DEBUG_SPI("System Freq = 108 MHz.\n");
        DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (108/2/(spiClkDiv +1)));

#elif(SYS_CPU_CLK_FREQ == 160000000)	/* for PA9003 */
        spiClkDiv = 1;
        DEBUG_SPI("System Freq = 160 MHz.\n");
        DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (160/2/(spiClkDiv +1)));
		        
#elif(SYS_CPU_CLK_FREQ == 192000000)
        spiClkDiv = 1;
        DEBUG_SPI("System Freq = 192 MHz.\n");
        DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (192/2/(spiClkDiv +1)));


#endif

        SYS_CTL0 |= SYS_CTL0_SPI_CK_EN;
#if(CHIP_OPTION == CHIP_PA9002D)
        GpioActFlashSelect = GPIO_ACT_FLASH_SPI;
#else
        GpioActFlashSelect = (GpioActFlashSelect | GPIO_ACT_FLASH_SPI) & (~GPIO_ACT_FLASH_SD);
#endif

        if (spiDmaEndSemEvt == NULL)
            spiDmaEndSemEvt = OSSemCreate(0);

        if (spiRXOvrSemEvt == NULL)
            spiRXOvrSemEvt = OSSemCreate(0);

        if (spiBusFinSemEvt == NULL)
            spiBusFinSemEvt = OSSemCreate(0);

        if (spiTXEmptySemEvt == NULL)
            spiTXEmptySemEvt = OSSemCreate(0);

        if (spiRDSRSemEvt == NULL)
            spiRDSRSemEvt = OSSemCreate(1);


    }

}

/*

Routine Description:

	Check if dma is completed.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 spiCheckDmaComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSPIFlashReadDMAId);
    guiSPIFlashReadDMAId = 0x55;
    return (err);
    /*
    u8 err;

    OSSemPend(dmaSemChFin[DMA_CH_SPI], SPI_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SPI("Error: dmaSemChFin[DMA_CH_SPI] is %d.\n", err);
        DEBUG_SPI("Read DmaCh3CycCnt = 0x%8x\n", DmaCh3CycCnt);
        return 0;
    }

    switch (dmaChResp[DMA_CH_SPI])
    {
    case DMA_CH_FINISH:
        return 1;

    case DMA_CH_ERROR:
        DEBUG_SPI("Error: dmaChResp[DMA_CH_SPI] is channel error.\n");
        return -1;

    default:
        DEBUG_SPI("Error: dmaChResp[DMA_CH_SPI] is error.\n");
        return -2;
    }*/
}

/*

Routine Description:

	Set read data dma.

Arguments:

	buf - The buffer to read to.
	siz - The size to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 spiSetReadDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)&(SpiRxData);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_READ, &dmaCfg) == 0)
    //    return 0;
    guiSPIFlashReadDMAId = marsDMAReq(DMA_REQ_SPI_READ, &dmaCfg);
    
    return 1;
}

/*

Routine Description:

	SPI Read routine.

Arguments:

	unAddr - the data want to be read out at the addr.
	pucDstBuf - the buffer where put data
	unSize - read length (byte)

Return Value:

	1 - success
	0 - failed

*/
s32 spiRead(u8* pucDstBuf, u32 unAddr, u32 unSize)
{
    u32 unData;
    u8 err;


    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_32BIT|
              SPI_EN;

    SpiIntEn = SPI_INT_BUS_FIN_EN|
               SPI_INT_DMA_END_EN;

    SpiCtrl |= SPI_SSOE_EN;		/* Set CE# low */

    SpiTxData = ( ((u32) SPI_CMD_READ) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
    unData = SpiRxData;		/* read to clear it */

    //if ( ((SYS_CPU_CLK_FREQ == 48000000) && (spiClkDiv == 0)) || ((SYS_CPU_CLK_FREQ == 96000000) && (spiClkDiv == 1)))
    if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
        SpiCtrl |= SPI_CPHA_HIGH;


    spiSetReadDataDma(pucDstBuf, unSize);
    SpiDmaLen = unSize/ 4;
    SpiCtrl |=SPI_DMA_EN;

    spiSemProcess(SPI_SEM_FLG_DMA_END, SPI_SEM_CMD_IDX_READ);

    spiCheckDmaComplete();
#if 0
    OSSemPend(dmaSemForSPIRead, SPI_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SPI("Error: [dmaSemForSPIRead] dmaSemForSPIRead is %d.\n", err);
        DEBUG_SPI("Read DmaCh3CycCnt = 0x%8x\n", DmaCh3CycCnt);
        return 0;
    }
#endif
    SpiCtrl &= ~SPI_DMA_EN;
    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high */

    return 1;

}

/*

Routine Description:

	SPI Read ID routine.

Arguments:

	none
Return Value:

	return ID.

*/
u32 spiReadID(void)
{

    u32 spiID;
    u32 unDummyAddr = 0x00000000;
    u8 i;

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_32BIT|
              SPI_EN;

    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;		/* Set CE# low */

    /* execute Tx cmd twice to match ESMT serial flash Read ID control property */
    for (i=0; i<2; i++)
    {
        {
            u32	temp;
            temp = SpiStat;		/* read to clear interrupt status */
        }

        SpiTxData = ( ((u32) SPI_CMD_READ_ID) << 24)| unDummyAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

        spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READID);
        spiID = SpiRxData;		/* Read-out SpiRxData to clear it. The second time we read is the correct one. */

        if (i==0)
            SpiIntEn = SPI_INT_BUS_FIN_EN;

        /* change the polarity to be negative edge when read data */
        //if ( ((SYS_CPU_CLK_FREQ == 48000000) && (spiClkDiv == 0)) || ((SYS_CPU_CLK_FREQ == 96000000) && (spiClkDiv == 1)))
        if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
            SpiCtrl |= SPI_CPHA_HIGH;

    }
    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to complete one process */

    return spiID;

}

/*

Routine Description:

	SPI Identification routine.

Arguments:

	None.

Return Value:

	1 - successful
	0 - failed

*/
s32 spiIdentification(void)
{

    u32 	unID;

    unID = spiReadID();
    spiManufID = (u8) (unID >> 8);
    spiDevID = (u8) unID;
#ifdef SPI_DEBUG_FLAG
	DEBUG_SPI("ucManufID = %#X\n", spiManufID);
	DEBUG_SPI("ucDevID = %#X\n", spiDevID);
#endif

	switch (spiManufID)
	{
		case 0xBF:	/* SST */
			switch (spiDevID)
			{
				case 0x4A:
					spiTotalSize = 0x400000;	/* 4MB */
					spiBlockSize = 0x10000; 	/* 64KB */
					spiSectorSize = 0x01000;	/* 4KB */
					spiPageSize = 0x100;			/* 256 B */
					DEBUG_SPI("\n");
					DEBUG_SPI("The Device is: ");
					DEBUG_SPI("SST-25VF0328 -[32Mb]\n");
					DEBUG_SPI("\n");
					break;
				
				default:
					DEBUG_SPI("Unknow Device!\n");
					DEBUG_SPI("Quit Verification.\n");
					DEBUG_SPI("\n");
					return 1;
				}
			break;
		case 0x8C:	/* ESMT */
			switch (spiDevID)
			{

				case 0x16:
					spiTotalSize = 0x800000;	/* 8MB */
					spiBlockSize = 0x10000; 	/* 64KB */
					spiSectorSize = 0x1000;	/* 4KB */
					spiPageSize = 0x100;			/* 256 B */
					DEBUG_SPI("\n");
					DEBUG_SPI("The Device is: ");
					DEBUG_SPI("ESMT-F25L64QA -[64Mb]\n");
					DEBUG_SPI("\n");
					break;

				case 0x15:
					spiTotalSize = 0x400000;	/* 4MB */
					spiBlockSize = 0x10000; 	/* 64KB */
					spiSectorSize = 0x01000;	/* 4KB */
					spiPageSize = 0x100;			/* 256 B */
					DEBUG_SPI("\n");
					DEBUG_SPI("The Device is: ");
					DEBUG_SPI("ESMT-25L32PA -[32Mb]\n");
					DEBUG_SPI("\n");
					break;
				
				default:
					DEBUG_SPI("Unknow Device!\n");
					DEBUG_SPI("Quit Verification.\n");
					DEBUG_SPI("\n");
					return 1;
				}
				break;
			break;

	}

		


    return 1;

}

/*

Routine Description:

	SPI Write Register Status (WRSR) routine.

Arguments:

	ucSRData - Register data to write.

Return Value:

	1 - success
	0 - failed

*/
s32 spiWRSR(u8 ucSRData)
{
    u8 ucDataFilter = 0x00;
    u32	unData;


    ucDataFilter &= ucSRData;		/* Disable the entire chip protection */

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_16BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;

    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */

    SpiTxData = (u32) (((u16)SPI_CMD_WRSR << 8)|
                       (u16)ucDataFilter);		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WRSR);
    unData = SpiRxData;		/* read to clear it */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}

/*

Routine Description:

	SPI Enable Write-Status (EWSR) routine.

Arguments:

	none.

Return Value:

	1 - success
	0 - failed

*/
s32 spiEWSR(void)
{
//	u8 err;
    u32 unData;


    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_8BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */

    SpiTxData = SPI_CMD_EWSR;		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_EWSR);
    unData = SpiRxData;		/* read to clear it */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}

/*

Routine Description:

	SPI Write-Enable (WREN) routine.

Arguments:

	none.

Return Value:

	1 - success
	0 - failed

*/
s32 spiWREN(void)
{
//	u8 err;
    u32 unData;


    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_8BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;

    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */

    SpiTxData = SPI_CMD_WREN;		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);
    unData = SpiRxData;		/* read to clear it */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
//	SpiCtrl = (spiClkDiv << SPI_FREQ_MASK);
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}

/*

Routine Description:

	SPI Write-Disable (WRDI) routine.

Arguments:

	none.

Return Value:

	1 - success
	0 - failed

*/
s32 spiWRDI(void)
{
//	u8 err;
    u32 unData;


    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_8BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;

    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */

    SpiTxData = SPI_CMD_WRDI;		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);
    unData = SpiRxData;		/* read to clear it */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;
//	SpiCtrl = (spiClkDiv << SPI_FREQ_MASK);

    return 1;

}

/*

Routine Description:

	SPI Hardware Write Detection Enable (HWWDEN) routine.

Arguments:

	None

Return Value:

	Return register status.

*/
u8 spiHWWDEN(void)
{

//	u8 err;
    u32 unData;


    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_8BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */
    SpiTxData = SPI_CMD_HWWDEN;		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_HWWDEN);
    unData = SpiRxData;		/* read to clear it */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
//	SpiCtrl = (spiClkDiv << SPI_FREQ_MASK);

    return 1;

}

/*

Routine Description:

	SPI Hardware Write Detection Disable (HWWDDISA) routine.

Arguments:

	None

Return Value:

	Return register status.

*/
u8 spiHWWDDISA(void)
{

//	u8 err;
    u32 unData;


    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_8BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;

    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */

    SpiTxData = SPI_CMD_HWWDDISA;		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_HWWDDISA);
    unData = SpiRxData;		/* read to clear it */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;
//	SpiCtrl = (spiClkDiv << SPI_FREQ_MASK);

    return 1;

}

/*

Routine Description:

	SPI Read Register Status (RDSR) routine.

Arguments:

	None

Return Value:

	Return register status.

*/
u8 spiRDSR(void)
{
    u32 unSRData = 1;
    u8 err;
    u32 unData = 0;


        SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                  SPI_TX_LEN_32BIT|
                  SPI_EN;
        SpiIntEn = SPI_INT_BUS_FIN_EN;
        SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */

        SpiTxData = ((u32)SPI_CMD_RDSR << 24) |unData;		/* set cmd and trigger */

        spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_RDSR);
        unSRData = SpiRxData;

        SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
        SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */

    return unSRData;

}

/*

Routine Description:

	SPI sector erase routine.

Arguments:

	unAddr - the addr to be erased.

Return Value:

	1 - success
	0 - failed

Remark:

	unAddr will be mapped to the sector addr for erasing.

*/
s32 spiSectorErase(u32 unAddr)
{
    u8 ucSRData;
    u32	unCnt;

	if (SYS_CPU_CLK_FREQ == 24000000)
		unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 or 192 MHz */
        unCnt = 0x3ffff;

    unAddr &= 0x000FF000;

    spiEWSR();
    spiWRSR(0x00);
    spiWREN();

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_32BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

    SpiTxData = ( ((u32) SPI_CMD_SEC_ERASE) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_SECTOR_ERASE);
    ucSRData = SpiRxData;		/* read to clear it */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
	while (spiRDSR() & 1)
	{
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            return 0;
        }
        unCnt --;
	}

    return 1;

}

/*

Routine Description:

	SPI block erase routine.

Arguments:

	unAddr - the addr to be erased.

Return Value:

	1 - success
	0 - failed

Remark:

	unAddr will be mapped to the block addr for erasing.

*/
s32 spiBlockErase(u32 unAddr)
{
	u32	unCnt;

    if (SYS_CPU_CLK_FREQ == 24000000)
		unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;
//    unAddr &= 0x000F0000;
    unAddr = ((unAddr/spiBlockSize)*spiBlockSize);		/* abstrct the addres for block erase */

    spiEWSR();
    spiWRSR(0x00);
    spiWREN();


    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_32BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */
    SpiTxData = ( ((u32) SPI_CMD_BLK_ERASE) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BLOCK_ERASE);

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */
	while (spiRDSR() & 1)
	{
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            return 0;
        }
        unCnt --;
	}

    return 1;

}

/*

Routine Description:

	SPI chip erase routine.

Arguments:

	None.

Return Value:

	1 - success
	0 - failed

*/
s32 spiChipErase(void)
{
    u8 err;
    u32 unData;
	u32	unCnt;

    if (SYS_CPU_CLK_FREQ == 24000000)
		unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    spiEWSR();
    spiWRSR(0x00);
    spiWREN();

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_8BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

    SpiTxData = SPI_CMD_CHIP_ERASE;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_CHIP_ERASE);
    unData = SpiRxData;		/* read to clear it */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */

	while (spiRDSR() & 1)
	{
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            return 0;
        }
        unCnt --;
	}

    return 1;

}



#if 0

/*

Routine Description:

	SPI Write selcetion routine.

Arguments:

	ucData - data to write.
	unDstAddr - address which data write to .
	unSrcAddr - address which data write from.
	unLen - data size to write in byte.

	u32 unDstAddr, u32 unSrcAddr

Return Value:

	1 - success
	0 - failed

*/
s32 spiWrite(u8 ucData, u32 unDstAddr, u32 unSrcAddr, u32 unLen)
{


    if (unLen == 1)
        spiByteProgram(ucData,unDstAddr);
    else
        spiWrite(unDstAddr, unSrcAddr, unLen);

    return 1;

}
#endif

/*

Routine Description:

	SPI Page program routine.

Arguments:

	ucData - data to write.
	unAddr - address which data to write to.
	unLen - data length to write.

Return Value:

	1 - success
	0 - failed

*/
s32 spiPageProgram(u8 ucData, u32 unAddr, u32 unLen)
{
    u8 err;
	u32	unCnt;

    if (SYS_CPU_CLK_FREQ == 24000000)
		unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    spiWREN();

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_32BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

    SpiTxData = (SPI_CMD_BYTE_PROGRAM<<24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_8BIT|
              SPI_SSOE_EN|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiTxData = (u32) ucData;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */

	while (spiRDSR() & 1)
	{
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            return 0;
        }
        unCnt --;
	}

    return 1;

}



/*

Routine Description:

	SPI Byte program routine.

Arguments:

	ucData - data to write.
	unAddr - address which data to write.

Return Value:

	1 - success
	0 - failed

*/
s32 spiByteProgram(u8 ucData, u32 unAddr)
{
    u8 err;
	u32	unCnt;

    if (SYS_CPU_CLK_FREQ == 24000000)
		unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    spiEWSR();
    spiWRSR(0x00);
    spiWREN();

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_32BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

    SpiTxData = (SPI_CMD_BYTE_PROGRAM<<24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_8BIT|
              SPI_SSOE_EN|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiTxData = (u32) ucData;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */

	while (spiRDSR() & 1)
	{
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            return 0;
        }
        unCnt --;
	}

    return 1;

}

/*

Routine Description:

	SPI Auto-Address-Increment-Word Program routine.
	The AAIProgram function is the specific feature of ESMT's serial flash. Windbond and EON only write 256 bytes maximum in one program cycle.

Arguments:

	pucSrcBuf - source buffer address where data is located.
	unDstAddr - destination buffer address where data to write on.
	unSize - Data length to write in byte.

Return Value:

	1 - success
	0 - failed

*/
s32 spiWrite (u32 unDstAddr, u8* pucSrcBuf, u32 unSize)
{
    u8 err;
    u8 ucDataByte1, ucDataByte2, ucDataByte3, ucDataByte4;
    u32 i, j;
    u32 unData;
	u32	unCnt;

    if (SYS_CPU_CLK_FREQ == 24000000)
		unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;


    spiWREN();
    spiWRSR(0x00);

    spiWREN();
    spiHWWDEN;

    for (i=0; i<unSize ; i+=4)
    {
        ucDataByte1 = *((u8*) pucSrcBuf+3);
        ucDataByte2 = *((u8*) pucSrcBuf+2);
        ucDataByte3 = *((u8*) pucSrcBuf+1);
        ucDataByte4 = *((u8*) pucSrcBuf+0);

        if (i != 0)
        {
            SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                      SPI_TX_LEN_24BIT|
                      SPI_EN;
            SpiIntEn = SPI_INT_BUS_FIN_EN;

            SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

            SpiTxData = (	(SPI_CMD_AAI_PROGRAM <<16)|
                          ((u32) ucDataByte1 << 8) |
                          ((u32) ucDataByte2));		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
            spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_AAI_PROGRAM);
            unData = SpiRxData;

            SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */
            SpiCtrl &= ~SPI_EN;

            unData = 0;
            /* this is for HW end of write detection */
            while (!(unData & 1))
            {
                SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                          SPI_TX_LEN_16BIT|
                          SPI_EN;
                SpiTxData = unData;
                unData = SpiRxData;
                SpiCtrl &= ~SPI_EN;
            }

        }
        else
        {
            SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                      SPI_TX_LEN_32BIT|
                      SPI_EN;
            SpiIntEn = SPI_INT_BUS_FIN_EN;

            SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

            SpiTxData = (SPI_CMD_AAI_PROGRAM<< 24)| unDstAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
            spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_AAI_PROGRAM);
            unData = SpiRxData;

            SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                      SPI_TX_LEN_16BIT|
                      SPI_SSOE_EN |
                      SPI_EN;
            SpiIntEn = SPI_INT_BUS_FIN_EN;

            SpiTxData = (	((u32) ucDataByte1 << 8) |((u32) ucDataByte2));		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
            spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_AAI_PROGRAM);
            unData = SpiRxData;

            SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */
            SpiCtrl &= ~SPI_EN;

            unData = 0;
            /* this is for HW end of write detection */
            while (!(unData & 1))
            {
                SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                          SPI_TX_LEN_16BIT|
                          SPI_EN;
                SpiTxData = unData;
                unData = SpiRxData;
                SpiCtrl &= ~SPI_EN;
            }

        }

        SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                  SPI_TX_LEN_24BIT|
                  SPI_EN;
        SpiIntEn = SPI_INT_BUS_FIN_EN;

        SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

        SpiTxData = (	(SPI_CMD_AAI_PROGRAM <<16)|
                      ((u32) ucDataByte3 << 8) |
                      ((u32) ucDataByte4));		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
        spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_AAI_PROGRAM);
        unData = SpiRxData;

        SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */
        SpiCtrl &= ~SPI_EN;

        unData = 0;
        /* this is for HW end of write detection */
        while (!(unData & 1))
        {
            SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                      SPI_TX_LEN_16BIT|
                      SPI_EN;
            SpiTxData = unData;
            unData = SpiRxData;
            SpiCtrl &= ~SPI_EN;
        }

//        unSrcAddr += 4;
		pucSrcBuf += 4;

    }

    if (spiHWWDDISA() == 0)
        return 0;

    spiWRDI();

	while (spiRDSR() & 1)
	{
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            return 0;
        }
        unCnt --;
	}

    return 1;

}


/*

Routine Description:

	SPI WinBond  Write routine. Page write 256 Bytes maximum.

Arguments:

	ucSrc - Data source address.
	unAddr - Data destination address.
	ucSize - Total size to write. Maximum 256 bytes to write per cycle.

Return Value:

	1 - success
	0 - failed

*/
s32 spiWinbondWrite(u32 unAddr, u8* ucSrc, u16 ucSize)
{
    u8 err;
    u32 i=0;
    u32* temp_addr=(u32*) ucSrc;
	u32	unCnt;
	
    if (SYS_CPU_CLK_FREQ == 24000000)
		unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    spiEWSR();
    spiWRSR(0x00);
    spiWREN();

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_32BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

    SpiTxData = (SPI_CMD_PAGE_PROGRAM<<24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_8BIT|
              SPI_SSOE_EN|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    for (i=0;i<ucSize/4;i++) // must align 4 if not need coding more
    {
        SpiTxData = (u8)((*temp_addr)>>24 &0xFF);		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
        SpiTxData = (u8)((*temp_addr)>>16 &0xFF);
        SpiTxData = (u8)((*temp_addr)>>8 &0xFF);
        SpiTxData = (u8)((*temp_addr)&0xFF);
        //spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
        temp_addr++;
    }
    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */

	while (spiRDSR() & 1)
	{
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            return 0;
        }
        unCnt --;
	}
//	SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK);

    return 1;

}


/*

Routine Description:

	SPI Read ID Test routine.

Arguments:

	None.

Return Value:

	1 - success
	0 - failed

*/
s32 spiReadIDTest (void)
{

    if (spiIdentification() == 0)
        return 0;

    return 1;
}


/*

Routine Description:

	SPI Chip Erase Test routine.

Arguments:

	None.

Return Value:

	1 - success
	0 - failed

*/
s32 spiChipEraseTest (void)
{
    u32 i, j, k;
    u8	ucCnt;


    for (j=0; j<2; j++)			/* Test twice to verify the function fully */
    {
        DEBUG_SPI("Cycle %d...\n", j+1);
        DEBUG_SPI("Wait about 8 Secs (Typical)\n");

        if (spiChipErase()==0)
            return 0;
        for (i=0; i<spiTotalSize; i+= BUF_SIZE)
        {
            memset(spiReadBuf, 0, BUF_SIZE);
            if (spiRead(spiReadBuf, i, BUF_SIZE) == 0)
                return 0;

            for (k=0; k<BUF_SIZE; k++)
                if (spiReadBuf[k] != 0xFF)
                {
                    DEBUG_SPI("spiReadBuf[%d] = %d \n", k, spiReadBuf[k]);
                    DEBUG_SPI("Chip Erase Failed!\n");
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }
        }
        DEBUG_SPI("\n");
        ucCnt = 0;
//		if (spiManufactor == SPI_MANUF_ESMT)	/* ESMT */
//        if (spiManufID == SPI_MANUF_ESMT)	/* ESMT */
            for (i=0; i<spiTotalSize; i+= BUF_SIZE)
            {
                if (spiWrite(i , spiWriteBuf, BUF_SIZE) == 0)
                {
                    DEBUG_SPI("[Error]: AAIProgram Error!\n");
                    return 0;
                }

                if (ucCnt == 50)
                {
                    DEBUG_SPI("\b\b\b\b\b");
                    ucCnt = 0;
                }
                else
                {
                    if ((ucCnt%10) == 0)
                        DEBUG_SPI(".");
                    ucCnt ++;
                }

            }
        /* Winbond SPI flash */
//		else if (spiManufactor == SPI_MANUF_WINBOND)	/* Winbond */
#if 0
        else if (spiManufID == SPI_MANUF_WINBOND)	/* Winbond */
            for (i=0; i<spiTotalSize; i+= BUF_SIZE)
            {
                if (spiWinbondWrite(i, spiWriteBuf, BUF_SIZE) == 0)
                {
                    DEBUG_SPI("[Error]: WinBondWrite Error!\n");
                    return 0;
                }

                if (ucCnt == 50)
                {
                    DEBUG_SPI("\b\b\b\b\b");
                    ucCnt = 0;
                }
                else
                {
                    if ((ucCnt%10) == 0)
                        DEBUG_SPI(".");
                    ucCnt ++;
                }
            }
#endif
        DEBUG_SPI("\n");
        ucCnt = 0;
        for (i=0; i<spiTotalSize; i+= BUF_SIZE)
        {
            memset(spiReadBuf, 0, BUF_SIZE);
            if (spiRead(spiReadBuf, i, BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<BUF_SIZE; k++)
                if (spiReadBuf[k] != spiWriteBuf[k])
                {
                    DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    DEBUG_SPI("spiWriteBuf[%d] = 0x%8x\n", k, spiWriteBuf[k]);
                    DEBUG_SPI("Read & write result don't match!\n");
                    for (k=0; k<10; k++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }

        }
        DEBUG_SPI("\n");
    }
    DEBUG_SPI("[Pass]: Chip Erase\n");
    return 1;


}


/*

Routine Description:

	SPI Block Erase Test routine.

Arguments:

	unAddr - the address to erase.

Return Value:

	1 - success
	0 - failed

*/
s32 spiBlockEraseTest (u32 unAddr)
{
    u32 i, j, k;
    u8	ucCnt;

    unAddr &= 0x000F0000;

    DEBUG_SPI("The testing Block Address = %#x\n", unAddr);

    //DEBUG_SPI("\n\nBlock-Erase Testing.....\n");
    for (j=0; j<2; j++)			/* Test twice to verify the function fully */
    {
        DEBUG_SPI("Cycle %d...\n", j+1);

        if (spiBlockErase(unAddr)==0)
            return 0;

        ucCnt = 0;
        for (i=unAddr; i<(unAddr + spiBlockSize); i+= BUF_SIZE)
        {
            memset(spiReadBuf, 0, BUF_SIZE);
            if (spiRead(spiReadBuf, i, BUF_SIZE) == 0)
                return 0;

            for (k=0; k<BUF_SIZE; k++)
                if (spiReadBuf[k] != 0xFF)
                {
                    DEBUG_SPI("Block Erase Failed. spiReadBuf not equals to 0xff\n");

                    for (j=k; (j<(k+10)) && (k<BUF_SIZE); j++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", j, spiReadBuf[j]);

                    DEBUG_SPI("i = %#x\n", i);
                    DEBUG_SPI("Block Erase Failed!\n");
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)

                    DEBUG_SPI(".");
                ucCnt ++;
            }

        }
        DEBUG_SPI("\n");
        ucCnt = 0;
        /* ESMT SPI flash */
//		if (spiManufactor == SPI_MANUF_ESMT)	/* ESMT */
//        if (spiManufID == SPI_MANUF_ESMT)	/* ESMT */
        {
            for (i=unAddr; i<(unAddr + spiBlockSize); i+= BUF_SIZE)
            {
                if (spiWrite(i , spiWriteBuf, BUF_SIZE) == 0)
                {
                    DEBUG_SPI("[Error]: AAIProgram Error!\n");
                    return 0;
                }

                if (ucCnt == 50)
                {
                    DEBUG_SPI("\b\b\b\b\b");
                    ucCnt = 0;
                }
                else
                {
                    if ((ucCnt%10) == 0)
                        DEBUG_SPI(".");
                    ucCnt ++;
                }

            }
        }
        /* Winbond SPI flash */
//		else if (spiManufactor == SPI_MANUF_WINBOND)	/* Winbond */
#if 0
        else if (spiManufID == SPI_MANUF_WINBOND)	/* Winbond */
        {
            for (i=unAddr; i<(unAddr + spiBlockSize); i+= BUF_SIZE)
            {
                if (spiWinbondWrite(i, spiWriteBuf, BUF_SIZE) == 0)
                {
                    DEBUG_SPI("[Error]: WinBondWrite Error!\n");
                    return 0;
                }

                if (ucCnt == 50)
                {
                    DEBUG_SPI("\b\b\b\b\b");
                    ucCnt = 0;
                }
                else
                {
                    if ((ucCnt%10) == 0)
                        DEBUG_SPI(".");
                    ucCnt ++;
                }
            }
        }
#endif
        DEBUG_SPI("\n");
        ucCnt = 0;
        for (i=unAddr; i<(unAddr + spiBlockSize); i+= BUF_SIZE)
        {
            memset(spiReadBuf, 0, BUF_SIZE);
            if (spiRead(spiReadBuf, i, BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<BUF_SIZE; k++)
                if (spiReadBuf[k] != spiWriteBuf[k])
                {
                    DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    DEBUG_SPI("spiWriteBuf[%d] = 0x%8x\n", k, spiWriteBuf[k]);
                    DEBUG_SPI("Read & write result don't match!\n");
                    for (k=0; k<10; k++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    return 0;
                }

            if (ucCnt == 50)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                if ((ucCnt%10) == 0)
                    DEBUG_SPI(".");
                ucCnt ++;
            }

        }
        DEBUG_SPI("\n");
    }
    DEBUG_SPI("[Pass]: Block Erase\n");

    unAddrForTest += spiBlockSize;

    return 1;


}


/*

Routine Description:

	SPI Sector Erase Test routine.

Arguments:

	unAddr - the address to erase.

Return Value:

	1 - success
	0 - failed

*/
s32 spiSectorEraseTest (u32 unAddr)
{
    u32 i, j, k;

    unAddr &= 0x000FF000;
    DEBUG_SPI("The testing Sector Address = %#x\n", unAddr);

    //DEBUG_SPI("\n\nSector-Erase Testing.....\n");
    for (j=0; j<2; j++)			/* Test twice to verify the function fully */
    {
        DEBUG_SPI("Cycle %d...\n", j+1);

        memset(spiReadBuf, 0, BUF_SIZE);
        if (spiSectorErase(unAddr)==0)
            return 0;

        for (i=unAddr; i<(unAddr + spiSectorSize); i+= BUF_SIZE)
        {
            if (spiRead(spiReadBuf, i, BUF_SIZE) == 0)
                return 0;

            for (k=0; k<BUF_SIZE; k++)
                if (spiReadBuf[k] != 0xFF)
                {
                    DEBUG_SPI("Sector Erase Failed. spiReadBuf not equals to 0xff\n");

                    for (j=k; (j<k+10) && (j<BUF_SIZE); j++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", j, spiReadBuf[j]);

                    DEBUG_SPI("Sector Erase Failed!\n");
                    return 0;
                }
        }
        /* ESMT SPI flash */
        for (i=unAddr; i<(unAddr + spiSectorSize); i+= BUF_SIZE)
            if (spiWrite(i , spiWriteBuf, BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: AAIProgram Error!\n");
                return 0;
            }

        for (i=unAddr; i<(unAddr + spiSectorSize); i+= BUF_SIZE)
        {
            memset(spiReadBuf, 0, BUF_SIZE);
            if (spiRead(spiReadBuf, i, BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<BUF_SIZE; k++)
                if (spiReadBuf[k] != spiWriteBuf[k])
                {
                    DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    DEBUG_SPI("spiWriteBuf[%d] = 0x%8x\n", k, spiWriteBuf[k]);
                    DEBUG_SPI("Read & write result don't match!\n");
                    for (j=k; (j<k+10) && (j<BUF_SIZE); j++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", j, spiReadBuf[j]);

                    return 0;
                }
        }

    }

    DEBUG_SPI("[Pass]: Sector Erase\n");

    unAddrForTest += spiSectorSize;

    return 1;


}


/*

Routine Description:

	SPI Byte Program Test routine.

Arguments:

	unAddr - the address to erase.

Return Value:

	1 - success
	0 - failed

*/
s32 spiByteProgramTest (u32 unAddr)
{
    u32 i;


    //DEBUG_SPI("\n\nByte-Program Testing.....\n");

    unAddr &= 0x000FF000;

    /* Sector Erase*/
    if (spiSectorErase(unAddr)==0)
        return 0;

    if (spiRead(spiReadBuf, unAddr, BUF_SIZE) == 0)
        return 0;

    for (i=0; i<BUF_SIZE; i++)
        if (spiReadBuf[i] != 0xFF)
        {
            DEBUG_SPI("Sector Erase Failed in Byte-Program Testing!\n");
            DEBUG_SPI("Finish The Verification\n");
            return 0;
        }

    /* Byte Program Test */
    if (spiByteProgram(0xA5, unAddr) == 0)
        return 0;
    if (spiByteProgram(0x5A, unAddr+1) == 0)
        return 0;
    if (spiByteProgram(0xF0, unAddr+2) == 0)
        return 0;
    if (spiByteProgram(0x0F, unAddr+3) == 0)
        return 0;

    if (spiRead(spiReadBuf, unAddr, 16) == 0)
        return 0;

    if (spiReadBuf[3] != 0xA5)
        return 0;
    if (spiReadBuf[2] != 0x5A)
        return 0;
    if (spiReadBuf[1] != 0xF0)
        return 0;
    if (spiReadBuf[0] != 0x0F)
        return 0;

    DEBUG_SPI("[Pass]: Byte Program\n");
    return 1;


}

/*

Routine Description:

	SPI Mass Data Read/ Write Test routine.

Arguments:

	None.

Return Value:

	1 - success
	0 - failed

*/
s32 spiMassRWTest (void)
{
    u32 i, j;
    u8	ucCnt;

    DEBUG_SPI("Chip Erase\n");
    DEBUG_SPI("Wait about 8 Secs (Typical)\n");
    /* erase the whole chip */
    if (spiChipErase()==0)
    {
        DEBUG_SPI("Chip erase failed.\n");
        return 0;
    }

    DEBUG_SPI("Read out to check chip erase results.\n");
    /* read out to verify erase process */
    for (i=0; i<spiTotalSize; i+= BUF_SIZE)
    {
        memset(spiReadBuf, 0, BUF_SIZE);
        if (spiRead(spiReadBuf, i, BUF_SIZE) == 0)
            return 0;

        for (j=0; j<BUF_SIZE; j++)
            if (spiReadBuf[j] != 0xFF)
            {
                DEBUG_SPI("Chip Erase Failed in Mass RW Testing!\n");
                DEBUG_SPI("Quit The Verification!\n");
                DEBUG_SPI("i = %#x, j = %d\n", i, j);
                DEBUG_SPI("spiReadBuf[%d] = %#x\n", j, spiReadBuf[j]);
                return 0;
            }
    }

    ucCnt = 0;
    DEBUG_SPI("Program data to Serial Flash.\n");

    /* program the specific data into serial flash */
    for (i=0; i<spiTotalSize; i+= BUF_SIZE)
    {
        memset(spiReadBuf, 0, BUF_SIZE);

        if (spiManufID == SPI_MANUF_ESMT)	/* ESMT */
        {
            if (spiWrite(i, spiWriteBuf, BUF_SIZE) == 0)
                return 0;
        }
        else if (spiManufID == SPI_MANUF_WINBOND)	/* WinBond */
        {
            if (spiWinbondWrite(i, spiWriteBuf, BUF_SIZE) == 0)
                return 0;
        }

        if (spiRead(spiReadBuf, i, BUF_SIZE) == 0)
            return 0;

        for (j=0; j<BUF_SIZE; j++)
            if (spiReadBuf[j] != j)
            {
                DEBUG_SPI("Program Failed in Mass RW Testing!\n");
                DEBUG_SPI("Quit The Verification!\n");
                DEBUG_SPI("i = %#x, j = %d\n", i, j);
                DEBUG_SPI("spiReadBuf[%d] = %#x\n", j, spiReadBuf[j]);
                return 0;
            }
#if 1
        if (ucCnt == 5)
        {
            DEBUG_SPI("\b\b\b\b\b");
            ucCnt = 0;
        }
        else
        {
            DEBUG_SPI(".");
            ucCnt ++;
        }
#endif
    }

    DEBUG_SPI("\n[Pass]: Mass Read/ Write Test.\n");
    return 1;

}




/****************************
Funcstions For Verification
****************************/

/*

Routine Description:

	Serial flash Bit-wise Test routine.

Arguments:

	None.

Return Value:

	1 - success
	0 - failed

*/
s32 spiBitWiseTest (void)
{
    u32 i, j, k;
    u8	ucCnt;
    u8 	ucPattern;




    for (k=0; k<4; k++)
    {

        /* set the test pattern */
        switch (k)
        {
        case 0:
            ucPattern = 0;
            break;

        case 1:
            ucPattern = 0xff;
            break;

        case 2:
            ucPattern = 0xaa;
            break;

        case 3:
            ucPattern = 0x55;
            break;

        }

        DEBUG_SPI("\n@@ Cycle = %d @@\n", k);
        DEBUG_SPI("Test pattern [%#x]\n", ucPattern);

        DEBUG_SPI("Chip Erase....\n");
        DEBUG_SPI("Wait about 8 Secs (Typical)\n");

        /* erase the whole chip */
        if (spiChipErase()==0)
        {
            DEBUG_SPI("Chip erase failed.\n");
            return 0;
        }

        DEBUG_SPI("Read out to check chip erase results.\n");
        /* read out to verify erase process */
        for (i=0; i<spiTotalSize; i+= BUF_SIZE)
        {
            memset(spiReadBuf, 0, BUF_SIZE);
            if (spiRead(spiReadBuf, i, BUF_SIZE) == 0)
                return 0;

            for (j=0; j<BUF_SIZE; j++)
                if (spiReadBuf[j] != 0xFF)
                {
                    DEBUG_SPI("Chip Erase Failed in Mass RW Testing!\n");
                    DEBUG_SPI("Quit The Verification!\n");
                    DEBUG_SPI("i = %#x, j = %d\n", i, j);
                    DEBUG_SPI("spiReadBuf[%d] = %#x\n", j, spiReadBuf[j]);
                    return 0;
                }
        }

        ucCnt = 0;
        /* Set value to buffer */
        memset(spiWriteBuf, ucPattern, BUF_SIZE);

        DEBUG_SPI("Write and check the data.\n");
        /* program the specific data into serial flash */
        for (i=0; i<spiTotalSize; i+= BUF_SIZE)
        {
            memset(spiReadBuf, 0, BUF_SIZE);
            if (spiWrite(i, spiWriteBuf, BUF_SIZE) == 0)
                return 0;

            if (spiRead(spiReadBuf, i, BUF_SIZE) == 0)
                return 0;

            for (j=0; j<BUF_SIZE; j++)
                if (spiReadBuf[j] != ucPattern)
                {
                    DEBUG_SPI("Chip Erase Failed in Mass RW Testing!\n");
                    DEBUG_SPI("Quit The Verification!\n");
                    DEBUG_SPI("i = %#x, j = %d\n", i, j);
                    DEBUG_SPI("spiReadBuf[%d] = %#x\n", j, spiReadBuf[j]);
                    return 0;
                }

            if (ucCnt == 5)
            {
                DEBUG_SPI("\b\b\b\b\b");
                ucCnt = 0;
            }
            else
            {
                DEBUG_SPI(".");
                ucCnt ++;
            }
            //		DEBUG_SPI("Program i = %#x\n", i);
        }
        DEBUG_SPI("\n");

    }


    DEBUG_SPI("\n[Pass]: Serial Flash Bit-Wise Full Test.\n");

    return 1;

}

/*

Routine Description:

	Spi Read Address for test routine.

Arguments:

	None.

Return Value:

	Block Address.

*/
u32 spiReadAddr4Test (void)
{
    u8	pucBuf[4];
    u32	unLastBlk;
    u32	unRtnAddr;


    unLastBlk = spiTotalSize - spiBlockSize;

    /* Read 4 bytes */
    spiRead(pucBuf, unLastBlk, 4);

    unRtnAddr = *(u32*)pucBuf;

    unRtnAddr &= 0x000FF000;

    return unRtnAddr;

}


/*

Routine Description:

	Spi Write Address for the next test routine.

Arguments:

	unAdd - the addr will be tested next time.

Return Value:

	1 - Success
	0 - Failed

*/
u32 spiWriteAddr4Test (u32 unAddr)
{
    u8	pucBuf[4];
    u32	unLastBlk;		// = 0x000F0000;		/* start address of block-15 */
    u8 	ucWriteData, i;


    unLastBlk = spiTotalSize - spiBlockSize;

    unAddr += (spiBlockSize + spiSectorSize);

    if (unAddr >= spiTotalSize)
        unAddr = spiSectorSize;		/* start from sector 1 */

    if (spiBlockErase(unLastBlk) == 0)
    {
        DEBUG_SPI("Block erase failed in Write Block Addr 4 Test.\n");
        return 0;
    }

    /* store the next block address for test */
    for (i=0; i<4; i++)
    {
        ucWriteData = (u8)(unAddr >> (i<<3));
        if (spiByteProgram(ucWriteData, (u32)(unLastBlk +3 -i)) == 0)
        {
            DEBUG_SPI("Byte Program Failed in Write Block Addr 4 Test.\n");
            return 0;
        }

    }

    /* read out to check the result */
    if (spiRead(pucBuf, unLastBlk, 4) == 0)
    {
        DEBUG_SPI("spiRead Failed in Write Block Addr 4 Test.\n");
        return 0;
    }

    if (unAddr != *(u32*)pucBuf)
    {
        DEBUG_SPI("Write Error.\n");
        DEBUG_SPI("Saved Error\n");
//		DEBUG_SPI("unBlkAddr = %#x\n", unAddr);
        DEBUG_SPI("Saved Addr = %#x\n", *(u32*)pucBuf);
        return 0;
    }
    else
        DEBUG_SPI("The saved block address is [%#x].\n", unAddr);

    return 1;

}


void spiReadReg(u32 unAdd, u32 *unContent, u32 unLeng)
{
    u32 i;
    u32*	punAddr;

    punAddr = (u32*) unAdd;

    DEBUG_SPI("\n");
    for (i=0; i<unLeng; i++)
        //DEBUG_SPI("Addr[%#x] = %#x\n", unAdd + i, *(unsigned long *)(unAdd+ i));
        DEBUG_SPI("Addr[%#x] = %#x\n", unAdd + i, *(punAddr+ i));

}

void spiWriteReg(u32 unAdd, u32 unContent)
{
    *(unsigned long *)unAdd = unContent;
}



/*

Routine Description:

	SPI verification main functions

Arguments:

	None.

Return Value:

	None.

*/
void spiVerify(void)
{

    u32 i, j;
    u8 ucVerifyIdx = 1;
    u8 ucTestItemSel;
    u8 ucMounted;

    u32 unScanAdd, unContent, unLeng;

	sysSD_Disable();
	sysSPI_Enable();


    if (spiIdentification() == 0)
        DEBUG_SPI("ID Detection Error!\n");

    unAddrForTest = spiReadAddr4Test();
//	unAddrForTest = 0x12000;
    DEBUG_SPI("Addr For Test = %#x\n", unAddrForTest);


    /* set write data */
    for (i = 0; i < BUF_SIZE; i++)
        spiWriteBuf[i]= i;

    while (ucVerifyIdx == 1)
    {
        DEBUG_SPI("\n");
        DEBUG_SPI("\n*****************************************\n");
        DEBUG_SPI("*					*\n");
        DEBUG_SPI("*	Serial Flash Verification 	*\n");
        DEBUG_SPI("*					*\n");
        DEBUG_SPI("*****************************************\n");
        DEBUG_SPI("\n");


        DEBUG_SPI("Please Select a item to test! \nKey-in a number to test.\n");
        DEBUG_SPI("[1] - 	Serial Flash - ReadID.\n");
        DEBUG_SPI("[2] -	Serial Flash - Chip Erase Test.\n");
        DEBUG_SPI("[3] -	Serial Flash - Block Erase Test.\n");
        DEBUG_SPI("[4] -	Serial Flash - Sector Erase Test.\n");
        DEBUG_SPI("[5] -	Serial Flash - Byte Program Test.\n");
        DEBUG_SPI("[6] -	Serial Flash - Full Test.\n");
        DEBUG_SPI("[7] -	Serial Flash - Mass Read/Write Test.\n");
        DEBUG_SPI("[8] -	Serial Flash - Read/Write [bit-wise] Test.\n");
        DEBUG_SPI("[9] -	Serial Flash - Adjust SPI Clock.\n");
        DEBUG_SPI("[10] -	Serial Flash - Sector Erase.\n");
        DEBUG_SPI("[11] -	Serial Flash - Read.\n");
        DEBUG_SPI("[12] -	Serial Flash - AAIProgram.\n");
        DEBUG_SPI("[13] -	Serial Flash - Routione Identification Test\n");
        DEBUG_SPI("[15] -	Serial Flash - Read Register.\n");
        DEBUG_SPI("[16] -	Serial Flash - Write Register.\n");
        DEBUG_SPI("[99] -	Finish verification.\n");


        scanf("%d", &ucTestItemSel);
        DEBUG_SPI("\n");

        switch (ucTestItemSel)
        {

        case	1:		/* ID Detection */
            if (spiReadIDTest() == 0)
            {
                DEBUG_SPI("ID Detection Error!\n");
                DEBUG_SPI("Finish Verification!\n");
                //ucVerifyIdx = 0;		/* End Verification */
            }

            ucMounted = 1;	/* Serial Flash is mounted. */

            break;

        case	2:
            DEBUG_SPI("Start to [Chip Erase Test]\n");
            if (ucMounted != 1)
                if (spiIdentification() == 0)
                {
                    DEBUG_SPI("ID Detection Error!\n");
                    DEBUG_SPI("Finish Verification!\n");
                    ucVerifyIdx = 0;		/* End Verification */
                    break;
                }
            if (spiChipEraseTest() == 0)
                DEBUG_SPI("spiChipEraseTest failed\n");

            ucMounted = 1;	/* Serial Flash is mounted. */

            break;

        case	3:
            DEBUG_SPI("Start to [Block Erase Test]\n");
            if (ucMounted != 1)
                if (spiIdentification() == 0)
                {
                    DEBUG_SPI("ID Detection Error!\n");
                    DEBUG_SPI("Finish Verification!\n");
                    ucVerifyIdx = 0;		/* End Verification */
                    break;
                }
            if (spiBlockEraseTest(unAddrForTest) == 0)
                DEBUG_SPI("spiBlockEraseTest failed\n");

            ucMounted = 1;	/* Serial Flash is mounted. */

            break;

        case	4:
            DEBUG_SPI("Start to [Secter Erase Test]\n");
            if (ucMounted != 1)
                if (spiIdentification() == 0)
                {
                    DEBUG_SPI("ID Detection Error!\n");
                    DEBUG_SPI("Finish Verification!\n");
                    ucVerifyIdx = 0;		/* End Verification */
                    break;
                }
            if (spiSectorEraseTest(unAddrForTest) == 0)
                DEBUG_SPI("spiSectorEraseTest failed\n");

            ucMounted = 1;	/* Serial Flash is mounted. */

            break;

        case	5:
            DEBUG_SPI("Start to [Byet Program Test]\n");
            if (ucMounted != 1)
                if (spiIdentification() == 0)
                {
                    DEBUG_SPI("ID Detection Error!\n");
                    DEBUG_SPI("Finish Verification!\n");
                    ucVerifyIdx = 0;		/* End Verification */
                    break;
                }
            if (spiByteProgramTest(unAddrForTest) == 0)
                DEBUG_SPI("spiByteProgramTest failed\n");

            ucMounted = 1;	/* Serial Flash is mounted. */

            break;

        case	6:
            DEBUG_SPI("\nStart to [Full Test]\n");

            if (spiIdentification() == 0)
            {
                DEBUG_SPI("ID Detection Error!\n");
                DEBUG_SPI("Finish Verification!\n");
                ucVerifyIdx = 0;		/* End Verification */
                return;
            }
            ucMounted = 1;	/* Serial Flash is mounted. */

            if (	(spiChipEraseTest() == 0) ||
                    (spiBlockEraseTest(unAddrForTest) == 0) ||
                    (spiSectorEraseTest(unAddrForTest) == 0) ||
                    (spiByteProgramTest(unAddrForTest) == 0) )
            {
                DEBUG_SPI("Full Test Failed\n");
            }
            else
                DEBUG_SPI("[Pass]: Full Test\n");

            break;

        case	7:
            DEBUG_SPI("\nStart to [Mass Read/ Write Test]\n");

            if (ucMounted != 1)
                if (spiIdentification() == 0)
                {
                    DEBUG_SPI("ID Detection Error!\n");
                    DEBUG_SPI("Finish Verification!\n");
                    ucVerifyIdx = 0;		/* End Verification */
                    return;
                }
            ucMounted = 1;	/* Serial Flash is mounted. */
            if (spiMassRWTest() == 0)
                DEBUG_SPI("spiMassRWTest failed\n");

            break;

        case	8:
            DEBUG_SPI("\nStart to [Serial Flash Bit-Wise R/W Test]\n");

            if (ucMounted != 1)
                if (spiIdentification() == 0)
                {
                    DEBUG_SPI("ID Detection Error!\n");
                    DEBUG_SPI("Finish Verification!\n");
                    ucVerifyIdx = 0;		/* End Verification */
                    return;
                }
            ucMounted = 1;	/* Serial Flash is mounted. */
            if (spiBitWiseTest() == 0)
                DEBUG_SPI("spiBitWiseTest failed\n");

            break;

        case	9:
            DEBUG_SPI("\nAdjust SPI Clock Frequency.\n");
            DEBUG_SPI("System Freq = %d MHz.\n", (SYS_CPU_CLK_FREQ / 1000000));
            DEBUG_SPI("Please Key-in the SPI Clock Divisor for required SPI Clock\n");
            DEBUG_SPI("[0] - [48 -> 24 MHz], [96 -> 48 MHz]\n");
            DEBUG_SPI("[1] - [48 -> 12 MHz], [96 -> 24 MHz]\n");
            DEBUG_SPI("[2] - [48 -> 8 MHz], [96 -> 16 MHz]\n");
            DEBUG_SPI("[3] - [48 -> 6 MHz], [96 -> 12 MHz]\n");
            scanf("%d", &unLeng);

            spiClkDiv = (u8) unLeng;

            if (SYS_CPU_CLK_FREQ == 24000000)
            {
                DEBUG_SPI("System Freq = 24 MHz.\n");
                DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (24/2/(spiClkDiv +1)));
            }
            else if (SYS_CPU_CLK_FREQ == 32000000)
            {
                DEBUG_SPI("System Freq = 32 MHz.\n");
                DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (32/2/(spiClkDiv +1)));
            }
            else if (SYS_CPU_CLK_FREQ == 48000000)
            {
                DEBUG_SPI("System Freq = 48 MHz.\n");
                DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (48/2/(spiClkDiv +1)));
            }
            else if (SYS_CPU_CLK_FREQ == 96000000)
            {
                DEBUG_SPI("System Freq = 96 MHz.\n");
                DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (96/2/(spiClkDiv +1)));
            }
			else if (SYS_CPU_CLK_FREQ == 108000000)
            {
                DEBUG_SPI("System Freq = 108 MHz.\n");
                DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (108/2/(spiClkDiv +1)));
            }
            else if (SYS_CPU_CLK_FREQ == 192000000)
            {
                DEBUG_SPI("System Freq = 192 MHz.\n");
                DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (192/2/(spiClkDiv +1)));
            }
            else
                DEBUG_SPI("No Support System Freq: [%d] MHz.\n", (SYS_CPU_CLK_FREQ / 1000000));

            break;

        case	10:

            if (spiSectorErase(unAddrForTest) == 0)
                DEBUG_SPI("Sector Erased Failed\n");

            else
                DEBUG_SPI("Sector Erased CMP.\n");


            break;

        case	11:

            if (spiRead(spiReadBuf, unAddrForTest, BUF_SIZE) == 0)
                DEBUG_SPI("Read Failed\n");

            {
                u32 m;

                for (m=0; m<BUF_SIZE; m++)
                    DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", m, spiReadBuf[m]);

            }

            break;

        case	12:

            if (spiWrite(unAddrForTest, spiWriteBuf, BUF_SIZE) == 0)
                DEBUG_SPI("spiWrite Failed\n");

            else
                DEBUG_SPI("spiWrite CMP.\n");

            break;


        case	13:

        {
            u32 unTestCnt;
            u32	unID;
            u8 ucMenufID, ucDevID;
            u32	unErrIdx = 0;
            u32	unTestTimes = 100;


            DEBUG_SPI("Routione Identification Test %d Times.\n", unTestTimes);
            DEBUG_SPI("Starting....\n");
            for (unTestCnt=0; unTestCnt<unTestTimes; unTestCnt++)
            {
                unID = spiReadID();
                ucMenufID = (u8) (unID >> 8);
                ucDevID = (u8) unID;

                if ((ucMenufID != 0x8c && ucMenufID != 0xef) ||(ucDevID != 0x13 && ucDevID != 0x12))
                {
                    DEBUG_SPI("Error! Identification Error!\n");
                    DEBUG_SPI("Test Count = %d\n", unTestCnt);
                    unErrIdx ++;
                }

            }
            if (unErrIdx == 0)
                DEBUG_SPI("Test %d Times --> All Pass\n", unTestTimes);

            DEBUG_SPI("Test Finished\n");

        }
        break;



        case	15:
            DEBUG_SPI("Please key-in by Hex format.\n");
            DEBUG_SPI("Address = ");
            scanf("%x", &unScanAdd);
            DEBUG_SPI("Length (word) = ");
            scanf("%x", &unLeng);
            spiReadReg(unScanAdd, &unContent, unLeng);

            break;

        case	16:
            DEBUG_SPI("Please key-in by Hex format.\n");
            DEBUG_SPI("Address = ");
            scanf("%x", &unScanAdd);
            DEBUG_SPI("\nContent = ");
            scanf("%x", &unContent);
            DEBUG_SPI("\n");
            spiWriteReg(unScanAdd, unContent);
            break;


        case	99:
            DEBUG_SPI("Finish Verification!\n");
            ucVerifyIdx = 0;		/* End Verification */
            break;


        default:
            DEBUG_SPI("Error! Wrong item selected!\n");

            break;
        }

        if (unAddrForTest > spiTotalSize)
            unAddrForTest = 0;


    }

    spiWriteAddr4Test (unAddrForTest);


    DEBUG_SPI("\nEnd of SPI verification\n");

}
#endif
