/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	uartapi.h

Abstract:

   	The application interface of the file system.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __FS_API_H__
#define __FS_API_H__

// Use the assert define
#ifndef NOHALT_ASSERT
    #define NOHALT_ASSERT
#endif
#ifndef LOG_LEVEL
    #define LOG_LEVEL LL_INFO
#endif

#include "log.h"
#include "assertions.h"

#include "general.h"
#include ".\fs\fs_api.h"
#include ".\fs\fs_dev.h"
#include ".\fs\fs_lbl.h"
#include ".\dcf\dcf.h"
#include "sysopt.h"

#define FS_BIT_WISE_OPERATION 1

#define FS_V_TOTAL_CACHE_BLOCK	0x7
#define FS_V_FAT_SECTOR_SIZE	0x200
#define FS_V_FAT_CLUSTER_NUMBER	0x40
#define FS_V_FAT_RSVD_SECTOR	0x20
#define FS_V_MEMBUF_SIZE	(FS_V_FAT_SECTOR_SIZE * FS_V_FAT_CLUSTER_NUMBER * 2) 	// Base on 128 cluster set

//FS error define
#define BPB_SETTING_ERROR	0xFFFF0110
#define FAT_SETTING_ERROR	0xFFFF0220
#define READ_SECTOR_ERROR	0xFFFF0330
#define BUFFER_ALLOC_ERROR	0xFFFF0440
#define GET_STATUS_ERROR	0xFFFF0550

#define FS_DEVICE_FIND_ERR		0xFFFFFFF0

#define FS_LB_STATUS_GET_ERR	0xFFFFFFEF
#define FS_LB_READ_DAT_ERR		0xFFFFFFEE
#define FS_LB_DIR_READ_DAT_ERR	0xFFFFFFED
#define FS_LB_MUL_READ_DAT_ERR	0xFFFFFFEC
#define FS_LB_WRITE_DAT_ERR		0xFFFFFFEB
#define FS_LB_DIR_WRITE_DAT_ERR	0xFFFFFFEA
#define FS_LB_MUL_WRITE_DAT_ERR	0xFFFFFFE9
#define FS_LB_READ_FAT_TBL_ERR	0xFFFFFFE8
#define FS_LB_CACHE_CLEAN_ERR	0xFFFFFFF7
#define FS_LB_CACHE_INIT_ERR	0xFFFFFFF6

#define FS_DEV_ACCESS_ERR		0xFFFFFFDF
#define FS_REAL_SEC_READ_ERR	0xFFFFFFDE
#define FS_REAL_SEC_WRTIE_ERR	0xFFFFFFDD
#define FS_DEV_IOCTL_ERR		0xFFFFFFDC

#define FS_FUNC_PRT_ASSIGN_ERR	0xFFFFFFCF

#define FS_PARAM_PTR_EXIST_ERR	0xFFFFFFCE
#define FS_PARAM_VALUE_ERR		0xFFFFFFCD
#define FS_MEMORY_ALLOC_ERR		0xFFFFFFCC

#define FS_FAT_CLUT_FIND_ERR	0xFFFFFFBF
#define FS_FAT_SEC_CAL_ERR		0xFFFFFFBE
#define FS_FAT_BUF_SET_ERR		0xFFFFFFBD
#define FS_FAT_CLUT_LINK_ERR	0xFFFFFFBC
#define FS_FAT_EOF_FIND_ERR		0xFFFFFFBB
#define FS_FAT_DAT_FORM_ERR		0xFFFFFFBA
#define FS_FAT_CLUT_SHIFT_ERR	0xFFFFFFB9
#define FS_FAT_CLUT_ALLOC_ERR	0xFFFFFFB8
#define FS_FAT_FSIS_UPDATE_ERR	0xFFFFFFB8
#define FS_FAT_LINK_DELETE_ERR	0xFFFFFFB7

#define FS_DIR_CREATE_ERR		0xFFFFFFAF
#define FS_DIR_OPEN_ERR			0xFFFFFFAE
#define FS_DIR_READ_ERR			0xFFFFFFAD
#define FS_DIR_FIND_ERR			0xFFFFFFAC
#define FS_DIR_DELETE_ERR		0xFFFFFFAB
#define FS_DIR_ENT_SIZE_ERR		0xFFFFFFAA
#define FS_DIR_PTR_USE_ERR 		0xFFFFFFA9
#define FS_DIR_OVER_OPEN_ERR	0xFFFFFFA8

#define FS_FILE_FIND_ERR		0xFFFFFF9F
#define FS_FILE_ENT_UPDATE_ERR	0xFFFFFF9E
#define FS_FILE_ENT_READ_ERR	0xFFFFFF9D
#define FS_FILE_ENT_WRITE_ERR	0xFFFFFF9C
#define FS_FILE_PTR_USE_ERR 	0xFFFFFF9B
#define FS_FILE_OVER_OPEN_ERR	0xFFFFFF9A
#define FS_FILE_DELETE_ERR		0xFFFFFF99
#define FS_FILE_NAME_REPEAT_ERR	0xFFFFFF98
#define FS_FILE_OPEN_ERR		0xFFFFFF97

#define FS_DATA_READ_ERR		0xFFFFFF8F
#define FS_DATA_WRITE_ERR		0xFFFFFF8E

#define FS_DEV_UNIT_CHECK_ERR	0xFFFFFF7F

////////////////////////////////////////////////
#define FS_FILE_PRT_ASSIAN_ERR	0xFFFFFFE3

#define FS_FILE_ENT_FIND_ERR	0xFFFFFFE3


#define FS_FAT_MKRMDIR_ERROR	0xFFFFFFE3


//  old 8200
#define FS_REAL_SEC_FIND_ERR	0xFFFFFFE3


#define FS_FUNC_PTR_EXIST_ERR	0xFFFFFFDF	// -33

#define FS_FILE_PRT_EXIST_ERR	0xFFFFFFDD	// -35
#define FS_DIR_FILE_FIND_ERR	0xFFFFFFDC	// -36
#define FS_DIR_FILE_ENT_RO_ERR	0xFFFFFFDB	// -37
#define FS_FILE_NOT_EXIST_ERR	0xFFFFFFDA	// -38
#define FS_FILE_CREATE_ERR		0xFFFFFFD9	// -39
#define FS_FAT_SEC_SIZE_ERR		0xFFFFFFD8	// -40
#define FS_FAT_CLUS_SIZE_ERR	0xFFFFFFD7	// -41
#define FS_DEVICE_ACCESS_ERR	0xFFFFFFD6	// -42
#define FS_FAT_READ_LEN_ERR		0xFFFFFFD5	// -43
#define FS_FAT_FIND_CLUS_ERR	0xFFFFFFD4	// -44
#define FS_FAT_FIND_EOF_CLUS_ERR	0xFFFFFFD3	// -45
#define FS_FAT_FIND_NEW_CLUS_ERR	0xFFFFFFD2	// -46
#define FS_FAT_FILE_UPDATE_ERR	0xFFFFFFD1	// -47
#define FS_FILE_BUF_EXIST_ERR	0xFFFFFFCF	// -48
#define FS_DEVICE_ERASE_ERR		0xFFFFFFCE	// -49
#define FS_DEVICE_READY_ERR		0xFFFFFFCD	// -50
#define FS_DEVICE_FORMAT_ERR	0xFFFFFFCC	// -51
#define FS_FAT_GET_SIZE_ERR		0xFFFFFFCB	// -52
#define FS_FAT_SEC_ACCESS_ERR	0xFFFFFFCA	// -53
#define FS_DIR_PRT_EXIST_ERR	0xFFFFFFBF	// -54
#define FS_DIR_CLOSE_ERR		0xFFFFFFBE	// -55
#define FS_FILE_NAME_LEN_ERR	0xFFFFFFBD	// -56

#define FS_FAT_MKDIR_OP_ERR		0xFFFFFFBB	// -58
	// -59

enum
{
	FS_E_CLUST_CLEAN_OFF = 0,
	FS_E_CLUST_CLEAN_ON,
};

enum
{
	FS_E_DELETE_TYPE_AUTO = 0,
	FS_E_DELETE_TYPE_ORDER,
	FS_E_DELETE_TYPE_DIR,
};

enum
{
	FS_E_BUF_TYPE_VALUE = 0, 
	FS_E_BUF_TYPE_LIST,
};

// extern Valuable
extern OS_EVENT *FSFATClustSemEvt;
extern FS_ClusterListCache FSReadCLCache;
extern int FSPlaybackCacheBufferReset(void);
extern u8 *FSBuf;

extern void fsTest(void);
extern unsigned int OSTimeGet(void);

extern int DCF_GetDeviceIndex(char *DevName);

extern int FS__find_fsl(const char *pFullName, FARCHARPTR *pFileName);


extern int FS__fat_findpath(int Idx, const char *pFullName, FARCHARPTR *pFileName, u32 *pUnit, u32 *pDirStart, u32 *pDirSize);
extern int FS_fat_rename(char *pOldFilePath,char *pNewFileName,int index);

extern int FS_ScanWholeDir(FS_DIR *pDir, FS_DIRENT *dst_DirEnt, u8* buffer, u32 DirEntMax, u32 *pOldestEntry, int DoBadFile);
extern int FS_SearchWholeDir(FS_DIR *pDir, FS_DIRENT *dst_DirEnt, u8* buffer, u32 DirEntMax, u32 *pOldestEntry,
                      char CHmap, u32 Typesel, u32 StartMin, u32 EndMin, int DoBadFile);
extern int FS_FetchItems(FS_DIR *pDir, FS_DIRENT *pDstEnt, FS_SearchCondition *pCondition);                      
extern int FS_ReadWholeDir(FS_DIR *pDir,FS_DIRENT *dst_DirEnt, u8* buffer, unsigned int DirEntMax,
                                    DEF_FILEREPAIR_INFO *pdcfBadFileInfo, u8 IsUpdateEntrySect, int DoBadFile);
extern int FS__fat_readwholedir(FS_DIR *pDir,FS_DIRENT *dst_DirEnt, u8* buffer, unsigned int DirEntMax,
        DEF_FILEREPAIR_INFO *pdcfBadFileInfo, u8 IsUpdateEntrySect, int DoBadFile);
extern int FS__fat_ScanWholedir(FS_DIR *pDir, FS_DIRENT *dst_DirEnt, u8* buffer, u32 DirEntMax, u32 *pOldestEntry, int DoBadFile);
extern int FS__fat_FetchItems(FS_DIR *pDir, FS_DIRENT *pDstEnt, FS_SearchCondition *pCondition);
extern int FS__fat_SearchWholedir(FS_DIR *pDir, FS_DIRENT *dst_DirEnt, u8* buffer, u32 DirEntMax, u32 *pOldestEntry,
                                  char CHmap, u32 Typesel, u32 StartMin, u32 EndMin, int DoBadFile);

extern u32 GetTotalBlockCount(int Idx, int Unit);

extern s32 sdcMount(void);

extern int FSMCacheBufInit(void);
extern u8 *FSMalloc(u32 SizeOfRequest);
extern int FSFree(u8 *MemoryAddress);
extern int FSFATLWScanClusterLink(int Idx, u32 Unit, u32 StartClust);
extern int FSFATGetClusterList(int Idx, u32 Unit, u32 StrtClst, u32 *pClustList);
extern int FSFATGoForwardCluster(int Idx, u32 Unit, u32 StrtClst, u32 NumOfCluster, u32 *pClust);
extern int FSFATGoForwardClusterList(int Idx, u32 Unit, u32 StrtClst, u32 NumOfCluster, u32 *pClustList);
extern int FSFATCalculateSectorByCluster(int Idx, u32 Unit, u32 *pSrcCluster, u32 *pDestSector);
extern int FSFATAllocateFreeCluster(int Idx, u32 Unit, u32 *LastUsedCluster, u32 NumberOfCluster, u32 CleanFlag, u32 LinkFlag);
extern int FSFATIncEntry(int Idx, u32 Unit, u32 DirStart, u32 NumberOfCluster, u32 *pSize, u32 CleanFlag);
extern int FSFATNewEntry(int Idx, u32 Unit, u32 *DirStart, u32 NumberOfCluster, u32 *pSize, u32 CleanFlag);
extern int FSFATFreeFATLink(int Idx, u32 Unit, u32 StartCluster);
extern int FSFATFreeFATLink_bg(s32 Idx, s32 Unit, s32 StartCluster, s32 dummy);
extern int FSFATFileEntryUpdate(FS_FILE *pFile);
extern int FSFATFileDelete(FS_DIR *pDir, char *pFileName, FS_DeleteCondition *pCondition);
extern int FSFATDirDelete(FS_DIR *pDir);

extern int FS__LB_Init(void);
extern int _FS_LB_GetDriverIndex(const FS__device_type *pDriver);
#if FS_USE_LB_READCACHE
extern int FS_LB_Cache_Init(int idx, u32 Unit);
extern int FS_LB_Cache_Enable(int idx, u32 Unit);
extern int FS_LB_Cache_Clean(int idx, u32 Unit);
extern int FS_LB_Cache_Clear(int idx, u32 Unit);
#endif


#endif
