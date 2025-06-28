/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	dcfapi.h

Abstract:

   	The application interface of DCF.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __DCF_API_H__
#define __DCF_API_H__

#include "sdcapi.h"
#include ".\dcf\dcf.h"
#include ".\dcf\exifdef.h"
#include ".\dcf\exififd.h"
#include ".\dcf\exif.h"

/* FDB attribute */
#define DCF_FAT_ATTR_READ_ONLY		0x01
#define DCF_FAT_ATTR_HIDDEN		    0x02
#define DCF_FAT_ATTR_SYSTEM		    0x04
#define DCF_FAT_VOLUME_ID		    0x08
#define DCF_FAT_ATTR_ARCHIVE		0x20
#define DCF_FAT_ATTR_DIRECTORY		0x10

/* File type */
#define DCF_FILE_TYPE_ASF		    0x00
#define DCF_FILE_TYPE_JPG		    0x01
#define DCF_FILE_TYPE_WAV		    0x02
#define DCF_FILE_TYPE_RAW		    0x03
#define DCF_FILE_TYPE_AVI		    0x04
#define DCF_FILE_TYPE_MOV		    0x05
#define DCF_FILE_TYPE_AXF		    0x06
//#define DCF_FILE_TYPE_THM		    0x07
//#define DCF_FILE_TYPE_MP4		    0x08
#define DCF_FILE_TYPE_MAX		    0x07
//--------------//

#define DCF_FLAG_CREATE_FILE        0x00000001   //Lucian: 避免create/ delete 同時做. 2013/10/30
#define DCF_FLAG_PLAYBACK_CUR       0x00000002

//------File Distribution------//
#define DCF_DISTRIB_CH1             0x00000001
#define DCF_DISTRIB_CH2             0x00000002
#define DCF_DISTRIB_CH3             0x00000004
#define DCF_DISTRIB_CH4             0x00000008

#define DCF_DISTRIB_MANU            0x00000100
#define DCF_DISTRIB_SCHE            0x00000200
#define DCF_DISTRIB_MOTN            0x00000400
#define DCF_DISTRIB_RING            0x00000800
#define DCF_DISTRIB_ALL_TYPE        0x0000ff00

enum
{
	DCF_E_FILE_NAME_DASH = 0x1,
	DCF_E_FILE_NAME_MANUAL = 0x2,
	DCF_E_FILE_NAME_SCHE = 0x4,
	DCF_E_FILE_NAME_DYNAMIC = 0x8,
	DCF_E_FILE_NAME_APM = 0x10,
	DCF_E_FILE_NAME_FORCE_TRIGGER = 0x20,
	DCF_E_FILE_NAME_RING = 0x40,
	DCF_E_FILE_NAME_ALL = 0xff,
};

enum
{
    DCF_OVERWRITE_OP_OFF = 0,
    DCF_OVERWRITE_OP_01_DAYS = 1,
    DCF_OVERWRITE_OP_07_DAYS = 7,
    DCF_OVERWRITE_OP_30_DAYS = 30,
    DCF_OVERWRITE_OP_60_DAYS = 60,
    DCF_OVERWRITE_OP_AUTO
};

enum
{
	DCF_I_FILE_NAME_DASH = 0,
	DCF_I_FILE_NAME_MANUAL,
	DCF_I_FILE_NAME_SCHE,
	DCF_I_FILE_NAME_DYNAMIC,
	DCF_I_FILE_NAME_APM,
	DCF_I_FILE_NAME_FORCE_TRIGGER,
	DCF_I_FILE_NAME_RING,
};

typedef struct _app3prefix
{
    unsigned short APP3Marker;
    unsigned short APP3Size;
    unsigned int ID;
} DEF_APP3PREFIX;

typedef struct _app4prefix
{
    unsigned short APP4Marker;
    unsigned short APP4Size;
} DEF_APP4PREFIX;


/* Constant */
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
#define DCF_DIR_SUFFIX		"VIDEO"
#define DCF_FILE_PREFIX		"NK"
#define DCF_WRITE_BUF_SIZE	512
#define DCF_READ_BUF_SIZE	512
#define DCF_DIRENT_MAX		512                      // lucian: optimizrd 512->128
#define DCF_FILEENT_MAX		2560                    // 512 * 64 / 32 * N - 2// "100VIDEO" directory 同步增加至 DCF_FILEENT_MAX entry. 60min*24/2min*4CH=2880, Section time 最低為2min.
#define DCF_FILE_PER_DIR	DCF_FILEENT_MAX
#define DCF_FILENAME_MAX	99999999
#endif

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
#define DCF_LOGENT_MAX		    16                      // lucian: Log file max num
#define DCF_LOG_RESV_NUM         7

typedef struct _DEF_LOG_DATETIME
{
    u16	year;	/* year since 2000 - up to 2063 */
    u8	month;	/* 1 - 12 */
    u8 	day;	/* 1 - 31 */
    u8	hour;	/* 0 - 23 */
    u8	min;	/* 0 - 59 */
    u8	sec;	/* 0 - 59 */
} DEF_LOG_DATETIME;

typedef struct _DEF_LOG_INFO
{
    u8 FileName[32];
    DEF_LOG_DATETIME   DateTime;
    u32 FileSize;
} DEF_LOG_INFO;
#endif

/* DCF related definition */
#define CURRENTFILEENTRY     0x00
#define DELFILEENTRY         0x01
#define PLAYFILEENTRY        0x04
#define SCANFILEENTRY        0x05

#define FILEENTRY_ALL		0x00		//For backward compatible

#define FILEGROUP_A			0x00		//Manual mode
#define FILEGROUP_B			0x01		//Event mode
#define FILEGROUP_C			0x02		//Event mode
#define FILEGROUP_D			0x03		//Event mode


#define DCF_REC_TYPE_MANUAL      0
#define DCF_REC_TYPE_MOTION      1
#define DCF_REC_TYPE_SCHEDULE    2
#define DCF_REC_TYPE_NONE        3


/*=======================External Variable =====================*/
extern OS_EVENT *dcfReadySemEvt;
extern OS_EVENT *dcfWriteSemEvt;

extern u8 dcfChannelRecType[];
extern s32 dcfOverWriteOP;
extern u32 dcfStorageType; /*CY 1023*/
extern u32 exifThumbnailImageHeaderSize;
#if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
extern u32	EXIF_DRI_SIZE;
extern u32	EXIF_DQT_SIZE;
extern u32	EXIF_SOF0_SIZE;
extern u32	EXIF_SOS_SIZE;
extern u32 exifMJPGImageHeaderSize;
extern __align(4) EXIF_MJPG exifMJPGImage;
#endif
extern __align(4) EXIF_THUMBNAIL_IMAGE exifThumbnailImage;
extern u32 exifPrimaryImageHeaderSize;
extern __align(4) EXIF_PRIMARY_IMAGE exifPrimaryImage;
extern __align(4) EXIF_PRIMARY_IMAGE exifAPP3VGAImage;
extern DEF_APP3PREFIX exifAPP3Prefix;
extern DEF_APP4PREFIX exifAPP4Prefix;
extern DCF_LIST_DIRENT* dcfPlaybackCurDir;
extern DCF_LIST_FILEENT* dcfPlaybackCurFile;
extern u8 ucNANDInit;

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
extern u32 dcfLogFileNumLast; //表示該目錄下最新開檔的編號,下次開目錄以此為基準以流水號編織.
extern DCF_LIST_LOGENT* dcfListLogEntHead; //指向file link的第一個.
extern DCF_LIST_LOGENT* dcfListLogEntTail; //指向file link的最後一個.
extern DCF_LIST_LOGENT* dcfListLogEntRead;
extern DEF_LOG_INFO dcfLogFileInfo;
extern int dcfLogInitReady;
#endif

#if RX_SNAPSHOT_SUPPORT
extern int dcfPhotoInitReady;
#endif

extern u8  dcfWriteFromBuffer;

extern u32 dcfDirRescanSwitch;

extern u8  gInsertNAND;

extern s8 dcfCurDrive[]; 	/*CY 1023*/
extern s8 dcfTargetPath[]; 	/*CY 1023*/

extern s8 dcfBackupDrive[]; 	/*CY 1023*/
extern s8 dcfBackupTargetPath[]; 	/*CY 1023*/

extern s8 dcfCurDir[]; 	/*CY 1023*/
extern s8 dcfCurPath[]; 	/*CY 1023*/

extern s8 dcfBackupDir[]; 	/*CY 1023*/
extern s8 dcfBackupPath[]; 	/*CY 1023*/


extern s8 dcfDelDir[64];
extern s8 dcfDelPath[64];

extern FS_DISKFREE_T global_diskInfo;
extern FS_DISKFREE_T Backup_diskInfo;

extern char gsDirName[9];
extern u16  gJPGDecodedWidth, gJPGDecodedHeight;
extern u16  gJPGValidWidth, gJPGValidHeight;

extern u8  dcfNewFile;
/*=======================External Function prototype ======================*/

// dcf
extern int dcfRWTest(void);
extern int dcfSetChannelRecType(u8 Idx, u8 RecType);
extern int dcfInit(u32 type);
extern int dcfUninit(void);
extern int dcfDirInit(void);
extern int dcfBackupDirInit(void);
extern int dcfFileInit(DCF_LIST_DIRENT* pListDirEnt, u8 Type);
extern int dcfCreateNextDir(void);
extern FS_FILE* dcfCreateNextFile(u8 fileType, u8 index);



#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
extern DCF_LIST_DIRENT *dcfGetVideoDirListHead(void);
extern DCF_LIST_DIRENT *dcfGetVideoDirListTail(void);
extern DCF_LIST_FILEENT *dcfGetPlaybackFileListHead(void);
extern DCF_LIST_FILEENT *dcfGetPlaybackFileListTail(void);

extern int dcfGetCurDirFileCount(void);
extern int dcfGetTotalDirCount(void);
extern int dcfScanDiskAll(u8 year, u8 month, u32 CamList, u32 TypeList);

extern int dcfSearchFileOnPlaybackDir(char CHmap, u32 Typesel, u32 StartMin, u32 EndMin);
extern int dcfPlaybackDirDetailCounts(DCF_LIST_DIRENT *CurDir);
extern int dcfPlaybackDirDetailSearch(u32 *pDirCount, u32 NumOfMaxEnt, u32 IdxOfFirstItem, u8 CamList, u16 TypeSel, 
                                u32 StartSec, u32 EndSec, u8 DelBadFile, u8 LightingSearch);
extern int dcfCloseFileByIdx(FS_FILE* pFile, u8 idx, u8* pOpenFile);
#endif


// dcffs
extern int dcfOWDel(char *pDirName);
extern int dcfDel(char *pDirName, char *pFileName);
extern int dcfOWDelDir(char *pDirName);
extern int dcfPlayDel(char *pDirName, char *pFileName);

extern int dcfClose(FS_FILE* pFile, u8* pOpenFile);

extern int dcfCheckDirExist(char *pDirPath);
extern int dcfItemExist(char *pFileName, char *Mode);
extern int dcfFsInit(void);
extern s32 dcfBackupFsInit(u32 BackupType);
extern int dcfChBackupDir(char *pDirName);
extern int dcfDirScan(char *pDirName, FS_DIRENT *pDirEnt, u32* pDirCount, u32 *pOldestEntry, int DoBadFile);
extern u32 dcfGetMainStorageFreeSize(void);
extern int dcfGetPlaybackNameMaskIndex(void);
extern int dcfFilterNameFormat(const char *ItemName, u32 SearchMode, u16 NameTypeMask);
extern int dcfFetchDirItems(char *pDirName, FS_DIRENT *pDirEnt, FS_SearchCondition *pCondition);




extern FS_FILE* dcfCreateNextBackupFile(u8 fileType, u8 CHnum,u8 RecType,RTC_DATE_TIME  *pFileTime);

extern void dcfGetCurFileName (s8 *FileName);
extern s32 dcfCheckDir(FS_DIRENT*, u32*);
extern s32 dcfCheckFile(FS_DIRENT*, u32*, u8*, u8);
extern int dcfCheckBackupUnit(void);
extern int dcfCheckUnit(void);

extern s32 dcfDrive(s8*);
extern s32 dcfFormat(s8*);
extern s32 dcfFormatBackup(s8* pDriveName);

extern int dcfDir(char *pDirName, FS_DIRENT *pDirEnt, u32 *pDirCount, u8 IsUpdateEntrySect, int DoBadFile, u32 MaxEntry);
extern int dcfMkDir(char *pDirName);
extern int dcfMkBackupDir(char *pDirName);

extern s32 dcfCreateOWDelCurPath(void);
extern s32 dcfCreatePlayCurPath(void);
extern int dcfCreateFullPath(char *pFullName, char *pDirName);

extern int dcfRmDir(char *pDirName, u8 checkflag);
extern s32 dcfChDir(char *pDirName);
extern int dcfChPlayDir(char *pDirName);
extern s32 dcfChOWDelDir(s8* pDirName);

extern FS_FILE* dcfOpen(char *pFileName, s8* pMode);
FS_FILE* dcfBackupOpen(char *pFileName, s8* pMode);

extern s32 dcfOWEnable(void);


extern s32 dcfBackupClose(FS_FILE* pFile);

extern int dcfPlayDirScan(char *pDirName, FS_DIRENT *pDirEnt, u32* pDirCount, u32 *pOldestEntry, int DoBadFile);
extern int dcfPlayDirSearch(char *pDirName, FS_DIRENT *pDirEnt, u32* pDirCount, u32 *pOldestEntry,
                     char CHmap, u32 Typesel, u32 StartMin, u32 EndMin, int DoBadFile);


extern s32 dcfWrite(FS_FILE*, u8*, u32, u32*);
s32 dcfBackupWrite(FS_FILE* pFile, u8* pData, u32 dataSize, u32* pWriteSize);

extern s32 dcfRead(FS_FILE*, u8*, u32, u32*);
s32 dcfBackupRead(FS_FILE* pFile, u8* pData, u32 dataSize, u32* pReadSize);

extern int dcfSeek(FS_FILE *pFile, s32 Offset, int Whence);
extern s32 dcfTell(FS_FILE *pFile);

extern s32 dcfOverWriteDel_bgT(s32 OWDays, s32 dummy1, s32 dummy2, s32 dummy3);
extern s32 dcfScanDiskAll_bdT(s32 dummy);


extern int dcfOWDelRmDir(char *pDirName,u8 checkflag);
extern int dcfOWDelDirScan(char *pDirName, FS_DIRENT *pDirEnt, s32* pDirCount, u32 *pOldestEntry, int DoBadFile);



extern int dcfDriveInfo(FS_DISKFREE_T* pInfo, int Initilized);
extern int dcfBackupDriveInfo(FS_DISKFREE_T* pInfo, int Initilized);

extern s32 dcfFindLastEofCluster(u32 StorageType);

extern int dcfGetDirEnt(char *pDirName, FS_DIRENT *pDirEnt, char *pSubDirName);
extern int dcfIncDirEnt(char *pDirName, s32 IncEntNum);
extern int dcfCacheInit(void);
extern int dcfCacheEnable(void);
extern int dcfCacheClean(void);
extern int dcfCacheClear(void);

extern int dcfBackupCacheClean(void);
extern int dcfBackupCacheClear(void);

extern s32 dcfFlushTempBuf(FS_FILE* pFile);

extern s32 dcfPlaybackFileNext(void);
extern s32 dcfPlaybackFilePrev(void);
extern s32 dcfPlaybackDirForward(void);
extern s32 dcfPlaybackDirBackward(void);
extern s32 dcfScanFileOnPlaybackDir(void);

extern s32 dcfPlaybackDel(void);
extern s32 dcfPlaybackDelAll(void);
extern int dcfPlaybackFormat(void);
extern s32 dcfBackupFormat(void);

extern s32 dcfPlaybackDelDir(void);



extern s32 dcfBackupInit(u32 type);


extern s32 dcfBackupUninit(int BackupDeviceType);


extern s8 ReturnDcf(void);
extern s32 exifFileInit(void);
extern s32 exifFileParse(u8, u8*, u32, u32*, u16*, u16*);

extern s32 exifSetPrimaryQuantizationTable(u8*, u8*);
extern s32 exifSetQuantizationTable(u8, const u8*, const u8*);
extern s32 exifSetImageResolution(u16, u16);
extern s32 exifSetThumbnailSize(u32);
extern s32 exifSetCompressedBitsPerPixel(u32);
extern s32 exifSetCopyRightVersion(s8 *copyright);
extern s32 exifSetDateTime(RTC_DATE_TIME*);

extern s32 exifSetFlash(u16);
extern s32 exifSetLightSource(u16);

extern s32 exifSetExposureBiasValue(s32);
extern s32 exifSetApertureValue(RATIONAL*, RATIONAL*);
extern s32 exifSetShutterSpeedValue(RATIONAL*, RATIONAL*);
extern s32 exifSetBrightnessValue(RATIONAL*);

extern s32 exifSetSubjectDistance(u32);
extern s32 exifSetFocalLength(u32);

extern s32 exifWriteFile(u32, u32, u8);
extern s32 exifReadFile(void);
extern s32 RawWriteFile(u32,u8 *);
extern s32 exifReadThumbnailFile(void);
extern u8 dcfGetFileType(char* pcsubfilename);
extern void dcfFileTypeCount_Clean(void);
extern void dcfFileTypeCount_Inc(u8 ucFileType);
extern void dcfFileTypeCount_Dec(u8 ucFileType);
extern s32 dcfPlaybackCalendarInit(u8 year, u8 month, u32 CamList,u32 TypeList,u8 Rescan);
extern u8 dcfCheckFileChannel(char , char *);
extern s32 dcfOverWriteDel(void);


#if ((SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB1) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3))
extern void dcfSetFileGroup(u8 ucmode);
extern u8 dcfGetFileGroup(void);
extern u8 dcfRestoreFileGroup(void);
extern u8 dcfSaveFileGroup(void);
#endif

#if (CDVR_LOG || CDVR_TEST_LOG)
extern FS_FILE* dcfCreatLogFile(void);
#endif
extern DCF_PLAYBACK_DAY_INFO dcfPlaybackDayInfo[31];


#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
extern s32 dcfCheckLogFile(FS_DIRENT* pDirEnt, u32* pDirNum);
extern int dcfLogDel(char *pFileName);
extern int dcfWriteLogFile(u8 *event,int size);
extern int dcfLogFileSaveRefresh(void);

extern FS_FILE* dcfCreateNextLog(void);
extern FS_FILE* dcfOpenLogFile(int Nday);
extern int dcfGetLogFileInfo(int Nday);
extern void dcfLogTest(u8 *cmd);
extern int dcfReadLogFile(int Nday,int Nblock,int *pRblock,int *pTblock);
extern int dcfLogFileUsrDel(int Nday);

#endif

#if RX_SNAPSHOT_SUPPORT
int dcfWriteNextPhotoFile(int RFUnit,u8 *buffer,int size);
s32 dcfPhotoFileInit(int CHsel);
#endif
#endif

