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
#if 1
#include "general.h"
#if(FLASH_OPTION == FLASH_NAND_9002_NORMAL)
#include "board.h"
#include "osapi.h"
#include "smcapi.h"
#include "smc.h"
#include "smcreg.h"
#include "smcwc.h"
//#include "dmaapi.h"
#include <../inc/mars_controller/mars_dma.h>
#include "fsapi.h"	/* cytsai: 0315 */
#include "rtcapi.h"
#include "dcfapi.h"
#include "SmcECCSW.h"
#include "MemoryPool.h"
#include "sysapi.h"

/*
 *********************************************************************************************************
 *												 CONSTANTS
 *********************************************************************************************************
 */

/* SMC time out value */
#define SMC_TIMEOUT				20	/*CY 1023*/

#if 0
/* Max. SMC page size */
#define SMC_MAX_PAGE_SIZE			512
#define SMC_MAX_REDUN_SIZE			16

/* Multi-page test */
#define SMC_MULTI_PAGE				4
#endif

/* SMC Zone */
#define SMC_MAX_ZONE_ENTRY			1024

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

__align(4) SMC_REDUN_AREA smcRedunArea;
__align(4) u8  smcReadBuf[SMC_MAX_PAGE_SIZE];
__align(4) u8  smcWriteBuf[SMC_MAX_PAGE_SIZE];

//u8 *smcReadBuf;// = (u8*) PKBuf0;
//u8 *smcWriteBuf;// = (u8*) PKBuf1;

u8 make_BitMap_ok = 0;

u32 smcZoneSize = 0;
u32 smcTotalSize, smcBlockSize, smcPageSize, smcPagePerBlock;
u32 smcTotalPageCount; /* cytsai */
u32 smcPageRedunSize;
u32 smcTotalZone;
u32 smcAddrCycle;
u32 smcPagePerBlock;
u32	smcSecPerBlock;
u32	smcSecPerPage;
u8	ucAccessFlag = 0;	/* flag to indicates the status for other access */

void	*smcMboxMsg;

OS_EVENT* smcMboxEvt;		/* mbox to tramsmitt message */
OS_EVENT* smcSemEvt;		/* semaphore to synchronize smc operation completion event processing */
OS_EVENT* smcProcProtSemEvt;		/* semaphore to synchronize procedure protection */

extern s32 dcfStorageSize;	/*CY 0718*/
extern u8 Rerseved_Algo_Start;

NAND_FAT_BPB NAND_FAT_PARAMETER;
NAND_BBM SMC_BBM;

unsigned char	 smcBitMap[SMC_MAX_MAP_SIZE_IN_BYTE];
extern u8 Rerseved_Algo_Start;

u32 guiSMCReadDMAId=0xFF, guiSMCWriteDMAId=0xFF;
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
/* Driver Function */
s32 smcDetectCard(void);
u8 smcParityCheck(u16);
u16 smcCalcLbaField(u16);
u16 smcCalcLba(u16);
s32 smcSetReadWriteAddr(u32);
s32 smcSetEraseAddr(u32);
s32 smcGetRedunArea(SMC_REDUN_AREA*);
s32 smcSetRedunArea(u16);
s32 smcSetReadDataDma(u8*, u32);
s32 smcSetWriteDataDma(u8*, u32);
s32 smcCheckDmaReadReady(void);
s32 smcCheckDmaWriteReady(void);
s32 smcCheckDeviceReady(void);
s32 smcCheckOperationReady(void);
s32 smcCheckReadReady(void);
s32 smcCheckRead2CReady(void);
s32 smcCheckWriteReady(void);

/* Middleware Function */
s32 smcIdentification(void);
s32 smcReset(void);
s32 smcReadStatus(u8*);
s32 smcIdRead(u8*, u8*);
s32 smcPage1ARead(u32, u8*);
s32 smcPage1BRead(u32, u8*);
s32 smcPage2CRead(u32);
s32 smcPageProgram(u32, u8*, u16);
s32 smcRedunProgram(u32, u16);
s32 smcBlockErase(u32);
//s32 smcTotalBlockErase(void);
void smcTotalBlockErase(char);
s32 smcMultiBlockErase(u32, u32);
//u8 BBM_Handle(u32,u8*);
NAND_FAT_BPB NAND_FAT_PARAMETER;
NAND_BBM SMC_BBM;
extern u8 smcMakeBitMap(char);
void smcBBM_init(void);



u8 smcRegReadWriteTest(void);	//Lori-test
u8 smcECCSWResult(u32*);	//Lori-test
s8 smcDMAVerify(u32, u16);		//Lori-test
s8 smcVerification(void);	//Lori-test

//extern s32 smcMakeTable(void);
extern s32 smcSectorsRead(u32, u32, u8*);
extern s32 smcSectorsWrite(u32, u32, u8*);
extern s32 dmaConfigTest(u8, DMA_CFG*, u8, u8);

extern s32 gpioSetLevel(u8, u8, u8);

/*
 *********************************************************************************************************
 * Driver function
 *********************************************************************************************************
 */

//****************************
//Funcstions For Verification
//****************************

u8 smcRegReadWriteTest(void);
u8 smcECCSWResult(u32*);
s8 smcVerifyDefultValue(void);
s8 smcVerifyBlockErase(u32);
void smcSetRedunAreaTest(u16, u32);
s8 smcSetWriteDataDmaTest(u8*, u32, s8, u8);
s8 smcPageProgramDMATest(u32, u8*, u16, s8, u8);
s32 smcPage1AReadDMATest(u32, u8*, s8, u8);
s32 smcSetReadDataDMATest(u8*, u32, s8, u8);
s8 smcDMAVerify(u32, u16);
s8 smcVerification(void);




/*
 *********************************************************************************************************
 * Function Body
 *********************************************************************************************************
 */


/*

Routine Description:

	Parity check the Block Address Field of SMC/NAND gate flash.

Arguments:

	d - The Block Address Field.

Return Value:

	The parity.

*/
u8 smcParityCheck(u16 d)
{
    u8 d1 = (u8)(d >> 8);
    u8 d2 = (u8) d;
    u8 p;

    p = d1 ^ d2;					/* byte   */
    p = (p >> 4) ^ (p & 0x0f);		/* nibble */
    p = (p >> 2) ^ (p & 0x03);		/* 2-bit  */
    p = (p >> 1) ^ (p & 0x01);		/* 1-bit  */

    return p;
}

/*

Routine Description:

	Calculate the Block Address Field from logical block address of SMC/NAND gate flash.

Arguments:

	logBlockAddr - The logical block address.

Return Value:

	The Block Address Field.

*/
u16 smcCalcLbaField(u16 logBlockAddr)
{
    u16 adrs = logBlockAddr;

    adrs = (adrs << 1) | 0x1000;
    adrs |= smcParityCheck(adrs);

    return adrs;
}

/*

Routine Description:

	Calculate the logical block address from the Block Adderss Field of SMC/NAND gate flash.

Arguments:

	adrsfield - The Block Address field.

Return Value:

	The logical block address.

*/
u16 smcCalcLba(u16 adrsfield)
{
    u16 adrs;

    adrs = (adrsfield & 0x07fe) >> 1;

    return adrs;
}


/*

Routine Description:

	Read/ Write procedure protection. To prevent other new accessing to interrupt the previous running ones.

Arguments:

	ucSetReq - "1"-> Set request.
	ucAccEna - "0"->Access Prohibited. "1"->Access Permitted.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8	smcProcProt(u8 ucSetReq, u8 ucAccEna)
{
    u8	err;


    /* acquire a semaphore */
    OSSemPend(smcProcProtSemEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_USB("SMC ERR: smcProcProtSemEvt is %d.\n", err);
    }

    if (ucSetReq == 1)
    {	/* Set Status */
        ucAccessFlag = ucAccEna;
    }

    /* release semaphore */
    OSSemPost(smcProcProtSemEvt);

    return ucAccessFlag;

}

/*

Routine Description:

	Set the address of page read/write command.

Arguments:

	addr - The page to read/write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcSetReadWriteAddr(u32 addr)
{

    if (smcAddrCycle == 4)
    {
        SmcCfg |= SMC_CFG_ADDR_4CYCL;
        SmcAddr = ((addr >> 9) << 8) | (addr & 0x000000ff);
    }
    else if (smcAddrCycle == 3)
    {
        SmcCfg |= SMC_CFG_ADDR_3CYCL;
        SmcAddr = ((addr >> 9) << 8) | (addr & 0x000000ff);
    }
    else
    {
        /* not supported */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Set the address of block erase command.

Arguments:

	addr - The block address to erase.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcSetEraseAddr(u32 addr)
{
    if (smcAddrCycle == 4)
    {
        SmcCfg |= SMC_CFG_ADDR_3CYCL;
        SmcAddr = addr >> 9;
    }
    else if (smcAddrCycle == 3)
    {
        SmcCfg |= SMC_CFG_ADDR_2CYCL;
        SmcAddr = addr >> 9;
    }
    else
    {
        /* not supported */
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Get redundant area.

Arguments:

	pRedunArea - The redundant area.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcGetRedunArea(SMC_REDUN_AREA* pRedunArea)
{
    u32 ecc1 = SmcEcc1;
    u32 ecc2 = SmcEcc2;
    u8 redun[16];
    u32* redunPtr = (u32*) redun;

    *redunPtr++ = SmcRedun0;
    *redunPtr++ = SmcRedun1;
    *redunPtr++ = SmcRedun2;
    *redunPtr	= SmcRedun3;

    pRedunArea->user_data = (((u32)redun[0]) << 24) | (((u32)redun[1]) << 16) |
                            (((u32)redun[2]) <<  8) | (((u32)redun[3]));
    pRedunArea->data_status = redun[1];
    pRedunArea->block_status = redun[0];
    pRedunArea->block_addr1 = (((u16)redun[6]) <<  8) | ((u16)redun[7]);
    pRedunArea->ecc2[0] = redun[8];
    pRedunArea->ecc2[1] = redun[9];
    pRedunArea->ecc2[2] = redun[10];
    pRedunArea->block_addr2 = (((u16)redun[11]) <<	8) | ((u16)redun[12]);
    pRedunArea->ecc1[0] = redun[13];
    pRedunArea->ecc1[1] = redun[14];
    pRedunArea->ecc1[2] = redun[15];

    return 1;

}

/*

Routine Description:

	Set the redundant area.

Arguments:

	blockAddrField - The Block Address Field.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcSetRedunArea(u16 blockAddrField)
{
    u8 blockAddrFieldHiByte = (u8)(blockAddrField >> 8);
    u8 blockAddrFieldLoByte = (u8)blockAddrField;

    /* User Data Field */
    SmcRedun0 = 0xffffffff;
    /* Block Address Field 1, Data Status Byte and Block Status Byte */
    SmcRedun1 =  (((u32)blockAddrFieldLoByte) << 24) | (((u32)blockAddrFieldHiByte) << 16) | 0xffff;
    /* Block Address Field 2 and Error Correction Code 2 (hardware auto-filled) */
    SmcRedun2 = (((u32)blockAddrFieldHiByte) << 24) | 0xffffff;
    /* Error Correction Code 1 (hardware auto-filled) and Block Address Field 1 */
    SmcRedun3 = 0xffffff00 | ((u32)blockAddrFieldLoByte);
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
s32 smcSetReadDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)&(SmcData);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / 16;
    dmaCfg.burst = 1;	/*CY 0907*/
    //if (dmaConfig(DMA_REQ_SMC_READ, &dmaCfg) == 0)
    //    return 0;
    guiSMCReadDMAId = marsDMAReq(DMA_REQ_SMC_READ, &dmaCfg);
    
    return 1;
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
s32 smcSetWriteDataDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    /* set write data dma */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(SmcData);
    dmaCfg.cnt = siz / 16;
    dmaCfg.burst = 1;	/*CY 0907*/
    //if (dmaConfig(DMA_REQ_SMC_WRITE, &dmaCfg) == 0)
    //    return 0;
    guiSMCWriteDMAId = marsDMAReq(DMA_REQ_SMC_WRITE, &dmaCfg);
    
    return 1;
}

/*

Routine Description:

	Check dma ready.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcCheckDmaReadReady(void)
{
    u8 err;

    err = marsDMACheckReady(guiSMCReadDMAId);  // 1->ok
    guiSMCReadDMAId = 0x55;
    return err;
/*
    OSSemPend(dmaSemChFin[DMA_CH_SMC], SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: dmaSemChFin[DMA_CH_SMC] is %d.\n", err);
        return 0;
    }

    switch (dmaChResp[DMA_CH_SMC])
    {
    case DMA_CH_FINISH:
        return 1;

    case DMA_CH_ERROR:
        DEBUG_SMC("Error: dmaChResp[DMA_CH_SMC] is channel error.\n");
        return -1;

    default:
        DEBUG_SMC("Error: dmaChResp[DMA_CH_SMC] is error.\n");
        return -2;
    }*/
}

s32 smcCheckDmaWriteReady(void)
{
    u8 err;

    err = marsDMACheckReady(guiSMCWriteDMAId);  // 1->ok
    guiSMCWriteDMAId = 0x55;
    return err;
}

/*

Routine Description:

	Check device ready.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcCheckDeviceReady(void)
{
    u32 timeout = 0x0000ffff;

    /* check device ready */
    while (timeout != 0)
    {
        if ((SmcStat & SMC_STAT_READY) == SMC_STAT_READY)
            return 1;

        timeout--;
    }

    DEBUG_SMC("Error: Check device ready is timeout.\n");
    return 0;
}

/*

Routine Description:

	Check operation ready.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcCheckOperationReady(void)
{
    u8 err;
#if 0
    OSSemPend(smcSemEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcSemEvt is %d.\n", err);
        return 0;
    }
#else
    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcMboxEvt is %d in smcCheckOperationReady.\n", err);
        return SMC_OP_STAT_FAIL;
    }
#endif
    return 1;
}

/*

Routine Description:

	Check read ready.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcCheckReadReady(void)
{
    u8 err;

#if 0
    OSSemPend(smcSemEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcSemEvt is %d.\n", err);
        return 0;
    }
#else
    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcMboxEvt is %d in smcCheckOperationReady.\n", err);
        return SMC_OP_STAT_FAIL;
    }
#endif
    /*CY 0907*/
#if 0
    OSSemPend(smcReadWriteSemEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcReadWriteSemEvt is %d.\n", err);
        return 0;
    }
#endif

    if (smcCheckDmaReadReady() != 1)
       {
        DEBUG_SMC("Error: DmaRead .\n");
        return 0;
        }

    return 1;
}

/*

Routine Description:

	Check page2C read ready.
	Only for smcPage2CRead Command.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcCheckRead2CReady(void)
{
    u8 err;

#if 0
    OSSemPend(smcSemEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcSemEvt is %d.\n", err);
        return 0;
    }
#else
    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcMboxEvt is %d in smcCheckOperationReady.\n", err);
        return SMC_OP_STAT_FAIL;
    }
#endif

    return 1;
}
/*

Routine Description:

	Check write ready.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcCheckWriteReady(void)
{
    u8 err;

#if 0
    OSSemPend(smcSemEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcSemEvt is %d.\n", err);
        return SMC_OP_STAT_FAIL;
    }
#else
    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcMboxEvt is %d in smcCheckOperationReady.\n", err);
        return SMC_OP_STAT_FAIL;
    }
#endif
    /*CY 0907*/
#if 0
    OSSemPend(smcReadWriteSemEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcReadWriteSemEvt is %d.\n", err);
        return 0;
    }
#endif

    if (smcCheckDmaWriteReady() != SMC_OP_STAT_PASS)
        return SMC_OP_STAT_FAIL;

    if (smcCheckDeviceReady() != SMC_OP_STAT_PASS)
        return SMC_OP_STAT_FAIL;

    return SMC_OP_STAT_PASS;
}

/*

Routine Description:

	Check erase ready.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcCheckEraseReady(void)
{

    /* wait for Ready/Busy bit trigger */
    if (smcCheckOperationReady() != 1)
        return 0;

    /* polling flash stauts about operation */
    if (smcCheckDeviceReady() != 1)
        return 0;

    return 1;
}

/*
 *********************************************************************************************************
 * Middleware function
 *********************************************************************************************************
 */

/*

Routine Description:

	SMC/NAND gate flash identification.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcIdentification(void)
{
    u8 makerCode, deviceCode;
    u32 stat = 1;

    makerCode = 0;
    deviceCode = 0;

    smcIdRead(&makerCode, &deviceCode);
	DEBUG_SMC("makerCode %#x deviceCode %#x \n",makerCode,deviceCode);
    switch (deviceCode)
    {
    case 0x79:		/* 1 Gb = 128 MB */
        smcTotalSize = 0x08000000;	/* 128 MB */
        smcBlockSize = 0x00004000;	/* 16 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 8;		/* 8 zone */
        smcAddrCycle = 4;		/* 4 address cycle */
        dcfStorageSize = FS_MEDIA_SMC_128MB;	/*CY 0718*/
        DEBUG_SMC("Trace: SMC 128MB.\n");
        break;

    case 0x76:		/* 512 Mb = 64 MB */
        smcTotalSize = 0x04000000;	/* 64 MB */
        smcBlockSize = 0x00004000;	/* 16 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 4;		/* 4 zone */
        smcAddrCycle = 4;		/* 4 address cycle */
        dcfStorageSize = FS_MEDIA_SMC_64MB; /*CY 0718*/
        DEBUG_SMC("Trace: SMC 64MB.\n");
        break;

    case 0x75:		/* 256 Mb = 32 MB */
        smcTotalSize = 0x02000000;	/* 32 MB */
        smcBlockSize = 0x00004000;	/* 16 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 2;		/* 2 zone */
        smcAddrCycle = 3;		/* 3 address cycle */
        dcfStorageSize = FS_MEDIA_SMC_32MB; /*CY 0718*/
        DEBUG_SMC("Trace: SMC 32MB.\n");
        break;

    case 0x73:		/* 128 Mb = 16 MB */
        smcTotalSize = 0x01000000;	/* 16 MB */
        smcBlockSize = 0x00004000;	/* 16 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 1;		/* 1 zone */
        smcAddrCycle = 3;		/* 3 address cycle */
        dcfStorageSize = FS_MEDIA_SMC_16MB; /*CY 0718*/
        DEBUG_SMC("Trace: SMC 16MB.\n");
        break;

    case 0xe6:		/* 64 Mb = 8 MB */
        smcTotalSize = 0x00800000;	/* 8 MB */
        smcBlockSize = 0x00002000;	/* 8 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 1;		/* 1 zone */
        smcAddrCycle = 3;		/* 3 address cycle */
        dcfStorageSize = FS_MEDIA_SMC_8MB;	/*CY 0718*/
        DEBUG_SMC("Trace: SMC 8MB.\n");
        break;

    case 0x6b:		/* 32 Mb = 4 MB */
    case 0xe3:
    case 0xe5:
        smcTotalSize = 0x00400000;	/* 4 MB */
        smcBlockSize = 0x00002000;	/* 8 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 1;		/* 1 zone */
        smcAddrCycle = 3;		/* 3 address cycle */
        dcfStorageSize = FS_MEDIA_SMC_4MB;	/*CY 0718*/
        DEBUG_SMC("Trace: SMC 4MB.\n");
        break;

    default:	/* unknown device code */
        stat = 0;
        DEBUG_SMC("Trace: SMC unknown size.\n");
        DEBUG_SMC("Trace: makerCode = %#x\n", makerCode);
        DEBUG_SMC("Trace: deviceCode = %#x\n", deviceCode);
        break;
    }

    if (stat)
    {
        smcTotalPageCount = smcTotalSize / smcPageSize;
        smcPagePerBlock = smcBlockSize / smcPageSize;
        smcZoneSize = SMC_MAX_ZONE_ENTRY * smcBlockSize;

        smcSecPerBlock = smcBlockSize / FS_SECTOR_SIZE;
        smcSecPerPage = smcPageSize / FS_SECTOR_SIZE;

    }

    return stat;
}

/*

Routine Description:

	Reset.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcReset(void)
{
    u8* pucStat;

    SmcCfg = SMC_CFG_CMD_ONLY | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_DISA; // | SMC_CFG_ADDR_1CYCL
    SmcCmd = SMC_CMD_RESET;
    //SmcAddr = 0;
    SmcCtrl = SMC_CTRL_OP_START;

    if (smcCheckOperationReady() != 1)
        return 0;

    smcReadStatus(pucStat);

    if (*pucStat != 0xC0)
        return 0;

    return 1;
}

/*

Routine Description:

	Read status.

Arguments:

	pStat - The content of status register.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcReadStatus(u8* pStat)
{

    /* cytsai: working or not is unknown */
    SmcCfg = SMC_CFG_CMD_ONLY | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_DISA; // | SMC_CFG_ADDR_1CYCL
    SmcCmd = SMC_CMD_READ_STATUS;
    //SmcAddr = 0;
    SmcCtrl = SMC_CTRL_OP_START;

    if (smcCheckOperationReady() != 1)
        return 0;

    *pStat = SmcStat;

    return 1;
}

/*

Routine Description:

	Read ID command.

Arguments:

	makerCode - The maker code.
	deviceCode - The device code.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcIdRead(u8* makerCode, u8* deviceCode)
{
    u32 stat;

    SmcCfg = SMC_CFG_READ_ID1 | SMC_CFG_ADDR_1CYCL | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_DISA;
    SmcCmd = SMC_CMD_READ_ID;
    SmcAddr = 0;
    SmcCtrl = SMC_CTRL_OP_START;

    if (smcCheckOperationReady() != 1)
        return 0;

    stat = SmcStat;
    *makerCode = (u8) (stat >> 8);
    *deviceCode = (u8) (stat >> 16);

    return 0;
}

/*

Routine Description:

	Read one page starting from A area.

Arguments:

	addr - The page address to read.
	dataBuf - The buffer to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcPage1ARead(u32 addr, u8* dataBuf)
{

    SmcCfg = SMC_CFG_READ 
		     | SMC_CFG_NAND 
		     | SMC_CFG_WP_HI 
		     | SMC_CFG_ECC_ENA 
		     | SMC_CFG_DATA_LEN_512B
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
              |SMC_CFG_RS_CODES_EN 
#endif
				;
    SmcCmd = SMC_CMD_READ_1A;
    smcSetReadWriteAddr(addr);
    DEBUG_SMC("Page Read Addr %#x \n",addr);
    SmcCtrl = SMC_CTRL_OP_START;

    /* set read data dma */
    smcSetReadDataDma(dataBuf, smcPageSize);
        	DEBUG_SMC("E1\n");
    if (smcCheckReadReady() != 1)
        return 0;
	if ((u32)smcMboxMsg == SMC_OP_ECC_ERROR)
    	{
        	DEBUG_SMC("Error Addr = %#x\n", addr);
        	DEBUG_SMC("ECC ERROR\n");
		
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
			smcRsCorrection(dataBuf);
#endif		
        	return SMC_OP_ECC_ERROR;
    	}
    /* get redundant area */
    smcGetRedunArea(&smcRedunArea);

    if ((smcRedunArea.data_status != 0xff) || (smcRedunArea.block_status != 0xff))
        {
            DEBUG_SMC("smcRedunArea ERROR\n");
        return 0;
        }

    return 1;
}


/*

Routine Description:

	Read one page starting from B area.

Arguments:

	addr - The page address to read.
	dataBuf - The buffer to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcPage1BRead(u32 addr, u8* dataBuf)
{
    /* cytsai: working or not is unknown */
    SmcCfg = SMC_CFG_READ | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_ENA | SMC_CFG_DATA_LEN_256B;	/* Lori-test */
    SmcCmd = SMC_CMD_READ_1B;
    smcSetReadWriteAddr(addr);
    SmcCtrl = SMC_CTRL_OP_START;

    /* set read data dma */
    smcSetReadDataDma(dataBuf, smcPageSize / 2);

    if (smcCheckReadReady() != 1)
        return 0;

    /* get redundant area */
    smcGetRedunArea(&smcRedunArea);

    if ((smcRedunArea.data_status != 0xff) || (smcRedunArea.block_status != 0xff))
        return 0;

    return 1;
}

/*

Routine Description:

	Read one page starting from C area.

Arguments:

	addr - The page address to read.
	dataBuf - The buffer to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcPage2CRead(u32 addr)
{
    SmcCfg = SMC_CFG_READ | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_DISA | SMC_CFG_DATA_LEN_16B;
    SmcCmd = SMC_CMD_READ_2C;

    smcSetReadWriteAddr(addr);

    SmcCtrl = SMC_CTRL_OP_START;

    if (smcCheckRead2CReady() != 1)
        return 0;

    /* get redundant area */
    smcGetRedunArea(&smcRedunArea);

    /*
    if ((smcRedunArea.data_status != 0xff) || (smcRedunArea.block_status != 0xff))
    	return 0;
    */
    return 1;

}

/*

Routine Description:

	Write data in Redundant area.

Arguments:

	addr - The page address to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcPage2CWrite(u32 addr)
{

    SmcCfg = 	SMC_CFG_WRITE |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_DATA_LEN_16B;

    SmcCmd= 	(u32) SMC_CMD_PAGE_PROGRAM_1ST |
             ((u32) SMC_CMD_PAGE_PROGRAM_2ND << 8) |
             ((u32) SMC_CMD_READ_STATUS << 16) |
             ((u32) SMC_CMD_WRITE_2C << 24);


    smcSetReadWriteAddr(addr);

    SmcCtrl = 	SMC_CTRL_OP_START |
               SMC_CTRL_REDUN_CMD_TRIG;		/* Special for redundant area writing */

    if (smcCheckRead2CReady() != 1)
        return 0;

    /* get redundant area */
    smcGetRedunArea(&smcRedunArea);

    /*
    if ((smcRedunArea.data_status != 0xff) || (smcRedunArea.block_status != 0xff))
    	return 0;
    */
    return 1;

}



/*

Routine Description:

	Read at any address within a page.

Arguments:

	addr - The page address to read.
	dataBuf - The buffer to read.
	unDataLen - The data length to read. (Length in byte)

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcPageRead(u32 addr, u8* dataBuf, u32 unDataLen)
{
    u32 unDataLenWord;

    unDataLenWord = unDataLen >> 2;		/* change to word length */
    unDataLenWord <<= 8;

    SmcCfg = 	SMC_CFG_CMD1|
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_ENA |
              SMC_CFG_ADV_EN |
              SMC_CFG_WAIT_BUSY_EN |
              unDataLenWord;

    SmcCmd = SMC_CMD_READ_1ST;
    SmcCmd = SMC_CMD_READ_2ND;

    smcSetReadWriteAddr(addr);
    SmcCtrl = SMC_CTRL_OP_START;


    /* set read data dma */
    smcSetReadDataDma(dataBuf, smcPageSize);

    if (smcCheckReadReady() != 1)
        return 0;

    /* get redundant area */
    //smcGetRedunArea(&smcRedunArea);

    /*
    if ((smcRedunArea.data_status != 0xff) || (smcRedunArea.block_status != 0xff))
    	return 0;
    */

    return 1;
}


/*

Routine Description:

	Write one page.

Arguments:

	addr - The page address to read.
	dataBuf - The buffer to read.
	blockAddrField - The Block Address Field of the page to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcPageProgram(u32 addr, u8* dataBuf, u16 blockAddrField)
{

    /* Cause of PA9002D bug, it should perform any command before "PageProgram".
     *  This is the temporarily solution, this bug has not be cleared yet.
     */
//    smcReset();


    SmcCfg = SMC_CFG_PAGE_PROGRAM 
    		| SMC_CFG_NAND 
    		| SMC_CFG_WP_HI 
    		| SMC_CFG_ECC_ENA 
    		| SMC_CFG_DATA_LEN_512B
            | SMC_CFG_RS_CODES_EN ;

    SmcCmd = SMC_CMD_PAGE_PROGRAM_1ST |
             (((u32) SMC_CMD_PAGE_PROGRAM_2ND) << 8) |
             (((u32) SMC_CMD_READ_STATUS) << 16);
    smcSetReadWriteAddr(addr);
    SmcCtrl = SMC_CTRL_OP_START;

    /* set write data dma */
    smcSetWriteDataDma(dataBuf, smcPageSize);

    /* set redundant area */
 //   smcSetRedunArea(blockAddrField);//

    if (smcCheckWriteReady() != 1)
        return 0;

    if (SmcStat & SMC_SR_FAIL)
        return 0;

    /* Cause of PA9002D bug, it should perform any command before "PageProgram".
     *  This is the temporarily solution, this bug has not be cleared yet.
     */
//    smcReset();

    return 1;
}


/*

Routine Description:

	Write the redundant area (C area) of a page.

Arguments:

	addr - The page address to write.
	blockAddrField - The Block Address Field of the page to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcRedunProgram(u32 addr, u16 blockAddrField)
{
    u32 i;
    u8 dataBuf[512];
    u32* dataPtr = (u32*)dataBuf;

    /* initialize fake data to write */
    for (i = 0; i < (smcPageSize / 4); i++)
        *dataPtr++ = 0xffffffff;

    SmcCfg = SMC_CFG_PAGE_PROGRAM | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_ENA | SMC_CFG_DATA_LEN_512B;
    SmcCmd = SMC_CMD_PAGE_PROGRAM_1ST |
             (((u32) SMC_CMD_PAGE_PROGRAM_2ND) << 8) |
             (((u32) SMC_CMD_READ_STATUS) << 16);
    smcSetReadWriteAddr(addr);
    SmcCtrl = SMC_CTRL_OP_START;

    /* set write data dma */
    smcSetWriteDataDma(dataBuf, smcPageSize);

    /* set redundant area */
    smcSetRedunArea(blockAddrField);

    if (smcCheckWriteReady() != 1)
        return 0;

    //if (SmcStat & SMC_SR_FAIL)
    //	return 0;

    return 1;
}


/*

Routine Description:

	Erase one block.

Arguments:

	addr - The block address to erase.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcBlockErase(u32 addr)
{

    SmcCfg = SMC_CFG_BLOCK_ERASE | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_DISA;
    SmcCmd = SMC_CMD_BLOCK_ERASE_1ST |
             (((u32) SMC_CMD_BLOCK_ERASE_2ND) << 8) |
             (((u32) SMC_CMD_READ_STATUS) << 16);


    smcSetEraseAddr(addr);

    SmcCtrl = SMC_CTRL_OP_START;
    if (smcCheckEraseReady() != 1)
        return 0;

    /* check register status */
    if (SmcStat & SMC_SR_FAIL)
    {
        DEBUG_SMC("Block Erase Failed!\n");
        return 0;
    }
    return 1;
}

/*

Routine Description:

	Erase multiple blocks.

Arguments:

	count - The number of blocks to erase.
	addr - The starting block address to erase.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcMultiBlockErase(u32 count, u32 addr)
{
    u32 i;

    for (i = 0; i < (count - 1); i++)
    {
        SmcCfg = SMC_CFG_MULTI_BLOCK_ERASE | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_DISA;
        SmcCmd = SMC_CMD_MULTI_BLOCK_ERASE_1ST;
        smcSetEraseAddr(addr);

        SmcCtrl = SMC_CTRL_OP_START;

        if (smcCheckEraseReady() != 1)
            return 0;

        //if (SmcStat & SMC_SR_FAIL)
        //	return 0;

        addr += smcBlockSize;
    }

    /* last block */
    SmcCfg = SMC_CFG_BLOCK_ERASE | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_DISA;
    SmcCmd = SMC_CMD_BLOCK_ERASE_1ST |
             (((u32) SMC_CMD_BLOCK_ERASE_2ND) << 8) |
             (((u32) SMC_CMD_READ_MULTI_STATUS) << 16);
    smcSetEraseAddr(addr);

    SmcCtrl = SMC_CTRL_OP_START;

    if (smcCheckEraseReady() != 1)
        return 0;

    //if (SmcStat & SMC_SR_FAIL)
    //	return 0;
    return 1;
}

/*

Routine Description:

	Erase total valid blocks.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
void smcTotalBlockErase(char format_flag)
{
    /* cytsai: 0315 */
    u32 addr;
    u32 start_addr=0;
    u16 i=0;
    u32 bad_flag=0;

    smcBBM_init();

    for (addr = SMC_RESERVED_SIZE; addr < smcTotalSize; addr += smcBlockSize)
    {
#if 1
        smcPage2CRead(addr);
        if (smcRedunArea.block_status != 0xff)
        {
            DEBUG_SMC("Bad Block: %x \n",addr);
            bad_flag=1;
            if (i>SMC_BBM.index)
            {
                DEBUG_SMC("Bad Block too much \n"); 	// need err handle or UI error message
                Rerseved_Algo_Start=bad_flag;
            }
            else
            {
                SMC_BBM.Bad_Block_addr[i]=addr;
                i++;
                SMC_BBM.b_index=i;
            }

            continue;
        }
        if (addr>= SMC_MAP_ADDR)
#endif
            smcBlockErase(addr);
    }
    Rerseved_Algo_Start=bad_flag;
    //Rerseved_Algo_Start=0;

//    return bad_flag;
}


/*

Routine Description:

	Bad Block Management init.
	Fill the Reserved Block Address

Arguments:

	None.

Return Value:

	None.

*/
void smcBBM_init(void)
{
    u8 i;
    u32 addr;

    for (i=0;i<SMC_RESERVED_BLOCK;i++)
    {
        SMC_BBM.Bad_Block_addr[i]=0;
        SMC_BBM.New_Block_addr[i]=0;
    }

    SMC_BBM.index=0;
    SMC_BBM.bbm_flag=0;
    SMC_BBM.b_index=0;

    for (addr = SMC_RESERVED_LOGICAL_START; addr < smcTotalSize; addr += smcBlockSize)
    {
        smcPage2CRead(addr);
        if (smcRedunArea.block_status != 0xff)
        {
            DEBUG_SMC("Bad Block in Reserved Area: %x \n",addr);
            continue;
        }
        SMC_BBM.New_Block_addr[SMC_BBM.index]=addr;
        SMC_BBM.index++;
    }

}


/*

Routine Description:

	Check total blocks.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcCheckTotalBlock()
{
    /* cytsai: 0315 */
    u32 addr;
    u32 start_addr=0;
    u16 i=0,j=0;
    u32 bad_flag=0;

    smcBBM_init();

    for (addr = SMC_RESERVED_SIZE; addr < smcTotalSize; addr += smcBlockSize)
    {
        smcPage2CRead(addr);
        if (smcRedunArea.block_status != 0xff)
        {
            DEBUG_SMC("Bad Block: %x \n",addr);
            bad_flag=1;
            if (i>SMC_BBM.index)
            {
                DEBUG_SMC("Bad Block too much \n");     // need err handle or UI error message
                Rerseved_Algo_Start=bad_flag;
            }
            else
            {
                SMC_BBM.Bad_Block_addr[i]=addr;
                i++;
                SMC_BBM.b_index=i;
            }

            continue;
        }
        //smcBlockErase(addr);
    }
    Rerseved_Algo_Start=bad_flag;
    //Rerseved_Algo_Start=0;
    return bad_flag;
}


/*
 *********************************************************************************************************
 * Application function
 *********************************************************************************************************
 */

/*

Routine Description:

	Initialize SMC/NAND gate flash.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcInit(void)
{

    u32 tRP, tRH, tWP, tWH;
    /*CY 0601 S*/
    /* Clock enable of SMC controller */
    SYS_CTL0 |= SYS_CTL0_SMC_CK_EN;

    /* Flash io pin seleted */
#if(CHIP_OPTION == CHIP_PA9002D)
    GpioActFlashSelect = GPIO_ACT_FLASH_SMC;
#else
    GpioActFlashSelect |= GPIO_ACT_FLASH_SMC;
#endif
    /*CY 0601 E*/

    /* Enable interrupt */
    SmcIntMask = SMC_INT_OP_CMPL_ENA| SMC_INT_ECC_ERR_ENA; // | SMC_INT_READ_CMPL_ENA; /*CY 0907*/

    /* Set timing */
    /*CY 0907*/
    /* Set clock divisor to make NAND gate flash read / write pulse greater than 50ns (20MHz). */
    if (SYS_CPU_CLK_FREQ > 60000000)
    {	/* System clock won't be greater than 80MHz. */
        tRP = tRH = tWP = tWH = 1;
    }
    else if (SYS_CPU_CLK_FREQ == 48000000)
    {
        
        tRP = 1;
        tRH = tWP = tWH = 1;
    }
    else if (SYS_CPU_CLK_FREQ == 32000000)
    {
        
        tRP = 3;
        tRH = tWP = tWH = 3;
    }

    else if (SYS_CPU_CLK_FREQ ==  24000000)
    {	/* System clock won't be greater than 80MHz. */
        tRP = 1; // modified by lhchen
        tWP = 1;
        tRH  = tWH = 1;
    }
    else
    {
        tRP = 1; // modified by lhchen
        tWP = 1;
        tRH  = tWH = 1;
    }

    SmcTimeCtrl = tRP | (tRH << 4) | (tWP << 12) | (tWH << 16); /*CY 0907*/

    /* Create the semaphore */
    if (smcSemEvt == NULL) /*CY 1023*/
        smcSemEvt = OSSemCreate(0);
    //smcReadWriteSemEvt = OSSemCreate(0); /*CY 0907*/
    if (smcMboxEvt == NULL)
    {
        smcMboxEvt = OSMboxCreate(smcMboxMsg);
    }
#if SMC_USE_LB_WRITE_CACHE /*CY 0907*/
//	smcCacheClear();
#endif

    if (smcProcProtSemEvt == NULL)
        smcProcProtSemEvt = OSSemCreate(1);


    return 1;
}

/*

Routine Description:

	Mount SMC/NAND gate flash.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcMount(void)
{
    u8 BitMap_return;

    /* SMC/NAND gate flash reset */
    smcReset();

    /* SMC/NAND gate flash identification */
    if (smcIdentification() == 0)
        return 0;

#if 1

    BitMap_return=smcMakeBitMap(1);
    make_BitMap_ok=1;

#endif

    return BitMap_return;
}

/*

Routine Description:

	Unmount SMC/NAND gate flash.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcUnmount(void)
{
    return 1;
}


/*

Routine Description:

	Start smc module.

Arguments:

	None.

Return Value:

	None.

*/
void smcStart(void)
{
    sysSD_Disable();
    sysNAND_Enable();

    smcInit();

    smcReset();

    /* SMC/NAND gate flash identification */
    if (smcIdentification() == 0)
    {
        DEBUG_DCF("Error: SMC/NAND smcIdentification error.\n");
    }

}

/*

Routine Description:

	The IRQ handler of SMC/NAND gate flash.

Arguments:

	None.

Return Value:

	None.

*/
void smcIntHandler(void)
{
    u32 intStat = SmcIntStat;
    u8 unMboxResult = 0;
/* check if ECC comparison error. */
    if (intStat & SMC_INT_ECC_ERR)
    {
    printf("RS_ECC_ERR\n");
        /* ECC comparison error */
        unMboxResult = SMC_OP_ECC_ERROR;
        OSMboxPost(smcMboxEvt, (void*) unMboxResult);
    }
	
    /* check if operation completion is set */
    if (intStat & SMC_INT_OP_CMPL)
    {
        /* Operation cycle is completed */
        unMboxResult = SMC_OP_PASS;
		printf("SMC_OP_PASS\n");
        OSMboxPost(smcMboxEvt, (void*) SMC_OP_PASS);
    }

    /*CY 0907*/
#if 0
    /* check if read /write completion is set */
    if (intStat & SMC_INT_READ_CMPL_ENA)
    {
        /* One page read is completed */
        OSSemPost(smcReadWriteSemEvt);
    }
#endif
}

void smc_temp(void)
{

    smcReadFBSetting();
}

#if INTERNAL_STORAGE_TEST
s32 smcDisEccPageProgram(u32 addr, u8* dataBuf, u16 blockAddrField )
{

    /* Cause of PA9002D bug, it should perform any command before "PageProgram".
     *  This is the temporarily solution, this bug has not be cleared yet.
     */
//    smcReset();


    SmcCfg = SMC_CFG_PAGE_PROGRAM 
    		| SMC_CFG_NAND 
    		| SMC_CFG_WP_HI  
    		| SMC_CFG_DATA_LEN_512B;

    SmcCmd = SMC_CMD_PAGE_PROGRAM_1ST |
             (((u32) SMC_CMD_PAGE_PROGRAM_2ND) << 8) |
             (((u32) SMC_CMD_READ_STATUS) << 16);
    smcSetReadWriteAddr(addr);
    SmcCtrl = SMC_CTRL_OP_START;

    /* set write data dma */
    smcSetWriteDataDma(dataBuf, smcPageSize);

    /* set redundant area */
    //smcSetRedunArea(blockAddrField);

    if (smcCheckWriteReady() != 1)
        return 0;

    if (SmcStat & SMC_SR_FAIL)
        return 0;

    /* Cause of PA9002D bug, it should perform any command before "PageProgram".
     *  This is the temporarily solution, this bug has not be cleared yet.
     */
//    smcReset();

    return 1;
}
/*

Routine Description:

	Verification of Block Erase routine of SMC/NAND gate flash.

Arguments:

	unaddr - the address which block would be erased

Return Value:

	0 - Failed
	1 - Success

*/
s8 smcVerifyBlockErase(u32 unaddr)
{

    u32* readPtr;
    u32	 j;
    u8   RtnSig;
    u32* unErrAddr;


    if (smcBlockErase(unaddr) == 0)
    {
        DEBUG_SMC("Error: Block Erase Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    if (smcPage1ARead(unaddr, smcReadBuf) == 0)
    {
        DEBUG_SMC("Error: Page1A Read Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    readPtr = (u32*) smcReadBuf;
    if (*readPtr != 0xffffffff)
    {
        DEBUG_SMC("\nThis is Pre-read before Block Erase.\n");
        DEBUG_SMC("Error: Block Erase Failed in smcVerifyBlockErase!\n");
        return 0;
    }

    /* write dato to NAND for verification */
    if (smcPageProgram(unaddr, smcWriteBuf, 0x1001) == 0)
    {
        DEBUG_SMC("Error1: Page Program Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    if (smcPage1ARead(unaddr, smcReadBuf) == 0)
    {
        DEBUG_SMC("Error1: Page1A Read Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    readPtr = (u32*) smcReadBuf;
    for (j = 0; j < (SMC_MAX_PAGE_SIZE/4)  ; j++)
    {

        if (*readPtr++ != j)
        {
           	DEBUG_SMC("1Verify Error:%d \n",j);
            break;
        }
    }

	DEBUG_SMC("TEST RS Fountion in VerifyBlockErase!\n");

//	smcRSErrIntrTest(unaddr);
#if 1
	DEBUG_SMC("The original data WriteBuf[100]=%x\n",smcReadBuf[100]);
	smcReadBuf[100] = 0x33;
	DEBUG_SMC("The modifed data is WriteBuf[100]=%x\n",smcReadBuf[100]);
	
	DEBUG_SMC("The original data WriteBuf[101]=%x\n",smcReadBuf[101]);
	smcReadBuf[101] = 0x55;
	DEBUG_SMC("The modifed data is WriteBuf[101]=%x\n",smcReadBuf[101]);
	
	DEBUG_SMC("The original data WriteBuf[123]=%x\n",smcReadBuf[123]);
	smcReadBuf[123] = 0x66;
	DEBUG_SMC("The modifed data is WriteBuf[123]=%x\n",smcReadBuf[123]);
	
	DEBUG_SMC("The original data WriteBuf[511]=%x\n",smcReadBuf[511]);
	smcReadBuf[511] = 0xaa;
	DEBUG_SMC("The modifed data is WriteBuf[511]=%x\n",smcReadBuf[511]);

	unaddr =unaddr + smcPageSize;
 /* write error data to NAND for verification */
    if (smcDisEccPageProgram(unaddr, smcReadBuf, 0x1001) == 0)
    {
        DEBUG_SMC("Error2: Page Program Function Failed in VerifyBlockErase!\n");
        return 0;
    }

 		RtnSig = smcPage1ARead(unaddr, smcReadBuf);
#if 1		
    if (RtnSig == SMC_OP_STAT_FAIL)
    {
        DEBUG_SMC("Error2: Page1A Read Function Failed in VerifyBlockErase!\n");
        return 0;
    }
	else if (RtnSig == SMC_OP_ECC_ERROR)
		{  
				unaddr =unaddr + smcPageSize;
		        DEBUG_SMC("RS Correction data Write back Nand Flash ");
    		if (smcDisEccPageProgram(unaddr, smcReadBuf, 0x1001) == 0)
    		{
        		DEBUG_SMC("Error3: Page Program Function Failed in VerifyBlockErase RS!\n");
        		return 0;
    		}
	    	if (smcPage1ARead(unaddr, smcReadBuf) == 0)
    		{
        		DEBUG_SMC("Error3: Page1A Read Function Failed in VerifyBlockErase RS!\n");
        		return 0;
    		}
		}
    else if (RtnSig == SMC_OP_STAT_PASS)
        {
          		DEBUG_SMC("Error3: VerifyBlockErase RS no happen!\n");
        		//return 0;
        }
    readPtr = (u32*) smcReadBuf;
    for (j = 0; j < (SMC_MAX_PAGE_SIZE/4)  ; j++)
    {
        if (*readPtr != j)
        {
        	DEBUG_SMC("Verify Error:%d \n",j);
            DEBUG_SMC("Verify Error %x=%x\n",*readPtr,j);
            return 0;
        }
        readPtr++;
    }
//    DEBUG_SMC("[Pass]: Block Erase.\n");
#endif
#endif
    return 1;

}
/*

Routine Description:

	SMC/NAND gate flash test routine.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s8	smcTest(void)
{
	u32	i, j;
	u32 *punWriteBuf = (u32*) smcWriteBuf;
	u32 unAddrForTest;

	DEBUG_SMC("\n\n");
    DEBUG_SMC("***************************\n");
	DEBUG_SMC("*                         *\n");
	DEBUG_SMC("*9002_NORMAL NAND Flash Test\n");
	DEBUG_SMC("*                         *\n");
    DEBUG_SMC("***************************\n\n");


	/* re-init smc */
	smcInit();	

    /* SMC/NAND gate flash reset */
    smcReset();

    /* SMC/NAND gate flash identification */
    if (smcIdentification() == 0)
        return 0;

    /* set write data for testing */
    for (i = 0; i < (SMC_MAX_PAGE_SIZE/4); i++)
        *punWriteBuf++ = i;

	/* Use the last block to test */
	unAddrForTest = smcTotalSize - smcBlockSize;
	
	/* Test routine */
	if (smcVerifyBlockErase(unAddrForTest) == 0)
		return 0;

	return 1;

}
#endif	//#if INTERNAL_STORAGE_TEST




//#define SMC_VERIFY 1
//#ifdef SMC_VERIFY
#if 0

/*

Routine Description:

	Redundent area access test.

Arguments:

	unaddr - The block address to test.
	usblockaddrField -

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcRedunAreaAccessTest(u32 unaddr, u16 usblockaddrField)
{
    u32 unData;
    u8 i;
    u32 unCompare;


    for (i=0; i<4; i++)
    {
        switch (i)
        {
        case 0:
            unData = 0xa5a5a5a5;
            break;

        case 1:
            unData = 0x5a5a5a5a;
            break;

        case 2:
            unData = 0xffffffff;
            break;

        case 3:
            unData = 0x00000000;
            break;
        }

        smcSetRedunAreaTest(usblockaddrField, unData);

        smcBlockErase(unaddr);

        if (smcPage2CWrite(unaddr) == 0)
        {
            DEBUG_SMC("Error: Redundant area C write error\n");
            return 0;
        }

#if 0
        SmcRedun0 = 0xa5a5a5a5;
        SmcRedun1 = 0xa5a5a5a5;
        SmcRedun2 = 0xa5a5a5a5;
        SmcRedun3 = 0xa5a5a5a5;

        DEBUG_SMC("1 unaddr = %#x\n", unaddr);
        smcPageProgram(unaddr, smcWriteBuf, 0x1001);

        DEBUG_SMC("addr in reg = %#x\n", SmcAddr);

        /* refresh data in register for redundant area verifications */
        SmcRedun0 = 0x12345678;
        SmcRedun1 = 0xFEDCBA98;
        SmcRedun2 = 0x12345678;
        SmcRedun3 = 0xFEDCBA98;

        DEBUG_SMC("2 unaddr = %#x\n", unaddr);

        smcPage1ARead(unaddr, smcReadBuf);
        DEBUG_SMC("addr in reg = %#x\n", SmcAddr);
#endif

        if (smcPage2CRead(unaddr) == 0)
        {
            DEBUG_SMC("Error: Redundant area C read error\n");
            return 0;
        }

//		DEBUG_SMC("SmcRedun0 = %#x\n", SmcRedun0);
//		DEBUG_SMC("SmcRedun1 = %#x\n", SmcRedun1);
//		DEBUG_SMC("SmcRedun2 = %#x\n", SmcRedun2);
//		DEBUG_SMC("SmcRedun3 = %#x\n", SmcRedun3);

        /* comparison */
        unCompare = SmcRedun0;
        if (unData != unCompare)
        {
            DEBUG_SMC("[Error]: Data compared mismatch in Redundant area 0!\n");
            DEBUG_SMC("SmcRedun0 = %#x\n", SmcRedun0);
            return 0;
        }

        unCompare = SmcRedun1;
        if ( (unCompare &  0x0000FFFF) != (unData &  0x0000FFFF))
        {
            DEBUG_SMC("[Error]: Data compared mismatch in Redundant area 1!\n");
            DEBUG_SMC("SmcRedun1 = %#x\n", SmcRedun1);
            return 0;
        }

        unCompare = SmcRedun2;
        if ( (unCompare &  0x00FFFFFF) != (unData &  0x00FFFFFF))
        {
            DEBUG_SMC("[Error]: Data compared mismatch in Redundant area 2!\n");
            DEBUG_SMC("SmcRedun2 = %#x\n", SmcRedun2);
            return 0;
        }

        unCompare = SmcRedun3;
        if ( (unCompare &  0xFFFFFF00) != (unData &  0xFFFFFF00))
        {
            DEBUG_SMC("[Error]: Data compared mismatch in Redundant area 3!\n");
            DEBUG_SMC("SmcRedun3 = %#x\n", SmcRedun3);
            return 0;
        }
    }

    DEBUG_SMC("[Pass]: Redundant Area Access Test Pass.\n");
    return 1;

}

/*

Routine Description:

	Operation Mode Properties Test of SMC/NAND Controller Related Registers.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 smcRegReadWriteTest(void)
{
    u32	unRegOriData;	//The original data in registers.
    u32	unTestVariable1 = 0x00000001;	//Test Variable 1
    u32	unTestVariable2;	//Test Variable 2
    u8	i, j;
    u32	unRegTestListAddr[21]={
                                  (SmcCtrlBase),				//SmcCfg
                                  (SmcCtrlBase + 0x0004),	//SmcTimeCtrl
                                  (SmcCtrlBase + 0x0008),	//SmcCtrl
                                  (SmcCtrlBase + 0x000c),	//SmcCmd
                                  (SmcCtrlBase + 0x0010),	//SmcAddr
                                  (SmcCtrlBase + 0x0014),	//SmcStat
                                  (SmcCtrlBase + 0x0018),	//SmcData
                                  (SmcCtrlBase + 0x001c),	//SmcIntMask
                                  (SmcCtrlBase + 0x0020),	//SmcIntStat
                                  (SmcCtrlBase + 0x0024),	//SmcRedun0
                                  (SmcCtrlBase + 0x0028),	//SmcRedun1
                                  (SmcCtrlBase + 0x002c),	//SmcRedun2
                                  (SmcCtrlBase + 0x0030),	//SmcRedun3
                                  (SmcCtrlBase + 0x0034),	//SmcEcc1
                                  (SmcCtrlBase + 0x0038),	//SmcEcc2

                                  (DmaCtrlBase + 0x0010),	//DmaCh1SrcAddr
                                  (DmaCtrlBase + 0x0014),	//DmaCh1DstAddr
                                  (DmaCtrlBase + 0x0018),	//DmaCh1CycCnt
                                  (DmaCtrlBase + 0x001C),	//DmaCh1Cmd

                                  (GpioCtrlBase + 0x0020),	//GpioActFlashSelect(MBUS_SEL)

                                  (SysCtrlBase)				//SYS_CTL0

                              };	//Address list of registers which will be tested.

    u8	unRegReadWriteIndicationTable[21]={
                                             0,	//SmcCfg
                                             0,	//SmcTimeCtrl
                                             0,	//SmcCtrl
                                             0,	//SmcCmd
                                             0,	//SmcAddr
                                             1,	//SmcStat
                                             0,	//SmcData
                                             0,	//SmcIntMask
                                             1,	//SmcIntStat
                                             0,	//SmcRedun0
                                             0,	//SmcRedun1
                                             0,	//SmcRedun2
                                             0,	//SmcRedun3
                                             1,	//SmcEcc1
                                             1,	//SmcEcc2

                                             0,	//DmaCh1SrcAddr
                                             0,	//DmaCh1DstAddr
                                             0,	//DmaCh1CycCnt
                                             0,	//DmaCh1Cmd

                                             0,	//GpioActFlashSelect(MBUS_SEL)

                                             0	//SYS_CTL0

                                         };		//Indication of registers operation mode. 0: R/W; 1: Read Only


    for (i=0; i<21; i++)		//Total 21 Registers
    {
        unRegOriData = *((u32*) unRegTestListAddr[i]);
        unTestVariable1 = 0x00000001;


        if ( unRegReadWriteIndicationTable[i] == 0)		//Operation Mode: R/W
        {

            for (j=0; j<32; j++)	//bit by bit test
            {
                *((u32*) unRegTestListAddr[i]) = unTestVariable1;
                unTestVariable2 = *((u32*) unRegTestListAddr[i]);

                if (unTestVariable1 != unTestVariable2)
                {
                    DEBUG_SMC("Default Values Are Not Matched!\n");
                    return 0;
                }

                unTestVariable1 <<= 1;		//bit by bit test

            }
            *((u32*) unRegTestListAddr[i]) = unRegOriData;	//Set the original data back.

        }
        else			//Operation Mode: Read Only
        {
            *((u32*) unRegTestListAddr[i]) = *((u32*) unRegTestListAddr[i]) + 1;		//plus 1 to test read property.

            if (unRegOriData != *((u32*) unRegTestListAddr[i]))		//Read Only Test Fail
            {
                DEBUG_SMC("Error! Read Only Failed!\n");
                return 0;
            }
        }

    }

    return 1;

}


/*

Routine Description:

	Error Correction Code S/W Implementation of SMC/NAND Controller Writing Process.
	This is used to verify H/W ECC results.

Arguments:

	punECCwritebuf - the write data which is used to make Error Correction Code

Return Value:

	0 - Failure.
	1 - Success.

*/
u8 smcECCSWResult(u32* punECCwritebuf)
{

    u32	unECCResult;

    unECCResult = smcECC_SW((u32*) punECCwritebuf);


    if ( (smcRedunArea.ecc1[0] != (u8)(unECCResult & 0x0000FF))||
            (smcRedunArea.ecc1[1] != (u8)((unECCResult>>8) & 0x0000FF))||
            (smcRedunArea.ecc1[2] != (u8)((unECCResult >>16)& 0x0000FF))
       )
    {
        DEBUG_SMC("Error!ECC1 Default Value is error.\n");
        return 0;
    }


    unECCResult = smcECC_SW((u32*) punECCwritebuf +64);

    if ( (smcRedunArea.ecc2[0] != (u8)(unECCResult & 0x0000FF))||
            (smcRedunArea.ecc2[1] != (u8)((unECCResult>>8) & 0x0000FF))||
            (smcRedunArea.ecc2[2] != (u8)((unECCResult >>16)& 0x0000FF))
       )
    {
        DEBUG_SMC("Error!ECC2 Default Value is error.\n");
        return 0;
    }
    smcVrfResult[VrfECC] = 1;

    DEBUG_SMC("[Pass]: ECC SW Test\n");


    /*

    Routine Description:

    	Reset and Registers Deault Value Verification of SMC/NAND gate flash.

    Arguments:

    	None.

    Return Value:

    	0 - Failed
    	1 - Success

    */
    s8 smcVerifyDefultValue(void)
    {
        u8	i;
        u8	test_count;
        //u8	ucRegTestResult[21];		//The result of default value verification after reset operation.
        u32	unReadOutValue;

        u32	unRegTestListAddr[21]={
                                      (SmcCtrlBase),				//SmcCfg
                                      (SmcCtrlBase + 0x0004),	//SmcTimeCtrl
                                      (SmcCtrlBase + 0x0008),	//SmcCtrl
                                      (SmcCtrlBase + 0x000c),	//SmcCmd
                                      (SmcCtrlBase + 0x0010),	//SmcAddr
                                      (SmcCtrlBase + 0x0014),	//SmcStat
                                      (SmcCtrlBase + 0x0018),	//SmcData
                                      (SmcCtrlBase + 0x001c),	//SmcIntMask
                                      (SmcCtrlBase + 0x0020),	//SmcIntStat
                                      (SmcCtrlBase + 0x0024),	//SmcRedun0
                                      (SmcCtrlBase + 0x0028),	//SmcRedun1
                                      (SmcCtrlBase + 0x002c),	//SmcRedun2
                                      (SmcCtrlBase + 0x0030),	//SmcRedun3
                                      (SmcCtrlBase + 0x0034),	//SmcEcc1
                                      (SmcCtrlBase + 0x0038),	//SmcEcc2

                                      (DmaCtrlBase + 0x0010),	//DmaCh1SrcAddr
                                      (DmaCtrlBase + 0x0014),	//DmaCh1DstAddr
                                      (DmaCtrlBase + 0x0018),	//DmaCh1CycCnt
                                      (DmaCtrlBase + 0x001C),	//DmaCh1Cmd

                                      (GpioCtrlBase + 0x0020),	//GpioActFlashSelect(MBUS_SEL)

                                      (SysCtrlBase)				//SYS_CTL0

                                  };	//Address list of registers which will be tested.

        u32	unRegDeaultValue[21]={
                                     0x00000040,		//(SmcCtrlBase),		//SmcCfg
                                     0x00000000,		//(SmcCtrlBase + 0x0004),	//SmcTimeCtrl
                                     0x00000000,		//(SmcCtrlBase + 0x0008),	//SmcCtrl
                                     0x00000000,		//(SmcCtrlBase + 0x000c),	//SmcCmd
                                     0x00000000,		//(SmcCtrlBase + 0x0010),	//SmcAddr
                                     0x00000000,		//(SmcCtrlBase + 0x0014),	//SmcStat
                                     0x00000000,		//(SmcCtrlBase + 0x0018),	//SmcData
                                     0x00000000,		//(SmcCtrlBase + 0x001c),	//SmcIntMask
                                     0x00000000,		//(SmcCtrlBase + 0x0020),	//SmcIntStat
                                     0xFFFFFFFF,		//(SmcCtrlBase + 0x0024),	//SmcRedun0
                                     0xFFFFFFFF,		//(SmcCtrlBase + 0x0028),	//SmcRedun1
                                     0xFFFFFFFF,		//(SmcCtrlBase + 0x002c),	//SmcRedun2
                                     0xFFFFFFFF,		//(SmcCtrlBase + 0x0030),	//SmcRedun3
                                     0x00000000,		//(SmcCtrlBase + 0x0034),	//SmcEcc1
                                     0x00000000,		//(SmcCtrlBase + 0x0038),	//SmcEcc2

                                     0x00000000,		//(DmaCtrlBase + 0x0010),	//DmaCh1SrcAddr
                                     0x00000000,		//(DmaCtrlBase + 0x0014),	//DmaCh1DstAddr
                                     0x00000000,		//(DmaCtrlBase + 0x0018),	//DmaCh1CycCnt
                                     0x00000000,		//(DmaCtrlBase + 0x001C),	//DmaCh1Cmd

                                     0x00000000,		//(GpioCtrlBase + 0x0020),	//GpioActFlashSelect(MBUS_SEL)

                                     0x00000600,		//(SysCtrlBase)			//SYS_CTL0

                                 };	//Address list of registers which will be tested.


        unReadOutValue = SmcCfg;

        SmcCfg =~ unReadOutValue;

        SYS_RSTCTL |= 0x00040000;
        SYS_RSTCTL &= 0x00000000;

        if (SmcCfg != unReadOutValue)
        {
            DEBUG_SMC("Reset and defult value verification pass!\n");
            smcVrfResult[VrfReset] = 1;
            return 1;
        }
        else
        {
            DEBUG_SMC("Reset verification failed!\n");
            return 0;
        }
        /*
        	if (smcReset() == 0)
        		return 0;



        	if(test_count==0)
        	{
        		DEBUG_SMC("Reset and defult value verification pass!\n");
        		return 1;
        	}
        	else
        		return 0;
        */


    }


    /*

    Routine Description:

    	Verification of Block Erase routine of SMC/NAND gate flash.

    Arguments:

    	unaddr - the address which block would be erased

    Return Value:

    	0 - Failed
    	1 - Success

    */
    s8 smcVerifyBlockErase(u32 unaddr)
    {

        u32* readPtr;
        u8	j;
        u32* unErrAddr;


        if (smcBlockErase(unaddr) == 0)
        {
            DEBUG_SMC("Error: Block Erase Function Failed in VerifyBlockErase!\n");
            return 0;
        }

        if (smcPage1ARead(unaddr, smcReadBuf) == 0)
        {
            DEBUG_SMC("Error: Page1A Read Function Failed in VerifyBlockErase!\n");
            return 0;
        }


        readPtr = (u32*) smcReadBuf;
        if (*readPtr != 0xffffffff)
        {
            DEBUG_SMC("\nThis is Pre-read before Block Erase.\n");
            DEBUG_SMC("Error: Block Erase Failed in smcVerifyBlockErase!\n");
            return 0;
        }

        /* write dato to NAND for verification */
        if (smcPageProgram(unaddr, smcWriteBuf, 0x1001) == 0)
        {
            DEBUG_SMC("Error: Page Program Function Failed in VerifyBlockErase!\n");
            return 0;
        }

        if (smcPage1ARead(unaddr, smcReadBuf) == 0)
        {
            DEBUG_SMC("Error: Page1A Read Function Failed in VerifyBlockErase!\n");
            return 0;
        }

        readPtr = (u32*) smcReadBuf;
        for (j = 0; j < (512/4)  ; j++)
        {

            if (*readPtr++ != j)
            {
                break;
            }
        }

        DEBUG_SMC("[Pass]: Block Erase.\n");

        return 1;

    }


    /*

    Routine Description:

    	  For Page2CRead Test.
    	Set the redundant area.

    Arguments:

    	usblockAddrField - The Block Address Field.

    Return Value:

    	None.

    */
    void smcSetRedunAreaTest(u16 usblockAddrField, u32 unData)
    {
        u8 ucblockAddrFieldHiByte = (u8)(usblockAddrField >> 8);
        u8 ucblockAddrFieldLoByte = (u8)usblockAddrField;

        /* User Data Field */
        SmcRedun0 = unData;
        /* Block Address Field 1, Data Status Byte and Block Status Byte */
        SmcRedun1 =  (((u32)ucblockAddrFieldLoByte) << 24) | (((u32)ucblockAddrFieldHiByte) << 16) | (unData & 0x0000ffff);
        /* Block Address Field 2 and Error Correction Code 2 (hardware auto-filled) */
        SmcRedun2 = (((u32)ucblockAddrFieldHiByte) << 24) | (unData & 0x00FFFFFF);
        /* Error Correction Code 1 (hardware auto-filled) and Block Address Field 1 */
        SmcRedun3 = (unData & 0xFFFFFF00) | ((u32)ucblockAddrFieldLoByte);

    }

    /*

    Routine Description:

    	Set data write dma.

    Arguments:

    	buf - The buffer to write from.
    	siz - The size to write.
    	ucBurstIdx - Burst Mode Index			0: No Burst, 1: Burst
    	ucDataWidth - Data Width		0: Byte, 1: Half-Word, 2: Word

    Return Value:

    	0 - Failure.
    	1 - Success.

    */
    s8 smcSetWriteDataDmaTest(u8* buf, u32 siz, s8 cBurstIdx, u8 ucDataWidth)
    {

        DMA_CFG dmaCfg;

        dmaCfg.src = (u32)buf;
        dmaCfg.dst = (u32)&(SmcData);
        dmaCfg.cnt = siz/ (u32) ( (u8) cBurstIdx * (u8) ucDataWidth);
        dmaCfg.burst = (u32) cBurstIdx;
        if (dmaConfigTest(DMA_REQ_SMC_WRITE, &dmaCfg, cBurstIdx, ucDataWidth) == 0)
            return 0;

        return 1;
    }


    /*

    Routine Description:

    	Page Program function for DMA verification

    Arguments:

    	unaddr - the address which block would be erased
    	dataBuf - the pointer of data buf
    	blockAddrField - block address field
    	ucBurstIdx - Burst Mdoe Index			0: No Burst, 1: Burst
    	ucDataWidth - Data Width		0: Byte, 1: Half-Word, 2: Word

    Return Value:

    	0 - Failed
    	1 - Success

    */
    s8 smcPageProgramDMATest(u32 unaddr, u8* dataBuf, u16 blockAddrField, s8 cBurstIdx, u8 ucDataWidth)
    {

        SmcCfg = SMC_CFG_PAGE_PROGRAM | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_ENA | SMC_CFG_DATA_LEN_512B;
        SmcCmd = SMC_CMD_PAGE_PROGRAM_1ST |
                 (((u32) SMC_CMD_PAGE_PROGRAM_2ND) << 8) |
                 (((u32) SMC_CMD_READ_STATUS) << 16);

        smcSetReadWriteAddr(unaddr);
        SmcCtrl = SMC_CTRL_OP_START;

        smcSetWriteDataDmaTest(dataBuf, smcPageSize, cBurstIdx, ucDataWidth);

        /* set redundant area */
        smcSetRedunArea(blockAddrField);

        if (smcCheckWriteReady() != 1)
            return 0;

        if (SmcStat & SMC_SR_FAIL)
            return 0;

        return 1;

    }


    /*

    Routine Description:

    	Read one page starting from A area.
    	For DMA Test.

    Arguments:

    	addr - The page address to read.
    	dataBuf - The buffer to read.
    	ucBurstIdx - Burst Mdoe Index
    	ucDataWidth - Data Width

    Return Value:

    	0 - Failure.
    	1 - Success.

    */
    s32 smcPage1AReadDMATest(u32 addr, u8* dataBuf, s8 cBurstIdx, u8 ucDataWidth)
    {
        SmcCfg = SMC_CFG_READ | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_ENA | SMC_CFG_DATA_LEN_512B;
        SmcCmd = SMC_CMD_READ_1A;
        smcSetReadWriteAddr(addr);
        SmcCtrl = SMC_CTRL_OP_START;

        /* set read data dma */
        smcSetReadDataDMATest(dataBuf, smcPageSize, cBurstIdx, ucDataWidth);

        if (smcCheckReadReady() != 1)
        {
            DEBUG_SMC("Cycle count of dma = [0x%lx] NEQ [0]!\n", DmaCh1CycCnt);
            return 0;
        }

        return 1;
    }

    /*

    Routine Description:

    	Set read data dma.
    	For DMA Test.

    Arguments:

    	buf - The buffer to read to.
    	siz - The size to read.
    	ucBurstIdx - Burst Mode Index
    	ucDataWidth - Data Width

    Return Value:

    	0 - Failure.
    	1 - Success.

    */
    s32 smcSetReadDataDMATest(u8* buf, u32 siz, s8 cBurstIdx, u8 ucDataWidth)
    {
        DMA_CFG dmaCfg;

        /* set read data dma */
        dmaCfg.src = (u32)&(SmcData);
        dmaCfg.dst = (u32)buf;
        dmaCfg.cnt = siz / (u32) ((u8 )cBurstIdx *  ucDataWidth);
        dmaCfg.burst = (u32) cBurstIdx;
        if (dmaConfigTest(DMA_REQ_SMC_READ, &dmaCfg, cBurstIdx, ucDataWidth) == 0)
            return 0;

        return 1;
    }
    /*

    Routine Description:

    	DMA verification on Channel 1

    Arguments:

    	unaddr - the address which block would be erased
    	usblockaddrField - the block address field of the page to write

    Return Value:

    	0 - Failed
    	1 - Success

    */
    s8 smcDMAVerify(u32 unaddr, u16 usblockAddrField)
    {
        u8	j;
        s8	cBurstIdx;	//Burst Mode Index,		0: No Burst, 1: Burst
        u8	ucDataWidth;	//Data Width,		0: Byte, 1: Half-Word, 2: Word
        u32*	readPtr;
        u8*	pucRead;
        s8	cDataWidthIdx;
        u8	uctest_idx = 0;
        u8	ucsmcDMATestFuncIdx = 0;
        u8	i, ucLoopIdx;


        ucDataWidth = 4;

        /*
         *		Test two modes: Burst and No Burst 
         */
        for (ucLoopIdx=0; ucLoopIdx<2; ucLoopIdx++)
        {

            switch (ucLoopIdx)
            {
            case 0:
                cBurstIdx = 4;
                DEBUG_SMC("\n");
                DEBUG_SMC("1. >>>> Burst Mode.\n");
                break;

            case 1:
                cBurstIdx = 1;
                DEBUG_SMC("\n");
                DEBUG_SMC("2. >>>> No Burst Mode.\n");
                break;
            }



            uctest_idx = 0;		//initialize test index;
            if (smcBlockErase(unaddr) == 0)
            {
                DEBUG_SMC("Error: BlockErase Failed in DMAVerify functions.\n");
                return 0;
            }

            memset((void*)smcReadBuf, 'a', 512);
            if (smcPage1AReadDMATest(unaddr, (u8*)smcReadBuf, cBurstIdx, ucDataWidth) == 0)
            {
                uctest_idx ++;
                DEBUG_SMC("[Error]: Page1ARead of [BlockErase] Failed in DMAVerify functions.\n");
                //return 0;
            }

            readPtr  = (u32*) smcReadBuf;
            for (j = 0; j < 128; j++)
                if (*readPtr ++ != 0xffffffff)
                {
                    uctest_idx ++;
                    DEBUG_SMC("[Error]: Data Comparison Failed After Block Erasing.\n");
                    break;
                }

            /* Execute the following Program test if Blcok Erase is success. */
            if (uctest_idx==0)
            {


                if (smcPageProgramDMATest(unaddr, (u8*)smcWriteBuf, usblockAddrField, cBurstIdx, ucDataWidth) == 0)
                {
                    uctest_idx ++;
                    DEBUG_SMC("[Error]: Page Program Failed in DMAVerify functions.\n");
                    //return 0;
                }

                memset((void*)smcReadBuf, 'a', 512);
                if (smcPage1AReadDMATest(unaddr, (u8*)smcReadBuf, cBurstIdx, ucDataWidth) == 0)
                {
                    uctest_idx ++;
                    DEBUG_SMC("[Error]: Page1ARead of [PgeProgram] Failed in DMAVerify functions.\n");
                    //return 0;
                }

                readPtr = (u32*) smcReadBuf;
                for (j = 0; j < 128; j++)
                    if (*readPtr++ != j)
                    {
                        DEBUG_SMC("[Error]: Data Comparison Failed After Page Programming.\n");
                        break;
                    }
            }

        }

        if (uctest_idx==0)
        {
            if (ucLoopIdx == 0) /* Burst Mode */
                DEBUG_SMC("[Pass]: DMA Test in [Burst Mode] and DataWidth [Word]!\n");
            else
                DEBUG_SMC("[Pass]: DMA Test in [No Burst Mode] and DataWidth [Word]!\n");
        }
        else
        {
            ucsmcDMATestFuncIdx ++;	/* Error Index plus 1 */

            if (ucLoopIdx == 0) /* Burst Mode */
                DEBUG_SMC("[Pass]: DMA Test in [Burst Mode] and DataWidth [Word]!\n");
            else
                DEBUG_SMC("[Pass]: DMA Test in [No Burst Mode] and DataWidth [Word]!\n");
        }

        if (ucsmcDMATestFuncIdx !=0)
            return 0;
        else
            return 1;


    }

    /*

    Routine Description:

    	Write the redundant area (C area) of a page.

    Arguments:

    	addr - The page address to write.

    Return Value:

    	0 - Failure.
    	1 - Success.

    */
    s32 smcRedunProgramUpdate(u32 addr)
    {
        u32 i;
        u8 dataBuf[512];
        u32* dataPtr = (u32*)dataBuf;

        /* initialize fake data to write */
        for (i = 0; i < (smcPageSize / 4); i++)
            *dataPtr++ = 0xffffffff;

        SmcCfg = SMC_CFG_PAGE_PROGRAM | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_ENA | SMC_CFG_DATA_LEN_512B;
        SmcCmd = SMC_CMD_PAGE_PROGRAM_1ST |
                 (((u32) SMC_CMD_PAGE_PROGRAM_2ND) << 8) |
                 (((u32) SMC_CMD_READ_STATUS) << 16);
        smcSetReadWriteAddr(addr);
        SmcCtrl = SMC_CTRL_OP_START;


        SmcRedun1 &= 0xffff00ff;

        /* set write data dma */
        smcSetWriteDataDma(dataBuf, smcPageSize);

        /* set redundant area */
        //	smcSetRedunArea(blockAddrField);

        if (smcCheckWriteReady() != 1)
            return 0;

        //if (SmcStat & SMC_SR_FAIL)
        //	return 0;

        return 1;
    }


    /*

    Routine Description:

    	Update address into the specific block.

    Arguments:

    	unAddrToUpdate - address to update.

    Return Value:

    	1 - Successful.
    	0 - Failed.

    */

    s32 smcTestUpdateAddrForTest(u32 unAddrToUpdate)
    {
        u32 i;

        *(u32*)smcWriteBuf = unAddrToUpdate;

        if (smcBlockErase(smcTotalSize-smcBlockSize) == 0)
        {
            DEBUG_SMC("Error: The last block erase error!\n");
            return 0;
        }

        if (smcPageProgram(smcTotalSize - smcBlockSize, smcWriteBuf, 0xffff) ==0)
        {
            DEBUG_SMC("Error: Page Program Error!\n");
            return 0;
        }

        if (smcPage1ARead(smcTotalSize-smcBlockSize, smcReadBuf)==0)
        {
            DEBUG_SMC("Error: Page Read Error\n");
            return 0;
        }

        for (i=0; i<4; i++)
        {
            if (smcWriteBuf[i] != smcReadBuf[i])
            {
                DEBUG_SMC("Error: Data Compare Error!\n");
                return 0;
            }
        }

        return 1;
    }

    /*

    Routine Description:

    	Read out the address which is for test.

    Arguments:

    	unReadAddr - the addr to read out data.

    Return Value:

    	return the read out data.

    */

    u32 smcTestReadAddrForTest(u32 unReadAddr)
    {
        u32 unAddr;

        if (smcPage1ARead(unReadAddr, smcReadBuf) == 0)
        {
            DEBUG_SMC("Error: Page Read Error!\n");
            return 0;
        }

        unAddr = *(u32*)smcReadBuf;

        return unAddr;
    }

    /*

    Routine Description:

    	Write one page.

    Arguments:

    	addr - The page address to read.
    	dataBuf - The buffer to read.
    	blockAddrField - The Block Address Field of the page to write.

    Return Value:

    	0 - Failure.
    	1 - Success.

    */
    s32 smcPageProgramUpdateRedun(u32 addr, u8* dataBuf, u16 blockAddrField)
    {

        SmcCfg = SMC_CFG_PAGE_PROGRAM | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_ENA | SMC_CFG_DATA_LEN_512B;
        SmcCmd = SMC_CMD_PAGE_PROGRAM_1ST |
                 (((u32) SMC_CMD_PAGE_PROGRAM_2ND) << 8) |
                 (((u32) SMC_CMD_READ_STATUS) << 16);
        smcSetReadWriteAddr(addr);
        SmcCtrl = SMC_CTRL_OP_START;

        /* set write data dma */
        smcSetWriteDataDma(dataBuf, smcPageSize);

        /* set redundant area */
//	smcSetRedunArea(blockAddrField);
        SmcRedun1 &= 0xFFFF00FF;	/* set block status not equals to 0xff */

        if (smcCheckWriteReady() != 1)
            return 0;

        if (SmcStat & SMC_SR_FAIL)
            return 0;

        return 1;
    }


    /*

    Routine Description:

    	The whole chip read/ write test routine.

    Arguments:

    	None.

    Return Value:

    	1 - Successful.
    	0 - Failed.

    */
    s32 smcTestEntireChipRWTest(void)
    {
        u32* readPtr;
        u32	i, j, k;
        u32* unErrAddr;
        u32 cnt=0;

        DEBUG_SMC("Erasing...\n");

        for (i=(smcBlockSize*2); i<smcTotalSize; i+=smcBlockSize)
        {
//		DEBUG_SMC("Erase BlockAddr = %#x\n", i);

            if (smcBlockErase(i) == 0)
            {
                DEBUG_SMC("Error: Block Erase Function Failed in smcTestEntireChipRWTest!\n");
                return 0;
            }

            for (j=0; j<smcBlockSize; j+=smcPageSize)
            {

//			DEBUG_SMC("Check erase result page addr = %#x\n", j);

                memset(smcReadBuf, 0, smcPageSize);
                if (smcPage1ARead(i+j, smcReadBuf) == 0)
                {
                    DEBUG_SMC("Error: Page1A Read Function Failed in smcTestEntireChipRWTest!\n");
                    return 0;
                }
                for (k=0; k<smcPageSize; k++)
                    if ((*(smcReadBuf+k)) != 0xff)
                    {
                        DEBUG_SMC("Error: Block erase failed. Data is not 0xff.\n");
                        DEBUG_SMC("BlockAddr = %#x, PageAddr = %#x.\n", i, j);
                        return 0;
                    }
            }

            if ((cnt %10)==0)
                DEBUG_SMC(".");

            if (cnt == 100)
            {
                DEBUG_SMC("\b\b\b\b\b\b\b\b\b\b");
                cnt = 0;
            }
            else
                cnt++;

        }

        DEBUG_SMC("\n\n");


        DEBUG_SMC("Programing...\n");

        for (i=(smcBlockSize*2); i<smcTotalSize; i+=smcBlockSize)
        {
//		DEBUG_SMC("Program BlockAddr = %#x\n", i);

            for (j=0; j<smcBlockSize; j+= smcPageSize)
            {
//			DEBUG_SMC("Check program result page addr = %#x\n", j);

                if (smcPageProgram(i+j, smcWriteBuf, 0x1001) == 0)
                {
                    DEBUG_SMC("Error: Page Program Function Failed in VerifyBlockErase!\n");
                    return 0;
                }
                memset(smcReadBuf, 0, smcPageSize);
                if (smcPage1ARead(i+j, smcReadBuf) == 0)
                {
                    DEBUG_SMC("Error: Page1A Read Function Failed in smcTestEntireChipRWTest!\n");
                    return 0;
                }

                for (k=0; k<smcPageSize; k++)
                {
                    if ((*(smcReadBuf+k)) != (*(smcWriteBuf+k)))
                    {
                        DEBUG_SMC("Error: Read and Write data are not match.\n");
                        DEBUG_SMC("BlockAddr = %#x, PageAddr = %#x.\n", i, j);

                        for (k=0; k<32; k++)
                            DEBUG_SMC("smcReadBuf[%d] = %#x \t\t smcWriteBuf[%d] = %#x .\n", k, *(smcReadBuf+k), k, *(smcWriteBuf+k));

                        return 0;
                    }
                }
            }

            if ((cnt %10)==0)
                DEBUG_SMC(".");

            if (cnt == 100)
            {
                DEBUG_SMC("\b\b\b\b\b\b\b\b\b\b");
                cnt = 0;
            }
            else
                cnt++;

        }

        DEBUG_SMC("\n");

        DEBUG_SMC("Entire chip r/w pass\n");

        return 1;
    }


    /*

    Routine Description:

    	Update the Block status of the whole flash.

    Arguments:

    	None.

    Return Value:

    	None.

    */

    void smcTestUpdateWholeFlashBlockStatus(void)
    {

        u32* readPtr;
        u32 i, j, k;
        u32* unErrAddr;
        u32 cnt=0;
        u8 smcTempBuf[SMC_MAX_PAGE_SIZE];

        DEBUG_SMC("Updating...\n");

        memset(smcTempBuf, 0xff, smcPageSize);

        for (i=0; i<smcTotalSize; i+=smcBlockSize)
        {
//		DEBUG_SMC("Erase BlockAddr = %#x\n", i);
#if 0
            if (smcPage1ARead(i, smcReadBuf)==0)
            {
                smcPageProgramUpdateRedun(i, smcReadBuf, 0x1001);
                DEBUG_SMC("Bad Block Addr = %#x\n", i);
                continue;
            }
#endif
            if (smcBlockErase(i) == 0)
            {
                DEBUG_SMC("Error: Block Erase Function Failed in smcTestEntireChipRWTest!\n");
                continue;
            }

            for (j=0; j<smcBlockSize; j+=smcPageSize)
            {
                memset(smcReadBuf, 0, smcPageSize);
                if (smcPage1ARead(i+j, smcReadBuf) == 0)
                {
                    DEBUG_SMC("Error: Page1A Read Function Failed in smcTestEntireChipRWTest!\n");
                    if (smcPageProgramUpdateRedun(i, smcReadBuf, 0x1001)==0)
                        DEBUG_SMC("Update the block status at the block [%#x].\n", i);
                    break;
                }

                if (memcmp(smcTempBuf, smcReadBuf, smcPageSize)!=0)
                {
                    smcPageProgramUpdateRedun(i, smcReadBuf, 0x1001);
                    DEBUG_SMC("Update the block status at the block [%#x].\n", i);
                    DEBUG_SMC("BlockAddr = %#x, PageAddr = %#x.\n", i, j);
                    break;
                }
            }
#if 0
            if ((cnt %10)==0)
                DEBUG_SMC(".");

            if (cnt == 100)
            {
                DEBUG_SMC("\b\b\b\b\b\b\b\b\b\b");
                cnt = 0;
            }
            else
                cnt++;
#endif
        }

        DEBUG_SMC("\n\n");


        DEBUG_SMC("Read/ Write Test...\n");

        for (i=0; i<smcTotalSize; i+=smcBlockSize)
        {
            DEBUG_SMC("Updating BlockAddr = %#x\n", i);

            if (smcPage1ARead(i, smcReadBuf)==0)
            {
                DEBUG_SMC("Bad Block Addr = %#x\n", i);
                continue;
            }

            for (j=0; j<smcBlockSize; j+= smcPageSize)
            {

                if (smcPageProgram(i+j, smcWriteBuf, 0x1001) == 0)
                {
                    DEBUG_SMC("Error: Page Program Function Failed in VerifyBlockErase!\n");
                    break;
                }
                memset(smcReadBuf, 0, smcPageSize);
                if (smcPage1ARead(i+j, smcReadBuf) == 0)
                {
                    DEBUG_SMC("Error: Page1A Read Function Failed in smcTestEntireChipRWTest!\n");
                    break;
                }

                if (memcmp(smcWriteBuf, smcReadBuf, smcPageSize)!=0)
                {
                    smcPageProgramUpdateRedun(i, smcReadBuf, 0x1001);
                    DEBUG_SMC("Update the block status at the block [%#x].\n", i);
                    DEBUG_SMC("BlockAddr = %#x, PageAddr = %#x.\n", i, j);
                    break;
                }

            }
#if 0
            if ((cnt %10)==0)
                DEBUG_SMC(".");

            if (cnt == 100)
            {
                DEBUG_SMC("\b\b\b\b\b\b\b\b\b\b");
                cnt = 0;
            }
            else
                cnt++;
#endif
        }

        DEBUG_SMC("\n");

        DEBUG_SMC("Entire Flash Block Status Updated Completed.\n");

    }

    /*

    Routine Description:

    	Test the specific address.

    Arguments:

    	unAddr - the address to test.

    Return Value:

    	1 -Successful.
    	0- Failed.

    */
    s32 smcTestSpecificAddress(u32 unAddr)
    {
        u8 ucTmpBuf[SMC_MAX_PAGE_SIZE];

        if (smcPage1ARead(unAddr, smcReadBuf)==0)
        {
            DEBUG_SMC("Bad Block Addr = %#x\n");
            return 0;
        }

        if (smcBlockErase(unAddr) == 0)
        {
            DEBUG_SMC("Error: Block Erase Function Failed in smcTestEntireChipRWTest!\n");
            return 0;
        }

        memset(smcReadBuf, 0, smcPageSize);
        memset(ucTmpBuf, 0xff, smcPageSize);
        if (smcPage1ARead(unAddr, smcReadBuf) == 0)
        {
            DEBUG_SMC("Error: Page1A Read Function Failed in smcTestSpecificAddress!\n");
            return 0;
        }

        if (memcmp(ucTmpBuf, smcReadBuf, smcPageSize)!=0)
        {
            DEBUG_SMC("Erase Failed. Not equals to 0xff.\n");
            return 0;
        }




        if (smcPageProgram(unAddr, smcWriteBuf, 0x1001) == 0)
        {
            DEBUG_SMC("Error: Page Program Function Failed in smcTestSpecificAddress!\n");
            return 0;
        }

        memset(smcReadBuf, 0, smcPageSize);
        if (smcPage1ARead(unAddr, smcReadBuf) == 0)
        {
            DEBUG_SMC("Error: Page1A Read Function Failed in smcTestSpecificAddress!\n");
            return 0;
        }

        if (memcmp(smcWriteBuf, smcReadBuf, smcPageSize)!=0)
        {
            DEBUG_SMC("Cmp error.\n");
            return 0;
        }


        return 1;
    }

    /*

    Routine Description:

    	The chip verification routine of SMC/NAND gate flash.

    Arguments:

    	None.

    Return Value:

    	None.

    */
    s8 smcVerification(void)
    {
        u32 i, j;
        u32 addr;
        u32* writePtr = (u32*) PKBuf0;
        u32* readPtr;
        u32	unaddr;
        u8	uctest_idx_dma=0;
        u32 unAddrForTest;


        u8 ucVerifyIdx = 1;
        u8 ucTestItemSel;
        u8 ucMounted;




        /* set write data */
        for (i = 0; i < (SMC_MAX_PAGE_SIZE * SMC_MULTI_PAGE / 4); i++)
            *writePtr++ = i;


        /* Specific Function Test */

        smcInit();
        smcMount();

        unAddrForTest = smcTestReadAddrForTest(smcTotalSize - smcBlockSize);
        DEBUG_SMC("The Address for test is %#x\n", unAddrForTest);


        while (ucVerifyIdx == 1)
        {
            DEBUG_SMC("\n\n\n");
            DEBUG_SMC("\n*******************************************\n");
            DEBUG_SMC("*\n");
            DEBUG_SMC("* Starting to verify NAND Flash Controller.\n");
            DEBUG_SMC("*\n");
            DEBUG_SMC("*******************************************\n");
            DEBUG_SMC("\n\n");


            DEBUG_SMC("Please Select a item to test! \nKey-in a number to test.\n");
            DEBUG_SMC("[1] - 	ID Detection.\n");
            DEBUG_SMC("[2] -	NAND Flash - Block Erase Test.\n");
            DEBUG_SMC("[3] -	NAND Flash - Redundant Area Access Test.\n");
            DEBUG_SMC("[4] -	NAND Flash - DMA Access Test.\n");
            DEBUG_SMC("[5] -	NAND Flash - ECC SW Test.\n");
            DEBUG_SMC("[6] -	NAND Flash - Full Test\n");
            DEBUG_SMC("[7] -	NAND Flash - Entire chip R/W Test.\n");
            DEBUG_SMC("[8] -	NAND Flash - Update Block Status of the Whole Flash.\n");
            DEBUG_SMC("[9] -	NAND Flash - Update Block Status.\n");
            DEBUG_SMC("[10] -	NAND Flash - Test Specific Address.\n");
            DEBUG_SMC("[99] -	Finish verification.\n");


            scanf("%d", &ucTestItemSel);
            DEBUG_SMC("\n");


            switch (ucTestItemSel)
            {

            case	1:		/* ID Detection */
                if (smcMount() == 0)
                {
                    DEBUG_SMC("ID Detection Error!\n");
                    DEBUG_SMC("Finish Verification!\n");
                    ucVerifyIdx = 0;		/* End Verification */
                }

                ucMounted = 1;	/* NAND Flash is mounted. */

                break;

            case	2:
                DEBUG_SMC("Start to [Block Erase Test]\n");
                smcVerifyBlockErase(unAddrForTest);

                break;

            case	3:
                DEBUG_SMC("Start to [Redundant Area Access Test]\n");
                smcRedunAreaAccessTest(unAddrForTest, 0x1001);

                break;
#if 0
            case	4:
                DEBUG_SMC("Start to [DMA Access Test]\n");
                if (ucMounted != 1)
                    if (smcMount() == 0)
                    {
                        DEBUG_SMC("ID Detection Error!\n");
                        DEBUG_SMC("Finish Verification!\n");
                        ucVerifyIdx = 0;		/* End Verification */
                        break;
                    }
                smcDMAVerify(unaddr, 0x1001);
                ucMounted = 1;	/* NAND Flash is mounted. */

                break;
            case	5:
                DEBUG_SMC("Start to [ECC SW Test]\n");
                if (ucMounted != 1)
                    if (smcMount() == 0)
                    {
                        DEBUG_SMC("ID Detection Error!\n");
                        DEBUG_SMC("Finish Verification!\n");
                        ucVerifyIdx = 0;		/* End Verification */
                        break;
                    }
                smcECCSWResult((u32*)smcWriteBuf);
                ucMounted = 1;	/* NAND Flash is mounted. */

                break;
#endif
#if 0
            case	6:
                DEBUG_SMC("\nStart to [Full Test]\n");

                if (smcMount() == 0)
                {
                    DEBUG_SMC("ID Detection Error!\n");
                    DEBUG_SMC("Finish Verification!\n");
                    ucVerifyIdx = 0;		/* End Verification */
                    break;
                }
                ucMounted = 1;	/* NAND Flash is mounted. */
                DEBUG_SMC("\nStart to [Block Erase Test]\n");
                smcVerifyBlockErase(unaddr);
                DEBUG_SMC("\nStart to [DMA Access Test]\n");
                smcDMAVerify(unaddr, 0x1001);
                DEBUG_SMC("\nStart to [ECC SW Test]\n");
                smcECCSWResult((u32*)smcWriteBuf);

                break;
#endif
            case 7:
                DEBUG_SMC("Entire chip R/W Test\n");
                smcTestEntireChipRWTest();

                break;

            case 8:
                DEBUG_SMC("update the Block Status of the Whole Flash\n");
                smcTestUpdateWholeFlashBlockStatus();

                break;

            case 9:
                DEBUG_SMC("update the Block Status at the specific block\n");

                smcRedunProgramUpdate(0x30000);
                smcRedunProgramUpdate(0x4A000);
                smcRedunProgramUpdate(0x60000);
                smcRedunProgramUpdate(0x70000);

                break;

            case 10:
                DEBUG_SMC("smcTestSpecificAddress\n");
                unaddr = 0x1ce00;
                if (smcTestSpecificAddress(unaddr) ==0)
                    DEBUG_SMC("Error: smcTestSpecificAddress error.\n");
                else
                    DEBUG_SMC("SmcTestSpecificAddress Pass.\n");

                break;

            case	99:
                DEBUG_SMC("Finish Verification!\n");
                ucVerifyIdx = 0;		/* End Verification */
                break;


            default:
                DEBUG_SMC("Error! Wrong item selected!\n");

                break;
            }

            unAddrForTest += smcBlockSize;

        }

        unAddrForTest += smcBlockSize;

        if ((unAddrForTest >= smcTotalSize) ||(unAddrForTest == 0xFFFFFFFF))
            unAddrForTest = smcBlockSize;

        DEBUG_SMC("unAddrTest = %#x\n", unAddrForTest);

        smcTestUpdateAddrForTest(unAddrForTest);

        DEBUG_SMC("End of Verifications.\n");

        return 1;
    }
#endif
#endif
#endif
