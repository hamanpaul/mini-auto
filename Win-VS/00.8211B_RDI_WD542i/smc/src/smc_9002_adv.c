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
#if(FLASH_OPTION == FLASH_NAND_9002_ADV)
#include "board.h"
#include "osapi.h"
#include "smc.h"
#include "smcreg.h"
#include "smcwc.h"
//#include "dmaapi.h"
#include <../inc/mars_controller/mars_dma.h>
#include "fsapi.h"	/* cytsai: 0315 */
#include "rtcapi.h"
#include "dcfapi.h"
#include "SmcECCSW.h"
#include "gpioapi.h"
#include "sysapi.h"


/*
 *********************************************************************************************************
 *												 CONSTANTS
 *********************************************************************************************************
 */

/* SMC time out value */
#define SMC_TIMEOUT				40	/*CY 1023*/


/* SMC Zone */
#define SMC_MAX_ZONE_ENTRY			1024

#define SMC_VERIFY


/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

//__align(4) SMC_REDUN_AREA smcRedunArea;
__align(4) SMC_REDUN_AREA_ADV smcRedunArea;

__align(4) u8  smcReadBuf[SMC_MAX_PAGE_SIZE];
__align(4) u8  smcWriteBuf[SMC_MAX_PAGE_SIZE];
__align(4) u8  smcRedunBuf[SMC_MAX_REDUN_SIZE];

#ifdef SMC_VERIFY
__align(4)	u8	smcTestBuf[SMC_MAX_PAGE_SIZE];
#endif

u8 smcMakeBitMapStat = 0;
u8 make_BitMap_ok = 0;

u32 smcZoneSize = 0;
u32 smcTotalSize, smcBlockSize, smcPageSize, smcPagePerBlock;
u32 smcTotalPageCount; /* cytsai */
u32 smcPageRedunSize;
u32 smcTotalZone;
u32 smcAddrCycle;
u32 smcPagePerBlock;
u8	smcDeviceCode;
u32	smcSecPerBlock;
u32	smcSecPerPage;
u32	smcSectionNum;
u8	ucAccessFlag = 0;	/* flag to indicates the status for other access */

void	*smcMboxMsg;

OS_EVENT* smcSemEvt;		/* semaphore to synchronize smc operation completion event processing */
OS_EVENT* smcQueEvt;		/* massage queue to synchronize smc operation completion event processing */
OS_EVENT* smcMboxEvt;		/* mbox to tramsmitt message */
OS_EVENT* smcProcProtSemEvt;		/* semaphore to synchronize procedure protection */

extern s32 dcfStorageSize;	/*CY 0718*/

NAND_FAT_BPB NAND_FAT_PARAMETER;
NAND_BBM SMC_BBM;
extern u8 Rerseved_Algo_Start;
unsigned char	 smcBitMap[SMC_MAX_MAP_SIZE_IN_BYTE];

u32 guiSMCReadDMAId=0xFF, guiSMCWriteDMAId=0xFF;
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
/* Driver Function */
u8 smcParityCheck(u16);
u16 smcCalcLbaField(u16);
u16 smcCalcLba(u16);
s32 smcSetReadWriteAddr(u32);
s32 smcSetEraseAddr(u32);
void smcGetRedunAreaAdv(SMC_REDUN_AREA_ADV*);
void smcSetRedunAreaAdv(void);
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
s32 smcPageProgram(u32, u8*, u16);
s32 smcRedunProgram(u32, u8*);
s32 smcBlockErase(u32);
void smcTotalBlockErase(char);
void smcBBM_init(void);
u8	smcProcProt(u8, u8);

extern u8 smcMakeBitMap(char);
extern s32 smcMakeTable(void);


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
//s32 smcSetRedunAreaTest(u16);
s8 smcSetWriteDataDmaTest(u8*, u32, s8, u8);
s8 smcPageProgramDMATest(u32, u8*, u16, s8, u8);
s32 smcPage1AReadDMATest(u32, u8*, s8, u8);
s32 smcSetReadDataDMATest(u8*, u32, s8, u8);
s8 smcDMAVerify(u32, u16);
s8 smcVerification(void);


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

//	DEBUG_SMC("SMC ERR: smcSetReadWriteAddr\n");

    if (smcAddrCycle == 5)
    {
        SmcCfg |= SMC_CFG_ADDR_5CYCL_EN;
        SmcAddr = (((addr>>11)<<12) & 0x0ffff000) << 4;
        SmcCmd |= (((addr>>11)<<12) & 0xf0000000) >> 12/* (>>28 <<16) */;
    }

#if (FLASH_OPTION == FLASH_NAND_9002_ADV)
    else if (smcAddrCycle == 4)
    {
        SmcCfg |= SMC_CFG_ADDR_4CYCL;
        SmcAddr = (((addr>>11)<<12) & 0x0ffff000) << 4;
    }

#elif (FLASH_OPTION == FLASH_NAND_9002_NORMAL)
    else if (smcAddrCycle == 4)
    {
        SmcCfg |= SMC_CFG_ADDR_4CYCL;
        SmcAddr = ((addr >> 9) << 8) | (addr & 0x000000ff);
    }
#endif

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

	Set the address of redundant area.

Arguments:

	addr - The page to read/write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcSetReadWriteRedunAddr(u32 addr)
{
    u32	unAddr;

    if (smcAddrCycle == 5)
    {
        SmcCfg |= SMC_CFG_ADDR_5CYCL_EN;
        SmcAddr = (((addr & 0xfffff000) << 1)<< 4) + 0x00000800;	/* find the page address, left shift 4 bits and multiply 2 */
        /* offset the start address to 2048 bytes */
        unAddr = ((addr & 0xfffff000) << 1) >> 28;		/* abstract [31:28] */
        SmcCmd |= (unAddr << 16);	/* set to SmcCmd[23:16] */
    }
    if (smcAddrCycle == 4)
    {
        SmcCfg |= SMC_CFG_ADDR_4CYCL;
        SmcAddr = ((addr & 0x0ffff000)<<5) + 0x00000800;		/* find the page address, left shift 4 bits and multiply 2 */
        /* offset the start address to 2048 bytes */
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

    if (smcAddrCycle == 5)
    {
        SmcCfg |= SMC_CFG_ADDR_3CYCL;
        SmcAddr = addr>>11;			/* (<< 1) >> 12 */
    }
    else if (smcAddrCycle == 4)
    {
        SmcCfg |= SMC_CFG_ADDR_2CYCL;
        SmcAddr= addr >> 11;		/* (<< 1) >> 12 */
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

	None.

*/
void smcGetRedunAreaAdv(SMC_REDUN_AREA_ADV* pRedunArea)
{
    u32	unRedunData;
//    u32	user_data[6];
    u8	ucRedun[4];

    unRedunData = SmcRedun0;
    pRedunArea->block_status = unRedunData & 0xff;
    pRedunArea->data_status = (unRedunData >> 8) & 0xff;
    pRedunArea->block_addr1 = ((unRedunData >>  8) | (unRedunData >> 24)) & 0xffff;

    unRedunData = SmcRedun1;
    pRedunArea->user_data1 = unRedunData;

    pRedunArea->user_data[0] = SmcRedun2;
    pRedunArea->user_data[1] = SmcRedun3;
    pRedunArea->user_data[2] = SmcRedun4;
    pRedunArea->user_data[3] = SmcRedun5;
    pRedunArea->user_data[4] = SmcRedun6;
    pRedunArea->user_data[5] = SmcRedun7;

    *ucRedun = SmcEcc2;
    pRedunArea->ecc2[0] = ucRedun[0];
    pRedunArea->ecc2[1] = ucRedun[1];
    pRedunArea->ecc2[2] = ucRedun[2];
    pRedunArea->extra_data0 = ucRedun[3];

    *ucRedun  = SmcEcc1;
    pRedunArea->ecc1[0] = ucRedun[0];
    pRedunArea->ecc1[1] = ucRedun[1];
    pRedunArea->ecc1[2] = ucRedun[2];
    pRedunArea->extra_data1 = ucRedun[3];

    *ucRedun = SmcEcc4;
    pRedunArea->ecc4[0] = ucRedun[0];
    pRedunArea->ecc4[1] = ucRedun[1];
    pRedunArea->ecc4[2] = ucRedun[2];
    pRedunArea->extra_data2 = ucRedun[3];

    *ucRedun = SmcEcc3;
    pRedunArea->ecc3[0] = ucRedun[0];
    pRedunArea->ecc3[1] = ucRedun[1];
    pRedunArea->ecc3[2] = ucRedun[2];
    pRedunArea->extra_data3= ucRedun[3];

    *ucRedun = SmcEcc6;
    pRedunArea->ecc6[0] = ucRedun[0];
    pRedunArea->ecc6[1] = ucRedun[1];
    pRedunArea->ecc6[2] = ucRedun[2];
    pRedunArea->extra_data4= ucRedun[3];

    *ucRedun = SmcEcc5;
    pRedunArea->ecc5[0] = ucRedun[0];
    pRedunArea->ecc5[1] = ucRedun[1];
    pRedunArea->ecc5[2] = ucRedun[2];
    pRedunArea->extra_data5= ucRedun[3];

    *ucRedun = SmcEcc8;
    pRedunArea->ecc8[0] = ucRedun[0];
    pRedunArea->ecc8[1] = ucRedun[1];
    pRedunArea->ecc8[2] = ucRedun[2];
    pRedunArea->extra_data6 = ucRedun[3];

    *ucRedun = SmcEcc7;
    pRedunArea->ecc7[0] = ucRedun[0];
    pRedunArea->ecc7[1] = ucRedun[1];
    pRedunArea->ecc7[2] = ucRedun[2];
    pRedunArea->extra_data7 = ucRedun[3];


}


/*

Routine Description:

	Set the redundant area.

Arguments:

	None.

Return Value:

	None.

*/
void smcSetRedunAreaAdv(void)
{
#if 0
    SmcRedun0 = 0xffffffff;
    SmcRedun1 = 0xffffffff;
    SmcRedun2 = 0xffffffff;
    SmcRedun3 = 0xffffffff;
    SmcRedun4 = 0xffffffff;
    SmcRedun5 = 0xffffffff;
    SmcRedun6 = 0xffffffff;
    SmcRedun7 = 0xffffffff;
    SmcRedun8 = 0xffffffff;
    SmcRedun9 = 0xffffffff;
    SmcRedun10 = 0xffffffff;
    SmcRedun11 = 0xffffffff;
    SmcRedun12 = 0xffffffff;
    SmcRedun13 = 0xffffffff;
    SmcRedun14 = 0xffffffff;
    SmcRedun15 = 0xffffffff;
#else
    SmcRedun0 = 0xffffffff;
    SmcRedun1 = 0xffffffff;
    SmcRedun2 = 0xffffffff;
    SmcRedun3 = 0xffffffff;
    SmcRedun4 = 0xffffffff;
    SmcRedun5 = 0xffffffff;
    SmcRedun6 = 0xffffffff;
    SmcRedun7 = 0xffffffff;
    SmcRedun8 = 0xffffffff;
    SmcRedun9 = 0xffffffff;
    SmcRedun10 = 0xffffffff;
    SmcRedun11 = 0xffffffff;
    SmcRedun12 = 0xffffffff;
    SmcRedun13 = 0xffffffff;
    SmcRedun14 = 0xffffffff;
    SmcRedun15 = 0xffffffff;
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
#if 0
    if (dmaSemChFin[1] == NULL)
    {
        dmaSemChFin[1] = OSSemCreate(0);
        DEBUG_SMC("sem created\n");
    }
    dmaSemChFin[1] = OSSemDel(dmaSemChFin[1], OS_DEL_ALWAYS, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: dmaSemChFin[DMA_CH_SMC] is %d.\n", err);
    }

    dmaSemChFin[1] = OSSemCreate(0);
#endif

    OSSemPend(dmaSemChFin[DMA_CH_SMC], SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: dmaSemChFin[DMA_CH_SMC] is %d.\n", err);
        return SMC_OP_STAT_FAIL;
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
            return SMC_OP_STAT_PASS;

        timeout--;
    }

    DEBUG_SMC("Error: Check device ready is timeout.\n");
    return SMC_OP_STAT_FAIL;
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
    return SMC_OP_STAT_PASS;
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

    if ((SmcCfg&0x00000007) != 0)
    {
        DEBUG_SMC("SmcCfg is %#x\n", SmcCfg);
        DEBUG_SMC("Before smcCheckReadReady\n");
    }

    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("SMC ERR: DEBUG TEST.\n");
        DEBUG_SMC("Error: smcMboxEvt is %d in smcCheckReadReady.\n", err);

//        DEBUG_SMC("Error: DmaCh1SrcAddr is %#x in smcCheckReadReady.\n", DmaCh1SrcAddr);
//        DEBUG_SMC("Error: DmaCh1DstAddr is %#x in smcCheckReadReady.\n", DmaCh1DstAddr);
//        DEBUG_SMC("Error: DmaCh1CycCnt is %#x in smcCheckReadReady.\n", DmaCh1CycCnt);
//        DEBUG_SMC("Error: DmaCh1Cmd is %#x in smcCheckReadReady.\n", DmaCh1Cmd);
//        DEBUG_SMC("Error: 0xc0130090 is %#x in smcCheckReadReady.\n", *(u32*) 0xc0130090);
        DEBUG_SMC("Error: SmcCfg is %#x in smcCheckReadReady.\n", SmcCfg);

        return SMC_OP_STAT_FAIL;
    }

    if (smcCheckDmaReadReady() != SMC_OP_STAT_PASS)
        return SMC_OP_STAT_FAIL;

    return SMC_OP_STAT_PASS;
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
        DEBUG_SMC("Error: smcMboxEvt is %d in smcCheckRead2CReady.\n", err);
        return 0;
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
        return 0;
    }
#else
    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcMboxEvt is %d in smcCheckWriteReady.\n", err);
        return 0;
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

    if (smcCheckDmaWriteReady() != 1)
        return 0;

    if (smcCheckDeviceReady() != 1)
        return 0;

    return 1;
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

    if (smcCheckOperationReady() != 1)
        return 0;

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

    smcIdRead(&makerCode, &deviceCode);

 	DEBUG_SMC("makerCode %#x deviceCode %#x \n",makerCode,deviceCode);
    if (makerCode == 0xEC)		/* Samsung */
    {
        switch (deviceCode)
        {
        case 0xD3:		/* 8 Gb = 1 GB, Advanced NAND */
            smcTotalSize = 0x40000000;	/* 1 GB */
            smcBlockSize = 0x00040000;	/* 256 KB */
            smcPageSize  = 0x00001000;	/* 4 KB */
            smcPageRedunSize = 0x00000080;	/* 64 B */
            smcTotalZone = 8;		/* 8 zone */
            smcAddrCycle = 5;		/* 5 address cycle */
            dcfStorageSize = FS_MEDIA_SMC_512MB; //??
            DEBUG_SMC("Trace: Samsung [Advanced] - 1 GB.\n");
            break;

        case 0xDC:		/* 4 Gb = 512 MB, Advanced NAND */
            smcTotalSize = 0x20000000;	/* 512 MB */
            smcBlockSize = 0x00020000;	/* 128 KB */
            smcPageSize  = 0x00000800;	/* 2 KB */
            smcPageRedunSize = 0x00000040;	/* 64 B */
            smcTotalZone = 8;		/* 8 zone */
            smcAddrCycle = 5;		/* 5 address cycle */
            dcfStorageSize = FS_MEDIA_SMC_512MB;
            DEBUG_SMC("Trace: Samsung [Advanced] - 512MB.\n");
            break;

        case 0xDA:		/* 2 Gb = 256 MB, Advanced NAND */
            smcTotalSize = 0x10000000;	/* 256 MB */
            smcBlockSize = 0x00020000;	/* 128 KB */
            smcPageSize  = 0x00000800;	/* 2 KB */
            smcPageRedunSize = 0x00000040;	/* 64 B */
            smcTotalZone = 8;		/* 8 zone */
            smcAddrCycle = 5;		/* 5 address cycle */
            dcfStorageSize = FS_MEDIA_SMC_256MB;
            DEBUG_SMC("Trace: Samsung [Advanced] - 256MB.\n");
            break;

        case 0xF1:		/* 1 Gb = 128 MB, Advanced NAND */
            smcTotalSize = 0x08000000;	/* 128 MB */
            smcBlockSize = 0x00020000;	/* 128 KB */
            smcPageSize  = 0x00000800;	/* 2 KB */
            smcPageRedunSize = 0x00000040;	/* 64 B */
            smcTotalZone = 8;		/* 8 zone */	/* Maximum 1024 blocks in one zone */
            smcAddrCycle = 4;		/* 4 address cycle */
            dcfStorageSize = FS_MEDIA_SMC_128MB;
            DEBUG_SMC("Trace: Samsung [Advanced] - 128MB.\n");
            break;

        case 0x79:		/* 1 Gb = 128 MB, Normal NAND */
            smcTotalSize = 0x08000000;	/* 128 MB */
            smcBlockSize = 0x00004000;	/* 16 KB */
            smcPageSize  = 0x00000200;	/* 512 B */
            smcPageRedunSize = 0x00000010;	/* 16 B */
            smcTotalZone = 8;		/* 8 zone */	/* Maximum 1024 blocks in one zone */
            smcAddrCycle = 4;		/* 4 address cycle */
            dcfStorageSize = FS_MEDIA_SMC_128MB;
            DEBUG_SMC("Trace: Samsung [Normal] - 128MB.\n");
            break;

        case 0x76:		/* 512 Mb = 64 MB */
            smcTotalSize = 0x04000000;	/* 64 MB */
            smcBlockSize = 0x00004000;	/* 16 KB */
            smcPageSize  = 0x00000200;	/* 512 B */
            smcPageRedunSize = 0x00000010;	/* 16 B */
            smcTotalZone = 4;		/* 4 zone */
            smcAddrCycle = 4;		/* 4 address cycle */
            dcfStorageSize = FS_MEDIA_SMC_64MB;
            DEBUG_SMC("Trace: Samsung - 64MB.\n");
            break;

        default:		/* unknown device code */
            stat = 0;
            DEBUG_SMC("Trace: SMC unknown size.\n");
            DEBUG_SMC("Trace: makerCode = %#x\n", makerCode);
            DEBUG_SMC("Trace: deviceCode = %#x\n", deviceCode);
            DEBUG_SMC("Trace: SMC unknown size.\n");
            break;
        }

    }
    else if (makerCode == 0xAD)		/* Hynix */
    {
        switch (deviceCode)
        {

        case 0xDC:		/* 4 Gb = 512 MB, Advanced NAND */
            smcTotalSize = 0x20000000;	/* 512 MB */
            smcBlockSize = 0x00020000;	/* 128 KB */
            smcPageSize  = 0x00000800;	/* 2 KB */
            smcPageRedunSize = 0x00000040;	/* 64 B */
            smcTotalZone = 8;		/* 8 zone */
            smcAddrCycle = 5;		/* 5 address cycle */
            dcfStorageSize = FS_MEDIA_SMC_512MB;
            DEBUG_SMC("Trace: Hynix [Advanced] - 512MB.\n");
            break;

        case 0xDA:		/* 2 Gb = 256 MB, Advanced NAND */
            smcTotalSize = 0x10000000;	/* 256 MB */
            smcBlockSize = 0x00020000;	/* 128 KB */
            smcPageSize  = 0x00000800;	/* 2 KB */
            smcPageRedunSize = 0x00000040;	/* 64 B */
            smcTotalZone = 8;		/* 8 zone */
            smcAddrCycle = 5;		/* 5 address cycle */
            dcfStorageSize = FS_MEDIA_SMC_256MB;
            DEBUG_SMC("Trace: Hynix [Advanced] - 256MB.\n");
            break;

        case 0xF1:		/* 1 Gb = 128 MB, Advanced NAND */
            smcTotalSize = 0x08000000;	/* 128 MB */
            smcBlockSize = 0x00020000;	/* 128 KB */
            smcPageSize  = 0x00000800;	/* 2 KB */
            smcPageRedunSize = 0x00000040;	/* 64 B */
            smcTotalZone = 8;		/* 8 zone */	/* Maximum 1024 blocks in one zone */
            smcAddrCycle = 4;		/* 4 address cycle */
            dcfStorageSize = FS_MEDIA_SMC_128MB;
            DEBUG_SMC("Trace: Hynix [Advanced] - 128MB.\n");
            break;

        default:		/* unknown device code */
            stat = 0;
            DEBUG_SMC("Trace: SMC unknown size.\n");
            DEBUG_SMC("Trace: makerCode = %#x\n", makerCode);
            DEBUG_SMC("Trace: deviceCode = %#x\n", deviceCode);
            DEBUG_SMC("Trace: SMC unknown size.\n");
            break;
        }

    }
    else
    {
        stat = 0;
        DEBUG_SMC("Trace: SMC unknown size.\n");
        DEBUG_SMC("Trace: makerCode = %#x\n", makerCode);
        DEBUG_SMC("Trace: deviceCode = %#x\n", deviceCode);
        DEBUG_SMC("Trace: SMC unknown size.\n");
    }

    if (stat)
    {
        smcTotalPageCount = smcTotalSize / smcPageSize;
        smcPagePerBlock = smcBlockSize / smcPageSize;
        smcZoneSize = SMC_MAX_ZONE_ENTRY * smcBlockSize;

        smcSecPerBlock = smcBlockSize / FS_SECTOR_SIZE;
        smcSecPerPage = smcPageSize / FS_SECTOR_SIZE;
        smcSectionNum = smcPagePerBlock / 32;	/* 32 pages indicates one section, one word represents 32 pages maximum */
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

    SmcCfg = 	SMC_CFG_CMD_ONLY |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_ADV_EN;

    SmcCmd = 	SMC_CMD_RESET;
    SmcCtrl = 	SMC_CTRL_OP_START;


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

    SmcCfg = 	SMC_CFG_CMD_STATUS |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_ADV_EN;

    SmcCmd = 	SMC_CMD_READ_STATUS;

    SmcCtrl = 	SMC_CTRL_OP_START;

    if (smcCheckOperationReady() != SMC_OP_STAT_PASS)
        return SMC_OP_STAT_FAIL;

    *pStat = SmcStat;

    return 1;
}

/*

Routine Description:

	Read EDC status.

Arguments:

	pucEdcStat - The content of status register.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcReadEdcStatus(u8* pucEdcStat)
{

    SmcCfg = 	SMC_CFG_CMD_STATUS |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_ADV_EN;

    SmcCmd = 	SMC_CMD_READ_EDC_STATUS;

    SmcCtrl = 	SMC_CTRL_OP_START;

    if (smcCheckOperationReady() != SMC_OP_STAT_PASS)
        return SMC_OP_STAT_FAIL;

    *pucEdcStat = SmcStat;

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

    SmcCfg = 	SMC_CFG_READ_ID1 |
              SMC_CFG_ADDR_1CYCL |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_ADV_EN;

    SmcCmd = 	SMC_CMD_READ_ID;
    SmcAddr = 	0;
    SmcCtrl = 	SMC_CTRL_OP_START;


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
	2 - ECC error.

*/
s32 smcPage1ARead(u32 addr, u8* dataBuf)
{
    u8 err;

    SmcCfg =   SMC_CFG_CMD1
              |SMC_CFG_NAND 
              |SMC_CFG_WP_HI 
              |SMC_CFG_ECC_ENA 
              |SMC_CFG_ADV_EN 
              |SMC_CFG_WAIT_BUSY_EN 
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
              |PAGE_MODE_2k 
              |SMC_CFG_RS_CODES_EN 
#endif
              ;
	if(smcPageSize == 0x00001000)
		SmcCfg |= PAGE_MODE_4k;

    SmcCmd = 	SMC_CMD_READ_1ST |
              ((u32) SMC_CMD_READ_2ND << 8);

	DEBUG_SMC("Page Read Addr %#x \n",addr);
    smcSetReadWriteAddr(addr);
	
    SmcCtrl = SMC_CTRL_OP_START;

    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcMboxEvt is %d in smcPage1ARead.\n", err);
        return 0;
    }


    SmcCfg = 	SMC_CFG_READ |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_ENA |
              SMC_CFG_ADV_EN |
              SMC_CFG_WAIT_BUSY_EN 
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
              |PAGE_MODE_2k 
              |SMC_CFG_RS_CODES_EN 
#endif
              ;

	if(smcPageSize == 0x00001000)
		SmcCfg |= PAGE_MODE_4k|SMC_CFG_DATA_LEN_4096B;
	else
		SmcCfg |= PAGE_MODE_2k|SMC_CFG_DATA_LEN_2048B;
	
    /* set read data dma */
    smcSetReadDataDma(dataBuf, smcPageSize);

    SmcCtrl = SMC_CTRL_OP_START;

    if (smcCheckReadReady() != 1)
        return 0;

	 DEBUG_SMC("smcMboxMsg %x\n",smcMboxMsg);
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
    smcGetRedunAreaAdv(&smcRedunArea);

    /* check the block status and data status */
    if ((smcRedunArea.data_status != 0xff) || (smcRedunArea.block_status != 0xff))
    {
        DEBUG_SMC("[Error]: Block Status or Data Status Error!\n");
        DEBUG_SMC("smcRedunArea.block_status = %#x\n", smcRedunArea.block_status);
        DEBUG_SMC("smcRedunArea.data_status = %#x\n", smcRedunArea.data_status);
        return 0;
    }

    return 1;
}

/*

Routine Description:

	Reset and init Ecc status .

Arguments:

	None.

Return Value:

	None.

*/
s32 smcResetEccStat(void)
{

    SmcCfg = SMC_CFG_RST_ECC_STAT;
    SmcCfg &= ~SMC_CFG_RST_ECC_STAT;

    return 1;
}


/*

Routine Description:

	Read redundant area data.

Arguments:

	unSrcPageAddr - The page address to read.
	pucReadBuf - The destination buffer to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcRedunAreaRead(u32 unSrcPageAddr, u8* pucReadBuf)
{
    u8 err;


    SmcCfg = 	SMC_CFG_CMD1|
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_ADV_EN |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_DATA_LEN_64B;

    SmcCmd = 	SMC_CMD_READ_1ST |
              ((u32) SMC_CMD_READ_2ND << 8);

    smcSetReadWriteRedunAddr(unSrcPageAddr);
    SmcCtrl = SMC_CTRL_OP_START;

    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcMboxEvt is %d in smcRedunAreaRead.\n", err);
        return 0;
    }

    SmcCfg = 	SMC_CFG_READ |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_ADV_EN |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_DATA_LEN_64B;

    /* set read data dma */
    smcSetReadDataDma(pucReadBuf, smcPageRedunSize);

    SmcCtrl = SMC_CTRL_OP_START;

    if (smcCheckReadReady() != 1)
        return 0;

	
#if 0
    /* read process doesn't need to check status, only used for program and erase */
    /*
    	if (SmcStat & SMC_SR_FAIL)
    		return 0;
    */
#endif

    return 1;

}

/*

Routine Description:

	Write one page.

Arguments:

	addr - The page address to read.
	dataBuf - The buffer to read.
	blockAddrField - Dummy for compatible with PA9001 and PA9002 Normal.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcPageProgram(u32 addr, u8* dataBuf, u16 blockAddrField )
{
    u8 err;
    u8 Stat;

    SmcCfg = 	SMC_CFG_CMD2 |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_ADV_EN |            
              SMC_CFG_RS_CODES_EN |
              PAGE_MODE_2k|
              SMC_CFG_ECC_ENA;  
	
	if(smcPageSize == 0x00001000)
		SmcCfg |= PAGE_MODE_4k;
	


    DEBUG_SMC("Page Program Addr %#x \n",addr);
    SmcCmd = SMC_CMD_PAGE_PROGRAM_1ST;

    smcSetReadWriteAddr(addr);
    SmcCtrl = SMC_CTRL_OP_START;

    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcMboxEvt is %d in smcPageProgram.\n", err);
        return SMC_OP_STAT_FAIL;
    }

    /* set redundant area */
    //smcSetRedunAreaAdv();

    SmcCfg = 	SMC_CFG_WRITE|
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_ADV_EN | 
              SMC_CFG_RS_CODES_EN |
              SMC_CFG_ECC_ENA; 
	
	if(smcPageSize == 0x00001000)
		SmcCfg |= PAGE_MODE_4k|SMC_CFG_DATA_LEN_4096B;
	else
		SmcCfg |= PAGE_MODE_2k|SMC_CFG_DATA_LEN_2048B;

    SmcCmd = 	SMC_CMD_PAGE_PROGRAM_2ND |
              ((u32) SMC_CMD_READ_STATUS << 8);

    SmcCtrl = SMC_CTRL_OP_START;

    /* set write data dma */
    smcSetWriteDataDma(dataBuf, smcPageSize);

    if (smcCheckWriteReady() != SMC_OP_STAT_PASS)
        return SMC_OP_STAT_FAIL;
#if 0
    {
        u32* 	punAddr;
        u32*	punTemp;
        u32 i;
        punAddr = ((u32*)0xd0080024);
        punTemp = punAddr;
        for (i=0; i<23; i++)
        {
            DEBUG_SMC("punTemp inside %#x = %#x\n", punTemp, *(u32*)punTemp);
            punTemp++;
        }
    }
#endif
    /* Return Register Status for NAND Writing Ability */
    if (smcReadStatus(&Stat) == SMC_OP_STAT_FAIL)
        return SMC_OP_STAT_FAIL;

    if (Stat & SMC_SR_FAIL)
        return SMC_OP_STAT_SR_FAIL;		/* Bad page, program fail */
    else
        return SMC_OP_STAT_SR_PASS;

}


/*

Routine Description:

	Write Redundant Area Data (Total 64 Bytes).

Arguments:

	unDestPageAddr - The page address to write.
	pucWriteBuf - The source buffer to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcRedunAreaProgram(u32 unDestPageAddr, u8* pucWriteBuf)
{
    u8 err;

    SmcCfg = 	SMC_CFG_CMD2 |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_ADV_EN |
              SMC_CFG_DATA_LEN_64B;

    SmcCmd = SMC_CMD_PAGE_PROGRAM_1ST;

    smcSetReadWriteRedunAddr(unDestPageAddr);
    SmcCtrl = SMC_CTRL_OP_START;

    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcMboxEvt is %d in smcRedunAreaProgram.\n", err);
        return 0;
    }

    SmcCfg = 	SMC_CFG_WRITE|
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_ADV_EN |
              SMC_CFG_DATA_LEN_64B;

    SmcCmd = 	SMC_CMD_PAGE_PROGRAM_2ND |
              ((u32) SMC_CMD_READ_STATUS << 8);

    SmcCtrl = SMC_CTRL_OP_START;

    /* set write data dma */
    smcSetWriteDataDma(pucWriteBuf, smcPageRedunSize);

    if (smcCheckWriteReady() != 1)
        return 0;
#if 0
    /* read process doesn't need to check status, only used for copy-back */
    if (SmcStat & SMC_SR_FAIL)
        return 0;
#endif

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
    u8 Stat;

    SmcCfg = 	SMC_CFG_BLOCK_ERASE |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_ADV_EN |
              SMC_CFG_WAIT_BUSY_EN;

    SmcCmd = SMC_CMD_BLOCK_ERASE_1ST |
             (((u32) SMC_CMD_BLOCK_ERASE_2ND) << 8);


    smcSetEraseAddr(addr);

    SmcCtrl = SMC_CTRL_OP_START;
    if (smcCheckEraseReady() != 1)
        return SMC_OP_STAT_FAIL;

#if 1
    /* Return Register Status for NAND Writing Ability */
    if (smcReadStatus(&Stat) == SMC_OP_STAT_FAIL)
        return SMC_OP_STAT_FAIL;

    if (Stat & SMC_SR_FAIL)
        return SMC_OP_STAT_SR_FAIL;		/* Bad page, program fail */
    else
        return SMC_OP_STAT_SR_PASS;
#else
    return 1;
#endif

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
    u16 i=0;
    u32 bad_flag=0;

    smcBBM_init();

    for (addr = SMC_RESERVED_SIZE; addr < smcTotalSize; addr += smcBlockSize)
    {
        smcRedunAreaRead(addr, smcReadBuf);
        if (smcReadBuf[0] != 0xff)
        {
            DEBUG_SMC("SMC TRACE1: Bad Block: %x \n",addr);
            bad_flag=1;
            if (i>SMC_BBM.index)
            {
                DEBUG_SMC("SMC TRACE: Bad Block too much \n"); 	// need err handle or UI error message
                Rerseved_Algo_Start = bad_flag;
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
            smcBlockErase(addr);
    }
    Rerseved_Algo_Start = bad_flag;
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

    for (addr = (smcTotalSize - SMC_RESERVED_BLOCK * smcBlockSize); addr<smcTotalSize; addr += smcBlockSize)
    {
        smcRedunAreaRead(addr, smcReadBuf);
        if (smcReadBuf[0] != 0xff)
        {
            DEBUG_SMC("Bad Block in Reserved Area: %x \n",addr);
            continue;
        }
        SMC_BBM.New_Block_addr[SMC_BBM.index]=addr;
        SMC_BBM.index++;
    }

#if 0
    DEBUG_SMC("SMC_TOTAL_SIZE = %#x\n", SMC_TOTAL_SIZE);
    DEBUG_SMC("SMC_RESERVED_SIZE = %#x\n", SMC_RESERVED_SIZE);
    DEBUG_SMC("SMC_CODE_END_ADDR = %#x\n", SMC_CODE_END_ADDR);
    DEBUG_SMC("SMC_MAX_PAGE_SIZE = %#x\n", SMC_MAX_PAGE_SIZE);
    DEBUG_SMC("SMC_MAX_REDUN_SIZE = %#x\n", SMC_MAX_REDUN_SIZE);
    DEBUG_SMC("SMC_MAX_PAGE_PER_BLOCK = %#x\n", SMC_MAX_PAGE_PER_BLOCK);
    DEBUG_SMC("SMC_BLOCK_SIZE = %#x\n", SMC_BLOCK_SIZE);
    DEBUG_SMC("SMC_UI_LIBRARY_ADDR = %#x\n", SMC_UI_LIBRARY_ADDR);
    DEBUG_SMC("SMC_SYS_PARAMETER_ADDR = %#x\n", SMC_SYS_PARAMETER_ADDR);
    DEBUG_SMC("SMC_DEFECT_PIXEL_ADDR = %#x\n", SMC_DEFECT_PIXEL_ADDR);
    DEBUG_SMC("SMC_UI_SECTOR_ADDR = %#x\n", SMC_UI_SECTOR_ADDR);
    DEBUG_SMC("SMC_AWB_SECTOR_ADDR = %#x\n", SMC_AWB_SECTOR_ADDR);
    DEBUG_SMC("SMC_TOTAL_PAGE_CNT = %#x\n", SMC_TOTAL_PAGE_CNT);
    DEBUG_SMC("SMC_MAX_MAP_SIZE_IN_BYTE = %#x\n", SMC_MAX_MAP_SIZE_IN_BYTE);
    DEBUG_SMC("SMC_MAX_MAP_SIZE_IN_PAGE = %#x\n", SMC_MAX_MAP_SIZE_IN_PAGE);
    DEBUG_SMC("SMC_MAX_MAP_BLOCKS = %#x\n", SMC_MAX_MAP_BLOCKS);
    DEBUG_SMC("SMC_MAP_ADDR = %#x\n", SMC_MAP_ADDR);
    DEBUG_SMC("SMC_FAT_START_ADDR = %#x\n\n", SMC_FAT_START_ADDR);
    DEBUG_SMC("SEC_PER_CLS = %#x\n", SEC_PER_CLS);
    DEBUG_SMC("TOSECT16 = %#x\n", TOSECT16);
    DEBUG_SMC("TOSECT32 = %#x\n", TOSECT32);
    DEBUG_SMC("RSVD_SEC_CNT = %#x\n", RSVD_SEC_CNT);
    DEBUG_SMC("FATSZ16 = %#x\n", FATSZ16);
    DEBUG_SMC("ROOT_ENTRY_CNT = %#x\n", ROOT_ENTRY_CNT);
    DEBUG_SMC("SMC_RESERVED_SEC_START = %#x\n", SMC_RESERVED_SEC_START);
    DEBUG_SMC("SMC_RESERVED_LOGICAL_START = %#x\n", SMC_RESERVED_LOGICAL_START);
#endif

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
s32 smcCheckTotalBlock(void)
{
    /* cytsai: 0315 */
    u32 addr;
    u16 i=0;
    u32 bad_flag=0;

    smcBBM_init();

    for (addr = SMC_RESERVED_SIZE; addr < smcTotalSize; addr += smcBlockSize)
    {
        smcRedunAreaRead(addr, smcReadBuf);
        if (smcReadBuf[0] != 0xff)
        {
            DEBUG_SMC("SMC TRACE2: Bad Block: %x \n",addr);
            bad_flag=1;
            if (i>SMC_BBM.index)
            {
                DEBUG_SMC("SMC TRACE: Bad Block too much \n");     // need err handle or UI error message
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

Routine Description:

	Copy-back function routine.

Arguments:

	unDstAddr - the destination address to move to.
	unSrcAddr - the source address to read from.
	pucStat - EDC Status

Return Value:

	0 - Failure.
	1 - Success.

Remark:

	Copy-back is only valid in the same plane.

*/
s32 smcPageCopyback(u32 unDstAddr, u32 unSrcAddr, u8* pucEdcStat)
{
    u8 err;


    /* copy-back read */
    SmcCfg = 	SMC_CFG_CMD1|
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_ADV_EN |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_DATA_LEN_2048B;

    SmcCmd = 	SMC_CMD_COPY_BACK_READ_1ST |
              ((u32) SMC_CMD_COPY_BACK_READ_2ND << 8);

    smcSetReadWriteAddr(unSrcAddr);

    SmcCtrl = SMC_CTRL_OP_START;

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
        DEBUG_SMC("Error: smcMboxEvt is %d in smcPageCopyback.\n", err);
        return SMC_OP_STAT_FAIL;
    }
#endif

    /* copy-back program */
    if (smcCheckReadReady() != SMC_OP_STAT_PASS)
        return SMC_OP_STAT_FAIL;

    SmcCfg = 	SMC_CFG_CMD1|
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_DISA |
              SMC_CFG_ADV_EN |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_DATA_LEN_2048B;

    SmcCmd = 	SMC_CMD_COPY_BACK_PROGRAM_1ST |
              ((u32) SMC_CMD_COPY_BACK_PROGRAM_2ND << 8);

    smcSetReadWriteAddr(unDstAddr);

    SmcCtrl = SMC_CTRL_OP_START;

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
        DEBUG_SMC("Error: smcMboxEvt is %d in smcPageCopyback.\n", err);
        return SMC_OP_STAT_FAIL;
    }
#endif

    if (smcCheckReadReady() != SMC_OP_STAT_PASS)
        return SMC_OP_STAT_FAIL;

#if 0
    /* Return Register Status for NAND Writing Ability */
    if (smcReadEdcStatus(pucEdcStat))
        return SMC_OP_STAT_FAIL;


    return SMC_OP_STAT_PASS;
#endif
    return 1;

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
    SmcIntMask = 	SMC_INT_OP_CMPL_ENA | SMC_INT_ECC_ERR_ENA ; //| SMC_INT_ECC_COMP_ENA | SMC_INT_READ_CMPL_ENA; /*CY 0907*/

    /* Set timing */
    /*CY 0907*/
    /* Set clock divisor to make NAND gate flash read / write pulse greater than 50ns (20MHz). */
#if 0
	tRP = tWP = tRH = tWH = 1;
#else
    if (SYS_CPU_CLK_FREQ > 60000000)
    {	/* System clock won't be greater than 80MHz. */
        tRP = tRH = tWP = tWH = 1;
    }
    else if (SYS_CPU_CLK_FREQ == 48000000)
    {
        tRP = tRH = tWP = tWH = 1;
    }
    else if (SYS_CPU_CLK_FREQ == 32000000)
    {
        tRP = tRH = tWP = tWH = 3;
    }
    else if (SYS_CPU_CLK_FREQ ==  24000000)
    {
       
        tRP = tRH = tWP = tWH = 3;
    }
    else
        tRP = tWP = tRH = tWH = 1;	/* for safe setting, all timing parameters should greater than 1 */
#endif

    SmcTimeCtrl = tRP | (tRH << 4) | (tWP << 12) | (tWH << 16); /*CY 0907*/

    /* Create the semaphore */
    if (smcSemEvt == NULL) /*CY 1023*/
        smcSemEvt = OSSemCreate(0);
    //smcReadWriteSemEvt = OSSemCreate(0); /*CY 0907*/

    if (smcMboxEvt == NULL)
    {
        smcMboxEvt = OSMboxCreate(smcMboxMsg);
    }

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
    DEBUG_SMC("SMC TRACE: smcMount\n");
    BitMap_return=smcMakeBitMap(1);
    DEBUG_SMC("SMC TRACE: BitMap_return = %d\n", BitMap_return);
    smcMakeBitMapStat=1;
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
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    /* check if ECC comparison error. */
    if (intStat & SMC_INT_ECC_COMP)
    {
        /* ECC comparison error */
#if ( (FLASH_OPTION == FLASH_NAND_9001_NORMAL) || (FLASH_OPTION == FLASH_NAND_9002_NORMAL))

        OSSemPost(smcSemEvt);

#elif (FLASH_OPTION == FLASH_NAND_9002_ADV)

        unMboxResult = SMC_OP_ECC_COMP;

        /* clear ecc error status */
        OSMboxPost(smcMboxEvt, (void*) unMboxResult);
#endif
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
    u8 err;
    u8 Stat;

    SmcCfg = 	SMC_CFG_CMD2 |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_ADV_EN ;

    DEBUG_SMC("Page Program Addr %#x \n",addr);
    SmcCmd = SMC_CMD_PAGE_PROGRAM_1ST;

    smcSetReadWriteAddr(addr);
    SmcCtrl = SMC_CTRL_OP_START;

    smcMboxMsg = OSMboxPend(smcMboxEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcMboxEvt is %d in smcPageProgram.\n", err);
        return SMC_OP_STAT_FAIL;
    }

    /* set redundant area */
    //smcSetRedunAreaAdv();

    SmcCfg = 	SMC_CFG_WRITE|
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_ADV_EN;
	
	if(smcPageSize == 0x00001000)
		SmcCfg |= PAGE_MODE_4k|SMC_CFG_DATA_LEN_4096B;
	else
		SmcCfg |= PAGE_MODE_2k|SMC_CFG_DATA_LEN_2048B;
	
    SmcCmd = 	SMC_CMD_PAGE_PROGRAM_2ND |
              ((u32) SMC_CMD_READ_STATUS << 8);

    SmcCtrl = SMC_CTRL_OP_START;

    /* set write data dma */
    smcSetWriteDataDma(dataBuf, smcPageSize);

    if (smcCheckWriteReady() != SMC_OP_STAT_PASS)
        return SMC_OP_STAT_FAIL;
#if 0
    {
        u32* 	punAddr;
        u32*	punTemp;
        u32 i;
        punAddr = ((u32*)0xd0080024);
        punTemp = punAddr;
        for (i=0; i<23; i++)
        {
            DEBUG_SMC("punTemp inside %#x = %#x\n", punTemp, *(u32*)punTemp);
            punTemp++;
        }
    }
#endif
    /* Return Register Status for NAND Writing Ability */
    if (smcReadStatus(&Stat) == SMC_OP_STAT_FAIL)
        return SMC_OP_STAT_FAIL;

    if (Stat & SMC_SR_FAIL)
        return SMC_OP_STAT_SR_FAIL;		/* Bad page, program fail */
    else
        return SMC_OP_STAT_SR_PASS;

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
    u8	RtnSig,TMP;
    u32 j;
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

    /* write data to NAND for verification */
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
    for (j = 0; j < (smcPageSize/4)  ; j++)
    {

        if (*readPtr++ != j)
        {
            break;
        }
    }
	DEBUG_SMC("TEST RS Fountion in VerifyBlockErase!\n");
    TMP = 0x03;
//	smcRSErrIntrTest(unaddr);
#if 1
	DEBUG_SMC("The original data WriteBuf[100]=%x\n",smcReadBuf[100]);
	smcReadBuf[100] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[100]=%x\n",smcReadBuf[100]);
	
	DEBUG_SMC("The original data WriteBuf[511]=%x\n",smcReadBuf[511]);
	smcReadBuf[511] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[511]=%x\n",smcReadBuf[511]);
	
	DEBUG_SMC("The original data WriteBuf[516]=%x\n",smcReadBuf[516]);
	smcReadBuf[516] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[516]=%x\n",smcReadBuf[516]);
	
	DEBUG_SMC("The original data WriteBuf[600]=%x\n",smcReadBuf[600]);
	smcReadBuf[600] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[600]=%x\n",smcReadBuf[600]);
	
	DEBUG_SMC("The original data WriteBuf[700]=%x\n",smcReadBuf[700]);
	smcReadBuf[700] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[700]=%x\n",smcReadBuf[700]);
	
	DEBUG_SMC("The original data WriteBuf[800]=%x\n",smcReadBuf[800]);
	smcReadBuf[800] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[800]=%x\n",smcReadBuf[800]);	

	DEBUG_SMC("The original data WriteBuf[1150]=%x\n",smcReadBuf[1150]);
	smcReadBuf[1150] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[1150]=%x\n",smcReadBuf[1150]);
	
	DEBUG_SMC("The original data WriteBuf[1700]=%x\n",smcReadBuf[1700]);
	smcReadBuf[1700] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[1700]=%x\n",smcReadBuf[1700]);
	
	DEBUG_SMC("The original data WriteBuf[1856]=%x\n",smcReadBuf[1856]);
	smcReadBuf[1856] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[1856]=%x\n",smcReadBuf[1856]);
	
	DEBUG_SMC("The original data WriteBuf[2000]=%x\n",smcReadBuf[2000]);
	smcReadBuf[2000] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[2000]=%x\n",smcReadBuf[2000]);


	if(smcPageSize == 0x00001000)
		{
	DEBUG_SMC("The original data WriteBuf[2100]=%x\n",smcReadBuf[2100]);
	smcReadBuf[2100] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[2100]=%x\n",smcReadBuf[2100]);
	
	DEBUG_SMC("The original data WriteBuf[2511]=%x\n",smcReadBuf[2511]);
	smcReadBuf[2511] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[2511]=%x\n",smcReadBuf[2511]);
	
	DEBUG_SMC("The original data WriteBuf[2516]=%x\n",smcReadBuf[2516]);
	smcReadBuf[2516] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[2516]=%x\n",smcReadBuf[2516]);
	
	DEBUG_SMC("The original data WriteBuf[2600]=%x\n",smcReadBuf[2600]);
	smcReadBuf[2600] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[2600]=%x\n",smcReadBuf[2600]);
	
	DEBUG_SMC("The original data WriteBuf[2700]=%x\n",smcReadBuf[2700]);
	smcReadBuf[2700] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[2700]=%x\n",smcReadBuf[2700]);
	
	DEBUG_SMC("The original data WriteBuf[2800]=%x\n",smcReadBuf[2800]);
	smcReadBuf[2800] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[2800]=%x\n",smcReadBuf[2800]);	

	DEBUG_SMC("The original data WriteBuf[3150]=%x\n",smcReadBuf[3150]);
	smcReadBuf[3150] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[3150]=%x\n",smcReadBuf[3150]);
	
	DEBUG_SMC("The original data WriteBuf[3700]=%x\n",smcReadBuf[3700]);
	smcReadBuf[3700] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[3700]=%x\n",smcReadBuf[3700]);
	
	DEBUG_SMC("The original data WriteBuf[3856]=%x\n",smcReadBuf[3856]);
	smcReadBuf[3856] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[3856]=%x\n",smcReadBuf[3856]);
	
	DEBUG_SMC("The original data WriteBuf[4000]=%x\n",smcReadBuf[4000]);
	smcReadBuf[4000] = TMP++;
	DEBUG_SMC("The modifed data is WriteBuf[4000]=%x\n",smcReadBuf[4000]);
		}
	unaddr =unaddr + smcPageSize;
 /* write error data to NAND for verification */
 	if (smcDisEccPageProgram(unaddr, smcReadBuf, 0x1001) == 0)
   		{
       		DEBUG_SMC("Error: Page Program Function Failed in VerifyBlockErase!\n");
       		return 0;
   		}
	
	RtnSig = smcPage1ARead(unaddr, smcReadBuf);
	
    if (RtnSig == 0)
    {
        DEBUG_SMC("Error: Page1A Read Function Failed in VerifyBlockErase!\n");
        return 0;
    }
	else if (RtnSig == SMC_OP_ECC_ERROR)
		{  
				unaddr =unaddr + smcPageSize;
		        DEBUG_SMC("RS Correction data Write back Nand Flash ");
    		if (smcDisEccPageProgram(unaddr, smcReadBuf, 0x1001) == 0)
    		{
        		DEBUG_SMC("Error: Page Program Function Failed in VerifyBlockErase RS!\n");
        		return 0;
    		}
	    	if (smcPage1ARead(unaddr, smcReadBuf) == 0)
    		{
        		DEBUG_SMC("Error: Page1A Read Function Failed in VerifyBlockErase RS!\n");
        		return 0;
    		}
		}	
    readPtr = (u32*) smcReadBuf;
    for (j = 0; j < (smcPageSize/4)  ; j++)
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
	u32	i;
	u32 *punWriteBuf = (u32*) smcWriteBuf;
	u32 unAddrForTest;

	DEBUG_SMC("\n\n");
    DEBUG_SMC("***************************\n");
	DEBUG_SMC("*                         *\n");
	DEBUG_SMC("*9002_ADVNAND Flash Test  *\n");
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
	DEBUG_SMC("TEST ECC Fountion in VerifyBlockErase!\n");
	if (smcVerifyBlockErase(unAddrForTest) == 0)
		return 0;

//	DEBUG_SMC("TEST ECC Fountion !!\n");
//	unAddrForTest = unAddrForTest - smcBlockSize;
	
#if 0
	if (smcRSErrIntrTest(u32 unAddrForTest)(unAddrForTest) == 0)

	 	{

    		{
        		u32* 	punAddr;
        		u32*	punTemp;
        		u32 i;
        		punAddr = ((u32*)0xd0080000);
        		punTemp = punAddr;
        		for (i=0; i<24; i++)
        		{
            	DEBUG_SMC("punTemp inside %#x = %#x\n", punTemp, *(u32*)punTemp);
            	punTemp++;
        		}
    		}
			 return 0;
		}
	
#endif
	return 1;

}

#endif	//#if INTERNAL_STORAGE_TEST

#if 0
//#ifdef SMC_VERIFY

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
                                  (SmcCtrlBase + 0x0024),	//SmcRedunA0
                                  (SmcCtrlBase + 0x0028),	//SmcRedunA1
                                  (SmcCtrlBase + 0x002c),	//SmcRedunA2
                                  (SmcCtrlBase + 0x0030),	//SmcRedunA3
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
                                             0,	//SmcRedunA0
                                             0,	//SmcRedunA1
                                             0,	//SmcRedunA2
                                             0,	//SmcRedunA3
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
#if 0
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

    DEBUG_SMC("[Pass]: ECC SW Test\n");
#endif

    return 1;

}

/*

Routine Description:

	Ecc Test Routine.

Arguments:

	unaddr - addr to write.

Return Value:

	0 - Failed
	1 - Success

*/

s32 smcEccTest(u32 unaddr)
{

    if (smcBlockErase(unaddr) == 0)
    {
        DEBUG_SMC("Error: Block Erase Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    if (smcPageProgram(unaddr, smcWriteBuf, 0x1001) == 0)
    {
        DEBUG_SMC("Error: Page Program Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    return smcECCSWResult((u32*)smcWriteBuf);


}
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
#if 0
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
                                  (SmcCtrlBase + 0x0024),	//SmcRedunA0
                                  (SmcCtrlBase + 0x0028),	//SmcRedunA1
                                  (SmcCtrlBase + 0x002c),	//SmcRedunA2
                                  (SmcCtrlBase + 0x0030),	//SmcRedunA3
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
                                 0xFFFFFFFF,		//(SmcCtrlBase + 0x0024),	//SmcRedunA0
                                 0xFFFFFFFF,		//(SmcCtrlBase + 0x0028),	//SmcRedunA1
                                 0xFFFFFFFF,		//(SmcCtrlBase + 0x002c),	//SmcRedunA2
                                 0xFFFFFFFF,		//(SmcCtrlBase + 0x0030),	//SmcRedunA3
                                 0x00000000,		//(SmcCtrlBase + 0x0034),	//SmcEcc1
                                 0x00000000,		//(SmcCtrlBase + 0x0038),	//SmcEcc2

                                 0x00000000,		//(DmaCtrlBase + 0x0010),	//DmaCh1SrcAddr
                                 0x00000000,		//(DmaCtrlBase + 0x0014),	//DmaCh1DstAddr
                                 0x00000000,		//(DmaCtrlBase + 0x0018),	//DmaCh1CycCnt
                                 0x00000000,		//(DmaCtrlBase + 0x001C),	//DmaCh1Cmd

                                 0x00000000,		//(GpioCtrlBase + 0x0020),	//GpioActFlashSelect(MBUS_SEL)

                                 0x00000600,		//(SysCtrlBase)			//SYS_CTL0

                             };	//Address list of registers which will be tested.

#if 0
    test_count = 0;
    for (i=0; i<21; i++)
        if ( *((u32*)unRegTestListAddr[i]) != unRegDeaultValue[i])
        {
            DEBUG_SMC("The register [0x%lx] = [0x%lx] NEQ default value [0x%lx]\n", (u32*)unRegTestListAddr[i],  *((u32*)unRegTestListAddr[i]), unRegDeaultValue[i]);

            //DEBUG_SMC("Not match with the deault values.\n");
            //return 0;
            test_count++;
        }
#endif

    unReadOutValue = SmcCfg;

    SmcCfg =~ unReadOutValue;

    SYS_RSTCTL |= 0x00040000;
    SYS_RSTCTL &= 0x00000000;

    if (SmcCfg != unReadOutValue)
    {
        DEBUG_SMC("Reset and defult value verification pass!\n");
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
#endif
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
    u32	i, j;
    u32* punReadBuf;
//	u8	smcTestBuf[SMC_MAX_PAGE_SIZE];

    DEBUG_SMC("Block Erasing...\n");
    if (smcBlockErase(unaddr) == 0)
    {
        DEBUG_SMC("Error: Block Erase Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    punReadBuf = (u32*) PKBuf;
    /* set write data */
    for (i = 0; i < SMC_MAX_PAGE_SIZE/4; i++)
        *punReadBuf++ = i;

    for (i=unaddr; i<(smcPageSize*smcPagePerBlock+unaddr); i+=smcPageSize)		/* Start from block 1 */
    {
        memset(smcReadBuf, 0, smcPageSize);
        memset(smcTestBuf, 0xff, smcPageSize);

        if (smcPage1ARead(i, smcReadBuf) == 0)
        {
            DEBUG_SMC("Read Fail.\n");
            return 0;
        }

        if (memcmp(smcTestBuf, smcReadBuf, smcPageSize)!=0)
        {
            DEBUG_SMC("\n[Error]: erased data don't equal to 0xff.\n");
            for (j=0; j<8; j++)
            {
                DEBUG_SMC("smcReadBuf[%d] = %#x\n", j, smcReadBuf[j]);
                return 0;
            }
            DEBUG_SMC("\n");
        }
    }

    DEBUG_SMC("Page Programming...\n");

    for (i=unaddr; i<(smcPageSize*smcPagePerBlock+unaddr); i+=smcPageSize)		/* Start from block 1 */
    {
        if (smcPage1ARead(i, smcReadBuf) == 0)
        {
            DEBUG_SMC("[Error]: Invalid page.\n");
            return 0;
        }
        if (smcPageProgram(i, smcWriteBuf, 0xFFFF)==0)
        {
            DEBUG_SMC("[Error]: Page Program Error in VerifyBlockErase\n");
            return 0;
        }
    }

    DEBUG_SMC("Page Reading...\n");

    for (i=unaddr; i<(smcPageSize*smcPagePerBlock+unaddr); i+=smcPageSize)		/* Start from block 1 */
    {
        memset(smcReadBuf, 0, smcPageSize);
        if (smcPage1ARead(i, smcReadBuf) == 0)
        {
            DEBUG_SMC("Read Fail.\n");
            return 0;
        }

        if (memcmp(PKBuf, smcReadBuf, smcPageSize) !=0)
        {
            DEBUG_SMC("[Error]: Read data don't match to Program data.\n");
            DEBUG_SMC("\nAddr = %#x\n", i);
#if 0
            for (j=0; j<2048; j++)
            {
                if (PKBuf[j]!= smcReadBuf[j])
                {
                    DEBUG_SMC("PKBuf[%d] = %#x\n", j, PKBuf[j]);
                    DEBUG_SMC("smcReadBuf[%d] = %#x\n", j, smcReadBuf[j]);
                }
            }
#endif
            return 0;
        }
    }

    DEBUG_SMC("[Pass]: Block Erase.\n");

    return 1;

}


/*

Routine Description:

	 Redundant area access test.

Arguments:

	usblockAddrField - The Block Address Field.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 smcRedunAreaAccessTest(u32 unaddr, u32* punWriteBuf)
{
    u32 * readPtr;
    u32 j;


    if (smcBlockErase(unaddr) == 0)
    {
        DEBUG_SMC("Error: Block Erase Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    memset(smcReadBuf, 0, 256);
    if (smcPage1ARead(unaddr, smcReadBuf) == 0)
    {
        DEBUG_SMC("Error: Page1A Read Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    readPtr = (u32*) smcReadBuf;
    for (j = 0; j<64/4; j++)
    {

        if (*readPtr++ != 0xFFFFFFFF)
        {
            DEBUG_SMC("Error! Content at Address[%#x] does not equal to 0xFF!\n", j);
            return 0;
        }
    }

    if (smcBlockErase(unaddr) == 0)
    {
        DEBUG_SMC("Error: Block Erase Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    if (smcRedunAreaProgram(unaddr, smcWriteBuf) == 0)
    {
        DEBUG_SMC("Error: Page Program Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    /* initialize read buffer */
    memset(smcReadBuf, 0, 256);

    if (smcRedunAreaRead(unaddr, smcReadBuf) == 0)
    {
        DEBUG_SMC("Error: Page1A Read Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    readPtr = (u32*) smcReadBuf;
    for (j = 0; j < (64/4)  ; j++)
    {
        if (*readPtr++ != j)
        {
            DEBUG_SMC("\nThis is Pre-program before Block Erase.\n");
            DEBUG_SMC("Content at Address[%#X] does not equal to #%x!\n", (u32*) readPtr, j);
            DEBUG_SMC("%#X = %#X\n", (u32 *)readPtr, *(u32 *)readPtr);
            return 0;
        }
    }

    DEBUG_SMC("\n[Pass]: Redundant Area Access Test.\n");
    return 1;
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
#if 0
    DMA_CFG dmaCfg;

    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(SmcData);
    dmaCfg.cnt = siz/ (u32) ( (u8) cBurstIdx * (u8) ucDataWidth);
    dmaCfg.burst = (u32) cBurstIdx;
    if (dmaConfigTest(DMA_REQ_SMC_WRITE, &dmaCfg, cBurstIdx, ucDataWidth) == 0)
        return 0;
#endif
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
s8 smcPageProgramDMATest(u32 addr, u8* dataBuf, u16 blockAddrField, s8 cBurstIdx, u8 ucDataWidth)
{

    u8 err;

    SmcCfg = 	SMC_CFG_CMD2 |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_ENA |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_ADV_EN |
              SMC_CFG_DATA_LEN_2048B;

    SmcCmd = SMC_CMD_PAGE_PROGRAM_1ST;

    smcSetReadWriteAddr(addr);
    SmcCtrl = SMC_CTRL_OP_START;

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
        DEBUG_SMC("Error: smcMboxEvt is %d in smcPageProgramDMATest.\n", err);
        return 0;
    }
#endif

    SmcCfg = 	SMC_CFG_WRITE|
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_ENA |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_ADV_EN |
              SMC_CFG_DATA_LEN_2048B;

    SmcCmd = 	SMC_CMD_PAGE_PROGRAM_2ND |
              ((u32) SMC_CMD_READ_STATUS << 8);

    SmcCtrl = SMC_CTRL_OP_START;

    /* set write data dma */
    smcSetWriteDataDmaTest(dataBuf, smcPageSize, cBurstIdx, ucDataWidth);

    /* set redundant area */
    smcSetRedunAreaAdv();

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
    u8 err;

    SmcCfg = 	SMC_CFG_CMD1|
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_ENA |
              SMC_CFG_ADV_EN |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_DATA_LEN_2048B;

    SmcCmd = 	SMC_CMD_READ_1ST |
              ((u32) SMC_CMD_READ_2ND << 8);


    smcSetReadWriteAddr(addr);
    SmcCtrl = SMC_CTRL_OP_START;


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
        DEBUG_SMC("Error: smcMboxEvt is %d in smcPage1AReadDMATest.\n", err);
        return 0;
    }
#endif

    SmcCfg = 	SMC_CFG_READ |
              SMC_CFG_NAND |
              SMC_CFG_WP_HI |
              SMC_CFG_ECC_ENA |
              SMC_CFG_ADV_EN |
              SMC_CFG_WAIT_BUSY_EN |
              SMC_CFG_DATA_LEN_2048B;


    /* set read data dma */
    smcSetReadDataDMATest(dataBuf, smcPageSize, cBurstIdx, ucDataWidth);

    SmcCtrl = SMC_CTRL_OP_START;

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
#if 0
    DMA_CFG dmaCfg;

    /* set read data dma */
    dmaCfg.src = (u32)&(SmcData);
    dmaCfg.dst = (u32)buf;
    dmaCfg.cnt = siz / (u32) ((u8 )cBurstIdx *  ucDataWidth);
    dmaCfg.burst = (u32) cBurstIdx;
    if (dmaConfigTest(DMA_REQ_SMC_READ, &dmaCfg, cBurstIdx, ucDataWidth) == 0)
        return 0;
#endif
    return 1;
}

/*

Routine Description:

	Mass Read/ Write Test Routine.

Arguments:


Return Value:

	0 - Failed
	1 - Success

*/
s32 smcMassReadWriteTest(u8* ucBuf)
{



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
    u32	j;
    s8	cBurstIdx;	//Burst Mode Index,		0: No Burst, 1: Burst
    u8	ucDataWidth;	//Data Width,		0: Byte, 1: Half-Word, 2: Word
    u32*	readPtr;
    u8	ucLoopIdx;
    u8	uctest_idx = 0;
    u8	ucsmcDMATestFuncIdx = 0;


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
            DEBUG_SMC("");
            break;
        }

        uctest_idx = 0;		//initialize test index;
        if (smcBlockErase(unaddr) == 0)
        {
            DEBUG_SMC("Error: BlockErase Failed in DMAVerify functions.\n");
            return 0;
        }

        memset(smcReadBuf, 'a', 2048);
        if (smcPage1AReadDMATest(unaddr, (u8*)smcReadBuf, cBurstIdx, ucDataWidth) == 0)
        {
            uctest_idx ++;
            DEBUG_SMC("\n");
            DEBUG_SMC("Error: Page1ARead of [BlockErase] Failed in DMAVerify functions.\n");
            DEBUG_SMC("\n");
            //return 0;
        }

        readPtr  = (u32*) smcReadBuf;
        for (j = 0; j < 2048/4; j++)
            if (*readPtr ++ != 0xffffffff)
            {
                uctest_idx ++;
                DEBUG_SMC("Error: Data Comparison Failed After Block Erasing.\n");
                DEBUG_SMC("readPtr[%#x] = %#x; j = %d\n", readPtr--, *readPtr, j);
                break;
            }

        /* Execute the following Program test if Blcok Erase is success. */
        if (uctest_idx==0)
        {


            if (smcPageProgramDMATest(unaddr, (u8*)smcWriteBuf, usblockAddrField, cBurstIdx, ucDataWidth) == 0)
            {
                uctest_idx ++;
                DEBUG_SMC("\n");
                DEBUG_SMC("Error: Page Program Failed in DMAVerify functions.\n");
                DEBUG_SMC("\n");
                //return 0;
            }

            memset(smcReadBuf, 'a', 2048);
            if (smcPage1AReadDMATest(unaddr, (u8*)smcReadBuf, cBurstIdx, ucDataWidth) == 0)
            {
                uctest_idx ++;
                DEBUG_SMC("\n");
                DEBUG_SMC("Error: Page1ARead of [PgeProgram] Failed in DMAVerify functions.\n");
                DEBUG_SMC("\n");
                //return 0;
            }

            readPtr = (u32*) smcReadBuf;
            for (j = 0; j < 2048/4; j++)
                if (*readPtr++ != j)
                {
                    DEBUG_SMC("Error: Data Comparison Failed After Page Programming.\n");
                    break;
                }
        }

        if (uctest_idx==0)
        {
            if (ucLoopIdx == 0) 		/* Burst Mode */
                DEBUG_SMC("[Pass]: DMA Test in [Burst Mode] and DataWidth [Word]!\n");
            else
                DEBUG_SMC("[Pass]: DMA Test in [No Burst Mode] and DataWidth [Word]!\n");
        }
        else
        {
            ucsmcDMATestFuncIdx ++;	/* Error Index plus 1 */
            if (ucLoopIdx == 0) 		/* Burst Mode */
                DEBUG_SMC("\n[Fail]:DMA Test in [Burst Mode] and DataWidth [Word]!\n");
            else
                DEBUG_SMC("\n[Fail]: DMA Test in [No Burst Mode] and DataWidth [Word]!\n");
        }


    }

    if (ucsmcDMATestFuncIdx !=0)
        return 0;
    else
        return 1;


}

/*

Routine Description:

	Whole chip status update.

Arguments:

	None.

Return Value:

	None.

*/
s32 smcWholeChipStatusUpdate(void)
{
    u32 i, j;
    u32	unCnt;
    u32* punWriteBuf = (u32*)PKBuf0;

    for (i=0; i<SMC_MAX_PAGE_SIZE/4; i++)
        *punWriteBuf++=i;

    for (i=0; i<smcTotalSize; i+=smcBlockSize)
    {
//        DEBUG_SMC("Addr = %#x\n", i);

        if (!smcRedunAreaRead(i, PKBuf))
        {
            DEBUG_SMC("Redun Read Error\n");
            continue;
        }
        if (*PKBuf != 0xFF)
        {
            DEBUG_SMC("Bad Block: %#x\n", i);
            continue;
        }

        if (!smcBlockErase(i))
        {
            DEBUG_SMC("Block Erase Error\n");
            break;
//			return 0;
        }

        for (j=i; j<(i+smcBlockSize); j+=smcPageSize)
        {
//			DEBUG_SMC("program addr = %#x\n", j);

            if (!smcPageProgram(j, PKBuf0, 0xFFFF))
            {
                DEBUG_SMC("Page Program Error\n");
                return 0;
            }

            if (!smcPage1ARead(j, PKBuf))
            {
                DEBUG_SMC("Read Error\n");
                return 0;
            }

            if (memcmp(PKBuf0, PKBuf, smcPageSize)!=0)
            {
                *PKBuf1 = 0x00;
                if (!smcRedunAreaProgram(i, PKBuf1))
                {
                    DEBUG_SMC("Redun Program Error\n");
                    return 0;
                }

                /* to confirm the write results */
                if (!smcRedunAreaRead(j, PKBuf))
                {
                    DEBUG_SMC("Redun Read Error\n");
                    return 0;
                }
                DEBUG_SMC("PKBuf = %#x\n", *PKBuf);
                break;
            }

            /* Erase to default 0xff */
            if (!smcBlockErase(i))
            {
                DEBUG_SMC("Block Erase Error\n");
                break;
                //			return 0;
            }


        }

        unCnt++;
        if ((unCnt % 30) == 0)
        {
            DEBUG_SMC(".");
        }

    }

    DEBUG_SMC("\n\n");
    return 1;

}

/*

Routine Description:

	ECC Error Interrupt test.

Arguments:

	None.

Return Value:

	None.

*/
s32 smcECCErrIntrTest(u32 unAddr)
{

    u32	i;
    u32*	punReadBuf;
    u32	unRegData;

    smcRedunAreaProgram(unAddr, smcWriteBuf);


    if (smcBlockErase(unAddr) == 0)
    {
        DEBUG_SMC("Error: Block Erase Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    if (smcPageProgram(unAddr, smcWriteBuf, 0x1001) == 0)
    {
        DEBUG_SMC("Error: Page Program Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    punReadBuf = (u32*) smcReadBuf;
    for (i=0; i<16; i++)
        *((u32*) punReadBuf ++) = 0;

    if (smcRedunAreaRead(unAddr, smcReadBuf) == 0)
    {
        DEBUG_SMC("Error: Redundant Area Read Error in [ECC Error Intr Test]!\n");
        return 0;
    }

    /* inverse the ECC2 result to make a ECC error occurs */
    punReadBuf = (u32*) smcReadBuf + 8;
    DEBUG_SMC("The original data is %#x\n", *((u32*)punReadBuf));
    *((u32*) punReadBuf) = ~*((u32*)punReadBuf);
    DEBUG_SMC("The modifed data is %#x\n", *((u32*)punReadBuf));

    /* Write the inversed data into the redundant area */
    if (smcRedunAreaProgram(unAddr + 0x00000800, (u8*) punReadBuf) == 0)
    {
        DEBUG_SMC("Error: Page Program Function Failed in VerifyBlockErase!\n");
        return 0;
    }

    /* Read the page data again to verify ECC */
    if (smcPage1ARead(unAddr, smcReadBuf) == 0)
    {
        DEBUG_SMC("Page1A Read Function Failed in VerifyPageProgram!\n");
        return 0;
    }
    else if  (smcPage1ARead(unAddr, smcReadBuf) == SMC_OP_ECC_ERROR)
    {
        unRegData = SmcStat;
        unRegData &= 0x3C000000;	/* to get ecc error status */

        if (unRegData == 0x3C000000)
            DEBUG_SMC("[Pass]: ECC Error Test\n");

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
s32 smcAllBlockErase(void)
{
    u32 i, j;



    for (i=0; i<smcTotalSize; i+= smcBlockSize)
    {
        if (!smcRedunAreaRead(i, PKBuf))
        {
            DEBUG_SMC("Error Redun Read Error\n");
            return 0;
        }

        if ((*PKBuf) == 0xff)
        {
//			DEBUG_SMC("addr = %#x\n", i);
            smcBlockErase(i);
        }
        else
        {
            DEBUG_SMC("Block Fail in Addr %#x\n", i);
        }

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
    u32 i;
    u32* writePtr = (u32*) smcWriteBuf;
    u32	unaddr;

    u8 ucVerifyIdx = 1;
    u8 ucTestItemSel;


    smcInit();

    smcIdentification();

    /* set write data */
    for (i = 0; i < SMC_MAX_PAGE_SIZE/4; i++)
        *writePtr++ = i;

    while (ucVerifyIdx == 1)
    {
        smcResetEccStat();

        DEBUG_SMC("\n\n\n");
        DEBUG_SMC("\n************************************\n");
        DEBUG_SMC("*\n");
        DEBUG_SMC("*	Starting to verify NAND Flash Controller.\n");
        DEBUG_SMC("*\n");
        DEBUG_SMC("************************************\n");
        DEBUG_SMC("\n\n");


        DEBUG_SMC("Please Select a item to test! \nKey-in a number to test.\n");
        DEBUG_SMC("[1] - ID Detection.\n");
        DEBUG_SMC("[2] - NAND Flash - Block Erase Test.\n");
        DEBUG_SMC("[3] - NAND Flash - Redundant Area Access Test.\n");
        DEBUG_SMC("[4] - NAND Flash - DMA Access Test.\n");
        DEBUG_SMC("[5] - NAND Flash - ECC SW Test.\n");
        DEBUG_SMC("[6] - NAND Flash - ECC ERROR Int Test.\n");
        DEBUG_SMC("[7] - NAND Flash - All Blocks Erase.\n");
        DEBUG_SMC("[8] - NAND Flash - Whole Chip R/W and Status Update.\n");
        DEBUG_SMC("[9] - NAND Flash - Full Test\n");
        DEBUG_SMC("[10] - NAND Flash - ECC Algorithm Test\n");
        DEBUG_SMC("[11] - NAND Flash - Mark the bad block\n");
        DEBUG_SMC("[12] - NAND Flash - ERASE FAT BLOCKS\n");
        DEBUG_SMC("[13] - NAND FLASH - UPDATE BLOCKS\n");
        DEBUG_SMC("[99] - Finish verification.\n");

        /* read out the data from the last block */
        /* make the data to be the intergrated number of block size */
        if (smcPage1ARead(smcTotalSize-smcBlockSize, smcReadBuf) !=1)
        {
            DEBUG_SMC("[Error]: Read Failed\n");
            DEBUG_SMC("Quit Verifications\n");
            return 0;
        }
        else
        {
            unaddr = *(u32*)smcReadBuf;

            /* to clear bits */
            {
                u32	unBlockSize;
                u32 bit=0;

                unBlockSize = smcBlockSize;

                while (1)
                {
                    if (((unBlockSize >> bit) & 0x00000001) == 1)
                    {
                        unaddr >>= bit;
                        unaddr <<= bit;
                        break;
                    }
                    else
                        bit++;
                };
            }

            if (unaddr >= smcTotalSize)
                unaddr = SMC_RESERVED_SIZE *2;		/* skip the first 2MB space avoid deleting boot code */

            DEBUG_SMC("The Block Address to Test is %#x\n", unaddr);
        }

        DEBUG_SMC("Please select a item to verify:");
        DEBUG_SMC("The Blcok Address to Test is %#x\n", unaddr);
        scanf("%d", &ucTestItemSel);
        DEBUG_SMC("\n");


        switch (ucTestItemSel)
        {

        case	1:		/* ID Detection */
            smcReset();
            smcIdentification();

            break;

        case	2:
            DEBUG_SMC("\nStart to [Block Erase Test]\n");
            smcVerifyBlockErase(unaddr);

            break;

        case	3:
            DEBUG_SMC("\nStart to [Redundant Area Access Test]\n");
            smcRedunAreaAccessTest(unaddr, writePtr);

            break;

        case	4:
            DEBUG_SMC("\nStart to [DMA Access Test]\n");
            smcDMAVerify(unaddr, 0x1001);

            break;

        case	5:
            DEBUG_SMC("\nStart to [ECC SW Test]\n");
            smcEccTest(unaddr);

            break;

        case	6:
            DEBUG_SMC("\nStart to [ECC ERROR Intr Test]\n");
            smcECCErrIntrTest(unaddr);
            break;

        case	7:
            DEBUG_SMC("\nStart to [All Block Erase]\n");
            smcAllBlockErase();

            break;

        case	8:
            DEBUG_SMC("\nStart to [Whole Chip R/W and Status Update.]\n");
            smcWholeChipStatusUpdate();
            break;

        case	9:
            DEBUG_SMC("\nStart to [Full Test]\n");
            DEBUG_SMC("\nStart to [Block Erase Test]\n");
            smcVerifyBlockErase(unaddr);
            DEBUG_SMC("\nStart to [Redundant Area Access Test]\n");
            smcRedunAreaAccessTest(unaddr, writePtr);
            DEBUG_SMC("\nStart to [DMA Access Test]\n");
            smcDMAVerify(unaddr, 0x1001);
            DEBUG_SMC("\nStart to [ECC SW Test]\n");
            smcEccTest(unaddr);

            break;

        case	10:
        {
            u32 i, j;
            u32	unBadBlockAddr[3];
            u32 unTestAddr;
#if 0
            u32* punTemp;
#endif
            u32* punAddr;
            DEBUG_SMC("\nStart to [ECC ALgorithm Test]\n");


            j = 0;
            for (i=smcTotalSize-smcBlockSize*5; i<smcTotalSize; i+= smcBlockSize)
            {
                if (!smcRedunAreaRead(i, smcReadBuf))
                {
                    DEBUG_SMC("Redun Read Error\n");
                }

                if (smcReadBuf[0] != 0xff)
                {
                    DEBUG_SMC("Bad Block : %#x\n", i);
                    unBadBlockAddr[j++] = i;
                }
                else
                {
                    smcBlockErase(i);
                    unTestAddr = i;
                    break;
                }
            }

            writePtr = (u32*)PKBuf;
            for (i = 0; i < SMC_MAX_PAGE_SIZE/4; i++)
                *writePtr++ = i;

            punAddr = ((u32*)0xd0080034);
            DEBUG_SMC("%#x = %#x\n", punAddr, *((u32*)0xd008003C));
            *(u32*)0xd008003C = 0x55aa5678;
            DEBUG_SMC("%#x = %#x\n", punAddr, *((u32*)0xd008003C));

            *((u8*)PKBuf+1) = 0x8A;

#if 0
            *((u32*)PKBuf+4) = 0x3E;
            *((u32*)PKBuf+12) = 0x7C;
            *((u32*)PKBuf+80) = 0xD2;
            DEBUG_SMC("data 1= %#x\n", *((u32*)PKBuf+4));
            DEBUG_SMC("data 1= %#x\n", *((u32*)PKBuf+12));
            DEBUG_SMC("data 1= %#x\n", *((u32*)PKBuf+80));
#endif
            smcPageProgram(unTestAddr, PKBuf, 0xFFFF);
#if 1
            smcRedunAreaRead(unTestAddr, PKBuf1);
            for (i=0; i<64/4; i++)
                DEBUG_SMC("%d = %#x\n", i, *((u32*)PKBuf1+i));
#endif
            punAddr = ((u32*)0xd0080024);

#if 0
            punTemp = punAddr;
            for (i=0; i<24; i++)
            {
                DEBUG_SMC("punTemp1 %#x = %#x\n", punTemp, *(u32*)punTemp);
                punTemp++;
            }
#endif
            unTestAddr += smcPageSize*4;

            *((u8*)PKBuf+1) = 0x8B;
#if 0
            *((u32*)PKBuf+4) = 0xD0;
            *((u32*)PKBuf+12) = 0x9F;
            *((u32*)PKBuf+80) = 0xA3;
            DEBUG_SMC("data 2= %#x\n", *((u32*)PKBuf+4));
            DEBUG_SMC("data 2= %#x\n", *((u32*)PKBuf+12));
            DEBUG_SMC("data 2= %#x\n", *((u32*)PKBuf+80));
#endif
            smcPageProgram(unTestAddr, PKBuf, 0xFFFF);

            smcRedunAreaRead(unTestAddr, PKBuf1);
            for (i=0; i<64/4; i++)
                DEBUG_SMC("%d = %#x\n", i, *((u32*)PKBuf1+i));

            punAddr = ((u32*)0xd0080024);
#if 0
            punTemp = punAddr;
            for (i=0; i<24; i++)
            {
//					DEBUG_SMC("SmcRedun %d = %#x\n", i, *(u32*)punTemp++);
                DEBUG_SMC("punTemp2 %#x = %#x\n", punTemp, *(u32*)punTemp);
                punTemp++;

            }
#endif
            *(u32*)PKBuf -= 1;

        }
        break;

        case 11:
        {
            u32 unMarkBlockAddr;

            DEBUG_SMC("Please key-in the block address[HEX] to mark:\n");

            scanf("%x", &unMarkBlockAddr);
            DEBUG_SMC("\n");

            if (!smcBlockErase(unMarkBlockAddr))
            {
                DEBUG_SMC("Block Erase Failed.\n");
                break;
            }
            unMarkBlockAddr = (unMarkBlockAddr / smcBlockSize)*smcBlockSize;
            *PKBuf1 = 0x00;
            if (!smcRedunAreaProgram(unMarkBlockAddr, PKBuf1))
            {
                DEBUG_SMC("smcRedunAreaProgram Failed.\n");
                break;
            }
            else
                DEBUG_SMC("Cmp\n");


        }
        break;

        case 12:
        {
            u32	unAddrTemp, k;

            unAddrTemp = SMC_FAT_START_ADDR;

            DEBUG_SMC("SMC TRACE: ERASE FAT BLOCKS.\n");

            for (k=0; k<6; k++)
            {
                if (smcBlockErase(unAddrTemp) == 0)
                {
                    DEBUG_SMC("SMC ERR: Block Erase Failed on addr %#x\n", unAddrTemp);
                }
                unAddrTemp += smcBlockSize;
            }
        }
        break;

        case 13:
        {
            u32	unAddrTemp, k;

            DEBUG_SMC("SMC DEBUG: UPDATE AND ERASE BLOCKS\n");
            for (k=0; k<smcTotalSize; k+=smcBlockSize)
            {
                if (smcRedunAreaRead(k,smcReadBuf) == 0)
                {
                    DEBUG_SMC("SMC ERR: smcRedunAreaRead Error\n");
                }
                if (*smcReadBuf != 0xFF)
                {
                    DEBUG_SMC("SMC ERR: BAD BLOCK %#x\n", k);
                    continue;
                }
                if (smcBlockErase(k) == 0)
                {
                    *smcWriteBuf = 0;
                    smcRedunAreaProgram(k, smcWriteBuf);
                }

            }

        }
        break;


        case	99:
            DEBUG_SMC("Finish Verification!\n");
            ucVerifyIdx = 0;		/* End Verification */
            break;


        default:
            DEBUG_SMC("Error! Wrong item selected!\n");

            break;
        }

    }

    DEBUG_SMC("End of Verifications.\n");

    return 1;
}
#endif
#endif
