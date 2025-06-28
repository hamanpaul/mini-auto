/*
 *	File:		RpHttpDy.c
 *
 *	Contains:	RomPager routines for dynamic HTTP
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
 * * * * Release 4.10  * * *
 *		06/08/02	bva		remove RomPagerSlaveIdentity
 * * * * Release 4.07  * * *
 * * * * Release 4.00  * * *
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		10/27/00	pjr		move index values to the connection structure
 *		07/25/00	rhb		Support SSL/TLS
 *		05/26/00	bva		use gRpDataPtr
 *		02/09/00	bva		use AsEngine.h
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.06  * * *
 * * * * Release 3.05  * * *
 *		09/19/99	bva		kHttpSeparator -> kFieldSeparator
 * * * * Release 3.04  * * *
 * * * * Release 3.02  * * *
 *		04/29/99	pjr		fix RomPagerSlaveIdentity compile error
 * * * * Release 3.0 * * * *
 *		12/28/98	pjr		created from RpHtml.c, RpHttp.c, and RpHttpPRq.c
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if RomPagerClientPull

void RpBuildRefresh(rpHttpRequestPtr theRequestPtr, char *theResponsePtr) {
	Unsigned32				theHttpNumber;
#if RomPagerSecure
	rpDataPtr				theDataPtr = theRequestPtr->fDataPtr;
#endif

#if RomPagerServerPush
	if (theRequestPtr->fServerPushActive) {
		/*
			If we're in server push mode the fRefreshSeconds and
			fRefreshObjectPtr are used there, so don't set up
			refresh headers.
		*/
		return;
	}
#endif
	theHttpNumber = theRequestPtr->fRefreshSeconds;
	if (theHttpNumber > 0) {
		RP_STRCAT(theResponsePtr, kHttpRefresh);
		RpCatUnsigned32ToString(theHttpNumber, theResponsePtr, 0);
		if (theRequestPtr->fRefreshObjectPtr !=
					(rpObjectDescriptionPtr) 0) {
			RP_STRCAT(theResponsePtr, kFieldSeparator);
			RP_STRCAT(theResponsePtr, kHttpURL);
#if RomPagerSecure
			if (theDataPtr->fCurrentConnectionPtr->fIsTlsFlag) {
				RP_STRCAT(theResponsePtr, kHttpsString);
			}
			else
#endif
			{
				RP_STRCAT(theResponsePtr, kHttpString);
			}
			RpBuildHostName(theRequestPtr, theResponsePtr);
#if RomPagerUrlState
			if (theRequestPtr->fUrlState[0] != '\0') {
				RP_STRCAT(theResponsePtr, kUrlStatePrefix);
				RP_STRCAT(theResponsePtr, theRequestPtr->fUrlState);
			}
#endif
			RP_STRCAT(theResponsePtr, theRequestPtr->fRefreshObjectPtr->fURL);
		}
		RP_STRCAT(theResponsePtr, kCRLF);
	}
	return;
}


void RpSetRefreshTime(void *theTaskDataPtr, Unsigned16 theRefreshSeconds) {

	((rpDataPtr) theTaskDataPtr)->
			fCurrentHttpRequestPtr->fRefreshSeconds = theRefreshSeconds;
	return;
}


void RpSetRefreshPage(void *theTaskDataPtr,
						rpObjectDescriptionPtr theRefreshPagePtr) {

	((rpDataPtr) theTaskDataPtr)->
			fCurrentHttpRequestPtr->fRefreshObjectPtr = theRefreshPagePtr;
	return;
}

#endif	/* RomPagerClientPull */


#if RomPagerServerPush

RpErrorCode RpHandleServerPushWait(rpConnectionPtr theConnectionPtr) {
	rpDataPtr				theDataPtr;
	rpObjectExtensionPtr	theExtensionPtr;
	rpObjectDescriptionPtr	theObjectPtr;
	rpHttpRequestPtr		theRequestPtr;
	RpErrorCode				theResult;

	/*
		a second has gone by....
	*/
	theResult = eRpNoError;
	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	theDataPtr = theRequestPtr->fDataPtr;
	if (theRequestPtr->fServerPushSeconds > 0) {
		/*
			decrement the server push time.
		*/
		theRequestPtr->fServerPushSeconds -= 1;
		/*
			Every second that the timer has not expired, call the
			user exit routine.  This way the user can check internal
			states and set up the next object to be served.
		*/
		RpUserServerPushExit(theDataPtr);
		/*
			Go start the connection timer to come back here again.
		*/
		theRequestPtr->fHttpTransactionState = eRpServerPushStartTimer;
	}
	else {
		/*
			The timer has expired, see what to do next.
		*/
		theObjectPtr = theRequestPtr->fServerPushObjectPtr;
		if (theObjectPtr == (rpObjectDescriptionPtr) 0) {
			/*
				There is no next object to serve, so close the connection
			*/
			theResult = RpConnectionCloseTcp(theConnectionPtr);
		}
		else {
			/*
				set up the structure links for the lower levels
			*/
			theDataPtr->fCurrentHttpRequestPtr =
					theConnectionPtr->fHttpRequestPtr;
			theDataPtr->fCurrentConnectionPtr = theConnectionPtr;
			/*
				Set up the object to serve after this one
			*/
			theExtensionPtr = theObjectPtr->fExtensionPtr;
			if (theExtensionPtr != (rpObjectExtensionPtr) 0) {
				theRequestPtr->fServerPushObjectPtr =
						theExtensionPtr->fRefreshPagePtr;
				theRequestPtr->fServerPushSeconds =
						theExtensionPtr->fRefreshSeconds;
				if (theExtensionPtr->fProcessDataFuncPtr !=
						(rpProcessDataFuncPtr) 0) {
					/*
						Do any initial page processing.  The initial
						page processing can issue callbacks to change
						the next server push parameters.
					*/
					theExtensionPtr->fProcessDataFuncPtr(theDataPtr,
							theConnectionPtr->fIndexValues);
				}
			}
			else {
				theRequestPtr->fServerPushObjectPtr =
						(rpObjectDescriptionPtr) 0;
				theRequestPtr->fServerPushSeconds = 0;
			}
			/*
				Set up the next object for server push, and
				start serving it.
			*/
			theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
			theRequestPtr->fHttpTransactionState = eRpSendingHttpResponse;
			theRequestPtr->fObjectPtr = theObjectPtr;
			RpInitializeHtmlPageReply(theRequestPtr);
			theResult = RpBuildReply(theConnectionPtr);
		}
	}
	return theResult;
}


void RpSendServerPushHeaders(rpHttpRequestPtr theRequestPtr) {
	Unsigned32				theContentLength;
	rpObjectDescriptionPtr	theObjectPtr;
	rpDataType 				theMimeType;

	/*
		Put out the headers for this part of the data
	*/
	theObjectPtr = theRequestPtr->fObjectPtr;
	theMimeType = theObjectPtr->fMimeDataType;
	RpSendDataOutZeroTerminated(theRequestPtr, kHttpContentType);
	RpSendDataOutZeroTerminated(theRequestPtr, gMimeTypes[theMimeType]);
	RpSendDataOutZeroTerminated(theRequestPtr, kCRLF);
	theContentLength = theObjectPtr->fLength;
	if (theContentLength > 0) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHttpContentLength);
		RpSendDataOutDecimalUnsigned(theRequestPtr, theContentLength);
		RpSendDataOutZeroTerminated(theRequestPtr, kCRLF);
	}
	RpSendDataOutZeroTerminated(theRequestPtr, kCRLF);
	return;
}


void RpSendServerPushSeparator(rpHttpRequestPtr theRequestPtr) {

	RpSendDataOutZeroTerminated(theRequestPtr, kCRLF);
	RpSendDataOutZeroTerminated(theRequestPtr, kTwoDashes);
	RpSendDataOutZeroTerminated(theRequestPtr, theRequestPtr->fSeparator);
	/*
		If this is the last item of the server push chain,
		we need to send out the two dashes again.
		Since we close the connection after the last item,
		it doesn't really matter.
	*/
	if (!theRequestPtr->fServerPushActive) {
		RpSendDataOutZeroTerminated(theRequestPtr, kTwoDashes);
	}
	RpSendDataOutZeroTerminated(theRequestPtr, kCRLF);
	return;
}


void RpSetServerPushTime(void *theTaskDataPtr,
						Unsigned16 theServerPushSeconds) {

	((rpDataPtr) theTaskDataPtr)->
			fCurrentHttpRequestPtr->fServerPushSeconds = theServerPushSeconds;
	return;
}


void RpSetServerPushPage(void *theTaskDataPtr,
						rpObjectDescriptionPtr theServerPushPagePtr) {

	((rpDataPtr) theTaskDataPtr)->
			fCurrentHttpRequestPtr->fServerPushObjectPtr = theServerPushPagePtr;
	return;
}

#endif	/* RomPagerServerPush */
