/*
 *	File:		AsConvrt.c
 *
 *	Contains:	Allegro Software String to Integer conversion routines
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2003 by Allegro Software Development Corporation
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
 *		09/14/03	rhb		fix strict conversion compiler warnings
 *		07/08/03	bva		rework AsVariableAccess ifdefs
 *		06/26/03	nam		support float and double xml types
 * * * * Release 4.21  * * *
 * * * * Release 4.20  * * *
 *		11/26/02	rhb		use better conditionals
 *		11/05/02	rhb		support enum types
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *		07/26/02	bva		change conditional on AsCheckAsciiString
 * * * * Release 4.10  * * *
 *		04/11/02	rhb		fix buffer overflow in AsSendDataOutDecimalSigned
 *		04/10/02	bva		fix compile warnings for AsCheckAsciiString
 * * * * Release 4.07  * * *
 *		03/22/02	pjr		prevent hang if both response buffers overflow
 *		03/22/02	rhb		add AsCheckAsciiString
 *		02/05/02	rhb		dotform strings don't have to be exact length
 *		10/16/01	rhb		fix compiler warnings
 * * * * Release 4.06  * * *
 * * * * Release 4.03  * * *
 *		09/28/01	bva		remove AsProtos.h include
 * * * * Release 4.01  * * *
 *		08/07/01	pjr		fix signed 64 string to integer conversion
 * * * * Release 4.00  * * *
 *		07/26/01	pjr		fix compile warning
 *		03/16/01	rhb		move SendDataOutDotForm and SendDataOutHex from 
 *								AsVarAcc.c and rename to As*
 *		02/15/01	rhb		rename kRpHexSeparator to kAsHexSeparator
 *		02/14/01	rhb		rename rpItemError and eRpItemError_* to 
 *								asItemError and eAsItemError_*
 *		02/13/01	rhb		rename RomPagerUse64BitIntegers to AsUse64BitIntegers
 *		12/15/00	pjr		rework conditionals
 *		10/27/00	pjr		initial version
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if AsVariableAccess || RomPagerServer

asItemError AsCheckAsciiString(char *theStringPtr, Unsigned16 theLength) {
	asItemError		theItemError;

	theItemError = eAsItemError_NoError;
	while (theLength > 0) {
		if (*theStringPtr++ & 0x80) {
			theItemError = eAsItemError_UnexpectedExtendedCharacter;
		}
		theLength--;
	}
	
	return theItemError;
}

#endif	/* AsVariableAccess || RomPagerServer */


#if AsVariableAccess || RomPagerServer || RxConversions

/*
	This routine converts an ASCII dot form string into an array of
	hex bytes.

	Inputs:
		theDotFormPtr		- pointer to the ASCII dot form string
							  to be converted
		theHexPtr			- pointer to the hex byte array for the
							  conversion result
		theOutputCharCount	- size of output byte array

	Returns:
		asItemError:
			eAsItemError_NoError
							- no error, conversion was successful
			eAsItemError_ExpectingADecimalDigit
							- a character was encountered that was not
							  a number or a dot
			eAsItemError_Unsigned8TooLarge
							- a value was too large to fit into a byte
*/

asItemError AsDotFormStrToHex(char *theDotFormPtr, char *theHexPtr,
								Unsigned8 theOutputByteCount) {
	Unsigned8			theCharCount;
	char				theChars[5];
	char *				theCharsPtr;
	Boolean				theDoneFlag;
	asItemError			theItemError;

	theItemError = eAsItemError_NoError;

	while (theOutputByteCount > 0 && *theDotFormPtr != '\0' && 
			theItemError == eAsItemError_NoError) {
		theCharsPtr = theChars;
		theCharCount = 5;
		theDoneFlag = False;
		while (!theDoneFlag) {
			*theCharsPtr = *theDotFormPtr;
			theCharCount--;
			if (*theDotFormPtr == kAscii_Dot) {
				*theCharsPtr = '\0';
				theDotFormPtr++;
				theDoneFlag = True;
			}
			else if (*theDotFormPtr == '\0' || theCharCount == 0) {
				theDoneFlag = True;
			}
			else {
				theCharsPtr++;
				theDotFormPtr++;
			}
		}
		if (theCharCount == 0) {
			theItemError = eAsItemError_TooManyCharacters;
		}
		else {
			theItemError = AsStrToUnsigned8(theChars,
									(Unsigned8Ptr) theHexPtr);
			theHexPtr++;

			if (theItemError == eAsItemError_NoError) {
				theOutputByteCount--;
			}
		}
	}

	return theItemError;
}


/*
	This routine converts an ASCII hex colon form string into an array of
	hex bytes.  An example of one of these strings would be:
	<78:9a:bc:de>.  The hex separator character does not have to be a
	colon and is defined by theSeparator parameter.

	Inputs:
		theStringPtr		- pointer to the ASCII colon form string
							  to be converted
		theSeparator		- the hex separator character
		theHexPtr			- pointer to the hex byte array for the
							  conversion result
		theOutputCharCount	- size of output byte array

	Returns:
		asItemError:
			eAsItemError_NoError
							- no error, conversion was successful
			eAsItemError_ExpectingADecimalDigit
							- a character was encountered that was not
							  a number or a dot
			eAsItemError_Unsigned8TooLarge
							- a value was too large to fit into a byte
			eAsItemError_ExpectingAColonInHexColonForm
							- incorrect character encountered when
							  expecting separator character
*/

typedef enum {
	eRpReadHex_GetFirstHexByte,
	eRpReadHex_GetSecondHexByte,
	eRpReadHex_GetHexSeparator
} rpReadHex;

asItemError AsStrToHex(char *theStringPtr, char theSeparator,
						char *theHexPtr, Unsigned8 theOutputByteCount) {
	Unsigned8			theByte;
	Boolean				theErrorFlag;
	rpReadHex			theHexState;
	asItemError			theItemError;
#if kRpHexShiftRight
	char *				theCharPtr;
	int					theCharCount;
#endif

	theItemError = eAsItemError_NoError;
	theHexState = eRpReadHex_GetFirstHexByte;
	theByte = 0;
	RP_MEMSET(theHexPtr, 0, theOutputByteCount);

#if kRpHexShiftRight
	theCharPtr = theStringPtr;
	theCharCount = 0;
	while (*theCharPtr++ != '\0') {
		theCharCount++;
	}
	if (theSeparator == kAsHexSeparator) {
		theCharCount = (theCharCount + 3) / 3;
	}
	else {
		if ((theCharCount % 2) != 0) {
			theHexState = eRpReadHex_GetSecondHexByte;
		}
		theCharCount = (theCharCount + 1) / 2;
	}
	theHexPtr += theOutputByteCount - theCharCount;
#endif

	while (theItemError == eAsItemError_NoError &&
			*theStringPtr != '\0' &&
			theOutputByteCount > 0) {
		switch (theHexState) {
			case eRpReadHex_GetFirstHexByte:
				theByte = (Unsigned8)
						(RpHexToNibble(&theErrorFlag, *theStringPtr) << 4);
				if (theErrorFlag) {
					theItemError = eAsItemError_IllegalHexCharacter;
				}
				theHexState = eRpReadHex_GetSecondHexByte;
				break;

			case eRpReadHex_GetSecondHexByte:
				theByte |= RpHexToNibble(&theErrorFlag, *theStringPtr);
				if (theErrorFlag) {
					theItemError = eAsItemError_IllegalHexCharacter;
				}
				*theHexPtr++ = (char) theByte;
				theOutputByteCount--;
				if (theSeparator == kAsHexSeparator) {
					theHexState = eRpReadHex_GetHexSeparator;
				}
				else {
					theHexState = eRpReadHex_GetFirstHexByte;
				}
				break;

			case eRpReadHex_GetHexSeparator:
				if (*theStringPtr != kAsHexSeparator) {
					theItemError = eAsItemError_ExpectingAColonInHexColonForm;
				}
				else {
					theHexState = eRpReadHex_GetFirstHexByte;
				}
				break;
		}
		theStringPtr++;
	}

	/* Store the last byte if it had single nibble */
	if (theHexState == eRpReadHex_GetSecondHexByte) {
		*theHexPtr = (char) theByte;
	}

	return theItemError;
}


asItemError AsStrToSigned8(char *theStringPtr, Signed8Ptr theSigned8Ptr) {
	asItemError		theItemError;
	Signed32		theSigned32;

	theItemError = AsStrToSigned32(theStringPtr, &theSigned32);

	if (theItemError == eAsItemError_NoError) {
		if (theSigned32 < 0) {
			if (theSigned32 < -(kSigned8_Max + 1)) {
				theItemError = eAsItemError_Signed8TooSmall;
			}
		}
		else if (theSigned32 > kSigned8_Max) {
			theItemError = eAsItemError_Signed8TooLarge;
		}
	}
	else if (theItemError == eAsItemError_Signed32TooSmall) {
		theItemError = eAsItemError_Signed8TooSmall;
	}
	else if (theItemError == eAsItemError_Signed32TooLarge) {
		theItemError = eAsItemError_Signed8TooLarge;
	}

	*theSigned8Ptr = (Signed8) theSigned32;
	return theItemError;
}


asItemError AsStrToSigned16(char *theStringPtr,
							Signed16Ptr theSigned16Ptr) {
	asItemError		theItemError;
	Signed32		theSigned32;

	theItemError = AsStrToSigned32(theStringPtr, &theSigned32);

	if (theItemError == eAsItemError_NoError) {
		if (theSigned32 < 0) {
			if (theSigned32 < -(kSigned16_Max + 1)) {
				theItemError = eAsItemError_Signed16TooSmall;
			}
		}
		else if (theSigned32 > kSigned16_Max) {
			theItemError = eAsItemError_Signed16TooLarge;
		}
	}
	else if (theItemError == eAsItemError_Signed32TooSmall) {
		theItemError = eAsItemError_Signed16TooSmall;
	}
	else if (theItemError == eAsItemError_Signed32TooLarge) {
		theItemError = eAsItemError_Signed16TooLarge;
	}

	*theSigned16Ptr = (Signed16) theSigned32;
	return theItemError;
}


asItemError AsStrToSigned32(char *theStringPtr, Signed32Ptr theSigned32Ptr) {
	asItemError			theItemError;
	Boolean				theMinusFlag;
	Signed32			theSigned32;
	Unsigned32			theUnsigned32;

	theSigned32 = 0;

	if (*theStringPtr == kAscii_Hyphen) {
		theStringPtr++;
		theMinusFlag = True;
	}
	else {
		theMinusFlag = False;
	}

	theItemError = AsStrToUnsigned32(theStringPtr, &theUnsigned32);

	if (theItemError == eAsItemError_NoError) {
		if (theMinusFlag) {
			if (theUnsigned32 > kSigned32_Max + 1) {
				theItemError = eAsItemError_Signed32TooSmall;
			}
			else {
				theSigned32 = - (Signed32) theUnsigned32;
			}
		}
		else {
			if (theUnsigned32 > kSigned32_Max) {
				theItemError = eAsItemError_Signed32TooLarge;
			}
			else {
				theSigned32 = (Signed32) theUnsigned32;
			}
		}
	}
	else if (theItemError == eAsItemError_Unsigned32TooLarge) {
		theItemError = eAsItemError_Signed32TooLarge;
	}

	*theSigned32Ptr = theSigned32;
	return theItemError;
}


asItemError AsStrToUnsigned8(char *theStringPtr,
								Unsigned8Ptr theUnsigned8Ptr) {
	asItemError		theItemError;
	Unsigned32		theUnsigned32;

	theItemError = AsStrToUnsigned32(theStringPtr, &theUnsigned32);

	if (theItemError == eAsItemError_Unsigned32TooLarge ||
			(theItemError == eAsItemError_NoError &&
			theUnsigned32 > kUnsigned8_Max)) {
		theItemError = eAsItemError_Unsigned8TooLarge;
	}

	*theUnsigned8Ptr = (Unsigned8) theUnsigned32;
	return theItemError;
}


asItemError AsStrToUnsigned16(char *theStringPtr,
								Unsigned16Ptr theUnsigned16Ptr) {
	asItemError		theItemError;
	Unsigned32		theUnsigned32;

	theItemError = AsStrToUnsigned32(theStringPtr, &theUnsigned32);

	if (theItemError == eAsItemError_Unsigned32TooLarge ||
			(theItemError == eAsItemError_NoError &&
			theUnsigned32 > kUnsigned16_Max)) {
		theItemError = eAsItemError_Unsigned16TooLarge;
	}

	*theUnsigned16Ptr = (Unsigned16) theUnsigned32;
	return theItemError;
}


asItemError AsStrToUnsigned32(char *theStringPtr,
								Unsigned32Ptr theUnsigned32Ptr) {
	char				theChar;
	asItemError			theItemError;
	Unsigned32			theUnsigned32;

	theUnsigned32 = 0;

	if (*theStringPtr == '\0') {
		theItemError = eAsItemError_ExpectingAnInteger;
	}
	else {
		theItemError = eAsItemError_NoError;
		do {
			theChar = *theStringPtr++;
			if (theChar < kAscii_0 || theChar > kAscii_9) {
				theItemError = eAsItemError_ExpectingADecimalDigit;
			}
			else if (theUnsigned32 > kUnsigned32_Max / 10) {
				theItemError = eAsItemError_Unsigned32TooLarge;
			}
			else {
				theUnsigned32 *= 10;
				theChar -= kAscii_0;
				if (kUnsigned32_Max - theUnsigned32 < (Unsigned32) theChar) {
					theItemError = eAsItemError_Unsigned32TooLarge;
				}
				else {
					theUnsigned32 += theChar;
				}
			}
		} while (*theStringPtr != '\0' && theItemError == eAsItemError_NoError);
	}

	*theUnsigned32Ptr = theUnsigned32;
	return theItemError;
}


#if AsUse64BitIntegers

asItemError AsStrToSigned64(char *theStringPtr, Signed64Ptr theSigned64Ptr) {
	asItemError			theItemError;
	Boolean				theMinusFlag;
	Signed64			theSigned64;
	Unsigned64			theUnsigned64;

	theSigned64 = 0;

	if (*theStringPtr == kAscii_Hyphen) {
		theStringPtr++;
		theMinusFlag = True;
	}
	else {
		theMinusFlag = False;
	}

	theItemError = AsStrToUnsigned64(theStringPtr, &theUnsigned64);

	if (theItemError == eAsItemError_NoError) {
		if (theMinusFlag) {
			if (theUnsigned64 > kSigned64_Max + 1) {
				theItemError = eAsItemError_Signed64TooSmall;
			}
			else {
				theSigned64 = -(Signed64) theUnsigned64;
			}
		}
		else {
			if (theUnsigned64 > kSigned64_Max) {
				theItemError = eAsItemError_Signed64TooLarge;
			}
			else {
				theSigned64 = (Signed64) theUnsigned64;
			}
		}
	}
	else if (theItemError == eAsItemError_Unsigned64TooLarge) {
		theItemError = eAsItemError_Signed64TooLarge;
	}

	*theSigned64Ptr = theSigned64;
	return theItemError;
}


asItemError AsStrToUnsigned64(char *theStringPtr,
								Unsigned64Ptr theUnsigned64Ptr) {
	char				theChar;
	asItemError			theItemError;
	Unsigned64			theUnsigned64;

	theUnsigned64 = 0;

	if (*theStringPtr == '\0') {
		theItemError = eAsItemError_ExpectingAnInteger;
	}
	else {
		theItemError = eAsItemError_NoError;
		do {
			theChar = *theStringPtr++;
			if (theChar < kAscii_0 || theChar > kAscii_9) {
				theItemError = eAsItemError_ExpectingADecimalDigit;
			}
			else if (theUnsigned64 > kUnsigned64_Max / 10) {
				theItemError = eAsItemError_Unsigned64TooLarge;
			}
			else {
				theUnsigned64 *= 10;
				theChar -= kAscii_0;
				if (kUnsigned64_Max - theUnsigned64 < (Unsigned64) theChar) {
					theItemError = eAsItemError_Unsigned64TooLarge;
				}
				else {
					theUnsigned64 += theChar;
				}
			}
		} while (*theStringPtr != '\0' && theItemError == eAsItemError_NoError);
	}

	*theUnsigned64Ptr = theUnsigned64;
	return theItemError;
}

#endif	/* AsUse64BitIntegers */

#endif	/* AsVariableAccess || RomPagerServer || RxConversions */

#if AsVariableAccess || RxConversions 

#if AsUseEnums

asItemError AsStrToEnum8(char 			*theStringPtr, 
					asEnumElementPtr 	theEnumTablePtr, 
					Unsigned8Ptr 		theValuePtr) {
	asItemError	theItemError;

	while (theEnumTablePtr->fStringPtr != (char *) 0 && 
			RP_STRCMP(theEnumTablePtr->fStringPtr, theStringPtr) != 0) {
		theEnumTablePtr += 1;
	}
	if (theEnumTablePtr->fStringPtr == (char *) 0) {
		theItemError = eAsItemError_NoSuchEnumString;
	}
	else {
		*theValuePtr = (Unsigned8) theEnumTablePtr->fValue;
		theItemError = eAsItemError_NoError;
	}
	return theItemError;
}


asItemError AsStrToEnum16(char 			*theStringPtr, 
					asEnumElementPtr 	theEnumTablePtr, 
					Unsigned16Ptr 		theValuePtr) {
	asItemError	theItemError;

	while (theEnumTablePtr->fStringPtr != (char *) 0 && 
			RP_STRCMP(theEnumTablePtr->fStringPtr, theStringPtr) != 0) {
		theEnumTablePtr += 1;
	}
	if (theEnumTablePtr->fStringPtr == (char *) 0) {
		theItemError = eAsItemError_NoSuchEnumString;
	}
	else {
		*theValuePtr = (Unsigned16) theEnumTablePtr->fValue;
		theItemError = eAsItemError_NoError;
	}
	return theItemError;
}


asItemError AsStrToEnum32(char 			*theStringPtr, 
					asEnumElementPtr 	theEnumTablePtr, 
					Unsigned32Ptr 		theValuePtr) {
	asItemError	theItemError;

	while (theEnumTablePtr->fStringPtr != (char *) 0 && 
			RP_STRCMP(theEnumTablePtr->fStringPtr, theStringPtr) != 0) {
		theEnumTablePtr += 1;
	}
	if (theEnumTablePtr->fStringPtr == (char *) 0) {
		theItemError = eAsItemError_NoSuchEnumString;
	}
	else {
		*theValuePtr = (Unsigned32) theEnumTablePtr->fValue;
		theItemError = eAsItemError_NoError;
	}
	return theItemError;
}

#endif	/* AsUseEnums */


/*****************************************************************************
	Output conversion routines.
*****************************************************************************/




void AsSetupChunkedBuffer(asResponsePtr theResponsePtr) {

#if 0
	/*
		PJR!!!???

		Until these routines are used for the RomPager Server,
		the responses will not be chunked.
	*/
#endif
	return;
}


void AsCloseChunkedBuffer(asResponsePtr theResponsePtr) {

#if 0
	/*
		PJR!!!???

		Until these routines are used for the RomPager Server,
		the responses will not be chunked.
	*/

#endif

	return;
}


/*
	This routine is called to send out a Signed8, Signed16, or Signed32
	hex value.  The value is converted to an ASCII string and placed
	in the buffers pointed to by theResponsePtr.

	If there is any question whether or not the ASCII output will fit
	into the buffers supplied in theResponsePtr, the caller should check
	the fOverflowError field of theResponsePtr when this routine returns.
*/

void AsSendDataOutDecimalSigned(asResponsePtr theResponsePtr,
								Signed32 theData) {
// +++ _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#if 1
	//~~modified by zlong, 7/11/2002. modified "Unsigned16 theCharsWritten" to "Unsigned32 theCharsWritten"
	Unsigned32      theCharsWritten; 
#else
	Unsigned16		theCharsWritten;
#endif
// --- _Alphanetworks_Patch_, 11/28/2003, jacob_shih
	char			theValue[12];

	theCharsWritten = RpConvertSigned32ToAscii(theData, theValue);

	if (theResponsePtr->fFillBufferAvailable < (theCharsWritten + 1)) {
		AsFlipResponseBuffers(theResponsePtr);
	}

	if (!theResponsePtr->fOverflowError) {
		RP_MEMCPY(theResponsePtr->fFillPtr, theValue, theCharsWritten);
		theResponsePtr->fFillPtr += theCharsWritten;
		theResponsePtr->fFillBufferAvailable -= theCharsWritten;
	}

	return;
}


void AsSendDataOutDotForm(asResponsePtr theResponsePtr, 
		Unsigned8 theLength, char *theHexDataPtr) {

	while (theLength > 0) {
		AsSendDataOutDecimalUnsigned(theResponsePtr,
				(Unsigned32) (Unsigned8) *theHexDataPtr++);
		theLength--;
		if (theLength > 0) {
			if (theResponsePtr->fFillBufferAvailable < 1) {
				AsFlipResponseBuffers(theResponsePtr);
			}

			if (theResponsePtr->fOverflowError) {
				break;
			}
			else {
				*theResponsePtr->fFillPtr++ = '.';
				theResponsePtr->fFillBufferAvailable--;
			}
		}
	}

	return;
}


void AsSendDataOutHex(asResponsePtr theResponsePtr, Unsigned8 theLength,
		char theSeparator, char *theHexPtr) {
	Unsigned8	theByte;

	while (theLength > 0 && !theResponsePtr->fOverflowError) {
		if (theResponsePtr->fFillBufferAvailable >= 3) {
			theByte = (Unsigned8) *theHexPtr++;
			*theResponsePtr->fFillPtr++ =
					(char) NIBBLE_TO_HEX((theByte >> 4) & 0x0f);
			*theResponsePtr->fFillPtr++ =
					(char) NIBBLE_TO_HEX(theByte & 0x0f);
			theResponsePtr->fFillBufferAvailable -= 2;
			theLength -= 1;
			if (theSeparator != '\0' && theLength > 0) {
				*theResponsePtr->fFillPtr++ = theSeparator;
				theResponsePtr->fFillBufferAvailable -= 1;
			}
		}
		else {
			AsFlipResponseBuffers(theResponsePtr);
		}
	}

	return;
}


#if AsUse64BitIntegers

void AsSendDataOutDecimalSigned64(asResponsePtr theResponsePtr,
		Signed64 theData) {
	Unsigned16	theCharsWritten;

	if (theResponsePtr->fFillBufferAvailable < 21) {
		AsFlipResponseBuffers(theResponsePtr);
	}

	if (!theResponsePtr->fOverflowError) {
		theCharsWritten = RpConvertSigned64ToAscii(theData,
									theResponsePtr->fFillPtr);
		theResponsePtr->fFillPtr += theCharsWritten;
		theResponsePtr->fFillBufferAvailable -= theCharsWritten;
	}

	return;
}


void AsSendDataOutDecimalUnsigned64(asResponsePtr theResponsePtr,
		Unsigned64 theData) {
	Unsigned16	theCharsWritten;

	if (theResponsePtr->fFillBufferAvailable < 20) {
		AsFlipResponseBuffers(theResponsePtr);
	}

	if (!theResponsePtr->fOverflowError) {
		theCharsWritten = RpConvertUnsigned64ToAscii(theData,
									theResponsePtr->fFillPtr);
		theResponsePtr->fFillPtr += theCharsWritten;
		theResponsePtr->fFillBufferAvailable -= theCharsWritten;
	}

	return;
}

#endif	/* AsUse64BitIntegers */

#if AsUseFloatingPoint

void AsSendDataOutFloat(asResponsePtr theResponsePtr,
		double theData) {
	Unsigned16	theCharsWritten;

	if (theResponsePtr->fFillBufferAvailable < 20) {
		AsFlipResponseBuffers(theResponsePtr);
	}
	if (!theResponsePtr->fOverflowError) {
		theCharsWritten = AsFloatToString(theData,
				theResponsePtr->fFillPtr);
		theResponsePtr->fFillPtr += theCharsWritten;
		theResponsePtr->fFillBufferAvailable -= theCharsWritten;
	}
	return;
}

#endif	/* AsUseFloatingPoint */

#if AsUseEnums

asItemError AsSendDataOutEnum(asResponsePtr theResponsePtr, 
						asEnumElementPtr 	theEnumTablePtr, 
						Unsigned32 			theValue) {
	asItemError	theItemError;
	size_t		theStringLength;

	while (theEnumTablePtr->fStringPtr != (char *) 0 && 
			theEnumTablePtr->fValue != theValue) {
		theEnumTablePtr += 1;
	}
	if (theEnumTablePtr->fStringPtr == (char *) 0) {
		theItemError = eAsItemError_NoSuchEnumValue;
	}
	else {
		theStringLength = RP_STRLEN(theEnumTablePtr->fStringPtr);
		if (theResponsePtr->fFillBufferAvailable < theStringLength) {
			AsFlipResponseBuffers(theResponsePtr);
		}

		if (!theResponsePtr->fOverflowError && 
				theResponsePtr->fFillBufferAvailable >= theStringLength) {
			RP_MEMCPY(theResponsePtr->fFillPtr, theEnumTablePtr->fStringPtr, 
					theStringLength);
			theResponsePtr->fFillPtr += theStringLength;
			theResponsePtr->fFillBufferAvailable -= theStringLength;
			theItemError = eAsItemError_NoError;
		}
		else {
			theItemError = eAsItemError_TooManyCharacters;
		}
	}
	return theItemError;
}
#endif	/* AsUseEnums */

#endif	/* AsVariableAccess || RxConversions */


#if AsVariableAccess || RomXml

/*
	This routine is called to initialize a response control structure.

	Inputs:
		theResponsePtr		- structure to initialize
		theBufferOnePtr		- pointer to first response buffer (required)
		theBufferTwoPtr		- pointer to second response buffer (optional)
		theBufferSize		- response buffer size (if two buffers are used,
							  they must be the same size)
		theChunkedFlag		- flag indicating that the response is chunked

	Returns:
		none
*/

// +++ _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#if 1
//~~modified by zlong, 7/11/2002. modified "Unsigned16 theBufferSize" to "Unsigned32 theBufferSize"
void AsInitializeResponse(rpDataPtr theDataPtr, asResponsePtr theResponsePtr,
							char *theBufferOnePtr, char *theBufferTwoPtr,
							Unsigned32 theBufferSize, Boolean theChunkedFlag) {
#else
void AsInitializeResponse(rpDataPtr theDataPtr, asResponsePtr theResponsePtr,
							char *theBufferOnePtr, char *theBufferTwoPtr,
							Unsigned16 theBufferSize, Boolean theChunkedFlag) {
#endif
// --- _Alphanetworks_Patch_, 11/28/2003, jacob_shih

	RP_MEMSET(theResponsePtr, 0, sizeof(asResponse));
	theResponsePtr->fDataPtr = theDataPtr;
	theResponsePtr->fCurrentFillBuffer = 1;
	theResponsePtr->fBufferOnePtr = theBufferOnePtr;
	theResponsePtr->fBufferTwoPtr = theBufferTwoPtr;
	theResponsePtr->fFillPtr = theBufferOnePtr;
	theResponsePtr->fResponsePtr = theBufferOnePtr;
	theResponsePtr->fBufferSize = theBufferSize;
	theResponsePtr->fFillBufferAvailable = theBufferSize;
	theResponsePtr->fResponseIsChunked = theChunkedFlag;

	return;
}
#endif  /* AsVariableAccess || RomXml */

#if AsVariableAccess || RxConversions || RxExplicitIndexFraming
void AsFlipResponseBuffers(asResponsePtr theResponsePtr) {

	if (theResponsePtr->fBufferReadyToSend == False) {
		theResponsePtr->fBufferReadyToSend = True;
		if (theResponsePtr->fCurrentFillBuffer == 1) {
			theResponsePtr->fResponsePtr =
					theResponsePtr->fBufferOnePtr;
			if (theResponsePtr->fBufferTwoPtr == (char *) 0) {
				theResponsePtr->fOverflowError = True;
			}
			else {
				theResponsePtr->fFillPtr =
						theResponsePtr->fBufferTwoPtr;
				theResponsePtr->fCurrentFillBuffer = 2;
			}
		}
		else {
			theResponsePtr->fResponsePtr =
					theResponsePtr->fBufferTwoPtr;
			theResponsePtr->fFillPtr =
					theResponsePtr->fBufferOnePtr;
			theResponsePtr->fCurrentFillBuffer = 1;
		}
		theResponsePtr->fResponseLength =
				(Unsigned16) (theResponsePtr->fBufferSize -
				theResponsePtr->fFillBufferAvailable);
		theResponsePtr->fFillBufferAvailable = theResponsePtr->fBufferSize;
#if RomPagerHttpOneDotOne
		AsCloseChunkedBuffer(theResponsePtr);
		AsSetupChunkedBuffer(theResponsePtr);
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
		theResponsePtr->fBufferReadyToSend = False;
		theResponsePtr->fCurrentFillBuffer = 1;
		theResponsePtr->fFillBufferAvailable = theResponsePtr->fBufferSize;
		theResponsePtr->fFillPtr = theResponsePtr->fBufferOnePtr;
		theResponsePtr->fResponsePtr = theResponsePtr->fBufferOnePtr;
		theResponsePtr->fResponseLength = 0;
#if RomPagerHttpOneDotOne
		AsSetupChunkedBuffer(theResponsePtr);
#endif
#if AsDebug
		RP_PRINTF("AsFlipResponseBuffers, buffer overflow error!\n");
#endif	
	}

	return;
}


/*
	This routine is called to send out an Unsigned8, Unsigned16, or
	Unsigned32 hex value.  The value is converted to an ASCII string
	and placed in the buffers pointed to by theResponsePtr.

	If there is any question whether or not the ASCII output will fit
	into the buffers supplied in theResponsePtr, the caller should check
	the fOverflowError field of theResponsePtr when this routine returns.
*/

void AsSendDataOutDecimalUnsigned(asResponsePtr theResponsePtr,
									Unsigned32 theData) {
// +++ _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#if 1
	//~~modified by zlong, 7/11/2002. modified "Unsigned16 theCharsWritten" to "Unsigned32 theCharsWritten"
	Unsigned32		theCharsWritten;
#else
	Unsigned16		theCharsWritten;
#endif
// --- _Alphanetworks_Patch_, 11/28/2003, jacob_shih
	char			theValue[12];

	theCharsWritten = RpConvertUnsigned32ToAscii(theData, theValue);

	if (theResponsePtr->fFillBufferAvailable < theCharsWritten ) {
		AsFlipResponseBuffers(theResponsePtr);
	}

	if (!theResponsePtr->fOverflowError) {
		RP_MEMCPY(theResponsePtr->fFillPtr, theValue, theCharsWritten);
		theResponsePtr->fFillPtr += theCharsWritten;
		theResponsePtr->fFillBufferAvailable -= theCharsWritten;
	}

	return;
}
#endif	/* AsVariableAccess || RxConversions || RxExplicitIndexFraming */


