/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	smcfs.c

Abstract:

   	The routines of SMC and NAND gate flash erlated to file system.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include	"general.h"

#if (FLASH_OPTION == FLASH_NAND_9002_ADV)

#include	"osapi.h"
#include	"fsapi.h"
#include	"smcapi.h"
#include	"smcwc.h"
#include	"uiapi.h"
#include    "dcfapi.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */




extern u32 smcTotalSize;
extern u32 smcTotalPageCount;	/* cytsai */
extern u8*	smcGeneralBuf;

extern u32 smcTotalSize, smcBlockSize, smcPageSize,smcPagePerBlock;
extern u8 UI_update;		//civic 071001
//extern u8 UItempBuf[512];		//civic 071001
extern u8 iconflag[UIACTIONNUM];
extern u8 start_iconflag[UIACTIONNUM];
extern int sysStorageOnlineStat[];
extern u8 Host_Cmd_Format;
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

/* Middleware Function */
extern s32 smcIdentification(void);
extern s32 smcReset(void);
extern s32 smcReadStatus(u8*);
extern s32 smcIdRead(u8*, u8*);
extern s32 smcPage1ARead(u32, u8*);
extern s32 smcPage1BRead(u32, u8*);
extern s32 smcPage2CRead(u32);
extern s32 smcPageProgram(u32, u8*, u16);
extern s32 smcBlockErase(u32);
extern s32 smcMultiBlockErase(u32, u32);
/* Map Function */
extern s32 smcSectorsRead(u32, u32, u8*);
extern s32 smcSectorsWrite(u32, u32, u32, u8*);
extern s32 smcPagesWrite(u32, u32, u8*);
extern void smcTotalBlockErase(char );
extern u8 smcMakeBitMap(char);

extern u8 userClickFormat;

extern s32 siuWBComp_RBRatio[4];
/*
 *********************************************************************************************************
 * Application function
 *********************************************************************************************************
 */

/*

Routine Description:

	FS driver function. Get status of SMC/NAND gate flash.

Arguments:

	Unit - The unit number.

Return Value:

	1 (FS_LBL_MEDIACHANGED) - The media of the device has changed.
	0                       - Device okay and ready for operation.
	< 0                     - An error has occured.

*/
int smcDevStatus(u32 Unit)
{
    /*CY 0601 S*/
    int DevIdx;

    if (Unit != 0)
    {
        return -1;  /* Invalid unit number */
    }
    DevIdx=DCF_GetDeviceIndex("smc");

    if (!sysStorageOnlineStat[DevIdx])
    {
        /*
         		   Make sure, the function returns FS_LBL_MEDIACHANGED when it is
          	   called the first time
        */
        sysStorageOnlineStat[DevIdx] = 1;
        return FS_LBL_MEDIACHANGED;
    }
    /*CY 0601 E*/

    return 0;
}


/*

Routine Description:

	FS driver function. Read a sector from SMC/NAND gate flash.

Arguments:

	Unit - The unit number.
	Sector - The sector to be read from the device.
	pBuffer - The pointer to buffer for storing the data.

Return Value:

	0   - The sector has been read and copied to pBuffer.
	< 0 - An error has occured.

*/
int smcDevRead(u32 Unit, u32 Sector, void *pBuffer)
{
    int status = -1;
    char* buffer;
    u8* MbrBuf = smcMBRCache;

    buffer = (char *)pBuffer;

#ifdef FAT_READ_DEBUG
    DEBUG_SMC("------smcDevRead------\n");
    DEBUG_SMC("DEBUG: smcDevRead Sector %x\n",Sector);
    DEBUG_SMC("DEBUG: smcDevRead Unit %d\n",Unit);
#endif
    if (Sector == 0)		//Read BPB request then fake the BPB
    {
        memcpy(buffer, MbrBuf, FS_SECTOR_SIZE);
        return 0;
    }


    if (Sector < (smcTotalSize / FS_SECTOR_SIZE))
        if (smcSectorsRead(1, Sector, (u8*)buffer))
            status = 0;
        else
            status =-1;

#ifdef FAT_READ_DEBUG
    DEBUG_SMC("======smcDevRead======\n");
#endif

    return status;
}

/*

Routine Description:

	FS driver function. Read multi-sectors from SMC/NAND gate flash.

Arguments:

	Unit - The unit number.
	unSectorPos - The sector position to be read from the device.
	unNumOfSectors - Sector numbers to read.
	pBuffer - The pointer to buffer for storing the data.

Return Value:

	0   - The sector has been read and copied to pBuffer.
	< 0 - An error has occured.

*/
int smcDevMultiRead(u32 Unit, u32 unSectorPos, u32 unNumOfSectors, void *pBuffer)
{
    int status = 0;
    char* pucBuffer;
    u8* MbrBuf = smcMBRCache;

    pucBuffer = (char *)pBuffer;

    if (unSectorPos == 0)		//Read BPB request then fake the BPB
    {
        memcpy(pucBuffer, MbrBuf, smcPageSize);
        return 0;
    }

    if (unSectorPos < ((smcTotalSize - SMC_FAT_START_ADDR)/FS_SECTOR_SIZE))
    {
        if (smcSectorsRead(unNumOfSectors, unSectorPos, pucBuffer) == 0)
            status = -1;
    }

    if (status == -1)	/* cytsai: 0315 */
    {
        status = status;
    }

    return status;

}

/*

Routine Description:

	FS driver function. Write a sector to SMC/NAND gate flash.

Arguments:

	Unit - The unit number.
	Sector - The sector to write to the device.
	pBuffer - The pointer to data to write to the device.

Return Value:

	0   - The sector has been written to the device.
	< 0 - An error has occured.

*/
int smcDevWrite(u32 Unit, u32 SectorNum, void *pBuffer)
{
    int status = -1;
    u8 *temp = (u8*)pBuffer;
    u8* MbrBuf = smcMBRCache;

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("------------smcDevWrite------------\n");
#endif
    /* Sectot 0 stores the MBR */
    if (SectorNum==0)
    {
        memcpy(MbrBuf, temp, smcPageSize);
        if (Host_Cmd_Format==1 && (MbrBuf[0]!=0xE9 || MbrBuf[0]!=0xEB))     // Mass-storage execute format instruction
        {
            // When host execute format instruction  -- civic
            // 1. Write MBR with magic  number
            // 2. Write last sector with zero padding
            // 3. Clear FAT1 area with 0xF8FFFF (FAT12)
            // 4. Clear FAT2 area with 0xF8FFFF (FAT12)
            // 5. Clear All FDB sector
            // 6. Write MBR until FAT start sector
            smcTotalBlockErase(0); // receive magic number then erase all block
            // Clear BitMap information
            memset((void*)smcBitMap, 0xFF,SMC_MAX_MAP_SIZE_IN_BYTE);

            Host_Cmd_Format=2;
#ifdef PRINT_MBR
            {
                u32 j;
                for (j=1; j<513; j++)
                {
                    DEBUG_SMC("%x ",MbrBuf[j-1]);
                    if (j%16==0)
                        DEBUG_SMC("\n");
                }
                DEBUG_SMC("---------------------\n");
            }
#endif
        }
        // MBR change then re-calculate all FAT address
        if (Host_Cmd_Format==3 && (MbrBuf[0]==0xE9 || MbrBuf[0]==0xEB))     // Mass-storage execute format instruction
        {
            Host_Cmd_Format=0;
            // Get the new reserved sector
            NAND_FAT_PARAMETER.RsvdSecCnt = MbrBuf[0x0E] + (MbrBuf[0x0F]<<8);
            // Get the new FAT parameter
            NAND_FAT_PARAMETER.BytesPerSec = MbrBuf[0x0B] + (MbrBuf[0x0C]<<8);
            NAND_FAT_PARAMETER.FATSz16 = MbrBuf[0x16] + (MbrBuf[0x17]<<8);
            NAND_FAT_PARAMETER.NumFATs = MbrBuf[0x10];
            NAND_FAT_PARAMETER.RootEntCnt = MbrBuf[0x11] + (MbrBuf[0x12]<<8);
            NAND_FAT_PARAMETER.SecPerClus = MbrBuf[0x0D];    // Important parameter it will affect NAND behavior
            NAND_FAT_PARAMETER.TotSec16= MbrBuf[0x13] + (MbrBuf[0x14]<<8);
            // calculate the essential NAND FAT cache parameter
            nand_fat_start =  NAND_FAT_PARAMETER.RsvdSecCnt;
            nand_fat_end = nand_fat_start + NAND_FAT_PARAMETER.NumFATs*NAND_FAT_PARAMETER.FATSz16;
            nand_fat_size = NAND_FAT_PARAMETER.FATSz16;

            /* MBR occupied one sector (512 Bytes) */
            if (smcSectorsWrite(1, 0, SMC_FAT_START_ADDR, (u8*)MbrBuf)==0)
            {
                DEBUG_SMC("Write MBR error \n");
            }
            //don't need to DCF file system initial because device can't operate while pluging USB in V5O
        }
        return 0;
    }
    // Add Host_Cmd_Format!=2 because when host format start, we will know the MBR parameter after wrtie FAT cache.

    //addr=Sector * FS_SECTOR_SIZE+ SMC_FAT_START_ADDR;
    //addr = addr & 0xFFFFFD00;
    if (SectorNum < (smcTotalSize/FS_SECTOR_SIZE))
        if (smcSectorsWrite(1, SectorNum, SMC_FAT_START_ADDR, (u8*)temp))
            status = 0;

    if (status == -1)	/* cytsai: 0315 */
    {
        status = status;
    }

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("============smcDevWrite============\n");
#endif
    return status;
}


/*

Routine Description:

	FS driver function. Write multi sector to SMC/NAND gate flash.

Arguments:

	Unit - The unit number.
	Sector - The sector to be read from the device.
	pBuffer - The pointer to data to be stored.
	SectorCount - The total sector count to write to.

Return Value:

	0   - The sector has been written to the device.
	< 0 - An error has occured.

*/
int smcDevMultiWrite(u32 Unit, u32 SectorNum, u32 SectorCount, void *pBuffer)
{
    u32	status = 0;
    u8* MbrBuf = smcMBRCache;
    u8* temp;

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("------------smcDevMultiWrite------------\n");
#endif

    temp = (u8*)pBuffer;


    if (SectorNum == 0)
    {
        memcpy(MbrBuf, temp,smcPageSize);
        temp += smcPageSize;
        SectorCount --;
        if (Host_Cmd_Format==1 && (MbrBuf[0]!=0xE9 || MbrBuf[0]!=0xEB))     // Mass-storage execute format instruction
        {
            // When host execute format instruction  -- civic
            // 1. Write MBR with magic  number
            // 2. Write last sector with zero padding
            // 3. Clear FAT1 area with 0xF8FFFF (FAT12)
            // 4. Clear FAT2 area with 0xF8FFFF (FAT12)
            // 5. Clear All FDB sector
            // 6. Write MBR until FAT start sector
            smcTotalBlockErase(0); // receive magic number then erase all block
            // Clear BitMap information
            memset((void*)smcBitMap, 0xFF, SMC_MAX_MAP_SIZE_IN_BYTE);

            Host_Cmd_Format=2;
#ifdef PRINT_MBR
            {
                u32	j;
                for (j=1; j<513; j++)
                {
                    DEBUG_SMC("%x ",MbrBuf[j-1]);
                    if (j%16 == 0)
                        DEBUG_SMC("\n");
                }
                DEBUG_SMC("---------------------\n");
            }
#endif
        }
        // MBR change then re-calculate all FAT address
        if (Host_Cmd_Format==3 && (MbrBuf[0]==0xE9 || MbrBuf[0]==0xEB))     // Mass-storage execute format instruction
        {
            Host_Cmd_Format=0;
            // Get the new reserved sector
            NAND_FAT_PARAMETER.RsvdSecCnt = MbrBuf[14] + (MbrBuf[15]<<8);
            // Get the new FAT parameter
            NAND_FAT_PARAMETER.BytesPerSec = MbrBuf[11] + (MbrBuf[12]<<8);
            NAND_FAT_PARAMETER.FATSz16 = MbrBuf[22] + (MbrBuf[23]<<8);
            NAND_FAT_PARAMETER.NumFATs = MbrBuf[16];
            NAND_FAT_PARAMETER.RootEntCnt = MbrBuf[17] + (MbrBuf[18]<<8);
            NAND_FAT_PARAMETER.SecPerClus = MbrBuf[13];    // Important parameter it will affect NAND behavior
            NAND_FAT_PARAMETER.TotSec16= MbrBuf[19] + (MbrBuf[20]<<8);
            // calculate the essential NAND FAT cache parameter
            nand_fat_start =  NAND_FAT_PARAMETER.RsvdSecCnt;
            nand_fat_end = nand_fat_start + NAND_FAT_PARAMETER.NumFATs*NAND_FAT_PARAMETER.FATSz16;
            nand_fat_size = NAND_FAT_PARAMETER.FATSz16;
            //addr=Sector * FS_SECTOR_SIZE+ SMC_FAT_START_ADDR;
            //addr = addr & 0xFFFFFD00;
            if (smcSectorsWrite(1, 0, SMC_FAT_START_ADDR, (u8*)MbrBuf)==0)
            {
                DEBUG_SMC("Write MBR error \n");
            }
            //don't need to DCF file system initial because device can't operate while pluging USB in V5O
        }
    }



    if (SectorNum < (smcTotalSize/FS_SECTOR_SIZE))
    {
        if (smcSectorsWrite(SectorCount, SectorNum, SMC_FAT_START_ADDR, (u8*)temp)==0)
            status = -1;
    }


    if (status == -1)	/* cytsai: 0315 */
    {
        status = status;
    }

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("============smcDevMultiWrite============\n");
#endif

    return status;
}


/*

Routine Description:

	FS driver function. Write a sector to SMC/NAND gate flash.

Arguments:

	unEraseBlkAddr - The block address to erase.
	unStartSecNum - The start sector number.
	unEndSecNum - The end sector number.

Return Value:

	0 - Failure
	1 - Success.

*/
int smcErase(u32 unEraseBlkAddr, u32 unStartSecNum, u32 unEndSecNum)
{
    u8* read_buf = smcGeneralBuf;
    u32	unWriteSecCnt;

    unWriteSecCnt = unEndSecNum - unStartSecNum;

    if (smcSectorsRead(unWriteSecCnt, unStartSecNum, read_buf) == 0)
    {
        DEBUG_SMC("Error! Read error on block address %#x", unEraseBlkAddr);
        return 0;
    }

    smcBlockErase(unEraseBlkAddr);
    read_buf= smcGeneralBuf;

    if (smcSectorsWrite(unWriteSecCnt, unStartSecNum, SMC_FAT_START_ADDR, read_buf) == 0)
    {
        DEBUG_SMC("Error! Read error on block address %#x", unEraseBlkAddr);
        return 0;
    }

    /* Won't need to update the Bit Map data */
    return 1;
}

/*

Routine Description:

	FS driver function. Execute device command.

Arguments:

	Unit - The unit number.
	Cmd - The command to be executed.
	Aux - The parameter depending on command.
	pBuffer - The pointer to a buffer used for the command.

Return Value:

	Command specific. In general a negative value means an error.

*/
int smcDevIoCtl(u32 Unit, s32 Cmd, s32 Aux, void *pBuffer)
{
    u32 *info;
    u32 val;
    int DevIdx;

#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("---------smcDevIoCtl---------\n");
#endif
    DevIdx=DCF_GetDeviceIndex("smc");

    Aux = Aux;  /* Get rid of compiler warning */
    if (Unit != 0)
    {
        return -1;  /* Invalid unit number */
    }
    switch (Cmd)
    {
    case FS_CMD_GET_DEVINFO:
        if (!pBuffer)
        {
            return -1;
        }
        info = pBuffer;

        *info = 0;			/* hidden */
        info++;

        if (smcTotalSize >= 0x08000000)	/* heads/track*/
            val = 16; /* >= 128MB */
        else if (smcTotalSize >= 0x02000000)
            val = 8;  /* >= 32MB */
        else
            val = 4;  /* < 32MB */
        *info = val;
        info++;

        if (smcTotalSize >= 0x04000000)	/* sectors/head */
            val = 32; /* >= 64MB */
        else if (smcTotalSize >= 0x00800000)
            val = 16; /* >= 8MB */
        else if (smcTotalSize >= 0x00200000)
            val = 8;  /* >= 2MB */
        else
            val = 4;  /* < 2MB */
        *info = val;
        info++;

        *info = smcTotalPageCount * (smcPageSize / FS_SECTOR_SIZE);	/* total sector count */

        break;
    case FS_CMD_FORMAT_MEDIA:
        /* Format the NAND */
        //smcTotalBlockErase();
        smcMakeBitMap(0);
        sysStorageOnlineStat[DevIdx] = 0;
        

        break;
    default:
        break;
    }
#ifdef FAT_WRITE_DEBUG
    DEBUG_SMC("=========smcDevIoCtl=========\n");
#endif
    return 0;
}
#if 1
char smcWriteBackUISetting(void)
{
    u32 WB_logAddr =  SMC_SYS_PARAMETER_ADDR + SMC_UI_SECTOR_ADDR*SMC_MAX_PAGE_SIZE;// Reservese size

    memset((char*)smcReadBuf,0x00,smcPageSize);
    memcpy ((void *)smcReadBuf,(void *)iconflag,  UIACTIONNUM);
    if (memcmp((void*)start_iconflag, (void*)iconflag, UIACTIONNUM) != 0)
    {    // user changed setting then saved it
#if 0
        if (smcSectorsWrite(1,WB_logAddr,smcReadBuf)==0)
#else
        if (smcPagesWrite(1,WB_logAddr,smcReadBuf)==0)
#endif
        {
            DEBUG_SMC("Write error on block byte address %#x\n",WB_logAddr);
            return 0;
        }
        else
        {
            DEBUG_SMC("Write UI Setting Okay \n");
        }
        //smcSectorsRead(1,WB_logAddr,UItempBuf);
    }
    else
    {
        DEBUG_SMC("Don't need to Write UI Setting \n");
        return 2;
    }
    return 1;
}
#endif

char smcWriteBackAWBSetting(void)
{
    u32 WB_logAddr =  SMC_SYS_PARAMETER_ADDR + SMC_AWB_SECTOR_ADDR*SMC_MAX_PAGE_SIZE;// Reservese size

    memset((char*)smcReadBuf,0x00,smcPageSize);
    memcpy ((void *)smcReadBuf,(void *)siuWBComp_RBRatio,  16);

#if 0
    if (smcSectorsWrite(SMC_AWB_SECTOR_SIZE,WB_logAddr,smcReadBuf)==0)
#else
    if (smcPagesWrite(SMC_AWB_SECTOR_SIZE, WB_logAddr, smcReadBuf) == 0)
#endif
    {
        DEBUG_SMC("Write AWB error on block byte address %#x", WB_logAddr);
        return 0;
    }
    else
    {
        DEBUG_SMC("Write AWB Setting Okay \n");
    }

    return 1;
}
s8 smcWriteBackFAT()
{
    u8* BitMapBuf=smcBitMap;

    //Write-back Bit-Map
#if 0
    if (smcSectorsWrite(SMC_MAX_MAP_SIZE_IN_PAGE,SMC_MAP_ADDR,BitMapBuf)==0)
#else
    if (smcPagesWrite(SMC_MAX_MAP_SIZE_IN_PAGE, SMC_MAP_ADDR, BitMapBuf) == 0)
#endif
    {
        DEBUG_SMC("Write error on block byte address %#x\n",SMC_MAP_ADDR);
        return 0;
    }


    DEBUG_SMC("Write FAT Cache okay \n");
    return 1;
}
#endif
