/*
 *	File:		RpHttpRq.c
 *
 *	Contains:	RomPager routines for HTTP request handling
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
 *      06/20/03    bva     add eRpHandleXmlRequest
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 * * * * Release 4.20  * * *
 *		02/05/03	bva		add RomPagerServer conditional to RomPagerSecurity
 *      10/07/02    bva     add graceful M-POST rejection
 *		10/04/02	amp		call GENA functions only if CpEventSupport
 *      10/04/02    amp     update for new RomPagerSecure
 * * * * Release 4.12  * * *
 * * * * Release 4.10  * * *
 *		06/08/02	bva		remove RomPagerSlaveIdentity
 *		04/25/02	rhb		always allow CGI to recall application
 * * * * Release 4.07  * * *
 * * * * Release 4.03  * * *
 *		10/19/01	pjr		set up the local object for the trace response
 *		10/16/01	rhb		update SSL/TLS debug code
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		07/09/01	amp		change SetUpObjectHandling for RomUpnpControl
 *		05/14/01	pjr		fix sending of HTTP "100 Continue" header
 *		01/26/01	pjr		fix chunked length spaces
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		10/30/00	pjr		ignore spaces in a chunked length
 *		09/02/00	bva		add UPNP subscription support
 *		07/25/00	rhb		Support SSL/TLS
 *		05/26/00	bva		use gRpDataPtr
 *		04/24/00	rhb		test against fDataType not fHttpContentType in 
 *								RpEndHeaders() and SetUpObjectHandling()
 *		02/09/00	bva		use AsEngine.h
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.10  * * *
 *		05/16/00	rhb		fix file upload for no security case
 * * * * Release 3.06  * * *
 * * * * Release 3.05  * * *
 *		09/21/99	rhb		move URL State checking from RpFindHttpObject()
 * * * * Release 3.04  * * *
 * * * * Release 3.0 * * * *
 *		01/23/99	pjr		add RpStartHttpResponse (moved from AsMain.c)
 *		01/20/99	pjr		change RpHexToNibble usage
 *		01/14/99	pjr		change error page handling for RomPagerBasic
 *		12/31/98	pjr		use RP_MEMCMP
 *		12/28/98	pjr		move HandleServerPushWait to RpHttpDy.c
 *		12/11/98	rhb		support Soft Pages
 *		12/03/98	bva		fix extra <CR><LF> in Keep-Alive POSTs
 *		12/02/98	pjr		move RpIdentifyObjectSource to RpUrl.c,
 *								move routines to RpQuery.c:
 *								RpCheckQuery, RpStoreQueryValues, 
 *								RpPushQueryIndex, RpPopQueryIndex, 
 *								RpGetQueryIndexLevel, RpGetRepeatWhileValue
 * * * * Release 2.2 * * * *
 *		11/30/98	bva		fix compile warnings
 *		11/25/98	rhb		fix multi buffer bug with eRpItemType_File
 *		11/10/98	rhb		implement eRpItemType_File
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		10/21/98	bva		clean up Server Push
 *		10/05/98	pjr		move some routines to RpParse.c
 *		09/24/98	bva		add Expect header support
 *		09/02/98	bva		fix static prototypes
 *		09/01/98	bva		change rpParsingControl structure
 *		08/31/98	bva		change <CR><LF> definitions
 *		08/04/98	bva		change multi Remote/User host to support 3 digits
 *		07/01/98	pjr		add multi-host support for User Exit.
 *		06/16/98	bva		add RpSetSlaveIdentity
 *		06/10/98	bva		add PUT command support
 *		06/08/98	bva		add support for multiple Remote Hosts
 * * * * Release 2.1 * * * *
 *		04/15/98	bva		use fObjectSource in eRpHttpResponseComplete case
 *		03/23/98	bva		fixed Host header check for 1.0 requests in 1.1 environ
 *		03/18/98	rhb		use kMaxParseLength to determine the max parse line
 *		02/17/98	bva		IPP support for HTTP 1.0
 *		01/21/98	bva		change RpConnectionReceiveTcp in RpHandleHttpAction
 *		01/07/98	pjr		fix TRACE to not serve page after trace data
 * * * * Release 2.0 * * * *
 *		12/16/97	pjr		add new line character to debug printf
 *		12/16/97	bva		put overflow check into RpGetObjectData
 *		12/11/97	bva		kTypePrintForm -> kTypeIppObject
 *		11/22/97	bva		fix HTTP/1.1 OPTIONS support,
 *							rework object overflow handling
 *		11/21/97	bva		fix HTTP/1.1 TRACE support
 *		11/14/97	pjr		add code to RpPushQueryIndex to protect against
 *							fIndexDepth overflow and report it if RomPagerDebug.
 *		11/12/97	pjr		add RomPagerDebug code to serve up an error page
 *							if the request is too large for the work buffer
 *							in RpGetObjectData and RpGetChunkedObjectData.
 *		11/03/97	pjr		parsing changes (use rpParsingControl)
 *		10/28/97	bva		rework User Exit support 
 *		10/14/97	bva		fix compiler warnings
 *		08/27/97	pjr		add eRpAnalyzeHttpRequest state to
 *							RpHandleHttpAction.
 *		08/21/97	bva		rework URL dispatching
 *		08/20/97	bva		add RpIdentifyObjectSource
 *		08/03/97	bva		add RpGetChunkedObjectData, test IPP
 *		07/31/97	rhb		add RpSetUrlState.
 *		07/30/97	bva		rework RpParseHttpHeaders for HTTP 1.1
 *		07/29/97	pjr		remove eRpParsingMultipartWaitForFS case from
 *							RpHandleHttpAction.
 *		07/25/97	pjr		RpParseHeader is now passed thePatternTablePtr.
 *		07/14/97	bva		add OPTIONS and TRACE support
 *		07/12/97	bva		add HTTP->IPP interface,
 *							split off HTTP Header parsing to RpHttpPs.c,
 *							add RpGetChunkedData
 *		07/07/97	bva		rework ParseBrowserDate for memory savings
 *		07/02/97	pjr		rework parsing routines to reduce code size 
 *		06/28/97	bva		rework ParseLine for case-insensitive HTTP headers
 *		06/26/97	pjr		add form based file upload feature
 *		06/19/97	bva		add RpGetRepeatWhileValue 
 *		06/12/97	bva		add URL State handling
 *		05/24/97	bva		rework URL dispatching
 *		05/24/97	bva		get full Content-Type header
 * * * * Release 1.6 * * * *
 *		04/21/97	bva		fix GetBufferRequestLine to accumulate fragments
 *		04/18/97	pjr		add Security Digest feature
 *		04/18/97	bva		fix ProcessHost to get whole host name
 *		04/16/97	bva		add HTTP version detection
 *		04/13/97	bva		fix fCurrentLineLength in GetBufferRequestLine 
 *		04/04/97	bva		cleanup warnings
 *		03/31/97	bva		rework request line handling
 *		03/20/97	bva		fHttpParseState -> fHttpTransactionState
 *		03/05/97	cdl		force exit in RpParseHttpRequest
 *		03/05/97	bva		make RpStoreQueryValues separate routine
 *		01/24/97	bva		add ProcessLanguage, RpGetAcceptLanguage,
 *								make ProcessContentType conditional
 *		01/20/97	bva		make ProcessAgent conditional, capture entire
 *								UserAgent line, add RpGetUserAgent
 *		01/04/97	bva		add RpGetQueryIndexLevel
 *		12/27/96	bva		debug Keep-Alive support, RpCheckQuery
 *		12/12/96	bva		rework RpCheckQuery to add support for 
 * 								"name=value" queries
 *		12/11/96	bva		rework request parsing
 *		12/09/96	rhb		cast theDataLength for memcpy in GetObjectData 
 *		12/06/96	bva		add Keep-Alive support
 *		12/05/96	rhb		GetObjectData no longer writes past end of buffer
 *		12/04/96	bva		save/restore fItemError in RpCheckQuery
 * * * * Release 1.5 * * * *
 *		11/05/96	bva		add conditional compiles
 *		10/18/96	bva		consolidate HTTP headers and form items buffers,
 *							fix ParseBrowserDate
 *		10/10/96	bva		add initializations for picky compilers
 *		10/03/96	bva		fix length problems in GetObjectData 
 *		09/24/96	rhb		support dynamically allocated engine data 
 *		09/22/96	bva		add parsing for HTTP Host header
 *		09/20/96	rhb		allow more than one HTTP request
 * * * * Release 1.4 * * * *
 * * * * Release 1.3 * * * *
 *		07/23/96	bva		cleanup compiler warnings
 *		07/05/96	bva		created from RpHttp.c
 *							generalize query handling for image maps
 * * * * Release 1.2 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if RomPagerServer || RomPagerBasic

static void 		RpEndHeaders(rpHttpRequestPtr theRequestPtr);
static void			SetEndObjectState(rpHttpRequestPtr theRequestPtr);

#if RomPagerHttpOneDotOne
static void			SetUpObjectHandling(rpHttpRequestPtr theRequestPtr);
#endif

extern int redFlag;

RpErrorCode RpHandleHttpAction(rpConnectionPtr theConnectionPtr) {
	Boolean				theReadMoreFlag;
	rpHttpRequestPtr 	theRequestPtr;
	RpErrorCode			theResult;
#if RomPagerSecure && RpExpectHeader
    Boolean             theSendStarted;
    Signed32            theSslResult;
    Unsigned32          theSslSendLength;
#endif

	theResult = eRpNoError;
	theReadMoreFlag = False;
	theRequestPtr = theConnectionPtr->fHttpRequestPtr;

// +++ _Alphanetworks_Patch_, 10/08/2003, jacob_shih
#if defined(_jxWeb)
			_jxWeb_RpTrace("RpHttpRq.c", "RpHandleHttpAction", "");
#endif
// --- _Alphanetworks_Patch_, 10/08/2003, jacob_shih

	switch (theRequestPtr->fHttpTransactionState) {
		case eRpParsingHeaders:
			/*	
				Look for the header info	
			*/
			theReadMoreFlag = RpParseHttpHeaders(theRequestPtr);
			break;
			
		case eRpParsingObjectBody:
			theReadMoreFlag = RpGetObjectData(theRequestPtr);
			break;
			
#if RomPagerHttpOneDotOne
		case eRpParsingChunkedObjectBody:
			theReadMoreFlag = RpGetChunkedObjectData(theRequestPtr);
			break;
			
#endif

#if RomPagerFileUpload
		case eRpParsingMultipart:
			/*	
				Process Multipart headers or data.
			*/
			theReadMoreFlag = RpProcessMultipart(theRequestPtr);
			break;
#endif
			
#if RomPagerIpp
		case eRpParsingIpp:
			theReadMoreFlag = RpHandleIpp(theRequestPtr);
			break;
#endif
			
#if RomPagerPutMethod
		case eRpHandlePut:
			theReadMoreFlag = RpHandlePut(theRequestPtr);
			break;
#endif
			
#if RomPlugAdvanced || RomPagerXmlServices
        case eRpHandleXmlRequest:
            theReadMoreFlag = RpHandleXmlServices(theConnectionPtr);
            break;
#endif

#if RomPlugControl && CpEventSupport
		case eRpHandleGena:
			CpProcessGenaEvent(theRequestPtr);
			theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
			break;
#endif
			
#if RomPagerSoftPages
		case eRpParsingHtml:
			RpHandleParsingHtml(theRequestPtr);
			if (theRequestPtr->fHtmlResponseLength > 0) {
				/*
					Normally, we want to send out a buffer that
					the lower layer built for us.  In the rare
					case that the previous buffer exactly lined
					up with the end marker, we'll get back an
					empty buffer that we don't want to send.
				*/
				theResult = RpSendReplyBuffer(theConnectionPtr);
			}
			break;
#endif
			
		case eRpUnParseable:
			theRequestPtr->fHttpResponseState = eRpHttpBadRequest;
			theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
			break;

		case eRpFindUrl:
			/*
				The RpFindHttpObject routine figures out where the HTTP 
				object will be found.  The object can be in ROM, file 
				system, handled by user exit, or not be found.  The next 
				connection and transaction states will be set up by this 
				routine depending on where it finds (or doesn't) the object.
			*/
			RpFindHttpObject(theConnectionPtr);
			break;

		case eRpEndUrlSearch:
			/*
				Finish the URL search process.
			*/
			RpFinishUrlSearch(theRequestPtr);
			break;

#if RomPagerServer && RomPagerSecurity
		case eRpEndSecurityCheck:
			/*
				Complete the security check process.
			*/
#if RomPagerExternalPassword
			if (theRequestPtr->fPasswordState == eRpPasswordPending) { 
				/*
					If we had to go out to the external password routine,
					then we need to resolve the rest of the security issues.
				*/
				theRequestPtr->fPasswordState =
						RpCheckAccess2(theRequestPtr->fDataPtr,
									theRequestPtr->fObjectPtr->fObjectAccess);
			}
#endif
			if (theRequestPtr->fPasswordState == eRpPasswordAuthorized) {
				switch (theRequestPtr->fObjectSource) {			

#if RomPagerFileUpload
					case eRpFileUploadUrl:
						theRequestPtr->fHttpTransactionState = 
								eRpParsingMultipart;
						break;
#endif
						
#if RomPagerIpp
					case eRpIppUrl:
						theRequestPtr->fHttpTransactionState = 
								eRpParsingIpp;
						break;
#endif

#if RomPlugControl
					case eRpGenaUrl:
						theRequestPtr->fHttpTransactionState = 
								eRpHandleGena;
						break;
#endif

#if RomPagerPutMethod
					case eRpPutUrl:
						theRequestPtr->fHttpTransactionState = 
								eRpHandlePut;
						break;
#endif
						
					case eRpRomUrl:
					default:
						theRequestPtr->fHttpTransactionState = 
								eRpAnalyzeHttpRequest;
						break;
				}
			}
			else {
				theRequestPtr->fPersistent = False;
				theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
			}
			break;
#else	/* RomPagerSecurity */
		case eRpEndSecurityCheck:
			/*
				No security, so just go on
			*/
			switch (theRequestPtr->fObjectSource) {			
#if RomPagerFileUpload
				case eRpFileUploadUrl:
					theRequestPtr->fHttpTransactionState = 
							eRpParsingMultipart;
					break;
#endif
					
#if RomPagerIpp
				case eRpIppUrl:
					theRequestPtr->fHttpTransactionState = 
							eRpParsingIpp;
					break;
#endif
					
#if RomPlugControl
				case eRpGenaUrl:
					theRequestPtr->fHttpTransactionState = 
							eRpHandleGena;
					break;
#endif
					
				case eRpRomUrl:
				default:
					theRequestPtr->fHttpTransactionState = 
							eRpAnalyzeHttpRequest;
					break;
			}
			break;
#endif	/* RomPagerSecurity */

		case eRpAnalyzeHttpRequest:
			/*
				Analyze the HTTP request.
			*/
			theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
			RpAnalyzeHttpRequest(theRequestPtr);
			break;

		case eRpSendingHttpHeaders:
			theResult = RpStartHttpResponse(theConnectionPtr);
			break;

		case eRpSendingHttpResponse:
			/*
				go build a reply buffer, and send it.
			*/
			theResult = RpBuildReply(theConnectionPtr);
			break;

#if RpFileInsertItem
		case eRpOpenInsertFileItem:
			/*
				start opening the file for a File Insert Item.
			*/
			theResult = SfsOpenFile(theConnectionPtr->fIndex, 
					theRequestPtr->fFileNamePtr);
			if (theResult == eRpNoError) {
				theConnectionPtr->fState = eRpConnectionWaitingFileOpen;
				theRequestPtr->fHttpTransactionState = eRpOpeningInsertFileItem;
			}
			else {
				RpSendFileInsertItemError(theConnectionPtr, 
					eRpSendingHttpResponse);
				theResult = eRpNoError;
			}
			break;

		case eRpOpeningInsertFileItem:
		case eRpReadingInsertFileItem:
			/*
				the file is open, start or continue reading it.
			*/
			theConnectionPtr->fFileState = eRpFileReading;
			theResult = SfsReadFile(theConnectionPtr->fIndex,
										theRequestPtr->fHtmlFillPtr,
										theRequestPtr->fFillBufferAvailable);
			theConnectionPtr->fState = eRpConnectionWaitingFileRead;
			theRequestPtr->fHttpTransactionState = eRpReadingInsertFileItem;
			break;

		case eRpClosingInsertFileItem:
			/*
				the file read is complete or in error, start closing it and
				then continue processing items.
			*/
			theResult = RpFileClose(theConnectionPtr);
			theRequestPtr->fHttpTransactionState = eRpSendingHttpResponse;
			break;
#endif

#if RpExpectHeader
		case eRpSendingHttpContinue:
			/*
				Send the 100 Continue header and then
				continue with the object handling.
			*/
#if RomPagerSecure
            theSendStarted = False;

            if (theConnectionPtr->fIsTlsFlag) {
                theSslSendLength = sizeof(kContinue) - 1;
                theSslResult = SSLSend(theConnectionPtr->fSslContext, kContinue,
                        theSslSendLength);
				theSslSendLength = theSslResult;
				   if (theSslResult == SSL_WOULD_BLOCK) {
				   theResult = eRpNoError;
					theConnectionPtr->fPendingSendBuffer = kContinue;
					theConnectionPtr->fPendingSendBufferLen = sizeof(kContinue) - 1;
					theConnectionPtr->fState = eRpConnectionProtocolWaitExternal;
			}else if(theSslResult == SSL_ERROR) {
                theResult = eRpTcpSendError;
                theSendStarted = True;
#if RomPagerDebug
                    RP_PRINTF("RpHandleHttpAction/eRpSendingHttpContinue, "
                        "send error, theSslResult = %d\n", theSslResult);
#endif
			} else 	{
				if(theSslSendLength == (sizeof(kContinue) - 1)) {
					theResult = eRpNoError;
					theSendStarted = True;
					theConnectionPtr->fPendingSendBuffer = NULL;
					theConnectionPtr->fPendingSendBufferLen = 0;
				} else {
					theResult = eRpNoError;
					theSendStarted = True;
					theConnectionPtr->fPendingSendBuffer = kContinue + theSslSendLength; 
					theConnectionPtr->fPendingSendBufferLen = (sizeof(kContinue) - 1) - theSslSendLength; 
#if RomPagerDebug
                        RP_PRINTF("RpHandleHttpAction/eRpSendingHttpContinue, "
                            "partial send: (sizeof(kContinue) - 1) = %d, "
                            "theSslSendLength = %d\n",
                            (sizeof(kContinue) - 1), theSslSendLength);
#endif

				}
			}
			}
            else {
                theResult = StcpSend(theConnectionPtr->fIndex, kContinue,
                                    (StcpLength) (sizeof(kContinue) - 1));
                theSendStarted = True;
            }

            if (theSendStarted) {
                if (theResult == eRpNoError) {
                    theConnectionPtr->fState = eRpConnectionSendingReply;
                    SetUpObjectHandling(theRequestPtr);
                }
                else {
                    theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
                }
            }

#else   /* !RomPagerSecure */

            theResult = StcpSend(theConnectionPtr->fIndex, kContinue,
                                (StcpLength) (sizeof(kContinue) - 1));

            if (theResult == eRpNoError) {
                theConnectionPtr->fState = eRpConnectionSendingReply;
                SetUpObjectHandling(theRequestPtr);
            }
            else {
                theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
            }
#endif  /* RomPagerSecure */

            break;
#endif  /* RpExpectHeader */


#if RomPagerTraceMethod
		case eRpSendingTraceResponse:
			/*
				go send the reply buffer that was built by tracing.
			*/
			theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
			theRequestPtr->fHtmlResponsePtr = 
					theRequestPtr->fHtmlResponseBufferOne;
			theRequestPtr->fHtmlResponseLength =
					(Unsigned16) theRequestPtr->fTraceLength;
			theResult = RpSendReplyBuffer(theConnectionPtr);
			break;
#endif

#if RomPagerServerPush
		case eRpSendingLastDataBuffer:
			RpSendServerPushSeparator(theRequestPtr);
			theRequestPtr->fHtmlBufferReady = False;
			RpFlipResponseBuffers(theRequestPtr);
			theRequestPtr->fHttpTransactionState = eRpServerPushStartTimer;
			theResult = RpSendReplyBuffer(theConnectionPtr);
			break;

		case eRpServerPushStartTimer:
			/*
				Start the server push timing
			*/
			theRequestPtr->fHttpTransactionState = eRpServerPushTimer;
			theConnectionPtr->fState = eRpConnectionHolding;
			theConnectionPtr->fProtocolTimer = 1;
			break;

		case eRpServerPushTimer:
			theResult = RpHandleServerPushWait(theConnectionPtr);
			break;
#endif

		case eRpHttpResponseComplete:
#if RomPagerUserExit
			if (theRequestPtr->fObjectSource != eRpUserUrl ||
					theRequestPtr->fCgi.fResponseState == eRpCgiLastBuffer) {
				theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete2;
			}
			else {
				theConnectionPtr->fState = eRpConnectionWaitingUserExit;
			}
#else
			theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete2;
#endif
#if RomPagerFileSystem
			if (theConnectionPtr->fFileState != eRpFileClosed) {
				theResult = RpFileClose(theConnectionPtr);
			}
#endif
			break;
					
		case eRpHttpResponseComplete2:
#if RomPagerServerPush
			if (theRequestPtr->fServerPushActive) {
				theRequestPtr->fHttpTransactionState = eRpSendingLastDataBuffer;
				break;
			}
#endif
			theResult = RpConnectionCheckTcpClose(theConnectionPtr);

			break;

		default:
			break;
			
	} 	/* end switch */
	/*
		fire off another receive if we need to
	*/
	if (theReadMoreFlag) {
		theResult = RpConnectionReceiveTcp(theConnectionPtr);
	}
	return theResult;
}

Boolean RpParseHttpHeaders(rpHttpRequestPtr theRequestPtr) {
	rpLineState		theLineState;
	Boolean			theReadMoreFlag;

	theReadMoreFlag = False;
	theLineState = RpParseReplyBuffer(theRequestPtr->fParsingControlPtr);
	switch (theLineState) {
		case eRpLineComplete:
			if (theRequestPtr->fHaveRequestLine) {
				/*
					convert headers to lower case for all lines but the
					first which is the request line.
				*/
				RpConvertHeaderToLowerCase(theRequestPtr->fParsingControlPtr);
			}
			else {
				/*
					we got the request line
				*/
				theRequestPtr->fHaveRequestLine = True;
			}
			RpParseHeader(theRequestPtr,
							theRequestPtr->fDataPtr->fHttpPatternTable);
			break;

		case eRpLinePartial:
			theReadMoreFlag = True;
			break;

		case eRpLineError:
			theRequestPtr->fHttpTransactionState = eRpUnParseable;	
			break;

		case eRpLineEmpty:
// +++ _Alphanetworks_Patch_, 10/08/2003, jacob_shih
#if defined(_jxWeb)
			_jxWeb_TraceHttpRequest();
#endif
// --- _Alphanetworks_Patch_, 10/08/2003, jacob_shih
			/*	
				we have an empty line (CRLF or LF), so we 
				are done parsing the headers.
			*/
			RpEndHeaders(theRequestPtr);
			break;
			
	}
	return theReadMoreFlag;
}
//This function "RpWebID_GetAccessRight()" is written in file "sRpWebID.c"
extern int RpWebID_GetAccessRight(char *l_RpWebID);
static void RpEndHeaders(rpHttpRequestPtr theRequestPtr) {
	if (theRequestPtr->fHttpCommand == eRpHttpNoCommand && 
			!theRequestPtr->fHaveRequestLine) {
		/*
			Well, the only thing we've read so far is a CRLF and we
			didn't even get a request line.  We're just going to ignore 
			this and go read some more headers.  How could this happen you ask?
			
			Well some browsers (who shall remain nameless but their
			initials are MSIE) append a CRLF to the object in a POST
			command.  In the bad old days of HTTP 1.0 we could just
			ignore it after reading the data we want, because we 
			would close the connection anyway.  In these modern days 
			of HTTP 1.1 or Keep-Alive, we need to support pipelined 
			requests, so anything in the buffer must handled in a 
			gracious manner in order to keep the connection open.
		*/
		return;
	}

#if RomPagerUrlState
	RpCheckUrlState(theRequestPtr);
#endif

#if RomPagerHttpOneDotOne

	if (!theRequestPtr->fHaveHost && 
		theRequestPtr->fHttpVersion == eRpHttpOneDotOne) {
		/*
			All HTTP/1.1 clients MUST send in the Host header field.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpBadRequest;
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
#if RomPagerMPostMethod
	else if (theRequestPtr->fHttpCommand == eRpHttpMPostCommand) {
		/*
			We got an M-POST request. Reject it with a "501 Not Implemented",
			so that the requester will try a regular POST. 
		*/
		theRequestPtr->fHttpResponseState = eRpHttpNotImplemented;
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
#endif
	else if (theRequestPtr->fHaveRequestObject) {
#if RpExpectHeader
		if (theRequestPtr->fWantsContinue) {
			/*
				The client wants us to send a 100 Continue header.
			*/
			if (theRequestPtr->fPostRequestLength >= kHttpWorkSize) {
				/*
					We know we don't have room for the object, so reject the
					request.
				*/
				theRequestPtr->fHttpResponseState = eRpHttpExpectFailed;
				theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
			}
			else {
				/*
					Go send the Continue header.
				*/
				theRequestPtr->fHttpTransactionState = eRpSendingHttpContinue;
			}
			return;
		}
#endif
		SetUpObjectHandling(theRequestPtr);		
	}
	else if (theRequestPtr->fHttpCommand == eRpHttpNoCommand) {
		/*
			We got at least one header, and no request object,
			but we didn't find a method we support in the parsing, 
			so just reject the request. GET, POST, and HEAD are the 
			supported methods for HTTP/1.0, and HTTP/1.1 adds 
			optional support for OPTIONS, PUT, DELETE, and TRACE. 
		*/
		theRequestPtr->fHttpResponseState = eRpHttpBadCommand;
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
	else if (theRequestPtr->fHttpCommand == eRpHttpPostCommand) {
		/*
			If the POST request is a HTTP 1.1 request and we don't
			have Content-Length or Transfer-Coding of chunked, then
			we have a bad request.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpBadRequest;
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
#if RomPagerPutMethod
	else if (theRequestPtr->fHttpCommand == eRpHttpPutCommand) {
		/*
			If we have a PUT request and we don't have Content-Length 
			or Transfer-Coding of chunked, then we have a bad request.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpBadRequest;
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
#endif
#if RomPagerTraceMethod
	else if (theRequestPtr->fHttpCommand == eRpHttpTraceCommand) {
		/*
			If we got a HTTP/1.1 TRACE request we just want to
			start sending back the reply.
		*/
		theRequestPtr->fObjectPtr = &theRequestPtr->fLocalObject;
		theRequestPtr->fObjectPtr->fCacheObjectType = eRpObjectTypeDynamic;
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
#endif
#if RomPagerOptionsMethod
	/*
		If we got a HTTP/1.1 OPTION * request we just want to
		start sending back the reply.
	*/
	else if (theRequestPtr->fHttpCommand == eRpHttpOptionsCommand &&
			*(theRequestPtr->fPath) == '*') {
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
#endif
#if	RomPlugAdvanced
	else if (theRequestPtr->fHttpCommand == eRpHttpSubscribeCommand) {
		/*
			If we got a UPnP SUBSCRIBE request we need to validate it,
			and set up the subscription before sending the response headers.
		*/
		RuHandleSubscribeCommand(theRequestPtr);
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
	else if (theRequestPtr->fHttpCommand == eRpHttpUnsubscribeCommand) {
		/*
			If we got a UPnP UNSUBSCRIBE request we need to validate it,
			and cancel the subscription before sending the response headers.
		*/
		RuHandleUnsubscribeCommand(theRequestPtr);
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
#endif
	else {
		/*
			We have normal request (probably a GET) with no
			request object, so go handle it.
		*/
		theRequestPtr->fHttpTransactionState = eRpFindUrl;
	}
	return;
}
					
static void SetUpObjectHandling(rpHttpRequestPtr theRequestPtr) {

#if RomPagerIpp
	if (theRequestPtr->fDataType == eRpDataTypeIpp) {
		RpSetupIppRequest(theRequestPtr);
		return;
	}
#endif
#if RomPagerFileUpload
	if (theRequestPtr->fDataType == eRpDataTypeFormMultipart) {
		RpSetupFileUploadRequest(theRequestPtr);
		return;
	} 
#endif
#if RomPagerPutMethod
	if (theRequestPtr->fHttpCommand == eRpHttpPutCommand) {
		/*
			We might want to set up something special for the PUT method
		*/
		RpSetupPutRequest(theRequestPtr);
		return;
	}
#endif
#if RomPlugControl && CpEventSupport
	if (theRequestPtr->fHttpCommand == eRpHttpNotifyCommand) {
		/*
			Set up to process a GENA event notification.
		*/
		CpSetupGenaRequest(theRequestPtr);
	}
#endif
	/*
		We have an object with the request and it's not anything else,
		so it must be a POST request.  Set up the object handling state.
	*/
	if (theRequestPtr->fObjectIsChunked) {
		theRequestPtr->fHttpTransactionState = eRpParsingChunkedObjectBody;
	}
	else {
		theRequestPtr->fHttpTransactionState = eRpParsingObjectBody;
	}
	return;
}

#else 	/* RomPagerHttpOneDotOne */


	if (theRequestPtr->fHttpCommand == eRpHttpNoCommand) {
		theRequestPtr->fHttpResponseState = eRpHttpBadCommand;
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
	else if (theRequestPtr->fHttpCommand == eRpHttpPostCommand) {
#if RomPagerFileUpload
		if (theRequestPtr->fDataType == eRpDataTypeFormMultipart) {
			RpSetupFileUploadRequest(theRequestPtr);
			return;
		} 
#endif
#if RomPagerIpp
		/*
			In theory IPP requires HTTP 1.1, but if we have this code here,
			it should work with 1.0, providing the client sends the
			Content-Length header.
		*/
		if (theRequestPtr->fDataType == eRpDataTypeIpp) {
			RpSetupIppRequest(theRequestPtr);
			return;
		}
#endif
		theRequestPtr->fHttpTransactionState = eRpParsingObjectBody;
	}
	else {
		theRequestPtr->fHttpTransactionState = eRpFindUrl;
	}
	return;
}

#endif 	/* RomPagerHttpOneDotOne */


RpErrorCode RpStartHttpResponse(rpConnectionPtr theConnectionPtr) {
	rpHttpRequestPtr	theRequestPtr;
	RpErrorCode			theResult;
	Boolean             theSendStarted;
	
	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	RpBuildHttpResponseHeader(theRequestPtr);
	theSendStarted = False;
	
// +++ _Alphanetworks_Patch_, 10/08/2003, jacob_shih
#if defined(_jxWeb)
	_jxWeb_TraceHttpResponseHeader();
#endif
// --- _Alphanetworks_Patch_, 10/08/2003, jacob_shih

#if RomPagerSecure
    if (theConnectionPtr->fIsTlsFlag) {
        Signed32      theSslResult;
        Unsigned32  theSslSendLength;

        theSslSendLength = RP_STRLEN(theRequestPtr->fHttpWorkBuffer);
        theSslResult = SSLSend(theConnectionPtr->fSslContext, theRequestPtr->fHttpWorkBuffer,
                theSslSendLength);
		theSslSendLength = theSslResult;

            if (theSslResult == SSL_WOULD_BLOCK) {
				   theResult = eRpNoError;
					theConnectionPtr->fPendingSendBuffer = theRequestPtr->fHttpWorkBuffer;
					theConnectionPtr->fPendingSendBufferLen = RP_STRLEN(theRequestPtr->fHttpWorkBuffer);
					theConnectionPtr->fState = eRpConnectionProtocolWaitExternal;
			}else if(theSslResult == SSL_ERROR) {
                theResult = eRpTcpSendError;
                theSendStarted = True;
#if AsDebug
            RP_PRINTF("RpStartHttpResponse, send:error, theSslResult = %d\n",
                    theSslResult);
#endif
			} else 	{
				if(theSslSendLength == RP_STRLEN(theRequestPtr->fHttpWorkBuffer)) {
					theResult = eRpNoError;
					theSendStarted = True;
					theConnectionPtr->fPendingSendBuffer = NULL;
					theConnectionPtr->fPendingSendBufferLen = 0;
				} else {
					theResult = eRpNoError;
					theSendStarted = True;
					theConnectionPtr->fPendingSendBuffer = theRequestPtr->fHttpWorkBuffer + theSslSendLength; 
					theConnectionPtr->fPendingSendBufferLen = RP_STRLEN(theRequestPtr->fHttpWorkBuffer) - theSslSendLength; 
#if AsDebug
                RP_PRINTF("RpStartHttpResponse, partial send: "
                    "RP_STRLEN(theRequestPtr->fHttpWorkBuffer) = %d, "
                    "theSslSendLength = %d\n",
                    RP_STRLEN(theRequestPtr->fHttpWorkBuffer), theSslSendLength);
#endif
				}
			}
	}
  else {
        theResult = StcpSend(theConnectionPtr->fIndex,
                        theRequestPtr->fHttpWorkBuffer,
                        (StcpLength) RP_STRLEN(theRequestPtr->fHttpWorkBuffer));
        theSendStarted = True;
    }

#else /* RomPagerSecure */

    theResult = StcpSend(theConnectionPtr->fIndex,
                    theRequestPtr->fHttpWorkBuffer,
                    (StcpLength) RP_STRLEN(theRequestPtr->fHttpWorkBuffer));
    theSendStarted = True;
#endif /* RomPagerSecure */

    if (theSendStarted) {
        if (theResult == eRpNoError) {
            theConnectionPtr->fState = eRpConnectionSendingReply;
        }
        else {
            theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
        }
    }
    return theResult;
}


Boolean RpGetObjectData(rpHttpRequestPtr theRequestPtr) {
	Signed32			theDataLength;
	rpParsingControlPtr	theParsingPtr;
	char *				theWorkBufferPtr;
	Boolean				theReadMoreFlag;

	/*
		If we didn't get passed an object length, or we got passed a
		length of 0, mark the request unparseable and return.  We could
		just use what's left in the buffer, possibly searching for CRLF
		which some incorrect browsers add on, but that would be wrong.
	*/
	if (theRequestPtr->fPostRequestLength < 1) {
		theRequestPtr->fHttpTransactionState = eRpUnParseable;
		return False;
	}

	/*
		If the request is too large, just return.  We will send a
		response to the browser and then close the connection to
		flush the request.
	*/
	if (theRequestPtr->fPostRequestLength >= kHttpWorkSize) {
		theRequestPtr->fHttpResponseState = eRpHttpRequestTooLarge;
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
#if RomPagerHttpOneDotOne || RomPagerDebug
		/*
			Set up the notification page.
		*/
#if RomPagerBasic
		RpInternalCgi(theRequestPtr, gRpInputTooLargeText);
#else
		theRequestPtr->fObjectPtr = &gRpInputTooLargePage;
#endif
#endif	/* RomPagerHttpOneDotOne || RomPagerDebug */
#if RomPagerHttpOneDotOne || RomPagerKeepAlive
		/*
			Mark the connection to close after we send the notification.
		*/
		theRequestPtr->fPersistent = False;
#endif
		return False;
	}

	theParsingPtr = theRequestPtr->fParsingControlPtr;

	/*
		On the first pass, set up the global object length
		to be used for multi-buffer collection of object data.
	*/
	if (theParsingPtr->fHttpObjectLengthToRead == 0) {
		theParsingPtr->fHttpObjectLengthToRead = 
				theRequestPtr->fPostRequestLength;
	}

	if (theParsingPtr->fIncomingBufferLength >=
							theParsingPtr->fHttpObjectLengthToRead) {
		/*
			The buffer contains all we need for this object, so set the 
			length to the remaining length of the object.  In a HTTP 1.1
			pipelined environment, there may be more in the buffer than 
			just our object. Some browsers also incorrectly append a CRLF 
			to the object. 
		*/
		theDataLength = theParsingPtr->fHttpObjectLengthToRead;
		SetEndObjectState(theRequestPtr);
		theReadMoreFlag = False;
	}
	else {
		/*
			The buffer has less than what we need for the object, so
			take everything and go get some more.
		*/
		theDataLength = theParsingPtr->fIncomingBufferLength;
		theParsingPtr->fHttpObjectLengthToRead -= theDataLength;
		theReadMoreFlag = True;
	}
	
	/*
		Store the POST object in the request structure and
		make it a C string for our parsing convenience.
	*/
	theWorkBufferPtr = theRequestPtr->fHttpWorkBuffer + 
			RP_STRLEN(theRequestPtr->fHttpWorkBuffer);
	RP_MEMCPY(theWorkBufferPtr, theParsingPtr->fCurrentBufferPtr, 
		(Unsigned16) theDataLength);
	*(theWorkBufferPtr + theDataLength) = '\0';

	/*	
		set up to process the rest of the buffer
	*/
	theParsingPtr->fCurrentBufferPtr += theDataLength;
	theParsingPtr->fIncomingBufferLength -= theDataLength;

	return theReadMoreFlag;
}

#if RomPagerHttpOneDotOne

Boolean RpGetChunkedObjectData(rpHttpRequestPtr theRequestPtr) {
	Signed32			theDataLength;
	rpParsingControlPtr	theParsingPtr;
	char *				theWorkBufferPtr;
	Boolean				theReadMoreFlag;

	theParsingPtr = theRequestPtr->fParsingControlPtr;

	if (theParsingPtr->fIncomingBufferLength == 0) {
		/*
			If there is no more data in the TCP buffer, signal the higher 
			layer that we want some.
		*/
		return True;
	}

	theReadMoreFlag = RpGetChunkedData(theRequestPtr);
	if (theRequestPtr->fHaveChunk) {
		/*
			If the chunked input will not fit in the buffer, then
			set up to notify the browser.  We will close the connection
			to flush the data after notification, so don't bother
			reading any more.
		*/
		if (theRequestPtr->fChunkedContentLength >= kHttpWorkSize) {
			theRequestPtr->fHttpResponseState = eRpHttpRequestTooLarge;
			theRequestPtr->fPersistent = False;
#if RomPagerBasic
			RpInternalCgi(theRequestPtr, gRpInputTooLargeText);
#else
			theRequestPtr->fObjectPtr = &gRpInputTooLargePage;
#endif
			return False;
		}
		
		/*
			Store the data in the work buffer and make it a 
			C string for convenience in parsing.
		*/
		theDataLength = theRequestPtr->fCurrentChunkBufferLength;
		theWorkBufferPtr = theRequestPtr->fHttpWorkBuffer + 
				RP_STRLEN(theRequestPtr->fHttpWorkBuffer);
		RP_MEMCPY(theWorkBufferPtr, theRequestPtr->fCurrentChunkBufferPtr, 
				(Unsigned16) theDataLength);
		*(theWorkBufferPtr + theDataLength) = '\0';
	}
	/*
		When we have copied the chunked data, we need to move the
		input buffer pointer past the optional chunked footer and 
		the trailing empty line.
	*/
	while (theRequestPtr->fChunkState == eRpGettingChunkedEnd && 
			!theReadMoreFlag) {
		theReadMoreFlag = RpGetChunkedData(theRequestPtr);
	}
	/*
		Is this the last buffer?
	*/
	if (theRequestPtr->fChunkState == eRpEndChunked) {
		SetEndObjectState(theRequestPtr);
		theReadMoreFlag = False;
	}

	return theReadMoreFlag;
}

#endif /* RomPagerHttpOneDotOne */


static void SetEndObjectState(rpHttpRequestPtr theRequestPtr) {

	theRequestPtr->fHttpTransactionState = eRpFindUrl;
#if RomPagerHttpOneDotOne
	if (theRequestPtr->fHttpCommand == eRpHttpNoCommand) {
		/*
			We got headers and a request object, but we didn't find a 
			method we support in the parsing, so reject the request
			after swallowing the object.  GET, POST, and HEAD are the 
			supported methods for HTTP/1.0, and HTTP/1.1 adds 
			optional support for OPTIONS, PUT, DELETE, and TRACE. 
		*/
		theRequestPtr->fHttpResponseState = eRpHttpBadCommand;
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	}
#endif 
	return;
}


#if RomPagerUrlState
void RpCheckUrlState(rpHttpRequestPtr theRequestPtr) {
	char *			thePathPtr;
	Unsigned16		theTokenLength;
  	char			theTempPath[kMaxLineLength];
	
	/*
		This routine looks for URLs of the form "/xx/yyy/zzzz" where "xx"
		is a magic identifier, "yyy" is a state to be stored away, and
		"zzzz" is the rest of the URL (hopefully a normal one).  
		
		If "xx" matches the string defined by kUrlState, then "yyy" (an 
		arbitrary string) is stored in the fUrlState field of the request 
		block, the "/xx/yyy" is stripped off the front of the URL, so
		that the rest of it can be processed in a normal fashion.
	*/

	thePathPtr = theRequestPtr->fPath;
	thePathPtr++;
	theTokenLength = RpFindTokenDelimited(thePathPtr, kAscii_Slash);
	if (*(thePathPtr + theTokenLength) == '\0') {
		/*
			No other slashes, so return.
		*/			
		return;
	}
	if (RP_MEMCMP(thePathPtr, kUrlState, theTokenLength) != 0) {
		/*
			The "xx" doesn't match the magic identifier
		*/			
		return;
	}
	/*
		We found the "xx", so find the "yyy"
	*/
	thePathPtr += theTokenLength + 1;
	theTokenLength = RpFindTokenDelimited(thePathPtr, kAscii_Slash);
	if (*(thePathPtr + theTokenLength) == '\0') {
		/*
			No other slashes, so the "xx" must have matched by mistake,
			or this is an invalidly formed URL (no "zzzz").  Just return 
			and let the URL get handled elsewhere (probably rejected).
		*/			
		return;
	}
	/*
		save the "yyy" away
	*/
	RP_MEMCPY(theRequestPtr->fUrlState, thePathPtr, theTokenLength);
	*(theRequestPtr->fUrlState + theTokenLength) = '\0';
	/*
		strip off the "/xx/yyy" from the URL
	*/
	thePathPtr += theTokenLength;
	RP_STRCPY(theTempPath, thePathPtr);	
	RP_STRCPY(theRequestPtr->fPath, theTempPath);	

	return;
}

char * RpGetUrlState(void *theTaskDataPtr) {

	return ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fUrlState;
}

void RpSetUrlState(void *theTaskDataPtr, char *theUrlState) {

	RP_STRCPY(((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fUrlState,
			theUrlState);
	return;
}
#endif	/* RomPagerUrlState */


#if RomPagerHttpOneDotOne

Boolean RpGetChunkedData(rpHttpRequestPtr theRequestPtr) {
	char *				theInputBufferPtr;
	rpParsingControlPtr	theParsingPtr;
	Signed32			theChunkLength;
	Signed32			theInputBufferLength;
	Boolean				theErrorFlag;
	char				theHexChar;
	rpLineState			theLineState;
	Unsigned8			theNibble;
	Boolean				theReadMoreFlag;

	theReadMoreFlag = False;
	theRequestPtr->fHaveChunk = False;
	theParsingPtr = theRequestPtr->fParsingControlPtr;
	if (theRequestPtr->fChunkState == eRpGettingChunkedLength) {
		theLineState = RpParseReplyBuffer(theParsingPtr);
		switch (theLineState) {
			case eRpLineComplete:
				/*
					We must have a chunked length to look at.
				*/
				theInputBufferPtr = theParsingPtr->fCurrentBeginLinePtr;
				theHexChar = *theInputBufferPtr++;
				theChunkLength = 0;
				while (theHexChar != '\x0d' &&
						theHexChar != kAscii_SemiColon) {
					/*
						Ignore spaces in the chunked length.  Some servers
						have been known to incorrectly pad a 2 digit chunked
						length with a space (example: af<SP> instead of 0af).
						Although we have only seen a chunked length padded
						with a space on the right, ignoring a space anywhere
						in the length will also protect us should someone
						decide to pad the length with a space on the left.
 					*/
					if (theHexChar != kAscii_Space) {
						theNibble = RpHexToNibble(&theErrorFlag, theHexChar);
						theChunkLength = (theChunkLength << 4) + theNibble;
					}
					theHexChar = *theInputBufferPtr++;
				}
				if (theChunkLength > 0) {
					theRequestPtr->fChunkLength = theChunkLength;
					theRequestPtr->fChunkLengthProcessed = 0;
					theRequestPtr->fChunkState = eRpGettingChunkedData;
				}
				else {
					/*
						a chunk length of 0 means no more chunks
					*/
					theRequestPtr->fChunkState = eRpGettingChunkedEnd;
				}
				break;
	
			case eRpLinePartial:
				theReadMoreFlag = True;
				break;
	
			case eRpLineError:
				break;
	
			case eRpLineEmpty:
				/*	
					we have an empty line (CRLF or LF), so we have
					the line after the chunked data.
				*/
				break;
		}
	}
	else if (theRequestPtr->fChunkState == eRpGettingChunkedEnd) {
		theLineState = RpParseReplyBuffer(theParsingPtr);
		switch (theLineState) {
			case eRpLineComplete:
				/*
					We must have a chunked footer to look at.
				*/
				break;
	
			case eRpLinePartial:
				theReadMoreFlag = True;
				break;
	
			case eRpLineError:
				break;
	
			case eRpLineEmpty:
				/*	
					we have an empty line (CRLF or LF), so we 
					have the end of all the chunks and footers.
				*/
				theRequestPtr->fChunkState = eRpEndChunked;
				break;
		}
	}
	else {
		/*
			must be getting chunked data
		*/
		theInputBufferLength = theParsingPtr->fIncomingBufferLength;
		if (theInputBufferLength + theRequestPtr->fChunkLengthProcessed >= 
				theRequestPtr->fChunkLength) {
			/*
				This buffer will finish our chunk
				Get the length of the remaining piece of the chunk
			*/
			theChunkLength = theRequestPtr->fChunkLength - 
					theRequestPtr->fChunkLengthProcessed;
			theRequestPtr->fChunkState = eRpGettingChunkedLength;
		}
		else {
			/*
				Since this buffer won't finish our chunk, just grab the
				whole thing and set up for the caller to use it.
			*/
			theChunkLength = theInputBufferLength;
		}
		theRequestPtr->fCurrentChunkBufferLength = theChunkLength;
		theRequestPtr->fChunkLengthProcessed += theChunkLength;
		theRequestPtr->fChunkedContentLength += theChunkLength;
		theRequestPtr->fCurrentChunkBufferPtr = theParsingPtr->fCurrentBufferPtr;
		theParsingPtr->fCurrentBufferPtr += theChunkLength;
		theParsingPtr->fIncomingBufferLength -= theChunkLength;
		theRequestPtr->fHaveChunk = True;
	}
	return theReadMoreFlag;
}

#endif /* RomPagerHttpOneDotOne */

#endif	/* RomPagerServer || RomPagerBasic */
