/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	dcf.h

Abstract:

   	The declarations of DCF.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __DCF_H__
#define __DCF_H__
#include "general.h"

/* Constant */
#define DCF_FILE_USE_NONE	0x00
#define DCF_FILE_USE_CREATE	0x01
#define DCF_FILE_USE_CLOSE	0xAA

#define DCF_MAX_MULTI_FILE	4

/* Variable */

/* Type definition */

typedef struct _DCF_LIST_DIRENT
{
    struct _DCF_LIST_DIRENT *prev;
    struct _DCF_LIST_DIRENT *next;
    FS_DIRENT *pDirEnt;
    u32 dirNum;
    // below; first entry number
    // | bit 15: exist or not
    // | bit 0-14: number to locate the first file entry position.
    u16 PosSchedule;
    u16 PosDetect;
    u16 PosManual;
    u16 PosRing;
    // file counter
    u16 SumSchedule[DCF_MAX_MULTI_FILE];
    u16 SumDetect[DCF_MAX_MULTI_FILE];
    u16 SumManual[DCF_MAX_MULTI_FILE];
    u16 SumRing[DCF_MAX_MULTI_FILE];
    // record timeline for file list
    // bit 0: 00.~01. | bit 1: 01.~02.........| bit 23: 23.~00.
    u32 DistributionSchedule[DCF_MAX_MULTI_FILE];
    u32 DistributionDetect[DCF_MAX_MULTI_FILE];
    u32 DistributionManual[DCF_MAX_MULTI_FILE];
    u32 DistributionRing[DCF_MAX_MULTI_FILE];
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
    u32 CH_Distr;	// L: 用於統計目錄夾內Channel的分布, bit0 -->CH1, bit1-->CH2
    u32 FileTotal;
    u32 ChTotal[DCF_MAX_MULTI_FILE];
    struct _DCF_LIST_DIRENT *playbackPrev;
    struct _DCF_LIST_DIRENT *playbackNext;
#endif
    u8 used_flag;

} DCF_LIST_DIRENT;

typedef struct _DCF_LIST_FILEENT
{
    struct _DCF_LIST_FILEENT *prev;
    struct _DCF_LIST_FILEENT *next;
    struct _DCF_LIST_FILEENT *CamPrev;
    struct _DCF_LIST_FILEENT *CamNext;
    FS_DIRENT *pDirEnt;
    u32 fileNum;
    u8 fileType;
    u8 used_flag;
} DCF_LIST_FILEENT;

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
typedef struct _DCF_LIST_LOGENT
{
    struct _DCF_LIST_LOGENT *prev;
    struct _DCF_LIST_LOGENT *next;
    FS_DIRENT *pDirEnt;
    unsigned int LogNum;
    u8 used_flag;
} DCF_LIST_LOGENT;

typedef struct _DCF_LIST_SYSLOG
{
    struct _DCF_LIST_SYSLOG *prev;
    struct _DCF_LIST_SYSLOG *next;
    u32 beginSec;
    u32 endSec;
    u8 eventType;
    u8 channel;
} DCF_LIST_SYSLOG;


#endif


typedef struct _filerepairinfo
{
    unsigned int BadFileNum;
    u8 BadFileName[4][16];
} DEF_FILEREPAIR_INFO;

typedef struct _dcfFileInfo
{
    u32 dcfFileInfoCount;
    u32 dcfFileInfoListCount;
    u32 dcfFileInfoNumLast;
    u32 dcfFileInfoFileType;
} DCF_FILE_INFO;

typedef struct _DCF_PLAYBACK_DAY_INFO
{
    DCF_LIST_DIRENT *dirHead;
    DCF_LIST_DIRENT *dirTail;
    u32 DirNum;
} DCF_PLAYBACK_DAY_INFO;


#endif
