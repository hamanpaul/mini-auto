/****************************************************************************

* Copyright Information : This software is the property of D-Link and shall 
						  not be reproduced distributed and copied without 
						  the permission from  D-Link

* Project Name			: Timer Library

* Module Name			: Generic Timer

* File Name				: DlkTimer.c

* Author Name			: Yogesh Kumar Sharma
						  D-Link (India) Ltd.
						  Bangalore, India

* Extern Function(s)	: None

* Description			: This file contains the implementation of Generic 
						  Timers

* Related Document(s)	: Generic Timer libraries for the Stacks2.doc

* A brief summary:
   ===============
	There will be two doubly linked lists for the timer libraries. One 
	will store all the inactive (not running) timers and other will store 
	all the active (running) timers.

	The first doubly linked list containing all the inactive timers will 
	be sorted in the order of the timer Id.

	The second doubly linked list containing all the active timers will 
	be sorted in order of time of expiration.

	When we will create a timer it will add into the first doubly linked 
	list with sorted order of the Timer Id, when we start a timer then only 
	it will add into the active timer list.

  	We have a function DlkpProcessTick that will be called. Whenever 
	DlkpProcessTick function is called, it will decrease the Delta value of 
	the first node (TimerId =0) by the time difference btween last updated 
	and current time of updation. Then check whether Delta value is 0 nor 
	not. If 0, timer is expired, call the callback function, then check next
	timer for whether Delta value is 0 or not and so on.

****************************************************************************/

/*****************/
/* Macro(s) used */
/*****************/
#define	SSL_DLK_TIMER_MAX_POOL_SIZE			20000
#define	SSL_DLK_TIMER_MIN_POOL_SIZE			40
#define	SSL_DLK_TIMER_MAX_HASH_NODES		1024	/* Change this to whatever 
												   number of nodes one desire 
												   in the HASH table */
#define	SSL_DLK_TIMER_GET_HASH_VAL(x)		((x) % SSL_DLK_TIMER_MAX_HASH_NODES)

/*******************/
/* Include File(s) */
/*******************/
#include	"OS_Adpt.h"
#include	"DlkTimer.h"

#include <inc/ctypes.h>
#include <l3/port/inc/la3_os.h>

#if	SSL_DLK_TIMER_LINUX
	/* File(s) Included */
	#include 	<sys/time.h>
	#include 	<pthread.h>
	#include 	<errno.h>
#elif	SSL_DLK_TIMER_NUCLEUS
	/* Macro(s) */
	#define	SSL_DLK_TIMER_STACK_SIZE		4*1024//5000
	#define	SSL_DLK_TIMER_TASK_PRIORITY		60//15

	/* File(s) Included */
#endif

/******************/
/* Extern(s) used */
/******************/


/*********************/
/* Data Structure(s) */
/*********************/
/* Timer Data Structure */
struct SSL_DlkTimer
{
	UINT32	u32TimerId;					/* Unique identifier of the Timer */
	//void	(*pCallBack)(void *pPrm);	/* Function to be invoked on timer 
	void	(*pCallBack)(struct SslTimerQ *pPrm);	/* Function to be invoked on timer 
										   expiry */
	void	*pPrm;						/* Argument to the callback function */
	INT32	i32Delta;					/* Time remaining for the timer to exp-
										   ire, or stores the timer difference 
										   between adjacent timer nodes */
	UINT32	u32TimeOutVal;				/* Expiration time of the timer, rema-
										   ins unchanged */
	struct	SSL_DlkTimer	*pPrevTimer;	/* Linked to the Previous Timer link */
	struct	SSL_DlkTimer	*pNextTimer;	/* Linked to the Next Timer link */
	UINT8	u8Type;						/* Type of timer: One shot, Recursive */
};


#if	SSL_DLK_TIMER_HASH_ENABLED
/* DlkTimers' Hash List */
struct	SSL_DlkTimerHashNodes
{
	UINT32	u32TimerId;									/* Timer Id, the hash index */
	struct	SSL_DlkTimer			*psAddrActiveNode;		/* Pointer to Active List Node */
	struct	SSL_DlkTimer			*psAddrInactiveNode;	/* Pointer to Inactive List Node */
	struct	SSL_DlkTimerHashNodes	*psNext;				/* Pointer to next hash node */
	struct	SSL_DlkTimerHashNodes	*psPrev;				/* Pointer to previous hash node */
};
#endif	/* DLK_TIMER_HASH_ENABLED */


/* Data structure containing the information of timers to be restarted during 
   DlkTimerProcess */
struct	SSL_DlkTimerRestart
{
	UINT32		u32TimerId;					/* Holds timer Id */
	UINT32		u32RestartTimeOutVal;		/* The time out value to be restar-
											   ted with */
	struct		SSL_DlkTimerRestart	*pNext;		/* Pointer to next link */
};


/* Static variables */
struct	SSL_DlkTimerStaticVariables
{
	UINT32					u32TimerIdCtr;
	struct	SSL_DlkTimer		*pHeadInactiveTimer;
	struct	SSL_DlkTimer		*pHeadActiveTimer;
	struct	SSL_DlkTimerRestart	*pHeadRestartTimer;
	SSL_dlk_time				sTODFrstNodeCr;		/* Time of the Day when first
												   node in the list of Active
												   timers was created */
	UINT32					u32TimersCreated;
	UINT32					u32TimersDeleted;
};


/***************************/
/* Fucntion Decleration(s) */
/***************************/
static	struct		SSL_DlkTimer	*SSL_DlkTimerLocateNode(struct SSL_DlkTimer *, UINT32);
static	UINT32		SSL_DlkTimerInsertActive(struct SSL_DlkTimer *);
static	void		SSL_DlkRestartTimersInList(void);
static	INT32		SSL_DlkTimersToRestart(UINT32, UINT32);
static	void		SSL_DlkPrintActiveList(void);
static	UINT32		SSL_DlkElapsedTime(SSL_dlk_time sCurTick);
static	void		SSL_DlkTimerLockSema(void);
static	void		SSL_DlkTimerUnlockSema(void);
//static	INT32		DlkSelfCreateTimer(UINT32 *, UINT32, void (*)(void *), void *, UINT8);
static	INT32		SSL_DlkSelfCreateTimer(UINT32 *, UINT32, void (*)(struct SslTimerQ *), void *, UINT8);
static	INT32		SSL_DlkSelfStartTimer(UINT32);
static	INT32		SSL_DlkSelfStopTimer(UINT32);
static	INT32		SSL_DlkSelfDeleteTimer(UINT32);
static	INT32		SSL_DlkSelfResetTimer(UINT32, UINT32);
//static	void		DlkTimerStartTask(void);
static void SSL_DlkTimerStartTask(UI32_T argc,void *argv);

#if	SSL_DLK_TIMER_HASH_ENABLED
static	struct		SSL_DlkTimerHashNodes	*SSL_DlkTimerGetHashNode(UINT32);
static	INT8		SSL_DlkTimerAddHashNode(UINT32, struct SSL_DlkTimer *, struct SSL_DlkTimer *);
static	INT8		SSL_DlkTimerDeleteHashNode(UINT32);
static	INT8		SSL_DlkTimerUpdateHashNode(UINT32, struct SSL_DlkTimer *);
static	struct		SSL_DlkTimerHashNodes	*SSL_DlkTimerCreateHashNode(void);
#endif	/* DLK_TIMER_HASH_ENABLED */


/***************************/
/* Static Variable(s) used */
/***************************/
/* For debugging */
#if	SSL_DLK_TIMER_DEBUG_ALLOWED
	#if SSL_DLK_TIMER_DEBUG_FILE_OUT
		static FILE	*SSL_Dlk_Timer_FP;
	#endif	/* DLK_TIMER_DEBUG_FILE_OUT */
#endif	/* DLK_TIMER_DEBUG_ALLOWED */

#if	SSL_DLK_TIMER_NUCLEUS
	NU_TASK	SSL_DlkTimerTask;
	void	*SSL_pvDlkTimerSema;
	UINT8	SSL_au8DlkTimerTaskStack[SSL_DLK_TIMER_STACK_SIZE];
#endif

void	*SSL_pvDlkTimerMemPool;
static	struct	SSL_DlkTimerStaticVariables	SSL_DlkTimerStVar;
#if	SSL_DLK_TIMER_HASH_ENABLED
static	struct	SSL_DlkTimerHashNodes		*SSL_DlkTimerHashList[SSL_DLK_TIMER_MAX_HASH_NODES];	/* Array of Pointers */
#endif	/* DLK_TIMER_HASH_ENABLED */

#if SSL_DLK_TIMER_LINUX
	pthread_t Thrd;
	pthread_mutex_t semTimerMutex = PTHREAD_MUTEX_INITIALIZER;
#endif 


/****************************************************************************
Function Name : DlkTimerInit
Parameters	  : None
Return Value  : None
Description	  : This function initialized the variables required throughout 
				the span of Timer library.
****************************************************************************/
void SSL_DlkTimerInit(void)
{
	/* Variable Decleration and Initialization */
	#if	SSL_DLK_TIMER_NUCLEUS
		INT32	i32Status;
	#endif
	static UINT32	su32TimerInitCnt = 0;	/* This variable helps in making 
											   this function execute only once*/

	/* If this fucntion has been invoked once, then return */
	if(su32TimerInitCnt > 0)
	{
		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Timers already Initialized\n");
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		return;
	}

	#if SSL_DLK_TIMER_NUCLEUS
		memset(SSL_au8DlkTimerTaskStack, 0, SSL_DLK_TIMER_STACK_SIZE);
	#endif /* DLK_TIMER_NUCLEUS */

#if	SSL_DLK_TIMER_HASH_ENABLED
	/* Memsetting the Hash List */
	memset(SSL_DlkTimerHashList, 0, SSL_DLK_TIMER_MAX_HASH_NODES * sizeof(UINT32));
#endif	/* DLK_TIMER_HASH_ENABLED */

	SSL_DlkTimerStVar.u32TimerIdCtr		   = 1;	/* Initializing TimerIdCtr */
	SSL_DlkTimerStVar.u32TimersCreated	   = 0;
	SSL_DlkTimerStVar.u32TimersDeleted	   = 0;
	SSL_DlkTimerStVar.sTODFrstNodeCr.sec   = 0;
	SSL_DlkTimerStVar.sTODFrstNodeCr.micro = 0;
	SSL_DlkTimerStVar.pHeadInactiveTimer   = NULL;
	SSL_DlkTimerStVar.pHeadActiveTimer	   = NULL;
	SSL_DlkTimerStVar.pHeadRestartTimer	   = NULL;

	/* Testing by printing output to the file */
	#if SSL_DLK_TIMER_DEBUG_ALLOWED
		#if SSL_DLK_TIMER_DEBUG_FILE_OUT
			SSL_Dlk_Timer_FP = fopen("Timer_Output.txt", "w+");
		#endif	/* DLK_TIMER_DEBUG_FILE_OUT */
	#endif	/* DLK_TIMER_DEBUG_ALLOWED */

#if	SSL_DLK_TIMER_NUCLEUS
	/* Creating a Semaphore */
#if	0	//For DES-3226
	pvDlkTimerSema = OS_ADPT_Create_Semaphore(BINARY_SEMAPHORE);
#else	//For DES-3326
	SSL_pvDlkTimerSema = OS_ADPT_Create_Semaphore(BINARY_SEMAPHORE, 1, "web_ssl");
#endif

	if(SSL_pvDlkTimerSema == NULL)
	{
		return;
	}

	/* Creating the Memory Pool */
	SSL_pvDlkTimerMemPool = OS_ADPT_Create_Memory_Pool("SSL_1", SSL_DLK_TIMER_MAX_POOL_SIZE, SSL_DLK_TIMER_MIN_POOL_SIZE);

	if(SSL_pvDlkTimerMemPool == NULL)
	{
		return;
	}
#if 0
	/* Creating a task for the Timer to run on */
	i32Status = NU_Create_Task(&DlkTimerTask,			\
							   "D-LinkTimer",			\
							   DlkTimerStartTask,		\
							   0,						\
							   NULL,					\
							   &au8DlkTimerTaskStack[0],\
							   DLK_TIMER_STACK_SIZE,	\
							   DLK_TIMER_TASK_PRIORITY, \
							   0,						\
							   NU_PREEMPT,				\
							   NU_START);
#else
	i32Status = LA3_OS_Create_Task(
&SSL_DlkTimerTask,			\
								 "D-LinkTimer",			\
							   SSL_DlkTimerStartTask,		\
							   0,						\
							   NULL,					\
							   &SSL_au8DlkTimerTaskStack[0],\
							   SSL_DLK_TIMER_STACK_SIZE,	\
							   SSL_DLK_TIMER_TASK_PRIORITY, \
							   0,						\
							   LA3_OS_NO_PREEMPT,				\
							   LA3_OS_START);
      						  
        
#endif        
	
	if(i32Status != 0)
	{
		OS_ADPT_Delete_Memory_Pool(SSL_pvDlkTimerMemPool);
		DEBUG_WEB("\n SSL_TIMER_TIC TASK CREATE FAILED");
		SSL_pvDlkTimerMemPool = NULL;
		return;
	}
#endif	//DLK_TIMER_NUCLEUS

#if SSL_DLK_TIMER_LINUX
	SSL_pvDlkTimerMemPool = NULL;

	printf("\n Creating timer thread...");
	if(pthread_create(&Thrd, NULL, SSL_DlkTimerStartTask, (void *)0) != 0)
	{
		printf("\n Thread creation failed...");
		exit(1);
	}
#endif /* DLK_TIMER_LINUX */

	/* Incrementing the initialized counter */
	su32TimerInitCnt++;
}


/****************************************************************************
Function Name : DlkTimerStartTask
Parameters	  : None
Return Value  : None
Description	  : This function is called in the DlkTimerTask for processing
				the timers
****************************************************************************/
//static	void	DlkTimerStartTask(void)
static void SSL_DlkTimerStartTask(UI32_T argc,void *argv)
{
	
	if(argc || argv)
	{}
	
	while(1)
	{
		SSL_DlkProcessTick();

		#if	SSL_DLK_TIMER_NUCLEUS
			/* Releasing the control for some time */
			NU_Sleep(10);	//25 Earlier, Yogesh, 21102003
		#elif SSL_DLK_TIMER_LINUX
			usleep(100000);	//250000 Earlier, Yogesh, 21102003
		#endif	/* DLK_TIMER_NUCLEUS */
	}
}


/****************************************************************************
Function Name : DlkCreateNWaitTimer
Parameters	  : UINT32	*pu32TimerId             - [OUT] Will contain the uniq-
												   ue Timer ID generated.
   				UINT32	u32TimeoutValue	         - [IN] The timeout value after 
										           which the timer will expire.
				void	(*pCallBack)(void *pPrm) - [IN] Callback routine to be 
												   invoked at the time of timer
												   expiry.
				void	*pPrm					 - [IN] Argument for callback 
												   routine.
				UINT8	u8Type					 - [IN] Input argument 
												   specifying the type of timer,
												   whether One-shot or cyclic/
												   recursive.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description	  : This function creates a link in the Inactive linked list
				and adds the node in the list of timers to be added in the
				Active list of timers after DlkProcessTick function does the
				job with the Active timers in hand. 
****************************************************************************/
INT32	SSL_DlkCreateNWaitTimer(UINT32	*pu32TimerId,
			      			UINT32	u32TimeoutValue,
					    	//void	(*pCallBack)(void *pPrm),
					    	void	(*pCallBack)(struct SslTimerQ *pPrm),
						    void	*pPrm,
							UINT8	u8Type)
{
	/* Semaphore Locking */
	SSL_DlkTimerLockSema();

	if(SSL_DlkSelfCreateTimer(pu32TimerId, u32TimeoutValue, pCallBack, pPrm, u8Type) == SSL_DLK_TIMER_FAILURE)
	{
		/* Semaphore Unlocking */
		SSL_DlkTimerUnlockSema();

		return SSL_DLK_TIMER_FAILURE;
	}

	if(SSL_DlkTimersToRestart(*pu32TimerId,u32TimeoutValue) == SSL_DLK_TIMER_FAILURE)
	{
		SSL_DlkSelfDeleteTimer(*pu32TimerId);
		*pu32TimerId = 0;

		/* Semaphore Unlocking */
		SSL_DlkTimerUnlockSema();

		return SSL_DLK_TIMER_FAILURE;
	}

	/* Semaphore Unlocking */
	SSL_DlkTimerUnlockSema();

	return SSL_DLK_TIMER_SUCCESS;
}


/****************************************************************************
Function Name : DlkCreateNStartTimer
Parameters	  : UINT32	*pu32TimerId             - [OUT] Will contain the unique
										           Timer ID generated.
   				UINT32	u32TimeoutValue	         - [IN] The timeout value after 
										           which the timer will expire.
				void	(*pCallBack)(void *pPrm) - [IN] Callback routine to be 
												   invoked at the time of timer
												   expiry.
				void	*pPrm					 - [IN] Argument for callback 
												   routine.
				UINT8	u8Type					 - [IN] Input argument 
				                                   specifying the type of timer,
												   whether One-shot or cyclic/
												   recursive.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description	  : This function creates a link in both the Inactive linked list
				and Active linked list of timers.
****************************************************************************/
INT32	SSL_DlkCreateNStartTimer(UINT32	*pu32TimerId,
					       	 UINT32	u32TimeoutValue,
			       			 //void	(*pCallBack)(void *pPrm),
			       			 void	(*pCallBack)(struct SslTimerQ *pPrm),
					         void	*pPrm,
					         UINT8	u8Type)
{
	/* Semaphore Locking */
	SSL_DlkTimerLockSema();

	if(SSL_DlkSelfCreateTimer(pu32TimerId, u32TimeoutValue, pCallBack, pPrm, u8Type) == SSL_DLK_TIMER_FAILURE)	
	//if(DlkSelfCreateTimer(pu32TimerId, u32TimeoutValue, pCallBack, pPrm, u8Type) == DLK_TIMER_FAILURE)
	{
		/* Semaphore Unlocking */
		SSL_DlkTimerUnlockSema();

		return SSL_DLK_TIMER_FAILURE;
	}
		
	if(SSL_DlkSelfStartTimer(*pu32TimerId) == SSL_DLK_TIMER_FAILURE)
	{
		SSL_DlkSelfDeleteTimer(*pu32TimerId);
		*pu32TimerId = 0;

		/* Semaphore Unlocking */
		SSL_DlkTimerUnlockSema();

		return SSL_DLK_TIMER_FAILURE;
	}

	/* Semaphore Unlocking */
	SSL_DlkTimerUnlockSema();

	return SSL_DLK_TIMER_SUCCESS;
}


/****************************************************************************
Function Name : DlkCreateTimer
Parameters	  : UINT32	*pu32TimerId             - [OUT] Will contain the unique
										           Timer ID generated.
	  	        UINT32	u32TimeoutValue	         - [IN] The timeout value after 
										           which the timer will expire.
				void	(*pCallBack)(void *pPrm) - [IN] Callback routine to be 
												   invoked at the time of timer
												   expiry.
				void	*pPrm					 - [IN] Argument for callback 
												   routine.
				UINT8	u8Type					 - [IN] Input argument 
												   specifying the type of timer,
												   whether One-shot or cyclic/
												   recursive.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description	  : This function creates a link in the Inactive linked list of 
				timers.
				
				==============================================================
				|| NOTE: This function expects the timeout in MILLIESECONDS ||
				==============================================================
*****************************************************************************/
INT32	SSL_DlkCreateTimer(UINT32	*pu32TimerId,
					   UINT32	u32TimeoutValue,
					   //void		(*pCallBack)(void *pPrm),
					   void		(*pCallBack)(struct SslTimerQ *pPrm),
					   void		*pPrm,
					   UINT8	u8Type)
{
	/* Variable(s) Decleration */
	INT32	i32RetStatus;

	/* Semaphore Locking */
	SSL_DlkTimerLockSema();

	i32RetStatus = SSL_DlkSelfCreateTimer(pu32TimerId, u32TimeoutValue, 
									  pCallBack, pPrm, u8Type);

	/* Semaphore Unlocking */
	SSL_DlkTimerUnlockSema();

	return i32RetStatus;
}


/****************************************************************************
Function Name : DlkDeleteTimer
Parameters	  : UINT32	u32TimerId - [IN] The timer link identifier, to be
									 removed.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description	  : This function deletes a link from the Inactive as well as 
				Active linked list of timers, depending on the unique 
				Timer identifier.
****************************************************************************/
INT32	SSL_DlkDeleteTimer(UINT32	u32TimerId)
{
	/* Variable(s) Decleration */
	INT32	i32RetVal;

	/* Semaphore Locking */
	SSL_DlkTimerLockSema();

	i32RetVal = SSL_DlkSelfDeleteTimer(u32TimerId);
		
	/* Semaphore Unlocking */
	SSL_DlkTimerUnlockSema();

	return i32RetVal;
}


/****************************************************************************
Function Name : DlkStartTimer
Parameters	  : UINT32	u32TimerId - [IN] The timer link identifier.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description	  : This function starts a timer by putting the timer link from
				Inactive timer linked list into linked list of Active timer.
****************************************************************************/
INT32	SSL_DlkStartTimer(UINT32	u32TimerId)
{
	/* Variable(s) Decleration */
	INT32	i32RetVal;

	/* Semaphore Locking */
	SSL_DlkTimerLockSema();

	i32RetVal = SSL_DlkSelfStartTimer(u32TimerId);

	/* Semaphore Unlocking */
	SSL_DlkTimerUnlockSema();

	return i32RetVal;
}


/****************************************************************************
Function Name : DlkStopTimer
Parameters	  : UINT32	u32TimerId - [IN] The timer link identifier.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description   : This function stops the timer by removing the timer link from
				linked list of Active timers.
****************************************************************************/
INT32	SSL_DlkStopTimer(UINT32		u32TimerId)
{
	/* Variable(s) Decleration */
	INT32	i32RetVal;

	/* Semaphore Locking */
	SSL_DlkTimerLockSema();

	i32RetVal = SSL_DlkSelfStopTimer(u32TimerId);

	/* Semaphore Unlocking */
	SSL_DlkTimerUnlockSema();

	return SSL_DLK_TIMER_SUCCESS;
}


/****************************************************************************
Function Name : DlkResetTimer
Parameters	  : UINT32	u32TimerId			 - [IN] The timer link identifier.
				UINT32	u32ResetTimeoutValue - [IN] The time out of the timer
											   to be reset again with.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description	  : This function restarts the timer with the reset timeout value
				passed as input argument.
****************************************************************************/
INT32	SSL_DlkResetTimer(UINT32	u32TimerId,
					  UINT32	u32ResetTimeoutValue)
{
	/* Variable(s) Decleration */
	INT32	i32RetVal;

	/* Semaphore Locking */
	SSL_DlkTimerLockSema();

	i32RetVal = SSL_DlkSelfResetTimer(u32TimerId, u32ResetTimeoutValue);

	/* Semaphore Unlocking */
	SSL_DlkTimerUnlockSema();

	return i32RetVal;
}


/****************************************************************************
Function Name : DlkProcessTick
Parameters	  : None
Return Value  : None
Description	  : This function process the timer links.
****************************************************************************/
void	SSL_DlkProcessTick(void)
{
	/* Variable(s) Decleration */
	struct	SSL_DlkTimer	*pTemp;
	struct	SSL_DlkTimer	*pNode;
	UINT8	u8TimerType;
	UINT32	u32TimeOutVal;
	UINT32	u32TId;
	UINT32	u32DiffTicks;
	INT32	i32DiffCaryOvr;
	SSL_dlk_time sTimeTick;		/* Holds the current time of the day in msec */
#if	SSL_DLK_TIMER_HASH_ENABLED
	struct	SSL_DlkTimerHashNodes	*pTempNodeForChk_Hash;
#else
	struct	SSL_DlkTimer	*pTempNodeForChk;
#endif	/* DLK_TIMER_HASH_ENABLED */

	/* Semaphore Locking */
	SSL_DlkTimerLockSema();

	/* Updating the first node of the link list of Active timers */
	if(SSL_DlkTimerStVar.pHeadActiveTimer != NULL)
	{
		/* Getting current time of Day */
		sTimeTick = SSL_DlkTimerGetTimeOfDay();

		/* Get time elapsed (in milli sec) since last updated*/
		u32DiffTicks = SSL_DlkElapsedTime(sTimeTick);
			
		if(u32DiffTicks != 0)
		{
			/* Storing the time tick, the link list was last updated */
			SSL_DlkTimerStVar.sTODFrstNodeCr = sTimeTick;

			/* Checking if head node has still to go before expiring */
			if((UINT32)SSL_DlkTimerStVar.pHeadActiveTimer->i32Delta > u32DiffTicks)
			{
				SSL_DlkTimerStVar.pHeadActiveTimer->i32Delta -= u32DiffTicks;
			}
			else
			{
				i32DiffCaryOvr = u32DiffTicks;
				pTemp = SSL_DlkTimerStVar.pHeadActiveTimer; 

				while((pTemp != NULL) && (pTemp->i32Delta <= i32DiffCaryOvr))
				{
					i32DiffCaryOvr	-= pTemp->i32Delta;
					pTemp->i32Delta	= 0;
					u8TimerType	 	= pTemp->u8Type;
					u32TId		 	= pTemp->u32TimerId;
					u32TimeOutVal	= pTemp->u32TimeOutVal;
					pNode		 	= pTemp->pNextTimer;

					/* Semaphore Unlocking */
					SSL_DlkTimerUnlockSema();

					/* Calling the callback function */
					(*(pTemp->pCallBack))(pTemp->pPrm);

					/* Semaphore Locking */
					SSL_DlkTimerLockSema();

					/* Verifying if the node still persists */
#if SSL_DLK_TIMER_HASH_ENABLED
					pTempNodeForChk_Hash = SSL_DlkTimerGetHashNode(u32TId);

					if((pTempNodeForChk_Hash != NULL)
					   && (pTempNodeForChk_Hash->psAddrActiveNode != NULL)
					   && (pTempNodeForChk_Hash->psAddrActiveNode->u32TimeOutVal == u32TimeOutVal))
#else
					pTempNodeForChk = SSL_DlkTimerLocateNode(SSL_DlkTimerStVar.pHeadActiveTimer, u32TId);

					if((pTempNodeForChk != NULL)
					   && (pTempNodeForChk->u32TimeOutVal == u32TimeOutVal))
#endif	/* DLK_TIMER_HASH_ENABLED */
					{
						/* If it is a one shot timer then stop it */
						/* u8TimerType can also replace pTemp->u8Type, in case
						   we are not letting someone change the Type of the 
						   timer in the callback function */
						if(pTemp->u8Type /*u8TimerType*/ == SSL_DLK_TIMER_ONE_SHOT)
						{
							/* If timer type is One Shot, then stop it */
							SSL_DlkSelfDeleteTimer(u32TId);
						}
						else if(pTemp->u8Type /*u8TimerType*/ == SSL_DLK_TIMER_RECURSIVE)
						{
							/* If timer type is Recursive, then restart it */
							SSL_DlkTimersToRestart(u32TId, u32TimeOutVal);
							SSL_DlkSelfStopTimer(u32TId);
						}
						else
						{
							/* If timer type is none of the above, then stop it.
							   Will be useful, in case someone has deleted the
							   timer thus freeing the datastructure and filling 
							   it with junk data */
							SSL_DlkSelfDeleteTimer(u32TId);
						}
					}

					pTemp = pNode;
				}

				/* Modifying the Delta value of the Head of Active timers' 
				   list */
				if(pTemp != NULL)
				{
					pTemp->i32Delta -= i32DiffCaryOvr;
				}

				/* Restarting all the timers present in the linked list of 
				   timers to be restarted */
				SSL_DlkRestartTimersInList();

				#if SSL_DLK_TIMER_DEBUG_ALLOWED
					SSL_DlkPrintTimerCounts();
					SSL_DlkPrintActiveList();
				#endif /* DLK_TIMER_DEBUG_ALLOWED */
			}
		}
	}

	/* Semaphore Unlocking */
	SSL_DlkTimerUnlockSema();
}


/****************************************************************************
Function Name : DlkTimerLocateNode
Parameters	  : struct DlkTimer * - [IN] The head pointer to the list in which
									the timer node is to be located.
				UINT32			  - [IN] The timer ID to locate in the list
Return Value  : struct DlkTimer * - Returns NULL on Failure else returns 
									pointer to the node located
Description	  : This function locates the node with timer ID given as second
				input argument to the function, in the linked list pointed by
				pHeadPtr, first argument to the function.
****************************************************************************/
static	struct	SSL_DlkTimer	*SSL_DlkTimerLocateNode(struct	SSL_DlkTimer	*pHeadPtr, 
												UINT32	u32FindTimerId)
{
	/* Variable(s) Decleration */
	struct SSL_DlkTimer	*pTemp;

	/* Locating the node */
	pTemp = pHeadPtr;

	while((pTemp != NULL) && (pTemp->u32TimerId != u32FindTimerId))
	{
		pTemp = pTemp->pNextTimer;
	}

	return pTemp;
}


/****************************************************************************
Function Name : DlkTimerInsertActive
Parameters	  : struct DlkTimer * - [IN] The head pointer to the list in which
									the timer node is to be located.
Return Value  : UINT32 - Returns DLK_TIMER_SUCCESS on success, DLK_TIMER_FAI-
						 LURE otherwise.
Description	  : This function inserts a new node in the linked list of Active 
				timers.
****************************************************************************/
static	UINT32	SSL_DlkTimerInsertActive(struct SSL_DlkTimer *pNewNode)
{
	/* Variable(s) Decleration */
	struct	SSL_DlkTimer	*pTemp;
	struct	SSL_DlkTimer	*pPrev;
	UINT32	u32CumDelta;		/* Accumulated Delta value */

	/* If it is first node to be inserted */
	if(SSL_DlkTimerStVar.pHeadActiveTimer == NULL)
	{
		SSL_DlkTimerStVar.pHeadActiveTimer = pNewNode;

		/* Modifying the Delta value(s) */
		pNewNode->i32Delta = pNewNode->u32TimeOutVal;

		/* Filling the Time of the day value in the file static variable */
		SSL_DlkTimerStVar.sTODFrstNodeCr = SSL_DlkTimerGetTimeOfDay();
	}
	/* Else locate the apposite place */
	else
	{
		pTemp = SSL_DlkTimerStVar.pHeadActiveTimer;

		/* Initializing the value of Cumulative Delta */
		u32CumDelta = pTemp->i32Delta;

		while((pTemp != NULL) && (u32CumDelta <= pNewNode->u32TimeOutVal))
		{
			pPrev = pTemp;
			pTemp = pTemp->pNextTimer;

			if(pTemp != NULL)
			{
				u32CumDelta += pTemp->i32Delta;
			}
		}
		if(pTemp == NULL)
		{
			/* Inserting at the end */
			pPrev->pNextTimer = pNewNode;
			pNewNode->pPrevTimer = pPrev;

			/* Filling the Delta value of the New Node */
			pNewNode->i32Delta = pNewNode->u32TimeOutVal - u32CumDelta;
		}
		else
		{
			/* When inserting in the chain, somewhere in between */
			if(pTemp != SSL_DlkTimerStVar.pHeadActiveTimer)
			{
				pTemp->pPrevTimer->pNextTimer = pNewNode;
				pNewNode->pPrevTimer = pTemp->pPrevTimer;
				pTemp->pPrevTimer = pNewNode;
				pNewNode->pNextTimer = pTemp;
				pNewNode->i32Delta = pNewNode->u32TimeOutVal 
										- (u32CumDelta - pTemp->i32Delta);
				pNewNode->pNextTimer->i32Delta -= pNewNode->i32Delta;
			}
			/* When inserting at the start of the chain */
			else
			{
				pNewNode->pNextTimer = SSL_DlkTimerStVar.pHeadActiveTimer;
				SSL_DlkTimerStVar.pHeadActiveTimer->pPrevTimer = pNewNode;
				SSL_DlkTimerStVar.pHeadActiveTimer  = pNewNode;

				pNewNode->i32Delta = pNewNode->u32TimeOutVal;
				pNewNode->pNextTimer->i32Delta -= pNewNode->i32Delta;
			}
		}
	}

	return SSL_DLK_TIMER_SUCCESS;
}


/****************************************************************************
Function Name : DlkTimersToRestart
Parameters	  : UINT32 u32TimerId    - [IN] The timer Id of the timer to be 
									    restarted
				UINT32 u32RestartVal - [IN] The time out value
Return Value  : INT32 - DLK_TIMER_SUCCESS on success DLK_TIMER_FAILURE othe-
						rwise
Description	  : This function adds a new node in the linked list of the timers
				to be restarted.
****************************************************************************/
static	INT32	SSL_DlkTimersToRestart(UINT32	u32TimerId, 
								   UINT32	u32RestartVal)
{
	struct SSL_DlkTimerRestart	*pNode;
	struct SSL_DlkTimerRestart	*pTempNode;

	pNode = (struct SSL_DlkTimerRestart *)OS_ADPT_Allocate_Memory(SSL_pvDlkTimerMemPool, \
											sizeof(struct SSL_DlkTimerRestart));

	if(pNode == NULL)
	{
		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Memory Allocation Failure\n");
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		return SSL_DLK_TIMER_FAILURE;
	}

	pNode->u32RestartTimeOutVal	= u32RestartVal;
	pNode->u32TimerId			= u32TimerId;
	pNode->pNext				= NULL;

	/* Putting the node at the end */
	pTempNode = SSL_DlkTimerStVar.pHeadRestartTimer;

	if(pTempNode == NULL)
	{
		SSL_DlkTimerStVar.pHeadRestartTimer = pNode;
	}
	else
	{
		while(pTempNode->pNext != NULL)
		{
			pTempNode = pTempNode->pNext;
		}
		pTempNode->pNext = pNode;
	}

	return SSL_DLK_TIMER_SUCCESS;
}


/****************************************************************************
Function Name : DlkRestartTimersInList
Parameters	  : None
Return Value  : None
Description	  : This function restarts the timers contained in the linked list
				of timers to be restarted.
****************************************************************************/
static	void	SSL_DlkRestartTimersInList(void)
{
	struct SSL_DlkTimerRestart	*pTemp;
	struct SSL_DlkTimerRestart	*pNode;

	if(SSL_DlkTimerStVar.pHeadRestartTimer != NULL)
	{
		pTemp = SSL_DlkTimerStVar.pHeadRestartTimer;

		while(pTemp != NULL)
		{
			SSL_DlkSelfResetTimer(pTemp->u32TimerId, pTemp->u32RestartTimeOutVal);

			pNode = pTemp->pNext;
			OS_ADPT_Deallocate_Memory(pTemp);
			pTemp = NULL;
			pTemp = pNode;
		}

		SSL_DlkTimerStVar.pHeadRestartTimer = NULL;
	}
}


/****************************************************************************
Function Name : DlkPrintActiveList
Parameters	  : None
Return Value  : None
Description	  : This function prints the Delta Value, TimerId and Delta values
				of the timers in linked list of activer timers
****************************************************************************/
static	void	SSL_DlkPrintActiveList(void)
{
	#if SSL_DLK_TIMER_DEBUG_ALLOWED
		struct	SSL_DlkTimer	*pTemp;
		static	UINT32		su32CtrCounts = 0;

		pTemp = SSL_DlkTimerStVar.pHeadActiveTimer;

		#if SSL_DLK_TIMER_DEBUG_SCR_OUT_SHRT
			SSL_DLK_TIMER_DEBUG("In %d\n", ++su32CtrCounts);
		#endif /* DLK_TIMER_DEBUG_SCR_OUT_SHRT */

		#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
			SSL_DLK_TIMER_DEBUG("\n");
		#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */

		while(pTemp != NULL)
		{
			#if SSL_DLK_TIMER_DEBUG_FILE_OUT
			if(SSL_Dlk_Timer_FP)
			{
				fprintf(SSL_Dlk_Timer_FP, "%d %d %d\n", pTemp->u32TimerId, pTemp->u32TimeOutVal, pTemp->i32Delta);
			}
			#endif	/* DLK_TIMER_DEBUG_FILE_OUT */

			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("%d %d %d\n", pTemp->u32TimerId, pTemp->u32TimeOutVal, pTemp->i32Delta);
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */

			pTemp = pTemp->pNextTimer;
		}

		#if SSL_DLK_TIMER_DEBUG_FILE_OUT
		if(SSL_Dlk_Timer_FP)
		{
			fprintf(SSL_Dlk_Timer_FP, "\n");
		}
		#endif	/* DLK_TIMER_DEBUG_FILE_OUT */
	#endif	/* DLK_TIMER_DEBUG_ALLOWED */
}


/****************************************************************************
Function Name : DlkTimerDeltaRemain
Parameters	  : UINT32 u32TimerId - [IN] The timer identifier whose delta value
									remaining is to be returned
Return Value  : UINT32 - Returns the Delta value remaining for the timer to
						 expire, when the timer is active. Else returns 0.
Description	  : This function returns the Delta value of the timer remaining
				for the timer to expire
****************************************************************************/
UINT32	SSL_DlkTimerDeltaRemain(UINT32	u32TimerId)
{
	struct	SSL_DlkTimer	*pNode;
	INT32	i32DeltaRemain;
#if	SSL_DLK_TIMER_HASH_ENABLED
	struct	SSL_DlkTimerHashNodes	*pTempNodeHash;
#endif	/* DLK_TIMER_HASH_ENABLED */

	/* Semaphore Locking */
	SSL_DlkTimerLockSema();

#if	SSL_DLK_TIMER_HASH_ENABLED
	pTempNodeHash = SSL_DlkTimerGetHashNode(u32TimerId);
	if(pTempNodeHash != NULL)
	{
		pNode = pTempNodeHash->psAddrActiveNode;
	}
	else
	{
		pNode = NULL;
	}
#else
	pNode = SSL_DlkTimerLocateNode(SSL_DlkTimerStVar.pHeadActiveTimer, u32TimerId);
#endif	/* DLK_TIMER_HASH_ENABLED */

	if(pNode != NULL)
	{
		i32DeltaRemain = pNode->i32Delta;
	}
	else
	{
		i32DeltaRemain = 0;
	}

	/* Semaphore Unlocking */
	SSL_DlkTimerUnlockSema();

	return i32DeltaRemain;
}


/****************************************************************************
Function Name : DlkPrintTimerCounts
Parameters	  : None 
Return Value  : None
Description	  : This function prints the accounts of timers created, timers 
				deleted and the timers there in Active and Inactive list of
				timers
****************************************************************************/
void SSL_DlkPrintTimerCounts(void)
{
	#if SSL_DLK_TIMER_DEBUG_ALLOWED
	#if SSL_DLK_TIMER_DEBUG_COUNTS
		UINT32	u32ALCtr = 0;
		UINT32	u32ILCtr = 0;
		struct	SSL_DlkTimer *pNode;

		pNode = SSL_DlkTimerStVar.pHeadActiveTimer;

		while(pNode != NULL)
		{
			u32ALCtr++;
			pNode = pNode->pNextTimer;
		}

		pNode = SSL_DlkTimerStVar.pHeadInactiveTimer;

		while(pNode != NULL)
		{
			u32ILCtr++;
			pNode = pNode->pNextTimer;
		}

		SSL_DLK_TIMER_DEBUG("TC:%d TD:%d TAL:%d TIL:%d\n", DlkTimerStVar.u32TimersCreated, DlkTimerStVar.u32TimersDeleted, u32ALCtr, u32ILCtr);
	#endif	//DLK_TIMER_DEBUG_COUNTS
	#endif	//DLK_TIMER_DEBUG_ALLOWED
}


/****************************************************************************
Function Name : DlkTimerGetTimeOfDay
Parameters	  : None
Return Value  : UINT32 - Time of the day
Description	  : This function returns the time in seconds
****************************************************************************/
SSL_dlk_time	SSL_DlkTimerGetTimeOfDay(void)
{
	/* Variable(s) Decleration */
	SSL_dlk_time	tm = {0,0};

	/* For Windows */
	#if	SSL_DLK_TIMER_WINDOWS
		time_t	time_ds;
		tm.sec = time(&time_ds);
	#elif SSL_DLK_TIMER_LINUX
		struct timeval timer_now;
		gettimeofday (&timer_now, NULL);
		tm.sec = timer_now.tv_sec;
		tm.micro = timer_now.tv_usec;
	#elif SSL_DLK_TIMER_NUCLEUS
		UINT32 u32TempStamp;
		u32TempStamp = OS_ADPT_Glue_Now();
		tm.sec       = u32TempStamp / 1000;	//Since OS_ADPT_Glue_Now returns in
											//msec
		u32TempStamp = u32TempStamp % 1000;
		tm.micro     = u32TempStamp * 1000;
	#endif

	return tm;
}


/****************************************************************************
Function Name : DlkElapsedTime 
Parameters	  : dlk_time sCurTick - [IN] Current time in secs & usecs
Return Value  : UINT32 - Difference between last updation and current time. 
Description	  : This function calculates the time elapsed since last updation.
				The difference returned can be in milli , micro or secs depe-
				nding on configuration.
******************************************************************************/
UINT32	SSL_DlkElapsedTime(SSL_dlk_time	sCurTick)
{
	/* Variable(s) Decleration */
	UINT32		u32Diff;
	SSL_dlk_time	sDiff;
	SSL_dlk_time	sLastTime;

	/* Variable(s) Initialization */
	u32Diff     = 0;
	sDiff.micro = 0;
	sDiff.sec   = 0;
	sLastTime   = SSL_DlkTimerStVar.sTODFrstNodeCr;

	if(sCurTick.sec >= sLastTime.sec)
	{
		sDiff.sec = sCurTick.sec - sLastTime.sec;
		if(sCurTick.micro >= sLastTime.micro)
		{
			sDiff.micro = sCurTick.micro - sLastTime.micro;
		}
		else /* micro wrap around*/
		{
			sDiff.micro = sCurTick.micro + (1000000 - sLastTime.micro) ;
			sDiff.sec -= 1;
		}
	}
	else /* sec wrap around*/
	{
		sDiff.sec = sCurTick.sec + ( 0xffffffff - sLastTime.sec);
		if(sCurTick.micro >= sLastTime.micro)
		{
			sDiff.micro = sCurTick.micro - sLastTime.micro;
		}
		else /* micro wrap around*/
		{
			sDiff.micro = sCurTick.micro + (1000000 - sLastTime.micro);
			sDiff.sec -= 1;
		}
	}
#if SSL_DLK_TIMER_MILLI
	u32Diff = sDiff.micro/1000;
	u32Diff += sDiff.sec * 1000;
#elif SSL_DLK_TIMER_MICRO
	u32Diff = sDiff.micro;
	u32Diff += sDiff.sec * 1000000;
#else
	u32Diff = sDiff.sec;
#endif

	return u32Diff;
}


/****************************************************************************
Function Name : DlkTimerLockSema
Parameters	  : None
Return Value  : None
Description	  : This function is used for Locking the semaphore
******************************************************************************/
static	void	SSL_DlkTimerLockSema(void)
{
	#if	SSL_DLK_TIMER_NUCLEUS

		/* lock the semaphore */
		OS_ADPT_Lock_Semaphore(SSL_pvDlkTimerSema);	//For Time Being...YKS
	
	#elif SSL_DLK_TIMER_LINUX

		pthread_mutex_trylock(&SSL_semTimerMutex);

	#endif	/* DLK_TIMER_NUCLEUS */
}


/****************************************************************************
Function Name : DlkTimerUnlockSema
Parameters	  : None
Return Value  : None
Description	  : This function is used for Unlocking the semaphore
******************************************************************************/
static	void	SSL_DlkTimerUnlockSema(void)
{
	#if	SSL_DLK_TIMER_NUCLEUS

		/* Semaphore Unlocked */
		OS_ADPT_Unlock_Semaphore(SSL_pvDlkTimerSema);	//For Time Being...YKS
	
	#elif SSL_DLK_TIMER_LINUX

		pthread_mutex_unlock(&SSL_semTimerMutex);		

	#endif
}


/****************************************************************************
Function Name : DlkSelfCreateTimer
Parameters	  : UINT32	*pu32TimerId			 - [OUT] Will contain the uni-
												   que Timer ID generated.
	  	        UINT32	u32TimeoutValue	         - [IN] The timeout value after 
										           which the timer will expire.
				void	(*pCallBack)(void *pPrm) - [IN] Callback routine to be 
												   invoked at the time of timer
												   expiry.
				void	*pPrm					 - [IN] Argument for callback 
												   routine.
				UINT8	u8Type					 - [IN] Input argument 
												   specifying the type of timer,
												   whether One-shot or cyclic/
												   recursive.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description	  : This is an internal function performing the functionality of 
				creating a timer
******************************************************************************/
static	INT32	SSL_DlkSelfCreateTimer(UINT32	*pu32TimerId,
								   UINT32	u32TimeoutValue,
								   //void		(*pCallBack)(void *pPrm),
								   void		(*pCallBack)(struct SslTimerQ *pPrm),
								   void		*pPrm,
								   UINT8	u8Type)
{
	/* Variable(s) Decleration */
	struct	SSL_DlkTimer	*pNewNode;
	struct	SSL_DlkTimer	*pTemp;
	struct	SSL_DlkTimer	*pPrev;

	/* Checking the input arguments */
	if((u32TimeoutValue == 0)
	   || (pCallBack == NULL)
	   || ((u8Type != SSL_DLK_TIMER_ONE_SHOT) && (u8Type != SSL_DLK_TIMER_RECURSIVE)))
	{
		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Incorrect input arguments\n");
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		*pu32TimerId = 0;
		return SSL_DLK_TIMER_FAILURE;
	}

	/* Variable(s) Initialization */
	pPrev = NULL;

	/* If timeout value is less than equal to ZERO than return failure */
	//Not possible since it is an unsigned variable....in future, if changing to
	//signed variable, then activate this check
	//if(u32TimeoutValue <= 0)
	//{
	//	*pu32TimerId = 0;
	//	return DLK_TIMER_FAILURE;
	//}

	/* Allocating memory to the New Node */
	pNewNode = (struct SSL_DlkTimer *)OS_ADPT_Allocate_Memory(SSL_pvDlkTimerMemPool, sizeof(struct SSL_DlkTimer));

	/* In case of memory allocation failure, return */
	if(pNewNode == NULL)
	{
		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Memory Allocation Failure\n");
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		*pu32TimerId = 0;
		return SSL_DLK_TIMER_FAILURE;
	}

	/* Filling up the values in the New Node created */
	pNewNode->pCallBack		= pCallBack;
	pNewNode->pNextTimer	= NULL;
	pNewNode->pPrevTimer	= NULL;
	pNewNode->pPrm			= pPrm;
	pNewNode->i32Delta		= 0;
	pNewNode->u32TimeOutVal = u32TimeoutValue;
	pNewNode->u8Type		= u8Type;
	if(SSL_DlkTimerStVar.u32TimerIdCtr == 0)
	{
		SSL_DlkTimerStVar.u32TimerIdCtr = 1;
	}
	pNewNode->u32TimerId	= SSL_DlkTimerStVar.u32TimerIdCtr++;

	*pu32TimerId			= pNewNode->u32TimerId;

	/* Locating an appropriate place to Insert the Timer, generally added at the
	   end of linked list */
	if(SSL_DlkTimerStVar.pHeadInactiveTimer == NULL)
	{
		SSL_DlkTimerStVar.pHeadInactiveTimer = pNewNode;
	}
	else
	{
		pTemp = SSL_DlkTimerStVar.pHeadInactiveTimer;

		while((pTemp != NULL) && (pTemp->u32TimerId < pNewNode->u32TimerId))
		{
			if(pTemp->pNextTimer == NULL)
			{
				pPrev = pTemp;
			}
			pTemp = pTemp->pNextTimer;
		}
		if(pTemp == NULL)
		{
			/* That is, inserting at the end */
			pPrev->pNextTimer    = pNewNode;
			pNewNode->pPrevTimer = pPrev;
		}
		else
		{
			/* When inserting in the chain, somewhere in between */
			if(pTemp != SSL_DlkTimerStVar.pHeadInactiveTimer)
			{
				pTemp->pPrevTimer->pNextTimer	= pNewNode;
				pNewNode->pPrevTimer			= pTemp->pPrevTimer;
				pTemp->pPrevTimer				= pNewNode;
				pNewNode->pNextTimer			= pTemp;
			}
			/* When inserting at the start of the chain */
			else
			{
				pNewNode->pNextTimer = SSL_DlkTimerStVar.pHeadInactiveTimer;
				SSL_DlkTimerStVar.pHeadInactiveTimer->pPrevTimer = pNewNode;
				SSL_DlkTimerStVar.pHeadInactiveTimer			 = pNewNode;
			}
		}
	}

	/* Incrementing the count of Timers created this far */
	SSL_DlkTimerStVar.u32TimersCreated++;

#if	SSL_DLK_TIMER_HASH_ENABLED
	/* Adding the same in the HASH List */
	SSL_DlkTimerAddHashNode(pNewNode->u32TimerId, NULL, pNewNode);
#endif	/* DLK_TIMER_HASH_ENABLED */

	return SSL_DLK_TIMER_SUCCESS;
}


/****************************************************************************
Function Name : DlkSelfStartTimer
Parameters	  : UINT32	u32TimerId - [IN] The timer link identifier.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description	  : This is an internal function performing the functionality of 
				starting a timer
******************************************************************************/
static	INT32	SSL_DlkSelfStartTimer(UINT32	u32TimerId)
{
	/* Variable(s) Decleration */
	struct	SSL_DlkTimer	*pTempNode;
	struct	SSL_DlkTimer	*pNode;
#if	SSL_DLK_TIMER_HASH_ENABLED
	struct	SSL_DlkTimerHashNodes	*pTempNodeHash;
#endif	/* DLK_TIMER_HASH_ENABLED */

	/* Allocating memory to the Node to be inserted */
	pTempNode = (struct SSL_DlkTimer *)OS_ADPT_Allocate_Memory(SSL_pvDlkTimerMemPool, sizeof(struct SSL_DlkTimer));

	/* Returning in case of Memory allocation failure */
	if(pTempNode == NULL)
	{
		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Memory Allocation Failure\n");
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		return SSL_DLK_TIMER_FAILURE;
	}

	/* Searching the Active list if the timer which we are abt to add is already
	   there */
#if	SSL_DLK_TIMER_HASH_ENABLED
	pTempNodeHash = SSL_DlkTimerGetHashNode(u32TimerId);
	if(pTempNodeHash != NULL)
	{
		pNode = pTempNodeHash->psAddrActiveNode;
	}
	else
	{
		pNode = NULL;
	}
#else
	pNode = SSL_DlkTimerLocateNode(SSL_DlkTimerStVar.pHeadActiveTimer, u32TimerId);
#endif	/* DLK_TIMER_HASH_ENABLED */
	
	if(pNode != NULL)
	{
		OS_ADPT_Deallocate_Memory(pTempNode);
		pTempNode = NULL;

		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Timer with Timer Id %d is already running\n", u32TimerId);
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		return SSL_DLK_TIMER_FAILURE;
	}

	/* Identifying the node to be moved from list of Inactive timers to Active 
	   timers */
#if	SSL_DLK_TIMER_HASH_ENABLED
	pTempNodeHash = SSL_DlkTimerGetHashNode(u32TimerId);
	if(pTempNodeHash != NULL)
	{
		pNode = pTempNodeHash->psAddrInactiveNode;
	}
	else
	{
		pNode = NULL;
	}
#else
	pNode = SSL_DlkTimerLocateNode(SSL_DlkTimerStVar.pHeadInactiveTimer, u32TimerId);
#endif	/* DLK_TIMER_HASH_ENABLED */

	if(pNode == NULL)
	{
		OS_ADPT_Deallocate_Memory(pTempNode);
		pTempNode = NULL;

		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Unable to locate the Node with Timer Id %d\n", u32TimerId);
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		return SSL_DLK_TIMER_FAILURE;
	}

	/* Copying the data from the node to the new node to be inserted */
	memcpy(pTempNode, pNode, sizeof(struct SSL_DlkTimer));
	pTempNode->pNextTimer = NULL;
	pTempNode->pPrevTimer = NULL;

	/* Inserting the node in the list of Active timers */
	if(SSL_DlkTimerInsertActive(pTempNode) != SSL_DLK_TIMER_SUCCESS)
	{
		OS_ADPT_Deallocate_Memory(pTempNode);
		pTempNode = NULL;
		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Unable to insert the node in the Active timer linked list\n");
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		return SSL_DLK_TIMER_FAILURE;
	}

#if	SSL_DLK_TIMER_HASH_ENABLED
	/* Updating the same in the HASH List */
	SSL_DlkTimerUpdateHashNode(pTempNode->u32TimerId, pTempNode);
#endif	/* DLK_TIMER_HASH_ENABLED */

	return SSL_DLK_TIMER_SUCCESS;
}


/****************************************************************************
Function Name : DlkSelfStopTimer
Parameters	  : UINT32	u32TimerId - [IN] The timer link identifier.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description	  : This is an internal function performing the functionality of 
				stopping a timer
******************************************************************************/
static	INT32	SSL_DlkSelfStopTimer(UINT32		u32TimerId)
{
	/* Variable(s) Decleration */
	struct	SSL_DlkTimer	*pNode;
#if	SSL_DLK_TIMER_HASH_ENABLED
	struct	SSL_DlkTimerHashNodes	*pTempNodeHash;
#endif	/* DLK_TIMER_HASH_ENABLED */

	/* Identifying the node to be moved out from the list of Active timers */
#if	SSL_DLK_TIMER_HASH_ENABLED
	pTempNodeHash = SSL_DlkTimerGetHashNode(u32TimerId);
	if(pTempNodeHash != NULL)
	{
		pNode = pTempNodeHash->psAddrActiveNode;
	}
	else
	{
		pNode = NULL;
	}
#else
	pNode = SSL_DlkTimerLocateNode(SSL_DlkTimerStVar.pHeadActiveTimer, u32TimerId);
#endif	/* DLK_TIMER_HASH_ENABLED */

	if(pNode == NULL)
	{
		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Unable to locate the Node with Timer Id %d\n", u32TimerId);
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		return SSL_DLK_TIMER_FAILURE;
	}

	/* Removing the node from the list of Active timers */
	if(pNode->pPrevTimer != NULL)
	{
		pNode->pPrevTimer->pNextTimer = pNode->pNextTimer;
	}
	else
	{
		SSL_DlkTimerStVar.pHeadActiveTimer = pNode->pNextTimer;
	}

	if(pNode->pNextTimer != NULL)
	{
		pNode->pNextTimer->pPrevTimer = pNode->pPrevTimer;
		pNode->pNextTimer->i32Delta += pNode->i32Delta;
	}

	/* Freeing the node */
	OS_ADPT_Deallocate_Memory(pNode);


#if	SSL_DLK_TIMER_HASH_ENABLED
	/* Updating the same in the HASH List */
	SSL_DlkTimerUpdateHashNode(u32TimerId, NULL);
#endif	/* DLK_TIMER_HASH_ENABLED */

	return SSL_DLK_TIMER_SUCCESS;
}


/****************************************************************************
Function Name : DlkSelfDeleteTimer
Parameters	  : UINT32	u32TimerId - [IN] The timer link identifier, to be
									 removed.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE othe-
						rwise
Description	  : This is an internal function performing the functionality of 
				deleting a timer
******************************************************************************/
static	INT32	SSL_DlkSelfDeleteTimer(UINT32	u32TimerId)
{
	/* Variable(s) Decleration */
	INT32	i32RetVal;
	INT8	i8Found;
	struct	SSL_DlkTimer	*pTemp;
#if	SSL_DLK_TIMER_HASH_ENABLED
	struct	SSL_DlkTimerHashNodes	*pTempNodeHash;
#endif	/* DLK_TIMER_HASH_ENABLED */

	/* Variable(s) Initialization */
	i32RetVal = SSL_DLK_TIMER_SUCCESS;
	i8Found	  = SSL_DLK_TIMER_FAILURE;

#if	SSL_DLK_TIMER_HASH_ENABLED
	pTempNodeHash = SSL_DlkTimerGetHashNode(u32TimerId);

	if((pTempNodeHash != NULL) && (pTempNodeHash->psAddrActiveNode != NULL))
	{
		pTemp   = pTempNodeHash->psAddrActiveNode;
		i8Found = SSL_DLK_TIMER_SUCCESS;
	}
#else
	/* Locating the node in the Active timers' list */
	pTemp = SSL_DlkTimerStVar.pHeadActiveTimer;

	while(pTemp != NULL)
	{
		if(pTemp->u32TimerId == u32TimerId)
		{
			i8Found = SSL_DLK_TIMER_SUCCESS;
			break;
		}

		pTemp = pTemp->pNextTimer;
	}
#endif	/* DLK_TIMER_HASH_ENABLED */

	if(i8Found == SSL_DLK_TIMER_SUCCESS)
	{
		/* Shuffling the pointers */
		if(pTemp->pNextTimer != NULL)
		{
			pTemp->pNextTimer->pPrevTimer = pTemp->pPrevTimer;
			pTemp->pNextTimer->i32Delta += pTemp->i32Delta;
		}

		if(pTemp->pPrevTimer != NULL)
		{
			pTemp->pPrevTimer->pNextTimer = pTemp->pNextTimer;
		}
		else
		{
			/* If the node to be deleted is the Head node then assign next 
			   node to the Head node */
			SSL_DlkTimerStVar.pHeadActiveTimer = pTemp->pNextTimer;
		}

		/* Freeing the memory */
		OS_ADPT_Deallocate_Memory(pTemp);
		pTemp = NULL;

		i8Found = SSL_DLK_TIMER_FAILURE;
	}

#if	SSL_DLK_TIMER_HASH_ENABLED
	pTempNodeHash = SSL_DlkTimerGetHashNode(u32TimerId);

	if((pTempNodeHash != NULL) && (pTempNodeHash->psAddrInactiveNode != NULL))
	{
		pTemp   = pTempNodeHash->psAddrInactiveNode;
		i8Found = SSL_DLK_TIMER_SUCCESS;
	}
#else
	/* Locating the node in the Inactive timers' list */
	pTemp = SSL_DlkTimerStVar.pHeadInactiveTimer;

	while(pTemp != NULL)
	{
		if(pTemp->u32TimerId == u32TimerId)
		{
			i8Found = SSL_DLK_TIMER_SUCCESS;
			break;
		}

		pTemp = pTemp->pNextTimer;
	}
#endif	/* DLK_TIMER_HASH_ENABLED */

	if(i8Found == SSL_DLK_TIMER_SUCCESS)
	{
		/* Shuffling the pointers */
		if(pTemp->pNextTimer != NULL)
		{
			pTemp->pNextTimer->pPrevTimer = pTemp->pPrevTimer;
		}

		if(pTemp->pPrevTimer != NULL)
		{
			pTemp->pPrevTimer->pNextTimer = pTemp->pNextTimer;
		}
		else
		{
			/* If the node to be deleted is the Head node then assign next 
			   node to the Head node */
			SSL_DlkTimerStVar.pHeadInactiveTimer = pTemp->pNextTimer;
		}

		/* Freeing the memory */
		OS_ADPT_Deallocate_Memory(pTemp);
		pTemp = NULL;

		/* Incrementing the counter of Timers deleted this far */
		SSL_DlkTimerStVar.u32TimersDeleted++;

#if	SSL_DLK_TIMER_HASH_ENABLED
		/* Deleting the same from the HASH List */
		SSL_DlkTimerDeleteHashNode(u32TimerId);
#endif	/* DLK_TIMER_HASH_ENABLED */
	}

	return i32RetVal;
}


/****************************************************************************
Function Name : DlkSelfResetTimer
Parameters	  : UINT32	u32TimerId			 - [IN] The timer link identifier.
				UINT32	u32ResetTimeoutValue - [IN] The time out of the timer
											   to be reset again with.
Return Value  : INT32 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE 
					    otherwise
Description	  : This is an internal function performing the functionality of 
				resetting a timer
******************************************************************************/
INT32	SSL_DlkSelfResetTimer(UINT32	u32TimerId,
						  UINT32	u32ResetTimeoutValue)
{
	/* Variable(s) Decleration */
	struct	SSL_DlkTimer	*pNode;
	struct	SSL_DlkTimer	*pNodeInactive;
#if	SSL_DLK_TIMER_HASH_ENABLED
	struct	SSL_DlkTimerHashNodes	*pTempNodeHash;
#endif	/* DLK_TIMER_HASH_ENABLED */

	/* Identifying the node to be copied from list of Inactive timers to Active 
	   timers */
#if	SSL_DLK_TIMER_HASH_ENABLED
	pTempNodeHash = SSL_DlkTimerGetHashNode(u32TimerId);
	if(pTempNodeHash != NULL)
	{
		pNodeInactive = pTempNodeHash->psAddrInactiveNode;
	}
	else
	{
		pNodeInactive = NULL;
	}
#else
	pNodeInactive = SSL_DlkTimerLocateNode(SSL_DlkTimerStVar.pHeadInactiveTimer, u32TimerId);
#endif	/* DLK_TIMER_HASH_ENABLED */

	if(pNodeInactive == NULL)
	{
		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Unable to locate the Node with Timer Id %d\n", u32TimerId);
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		return SSL_DLK_TIMER_FAILURE;
	}

	/* Searching the Active list if the timer which we are abt to add is already there */
#if	SSL_DLK_TIMER_HASH_ENABLED
	pTempNodeHash = SSL_DlkTimerGetHashNode(u32TimerId);
	if(pTempNodeHash != NULL)
	{
		pNode = pTempNodeHash->psAddrActiveNode;
	}
	else
	{
		pNode = NULL;
	}
#else
	pNode = SSL_DlkTimerLocateNode(SSL_DlkTimerStVar.pHeadActiveTimer, u32TimerId);
#endif	/* DLK_TIMER_HASH_ENABLED */

	if(pNode != NULL)
	{
		SSL_DlkSelfStopTimer(u32TimerId);
	}

	/* Allocating memory to the Node to be inserted */
	pNode = (struct SSL_DlkTimer *)OS_ADPT_Allocate_Memory(SSL_pvDlkTimerMemPool, sizeof(struct SSL_DlkTimer));

	/* Returning in case of Memory allocation failure */
	if(pNode == NULL)
	{
		#if SSL_DLK_TIMER_DEBUG_ALLOWED
			#if SSL_DLK_TIMER_DEBUG_SCREEN_OUT
				SSL_DLK_TIMER_DEBUG("Memory Allocation Failure\n");
			#endif /* DLK_TIMER_DEBUG_SCREEN_OUT */
		#endif	/* DLK_TIMER_DEBUG_ALLOWED */

		return SSL_DLK_TIMER_FAILURE;
	}

	/* Copying the data from the node to the new node to be inserted */
	memcpy(pNode, pNodeInactive, sizeof(struct SSL_DlkTimer));
	pNode->u32TimeOutVal = u32ResetTimeoutValue;
	pNode->pNextTimer	 = NULL;
	pNode->pPrevTimer	 = NULL;

	/* Inserting the node in the list of Active timers */
	SSL_DlkTimerInsertActive(pNode);

#if	SSL_DLK_TIMER_HASH_ENABLED
	/* Updating the same in the HASH List */
	SSL_DlkTimerUpdateHashNode(pNode->u32TimerId, pNode);
#endif	/* DLK_TIMER_HASH_ENABLED */

	return SSL_DLK_TIMER_SUCCESS;
}


#if	SSL_DLK_TIMER_HASH_ENABLED
/****************************************************************************
Function Name : DlkTimerGetHashNode
Parameters	  : UINT32	u32TimerId - [IN] The timer identifier, index in hash 
									 list
Return Value  : struct DlkTimerHashNodes * - The found node on success, NULL
											 otherwise
Description	  : This is an internal function performing the functionality of 
				getting the node from the hash list indexed by the Timer ID
******************************************************************************/
static	struct		SSL_DlkTimerHashNodes	*SSL_DlkTimerGetHashNode(UINT32	u32TimerId)
{
	/* Variable(s) Declaration */
	UINT32	u32HashedVal;
	struct	SSL_DlkTimerHashNodes	*psHashNodeTemp;

	/* Variable(s) Initialization */
	u32HashedVal   = SSL_DLK_TIMER_GET_HASH_VAL(u32TimerId);

	/* Locating the entry */
	for(psHashNodeTemp = SSL_DlkTimerHashList[u32HashedVal]; 
	    psHashNodeTemp != NULL; 
		psHashNodeTemp = psHashNodeTemp->psNext)
	{
		if(u32TimerId == psHashNodeTemp->u32TimerId)
		{
			/* When located */
			break;
		}
	}

	return psHashNodeTemp;
}
#endif	/* DLK_TIMER_HASH_ENABLED */

#if	SSL_DLK_TIMER_HASH_ENABLED
/****************************************************************************
Function Name : DlkTimerAddHashNode
Parameters	  : UINT32 u32TimerId					- [IN] The Timer Id, hash
														   index
				struct DlkTimer *psActiveAddrPtr	- [IN] The pointer to node
														   in active list
				struct DlkTimer *psInactiveAddrPtr	- [IN] The pointer to node
														   in inactive list
Return Value  : INT8 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE 
					   otherwise
Description	  : This is an internal function performing the functionality of 
				adding a node in the hash list of timers
******************************************************************************/
static	INT8	SSL_DlkTimerAddHashNode(UINT32				u32TimerId, 
									struct	SSL_DlkTimer	*psActiveAddrPtr, 
									struct	SSL_DlkTimer	*psInactiveAddrPtr)
{
	/* Variable(s) Declaration */
	UINT32	u32HashVal;
	struct	SSL_DlkTimerHashNodes	*psHashNodeTemp;
	struct	SSL_DlkTimerHashNodes	*psHashNodeNew;

	/* Variable(s) Initialization */
	u32HashVal = SSL_DLK_TIMER_GET_HASH_VAL(u32TimerId);

	psHashNodeTemp = SSL_DlkTimerHashList[u32HashVal];

	/* Searching the hash list for the same TimerID, for verifying an already 
	    existing node with the same timer ID */
	psHashNodeNew = SSL_DlkTimerGetHashNode(u32TimerId);

	if(psHashNodeNew != NULL)
	{
		return SSL_DLK_TIMER_FAILURE;
	}

	/* Creating the node */
	psHashNodeNew = SSL_DlkTimerCreateHashNode();
	if(psHashNodeNew == NULL)
	{
		return SSL_DLK_TIMER_FAILURE;
	}

	/* Filling the fields */
	psHashNodeNew->psAddrActiveNode   = psActiveAddrPtr;
	psHashNodeNew->psAddrInactiveNode = psInactiveAddrPtr;
	psHashNodeNew->u32TimerId		  = u32TimerId;
	psHashNodeNew->psNext			  = NULL;
	psHashNodeNew->psPrev			  = NULL;

	/* Adding the node */
	if(psHashNodeTemp == NULL)
	{
		SSL_DlkTimerHashList[u32HashVal] = psHashNodeNew;
	}
	else
	{
		while(psHashNodeTemp->psNext != NULL)
		{
			psHashNodeTemp = psHashNodeTemp->psNext;
		}

		psHashNodeTemp->psNext = psHashNodeNew;
		psHashNodeNew->psPrev  = psHashNodeTemp;
	}

	return SSL_DLK_TIMER_SUCCESS;
}
#endif	/* DLK_TIMER_HASH_ENABLED */

#if	SSL_DLK_TIMER_HASH_ENABLED
/****************************************************************************
Function Name : DlkTimerDeleteHashNode
Parameters	  : UINT32	u32TimerId - [IN] The timer identifier, index in hash
										  list
Return Value  : INT8 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE 
					   otherwise
Description	  : This is an internal function performing the functionality of 
				deleting the node from the Hash list of timers
******************************************************************************/
static	INT8		SSL_DlkTimerDeleteHashNode(UINT32	u32TimerId)
{
	/* Variable(s) Declaration */
	UINT32	u32HashVal;
	struct	SSL_DlkTimerHashNodes	*psHashNodeTemp;
	struct	SSL_DlkTimerHashNodes	*psHashNodePrev;
	struct	SSL_DlkTimerHashNodes	*psHashNodeDel;

	/* Variable(s) Initialization */
	psHashNodePrev = NULL;

	/* Searching the node */
	psHashNodeDel = SSL_DlkTimerGetHashNode(u32TimerId);

	if(psHashNodeDel == NULL)
	{
		return SSL_DLK_TIMER_FAILURE;
	}

	/* Deleting the node */
	u32HashVal = SSL_DLK_TIMER_GET_HASH_VAL(u32TimerId);

	if(psHashNodeDel->psPrev == NULL)
	{
		SSL_DlkTimerHashList[u32HashVal] = psHashNodeDel->psNext;
	}
	else
	{
		psHashNodeDel->psPrev->psNext = psHashNodeDel->psNext;
	}
	
	if(psHashNodeDel->psNext != NULL)
	{
		psHashNodeDel->psNext->psPrev = psHashNodeDel->psPrev;
	}

	/* Deallocating the memory */
	OS_ADPT_Deallocate_Memory(psHashNodeDel);

	return SSL_DLK_TIMER_SUCCESS;
}
#endif	/* DLK_TIMER_HASH_ENABLED */

#if	SSL_DLK_TIMER_HASH_ENABLED
/****************************************************************************
Function Name : DlkTimerUpdateHashNode
Parameters	  : UINT32 u32TimerId					- [IN] The Timer Id, hash
														   index
				struct DlkTimer *psActiveAddrPtr	- [IN] The pointer to node
														   in active list
Return Value  : INT8 - DLK_TIMER_SUCCESS on success, DLK_TIMER_FAILURE 
					   otherwise
Description	  : This is an internal function performing the functionality of 
				updating a node in the hash list of timers
******************************************************************************/
static	INT8	SSL_DlkTimerUpdateHashNode(UINT32	u32TimerId,
									   struct	SSL_DlkTimer	*psActiveAddrPtr)
{
	/* Variable(s) Declaration */
	struct	SSL_DlkTimerHashNodes	*psHashNodeUpdate;

	/* Searching for the node */
	psHashNodeUpdate = SSL_DlkTimerGetHashNode(u32TimerId);

	if(psHashNodeUpdate == NULL)
	{
		return SSL_DLK_TIMER_FAILURE;
	}

	/* Updating the node */
	psHashNodeUpdate->psAddrActiveNode = psActiveAddrPtr;

	return SSL_DLK_TIMER_SUCCESS;
}
#endif	/* DLK_TIMER_HASH_ENABLED */

#if	SSL_DLK_TIMER_HASH_ENABLED
/****************************************************************************
Function Name : DlkTimerCreateHashNode
Parameters	  : None
Return Value  : struct DlkTimerHashNodes * - NULL if memory allocation is 
										     unsuccessful, else pointer to 
											 allocated memory block
Description	  : This is an internal function for allocating the memory to
				the node for Hash table
******************************************************************************/
static	struct	SSL_DlkTimerHashNodes	*SSL_DlkTimerCreateHashNode(void)
{
	/* Variable(s) Declaration */
	struct	SSL_DlkTimerHashNodes	*psHashNodeCreate;

	psHashNodeCreate = (struct SSL_DlkTimerHashNodes *)OS_ADPT_Allocate_Memory(SSL_pvDlkTimerMemPool, sizeof(struct SSL_DlkTimerHashNodes));

	if(psHashNodeCreate != NULL)
	{
		memset(psHashNodeCreate, 0, sizeof(struct SSL_DlkTimerHashNodes));
	}

	return psHashNodeCreate;
}
#endif	/* DLK_TIMER_HASH_ENABLED */

