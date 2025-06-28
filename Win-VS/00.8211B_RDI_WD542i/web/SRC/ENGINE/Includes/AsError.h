/*
 *	File:		AsError.h
 *
 *	Contains:	Error code for external interface routines
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
 * * * * Release 4.20  * * *
 *		10/09/02	amp		add eRpCertImportFailed
 *		10/04/02	amp		Ssis --> Ssi for new RomPagerSecure
 * * * * Release 4.11  * * *
 *		07/01/02	pjr		add eRpFileDeleteError
 * * * * Release 4.10  * * *
 *		05/20/02	pjr		add eRpTcpConnectionClosing
 * * * * Release 4.07  * * *
 *		12/11/01	rhb		add eRpTlsNoExportKeyError
 * * * * Release 4.06  * * *
 * * * * Release 4.03  * * *
 *		10/10/01	amp		add eRpCertFailFieldLength
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		06/19/01	pjr		add Simple Serial Interface error codes
 *		05/09/01	bva		remove eRpPatchingError
 *		02/06/01	bva		add eRpUdpCannotOpenMulti
 *		10/06/00	pjr		rework Simple Printer Interface error codes
 *		07/31/00	dts		Support RomTelnet
 *		07/25/00	rhb		Support SSL/TLS
 *		07/20/99	pjr		add Simple Printer Interface error codes
 * * * * Release 3.0 * * * *
 *		02/06/99	bva		RpError.h -> AsError.h
 *		01/29/99	pjr		add eRpUdpAlreadyOpen, eRpUdpDeInitError
 * * * * Release 2.2 * * * *
 *		10/03/98	dts		add udp error codes
 * * * * Release 2.1 * * * *
 *		02/24/98	pjr		add eRpPatchingError
 * * * * Release 2.0 * * * *
 *		08/01/97	bva		add eRpTcpCannotOpenActive
 *		06/25/97	pjr		add eRpFatalFileSystemError and eRpFileAlreadyExists
 * * * * Release 1.6 * * * *
 *		05/14/97	rhb		add eRpFileSystemError
 *		03/26/97	bva		created from Stcp.h and Sfil.h
 * * * * Release 1.5 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */


#ifndef	_AS_ERROR_
#define	_AS_ERROR_


typedef enum { 
	eRpNoError 				= 0,

/* 
	Simple Tcp error codes 
*/
	eRpOutOfMemory 			= -1100,
	eRpTcpInitError 		= -1101,
	eRpTcpReceiveError 		= -1102,
	eRpTcpSendError 		= -1103,
	eRpTcpConnectError 		= -1104,
	eRpTcpDeInitError 		= -1105,
	eRpTcpInternalError 	= -1106,
	eRpTcpAlreadyOpen 		= -1107,
	eRpTcpCannotOpenPassive	= -1108,
	eRpTcpCloseError		= -1109,
	eRpTcpNoConnection	 	= -1110,
	eRpTcpZeroLengthRead	= -1111,
	eRpTcpAbortError		= -1112,
	eRpTcpAlreadyClosed	 	= -1113,
	eRpTcpCannotOpenActive	= -1114,
	eRpTcpConnectionClosing	= -1115,

/* 
	Simple File System error codes 
*/
	eRpFatalFileSystemError	= -1200,
	eRpFileSystemNotOpen 	= -1201,
	eRpFileNotOpen 			= -1202,
	eRpFileReadError 		= -1203,
	eRpFileWriteError 		= -1204,
	eRpFileOpenError 		= -1205,
	eRpFileCloseError 		= -1206,
	eRpFileCreateError		= -1207,
	eRpFileDeleteError		= -1208,
	eRpFileNotFound 		= -1209,
	eRpFileInvalidPosition 	= -1210,
	eRpFileZeroLengthRead 	= -1211,
	eRpFileAlreadyClosed	= -1212,
	eRpFileSystemNotClosed	= -1213,
	eRpFileAlreadyExists	= -1214,
	eRpFileNoRoom			= -1215,
	eRpFileSystemError 		= -1216,

/* 
	Simple Serial Interface error codes 
*/
	eRpSerialInitError 		= -1301,
	eRpSerialDeInitError 	= -1302,
	eRpSerialReceiveError 	= -1303,
	eRpSerialSendError 		= -1304,

/*
	Simple Printer Interface error codes
*/
	eRpPpServerError		= -1400,
	eRpPpInitError			= -1401,
	eRpPpCreateError		= -1402,
	eRpPpJobDoesNotExist	= -1403,
	eRpPpAttachError		= -1404,
	eRpPpDocFormatError		= -1405,
	eRpPpAttrValueError		= -1406,
	eRpPpAttrSubstitution	= -1407,
	eRpPpSendDataError		= -1408,
	eRpPpCloseJobError		= -1409,
	eRpPpAbortJobError		= -1410,
	eRpPpCancelJobError		= -1411,
	eRpPpFreeJobError		= -1412,
	eRpPpGetSprtdAttrError	= -1413,
	eRpPpGetAttrError		= -1414,
	eRpPpSetAttrError		= -1415,
	eRpPpValidateError		= -1416,

/* 
	Simple UDP error codes
*/
	eRpUdpInitError			= -1500,
	eRpUdpCannotOpenActive	= -1501,
	eRpUdpReceiveError		= -1502,
	eRpUdpAlreadyClosed		= -1503,
	eRpUdpCloseError		= -1504,
	eRpUdpNoConnection		= -1505,
	eRpUdpSendError			= -1506,
	eRpUdpAlreadyOpen		= -1507,
	eRpUdpDeInitError 		= -1508,
	eRpUdpCannotOpenMulti	= -1509,

/* 
	TLS error codes
*/
	eRpTlsInitError			= -1600,
	eRpTlsHandshakeError	= -1601,
	eRpTlsCloseError		= -1602,
	eRpTlsOutOfMemoryError	= -1603,
	eRpTlsReadError			= -1604,
	eRpTlsNoExportKeyError	= -1605,

/* 
	Simple security information storage error codes
*/
	eRpSsiBusyError			= -1700,
	eRpSsiRetrievalError	= -1701,
	eRpSsiStorageError		= -1702,

	eRpCertCreationBusy		= -1703,
	eRpCertCreateFailed		= -1704,
	eRpCertFailFieldLength	= -1705,
	eRpCertImportFailed		= -1706,

/*
	Cross-linkage for Telnet/CLI Errors
*/
	eRpCliLinkError			= -1800

} RpErrorCode;

#endif
