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
#define DCF_FILE_USE_NONE           0x00
#define DCF_FILE_USE_CREATE         0x01
#define DCF_FILE_USE_CLOSE          0xAA

#define DCF_MAX_MULTI_FILE          4

/* Variable */

/* Type definition */

typedef struct _DCF_LIST_DIRENT {
	struct _DCF_LIST_DIRENT*	prev;
	struct _DCF_LIST_DIRENT*	next;
	struct FS_DIRENT*		    pDirEnt;
	unsigned int 				dirNum; 
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
    u32                         CH_Distr;
    u32                         FileTotal;
    u32                         ChTotal[DCF_MAX_MULTI_FILE];
	struct _DCF_LIST_DIRENT*	playbackPrev;
	struct _DCF_LIST_DIRENT*	playbackNext;
#endif    
#if((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR))
	unsigned char			    used_flag;
#endif

} DCF_LIST_DIRENT;

typedef struct _DCF_LIST_FILEENT {
	struct _DCF_LIST_FILEENT*	prev;
	struct _DCF_LIST_FILEENT*	next;
	struct FS_DIRENT*		    pDirEnt;
	unsigned int 				fileNum;
	unsigned char				fileType; 
 #if((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)||\
    (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR))
    unsigned char               used_flag;
 #endif
} DCF_LIST_FILEENT;

#if CDVR_iHome_LOG_SUPPORT
typedef struct _DCF_LIST_LOGENT {
	struct _DCF_LIST_LOGENT*	prev;
	struct _DCF_LIST_LOGENT*	next;
	struct FS_DIRENT*		    pDirEnt;
	unsigned int 				LogNum; 
	unsigned char			    used_flag;
} DCF_LIST_LOGENT;
#endif



typedef struct _filerepairinfo
{
   unsigned int BadFileNum;
   unsigned char BadFileName[4][16];
}DEF_FILEREPAIR_INFO;

typedef struct _dcfFileInfo
{
   u32  dcfFileInfoCount;
   u32  dcfFileInfoListCount;
   u32  dcfFileInfoNumLast;
   u32  dcfFileInfoFileType;   
}DCF_FILE_INFO;

typedef struct _DCF_PLAYBACK_DAY_INFO
{
	DCF_LIST_DIRENT  *dirHead;
	DCF_LIST_DIRENT  *dirTail;
    u32              DirNum;
}DCF_PLAYBACK_DAY_INFO;


#endif
