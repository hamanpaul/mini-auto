/*
 *	File:		RpIpp.h
 *
 *	Contains:	Definitions for HTTP to IPP services
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
 *		10/06/00	pjr		move printer name and job ident strings into the
 *							ippRequest, add fields for printer uri, security,
 *							and authentication
 *		06/29/00	pjr		add fPrinterNamePtr to ippRequest structure
 *		06/27/00	rhb		add support for multiple IPP response buffers
 * * * * Release 3.0 * * * *
 * * * * Release 2.2 * * * *
 *		08/09/98	bva		make fResponseLength Unsigned16
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 *		12/04/97	bva		updated documentation
 *		11/03/97	rhb		add modifications for ippRequest structure
 * * * * Release 1.0 * * * *
 *
 *
 *	To Do:
 */

#ifndef	_RPIPP_
#define	_RPIPP_

#include "AsError.h"


/*
	HTTP to IPP Operating System model
	
	From the point of view of the host operating system, the RomPager engine 
	is a single task. It contains its own scheduler and control blocks for 
	supporting multiple HTTP requests. The calls it makes to the IPP 
	Parser processing world must be considered asynchronous for any activity 
	that will incur delay. The calls have status completion codes to determine 
	whether an IPP activity has been completed. Since printing engines have 
	latencies, any operation that can incur delay needs to interface 
	asynchronously with the printing engine, or create an operating system task 
	that can block on call completion. In this way, the RomPager engine can 
	continue to service other simultaneous HTTP requests.

	HTTP to IPP processing flow
	
	The RomPager engine will issue the IppInit call when it is started to 
	initialize the IPP parser.  The parser should allocate any private memory
	it needs and return a pointer.  The IppDeInit call will be issued when
	the RomPager engine shuts down.  Buffers are passed from HTTP to IPP using
	the ippRequest structure which contains pointers to the IPP request as well as
	pointers which the parser can use to indicate the IPP response.  

	The IppStartParser call will be sent to the parser with a pointer to the
	ippRequest structure.  The parser will provide a response of 
	eIppHttpResponse_Pending if it needs more time with the request buffer or
	eIppHttpResponse_NeedNextBuffer if it is ready for the next request buffer.
	If the parser has sent the eIppHttpResponse_Pending response, the HTTP engine
	will periodically call the parser with the IppCheckParser call until it
	receives a eIppHttpResponse_NeedNextBuffer response. Request buffers will be 
	collected and sent to the parser until there are no more, at which point the 
	fLastBufferFlag field will be set to True in the ippRequest structure.  
	
	When the parser has completed all activities on the IPP request, it will 
	prepare an IPP response buffer and return a status of eIppHttpResponse_TransmitOK.  
	If the operation requires an abort to avoid reading in a large series of 
	request buffers, the eIppHttpResponse_TransmitAbort should be returned. If
	the IPP parser needs to send back multiple buffers to prepare the response,
	the fLastBufferFlag field should be set to False for all the response buffers except
	for the last one which should have the fLastBufferFlag field set to True.
	
*/


/*
 	IPP Responses.
*/

typedef enum {							
	eIppHttpResponse_Pending,			/* still processing buffer			*/
	eIppHttpResponse_NeedNextBuffer,	/* done w/buffer but op. incomplete	*/
	eIppHttpResponse_TransmitOK,		/* transmit response packet--no err.*/
	eIppHttpResponse_TransmitAbort		/* transmit response packet--error	*/
} ippHttpResponse;


typedef enum {
	eIppAuth_None,
	eIppAuth_Basic,
	eIppAuth_Digest
} ippAuth;


typedef enum {
	eIppSecurity_None,
	eIppSecurity_Ssl3
} ippSecurity;


/*
	These maximum lengths include the actual string length from
	the IPP Model and Semantics document plus 1 for the string
	terminator.
*/

#define kMaxIppPrinterNameLength	128
#define kMaxIppNameLength			256
#define kMaxIppUriLength			1024


/*
	IPP Request Structure.
*/

typedef struct {
	void *		fIppDataPtr;			/* IPP parser private data pointer	*/
	Unsigned8	fConnectionId;			/* TCP connection number			*/
  	char		fJobIdent[kMaxIppUriLength];			/* IPP Job ID		*/
  	char		fPrinterName[kMaxIppPrinterNameLength];	/* IPP printer name */
  	char		fPrinterUri[kMaxIppUriLength];			/* IPP printer uri	*/
	ippAuth		fAuthentication;		/* IPP authentication enum			*/
	ippSecurity	fSecurity;				/* IPP security enum				*/
	char *		fRequestPtr;			/* IPP request						*/
	Signed32	fRequestLength;		
	Boolean		fLastBufferFlag;		/* True if last buffer of request	*/
	char *		fResponsePtr;			/* IPP response						*/
	Unsigned16	fResponseLength;		/* Size of buffer-then size of data	*/
} ippRequest, *ippRequestPtr;


/*
	Calls from HTTP to IPP parser.
*/

extern void *				IppInit(void *theTaskDataPtr);
extern void					IppDeInit(void *theIppDataPtr);
extern ippHttpResponse		IppStartParser(ippRequestPtr theIppRequestPtr);
extern ippHttpResponse		IppCheckParser(ippRequestPtr theIppRequestPtr);
extern RpErrorCode			IppTimer(void *theIppDataPtr,
								Unsigned32 theCurrentTime);

#endif	/* _RPIPP_ */
