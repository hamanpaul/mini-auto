/*

Copyright (c) 2007  Himax Technologies, Inc.

Module Name:

    cf.c

Abstract:

    The routines of CF Card & ATA HD.

Environment:

        ARM RealView Developer Suite

Revision History:

    2007/10/20  VCC Creates


*/

#include "general.h"
#include "board.h"
#include "osapi.h"
#include "sysapi.h"
#include "cfapi.h"
#include "cf.h"
#include "cfreg.h"
#include "cferr.h"
//#include "dmaapi.h"
#include <../inc/mars_controller/mars_dma.h>
#include "fsapi.h" /*CY 0718*/

#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
u32 guiCFReadDMAId=0xFF, guiCFWriteDMAId=0xFF;
/*********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */

#define Layer1Function     1
#define Layer2Function     1
#define Layer3Function     1

#define CF_PROTOCOL_NUM    10


/* CF time out value */
/* timeout value = 20 * 100 ms = 2 s */ 
/* (VCC) Pay attention to the time per timer tick */
/* (VCC$) */
#if REAL_CHIP
#define CF_TIMEOUT        20 
#else
#define CF_TIMEOUT        10
#endif

//extern u32 g_intStat;
extern u32 Dma_Ch0_Cyc_Cnt;
extern u32 Dma_Ch1_Cyc_Cnt;
//extern u32 DMA_CH1_ENA;
extern u32 DMA_CH0_ENA;
/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */

/* HD header info format */
HD_INFORMATION cfSHDInformation;   
u16 cfCylindersMax;
u16 cfSectorsMax; 
u16 cfSerNoMax;
u32 cfTotalBlockCount;

/* Command, C/D/H, Cyl High, Cyl Low, Sec Num, Sec Cnt, Feature, 
       Status, 
       Error */
u8 ucProtocolEnable[CF_PROTOCOL_NUM][9] = 
    { 
        {CF_CMD_IDENTIFY_DEV, 1, 0, 0, 0, 0, 0, 
            CF_CARD_DEVICE_STATUS_ALTERNATE_RDY, 
            0}, 
        {CF_CMD_SET_FEATURE,  1, 1, 1, 1, 1, 1, 
            CF_CARD_DEVICE_STATUS_ALTERNATE_RDY, 
            CF_CARD_DEVICE_ERROR_ABRT}, 
        {CF_CMD_SET_MULTIPLE, 1, 0, 0, 0, 1, 0, 
            CF_CARD_DEVICE_STATUS_ALTERNATE_RDY,
            0,},
        {CF_CMD_READ_SECTOR, 1, 1, 1, 1, 1, 0, 
            CF_CARD_DEVICE_STATUS_ALTERNATE_ALL,
            0}, /* Should consider error condition */               
        {CF_CMD_READ_MULTIPLE, 1, 1, 1, 1, 1, 0, 
            CF_CARD_DEVICE_STATUS_ALTERNATE_ALL,
            0}, /* Should consider error condition */               
        {CF_CMD_WRITE_SECTOR, 1, 1, 1, 1, 1, 0, 
            CF_CARD_DEVICE_STATUS_ALTERNATE_ALL,
            0}, /* Should consider error condition */            
        {CF_CMD_WRITE_MULTIPLE, 1, 1, 1, 1, 1, 0, 
            CF_CARD_DEVICE_STATUS_ALTERNATE_ALL,
            0}, /* Should consider error condition */            
        {CF_CMD_ERASE_SECTORS, 1, 1, 1, 1, 1, 0, 
            CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY | 
            CF_CARD_DEVICE_STATUS_ALTERNATE_ERR,
            0},      
        {CF_CMD_READ_DMA, 1, 1, 1, 1, 1, 0, 
            CF_CARD_DEVICE_STATUS_ALTERNATE_ALL,
            0}, /* Should consider error condition */               
        {CF_CMD_WRITE_DMA, 1, 1, 1, 1, 1, 0, 
            CF_CARD_DEVICE_STATUS_ALTERNATE_ALL,
            0}, /* Should consider error condition */               
    };
/* Tables for CF True-IDE Mode Timing Control - PIO data transfer mode */
/* Refer to p283 of ATA Spec. 4 */
/* 
                                     Mode 0  Mode 1  Mode 2  Mode 3  Mode 4
    ts    Addr to DIOR-/DIOW- setup    70      50       30     30      25 
    tA    DIOR-/DIORW- 8bit           290     290      290     80      70   
    tA    DIOR-/DIORW- 16bit          165     125      100     80      70
    th    DIOR-/DIOW- to Addr hold     20      15       10     10      10
*/
/* Tables for CF True-IDE Mode Timing Control - Multiword DMA data transfer mode */
/* Refer to p283 of ATA Spec. 4 */
/* 
                                     Mode 0  Mode 1  Mode 2
    tI    DMACK to DIOR-/DIOW- setup      0       0       0                                     
    tD    DIOR-/DIORW-                  215      80      70
    tKR   DIOR- negated pulse width      50      50      25
    tKW   DIOW- negated pulse width     215      50      25
*/

#if (SYS_CPU_CLK_FREQ == 24000000)
u32 cfPIOTiming[5] = {
                       0x00020801,   /* Mode 0 for debug */
                       0x00020401,   /* Mode 1 */
                       0x00010301,   /* Mode 2 */
                       0x00010201,   /* Mode 3 */
                       0x00010201};   /* Mode 4 */
					   
u32 cfMultiDMATiming[3] = {  
                            0x00000606,   /* Mode 0 for debug */    
                            0x00000202,   /* Mode 1 */
                            0x00000201};   /* Mode 2 */
#elif (SYS_CPU_CLK_FREQ == 32000000)
u32 cfPIOTiming[5] = {
                       0x00030A01,   /* Mode 0 for debug */
                       0x00020A01,   /* Mode 1 */
                       0x00010A01,   /* Mode 2 */
                       0x00010301,   /* Mode 3 */
                       0x00010301};   /* Mode 4 */
					   
u32 cfMultiDMATiming[3] = {  
                            0x00000808,   /* Mode 0 for debug */    
                            0x00000302,   /* Mode 1 */
                            0x00000301};   /* Mode 2 */
#elif(SYS_CPU_CLK_FREQ == 48000000)
u32 cfPIOTiming[5] = {
                       0x00030A01,   /* Mode 0 for debug */
                       0x00020A01,   /* Mode 1 */
                       0x00010A01,   /* Mode 2 */
                       0x00010301,   /* Mode 3 */
                       0x00010301};   /* Mode 4 */
					   
u32 cfMultiDMATiming[3] = {  
                            0x00000808,   /* Mode 0 for debug */    
                            0x00000302,   /* Mode 1 */
                            0x00000301};   /* Mode 2 */

#elif(SYS_CPU_CLK_FREQ == 96000000)
u32 cfPIOTiming[5] = {
                       0x00030A01,   /* Mode 0 for debug */
                       0x00020A01,   /* Mode 1 */
                       0x00010A01,   /* Mode 2 */
                       0x00010301,   /* Mode 3 */
                       0x00010301};   /* Mode 4 */
					   
u32 cfMultiDMATiming[3] = {  
                            0x00000808,   /* Mode 0 for debug */    
                            0x00000302,   /* Mode 1 */
                            0x00000301};   /* Mode 2 */

#elif(SYS_CPU_CLK_FREQ == 108000000)
u32 cfPIOTiming[5] = {
                       0x00030A01,   /* Mode 0 for debug */
                       0x00020A01,   /* Mode 1 */
                       0x00010A01,   /* Mode 2 */
                       0x00010301,   /* Mode 3 */
                       0x00010301};   /* Mode 4 */
					   
u32 cfMultiDMATiming[3] = {  
                            0x00000808,   /* Mode 0 for debug */    
                            0x00000302,   /* Mode 1 */
                            0x00000301};   /* Mode 2 */
							
#elif(SYS_CPU_CLK_FREQ == 192000000)
u32 cfPIOTiming[5] = {
                       0x00030A01,   /* Mode 0 for debug */
                       0x00020A01,   /* Mode 1 */
                       0x00010A01,   /* Mode 2 */
                       0x00010301,   /* Mode 3 */
                       0x00010301};   /* Mode 4 */
					   
u32 cfMultiDMATiming[3] = {  
                            0x00000808,   /* Mode 0 for debug */    
                            0x00000302,   /* Mode 1 */
                            0x00000301};   /* Mode 2 */                            
#else
u32 cfPIOTiming[5] = {
                       0x00030A01,   /* Mode 0 for debug */
                       0x00020A01,   /* Mode 1 */
                       0x00010A01,   /* Mode 2 */
                       0x00010301,   /* Mode 3 */
                       0x00010301};   /* Mode 4 */
					   
u32 cfMultiDMATiming[3] = {  
                            0x00000808,   /* Mode 0 for debug */    
                            0x00000302,   /* Mode 1 */
                            0x00000301};   /* Mode 2 */                            
#endif

#if 0
u32 cfPIOTiming[5] = {
                       0x000F5005,   /* Mode 0 for debug */
                       0x000D1F04,   /* Mode 1 */
                       0x00081903,   /* Mode 2 */
                       0x00081403,   /* Mode 3 */
                       0x00070F03};   /* Mode 4 */
					   
u32 cfMultiDMATiming[3] = {  
                            0x00003535,   /* Mode 0 for debug */    
                            0x0000160e,   /* Mode 1 */
                            0x00001106};   /* Mode 2 */
#endif


#if VERIFYCF
__align(4) u8 cfWriteBuf[CF_BLK_LEN * CF_MAX_SECTOR]; /* (VCC) A buffer to store the test pattern */
__align(4) u8 cfReadBuf[CF_BLK_LEN * CF_MAX_SECTOR]; /* (VCC) A buffer to read data from CF card */
#endif

OS_EVENT *cfSemEvt;        /* semaphore to synchronize event processing */

OS_EVENT *cfSemDMAComplete;        /* semaphore to synchronize CF DMA complete  */
OS_EVENT *cfSemINTRQ;      /* Semaphore to synchronize INTRQ interrupt. */
u8 ucCFCurrentMode;
u8 cfDMAComplete;   /* Interrupt status for CF DMA complete */
u8 cfAccessTimeout; /* Interrupt status for CF access time out */
u8 cfDMAModeAccessCFCardErr;
u8 cfCardCD1Signal;
u8 cfCardCD2Signal;
u8 cfCardIRQRDYSignal;

extern s32 dcfStorageSize;  /*CY 0718*/



extern u32 unCFErrorCode;


#if VERIFYCF
//DMA_CFG_2_E eDmaCfg2; /* (VCC) SD Controller only supports burst mode and 4 bytes data width */
#endif


/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */
 
/* Macro Definition */
#define SWAP(x, y, z)   ((z) = (x), (x) = (y), (y) = (z))
// #define SWAP(x, y)  ((x)^=(y), (y)^=(x), (x)^=(y))

/* Layer 3 Functions in driver */

/* Driver Function */


/*
 *********************************************************************************************************
 *                                                Layer 3 Functions in driver
 *********************************************************************************************************
 */
#ifdef Layer3Function

/*

Routine Description:

    Delay.

Arguments:

    delay - Count of delay.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 cfDelay(u32 delay)
{
    while (delay != 0)
        delay--;

    return CF_OK;
}

/*

Routine Description:

    Initialize CF/ATA Host Controller.

Arguments:

    None.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfInitCFHost(void)
{

    /* CF True-IDE Mode Timing Control Register */
//    CfIdemodeTime = 0xFFFFFFFF;

#if USE_INT
    /* (VCC $) Enable interrupts */
    CfIntEn = CF_INT_CF_DMA_COMPLETE |
              CF_INT_CF_CARD_IRQRDY_SIGNAL;
#else
    /* Disable all interrupts */
    CfIntEn = 0x0;
#endif

    /* Set True IDE-Mode & Word access */
    CfSelAccmode = CF_SEL_ACCESS_DATA_WIDTH_WORD_ACCESS |
                   CF_SEL_ACCESS_ACCESS_MODE_TRUE_IDE;

    /* Disable DMA Mode Auto Check status */
//    CfAutoCheckCardStatus &= ~CF_AUTO_CHECK_CARD_STATUS_ENA;

//    CfAutoCheckCardStatus = 0x158;

    return CF_OK;
}

/*

Routine Description:

    The IRQ handler of SD/MMC.

Arguments:

    None.

Return Value:

    None.

*/
void cfIntHandler(void)
{
    u32 intStat = CfIntStatus;

    /* Clear the interrupt flags */
    cfDMAComplete = cfAccessTimeout = cfDMAModeAccessCFCardErr =
        cfCardCD1Signal = cfCardCD2Signal = cfCardIRQRDYSignal = 0;

    if (intStat & CF_INT_CF_DMA_COMPLETE)
    {
        /*  */
        cfDMAComplete = 1;

        /* Signal completion */
        OSSemPost(cfSemDMAComplete);
    }

    if (intStat & CF_INT_CF_ACCESS_TIMEOUT)
    {
        /* cfAccessTimeout */
        cfAccessTimeout = 1;
    }

    if (intStat & CF_INT_DMA_MODE_ACCESS_CF_CARD_ERR)
    {
        /* cfDMAModeAccessCFCardErr */
        cfDMAModeAccessCFCardErr = 1;
    }

    if (intStat & CF_INT_CF_CARD_CD1_SIGNAL)
    {
        /* cfCardCD1Signal */
        cfCardCD1Signal = 1;
    }

    if (intStat & CF_INT_CF_CARD_CD2_SIGNAL)
    {
        /* cfCardCD2Signal */
        cfCardCD2Signal = 1;
    }

    if (intStat & CF_INT_CF_CARD_IRQRDY_SIGNAL)
    {
        cfCardIRQRDYSignal = 1;

        /* Signal completion - INTRQ */
        OSSemPost(cfSemINTRQ);
    }

    /* Signal completion */
}

/*

Routine Description:

    Write to CF/ATA device controller registers.

Arguments:

    ucCfAddr - The CF Address.
    ucCfData - The CF Data Port.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfWriteCFATADeviceRegister(u32 ucCfAddr, u32 ucCfData)
{
    u32 ucTimeout;

    /* Set CF Address Register and CF Data Port Register */
    CfAddr = ucCfAddr;
    CfDataport = ucCfData;

    /* Write command */
    CfHostCtrlReg = CF_HOST_CONTROL_WRITE_DATA | CF_HOST_CONTROL_TRUE_IDE_MODE_IN_CPU;

    /* Check CF Host Ready bit */
    /* (VCC $) How long? */
    ucTimeout = 0x000fffff;
	DEBUG_CF("Write CfAddr %d:%x \n",ucCfAddr,ucCfData);
	
    do
    {
        ucTimeout--;

        if (ucTimeout == 0)
        {
            return cfErrHandle(CF_ERROR_DEVICE_IS_NOT_CHANGED_TO_READY, "CF/ATA Device is not changed from busy to ready");
        }
    } while (!(CfHostrdy & 0x00000001));

    return CF_OK;
}

/*

Routine Description:

    Read from CF/ATA device controller registers.

Arguments:

    ucCfAddr - The CF Address.
    ucCfData - The CF Data Port.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfReadCFATADeviceRegister(u32 ucCfAddr, u32 *ucCfData)
{
    u32 ucTimeout = 0x000fffff; /* (VCC $) How long? */

    /* Set CF Address Register and CF Data Port Register */
    CfAddr = ucCfAddr;

    /* Read command */
    CfHostCtrlReg = CF_HOST_CONTROL_READ_DATA | CF_HOST_CONTROL_TRUE_IDE_MODE_IN_CPU;

    /* Check CF Host Ready bit */
    do
    {
        ucTimeout--;

        if (ucTimeout == 0)
        {
            return cfErrHandle(CF_ERROR_DEVICE_IS_NOT_CHANGED_TO_READY, "CF/ATA Device is not changed from busy to ready");
        }
    } while (!(CfHostrdy & 0x00000001));

    *ucCfData = CfDataport;
    
    return CF_OK;
}

/*

Routine Description:

    Read from or write to CF/ATA device controller registers in DMA mode.

Arguments:

    ucReadWriteCFATA - Read/Write CF/ATA device controller registers.
    ucCfData - The CF Data Port.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfReadWriteCFATADeviceRegisterInDMAMode(u32 unReadWriteCFATA)
{
    CfHostCtrlReg = unReadWriteCFATA;
    return CF_OK;
}

/*

Routine Description:

    Check the Bit 7 (BUSY) & Bit 3 (DRQ) in the Status & Alternate Status Registers of CF/ATA device

Arguments:

    None.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfWaitBusytoReady(void)
{
    u32 unStatus;
    volatile u32 i, j;
  
    for (i=0; i<100000; i++)
    {
        cfReadCFATADeviceRegister(CF_ADDRESS_STATUS, &unStatus);
        unStatus &= (CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ); 
        // DEBUG_CF("wait busy : status = %x\n",status);
        if (unStatus == 0x00)    
        {
            /* For debug */
//            DEBUG_CF("Polling %d times\n", i);

            return CF_OK;
        }

        /* For debug */
#if 0
        if (i > 10)
        {
            cfReadCFATADeviceRegister(CF_ADDRESS_ERROR_REGISTER, &unStatus);
        }    
        if (i == 11)
            DEBUG_CF("Polling %d times\n", i);
#endif        
    }


    return cfErrHandle(CF_ERROR_DEVICE_IS_NOT_CHANGED_TO_READY, 
                       "CF/ATA Device is not changed from busy to ready");
}

/*

Routine Description:

    Wait to Read/Write Data Register

Arguments:

    None.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfWaitToReadWrite(void)
{
    u32 unStatus;
    volatile u32 unI;
  
    for (unI = 0; unI < 100000; unI++)
    {
        cfReadCFATADeviceRegister(CF_ADDRESS_STATUS, &unStatus);
        unStatus &= (CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ); 
        // DEBUG_CF("wait busy : status = %x\n",status);
        if (unStatus == CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ)     
            return CF_OK;
    }

    return cfErrHandle(CF_ERROR_DEVICE_IS_NOT_CHANGED_TO_READY_RW, 
                       "CF/ATA Device is not changed to ready to Read/Write Data Register");
}

/*

Routine Description:

    Set Sector Count and Size

Arguments:

    ucSectorCount - Transfer sector count.
    unSectorSize - The size of one sector.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfSectorCountAndSize(u8 ucSectorCount, u32 unSectorSize)
{

    switch (unSectorSize)
    {
    case 128:
             CfSectorSize = 0x0;             
             break;
    case 256:
             CfSectorSize = 0x1;             
             break;
    case 512:
             CfSectorSize = 0x2;             
             break;
    case 1024:
             CfSectorSize = 0x3;             
             break;
    case 2048:
             CfSectorSize = 0x4;             
             break;
    case 4096:
             CfSectorSize = 0x5;             
             break;
    case 8192:
             CfSectorSize = 0x6;             
             break;
    case 16384:
             CfSectorSize = 0x7;             
    }

    CfSectorCount = ucSectorCount;

    return CF_OK;
}

/*

Routine Description:

    Check Status Register

Arguments:

    unBitsCheck - Which bits should be checked.
    unBitsOn - Which bits should be 1.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfCheckStatus(u32 unBitsCheck, u32 unBitsOn)
{
    u32 unStatus;
    u32 unI;
    u32 unTimeout = 100000;

    for (unI = 0; unI < unTimeout; unI++)
    {
        cfReadCFATADeviceRegister(CF_ADDRESS_STATUS, &unStatus);
              
        if (unStatus & CF_CARD_DEVICE_STATUS_ALTERNATE_ERR)
            {
                DEBUG_CF("Error status : status = %x\n",unStatus); 
            return cfErrHandle(CF_ERROR_OCCURS, "Error occurs");
            }
        unStatus &= unBitsCheck; 

        if (unStatus == unBitsOn)    
            return CF_OK;
    }
	
	 DEBUG_CF("CF_ERROR : status = %x\n",unStatus);
    return cfErrHandle(CF_ERROR_DEVICE_IS_NOT_CHANGED_TO_READY_RW, 
                       "CF/ATA Device is not changed to ready to Read/Write Data Register");
}

#if 0
/*

Routine Description:

    Write to CF/ATA device controller registers.

Arguments:

    ucCfSectorCount - The CF Sector count.
    0 means 256 sector counts
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfSetMultiple(CF_DEVICE_REGISTER tCfDeviceRegister)
{
    /* Program Device Card/Drive/Head Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_SELECT_CARD_HEAD, tCfDeviceRegister.ucCardDriveHead);

    /* Program Device Cylinder Registers */
    cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_HIGH, tCfDeviceRegister.ucCylHigh);
    cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_LOW, tCfDeviceRegister.ucCylLow);

    /* Program Device Sector Number Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_NO, tCfDeviceRegister.ucSecNum);

    /* Program Device Sector Count Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_COUNT, tCfDeviceRegister.ucSecCnt);

    /* Program Device Sector Count Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_FEATURES, tCfDeviceRegister.ucFeature);

    /* Program Device Command Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_COMMAND, tCfDeviceRegister.ucCommand);
 
    return CF_OK;
}
#endif

/*

Routine Description:

    Write to CF/ATA device controller registers.

Arguments:

    ucCfSectorCount - The CF Sector count.
    0 means 256 sector counts
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfSetCommand(CF_DEVICE_REGISTER tCfDeviceRegister)
{
    /* Program Device Card/Drive/Head Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_SELECT_CARD_HEAD, tCfDeviceRegister.ucCardDriveHead);

    /* Program Device Cylinder Registers */
    cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_HIGH, tCfDeviceRegister.ucCylHigh);
    cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_LOW, tCfDeviceRegister.ucCylLow);

    /* Program Device Sector Number Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_NO, tCfDeviceRegister.ucSecNum);

    /* Program Device Sector Count Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_COUNT, tCfDeviceRegister.ucSecCnt);

    /* Program Device Sector Count Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_FEATURES, tCfDeviceRegister.ucFeature);

    /* Program Device Command Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_COMMAND, tCfDeviceRegister.ucCommand);
 
    return CF_OK;
}

/*

Routine Description:

    AT Disk Controller Soft Reset operation.

Arguments:

    ucDriveNum - The selected device number.
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfSWResetDevice(void)
{
    u32 unStatus;
    volatile u32 unI;

    /* Perform an AT Disk controller Soft Reset operation */
    cfWriteCFATADeviceRegister(CF_ADDRESS_DEVICE_CONTROL, 0x04);

    for (unI = 0; unI < 100000; unI++)
    {
        cfReadCFATADeviceRegister(CF_ADDRESS_DEVICE_CONTROL, &unStatus);
        unStatus &= 0x04; 
        // DEBUG_CF("wait busy : status = %x\n",status);
        if (!unStatus)     
            return CF_OK;
    }

    return cfErrHandle(CF_ERROR_DEVICE_IS_NOT_CHANGED_TO_READY_RW, 
                       "AT Disk controller Soft Reset fails");

 
//    return CF_OK;
}


#endif

/*
 *********************************************************************************************************
 *                                                Layer 2 Functions in driver
 *********************************************************************************************************
 */
#ifdef Layer2Function

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
s32 cfSetReadDataDma(u8 *buf, u32 siz)
{
    DMA_CFG dmaCfg;
    
    /* Set read data dma */
    dmaCfg.src = (u32)&(CfDataport);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / 16;
    dmaCfg.burst = 1;   /*CY 0907*/
//    if (dmaConfig(DMA_REQ_CF_READ, &dmaCfg) == 0)
//        return cfErrHandle(CF_ERROR_DMA_CONTROLLER_READ_ERROR, "DMA controller fails to set to read");
    guiCFReadDMAId = marsDMAReq(DMA_REQ_CF_READ, &dmaCfg);
    return CF_OK;
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
s32 cfSetWriteDataDma(u8 *buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* Set write data dma */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(CfDataport);
    dmaCfg.cnt = siz / 16;
    dmaCfg.burst = 1;   /*CY 0907*/
//    if (dmaConfig(DMA_REQ_CF_WRITE, &dmaCfg) == 0)
//        return cfErrHandle(CF_ERROR_DMA_CONTROLLER_WRITE_ERROR, "DMA controller fails to set to write");
  guiCFReadDMAId = marsDMAReq(DMA_REQ_CF_WRITE, &dmaCfg);
    return CF_OK;
}

/*

Routine Description:

    Check if dma is completed.

Arguments:

    None.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfCheckDmaComplete(void)
{

    u8 err;
    err = marsDMACheckReady(guiCFReadDMAId);
    guiCFReadDMAId = 0x55;
    return (err);
/*	
    u8 err;
    u32 i;

	
    OSSemPend(dmaSemChFin[DMA_CH_CF], CF_TIMEOUT, &err);

    if (err != OS_NO_ERR)
    {
        SYS_RSTCTL |= 0x00020000; 
        for (i = 0; i < 10000; i++);
        SYS_RSTCTL &= ~0x00020000; 

        
    
        DEBUG_CF("Error: dmaSemChFin[DMA_CH_SD] is %d.\n", err);

        return cfErrHandle(CF_ERROR_DMA_CF_CHANNEL_TIME_OUT, "dmaSemChFin[DMA_CH_SD] is time-out");
    }

    switch (dmaChResp[DMA_CH_CF])
    {
    case DMA_CH_FINISH:
        return CF_OK;

    case DMA_CH_ERROR:
        DEBUG_CF("-1 SDC: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
        DEBUG_CF("-1 SDC: DMA_INT = %d\n", g_intStat);
        return cfErrHandle(CF_ERROR_DMA_CH_ERROR, "dmaChResp[DMA_CH_SD] is channel error");

    default:
        DEBUG_CF("-2 SDC: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
        DEBUG_CF("-2 SDC: DMA_INT = %d\n", g_intStat);
        return cfErrHandle(CF_ERROR_DMA_CH_ERROR_2, "Error: dmaChResp[DMA_CH_SD] is error - 2");
    }
    */
}

/*

Routine Description:

    Device Selection Protocol

Arguments:

    ucDriveNum - The selected device number.
        0: Master
        1: Slave

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfDeviceSelectionProtocol(u8 ucDriveNum)
{
    if (cfWaitBusytoReady() != CF_OK)
        return cfErrHandle(CF_ERROR_UNDEF, "Undefine error");    
    
    /* Program Device Card/Drive/Head Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_SELECT_CARD_HEAD, ucCFCurrentMode);

    if (cfWaitBusytoReady() != CF_OK)
        return cfErrHandle(CF_ERROR_UNDEF, "Undefine error");    
    
    return CF_OK;
}

/*

Routine Description:

    Non-data command Protocol

Arguments:

    psDeviceRegisters - The task file.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfNonDataCommandProtocol(CF_DEVICE_REGISTER *psDeviceRegisters)
{
    u8 ucProtocolIndex;
    u8 ucI;
    u8 ucErr;

    /* Search command setting */
    ucProtocolIndex = 0xff;
    
    for (ucI = 0; ucI < CF_PROTOCOL_NUM; ucI++)
        if (ucProtocolEnable[ucI][0] == psDeviceRegisters->ucCommand)
        {
            ucProtocolIndex = ucI;
            break;
        }

    if (ucProtocolIndex == 0xff)
        return cfErrHandle(CF_ERROR_UNDEF_COMMAND, "Undefine command"); 


    /* Device Selection Protocol */
    cfDeviceSelectionProtocol(ucCFCurrentMode);

    /* Program Features Register */
    if (ucProtocolEnable[ucProtocolIndex][6])
        cfWriteCFATADeviceRegister(CF_ADDRESS_FEATURES, psDeviceRegisters->ucFeature);

    /* Program Device Sector Number Register */
    if (ucProtocolEnable[ucProtocolIndex][4])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_NO, psDeviceRegisters->ucSecNum);

    /* Program Device Sector Count Register */
    if (ucProtocolEnable[ucProtocolIndex][5])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_COUNT, psDeviceRegisters->ucSecCnt);

    /* Program Device Cylinder Registers */
    if (ucProtocolEnable[ucProtocolIndex][2] & ucProtocolEnable[ucProtocolIndex][3])
    {
        cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_HIGH, psDeviceRegisters->ucCylHigh);
        cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_LOW, psDeviceRegisters->ucCylLow);
    }

    /* Program Device Card/Drive/Head Register */
    if (ucProtocolEnable[ucProtocolIndex][1])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SELECT_CARD_HEAD, psDeviceRegisters->ucCardDriveHead);


    /* Program Device Command Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_COMMAND, psDeviceRegisters->ucCommand);


#if USE_INT /* Use Interrupt */
        OSSemPend(cfSemINTRQ, CF_TIMEOUT, &ucErr);

        if (ucErr != OS_NO_ERR)
        {
#if 0
            DEBUG_CF("Error: cfSemINTRQ is %d.\n", ucErr);
            DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
            DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
#endif            
            return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemINTRQ is time-out");
        }

#if 0
        /* Status register is read to clear pending interupt. */
        /* Read Status register */
        if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY,
                      CF_CARD_DEVICE_STATUS_ALTERNATE_RDY) != CF_OK)
            cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
#endif        
#else /* Use Polling */ /* No verification */
    /* This prevents polling host from reading status before it is valid. */
    /* Read Alternate Status register and ignore results */
    cfReadCFATADeviceRegister(CF_ADDRESS_ALT_STATUS, &unStatus);

    
    /* Status register is read to clear pending interupt. */
    /* Read Status register */
    if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ,
                  CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ) != CF_OK)
        cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
#endif

    /* Read and check Status register */
    cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_ALL, ucProtocolEnable[ucProtocolIndex][7]);
    
    return CF_OK;
}

/*

Routine Description:

    PIO dta in command Protocol

Arguments:

    psDeviceRegisters - The task file.
    unReadNumOfSectors - Number of sectors to be read
    pucBuffer - Allocated buffer to store the read data 

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfPIODataInProtocol(CF_DEVICE_REGISTER *psDeviceRegisters, u32 unReadNumOfSectors, u8 *pucBuffer)
{
    u8 ucProtocolIndex = 0xff;
    u32 unI;
    u8 ucErr;
    u32 unStatus;
    u8 *ReadBuffer;
//    u32 unJ;

#if 0
    /* Check parameters */
    if (unReadNumOfSectors > CF_MULTI_BLK)
        return cfErrHandle(CF_ERROR_INVALID_SECTOR_COUNT, "Invalid Sector Count"); 
#endif    

    /* Search command setting */
    for (unI = 0; unI < CF_PROTOCOL_NUM; unI++)
        if (ucProtocolEnable[unI][0] == psDeviceRegisters->ucCommand)
        {
            ucProtocolIndex = (u8)unI;
            break;
        }

    if (ucProtocolIndex == 0xff)
        return cfErrHandle(CF_ERROR_UNDEF_COMMAND, "Undefine command"); 


    /* Device Selection Protocol */
    cfDeviceSelectionProtocol(ucCFCurrentMode);

    /* Program Features Register */
    if (ucProtocolEnable[ucProtocolIndex][6])
        cfWriteCFATADeviceRegister(CF_ADDRESS_FEATURES, psDeviceRegisters->ucFeature);

    /* Program Device Sector Number Register */
    if (ucProtocolEnable[ucProtocolIndex][4])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_NO, psDeviceRegisters->ucSecNum);

    /* Program Device Sector Count Register */
    if (ucProtocolEnable[ucProtocolIndex][5])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_COUNT, psDeviceRegisters->ucSecCnt);

    /* Program Device Cylinder Registers */
    if (ucProtocolEnable[ucProtocolIndex][2] & ucProtocolEnable[ucProtocolIndex][3])
    {
        cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_HIGH, psDeviceRegisters->ucCylHigh);
        cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_LOW, psDeviceRegisters->ucCylLow);
    }

    /* Program Device Card/Drive/Head Register */
    if (ucProtocolEnable[ucProtocolIndex][1])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SELECT_CARD_HEAD, psDeviceRegisters->ucCardDriveHead);


    /* Program Device Command Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_COMMAND, psDeviceRegisters->ucCommand);

#if USE_INT /* Use Interrupt */
        OSSemPend(cfSemINTRQ, CF_TIMEOUT, &ucErr);

        if (ucErr != OS_NO_ERR)
        {
            DEBUG_CF("Error: cfSemINTRQ is %d.\n", ucErr);
//            DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
//            DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
            return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemINTRQ is time-out");
        }

        /* Status register is read to clear pending interupt. */
        /* Read Status register */
        if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ,
                      CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ) != CF_OK)
            cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
#else /* Use Polling */
    /* This prevents polling host from reading status before it is valid. */
    /* Read Alternate Status register and ignore results */
    cfReadCFATADeviceRegister(CF_ADDRESS_ALT_STATUS, &unStatus);

    
    /* Status register is read to clear pending interupt. */
    /* Read Status register */
    if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ,
                  CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ) != CF_OK)
        cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
#endif

//    for (unJ = 0; unJ < 100000; unJ++);

    /* (VCC $) Number of sectors to be read */
    /* Set Sector Count and Size */
    cfSectorCountAndSize(unReadNumOfSectors, CF_BLK_LEN);    

    /* Re-configure the number of sectors */
    if (unReadNumOfSectors == 0)
        unReadNumOfSectors = 256; 
    
    /* Read MULTIPLE */
    /* Read data from ATA device buffer. 256 words = 512 bytes */
    ReadBuffer = pucBuffer;
    if (psDeviceRegisters->ucCommand == CF_CMD_READ_MULTIPLE)
    {
        /* (VCC $) Set read data dma */
        cfSetReadDataDma(ReadBuffer, unReadNumOfSectors * CF_BLK_LEN);

        /* Read Data register */
        cfReadWriteCFATADeviceRegisterInDMAMode(CF_HOST_CONTROL_READ_DATA_PIO);

        /* Check dma completion */
        if (cfCheckDmaComplete() != CF_OK)
            return cfErrHandle(CF_ERROR_DMA_FAILS, "DMA operation fails");    

#if USE_INT /* Use Interrupt */
        OSSemPend(cfSemDMAComplete, CF_TIMEOUT, &ucErr);

        if (ucErr != OS_NO_ERR)
        {
            DEBUG_CF("Error: cfSemDMAComplete is %d.\n", ucErr);
 //           DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
//            DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
            return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemDMAComplete is time-out");
        }
#endif        

#if BIGENDIAN          
        for (unI = 0; unI < ((unReadNumOfSectors * CF_BLK_LEN) >> 1); unI++)
        {
            *ReadBuffer++ = (u8) ( (unData >> 8) & 0x00ff);
            *ReadBuffer++ = (u8) (  unData & 0x00ff);
        }
#endif                    
    }  
    else if (psDeviceRegisters->ucCommand == CF_CMD_READ_DMA)
    {
        /* (VCC $) Set read data dma */
        cfSetReadDataDma(ReadBuffer, unReadNumOfSectors * CF_BLK_LEN);
    
        /* Read Data register */
        cfReadWriteCFATADeviceRegisterInDMAMode(CF_HOST_CONTROL_READ_DATA_DMA);

        /* Check dma completion */
        if (cfCheckDmaComplete() != CF_OK)
            return cfErrHandle(CF_ERROR_DMA_FAILS, "DMA operation fails");    

#if USE_INT /* Use Interrupt */
        OSSemPend(cfSemDMAComplete, CF_TIMEOUT, &ucErr);

        if (ucErr != OS_NO_ERR)
        {
            DEBUG_CF("Error: cfSemDMAComplete is %d.\n", ucErr);
//            DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
//            DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
            return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemDMAComplete is time-out");
        }
#endif        

#if BIGENDIAN          
        for (unI = 0; unI < ((unReadNumOfSectors * CF_BLK_LEN) >> 1); unI++)
        {
            *ReadBuffer++ = (u8) ( (unData >> 8) & 0x00ff);
            *ReadBuffer++ = (u8) (  unData & 0x00ff);
        }
#endif                    
    }    
    /* Other PIO Data In commands */    
    else
    {
        for (unI = 0; unI < unReadNumOfSectors; unI++)
        {
            cfSectorCountAndSize(1, CF_BLK_LEN);
        
            /* (VCC $) Set read data dma */
            cfSetReadDataDma(ReadBuffer + CF_BLK_LEN * unI, CF_BLK_LEN);

            /* Read data from ATA device buffer. 256 words = 512 bytes */
            /* Read Data register */
            cfReadWriteCFATADeviceRegisterInDMAMode(CF_HOST_CONTROL_READ_DATA_PIO);

            /* Check dma completion */
            if (cfCheckDmaComplete() != CF_OK)
                return cfErrHandle(CF_ERROR_DMA_FAILS, "DMA operation fails");    

#if USE_INT /* Use Interrupt */
            OSSemPend(cfSemDMAComplete, CF_TIMEOUT, &ucErr);

            if (ucErr != OS_NO_ERR)
            {
                DEBUG_CF("Error: cfSemDMAComplete is %d.\n", ucErr);
//                DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
//                DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
                return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemDMAComplete is time-out");
            }
#endif        

#if 1
            /* Status register is read to clear pending interupt. */
            /* Read Status register */
            if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY,
                          CF_CARD_DEVICE_STATUS_ALTERNATE_RDY) != CF_OK)
                cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
#endif        

#if BIGENDIAN          
            /* Reformat */
            for (ucJ = 0; ucJ < (CF_BLK_LEN >> 1); ucJ++)
            {

                *ReadBuffer++ = (u8) ( (unData >> 8) & 0x00ff);
                *ReadBuffer++ = (u8) (  unData & 0x00ff);
            }    
#endif                    
    }
    }

    /* Read and check Status register */
    /* This prevents polling host from reading status before it is valid. */
    /* Read Alternate Status register and ignore results */
    cfReadCFATADeviceRegister(CF_ADDRESS_ALT_STATUS, &unStatus);

    
    /* Status register is read to clear pending interupt. */
    /* Read Status register */
    cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY,
                  CF_CARD_DEVICE_STATUS_ALTERNATE_RDY);
    
    return CF_OK;
}

/*

Routine Description:

    PIO dta out command Protocol

Arguments:

    psDeviceRegisters - The task file.
    unWriteNumOfSectors - Number of sectors to be write
    pucBuffer - Allocated buffer to store the write data 

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfPIODataOutProtocol(CF_DEVICE_REGISTER *psDeviceRegisters, u32 unWriteNumOfSectors, u8 *pucBuffer)
{
    u32 ucProtocolIndex = 0xff;
    u32 unI;
    u8 ucErr;
    u32 unStatus;
//    u16 *usWriteBuffer = (u8 *)pucBuffer;

#if 0
    /* Check parameters */
    if (unWriteNumOfSectors > CF_MULTI_BLK)
        return cfErrHandle(CF_ERROR_INVALID_SECTOR_COUNT, "Invalid Sector Count"); 
#endif

    /* Search command setting */
    for (unI = 0; unI < CF_PROTOCOL_NUM; unI++)
        if (ucProtocolEnable[unI][0] == psDeviceRegisters->ucCommand)
        {
            ucProtocolIndex = (u8)unI;
            break;
        }

    if (ucProtocolIndex == 0xff)
        return cfErrHandle(CF_ERROR_UNDEF_COMMAND, "Undefine command"); 


    /* Device Selection Protocol */
    cfDeviceSelectionProtocol(ucCFCurrentMode);

    /* Program Features Register */
    if (ucProtocolEnable[ucProtocolIndex][6])
        cfWriteCFATADeviceRegister(CF_ADDRESS_FEATURES, psDeviceRegisters->ucFeature);

    /* Program Device Sector Number Register */
    if (ucProtocolEnable[ucProtocolIndex][4])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_NO, psDeviceRegisters->ucSecNum);

    /* Program Device Sector Count Register */
    if (ucProtocolEnable[ucProtocolIndex][5])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_COUNT, psDeviceRegisters->ucSecCnt);

    /* Program Device Cylinder Registers */
    if (ucProtocolEnable[ucProtocolIndex][2] & ucProtocolEnable[ucProtocolIndex][3])
    {
        cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_HIGH, psDeviceRegisters->ucCylHigh);
        cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_LOW, psDeviceRegisters->ucCylLow);
    }

    /* Program Device Card/Drive/Head Register */
    if (ucProtocolEnable[ucProtocolIndex][1])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SELECT_CARD_HEAD, psDeviceRegisters->ucCardDriveHead);


    /* Program Device Command Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_COMMAND, psDeviceRegisters->ucCommand);


    /* This prevents polling host from reading status before it is valid. */
    /* Read Alternate Status register and ignore results */
    cfReadCFATADeviceRegister(CF_ADDRESS_ALT_STATUS, &unStatus);

    
    /* Status register is read to clear pending interupt. */
    /* Read Status register */
    if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ,
                  CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ) != CF_OK)
        cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            


//    usWriteBuffer = (u16 *)pucBuffer;
    /* (VCC $) Number of sectors to be written */
    /* Set Sector Count and Size */
    cfSectorCountAndSize(unWriteNumOfSectors, CF_BLK_LEN);

    /* Re-configure the number of sectors */
    if (unWriteNumOfSectors == 0)
        unWriteNumOfSectors = 256; 
    
    /* WRITE MULTIPLE */
    if (psDeviceRegisters->ucCommand == CF_CMD_WRITE_MULTIPLE)
    {
        /* Set write data dma */
        if (cfSetWriteDataDma(pucBuffer, unWriteNumOfSectors * CF_BLK_LEN) != 1)
            return cfErrHandle(CF_ERROR_DMA_CONTROLLER_WRITE_ERROR, "DMA controller fails to set to write");
   
        /* Write data to ATA device buffer. 256 words = 512 bytes */
//        for (ucI = 0; ucI < ((unWriteNumOfSectors * CF_BLK_LEN) >> 1); ucI++)
            /* Write Data register */
            cfReadWriteCFATADeviceRegisterInDMAMode(CF_HOST_CONTROL_WRITE_DATA_PIO);

        /* Check dma completion */
        if (cfCheckDmaComplete() != CF_OK)
            return cfErrHandle(CF_ERROR_DMA_FAILS, "DMA operation fails");    

#if USE_INT /* Use Interrupt */
        OSSemPend(cfSemINTRQ, CF_TIMEOUT, &ucErr);

        if (ucErr != OS_NO_ERR)
        {
            DEBUG_CF("Error: cfSemINTRQ is %d.\n", ucErr);
//            DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
//            DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
            return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemINTRQ is time-out");
        }

        /* Status register is read to clear pending interupt. */
        /* Read Status register */
        if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY,
                      CF_CARD_DEVICE_STATUS_ALTERNATE_RDY) != CF_OK)
            cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
#endif        
    }
    /* WRITE DMA */
    else if (psDeviceRegisters->ucCommand == CF_CMD_WRITE_DMA)
    {
        /* Set write data dma */
        if (cfSetWriteDataDma(pucBuffer, unWriteNumOfSectors * CF_BLK_LEN) != 1)
            return cfErrHandle(CF_ERROR_DMA_CONTROLLER_WRITE_ERROR, "DMA controller fails to set to write");

        /* Write data to ATA device buffer. 256 words = 512 bytes */
//        for (ucI = 0; ucI < ((unWriteNumOfSectors * CF_BLK_LEN) >> 1); ucI++)
            /* Write Data register */
            cfReadWriteCFATADeviceRegisterInDMAMode(CF_HOST_CONTROL_WRITE_DATA_PIO);

        /* Check dma completion */
        if (cfCheckDmaComplete() != CF_OK)
            return cfErrHandle(CF_ERROR_DMA_FAILS, "DMA operation fails");    

#if USE_INT /* Use Interrupt */
        OSSemPend(cfSemINTRQ, CF_TIMEOUT, &ucErr);

        if (ucErr != OS_NO_ERR)
        {
            DEBUG_CF("Error: cfSemINTRQ is %d.\n", ucErr);
 //           DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
//            DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
            return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemINTRQ is time-out");
        }

        /* Status register is read to clear pending interupt. */
        /* Read Status register */
        if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ,
                      CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ) != CF_OK)
            cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
#endif        
    }    
    /* Other PIO Data Out commands */     
    else
    {
        do
        {
            cfSectorCountAndSize(1, CF_BLK_LEN);
        
            /* Set write data dma */
            if (cfSetWriteDataDma(pucBuffer, CF_BLK_LEN) != 1)
                return cfErrHandle(CF_ERROR_DMA_CONTROLLER_WRITE_ERROR, "DMA controller fails to set to write");

            /* Write data to ATA device buffer. 256 words = 512 bytes */
//            for (ucI = 0; ucI < (CF_BLK_LEN >> 1); ucI++)
                /* Write Data register */
                cfReadWriteCFATADeviceRegisterInDMAMode(CF_HOST_CONTROL_WRITE_DATA_PIO);

            /* Check dma completion */
            if (cfCheckDmaComplete() != CF_OK)
                return cfErrHandle(CF_ERROR_DMA_FAILS, "DMA operation fails");    

#if USE_INT /* Use Interrupt */
            OSSemPend(cfSemINTRQ, CF_TIMEOUT, &ucErr);

            if (ucErr != OS_NO_ERR)
            {
                DEBUG_CF("Error: cfSemINTRQ is %d.\n", ucErr);
 //               DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
 //               DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
                return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemINTRQ is time-out");
            }

#if 1
            /* Status register is read to clear pending interupt. */
            /* Read Status register */
            if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY,
                          CF_CARD_DEVICE_STATUS_ALTERNATE_RDY) != CF_OK)
            {
                /* For debug */
#if 1
                /* Read and check Status register */
                cfReadCFATADeviceRegister(CF_ADDRESS_ALT_STATUS, &unStatus);
                DEBUG_CF("Status Register = 0x%x\n", unStatus);
                cfReadCFATADeviceRegister(CF_ADDRESS_ERROR, &unStatus);
                DEBUG_CF("Error Register = 0x%x\n", unStatus);            
#endif
                cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
            }
#endif        

#endif        
        } while (--unWriteNumOfSectors);
    }

#if 0
#if USE_INT /* Use Interrupt */
    OSSemPend(cfSemINTRQ, CF_TIMEOUT, &ucErr);

    if (ucErr != OS_NO_ERR)
    {
        DEBUG_CF("Error: cfSemINTRQ is %d.\n", ucErr);
        DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
        DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
        return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemINTRQ is time-out");
    }
#else /* Use Polling */
    
    /* Read and check Status register */
    /* This prevents polling host from reading status before it is valid. */
    /* Read Alternate Status register and ignore results */
    cfReadCFATADeviceRegister(CF_ADDRESS_ALT_STATUS, &unStatus);

    
    /* Status register is read to clear pending interupt. */
    /* Read Status register */
    cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY,
                  CF_CARD_DEVICE_STATUS_ALTERNATE_RDY);
#endif
#endif    
    return CF_OK;
}

/*

Routine Description:

    DMA command Protocol

Arguments:

    psDeviceRegisters - The task file.
    unReadWriteNumOfSectors - Number of sectors to be read
    pucBuffer - Allocated buffer to store the read/write data

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfDMACommandProtocol(CF_DEVICE_REGISTER *psDeviceRegisters, u32 unReadWriteNumOfSectors, u8 *pucBuffer)
{
    u8 ucProtocolIndex = 0xff;
    u32 unI;
    u8 ucErr;
    u32 unStatus;
//    u8 *ReadBuffer;
//    u32 unJ;

#if 0
    /* Check parameters */
    if (unReadWriteNumOfSectors > CF_MULTI_BLK)
        return cfErrHandle(CF_ERROR_INVALID_SECTOR_COUNT, "Invalid Sector Count"); 
#endif

    /* Search command setting */
    for (unI = 0; unI < CF_PROTOCOL_NUM; unI++)
        if (ucProtocolEnable[unI][0] == psDeviceRegisters->ucCommand)
        {
            ucProtocolIndex = (u8)unI;
            break;
        }

    if (ucProtocolIndex == 0xff)
        return cfErrHandle(CF_ERROR_UNDEF_COMMAND, "Undefine command"); 


    /* Device Selection Protocol */
    cfDeviceSelectionProtocol(ucCFCurrentMode);

    /* Program Features Register */
    if (ucProtocolEnable[ucProtocolIndex][6])
        cfWriteCFATADeviceRegister(CF_ADDRESS_FEATURES, psDeviceRegisters->ucFeature);

    /* Program Device Sector Number Register */
    if (ucProtocolEnable[ucProtocolIndex][4])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_NO, psDeviceRegisters->ucSecNum);

    /* Program Device Sector Count Register */
    if (ucProtocolEnable[ucProtocolIndex][5])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SECTOR_COUNT, psDeviceRegisters->ucSecCnt);

    /* Program Device Cylinder Registers */
    if (ucProtocolEnable[ucProtocolIndex][2] & ucProtocolEnable[ucProtocolIndex][3])
    {
        cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_HIGH, psDeviceRegisters->ucCylHigh);
        cfWriteCFATADeviceRegister(CF_ADDRESS_CYLINDER_LOW, psDeviceRegisters->ucCylLow);
    }

    /* Program Device Card/Drive/Head Register */
    if (ucProtocolEnable[ucProtocolIndex][1])
        cfWriteCFATADeviceRegister(CF_ADDRESS_SELECT_CARD_HEAD, psDeviceRegisters->ucCardDriveHead);

#if 1
    /* Program Device Command Register */
    cfWriteCFATADeviceRegister(CF_ADDRESS_COMMAND, psDeviceRegisters->ucCommand);

#if 0
#if USE_INT /* Use Interrupt */
    OSSemPend(cfSemINTRQ, CF_TIMEOUT, &ucErr);

    if (ucErr != OS_NO_ERR)
    {
        DEBUG_CF("Error: cfSemINTRQ is %d.\n", ucErr);
        DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
        DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
        return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemINTRQ is time-out");
    }

    /* Status register is read to clear pending interupt. */
    /* Read Status register */
    if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ,
                  CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ) != CF_OK)
        cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
#else /* Use Polling */
    /* This prevents polling host from reading status before it is valid. */
    /* Read Alternate Status register and ignore results */
    cfReadCFATADeviceRegister(CF_ADDRESS_ALT_STATUS, &unStatus);

    
    /* Status register is read to clear pending interupt. */
    /* Read Status register */
    if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ,
                  CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ) != CF_OK)
        cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
#endif
#endif

#endif

    /* (VCC $) Number of sectors to be written */
    /* Set Sector Count and Size */
    cfSectorCountAndSize(unReadWriteNumOfSectors, CF_BLK_LEN);

    /* Re-configure the number of sectors */
    if (unReadWriteNumOfSectors == 0)
        unReadWriteNumOfSectors = 256; 
    
    /* WRITE DMA */
    /* Write data to ATA device buffer. 256 words = 512 bytes */
    if (psDeviceRegisters->ucCommand == CF_CMD_WRITE_DMA)
    {
        /* Set write data dma */
        if (cfSetWriteDataDma(pucBuffer, unReadWriteNumOfSectors * CF_BLK_LEN) != 1)
            return cfErrHandle(CF_ERROR_DMA_CONTROLLER_WRITE_ERROR, "DMA controller fails to set to write");

        /* Write data to ATA device buffer. 256 words = 512 bytes */
//        for (ucI = 0; ucI < ((unWriteNumOfSectors * CF_BLK_LEN) >> 1); ucI++)
            /* Write Data register */
        cfReadWriteCFATADeviceRegisterInDMAMode(CF_HOST_CONTROL_WRITE_DATA_DMA);

        /* Check dma completion */
        if (cfCheckDmaComplete() != CF_OK)
            return cfErrHandle(CF_ERROR_DMA_FAILS, "DMA operation fails");    

#if USE_INT/* Use Interrupt */
        OSSemPend(cfSemINTRQ, CF_TIMEOUT, &ucErr);

        if (ucErr != OS_NO_ERR)
        {
            DEBUG_CF("Error: cfSemINTRQ is %d.\n", ucErr);
 //           DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
 //           DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
            return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemINTRQ is time-out");
        }

        /* Status register is read to clear pending interupt. */
        /* Read Status register */
        if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY,
                      CF_CARD_DEVICE_STATUS_ALTERNATE_RDY) != CF_OK)
            cfErrHandle(CF_ERROR_UNDEF, "Undefine error");            
#endif        
    }  
    else if (psDeviceRegisters->ucCommand == CF_CMD_READ_DMA)
    {
        /* (VCC $) Set read data dma */
        cfSetReadDataDma(pucBuffer, unReadWriteNumOfSectors * CF_BLK_LEN);

        /* Read Data register */
        cfReadWriteCFATADeviceRegisterInDMAMode(CF_HOST_CONTROL_READ_DATA_DMA);

        /* Check dma completion */
        if (cfCheckDmaComplete() != CF_OK)
            return cfErrHandle(CF_ERROR_DMA_FAILS, "DMA operation fails");    

#if USE_INT /* Use Interrupt */
        OSSemPend(cfSemDMAComplete, CF_TIMEOUT, &ucErr);

        if (ucErr != OS_NO_ERR)
        {
            DEBUG_CF("Error: cfSemDMAComplete is %d.\n", ucErr);
 //           DEBUG_CF("CF: CH1=%d, Ccnt=%d\n", DMA_CH1_ENA, DmaCh1CycCnt);
//            DEBUG_CF("CF: DMA_INT = %d\n", g_intStat);
            return cfErrHandle(CF_ERROR_COMMAND_RESPONSE_TIME_OUT, "cfSemDMAComplete is time-out");
        }
#endif        

#if BIGENDIAN          
        for (unI = 0; unI < ((unReadWriteNumOfSectors * CF_BLK_LEN) >> 1); unI++)
        {
            *pucBuffer++ = (u8) ( (unData >> 8) & 0x00ff);
            *pucBuffer++ = (u8) (  unData & 0x00ff);
        }
#endif                    
    }    
    

    /* Read and check Status register */
    /* This prevents polling host from reading status before it is valid. */
    /* Read Alternate Status register and ignore results */
    cfReadCFATADeviceRegister(CF_ADDRESS_ALT_STATUS, &unStatus);

    
    /* Status register is read to clear pending interupt. */
    /* Read Status register */
    cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY
                | CF_CARD_DEVICE_STATUS_ALTERNATE_DRQ,
                  CF_CARD_DEVICE_STATUS_ALTERNATE_RDY);
    
    return CF_OK;
}

/*

Routine Description:

    Initialize ATA device.

Arguments:

    None.

Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfInitCFDevice(void)
{
//    CF_DEVICE_REGISTER sDeviceRegisters;

    /* (VCC $) AT Disk Controller Soft Reset operation */
    CfCardResetTime = 0x00000400;
    CfCardReset = CF_CARD_RESET_TRUE_IDE;
//    cfSWResetDevice();
        
    return CF_OK;
}

#endif

/*
 *********************************************************************************************************
 *                                                Layer 1 Functions in driver
 *********************************************************************************************************
 */

#ifdef Layer1Function

/*

Routine Description:

    Identify Drive and get the ATA device information.

Arguments:

    ucDriveNum - The selected device number.
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfIdentifyDrive(u8 ucDriveNum)
{
    u32 unI;
    u8  ucTempChar;
    CF_DEVICE_REGISTER sDeviceRegisters;

//    /* Need to provide? */
//    ucCFCurrentMode = ucDriveNum;

    /* IDENTIFY DEVICE Command */
    sDeviceRegisters.ucCommand = CF_CMD_IDENTIFY_DEV;
	sDeviceRegisters.ucCardDriveHead = ucDriveNum;
    cfPIODataInProtocol(&sDeviceRegisters, 1, (u8 *)(&cfSHDInformation));

    /* Swap ASCII chars in the HDInformation */
#if !BIGENDIAN
    for (unI = 0; unI < 40; unI+=2)
        SWAP(cfSHDInformation.model_num[unI], cfSHDInformation.model_num[unI + 1], ucTempChar);
    
    for (unI = 0; unI < 8; unI+=2)
        SWAP(cfSHDInformation.firware_rev[unI], cfSHDInformation.firware_rev[unI + 1], ucTempChar);
    
    for (unI = 0; unI < 20; unI += 2)
        SWAP(cfSHDInformation.ser_no[unI], cfSHDInformation.ser_no[unI + 1], ucTempChar);
#endif    

    cfSHDInformation.support_max_sector_no &= 0xFF;

    /* Get Number of logical cylinders, Number of logical heads, 
       Number of logical sectors per logical track */
    cfCylindersMax  = cfSHDInformation.cylinders;
    cfSectorsMax = cfSHDInformation.heads;
    cfSerNoMax  = cfSHDInformation.sectors;    
 	cfTotalBlockCount = cfSHDInformation.lba_capacity;
    return CF_OK;
}

/*

Routine Description:

    SET MULTIPLE command.

Arguments:

    unSectorCount - The number of sectors to be written.
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
static s32 cfSetMultiple(u32 unSectorCount)
{
    CF_DEVICE_REGISTER sDeviceRegisters;

    /* SET MULTIPLE MODE Command */
    sDeviceRegisters.ucCommand = CF_CMD_SET_MULTIPLE;
    sDeviceRegisters.ucCardDriveHead = ucCFCurrentMode;
//    sDeviceRegisters.ucSecCnt  = CF_MULTI_BLK;
    sDeviceRegisters.ucSecCnt  = unSectorCount;
    cfNonDataCommandProtocol(&sDeviceRegisters); 

    return CF_OK;
}

/*

Routine Description:

    Read single/multiple sectors.

Arguments:

    unStartSector - The start sector to be read.
    unSectorCount - The number of sectors to be read.
    dataBuf - The buffer stored read data
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfReadSectors(u32 unStartSector, u32 unSectorCount, u8 *dataBuf)
{
    CF_DEVICE_REGISTER sDeviceRegisters;

    /* Check parameters */
    if (unSectorCount > 256)
        DEBUG_CF("CF Error!!\n");
    else if (unSectorCount == 256)
        unSectorCount = 0;

    /* READ SECTOR(S) Command */
    sDeviceRegisters.ucCommand = CF_CMD_READ_SECTOR;
    sDeviceRegisters.ucCardDriveHead = ucCFCurrentMode | (u8)((unStartSector & 0x0f000000) >> 24);   
    sDeviceRegisters.ucCylHigh = (u8)((unStartSector & 0x00ff0000) >> 16);    
    sDeviceRegisters.ucCylLow =  (u8)((unStartSector & 0x0000ff00) >> 8);    
    sDeviceRegisters.ucSecNum =  (u8)(unStartSector & 0x000000ff);
    sDeviceRegisters.ucSecCnt =  unSectorCount;    
    cfPIODataInProtocol(&sDeviceRegisters, unSectorCount, dataBuf);
 
    return CF_OK;
}

/*

Routine Description:

    Read single/multiple sectors with READ MULTIPLE command.

Arguments:

    unStartSector - The start sector to be read.
    unSectorCount - The number of sectors to be read.
    dataBuf - The buffer stored read data
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfReadMultipleSectors(u32 unStartSector, u32 unSectorCount, u8 *dataBuf)
{
    CF_DEVICE_REGISTER sDeviceRegisters;

    /* Check parameters */
    if (unSectorCount > 256)
        DEBUG_CF("CF Error!!\n");
    else if (unSectorCount == 256)
        unSectorCount = 0;

    /* READ MULTIPLE Command */
    sDeviceRegisters.ucCommand = CF_CMD_READ_MULTIPLE;
    sDeviceRegisters.ucCardDriveHead = ucCFCurrentMode | (u8)((unStartSector & 0x0f000000) >> 24);   
    sDeviceRegisters.ucCylHigh = (u8)((unStartSector & 0x00ff0000) >> 16);    
    sDeviceRegisters.ucCylLow =  (u8)((unStartSector & 0x0000ff00) >> 8);    
    sDeviceRegisters.ucSecNum =  (u8)(unStartSector & 0x000000ff);
    sDeviceRegisters.ucSecCnt =  unSectorCount;    
    cfPIODataInProtocol(&sDeviceRegisters, unSectorCount, dataBuf);
 
    return CF_OK;
}

/*

Routine Description:

    Read single/multiple sectors with READ DMA command.

Arguments:

    unStartSector - The start sector to be read.
    unSectorCount - The number of sectors to be read.
    dataBuf - The buffer stored read data
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfReadMultipleSectorsDMA(u32 unStartSector, u32 unSectorCount, u8 *dataBuf)
{
    CF_DEVICE_REGISTER sDeviceRegisters;

    /* Check parameters */
    if (unSectorCount > 256)
        DEBUG_CF("CF Error!!\n");
    else if (unSectorCount == 256)
        unSectorCount = 0;

    /* READ MULTIPLE Command */
    sDeviceRegisters.ucCommand = CF_CMD_READ_DMA;
    sDeviceRegisters.ucCardDriveHead = ucCFCurrentMode | (u8)((unStartSector & 0x0f000000) >> 24);   
    sDeviceRegisters.ucCylHigh = (u8)((unStartSector & 0x00ff0000) >> 16);    
    sDeviceRegisters.ucCylLow =  (u8)((unStartSector & 0x0000ff00) >> 8);    
    sDeviceRegisters.ucSecNum =  (u8)(unStartSector & 0x000000ff);
    sDeviceRegisters.ucSecCnt =  unSectorCount;    
    cfDMACommandProtocol(&sDeviceRegisters, unSectorCount, dataBuf);
 
    return CF_OK;
}

/*

Routine Description:

    Write single/multiple sectors.

Arguments:

    unStartSector - The start sector to be written.
    unSectorCount - The number of sectors to be written.
    dataBuf - The buffer stored write data
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfWriteSectors(u32 unStartSector, u32 unSectorCount, u8 *dataBuf)
{
    CF_DEVICE_REGISTER sDeviceRegisters;

    /* Check parameters */
    if (unSectorCount > 256)
        DEBUG_CF("CF Error!!\n");
    else if (unSectorCount == 256)
        unSectorCount = 0;

    /* WRITE SECTOR(S) Command */
    sDeviceRegisters.ucCommand = CF_CMD_WRITE_SECTOR;
    sDeviceRegisters.ucCardDriveHead = ucCFCurrentMode | (u8)((unStartSector & 0x0f000000) >> 24);   
    sDeviceRegisters.ucCylHigh = (u8)((unStartSector & 0x00ff0000) >> 16);    
    sDeviceRegisters.ucCylLow =  (u8)((unStartSector & 0x0000ff00) >> 8);    
    sDeviceRegisters.ucSecNum =  (u8)(unStartSector & 0x000000ff);
    sDeviceRegisters.ucSecCnt =  unSectorCount;    
    cfPIODataOutProtocol(&sDeviceRegisters, unSectorCount, dataBuf);
 
    return CF_OK;
}

/*

Routine Description:

    Write single/multiple sectors with WRITE MULTIPLE command.

Arguments:

    unStartSector - The start sector to be written.
    unSectorCount - The number of sectors to be written.
    dataBuf - The buffer stored write data
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfWriteMultipleSectors(u32 unStartSector, u32 unSectorCount, u8 *dataBuf)
{
    CF_DEVICE_REGISTER sDeviceRegisters;

    /* Check parameters */
    if (unSectorCount > 256)
        DEBUG_CF("CF Error!!\n");
    else if (unSectorCount == 256)
        unSectorCount = 0;

    /* WRITE MULTIPLE Command */
    sDeviceRegisters.ucCommand = CF_CMD_WRITE_MULTIPLE;
    sDeviceRegisters.ucCardDriveHead = ucCFCurrentMode | (u8)((unStartSector & 0x0f000000) >> 24);   
    sDeviceRegisters.ucCylHigh = (u8)((unStartSector & 0x00ff0000) >> 16);    
    sDeviceRegisters.ucCylLow =  (u8)((unStartSector & 0x0000ff00) >> 8);    
    sDeviceRegisters.ucSecNum =  (u8)(unStartSector & 0x000000ff);
    sDeviceRegisters.ucSecCnt =  unSectorCount;    
    cfPIODataOutProtocol(&sDeviceRegisters, unSectorCount, dataBuf);
 
    return CF_OK;
}

/*

Routine Description:

    Write single/multiple sectors with WRITE DMA command.

Arguments:

    unStartSector - The start sector to be written.
    unSectorCount - The number of sectors to be written.
    dataBuf - The buffer stored write data
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfWriteMultipleSectorsDMA(u32 unStartSector, u32 unSectorCount, u8 *dataBuf)
{
    CF_DEVICE_REGISTER sDeviceRegisters;

    /* Check parameters */
    if (unSectorCount > 256)
        DEBUG_CF("CF over Error!!\n");
    else if (unSectorCount == 256)
        unSectorCount = 0;

    /* WRITE MULTIPLE Command */
    sDeviceRegisters.ucCommand = CF_CMD_WRITE_DMA;
    sDeviceRegisters.ucCardDriveHead = ucCFCurrentMode | (u8)((unStartSector & 0x0f000000) >> 24);   
    sDeviceRegisters.ucCylHigh = (u8)((unStartSector & 0x00ff0000) >> 16);    
    sDeviceRegisters.ucCylLow =  (u8)((unStartSector & 0x0000ff00) >> 8);    
    sDeviceRegisters.ucSecNum =  (u8)(unStartSector & 0x000000ff);
    sDeviceRegisters.ucSecCnt =  unSectorCount;    
    cfDMACommandProtocol(&sDeviceRegisters, unSectorCount, dataBuf);
 
    return CF_OK;
}

/*

Routine Description:

    Erase sectors command.

Arguments:

    unStartSector - The start sector to be written.
    unSectorCount - The number of sectors to be written.
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfEraseSectors(u32 unStartSector, u8 unSectorCount)
{
    CF_DEVICE_REGISTER sDeviceRegisters;

    /* CFA ERASE SECTORS Command */
    sDeviceRegisters.ucCommand = CF_CMD_ERASE_SECTORS;
    sDeviceRegisters.ucCardDriveHead = ucCFCurrentMode | (u8)((unStartSector & 0x0f000000) >> 24);   
    sDeviceRegisters.ucCylHigh = (u8)((unStartSector & 0x00ff0000) >> 16);    
    sDeviceRegisters.ucCylLow =  (u8)((unStartSector & 0x0000ff00) >> 8);    
    sDeviceRegisters.ucSecNum =  (u8)(unStartSector & 0x000000ff);
    sDeviceRegisters.ucSecCnt =  unSectorCount;    
    cfNonDataCommandProtocol(&sDeviceRegisters);
 
    return CF_OK;
}

/*

Routine Description:

    SET FEATURES command.
#define 
Arguments:

    unStartSector - The start sector to be written.
    unSectorCount - The number of sectors to be written.
    
Return Value:

    <0 - Failure.
    1 - Success.

*/
s32 cfSetTransferMode(u8 ucTransferMode, u8 ucModeType)
{
    CF_DEVICE_REGISTER sDeviceRegisters;

#if 1
    /* CFA ERASE SECTORS Command */
    sDeviceRegisters.ucCommand = CF_CMD_SET_FEATURE;
    sDeviceRegisters.ucCardDriveHead = ucCFCurrentMode;
    sDeviceRegisters.ucSecCnt =  ucTransferMode | ucModeType;    
    sDeviceRegisters.ucFeature = 0x03;
    cfNonDataCommandProtocol(&sDeviceRegisters);
#endif    
      DEBUG_CF("SetTransferMode :%x \n",sDeviceRegisters.ucSecCnt);
#if 1
    /* For debug */    
    /* CF True-IDE Mode Timing Control Register */
    if (ucTransferMode == PIO_MODE)
    	{
        	CfIdemodeReadTime = cfPIOTiming[ucModeType];
			CfIdemodeWriteTime= cfPIOTiming[ucModeType];
    	}
    else if (ucTransferMode == DMA_MODE)
    	{
        	CfIdemodeReadTime= cfMultiDMATiming[ucModeType];
			CfIdemodeWriteTime= cfMultiDMATiming[ucModeType];
    	}
#endif
 
    return CF_OK;
}

/*

Routine Description:

    Initialize System, GPIO, CF/ATA Host Controller, and ATA device.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 cfInit(void)
{
    /* Create the semaphore */
    // cfSemEvt = OSSemCreate(0);
    cfSemDMAComplete = OSSemCreate(0);    
    cfSemINTRQ = OSSemCreate(0);    

    /* For debug */
 //   *((volatile unsigned *)(IdeCtrlBase + 0x0054)) = 7;

    /* Clock enable of CF Host controller */
    //SYS_CTL0 |= SYS_CTL0_SD_CK_EN;
	SYSClkEnable(SYS_CTL0_CF_CKEN);
    SYSReset(SYS_RSTCTL_CF_RST);  


    /* Flash io pin seleted */
    GpioActFlashSelect |= GPIO_ACT_FLASH_CF;

    /* Initialize CF/ATA Device Controller */
    cfInitCFDevice();
	
    /* Initialize CF/ATA Host Controller */
    cfInitCFHost();
	ucCFCurrentMode =CF_CMD_master_mode;
   if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY,
                          CF_CARD_DEVICE_STATUS_ALTERNATE_RDY) != CF_OK)
   	{
		DEBUG_CF("Master_Unready \n");
		ucCFCurrentMode =CF_CMD_slave_mode;
   		if (cfCheckStatus(CF_CARD_DEVICE_STATUS_ALTERNATE_BUSY | CF_CARD_DEVICE_STATUS_ALTERNATE_RDY,
                          CF_CARD_DEVICE_STATUS_ALTERNATE_RDY) != CF_OK)
   		{
			DEBUG_CF("Slave_Unready \n");  
			return CF_Fail; //IDE fail
   		}
		else
   		{
   			DEBUG_CF("Slave_Ready \n");
   		}
   	}
   else
   	{
   		DEBUG_CF("Master_Ready \n");
   	}
    /* Set transfer mode */
    cfSetTransferMode(PIO_MODE, MODE_0);

    /* Identify Drive and get the ATA device information */
    /* Select master mode */ 

    cfIdentifyDrive(ucCFCurrentMode);

    /* Set sector count per block */
 //   cfSetMultiple(cfSHDInformation.support_max_sector_no);
    
    return CF_OK;
}

/*

Routine Description:

    Mount CF/ATA device.

Arguments:

    None.

Return Value:


*/
s32 cfMount(void)
{
#if 0

#endif    
    return CF_OK;
}

#endif
#endif 
