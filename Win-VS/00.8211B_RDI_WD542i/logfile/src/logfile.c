/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	logfile.c

Abstract:

   	The routines of log file function

Environment:

    	ARM RealView Developer Suite

Revision History:

	2011/02/23	Peter Hsu	Create

*/

#include "general.h"
#if CDVR_LOG
#include "board.h"
#include "uiapi.h"
#include "sysapi.h"
#include "siuapi.h"
#include "iduapi.h"
#include "spiapi.h"
#include "asfapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "asfapi.h"
#include "usbapi.h"
#include "logfileapi.h"

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */


 /*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */

// task and event related
OS_STK      LogStringTaskStack[LOGSTRING_TASK_STACK_SIZE]; /* Stack of task LogStringTask() */
OS_EVENT*   LogStringTrgSemEvt;

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */


/*

Routine Description:

    Suspend Generating log string task.

Arguments:

    None.

Return Value:

    None.

*/
void LogStringSuspendTask(void)
{
    /* Suspend the task */
    OSTaskSuspend(LOGSTRING_TASK_PRIORITY);
}

/*

Routine Description:

    Initialize generating log string task.

Arguments:

    None.

Return Value:

    None.

*/
void LogStringTaskInit(void)
{
    /* Create the semaphore */
    LogStringTrgSemEvt  = OSSemCreate(0);

    /* Create the task */
    OSTaskCreate(LOGSTRING_TASK, LOGSTRING_TASK_PARAMETER, LOGSTRING_TASK_STACK, LOGSTRING_TASK_PRIORITY);
    //LogStringSuspendTask();
}

/*

Routine Description:

    The generating log string task.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
void LogStringTask(void* pData)
{
    u8          err;

    while (1)
    {
        OSSemPend(LogStringTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
        GenerateLogString();
    }
}


#endif  // #if CDVR_LOG
