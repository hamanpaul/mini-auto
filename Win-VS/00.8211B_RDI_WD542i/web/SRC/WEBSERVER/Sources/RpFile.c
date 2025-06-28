/*
 *	File:		RpFile.c
 *
 *	Contains:	Routines to support external file system pages
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
 *		08/16/03	bva		add support for Range requests
 * * * * Release 4.20  * * *
 *		01/31/03	rhb		fix Soft Pages from Remote Host
 *		01/28/03	rhb		call Soft Pages Preprocessing function
 * * * * Release 4.12  * * *
 * * * * Release 4.07  * * *
 *		03/04/02	rhb		fix end of connection handling when closing file
 * * * * Release 4.06  * * *
 * * * * Release 4.03  * * *
 *		03/04/02	rhb		check for end of connection better when closing file
 *		10/16/01	rhb		don't enable SoftPages in RpFileInsertItem
 *		10/07/01	rhb		handle file read of 0 bytes with EOF better
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		05/26/00	bva		use gRpDataPtr
 *		05/11/00	pjr		enable eRpConnectionWaitingFileWrite code for
 * 							RomPagerPutMethod
 *		02/09/00	bva		use AsEngine.h
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 * * * * Release 3.06  * * *
 * * * * Release 3.02  * * *
 *		05/03/99	bva		fix SoftPages EOF handling in eRpConnectionWaitingFileRead
 * * * * Release 3.0 * * * *
 *		04/02/99	pjr		fix compiler warnings
 *		01/19/99	pjr		set up fObjectAccess only for RomPagerServer with
 *								RomPagerSecurity
 *		01/14/99	pjr		change the conditional for the whole file
 *		12/11/98	rhb		merge Soft Pages
 * * * * Release 2.2 * * * *
 *		11/25/98	rhb		fix multi buffer bug with eRpItemType_File
 *		11/10/98	rhb		implement eRpItemType_File
 *		11/03/98	pjr		RomPagerPutMethod uses the same file close code as
 *							File Upload
 *		10/21/98	bva		add PUT support
 *		09/21/98	pjr		move file related fields to the connection block
 *		08/20/98	pjr		rework connection error handling
 *		08/13/98	bva		add theCompleteFlag to SfsCloseFile call
 * * * * Release 2.1 * * * *
 *		04/15/98	bva		remove eRpObjectTypeFile 
 *		03/04/98	rhb		add cast to theByteCount in 
 *								eRpConnectionWaitingFileRead state
 *		02/18/98	pjr		fix eRpConnectionWaitingFileRead case to handle
 *							an End Of File with 0 bytes read.
 *		01/19/98	bva		eRpConnectionNeedsHttpAction ->
 *								eRpConnectionNeedsProtocolAction
 * * * * Release 2.0 * * * *
 *		08/07/97	bva		set file dates of 0 to system date
 *		08/06/97	bva		add default to switch statement
 *		07/29/97	pjr		eRpParsingMultipartWaitForFS -> eRpParsingMultipart.
 *		07/12/97	bva		fFileObject -> fLocalObject
 *		07/07/97	pjr		changed file upload state name
 *		06/26/97	pjr		add support for form based file upload
 *								improve error handling
 *		05/25/97	bva		rework URL dispatching
 * * * * Release 1.6 * * * *
 *		05/14/97	rhb		treat eRpFileOpenError like eRpFileNotFound
 *		03/26/97	bva		change error codes
 *		03/10/97	bva		rework connection states
 *		01/31/97	bva		add support for RpSearchRomFirst
 *		01/29/97	bva		use RpConnectionCheckTcpClose
 *		01/24/97	rhb		clean up error types
 *		01/12/97	bva		added close/read file
 *		12/20/97	bva		added open file
 *		12/13/96	bva		created
 * * * * Release 1.5 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if (RomPagerServer || RomPagerBasic) && RomPagerFileSystem

static RpErrorCode	ConnErrorCloseFile(rpConnectionPtr theConnectionPtr);

RpErrorCode RpHandleFileStates(rpDataPtr theDataPtr) {
	rpConnectionPtr 		theConnectionPtr;
	rpHttpRequestPtr		theRequestPtr;
	rpObjectDescriptionPtr	theObjectPtr;
	RpErrorCode				theResult;
	SfsStatus				theCompletionStatus;
	SfsFileInfoPtr			theFileInfoPtr;
	SfsLength				theByteCount;

	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	theObjectPtr = &theRequestPtr->fLocalObject;
	theFileInfoPtr = &theConnectionPtr->fFileInfo;
	theResult = eRpNoError;

	switch (theConnectionPtr->fState) {

		case eRpConnectionWaitingFileOpen:

			theResult = SfsOpenStatus(theConnectionPtr->fIndex, 
							&theCompletionStatus,
							theFileInfoPtr);

			if (theResult == eRpNoError) {
				if (theCompletionStatus == eSfsComplete) {
					theConnectionPtr->fFileState = eRpFileOpen;
					/*
						Set up the connection error callback routine.
					*/
					theConnectionPtr->fErrorProcPtr = &ConnErrorCloseFile;

					/*
						We opened the file, so set up the file system object,
						so it can be served.
					*/
					theRequestPtr->fObjectPtr = theObjectPtr;
					theObjectPtr->fURL = theRequestPtr->fPath;
					theObjectPtr->fLength = theFileInfoPtr->fFileSize;
					theObjectPtr->fMimeDataType = theFileInfoPtr->fFileType;
#if RomPagerServer && RomPagerSecurity
					theObjectPtr->fObjectAccess = theFileInfoPtr->fFileAccess;
#endif
					/*
						File objects are assumed to be relatively static
						from a caching point of view.  If there is a date
						from the file system, we will use it to notify
						the browsers for cache control.
					*/
					theObjectPtr->fCacheObjectType = eRpObjectTypeStatic;

#if RomPagerSoftPages
					/*
						If SoftPages is enabled, all HTML files are scanned 
						for SoftPages tags and they are considered dynamic.
					*/
					if (theFileInfoPtr->fFileType == eRpDataTypeHtml
#if RpFileInsertItem
						&& theRequestPtr->fObjectSource != eRpRomUrl
#endif
					) {
						theRequestPtr->fSoftPageFlag = True;
						theObjectPtr->fCacheObjectType = eRpObjectTypeDynamic;
						PreprocessSoftPage(theDataPtr, 
								theConnectionPtr->fIndexValues);
					}
#endif
					if (theFileInfoPtr->fFileDate == 0) {
						/*
							If we didn't get a file date from the file system,
							set the date to the rom date.
						*/
						theFileInfoPtr->fFileDate = theDataPtr->fRomSeconds;
					}
					theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
				}
				/*
					else, keep trying to open it
				*/
			}
			else {

				if (theResult == eRpFatalFileSystemError) {
					/*
						If this is a fatal error, abort the TCP connection
						and return the error.
					*/
					RpConnectionAbortTcp(theConnectionPtr);
				}
#if RpFileInsertItem
				if (theRequestPtr->fObjectSource == eRpRomUrl) {
					/*
						If this is not a file object, just generate some error
						text on the page and continue.
					*/
					RpSendFileInsertItemError(theConnectionPtr, 
							eRpSendingHttpResponse);
					theResult = eRpNoError;
				}
#endif
				else {
					/*
						Save the error away in the connection block, to be
						handled at a higher level and return eRpNoError.
					*/
					theConnectionPtr->fFileSystemError = theResult;
					theResult = eRpNoError;

					/*
						we couldn't open the file, so set up the 
						file not found states.
					*/
					theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
				}
			}
			break;


		case eRpConnectionWaitingFileClose:

			theResult = SfsCloseStatus(theConnectionPtr->fIndex, 
							&theCompletionStatus);

			switch (theRequestPtr->fHttpTransactionState) {

#if RomPagerFileUpload || RomPagerPutMethod
				case eRpParsingMultipart:
				case eRpHandlePut:

					if (theResult == eRpNoError) {
						if (theCompletionStatus == eSfsComplete) {
							theConnectionPtr->fFileState = eRpFileClosed;
							theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
						}
						/*
							else, keep trying to close it
						*/
					}
					else {
						/*
							We got an error closing the file.  Mark it
							as closed anyway.
						*/
						theConnectionPtr->fFileState = eRpFileClosed;

						if (theResult == eRpFatalFileSystemError) {
							/*
								If this is a fatal error, abort the TCP connection.
							*/
							RpConnectionAbortTcp(theConnectionPtr);
						}
						else {
							/*
								If there isn't already an error saved in the
								connection block, save this one.
							*/
							if (theConnectionPtr->fFileSystemError == eRpNoError) {
								theConnectionPtr->fFileSystemError = theResult;
							}

							theResult = eRpNoError;
							theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
						}
					}

					break;
#endif	/* RomPagerFileUpload || RomPagerPutMethod */

			default:
				/*
					This must have been a file read operation (serving a file).
				*/
				if ((theResult == eRpNoError &&
					theCompletionStatus == eSfsComplete) ||
					theResult != eRpNoError) {
					/*
						The file close either completed, or failed.  In
						either case, Mark the file as closed.
						
						If this is a file object see if we can close the 
						connection and/or free any request blocks.

						Otherwise, just continue processing items.
					*/
					theConnectionPtr->fFileState = eRpFileClosed;
#if RpFileInsertItem
					if (theRequestPtr->fHttpTransactionState !=
							eRpClosingInsertFileItem) {
						theResult = RpConnectionCheckTcpClose(theConnectionPtr);
					}
					else {
						theConnectionPtr->fState = 
								eRpConnectionNeedsProtocolAction;
						theRequestPtr->fHttpTransactionState = 
								eRpSendingHttpResponse;
					}
#else
					theResult = RpConnectionCheckTcpClose(theConnectionPtr);
#endif
				}
				/*
					else keep waiting for the file close to complete or fail.
				*/
				break;
			}
			break;


		case eRpConnectionWaitingFileRead:
			/*
				The assumption for file reading is that the file
				buffers are being sent out the TCP port as part of a
				HTTP object or SMTP message body.
			*/ 
			theResult = SfsReadStatus(theConnectionPtr->fIndex, 
							&theCompletionStatus, &theByteCount);
			if (theResult == eRpNoError) {
				if (theCompletionStatus == eSfsComplete ||
						theCompletionStatus == eSfsEndOfFile) {
					theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
#if RomPagerRanges
					if (theRequestPtr->fHaveRangeRequest) {
						if (theRequestPtr->fBytesSent + theByteCount >= 
								theRequestPtr->fObjectPtr->fLength) {
							/*
								This file buffer will complete the range request,
								so adjust the byte count and mark for file close.
							*/
							theByteCount = theRequestPtr->fObjectPtr->fLength -
								theRequestPtr->fBytesSent;	
							theCompletionStatus = eSfsEndOfFile;
						}
						else {
							/*
								Just accumulate the bytes sent from this buffer.
							*/
							theRequestPtr->fBytesSent += theByteCount;
						} 
					}
#endif
					theRequestPtr->fFillBufferAvailable -= 
							(Unsigned16) theByteCount;
#if RomPagerSoftPages
					if (theRequestPtr->fSoftPageFlag) {
						/*
							The file is HTML and needs parsing.
						*/
						theRequestPtr->fHtmlFillPtr += theByteCount;
						if (theRequestPtr->fFillBufferAvailable > 0 &&
								theCompletionStatus != eSfsEndOfFile) {
							/*
								There's more file and room for it, read it.
							*/
							theConnectionPtr->fState = 
									eRpConnectionWaitingFileRead;
							theResult = SfsReadFile(theConnectionPtr->fIndex,
									theRequestPtr->fHtmlFillPtr,
									theRequestPtr->fFillBufferAvailable);
						}
						else {
							/*
								There's no more file or no more room.
							*/
							RpInitializeParsingHtml(theConnectionPtr, (Boolean)
									(theCompletionStatus == eSfsEndOfFile));
						}
						break;
					}
#endif	/* RomPagerSoftPages */
					if (theByteCount != 0) {
						if (theCompletionStatus == eSfsEndOfFile) {
#if RpFileInsertItem
							if (theRequestPtr->fObjectSource != eRpRomUrl) {
								theRequestPtr->fHttpTransactionState = 
										eRpHttpResponseComplete;
							}
							else {
								theRequestPtr->fHttpTransactionState = 
										eRpClosingInsertFileItem;
							}
#else
							theRequestPtr->fHttpTransactionState = 
									eRpHttpResponseComplete;
#endif
						}
						theRequestPtr->fHtmlBufferReady = False;
						RpFlipResponseBuffers(theRequestPtr);
						/*
							We don't care about the RpSendReplyBuffer result.
							If it fails, the Connection Error code will call
							our error callback routine (ConnErrCloseFile), which
							will do clean up, including closing the file.
						*/
						(void) RpSendReplyBuffer(theConnectionPtr);
					}
					else if (theCompletionStatus == eSfsEndOfFile) {
						/*
							We may get an EOF with no data.
						*/
#if RpFileInsertItem
						if (theRequestPtr->fObjectSource != eRpRomUrl) {
							theRequestPtr->fHttpTransactionState = 
									eRpHttpResponseComplete2;
						}
						else {
							theRequestPtr->fHttpTransactionState = 
									eRpClosingInsertFileItem;
						}
#else
						theRequestPtr->fHttpTransactionState = 
								eRpHttpResponseComplete2;
#endif
						theResult = RpFileClose(theConnectionPtr);
						
					}
				}
				/*
					else, keep trying to read it
				*/
			}
			else {
				/*
					We got an error on the read, so close the file. Then
					if the source is ROM, the generate error text on the page, 
					otherwise close the TCP connection.
				*/
#if RpFileInsertItem
				if (theRequestPtr->fObjectSource == eRpRomUrl) {
					RpSendFileInsertItemError(theConnectionPtr, 
							eRpClosingInsertFileItem);
				}
				else {
					theRequestPtr->fHttpTransactionState = 
							eRpHttpResponseComplete2;
				}
#else
				theRequestPtr->fHttpTransactionState = eRpHttpResponseComplete2;
#endif
				theResult = RpFileClose(theConnectionPtr);
			}
			break;


		case eRpConnectionWaitingFileCreate:

#if RomPagerFileUpload || RomPagerPutMethod

			theResult = SfsCreateStatus(theConnectionPtr->fIndex, 
										&theCompletionStatus);

			if (theResult == eRpNoError) {
				if (theCompletionStatus == eSfsComplete) {
					theConnectionPtr->fFileState = eRpFileCreated;
					/*
						Set up the connection error callback routine.
					*/
					theConnectionPtr->fErrorProcPtr = &ConnErrorCloseFile;

					/*
						We've created the file. Go write something to it.
					*/
					theConnectionPtr->fState =
							eRpConnectionNeedsProtocolAction;
				}
			}
			else {
				/*
					We got an error creating the file.
					Assume the file is closed.
				*/
				theConnectionPtr->fFileState = eRpFileClosed;

				if (theResult == eRpFatalFileSystemError) {
					/*
						If this is a fatal error abort the TCP connection.
					*/
					RpConnectionAbortTcp(theConnectionPtr);
				}
				else {
					/*
						Save the error away in the connection block, to be
						handled at a higher level and return eRpNoError.
					*/
					theConnectionPtr->fFileSystemError = theResult;
					theResult = eRpNoError;

					theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
				}
			}
#endif	/* RomPagerFileUpload || RomPagerPutMethod */

			break;


		case eRpConnectionWaitingFileWrite:

#if RomPagerFileUpload || RomPagerPutMethod
			theResult = SfsWriteStatus(theConnectionPtr->fIndex, 
							&theCompletionStatus, &theByteCount);

			if (theResult == eRpNoError) {
				if (theCompletionStatus == eSfsComplete) {
					theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
				}
			}
			else {
				/*
					we got an error writing the file.
				*/
				if (theResult == eRpFatalFileSystemError) {
					/*
						if this is a fatal error, try to close the file
						and abort the TCP connection.
					*/
					SfsCloseFile(theConnectionPtr->fIndex, False);
					RpConnectionAbortTcp(theConnectionPtr);
				}
				else {
					/*
						Try to handle this file system error as non-fatal.
						Save the error away in the connection block, to be
						handled at a higher level and return eRpNoError.
					*/
					theConnectionPtr->fFileSystemError = theResult;
					theResult = eRpNoError;
					theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
				}
			}
#endif	/* RomPagerFileUpload || RomPagerPutMethod */

			break;

		default:
			break;

	}
	return theResult;
}


RpErrorCode RpFileClose(rpConnectionPtr theConnectionPtr) {
	RpErrorCode 		theResult;

	theResult = SfsCloseFile(theConnectionPtr->fIndex, True);
	if (theResult == eRpNoError) {
		theConnectionPtr->fState = eRpConnectionWaitingFileClose;
	}
	else {
		theConnectionPtr->fFileState = eRpFileClosed;
		theResult = RpConnectionAbortTcp(theConnectionPtr);
	}
	return theResult;
}


#if RomPagerFileUpload || RomPagerPutMethod
/*
	This is the connection error callback routine for the HTTP Server
	protocol with file upload support.

	A file that is open for read (being served as an HTTP object)
	will be closed with theCompleteFlag set to True.

	A file that has been created (for file upload), will be closed
	with theCompleteFlag set to False.
*/

static RpErrorCode ConnErrorCloseFile(rpConnectionPtr theConnectionPtr) {
	Boolean				theNeedCloseFlag;
	int					theCompleteFlag;
	rpHttpRequestPtr	theRequestPtr;
	RpErrorCode			theResult;

	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	theResult = eRpNoError;
	theNeedCloseFlag = True;

	if (theConnectionPtr->fFileState != eRpFileClosed) {
		if (theConnectionPtr->fFileState == eRpFileCreated ||
			theConnectionPtr->fFileState == eRpFileWriting) {
			/*
				We created the file.  Tell the close routine that it's
				not complete and set the Multipart or PUT State to handle 
				the connection error.
			*/
			theCompleteFlag = False;
#if RomPagerFileUpload
			theRequestPtr->fMultipartState = eRpMpConnectionError;
#endif
#if RomPagerPutMethod
			theRequestPtr->fHttpPutState = eRpHttpPutConnectionError;
#endif
		}
		else {
			/*
				We must be serving this file.  Tell the close routine
				that it's complete.
			*/
			theCompleteFlag = True;
		}

		theResult = SfsCloseFile(theConnectionPtr->fIndex, theCompleteFlag);

		if (theResult == eRpNoError) {
			theConnectionPtr->fState = eRpConnectionWaitingFileClose;
			/*
				The connection will be closed later, after the file
				close completes.
			*/
			theNeedCloseFlag = False;
		}
		else {
			/*
				We got an error closing the file.  Mark it
				as closed anyway.
			*/
			theConnectionPtr->fFileState = eRpFileClosed;
		}
	}

	if (theNeedCloseFlag) {
		/*
			Either the file's already closed or we got an error trying
			to close it.  Either way, our clean up is done.
			Close the connection.
		*/
		(void) RpConnectionCheckTcpClose(theConnectionPtr);
	}

	return eRpNoError;
}

#else	/* !RomPagerFileUpload */

/*
	This is the connection error callback routine for the HTTP Server
	protocol without file upload support.

	A file that is open for read (being served as an HTTP object)
	will be closed with theCompleteFlag set to True.
*/

static RpErrorCode ConnErrorCloseFile(rpConnectionPtr theConnectionPtr) {
	Boolean				theNeedCloseFlag;
	RpErrorCode			theResult;

	theNeedCloseFlag = True;

	if (theConnectionPtr->fFileState != eRpFileClosed) {
		theResult = SfsCloseFile(theConnectionPtr->fIndex, True);

		if (theResult == eRpNoError) {
			theConnectionPtr->fState = eRpConnectionWaitingFileClose;
			/*
				The connection will be closed later, after the file
				close completes.
			*/
			theNeedCloseFlag = False;
		}
		else {
			/*
				We got an error closing the file.  Mark it
				as closed anyway.
			*/
			theConnectionPtr->fFileState = eRpFileClosed;
		}
	}

	if (theNeedCloseFlag) {
		/*
			Either the file's already closed or we got an error trying
			to close it.  Either way, our clean up is done.
			Close the connection.
		*/
		(void) RpConnectionCheckTcpClose(theConnectionPtr);
	}

	return eRpNoError;
}

#endif	/* RomPagerFileUpload */


#if RpFileInsertItem

/*
	Send out the text for an error to the page for a File Insert Item and set 
	the states to continue processing items (possibly closing the file first).
*/

void RpSendFileInsertItemError(rpConnectionPtr theConnectionPtr, 
		rpHttpTransactionState theHttpTransactionState) {
	rpHttpRequestPtr	theRequestPtr;

	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	RpSendDataOutZeroTerminated(theRequestPtr, C_S_FSErrorDetected);
	RpSendDataOutZeroTerminated(theRequestPtr,
			theRequestPtr->fDataPtr->fBoxNameText);
	theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
	theRequestPtr->fHttpTransactionState = theHttpTransactionState;
	return;
}
#endif

#endif	/* (RomPagerServer || RomPagerBasic) && RomPagerFileSystem */
