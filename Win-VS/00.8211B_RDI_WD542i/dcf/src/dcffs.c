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
#include "board.h"
#include "asfapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "siuapi.h"
#include "gpioapi.h"
#include "sysapi.h"
#include "iisapi.h"


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
s8 dcfCurDir[64];   /*CY 1023*/
s8 dcfCurPath[64];  /*CY 1023*/
s8 dcfTargetPath[64];   /*CY 1023*/
s8 Temp_dcfCurDrive[64];
s8 Temp_dcfCurDir[64];
s8 Temp_dcfCurPath[64];
s8 Temp_dcfTargetPath[64];
s8 dcfDelDir[64];
s8 dcfDelPath[64];
s8 dcfPlayDir[64];
s8 dcfPlayPath[64];

#if CDVR_iHome_LOG_SUPPORT
s8 dcfLogDir[64];
s8 dcfLogPath[64];
#endif


u32 dcfStorageType;     /*CY 1023*/
s32 dcfStorageSize = 0; /*CY 0718 */
u32 dcfBufRemain; /* VCC */
u8  gInsertCard;

s32 dcfLasfEofCluster=-1;  //Last free cluster in disk. update it when 1. power-on, 2. SD/NAND switch, 3. file open, 4. file close, 5.memory full

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

#if CDVR_iHome_LOG_SUPPORT
extern s8 gsLogDirName[9];
#endif

#if RX_SNAPSHOT_SUPPORT
extern s8 gsPhotoDirName[9];
#endif


extern u8 userClickFormat;
extern u8 siuOpMode;
extern u8 RepairASF;
extern BOOLEAN MemoryFullFlag;
//civic E
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 dcfDumpDir(struct FS_DIRENT*, u32);
s32 dcfDumpBuf(u8*, u32);
s32 dcfDumpDriveInfo(FS_DISKFREE_T*);
s32 dcfDirExist(s8*);
s32 dcfCreateTargetPath(s8*);
s32 dcfCreateFullPath(s8* pFullName ,s8* pDirName);

s32 dcfCreateCurPath(void);

s32 dcfDrive(s8*);
s32 dcfFormat(s8*);
s32 dcfDir(s8*, struct FS_DIRENT*, u32*,u8 IsUpdateEntrySect,int DoBadFile,unsigned int MaxEntry);
s32 dcfMkDir(s8*);
s32 dcfRmDir(s8*,u8 checkflag);
s32 dcfChDir(s8*);
FS_FILE* dcfOpen(s8*, s8*);
s32 dcfClose(FS_FILE*, u8*);
s32 dcfWrite(FS_FILE*, u8*, u32, u32*);
s32 dcfRead(FS_FILE*, u8*, u32, u32*);

s32 dcfDriveInfo(FS_DISKFREE_T*);
s32 dcfFindLastEofCluster(u32 StorageType);

s32 dcfGetDirEnt(s8*, struct FS_DIRENT*, s8*);
s32 dcfIncDirEnt(s8* pDirName, s32 IncEntNum);

#if CDVR_iHome_LOG_SUPPORT
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
extern int FS_fat_rename(char *pOldFilePath,char *pNewFileName,int index);

#if RX_SNAPSHOT_SUPPORT
extern int FS_SearchPhotoWholeDir(  FS_DIR *pDir,
                                         struct FS_DIRENT *dst_DirEnt,
                                         unsigned char* buffer, 
                                         unsigned int DirEntMax,
                                         unsigned int *pOldestEntry,
                                         int DoBadFile,
                                         int Year,
                                         int Month
                                     );
#endif


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
s32 dcfFsInit(void)
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

    default:
        return 0;
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
        DEBUG_DCF("0x%02x ", pBuf[i]);
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
s32 dcfCreateTargetPath(s8* pDirName)
{
    /*
        strcpy((char*)dcfCurDrive, "sdmmc:0:");
        strcpy((char*)dcfCurDir, "\\");
        strcpy((char*)dcfCurPath, "sdmmc:0:\\");
        strcpy((char*)dcfTargetPath, "sdmmc:0:\\");
    */

    strcpy((char*)dcfTargetPath, (const char*)dcfCurDrive); //sdcmmc:0:
    if (pDirName[0] == '\\')
    {   /* pDirName is an absolute path */
        strcat((char*)dcfTargetPath, (const char*)pDirName);
    }
    else
    {   /* pDirName is a relative path */
        if (strcmp((const char*)dcfCurDir, "\\") != 0)
            strcat((char*)dcfTargetPath, (const char*)dcfCurDir);
        strcat((char*)dcfTargetPath, "\\");
        strcat((char*)dcfTargetPath, (const char*)pDirName);
    }

    //DEBUG_DCF("Trace: dcfTargetPath = %s\n", dcfTargetPath);

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
s32 dcfCreateOWDelTargetPath(s8* pDirName, s8* TargetPath)
{
    if (TargetPath == NULL)
        return 0;

    strcpy((char*)TargetPath, (const char*)dcfCurDrive); //sdcmmc:0:
    if (pDirName[0] == '\\')
    {   /* pDirName is an absolute path */
        strcat((char*)TargetPath, (const char*)pDirName);
    }
    else
    {   /* pDirName is a relative path */
        if (strcmp((const char*)dcfDelDir, "\\") != 0)
            strcat((char*)TargetPath, (const char*)dcfDelDir);
        strcat((char*)TargetPath, "\\");
        strcat((char*)TargetPath, (const char*)pDirName);
    }

    //DEBUG_DCF("Trace: dcfTargetPath = %s\n", dcfTargetPath);

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
s32 dcfCreatePlaybackTargetPath(s8* pDirName, s8* TargetPath)
{
    if (TargetPath == NULL)
        return 0;

    strcpy((char*)TargetPath, (const char*)dcfCurDrive); //sdcmmc:0:
    if (pDirName[0] == '\\')
    {   /* pDirName is an absolute path */
        strcat((char*)TargetPath, (const char*)pDirName);
    }
    else
    {   /* pDirName is a relative path */
        if (strcmp((const char*)dcfPlayDir, "\\") != 0)
            strcat((char*)TargetPath, (const char*)dcfPlayDir);
        strcat((char*)TargetPath, "\\");
        strcat((char*)TargetPath, (const char*)pDirName);
    }

    //DEBUG_DCF("Trace: dcfTargetPath = %s\n", dcfTargetPath);

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
s32 dcfCreateFullPath(s8* pFullName ,s8* pDirName)
{
    /*
        strcpy((char*)dcfCurDrive, "sdmmc:0:");
        strcpy((char*)dcfCurDir, "\\");
        strcpy((char*)dcfCurPath, "sdmmc:0:\\");
        strcpy((char*)dcfTargetPath, "sdmmc:0:\\");
    */

    strcpy((char*)pFullName, (const char*)dcfCurDrive); //sdcmmc:0:
    if (pDirName[0] == '\\')
    {   /* pDirName is an absolute path */
        strcat((char*)pFullName, (const char*)pDirName);
    }
    else
    {   /* pDirName is a relative path */
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

    //DEBUG_DCF("Trace: dcfCurPath = %s\n", dcfCurPath);

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
	return dcfDirExist(pTargetPath);
}

s32 dcfDirExist(s8* pDirName)
{
    FS_DIR *pDir;

    pDir = FS_OpenDir((const char*)pDirName);
    if (pDir == NULL)
    {   /* directory not exists */
        return 0;
    }

    /* directory exists */
    FS_CloseDir(pDir);

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
    //Civic remark 070907
    //if (dcfStorageType == STORAGE_MEMORY_SMC_NAND) /*CY 1023*/
    //smcCacheBlockWrite();
    //Civic remark 070907
    strcpy((char*)dcfTargetPath, (const char*)pDriveName);
    strcat((char*)dcfTargetPath, "\\");
    if (dcfDirExist(dcfTargetPath) == 0)
    {
        DEBUG_DCF("Error: Change drive %s failed.\n", pDriveName);

        return 0;
    }

    strcpy((char*)dcfCurDrive, (const char*)pDriveName);
    strcpy((char*)dcfCurDir, "\\");
    strcpy((char*)dcfCurPath, (const char*)dcfTargetPath);
    //FS_IoCtl(dcfCurDrive, FS_CMD_CHK_DSKCHANGE, 0, 0); /* check dcfCurDrive periodically */
    //DEBUG_DCF("Trace: Change drive %s is successful.\n", pDriveName);

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
    DEBUG_DCF("dcfFormat\n");
    if (dcfStorageSize == 0)
    {
        DEBUG_DCF("Error: Unknown storage size\n");
    }
#if IS_COMMAX_DOORPHONE
    if (FS_IoCtl((const char*)pDriveName,FS_CMD_FORMAT_FAST,dcfStorageSize,0) != 0)
    {
        DEBUG_DCF("Error: Format %s failed\n", pDriveName);

        return 0;
    }
#else
    if (FS_IoCtl((const char*)pDriveName,FS_CMD_FORMAT_MEDIA,dcfStorageSize,0) != 0)
    {
        DEBUG_DCF("Error: Format %s failed\n", pDriveName);

        return 0;
    }
#endif
    dcfFileTypeCount_Clean();
    global_totalfile_count = 0;

    if(dcfCheckUnit()==0)
    {
        DEBUG_DCF("Error: dcfCheckUnit failed in Formatting\n");
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
s32 dcfPlaybackDir(s8* pDirName, struct FS_DIRENT *pDirEnt, u32* pDirCount,u8 IsUpdateEntrySect,int DoBadFile)
{
    FS_DIR* pDir;
    s8      PlayTargetPath[64];


    if (pDirName)
    {   /* directory path specified */
        dcfCreatePlaybackTargetPath(pDirName, PlayTargetPath);
    }
    else
    {   /* current directory */
        strcpy((char*)PlayTargetPath, (const char*)dcfPlayPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)PlayTargetPath);
    if (pDir)
    {   /* pDirName exists */

        *pDirCount=FS_ReadWholeDir(pDir,pDirEnt,dcfBuf,DCF_FILEENT_MAX,&dcfBadFileInfo,IsUpdateEntrySect,DoBadFile); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (*pDirCount==0)
        {
            DEBUG_DCF("Parsing Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);

        //DEBUG_DCF("Trace: Dir %s is successful.\n", dcfTargetPath);
        //dcfDumpDir(pDirEnt, *pDirCount);

        return 1;
    }
    else
    {   /* pDirName not exists */
        DEBUG_DCF("Error: Dir %s failed.\n", PlayTargetPath);

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
s32 dcfDir(s8* pDirName, struct FS_DIRENT *pDirEnt, u32* pDirCount,u8 IsUpdateEntrySect,int DoBadFile,unsigned int MaxEntry)
{
    FS_DIR* pDir;

    if (pDirName)
    {   /* directory path specified */
        dcfCreateTargetPath(pDirName);
    }
    else
    {   /* current directory */
        strcpy((char*)dcfTargetPath, (const char*)dcfCurPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)dcfTargetPath);
    if (pDir)
    {   /* pDirName exists */

        *pDirCount=FS_ReadWholeDir(pDir,pDirEnt,dcfBuf,MaxEntry,&dcfBadFileInfo,IsUpdateEntrySect,DoBadFile); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (*pDirCount==0)
        {
            DEBUG_DCF("Parsing Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);

        //DEBUG_DCF("Trace: Dir %s is successful.\n", dcfTargetPath);
        //dcfDumpDir(pDirEnt, *pDirCount);

        return 1;
    }
    else
    {   /* pDirName not exists */
        DEBUG_DCF("Error: Dir %s failed.\n", dcfTargetPath);

        return 0;
    }
}
/*
 return  0: FAIL
         1: SUCCESS
*/

s32 dcfOWDelDirScan(  s8* pDirName,
                    struct FS_DIRENT *pDirEnt,
                    s32* pDirCount,
                    unsigned int *pOldestEntry,
                    int DoBadFile)
{
    FS_DIR* pDir;
    s8              delTargetPath[64];

    if (pDirName)
    {   /* directory path specified */
        dcfCreateOWDelTargetPath(pDirName, delTargetPath);
    }
    else
    {   /* current directory */
        strcpy((char*)delTargetPath, (const char*)dcfDelPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)delTargetPath);
    if (pDir)
    {   /* pDirName exists */

        *pDirCount=FS_ScanWholeDir(pDir,pDirEnt,dcfBuf,DCF_FILEENT_MAX,pOldestEntry,DoBadFile); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (*pDirCount < 0)
        {
            DEBUG_DCF("Scan Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);

        //DEBUG_DCF("Trace: Dir %s is successful.\n", dcfTargetPath);
        //dcfDumpDir(pDirEnt, *pDirCount);

        return 1;
    }
    else
    {   /* pDirName not exists */
        DEBUG_DCF("Error: Dir-Scan %s failed.\n", delTargetPath);

        return 0;
    }
}

/*
 return  0: FAIL
         1: SUCCESS
*/

s32 dcfPlayDirScan(  s8* pDirName,
                    struct FS_DIRENT *pDirEnt,
                    s32* pDirCount,
                    unsigned int *pOldestEntry,
                    int DoBadFile)
{
    FS_DIR* pDir;
    s8      PlayTargetPath[64];

    if (pDirName)
    {   /* directory path specified */
        dcfCreatePlaybackTargetPath(pDirName, PlayTargetPath);
    }
    else
    {   /* current directory */
        strcpy((char*)PlayTargetPath, (const char*)dcfPlayPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)PlayTargetPath);
    if (pDir)
    {   /* pDirName exists */

        *pDirCount=FS_ScanWholeDir(pDir,pDirEnt,dcfBuf,DCF_FILEENT_MAX,pOldestEntry,DoBadFile); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (*pDirCount < 0)
        {
            DEBUG_DCF("Scan Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);

        //DEBUG_DCF("Trace: Dir %s is successful.\n", dcfTargetPath);
        //dcfDumpDir(pDirEnt, *pDirCount);

        return 1;
    }
    else
    {   /* pDirName not exists */
        DEBUG_DCF("Error: Dir-Scan %s failed.\n", PlayTargetPath);

        return 0;
    }
}

s32 dcfPlayDirSearch(s8* pDirName, struct FS_DIRENT *pDirEnt, s32* pDirCount, unsigned int *pOldestEntry, 
					char CHmap, char Typesel, u32 StartMin, u32 EndMin, int DoBadFile)
{
    FS_DIR* pDir;
    s8      PlayTargetPath[64];

    if (pDirName)
    {   /* directory path specified */
        dcfCreatePlaybackTargetPath(pDirName, PlayTargetPath);
    }
    else
    {   /* current directory */
        strcpy((char*)PlayTargetPath, (const char*)dcfPlayPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)PlayTargetPath);
    if (pDir)
    {   /* pDirName exists */

        *pDirCount=FS_SearchWholeDir(pDir,
                                     pDirEnt,
                                     dcfBuf,
                                     DCF_FILEENT_MAX,
                                     pOldestEntry,
                                     CHmap,
                                     Typesel,
                                     StartMin,
                                     EndMin,
                                     DoBadFile); 
        if (*pDirCount < 0)
        {
            DEBUG_DCF("Search Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);
        return 1;
    }
    else
    {   /* pDirName not exists */
        DEBUG_DCF("Error: Dir-Search %s failed.\n", PlayTargetPath);
        return 0;
    }
}


/*
 return  0: FAIL
         1: SUCCESS
*/

s32 dcfDirScan(  s8* pDirName,
                    struct FS_DIRENT *pDirEnt,
                    s32* pDirCount,
                    unsigned int *pOldestEntry,
                    int DoBadFile)
{
    FS_DIR* pDir;

    if (pDirName)
    {   /* directory path specified */
        dcfCreateTargetPath(pDirName);
    }
    else
    {   /* current directory */
        strcpy((char*)dcfTargetPath, (const char*)dcfCurPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)dcfTargetPath);
    if (pDir)
    {   /* pDirName exists */

        *pDirCount=FS_ScanWholeDir(pDir,pDirEnt,dcfBuf,DCF_FILEENT_MAX,pOldestEntry,DoBadFile); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (*pDirCount < 0)
        {
            DEBUG_DCF("Scan Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);

        //DEBUG_DCF("Trace: Dir %s is successful.\n", dcfTargetPath);
        //dcfDumpDir(pDirEnt, *pDirCount);

        return 1;
    }
    else
    {   /* pDirName not exists */
        DEBUG_DCF("Error: Dir-Scan %s failed.\n", dcfTargetPath);

        return 0;
    }
}


#if RX_SNAPSHOT_SUPPORT
s32 dcfPlayPhotoDirSearch(  s8* pDirName,
                                    struct FS_DIRENT *pDirEnt,
                                    s32* pDirCount,
                                    unsigned int *pOldestEntry,
                                    int DoBadFile,
                                    int Year,
                                    int Month)
{
    FS_DIR* pDir;

    if (pDirName)
    {   /* directory path specified */
        dcfCreateTargetPath(pDirName);
    }
    else
    {   /* current directory */
        strcpy((char*)dcfTargetPath, (const char*)dcfCurPath);
    }

    *pDirCount = 0;
    pDir = FS_OpenDir((const char*)dcfTargetPath);
    if (pDir)
    {   /* pDirName exists */

        *pDirCount=FS_SearchPhotoWholeDir(pDir,pDirEnt,dcfBuf,DCF_FILEENT_MAX,pOldestEntry,DoBadFile,Year,Month); //Lucian: parsing whole FDB of DIR to DirEnt.
        if (*pDirCount < 0)
        {
            DEBUG_DCF("Scan Directory Fail\n");
            FS_CloseDir(pDir);
            return 0;
        }
        FS_CloseDir(pDir);

        //DEBUG_DCF("Trace: Dir %s is successful.\n", dcfTargetPath);
        //dcfDumpDir(pDirEnt, *pDirCount);

        return 1;
    }
    else
    {   /* pDirName not exists */
        DEBUG_DCF("Error: Dir-Scan %s failed.\n", dcfTargetPath);

        return 0;
    }
}

#endif
/*

Routine Description:

    Make directory.

Arguments:

    pDirName - Directory name.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfMkDir(s8* pDirName)
{
    dcfCreateTargetPath(pDirName);

    if (FS_MkDir((const char *)dcfTargetPath) != 0)
    {
        DEBUG_DCF("Error: Mkdir %s failed\n", dcfTargetPath);

        return 0;
    }
#if !FS_NEW_VERSION
    global_diskInfo.avail_clusters -=1;
#endif
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
s32 dcfOWDelRmDir(s8* pDirName,u8 checkflag)
{
    s8              delTargetPath[64];

    dcfCreateOWDelTargetPath(pDirName, delTargetPath);
    if (FS_RmDir((const char*)delTargetPath,checkflag) != 0)
    {
        DEBUG_DCF("Error: Rmdir %s failed. Be sure Directory is empty\n", delTargetPath);

        return 0;
    }
    //Civic remark 070907
    //if (dcfStorageType == STORAGE_MEMORY_SMC_NAND) /*CY 1023*/
    //smcCacheBlockWrite();
    //Civic remark 070907
    //DEBUG_DCF("Trace: Rmdir %s is successful.\n", dcfTargetPath);

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
s32 dcfRmDir(s8* pDirName,u8 checkflag)
{
    dcfCreateTargetPath(pDirName);
    if (FS_RmDir((const char*)dcfTargetPath,checkflag) != 0)
    {
        DEBUG_DCF("Error: Rmdir %s failed. Be sure Directory is empty\n", dcfTargetPath);

        return 0;
    }
    //Civic remark 070907
    //if (dcfStorageType == STORAGE_MEMORY_SMC_NAND) /*CY 1023*/
    //smcCacheBlockWrite();
    //Civic remark 070907
    //DEBUG_DCF("Trace: Rmdir %s is successful.\n", dcfTargetPath);

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
    {   /* change to current directory */
        //DEBUG_DCF("Trace: Chdir %s is successful.\n", pDirName);

        return 1;
    }
    else if (strcmp((const char*)pDirName, "..") == 0)
    {   /* change to parent directory */
        if (strcmp((const char*)dcfDelDir, "\\") == 0)
        {   /* current directory is root directory */
            DEBUG_DCF("Trace: Chdir %s failed - this is a root directory.\n", pDirName);

            return 0;
        }
        else
        {   /* current directory is not root directory */
            curDirPos = strlen((const char*)dcfDelDir);
            while (curDirPos--)
            {
                if (dcfDelDir[curDirPos] == '\\')
                {   /* back slash of parent directory is found */
                    if (curDirPos != 0)
                    {   /* parent directory is not root directory */
                        dcfDelDir[curDirPos] = '\0';
                    }
                    break;
                }
                else
                {   /* clear current directory until back slash of parent directory */
                    dcfDelDir[curDirPos] = '\0';
                }
            }
        }
    }
    else
    {   /* change to a general directory */
        if (pDirName[0] == '\\')
        {   /* change to an absolute path */
            strcpy((char*)dcfDelDir, (const char*)pDirName);
        }
        else
        {   /* change to a relative path */
            if (strcmp((const char*)dcfDelDir, "\\") != 0)
            {   /* current directory is not root directory */
                strcat((char*)dcfDelDir, "\\");
            }
            strcat((char*)dcfDelDir, (const char*)pDirName);
        }
    }

    dcfCreateOWDelCurPath();
    if (dcfDirExist(dcfDelPath) == 0)
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
s32 dcfChPlayDir(s8* pDirName)
{
    s8 backupCurDir[64];
    u8 curDirPos = 0;

    strcpy((char*)backupCurDir, (const char*)dcfPlayDir);
    if (strcmp((const char*)pDirName, ".") == 0)
    {   /* change to current directory */
        //DEBUG_DCF("Trace: Chdir %s is successful.\n", pDirName);

        return 1;
    }
    else if (strcmp((const char*)pDirName, "..") == 0)
    {   /* change to parent directory */
        if (strcmp((const char*)dcfPlayDir, "\\") == 0)
        {   /* current directory is root directory */
            DEBUG_DCF("Trace: Chdir %s failed - this is a root directory.\n", pDirName);

            return 0;
        }
        else
        {   /* current directory is not root directory */
            curDirPos = strlen((const char*)dcfPlayDir);
            while (curDirPos--)
            {
                if (dcfPlayDir[curDirPos] == '\\')
                {   /* back slash of parent directory is found */
                    if (curDirPos != 0)
                    {   /* parent directory is not root directory */
                        dcfPlayDir[curDirPos] = '\0';
                    }
                    break;
                }
                else
                {   /* clear current directory until back slash of parent directory */
                    dcfPlayDir[curDirPos] = '\0';
                }
            }
        }
    }
    else
    {   /* change to a general directory */
        if (pDirName[0] == '\\')
        {   /* change to an absolute path */
            strcpy((char*)dcfPlayDir, (const char*)pDirName);
        }
        else
        {   /* change to a relative path */
            if (strcmp((const char*)dcfPlayDir, "\\") != 0)
            {   /* current directory is not root directory */
                strcat((char*)dcfPlayDir, "\\");
            }
            strcat((char*)dcfPlayDir, (const char*)pDirName);
        }
    }

    dcfCreatePlayCurPath();
    if (dcfDirExist(dcfPlayPath) == 0)
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
s32 dcfChDir(s8* pDirName)
{
    s8 backupCurDir[64];
    u8 curDirPos = 0;

    strcpy((char*)backupCurDir, (const char*)dcfCurDir);
    if (strcmp((const char*)pDirName, ".") == 0)
    {   /* change to current directory */
        //DEBUG_DCF("Trace: Chdir %s is successful.\n", pDirName);

        return 1;
    }
    else if (strcmp((const char*)pDirName, "..") == 0)
    {   /* change to parent directory */
        if (strcmp((const char*)dcfCurDir, "\\") == 0)
        {   /* current directory is root directory */
            DEBUG_DCF("Trace: Chdir %s failed - this is a root directory.\n", pDirName);

            return 0;
        }
        else
        {   /* current directory is not root directory */
            curDirPos = strlen((const char*)dcfCurDir);
            while (curDirPos--)
            {
                if (dcfCurDir[curDirPos] == '\\')
                {   /* back slash of parent directory is found */
                    if (curDirPos != 0)
                    {   /* parent directory is not root directory */
                        dcfCurDir[curDirPos] = '\0';
                    }
                    break;
                }
                else
                {   /* clear current directory until back slash of parent directory */
                    dcfCurDir[curDirPos] = '\0';
                }
            }
        }
    }
    else
    {   /* change to a general directory */
        if (pDirName[0] == '\\')
        {   /* change to an absolute path */
            strcpy((char*)dcfCurDir, (const char*)pDirName);
        }
        else
        {   /* change to a relative path */
            if (strcmp((const char*)dcfCurDir, "\\") != 0)
            {   /* current directory is not root directory */
                strcat((char*)dcfCurDir, "\\");
            }
            strcat((char*)dcfCurDir, (const char*)pDirName);
        }
    }

    dcfCreateCurPath();
    if (dcfDirExist(dcfCurPath) == 0)
    {
        DEBUG_DCF("Error: Chdir %s failed.\n", dcfCurPath);
        DEBUG_DCF("SYS_CTL0 = 0x%08x\n", SYS_CTL0);

        /* rewind to backup direcotry */
        strcpy((char*)dcfCurDir, (const char*)backupCurDir);
        dcfCreateCurPath();

        return 0;
    }

    //DEBUG_DCF("Trace: Chdir %s is successful.\n", dcfCurPath);

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
FS_FILE* dcfOpen(s8* pFileName, s8* pMode)
{
    FS_FILE *pFile;
    s8      CurrTargetPath[64];
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
    if(!strcmp((char*)pMode,"r"))
        dcfCreatePlaybackTargetPath(pFileName, CurrTargetPath);
    else
    {
        dcfCreateTargetPath(pFileName);
        strcpy((char*)CurrTargetPath, (const char*)dcfTargetPath);
    }
#else
    dcfCreateTargetPath(pFileName);
    strcpy((char*)CurrTargetPath, (const char*)dcfTargetPath);
#endif
    pFile = FS_FOpen((const char*)CurrTargetPath, (const char*)pMode); /* open file */
    if (pFile == NULL)
    {   /* file open failed */
        DEBUG_DCF("Error: Open %s failed.\n", CurrTargetPath);

        return NULL;
    }

    /* file open is successful */
    //DEBUG_DCF("DcfOpen %s (0x%08x) is successful.\n", CurrTargetPath, (u32)pFile);

    dcfBufRemain = DCF_BUF_SIZE;

    return pFile;
}


int dcfOWDelRename(s8* pNewFileName,s8* pOldFileName)
{
    char *s;
    int idx;
    s8              delTargetPath[64];

    dcfCreateOWDelTargetPath(pOldFileName, delTargetPath);
    idx = FS__find_fsl(delTargetPath, &s);
    if (idx < 0)
    {
        return 0;  /* Device not found */
    }

    if( FS_fat_rename(s,pNewFileName,idx) < 0)
       return 0;


    return 1;

}


int dcfRename(s8* pNewFileName,s8* pOldFileName)
{
    char *s;
    int idx;

    dcfCreateTargetPath(pOldFileName);
    idx = FS__find_fsl(dcfTargetPath, &s);
    if (idx < 0)
    {
        return 0;  /* Device not found */
    }

    if( FS_fat_rename(s,pNewFileName,idx) < 0)
       return 0;


    return 1;

}

s32 dcfDump(void)
{
    DCF_LIST_FILEENT* cur;

    DEBUG_DCF("Trace: DCF dump\n");

    cur= dcfListFileEntHead;

    do
    {
        DEBUG_DCF("%s\n", cur->pDirEnt->d_name);
        cur=cur->next;
    }while( cur != dcfListFileEntHead);

    return 1;
}



/*

Routine Description:

    Close file.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfClose(FS_FILE* pFile, u8* pOpenFile)
{

    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster;
    u32 bytes_per_cluster;
	#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
	#endif

    dcfFlushTempBuf(pFile);

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
#if !FS_NEW_VERSION
        if (diskInfo->avail_clusters >= used_cluster)
            diskInfo->avail_clusters -= used_cluster;
        else
            diskInfo->avail_clusters = diskInfo->avail_clusters ;
#endif
		*pOpenFile = 0;
		OS_EXIT_CRITICAL();
        DEBUG_FS("Available Cluster=%d\n",global_diskInfo.avail_clusters);
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
    u8 err;
    s32 ret;
    u32 uWriteDataSize, uSize, uSize1;
    int TimeVal, i;

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
    #if (IIS_TEST || H264_TEST || ASF_SPLIT_FILE)
    if(1)
    #else
    if (gInsertCard ||gInsertNAND)
    #endif
    {
        //-----------------Video Mode Use (Optimized)------------------------//
#if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_ASF)
        //#if (IIS_TEST || H264_TEST || ASF_SPLIT_FILE)
        #if 1   // 邊錄邊放一定要用這個
        if(1)
        #else   // 沒有邊錄邊放可以用這個
        if (!dcfWriteFromBuffer && ((siuOpMode == SIUMODE_MPEGAVI) || (sysCameraMode == SYS_CAMERA_MODE_PLAYBACK) || (RepairASF == 1))) //只有在Video Clip, 才直接寫入
        #endif
        {
            OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
        	TimeVal = OSTimeGet();
        	*pWriteSize = FS_FWrite(pData, sizeof(u8), dataSize, pFile); /* write to file */
        	TimeVal = OSTimeGet() - TimeVal;
        	OSSemPost(dcfReadySemEvt);
        	if(TimeVal > 1)
        	{
        		DEBUG_DCF("[W] DCF WrTick: %d (x50ms)\n", TimeVal);
        	}
        	if (*pWriteSize < dataSize)
        	{
        		/* log error */
        		ret = FS_FError(pFile);
        		DEBUG_DCF("Error: Write 0x%08x failed - error number is %x.\n", (u32)pFile, ret);
        		return 0;
        	}

#if SDC_DEBUG_ENA
            gpioSetLevel(0, 12, 0 );
#endif

            return 1;
        }
#endif  // #if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_ASF)
        DEBUG_DCF("Warning dcfWrite() not in video mode!!\n");
        DEBUG_DCF("dcfWriteFromBuffer   = %d.\n", dcfWriteFromBuffer);
        DEBUG_DCF("siuOpMode            = %d.\n", siuOpMode);
        DEBUG_DCF("sysCameraMode        = %d.\n", sysCameraMode);
        DEBUG_DCF("RepairASF            = %d.\n", RepairASF);

        //-----------------Capture Mode Use------------------------//
        uSize1  = dataSize;
        /* A <= dcfBufRemain */
        if (dataSize <= dcfBufRemain)
        {
            CopyMemory(dcfBuf + DCF_BUF_SIZE - dcfBufRemain, pData, dataSize);
            dcfBufRemain -= dataSize;
        }
        /* A > dcfBufRemain */
        else
        {
            /* Buffer is empty */
            if (dcfBufRemain == DCF_BUF_SIZE)
            {
                if(( (u32)pData & 0x0000000f)==0)
                {
                    uWriteDataSize  = (dataSize / DCF_BUF_SIZE) * DCF_BUF_SIZE;
                    uSize           = FS_FWrite(pData, sizeof(u8), uWriteDataSize, pFile); /* write to file */
                    if (uSize < uWriteDataSize)
                    {   /* log error */
                        ret = FS_FError(pFile);
                        DEBUG_DCF("Error: Write 0x%08x failed - error number is %x.\n", (u32)pFile, ret);

                        return 0;
                    }
                    pData += uWriteDataSize;
                    dataSize -= uWriteDataSize;
                }
                else
                {
                    while(dataSize>=DCF_BUF_SIZE)
                    {
                        CopyMemory(dcfBuf, pData, DCF_BUF_SIZE);
                        uSize = FS_FWrite(dcfBuf, sizeof(u8), DCF_BUF_SIZE, pFile); /* write to file */
                        if (uSize < DCF_BUF_SIZE)
                        {   /* log error */
                            ret = FS_FError(pFile);
                            DEBUG_DCF("Error: Write 0x%08x failed - error number is %x.\n", (u32)pFile, ret);

                            return 0;
                        }

                        pData += DCF_BUF_SIZE;
                        dataSize -=DCF_BUF_SIZE;
                    }
                }

                CopyMemory(dcfBuf, pData, dataSize);
                dcfBufRemain = DCF_BUF_SIZE - dataSize;

            }
            /* Buffer is not empty */
            else
            {
                CopyMemory(dcfBuf + DCF_BUF_SIZE - dcfBufRemain, pData, dcfBufRemain);
                uSize   = FS_FWrite(dcfBuf, sizeof(u8), DCF_BUF_SIZE, pFile); /* write to file */
                if (uSize < DCF_BUF_SIZE)
                {   /* log error */
                    ret = FS_FError(pFile);
                    DEBUG_DCF("Error: Write 0x%08x failed - error number is %x.\n", (u32)pFile, ret);

                    return 0;
                }
                dataSize -= dcfBufRemain;
                pData += dcfBufRemain;
                dcfBufRemain = DCF_BUF_SIZE;
                /* A" < dcfBufRemain */
                if (dataSize < dcfBufRemain)
                {
                    CopyMemory(dcfBuf, pData, dataSize);
                    dcfBufRemain -= dataSize;
                }
                /* A" >= dcfBufRemain */
                else
                {
                    if(( (u32)pData & 0x0000000f)==0)
                    {
                        uWriteDataSize = (dataSize / DCF_BUF_SIZE) * DCF_BUF_SIZE;
                        uSize   = FS_FWrite(pData, sizeof(u8), uWriteDataSize, pFile); /* write to file */
                        if (uSize < uWriteDataSize)
                        {   /* log error */
                            ret = FS_FError(pFile);
                            DEBUG_DCF("Error: Write 0x%08x failed - error number is %x.\n", (u32)pFile, ret);

                            return 0;
                        }
                        pData += uWriteDataSize;
                        dataSize -= uWriteDataSize;
                    }
                    else
                    {
                       while(dataSize>=DCF_BUF_SIZE)
                       {
                            CopyMemory(dcfBuf, pData, DCF_BUF_SIZE);
                            uSize     = FS_FWrite(dcfBuf, sizeof(u8), DCF_BUF_SIZE, pFile); /* write to file */
                            if (uSize < DCF_BUF_SIZE)
                            {   /* log error */
                                ret = FS_FError(pFile);
                                DEBUG_DCF("Error: Write 0x%08x failed - error number is %x.\n", (u32)pFile, ret);

                                return 0;
                            }

                            pData += DCF_BUF_SIZE;
                            dataSize -=DCF_BUF_SIZE;
                        }
                    }
                    CopyMemory(dcfBuf, pData, dataSize);
                    dcfBufRemain = DCF_BUF_SIZE - dataSize;
                }
            }
        }
    }
    //DEBUG_DCF("Trace: Write 0x%08x is successful.\n", (u32)pFile);
#if SDC_DEBUG_ENA
    gpioSetLevel(0, 12, 0 );
#endif

    *pWriteSize = uSize1;
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
    s32 err;

    *pReadSize = FS_FRead(pData, sizeof(u8), dataSize, pFile); /* read from file */
    if (*pReadSize < dataSize)
    {   /* log error */
        err = FS_FError(pFile);
        if (err == FS_ERR_EOF)
        {   /* EOF */
            //DEBUG_DCF("Trace: Read 0x%08x is successful - EOF is reached.\n", (u32)pFile);
            //dcfDumpBuf(pData, *pReadSize);

            return 2;
        }
        else
        {
            DEBUG_DCF("Error: Read 0x%08x failed - error number is %d.\n", (u32)pFile, err);

            return 0;
        }
    }

    //DEBUG_DCF("Trace: Read 0x%08x is successful.\n", (u32)pFile);
    //dcfDumpBuf(pData, *pReadSize);

    return 1;
}

s32 dcfSetDelDir(s8* pDirName)
{

    if (pDirName[0] == '\\')
    {   /* change to an absolute path */
        strcpy((char*)dcfDelDir, (const char*)pDirName);
    }
    else
    {   /* change to a relative path */
        if (strcmp((const char*)dcfDelDir, "\\") != 0)
        {   /* current directory is not root directory */
            strcat((char*)dcfDelDir, "\\");
        }
        strcat((char*)dcfDelDir, (const char*)pDirName);
    }
    return 1;
}

s32 dcfOWEnable(void)
{
    u32 free_size;

    free_size = global_diskInfo.avail_clusters * (global_diskInfo.bytes_per_sector / 512) / 2;

    while((free_size < DCF_OVERWRITE_THR_KBYTE) || (global_totaldir_count > (DCF_DIRENT_MAX - 2)) )
    {
        // Find the oldest file pointer and delete it
        if(dcfOverWriteDel()==0)
        {
            DEBUG_DCF("Over Write delete fail!!\n");
            return 0;
        }
        else
        {
            //DEBUG_ASF("Over Write delete Pass!!\n");
        }
        free_size = global_diskInfo.avail_clusters * (global_diskInfo.bytes_per_sector / 512) / 2;
        DEBUG_DCF("Free Space=%d (KBytes) \n", free_size);
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
#if FS_NEW_VERSION
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

	if (MemoryFullFlag == TRUE)
    {
        FS_DISKFREE_T *diskInfo = &global_diskInfo;
        u32 clusterSize = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        u32 free_size = diskInfo->avail_clusters * (clusterSize / 1024); //KByte unit
            
        if(free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM)) / 1024) //Notice: K-Byte unit
        {
            MemoryFullFlag = TRUE;
			sysProjectDeviceStatus(DEV_SD_FULL);
        }
        else
        {
            MemoryFullFlag = FALSE;
			sysProjectDeviceStatus(DEV_SD_NOT_FULL);
        }
    }

	DEBUG_DCF("Free Space = %d (KBytes)\n", global_diskInfo.avail_clusters * global_diskInfo.bytes_per_sector * global_diskInfo.sectors_per_cluster / 1024);
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

	if (MemoryFullFlag == TRUE)
    {
        FS_DISKFREE_T *diskInfo = &global_diskInfo;
        u32 clusterSize = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        u32 free_size = diskInfo->avail_clusters * (clusterSize / 1024); //KByte unit
            
        if(free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM)) / 1024) //Notice: K-Byte unit
        {
            MemoryFullFlag = TRUE;
			sysProjectDeviceStatus(DEV_SD_FULL);
        }
        else
        {
            MemoryFullFlag = FALSE;
			sysProjectDeviceStatus(DEV_SD_NOT_FULL);
        }
    }
    
	DEBUG_DCF("Free Space = %d (KBytes)\n", global_diskInfo.avail_clusters * global_diskInfo.bytes_per_sector * global_diskInfo.sectors_per_cluster / 1024);
	return ret;
}

int dcfPlayDel(char *pDirName, char *pFileName)
{
	FS_DIR *pDir;
	FS_DeleteCondition pCondition;
	char PlayTargetPath[64];
	int ret;

	pCondition.DeleteMode = FS_E_DELETE_TYPE_ORDER;
	
	//dcfCreatePlaybackTargetPath(NULL, PlayTargetPath);
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

	if (MemoryFullFlag == TRUE)
    {
        FS_DISKFREE_T *diskInfo = &global_diskInfo;
        u32 clusterSize = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        u32 free_size = diskInfo->avail_clusters * (clusterSize / 1024); //KByte unit
            
        if(free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM)) / 1024) //Notice: K-Byte unit
        {
            MemoryFullFlag = TRUE;
			sysProjectDeviceStatus(DEV_SD_FULL);
        }
        else
        {
            MemoryFullFlag = FALSE;
			sysProjectDeviceStatus(DEV_SD_NOT_FULL);
        }
    }

	DEBUG_DCF("Free Space = %d (KBytes)\n", global_diskInfo.avail_clusters * global_diskInfo.bytes_per_sector * global_diskInfo.sectors_per_cluster / 1024);
	return ret;
}

int dcfVideoDel(FS_FILE *pFile)
{
#if (HW_BOARD_OPTION != MR8211B_TX_RDI_WD542I)
	int i; 

	for(i = dcfFileCount; i >= 0; i--)
	{
		if(pFile->fileid_hi == dcfFileEnt[i].d_ino)
			break;
	}

	if(i < 0)
		return -1;

	return dcfDel(dcfPlaybackCurDir->pDirEnt->d_name, dcfFileEnt[i].d_name);
#else
	return 0;
#endif
}

#else
s32 dcfOWDel(s8* pFileName,u16 EstFileEntrySect)
{
	FS_DISKFREE_T   *diskInfo;
    u32 bytes_per_cluster, free_size, used_cluster;
    s8 delTargetPath[64];
    int ret;
#if (OS_CRITICAL_METHOD == 3)                      
    unsigned int  cpu_sr = 0;	// Allocate storage for CPU status register to prevent warning when complier execute
#endif
	
	used_cluster = 1;

    dcfCreateOWDelTargetPath(pFileName, delTargetPath);
    
    if ((ret = _FS_Remove((const char*)delTargetPath, EstFileEntrySect, &used_cluster, &global_diskInfo)) <= 0)
    {   
    	// file remove failed
        DEBUG_DCF("Error: Remove %s failed.\n", delTargetPath);
        return ret;
    }

    if (MemoryFullFlag == TRUE)
    {
        diskInfo            = &global_diskInfo;
        bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit
		OS_ENTER_CRITICAL();
        if (diskInfo->avail_clusters >= used_cluster)
            diskInfo->avail_clusters -= used_cluster;
        else
            diskInfo->avail_clusters = diskInfo->avail_clusters ;
        OS_EXIT_CRITICAL();
            
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

/*

Routine Description:

    Delete file.

Arguments:

    pFileName - File name.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 dcfDel(s8* pFileName,u16 EstFileEntrySect)
{
	FS_DISKFREE_T *diskInfo;
    u32 bytes_per_cluster, free_size, used_cluster;
    int ret;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
	unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    used_cluster = 1;	// use this params to enable calculation

    dcfCreateTargetPath(pFileName);

    if ((ret = _FS_Remove((const char*)dcfTargetPath, EstFileEntrySect, &used_cluster, &global_diskInfo)) <= 0)
    {   
    	// file remove failed
        DEBUG_DCF("Error: Remove %s failed.\n", dcfTargetPath);
        return ret;
    }

    if (MemoryFullFlag == TRUE)
    {
        diskInfo            = &global_diskInfo;
        bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit

		OS_ENTER_CRITICAL();
		if (diskInfo->avail_clusters >= used_cluster)
			diskInfo->avail_clusters -= used_cluster;
		else
			diskInfo->avail_clusters = diskInfo->avail_clusters ;
		OS_EXIT_CRITICAL();
        
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

s32 dcfPlayDel(s8* pFileName,u16 EstFileEntrySect)
{
	FS_DISKFREE_T   *diskInfo;
    u32 bytes_per_cluster, free_size, used_cluster;
    s8      PlayTargetPath[64];
    int ret;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
	unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    used_cluster = 1;

    if (pFileName)
    {   /* directory path specified */
        dcfCreatePlaybackTargetPath(pFileName, PlayTargetPath);
    }

    if ((ret = _FS_Remove((const char*)PlayTargetPath, EstFileEntrySect, &used_cluster, &global_diskInfo)) <= 0)
    {   
    	// file remove failed
        DEBUG_DCF("Error: Remove %s failed.\n", PlayTargetPath);
        return ret;
    }
    
    DEBUG_DCF("Remove %s OK.\n", pFileName);
    if (MemoryFullFlag == TRUE)
    {
    	OS_ENTER_CRITICAL();
        diskInfo            = &global_diskInfo;
        bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit

        
		if (diskInfo->avail_clusters >= used_cluster)
			diskInfo->avail_clusters -= used_cluster;
		else
			diskInfo->avail_clusters = diskInfo->avail_clusters ;
		OS_EXIT_CRITICAL();

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

int dcfVideoDel(FS_FILE *pFile)
{
#if (HW_BOARD_OPTION != MR8211B_TX_RDI_WD542I)
	int i; 

	for(i = dcfFileCount; i >= 0; i--)
	{
		if(pFile->fileid_hi == dcfFileEnt[i].d_ino)
			break;
	}

	if(i < 0)
		return -1;

	return dcfDel(dcfFileEnt[i].d_name, pFile->FileEntrySect);
#else
	return 0;
#endif
}
#endif



#if CDVR_iHome_LOG_SUPPORT
s32 dcfLogDel(s8* pFileName,u16 EstFileEntrySect)
{
	FS_DISKFREE_T   *diskInfo;
    u32 free_size, bytes_per_cluster, used_cluster;
    s8 LogTargetPath[64];
    int ret;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
	unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    used_cluster = 1;

    if (pFileName)
    {   /* directory path specified */
        dcfCreateLogTargetPath(pFileName, LogTargetPath);
    }

    if ((ret = _FS_Remove((const char*)LogTargetPath, EstFileEntrySect, &used_cluster, &global_diskInfo)) <= 0)
    {   
    	// file remove failed
        DEBUG_DCF("Error: Remove %s failed.\n", LogTargetPath);
        return ret;
    }
    DEBUG_DCF("Remove %s OK.\n", pFileName);
    if (MemoryFullFlag == TRUE)
    {

        diskInfo            = &global_diskInfo;
        bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit
#if !FS_NEW_VERSION
		OS_ENTER_CRITICAL();
		if (diskInfo->avail_clusters >= used_cluster)
			diskInfo->avail_clusters -= used_cluster;
		else
			diskInfo->avail_clusters = diskInfo->avail_clusters ;
		OS_EXIT_CRITICAL();
#endif 

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

s32 dcfCreateLogTargetPath(s8* pDirName, s8* TargetPath)
{
    if (TargetPath == NULL)
        return 0;

    strcpy((char*)TargetPath, (const char*)dcfCurDrive); //sdcmmc:0:
    if (pDirName[0] == '\\')
    {   /* pDirName is an absolute path */
        strcat((char*)TargetPath, (const char*)pDirName);
    }
    else
    {   /* pDirName is a relative path==> device:unit\\LOG\\  */
        strcat((char*)TargetPath, "\\");
        strcat((char*)TargetPath, gsLogDirName);
        strcat((char*)TargetPath, "\\");
        strcat((char*)TargetPath, (const char*)pDirName);
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
s32 dcfDriveInfo(FS_DISKFREE_T* pInfo)
{
  #if FILE_SYSTEM_DVF_TEST
    u32 time1,time2;
  #endif

  #if FILE_SYSTEM_DVF_TEST
     time1=OSTimeGet();
  #endif
    if (FS_IoCtl((const char*)dcfCurDrive, FS_CMD_GET_DISKFREE, 0, (void*)pInfo) != 0)
    {
        DEBUG_DCF("Error: Get drive %s information failed.\n", dcfCurDrive);

        return 0;
    }
  #if FILE_SYSTEM_DVF_TEST
    time2=OSTimeGet();
    DEBUG_DCF("--->dcfDriveInfo Time=%d (x100ms)\n",time2-time1);
  #endif
    //DEBUG_DCF("Trace: Get drive %s information is successful.\n", dcfCurDrive);
    dcfDumpDriveInfo(pInfo);

    return 1;
}


/**********************************

Routine Description:

    Findout Free cluster in disk.

Arguments:

    pInfo - None.

Return Value:

    0 - Failure.
    1 - Success.

************************************/
s32 dcfFindLastEofCluster(u32 StorageType)
{
   u32 unit;
   int Idx;
   int freecluster;
   char *s;

#if 1//( (HW_BOARD_OPTION == MMR6720_EBELL) || ((HW_BOARD_OPTION == ELEGANT_KFCDVR)) ) //Lucian:找尋最後寫入的檔案,其所屬的cluster,可解決開關機10000次所產生的問題.
  if(dcfListFileEntTail==NULL)
  {
     DEBUG_DCF("dcfListFileEntTail=NULL\n");
     dcfLasfEofCluster=-1;
  }
  else
  {
     dcfCreateTargetPath(dcfListFileEntTail->pDirEnt->d_name);
     Idx = FS__find_fsl(dcfTargetPath, &s);
     dcfLasfEofCluster=FS__fat_FindLastFileCluster(Idx,s);
  }
#else //找尋空的cluster
   unit=0; //unit is always 0 in our system.
   Idx = FS__find_fsl((const char *)dcfTargetPath, &s);

   freecluster=FS__fat_FindLastFreeCluster(Idx, unit);
   dcfLasfEofCluster=freecluster-1;
#endif
   return dcfLasfEofCluster;
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
s32 dcfGetDirEnt(s8* pDirName, struct FS_DIRENT* pDirEnt, s8* pSubDirName)
{
    FS_DIR* pDir;
    struct FS_DIRENT* pEnt;

    if (pDirName)
    {   /* directory path specified */
        dcfCreateTargetPath(pDirName);
    }
    else
    {   /* current directory */
        strcpy((char*)dcfTargetPath, (const char*)dcfCurPath);
    }

    pDir = FS_OpenDir((const char*)dcfTargetPath);
    if (pDir)
    {   /* pDirName exists */
        do
        {
            pEnt = FS_ReadDir(pDir);
            if (pEnt)
            {
                //DEBUG_DCF("Trace: Getdirent %s , %s \n", pEnt->d_name, pSubDirName);
                /* extension of dcf directory name is composed of space characters */
                if (strcmp(&pEnt->d_name[0], (const char*)pSubDirName) != 0)
                    continue;

                memcpy(pDirEnt, pEnt, sizeof(struct FS_DIRENT));
                FS_CloseDir(pDir);

                //DEBUG_DCF("Trace: Getdirent %s of %s is successful.\n", pSubDirName, dcfTargetPath);

                return 1;
            }
        }
        while (pEnt);

        FS_CloseDir(pDir);

        DEBUG_DCF("Error1: Getdirent %s of %s failed.\n", pSubDirName, dcfTargetPath);

        return 0;
    }
    else
    {   /* pDirName not exists */
        DEBUG_DCF("Error2: Getdirent %s of %s failed\n", pSubDirName, dcfTargetPath);

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
s32 dcfIncDirEnt(s8* pDirName, s32 IncEntNum)
{
   int err;
   int idx;
   u32 dstart;
   u32 dsize;
   u32 unit;
   char *fname;
   int IncSect;
   char *PathName;
   s8 FullName[64];
   unsigned char SecPerClus;
   int AllocFatNum;

   dcfCreateFullPath(FullName,pDirName); //make full path.
   DEBUG_DCF("FULL Path=%s\n",FullName);

   idx=FS__find_fsl((const char *)FullName, &PathName);
   DEBUG_DCF("File Path=%s,Device index=%d\n",PathName,idx);

   strcat((char*)PathName, "\\"); //Lucian: 讓它找到100VIDEO
   dsize = FS__fat_findpath(idx, PathName, &fname, &unit, &dstart);
   if(dsize==0)
   {
     DEBUG_DCF("FS__fat_findpath Fail!!\n");
     return 0;
   }
   DEBUG_DCF("fname=%s,unit=%d,dstart=%d,dsize=%d\n",fname,unit,dstart,dsize);

   IncSect= (IncEntNum*32/512)-dsize;  //將directory entry number 換算成所需增加的 sector size.

   if(IncSect >0)
      err = _FS_fat_IncDir_Multi(idx, unit, dstart, &dsize,(u32)IncSect);
   else
      err =1;

   if(err<=0)
   {
     DEBUG_DCF("Increase Dir size Fail!!\n");
     return 0;
   }
   else
   {
      SecPerClus = global_diskInfo.sectors_per_cluster;
      if(SecPerClus==0)
      {
         DEBUG_DCF("Warning! SecPerClus=0\n");
      }
      AllocFatNum=(IncSect + SecPerClus -1 )/SecPerClus; //Lucian: 轉換成cluster unit.(無條件進位)
#if !FS_NEW_VERSION
      global_diskInfo.avail_clusters -= AllocFatNum;
#endif
   }

   dsize = FS__fat_findpath(idx, PathName, &fname, &unit, &dstart);
   if(dsize==0)
   {
     DEBUG_DCF("FS__fat_findpath Fail!!\n");
     return 0;
   }
   DEBUG_DCF("fname=%s,unit=%d,dstart=%d,dsize=%d\n",fname,unit,dstart,dsize);

   err=FS_LB_Cache_Clean(idx, unit); //write back to SD card.
   if(err<0)
   {
      DEBUG_DCF("FS_LB_Cache_Clean is Fail\n");
   }

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
        WriteSize = FS_FWrite(dcfBuf, sizeof(u8), DCF_BUF_SIZE - dcfBufRemain, pFile); /* write to file */
        if (WriteSize < (DCF_BUF_SIZE - dcfBufRemain)) // modify by Peter 2007/10/09
        {   /* log error */
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

int dcfSeek(FS_FILE *pFile, FS_i32 Offset, int Whence)
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
    //Lucian: 要做Seek前,必須先清空temp buffer. 注意: 做seek 會嚴重降低performance.
    if (dcfFlushTempBuf(pFile)==0)
    {
        DEBUG_DCF("dcfSeek Error: FlushTempBuf Fail\n");
        return 0;
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

    if (pFile->mode_w) //Lucian: 因DCF layer Temp buffer存在,回報 buffer flush 後的位置.
    {
        return filepos + DCF_BUF_SIZE - dcfBufRemain;
    }
    else
    {
        return filepos;
    }

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
s32 dcfCacheClean(void)
{
   u32 unit;
   int Idx,err;
   char *s;

   unit=0; //unit is always 0 in our system.

   DEBUG_DCF("dcfCacheClean: dcfTargetPath= %s \n",dcfTargetPath);
   Idx = FS__find_fsl((const char *)dcfTargetPath, &s);

   err=FS_LB_Cache_Clean(Idx, unit); //write back to SD card.
   if(err<0)
   {
      DEBUG_DCF("FS_LB_Cache_Clean is Fail\n");
	  return 0;
   }


   return 1;
}

s32 dcfCacheClear(void)
{
   u32 unit;
   int Idx,err;
   char *s;

   unit=0; //unit is always 0 in our system.

   DEBUG_DCF("dcfCacheClear: dcfTargetPath= %s \n",dcfTargetPath);
   Idx = FS__find_fsl((const char *)dcfTargetPath, &s);

   err=FS_LB_Cache_Clear(Idx, unit); //Clear DRAM cache.
   if(err<0)
   {
      DEBUG_DCF("FS_LB_Cache_Clear is Fail\n");
   return 0;
   }


   return 1;
}
