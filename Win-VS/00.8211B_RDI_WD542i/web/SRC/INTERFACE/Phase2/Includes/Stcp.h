/*
 *	File:		Stcp.h
 *
 *	Contains:	Prototypes for simple interface routines to TCP/IP
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2003 by Allegro Software Development Corporation
 *  All rights reserved.
 *
 *  This module contains confidential, unpublished, proprietary 
 *  source code of Allegro Software Development Corporation.
 *
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 *
 *  License is granted for specific uses only under separate 
 *  written license by Allegro Software Development Corporation.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 * * * * Release 4.20  * * *
 *		12/18/02	bva		bump StcpConnection size
 * * * * Release 4.12  * * *
 * * * * Release 4.00  * * *
 *		10/26/00	bva		StcpSetListeners -> StcpSetServerPortNumbers
 *		01/19/00	bva		add StcpSetListeners, kStcpListeners
 * * * * Release 3.06  * * *
 * * * * Release 3.05  * * *
 * * * * Release 3.04  * * *
 * * * * Release 3.03  * * *
 * * * * Release 3.02  * * *
 * * * * Release 3.01  * * *
 * * * * Release 3.0 * * * *
 *		02/26/99	bva		change connection setup definitions
 * * * * Release 2.2 * * * *
 * * * * Release 2.1 * * * *
 *		01/13/98	rhb		add kPop3Port
 * * * * Release 2.0 * * * *
 *		08/01/97	bva		add StcpOpenActive, StcpActiveConnectionStatus, 
 *								kSmtpPort
 * * * * Release 1.6 * * * *
 *		03/26/97	bva		move error codes to RpError.h
 *		11/25/96	bva		add kHttpPort
 * * * * Release 1.5 * * * *
 *		10/02/96	bva		rework port support
 * * * * Release 1.4 * * * *
 * * * * Release 1.3 * * * *
 *		07/03/96	bva		add StcpStatus for call completion to support
 *							TCP TIME_WAIT processing
 * * * * Release 1.2 * * * *
 * * * * Release 1.1 * * * *
 * * * * Release 1.0 * * * *
 *		12/08/95	bva		created 
 *
 *	To Do:
 */


#include "AsError.h"


#ifndef	_STCP_
#define	_STCP_

typedef unsigned short	StcpLength;
typedef unsigned long	StcpIpAddress;
typedef unsigned short	StcpPort;
typedef unsigned short	StcpConnection;

#define kStcpReceiveBufferSize		(512)	/*	size of receive buffer	*/


/*
	kStcpNumberOfConnections, kStcpActiveConnections, kStcpServerCount
	
	The total number of TCP connections that the Allegro scheduler manages
	is defined by the value kStcpNumberOfConnections.  The Web server products 
	and other server products use passive connections to wait for a browser 
	or other client to initiate a request on the connection. The RomWebClient, 
	RomPager Remote Host, RomMailer and RomPOP products use active TCP 
	connections to connect to other Web and email servers.  
	
	The number of active connections determines the number external mail or 
	Web server connections that can be handled simultaneously for RomPager 
	client software. The active connections are reserved from the total 
	connections pool using the value of kStcpActiveConnections.  Make sure 
	that the total pool is large enough to handle the expected number of 
	simultaneous server and client connections.
	
	The Allegro engine can listen for passive connections on multiple
	simultaneous port numbers. The number of passive connections available
	to the engine is (kStcpNumberOfConnections - kStcpActiveConnections). 
	The passive connections can all be HTTP servers on one port number, can
	be all HTTP servers on multiple port numbers or can be servers of 
	different protocols each with their own port number. The kStcpServerCount 
	value sets the maximum number of different server port numbers for the 
	engine to listen to. In socket implementations, this number is the maximum
	number of simultaneous master listener sockets or incoming port numbers 
	that the TCP interface will be expected maintain.
*/

// +++ _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#if defined(WIN32)
#define L3_WEB_MAX_CONNECT_NUMBER	5
#else   /*Elsa modify for web*/
#define L3_WEB_MAX_CONNECT_NUMBER	4
#endif
// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih

#define kStcpNumberOfConnections	(L3_WEB_MAX_CONNECT_NUMBER * 2)
#define kStcpActiveConnections		(L3_WEB_MAX_CONNECT_NUMBER)
#define kStcpServerCount			(5)

#define kPop3Port	110
#define kSmtpPort	25
#define kHttpPort	80

/* 
	Simple Tcp call completion states 
*/

typedef enum { 
	eStcpPending,
	eStcpComplete, 		
	eStcpOpenPassiveTimeout,
	eStcpTimeWait
} StcpStatus;


extern RpErrorCode StcpAbortConnection(StcpConnection theConnection);
extern RpErrorCode StcpActiveConnectionStatus(StcpConnection theConnection,
										StcpStatus *theCompletionStatusPtr,
										StcpIpAddress *theLocalAddressPtr); 
extern RpErrorCode StcpDeInit(void);
extern RpErrorCode StcpCloseConnection(StcpConnection theConnection);
extern RpErrorCode StcpCloseStatus(StcpConnection theConnection, 
										StcpStatus *theCompletionStatusPtr);
extern RpErrorCode StcpConnectionStatus(StcpConnection theConnection, 
										StcpStatus *theCompletionStatusPtr, 
										StcpIpAddress *theRemoteAddressPtr, 
										StcpIpAddress *theLocalAddressPtr, 
										StcpPort *theLocalPortPtr, int fd); //Arthur
extern RpErrorCode StcpInit(void);
extern RpErrorCode StcpOpenActive(StcpConnection theConnection, 
										StcpIpAddress theRemoteAddress, 
										StcpPort theRemotePort);
extern RpErrorCode StcpOpenPassive(StcpConnection theConnection);
extern RpErrorCode StcpReceive(StcpConnection theConnection);
extern RpErrorCode StcpReceiveStatus(StcpConnection theConnection, 
										StcpStatus *theCompletionStatusPtr, 
										char **theReceivePtrPtr, 
										StcpLength *theReceiveLengthPtr);
extern RpErrorCode StcpSend(StcpConnection theConnection, 
										char *theSendPtr, 
										StcpLength theSendLength);
extern RpErrorCode StcpSendStatus(StcpConnection theConnection, 
										StcpStatus *theCompletionStatusPtr);
extern RpErrorCode StcpSetServerPortNumbers(int theServerPortCount, 
										StcpPort *theServerPortArrayPtr);
#endif
