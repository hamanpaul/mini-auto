/*
 *	File:		AsMain.c
 *
 *	Contains:	Main entry points for RomPager family scheduler
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
 *		09/21/03	bva		improve resource locks initialization
 *		09/19/03	pjr		change AbortTimer back to MaxIdleTimeout
 *							(instead of CloseTimeout) in RpConnectionCloseTcp
 *		09/08/03	pjr		fix closing of UDP connections in AllegroTaskDeInit
 *		08/29/03    rhb		fix SoftPages RomPagerDynamicRequestBlocks mem. leak
 *		08/28/03    bva		rework TCP connection closing in AllegroTaskDeInit
 *		08/07/03    bva		add call to RpHandleFilePosition 
 *		06/30/03	amp		add resource locks
 *		06/20/03    bva		remove call to RpHandleSoapAction
 *		06/16/03    bva		use kNumberOfFiles for file allocation
 * * * * Release 4.21  * * *
 *		05/13/03    rhb		Add Entropy for RomCLI Secure
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 *		03/14/03	bva		change some conditionals to AsMultiTaskingOption
 *		03/13/03	bva		fix conditional for AllegroTaskDeInit
 * * * * Release 4.20  * * *
 *		02/12/03	bva		remove unused debug code
 *		02/10/03	amp		add theTaskDataPtr arg to SsiReceive
 *		02/05/03	pjr		rework closing of persistent connections and use
 *							CloseTimeout instead of MaxIdleTimeout
 *		02/04/03	bva		bump gRpHighestUsedConnection size
 *		01/31/03	pjr		clear more parameters in eRpConnectionClosed state
 *		01/22/03	amp		send TLS close notify after persistent timeout
 *		12/18/02	bva		bump theConnection size
 *      12/12/02    amp     add RomCliSecure
 *		12/01/02	bva		integrate multitasking improvements
 *		11/16/02	bva		rework Ssi interface calls
 *		10/04/02	amp		call SsiTimer for RomWebClient Secure
 *		10/04/02	amp		return immediately after UPnP listener is started
 *      10/04/02    amp     improve SsiOpenPassive
 *      10/04/02    amp     support Ssi functions for RomPagerSecure
 *		09/04/02	pjr		add RpSecurityDeInit call
 * * * * Release 4.12  * * *
 *		08/08/02	rhb		initialize phrase dictionary for RomCLI
 * * * * Release 4.11  * * *
 *		07/18/02	pjr		enable protocol timer support for RmProtocolTiming
 *		07/01/02	pjr		add Web caching support
 * * * * Release 4.10  * * *
 *		06/12/02	bva		rework closed connection dispatching
 *		06/05/02	bva		rework CLI Initial Playback
 *		05/20/02	pjr		add state to RpConnectionCheckTcpClose to handle
 *							eRpTcpConnectionClosing
 *		04/01/02	dts		add call to RomCli Timer handler
 * * * * Release 4.07  * * *
 *		03/26/02	dts		open file system with total number of connections
 *							rather than number of TCP connections.
 *		03/26/02	dts		add support for Cli Initial Playback feature
 *		03/22/02	rhb		add parameter to SsisRetrieve, change RtTimer
 *								parameter, fix closing of SSL connections
 * * * * Release 4.06  * * *
 *		01/18/02	pjr		add AsHasTcpInterface, clean up for products
 *							that do not have a TCP interface
 *		01/10/02	pjr		only include TCP connections in TCP task count
 * * * * Release 4.05  * * *
 *		01/07/02	bva		add fCliActive suspend/resume support
 * * * * Release 4.04  * * *
 *		11/28/01	pjr		formatting
 *		11/27/01	pjr		make connection close callback conditional
 *		11/26/01	pjr		in AllegroTaskDeInit, don't abort active TCP
 *							connections that are already closed anyway
 *		11/26/01	pjr		fix compile problems and optimize code for the
 *							case where there are no server (listener) ports
 * * * * Release 4.03  * * *
 *		10/16/01	rhb		update SSL/TLS debug code
 *		10/14/01	rhb		add RpSetConnectionCloseFunction support
 *		10/10/01	pjr		make SetServerProtocol prototype conditional
 *		10/09/01	rhb		don't allow a secure connection during certificate
 *								creation
 *		09/26/01	amp		add RmCharsetOption
 * * * * Release 4.02  * * *
 *		09/19/01	bva		fix Telnet too many sessions crash
 *		09/18/01	bva		fix warning
 * * * * Release 4.01  * * *
 *		08/03/01	pjr		init. fSkipHttpRequestBlock in HandleConnectionTask
 * * * * Release 4.00  * * *
 *		07/18/01	bva		add RomConsole support to HandleProtocolAction
 *		07/09/01	pjr		remove eRpSerialPortDead connection state
 *							add (kStcpNumberOfConnections > 0) conditionals
 *		06/29/01	amp		add RomUpnpControl support
 *		06/29/01	amp		add Upnp SOAP support
 *		06/28/01	pjr		reanme RpInitializeBox -> AsInitializeBox and call
 *							it for RomCli as well as RomPagerServer
 *		06/19/01	pjr		CsFiniteStateMachine now returns an RpErrorCode
 *		06/14/01	pjr		include Sser.h, and call SserInit and SserDeInit for
 *							RomConsole and remove thePortNumber from the calls
 *		06/07/01	pjr		call HandleWaitingForConnection for RomTelnet
 *		04/25/01	dts		add RomTime support
 *		04/18/01	bva		clean up Upnp/Dns deinit
 *		03/30/01	bva		add call for RpHttpPutComplete
 *		03/22/01	bva		fix Telnet/RomCLI connection initialization
 *		02/27/01	rhb		remove eRpConnectionReceive state
 *		02/20/01	rhb		replace theDataPtr with theConnectionPtr as the
 *								parameter to RtGetServerContext()
 *		02/15/01	bva		remove dictionary patching
 *		02/09/01	dts		add RomConsole support
 *		02/08/01	rhb		remove theDataPtr as a parameter to
 *								RtHandleWaitingForSecurityInformation()
 *		02/07/01	rhb		Make SSLConnectionClosedGraceful a special case
 *							initialize theResult and use eRpTcpSendError when
 *								calling RpHandleSendReceiveError() in
 *								HandleSendingReply()
 *							eliminate global data pointer (gRpDataPtr)
 *		02/06/01	bva		add HandleMultiTaskRescheduling
 *		02/01/01	bva		optimize parameter passing
 *		12/22/00	pjr		change theInitFlags to 32 bits in AllegroTaskDeInit
 *		12/15/00	pjr		rework conditionals for variable access
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		11/03/00	amp		fix C++ compile warning
 *		10/27/00	pjr		initialize index parameters in the connection struct
 *		10/06/00	pjr		rework IPP initialization, move printer name
 *							initialization to RpUser.c
 *		09/03/00	rhb		fix compile warning
 *		09/02/00	bva		Telnet connections don't need HTTP request control blocks,
 *							change handling for RomCLI with no chars received
 *		09/01/00	bva/dts	RomCLI/Telnet integration
 *		08/31/00	bva		change SSL/TLS buffer support
 *		08/08/00	amp		initialize sudp if RomUpnp is active
 *		07/30/00	bva		add RomUpnp support
 *		07/25/00	rhb		Support SSL/TLS
 *		06/29/00	pjr		use RpSetIppPrinterName to set the default name
 *		06/27/00	rhb		add support for multiple IPP response buffers
 *		06/26/00	pjr		add call to IppTimer
 *		06/12/00	bva		add CLI support for no input characters.
 *							add Telnet timeout support
 *		05/30/00	bva		fix compile checks for SH1 compiler
 *		05/26/00	amp		Initialize fEpochReference.
 *		05/26/00	bva		use gRpDataPtr
 *		05/25/00	bva		eliminate fErrorProcPtr warning
 *		05/19/00	pjr		add RxInit and RxDeInit calls
 *		04/21/00	bva		reject incoming connections with the wrong port
 *		02/05/00	bva		kStcpListeners -> kStcpServerCount
 *		02/04/00	amp		add RmMessageTiming
 *		02/01/00	rhb		eliminate warning
 *		01/19/00	bva		add multiple server port support
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 *		01/13/00	amp		add init for RomPopBasic, RomMailerBasic
 * * * * Release 3.06  * * *
 * * * * Release 3.03  * * *
 *		07/08/99	rhb		fix memory allocation error reporting
 *		07/07/99	rhb		fix SoftPages dynamic allocation error
 *		06/25/99	bva		remove extra RpFreeRequestControlBlock call
 *							in RpConnectionCloseTcp
 *		06/17/99	rhb		fix crash of pipelined HTTP 1.1 file system requests
 *		06/14/99	rhb		RomPop uses Protocol Timer
 * * * * Release 3.02  * * *
 *		05/17/99	pjr		make FreeRequestControlBlock global
 *		05/06/99	pjr		fix VxWorks compiler warnings
 *		05/03/99	pjr		WcHttpFiniteStateMachine now returns error code
 *		04/30/99	bva		RomPagerDebug -> AsDebug
 * * * * Release 3.01  * * *
 *		04/16/99	rhb		fix memory initialization error handling,
 *							add Soft Page Initialization & Request Blocks
 * * * * Release 3.0 * * * *
 *		04/02/99	pjr		fix compiler warning
 *		03/22/99	pjr		add eRpConnectionWcAvailable to HandleConnectionTask
 *		03/16/99	pjr		rework debug code
 *		03/01/99	pjr		add more Init Flags
 *		02/26/99	bva		change connection setup definitions
 *		02/22/99	bva		rework global data allocation
 *		02/11/99	bva		rework persistent timer setting
 *		02/04/99	pjr		initialize fProtocol in HandleConnectionTask
 *								instead of AllegroTaskInit
 *		01/29/99	bva		add Etag support
 *		01/29/99	pjr		add SudpDeInit call
 *		01/24/99	pjr		change HandleUserExit to RpHandleUserExit and move
 *								to RpHttp.c
 *		01/23/99	pjr		move RpSendReplyBuffer to RpHttp.c, and
 *								RpStartHttpResponse to RpHttpRq.c
 *		01/10/99	pjr		merge with Mail Client scheduler
 *		01/04/99	pjr		rework AllegroTaskInit and AllegroTaskDeInit
 *		12/31/98	pjr		rework to create a common scheduler for Allegro
 *							products.  move some routines to RpAccess.c and
 *							RpCallBk.c.
 *		12/30/98	bva		eRpHttpClient -> eRpHttpProxy,
 *							eRpWebClient -> eRpHttpClient
 *		12/29/98	bva		merge RomWebClient
 *		12/10/98	pjr		Entry point names change from
 *								RomPagerXxxx -> AllegroXxxx
 * * * * Release 2.2 * * * *
 *		11/30/98	bva		fix compile warnings
 *		11/23/98	bva		rework date initialization
 *		11/17/98	bva		add theDataPtr to SpwdGetExternalPassword
 *		11/15/98	bva		add set up for fExpiresDateString
 *		11/12/98	rhb		give a type to fDataPtr in the request structure
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		10/27/98	pjr		eliminate RpHandleWaitingForRHReceive
 *		10/22/98	pjr		errors in RpSendReplyBuffer are handled by the
 *							callback routine, no longer need to return an error
 *		10/21/98	bva		clean up Server Push
 *		10/06/98	bva		clear fErrorProcPtr for persistent connections
 *		10/03/98	bva		merge in RomDns support
 *		09/21/98	pjr		initialize the file states in the connection block.
 *		09/11/98	bva		use HandleProtocolAction for
 *								eRpConnectionProtocolWaitExternal
 *		09/03/98	bva		modify HandleExternalSecurity to allow return of
 *								eRpPasswordAuthorized
 *		09/01/98	bva		rpParsingControl moves to connection block
 *		09/01/98	pjr		set fields in theConnectionPtr->fParsingControl
 *		08/31/98	rhb		rework connection error for RomMailer and RomPop
 *		08/26/98	pjr		clear the abort and persistence timers in
 *								RpConnectionAbortTcp
 *		08/25/98	pjr		move HandleWaitingForRHReceive to RpRemHst.c
 *		08/20/98	pjr		rework connection error handling
 *		07/23/98	pjr		fix compiler warnings
 *		07/18/98	bva		add debugging support for HTTP flow
 *		07/01/98	pjr		add calls to RpUserExitInit and RpUserExitDeInit
 *		06/23/98	bva		add protocol timer support
 *		06/22/98	pjr		add Http Client support to RpHandleSendReceiveError
 *		06/16/98	bva		add initialization of user data areas
 *		06/08/98	bva		add support for multiple Remote Hosts
 *		06/06/98	bva		RomPagerGroupedTasking -> RomPagerSingleTasking
 *		06/04/98	bva		add RpSetCurrentConnection
 *		06/03/98	bva		fix RpHandleSendReceiveError for
 *							IPP persistent connections
 * * * * Release 2.1 * * * *
 *		04/29/98	bva		add RomPagerConnectionTask, RomPagerTimerTask
 *		04/28/98	bva		add init of IPP printer name and cleanup of
 *								IPP parser on connection close
 *		04/16/98	bva		clear statically initialized date in RomPagerInit
 *		04/02/98	rhb		don't change session status in
 *								RpHandleSendReceiveError if already error
 *		04/02/98	bva		add user exit support for redirect and
 *								not modified
 *		03/26/98	pjr		use theCgiPtr->fDataType for User Exit URLs
 *		03/04/98	rhb		remove unused variable in HandleActiveAvailable,
 *								add cast to theReceiveLength in
 *								HandleReceiveBuffer
 *		02/24/98	pjr		rework phrase dictionary patching feature to be
 *							connection block driven.
 *		02/18/98	rhb		free HTTP request blocks only if dynamically
 *								allocated and move HandleWaitingForRPReceive
 *								to PrPop3.c as PrHandleWaitingForReceive
 *		02/17/98	pjr		rework some remote host protocol states.  remove
 *							eRpWaitingForActiveConnection state and
 *							HandleWaitingForActiveConnection routine.
 *		02/14/98	bva		eRpConnectionRemoteHostWait->eRpConnectionHolding,
 *							clean up HandleWaitingForActiveConnection
 *		02/02/98	bva		HandleSendReceiveError->RpHandleSendReceiveError
 *		01/23/98	pjr		add phrase dictionary patching feature
 *		01/21/98	bva		add eRpConnectionProtocolWaitExternal,
 *							eRpConnectionWaitingForRHReceive
 *							add error check on kStcpNumberOfConnections
 *		01/20/98	bva		rework RomMailer initial states
 *		01/19/98	bva		add eRpConnectionNeedsProtocolAction
 *							remove eRpConnectionNeedsHttpAction,
 *							eRpConnectionNeedsRemoteHostAction,
 *							eRpConnectionNeedsSmtpAction
 *		01/16/98	bva		add error handling on StcpOpenActive for SMTP
 *		01/13/98	pjr		call RpLoggingInit from RomPagerInit
 *		01/13/98	rhb		add POP3 support
 *		01/13/98	rhb/bva	support Trace Method for busy environments
 * * * * Release 2.0 * * * *
 *		11/26/97	pjr		make RpConnectionCloseTcp global
 *		11/07/97	pjr		add Remote Host feature
 *							eRpConnectionWaitingForRequest ->
 *							eRpConnectionWaitingForReceive
 *		11/03/97	pjr		parsing changes (use rpParsingControl)
 *		11/03/97	rhb		add IPP init/deinit
 *		10/28/97	bva		rework User Exit support
 *		10/21/97	bva		rework request block allocation for initial pool
 *		10/16/97	bva		fix RpConnectionCloseTcp for persistent
 *								connection timeout
 *		10/10/97	bva		fix compiler warnings
 *		10/07/97	pjr		add dynamic request block allocation.
 *		09/24/97	pjr		change gUserPhrases to gUserPhrasesEnglish.
 *		09/14/97	bva		integrate RomMailer
 *		08/21/97	bva		rework URL dispatching
 *		08/11/97	pjr		conditionalize fPersistenceTimer code.
 *		08/07/97	bva		rework persistent connection closing
 *		08/03/97	bva		add call to RpGetChunkedObjectData
 *		07/31/97	edk		rework RpSetUserPhraseDictionary for broader
 *								compiler support
 *		07/26/97	bva		change RpGetObjectData call
 *		07/25/97	pjr		fPatternTable -> fHttpPatternTable.
 *							initialize Multipart pattern recognition tables.
 *							RpProcessMultipart now returns theReadMoreFlag.
 *		07/21/97	bva		narrow window of receive control block usage
 *		07/20/97	bva		change RomPagerMainTask to return
 *								number of active TCP tasks
 *		07/12/97	bva		add HTTP->IPP interface
 *		07/10/97	bva		modify HTTP header parsing
 *		07/07/97	pjr		change Multipart parsing enums.
 *		06/26/97	pjr		add form based file upload feature.
 *		05/24/97	bva		rework URL dispatching
 * * * * Release 1.6 * * * *
 *		05/14/97	rhb		initialize *theHttpTasks
 *		04/26/97	bva		return fHttpTaskCount from RomPagerMainTask
 *		04/22/97	bva		fix server push in RpConnectionCheckTcpClose
 *		04/18/97	pjr		add Security Digest feature
 *		04/13/97	bva		fix fHttpTransactionState usage
 *		04/04/97	bva		cleanup warnings
 *		03/26/97	bva		change error codes
 *		03/24/97	bva		server push debugging
 *		03/15/97	bva		server push
 *		03/10/97	bva		rework connection states
 *		02/10/97	bva		cleanup warnings
 *		02/02/97	bva		rpDataPtr stored in rpHttpRequest
 *								UserDataPtr -> Cookie
 *		01/31/97	bva		add selectable user dictionary
 *		01/26/97	rhb		eliminate Boolean and return from RomPagerDeInit
 *								eliminate Boolean from RomPagerMainTask
 *		01/24/97	rhb		initialize and shut down the file server and
 *								remove unused variable
 *		01/20/97	bva		add RpGetRequestCookie, RpSetRequestCookie,
 *								RpSetUserFormItemFunction
 *		01/16/97	bva		add RpSetCookie, RpGetCookie
 *		12/13/96	bva		add file system support
 *		12/06/96	bva		add Keep-Alive support
 * * * * Release 1.5 * * * *
 *		11/05/96	bva		eliminate unused variable
 *		10/22/96	bva		add delayed function support
 *		10/18/96	bva		consolidate HTTP headers and form items buffers
 *		10/16/96	bva		rework abort handling
 *		10/03/96	bva		add conditional compile flags for security
 *		10/02/96	bva		rework port support for passive opens
 *		09/24/96	rhb		support dynamically allocated engine data
 *		09/20/96	rhb		allow more than one HTTP request
 * * * * Release 1.4 * * * *
 *		08/06/96	bva		initialize gRpHttpRequestPtr in RomPagerInit
 * * * * Release 1.3 * * * *
 *		07/05/96	bva		explicitly handle TCP TimeWait state
 *		06/28/96	bva		rework connection states for memory savings
 * * * * Release 1.2 * * * *
 *		05/31/96	bva		use RomPagerDebug for gRpHighestUsedConnection
 * * * * Release 1.1 * * * *
 *		04/25/96	bva		rework top level multi-tasking in preparation for
 *							Keep-Alive support
 * * * * Release 1.0 * * * *
 *		11/01/95	bva		created
 *
 *	To Do:
 */

#include "AsEngine.h"
#include "AsProtos.h"

#if 0   /*Elsa modify for web*/
// +++ _Alphanetworks_Patch_, 02/26/2003, jacob_shih
#include <utl\inc\os_adpt.h>
//#include <dev/inc/utl_if.h>
#endif
// --- _Alphanetworks_Patch_, 02/26/2003, jacob_shih

// +++ _Alphanetworks_Patch_, 12/20/2003, jacob_shih
#ifndef WEB_MAX_AGE
	#define WEB_MAX_AGE		(10 * 24 * 60 * 60)	// expires after 10 days
#endif
// --- _Alphanetworks_Patch_, 12/20/2003, jacob_shih

#if 0   /*Elsa modify for web*/
//#include "SSL.h"
#include <web/ssl/inc/SSL.h>
#include <web/ssl/porting/inc/dlk_ssl.h>
#include <l3/port/inc/la3_os.h> 
#endif

extern void SSLLibraryInit(void);
#define SESSION_RESUME		0

Signed32 gAcceptFD;

#undef WEB_ROM_DATE_STRING
#define WEB_ROM_DATE_STRING		"2004/02/11 09:49PM"

// +++ _Alphanetworks_Patch_, 12/20/2003, jacob_shih
#ifndef WEB_ROM_DATE_STRING
	#error WEB_ROM_DATE_STRING is not defined
	#define WEB_ROM_DATE_STRING		"2004/01/31 09:49PM"
#endif

#ifndef WEB_MAX_AGE
	#define WEB_MAX_AGE		(10 * 24 * 60 * 60)	// expires after 10 days
#endif
// --- _Alphanetworks_Patch_, 12/20/2003, jacob_shih

#define RomPagerHouseDemo	0

#if RomConsole || RomPagerHouseDemo
#include "Sser.h"
#endif

#if RomPagerServer || RomCli
extern char *	gUserPhrasesEnglish[];
#endif

/*
	Determine whether there's a TCP interface or not.
*/

#if (RomPagerBasic || RomPagerServer || RomMailerBasic || RomMailer || \
		RomPopBasic || RomPop || RomWebClient || RomTelnet || RomCliSecure)
#define AsHasTcpInterface	1
#else
#define AsHasTcpInterface	0
#endif


/*
	Initialization flags.
*/
#define	kRpInitFlagIpp				(0x0001)
#define	kRpInitFlagFileSystem		(kRpInitFlagIpp << 1)
#define	kRpInitFlagSerial			(kRpInitFlagFileSystem << 1)
#define	kRpInitFlagTcp				(kRpInitFlagSerial << 1)
#define	kRpInitFlagUdp				(kRpInitFlagTcp << 1)
#define	kRpInitFlagRomMailer		(kRpInitFlagUdp << 1)
#define	kRpInitFlagRomMailerBasic	(kRpInitFlagRomMailer << 1)
#define	kRpInitFlagRomPop			(kRpInitFlagRomMailerBasic << 1)
#define	kRpInitFlagRomPopBasic		(kRpInitFlagRomPop << 1)
#define	kRpInitFlagWebClient		(kRpInitFlagRomPopBasic << 1)
#define	kRpInitFlagRomDns			(kRpInitFlagWebClient << 1)
#define	kRpInitFlagRomXml			(kRpInitFlagRomDns << 1)
#define	kRpInitFlagUserExit			(kRpInitFlagRomXml << 1)
#define	kRpInitFlagSoftPage			(kRpInitFlagUserExit << 1)
#define	kRpInitFlagRomPlug			(kRpInitFlagSoftPage << 1)
#define	kRpInitFlagRomPlugControl	(kRpInitFlagRomPlug << 1)
#define kRpInitFlagTls				(kRpInitFlagRomPlugControl << 1)
#define	kRpInitFlagRomTelnet		(kRpInitFlagTls << 1)
#define	kRpInitFlagRomCli			(kRpInitFlagRomTelnet << 1)
#define	kRpInitFlagRomConsole		(kRpInitFlagRomCli << 1)
#define	kRpInitFlagRomTime			(kRpInitFlagRomConsole << 1)
#define kRpInitFlagRomCliSecure		(kRpInitFlagRomTime << 1)


static void			DeallocateGlobals(rpDataPtr theDataPtr);
static RpErrorCode	HandleConnectionTask(rpConnectionPtr theConnectionPtr, int fd);
static RpErrorCode	HandleRHConnectionTask(rpConnectionPtr theConnectionPtr);
static RpErrorCode	HandleProtocolAction(rpConnectionPtr theConnectionPtr);
static RpErrorCode	HandleTimerActions(rpDataPtr theDataPtr, int fd);

#if AsHasTcpInterface
static RpErrorCode	AbortConnection(rpConnectionPtr theConnectionPtr);
static RpErrorCode	HandleClosing(rpConnectionPtr theConnectionPtr);
static RpErrorCode	HandleReceive(rpConnectionPtr theConnectionPtr);
static RpErrorCode	HandleSendingReply(rpConnectionPtr theConnectionPtr);
static RpErrorCode	HandleWaitingForReceive(rpConnectionPtr theConnectionPtr);

#if RomPagerServer || RomPagerBasic || RomTelnet || RomCliSecure
static RpErrorCode	HandleWaitingForConnection(rpConnectionPtr theConnectionPtr, int fd);
static RpErrorCode	SetServerProtocol(rpConnectionPtr theConnectionPtr);
#endif

#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
static RpErrorCode	HandleNeedsRequestBlock(rpConnectionPtr theConnectionPtr);
static RpErrorCode	HandleReceiveBuffer(rpConnectionPtr theConnectionPtr);
#endif

#if (kStcpActiveConnections > 0)
static void			HandleActiveAvailable(rpConnectionPtr theConnectionPtr);
#endif
#endif	/* AsHasTcpInterface */

#if RomDns || RomPlug || RomPlugControl || RomTime
static void			HandleUdpAvailable(rpConnectionPtr theConnectionPtr);
#endif

#if AllegroMultiTasking
static RpErrorCode	HandleMultiTaskRescheduling(rpDataPtr theDataPtr, int fd);
#endif

#if RomPagerSecure
#if 0
static RpErrorCode 	HandleWaitingForTlsHandshake(rpConnectionPtr theConnectionPtr);
#endif
#endif


#if (AsDebug && (RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption))
/*
	This variable is useful for determining how many TCP/IP connections
	need to be allocated for optimal performance.  The variables
	kConnectionCloseTimeout and kStcpNumberOfConnections trade off
	against each other.
*/
static Unsigned16 gRpHighestUsedConnection;
#endif

char firmware_data[]=""__DATE__" "__TIME__;

int dlk_GetWebDefaultTimeout(void)
{
	return 60;
}
int webDefaultTimeout =60;
// -------------------Modify by jesson for timeout issue 7/26/2002
extern int	webDefaultTimeout;		// using this to record the timeout control section
extern int dlk_GetWebDefaultTimeout(void); // add by jesson 


//-- end add 91.8.14

/*
	Demons execute initialization code that completes asynchronously.
	This is the list of demon functions.
*/
static asDemonFuncPtr gDemonList[] = {
#if RomCliSecure
	ShInitStatus,
#endif
	(asDemonFuncPtr) 0
};


void *AllegroTaskInit(int sessionID) {
	rpDataPtr	theDataPtr;
	int			theFlag = True;

	theDataPtr = (rpDataPtr) AllegroTaskInitMemory(sessionID);
	
	if (theDataPtr != (rpDataPtr) 0) {
#if RomPagerServer || RomPagerBasic
		theFlag = AllegroTaskInitPort(theDataPtr,
									kDefaultHttpPort,
									eAsHttpServer);
#endif
#if RomPlugControl && CpEventSupport
		theFlag = AllegroTaskInitPort(theDataPtr,
									kCpEventPort,
									eAsHttpServer);
#endif
#if RomPagerIpp
		if (theFlag) {
			theFlag = AllegroTaskInitPort(theDataPtr,
									kDefaultIppPort,
									eAsIppServer);
		}
#endif
#if RomPagerSecure
		if (theFlag) {
			theFlag = AllegroTaskInitPort(theDataPtr,
									kDefaultTlsPort,
									eAsTlsServer);
		}
#endif	/* RomPagerSecure */
#if RomTelnet
		if (theFlag) {
			theFlag = AllegroTaskInitPort(theDataPtr,
									kDefaultTelnetPort,
									eAsTelnetServer);
		}
#endif
#if RomCliSecure
        if (theFlag) {
            theFlag = AllegroTaskInitPort(theDataPtr,
                                    kDefaultSshPort,
                                    eAsSshServer);
        }
#endif

		if (theFlag) {
			theFlag = AllegroTaskInitStart(theDataPtr, sessionID);
		}
		if (!theFlag) {
			theDataPtr = (rpDataPtr) 0;
		}
	}
	return theDataPtr;
}

extern void Initial_CHALLENGE_LOGIN_DataBase(void);

void *AllegroTaskInitMemory(int sessionID) {
	rpConnectionPtr		theConnectionPtr;
	Unsigned32			theDataLength;
	rpDataPtr			theDataPtr;
	Unsigned8			theIndex;
	RpErrorCode			theResult = eRpNoError;
#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
#if (kRpNumberOfHttpRequests > 0)
#if RomPagerSoftPages
	rpHttpRequestPtr	theRequestPtr;
	spRequestPtr		theSoftPageRequestPtr;
#endif
#endif
#endif
#if RomPagerServer || RomPagerBasic
	Unsigned32			theExpiresSeconds;
#endif

	// add by Arthur 2004/02/03
	Initial_CHALLENGE_LOGIN_DataBase();

	/*
		Allocate any initial memory.
	*/

	//--- end add 91.8.14
	
	theDataLength = sizeof(rpData);

#if RomPagerDynamicGlobals
	theDataPtr = (rpDataPtr) RP_ALLOC(theDataLength);
#else
//      theDataPtr = &gRpData; // vic edited (04443 masked) for fixed timer. but not efficient.
// Use Multi-Session. - Arthur Chow
    theDataPtr=&gRpData[sessionID];//~jesson Rp4.0 , 
#endif

	if (theDataPtr != (rpDataPtr) 0) {
		char szOSDate[256];
        memcpy(szOSDate, firmware_data, sizeof(firmware_data));

		/*
			Clear the data block.
		*/
		RP_MEMSET(theDataPtr, 0, theDataLength);

		/*
			Initialize date handling.
		*/
		
#if RomPagerUseStandardTime && RomPagerCalendarTime
		theDataPtr->fEpochReference = RpComputeEpochReference();
#endif
// +++ _Alphanetworks_Patch_, 12/20/2003, jacob_shih
//	get rom seconds with pre-defined rom date string.
#if 1
		theDataPtr->fRomSeconds =
				RpParseOSDate(theDataPtr, szOSDate);
#else
		theDataPtr->fRomSeconds =
				RpGetMonthDayYearInSeconds(theDataPtr,
				kHttpRomMonth, kHttpRomDay, kHttpRomYear);
#endif
// --- _Alphanetworks_Patch_, 12/20/2003, jacob_shih

#if RomPagerServer || RomPagerBasic
		RpBuildDateString(theDataPtr, theDataPtr->fRomDateString,
							theDataPtr->fRomSeconds);

// +++ _Alphanetworks_Patch_, 12/20/2003, jacob_shih
//	set expire date to the WEB_MAX_AGE after rom date
#if 1
		theExpiresSeconds = theDataPtr->fRomSeconds + WEB_MAX_AGE;
#else
		theExpiresSeconds =
				RpGetMonthDayYearInSeconds(theDataPtr, 10, 26, 1995);
#endif
// --- _Alphanetworks_Patch_, 12/20/2003, jacob_shih

		RpBuildDateString(theDataPtr, theDataPtr->fExpiresDateString,
							theExpiresSeconds);
#if RpEtagHeader
		RpBuildEtagString(theDataPtr->fRomEtagString, theDataPtr->fRomSeconds);
#endif
#endif	/* RomPagerServer || RomPagerBasic */

#if RomPagerServer || RomPagerBasic
		/*
			Initialize the HTTP header pattern recognition table.
		*/
		RpInitPatternTable(theDataPtr->fHttpPatternTable);
#endif

#if RomPagerServer
#if RomPagerSecurity
        
		//theDataPtr->fPasswordSessionTimeout = kPasswordSessionTimeout;

		
		//from Olivia version, modify by jesson 91.7.30
		webDefaultTimeout=60*dlk_GetWebDefaultTimeout();
		theDataPtr->fPasswordSessionTimeout=webDefaultTimeout;
		//--- end modify
#endif

#if RomPagerSecurityDigest
		/*
			Initialize the authorization pattern recognition table.
		*/
		RpInitAuthPatternTable(theDataPtr->fAuthPatternTable);
#endif

#if RomPagerFileUpload
		/*
			Initialize the Multipart pattern recognition tables.
		*/
		RpInitMpPatternTable(theDataPtr->fMpPatternTable);
		RpInitDispositionPatternTable(theDataPtr->fDispositionPatternTable);
#endif
#endif	/* RomPagerServer */

#if RomMailer
		if (theResult == eRpNoError) {
			theResult = RmSmtpInit(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomMailer;
			}
		}
#endif

#if RomMailerBasic
		if (theResult == eRpNoError) {
			theResult = ScSmtpInit(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomMailerBasic;
			}
		}
#endif

#if RomPop
		if (theResult == eRpNoError) {
			theResult = PrRomPopInit(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomPop;
			}
		}
#endif

#if RomPopBasic
		if (theResult == eRpNoError) {
			theResult = PcRomPopInit(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomPopBasic;
			}
		}
#endif

#if RomWebClient
		if (theResult == eRpNoError) {
			theResult = WcWebClientInit(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagWebClient;
			}
		}
#endif

#if RomPlug
		if (theResult == eRpNoError) {
			theResult = RuInitUpnpDevice(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomPlug;
			}
		}
#endif

#if RomPlugControl
		if (theResult == eRpNoError) {
			theResult = CpInitUpnpControlPoint(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomPlugControl;
			}
		}
#endif

#if RomDns
		if (theResult == eRpNoError) {
			theResult = RdInitDns(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomDns;
			}
		}
#endif

#if RomTime
		if (theResult == eRpNoError) {
			theResult = TmInitTime(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomTime;
			}
		}
#endif

#if RomXml
		if (theResult == eRpNoError) {
			theDataPtr->fRxDataPtr = RxInit();
			if (theDataPtr->fRxDataPtr != (void *) 0) {
				theDataPtr->fInitFlags |= kRpInitFlagRomXml;
			}
			else {
				theResult = eRpOutOfMemory;
			}
		}
#endif

#if RomConsole
		if (theResult == eRpNoError) {
			theResult = CsInitConsole(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomConsole;
			}
		}
#endif

#if RomTelnet
		if (theResult == eRpNoError) {
			theResult = TnInitTelnet(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomTelnet;
			}
		}
#endif

#if RomCliSecure
        if (theResult == eRpNoError) {
			AcRandomInit(theDataPtr);
			BnMemMgrInit(theDataPtr);
            theResult = ShInitRomCliSecure(theDataPtr);
            if (theResult == eRpNoError) {
                theDataPtr->fInitFlags |= kRpInitFlagRomCliSecure;
            }
        }
#endif

#if RomCli
		if (theResult == eRpNoError) {
			theResult = RcInitCli(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagRomCli;
			}
		}
#endif

#if RomPagerServer || RomCli
		/*
			Initialize the default User Phrase dictionary.
		*/
		theDataPtr->fUserPhrases = (char **) gUserPhrasesEnglish;
		theDataPtr->fUserPhrasesCanBeCompressed = True;

		/*
			Initialize the device specific information.
		*/
		if (theResult == eRpNoError) {
			RpInitializeBox(theDataPtr);
		}
#endif

#if RomPagerServer || RomPagerBasic || RmFileOptions
#if (kRpNumberOfHttpRequests > 0)
#if RomPagerSoftPages
		/*
			Initialize Soft Pages.
		*/
		if (theResult == eRpNoError) {
			theResult = RpInitializeSoftPages(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagSoftPage;
				theSoftPageRequestPtr =
						theDataPtr->fSoftPageDataPtr->fRequests;
				/*
					Initialize the request structures.
				*/
				theRequestPtr = theDataPtr->fHttpRequests;
				while (theRequestPtr <
						&theDataPtr->fHttpRequests[kRpNumberOfHttpRequests]) {
					theRequestPtr->fSoftPageRequestPtr = theSoftPageRequestPtr;
					theSoftPageRequestPtr += 1;
					theRequestPtr += 1;
				}
			}
		}
#endif
#endif
#endif

		/*
			Initialize all the connections to the closed state,
			and save away the master data pointer.
		*/
		theIndex = 0;
		theConnectionPtr = theDataPtr->fConnections;
		while (theIndex < kNumberOfConnections) {
//jesson
			theConnectionPtr->fIndex = theIndex;
//			theConnectionPtr->fState = eRpConnectionClosed;
//            theConnectionPtr->fIndex = sessionID;
			if (theIndex < kStcpActiveBase)
				theConnectionPtr->fState = eRpConnectionWaitingForConnection;
			else
				theConnectionPtr->fState = eRpConnectionActiveAvailable;

			theConnectionPtr->fDataPtr = theDataPtr;
			theIndex += 1;
			theConnectionPtr += 1;
		}

		if (theResult != eRpNoError) {
			AllegroTaskDeInit(theDataPtr);
			theDataPtr = (rpDataPtr) 0;
		}
	}
	return theDataPtr;
}


/*
	AllegroTaskInitPort - Initialize a listener port.

	This routine is called to set up a TCP listener port and associate it
	with a particular protocol.  The port information is saved, and used
	later when initializing the TCP interface.

	The number of server ports (listener ports) allowed is defined by
	kStcpServerCount, in Stcp.h.  If an attempt is made to set up more
	server ports than are allowed by kStcpServerCount, this function
	will return false.

	Inputs:
		theTaskDataPtr		- pointer to the tasks data structure
		thePort				- port number to listen on
		theProtocol			- protocol to associate with the port

	Returns:
		True				- the port information has been accepted
		False				- can't set up a the port as requested
*/

int	AllegroTaskInitPort(void *theTaskDataPtr,
						int thePort,
						asProtocolType theProtocol) {

#if (kStcpServerCount > 0)
	rpDataPtr	theDataPtr = (rpDataPtr) theTaskDataPtr;
	Unsigned8	theIndex;

	if (theDataPtr == (rpDataPtr) 0) {
		return False;
	}

	theIndex = theDataPtr->fServerPortIndex;

	if (theIndex < kStcpServerCount) {
		theDataPtr->fServerPorts[theIndex].fPort = thePort;
		theDataPtr->fServerPorts[theIndex].fProtocol =
				(rpConnectionType) theProtocol;
		theDataPtr->fServerPortIndex = theIndex + 1;
		/*
			The port configuration has been accepted.
		*/
		return True;
	}
#endif

	/*
		We either ran out of server ports or there are none
		allowed at all (kStcpServerCount = 0).
	*/
	return False;
}


int AllegroTaskInitStart(void *theTaskDataPtr, int sessionID) {
	rpDataPtr		theDataPtr = (rpDataPtr) theTaskDataPtr;
	Unsigned8		theIndex = 0;
	RpErrorCode		theResult = eRpNoError;
#if AsHasTcpInterface && (kStcpServerCount > 0)
	Unsigned8		theServerCount;
	StcpPort		theServerPortArray[kStcpServerCount];
#endif

	if (theDataPtr != (rpDataPtr) 0) {

#if AsHasTcpInterface
		/*
			Start up the TCP handler.
		*/
		theResult = StcpInit();
		if (theResult == eRpNoError) {
			theDataPtr->fInitFlags |= kRpInitFlagTcp;
#if (kStcpServerCount > 0)
			theServerCount = theDataPtr->fServerPortIndex;
			/*
				If there are any servers, notify the Stcp
				interface so it can set them up.
			*/
			if (theServerCount > 0) {
				while (theIndex < theServerCount) {
					theServerPortArray[theIndex] =
							theDataPtr->fServerPorts[theIndex].fPort;
					theIndex++;
				}
				theResult = StcpSetServerPortNumbers(theServerCount,
						theServerPortArray);
			}
#endif	/* (kStcpServerCount > 0) */
		}
#endif	/* AsHasTcpInterface */

#if (kSudpNumberOfConnections > 0)
		/*
			Start up the UDP handler.
		*/
		if (theResult == eRpNoError) {
			theResult = SudpInit();
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagUdp;
			}
		}
#endif

#if RomConsole || RomPagerHouseDemo
		/*
			Start up the Serial interface.
		*/
		if (theResult == eRpNoError) {
			theResult = SserInit();
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagSerial;
			}
		}
#endif

#if RomPagerFileSystem
		/*
			Start up the File System handler.
		*/
		if (theResult == eRpNoError) {
			theResult = SfsOpenFileSystem(kNumberOfFiles);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagFileSystem;
			}
		}
#endif

#if RomPagerSecure || RomWebClientSecure
		/*
			Initialize SSL/TLS.
		*/
		if (theResult == eRpNoError) {
			/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
			/* theResult = SsiInit(theDataPtr); */
			if(sessionID ==0)
            	SSLLibraryInit();
			/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagTls;
			}
		}
#endif

#if RomPagerUserExit
		/*
			Initialize the User Exit feature.
		*/
		if (theResult == eRpNoError) {
			theResult = RpUserExitInit(theDataPtr);
			if (theResult == eRpNoError) {
				theDataPtr->fInitFlags |= kRpInitFlagUserExit;
			}
		}
#endif

#if RomPagerRemoteHost || RomPagerLogging || RomPagerIpp

		if (theResult == eRpNoError) {
#if RomPagerRemoteHost
			RpRemoteHostInit(theDataPtr);
#endif

#if RomPagerLogging
			RpLoggingInit(theDataPtr);
#endif

#if RomPagerIpp
			theDataPtr->fIppDataPtr = IppInit(theDataPtr);
			if (theDataPtr->fIppDataPtr != (void *) 0) {
				/*
					Indicate successful IPP initialization.
				*/
				theDataPtr->fInitFlags |= kRpInitFlagIpp;
			}
			else {
				/*
					IPP initialization was unsuccessful,
					so indicate an error.
				*/
				theResult = eRpOutOfMemory;
			}
#endif
		}

#endif	/* RomPagerRemoteHost || RomPagerLogging || RomPagerIpp */

#if AsResourceLocks
		if (theResult == eRpNoError) {
			/*
				Initialize resource locks.
			*/
			theDataPtr->fLockDataPtr = AsCreateLocks(theDataPtr);
			if (theDataPtr->fLockDataPtr == (void *) 0) {
				theResult = eRpOutOfMemory;
			}
		}
#endif

		if (theResult == eRpNoError) {
			return True;
		}
		else {
			AllegroTaskDeInit(theDataPtr);
			return False;
		}
	}
	else {
		return False;
	}
}


void AllegroTaskDeInit(void *theTaskDataPtr) {
	rpDataPtr		theDataPtr = (rpDataPtr) theTaskDataPtr;
	Unsigned32		theInitFlags;
#if AsHasTcpInterface || (kSudpNumberOfConnections > 0)
	rpConnectionPtr	theConnectionPtr;
#endif
#if (kSudpNumberOfConnections > 0)
	Unsigned8		theCount;
#endif

	if (theDataPtr != (rpDataPtr) 0) {

		theInitFlags = theDataPtr->fInitFlags;

#if RomPagerIpp
		if (theInitFlags & kRpInitFlagIpp) {
			IppDeInit(theDataPtr->fIppDataPtr);
		}
#endif

#if RomPop
		if (theInitFlags & kRpInitFlagRomPop) {
			PrRomPopDeInit(theDataPtr);
		}
#endif

#if RomPopBasic
		if (theInitFlags & kRpInitFlagRomPopBasic) {
			PcRomPopDeInit(theDataPtr);
		}
#endif

#if RomMailer
		if (theInitFlags & kRpInitFlagRomMailer) {
			RmSmtpDeInit(theDataPtr);
		}
#endif

#if RomMailerBasic
		if (theInitFlags & kRpInitFlagRomMailerBasic) {
			ScSmtpDeInit(theDataPtr);
		}
#endif

#if RomTelnet
		if (theInitFlags & kRpInitFlagRomTelnet) {
			TnTelnetDeInit(theDataPtr);
		}
#endif

#if RomConsole
		if (theInitFlags & kRpInitFlagRomConsole) {
			CsConsoleDeInit(theDataPtr);
		}
#endif

#if RomCliSecure
        if (theInitFlags & kRpInitFlagRomCliSecure) {
            ShRomCliSecureDeInit(theDataPtr);
        }
#endif

#if RomCli
		if (theInitFlags & kRpInitFlagRomCli) {
			RcCliDeInit(theDataPtr);
		}
#endif

#if AsHasTcpInterface
		if (theInitFlags & kRpInitFlagTcp) {
			theConnectionPtr = theDataPtr->fConnections;
			while (theConnectionPtr <
					&theDataPtr->fConnections[kStcpNumberOfConnections]) {

#if (kStcpActiveConnections > 0)
			if (theConnectionPtr->fIndex >= kStcpActiveBase) {
				if (theConnectionPtr->fState != eRpConnectionClosed &&
						theConnectionPtr->fState !=
						eRpConnectionActiveAvailable) {
					(void) StcpAbortConnection(theConnectionPtr->fIndex);
				}
			}
			else {
				if (theConnectionPtr->fState != eRpConnectionClosed) {
					(void) StcpAbortConnection(theConnectionPtr->fIndex);
				}
			}
#else	/* (kStcpActiveConnections > 0) */
				if (theConnectionPtr->fState != eRpConnectionClosed) {
					(void) StcpAbortConnection(theConnectionPtr->fIndex);
				}
#endif	/* (kStcpActiveConnections > 0) */

				theConnectionPtr += 1;
			}

			/*
				Close down the TCP handler.
			*/
			(void) StcpDeInit();
		}
#endif	/* AsHasTcpInterface */

#if RomDns
		if (theInitFlags & kRpInitFlagRomDns) {
			RdDeInitDns(theDataPtr);
		}
#endif

#if RomPlug
		if (theInitFlags & kRpInitFlagRomPlug) {
			RuUpnpDeviceDeInit(theDataPtr);
		}
#endif

#if RomPlugControl
		if (theInitFlags & kRpInitFlagRomPlugControl) {
			CpDeInitUpnpControlPoint(theDataPtr);
		}
#endif

#if RomTime
		if (theInitFlags & kRpInitFlagRomTime) {
			TmDeInitTime(theDataPtr);
		}
#endif

#if (kSudpNumberOfConnections > 0)
		if (theInitFlags & kRpInitFlagUdp) {
			theConnectionPtr =
 					theDataPtr->fConnections + kStcpNumberOfConnections;
			theCount = 0;
			while (theCount < kSudpNumberOfConnections) {
				if (theConnectionPtr->fState != eRpConnectionClosed &&
						theConnectionPtr->fState != eRpConnectionUdpAvailable) {
					(void) SudpCloseConnection((SudpConnection)
							(theConnectionPtr->fIndex - kStcpNumberOfConnections));
				}
				theConnectionPtr++;
				theCount++;
			}
			(void) SudpDeInit();
		}
#endif	/* kSudpNumberOfConnections > 0 */

#if RomXml
		if (theInitFlags & kRpInitFlagRomXml) {
			RxDeInit(theDataPtr->fRxDataPtr);
		}
#endif

#if RomPagerSoftPages
		if (theInitFlags & kRpInitFlagSoftPage) {
			RpSoftPageDeInit(theDataPtr);
		}
#endif

#if RomWebClient
		if (theInitFlags & kRpInitFlagWebClient) {
			WcWebClientDeInit(theDataPtr);
		}
#endif

#if RomPagerFileSystem
		if (theInitFlags & kRpInitFlagFileSystem) {
			/*
				Close down the File System handler.
			*/
			(void) SfsCloseFileSystem();
		}
#endif

#if RomPagerSecure || RomWebClientSecure
		if (theInitFlags & kRpInitFlagTls) {
			/*
				Close down the TLS environment.
			*/
			/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
			/* (void) SsiDeInit(theDataPtr); */
			/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
		}
#endif

#if RomPagerUserExit
		if (theInitFlags & kRpInitFlagUserExit) {
			/*
				Close any User Exit interface and free it's resources.
			*/
			(void) RpUserExitDeInit();
		}
#endif

#if RomConsole || RomPagerHouseDemo
		if (theInitFlags & kRpInitFlagSerial) {
			/*
				Close down the Serial interface and free it's resources.
			*/
			SserDeInit();
		}
#endif

#if RomPagerServer && RomPagerSecurity
		/*
			Reset any active user sessions.
		*/
		RpSecurityDeInit(theDataPtr);
#endif

#if AsResourceLocks
		/*
			Initialize resource locks.
		*/
		AsDestroyLocks(theDataPtr, theDataPtr->fLockDataPtr);
#endif

		/*
			Deallocate global memory.
		*/
		DeallocateGlobals(theDataPtr);
	}

	return;
}


static void DeallocateGlobals(rpDataPtr theDataPtr) {

#if RomPagerDynamicGlobals
	RP_FREE(theDataPtr);
#endif
	theDataPtr = (rpDataPtr) 0;
	return;
}

//~jesson Rp4.0
void RpGetRealms(int sessionID)
{

    if(sessionID==0)
        return;

    memcpy(gRpData[sessionID].fRealms,gRpData[0].fRealms,sizeof(rpRealm)*kRpNumberOfRealms);
// +++ 11/25/2003, jacob_shih
    memcpy(gRpData[sessionID].fUsers,gRpData[0].fUsers,sizeof(rpUser)*kRpNumberOfUsers);
// --- 11/25/2003, jacob_shih
    return;
}


int AllegroMainTask(void *theTaskDataPtr, int *theHttpTasks,
							int *theTcpTasks) {
	rpConnectionPtr		theConnectionPtr;
	rpDataPtr			theDataPtr = (rpDataPtr) theTaskDataPtr;
	RpErrorCode			theResult;
	rpConnectionState	theState;
	int					theTcpTaskCount;

	/*
		This is the main routine of the Allegro Scheduler.  Here we handle
		the states for each possible connection.

		The Stcp routines are used to maintain multiple connections with
		the browser.  This is primarily necessary because each connection
		closing takes a while and some browsers (Netscape in particular) will
		fire off multiple requests for the graphics that may be in a page.
		The HTTP over TCP protocol is sensitive to the need for a connection
		to support a request.  It is not very sensitive to the processing
		time for a request.

		So..... what we do here is handle all the multiple connection
		issues, using theDataPtr->fConnections to store the individual
		states of the connections.  When we find one that has actually
		opened and has data for us to read, we look for request control
		block (rpHttpRequest) that is not in use.  If we find an available
		request control block, we assign it to the connection and continue
		processing the request.  If we don't, we won't read the data and
		process the request until one frees up.  Since the request can stay
		pended at the TCP layer, it means there can be fewer HTTP request
		control blocks (which are memory expensive) than TCP connection
		blocks (which use little memory for RomPager).
	*/

	theTcpTaskCount = 0;
	*theTcpTasks = 0;
	*theHttpTasks = 0;
	theConnectionPtr = theDataPtr->fConnections;

	while (theConnectionPtr <
			&theDataPtr->fConnections[kNumberOfConnections]) {

		/*
			For each possible connection check out the state and try
			to move it along to the next state.
		*/
		theState = theConnectionPtr->fState;

		/*
			Only increment the TCP task count for TCP connections.
		*/
		if (theConnectionPtr->fIndex < kStcpNumberOfConnections) {
			if (theState != eRpConnectionClosed &&
					theState != eRpConnectionWaitingForConnection &&
					theState != eRpConnectionActiveAvailable) {
				theTcpTaskCount++;
			}
		}
		theResult = HandleConnectionTask(theConnectionPtr, 60);

		theConnectionPtr += 1;
		if (theResult != eRpNoError) {
			/*
				If we have a problem, blow out of the while loop, and
				notify the caller.
			*/
			return False;
		}
	}
	/*
		Now do the general timekeeping tasks, and check for password
		expirations.
	*/
	theResult = HandleTimerActions(theDataPtr, 60);

	if (theResult == eRpNoError) {
		*theTcpTasks = theTcpTaskCount;
#if RomPagerServer || RomPagerBasic
		*theHttpTasks = theDataPtr->fHttpTaskCount;
#endif
		return True;
	}
	else {
		return False;
	}
}


#if AllegroMultiTasking

int AllegroConnectionTask(void *theTaskDataPtr, int theConnection,
							int *theHttpFlag, int *theTcpFlag, int acceptfd) {
	rpConnectionPtr		theConnectionPtr;
	rpConnectionPtr		theRHConnectionPtr;
	rpDataPtr			theDataPtr = (rpDataPtr) theTaskDataPtr;
	RpErrorCode			theResult;
	RpErrorCode			theRHResult;
	rpConnectionState	theState;

	*theTcpFlag = False;
	*theHttpFlag = False;

// +++ 12/10/2003, jacob_shih
//	exception protection, discard the session which index g.e. kStcpActiveBase
	if (theConnection >= kStcpActiveBase) {
		return False;
	}
// --- 12/10/2003, jacob_shih
	/*
		Make sure it's a valid connection number.
	*/
	if (theConnection >= kNumberOfConnections) {
		return False;
	}
// +++ 11/25/2003, jacob_shih
// to copy the realms from gRpData[0] to gRpData[theConnection]
	//~jesson Rp4.0
	RpGetRealms(theConnection);
// --- 11/25/2003, jacob_shih
	theConnectionPtr = theDataPtr->fConnections + theConnection;
	theRHConnectionPtr = theDataPtr->fConnections + kStcpActiveBase + theConnection;
//	theRHConnectionPtr = theDataPtr->fConnections + kStcpActiveBase;
	/*
		Run the connection processing.
	*/

	theResult = HandleConnectionTask(theConnectionPtr, acceptfd);
	do {
	theRHResult = HandleRHConnectionTask(theRHConnectionPtr);
	} while ((theRHConnectionPtr->fState != eRpConnectionActiveAvailable)&&
		     (theRHConnectionPtr->fState != eRpConnectionWaitingForConnection)&&
			 (theRHConnectionPtr->fState != eRpConnectionHolding));
	if (theResult == eRpNoError) {
		/*
			Set up the status information.
		*/
		theState = theConnectionPtr->fState;
		if (
//			theState != eRpConnectionWaitingForConnection &&
#if RomDns || RomTime
			theState != eRpConnectionUdpAvailable &&
#endif
            theState != eRpConnectionActiveAvailable &&
			theState != eRpConnectionClosed) 
        {
            *theTcpFlag = True;
        }
        else
		{
        	return False;//~jesson 
		}
#if RomPagerServer || RomPagerBasic || RmFileOptions
		if (theConnectionPtr->fHttpRequestPtr != (rpHttpRequestPtr) 0) {
			*theHttpFlag = True;
		}
#endif
		return True;
	}
	else {
		return False;
	}
}


int AllegroTimerTask(void *theTaskDataPtr, int fd) {
	rpDataPtr			theDataPtr = (rpDataPtr) theTaskDataPtr;
	RpErrorCode			theResult;

	theResult = HandleTimerActions(theDataPtr, fd);
	if (theResult == eRpNoError) {
		return True;
	}
	else {
		return False;
	}
}

#endif	/* AllegroMultiTasking */


static RpErrorCode HandleConnectionTask(rpConnectionPtr theConnectionPtr, int fd) {
	rpDataPtr			theDataPtr;
	asDemonFuncPtr		theDemonPtr;
	RpErrorCode			theResult;
#if RomPagerServer
	rpConnCloseFuncPtr	theCloseFunctionPtr;
#endif

	Signed32			theSslResult;
	
	/*
		Process a single connection to the next I/O break point.
	*/
	theResult = eRpNoError;
	theDataPtr = theConnectionPtr->fDataPtr;
	theDataPtr->fCurrentConnectionPtr = theConnectionPtr;
#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
	theDataPtr->fCurrentHttpRequestPtr = theConnectionPtr->fHttpRequestPtr;
#endif

	switch (theConnectionPtr->fState) {

		case eRpConnectionClosed:


#if RomPagerServer
			/*
				Notify application that connection was closed.
			*/
			theCloseFunctionPtr = theConnectionPtr->fCloseFuncPtr;
			if (theCloseFunctionPtr != (rpConnCloseFuncPtr) 0) {
				theCloseFunctionPtr(theDataPtr, theConnectionPtr->fCloseCookie);
				theConnectionPtr->fCloseFuncPtr = (rpConnCloseFuncPtr) 0;
			}
#endif

			/*
				Initialize the connection block.
			*/
			theConnectionPtr->fIpRemote = 0;
			theConnectionPtr->fRemotePort = 0;
			theConnectionPtr->fParsingControl.fPartialLineLength = 0;
			theConnectionPtr->fParsingControl.fHttpObjectLengthToRead = 0;
			theConnectionPtr->fErrorProcPtr = (rpConnErrorProcPtr) 0;
			theConnectionPtr->fError = eRpNoError;
			theConnectionPtr->fAbortTimer = 0;
			theConnectionPtr->fSkipHttpRequestBlock = False;
#if RomPagerKeepAlive || RomPagerHttpOneDotOne
			theConnectionPtr->fPersistenceTimer = 0;
#endif
#if RomPagerServerPush || RomWebClient || RomDns || RmProtocolTiming || \
		RomPop || RomPopBasic || RomTime
			theConnectionPtr->fProtocolTimer = 0;
#endif
#if RomPagerSecure || RomWebClientSecure
			/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
			/* theConnectionPtr->fIsTlsFlag = False; */
			if(theConnectionPtr->fSslContext) {
				SSLCloseSession(theConnectionPtr->fSslContext);
				RpConnectionCloseTcp(theConnectionPtr);
				theConnectionPtr->fSslContext = NULL;
			}
			theConnectionPtr->fPendingSendBuffer = NULL;
			theConnectionPtr->fPendingSendBufferLen = 0;
			/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#endif

#if RomWebClientSecure
			theConnectionPtr->fConnectAddress = 0;
			theConnectionPtr->fConnectPort = 0;
#endif

#if RomPagerFileSystem
			theConnectionPtr->fFileSystemError = eRpNoError;
			theConnectionPtr->fFileInfo.fFileType = eRpDataTypeNone;
			theConnectionPtr->fFileInfo.fFileDate = 0;
#endif
#if AsVariableAccess || RomPagerServer
			theConnectionPtr->fIndexDepth = -1;
			theConnectionPtr->fIndexValues[0] = 0;
#endif

#if (kStcpActiveConnections > 0)
			if (theConnectionPtr->fIndex >= kStcpActiveBase &&
					theConnectionPtr->fIndex < kStcpNumberOfConnections) {
				theConnectionPtr->fState = eRpConnectionActiveAvailable;
				break;
			}
#endif

#if (kSudpNumberOfConnections > 0)
			if ((theConnectionPtr->fIndex >= kStcpNumberOfConnections) &&
					theConnectionPtr->fIndex < kSerialBase) {
				theConnectionPtr->fState = eRpConnectionUdpAvailable;
				break;
			}
#endif

#if RomConsole
			if (theConnectionPtr->fIndex >= kSerialBase
					&& theConnectionPtr->fIndex < kInitialBase) {
				theConnectionPtr->fState = eRpSerialPortStart;
				break;
			}
#endif
			/*
				Run demons or RomCli initial playback.
			*/
			if (theConnectionPtr->fIndex >= kInitialBase) {
				theDemonPtr = gDemonList[theDataPtr->fDemonIndex];
                if (theDemonPtr != (asDemonFuncPtr) 0) {
					if (theDemonPtr(theDataPtr, theConnectionPtr)) {
						theDataPtr->fDemonIndex += 1;
					}
				}
#if RomCliInitialPlayback
				/*
					Run RomCli initial playback after all demons 
					have completed.
				*/
				else {
					theConnectionPtr->fState = eRpCliInitPlaybackStart;
				}
#endif /* RomCliInitialPlayback */
                break;
            }

#if AsHasTcpInterface
#if (kStcpActiveConnections < kStcpNumberOfConnections)
			/*
				We are closed, so try to open an HTTP listener.
			*/
            #if 0//~seems for sd1 this work is done by L3, so mask
			theResult = StcpOpenPassive(theConnectionPtr->fIndex);

			if (theResult == eRpNoError) {
				theConnectionPtr->fState =
						eRpConnectionWaitingForConnection;
			}
			else {
				/*
					We shouldn't get errors here.  If we do, set a
					breakpoint and figure out why.
				*/
#if AsDebug
				RP_PRINTF("HandleConnectionTask, StcpOpenPassive failed!\n");
#endif
				theResult = eRpNoError;
			}
            #endif//~end jesson
#endif
#endif	/* AsHasTcpInterface */

	//~jesson , to change state for receive data
	theConnectionPtr->fState =
                        eRpConnectionWaitingForConnection;
            break;
			//#endif //~if 0
#if AsHasTcpInterface
#if (kStcpActiveConnections > 0)
		case eRpConnectionActiveAvailable:
			/*
				Routine that checks for something to do with an active
				connection.
			*/
			HandleActiveAvailable(theConnectionPtr);
			break;
#endif
#endif	/* AsHasTcpInterface */

#if RomWebClient && (WcKeepAlive || WcHttpOneDotOne)
		case eRpConnectionWcAvailable:
			/*
				This is an open persistent connection that was used
				by the Web Client.  See if there's another Web Client
				request to this same host.
			*/
			WcHandlePersistentAvailable(theConnectionPtr);
			break;
#endif	/* RomWebClient && (WcKeepAlive || WcHttpOneDotOne) */

#if (kSudpNumberOfConnections > 0)
		case eRpConnectionUdpAvailable:
			/*
				Routine that checks for something to do with an
				available UDP connection.
			*/
			HandleUdpAvailable(theConnectionPtr);
			break;
#endif

#if RomPagerServer || RomPagerBasic || RomTelnet || RomCliSecure
		case eRpConnectionWaitingForConnection:
			/*
				Idle connections loop here to wait for an incoming
				connection.  If there is an incoming connection,
				go see about getting a request buffer.
			*/
			theResult = HandleWaitingForConnection(theConnectionPtr, fd);
			break;
#endif

#if AsHasTcpInterface
		case eRpConnectionWaitingForReceive:
			/*
				Routine that handles the receive of an HTTP request.
			*/

			theResult = HandleWaitingForReceive(theConnectionPtr);
			break;
#endif

#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
		case eRpConnectionNeedsRequestBlock:
			/*
				Routine that allocates an HTTP request control block.
			*/
			theResult = HandleNeedsRequestBlock(theConnectionPtr);
			break;
#endif

		case eRpConnectionProtocolWaitExternal:
		case eRpConnectionNeedsProtocolAction:
			theResult = HandleProtocolAction(theConnectionPtr);
			break;

#if AsHasTcpInterface
		case eRpConnectionSendingReply:
			/*
				Routine that waits for acknowledgement of the response
				to an HTTP request.
			*/
			theResult = HandleSendingReply(theConnectionPtr);
			break;

		case eRpConnectionClosing:
			theResult = HandleClosing(theConnectionPtr);
			break;
#endif

#if ((RomPagerServer || RomPagerBasic) && RomPagerFileSystem)
		case eRpConnectionWaitingFileOpen:
		case eRpConnectionWaitingFileClose:
		case eRpConnectionWaitingFileRead:
		case eRpConnectionWaitingFileCreate:
		case eRpConnectionWaitingFileWrite:
			theResult = RpHandleFileStates(theDataPtr);
			break;
#endif

#if RomPagerServer && RomPagerFileSystem && RomPagerRanges
		case eRpConnectionWaitingFilePosition:
			theResult = RpHandleFilePosition(theConnectionPtr);
			break;
#endif

#if RomPagerRemoteHost
		case eRpConnectionNeedsRemoteRequestBlock:
			/*
				It has been determined that the requested URL
				needs to be retrieved from a Remote Host.

				The Client Connection needs a Remote Host
				Request Block.
			*/
			RpHandleNeedsRemoteHostRequestBlock(theConnectionPtr);
			break;
#endif	/* RomPagerRemoteHost */

#if RomPagerDelayedFunctions || RomPagerRemoteHost || RomPagerServerPush || \
		RomPopBasic || RomPop || RomCli
		case eRpConnectionHolding:
			/*
				This state is used by connections when there is nothing for
				a connection to do. It acts as an idling point for the
				connection so that other connections can run while this one
				is pended for some user or application action. The
				connection's timer still runs so that open connections can
				be closed if the user or application fails to make another
				action.
			*/
//			theConnectionPtr->fState = eRpConnectionActiveAvailable;
			break;
#endif	/* RomPagerDelayedFunctions || RomPagerRemoteHost || RomPagerServerPush || \
			RomPopBasic || RomPop || RomCli */

#if RomPagerExternalPassword
		case eRpConnectionWaitingExternalSecurity:
			RpHandleExternalSecurity(theConnectionPtr);
			break;
#endif

#if RomPagerUserExit
		case eRpConnectionWaitingUserExit:
			RpHandleUserExit(theConnectionPtr);
			break;
#endif

#if RomPagerSecure
		/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
		#if 0
		case eRpConnectionWaitingForTlsHandshake:
			theResult = HandleWaitingForTlsHandshake(theConnectionPtr);
			break;
		#else
		case eRpConnectionWaitingForTlsHandshake:
			theSslResult = SSLServerHandshake(theConnectionPtr->fSslContext);
			theResult = eRpNoError;
			if(theSslResult == SSL_WOULD_BLOCK) {
				theConnectionPtr->fState = eRpConnectionWaitingForTlsHandshake;
				// added by aaron
#if 1 /* zenrong, 2005/4/16 06:44¤U¤È, use min-sec */
                LA3_OS_Sleep(LA3_OS_TICK_2_MSEC(25));
#else				
				LA3_OS_Sleep(25);
#endif				
				// end added
			} else if (theSslResult == SSL_ERROR || 
				theSslResult == SSL_CONNECTION_CLOSED) {
            		theResult =
                    RpConnectionAbortTcp(theConnectionPtr);

			} else {
				theConnectionPtr->fState = eRpConnectionWaitingForReceive;
			}
            break;
            
            case eRpConnectionWaitingForTlsClose:
			if(theConnectionPtr->fSslContext) {
				theSslResult = SSLCloseSession(theConnectionPtr->fSslContext);
				theResult = eRpNoError;
				if(theSslResult == SSL_WOULD_BLOCK) {
					theConnectionPtr->fState = eRpConnectionWaitingForTlsClose;
				} else if (theSslResult == SSL_ERROR || 
					theSslResult == SSL_CONNECTION_CLOSED) {
				theResult = eRpTcpSendError;
				theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
				} else {
					theConnectionPtr->fSslContext = NULL;
					theConnectionPtr->fPendingSendBuffer = NULL;
					theConnectionPtr->fPendingSendBufferLen = 0;
					theResult = RpConnectionCloseTcp(theConnectionPtr);
				}
			}
            break;
        #endif
        /* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#endif

#if RomConsole
		case eRpSerialPortListening:
			theResult = CsFiniteStateMachine(theConnectionPtr);
#if RomCli
			if (theConnectionPtr->fCliActive) {
				RcFiniteStateMachine(theConnectionPtr);
			}
#endif
			break;

		case eRpSerialPortStart:
			theConnectionPtr->fSkipHttpRequestBlock = True;
			theResult = CsConsoleStart(theConnectionPtr);
#if RomCli
			if (theResult == eRpNoError) {
				theResult = RcCliStart(theConnectionPtr, eRcSerialLinkage);
			}
#endif
			break;
#endif	/* RomConsole */


#if RomCliInitialPlayback
		case eRpCliInitPlaybackStart:
			theConnectionPtr->fSkipHttpRequestBlock = True;
			theResult = RcCliStart(theConnectionPtr, eRcInitPlaybackLinkage);
			if (theResult == eRpNoError) {
				theConnectionPtr->fState = eRpCliInitPlaybackRun;
			}
			break;

		case eRpCliInitPlaybackRun:
			if (theConnectionPtr->fCliActive) {
				RcFiniteStateMachine(theConnectionPtr);
			}
			break;
#endif /* RomCliInitialPlayback */


		default:
			break;
	}
	return theResult;
}

static RpErrorCode HandleRHConnectionTask(rpConnectionPtr theConnectionPtr) {
	rpDataPtr			theDataPtr;
	asDemonFuncPtr		theDemonPtr;
	RpErrorCode			theResult;
#if RomPagerServer
	rpConnCloseFuncPtr	theCloseFunctionPtr;
#endif

	/*
		Process a single connection to the next I/O break point.
	*/
	theResult = eRpNoError;
	theDataPtr = theConnectionPtr->fDataPtr;
	theDataPtr->fCurrentConnectionPtr = theConnectionPtr;
#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
	theDataPtr->fCurrentHttpRequestPtr = theConnectionPtr->fHttpRequestPtr;
#endif


	switch (theConnectionPtr->fState) {

		case eRpConnectionClosed:


#if RomPagerServer
			/*
				Notify application that connection was closed.
			*/
			theCloseFunctionPtr = theConnectionPtr->fCloseFuncPtr;
			if (theCloseFunctionPtr != (rpConnCloseFuncPtr) 0) {
				theCloseFunctionPtr(theDataPtr, theConnectionPtr->fCloseCookie);
				theConnectionPtr->fCloseFuncPtr = (rpConnCloseFuncPtr) 0;
			}
#endif

			/*
				Initialize the connection block.
			*/
			theConnectionPtr->fIpRemote = 0;
			theConnectionPtr->fRemotePort = 0;
			theConnectionPtr->fParsingControl.fPartialLineLength = 0;
			theConnectionPtr->fParsingControl.fHttpObjectLengthToRead = 0;
			theConnectionPtr->fErrorProcPtr = (rpConnErrorProcPtr) 0;
			theConnectionPtr->fError = eRpNoError;
			theConnectionPtr->fAbortTimer = 0;
			theConnectionPtr->fSkipHttpRequestBlock = False;
#if RomPagerKeepAlive || RomPagerHttpOneDotOne
			theConnectionPtr->fPersistenceTimer = 0;
#endif
#if RomPagerServerPush || RomWebClient || RomDns || RmProtocolTiming || \
		RomPop || RomPopBasic || RomTime
			theConnectionPtr->fProtocolTimer = 0;
#endif
#if RomPagerSecure || RomWebClientSecure
			theConnectionPtr->fIsTlsFlag = False;
#endif
#if RomWebClientSecure
			theConnectionPtr->fConnectAddress = 0;
			theConnectionPtr->fConnectPort = 0;
#endif
#if RomPagerFileSystem
			theConnectionPtr->fFileSystemError = eRpNoError;
			theConnectionPtr->fFileInfo.fFileType = eRpDataTypeNone;
			theConnectionPtr->fFileInfo.fFileDate = 0;
#endif
#if AsVariableAccess || RomPagerServer
			theConnectionPtr->fIndexDepth = -1;
			theConnectionPtr->fIndexValues[0] = 0;
#endif

#if (kStcpActiveConnections > 0)
			if (theConnectionPtr->fIndex >= kStcpActiveBase &&
					theConnectionPtr->fIndex < kStcpNumberOfConnections) {
				theConnectionPtr->fState = eRpConnectionActiveAvailable;
				break;
			}
#endif

#if (kSudpNumberOfConnections > 0)
			if ((theConnectionPtr->fIndex >= kStcpNumberOfConnections) &&
					theConnectionPtr->fIndex < kSerialBase) {
				theConnectionPtr->fState = eRpConnectionUdpAvailable;
				break;
			}
#endif

#if RomConsole
			if (theConnectionPtr->fIndex >= kSerialBase
					&& theConnectionPtr->fIndex < kInitialBase) {
				theConnectionPtr->fState = eRpSerialPortStart;
				break;
			}
#endif
			/*
				Run demons or RomCli initial playback.
			*/
			if (theConnectionPtr->fIndex >= kInitialBase) {
				theDemonPtr = gDemonList[theDataPtr->fDemonIndex];
                if (theDemonPtr != (asDemonFuncPtr) 0) {
					if (theDemonPtr(theDataPtr, theConnectionPtr)) {
						theDataPtr->fDemonIndex += 1;
					}
				}
#if RomCliInitialPlayback
				/*
					Run RomCli initial playback after all demons 
					have completed.
				*/
				else {
					theConnectionPtr->fState = eRpCliInitPlaybackStart;
				}
#endif /* RomCliInitialPlayback */
                break;
            }

#if AsHasTcpInterface
#if (kStcpActiveConnections < kStcpNumberOfConnections)
			/*
				We are closed, so try to open an HTTP listener.
			*/
            #if 0//~seems for sd1 this work is done by L3, so mask
			theResult = StcpOpenPassive(theConnectionPtr->fIndex);

			if (theResult == eRpNoError) {
				theConnectionPtr->fState =
						eRpConnectionWaitingForConnection;
			}
			else {
				/*
					We shouldn't get errors here.  If we do, set a
					breakpoint and figure out why.
				*/
#if AsDebug
				RP_PRINTF("HandleConnectionTask, StcpOpenPassive failed!\n");
#endif
				theResult = eRpNoError;
			}
            #endif//~end jesson
#endif
#endif	/* AsHasTcpInterface */

	//~jesson , to change state for receive data
	theConnectionPtr->fState =
                        eRpConnectionWaitingForConnection;
            break;
			//#endif //~if 0
#if AsHasTcpInterface
#if (kStcpActiveConnections > 0)
		case eRpConnectionActiveAvailable:
			/*
				Routine that checks for something to do with an active
				connection.
			*/
			HandleActiveAvailable(theConnectionPtr);
			break;
#endif
#endif	/* AsHasTcpInterface */

#if RomWebClient && (WcKeepAlive || WcHttpOneDotOne)
		case eRpConnectionWcAvailable:
			/*
				This is an open persistent connection that was used
				by the Web Client.  See if there's another Web Client
				request to this same host.
			*/
			WcHandlePersistentAvailable(theConnectionPtr);
			break;
#endif	/* RomWebClient && (WcKeepAlive || WcHttpOneDotOne) */

#if (kSudpNumberOfConnections > 0)
		case eRpConnectionUdpAvailable:
			/*
				Routine that checks for something to do with an
				available UDP connection.
			*/
			HandleUdpAvailable(theConnectionPtr);
			break;
#endif

#if RomPagerServer || RomPagerBasic || RomTelnet || RomCliSecure
		case eRpConnectionWaitingForConnection:
			/*
				Idle connections loop here to wait for an incoming
				connection.  If there is an incoming connection,
				go see about getting a request buffer.
			*/
			break;
#endif

#if AsHasTcpInterface
		case eRpConnectionWaitingForReceive:
			/*
				Routine that handles the receive of an HTTP request.
			*/
			theResult = HandleWaitingForReceive(theConnectionPtr);
			break;
#endif

#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
		case eRpConnectionNeedsRequestBlock:
			/*
				Routine that allocates an HTTP request control block.
			*/
			theResult = HandleNeedsRequestBlock(theConnectionPtr);
			break;
#endif

		case eRpConnectionProtocolWaitExternal:
		case eRpConnectionNeedsProtocolAction:
			theResult = HandleProtocolAction(theConnectionPtr);
			break;

#if AsHasTcpInterface
		case eRpConnectionSendingReply:
			/*
				Routine that waits for acknowledgement of the response
				to an HTTP request.
			*/
			theResult = HandleSendingReply(theConnectionPtr);
			break;

		case eRpConnectionClosing:
			theResult = HandleClosing(theConnectionPtr);
			break;
#endif

#if ((RomPagerServer || RomPagerBasic) && RomPagerFileSystem)
		case eRpConnectionWaitingFileOpen:
		case eRpConnectionWaitingFileClose:
		case eRpConnectionWaitingFileRead:
		case eRpConnectionWaitingFileCreate:
		case eRpConnectionWaitingFileWrite:
			theResult = RpHandleFileStates(theDataPtr);
			break;
#endif

#if RomPagerServer && RomPagerFileSystem && RomPagerRanges
		case eRpConnectionWaitingFilePosition:
			theResult = RpHandleFilePosition(theConnectionPtr);
			break;
#endif

#if RomPagerRemoteHost
		case eRpConnectionNeedsRemoteRequestBlock:
			/*
				It has been determined that the requested URL
				needs to be retrieved from a Remote Host.

				The Client Connection needs a Remote Host
				Request Block.
			*/
			RpHandleNeedsRemoteHostRequestBlock(theConnectionPtr);
			break;
#endif	/* RomPagerRemoteHost */

//#if RomPagerDelayedFunctions || RomPagerRemoteHost || RomPagerServerPush || \
//		RomPopBasic || RomPop || RomCli
		case eRpConnectionHolding:
			/*
				This state is used by connections when there is nothing for
				a connection to do. It acts as an idling point for the
				connection so that other connections can run while this one
				is pended for some user or application action. The
				connection's timer still runs so that open connections can
				be closed if the user or application fails to make another
				action.
			*/
//			theConnectionPtr->fState = eRpConnectionActiveAvailable;
			break;
//#endif	/* RomPagerDelayedFunctions || RomPagerRemoteHost || RomPagerServerPush || \
//			RomPopBasic || RomPop || RomCli */

#if RomPagerExternalPassword
		case eRpConnectionWaitingExternalSecurity:
			RpHandleExternalSecurity(theConnectionPtr);
			break;
#endif

#if RomPagerUserExit
		case eRpConnectionWaitingUserExit:
			RpHandleUserExit(theConnectionPtr);
			break;
#endif

#if RomPagerSecure
#if 0
		case eRpConnectionWaitingForTlsHandshake:
			theResult = HandleWaitingForTlsHandshake(theConnectionPtr);
			break;
#endif
#endif

#if RomConsole
		case eRpSerialPortListening:
			theResult = CsFiniteStateMachine(theConnectionPtr);
#if RomCli
			if (theConnectionPtr->fCliActive) {
				RcFiniteStateMachine(theConnectionPtr);
			}
#endif
			break;

		case eRpSerialPortStart:
			theConnectionPtr->fSkipHttpRequestBlock = True;
			theResult = CsConsoleStart(theConnectionPtr);
#if RomCli
			if (theResult == eRpNoError) {
				theResult = RcCliStart(theConnectionPtr, eRcSerialLinkage);
			}
#endif
			break;
#endif	/* RomConsole */


#if RomCliInitialPlayback
		case eRpCliInitPlaybackStart:
			theConnectionPtr->fSkipHttpRequestBlock = True;
			theResult = RcCliStart(theConnectionPtr, eRcInitPlaybackLinkage);
			if (theResult == eRpNoError) {
				theConnectionPtr->fState = eRpCliInitPlaybackRun;
			}
			break;

		case eRpCliInitPlaybackRun:
			if (theConnectionPtr->fCliActive) {
				RcFiniteStateMachine(theConnectionPtr);
			}
			break;
#endif /* RomCliInitialPlayback */


		default:
			break;
	}

	return theResult;
}

static RpErrorCode HandleProtocolAction(rpConnectionPtr theConnectionPtr) {
	RpErrorCode		theResult = eRpNoError;

	/*
		Most of external I/O activity (TCP, File system, etc.)
		is handled by the connection state machine using different
		states in the connection pointer.  The various parsing and
		other protocol activities for HTTP server, HTTP client,
		SMTP client and POP3 client are handled by the individual
		protocol state machines.

		The eRpConnectionNeedsProtocolAction state is used to drive a
		particular connection to the point where it needs more external
		I/O, at which point the connection state will be changed and
		the protocol state machine will exit.

		The eRpConnectionProtocolWaitExternal state is used for cases
		where an I/O activity is best handled by the individual
		protocol state machine.  We use this state to trigger
		a single call to the protocol state machine.

		Any I/O calls in both the connection state machines and the
		protocol state machines should be non-blocking, so that multiple
		connections are handled gracefully.
	*/

	do {
		switch (theConnectionPtr->fProtocol) {
#if RomPagerServer || RomPagerBasic
			case eRpHttpServer:

#if RomPagerIpp
			case eRpIppServer:
#endif

#if RomPagerSecure
			case eRpTlsServer:
#endif
				theResult = RpHandleHttpAction(theConnectionPtr);
				break;
#endif

#if RomCli && RomTelnet
			case eRpTelnetServer:
				TnFiniteStateMachine(theConnectionPtr);
				if (theConnectionPtr->fCliActive) {
					RcFiniteStateMachine(theConnectionPtr);
				}
				break;
#endif

#if RomCli && RomCliSecure
            case eRpSshServer:
				ShFiniteStateMachine(theConnectionPtr);
				if (theConnectionPtr->fCliActive) {
					RcFiniteStateMachine(theConnectionPtr);
				}
                break;
#endif

#if RomConsole
			/*
				This case can be entered if a CLI session is
				running on a serial port and the session is
				suspended and resumed with the RcSuspendSession and
				RcResumeSession calls. After kicking the protocol
				state machines, set the connection state back to
				the serial port.
			*/
			case eRpConsoleServer:
				theResult = CsFiniteStateMachine(theConnectionPtr);
#if RomCli
				if (theConnectionPtr->fCliActive) {
					RcFiniteStateMachine(theConnectionPtr);
				}
#endif
				theConnectionPtr->fState = eRpSerialPortListening;
				break;
#endif

#if RomPagerRemoteHost
			case eRpHttpProxy:
// +++ jx_debug
//TRACE("theConnectionPtr->fProtocol==eRpHttpProxy...call RpHandleRemoteHostAction()\n");
// --- jx_debug
				RpHandleRemoteHostAction(theConnectionPtr);
				break;
#endif
#if RomMailer
			case eRpSmtpClient:
				RmSmtpClientFiniteStateMachine(theConnectionPtr);
				break;
#endif
#if RomMailerBasic
			case eRpSmtpbClient:
				ScSmtpClientFiniteStateMachine(theConnectionPtr);
				break;
#endif
#if RomPop
			case eRpPop3Client:
				PrPop3ClientFiniteStateMachine(theConnectionPtr);
				break;
#endif
#if RomPopBasic
			case eRpPop3bClient:
				PcPop3ClientFiniteStateMachine(theConnectionPtr);
				break;
#endif
#if RomWebClient
			case eRpHttpClient:
				theResult = WcHttpFiniteStateMachine(theConnectionPtr);
				break;
#endif
#if RomDns
			case eRpDnsClient:
				RdFiniteStateMachine(theConnectionPtr);
				break;
#endif
#if RomTime
			case eRpTimeClient:
				TmFiniteStateMachine(theConnectionPtr);
				break;
#endif
#if RomPlug
			case eRpUpnpDeviceListener:
				RuCheckForDiscovery(theConnectionPtr);
#if RomPlugAdvanced
				RuDeliverNextEventMessage(theConnectionPtr);
#endif	/* RomPlugAdvanced */
				break;

			case eRpUpnpDevice:
				break;
#endif	/* RomPlug */
#if RomPlugControl
			case eRpUpnpControlPoint:
			case eRpUpnpControlPointListener:
				CpCheckForSearchResponse(theConnectionPtr);
				break;
#endif	/* RomPlugControl */

			default:
				/*
					This shouldn't happen!
				*/
#if AsDebug
				RP_PRINTF("HandleProtocolAction, default!\n");
#endif
				break;
		}
	} while (theConnectionPtr->fState == eRpConnectionNeedsProtocolAction &&
			theResult == eRpNoError);

	return theResult;
}


static RpErrorCode HandleTimerActions(rpDataPtr theDataPtr, int fd) {
	rpConnectionPtr		theConnectionPtr;
	Unsigned32			theCurrentTime;
	RpErrorCode			theResult;

	theResult = eRpNoError;
	theCurrentTime = RpGetSysTimeInSeconds(theDataPtr);

#if AllegroMultiTasking
	theResult = HandleMultiTaskRescheduling(theDataPtr, fd);
	if (theResult != eRpNoError) {
		return theResult;
	}
#endif
	if (theCurrentTime > theDataPtr->fLastTime) {
		/*
			A second has passed, so do the once per second tasks.
		*/
		theDataPtr->fLastTime = theCurrentTime;
#if RomPagerServer && RomPagerSecurity
		RpCheckPasswordTimers(theDataPtr);
#endif

#if RmMessageTiming
		RmHandleTimers(theDataPtr->fSmtpDataPtr);
#endif

#if RomCli
		RcHandleTimers(theDataPtr);
#endif

#if RomCliSecure
		ShHandleTimers(theDataPtr);
#endif

#if RomPlug
		RuHandleNotifyTimers(theDataPtr);
#endif
#if RomPlugControl
		CpHandleTimers(theDataPtr);
#endif
		/*
			Check for connections that need time action.
		*/
		theConnectionPtr = theDataPtr->fConnections;
		while (theConnectionPtr <
				&theDataPtr->fConnections[kNumberOfConnections]) {
			if (theConnectionPtr->fAbortTimer != 0) {
				/*
					If there was a timer started, decrement it.
				*/
				theConnectionPtr->fAbortTimer -= 1;
				if (theConnectionPtr->fAbortTimer == 0) {
					/*
						If the timer expired, abort the connection.
					*/
					theResult = RpHandleSendReceiveError(theConnectionPtr,
							eRpTcpAbortError);
				}
			}
#if RomPagerServerPush || RomWebClient || RomDns || RmProtocolTiming || \
		RomPop || RomPopBasic || RomTime
			if (theConnectionPtr->fProtocolTimer != 0) {
				/*
					If there was a timer started, decrement it.
				*/
				theConnectionPtr->fProtocolTimer -= 1;
				if (theConnectionPtr->fProtocolTimer == 0) {
					/*
						If the timer has expired,
						restart the protocol.
					*/
					theConnectionPtr->fState =
							eRpConnectionNeedsProtocolAction;
				}
			}
#endif	/* RomPagerServerPush || RomWebClient || RomDns || RmProtocolTiming || \
			RomPop || RomPopBasic || RomTime */
#if RomPagerKeepAlive || RomPagerHttpOneDotOne || RomWebClient
			if (theConnectionPtr->fPersistenceTimer != 0) {
				/*
					If there was a timer started, decrement it.
				*/
				theConnectionPtr->fPersistenceTimer -= 1;
				if (theConnectionPtr->fPersistenceTimer == 0) {
					/*
						If the timer expired, close the connection.
					*/
					theResult = RpConnectionCloseTcp(theConnectionPtr);
				}
			}
#endif	/* RomPagerKeepAlive || RomPagerHttpOneDotOne || RomWebClient */
			theConnectionPtr += 1;
		}
#if RomPagerIpp
		if (theResult == eRpNoError) {
			/*
				Give the IPP parser time to handle it's
				housekeeping tasks.
			*/
			theResult = IppTimer(theDataPtr->fIppDataPtr, theCurrentTime);
		}
#endif
#if RomPagerSecure || RomWebClientSecure
		/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
		#if 0
		SsiTimer(theDataPtr);
		#else
		/* SslReadFromQ(); */
		#endif
		/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#endif
#if RomPagerSecure || RomWebClientSecure || RomCliSecure
#if 0
		AcTimer(theDataPtr);
#endif
#endif
	}

#if WcCaching
	if (theResult == eRpNoError) {
		theResult = WcHandleTimerActions(theDataPtr);
	}
#endif

	return theResult;
}


void RpFreeRequestControlBlock(rpConnectionPtr theConnectionPtr) {
#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
	rpHttpRequestPtr		theRequestPtr;
#endif
#if RomPagerServer
	rpProcessCloseFuncPtr	theCloseFunctionPtr;
#endif
#if RomPagerServer || RomPagerBasic
	rpDataPtr				theDataPtr = theConnectionPtr->fDataPtr;
#endif

#if RomMailer
	/*
		If we were using the request control block for SMTP,
		free up the SMTP request pointer also.
	*/
	if (theConnectionPtr->fProtocol == eRpSmtpClient) {
		RmFreeSmtpRequestBlock(theConnectionPtr);
	}
#endif	/* RomMailer */

#if RomMailerBasic
	/*
		If we were using the request control block for SMTP,
		free up the SMTP request pointer also.
	*/
	if (theConnectionPtr->fProtocol == eRpSmtpbClient) {
		ScFreeSmtpRequestBlock(theConnectionPtr);
	}
#endif	/* RomMailerBasic */

#if RomPop
	if (theConnectionPtr->fProtocol == eRpPop3Client) {
		PrReleaseRequestBlock(theConnectionPtr);
	}
#endif	/* RomPop */

#if RomPopBasic
	if (theConnectionPtr->fProtocol == eRpPop3bClient) {
		PcReleaseRequestBlock(theConnectionPtr);
	}
#endif	/* RomPopBasic */

#if RomWebClient
	if (theConnectionPtr->fProtocol == eRpHttpClient) {
		WcFinishRequest(theConnectionPtr);
	}
#endif	/* RomWebClient */

#if RomCli && RomTelnet
	if (theConnectionPtr->fProtocol == eRpTelnetServer) {
		TnTelnetCleanupConnection(theConnectionPtr);
        RcCliCleanupConnection(theConnectionPtr);
    }
#endif	/* RomCli && RomTelnet */

#if RomCli && RomCliSecure
	if (theConnectionPtr->fProtocol == eRpSshServer) {
        ShCleanupConnection(theConnectionPtr);
		if (theConnectionPtr->fCliSessionPtr != (rcSessionPtr) 0) {
	        RcCliCleanupConnection(theConnectionPtr);
		}
    }
#endif	/* RomCli && RomCliSecure */

#if RomPagerRemoteHost
	/*
		If there's a Remote Host Request Control Block, free it.
	*/
	if (theConnectionPtr->fRHRequestPtr != (rpRHRequestPtr) 0) {
		RpFreeRemoteHostRequestBlock(theConnectionPtr);
	}
#endif	/* RomPagerRemoteHost */

#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
	/*
		If there's an HTTP request block associated with this connection,
		free it up.
	*/
	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	if (theRequestPtr != (rpHttpRequestPtr) 0) {
#if RomPagerServer
#if RomPagerPutMethod
		/*
			If this was a PUT request and it was successful,
			send a notification to the user application
			telling it what file was uploaded.
		*/
		if (theRequestPtr->fObjectSource == eRpPutUrl &&
				theRequestPtr->fHttpPutState == eRpHttpPutCloseFileDone) {
			RpHttpPutComplete(theRequestPtr->fDataPtr, theRequestPtr->fPath);
		}
#endif	/* RomPagerPutMethod */
		/*
			If there's a user function to handle on the close, fire it
			off.  Since these user functions are sometimes used to
			reset the device, this routine may not fully execute.
		*/
		theCloseFunctionPtr = theRequestPtr->fProcessCloseFuncPtr;
		if (theCloseFunctionPtr != (rpProcessCloseFuncPtr) 0) {
			theCloseFunctionPtr(theDataPtr);
		}
#endif	/* RomPagerServer */
#if RomPagerServer || RomPagerBasic
		theDataPtr->fHttpTaskCount--;
#endif
		theConnectionPtr->fHttpRequestPtr = (rpHttpRequestPtr) 0;
		theRequestPtr->fInUse = False;
#if RomPagerDynamicRequestBlocks
		if (theRequestPtr->fDynamicallyAllocated) {
#if RomPagerSoftPages
			RP_FREE(theRequestPtr->fSoftPageRequestPtr);
#endif
			RP_FREE(theRequestPtr);
		}
#endif
	}
#endif	/* RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption */

	return;
}


RpErrorCode	RpHandleSendReceiveError(rpConnectionPtr theConnectionPtr,
							RpErrorCode theError) {
	RpErrorCode			theResult;

	theConnectionPtr->fError = theError;
	theConnectionPtr->fAbortTimer = 0;
#if RomPagerKeepAlive || RomPagerHttpOneDotOne
	theConnectionPtr->fPersistenceTimer = 0;
#endif

	/*
		If there's a specific callback routine to handle connection errors,
		call it, otherwise just close the connection.
	*/
	if (theConnectionPtr->fErrorProcPtr != (rpConnErrorProcPtr) 0) {
		theResult = (*theConnectionPtr->fErrorProcPtr)(theConnectionPtr);
	}
#if AsHasTcpInterface
	else {
		/*
			Just do the default error handling.

			RpConnectionCheckTcpClose will clean up the connection
			and free any request blocks.
		*/
		theResult = RpConnectionCheckTcpClose(theConnectionPtr);
	}
#endif

	return theResult;
}


#if AllegroMultiTasking
static RpErrorCode HandleMultiTaskRescheduling(rpDataPtr theDataPtr, int fd) {
	rpConnectionPtr		theConnectionPtr;
	RpErrorCode			theResult;

	/*
		Check for connections that are in special states that need
		a forced reschedule.
	*/
	theResult = eRpNoError;
	theConnectionPtr = theDataPtr->fConnections;
	while (theResult == eRpNoError && (theConnectionPtr <
			&theDataPtr->fConnections[kNumberOfConnections])) {
		if (theConnectionPtr->fState == eRpConnectionActiveAvailable ||
				theConnectionPtr->fState == eRpConnectionWcAvailable ||
				theConnectionPtr->fState == eRpConnectionUdpAvailable ||
				theConnectionPtr->fState == eRpConnectionNeedsRequestBlock ||
				theConnectionPtr->fState == eRpConnectionNeedsRemoteRequestBlock) {
			theResult = HandleConnectionTask(theConnectionPtr, fd);
		}
		theConnectionPtr += 1;
	}
	return theResult;
}
#endif


#if AsHasTcpInterface

#if (kStcpActiveConnections > 0)

static void HandleActiveAvailable(rpConnectionPtr theConnectionPtr) {
	rpDataPtr			theDataPtr;
#if RomPagerRemoteHost
	rpRHRequestPtr		theRHRequestPtr;
#endif
#if RomMailer
	rmSmtpRequestPtr	theSmtpRequestPtr;
#endif
#if RomMailerBasic
	scSmtpRequestPtr	theScRequestPtr;
#endif
#if RomPop
	rpPop3RequestPtr	thePop3RequestPtr;
#endif
#if RomPopBasic
	rpPcRequestPtr		thePcRequestPtr;
#endif
#if RomWebClient
	wcHttpSessionPtr	theWebSessionPtr;
#endif
	theDataPtr = theConnectionPtr->fDataPtr;
#if RomPagerRemoteHost
	/*
		If there is a Remote Host request pending,
		grab this connection.
	*/
	theRHRequestPtr = RpCaptureRemoteHostRequest(theDataPtr);
	if (theRHRequestPtr) {
#if RpRemoteHostMulti
		theConnectionPtr->fIpRemote =
				theDataPtr->fRemoteHostIpAddress[theRHRequestPtr->fHostIndex];
#else
		theConnectionPtr->fIpRemote = theDataPtr->fRemoteHostIpAddress;
#endif
// +++ jx_debug
//TRACE("HandleActiveAvailable...call RpOpenRemoteHostConnection()\n");
// --- jx_debug
		theRHRequestPtr->fParsingControlPtr =
				&theConnectionPtr->fParsingControl;
		RpOpenRemoteHostConnection(theConnectionPtr, theRHRequestPtr);
		return;
	}
#endif	/* RomPagerRemoteHost */

#if RomMailer
	/*
		If there is an SMTP request pending, grab this connection.
	*/
	theSmtpRequestPtr = RmCaptureSmtpRequest(theDataPtr);
	if (theSmtpRequestPtr) {
		theConnectionPtr->fIpRemote =
				theDataPtr->fSmtpDataPtr->fSmtpServerIpAddress;
		RmStartSmtp(theConnectionPtr, theSmtpRequestPtr);
		return;
	}
#endif	/* RomMailer */

#if RomMailerBasic
	/*
		If there is an SMTP request pending, grab this connection.
	*/
	theScRequestPtr = ScCaptureSmtpRequest(theDataPtr);
	if (theScRequestPtr) {
		theConnectionPtr->fIpRemote =
				theDataPtr->fScDataPtr->fSmtpServerIpAddress;
		ScStartSmtp(theConnectionPtr, theScRequestPtr);
		return;
	}
#endif	/* RomMailerBasic */

#if RomPop
	/*
		If there is a POP3 request pending, grab this connection.
	*/
	thePop3RequestPtr = PrCapturePop3Request(theDataPtr);
	if (thePop3RequestPtr) {
		theConnectionPtr->fIpRemote =
				thePop3RequestPtr->fPop3ServerIpAddress;
		PrStartPop3(theConnectionPtr, thePop3RequestPtr);
		return;
	}
#endif	/* RomPop */

#if RomPopBasic
	/*
		If there is a POP3 request pending, grab this connection.
	*/
	thePcRequestPtr = PcCapturePop3Request(theDataPtr);
	if (thePcRequestPtr) {
		theConnectionPtr->fIpRemote =
				thePcRequestPtr->fPop3ServerIpAddress;
		PcStartPop3(theConnectionPtr, thePcRequestPtr);
		return;
	}
#endif	/* RomPopBasic */

#if RomWebClient
	/*
		If there is a Web Client request pending, grab this connection.
	*/
	theWebSessionPtr = WcCaptureHttpSession(theConnectionPtr);
	if (theWebSessionPtr) {
		theConnectionPtr->fWcSessionPtr = theWebSessionPtr;
		theConnectionPtr->fProtocol = eRpHttpClient;
		theConnectionPtr->fState = eRpConnectionProtocolWaitExternal;
		theWebSessionPtr->fParsingControlPtr =
				&theConnectionPtr->fParsingControl;
#if RomPagerFileSystem
		theWebSessionPtr->fFileInfoPtr = &theConnectionPtr->fFileInfo;
#endif
#if AsMultiTaskingOption
		/*
			Run this connection again to start the session.
		*/
		AsSendTaskMsg(eAsTaskMain, theConnectionPtr->fIndex);
#endif	/* AsMultiTaskingOption */
		return;
	}
#endif	/* RomWebClient */

	return;
}

#endif	/* (kStcpActiveConnections > 0) */


#if RomPagerServer || RomPagerBasic || RomTelnet || RomCliSecure

#if 0//~jesson rp4.0
static RpErrorCode HandleWaitingForConnection(rpConnectionPtr theConnectionPtr) {
#else
static RpErrorCode HandleWaitingForConnection(rpConnectionPtr theConnectionPtr, int fd) {
#endif
	StcpStatus		theConnectedState;
	RpErrorCode		theResult;
/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#if RomPagerSecure
    Signed32		theSslResult;
#endif
/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */

#if AsServerPollReduction
	Unsigned16		theConnection;
	rpDataPtr       theDataPtr;
#endif

#if AsServerPollReduction
	/*
		In some RTOS environments this polling reduction code can
		have a significant performance improvement by reducing the
		number of times a non-blocking socket select call is issued.
	*/
	theConnection = theConnectionPtr->fIndex;
	theDataPtr = theConnectionPtr->fDataPtr;
	if (theDataPtr->fSkipNextConnectionFlag) {
		if (theConnection > theDataPtr->fLastConnectionChecked) {
			return eRpNoError;
		}
	}
	else {
		/*
			Check this connection.
		*/
		theDataPtr->fLastConnectionChecked = theConnection;
		theDataPtr->fSkipNextConnectionFlag = True;
	}
#endif

	#if 0//~jesson for sd1 Rp4.0
    theResult = StcpConnectionStatus(theConnectionPtr->fIndex,
                        &theConnectedState,
                        (StcpIpAddress *) &theConnectionPtr->fIpRemote,
                        (StcpIpAddress *) &theConnectionPtr->fIpLocal,
                        (StcpPort *) &theConnectionPtr->fLocalPort);
	#else
	theResult = StcpConnectionStatus(theConnectionPtr->fIndex,
                        &theConnectedState,
                        (StcpIpAddress *) &theConnectionPtr->fIpRemote,
                        (StcpIpAddress *) &theConnectionPtr->fIpLocal,
                        (StcpPort *) &theConnectionPtr->fLocalPort, fd);
	#endif

	if (theResult == eRpNoError) {
		if (theConnectedState == eStcpComplete) {
			/*
				We have a connection, so set up the protocol
				for the connection.
			*/
			theResult = SetServerProtocol(theConnectionPtr);

			if (theResult == eRpNoError) {
#if AsServerPollReduction
				/*
					Since we're going to use this connection, set
					the flag so we can start looking at the next one.
				*/
				theDataPtr->fSkipNextConnectionFlag = False;
#endif
				/*
					If we have a configured protocol, go read the TCP
					buffer and get the server request.
				*/
#if RomPagerSecure
/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#if 0
				if (theConnectionPtr->fProtocol == eAsTlsServer) {
					theResult = SsiOpenPassive(theConnectionPtr->fDataPtr,
									theConnectionPtr->fIndex,
									(StcpIpAddress) theConnectionPtr->fIpRemote,
									(StcpIpAddress) theConnectionPtr->fIpLocal,
									(StcpPort) theConnectionPtr->fLocalPort);
					if (theResult != eRpNoError) {
						/*
							TLS could not be initialized on this connection 
							which requires TLS so shut the connection down.
						*/
						theConnectionPtr->fState = eRpConnectionClosed;
						theConnectionPtr->fIsTlsFlag = False;
					}
				}
				else {
					theConnectionPtr->fIsTlsFlag = False;
				}
#else
				if (theConnectionPtr->fProtocol == eAsTlsServer) {
                    theConnectionPtr->fIsTlsFlag = True;

                    theConnectionPtr->fSslContext = SSLCreateSession();
//jesson add for session resume 12/18 2002
#if SESSION_RESUME
	SSLSetSessionResumeFunc(theConnectionPtr->fSslContext, HTTPRetrieveSession, HTTPStoreSession,HTTPRemoveSession);
#endif

                    if (theConnectionPtr->fSslContext) {
								
						SSLSetTransportPtr(theConnectionPtr->fSslContext, fd);//Jesson for sd1 fd
						theSslResult = SSLServerHandshake(theConnectionPtr->fSslContext);
						theResult = eRpNoError;
						if(theSslResult == SSL_WOULD_BLOCK) {
							theConnectionPtr->fState = eRpConnectionWaitingForTlsHandshake;
						} else if (theSslResult == SSL_ERROR || 
							theSslResult == SSL_CONNECTION_CLOSED) {
                            theResult = RpConnectionAbortTcp(theConnectionPtr);
						} else {
							theConnectionPtr->fState = eRpConnectionWaitingForReceive;
						}
                    }
                    else {
                    	/*
                        We couldn't get an SSL context,
                        so abort the connection.
                        */
#if AsDebug
                    	RP_PRINTF("HandleWaitingForConnection, handshake "
                    		"failed, theResult = %d\n", theResult);
#endif
                  		theResult =
                    		RpConnectionAbortTcp(theConnectionPtr);
					}
				}
           	 	else {
            		theConnectionPtr->fIsTlsFlag = False;
          		}
#endif //jesson for ssl
/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#endif	/* RomPagerSecure */
#if RomTelnet && RomCli
				/*
					If we have a Telnet connection, we don't need an HTTP request control
					block, but Telnet needs to know right away if it has an available
					control block. If not, the TnTelnetStart call will slam
					the connection shut.

					Right now, we only want to support Telnet if we also have RomCLI.
				*/
				if (theConnectionPtr->fProtocol == eRpTelnetServer) {
					theConnectionPtr->fSkipHttpRequestBlock = True;
					theResult = TnTelnetStart(theConnectionPtr);
					if (theResult == eRpNoError) {
						theResult = RcCliStart(theConnectionPtr, eRcTelnetLinkage);
						if (theResult == eRpNoError) {
							theResult = RpConnectionReceiveTcp(theConnectionPtr);
						}
					}
					else {
						/*
							There weren't enough session blocks, so the connection
							was aborted.  Clear the error so the server keeps running.
						*/
						theResult = eRpNoError;
					}
				}
#endif	/* RomTelnet && RomCli */
#if RomCliSecure && RomCli
                /*
                    There is a new connection for SSH server.
					It does not need an HTTP request control block.
                */
                if (theConnectionPtr->fProtocol == eRpSshServer) {
                    theConnectionPtr->fSkipHttpRequestBlock = True;
					/*
						I'm not sure why Telnet does not need this code too.
					*/
					theConnectionPtr->fCliSessionPtr = (rcSessionPtr) 0;
					theConnectionPtr->fCliActive = False;
                    theResult = ShRomCliSecureStart(theConnectionPtr);
                    if (theResult == eRpNoError) {
						/*
							The SSH protocol starts with the server sending
							a version string.
						*/
						theConnectionPtr->fState = 
								eRpConnectionNeedsProtocolAction;
                    }
                    else {
                        /*
                            There weren't enough session blocks, so the connection
                            was aborted.  Clear the error so the server keeps running.
                        */
                        theResult = eRpNoError;
                    }
                }
#endif	/* RomCliSecure && RomCli */
				/*
					We have a connection for a Web server or IPP request,
					so read the request for processing.
					The request read for SSL or Telnet servers is done
					in the conditional code above.
				*/
				if (theConnectionPtr->fProtocol != eAsTlsServer &&
							theConnectionPtr->fProtocol != eRpTelnetServer) {
					theResult = RpConnectionReceiveTcp(theConnectionPtr);
				}
			}
			else {
				/*
					The connection came in on a port we aren't set up
					to handle, so shut it down.
				*/
				theResult = StcpAbortConnection(theConnectionPtr->fIndex);
				theConnectionPtr->fState = eRpConnectionClosed;
#if AsMultiTaskingOption
				/*
					Run this connection again.
				*/
				AsSendTaskMsg(eAsTaskMain, theConnectionPtr->fIndex);
#endif	/* AsMultiTaskingOption */
			}
		}
		else if (theConnectedState == eStcpOpenPassiveTimeout) {
			/*
				If the state is a passive timeout, mark the connection
				closed, and we'll reopen it the next time around.
			*/
			theConnectionPtr->fState = eRpConnectionClosed;
		}
	}
	else {
		/*
			We have an error with the connection, so mark it closed.
		*/
		theConnectionPtr->fState = eRpConnectionClosed;
	}

	return theResult;
}


/*
	A TCP connection has been established and now the TLS handshake 
	is in progress.
*/

#if RomPagerSecure
#if 0
static RpErrorCode HandleWaitingForTlsHandshake(rpConnectionPtr theConnectionPtr) {
	StcpStatus		theConnectedState;
	RpErrorCode		theResult;

	theResult = SsiConnectionStatus(theConnectionPtr->fDataPtr,
									theConnectionPtr->fIndex,
									& theConnectedState);
	if (theResult == eRpNoError) {
		if (theConnectedState == eStcpComplete) {
			/*
				The  TLS handshake has completed.
				Read the request for processing.
			*/ 
			theResult = RpConnectionReceiveTcp(theConnectionPtr);
		}
	}
	else {
		/*
			We have an error with the connection, so mark it closed.
		*/
		theConnectionPtr->fState = eRpConnectionClosed;
		theResult = eRpNoError;
	}

	return theResult;
}
#endif
#endif	/* RomPagerSecure */


static RpErrorCode SetServerProtocol(rpConnectionPtr theConnectionPtr) {
	rpDataPtr	theDataPtr;
	StcpPort	thePort;
	Unsigned8	theIndex;
	RpErrorCode	theResult;

	/*
		Set the default error to a connection error.
	*/
	theResult = eRpTcpConnectError;
	theDataPtr = theConnectionPtr->fDataPtr;

	/*
		Search the server port table for a match.
	*/
	theIndex = 0;
	thePort = theConnectionPtr->fLocalPort;
	while (theIndex < theDataPtr->fServerPortIndex) {
      //  if (thePort == theDataPtr->fServerPorts[theIndex].fPort) {//ArthurChow  masked (04443)
//ArthurChow Begin
//Force "theDataPtr->fServerPorts[theIndex].fPort = thePort"
//to Fixed the "Set HTTP Port" Problem;
		/* Start add by Scott Tsai for SSL */
		if(thePort != 443)
			theDataPtr->fServerPorts[theIndex].fPort=thePort;
        /* End add by Scott Tsai for SSL */
//ArthurChow End
		if (thePort == theDataPtr->fServerPorts[theIndex].fPort) {
			/*
				We have a match, so set the connection protocol
				and return with no error.
			*/
			theConnectionPtr->fProtocol =
					theDataPtr->fServerPorts[theIndex].fProtocol;
					theResult = eRpNoError;
			break;
		}
		theIndex++;
	}
	return theResult;
}

#endif	/* RomPagerServer || RomPagerBasic || RomTelnet || RomCliSecure */


static RpErrorCode HandleWaitingForReceive(rpConnectionPtr theConnectionPtr) {
	StcpStatus			theReceiveCompleteState;
	char *				theReceiveBufferPtr;
	Unsigned16			theReceiveLength;
	RpErrorCode			theResult;

#if RomPagerSecure || RomWebClientSecure
/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
	Unsigned32  theSslReceiveLength;
    Signed32      theSslResult;
#if 0
	if (theConnectionPtr->fIsTlsFlag) {
		theResult = SsiReceiveStatus(theConnectionPtr->fDataPtr,
							theConnectionPtr->fIndex,
							(SsiStatus*) &theReceiveCompleteState,
							&theReceiveBufferPtr,
							&theReceiveLength);
	}
#else
	if (theConnectionPtr->fIsTlsFlag) {
        theResult = eRpNoError;
        theReceiveBufferPtr = theConnectionPtr->fReadBufferPtr;
        theSslReceiveLength = kRtReadbufferLength;
#if 1 /* zenrong, use m-sec */
        LA3_OS_Sleep(LA3_OS_TICK_2_MSEC(3));
#else        
        LA3_OS_Sleep(3);
#endif        
        theSslResult = SSLRecv(theConnectionPtr->fSslContext, 
			theReceiveBufferPtr, theSslReceiveLength);
		theSslReceiveLength = theSslResult;

		if(theSslResult == SSL_WOULD_BLOCK) {
			theReceiveCompleteState = eStcpPending;
		} else if(theSslResult == SSL_CONNECTION_CLOSED || theSslResult == SSL_ERROR) {
			theResult = eRpTcpReceiveError;
#if AsDebug
                RP_PRINTF("HandleWaitingForReceive, SSL/TLS read error, "
                        "theSslResult = %d\n", theSslResult);
#endif
	} else {
             theReceiveLength = (Unsigned16) theSslReceiveLength;
             theReceiveCompleteState = eStcpComplete;
		}
    }
#endif
/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
	else 
#endif	/* RomPagerSecure || RomWebClientSecure */
	{
		theResult = StcpReceiveStatus(theConnectionPtr->fIndex,
							&theReceiveCompleteState,
							&theReceiveBufferPtr,
							&theReceiveLength);
	}
	if (theResult == eRpNoError) {
		if (theReceiveCompleteState == eStcpComplete) {
			/*
				Save the receive buffer info.
			*/
			theConnectionPtr->fParsingControl.fCurrentBufferPtr =
					theReceiveBufferPtr;
			theConnectionPtr->fParsingControl.fIncomingBufferLength =
					theReceiveLength;
			theResult = HandleReceive(theConnectionPtr);
		}
#if RomCli && (RomTelnet || RomCliSecure)
		/*
			No characters have been received, but we want Telnet 
			or RomCliSecure to receive control to keep the output going and
			we want the CLI protocol handler to get control to
			keep the CLI input and output processing going, also.
		*/
		else {
#if RomTelnet
			if (theConnectionPtr->fProtocol == eRpTelnetServer) {
				theConnectionPtr->fParsingControl.fIncomingBufferLength = 0;
				TnFiniteStateMachine(theConnectionPtr);
				if (theConnectionPtr->fCliActive) {
					RcFiniteStateMachine(theConnectionPtr);
				}
			}
#endif
#if RomCliSecure
            if (theConnectionPtr->fProtocol == eRpSshServer) {
				theConnectionPtr->fParsingControl.fIncomingBufferLength = 0;
                ShFiniteStateMachine(theConnectionPtr);
                if (theConnectionPtr->fCliActive) {
                    RcFiniteStateMachine(theConnectionPtr);
                }
            }
#endif
		}
#endif	/* RomCli && (RomTelnet || RomCliSecure) */
	}
	else {
		theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
	}

	return theResult;
}


static RpErrorCode HandleReceive(rpConnectionPtr theConnectionPtr) {
	RpErrorCode	theResult;

	theResult = eRpNoError;
	/*
		Reset the receive timers.
	*/
	theConnectionPtr->fAbortTimer = 0;
#if RomPagerKeepAlive || RomPagerHttpOneDotOne
	theConnectionPtr->fPersistenceTimer = 0;
#endif

#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
	if (theConnectionPtr->fSkipHttpRequestBlock) {
		/*
			RomPOP, Remote Host and sometimes RomMailer
			don't need an HTTP request control block.
		*/
		theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
		theResult = HandleProtocolAction(theConnectionPtr);
	}
	else {
		/*
			Set up the request control block.
		*/
		if (theConnectionPtr->fHttpRequestPtr ==
					(rpHttpRequestPtr) 0) {
			/*
				We don't have a request block yet,
				so this must be the initial request.
				Store the buffer info and go get a request block.
			*/
			theConnectionPtr->fState = eRpConnectionNeedsRequestBlock;
			theResult = HandleNeedsRequestBlock(theConnectionPtr);
		}
		else {
			/*
				We have a request block, so this is an additional
				receive buffer.  Go parse it.
			*/
			theResult = HandleReceiveBuffer(theConnectionPtr);
		}
	}
#else
	theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
	theResult = HandleProtocolAction(theConnectionPtr);
#endif

	return theResult;
}


static RpErrorCode HandleSendingReply(rpConnectionPtr theConnectionPtr) {
	RpErrorCode			theResult;
	StcpStatus			theSendCompleteState;

#if RomPagerSecure || RomWebClientSecure
/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
	Signed32     theSslResult;

    theResult = eRpNoError;
	/*
		Check the send status.
	*/
	if (theConnectionPtr->fIsTlsFlag) {
#if 0	/* second #if start */
		theResult = SsiSendStatus(theConnectionPtr->fDataPtr,
						theConnectionPtr->fIndex, 
						&theSendCompleteState);
#else	/* second #if */
		if(theConnectionPtr->fPendingSendBufferLen == 0) {
			theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
			theResult = HandleProtocolAction(theConnectionPtr);
		} else {
        	theSslResult = SSLSend(theConnectionPtr->fSslContext, theConnectionPtr->fPendingSendBuffer, 
			theConnectionPtr->fPendingSendBufferLen);

            if (theSslResult == SSL_WOULD_BLOCK) {
			}else if(theSslResult == SSL_ERROR) {
                theResult = eRpTcpSendError;
#endif	/* second #if end */

#if AsDebug
                RP_PRINTF("HandleSendingReply, SSL/TLS write error, "
                        "theSslResult = %d\n", theSslResult);
#endif
                theResult = RpHandleSendReceiveError(theConnectionPtr,
                        eRpTcpSendError);
			} else {
				if(theSslResult == theConnectionPtr->fPendingSendBufferLen) {
					theResult = eRpNoError;
					theConnectionPtr->fPendingSendBuffer = NULL;
					theConnectionPtr->fPendingSendBufferLen = 0;
					theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
					theResult = HandleProtocolAction(theConnectionPtr);
#if AsDebug
                    RP_PRINTF("RpSendReplyBuffer, complete send: "
                            "theSendLength = %d\n", theConnectionPtr->fPendingSendBufferLen);
#endif
				} else {
					theResult = eRpNoError;
					theConnectionPtr->fPendingSendBuffer = theConnectionPtr->fPendingSendBuffer + theSslResult;
					theConnectionPtr->fPendingSendBufferLen = theConnectionPtr->fPendingSendBufferLen - theSslResult; 
#if AsDebug
                    RP_PRINTF("RpSendReplyBuffer, partial send: "
                            "theSendLength = %d, theSslSendLength = %d\n",
                            theConnectionPtr->fPendingSendBufferLen , theSslResult);
#endif
				}
			}
		}
	}
	else {
		theResult = StcpSendStatus(theConnectionPtr->fIndex,
                            &theSendCompleteState);

        if (theResult == eRpNoError) {
            if (theSendCompleteState == eStcpComplete) {
                theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
                theResult = HandleProtocolAction(theConnectionPtr);
            }
            /*
                else wait for send to complete
            */
        }
        else {
            theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
        }
    }
#else   /* RomPagerSecure || RomWebClientSecure */
    theResult = StcpSendStatus(theConnectionPtr->fIndex, &theSendCompleteState);

    if (theResult == eRpNoError) {
        if (theSendCompleteState == eStcpComplete) {
            theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
            theResult = HandleProtocolAction(theConnectionPtr);
        }
        /*
            else wait for send to complete
        */
    }
    else {
        theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
    }
/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#endif  /* RomPagerSecure || RomWebClientSecure */
	return theResult;
}


static RpErrorCode HandleClosing(rpConnectionPtr theConnectionPtr) {
	StcpStatus      theCloseState;
    RpErrorCode theResult;

    /*
        See if the connection has settled out and closed.
    */
    theResult = StcpCloseStatus(theConnectionPtr->fIndex, &theCloseState);

    if (theResult == eRpNoError) {
        if (theCloseState == eStcpComplete) {
            theConnectionPtr->fAbortTimer = 0;
            theConnectionPtr->fState = eRpConnectionClosed;
            
            RpFreeRequestControlBlock(theConnectionPtr);
        }
        else if (theCloseState == eStcpTimeWait) {
            /*
                The basic part of the close is complete, and the
                connection has gone into the TCP TIME_WAIT state.
                We don't want to stay in this state very long, and
                it is now safe to close it soon.
            */
            if (theConnectionPtr->fAbortTimer > kConnectionCloseTimeout) {
                theConnectionPtr->fAbortTimer = kConnectionCloseTimeout;
            }
        }
    }
    else {
        /*
            We got an error on the connection status check, so just slam
            the connection closed.
        */
        theResult = RpConnectionAbortTcp(theConnectionPtr);
    }
    return theResult;
}


#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption

static RpErrorCode HandleNeedsRequestBlock(rpConnectionPtr theConnectionPtr) {
	rpDataPtr			theDataPtr;
	rpHttpRequestPtr	theRequestPtr;
	RpErrorCode			theResult;
	Boolean				theGotRequestBlockFlag;

	theResult = eRpNoError;
	theDataPtr = theConnectionPtr->fDataPtr;
	theGotRequestBlockFlag = False;

#if (kRpNumberOfHttpRequests > 0)
	/*
		Search the pool of pre-allocated request blocks.
	*/
	theRequestPtr = theDataPtr->fHttpRequests;
	while (theRequestPtr <
			&theDataPtr->fHttpRequests[kRpNumberOfHttpRequests] &&
			theRequestPtr->fInUse) {
		theRequestPtr += 1;
	}
	if (theRequestPtr < &theDataPtr->fHttpRequests[kRpNumberOfHttpRequests]) {
		theGotRequestBlockFlag = True;
	}
#endif	/* (kRpNumberOfHttpRequests > 0) */

#if RomPagerDynamicRequestBlocks
	/*
		If we don't have a request block yet,
		see if we can allocate memory for one.
	*/
	if (!theGotRequestBlockFlag) {
		theRequestPtr = RP_ALLOC(sizeof(rpHttpRequest));
		if (theRequestPtr != (rpHttpRequestPtr) 0) {
			RP_MEMSET(theRequestPtr, 0, sizeof(rpHttpRequest));
			theRequestPtr->fDynamicallyAllocated = True;
#if RomPagerSoftPages
			theRequestPtr->fSoftPageRequestPtr = RP_ALLOC(sizeof(spRequest));
			if (theRequestPtr->fSoftPageRequestPtr != (spRequestPtr) 0) {
				theGotRequestBlockFlag = True;
			}
			else {
				RP_FREE(theRequestPtr);
			}
#else
			theGotRequestBlockFlag = True;
#endif
		}
	}
#endif	/* RomPagerDynamicRequestBlocks */

	if (theGotRequestBlockFlag) {
		/*
			We have a free request control block, so grab it and set up
			to process the request.
		*/
#if AsDebug
		if (theConnectionPtr->fIndex > gRpHighestUsedConnection) {
			gRpHighestUsedConnection = theConnectionPtr->fIndex;
		}
#endif
#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
		theConnectionPtr->fHttpRequestPtr = theRequestPtr;
#endif
#if RomPagerServer || RomPagerBasic
		theDataPtr->fCurrentHttpRequestPtr = theRequestPtr;
		theDataPtr->fHttpTaskCount++;
#endif
		theConnectionPtr->fParsingControl.fTempBufferPtr =
				theRequestPtr->fHttpTempLineBuffer;
		theConnectionPtr->fParsingControl.fPartialLineLength = 0;
		theConnectionPtr->fParsingControl.fHttpObjectLengthToRead = 0;
		RpInitRequestStates(theDataPtr, theRequestPtr);
#if RomPagerServer || RomPagerBasic
		theRequestPtr->fParsingControlPtr = &theConnectionPtr->fParsingControl;
#endif
#if ((RomPagerServer || RomPagerBasic) && RomPagerFileSystem)
		theRequestPtr->fFileInfoPtr = &theConnectionPtr->fFileInfo;
#endif
		theResult = HandleReceiveBuffer(theConnectionPtr);
	}
	return theResult;
}


static RpErrorCode HandleReceiveBuffer(rpConnectionPtr theConnectionPtr) {
#if RomPagerTraceMethod
	Unsigned32			theAvailableLength;
	Unsigned32			theReceiveLength;
	rpHttpRequestPtr	theRequestPtr;

	/*
		For HTTP 1.1 servers, capture the incoming request buffer if
		the TRACE method was requested.
	*/
	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	theReceiveLength =
			theConnectionPtr->fParsingControl.fIncomingBufferLength;
	if ((theConnectionPtr->fProtocol == eRpHttpServer ||
			theConnectionPtr->fProtocol == eRpIppServer) &&
			theRequestPtr->fTracing &&
			theRequestPtr->fTraceLength < kHtmlMemorySize) {
		theAvailableLength = kHtmlMemorySize - theRequestPtr->fTraceLength;
		if (theReceiveLength > theAvailableLength) {
			theReceiveLength = theAvailableLength;
		}
		RP_MEMCPY(theRequestPtr->fTraceBufferPtr,
				theConnectionPtr->fParsingControl.fCurrentBufferPtr,
				theReceiveLength);
		theRequestPtr->fTraceLength += theReceiveLength;
		theRequestPtr->fTraceBufferPtr += theReceiveLength;
	}
#endif

	theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
	return HandleProtocolAction(theConnectionPtr);
}

#endif	/* RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption */

/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#if RomPagerSecure
RpErrorCode RpConnectionReceiveTcp(rpConnectionPtr theConnectionPtr) {
    char *          theReceiveBufferPtr;
    Unsigned32      theReceiveLength;
    Boolean         theReceiveStarted;
    RpErrorCode     theResult;
    //SSLErr          theSslResult;
    Signed32        theSslResult;

#if 0//ssl
    if (theConnectionPtr->fIsTlsFlag) {
        theReceiveStarted = False;
        theConnectionPtr->fState = eRpConnectionWaitingForReceive;
        theSslResult = SSLGetReadPendingSize(theConnectionPtr->fSslContext,
                &theReceiveLength);
        if (theSslResult != SSLNoErr) {
            theResult = eRpTlsReadError;
        }
        else if (theReceiveLength == 0) {
            theReceiveBufferPtr = theConnectionPtr->fReadBufferPtr;
            theReceiveLength = kRtReadbufferLength;
            theSslResult = SSLRead(theReceiveBufferPtr, &theReceiveLength,
                    theConnectionPtr->fSslContext);
            if (theSslResult == SSLWouldBlockErr) {
                theReceiveStarted = True;
                theResult = eRpNoError;
            }
            else if (theSslResult == SSLNoErr) {
#else//ssl
if (theConnectionPtr->fIsTlsFlag) {
        theReceiveStarted = False;
        theConnectionPtr->fState = eRpConnectionWaitingForReceive;
            theReceiveBufferPtr = theConnectionPtr->fReadBufferPtr;
			theSslResult = SSLRecv(theConnectionPtr->fSslContext, 
				theReceiveBufferPtr, kRtReadbufferLength);
			theReceiveLength = theSslResult;

		if(theSslResult == SSL_WOULD_BLOCK) {
			theReceiveStarted = True;
            theResult = eRpNoError;
		} else if(theSslResult == SSL_CONNECTION_CLOSED || theSslResult == SSL_ERROR) {
#if AsDebug
                RP_PRINTF("RpConnectionReceiveTcp, SSL/TLS receive error, "
                        "theSslResult = %d\n", theSslResult);
#endif
                theResult = eRpNoError;
		} else {
#endif //ssl

                /*
                    Save the receive buffer info.
                */
                theConnectionPtr->fParsingControl.fCurrentBufferPtr =
                        theReceiveBufferPtr;
                theConnectionPtr->fParsingControl.fIncomingBufferLength =
                        theReceiveLength;
                theResult = HandleReceive(theConnectionPtr);
#if 0//ssl
            }
            else {
#if AsDebug
                RP_PRINTF("RpConnectionReceiveTcp, SSL/TLS receive error, "
                        "theSslResult = %d\n", theSslResult);
#endif
                theResult = eRpTcpReceiveError;
            }
        
#endif//ssl
        }
    }
    else {

        /*
            Routine that handles the receive of a TCP packet.
        */
        theResult = StcpReceive(theConnectionPtr->fIndex);
        theReceiveStarted = True;
    }

    if (theReceiveStarted) {
        if (theResult == eRpNoError) {
            theConnectionPtr->fState = eRpConnectionWaitingForReceive;
            /*
                We have started a TCP receive, so wait for it
                to complete and set an abort timer in case it doesn't.

                In some cases, when a user hits the reload button, the
                browser will abort an outstanding connection.  Depending
                on timing and TCP implementation, we may not be able to
                detect at the Stcp layer that the connection has
                disappeared.  So if after starting the read, we don't
                get something in a bit, we terminate the connection.

                Use a short abort timer for the HTTP server environment,
                and a long abort timer for other protocols.
            */
            theConnectionPtr->fAbortTimer = kConnectionMaxIdleTimeout;
#if RomPagerServer || RomPagerBasic
            if (theConnectionPtr->fProtocol == eRpHttpServer ||
                    theConnectionPtr->fProtocol == eRpIppServer) {
                theConnectionPtr->fAbortTimer = kConnectionReceiveTimeout;
            }
#endif  /* RomPagerServer || RomPagerBasic */
#if RomTelnet
            if (theConnectionPtr->fProtocol == eRpTelnetServer) {
                theConnectionPtr->fAbortTimer = kTelnetReceiveTimeout;
            }
#endif  /* RomTelnet */
#if RmProtocolTiming
            if (theConnectionPtr->fProtocol == eRpSmtpClient) {
                theConnectionPtr->fAbortTimer = kSmtpConnectionMaxIdleTimeout;
            }
#endif
        }
        else {
            theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
        }
    }
    return theResult;
}

        /* RomPagerSecure */
#else
        /* !RomPagerSecure */

RpErrorCode RpConnectionReceiveTcp(rpConnectionPtr theConnectionPtr) {
    RpErrorCode     theResult;

    /*
        Routine that handles the receive of a TCP packet.
    */
    theResult = StcpReceive(theConnectionPtr->fIndex);
    if (theResult == eRpNoError) {
        theConnectionPtr->fState = eRpConnectionWaitingForReceive;
        /*
            We have started a TCP receive, so wait for it
            to complete and set an abort timer in case it doesn't.

            In some cases, when a user hits the reload button, the
            browser will abort an outstanding connection.  Depending
            on timing and TCP implementation, we may not be able to
            detect at the Stcp layer that the connection has
            disappeared.  So if after starting the read, we don't
            get something in a bit, we terminate the connection.

            Use a short abort timer for the HTTP server environment,
            and a long abort timer for other protocols.
        */
        theConnectionPtr->fAbortTimer = kConnectionMaxIdleTimeout;
#if RomPagerServer || RomPagerBasic
        if (theConnectionPtr->fProtocol == eRpHttpServer ||
                theConnectionPtr->fProtocol == eRpIppServer) {
            theConnectionPtr->fAbortTimer = kConnectionReceiveTimeout;
        }
#endif  /* RomPagerServer || RomPagerBasic */
#if RomTelnet
        if (theConnectionPtr->fProtocol == eRpTelnetServer) {
            theConnectionPtr->fAbortTimer = kTelnetReceiveTimeout;
        }
#endif  /* RomTelnet */
#if RmProtocolTiming
        if (theConnectionPtr->fProtocol == eRpSmtpClient) {
            theConnectionPtr->fAbortTimer = kSmtpConnectionMaxIdleTimeout;
        }
#endif
    }
    else {
        theResult = RpHandleSendReceiveError(theConnectionPtr, theResult);
    }
    return theResult;
}
#endif  /* !RomPagerSecure */
/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */


RpErrorCode RpConnectionCheckTcpClose(rpConnectionPtr theConnectionPtr) {
	Boolean				theNeedCloseFlag = True;
	RpErrorCode			theResult = eRpNoError;
#if RomPagerKeepAlive || RomPagerHttpOneDotOne
	rpHttpRequestPtr	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
#endif

	/*
		We have sent a complete response, so see if we should
		close the connection and free up the request block.

		If there's been an error on the connection, we will close
		or abort it.

		Otherwise, If we have received a Keep-Alive request from the
		browser that we can honor, we keep the connection open and read
		another request.  Otherwise, we close the connection.
	*/
	switch (theConnectionPtr->fError) {

		case eRpNoError:
			/*
				There hasn't been a connection error.
				Just do a normal close.
			*/
#if RomPagerKeepAlive || RomPagerHttpOneDotOne
			if (theRequestPtr != (rpHttpRequestPtr) 0 &&
					theRequestPtr->fPersistent) {
				theNeedCloseFlag = False;
				/*
					This is a persistent connection, so clear some error
					states and get ready for the next request.
				*/
				theConnectionPtr->fErrorProcPtr = (rpConnErrorProcPtr) 0;
#if RomPagerFileSystem
				theConnectionPtr->fFileSystemError = eRpNoError;
				theConnectionPtr->fFileInfo.fFileType = eRpDataTypeNone;
#endif
#if AsVariableAccess || RomPagerServer
				theConnectionPtr->fIndexDepth = -1;
				theConnectionPtr->fIndexValues[0] = 0;
#endif
				if (theConnectionPtr->fParsingControl.fIncomingBufferLength > 0) {
					/*
						We must have had a pipelined request,
						so go process it.
					*/
					RpInitRequestStates(theConnectionPtr->fDataPtr,
										theRequestPtr);
					theRequestPtr->fParsingControlPtr =
							&theConnectionPtr->fParsingControl;
#if RomPagerFileSystem
					theRequestPtr->fFileInfoPtr =
							&theConnectionPtr->fFileInfo;
#endif
#if 1
					theRequestPtr->fParsingControlPtr->fHttpObjectLengthToRead = 0;
#endif
					theConnectionPtr->fState =
						eRpConnectionNeedsProtocolAction;
				}
				else {
					/*
						Free the request control block,
						and go wait for another request.
					*/
					RpFreeRequestControlBlock(theConnectionPtr);
					theResult = RpConnectionReceiveTcp(theConnectionPtr);
					/*
						We are in a persistent connection environment, so
						set a timer to close the connection gracefully, if
						no further request is made and clear the abort timer.
					*/
					theConnectionPtr->fPersistenceTimer =
							kConnectionMaxIdleTimeout;
					theConnectionPtr->fAbortTimer = 0;
				}
			}
#endif	/* RomPagerKeepAlive || RomPagerHttpOneDotOne */

			if (theNeedCloseFlag) {
#if RomPagerSecure || RomWebClientSecure
                if (theConnectionPtr->fIsTlsFlag) {
                	
                	//jesson for ssl
                    //theSslResult = SSLClose(theConnectionPtr->fSslContext);
                    theConnectionPtr->fState = eRpConnectionWaitingForTlsClose;
                }
                else {
                    theResult = RpConnectionCloseTcp(theConnectionPtr);
                }
#else
                theResult = RpConnectionCloseTcp(theConnectionPtr);
#endif  /* RomPagerSecure || RomWebClientSecure */
            }
            break;

        case eRpTcpNoConnection:
            /*
                There has been a "No Connection" error.  We may have gotten
                a close from the other side.  This is especially possible
                in a persistent connection environment.  Indicate that the
                connection is closed (returning it to the connection pool),
                and free any request blocks.
            */
            theConnectionPtr->fState = eRpConnectionClosed;
            theConnectionPtr->fAbortTimer = 0;
#if RomPagerKeepAlive || RomPagerHttpOneDotOne
            theConnectionPtr->fPersistenceTimer = 0;
#endif
            RpFreeRequestControlBlock(theConnectionPtr);
            break;

#if RomDns || RomTime
        case eRpUdpReceiveError:
        case eRpUdpSendError:
            /*
                There's been a UDP error so clean up the UDP environment
            */
            theResult = SudpCloseConnection(theConnectionPtr->fIndex);
            break;
#endif

        default:
            /*
                There's been a connection error (other than "No Connection").
                Abort the connection and free any request blocks.
            */
            theResult = RpConnectionAbortTcp(theConnectionPtr);
            break;
    }

    return theResult;
}


/*
	Close a TCP connection in an orderly fashion.
*/

RpErrorCode RpConnectionCloseTcp(rpConnectionPtr theConnectionPtr) {
	RpErrorCode			theResult;
#if RomPagerServer
	rpHttpRequestPtr	theRequestPtr;
#endif

#if RomPagerSecure || RomWebClientSecure
	Boolean             theReallyCloseFlag;
	
#if 0
	if (theConnectionPtr->fIsTlsFlag) { 
		theResult = SsiCloseConnection(theConnectionPtr->fDataPtr,
						theConnectionPtr->fIndex);
	}
	else
#endif
	theReallyCloseFlag = True;
	if (theReallyCloseFlag)
#endif	/*	RomPagerSecure || RomWebClientSecure	*/
	{
		theResult = StcpCloseConnection(theConnectionPtr->fIndex);
	}

	if (theResult == eRpTcpAlreadyClosed) {
		theConnectionPtr->fState = eRpConnectionClosed;
		RpFreeRequestControlBlock(theConnectionPtr);
		theResult = eRpNoError;
	}
	else {
		theConnectionPtr->fState = eRpConnectionClosing;
		/*
			We don't want to stay in this state forever, so
			set up the timeout.
		*/
		theConnectionPtr->fAbortTimer = kConnectionMaxIdleTimeout;

#if RomPagerServer
		theRequestPtr = theConnectionPtr->fHttpRequestPtr;
		if (theRequestPtr != (rpHttpRequestPtr) 0) {
			/*
				If there is a request block, maybe we can free it.
			*/
			if (theRequestPtr->fProcessCloseFuncPtr ==
					(rpProcessCloseFuncPtr) 0) {
				/*
					If there is no function to call when the connection
					closes, we can free the request block for this
					connection.
				*/
				RpFreeRequestControlBlock(theConnectionPtr);
			}
		}
#endif
	}
	return theResult;
}


RpErrorCode RpConnectionAbortTcp(rpConnectionPtr theConnectionPtr) {
	RpErrorCode		theResult;

	/*
		Shut the connection down.
	*/
	theResult = StcpAbortConnection(theConnectionPtr->fIndex);
	theConnectionPtr->fState = eRpConnectionClosed;
	theConnectionPtr->fAbortTimer = 0;
#if (RomPagerKeepAlive || RomPagerHttpOneDotOne)
	theConnectionPtr->fPersistenceTimer = 0;
#endif
	RpFreeRequestControlBlock(theConnectionPtr);
#if AsMultiTaskingOption
	/*
		Run this connection again.
	*/
	AsSendTaskMsg(eAsTaskMain, theConnectionPtr->fIndex);
#endif	/* AsMultiTaskingOption */
	return theResult;
}

#if 0
static RpErrorCode AbortConnection(rpConnectionPtr theConnectionPtr) {
	RpErrorCode		theResult;

#if RomPagerSecure || RomWebClientSecure
	if (theConnectionPtr->fIsTlsFlag) { 
		theResult = SsiAbortConnection(theConnectionPtr->fDataPtr,
						theConnectionPtr->fIndex);
	}
	else 
#endif	/* RomPagerSecure || RomWebClientSecure	*/
	{
		theResult = StcpAbortConnection(theConnectionPtr->fIndex);
	}
	return theResult;
}
#endif
#endif	/* AsHasTcpInterface */


#if (kSudpNumberOfConnections > 0)

static void HandleUdpAvailable(rpConnectionPtr theConnectionPtr) {
	rpDataPtr				theDataPtr;
#if RomDns
	rdRequestPtr			theDnsRequestPtr;
#endif
#if RomTime
	tmRequestPtr			theTimeRequestPtr;
#endif
#if RomPlug
	upnpNotifyRequestPtr	theNotifyRequestPtr;
#endif

	theDataPtr = theConnectionPtr->fDataPtr;
#if RomDns
	/*
		If there is a DNS request pending, grab this connection.
	*/
	theDnsRequestPtr = RdCaptureRequest(theDataPtr);
	if (theDnsRequestPtr) {
		theConnectionPtr->fIpRemote =
				theDataPtr->fDnsDataPtr->fDnsServerIpAddress;
		RdStartDns(theDataPtr, theConnectionPtr, theDnsRequestPtr);
		return;
	}
#endif	/* RomDns */

#if RomTime
	/*
		If there is a Time request pending, grab this connection.
	*/
	theTimeRequestPtr = TmCaptureRequest(theDataPtr);
	if (theTimeRequestPtr) {
		theConnectionPtr->fIpRemote =
				theDataPtr->fTimeDataPtr->fTimeServerIpAddress;
		TmStartTime(theDataPtr, theConnectionPtr, theTimeRequestPtr);
		return;
	}
#endif	/* RomTime */

#if RomPlug
	/*
		See if the UPnP Discovery Listener is running.
	*/
	if (RuCheckUpnpListener(theDataPtr)) {
		/*
			The listener is running so we can use this connection for a
			Notification request. If there is a UPnP Notify request pending,
			grab this connection.
		*/
		theNotifyRequestPtr = RuCaptureNotifyRequest(theDataPtr);
		if (theNotifyRequestPtr) {
			RuSendNotification(theConnectionPtr, theNotifyRequestPtr);
		}
	}
	else {
		/*
			The UPnP Discovery Listener isn't running yet, so start it up.
		*/
		RuStartUpnpListener(theConnectionPtr);
		/*
			Return here so RomPlugControl doesn't grab the connection too.
		*/
		return;
	}
#endif	/* RomPlug */

#if RomPlugControl
	if (!CpCheckUpnpListener(theDataPtr)) {
		/*
			The Control Point Discovery Listener isn't running yet,
			so start it up.
		*/
		CpStartUpnpListener(theConnectionPtr);
	}
	else {
		/*
			Send a search packet, if needed.
		*/
		CpSendSearch(theConnectionPtr);
	}
#endif	/* RomPlugControl */
	return;
}

#endif	/* (kSudpNumberOfConnections > 0) */
