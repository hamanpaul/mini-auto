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
#include "fsapi.h"
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
#define DCF_DISTRIB_ALL_TYPE        0x00000f00


#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
enum
{
    DCF_CH_IDX_SIU     = 0,
    DCF_CH_IDX_CAMERA1,
    DCF_CH_IDX_CAMERA2,
    DCF_CH_IDX_CAMERA3,
    DCF_CH_IDX_CAMERA4,
    DCF_CH_IDX_DVR1 = DCF_CH_IDX_CAMERA3,
    DCF_CH_IDX_DVR2 = DCF_CH_IDX_CAMERA4,
    DCF_CH_IDX_PIP1,
    DCF_CH_IDX_PIP2,
    DCF_MAX_CHANNEL_IDX,
};

enum
{
    DCF_DOOR_DIR_MOVIE = 0,
    DCF_DOOR_DIR_PICTURE,
    DCF_DOOR_DIR_ALBUM,
};

#endif
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
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
#define DCF_DIR_SUFFIX           "VIDEO"
#define DCF_FILE_PREFIX		    "NK"
#define DCF_WRITE_BUF_SIZE	    512
#define DCF_READ_BUF_SIZE	    512
#define DCF_DIRENT_MAX		    16                      // lucian: optimizrd 32->16
#define DCF_FILEENT_MAX		    8192                    // "100VIDEO" directory 同步增加至 DCF_FILEENT_MAX entry.
#define DCF_FILE_PER_DIR	        DCF_FILEENT_MAX
#if (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB0)
#define DCF_FILENAME_MAX      99999999
#elif ((SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB1) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3))
#define DCF_FILENAME_MAX      9999999
#endif
#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
#define DCF_DIR_SUFFIX           "VIDEO"
#define DCF_FILE_PREFIX		    "NK"
#define DCF_WRITE_BUF_SIZE	    512
#define DCF_READ_BUF_SIZE	    512
#if RX_SNAPSHOT_SUPPORT
#define DCF_DIRENT_MAX		    64
#define DCF_FILEENT_MAX          3560
#else
#define DCF_DIRENT_MAX		    512                      // lucian: optimizrd 512->128
#define DCF_FILEENT_MAX          2560                     // "100VIDEO" directory 同步增加至 DCF_FILEENT_MAX entry. 60min*24/2min*4CH=2880, Section time 最低為2min.
#endif
#define DCF_FILE_PER_DIR	        DCF_FILEENT_MAX
#define DCF_FILENAME_MAX         99999999
#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
#define DCF_DIR_SUFFIX           "VIDEO"
#define DCF_FILE_PREFIX		    "NK"
#define DCF_WRITE_BUF_SIZE	    512
#define DCF_READ_BUF_SIZE	    512
#define DCF_DIRENT_MAX		    16                 // lucian: optimizrd 512->128
#define DCF_FILEENT_MAX          8192                // "100VIDEO" directory 同步增加至 DCF_FILEENT_MAX entry.
#define DCF_FILE_PER_DIR	        DCF_FILEENT_MAX
#if (HW_BOARD_OPTION == MR9670_WOAN)
#define DCF_PIC_FILE_PER_DIR	    500
#elif (HW_BOARD_OPTION == MR9670_COMMAX_71UM)
#define DCF_PIC_FILE_PER_DIR	    256
#else
#define DCF_PIC_FILE_PER_DIR	    128
#endif
#define DCF_FILENAME_MAX         999999
#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DSC)
#define DCF_DIR_SUFFIX          "IMG  "
#define DCF_FILE_PREFIX		    "MDIA"
#define DCF_WRITE_BUF_SIZE	    512
#define DCF_READ_BUF_SIZE	    512
#define DCF_DIRENT_MAX		    128         // civic optimizrd 512->128.
#define DCF_FILEENT_MAX		    1024
#define DCF_FILE_PER_DIR	        1000         /* < DCF_FILEENT_MAX */
#endif

#if CDVR_iHome_LOG_SUPPORT
#define DCF_LOGENT_MAX		    64                      // lucian: Log file max num
#define DCF_LOG_RESV_NUM         30

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
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
#define CUR_PIC_FILE_ENTRY   0x02
#define CUR_ALB_FILE_ENTRY   0x03
#endif
#define PLAYFILEENTRY        0x04
#define SCANFILEENTRY        0x05


#define FILEENTRY_ALL		0x00		//For backward compatible

#define FILEGROUP_A			0x00		//Manual mode
#define FILEGROUP_B			0x01		//Event mode
#define FILEGROUP_C			0x02		//Event mode
#define FILEGROUP_D			0x03		//Event mode

#if 1 //RX_SNAPSHOT_SUPPORT
#define DCF_PHOTO_DIST_MAX  240         //search 20 years
#endif


/*=======================External Variable =====================*/
extern OS_EVENT *dcfReadySemEvt;;
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
extern DCF_LIST_DIRENT* dcfListDirEntHead;
extern DCF_LIST_DIRENT* dcfListDirEntTail;
extern DCF_LIST_DIRENT* dcfPlaybackCurDir;
extern DCF_LIST_DIRENT* dcfPlaybackTmpDir;
extern DCF_LIST_FILEENT* dcfPlaybackCurFile;
extern DCF_LIST_FILEENT* dcfListFileEntHead;
extern DCF_LIST_FILEENT* dcfListReadFileEntHead;
extern DCF_LIST_FILEENT* dcfListReadFileEntTail;
extern DCF_LIST_FILEENT* dcfListFileEntTail;  //Lsk 090430 : for ROULE_DOORPHONE cycle playback
extern DCF_LIST_FILEENT* dcfPlaybackTempFile; //Lsk 090430 : for ROULE_DOORPHONE cycle playback
extern u32 dcfListFileCount;
extern u8 ucNANDInit;
extern u32 dcfFileNumLast; //表示該目錄下最新開檔的編號,下次開目錄以此為基準以流水號編織.

#if CDVR_iHome_LOG_SUPPORT
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

#if CDVR_REC_WITH_PLAY_SUPPORT
extern s32 dcfReadFileCount;
#endif

extern struct FS_DIRENT dcfFileEnt[];
extern u32 dcfDirRescanSwitch;
extern u32 dcfFileCount;

extern u32 global_totalfile_count;
extern u32 global_totalCamfile_count[DCF_MAX_MULTI_FILE];
extern u32 ChDistrInDir;   //Lucian: 用於統計目錄夾內Channel的分布, bit0 -->CH1, bit1-->CH2

extern u8  dcfWriteFromBuffer;

extern u32 global_totalfile_count;
extern u32 global_totaldir_count;
extern u8  gInsertCard;
extern u8  gInsertNAND;

extern FS_DISKFREE_T global_diskInfo;
extern s8 dcfCurDrive[]; 	/*CY 1023*/
extern s8 dcfCurDir[]; 	/*CY 1023*/
extern s8 dcfCurPath[]; 	/*CY 1023*/
extern s8 dcfTargetPath[]; 	/*CY 1023*/
extern s8 Temp_dcfCurDrive[64];
extern s8 Temp_dcfCurDir[64];
extern s8 Temp_dcfCurPath[64];
extern s8 Temp_dcfTargetPath[64];
extern s8 dcfDelDir[64];
extern s8 dcfDelPath[64];
extern FS_DISKFREE_T global_diskInfo;
#if (ROOTWORK==0)
extern s8 gsDirName[9];
#endif
extern u16  gJPGDecodedWidth, gJPGDecodedHeight;
extern u16  gJPGValidWidth, gJPGValidHeight;
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
extern u32 global_total_Pic_file_count;
extern u32 global_total_Alb_file_count;
extern DCF_LIST_FILEENT* dcfListPicFileEntHead;
extern DCF_LIST_FILEENT* dcfListPicFileEntTail;
extern DCF_LIST_FILEENT* dcfListAlbFileEntHead;
extern DCF_LIST_FILEENT* dcfListAlbFileEntTail;
extern u8 dcfPlaybackDirMode;
#endif
extern DCF_LIST_DIRENT* dcfListPlaybackDirHead;
extern DCF_LIST_DIRENT* dcfListPlaybackDirTail;
extern DCF_LIST_FILEENT* dcfListDelFileEntHead;
extern DCF_LIST_FILEENT* dcfListDelFileEntTail;
extern u8  dcfNewFile;
extern const char *DcfFileNameTag;
/*=======================External Function prototype ======================*/
extern void dcfGetCurFileName (s8 *FileName);
extern s32 dcfCheckDir(struct FS_DIRENT*, u32*);
extern s32 dcfCheckFile(struct FS_DIRENT*, u32*, u8*, u8);
extern s32 dcfCreateNextDir(void);
extern FS_FILE* dcfCreateNextFile(u8 fileType, u8 index);
extern s32 dcfCloseFileByIdx(FS_FILE* pFile, u8 idx, u8* pOpenFile);

extern s32 dcfDrive(s8*);
extern s32 dcfFormat(s8*);
extern s32 dcfDir(s8*, struct FS_DIRENT*, u32*,u8 IsUpdateEntrySect,int DoBadFile,unsigned int MaxEntry);
extern s32 dcfMkDir(s8*);
extern s32 dcfRmDir(s8*,u8 checkflag);
extern s32 dcfChDir(s8*);
extern FS_FILE* dcfOpen(s8*, s8*);
extern s32 dcfClose(FS_FILE*, u8*);
extern s32 dcfWrite(FS_FILE*, u8*, u32, u32*);
extern s32 dcfRead(FS_FILE*, u8*, u32, u32*);
extern int dcfSeek(FS_FILE *pFile, FS_i32 Offset, int Whence);
extern s32 dcfTell(FS_FILE *pFile);

extern s32 dcfOWEnable(void);


#if FS_NEW_VERSION
extern int dcfCheckDirExist(char *pDirPath);
extern int dcfOWDel(char *pDirName);
extern int dcfDel(char *pDirName, char *pFileName);
extern int dcfOWDelDir(char *pDirName);
extern int dcfPlayDel(char *pDirName, char *pFileName);

#else
extern s32 dcfDel(s8*,u16);
extern s32 dcfOWDel(s8* pFileName,u16 EstFileEntrySect);
extern s32 dcfPlayDel(s8* pFileName,u16 EstFileEntrySect);
#endif

extern int dcfVideoDel(FS_FILE *pFile);

extern s32 dcfDriveInfo(FS_DISKFREE_T*);
extern s32 dcfFindLastEofCluster(u32 StorageType);

extern s32 dcfGetDirEnt(s8*, struct FS_DIRENT*, s8*);
extern s32 dcfIncDirEnt(s8* pDirName, s32 IncEntNum);
extern s32 dcfCacheClean(void);
extern s32 dcfCacheClear(void);

extern s32 dcfFlushTempBuf(FS_FILE* pFile);

extern s32 dcfPlaybackFileNext(void);
extern s32 dcfPlaybackFilePrev(void);
extern s32 dcfPlaybackDirForward(void);
extern s32 dcfPlaybackDirBackward(void);
extern s32 dcfScanFileOnPlaybackDir(void);

extern s32 dcfPlaybackDel(void);
extern s32 dcfPlaybackDelAll(void);
extern s32 dcfPlaybackFormat(void);
extern s32 dcfPlaybackDelDir(void);

extern s32 dcfInit(u32); /*CY 1023*/
extern s32 dcfUninit(void);
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
extern s32 dcfPlaybackCalendarInit(u8 year, u8 month,u32 CamList, u32 bRescan);
extern u8 dcfCheckFileChannel(char CHmap,char CHch);


#if ((SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB1) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB2) || (SUB_FILE_SYSTEM_DVR_SEL == FILE_SYSTEM_DVR_SUB3))
extern void dcfSetFileGroup(u8 ucmode);
extern u8 dcfGetFileGroup(void);
extern u8 dcfRestoreFileGroup(void);
extern u8 dcfSaveFileGroup(void);
#endif

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
extern s32 dcfScanDiskAll(u8 year, u8 month,u32 CamList);
#elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
extern s32 dcfDoorChangeDir(u8  dirIdx);
#endif

#if (CDVR_LOG || CDVR_TEST_LOG)
extern FS_FILE* dcfCreatLogFile(void);
#endif
extern DCF_PLAYBACK_DAY_INFO dcfPlaybackDayInfo[31];
#endif

#if CDVR_iHome_LOG_SUPPORT
extern s32 dcfCheckLogFile(struct FS_DIRENT* pDirEnt, u32* pDirNum);
extern s32 dcfLogDel(s8* pFileName,u16 EstFileEntrySect);
extern int dcfWriteLogFile(u8 *event,int size);
extern FS_FILE* dcfCreateNextLog();
extern FS_FILE* dcfOpenLogFile(int Nday);
extern int dcfGetLogFileInfo(int Nday);
extern void dcfLogTest(u8 *cmd);
extern int dcfReadLogFile(int Nday,int Nblock,int *pRblock,int *pTblock);
extern int dcfLogFileUsrDel(int Nday);
extern int dcfLogFileSaveRefresh();

#endif

#if RX_SNAPSHOT_SUPPORT
extern int dcfWriteNextPhotoFile(int RFUnit,u8 *buffer,int size);
extern s32 dcfPhotoFileInit(int CHsel);
extern int dcfPlaybackPhotoMapInit(int CHsel,int Year,int Month);
extern int dcfPlaybackPhotoFileInit(int CHsel,int Year,int Month);
extern int dcfPlaybackPhotoDelAll(void);

#endif



