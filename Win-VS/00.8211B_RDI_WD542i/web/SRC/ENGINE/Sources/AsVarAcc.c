/*
 *	File:		AsVarAcc.c
 *
 *	Contains:	routines for accessing RomPager variables as ASCII strings
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
 *		09/24/03	rhb		add warning if Set Pointer is NULL
 *		09/22/03	rhb		change prototypes for AsReceiveText and SendTextOut
 *		09/15/03	rhb		support null access types
 *		08/25/03	rhb		Variable Access is required for floating point vars.
 *		08/08/03	nam		support floating point for Variable Access
 *		07/08/03	bva		rework AsVariableAccess ifdefs
 * * * * Release 4.21  * * *
 *		02/27/03	rhb		add casts to char * in RpGetBytesPtr
 *		02/11/03	rhb		support enum types
 * * * * Release 4.20  * * *
 *		01/06/03	rhb		try to protect against too long input strings
 *		01/03/03	rhb		get Size and MaxLength right
 * * * * Release 4.12  * * *
 * * * * Release 4.10  * * *
 *		05/16/02	rhb		parameter to AsCheckAscii was 8-bits
 *		04/30/02	rhb		fix compiler warning, use 8 & 16 bit types for 8 & 16 
 *								bit SNMP & Custom accesses, and mark hex, hexsep, 
 *								& dotform data in calls to AsStoreSnmpItem/
 *								AsStoreCustomItem as eAsTextType_Hex
 *		04/10/02	bva		fix compile warning
 * * * * Release 4.07  * * *
 *		03/21/02	rhb		check ASCII input for bad characters
 * * * * Release 4.06  * * *
 * * * * Release 4.05  * * *
 *		12/10/01	amp		add eAsTextType_BoolValue support
 * * * * Release 4.04  * * *
 * * * * Release 4.03  * * *
 *		10/10/01	rhb		remove unused variable from AsSetItemValue
 * * * * Release 4.02  * * *
 *		09/81/01	bva		fix warnings 
 * * * * Release 4.01  * * *
 * * * * Release 4.00  * * *
 *		07/25/01	pjr		add default to switch in AsReceiveText
 *		03/16/01	rhb		move SendDataOutDotForm and SendDataOutHex to 
 *								AsConvrt.c and rename to As*
 *		03/06/01	bva		rework AsExpandString overflow 
 *		03/01/01	bva/pjr	rework AsGetItemValue overflow handling 
 *		02/20/01	bva		add variable index routines 
 *		02/16/01	bva		modify SendDataOutZeroTerminated to use connection
 *							block for user phrase dictionary
 *		02/16/01	rhb		rename Rp*CustomItem to As*CustomItem
 *		02/15/01	bva		remove dictionary patching, add AsExpandString
 *		02/15/01	rhb		rename kRpHexSeparator to kAsHexSeparator
 *		02/13/01	rhb		RomPagerSnmpAccess->AsSnmpAccess, 
 *							RomPagerUse64BitIntegers->AsUse64BitIntegers,
 *							RpCustomVariableAccess->AsCustomVariableAccess
 *		02/09/01	rhb		Variable Access Lists are arrays
 *		12/15/00	pjr		rework conditional
 *		11/25/00	pjr		initial version
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"


#if AsVariableAccess || RomPagerServer

static void					StoreTextTypeUnsigned8(rpDataPtr theDataPtr,
								Unsigned8 theUnsigned8,
								rpVariableType theSetPtrType,
								void *theSetPtr,
								char *theNamePtr,
								Signed16Ptr theIndexValuesPtr);
static void					StoreTextTypeUnsigned16(rpDataPtr theDataPtr,
								Unsigned16 theUnsigned16,
								rpVariableType theSetPtrType,
								void *theSetPtr,
								char *theNamePtr,
								Signed16Ptr theIndexValuesPtr);
static void					StoreTextTypeUnsigned32(rpDataPtr theDataPtr,
								Unsigned32 theUnsigned32,
								rpVariableType theSetPtrType,
								void *theSetPtr,
								char *theNamePtr,
								Signed16Ptr theIndexValuesPtr);
#endif	/* AsVariableAccess || RomPagerServer */


#if AsVariableAccess

static asVarAccessItemPtr	FindNamedVariable(rpDataPtr theDataPtr,
								char *theNamePtr);


void AsSetVarAccessItemList(void *theTaskDataPtr,
							asVarAccessItemHdl theMasterItemListPtr) {

	((rpDataPtr) theTaskDataPtr)->
			fMasterVarAccessItemListPtr = theMasterItemListPtr;
	return;
}


static asVarAccessItemPtr FindNamedVariable(rpDataPtr theDataPtr,
											char *theNamePtr) {
	asVarAccessItemHdl	theListPtr;
	asVarAccessItemPtr	theItemPtr;

	theListPtr = theDataPtr->fMasterVarAccessItemListPtr;

	if (theListPtr != (asVarAccessItemHdl) 0) {
		while (*theListPtr != (asVarAccessItemPtr) 0) {
			theItemPtr = *theListPtr;
			while (theItemPtr->fNamePtr != (char *) 0) {
				if (RP_STRCMP(theNamePtr, theItemPtr->fNamePtr) == 0) {

				/*
					This is the variable we're looking for.
				*/
					return theItemPtr;
				}
				else {
					theItemPtr += 1;
				}
			}
			theListPtr += 1;
		}
	}

	return (asVarAccessItemPtr) 0;
}


/*****************************************************************************
	Routines for setting RomPager Variables follow:
*****************************************************************************/

/*
	The AsSetItemValue routine is called by the user application to
	set the value of a named user item.  RomPager converts the value
	string to the format of the item.  RomPager Variable Access only
	supports the eRpItemType_FormAsciiText item type.

	Inputs:
		theTaskDataPtr:			- pointer to the tasks data structure
		theNamePtr:				- pointer to item name string
		theValuePtr:			- pointer to item value string

	Returns:
		asItemError				- error from conversion routine
*/

asItemError AsSetItemValue(void *theTaskDataPtr, char *theNamePtr,
							char *theValuePtr) {
	rpDataPtr			theDataPtr;
	asItemError			theItemError;
	Signed16Ptr			theIndexValuesPtr;
	asVarAccessItemPtr	theItemPtr;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theItemPtr = FindNamedVariable(theDataPtr, theNamePtr);

	if (theItemPtr == (asVarAccessItemPtr) 0) {
		theItemError = eAsItemError_ItemNotFound;
	}
	else {
		theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
		theItemError = eAsItemError_NoError;
	
		theItemError = AsReceiveText(theDataPtr,
									theValuePtr,
									theItemPtr->fSetPtrType,
									theItemPtr->fSetPtr,
									theItemPtr->fTextType,
									theItemPtr->fFieldMaxLength,
									theNamePtr,
#if AsUseEnums
									theItemPtr->fEnumElementPtr,
#endif
									theIndexValuesPtr);
	}
	return theItemError;
}

#endif /* AsVariableAccess */

#if AsVariableAccess || RomPagerServer

asItemError AsReceiveText(rpDataPtr theDataPtr,
							char *theValuePtr,
							rpVariableType theSetPtrType,
							void *theSetPtr,
							asTextType theTextType,
							Unsigned8 theFieldSize,
							char *theNamePtr,
#if AsUseEnums
							asEnumElementPtr theEnumElementPtr,
#endif
							Signed16Ptr theIndexValuesPtr) {
	char				theConvertedData[128];	/* max internal size of field */
#if AsVariableAccess && AsUseFloatingPoint
	double				theConvertedDouble;
	float				theConvertedFloat;
#endif
	Signed8				theConvertedSigned8;
	Signed16			theConvertedSigned16;
	Signed32			theConvertedSigned32;
#if AsUse64BitIntegers
	Signed64			theConvertedSigned64;
#endif
	Unsigned8			theConvertedUnsigned8;
	Unsigned16			theConvertedUnsigned16;
	Unsigned32			theConvertedUnsigned32;
#if AsUse64BitIntegers
	Unsigned64			theConvertedUnsigned64;
#endif
	Unsigned8			theDataLength;
	char				theSeparator;
	asItemError			theItemError;
	size_t				theValueLength;

	theItemError = eAsItemError_NoError;

	if (theSetPtr != (void *) 0) {
		switch (theTextType) {

			case eAsTextType_ASCII:
				/*
					Non-HTML sources (ie. Java applets) may submit forms 
					without checking data lengths.  We'd like to check 
					the incoming value length against the field size and 
					report an error if the data won't fit. We can't 
					necessarily do this since PageBuilder and VarBuilder 
					didn't always let the user supply the field size.
				*/
				theValueLength = RP_STRLEN(theValuePtr);
				if (theFieldSize > 0 && theValueLength > theFieldSize) {
					theItemError = eAsItemError_TooManyCharacters;
				}
				else {
					theItemError = AsCheckAsciiString(theValuePtr,
							(Unsigned16) theValueLength);
				}
				if (theItemError == eAsItemError_NoError) {
					/*
						Send the text to the appropriate routine, or
						store it directly.
					*/
					if (theSetPtrType == eRpVarType_Function) {
						(*(rpStoreAsciiTextFuncPtr) theSetPtr)(theValuePtr);
					}
#if AsSnmpAccess
					else if (theSetPtrType == eRpVarType_Snmp) {
						AsStoreSnmpItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_ASCII,
									theSetPtr, 
									theValuePtr);
					}
#endif
#if AsCustomVariableAccess
					else if (theSetPtrType == eRpVarType_Custom) {
						AsStoreCustomItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_ASCII,
									theValuePtr);
					}
#endif
					else if (theSetPtrType == eRpVarType_Complex) {
						(*(rpStoreAsciiTextComplexPtr) theSetPtr)(theDataPtr,
									theValuePtr, 
									theNamePtr, 
									theIndexValuesPtr);
					}
					else {
						RP_STRCPY((char *) theSetPtr, theValuePtr);
					}
				}
				break;

			case eAsTextType_Hex:
			case eAsTextType_HexColonForm:
				if (theTextType == eAsTextType_Hex) {
					theSeparator = '\0';
					theDataLength = (theFieldSize + 1) / 2;
				}
				else {
					theSeparator = kAsHexSeparator;
					theDataLength = (theFieldSize + 2) / 3;
				}
				theItemError = AsStrToHex(theValuePtr, 
									theSeparator,
									theConvertedData, 
									theDataLength);
				if (theItemError == eAsItemError_NoError) {
					if (theSetPtrType == eRpVarType_Function) {
						(* (rpStoreAsciiTextFuncPtr) theSetPtr)(
									theConvertedData);
					}
#if AsSnmpAccess
					else if (theSetPtrType == eRpVarType_Snmp) {
						AsStoreSnmpItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Hex,
									theSetPtr, 
									&theConvertedData);
					}
#endif
#if AsCustomVariableAccess
					else if (theSetPtrType == eRpVarType_Custom) {
						AsStoreCustomItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Hex,
									&theConvertedData);
					}
#endif
					else if (theSetPtrType == eRpVarType_Complex) {
						(*(rpStoreAsciiTextComplexPtr) theSetPtr)(theDataPtr,
									theConvertedData, 
									theNamePtr, 
									theIndexValuesPtr);
					}
					else {
						RP_MEMCPY((char *) theSetPtr, 
									theConvertedData, 
									theDataLength);
					}
				}
				break;

			case eAsTextType_DotForm:
				theDataLength = (theFieldSize + 1) / 4;
				theItemError = AsDotFormStrToHex(theValuePtr,
									theConvertedData, 
									theDataLength);

				if (theItemError == eAsItemError_NoError) {
					if (theSetPtrType == eRpVarType_Function) {
						(* (rpStoreAsciiTextFuncPtr) theSetPtr)(
									theConvertedData);
					}
#if AsSnmpAccess
					else if (theSetPtrType == eRpVarType_Snmp) {
						AsStoreSnmpItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Hex,
									theSetPtr, 
									&theConvertedData);
					}
#endif
#if AsCustomVariableAccess
					else if (theSetPtrType == eRpVarType_Custom) {
						AsStoreCustomItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Hex,
									&theConvertedData);
					}
#endif
					else if (theSetPtrType == eRpVarType_Complex) {
						(*(rpStoreAsciiTextComplexPtr) theSetPtr)(theDataPtr,
									theConvertedData, 
									theNamePtr, 
									theIndexValuesPtr);
					}
					else {
						RP_MEMCPY((char *) theSetPtr, 
									theConvertedData, 
									theDataLength);
					}
				}
				break;

			case eAsTextType_Signed8:
				theItemError = AsStrToSigned8(theValuePtr, 
									&theConvertedSigned8);
				if (theItemError == eAsItemError_NoError) {
					if (theSetPtrType == eRpVarType_Function) {
						(*(rpStoreSigned8FuncPtr) theSetPtr)(
									theConvertedSigned8);
					}
#if AsSnmpAccess
					else if (theSetPtrType == eRpVarType_Snmp) {
						AsStoreSnmpItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Signed8,
									theSetPtr, 
									&theConvertedSigned8);
					}
#endif
#if AsCustomVariableAccess
					else if (theSetPtrType == eRpVarType_Custom) {
						AsStoreCustomItem(theDataPtr, 
									theNamePtr, 
									theIndexValuesPtr,
									eAsTextType_Signed8,
									&theConvertedSigned8);
					}
#endif
					else if (theSetPtrType == eRpVarType_Complex) {
						(*(rpStoreSigned8ComplexPtr) theSetPtr)(theDataPtr,
									theConvertedSigned8, 
									theNamePtr, 
									theIndexValuesPtr);
					}
					else {
						*(Signed8Ptr) theSetPtr = theConvertedSigned8;
					}
				}
				break;

			case eAsTextType_Signed16:
				theItemError = AsStrToSigned16(theValuePtr, 
									&theConvertedSigned16);
				if (theItemError == eAsItemError_NoError) {
					if (theSetPtrType == eRpVarType_Function) {
						(*(rpStoreSigned16FuncPtr) theSetPtr)(
									theConvertedSigned16);
					}
#if AsSnmpAccess
					else if (theSetPtrType == eRpVarType_Snmp) {
						AsStoreSnmpItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Signed16,
									theSetPtr, 
									&theConvertedSigned16);
					}
#endif
#if AsCustomVariableAccess
					else if (theSetPtrType == eRpVarType_Custom) {
						AsStoreCustomItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Signed16,
									&theConvertedSigned16);
					}
#endif
					else if (theSetPtrType == eRpVarType_Complex) {
						(*(rpStoreSigned16ComplexPtr) theSetPtr)(theDataPtr,
									theConvertedSigned16, 
									theNamePtr,
									theIndexValuesPtr);
					}
					else {
						*(Signed16Ptr) theSetPtr = theConvertedSigned16;
					}
				}
				break;

			case eAsTextType_Signed32:
				theItemError = AsStrToSigned32(theValuePtr, 
									&theConvertedSigned32);
				if (theItemError == eAsItemError_NoError) {
					if (theSetPtrType == eRpVarType_Function) {
						(*(rpStoreSigned32FuncPtr) theSetPtr)(
									theConvertedSigned32);
					}
#if AsSnmpAccess
					else if (theSetPtrType == eRpVarType_Snmp) {
						AsStoreSnmpItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Signed32,
									theSetPtr, 
									&theConvertedSigned32);
					}
#endif
#if AsCustomVariableAccess
					else if (theSetPtrType == eRpVarType_Custom) {
						AsStoreCustomItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Signed32,
									&theConvertedSigned32);
					}
#endif
					else if (theSetPtrType == eRpVarType_Complex) {
						(*(rpStoreSigned32ComplexPtr) theSetPtr)(theDataPtr,
									theConvertedSigned32, 
									theNamePtr,
									theIndexValuesPtr);
					}
					else {
						*(Signed32Ptr) theSetPtr = theConvertedSigned32;
					}
				}
				break;

#if AsUse64BitIntegers
			case eAsTextType_Signed64:
				theItemError = AsStrToSigned64(theValuePtr, 
									&theConvertedSigned64);
				if (theItemError == eAsItemError_NoError) {
					if (theSetPtrType == eRpVarType_Function) {
						(*(rpStoreSigned64FuncPtr) theSetPtr)(
									theConvertedSigned64);
					}
#if AsSnmpAccess
					else if (theSetPtrType == eRpVarType_Snmp) {
						AsStoreSnmpItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Signed64,
									theSetPtr, 
									&theConvertedSigned64);
					}
#endif
#if AsCustomVariableAccess
					else if (theSetPtrType == eRpVarType_Custom) {
						AsStoreCustomItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Signed64,
									&theConvertedSigned64);
					}
#endif
					else if (theSetPtrType == eRpVarType_Complex) {
						(*(rpStoreSigned64ComplexPtr) theSetPtr)(theDataPtr,
									theConvertedSigned64, 
									theNamePtr,
									theIndexValuesPtr);
					}
					else {
						*(Signed64Ptr) theSetPtr = theConvertedSigned64;
					}
				}
				break;
#endif	/* AsUse64BitIntegers */

			case eAsTextType_BoolValue:
			case eAsTextType_Unsigned8:
				theItemError = AsStrToUnsigned8(theValuePtr,
									&theConvertedUnsigned8);
				if (theItemError == eAsItemError_NoError) {
					StoreTextTypeUnsigned8(theDataPtr,
									theConvertedUnsigned8,
									theSetPtrType,
									theSetPtr,
									theNamePtr,
									theIndexValuesPtr);
				}
				break;

			case eAsTextType_Unsigned16:
				theItemError = AsStrToUnsigned16(theValuePtr,
									&theConvertedUnsigned16);
				if (theItemError == eAsItemError_NoError) {
					StoreTextTypeUnsigned16(theDataPtr,
									theConvertedUnsigned16,
									theSetPtrType,
									theSetPtr,
									theNamePtr,
									theIndexValuesPtr);
				}
				break;

			case eAsTextType_Unsigned32:
				theItemError = AsStrToUnsigned32(theValuePtr,
									&theConvertedUnsigned32);
				if (theItemError == eAsItemError_NoError) {
					StoreTextTypeUnsigned32(theDataPtr,
									theConvertedUnsigned32,
									theSetPtrType,
									theSetPtr,
									theNamePtr,
									theIndexValuesPtr);
				}
				break;

#if AsUse64BitIntegers
			case eAsTextType_Unsigned64:
				theItemError = AsStrToUnsigned64(theValuePtr,
									&theConvertedUnsigned64);
				if (theItemError == eAsItemError_NoError) {
					if (theSetPtrType == eRpVarType_Function) {
						(*(rpStoreUnsigned64FuncPtr) theSetPtr)(
									theConvertedUnsigned64);
					}
#if AsSnmpAccess
					else if (theSetPtrType == eRpVarType_Snmp) {
						AsStoreSnmpItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Unsigned64,
									theSetPtr, 
									&theConvertedUnsigned64);
					}
#endif
#if AsCustomVariableAccess
					else if (theSetPtrType == eRpVarType_Custom) {
						AsStoreCustomItem(theDataPtr, 
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Unsigned64,
									&theConvertedUnsigned64);
					}
#endif
					else if (theSetPtrType == eRpVarType_Complex) {
						(*(rpStoreUnsigned64ComplexPtr) theSetPtr)(theDataPtr,
									theConvertedUnsigned64, 
									theNamePtr,
									theIndexValuesPtr);
					}
					else {
						*(Unsigned64Ptr) theSetPtr = theConvertedUnsigned64;
					}
				}
				break;
#endif	/* AsUse64BitIntegers */

#if AsVariableAccess && AsUseFloatingPoint

			case eAsTextType_Float:
				theConvertedFloat = (float)AsStringToFloat(theValuePtr); 
				if (theSetPtrType == eRpVarType_Direct) {
					*(float *) theSetPtr = theConvertedFloat;
				}
				else if (theSetPtrType == eRpVarType_Function) {
					(*(rpStoreFloatFuncPtr) theSetPtr)(theConvertedFloat);
				}
#if AsSnmpAccess
				else if (theSetPtrType == eRpVarType_Snmp) {
					AsStoreSnmpItem(theDataPtr, 
									theNamePtr, 
									theIndexValuesPtr,
									eAsTextType_Float, 
									theSetPtr, 
									&theConvertedFloat);
				}
#endif
#if AsCustomVariableAccess
				else if (theSetPtrType == eRpVarType_Custom) {
					AsStoreCustomItem(theDataPtr, 
									theNamePtr, 
									theIndexValuesPtr,
									eAsTextType_Float, 
									&theConvertedFloat);
				}
#endif
				else {	/* complex type */
					(*(rpStoreFloatComplexPtr) theSetPtr)(theDataPtr,
									theConvertedFloat, 
									theNamePtr,
									theIndexValuesPtr);
				}
				break;

			case eAsTextType_Double:
				theConvertedDouble = AsStringToFloat(theValuePtr); 
				if (theSetPtrType == eRpVarType_Direct) {
					*(double *) theSetPtr = theConvertedDouble;
				}
				else if (theSetPtrType == eRpVarType_Function) {
					(*(rpStoreDoubleFuncPtr) theSetPtr)(theConvertedDouble);
				}
#if AsSnmpAccess
				else if (theSetPtrType == eRpVarType_Snmp) {
					AsStoreSnmpItem(theDataPtr, 
									theNamePtr, 
									theIndexValuesPtr,
									eAsTextType_Double, 
									theSetPtr, 
									&theConvertedDouble);
				}
#endif
#if AsCustomVariableAccess
				else if (theSetPtrType == eRpVarType_Custom) {
					AsStoreCustomItem(theDataPtr, 
									theNamePtr, 
									theIndexValuesPtr,
									eAsTextType_Double, 
									&theConvertedDouble);
				}
#endif
				else {	/* complex type */
					(*(rpStoreDoubleComplexPtr) theSetPtr)(theDataPtr,
									theConvertedDouble, 
									theNamePtr,
									theIndexValuesPtr);
				}
				break;
#endif	/* AsVariableAccess && AsUseFloatingPoint */

#if AsVariableAccess && AsUseEnums
			case eAsTextType_Enum1:
					theItemError = AsStrToEnum8(theValuePtr, 
									theEnumElementPtr + theFieldSize, 
									&theConvertedUnsigned8);
					if (theItemError == eAsItemError_NoError) {
						StoreTextTypeUnsigned8(theDataPtr,
									theConvertedUnsigned8,
									theSetPtrType,
									theSetPtr,
									theNamePtr,
									theIndexValuesPtr);
					}
				break;

			case eAsTextType_Enum2:
					theItemError = AsStrToEnum16(theValuePtr, 
									theEnumElementPtr + theFieldSize, 
									&theConvertedUnsigned16);
					if (theItemError == eAsItemError_NoError) {
						StoreTextTypeUnsigned16(theDataPtr,
									theConvertedUnsigned16,
									theSetPtrType,
									theSetPtr,
									theNamePtr,
									theIndexValuesPtr);
					}
				break;

			case eAsTextType_Enum4:
					theItemError = AsStrToEnum32(theValuePtr, 
									theEnumElementPtr + theFieldSize, 
									&theConvertedUnsigned32);
					if (theItemError == eAsItemError_NoError) {
						StoreTextTypeUnsigned32(theDataPtr,
									theConvertedUnsigned32,
									theSetPtrType,
									theSetPtr,
									theNamePtr,
									theIndexValuesPtr);
					}
				break;

#endif	/* AsVariableAccess && AsUseEnums */

			default:
				/*
					This shouldn't happen under normal conditions.
					However, eAsTextType_BoolString
					is not supported yet by variable access.
				*/
#if AsDebug
				RP_PRINTF("AsSetItemValue, unknown asTextType!\n");
#endif
				break;
		}
	}
#if RomPagerDebug
	else {
		RP_PRINTF("RpSetItemValue, theSetPtr = NULL!\n");
	}
#endif
	return theItemError;
}


static void StoreTextTypeUnsigned8(rpDataPtr theDataPtr,
									Unsigned8 theUnsigned8,
									rpVariableType theSetPtrType,
									void *theSetPtr,
									char *theNamePtr,
									Signed16Ptr theIndexValuesPtr) {

	if (theSetPtrType == eRpVarType_Function) {
		(*(rpStoreUnsigned8FuncPtr) theSetPtr)(theUnsigned8);
	}
#if AsSnmpAccess
	else if (theSetPtrType == eRpVarType_Snmp) {
		AsStoreSnmpItem(theDataPtr, theNamePtr, theIndexValuesPtr,
				eAsTextType_Unsigned8, theSetPtr, &theUnsigned8);
	}
#endif
#if AsCustomVariableAccess
	else if (theSetPtrType == eRpVarType_Custom) {
		AsStoreCustomItem(theDataPtr, theNamePtr, theIndexValuesPtr,
				eAsTextType_Unsigned8, &theUnsigned8);
	}
#endif
	else if (theSetPtrType == eRpVarType_Complex) {
		(*(rpStoreUnsigned8ComplexPtr) theSetPtr)(theDataPtr,
				theUnsigned8, theNamePtr, theIndexValuesPtr);
	}
	else {
		*(Unsigned8Ptr) theSetPtr = theUnsigned8;
	}
	return;
}


static void StoreTextTypeUnsigned16(rpDataPtr theDataPtr,
							Unsigned16 theUnsigned16,
							rpVariableType theSetPtrType,
							void *theSetPtr,
							char *theNamePtr,
							Signed16Ptr theIndexValuesPtr) {

	if (theSetPtrType == eRpVarType_Function) {
		(*(rpStoreUnsigned16FuncPtr) theSetPtr)(theUnsigned16);
	}
#if AsSnmpAccess
	else if (theSetPtrType == eRpVarType_Snmp) {
		AsStoreSnmpItem(theDataPtr, theNamePtr,
								theIndexValuesPtr,
								eAsTextType_Unsigned16,
								theSetPtr, &theUnsigned16);
	}
#endif
#if AsCustomVariableAccess
	else if (theSetPtrType == eRpVarType_Custom) {
		AsStoreCustomItem(theDataPtr, theNamePtr,
								theIndexValuesPtr,
								eAsTextType_Unsigned16,
								&theUnsigned16);
	}
#endif
	else if (theSetPtrType == eRpVarType_Complex) {
		(*(rpStoreUnsigned16ComplexPtr) theSetPtr)(theDataPtr,
				theUnsigned16, theNamePtr,
				theIndexValuesPtr);
	}
	else {
		*(Unsigned16Ptr) theSetPtr = theUnsigned16;
	}
	return;
}


static void StoreTextTypeUnsigned32(rpDataPtr theDataPtr,
							Unsigned32 theUnsigned32,
							rpVariableType theSetPtrType,
							void *theSetPtr,
							char *theNamePtr,
							Signed16Ptr theIndexValuesPtr) {

	if (theSetPtrType == eRpVarType_Function) {
		(*(rpStoreUnsigned32FuncPtr) theSetPtr)(theUnsigned32);
	}
#if AsSnmpAccess
	else if (theSetPtrType == eRpVarType_Snmp) {
		AsStoreSnmpItem(theDataPtr, theNamePtr,
								theIndexValuesPtr,
								eAsTextType_Unsigned32,
								theSetPtr, &theUnsigned32);
	}
#endif
#if AsCustomVariableAccess
	else if (theSetPtrType == eRpVarType_Custom) {
		AsStoreCustomItem(theDataPtr, theNamePtr,
								theIndexValuesPtr,
								eAsTextType_Unsigned32,
								&theUnsigned32);
	}
#endif
	else if (theSetPtrType == eRpVarType_Complex) {
		(*(rpStoreUnsigned32ComplexPtr) theSetPtr)(theDataPtr,
				theUnsigned32, theNamePtr,
				theIndexValuesPtr);
	}
	else {
		*(Unsigned32Ptr) theSetPtr = theUnsigned32;
	}
	return;
}

#endif	/* AsVariableAccess || RomPagerServer */

#if AsVariableAccess

/*****************************************************************************
	Routines for getting RomPager Variables follow:
*****************************************************************************/

static char *		GetBytesPtr(rpDataPtr theDataPtr, 
						void * theGetPtr,
						rpVariableType theGetPtrType, 
						char *theNamePtr,
						Signed16Ptr theIndexValuesPtr);

static asItemError	SendTextOut(rpDataPtr theDataPtr,
						asResponsePtr theResponsePtr,
						rpVariableType theGetPtrType,
						void *theGetPtr,
						asTextType theTextType,
						Unsigned8 theFieldSize,
#if AsUseEnums
						asEnumElementPtr theEnumElementPtr,
#endif
						char *theNamePtr);

static void			SendDataOutZeroTerminated(asResponsePtr theResponsePtr,
						const char *theCharPtr);

static void			SendDataOutExtendedAscii(asResponsePtr theResponsePtr,
						const char *theAsciiPtr);


/*
	This routine returns the value of a RomPager variable in the
	form of a null terminated ASCII string.

	Inputs:
		theTaskDataPtr		- pointer to the tasks data structure
		theNamePtr			- pointer to item name string
		theValuePtr			- pointer to a buffer for storing the value string
		theLength			- length of the buffer for storing the value string

	Returns:
		asItemError:
			eAsItemError_NoError
							- no error, an ASCII string representing the
							  item's value has been copied to theValuePtr
			eAsItemError_ItemNotFound
							- an item matching the supplied name was
							  not found
			eAsItemError_TooManyCharacters
							- the item value string is too large to fit
							  in the supplied buffer
*/

asItemError AsGetItemValue(void *theTaskDataPtr, char *theNamePtr,
							char *theValuePtr, Unsigned16 theLength) {
	rpDataPtr			theDataPtr;
	asItemError			theItemError;
	asVarAccessItemPtr	theItemPtr;
	asResponse			theResponse;

	theItemError = eAsItemError_NoError;
	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theItemPtr = FindNamedVariable(theDataPtr, theNamePtr);

	if (theItemPtr == (asVarAccessItemPtr) 0) {
		theItemError = eAsItemError_ItemNotFound;
	}
	if (theItemError == eAsItemError_NoError) {

		/*
			Since the SendTextOut routine does not null terminate
			the string it returns, and we want to for this routine,
			reserve room for the null by decrementing the length.
		*/   
		AsInitializeResponse(theDataPtr, &theResponse, theValuePtr,
								(char *) 0, (Unsigned16)(theLength - 1), False);

		if (theItemPtr->fGetPtr != 0) {
			theItemError = SendTextOut(theDataPtr, 
									&theResponse, 
									theItemPtr->fGetPtrType, 
									theItemPtr->fGetPtr, 
									theItemPtr->fTextType, 
									theItemPtr->fFieldMaxLength, 
#if AsUseEnums
									theItemPtr->fEnumElementPtr,
#endif
									(char *) theItemPtr->fNamePtr);
		}
#if RomPagerDebug
		else {
			RP_PRINTF("RpGetItemValue, fGetPtr = NULL!\n");
		}
#endif
	}
	if (theItemError == eAsItemError_NoError) {
		if (theResponse.fOverflowError) {
			/*
				The supplied response buffer is not large
				enough to hold the response string.
			*/
			theItemError = eAsItemError_TooManyCharacters;
			*theValuePtr = '\0';
		}
		else {
			/*
				The response string has been copied to the response
				buffer. Terminate the response string.
			*/
			*theResponse.fFillPtr = '\0';
		}
	}
	return theItemError;
}

asItemError AsExpandString(void *theTaskDataPtr, char *theInputPtr,
							char *theOutputPtr, Unsigned16 theBufferLength) {
	rpDataPtr			theDataPtr;
	asItemError			theItemError;
	asResponse			theResponse;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	/*
		Since the SendDataOutZeroTerminated routine does not null terminate
		the string it returns, and we want to for this routine, reserve room
		for the null by decrementing the length. This routine will truncate 
		strings that expand beyond the provided buffer.
	*/   
	AsInitializeResponse(theDataPtr, &theResponse, theOutputPtr,
							(char *) 0, (Unsigned16)(theBufferLength - 1), False);
	SendDataOutZeroTerminated(&theResponse, theInputPtr);

	if (theResponse.fOverflowError) {
		/*
			The supplied response buffer is not large
			enough to hold the response string.
		*/
		theItemError = eAsItemError_TooManyCharacters;
		*theOutputPtr = '\0';
	}
	else {
		/*
			The response string has been copied to the response
			buffer. Terminate the response string.
		*/
		theItemError = eAsItemError_NoError;
		*theResponse.fFillPtr = '\0';
	}
	return theItemError;
}

static asItemError SendTextOut(rpDataPtr theDataPtr, 
							asResponsePtr theResponsePtr,
							rpVariableType theGetPtrType, 
							void *theGetPtr,
							asTextType theTextType, 
							Unsigned8 theFieldSize,
#if AsUseEnums
							asEnumElementPtr theEnumElementPtr,
#endif
							char *theNamePtr) {
	char *				theBytesPtr;
	Unsigned8			theDataLength;
	Signed16Ptr			theIndexValuesPtr;
	asItemError			theItemError;
	char				theSeparator;
	Signed32			theSigned32;
	Unsigned32			theUnsigned32;
#if AsUse64BitIntegers
	Signed64			theSigned64;
	Unsigned64			theUnsigned64;
#endif
#if AsUseFloatingPoint
	float				theFloat;
	double				theDouble;
#endif

	theItemError = eAsItemError_NoError;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;

	switch (theTextType) {

		case eAsTextType_ASCII:
#if 0
		case eAsTextType_ASCII_Extended:
		case eAsTextType_ASCII_Fixed:
#endif
		case eAsTextType_DotForm:
		case eAsTextType_Hex:
		case eAsTextType_HexColonForm:
			theBytesPtr = GetBytesPtr(theDataPtr, theGetPtr, theGetPtrType,
										theNamePtr, theIndexValuesPtr);

			switch (theTextType) {
				case eAsTextType_ASCII:
					SendDataOutZeroTerminated(theResponsePtr, theBytesPtr);
					break;

#if 0
				case eAsTextType_ASCII_Extended:
					SendDataOutExtendedAscii(theResponsePtr, theBytesPtr);
					break;


				case eAsTextType_ASCII_Fixed:
					SendDataOutFixedText(theResponsePtr, theFieldSize, 
							theBytesPtr);
					break;
#endif

				case eAsTextType_Hex:
					theSeparator = '\0';
					theDataLength = (theFieldSize + 1) / 2;
					AsSendDataOutHex(theResponsePtr, 
										theDataLength, 
										theSeparator, 
										theBytesPtr);
					break;

				case eAsTextType_HexColonForm:
					theSeparator = kAsHexSeparator;
					theDataLength = (theFieldSize + 2) / 3;
					AsSendDataOutHex(theResponsePtr, 
										theDataLength, 
										theSeparator, 
										theBytesPtr);
					break;

				case eAsTextType_DotForm:
					theDataLength = (theFieldSize + 1) / 4;
					AsSendDataOutDotForm(theResponsePtr, 
										theDataLength, 
										theBytesPtr);
					break;

				default:
					break;
			}
			break;

#if AsUseEnums
		case eAsTextType_Enum1:
#endif
		case eAsTextType_Signed8:
			if (theGetPtrType == eRpVarType_Direct) {
				theSigned32 = (Signed32) *(Signed8Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theSigned32 =
						(Signed32)(*(rpFetchSigned8FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theSigned32 = 
						(Signed32) *((Signed8Ptr) AsRetrieveSnmpItem(
										theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType,
										theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned32 = 
						(Signed32) *((Signed8Ptr) AsRetrieveCustomItem(
										theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType));
			}
#endif
			else {
				theSigned32 =
						(Signed32)(*(rpFetchSigned8ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
#if AsUseEnums
			if (theTextType == eAsTextType_Enum1) {
				theItemError = AsSendDataOutEnum(theResponsePtr, 
						theEnumElementPtr, theSigned32);
			}
			else
#endif
			{
				AsSendDataOutDecimalSigned(theResponsePtr, theSigned32);
			}
			break;

#if AsUseEnums
		case eAsTextType_Enum2:
#endif
		case eAsTextType_Signed16:
			if (theGetPtrType == eRpVarType_Direct) {
				theSigned32 = (Signed32) *(Signed16Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theSigned32 =
						(Signed32)(*(rpFetchSigned16FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theSigned32 = (Signed32) *((Signed16Ptr) AsRetrieveSnmpItem(
										theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType,
										theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned32 = (Signed32) *((Signed16Ptr) AsRetrieveCustomItem(
										theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType));
			}
#endif
			else {
				theSigned32 =
						(Signed32)(*(rpFetchSigned16ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
#if AsUseEnums
			if (theTextType == eAsTextType_Enum2) {
				theItemError = AsSendDataOutEnum(theResponsePtr, 
						theEnumElementPtr, theSigned32);
			}
			else
#endif
			{
				AsSendDataOutDecimalSigned(theResponsePtr, theSigned32);
			}
			break;

#if AsUseEnums
		case eAsTextType_Enum4:
#endif
		case eAsTextType_Signed32:
			if (theGetPtrType == eRpVarType_Direct) {
				theSigned32 = *(Signed32Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theSigned32 =
						(*(rpFetchSigned32FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theSigned32 = *((Signed32 *) AsRetrieveSnmpItem(theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType,
										theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned32 = *((Signed32 *) AsRetrieveCustomItem(theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType));
			}
#endif
			else {
				theSigned32 =
						(*(rpFetchSigned32ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
#if AsUseEnums
			if (theTextType == eAsTextType_Enum4) {
				theItemError = AsSendDataOutEnum(theResponsePtr, 
						theEnumElementPtr, theSigned32);
			}
			else
#endif
			{
				AsSendDataOutDecimalSigned(theResponsePtr, theSigned32);
			}
			break;

#if AsUse64BitIntegers
		case eAsTextType_Signed64:
			if (theGetPtrType == eRpVarType_Direct) {
				theSigned64 = *(Signed64Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theSigned64 =
						(*(rpFetchSigned64FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theSigned64 = *((Signed64 *) AsRetrieveSnmpItem(theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType,
										theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned64 = *((Signed64 *) AsRetrieveCustomItem(theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType));
			}
#endif
			else {
				theSigned64 =
						(*(rpFetchSigned64ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			AsSendDataOutDecimalSigned64(theResponsePtr, theSigned64);
			break;
#endif	/* AsUse64BitIntegers */

		case eAsTextType_BoolValue:
		case eAsTextType_Unsigned8:
			if (theGetPtrType == eRpVarType_Direct) {
				theUnsigned32 = (Unsigned32) *(Unsigned8Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned8FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theUnsigned32 = 
						(Unsigned32) *((Unsigned8Ptr) AsRetrieveSnmpItem(
										theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType,
										theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theUnsigned32 = 
						(Unsigned32) *((Unsigned8Ptr) AsRetrieveCustomItem(
										theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType));
			}
#endif
			else {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned8ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			AsSendDataOutDecimalUnsigned(theResponsePtr, theUnsigned32);
			break;

		case eAsTextType_Unsigned16:
			if (theGetPtrType == eRpVarType_Direct) {
				theUnsigned32 = (Unsigned32) *(Unsigned16Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned16FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theUnsigned32 = 
						(Unsigned32) *((Unsigned16Ptr) AsRetrieveSnmpItem(
										theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType,
										theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theUnsigned32 = 
						(Unsigned32) *((Unsigned16Ptr) AsRetrieveCustomItem(
										theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType));
			}
#endif
			else {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned16ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			AsSendDataOutDecimalUnsigned(theResponsePtr, theUnsigned32);
			break;

		case eAsTextType_Unsigned32:
			if (theGetPtrType == eRpVarType_Direct) {
				theUnsigned32 = *(Unsigned32Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned32FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theUnsigned32 = *((Unsigned32 *) AsRetrieveSnmpItem(theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType,
										theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theUnsigned32 = *((Unsigned32 *) AsRetrieveCustomItem(theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType));
			}
#endif
			else {
				theUnsigned32 =
						(*(rpFetchUnsigned32ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			AsSendDataOutDecimalUnsigned(theResponsePtr, theUnsigned32);
			break;

#if AsUse64BitIntegers
		case eAsTextType_Unsigned64:
			if (theGetPtrType == eRpVarType_Direct) {
				theUnsigned64 = *(Unsigned64Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theUnsigned64 = (Unsigned64)
						(*(rpFetchUnsigned64FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theUnsigned64 = *((Unsigned64 *) AsRetrieveSnmpItem(theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType,
										theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theUnsigned64 = *((Unsigned64 *) AsRetrieveCustomItem(theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										theTextType));
			}
#endif
			else {
				theUnsigned64 = (*(rpFetchUnsigned64ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			AsSendDataOutDecimalUnsigned64(theResponsePtr, theUnsigned64);
			break;
#endif	/* AsUse64BitIntegers */

#if AsUseFloatingPoint
		case eAsTextType_Float:
			if (theGetPtrType == eRpVarType_Direct) {
				theFloat = *(float *) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theFloat = (float)(*(rpFetchFloatFuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theFloat = *((float *) AsRetrieveSnmpItem(theDataPtr,
						theNamePtr, theIndexValuesPtr, theTextType, theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theFloat = *((float *) AsRetrieveCustomItem(theDataPtr,
						theNamePtr, theIndexValuesPtr, theTextType));
			}
#endif
			else {	/* complex type */
				theFloat = (*(rpFetchFloatComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			AsSendDataOutFloat(theResponsePtr, (double) theFloat);
			break;

		case eAsTextType_Double:
			if (theGetPtrType == eRpVarType_Direct) {
				theDouble = *(double *) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theDouble = (double)(*(rpFetchDoubleFuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theDouble = *((double *) AsRetrieveSnmpItem(theDataPtr,
						theNamePtr, theIndexValuesPtr, theTextType, theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theDouble = *((double *) AsRetrieveCustomItem(theDataPtr,
						theNamePtr, theIndexValuesPtr, theTextType));
			}
#endif
			else {	/* complex type */
				theDouble = (*(rpFetchDoubleComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			AsSendDataOutFloat(theResponsePtr, theDouble);
			break;
#endif	/* AsUseFloatingPoint */

		default:
			break;
	}

	return theItemError;
}


static char *GetBytesPtr(rpDataPtr theDataPtr, void * theGetPtr,
		rpVariableType theGetPtrType, char *theNamePtr, 
		Signed16Ptr theIndexValuesPtr) {

	if (theGetPtrType == eRpVarType_Direct) {
		return (char *) theGetPtr;
	}
	else if (theGetPtrType == eRpVarType_Function) {
		return (*(rpFetchBytesFuncPtr) theGetPtr)();
	}
#if AsSnmpAccess
	else if (theGetPtrType == eRpVarType_Snmp) {
		return (char *) AsRetrieveSnmpItem(theDataPtr,
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_ASCII,
									theGetPtr);
	}
#endif
#if AsCustomVariableAccess
		else if (theGetPtrType == eRpVarType_Custom) {
			return (char *) AsRetrieveCustomItem(theDataPtr,
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_ASCII);
		}
#endif
	else {
		return (*(rpFetchBytesComplexPtr) theGetPtr)
				(theDataPtr, theNamePtr, theIndexValuesPtr);
	}
}


/*
	The SendDataOutZeroTerminated routine is the main routine to place
	null-terminated strings in the output buffers.  It is used for both
	direct and indirect user data.  All the strings that are sent by
	this routine will look at the string for compression dictionary
	expansion.  A more complete description of the phrase dictionary and
	the system dictionary phrase definitions can be found in RpDict.h.
	This version uses the user phrase dictionary from the connection block,
	if one exists, otherwise it uses the master phrase dictionary.

	NOTES:

		This routine is similar to SendDataOutZeroTerminated.
		It differs in the following ways:

			- it is passed an asResponsePtr instead of an rpHttpRequestPtr

			- it does not leave room in the buffers for RomMailer

			- it does not null terminate the buffer

		Eventually, this routine could be modified to do the above things
		and then replace the existing SendDataOutZeroTerminated routine.
*/

static void SendDataOutZeroTerminated(asResponsePtr theResponsePtr,
										const char *theCharPtr) {
	Unsigned8		theCharacter;
	rpConnectionPtr theConnectionPtr;
	rpDataPtr 		theDataPtr;
	Unsigned32		theIndex;
	const char *	theInputPtr;
	const char *	thePhrasePtr;
	Boolean			thePhrasesCanBeCompressedFlag;
	char **			theUserPhrases;

	theDataPtr = theResponsePtr->fDataPtr;
	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
	if (theConnectionPtr->fUserPhrases != (char **) 0) {
		theUserPhrases = theConnectionPtr->fUserPhrases;
		thePhrasesCanBeCompressedFlag = theConnectionPtr->fUserPhrasesCanBeCompressed;		
	}
	else {
		theUserPhrases = theDataPtr->fUserPhrases;
		thePhrasesCanBeCompressedFlag = theDataPtr->fUserPhrasesCanBeCompressed;		
	}
	theInputPtr = theCharPtr;
	

	if (theInputPtr != (const char *) 0) {
		theCharacter = *theInputPtr++;

		while (theCharacter != (char) 0 && !theResponsePtr->fOverflowError) {

			if (theResponsePtr->fFillBufferAvailable > 0) {

				if (theCharacter <= (Unsigned8) '\177') {
					/*
						Send out a normal character.
					*/
					*theResponsePtr->fFillPtr++ = theCharacter;
					theResponsePtr->fFillBufferAvailable -= 1;
				}
#if RomPagerServer
				else if (theCharacter < (Unsigned8) kAsCompressionEscape) {
					/*
						send out a system dictionary phrase
						(note the recursion)
					*/
					thePhrasePtr = (gRpPhrases[theCharacter -
												(Unsigned8) '\200']);
					SendDataOutZeroTerminated(theResponsePtr, thePhrasePtr);
				}
#endif
				else if (theCharacter == (Unsigned8) kAsCompressionEscape) {
					/*
						send out the next character with no expansion
					*/
					*theResponsePtr->fFillPtr++ = *theInputPtr++;
					theResponsePtr->fFillBufferAvailable -= 1;
				}
				else {
					/*
						the next character is used to index into the
						section of the user dictionary selected by
						the signal character.
					*/
					theIndex = (Unsigned8) *theInputPtr++;
					theIndex += ((Unsigned8) '\377' - theCharacter) *
									k8_BitsValue;
					thePhrasePtr = theUserPhrases[theIndex];

					if (thePhrasesCanBeCompressedFlag) {
						/*
							send out a user dictionary phrase
							(note the recursion)
						*/
						SendDataOutZeroTerminated(theResponsePtr, thePhrasePtr);
					}
					else {
						/*
							Send out a user dictionary phrase with no
							further expansion (useful for international).
						*/
						SendDataOutExtendedAscii(theResponsePtr, thePhrasePtr);
					}
				}
				theCharacter = *theInputPtr++;
			}
			else {
				AsFlipResponseBuffers(theResponsePtr);
			}
		}
	}

	return;
}


static void SendDataOutExtendedAscii(asResponsePtr theResponsePtr,
										const char *theAsciiPtr) {

	if (theAsciiPtr != (const char *) 0) {
		while (*theAsciiPtr != '\0' && !theResponsePtr->fOverflowError) {
			if (theResponsePtr->fFillBufferAvailable > 0) {
				*theResponsePtr->fFillPtr++ = *theAsciiPtr++;
				theResponsePtr->fFillBufferAvailable--;
			}
			else {
				AsFlipResponseBuffers(theResponsePtr);
			}
		}
	}
	return;
}
#endif	/* AsVariableAccess */

#if AsVariableAccess || RomPagerServer

/*
	Increments the variable index level by one and sets the value of the new 
	query index level (0-relative format) to the passed in value.
*/

void AsPushVariableIndex(void *theTaskDataPtr, Signed16 theMewIndexValue) {
	rpConnectionPtr		theConnectionPtr;

	theConnectionPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentConnectionPtr;

	if (theConnectionPtr->fIndexDepth < kAsIndexQueryDepth) {
		theConnectionPtr->fIndexDepth += 1;
		theConnectionPtr->fIndexValues[theConnectionPtr->fIndexDepth] =
				theMewIndexValue;
	}
#if AsDebug
	else {
		RP_PRINTF("AsPushVariableIndex, fIndexDepth overflow, discarding value!\n");
	}
#endif

	return;
}


/*
	Returns the value of the current variable index level (0-relative format),
	and reduces the variable index level by one.
*/

Signed16 AsPopVariableIndex(void *theTaskDataPtr) {
	rpConnectionPtr		theConnectionPtr;
	Signed16 			theIndexValue;

	theConnectionPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentConnectionPtr;

	if (theConnectionPtr->fIndexDepth != -1) {
		theIndexValue = theConnectionPtr->fIndexValues
								[theConnectionPtr->fIndexDepth];
		theConnectionPtr->fIndexDepth -= 1;
	}
	else {
		theIndexValue = 0;
	}
	return theIndexValue;
}


/*
	Returns the value of the variable index depth.
*/

Signed8 AsGetVariableIndexLevel(void *theTaskDataPtr) {
	rpConnectionPtr		theConnectionPtr;

	theConnectionPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentConnectionPtr;

	return theConnectionPtr->fIndexDepth;
}

#endif	/* AsVariableAccess || RomPagerServer */
