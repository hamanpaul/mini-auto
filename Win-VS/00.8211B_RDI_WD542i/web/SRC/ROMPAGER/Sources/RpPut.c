/*
 *	File:		RpPut.c
 *
 *	Contains:	RomPager routines for handling PUT method
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
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		09/21/00	pjr		eRpHttpFileSystemError -> eRpHttpInternalServerError
 *		06/08/00	pjr		handle extra CR LF at end of a non-chunked object
 *		05/26/00	bva		use gRpDataPtr
 *		04/24/00	rhb		move call of RpStringToMimeType() from 
 *								SetupFileInfo() to ProcessContentType()
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 *		03/29/99	pjr		fix chunked PUT
 * * * * Release 2.2 * * * *
 *		11/12/98	rhb		give a type to fDataPtr in the request structure
 *		11/03/98	pjr		debug, convert Content type to file type
 *		10/02/98	bva		created
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"

#if RomPagerPutMethod

static void			SetupFileInfo(rpHttpRequestPtr theRequestPtr);
static Boolean 		SetupPutBuffer(rpHttpRequestPtr theRequestPtr);

void RpSetupPutRequest(rpHttpRequestPtr theRequestPtr) {
	rpObjectDescriptionPtr	theObjectPtr;

	if (!theRequestPtr->fObjectIsChunked) {
		theRequestPtr->fParsingControlPtr->fHttpObjectLengthToRead = 
				theRequestPtr->fPostRequestLength;
	}
	theObjectPtr = &theRequestPtr->fLocalObject;
	theObjectPtr->fCacheObjectType = eRpObjectTypeDynamic;
#if RomPagerSecurity
	theObjectPtr->fObjectAccess = theRequestPtr->fDataPtr->fPutAccess;
#endif
	theRequestPtr->fObjectPtr = theObjectPtr;
	theRequestPtr->fObjectSource = eRpPutUrl;
	theRequestPtr->fHttpTransactionState = eRpEndUrlSearch;
	theRequestPtr->fHttpPutState = eRpHttpPutStart;
	return;
}


Boolean RpHandlePut(rpHttpRequestPtr theRequestPtr) {
	rpConnectionPtr			theConnectionPtr;
	Boolean					theReadMoreFlag;
	RpErrorCode				theResult;

	theConnectionPtr = theRequestPtr->fDataPtr->fCurrentConnectionPtr;
	theReadMoreFlag = False;

	switch (theRequestPtr->fHttpPutState) {

		case eRpHttpPutStart:
			/*
				Start the file create using the request URL.
			*/
			SetupFileInfo(theRequestPtr);

			theResult = SfsCreateFile(theConnectionPtr->fIndex,
										theRequestPtr->fPath,
										theRequestPtr->fFileInfoPtr);

			if (theResult == eRpNoError) {
				/*
					we didn't get an error on the file create call.
					change the state to wait for the file create
					to finish.
				*/
				theConnectionPtr->fState = eRpConnectionWaitingFileCreate;
				theRequestPtr->fHttpPutState = eRpHttpPutCreateFileDone;
			}
			else {
				/*
					we got an error creating the file.
				*/
				theConnectionPtr->fFileSystemError = theResult;
				theRequestPtr->fHttpPutState = eRpHttpPutFileSystemError;
			}
			break;

		case eRpHttpPutCreateFileDone:
			if ((theConnectionPtr->fFileState == eRpFileCreated) &&
				(theConnectionPtr->fFileSystemError == eRpNoError)) {
				/*
					the file was create was successfull.
				*/
				theRequestPtr->fHttpPutState = eRpHttpPutParsingData;
			}
			else {
				/*
					must have gotten an error creating the file.
				*/
				theRequestPtr->fHttpPutState = eRpHttpPutFileSystemError;
			}
			break;

		case eRpHttpPutParsingData:
			theReadMoreFlag = SetupPutBuffer(theRequestPtr);
			if (theRequestPtr->fPutBufferLength > 0) {
				/*
					write the data.
				*/
				theConnectionPtr->fFileState = eRpFileWriting;
				theResult = SfsWriteFile(theConnectionPtr->fIndex,
											theRequestPtr->fPutBufferPtr, 
											theRequestPtr->fPutBufferLength);

				if (theResult == eRpNoError) {
					theConnectionPtr->fState = eRpConnectionWaitingFileWrite;
					/*
						the next time RpHandlePut is entered, the
						file write should be done.
					*/
					theRequestPtr->fHttpPutState = eRpHttpPutWriteFileDone;
				}
				else {
					/*
						SfsWriteFile returned an error.  set up to let
						eRpHttpPutWriteFileDone handle the error.

						preserve the first file system error.
					*/
					if (theConnectionPtr->fFileSystemError == eRpNoError) {
						theConnectionPtr->fFileSystemError = theResult;
					}
					theRequestPtr->fHttpPutState = eRpHttpPutWriteFileDone;
				}
			}
			else if (theRequestPtr->fChunkState == eRpEndChunked) {
				/*
					we have received all of the data.
					set up to close the file.
				*/
				theRequestPtr->fIncomingBufferEnd = True;
				theRequestPtr->fHttpPutState = eRpHttpPutCloseFile;
			}
			break;

		case eRpHttpPutWriteFileDone:
			if (theConnectionPtr->fFileSystemError == eRpNoError) {
				/*
					a file write has finished.
				*/
				if (theRequestPtr->fIncomingBufferEnd) {
					/*
						we have received all of the data.
						set up to close the file.
					*/
					theRequestPtr->fHttpPutState = eRpHttpPutCloseFile;
				}
				else {
					/*
						we have not received all of the data.
						set up to get more data.
					*/
					theRequestPtr->fHttpPutState = eRpHttpPutParsingData;
					if (theRequestPtr->fParsingControlPtr->
							fIncomingBufferLength == 0) {
						theReadMoreFlag = True;
					}
				}
			}
			else {
				/*
					there has been a file system error.
					set up to close the file.
				*/
				theRequestPtr->fHttpPutState = eRpHttpPutCloseFile;
			}
			break;

		case eRpHttpPutCloseFile:
			/*
				close the file.
			*/
			if (theConnectionPtr->fFileSystemError == eRpNoError) {
				theResult = SfsCloseFile(theConnectionPtr->fIndex, True);
			}
			else {
				theResult = SfsCloseFile(theConnectionPtr->fIndex, False);
			}

			if (theResult == eRpNoError) {
				theConnectionPtr->fState = eRpConnectionWaitingFileClose;
				theRequestPtr->fHttpPutState = eRpHttpPutCloseFileDone;
			}
			else {
				/*
					we got an error closing the file!
					preserve the first file system error.
				*/
				if (theConnectionPtr->fFileSystemError == eRpNoError) {
					theConnectionPtr->fFileSystemError = theResult;
				}
				theRequestPtr->fHttpPutState = eRpHttpPutFileSystemError;
			}
			break;

		case eRpHttpPutCloseFileDone:
			if (theConnectionPtr->fFileSystemError == eRpNoError) {
				/*
					there has not been a file system error.
				*/
				theRequestPtr->fHttpResponseState = eRpHttpPutCompleted;
				theRequestPtr->fHttpTransactionState = 
						eRpSendingHttpHeaders;
			}
			else {
				/*
					there has been a file system error.
				*/
				theRequestPtr->fHttpPutState = eRpHttpPutFileSystemError;
			}
			break;

		case eRpHttpPutFileSystemError:
			theRequestPtr->fHttpResponseState = eRpHttpInternalServerError;
			theRequestPtr->fObjectPtr = &gRpFileSystemErrorPage;
			theRequestPtr->fHttpPutState = eRpHttpPutFlush;
			break;

		case eRpHttpPutConnectionError:
			/*
				We've had a connection error.  If a file was open,
				it should already be closed.
			*/
			(void) RpConnectionCheckTcpClose(theConnectionPtr);
			break;

		case eRpHttpPutFlush:
			theReadMoreFlag = SetupPutBuffer(theRequestPtr);
			if (theRequestPtr->fIncomingBufferEnd) {
				/*
					we have received all of the data.
					set up to start the response.
				*/
				theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
			}
			break;

	}	/* switch (theRequestPtr->fHttpPutState) */

	return theReadMoreFlag;
}


/*
	Set up the file info. parameters.

	This includes the file access (protection), file date, and file type.
*/

static void SetupFileInfo(rpHttpRequestPtr theRequestPtr) {
	SfsFileInfoPtr	theFileInfoPtr;

	theFileInfoPtr = theRequestPtr->fFileInfoPtr;
#if RomPagerSecurity
	theFileInfoPtr->fFileAccess = theRequestPtr->fDataPtr->fPutAccess;
#else
	theFileInfoPtr->fFileAccess = kRpPageAccess_Unprotected;
#endif
	theFileInfoPtr->fFileDate = theRequestPtr->fBrowserDate;

	if (theRequestPtr->fDataType == eRpDataTypeOther) {
		RP_STRCPY(theRequestPtr->fFileInfoPtr->fOtherMimeType,
				theRequestPtr->fHttpContentType);
	}

	theFileInfoPtr->fFileType = theRequestPtr->fDataType;

	return;
}


static Boolean SetupPutBuffer(rpHttpRequestPtr theRequestPtr) {
	Signed32			theBufferLength;
	rpParsingControlPtr	theParsingPtr;
	Boolean				theReadMoreFlag;

	theParsingPtr = theRequestPtr->fParsingControlPtr;
	theRequestPtr->fPutBufferLength = 0;
	theReadMoreFlag = False;

	if (theParsingPtr->fIncomingBufferLength == 0) {
		/*
			If there is no more data in the TCP buffer, signal the higher
			layer that we want some.
		*/
		return True;
	}

	/*
		The incoming buffer may be chunked.  If it is, we will pass a
		complete chunk if it is contained in the TCP buffer and multiple
		pieces of the chunk if it overlaps TCP buffers.  If it isn't,
		we'll pass the full TCP buffer to the  PUT parser.  Depending on
		how the client sends the data, this may have interesting
		performance implications.
	*/
	if (theRequestPtr->fPostRequestLength > 0) {
		/*
			We were passed in a Content-Length, so we aren't chunked.
		*/
		if (theParsingPtr->fIncomingBufferLength >=
				theParsingPtr->fHttpObjectLengthToRead) {
			/*
				The buffer contains all we need for this object, so set
				the length to the remaining length of the object.  In an
				HTTP 1.1 pipelined environment, there may be more in the
				buffer than just our object.  Some clients also
				incorrectly append a CRLF to the object.
			*/
			theBufferLength = theParsingPtr->fHttpObjectLengthToRead;
			theRequestPtr->fIncomingBufferEnd = True;
		}
		else {
			/*
				The buffer has less data than what we need for the
				object, so take the whole thing.
			*/
			theBufferLength = theParsingPtr->fIncomingBufferLength;
		}

		theRequestPtr->fPutBufferPtr = theParsingPtr->fCurrentBufferPtr;
		theRequestPtr->fPutBufferLength = theBufferLength;
		theParsingPtr->fCurrentBufferPtr += theBufferLength;
		theParsingPtr->fIncomingBufferLength -= theBufferLength;
		theParsingPtr->fHttpObjectLengthToRead -= theBufferLength;
	}
	else {
		/*
			We must be chunked or we wouldn't have gotten here, so get a chunk.
		*/
		theReadMoreFlag = RpGetChunkedData(theRequestPtr);
		if (theRequestPtr->fHaveChunk) {
			theRequestPtr->fPutBufferPtr =
					theRequestPtr->fCurrentChunkBufferPtr;
			theRequestPtr->fPutBufferLength =
					theRequestPtr->fCurrentChunkBufferLength;
		}
		/*
			If we have finished with all the chunked data, we need to move the
			input buffer pointer past the optional chunked footer and the
			trailing empty line.
		*/
		while (theRequestPtr->fChunkState == eRpGettingChunkedEnd &&
				!theReadMoreFlag) {
			theReadMoreFlag = RpGetChunkedData(theRequestPtr);
		}
		/*
			Is this the last buffer?
		*/
		if (theRequestPtr->fChunkState == eRpEndChunked) {
			theRequestPtr->fIncomingBufferEnd = True;
		}
	}
	return theReadMoreFlag;
}

#endif	/* RomPagerPutMethod */
