/*
 *	File:		AsBase64.c
 *
 *	Contains:	Base64 decoding and encoding routines.
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
 *		09/19/03	pjr		enable Base64 encoding for RpRhLocalCredentials
 *		09/14/03	rhb		fix strict conversion compiler warnings
 *		08/25/03	rhb		expose RpDecodeBase64Data for RomPagerSecure
 * * * * Release 4.20  * * *
 *		01/06/03	amp		expose AsBuildBase64String for RomCliSecure
 * * * * Release 4.12  * * *
 * * * * Release 4.07  * * *
 *		01/14/02	rhb		RpBuildBase64String->AsBuildBase64String
 *		01/11/02	rhb		support Base64 conversions for RomXML
 * * * * Release 4.06  * * *
 * * * * Release 4.04  * * *
 *		11/26/01	pjr		cleanup
 * * * * Release 4.03  * * *
 *		09/26/01	amp		add RmCharsetOption to RomMailer
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.10  * * *
 * * * * Release 3.05  * * *
 *		11/21/99	rhb		mask high bits in RpSendOutBase64Padding
 *		10/12/99	rhb		remove theThirdInputByte from RpBuildBase64String
 * * * * Release 3.04  * * *
 * * * * Release 3.0 * * * *
 *		01/22/99	pjr		RpBase64.c -> AsBase64.c
 * * * * Release 2.2 * * * *
 *		10/23/98	pjr		rework conditional compile flags
 *		10/22/98	pjr		initial version, moved routines from other sources
 * * * * Release 2.1 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if RomPagerSecurity || RomPop || RxBase64Conversions || RomPagerSecure
static Unsigned8	ConvertBase64Character(char theInputCharacter);
#endif

#if ((RomPagerServer && RomMailer) || RmFileOptions || RmCharsetOption)
static void			SendOutBase64Character(rpHttpRequestPtr theRequestPtr,
						Unsigned8 theByteIndex);
static void			SendOutBase64Group(rpHttpRequestPtr theRequestPtr,
						char *theBytesPtr);
#endif

#if ((RomPagerServer && RomMailer) || RmFileOptions || RmCharsetOption) || \
		RomWebClient || RxBase64Conversions || RomCliSecure || \
		RpRhLocalCredentials
static const char	gBase64Table[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#endif

#if RomWebClient || RxBase64Conversions || RomCliSecure || RpRhLocalCredentials
static void			BuildBase64Group(char *theInputPtr, char *theOutputPtr);
#endif


#if RomPagerSecurity || RomPop || RxBase64Conversions || RomPagerSecure

/*
	RpDecodeBase64Data decodes an array of characters encoded as described
	in section 5.2 of RFC 1521 (MIME Part One)
	"Base64 Content-Transfer-Encoding"

	* theInputPtr points to an array of bytes that is at least as long
	  as theInputCount
	* theInputPtr must not contain any white space that should be ignored
	* theOutputPtr is long enough to accept the result including a
	  terminating null
*/

Unsigned16 RpDecodeBase64Data(char *theInputPtr, Unsigned16 theInputCount,
		char *theOutputPtr) {
	Unsigned32	thePackedGroup;
	char *		theOriginalOutputPtr;

	theOriginalOutputPtr = theOutputPtr;

	/*
		Make sure theInputCount is a multiple of 4.
	*/
	theInputCount -= (Unsigned16) (theInputCount % 4);

	while (theInputCount > 0) {
		thePackedGroup =
				(Unsigned32) ConvertBase64Character(*theInputPtr++) << 18;
		thePackedGroup |=
				(Unsigned32) ConvertBase64Character(*theInputPtr++) << 12;
		*theOutputPtr++ = (char) (thePackedGroup >> 16);
		if (*theInputPtr != kAscii_Equal) {
			thePackedGroup |=
					(Unsigned32) ConvertBase64Character(*theInputPtr++) << 6;
			*theOutputPtr++ = (char) (thePackedGroup >> 8);
			if (*theInputPtr != kAscii_Equal) {
				thePackedGroup |=
						(Unsigned32) ConvertBase64Character(*theInputPtr++);
				*theOutputPtr++ = (char) (thePackedGroup);
			}
		}
		theInputCount -= 4;
	}
	*theOutputPtr = '\0';

	return (Unsigned16) (theOutputPtr - theOriginalOutputPtr);
}


static Unsigned8 ConvertBase64Character(char theInputCharacter) {
	Unsigned8	theResult;

	theResult = 0;
	if (theInputCharacter >= kAscii_A &&
		theInputCharacter <= kAscii_Z ) {
		theResult = (Unsigned8) (theInputCharacter - kAscii_A);
	}
	else if (theInputCharacter >= kAscii_a &&
				theInputCharacter <= kAscii_z) {
		theResult = (Unsigned8) (theInputCharacter - kAscii_a + 26);
	}
	else if (theInputCharacter >= kAscii_0 &&
				theInputCharacter <= kAscii_9) {
		theResult = (Unsigned8) (theInputCharacter - kAscii_0 + 52);
	}
	else if (theInputCharacter == kAscii_Plus) {
		theResult = 62;
	}
	else if (theInputCharacter == kAscii_Slash) {
		theResult = 63;
	}
	return theResult;
}

#endif	/* RomPagerSecurity || RomPop || RxBase64Conversions */


#if ((RomPagerServer && RomMailer) || RmFileOptions || RmCharsetOption)

void RpSendDataOutBase64(rpHttpRequestPtr theRequestPtr,
		Unsigned16 theLength, char *theBytesPtr) {

	if (theRequestPtr->fBase64State == eRpBase64_1) {
		theRequestPtr->fBase64Chars[1] = *theBytesPtr++;
		theLength -= 1;
		theRequestPtr->fBase64State = eRpBase64_2;
	}

	if (theRequestPtr->fBase64State == eRpBase64_2) {
		theRequestPtr->fBase64Chars[2] = *theBytesPtr++;
		theLength -= 1;
		theRequestPtr->fBase64State = eRpBase64_0;
		SendOutBase64Group(theRequestPtr, theRequestPtr->fBase64Chars);
	}

	while (theLength > 2) {
		SendOutBase64Group(theRequestPtr, theBytesPtr);
		theRequestPtr->fBase64LineLength += 4;
		if (theRequestPtr->fBase64LineLength == kBase64LineLength) {
			theRequestPtr->fBase64LineLength = 0;
			RpSendDataOutExtendedAscii(theRequestPtr, kCRLF);
		}
		theLength -= 3;
		theBytesPtr += 3;
	}

	if (theLength == 2) {
		theRequestPtr->fBase64State = eRpBase64_2;
		theRequestPtr->fBase64Chars[0] = *theBytesPtr++;
		theRequestPtr->fBase64Chars[1] = *theBytesPtr++;
	}
	else if (theLength == 1) {
		theRequestPtr->fBase64State = eRpBase64_1;
		theRequestPtr->fBase64Chars[0] = *theBytesPtr++;
	}
	return;
}


void RpSendOutBase64Padding(rpHttpRequestPtr theRequestPtr) {
	Unsigned8 theFirstByte, theSecondByte, theThirdByte;

	if (theRequestPtr->fBase64State == eRpBase64_1) {
		theRequestPtr->fBase64Chars[1] = kAscii_Null;
	}
	else {
		theRequestPtr->fBase64Chars[2] = kAscii_Null;
	}

	theFirstByte = (theRequestPtr->fBase64Chars[0] >> 2) & 077;
	theSecondByte = ((theRequestPtr->fBase64Chars[0] << 4) & 060) |
			((theRequestPtr->fBase64Chars[1] >> 4) & 017);

	SendOutBase64Character(theRequestPtr, theFirstByte);
	SendOutBase64Character(theRequestPtr, theSecondByte);

	if (theRequestPtr->fBase64State == eRpBase64_1) {
		RpSendDataOutExtendedAscii(theRequestPtr, kEqual);
		RpSendDataOutExtendedAscii(theRequestPtr, kEqual);
	}
	else {
		theThirdByte = ((theRequestPtr->fBase64Chars[1] << 2) & 074) |
				((theRequestPtr->fBase64Chars[2] >> 6) & 03);

		SendOutBase64Character(theRequestPtr, theThirdByte);
		RpSendDataOutExtendedAscii(theRequestPtr, kEqual);
	}
	return;
}


/*
	Send out one group of 3 bytes.
*/

static void SendOutBase64Group(rpHttpRequestPtr theRequestPtr,
		char *theBytesPtr) {
	Unsigned8 theFirstByte, theSecondByte, theThirdByte, theFourthByte;

	theFirstByte = (*theBytesPtr >> 2) & 077;
	theSecondByte =
			((theBytesPtr[0] << 4) & 060) | ((theBytesPtr[1] >> 4) & 017);
	theThirdByte =
			((theBytesPtr[1] << 2) & 074) | ((theBytesPtr[2] >> 6) & 03);
 	theFourthByte = theBytesPtr[2] & 077;

	SendOutBase64Character(theRequestPtr, theFirstByte);
	SendOutBase64Character(theRequestPtr, theSecondByte);
 	SendOutBase64Character(theRequestPtr, theThirdByte);
 	SendOutBase64Character(theRequestPtr, theFourthByte);

	return;
}


static void SendOutBase64Character(rpHttpRequestPtr theRequestPtr,
		Unsigned8 theByteIndex) {
	char	theCharacter;

	theCharacter = gBase64Table[theByteIndex];
	*theRequestPtr->fHtmlFillPtr++ = theCharacter;
	theRequestPtr->fFillBufferAvailable -= 1;
	if (theRequestPtr->fFillBufferAvailable < 5) {
		/*
			Leave room in the buffer for the
			end padding that might occur.
		*/
		RpFlipResponseBuffers(theRequestPtr);
	}
	return;
}

#endif	/* ((RomPagerServer && RomMailer) || RmFileOptions || RmCharsetOption) */


#if RomWebClient || RxBase64Conversions || RomCliSecure || RpRhLocalCredentials

void AsBuildBase64String(char *theInputPtr, Unsigned16 theInputCount, 
		char *theOutputPtr) {
	char		theFirstInputByte, theSecondInputByte;
	Unsigned8	theFirstOutputByte, theSecondOutputByte, theThirdOutputByte;
	
	while (theInputCount > 2) {
		BuildBase64Group(theInputPtr, theOutputPtr);
		theInputCount -= 3;
		theInputPtr += 3;
		theOutputPtr += 4;
	}
	if (theInputCount == 2) {
		theFirstInputByte = *theInputPtr++;
		theSecondInputByte = *theInputPtr;
	}
	else if (theInputCount == 1) {
		theFirstInputByte = *theInputPtr;
		theSecondInputByte = kAscii_Null;
	}
	else {
		/*
			Must be 0, so terminate the string and return.
		*/
		*theOutputPtr = '\0';
		return;
	}
	theFirstOutputByte = (Unsigned8) (theFirstInputByte >> 2);
	theSecondOutputByte = (Unsigned8) (((theFirstInputByte << 4) & 060) |
			((theSecondInputByte >> 4) & 017));

	*theOutputPtr++ = gBase64Table[theFirstOutputByte];
	*theOutputPtr++ = gBase64Table[theSecondOutputByte];

	if (theInputCount == 1) {
		*theOutputPtr++ = kAscii_Equal;
		*theOutputPtr++ = kAscii_Equal;
	}
	else {
		theThirdOutputByte = (Unsigned8) ((theSecondInputByte << 2) & 074);
		*theOutputPtr++ = gBase64Table[theThirdOutputByte];
		*theOutputPtr++ = kAscii_Equal;
	}
	*theOutputPtr = '\0';
	return;
}

static void BuildBase64Group(char *theInputPtr, char *theOutputPtr) {
	Unsigned8 theFirstByte, theSecondByte, theThirdByte, theFourthByte;

	theFirstByte = (Unsigned8) ((*theInputPtr >> 2) & 077);
	theSecondByte = (Unsigned8)
			(((theInputPtr[0] << 4) & 060) | ((theInputPtr[1] >> 4) & 017));
	theThirdByte = (Unsigned8)
			(((theInputPtr[1] << 2) & 074) | ((theInputPtr[2] >> 6) & 03));
 	theFourthByte = (Unsigned8) (theInputPtr[2] & 077);

	*theOutputPtr++ = gBase64Table[theFirstByte];
	*theOutputPtr++ = gBase64Table[theSecondByte];
	*theOutputPtr++ = gBase64Table[theThirdByte];
	*theOutputPtr = gBase64Table[theFourthByte];
	return;
}

#endif	/* RomWebClient || RxBase64Conversions  || RomCliSecure || RpRhLocalCredentials*/
