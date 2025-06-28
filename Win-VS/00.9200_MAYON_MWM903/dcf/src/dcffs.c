/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    dcffs.c

Abstract:

    The routines of DCF file sysem related.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "asfapi.h"
#include "board.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "siuapi.h"
#include "gpioapi.h"
#include "sysapi.h"
#include "iisapi.h"
#include "GlobalVariable.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
DEF_FILEREPAIR_INFO dcfBadFileInfo;
s8 dcfCurDrive[64];     /*CY 1023*/
s8 dcfTargetPath[64];   /*CY 1023*/

s8 dcfBackupDrive[64];     /*CY 1023*/
s8 dcfBackupTargetPath[64];   /*CY 1023*/

//-----//
s8 dcfCurDir[64];   /*CY 1023*/
s8 dcfCurPath[64];  /*CY 1023*/

s8 dcfBackupDir[64];   /*CY 1023*/
s8 dcfBackupPath[64];  /*CY 1023*/

//-------//
s8 dcfDelDir[64];
s8 dcfDelPath[64];

s8 dcfPlayDir[64];
s8 dcfPlayPath[64];

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
s8 dcfLogDir[64];
s8 dcfLogPath[64];
#endif

u32 dcfStorageType;     /*CY 1023*/
s32 dcfStorageSize[STORAGE_MEMORY_MAX]; /*CY 0718 */
u32 dcfBufRemain; /* VCC */

#if DCF_WRITE_STATISTIC
u32 dcfWriteAccum=0;
u32 dcfMpegBufRemain=MPEG4_MAX_BUF_SIZE;
#endif

u8  dcfWriteFromBuffer;

/*
 *********************************************************************************************************
 * Extern Variable
 *********************************************************************************************************
 */
extern u8 gInsertNAND;
extern u8 *dcfBuf;
//civic S
extern FS_DISKFREE_T global_diskInfo; //civic 070829

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
extern u8 make_BitMap_ok;
#endif

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
extern s8 gsLogDirName[9];
#endif

#if RX_SNAPSHOT_SUPPORT
extern s8 gsPhotoDirName[9];
#endif

extern u8 siuOpMode;
extern u8 RepairASF;
extern BOOLEAN MemoryFullFlag;
//civic E
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 dcfDumpDir(FS_DIRENT*, u32);
s32 dcfDumpBuf(u8*, u32);
s32 dcfDumpDriveInfo(FS_DISKFREE_T*);
int dcfDirExist(s8*);
int dcfCreateTargetPath(char *pDirName);
int dcfCreateBackupTargetPath(char *pDirName);

s32 dcfCreateCurPath(void);

s32 dcfDrive(s8*);
s32 dcfFormat(s8*);
s32 dcfWrite(FS_FILE*, u8*, u32, u32*);
s32 dcfRead(FS_FILE*, u8*, u32, u32*);
s32 dcfFindLastEofCluster(u32 StorageType);


#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
s32 dcfCreateLogTargetPath(s8* pDirName, s8* TargetPath);
#endif

/*
 *********************************************************************************************************
 * External Function
 *********************************************************************************************************
 */
extern s32 smcCacheBlockWrite(void); /*CY 1023*/

#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
extern char smcWriteBackFAT(void);
#endif


//Albert lee 20090609
extern void* memcpy_hw(void *dest, const void *src, u32 count);

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */
/*

Routine Description:

    Dump directory.

Arguments:

    pDirEnt - Directory list.
    dirCount - Directory count.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfFsInit(void)
{
    /*CY 1023*/
    switch (dcfStorageType)
    {
        case STORAGE_MEMORY_RAMDISK:
            strcpy((char*)dcfCurDrive, "ram:0:");
            strcpy((char*)dcfCurDir, "\\");
            strcpy((char*)dcfCurPath, "ram:0:\\");
            strcpy((char*)dcfTargetPath, "ram:0:\\");
            break;

        case STORAGE_MEMORY_SD_MMC:
            strcpy((char*)dcfCurDrive, "sdmmc:0:");
            strcpy((char*)dcfCurDir, "\\");
            strcpy((char*)dcfCurPath, "sdmmc:0:\\");
            strcpy((char*)dcfTargetPath, "sdmmc:0:\\");
            break;

        case STORAGE_MEMORY_SMC_NAND:
            strcpy((char*)dcfCurDrive, "smc:0:");
            strcpy((char*)dcfCurDir, "\\");
            strcpy((char*)dcfCurPath, "smc:0:\\");
            strcpy((char*)dcfTargetPath, "smc:0:\\");
            break;

#if USB_HOST_MASS_SUPPORT
        case STORAGE_MEMORY_USB_HOST:
            strcpy((char*)dcfCurDrive, "usbfs:0:");
            strcpy((char*)dcfCurDir, "\\");
            strcpy((char*)dcfCurPath, "usbfs:0:\\");
            strcpy((char*)dcfTargetPath, "usbfs:0:\\");
            break;

#endif

        default:
            return -1;
    }

    return 1;
}


s32 dcfBackupFsInit(u32 BackupType)
{
    /*CY 1023*/
    switch (BackupType)
    {
        case STORAGE_MEMORY_SD_MMC:
            strcpy((char*)dcfBackupDrive, "sdmmc:0:");
            strcpy((char*)dcfBackupDir, "\\");
            strcpy((char*)dcfBackupPath, "sdmmc:0:\\");
            strcpy((char*)dcfBackupTargetPath, "sdmmc:0:\\");
            break;

#if USB_HOST_MASS_SUPPORT
        case STORAGE_MEMORY_USB_HOST:
            strcpy((char*)dcfBackupDrive, "usbfs:1:");
            strcpy((char*)dcfBackupDir, "\\");
            strcpy((char*)dcfBackupPath, "usbfs:1:\\");
            strcpy((char*)dcfBackupTargetPath, "usbfs:1:\\");
            break;
#endif

        default:
            return -1;
    }

    return 1;
}
/*

Routine Description:

    Dump buffer.

Arguments:

    pBuf - Buffer pointer.
    size - Buffer size.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfDumpBuf(u8* pBuf, u32 size)
{
    u32 i;

    DEBUG_DCF("Trace: Dump buffer:\n");
    for (i = 0; i < size; i++)
    {
        if ((i != 0) && ((i % 16) == 0))
        {
            DEBUG_DCF("\n");
        }
        DEBUG_DCF("%02x ", pBuf[i]);
    }
    DEBUG_DCF("\n");

    return 1;
}

/*

Routine Description:

    Dump drive information.

Arguments:

    pInfo - Drive information.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfDumpDriveInfo(FS_DISKFREE_T* pInfo)
{
    DEBUG_DCF("Trace: Dump drive information:\n");
    DEBUG_DCF("Total clusters:      %lu\n", pInfo->total_clusters);
    DEBUG_DCF("Available clusters:  %lu\n", pInfo->avail_clusters);
    DEBUG_DCF("Sectors per cluster: %u\n",  pInfo->sectors_per_cluster);
    DEBUG_DCF("Bytes per sector:    %u\n",  pInfo->bytes_per_sector);

    return 1;
}

/*

Routine Description:

    Create target path.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfCreateTargetPath(char *pDirName)
{

    strcpy((char*)dcfTargetPath, (const char*)dcfCurDrive); //sdcmmc:0:
    if (pDirName[0] == '\\')
    {
        /* pDirName is an absolute path */
        strcat((char*)dcfTargetPath, (const char*)pDirName);
    }
    else
    {
        /* pDirName is a relative path */
        if (strcmp((const char*)dcfCurDir, "\\") != 0)
            strcat((char*)dcfTargetPath, (const char*)dcfCurDir);
        strcat((char*)dcfTargetPath, "\\");
        strcat((char*)dcfTargetPath, (const char*)pDirName);
    }

    return 1;
}


int dcfCreateBackupTargetPath(char *pDirName)
{

    strcpy((char*)dcfBackupTargetPath, (const char*)dcfBackupDrive); //sdcmmc:0:
    if (pDirName[0] == '\\')
    {
        /* pDirName is an absolute path */
        strcat((char*)dcfBackupTargetPath, (const char*)pDirName);
    }
    else
    {
        /* pDirName is a relative path */
        if (strcmp((const char*)dcfBackupDir, "\\") != 0)
            strcat((char*)dcfBackupTargetPath, (const char*)dcfBackupDir);
        strcat((char*)dcfBackupTargetPath, "\\");
        strcat((char*)dcfBackupTargetPath, (const char*)pDirName);
    }

    return 1;
}
/*

Routine Description:

    Create overwrit delete target path.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfCreateOWDelTargetPath(char *pDirName, char *TargetPath)
{
    if (TargetPath == NULL)
        return 0;

    strcpy((char*)TargetPath, (const char*)dcfCurDrive); //sdcmmc:0:
    if (pDirName[0] == '\\')
    {
        /* pDirName is an absolute path */
        strcat((char*)TargetPath, (const char*)pDirName);
    }
    else
    {
        /* pDirName is a relative path */
        if (strcmp((const char*)dcfDelDir, "\\") != 0)
            strcat((char*)TargetPath, (const char*)dcfDelDir);
        if(pDirName)
        {
        	strcat((char*)TargetPath, "\\");
        	strcat((char*)TargetPath, (const char*)pDirName);
        }
    }
    return 1;
}

/*

Routine Description:

    Create playback target path.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfCreatePlaybackTargetPath(char *pDirName, char *TargetPath)
{
    if (TargetPath == NULL)
        return 0;

    strcpy((char*)TargetPath, (const char*)dcfCurDrive); //sdcmmc:0:
    if (pDirName[0] == '\\')
    {
        /* pDirName is an absolute path */
        strcat((char*)TargetPath, (const char*)pDirName);
    }
    else
    {
        /* pDirName is a relative path */
        if (strcmp((const char*)dcfPlayDir, "\\") != 0)
            strcat((char*)TargetPath, (const char*)dcfPlayDir);
        if(pDirName)
        {
        	strcat((char*)TargetPath, "\\");
        	strcat((char*)TargetPath, (const char*)pDirName);
        }
    }


    return 1;
}

/*

Routine Description:

    Create Full path.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfCreateFullPath(char *pFullName, char *pDirName)
{
    strcpy((char*)pFullName, (const char*)dcfCurDrive); //sdcmmc:0:
    if (pDirName[0] == '\\')
    {
        /* pDirName is an absolute path */
        strcat((char*)pFullName, (const char*)pDirName);
    }
    else
    {
        /* pDirName is a relative path */
        if (strcmp((const char*)dcfCurDir, "\\") != 0)
            strcat((char*)pFullName, (const char*)dcfCurDir);
        strcat((char*)pFullName, "\\");
        strcat((char*)pFullName, (const char*)pDirName);
    }

    return 1;
}

/*

Routine Description:

    Create overwrite current path.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfCreateOWDelCurPath(void)
{
    strcpy((char*)dcfDelPath, (const char*)dcfCurDrive);
    strcat((char*)dcfDelPath, (const char*)dcfDelDir);

    //DEBUG_DCF("Trace: dcfCurPath = %s\n", dcfCurPath);

    return 1;
}

/*

Routine Description:

    Create playback current path.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfCreatePlayCurPath(void)
{
    strcpy((char*)dcfPlayPath, (const char*)dcfCurDrive);
    strcat((char*)dcfPlayPath, (const char*)dcfPlayDir);

    //DEBUG_DCF("Trace: dcfCurPath = %s\n", dcfCurPath);

    return 1;
}

/*

Routine Description:

    Create current path.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfCreateCurPath(void)
{
    strcpy((char*)dcfCurPath, (const char*)dcfCurDrive);
    strcat((char*)dcfCurPath, (const char*)dcfCurDir);

    return 1;
}

s32 dcfCreateBackupPath(void)
{
    strcpy((char*)dcfBackupPath, (const char*)dcfBackupDrive);
    strcat((char*)dcfBackupPath, (const char*)dcfBackupDir);

    return 1;
}
/*

Routine Description:

    Check if directory exists.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfCheckDirExist(char *pDirPath)
{
	char pTargetPath[64];
	strcpy((char *)pTargetPath, (const char*) dcfCurDrive); //sdcmmc:0:
	return dcfDirExist((s8 *)pTargetPath);
}

int dcfDirExist(s8* pDirName)
{
    FS_DIR *pDir;

    pDir = FS_OpenDir((const char*)pDirName);
    if(!pDir)
    {
        ERRD(FS_DIR_OPEN_ERR);	// directory not exists
        return -1;
    }

    // directory exists
    FS_CloseDir(pDir);
    return 1;
}

int dcfItemExist(char *pFileName, char *Mode)
{
	FS_FILE *pFile;
	u8 tmp;

	if((pFile = dcfOpen(pFileName, (s8 *)Mode)) == NULL)
	{
		ERRD(FS_FILE_OPEN_ERR);	// directory not exists
		return -1;
	}

	if(dcfClose(pFile, &tmp) == 0) 
	{
		ERRD(FS_FILE_ENT_UPDATE_ERR);
	}

	return 1;
}

/*

Routine Description:

    Drive change.

Arguments:

    pDriveName - Drive name.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfDrive(s8* pDriveName)
{
    strcpy((char*)dcfTargetPath, (const char*)pDriveName);
    strcat((char*)dcfTargetPath, "\\");
    if (dcfDirExist(dcfTargetPath) < 0)
    {
        DEBUG_DCF("Error: Change drive %s failed.\n", pDriveName);
        return 0;
    }

    strcpy((char*)dcfCurDrive, (const char*)pDriveName);
    strcpy((char*)dcfCurDir, "\\");
    strcpy((char*)dcfCurPath, (const char*)dcfTargetPath);
    return 1;
}

/*

Routine Description:

    Format drive.

Arguments:

    pDriveName - Drive name.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfFormat(s8* pDriveName)
{
    /* format current drive */
    /* CY 0718 */
    int DeviceIdx=1;

    if( (pDriveName[0]=='s') && (pDriveName[1]=='d') )
        DeviceIdx=STORAGE_MEMORY_SD_MMC;
    else if( (pDriveName[0]=='u') && (pDriveName[1]=='s') )
        DeviceIdx=STORAGE_MEMORY_USB_HOST;
    else if( (pDriveName[0]=='s') && (pDriveName[1]=='m') )
        DeviceIdx=STORAGE_MEMORY_SMC_NAND;


    DEBUG_DCF("dcfFormat\n");
    if (dcfStorageSize[DeviceIdx] == 0)
    {
        DEBUG_DCF("Error: Unknown storage size\n");
    }

    if (FS_IoCtl((const char*)pDriveName,FS_CMD_FORMAT_MEDIA,dcfStorageSize[DeviceIdx],0) < 0)
    {
        DEBUG_DCF("Error: Format %s failed\n", pDriveName);

        return 0;
    }

    dcfFileTypeCount_Clean();

    if(dcfCheckUnit() < 0)
    {
        DEBUG_DCF("Error: dcfCheckUnit failed in Formatting\n");
        return 0;
    }
    return 1;
}


s32 dcfFormatBackup(s8* pDriveName)
{
    /* format current drive */
    /* CY 0718 */
    int DeviceIdx=1;

    if( (pDriveName[0]=='s') && (pDriveName[1]=='d') )
        DeviceIdx=STORAGE_MEMORY_SD_MMC;
    else if( (pDriveName[0]=='u') && (pDriveName[1]=='s') )
        DeviceIdx=STORAGE_MEMORY_USB_HOST;
    else if( (pDriveName[0]=='s') && (pDriveName[1]=='m') )
        DeviceIdx=STORAGE_MEMORY_SMC_NAND;


    DEBUG_DCF("dcfFormatBackup:%d,%d\n",DeviceIdx,dcfStorageSize[DeviceIdx]);

    if (dcfStorageSize[DeviceIdx] == 0)
    {
        DEBUG_DCF("Error: Unknown storage size\n");
    }

    if (FS_IoCtl((const char*)pDriveName,FS_CMD_FORMAT_MEDIA,dcfStorageSize[DeviceIdx],0) < 0)
    {
        DEBUG_DCF("Error: Format %s failed\n", pDriveName);

        return 0;
    }

    if(dcfCheckBackupUnit() < 0)
    {
        DEBUG_DCF("Error: dcfCheckBackupUnit failed in Formatting\n");
        return 0;
    }
    return 1;
}
/*

Routine Description:

    Directory list.

Arguments:

    pDirName - Directory name.
    pDirEnt - Directory list.
    pDirCount - Directory count.
    IsUpdateEntrySect- 是否更新錄影時file entry.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfPlaybackDir(char *pDirName, FS_DIRENT *pDirEnt, u32* pDirCount,u8 IsUpdateEntrySect,int DoBadFile)
{
    FS_DIR *pDir;
    char PlayTargetPath[64];


    if (pDirName)
    {
        /* directory path specified */
        dcfCreatePlaybackTargetPath(pDirName, PlayTargetPath);
    }
    else
    {
        /* current directory */
        strcpy((char*)PlayTargetPath, (const char*)dcfPlayPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)PlayTargetPath);
    if (pDir)
    {
        // pDirName exists
        *pDirCount = FS_ReadWholeDir(pDir,pDirEnt,dcfBuf,DCF_FILEENT_MAX,&dcfBadFileInfo,IsUpdateEntrySect,DoBadFile); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (*pDirCount <= 0)
        {
            DEBUG_DCF("Parsing Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);
        return 1;
    }
    else
    {
        // pDirName not exists
        DEBUG_DCF("Error: Dir %s failed.\n", PlayTargetPath);
        ERRD(FS_DIR_OPEN_ERR);
        return 0;
    }
}

/*

Routine Description:

    Directory list.

Arguments:

    pDirName - Directory name.
    pDirEnt - Directory list.
    pDirCount - Directory count.
    IsUpdateEntrySect- 是否更新錄影時file entry.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfDir(char *pDirName, FS_DIRENT *pDirEnt, u32 *pDirCount, u8 IsUpdateEntrySect, int DoBadFile, u32 MaxEntry)
{
    FS_DIR* pDir;
    int ret;

    if (pDirName)
    {
        // directory path specified
        dcfCreateTargetPath(pDirName);
    }
    else
    {
        // current directory
        strcpy((char*)dcfTargetPath, (const char*)dcfCurPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)dcfTargetPath);
    if (pDir)
    {
        // pDirName exists
        ret = FS_ReadWholeDir(pDir,pDirEnt,dcfBuf,MaxEntry,&dcfBadFileInfo,IsUpdateEntrySect,DoBadFile); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (ret <= 0)
        {
            DEBUG_DCF("Parsing Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);
        *pDirCount = ret;
        return 1;
    }
    else
    {
        // pDirName not exists
        DEBUG_DCF("Error: Dir %s failed.\n", dcfTargetPath);
        ERRD(FS_DIR_OPEN_ERR);
        return 0;
    }
}
/*
 return  0: FAIL
         1: SUCCESS
*/

int dcfOWDelDirScan(char *pDirName, FS_DIRENT *pDirEnt, s32* pDirCount, u32 *pOldestEntry, int DoBadFile)
{
    FS_DIR* pDir;
    char delTargetPath[64];

    if (pDirName)
    {
        // directory path specified
        dcfCreateOWDelTargetPath(pDirName, delTargetPath);
    }
    else
    {
        // current directory
        strcpy((char*)delTargetPath, (const char*)dcfDelPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)delTargetPath);
    if (pDir)
    {
        // pDirName exists
        *pDirCount = FS_ScanWholeDir(pDir, pDirEnt, dcfBuf, DCF_FILEENT_MAX, pOldestEntry, DoBadFile); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (*pDirCount < 0)
        {
            DEBUG_DCF("Scan Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);
        return 1;
    }
    else
    {
        // pDirName not exists
        DEBUG_DCF("Error: Dir-Scan %s failed.\n", delTargetPath);
        ERRD(FS_DIR_READ_ERR);
        return 0;
    }
}

/*
 return  0: FAIL
         1: SUCCESS
*/

int dcfPlayDirScan(char *pDirName, FS_DIRENT *pDirEnt, u32* pDirCount, u32 *pOldestEntry, int DoBadFile)
{
    FS_DIR *pDir;
    int ret;
    char PlayTargetPath[64];

    if (pDirName)
    {
        /* directory path specified */
        dcfCreatePlaybackTargetPath(pDirName, PlayTargetPath);
    }
    else
    {
        /* current directory */
        strcpy((char*)PlayTargetPath, (const char*)dcfPlayPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)PlayTargetPath);
    if (pDir)
    {
        // pDirName exists
        ret = FS_ScanWholeDir(pDir,pDirEnt,dcfBuf,DCF_FILEENT_MAX,pOldestEntry,DoBadFile); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (ret < 0)
        {
            DEBUG_DCF("Scan Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        *pDirCount = ret;
        FS_CloseDir(pDir);
        return 1;
    }
    else
    {
        // pDirName not exists
        DEBUG_DCF("Error: Dir-Scan %s failed.\n", PlayTargetPath);
        ERRD(FS_DIR_READ_ERR);
        return 0;
    }
}


int dcfPlayDirSearch(char *pDirName, FS_DIRENT *pDirEnt, u32* pDirCount, u32 *pOldestEntry,
                     char CHmap, u32 Typesel, u32 StartMin, u32 EndMin, int DoBadFile)
{
    FS_DIR *pDir;
    int ret;
    char PlayTargetPath[64];

    if (pDirName)
    {
        /* directory path specified */
        dcfCreatePlaybackTargetPath(pDirName, PlayTargetPath);
    }
    else
    {
        /* current directory */
        strcpy((char*)PlayTargetPath, (const char*)dcfPlayPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)PlayTargetPath);
    if (pDir)
    {
        // pDirName exists
        ret =FS_SearchWholeDir(pDir,
                                     pDirEnt,
                                     dcfBuf,
                                     DCF_FILEENT_MAX,
                                     pOldestEntry,
                                     CHmap,
                                     Typesel,
                                     StartMin,
                                     EndMin,
                                     DoBadFile);
        if (ret < 0)
        {
            DEBUG_DCF("Search Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        *pDirCount = ret;
        FS_CloseDir(pDir);
        return 1;
    }
    else
    {
        // pDirName not exists
        DEBUG_DCF("Error: Dir-Search %s failed.\n", PlayTargetPath);
        ERRD(FS_DIR_OPEN_ERR);
        return 0;
    }
}

/*
 return  0: FAIL
         1: SUCCESS
*/

int dcfDirScan(char *pDirName, FS_DIRENT *pDirEnt, u32* pDirCount, u32 *pOldestEntry, int DoBadFile)
{
    FS_DIR* pDir;
	int ret;
    if (pDirName)
    {
        /* directory path specified */
        dcfCreateTargetPath(pDirName);
    }
    else
    {
        /* current directory */
        strcpy((char*)dcfTargetPath, (const char*)dcfCurPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)dcfTargetPath);
    if (pDir)
    {
        // pDirName exists
        ret = FS_ScanWholeDir(pDir,pDirEnt,dcfBuf,DCF_FILEENT_MAX,pOldestEntry,DoBadFile); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (ret < 0)
        {
            DEBUG_DCF("Scan Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        *pDirCount = ret;
        FS_CloseDir(pDir);
        return 1;
    }
    else
    {
        // pDirName not exists
        DEBUG_DCF("Error: Dir-Scan %s failed.\n", dcfTargetPath);
        ERRD(FS_DIR_READ_ERR);
        return 0;
    }
}
/*

Routine Description:

    Make directory.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfMkDir(char *pDirName)
{
    int ret;
    dcfCreateTargetPath(pDirName);
    if ((ret = FS_MkDir((const char *)dcfTargetPath)) < 0)
    {
        DEBUG_DCF("Error: Mkdir %s failed\n", dcfTargetPath);
        ERRD(FS_DIR_CREATE_ERR);
        return ret;
    }
    global_diskInfo.avail_clusters -= 1;
    return ret;
}


int dcfMkBackupDir(char *pDirName)
{
    dcfCreateBackupTargetPath(pDirName);

    if (FS_MkDir((const char *)dcfBackupTargetPath) < 0)
    {
        DEBUG_DCF("Error: dcfMkBackupDir %s failed\n", dcfBackupTargetPath);
        ERRD(FS_DIR_CREATE_ERR);
        return 0;
    }
    global_diskInfo.avail_clusters -=1;

    return 1;
}
/*

Routine Description:

    Remove directory.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfOWDelRmDir(char *pDirName,u8 checkflag)
{
    char delTargetPath[64];

    dcfCreateOWDelTargetPath(pDirName, delTargetPath);
    if (FS_RmDir((const char*)delTargetPath,checkflag) < 0)
    {
        DEBUG_DCF("Error: Rmdir %s failed. Be sure Directory is empty\n", delTargetPath);
        ERRD(FS_DIR_DELETE_ERR);
        return 0;
    }
    return 1;
}

/*

Routine Description:

    Remove directory.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfRmDir(char *pDirName, u8 checkflag)
{
    dcfCreateTargetPath(pDirName);
    if (FS_RmDir((const char*)dcfTargetPath,checkflag) < 0)
    {
        DEBUG_DCF("Error: Rmdir %s failed. Be sure Directory is empty\n", dcfTargetPath);
        ERRD(FS_DIR_DELETE_ERR);
        return 0;
    }
    return 1;
}

/*

Routine Description:

    Change overwrit deletedirectory.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfChOWDelDir(s8* pDirName)
{
    s8 backupCurDir[64];
    u8 curDirPos = 0;

    strcpy((char*)backupCurDir, (const char*)dcfDelDir);
    if (strcmp((const char*)pDirName, ".") == 0)
    {
        /* change to current directory */
        //DEBUG_DCF("Trace: Chdir %s is successful.\n", pDirName);

        return 1;
    }
    else if (strcmp((const char*)pDirName, "..") == 0)
    {
        /* change to parent directory */
        if (strcmp((const char*)dcfDelDir, "\\") == 0)
        {
            /* current directory is root directory */
            DEBUG_DCF("Trace: Chdir %s failed - this is a root directory.\n", pDirName);

            return 0;
        }
        else
        {
            /* current directory is not root directory */
            curDirPos = strlen((const char*)dcfDelDir);
            while (curDirPos--)
            {
                if (dcfDelDir[curDirPos] == '\\')
                {
                    /* back slash of parent directory is found */
                    if (curDirPos != 0)
                    {
                        /* parent directory is not root directory */
                        dcfDelDir[curDirPos] = '\0';
                    }
                    break;
                }
                else
                {
                    /* clear current directory until back slash of parent directory */
                    dcfDelDir[curDirPos] = '\0';
                }
            }
        }
    }
    else
    {
        /* change to a general directory */
        if (pDirName[0] == '\\')
        {
            /* change to an absolute path */
            strcpy((char*)dcfDelDir, (const char*)pDirName);
        }
        else
        {
            /* change to a relative path */
            if (strcmp((const char*)dcfDelDir, "\\") != 0)
            {
                /* current directory is not root directory */
                strcat((char*)dcfDelDir, "\\");
            }
            strcat((char*)dcfDelDir, (const char*)pDirName);
        }
    }

    dcfCreateOWDelCurPath();
    if (dcfDirExist(dcfDelPath) < 0)
    {
        DEBUG_DCF("Error:dcfChOWDelDir Chdir %s failed.\n", dcfDelPath);
        DEBUG_DCF("SYS_CTL0 = 0x%08x\n", SYS_CTL0);

        /* rewind to backup direcotry */
        strcpy((char*)dcfDelDir, (const char*)backupCurDir);
        dcfCreateOWDelCurPath();

        return 0;
    }

    //DEBUG_DCF("Trace: Chdir %s is successful.\n", dcfCurPath);

    return 1;
}

/*

Routine Description:

    Change Playback deletedirectory.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfChPlayDir(char *pDirName)
{
    s8 backupCurDir[64];
    u8 curDirPos = 0;

    strcpy((char*)backupCurDir, (const char*)dcfPlayDir);
    if (strcmp((const char*)pDirName, ".") == 0)
    {
        /* change to current directory */
        //DEBUG_DCF("Trace: Chdir %s is successful.\n", pDirName);

        return 1;
    }
    else if (strcmp((const char*)pDirName, "..") == 0)
    {
        /* change to parent directory */
        if (strcmp((const char*)dcfPlayDir, "\\") == 0)
        {
            /* current directory is root directory */
            DEBUG_DCF("Trace: Chdir %s failed - this is a root directory.\n", pDirName);

            return 0;
        }
        else
        {
            /* current directory is not root directory */
            curDirPos = strlen((const char*)dcfPlayDir);
            while (curDirPos--)
            {
                if (dcfPlayDir[curDirPos] == '\\')
                {
                    /* back slash of parent directory is found */
                    if (curDirPos != 0)
                    {
                        /* parent directory is not root directory */
                        dcfPlayDir[curDirPos] = '\0';
                    }
                    break;
                }
                else
                {
                    /* clear current directory until back slash of parent directory */
                    dcfPlayDir[curDirPos] = '\0';
                }
            }
        }
    }
    else
    {
        /* change to a general directory */
        if (pDirName[0] == '\\')
        {
            /* change to an absolute path */
            strcpy((char*)dcfPlayDir, (const char*)pDirName);
        }
        else
        {
            /* change to a relative path */
            if (strcmp((const char*)dcfPlayDir, "\\") != 0)
            {
                /* current directory is not root directory */
                strcat((char*)dcfPlayDir, "\\");
            }
            strcat((char*)dcfPlayDir, (const char*)pDirName);
        }
    }

    dcfCreatePlayCurPath();
    if (dcfDirExist(dcfPlayPath) < 0)
    {
        DEBUG_DCF("Error: Chdir %s failed.\n", dcfPlayPath);
        DEBUG_DCF("SYS_CTL0 = 0x%08x\n", SYS_CTL0);

        /* rewind to backup direcotry */
        strcpy((char*)dcfPlayDir, (const char*)backupCurDir);
        dcfCreatePlayCurPath();

        return 0;
    }

    //DEBUG_DCF("Trace: Chdir %s is successful.\n", dcfCurPath);

    return 1;
}

/*

Routine Description:

    Change directory.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfChDir(char *pDirName)
{
    s8 backupCurDir[64];
    u8 curDirPos = 0;

    strcpy((char*)backupCurDir, (const char*)dcfCurDir);
    if (strcmp((const char*)pDirName, ".") == 0)
    {
        /* change to current directory */
        //DEBUG_DCF("Trace: Chdir %s is successful.\n", pDirName);
        return 1;
    }
    else if (strcmp((const char*)pDirName, "..") == 0)
    {
        /* change to parent directory */
        if (strcmp((const char*)dcfCurDir, "\\") == 0)
        {
            /* current directory is root directory */
            DEBUG_DCF("Trace: Chdir %s failed - this is a root directory.\n", pDirName);
            return 0;
        }
        else
        {
            /* current directory is not root directory */
            curDirPos = strlen((const char*)dcfCurDir);
            while (curDirPos--)
            {
                if (dcfCurDir[curDirPos] == '\\')
                {
                    /* back slash of parent directory is found */
                    if (curDirPos != 0)
                    {
                        /* parent directory is not root directory */
                        dcfCurDir[curDirPos] = '\0';
                    }
                    break;
                }
                else
                {
                    /* clear current directory until back slash of parent directory */
                    dcfCurDir[curDirPos] = '\0';
                }
            }
        }
    }
    else
    {
        /* change to a general directory */
        if (pDirName[0] == '\\')
        {
            /* change to an absolute path */
            strcpy((char*)dcfCurDir, (const char*)pDirName);
        }
        else
        {
            /* change to a relative path */
            if (strcmp((const char*)dcfCurDir, "\\") != 0)
            {
                /* current directory is not root directory */
                strcat((char*)dcfCurDir, "\\");
            }
            strcat((char*)dcfCurDir, (const char*)pDirName);
        }
    }

    dcfCreateCurPath();
    if (dcfDirExist(dcfCurPath) < 0)
    {
        DEBUG_DCF("Error: Chdir %s failed.\n", dcfCurPath);
        DEBUG_DCF("SYS_CTL0 = 0x%08x\n", SYS_CTL0);

        /* rewind to backup direcotry */
        strcpy((char*)dcfCurDir, (const char*)backupCurDir);
        dcfCreateCurPath();

        return 0;
    }
    else
    {
        //DEBUG_DCF("Chdir %s\n", dcfCurPath);
    }

    //DEBUG_DCF("Trace: Chdir %s is successful.\n", dcfCurPath);

    return 1;
}


int dcfChBackupDir(char *pDirName)
{
    s8 backupCurDir[64];
    u8 curDirPos = 0;

    strcpy((char*)backupCurDir, (const char*)dcfBackupDir);
    if (strcmp((const char*)pDirName, ".") == 0)
    {
        /* change to current directory */
        return 1;
    }
    else if (strcmp((const char*)pDirName, "..") == 0)
    {
        /* change to parent directory */
        if (strcmp((const char*)dcfBackupDir, "\\") == 0)
        {
            /* current directory is root directory */
            DEBUG_DCF("Trace: Chdir %s failed - this is a root directory.\n", pDirName);

            return 0;
        }
        else
        {
            /* current directory is not root directory */
            curDirPos = strlen((const char*)dcfBackupDir);
            while (curDirPos--)
            {
                if (dcfBackupDir[curDirPos] == '\\')
                {
                    /* back slash of parent directory is found */
                    if (curDirPos != 0)
                    {
                        /* parent directory is not root directory */
                        dcfBackupDir[curDirPos] = '\0';
                    }
                    break;
                }
                else
                {
                    /* clear current directory until back slash of parent directory */
                    dcfBackupDir[curDirPos] = '\0';
                }
            }
        }
    }
    else
    {
        /* change to a general directory */
        if (pDirName[0] == '\\')
        {
            /* change to an absolute path */
            strcpy((char*)dcfBackupDir, (const char*)pDirName);
        }
        else
        {
            /* change to a relative path */
            if (strcmp((const char*)dcfBackupDir, "\\") != 0)
            {
                /* current directory is not root directory */
                strcat((char*)dcfBackupDir, "\\");
            }
            strcat((char*)dcfBackupDir, (const char*)pDirName);
        }
    }

    dcfCreateBackupPath();
    if (dcfDirExist(dcfBackupPath) < 0)
    {
        DEBUG_DCF("Error: ChBackupdir %s failed.\n", dcfBackupPath);

        /* rewind to backup direcotry */
        strcpy((char*)dcfBackupDir, (const char*)backupCurDir);
        dcfCreateBackupPath();

        return 0;
    }
    else
    {
        DEBUG_DCF("ChBackupdir %s Success\n", dcfBackupPath);
    }

    return 1;
}
/*

Routine Description:

    Open file.

Arguments:

    pFileName - File name.
    pMode - Mode.

Return Value:

    File handle.

*/
FS_FILE* dcfOpen(char *pFileName, s8* pMode)
{
    FS_FILE *pFile;
    char CurrTargetPath[64];
    
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
    if(!strcmp((char*)pMode,"r"))
        dcfCreatePlaybackTargetPath(pFileName, CurrTargetPath);
    else
    {
        dcfCreateTargetPath(pFileName);
        strcpy((char *)CurrTargetPath, (const char*)dcfTargetPath);
    }
#else
    dcfCreateTargetPath(pFileName);
    strcpy((char*)CurrTargetPath, (const char*)dcfTargetPath);
#endif

    pFile = FS_FOpen((const char*)CurrTargetPath, (const char*)pMode); /* open file */
    if (pFile == NULL)
    {
        /* file open failed */
        DEBUG_DCF("Error: Open %s failed.\n", CurrTargetPath);

        return NULL;
    }

    /* file open is successful */
    //DEBUG_DCF("DcfOpen %s (0x%08x) is successful.\n", CurrTargetPath, (u32)pFile);

    dcfBufRemain = DCF_BUF_SIZE;

    return pFile;
}


FS_FILE* dcfBackupOpen(char *pFileName, s8* pMode)
{
    FS_FILE *pFile;
    char CurrTargetPath[64];
    //------------------------------//

    dcfCreateBackupTargetPath(pFileName);
    strcpy((char*)CurrTargetPath, (const char*)dcfBackupTargetPath);
    pFile = FS_FOpen((const char*)CurrTargetPath, (const char*)pMode); /* open file */
    if (pFile == NULL)
    {
        /* file open failed */
        DEBUG_DCF("Error: BackupOpen %s failed.\n", CurrTargetPath);
        return NULL;
    }

    /* file open is successful */
    DEBUG_DCF("dcfBackupOpen %s (0x%08x) is successful.\n", CurrTargetPath, (u32)pFile);

    return pFile;
}

int dcfOWDelRename(char *pNewFileName, char *pOldFileName)
{
    char *s;
    int idx;
    char delTargetPath[64];

    dcfCreateOWDelTargetPath(pOldFileName, delTargetPath);
    idx = FS__find_fsl((char *) delTargetPath, &s);
    if (idx < 0)
    {
        return 0;  /* Device not found */
    }

    if( FS_fat_rename(s, (char *) pNewFileName,idx) < 0)
        return 0;


    return 1;

}


int dcfRename(s8* pNewFileName, char *pOldFileName)
{
    char *s;
    int idx;

    dcfCreateTargetPath(pOldFileName);
    idx = FS__find_fsl((char *) dcfTargetPath, &s);
    if (idx < 0)
    {
        return 0;  // Device not found
    }

    if(FS_fat_rename(s, (char *)pNewFileName,idx) < 0)
        return 0;
    return 1;
}

#if 0
s32 dcfDump(void)
{
    DCF_LIST_FILEENT* cur;

    DEBUG_DCF("Trace: DCF dump\n");

    cur= dcfGetPlaybackFileListHead();

    do
    {
        DEBUG_DCF("%s\n", cur->pDirEnt->d_name);
        cur=cur->next;
    }
    while( cur != dcfGetPlaybackFileListHead());

    return 1;
}
#endif


/*

Routine Description:

    Close file.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfClose(FS_FILE* pFile, u8* pOpenFile)
{
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster, bytes_per_cluster;
    u32 i, cnt;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif


    //-----每次close file 則從新做一次檔案空間統計-----//
    if (pFile->mode_w)
    {
        diskInfo    =   &global_diskInfo;
        bytes_per_cluster   =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        if (bytes_per_cluster!=0)
        {
            if (pFile->mode_a)
                used_cluster=(pFile->append_size+ bytes_per_cluster-1) / bytes_per_cluster;
            else
                used_cluster=(pFile->size + bytes_per_cluster-1) / bytes_per_cluster;
        }
        OS_ENTER_CRITICAL();
        if (diskInfo->avail_clusters >= used_cluster)
            diskInfo->avail_clusters -= used_cluster;
        else
            diskInfo->avail_clusters = diskInfo->avail_clusters ;
        *pOpenFile = 0;
        OS_EXIT_CRITICAL();
        DEBUG_FS2("Available Cluster=%d\n",global_diskInfo.avail_clusters);
    }
	
    FS_FClose(pFile); // close file

#if (MULTI_CHANNEL_VIDEO_REC)
	// Saving all cache when no video is recording.
	for(i = 0, cnt = 0; i < DCF_MAX_MULTI_FILE; i++)
	{
		if(MultiChannelGetCaptureVideoStatus(i + MULTI_CHANNEL_LOCAL_MAX) != 0)
			cnt++;
	}
#endif
	if(cnt == 0x1)
		dcfCacheClean();
		
    return 1;
}


s32 dcfBackupClose(FS_FILE* pFile)
{

    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;


    //-----每次close file 則從新做一次檔案空間統計-----//

    if (pFile->mode_w)
    {
        diskInfo    =   &Backup_diskInfo;
        bytes_per_cluster   =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        if (bytes_per_cluster!=0)
        {
            if (pFile->mode_a)
                used_cluster=(pFile->append_size+ bytes_per_cluster-1) / bytes_per_cluster;
            else
                used_cluster=(pFile->size + bytes_per_cluster-1) / bytes_per_cluster;
        }
        if (diskInfo->avail_clusters >= used_cluster)
            diskInfo->avail_clusters -= used_cluster;
        else
            diskInfo->avail_clusters = diskInfo->avail_clusters ;
        DEBUG_FS2("Backup Available Cluster=%d\n",Backup_diskInfo.avail_clusters);
    }



    FS_FClose(pFile); /* close file */
    return 1;
}
/*

Routine Description:

    Write file.

Arguments:

    pFile - File handle.
    pData - Data pointer.
    dataSize - Data size.
    pWriteSize - Write size.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfWrite(FS_FILE* pFile, u8* pData, u32 dataSize, u32* pWriteSize)
{
    int ret, timeVal;
    u8 err;
    //---------------------------------------------//
#if SDC_DEBUG_ENA
    gpioSetLevel(0, 12, 1 );
#endif

    if(dataSize >= 0x80000000)
    {
        DEBUG_DCF("Error: dcfWrite(),invalid datasize=0x%x",dataSize);
        return 0;
    }

#if DCF_WRITE_STATISTIC
    dcfWriteAccum += (dataSize>>10);
#endif
#if (IIS_TEST || H264_TEST || ASF_SPLIT_FILE || H1_264TEST_ENC)
    if(1)
#else
    if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY ||gInsertNAND)
#endif
    {
    	timeVal = OSTimeGet();
    	OSSemPend(dcfWriteSemEvt, OS_IPC_WAIT_FOREVER, &err);
        //-----------------Video Mode Use (Optimized)------------------------//
        //Lucian: ASF 有提供Mass write(32KB)的功能,能達到寫入最佳化.
        ret = FS_FWrite(pFile, pData, dataSize, pWriteSize); /* write to file */
        timeVal = OSTimeGet() - timeVal;
        if(timeVal > 1)
        	DEBUG_DCF("[W] DCF WrTick: %d (x50ms)\n", timeVal);
        OSSemPost(dcfWriteSemEvt);
        if (ret < 0 || *pWriteSize < dataSize)
        {
            // log error
            ret = FS_FError(pFile);
            DEBUG_DCF("Error: Write 0x%08x failed - error number is %x.\n", (u32)pFile, ret);
            return 0;
        }
    }
#if SDC_DEBUG_ENA
    gpioSetLevel(0, 12, 0 );
#endif

    return 1;
}


s32 dcfBackupWrite(FS_FILE* pFile, u8* pData, u32 dataSize, u32* pWriteSize)
{
    s32 err;
    //---------------------------------------------//

    if(dataSize >= 0x80000000)
    {
        DEBUG_DCF("Error: dcfBackupWrite(),invalid datasize=0x%x",dataSize);
        return 0;
    }

    if (sysGetStorageStatus(SYS_I_STORAGE_BACKUP) == SYS_V_STORAGE_READY)
    {
        err = FS_FWrite(pFile, pData, dataSize, pWriteSize); // write to file
        if (err < 0 || *pWriteSize < dataSize)
        {
            // log error
            err = FS_FError(pFile);
            DEBUG_DCF("Error: dcfBackupWrite 0x%08x failed - error number is %x.\n", (u32)pFile, err);
            return 0;
        }
    }

    return 1;
}
/*

Routine Description:

    Read file.

Arguments:

    pFile - File handle.
    pData - Data pointer.
    dataSize - Data size.
    pReadSize - Read size.

Return Value:

    0 - Failure.
    1 - Success.
    2 - Success - EOF is reached.

*/
s32 dcfRead(FS_FILE* pFile, u8* pData, u32 dataSize, u32* pReadSize)
{
    int result;
    //u8 err;

    //DEBUG_YELLOW("dcfRead :%#x\n", pFile->filepos);
	//OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    result = FS_FRead(pFile, pData, dataSize, pReadSize); // read from file
   	//OSSemPost(dcfReadySemEvt);
    if((result < 0) || (*pReadSize < dataSize))
    {
        // log error
        result = FS_FError(pFile);
        if (result == FS_ERR_EOF)
        {
            // EOF
            //DEBUG_DCF("Trace: Read 0x%08x is successful - EOF is reached.\n", (u32)pFile);
            //dcfDumpBuf(pData, *pReadSize);
            return 2;
        }
        else
        {
            DEBUG_DCF("Error: Read 0x%08x failed - error number is %d.\n", (u32)pFile, result);
            return 0;
        }
    }

    //DEBUG_DCF("Trace: Read 0x%08x is successful.\n", (u32)pFile);
    //dcfDumpBuf(pData, *pReadSize);

    return 1;
}

s32 dcfBackupRead(FS_FILE* pFile, u8* pData, u32 dataSize, u32* pReadSize)
{
    int result;

    result = FS_FRead(pFile, pData, dataSize, pReadSize); // read from file
    if((result < 0) || (*pReadSize < dataSize))
    {
        // log error
        result = FS_FError(pFile);
        if (result == FS_ERR_EOF)
        {
            // EOF
            return 2;
        }
        else
        {
            DEBUG_DCF("Error: Read 0x%08x failed - error number is %d.\n", (u32)pFile, result);
            return 0;
        }
    }
    return 1;
}

s32 dcfSetDelDir(s8* pDirName)
{

    if (pDirName[0] == '\\')
    {
        /* change to an absolute path */
        strcpy((char*)dcfDelDir, (const char*)pDirName);
    }
    else
    {
        /* change to a relative path */
        if (strcmp((const char*)dcfDelDir, "\\") != 0)
        {
            /* current directory is not root directory */
            strcat((char*)dcfDelDir, "\\");
        }
        strcat((char*)dcfDelDir, (const char*)pDirName);
    }
    return 1;
}

s32 dcfOWEnable(void)
{
    u32 free_size = dcfGetMainStorageFreeSize();

    while((free_size < DCF_OVERWRITE_THR_KBYTE) || (dcfGetTotalDirCount() > (DCF_DIRENT_MAX - 2)) )
    {
        // Find the oldest file pointer and delete it
        if(dcfOverWriteDel() == 0)
        {
            DEBUG_DCF("Over Write delete fail!!\n");
            return 0;
        }
        
        free_size = dcfGetMainStorageFreeSize();
        DEBUG_DCF("Free Space = %d (KBytes)\n", free_size);
    }
    return 1;
}

/*

Routine Description:

    Overwrite Delete file.

Arguments:

    pFileName - File name.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfStrCatVideoPath(char *TargetPath, char *pDirName, char *pFileName)
{
	if (TargetPath == NULL)
		return 0;

	strcpy((char*)TargetPath, (const char*) dcfCurDrive); //sdcmmc:0:
	strcat((char*)TargetPath, "\\");
	strcat((char*)TargetPath, (const char*) gsDirName);

	// pDirName is a relative path
	if(pDirName)
	{
		strcat((char*)TargetPath, "\\");
		strcat((char*)TargetPath, (const char*) pDirName);

		if(pFileName)
		{
			strcat((char*)TargetPath, "\\");
			strcat((char*)TargetPath, (const char*) pFileName);
		}
	}
	
    return 1;
}

int dcfOWDel(char *pDirName)
{
	FS_DIR *pDir;
	FS_DeleteCondition pCondition;
	int ret;
	char delTargetPath[64];

	pCondition.DeleteMode = FS_E_DELETE_TYPE_AUTO;

	// directory path specified
	dcfStrCatVideoPath(delTargetPath, pDirName, NULL);
	if((pDir = FS_OpenDir((const char*)delTargetPath)) == NULL)
	{
		DEBUG_DCF("Error: Directory open %s failed.\n", delTargetPath);
		return -1;
	}

	memcpy(&pDir->dirent.d_name, pDirName, FS_V_FAT_ENTEY_SHORT_NAME);
	if((ret = FS_Remove(pDir, NULL, &pCondition)) < 0)
		DEBUG_DCF("Error: Remove %s failed.\n", delTargetPath);
	
	FS_CloseDir(pDir);
	if(MemoryFullFlag == TRUE)
    {
        FS_DISKFREE_T *diskInfo = &global_diskInfo;
        u32 free_size = diskInfo->avail_clusters * (diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector) /1024; //KByte unit

        if(free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE*IIS_BUF_NUM))/1024) //Notice: K-Byte unit
        {
            DEBUG_DCF("Disk Full\n");
            MemoryFullFlag = TRUE;
            sysProjectDeviceStatus(DEV_SD_FULL);
        }
        else
        {
            MemoryFullFlag = FALSE;
            sysProjectDeviceStatus(DEV_SD_NOT_FULL);
        }
    }
	return ret;
}

int dcfOWDelDir(char *pDirName)
{
	FS_DIR *pDir;
	FS_DeleteCondition pCondition;
	int ret;
	char delTargetPath[64];
	
	pCondition.DeleteMode = FS_E_DELETE_TYPE_DIR;

	// directory path specified
	dcfStrCatVideoPath(delTargetPath, pDirName, NULL);
	if((pDir = FS_OpenDir((const char*)delTargetPath)) == NULL)
	{
		DEBUG_DCF("Error: Directory open %s failed.\n", delTargetPath);
		return -1;
	}

	if((ret = FS_Remove(pDir, NULL, &pCondition)) < 0)
		DEBUG_DCF("Error: Remove %s failed.\n", delTargetPath);

	FS_CloseDir(pDir);
	return ret;
}

int dcfDel(char *pDirName, char *pFileName)
{
	FS_DIR *pDir;
	FS_DeleteCondition pCondition;
	int ret;

	pCondition.DeleteMode = FS_E_DELETE_TYPE_ORDER;
	
	// directory path specified
	dcfStrCatVideoPath((char *)dcfTargetPath, pDirName, NULL);
	if((pDir = FS_OpenDir((const char*)dcfTargetPath)) == NULL)
	{
		DEBUG_DCF("Error: Directory open %s failed.\n", dcfTargetPath);
		return -1;
	}

	if((ret = FS_Remove(pDir, pFileName, &pCondition)) < 0)
		DEBUG_DCF("Error: Remove %s failed.\n", dcfTargetPath);

	FS_CloseDir(pDir);
	if(MemoryFullFlag == TRUE)
    {
        FS_DISKFREE_T *diskInfo = &global_diskInfo;
        u32 free_size = diskInfo->avail_clusters * (diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector) /1024; //KByte unit

        if(free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE*IIS_BUF_NUM))/1024) //Notice: K-Byte unit
        {
            DEBUG_DCF("Disk Full\n");
            MemoryFullFlag = TRUE;
            sysProjectDeviceStatus(DEV_SD_FULL);
        }
        else
        {
            MemoryFullFlag = FALSE;
            sysProjectDeviceStatus(DEV_SD_NOT_FULL);
        }
    }
	return ret;
}

int dcfPlayDel(char *pDirName, char *pFileName)
{
	FS_DIR *pDir;
	FS_DeleteCondition pCondition;
	char PlayTargetPath[64];
	int ret;

	pCondition.DeleteMode = FS_E_DELETE_TYPE_ORDER;
	
	dcfStrCatVideoPath(PlayTargetPath, pDirName, NULL);
	if((pDir = FS_OpenDir((const char*)PlayTargetPath)) == NULL)
	{
		DEBUG_DCF("Error: Directory open %s failed.\n", PlayTargetPath);
		return -1;
	}

	memcpy(&pDir->dirent.d_name, pDirName, FS_V_FAT_ENTEY_SHORT_NAME);
	if((ret = FS_Remove(pDir, pFileName, &pCondition)) < 0)
		DEBUG_DCF("Error: Remove %s failed.\n", pFileName);
		
	FS_CloseDir(pDir);
	if(MemoryFullFlag == TRUE)
    {
        FS_DISKFREE_T *diskInfo = &global_diskInfo;
        u32 free_size = diskInfo->avail_clusters * (diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector) /1024; //KByte unit

        if(free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE*IIS_BUF_NUM))/1024) //Notice: K-Byte unit
        {
            DEBUG_DCF("Disk Full\n");
            MemoryFullFlag = TRUE;
            sysProjectDeviceStatus(DEV_SD_FULL);
        }
        else
        {
            MemoryFullFlag = FALSE;
            sysProjectDeviceStatus(DEV_SD_NOT_FULL);
        }
    }
	return ret;
}


#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
int dcfStrCatLogPath(char *TargetPath, char *pFileName)
{
	if (TargetPath == NULL)
		return 0;

	strcpy((char*)TargetPath, (const char*) dcfCurDrive); //sdcmmc:0:
	strcat((char*)TargetPath, "\\");
	strcat((char*)TargetPath, (const char*) gsLogDirName);

	if(pFileName)
	{
		strcat((char*)TargetPath, "\\");
		strcat((char*)TargetPath, (const char*) pFileName);
	}
	
    return 1;
}


int dcfLogDel(char *pFileName)
{
	FS_DIR *pDir;
	FS_DeleteCondition pCondition;
    char LogTargetPath[64];
    int ret;

    pCondition.DeleteMode = FS_E_DELETE_TYPE_ORDER;


    dcfStrCatLogPath(LogTargetPath, NULL);
	if((pDir = FS_OpenDir((const char*)LogTargetPath)) == NULL)
	{
		DEBUG_DCF("Error: Directory open %s failed.\n", LogTargetPath);
		return -1;
	}
	
	memcpy(&pDir->dirent.d_name, gsLogDirName, FS_V_FAT_ENTEY_SHORT_NAME);
	if((ret = FS_Remove(pDir, pFileName, &pCondition)) < 0)
		DEBUG_DCF("Error: Remove %s failed.\n", pFileName);
	
    FS_CloseDir(pDir);
	if(MemoryFullFlag == TRUE)
    {
        FS_DISKFREE_T *diskInfo = &global_diskInfo;
        u32 free_size = diskInfo->avail_clusters * (diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector) /1024; //KByte unit

        if(free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE*IIS_BUF_NUM))/1024) //Notice: K-Byte unit
        {
            DEBUG_DCF("Disk Full\n");
            MemoryFullFlag = TRUE;
            sysProjectDeviceStatus(DEV_SD_FULL);
        }
        else
        {
            MemoryFullFlag = FALSE;
            sysProjectDeviceStatus(DEV_SD_NOT_FULL);
        }
    }
    return 1;
}
#endif

/**********************************

Routine Description:

    Current drive information.

Arguments:

    pInfo - None.

Return Value:

    0 - Failure.
    1 - Success.

************************************/
int dcfDriveInfo(FS_DISKFREE_T* pInfo, int Initilized)
{
    int ret;
#if FILE_SYSTEM_DVF_TEST
    u32 time1,time2;
    time1 = OSTimeGet();
#endif
    if((ret = FS_IoCtl((const char*)dcfCurDrive, FS_CMD_GET_DISKFREE, Initilized, (void*)pInfo)) < 0)
    {
        DEBUG_DCF("Error: Get drive %s information failed.\n", dcfCurDrive);
        return ret;
    }
#if FILE_SYSTEM_DVF_TEST
    time2 = OSTimeGet();
    DEBUG_DCF("--->dcfDriveInfo Time=%d (x100ms)\n",time2-time1);
#endif
    //DEBUG_DCF("Trace: Get drive %s information is successful.\n", dcfCurDrive);
    dcfDumpDriveInfo(pInfo);
    return ret;
}

int dcfBackupDriveInfo(FS_DISKFREE_T* pInfo, int Initilized)
{
    int ret;
#if FILE_SYSTEM_DVF_TEST
    u32 time1,time2;
    time1 = OSTimeGet();
#endif
    if((ret = FS_IoCtl((const char*)dcfBackupDrive, FS_CMD_GET_DISKFREE, Initilized, (void*)pInfo)) < 0)
    {
        DEBUG_DCF("Error: Get drive %s information failed.\n", dcfBackupDrive);
        return ret;
    }
#if FILE_SYSTEM_DVF_TEST
    time2 = OSTimeGet();
    DEBUG_DCF("--->dcfBackupDriveInfo Time=%d (x100ms)\n",time2-time1);
#endif
    //DEBUG_DCF("Trace: Get drive %s information is successful.\n", dcfCurDrive);
    dcfDumpDriveInfo(pInfo);
    return ret;
}

/*

Routine Description:

    Get directory entry.

Arguments:

    pDirName - Directory name.
    pDirEnt - Directory entry.
    pSubDirName - Sub-directory count.

Return Value:

    0 - Failure.
    1 - Success.

*/
int dcfGetDirEnt(char *pDirName, FS_DIRENT *pDirEnt, char *pSubDirName)
{
    FS_DIR* pDir;
    FS_DIRENT pEnt;
    int result;

    if (pDirName)
    {
        // directory path specified
        dcfCreateTargetPath(pDirName);
    }
    else
    {
        // current directory
        strcpy((char*)dcfTargetPath, (const char*)dcfCurPath);
    }

    pDir = FS_OpenDir((const char*)dcfTargetPath);
    if (pDir)	// pDirName exists
    {
        do
        {
            result = FS_ReadDir(pDir, &pEnt);
            if(result < 0)
            {
                ERRD(FS_DIR_READ_ERR);
                break;
            }
            //DEBUG_DCF("Trace: Getdirent %s , %s \n", pEnt->d_name, pSubDirName);
            /* extension of dcf directory name is composed of space characters */
            if (strcmp(&pEnt.d_name[0], (const char*)pSubDirName) != 0)
                continue;
            // Fetch the local value to attach ptr we put is dangerous behavior,
            // 	so, here using the memcpy to prepare the content
            memcpy(pDirEnt, &pEnt, sizeof(FS_DIRENT));
            FS_CloseDir(pDir);
            return 1;
        }
        while (result >= 0);

        FS_CloseDir(pDir);
        DEBUG_DCF("Error1: Getdirent %s of %s failed.\n", pSubDirName, dcfTargetPath);
        return 0;
    }
    else
    {
        // pDirName not exists
        DEBUG_DCF("Error2: Getdirent %s of %s failed\n", pSubDirName, dcfTargetPath);
        ERRD(FS_DIR_OPEN_ERR);
        return 0;
    }
}

/****************************

Routine Description:

    Increase directory entry.

Arguments:

    pDirName - Directory name.
    IncEntNum - Want to increase Directory entry num.

Return Value:

    0 - Failure.
    1 - Success.

****************************/
int dcfIncDirEnt(char *pDirName, s32 IncEntNum)
{
    int err;
    int idx;
    u32 dstart;
    u32 dsize;
    u32 unit;
    char *fname;
    int IncSect;
    char *PathName;
    char FullName[64];
    u8 SecPerClus;
    int AllocFatNum;

    dcfCreateFullPath(FullName, pDirName); //make full path.
    DEBUG_DCF("FULL Path: %s\n",FullName);

    idx = FS__find_fsl((const char *)FullName, &PathName);
    if(idx < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return 0;
    }
    DEBUG_DCF("File Path: %s, Device index: %d\n",PathName, idx);

    strcat((char*)PathName, "\\"); //Lucian: 讓它找到100VIDEO
    err = FS__fat_findpath(idx, PathName, &fname, &unit, &dstart, &dsize);
    if(err < 0)
    {
        DEBUG_DCF("[Error]: Find path: %d, %s\n", idx, pDirName);
        ERRD(FS_DIR_ENT_SIZE_ERR);
        return err;
    }
    DEBUG_DCF("fname: %s, unit: %d, dstart: %d, dsize: %d\n",fname,unit,dstart,dsize);

    IncSect = (IncEntNum*32/512)-dsize;  //將directory entry number 換算成所需增加的 sector size.

    if(IncSect > 0)
        err = FSFATIncEntry(idx, unit, dstart, 1, &dsize, FS_E_CLUST_CLEAN_ON);
    else
        err = 1;

    if(err < 0)
    {
        DEBUG_DCF("Increase Dir size Fail!!\n");
        return err;
    }
    else
    {
        SecPerClus = global_diskInfo.sectors_per_cluster;
        if(SecPerClus == 0)
        {
            DEBUG_DCF("Warning! SecPerClus=0\n");
        }
        AllocFatNum = (IncSect + SecPerClus -1 )/SecPerClus; //Lucian: 轉換成cluster unit.(無條件進位)
        global_diskInfo.avail_clusters -= AllocFatNum;
    }

    err = FS__fat_findpath(idx, PathName, &fname, &unit, &dstart, &dsize);
    if(err < 0)
    {
        DEBUG_DCF("Find path: %d, %s\n", idx, pDirName);
        ERRD(FS_DIR_ENT_SIZE_ERR);
        return err;
    }
    DEBUG_DCF("fname: %s, unit: %d, dstart: %d, dsize: %d\n",fname,unit,dstart,dsize);
    
    return 1;
}


/*

Routine Description:

    Flush file buffer.

Arguments:

    pFile - File handle.

Return Value:

     0 - Failure.
     1 - Success.

*/
s32 dcfFlushTempBuf(FS_FILE* pFile)
{
    u32 WriteSize;
    s32 err;

    if (dcfBufRemain < DCF_BUF_SIZE && pFile->mode_w)
    {
        err = FS_FWrite(pFile, dcfBuf, DCF_BUF_SIZE - dcfBufRemain, &WriteSize); /* write to file */
        if (err < 0 || WriteSize < (DCF_BUF_SIZE - dcfBufRemain)) // modify by Peter 2007/10/09
        {
            // log error
            err = FS_FError(pFile);
            DEBUG_DCF("Error: Write 0x%08x failed - error number is %x.\n", (u32)pFile, err);
            return 0;
        }
    }
    dcfBufRemain    = DCF_BUF_SIZE;

    return  1;
}

/*********************************************************************
*
*             dcfSeek
*
  Description:
  API function. Set current position of a file pointer.
  dcfSeek does not support to position the fp behind end of a file.

  Parameters:
  pFile       - Pointer to a FS_FILE data structure.
  Offset      - Offset for setting the file pointer position.
  Whence      - Mode for positioning the file pointer.

  Return value:
     0 - Failure.
     1 - Success.
*/

int dcfSeek(FS_FILE *pFile, s32 Offset, int Whence)
{

    if (!pFile)
    {
        return 0;
    }

    if(Whence == FS_SEEK_SET)
    {
        if(Offset < 0)
        {
            DEBUG_DCF("Error: dcfSeek(),invalid Offset=0x%x",Offset);
            return 0;
        }

    }

    if (FS_FSeek(pFile, Offset,Whence))
    {
        DEBUG_DCF("dcfSeek Error: FS_FSeek Fail\n");
        return 0;
    }

    return 1;
}

/*********************************************************************
*
*             dcfTell
*
  Description:
  API function. Return position of a file pointer.

  Parameters:
  pFile       - Pointer to a FS_FILE data structure.

  Return value:
  >=0         - Current position of the file pointer.
  ==-1        - An error has occured.
*/

s32 dcfTell(FS_FILE *pFile)
{
    s32 filepos;

    if (!pFile)
    {
        return -1;
    }

    filepos=FS_FTell(pFile);

    return filepos;
}

/**********************************

Routine Description:

    clean the cache of file system

Arguments:

    pInfo - None.

Return Value:

    0 - Failure.
    1 - Success.

************************************/
int dcfCacheClean(void)
{
    u32 unit;
    int Idx,err;
    char *s;

    unit=0; //unit is always 0 in our system.

    //DEBUG_DCF("[INF] DCF Cache clean. %s \n", dcfCurDrive);
    Idx = FS__find_fsl((const char *)dcfTargetPath, &s);
    if(Idx < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return 0;
    }
#if FS_USE_LB_READCACHE
    err = FS_LB_Cache_Clean(Idx, unit); //write back to SD card.
    if(err < 0)
    {
        ERRD(FS_LB_CACHE_CLEAN_ERR);
        return err;
    }
#endif
    return 1;
}

int dcfBackupCacheClean(void)
{
    u32 unit;
    int Idx,err;
    char *s;
    //
    unit = 0; //unit is always 0 in our system.
    //
    DEBUG_DCF("dcfBackupCacheClean: dcfBackupTargetPath= %s \n",dcfBackupTargetPath);
    Idx = FS__find_fsl((const char *)dcfBackupTargetPath, &s);
    if(Idx < 0)
    {
        ERRD(FS_DEVICE_FIND_ERR);
        return 0;
    }
#if FS_USE_LB_READCACHE
    err = FS_LB_Cache_Clean(Idx, unit); //write back to SD card.
    if(err < 0)
    {
        ERRD(FS_LB_CACHE_CLEAN_ERR);
        return err;
    }
#endif
    return 1;
}

int dcfCacheInit(void)
{
	u32 Unit = 0;
	int idx;
	char *s;

	if((idx = FS__find_fsl((const char *)dcfTargetPath, &s)) < 0)
	{
		ERRD(FS_DEVICE_FIND_ERR);
		return idx;
	}
#if FS_USE_LB_READCACHE
	FS_LB_Cache_Init(idx, Unit);
#endif
	return 1;
}

int dcfCacheEnable(void)
{
	u32 Unit = 0;
	int idx;
	char *s;

	if((idx = FS__find_fsl((const char *)dcfTargetPath, &s)) < 0)
	{
		ERRD(FS_DEVICE_FIND_ERR);
		return idx;
	}
#if FS_USE_LB_READCACHE
	FS_LB_Cache_Enable(idx, Unit);
#endif
	return 1;
}

int dcfCacheClear(void)
{
    u32 unit = 0;
    int Idx;
    char *s;

    unit = 0; // unit is always 0 in our system.

    Idx = FS__find_fsl((const char *)dcfTargetPath, &s);
#if FS_USE_LB_READCACHE
    FS_LB_Cache_Clear(Idx, unit);
#endif
    return 1;
}

int dcfBackupCacheClear(void)
{
    u32 unit;
    int Idx;
    char *s;

    unit = 0; //unit is always 0 in our system.
    
    Idx = FS__find_fsl((const char *)dcfBackupTargetPath, &s);
#if FS_USE_LB_READCACHE
    FS_LB_Cache_Clear(Idx, unit); //Clear DRAM cache.
#endif
    return 1;
}

u32 dcfGetMainStorageFreeSize(void)
{
	FS_DISKFREE_T *diskInfo = &global_diskInfo;
	u32 SumOfFree = 0;

	// Free = Size of Cluster * Avalibile size of cluster / KB Unit
	SumOfFree = (diskInfo->avail_clusters * ((diskInfo->bytes_per_sector * diskInfo->sectors_per_cluster) >> 10));	
	
	return SumOfFree;
}


int dcfGetPlaybackNameMaskIndex(void)
{
#if DCF_RECORD_TYPE_API
    switch(dcfChannelRecType[0])
    {
        case DCF_E_FILE_NAME_APM:
            return 0;

        case DCF_E_FILE_NAME_DASH:
        case DCF_E_FILE_NAME_MANUAL:
        case DCF_E_FILE_NAME_SCHE:
        case DCF_E_FILE_NAME_DYNAMIC:
        case DCF_E_FILE_NAME_FORCE_TRIGGER:
        default:
            return 6;
    }
#else

#if ((UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_YMD_APM) || (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_MDY_APM))
    return 0;
#else
    return 6;
#endif    

#endif    
}

int dcfFilterNameFormat(const char *ItemName, u32 SearchMode, u16 NameTypeMask)
{
    if(SearchMode != DCF_E_FILE_NAME_ALL)
    {
        switch(ItemName[NameTypeMask])
        {
            case '-':
                if(SearchMode & DCF_E_FILE_NAME_DASH)
                    break;
                return 0;
            case 'M':
                if(SearchMode & DCF_E_FILE_NAME_MANUAL)
                    break;
                return 0;
            case 'S':
                if(SearchMode & DCF_E_FILE_NAME_SCHE)
                    break;
                return 0;
            case 'D':
                if(SearchMode & DCF_E_FILE_NAME_DYNAMIC)
                    break;
                return 0;
            case 'R':
                if(SearchMode & DCF_E_FILE_NAME_RING)
                    break;
                return 0;
            case 'A':
            case 'P':
                if(SearchMode & DCF_E_FILE_NAME_APM)
                    break;
                return 0;
            default:
                return 0;
        }
    }
    return 1;
}

int dcfFetchDirItems(char *pDirName, FS_DIRENT *pDirEnt, FS_SearchCondition *pCondition)
{
    FS_DIR *pDir;
    int ret;
    char PlayTargetPath[64];

    if(pDirName)
    {
        /* directory path specified */
        dcfCreatePlaybackTargetPath(pDirName, PlayTargetPath);
    }
    else
    {
        /* current directory */
        strcpy((char*)PlayTargetPath, (const char*)dcfPlayPath);
    }

    pDir = FS_OpenDir((const char*)PlayTargetPath);
    if(pDir)
    {
        // pDirName exists
        ret = FS_FetchItems(pDir, pDirEnt, pCondition);
        if (ret < 0)
        {
            DEBUG_DCF("Fetch directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);
        return 1;
    }
    else
    {
        // pDirName not exists
        DEBUG_DCF("Error: Fetch directory %s failed.\n", PlayTargetPath);
        ERRD(FS_DIR_READ_ERR);
        return 0;
    }
}


