/*
 *	File:		AsCommon.c
 *
 *	Contains:	Common subroutines used by the RomPager product line
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
 *		09/19/03	pjr		enable RpCatSigned32ToString and
 *							RpConvertSigned32ToAscii for RomPagerServer
 *		09/14/03	rhb		fix strict conversion compiler warnings
 *		07/08/03	bva		rework AsVariableAccess ifdefs
 * * * * Release 4.21  * * *
 *      04/29/03    amp     change UPnP toolkit name to RomPlug
 *		03/13/03	rhb		enable RpStrLenCpyTruncate for RomPagerServer and 
 *								RomPagerBasic 
 * * * * Release 4.20  * * *
 *		10/04/02	amp		enable AsStrcasecmp for RomPagerSecure
 *		10/02/02	pjr		enable RpStrLenCpyTruncate for WcHttpCookies
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *		07/18/02	pjr		enable Strcasecmp routines for RomWebClient
 * * * * Release 4.10  * * *
 * * * * Release 4.07  * * *
 *		03/22/02	pjr		prevent hang if both response buffers overflow
 * * * * Release 4.06  * * *
 * * * * Release 4.04  * * *
 *		11/26/01	pjr		enable RpCatSigned32ToString for RomPagerTraceMethod
 * * * * Release 4.03  * * *
 *		08/26/01	amp		add RmCharsetOption to RomMailer
 *		08/23/01	amp		fix RpStrLenCpyTruncate protection fault
 *		08/14/01	pjr		add new line at end of file
 * * * * Release 4.00  * * *
 *		07/26/01	rhb/pjr	use As64Bit macro in g64BitPowersOf10 table
 *		05/02/01	bva		rework RpConvertUnsigned32ToHex, RpCloseChunkedBuffer
 *		03/07/01	bva		make RpStrLenCpyTruncate available for RomCLI
 *		03/06/01	bva		rework Rp_strcasecmp, Rp_strncasecmp
 *		02/21/01	bva		rework conditionals
 *		02/13/01	rhb		rename RomPagerUse64BitIntegers to AsUse64BitIntegers
 *		01/24/01	rhb		fix SoftPages/HTTP 1.1 bug
 *		01/11/01	rhb		remove implicit conversion warnings
 *		12/19/00	pjr		enable RpHexToNibble for RomCli
 *		12/18/00	amp		fix return value of RpHexToUnsigned32
 *		12/12/00	pjr		enable RpWriteIpAddressInDotForm for RomWebClient
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		10/12/00	bva		add RpGetRemoteIpAddress
 *		09/06/00	bva		add RpHexToUnsigned32
 *		09/06/00	pjr		enable RpStrLenCpyTruncate for RomPagerIpp
 *		09/01/00	bva/dts	add Rp_strcasecmp, Rp_strncasecmp
 *		08/25/00	bva		fix RpBuildIpAddressHostName for RomPagerSecure
 *		08/08/00	bva		fix warnings
 *		07/31/00	bva		add RpBuildIpAddressHostName, RpWriteIpAddressInDotForm
 *		07/05/00	rhb		enable RpHexToString for WcSecurityDigest
 *		06/02/00	bva		change RpBuildSeparator to eliminate passing connection ptr
 *		02/09/00	bva		use AsEngine.h
 *		01/28/00	bva		change RpSendDataOutExtendedAscii to fix warnings
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 *		01/13/00	amp		expose RpStrLenCpy for RomMailerBasic
 *		12/27/99	rhb		add RpStrLenCpyTruncate
 * * * * Release 3.06  * * *
 * * * * Release 3.05  * * *
 *		09/19/99	rhb		add RomXml support
 * * * * Release 3.04  * * *
 *		07/19/99	rhb		enable RpStrLenCpy for RomPop
 * * * * Release 3.03  * * *
 * * * * Release 3.02  * * *
 *		05/06/99	pjr		enable RpHexToNibble for RomWebClient
 * * * * Release 3.01  * * *
 *		04/16/99	rhb		use Soft Page request blocks
 * * * * Release 3.0 * * * *
 *		04/01/99	pjr		enable RpConvertUnsigned32ToHex for WcHttpOneDotOne
 *		03/31/99	rhb		fix soft page buffer size in 
 *								RpInitializeHtmlPageReply()
 *		03/22/99	pjr		make RpConvertUnsigned32ToHex global
 *		03/16/99	pjr		rework debug code
 *		03/08/99	bva		add RpStrLenCpy
 *		01/22/99	pjr		RpCommon.c -> AsCommon.c
 *		01/20/99	pjr		change RpHexToNibble to return a Boolean error
 *								flag instead of an rpItemError
 *		01/19/99	pjr		enable RpCatSigned32ToString and
 *								RpConvertSigned32ToAscii for RomPagerQueryIndex
 *		01/11/99	bva		rework conditionals for RomWebClient
 *		12/31/98	pjr		more conditional rework
 *		12/24/98	pjr		rework conditionals for RomPagerBasic and
 *								move chunked logic from RpHtml.c
 *		12/11/98	rhb		merge Soft Pages
 * * * * Release 2.2 * * * *
 *		11/24/98	bva		rework #if logic
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		10/23/98	pjr		add more conditionals
 *		10/22/98	pjr		move Base64 routines to RpBase64.c
 *		10/01/98	pjr		initial version, moved routines from other sources
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


static Unsigned32 gPowersOf10[] = {
	1000000000,
	100000000,
	10000000,
	1000000,
	100000,
	10000,
	1000,
	100,
	10,
	1,
	0
};


#if AsUse64BitIntegers
static Unsigned64 g64BitPowersOf10[] = {
	As64Bit(10000000000000000000U),
	As64Bit(1000000000000000000U),
	As64Bit(100000000000000000U),
	As64Bit(10000000000000000U),
	As64Bit(1000000000000000U),
	As64Bit(100000000000000U),
	As64Bit(10000000000000U),
	As64Bit(1000000000000U),
	As64Bit(100000000000U),
	As64Bit(10000000000U),
	As64Bit(1000000000U),
	As64Bit(100000000U),
	As64Bit(10000000U),
	As64Bit(1000000U),
	As64Bit(100000U),
	As64Bit(10000U),
	As64Bit(1000U),
	As64Bit(100U),
	As64Bit(10U),
	As64Bit(1U),
	As64Bit(0U)
};
#endif


#if RomPagerHttpOneDotOne || WcHttpOneDotOne
static Unsigned32 gPowersOf16[] = {
	268435456,
	16777216,
	1048576,
	65536,
	4096,
	256,
	16,
	1,
	0
};
#endif


#if RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption

/*
	Initialize the reply (send) buffer states.
*/

void RpInitializeHtmlPageReply(rpHttpRequestPtr theRequestPtr) {
		
#if RomPagerSoftPages
	if (theRequestPtr->fSoftPageFlag) {
#if RomPagerSoftPageCompression
		/*
			Leave space for null termination.
		*/
		theRequestPtr->fFillBufferAvailable = kSpParseBufferSize - 1;
#else
		theRequestPtr->fFillBufferAvailable = kSpParseBufferSize;
#endif
		theRequestPtr->fHtmlFillPtr = 
				theRequestPtr->fSoftPageRequestPtr->fParseBuffer1;
	}
	else 
#endif
	{
		theRequestPtr->fFillBufferAvailable = kHtmlMemorySize;
		theRequestPtr->fHtmlFillPtr = theRequestPtr->fHtmlResponseBufferOne;
	}
	theRequestPtr->fHtmlCurrentBuffer = 1;
#if RomPagerServer
	theRequestPtr->fItemState = eRpHtmlFirst;
#endif

#if RomPagerHttpOneDotOne
#if RomPagerSoftPages
	if (!theRequestPtr->fSoftPageFlag) {
		RpSetupChunkedBuffer(theRequestPtr);
	}
#else
	RpSetupChunkedBuffer(theRequestPtr);
#endif
#endif

	return;
}


void RpFlipResponseBuffers(rpHttpRequestPtr theRequestPtr) {

	if (theRequestPtr->fHtmlBufferReady == False) {
		theRequestPtr->fHtmlBufferReady = True;
		if (theRequestPtr->fHtmlCurrentBuffer == 1) {
			theRequestPtr->fHtmlResponsePtr = 
					theRequestPtr->fHtmlResponseBufferOne;
			theRequestPtr->fHtmlFillPtr = 
					theRequestPtr->fHtmlResponseBufferTwo;
			theRequestPtr->fHtmlCurrentBuffer = 2;
		}
		else {
			theRequestPtr->fHtmlResponsePtr = 
					theRequestPtr->fHtmlResponseBufferTwo;
			theRequestPtr->fHtmlFillPtr = 
					theRequestPtr->fHtmlResponseBufferOne;
			theRequestPtr->fHtmlCurrentBuffer = 1;
		}
		theRequestPtr->fHtmlResponseLength = kHtmlMemorySize - 
				theRequestPtr->fFillBufferAvailable;
		theRequestPtr->fFillBufferAvailable = kHtmlMemorySize;
#if RomPagerHttpOneDotOne
		RpCloseChunkedBuffer(theRequestPtr);
		RpSetupChunkedBuffer(theRequestPtr);
#endif
	}
	else {
		/*
			Both of the buffers are full! This will happen in an improper
			page design if a single item is too long to be handled properly. 
			So to avoid causing a hang, we will throw away both buffers and 
			start filling the first one again.  This should make it obvious 
			to the developer that something is wrong. In debug mode, we
			can notify the developer.

			One of the common causes of this problem is using a DisplayText
			item to insert a large amount of text data into a Web page. The
			DisplayBuffer item should be used instead.		
		*/
		theRequestPtr->fHtmlBufferReady = False;
		theRequestPtr->fHtmlCurrentBuffer = 1;
		theRequestPtr->fFillBufferAvailable = kHtmlMemorySize;
		theRequestPtr->fHtmlFillPtr = theRequestPtr->fHtmlResponseBufferOne;
		theRequestPtr->fHtmlResponsePtr = theRequestPtr->fHtmlResponseBufferOne;
		theRequestPtr->fHtmlResponseLength = 0;
#if RomPagerHttpOneDotOne
		RpSetupChunkedBuffer(theRequestPtr);
#endif
#if AsDebug
		RP_PRINTF("RpFlipResponseBuffers, buffer overflow error!\n");
#endif	
	}

	return;
}

#endif	/* RomPagerServer || RomPagerBasic || RmFileOptions || RmCharsetOption */


#if RomPagerServer || RmFileOptions || RmCharsetOption

void RpSendDataOutExtendedAscii(rpHttpRequestPtr theRequestPtr,
		const char * theAsciiPtr) {
	if (theAsciiPtr != (const char *) 0) {
		while (*theAsciiPtr != (char) 0) {
			if (theRequestPtr->fFillBufferAvailable > 0) {
				*theRequestPtr->fHtmlFillPtr++ = *theAsciiPtr++;
				theRequestPtr->fFillBufferAvailable -= 1;
			} 
			else {
				RpFlipResponseBuffers(theRequestPtr);
			}
		}
	}
	return;
}

#endif	/* RomPagerServer || RmFileOptions || RmCharsetOption */


#if RomPagerHttpOneDotOne

void RpSetupChunkedBuffer(rpHttpRequestPtr theRequestPtr) {

	if (theRequestPtr->fResponseIsChunked) {
		/*
			If the output is chunked, the buffer needs to have room
			for the leading chunk size "HHH<CR><LF>", the trailing "<CR><LF>" 
			and a possible last chunk marker of "0<CR><LF><CR><LF>".
		*/
		theRequestPtr->fHtmlFillPtr += 3;
		*(theRequestPtr->fHtmlFillPtr++) = '\x0d';
		*(theRequestPtr->fHtmlFillPtr++) = '\x0a';
		theRequestPtr->fFillBufferAvailable -= kChunkedOverhead;	
	}
	return;
}


void RpCloseChunkedBuffer(rpHttpRequestPtr theRequestPtr) {
	Unsigned32		theChunkSize;
	char			*theBufferPtr;

	if (theRequestPtr->fResponseIsChunked) {
		theChunkSize = theRequestPtr->fHtmlResponseLength - kChunkedOverhead;
		if (theChunkSize == 0) {
			/*
				We must have put out all of the chunked data previously,
				so just set up this buffer with final last chunk of 0.
			*/
			RP_STRCPY(theRequestPtr->fHtmlResponsePtr, kLastChunk);	
			theRequestPtr->fHtmlResponseLength = kChunkedOffset;
		}
		else {

			/*
				Mark the end of the buffer.
				First add a "<CR><LF>" to terminate the current chunk.
			*/
			theBufferPtr = theRequestPtr->fHtmlResponsePtr + 
					theChunkSize + kChunkedOffset;
			RP_STRCPY(theBufferPtr, kCRLF);	
			if (theRequestPtr->fHttpTransactionState == eRpHttpResponseComplete) {
				/*
					We have the last buffer, so put out a 0 length chunk at 
					the end of the buffer.
				*/
				RP_STRCAT(theBufferPtr, kLastChunk);	
			}
			else {
				/*
					This is an intermediate buffer, so adjust the response 
					length by the size of a last chunk.
				*/	
				theRequestPtr->fHtmlResponseLength -= kChunkedOffset;
			}
			/*
				Turn the chunk size into a hex string and place it at
				the beginning of the buffer.
			*/
			RpConvertUnsigned32ToHex(theChunkSize,
					theRequestPtr->fHtmlResponsePtr);
		}
	}
	return;
}

#endif	/* RomPagerHttpOneDotOne */


#if RomPagerHttpOneDotOne || WcHttpOneDotOne

/*
	This routine in only used for chunked encoding and will always put
	out three digits. The max chunk size supported is 4095, which means
	that the largest size of HTTP response buffer supported is limited to
	4096 or so. Default is 1450 and could only be enlarged if the underlying
	TCP layer supports fragmentation.
*/
void RpConvertUnsigned32ToHex(Unsigned32 theData,
											char *theBufferPtr) {
	char *			theCurrentPtr;
	Unsigned8		theDigitCount;
	Unsigned32 *	thePowersOf16Ptr;

	theCurrentPtr = theBufferPtr;
	if (theData == 0) {
		*theCurrentPtr++ = kAscii_0;
		*theCurrentPtr++ = kAscii_0;
		*theCurrentPtr++ = kAscii_0;
	} 
	else {
		/*
			The latest specification for HTTP/1.1 says leading zeroes 
			are ok for chunked encoding and will eliminate buffer offset 
			games, so just start at 256 and accept a leading zero 
			if it happens.
		*/
		thePowersOf16Ptr = gPowersOf16 + 5;
		/* pump out the digits */
		while (theData > 0) {
			theDigitCount = 0;
			while (theData >= *thePowersOf16Ptr) {
				theData -= *thePowersOf16Ptr;
				theDigitCount += 1;
			}
			*theCurrentPtr++ = NIBBLE_TO_HEX(theDigitCount & 0x0f);
			thePowersOf16Ptr += 1;
		}
		/* pump out any trailing zeros */
		while (*thePowersOf16Ptr > 0) {
			*theCurrentPtr++ = kAscii_0;
			thePowersOf16Ptr += 1;
		}
	}
	return;
}

#endif	/* RomPagerHttpOneDotOne || WcHttpOneDotOne */


void RpCatUnsigned32ToString(Unsigned32 theNumber, char *theStringPtr, 
		Unsigned8 theMinimumDigits) {
	Unsigned16		theCharsWritten;
	int				theDigitCount;
	char 			*theNumberStringPtr;
	Unsigned32Ptr	thePowerOf10Ptr;

	theNumberStringPtr = theStringPtr + RP_STRLEN(theStringPtr);
	theDigitCount = 1;
	thePowerOf10Ptr = 
		gPowersOf10 + (sizeof(gPowersOf10) / sizeof(Unsigned32) - 2);
	while (thePowerOf10Ptr >= gPowersOf10  && 
			theNumber >= *--thePowerOf10Ptr) {
		theDigitCount += 1;
	}
	while (theMinimumDigits > theDigitCount) {
		*theNumberStringPtr++ = kAscii_0;
		theDigitCount += 1;
	}
	*theNumberStringPtr = '\0';
	theCharsWritten = RpConvertUnsigned32ToAscii(theNumber, theNumberStringPtr);
	*(theNumberStringPtr + theCharsWritten) = '\0';
	return;
}


Unsigned32 RpGetRemoteIpAddress(void *theTaskDataPtr) {

	return ((rpDataPtr) theTaskDataPtr)->fCurrentConnectionPtr->fIpRemote;
}

Unsigned16 RpConvertUnsigned32ToAscii(Unsigned32 theData,
										char *theBufferPtr) {
	char * 			theCurrentPtr;
	Unsigned8		theDigitCount;
	Unsigned32 *	thePowersOf10Ptr;

	theCurrentPtr = theBufferPtr;
	if (theData == 0) {
		*theCurrentPtr++ = kAscii_0;
	} 
	else {
		thePowersOf10Ptr = gPowersOf10;
		
		/*
			Cycle to a useful power of 10.
		*/
		while (*thePowersOf10Ptr > theData) {
			thePowersOf10Ptr += 1;
		}
		
		/*
			Pump out the digits.
		*/
		while (theData > 0) {
			theDigitCount = 0;
			while (theData >= *thePowersOf10Ptr) {
				theData -= *thePowersOf10Ptr;
				theDigitCount += 1;
			}
			*theCurrentPtr++ = (char) (kAscii_0 + theDigitCount);
			thePowersOf10Ptr += 1;
		}
		
		/*
			Pump out any trailing zeros.
		*/
		while (*thePowersOf10Ptr > 0) {
			*theCurrentPtr++ = kAscii_0;
			thePowersOf10Ptr += 1;
		}
	}
	return (Unsigned16) (theCurrentPtr - theBufferPtr);
}


#if RomPagerServerPush || RomMailer

void RpBuildSeparator(rpDataPtr theDataPtr, char * theSeparatorPtr) {
	Unsigned32			theNumber;

	RpWriteIpAddressInDotForm(theSeparatorPtr,
			theDataPtr->fCurrentConnectionPtr->fIpLocal);
	RP_STRCAT(theSeparatorPtr, kAllegro);
	theNumber = RpGetSysTimeInSeconds(theDataPtr);
	RpCatUnsigned32ToString(theNumber, theSeparatorPtr, 0);
	return;
}

#endif	/* RomPagerServerPush || RomMailer */


#if AsVariableAccess || RomPagerServer || RomWebClient || RxConversions

void RpCatSigned32ToString(Signed32 theNumber, char *theStringPtr) {
	Unsigned16		theCharsWritten;
	char * 			theNumberStringPtr;

	theNumberStringPtr = theStringPtr + RP_STRLEN(theStringPtr);
	theCharsWritten = RpConvertSigned32ToAscii(theNumber, theNumberStringPtr);
	*(theNumberStringPtr + theCharsWritten) = '\0';
	return;
}


Unsigned16 RpConvertSigned32ToAscii(Signed32 theData, char *theBufferPtr) {
	Unsigned16		theCharacterCount;
	Unsigned16		theSignCount;

	if (theData < 0) {
		*theBufferPtr++ = '-';
		theData = -theData;
		theSignCount = 1;
	}
	else {
		theSignCount = 0;
	}
	theCharacterCount = RpConvertUnsigned32ToAscii((Unsigned32) theData, 
			theBufferPtr);
	return (Unsigned16) (theCharacterCount + theSignCount);
}

#endif	/* AsVariableAccess || RomPagerServer || RomWebClient || RxConversions */


#if AsUse64BitIntegers

Unsigned16 RpConvertUnsigned64ToAscii(Unsigned64 theData,
										char *theBufferPtr) {
	char * 			theCurrentPtr;
	Unsigned8		theDigitCount;
	Unsigned64Ptr	thePowersOf10Ptr;

	theCurrentPtr = theBufferPtr;
	if (theData == 0) {
		*theCurrentPtr++ = kAscii_0;
	} 
	else {
		thePowersOf10Ptr = g64BitPowersOf10;
		/* cycle to a useful power of 10 */
		while (*thePowersOf10Ptr > theData) {
			thePowersOf10Ptr += 1;
		}
		/* pump out the digits */
		while (theData > 0) {
			theDigitCount = 0;
			while (theData >= *thePowersOf10Ptr) {
				theData -= *thePowersOf10Ptr;
				theDigitCount += 1;
			}
			*theCurrentPtr++ = (char) (kAscii_0 + theDigitCount);
			thePowersOf10Ptr += 1;
		}
		/* pump out any trailing zeros */
		while (*thePowersOf10Ptr > 0) {
			*theCurrentPtr++ = kAscii_0;
			thePowersOf10Ptr += 1;
		}
	}
	return (Unsigned16) (theCurrentPtr - theBufferPtr);
}

Unsigned16 RpConvertSigned64ToAscii(Signed64 theData, char *theBufferPtr) {
	Unsigned16		theCharacterCount;
	Unsigned16		theSignCount;

	if (theData < 0) {
		*theBufferPtr++ = '-';
		theData = -theData;
		theSignCount = 1;
	}
	else {
		theSignCount = 0;
	}
	theCharacterCount = RpConvertUnsigned64ToAscii((Unsigned64) theData, 
			theBufferPtr);
	return (Unsigned16) (theCharacterCount + theSignCount);
}

#endif	/* AsUse64BitIntegers */


#if AsVariableAccess || RomPagerServer || RomPagerBasic || RomPop || RomWebClient || RxConversions
Unsigned8 RpHexToNibble(Boolean *theErrorFlag, char theHex) {
	Unsigned8 theNibble = 0;

	*theErrorFlag = False;
	if (theHex >= kAscii_0 && theHex <= kAscii_9) {
		theNibble = (Unsigned8) (theHex - kAscii_0);
	} 
	else if (theHex >= kAscii_A && theHex <= kAscii_F) {
		theNibble = (Unsigned8) (theHex - kAscii_A + 10);
	} 
	else if (theHex >= kAscii_a && theHex <= kAscii_f) {
		theNibble = (Unsigned8) (theHex - kAscii_a + 10);
	} 
	else {
		*theErrorFlag = True;
	}

	return theNibble;
}
#endif

#if RomPlugAdvanced
/*
	This routine takes a string of 8 hex characters and returns
	an Unsigned32 binary value that is equivalent. As long as the
	hex string was created using RpHexToString from an Unsigned32
	on the same machine, there are no byte order issues. The error 
	flag is set if there is a conversion problem.
*/
Unsigned32 RpHexToUnsigned32(Boolean *theErrorFlag, char *theHexPtr) {
	char 		theCharacterArray[4];
	char		*theCharacterPtr;
	Unsigned8	theCount;
	
	*theErrorFlag = False;
	theCount = 4;
	theCharacterPtr = theCharacterArray;
	while (theCount > 0) {
		*theCharacterPtr = RpHexToNibble(theErrorFlag, *theHexPtr++) << 4;
		if (*theErrorFlag) break;
		*theCharacterPtr++ |= RpHexToNibble(theErrorFlag, *theHexPtr++);
		if (*theErrorFlag) break;
		theCount--;
	}

	return *((Unsigned32 *) theCharacterArray);
}
#endif

#if (RomPagerServer && RomPagerSecurityDigest) || \
	(RomWebClient && WcDigestAuthentication) || \
	(RomMailer && RmAggregateHtml) || \
	RomPlugAdvanced || \
	(RomPop && PrUseApop)

void RpHexToString(unsigned char *theHexDataPtr, char *theStringPtr,
				Unsigned16 theLength) {

	while (theLength) {
		*theStringPtr++ = NIBBLE_TO_HEX(*theHexDataPtr >> 4);
		*theStringPtr++ = NIBBLE_TO_HEX(*theHexDataPtr & 0x0f);
		theHexDataPtr++;
		theLength--;
	}

	/*
		Make it a C string.
	*/
	*theStringPtr = '\0';

	return;
}
#endif

#if RomMailer || RomMailerBasic || RomPagerServer || RomPagerBasic || RomPop || RomPopBasic
void RpStrLenCpy(char *theToPtr, char *theFromPtr, Unsigned32 theLength) {
	/*
		This routine checks the length of the incoming string 
		before doing the copy. The output string will be
		either null or the input string.
	*/
	*theToPtr = '\0';
	if (RP_STRLEN(theFromPtr) < theLength) {
		RP_STRCPY(theToPtr, theFromPtr);
	}
#if AsDebug
	else {
		RP_PRINTF("\nString greater than %ld.\nString: %s\n", theLength, theFromPtr);
	}
#endif	
	return;
} 
#endif

#if RomMailer || RomPop || RomPagerIpp || RomCli || WcHttpCookies || \
		RomPagerServer || RomPagerBasic
void RpStrLenCpyTruncate(char *theToPtr, char *theFromPtr, Unsigned32 theLength) {
	Unsigned32		theCopySize;

	/*
		This routine copies as much of the incoming string as will 
		fit into the output string. Any excess is truncated.
	*/
	theCopySize = RP_STRLEN(theFromPtr);
	if (theCopySize >= theLength) {
		theCopySize = theLength - 1;
	}
	RP_MEMCPY(theToPtr, theFromPtr, theCopySize);
	*(theToPtr + theCopySize) = '\0';
	return;
}
#endif

#if RomPagerFullHostName || RomPlug

void RpBuildIpAddressHostName(rpConnectionPtr theConnectionPtr, char *theBufferPtr) {
	Unsigned16			theCharsWritten;
	Unsigned32			theNumber;
	char 				*theStringPtr;
	Boolean				theNeedPortFlag = False;
	
	/*
		Create a host name using the local IP address.
	*/
	theStringPtr = theBufferPtr + RP_STRLEN(theBufferPtr);
	RpWriteIpAddressInDotForm(theStringPtr, 
			theConnectionPtr->fIpLocal);
	theStringPtr = theBufferPtr + RP_STRLEN(theBufferPtr);

#if RomPagerSecure
	if ((theConnectionPtr->fIsTlsFlag && 
				theConnectionPtr->fLocalPort != kDefaultTlsPort) ||
		(!theConnectionPtr->fIsTlsFlag && 
				theConnectionPtr->fLocalPort != kDefaultHttpPort)) {
		theNeedPortFlag = True;
	}
#else
	if (theConnectionPtr->fLocalPort != kDefaultHttpPort) {
		theNeedPortFlag = True;
	}
#endif
	if (theNeedPortFlag) {
		*theStringPtr++ = ':';
		theNumber = theConnectionPtr->fLocalPort;
		theCharsWritten = 
				RpConvertUnsigned32ToAscii(theNumber, theStringPtr);
		theStringPtr += theCharsWritten;
	}

	*theStringPtr = '\0';
	return;
}

#endif /* RomPagerFullHostName || RomPlug */


#if RomPagerServerPush || RomPagerFullHostName || RomMailer || \
	RomMailerBasic || RomWebClient || RomPlug

void RpWriteIpAddressInDotForm(char *theBufferPtr, Unsigned32 theIpAddress) {
	Unsigned8		theCount;
	Unsigned8 *		theIpAddressPtr;

	theIpAddressPtr = (Unsigned8 *) &theIpAddress;
	for (theCount = 0; theCount < 4; theCount++) {
		theBufferPtr += RpConvertUnsigned32ToAscii(*theIpAddressPtr++,
				theBufferPtr);
		*theBufferPtr++ = '.';
	}
	*(theBufferPtr - 1) = '\0';
	return;
}

#endif


#if RomCli || RomWebClient || RomPagerSecure || RomWebClientSecure

/*
	These routines are the equivalent of the C library routines
	strcasecmp and strncasecmp that are included in some system libraries.
	They can be disabled if they are otherwise available.
*/
#if 1
Signed16 strcasecmp(const char *theFirstStringPtr, const char *theSecondStringPtr) {
	char 	theFirstCharacter;
	char	theSecondCharacter;

	while(*theFirstStringPtr != '\0') {
		theFirstCharacter = *theFirstStringPtr;
		if (theFirstCharacter >= 'A' && theFirstCharacter <= 'Z') {
			theFirstCharacter += ('a' - 'A');
		}
		theSecondCharacter = *theSecondStringPtr;
		if (theSecondCharacter >= 'A' && theSecondCharacter <= 'Z') {
			theSecondCharacter += ('a' - 'A');
		}
		if (theFirstCharacter > theSecondCharacter) {
			return 1;
		}
		if (theFirstCharacter < theSecondCharacter) {
			return -1;
		}
		theFirstStringPtr++;
		theSecondStringPtr++;
	}
	if (*theSecondStringPtr == '\0') {
		return 0;
	}
	return -1;
}

Signed16 strncasecmp(const char *theFirstStringPtr, 
				const char 		*theSecondStringPtr, 
				Unsigned32 		theCompareLength) {
	char 	theFirstCharacter;
	char	theSecondCharacter;

	while(*theFirstStringPtr != '\0' && theCompareLength > 0) {
		theFirstCharacter = *theFirstStringPtr;
		if (theFirstCharacter >= 'A' && theFirstCharacter <= 'Z') {
			theFirstCharacter += ('a' - 'A');
		}
		theSecondCharacter = *theSecondStringPtr;
		if (theSecondCharacter >= 'A' && theSecondCharacter <= 'Z') {
			theSecondCharacter += ('a' - 'A');
		}
		if (theFirstCharacter > theSecondCharacter) {
			return 1;
		}
		if (theFirstCharacter < theSecondCharacter) {
			return -1;
		}
		theFirstStringPtr++;
		theSecondStringPtr++;
		theCompareLength--;
	}
	return 0;
}
#endif
#endif	/* RomCli || RomWebClient || RomPagerSecure || RomWebClientSecure */
