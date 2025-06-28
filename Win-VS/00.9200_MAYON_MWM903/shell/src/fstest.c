/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
description:	main

date:		20050429
author:		ganganwen Email:ganganwen@163.com 
*********************************************************************************************************
*/

#include	"general.h"
#include	"osapi.h"
#include	"commands.h"
#include	"shelltask.h"
#include	"fsapi.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/
#define  TASK_STK_SIZE                 	2048	/* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        10      /* Number of identical tasks                          */

#define TaskStart_Prio			1
#define Task1_Prio			2

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/
OS_STK  TaskStk[N_TASKS][TASK_STK_SIZE];    // Tasks stacks

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void TaskStart(void * pParam) ;
void Task1(void * pParam) ;                            /* Function prototypes of tasks                  */

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void fsTest(void)
{
	FS_Init();         /* Init the file system */
	
	OSTaskCreate(TaskStart, 0, &TaskStk[0][TASK_STK_SIZE-1], TaskStart_Prio);
	OSTaskCreate(shelltask, 0, &TaskStk[1][TASK_STK_SIZE-1], Task1_Prio);
		
	//FS_Exit();      /* End using the file system */
}

void TaskStart(void * pParam) 
{	
	char err;	
	OS_EVENT *sem1;

#if 0	
	timeSetEvent(1000/OS_TICKS_PER_SEC, 0, OSTickISR, 0, TIME_PERIODIC);
#endif
	
	sem1 = OSSemCreate(0);
	while(1)
	{		
		OSSemPend(sem1, 0, (INT8U *)&err);   // sleep, wait for sem1, run shell
	}
}
