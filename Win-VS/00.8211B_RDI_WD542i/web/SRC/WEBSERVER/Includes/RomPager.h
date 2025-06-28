/*
 *	File:		RomPager.h
 *
 *	Contains:	Internal Embedded Web Server Definitions
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
 *		09/19/03	pjr		add fAuthorizationCount to rpRHRequest structure
 *		09/03/03	pjr		change fErrorPath size to kMaxSaveHeaderLength
 *		08/25/03	rhb		hide security structures unless needed
 *		08/13/03	bva		add kUrlStateRelativePrefix
 *		08/06/03	bva		add Range support to rpHttpRequest structure
 *		07/13/03	bva		add fRedirectScheme to rpHttpRequest structure
 *		06/20/03	bva		add XML services support to rpHttpRequest structure
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 * * * * Release 4.20  * * *
 *		02/21/03	pjr		add separate username and password max lengths
 *		02/05/03	bva		add RomPagerServer conditional to RomPagerSecurity
 *		12/12/02	pjr		add fChallengeTimeout to rpRealm structure, remove
 *							fChallengeObjectPtr
 *		10/04/02	amp		use larger fMultipartWorkBuffer for RomPagerSecure
 *		09/04/02	pjr		change security session timers to Signed32
 *		08/09/02	rhb		move Phase Dictionary declarations to AsVarAcc.h
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *		07/25/02	bva		add RomPagerCaptureSoapAction
 * * * * Release 4.10  * * *
 *		06/08/02	bva		remove RpUserExitMulti
 * * * * Release 4.07  * * *
 * * * * Release 4.04  * * *
 *		11/16/01	bva		bump fBufferItemLength to Unsigned32
 * * * * Release 4.03  * * *
 *		09/26/01	amp		RmCharsetOption needs Html Response Buffers
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		06/29/01	amp		add kHttpPatternNTS and kHttpPatternSeq
 *		06/29/01	amp		add fSoapRequest to fHttpRequest
 *		02/16/01	rhb		rename rpSnmpAccessItem to asSnmpAccessItem and 
 *								define AsSnmpAccess items if just RomPagerServer
 *		02/15/01	bva		remove dictionary patching
 *		02/14/01	rhb		rename rpItemError to asItemError
 *		02/13/01	rhb		rename RomPagerSnmpAccess to AsSnmpAccess and
 *								kRpSnmpGetNextCount to kAsSnmpGetNextCount
 *		01/23/01	pjr		increase size of fCnonce field in HTTP request
 *		01/05/01	pjr		add parameters for buffer display item support
 *		12/18/00	pjr		move common HTTP definitions to AsHttp.h
 *		11/07/00	pjr		add fDataPtr to rpHttpRequest structure
 *		10/29/00	pjr		add kHttpCookie
 *		09/14/00	bva		move some character constants to AsEngine.h
 *		09/08/00	pjr		fix compile errors
 *		08/31/00	bva		add upnp support
 *		08/31/00	pjr		modify for the new security model
 *		08/09/00	rhb		support RpHtmlFormImage
 *		06/29/00	pjr		add support for multiple IPP printer names
 *		06/08/00	pjr		remove fPutRequestLength from rpHttpRequest
 *		05/26/00	bva		remove fDataPtr from rpHttpRequest
 *		04/24/00	rhb		add fDataType to rpHttpRequest
 *		04/05/00	pjr		add HTTP cookie support to Web Client
 *		03/30/00	pjr		add digest authentication to Web Client
 *		02/06/00	bva		move product family definitions to AsEngine.h
 *		01/28/00	bva		change RpSendDataOutExtendedAscii prototype
 *		01/19/00	bva		add multiple server port support
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 *		01/17/00	bva		fix fHttpCookies array 
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 *		01/13/99	amp		add RomPopBasic support
 *		01/11/99	amp		add RomMailerBasic support
 *		12/28/99	rhb		add prototype for RpStrLenCpyTruncate()
 * * * * Release 3.10  * * *
 * * * * Release 3.06  * * *
 *		12/06/99	bva		fHttpTempLineBuffer uses kMaxParseLength 
 *								instead of kMaxLineLength
 *		11/29/99	rhb		add pointer to next item to request control block
 * * * * Release 3.05  * * *
 *		09/19/99	bva		kHttpSeparator -> kFieldSeparator
 *		08/23/99	rhb		add RomXML support
 *		07/21/99	rhb		add fIppResponseStarted
 * * * * Release 3.04  * * *
 * * * * Release 3.03  * * *
 *		07/08/99	bva		add kHttpPatternServer
 *		07/01/99	pjr		change conditional for RpHexToString
 *		07/01/99	bva		add kOpenAngle, kAscii_OpenAngle, kAscii_CloseAngle
 *		06/23/99	bva		make fHttpPatternTable conditional
 * * * * Release 3.02  * * *
 *		05/17/99	bva		change length of fFilename in rpHttpRequest
 *		05/17/99	pjr		add RpFreeRequestControlBlock prototype
 *		05/06/99	pjr		enable RpHexToNibble for RomWebClient
 *		05/03/99	pjr		WcHttpFiniteStateMachine now returns error code
 * * * * Release 3.01  * * *
 *		04/19/99	pjr		add support for digest security "qop=auth" header
 *		04/16/99	rhb		RmSmtpInit(), PrRomPopInit(), & WcWebClientInit()
 *								return results add Soft Page Requests, and add 
 *								Soft Page Init & DeInit Prototypes
 * * * * Release 3.0 * * * *
 *		03/30/99	rhb		add prototype for RpGetDynamicDisplayIndex and
 *								fSwitchValue & fSwitchCount to rpHttpRequest
 *		03/22/99	pjr		add prototypes for RpConvertUnsigned32ToHex and
 *								WcHandlePersistentAvailable
 *		03/18/99	pjr		remove duplicate IppInit prototype
 *		03/08/99	bva		add RpStrLenCpy
 *		02/27/99	bva		add HTTP Cookie support
 *		02/27/99	bva		more rework global data allocation
 *		02/26/99	bva		change connection setup definitions
 *		02/22/99	bva		rework global data allocation
 *		02/19/99	bva		add RpFindTokenDelimitedPtr
 *		02/16/99	bva		split RpStates.h
 *		02/14/99	bva		make fIndexValues conditional
 *		02/06/99	bva		RpExtern.h -> AsExtern.h
 *		02/03/99	pjr		add RpSoftPg.c prototypes
 *		01/31/99	bva		add fUserPhrases to rpHttpRequest
 *		01/29/99	bva		add Etag support
 *		01/22/99	pjr		include stdio.h if RomPagerDebug
 *		01/20/99	pjr		change RpHexToNibble prototype
 *		01/19/99	pjr		only include fItemPtr and fItemError in the request
 *								structure for RomPagerServer
 *		01/15/99	pjr		rework rpData, rpConnection, rpHttpRequest
 *		01/06/99	pjr		add fInitFlags, move RpHandleExternalSecurity
 *							prototype to RpAccess.c, move data and time routine
 *							prototypes to RpDate.c.
 *		01/06/99	rhb		RpSoftSnmpAccess() now returns void 
 *		12/30/98	bva		added WcClient.h, moved state enums to RpStates.h
 *		12/28/98	pjr		add RpHttpDy.c prototypes
 *		12/29/98	bva		merge with WebClient
 *		12/28/98	bva		add Custom varible access prototypes
 *		12/15/98	bva		add request serial number
 *		12/11/98	rhb		merge Soft Pages 
 * * * * Release 2.2 * * * *
 *		11/30/98	bva		fix compile warnings
 *		11/25/98	rhb		fix multi buffer bug with eRpItemType_File
 *		11/17/98	pjr		add fStartUpTime to rpData structure
 *		11/17/98	bva		add theDataPtr to SpwdGetExternalPassword
 *		11/15/98	bva		RpGetRomTimeInSeconds -> RpGetMonthDayYearInSeconds
 *		11/14/98	bva		add fExpiresDateString to rpData structure
 *		11/12/98	rhb		give a type to fDataPtr in the request structure
 *		11/10/98	rhb		implement eRpItemType_File and make 
 *								SendDataOutZeroTerminated public
 *		11/03/98	pjr		RomPagerPutMethod needs fHttpContentType
 *		10/23/98	bva		move rpPop3Request rpPop3PatternTable to PrRomPop.h
 *		10/23/98	pjr		remove fParsingControl from Remote Host request
 *							block, eliminate RpHandleWaitingForRHReceive
 *		10/22/98	pjr		create RpBase64.c, change routine prefixes to Rp,
 *							move Base64 states from RpSmtp.c
 *		10/21/98	bva		add PUT support
 *		10/16/98	pjr		add RpCommon.c and RpParse.c move some routine
 *							prototypes.  add kAllegro
 *		10/14/98	bva		add eRpHttpNeedCgiAuthorization
 *		10/12/98	bva		make fHaveRequestObject always part of request block
 *		10/03/98	bva		merge in RomDNS support
 *		09/25/98	bva		fix kLastChunk
 *		09/24/98	bva		add support for Expect and Update headers
 *		09/21/98	pjr		move file related fields from the HTTP request
 *							block to the connection block.
 *		09/11/98	bva		HandleIppParserResponse needs rpConnectionPtr
 *		09/03/98	pjr		enable RpHexToString for RomPop
 *		09/01/98	bva		parsing control moves to connection block
 *		08/31/98	rhb		change prototype for RmFreeSmtpRequestBlock
 *		08/31/98	bva		change <CR><LF> definitions
 *		08/25/98	rhb		add prototype for PrReleaseRequestBlock
 *		08/20/98	pjr		rework connection error handling
 *		08/12/98	pjr		remove unused state eRpMpStart
 *		08/09/98	bva		make rpData.fIppAccess rpAccess
 *		08/04/98	bva		make fRemoteHostIndex Unsigned16
 *		07/27/98	pjr		increase fHttpPatternTable to 19 elements
 *		07/16/98	rhb		add fUuencodeState to rpPop3Request structure
 *		07/16/98	rhb		replace fPreviousDataType with fScanForBoundaryFlag
 *		07/06/98	bva		change kHttpContentDisposition,
 *							move Mime Type strings to RpMimes.h
 *		07/01/98	pjr		enable fRemoteHostIndex for RpUserExitMulti
 *		06/26/98	pjr		SlavePager changes
 *		06/23/98	bva		add fProtocolTimer to rpConnection
 *		06/22/98	pjr		add basic authentication support to Remote Host.
 *		06/18/98	pjr		add form processing and redirect support to
 *							Remote Host.
 *		06/16/98	bva		add fUserDataArea to rpHttpRequest,
 *							add fSlaveIdentity to rpData
 *		06/08/98	bva		add support for multiple Remote Hosts
 *		06/05/98	bva		conditionally compile 64-bit definitions
 *		05/27/98	bva		add kTypeXmlObject
 * * * * Release 2.1 * * * *
 *		05/20/98	rhb		add kHtmlOptionOpen	and kHtmlSelected
 *		05/08/98	bva		add fSessionCloseFuncPtr to rpRealm
 *		04/28/98	bva		add fIppPrinterName to rpHttpRequest
 *		04/20/98	rhb		add fItemNumber to rpHttpRequest and RpSendItem() 
 *								now returns a value
 *		03/25/98	pjr		remove fResponseLengthSent from rpHttpRequest
 *		03/21/98	rhb		rename kPrMaxFileInfoLength to kPrMaxFileNameLength
 *		03/20/98	bva		rework header storage in rpHttpRequest
 *		03/19/98	rhb		add fMimeType to rpPop3Request
 *		03/18/98	rhb		define kMaxParseLength
 *		02/26/98	rhb		add fForceInsecureLogOnFlag field to rpPop3Request
 *		02/24/98	pjr		rework phrase dictionary patching feature to be
 *							connection block driven.
 *		02/18/98	rhb		add prototype for PrHandleWaitingForReceive
 *		02/17/98	pjr		rework some remote host protocol states.  add
 *							RpOpenRemoteHostConnection function prototype.
 *							remove eRpWaitingForActiveConnection.
 *		02/16/98	bva		add RpFindItemFromName
 *		02/14/98	bva		eRpConnectionRemoteHostWait->eRpConnectionHolding
 *		02/13/98	rhb		compile RpStringToMimeType for RomPop and change
 *								parameter to RpCatUnsigned32ToString
 *		02/06/98	rhb		add prototype for RpDecodeBase64Data and add
 *								a return value
 *		02/03/98	rhb		add prototype for RpParseDate and 
 *								enable MD5 for PrUseApop
 *		02/02/98	rhb		add eRpConnectionWaitingForRPReceive
 *		02/02/98	bva		add RpHandleSendReceiveError prototype
 *		02/01/98	bva		fNewLineLast -> fLastCharacter
 *		01/23/98	pjr		add phrase dictionary patching feature.
 *		01/21/98	bva		add eRpConnectionProtocolWaitExternal,
 *							eRpConnectionWaitingForRHReceive,
 *							RmSendDataOutBase64, RmSendOutBase64Padding
 *		01/20/98	bva		add RmStartSmtp
 *		01/19/98	bva		add eRpConnectionNeedsProtocolAction
 *							remove eRpConnectionNeedsHttpAction,
 *							eRpConnectionNeedsRemoteHostAction, 
 *							eRpConnectionNeedsSmtpAction
 *		01/16/98	bva		add RmFreeSmtpRequestBlock
 *		01/13/98	bva		add kUrlStatePrefix
 *		01/12/98	rhb		add POP3 support
 *		01/06/98	pjr		add HTTP event logging (RomPagerLogging).
 * * * * Release 2.0 * * * *
 *		12/11/97	bva		kTypePrintForm -> kTypeIppObject
 *		12/09/97	pjr		add "Pragma: no-cache" support to Remote Host.
 *		12/09/97	bva		add kHttpOneOneNoCache
 *		12/04/97	bva		rework SpwdGetExternalPassword documentation
 *		12/03/97	bva		move RpGetFormItem and RpReceiveItem to RpCallBk.h
 *		11/26/97	pjr		add more HTTP headers and date handling to the
 *							Remote Host feature.
 *		11/22/97	bva		add eRpHttpRequestTooLarge
 *		11/21/97	bva		add fHaveHost to rpHttpRequest
 *		11/19/97	pjr		add defines related to the Remote Host feature.
 *		11/10/97	rhb		add kUnsigned64_Max and kSigned64_Max
 *		11/03/97	pjr		move receive buffer parsing parameters from the
 *							rpHttpRequest structure into new rpParsingControl
 *							structure.
 *		11/03/97	rhb		IPP changes
 *		10/28/97	bva		rework User Exit support 
 *		10/07/97	pjr		don't include fHttpRequests in the rpData
 *							structure if RomPagerDynamicRequestBlock.
 *		09/30/97	pjr		change RpHexToString prototype.
 *		09/24/97	pjr		replace gUserPhrases with international versions:
 *							gUserPhrasesEnglish, gUserPhrasesFrench...
 *		09/14/97	bva		integrate RomMailer support 
 *		09/12/97	pjr		change some strings used by the Security feature.
 *		09/03/97	pjr		reduce MD5 entry points to one.
 *		08/29/97	bva		add RpHtmlAlignmentUsesPreTag
 *		08/28/97	pjr		RpCheckAccess now returns rpPasswordState instead
 *							of Boolean.  add RpCheckAccess2 prototype.
 *		08/27/97	pjr		add eRpAnalyzeHttpRequest to rpHttpTransactionState.
 *							add eRpPasswordAuthorized to rpPasswordState.
 *		08/26/97	pjr		remove CR, LF from MIME type strings.  add
 *							kTypePictImageAlt and kTypePictImageAlt2.
 *							add rpStringToMimeTypeTable structure typedef.
 *		08/21/97	bva		rework URL dispatching
 * * * * Release 1.70b1 * * * *
 *		08/18/97	bva		kServerName -> kServerHeader
 *		08/09/97	bva		add fHaveRequestObject to rpHttpRequest
 *		08/04/97	pjr		add fFinalBoundary to rpHttpRequest structure.
 *		08/03/97	bva		add RpGetChunkedObjectData
 *		07/31/97	rhb		add kHtmlPathSeparator, fCurrentItemValue size can vary 
 *		07/31/97	edk		rework user phrase dictionary for broader compiler support
 *		07/30/97	pjr		add RpFindLineEnd prototype.  move some prototypes
 *							from RpHttpRq.c to RpHttpPs.c.
 *		07/29/97	pjr		kAscii_Dash -> kAscii_Hyphen.  remove
 *							eRpParsingMultipartWaitForFS.
 *		07/25/97	pjr		rework Content-Type/Mime Type strings.  add
 *							thePatternTablePtr to RpParseHeader routine.
 *							miscellaneous Multipart data changes.
 *		07/21/97	bva		add fReceiveBufferPtr, fReceiveBufferLength move
 *							fIpRemote, fIpLocal, fLocalPort to rpConnection
 *		07/15/97	pjr		add fChallengeNonce to rpRealm struct.  change
 *							fAuthPatternTable to have 5 elements.
 *		07/12/97	bva		add HTTP->IPP interface,
 *							fFormRequestLength -> fPostRequestLength
 *		07/10/97	bva		add RpFindFormItemType
 *		07/07/97	pjr		use kStcpReceiveBufferSize instead of kHttpWorkSize
 *							for fMultipartWorkBuffer size.  change Multipart
 *							parsing state enums.
 *		07/04/97	bva		eliminate RpExpandString, add RpExpandedMatch
 *		06/28/97	bva		reduce fPatternTable size, add kHttpPattern constants
 *		06/27/97	bva		rework kHtmlFormHeader
 *		06/25/97	pjr		add form based file upload feature.
 *		06/19/97	bva		add fRepeatWhileValue to rpNesting
 *		06/12/97	bva		add fUrlState to rpHttpRequest, fURL -> fRefererUrl
 *		05/26/97	bva		eRpHttpServerBusy -> eRpHttpForbidden
 *		05/24/97	bva		rework URL dispatching
 * * * * Release 1.6 * * * *
 *		04/26/97	bva		add fHttpTaskCount to rpData
 *		04/18/97	pjr		add Security Digest feature
 *		04/15/97	bva		add fHttpVersion to rpHttpRequest
 *		04/07/97	bva		add kHtmlValueClose, change kHtmlChecked, kHtmlInputClose
 *		04/04/97	bva		cleanup warnings
 *		03/31/97	bva		rework request line handling
 *		03/22/97	bva		add RpSendServerPushSeparator, RpSetLastBuffer
 *		03/20/97	bva		add rpConnectionType, RpSendServerPushHeaders
 *		03/10/97	bva		add kTypePngImage, rework connection states
 *		03/05/97	bva		add RpStoreQueryValues
 *		02/23/97	bva		remove eRpConnectionNeedsRead, 
 *								eRpConnectionNeedsClose, 
 *								eRpConnectionBuildingReply,
 *								eRpConnectionNeedsAbort from rpConnectionState
 *		02/17/97	bva		fCloseTimer->fAbortTimer, 
 *								remove eRpConnectionCloseTimeWait state
 *		02/02/97	bva		add fDataPtr to rpHttpRequest
 *		02/01/97	bva		fChallengeIpAddress moved to rpRealm,
 *								refreshSeconds becomes Unsigned16
 *		01/31/97	bva		add selectable user dictionary
 *		01/24/97	bva		add fLanguage to rpHttpRequest,
 *								fAccess->fRealmPtr
 *		01/20/97	bva		make fAgent conditional in rpHttpRequest,
 *								add kHttpNoCache 
 *		12/13/96	bva		add rpParseState
 *		12/12/96	rhb		correct kSecsPerHour, kSecsPerDay, & kSecsPerYear
 *								and add gRpMonthDaysLeapYear & kSecsPerFourYears
 *		12/07/96	bva		adjust buffer sizes in rpHttpRequest
 *		12/06/96	bva		add fKeepAlive to rpHttpRequest
 *		12/04/96	bva		add gMimeTypes
 * * * * Release 1.5 * * * *
 *		11/06/96	bva		add fRefreshObjectPtr, fRefreshSeconds to
 *								rpHttpRequest structure
 *		10/16/96	bva		add RpBuildQueryValues, RpPopQueryIndex, 
 *								RpPushQueryIndex
 *		10/18/96	bva		consolidate HTTP headers and form items buffers
 *		10/16/96	bva		add eRpConnectionNeedsAbort to rpConnectionState
 *		10/14/96	bva		add kTypeText
 *		10/10/96	bva		add fSecurityLevel to rpRealm
 *		10/02/96	bva		add fLocalPort to rpHttpRequest
 *		09/24/96	rhb		support dynamically allocated engine data 
 *		09/22/96	bva		bump fPatternTable size for kHttpHost
 *		09/22/96	bva		add kHttpHost, bump rev in kServerName
 *		09/20/96	rhb		allow more than one HTTP request
 *		09/06/96	bva		kHttpContentTransfer-Encoding -> kHttpContentTransferEncoding
 * * * * Release 1.4 * * * *
 *		08/17/96	bva		add kAscii_Colon, gRpItemErrorPage, 
 *								eRpHttpRedirectMap
 *		08/16/96	bva		add kMaxStringLength, gRpSubmitButtonValue, 
 *								RpSetNextPage
 * * * * Release 1.3 * * * *
 *		07/23/96	bva		add kSignedn_Max constants
 *		07/22/96	bva		add index pointer to RpResetCurrentSession
 *		07/10/96	bva		add RpCatSigned32ToString and 
 *								RpConvertSigned32ToAscii
 *		07/08/96	bva		add gRpIndexDepth, gRpIndexValues
 *		07/05/96	bva		remove fForm from rpHttpRequest and use fPath
 *							add RpCheckQuery
 *							merge page and form headers
 *		07/03/96	bva		add gRpBoxNameText, eRpConnectionCloseTimeWait
 *		07/02/96	bva		add gRpStatus
 *		06/28/96	bva		add rpConnection structure
 *		06/27/96	bva		add support for eRpItemType_HtmlReferer
 *		06/25/96	rhb		add error support for eRpTextType_HexColonForm
 *		06/23/96	bva		add kHttpURL for Netscape refresh, add 
 *								eRpHttpProcessedForm
 *		06/22/96	bva		include "<" in kHttpURIHeader
 *		06/18/96	bva		merge fFirstItem and fNested to fItemState
 *		06/13/96	bva		add gRpChallengeIpAddress and fObject to rpSession
 *		06/12/96	bva		add spaces to some strings to save strcat calls,
 *								gRpFoundFormFlag + gRpFoundPageFlag -> 
 *								gRpFoundContentFlag
 *		06/09/96	bva		add kUnauthorized, gRpHttpNeedAuthorization
 * * * * Release 1.2 * * * *
 *		06/01/96	bva		add RpCatUnsigned32ToString
 *		05/31/96	bva		RpGetBrowserDate -> RpGetDateInSeconds, add time 
 *								constants
 *		05/30/96	bva		align variable sizes to eliminate warnings
 * * * * Release 1.1 * * * *
 * * * * Release 1.0 * * * *
 *		03/25/96	bva		rework top level multi-tasking
 *		03/15/96	bva		added kTypeApplet
 *		03/09/96	bva		added gRpFormAccessNotAllowed, kHtmlHidden
 *		02/16/96	bva		added html end fragments
 *		02/13/96	bva		added html title fragments
 *		11/01/95	bva		created
 *
 *	To Do:
 */
//~~added by zlong, 6/26/2002;
#include "AsTarget.h"
#include "AsConfig.h"
#include "AsEngine.h"
#include "AsError.h"
#include "AsExtern.h"
#include "AsTypes.h"
#include "Stcp.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef	_ROMPAGER_
#define	_ROMPAGER_


#if RomPagerSoftPages
#include "RpSoftPg.h"
#endif

#define	kMaxNumberOfRealms		8


/* 
	UPnP Pattern Headers 
*/
#define kHttpPatternCallback			"callback"
#define kHttpPatternNT					"nt"
#define kHttpPatternNTS					"nts"
#define kHttpPatternSeq					"seq"
#define kHttpPatternSID					"sid"
#define kHttpPatternSoapAction			"soapaction"
#define kHttpPatternTimeout				"timeout"


/* 
	HTML Fragments 
*/
#define kHtmlAnchorEnd					"</A>"
#define kHtmlAnchorMiddle				"\">"
#define kHtmlAnchorStart				"<A HREF=\""
#define kHtmlCheckbox					"CHECKBOX\""
#define kHtmlChecked					" CHECKED"
#define kHtmlColumns					"\" COLS=\""
#define kHtmlFormEnd					"\n</FORM>"
#define kHtmlFormHeader					"<FORM METHOD=\"" 
#define kHtmlFormHeaderMulti			"<FORM ENCTYPE=\"multipart/form-data\" " \
											"METHOD=\""
#define kHtmlFormHeaderPost				"POST"
#define kHtmlFormHeaderGet				"GET"
#define kHtmlFormHeaderAction			"\" ACTION=\""
#define kHtmlHidden						"HIDDEN\""
#define kHtmlImage						"IMAGE\""
#define kHtmlInputType					"<INPUT TYPE=\""
#define kHtmlMaxLength					"\" MAXLENGTH=\""
#define kHtmlMultiple					" MULTIPLE"
#define kHtmlName						" NAME=\""
#define kHtmlOption						"<OPTION>"
#define kHtmlOptionSelected				"<OPTION SELECTED>"
#define kHtmlOptionValue				"<OPTION VALUE="
#define kHtmlPageEnd					"\n</BODY>\n</HTML>\n"
#define kHtmlPassword					"PASSWORD\""
#define kHtmlPathSeparator				"/"
#define kHtmlRadio						"RADIO\""
#define kHtmlFile						"FILE\""
#define kHtmlReset						"RESET"
#define kHtmlRows						"\" ROWS=\""
#define kHtmlSelect						"<SELECT"
#define kHtmlSelected					" SELECTED"
#define kHtmlSelectEnd					"</SELECT>"
#define kHtmlSize						"\" SIZE=\""
#define kHtmlSubmit						"SUBMIT\""
#define kHtmlButton						"BUTTON\""
#define kHtmlSubmitLower				"Submit"
#define kHtmlText						"TEXT\""
#define kHtmlTextArea					"<TEXTAREA"
#define kHtmlTextAreaEnd				"</TEXTAREA>"
#define kHtmlTitleEnd					"</TITLE>\n</HEAD>\n<BODY>\n"
#define kHtmlTitleStart					"<HTML>\n<HEAD>\n<TITLE>"
#define kHtmlValue						"\" VALUE=\""
#define kHtmlValueClose					"\""

#if RomPagerUrlState
#define kUrlStatePrefix					"/" \
										kUrlState \
										"/"
#define kUrlStateRelativePrefix			kUrlState \
										"/"
#endif

#if AsSnmpAccess
#define kBadSnmpGetRequest				"SNMP MIB Object Not Available\n"
#endif

#if RpHtmlAlignmentUsesPreTag
#define kHtmlValueInputClose			"\">"
#define kHtmlInputClose					">"
#else
#define kHtmlValueInputClose			"\">\n"
#define kHtmlInputClose					">\n"
#endif


#if RomPagerServer || RomPagerBasic
/*
	Define the maximum length of a Base64 encoded Username:Password string.
*/ 
#define kRpMaxEncUserPwdLen	(kRpMaxUserNameLength + kRpMaxPasswordLength + \
							((kRpMaxUserNameLength + kRpMaxPasswordLength) / 2))
#endif


#if RomPagerServer

#if RomPagerExternalPassword || RomPagerSecurity

/*
	security structures 
*/
typedef struct {
	char					fUsername[kRpMaxUserNameLength];
	char					fPassword[kRpMaxPasswordLength];
	rpAccess				fAccessCode;
	rpAccess				fHttpSessionCode;
	rpAccess				fInternalSessionCode;
	Signed32				fSessionTimeout;
	Signed32				fHttpSessionTimer;
	rpSessionCloseFuncPtr	fSessionCloseFuncPtr;
	void *					fPasswordCookie;
	void *					fUserCookie;
	Unsigned32				fIpAddress;
#if RomPagerExternalPassword
	Unsigned32				fLastUsedTime;
#endif
} rpUser, *rpUserPtr;


typedef struct {
	char					fRealmName[kMaxNameLength];
	rpSecurityLevel			fSecurityLevel;
	rpUserPtr				fUserLock;
	Signed32				fChallengeTimeout;
#if RomPagerSecurityDigest
	char					fNonce[kDigestStringLength];
	char					fChallengeNonce[kDigestStringLength];
#endif	/* RomPagerSecurityDigest */
} rpRealm, *rpRealmPtr;

#endif	/* RomPagerExternalPassword || RomPagerSecure */


/*
	rpNesting structure 
*/
typedef struct {
 	rpItemPtr				fReturnItemPtr;
 	rpItemPtr				fRepeatItemPtr;
	rpRepeatWhileFuncPtr	fRepeatWhileFunctionPtr;
	void *					fRepeatWhileValue;
	Signed16				fIndexLimit;
	Signed16				fIndexIncrement;
#if RomPagerSoftPages
	rpItemType				fItemType;
	char *					fLoopStartPtr;
	Unsigned16				fLoopStartLength;
#endif
} rpNesting, *rpNestingPtr;

#if RomPagerLogging
/*
	Structure for logging HTTP events.
*/
typedef struct {
	Unsigned32				fEventTime;
	void *					fEventInfoPtr;
	rpObjectSource			fObjectSource;
	rpHttpResponseAction	fEventType;
} rpLogItem, *rpLogItemPtr;
#endif


#if RomPagerRemoteHost

/*
	Remote Host Request block
*/
typedef struct {
	struct rpConnection *	fServerConnectionPtr;
	struct rpConnection *	fClientConnectionPtr;
	char *					fDateStringPtr;
	rpRemoteHostState		fRemoteHostState;
	rpRemoteRequestState	fRemoteRequestState;
	rpHttpResponseAction	fHttpResponseState;
	char					fRedirectPath[kMaxSaveHeaderLength];
	char					fWorkBuffer[kRemoteHostWorkBufferSize];
	char					fTempLineBuffer[kMaxLineLength];
	rpParsingControlPtr		fParsingControlPtr;
	rpObjectDescription		fObject;
	Unsigned16				fHostIndex;
#if RomPagerSecurity
	rpRealm					fRealm;
#if RpRhLocalCredentials
	Unsigned8				fAuthorizationCount;
#endif
#endif
} rpRHRequest, *rpRHRequestPtr;


/*
	Remote Host response pattern Structure 
*/
typedef void rpRHPatternActionProcedure(rpRHRequestPtr theRHRequestPtr,
										char *theStartOfTokenPtr,
										Unsigned16 theTokenLength);

typedef struct {
	rpRHPatternActionProcedure *	fAction;
	char *							fPattern;
	Unsigned16						fPatternLength;
} rpRHPatternTable, *rpRHPatternTablePtr;

#endif /* RomPagerRemoteHost */

#endif /* RomPagerServer */

/*
	rpHttpRequest structure 
*/
typedef struct rpHttpRequest {
	struct rpData *			fDataPtr;
	Boolean					fInUse;
#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
/*
	keep fHtmlResponseBufferOne, fHtmlResponseBufferTwo, fHttpWorkBuffer in this order
*/
	char					fHtmlResponseBufferOne[kHtmlMemorySize];
	char					fHtmlResponseBufferTwo[kHtmlMemorySize];
#if RomPagerServer || RomPagerBasic
	char					fHttpWorkBuffer[kHttpWorkSize];
#else
	char					fHttpWorkBuffer[2];
#endif
#endif	/* RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption */
	char					fHttpTempLineBuffer[kMaxParseLength];
#if RomPagerBasic
	char					fInternalResponseBuffer[256];
#endif
	Boolean					fHtmlBufferReady;
	Signed8					fHtmlCurrentBuffer;
	char *					fHtmlFillPtr;
	char *					fHtmlResponsePtr;
	Unsigned16				fFillBufferAvailable;
	Unsigned16				fHtmlResponseLength;
#if RomMailer
	rpBase64State			fBase64State;
	Signed32				fBase64LineLength;
	char					fBase64Chars[3];
#endif
#if RomPagerServer || RomPagerBasic
#if RomPagerFileSystem
	SfsFileInfoPtr			fFileInfoPtr;
#endif
#if RomPagerFileSystem || RomPagerIpp || RomPagerUserExit || RomPlugControl
	rpObjectDescription		fLocalObject;
#endif
#if RomPagerUserExit
	rpCgi					fCgi;
#endif
#if RomPlugAdvanced || RomPagerXmlServices
    rpHttpXmlState          fHttpXmlState;
	Unsigned16				fXmlBufferLength;
	char *					fXmlBufferPtr;
	Boolean					fLastXmlBuffer;
	void *					fRxContextPtr;
	void *					fRxBodyContextPtr;
#endif
	rpParsingControlPtr		fParsingControlPtr;
	Signed32 				fPostRequestLength;
 	Unsigned32 				fBrowserDate;
	rpObjectDescriptionPtr	fObjectPtr;
	rpObjectDescriptionPtr	fCurrentFormDescriptionPtr;
	rpObjectDescriptionPtr	fRefreshObjectPtr;
	char *					fUserErrorPtr;
	rpDataType				fAcceptType;
	rpObjectSource			fObjectSource;
	rpHttpCommand			fHttpCommand;
	rpHttpResponseAction	fHttpResponseState;
	rpHtmlState				fItemState;
	rpHttpTransactionState	fHttpTransactionState;
	Unsigned16				fRefreshSeconds;
	rpScheme				fRedirectScheme;
#if RomPagerCaptureUserAgent
	char					fAgent[kMaxSaveHeaderLength];
#endif
#if RomPagerCaptureLanguage
	char					fLanguage[kMaxSaveHeaderLength];
#endif
#if RomPlugAdvanced || RomPagerCaptureSoapAction
	char					fSoapAction[kMaxSaveHeaderLength];
#endif
#if RomPlugAdvanced
	char					fCallback[kMaxSaveHeaderLength];
	upnpSubscriptionPtr		fUpnpSubscriptionPtr;
	rpSoapRequest			fSoapRequest;
#endif
#if RomPlugAdvanced || RomPlugControl
	Boolean					fNtReceived;
	char					fSubscriptionID[kMaxSaveHeaderLength];
	Signed32				fSubscriptionTimeout;
#endif
#if RomPlugControl
	Boolean					fNtsReceived;
	Unsigned32				fSeq;
#endif
	char					fHost[kMaxSaveHeaderLength];
	char					fIfModified[kMaxSaveHeaderLength];
	char					fPath[kMaxSaveHeaderLength];
 	char					fRefererUrl[kMaxSaveHeaderLength];
// +++ _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#if 1
	char					fSearch[kMaxStringLength];	// URL query values
#endif
// --- _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#if RomPagerSecurity
  	char					fUsername[kRpMaxUserNameLength];
 	char					fPassword[kRpMaxPasswordLength];
#endif
#if RomPagerFileUpload || RomPagerIpp || RomPagerRemoteHost || RomPagerPutMethod
	char					fHttpContentType[kMaxMimeTypeLength];
	rpDataType				fDataType;
#endif
#if RomPagerServerPush || RomMailer
 	char					fSeparator[kMaxNameLength];
#endif
#if RomPagerServer
	asItemError				fItemError;
	rpItemPtr				fItemPtr;
	rpItemPtr				fNextItemPtr;
	rpProcessCloseFuncPtr	fProcessCloseFuncPtr;	/* Pointer to optional close handler */
	Signed32				fSerial;
	rpNesting				fNestedItems[kAsIndexQueryDepth + 3];
	char					fErrorPath[kMaxSaveHeaderLength];
	char 					fSubmitButtonValue[kMaxStringLength];
	char					fCurrentItemName[kMaxNameLength];
	char **					fUserPhrases;
	Boolean					fUserPhrasesCanBeCompressed;
#if RpHtmlTextAreaBuf
	char					fCurrentItemValue[kHttpWorkSize];
#else
	char					fCurrentItemValue[kMaxValueLength];
#endif
#if RomPagerUrlState
	char					fUrlState[kMaxStringLength];
#endif
#if RomPagerImageMapping
	Signed16				fMapHorizontal;
	Signed16				fMapVertical;
#endif
#if RomPagerSecurity
	rpUserPtr				fAuthenticatedUserPtr;
	rpRealmPtr				fRealmPtr;
#endif
#if RpUserCookies
	void *					fUserCookie;
#endif
#if RomPagerUserDataAreas
	char					fUserDataArea[kRpUserDataSize];
#endif
#if RomPagerHttpCookies
	char					fHttpCookies[kNumberOfHttpCookies][kHttpCookieSize];
	Boolean					fNewHttpCookie[kNumberOfHttpCookies];
#endif
#if RomPagerIpp
	ippRequest				fIppRequest;
	Signed32				fIppRequestLength;
	rpHttpIppState			fHttpIppState;
#if 0
	char					fIppJobIdent[kMaxNameLength];
	char					fIppPrinterName[kMaxNameLength];
#endif
	Boolean					fIppResponseStarted;
#endif
#if RomPagerIpp || RomPagerPutMethod
	Boolean					fIncomingBufferEnd;
#endif
#if RomPagerServerPush
	Boolean					fServerPushActive;
	rpObjectDescriptionPtr	fServerPushObjectPtr;
	Unsigned16				fServerPushSeconds;
#endif
#if RpRemoteHostMulti
	Unsigned16				fRemoteHostIndex;
#endif
#if RpHtmlSelectVariable || RpHtmlSelectVarValue || RpHtmlVariableDynamic
	Unsigned8				fItemNumber;
#endif
#if RomPagerFileUpload
	rpMultipartState		fMultipartState;
	Signed32				fBoundaryLength;
	Signed32				fMultipartRemainderLength;
	char					fFilename[kMaxValueLength];
	char					fItemName[kMaxNameLength];
	char					fItemValue[kMaxValueLength];
	char					fBoundary[kMaxBoundaryLength];
	char					fMultipartRemainderBuffer[kMaxBoundaryLength + 8];
#if RomPagerSecure
#if 0
	char					fMultipartWorkBuffer[kTLSBufferLen + kMaxBoundaryLength];
#else
	char					fMultipartWorkBuffer[kStcpReceiveBufferSize + kMaxBoundaryLength];
#endif
#else
	char					fMultipartWorkBuffer[kStcpReceiveBufferSize + kMaxBoundaryLength];
#endif
	Boolean					fFileDone;
	Boolean					fFinalBoundary;
#endif	/* RomPagerFileUpload */
#if RomMailer
	char					fLastCharacter;
	Boolean					fAggregate;
	Signed8					fImageCount;
	Signed8					fImageSent;
	rpObjectDescriptionPtr	fImageList[kRmMaxEmbeddedImages];
#endif
#if RpHtmlBufferDisplay
	Boolean					fBufferItemLastBuffer;
	Unsigned32				fBufferItemLength;
	char *					fBufferItemPtr;
#endif
#if AsSnmpAccess
	Boolean					fSnmpInitialized;
	asSnmpAccessItem		fSoftPageSnmpAccess;
	asSnmpAccessItem		fSnmpAccess[kAsSnmpGetNextCount];
#endif
#if RomPagerRanges
	Boolean					fHaveRangeRequest;
	Signed32				fRangeBegin;
	Signed32				fRangeEnd;
	Unsigned32				fBytesSent;
#endif
#endif	/* RomPagerServer */
	Signed8					fNestedDepth;
#if RomPagerHttpOneDotOne
	Signed32				fChunkLength;
	Signed32				fChunkedContentLength;
	Signed32				fChunkLengthProcessed;
	Signed32				fCurrentChunkBufferLength;
	char *					fCurrentChunkBufferPtr;
	rpHttpChunkState		fChunkState;
	rpHttpVersion			fHttpVersion;
	Boolean					fHaveChunk;
	Boolean					fObjectIsChunked;
	Boolean					fResponseIsChunked;
	Boolean					fHaveHost;
#endif
#if RpExpectHeader
	Boolean					fWantsContinue;
#endif	
#if RomPagerTraceMethod
	Boolean					fTracing;
	Signed32				fTraceLength;
	char *					fTraceBufferPtr;
#endif
	Boolean					fHaveRequestObject;
	Boolean					fHaveRequestLine;
	Boolean					fKeepAlive;
	Boolean					fPersistent;
#if RomPagerDynamicRequestBlocks
	Boolean					fDynamicallyAllocated;
#endif
#if RomPagerServer && RomPagerSecurity
	rpPasswordState			fPasswordState;
#endif
#if RomPagerSecurityDigest
 	char					fDigest[kDigestStringLength];
 	char					fCnonce[kMaxStringLength];
 	char					fNonce[kDigestStringLength];
 	char					fNonceCount[kNonceCountLength];
 	char					fDigestURI[kMaxSaveHeaderLength];
	Boolean					fClientSupportsDigest;
	Boolean					fNonceStale;
	Boolean					fQopAuth;
#endif
#if RpPageFlowDebug
	Boolean					fDebugPageFlow;
#endif
#if RomPagerRemoteHost
	char					fOtherMimeType[kMaxMimeTypeLength];
	char *					fRemoteDateStringPtr;
#if RomPagerSecurity
	char					fEncodedUserPassword[kRpMaxEncUserPwdLen];
#endif
#endif
#if RomPagerPutMethod
	char *					fPutBufferPtr;
	Signed32				fPutBufferLength;
	rpHttpPutState			fHttpPutState;
#endif
#if RpFileInsertItem
	char *					fFileNamePtr;
#endif
#if RpHtmlFormImage
	Boolean					fFormImageDetectedFlag;
#endif
#if RomPagerSoftPages
	Boolean					fSoftPageFlag;
	spRequestPtr			fSoftPageRequestPtr;
#endif
#if RpEtagHeader
	Boolean					fClientHasObject;
#endif
#endif	/* RomPagerServer || RomPagerBasic */

} rpHttpRequest, *rpHttpRequestPtr;


/*
	HTTP response pattern Structure 
*/
typedef void rpPatternActionProcedure(rpHttpRequestPtr theHRPtr, 
		char *theStartOfTokenPtr, Unsigned16 theTokenLength);

typedef struct {
	rpPatternActionProcedure *	fAction;
	char * 						fPattern; 
	Unsigned16 					fPatternLength; 
} rpPatternTable, *rpPatternTablePtr;


/*
	RomPager global data
*/
extern const char *			gRpPhrases[];


/*
	RomPager global page data
*/
#if RomPagerBasic
extern char					gRpAllegroCopyrightText[];
extern char					gRpHttpNoObjectFoundText[];
extern char					gRpInputTooLargeText[];
extern char					gRpProtectedObjectText[];
#endif
#if RomPagerServer
extern rpObjectDescription	gRpAccessNotAllowedPage;
extern rpObjectDescription	gRpAllegroCopyrightPage;
extern rpObjectDescription	gRpHttpNoObjectFoundPage;
extern rpObjectDescription	gRpItemErrorPage;
extern rpObjectDescription	gRpServerBusyPage;
extern rpObjectDescription	gRpDigestUnsupportedPage;
extern rpObjectDescription	gRpSslRequiredPage;
extern rpObjectDescription	gRpFileSystemErrorPage;
extern rpObjectDescription	gRpUnexpectedMultipart;
extern rpObjectDescription	gRpInputTooLargePage;
extern rpObjectDescription	gRpListTest;
#endif
#if RomPagerSecure
extern rpObjectDescription	gRpSslRequiredPage;
#endif


#endif	/* _ROMPAGER_ */
