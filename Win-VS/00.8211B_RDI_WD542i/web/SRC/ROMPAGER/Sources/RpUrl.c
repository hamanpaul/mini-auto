/*
 *	File:		RpUrl.c
 *
 *	Contains:	URL search routines for RomPager
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	?1995-2003 by Allegro Software Development Corporation
 *	All rights reserved.
 *
 *	This module contains confidential, unpublished, proprietary
 *	source code of Allegro Software Development Corporation.
 *
 *	The copyright notice above does not evidence any actual or intended
 *	publication of such source code.
 *
 *	License is granted for specific uses only under separate
 *	written license by Allegro Software Development Corporation.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 * * * * Release 4.30  * * *
 *		09/03/03	pjr		copy the URL to fErrorPath before modifying it
 *		08/07/03	bva		add support for Range requests 
 *		07/30/03	bva		fix prefix stripping for RpUnknownUrlsAreRemote 
 *      06/20/03    bva     eRpSoapUrl -> eRpUpnpSoapUrl
 * * * * Release 4.21  * * *
 *		05/07/03	amp		add RpRootObjectIsUserExit
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 * * * * Release 4.20  * * *
 * * * * Release 4.11  * * *
 *      07/25/02    bva     add SOAP processing for UserExit routines
 * * * * Release 4.10  * * *
 *		06/08/02	bva		remove RpUserExitMulti
 * * * * Release 4.07  * * *
 * * * * Release 4.00  * * *
 *		06/29/01	amp		support RomUpnpControl in RpIdentifyObjectSource
 *		06/29/01	amp		add support for SOAP requests
 *		05/23/01	rhb		add fIsTlsFlag to rpCgi
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		06/23/00	pjr		add support for a file system root object
 *		06/02/00	bva		change prototypes for RpQuery.c routines
 *		05/26/00	bva		use gRpDataPtr
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.06  * * *
 * * * * Release 3.05  * * *
 *		09/21/99	rhb		remove URL State checking from RpFindHttpObject()
 * * * * Release 3.04  * * *
 * * * * Release 3.0 * * * *
 *		03/16/99	pjr		rework debug code
 *		12/31/98	pjr		use RP_MEMCMP
 *		12/24/98	pjr		created, moved routines from other sources
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if RomPagerServer

void RpFindHttpObject(rpConnectionPtr theConnectionPtr) {
	rpHttpRequestPtr 		theRequestPtr;

	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	RpIdentifyObjectSource(theRequestPtr);

#if RomPagerQueryIndex || RomPagerImageMapping
#if RomPagerRemoteHost
	if (theRequestPtr->fObjectSource != eRpRemoteUrl) {
		RpCheckQuery(theRequestPtr);
	}
#else	/* !RomPagerRemoteHost */
	RpCheckQuery(theRequestPtr);
#endif	/* RomPagerRemoteHost */
#endif	/* RomPagerQueryIndex || RomPagerImageMapping */

	theRequestPtr->fHttpTransactionState = eRpEndUrlSearch;
	theRequestPtr->fObjectPtr = (rpObjectDescriptionPtr) 0;
	RpFindUrl(theConnectionPtr);
	return;
}


void RpFindUrl(rpConnectionPtr theConnectionPtr) {
	char *					thePathPtr;
	rpHttpRequestPtr 		theRequestPtr;
#if RomPagerFileSystem
	RpErrorCode				theResult;
#endif
#if RomPagerUserExit
	rpCgiPtr				theCgiPtr;
#endif

	theRequestPtr = theConnectionPtr->fHttpRequestPtr;
	thePathPtr = theRequestPtr->fPath;

#if RpHttpFlowDebug
	RP_PRINTF("\nPath: %s\n", thePathPtr);
#endif

#if RpRootObjectIsFile
	/*
		If the request is for the root object, change the object
		source to the file system and set the root filename.
	*/
	if (RP_STRCMP(thePathPtr, kRootPath) == 0) {
		theRequestPtr->fObjectSource = eRpFileUrl;
		RP_STRCPY(theRequestPtr->fPath, kRpRootFilename);
	}
#endif

#if RpRootObjectIsUserExit
	/*
		If the request is for the root object, change the object
		source to user exit.
	*/
	if (RP_STRCMP(thePathPtr, kRootPath) == 0) {
		theRequestPtr->fObjectSource = eRpUserUrl;
	}
#endif

	switch (theRequestPtr->fObjectSource) {

#if RomPagerFileSystem
		case eRpFileUrl:
			theResult = SfsOpenFile(theConnectionPtr->fIndex, 
									theRequestPtr->fPath);
			if (theResult == eRpNoError) {
				theConnectionPtr->fState = eRpConnectionWaitingFileOpen;
			}
			break;
#endif
						
#if RomPlugAdvanced
		case eRpUpnpSoapUrl:
			/*
				Set up to proocess a SOAP request.
			*/
			RpProcessXmlRequest(theConnectionPtr);
			break;
#endif
						
#if RomPlugControl
		case eRpGenaUrl:
			/*
				Process a GENA event notification.
			*/
			CpProcessGenaEvent(theRequestPtr);
			break;
#endif
						
#if RomPagerRemoteHost
		case eRpRemoteUrl:
			/*
				We need to get a Remote Host Request Block
				for this connection.
			*/
			theConnectionPtr->fState = eRpConnectionNeedsRemoteRequestBlock;
			break;
#endif
			
#if RomPagerUserExit
		case eRpUserUrl:
			/*
				Set up the CGI structure.
			*/
			theCgiPtr = &theRequestPtr->fCgi;
			theCgiPtr->fConnectionId = theConnectionPtr->fIndex;
			theCgiPtr->fHttpRequest = 
					(rpCgiHttpRequest) theRequestPtr->fHttpCommand;
			theCgiPtr->fPathPtr = theRequestPtr->fPath;
			theCgiPtr->fHostPtr = theRequestPtr->fHost;
			theCgiPtr->fRefererPtr = theRequestPtr->fRefererUrl;
#if RomPagerCaptureUserAgent
			theCgiPtr->fAgentPtr = theRequestPtr->fAgent;
#else
			theCgiPtr->fAgentPtr = kCRLF;
#endif
#if RomPagerCaptureLanguage
			theCgiPtr->fLanguagePtr = theRequestPtr->fLanguage;
#else
			theCgiPtr->fLanguagePtr = kCRLF;
#endif
#if RomPagerCaptureSoapAction
			theCgiPtr->fSoapActionPtr = theRequestPtr->fSoapAction;
#endif
#if RomPagerSecurity
			theCgiPtr->fUserNamePtr = theRequestPtr->fUsername;
			theCgiPtr->fPasswordPtr = theRequestPtr->fPassword;
#else
			theCgiPtr->fUserNamePtr = kCRLF;
			theCgiPtr->fPasswordPtr = kCRLF;
#endif
			theCgiPtr->fBrowserDate = theRequestPtr->fBrowserDate;
			theCgiPtr->fArgumentBufferPtr = theRequestPtr->fHttpWorkBuffer;
			theCgiPtr->fArgumentBufferLength = 
					theRequestPtr->fPostRequestLength;
			theCgiPtr->fResponseBufferLength = 0;
			theCgiPtr->fDataType = eRpDataTypeHtml;
#if RomPagerSecure
			theCgiPtr->fIsTlsFlag = theConnectionPtr->fIsTlsFlag;
#endif

			/*
				Set up default response states to
				protect against badly written User Exit 
				routines.
			*/
			theCgiPtr->fResponseState = eRpCgiLastBuffer;
			theCgiPtr->fHttpResponse = eRpCgiHttpNotFound;

			/*
				Call the User Exit routine.
			*/
			RpExternalCgi(theRequestPtr->fDataPtr, theCgiPtr);
			if (theCgiPtr->fResponseState == eRpCgiPending) {
				theConnectionPtr->fState = eRpConnectionWaitingUserExit;
			}
			else {
				RpHandleCgiResponse(theRequestPtr);
			}
			break;
#endif
			
		case eRpRomUrl:
		default:
			if (RP_STRCMP(thePathPtr, gRpItemErrorPage.fURL) == 0) {
				theRequestPtr->fObjectPtr = &gRpItemErrorPage;
			}
#if RpTcnTest
			else if (RP_STRCMP(thePathPtr, gRpAllegroCopyrightPage.fURL) == 0) {
				theRequestPtr->fObjectPtr = &gRpListTest;
				theRequestPtr->fHttpResponseState = eRpHttpMultipleChoices;
			}
#else
			else if (RP_STRCMP(thePathPtr, gRpAllegroCopyrightPage.fURL) == 0) {
				theRequestPtr->fObjectPtr = &gRpAllegroCopyrightPage;
			}
#endif
			else {
				RpSearchRomObjectList(theRequestPtr);
#if RpUnknownUrlsAreFiles || RpUnknownUrlsAreCgi || RpUnknownUrlsAreRemote
				if (theRequestPtr->fObjectPtr == (rpObjectDescriptionPtr) 0 &&
						RP_STRCMP(thePathPtr, kRootPath) != 0) {
					/*
						We didn't find the object in the ROM object list
						and it's not the root object.  Look elsewhere.
					*/
#if RpUnknownUrlsAreFiles
					theRequestPtr->fObjectSource = eRpFileUrl;
#endif
#if RpUnknownUrlsAreRemote
					theRequestPtr->fObjectSource = eRpRemoteUrl;
#if RpRemoteHostMulti
					theRequestPtr->fRemoteHostIndex = 0;
#endif
#endif
#if RpUnknownUrlsAreCgi
					theRequestPtr->fObjectSource = eRpUserUrl;
#endif
					RpFindUrl(theConnectionPtr);
				}
#endif /* RpUnknownUrlsAreFiles || RpUnknownUrlsAreCgi || RpUnknownUrlsAreRemote */
			}
			break;
	}
	return; 	
}


/*
	RpIdentifyObjectSource

	URLs represent objects that may be stored as ROM objects, File System 
	objects, Remote Objects on other Web servers, or objects handled by 
	User exits.  Objects other than ROM objects are identified by a leading 
	pathname identifier, so that the URL takes the form "/xx/yyyyy" where 
	"xx" is the identifier and "yyyyy" is the rest of the URL.  The "xx" 
	strings are defined in RpConfig.h. The default values are "FS" for 
	File System objects, "RH" for Remote Host objects, 
	"UD" for UPnP Simple Object Access Protocol requests,
	and "UE" for User Exit objects.
	
	If multiple Remote Hosts are supported, the "xx" token is of the form 
	"RHnnn" where "RH" is the Remote Host prefix and "nnn" is the index to
	which Remote Host to address the request.

	If the General Event Notification Architecture (GENA) is supported, 
	all objects sent with a NOTIFY method are GENA objects regardless of URL.
*/

void RpIdentifyObjectSource(rpHttpRequestPtr theRequestPtr) {
#if	RomPagerFileSystem || RomPagerUserExit || RomPagerRemoteHost \
		|| RomPlugAdvanced || RomPlugControl
	char *			thePathPtr;
	Unsigned16		theTokenLength;
#endif
#if	RpRemoteHostMulti
	char			theIndexString[4];
	rpDataPtr		theDataPtr = theRequestPtr->fDataPtr;
#endif

	/*
		We may need the original URL for a 404 Not Found response,
		so copy it before we start modifying it.
	*/
	RP_STRCPY(theRequestPtr->fErrorPath, theRequestPtr->fPath);

#if	RomPlugControl
	/*
		GENA events are always delivered with a NOTIFY method.
		The subscription ID will be used to associate the object
		with a specific subscription. 
	*/
	if (theRequestPtr->fHttpCommand == eRpHttpNotifyCommand) {
		theRequestPtr->fObjectSource = eRpGenaUrl;
		return;
	}
#endif

	theRequestPtr->fObjectSource = eRpRomUrl;
#if	RomPagerFileSystem || RomPagerUserExit || RomPagerRemoteHost \
		|| RomPlugAdvanced || RomPlugControl
	thePathPtr = theRequestPtr->fPath;
	/*
		Position past the first slash.
	*/
	thePathPtr++;
	theTokenLength = RpFindTokenDelimited(thePathPtr, kAscii_Slash);
	if (*(thePathPtr + theTokenLength) == '\0') {
		/*
			No other slashes, so return.
		*/			
		return;
	}
#if	RomPagerFileSystem
	if (RP_MEMCMP(thePathPtr, kFileSystemPrefix, theTokenLength) == 0) {
		theRequestPtr->fObjectSource = eRpFileUrl;
		/*
			Strip the file system prefix from the URL.
		*/
		thePathPtr += theTokenLength;
		RP_STRCPY(theRequestPtr->fPath, thePathPtr);
		return;
	}
#endif
	
#if	RomPlugAdvanced
	if (RP_MEMCMP(thePathPtr, kUpnpDevicePrefix, theTokenLength) == 0) {
		theRequestPtr->fObjectSource = eRpUpnpSoapUrl;
		return;
	}
#endif
	
#if	RomPagerRemoteHost
#if	RpRemoteHostMulti
// +++ _Alphanetworks_Patch_, 10/29/2003, jacob_shih
#if 0
	if (RP_MEMCMP(thePathPtr, kRemoteHostPrefix, theTokenLength - 3) == 0) {
		theRequestPtr->fObjectSource = eRpRemoteUrl;
		thePathPtr += theTokenLength - 3;
		theIndexString[0] = *thePathPtr++;
		theIndexString[1] = *thePathPtr++;
		theIndexString[2] = *thePathPtr++;
		theIndexString[3] = '\0';
		theRequestPtr->fRemoteHostIndex = 
			RpConvertStringToUnsigned16(theDataPtr, theIndexString);
		/*
			Strip the remote host prefix from the URL.
		*/
		RP_STRCPY(theRequestPtr->fPath, thePathPtr);
		return;
	}
#else
	if (RP_MEMCMP(thePathPtr, kRemoteHostPrefix, RP_STRLEN(kRemoteHostPrefix)) == 0) {
		Unsigned16		theHostIndex;
		thePathPtr += RP_STRLEN(kRemoteHostPrefix);
		theIndexString[0] = *thePathPtr++;
		theIndexString[1] = *thePathPtr++;
		theIndexString[2] = *thePathPtr++;
		theIndexString[3] = '\0';
		theHostIndex= RpConvertStringToUnsigned16(theDataPtr, theIndexString);
		if(alphaRpIsRemoteHostValid(theDataPtr, theHostIndex))
		{
			theRequestPtr->fRemoteHostIndex = theHostIndex;
			theRequestPtr->fObjectSource = eRpRemoteUrl;
			/*
				Strip the remote host prefix from the URL.
			*/
			RP_STRCPY(theRequestPtr->fPath, thePathPtr);
			return;
		}
		else
		{
		}
	}
#endif
// --- _Alphanetworks_Patch_, 10/29/2003, jacob_shih
#else
	if (RP_MEMCMP(thePathPtr, kRemoteHostPrefix, theTokenLength) == 0) {
		theRequestPtr->fObjectSource = eRpRemoteUrl;
		/*
			Strip the remote host prefix from the URL.
		*/
		thePathPtr += theTokenLength;
		RP_STRCPY(theRequestPtr->fPath, thePathPtr);
		return;
	}
#endif
#endif	/* RomPagerRemoteHost */

#if	RomPagerUserExit
	if (RP_MEMCMP(thePathPtr, kUserExitPrefix, theTokenLength) == 0) {
		theRequestPtr->fObjectSource = eRpUserUrl;
		return;
	}
#endif	/* RomPagerUserExit */

#endif	/* RomPagerFileSystem || RomPagerUserExit || RomPagerRemoteHost */

	return;
}


void RpSearchRomObjectList(rpHttpRequestPtr theRequestPtr) {
	Unsigned16				theIndex;
	Unsigned16				theListIndex;
	rpObjectDescriptionPtr	theObjectPtr;
	rpObjectDescPtrPtr		theListPtr;
	rpObjectDescPtrPtr *	theListPtrPtr;
	char *					thePathPtr;

	theListPtrPtr = theRequestPtr->fDataPtr->fMasterObjectListPtr;
	if (theListPtrPtr != (rpObjectDescPtrPtr *) 0) {
		theListIndex = 0;
		theListPtr = theListPtrPtr[theListIndex++];
		thePathPtr = theRequestPtr->fPath;

// +++ _Alphanetworks_Patch_, 10/29/2003, jacob_shih
		if(strstr(thePathPtr, "/Forms/"))
		{
			thePathPtr= strstr(thePathPtr, "/Forms/");
		}
		else if(strstr(thePathPtr, "/Images/"))
		{
			thePathPtr= strstr(thePathPtr, "/Images/");
		}
// --- _Alphanetworks_Patch_, 10/29/2003, jacob_shih
		if (theListPtr != (rpObjectDescPtrPtr) 0) {
			/*
				We've got a pointer to the first object list.
				If the page being requested is the root page,
				return the first object in the list.
			*/
			if (RP_STRCMP(thePathPtr, kRootPath) == 0) {
				theRequestPtr->fObjectPtr = theListPtr[0];
				return;
			}
		}

		while (theListPtr != (rpObjectDescPtrPtr) 0) {

			theIndex = 0;
			theObjectPtr = theListPtr[theIndex++];
	
			while (theObjectPtr != (rpObjectDescriptionPtr) 0) {
				/*
						See if this is the object we're looking for.
				*/
				if (RP_STRCMP(thePathPtr, theObjectPtr->fURL) == 0) {
					theRequestPtr->fObjectPtr = theObjectPtr;
					return;
				}
				else {
					theObjectPtr = theListPtr[theIndex++];
				}
			}
			theListPtr = theListPtrPtr[theListIndex++];
		}
	}

	return;
}


void RpFinishUrlSearch(rpHttpRequestPtr theRequestPtr) {

	theRequestPtr->fHttpTransactionState = eRpSendingHttpHeaders;

	if (theRequestPtr->fObjectPtr == (rpObjectDescriptionPtr) 0) {
		/*
			We didn't find the URL, so notify the user.
		*/
		theRequestPtr->fObjectPtr = &gRpHttpNoObjectFoundPage;
		theRequestPtr->fHttpResponseState = eRpHttpNoObjectFound;
		theRequestPtr->fObjectSource = eRpRomUrl;
	}
	else {
		switch (theRequestPtr->fObjectSource) {			

#if RomPagerRemoteHost
			case eRpRemoteUrl:
				break;
#endif

#if RomPagerUserExit
			case eRpUserUrl:
				/*
					We handled the URL, so send the headers,
					and then send back the response buffer.
				*/
				break;
#endif

#if RomPagerRanges
			case eRpFileUrl:
				/*
					If we got a Range request, we need to check 
					the validity of the range.
				*/
				if (theRequestPtr->fHaveRangeRequest) {
					RpCheckRange(theRequestPtr);
				}
				break;
#endif

			case eRpRomUrl:
			default:
				theRequestPtr->fHttpTransactionState = eRpEndSecurityCheck;
#if RomPagerSecurity
				theRequestPtr->fPasswordState =
						RpCheckAccess(theRequestPtr->fDataPtr,
						theRequestPtr->fObjectPtr->fObjectAccess);
#endif	
				break;
		}
	}
	return;
}

#endif	/* RomPagerServer */
