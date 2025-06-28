/*
 *	File:		Sser.h
 *
 *	Contains:	Simple Serial driver interface definitions and prototypes.
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
 * * * * Release 4.00  * * *
 *		06/19/01	pjr		rework interface
 *		02/09/01	bva		add SserPort, SserAbortConnection
 *		01/15/01	dts		revise for multiple ports
 * * * * Release 3.0 * * * *
 *		01/26/98	pjr		initial version.
 * * * * Release 2.0 * * * *
 * * * * Release 1.6 * * * *
 * * * * Release 1.5 * * * *
 * * * * Release 1.4 * * * *
 * * * * Release 1.3 * * * *
 * * * * Release 1.2 * * * *
 * * * * Release 1.1 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsError.h"

#ifndef	_SSER_
#define	_SSER_

typedef unsigned short SserLength;
typedef unsigned short SserPort;

typedef enum { 
	eSserPending,
	eSserComplete
} SserStatus;

#define	kSserNumberOfConnections	(1)


extern RpErrorCode SserInit(void);
extern RpErrorCode SserDeInit(void);
extern RpErrorCode SserSend(SserPort thePortNumber, 
								char *theSendPtr, 
								SserLength theSendLength);
extern RpErrorCode SserSendStatus(SserPort thePortNumber, 
								SserStatus *theSendStatusPtr);
extern RpErrorCode SserReceive(SserPort thePortNumber);
extern RpErrorCode SserReceiveStatus(SserPort thePortNumber, 
								SserStatus *theReceiveStatusPtr, 
								char **theReceivePtrPtr, 
								SserLength *theReceiveLengthPtr);

#endif
