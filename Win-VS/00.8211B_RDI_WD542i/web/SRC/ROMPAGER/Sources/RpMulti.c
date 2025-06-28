/*
 *	File:		RpMulti.c
 *
 *	Contains:	RomPager routines for handling Multipart Data.
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
 * * * * Release 4.21  * * *
 *		03/12/03	bva		move file upload specific code from AsParse.c
 * * * * Release 4.20  * * *
 * * * * Release 4.00  * * *
 *		02/14/01	rhb		rename eRpItemError_* to eAsItemError_*
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		09/21/00	pjr		eRpHttpFileSystemError -> eRpHttpInternalServerError
 * * * * Release 3.10  * * *
 *		06/06/00	pjr		add check for item value length
 *		06/02/00	bva		change prototypes for RpQuery.c routines
 *		05/26/00	bva		use gRpDataPtr
 *		05/02/00	pjr		fix sending a direct response object
 *		02/09/00	bva		use AsEngine.h
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.06  * * *
 * * * * Release 3.03  * * *
 *		06/10/99	pjr		terminate properly when the last item processed
 *							before the final boundary is file data
 * * * * Release 3.02  * * *
 *		04/30/99	bva		add RpCheckQuery call to RpSetupFileUploadRequest
 * * * * Release 3.0 * * * *
 *		12/31/98	pjr		use RP_MEMCMP
 *		12/08/98	rhb		eliminate warnings	
 * * * * Release 2.2 * * * *
 *		12/01/98	bva		ignore empty file
 *		11/12/98	rhb		give a type to fDataPtr in the request structure
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		10/20/98	bva		move state initialization to RpSetupFileUploadRequest
 *		09/25/98	bva		fix rpParsingControl changes
 *		09/21/98	pjr		move file related fields to the connection block
 *		09/01/98	bva		change rpParsingControl structure
 *		08/31/98	bva		change <CR><LF> definitions
 *		08/31/98	pjr		handle object not found in RpSetupFileUploadRequest
 *							add RpGetFileUploadStatus routine
 *		08/20/98	pjr		rework connection error handling
 *		08/13/98	bva		add theCompleteFlag to SfsCloseFile call
 * * * * Release 2.1 * * * *
 *		02/27/98	pjr		no longer accept a boundary that is missing the
 *							2 leading hyphens
 *		02/16/98	bva		use RpFindItemFromName instead of RpFindFormItemType
 *		01/27/98	pjr		fix final boundary detection when the file is the
 *							last item processed
 *		01/19/98	bva		eRpConnectionNeedsHttpAction ->
 *								eRpConnectionNeedsProtocolAction
 * * * * Release 2.0 * * * *
 *		11/03/97	pjr		parsing changes (use rpParsingControl)
 *		09/12/97	pjr		change some comments.
 *		08/22/97	bva		add RpConvertHeaderToLowerCase call
 *		08/12/97	pjr		add RpCheckAccess call and get the next page to be
 *							served from the object extension.
 *		07/12/97	bva		fFormRequestLength -> fPostRequestLength
 *		07/07/97	pjr		rework ScanBufferForBoundary for use with begin
 *							boundaries as well as end boundaries.
 *		07/02/97	pjr		add code to flush the remainder of the incoming
 *							data on an error.
 *		07/02/97	pjr		improve error handling.  flush buffers on error.
 *		07/01/97	pjr		fix bugs in ScanBufferForBoundary
 *		06/28/97	bva		modified ScanBufferForBoundary
 *		06/26/97	pjr		initial version
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"

#if RomPagerFileUpload

/*
	ScanBufferForBoundary results
*/
typedef enum {
	eFoundBoundary,
	eBoundaryNotFound,
	ePartialBoundary
} rpScanResult;


/*
	CheckForFinalBoundary results
*/
typedef enum {
	eIntermediateBoundary,
	eFinalBoundary,
	eBoundaryError
} rpBoundaryType;

static void HandleMultipartDataDone(rpHttpRequestPtr theRequestPtr);

static rpScanResult ScanBufferForBoundary(rpHttpRequestPtr theRequestPtr,
						char * theDataPtr,
						Signed32 theLength,
						Signed32 * theDataLengthPtr,
						Signed32 * theBoundaryLengthPtr);

static rpBoundaryType CheckForFinalBoundary(char * theDataPtr);


/*
	File Upload Status

	The RpGetFileUploadStatus returns True if a File Upload has been
	completed successfully, otherwise it returns False.
*/

Boolean RpGetFileUploadStatus(void * theTaskDataPtr) {

	return((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fFileDone;
}


void RpSetupFileUploadRequest(rpHttpRequestPtr theRequestPtr) {
	rpObjectFlags			theObjectFlags;
	rpObjectDescriptionPtr	theObjectPtr;
	rpParsingControlPtr		theParsingPtr;

	theParsingPtr = theRequestPtr->fParsingControlPtr;

	/*
		set up the global form request length.  this is the total
		length, which may include multiple buffers.
	*/
	theParsingPtr->fHttpObjectLengthToRead = theRequestPtr->fPostRequestLength;

	/*
		If the URL for this request has been set up to expect 
		multipart form data, then we'll take it.  Otherwise, 
		we flush the data and send back '403 Forbidden'.
	*/
#if RomPagerQueryIndex
	RpCheckQuery(theRequestPtr);
#endif	/* RomPagerQueryIndex */
	RpSearchRomObjectList(theRequestPtr);
	theObjectPtr = theRequestPtr->fObjectPtr;

	if (theObjectPtr == (rpObjectDescriptionPtr) 0) {
		/*
			We didn't find the URL.  Set up the states to send the
			"object not found" response.  Mark the connection to close
			after we send the response which will flush any data.
		*/
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
		theRequestPtr->fHttpResponseState = eRpHttpNoObjectFound;
		theRequestPtr->fObjectSource = eRpRomUrl;
		theRequestPtr->fObjectPtr = &gRpHttpNoObjectFoundPage;
		theRequestPtr->fPersistent = False;
	}
	else if (theObjectPtr->fMimeDataType == eRpDataTypeFormMultipart) {
		/*
			We have valid access to a form.  Forms process the data and then 
			trigger a request to serve up a page. As a default, we set up to 
			serve the page stored in our object descriptor. In some cases, 
			the forms processing may change the page to be served.  
		*/
		theRequestPtr->fCurrentFormDescriptionPtr = theObjectPtr;
		theRequestPtr->fObjectPtr = theObjectPtr->fExtensionPtr->fPagePtr;

		/*
			Get the flags from the form object.
		*/
		theObjectFlags = theObjectPtr->fExtensionPtr->fFlags;

		if (!(theObjectFlags & kRpObjFlag_Direct)) {
			/*
				Set the state so we will respond with a redirect to the
				next page to be served.
			*/ 
			theRequestPtr->fHttpResponseState = eRpHttpRedirect;
		}
		/*
			Set up the multipart states, then
			go check the URL and security.
		*/
		theRequestPtr->fObjectSource = eRpFileUploadUrl;
		theRequestPtr->fHttpTransactionState = eRpEndUrlSearch;
		theRequestPtr->fFileDone = False;
		theRequestPtr->fFinalBoundary = False;
		theRequestPtr->fMultipartState = eRpMpFindBoundary;
	}
	else {
		/*
			The object type is not multipart.  Set up the states to send
			the "unexpected multipart" page.  Mark the connection to close
			after we send the response which will flush any data.
		*/
		theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
		theRequestPtr->fHttpResponseState = eRpHttpForbidden;
		theRequestPtr->fObjectSource = eRpRomUrl;
		theRequestPtr->fObjectPtr = &gRpUnexpectedMultipart;
		theRequestPtr->fPersistent = False;
	}

	return;
}


Boolean RpProcessMultipart(rpHttpRequestPtr theRequestPtr) {
	rpConnectionPtr 		theConnectionPtr;
	char *					theCurrentDataPtr;
	rpDataPtr				theDataPtr;
	rpParsingControlPtr		theParsingPtr;
	Signed32				theBoundaryLength;
	Signed32				theDataLength;
	Signed32				theTempLength;
	RpErrorCode				theResult;
	rpScanResult			theScanResult;
	rpLineState				theLineState;
	rpItemPtr				theItemPtr;
	rpBoundaryType			theBoundaryType;
	Boolean					theReadMoreFlag;

	theDataPtr = theRequestPtr->fDataPtr;
	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	theParsingPtr = theRequestPtr->fParsingControlPtr;
	theReadMoreFlag = False;

/*
	theRequestPtr->fIncomingBufferLength - length remaining in this buffer
	theRequestPtr->fHttpObjectLengthToRead - total length of HTTP request -
			could be multiple buffers - length of total request
			(multipart headers and file content)
	theRequestPtr->fPostRequestLength - length from the request (Content-Length)
			same as fHttpObjectLengthToRead in the beginning.
			gets decremented as data is processed.
*/

	/*
		things that we want to get us out of this loop:
			1. theRequestPtr->fIncomingBufferLength has gone to 0
			   and we are either done, or we need to get more data.
			2. a file system operation has been started and we must wait
			   for it to complete before we can continue.
	*/

	do {

		switch (theRequestPtr->fMultipartState) {

			case eRpMpFindBoundary:
				/*
					get a complete line
				*/
				theLineState = RpParseReplyBuffer(theParsingPtr);

				switch (theLineState) {

					case eRpLineComplete:
						if (theParsingPtr->fHttpObjectLengthToRead > 0) {
							theParsingPtr->fHttpObjectLengthToRead -= 
									theParsingPtr->fCompleteLineLength;
						}
						theScanResult = eBoundaryNotFound;
						theCurrentDataPtr = 
								theParsingPtr->fCurrentBeginLinePtr;
						if (theParsingPtr->fCompleteLineLength >= 
								(theRequestPtr->fBoundaryLength + 2)) {
							theScanResult = eBoundaryNotFound;

							/*
								At this point we should be at the two hyphens 
								that begin the boundary.  If we are not at a 
								boundary, this is a bad request.
							*/
							if ((*theCurrentDataPtr == kAscii_Hyphen) &&
								(*(theCurrentDataPtr + 1) == kAscii_Hyphen)) {

								if (RP_MEMCMP((theCurrentDataPtr + 2),
										theRequestPtr->fBoundary,
									theRequestPtr->fBoundaryLength) == 0) {
									/*
										found the boundary.  
									*/
									theScanResult = eFoundBoundary;
									theCurrentDataPtr += 
											(theRequestPtr->fBoundaryLength + 2);
								}
							}
						}

						if (theScanResult == eFoundBoundary) {
							/*
								a boundary was found.  see if it's the 
								final boundary.
							*/
							theBoundaryType = 
									CheckForFinalBoundary(theCurrentDataPtr);

							switch (theBoundaryType) {

								case eIntermediateBoundary:
									/*
										this is an intermediate boundary.
										it should be followed by 
										multipart headers.
									*/
									theRequestPtr->fMultipartState = 
											eRpMpParsingHeaders;
									break;

								case eFinalBoundary:
									/*
										we got the final boundary so we're 
										done processing multipart data.

										set the lengths to 0.  some browsers 
										send extra CR, LF pairs after the final 
										boundary.  this makes the lengths equal 
										to 2 or 4, even though we're really done.
									*/
									theParsingPtr->fHttpObjectLengthToRead = 0;
									theParsingPtr->fIncomingBufferLength = 0;

									HandleMultipartDataDone(theRequestPtr);
									break;

								case eBoundaryError:
								default:
									/*
										got some kind of error parsing the 
										boundary.  bad request.
									*/
									theRequestPtr->fMultipartState = 
											eRpMpBadRequest;
									break;
							}
						}
						else {
							/*
								no boundary was found.  bad request.
							*/
							theRequestPtr->fMultipartState = eRpMpBadRequest;
						}

						break;


					case eRpLinePartial:
						theReadMoreFlag = True;
						break;


					case eRpLineError:
						/*
							We got a line error.  This is a bad request.
						*/
						theRequestPtr->fMultipartState = eRpMpBadRequest;
						break;


					case eRpLineEmpty:
						/*
							We got an empty line.  Normally, we would not allow
							this while looking for the boundary.  It has been
							discovered that some browsers insert an extra 
							<CR><LF> before the boundary.  So, we'll just 
							ignore the empty line.
						*/
						break;

				}

				break;


			case eRpMpParsingHeaders:

				while ((theReadMoreFlag == False) &&
						(theRequestPtr->fMultipartState == eRpMpParsingHeaders)) {

					theLineState = RpParseReplyBuffer(theParsingPtr);

					switch (theLineState) {

						case eRpLineComplete:
							/*
								we got a complete line.
							*/
							if (theParsingPtr->fHttpObjectLengthToRead > 0) {
								theParsingPtr->fHttpObjectLengthToRead -= 
										theParsingPtr->fCompleteLineLength;
							}
							RpConvertHeaderToLowerCase(theParsingPtr);
							RpParseHeader(theRequestPtr, 
									theDataPtr->fMpPatternTable);
							break;

						case eRpLinePartial:
							theReadMoreFlag = True;
							break;

						case eRpLineError:
							theRequestPtr->fMultipartState = eRpMpBadRequest;
							break;

						case eRpLineEmpty:
							/*	
								We have an empty line (CRLF or LF), so we are 
								done parsing the multipart headers.  Make sure 
								there was an item name in the multipart headers.
							*/
							if (*theRequestPtr->fItemName != '\0') {
								theItemPtr = RpFindItemFromName(
										theRequestPtr->fCurrentFormDescriptionPtr->
												fItemsArrayPtr, 
										theRequestPtr->fItemName, 
										theRequestPtr->fItemName);
								if (theItemPtr->fType == eRpItemType_FormFile) {
									/*
										the item type is file.  
										set up to create the file.
									*/
									theRequestPtr->fMultipartState = 
											eRpMpCreateFile;
								}
								else {
									/*
										the item type is not file.  
										set up to get the item value.
									*/
									theRequestPtr->fMultipartState = 
											eRpMpParsingItemValue;
								}
							}
							else {
								/*
									We're done parsing the multipart headers 
									but we don't have an item name.
								*/
								theRequestPtr->fMultipartState = 
										eRpMpBadRequest;
							}

							break;

					}	/* switch (theLineState) */

				}	/*  while ((theReadMoreFlag == False) &&
						(theRequestPtr->fMultipartState == eRpMpParsingHeaders)) */

				break;


			case eRpMpParsingItemValue:
				while ((theReadMoreFlag == False) &&
						(theRequestPtr->fMultipartState == eRpMpParsingItemValue)) {

					theLineState = RpParseReplyBuffer(theParsingPtr);

					switch (theLineState) {

						case eRpLineComplete:
							/*
								we have a complete line.  get the item value.
							*/
							if (theParsingPtr->fHttpObjectLengthToRead > 0) {
								theParsingPtr->fHttpObjectLengthToRead -= 
										theParsingPtr->fCompleteLineLength;
							}
							theTempLength = RpFindLineEnd(
											theParsingPtr->fCurrentBeginLinePtr);
							*(theParsingPtr->fCurrentBeginLinePtr + 
											theTempLength) = '\0';

							if (theTempLength < kMaxValueLength) {
								/*
									copy the item value and process the item.
								*/
								theRequestPtr->fItemError =
										eAsItemError_NoError;
								RP_STRCPY(theRequestPtr->fItemValue,
										theParsingPtr->fCurrentBeginLinePtr);
								RpReceiveItem(theDataPtr,
										theRequestPtr->fItemName,
										theRequestPtr->fItemValue);
							}
							else {
								/*
									the item value is too long.
								*/
								theRequestPtr->fItemError =
										eAsItemError_TooManyCharacters;
							}

							if (theRequestPtr->fItemError == eAsItemError_NoError) {
								/*
									processed the item okay.  there should be
									another boundary after the item value.
								*/
								theRequestPtr->fMultipartState = eRpMpFindBoundary;
							}
							else {
								/*
									we got an error processing the item.  
									bad request.
								*/
								theRequestPtr->fMultipartState = eRpMpBadRequest;
							}
							break;

						case eRpLinePartial:
							theReadMoreFlag = True;
							break;

						case eRpLineError:
						case eRpLineEmpty:
							/*	
								we have a line error or an empty line while parsing
								the item value.  bad request.
							*/
							theRequestPtr->fMultipartState = eRpMpBadRequest;
							break;
					}	/* switch (theLineState) */
				}	/* while ((theReadMoreFlag == False) &&
					(theRequestPtr->fMultipartState == eRpMpParsingItemValue)) */

				break;


			case eRpMpCreateFile:
				if (*theRequestPtr->fFilename != '\0') {
					/*
						it looks like we have a filename, start the file create.
					*/
					theResult = SfsCreateFile(theConnectionPtr->fIndex,
												theRequestPtr->fFilename,
												theRequestPtr->fFileInfoPtr);

					if (theResult == eRpNoError) {
						/*
							we didn't get an error on the file create call.
							change the state to wait for the file create
							to finish.
						*/
						theConnectionPtr->fState = eRpConnectionWaitingFileCreate;
						theRequestPtr->fMultipartState = eRpMpCreateFileDone;
					}
					else {
						/*
							we got an error creating the file.
						*/
						theConnectionPtr->fFileSystemError = theResult;
						theRequestPtr->fMultipartState = eRpMpFileSystemError;
					}
				}
				else {
					/*
						We don't have a filename, so just pretend we've
						already uploaded the file.
					*/
					theRequestPtr->fMultipartState = eRpMpFindBoundary;
				}

				break;


			case eRpMpCreateFileDone:
				if ((theConnectionPtr->fFileState == eRpFileCreated) &&
					(theConnectionPtr->fFileSystemError == eRpNoError)) {
					/*
						the file was create was successfull.
					*/
					theRequestPtr->fMultipartState = eRpMpParsingData;
				}
				else {
					/*
						must have gotten an error creating the file.
					*/
					theRequestPtr->fMultipartState = eRpMpFileSystemError;
				}

				break;


			case eRpMpParsingData:
				/*
					If the length is still 0, we must not have 
					received more data yet.
				*/
				if (theParsingPtr->fIncomingBufferLength == 0) {
					theReadMoreFlag = True;
					break;
				}

				if (theRequestPtr->fMultipartRemainderLength > 0) {
					/*
						we have a remainder from last time.  copy the data from
						the remainder and TCP buffers into the work buffer.
					*/
					RP_MEMCPY(theRequestPtr->fMultipartWorkBuffer,			/* to */
							theRequestPtr->fMultipartRemainderBuffer,		/* from */
							theRequestPtr->fMultipartRemainderLength);		/* size */

					RP_MEMCPY((theRequestPtr->fMultipartWorkBuffer +
							theRequestPtr->fMultipartRemainderLength),		/* to */
							theParsingPtr->fCurrentBufferPtr,				/* from */
							theParsingPtr->fIncomingBufferLength);			/* size */

					/*
						adjust the lengths and pointers
					*/
					theParsingPtr->fCurrentBufferPtr = 
							theRequestPtr->fMultipartWorkBuffer;
					theParsingPtr->fHttpObjectLengthToRead += 
							theRequestPtr->fMultipartRemainderLength;
					theParsingPtr->fIncomingBufferLength += 
							theRequestPtr->fMultipartRemainderLength;

					theRequestPtr->fMultipartRemainderLength = 0;
					theCurrentDataPtr = theRequestPtr->fMultipartWorkBuffer;
				}
				else {
					theCurrentDataPtr = theParsingPtr->fCurrentBufferPtr;
				}

				/*
					ScanBufferForBoundary will set theDataLength to the amount
					of data that can be written to the file now.
				*/
				theScanResult = ScanBufferForBoundary(theRequestPtr,
									theCurrentDataPtr,
									theParsingPtr->fIncomingBufferLength,
									&theDataLength,
									&theBoundaryLength);

				/*
					adjust the lengths and pointers by the amount
					processed by ScanBufferForBoundary.
				*/
				theTempLength = theDataLength + theBoundaryLength;
				theParsingPtr->fHttpObjectLengthToRead -= theTempLength;
				theParsingPtr->fIncomingBufferLength -= theTempLength;
				theParsingPtr->fCurrentBufferPtr += theTempLength;

				if (theScanResult == eFoundBoundary) {
					/*
						a boundary was found, we are done with the file.
					*/
					theRequestPtr->fFileDone = True;
				}
				else {
					/*
						either no boundary was found, or a partial 
						boundary was found.
					*/
					if (theScanResult == ePartialBoundary) {
						/*
							a partial boundary was found.  we have to save 
							the data that may be a boundary for future processing.
						*/
						theRequestPtr->fMultipartRemainderLength = 
								theBoundaryLength;

						RP_MEMCPY(theRequestPtr->fMultipartRemainderBuffer,
								(theCurrentDataPtr + theDataLength),
								theRequestPtr->fMultipartRemainderLength);
					}
				}

				if (theDataLength > 0) {
					/*
						write the data.
					*/
					theConnectionPtr->fFileState = eRpFileWriting;
					theResult = SfsWriteFile(theConnectionPtr->fIndex, 
												theCurrentDataPtr, theDataLength);

					if (theResult == eRpNoError) {
						theConnectionPtr->fState = eRpConnectionWaitingFileWrite;
						/*
							the next time RpProcessMultipart is entered, the
							file write should be done.
						*/
						theRequestPtr->fMultipartState = eRpMpWriteFileDone;
					}
					else {
						/*
							SfsWriteFile returned an error.  set up to let
							eRpMpWriteFileDone handle the error.

							preserve the first file system error.
						*/
						if (theConnectionPtr->fFileSystemError == eRpNoError) {
							theConnectionPtr->fFileSystemError = theResult;
						}
						theRequestPtr->fMultipartState = eRpMpWriteFileDone;
					}
				}
				else {	/* nothing to write */
					theRequestPtr->fMultipartState = eRpMpWriteFileDone;
				}

				break;


			case eRpMpWriteFileDone:
				if (theConnectionPtr->fFileSystemError == eRpNoError) {
					/*
						a file write has finished.
					*/
					if (theRequestPtr->fFileDone == True) {
						/*
							we have received all of the data.
							set up to close the file.
						*/
						theRequestPtr->fMultipartState = eRpMpCloseFile;
					}
					else {
						/*
							we have not received all of the data.
							set up to get more data.
						*/
						theRequestPtr->fMultipartState = eRpMpParsingData;
						theReadMoreFlag = True;
					}
				}
				else {
					/*
						there has been a file system error.
						set up to close the file.
					*/
					theRequestPtr->fMultipartState = eRpMpCloseFile;
				}

				break;


			case eRpMpCloseFile:
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
					theRequestPtr->fMultipartState = eRpMpCloseFileDone;
				}
				else {
					/*
						we got an error closing the file!
						preserve the first file system error.
					*/
					if (theConnectionPtr->fFileSystemError == eRpNoError) {
						theConnectionPtr->fFileSystemError = theResult;
					}
					theRequestPtr->fMultipartState = eRpMpFileSystemError;
				}

				break;


			case eRpMpCloseFileDone:
				if (theConnectionPtr->fFileSystemError == eRpNoError) {
					/*
						there has not been a file system error.
					*/
					if (theRequestPtr->fFinalBoundary == True) {
						/*
							a final boundary has been found so we're 
							done processing multipart data.

							set the lengths to 0.  some browsers 
							send extra CR, LF pairs after the final 
							boundary.  this makes the lengths equal 
							to 2 or 4, even though we're really done.
						*/
						theParsingPtr->fHttpObjectLengthToRead = 0;
						theParsingPtr->fIncomingBufferLength = 0;

						HandleMultipartDataDone(theRequestPtr);
					}
					else {
						/*
							we have not found a final boundary.
							continue parsing multipart data.
						*/
						theRequestPtr->fMultipartState = eRpMpParsingHeaders;
					}
				}
				else {
					/*
						there has been a file system error.
					*/
					theRequestPtr->fMultipartState = eRpMpFileSystemError;
				}

				break;


			case eRpMpBadRequest:
				theRequestPtr->fHttpResponseState = eRpHttpBadRequest;
				theRequestPtr->fMultipartState = eRpMpFlush;
				break;


			case eRpMpFileSystemError:
				theRequestPtr->fHttpResponseState = eRpHttpInternalServerError;
				theRequestPtr->fObjectPtr = &gRpFileSystemErrorPage;
				theRequestPtr->fMultipartState = eRpMpFlush;
				break;


			case eRpMpConnectionError:
				/*
					We've had a connection error.  If a file was open,
					it should already be closed.
				*/
				(void) RpConnectionCheckTcpClose(theConnectionPtr);
				break;


			case eRpMpFlush:
				/*
					throw away this data.  some browsers send in more data than
					is needed to satisfy the form length, so prevent the form
					length from going negative.
				*/
				if (theParsingPtr->fIncomingBufferLength > 
						theParsingPtr->fHttpObjectLengthToRead) {
					theParsingPtr->fHttpObjectLengthToRead = 0;
				}
				else {
					theParsingPtr->fHttpObjectLengthToRead -= 
							theParsingPtr->fIncomingBufferLength;
				}

				theParsingPtr->fIncomingBufferLength = 0;

				if (theParsingPtr->fHttpObjectLengthToRead > 0) {
					/*
						we have not received all of the data.
						set up to get more.
					*/
					theReadMoreFlag = True;
				}
				else {
					/*
						we have received all of the data.
						set up to start the response.
					*/
					theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
				}

				break;

		}	/* switch (theRequestPtr->fMultipartState) */

	} while ((theConnectionPtr->fState == eRpConnectionNeedsProtocolAction) &&
				(theParsingPtr->fIncomingBufferLength > 0));

	return theReadMoreFlag;
}


static void HandleMultipartDataDone(rpHttpRequestPtr theRequestPtr) {
	rpObjectExtensionPtr	theExtensionPtr;

	/*
		The file upload process is complete, so do any optional
		form post-processing and send a normal response.
		
		If there was an empty file, 
		then theRequestPtr->fFileDone = False, and the Web 
		application can detect it.
	*/
	theExtensionPtr = theRequestPtr->fCurrentFormDescriptionPtr->fExtensionPtr;
	if (theExtensionPtr != (rpObjectExtensionPtr) 0 && 
		theExtensionPtr->fProcessDataFuncPtr != (rpProcessDataFuncPtr) 0) {
		theExtensionPtr->fProcessDataFuncPtr(theRequestPtr->fDataPtr,
				theRequestPtr->fDataPtr->fCurrentConnectionPtr->fIndexValues);
	}

	/*
		Set up the states to begin sending the response.
		If there is a response object (rather than a redirect),
		it will come from ROM.
	*/
	theRequestPtr->fObjectSource = eRpRomUrl;
	theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;
	return;
}


/*
		This routine will detect a boundary in the data stream.

		Normally, a boundary begins with CR, LF, Hyphen, Hyphen.
		There is an exception to this.  Some browsers send the
		boundary string right after the CR, LF, omitting the
		two leading hyphens.  This routine will accept a
		boundary that is missing the two leading hyphens as
		long as the boundary string itself begins with at least
		two hyphens.  At this point, all boundaries we have seen
		begin with at least two hyphens (usually about 27).
		Here is a sample boundary:
			----------------------------168071508944249

		A boundary followed by HYPHEN, HYPHEN, CR, LF, is the final
		boundary.

		A boundary followed by CR, LF (without the two hyphens), is an
		intermediate boundary, and more form data should follow.

		<CR><LF><--><BOUNDARY><CR><LF>			intermediate boundary
		<CR><LF><--><BOUNDARY><--><CR><LF>		final boundary

		Return values:

		theRequestPtr->fFinalBoundary:
			This is a boolean that is set to True if a final boundary
			was found.

		theScanResult:
			This is an enum which indicates whether a complete boundary,
			partial boundary, or no boundary was found.

		theDataLength:
			The length of data before the beginning of the boundary.  This
			does NOT include any of the boundary's leading characters
			(CR, LF, HYPHEN, HYPHEN).

		theBoundaryLength:
			The length of data consumed by the boundary and it's surrounding
			qualification characters.  This includes all boundary related
			leading and trailing CR's, LF's, and HYPHEN's.
*/

static rpScanResult ScanBufferForBoundary(rpHttpRequestPtr theRequestPtr,
						char * theDataPtr,
						Signed32 theLength,
						Signed32 * theDataLengthPtr,
						Signed32 * theBoundaryLengthPtr) {

	char *			theStartDataPtr = theDataPtr;
	char *			theStartBoundaryPtr = theDataPtr;
	Boolean			theNeedMoreDataFlag = False;
	Boolean			theTestCharacterAgainFlag = False;
	Unsigned8		theScanState;
	rpScanResult	theScanResult;
	Signed32		theStartLength = theLength;
	rpBoundaryType	theBoundaryType;

	*theBoundaryLengthPtr = 0;
	theScanState = 0;
	theScanResult = eBoundaryNotFound;

	/*
		scan the buffer for an end boundary.  the end boundary must be
		preceded by a '<CR><LF><HYPHEN><HYPHEN>' sequence.
	*/
	while (theScanResult != eFoundBoundary && theLength > 0 &&
			theNeedMoreDataFlag == False) {
		switch (theScanState) {

			case 0:		
				/*	
					we have no match  
				*/
				if (*theDataPtr == '\x0d') {
					theScanState++;
					theScanResult = ePartialBoundary;
					theStartBoundaryPtr = theDataPtr;
				}
				break;
			
			case 1:		
				/*	
					we've matched '<CR>'  
				*/
				if (*theDataPtr == '\x0a') {
					theScanState++;
				}
				else {
					theScanState = 0;
					theScanResult = eBoundaryNotFound;
					theTestCharacterAgainFlag = True;
				}
				break;

			case 2:		
				/*	
					we've matched '<CR><LF>'  
				*/
				if (*theDataPtr == '-') {
					theScanState++;
				}
				else {
					theScanState = 0;
					theScanResult = eBoundaryNotFound;
					theTestCharacterAgainFlag = True;
				}
				break;

			case 3:		
				/*	
					we've matched '<CR><LF><HYPHEN>'  
				*/
				if (*theDataPtr == '-') {
					theScanState++;
				}
				else {
					theScanState = 0;
					theScanResult = eBoundaryNotFound;
					theTestCharacterAgainFlag = True;
				}
				break;

			case 4:		
				/*	
					we've matched '<CR><LF><HYPHEN><HYPHEN>',  
					so look for the rest of the boundary.
				*/
				if (theLength >= (theRequestPtr->fBoundaryLength + 4)) {
					/*
						there is enough data left in the buffer that
						there could be an entire boundary.
 						CR, LF, HYPHEN, HYPHEN, BOUNDARY, HYPHEN, HYPHEN, CR, LF
						(theLength has already been decremented for the leading
						CR, LF, HYPHEN, HYPHEN).
					*/
					if (RP_MEMCMP(theDataPtr, theRequestPtr->fBoundary,
						theRequestPtr->fBoundaryLength) == 0) {
						/*
								found the boundary.  
						*/
						theScanResult = eFoundBoundary;

						/* CR, LF, Hyphen, Hyphen, Boundary, CR, LF */
						*theBoundaryLengthPtr = (4 + theRequestPtr->fBoundaryLength + 2);
					}
				}
				else {
					theNeedMoreDataFlag = True;
				}

				if (theScanResult == eFoundBoundary) {
					/*
						see if it's the final boundary
					*/
					theBoundaryType = CheckForFinalBoundary(theDataPtr + 
							theRequestPtr->fBoundaryLength);

					if (theBoundaryType == eFinalBoundary) {
						theRequestPtr->fFinalBoundary = True;
						*theBoundaryLengthPtr += 2;
						theDataPtr += 3;
						theLength -= 3;
					}
					else {
						theRequestPtr->fFinalBoundary = False;
						theDataPtr++;
						theLength--;
					}
				}
				else {
					if (theNeedMoreDataFlag == False) {
						/*
							we had found the first four characters, but the
							rest doesn't match, so start looking some more.
						*/
						theScanState = 0;
						theScanResult = eBoundaryNotFound;
						theTestCharacterAgainFlag = True;
					}
				}

				break;

		}	/* switch (theScanState) */


		if (theTestCharacterAgainFlag == True) {
			theTestCharacterAgainFlag = False;
		}
		else {
			theDataPtr++;
			theLength--;
		}

	}	/* while (theScanResult != eFoundBoundary && theLength > 0 &&
					theNeedMoreDataFlag == False)	*/


	/*
		calculate the length of the data to the beginning of the boundary string.
	*/
	if (theScanResult == eBoundaryNotFound) {
		*theDataLengthPtr = theStartLength;
	}
	else {
		*theDataLengthPtr = theStartBoundaryPtr - theStartDataPtr;
	}

	if (theScanResult == ePartialBoundary) {
		*theBoundaryLengthPtr = (theStartLength - *theDataLengthPtr);
	}

	return theScanResult;
}


static rpBoundaryType CheckForFinalBoundary(char * theDataPtr) {

	/*
		if the boundary is followed by a new line,
		it's an intermediate boundary
	*/
	if ((*theDataPtr == '\x0d') && (*(theDataPtr + 1) == '\x0a')) {
		return eIntermediateBoundary;
	}

	/*
		if the boundary is followed by 2 hyphens and a new line,
		it's the final boundary
	*/
	if ((*theDataPtr == kAscii_Hyphen) && (*(theDataPtr + 1) == kAscii_Hyphen) &&
		(*(theDataPtr + 2) == '\x0d') && (*(theDataPtr + 3) == '\x0a')) {
		return eFinalBoundary;
	}

	/*
		if the boundary is not terminated by either of the character
		sequences listed above, return an error.
	*/
	return eBoundaryError;
}

#endif	/* RomPagerFileUpload */
