/*
 *	File:		AsStates.h
 *
 *	Contains:	Embedded Internet Internal States
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
 *		08/07/03	bva		add eRpConnectionWaitingFilePosition
 * * * * Release 4.21  * * *
 * * * * Release 4.20  * * *
 *      12/18/02    amp     add eRpSshServer for RomCliSecure
 * * * * Release 4.12  * * *
 * * * * Release 4.07  * * *
 *		03/26/02	dts		add states for Cli Initial Playback
 * * * * Release 4.06  * * *
 * * * * Release 4.00  * * *
 *		06/29/01	amp		add eRpUpnpControlPointListener
 *		06/29/01	amp		add eRpConnectionWaitingSoapAction
 *		06/19/01	pjr		remove eRpSerialPortDead connection state
 *		05/09/01	bva		remove eRpProtocolDictPatch
 *		05/07/01	dts		add eRpTimeClient
 *		02/27/01	rhb		remove eRpConnectionReceive state 
 *		02/09/01	dts		add serial connection states
 *		07/30/00	bva		add RomUpnp support
 *		07/25/00	rhb		Support SSL/TLS
 *		01/18/00	bva		add protocol types
 *		01/17/00	amp		add protocol types
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 *		03/22/99	pjr		add eRpConnectionWcAvailable
 *		03/09/99	pjr		moved rpHttpChunkState from RpStates.h
 *		02/16/99	bva		derived from RpStates.h
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */


#ifndef	_AS_STATES_
#define	_AS_STATES_


/*
	RomPager family connection states 
*/
typedef enum {
	eRpConnectionClosed,
	eRpConnectionActiveAvailable,
	eRpConnectionWcAvailable,
	eRpConnectionUdpAvailable,
	eRpConnectionWaitingForConnection,
	eRpConnectionWaitingForReceive,
	eRpConnectionNeedsRequestBlock,
	eRpConnectionSendingReply,
	eRpConnectionClosing,
	eRpConnectionWaitingFileOpen,
	eRpConnectionWaitingFileClose,
	eRpConnectionWaitingFilePosition,
	eRpConnectionWaitingFileRead,
	eRpConnectionWaitingFileCreate,
	eRpConnectionWaitingFileWrite,
	eRpConnectionNeedsProtocolAction,
	eRpConnectionProtocolWaitExternal,
	eRpConnectionWaitingExternalSecurity,
	eRpConnectionWaitingUserExit,
	eRpConnectionWaitingSoapAction,
	eRpConnectionNeedsRemoteRequestBlock,
	eRpWaitingForSecurityInformation,
	eRpConnectionWaitingForTlsHandshake,
	eRpConnectionWaitingForTlsClose,
	eRpSerialPortStart,
	eRpSerialPortListening,
	eRpCliInitPlaybackStart,
	eRpCliInitPlaybackRun,
	eRpConnectionHolding
} rpConnectionState;


/* 
	RomPager family protocols 
*/
typedef enum {
	eRpHttpServer,
	eRpIppServer,
	eRpTlsServer,
	eRpTelnetServer,
    eRpSshServer,
/*
	The states above must match the protocol states in AsProtos.h.
*/
	eRpConsoleServer,
	eRpSmtpServer,
	eRpPop3Server,
	eRpHttpProxy,
	eRpHttpClient,
	eRpSmtpClient,
	eRpSmtpbClient,
	eRpPop3Client,
	eRpPop3bClient,
	eRpDnsClient,
	eRpUpnpDeviceListener,
	eRpUpnpDevice,
	eRpUpnpControlPoint,
	eRpUpnpControlPointListener,
	eRpTimeClient
} rpConnectionType;


/*
	File states 
*/
typedef enum {
	eRpFileClosed,
	eRpFileOpen,
	eRpFileCreated,
	eRpFileReading,
	eRpFileWriting
} rpFileState;


/* 
 	Line parsing states 
*/
typedef enum {
	eRpLineComplete,
	eRpLinePartial,
	eRpLineError,
	eRpLineEmpty
} rpLineState;


/* 
	Base64 encoding states
*/
typedef enum {
	eRpBase64_0,
	eRpBase64_1,
	eRpBase64_2
} rpBase64State;


/*
	HTTP chunked encoding states
*/
typedef enum {
	eRpGettingChunkedLength,
	eRpGettingChunkedData,
	eRpGettingChunkedEnd,
	eRpEndChunked
} rpHttpChunkState;


#endif	/* _AS_STATES_ */
