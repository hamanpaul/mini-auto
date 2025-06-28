/*
 *	File:		AsChkDef.h
 *
 *	Contains:	Define any compile flags that are not already defined as 0.
 *				This is needed because some compilers will generate errors,
 *				rather than assuming undefined flags are 0.
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
 *		09/19/03	pjr		add RpRhLocalCredentials
 *      06/27/03    rhb     add RxDebug, RxDynXnames, RxWarningsAreErrors, 
 *								WcPostCaching, WcDebug, and WcUnknownHeaders
 * * * * Release 4.21  * * *
 *		05/07/03	amp		add RpRootObjectIsUserExit
 *		04/27/03	pjr		add WcStreaming
 *		03/14/03	bva		add AsMultiTaskingOption
 * * * * Release 4.20  * * *
 *      01/28/03    rhb     add RomPagerSoftPageProcessUserExit
 *		12/18/02	amp		add RomCliSecure definitions
 *		12/13/02	rhb		add RxChildModeConversions
 *		12/13/02	rhb		add RomPagerSoftPageUserExit
 *		11/05/02	pjr		add WcPipelining and WcProxyAuthentication
 *		10/23/02	rhb		add  RxOmitDefaultFraming
 *		10/02/02	pjr		add and RomWebClientAdvanced and WcCookieParameters
 * * * * Release 4.12  * * *
 *      08/21/02    bva     add RomCliRecord, RomCliPlayback, RomCliShowCommands
 * * * * Release 4.11  * * *
 *      07/25/02    bva     add RomPagerCaptureSoapAction
 *		07/01/02	pjr		add WcCaching
 *		06/26/02	rhb		add RpUserExitAggregate, RxBase64Conversions, 
 *								RxCallbackConversions, RxRawInPlaceConversions,  
 *								RxReduceWhitespace
 * * * * Release 4.10  * * *
 *		06/12/02	bva		add RomCliInitialPlayback
 *		06/08/02	bva		remove RomPagerSlaveIdentity, RpUserExitMulti
 * * * * Release 4.07  * * *
 * * * * Release 4.03  * * *
 *		10/16/01	rhb		replace RomPagerTLS with RomPagerSecureExport
 *		10/03/01	pjr		add more WcConfig.h definitions
 *		09/25/01	amp		add RmCharsetOption
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		07/25/01	pjr		add RcConfig.h section
 *		07/10/01	pjr		add kStcpNumberOfConnections
 *		05/09/01	bva		remove RpPhraseDictPatching
 *		04/18/01	bva		add RxConfig.h section
 *		02/13/01	rhb		add section for AsConfig.h moving to it and renaming
 *								RomPagerUse64BitIntegers to AsUse64BitIntegers, 
 *								RomPagerSnmpAccess to AsSnmpAccess, and
 *								RpCustomVariableAccess to AsCustomVariableAccess
 *		11/17/00	pjr		add Variable
 *		07/20/00	amp		add PrParseMdnReceipts and PrParseDsnReceipts
 *		06/23/00	pjr		add RpRootObjectIsFile
 * * * * Release 3.10  * * *
 *		02/16/00	bva		add SlavePager
 *		01/31/00	amp		add RomMailBasic and RomPopBasic definitions
 * * * * Release 3.06  * * *
 * * * * Release 3.05  * * *
 *		10/31/99	bva		add WcRequestCancel
 *		10/13/99	bva		add PrKeepConnectionAlive
 *		09/14/99	bva		add RmXtraHeaders
 * * * * Release 3.04  * * *
 *		07/14/99	bva		add RpLight, RomDNS, RomPOP and RomMailer options,
 *							remove WcFileOptions
 * * * * Release 3.03  * * *
 * * * * Release 3.0 * * * *
 *		02/04/99	bva		add RomPagerTLS, RomPagerHttpCookies and alphabetize
 *		01/29/99	bva		add RpEtagHeader
 *		01/24/99	pjr		created
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_ASCHKDEF_
#define	_ASCHKDEF_


/* 
	AsConfig.h
*/

#ifndef	AsUse64BitIntegers
#define AsUse64BitIntegers				0
#endif

#ifndef	AsSnmpAccess
#define AsSnmpAccess					0
#endif

#ifndef	AsCustomVariableAccess
#define AsCustomVariableAccess			0
#endif


#ifndef	AsMultiTaskingOption
#define AsMultiTaskingOption			0
#endif

/* 
	RpConfig.h
*/

#ifndef	RomPagerCaptureLanguage
#define RomPagerCaptureLanguage			0
#endif

#ifndef	RomPagerCaptureUserAgent
#define RomPagerCaptureUserAgent		0
#endif

#ifndef	RomPagerCaptureSoapAction
#define RomPagerCaptureSoapAction		0
#endif

#ifndef	RomPagerChassisDemo
#define RomPagerChassisDemo				0
#endif

#ifndef	RomPagerClientPull
#define RomPagerClientPull				0
#endif

#ifndef	RomPagerContentDisposition
#define RomPagerContentDisposition		0
#endif

#ifndef	RomPagerDebug
#define RomPagerDebug					0
#endif

#ifndef	RomPagerDelayedFunctions
#define RomPagerDelayedFunctions		0
#endif

#ifndef	RomPagerDemo
#define RomPagerDemo					0
#endif

#ifndef	RomPagerDynamicRequestBlocks
#define RomPagerDynamicRequestBlocks	0
#endif

#ifndef	RomPagerExternalPassword
#define RomPagerExternalPassword		0
#endif

#ifndef	RomPagerFileUpload
#define RomPagerFileUpload				0
#endif

#ifndef	RomPagerForms
#define RomPagerForms					0
#endif

#ifndef	RomPagerFullHostName
#define RomPagerFullHostName			0
#endif

#ifndef	RomPagerHttpCookies
#define RomPagerHttpCookies				0
#endif

#ifndef	RomPagerHttpOneDotOne
#define RomPagerHttpOneDotOne			0
#endif

#ifndef	RomPagerImageMapping
#define RomPagerImageMapping			0
#endif

#ifndef	RomPagerIpp
#define RomPagerIpp						0
#endif

#ifndef	RomPagerJavaAppletDemo
#define RomPagerJavaAppletDemo			0
#endif

#ifndef	RomPagerKeepAlive
#define RomPagerKeepAlive				0
#endif

#ifndef	RomPagerLogging
#define RomPagerLogging					0
#endif

#ifndef	RomPagerMimeTypeChecking
#define RomPagerMimeTypeChecking		0
#endif

#ifndef	RomPagerOptionsMethod
#define RomPagerOptionsMethod			0
#endif

#ifndef	RomPagerPutMethod
#define RomPagerPutMethod				0
#endif

#ifndef	RomPagerQueryIndex
#define RomPagerQueryIndex				0
#endif

#ifndef	RomPagerRemoteHost
#define RomPagerRemoteHost				0
#endif

#ifndef	RomPagerSecurity
#define RomPagerSecurity				0
#endif

#ifndef	RomPagerSecurityDigest
#define RomPagerSecurityDigest			0
#endif

#ifndef	RomPagerServerPush
#define RomPagerServerPush				0
#endif

#ifndef	RomPagerSoftPageCompression
#define RomPagerSoftPageCompression		0
#endif

#ifndef	RomPagerSoftPageFunction
#define RomPagerSoftPageFunction		0
#endif

#ifndef	RomPagerSoftPageUserExit
#define RomPagerSoftPageUserExit		0
#endif

#ifndef	RomPagerSoftPageProcessUserExit
#define RomPagerSoftPageProcessUserExit	0
#endif

#ifndef	RomPagerSoftPages
#define RomPagerSoftPages				0
#endif

#ifndef	RomPagerSecureExport
#define RomPagerSecureExport			0
#endif

#ifndef	RomPagerTraceMethod
#define RomPagerTraceMethod				0
#endif

#ifndef	RomPagerUrlState
#define RomPagerUrlState				0
#endif

#ifndef	RomPagerUserExit
#define RomPagerUserExit				0
#endif

#ifndef	RpAbsoluteUri
#define RpAbsoluteUri					0
#endif

#ifndef	RpDictionariesDemo
#define RpDictionariesDemo				0
#endif

#ifndef	RpEtagHeader
#define RpEtagHeader					0
#endif

#ifndef	RpExpectHeader
#define RpExpectHeader					0
#endif

#ifndef	RpFileInsertItem
#define RpFileInsertItem				0
#endif

#ifndef	RpFormFlowDebug
#define RpFormFlowDebug					0
#endif

#ifndef	RpHtmlCheckbox
#define RpHtmlCheckbox					0
#endif

#ifndef	RpHtmlCheckboxDynamic
#define RpHtmlCheckboxDynamic			0
#endif

#ifndef	RpHtmlNamedSubmit
#define RpHtmlNamedSubmit				0
#endif

#ifndef	RpHtmlRadio
#define RpHtmlRadio						0
#endif

#ifndef	RpHtmlRadioDynamic
#define RpHtmlRadioDynamic				0
#endif

#ifndef	RpHtmlSelectFixedMulti
#define RpHtmlSelectFixedMulti			0
#endif

#ifndef	RpHtmlSelectFixedSingle
#define RpHtmlSelectFixedSingle			0
#endif

#ifndef	RpHtmlSelectVariable
#define RpHtmlSelectVariable			0
#endif

#ifndef	RpHtmlSelectVarValue
#define RpHtmlSelectVarValue			0
#endif

#ifndef	RpHtmlSubmit
#define RpHtmlSubmit					0
#endif

#ifndef	RpHtmlTextArea
#define RpHtmlTextArea					0
#endif

#ifndef	RpHtmlTextAreaBuf
#define RpHtmlTextAreaBuf				0
#endif

#ifndef	RpHtmlTextFormDynamic
#define RpHtmlTextFormDynamic			0
#endif

#ifndef	RpHttpFlowDebug
#define RpHttpFlowDebug					0
#endif

#ifndef	RpNoCache
#define RpNoCache						0
#endif

#ifndef	RpPageFlowDebug
#define RpPageFlowDebug					0
#endif

#ifndef	RpRemoteHostMulti
#define RpRemoteHostMulti				0
#endif

#ifndef	RpRhLocalCredentials
#define RpRhLocalCredentials			0
#endif

#ifndef	RpRootObjectIsFile
#define RpRootObjectIsFile				0
#endif

#ifndef	RpRootObjectIsUserExit
#define RpRootObjectIsUserExit			0
#endif

#ifndef	RpUnknownUrlsAreCgi
#define RpUnknownUrlsAreCgi				0
#endif

#ifndef	RpUserExitAggregate
#define RpUserExitAggregate				0
#endif

#ifndef	RpUnknownUrlsAreFiles
#define RpUnknownUrlsAreFiles			0
#endif

#ifndef	RpUnknownUrlsAreRemote
#define RpUnknownUrlsAreRemote			0
#endif

#ifndef	RpUserCookies
#define RpUserCookies					0
#endif

#ifndef	SlavePager
#define SlavePager						0
#endif


/* 
	RsConfig.h
*/

#ifndef	RsGetQuery
#define RsGetQuery						0
#endif


/*
	RmConfig.h
*/

#ifndef	RmAggregateHtml
#define RmAggregateHtml					0
#endif

#ifndef	RmFileOptions
#define RmFileOptions					0
#endif

#ifndef	RmMdnSupport
#define RmMdnSupport					0
#endif

#ifndef	RmXtraHeaders
#define RmXtraHeaders					0
#endif

#ifndef	RmCharsetOption
#define RmCharsetOption					0
#endif

/*
	PrConfig.h
*/

#ifndef	PrCaptureDispNotifyTo
#define PrCaptureDispNotifyTo			0
#endif

#ifndef	PrCaptureMessageId
#define PrCaptureMessageId				0
#endif

#ifndef	PrCaptureReplyTo
#define PrCaptureReplyTo				0
#endif

#ifndef	PrCaptureReturnPath
#define PrCaptureReturnPath				0
#endif

#ifndef	PrDecodeUuencoding
#define PrDecodeUuencoding				0
#endif

#ifndef	PrGetUniqueIds
#define PrGetUniqueIds					0
#endif

#ifndef	PrUseApop
#define PrUseApop						0
#endif

#ifndef	PrKeepConnectionAlive
#define PrKeepConnectionAlive			0
#endif

#ifndef	PrParseMdnReceipts
#define PrParseMdnReceipts				0
#endif

#ifndef	PrParseDsnReceipts
#define PrParseDsnReceipts				0
#endif


/*
	WcConfig.h
*/

#ifndef	RomWebClientAdvanced
#define RomWebClientAdvanced			0
#endif

#ifndef	WcHttpOneDotOne
#define WcHttpOneDotOne					0
#endif

#ifndef	WcKeepAlive
#define WcKeepAlive						0
#endif

#ifndef WcServerName
#define WcServerName					0
#endif

#ifndef	WcRequestCancel
#define WcRequestCancel					0
#endif

#ifndef	WcDigestAuthentication
#define WcDigestAuthentication			0
#endif

#ifndef	WcRefresh
#define WcRefresh						0
#endif

#ifndef	WcHttpCookies
#define WcHttpCookies					0
#endif

#ifndef	WcCookieParameters
#define WcCookieParameters				0
#endif

#ifndef	WcCaching
#define WcCaching						0
#endif

#ifndef	WcPipelining
#define WcPipelining					0
#endif

#ifndef	WcProxyAuthentication
#define WcProxyAuthentication			0
#endif

#ifndef	WcStreaming
#define WcStreaming						0
#endif

#ifndef	WcPostCaching
#define WcPostCaching					0
#endif

#ifndef	WcUnknownHeaders
#define WcUnknownHeaders				0
#endif

#ifndef	WcDebug
#define WcDebug							0
#endif

/*
	RxConfig.h
*/

#ifndef	RxDebug
#define RxDebug							0
#endif

#ifndef	RxDynamicGlobals
#define RxDynamicGlobals				0
#endif

#ifndef	RxExplicitIndexFraming
#define RxExplicitIndexFraming			0
#endif

#ifndef RxSparseArrayFraming
#define RxSparseArrayFraming			0
#endif

#ifndef RxOmitDefaultFraming
#define RxOmitDefaultFraming			0
#endif

#ifndef	RxLookUpObject
#define RxLookUpObject					0
#endif

#ifndef	RxConversions
#define RxConversions					0
#endif

#ifndef	RxBase64Conversions
#define RxBase64Conversions				0
#endif

#ifndef	RxRawConversions
#define RxRawConversions				0
#endif

#ifndef	RxCallbackConversions
#define RxCallbackConversions			0
#endif

#ifndef	RxChildModeConversions
#define RxChildModeConversions			0
#endif

#ifndef	RxReduceWhitespace
#define RxReduceWhitespace				0
#endif

#ifndef	RxDynXnames
#define RxDynXnames						0
#endif

#ifndef	RxWarningsAreErrors
#define RxWarningsAreErrors				0
#endif

#ifndef	RxActiveElements
#define RxActiveElements				0
#endif

#ifndef	RxLinkedListConversions
#define RxLinkedListConversions			0
#endif

#ifndef	RxRawInPlaceConversions
#define RxRawInPlaceConversions			0
#endif


/* 
	RdConfig.h
*/

#ifndef	RdAllowSoa
#define RdAllowSoa						0
#endif

#ifndef	RdDebugDns
#define RdDebugDns						0
#endif

#ifndef	RdUnpackAdditionals
#define RdUnpackAdditionals				0
#endif

#ifndef	RdUnpackAuthority
#define RdUnpackAuthority				0
#endif

#ifndef	RomDnsAlloc
#define RomDnsAlloc						0
#endif


/*
	PcConfig.h
*/

#ifndef	PcCheckHeadersForAttachments
#define	PcCheckHeadersForAttachments	0
#endif

#ifndef	kPcTimeout
#define kPcTimeout						0
#endif

#ifndef	kPcMaxHeaderLength
#define kPcMaxHeaderLength				0
#endif

#ifndef	PcValidateArgs
#define PcValidateArgs					0
#endif


/*
	RcConfig.h
*/

#ifndef kRcNumberOfSerialPortSessions
#define kRcNumberOfSerialPortSessions	0
#endif

#ifndef kRcNumberOfCliSessions
#define kRcNumberOfCliSessions			0
#endif

#ifndef RomCliRecord
#define RomCliRecord					0
#endif

#ifndef RomCliPlayback
#define RomCliPlayback					0
#endif

#ifndef RomCliEscapeCharacters
#define RomCliEscapeCharacters			0
#endif

#ifndef RomCliInitialPlayback
#define RomCliInitialPlayback			0
#endif

#ifndef RomCliShowCommands
#define RomCliShowCommands				0
#endif


/*
	ScConfig.h
*/

#ifndef	kScNumberOfSmtpRequests
#define kScNumberOfSmtpRequests			0
#endif

#ifndef	kScMaxNameLength
#define kScMaxNameLength				0
#endif

#ifndef	kScMaxSubjectLength
#define kScMaxSubjectLength				0
#endif

#ifndef	kSmtpMessageBufferLength
#define kSmtpMessageBufferLength		0
#endif

/*
	ShConfig.h
*/

#ifndef	kShNumberOfSessions
#define	kShNumberOfSessions				0
#endif

/*
	Stcp.h
*/

#ifndef	kStcpNumberOfConnections
#define kStcpNumberOfConnections		0
#endif


#endif	/* _ASCHKDEF_ */
