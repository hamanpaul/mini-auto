/*****************************************************************************************

* Copyright Information :	This software is the property of D-Link and shall not be 
							reproduced distributed and copied without the permission 
							from D-Link

* Module Name			:	Secure Socket Layer

* Interface Spec		:	None.

* Description			:	This file contains OS dependent functions which needs to be
							modified for porting

* Author Name			:	M.K.Saravanan

* Revision				:	1.0

* Known Bugs			:	None

******************************************************************************************/
#define DEFINE_SSL_MEMORY
#include <SSLCommon.h>
#include <DlkTimer.h>
#include <dlk_ssl.h>

//BEGIN - YKS & Debu... 13/01/2004
#define	SSL_ELEMENT_SIZE	1	/* Size of the data structure to put in the queue */
#define	SSL_NO_OF_NODES		60	/* Number of nodes in the queue */
#define	SSL_QUEUE_SIZE		(SSL_ELEMENT_SIZE * SSL_NO_OF_NODES)

extern	void				OS_ADPT_Delete_Memory_Pool(void *);
#if SSH_SERVER_INCLUDED
	 extern BOOLEAN_T   InitSSH_Done;
#endif

UINT8						*pu8SslQueueBuf;
static	NU_QUEUE			SslQueueID;


void	SslWriteToQ(struct SslTimerQ *);
void	SslReadFromQ(void);
INT8	SslCreateTimer(UINT8 *, UINT32, UINT32, void (*)(UINT8 *, UINT32));
void	SslTimerRemoveDs(UINT8 *, UINT32);
//END - YKS & Debu...


/*****************************************************************************************
* Function Name	:	SSLWriteToTransport
* Parameters	:	INT32 lSocketDescr	- (IN) The socket descriptor for sending
					UINT8 *pData - (IN) The data to be sent
					INT32 lSize - (IN) The data size
* Return Value	:	INT32 - No of bytes sent on Success
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case the socket would block for this 
							operation
* Description	:	This function sends the data passed to the socket.
******************************************************************************************/
INT32 SSLWriteToTransport(INT32 lSocketDescr, UINT8 *pData, INT32 lSize)
{
	INT32	lSentSize;

	/* send the data to TCP */
	lSentSize = send(lSocketDescr, pData, lSize, 0);
#if SSL_CFG_WINDOWS
	if (lSentSize == SOCKET_ERROR) {
		/* windows error handling */
		if(WSAGetLastError() == WSAEWOULDBLOCK) {
			return SSL_WOULD_BLOCK;
		} else {
			return SSL_ERROR;
		}
	}
#endif
#if SSL_CFG_NUCLEUS
	if (lSentSize < 0) {
		/* nucleus error handling */
		if(lSentSize == EWOULDBLOCK) {
			return SSL_WOULD_BLOCK;
		} else {
			return SSL_ERROR;
		}
	}
#endif
	return lSentSize;
}

/*****************************************************************************************
* Function Name	:	SSLReadFromTransport
* Parameters	:	INT32 lSocketDescr	- (IN) The socket descriptor for sending
					UINT8 *pData - (OUT) The pointer to data
					INT32 lDataSize - (IN) The data size
* Return Value	:	INT32 - No of bytes received on Success
							SSL_CONNECTION_CLOSED(0) on case TCP connection closed
							SSL_ERROR(-1) in case of socket error
							SSL_WOULD_BLOCK(-2) in case the socket would block for this 
							operation
* Description	:	This function receives specifed no. of bytes from TCP
******************************************************************************************/
INT32 SSLReadFromTransport(INT32 lSocketDescr, UINT8 *pData, INT32 lDataSize)
{
	INT32                   lRecvSize = 0;
    struct timeval          tout;
    fd_set                  rs;   
    int                     n=0, loop ;

#if 1 /* zenrong , 2004/11/24 08:23¤U¤È*/
    /* fix SSL connection is closed if no data comes.
     * if there is no any data coming after 15 min, this connection should be 
     * closed.
     */  
    tout.tv_sec=60;
#else        
    tout.tv_sec=1;
#endif    
    tout.tv_usec=0;
    
    FD_ZERO(&rs);
    FD_SET(lSocketDescr,&rs);

#if 1 
/*++ steven, 2004/9/3 
 * check if it is readable on the socket,
 * else task will hang on the socket read. 
 */		
#if 1 /* zenrong 2004/11/24 08:23¤U¤È*/ 
    for (loop = 0; loop < 15; loop ++)
    {
        FD_ZERO(&rs);
        FD_SET(lSocketDescr,&rs);
        n = select(lSocketDescr+1, &rs, NULL, NULL, &tout);	
        if(n <= 0) 
            continue;    
        else
            break;          
    }
#endif    
    if(n <= 0)
        return lRecvSize;
/*-- steven */	
#endif
	/* reads the data from TCP */
	lRecvSize = recv(lSocketDescr, pData, lDataSize, 0);
#if SSL_CFG_WINDOWS
	if (lRecvSize == SOCKET_ERROR) {
		/* windows error handling */
		if(WSAGetLastError() == WSAEWOULDBLOCK) {
			return SSL_WOULD_BLOCK;
		} else {
			return SSL_ERROR;
		}
	}
#endif
#if SSL_CFG_NUCLEUS
	if (lRecvSize < 0) {
		/* nucleus error handling */
		if(lRecvSize == EWOULDBLOCK) {
			return SSL_WOULD_BLOCK;
		} else {
			return SSL_ERROR;
		}
	}
#endif
	return (lRecvSize);
}


/*****************************************************************************************
* Function Name	:	SSLRand
* Parameters	:	UINT8 *pRandomBytes - (OUT) The random output
					UINT32 ulNoOfBytes - (IN) The required randmom byte size
* Return Value	:	INT32 - SSL_SUCCESS(1) on success
						  - SSL_FAILURE(0) on error
* Description	:	This generates required number of random bytes
******************************************************************************************/
INT32 SSLRand(UINT8 *pRandomBytes, UINT32 ulNoOfBytes)
{
	INT32 lCount;
	INT32 lRandom;
	INT32 lNoOfTimes;
	INT32 lTime;
	static INT32 lStatic=0;

	/* initialise the random seeding with the static variable count and 
	current time ticks */
	lStatic++;

	/* get the time ticks */
#if SSL_CFG_WINDOWS
	lTime = time(NULL);
#endif

#if SSL_CFG_NUCLEUS
	lTime = OS_ADPT_Glue_Now();
#endif

	srand(lStatic+lTime);

	/* calculate the no of iteration needed to produce no. of random bytes required */
	lNoOfTimes = ulNoOfBytes/sizeof(INT32);
	lCount = ulNoOfBytes%sizeof(INT32);
	if(lCount != 0) {
		lNoOfTimes++;
	}

	for(lCount=0;lCount<lNoOfTimes;lCount++)
	{
		/* get the random number */
		lRandom = rand();

		/* copy the random output to the result buffer */
		if ( (lCount+1) * sizeof(INT32) <= ulNoOfBytes) {
			memcpy(&pRandomBytes[lCount * sizeof(INT32)], &lRandom, sizeof(INT32));
		} else {
			memcpy(&pRandomBytes[lCount * sizeof(INT32)], &lRandom, 
				ulNoOfBytes - lCount * sizeof(INT32));
		}
	}
	return(SSL_SUCCESS);
}

/*****************************************************************************************
* Function Name	:	SSLLibraryInit
* Parameters	:	None.
* Return Value	:	None.
* Description	:	This function initializes SSL library for use by applications
******************************************************************************************/
void SSLLibraryInit(void)
{
	extern CipherSuite	 gCipherPriority;
	extern RSA *pTempRsa;
	extern	struct	rsa_st	*RsaDS;
	INT32  i32Status;
	/* cipher priority to none */
	memset(&gCipherPriority[0],0,sizeof(gCipherPriority));
	
	/* initialses crypto library */
	CryptLibraryInit();

#if SSL_CFG_NUCLEUS
	SSL_Memory = OS_ADPT_Create_Memory_Pool("SSL_MEM", SSL_MEM_POOL_SIZE, SSL_MIN_ALLOC);
	DEBUG_WEB("SSLLibraryInit: OS_ADPT_Create_Memory_Pool() fail !!!\n");
	if(SSL_Memory == NULL) {
		for(;;) {
			NU_Sleep(10);
		}
	}
	
	//BEGIN - YKS & Debu... 13/01/2004
	//Creating a queue for timer invocations
    /* Allocate Memory for VRRP Internal Queue */
    pu8SslQueueBuf = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, SSL_QUEUE_SIZE * sizeof(UINT32));
    if(pu8SslQueueBuf == NULL)
    {
		OS_ADPT_Delete_Memory_Pool(SSL_Memory);
		DEBUG_WEB("SSLLibraryInit: OS_ADPT_Allocate_Memory() fail !!!\n");
		for(;;) 
		{
			NU_Sleep(10);
		}
    }
	memset(pu8SslQueueBuf, 0, SSL_QUEUE_SIZE * sizeof(UINT32));

	/* Creating a Queue */
	i32Status = NU_Create_Queue ( &SslQueueID,					/* queue id */	   \
								  "SslIntQ",					/* name */		   \
								  pu8SslQueueBuf,				/* start address */\
								  SSL_QUEUE_SIZE,				/* queue size */   \
								  NU_FIXED_SIZE,				/* message type */ \
								  SSL_ELEMENT_SIZE,				/* message size */ \
								  NU_FIFO						/* suspend mode */ \
								 );

	if(i32Status != 0)
	{
		OS_ADPT_Deallocate_Memory(pu8SslQueueBuf);
		OS_ADPT_Delete_Memory_Pool(SSL_Memory);
		DEBUG_WEB("\n SSLLibraryInit() fail to allocate queue");
		for(;;) 
		{
			NU_Sleep(10);
		}
	}

	//Initializing the timer library
	//DlkTimerInit();
	SSL_DlkTimerInit();

	//END - YKS & Debu...
	
	CERT_EPM_Open_Data_Base();
	CertLoadFromEPM();
	//Cert_CLI_CMD_Reg();
	if(SSL_CFG_MAX_CSUITE_SUPPORTED < SSLGetNoOfCipherSuites()) {
		DEBUG_WEB("\n SSLLibraryInit() Ciphersuite init fail");
		for(;;) {
			NU_Sleep(10);
		}
	}

	SSL_EPM_Open_Data_Base();
	SSLLoadDataFromEpm();
	//SSL_CLI_CMD_Reg();
#endif

#if SSL_CFG_RSA_EXPORT_ENABLED
	 #if SSH_SERVER_INCLUDED
	 //By Shan Lin. 
	 pTempRsa = RSA_new();
#if 1 /* zenrong, temporary fix timing issue, need to find good soluation */
    while (RsaDS == NULL || !InitSSH_Done)
        NU_Sleep(20);
#endif	 
	 memcpy(pTempRsa, RsaDS, sizeof(RSA));
        RSA_generate_key_wo_Primes(pTempRsa, SSL_RSA_EXPORT_KEY_LEN*8, 3, NULL, NULL);
    #else
	pTempRsa = RSA_generate_key(SSL_RSA_EXPORT_KEY_LEN*8, 3, NULL, NULL);
    #endif
#endif
}


//BEGIN - YKS & Debu... 13/01/2004
/*******************************************************************************
* Function Name : SslWriteToQ
* Parameters	: struct SslTimerQ * - [IN] The data structure having the 
								  	        data to be written into queue
* Return Value	: None
* Description	: The funtion for writing the data to internal queue, function 
				  invoked from Timer Task
*******************************************************************************/
void	SslWriteToQ(struct SslTimerQ *psIntQData)
{
	/* Variable(s) Decleration */
	INT8	i8Status;

	/* Sending the data to the queue */
	i8Status = NU_Send_To_Queue (&SslQueueID,							\
							     (void *)&psIntQData,					\
								 (sizeof(psIntQData)/sizeof(UINT32)),	\
								 NU_NO_SUSPEND);

	if(i8Status != 0)
	{
		OS_ADPT_Deallocate_Memory(psIntQData->pu8SessionID);
		OS_ADPT_Deallocate_Memory(psIntQData);
	}
}


/*******************************************************************************
* Function Name : SslReadFromQ
* Parameters	: None
* Return Value	: None
* Description	: The funtion for reading the data from internal queue, function 
				  invoked to check whether Timer task has logged any timer expiry
				  event(s)
*******************************************************************************/
void	SslReadFromQ(void)
{
	/* Variable(s) Decleration */
    UINT32  u32Address;
	struct	SslTimerQ	*psIntQData = NULL;
	INT8	i32Status = 0;
	INT32	i32DataLen;

	while(i32Status == 0)
	{
		/* Sending the data to the queue */
		i32Status = NU_Receive_From_Queue (&SslQueueID,				\
										   (void *)&u32Address,		\
										   SSL_ELEMENT_SIZE,		\
										   (UINT32 *)&i32DataLen,	\
										   NU_NO_SUSPEND);

		psIntQData = (struct SslTimerQ *)u32Address;

		/* Checking the returned status from the queue read operation */
		if((i32Status == 0) && (i32DataLen > 0) && (psIntQData != NULL))
		{
			(psIntQData->pCallBack)(psIntQData->pu8SessionID, psIntQData->u32SessionIdLen);

			/* Freeing the memories occupied */
			OS_ADPT_Deallocate_Memory(psIntQData->pu8SessionID);
			OS_ADPT_Deallocate_Memory(psIntQData);
		}
	}
}


/*******************************************************************************
* Function Name : SslCreateTimer
* Parameters	: UINT8 *  - [IN] The session identifier
				  UINT32   - [IN] The session identifier length
				  UINT32   - [IN] Timeout period for the timer
				  void (*) - [IN] The callback function
* Return Value	: INT8 - SSL_SUCCESS (0) on success, SSL_FAILURE (-1) otherwise
* Description	: The funtion for reading the data from internal queue, function 
				  invoked to check whether Timer task has logged any timer expiry
				  event(s)
*******************************************************************************/
INT8	SslCreateTimer(UINT8	*pu8SessionId,
					   UINT32	u32SessionIdLen,
					   UINT32	u32Timeout,
					   void		(*pCallback)(UINT8 *, UINT32))
{
	/* Variable(s) Initialization */
	INT8	i8Status;
	struct	SslTimerQ	*psIntQInst;

	/* Allocating the memory */
	psIntQInst = (struct SslTimerQ *)OS_ADPT_Allocate_Memory(SSL_Memory, sizeof(struct SslTimerQ));
	if(psIntQInst == NULL)
	{
		return SSL_FAILURE;
	}
	memset(psIntQInst, 0, sizeof(struct SslTimerQ));

	psIntQInst->pu8SessionID = (UINT8 *)OS_ADPT_Allocate_Memory(SSL_Memory, u32SessionIdLen);
	if(psIntQInst->pu8SessionID == NULL)
	{
		OS_ADPT_Deallocate_Memory(psIntQInst);
		return SSL_FAILURE;
	}
	memset(psIntQInst->pu8SessionID, 0, u32SessionIdLen);

	/* Filling the timer Data structures */
	psIntQInst->pCallBack		= pCallback;
	psIntQInst->u32SessionIdLen = u32SessionIdLen;
	psIntQInst->sTimeOfCreation	= SSL_DlkTimerGetTimeOfDay();
	memcpy(psIntQInst->pu8SessionID, pu8SessionId, u32SessionIdLen);

	/* Start the Master Down Timer in the Timer Library with the Timer ID and
	   the timeout value passed as input arguments, timer unit is in Milli-second */
	i8Status = (INT8)SSL_DlkCreateNStartTimer(&(psIntQInst->u32TimerId),			\
		       							  (u32Timeout * SSL_CONVERT_MILLISEC),	\
										  SslWriteToQ,							\
										  (void *)(psIntQInst),					\
										  (UINT8)SSL_DLK_TIMER_ONE_SHOT);

	return i8Status;
}


/*******************************************************************************
* Function Name : SslTimerRemoveDs
* Parameters	: UINT8 *  - [IN] The session identifier
				  UINT32   - [IN] The session identifier length
* Return Value	: None
* Description	: The funtion for invoked after reading the data from the logged
				  SSL_Timer queue
*******************************************************************************/
void	SslTimerRemoveDs(UINT8	*pu8SessionId,
						 UINT32	u32SessionIdLen)
{
	/* Variable(s) Declaration */
	SSLSessionCache		*pvCache;

	if((pu8SessionId == NULL) || ((INT32)u32SessionIdLen <= 0))
	{
		return;
	}
	else
	{
		//Checking whether the data structure is there in the Cache or not
		pvCache = (SSLSessionCache *)HTTPRetrieveSession(pu8SessionId, u32SessionIdLen);

		if(pvCache != NULL)
		{
			//Let's rock dude... and free this data structure
			HTTPRemoveSession (pu8SessionId, u32SessionIdLen);
		}
		//If Cache is NULL...
		//Indicates that the data structure doesn't exist
		//Holiday time - nothing to be done
	}
}


//END - YKS & Debu...
