/*
 *	File:		AsHttpRq.h
 *
 *	Contains:	Web Server Definitions for when there is no Web Server
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
 * * * * Release 4.03  * * *
 *		09/26/01	amp		add RmCharsetOption to RomMailer
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		03/15/01	amp		created
 * * * * Release 3.10  * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_ASHTTPRQ_
#define	_ASHTTPRQ_

#if !RomPagerServer && (RmFileOptions || RmCharsetOption)
/*
	rpHttpRequest structure 
*/
typedef struct rpHttpRequest {
	struct rpData *			fDataPtr;
	Boolean					fInUse;
/*
	keep fHtmlResponseBufferOne, fHtmlResponseBufferTwo, fHttpWorkBuffer in this order
*/
	char					fHtmlResponseBufferOne[kHtmlMemorySize];
	char					fHtmlResponseBufferTwo[kHtmlMemorySize];
	char					fHttpWorkBuffer[2];
	char					fHttpTempLineBuffer[kMaxParseLength];
	Boolean					fHtmlBufferReady;
	Signed8					fHtmlCurrentBuffer;
	char *					fHtmlFillPtr;
	char *					fHtmlResponsePtr;
	Unsigned16				fFillBufferAvailable;
	Unsigned16				fHtmlResponseLength;
	rpBase64State			fBase64State;
	Signed32				fBase64LineLength;
	char					fBase64Chars[3];
} rpHttpRequest, *rpHttpRequestPtr;

#endif	/* !RomPagerServer && (RmFileOptions || RmCharsetOption) */

#endif	/* _ASHTTPRQ_ */
