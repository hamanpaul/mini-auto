/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    sdc.c

Abstract:

    The routines of SD Card.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create
    2007/01/01  VCC Modify

*/

#include "general.h"
#include "board.h"

#include "sysapi.h"

#include "sdcapi.h"
#include "sdc.h"
#include "sdcreg.h"
#include "sdcerr.h"
#include "usbapi.h"
//#include "dmaapi.h"
#include <../inc/mars_controller/mars_dma.h>
#include "fsapi.h" /*CY 0718*/
#include "osapi.h"
#include "uiapi.h"
#if (HW_BOARD_OPTION == MR9670_COMMAX_71UM)
#include "..\..\ui\inc\PortControl.h"
#endif

/*********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */
/* define debug print */
//#define sdcDebugPrint 			printf
/* (VCC) DIVISOR0 for Identification mode and DIVISOR1 for Data transfer mode */
/* SD clock divisor */
#define SDC_CLOCK_DIVISOR0         0x000000a0   //Lucian: 64-->160, Identify mode 必須操作在 400KHz 

#if (CHIP_OPTION != CHIP_PA9001D && CHIP_OPTION != CHIP_PA9002A)
#define SDC_CLOCK_DIVISOR1         0x00000001
#define SDC_CLOCK_DIVISORHISPEED   0x00000000 /* (VCC) Not used becuase the SD controller can't run
above 25 MHz. */
#else
#define SDC_CLOCK_DIVISOR1         0x00000002
#define SDC_CLOCK_DIVISORHISPEED   0x00000001 /* (VCC) Not used becuase the SD controller can't run
above 25 MHz. */
#endif

/* SD time out value */
/* cytsai: timeout value = 20 * 100 ms = 2 s */
/* (VCC) Pay attention to the time per timer tick */
#if REAL_CHIP
#define SDC_TIMEOUT        20 	//Civic 070822
#else
#define SDC_TIMEOUT        100
#endif

/* SD initial OCR */
#define SDC_INIT_RCA            0x00000000
#define SDC_INIT_OCR            0x00ff8000

/* Multi-block test */
#define SDC_MULTI_BLK           8

/* SCR data length */
#define SDC_SCR_LEN			8		/*CY 0907*/

/* Switch function data length */
#define SDC_SWITCH_LEN          64

#define SDC_MAX_ACMD_LOOP        2048

/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */

u8  ucSpec2Dot0 = 0; /* 0: 1.1 or earlier, 1: 2.0 or later */
u32 sdcMmc;
u8 WriteMultipleBlock;
u32 sdcRca; /* (VCC) Relative Card Address */

#if VERIFYSD
__align(4) u8 sdcReadBuf[SDC_BLK_LEN * SDC_MULTI_BLK]; /* (VCC) A buffer to read data from SD card */
__align(4) u8 sdcWriteBuf[SDC_BLK_LEN * SDC_MULTI_BLK]; /* (VCC) A buffer to store the tset pattern */
#endif

OS_EVENT *sdcSemEvt;        /* semaphore to synchronize event processing */
OS_EVENT *sdcSemTx_INT;        /* (VCC) Semaphore to synchronize TX_INT interrupt. The interrupt occurs when
                                  one block is written and programming to SD card */
OS_EVENT *sdcSemMulTx_INT;     /* (VCC 20070314) */

u8 sdcCmdRspCmpl, sdcDatRxCrcErr, sdcRspRxCrcErr, sdcDatTxErr, sdcDmaDatTxErr, sdcDmaDatRxErr;
u8 sdcWriteToSD; /* (VCC 20061130) The flag to indicate which the TX_INT occurs */

u32 sdcTotalBlockCount;     /* cytsai */ /* (VCC) Get the total block information from the SD card */

u32 SdcDmaFlag = 1; /* Not used */
u32 sdcTryInvertSDClk=0;


#if VERIFYSD
DMA_CFG_2_E eDmaCfg2; /* (VCC) SD Controller only supports burst mode and 4 bytes data width */
#endif

/*
 *********************************************************************************************************
 *                                             Extern  VARIABLES
 *********************************************************************************************************
 */
extern u8 sdcCurrStat;
extern u8 ucSDCInit;
extern u8 ucSDCUnInit;
extern s32 dcfStorageSize;  /*CY 0718*/
extern OS_FLAG_GRP *gSdUsbProcFlagGrp;		/* FLAG for SD and USB */
extern u8  gInsertCard;
extern u8 syscheckSDStatus;
extern u8 Fileplaying;

/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */
/* Driver Function */

s32 sdcDetectCard(void);
s32 sdcSetCommand(u32, u32, u32);
s32 sdcCheckCommandResponseComplete(void);
s32 sdcCheckDmaReadComplete(void);
s32 sdcCheckDmaWriteComplete(void);
s32 sdcCheckReadError(void);
s32 sdcCheckWriteError(void);
s32 sdcGetResponseR1(SDC_R1 *);
s32 sdcGetResponseR2Cid(SDC_R2_CID *);
s32 sdcGetResponseR2Csd(SDC_R2_CSD *);
s32 sdcGetResponseR3Ocr(SDC_R3_OCR *);
s32 sdcGetResponseR6Rca(SDC_R6_RCA *);
s32 sdcSetReadDataDma(u8 *, u32);
s32 sdcSetWriteDataDma(u8 *, u32);

/* Middleware Function */
s32 sdcCardIdentificationMode(void);
#if SD_SPEC_2DOT0
s32 sdcGetCsd2Dot0(void);
#endif
s32 sdcGetCsd(void);
s32 sdcGetScr(u8*);			/*CY 0907*/
s32 sdcSwitchFunction(u32, u8*);
s32 sdcDataTransferMode(u32);
s32 sdcErase(u32, u32);
s32 sdcReadSingleBlock(u32, u8 *);
s32 sdcReadMultipleBlock(u32, u32, u8 *);
s32 sdcWriteSingleBlock(u32, u8 *);
s32 sdcWriteMultipleBlock(u32, u32, u8 *);
s32 sdcGetCardStatus(u32 *);
u32 sdcGetTotalBlockCount();

#if SDC_POWER_ON_RESET_ENA
  s32 sdcPowerOnRest();
#endif
s32 sdcCheckTransferState(void);
s32 sdcCheckNotProgramState(void);

s32 sdcDelay(u32);
s8 sdcChangeMediaStat(u8);


extern s32 mmcCardIdentificationMode(void);
extern s32 mmcGetCsd(void);
extern s32 mmcDataTransferMode(u32);
extern s32 mmcErase(u32, u32);
extern s32 mmcReadSingleBlock(u32, u8 *);
extern s32 mmcReadMultipleBlock(u32, u32, u8 *);
extern s32 mmcWriteSingleBlock(u32, u8 *);
extern s32 mmcWriteMultipleBlock(u32, u32, u8 *);
extern s32 mmcGetCardStatus(u32 *);
extern s32 mmcIsTranStat(void);


u32 guiSDReadDMAId=0x55, guiSDWriteDMAId=0x55;
/*
 *********************************************************************************************************
 *                                                Driver Function
 *********************************************************************************************************
 */

u32 sdcGetTotalBlockCount()
{
   return sdcTotalBlockCount; 

}

/*

Routine Description:

    SD card detection function.

Arguments:

    None.

Return Value:

    0 - SD is not inserted.
    1 - SD is inserted.

*/

s32 sdcDetectCard(void)
{
    /* (VCC) Enable the codes */
    /* (VCC 20070326) This function fails if running in the FPGA board */
#if SDC_SDCD_ENA
    if ( ( (SdcStat & SDC_DETECT) >> 2) == SDC_CD_IN )
        return SD_OK;

    return errHandle(SD_ERROR_NO_CARD_IS_INSERTED, "No SD Card is inserted #1");
#endif

    return SD_OK;
}

/*

Routine Description:

    Set the command of SD.

Arguments:

    cmdIdx - The command index.
    cmdParm - The command parameter.
    cmdCtrl -The command control of SD Control Register 0

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcSetCommand(u32 cmdIdx, u32 cmdParm, u32 cmdCtrl)
{
    /* set command */
    SdcCmd = 0x40 | cmdIdx; /* start bit + transmission bit = 0x40 */
    SdcParm = cmdParm;
    SdcCtrl0 = cmdCtrl;
    SdcCtrl1 = SDC_CMD_START;

    return SD_OK;
}

/*

Routine Description:

    Check if the command and response cycle is completed.

Arguments:

    None.

Return Value:

    0 - Failure of timeout.
    1 - Success.

*/
s32 sdcCheckCommandResponseComplete(void)
{
    u8 err, cnt;
    u8 level;

    /* check command/response completion */
    level= sysCheckSDCD();  

    if(level==SDC_CD_IN)
        OSSemPend(sdcSemEvt, SDC_TIMEOUT, &err);
    else
        return errHandle(SD_ERROR_FAIL_TO_GET_CARD_STATUS, "SD Remove when card reading ");
    if (err != OS_NO_ERR)
    {
		DEBUG_SDC("Trace: sdcCheckCommandResponseComplete.\n");
		DEBUG_SDC("Error: sdcSemEvt is %d.\n", err);
		
#if SDC_SEM_REPAIR
		if(err == OS_ERR_EVENT_TYPE)
		{
			cnt = sdcSemEvt->OSEventCnt;
			sdcSemEvt = OSSemDel(sdcSemEvt, OS_DEL_ALWAYS, &err);
			if(cnt == 0)
				sdcSemEvt = OSSemCreate(0);
			else
				sdcSemEvt = OSSemCreate(1);
			OSSemPend(sdcSemEvt, SDC_TIMEOUT, &err);
			if(err != OS_NO_ERR)
		    {
				DEBUG_SDC("[E] SDC ReCreate failed. %d\n", err);
				return errHandle(SD_ERROR_COMMAND_RESPONSE_TIME_OUT, "sdcSemEvt is time-out");

			}
		}
		else
			return errHandle(SD_ERROR_COMMAND_RESPONSE_TIME_OUT, "sdcSemEvt is time-out");

#else
		//DEBUG_SDC("SDC: CH0=%d, Ccnt=%d\n", DMA_CH0_ENA, Dma_Ch0_Cyc_Cnt);
		//DEBUG_SDC("SDC: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, Dma_Ch1_Cyc_Cnt);
		//DEBUG_SDC("SDC: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
		//DEBUG_SDC("SDC: DMA_INT = %d\n", g_intStat);
		return errHandle(SD_ERROR_COMMAND_RESPONSE_TIME_OUT, "sdcSemEvt is time-out");

#endif
    }

    if (sdcCmdRspCmpl != 1)
    {
        DEBUG_SDC("Error: sdcCmdRspCmpl is not done.\n");
        if (sdcRspRxCrcErr)
        {
            /* Reponse Rx CRC error */
            DEBUG_SDC("Reponse Rx CRC error (sdcRspRxCrcErr) (SD Host Controller <- SD Card)\n");
            return errHandle(SD_ERROR_REPONSE_RX_CRC_ERROR, "Reponse Rx CRC error");
        }

        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    }

    /* check dma completion */ /* cytsai: 0315 */
    //if (sdcCheckDmaComplete() != 1) /* cytsai: though it is not necessary, the controller does signal DMA completion */
    //    return errHandle(SD_ERROR_DMA_FAILS, "DMA operation fails");
    //  return 0;

    return SD_OK; /* cytsai: 0315 */
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
s32 sdcCheckDmaReadComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSDReadDMAId);
    guiSDReadDMAId = 0x55;
    return (err);

}

s32 sdcCheckDmaWriteComplete(void)
{
    u8 err;
    err = marsDMACheckReady(guiSDWriteDMAId);
    guiSDWriteDMAId = 0x55;
    return (err);
}
    
/*

Routine Description:

    Check if read error.

Arguments:

    None.

Return Value:

    0 - Failure of timeout.
    1 - Success.

*/
s32 sdcCheckReadError(void)
{
#if 1
    if (sdcDatRxCrcErr == 1)
    {
        /* Data RX CRC Error */
//        DEBUG_SDC("Data RX CRC Error (sdcDatRxCrcErr)\n");
        return errHandle(SD_ERROR_DATA_RX_CRC_ERROR, "Data RX CRC Error (sdcDatRxCrcErr)");
    }

    if (sdcDmaDatRxErr == 1)
    {
        /* DMA Data RX Error */
        DEBUG_SDC("DMA Data RX Error (sdcDmaDatRxErr)\n");
        return errHandle(SD_ERROR_DMA_DATA_RX_ERROR, "DMA Data RX Error (sdcDmaDatRxErr)");
    }
#else
    if ((sdcDatRxCrcErr == 1) || (sdcDmaDatRxErr == 1))
    {
        /* Data RX CRC Error or DMA Data RX Error */
        DEBUG_SDC("Error: sdcDatRxCrcErr or sdcDmaDatRxErr.\n");
        return 0;
    }
#endif

    return SD_OK;
}

/*

Routine Description:

    Check if write complete or error.

Arguments:

    None.

Return Value:

    0 - Failure of timeout.
    1 - Success.

*/
s32 sdcCheckMultipleWriteError(u32 count)
{
    u8 err;
    u32 timeout;
    u32 uCRCStatus;

    /* check Data Write to SD card interrupt */
    OSSemPend(sdcSemMulTx_INT, SDC_TIMEOUT * count / 20, &err);

    if (err != OS_NO_ERR)
    {
        return errHandle(SD_ERROR_MUL_TX_TIME_OUT, "sdcSemMulTx_INT is time-out");
    }

    //----Check SD busy----//
    timeout = 0x000ffff;
    do
    {
        timeout--;
        if (timeout == 0)
        {
            return errHandle(SD_ERROR_CARD_IS_NOT_CHANGED_TO_READY, "SDC is Busy!!");
        }
    }
    while (SdcStat & SDC_BUSY);


    //---Check SD FIFO empty---//
    timeout = 0x000ffff;
    do
    {
        timeout--;
        if (timeout == 0)
        {
            return errHandle(SD_ERROR_CARD_IS_NOT_CHANGED_TO_READY, "SDC FIFO is not empty!!\n");
        }
    }
    while ( (SdcStat & SDC_FIFO_EMPTY)==0 );
    
    /* check CRC status */
    uCRCStatus = SdcStat & SDC_CRC_STAT_MASK;
    if (uCRCStatus != SDC_XMT_SUCC) /* CRC status = '010' denotes success */
    {
        DEBUG_SDC("Error: CRC status is %x, not 010.\n", uCRCStatus >> 3);
        return errHandle(SD_ERROR_CRC_ERROR, "CRC status is not 010");
    }
#if 1
    /* check if any tx error */
    if (sdcDatTxErr == 1)
    {
//        DEBUG_SDC("Data Tx CRC Error (sdcDatTxErr) (SD Host Controller -> SD Card)\n");
        return errHandle(SD_ERROR_DATA_TX_CRC_ERROR, "Data Tx CRC Error (sdcDatTxErr) (SD Host Controller -> SD Card)");
    }

    if (sdcDmaDatTxErr == 1)
    {
        DEBUG_SDC("DMA Data Tx Error (sdcDmaDatTxErr) (DMA Controller -> SD Host Controller)\n");
        return errHandle(SD_ERROR_DMA_DATA_TX_ERROR, "DMA Data Tx Error (sdcDmaDatTxErr) (DMA Controller -> SD Host Controller)");
    }
#else
    /* check if any tx error */
    if ((sdcDatTxErr == 1) || (sdcDmaDatTxErr == 1))
    {
        DEBUG_SDC("Error: sdcDatTxErr or sdcDmaDatTxErr.\n");
        return 0;
    }
#endif

    return SD_OK;
}


/*

Routine Description:

    Check if write complete or error.

Arguments:

    None.

Return Value:

    0 - Failure of timeout.
    1 - Success.

*/
s32 sdcCheckWriteError(void)
{
    u8 err;
    u32 timeout;
    u32 uCRCStatus;

#if USE_TX_INT
    /* check Data Write to SD card interrupt */

    OSSemPend(sdcSemTx_INT, SDC_TIMEOUT, &err);

    if (err != OS_NO_ERR)
    {
//        DEBUG_SDC("Error: sdcSemTx_INT is %d.\n", err);
//        DEBUG_SDC("SDC: CH0=%d, Ccnt=%d\n", DMA_CH0_ENA, Dma_Ch0_Cyc_Cnt);
//        DEBUG_SDC("SDC: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, Dma_Ch1_Cyc_Cnt);
//        DEBUG_SDC("SDC: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
//        DEBUG_SDC("SDC: DMA_INT = %d\n", g_intStat);
        return errHandle(SD_ERROR_DATA_WRITE_TO_SD_CARD_TIME_OUT, "sdcSemTx_INT is time-out");
    }

    if (sdcWriteToSD != 1)
    {
//        DEBUG_SDC("Error: sdcWriteToSD is not done.\n");
        DEBUG_SDC("SDC: sdcWriteToSD = %d\n", sdcWriteToSD);
        //DEBUG_SDC("SDC: DMA_INT = %d\n", g_intStat);
        DEBUG_SDC("SDC: SdcStat = %x\n", SdcStat);
        DEBUG_SDC("SDC: SdcIntMask = %x\n", SdcIntMask);
        DEBUG_SDC("SDC: SdcIntStat = %x\n", SdcIntStat);
        //return errHandle(SD_ERROR_WRITE_TO_SD_TIME_OUT, "sdcWriteToSD is not done");
    }

#else
    /*CY 0601 S*/
    /* check SD from busy to ready */
    timeout = 0x000fffff;

    do
    {
        timeout--;

        if (timeout == 0)
        {
            //            DEBUG_SDC("Error: SDC is not changed from busy to ready.\n");
            return errHandle(SD_ERROR_CARD_IS_NOT_CHANGED_TO_READY, "SDC is not changed from busy to ready");
        }
    }
    while (SdcStat & SDC_BUSY);
#endif

    /* check card leaves program state */
    //if (sdcCheckNotProgramState() != 1)
    //  return 0;
    /*CY 0601 E*/

    /* For debug */
#if 0
    DEBUG_SDC("Polling count=%d\n", 0xfffff - timeout);
#endif

    /* check CRC status */
    uCRCStatus = SdcStat & SDC_CRC_STAT_MASK;
    if (uCRCStatus != SDC_XMT_SUCC) /* CRC status = '010' denotes success */
    {
        DEBUG_SDC("Error: CRC status is %x, not 010.\n", uCRCStatus >> 3);
        return errHandle(SD_ERROR_CRC_ERROR, "CRC status is not 010");
    }
#if 1
    /* check if any tx error */
    if (sdcDatTxErr == 1)
    {
//        DEBUG_SDC("Data Tx CRC Error (sdcDatTxErr) (SD Host Controller -> SD Card)\n");
        return errHandle(SD_ERROR_DATA_TX_CRC_ERROR, "Data Tx CRC Error (sdcDatTxErr) (SD Host Controller -> SD Card)");
    }

    if (sdcDmaDatTxErr == 1)
    {
//        DEBUG_SDC("DMA Data Tx Error (sdcDmaDatTxErr) (DMA Controller -> SD Host Controller)\n");
        return errHandle(SD_ERROR_DMA_DATA_TX_ERROR, "DMA Data Tx Error (sdcDmaDatTxErr) (DMA Controller -> SD Host Controller)");
    }
#else
    /* check if any tx error */
    if ((sdcDatTxErr == 1) || (sdcDmaDatTxErr == 1))
    {
        DEBUG_SDC("Error: sdcDatTxErr or sdcDmaDatTxErr.\n");
        return 0;
    }
#endif

    return SD_OK;
}

/*

Routine Description:

    Check if the SD card is ready.

Arguments:

    None.

Return Value:

    0 - Failure of timeout.
    1 - Success.

*/
s32 sdcCheckCardBusy(u32 uTimeout)
{
    u8 err;
    u32 timeout;
    u32 uCRCStatus;

    /* VCC */
    /* check SD from busy to ready */
    //Lucian: wait max 250 ms.
    //poring
    timeout = uTimeout;
    while (!(SdcStat & SDC_CARD_READY))
    {
        timeout--; 
        if (timeout == 0)
        {
            break;
        }
    }

    //use Time delay//
    timeout = 8;
    while (!(SdcStat & SDC_CARD_READY))
    {
        OSTimeDly(1);
        if (timeout == 0)
        {
            return errHandle(SD_ERROR_CARD_IS_NOT_CHANGED_TO_READY, "SDC is not changed from busy to ready");
        }
        timeout--;
        DEBUG_SDC("-->SD Wait ready!\n");
    }
    
    
    return SD_OK;
}

//Lucian: Special purpose for format.
s32 sdcCheckCardBusy_Erase(u32 uTimeout)
{
    u8 err;
    u32 timeout;
    u32 uCRCStatus;

    /* VCC */
    /* check SD from busy to ready */
    timeout = uTimeout;

    do
    {
        OSTimeDly(2); 
        timeout--;
        if (timeout == 0)
        {
            return errHandle(SD_ERROR_CARD_IS_NOT_CHANGED_TO_READY, "SDC is not changed from busy to ready");
        }
    }while (!(SdcStat & SDC_CARD_READY));
    
    
    return SD_OK;
}
/*

Routine Description:

    Get the response R1.

Arguments:

    rspR1 - The response R1.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcGetResponseR1(SDC_R1 *rspR1)
{
    u32 *rspPtr = (u32 *)((void *)rspR1);

#if 0
    if (sdcRspRxCrcErr)
    {
        /* Reponse Rx CRC error */
        DEBUG_SDC("Reponse Rx CRC error (sdcRspRxCrcErr) (SD Host Controller <- SD Card)\n");
//        return 0;
    }
#endif

    *rspPtr++ = (SdcRsp2 << 24);
    *rspPtr = SdcRsp1;

    return SD_OK;
}

/*

Routine Description:

    Get the response R2 of CID.

Arguments:

    rspR2Cid - The response R2 of CID.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcGetResponseR2Cid(SDC_R2_CID *rspR2Cid)
{
    u32 *rspPtr = (u32 *)((void *)rspR2Cid);

#if 0
    if (sdcRspRxCrcErr)
    {
        /* Reponse Rx CRC error */
        DEBUG_SDC("Reponse Rx CRC error (sdcRspRxCrcErr) (SD Host Controller <- SD Card)\n");
//        return 0;
    }
#endif

    *rspPtr++ = (SdcRsp5 << 24);
    *rspPtr++ = SdcRsp4;
    *rspPtr++ = SdcRsp3;
    *rspPtr++ = SdcRsp2;
    *rspPtr = SdcRsp1;

    return SD_OK;
}

/*

Routine Description:

    Get the response R2 of CSD.

Arguments:

    rspR2Csd - The response R2 of CSD.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcGetResponseR2Csd(SDC_R2_CSD *rspR2Csd)
{
    u32 *rspPtr = (u32 *)((void *)rspR2Csd);

#if 0
    if (sdcRspRxCrcErr)
    {
        /* Reponse Rx CRC error */
        DEBUG_SDC("Reponse Rx CRC error (sdcRspRxCrcErr) (SD Host Controller <- SD Card)\n");
//        return 0;
    }
#endif

    *rspPtr++ = (SdcRsp5 << 24);
    *rspPtr++ = SdcRsp4;
    *rspPtr++ = SdcRsp3;
    *rspPtr++ = SdcRsp2;
    *rspPtr = SdcRsp1;

    return SD_OK;
}

/*

Routine Description:

    Get the response R3 of OCR.

Arguments:

    rspR3Ocr - The response R3 of OCR.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcGetResponseR3Ocr(SDC_R3_OCR *rspR3Ocr)
{
    u32 *rspPtr = (u32 *)((void *)rspR3Ocr);

#if 0
    if (sdcRspRxCrcErr)
    {
        /* Reponse Rx CRC error */
        DEBUG_SDC("Reponse Rx CRC error (sdcRspRxCrcErr) (SD Host Controller <- SD Card)\n");
//        return 0;
    }
#endif

    *rspPtr++ = (SdcRsp2 << 24);
    *rspPtr = SdcRsp1;

    return SD_OK;
}

/*

Routine Description:

    Get the response R6 of RCA.

Arguments:

    rspR6Rca - The response R6 of RCA.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcGetResponseR6Rca(SDC_R6_RCA *rspR6Rca)
{
    u32 *rspPtr = (u32 *)((void *)rspR6Rca);

#if 0
    if (sdcRspRxCrcErr)
    {
        /* Reponse Rx CRC error */
        DEBUG_SDC("Reponse Rx CRC error (sdcRspRxCrcErr) (SD Host Controller <- SD Card)\n");
//        return 0;
    }
#endif

    *rspPtr++ = (SdcRsp2 << 24);
    *rspPtr = SdcRsp1;

    return SD_OK;
}

#if SD_SPEC_2DOT0
/*

Routine Description:

    Get the response R7 of CMD8.

Arguments:

    rspR7 - The response R7 of CMD8.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcGetResponseR7(SDC_R7 *rspR7)
{
    u32 *rspPtr = (u32 *)((void *)rspR7);

#if 0
    if (sdcRspRxCrcErr)
    {
        /* Reponse Rx CRC error */
        DEBUG_SDC("Reponse Rx CRC error (sdcRspRxCrcErr) (SD Host Controller <- SD Card)\n");
//        return 0;
    }
#endif

    *rspPtr++ = (SdcRsp2 << 24);
    *rspPtr = SdcRsp1;

    return SD_OK;
}
#endif

/*

Routine Description:

	Set read status dma.

Arguments:

	buf - The buffer to read to.
	siz - The size to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
/*CY 0907*/
s32 sdcSetReadStatusDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)&(SdcData);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / 4;
    dmaCfg.burst = 0;	/*CY 0907*/
    //if (dmaConfig(DMA_REQ_SD_READ, &dmaCfg) == 0)
    //    return 0;
    guiSDReadDMAId = marsDMAReq(DMA_REQ_SD_READ, &dmaCfg);
    
    return 1;
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
s32 sdcSetReadDataDma(u8 *buf, u32 siz)
{
    DMA_CFG dmaCfg;
    if(guiSDReadDMAId != 0x55)
    {
        DEBUG_SDC("Warning! Read DMA request error!! guiSDReadDMAId=%d\n",guiSDReadDMAId);
        return 0;
    }
    /* set read data dma */
    dmaCfg.src = (u32)&(SdcData);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / 16;
    dmaCfg.burst = 1;	/*CY 0907*/
    guiSDReadDMAId = marsDMAReq(DMA_REQ_SD_READ, &dmaCfg);
    
    return SD_OK;
}

/*

Routine Description:

    Set data write dma.

Arguments:

    buf - The buffer to write from.
    siz - The size to write.

Return Value:

    0 - Failure.
    1 - Success.

*/
	s32 sdcSetWriteDataDma(u8 *buf, u32 siz)
	{
		DMA_CFG dmaCfg;
	
		if(guiSDWriteDMAId != 0x55)
		{
			DEBUG_SDC("Warning! Write DMA request error!! guiSDWriteDMAId=%d\n",guiSDWriteDMAId);
			return 0;
		}
		
		/* set write data dma */
		dmaCfg.src = (u32)buf;
		dmaCfg.dst = (u32)&(SdcData);
		dmaCfg.cnt = siz / 16;
		dmaCfg.burst = 1;	/*CY 0907*/
		//if (dmaConfig(DMA_REQ_SD_WRITE, &dmaCfg) == 0)
		//	  return errHandle(SD_ERROR_DMA_CONTROLLER_WRITE_ERROR, "DMA controller fails to set to write");
		guiSDWriteDMAId = marsDMAReq(DMA_REQ_SD_WRITE, &dmaCfg);
		
		return SD_OK;
	}


/*
 *********************************************************************************************************
 *                                                Middleware Function
 *********************************************************************************************************
 */

/* (VCC) FOR Spec 2.0 */
#if SD_SPEC_2DOT0
/*

Routine Description:

    Card identification mode of SD compliant to Spec. 2.0 .

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcCardIdentificationMode2Dot0(void)
{
    s32 i;
    u32 temp = 0;
    SDC_R1 rspR1;
    SDC_R3_OCR rspR3Ocr;
    SDC_R2_CID rspR2Cid;
    SDC_R6_RCA rspR6Rca;
    SDC_R7     rspR7;
    u32        uCheckPattern = 0xAA;
    u32 count;
	u8	ucTestHighCap = 0;

    ucSpec2Dot0 = 0;

    /* SDC configuration */
    SdcCfg = SDC_CLOCK_DIVISOR0 | SDC_ENA | SDC_BUS_4BIT | SDC_SD;

    /* initialization delay to supply ramp up time = max(1 msec, 74 clock cycles) */
    for (i = 0; i < 0xffff; i++)
        temp++;

    /* CMD0 */
    sdcSetCommand(SDC_CMD_GO_IDLE_STATE, 0, SDC_CTRL0_CMD);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");

    /* CMD8 */
    sdcSetCommand(SDC_CMD_SEND_IF_COND, (0x1 << 8) | uCheckPattern, SDC_CTRL0_CMD_RSP_R7);

    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() == 1)
        ucSpec2Dot0 = 1;
    else
    {
        /* CMD55 */
        sdcSetCommand(SDC_CMD_APP_CMD, SDC_INIT_RCA, SDC_CTRL0_CMD_RSP_R1);

        /* check command response completion */
        if (sdcCheckCommandResponseComplete() != 1)
            return errHandle(SD_ERROR_NO_CARD_IS_INSERTED, "No SD Card is inserted #2");

        /* get response */
        sdcGetResponseR1(&rspR1);

        /* ACMD41 with Argument[30] = 1(HCS) */
        sdcSetCommand(SDC_ACMD_SD_SEND_OP_COND, 0x40000000 | SDC_INIT_OCR, SDC_CTRL0_CMD_RSP_R3);
        /* check command/response completion */
        if (sdcCheckCommandResponseComplete() != 1)
             return errHandle(SD_ERROR_NO_CARD_IS_INSERTED, "No SD Card is inserted #3");
    }

    if (ucSpec2Dot0)
    {
        /*
           Initialization and identification flow for v2.0 
        */

        /* get response */
        sdcGetResponseR7(&rspR7);

        /* Check the voltage range and check pattern which are return from SD card */
        if ((rspR7.voltage_accepted != 0x1) || rspR7.check_pattern != uCheckPattern)
        {
            DEBUG_SDC("Voltage range isn't supported or check pattern doesn't match:0x%x\n",SdcRsp2);
            return errHandle(SD_ERROR_WRONG_VOLTAGE_RANGE_OR_PATTERN, "Voltage range isn't supported or check pattern doesn't match");
        }
        //    ucSpec2Dot0 = 1;

        count=0;
        do
        {
            /* CMD55 */
            sdcSetCommand(SDC_CMD_APP_CMD, SDC_INIT_RCA, SDC_CTRL0_CMD_RSP_R1);

            /* check command response completion */
            if (sdcCheckCommandResponseComplete() != 1)
                return errHandle(SD_ERROR_NO_CARD_IS_INSERTED, "No SD Card is inserted #4");
                //return 2; /* it is an MMC */
            /* get response */
            sdcGetResponseR1(&rspR1);

            /* ACMD41 with Argument[30] = 1(HCS) */
            sdcSetCommand(SDC_ACMD_SD_SEND_OP_COND, 0x40000000 | SDC_INIT_OCR, SDC_CTRL0_CMD_RSP_R3);
            /* check command/response completion */
            if (sdcCheckCommandResponseComplete() != 1)
                return errHandle(SD_ERROR_NO_CARD_IS_INSERTED, "No SD Card is inserted #5");
                //return 2; /* it is an MMC */
            /* get response */
            sdcGetResponseR3Ocr(&rspR3Ocr);
            count ++;
            if(count > SDC_MAX_ACMD_LOOP) 
            {
                DEBUG_SDC("SDC ACMD Loop is overflow!! count=%d\n",count);
                return 0;
            }
        }
        while ((rspR3Ocr.ocr&SDC_OCR_POWER_UP_BUSY) == 0);

        DEBUG_SDC("Check SDC ready count=%d,OCR=0x%x\n",count,rspR3Ocr.ocr);
        if( (rspR3Ocr.ocr & 0x00300000) == 0)
        {
           DEBUG_SDC("Error!! SDC VDD voltage windown not support 3.3V !\n");
           return 0;
        }
        /* Check CCS(Card Capacity Status) field of Response token to ensure it's a SD card compliant
           to Spec. 2.0 */
        if (!(rspR3Ocr.ocr & 0x40000000))
        {
            DEBUG_SDC("Ver 2.0 Standard Capacity SD Memory Card is inserted\n");
            ucSpec2Dot0 = 0;
        }
        else
        {
	        ucTestHighCap = 1;
            DEBUG_SDC("Ver 2.0 High Capacity SD Memory Card is inserted\n");
        }
    }
    else
    {
        /*
           Initialization and identification flow for v1.1 
        */
        count=0;
        do
        {
            /* CMD55 */
            sdcSetCommand(SDC_CMD_APP_CMD, SDC_INIT_RCA, SDC_CTRL0_CMD_RSP_R1);
            /* check command response completion */
            if (sdcCheckCommandResponseComplete() != 1)
                return 2; /* it is an MMC */
            /* get response */
            sdcGetResponseR1(&rspR1);

            /* ACMD41 */
            sdcSetCommand(SDC_ACMD_SD_SEND_OP_COND, SDC_INIT_OCR, SDC_CTRL0_CMD_RSP_R3);
            /* check command/response completion */
            if (sdcCheckCommandResponseComplete() != 1)
                return 2; /* it is an MMC */
            /* get response */
            sdcGetResponseR3Ocr(&rspR3Ocr);
            count ++;
            if(count>SDC_MAX_ACMD_LOOP) 
            {
                DEBUG_SDC("SDC ACMD Loop is overflow!! count=%d\n",count);
                return 0;
            }    

        }
        while ((rspR3Ocr.ocr&SDC_OCR_POWER_UP_BUSY) == 0);
        DEBUG_SDC("Check SDC ready count=%d,OCR=0x%x\n",count,rspR3Ocr.ocr);

    }

    if( (rspR3Ocr.ocr & 0x00300000) == 0)
    {
       DEBUG_SDC("Error!! SDC VDD voltage windown not support 3.3V !\n");
       return 0;
    }
    /* CMD2 */
    sdcSetCommand(SDC_CMD_ALL_SEND_CID, 0, SDC_CTRL0_CMD_RSP_R2);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR2Cid(&rspR2Cid);

    /* CMD3 */
    sdcSetCommand(SDC_CMD_SEND_RELATIVE_ADDR, 0, SDC_CTRL0_CMD_RSP_R6);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR6Rca(&rspR6Rca);
    /* save RCA */
    sdcRca = ((u32)rspR6Rca.rca) << 16;

    /* get CSD */
    sdcGetCsd2Dot0();

    return SD_OK;
}
#endif

/*

Routine Description:

    Card identification mode of SD.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcCardIdentificationMode(void)
{
    s32 i;
    u32 temp = 0;
    SDC_R1 rspR1;
    SDC_R3_OCR rspR3Ocr;
    SDC_R2_CID rspR2Cid;
    SDC_R6_RCA rspR6Rca;
    u32 count;

    /* SDC configuration */
    SdcCfg = SDC_CLOCK_DIVISOR0 | SDC_ENA | SDC_BUS_4BIT | SDC_SD;

    /* initialization delay to supply ramp up time = max(1 msec, 74 clock cycles) */
    for (i = 0; i < 0xffff; i++)
        temp++;

    /* CMD0 */
    sdcSetCommand(SDC_CMD_GO_IDLE_STATE, 0, SDC_CTRL0_CMD);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");

    count=0;
    do
    {
        /* CMD55 */
        sdcSetCommand(SDC_CMD_APP_CMD, SDC_INIT_RCA, SDC_CTRL0_CMD_RSP_R1);
        /* check command response completion */
        if (sdcCheckCommandResponseComplete() != 1)
            return 2; /* it is an MMC */
        /* get response */
        sdcGetResponseR1(&rspR1);

        /* ACMD41 */
        sdcSetCommand(SDC_ACMD_SD_SEND_OP_COND, SDC_INIT_OCR, SDC_CTRL0_CMD_RSP_R3);
        /* check command/response completion */
        if (sdcCheckCommandResponseComplete() != 1)
            return 2; /* it is an MMC */
        /* get response */
        sdcGetResponseR3Ocr(&rspR3Ocr);
        count ++;
        if(count > SDC_MAX_ACMD_LOOP) 
        {
            DEBUG_SDC("SDC ACMD Loop is overflow!! count=%d\n",count);
            return 0;
        }
            
    }
    while ((rspR3Ocr.ocr & SDC_OCR_POWER_UP_BUSY) == 0);

    DEBUG_SDC("Check SDC ready count=%d,OCR=0x%x\n",count,rspR3Ocr.ocr);
    if( (rspR3Ocr.ocr & 0x00300000) == 0)
    {
       DEBUG_SDC("Error!! SDC VDD voltage windown not support 3.3V !\n");
       return 0;
    }
    /* CMD2 */
    sdcSetCommand(SDC_CMD_ALL_SEND_CID, 0, SDC_CTRL0_CMD_RSP_R2);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR2Cid(&rspR2Cid);

    /* CMD3 */
    sdcSetCommand(SDC_CMD_SEND_RELATIVE_ADDR, 0, SDC_CTRL0_CMD_RSP_R6);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR6Rca(&rspR6Rca);
    /* save RCA */
    sdcRca = ((u32)rspR6Rca.rca) << 16;

    /* get CSD */
    sdcGetCsd();

    return SD_OK;
}

#if SD_SPEC_2DOT0
/*

Routine Description:

    Get CSD of SD.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcGetCsd2Dot0(void)
{
    SDC_R2_CSD rspR2Csd;
    u32 capacity, block_num, mult, block_len;

    /* CMD9 */
    sdcSetCommand(SDC_CMD_SEND_CSD, sdcRca, SDC_CTRL0_CMD_RSP_R2);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR2Csd(&rspR2Csd);

    /* calculate volume capacity */
    block_len = 1 << ((rspR2Csd.csd[2] & SDC_CSD2_READ_BL_LEN_MASK) >> SDC_CSD2_READ_BL_LEN_SHFT);
    if (ucSpec2Dot0)
        block_num = ( ((rspR2Csd.csd[2] & SDC_CSD2_2Dot0_DEVICE_SIZE_MASK) << SDC_CSD2_2Dot0_DEVICE_SIZE_SHFT) +
                      ((rspR2Csd.csd[1] & SDC_CSD1_2Dot0_DEVICE_SIZE_MASK) >> SDC_CSD1_2Dot0_DEVICE_SIZE_SHFT) +
                      1 );
    else
    {
        mult = 1 << (((rspR2Csd.csd[1]&SDC_CSD1_C_SIZE_MULT_MASK) >> SDC_CSD1_C_SIZE_MULT_SHFT) + 2);
        block_num = ( ((rspR2Csd.csd[2] & SDC_CSD2_C_SIZE_MASK) << SDC_CSD2_C_SIZE_SHFT) +
                      ((rspR2Csd.csd[1] & SDC_CSD1_C_SIZE_MASK) >> SDC_CSD1_C_SIZE_SHFT) +
                      1 ) * mult;
    }
    capacity = block_num * block_len;
    if (ucSpec2Dot0)
        DEBUG_SDC("Trace: SD capacity = %d Kbytes.\n", capacity);
    else
    {
        capacity >>= 10;   /* (VCC) Convert bytes to Kbytes */
        DEBUG_SDC("Trace: SD capacity = %d Kbytes.\n", capacity);

    }

    /* select format size */
    if (capacity > 510046848) /* 4GB tolerance for 256GB */
    {
        dcfStorageSize = FS_MEDIA_SD_512GB;
    }
    else if (capacity > 252046848) /* 4GB tolerance for 256GB */
    {
        dcfStorageSize = FS_MEDIA_SD_256GB;
    }
    else if (capacity > 125023424) /* 4GB tolerance for 128GB */
    {
        dcfStorageSize = FS_MEDIA_SD_128GB;
    }
    else if (capacity > 61962087) /* 2048MB tolerance for 64GB */
    {
        dcfStorageSize = FS_MEDIA_SD_64GB;
    }
	else if (capacity > 31459280) /* 2048MB tolerance for 32GB */
    {
        dcfStorageSize = FS_MEDIA_SD_32GB;
    }
	else if (capacity > 14680064) /* 2048MB tolerance for 16GB */
    {
        dcfStorageSize = FS_MEDIA_SD_16GB;
    }	
    else if (capacity > 7562500) /* 256MB tolerance for 8GB */
    {
        dcfStorageSize = FS_MEDIA_SD_8GB;
    }
    else if (capacity > 3656250) /* 256MB tolerance for 4GB */
    {
        dcfStorageSize = FS_MEDIA_SD_4GB;
    }
    else if (capacity > 1703125) /* 256MB tolerance for 2GB */
    {
        dcfStorageSize = FS_MEDIA_SD_2GB;
    }
    else if (capacity > 726562) /* 256MB tolerance for 1GB */
    {
        dcfStorageSize = FS_MEDIA_SD_1GB;
    }
    else if (capacity > 375000) /* 128MB tolerance for 512MB */
    {
        dcfStorageSize = FS_MEDIA_SD_512MB;
    }
    else if (capacity > 187500) /* 64MB tolerance for 256MB */
    {
        dcfStorageSize = FS_MEDIA_SD_256MB;
    }
    else if (capacity > 93750) /* 32MB tolerance for 128MB */
    {
        dcfStorageSize = FS_MEDIA_SD_128MB;
    }
    else if (capacity > 46875) /* 16MB tolerance for 64MB */
    {
        dcfStorageSize = FS_MEDIA_SD_64MB;
    }
    else if (capacity > 23437) /* 8MB tolerance for 32MB */
    {
        dcfStorageSize = FS_MEDIA_SD_32MB;
    }
    else if (capacity > 11718) /* 4MB tolerance for 16MB */
    {
        dcfStorageSize = FS_MEDIA_SD_16MB;
    }
    else
    {
        dcfStorageSize = 0;
    }

    /* save number of total block */
    if (ucSpec2Dot0)
        sdcTotalBlockCount = block_num * 1024;
    else
    {
        if (block_len == 0x200)
            sdcTotalBlockCount = block_num ;
        else
            sdcTotalBlockCount = block_num *(block_len/0x200);
    }

#if 0
    /* check block length */
    if (block_len != SDC_BLK_LEN)
        return 0;
#endif

    return SD_OK;
}
#endif

/*

Routine Description:

    Get CSD of SD.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcGetCsd(void)
{
    SDC_R2_CSD rspR2Csd;
    u32 capacity, block_num, mult, block_len;

    /* CMD9 */
    sdcSetCommand(SDC_CMD_SEND_CSD, sdcRca, SDC_CTRL0_CMD_RSP_R2);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR2Csd(&rspR2Csd);

    /* calculate volume capacity */
    block_len = 1 << ((rspR2Csd.csd[2] & SDC_CSD2_READ_BL_LEN_MASK) >> SDC_CSD2_READ_BL_LEN_SHFT);
    mult = 1 << (((rspR2Csd.csd[1]&SDC_CSD1_C_SIZE_MULT_MASK) >> SDC_CSD1_C_SIZE_MULT_SHFT) + 2);
    block_num = ( ((rspR2Csd.csd[2] & SDC_CSD2_C_SIZE_MASK) << SDC_CSD2_C_SIZE_SHFT) +
                  ((rspR2Csd.csd[1] & SDC_CSD1_C_SIZE_MASK) >> SDC_CSD1_C_SIZE_SHFT) +
                  1 ) * mult;
    capacity = block_num * block_len;
    DEBUG_SDC("Trace: SD capacity = %d bytes.\n", capacity);
    /*CY 0718*/
    /* select format size */
    if (capacity > (2000000000 - 256000000)) /* 256MB tolerance for 2GB */
    {
        dcfStorageSize = FS_MEDIA_SD_2GB;
    }
    else if (capacity > (1000000000 - 256000000)) /* 256MB tolerance for 1GB */
    {
        dcfStorageSize = FS_MEDIA_SD_1GB;
    }
    else if (capacity > (512000000 - 128000000)) /* 128MB tolerance for 512MB */
    {
        dcfStorageSize = FS_MEDIA_SD_512MB;
    }
    else if (capacity > (256000000 - 64000000)) /* 64MB tolerance for 256MB */
    {
        dcfStorageSize = FS_MEDIA_SD_256MB;
    }
    else if (capacity > (128000000 - 32000000)) /* 32MB tolerance for 128MB */
    {
        dcfStorageSize = FS_MEDIA_SD_128MB;
    }
    else if (capacity > (64000000 - 16000000)) /* 16MB tolerance for 64MB */
    {
        dcfStorageSize = FS_MEDIA_SD_64MB;
    }
    else if (capacity > (32000000 - 8000000)) /* 8MB tolerance for 32MB */
    {
        dcfStorageSize = FS_MEDIA_SD_32MB;
    }
    else if (capacity > (16000000 - 4000000)) /* 4MB tolerance for 16MB */
    {
        dcfStorageSize = FS_MEDIA_SD_16MB;
    }
    else
    {
        dcfStorageSize = 0;
    }

    /* save number of total block */
#if 1
    if (block_len == 1024)
        sdcTotalBlockCount = block_num << 1;
    else
#endif
        sdcTotalBlockCount = block_num;

    /* check block length */
    if (block_len != SDC_BLK_LEN)
        return errHandle(SD_ERROR_WRONG_BLOCK_LENGTH, "Blcok length isn't 512 Bytes");

    return SD_OK;
}

/*

Routine Description:

	Get SCR of SD.

Arguments:

	scr - SCR register content.

Return Value:

	0 - Failure.
	1 - Success.

*/
/*CY 0907 */
s32 sdcGetScr(u8* scr)
{
    SDC_R1 rspR1;
    u32 dmaTrig = SdcDmaTrig;

    /* set dma trigger level to 1 word */
    SdcDmaTrig = 0;

    /* CMD55 */
    sdcSetCommand(SDC_CMD_APP_CMD, sdcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return 2;	/* it is an MMC */
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* ACMD51 */
    sdcSetCommand(SDC_ACMD_SEND_SCR, 0, SDC_CTRL0_CMD_RSP_R1_SCR_RD);
    /* set read data dma */
    sdcSetReadStatusDma(scr, SDC_SCR_LEN);

    /* check command/response completion */
    //if (sdcCheckCommandResponseComplete() != 1) /* cytsai: the controller does not signal command/response completion */
    //	return 0;
    /* check dma completion */
    if (sdcCheckDmaReadComplete() != 1)
    {
        SdcDmaTrig = dmaTrig;
        return 0;
    }
    SdcDmaTrig = dmaTrig;

    /* get response */
    sdcGetResponseR1(&rspR1);
    /* check read error */
    if (sdcCheckReadError() != 1)
        return 0;

    /*CY 0907*/
    {
        s32 i;

        DEBUG_SDC("Trace: SD SCR = 0x");
        for (i = 0; i < SDC_SCR_LEN; i++)
            DEBUG_SDC("%02x", scr[i]);
        DEBUG_SDC("\n");
    }


    return 1;
}


char Write_protet(void)
{
#if SDC_WRPROT_ENA    
    if (SdcStat & SDC_WRITE_PROTECT)
        return 1;
    else
#endif
        return 0;
}
/*

Routine Description:

    Switch function.

Arguments:

    param - Parameter.
    swStat - Switch function status.

Return Value:

    0 - Failure.
    1 - Success.

*/
/*CY 0907*/
s32 sdcSwitchFunction(u32 param, u8* swStat)
{
    SDC_R1 rspR1;

    /* CMD6 */
    sdcSetCommand(SDC_CMD_SWITCH_FUNC, param, SDC_CTRL0_CMD_RSP_R1_SWITCH_RD);
    /* set read data dma */
    if(sdcSetReadDataDma(swStat, SDC_SWITCH_LEN) != 1)
    {
        return errHandle(SD_ERROR_CARD_DMA_REQERR, "Read DMA request fails");
    }

    /* check command/response completion */
    /* check dma completion */
    if (sdcCheckDmaReadComplete() != 1)
        return errHandle(SD_ERROR_DMA_FAILS, "DMA operation fails");


    /* get response */
    sdcGetResponseR1(&rspR1);
    /* check read error */
    if (sdcCheckReadError() != 1)
        return errHandle(SD_ERROR_DATA_RX_ERROR, "Data RX CRC Error (sdcDatRxCrcErr) or DMA Data RX Error (sdcDmaDatRxErr)");

    /* Check supported functions */
    if (!(swStat[(512 - 400) / 8 - 1] & 0x2))
    {
        DEBUG_SDC("This card doesn't support High-Speed Mode!\n");
        return errHandle(SD_ERROR_CARD_DONT_SUPPORT_HIGH_SPEED, "This card doesn't support High-Speed Mode");
    }
    if ((swStat[(512 - 384) / 8 - 1] == 0xF) || (swStat[(512 - 392) / 8 - 1] == 0xF))
    {
        DEBUG_SDC("Check function with error!\n");
        return errHandle(SD_ERROR_CHECK_FUNCTION_ERROR, "Check function with error");
    }


    return SD_OK;
}
/*

Routine Description:

    Data transfer mode of SD.

Arguments:

    busWidth - The width of data bus.

Return Value:

    0 - Failure.
    1 - Success.

*/


s32 sdcDataTransferMode(u32 busWidth)
{
    SDC_R1 rspR1;
    u32 setCd = 0x00000001;  /* default 1 bit */
    u32 busWid = 0x00000000; /* default 1 bit */
    u32 clkDiv;			/*CY 0907 */
 #if SUPPORT_SD_HIGHSPEED_MODE
    u8 scr[SDC_SCR_LEN];		/*CY 0907*/
 #endif
    u32 i;
 
    u8 swStat[SDC_SWITCH_LEN];

    /* CMD13 */
    sdcSetCommand(SDC_CMD_SEND_STATUS, sdcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* change from standby state to transfer state */
    if ((rspR1.card_status & SDC_CS_CURRENT_STATE) == SDC_CS_STAT_STBY)
    {
        /* CMD7 */
        sdcSetCommand(SDC_CMD_SELECT_DESELECT_CARD, sdcRca, SDC_CTRL0_CMD_RSP_R1);
        /* check command/response completion */
        if (sdcCheckCommandResponseComplete() != 1)
            return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
        /* get response */
        sdcGetResponseR1(&rspR1);
    }

    /* CMD13 */
    sdcSetCommand(SDC_CMD_SEND_STATUS, sdcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* check if current state is transfer state */
    //if ((rspR1.card_status&SDC_CS_CURRENT_STATE) != SDC_CS_STAT_TRAN)
    //    return errHandle(SD_ERROR_NOT_TRASFER_STATE, "Current state is not transfer state");
    sdcCheckTransferState();
    
    /* CMD55 */
    sdcSetCommand(SDC_CMD_APP_CMD, sdcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command/ersponse completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* ACMD42 */
    if (busWidth == SDC_BUS_4BIT) /* check bus width */
        setCd = 0x00000000;       /* disconnect 50 KOhm pull-up resistor on CD/DAT3 for 4-bit mode */

    sdcSetCommand(SDC_ACMD_SET_CLR_CARD_DETECT, setCd, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* CMD55 */
    sdcSetCommand(SDC_CMD_APP_CMD, sdcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* ACMD6 */
    if (busWidth == SDC_BUS_4BIT) /* check bus width */
        busWid = 0x00000002;

    sdcSetCommand(SDC_ACMD_SET_BUS_WIDTH, busWid, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* SDC configuration */
    /* get SCR */
    #if SUPPORT_SD_HIGHSPEED_MODE  
    sdcGetScr(scr);
    if (scr[0] != 0x00) /* 0x0 - SCR structure version No. 1.0, 0x0 - Physical layer specification version number version 1.0 - 1.01 */
    {
        clkDiv = 0 ;
        /* Switch set function */
        DEBUG_SDC("# Switch Command\n");
        if (sdcSwitchFunction(0x80ffff01, swStat))
        {
            DEBUG_SDC("# Change clock\n");
            /* SDC configuration */
            /* Set clock divisor to make SD clock less than 50MHz of high-speed mode. */
            /* System clock won't be greater than 100MHz. */
        #if (SYS_CPU_CLK_FREQ == 24000000)
            clkDiv = 0;  // 24MHz
        #elif(SYS_CPU_CLK_FREQ == 32000000)
            clkDiv = 0;  // 32MHz
        #elif(SYS_CPU_CLK_FREQ == 48000000)
            clkDiv = 0;  // 48MHz
        #elif(SYS_CPU_CLK_FREQ == 72000000)
            clkDiv = 1;  // 36MHz
        #elif(SYS_CPU_CLK_FREQ == 96000000)
            clkDiv = 1;  // 48MHz
        #elif(SYS_CPU_CLK_FREQ == 108000000)
            clkDiv = 2;  // 36MHz    
        #elif(SYS_CPU_CLK_FREQ == 192000000)
            clkDiv = 3;  // 48MHz
        #elif(SYS_CPU_CLK_FREQ == 160000000)
            clkDiv = 3;  // 40MHz
        #else    
            clkDiv = 0;
        #endif        
            SdcCfg = clkDiv | SDC_ENA | SDC_BUS_4BIT | SDC_SD;
            if(sdcTryInvertSDClk)
               SdcCfg ^= SDC_CLK_INV;
            DEBUG_SDC("Trace: High-speed mode %dHz,sdcTryInvertSDClk=%d\n", SYS_CPU_CLK_FREQ/(clkDiv+1),sdcTryInvertSDClk);
        }
    }
    else
    {
        #if (SYS_CPU_CLK_FREQ == 24000000)
            clkDiv = 1;  // 12MHz
        #elif(SYS_CPU_CLK_FREQ == 32000000)
            clkDiv = 1;  // 16MHz
        #elif(SYS_CPU_CLK_FREQ == 48000000)
            clkDiv = 1;  // 24MHz
        #elif(SYS_CPU_CLK_FREQ == 72000000)
            clkDiv = 2;  // 24MHz
        #elif(SYS_CPU_CLK_FREQ == 96000000)
            clkDiv = 3;  // 24MHz        
        #elif(SYS_CPU_CLK_FREQ == 108000000)
            clkDiv = 4;  // 21.6 MHz             
        #elif(SYS_CPU_CLK_FREQ == 192000000)
            clkDiv = 7;  // 24MHz
        #elif(SYS_CPU_CLK_FREQ == 160000000)
            clkDiv = 7;  // 20MHz
        #else    
            clkDiv = 1;
        #endif  
        SdcCfg = clkDiv | SDC_ENA | SDC_BUS_4BIT | SDC_SD;
        SdcCfg |= SDC_CLK_INV;
        if(sdcTryInvertSDClk)
           SdcCfg ^= SDC_CLK_INV;
        DEBUG_SDC("Trace: Default mode %dHz,sdcTryInvertSDClk=%d\n", SYS_CPU_CLK_FREQ/(clkDiv+1),sdcTryInvertSDClk );
    }
    #else
        #if (SYS_CPU_CLK_FREQ == 24000000)
            clkDiv = 1;  // 12MHz
        #elif(SYS_CPU_CLK_FREQ == 32000000)
            clkDiv = 1;  // 16MHz
        #elif(SYS_CPU_CLK_FREQ == 48000000)
            clkDiv = 1;  // 24MHz
        #elif(SYS_CPU_CLK_FREQ == 72000000)
            clkDiv = 2;  // 24MHz
        #elif(SYS_CPU_CLK_FREQ == 96000000)
            clkDiv = 3;  // 24MHz              
        #elif(SYS_CPU_CLK_FREQ == 108000000)
            clkDiv = 4;  // 21.6MHz                  
        #elif(SYS_CPU_CLK_FREQ == 192000000)
            clkDiv = 15;  // 16MHz
        #elif(SYS_CPU_CLK_FREQ == 160000000)
            clkDiv = 7;  // 20MHz
        #else    
            clkDiv = 1;
        #endif   
        SdcCfg = clkDiv | SDC_ENA | SDC_BUS_4BIT | SDC_SD;
        SdcCfg |= SDC_CLK_INV;
        if(sdcTryInvertSDClk)
           SdcCfg ^= SDC_CLK_INV;
        DEBUG_SDC("Trace: Default mode %dHz,sdcTryInvertSDClk=%d\n", SYS_CPU_CLK_FREQ/(clkDiv+1),sdcTryInvertSDClk ); 
    #endif

    return SD_OK;
}

/*

Routine Description:

    Erase blocks of SD.

Arguments:

    startAddr - The start block address to erase.
    endAddr - The end block address to erase.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcErase(u32 startAddr, u32 endAddr)
{
    SDC_R1 rspR1;

    if(gInsertCard == 0)
        return -2;

    if (sdcDetectCard() != SD_OK)
    {
        DEBUG_SDC("No SD Card.#3\n");
        OSMboxPost(general_MboxEvt, "FAIL");
        gInsertCard=0;
        return -2;
    }
    /* check write protect */
#if SDC_WRPROT_ENA        
    if (SdcStat & SDC_WRITE_PROTECT)
        return errHandle(SD_ERROR_CARD_IS_WRITE_PROTECTED, "Card is write-protected");
#endif
    /* CMD32 */
    sdcSetCommand(SDC_CMD_ERASE_WR_BLK_START, startAddr, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* CMD33 */
    sdcSetCommand(SDC_CMD_ERASE_WR_BLK_END, endAddr, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* CMD38 */
    sdcSetCommand(SDC_CMD_ERASE, 0, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* check write error */
    /* Need more accurate evaluation */
    if (sdcCheckCardBusy_Erase(600) != 1) //60 sec
        return errHandle(SD_ERROR_CARD_IS_NOT_CHANGED_TO_READY, "SDC is not changed from busy to ready");

    return SD_OK;
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
s32 sdcReadSingleBlock(u32 addr, u8 *dataBuf)
{
    SDC_R1 rspR1;
    
    sdcCheckTransferState();
    
#if 1
    /* CMD17 */
    sdcSetCommand(SDC_CMD_READ_SINGLE_BLOCK, addr, SDC_CTRL0_CMD_RSP_R1_DAT_RD);
#else
    /* Try to get SD Card Status */
    /* CMD55 */
    sdcSetCommand(SDC_CMD_APP_CMD, sdcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command/ersponse completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* ACMD13 */
    sdcSetCommand(SDC_ACMD_SD_STATUS, 0x1, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);
#endif
    /* set read data dma */
    //sdcSetReadDataDma(dataBuf, SDC_BLK_LEN);
    if(sdcSetReadDataDma(dataBuf, SDC_BLK_LEN) != 1)
    {
        return errHandle(SD_ERROR_CARD_DMA_REQERR, "Read DMA request fails");
    }
    /* check command/response completion */
    //if (sdcCheckCommandResponseComplete() != 1) /* cytsai: the controller does not signal command/response completion */
    //  return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");

    /* check dma completion */
    if (sdcCheckDmaReadComplete() != 1)
        return errHandle(SD_ERROR_DMA_FAILS, "DMA operation fails");

    /* get response */
    sdcGetResponseR1(&rspR1);
    /* check read error */
    if (sdcCheckReadError() != 1)
        return errHandle(SD_ERROR_DATA_RX_ERROR, "Data RX CRC Error (sdcDatRxCrcErr) or DMA Data RX Error (sdcDmaDatRxErr)");

    return SD_OK;
}

/*

Routine Description:
    (VCC) Have Problem ???
    Read multiple blocks.

Arguments:

    count - The number of blocks.
    addr - The start block address of the multiple blocks.
    dataBuf - The buffer to read.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcReadMultipleBlock(u32 count, u32 addr, u8 *dataBuf)
{
    SDC_R1 rspR1;
    u32 i;

    sdcCheckTransferState();
    
    /* CMD18 */
    sdcSetCommand(SDC_CMD_READ_MULTIPLE_BLOCK, addr, SDC_CTRL0_CMD_RSP_R1_DAT_RD);
#if SD_MUL_READ_BURST
    /* set read data dma */
    //sdcSetReadDataDma(dataBuf, SDC_BLK_LEN * count);
    if(sdcSetReadDataDma(dataBuf, SDC_BLK_LEN * count) != 1)
    {
        return errHandle(SD_ERROR_CARD_DMA_REQERR, "Read DMA request fails");
    }
    /* check dma complete */
    if (sdcCheckDmaReadComplete() != 1)
        return errHandle(SD_ERROR_DMA_FAILS, "DMA operation fails");

    /* check read error */
    if (sdcCheckReadError() != 1)
        return errHandle(SD_ERROR_DATA_RX_ERROR, "Data RX CRC Error (sdcDatRxCrcErr) or DMA Data RX Error (sdcDmaDatRxErr)");
    
#else
    for (i = 0; i < count; i++)
    {
        /* set read data dma */
        //sdcSetReadDataDma(dataBuf, SDC_BLK_LEN);
        if(sdcSetReadDataDma(dataBuf, SDC_BLK_LEN) != 1)
        {
            return errHandle(SD_ERROR_CARD_DMA_REQERR, "Read DMA request fails");
        }
        /* check command/response completion */
        //if (sdcCheckCommandResponseComplete() != 1) /* cytsai: the controller does not signal command/response completion */
        //  return 0;

        /* check dma complete */
        if (sdcCheckDmaReadComplete() != 1)
            return errHandle(SD_ERROR_DMA_FAILS, "DMA operation fails");

        /* get response */
        // sdcGetResponseR1(&rspR1);
        /* check read error */
        if (sdcCheckReadError() != 1)
            return errHandle(SD_ERROR_DATA_RX_ERROR, "Data RX CRC Error (sdcDatRxCrcErr) or DMA Data RX Error (sdcDmaDatRxErr)");

        /* advance pointer of data buffer */
        dataBuf += SDC_BLK_LEN;
    }
#endif

    /* CMD12 */
    sdcSetCommand(SDC_CMD_STOP_TRANSMISSION, 0, SDC_CTRL0_CMD_RSP_R1);
    /* check command response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);
    /* check read error */
    if (sdcCheckReadError() != 1)
        return errHandle(SD_ERROR_DATA_RX_ERROR, "Data RX CRC Error (sdcDatRxCrcErr) or DMA Data RX Error (sdcDmaDatRxErr)");


    return SD_OK;
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
s32 sdcWriteSingleBlock(u32 addr, u8 *dataBuf)
{
    SDC_R1 rspR1;

#if 0
    /* Clear "Data Write to SD card interrupt" */
    sdcWriteToSD = 0;
#endif

    /* check write protect */
#if SDC_WRPROT_ENA    
    if (SdcStat & SDC_WRITE_PROTECT)
        return errHandle(SD_ERROR_CARD_IS_WRITE_PROTECTED, "Card is write-protected");
#endif

#if 1
    /* Enable interrupts */
    SdcIntMask = SDC_CMD_RSP_CMPL_INT_ENA |
                 SDC_DAT_RX_ERR_INT_ENA |
                 SDC_RSP_RX_ERR_INT_ENA |
                 SDC_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_RX_ERR_INT_ENA |
                 SDC_CARD_DETECT_INTR_ENA |
                 SDC_DAT_WRITE_TO_SD_CARD_INT_ENA;
#endif

    

    sdcCheckTransferState();
    
    /* (VCC) Redundant??? */
    /* CMD24 */
    sdcSetCommand(SDC_CMD_WRITE_BLOCK, addr, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* set write data dma */ /* cytsai: 0315 */
    if (sdcSetWriteDataDma(dataBuf, SDC_BLK_LEN) != 1)
        return errHandle(SD_ERROR_DMA_CONTROLLER_WRITE_ERROR, "DMA controller fails to set to write");

#if SDWRI_DEBUG_ENA
    gpioSetLevel(1, 9, 1);
#endif
    /* write data block cycle */
    sdcSetCommand(SDC_CMD_WRITE_BLOCK, addr, SDC_CTRL0_CMD_RSP_R1_DAT_WR);

    /* check dma complete */
    if (sdcCheckDmaWriteComplete() != 1)
        return errHandle(SD_ERROR_DMA_FAILS, "DMA operation fails");

    /* check write error */
    if (sdcCheckWriteError() != 1)
        return errHandle(SD_ERROR_FAIL_TO_WRITE_DATA_TO_CARD, "Fail to write data to card");

#if SDWRI_DEBUG_ENA
    gpioSetLevel(1, 9, 0);
#endif

    return SD_OK;
}

#if !SW_MUL_WRITE
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
s32 sdcWriteMultipleBlock(u32 count, u32 addr, u8 *dataBuf)
{
    SDC_R1 rspR1;
    u32 i;
    u32 j;

    WriteMultipleBlock=1;
    /* check write protect */
#if SDC_WRPROT_ENA        
    if (SdcStat & SDC_WRITE_PROTECT)
        return errHandle(SD_ERROR_CARD_IS_WRITE_PROTECTED, "Card is write-protected");
#endif

    /* Enable interrupts */
#if 1
    SdcIntMask = SDC_CMD_RSP_CMPL_INT_ENA |
                 SDC_DAT_RX_ERR_INT_ENA |
                 SDC_RSP_RX_ERR_INT_ENA |
                 SDC_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_RX_ERR_INT_ENA |
                 SDC_CARD_DETECT_INTR_ENA |
                 SDC_DAT_MUL_WRITE_TO_SD_CARD_INT_ENA;
#endif

    /* Number of block to be written by a H/W multiple write operation */

    SdcMulBlkNum = count - 1;

    sdcCheckTransferState();
    
    /* CMD25 */
    sdcSetCommand(SDC_CMD_WRITE_MULTIPLE_BLOCK, addr, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");


    /* get response */
    sdcGetResponseR1(&rspR1);

    /* write data block cycle */
    sdcSetCommand(SDC_CMD_WRITE_MULTIPLE_BLOCK, addr, SDC_CTRL0_CMD_RSP_R1_DAT_WR);

    /* casino test s */
    if (sdcSetWriteDataDma(dataBuf, SDC_BLK_LEN * count) != 1)
        /* casino test e */
        return errHandle(SD_ERROR_DMA_CONTROLLER_WRITE_ERROR, "DMA controller fails to set to write");

    /* check dma complete */
#if SDWRI_DEBUG_ENA
    gpioSetLevel(1, 9, 1);
#endif


    if (sdcCheckDmaWriteComplete() != 1)
        return errHandle(SD_ERROR_DMA_FAILS, "DMA operation fails");

    /* check write error */
    if (sdcCheckMultipleWriteError(count) != 1)
        return errHandle(SD_ERROR_FAIL_TO_MUL_WRITE_DATA_TO_CARD, "Fail to multiple write data to card");

#if SDWRI_DEBUG_ENA
    gpioSetLevel(1, 9, 0);
#endif

    /* CMD12 */
    sdcSetCommand(SDC_CMD_STOP_TRANSMISSION, 0, SDC_CTRL0_CMD_RSP_R1);
    /* check command response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    /* check write error */
    if (sdcCheckCardBusy(0x000ffff) != 1)
    {
        sdcSetCommand(SDC_CMD_SEND_STATUS, sdcRca, SDC_CTRL0_CMD_RSP_R1);
        /* check command/response completion */
        if (sdcCheckCommandResponseComplete() != 1)
            return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
        /* get response */
        sdcGetResponseR1(&rspR1);
        DEBUG_SDC("SDC Status=0x%x\n",rspR1.card_status);
        
        return errHandle(SD_ERROR_CARD_IS_NOT_CHANGED_TO_READY, "SDC is not changed from busy to ready");
    }
    WriteMultipleBlock=0;

#if SDC_ECC_DETECT
    /*CMD 13*/
    sdcSetCommand(SDC_CMD_SEND_STATUS, sdcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    sdcGetResponseR1(&rspR1);
    
    //check bit-21 iff ECC error
    if(rspR1.card_status & 0x00200000)
        return errHandle(SD_ERROR_CARD_ECC_ERROR, "SDC ECC error");
#endif

    return SD_OK;
}

#else
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
s32 sdcWriteMultipleBlock(u32 count, u32 addr, u8 *dataBuf)
{
    SDC_R1 rspR1;
    u32 i;
    u32 j;

    //u32 status;

    /* check write protect */
#if SDC_WRPROT_ENA        
    if (SdcStat & SDC_WRITE_PROTECT)
        return errHandle(SD_ERROR_CARD_IS_WRITE_PROTECTED, "Card is write-protected");
#endif
    /* Enable interrupts */
#if 1
    SdcIntMask = SDC_CMD_RSP_CMPL_INT_ENA |
                 SDC_DAT_RX_ERR_INT_ENA |
                 SDC_RSP_RX_ERR_INT_ENA |
                 SDC_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_RX_ERR_INT_ENA |
                 SDC_CARD_DETECT_INTR_ENA |
                 SDC_DAT_WRITE_TO_SD_CARD_INT_ENA;
#endif
    
    sdcCheckTransferState();
    
    /* CMD25 */
    sdcSetCommand(SDC_CMD_WRITE_MULTIPLE_BLOCK, addr, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);

    for (i = 0; i < count; i++)
    {
        /* write data block cycle */
        sdcSetCommand(SDC_CMD_WRITE_MULTIPLE_BLOCK, addr, SDC_CTRL0_CMD_RSP_R1_DAT_WR);

        /* set write data dma */ /* cytsai: 0315 */
        if (sdcSetWriteDataDma(dataBuf, SDC_BLK_LEN) != 1)
            return errHandle(SD_ERROR_DMA_CONTROLLER_WRITE_ERROR, "DMA controller fails to set to write");

        /* check dma complete */
        if (sdcCheckDmaWriteComplete() != 1)
            return errHandle(SD_ERROR_DMA_FAILS, "DMA operation fails");

        /* check write error */
        if (sdcCheckWriteError() != 1)
            return errHandle(SD_ERROR_FAIL_TO_WRITE_DATA_TO_CARD, "Fail to write data to card");

        /* advance pointer of data buffer */
        dataBuf += SDC_BLK_LEN;
    }

    /* CMD12 */
    sdcSetCommand(SDC_CMD_STOP_TRANSMISSION, 0, SDC_CTRL0_CMD_RSP_R1);
    /* check command response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);
    /* check write error */
    if (sdcCheckCardBusy(0x0000ffff) != 1)
        return errHandle(SD_ERROR_CARD_IS_NOT_CHANGED_TO_READY, "SDC is not changed from busy to ready");

    return SD_OK;
}
#endif

/*
 *********************************************************************************************************
 *                                                Application Function
 *********************************************************************************************************
 */

/*

Routine Description:

    Initialize SD/MMC.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcInit(void)
{
	u8	err;
	
    /* Clock enable of SD controller */
    sysSDCRst();
    SYS_CTL0 |= SYS_CTL0_SD_CK_EN;

    /* (VCC) bit 0 or bit 1 ??? */
    /* Flash io pin seleted */

    GpioActFlashSelect = (GpioActFlashSelect | GPIO_ACT_FLASH_SD) & (~GPIO_ACT_FLASH_SPI) ;

    /* Enable interrupts */
    SdcIntMask = SDC_CMD_RSP_CMPL_INT_ENA |
                 SDC_DAT_RX_ERR_INT_ENA |
                 SDC_RSP_RX_ERR_INT_ENA |
                 SDC_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_RX_ERR_INT_ENA |
                 SDC_CARD_DETECT_INTR_ENA |
                 //SDC_DAT_MUL_WRITE_TO_SD_CARD_INT_ENA |
                 SDC_DAT_WRITE_TO_SD_CARD_INT_ENA;

    /* Set read/write dma trigger level */
#if REAL_CHIP
    SdcDmaTrig = SDC_DMA_TRIG_04W4R; /* DMA trigger after 4 words in fifo */
#else
    SdcDmaTrig = SDC_DMA_TRIG_04W4R; /* DMA trigger after 4 words in fifo */
#endif
    if (ucSDCInit) //僅第一次才 create semephore
    {
        /* Create the semaphore */
        sdcSemEvt = OSSemCreate(0);
#if USE_TX_INT
        sdcSemTx_INT = OSSemCreate(0);
#endif
        sdcSemMulTx_INT = OSSemCreate(0);

    }
    else
    {
        if(ucSDCUnInit == 0)
        {
            sdcSemEvt = OSSemDel(sdcSemEvt, OS_DEL_ALWAYS, &err);
            if (err != OS_NO_ERR)
            {
                DEBUG_SDC("SDC Error: Del sdcSemEvt Failed\n");
            }

        #if USE_TX_INT
            sdcSemTx_INT = OSSemDel(sdcSemTx_INT, OS_DEL_ALWAYS, &err);
            if (err != OS_NO_ERR)
            {
                DEBUG_SDC("SDC Error: Del sdcSemTx_INT Failed\n");
            }
        #endif

            sdcSemMulTx_INT = OSSemDel(sdcSemMulTx_INT, OS_DEL_ALWAYS, &err);
            if (err != OS_NO_ERR)
            {
                DEBUG_SDC("SDC Error: Del sdcSemMulTx_INT Failed\n");
            }

        }
        ucSDCUnInit=0;

        /* Create the semaphore */
        sdcSemEvt = OSSemCreate(0);
    #if USE_TX_INT
        sdcSemTx_INT = OSSemCreate(0);
    #endif
        sdcSemMulTx_INT = OSSemCreate(0);

    }
    return SD_OK;
}


/*

Routine Description:

    Un-Initialize SD/MMC.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcUnInit(void)
{
	INT8U	err;
	
    /* Clock enable of SD controller */
    sysSDCRst();
    SdcIntMask =  SDC_CARD_DETECT_INTR_ENA;             

    sdcSemEvt = OSSemDel(sdcSemEvt, OS_DEL_ALWAYS, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SDC("SDC Error: Del sdcSemEvt Failed\n");
    }

#if USE_TX_INT
    sdcSemTx_INT = OSSemDel(sdcSemTx_INT, OS_DEL_ALWAYS, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SDC("SDC Error: Del sdcSemTx_INT Failed\n");
    }
#endif

    sdcSemMulTx_INT = OSSemDel(sdcSemMulTx_INT, OS_DEL_ALWAYS, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SDC("SDC Error: Del sdcSemMulTx_INT Failed\n");
    }

    ucSDCUnInit = 1;
    return SD_OK;
}

/*

Routine Description:

    Mount SD/MMC.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - SD.
    2 - MMC.

*/
s32 sdcMount(void)
{
    int cardType;

    //if(gInsertCard == 0)
        //return -2;

    if (sdcDetectCard() != SD_OK)
    {
        DEBUG_SDC("No SD Card.#2\n");
        gInsertCard=0;
        return -2;
    }


#if (NIC_SUPPORT==1)
	Fileplaying = 0; //20160930 Sean
#endif

#if SDC_POWER_ON_RESET_ENA
    sysSDCRst();

    /* Enable interrupts */
    SdcIntMask = SDC_CMD_RSP_CMPL_INT_ENA |
                 SDC_DAT_RX_ERR_INT_ENA |
                 SDC_RSP_RX_ERR_INT_ENA |
                 SDC_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_RX_ERR_INT_ENA |
                 SDC_CARD_DETECT_INTR_ENA |
                 //SDC_DAT_MUL_WRITE_TO_SD_CARD_INT_ENA |
                 SDC_DAT_WRITE_TO_SD_CARD_INT_ENA;

    /* Set read/write dma trigger level */
#if REAL_CHIP
    SdcDmaTrig = SDC_DMA_TRIG_04W4R; /* DMA trigger after 4 words in fifo */
#else
    SdcDmaTrig = SDC_DMA_TRIG_04W4R; /* DMA trigger after 4 words in fifo */
#endif


    sdcPowerOnRest();
#endif
    /* SD/MMC identification */
#if SD_SPEC_2DOT0
    cardType = sdcCardIdentificationMode2Dot0();
#else
    cardType = sdcCardIdentificationMode();
#endif

    if (cardType == 1)
    {
        /* SD */
        if ((sdcDataTransferMode(SDC_BUS_4BIT)) == 1)
        {
            sdcMmc = 1;
            gInsertCard=1;
            return SD_OK;
        }
    }    
    else if(cardType == SD_ERROR_NO_CARD_IS_INSERTED)
    {
        sdcMmc = 1;
        gInsertCard=0;
        DEBUG_SDC("No SD Card.#4\n");
        return -2;
    }
    else
    {
        /* Error */
        sdcMmc = 1;
        DEBUG_SDC("SD indentify Error.\n");
        return -1;
    }

    return 0;
}

/*

Routine Description:

    Unmount SD/MMC.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcUnmount(void)
{
    return SD_OK;
}

/*

Routine Description:

    The IRQ handler of SD/MMC.

Arguments:

    None.

Return Value:

    None.

*/
void sdcIntHandler(void)
{
    u32 intStat = SdcIntStat;

    /* clear the interrupt flags */
    sdcCmdRspCmpl = sdcDatRxCrcErr = sdcRspRxCrcErr =
                                         sdcDatTxErr = sdcDmaDatTxErr = sdcDmaDatRxErr = 0;
#if 1
    sdcWriteToSD = 0;
#endif

    if (intStat & SDC_CMD_RSP_CMPL)
    {
        /* Command and response cycle completed */
        sdcCmdRspCmpl = 1;

        /* Signal completion */
        OSSemPost(sdcSemEvt);
    }

    if (intStat & SDC_DAT_RX_CRC_ERR)
    {
        /* Data RX CRC error */
        sdcDatRxCrcErr = 1;
        DEBUG_SDC("sdcDatRxCrcErr\n");
    }

    if (intStat & SDC_RSP_RX_CRC_ERR)
    {
        /* Response RX CTC error */
        sdcRspRxCrcErr = 1;
        DEBUG_SDC("sdcRspRxCrcErr\n");
    }

    if (intStat & SDC_DAT_TX_ERR)
    {
        /* Data TX error */
        sdcDatTxErr = 1;
        DEBUG_SDC("sdcDatTxErr\n");
    }

    if (intStat & SDC_DMA_DAT_TX_ERR)
    {
        /* DMA data TX error */
        sdcDmaDatTxErr = 1;
        DEBUG_SDC("sdcDmaDatTxErr\n");
    }

    if (intStat & SDC_DMA_DAT_RX_ERR)
    {
        /* DMA data RX error */
        sdcDmaDatRxErr = 1;
        DEBUG_SDC("ISR: sdcDmaDatRxErr\n");
    }

    if (intStat & SDC_DAT_WRITE_TO_SD_CARD)
    {
        /* (VCC 20061130) H/W doesn't use this interrupt */
        /* Data Write to SD card */
        sdcWriteToSD = 1;
        /* For debug */

#if USE_TX_INT
        /* Signal completion */
        if (SdcIntMask & SDC_DAT_WRITE_TO_SD_CARD_INT_ENA)
            OSSemPost(sdcSemTx_INT);
#endif
    }

    if (intStat & SDC_DAT_MUL_WRITE_TO_SD_CARD)
    {
        /* Signal completion */
        OSSemPost(sdcSemMulTx_INT);
    }

    if (intStat & SDC_CARD_DETECT)
    {
        if(((SdcStat & SDC_DETECT) >> 2) == SDC_CD_OFF)
        {
        	//High
        	gSystemStroageReady = 0;	// Storage is not ready to record films
        	DEBUG_SDC("SD Remove!!\n\r");
        }
        else
        {
        	//Low
            DEBUG_SDC("SD Card In!!\n\r");
        }
				
        if(uiSentKeyToUi(UI_KEY_SDCD)==0)
        {
            syscheckSDStatus = 1;
        }
            
    }

}


#if 0
/*

Routine Description:

    Get SD card status.

Arguments:

    pStatus - Card status.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 sdcGetCardStatus(u32 *pStatus)
{
    SDC_R1 rspR1;

    /* ACMD13 */
    sdcSetCommand(SDC_CMD_SEND_STATUS, sdcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != 1)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);
    *pStatus = rspR1.card_status;

    return SD_OK;
}
#endif

/*

Routine Description:

    Get card status.

Arguments:

    pStatus - Card status.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 sdcGetCardStatus(u32 *pStatus)
{
    SDC_R1 rspR1;

    /* CMD13 */
    sdcSetCommand(SDC_CMD_SEND_STATUS, sdcRca, SDC_CTRL0_CMD_RSP_R1);
    /* check command/response completion */
    if (sdcCheckCommandResponseComplete() != SD_OK)
        return errHandle(SD_ERROR_NO_COMMAND_RESPONSE_INTERRUPT, "No command/response completion");
    /* get response */
    sdcGetResponseR1(&rspR1);
    *pStatus = rspR1.card_status;

    return SD_OK;
}

#if SDC_POWER_ON_RESET_ENA
s32 sdcPowerOnRest()
{
    int i;
    u32 gpio1_en;
    u32 gpio1_dir;

    gpio1_en=Gpio1Ena;
    gpio1_dir=Gpio1Dir;

    DEBUG_SDC("======= sdcPowerOnRest ===========\n");
    //Gpio-1: 7~14,except: 9(WP),10(CD)
    Gpio1Ena |=0x00007980;
    //DEBUG_SDC("Gpio1Ena=0x%x\n",Gpio1Ena);
    Gpio1Dir &= (~0x00007980);
    //DEBUG_SDC("Gpio1Dir=0x%x\n",Gpio1Dir);
    Gpio1Level &= (~0x00007980);
    //DEBUG_SDC("Gpio1Level=0x%x\n",Gpio1Level);
#if(HW_BOARD_OPTION == MR9670_COMMAX_71UM)
    //gpioSetLevel(GPIO_POWEROFFGRP_SDC, GPIO_POWEROFFBIT_SDC, 1);
    SdPower_On();
    //MA009_P2_WriteBit(PIN_SdPower, PowerOn);
    OSTimeDly(2);
    #if(GPIO_SD_WRITE_PROTECT_ENA)
        Gpio1Ena = gpio1_en & (~0x00007f80);
    #else
        Gpio1Ena = gpio1_en & (~0x00007d80);
    #endif
    Gpio1Dir = gpio1_dir;
    //gpioSetLevel(GPIO_POWEROFFGRP_SDC, GPIO_POWEROFFBIT_SDC, 0);
    SdPower_Off();
    //MA009_P2_WriteBit(PIN_SdPower, PowerOff);
    OSTimeDly(2);
#else
 #if(GPIO_POWEROFF_ACT)
    gpioSetLevel(GPIO_POWEROFFGRP_SDC, GPIO_POWEROFFBIT_SDC, 1);
    OSTimeDly(3);
    #if(GPIO_SD_WRITE_PROTECT_ENA)
        Gpio1Ena = gpio1_en & (~0x00007f80);
    #else
        Gpio1Ena = gpio1_en & (~0x00007d80);
    #endif
    Gpio1Dir = gpio1_dir;
    gpioSetLevel(GPIO_POWEROFFGRP_SDC, GPIO_POWEROFFBIT_SDC, 0);
    OSTimeDly(1);
 #else

    gpioSetLevel(GPIO_POWEROFFGRP_SDC, GPIO_POWEROFFBIT_SDC, 0);
    OSTimeDly(3);
    #if(GPIO_SD_WRITE_PROTECT_ENA)
        Gpio1Ena = gpio1_en & (~0x00007f80);
    #else
        Gpio1Ena = gpio1_en & (~0x00007d80);
    #endif
    Gpio1Dir = gpio1_dir;
    gpioSetLevel(GPIO_POWEROFFGRP_SDC, GPIO_POWEROFFBIT_SDC, 1);
    OSTimeDly(1);

 #endif
#endif


    return 1;
}    

#endif  /* #if SDC_POWER_ON_RESET_ENA */
/*

Routine Description:

    Check if card is in transfer state.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcCheckTransferState(void)
{
    u32 status, cnt=20;
    
    do
    {
        if (sdcGetCardStatus(&status) != SD_OK)
            return errHandle(SD_ERROR_FAIL_TO_GET_CARD_STATUS, "Fail to get card status");

        if ((status & SDC_CS_CURRENT_STATE) == SDC_CS_STAT_TRAN)
            return SD_OK;
        else //if ((status&SDC_CS_CURRENT_STATE) != SDC_CS_STAT_TRAN)
        {
            DEBUG_SDC("\r\n Current state is not transfer state !! \r\n");
            //return errHandle(SD_ERROR_NOT_TRASFER_STATE, "Current state is not transfer state");
        }
        cnt--;
    }while(cnt != 0);
    
    return errHandle(SD_ERROR_NOT_TRASFER_STATE, "Not transfer state !! \r\n");
}

/*

Routine Description:

    Check if card is in transfer state.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcCheckNotProgramState(void)
{
    u32 status;

    /* wait until card leaves program state */
    do
    {
        if (sdcGetCardStatus(&status) == 0)
            return errHandle(SD_ERROR_FAIL_TO_GET_CARD_STATUS, "Fail to get card status");
    }
    while ((status&SDC_CS_CURRENT_STATE) == SDC_CS_STAT_PRG);

    return SD_OK;
}

/*

Routine Description:

    Delay.

Arguments:

    delay - Count of delay.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sdcDelay(u32 delay)
{
    while (delay != 0)
        delay--;

    return SD_OK;
}


/*

Routine Description:

    Notify to usb about media changed.

Arguments:

    ucStat - SD card status. 0->Not Ready, 1->Ready.

Return Value:

    0 - Failure.
    1 - Success.

*/
s8 sdcChangeMediaStat(u8	ucStat)
{
	u8 sdcStat = sdcCurrStat;

	if (ucStat == SDC_USB_MEDIA_REMOVED)
	{
		sdcStat &= ~(USB_MSC_LUN_MOUNT | USB_MSC_LUN_START | USB_MSC_LUN_MEDIA_CHANGED);
		sdcStat |= USB_MSC_LUN_MEDIA_REMOVED;
	}
	else if (ucStat == SDC_USB_MEDIA_CHANGED)
	{
		sdcStat &= ~USB_MSC_LUN_MEDIA_REMOVED;
		sdcStat |= (USB_MSC_LUN_MOUNT | USB_MSC_LUN_START | USB_MSC_LUN_MEDIA_CHANGED);
	}
	else if (ucStat == SDC_USB_MEDIA_READY)
	{
		sdcStat &= ~(SDC_USB_MEDIA_CHANGED | SDC_USB_MEDIA_REMOVED);
		sdcStat |= (USB_MSC_LUN_MOUNT | USB_MSC_LUN_START);
	}

	if (Write_protet() == 1)
		sdcStat |= USB_MSC_LUN_WRITE_PROTECT;
	else
		sdcStat &= ~USB_MSC_LUN_WRITE_PROTECT;

	sdcSetStat(sdcStat);
	
	return 1;
}


#if FPGA_TEST_SDC

s32 FPGA_SDC_TEST(void)
{
    u32 i ;
    s32 cardType;

    // Card Detect
    
    while ( ( (SdcStat & SDC_DETECT) >> 2) == SDC_CD_OFF )
    {
        printf("SD Card is OFF !!!\n");
    }

    sdcInit();
    
    SdcDmaTrig = SDC_DMA_TRIG_04W4R; /* DMA trigger after 4 words in fifo */

    /* Enable interrupts */
    SdcIntMask = SDC_CMD_RSP_CMPL_INT_ENA |
                 SDC_DAT_RX_ERR_INT_ENA |
                 SDC_RSP_RX_ERR_INT_ENA |
                 SDC_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_TX_ERR_INT_ENA |
                 SDC_DMA_DAT_RX_ERR_INT_ENA |
                 SDC_CARD_DETECT_INTR_ENA |
                 //SDC_DAT_MUL_WRITE_TO_SD_CARD_INT_ENA |
                 SDC_DAT_WRITE_TO_SD_CARD_INT_ENA;

    cardType = sdcCardIdentificationMode2Dot0();

    if (cardType == 1)
    {
        /* SD */
        if ((sdcDataTransferMode(SDC_BUS_4BIT)) == 1)
        {
            sdcMmc = 1;
            return SD_OK;
        }
    }
    else
        printf("Not support Card Type !!!\n");
}
#endif

