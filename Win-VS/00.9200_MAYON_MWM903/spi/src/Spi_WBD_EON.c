/*

Copyright (c) 2005	MARS Semiconductor Corp

Module Name:

	smc.c

Abstract:

	The routines of Serial flash.

Environment:

		ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"
#if ( (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON) ||(FLASH_OPTION == FLASH_SERIAL_ESMT))
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

#if ((CHIP_OPTION == CHIP_A1013A) || \
	 (CHIP_OPTION == CHIP_A1013B) || \
	 (CHIP_OPTION == CHIP_A1016A) || \
	 (CHIP_OPTION == CHIP_A1016B) || \
	 (CHIP_OPTION == CHIP_A1018A) || \
	 (CHIP_OPTION == CHIP_A1018B) || \
	 (CHIP_OPTION == CHIP_A1019A) || \
	 (CHIP_OPTION == CHIP_A1021A) || \
	 (CHIP_OPTION == CHIP_A1025A))
  #if ICOMMWIFI_SUPPORT
    #define SPI_MULTI_CH_ENABLE	1
  #else
    #define SPI_MULTI_CH_ENABLE	0
  #endif
#endif


#if ((CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1021A) || (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1022A))
#define SPI_ENDIAN_ENABLE 1
#endif

/*
 *********************************************************************************************************
 *												 CONSTANTS
 *********************************************************************************************************
 */
/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
__align(4) u8 	spiReadBuf[SPI_MAX_BUF_SIZE];
__align(4) u8 	spiWriteBuf[SPI_MAX_BUF_SIZE];

OS_EVENT* spiDmaEndSemEvt;
OS_EVENT* spiRXOvrSemEvt;
OS_EVENT* spiBusFinSemEvt;
OS_EVENT* spiTXEmptySemEvt;
OS_EVENT* spiRWHookSemEvt; /* check read or write is hold the resource or not */

#if SPI_MULTI_CH_ENABLE
OS_EVENT* spi2DmaEndSemEvt;
OS_EVENT* spi2RXOvrSemEvt;
OS_EVENT* spi2BusFinSemEvt;
OS_EVENT* spi2TXEmptySemEvt;
OS_EVENT* spi2RWHookSemEvt;

OS_EVENT* spi3DmaEndSemEvt;
OS_EVENT* spi3RXOvrSemEvt;
OS_EVENT* spi3BusFinSemEvt;
OS_EVENT* spi3TXEmptySemEvt;
OS_EVENT* spi3RWHookSemEvt;

OS_EVENT* spi4DmaEndSemEvt;
OS_EVENT* spi4RXOvrSemEvt;
OS_EVENT* spi4BusFinSemEvt;
OS_EVENT* spi4TXEmptySemEvt;
OS_EVENT* spi4RWHookSemEvt;

OS_EVENT* spi5DmaEndSemEvt;
OS_EVENT* spi5RXOvrSemEvt;
OS_EVENT* spi5BusFinSemEvt;
OS_EVENT* spi5TXEmptySemEvt;
OS_EVENT* spi5RWHookSemEvt;
#endif

OS_EVENT* spiRDSRSemEvt;

u32 spiClkDiv;
u8 	cnt = 0;
u8	ucSpiTestResult[8] = {0};
u32	unAddrForTest;

u32 spiTotalSize;
u32	spiBlockSize;
u32	spiSectorSize;
u32	spiPageSize;
u8	spiManufID, spiDevID;
u32 guiSPIFlashReadDMAId=0xFF, guiSPIFlashWriteDMAId=0xFF;
u32 guiSPI2FlashReadDMAId=0xFF, guiSPI2FlashWriteDMAId=0xFF;
u32 guiSPI3FlashReadDMAId=0xFF, guiSPI3FlashWriteDMAId=0xFF;
u32 guiSPI4FlashReadDMAId=0xFF, guiSPI4FlashWriteDMAId=0xFF;
u32 guiSPI5FlashReadDMAId=0xFF, guiSPI5FlashWriteDMAId=0xFF;
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

void spiBlkSemProcess(void)
{
	u8 err;
#if RW_PROTECT_SPI
	OSSemPend(spiRWHookSemEvt, OS_IPC_WAIT_FOREVER, &err);
	if(err != OS_NO_ERR)
	{
		DEBUG_SPI("[E] SPI hook sem %d\n", err);
	}
#endif	
}

void spiReleSemProcess(void)
{
#if RW_PROTECT_SPI
	OSSemPost(spiRWHookSemEvt);
#endif	
}


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

        case SPI_SEM_FLG_RW_HOOK:
            OSSemPend(spiRWHookSemEvt, 0, &err);
            break;

#if SPI_MULTI_CH_ENABLE
        case SPI2_SEM_FLG_BUSY:
            //OSSemPend(spi2BusySemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI2_SEM_FLG_DMA_END:
            OSSemPend(spi2DmaEndSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI2_SEM_FLG_RX_OVR:
            OSSemPend(spi2RXOvrSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI2_SEM_FLG_BUS_FIN:
            OSSemPend(spi2BusFinSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI2_SEM_FLG_TX_EMPTY:
            OSSemPend(spi2TXEmptySemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI2_SEM_FLG_RW_HOOK:
            OSSemPend(spi2RWHookSemEvt, 0, &err);
            break;

        case SPI3_SEM_FLG_BUSY:
            //OSSemPend(spi3BusySemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI3_SEM_FLG_DMA_END:
            OSSemPend(spi3DmaEndSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI3_SEM_FLG_RX_OVR:
            OSSemPend(spi3RXOvrSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI3_SEM_FLG_BUS_FIN:
            OSSemPend(spi3BusFinSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI3_SEM_FLG_TX_EMPTY:
            OSSemPend(spi3TXEmptySemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI3_SEM_FLG_RW_HOOK:
            OSSemPend(spi3RWHookSemEvt, 0, &err);
            break;

        case SPI4_SEM_FLG_BUSY:
            //OSSemPend(spi4BusySemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI4_SEM_FLG_DMA_END:
            OSSemPend(spi4DmaEndSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI4_SEM_FLG_RX_OVR:
            OSSemPend(spi4RXOvrSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI4_SEM_FLG_BUS_FIN:
            OSSemPend(spi4BusFinSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI4_SEM_FLG_TX_EMPTY:
            OSSemPend(spi4TXEmptySemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI4_SEM_FLG_RW_HOOK:
            OSSemPend(spi4RWHookSemEvt, 0, &err);
            break;

        case SPI5_SEM_FLG_BUSY:
            //OSSemPend(spi5BusySemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI5_SEM_FLG_DMA_END:
            OSSemPend(spi5DmaEndSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI5_SEM_FLG_RX_OVR:
            OSSemPend(spi5RXOvrSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI5_SEM_FLG_BUS_FIN:
            OSSemPend(spi5BusFinSemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI5_SEM_FLG_TX_EMPTY:
            OSSemPend(spi5TXEmptySemEvt, SPI_TIMEOUT, &err);
            break;

        case SPI5_SEM_FLG_RW_HOOK:
            OSSemPend(spi5RWHookSemEvt, 0, &err);
            break;
#endif
    }

    if (err != OS_NO_ERR)
    {
        switch (ucCmdIdx)
        {
            case SPI_SEM_CMD_IDX_READ:
                DEBUG_SPI("Error: [spiREAD] SemEvt is %d.\n", err);
                break;

            case SPI_SEM_CMD_IDX_WRITE:
                DEBUG_SPI("Error: [spiWRITE] SemEvt is %d.\n", err);
                break;

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
            case SPI_SEM_CMD_IDX_ENTER_4BM:
            	DEBUG_SPI("Error: [spiEn4BM] SemEvt is %d.\n", err);
            	break;
            case SPI_SEM_CMD_IDX_EXIT_4BM:
            	DEBUG_SPI("Error: [spiEx4BM] SemEvt is %d.\n", err);
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

#if SPI_MULTI_CH_ENABLE
void spiIntHandler(void)
{
    u32 spiIntStat = SpiStat;

    SpiStat = 0x0000CCCCC;	/* write 1 to clear status */

    if (spiIntStat & SPI_SR_DMA_END)
    {
        SpiIntEn &= ~SPI_INT_DMA_END_EN;
        OSSemPost(spiDmaEndSemEvt);
        //DEBUG_SPI("spi1DmaEndSemEvt\n");
    }

    if (spiIntStat & SPI_SR_RX_OVR)
    {
        SpiIntEn &= ~SPI_INT_RX_OVR_EN;
    }

    if (spiIntStat & SPI_SR_BUS_FIN)
    {
        SpiIntEn &= ~SPI_INT_BUS_FIN_EN;
        OSSemPost(spiBusFinSemEvt);
        //DEBUG_SPI("spi1BusFinSemEvt\n");
    }

    if (spiIntStat & SPI_SR_TX_EMPTY)
    {
        SpiIntEn &= ~SPI_INT_TX_EMPTY_EN;
    }


    if (spiIntStat & SPI2_SR_DMA_END)
    {
        SpiIntEn &= ~SPI2_INT_DMA_END_EN;
        OSSemPost(spi2DmaEndSemEvt);
    }

    if (spiIntStat & SPI2_SR_RX_OVR)
    {
        SpiIntEn &= ~SPI2_INT_RX_OVR_EN;
    }

    if (spiIntStat & SPI2_SR_BUS_FIN)
    {
        SpiIntEn &= ~SPI2_INT_BUS_FIN_EN;
        OSSemPost(spi2BusFinSemEvt);
    }

    if (spiIntStat & SPI2_SR_TX_EMPTY)
    {
        SpiIntEn &= ~SPI2_INT_TX_EMPTY_EN;
    }

    if (spiIntStat & SPI3_SR_DMA_END)
    {
        SpiIntEn &= ~SPI3_INT_DMA_END_EN;
        OSSemPost(spi3DmaEndSemEvt);
    }

    if (spiIntStat & SPI3_SR_RX_OVR)
    {
        SpiIntEn &= ~SPI3_INT_RX_OVR_EN;
    }

    if (spiIntStat & SPI3_SR_BUS_FIN)
    {
        SpiIntEn &= ~SPI3_INT_BUS_FIN_EN;
        OSSemPost(spi3BusFinSemEvt);
    }

    if (spiIntStat & SPI3_SR_TX_EMPTY)
    {
        SpiIntEn &= ~SPI3_INT_TX_EMPTY_EN;
    }

    if (spiIntStat & SPI4_SR_DMA_END)
    {
        SpiIntEn &= ~SPI4_INT_DMA_END_EN;
        OSSemPost(spi4DmaEndSemEvt);
    }

    if (spiIntStat & SPI4_SR_RX_OVR)
    {
        SpiIntEn &= ~SPI4_INT_RX_OVR_EN;
    }

    if (spiIntStat & SPI4_SR_BUS_FIN)
    {
        SpiIntEn &= ~SPI4_INT_BUS_FIN_EN;
        OSSemPost(spi4BusFinSemEvt);
    }

    if (spiIntStat & SPI4_SR_TX_EMPTY)
    {
        SpiIntEn &= ~SPI4_INT_TX_EMPTY_EN;
    }

    if (spiIntStat & SPI5_SR_DMA_END)
    {
        SpiIntEn &= ~SPI5_INT_DMA_END_EN;
        OSSemPost(spi5DmaEndSemEvt);
    }

    if (spiIntStat & SPI5_SR_RX_OVR)
    {
        SpiIntEn &= ~SPI5_INT_RX_OVR_EN;
    }

    if (spiIntStat & SPI5_SR_BUS_FIN)
    {
        SpiIntEn &= ~SPI5_INT_BUS_FIN_EN;
        OSSemPost(spi5BusFinSemEvt);
    }

    if (spiIntStat & SPI5_SR_TX_EMPTY)
    {
        SpiIntEn &= ~SPI5_INT_TX_EMPTY_EN;
    }
}
#else
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
#endif
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
    if (SYS_CPU_CLK_FREQ == 24000000)
        spiClkDiv = 0;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        spiClkDiv = 0;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        spiClkDiv = 0;
    else if (SYS_CPU_CLK_FREQ == 96000000)	    /* for PA9003 */
        spiClkDiv = 2;
    else if (SYS_CPU_CLK_FREQ == 108000000)	/* for PA9003 */
        spiClkDiv = 2;
    else if ( (SYS_CPU_CLK_FREQ == 160000000) || (SYS_CPU_CLK_FREQ == 162000000) )	/* for PA9003 */
        spiClkDiv = 3;
    else if (SYS_CPU_CLK_FREQ == 192000000)
        spiClkDiv = 3;

#ifdef SPI_DEBUG_FLAG
    DEBUG_SPI("System Freq = %d MHz.\n", (SYS_CPU_CLK_FREQ/1000000));
    DEBUG_SPI("SPI Clock Freq = %d MHz.\n", ((SYS_CPU_CLK_FREQ/1000000)/2/(spiClkDiv +1)));
#endif

    SYS_CTL0 |= SYS_CTL0_SPI_CK_EN;
#if(CHIP_OPTION == CHIP_PA9002D)
    GpioActFlashSelect = GPIO_ACT_FLASH_SPI;
#else
    GpioActFlashSelect = (GpioActFlashSelect | GPIO_ACT_FLASH_SPI) & (~GPIO_ACT_FLASH_SD);
#endif

    spiInitIdx = SPI_UNINITED;
    spiInitTable = SPI_UNINITED;

    //if(spiDmaEndSemEvt == NULL)
    spiDmaEndSemEvt = OSSemCreate(0);

    //if(spiRXOvrSemEvt == NULL)
    spiRXOvrSemEvt = OSSemCreate(0);

    //if(spiBusFinSemEvt == NULL)
    spiBusFinSemEvt = OSSemCreate(0);

    //if(spiTXEmptySemEvt == NULL)
    spiTXEmptySemEvt = OSSemCreate(0);

    //if(spiRDSRSemEvt == NULL)
    spiRDSRSemEvt = OSSemCreate(1);

    //if(spiRWHookSemEvt == NULL)
    spiRWHookSemEvt= OSSemCreate(1);

#if SPI_MULTI_CH_ENABLE
    spi2DmaEndSemEvt = OSSemCreate(0);
    spi2RXOvrSemEvt = OSSemCreate(0);
    spi2BusFinSemEvt = OSSemCreate(0);
    spi2TXEmptySemEvt = OSSemCreate(0);
    spi2RWHookSemEvt= OSSemCreate(1);

    spi3DmaEndSemEvt = OSSemCreate(0);
    spi3RXOvrSemEvt = OSSemCreate(0);
    spi3BusFinSemEvt = OSSemCreate(0);
    spi3TXEmptySemEvt = OSSemCreate(0);
    spi3RWHookSemEvt= OSSemCreate(1);

    spi4DmaEndSemEvt = OSSemCreate(0);
    spi4RXOvrSemEvt = OSSemCreate(0);
    spi4BusFinSemEvt = OSSemCreate(0);
    spi4TXEmptySemEvt = OSSemCreate(0);
    spi4RWHookSemEvt= OSSemCreate(1);

    spi5DmaEndSemEvt = OSSemCreate(0);
    spi5RXOvrSemEvt = OSSemCreate(0);
    spi5BusFinSemEvt = OSSemCreate(0);
    spi5TXEmptySemEvt = OSSemCreate(0);
    spi5RWHookSemEvt= OSSemCreate(1);
#endif
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

#if SPI_MULTI_CH_ENABLE
s32 spi2SetReadDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)&(Spi2RxData);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_READ, &dmaCfg) == 0)
    //    return 0;
    guiSPI2FlashReadDMAId = marsDMAReq(DMA_REQ_SPI2_READ, &dmaCfg);

    return 1;
}

s32 spi2SetReadDataDma_P(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)&(Spi2RxData);
    dmaCfg.dst = (u32)buf;
    //dmaCfg.cnt = siz / 4;
    dmaCfg.cnt = siz >> 2;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_READ, &dmaCfg) == 0)
    //    return 0;
    guiSPI2FlashReadDMAId = marsDMAReq(DMA_REQ_SPI2_READ_P, &dmaCfg);

    return 1;
}

s32 spi3SetReadDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)&(Spi3RxData);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_READ, &dmaCfg) == 0)
    //    return 0;
    guiSPI3FlashReadDMAId = marsDMAReq(DMA_REQ_SPI3_READ, &dmaCfg);

    return 1;
}

s32 spi4SetReadDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)&(Spi4RxData);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_READ, &dmaCfg) == 0)
    //    return 0;
    guiSPI4FlashReadDMAId = marsDMAReq(DMA_REQ_USB_READ, &dmaCfg);

    return 1;
}

s32 spi5SetReadDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)&(Spi5RxData);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_READ, &dmaCfg) == 0)
    //    return 0;
    guiSPI5FlashReadDMAId = marsDMAReq(DMA_REQ_SPI5_READ, &dmaCfg);

    return 1;
}
#endif
/*

Routine Description:

	Set write data dma.

Arguments:

	buf - The buffer to write from.
	siz - The size to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 spiSetWriteDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(SpiTxData);
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_WRITE, &dmaCfg) == 0)
    //    return 0;
    guiSPIFlashWriteDMAId = marsDMAReq(DMA_REQ_SPI_WRITE, &dmaCfg);

    return 1;
}
#if SPI_MULTI_CH_ENABLE
s32 spi2SetWriteDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(Spi2TxData);
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_WRITE, &dmaCfg) == 0)
    //    return 0;
    guiSPI2FlashWriteDMAId = marsDMAReq(DMA_REQ_SPI2_WRITE, &dmaCfg);

    return 1;
}

s32 spi2SetWriteDataDma_P(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(Spi2TxData);
    //dmaCfg.cnt = siz / 4;
    dmaCfg.cnt = siz >> 2;;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_WRITE, &dmaCfg) == 0)
    //    return 0;
    guiSPI2FlashWriteDMAId = marsDMAReq(DMA_REQ_SPI2_WRITE_P, &dmaCfg);

    return 1;
}

s32 spi3SetWriteDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(Spi3TxData);
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_WRITE, &dmaCfg) == 0)
    //    return 0;
    guiSPI3FlashWriteDMAId = marsDMAReq(DMA_REQ_SPI3_WRITE, &dmaCfg);

    return 1;
}

s32 spi4SetWriteDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(Spi4TxData);
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_WRITE, &dmaCfg) == 0)
    //    return 0;
    guiSPI4FlashWriteDMAId = marsDMAReq(DMA_REQ_USB_WRITE, &dmaCfg);

    return 1;
}

s32 spi5SetWriteDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(Spi5TxData);
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;

    //if (dmaConfig(DMA_REQ_SPI_WRITE, &dmaCfg) == 0)
    //    return 0;
    guiSPI5FlashWriteDMAId = marsDMAReq(DMA_REQ_SPI5_WRITE, &dmaCfg);

    return 1;
}
#endif
/*

Routine Description:

	Check if dma is completed.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 spiCheckDmaReadComplete(void)
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

s32 spi2CheckDmaReadComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSPI2FlashReadDMAId);
    guiSPI2FlashReadDMAId = 0x55;
    return (err);
}

s32 spi2CheckDmaReadComplete_P(void)
{
    u8 err;
    err = marsDMACheckReady_P(guiSPI2FlashReadDMAId);
    guiSPI2FlashReadDMAId = 0x55;
    return (err);
}

s32 spi3CheckDmaReadComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSPI3FlashReadDMAId);
    guiSPI3FlashReadDMAId = 0x55;
    return (err);
}

s32 spi4CheckDmaReadComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSPI4FlashReadDMAId);
    guiSPI4FlashReadDMAId = 0x55;
    return (err);
}

s32 spi5CheckDmaReadComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSPI5FlashReadDMAId);
    guiSPI5FlashReadDMAId = 0x55;
    return (err);
}
s32 spiCheckDmaWriteComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSPIFlashWriteDMAId);
    guiSPIFlashWriteDMAId = 0x55;
    return (err);
}

s32 spi2CheckDmaWriteComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSPI2FlashWriteDMAId);
    guiSPI2FlashWriteDMAId = 0x55;
    return (err);
}

s32 spi2CheckDmaWriteComplete_P(void)
{
    u8 err;
    err = marsDMACheckReady_P(guiSPI2FlashWriteDMAId);
    guiSPI2FlashWriteDMAId = 0x55;
    return (err);
}

s32 spi3CheckDmaWriteComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSPI3FlashWriteDMAId);
    guiSPI3FlashWriteDMAId = 0x55;
    return (err);
}

s32 spi4CheckDmaWriteComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSPI4FlashWriteDMAId);
    guiSPI4FlashWriteDMAId = 0x55;
    return (err);
}

s32 spi5CheckDmaWriteComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSPI5FlashWriteDMAId);
    guiSPI5FlashWriteDMAId = 0x55;
    return (err);
}

int spiEnter4ByteMode(void)
{
	u32 unData;
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_ENTER_4BM);
#endif

	SpiCtrl = (spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    SpiIntEn = SPI_INT_BUS_FIN_EN|
               SPI_INT_DMA_END_EN;

    SpiCtrl |= SPI_SSOE_EN;		/* Set CE# low */

#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		unData = (SPI_CMD_ENTER_4BM << 24);
	else
#endif
	unData = SPI_CMD_ENTER_4BM;

#if SPI_ENDIAN_TEST
	DEBUG_SPI("unData cmd: %#x\n", unData);
#endif

    SpiTxData = unData;
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
    unData = SpiRxData;		/* read to clear it */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    
#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif
    return 1;
}

int spiExit4ByteMode(void)
{
	u32 unData;
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_EXIT_4BM);
#endif

	SpiCtrl = (spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    SpiIntEn = SPI_INT_BUS_FIN_EN|
               SPI_INT_DMA_END_EN;

    SpiCtrl |= SPI_SSOE_EN;		/* Set CE# low */
    
#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		unData = (SPI_CMD_EXIT_4BM << 24);
	else
#endif
    unData = SPI_CMD_EXIT_4BM;

#if SPI_ENDIAN_TEST
	DEBUG_SPI("unData cmd: %#x\n", unData);
#endif
    
    SpiTxData = unData;
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
    unData = SpiRxData;		/* read to clear it */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    
#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif
    return 1;
}

int spi4ByteRead(u8* pucDstBuf, u32 unAddr, u32 unSize)
{
    u32 unData;
    int err;
        
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READ);
#endif

	SpiIntEn = SPI_INT_BUS_FIN_EN | SPI_INT_DMA_END_EN;
    SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) |
    			SPI_TX_LEN_8BIT |
    			SPI_EN |
    			SPI_SSOE_EN;

#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		unData = (SPI_CMD_READ_4BM << 24);
	else
#endif
	unData = SPI_CMD_READ_4BM;

    SpiTxData = unData;
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
	SpiIntEn = SPI_INT_BUS_FIN_EN| SPI_INT_DMA_END_EN;
	SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) |
    			SPI_TX_LEN_32BIT |
    			SPI_EN |
    			SPI_SSOE_EN;

#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		unData = ((unAddr & 0xff) << 24) | ((unAddr & 0xff00) << 8) | ((unAddr & 0xff0000) >> 8) | ((unAddr & 0xff000000) >> 24);
	else
#endif
	unData = unAddr;
                
	SpiTxData = unData;
	spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
    unData = SpiRxData;		/* read to clear it */

    spiSetReadDataDma(pucDstBuf, unSize);
    SpiDmaLen = (unSize >> 2);
    SpiCtrl |= SPI_DMA_EN;

    spiSemProcess(SPI_SEM_FLG_DMA_END, SPI_SEM_CMD_IDX_READ);
    err = spiCheckDmaReadComplete();

    SpiCtrl &= ~SPI_DMA_EN;
    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high */

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return err;
}

int spi4ByteWrite(u32 unDst, u8* pucSrc, u32 unSize)
{
    u32	unCnt, unData;
    int ret = 1;
        
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    spiWREN();

	SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl = (spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN|
                SPI_SSOE_EN;

#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		unData = (SPI_CMD_WRITE_4BM << 24);
	else
#endif
	unData = SPI_CMD_WRITE_4BM;

    SpiTxData = unData;    
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl = (spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN|
                SPI_SSOE_EN;
                
#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		unData = ((unDst & 0xff) << 24) | ((unDst & 0xff00) << 8) | ((unDst & 0xff0000) >> 8) | ((unDst & 0xff000000) >> 24);
	else
#endif
	unData = unDst;
				
	SpiTxData = unData;
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    SpiCtrl = (spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_DMA_MODE_TX|
                SPI_SSOE_EN|
                SPI_EN;

    spiSetWriteDataDma(pucSrc, unSize);
    SpiDmaLen = (unSize >> 2);
    SpiCtrl |= SPI_DMA_EN;
    
    ret = spiCheckDmaWriteComplete();

    SpiCtrl &= ~SPI_DMA_EN;
    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to end the progress */

    while(spiRDSR() & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return ret;
}

int spi4ByteSectorErase(u32 unAddr)
{
    u8 ucSRData;
    u32	unCnt, unData;
    int ret = 1;

#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_SECTOR_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    unAddr = ((unAddr / spiSectorSize) * spiSectorSize);		/* abstrct the addres for sector erase */

    spiWREN();

    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) |
                SPI_TX_LEN_8BIT|
                SPI_EN|
                SPI_SSOE_EN;
#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		unData = (SPI_CMD_SEC_ERASE_4BM << 24);
#endif
	unData = SPI_CMD_SEC_ERASE_4BM;

	SpiTxData = unData; 
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_SECTOR_ERASE);

    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl = (spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN|
                SPI_SSOE_EN;

#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		unData = ((unAddr & 0xff) << 24) | ((unAddr & 0xff00) << 8) | ((unAddr & 0xff0000) >> 8) | ((unAddr & 0xff000000) >> 24);
	else
#endif
	unData = unAddr;
				
	SpiTxData = unData;
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_SECTOR_ERASE);
    
    ucSRData = SpiRxData;		/* read to clear it */
    SpiDmaLen = ucSRData;		/* no meaning, just avoid warning message */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    while (spiRDSR() & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return ret;
}

int spi4ByteBlockErase(u32 unAddr)
{
    u8 ucSRData;
    u32	unCnt, unData;
    int ret = 1;

#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_SECTOR_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    unAddr = ((unAddr / spiBlockSize) * spiBlockSize);		/* abstrct the addres for sector erase */

    spiWREN();

    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) |
                SPI_TX_LEN_8BIT|
                SPI_EN|
                SPI_SSOE_EN;
#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		unData = (SPI_CMD_BLK_ERASE_4BM << 24);
#endif
	unData = SPI_CMD_BLK_ERASE_4BM;

	SpiTxData = unData; 
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BLOCK_ERASE);

    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl = (spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN|
                SPI_SSOE_EN;

#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		unData = ((unAddr & 0xff) << 24) | ((unAddr & 0xff00) << 8) | ((unAddr & 0xff0000) >> 8) | ((unAddr & 0xff000000) >> 24);
	else
#endif
	unData = unAddr;
				
	SpiTxData = unData;
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BLOCK_ERASE);
    
    ucSRData = SpiRxData;		/* read to clear it */
    SpiDmaLen = ucSRData;		/* no meaning, just avoid warning message */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    while(spiRDSR() & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return ret;
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
    s32 err;
    
    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }

	if(spiTotalSize > 0x1000000)
		return spi4ByteRead(pucDstBuf, unAddr, unSize);
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READ);
#endif

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI_INT_BUS_FIN_EN|
               SPI_INT_DMA_END_EN;

    SpiCtrl |= SPI_SSOE_EN;		/* Set CE# low */
    
#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
        unData = (unAddr & 0xff) << 24 | (unAddr & 0xff00) << 8 | (unAddr & 0xff0000) >> 8 | SPI_CMD_READ;
    else
#endif
        unData = (SPI_CMD_READ << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
        
#if SPI_ENDIAN_TEST
    DEBUG_SPI("unData cmd: %#x\n", unData);
#endif

    SpiTxData = unData;
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
    unData = SpiRxData;		/* read to clear it */
    SpiDmaLen = unData;		/* no meaning, just avoid warning message */

    /* for high speed polarity alternation */
    if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
        SpiCtrl |= SPI_CPHA_HIGH;

    spiSetReadDataDma(pucDstBuf, unSize);
    SpiDmaLen = unSize/ 4;
    SpiCtrl |=SPI_DMA_EN;

    spiSemProcess(SPI_SEM_FLG_DMA_END, SPI_SEM_CMD_IDX_READ);
    err = spiCheckDmaReadComplete();

    SpiCtrl &= ~SPI_DMA_EN;
    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high */

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return err;

}
#if ICOMMWIFI_SUPPORT

s32 spiRead_Icomm_DMA(u8 cmd,u8* pucDstBuf, u32 unSize)
{
    u32 unData,spiIntStat;
	int i, err_cnt=0,ret;
	
    Spi2Ctrl  =Spi2Ctrl |(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    Spi2CTL2 = 0;				/* Clr CE2# low */
	if((unSize & 3)!= 0)
		unSize += 4 - (unSize & 3);
		
    Spi2TxData = ( ((u32) cmd) << 24);		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    unData= Spi2RxData;		/* read to clear it */
    Spi2DmaLen = unData;		/* no meaning, just avoid warning message */

#if ICOM_SPI_USE_POLLING
    spi2SetReadDataDma_P(pucDstBuf, unSize);
#else
    spi2SetReadDataDma(pucDstBuf, unSize);
#endif
    Spi2DmaLen = unSize >> 2;
    
    Spi2Ctrl |=SPI_DMA_EN;
#if 1    
  #if ICOM_SPI_USE_POLLING
    ret = spi2CheckDmaReadComplete_P();
  #else
    ret=spi2CheckDmaReadComplete();
  #endif
	do
	{
		if(err_cnt > 4000)
		{
			printf("\x1B[91mREAD\x1B[0m\n");
			break;
		}
		err_cnt++;
		spiIntStat = SpiStat;
	} while((spiIntStat & SPI2_SR_DMA_END)==0);
    SpiStat= SPI2_SR_DMA_END;
    Spi2TxData=0xff;
    Spi2Ctrl &= ~SPI_DMA_EN;

#else
  #if 0
    spiSemProcess(SPI2_SEM_FLG_DMA_END, SPI_SEM_CMD_IDX_READ);
  #else
    
	do
	{
		if(err_cnt > 4000)
		{
			printf("\x1B[91mREAD\x1B[0m\n");
			break;
		}
		err_cnt++;
		spiIntStat = SpiStat;
	} while((spiIntStat & SPI2_SR_DMA_END)==0);
    SpiStat= SPI2_SR_DMA_END;
  #endif    
    Spi2TxData=0xff;

  #if ICOM_SPI_USE_POLLING
    ret = spi2CheckDmaReadComplete_P();
  #else
    ret=spi2CheckDmaReadComplete();
  #endif
    Spi2Ctrl &= ~SPI_DMA_EN;
#endif
    return ret;

}
s32 spiRead_Icomm_cpu(u8* pucDstBuf, u32 unSize)
{
	int i=0,err_cnt=0,ret=1;
	u32 spiIntStat;
//printf("11@\n");
	
	Spi2Ctrl &= ~SPI_TX_LEN_32BIT;
	Spi2Ctrl |=	(spiClkDiv << SPI_FREQ_MASK)|
       SPI_TX_LEN_8BIT|
                SPI_EN;
//printf("12@\n");
for (i=0;i<unSize;i++)	
{
//SpiIntEn = SPI2_INT_BUS_FIN_EN;//Ray Modify,disable INT
        Spi2TxData = 0;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
/*!LI¢FFDIpolling !LO¢FFDN!|Ainterrupt?T?{SPI?O!!O_3Bcoready,?¢FFX?F¢FFD[3tspi?C?e3t?!N*/
#if 1 //polling
	do 
	{
		if(err_cnt > 8000)
		{
			ret = 0;
			printf("\x1B[91mREAD_C\x1B[0m\n");
			break;
		}
		err_cnt++;
		spiIntStat = SpiStat;
	} while((spiIntStat & SPI2_SR_BUS_FIN)==0);
    SpiStat= SPI2_SR_BUS_FIN;
#else   // int		
	    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);
#endif
   pucDstBuf[i] = Spi2RxData;	
//printf("13@\n");
}
    return ret;

}

#endif

#if SPI_MULTI_CH_ENABLE
s32 spiRead2(u8* pucDstBuf, u32 unAddr, u32 unSize,u8 SEL1ST)
{
    u32 unData;
    
    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI2_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READ);
#endif

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI2_INT_BUS_FIN_EN|SPI2_INT_DMA_END_EN;
    Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi2CTL2 = 0;				/* Clr CE2# low */

    Spi2TxData = ( ((u32) SPI_CMD_READ) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READID);


    unData = Spi2RxData;		/* read to clear it */
    Spi2DmaLen = unData;		/* no meaning, just avoid warning message */

    /* for high speed polarity alternation */
    if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
        Spi2Ctrl |= SPI_CPHA_HIGH;

    spi2SetReadDataDma(pucDstBuf, unSize);

    Spi2DmaLen = unSize/ 4;
    Spi2Ctrl |=SPI_DMA_EN;

    spiSemProcess(SPI2_SEM_FLG_DMA_END, SPI_SEM_CMD_IDX_READ);

    spi2CheckDmaReadComplete();

    Spi2Ctrl &= ~SPI_DMA_EN;
    Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

#if(RW_PROTECT_SPI)
    OSSemPost(spi2RWHookSemEvt);
#endif

    return 1;

}
s32 spiRead3(u8* pucDstBuf, u32 unAddr, u32 unSize,u8 SEL1ST)
{
    u32 unData;
    
    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI3_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READ);
#endif

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI3_INT_BUS_FIN_EN|SPI3_INT_DMA_END_EN;
    Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi3CTL2 = 0;				/* Clr CE2# low */

    Spi3TxData = ( ((u32) SPI_CMD_READ) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READID);

    unData = Spi3RxData;		/* read to clear it */
    Spi3DmaLen = unData;		/* no meaning, just avoid warning message */

    /* for high speed polarity alternation */
    if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
        Spi3Ctrl |= SPI_CPHA_HIGH;

    spi3SetReadDataDma(pucDstBuf, unSize);

    Spi3DmaLen = unSize/ 4;
    Spi3Ctrl |=SPI_DMA_EN;

    spiSemProcess(SPI3_SEM_FLG_DMA_END, SPI_SEM_CMD_IDX_READ);

    spi3CheckDmaReadComplete();

    Spi3Ctrl &= ~SPI_DMA_EN;

    Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

#if(RW_PROTECT_SPI)
    OSSemPost(spi3RWHookSemEvt);
#endif

    return 1;

}
s32 spiRead4(u8* pucDstBuf, u32 unAddr, u32 unSize,u8 SEL1ST)
{
    u32 unData;
    
    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI4_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READ);
#endif

    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI4_INT_BUS_FIN_EN|SPI4_INT_DMA_END_EN;
    Spi4Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi4CTL2 = 0;				/* Clr CE2# low */

    Spi4TxData = ( ((u32) SPI_CMD_READ) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
    spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READID);


    unData = Spi4RxData;		/* read to clear it */
    Spi4DmaLen = unData;		/* no meaning, just avoid warning message */

    /* for high speed polarity alternation */
    if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
        Spi4Ctrl |= SPI_CPHA_HIGH;

    spi4SetReadDataDma(pucDstBuf, unSize);

    Spi4DmaLen = unSize/ 4;
    Spi4Ctrl |=SPI_DMA_EN;

    spiSemProcess(SPI4_SEM_FLG_DMA_END, SPI_SEM_CMD_IDX_READ);
    spi4CheckDmaReadComplete();

    Spi4Ctrl &= ~SPI_DMA_EN;
    Spi4Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

#if(RW_PROTECT_SPI)
    OSSemPost(spi4RWHookSemEvt);
#endif

    return 1;
}
s32 spiRead5(u8* pucDstBuf, u32 unAddr, u32 unSize,u8 SEL1ST)
{
    u32 unData;
    
    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI5_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READ);
#endif

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI5_INT_BUS_FIN_EN|SPI5_INT_DMA_END_EN;
    Spi5Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi5CTL2 = 0;				/* Clr CE2# low */

    Spi5TxData = ( ((u32) SPI_CMD_READ) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READID);

    unData = Spi5RxData;		/* read to clear it */
    Spi5DmaLen = unData;		/* no meaning, just avoid warning message */

    /* for high speed polarity alternation */
    if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
        Spi5Ctrl |= SPI_CPHA_HIGH;

    spi5SetReadDataDma(pucDstBuf, unSize);

    Spi5DmaLen = unSize/ 4;
    Spi5Ctrl |=SPI_DMA_EN;

    spiSemProcess(SPI5_SEM_FLG_DMA_END, SPI_SEM_CMD_IDX_READ);

    spi5CheckDmaReadComplete();

    Spi5Ctrl &= ~SPI_DMA_EN;
    Spi5Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

#if(RW_PROTECT_SPI)
    OSSemPost(spi5RWHookSemEvt);
#endif

    return 1;

}
#endif
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
    u32 unData = 0;
    u8 i;

#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READID);
#endif

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;		/* Set CE# low */

    /* execute Tx cmd twice to match ESMT serial flash Read ID control property */
    for (i=0; i<2; i++)
    {
        spiID = SpiStat;		/* read to clear interrupt status */
#if SPI_ENDIAN_ENABLE
        if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
            unData = (unData & 0xffffff00) | SPI_CMD_READ_ID;
        else
#endif
            unData = (SPI_CMD_READ_ID << 24) | unData;

        SpiTxData = unData;		/* set cmd and trigger */
        spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READID);
        spiID = SpiRxData;		/* Read-out SpiRxData to clear it. The second time we read is the correct one. */

        if (i==0)
            SpiIntEn = SPI_INT_BUS_FIN_EN;

        /* for high speed polarity alternation */
        if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
            SpiCtrl |= SPI_CPHA_HIGH;

    }

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to complete one process */

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return spiID;

}
#if SPI_MULTI_CH_ENABLE
u32 spiReadID2(u8 SEL1ST)
{

    u32 spiID;
    u32 unDummyAddr = 0x00000000;
    u8 i;

#if(RW_PROTECT_SPI)
    spiSemProcess(SPI2_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READID);
#endif

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI2_INT_BUS_FIN_EN;
    Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi2CTL2 = 0;				/* Clr CE2# low */

    /* execute Tx cmd twice to match ESMT serial flash Read ID control property */
    for (i=0; i<2; i++)
    {
        spiID = SpiStat;		/* read to clear interrupt status */

        Spi2TxData = ( ((u32) SPI_CMD_READ_ID) << 24)| unDummyAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

        spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READID);

        spiID = Spi2RxData;		/* Read-out SpiRxData to clear it. The second time we read is the correct one. */

        if (i==0)
            SpiIntEn = SPI2_INT_BUS_FIN_EN;

        /* for high speed polarity alternation */
        if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
            Spi2Ctrl |= SPI_CPHA_HIGH;

    }

    Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi2Ctrl &= ~SPI_EN;		/* Set CE# high to complete one process */

#if(RW_PROTECT_SPI)
    OSSemPost(spi2RWHookSemEvt);
#endif

    return spiID;

}
u32 spiReadID3(u8 SEL1ST)
{
    u32 spiID;
    u32 unDummyAddr = 0x00000000;
    u8 i;

#if(RW_PROTECT_SPI)
    spiSemProcess(SPI3_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READID);
#endif

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI3_INT_BUS_FIN_EN;
    Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi3CTL2 = 0;				/* Clr CE2# low */


    /* execute Tx cmd twice to match ESMT serial flash Read ID control property */
    for (i=0; i<2; i++)
    {
        spiID = SpiStat;		/* read to clear interrupt status */

        Spi3TxData = ( ((u32) SPI_CMD_READ_ID) << 24)| unDummyAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

        spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READID);

        spiID = Spi3RxData;		/* Read-out SpiRxData to clear it. The second time we read is the correct one. */

        if (i==0)
            SpiIntEn = SPI3_INT_BUS_FIN_EN;

        /* for high speed polarity alternation */
        if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
            Spi3Ctrl |= SPI_CPHA_HIGH;

    }
    Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi3Ctrl &= ~SPI_EN;		/* Set CE# high to complete one process */

#if(RW_PROTECT_SPI)
    OSSemPost(spi3RWHookSemEvt);
#endif

    return spiID;

}
u32 spiReadID4(u8 SEL1ST)
{
    u32 spiID;
    u32 unDummyAddr = 0x00000000;
    u8 i;

#if(RW_PROTECT_SPI)
    spiSemProcess(SPI4_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READID);
#endif

    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI4_INT_BUS_FIN_EN;
    Spi4Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi4CTL2 = 0;				/* Clr CE2# low */


    /* execute Tx cmd twice to match ESMT serial flash Read ID control property */
    for (i=0; i<2; i++)
    {
        spiID = SpiStat;		/* read to clear interrupt status */

        Spi4TxData = ( ((u32) SPI_CMD_READ_ID) << 24)| unDummyAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

        spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READID);

        spiID = Spi4RxData;		/* Read-out SpiRxData to clear it. The second time we read is the correct one. */

        if (i==0)
        {
            SpiIntEn = SPI4_INT_BUS_FIN_EN;
        }

        /* for high speed polarity alternation */
        if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
            Spi4Ctrl |= SPI_CPHA_HIGH;

    }
    Spi4Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi4Ctrl &= ~SPI_EN;		/* Set CE# high to complete one process */

#if(RW_PROTECT_SPI)
    OSSemPost(spi4RWHookSemEvt);
#endif

    return spiID;

}
u32 spiReadID5(u8 SEL1ST)
{
    u32 spiID;
    u32 unDummyAddr = 0x00000000;
    u8 i;

#if(RW_PROTECT_SPI)
    spiSemProcess(SPI5_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_READID);
#endif

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI5_INT_BUS_FIN_EN;
    Spi5Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi5CTL2 = 0;				/* Clr CE2# low */


    /* execute Tx cmd twice to match ESMT serial flash Read ID control property */
    for (i=0; i<2; i++)
    {
        spiID = SpiStat;		/* read to clear interrupt status */

        Spi5TxData = ( ((u32) SPI_CMD_READ_ID) << 24)| unDummyAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

        spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READID);

        spiID = Spi5RxData;		/* Read-out SpiRxData to clear it. The second time we read is the correct one. */

        if (i==0)
            SpiIntEn = SPI5_INT_BUS_FIN_EN;


        /* for high speed polarity alternation */
        if ( SYS_CPU_CLK_FREQ/2/(spiClkDiv+1) >= 24000000 )
            Spi5Ctrl |= SPI_CPHA_HIGH;

    }
    Spi5Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi5Ctrl &= ~SPI_EN;		/* Set CE# high to complete one process */

#if(RW_PROTECT_SPI)
    OSSemPost(spi5RWHookSemEvt);
#endif

    return spiID;

}
#endif

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

#if 0
    spiManufID = (u8) (unID >> 16);
#else
    spiManufID = (u8) (unID >> 8);
    spiDevID = (u8) unID;
#endif

#ifdef SPI_DEBUG_FLAG
    DEBUG_SPI("unID = %#X\n", unID);
    DEBUG_SPI("ucManufID = %#X\n", spiManufID);
    DEBUG_SPI("ucDevID = %#X\n", spiDevID);
#endif

    switch (spiManufID)
    {
        /* Winbond or POINTEC */
        case 0xEF:

            switch (spiDevID)
            {
                case 0x10:
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25X10L -[1Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x11:
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25X20L -[2Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x12:
                    spiTotalSize = 0x80000;	/* 512KB */
                    spiBlockSize = 0x10000;	/* 64KB */
                    spiSectorSize = 0x01000;	/* 4KB */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25X40L -[4Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x13:
                    spiTotalSize = 0x100000;	/* 1MB */
                    spiBlockSize = 0x10000;	/* 64KB */
                    spiSectorSize = 0x01000;	/* 4KB */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25X80L -[8Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x15:
                    spiTotalSize = 0x400000;	/* 4MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("POINTEC-PT25X032 -[32Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x16:
                    spiTotalSize = 0x800000;	// 8MB
                    spiBlockSize = 0x10000;		// 64KB
                    spiSectorSize = 0x1000;		// 4KB
                    spiPageSize = 0x100;		// 256B
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25Q64FVFIG -[64Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x17:
                    spiTotalSize = 0x1000000;	/* 16MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25Q128 -[16MB]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x18:
                    spiTotalSize = 0x2000000;	/* 32MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;		/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25Q256JV -[32MB]\n");
                    DEBUG_SPI("\n");
                    spiEnter4ByteMode();
                    break;

                default:
                    spiTotalSize = 0x00000;	/* 0MB */
                    spiBlockSize = 0x0000;	/* 0KB */
                    spiSectorSize = 0x0000;	/* 0KB */
                    spiPageSize = 0x00;			/* 0B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("Unknown Device!\n");
                    DEBUG_SPI("ucDevID = %#X\n", spiDevID);
                    DEBUG_SPI("\n");
                    #endif
                    return 0;
            }
            break;

        /* EON */
        case 0x1C:

            switch (spiDevID)
            {
                case 0x17:
                    spiTotalSize = 0x1000000;	/* 16MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("EON-EN25F12 -[128Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x16:
                    spiTotalSize = 0x800000;	/* 8MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("EON-EN25F64 -[64Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x15:
                    spiTotalSize = 0x400000;	/* 4MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                   #ifdef SPI_DEBUG_FLAG 
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("EON-EN25F32 -[32Mb]\n");
                    DEBUG_SPI("\n");
                   #endif
                    break;

                case 0x14:
                    spiTotalSize = 0x200000;	/* 2MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;		/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("EON-EN25F16 -[16Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x13:
                    spiTotalSize = 0x100000;	/* 1MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;		/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("EON-EN25F80 -[8Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                default:
                    spiTotalSize = 0x00000;	/* 0MB */
                    spiBlockSize = 0x0000;	/* 0KB */
                    spiSectorSize = 0x0000;	/* 0KB */
                    spiPageSize = 0x00;			/* 0B */
                    DEBUG_SPI("Unknown Device!\n");
                    DEBUG_SPI("ucDevID = %#X\n", spiDevID);
                    DEBUG_SPI("\n");
                    return 0;
            }
            break;

        case 0x8C:	/* ESMT */

            switch (spiDevID)
            {

                case 0x16:
                    spiTotalSize = 0x800000;	/* 8MB */
                    spiBlockSize = 0x10000;		/* 64KB */
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
                    spiTotalSize = 0x00000;	/* 0MB */
                    spiBlockSize = 0x0000;	/* 0KB */
                    spiSectorSize = 0x0000;	/* 0KB */
                    spiPageSize = 0x00;			/* 0B */
                    DEBUG_SPI("Unknow Device!\n");
                    DEBUG_SPI("Quit Verification.\n");
                    DEBUG_SPI("\n");
                    return 1;
            }
            break;

        case 0x9D:	/* ISSI */
            switch (spiDevID)
            {
                case 0x13:
                    spiTotalSize = 0x100000;	// 1MB
                    spiBlockSize = 0x10000;		// 64KB
                    spiSectorSize = 0x1000;		// 4KB
                    spiPageSize = 0x100;		// 256B
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("ISSI-IS25LP080 -[8Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x14:
                    spiTotalSize = 0x200000;	// 2MB
                    spiBlockSize = 0x10000;		// 64KB
                    spiSectorSize = 0x1000;		// 4KB
                    spiPageSize = 0x100;		// 256B
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("ISSI-IS25LP016 -[16Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x15:
                    spiTotalSize = 0x400000;	// 4MB
                    spiBlockSize = 0x10000;		// 64KB
                    spiSectorSize = 0x1000;		// 4KB
                    spiPageSize = 0x100;		// 256B
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("ISSI-IS25LP032 -[32Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x16:
                    spiTotalSize = 0x800000;	// 8MB
                    spiBlockSize = 0x10000;		// 64KB
                    spiSectorSize = 0x1000;		// 4KB
                    spiPageSize = 0x100;		// 256B
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("ISSI-IS25LP064A -[64Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x17:
                    spiTotalSize = 0x1000000;	// 16MB
                    spiBlockSize = 0x10000;		// 64KB
                    spiSectorSize = 0x1000;		// 4KB
                    spiPageSize = 0x100;		// 256B
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("ISSI-IS25LP128A -[128Mb]\n");
                    DEBUG_SPI("\n");
                    break;
                default:
                    DEBUG_SPI("Unknow Device!\n");
                    DEBUG_SPI("Quit Verification.\n");
                    DEBUG_SPI("\n");
                    break;
            }
            break;

        case 0xC2:	/* MXIC */

            switch (spiDevID)
            {

                case 0x15:
                    spiTotalSize = 0x400000;	/* 4MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("MX25-L1606E -[32Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x14:
                    spiTotalSize = 0x200000;	/* 2MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;		/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("MX25-L1606E [16Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                default:
                    spiTotalSize = 0x00000;	/* 0MB */
                    spiBlockSize = 0x0000;	/* 0KB */
                    spiSectorSize = 0x0000;	/* 0KB */
                    spiPageSize = 0x00;			/* 0B */
                    DEBUG_SPI("Unknow Device!\n");
                    DEBUG_SPI("Quit Verification.\n");
                    DEBUG_SPI("\n");
                    return 1;
            }
            break;

        case 0xC8:	/* Giga Device */

            switch (spiDevID)
            {
                case 0x17:
                    spiTotalSize = 0x1000000;	/* 16MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Giga-GD25Q12 -[128Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x16:
                    spiTotalSize = 0x800000;	/* 8MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Giga-GD25Q64 -[64Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;


                case 0x15:
                    spiTotalSize = 0x400000;	/* 4MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Giga-GD25Q32 -[32Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                case 0x14:
                    spiTotalSize = 0x200000;	/* 2MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;		/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    #ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Giga-GD25Q16 [16Mb]\n");
                    DEBUG_SPI("\n");
                    #endif
                    break;

                default:
                    DEBUG_SPI("Unknow Device!\n");
                    DEBUG_SPI("Quit Verification.\n");
                    DEBUG_SPI("\n");
                    return 1;
            }
            break;

        case 0x0B:	/* XTX */
            switch (spiDevID)
            {
                case 0x14:
                    spiTotalSize = 0x200000;	/* 2MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
#ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("XTX-XT25F16B -[16Mb]\n");
                    DEBUG_SPI("\n");
#endif
                    break; 
                    
                case 0x16:
                    spiTotalSize = 0x800000;	/* 8MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
#ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("XTX-XT25F64B -[64Mb]\n");
                    DEBUG_SPI("\n");
#endif
                    break; 
                    
                default:
                    DEBUG_SPI("Unknow Device!\n");
                    DEBUG_SPI("Quit Verification.\n");
                    DEBUG_SPI("\n");
                    return 1;
            }
            break;

        case 0x68:	/* BOYA */
            switch (spiDevID)
            {
                case 0x14:
                    spiTotalSize = 0x200000;	/* 2MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
#ifdef SPI_DEBUG_FLAG
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("BOYA-BY25D16 -[16Mb]\n");
                    DEBUG_SPI("\n");
#endif
                    break; 
                                        
                default:
                    DEBUG_SPI("Unknow Device!\n");
                    DEBUG_SPI("Quit Verification.\n");
                    DEBUG_SPI("\n");
                    return 1;
            }
            break;
            
        default:
            spiTotalSize = 0x00000;	/* 0MB */
            spiBlockSize = 0x0000;	/* 0KB */
            spiSectorSize = 0x0000;	/* 0KB */
            spiPageSize = 0x00;			/* 0B */
            DEBUG_SPI("Unknown Device!\n");
            DEBUG_SPI("ucManufID = %#X\n", spiManufID);
            DEBUG_SPI("\n");
            return 0;

    }
    return 1;

}
#if SPI_MULTI_CH_ENABLE

/*

Routine Description:

	SPI Identification routine.

Arguments:

	None.

Return Value:

	1 - successful
	0 - failed

*/
s32 spiIdentificationSel(u8 SPIno)
{

    u32 	unID;
    DEBUG_SPI("SPIno%d!\n",SPIno);
    switch (SPIno)
    {
        case 1: //spi1
            unID = spiReadID();
            break;
        case 2: //spi2
            unID = spiReadID2(1);
            break;
        case 3: //spi3
            unID = spiReadID3(0);
            break;
        case 4: //spi4
            unID = spiReadID4(1);
            break;
        case 5: //spi5
            unID = spiReadID5(0);
            break;
        default:
            DEBUG_SPI("Unknown Device!\n");
            DEBUG_SPI("SPIno = %d\n", SPIno);
            DEBUG_SPI("\n");
            return 0;
    }

    spiManufID = (u8) (unID >> 8);
    spiDevID = (u8) unID;
    DEBUG_SPI("ucManufID = %#X\n", spiManufID);
    DEBUG_SPI("ucDevID = %#X\n", spiDevID);
    switch (spiManufID)
    {
        /* Winbond or POINTEC */
        case 0xEF:

            switch (spiDevID)
            {
                case 0x10:
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25X10L -[1Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x11:
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25X20L -[2Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x12:
                    spiTotalSize = 0x080000;		/* 512KB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x01000;	/* 4KB */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25X40L -[4Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x13:
                    spiTotalSize = 0x100000;	/* 1MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x01000;	/* 4KB */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Winbond-W25X80L -[8Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x15:
                    spiTotalSize = 0x400000;	/* 4MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("POINTEC-PT25X032 -[32Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                default:
                    DEBUG_SPI("Unknown Device!\n");
                    DEBUG_SPI("ucDevID = %#X\n", spiDevID);
                    DEBUG_SPI("\n");
                    return 0;
            }
            break;

        /* EON */
        case 0x1C:

            switch (spiDevID)
            {
                case 0x17:
                    spiTotalSize = 0x1000000;	/* 16MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("EON-EN25F12 -[128Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x16:
                    spiTotalSize = 0x800000;	/* 8MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("EON-EN25F64 -[64Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x15:
                    spiTotalSize = 0x400000;	/* 4MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("EON-EN25F32 -[32Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x14:
                    spiTotalSize = 0x200000;	/* 2MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;		/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("EON-EN25F16 -[16Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x13:
                    spiTotalSize = 0x100000;	/* 1MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;		/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("EON-EN25F80 -[8Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                default:
                    DEBUG_SPI("EON Unknown Device!\n");
                    DEBUG_SPI("ucDevID = %#X\n", spiDevID);
                    DEBUG_SPI("\n");
                    return 0;
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
                    DEBUG_SPI(" ESMT Unknow Device!\n");
                    DEBUG_SPI("Quit Verification.\n");
                    DEBUG_SPI("\n");
                    return 1;
            }
            break;

        case 0xC2:	/* MXIC */

            switch (spiDevID)
            {

                case 0x15:
                    spiTotalSize = 0x400000;	/* 4MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("MX25L3206E-[32Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x14:
                    spiTotalSize = 0x200000;	/* 2MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;		/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("MX25-L1606E [16Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                default:
                    DEBUG_SPI("Unknow Device!\n");
                    DEBUG_SPI("Quit Verification.\n");
                    DEBUG_SPI("\n");
                    return 1;
            }
            break;

        case 0xC8:	/* Giga Device */

            switch (spiDevID)
            {

                case 0x16:
                    spiTotalSize = 0x800000; /* 8MB */
                    spiBlockSize = 0x10000;  /* 64KB */
                    spiSectorSize = 0x1000; /* 4KB */
                    spiPageSize = 0x100;   /* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("Giga-GD25Q64 -[64Mb]\n");
                    DEBUG_SPI("\n");
                    break;


                case 0x15:
                    spiTotalSize = 0x400000;	/* 4MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;	/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("GD25Q32 -[32Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                case 0x14:
                    spiTotalSize = 0x200000;	/* 2MB */
                    spiBlockSize = 0x10000;		/* 64KB */
                    spiSectorSize = 0x1000;		/* 4KB */
                    spiPageSize = 0x100;			/* 256 B */
                    DEBUG_SPI("\n");
                    DEBUG_SPI("The Device is: ");
                    DEBUG_SPI("GD25Q16 [16Mb]\n");
                    DEBUG_SPI("\n");
                    break;

                default:
                    DEBUG_SPI("Unknow Device!\n");
                    DEBUG_SPI("Quit Verification.\n");
                    DEBUG_SPI("\n");
                    return 1;
            }
            break;


        default:
            DEBUG_SPI("Unknown Device!\n");
            DEBUG_SPI("ucManufID = %#X\n", spiManufID);
            DEBUG_SPI("\n");
            return 0;

    }
    return 1;

}
#endif
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
    u32 unData;

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;

    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */

#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
        unData = (SPI_CMD_WREN << 24) | unData;
    else
#endif
        unData = SPI_CMD_WREN;

    SpiTxData = unData;		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);
    unData = SpiRxData;		/* read to clear it */
    SpiDmaLen = unData;		/* no meaning, just avoid warning message */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
#if SPI_MULTI_CH_ENABLE

s32 spiWREN2(u8 SEL1ST)
{
    u32 unData;

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    SpiIntEn = SPI2_INT_BUS_FIN_EN;
    Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi2CTL2 = 0;				/* Clr CE2# low */

    Spi2TxData = SPI_CMD_WREN;		/* set cmd and trigger */

    spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);

    unData = Spi2RxData;		/* read to clear it */
    Spi2DmaLen = unData;		/* no meaning, just avoid warning message */

    Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi2Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
s32 spiWREN3(u8 SEL1ST)
{
    u32 unData;

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    SpiIntEn = SPI3_INT_BUS_FIN_EN;
    Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi3CTL2 = 0;				/* Clr CE2# low */

    Spi3TxData = SPI_CMD_WREN;		/* set cmd and trigger */
    spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);
    unData = Spi3RxData;		/* read to clear it */
    Spi3DmaLen = unData;		/* no meaning, just avoid warning message */

    Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi3Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */

    return 1;

}

s32 spiWREN4(u8 SEL1ST)
{
    u32 unData;

    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    SpiIntEn = SPI4_INT_BUS_FIN_EN;
    Spi4Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi4CTL2 = 0;				/* Clr CE2# low */

    Spi4TxData = SPI_CMD_WREN;		/* set cmd and trigger */

    spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);

    unData = Spi4RxData;		/* read to clear it */
    Spi4DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi4Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi4Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}

s32 spiWREN5(u8 SEL1ST)
{
    u32 unData;

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    SpiIntEn = SPI5_INT_BUS_FIN_EN;
    Spi5Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi5CTL2 = 0;				/* Clr CE2# low */

    Spi5TxData = SPI_CMD_WREN;		/* set cmd and trigger */

    spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);

    unData = Spi5RxData;		/* read to clear it */
    Spi5DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi5Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi5Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
#endif
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
    u32 unData;

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;

    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */

#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
        unData = (SPI_CMD_WRDI << 24) | unData;
    else
#endif    
        unData = SPI_CMD_WRDI;

    SpiTxData = unData;		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);
    unData= SpiRxData;		/* read to clear it */
    SpiDmaLen = unData;		/* no meaning, just avoid warning message */


    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;

    return 1;

}

#if SPI_MULTI_CH_ENABLE
s32 spiWRDI2(u8 SEL1ST)
{
    u32 unData;

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    SpiIntEn = SPI2_INT_BUS_FIN_EN;
    Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi2CTL2 = 0;				/* Clr CE2# low */

    Spi2TxData = SPI_CMD_WRDI;		/* set cmd and trigger */

    if(SEL1ST)
        spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);
    else
        spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);
    unData = Spi2RxData;		/* read to clear it */
    Spi2DmaLen = unData;		/* no meaning, just avoid warning message */

    Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi2Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
s32 spiWRDI3(u8 SEL1ST)
{
    u32 unData;

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    SpiIntEn = SPI3_INT_BUS_FIN_EN;
    Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi3CTL2 = 0;				/* Clr CE2# low */

    Spi3TxData = SPI_CMD_WRDI;		/* set cmd and trigger */

    spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);
    unData = Spi3RxData;		/* read to clear it */
    Spi3DmaLen = unData;		/* no meaning, just avoid warning message */

    Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi3Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}

s32 spiWRDI4(u8 SEL1ST)
{
    u32 unData;

    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;
    SpiIntEn = SPI4_INT_BUS_FIN_EN;
    Spi4Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi4CTL2 = 0;				/* Clr CE2# low */

    Spi4TxData = SPI_CMD_WRDI;		/* set cmd and trigger */

    spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);

    unData = Spi4RxData;		/* read to clear it */
    Spi4DmaLen = unData;		/* no meaning, just avoid warning message */

    Spi4Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi4Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
s32 spiWRDI5(u8 SEL1ST)
{
    u32 unData;

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    SpiIntEn = SPI5_INT_BUS_FIN_EN;
    Spi5Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi5CTL2 = 0;				/* Clr CE2# low */

    Spi5TxData = SPI_CMD_WRDI;		/* set cmd and trigger */

    spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);

    unData = Spi5RxData;		/* read to clear it */
    Spi5DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi5Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi5Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
#endif
#if 0
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

    return 1;

}
#endif
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
    u32 unData = 0;

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */

#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
        unData = (unData & 0xffffff00) | SPI_CMD_RDSR;
    else
#endif
        unData = (SPI_CMD_RDSR << 24) | unData;

    SpiTxData = unData;		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_RDSR);
    unSRData = SpiRxData;

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */

#if SPI_ENDIAN_ENABLE
	if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
		return (unSRData & 0xff000000) >> 24;
	else

#endif
		return unSRData;

}
#if SPI_MULTI_CH_ENABLE

s32 spiRDSR2(u8 SEL1ST)
{
    u32 unSRData = 1;
    u32 unData = 0;

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI2_INT_BUS_FIN_EN;
    Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi2CTL2 = 0;				/* Clr CE2# low */


    Spi2TxData = ((u32)SPI_CMD_RDSR << 24) |unData;		/* set cmd and trigger */

    spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_RDSR);

    unSRData = Spi2RxData;		/* read to clear it */
    Spi2DmaLen = unSRData;		/* no meaning, just avoid warning message */
    Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi2Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return unSRData;

}
s32 spiRDSR3(u8 SEL1ST)
{
    u32 unSRData = 1;
    u32 unData = 0;

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI3_INT_BUS_FIN_EN;
    Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi3CTL2 = 0;				/* Clr CE2# low */

    Spi3TxData = ((u32)SPI_CMD_RDSR << 24) |unData;		/* set cmd and trigger */

    spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_RDSR);
    unSRData = Spi3RxData;		/* read to clear it */
    Spi3DmaLen = unSRData;		/* no meaning, just avoid warning message */

    Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi3Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return unSRData;

}

s32 spiRDSR4(u8 SEL1ST)
{
    u32 unSRData = 1;
    u32 unData = 0;

    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI4_INT_BUS_FIN_EN;
    Spi4Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi4CTL2 = 0;				/* Clr CE2# low */


    Spi4TxData = ((u32)SPI_CMD_RDSR << 24) |unData;		/* set cmd and trigger */

    spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_RDSR);

    unSRData = Spi4RxData;		/* read to clear it */
    Spi4DmaLen = unSRData;		/* no meaning, just avoid warning message */

    Spi4Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi4Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return unSRData;
}

s32 spiRDSR5(u8 SEL1ST)
{
    u32 unSRData = 1;
    u32 unData = 0;

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI5_INT_BUS_FIN_EN;
    Spi5Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi5CTL2 = 0;				/* Clr CE2# low */


    Spi5TxData = ((u32)SPI_CMD_RDSR << 24) |unData;		/* set cmd and trigger */

    spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_RDSR);

    unSRData = Spi5RxData;		/* read to clear it */
    Spi5DmaLen = unSRData;		/* no meaning, just avoid warning message */

    Spi5Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi5Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return unSRData;
}
#endif
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

#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
        unData = (SPI_CMD_WRSR << 24) | unData;
    else
#endif
        unData = (SPI_CMD_WRSR << 8) | ucDataFilter;

    SpiTxData = unData;		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WRSR);
    unData = SpiRxData;		/* read to clear it */
    SpiDmaLen = unData;		/* no meaning, just avoid warning message */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
#if SPI_MULTI_CH_ENABLE

s32 spiWRSR2(u8 ucSRData,u8 SEL1ST)
{
    u8 ucDataFilter = 0x00;
    u32	unData;

    ucDataFilter &= ucSRData;		/* Disable the entire chip protection */

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_16BIT|
                SPI_EN;

    SpiIntEn = SPI2_INT_BUS_FIN_EN;
    Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi2CTL2 = 0;				/* Clr CE2# low */

    Spi2TxData = (u32) (((u16)SPI_CMD_WRSR << 8)| (u16)ucDataFilter);		/* set cmd and trigger */

    spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WRSR);

    unData = Spi2RxData;		/* read to clear it */
    Spi2DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi2Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}

s32 spiWRSR3(u8 ucSRData,u8 SEL1ST)
{
    u8 ucDataFilter = 0x00;
    u32	unData;

    ucDataFilter &= ucSRData;		/* Disable the entire chip protection */

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_16BIT|
                SPI_EN;

    SpiIntEn = SPI3_INT_BUS_FIN_EN;
    Spi3Ctrl &= (~SPI_SSOE_EN);		/* Clr CE1# low */
    Spi3CTL2 = SPI_SSOE2_EN;		/* Set CE2# low */

    Spi3TxData = (u32) (((u16)SPI_CMD_WRSR << 8)| (u16)ucDataFilter);		/* set cmd and trigger */
    spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WRSR);
    unData = Spi3RxData;		/* read to clear it */

    Spi3DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi3Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
s32 spiWRSR4(u8 ucSRData,u8 SEL1ST)
{
    u8 ucDataFilter = 0x00;
    u32	unData;

    ucDataFilter &= ucSRData;		/* Disable the entire chip protection */

    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_16BIT|
                SPI_EN;

    SpiIntEn = SPI4_INT_BUS_FIN_EN;
    Spi4Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi4CTL2 = 0;				/* Clr CE2# low */

    Spi4TxData = (u32) (((u16)SPI_CMD_WRSR << 8)| (u16)ucDataFilter);		/* set cmd and trigger */

    spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WRSR);

    unData = Spi4RxData;		/* read to clear it */
    Spi4DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi4Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi4Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
s32 spiWRSR5(u8 ucSRData,u8 SEL1ST)
{
    u8 ucDataFilter = 0x00;
    u32	unData;

    ucDataFilter &= ucSRData;		/* Disable the entire chip protection */

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_16BIT|
                SPI_EN;

    SpiIntEn = SPI5_INT_BUS_FIN_EN;
    Spi5Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi5CTL2 = 0;				/* Clr CE2# low */

    Spi5TxData = (u32) (((u16)SPI_CMD_WRSR << 8)| (u16)ucDataFilter);		/* set cmd and trigger */

    spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WRSR);

    unData = Spi5RxData;		/* read to clear it */
    Spi5DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi5Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi5Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
#endif
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
    u32 unData;

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start process */

#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
        unData = (SPI_CMD_EWSR << 24) | unData;
    else
#endif
        unData = SPI_CMD_EWSR;

    SpiTxData = unData;		/* set cmd and trigger */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_EWSR);
    unData = SpiRxData;		/* read to clear it */
    SpiDmaLen = unData;		/* no meaning, just avoid warning message */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to finish the procedure */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
#if SPI_MULTI_CH_ENABLE

s32 spiEWSR2(u8 SEL1ST)
{
    u32	unData;
    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    SpiIntEn = SPI2_INT_BUS_FIN_EN;
    Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi2CTL2 = 0;				/* Clr CE2# low */

    Spi2TxData = SPI_CMD_EWSR;		/* set cmd and trigger */

    spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WRSR);

    unData = Spi2RxData;		/* read to clear it */
    Spi2DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi2Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
s32 spiEWSR3(u8 SEL1ST)
{
    u32	unData;
    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;
    SpiIntEn = SPI3_INT_BUS_FIN_EN;
    Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi3CTL2 = 0;				/* Clr CE2# low */

    Spi3TxData = SPI_CMD_EWSR;		/* set cmd and trigger */

    spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WRSR);
    unData = Spi3RxData;		/* read to clear it */
    Spi3DmaLen = unData;		/* no meaning, just avoid warning message */

    Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi3Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
s32 spiEWSR4(u8 SEL1ST)
{
    u32	unData;

    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;
    SpiIntEn = SPI4_INT_BUS_FIN_EN;
    Spi4Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi4CTL2 = 0;				/* Clr CE2# low */

    Spi4TxData = SPI_CMD_EWSR;		/* set cmd and trigger */

    spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WRSR);

    unData = Spi4RxData;		/* read to clear it */
    Spi4DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi4Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi4Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}

s32 spiEWSR5(u8 SEL1ST)
{
    u32	unData;

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;
    SpiIntEn = SPI5_INT_BUS_FIN_EN;
    Spi5Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi5CTL2 = 0;				/* Clr CE2# low */

    Spi5TxData = SPI_CMD_EWSR;		/* set cmd and trigger */

    spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WRSR);

    unData = Spi5RxData;		/* read to clear it */
    Spi5DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi5Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi5Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    return 1;

}
#endif
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
    u32	unCnt, unData;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }

    if(spiTotalSize > 0x1000000)
		return spi4ByteSectorErase(unAddr);

#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_SECTOR_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    unAddr = ((unAddr/spiSectorSize)*spiSectorSize);		/* abstrct the addres for sector erase */

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR();
        spiWRSR(0x00);
    }
    spiWREN();

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
        unData = (unAddr & 0xff) << 24 | (unAddr & 0xff00) << 8 | (unAddr & 0xff0000) >> 8 | SPI_CMD_SEC_ERASE;
    else
#endif
        unData = (SPI_CMD_SEC_ERASE << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
        
#if SPI_ENDIAN_TEST
    DEBUG_SPI("unData cmd: %#x\n", unData);
#endif

    SpiTxData = unData;
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_SECTOR_ERASE);
    ucSRData = SpiRxData;		/* read to clear it */
    SpiDmaLen = ucSRData;		/* no meaning, just avoid warning message */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */
    SpiCtrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */
    while (spiRDSR() & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return ret;

}

#if SPI_MULTI_CH_ENABLE
s32 spiSectorErase2(u32 unAddr,u8 SEL1ST)
{
    u8 ucSRData;
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    if(SEL1ST)
        spiSemProcess(SPI2_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_SECTOR_ERASE);
    else
        spiSemProcess(SPI3_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_SECTOR_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    unAddr = ((unAddr/spiSectorSize)*spiSectorSize);		/* abstrct the addres for sector erase */

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR2(SEL1ST);
        spiWRSR2(0x00,SEL1ST);
    }
    spiWREN2(SEL1ST);

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    if(SEL1ST)
    {
        SpiIntEn = SPI2_INT_BUS_FIN_EN;
        Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
        Spi2CTL2 = 0;				/* Clr CE2# low */
    }
    else
    {

        SpiIntEn = SPI3_INT_BUS_FIN_EN;
        Spi2Ctrl &= (~SPI_SSOE_EN);		/* Clr CE1# low */
        Spi2CTL2 = SPI_SSOE2_EN;		/* Set CE2# low */
    }

    Spi2TxData = ( ((u32) SPI_CMD_SEC_ERASE) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    if(SEL1ST)
        spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_SECTOR_ERASE);
    else
        spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_SECTOR_ERASE);

    ucSRData = Spi2RxData;		/* read to clear it */
    Spi2DmaLen = ucSRData;		/* no meaning, just avoid warning message */

    if(SEL1ST)
        Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    else
        Spi2CTL2&= ~SPI_SSOE2_EN;		/* Set CE# high to complete one process */

    Spi2Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */


    while (spiRDSR2(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    if(SEL1ST)
        OSSemPost(spi2RWHookSemEvt);
    else
        OSSemPost(spi3RWHookSemEvt);
#endif

    return ret;

}

s32 spiSectorErase3(u32 unAddr,u8 SEL1ST)
{
    u8 ucSRData;
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    if(SEL1ST)
        spiSemProcess(SPI4_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_SECTOR_ERASE);
    else
        spiSemProcess(SPI5_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_SECTOR_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    unAddr = ((unAddr/spiSectorSize)*spiSectorSize);		/* abstrct the addres for sector erase */

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR3(SEL1ST);
        spiWRSR3(0x00,SEL1ST);
    }
    spiWREN3(SEL1ST);

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    if(SEL1ST)
    {
        SpiIntEn = SPI4_INT_BUS_FIN_EN;
        Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
        Spi3CTL2 = 0;				/* Clr CE2# low */
    }
    else
    {

        SpiIntEn = SPI5_INT_BUS_FIN_EN;
        Spi3Ctrl &= (~SPI_SSOE_EN);		/* Clr CE1# low */
        Spi3CTL2 = SPI_SSOE2_EN;		/* Set CE2# low */
    }

    Spi3TxData = ( ((u32) SPI_CMD_SEC_ERASE) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    if(SEL1ST)
        spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_SECTOR_ERASE);
    else
        spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_SECTOR_ERASE);

    ucSRData = Spi3RxData;		/* read to clear it */
    Spi3DmaLen = ucSRData;		/* no meaning, just avoid warning message */

    if(SEL1ST)
        Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    else
        Spi3CTL2&= ~SPI_SSOE2_EN;		/* Set CE# high to complete one process */

    Spi3Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */


    while (spiRDSR3(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }
    
#if(RW_PROTECT_SPI)
    if(SEL1ST)
        OSSemPost(spi4RWHookSemEvt);
    else
        OSSemPost(spi5RWHookSemEvt);
#endif

    return ret;

}
#endif
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
    u32	unCnt, unData;
	int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }

    if(spiTotalSize > 0x1000000)
		return spi4ByteBlockErase(unAddr);
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BLOCK_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    unAddr = ((unAddr/spiBlockSize)*spiBlockSize);		/* abstrct the addres for block erase */

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR();
        spiWRSR(0x00);
    }
    spiWREN();

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
        unData = (unAddr & 0xff) << 24 | (unAddr & 0xff00) << 8 | (unAddr & 0xff0000) >> 8 | SPI_CMD_BLK_ERASE;
    else
#endif
        unData = (SPI_CMD_BLK_ERASE << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
        
#if SPI_ENDIAN_TEST
    DEBUG_SPI("unData cmd: %#x\n", unData);
#endif

    SpiTxData = unData;
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BLOCK_ERASE);

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */
    while (spiRDSR() & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return ret;
}

#if SPI_MULTI_CH_ENABLE
s32 spiBlockErase2(u32 unAddr,u8 SEL1ST)
{
    u8 ucSRData;
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI2_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BLOCK_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    unAddr = ((unAddr/spiSectorSize)*spiSectorSize);		/* abstrct the addres for sector erase */

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR2(SEL1ST);
        spiWRSR2(0x00,SEL1ST);
    }
    spiWREN2(SEL1ST);

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI2_INT_BUS_FIN_EN;
    Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi2CTL2 = 0;				/* Clr CE2# low */

    Spi2TxData = ( ((u32) SPI_CMD_BLK_ERASE) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
    spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BLOCK_ERASE);
    ucSRData = Spi2RxData;		/* read to clear it */
    Spi2DmaLen = ucSRData;		/* no meaning, just avoid warning message */

    Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi2Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */

    while (spiRDSR2(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi2RWHookSemEvt);
#endif

    return ret;

}

s32 spiBlockErase3(u32 unAddr,u8 SEL1ST)
{
    u8 ucSRData;
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI3_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BLOCK_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    unAddr = ((unAddr/spiSectorSize)*spiSectorSize);		/* abstrct the addres for sector erase */

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR3(SEL1ST);
        spiWRSR3(0x00,SEL1ST);
    }
    spiWREN3(SEL1ST);

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;


    SpiIntEn = SPI3_INT_BUS_FIN_EN;
    Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi3CTL2 = 0;				/* Clr CE2# low */

    Spi3TxData = ( ((u32) SPI_CMD_BLK_ERASE) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BLOCK_ERASE);

    ucSRData = Spi3RxData;		/* read to clear it */
    Spi3DmaLen = ucSRData;		/* no meaning, just avoid warning message */

    Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    Spi3Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */

    while (spiRDSR3(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi3RWHookSemEvt);
#endif

    return ret;

}

s32 spiBlockErase4(u32 unAddr,u8 SEL1ST)
{
    u8 ucSRData;
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI4_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BLOCK_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    unAddr = ((unAddr/spiSectorSize)*spiSectorSize);		/* abstrct the addres for sector erase */

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR4(SEL1ST);
        spiWRSR4(0x00,SEL1ST);
    }
    spiWREN4(SEL1ST);

    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI4_INT_BUS_FIN_EN;
    Spi4Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi4CTL2 = 0;				/* Clr CE2# low */

    Spi4TxData = ( ((u32) SPI_CMD_BLK_ERASE) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BLOCK_ERASE);

    ucSRData = Spi4RxData;		/* read to clear it */
    Spi4DmaLen = ucSRData;		/* no meaning, just avoid warning message */

    Spi4Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi4Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */

    while (spiRDSR4(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi4RWHookSemEvt);
#endif

    return ret;

}

s32 spiBlockErase5(u32 unAddr,u8 SEL1ST)
{
    u8 ucSRData;
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI5_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BLOCK_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    unAddr = ((unAddr/spiSectorSize)*spiSectorSize);		/* abstrct the addres for sector erase */

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR5(SEL1ST);
        spiWRSR5(0x00,SEL1ST);
    }
    spiWREN5(SEL1ST);

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI5_INT_BUS_FIN_EN;
    Spi5Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi5CTL2 = 0;				/* Clr CE2# low */

    Spi5TxData = ( ((u32) SPI_CMD_BLK_ERASE) << 24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BLOCK_ERASE);


    ucSRData = Spi5RxData;		/* read to clear it */
    Spi5DmaLen = ucSRData;		/* no meaning, just avoid warning message */

    Spi5Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi5Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */

    while (spiRDSR5(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi5RWHookSemEvt);
#endif

    return ret;

}
#endif
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
    u32 unData, unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_CHIP_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR();
        spiWRSR(0x00);
    }
    spiWREN();

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */

    SpiTxData = SPI_CMD_CHIP_ERASE;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_CHIP_ERASE);
    unData = SpiRxData;		/* read to clear it */
    SpiDmaLen = unData;		/* no meaning, just avoid warning message */

    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to start the progress */

    while (spiRDSR() & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return ret;

}

#if SPI_MULTI_CH_ENABLE
s32 spiChipErase2(u8 SEL1ST)
{
    u32 unData, unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI2_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_CHIP_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR2(SEL1ST);
        spiWRSR2(0x00,SEL1ST);
    }
    spiWREN2(SEL1ST);

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI2_INT_BUS_FIN_EN;
    Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi2CTL2 = 0;				/* Clr CE2# low */

    Spi2TxData = SPI_CMD_CHIP_ERASE;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_CHIP_ERASE);


    unData = Spi2RxData;		/* read to clear it */
    Spi2DmaLen = unData;		/* no meaning, just avoid warning message */

    Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi2Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */


    while (spiRDSR2(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi2RWHookSemEvt);
#endif

    return ret;

}

s32 spiChipErase3(u8 SEL1ST)
{
    u32 unData, unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI3_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_CHIP_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR3(SEL1ST);
        spiWRSR3(0x00,SEL1ST);
    }
    spiWREN3(SEL1ST);

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI3_INT_BUS_FIN_EN;
    Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi3CTL2 = 0;				/* Clr CE2# low */

    Spi3TxData = SPI_CMD_CHIP_ERASE;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_CHIP_ERASE);

    unData = Spi3RxData;		/* read to clear it */
    Spi3DmaLen = unData;		/* no meaning, just avoid warning message */

    Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi3Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */

    while (spiRDSR3(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi3RWHookSemEvt);
#endif

    return ret;
}

s32 spiChipErase4(u8 SEL1ST)
{
    u32 unData, unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI4_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_CHIP_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;
    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR4(SEL1ST);
        spiWRSR4(0x00,SEL1ST);
    }
    spiWREN4(SEL1ST);

    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    if(SEL1ST)
    {
        SpiIntEn = SPI4_INT_BUS_FIN_EN;
        Spi4Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
        Spi4CTL2 = 0;				/* Clr CE2# low */
    }

    Spi4TxData = SPI_CMD_CHIP_ERASE;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_CHIP_ERASE);

    unData = Spi4RxData;		/* read to clear it */
    Spi4DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi4Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi4Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */

    while (spiRDSR4(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi4RWHookSemEvt);
#endif

    return ret;

}

s32 spiChipErase5(u8 SEL1ST)
{
    u32 unData, unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI5_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_CHIP_ERASE);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR5(SEL1ST);
        spiWRSR5(0x00,SEL1ST);
    }
    spiWREN5(SEL1ST);

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI5_INT_BUS_FIN_EN;
    Spi5Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi5CTL2 = 0;				/* Clr CE2# low */

    Spi5TxData = SPI_CMD_CHIP_ERASE;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_CHIP_ERASE);

    unData = Spi5RxData;		/* read to clear it */
    Spi5DmaLen = unData;		/* no meaning, just avoid warning message */
    Spi5Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    Spi5Ctrl &= ~SPI_EN;		/* Set CE# high to finish the procedure */

    while (spiRDSR5(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi5RWHookSemEvt);
#endif

    return ret;

}
#endif

/*

Routine Description:

	WinBond and EON Write routine. Page write 256 Bytes maximum.

Arguments:

	unDst - Destination address data to write to.
	pucSrc - pointer of Data source address.
	unSize - Total size to write. Maximum 256 bytes to write per cycle.

Return Value:

	1 - success
	0 - failed

*/
s32 spiWrite(u32 unDst, u8* pucSrc, u32 unSize)
{
    u32	unCnt, unData;
    int ret = 1;
    
    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }

    if(spiTotalSize > 0x1000000)
		return spi4ByteWrite(unDst, pucSrc, unSize);
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR();
        spiWRSR(0x00);
    }
    spiWREN();

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;			/* Set CE# low to start the procedure */
    
#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LITTLE)
        unData = (unDst & 0xff) << 24 | (unDst & 0xff00) << 8 | (unDst & 0xff0000) >> 8 | SPI_CMD_PAGE_PROGRAM;
    else
#endif
        unData = (SPI_CMD_PAGE_PROGRAM << 24)| unDst;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
#if SPI_ENDIAN_TEST
    DEBUG_SPI("unData cmd: %#x\n", unData);
#endif
    
    SpiTxData = unData;
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    SpiCtrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_DMA_MODE_TX|
                SPI_SSOE_EN|
                SPI_EN;

    spiSetWriteDataDma(pucSrc, unSize);
    SpiDmaLen = unSize/ 4;
    SpiCtrl |=SPI_DMA_EN;


    ret = spiCheckDmaWriteComplete();

    SpiCtrl &= ~SPI_DMA_EN;
    SpiCtrl &= ~SPI_SSOE_EN;		/* Set CE# high to end the progress */

    while (spiRDSR() & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return ret;

}
#if ICOMMWIFI_SUPPORT
s32 spiWrite_Icomm_cpu(u8* pucSrc, u32 unSize)  //CPU mode
{
	int i=0, err_cnt=0, ret=1;
	u32 spiIntStat;

	
	Spi2Ctrl &= ~SPI_TX_LEN_32BIT;
	Spi2Ctrl |=	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    Spi2CTL2 = 0;				/* Clr CE2# low */
	
	//SpiEndian &= ~SPI2_SR_RX_ENDI_LITTLE;
	//SpiEndian &= ~SPI2_SR_TX_ENDI_LITTLE;
for (i=0;i<unSize;i++)	
{
    //SpiIntEn = SPI2_INT_BUS_FIN_EN;//Ray Modify,disable INT
        Spi2TxData =  ( pucSrc[i]) ;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
 /*!LI¢FFDIpolling !LO¢FFDN!|Ainterrupt?T?{SPI?O!!O_3Bcoready,?¢FFX?F¢FFD[3tspi?C?e3t?!N*/
#if 1

#if 1
	do 
	{
		if(err_cnt > 8000)
		{
			//printf("\x1B[96m CPU((spiIntStat & SPI2_SR_BUS_FIN)==0) \x1B[0m\n");
			printf("\x1B[91m.\x1B[0m");
			ret = 0;
			break;
		}
		err_cnt++;
		spiIntStat = SpiStat;
	} while((spiIntStat & SPI2_SR_BUS_FIN)==0);
	//if(err_cnt>50)
	//	printf("\x1B[91m %d \x1B[0m",err_cnt);
    SpiStat= SPI2_SR_BUS_FIN;
#else
	if((spiIntStat & SPI2_SR_BUS_FIN)==0)
	{
		spiIntStat = SpiStat;
	}
	else
	{
		printf("\x1B[92m@\x1B[0m\n");
		ret = 0;
	}
	//if(err_cnt>50)
	//	printf("\x1B[91m %d \x1B[0m",err_cnt);
    SpiStat= SPI2_SR_BUS_FIN;
#endif
#else   		
	    spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_WREN);
#endif

}
    return ret;

}
s32 spiWrite_Icomm_DMA(u8* pucSrc, u32 unSize)  //DMA mode
{
  
    int i;
    u32 remain_len=0;
    int j;
    u8 remain_buf[3];
	bool ret=TRUE;

    remain_len=(unSize & 3);
    if(remain_len>0)
    {
		memcpy(remain_buf,(pucSrc+unSize-remain_len),remain_len);
		unSize-=remain_len;
	}
    
    if(unSize>=4)
    {
    Spi2Ctrl |=	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_DMA_MODE_TX|
                SPI_EN;
 #if ICOM_SPI_USE_POLLING
    spi2SetWriteDataDma_P(pucSrc, unSize);
 #else
    spi2SetWriteDataDma(pucSrc, unSize);
 #endif
    Spi2DmaLen = unSize >> 2;
    Spi2Ctrl |=SPI_DMA_EN;

 #if ICOM_SPI_USE_POLLING
    ret = spi2CheckDmaWriteComplete_P();
 #else
    ret = spi2CheckDmaWriteComplete();
 #endif
    if(ret == 0)
    	return 0; //ERR
    	
    Spi2Ctrl=SPI_SSOE_EN;
    if(remain_len>0)
    {
		ret = spiWrite_Icomm_cpu((u8*)(remain_buf),(u32)remain_len);
	}
    }
	return ret;
}

#endif

#if SPI_MULTI_CH_ENABLE
s32 spiWrite2(u32 unDst, u8* pucSrc, u32 unSize,u8 SEL1ST)
{
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI2_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR2(SEL1ST);
        spiWRSR2(0x00,SEL1ST);
    }
    spiWREN2(SEL1ST);

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;
    SpiIntEn = SPI2_INT_BUS_FIN_EN;
    Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi2CTL2 = 0;				/* Clr CE2# low */


    Spi2TxData = (SPI_CMD_PAGE_PROGRAM<<24)| unDst;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_DMA_MODE_TX|
                SPI_SSOE_EN|
                SPI_EN;

    spi2SetWriteDataDma(pucSrc, unSize);
    Spi2DmaLen = unSize/ 4;
    Spi2Ctrl |=SPI_DMA_EN;


    spi2CheckDmaWriteComplete();

    Spi2Ctrl &= ~SPI_DMA_EN;
    Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */


    while (spiRDSR2(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi2RWHookSemEvt);
#endif

    return ret;

}

s32 spiWrite3(u32 unDst, u8* pucSrc, u32 unSize,u8 SEL1ST)
{
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI3_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR3(SEL1ST);
        spiWRSR3(0x00,SEL1ST);
    }
    spiWREN3(SEL1ST);

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI3_INT_BUS_FIN_EN;
    Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi3CTL2 = 0;				/* Clr CE2# low */

    Spi3TxData = (SPI_CMD_PAGE_PROGRAM<<24)| unDst;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
    spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_DMA_MODE_TX|
                SPI_SSOE_EN|
                SPI_EN;

    spi3SetWriteDataDma(pucSrc, unSize);
    Spi3DmaLen = unSize/ 4;
    Spi3Ctrl |=SPI_DMA_EN;


    spi3CheckDmaWriteComplete();

    Spi3Ctrl &= ~SPI_DMA_EN;
    Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */


    while (spiRDSR3(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi3RWHookSemEvt);
#endif

    return ret;

}
s32 spiWrite4(u32 unDst, u8* pucSrc, u32 unSize, u8 SEL1ST)
{
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI4_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR4(SEL1ST);
        spiWRSR4(0x00,SEL1ST);
    }
    spiWREN4(SEL1ST);

    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI4_INT_BUS_FIN_EN;
    Spi4Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi4CTL2 = 0;				/* Clr CE2# low */


    Spi4TxData = (SPI_CMD_PAGE_PROGRAM<<24)| unDst;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
    spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);


    Spi4Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_DMA_MODE_TX|
                SPI_SSOE_EN|
                SPI_EN;

    spi4SetWriteDataDma(pucSrc, unSize);
    Spi4DmaLen = unSize/ 4;
    Spi4Ctrl |=SPI_DMA_EN;


    spi4CheckDmaWriteComplete();

    Spi4Ctrl &= ~SPI_DMA_EN;
    Spi4Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    while (spiRDSR4(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi4RWHookSemEvt);
#endif

    return ret;

}

s32 spiWrite5(u32 unDst, u8* pucSrc, u32 unSize, u8 SEL1ST)
{
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI5_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR5(SEL1ST);
        spiWRSR5(0x00,SEL1ST);
    }
    spiWREN5(SEL1ST);

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;

    SpiIntEn = SPI5_INT_BUS_FIN_EN;
    Spi5Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
    Spi5CTL2 = 0;				/* Clr CE2# low */

    Spi5TxData = (SPI_CMD_PAGE_PROGRAM<<24)| unDst;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
    spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    Spi5Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_DMA_MODE_TX|
                SPI_SSOE_EN|
                SPI_EN;

    spi5SetWriteDataDma(pucSrc, unSize);
    Spi5DmaLen = unSize/ 4;
    Spi5Ctrl |=SPI_DMA_EN;

    spi5CheckDmaWriteComplete();

    Spi5Ctrl &= ~SPI_DMA_EN;
    Spi5Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */

    while (spiRDSR5(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spi5RWHookSemEvt);
#endif

    return ret;

}

#endif

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
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    spiSemProcess(SPI_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR();
        spiWRSR(0x00);
    }
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
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    OSSemPost(spiRWHookSemEvt);
#endif

    return ret;
}

#if SPI_MULTI_CH_ENABLE
s32 spiByteProgram2(u8 ucData, u32 unAddr, u8 SEL1ST )
{
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    if(SEL1ST)
        spiSemProcess(SPI2_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
    else
        spiSemProcess(SPI3_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR2(SEL1ST);
        spiWRSR2(0x00,SEL1ST);
    }
    spiWREN2(SEL1ST);

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;
    if(SEL1ST)
    {
        SpiIntEn = SPI2_INT_BUS_FIN_EN;
        Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
        Spi2CTL2 = 0;				/* Clr CE2# low */
    }
    else
    {
        SpiIntEn = SPI3_INT_BUS_FIN_EN;
        Spi2Ctrl &= (~SPI_SSOE_EN);		/* Clr CE1# low */
        Spi2CTL2 = SPI_SSOE2_EN;		/* Set CE2# low */
    }

    Spi2TxData = (SPI_CMD_BYTE_PROGRAM<<24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
    if(SEL1ST)
        spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
    else
        spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    Spi2Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    if(SEL1ST)
    {
        SpiIntEn = SPI2_INT_BUS_FIN_EN;
        Spi2Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
        Spi2CTL2 = 0;				/* Clr CE2# low */
    }
    else
    {
        SpiIntEn = SPI3_INT_BUS_FIN_EN;
        Spi2Ctrl &= (~SPI_SSOE_EN);		/* Clr CE1# low */
        Spi2CTL2 = SPI_SSOE2_EN;		/* Set CE2# low */
    }
    Spi2TxData = (u32) ucData;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    if(SEL1ST)
        spiSemProcess(SPI2_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
    else
        spiSemProcess(SPI3_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    if(SEL1ST)
        Spi2Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    else
        Spi2CTL2&= ~SPI_SSOE2_EN;		/* Set CE# high to complete one process */

    while (spiRDSR2(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    if(SEL1ST)
        OSSemPost(spi2RWHookSemEvt);
    else
        OSSemPost(spi3RWHookSemEvt);
#endif

    return ret;

}

s32 spiByteProgram3(u8 ucData, u32 unAddr, u8 SEL1ST )
{
    u32	unCnt;
    int ret = 1;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }
    
#if(RW_PROTECT_SPI)
    if(SEL1ST)
        spiSemProcess(SPI4_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
    else
        spiSemProcess(SPI5_SEM_FLG_RW_HOOK, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
#endif

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)		/* ESMT */
    {
        spiEWSR3(SEL1ST);
        spiWRSR3(0x00,SEL1ST);
    }
    spiWREN3(SEL1ST);

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_32BIT|
                SPI_EN;
    if(SEL1ST)
    {
        SpiIntEn = SPI4_INT_BUS_FIN_EN;
        Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
        Spi3CTL2 = 0;				/* Clr CE2# low */
    }
    else
    {
        SpiIntEn = SPI5_INT_BUS_FIN_EN;
        Spi3Ctrl &= (~SPI_SSOE_EN);		/* Clr CE1# low */
        Spi3CTL2 = SPI_SSOE2_EN;		/* Set CE2# low */
    }

    Spi3TxData = (SPI_CMD_BYTE_PROGRAM<<24)| unAddr;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
    if(SEL1ST)
        spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
    else
        spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    Spi3Ctrl =	(spiClkDiv << SPI_FREQ_MASK)|
                SPI_TX_LEN_8BIT|
                SPI_EN;

    if(SEL1ST)
    {
        SpiIntEn = SPI4_INT_BUS_FIN_EN;
        Spi3Ctrl |= SPI_SSOE_EN;		/* Set CE1# low */
        Spi3CTL2 = 0;				/* Clr CE2# low */
    }
    else
    {
        SpiIntEn = SPI5_INT_BUS_FIN_EN;
        Spi3Ctrl &= (~SPI_SSOE_EN);		/* Clr CE1# low */
        Spi3CTL2 = SPI_SSOE2_EN;		/* Set CE2# low */
    }
    Spi3TxData = (u32) ucData;		/* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    if(SEL1ST)
        spiSemProcess(SPI4_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);
    else
        spiSemProcess(SPI5_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    if(SEL1ST)
        Spi3Ctrl &= ~SPI_SSOE_EN;		/* Set CE# high to complete one process */
    else
        Spi3CTL2&= ~SPI_SSOE2_EN;		/* Set CE# high to complete one process */

    while (spiRDSR3(SEL1ST) & 1)
    {
        if (unCnt == 0)
        {
            DEBUG_SPI("\n[Error]: Operation Time-out\n");
            ret = 0;
        }
        unCnt --;
    }

#if(RW_PROTECT_SPI)
    if(SEL1ST)
        OSSemPost(spi4RWHookSemEvt);
    else
        OSSemPost(spi5RWHookSemEvt);
#endif

    return ret;

}
#endif
/*
 ****************************
 * Funcstions For Verification
 ****************************
*/
#if SPI_ENDIAN_TEST
s32 spiReadCPU(u8* pucDstBuf, u32 unAddr, u32 unSize, u32 lenSwitch)
{
    u32 unData, i;
    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }

    SpiCtrl = (spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_32BIT|
              SPI_EN;

    SpiIntEn = SPI_INT_BUS_FIN_EN|
               SPI_INT_DMA_END_EN;

    SpiCtrl |= SPI_SSOE_EN;     /* Set CE# low */

#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LIT)
        unData = (unAddr & 0xff) << 24 | (unAddr & 0xff00) << 8 | (unAddr & 0xff0000) >> 8 | SPI_CMD_READ;
    else
#endif
        unData = (SPI_CMD_READ << 24)| unAddr;       /* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    DEBUG_SPI("unData cmd: 0x%08x\n", unData);

    SpiTxData = unData;     /* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */
    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
    unData = SpiRxData;     /* read to clear it */

    switch(lenSwitch)
    {
        case 1:
            SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) | SPI_TX_LEN_8BIT | SPI_EN | SPI_SSOE_EN;
            for(i = 0; i < SPI_MAX_BUF_SIZE; i += lenSwitch)
            {
                SpiIntEn = SPI_INT_BUS_FIN_EN;
                SpiTxData = 0;
                spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
                unData = SpiRxData;
                DEBUG_SPI("*(tg8): 0x%08x\n", unData);
#if SPI_ENDIAN_ENABLE
                if(SpiEndian & SPI_SR_RX_ENDI_LIT)
                    *pucDstBuf = (unData >> 24) & 0xff;
                else
#endif
                    *pucDstBuf = unData & 0xff;
                pucDstBuf += lenSwitch;
            }
            break;
        case 2:
            SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) | SPI_TX_LEN_16BIT | SPI_EN | SPI_SSOE_EN;
            for(i = 0; i < SPI_MAX_BUF_SIZE; i += lenSwitch)
            {
                SpiIntEn = SPI_INT_BUS_FIN_EN;
                SpiTxData = 0;
                spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
                unData = SpiRxData;
                DEBUG_SPI("*(tg16): 0x%08x\n", unData);
#if SPI_ENDIAN_ENABLE
                if(SpiEndian & SPI_SR_RX_ENDI_LIT)
                {
                    *(pucDstBuf + 0) = (unData >> 16) & 0xff;
                    *(pucDstBuf + 1) = (unData >> 24) & 0xff;
                }
                else
#endif
                {
                    *(pucDstBuf + 0) = (unData >> 8) & 0xff;
                    *(pucDstBuf + 1) = unData & 0xff;
                }
                pucDstBuf += lenSwitch;

            }
            break;
        case 3:
            SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) | SPI_TX_LEN_24BIT | SPI_EN | SPI_SSOE_EN;
            for(i = 0; i < SPI_MAX_BUF_SIZE; i += lenSwitch)
            {
                SpiIntEn = SPI_INT_BUS_FIN_EN;
                SpiTxData = 0;
                spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
                unData = SpiRxData;
                DEBUG_SPI("*(tg24): 0x%08x\n", unData);
#if SPI_ENDIAN_ENABLE
                if(SpiEndian & SPI_SR_RX_ENDI_LIT)
                {
                    *(pucDstBuf + 0) = (unData >> 8) & 0xff;
                    *(pucDstBuf + 1) = (unData >> 16) & 0xff;
                    *(pucDstBuf + 2) = (unData >> 24) & 0xff;
                }
                else
#endif
                {
                    *(pucDstBuf + 0) = (unData >> 16) & 0xff;
                    *(pucDstBuf + 1) = (unData >> 8) & 0xff;
                    *(pucDstBuf + 2) = unData & 0xff;
                }
                pucDstBuf += lenSwitch;
            }
            break;
        case 4:
            SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) | SPI_TX_LEN_32BIT | SPI_EN | SPI_SSOE_EN;
            for(i = 0; i < SPI_MAX_BUF_SIZE; i += lenSwitch)
            {
                SpiIntEn = SPI_INT_BUS_FIN_EN;
                SpiTxData = 0;
                spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_READ);
                unData = SpiRxData;
                DEBUG_SPI("*(tg32): 0x%08x\n", unData);
#if SPI_ENDIAN_ENABLE
                if(SpiEndian & SPI_SR_RX_ENDI_LIT)
                {
                    *(pucDstBuf + 0) = (unData) & 0xff;
                    *(pucDstBuf + 1) = (unData >> 8) & 0xff;
                    *(pucDstBuf + 2) = (unData >> 16) & 0xff;
                    *(pucDstBuf + 3) = (unData >> 24) & 0xff;
                }
                else
#endif
                {
                    *(pucDstBuf + 0) = (unData >> 24) & 0xff;
                    *(pucDstBuf + 1) = (unData >> 16) & 0xff;
                    *(pucDstBuf + 2) = (unData >> 8) & 0xff;
                    *(pucDstBuf + 3) = unData & 0xff;
                }
                pucDstBuf += lenSwitch;
            }
            break;
        default:
            DEBUG_SPI("[Error]: wrong bit lens.\n");
            return 0;
    }

    SpiCtrl &= ~SPI_SSOE_EN;        /* Set CE# high */
    SpiCtrl &= ~SPI_EN;
    return 1;
}

s32 spiWriteCPU(u32 unDst, u8* pucSrc, u32 unSize, u32 lenSwitch)
{
    u32 *tg32;
    u32 i, unData, unCnt;

    if (spiTotalSize == 0)
    {
        DEBUG_SPI("\n[Error]: spi no space\n");
        return 0;
    }

    if (SYS_CPU_CLK_FREQ == 24000000)
        unCnt = 0x0ffff;
    else if (SYS_CPU_CLK_FREQ == 32000000)
        unCnt = 0x1ffff;
    else if (SYS_CPU_CLK_FREQ == 48000000)
        unCnt = 0x1ffff;
    else/* system freq is 64MHz or 96 MHz */
        unCnt = 0x3ffff;

    if (spiManufID == 0x8C)     /* ESMT */
    {
        spiEWSR();
        spiWRSR(0x00);
    }
    spiWREN();

    SpiCtrl = (spiClkDiv << SPI_FREQ_MASK)|
              SPI_TX_LEN_32BIT|
              SPI_EN;
    SpiIntEn = SPI_INT_BUS_FIN_EN;
    SpiCtrl |= SPI_SSOE_EN;         /* Set CE# low to start the procedure */
    
#if SPI_ENDIAN_ENABLE
    if(SpiEndian & SPI_SR_TX_ENDI_LIT)
        unData = (unDst & 0xff) << 24 | (unDst & 0xff00) << 8 | (unDst & 0xff0000) >> 8 | SPI_CMD_PAGE_PROGRAM;
    else
#endif
        unData = (SPI_CMD_PAGE_PROGRAM << 24)| unDst;       /* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    DEBUG_SPI("unData cmd: 0x%08x\n", unData);
    SpiTxData = unData;

    //SpiTxData = (SPI_CMD_PAGE_PROGRAM<<24)| unDst;        /* set cmd and data to trigger SPI, TX_empty will be "0" simultaneously */

    spiSemProcess(SPI_SEM_FLG_BUS_FIN, SPI_SEM_CMD_IDX_BYTE_PROGRAM);

    switch(lenSwitch)
    {
        case 1:
            SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) | SPI_TX_LEN_8BIT | SPI_SSOE_EN | SPI_EN;
            for(i = 0; i < SPI_MAX_BUF_SIZE; i += lenSwitch)
            {
#if SPI_ENDIAN_ENABLE
                if(SpiEndian & SPI_SR_TX_ENDI_LIT)
                    SpiTxData = (*(pucSrc+i) << 24);
                else
#endif
                    SpiTxData = *(pucSrc+i);
                DEBUG_SPI("*(tg8): 0x%08x\n", SpiTxData);
                SpiIntEn = SPI_INT_BUS_FIN_EN;
            }
            break;
        case 2:
            SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) | SPI_TX_LEN_16BIT | SPI_SSOE_EN | SPI_EN;
            for(i = 0; i < SPI_MAX_BUF_SIZE; i += lenSwitch)
            {
#if SPI_ENDIAN_ENABLE
                if(SpiEndian & SPI_SR_TX_ENDI_LIT)
                    SpiTxData = (*(pucSrc+i+1) << 24) | (*(pucSrc+i) << 16);
                else
#endif
                    SpiTxData = (*(pucSrc+i) << 8) | *(pucSrc+i+1);
                DEBUG_SPI("*(tg16): 0x%08x\n", SpiTxData);
                SpiIntEn = SPI_INT_BUS_FIN_EN;
            }
            break;
        case 3:
            SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) | SPI_TX_LEN_24BIT | SPI_SSOE_EN | SPI_EN;
            for(i = 0; i < SPI_MAX_BUF_SIZE; i += lenSwitch)
            {
#if SPI_ENDIAN_ENABLE
                if(SpiEndian & SPI_SR_TX_ENDI_LIT)
                    SpiTxData = (*(pucSrc+i+2) << 24) | (*(pucSrc+i+1) << 16) | (*(pucSrc+i) << 8);
                else
#endif
                    SpiTxData = (*(pucSrc+i) << 16) | (*(pucSrc+i+1) << 8) | *(pucSrc+i+2);
                DEBUG_SPI("*(tg24): 0x%08x\n", SpiTxData);
                SpiIntEn = SPI_INT_BUS_FIN_EN;
            }
            break;
        case 4:
            SpiCtrl = (spiClkDiv << SPI_FREQ_MASK) | SPI_TX_LEN_32BIT | SPI_SSOE_EN | SPI_EN;
            for(i = 0; i < SPI_MAX_BUF_SIZE; i += lenSwitch)
            {
#if SPI_ENDIAN_ENABLE
                if(SpiEndian & SPI_SR_TX_ENDI_LIT)
                    SpiTxData = (*(pucSrc+i+3) << 24) | (*(pucSrc+i+2) << 16) | (*(pucSrc+i + 1) << 8) | (*(pucSrc+i));
                else
#endif
                    SpiTxData = (*(pucSrc+i) << 24) | (*(pucSrc+i+1) << 16) | (*(pucSrc+i+2) << 8) | *(pucSrc+i+3);
                DEBUG_SPI("*(tg32): 0x%08x\n", SpiTxData);
                SpiIntEn = SPI_INT_BUS_FIN_EN;
            }
            break;
        default:
            DEBUG_SPI("[Error]: wrong bit lens.\n");
            return 0;
    }

    SpiCtrl &= ~SPI_SSOE_EN;        /* Set CE# high to end the progress */


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
#endif

#if 0

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
        for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
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

        for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
        {
            if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
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

        DEBUG_SPI("\n");
        ucCnt = 0;
        for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
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

    for (j=0; j<2; j++)			/* Test twice to verify the function fully */
    {
        DEBUG_SPI("Cycle %d...\n", j+1);

        if (spiBlockErase(unAddr)==0)
            return 0;

        ucCnt = 0;
        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != 0xFF)
                {
                    DEBUG_SPI("Block Erase Failed. spiReadBuf not equals to 0xff\n");

                    for (j=k; (j<(k+10)) && (k<SPI_MAX_BUF_SIZE); j++)
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

        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
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

        DEBUG_SPI("\n");
        ucCnt = 0;
        for (i=unAddr; i<(unAddr + spiBlockSize); i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
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

    for (j=0; j<2; j++)			/* Test twice to verify the function fully */
    {
        DEBUG_SPI("Cycle %d...\n", j+1);

        memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
        if (spiSectorErase(unAddr)==0)
            return 0;

        for (i=unAddr; i<(unAddr + spiSectorSize); i+= SPI_MAX_BUF_SIZE)
        {
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != 0xFF)
                {
                    DEBUG_SPI("Sector Erase Failed. spiReadBuf not equals to 0xff\n");

                    for (j=k; (j<k+10) && (j<SPI_MAX_BUF_SIZE); j++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", j, spiReadBuf[j]);

                    DEBUG_SPI("Sector Erase Failed!\n");
                    return 0;
                }
        }
        /* only for winbond and EON */
        for (i=unAddr; i<(unAddr + spiSectorSize); i+= SPI_MAX_BUF_SIZE)
        {
            if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: WinBondWrite Error!\n");
                return 0;
            }
        }

        for (i=unAddr; i<(unAddr + spiSectorSize); i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            {
                DEBUG_SPI("[Error]: spiRead Error!\n");
                return 0;
            }

            for (k=0; k<SPI_MAX_BUF_SIZE; k++)
                if (spiReadBuf[k] != spiWriteBuf[k])
                {
                    DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", k, spiReadBuf[k]);
                    DEBUG_SPI("spiWriteBuf[%d] = 0x%8x\n", k, spiWriteBuf[k]);
                    DEBUG_SPI("Read & write result don't match!\n");
                    for (j=k; (j<k+10) && (j<SPI_MAX_BUF_SIZE); j++)
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

    unAddr &= 0x000FF000;

    /* Sector Erase*/
    if (spiSectorErase(unAddr)==0)
        return 0;

    if (spiRead(spiReadBuf, unAddr, SPI_MAX_BUF_SIZE) == 0)
        return 0;

    for (i=0; i<SPI_MAX_BUF_SIZE; i++)
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
    for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
    {
        memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
        if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            return 0;

        for (j=0; j<SPI_MAX_BUF_SIZE; j++)
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
    for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
    {
        memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);

        if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
            return 0;

        if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
            return 0;

        for (j=0; j<SPI_MAX_BUF_SIZE; j++)
            if (spiReadBuf[j] != j)
            {
                DEBUG_SPI("Program Failed in Mass RW Testing!\n");
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
        for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (j=0; j<SPI_MAX_BUF_SIZE; j++)
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
        memset(spiWriteBuf, ucPattern, SPI_MAX_BUF_SIZE);

        DEBUG_SPI("Write and check the data.\n");
        /* program the specific data into serial flash */
        for (i=0; i<spiTotalSize; i+= SPI_MAX_BUF_SIZE)
        {
            memset(spiReadBuf, 0, SPI_MAX_BUF_SIZE);
            if (spiWrite(i, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            if (spiRead(spiReadBuf, i, SPI_MAX_BUF_SIZE) == 0)
                return 0;

            for (j=0; j<SPI_MAX_BUF_SIZE; j++)
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

    spiInit();
    if (spiIdentification() == 0)
        DEBUG_SPI("ID Detection Error!\n");

    unAddrForTest = spiReadAddr4Test();
    DEBUG_SPI("Addr For Test = %#x\n", unAddrForTest);

    /* set write data */
    for (i = 0; i < SPI_MAX_BUF_SIZE; i++)
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
        DEBUG_SPI("[11] -	Serial Flash - DMA Read.\n");
        DEBUG_SPI("[12] -	Serial Flash - DMA Write.\n");
        DEBUG_SPI("[13] -	Serial Flash - Routione Identification Test\n");
        DEBUG_SPI("[15] -	Serial Flash - Read Register.\n");
        DEBUG_SPI("[16] -	Serial Flash - Write Register.\n");
        DEBUG_SPI("[17] -	Serial Flash - Dump Write Buf.\n");
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

                if (SYS_CPU_CLK_FREQ == 48000000)
                {
                    DEBUG_SPI("System Freq = 48 MHz.\n");
                    DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (48/2/(spiClkDiv +1)));
                }
                else if (SYS_CPU_CLK_FREQ == 96000000)
                {
                    DEBUG_SPI("System Freq = 96 MHz.\n");
                    DEBUG_SPI("SPI Clock Freq = %d MHz.\n", (96/2/(spiClkDiv +1)));
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

                if (spiRead(spiReadBuf, unAddrForTest, SPI_MAX_BUF_SIZE) == 0)
                    DEBUG_SPI("Read Failed\n");

                {
                    u32 m;

                    for (m=0; m<16; m++)
                        DEBUG_SPI("spiReadBuf[%d] = 0x%8x\n", m, spiReadBuf[m]);

                }

                break;

            case	12:

                if (spiWrite(unAddrForTest, spiWriteBuf, SPI_MAX_BUF_SIZE) == 0)
                    DEBUG_SPI("[Error]: WinBondWrite Error!\n");
                else
                    DEBUG_SPI("WinBondWrite Finished!\n");

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

            case 17:
                DEBUG_SPI("Dump spiWriteBuf data\n");
                {
                    u32 j;
                    for (j=0; j<16; j++)
                        DEBUG_SPI("spiWriteBuf[%d] = %#x\n", j, spiWriteBuf[j]);
                }
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

#endif
