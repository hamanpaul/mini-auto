/*
 *	File:		RpFrmItm.c
 *
 *	Contains:	Routines for Forms item handling
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
 * * * * Release 4.20  * * *
 *		12/02/02	rhb		decode form items into original buffer
 * * * * Release 4.12  * * *
 * * * * Release 4.00  * * *
 *		02/09/00	bva		use AsEngine.h
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 * * * * Release 3.06  * * *
 * * * * Release 3.02  * * *
 *		04/30/99	bva		add debug length checking to RpGetFormItem
 * * * * Release 3.0 * * * *
 *		01/20/99	pjr		change RpHexToNibble usage
 *		01/08/99	pjr		create from routines in RpForm.c
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if RomPagerServer || RomPagerBasic

void RpGetFormItem(char 	**theBufferPtr,
					char 	*theNamePtr, 
					char 	*theValuePtr) {
	char		theTerminator;
	char *		theTerminatorPtr;
	char *		theTokenPtr;

	/*
		Find the end of the Form Item Name and null terminate it. We save the 
		old termination in case the data is malformed and there is no '='.
	*/
	theTokenPtr = *theBufferPtr;
	theTerminatorPtr = theTokenPtr + 
			RpFindTokenDelimited(*theBufferPtr, kAscii_Equal);
	theTerminator = *theTerminatorPtr;
	*theTerminatorPtr = '\0';

	/*
		Decode the Form Item Name, make sure it will 
		fit into the Name buffer, and then copy it.
	*/
	RpEscapeDecodeString(theTokenPtr, theTokenPtr);
	if (RP_STRLEN(theTokenPtr) >= kMaxNameLength) {
		*(theTokenPtr + kMaxNameLength - 1) = '\0';
#if RomPagerDebug
		RP_PRINTF("RpGetFormItem, Name Item too big!\n");
#endif
	}
	RP_STRCPY(theNamePtr, theTokenPtr);

	/*
		If the Name was terminated by an '=' bump the pointer to 
		start looking for the Form Item Value token. If the Name 
		was terminated by a null, keep the pointer pointing to 
		the null and process the "null" Form Item Value token.
	*/
	if (theTerminator == kAscii_Equal) {
		theTerminatorPtr += 1;
	}

	/*
		Update the input pointer.
	*/
	theTokenPtr = theTerminatorPtr;

	/*
		Find the end of the Form Item Value and null terminate it. 
	*/
	theTerminatorPtr = theTokenPtr + 
			RpFindTokenDelimited(theTokenPtr, kAscii_Ampersand);
	theTerminator = *theTerminatorPtr;
	*theTerminatorPtr = '\0';

#if RomPagerServer && RpHtmlTextAreaBuf

	/*
		The Form Item Value has to accommodate a value of 
		any size. Of course, the value cannot be bigger 
		than kHttpWorkSize, so the buffer is that size.
	*/
	RpEscapeDecodeString(theTokenPtr, theValuePtr);

#else

	/*
		Decode the Form Item Value, make sure it will 
		fit into the Value buffer, and then copy it.
	*/
	RpEscapeDecodeString(theTokenPtr, theTokenPtr);
	if (RP_STRLEN(theTokenPtr) >= kMaxValueLength) {
		*(theTokenPtr + kMaxValueLength - 1) = '\0';
#if RomPagerDebug
		RP_PRINTF("RpGetFormItem, Form Item too big!\n");
#endif
	}
	RP_STRCPY(theValuePtr, theTokenPtr);

#endif	/* RomPagerServer && RpHtmlTextAreaBuf */

	/*
		If the Value was terminated by an '&' bump the pointer 
		to return a pointer to the next Name=Value pair. If 
		the Value was terminated by a null, keep the pointer 
		pointing to the null--there is no next Name=Value pair.
	*/
	if (theTerminator == kAscii_Ampersand) {
		theTerminatorPtr += 1;
	}

	/*
		Update the input pointer.
	*/
	*theBufferPtr = theTerminatorPtr;
	return;
}


void RpEscapeDecodeString(char * theEncodedStringPtr, 
							char * theDecodedStringPtr) {
	char				theCurrentCharacter;
	Boolean				theErrorFlag;
	rpHexEscapeState	theHexEscapeState;
	
	theHexEscapeState = eRpHexEscapeState_LookForPercent;
	theCurrentCharacter = *theEncodedStringPtr++;
	while (theCurrentCharacter != '\0') {
		switch(theHexEscapeState) {
			case eRpHexEscapeState_LookForPercent:
				if (theCurrentCharacter == kAscii_Percent) {
					/*
						special characters are encoded as '%xx' 
						where x is a hex digit.
					*/
					theHexEscapeState = eRpHexEscapeState_GetFirstHexByte;
				}
				else if (theCurrentCharacter == kAscii_Plus) {
					*theDecodedStringPtr++ = ' ';
				}
				else {
					*theDecodedStringPtr++ = theCurrentCharacter;
				}
				break;
	
			case eRpHexEscapeState_GetFirstHexByte:
				*theDecodedStringPtr = RpHexToNibble(&theErrorFlag,
												theCurrentCharacter) << 4;
				theHexEscapeState = eRpHexEscapeState_GetSecondHexByte;
				break;
	
			case eRpHexEscapeState_GetSecondHexByte:
				*theDecodedStringPtr++ |= RpHexToNibble(&theErrorFlag,
													theCurrentCharacter);
				theHexEscapeState = eRpHexEscapeState_LookForPercent;
				break;
		}
		theCurrentCharacter = *theEncodedStringPtr++;
	}
	*theDecodedStringPtr = '\0';
	return;
}

#endif	/* RomPagerServer || RomPagerBasic */
