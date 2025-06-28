/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    sdcfs.c

Abstract:

    The routines of MMC/SD related to file system.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include    "general.h"
#include    "osapi.h"
#include    "fsapi.h"
#include    "sdcapi.h"
#include    "smcapi.h"
#include    "sdcerr.h"
#include    "dcfapi.h"

/*
 *********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */



extern u32 sdcMmc; /* cytsai */
extern u32 sdcTotalBlockCount; /* cytsai */
extern u8 ucSpec2Dot0; /* 0: 1.1 or earlier, 1: 2.0 or later */
extern int sysStorageOnlineStat[];

/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

/* Driver Function */
extern s32 sdcDetectCard(void);

/* Middleware Function */
extern s32 sdcCardIdentificationMode(void);
extern s32 sdcGetCsd(void);
extern s32 sdcDataTransferMode(u32);
extern s32 sdcErase(u32, u32);
extern s32 sdcReadSingleBlock(u32, u8*);
extern s32 sdcReadMultipleBlock(u32, u32, u8*);
extern s32 sdcWriteSingleBlock(u32, u8*);
extern s32 sdcWriteMultipleBlock(u32, u32, u8*);
extern s32 sdcGetCardStatus(u32*);
extern s32 sdcCheckTransferState(void);

extern s32 mmcCardIdentificationMode(void);
extern s32 mmcGetCsd(void);
extern s32 mmcDataTransferMode(u32);
extern s32 mmcErase(u32, u32);
extern s32 mmcReadSingleBlock(u32, u8*);
extern s32 mmcReadMultipleBlock(u32, u32, u8*);
extern s32 mmcWriteSingleBlock(u32, u8*);
extern s32 mmcWriteMultipleBlock(u32, u32, u8*);
extern s32 mmcGetCardStatus(u32*);
extern s32 mmcCheckTransferState(void);

/*
 *********************************************************************************************************
 *                                                Application Function
 *********************************************************************************************************
 */

/*

Routine Description:

    FS driver function. Get status of SD/MMC.

Arguments:

    Unit - The unit number.

Return Value:

    1 (FS_LBL_MEDIACHANGED) - The media of the device has changed.
    0                       - Device okay and ready for operation.
    < 0                     - An error has occured.

*/
int sdcDevStatus(u32 Unit)
{
    u32 ret;
    int DevIdx;

    if (Unit != 0)
    {
        DEBUG_SDC("SDC Err: Unit=%d\n",Unit);
        return -1; /* Invalid unit number */
    }

    DevIdx=DCF_GetDeviceIndex("sdmmc");


    if (!sysStorageOnlineStat[DevIdx])
    {
        /*
           Make sure, the function returns FS_LBL_MEDIACHANGED when it is
           called the first time
        */
        sysStorageOnlineStat[DevIdx] = 1;
        return FS_LBL_MEDIACHANGED;
    }

    if (sdcDetectCard() != SD_OK)
    {
        DEBUG_SDC("SDC Err: sdcDetectCard\n");
        return -1;
    }

    if (sdcMmc == 1)
        ret = sdcCheckTransferState();
    else
        ret = mmcCheckTransferState();

    if (sdcDetectCard() != SD_OK)
    {
        DEBUG_SDC("SDC Err: sdcDetectCard \n");
        return -1;
    }
    if (ret != SD_OK)
    {
        DEBUG_SDC("SDC Err: sdcCheckTransferState=%d \n",ret);
        DEBUG_SDC("SDC error, Re-Mount again #1!\n");
        if (sdcMount() < 0)
            return -1;
    }
    /*CY 0601 E*/

    return 0;
}

/*

Routine Description:

    FS driver function. Read a sector from SD/MMC.

Arguments:

    Unit - The unit number.
    Sector - The sector to be read from the device.
    pBuffer - The pointer to buffer for storing the data.

Return Value:

    0   - The sector has been read and copied to pBuffer.
    < 0 - An error has occured.

*/
int sdcDevRead(u32 Unit, u32 Sector, void *pBuffer)
{
#if SDC_FS_SIN_TO_MUL_WRITE
    return sdcDevMulRead(Unit, Sector, 1, pBuffer);
#else
    int retValue = 0;
    //DEBUG_SDC("sdcDevRead Sector %x\n",Sector);
    if (Unit != 0)
    {
        DEBUG_SDC("SDC Err: Unit=%d\n",Unit);
        return -1; /* Invalid unit number */
    }
    if (Sector >= sdcTotalBlockCount)
    {
        DEBUG_SDC("SDC Read Err: Sector=%d,sdcTotalBlockCount=%d\n",Sector,sdcTotalBlockCount);
        return -1;
    }

    if (sdcDetectCard() != SD_OK)
    {
        DEBUG_SDC("SDC Err: sdcDetectCard \n");
        return -1;
    }
    if (sdcMmc == 1)
    {
#if SD_SPEC_2DOT0
        if (ucSpec2Dot0)
            retValue = sdcReadSingleBlock(Sector, ((u8 *)pBuffer));
        else
#endif
            retValue = sdcReadSingleBlock(Sector * FS_SECTOR_SIZE, ((u8*)pBuffer));        /*CY 1023*/
#if 1
        if (retValue < SD_OK)
        {
            DEBUG_SDC("SDC Err: sdcReadSingleBlock=0x%x \n",retValue);
            return retValue;
        }
#endif
        //sdcReadMultipleBlock(1, Sector * FS_SECTOR_SIZE, ((u8 *) pBuffer));   /*CY 1023*/
    }
    else
    {
        mmcReadSingleBlock(Sector * FS_SECTOR_SIZE, ((u8*)pBuffer));        /*CY 1023*/
        //mmcReadMultipleBlock(1, Sector * FS_SECTOR_SIZE, ((u8 *) pBuffer));   /*CY 1023*/
    }

    return 0;
#endif
}

/*

Routine Description:

    FS driver function. Read a sector from SD/MMC.

Arguments:

    Unit - The unit number.
    Sector - The sector to be read from the device.
    pBuffer - The pointer to buffer for storing the data.

Return Value:

    0   - The sector has been read and copied to pBuffer.
    < 0 - An error has occured.

*/
int sdcDevMulRead(u32 Unit, u32 Sector, u32 NumofSector, void *pBuffer)
{
    int retValue = 0;
    //DEBUG_SDC("sdcDevMulRead %d",Sector);
    //DEBUG_SDC("     NumofSector %d \n",NumofSector);
    if (Unit != 0)
    {
        return -1; /* Invalid unit number */
    }
    if (Sector >= sdcTotalBlockCount)
    {
        DEBUG_SDC("SDC MulRead Err: Sector=%d,sdcTotalBlockCount=%d\n",Sector,sdcTotalBlockCount);
        return -1; /* Out of physical range */
    }

    if (sdcDetectCard() != SD_OK)
        return -1;

    if (sdcMmc == 1)
    {
#if SD_SPEC_2DOT0
        if (ucSpec2Dot0)
            retValue = sdcReadMultipleBlock(NumofSector, Sector, ((u8 *)pBuffer));
        else
#endif
            retValue = sdcReadMultipleBlock(NumofSector, Sector * FS_SECTOR_SIZE, ((u8 *)pBuffer));

        if (retValue < SD_OK)
            return retValue;
    }
    else
    {
        mmcReadSingleBlock(Sector * FS_SECTOR_SIZE, ((u8*)pBuffer));        /*CY 1023*/
    }

    return 0;
}

/*

Routine Description:

    FS driver function. Write a sector to SD/MMC.

Arguments:

    Unit - The unit number.
    Sector - The sector to be read from the device.
    pBuffer - The pointer to data to be stored.

Return Value:

    0   - The sector has been written to the device.
    < 0 - An error has occured.

*/
int sdcDevWrite(u32 Unit, u32 Sector, void *pBuffer)
{
    int retValue = 0;


    if (Unit != 0)
    {
        return -1; /* Invalid unit number */
    }
    if (Sector >= sdcTotalBlockCount)
    {
        DEBUG_SDC("SDC Write Err: Sector=%d,sdcTotalBlockCount=%d\n",Sector,sdcTotalBlockCount);
        return -1; /* Out of physical range */
    }

    if (sdcDetectCard() != SD_OK)
        return -1;

    if (sdcMmc == 1)
    {

#if SD_SPEC_2DOT0
        if (ucSpec2Dot0)
            retValue = sdcWriteSingleBlock(Sector, ((u8 *)pBuffer));
        else
#endif
            retValue = sdcWriteSingleBlock(Sector * FS_SECTOR_SIZE, ((u8 *)pBuffer));
        if (retValue < SD_OK)
            return retValue;
    }
    else
    {
        mmcWriteSingleBlock(Sector * FS_SECTOR_SIZE, ((u8 *) pBuffer));     /*CY 1023*/
        //mmcWriteMultipleBlock(1, Sector * FS_SECTOR_SIZE, ((u8*)pBuffer));    /*CY 1023*/
    }

    return 0;
}

/*

Routine Description:

    FS driver function. Write a sector to SD/MMC.

Arguments:

    Unit - The unit number.
    Sector - The sector to be read from the device.
    pBuffer - The pointer to data to be stored.

Return Value:

    0   - The sector has been written to the device.
    < 0 - An error has occured.

*/
int sdcDevMulWrite(u32 Unit, u32 Sector, u32 NumofSector, void *pBuffer)
{
    u8 *pBuf = pBuffer;
    int retValue = 0;

    if (Unit != 0)
    {
        return -1; /* Invalid unit number */
    }

    if (Sector >= sdcTotalBlockCount)
    {
        DEBUG_SDC("SDC MULWrite Err: Sector=%d,sdcTotalBlockCount=%d\n",Sector,sdcTotalBlockCount);
        return -1; /* Out of physical range */
    }

    if (sdcDetectCard() != SD_OK)
        return -1;

    if (NumofSector==0)
        return 0;

    if (sdcMmc == 1)
    {
        //SD card
        if (NumofSector==1)
        {
            retValue =  sdcDevWrite( Unit,  Sector, pBuf);
        }
        else
        {
            while (NumofSector>256)
            {
#if SD_SPEC_2DOT0
                if (ucSpec2Dot0)
                    retValue = sdcWriteMultipleBlock(256, Sector, pBuf);
                else
#endif
                    retValue = sdcWriteMultipleBlock(256, Sector * SDC_BLK_LEN, pBuf);

#if SD_SPEC_2DOT0
                Sector +=256;
#else
                Sector += (256*SDC_BLK_LEN);
#endif

                pBuf += (256*SDC_BLK_LEN);
                NumofSector -=256;

                if (retValue < SD_OK)
                    return retValue;
            }

            if (NumofSector==1)
            {
                retValue =  sdcDevWrite( Unit,  Sector, pBuf);
            }
            else if (NumofSector >1)
            {
#if SD_SPEC_2DOT0
                if (ucSpec2Dot0)
                    retValue = sdcWriteMultipleBlock(NumofSector, Sector, pBuf);
                else
#endif
                    retValue = sdcWriteMultipleBlock(NumofSector, Sector * SDC_BLK_LEN, pBuf);
            }
        }
        if (retValue < SD_OK)
            return retValue;
    }
    else
    {
        //MMC card
        mmcWriteSingleBlock(Sector * FS_SECTOR_SIZE, pBuf);      /*CY 1023*/
    }

    return 0;
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
int sdcDevIoCtl(u32 Unit, s32 Cmd, s32 Aux, void *pBuffer)
{
    u32 *info;
    int DevIdx;

    Aux = Aux;  /* Get rid of compiler warning */
    DevIdx=DCF_GetDeviceIndex("sdmmc");

    if (Unit != 0)
    {
        return -1; /* Invalid unit number */
    }

    if (sdcDetectCard() != SD_OK)
    {
        return -1;
    }

    switch (Cmd)
    {
        case FS_CMD_GET_DEVINFO:
            if (!pBuffer)
            {
                return -1;
            }

            info = pBuffer;
            *info = 0;                  /* hidden */
            info++;
            *info = 2;                  /* head */
            info++;
            *info = 4;                  /* sec per track */
            info++;
            *info = sdcTotalBlockCount; /* total block count */
            break;

        case FS_CMD_FORMAT_MEDIA:
            /* Format the SD card */
#if SD_SPEC_2DOT0
            if (ucSpec2Dot0)
                sdcErase(0, sdcTotalBlockCount - 1);
            else
#endif
                sdcErase(0, (sdcTotalBlockCount - 1) * FS_SECTOR_SIZE);

            break;
        case FS_CMD_SET_STATUS:
            sysStorageOnlineStat[DevIdx] = Aux;
            break;

        default:
            break;
    }

    return 0;
}

int sdcErrorResultFilter(int funcValue)
{
	int ret;
	
	switch(funcValue)	
	{
		case SD_ERROR_DATA_RX_CRC_ERROR:
        case SD_ERROR_DMA_DATA_RX_ERROR:
        case SD_ERROR_DATA_RX_ERROR:
        	ret = -1;
        	break;
        default: 
        	ret = 1;
        	break;
	}
	return ret;
}
