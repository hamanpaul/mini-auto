/***************************************************************************

* Copyright Information : This software is the property of D-Link and shall
						  not be reproduced distributed and copied without
						  the permission from  D-Link

* Project Name			: Timer Library

* Module Name			: Generic Timer

* File Name				: DlkTimer.h
		
* Author Name			: Yogesh Kumar Sharma
						  D-Link (India) Ltd.
						  Bangalore, India

* Description			: This file contains the macros & function declerat-
						  ions for the Timer.c

* Related Documents		: Generic Timer libraries for the Stacks2.doc

****************************************************************************/

#ifndef _SSL_DLKTIMER_H
#define _SSL_DLKTIMER_H

/*==================== File(s) Included ===================================*/
#include <stdio.h>
#include <string.h>


/*==================== Macro Definition(s) ================================*/
#define	SSL_DLK_TIMER_LINUX					0
#define	SSL_DLK_TIMER_WINDOWS				0
#define	SSL_DLK_TIMER_NUCLEUS				1

#define	SSL_DLK_TIMER_HASH_ENABLED			1			/* 1: Enabled; 0: Disabled */

#define	SSL_DLK_TIMER_FAILURE				-1
#define	SSL_DLK_TIMER_SUCCESS				0

#define	SSL_DLK_TIMER_ONE_SHOT				0
#define	SSL_DLK_TIMER_RECURSIVE				1

#define	SSL_DLK_TIMER_DEBUG_ALLOWED			0
#define	SSL_DLK_TIMER_DEBUG					printf		/* For Debugging */
#define	SSL_DLK_TIMER_DEBUG_FILE_OUT		0			/* For logging in the file  */
#define	SSL_DLK_TIMER_DEBUG_COUNTS			0			/* For timer counts */
#define	SSL_DLK_TIMER_DEBUG_SCREEN_OUT		0			/* For logging on the 
													   console */
#define	SSL_DLK_TIMER_DEBUG_SCR_OUT_SHRT	0			/* For displaying short debug 
													   messages on the console */

#if	SSL_DLK_TIMER_LINUX
	#define	SSL_DLK_INT32_MAX				0xFFFFFFFF	/* 2^32 - 1 */
#else
	#define	SSL_DLK_INT32_MAX				0x7FFFFFFF	/* 2^31 - 1 */
#endif

/*Macro to define the granularity of time measured*/
#define	SSL_DLK_TIMER_MILLI					1			/* Time measured in mill-
													   isecs */

/*==================== Data Structure(s) ==================================*/
/* data structure to store time returned by DlkTimerGetTimeOfDay() */
typedef struct
{
	UINT32 sec;
	UINT32 micro;
} SSL_dlk_time;

struct	SslTimerQ
{
	UINT32		u32TimerId;							/* The timer identifier */
	SSL_dlk_time	sTimeOfCreation;					/* Time stamp taken at the time of 
													   creation of the timer */
	void		(*pCallBack)(UINT8 *, UINT32);		/* Callback function */
	UINT8		*pu8SessionID;						/* First argument to the callback function,
													   session identifier */
	UINT32		u32SessionIdLen;					/* Second argument to the callback function,
													   session identifier length */
};


/*==================== Function Decleration(s) ============================*/

/* Function for initializing the Dlk timer */
void	SSL_DlkTimerInit(void);

/* Function for processing the list of Active timers */
void	SSL_DlkProcessTick(void);

/* Function for creating a timer */
INT32	SSL_DlkCreateTimer(UINT32	*u32TimerId,
				       UINT32	u32TimeoutValue,
				       //void		(*pCallBack)(void *pPrm),
				       void		(*pCallBack)(struct SslTimerQ *pPrm),
				       void		*pPrm,
				       UINT8	u8Type);

/* Function for deleting a timer */
INT32	SSL_DlkDeleteTimer(UINT32	u32TimerId);

/* Function for Activating the timer */
INT32	SSL_DlkStartTimer(UINT32	u32TimerId);

/* Function for Deactivating the timer */
INT32	SSL_DlkStopTimer(UINT32	u32TimerId);

/* Function for restarting the timer */
INT32	SSL_DlkResetTimer(UINT32	u32TimerId,
				      UINT32	u32ResetTimeoutValue);

/* Function for returning the Delta value of a timer */
UINT32	SSL_DlkTimerDeltaRemain(UINT32	u32TimerId);

/* Function for printing the timer statistics */
void	SSL_DlkPrintTimerCounts(void);

/* Function creates and starts the timer */
INT32	SSL_DlkCreateNStartTimer(UINT32	*u32TimerId,
				       	     UINT32	u32TimeoutValue,
		       			     //void	(*pCallBack)(void *pPrm),
		       			     void	(*pCallBack)(struct SslTimerQ *pPrm),
				             void	*pPrm,
				             UINT8	u8Type);

/* Function creates the timer and holds it in the restart timer list */
INT32	SSL_DlkCreateNWaitTimer(UINT32	*u32TimerId,
		      			    UINT32	u32TimeoutValue,
				       	    //void	(*pCallBack)(void *pPrm),
				       	    void	(*pCallBack)(struct SslTimerQ *pPrm),
				            void	*pPrm,
						    UINT8	u8Type);

SSL_dlk_time	SSL_DlkTimerGetTimeOfDay(void);


/*=========================================================================*/


#endif /* _SSL_DLKTIMER_H */


