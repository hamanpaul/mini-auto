/*
 *	File:		RpHttpPs.c
 *
 *	Contains:	RomPager routines for HTTP request parsing
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
 * * * * Release 4.30  * * *
 *      08/07/03    bva     improve comments for header parsing error handling
 *		08/06/03	bva		add ProcessRange
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 *		03/13/03	rhb		fix RpStoreHeader to handle empty headers as well as 
 *							overflow headers
 * * * * Release 4.20  * * *
 *		02/21/03	pjr		use separate username and password max lengths
 *		01/31/03	pjr		fix absolute URL parsing to handle https prefix
 *		12/12/02	pjr		fix ProcessAuthURI in case of a query with a comma
 *		11/26/02	rhb		catch Basic credentials that are too long
 *      10/07/02    bva     add graceful M-POST rejection
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *      07/25/02    bva     add RpGetSoapAction
 * * * * Release 4.10  * * *
 *		06/13/02	rhb/bva	allow ';' characters in URLs
 * * * * Release 4.07  * * *
 * * * * Release 4.00  * * *
 *		07/06/01	amp		Add NOTIFY support
 *		07/06/01	amp		use memcmp instead of strcmp in ProcessTimeout
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		08/31/00	bva		add parsing support for upnp
 *		05/26/00	bva		use gRpDataPtr
 *		04/24/00	rhb		convert MIME type to rpDataType in ProcessContentType
 *		03/30/00	pjr		use new pattern string names
 *		02/09/00	bva		use AsEngine.h
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.10  * * *
 *		04/17/00	pjr		move FindCookieEnd to AsParse.c
 *		04/05/00	pjr		move FindValueLength, FindValueStart to AsParse.c
 *		01/17/00	bva		rework ProcessCookie and allow spaces in HTTP cookies
 * * * * Release 3.06  * * *
 * * * * Release 3.03  * * *
 *		07/06/99	pjr		fix ProcessAuthURI to support IE 5.0 Windows
 * * * * Release 3.01  * * *
 *		04/19/99	pjr		add support for digest security "qop=auth" header
 * * * * Release 3.0 * * * *
 *		03/23/99	bva		make Transfer-Encoding and other token 
 *								comparisons case-insensitive
 *		03/15/99	pjr		discard the file path in ProcessFilename
 *		03/08/99	bva		use RpStrLenCpy
 *		02/27/99	bva		add HTTP Cookie support
 *		02/22/99	bva		fix ProcessAuthorization separation character
 *		02/08/99	bva		add RpGetHostName
 *		01/29/99	bva		add Etag support
 *		01/14/99	pjr		conditionalize whole file
 *		12/31/98	pjr		use RP_MEMCMP
 *		12/24/98	pjr		RpGetAcceptLanguage and RpGetUserAgent are only
 *							enabled for RomPagerServer
 * * * * Release 2.2 * * * *
 *		12/01/98	bva		use RP_ATOL
 *		11/30/98	bva		fix compile warnings
 *		11/12/98	rhb		fix ProcessAuthorization for bad spider
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		11/03/98	pjr		enable ProcessContentType for RomPagerPutMethod
 *		10/12/98	bva		always set fHaveRequestObject in ProcessContentLength
 *		10/05/98	pjr		move some routines to RpParse.c
 *		09/24/98	bva		add Expect header parsing
 *		09/21/98	bva		add Update header parsing for TLS
 *		09/16/98	rhb		ignore pad characters for Base64 decoding
 *		09/03/98	bva		change parsing pointer initialization
 *		08/31/98	bva		change <CR><LF> definitions
 *		07/31/98	pjr		fix compiler warning
 *		07/06/98	bva		fix warning by removing FindDateTokenEnd
 *		06/18/98	pjr		save the encoded Username/Password if Remote Host.
 *		06/17/98	pjr		enable ProcessContentType for Remote Host.
 *		06/10/98	bva		add PUT command support
 * * * * Release 2.1 * * * *
 *		04/22/98	bva		protect against bogus Authorization from MSIE 3.02
 *		03/16/98	bva		improve Accept type checking
 *		02/13/98	rhb		compile RpStringToMimeType for RomPop
 *		02/06/98	rhb		DecodeBase64Data -> RpDecodeBase64Data and add a
 *								return value
 *		02/03/98	rhb		ParseBrowserDate -> RpParseDate
 *		01/21/98	bva		kMaxContentTypeLength->kMaxMimeTypeLength
 *		01/05/98	pjr		fix ProcessAccept to store the new mime type.
 * * * * Release 2.0 * * * *
 *		12/16/97	bva		pull POST overflow check from ProcessContentLength
 *		12/08/97	bva		changes for absolute URI
 *		11/26/97	pjr		make ParseBrowserDate accept date strings with a
 *							single digit day number and/or without a day of
 *							the week name (Tues).
 *		11/24/97	bva		fix compiler warnings
 *		11/22/97	bva		rework object overflow handling
 *		11/21/97	bva		fix HTTP/1.1 TRACE support
 *		11/07/97	pjr		FindTokenStart -> RpFindTokenStart and
 *							FindTokenEnd -> RpFindTokenEnd.  rework multipart
 *							Content-Type processing, add RpStringToMimeType.
 *		11/03/97	pjr		parsing changes (use rpParsingControl)
 *		10/29/97	bva		change RpEscapeDecodeString call 
 *		09/12/97	pjr		change how ProcessExtension detects Digest Support.
 *		08/26/97	pjr		rework RpProcessMpContentType.
 *		08/22/97	bva		add absolute URI compliance
 *		08/21/97	bva		rework URL dispatching
 *		08/09/97	bva		add fHaveRequestObject flag
 *		07/30/97	pjr		FindLineEnd -> RpFindLineEnd.  eliminate some
 *							unused variables in RpProcessMpContentDisposition.
 *		07/25/97	pjr		RpParseHeader is now passed thePatternTablePtr.
 *							add routines for processing Multipart headers.
 *		07/15/97	pjr		eliminate infinite loop in ProcessAuthorization
 *							on an unrecognized token.
 *		07/14/97	bva		add OPTIONS and TRACE support
 *		07/12/97	bva		created from RpHttpRq.c
 * * * * Release 1.6 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if RomPagerServer || RomPagerBasic

static void 		RpStoreHeader(char *theHeaderPtr, 
								char *theStoragePtr,
								Unsigned16 theTokenLength);

static rpPatternActionProcedure ProcessContentLength;
static rpPatternActionProcedure ProcessGet;
static rpPatternActionProcedure ProcessHead;
static rpPatternActionProcedure ProcessHost;
static rpPatternActionProcedure ProcessIfModified;
static rpPatternActionProcedure ProcessMPost;
static rpPatternActionProcedure ProcessPost;
static rpPatternActionProcedure ProcessReferer;
#if	RomPagerMimeTypeChecking
static rpPatternActionProcedure ProcessAccept;
#endif
#if RomPagerKeepAlive || RomPagerHttpOneDotOne
static rpPatternActionProcedure ProcessConnection;
#endif
#if RomPagerHttpOneDotOne
static rpPatternActionProcedure ProcessTransferEncoding;
#endif
#if RomPagerCaptureUserAgent
static rpPatternActionProcedure ProcessAgent;
#endif
#if RomPagerCaptureLanguage
static rpPatternActionProcedure ProcessLanguage;
#endif
#if RomPagerFileUpload || RomPagerIpp || RomPagerRemoteHost || RomPagerPutMethod
static rpPatternActionProcedure ProcessContentType;
#endif
#if RomPagerPutMethod
static rpPatternActionProcedure ProcessPut;
#endif
#if RomPagerOptionsMethod
static rpPatternActionProcedure ProcessOptions;
#endif
#if RomPagerTraceMethod
static rpPatternActionProcedure ProcessTrace;
#endif
#if RomPagerSecurity
static rpPatternActionProcedure ProcessAuthorization;
#endif
#if RomPagerTLS
static rpPatternActionProcedure ProcessUpdate;
#endif
#if RpExpectHeader
static rpPatternActionProcedure ProcessExpect;
#endif
#if RpEtagHeader
static rpPatternActionProcedure ProcessIfNoneMatch;
#endif
#if RomPagerRanges
static rpPatternActionProcedure ProcessRange;
#endif
#if RomPagerSecurityDigest
static rpPatternActionProcedure ProcessAuthCnonce;
static rpPatternActionProcedure ProcessAuthNc;
static rpPatternActionProcedure ProcessAuthNonce;
static rpPatternActionProcedure ProcessAuthQop;
static rpPatternActionProcedure ProcessAuthResponse;
static rpPatternActionProcedure ProcessAuthURI;
static rpPatternActionProcedure ProcessAuthUsername;
static rpPatternActionProcedure ProcessExtension;
#endif	/* RomPagerSecurityDigest */
#if RomPagerHttpCookies
static rpPatternActionProcedure ProcessCookie;
static rpPatternActionProcedure Process_RpWebID;
#endif
#if RomPlugAdvanced || RomPagerCaptureSoapAction
static rpPatternActionProcedure ProcessSoapAction;
#endif
#if RomPlugAdvanced
static rpPatternActionProcedure ProcessSubscribe;
static rpPatternActionProcedure ProcessUnsubscribe;
static rpPatternActionProcedure ProcessCallback;
#endif
#if RomPlugControl
static rpPatternActionProcedure ProcessNotify;
static rpPatternActionProcedure ProcessNTS;
static rpPatternActionProcedure ProcessSeq;
#endif
#if RomPlugAdvanced || RomPlugControl
static rpPatternActionProcedure ProcessNT;
static rpPatternActionProcedure ProcessSID;
static rpPatternActionProcedure ProcessTimeout;
#endif
#if RomPagerSecurityDigest || RomPagerFileUpload
static void ParseNextItem(rpPatternTablePtr thePatternTablePtr,
				rpHttpRequestPtr theRequestPtr);
#endif

#if	RomPagerMimeTypeChecking
static rpDataType SetAcceptType(char *theTokenPtr);
static void StoreAcceptType(rpHttpRequestPtr theRequestPtr, 
		rpDataType	theDataType);
#endif

static void ProcessMethod(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength);


void RpParseHeader(rpHttpRequestPtr theRequestPtr,
					rpPatternTablePtr thePatternTablePtr) {
	rpParsingControlPtr	theParsingPtr;
	char *				theBeginLinePtr;
	char *				theStartOfTokenPtr;
	Unsigned16			theTokenLength;

	theParsingPtr = theRequestPtr->fParsingControlPtr;
	theBeginLinePtr = theParsingPtr->fCurrentBeginLinePtr;
	/*
		look for the HTTP header in the header table
	*/
	while (thePatternTablePtr->fPatternLength != 0) {
		if (RP_MEMCMP(theBeginLinePtr, thePatternTablePtr->fPattern,
					thePatternTablePtr->fPatternLength) == 0) {
			/*
				We found a line we need to do something with.
				Skip over the matching pattern....
			*/
			theBeginLinePtr += thePatternTablePtr->fPatternLength;
			/*
				find the next token
			*/
			theStartOfTokenPtr = RpFindTokenStart(theBeginLinePtr);
			theTokenLength = RpFindTokenEnd(theStartOfTokenPtr);

			/*
				pass the token to the action routine
			*/
			(*thePatternTablePtr->fAction)(theRequestPtr, 
					theStartOfTokenPtr, 
					theTokenLength);
			break;
		}
		else {
			thePatternTablePtr += 1;
		}
	}
	return;
}


#if RomPagerSecurityDigest || RomPagerFileUpload

static void ParseNextItem(rpPatternTablePtr thePatternTablePtr,
							rpHttpRequestPtr theRequestPtr) {
	char *				theStartOfValuePtr;
	char *				theBeginLinePtr;
	char *				theNextItemPtr;
	Unsigned16			theValueLength;
	rpParsingControlPtr	theParsingPtr;

	theParsingPtr = theRequestPtr->fParsingControlPtr;
	theBeginLinePtr = theParsingPtr->fCurrentBeginLinePtr;
	theNextItemPtr = theBeginLinePtr;

	/*
		Find the beginning of the next item/value pair, or
		the end of the line so we know how far to skip ahead
		after processing this item.

		Item/value pairs can be separated by commas (in the
		Authorization header), or semi-colons (in the
		Content-Disposition header).
	*/
	while (*theNextItemPtr != kAscii_Newline) {
		if (*theNextItemPtr == kAscii_Comma ||
				*theNextItemPtr == kAscii_SemiColon) {
			theNextItemPtr++;
			while (*theNextItemPtr == kAscii_Space) {
				theNextItemPtr++;
			}
			break;
		}
		else {
			theNextItemPtr++;
		}
	}

	while (thePatternTablePtr->fPatternLength != 0) {
		if (RP_MEMCMP(theBeginLinePtr, thePatternTablePtr->fPattern,
					thePatternTablePtr->fPatternLength) == 0) {
			/*
				We found a line we need to do something with,
				so skip what we matched.
			*/
			theBeginLinePtr += thePatternTablePtr->fPatternLength;

			/*
				Find the value.
			*/
			theStartOfValuePtr = RpFindValueStart(theBeginLinePtr);
			theValueLength = RpFindValueLength(theStartOfValuePtr);

			/*
				Pass the value to the action routine.
			*/
			(*thePatternTablePtr->fAction)(theRequestPtr,
					theStartOfValuePtr,
					theValueLength);
			break;
		}
		else {
			thePatternTablePtr += 1;
		}
	}

	theParsingPtr->fCurrentBeginLinePtr = theNextItemPtr;
	return;
}

#endif	/* RomPagerSecurityDigest || RomPagerFileUpload */


static void ProcessIfModified(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fIfModified, 0);
	if (*theRequestPtr->fIfModified == '\0') {

		/*
			the browser didn't pass us a date
		*/
		theRequestPtr->fBrowserDate = 0;
	}
	else {
		theRequestPtr->fBrowserDate =
				RpParseDate(theRequestPtr->fDataPtr,
							theRequestPtr->fIfModified);
	}
	return;
}


static void RpStoreHeader(char *theHeaderPtr, 
								char *theStoragePtr,
								Unsigned16 theTokenLength) {

	if (theTokenLength == 0) {
		theTokenLength = RpFindLineEnd(theHeaderPtr);
	}
	*(theHeaderPtr + theTokenLength) = '\0';
	RpStrLenCpyTruncate(theStoragePtr, theHeaderPtr, kMaxSaveHeaderLength);
	return;
}


static void ProcessContentLength(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {
	Signed32 	theContentLength;

	/*
		Make it a C string.
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	theContentLength = RP_ATOL(theStartOfTokenPtr);
	theRequestPtr->fPostRequestLength = theContentLength;
	if (theContentLength > 0) {
		theRequestPtr->fHaveRequestObject = True;
	}
	return;
}


#if RomPagerFileUpload || RomPagerIpp || RomPagerRemoteHost || RomPagerPutMethod
static void ProcessContentType(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	if (theTokenLength >= kMaxMimeTypeLength) {
		theRequestPtr->fHttpTransactionState = eRpUnParseable;
	}
	else {
		/*
			Make it a C string.
		*/
		RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fHttpContentType, theTokenLength);

#if RomPagerFileUpload
		/*
			If the Content-Type is multipart/form-data Form Data, we need 
			to get the boundary string.  The boundary token must be the next 
			token after the Content-Type value.
		*/
		RpConvertTokenToLowerCase(theStartOfTokenPtr, theTokenLength);
		if (RP_STRCMP(theStartOfTokenPtr, kTypeMultipartForm) == 0) {

			theStartOfTokenPtr += (theTokenLength + 1);
			theStartOfTokenPtr = RpFindTokenStart(theStartOfTokenPtr);

			/*
				Is this the boundary token?
			*/
			if (RP_MEMCMP(theStartOfTokenPtr, kBoundary,
					(sizeof(kBoundary) - 1)) == 0) {
				/*
					Found the boundary token.  Now find the beginning of the
					boundary value.
				*/
				theStartOfTokenPtr = 
						RpFindTokenDelimitedPtr(theStartOfTokenPtr, kAscii_Equal);
				theStartOfTokenPtr++;

				/*
					find the length of the boundary value and save it
					in the request structure.
				*/
				theTokenLength = RpFindLineEnd(theStartOfTokenPtr);
				theRequestPtr->fBoundaryLength = theTokenLength;

				/*
					make the boundary value a C string and save it
					in the request structure.
				*/
				*(theStartOfTokenPtr + theTokenLength) = '\0';
				RP_STRCPY(theRequestPtr->fBoundary, theStartOfTokenPtr);
			}

			/*
				If the Content Type is multipart/form-data but we didn't
				get a boundary, null out fHttpContentType so the data will
				be processed as regular object data.

				Why you ask would this ever arise?  Because some people's
				Java implementation of SendUrl are incorrect.  Now you know.
			*/
			if (*theRequestPtr->fBoundary == '\0') {
				*theRequestPtr->fHttpContentType = '\0';
			}
		}
#endif	/* RomPagerFileUpload */
		theRequestPtr->fDataType = RpStringToMimeType(theRequestPtr->fHttpContentType);
	}
	return;
}
#endif	/* RomPagerFileUpload || RomPagerIpp || RomPagerRemoteHost || RomPagerPutMethod */


static void ProcessReferer(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fRefererUrl, theTokenLength);
	return;
}


static void ProcessGet(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	theRequestPtr->fHttpCommand = eRpHttpGetCommand;
	ProcessMethod(theRequestPtr, theStartOfTokenPtr, theTokenLength);
	return;
}


static void ProcessHead(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	theRequestPtr->fHttpCommand = eRpHttpHeadCommand;
	ProcessMethod(theRequestPtr, theStartOfTokenPtr, theTokenLength);
	return;
}


static void ProcessPost(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	theRequestPtr->fHttpCommand = eRpHttpPostCommand;
	ProcessMethod(theRequestPtr, theStartOfTokenPtr, theTokenLength);
	return;
}

#if RomPagerMPostMethod
static void ProcessMPost(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	theRequestPtr->fHttpCommand = eRpHttpMPostCommand;
	ProcessMethod(theRequestPtr, theStartOfTokenPtr, theTokenLength);
	return;
}
#endif

#if RomPagerPutMethod
static void ProcessPut(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	theRequestPtr->fHttpCommand = eRpHttpPutCommand;
	ProcessMethod(theRequestPtr, theStartOfTokenPtr, theTokenLength);
	return;
}
#endif

#if RomPagerOptionsMethod
static void ProcessOptions(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	theRequestPtr->fHttpCommand = eRpHttpOptionsCommand;
	theRequestPtr->fHttpResponseState = eRpHttpOptions;
	ProcessMethod(theRequestPtr, theStartOfTokenPtr, theTokenLength);
	return;
}
#endif

#if RomPagerTraceMethod
static void ProcessTrace(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {
	Signed32 			theLength; 
	rpParsingControlPtr	theParsingPtr;
	char 				*theTraceBufferPtr;

	theRequestPtr->fHttpCommand = eRpHttpTraceCommand;
	theRequestPtr->fHttpResponseState = eRpHttpTracing;
	theParsingPtr = theRequestPtr->fParsingControlPtr;
	/*
		Set up to capture any other request buffers
	*/
	theRequestPtr->fTracing = True;
	/*
		The purpose of the TRACE method is to echo back to the
		client the request that was made to the server.  We store
		the incoming request buffers in the first outbound HTML
		buffer, set up the headers and then blast out the buffer.
		This will work as long as the length of all the request buffers
		is less than an outgoing TCP buffer.

		Capture the line that this request was made on.
	*/
	theTraceBufferPtr = theRequestPtr->fHtmlResponseBufferOne;
	theLength = theParsingPtr->fCurrentEndOfLinePtr - 
			theParsingPtr->fCurrentBeginLinePtr;
	RP_MEMCPY(theTraceBufferPtr, 
			theParsingPtr->fCurrentBeginLinePtr, theLength);
	theTraceBufferPtr += theLength;
	theRequestPtr->fTraceLength = theLength;
	/*
		Capture the rest of the request buffer.  
		Since kStcpReceiveBufferSize (512) is less 
		than kHtmlMemorySize (1450), we don't have to bother
		with length checking here.
	*/
	theLength = theParsingPtr->fIncomingBufferLength;
	RP_MEMCPY(theTraceBufferPtr, 
			theParsingPtr->fCurrentEndOfLinePtr, theLength);
	theTraceBufferPtr += theLength;
	theRequestPtr->fTraceBufferPtr = theTraceBufferPtr;
	theRequestPtr->fTraceLength += theLength;

	ProcessMethod(theRequestPtr, theStartOfTokenPtr, theTokenLength);
	return;
}
#endif

static void ProcessMethod(rpHttpRequestPtr theRequestPtr, 
		char *thePathPtr, 
		Unsigned16 theTokenLength) {
	char	*theTokenPtr;
#if RpAbsoluteUri
  	char			theAbsolutePath[kMaxLineLength];
#endif

	theTokenLength = 0;
	theTokenPtr = thePathPtr;
	while (	*theTokenPtr != ' '  &&
			*theTokenPtr != '\0' &&
			*theTokenPtr != '\x0a' &&
			*theTokenPtr != '\x0d' ) {
		theTokenPtr += 1;
		theTokenLength     += 1;
	}
	RpStoreHeader(thePathPtr, theRequestPtr->fPath, theTokenLength);

#if RomPagerHttpOneDotOne
	/*
		Find out what version the request is
	*/
	thePathPtr += theTokenLength + 8;
	if (*thePathPtr >= '1') {
		theRequestPtr->fHttpVersion = eRpHttpOneDotOne;
		/*
			HTTP 1.1 or greater connections are assumed to be persistent
		*/
		/* Start add by Scott Tsai for SSL 
		   Because HTTP 1.1 or greater connections are assumed to be persistent.
		   It will cause SSL block.
		*/
		/* By Shan Lin. to fix SSL connection speed too slow problem. 
		    It is not the perfact solution. If we use SSL, we only can prove one connection. */
		if(theRequestPtr->fDataPtr->fCurrentConnectionPtr->fLocalPort == 443)
			theRequestPtr->fPersistent = False;
		else
			theRequestPtr->fPersistent = True;
		/* End add by Scott Tsai for SSL */
#if RomPagerSecurityDigest
		/*
			Assume HTTP 1.1 requests support digest.
		*/
		theRequestPtr->fClientSupportsDigest = True;
#endif
#if RpAbsoluteUri
		/*
			Well the path given us might be an absolute URI of the form
			http://<hostname>/<absolutepath>.  In HTTP 1.1 we are never
			supposed to receive this, but you never know and in versions
			greater that 1.1 we can "definitely" expect to see this.

			So we need to see if the path is of this form, and if so
			dismember it into it's component parts.
		*/
		thePathPtr = theRequestPtr->fPath;
		theTokenLength = RpFindTokenDelimited(thePathPtr, kAscii_Colon);
		if (*(thePathPtr + theTokenLength) == '\0') {
			/*
				No colons, so this is a normal path 
				and we can just return.
			*/
			return;
		}
		if (*(thePathPtr + theTokenLength + 1) == kAscii_Slash &&
				*(thePathPtr + theTokenLength + 2) == kAscii_Slash) {
			/*
				Well, we have a "://" in the path, a sure sign of an
				absolute URI.  Sigh....

				Now we have to dismember it.

				First see if it is a http request.  You never know there
				might be somebody directing "ftp://" requests to us.

				Then, strip off the <hostname> section and store it away.

				Finally, get rid of the front absolute URI stuff so that 
				the stored path is just an absolute path.
			*/
			RpConvertTokenToLowerCase(thePathPtr, theTokenLength);
			if ((RP_MEMCMP(thePathPtr, kHttpString,
					sizeof(kHttpString) - 1) == 0)
#if RomPagerSecure
					|| (RP_MEMCMP(thePathPtr, kHttpsString,
							sizeof(kHttpsString) - 1) == 0)
#endif
					) {
				/*
					We have HTTP, so set the path pointer to the <hostname>.
				*/
				thePathPtr += theTokenLength + 3;
				theTokenLength = RpFindTokenDelimited(thePathPtr, kAscii_Slash);
				RP_MEMCPY(theRequestPtr->fHost, 
						thePathPtr, theTokenLength);
				theRequestPtr->fHaveHost = True;
				/*
					now set the path pointer to the <absolutepath>.
				*/
				thePathPtr += theTokenLength;
				RP_STRCPY(theAbsolutePath, thePathPtr);
				RP_STRCPY(theRequestPtr->fPath, theAbsolutePath);
			}
			else {
				/*
					it's not HTTP, so blow up the command, and
					let's swallow the rest of the headers.
				*/
				theRequestPtr->fHttpCommand = eRpHttpNoCommand;
			}
		}
#endif 	/* RpAbsoluteUri */
	}
	else {
		theRequestPtr->fHttpVersion = eRpHttpOneDotZero;
	}
#endif 	/* RomPagerHttpOneDotOne */
	return;
}


#if	RomPagerMimeTypeChecking
static void ProcessAccept(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {
	rpDataType	theDataType;
	char		*theTokenPtr;
	char		*theEndPtr;

	/*
		There can be multiple accept statements and multiple accept arguments
		per accept statement.

		Make the Accept line a C string.

		The reason that kHttpPatternAccept includes the colon is to deal with
		bad browsers that send in "Accept:<CR><LF>" headers.  We use less code
		in dealing with this situation by having RpFindLineEnd return a length
		of 0.
	*/
	theTokenLength = RpFindLineEnd(theStartOfTokenPtr);
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	/*
		Look for Accept tokens.
	*/
	theTokenPtr = theStartOfTokenPtr;
	theEndPtr = theStartOfTokenPtr + theTokenLength;
	while (theTokenPtr < theEndPtr) {
		theDataType = SetAcceptType(theTokenPtr);
		StoreAcceptType(theRequestPtr, theDataType);
		theTokenPtr += RpFindTokenDelimited(theTokenPtr, kAscii_Comma);
		if (theTokenPtr < theEndPtr) {
			theTokenPtr += 1;
			theTokenPtr = RpFindTokenStart(theTokenPtr);
		}
	}
	return;
}

static rpDataType SetAcceptType(char *theTokenPtr) {
	rpDataType	theDataType;
	char		*thePtr;

	theDataType = eRpDataTypeNone;
	thePtr = theTokenPtr;
	if (*thePtr == '*') {
		theDataType = eRpDataTypeAll;
	}
	else if (*thePtr == 't') {
		thePtr += 5;
		if (*thePtr == 'h') {
			theDataType = eRpDataTypeHtml;
		}
		else if (*thePtr == 'p') {
			theDataType = eRpDataTypeText;
		}
	}
	else if (*thePtr == 'i') {
		thePtr += 6;
		if (*thePtr == 'g') {
			theDataType = eRpDataTypeImageGif;
		}
		else if (*thePtr == 'p') {
			theDataType = eRpDataTypeImagePict;
		}
		else if (*thePtr == 'j') {
			theDataType = eRpDataTypeImageJpeg;
		}
		else if (*thePtr == 't') {
			theDataType = eRpDataTypeImageTiff;
		}
		else if (*thePtr == '*') {
			theDataType = eRpDataTypeAnyImage;
		}
	}
	else if (*thePtr == 'a') {
		theDataType = eRpDataTypeApplet;
	}
	return theDataType;
}

static void StoreAcceptType(rpHttpRequestPtr theRequestPtr, 
		rpDataType	theDataType) {
	rpDataType	theAcceptType;

	/*
		Store the acceptable data type and create compound
		types if necessary to support multiple accept arguments.
	*/
	theAcceptType = theRequestPtr->fAcceptType;
	if (theAcceptType == eRpDataTypeNone) {
		theAcceptType = theDataType;
	}
	else if (theAcceptType != eRpDataTypeAll) {
		/*
			We have assume we have a type that we didn't
			have before.  Set up the new accept type
			based on the old one plus this one.
		*/
		switch (theDataType) {
			/*
				For image types, if the previous type
				was an image, set the type to any image,
				otherwise set the type to send anything.
			*/
			case eRpDataTypeImageGif:
			case eRpDataTypeImagePict:
			case eRpDataTypeImageJpeg:
			case eRpDataTypeImageTiff:
			case eRpDataTypeAnyImage:
				if (theAcceptType == eRpDataTypeHtml ||
						theAcceptType == eRpDataTypeApplet) {
					theAcceptType = eRpDataTypeAll;
				}
				else {
					theAcceptType = eRpDataTypeAnyImage;
				}
				break;

			/*
				For applets and specific requests of all, 
				set the type to send anything.
			*/
			case eRpDataTypeApplet:
			case eRpDataTypeAll:
				theAcceptType = eRpDataTypeAll;
				break;

			/*
				For other types, don't bother to set compound
				types, since we always pass HTML and plain text,
				and we might actually want to stop other types.
			*/
			default:
				break;
		}
	}
	theRequestPtr->fAcceptType = theAcceptType;
	return;
}

#endif	/* RomPagerMimeTypeChecking */


#if RomPagerCaptureUserAgent
static void ProcessAgent(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fAgent, 0);
	return;
}

#if RomPagerServer
char * RpGetUserAgent(void *theTaskDataPtr) {

	return ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fAgent;
}
#endif	/* RomPagerServer */
#endif	/* RomPagerCaptureUserAgent */


#if RomPagerCaptureLanguage
static void ProcessLanguage(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fLanguage, 0);
	return;
}

#if RomPagerServer
char * RpGetAcceptLanguage(void *theTaskDataPtr) {

	return ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fLanguage;
}
#endif	/* RomPagerServer */
#endif	/* RomPagerCaptureLanguage */


static void ProcessHost(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

#if RomPagerHttpOneDotOne
#if RpAbsoluteUri
	if (!theRequestPtr->fHaveHost) {
		theRequestPtr->fHaveHost = True;
		RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fHost, 0);
	}
#else
	theRequestPtr->fHaveHost = True;
	RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fHost, 0);
#endif
#else
	RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fHost, 0);
#endif
	return;
}

#if RomPagerServer
char * RpGetHostName(void *theTaskDataPtr) {

	return ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fHost;
}
#endif	/* RomPagerServer */

#if RomPagerKeepAlive || RomPagerHttpOneDotOne
static void ProcessConnection(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
#if RomPagerHttpOneDotOne
	if (theRequestPtr->fHttpVersion == eRpHttpOneDotOne) {
		if (RP_STRCMP(theStartOfTokenPtr, kHttpClose) == 0) {
			theRequestPtr->fPersistent = False;
		}
	}
	else {
#endif
#if RomPagerKeepAlive
	if (RP_STRCMP(theStartOfTokenPtr, kHttpKeepAlive) == 0) {
		theRequestPtr->fKeepAlive = True;
		/* Start add by Scott Tsai for SSL 
		   Because HTTP 1.1 or greater connections are assumed to be persistent.
		   It will cause SSL block.
		*/
		if(theRequestPtr->fDataPtr->fCurrentConnectionPtr->fLocalPort == 443)
			theRequestPtr->fPersistent = False;
		else
			theRequestPtr->fPersistent = True;
		/* End add by Scott Tsai for SSL */
	}
	else if (RP_STRCMP(theStartOfTokenPtr, kHttpClose) == 0) {
		theRequestPtr->fKeepAlive = False;
		theRequestPtr->fPersistent = False;
	}
#endif
#if RomPagerHttpOneDotOne
	}
#endif
	return;
}

#endif /* RomPagerKeepAlive || RomPagerHttpOneDotOne */


#if RomPagerHttpOneDotOne
static void ProcessTransferEncoding(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	RpConvertTokenToLowerCase(theStartOfTokenPtr, theTokenLength);
	if (RP_STRCMP(theStartOfTokenPtr, kHttpChunked) == 0) {
		theRequestPtr->fObjectIsChunked = True;
		theRequestPtr->fHaveRequestObject = True;
	}
	else {
		/*
			We got a transfer encoding we don't understand,
			so reject with 501 Unimplemented.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpNotImplemented;

		/*
			Change the state, and make sure this error case causes
			the connection to close to swallow the rest of the headers.
		*/
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
	return;
}

#if RpExpectHeader
static void ProcessExpect(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	RpConvertTokenToLowerCase(theStartOfTokenPtr, theTokenLength);
	if (RP_STRCMP(theStartOfTokenPtr, kHttpContinue) == 0) {
		/*
			The client is looking for a 100 Continue header.
		*/
		theRequestPtr->fWantsContinue = True;
	}
	else {
		/*
			The client is looking for an expectation that we
			don't support, so send 417 Expect Failed.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpExpectFailed;

		/*
			Change the state, and make sure this error case causes
			the connection to close to swallow the rest of the headers.
		*/
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
	return;
}
#endif /* RpExpectHeader */

#endif /* RomPagerHttpOneDotOne */

#if RomPagerTLS
static void ProcessUpdate(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	if (RP_STRCMP(theStartOfTokenPtr, kHttpRequestTLS) == 0) {
		theRequestPtr->fSwitchToTLS = True;
	}
	return;
}
#endif /* RomPagerTLS */

#if RomPlugAdvanced || RomPagerCaptureSoapAction
static void ProcessSoapAction(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fSoapAction, 0);
	return;
}

char * RpGetSoapAction(void *theTaskDataPtr) {

	return ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fSoapAction;
}
#endif	/* RomPlugAdvanced || RomPagerCaptureSoapAction */

#if RomPlugAdvanced
static void ProcessSubscribe(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	theRequestPtr->fHttpCommand = eRpHttpSubscribeCommand;
	ProcessMethod(theRequestPtr, theStartOfTokenPtr, theTokenLength);
	return;
}

static void ProcessUnsubscribe(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	theRequestPtr->fHttpCommand = eRpHttpUnsubscribeCommand;
	ProcessMethod(theRequestPtr, theStartOfTokenPtr, theTokenLength);
	return;
}

static void ProcessCallback(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fCallback, 0);
	return;
}
#endif	/* RomPlugAdvanced */

#if RomPlugAdvanced || RomPlugControl
static void ProcessNT(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	if (RP_STRCMP(theStartOfTokenPtr, kHttpUpnpEvent) != 0) {
		theRequestPtr->fNtReceived = True;
	}
	else {
		/*
			We got an NT header but it isn't UPnP,
			so reject with 412 Precondition Failed.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpPreconditionFailed;

		/*
			Change the state, and make sure this error case causes
			the connection to close to swallow the rest of the headers.
		*/
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
	return;
}

static void ProcessSID(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	/*
		Bump past "uuid:" and store the subscription id
	*/
	theStartOfTokenPtr += 5;
	RpStoreHeader(theStartOfTokenPtr, theRequestPtr->fSubscriptionID, 0);
	return;
}

static void ProcessTimeout(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	/*
		Make the timeout a C string.
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	RpConvertTokenToLowerCase(theStartOfTokenPtr, theTokenLength);

	if (RP_STRCMP(theStartOfTokenPtr, kHttpInfinite) == 0) {
		theRequestPtr->fSubscriptionTimeout = -1;
	}
	else if (RP_MEMCMP(theStartOfTokenPtr, kHttpSecond, 
			sizeof(kHttpSecond) - 1) == 0) {
		theStartOfTokenPtr += sizeof(kHttpSecond) - 1;
		theRequestPtr->fSubscriptionTimeout = RP_ATOL(theStartOfTokenPtr);
	}
	else {
		/*
			We got a request that isn't UPnP,
			so reject with 412 Precondition Failed.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpPreconditionFailed;

		/*
			Change the state, and make sure this error case causes
			the connection to close to swallow the rest of the headers.
		*/
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
	return;
}
#endif

#if RomPlugControl
static void ProcessNotify(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	theRequestPtr->fHttpCommand = eRpHttpNotifyCommand;
	ProcessMethod(theRequestPtr, theStartOfTokenPtr, theTokenLength);
	return;
}

static void ProcessNTS(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	if (RP_STRCMP(theStartOfTokenPtr, kHttpUpnpPropchange) != 0) {
		theRequestPtr->fNtsReceived = True;
	}
	else {
		/*
			We got an NTS header but it isn't UPnP,
			so reject with 412 Precondition Failed.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpPreconditionFailed;

		/*
			Change the state, and make sure this error case causes
			the connection to close to swallow the rest of the headers.
		*/
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
	return;
}

static void ProcessSeq(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	*(theStartOfTokenPtr + theTokenLength) = '\0';
	theRequestPtr->fSeq = RP_ATOL(theStartOfTokenPtr);
	return;
}
#endif

#if RomPagerSecurity
static void ProcessAuthorization(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {
#if 1 //Jacob & Arthur 2004/02/05
	//char				theDecodedUserPassword[kRpMaxUserNameLength + kRpMaxPasswordLength];
	//By Shan Lin  to change the value
	char				theDecodedUserPassword[kRpMaxEncUserPwdLen];
#else
	char				theDecodedUserPassword[kMaxLineLength];
#endif


	char *				thePasswordPtr;
#if RomPagerSecurityDigest
	rpDataPtr 			theDataPtr;
	rpParsingControlPtr	theParsingPtr;
#endif	/* RomPagerSecurityDigest */

	/*
		MS IE 3.02 under Windows returns an empty Authorization header if
		the user cancels out of the Authentication dialog.  So, let's just
		pretend we never got it.
	*/
	if (theTokenLength < 5) {
		return;
	}

#if RomPagerSecurityDigest
	theDataPtr = theRequestPtr->fDataPtr;
	theParsingPtr = theRequestPtr->fParsingControlPtr;
	/*
		Skip past "Basic" or "Digest"
	*/
	thePasswordPtr = theStartOfTokenPtr;
	theStartOfTokenPtr += (theTokenLength + 1);

	/*
		Make the authorization type a C string.
	*/
	*(thePasswordPtr + theTokenLength) = '\0';
	RpConvertTokenToLowerCase(thePasswordPtr, theTokenLength);

	/*
		Find out if the authorization type is "Digest".
	*/
	if (RP_STRCMP(thePasswordPtr, kHttpPatternDigest) == 0) {

		theParsingPtr->fCurrentBeginLinePtr = theStartOfTokenPtr;

		do {
			ParseNextItem(theDataPtr->fAuthPatternTable, theRequestPtr);
		} while (*theParsingPtr->fCurrentBeginLinePtr != kAscii_Newline);

		theParsingPtr->fCurrentBeginLinePtr++;
	}
	else {
		/*
			Assume basic authorization.
		*/
#else
		/*
			Skip past "Basic" (already done above if RomPagerSecurityDigest).
		*/
		theStartOfTokenPtr += (theTokenLength + 1);
#endif	/* RomPagerSecurityDigest */

		/*
			Parse the authorization string.
		*/
		theTokenLength = RpFindLineEnd(theStartOfTokenPtr);

		if (theTokenLength > kRpMaxEncUserPwdLen) {
#if RomPagerDebug
			RP_PRINTF("ProcessAuthorization, encoded user/pwd too long\n");
#endif
			theTokenLength = 0;
		}

#if RomPagerRemoteHost
		/*
			Save the encoded Username/Password string.
		*/
		RP_MEMCPY(theRequestPtr->fEncodedUserPassword, theStartOfTokenPtr,
				theTokenLength);
		*(theRequestPtr->fEncodedUserPassword + theTokenLength) = '\0';
#endif

		(void) RpDecodeBase64Data(theStartOfTokenPtr, theTokenLength, 
				theDecodedUserPassword);

		/*
			Find the separation between the Username and Password.
		*/
		thePasswordPtr = theDecodedUserPassword + 
				RpFindTokenDelimited(theDecodedUserPassword, kAscii_Colon);

		/*
			If there was a password found, set up the pointer
			to the password and null terminate the username.
			If no password was found (an obscure case caused by
			a bad and loathsome spider), just leave the password 
			as null and the username is already terminated.
		*/
		if (*thePasswordPtr == kAscii_Colon) {
			*thePasswordPtr++ = '\0';
		}

#if RomPagerDebug
		if (RP_STRLEN(theDecodedUserPassword) >= kRpMaxUserNameLength) {
			RP_PRINTF("ProcessAuthorization, username too long\n");
		}
		if (RP_STRLEN(thePasswordPtr) >= kRpMaxPasswordLength) {
			RP_PRINTF("ProcessAuthorization, password too long\n");
		}
#endif

		/*
			Copy the Username and Password into the request.
			If either is too long, it won't be copied and the
			corresponding field in the request had already been
			initialized to a null string, so the authentication
			will fail.
		*/
		RpStrLenCpy(theRequestPtr->fUsername,
					theDecodedUserPassword,
					kRpMaxUserNameLength);
		RpStrLenCpy(theRequestPtr->fPassword,
					thePasswordPtr,
					kRpMaxPasswordLength);

#if RomPagerSecurityDigest
	}
#endif	/* RomPagerSecurityDigest */

	return;
}
#endif	/* RomPagerSecurity */


#if RomPagerSecurityDigest
static void ProcessExtension(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	/*
		Skip past "Extension"
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';

	/*
		For now we'll assume "Extension: Security" means that
		digest is supported.  NCSA Mosaic 3.0.0b4 sends
		"Extension: Security/Digest" and Microsoft Internet
		Explorer 4.0p1 Macintosh version sends
		"Extension: Security/Remote-Passphrase".
	*/
	if (RP_MEMCMP(theStartOfTokenPtr, kSecurity,
			(sizeof(kSecurity) - 1)) == 0) {
		theRequestPtr->fClientSupportsDigest = True;
	}

	return;
}


static void ProcessAuthUsername(rpHttpRequestPtr theRequestPtr,
		char *theStartOfTokenPtr,
		Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
#if RomPagerDebug
	if (RP_STRLEN(theStartOfTokenPtr) >= kRpMaxUserNameLength) {
		RP_PRINTF("ProcessAuthUsername, username too long\n");
	}
#endif
	RpStrLenCpy(theRequestPtr->fUsername,
				theStartOfTokenPtr,
				kRpMaxUserNameLength);
	return;
}


static void ProcessAuthNonce(rpHttpRequestPtr theRequestPtr,
		char *theStartOfTokenPtr,
		Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	RP_STRCPY(theRequestPtr->fNonce, theStartOfTokenPtr);
	return;
}


static void ProcessAuthURI(rpHttpRequestPtr theRequestPtr,
		char *theStartOfTokenPtr,
		Unsigned16 theTokenLength) {
	char 	*theStringPtr;

	/*
		Sometimes the URI string will have a query which
		can contain a comma, causing the length of the
		string to be calculated incorrectly.  Find the
		end of the string, skipping any commas.
	*/
	theStringPtr = theStartOfTokenPtr;
	while (*theStringPtr != kAscii_Quote &&
			*theStringPtr != kAscii_Return &&
			*theStringPtr != kAscii_Newline) {
		theStringPtr++;
	}

	/*
		make it a C string
	*/
	*theStringPtr = '\0';
	RP_STRCPY(theRequestPtr->fDigestURI, theStartOfTokenPtr);

	/*
		Compare the HTTP URI and the Digest Authorization URI header.

		When IE 4.0 Macintosh sends a query request, it appends the query
		to both URIs.

		When IE 5.0 Windows sends a query reqeust, it appends the query
		to the HTTP URI, but not the Digest Authorization URI header.

		So, when checking to see if the 2 URIs match, we only compare
		them up to the end of the Digest Authorization URI header.
	*/
	if (RP_MEMCMP(theRequestPtr->fDigestURI, theRequestPtr->fPath,
				RP_STRLEN(theRequestPtr->fDigestURI)) != 0) {
		*theRequestPtr->fDigestURI = '\0';
	}

	return;
}


static void ProcessAuthResponse(rpHttpRequestPtr theRequestPtr,
		char *theStartOfTokenPtr,
		Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	RP_STRCPY(theRequestPtr->fDigest, theStartOfTokenPtr);
	return;
}


static void ProcessAuthCnonce(rpHttpRequestPtr theRequestPtr,
		char *theStartOfTokenPtr,
		Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	RP_STRCPY(theRequestPtr->fCnonce, theStartOfTokenPtr);
	return;
}


static void ProcessAuthNc(rpHttpRequestPtr theRequestPtr,
		char *theStartOfTokenPtr,
		Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	RP_STRCPY(theRequestPtr->fNonceCount, theStartOfTokenPtr);
	return;
}


static void ProcessAuthQop(rpHttpRequestPtr theRequestPtr,
		char *theStartOfTokenPtr,
		Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	if (RP_STRCMP(theStartOfTokenPtr, kHttpAuth) == 0) {
		theRequestPtr->fQopAuth = True;
	}
	return;
}

#endif	/* RomPagerSecurityDigest */


#if RomPagerFileUpload

void RpProcessMpContentDisposition(rpHttpRequestPtr theRequestPtr,
		char *theStartOfTokenPtr, Unsigned16 theTokenLength) {
	rpDataPtr 			theDataPtr;
	rpParsingControlPtr	theParsingPtr;

	theDataPtr = theRequestPtr->fDataPtr;
	theParsingPtr = theRequestPtr->fParsingControlPtr;

	do {
		ParseNextItem(theDataPtr->fDispositionPatternTable, theRequestPtr);
	} while (*theParsingPtr->fCurrentBeginLinePtr != kAscii_Newline);

	theParsingPtr->fCurrentBeginLinePtr++;
	return;
}


void RpProcessMpContentType(rpHttpRequestPtr theRequestPtr,
							char *theStartOfTokenPtr,
							Unsigned16 theTokenLength) {
	rpDataType					theDataType;

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';

	theDataType = RpStringToMimeType(theStartOfTokenPtr);

	if (theDataType == eRpDataTypeOther) {
		RP_STRCPY(theRequestPtr->fFileInfoPtr->fOtherMimeType,
				theStartOfTokenPtr);
	}

	theRequestPtr->fFileInfoPtr->fFileType = theDataType;

	return;
}


static void ProcessName(rpHttpRequestPtr theRequestPtr,
				char *theStartOfTokenPtr,
				Unsigned16 theTokenLength) {

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	RP_STRCPY(theRequestPtr->fItemName, theStartOfTokenPtr);
	return;
}


static void ProcessFilename(rpHttpRequestPtr theRequestPtr,
				char *theStartOfTokenPtr,
				Unsigned16 theTokenLength) {
	char *			theStringPtr;
	char			theTempFilename[kMaxLineLength];

	/*
		make it a C string
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';

	/*
		use RpEscapeDecodeString to copy the filename from the incoming
		request to the request structure.  this will decode any '%xx'
		encoded characters.
	*/
	RpEscapeDecodeString(theStartOfTokenPtr, theTempFilename);
	theStringPtr = theTempFilename + RP_STRLEN(theTempFilename);

	/*
		Remove any path from the filename.
	*/
	while (theStringPtr >= theTempFilename) {
		if (*theStringPtr == kAscii_Slash ||
			*theStringPtr == kAscii_Backslash) {
			break;
		}
		theStringPtr--;
	}
	theStringPtr++;

	RP_STRCPY(theRequestPtr->fFilename, theStringPtr);

	return;
}

#endif	/* RomPagerFileUpload */


void RpInitPatternTable(rpPatternTablePtr thePatternTablePtr) {
	thePatternTablePtr->fPattern       = kHttpGet;
	thePatternTablePtr->fPatternLength = sizeof(kHttpGet) - 1;
	thePatternTablePtr->fAction        = ProcessGet;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPost;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPost) - 1;
	thePatternTablePtr->fAction        = ProcessPost;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpHead;
	thePatternTablePtr->fPatternLength = sizeof(kHttpHead) - 1;
	thePatternTablePtr->fAction        = ProcessHead;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternIfModified;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternIfModified) - 1;
	thePatternTablePtr->fAction        = ProcessIfModified;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternContentLength;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternContentLength) - 1;
	thePatternTablePtr->fAction        = ProcessContentLength;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternReferer;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternReferer) - 1;
	thePatternTablePtr->fAction        = ProcessReferer;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternHost;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternHost) - 1;
	thePatternTablePtr->fAction        = ProcessHost;

#if RomPagerSecurity
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternAuthorization;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternAuthorization) - 1;
	thePatternTablePtr->fAction        = ProcessAuthorization;
#endif

#if RomPagerMimeTypeChecking
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternAccept;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternAccept) - 1;
	thePatternTablePtr->fAction        = ProcessAccept;
#endif

#if RomPagerCaptureUserAgent
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternUserAgent;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternUserAgent) - 1;
	thePatternTablePtr->fAction        = ProcessAgent;
#endif

#if RomPagerKeepAlive || RomPagerHttpOneDotOne
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternConnection;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternConnection) - 1;
	thePatternTablePtr->fAction        = ProcessConnection;
#endif

#if RomPagerHttpOneDotOne
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternTransferEncoding;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternTransferEncoding) - 1;
	thePatternTablePtr->fAction        = ProcessTransferEncoding;
#endif

#if RomPagerCaptureLanguage
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternAcceptLanguage;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternAcceptLanguage) - 1;
	thePatternTablePtr->fAction        = ProcessLanguage;
#endif

#if RomPagerFileUpload || RomPagerIpp || RomPagerRemoteHost || RomPagerPutMethod
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternContentType;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternContentType) - 1;
	thePatternTablePtr->fAction        = ProcessContentType;
#endif

#if RomPagerSecurityDigest
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternExtension;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternExtension) - 1;
	thePatternTablePtr->fAction        = ProcessExtension;
#endif

#if RomPagerPutMethod
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPut;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPut) - 1;
	thePatternTablePtr->fAction        = ProcessPut;
#endif

#if RomPagerOptionsMethod
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpOptions;
	thePatternTablePtr->fPatternLength = sizeof(kHttpOptions) - 1;
	thePatternTablePtr->fAction        = ProcessOptions;
#endif

#if RomPagerTraceMethod
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpTrace;
	thePatternTablePtr->fPatternLength = sizeof(kHttpTrace) - 1;
	thePatternTablePtr->fAction        = ProcessTrace;
#endif

#if RomPagerTLS
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternUpdate;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternUpdate) - 1;
	thePatternTablePtr->fAction        = ProcessUpdate;
#endif

#if RpExpectHeader
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternExpect;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternExpect) - 1;
	thePatternTablePtr->fAction        = ProcessExpect;
#endif

#if RpEtagHeader
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternIfNoneMatch;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternIfNoneMatch) - 1;
	thePatternTablePtr->fAction        = ProcessIfNoneMatch;
#endif

#if RomPagerHttpCookies
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternCookie;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternCookie) - 1;
//	thePatternTablePtr->fAction        = ProcessCookie;
    thePatternTablePtr->fAction        = Process_RpWebID;
#endif

#if RomPagerMPostMethod
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpMPost;
	thePatternTablePtr->fPatternLength = sizeof(kHttpMPost) - 1;
	thePatternTablePtr->fAction        = ProcessMPost;
#endif

#if RomPagerRanges
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternRange;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternRange) - 1;
	thePatternTablePtr->fAction        = ProcessRange;
#endif

#if RomPlugAdvanced || RomPagerCaptureSoapAction
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternSoapAction;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternSoapAction) - 1;
	thePatternTablePtr->fAction        = ProcessSoapAction;
#endif

#if RomPlugAdvanced
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternCallback;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternCallback) - 1;
	thePatternTablePtr->fAction        = ProcessCallback;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpSubscribe;
	thePatternTablePtr->fPatternLength = sizeof(kHttpSubscribe) - 1;
	thePatternTablePtr->fAction        = ProcessSubscribe;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpUnsubscribe;
	thePatternTablePtr->fPatternLength = sizeof(kHttpUnsubscribe) - 1;
	thePatternTablePtr->fAction        = ProcessUnsubscribe;
#endif

#if RomPlugControl
	/*
		NTS must come before NT !
	*/
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternNTS;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternNTS) - 1;
	thePatternTablePtr->fAction        = ProcessNTS;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpNotify;
	thePatternTablePtr->fPatternLength = sizeof(kHttpNotify) - 1;
	thePatternTablePtr->fAction        = ProcessNotify;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternSeq;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternSeq) - 1;
	thePatternTablePtr->fAction        = ProcessSeq;
#endif


#if RomPlugAdvanced || RomPlugControl
	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternNT;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternNT) - 1;
	thePatternTablePtr->fAction        = ProcessNT;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternTimeout;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternTimeout) - 1;
	thePatternTablePtr->fAction        = ProcessTimeout;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternSID;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternSID) - 1;
	thePatternTablePtr->fAction        = ProcessSID;
#endif

	thePatternTablePtr += 1;
	thePatternTablePtr->fPatternLength = 0;
	return;
}


#if RomPagerSecurityDigest
void RpInitAuthPatternTable(rpPatternTablePtr thePatternTablePtr) {
	thePatternTablePtr->fPattern       = kHttpPatternUsername;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternUsername) - 1;
	thePatternTablePtr->fAction        = ProcessAuthUsername;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternNonce;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternNonce) - 1;
	thePatternTablePtr->fAction        = ProcessAuthNonce;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternUri;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternUri) - 1;
	thePatternTablePtr->fAction        = ProcessAuthURI;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternResponse;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternResponse) - 1;
	thePatternTablePtr->fAction        = ProcessAuthResponse;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternCnonce;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternCnonce) - 1;
	thePatternTablePtr->fAction        = ProcessAuthCnonce;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternNc;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternNc) - 1;
	thePatternTablePtr->fAction        = ProcessAuthNc;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternQop;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternQop) - 1;
	thePatternTablePtr->fAction        = ProcessAuthQop;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPatternLength = 0;
	return;
}
#endif	/* RomPagerSecurityDigest */


#if RomPagerFileUpload
void RpInitMpPatternTable(rpPatternTablePtr thePatternTablePtr) {
	thePatternTablePtr->fPattern       = kHttpPatternContentDisposition;
	thePatternTablePtr->fPatternLength = 
			sizeof(kHttpPatternContentDisposition) - 1;
	thePatternTablePtr->fAction        = RpProcessMpContentDisposition;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kHttpPatternContentType;
	thePatternTablePtr->fPatternLength = sizeof(kHttpPatternContentType) - 1;
	thePatternTablePtr->fAction        = RpProcessMpContentType;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPatternLength = 0;
	return;
}


void RpInitDispositionPatternTable(rpPatternTablePtr thePatternTablePtr) {
	thePatternTablePtr->fPattern       = kName;
	thePatternTablePtr->fPatternLength = sizeof(kName) - 1;
	thePatternTablePtr->fAction        = ProcessName;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPattern       = kFilename;
	thePatternTablePtr->fPatternLength = sizeof(kFilename) - 1;
	thePatternTablePtr->fAction        = ProcessFilename;

	thePatternTablePtr += 1;
	thePatternTablePtr->fPatternLength = 0;
	return;
}
#endif	/* RomPagerFileUpload */


#if RpEtagHeader
void RpBuildEtagString(char *theEtagStringPtr, Unsigned32 theTag) {
	Unsigned8	theByte;
	Unsigned8 	theLength;
	char		*theTagPtr;

	theLength = 4;
	theTagPtr = (char *) &theTag;
	while(theLength > 0) {
		theByte = *theTagPtr++;
		*theEtagStringPtr++ = NIBBLE_TO_HEX((theByte >> 4) & 0x0f);
		*theEtagStringPtr++ = NIBBLE_TO_HEX(theByte & 0x0f);
		theLength -= 1;
	}
	return;
}


static void ProcessIfNoneMatch(rpHttpRequestPtr theRequestPtr, 
		char *theStartOfTokenPtr, 
		Unsigned16 theTokenLength) {

	/*
		See if the tag matches our Rom tag.
	*/
	theStartOfTokenPtr += 1;
	if (RP_MEMCMP(theStartOfTokenPtr,
					theRequestPtr->fDataPtr->fRomEtagString, 8) == 0) {
		theRequestPtr->fClientHasObject = True;
	}
	return;
}
#endif	/* RpEtagHeader */


#if RomPagerRanges
static void ProcessRange(rpHttpRequestPtr theRequestPtr, 
			char *theStartOfTokenPtr, 
			Unsigned16 theTokenLength) {

	/*
		Null terminate the range request and go parse it.
	*/
	*(theStartOfTokenPtr + theTokenLength) = '\0';
	RpParseRange(theRequestPtr, theStartOfTokenPtr);
	return;
}
#endif	/* RomPagerRanges */

#if RomPagerHttpCookies

//Arthur
static void Process_RpWebID(rpHttpRequestPtr theRequestPtr,
                            char *theStartOfTokenPtr,
                            Unsigned16 theTokenLength) {
    char        *theCookiePtr;
    char        *theEndPtr;
    char        *theValuePtr;

    /*
        Find the end of the Cookies header.
    */
    theTokenLength = RpFindLineEnd(theStartOfTokenPtr);
    theEndPtr = theStartOfTokenPtr + theTokenLength;

    /*
        Look for Cookies.
    */
    theCookiePtr = theStartOfTokenPtr;
    while (theCookiePtr < theEndPtr) {
        if (*theCookiePtr == 'R') {
            /*
                This is one of our cookies, so set up the array.
            */
            theValuePtr = RpFindTokenDelimitedPtr(theCookiePtr, kAscii_Equal);
 // +++ _Alphanetworks_Patch_, 01/04/2005, Arthur Chow
                   if (theValuePtr!=NULL)
                   {
                                     //*theValuePtr = kAscii_Null;
                                     theValuePtr += 1;
                         if (!strncmp(theCookiePtr, "RpWebID=", 8))
                         {
                               memset(theRequestPtr->fHttpCookies[0], 0, kHttpCookieSize);
                               memcpy(theRequestPtr->fHttpCookies[0],theValuePtr, kRpWebIDLen);
                               theCookiePtr=theEndPtr;
                         }
                         //else
                         //{
                         //    theCookiePtr = theValuePtr;
                         //}
                   }
                   else
                   {
                         theCookiePtr=theEndPtr;
                   }
 // --- _Alphanetworks_Patch_, 01/04/2005, Arthur Chow             
        }
        if (theCookiePtr < theEndPtr) {
            theCookiePtr += 1;
            theCookiePtr = RpFindTokenStart(theCookiePtr);
        }
    }
    return;
}


static void ProcessCookie(rpHttpRequestPtr theRequestPtr, 
							char *theStartOfTokenPtr, 
							Unsigned16 theTokenLength) {
	char		*theCookiePtr;
	char		*theEndPtr;
	char		*theValuePtr;
	int			theIndex;

	/*
		Find the end of the Cookies header.
	*/
	theTokenLength = RpFindLineEnd(theStartOfTokenPtr);
	theEndPtr = theStartOfTokenPtr + theTokenLength;

	/*
		Look for Cookies.
	*/
	theCookiePtr = theStartOfTokenPtr;
	while (theCookiePtr < theEndPtr) {
		if (*theCookiePtr == 'C') {
			/* 
				This is one of our cookies, so set up the array.
			*/
			theCookiePtr += 1;
			theValuePtr = RpFindTokenDelimitedPtr(theCookiePtr, kAscii_Equal);
			*theValuePtr = kAscii_Null;
			theIndex = RP_ATOI(theCookiePtr);
			theValuePtr += 1;
			theTokenLength = RpFindCookieEnd(theValuePtr);
			*(theValuePtr + theTokenLength) = kAscii_Null;
			RpStrLenCpy(theRequestPtr->fHttpCookies[theIndex], 
					theValuePtr, kHttpCookieSize);
			theCookiePtr = theValuePtr + theTokenLength;
		}
		else {
			/*
				Must be another cookie, so skip over it.
			*/
			theValuePtr = RpFindTokenDelimitedPtr(theCookiePtr, kAscii_Equal);
			if (theValuePtr == (char *) 0) {
				/*
					No equal sign, so must be junk in there,
					just finish.
				*/
				theCookiePtr = theEndPtr;
			}
			else {
				theTokenLength = RpFindCookieEnd(theValuePtr);
				theCookiePtr = theValuePtr + theTokenLength;
			}
		}
		if (theCookiePtr < theEndPtr) {
			theCookiePtr += 1;
			theCookiePtr = RpFindTokenStart(theCookiePtr);
		}
	}
	return;
}
#endif	/* RomPagerHttpCookies */

#endif	/* RomPagerServer || RomPagerBasic */
