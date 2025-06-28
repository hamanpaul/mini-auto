/*
 *	File:		AsParse.c
 *
 *	Contains:	Parsing routines used by the RomPager product line
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
 *		09/08/03	pjr		enable RpFindTokenEnd for RomPlug
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 *		03/12/03	bva		move file upload specific code to RpMulti.c
 * * * * Release 4.20  * * *
 * * * * Release 4.11  * * *
 *		07/18/02	pjr		change conditional for Web Client
 * * * * Release 4.10  * * *
 * * * * Release 4.03  * * *
 *		08/30/01	pjr		fix some conditionals for RomUpnp
 * * * * Release 4.00  * * *
 *		03/15/01	amp		include RpFindTokenEnd for RomMailer
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		05/18/00	pjr		add more RomPopBasic conditionals
 *		04/17/00	pjr		add RpFindCookieEnd
 *		03/30/00	pjr		add RpFindValueLength and RpFindValueStart
 *		02/09/00	bva		use AsEngine.h
 *		02/02/00	rhb		add null check to RpFindTokenEnd
 *		01/24/00	bva		fix warning
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 *		01/12/00	amp		add RomMailerBasic RomPopBasic conditionals
 *		01/11/00	bva		NULL -> (char *) 0
 * * * * Release 3.06  * * *
 * * * * Release 3.03 * * * *
 *		07/05/99	pjr		add debug statement in RpParseReplyBuffer
 *		07/01/99	bva		RomMailer uses RpParseReplyBuffer, GetBufferRequestLine,
 *							RpFindTokenDelimitedPtr, RpFindTokenDelimited
 * * * * Release 3.02  * * *
 * * * * Release 3.0 * * * *
 *		02/19/99	bva		add RpFindTokenDelimitedPtr
 *		01/22/99	pjr		RpParse.c -> AsParse.c
 *		01/14/99	pjr		enable for RomWebClient
 *		12/24/98	pjr		RomPagerBasic conditionals
 * * * * Release 2.2 * * * *
 *		12/01/98	bva		use RP_ATOL
 *		11/11/98	rhb		fix GetBufferRequestLine for very long partial lines
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		10/23/98	pjr		move RpStringToMimeType routine to RpMimes.c
 *		10/22/98	pjr		move Base64 routines to RpBase64.c
 *		10/05/98	pjr		initial version, moved routines from other sources
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"

#if RomPagerServer || RomPagerBasic || RomPop || RomPopBasic || RomWebClient
static Unsigned8	MonthToNumber(char *theMonthPtr);
static void			ParseTime(char *theTimePtr, rpDatePtr theDatePtr);
#endif

#if RomPagerServer || RomPagerBasic  || RomMailer || RomMailerBasic || RomPop  || RomPopBasic || RomWebClient
static rpLineState	GetBufferRequestLine(rpParsingControlPtr theParsingPtr);
#endif


#if 1
Unsigned16 RpFindTokenDelimited(char *theStartOfTokenPtr, char theDelimiter) {
	Unsigned16	theTokenLength;

	/*
		Return an offset to the location in the string of the character 
		contained in theDelimiter.  If not found the offset points to
		the end of the string.
	*/
	theTokenLength = 0;
	while (*theStartOfTokenPtr != '\0' &&
			*theStartOfTokenPtr != theDelimiter) {
		theStartOfTokenPtr += 1;
		theTokenLength     += 1;
	}
	return theTokenLength;
}

char * RpFindTokenDelimitedPtr(const char *theStartOfTokenPtr, 
								const char theDelimiter) {
	Unsigned16	theTokenLength;
	char *		theTokenPtr;

	/*
		Return a character pointer to the location of the character contained
		in theDelimiter. If not found the char pointer is set to null. This 
		routine is equivalent to the standard C library routine 'strchr'.
	*/
	theTokenLength = RpFindTokenDelimited((char *) theStartOfTokenPtr, theDelimiter);
	if (theTokenLength == RP_STRLEN(theStartOfTokenPtr)) {
		theTokenPtr = (char *) 0;
	}
	else {
		theTokenPtr = (char *) theStartOfTokenPtr + theTokenLength;
	}
	return theTokenPtr;
}

#else

Unsigned16 RpFindTokenDelimited(char *theStartOfTokenPtr, char theDelimiter) {
	char *		theTokenPtr;

	/*
		Return an offset to the location in the string of the character 
		contained in theDelimiter.  If not found the offset points to
		the end of the string.
	*/
	theTokenPtr = RpFindTokenDelimitedPtr(theStartOfTokenPtr, theDelimiter);
	return (theTokenPtr - theStartOfTokenPtr);
}

char * RpFindTokenDelimitedPtr(const char *theStartOfTokenPtr, 
								const char theDelimiter) {
	/*
		Return a character pointer to the location of the character contained
		in theDelimiter. If not found the char pointer is set to null. This 
		routine is equivalent to the standard C library routine 'strchr'.
	*/
	while (*theStartOfTokenPtr != '\0' &&
			*theStartOfTokenPtr != theDelimiter) {
		theStartOfTokenPtr += 1;
	}
	return (char *) theStartOfTokenPtr;
}
#endif


#if RomPagerServer || RomPagerBasic || RomPop || RomPopBasic

void RpConvertHeaderToLowerCase(rpParsingControlPtr theParsingPtr) {
	char *		theLinePtr;
	Unsigned16	theTokenLength;

	theLinePtr = theParsingPtr->fCurrentBeginLinePtr;
	
	/*
		A HTTP header line is of the format <headername>:<headervalues>.
		This routine converts the <headername> to lower case.
	*/
	theTokenLength = RpFindTokenDelimited(theLinePtr, kAscii_Colon);
	if (theTokenLength < theParsingPtr->fCompleteLineLength) {
		RpConvertTokenToLowerCase(theLinePtr, theTokenLength);
	}
	return;
}

#endif


#if RomPagerServer || RomPagerBasic || RomPop || RomPopBasic || RomPlug || RomWebClient

void RpConvertTokenToLowerCase(char *theTokenPtr, Unsigned16 theTokenLength) {
	char		theChar;

	while (theTokenLength > 0) {
		theChar = *theTokenPtr;
		if (theChar >= kAscii_A && theChar <= kAscii_Z) {
			*theTokenPtr = theChar ^ kAscii_Space;
		}
		theTokenLength--;
		theTokenPtr++;
	}
	return;
}


Unsigned16 RpFindLineEnd(char *theStartOfTokenPtr) {
	Unsigned16	theTokenLength;

	theTokenLength = 0;
	while (	*theStartOfTokenPtr != '\x0a' &&
			*theStartOfTokenPtr != '\x0d' ) {
		theStartOfTokenPtr += 1;
		theTokenLength     += 1;
	}
	return theTokenLength;
}


char * RpFindTokenStart(char *theBeginLinePtr) {
	char *	theStartOfTokenPtr;

	theStartOfTokenPtr = theBeginLinePtr;
	while (	*theStartOfTokenPtr == ' '  ||
			*theStartOfTokenPtr == ':'  ||
			*theStartOfTokenPtr == '\t' ) {
		theStartOfTokenPtr += 1;
	}
	return theStartOfTokenPtr;
}

#endif


#if RomPagerServer || RomPagerBasic || RomMailer || RomPop || RomPopBasic || \
		RomPlug || RomWebClient
Unsigned16 RpFindTokenEnd(char *theStartOfTokenPtr) {
	Unsigned16	theTokenLength;

	theTokenLength = 0;
	while (	*theStartOfTokenPtr != ' '  &&
			*theStartOfTokenPtr != '\t' &&
			*theStartOfTokenPtr != ';'  &&
			*theStartOfTokenPtr != '\0' &&
			*theStartOfTokenPtr != '\x0a' &&
			*theStartOfTokenPtr != '\x0d' ) {
		theStartOfTokenPtr += 1;
		theTokenLength     += 1;
	}
	return theTokenLength;
}
#endif


#if RomPagerServer || RomPagerBasic || RomPop || RomPopBasic || RomWebClient

/*
	This routine parses an Internet format date string and
	turns it into a number of seconds in the internal 
	system time format.

	Dates can be passed in three formats:
		Sun, 06 Nov 1994 08:49:37 GMT	; RFC 822/RFC 1123
		Sunday, 06-Nov-94 08:49:37 GMT	; RFC 850/RFC 1036
		Sun Nov 6 08:49:37 1994			; ANSI C's asctime() format
		
	NOTES:	The first item (the day of the week) is optional.
			The day number (06 in this example) may be 1 or 2 digits.
			The year number (1994 in this example) may be 2 or 4 digits.

	For compatability with HTTP 1.0, dates must be accepted in any format,
	although they will be generated only in RFC 1123 format.
*/

Unsigned32 RpParseDate(rpDataPtr theDataPtr, char *theDateStringPtr) {
	rpDate		theDate;
	Unsigned16	theTokenLength;
	
	theDate.fDay = 0;
	theDate.fMonth = 0;
	theDate.fYear = 0;
	theDate.fHours = 0;
	theDate.fMinutes = 0;
	theDate.fSeconds = 0;
	
	if (*theDateStringPtr < kAscii_0 || *theDateStringPtr > kAscii_9) {
		/*
			The first character is not a number.  This
			token must be the day of the week, so skip it.
		*/
		theTokenLength = RpFindTokenEnd(theDateStringPtr);
		theDateStringPtr = RpFindTokenStart(theDateStringPtr + theTokenLength);
	}

	/*
		the second token (or possibly first) is either the month, 
		the day of the month, or the RFC 850 date.
	*/
	theTokenLength = RpFindTokenEnd(theDateStringPtr);
	if (theTokenLength == 3) {
		/*
			well it must be a month, so we have ANSI-C asctime format.
			first make it a C string
		*/
		*(theDateStringPtr + theTokenLength) = '\0';
		theDate.fMonth = MonthToNumber(theDateStringPtr);
		theDateStringPtr += 4;

		/*
			the third token in asctime will be the day.
			make it a C string after finding its start and length
		*/
		theDateStringPtr = RpFindTokenStart(theDateStringPtr);
		theTokenLength = RpFindTokenEnd(theDateStringPtr);
		*(theDateStringPtr + theTokenLength) = '\0';
		theDate.fDay = RP_ATOL(theDateStringPtr);
		theDateStringPtr += theTokenLength + 1;
		
		/*
			the fourth token in asctime will be the hours, minutes, seconds
		*/
		ParseTime(theDateStringPtr, &theDate);
		theDateStringPtr += 8;

		/*
			the fifth token in asctime format will be the year
			make it a C string after finding its length
		*/
		theTokenLength = RpFindTokenEnd(theDateStringPtr);
		*(theDateStringPtr + theTokenLength) = '\0';
		theDate.fYear = RP_ATOL(theDateStringPtr);
	}
	else {
		/*
			well it must be RFC 822 format or RFC 850 format which may be 
			handled the same way, if we're careful.
			first make the day a C string
		*/	
		theTokenLength = RpFindTokenEnd(theDateStringPtr);
		if (theTokenLength > 2) {
			/*
				The character separating the day from the month
				must be a hyphen.
			*/
			theTokenLength = RpFindTokenDelimited(theDateStringPtr,
													kAscii_Hyphen);
		}
		*(theDateStringPtr + theTokenLength) = '\0';
		theDate.fDay = RP_ATOL(theDateStringPtr);
		theDateStringPtr += theTokenLength + 1;

		/*
			the third token in RFC 822 format or RFC 850 format will be 
			the month. First make it a C string.
		*/
		*(theDateStringPtr + 3) = '\0'; 
		theDate.fMonth = MonthToNumber(theDateStringPtr);
		theDateStringPtr += 4;
		
		/*
			the fourth token in RFC 822 format or RFC 850 format will be 
			the year. Make it a C string after finding its length
		*/
		theTokenLength = RpFindTokenEnd(theDateStringPtr);
		*(theDateStringPtr + theTokenLength) = '\0';
		theDate.fYear = RP_ATOL(theDateStringPtr);
		theDateStringPtr += theTokenLength + 1;

		/*
			the fifth token in RFC 822 format or RFC 850 format 
			will be the hours, minutes, seconds
		*/
		ParseTime(theDateStringPtr, &theDate);
	}
	/*
		Unify the year representation
	*/
	if (theDate.fYear > 1900) {
		theDate.fYear -= 1900;
	}
	return RpGetDateInSeconds(theDataPtr, &theDate);
}


static Unsigned8 MonthToNumber(char *theMonthPtr) {
	Unsigned8	theMonthNumber;

	theMonthNumber = 0;
	while (RP_STRCMP(theMonthPtr,
			gRpMonthTable[theMonthNumber]) != 0 &&
			theMonthNumber < 11) {
		theMonthNumber += 1;
	}

	return theMonthNumber;
}


static void ParseTime(char *theTimePtr, rpDatePtr theDatePtr) {

	*(theTimePtr + 2) = '\0';
	theDatePtr->fHours = RP_ATOL(theTimePtr);
	theTimePtr += 3;
	*(theTimePtr + 2) = '\0';
	theDatePtr->fMinutes = RP_ATOL(theTimePtr);
	theTimePtr += 3;
	*(theTimePtr + 2) = '\0';
	theDatePtr->fSeconds = RP_ATOL(theTimePtr);
	return;
}

#endif	/* RomPagerServer || RomPagerBasic || RomPop || RomPopBasic || RomWebClient */


#if RomPagerServer || RomPagerBasic  || RomMailer || RomMailerBasic || RomPop || RomPopBasic || RomWebClient

rpLineState RpParseReplyBuffer(rpParsingControlPtr theParsingPtr) {
	Unsigned16		theCompleteLineLength;
	char *			theTempBufferPtr;
	Unsigned16		theCurrentLength;
	Unsigned16		thePartialLength;
	rpLineState		theLineState;

	theParsingPtr->fCurrentBeginLinePtr = theParsingPtr->fCurrentBufferPtr;
	theParsingPtr->fCurrentEndOfLinePtr = theParsingPtr->fCurrentBeginLinePtr;
	theParsingPtr->fCurrentEndOfBufferPtr = 
			theParsingPtr->fCurrentBeginLinePtr + 
			theParsingPtr->fIncomingBufferLength;
	theLineState = GetBufferRequestLine(theParsingPtr);
	if (theLineState == eRpLineComplete) {
		theCurrentLength = theParsingPtr->fCurrentLineLength;
		thePartialLength = theParsingPtr->fPartialLineLength;
		if (thePartialLength > 0) {
			/*	
				we need to join the two partial lines
			*/
			theCompleteLineLength = theCurrentLength + thePartialLength;
			if (theCompleteLineLength > kMaxParseLength) {
#if AsDebug
				RP_PRINTF("RpParseReplyBuffer, line too long, length = %d\n",
							theCompleteLineLength);
#endif	
				theLineState = eRpLineError;	
			}
			else {
				theTempBufferPtr = theParsingPtr->fTempBufferPtr + 
						thePartialLength;
				RP_MEMCPY(theTempBufferPtr, 
						theParsingPtr->fCurrentBeginLinePtr, 
						theCurrentLength);
				theParsingPtr->fCurrentBeginLinePtr = 
						theParsingPtr->fTempBufferPtr;
				theParsingPtr->fCompleteLineLength = theCompleteLineLength;
			}
			theParsingPtr->fPartialLineLength = 0;
		}
		else {
			theParsingPtr->fCompleteLineLength = theCurrentLength;
		}
		if (theParsingPtr->fCompleteLineLength <= 2) {
			theLineState = eRpLineEmpty;	
		}
		/*	
			set up to process the rest of the buffer
		*/
		theParsingPtr->fCurrentBufferPtr = theParsingPtr->fCurrentEndOfLinePtr;
		theParsingPtr->fIncomingBufferLength -= 
				theParsingPtr->fCurrentLineLength;
	}
	return theLineState;
}


static rpLineState GetBufferRequestLine(rpParsingControlPtr theParsingPtr) {
	char *		theBeginLinePtr;
	char *		theEndOfBufferPtr;
	char *		theEndOfLinePtr;
	Unsigned16	theFragmentLength;
	char *		theTempBufferPtr;
	rpLineState	theLineState;

	if (theParsingPtr->fIncomingBufferLength == 0) {
		/*
			The last complete line read happened to line up on the buffer
			boundary.  All we need to do is return the line state
			as eRpLinePartial, so the caller will go get another buffer.
		*/
		return eRpLinePartial;
	}
	theParsingPtr->fCurrentLineLength = 0;
	theFragmentLength = 0;
	theEndOfBufferPtr = theParsingPtr->fCurrentEndOfBufferPtr;
	theEndOfLinePtr = theParsingPtr->fCurrentEndOfLinePtr;
	theBeginLinePtr = theParsingPtr->fCurrentBeginLinePtr;
	theLineState = eRpLineError;
	while (theFragmentLength == 0) {
		if (theEndOfLinePtr < theEndOfBufferPtr) {
			if (*theEndOfLinePtr == '\x0a') {
				/*
					hey, we got a line!
				*/
				theFragmentLength = theEndOfLinePtr - theBeginLinePtr + 1;
				if (theFragmentLength <= kMaxParseLength) {
					theLineState = eRpLineComplete;
					theParsingPtr->fCurrentLineLength = theFragmentLength;
				}
			}
		}
		else {
			/*
				we're at the end of the buffer and we have only a partial
				line, so save it away, and return for another buffer.
			*/
			theFragmentLength = theEndOfBufferPtr - theBeginLinePtr;
			if (theFragmentLength <= 
					kMaxParseLength - theParsingPtr->fPartialLineLength) {
				theLineState = eRpLinePartial;
				theTempBufferPtr = theParsingPtr->fTempBufferPtr +
							theParsingPtr->fPartialLineLength;
				RP_MEMCPY(theTempBufferPtr, theBeginLinePtr, theFragmentLength);
				theParsingPtr->fPartialLineLength += theFragmentLength;
			}
			theParsingPtr->fIncomingBufferLength = 0;
		}
		theEndOfLinePtr += 1;
	}	/* end while */

	theParsingPtr->fCurrentEndOfLinePtr = theEndOfLinePtr;
	return theLineState;
}

#endif	/* RomPagerServer || RomPagerBasic || RomMailer || RomMailerBasic || RomPop || RomPopBasic || RomWebClient */


#if RomPagerSecurityDigest || RomPagerFileUpload || WcDigestAuthentication

/*
	The value may or may not be a quoted string.
	It either ends at a double quote, comma, or
	line end.  Examples:

		<nonce="dce53a5676dc672d21ec55d98d037522", >
		<nc=00000001, cnonce="6379bb6173a8312ca402c1ca02cb3438"CR LF>
*/

Unsigned16 RpFindValueLength(char *theValuePtr) {
	char *		theValueStartPtr;

	theValueStartPtr = theValuePtr;

	while (*theValuePtr != kAscii_Quote &&
			*theValuePtr != kAscii_Comma &&
			*theValuePtr != kAscii_Return &&
			*theValuePtr != kAscii_Newline) {
		theValuePtr++;
	}

	return theValuePtr - theValueStartPtr;
}

#endif	/* RomPagerSecurityDigest || RomPagerFileUpload || WcDigestAuthentication */


#if RomPagerSecurityDigest || RomPagerFileUpload || WcDigestAuthentication || WcHttpCookies || WcRefresh

/*
	The value may or may not be a quoted string.
	It either starts after an equals sign, or an
	equals sign, double quote <=">.
	Examples:

		<nc=00000001>
		<nonce="dce53a5676dc672d21ec55d98d037522">
*/

char *RpFindValueStart(char *theValuePtr) {

	while (*theValuePtr != kAscii_Equal) {
		if (*theValuePtr == kAscii_Return ||
				*theValuePtr == kAscii_Newline) {
			*theValuePtr = kAscii_Null;
			break;
		}
		else {
			theValuePtr++;
		}
	}

	if (*theValuePtr == kAscii_Equal) {
		theValuePtr++;
		if (*theValuePtr == kAscii_Quote) {
			theValuePtr++;
		}
	}

	return theValuePtr;
}

#endif	/* RomPagerSecurityDigest || RomPagerFileUpload || WcDigestAuthentication || WcHttpCookies || WcRefresh */


#if RomPagerHttpCookies || WcHttpCookies

Unsigned16 RpFindCookieEnd(char *theCookiePtr) {
	Unsigned16	theTokenLength;

	theTokenLength = 0;
	while ( *theCookiePtr != '\t'   &&
#if 0
			*theCookiePtr != ' '    &&
#endif
			*theCookiePtr != ';'    &&
			*theCookiePtr != ','    &&
			*theCookiePtr != '\x0a' &&
			*theCookiePtr != '\x0d' ) {
		theCookiePtr += 1;
		theTokenLength     += 1;
	}
	return theTokenLength;
}

#endif	/* RomPagerHttpCookies || WcHttpCookies */

// +++ _Alphanetworks_Patch_, 12/20/2003, jacob_shih
/*
	RpParseOSDate
 	
	This routine converts the OS date string (Mar 04 2004 14:07:08)
	into the internal systems seconds format.

	Input:		OS date string
	Returns:	the time in system time format in seconds.
*/

Unsigned32 RpParseOSDate(rpDataPtr theDataPtr, char* theDateStringPtr)
{
	rpDate		theDate;
	Unsigned16	theTokenLength;
	
	theDate.fDay = 0;
	theDate.fMonth = 0;
	theDate.fYear = 0;
	theDate.fHours = 0;
	theDate.fMinutes = 0;
	theDate.fSeconds = 0;
	
	/*
		The first token (or possibly first) is either the month, 
		the day of the month, or the RFC 850 date.
	*/
	theTokenLength = RpFindTokenEnd(theDateStringPtr);
	if (theTokenLength == 3) {
		/*
			well it must be a month, so we have ANSI-C asctime format.
			first make it a C string
		*/
		*(theDateStringPtr + theTokenLength) = '\0';
		theDate.fMonth = MonthToNumber(theDateStringPtr);
		theDateStringPtr += 4;

		/*
			the second token in asctime will be the day.
			make it a C string after finding its start and length
		*/
		theDateStringPtr = RpFindTokenStart(theDateStringPtr);
		theTokenLength = RpFindTokenEnd(theDateStringPtr);
		*(theDateStringPtr + theTokenLength) = '\0';
		theDate.fDay = RP_ATOL(theDateStringPtr);
		theDateStringPtr += theTokenLength + 1;
		
		/*
			the third token in asctime format will be the year
			make it a C string after finding its length
		*/
		theTokenLength = RpFindTokenEnd(theDateStringPtr);
		*(theDateStringPtr + theTokenLength) = '\0';
		theDate.fYear = RP_ATOL(theDateStringPtr);
		theDateStringPtr += theTokenLength + 1;

		/*
			the fourth token in asctime will be the hours, minutes, seconds
		*/
		ParseTime(theDateStringPtr, &theDate);

	}
	else {
		/*
			well it must be RFC 822 format or RFC 850 format which may be 
			handled the same way, if we're careful.
			first make the day a C string
		*/	
		theTokenLength = RpFindTokenEnd(theDateStringPtr);
		if (theTokenLength > 2) {
			/*
				The character separating the day from the month
				must be a hyphen.
			*/
			theTokenLength = RpFindTokenDelimited(theDateStringPtr,
													kAscii_Hyphen);
		}
		*(theDateStringPtr + theTokenLength) = '\0';
		theDate.fDay = RP_ATOL(theDateStringPtr);
		theDateStringPtr += theTokenLength + 1;

		/*
			the third token in RFC 822 format or RFC 850 format will be 
			the month. First make it a C string.
		*/
		*(theDateStringPtr + 3) = '\0'; 
		theDate.fMonth = MonthToNumber(theDateStringPtr);
		theDateStringPtr += 4;
		
		/*
			the fourth token in RFC 822 format or RFC 850 format will be 
			the year. Make it a C string after finding its length
		*/
		theTokenLength = RpFindTokenEnd(theDateStringPtr);
		*(theDateStringPtr + theTokenLength) = '\0';
		theDate.fYear = RP_ATOL(theDateStringPtr);
		theDateStringPtr += theTokenLength + 1;

		/*
			the fifth token in RFC 822 format or RFC 850 format 
			will be the hours, minutes, seconds
		*/
		ParseTime(theDateStringPtr, &theDate);
	}
	/*
		Unify the year representation
	*/
	if (theDate.fYear > 1900) {
		theDate.fYear -= 1900;
	}
	return RpGetDateInSeconds(theDataPtr, &theDate);
}
// --- _Alphanetworks_Patch_, 12/20/2003, jacob_shih
