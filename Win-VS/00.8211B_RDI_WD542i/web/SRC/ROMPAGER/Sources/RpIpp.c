/*
 *	File:		RpIpp.c
 *
 *	Contains:	RomPager routines for IPP request handling
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
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		10/06/00	pjr		give each printer name a realm access code
 *							build the printer name and job ident into the IPP
 *							request block instead of the HTTP request block
 *							add printer uri, authentication, and security
 *							fields to the IPP request block
 *		06/29/00	pjr		add support for multiple IPP printer names
 *		06/27/00	rhb		add support for multiple IPP response buffers
 *		06/02/00	bva		use gRpDataPtr
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.10  * * *
 *		06/05/00	bva		use RpStrLenCpy in RpSetIppPrinterName
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 *		03/16/99	rhb		clear fIppRequest.fResponsePtr at request init
 *		12/31/98	pjr		use RP_MEMCMP
 * * * * Release 2.2 * * * *
 *		11/12/98	rhb		give a type to fDataPtr in the request structure
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		10/20/98	bva		move state initialization to RpSetupIppRequest
 *		10/12/98	bva		add request length checking to RpSetupIppRequest
 *							allow HTTP 1.0 only environment
 *		09/11/98	bva		fix eIppHttpResponse_Pending in
 *							HandleIppParserResponse
 *		07/27/98	pjr		fix compiler warning
 * * * * Release 2.1 * * * *
 *		04/28/98	bva		add dynamic printer name support
 *		03/04/98	rhb		initialize theReadMoreFlag in SetupIppBuffer
 * * * * Release 2.0 * * * *
 *		12/11/97	bva		make response object dynamic and IPP type
 *		11/07/97	pjr		use rpParsingControl structure
 *		11/03/97	rhb		modifications for ippRequest structure
 *		09/23/97	bva		add connection id to parser calls
 *		08/30/97	bva		spun off from RpHttpRq.c
 * * * * Release 1.6 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"

#if RomPagerIpp

static Boolean 		CheckIppName(rpHttpRequestPtr theRequestPtr);
static void 		RejectIppRequest(rpHttpRequestPtr theRequestPtr);
static Boolean 		SetupIppBuffer(rpHttpRequestPtr theRequestPtr);


void RpSetupIppRequest(rpHttpRequestPtr theRequestPtr) {
	rpObjectDescriptionPtr	theObjectPtr;

	/*
		If the URL for this request matches any of the IPP printer names,
		this is a print operation, so we'll take it.  Otherwise, we flush
		the data and send back '403 Forbidden'.
		
		If we have a print operation, we then check security.
	*/
	if (CheckIppName(theRequestPtr)) {
		/*
			We have a valid IPP request, so set up the HTTP object.
		*/
		theObjectPtr = &theRequestPtr->fLocalObject;
		theObjectPtr->fMimeDataType = eRpDataTypeIpp;
		theObjectPtr->fCacheObjectType = eRpObjectTypeDynamic;
		theRequestPtr->fObjectPtr = theObjectPtr;
		theRequestPtr->fObjectSource = eRpIppUrl;
		theRequestPtr->fHttpTransactionState = eRpEndUrlSearch;
		theRequestPtr->fHttpIppState = eRpHttpIppSetupRequestBuffer;
		/*
			We provide buffers to the IPP parser for its response.
			The buffers are actually the HTML response buffers.

			For IPP, a response buffer must be provided to the IPP parser
			with the first request packet, but the response headers will
			not be sent until the parser is done parsing the request.

			In order to provide the parser with the response buffer now,
			we must call RpInitializeHtmlPageReply to initialize the
			response buffers and states.

			In order for RpInitializeHtmlPageReply to work properly, we
			must know now whether or not the response will be chunked.

			Since we don't know the Content-Length of an IPP response,
			the response will be chunked if it is HTTP 1.1.
		*/
#if RomPagerHttpOneDotOne
		if (theRequestPtr->fHttpVersion == eRpHttpOneDotOne) {
			theRequestPtr->fResponseIsChunked = True;
		}
#endif
		RpInitializeHtmlPageReply(theRequestPtr);
		theRequestPtr->fIppRequest.fResponsePtr =
				theRequestPtr->fHtmlFillPtr;
		theRequestPtr->fIppRequest.fResponseLength =
				theRequestPtr->fFillBufferAvailable;
	}
	else {
		RejectIppRequest(theRequestPtr);
		return;
	}

	/*
		Check for valid request length.
	*/
	if (!theRequestPtr->fHaveRequestObject) {
		RejectIppRequest(theRequestPtr);
	}
	
	return;
}


static void RejectIppRequest(rpHttpRequestPtr theRequestPtr) {
	/*
		Mark the connection to close after we send the response.
	*/
	theRequestPtr->fPersistent = False;
	theRequestPtr->fHttpResponseState = eRpHttpForbidden;
	/*
		Change this to real error page.
	*/
	theRequestPtr->fObjectPtr = &gRpHttpNoObjectFoundPage;
	theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	return;
}


Boolean RpHandleIpp(rpHttpRequestPtr theRequestPtr) {
	rpConnectionPtr 		theConnectionPtr;
	rpDataPtr				theDataPtr;
	ippRequestPtr			theIppRequestPtr;
	ippHttpResponse			theParserResponse;
	Boolean					theReadMoreFlag;
	RpErrorCode				theResult;

	theDataPtr = theRequestPtr->fDataPtr;
	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	theIppRequestPtr = &theRequestPtr->fIppRequest;
	theReadMoreFlag = False;

	switch (theRequestPtr->fHttpIppState) {

		case eRpHttpIppSetupRequestBuffer:
			theReadMoreFlag = SetupIppBuffer(theRequestPtr);
			break;

		case eRpHttpIppHaveRequestBuffer:
			theIppRequestPtr->fIppDataPtr = theDataPtr->fIppDataPtr;
			theIppRequestPtr->fConnectionId = theConnectionPtr->fIndex;
			theIppRequestPtr->fLastBufferFlag = 
				theRequestPtr->fIncomingBufferEnd;
			theParserResponse = IppStartParser(theIppRequestPtr);
			HandleIppParserResponse(theConnectionPtr, theParserResponse);
			break;

		case eRpHttpIppParserPending:
			theParserResponse = IppCheckParser(theIppRequestPtr);
			HandleIppParserResponse(theConnectionPtr, theParserResponse);
			break;

		case eRpHttpIppSendingHeaders:
			/*
				Send the HTTP headers.
			*/
			theRequestPtr->fHttpResponseState = eRpHttpIppNormal;
			theRequestPtr->fHttpIppState = eRpHttpIppSendingResponse;
			theResult = RpStartHttpResponse(theConnectionPtr);
			break;

		case eRpHttpIppSendingResponse:
			/*
				First we need to send out HTTP headers,
				then we need to send the IPP response.
			*/
			if (theIppRequestPtr->fLastBufferFlag) {
				theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
			}
			else {
				theRequestPtr->fHttpIppState = eRpHttpIppGettingResponse;
			}
			theRequestPtr->fFillBufferAvailable -=
					theIppRequestPtr->fResponseLength;
			theRequestPtr->fHtmlBufferReady = False;
			RpFlipResponseBuffers(theRequestPtr);
			theResult = RpSendReplyBuffer(theConnectionPtr);
			break;

		case eRpHttpIppGettingResponse:
			/*
				Get the next response buffer.
			*/
			theIppRequestPtr->fResponsePtr = theRequestPtr->fHtmlFillPtr;
			theIppRequestPtr->fResponseLength =
					theRequestPtr->fFillBufferAvailable;
			theIppRequestPtr->fLastBufferFlag = True;
			theParserResponse = IppStartParser(theIppRequestPtr);
			HandleIppParserResponse(theConnectionPtr, theParserResponse);
			break;
	}

	return theReadMoreFlag;
}


void HandleIppParserResponse(rpConnectionPtr theConnectionPtr, 
								ippHttpResponse theParserResponse) {
	rpHttpRequestPtr 	theRequestPtr;

	theRequestPtr = theConnectionPtr->fHttpRequestPtr;

	/*
		Handle the IPP parser response.
	*/
	switch (theParserResponse) {
		case eIppHttpResponse_Pending:
			theConnectionPtr->fState = eRpConnectionProtocolWaitExternal;
			theRequestPtr->fHttpIppState = eRpHttpIppParserPending;
			break;

		case eIppHttpResponse_NeedNextBuffer:
			theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
			theRequestPtr->fHttpIppState = eRpHttpIppSetupRequestBuffer;
			break;

		case eIppHttpResponse_TransmitAbort:
			/*
				Mark the connection to close after we send the response.
			*/
			theRequestPtr->fPersistent = False;
			/*
				Fall through to handle sending the response.
			*/

		case eIppHttpResponse_TransmitOK:
			theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
			if (theRequestPtr->fIppResponseStarted) {
				theRequestPtr->fHttpIppState = eRpHttpIppSendingResponse;
			}
			else {
				/*
					Make sure we don't send any more request buffers
					to the IPP parser.
				*/
				theRequestPtr->fIppRequest.fRequestPtr = (char *) 0;
				theRequestPtr->fIppRequest.fRequestLength = 0;
				theRequestPtr->fIppResponseStarted = True;
				theRequestPtr->fHttpIppState = eRpHttpIppSendingHeaders;
			}
			break;
	}
	return;
}


/*
	See if the URL matches any of our IPP printer names.
*/

static Boolean CheckIppName(rpHttpRequestPtr theRequestPtr) {
	rpDataPtr		theDataPtr;
	Unsigned8		theIndex;
	ippRequestPtr	theIppRequestPtr;
	char *			thePathPtr;
	char *			thePrinterNamePtr;
	Unsigned16		thePrinterNameLength;
#if RomPagerSecurity
	rpAccess		theAccessCode;
	rpRealmPtr 		theRealmPtr;
	rpSecurityLevel	theSecurityLevel;
#endif
	
	/*
		This routine looks for URLs of the form "/xx/yyy" where "xx"
		is the printer name and "yyy" is an optional job identification 
		to be stored away.  
		
		If "xx" matches any of our printer names, then we accept the job
		and the printer name is stored in the fIppPrinterName field of
		the IPP request block and "yyy" (an arbitrary string) is stored
		in the fIppJobIdent field of the IPP request block and the
		routine returns True.  Otherwise, the routine returns False.
	*/
	theDataPtr = theRequestPtr->fDataPtr;
	thePathPtr = theRequestPtr->fPath;
	thePathPtr++;

	/*
		Find the length of the printer name field.
	*/
	thePrinterNameLength = RpFindTokenDelimited(thePathPtr, kAscii_Slash);

	for (theIndex = 0; theIndex < kIppNumberOfPrinters; theIndex++) {

		thePrinterNamePtr = theDataPtr->fIppPrinterName[theIndex];

		if (thePrinterNameLength == RP_STRLEN(thePrinterNamePtr)) {
			/*
				This printer name is the same length as
				the printer name from the request.
			*/
			if (RP_MEMCMP(thePathPtr, thePrinterNamePtr,
							thePrinterNameLength) == 0) {
				/*
					The "xx" matches the printer name, so copy it, then
					skip over the name and the "/" and copy the rest
					into the Job Id.
				*/
				theIppRequestPtr = &theRequestPtr->fIppRequest;
				RP_STRCPY(theIppRequestPtr->fPrinterName, thePrinterNamePtr);
				thePathPtr += thePrinterNameLength + 1;
				RpStrLenCpyTruncate(theIppRequestPtr->fJobIdent,
									thePathPtr, kMaxIppUriLength);

				/*
					Build the PrinterUri.
				*/
				RP_STRCPY(theIppRequestPtr->fPrinterUri, kHttpString);
				RpBuildHostName(theRequestPtr, theIppRequestPtr->fPrinterUri);
				RP_STRCAT(theIppRequestPtr->fPrinterUri, kForwardSlash);
				RP_STRCAT(theIppRequestPtr->fPrinterUri,
							theIppRequestPtr->fPrinterName);

#if RomPagerSecurity
				/*
					Get the realm access code for this printer.
				*/
				theAccessCode = theDataPtr->fIppAccess[theIndex];
				theRequestPtr->fLocalObject.fObjectAccess = theAccessCode;

				/*
					Determine the security levels for this printer.
				*/
				theSecurityLevel = eRpSecurity_Disabled;

				if (theAccessCode != kRpPageAccess_Unprotected) {
					theRealmPtr = RpGetLeastSecureRealm(theDataPtr,
														theAccessCode);

					if (theRealmPtr != (rpRealmPtr) 0) {
						theSecurityLevel = theRealmPtr->fSecurityLevel;
					}
				}

#if RomPagerSecurityDigest
				if (theSecurityLevel & kRpSecurityDigest) {
					theIppRequestPtr->fAuthentication = eIppAuth_Digest;
				}
				else if (theSecurityLevel & kRpSecurityBasic){
					theIppRequestPtr->fAuthentication = eIppAuth_Basic;
				}
				else {
					theIppRequestPtr->fAuthentication = eIppAuth_None;
				}
#else	/* !RomPagerSecurityDigest */
				if (theSecurityLevel & kRpSecurityBasic){
					theIppRequestPtr->fAuthentication = eIppAuth_Basic;
				}
				else {
					theIppRequestPtr->fAuthentication = eIppAuth_None;
				}
#endif	/* RomPagerSecurityDigest */
#if RomPagerSecure
				if (theSecurityLevel & kRpSecuritySSL) {
					theIppRequestPtr->fSecurity = eIppSecurity_Ssl3;
				}
				else {
					theIppRequestPtr->fSecurity = eIppSecurity_None;
				}
#else	/* !RomPagerSecure */
				theIppRequestPtr->fSecurity = eIppSecurity_None;
#endif	/* RomPagerSecure */

#else	/* !RomPagerSecurity */
				theIppRequestPtr->fAuthentication = eIppAuth_None;
				theIppRequestPtr->fSecurity = eIppSecurity_None;
#endif	/* RomPagerSecurity */

				return True;
			}
		}
	}

	/*
		We didn't match any of the IPP printer names.
	*/
	return False;
}


static Boolean SetupIppBuffer(rpHttpRequestPtr theRequestPtr) {
	Signed32			theBufferLength;
	rpParsingControlPtr	theParsingPtr;
	Boolean				theReadMoreFlag;

	theParsingPtr = theRequestPtr->fParsingControlPtr;
	theReadMoreFlag = False;

	if (theParsingPtr->fIncomingBufferLength == 0) {
		/*
			If there is no more data in the TCP buffer, signal the higher
			layer that we want some.
		*/
		return True;
	}

	/*
		The incoming buffer may be chunked.  If it is, we will pass a complete
		chunk if it is contained in the TCP buffer and multiple pieces of the
		chunk if it overlaps TCP buffers.  If it isn't, we'll pass the full
		TCP buffer to the IPP parser.  Depending on how the client sends the
		data, this may have interesting performance implications.
	*/
	if (theRequestPtr->fPostRequestLength > 0) {
		/*
			We were passed in a Content-Length, so we aren't chunked.
		*/
		theBufferLength = theParsingPtr->fIncomingBufferLength;
		theParsingPtr->fIncomingBufferLength = 0;
		theRequestPtr->fIppRequest.fRequestPtr =
				theParsingPtr->fCurrentBufferPtr;
		theRequestPtr->fIppRequest.fRequestLength = theBufferLength;
		theRequestPtr->fIppRequestLength += theBufferLength;
		theRequestPtr->fHttpIppState = eRpHttpIppHaveRequestBuffer;
		/*
			Is this the last buffer?
		*/
		if (theRequestPtr->fIppRequestLength >=
				theRequestPtr->fPostRequestLength) {
			theRequestPtr->fIncomingBufferEnd = True;
		}
	}
#if RomPagerHttpOneDotOne
	else {
		/*
			We must be chunked or we wouldn't have gotten here, so get a chunk.
		*/
		theReadMoreFlag = RpGetChunkedData(theRequestPtr);
		if (theRequestPtr->fHaveChunk) {
			theRequestPtr->fHttpIppState = eRpHttpIppHaveRequestBuffer;
			theRequestPtr->fIppRequest.fRequestPtr =
					theRequestPtr->fCurrentChunkBufferPtr;
			theRequestPtr->fIppRequest.fRequestLength =
					theRequestPtr->fCurrentChunkBufferLength;
		}
		/*
			If we have finished with all the chunked data, we need to move the
			input buffer pointer past the optional chunked footer and the
			trailing empty line.
		*/
		while (theRequestPtr->fChunkState == eRpGettingChunkedEnd &&
				!theReadMoreFlag) {
			theReadMoreFlag = RpGetChunkedData(theRequestPtr);
		}
		/*
			Is this the last buffer?
		*/
		if (theRequestPtr->fChunkState == eRpEndChunked) {
			theRequestPtr->fHttpIppState = eRpHttpIppHaveRequestBuffer;
			theRequestPtr->fIncomingBufferEnd = True;
			theRequestPtr->fIppRequest.fRequestLength = 0;
		}
	}
#endif
	return theReadMoreFlag;
}


/*
	RpSetIppPrinterName - Set up an IPP printer name and access code

	This routine is used to set up an IPP printer name and access
	code.  The access code field contains the same security realm
	flags (such as kRpPageAccess_Realm1) that are stored in the
	fObjectAccess field of a page or form object.

	Inputs:
		theTaskDataPtr				- pointer to the tasks data structure
		theIndex					- 0-based printer index
		theAccessCode				- realm access code for the printer
		thePrinterName				- pointer to the printer name string

	Returns:
		none
*/

void RpSetIppPrinterName(void *theTaskDataPtr, Unsigned8 theIndex,
							rpAccess theAccessCode, char *thePrinterName) {

	if (theIndex < kIppNumberOfPrinters) {
		((rpDataPtr) theTaskDataPtr)->fIppAccess[theIndex] = theAccessCode;
		RpStrLenCpyTruncate(((rpDataPtr) theTaskDataPtr)->
				fIppPrinterName[theIndex], thePrinterName, kMaxNameLength);

	}
	return;
}

#endif	/* RomPagerIpp */
