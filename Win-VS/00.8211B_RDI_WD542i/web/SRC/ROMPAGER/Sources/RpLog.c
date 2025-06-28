/*
 *	File:		RpLog.c
 *
 *	Contains:	RomPager HTTP event logging routines.
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
 * * * * Release 4.00  * * *
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		09/21/00	pjr		eRpHttpFileSystemError -> eRpHttpInternalServerError
 *		02/16/00	bva		fixed compiler warning
 *		02/09/00	bva		use AsEngine.h
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 *		01/05/00	bva		fixed compiler warning
 * * * * Release 3.06  * * *
 * * * * Release 3.05  * * *
 *		09/06/99	bva		add support for addition object types
 * * * * Release 3.04  * * *
 * * * * Release 3.0 * * * *
 *		01/24/99	pjr		only enable for RomPagerServer
 * * * * Release 2.2 * * * *
 *		11/17/98	pjr		adjust log item time by server start up time
 *		11/09/98	bva		use macro abstraction for stdlib calls
 * * * * Release 2.1 * * * *
 *		02/13/98	rhb		change parameter to RpCatUnsigned32ToString
 *		01/26/98	pjr		change index usage in RpBuildHttpEventStrings.
 *							index 0 is the oldest item.
 *		01/06/98	pjr		initial version.
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if RomPagerServer && RomPagerLogging

static void		RpBuildTimeString(char *theDateString,
									Unsigned32 theDateSeconds);


void RpLoggingInit(rpDataPtr theDataPtr) {

	theDataPtr->fLogIndex = 0;
	theDataPtr->fLogItemCount = 0;
	theDataPtr->fStartUpTime = RpGetSysTimeInSeconds(theDataPtr);

	return;
}


void RpLogHttpEvent(rpDataPtr theDataPtr,
					rpHttpResponseAction theEventType,
					rpObjectSource theObjectSource,
					void * theEventInfoPtr) {
	rpLogItemPtr		theLogItemPtr;
	Unsigned16			theIndex;

	/*
		Log the item at the current index.
	*/
	theIndex = theDataPtr->fLogIndex;
	theLogItemPtr = &theDataPtr->fLogBuffer[theIndex];

	theLogItemPtr->fEventTime =
			(RpGetSysTimeInSeconds(theDataPtr) - theDataPtr->fStartUpTime);
	theLogItemPtr->fEventType = theEventType;
	theLogItemPtr->fObjectSource = theObjectSource;
	theLogItemPtr->fEventInfoPtr = theEventInfoPtr;

	/*
		Set the index to 0 if it's at the last item,
		otherwise increment it.
	*/
	if (theIndex == (kRpLogEvents -1)) {
		theIndex = 0;
	}
	else {
		theIndex++;
	}

	/*
		Save the new index.
	*/
	theDataPtr->fLogIndex = theIndex;

	/*
		Increment the log item count, if it's not already
		at the max.
	*/
	if (theDataPtr->fLogItemCount < kRpLogEvents) {
		theDataPtr->fLogItemCount++;
	}

	return;
}


/*
	Event Type Strings
*/
#define kTypeAppletServed		"Applet Served"
#define kTypeImageServed		"Image Served"
#define kTypeObjectServed		"Object Served"
#define kTypePageServed			"Page Served"
#define kTypeFormProcessed		"Form Processed"
#define kTypeObjectProcessed	"Object Processed"
#define kTypeAppletNotModified	"Applet Not Modified"
#define kTypeImageNotModified	"Image Not Modified"
#define kTypeObjectNotModified	"Object Not Modified"
#define kTypePageNotModified	"Page Not Modified"
#define kTypeBasicChallenge		"Basic Challenge"
#define kTypeDigestChallenge	"Digest Challenge"
#define kTypeForbidden			"Forbidden"
#define kTypeBadCommand			"Bad Command"
#define kTypeBadRequest			"Bad Request"
#define kTypeRequestTooLarge	"Request Too Large"
#define kTypeBadContentType		"Bad Content Type"
#define kTypeFileSystemError	"File System Error"
#define kTypeObjectNotFound		"Object Not Found"
#define kTypeOptions			"Options"
#define kTypeTrace				"Trace"
#define kTypeObjectRedirect		"Object Redirect"
#define kTypeRedirectMap		"Redirect Map"
#define kTypeUnknownHTTPCommand	"Unknown HTTP Command"
#define kTypeUnknownEvent		"Unknown Event"

/*
	Event Object Strings
*/
#define kObjectFileSystem		"File System Object"
#define kObjectRemote			"Remote Object"
#define kObjectRomObject		"ROM Object"
#define kObjectUser				"User Object"
#define kObjectFileUpload		"File Upload Object"
#define kObjectIpp				"IPP Object"


void RpBuildHttpEventStrings(void * theTaskDataPtr,
							Signed16 theIndex,
							char * theTimeStringPtr,
							char * theTypeStringPtr,
							char * theObjectStringPtr) {
	rpDataPtr				theDataPtr;
	rpLogItemPtr			theGetPtr;
	rpObjectDescriptionPtr	theObjectPtr;
#if kRpLoggingLevel == 1
	rpRealmPtr				theRealmPtr;
#endif
	Signed16				theItemIndex;

	theDataPtr = (rpDataPtr) theTaskDataPtr;

	/*
		default to null strings.
	*/
	*theTimeStringPtr = '\0';
	*theTypeStringPtr = '\0';
	*theObjectStringPtr = '\0';

	/*
		Return null strings if the requested item index is greater
		than the number of items currently in the log buffer.
	*/
	if (theIndex >= theDataPtr->fLogItemCount) {
		return;
	}

	theIndex = theDataPtr->fLogItemCount - theIndex;

	if (theIndex > theDataPtr->fLogIndex) {
		theIndex -= theDataPtr->fLogIndex;
		theItemIndex = kRpLogEvents - theIndex;
	}
	else {
		theItemIndex = theDataPtr->fLogIndex - theIndex;
	}

	theGetPtr = &theDataPtr->fLogBuffer[theItemIndex];

	/*
		Build the time string.
	*/
	RpBuildTimeString(theTimeStringPtr, theGetPtr->fEventTime);

	switch (theGetPtr->fEventType) {

		case eRpHttpNormal:
#if RomPagerIpp
		case eRpHttpIppNormal:
#endif
			switch (theGetPtr->fObjectSource) {

				case eRpRomUrl:
					/*
						We only know the data type of ROM objects.
					*/
					theObjectPtr =
							(rpObjectDescriptionPtr) theGetPtr->fEventInfoPtr;

					switch (theObjectPtr->fMimeDataType) {

						case eRpDataTypeImageGif:
						case eRpDataTypeImageJpeg:
						case eRpDataTypeImagePict:
						case eRpDataTypeImageTiff:
						case eRpDataTypeImagePng:
							RP_STRCPY(theTypeStringPtr, kTypeImageServed);
							break;

						case eRpDataTypeApplet:
							RP_STRCPY(theTypeStringPtr, kTypeAppletServed);
							break;

						case eRpDataTypeHtml:
						case eRpDataTypeText:
							RP_STRCPY(theTypeStringPtr, kTypePageServed);
							break;

						case eRpDataTypeXml:
						case eRpDataTypeCss:
						case eRpDataTypeWav:
						case eRpDataTypeSwf:
						default:
							RP_STRCPY(theTypeStringPtr, kTypeObjectServed);
							break;
					}
					break;

				case eRpFileUrl:
				case eRpRemoteUrl:
				case eRpUserUrl:
					RP_STRCPY(theTypeStringPtr, kTypeObjectServed);
					break;

				case eRpFileUploadUrl:
#if RomPagerIpp
				case eRpIppUrl:
#endif
					RP_STRCPY(theTypeStringPtr, kTypeObjectProcessed);
					break;

				default:
					break;
			}

			break;

		case eRpHttpNotModified:

			switch (theGetPtr->fObjectSource) {

				case eRpRomUrl:
					/*
						We only know the data type of ROM objects.
					*/
					theObjectPtr =
							(rpObjectDescriptionPtr) theGetPtr->fEventInfoPtr;

					switch (theObjectPtr->fMimeDataType) {

						case eRpDataTypeImageGif:
						case eRpDataTypeImageJpeg:
						case eRpDataTypeImagePict:
						case eRpDataTypeImageTiff:
						case eRpDataTypeImagePng:
							RP_STRCPY(theTypeStringPtr, kTypeImageNotModified);
							break;

						case eRpDataTypeApplet:
							RP_STRCPY(theTypeStringPtr, kTypeAppletNotModified);
							break;

						case eRpDataTypeHtml:
						case eRpDataTypeText:
							RP_STRCPY(theTypeStringPtr, kTypePageNotModified);
							break;

						case eRpDataTypeXml:
						case eRpDataTypeCss:
						case eRpDataTypeWav:
						case eRpDataTypeSwf:
						default:
							RP_STRCPY(theTypeStringPtr, kTypeObjectNotModified);
							break;
					}
					break;

				case eRpFileUrl:
				case eRpRemoteUrl:
				case eRpUserUrl:
				default:
					RP_STRCPY(theTypeStringPtr, kTypeObjectNotModified);
					break;
			}
			break;

#if (kRpLoggingLevel == 1 || kRpLoggingLevel == 2)
		case eRpHttpFormProcessed:
			RP_STRCPY(theTypeStringPtr, kTypeFormProcessed);
			break;
#endif

#if kRpLoggingLevel == 1
		case eRpHttpBadCommand:
			RP_STRCPY(theTypeStringPtr, kTypeBadCommand);
			break;

		case eRpHttpBadRequest:
			RP_STRCPY(theTypeStringPtr, kTypeBadRequest);
			break;

		case eRpHttpForbidden:
			RP_STRCPY(theTypeStringPtr, kTypeForbidden);
			break;

		case eRpHttpNotImplemented:
			RP_STRCPY(theTypeStringPtr, kTypeUnknownHTTPCommand);
			break;

		case eRpHttpNoObjectFound:
			RP_STRCPY(theTypeStringPtr, kTypeObjectNotFound);
			break;

		case eRpHttpInternalServerError:
			RP_STRCPY(theTypeStringPtr, kTypeFileSystemError);
			break;

		case eRpHttpRequestTooLarge:
			RP_STRCPY(theTypeStringPtr, kTypeRequestTooLarge);
			break;

		case eRpHttpRedirect:
			RP_STRCPY(theTypeStringPtr, kTypeObjectRedirect);
			break;

		case eRpHttpRedirectMap:
			RP_STRCPY(theTypeStringPtr, kTypeRedirectMap);
			break;

#if RomPagerSecurity
		case eRpHttpNeedBasicAuthorization:
			RP_STRCPY(theTypeStringPtr, kTypeBasicChallenge);
			break;
#endif

#if RomPagerSecurityDigest
		case eRpHttpNeedDigestAuthorization:
			RP_STRCPY(theTypeStringPtr, kTypeDigestChallenge);
			break;
#endif

#if RomPagerMimeTypeChecking
		case eRpHttpBadContentType:
			RP_STRCPY(theTypeStringPtr, kTypeBadContentType);
			break;
#endif

#if RomPagerOptionsMethod
		case eRpHttpOptions:
			RP_STRCPY(theTypeStringPtr, kTypeOptions);
			break;
#endif

#if RomPagerTraceMethod
		case eRpHttpTracing:
			RP_STRCPY(theTypeStringPtr, kTypeTrace);
			break;
#endif
#endif	/* kRpLoggingLevel == 1 */

		default:
			RP_STRCPY(theTypeStringPtr, kTypeUnknownEvent);
			break;
	}

	/*
		Now build the Object string.
	*/
	switch (theGetPtr->fEventType) {

		case eRpHttpNormal:
		case eRpHttpFormProcessed:
		case eRpHttpNotModified:
		case eRpHttpRedirect:
		case eRpHttpRedirectMap:
#if RomPagerIpp
		case eRpHttpIppNormal:
#endif
			switch (theGetPtr->fObjectSource) {

				case eRpRomUrl:
					/*
						We can only supply the object path for ROM objects.
					*/
					theObjectPtr =
							(rpObjectDescriptionPtr) theGetPtr->fEventInfoPtr;
					RP_STRCPY(theObjectStringPtr, theObjectPtr->fURL);
					break;

#if RomPagerFileSystem
				case eRpFileUrl:
					RP_STRCPY(theObjectStringPtr, kObjectFileSystem);
					break;
#endif

#if RomPagerFileUpload
				case eRpFileUploadUrl:
					RP_STRCPY(theObjectStringPtr, kObjectFileUpload);
					break;
#endif

#if RomPagerIpp
				case eRpIppUrl:
					RP_STRCPY(theObjectStringPtr, kObjectIpp);
					break;
#endif

#if RomPagerRemoteHost
				case eRpRemoteUrl:
					RP_STRCPY(theObjectStringPtr, kObjectRemote);
					break;
#endif

#if RomPagerUserExit
				case eRpUserUrl:
					RP_STRCPY(theObjectStringPtr, kObjectUser);
					break;
#endif

				default:
					break;
			}

			break;

#if kRpLoggingLevel == 1
		case eRpHttpNeedBasicAuthorization:
		case eRpHttpNeedDigestAuthorization:
			theRealmPtr = (rpRealmPtr) theGetPtr->fEventInfoPtr;
			RP_STRCPY(theObjectStringPtr, theRealmPtr->fRealmName);
			break;
#endif

		default:
			break;
	}

	return;
}


static void RpBuildTimeString(char *theDateString, Unsigned32 theDateSeconds) {
	Unsigned32		theHours;
	Unsigned32		theMinutes;
	Unsigned32		theSeconds;

	/*
		Break out the different time components.
	*/
	theHours       = theDateSeconds / kSecsPerHour;
	theDateSeconds = theDateSeconds % kSecsPerHour;
	theMinutes     = theDateSeconds / kSecsPerMin;
	theSeconds     = theDateSeconds % kSecsPerMin;

	/*
		Now, turn the components into a string.
	*/
	*theDateString = '\0';
	RpCatUnsigned32ToString(theHours, theDateString, 2);
	RP_STRCAT(theDateString, kColon);

	RpCatUnsigned32ToString(theMinutes, theDateString, 2);
	RP_STRCAT(theDateString, kColon);

	RpCatUnsigned32ToString(theSeconds, theDateString, 2);

	return;
}


Signed16 RpGetHttpLogItemCount(void * theTaskDataPtr) {

	return ((rpDataPtr) theTaskDataPtr)->fLogItemCount;
}

#endif	/* RomPagerServer && RomPagerLogging */
