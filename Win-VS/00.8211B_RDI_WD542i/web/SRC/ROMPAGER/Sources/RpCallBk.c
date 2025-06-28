/*
 *	File:		RpCallBk.c
 *
 *	Contains:	Callback entry points for the RomPager Server.
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
 *		07/24/03	bva		bump connection size for delayed function routines
 *		07/13/03	bva		move redirection routines from RpForm.c
 *							add RpSetRedirectAbsolute
 * * * * Release 4.21  * * *
 * * * * Release 4.03  * * *
 *		10/30/01	bva		move RpSetConnectionClose from RpHttp.c
 *		09/14/01	rhb		add RpSetConnectionCloseFunction
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		02/09/00	bva		use AsEngine.h
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 * * * * Release 3.06  * * *
 * * * * Release 3.01  * * *
 *		04/13/99	bva		add RpGetCurrentConnection
 * * * * Release 3.0 * * * *
 *		02/27/99	bva		add HTTP Cookie support
 *		01/31/99	bva		add RpSetRequestUserPhraseDictionary
 *		12/31/98	pjr		created from routines in RomPager.c (AsMain.c)
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if RomPagerServer

void RpSetUserPhraseDictionary(void *theTaskDataPtr, 
								char **theUserDictionaryPtr, 
								Boolean theCompressionFlag) {
	rpDataPtr		theDataPtr;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theDataPtr->fUserPhrases = theUserDictionaryPtr;
	theDataPtr->fUserPhrasesCanBeCompressed = theCompressionFlag;
	return;
}

void RpSetRequestUserPhraseDictionary(void *theTaskDataPtr, 
								char **theUserDictionaryPtr, 
								Boolean theCompressionFlag) {
	rpHttpRequestPtr 	theRequestPtr;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fUserPhrases = theUserDictionaryPtr;
	theRequestPtr->fUserPhrasesCanBeCompressed = theCompressionFlag;
	return;
}


void RpSetRomObjectList(void *theTaskDataPtr, 
					rpObjectDescPtrPtr *theMasterObjectListPtr) {
					
	rpDataPtr		theDataPtr;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theDataPtr->fMasterObjectListPtr = theMasterObjectListPtr;
	return;
}


void RpSetRequestCloseFunction(void *theTaskDataPtr, 
									rpProcessCloseFuncPtr theFunctionPtr) {
	rpHttpRequestPtr 	theRequestPtr;
	
	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	if (theRequestPtr->fInUse) {
		theRequestPtr->fProcessCloseFuncPtr = theFunctionPtr;
#if RomPagerKeepAlive || RomPagerHttpOneDotOne
		/*
			since we've requested a close function, we need to
			set the connection so it actually closes.
		*/
		theRequestPtr->fPersistent = False;
#endif
	}
	else {
		theRequestPtr->fProcessCloseFuncPtr = (rpProcessCloseFuncPtr) 0;
	}
	return;
}

void RpSetConnectionCloseFunction(void *theTaskDataPtr, 
		rpConnCloseFuncPtr theFunctionPtr, void *theCookie) {
	rpConnectionPtr 	theConnectionPtr;
	
	theConnectionPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentConnectionPtr;
	theConnectionPtr->fCloseFuncPtr = theFunctionPtr;
	theConnectionPtr->fCloseCookie = theCookie;
	return;
}

void RpSetConnectionClose(void *theTaskDataPtr) {
#if RomPagerKeepAlive || RomPagerHttpOneDotOne

	((rpDataPtr) theTaskDataPtr)->
			fCurrentHttpRequestPtr->fPersistent = False;
#endif	 
	return;
}

#if RomPagerDelayedFunctions

/*
	Callback routines to start and stop delayed function pending.
	
	The RpInitiateDelayedFunction should only be called from the
	optional initial or post page processing functions (fProcessDataFuncPtr 
	in the rpObjectExtension structure).  This function is used to
	take a connection out of processing state while something is 
	happening behind RomPager's back to set up some data.  This
	way, the engine won't block and is freed to handle other processing.
	
	The RpCompleteDelayedFunction should be called by the completion
	handling of whatever function was being waited for.  It will
	put a given connection back in service to process a page.
*/

Unsigned16 RpInitiateDelayedFunction(void *theTaskDataPtr) {
	rpConnectionPtr	theConnectionPtr;
	rpDataPtr		theDataPtr = (rpDataPtr) theTaskDataPtr;
	Unsigned16		theConnection;
							
	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	theConnection = theDataPtr->fCurrentConnectionPtr - 
							theDataPtr->fConnections;
	theConnectionPtr->fState = eRpConnectionHolding;
	return theConnection;
}


extern void RpCompleteDelayedFunction(void *theTaskDataPtr, 
						Unsigned16 theConnection) {
	rpConnectionPtr	theConnectionPtr;
	rpDataPtr		theDataPtr = (rpDataPtr) theTaskDataPtr;
							
	theConnectionPtr = &theDataPtr->fConnections[theConnection];
	if (theConnectionPtr->fState == eRpConnectionHolding) {
		theConnectionPtr->fState = eRpConnectionNeedsProtocolAction;
		/* 
			The HTTP task will now resume at whatever 
			fHttpTransactionState it was suspended.  Usually 
			this is eRpSendingHttpHeaders.
		*/
	}
	return;
}


Unsigned16 RpGetCurrentConnection(void *theTaskDataPtr) {
	rpConnectionPtr	theConnectionPtr;
	rpDataPtr		theDataPtr = (rpDataPtr) theTaskDataPtr;
	Unsigned16		theConnection;
							
	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	theConnection = theDataPtr->fCurrentConnectionPtr - 
							theDataPtr->fConnections;
	return theConnection;
}


Unsigned16 RpSetCurrentConnection(void *theTaskDataPtr, 
						Unsigned16 theConnection) {
	rpConnectionPtr	theConnectionPtr;
	rpDataPtr		theDataPtr = (rpDataPtr) theTaskDataPtr;
	Unsigned16		theOldConnection;

	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	theOldConnection = theDataPtr->fCurrentConnectionPtr - 
							theDataPtr->fConnections;
	theDataPtr->fCurrentConnectionPtr = 
			&theDataPtr->fConnections[theConnection];
	theDataPtr->fCurrentHttpRequestPtr = 
			theDataPtr->fCurrentConnectionPtr->fHttpRequestPtr;
	return theOldConnection;
}

#endif	/* RomPagerDelayedFunctions */


#if RpUserCookies

void * RpGetCookie(void *theTaskDataPtr) {
	return ((rpDataPtr) theTaskDataPtr)->fUserCookie;
}


void RpSetCookie(void *theTaskDataPtr, void *theCookie) {
	((rpDataPtr) theTaskDataPtr)->fUserCookie = theCookie;
	return;
}


void * RpGetRequestCookie(void *theTaskDataPtr) {
	rpHttpRequestPtr 	theRequestPtr;
	
	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	if (theRequestPtr != (rpHttpRequestPtr) 0 && theRequestPtr->fInUse) {
		return theRequestPtr->fUserCookie;
	}
	else {
		return (void *) 0;
	}
}


void RpSetRequestCookie(void *theTaskDataPtr, void *theCookie) {
	rpHttpRequestPtr 	theRequestPtr;
	
	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	if (theRequestPtr != (rpHttpRequestPtr) 0) { 	
		if (theRequestPtr->fInUse) {
			theRequestPtr->fUserCookie = theCookie;
		}
		else {
			theRequestPtr->fUserCookie = (void *) 0;
		}
	}
	return;
}

#endif	/* RpUserCookies */

#if RomPagerHttpCookies

char * RpGetHttpCookie(void *theTaskDataPtr, 
							Unsigned8 theIndex) {
	rpHttpRequestPtr 	theRequestPtr;
	
	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	return theRequestPtr->fHttpCookies[theIndex];
}

void RpSetHttpCookie(void *theTaskDataPtr, 
							Unsigned8 theIndex, 
							char * theCookie) {
	rpHttpRequestPtr 	theRequestPtr;
	
	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fNewHttpCookie[theIndex] = True;
	RpStrLenCpy(theRequestPtr->fHttpCookies[theIndex], 
			theCookie, kHttpCookieSize);
	return;
}

#endif	/* RomPagerHttpCookies */

#if RomPagerFileSystem || RomPagerUserExit
void RpSetNextFilePage(void *theTaskDataPtr,
		char  *theNextPagePtr) {
	rpHttpRequestPtr		theRequestPtr;
	rpObjectDescriptionPtr	theObjectPtr;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theObjectPtr = &theRequestPtr->fLocalObject;
	theObjectPtr->fURL = theNextPagePtr;
	theRequestPtr->fObjectPtr = theObjectPtr;
	return;
}

#endif

void RpSetNextPage(void *theTaskDataPtr,
		rpObjectDescriptionPtr theNextPagePtr) {
	((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fObjectPtr =
			theNextPagePtr;

	return;
}

void RpSetRedirect(void *theTaskDataPtr) {
	((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fHttpResponseState =
			eRpHttpRedirect;

	return;
}

void RpSetRedirectAbsolute(void 	*theTaskDataPtr,
							Boolean	theUseSslFlag,
							char 	*theHostNamePtr,
							char	*thePathPtr) {
	rpObjectDescriptionPtr	theObjectPtr;
	rpHttpRequestPtr		theRequestPtr;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fHttpResponseState = eRpHttpRedirect;
	theObjectPtr = &theRequestPtr->fLocalObject;
	theObjectPtr->fURL = thePathPtr;
	theRequestPtr->fObjectPtr = theObjectPtr;
	RpStrLenCpyTruncate(theRequestPtr->fHost, theHostNamePtr, kMaxSaveHeaderLength);
	if (theUseSslFlag) {
		theRequestPtr->fRedirectScheme = eRpSchemeHttps;
	}
	else {
		theRequestPtr->fRedirectScheme = eRpSchemeHttp;
	}
	return;
}

char * RpGetCurrentUrl(void *theTaskDataPtr) {
	return ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fObjectPtr->fURL;
}


#endif	/* RomPagerServer */
