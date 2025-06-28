/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	SDK playback.h

Abstract:

   	The declarations of SDK playback.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2015/12/14  Lsk  Create  

*/

#ifndef __SDK_PLAYBACK_API__
#define __SDK_PLAYBACK_API__

#include "general.h"
#include "sysapi.h"
#include "asfapi.h"
#include "Adcapi.h"
#include "GlobalVariable.h"
#include "dcfapi.h"

typedef enum {
    state_stop=0,    
    state_play,   
    state_pause   
} PlaybackState;

typedef enum {
    play_direct=0,    
    play_thumbnail    
} PlaybackCmd;

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
extern BOOLEAN SDK_PLAYBACK_PlayAllFile(u32 time, u8 flag);
#endif
extern BOOLEAN SDK_PLAYBACK_PlayOneFile(void);
extern BOOLEAN SDK_PLAYBACK_StopAllFile(void);
extern BOOLEAN SDK_PLAYBACK_PlayPause(void);
extern BOOLEAN SDK_PLAYBACK_FF(void);
extern BOOLEAN SDK_PLAYBACK_RF(void);
extern BOOLEAN SDK_PLAYBACK_SlowMotion(void);
extern BOOLEAN SDK_PLAYBACK_PrevFile(void);
extern BOOLEAN SDK_PLAYBACK_NextFile(void);
extern BOOLEAN SDK_PLAYBACK_DeleteDir(void);
extern BOOLEAN SDK_PLAYBACK_DeleteFile(void);
extern BOOLEAN SDK_PLAYBACK_AudioVolume(s8 setting);
extern u32 SDK_PLAYBACK_GetFileDuration(DCF_LIST_FILEENT* _pTempCurFile);
extern u8 SDK_PLAYBACK_GetVideoPresentTIme(u32* time);
extern u32 SDK_PLAYBACK_GetState(void);
extern BOOLEAN SDK_CopyFile(u32 BeginSec, u32 EndSec);
extern u32 SDK_CopyFile_GetState(void);
#endif
