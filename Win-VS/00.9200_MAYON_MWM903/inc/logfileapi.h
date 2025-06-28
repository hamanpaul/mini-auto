/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	logfileapi.h

Abstract:

   	The declarations of log file.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/02/23	Peter Hsu	Create

*/

#ifndef __LOGFILEAPI_H__
#define __LOGFILEAPI_H__
#if CDVR_LOG

#include    "task.h"
#include    "isuapi.h"

/*
#define LOGSTRING_TASK              LogStringTask
#define LOGSTRING_TASK_PARAMETER    (void *) 0
#define LOGSTRING_TASK_STACK_SIZE   1024
#define LOGSTRING_TASK_STACK        &LogStringTaskStack[LOGSTRING_TASK_STACK_SIZE-1]
*/

// task and event related
extern OS_STK       LogStringTaskStack[LOGSTRING_TASK_STACK_SIZE]; /* Stack of task LogStringTask() */
extern OS_EVENT*    LogStringTrgSemEvt;

extern u8           szVideoOverlay1[MAX_OVERLAYSTR];
extern u8           szVideoOverlay2[MAX_OVERLAYSTR];
extern u8           LogCounter;

/* function prototype */
void LogStringSuspendTask(void);
void LogStringTaskInit(void);
void LogStringTask(void* pData);
void LogStringTask(void* pData);
s32  GenerateLogString(void);
void AppendLogString(u8 *LogString);

#endif  // #if CDVR_LOG

#endif  // #ifndef __LOGFILEAPI_H__

