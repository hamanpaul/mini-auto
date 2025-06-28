/*
 *	File:		RpHttp.c
 *
 *	Contains:	RomPager routines for HTTP
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2003 by Allegro Software Development Corporation
 *	All rights reserved.
 *
 *	This module contains confidential, unpublished, proprietary
 *	source code of Allegro Software Development Corporation.
 *
 *	The copyright notice above does not evidence any actual or intended
 *	publication of such source code.
 *
 *	License is granted for specific uses only under separate
 *	written license by Allegro Software Development Corporation.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 * * * * Release 4.30  * * *
 *		09/19/03	pjr		only set error path if it isn't already set
 *		08/13/03	bva		rework persistence and Keep-Alive signalling
 *		08/06/03	bva		add Range request support
 *		07/17/03	amp/lxa	fix support for RpSetRedirectAbsolute
 *		07/13/03	bva		add support for RpSetRedirectAbsolute
 *      06/22/03    bva     set persistence off for certain error messages
 *      06/20/03    bva     eRpSoapUrl -> eRpUpnpSoapUrl
 *		06/09/03	nam		add RomPagerXmlServices support
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 * * * * Release 4.20  * * *
 *		02/21/03	pjr		try to protect against poorly written CGI routines
 *		02/13/03	bva		conditionals for RomPager Basic with RomPagerSecure
 *		02/05/03	bva		add conditionals to support RomPager Basic
 *		01/31/03	rhb		fix Soft Pages from Remote Host
 *      01/28/03    rhb     use RomPagerSoftPageProcessUserExit
 *		12/12/02	pjr		rework nonce generation, fix nextnonce generation
 *							for strict digest without IP address
 *		12/03/02	rhb		allow User Exit to return 500 Server Error
 *		10/04/02	amp		add kRpObjFlag_ForceHttps and kRpObjFlag_ForceHttp
 *      10/04/02    amp     update for new RomPagerSecure
 *		09/04/02	pjr		modify RpHandleCgiResponse for internal security
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 * 		06/26/02	rhb		Don't always aggregate User Exit response buffers
 * * * * Release 4.10  * * *
 *		06/08/02	bva		remove RomPagerSlaveIdentity
 *		04/25/02	rhb		always allow CGI to recall application
 *		04/03/02	bva		add support for eRpObjectTypeDynamicClose
 * * * * Release 4.07  * * *
 *		03/04/02	rhb		disable fSoftPageFlag if internal error detected
 *		11/28/01	rhb		fix Soft Page/User Exit bugs with buffer handling
 * * * * Release 4.06  * * *
 * * * * Release 4.03  * * *
 *		10/30/01	bva		move RpSetConnectionClose to RpCallBk.c
 *		10/16/01	rhb		update SSL/TLS debug code
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		08/08/01	bva		fix SoftPages persistence bug
 *		07/06/01	amp		change RpInitRequestStates to support RomUpnpControl
 *		07/06/01	amp		add GENA processing
 *		07/06/01	amp		add SOAP processing
 *		05/24/01	bva		fix obscure Keep-Alive problem with Static demo pages
 *		05/02/01	bva		improve UserExit buffer efficiency
 *		02/16/01	rhb		initialize fSnmpInitialized if just RomPagerServer
 *		02/14/01	rhb		rename eRpItemError_* to eAsItemError_*
 *		02/13/01	rhb		rename RomPagerSnmpAccess to AsSnmpAccess
 *		01/24/01	rhb		fix SoftPages/HTTP 1.1 and SoftPages/multiple
 *								buffer User Exit bugs
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		10/27/00	pjr		move index values to the connection structure
 *		09/21/00	pjr		eRpHttpFileSystemError -> eRpHttpInternalServerError
 *		09/13/00	rhb		don't set state to eRpHttpNotModified if RpNoCache
 *		09/08/00	pjr		fix compile warning
 *		08/31/00	bva		use Precondition Failed support
 *		08/07/00	rhb		use kRpQueryCharacter
 *		08/07/00	bva		add Content-Length support for RpHandleCgiResponse
 *		07/31/00	bva		use RpBuildIpAddressHostName
 *		07/25/00	rhb		Support SSL/TLS
 *		05/26/00	bva		use gRpDataPtr
 *		05/02/00	pjr		in RpBuildReply, if fObjectSource is bad (default),
 *							complete the response so we don't loop forever
 *		02/09/00	bva		use AsEngine.h
 *		01/24/00	bva		for user exit buffers in HTTP 1.1, make sure that
 *							the final chunk gets written if the user returns
 *							a 0 length buffer as the final buffer
 *							fix warning
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.10  * * *
 * * * * Release 3.01  * * *
 *		04/19/99	pjr		add "qop=auth" header to digest challenge
 *		04/16/99	rhb		remember fSoftPageRequestPtr in RpInitRequestStates
 * * * * Release 3.0 * * * *
 *		04/02/99	pjr		fix compiler warning
 *		03/16/99	pjr		rework debug code
 *		02/27/99	bva		add HTTP Cookie support
 *		01/31/99	bva		add user dictionary per connection
 *		01/29/99	bva		add Etag support
 *		01/24/99	pjr		add RpHandleUserExit (moved from AsMain.c)
 *		01/23/99	pjr		add RpSendReplyBuffer (moved from AsMain.c)
 *		01/19/99	pjr		rework some conditionals for RomPagerBasic
 *		01/06/99	pjr		move gMethodsAllowed from RpData.c
 *		01/06/99	rhb		don't create Content Length Header for Soft Pages
 *		01/05/99	pjr		disable ROM URLs for RomPagerBasic
 *		12/28/98	pjr		move some routines to RpHttpDy.c
 *		12/24/98	pjr		move URL search routines to RpUrl.c:
 *								RpFindHttpObject, RpFindUrl
 *								RpSearchRomObjectList, and RpFinishUrlSearch
 *		12/15/98	bva		add request serial number
 * * * * Release 2.2 * * * *
 *		11/30/98	bva		fix compile warnings
 *		11/23/98	bva		use an old date for "Expires" header
 *		11/13/98	bva		make RpSearchRomObjectList multi-tier
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		10/22/98	pjr		eRmBase64_0 -> eRpBase64_0
 *		10/16/98	pjr		move RpBuildSeparator to RpCommon.c
 *		10/14/98	bva		improve user exit support
 *		09/25/98	bva		remove unused variable
 *		09/24/98	bva		add Expect header support
 *		09/21/98	pjr		move file related fields to the connection block
 *		09/14/98	bva		disable eRpDataTypePageRedirect processing
 *		09/01/98	bva		rpParsingControl moves to connection block
 *		08/31/98	bva		change HTTP Trace
 *		08/31/98	pjr		handle multipart form type in RpAnalyzeHttpRequest
 *		08/26/98	pjr		clear the request block RpInitRequestStates
 *		07/31/98	bva		fix form error page redirection crash
 *		07/23/98	pjr		fix compiler errors and warnings
 *		07/18/98	bva		add debugging support for HTTP flow
 *		07/06/98	bva		fix warnings by removing BuildSeparator declaration
 *		07/06/98	bva		add outbound ContentDisposition header support
 *		07/01/98	pjr		add multi-host support for User Exit.
 *		06/22/98	pjr		space is now included in the content length string.
 *		06/19/98	bva		add slave identity and URL state to BuildRefresh
 *		06/18/98	pjr		don't call RpCheckQuery if the object source is
 *							Remote Host.
 *		06/17/98	pjr		initialize fHttpContentType for RomPagerRemoteHost.
 *		06/16/98	bva		add support for slave server identity
 * * * * Release 2.1 * * * *
 *		04/20/98	rhb		initialize fItemNumber in RpInitRequestStates()
 *		04/15/98	bva		rework BuildDateHeaders to improve user exit support
 *		04/02/98	bva		add user exit support for redirect and not modified
 *		03/26/98	pjr		use theCgiPtr->fDataType for User Exit URLs
 *		03/25/98	pjr		fix User Exit case in RpBuildReply to handle
 *							sending multiple buffers properly
 *		03/16/98	bva		improve Accept type checking
 *		02/14/98	bva		eRpConnectionRemoteHostWait->eRpConnectionHolding
 *		02/13/98	rhb		change parameter to RpCatUnsigned32ToString
 *		02/01/98	bva		fNewLineLast -> fLastCharacter
 *		01/20/98	bva		fix URL state support
 *		01/19/98	bva		eRpConnectionNeedsRemoteHostAction ->
 *								eRpConnectionNeedsProtocolAction
 *		01/13/98	bva		add URL state support for redirects
 *		01/13/98	rhb		remove the cast for fServerConnectionPtr
 *		01/07/98	bva		allow more than 256 objects in RpSearchRomObjectList
 *		01/06/98	pjr		add HTTP event logging (RomPagerLogging).
 * * * * Release 2.0 * * * *
 *		12/16/97	bva		check POST object overflow in RpGetObjectData
 *		12/09/97	bva		rework BuildDateHeaders for HTTP 1.1 and code savings
 *		12/08/97	bva		change OPTIONS behavior to match latest spec
 *		12/08/97	pjr		do not call RpSendReplyBuffer from the ROM object
 *							case in RpBuildReply if the response length is 0.
 *		11/26/97	pjr		change BuildDateHeaders for Remote Host feature
 *		11/26/97	bva		eliminate some compiler warnings
 *		11/22/97	bva		rework object overflow handling
 *		11/19/97	rhb		add support for eRpDataTypePageRedirect
 *		11/07/97	pjr		add Remote Host feature
 *		10/28/97	bva		add RpNoCache, add default URL searching,
 *							rework User Exit support
 *		10/10/97	bva		fix compiler warnings
 *		09/14/97	bva		integrate RomMailer
 *		09/12/97	pjr		change how stale nonce string is sent.
 *		08/27/97	pjr		optimize RpSearchRomObjectList.
 *		08/26/97	pjr		send kCRLF after MIME type string.
 *		08/21/97	bva		rework URL dispatching
 * * * * Release 1.70b1 * * * *
 *		08/18/97	bva		kServerName -> kServerHeader
 *		08/12/97	pjr		remove multipart related code from
 *							RpAnalyzeHttpRequest.  it is no longer used when
 *							processing multipart forms.
 *		08/11/97	pjr		only initialize fAnyHeader and fHaveRequestObject
 *							if RomPagerHttpOneDotOne.
 *		08/06/97	pjr		don't call RpProcessForm from RpAnalyzeHttpRequest
 *							if theObjectMimeType is eRpDataTypeFormMultipart.
 *		08/04/97	pjr		initialize fFinalBoundary, fItemName, and
 *							fItemValue in RpInitRequestStates.
 *		07/26/97	bva		rework RpBuildHttpResponseHeader for 1.1
 *		07/25/97	pjr		initialize fFileDone in RpInitRequestStates.
 *		07/21/97	bva		ip addresses move to connection
 *		07/15/97	pjr		initialize fRealmPtr in RpInitRequestStates.  use
 *							fChallengeNonce for a new digest challenge.
 *		07/14/97	bva		add OPTIONS and TRACE support
 *		07/12/97	bva		add HTTP->IPP interface
 *		07/03/97	bva		HTTP/1.1 change for redirection
 *		06/26/97	pjr		add form based file upload feature.
 *								add file system error responses.
 *		06/12/97	bva		add support for UrlState
 *		05/24/97	bva		rework URL dispatching
 * * * * Release 1.6 * * * *
 *		05/03/97	bva		fix BuildDateHeaders for standard time library
 *		04/18/97	pjr		add Security Digest feature
 *		04/13/97	bva		fix fHttpTransactionState usage
 *		04/04/97	bva		cleanup warnings
 *		03/31/97	bva		server push debugging
 *		03/10/97	bva		rework connection states
 *		02/22/97	bva		server push
 *		02/02/97	bva		fix refresh handling
 *		02/01/97	bva		refreshSeconds becomes Unsigned16
 *		01/31/97	bva		changed RpFindHttpObject for RpSearchRomFirst
 *		01/24/97	bva		cleaned up eRpHttpNeedAuthorization case
 *		01/24/97	rhb		removed unused variable
 *		01/20/97	bva		add kHttpNoCache to BuildDateHeaders
 *		12/27/96	bva		debug Keep-Alive
 *		12/11/96	bva		change initialization of fFormRequestLength
 *		12/06/96	bva		add Keep-Alive
 *		12/04/96	bva		change BuildContentType to use gMimeTypes
 * * * * Release 1.5 * * * *
 *		11/20/96	bva		fix refresh handling
 *		11/06/96	bva		add RpSetRefreshTime, RpSetRefreshPage callbacks
 *								add RpHandleUnknownUrl engine exit
 *		10/22/96	bva		use RpBuildQueryValues
 *		10/18/96	bva		consolidate HTTP headers and form items buffers
 *		10/14/96	bva		add plain text data type
 *		10/07/96	bva		use conditional compile flag with BuildHostName
 *		10/02/96	bva		provide alternate port support
 *		09/24/96	bva		add conditional compile flag for forms
 *		09/24/96	rhb		support dynamically allocated engine data
 *		09/22/96	bva		use Host name if provided for re-directs, refreshes
 * * * * Release 1.4 * * * *
 *		08/17/96	bva		fix redirect handling for gRpItemErrorPage,
 *							image maps don't generate queries on redirect
 *		08/06/96	bva		initialize gRpHttpRequestPtr in RomPagerInit
 * * * * Release 1.3 * * * *
 *		08/02/96	bva		add RomPagerContentTypeOnErrors,
 *								RomPagerContentLanguage compile flags
 *		07/30/96	bva		fix BuildRefresh bug introduced with page/form merge
 *		07/22/96	bva		pass index pointer to rpProcessDataFuncPtr call
 *		07/10/96	bva		make form redirects support nested queries
 *		07/05/96	bva		move security code to RpAccess.c,
 *							move request parsing to RpHttpRq.c
 *							merge page and form handling
 *		06/23/96	bva		add Location support and redirection for forms
 *								processing
 *		06/18/96	bva		merge fFirstItem and fNested to fItemState
 *		06/12/96	bva		add multiple realm support, rework for memory
 *								savings, report unfound forms properly
 *		06/09/96	bva		move line length checking in front of ParseLine,
 *								saving individual checks in parsed elements
 * * * * Release 1.2 * * * *
 *		06/01/96	bva		rework BuildDateHeaders,
 *								eliminate BuildLastModified
 *		05/31/96	bva		RpGetBrowserDate -> RpGetDateInSeconds,
 *							fix ParseBrowserDate for some year formats
 *		05/30/96	bva		cleanup to eliminate warnings
 * * * * Release 1.1 * * * *
 * * * * Release 1.0 * * * *
 *		03/30/96	bva		fStatic -> fDynamic
 *		03/15/96	bva		make mime type checking conditional,
 *							add applet mime type support
 *		03/11/96	bva		rework access checking
 *		03/09/96	bva		added access checking for forms
 *		02/17/96	bva		reworked date support to add Expires keyword
 *		02/03/96	bva		added image mapping support
 *		11/01/95	bva		created
 *
 *	To Do:
 */

#include "AsEngine.h"

#if RomPagerServer || RomPagerBasic

static void			BuildContentLength(rpHttpRequestPtr theRequestPtr,
						char *theResponsePtr);
static void			BuildContentType(rpHttpRequestPtr theRequestPtr,
						char *theResponsePtr);
static void			BuildDateHeaders(rpHttpRequestPtr theRequestPtr,
						char *theResponsePtr);

#if RomPagerHttpCookies
static void			RpBuildCookies(rpHttpRequestPtr theRequestPtr,
						char *theResponsePtr);
#endif
#if RomPagerKeepAlive
static void			BuildKeepAliveResponse(rpHttpRequestPtr theRequestPtr,
						char *theResponsePtr);
#endif

#if	RomPagerMimeTypeChecking
static Boolean		CheckAcceptType(rpHttpRequestPtr theRequestPtr,
						rpDataType theDataType);
#endif


/*
	Methods string

	This string is used to respond to the OPTIONS method request or
	bad command requests.
*/

static char gMethodsAllowed[] =	" GET, HEAD, POST"
#if RomPagerPutMethod
								", PUT"
#endif
#if RomPagerTraceMethod
								", TRACE"
#endif
#if RomPagerOptionsMethod
								", OPTIONS"
#endif
								"\x0d\x0a";


void RpInitRequestStates(rpDataPtr theDataPtr,
							rpHttpRequestPtr theRequestPtr) {
#if RomPagerDynamicRequestBlocks
	Boolean					theDynamicallyAllocatedFlag;
#endif
#if RomPagerFileSystem || RomPagerIpp || RomPagerUserExit || RomPlugControl
	rpObjectDescriptionPtr	theObjectPtr;
#endif
#if RomPagerSoftPages
	spRequestPtr			theSoftPageRequestPtr;
#endif

	/*
		Save the states we need,
		zero out the whole block,
		restore the saved states and
		init all the other values.
	*/

#if RomPagerDynamicRequestBlocks
	theDynamicallyAllocatedFlag = theRequestPtr->fDynamicallyAllocated;
#endif
#if RomPagerSoftPages
	theSoftPageRequestPtr = theRequestPtr->fSoftPageRequestPtr;
#endif

	RP_MEMSET(theRequestPtr, 0, sizeof(rpHttpRequest));

#if RomPagerDynamicRequestBlocks
	theRequestPtr->fDynamicallyAllocated = theDynamicallyAllocatedFlag;
#endif
#if RomPagerSoftPages
	theRequestPtr->fSoftPageRequestPtr = theSoftPageRequestPtr;
#endif
	theRequestPtr->fDataPtr = theDataPtr;
	theRequestPtr->fInUse = True;
	theRequestPtr->fHttpTransactionState = eRpParsingHeaders;
	theRequestPtr->fHttpCommand = eRpHttpNoCommand;
	theRequestPtr->fHttpResponseState = eRpHttpNormal;
	theRequestPtr->fNestedDepth = -1;
	theRequestPtr->fPostRequestLength = -1;
	theRequestPtr->fAcceptType = eRpDataTypeNone;
	theRequestPtr->fKeepAlive = False;
	theRequestPtr->fPersistent = False;
	theRequestPtr->fHaveRequestLine = False;

#if RomPagerServer
	/*
		Store the master request count in this request block and bump
		it for the next request.
	*/
	theRequestPtr->fSerial = theDataPtr->fSerial++;

	/*
		Set the request user dictionary to the master user dictionary.
	*/
	theRequestPtr->fUserPhrases = theDataPtr->fUserPhrases;
	theRequestPtr->fUserPhrasesCanBeCompressed =
			theDataPtr->fUserPhrasesCanBeCompressed;

#if AsSnmpAccess
	theRequestPtr->fSnmpInitialized = False;
#endif

#endif	/* RomPagerServer */

#if RpUserCookies && RomPagerUserDataAreas
	theRequestPtr->fUserCookie = (void *) &theRequestPtr->fUserDataArea[0];
#endif

#if RomPagerServerPush
	theRequestPtr->fServerPushActive = False;
#endif

#if RomPagerIpp || RomPagerPutMethod
	theRequestPtr->fIncomingBufferEnd = False;
#endif

#if RomPagerHttpOneDotOne
	theRequestPtr->fObjectIsChunked = False;
	theRequestPtr->fResponseIsChunked = False;
	theRequestPtr->fHaveChunk = False;
	theRequestPtr->fChunkState = eRpGettingChunkedLength;
	theRequestPtr->fHaveRequestObject = False;
	theRequestPtr->fHaveHost = False;
#endif

#if RpExpectHeader
	theRequestPtr->fWantsContinue = False;
#endif

#if RomPagerServer && RomPagerSecurity
	theRequestPtr->fPasswordState = eRpPasswordNotAuthorized;
#endif

#if RomPagerSecurityDigest
	theRequestPtr->fClientSupportsDigest = False;
	theRequestPtr->fNonceStale = False;
#endif

#if RpPageFlowDebug
	theRequestPtr->fDebugPageFlow = False;
#endif

#if RomPagerTraceMethod
	theRequestPtr->fTracing = False;
#endif

#if RomPagerFileSystem || RomPagerIpp || RomPagerUserExit || RomPlugControl
	theObjectPtr = &theRequestPtr->fLocalObject;
	theObjectPtr->fMimeDataType = eRpDataTypeHtml;
#endif

#if RomPagerServer && RomMailer
	theRequestPtr->fAggregate = False;
	theRequestPtr->fLastCharacter = kAscii_Null;
#endif

#if RomMailer
	theRequestPtr->fBase64State = eRpBase64_0;
#endif

#if RomPagerSoftPages
	theRequestPtr->fSoftPageFlag = False;
#endif

#if RpEtagHeader
	theRequestPtr->fClientHasObject = False;
#endif

#if RomPagerRanges
	theRequestPtr->fRangeBegin = -1;
	theRequestPtr->fRangeEnd = -1;
#endif
	return;
}


#if RomPagerUserExit

void RpHandleUserExit(rpConnectionPtr theConnectionPtr) {
	rpCgiPtr			theCgiPtr;
	rpHttpRequestPtr	theRequestPtr;

	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	theCgiPtr = &theRequestPtr->fCgi;
	RpExternalCgi(theRequestPtr->fDataPtr, theCgiPtr);
	if (theCgiPtr->fResponseState != eRpCgiPending) {
		theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
		/*
			We got a buffer back from the user routine.
			If it is the first time for a URL,
			see if he has handled the URL and set
			the object pointer to the local object to
			generate the headers.
		*/
		if (theRequestPtr->fHttpTransactionState != eRpSendingHttpResponse &&
			theRequestPtr->fHttpTransactionState != eRpHttpResponseComplete) {
			RpHandleCgiResponse(theRequestPtr);
		}
	}
	return;
}


void RpHandleCgiResponse(rpHttpRequestPtr theRequestPtr) {
	rpCgiPtr				theCgiPtr;
	rpObjectDescriptionPtr	theObjectPtr;

	/*
		We got a buffer back from the user routine.
		See if he has handled the URL, if not, just return.
	*/
	theCgiPtr = &theRequestPtr->fCgi;

#if RomPagerServer && RomPagerSecurity
	/*
		The CGI routine may have called RomPager's internal security
		routines and then decided that the page isn't protected after
		all, or that it doesn't exist.  In this case, we need to clear
		some states that may have been set up by RomPager's internal
		security routines in order to serve the CGI object or the
		object not found page.
	*/
	if (theCgiPtr->fHttpResponse == eRpCgiHttpOk ||
			theCgiPtr->fHttpResponse == eRpCgiHttpOkStatic ||
			theCgiPtr->fHttpResponse == eRpCgiHttpNotFound) {
		theRequestPtr->fHttpResponseState = eRpHttpNormal;
		theRequestPtr->fObjectPtr = (rpObjectDescriptionPtr) 0;
	}
#endif

	if (theCgiPtr->fHttpResponse != eRpCgiHttpNotFound) {
		/*
			The CGI routine handled the URL, so set up fObjectPtr
			(to get past the object not found logic) if it hasn't
			already been set up.  If the CGI routine called RomPager's
			internal security routines, they may have already set up
			the object pointer.
		*/
		if (theRequestPtr->fObjectPtr == (rpObjectDescriptionPtr) 0) {
			theRequestPtr->fObjectPtr = &theRequestPtr->fLocalObject;
		}
		theObjectPtr = theRequestPtr->fObjectPtr;

		switch (theCgiPtr->fHttpResponse) {

			case eRpCgiHttpIntServerErr:
				/*
					This is the same as normal object, 
					except we also set fHttpResponseState.
				*/
				theRequestPtr->fHttpResponseState = eRpHttpInternalServerError;
				/*
					Fall through.
				*/

			case eRpCgiHttpOk:
				/*
					Set up a normal dynamic object.
				*/
				theObjectPtr->fMimeDataType = theCgiPtr->fDataType;
				theObjectPtr->fCacheObjectType = eRpObjectTypeDynamic;
				if (theCgiPtr->fResponseState == eRpCgiLastBuffer) {
					theObjectPtr->fLength = theCgiPtr->fResponseBufferLength;
				}
#if RomPagerSoftPageProcessUserExit
				if (theObjectPtr->fMimeDataType == eRpDataTypeHtml) {
					theRequestPtr->fSoftPageFlag = True;
					PreprocessSoftPage(theRequestPtr->fDataPtr, theRequestPtr->
								fDataPtr->fCurrentConnectionPtr->fIndexValues);
				}
#endif
				break;

			case eRpCgiHttpOkStatic:
				/*
					Set up a normal static object.
				*/
				theObjectPtr->fMimeDataType = theCgiPtr->fDataType;
				theObjectPtr->fCacheObjectType = eRpObjectTypeStatic;
				if (theCgiPtr->fResponseState == eRpCgiLastBuffer) {
					theObjectPtr->fLength = theCgiPtr->fResponseBufferLength;
				}
				break;

			case eRpCgiHttpNotModified:
				theRequestPtr->fHttpResponseState = eRpHttpNotModified;
				break;

#if RomPagerSecurity
			case eRpCgiHttpUnauthorized:
				/*
					If the CGI routine has signalled that the request is not
					authorized, it must provide the realm name in the
					response buffer unless it's using RomPager's internal
					security.
				*/
				if (theRequestPtr->fHttpResponseState == eRpHttpNormal) {
					theRequestPtr->fHttpResponseState = eRpHttpNeedCgiAuthorization;
					RP_STRCPY(theRequestPtr->fUsername, theCgiPtr->fResponseBufferPtr);
				}
				/*
					else, if fHttpResponseState is not set to eRpHttpNormal, the
					user exit routine must be using RomPager's internal security
					which has already set up the fHttpResponseState for a basic
					or digest challenge and also set up the realm to be used for
					the challenge.
				*/

#if RomPagerBasic
				RpInternalCgi(theRequestPtr, gRpProtectedObjectText);
#else
				/*
					If the CGI routine is using RomPager's internal security
					routines, they may have already set up the response
					object.  So, only initialize the response object
					pointer if it isn't already set up.
				*/
				if (theRequestPtr->fObjectPtr == (rpObjectDescriptionPtr) 0) {
					theRequestPtr->fObjectPtr = &gRpAccessNotAllowedPage;
				}
#endif
				break;
#endif	/* RomPagerSecurity */

			case eRpCgiHttpRedirect:
				/*
					If the CGI routine has signalled a redirect, it must
					provide the path name of the object being pointed to in the
					response buffer.
				*/
				theRequestPtr->fHttpResponseState = eRpHttpRedirect;
				theRequestPtr->fObjectPtr->fURL = theCgiPtr->fResponseBufferPtr;
				break;

			default:
				break;
		}
	}
	return;
}

#endif	/* RomPagerUserExit */


RpErrorCode RpBuildReply(rpConnectionPtr theConnectionPtr) {
	rpHttpRequestPtr		theRequestPtr;
	RpErrorCode				theResult;
#if RomPagerUserExit
	rpCgiPtr				theCgiPtr;
	Unsigned16				theSendLength;
	char *					theSendPtr;
#endif
#if RomPagerRemoteHost
	rpConnectionPtr			theServerConnectionPtr;
#endif

	theResult = eRpNoError;
	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
#if RomPagerServerPush
	if (theRequestPtr->fItemState == eRpHtmlFirst &&
			theRequestPtr->fServerPushActive) {
		RpSendServerPushHeaders(theRequestPtr);
	}
#endif
#if RomPagerRanges
	if (theRequestPtr->fItemState == eRpHtmlFirst &&
			theRequestPtr->fObjectSource == eRpFileUrl &&
			theRequestPtr->fHaveRangeRequest) {
		RpSetFilePosition(theConnectionPtr);
		return;
	}
#endif
// +++ _Alphanetworks_Patch_, 10/08/2003, jacob_shih
#if defined(_jxWeb)
		_jxWeb_RpTrace("RpHttp.c", "RpBuildReply", "");
#endif
// --- _Alphanetworks_Patch_, 10/08/2003, jacob_shih
	switch (theRequestPtr->fObjectSource) {
#if RomPagerFileSystem
		case eRpFileUrl:
			theRequestPtr->fItemState = eRpHtmlNext;
			theConnectionPtr->fFileState = eRpFileReading;
			theResult = SfsReadFile(theConnectionPtr->fIndex,
										theRequestPtr->fHtmlFillPtr,
										theRequestPtr->fFillBufferAvailable);
			theConnectionPtr->fState = eRpConnectionWaitingFileRead;
			break;
#endif

#if RomPagerUserExit
		case eRpUserUrl:
			theCgiPtr = &theRequestPtr->fCgi;
			if (theCgiPtr->fResponseBufferLength > 0) {
				/*
					We have a buffer from the User exit function,
					so send it out.
				*/
				theSendPtr = theCgiPtr->fResponseBufferPtr;
				if (theCgiPtr->fResponseBufferLength >
									theRequestPtr->fFillBufferAvailable) {
					/*
						There is not enough room in the HTML buffer to send
						all of the data from the User Exit routine.  Send as
						much as we can now, the rest will be handled later.
					*/
					theSendLength = theRequestPtr->fFillBufferAvailable;
					theCgiPtr->fResponseBufferLength -= theSendLength;
					theCgiPtr->fResponseBufferPtr += theSendLength;
				}
				else {
					/*
						We are sending all of the buffer from the user exit
						routine.
					*/
					theSendLength = (Unsigned16) theCgiPtr->fResponseBufferLength;
					theCgiPtr->fResponseBufferLength = 0;

					/*
						See if it's the last buffer.
					*/
					if (theCgiPtr->fResponseState == eRpCgiLastBuffer) {
						theRequestPtr->fHttpTransactionState =
													eRpHttpResponseComplete;
					}
				}
				RP_MEMCPY(theRequestPtr->fHtmlFillPtr,
						theSendPtr, theSendLength);
				theRequestPtr->fFillBufferAvailable -= theSendLength;
#if RomPagerSoftPageProcessUserExit
				if (theRequestPtr->fSoftPageFlag) {
					/*
						The HTML object needs parsing.
					*/
					if (theCgiPtr->fResponseBufferLength > 0 ||
							theCgiPtr->fResponseState == eRpCgiLastBuffer) {
						/*
							There is not enough room in the parse buffer
							or the complete object has been received, make
							sure no more of the object is requested and
							start the parser.
						*/
						theRequestPtr->fHttpTransactionState =
													eRpHttpResponseComplete;
						RpInitializeParsingHtml(theConnectionPtr, (Boolean)
							(theCgiPtr->fResponseState == eRpCgiLastBuffer &&
								theCgiPtr->fResponseBufferLength == 0));
					}
					else {
						/*
							Bump the fill pointer for the next buffer.
						*/
						theRequestPtr->fHtmlFillPtr += theSendLength;
					}
				}
				else
#endif	/* RomPagerSoftPageProcessUserExit */
				{
					theRequestPtr->fHtmlFillPtr += theSendLength;

#if RpUserExitAggregate
					/*
						If we want to aggregate output buffers, see if
						we've filled the output buffer, or this is the
						last buffer, then send it out.
						Otherwise, send the buffer to the client as
						soon as we get it from the User Exit routine.
					*/
					if (theRequestPtr->fFillBufferAvailable == 0 ||
							theRequestPtr->fHttpTransactionState ==
									eRpHttpResponseComplete)
#endif
					{
						theRequestPtr->fHtmlBufferReady = False;
						RpFlipResponseBuffers(theRequestPtr);
						theResult = RpSendReplyBuffer(theConnectionPtr);
					}
				}
			}
			else {
				/*
					We have no data left in the buffer, see if it's the last buffer.
				*/
				if (theCgiPtr->fResponseState == eRpCgiLastBuffer) {
					theRequestPtr->fHttpTransactionState =
												eRpHttpResponseComplete;
					/*
						If there is data remaining to be sent,
						then send it out.
					*/
					if (theRequestPtr->fFillBufferAvailable != kHtmlMemorySize) {
#if RomPagerSoftPageProcessUserExit
						if (theRequestPtr->fSoftPageFlag) {
							RpInitializeParsingHtml(theConnectionPtr, (Boolean)
									(theCgiPtr->fResponseBufferLength == 0));
						}
						else
#endif
						{
							theRequestPtr->fHtmlBufferReady = False;
							RpFlipResponseBuffers(theRequestPtr);
							theResult = RpSendReplyBuffer(theConnectionPtr);
						}
					}
				}
				else {
					/*
						We need more data from the user exit routine.
					*/
					theConnectionPtr->fState = eRpConnectionWaitingUserExit;
				}
			}
			break;
#endif

#if RomPagerRemoteHost
		case eRpRemoteUrl:
			/*
				Suspend action on the Client connection while
				object data is read from the Remote Host.

				Set up the states to get object data from the Remote Host.
			*/
			theConnectionPtr->fState = eRpConnectionHolding;
			theServerConnectionPtr =
					theConnectionPtr->fRHRequestPtr->fServerConnectionPtr;
			theServerConnectionPtr->fState = eRpConnectionNeedsProtocolAction;

			break;
#endif

#if RomPagerServer
		case eRpRomUrl:
			/*
				normal ROM object URLs, internal URLs
			*/
			RpBuildHtmlPageReply(theRequestPtr);
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

#if RomPlugAdvanced
		case eRpUpnpSoapUrl:
		case eRpGenaUrl:
			/*
				Build the reply to a SOAP request
				or GENA event.
			*/
			theResult = RpSendReplyBuffer(theConnectionPtr);
			theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
			break;
#endif

#if RomPagerXmlServices
		case eRpXmlUrl:
			theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
			break;
#endif

		default:
			/*
				This shouldn't happen!  If it does, change the state to
				eRpHttpResponseComplete so we don't loop here forever.
			*/
#if RomPagerDebug
			RP_PRINTF("RpBuildReply, default!\n");
#endif
			theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
			break;
	}
	return theResult;
}


RpErrorCode RpSendReplyBuffer(rpConnectionPtr theConnectionPtr) {
	rpHttpRequestPtr	theRequestPtr;
	RpErrorCode			theResult;
	Unsigned16			theSendLength;
	Boolean             theSendStarted;

	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	theSendLength =	theRequestPtr->fHtmlResponseLength;
	theSendStarted = False;

	if (theSendLength != 0) {
// +++ _Alphanetworks_Patch_, 10/08/2003, jacob_shih
#if defined(_jxWeb)
		_jxWeb_TraceHttpResponseData();
		_jxWeb_RpTrace("RpHttp.c", "RpSendReplyBuffer", "call StcpSend()...");
#endif
// --- _Alphanetworks_Patch_, 10/08/2003, jacob_shih

#if RpHttpFlowDebug
		RP_PRINTF("Sending Object buffer: %d bytes\n", theSendLength);
#endif

#if RomPagerSecure
        if (theConnectionPtr->fIsTlsFlag) {
            Signed32     theSslResult;
            Unsigned32  theSslSendLength;

            theSslSendLength = theSendLength;
            theSslResult = SSLSend(theConnectionPtr->fSslContext, theRequestPtr->fHtmlResponsePtr,
                    theSslSendLength);
			theSslSendLength = theSslResult;
            if (theSslResult == SSL_WOULD_BLOCK) {
				   theResult = eRpNoError;
					theConnectionPtr->fPendingSendBuffer = theRequestPtr->fHtmlResponsePtr;
					theConnectionPtr->fPendingSendBufferLen = theSendLength;
					theConnectionPtr->fState = eRpConnectionProtocolWaitExternal;
			}else if(theSslResult == SSL_ERROR) {
                theResult = eRpTcpSendError;
                theSendStarted = True;
#if AsDebug
                RP_PRINTF("RpSendReplyBuffer, send error, theSslResult = %d\n",
                        theSslResult);
#endif
			} else 	{
				if(theSslSendLength == theSendLength) {
					theResult = eRpNoError;
					theSendStarted = True;
					theConnectionPtr->fPendingSendBuffer = NULL;
					theConnectionPtr->fPendingSendBufferLen = 0;
#if AsDebug
                    RP_PRINTF("RpSendReplyBuffer, complete send: "
                            "theSendLength = %d\n", theSendLength);
#endif
				} else {
					theResult = eRpNoError;
					theSendStarted = True;
					theConnectionPtr->fPendingSendBuffer = theRequestPtr->fHtmlResponsePtr + theSslSendLength; 
					theConnectionPtr->fPendingSendBufferLen = theSendLength - theSslSendLength; 
#if AsDebug
                    RP_PRINTF("RpSendReplyBuffer, partial send: "
                            "theSendLength = %d, theSslSendLength = %d\n",
                            theSendLength , theSslSendLength);
#endif
				}
			}
        }
        else {
            theResult = StcpSend(theConnectionPtr->fIndex,
                                theRequestPtr->fHtmlResponsePtr,
                                theSendLength);
            theSendStarted = True;
        }

#else   /* RomPagerSecure */

        theResult = StcpSend(theConnectionPtr->fIndex,
                            theRequestPtr->fHtmlResponsePtr,
                            theSendLength);
        theSendStarted = True;
#endif  /* RomPagerSecure */
        if (theSendStarted) {
            if (theResult == eRpNoError) {
                theConnectionPtr->fState = eRpConnectionSendingReply;
            }
            else {
                theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
            }
        }
    }
    else {
        theResult = eRpTcpSendError;
    }
    return theResult;
}


void RpAnalyzeHttpRequest(rpHttpRequestPtr theRequestPtr) {
	rpDataPtr				theDataPtr;
	rpDataType				theObjectMimeType;
	rpObjectDescriptionPtr	theObjectPtr;
#if RomPagerServer
	rpObjectExtensionPtr	theExtensionPtr;
	rpProcessDataFuncPtr	theFunctionPtr;
#endif
#if RomPagerForms
	rpObjectFlags			theObjectFlags;
#endif

	theDataPtr = theRequestPtr->fDataPtr;
	theObjectPtr = theRequestPtr->fObjectPtr;
	theObjectMimeType = theObjectPtr->fMimeDataType;

#if RomPagerImageMapping
	if (theObjectMimeType == eRpDataTypeMap) {
		/*
			We have valid access to an image map.  Image maps process the
			data and then trigger a request to serve up a page.  As a default,
			we set up to serve the page stored in our object descriptor. If a
			valid map location is found it will change the page to be served.
		*/
		theRequestPtr->fCurrentFormDescriptionPtr = theObjectPtr;
		theRequestPtr->fObjectPtr = theObjectPtr->fExtensionPtr->fPagePtr;
		/*
			Go figure out the location that was sent in.
		*/
		RpProcessImageMap(theDataPtr);
		theRequestPtr->fHttpResponseState = eRpHttpRedirectMap;
		return;		/* just bail out here */
	}
#endif	/* RomPagerImageMapping */

#if 0

	/*
		Moved to BuildContentLength.
	*/
#if RomPagerServer
	if (theObjectPtr->fCacheObjectType == eRpObjectTypeDynamicClose) {
		/*
			If we have a dynamic object that cannot use chunked
			encoding, then we have to close the connection to signal
			the length.
		*/
		theRequestPtr->fPersistent = False;
	}
#endif
#endif

#if RomPagerForms
	if (theObjectMimeType == eRpDataTypeForm ||
			theObjectMimeType == eRpDataTypeFormGet ||
			theObjectMimeType == eRpDataTypeFormMultipart) {
		/*
			We have valid access to a form.  Forms process the data and then
			trigger a request to serve up a page. As a default, we set up to
			serve the page stored in our object descriptor. In some cases,
			the forms processing may change the page to be served.
		*/
		theRequestPtr->fCurrentFormDescriptionPtr = theObjectPtr;
		theRequestPtr->fObjectPtr = theObjectPtr->fExtensionPtr->fPagePtr;

		/*
			Go handle all the form elements and set the variables.
			If this is an image map, go figure out the location.
		*/
		RpProcessForm(theDataPtr);

		/*
			Get the flags from the form object.
		*/
		theObjectFlags = theObjectPtr->fExtensionPtr->fFlags;

		if (theObjectFlags & kRpObjFlag_Direct ||
			theRequestPtr->fItemError != eAsItemError_NoError) {
			/*
				If kRpObjFlag_Direct is set, we want to serve the next page
				directly.  Most of the time this will be used with a Get-Query
				request.  Various browsers handle history and Reload functions
				for GET-Query and POST requests in different ways.  In general,
				using redirection leaves the history log in the browsers in
				a cleaner state.

				Since we are going to serve the page directly, set up the
				dereferenced pointers again, since forms processing may have
				changed them.
			*/
			theObjectPtr = theRequestPtr->fObjectPtr;
#if	RomPagerMimeTypeChecking
			theObjectMimeType = theObjectPtr->fMimeDataType;
#endif
		}
		else {
			/*
				Set the state so we will respond with a redirect to the
				next page to be served.
			*/
			theRequestPtr->fHttpResponseState = eRpHttpRedirect;
#if RomPagerSecure
			if (theObjectFlags & kRpObjFlag_ForceHttps) {
				/*
					Use https if the flags specify "force https".
				*/
				theRequestPtr->fRedirectScheme = eRpSchemeHttps;
			}
			else if (theObjectFlags & kRpObjFlag_ForceHttp) {
				/*
					Use http if the flags specify "force http".
				*/
				theRequestPtr->fRedirectScheme = eRpSchemeHttp;
			}
#endif
			return;		/* just bail out here */
		}
	}
#endif	/* RomPagerForms */
	/*
		We have valid access to a page, image, or applet.
	*/

#if	RomPagerMimeTypeChecking
	/*
		Is it the right type?
	*/
	if (!CheckAcceptType(theRequestPtr, theObjectMimeType)){
		/*
			the page doesn't match the request type
		*/
		theRequestPtr->fHttpResponseState = eRpHttpBadContentType;

		return;		/* just bail out here */
	}
#endif

#if !RpNoCache
#if RpEtagHeader
	/*
		We have the right content type, check for an Etag match.
	*/
	if (theObjectPtr->fCacheObjectType == eRpObjectTypeStatic &&
			theRequestPtr->fClientHasObject) {
		/*
			The page is static and the Etag that the browser has matches
			our ROM Etag, so just notify the browser that the page has
			not been modified.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpNotModified;
		return;		/* just bail out here */
	}
#endif

	/*
		We have the right content type, check the date stamp.
	*/
	if (theObjectPtr->fCacheObjectType == eRpObjectTypeStatic &&
			theRequestPtr->fBrowserDate == theDataPtr->fRomSeconds) {
		/*
			The page is static and the date that the browser has matches
			our rom date, so just notify the browser that the page has
			not been modified.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpNotModified;
		return;		/* just bail out here */
	}
#if RomPagerFileSystem
	if (theRequestPtr->fObjectSource == eRpFileUrl &&
			theRequestPtr->fBrowserDate ==
					theRequestPtr->fFileInfoPtr->fFileDate) {

		/*
			The page has been served from the file system already without
			a date change, so just notify the browser that the page has
			not been modified.
		*/
		theRequestPtr->fHttpResponseState = eRpHttpNotModified;
		return;		/* just bail out here */
	}
#endif	/* RomPagerFileSystem */
#endif	/* !RpNoCache */

#if RomPagerServer
	/*
		Everything is good, so set up the refresh parameters, and
		do any initial page processing.
	*/
	theExtensionPtr = theObjectPtr->fExtensionPtr;
#if RomPagerClientPull
	if (theExtensionPtr == (rpObjectExtensionPtr) 0) {
		theRequestPtr->fRefreshSeconds = 0;
		theRequestPtr->fRefreshObjectPtr = (rpObjectDescriptionPtr) 0;
	}
	else {
		theRequestPtr->fRefreshSeconds = theExtensionPtr->fRefreshSeconds;
		theRequestPtr->fRefreshObjectPtr = theExtensionPtr->fRefreshPagePtr;
		theFunctionPtr = theExtensionPtr->fProcessDataFuncPtr;
		if (theFunctionPtr != (rpProcessDataFuncPtr) 0) {
			theFunctionPtr(theDataPtr,
					theDataPtr->fCurrentConnectionPtr->fIndexValues);
		}
	}
#else
	if (theExtensionPtr != (rpObjectExtensionPtr) 0) {
		theFunctionPtr = theExtensionPtr->fProcessDataFuncPtr;
		if (theFunctionPtr != (rpProcessDataFuncPtr) 0) {
			theFunctionPtr(theDataPtr,
					theDataPtr->fCurrentConnectionPtr->fIndexValues);
		}
	}
#endif	/* RomPagerClientPull */
#endif	/* RomPagerServer */

	return;
}

#if	RomPagerMimeTypeChecking
static Boolean CheckAcceptType(rpHttpRequestPtr theRequestPtr,
			rpDataType theDataType) {
	rpDataType theAcceptType;
	theAcceptType = theRequestPtr->fAcceptType;
	/*
		We assume HTML is always the right type.
	*/
	if (theDataType == eRpDataTypeHtml) {
		return True;
	}
	/*
		We assume plain text is always the right type.
	*/
	if (theDataType == eRpDataTypeText) {
		return True;
	}
	/*
		If no Accept statement was received, then we assume everything
		is acceptable.
	*/
	if (theAcceptType == eRpDataTypeNone) {
		return True;
	}
	/*
		If the Accept statement specified 'asterisk/asterisk',
		then we can send anything.
	*/
	if (theAcceptType == eRpDataTypeAll) {
		return True;
	}
	/*
		If the Accept statement specified multiple images,
		we will send any image.
	*/
	if (theAcceptType == eRpDataTypeAnyImage) {
		if (theDataType == eRpDataTypeImageGif ||
				theDataType == eRpDataTypeImageJpeg ||
				theDataType == eRpDataTypeImagePict ||
				theDataType == eRpDataTypeImageTiff ||
				theDataType == eRpDataTypeImagePng) {
			return True;
		}
	}
	/*
		Well, shucks!  This must be a picky browser.
		Signal a rejection.
	*/
	return False;
}
#endif


void RpBuildHttpResponseHeader(rpHttpRequestPtr theRequestPtr) {
	char * 					theResponsePtr;
	rpHttpResponseAction	theResponseState;
	char *					theSchemePtr;
#if RomPagerLogging || RpEtagHeader || RomPagerSecure
	rpDataPtr				theDataPtr = theRequestPtr->fDataPtr;
#endif
#if RomPagerLogging && (kRpLoggingLevel == 1)
	void *					theEventInfoPtr;
#endif
#if RomPagerServer && RomPagerSecurity
	rpRealmPtr				theRealmPtr = theRequestPtr->fRealmPtr;
#endif
#if RomPagerRanges
	Unsigned32				theLength;
#endif

	theResponseState = theRequestPtr->fHttpResponseState;

#if RomPagerLogging
#if (kRpLoggingLevel == 1)
	/*
		Log all HTTP events.
	*/
	switch (theResponseState) {
#if RomPagerSecurity
		case eRpHttpNeedBasicAuthorization:
#if RomPagerSecurityDigest
		case eRpHttpNeedDigestAuthorization:
#endif	/* RomPagerSecurityDigest */
			theEventInfoPtr = (void *) theRealmPtr;
			break;
#endif	/* RomPagerSecurity */

		default:
			theEventInfoPtr = (void *) theRequestPtr->fObjectPtr;
			break;
	}

	RpLogHttpEvent(theDataPtr, theResponseState,
					theRequestPtr->fObjectSource, theEventInfoPtr);
#else
	/*
		Log only normal responses here.
	*/
	if (theResponseState == eRpHttpNormal) {
		RpLogHttpEvent(theDataPtr,
						theResponseState,
						theRequestPtr->fObjectSource,
						(void *) theRequestPtr->fObjectPtr);
	}
#endif	/* kRpLoggingLevel == 1 */
#endif	/* RomPagerLogging */

	/*
		Now build the HTTP response
	*/
	theResponsePtr = theRequestPtr->fHttpWorkBuffer;
	RP_STRCPY(theResponsePtr, kHttpVersion);
	/*
		Build the initial headers
	*/
	switch (theResponseState) {

		case eRpHttpBadCommand:
			/*
				we didn't find a good command
			*/
			RP_STRCAT(theResponsePtr, kMethodNotAllowed);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			RP_STRCAT(theResponsePtr, kHttpAllow);
			RP_STRCAT(theResponsePtr, gMethodsAllowed);
			break;

		case eRpHttpBadRequest:
			/*
				the request is screwed up
			*/
			RP_STRCAT(theResponsePtr, kBadRequest);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			break;

#if RpTcnTest
		case eRpHttpMultipleChoices:
			/*
				There are multiple responses possible
			*/
			RP_STRCAT(theResponsePtr, kMultipleChoices);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			RP_STRCAT(theResponsePtr, kTCNTest);
			RP_STRCAT(theResponsePtr, kTCNTest1);
			RP_STRCAT(theResponsePtr, kTCNTest2);
			RP_STRCAT(theResponsePtr, kTCNTest3);
			break;
#endif

#if RomPagerHttpOneDotOne
		case eRpHttpNotImplemented:
			/*
				the request asks for an unimplemented feature
			*/
			RP_STRCAT(theResponsePtr, kNotImplemented);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			break;
#endif

#if RpExpectHeader
		case eRpHttpExpectFailed:
			/*
				the request asks for an unimplemented feature
			*/
			RP_STRCAT(theResponsePtr, kExpectFailed);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			break;
#endif

#if RomPagerRanges
		case eRpHttpPartialContent:
			/*
				We're going to send a range
			*/
			RP_STRCAT(theResponsePtr, kPartial);
			RP_STRCAT(theResponsePtr, kHttpContentRange);
			RpCatUnsigned32ToString((Unsigned32) theRequestPtr->fRangeBegin, 
					theResponsePtr, 0);
			RP_STRCAT(theResponsePtr, kHyphen);
			RpCatUnsigned32ToString((Unsigned32) theRequestPtr->fRangeEnd, 
					theResponsePtr, 0);
			RP_STRCAT(theResponsePtr, kSlash);
			RpCatUnsigned32ToString(theRequestPtr->fObjectPtr->fLength, 
					theResponsePtr, 0);
			RP_STRCAT(theResponsePtr, kCRLF);
			BuildContentType(theRequestPtr, theResponsePtr);
			BuildDateHeaders(theRequestPtr, theResponsePtr);
			BuildContentLength(theRequestPtr, theResponsePtr);
			break;

		case eRpHttpBadRange:
			/*
				the request asks for an unimplemented feature
			*/
			RP_STRCAT(theResponsePtr, kBadRange);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			theLength = theRequestPtr->fObjectPtr->fLength;
			if (theLength) {
				RP_STRCAT(theResponsePtr, kHttpContentRange);
				RP_STRCAT(theResponsePtr, kHttpContentRangeError);
				RpCatUnsigned32ToString(theLength, theResponsePtr, 0);
				RP_STRCAT(theResponsePtr, kCRLF);
			}
			break;
#endif

#if RomPlugAdvanced || RomPlugControl
		case eRpHttpPreconditionFailed:
			/*
				the UPnP request was flawed
			*/
			RP_STRCAT(theResponsePtr, kPreconditionFailed);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			break;
#endif
#if RomPlugControl
		case eRpHttpGenaProcessed:
			/*
				Build the headers for event processing complete.
			*/
			RP_STRCAT(theResponsePtr, kPageFound);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			break;
#endif

#if RomPagerPutMethod
		case eRpHttpPutCompleted:
			/*
				The PUT request completed successfully.
			*/
			RP_STRCAT(theResponsePtr, kCreated);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			break;
#endif
		case eRpHttpForbidden:
			/*
				the request collides with another users request or realm
			*/
			RP_STRCAT(theResponsePtr, kForbidden);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			break;

		case eRpHttpRequestTooLarge:
			/*
				the request object was too large.
			*/
#if RomPagerHttpOneDotOne
			RP_STRCAT(theResponsePtr, kRequestTooLarge);
#else
			RP_STRCAT(theResponsePtr, kBadRequest);
#endif
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			break;

#if RomPagerOptionsMethod
		case eRpHttpOptions:

			/*
				the request is for HTTP Options
			*/
			RP_STRCAT(theResponsePtr, kPageFound);
			RP_STRCAT(theResponsePtr, kHttpAllow);
			RP_STRCAT(theResponsePtr, gMethodsAllowed);
			break;
#endif

#if RomPagerTraceMethod
		case eRpHttpTracing:

			/*
				the request is for HTTP tracing data
			*/
			RP_STRCAT(theResponsePtr, kPageFound);
			RP_STRCAT(theResponsePtr, kHttpContentType);
			RP_STRCAT(theResponsePtr, kTypeHttpTrace);
			RP_STRCAT(theResponsePtr, kCRLF);
			BuildDateHeaders(theRequestPtr, theResponsePtr);
			RP_STRCAT(theResponsePtr, kHttpContentLength);
			RpCatSigned32ToString(theRequestPtr->fTraceLength, theResponsePtr);
			RP_STRCAT(theResponsePtr, kCRLF);
			break;
#endif

#if RomPlugAdvanced
		case eRpHttpSoapProcessed:
			/*
				A SOAP request was handled without error.
			*/
		case eRpHttpSoapError:
			/*
				An error result was returned for the SOAP request processing.
			*/
			theRequestPtr->fSoapRequest.fBuildResponseHeaderFunc(theRequestPtr,
					theResponsePtr);
			BuildContentLength(theRequestPtr, theResponsePtr);
			break;

		case eRpUpnpSubscription:
		case eRpUpnpUnsubscription:
			/*
				the request is a UPNP subscription request
			*/
			RP_STRCAT(theResponsePtr, kPageFound);
			if (theResponseState == eRpUpnpSubscription) {
				BuildDateHeaders(theRequestPtr, theResponsePtr);
				RuBuildSubscriptionHeaders(theRequestPtr, theResponsePtr);
			}
			break;
#endif

#if RomPagerSecurity

#if RomPagerUserExit
		case eRpHttpNeedCgiAuthorization:
			/*
				the request is protected and has not been authorized,
				so request authentication from the browser
			*/
			RP_STRCAT(theResponsePtr, kUnauthorized);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			RP_STRCAT(theResponsePtr, kHttpWWWAuthenticate);
			RP_STRCAT(theResponsePtr, theRequestPtr->fUsername);
			RP_STRCAT(theResponsePtr, kQuoteCRLF);
			break;
#endif	/* RomPagerUserExit */

#if RomPagerServer
		case eRpHttpNeedBasicAuthorization:
			/*
				the request is protected and has not been authorized,
				so request authentication from the browser
			*/
			RP_STRCAT(theResponsePtr, kUnauthorized);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			RP_STRCAT(theResponsePtr, kHttpWWWAuthenticate);
			RP_STRCAT(theResponsePtr, theRealmPtr->fRealmName);
			RP_STRCAT(theResponsePtr, kQuoteCRLF);
			break;

#if RomPagerSecurityDigest
		case eRpHttpNeedDigestAuthorization:
			/*
				the request is protected and has not been authorized,
				so request authentication from the browser
			*/
			RP_STRCAT(theResponsePtr, kUnauthorized);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			RP_STRCAT(theResponsePtr, kHttpWWWAuthenticateDigest);
			RP_STRCAT(theResponsePtr, theRealmPtr->fRealmName);
			RP_STRCAT(theResponsePtr, kHttpNonce);

			if (theRealmPtr->fNonce[0] != '\0') {
				RP_STRCAT(theResponsePtr, theRealmPtr->fNonce);
			}
			else if (theRealmPtr->fChallengeNonce[0] != '\0') {
				RP_STRCAT(theResponsePtr, theRealmPtr->fChallengeNonce);
			}
			else {
				RpGenerateNonce(theRequestPtr, theRealmPtr->fChallengeNonce);
				RP_STRCAT(theResponsePtr, theRealmPtr->fChallengeNonce);
			}

			if (theRequestPtr->fNonceStale == True) {
				RP_STRCAT(theResponsePtr, kHttpStaleTrue);
			}
			else {
				RP_STRCAT(theResponsePtr, kQuote);
			}

			RP_STRCAT(theResponsePtr, kHttpQopAuth);
			RP_STRCAT(theResponsePtr, kCRLF);
			break;
#endif	/* RomPagerSecurityDigest */

#endif	/* RomPagerServer */

#endif	/* RomPagerSecurity */


		case eRpHttpNoObjectFound:
			/*
				good command and request, but no page.
			*/
			RP_STRCAT(theResponsePtr, kNoPageFound);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
#if RomPagerServer
			/*
				Set up the error URL if it hasn't already been done.
			*/
			if (*theRequestPtr->fErrorPath == '\0') {
				RP_STRCPY(theRequestPtr->fErrorPath, theRequestPtr->fPath);
			}
#endif
			break;


#if RomPagerFileSystem || RomPlugAdvanced
		case eRpHttpInternalServerError:
			/*
				there has been some kind of file system error.
			*/
			RP_STRCAT(theResponsePtr, kServerError);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			break;
#endif	/* RomPagerFileSystem */


#if	RomPagerMimeTypeChecking
		case eRpHttpBadContentType:
			/*
				we have the page, but it doesn't match the request type
			*/
			RP_STRCAT(theResponsePtr, kNoneAcceptable);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			BuildContentType(theRequestPtr, theResponsePtr);
			break;
#endif

		case eRpHttpNotModified:
			/*
				The page is static and the date that the browser has
				matches our rom date, so just notify the browser that
				the page has not been modified.
			*/

			RP_STRCAT(theResponsePtr, kNotModified);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
#if RpEtagHeader
			if (theRequestPtr->fHttpVersion == eRpHttpOneDotOne) {
				RP_STRCAT(theResponsePtr, kHttpEtag);
				RP_STRCAT(theResponsePtr, theDataPtr->fRomEtagString);
				RP_STRCAT(theResponsePtr, kQuoteCRLF);
			}
#endif
			break;

		case eRpHttpRedirect:
		case eRpHttpRedirectMap:
			/*
				We just a processed a form with a POST command or an
				image map with a GET query.  The page to be served is a
				response to the request.  We want to tell the browser the
				URI of the page so that it associates the correct URI for
				subsequent browser commands (like Reload and Back).
			*/
#if RomPagerHttpOneDotOne
			if (theRequestPtr->fHttpVersion == eRpHttpOneDotZero) {
				RP_STRCAT(theResponsePtr, kMoved);
			}
			else {
				RP_STRCAT(theResponsePtr, kSeeOther);
			}
#else
			RP_STRCAT(theResponsePtr, kMoved);
#endif
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif
			/*
				build Location Header
			*/
			RP_STRCAT(theResponsePtr, kHttpLocation);
			
			/*
				Now figure out which scheme to use on the redirect
			*/
#if RomPagerSecure
			if (theRequestPtr->fRedirectScheme == eRpSchemeUnknown && 
					theDataPtr->fCurrentConnectionPtr->fIsTlsFlag) {
				/*
					If the connection is TLS and the scheme is not forced,
					use https.
				*/
					theRequestPtr->fRedirectScheme = eRpSchemeHttps;
			}
#endif
			if (theRequestPtr->fRedirectScheme == eRpSchemeHttps) {
				theSchemePtr = kHttpsString;
			}
			else {
				theSchemePtr = kHttpString;
			}

			/*
				Add the scheme to the Location header.
			*/
			RP_STRCAT(theResponsePtr, theSchemePtr);

			/*
				Add the hostname to the Location header.
			*/
			RpBuildHostName(theRequestPtr, theResponsePtr);

#if RomPagerUrlState
			if (theRequestPtr->fUrlState[0] != '\0') {
				RP_STRCAT(theResponsePtr, kUrlStatePrefix);
				RP_STRCAT(theResponsePtr, theRequestPtr->fUrlState);
			}
#endif
			RP_STRCAT(theResponsePtr, theRequestPtr->fObjectPtr->fURL);
#if RomPagerQueryIndex
			/*
				If the form was processed with an indexed query, and this
				redirect is aimed at our host, make the redirect a query also.
			*/
			if (theResponseState == eRpHttpRedirect) {
				RpBuildQueryValues(theRequestPtr, theResponsePtr, True);
			}
#endif
			RP_STRCAT(theResponsePtr, kCRLF);
			break;

#if RomPlugAdvanced || RomPagerXmlServices
		case eRpHttpXmlError:
            /*
                HTTP 1.1 500 Internal Server Error
				without explanation
            */
            RP_STRCAT(theResponsePtr, kServerError);
			break;

		case eRpHttpXmlOk:
            /*
                HTTP 1.1 200 OK
				without explanation
            */
            RP_STRCAT(theResponsePtr, kPageFound);
			break;

		case eRpHttpXmlNormal:
            RP_STRCAT(theResponsePtr, kPageFound);
            BuildContentType(theRequestPtr, theResponsePtr);
            BuildContentLength(theRequestPtr, theResponsePtr);
			break;

#endif /* RomPlugAdvanced || RomPagerXmlServices */

		case eRpHttpNormal:
		case eRpHttpIppNormal:
		default:

			/*
				Build the headers for the normal case
			*/
			RP_STRCAT(theResponsePtr, kPageFound);
#if RpHttpFlowDebug
			RP_PRINTF("Response: %s", theResponsePtr);
#endif

			/*
				build the basic Headers
			*/
			BuildContentType(theRequestPtr, theResponsePtr);
			BuildDateHeaders(theRequestPtr, theResponsePtr);

#if RomPagerSecurityDigest
			if (theRealmPtr != (rpRealmPtr) 0) {
				if (theRealmPtr->fSecurityLevel & kRpSecurityNewNonce) {
					/*
						generate the next nonce
					*/
					RpGenerateNonce(theRequestPtr, theRealmPtr->fNonce);

					RP_STRCAT(theResponsePtr, kHttpNextNonce);
					RP_STRCAT(theResponsePtr, theRealmPtr->fNonce);
					RP_STRCAT(theResponsePtr, kQuoteCRLF);
				}
			}
#endif

#if RomPagerClientPull
			RpBuildRefresh(theRequestPtr, theResponsePtr);
#endif
			BuildContentLength(theRequestPtr, theResponsePtr);
			break;
	}

	/*
		Build some more headers and set up the transaction state
	*/
	switch (theResponseState) {

		case eRpHttpRedirect:
		case eRpHttpRedirectMap:
		case eRpHttpNotModified:
		case eRpHttpPutCompleted:
#if RomPagerKeepAlive
			BuildKeepAliveResponse(theRequestPtr, theResponsePtr);
#endif
			/*
				There is no object for this response.
			*/
			RP_STRCAT(theResponsePtr, kHttpContentLength);
			RP_STRCAT(theResponsePtr, kEmptyLength);
			theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
			break;

		case eRpHttpBadCommand:
		case eRpHttpBadRequest:
		case eRpHttpNotImplemented:
		case eRpHttpExpectFailed:
		case eRpHttpPreconditionFailed:
		case eRpHttpBadRange:
		case eRpHttpBadContentType:
#if !RomPagerHttpOneDotOne && !RomPagerDebug
		case eRpHttpRequestTooLarge:
#endif
#if RomPagerHttpOneDotOne || RomPagerKeepAlive
			/*
				For the above error messages, mark the connection to close 
				after we send the notification.
				
				This will cause us to skip over any other headers for this 
				request and any other requests that might be in the pipeline.
			*/
			theRequestPtr->fPersistent = False;
#endif
			/*
				There is no object for this response.
			*/
			RP_STRCAT(theResponsePtr, kHttpContentLength);
			RP_STRCAT(theResponsePtr, kEmptyLength);
			theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
			break;

#if RomPlugAdvanced
		case eRpUpnpSubscription:
		case eRpUpnpUnsubscription:
#endif
#if RomPagerOptionsMethod
		case eRpHttpOptions:
#endif
		default:
			/*
				There is no object for this response.
			*/
			RP_STRCAT(theResponsePtr, kHttpContentLength);
			RP_STRCAT(theResponsePtr, kEmptyLength);
			theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
			break;

#if RomPagerHttpOneDotOne || RomPagerDebug
		case eRpHttpRequestTooLarge:
#endif
		case eRpHttpForbidden:
		case eRpHttpNeedBasicAuthorization:
#if RomPagerUserExit
		case eRpHttpNeedCgiAuthorization:
#endif
		case eRpHttpNeedDigestAuthorization:
		case eRpHttpNoObjectFound:
		case eRpHttpInternalServerError:
		case eRpHttpMultipleChoices:
			/*
				There is an explanation object for the error response.
			*/
			RP_STRCAT(theResponsePtr, kHttpContentType);
			RP_STRCAT(theResponsePtr, kTypeHtml);
			RP_STRCAT(theResponsePtr, kCRLF);
#if RomPagerHttpOneDotOne
			if (theRequestPtr->fHttpVersion == eRpHttpOneDotOne) {
				RP_STRCAT(theResponsePtr, kHttpTransferEncodingChunked);
				theRequestPtr->fResponseIsChunked = True;
				theRequestPtr->fPersistent = False;
			}
#endif
			/*
				Set source for internal error messages.
			*/
#if RomPagerBasic
			theRequestPtr->fObjectSource = eRpUserUrl;
#else
			/*
				The object for 500 Server Error comes from the application if the 
				source was User Exit, so don't necessarily change the source.
			*/
			if (theRequestPtr->fObjectSource != eRpUserUrl || 
					theResponseState != eRpHttpInternalServerError) {
				theRequestPtr->fObjectSource = eRpRomUrl;
			}
#if RomPagerSoftPages
			theRequestPtr->fSoftPageFlag = False;
#endif
#endif	/* RomPagerBasic */
			theRequestPtr->fHttpTransactionState = eRpSendingHttpResponse;
			break;

		case eRpHttpNormal:
#if RomPagerRanges
		case eRpHttpPartialContent:
#endif
			if (theRequestPtr->fHttpCommand == eRpHttpHeadCommand ||
					theRequestPtr->fHttpCommand == eRpHttpOptionsCommand) {
				/*
					If the command is "HEAD" or "OPTIONS" all the client wants
					are the headers.
				*/
				theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
			}
			else {
				theRequestPtr->fHttpTransactionState = eRpSendingHttpResponse;
			}
			break;

#if RomPagerTraceMethod
		case eRpHttpTracing:
			theRequestPtr->fHttpTransactionState = eRpSendingTraceResponse;
			break;
#endif

#if RomPagerIpp
		case eRpHttpIppNormal:
			theRequestPtr->fHttpTransactionState = eRpParsingIpp;
			break;
#endif

#if RomPlugAdvanced || RomPagerXmlServices
		case eRpHttpXmlOk:
		case eRpHttpXmlError:
            theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
            break;

		case eRpHttpXmlNormal:
            theRequestPtr->fHttpTransactionState = eRpHandleXmlRequest;
			break;
#endif

#if RomPlugAdvanced
		case eRpHttpSoapProcessed:
		case eRpHttpSoapError:
			theRequestPtr->fHttpTransactionState = eRpSendingHttpResponse;
			break;

#endif

#if RomPlugControl
		case eRpHttpGenaProcessed:
			theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete;
			break;
#endif
	}

	/*
		Build the last headers and set up HTML buffer
	*/
	RP_STRCAT(theResponsePtr, kServerHeader);
#if RomPagerHttpCookies
	/*
		build Set-Cookie Header(s)
	*/
	RpBuildCookies(theRequestPtr, theResponsePtr);
#endif
#if RomPagerHttpOneDotOne
	if (theRequestPtr->fHttpVersion == eRpHttpOneDotOne &&
			!theRequestPtr->fPersistent) {
		/*
			If we have a HTTP 1.1 request, but for whatever
			reason (usually an error message) we are not going
			to keep the connection open, signal that we will
			close it.
		*/
		RP_STRCAT(theResponsePtr, kHttpConnection);
		RP_STRCAT(theResponsePtr, kHttpClose);
		RP_STRCAT(theResponsePtr, kCRLF);
	}
#endif
	/*
		Signal end of headers
	*/
	RP_STRCAT(theResponsePtr, kCRLF);

#if RomPagerServerPush
	if (theRequestPtr->fServerPushActive) {
		/*
			send out the boundary for initial data, to
			tell the browser to wait for more.
		*/
		RP_STRCAT(theResponsePtr, kTwoDashes);
		RP_STRCAT(theResponsePtr, theRequestPtr->fSeparator);
		RP_STRCAT(theResponsePtr, kCRLF);
	}
#endif
	RpInitializeHtmlPageReply(theRequestPtr);
	return;
}


static void BuildContentType(rpHttpRequestPtr theRequestPtr,
								char *theResponsePtr) {
	Boolean					theContentTypeIsComplete = False;
	rpDataType				theMimeType;
	rpObjectDescriptionPtr	theObjectPtr;
#if RomPagerServerPush || RomPagerContentDisposition
	rpObjectFlags			theObjectFlags;
	rpObjectExtensionPtr	theExtensionPtr;
#endif

	theObjectPtr = theRequestPtr->fObjectPtr;
	theMimeType = theObjectPtr->fMimeDataType;
	RP_STRCAT(theResponsePtr, kHttpContentType);

#if RomPagerServerPush
	theExtensionPtr = theObjectPtr->fExtensionPtr;
	if (theExtensionPtr != (rpObjectExtensionPtr) 0) {
		theObjectFlags = theExtensionPtr->fFlags;
		if (theObjectFlags & kRpObjFlag_ServerPush) {
			/*
				set up the first time states for server push
			*/
			RP_STRCAT(theResponsePtr, kTypeServerPush);
			RpBuildSeparator(theRequestPtr->fDataPtr,
								theRequestPtr->fSeparator);
			theRequestPtr->fServerPushActive = True;
			theRequestPtr->fServerPushSeconds =
					theExtensionPtr->fRefreshSeconds;
			theRequestPtr->fServerPushObjectPtr =
					theExtensionPtr->fRefreshPagePtr;
			RP_STRCAT(theResponsePtr, theRequestPtr->fSeparator);
			RP_STRCAT(theResponsePtr, kCRLF);
			theContentTypeIsComplete = True;
		}
	}
#endif	/* RomPagerServerPush */

#if RomPagerRemoteHost
	if (!theContentTypeIsComplete && theMimeType == eRpDataTypeOther) {
		if (theRequestPtr->fObjectSource == eRpRemoteUrl) {
			RP_STRCAT(theResponsePtr, theRequestPtr->fOtherMimeType);
			RP_STRCAT(theResponsePtr, kCRLF);
			theContentTypeIsComplete = True;
		}
	}
#endif

#if RomPagerFileSystem
	if (!theContentTypeIsComplete && theMimeType == eRpDataTypeOther) {
		RP_STRCAT(theResponsePtr, theRequestPtr->fFileInfoPtr->fOtherMimeType);
		RP_STRCAT(theResponsePtr, kCRLF);
		theContentTypeIsComplete = True;
	}
#endif

	if (!theContentTypeIsComplete) {
		RP_STRCAT(theResponsePtr, gMimeTypes[theMimeType]);
		RP_STRCAT(theResponsePtr, kCRLF);
	}

#if 0
	/*
		If this header isn't sent, the browsers assume that content
		is English.  Additionally, the notification to the browser of
		the character set in use may be accomplished by using META tags
		in the HTML document, without requiring HTTP headers.
	*/
	RP_STRCAT(theResponsePtr, kHttpContentLanguage);
	RP_STRCAT(theResponsePtr, kEnglish);
#endif

#if RomPagerContentDisposition
	theExtensionPtr = theObjectPtr->fExtensionPtr;
	if (theExtensionPtr != (rpObjectExtensionPtr) 0) {
		theObjectFlags = theExtensionPtr->fFlags;
		if (theObjectFlags & kRpObjFlag_Disposition) {
			/*
				We need to send the Content-Disposition header
			*/
			RP_STRCAT(theResponsePtr, kHttpContentDisposition);
			RP_STRCAT(theResponsePtr, theObjectPtr->fURL);
			RP_STRCAT(theResponsePtr, kQuoteCRLF);
		}
	}
#endif
	return;
}


static void BuildDateHeaders(rpHttpRequestPtr theRequestPtr,
								char *theResponsePtr) {
	Unsigned32			theCurrentTime;
	rpDataPtr			theDataPtr;
	char				theDateString[80];
#if !RpNoCache
	rpObjectSource		theObjectSource;
#endif

	theDataPtr = theRequestPtr->fDataPtr;

	/*
		For HTTP 1.0 requests, the "Date","Expires" and "Last-Modified"
		headers are created to obtain optimal caching from various browsers
		with different caching algorithms. The HTTP 1.1 specifications
		more clearly define caching behavior, so the headers we create
		follow the specifications.

		Rom Objects	"Date"

		For HTTP 1.0 requests, we report the current time as best we can.
		If we have calendar support, this will be pretty accurate.  If not, we
		look to see if the browser sent us a date in an "If-Modified-Since"
		header.  If it did, we create a current date based on the browser
		date + 1 second, otherwise, we create the date based on the rom date
		plus the time since the system was booted.

		For HTTP 1.1 requests, if we have a calendar date, we report it in
		the "Date" header.  Otherwise, we don't report a date and assume that
		the browser or a proxy along the way will fill in the value.

		Rom Objects	"Expires" and "Last-Modified"

		For static ROM objects, we use the ROM date for the
		"Last-Modified" header and do not send an "Expires" header.

		For dynamic ROM objects we send "Expires" header with an old
		date and a "Last-Modified" header with the current date.  For
		HTTP 1.0 requests, we also send the "Pragma: no-cache" header.
		For HTTP 1.1 requests, we send the "Cache-Control: no-cache"
		header, and only send the "Expires" and "Last-Modified" headers
		if we have calendar time.

		Remote Host Objects

		For remote host objects, we take the "Date", "Expires", "Pragma"
		and "Last-Modified" headers we get from the remote host and send
		them to the client.

		File System Objects

		For file system objects, we assume they are static and use the date
		from the file system for the "Last-Modified" header.

		User Exit (CGI) Objects

		User Exit objects can be either static or dynamic.  Dynamic objects
		are treated the same as dynamic ROM objects.  Static objects use the
		date from the Cgi control block for the "Last-Modified" header.
	*/

#if RomPagerCalendarTime
	theCurrentTime = RpGetSysTimeInSeconds(theDataPtr);
#else
	if (theRequestPtr->fBrowserDate != 0) {
		theCurrentTime = theRequestPtr->fBrowserDate + 1;
	}
	else {

		/*
			System time is seconds since system came up.
		*/
		theCurrentTime = RpGetSysTimeInSeconds(theDataPtr);
		theCurrentTime += theDataPtr->fRomSeconds;
	}
#endif
	RpBuildDateString(theDataPtr, theDateString, theCurrentTime);

	/*
		Build the Date header for non-Remote Host objects.
	*/
	if (theRequestPtr->fObjectSource != eRpRemoteUrl) {
#if RomPagerHttpOneDotOne
		if (theRequestPtr->fHttpVersion == eRpHttpOneDotOne) {
#if RomPagerCalendarTime
			RP_STRCAT(theResponsePtr, kHttpDate);
			RP_STRCAT(theResponsePtr, theDateString);
#endif
		}
		else {
			RP_STRCAT(theResponsePtr, kHttpDate);
			RP_STRCAT(theResponsePtr, theDateString);
		}
#else	/* RomPagerHttpOneDotOne */
		RP_STRCAT(theResponsePtr, kHttpDate);
		RP_STRCAT(theResponsePtr, theDateString);
#endif	/* RomPagerHttpOneDotOne */
	}

	/*
		Build the caching headers
	*/

#if RomPlugAdvanced
	/*
		No cache headers for subscription responses,
		just the Date header.
	*/
	if (theRequestPtr->fObjectSource == eRpUpnpUrl) {
		return;
	}
#endif

#if RpNoCache
	/*
		For debugging purposes force all objects to be refreshed.  This
		allows changing GIFs and other objects during the development
		process without having to flush browser caches.
	*/

#if RomPagerHttpOneDotOne
	if (theRequestPtr->fHttpVersion == eRpHttpOneDotOne) {
		RP_STRCAT(theResponsePtr, kHttpOneOneNoCache);
		RP_STRCAT(theResponsePtr, kHttpExpires);
		RP_STRCAT(theResponsePtr, theDataPtr->fExpiresDateString);
	}
	else {
#endif	/* RomPagerHttpOneDotOne */
		RP_STRCAT(theResponsePtr, kHttpExpires);
		RP_STRCAT(theResponsePtr, theDataPtr->fExpiresDateString);
		RP_STRCAT(theResponsePtr, kHttpLastModified);
		RP_STRCAT(theResponsePtr, theDateString);
		RP_STRCAT(theResponsePtr, kHttpNoCache);
#if RomPagerHttpOneDotOne
	}
#endif

#else	/* RpNoCache */
	/*
		For normal production environments we want to signal the browser
		with the correct caching info for each object type.
	*/

	theObjectSource = theRequestPtr->fObjectSource;

#if RomPagerRemoteHost
	if (theObjectSource == eRpRemoteUrl) {
		/*
			For remote objects we use the string that was built from
			the remote host response.
		*/
		if (*theRequestPtr->fRemoteDateStringPtr != '\0') {
			RP_STRCAT(theResponsePtr,
					theRequestPtr->fRemoteDateStringPtr);
		}
		return;
	}
#endif

	if (theRequestPtr->fObjectPtr->fCacheObjectType == eRpObjectTypeStatic) {
		/*
			We have a static object, so send out the caching headers.
		*/
		RP_STRCAT(theResponsePtr, kHttpLastModified);
		switch (theObjectSource) {
#if RomPagerFileSystem
			case eRpFileUrl:
				RpBuildDateString(theDataPtr, theDateString,
						theRequestPtr->fFileInfoPtr->fFileDate);
				RP_STRCAT(theResponsePtr, theDateString);
				break;
#endif
#if RomPagerUserExit
			case eRpUserUrl:
				RpBuildDateString(theDataPtr, theDateString,
						theRequestPtr->fCgi.fObjectDate);
				RP_STRCAT(theResponsePtr, theDateString);
				break;
#endif
			case eRpRomUrl:
			default:
				RP_STRCAT(theResponsePtr, theDataPtr->fRomDateString);
#if RpEtagHeader
				if (theRequestPtr->fHttpVersion == eRpHttpOneDotOne) {
					RP_STRCAT(theResponsePtr, kHttpEtag);
					RP_STRCAT(theResponsePtr, theDataPtr->fRomEtagString);
					RP_STRCAT(theResponsePtr, kQuoteCRLF);
				}
#endif
				break;
		}
	}
	else {
		/*
			We have a dynamic object, so force the browser to refresh it.
		*/
#if RomPagerHttpOneDotOne
		if (theRequestPtr->fHttpVersion == eRpHttpOneDotOne) {
			RP_STRCAT(theResponsePtr, kHttpOneOneNoCache);
			RP_STRCAT(theResponsePtr, kHttpExpires);
			RP_STRCAT(theResponsePtr, theDataPtr->fExpiresDateString);
		}
		else {
#endif	/* RomPagerHttpOneDotOne */
			RP_STRCAT(theResponsePtr, kHttpExpires);
			RP_STRCAT(theResponsePtr, theDataPtr->fExpiresDateString);
			RP_STRCAT(theResponsePtr, kHttpLastModified);
			RP_STRCAT(theResponsePtr, theDateString);
			RP_STRCAT(theResponsePtr, kHttpNoCache);
#if RomPagerHttpOneDotOne
		}
#endif
	}

#endif	/* RpNoCache */
	return;
}

static void BuildContentLength(rpHttpRequestPtr theRequestPtr,
							char *theResponsePtr) {
	Unsigned32		theLength;

#if RomPagerServerPush
	/*
		If we're in server push mode then don't
		send the Content-Length header with the
		first headers.  It will only stop and
		confuse the browser.
	*/
	if (theRequestPtr->fServerPushActive) {
		return;
	}
#endif

#if RomPagerSoftPages
	/*
		If we're sending a Soft Page then don't
		send the Content-Length header with the
		first headers.  It will be wrong and
		confuse the browser.
	*/
	if (theRequestPtr->fSoftPageFlag) {
		theRequestPtr->fObjectPtr->fLength = 0;
	}
#endif

	theLength = theRequestPtr->fObjectPtr->fLength;

	/*
		Send out Content-Length header, if we know the length.
		Otherwise, omit the header and the browser will figure
		it out after we close the connection.
	*/
	if (theLength) {
		RP_STRCAT(theResponsePtr, kHttpContentLength);
		RpCatUnsigned32ToString(theLength, theResponsePtr, 0);
		RP_STRCAT(theResponsePtr, kCRLF);
#if RomPagerKeepAlive
		/*
			For HTTP 1.0 Keep-Alive, we can only keep a persistent
			connection for objects that we know the Content-Length.
		*/
		BuildKeepAliveResponse(theRequestPtr, theResponsePtr);
#endif
	}
	else {
		/*
			We don't know the length of the object, but we still
			need to signal the browser that we have sent all the
			data.
			
			HTTP 1.1 specifies the ability to use chunked encoding
			to signal the length of data. As it turns out, there 
			are some "HTTP 1.1" implementations used in certain 
			UPnP gateway environments that don't support chunked 
			encoding, so we use an object type to mark dynamic 
			objects that cannot use chunked encoding.
			
			For these objects or for HTTP 1.0 requests, we signal
			the length of data by closing the connection after
			transmission.
		*/
#if RomPagerHttpOneDotOne
		if (theRequestPtr->fHttpVersion == eRpHttpOneDotOne &&
				theRequestPtr->fObjectPtr->fCacheObjectType !=
				eRpObjectTypeDynamicClose) {
			theRequestPtr->fResponseIsChunked = True;
			RP_STRCAT(theResponsePtr, kHttpTransferEncodingChunked);
		}
		else {
			theRequestPtr->fPersistent = False;
		}
#else
		theRequestPtr->fPersistent = False;
#endif
	}
	return;
}


#if RomPagerHttpCookies
static void RpBuildCookies(rpHttpRequestPtr theRequestPtr,
							char *theResponsePtr) {
	Unsigned8		theIndex;

	for (theIndex = 0; theIndex < kNumberOfHttpCookies; theIndex++) {
		if (theRequestPtr->fNewHttpCookie[theIndex] &&
				*theRequestPtr->fHttpCookies[theIndex] != '\0') {
			RP_STRCAT(theResponsePtr, kHttpSetCookie);
			RpCatUnsigned32ToString(theIndex, theResponsePtr, 0);
			RP_STRCAT(theResponsePtr, kEqual);
			RP_STRCAT(theResponsePtr, theRequestPtr->fHttpCookies[theIndex]);
			RP_STRCAT(theResponsePtr, kCookiePath);
			RP_STRCAT(theResponsePtr, kCRLF);
		}
	}
	return;
}
#endif


#if RomPagerKeepAlive
static void BuildKeepAliveResponse(rpHttpRequestPtr theRequestPtr,
							char *theResponsePtr) {

	/*
		Did the browser request to keep the connection open?
	*/
	if (theRequestPtr->fKeepAlive && theRequestPtr->fPersistent) {
		/*
			Send the keep-alive headers.
		*/
		RP_STRCAT(theResponsePtr, kHttpConnection);
		RP_STRCAT(theResponsePtr, kHttpKeepAlive);
		RP_STRCAT(theResponsePtr, kCRLF);
	}
	return;
}
#endif	/* RomPagerKeepAlive */


void RpBuildHostName(rpHttpRequestPtr theRequestPtr,
		char *theResponsePtr) {
#if RomPagerFullHostName
	/*
		For HTTP 1.1 requests and most modern HTTP 1.0 requests,
		we will have been provided the host name with the "Host"
		header.  If we haven't, we create one using the local
		IP address.
	*/
	if (*(theRequestPtr->fHost) != '\0') {
		RP_STRCAT(theResponsePtr, theRequestPtr->fHost);
	}
	else {
		RpBuildIpAddressHostName(theRequestPtr->fDataPtr->fCurrentConnectionPtr,
				theResponsePtr);
	}
#else
	RP_STRCAT(theResponsePtr, theRequestPtr->fHost);
#endif	/* RomPagerFullHostName */
	return;
}

#if RomPagerQueryIndex

/*
	RpBuildQueryValues

	Build a query string ("?,i,j,k") from the values of the indices
	in 1-relative format, so that we can pass them from page to page.
*/

void RpBuildQueryValues(rpHttpRequestPtr theRequestPtr,
		char *theStringPtr, Boolean theForUrlFlag) {
	rpConnectionPtr		theConnectionPtr;
	Signed8				theDepth;
	char				theIndexString[2];
	Signed16Ptr			theValuePtr;

	theConnectionPtr = theRequestPtr->fDataPtr->fCurrentConnectionPtr;

	if (theConnectionPtr->fIndexDepth != -1) {
		if (theForUrlFlag) {
			theIndexString[0] = kAscii_Question;
		}
		else {
			theIndexString[0] = kRpIndexCharacter;
		}
		theIndexString[1] = '\0';
		RP_STRCAT(theStringPtr, theIndexString);
		theValuePtr = theConnectionPtr->fIndexValues;
		theDepth = theConnectionPtr->fIndexDepth;
		while (theDepth >= 0) {
			RpCatSigned32ToString((Signed32) (*theValuePtr++) + 1, theStringPtr);
			if (theDepth > 0) {
				RP_STRCAT(theStringPtr, kComma);
			}
			theDepth--;
		}
	}
	return;
}

#endif

#endif	/* RomPagerServer || RomPagerBasic */
