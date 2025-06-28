/*
 *	File:		RpStates.h
 *
 *	Contains:	Internal Embedded Web Server States
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Copyright:	© 1995-2003 by Allegro Software Development Corporation
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
 *		08/07/03	bva		add eRpHttpBadRange, eRpHttpPartialContent
 *		07/13/03	bva		add rpScheme
 *      06/20/03    bva     eRpSoapUrl -> eRpUpnpSoapUrl
 *		06/09/03	nam		add RomPagerXmlServices support
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 * * * * Release 4.20  * * *
 *      10/07/02    bva     add eRpHttpMPostCommand
 *		09/04/02	pjr		move rpPasswordState to RpCallBk.h
 * * * * Release 4.12  * * *
 * * * * Release 4.00  * * *
 *		06/29/01	amp		add eRpHandleGena, eRpSoapUrl and eRpGenaUrl
 *		02/15/01	bva		remove dictionary patching
 *		09/21/00	bva		eRpHttpFileSystemError -> eRpHttpInternalServerError,
 *							add eRpUpnpSubscription, eRpUpnpUrl
 *		08/31/00	bva		add eRpHttpPreconditionFailed, eRpHttpSubscribeCommand,
 *							eRpHttpUnsubscribeCommand
 *		06/27/00	rhb		add eRpHttpIppGettingResponse
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 *		03/09/99	pjr		moved rpHttpChunkState to AsStates.h
 *		02/16/99	bva		moved some definitions to AsStates.h
 *		12/30/98	bva		derived from RomPager.h
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_RPSTATES_
#define	_RPSTATES_


/* 
	RomPager HTTP commands 
*/
typedef enum {
	eRpHttpNoCommand,
	eRpHttpGetCommand,
	eRpHttpHeadCommand,
	eRpHttpPostCommand,
	eRpHttpMPostCommand,
	eRpHttpOptionsCommand,
	eRpHttpPutCommand,
	eRpHttpDeleteCommand,
	eRpHttpTraceCommand,
	eRpHttpNotifyCommand,
	eRpHttpSubscribeCommand,
	eRpHttpUnsubscribeCommand
} rpHttpCommand;


/*
	character hex escape states 
*/
typedef enum {
	eRpHexEscapeState_LookForPercent,
	eRpHexEscapeState_GetFirstHexByte,
	eRpHexEscapeState_GetSecondHexByte
} rpHexEscapeState;


/* 
 	HTML item states 
*/
typedef enum {
	eRpHtmlFirst,
	eRpHtmlNext
} rpHtmlState;


/* 
	RomPager HTTP response actions 
*/
typedef enum {
	eRpHttpNormal,
	eRpHttpBadCommand,
	eRpHttpBadRequest,
	eRpHttpForbidden,
	eRpHttpNotImplemented,
	eRpHttpNeedBasicAuthorization,
	eRpHttpNeedDigestAuthorization,
	eRpHttpNeedCgiAuthorization,
	eRpHttpNoObjectFound,
	eRpHttpMultipleChoices,
	eRpHttpInternalServerError,
	eRpHttpBadContentType,
	eRpHttpExpectFailed,
	eRpHttpRedirect,
	eRpHttpRedirectMap,
	eRpHttpNotModified,
	eRpHttpRequestTooLarge,
	eRpHttpPreconditionFailed,
	eRpHttpBadRange,
	eRpHttpPartialContent,
	eRpHttpTracing,
	eRpHttpOptions,
	eRpHttpIppNormal,
	eRpUpnpSubscription,
	eRpUpnpUnsubscription,
	eRpHttpPutCompleted,
	eRpHttpFormProcessed,
#if RomPlugAdvanced || RomPagerXmlServices
	eRpHttpXmlOk,
	eRpHttpXmlNormal,
	eRpHttpXmlError,
#endif
	eRpHttpSoapProcessed,
	eRpHttpSoapError,
	eRpHttpSoapPending,
	eRpHttpGenaProcessed,
	eRpHttpGenaError
} rpHttpResponseAction;


/*
	HTTP request states 
*/
typedef enum {
	eRpParsingHeaders,
	eRpParsingObjectBody,
	eRpParsingChunkedObjectBody,
	eRpParsingMultipart,
	eRpParsingIpp,
	eRpParsingHtml,
	eRpUnParseable,
	eRpFindUrl,
	eRpEndUrlSearch,
	eRpEndSecurityCheck,
	eRpAnalyzeHttpRequest,
	eRpSendingHttpHeaders,
	eRpSendingHttpContinue,
	eRpHandlePut,
#if RomPlugControl
	eRpHandleGena,
#endif
#if RomPlugAdvanced || RomPagerXmlServices
	eRpHandleXmlRequest,
#endif
	eRpSendingHttpResponse,
#if RpFileInsertItem
	eRpOpenInsertFileItem,
	eRpOpeningInsertFileItem,
	eRpReadingInsertFileItem,
	eRpClosingInsertFileItem,
#endif
	eRpSendingTraceResponse,
	eRpSendingLastDataBuffer,
	eRpServerPushStartTimer,
	eRpServerPushTimer,
	eRpHttpResponseComplete,
	eRpHttpResponseComplete2
} rpHttpTransactionState;


/*
	Multipart Form Data states 
*/
typedef enum {
	eRpMpFindBoundary,
	eRpMpParsingHeaders,
	eRpMpParsingItemValue,
	eRpMpCreateFile,
	eRpMpCreateFileDone,
	eRpMpParsingData,
	eRpMpWriteFileDone,
	eRpMpCloseFile,
	eRpMpCloseFileDone,
	eRpMpBadRequest,
	eRpMpFileSystemError,
	eRpMpConnectionError,
	eRpMpFlush
} rpMultipartState;


/*
	HTTP -> IPP states 
*/
typedef enum {
	eRpHttpIppParserPending,
	eRpHttpIppSetupRequestBuffer,
	eRpHttpIppHaveRequestBuffer,
	eRpHttpIppSendingHeaders,
	eRpHttpIppSendingResponse,
	eRpHttpIppGettingResponse
} rpHttpIppState;


#if RomPlugAdvanced || RomPagerXmlServices
/*
    HTTP XML Services states
*/
typedef enum {
	eRpHttpXmlRequestReceived,
	eRpHttpXmlParsingRequest,
	eRpHttpXmlCommandSetup,
	eRpHttpXmlCommandPending,	
	eRpHttpXmlWaitingForAsyncReply,
	eRpHttpXmlSendHeaders,
	eRpHttpXmlSendData,
	eRpHttpXmlSendWait
} rpHttpXmlState;
#endif
/*
	HTTP PUT command states 
*/
typedef enum {
	eRpHttpPutStart,
	eRpHttpPutCreateFileDone,
	eRpHttpPutFileSystemError,
	eRpHttpPutParsingData,
	eRpHttpPutWriteFileDone,
	eRpHttpPutCloseFile,
	eRpHttpPutCloseFileDone,
	eRpHttpPutConnectionError,
	eRpHttpPutFlush
} rpHttpPutState;


/*
	HTTP request versions
*/
typedef enum {
	eRpHttpOneDotZero,
	eRpHttpOneDotOne
} rpHttpVersion;


/* 
 	HTTP Object Sources 
*/
typedef enum {
	eRpRomUrl,
	eRpFileUrl,
	eRpFileUploadUrl,
	eRpIppUrl,
	eRpPutUrl,
	eRpRemoteUrl,
	eRpUpnpUrl,
	eRpUpnpSoapUrl,
	eRpGenaUrl,
	eRpXmlUrl,
	eRpXmlSoapUrl,
	eRpXmlRpcUrl,
	eRpUserUrl
} rpObjectSource;


/*
	URL schemes
*/
typedef enum {
	eRpSchemeUnknown,
	eRpSchemeHttp,
	eRpSchemeHttps
} rpScheme;

#if RomPagerRemoteHost

/*
	Remote request states
*/
typedef enum {
	eRpRemoteRequestFree,
	eRpRemoteRequestNeedsConnection,
	eRpRemoteRequestHasConnection
} rpRemoteRequestState;


/*
	Remote Host transaction states
*/
typedef enum {
	eRpRemoteHostWaitingForConnection,
	eRpRemoteHostSendRequest,
	eRpRemoteHostSendData,
	eRpRemoteHostIssueFirstReceive,
	eRpRemoteHostParsingHeaders,
	eRpRemoteHostAnalyzeResponse,
	eRpRemoteHostReceiveObject,
	eRpRemoteHostIssueReceive,
	eRpRemoteHostObjectComplete,
	eRpRemoteHostNoObjectFound
} rpRemoteHostState;

#endif /* RomPagerRemoteHost */

#endif	/* _RPSTATES_ */
