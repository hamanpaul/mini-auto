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
#if (FLASH_OPTION == FLASH_NAND_9001_NORMAL)
#include "board.h"

#include "smc.h"
#include "smcreg.h"
#include "smcwc.h"
#include "smcapi.h"
//#include "dmaapi.h"
#include <../inc/mars_controller/mars_dma.h>
#include "fsapi.h"	/* cytsai: 0315 */
#include "rtcapi.h"
#include "dcfapi.h"
#include "sysapi.h"

/*
 *********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */

/* SMC time out value */
#define SMC_TIMEOUT				20	/*CY 1023*/

/* SMC Zone */
#define SMC_MAX_ZONE_ENTRY			1024

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
__align(4) SMC_REDUN_AREA smcRedunArea;
__align(4) u8	smcReadBuf[SMC_MAX_PAGE_SIZE];
__align(4) u8	smcWriteBuf[SMC_MAX_PAGE_SIZE];

u8 make_BitMap_ok = 0;

u32 smcTotalSize, smcBlockSize, smcPageSize,smcPagePerBlock;
u32 smcTotalPageCount;
u32 smcPageRedunSize;
u32 smcTotalZone;
u32 smcAddrCycle;
u32 smcSecPerBlock;
u8  ucAccessFlag = 0;	/* flag to indicates the status for other access */

OS_EVENT* smcSemEvt; 		/* semaphore to synchronize smc operation completion event processing */
OS_EVENT* smcProcProtSemEvt;		/* semaphore to synchronize procedure protection */

extern s32 dcfStorageSize[STORAGE_MEMORY_MAX];	/*CY 0718*/
extern u8 Rerseved_Algo_Start;
unsigned char	 smcBitMap[SMC_MAX_MAP_SIZE_IN_BYTE];

u32 guiSMCReadDMAId=0xFF, guiSMCWriteDMAId=0xFF;
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
/* Driver Function */

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
s32 smcBlockErase(u32);
s32 smcMultiBlockErase(u32, u32);
void smcTotalBlockErase(char);
//u8 BBM_Handle(u32,u8*);
NAND_FAT_BPB NAND_FAT_PARAMETER;
NAND_BBM SMC_BBM;
extern u8 smcMakeBitMap(char);
//extern s32 smcMakeTable(void);
extern s32 smcSectorsRead(u32, u32, u8*);
extern s32 smcSectorsWrite(u32, u32, u8*);

/*
 *********************************************************************************************************
 * Driver function
 *********************************************************************************************************
 */



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
        ;
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
    *redunPtr   = SmcRedun3;

    pRedunArea->user_data = (((u32)redun[3]) << 24) | (((u32)redun[2]) << 16) |
                            (((u32)redun[1]) <<  8) | (((u32)redun[0]));
    pRedunArea->data_status = redun[4];
    pRedunArea->block_status = redun[5];
    pRedunArea->block_addr1 = (((u16)redun[6]) <<  8) | ((u16)redun[7]);
    pRedunArea->ecc2[0] = redun[8];
    pRedunArea->ecc2[1] = redun[9];
    pRedunArea->ecc2[2] = redun[10];
    pRedunArea->block_addr2 = (((u16)redun[11]) <<  8) | ((u16)redun[12]);
    pRedunArea->ecc2[0] = redun[13];
    pRedunArea->ecc2[1] = redun[14];
    pRedunArea->ecc2[2] = redun[15];

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
    //if(blockAddrField!=0xFFFF)
    //SmcRedun1 =  (((u32)blockAddrFieldLoByte) << 24) | (((u32)blockAddrFieldHiByte) << 16) | 0x1234;
    //else
    SmcRedun1 =  (((u32)blockAddrFieldLoByte) << 24) | (((u32)blockAddrFieldHiByte) << 16) | 0xffff;
    /* Block Address Field 2 and Error Correction Code 2 (hardware auto-filled) */
    SmcRedun2 = (((u32)blockAddrFieldHiByte) << 24) | 0xffffff;
    /* Error Correction Code 1 (hardware auto-filled) and Block Address Field 1 */
    SmcRedun3 = 0xffffff | ((u32)blockAddrFieldHiByte);

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
    //if (dmaConfig(DMA_REQ_SD2_READ, &dmaCfg) == 0)
    //    return 0;
    guiSMCReadDMAId = marsDMAReq(DMA_REQ_SD2_READ, &dmaCfg);
    
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
    //if (dmaConfig(DMA_REQ_SD2_WRITE, &dmaCfg) == 0)
    //    return 0;
    guiSMCWriteDMAId = marsDMAReq(DMA_REQ_SD2_WRITE, &dmaCfg);
    
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

    OSSemPend(smcSemEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcSemEvt is %d.\n", err);
        return 0;
    }

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

    OSSemPend(smcSemEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcSemEvt is %d.\n", err);
        return 0;
    }

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
        return 0;

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

    OSSemPend(smcSemEvt, SMC_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SMC("Error: smcSemEvt is %d.\n", err);
        return 0;
    }

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
#if 1
    u8 makerCode, deviceCode;
    u32 stat = 1;

    smcIdRead(&makerCode, &deviceCode);
    switch (deviceCode)
    {
    case 0x79:      /* 1 Gb = 128 MB */
        smcTotalSize = 0x08000000; 	/* 128 MB */
        smcBlockSize = 0x00004000;	/* 16 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 8;		/* 8 zone */
        smcAddrCycle = 4;		/* 4 address cycle */
        dcfStorageSize[STORAGE_MEMORY_SMC_NAND] = FS_MEDIA_SMC_128MB;	/*CY 0718*/
        DEBUG_SMC("Trace: SMC 128MB.\n");
        break;

    case 0x76:      /* 512 Gb = 64 MB */
        smcTotalSize = 0x04000000; 	/* 64 MB */
        smcBlockSize = 0x00004000;	/* 16 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 4;		/* 4 zone */
        smcAddrCycle = 4;		/* 4 address cycle */
        dcfStorageSize[STORAGE_MEMORY_SMC_NAND] = FS_MEDIA_SMC_64MB;	/*CY 0718*/
        DEBUG_SMC("Trace: SMC 64MB.\n");
        break;

    case 0x75:    	/* 256 Gb = 32 MB */
        smcTotalSize = 0x02000000; 	/* 32 MB */
        smcBlockSize = 0x00004000;	/* 16 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 2;		/* 2 zone */
        smcAddrCycle = 3;		/* 3 address cycle */
        dcfStorageSize[STORAGE_MEMORY_SMC_NAND] = FS_MEDIA_SMC_32MB;	/*CY 0718*/
        DEBUG_SMC("Trace: SMC 32MB.\n");
        break;

    case 0x73:      /* 128 Mb = 16 MB */
        smcTotalSize = 0x01000000; 	/* 16 MB */
        smcBlockSize = 0x00004000;	/* 16 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 1;		/* 1 zone */
        smcAddrCycle = 3;		/* 3 address cycle */
        dcfStorageSize[STORAGE_MEMORY_SMC_NAND] = FS_MEDIA_SMC_16MB;	/*CY 0718*/
        DEBUG_SMC("Trace: SMC 16MB.\n");
        break;

    case 0xe6:      /* 64 Mb = 8 MB */
        smcTotalSize = 0x00800000; 	/* 8 MB */
        smcBlockSize = 0x00002000;	/* 8 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 1;		/* 1 zone */
        smcAddrCycle = 3;		/* 3 address cycle */
        dcfStorageSize[STORAGE_MEMORY_SMC_NAND] = FS_MEDIA_SMC_8MB;	/*CY 0718*/
        DEBUG_SMC("Trace: SMC 8MB.\n");
        break;

    case 0x6b:      /* 32 Mb = 4 MB */
    case 0xe3:
    case 0xe5:
        smcTotalSize = 0x00400000; 	/* 4 MB */
        smcBlockSize = 0x00002000;	/* 8 KB */
        smcPageSize  = 0x00000200;	/* 512 B */
        smcPageRedunSize = 0x00000010;	/* 16 B */
        smcTotalZone = 1;		/* 1 zone */
        smcAddrCycle = 3;		/* 3 address cycle */
        dcfStorageSize[STORAGE_MEMORY_SMC_NAND] = FS_MEDIA_SMC_4MB;	/*CY 0718*/
        DEBUG_SMC("Trace: SMC 4MB.\n");
        break;

    default:	/* unknown device code */
        stat = 0;
        DEBUG_SMC("Trace: SMC unknown size.\n");
        break;
    }

    if (stat)
    {
        smcTotalPageCount = smcTotalSize / smcPageSize;		/* used for file system sector management */
        smcPagePerBlock = smcBlockSize / smcPageSize;
        smcSecPerBlock = smcBlockSize / FS_SECTOR_SIZE;
        /*elsa mask, can't build*/
        //smcZoneSize = SMC_MAX_ZONE_ENTRY * smcBlockSize;

    }
    return stat;

#else
    smcTotalSize= SMC_TOTAL_SIZE;
    smcBlockSize= SMC_BLOCK_SIZE;
    smcPageSize=SMC_MAX_PAGE_SIZE;
    smcPagePerBlock= SMC_MAX_PAGE_PER_BLOCK;
    smcPageRedunSize=SMC_MAX_REDUN_SIZE;
    smcAddrCycle=SMC_ADDR_CYCLE;
    smcTotalPageCount=SMC_TOTAL_SIZE/SMC_MAX_PAGE_SIZE;
    dcfStorageSize[STORAGE_MEMORY_SMC_NAND] = FS_MEDIA_SMC_16MB;
    return 1;

#endif
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
    SmcCfg = SMC_CFG_CMD_ONLY | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_DISA; // | SMC_CFG_ADDR_1CYCL
    SmcCmd = SMC_CMD_RESET;
    //SmcAddr = 0;
    SmcCtrl = SMC_CTRL_OP_START;

    if (smcCheckOperationReady() != 1)
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
    SmcCfg = SMC_CFG_READ | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_ENA | SMC_CFG_DATA_LEN_512B;
    SmcCmd = SMC_CMD_READ_1A;
    smcSetReadWriteAddr(addr);
    SmcCtrl = SMC_CTRL_OP_START;

    /* set read data dma */
    smcSetReadDataDma(dataBuf, smcPageSize);

    if (smcCheckReadReady() != 1)
        return 0;

    /* get redundant area */
    smcGetRedunArea(&smcRedunArea);

    if ((smcRedunArea.data_status != 0xff) ||(smcRedunArea.block_status != 0xff))
        return 0;

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
    SmcCfg = SMC_CFG_READ | SMC_CFG_NAND | SMC_CFG_WP_HI | SMC_CFG_ECC_ENA | SMC_CFG_DATA_LEN_512B;
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
    u8* dataBuf=smcReadBuf;

    return(smcPage1ARead(addr, dataBuf));
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
    u8 Stat;

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

    //if (SmcStat & SMC_SR_FAIL)
    //	return 0;

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

	Bad Block Management init.
	Fill the Reserved Block Address
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

#if 0
/*

Routine Description:

	Bad Block Management
	Marked Specify address as bad block
*/

u8 BBM_Handle(u32 addr,u8* dataBuf)
{
    u32 Page_offset;

    if (SMC_BBM.b_index>SMC_BBM.index)
        return 0;

    Page_offset = addr % smcBlockSize;		// offset of page in a block
    Page_offset= Page_offset /smcPageSize;

    if (Page_offset==0)
    {
        SMC_BBM.Bad_Block_addr[SMC_BBM.b_index]=addr;
        smcPageProgram(SMC_BBM.New_Block_addr[SMC_BBM.b_index],dataBuf,0xFFFF);     // Program to another good block
        SMC_BBM.b_index++;
        SMC_BBM.bbm_flag=1;
        smcPageProgram(addr,dataBuf,0xE8E8);        //Mark fail in original block
        SMC_BBM.bbm_flag=0;
    }
    else
    {}



}
#endif
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
        if (addr>= SMC_MAP_ADDR)
#endif
            smcBlockErase(addr);
    }
    Rerseved_Algo_Start=bad_flag;
    //Rerseved_Algo_Start=0;
//    return bad_flag;
}
#if 1
s32 smcCheckTotalBlock()
{
    /* cytsai: 0315 */
    u32 addr;
    u32 start_addr=0;
    u16 i=0,j=0;
    u32 bad_flag=0;

    smcBBM_init();

    for (addr=SMC_RESERVED_SIZE; addr<smcTotalSize; addr+=smcBlockSize)
    {
        smcPage2CRead(addr);
        if (smcRedunArea.block_status != 0xff)
        {
            DEBUG_SMC("SMC TRACE: Bad Block: %x \n",addr);
            bad_flag=1;
            if (i>SMC_BBM.index)
            {
                DEBUG_SMC("SMC ERR: Bad Block too much \n");     // need err handle or UI error message
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
#endif
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
    SmcIntMask = SMC_INT_OP_CMPL_ENA; // | SMC_INT_READ_CMPL_ENA; /*CY 0907*/

    /* Set timing */
    /*CY 0907*/
    /* Set clock divisor to make NAND gate flash read / write pulse greater than 50ns (20MHz). */
    if (SYS_CPU_CLK_FREQ > 60000000)
    {	/* System clock won't be greater than 80MHz. */
        tRP = tRH = tWP = tWH = 1;
    }
    else
    {
        tRP = tWP = 1;
        tRH = tWH = 0;
    }
    SmcTimeCtrl = tRP | (tRH << 4) | (tWP << 12) | (tWH << 16); /*CY 0907*/

    /* Create the semaphore */
    if (smcSemEvt == NULL) /*CY 1023*/
        smcSemEvt = OSSemCreate(0);
    //smcReadWriteSemEvt = OSSemCreate(0); /*CY 0907*/

#if SMC_USE_LB_WRITE_CACHE /*CY 0907*/
    //smcCacheClear();
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

    BitMap_return=smcMakeBitMap(1);
    make_BitMap_ok=1;

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

    /* check if operation completion is set */
    if (intStat & SMC_INT_OP_CMPL)
    {
        /* Operation cycle is completed */
        OSSemPost(smcSemEvt);
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
    for (j = 0; j < (SMC_MAX_PAGE_SIZE/4)  ; j++)
    {

        if (*readPtr++ != j)
        {
            break;
        }
    }

//    DEBUG_SMC("[Pass]: Block Erase.\n");

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
	DEBUG_SMC("*     NAND Flash Test     *\n");
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


#endif
#endif
