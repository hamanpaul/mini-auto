/*
 *	File:		AsParse.h
 *
 *	Contains:	Parsing definitions for RomPager product family
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
 * * * * Release 4.00  * * *
 * * * * Release 3.06  * * *
 *		12/06/99	bva		bump kMaxParseLength to accomodate phone browser
 * * * * Release 3.05  * * *
 * * * * Release 3.0 * * * *
 *		02/06/99	bva		RpParse.h -> AsParse.h
 *		01/21/99	bva		add HTTP 1.1 chunked support
 * * * * Release 2.2 * * * *
 *		08/31/98	bva		created from RomPager.h
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_AS_PARSE_
#define	_AS_PARSE_

#if RomPop
#define kMaxParseLength		kPop3MaxLineLength
#else
// +++ _Alphanetworks_Patch_, 12/18/2003, jacob_shih
//	when the web server receives the request, 
//	theConnectionPtr->fParsingControl.fCurrentBufferPtr is assigned 
//	to the buffer StcpConnectInfo::fReceiveBuffer which size is 
//	kStcpReceiveBufferSize.
//	so define kMaxParseLength as double of kStcpReceiveBufferSize
//	to be large enough to save the incoming data.
#if 1
#define kMaxParseLength		kStcpReceiveBufferSize*2
#else
#define kMaxParseLength		300
#endif
// --- _Alphanetworks_Patch_, 12/18/2003, jacob_shih
#endif

/*
	rpParsingControl structure
*/
typedef struct rpParsingControl {
	char *					fCurrentBufferPtr;
	char *					fCurrentBeginLinePtr;
	char *					fCurrentEndOfLinePtr;
	char *					fCurrentEndOfBufferPtr;
	char *					fTempBufferPtr;
	Signed32				fIncomingBufferLength;
	Signed32 				fHttpObjectLengthToRead;
	Unsigned16 				fCompleteLineLength;
	Unsigned16 				fCurrentLineLength;
	Unsigned16 				fPartialLineLength;
#if WcHttpOneDotOne
	Signed32				fChunkLength;
	Signed32				fChunkedContentLength;
	Signed32				fChunkLengthProcessed;
	Signed32				fCurrentChunkBufferLength;
	char *					fCurrentChunkBufferPtr;
	rpHttpChunkState		fChunkState;
	Boolean					fHaveChunk;
	Boolean					fObjectIsChunked;
#endif
} rpParsingControl, *rpParsingControlPtr;


#endif

