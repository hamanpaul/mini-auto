/*
 *	File:		AsEngine.h
 *
 *	Contains:	Internal Engine Definitions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Copyright:	?1995-2003 by Allegro Software Development Corporation
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
 *		09/22/03	bva		rework includes
 *		09/22/03	rhb		change prototype for AsReceiveText
 *		09/19/03	pjr		add parameters for RpRhLocalCredentials, bump
 *							fHttpPatternTable for Range support
 *		09/03/03	pjr		remove kMaxPathLength
 *		08/28/03	rhb		don't declare fSoftPageDataPtr if
 *								kRpNumberOfHttpRequests == 0
 *		08/25/03	rhb		expose RpDecodeBase64Data for RomPagerSecure
 *		08/10/03	amp		add AsResourceLocks
 *		07/08/03	bva		rework AsVariableAccess ifdefs
 *		06/26/03	nam		add prototype for floating point conversion
 *		06/23/03	amp		add UPnP Media Server support
 *		06/20/03	bva		add RomPagerXmlServices include
 *		06/16/03	bva		add kNumberOfFiles
 *		05/11/03	rhb		support enum types for Variable Access
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 *		03/17/03	amp		add UPnP Media Renderer support
 *		03/05/03	bva		bump revision
 * * * * Release 4.20  * * *
 *		01/31/03	pjr		add more parameters to the connection structure
 *							for managing Web Client connections
 *		01/08/03	amp		change A1TLSCfg.h to conform to Allegro naming
 *		12/18/02	amp		add demon connections
 *      12/18/02    amp     add RomCliSecure
 *		12/18/02	bva		bump fIndex/fLastConnectionChecked sizes
 *		12/17/02	bva		fPasswordSessionTimeout becomes Signed32
 *		12/12/02	pjr		remove fUserExitObject
 *		11/26/02	bva		include AsHttp.h for RomMailerBasic
 *		11/09/02	rhb		move AsStrToxxx to AsVarAcc.h
 *		11/07/02	pjr		add kAscii_AtSign, default FTP, POP3, SMTP ports
 *		11/06/02	pjr		add fRemotePort to rpConnection structure
 *		11/05/02	rhb		support enum types
 *		10/07/02    amp     move crypto interface to AcProtos.h
 *		10/07/02	bva		bump fHttpPatternTable for M-POST
 *		10/04/02	bva		bump revision
 *		10/04/02    amp     add AsSHA1, AsRC4, and random number generator
 *		10/04/02	amp		expose AsStrcasecmp for RomPagerSecure
 *		10/04/02    amp     update for new RomPagerSecure interfaces
 *		09/04/02	pjr		add fSessionStartFuncPtr and fUserExitObject
 * * * * Release 4.12  * * *
 *		08/19/02	bva		bump revision
 * * * * Release 4.11  * * *
 *		07/18/02	pjr		enable Strcasecmp prototypes for RomWebClient
 * * * * Release 4.10  * * *
 *		06/13/02	rhb		move AsExpandString prototype to AsVarAcc.h
 *		06/12/02	bva		rework kNumberOfConnections definitions
 *		06/08/02	bva		remove RomPagerSlaveIdentity
 *		06/05/02	bva		add kAscii_Dollar
 *		05/16/02	rhb		parameter to AsCheckAscii was 8-bits
 *		04/10/02	bva		fix compile warnings for AsCheckAsciiString
 * * * * Release 4.07  * * *
 *		03/26/02	dts		add support for CLI Initial Playback
 *		03/21/02	rhb		add AsCheckAsciiString
 *		01/14/02	rhb		RpBuildBase64String->AsBuildBase64String
 *		01/11/02	rhb		support Base64 conversions for RomXML
 * * * * Release 4.06  * * *
 * * * * Release 4.05  * * *
 *		01/07/02	bva		add fCliActive
 *		01/05/02	bva		rearrange RomTime includes
 * * * * Release 4.04  * * *
 *		11/26/01	pjr		fix some conditionals
 * * * * Release 4.03  * * *
 *		11/08/01	bva		add conditionals for fCloseFuncPtr, fCloseCookie
 *		10/05/01	pjr		include WcClient.h before AsCheck.h
 *		10/03/01	pjr		enable RpMD5 prototype for WcDigestAuthentication
 *		09/14/01	rhb		add fCloseFuncPtr and fCloseCookie
 *		08/30/01	pjr		include AsHttp.h for RomUpnp
 *		08/26/01	amp		add RmCharsetOption for RomMailer
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		07/27/01	bva		AsChar.h -> AsChrHdl.h
 *		07/26/01	rhb/pjr	use As64Bit macro in 64 bit definitions
 *		06/29/01	amp		add RomUpnpControl support
 *		06/28/01	pjr		add RpInitializeBox prototype
 *		05/04/01	dts		RomTime support
 *		05/02/01	bva		change prototype for RpConvertUnsigned32ToHex
 *		04/18/01	rhb		move RomXml includes
 *		03/16/01	rhb		add AsSendDataOutDotForm and AsSendDataOutHex
 *		03/15/01	amp		add AsHttpRq.h if RomMailer && !RomPagerServer
 *		03/13/01	rhb		add kAscii_Exclamation, kAscii_OpenSquare, and
 *									kAscii_CloseSquare
 *		03/07/01	bva		fix conditional flag
 *		03/06/01	bva		add kBackspace, kAscii_Backspace
 *		02/16/01	rhb		move Custom & SNMP Access protos here from RpIntPrt.h
 *		02/16/01	bva		add fUserPhrases to rpConnection structure,
 *							add kAsterisk
 *		02/15/01	bva		remove dictionary patching, add AsExpandString
 *		02/15/01	rhb		query index code is not needed for RomXML
 *		02/14/01	rhb		add prototype for AsReceiveText()
 *		02/13/01	rhb		RomPagerUse64BitIntegers->AsUse64BitIntegers
 *		02/12/01	rhb		moved Variable Access definitions to AsVarAcc.h
 *		02/09/01	rhb		Variable Access Lists are arrays
 *		02/09/01	bva/dts	RomConsole integration
 *		01/29/01	amp		conditionalize AsHttp.h
 *		12/22/00	amp		add #include "RxIntPrt.h"
 *		12/21/00	amp		add kHash, kAscii_Apostrophe and kAscii_Tab
 *		12/19/00	pjr		enable RpHexToNibble for RomCli
 *		12/18/00	pjr		include AsHttp.h
 *		12/15/00	pjr		rework asResponse and asVarAccessItem conditionals
 *		12/14/00	pjr		add AsVarAcc.c routine prototypes
 *		12/13/00	pjr		add AsConvrt.c routine prototypes
 *		12/04/00	pjr		add fMasterVariableListPtr to rpData structure
 *		11/27/00	pjr		fix kUnsigned64_Max definition
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		10/27/00	pjr		add index values to rpConnection structure
 *		10/12/00	bva		add RpGetRemoteIpAddress
 *		10/06/00	pjr		each IPP printer now has it's own fIppAccess entry
 *		09/22/00	bva		move UPNP includes
 *		09/14/00	bva		add some character constants from RomPager.h
 *		09/01/00	bva/dts	RomCLI/Telnet integration
 *		08/31/00	pjr		modify for the new security model
 *		08/31/00	bva		bump fHttpPatternTable for upnp
 *		08/20/00	bva		make fInitFlags Unsigned32
 *		07/30/00	bva		add RomUpnp support
 *		07/25/00	rhb		Support SSL/TLS
 *		07/05/00	rhb		enable RpHexToString & RpMD5 for WcDigestAuthentication
 *		06/29/00	pjr		add support for multiple IPP printer names
 *		06/02/00	bva		change RpBuildSeparator prototype
 *		05/26/00	amp		add fEpochReference tp rpData
 *		05/26/00	bva		add gRpDataPtr extern
 *		05/19/00	pjr		add fRxDataPtr to rpData structure
 *		04/05/00	pjr		add HTTP cookie support to Web Client
 *		03/30/00	pjr		add digest authentication to Web Client
 *		02/08/00	amp		change rpConnection.fAbortTimer to Unsigned16
 *		02/06/00	bva		created from RomPager.h
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */


#ifndef	_ASENGINE_
#define	_ASENGINE_

// +++ _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#if 1
#define	kMaxNameLength			128
#else
#define	kMaxNameLength			32
#endif
// --- _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#define	kMaxValueLength			256

#define kMaxProtocolLength		10
#define kMaxLineLength			256
#define kMaxSaveHeaderLength	128
#define kMaxStringLength		64
#define kDigestStringLength		33
#define kNonceCountLength		9
#define kMaxBoundaryLength		70

#include "AsError.h"
#include "AsExtern.h"
#include "AsStates.h"

#if RomPagerFileSystem
#include "Sfil.h"
#endif

#if RomConsole
#include "Sser.h"
#endif

#if RomPop
#include "PrRomPop.h"
#endif

#include "AsParse.h"

#if RomPagerSecure || RomWebClientSecure || RomCliSecure
/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#if 0
#include "AcProtos.h"
#endif
/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#endif

#if RomPagerSecure || RomWebClientSecure
/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#if 0
#include "A1IntPrt.h"
#include "A1TlsCfg.h"
#else
#include <web/ssl/inc/SSL.h>            /* for SSL Plus 3 */
#endif
/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#endif

#if RomPlug || RomPlugAdvanced
#include "RuUpnp.h"
#endif

#if RomPagerServer || RomPagerBasic
#include "RpStates.h"
#include "RomPager.h"
#elif RmFileOptions || RmCharsetOption
#include "AsHttpRq.h"
#endif

#if RomPagerServer || RomPagerBasic || RomWebClient || RomMailer || RomMailerBasic || RomPop || RomPlug
#include "AsHttp.h"
#endif

#if RomWebClient
#include "WcClient.h"
#endif

#include "AsCheck.h"

#if RomMailer
#include "RmMailer.h"
#endif

#if RomMailerBasic
#include "ScMailer.h"
#endif

#if RomPopBasic
#include "PcRomPop.h"
#endif

#if RomDns
#include "RdDns.h"
#endif

#if RomTime
#include "TmNtp.h"
#include "TmTime.h"
#endif

#if RomTelnet || RomCli
#include "AsChrHdl.h"
#endif

#if RomTelnet
#include "TnTelnet.h"
#endif

#if RomCliSecure
#include "ShSsh.h"
#endif

#if RomConsole
#include "CsConsol.h"
#endif

#if RomCli
#include "RcSesson.h"
#endif

#if RomPlugControl
#include "CpUpnp.h"
#endif

#define kVersion			"4.30b3"
#define kAllegro			"Allegro"

#define k8_BitsValue		256U
#define kUnsigned8_Max		255U
#define kUnsigned16_Max		65535U
#define kUnsigned32_Max		4294967295U
#define kSigned8_Max		127
#define kSigned16_Max		32767
#define kSigned32_Max		2147483647U

#if AsUse64BitIntegers
#define kUnsigned64_Max		As64Bit(18446744073709551615U)
#define kSigned64_Max		As64Bit(9223372036854775807U)
#endif

#define	kAscii_Null			0x00

#define	kAscii_0			0x30
#define	kAscii_9			0x39

#define	kAscii_A			0x41
#define	kAscii_F			0x46
#define	kAscii_Z			0x5A

#define	kAscii_a			0x61
#define	kAscii_f			0x66
#define	kAscii_x			0x78
#define	kAscii_y			0x79
#define	kAscii_z			0x7A

#define	kAscii_Backspace	0x08
#define	kAscii_Tab			0x09
#define	kAscii_Newline		0x0A
#define	kAscii_Return		0x0D
#define	kAscii_Space		0x20
#define kAscii_Exclamation	0x21
#define	kAscii_Quote		0x22
#define	kAscii_Hash			0x23
#define	kAscii_Dollar		0x24
#define	kAscii_Percent		0x25
#define	kAscii_Ampersand	0x26
#define	kAscii_Apostrophe	0x27
#define	kAscii_Plus			0x2B
#define	kAscii_Comma		0x2C
#define	kAscii_Hyphen		0x2D
#define	kAscii_Dot			0x2E
#define	kAscii_Slash		0x2F
#define	kAscii_Colon		0x3A
#define	kAscii_SemiColon	0x3B
#define	kAscii_LeftArrow	0x3C
#define	kAscii_OpenAngle	0x3C
#define	kAscii_Equal		0x3D
#define	kAscii_RightArrow	0x3E
#define	kAscii_CloseAngle	0x3E
#define	kAscii_Question		0x3F
#define	kAscii_AtSign		0x40
#define kAscii_OpenSquare	0x5B
#define kAscii_Backslash	0x5C
#define kAscii_CloseSquare	0x5D

#define NIBBLE_TO_HEX(theNibble) (	((theNibble) <= 9)             ?  \
									((theNibble) + kAscii_0)       :  \
									((theNibble) + kAscii_a - 10)  )

#define kCRLF				"\x0d\x0a"
#define kQuoteCRLF			"\"\x0d\x0a"
#define kQuote				"\""
#define kSpace				" "
#define kHyphen				"-"
#define kQuestion			"?"
#define kComma				","
#define kPeriod				"."
#define kColon				":"
#define kEqual				"="
#define kAmpersand			"&"
#define kOpenAngle			"<"
#define kCloseAngle			">"
#define kHash				"#"
#define kAsterisk			"*"
#define kSlash				"/"
#define kBackspace			"\010"


/*
	Default Port Constants
*/
#define kDefaultFtpControlPort	21
#define kDefaultFtpDataPort		20
#define kDefaultHttpPort		80
#define kDefaultIppPort			631
#define kDefaultPop3Port		110
#define kDefaultSmtpPort		25
#define kDefaultSshPort			22
#define kDefaultTelnetPort		23
#define kDefaultTlsPort			443


/*
	Time Constants
*/
#define kSecsPerMin				60
#define kSecsPerHour			3600
#define kSecsPerDay				86400
#define kSecsPerYear			31536000
#define kSecsPerFourYears		(4 * kSecsPerYear + kSecsPerDay)


/*
	rpDate structure
*/
typedef struct {
	Unsigned32	fDay;
	Unsigned32	fMonth;
	Unsigned32	fYear;
	Unsigned32	fHours;
	Unsigned32	fMinutes;
	Unsigned32	fSeconds;
} rpDate, *rpDatePtr;


#if RomPagerServer || RomCli || RomPlug || RomXml

/*
	Define the response structure that is used by conversion
	routines to build outbound data.
*/

typedef struct {
	struct rpData *		fDataPtr;
	char *				fBufferOnePtr;
	char *				fBufferTwoPtr;
	char *				fFillPtr;
	char *				fResponsePtr;
// +++ _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#if 1
	Unsigned32			fBufferSize;			//~~ modified by zlong, 7/11/2002. For RomXml framer and tftp_data_buf;
	Unsigned32			fFillBufferAvailable;	//~~ modified by zlong, 7/11/2002. For RomXml framer and tftp_data_buf;
#else
	Unsigned16			fBufferSize;
	Unsigned16			fFillBufferAvailable;
#endif
// --- _Alphanetworks_Patch_, 11/28/2003, jacob_shih
	Unsigned16			fResponseLength;
	Signed8				fCurrentFillBuffer;
	Boolean				fBufferReadyToSend;
	Boolean				fResponseIsChunked;
	Boolean				fLastChunk;
	Boolean				fOverflowError;
} asResponse, *asResponsePtr;

#endif	/* RomPagerServer || RomCli || RomPlug || RomXml */


/*
	This dummy structure is needed to keep some compilers happy with
	the procedure definition that follows.
*/
typedef struct {
	struct rpConnection * 	fConnectionPtr;
} rpDummyStruct, *rpDummyStructPtr;

/*
	Connection Error callback routine.
*/
typedef RpErrorCode	(*rpConnErrorProcPtr)(struct rpConnection * theConnectionPtr);
#define kRtReadbufferLength 2048 /* jesson for ssl, I don't know why */

/*
	rpConnection structure
*/
typedef struct rpConnection {
	struct rpData *		fDataPtr;
	rpParsingControl	fParsingControl;
	rpConnErrorProcPtr	fErrorProcPtr;
 	Unsigned32 			fIpLocal;
 	Unsigned32 			fIpRemote;
    Unsigned32			fProtocolTimer;
 	Unsigned16 			fLocalPort;
	Unsigned16			fRemotePort;
    Unsigned16			fAbortTimer;
	RpErrorCode			fError;
    Unsigned16  		fIndex;
    rpConnectionType	fProtocol;
    rpConnectionState   fState;
    Boolean				fSkipHttpRequestBlock;
#if AsVariableAccess || RomPagerServer
	Signed8				fIndexDepth;
	Signed16			fIndexValues[kAsIndexQueryDepth];
	char **				fUserPhrases;
	Boolean				fUserPhrasesCanBeCompressed;
#endif
#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
	rpHttpRequestPtr	fHttpRequestPtr;
#endif
#if RomPagerKeepAlive || RomPagerHttpOneDotOne || RomWebClient
    Unsigned8			fPersistenceTimer;
#endif
#if RomPagerServer && RomPagerRemoteHost
    rpRHRequestPtr		fRHRequestPtr;
#endif
#if RomMailer
    rmSmtpRequestPtr	fSmtpRequestPtr;
#endif
#if RomMailerBasic
    scSmtpRequestPtr	fScRequestPtr;
#endif
#if RomPop
    rpPop3RequestPtr	fPop3RequestPtr;
#endif
#if RomPopBasic
    rpPcRequestPtr		fPcPop3RequestPtr;
#endif
#if RomWebClient
    wcHttpSessionPtr	fWcSessionPtr;
	wcConnectState		fConnectState;
#if RomWebClientSecure
 	Unsigned32 			fConnectAddress;
	Unsigned16			fConnectPort;
#endif
#endif	/* RomWebClient */
#if RomDns
    rdRequestPtr		fDnsRequestPtr;
#endif
#if RomTime
    tmRequestPtr		fTimeRequestPtr;
#endif
#if RomPlug
    upnpNotifyRequestPtr	fUpnpRequestPtr;
#endif
#if SlavePager
	rpCgiPtr			fCgiPtr;
#endif
#if RomCliSecure
    shSessionPtr        fShSessionPtr;
#endif
#if RomTelnet
	tnSessionPtr		fTelnetSessionPtr;
#endif
#if RomConsole
	csSessionPtr		fConsoleSessionPtr;
#endif
#if RomCli
	Boolean				fCliActive;
	rcSessionPtr		fCliSessionPtr;
#endif
#if RomPagerFileSystem
	SfsFileInfo			fFileInfo;
	rpFileState			fFileState;
	RpErrorCode			fFileSystemError;
#endif
#if RomPagerSecure || RomWebClientSecure
	Boolean				fIsTlsFlag;
/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */	
	SSL			 		*fSslContext;
    char                fReadBufferPtr[kRtReadbufferLength];
	char				*fPendingSendBuffer;
	Signed32			fPendingSendBufferLen;
/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#endif
#if RomPagerServer
	rpConnCloseFuncPtr	fCloseFuncPtr;
	void *				fCloseCookie;
#endif
} rpConnection, *rpConnectionPtr;

/*
	Connection control blocks are the equivalent of a task control block
	in the Allegro engine. Scheduling is normally done on a round-robin
	basis for each connection. If multi-tasking support is enabled, then
	connections are scheduled externally to the Allegro engine and each
	connection is run separately.

	The connection pool is organized into groups of different kinds of
	connections. Since each group is contiguous, the fIndex field is
	used to identify which group a connection belongs to.

	Depending on the products that are used and how they are configured,
	each group may or may not exist.

	The groups are (in this order):

	TCP server connections (passive)
	TCP client connections (active)
	UDP connections
	Serial port connections
	RomCLI Initial script connection
*/

#ifndef kSudpNumberOfConnections
#define kSudpNumberOfConnections 0
#endif

#ifndef kSserNumberOfConnections
#define kSserNumberOfConnections 0
#endif

#if RomCliInitialPlayback || RomCliSecure
#define kDemonConnections 1
#else
#define kDemonConnections 0
#endif

#define kStcpActiveBase (kStcpNumberOfConnections - kStcpActiveConnections)
#define kSerialBase (kStcpNumberOfConnections + kSudpNumberOfConnections)
#define kInitialBase (kSerialBase + kSserNumberOfConnections)
#define kNumberOfConnections (kInitialBase + kDemonConnections)

/*
	File system connection IDs generally are mapped one-to-one to connection
	numbers. For instance, the Web server will use its TCP connection ID when
	it opens a file to serve over the TCP connection. In a few cases, a given
	application needs some additional files open and needs to allocate file
	IDs above the connection number space.
*/

#if WcCaching
	/*
		Web Client caching needs a file entry for each session
		and 1 work file entry for managing the cache index file
		and deleteing expired cache files.
	*/
#define kCacheFiles (kWcNumberOfHttpSessions + 1)
#else
#define kCacheFiles 0
#endif

#if RomCliValidate
#define kValidationFiles (kRcNumberOfCliSessions)
#else
#define kValidationFiles 0
#endif

#define kNumberOfFiles (kNumberOfConnections + kCacheFiles + kValidationFiles)
#define kValidationBase (kNumberOfConnections + kCacheFiles)


/*
	Demon function definition.
*/
typedef Boolean (*asDemonFuncPtr)(void	*theTaskDataPtr,
								rpConnectionPtr		theConnectionPtr);


/*
	rpServerPort structure
*/
typedef struct rpServerPort {
	StcpPort			fPort;
	rpConnectionType	fProtocol;
} rpServerPort, *rpServerPortPtr;

/*
	RomPager product family global data
*/
typedef struct rpData {
	Unsigned32				fInitFlags;
	rpConnectionPtr			fCurrentConnectionPtr;
	Unsigned32				fLastTime;
	Unsigned32				fRomSeconds;
	rpConnection 			fConnections[kNumberOfConnections];
#if RomPagerUseStandardTime && RomPagerCalendarTime
	Unsigned32				fEpochReference;
#endif
#if (kStcpServerCount > 0)
	rpServerPort 			fServerPorts[kStcpServerCount];
#endif
	Unsigned8				fServerPortIndex;
	Unsigned8				fDemonIndex;
#if (kRpNumberOfHttpRequests > 0)
	rpHttpRequest			fHttpRequests[kRpNumberOfHttpRequests];
#endif
#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
	rpHttpRequestPtr		fCurrentHttpRequestPtr;
#endif
#if RomPagerServer || RomPagerBasic
	rpPatternTable			fHttpPatternTable[35];
	char					fRomDateString[40];
	char					fExpiresDateString[40];
	Unsigned8				fHttpTaskCount;
#endif
#if RomPagerServer || RomDns
	Signed32				fSerial;
#endif
#if RomPagerServer
	char 					fBoxNameText[kMaxStringLength];
	rpObjectDescPtrPtr *	fMasterObjectListPtr;
#if RomPagerSecurity
	Signed32				fPasswordSessionTimeout;
	Boolean					fRealmLockingFlag;
	rpRealm					fRealms[kRpNumberOfRealms];
	rpUser					fUsers[kRpNumberOfUsers];
	rpSessionStartFuncPtr	fSessionStartFuncPtr;
#if RomPagerSecurityDigest
	rpPatternTable			fAuthPatternTable[8];
#endif
#if RomPagerIpp
	rpAccess				fIppAccess[kIppNumberOfPrinters];
#endif
#if RomPagerPutMethod
	rpAccess				fPutAccess;
#endif
#endif	/* RomPagerSecurity */
#if RomPagerFileUpload
	rpPatternTable			fMpPatternTable[3];
	rpPatternTable			fDispositionPatternTable[3];
#endif
#if RomPagerIpp
	void *					fIppDataPtr;
	char					fIppPrinterName[kIppNumberOfPrinters][kMaxNameLength];
#endif
#if RomPagerLogging
	Signed16				fLogItemCount;
	Signed16				fLogIndex;
	rpLogItem				fLogBuffer[kRpLogEvents];
	Unsigned32				fStartUpTime;
#endif
#if RomPagerSoftPages && (kRpNumberOfHttpRequests > 0)
	rpSoftPageDataPtr		fSoftPageDataPtr;
#endif
#if RomPagerRemoteHost
	rpRHPatternTable		fHttpProxyPatternTable[9];
	rpRHRequest				fRHRequests[kStcpActiveConnections];
#if RpRemoteHostMulti
	Unsigned32				fRemoteHostIpAddress[RpRemoteHostCount];
#else
	Unsigned32				fRemoteHostIpAddress;
#endif
#if RpRhLocalCredentials
#if RpRemoteHostMulti
	char					fRemoteHostEncodedAuth[RpRemoteHostCount]
													[kRpMaxEncUserPwdLen];
#else
	char					fRemoteHostEncodedAuth[kRpMaxEncUserPwdLen];
#endif
#endif	/* RpRhLocalCredentials */
#endif	/* RomPagerRemoteHost */
#if RpUserCookies
	void *					fUserCookie;
#endif
#if RpEtagHeader
	char					fRomEtagString[8];
#endif
#endif	/* RomPagerServer */
#if AsVariableAccess || RomPagerServer
	char **					fUserPhrases;
	Boolean					fUserPhrasesCanBeCompressed;
	asVarAccessItemHdl		fMasterVarAccessItemListPtr;
#endif
#if AsServerPollReduction && (RomPagerServer || RomPagerBasic || RomCli)
	Boolean					fSkipNextConnectionFlag;
	Unsigned16				fLastConnectionChecked;
#endif
#if AsResourceLocks
	void *					fLockDataPtr;
#endif
#if RomMailer
	rpSmtpDataPtr			fSmtpDataPtr;
#endif
#if RomMailerBasic
	rpScDataPtr				fScDataPtr;
#endif
#if RomPop
	rpPopDataPtr			fPopDataPtr;
#endif
#if RomPopBasic
	rpPcDataPtr				fPcDataPtr;
#endif
#if RomWebClient
	rpWcDataPtr				fWcDataPtr;
#endif
#if RomDns
	rpDnsDataPtr			fDnsDataPtr;
#endif
#if RomPlug
	rpRuDataPtr				fRuDataPtr;
#endif
#if RomPlugControl
	rpCpDataPtr				fCpDataPtr;
#endif
#if RomXml
	void *					fRxDataPtr;
#endif

/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#if 0
#if RomPagerSecure || RomWebClientSecure || RomCliSecure
	acRandomNumberData		fRandomData;
    char					fMemMgrBlock[kTotalBNMemSize];
#endif
#if RomPagerSecure || RomWebClientSecure
    rpA1DataPtr             fA1DataPtr;
#endif
#endif
/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#if RomCliSecure
    rpShDataPtr				fShDataPtr;
#endif

#if RomTelnet
	rpTelnetDataPtr			fTelnetDataPtr;
#endif
#if RomConsole
	rpConsoleDataPtr		fConsoleDataPtr;
#endif
#if RomCli
	rpCliDataPtr			fCliDataPtr;
#endif
#if RomTime
	rpTimeDataPtr			fTimeDataPtr;
#endif
} rpData, *rpDataPtr;


/*
	MD5 context structure
*/
typedef struct {
	Unsigned32		fState[4];		/* state (ABCD) */
	Unsigned32		fCount[2];		/* number of bits, modulo 2^64 (lsb first) */
	Unsigned8		fBuffer[64];	/* input buffer */
} rpMD5Context, *rpMD5ContextPtr;


#if !RomPagerDynamicGlobals
#if defined(WIN32)
extern rpData			gRpData;
#else
extern rpData			gRpData[];
#endif
#if RomPagerSoftPages
extern rpSoftPageData	gSoftPageData;
#endif
#if RomMailer
extern rpSmtpData		gRpSmtpData;
#endif
#if RomMailerBasic
extern rpScData			gRpScData;
#endif
#if RomPop
extern rpPopData		gRpPopData;
#endif
#if RomPopBasic
extern rpPcData			gRpPcData;
#endif
#if RomDns
extern rpDnsData		gRpDnsData;
#endif
#if RomTime
extern rpTimeData		gRpTimeData;
#endif
#if RomTelnet
extern rpTelnetData		gRpTelnetData;
#endif
#if RomCliSecure
extern rpShData			gRpShData;
#endif
#if RomConsole
extern rpConsoleData	gRpConsoleData;
#endif
#if RomCli
extern rpCliData		gRpCliData;
#endif
#if RomWebClient
extern rpWcData			gRpWcData;
#endif
#if RomPlug
extern rpRuData			gRpRuData;
#endif
#if RomPlugControl
extern rpCpData			gRpCpData;
#endif
/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#if 0
#if RomPagerSecure || RomWebClientSecure
extern rpA1Data         gRpA1Data;
#endif
#endif
/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#endif /* !RomPagerDynamicGlobals */


extern const char *		gRpMonthTable[];

/*
	AsBase64.c routines
*/
#if RomPagerSecurity || RomPop || RxBase64Conversions || RomPagerSecure
extern Unsigned16	RpDecodeBase64Data(char *theInputPtr,
						Unsigned16 theInputCount, char *theOutputPtr);
#endif

#if ((RomPagerServer && (RomMailer || RomMailerBasic)) || RmFileOptions || RmCharsetOption)
extern void			RpSendDataOutBase64(rpHttpRequestPtr theRequestPtr,
						Unsigned16 theLength, char *theBytesPtr);
extern void			RpSendOutBase64Padding(rpHttpRequestPtr theRequestPtr);
#endif

#if RomWebClient || RxBase64Conversions || RomCliSecure || RpRhLocalCredentials
extern void			AsBuildBase64String(char *theInputPtr,
						Unsigned16 theInputCount, char *theOutputPtr);
#endif


/*
	AsCommon.c routines
*/
extern void 		RpCatSigned32ToString(Signed32 theNumber,
						char *theStringPtr);
extern void 		RpCatUnsigned32ToString(Unsigned32 theNumber,
						char *theStringPtr,
						Unsigned8 theMinimumDigits);
extern Unsigned32 	RpGetRemoteIpAddress(void *theTaskDataPtr);
extern void 		RpStrLenCpy(char *theToPtr,
						char *theFromPtr,
						Unsigned32 theLength);
extern void			RpStrLenCpyTruncate(char *theToPtr,
						char *theFromPtr,
						Unsigned32 theLength);
extern void RpBuildIpAddressHostName(rpConnectionPtr theConnectionPtr,
						char *theBufferPtr);
extern void RpWriteIpAddressInDotForm(char *theBufferPtr,
						Unsigned32 theIpAddress);

#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption
extern void 		RpFlipResponseBuffers(rpHttpRequestPtr theRequestPtr);
extern void 		RpInitializeHtmlPageReply(rpHttpRequestPtr theRequestPtr);
extern void			RpInitializeReply(rpHttpRequestPtr theRequestPtr);
extern void			RpSendDataOutExtendedAscii(rpHttpRequestPtr theRequestPtr,
						const char * theAsciiPtr);
#endif

#if !RomPagerServer
/*
	These prototypes are also in RpCallBk.h, they are only
	needed here for products that don't include RpCallBk.h.
*/
extern Unsigned16	RpConvertSigned32ToAscii(Signed32 theNumber,
						char *theBufferPtr);
extern Unsigned16	RpConvertUnsigned32ToAscii(Unsigned32 theNumber,
						char *theBufferPtr);

#if AsUse64BitIntegers
extern Unsigned16	RpConvertSigned64ToAscii(Signed64 theNumber,
						char *theBufferPtr);
extern Unsigned16	RpConvertUnsigned64ToAscii(Unsigned64 theNumber,
						char *theBufferPtr);
#endif

#endif /* !RomPagerServer */


#if RomPagerHttpOneDotOne || WcHttpOneDotOne
extern void			RpConvertUnsigned32ToHex(Unsigned32 theData,
						char *theBufferPtr);
#endif

#if RomPagerServerPush || RomMailer
extern void			RpBuildSeparator(rpDataPtr theDataPtr,
						char *theSeparatorPtr);
#endif

#if RomPagerServer || RomPagerBasic || RomCli || RomPop || \
	RomWebClient || RomXml || RomPlugAdvanced
extern Unsigned8	RpHexToNibble(Boolean *theErrorFlag, char theHex);
#endif

#if RomPlugAdvanced
Unsigned32 RpHexToUnsigned32(Boolean *theErrorFlag, char *theHexPtr);
#endif

#if (RomPagerServer && RomPagerSecurityDigest) || \
	(RomWebClient && WcDigestAuthentication) || \
	(RomMailer && RmAggregateHtml) || \
	RomPlugAdvanced || \
	(RomPop && PrUseApop)
extern void			RpHexToString(unsigned char *theHexDataPtr,
									char 		*theStringPtr,
									Unsigned16 	theLength);
#endif

#if RomCli || RomWebClient || RomPagerSecure || RomWebClientSecure

/*
	These routines are the equivalent of the C library routines
	strcasecmp and strncasecmp that are included in some system libraries.
	They can be disabled if they are otherwise available.
*/
#if 1
extern Signed16	strcasecmp(const char 		*theFirstStringPtr,
							const char 		*theSecondStringPtr);
extern Signed16	strncasecmp(const char 		*theFirstStringPtr,
							const char 		*theSecondStringPtr,
							Unsigned32 		theLength);

#endif
#endif	/* RomCli || RomWebClient || RomPagerSecure || RomWebClientSecure */



/*
	AsConvrt.c routines
*/

#if AsVariableAccess || RomPagerServer

extern asItemError	AsCheckAsciiString(char *theStringPtr,
									Unsigned16 theLength);

#endif	/* AsVariableAccess || RomPagerServer */

#if AsVariableAccess || RxConversions

extern void			AsCloseChunkedBuffer(asResponsePtr theResponsePtr);

extern void			AsSetupChunkedBuffer(asResponsePtr theResponsePtr);

extern void			AsSendDataOutDecimalSigned(asResponsePtr theResponsePtr,
									Signed32			theData);

extern void			AsSendDataOutDotForm(asResponsePtr 	theResponsePtr,
									Unsigned8 			theLength,
									char 				*theHexDataPtr);

extern void			AsSendDataOutHex(asResponsePtr 		theResponsePtr,
									Unsigned8 			theLength,
									char 				theSeparator,
									char 				*theHexPtr);

#if AsUse64BitIntegers
extern void			AsSendDataOutDecimalSigned64(asResponsePtr theResponsePtr,
									Signed64 theData);
extern void			AsSendDataOutDecimalUnsigned64(asResponsePtr theResponsePtr,
									Unsigned64 theData);
#endif

#if AsUseFloatingPoint
extern void			AsSendDataOutFloat(asResponsePtr theResponsePtr,
									double theData);
#endif

#if AsUseEnums
extern asItemError	AsSendDataOutEnum(asResponsePtr 	theResponsePtr,
									asEnumElementPtr 	theEnumTablePtr,
									Unsigned32 			theValue);

#endif

#endif	/* AsVariableAccess || RxConversions */

#if AsVariableAccess || RomXml

// +++ _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#if 1
extern void			AsInitializeResponse(rpDataPtr 	theDataPtr,
									asResponsePtr 	theResponsePtr,
									char 			*theBufferOnePtr,
									char 			*theBufferTwoPtr,
									Unsigned32 		theBufferSize,
									Boolean 		theChunkedFlag);
#else
extern void			AsInitializeResponse(rpDataPtr 	theDataPtr,
									asResponsePtr 	theResponsePtr,
									char 			*theBufferOnePtr,
									char 			*theBufferTwoPtr,
									Unsigned16 		theBufferSize,
									Boolean 		theChunkedFlag);
#endif
// --- _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#endif

#if AsVariableAccess || RxConversions || RxExplicitIndexFraming

extern void			AsFlipResponseBuffers(asResponsePtr theResponsePtr);

extern void			AsSendDataOutDecimalUnsigned(asResponsePtr theResponsePtr,
						Unsigned32 theData);

#endif	/* AsVariableAccess || RxConversions || RxExplicitIndexFraming */

/*
	AsDate.c routines
*/
extern void 		RpBuildDateString(rpDataPtr 	theDataPtr,
									char 			*theDateString,
									Unsigned32 		theSeconds);
#if RomPagerUseStandardTime && RomPagerCalendarTime
extern Unsigned32	RpComputeEpochReference(void);
#endif
extern Unsigned32	RpGetDateInSeconds(rpDataPtr theDataPtr,
						rpDatePtr theDatePtr);
						
// +++ _Alphanetworks_Patch_, 12/20/2003, jacob_shih
Unsigned32 RpParseOSDate(rpDataPtr theDataPtr, char* theDateStringPtr);
// --- _Alphanetworks_Patch_, 12/20/2003, jacob_shih

#if !(RomPagerServer || RomPagerBasic)
/*
	These prototypes are also in RpCallBk.h and RsCallBk.h.  They are only
	needed here for products that don't include RpCallBk.h or RsCallBk.h.
*/
extern Unsigned32	RpGetMonthDayYearInSeconds(void *theTaskDataPtr,
									Unsigned32 		theMonth,
									Unsigned32 		theDay,
									Unsigned32 		theYear);
extern Unsigned32	RpGetSysTimeInSeconds(void *theTaskDataPtr);
#endif


/*
	AsMain.c routines
*/
extern RpErrorCode	RpConnectionAbortTcp(rpConnectionPtr theConnectionPtr);
extern RpErrorCode 	RpConnectionCheckTcpClose(rpConnectionPtr theConnectionPtr);
extern RpErrorCode	RpConnectionCloseTcp(rpConnectionPtr theConnectionPtr);
extern RpErrorCode 	RpConnectionReceiveTcp(rpConnectionPtr theConnectionPtr);
extern void 		RpFreeRequestControlBlock(rpConnectionPtr theConnectionPtr);
extern RpErrorCode	RpHandleSendReceiveError(rpConnectionPtr theConnectionPtr,
							RpErrorCode theError);


#if RomPagerSecurityDigest || PrUseApop || WcDigestAuthentication || \
		RomPagerSecure || RomWebClientSecure || RomCliSecure
/*
	AsMD5.c routines
*/
extern void			RpMD5(char * theStringPtr, Unsigned8Ptr theDigestResult);
extern void         RpMD5Init(rpMD5ContextPtr theContext);
extern void         RpMD5Update(rpMD5ContextPtr theContext,
                                Unsigned8Ptr theInput,
                                Unsigned32 theInputLength);
extern void         RpMD5Final(Unsigned8Ptr theDigest,
								rpMD5ContextPtr theContext);
#endif


/*
	AsParse.c routines
*/
extern void			RpConvertHeaderToLowerCase(rpParsingControlPtr theParsingPtr);
extern void			RpConvertTokenToLowerCase(char *theTokenPtr,
						Unsigned16 theTokenLength);
extern Unsigned16	RpFindCookieEnd(char *theCookiePtr);
extern Unsigned16	RpFindLineEnd(char *theStartOfTokenPtr);
extern Unsigned16	RpFindTokenDelimited(char *theStartOfTokenPtr,
						char theDelimiter);
extern char *		RpFindTokenDelimitedPtr(const char *theStartOfTokenPtr,
						const char theDelimiter);
extern Unsigned16	RpFindTokenEnd(char *theStartOfTokenPtr);
extern char *		RpFindTokenStart(char *theBeginLinePtr);
extern Unsigned16	RpFindValueLength(char *theValuePtr);
extern char *		RpFindValueStart(char *theValuePtr);
extern Unsigned32	RpParseDate(rpDataPtr theDataPtr, char *theDateStringPtr);
extern rpLineState	RpParseReplyBuffer(rpParsingControlPtr theParsingPtr);


/*
	AsVarAcc.c routines
*/

#if AsVariableAccess

extern asItemError	AsGetItemValue(void *theTaskDataPtr,
						char *theNamePtr,
						char *theValuePtr,
						Unsigned16 theLength);

extern asItemError	AsSetItemValue(void *theTaskDataPtr,
						char *theNamePtr,
						char *theValuePtr);

extern void			AsSetVarAccessItemList(void *theTaskDataPtr,
						asVarAccessItemHdl theMasterItemListPtr);

#endif /* AsVariableAccess */

#if AsVariableAccess || RomPagerServer

extern asItemError	AsReceiveText(rpDataPtr theDataPtr,
						char *theValuePtr,
						rpVariableType theSetPtrType,
						void *theSetPtr,
						asTextType theTextType,
						Unsigned8 theFieldSize,
						char *theNamePtr,
#if AsUseEnums
						asEnumElementPtr theEnumElementPtr,
#endif
						Signed16Ptr theIndexValuesPtr);

#endif	/* AsVariableAccess || RomPagerServer */


/*
	AsCustom.c routines
*/

#if AsCustomVariableAccess
extern void * 		AsRetrieveCustomItem(void *theTaskDataPtr,
								char *theHtmlNamePtr,
								Signed16Ptr theIndexValuesPtr,
								asTextType theItemDataType);
extern void			AsStoreCustomItem(void *theTaskDataPtr,
								char *theHtmlNamePtr,
								Signed16Ptr theIndexValuesPtr,
								asTextType theItemDataType,
								void *theItemPtr);
#endif


#if AsSnmpAccess
/*
	AsSnmp.c routines
*/
extern void * 		AsRetrieveSnmpItem(void *theTaskDataPtr,
								char *theHtmlNamePtr,
								Signed16Ptr theIndexValuesPtr,
								asTextType theItemDataType,
								asSnmpAccessItemPtr theSnmpAccessPtr);
extern void			AsStoreSnmpItem(void *theTaskDataPtr,
								char *theHtmlNamePtr,
								Signed16Ptr theIndexValuesPtr,
								asTextType theItemDataType,
								asSnmpAccessItemPtr theSnmpAccessPtr,
								void *theItemPtr);
#if RomPagerServer
extern void 		RpSendOutSnmpDisplay(rpHttpRequestPtr theRequestPtr,
								asSnmpAccessItemPtr theSnmpAccessPtr);
#if RomPagerSoftPages
extern void 		RpSoftSnmpAccess(void *theTaskDataPtr,
								Signed16Ptr theIndexValuesPtr,
								char *theOIDStringPtr);
#endif
#endif
#endif /* AsSnmpAccess && AsVariableAccess */


/*
	RpUser.c routines
*/
extern void			RpInitializeBox(rpDataPtr theDataPtr);

/*
	AsLock.c
*/
#if AsResourceLocks
extern void *	AsCreateLocks(void *theTaskDataPtr);
extern void		AsDestroyLocks(void *theTaskDataPtr, void *theLockDataPtr);
#endif

#if RomPagerServer
#include "RpIntPrt.h"
#endif

#if RomPagerBasic
#include "RsIntPrt.h"
#endif

#if RomMailer
#include "RmIntPrt.h"
#endif

#if RomMailerBasic
#include "ScIntPrt.h"
#endif

#if RomPop
#include "PrIntPrt.h"
#endif

#if RomPopBasic
#include "PcIntPrt.h"
#endif

#if RomWebClient
#include "WcIntPrt.h"
#endif

#if RomXml
#include "RxTrans.h"
#include "RxIntPrt.h"
#endif

#if RomPlugAdvanced || RomPagerXmlServices
#include "RpSoap.h"
#endif	/* RomPlugAdvanced */
#if RomPlug
#include "RuIntPrt.h"
#if RomPlugAdvanced && RuMediaRenderer
#include "MrIntPrt.h"
#endif
#if RomPlugAdvanced && RuMediaServer
#include "MsIntPrt.h"
#endif
#endif

#if RomPlugControl
#include "CpIntPrt.h"
#endif

#if RomDns
#include "RdIntPrt.h"
#endif

#if RomTime
#include "TmIntPrt.h"
#endif

#if RomTelnet
#include "TnProtos.h"
#include "TnIntPrt.h"
#endif

#if RomConsole
#include "CsProtos.h"
#include "CsIntPrt.h"
#endif
/* ------------------------------------ Start add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#if 0
#if RomCliSecure
#include "ShIntPrt.h"
#endif
#endif
/* ------------------------------------ End add by Scott Tsai 2004/02/25 for SSL by porting guide v0.01 */
#if RomCli
#include "RcIntPrt.h"
#endif


// +++ _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#if defined(WIN32)
#define L3_WEB_MAX_CONNECT_NUMBER	5
#else   /*Elsa modify for web*/
#define L3_WEB_MAX_CONNECT_NUMBER	4
#endif

#define kNumberOfTaskData   L3_WEB_MAX_CONNECT_NUMBER
// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih

#endif	/* _ASENGINE_ */
