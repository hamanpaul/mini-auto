/*
 *	File:		RpForm.c
 *
 *	Contains:	routines for Forms processing
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
 *		09/22/03	rhb		change prototype for AsReceiveText
 *		07/13/03	bva		move call back routines to RpCallBk.c
 * * * * Release 4.21  * * *
 *		02/11/03	rhb		support enum types for Variable Access
 * * * * Release 4.20  * * *
 *		01/06/03	rhb		try to protect against too long input strings
 *		01/03/03	rhb		get Size and MaxLength right
 *		12/09/02	rhb		fix warning if RomPagerSoftPages and !RomPagerForms
 *		09/13/02	rhb		add casts to Boolean for OSE warnings
 * * * * Release 4.12  * * *
 * * * * Release 4.10  * * *
 *		05/16/02	rhb		parameter to AsCheckAscii was 8-bits
 *		04/18/02	rhb		fix compiler warnings
 *		04/10/02	bva		fix compile warning
 * * * * Release 4.07  * * *
 *		03/22/02	rhb		check ASCII_Fixed and TextAreaBuf input for bad
 *								characters
 * * * * Release 4.06  * * *
 * * * * Release 4.03  * * *
 *		10/30/01	rhb		fix ReceiveItemTextAreaBuf bug
 * * * * Release 4.02  * * *
 * * * * Release 4.01  * * *
 *		08/14/01	bva		add eRpItemType_FormPasswordDyn, 
 *							eRpItemType_FormHiddenDyn to Form support
 * * * * Release 4.00  * * *
 *		05/09/01	pjr		don't overwrite a user item error in RpReceiveItem
 *		02/16/01	rhb		rename RpStoreCustomItem to AsStoreCustomItem and
 *								RpStoreSnmpItem to AsStoreSnmpItem
 *		02/15/01	bva		change RpExpandedMatch parameter
 *		02/14/01	rhb		rename rpItemError and eRpItemError_* to 
 *								asItemError and eAsItemError_* and use 
 *								AsReceiveText()
 *		02/13/01	rhb		rename RomPagerSnmpAccess to AsSnmpAccess, 
 *								RomPagerUse64BitIntegers to AsUse64BitIntegers,
 *								RpCustomVariableAccess to AsCustomVariableAccess
 *		12/12/00	pjr		rework to use new AsXxx conversion routines
 *		11/25/00	pjr		ReceiveText is now global and returns rpItemError
 *		10/27/00	pjr		move index values to the connection structure
 *		08/09/00	rhb		support RpHtmlFormImage
 *		08/07/00	rhb		use kRpIndexCharacter instead of kAscii_Question
 *		06/02/00	bva		change prototypes for RpQuery.c routines
 *		05/26/00	bva		use gRpDataPtr
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.10  * * *
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 *		01/11/00	bva		NULL -> (char *) 0
 * * * * Release 3.06  * * *
 * * * * Release 3.02  * * *
 *		05/04/99	bva		add ASCII_Extended for rpTextType
 * * * * Release 3.0 * * * *
 *		04/02/99	pjr		fix compiler error
 *		03/27/99	bva		remove RpHtmlSubmit
 *		03/16/99	pjr		rework debug code
 *		03/08/99	bva		use RpStrLenCpy
 *		02/01/99	bva		change FixedSingleSelect items to use VALUE
 *		01/20/99	pjr		change RpHexToNibble usage
 *		01/09/99	pjr		move RpGetFormItem and RpEscapeDecodeString to
 *							RpFrmItm.c
 *		12/28/98	bva		add custom variable access support
 *		12/16/98	bva		merge general SNMP access routines
 * * * * Release 2.2 * * * *
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		10/06/98	pjr		move RpHexToNibble to RpCommon.c
 *		09/08/98	bva		fix eRpItemType_FormTextAreaBuf support
 *		07/13/98	bva		add more form debugging printfs
 *		07/06/98	bva		fix warnings in ReceiveItemFixedSingleSelect
 *		05/27/98	bva		fix RpDynamic definition
 * * * * Release 2.1 * * * *
 *		05/23/98	bva		rework compile flags
 *		05/22/98	bva		add RpSetRedirect, RpGetCurrentUrl
 *		05/20/98	rhb		support VarValue <SELECT> types
 *		05/14/98	bva		add RpGetFormBufferPtr
 *		04/04/98	bva		check incoming text field lengths
 *		03/20/98	bva		add form flow debugging
 *		03/14/98	bva		eliminate unused variables
 *		03/12/98	bva		use kMaxValueLength in RpGetFormItem
 *		03/09/98	bva		add support for eRpItemType_FormButton
 *		03/04/98	rhb		remove theSetPtr in ReceiveItemRadioGroupDynamic
 *		02/16/98	bva		remove RpFindFormItemType
 *		02/15/98	bva		rework item search and item setting to save memory
 *		01/06/98	pjr		add RpLogHttpEvent call from RpProcessForm.
 * * * * Release 2.0 * * * *
 *		12/08/97	bva		remove obsolete code
 *		12/03/97	bva		make RpReceiveItem available to CGI routines
 *		11/10/97	rhb		add support for 64 bit integers
 *		10/28/97	bva		rework User Exit support
 *		09/23/97	bva		add support for eRpItemType_FormNamedSubmit
 *		09/15/97	rhb		add support for eRpItemType_FormRadioButtonDyn,
 *								eRpItemType_FormFixedMultiDyn,
 *								eRpItemType_FormVariableSingleDyn, and
 *								eRpItemType_FormVariableMultiDyn
 *		09/09/97	bva		add support for eRpItemType_FormFixedSingleDyn
 *		08/26/97	rhb		add support for kRpHexShiftRight
 *		08/21/97	bva		rework URL dispatching
 *		08/20/97	bva		add RpSetNextFilePage
 *		07/31/97	rhb		add support for eRpItemType_FormTextAreaBuf
 *		07/29/97	pjr		kAscii_Dash -> kAscii_Hyphen.
 *		07/25/97	pjr		ReceiveItem -> RpReceiveItem.
 *		07/25/97	bva		use kRpHexSeparator for eRpTextType_HexColonForm
 *		07/16/97	bva		add support for eRpItemType_FormTextDyn
 *		07/12/97	bva		fFormRequestLength -> fPostRequestLength
 *		07/10/97	bva		add RpFindFormItemType
 *		07/04/97	bva		rework ExpandedMatch and move to RpHtml
 *		05/25/97	bva		rework RpProcessForm, split out RpEscapeDecodeString
 * * * * Release 1.6 * * * *
 *		04/04/97	bva		cleanup warnings
 *		03/05/97	bva		add support for eRpItemType_FormCheckboxDyn
 *		03/03/97	bva		add ExpandedMatch
 *		01/20/97	bva		add user intercept of form items to ReceiveItem
 *		12/20/96	rhb		fix length check in RpProcessForm
 *		12/11/96	bva		consolidate RpInitializeFormState with RpProcessForm
 * * * * Release 1.5 * * * *
 *		11/20/96	bva		fix compiler warning for RpConvertStringToSigned32
 *		11/13/96	bva		rework string conversion routines
 *		11/05/96	rhb		ifdef unused variable in ResetFormItems
 *		11/05/96	bva		add conditional compiles
 *		10/22/96	bva		move error page data to RpData
 *		10/21/96	bva		fix ReceiveText to only Set variables in
 *								conversions that are error free
 *		10/20/96	bva		use phrase dictionary on error messages
 *		10/18/96	bva		consolidate HTTP headers and form items buffers
 *		10/17/96	rhb		rename ResetCheckboxItems to ResetFormItems
 *		10/16/96	rhb		fixed tests for maximum values
 *		10/10/96	bva		add initializations for picky compilers
 *		09/24/96	rhb		change name from RpCgi.c
 *		09/24/96	rhb		support dynamically allocated engine data
 *		09/24/96	bva		add conditional compile flags for form items
 *		09/20/96	rhb		allow more than one HTTP request
 * * * * Release 1.4 * * * *
 *		08/17/96	bva		fix error message handling,
 *							fix ReceiveDataInHex,
 *							fix ReceiveDataInDotForm
 *		08/16/96	bva		add support for eRpItemType_FormRadioGroupDyn,
 *							add save of Submit button value,
 *							add RpSetNextPage,
 *							use page pointers in image map structures
 * * * * Release 1.3 * * * *
 *		07/30/96	bva		remove BooleanPtr, remove fAccess from page/form
 *								structures
 *		07/29/96	bva		add eRpTextType_ASCII_Fixed support
 *		07/23/96	bva		cleanup compiler warnings
 *		07/22/96	bva		pass index pointer to rpProcessDataFuncPtr call
 *		07/05/96	bva		image map handling becomes part of general query,
 *							merge page and form headers
 *		06/24/96	rhb		added eRpTextType_HexColonForm
 *		06/23/96	bva		reduced image map code size
 *		06/22/96	rhb		eliminate redundant ReceiveDataInDecimal functions
 *		06/11/96	rhb		added support for circle and polygon areas in
 *							image maps
 * * * * Release 1.2 * * * *
 *		05/30/96	bva		cleanup to eliminate warnings
 * * * * Release 1.1 * * * *
 *		04/25/96	rhb		split apart fixed variable select types
 *		04/24/96	rhb		convert text items when SetPtr is function
 * * * * Release 1.0 * * * *
 *		03/28/96	bva		fix ResetCheckboxItems
 *		03/09/96	bva		added hidden field support
 *		02/03/96	bva		added image mapping support
 *		01/01/96	bva		change hexdata format
 *		11/08/95	bva		fGciCommandPtr -> fCgiCommandPtr
 *		11/01/95	rhb		created
 *
 *	To Do:
 */

#include "AsEngine.h"


#if RomPagerServer

#if RomPagerForms

#define RpDynamic (RpHtmlRadioDynamic || RpHtmlSelectFixedSingle || RpHtmlSelectFixedMulti || \
		RpHtmlSelectVariable || RpHtmlSelectVarValue || RpHtmlTextFormDynamic)

#if RpHtmlCheckbox || RpHtmlCheckboxDynamic || RpHtmlSelectFixedMulti || \
	RpHtmlSelectVariable || RpHtmlSelectVarValue
static void		ResetFormItems(rpDataPtr theDataPtr);
#endif

#if RpHtmlCheckbox || RpHtmlSelectFixedMulti
static void SetBooleanItem(rpDataPtr theDataPtr,
							void * theSetPtr,
							rpVariableType theSetPtrType,
							Boolean theValue,
							char *theNamePtr,
							Signed16Ptr theIndexValuesPtr);
#endif

#if RpHtmlRadio || RpHtmlRadioDynamic || RpHtmlSelectFixedSingle
static void SetRadioItem(rpDataPtr theDataPtr,
							void * theSetPtr,
							rpVariableType theSetPtrType,
							rpOneOfSeveral theValue,
							char *theNamePtr,
							Signed16Ptr theIndexValuesPtr);
#endif
#if RpHtmlNamedSubmit || RomPagerJavaScript
static void		ReceiveItemNamedSubmit(rpDataPtr theDataPtr,
					rpCheckboxFormItemPtr theItemPtr, char *theNamePtr,
					char *theValuePtr, Signed16Ptr theIndexValuesPtr);
#endif


static void		ReceiveSubmitButton(rpDataPtr theDataPtr,
					rpButtonFormItemPtr theItemPtr, char *theNamePtr,
					char *theValuePtr);

#if RpHtmlRadioDynamic
static void		ReceiveItemRadioGroupDynamic(rpDataPtr theDataPtr,
					rpRadioGroupInfoPtr theItemPtr, char *theNamePtr,
					char *theValuePtr, Signed16Ptr theIndexValuesPtr);
#endif

#if RpHtmlCheckboxDynamic
static void		ReceiveItemCheckboxDynamic(rpDataPtr theDataPtr,
											rpDynCheckboxFormItemPtr theItemPtr,
											char *theNamePtr,
											char *theValuePtr,
											Signed16Ptr theIndexValuesPtr);
#endif

#if RpHtmlSelectFixedSingle
static void		ReceiveItemFixedSingleSelect(rpDataPtr theDataPtr,
					rpFixedSingleSelectFormItemPtr theItemPtr,
					char *theNamePtr, char *theValuePtr,
					Signed16Ptr theIndexValuesPtr);
#endif

#if RpHtmlTextAreaBuf
static void		ReceiveItemTextAreaBuf(rpDataPtr theDataPtr,
					rpTextAreaFormItemPtr theItemPtr, char *theNamePtr,
					char *theValuePtr, Signed16Ptr theIndexValuesPtr);
#endif

#if RpHtmlSelectFixedMulti
static void		ReceiveItemFixedMultiSelect(rpDataPtr theDataPtr,
					rpOption_MultiSelectPtr theFirstOptionPtr,
					char *theNamePtr,
					char *theValuePtr,
					Signed16Ptr theIndexValuesPtr);
#endif

#if RpHtmlSelectVarValue
static void		ReceiveItemVariableValueSelect(rpDataPtr theDataPtr,
						char * theValuePtr,
						rpVariableSelectFormItemPtr theItemPtr,
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
#endif

#endif	/* RomPagerForms */


#if RpHtmlFormImage
static void		ReceiveItemImage(rpDataPtr theDataPtr,
						rpImageFormItemPtr theItemPtr,
						char *theNamePtr, char *theValuePtr,
						Signed16Ptr theIndexValuesPtr);
#endif

#if RomPagerImageMapping
static Boolean	MapCircleMatches(rpHttpRequestPtr theRequestPtr,
					rpImageMapLocationPtr theItemPtr);
static Boolean	MapPolygonMatches(rpHttpRequestPtr theRequestPtr,
					rpImageMapLocationPtr theItemPtr);
static Boolean	MapRectangleMatches(rpHttpRequestPtr theRequestPtr,
					rpImageMapLocationPtr theItemPtr);
#endif

#if RomPagerForms

void RpProcessForm(rpDataPtr theDataPtr) {
	char *					theBufferPtr;
	char *					theNamePtr;
	char *					theValuePtr;
	rpObjectExtensionPtr	theExtensionPtr;
	Signed32				theLength;
	rpHttpRequestPtr		theRequestPtr;

#if RpHtmlCheckbox || RpHtmlCheckboxDynamic || RpHtmlSelectFixedMulti || \
	RpHtmlSelectVariable || RpHtmlSelectVarValue
	ResetFormItems(theDataPtr);
#endif

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = eAsItemError_NoError;
	theLength = theRequestPtr->fPostRequestLength;

#if RomPagerLogging
#if (kRpLoggingLevel == 1 || kRpLoggingLevel == 2)
	/*
		Log the time, response action (form processed), and
		the object description.
	*/
	RpLogHttpEvent(theDataPtr, eRpHttpFormProcessed, eRpRomUrl,
					theRequestPtr->fCurrentFormDescriptionPtr);
#endif
#endif

#if defined(_jxWeb)
		_jxWeb_TracePostData();
#endif

#if RpFormFlowDebug
	RP_PRINTF("\nBegin form - %s\n", theRequestPtr->fCurrentFormDescriptionPtr->fURL);
#endif

	if (theLength > 0) {
		theBufferPtr = theRequestPtr->fHttpWorkBuffer;
		*(theBufferPtr + theLength) = '\0';
		theNamePtr = theRequestPtr->fCurrentItemName;
		theValuePtr = theRequestPtr->fCurrentItemValue;
		while (*theBufferPtr != '\0' &&
				theRequestPtr->fItemError == eAsItemError_NoError) {
			RpGetFormItem(&theBufferPtr, theNamePtr, theValuePtr);
			RpReceiveItem(theDataPtr, theNamePtr, theValuePtr);
		}
	}
#if RpFormFlowDebug
	RP_PRINTF("End form items\n");
#endif
	/*
		Do any needed post processing.
	*/
	if (theRequestPtr->fItemError == eAsItemError_NoError) {
		theExtensionPtr =
				theRequestPtr->fCurrentFormDescriptionPtr->fExtensionPtr;
		if (theExtensionPtr != (rpObjectExtensionPtr) 0 &&
				theExtensionPtr->fProcessDataFuncPtr !=
					(rpProcessDataFuncPtr) 0) {
			theExtensionPtr->fProcessDataFuncPtr(theDataPtr,
					theDataPtr->fCurrentConnectionPtr->fIndexValues);
		}
	}

	/*
		if there was an error, set up the page for the reply
	*/
	if (theRequestPtr->fItemError != eAsItemError_NoError) {
		theRequestPtr->fObjectPtr = &gRpItemErrorPage;
	}
#if RpFormFlowDebug
	RP_PRINTF("End form\nPage about to be served is - %s\n", theRequestPtr->fObjectPtr->fURL);
#endif
	return;
}

char * RpGetFormBufferPtr(void *theTaskDataPtr) {

	return ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr->fHttpWorkBuffer;
}


char * RpGetSubmitButtonValue(void *theTaskDataPtr) {
	rpDataPtr			theDataPtr;

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	return theDataPtr->fCurrentHttpRequestPtr->fSubmitButtonValue;
}


#if RpHtmlCheckbox || RpHtmlCheckboxDynamic || RpHtmlSelectFixedMulti || \
	RpHtmlSelectVariable
static void ResetFormItems(rpDataPtr theDataPtr) {
	rpItemPtr							theItemPtr;
#if RpHtmlCheckbox || RpHtmlCheckboxDynamic || RpHtmlSelectFixedMulti
	Signed16Ptr							theIndexValuesPtr;
#if RpHtmlCheckbox
	rpCheckboxFormItemPtr				theCheckboxPtr;
#endif
#if RpHtmlCheckboxDynamic
	rpDynCheckboxFormItemPtr			theDynCheckboxPtr;
#endif
#if RpHtmlSelectFixedMulti
	rpFixedMultiSelectFormItemPtr		theSelectPtr;
	rpOption_MultiSelectPtr				theOptionPtr;
	rpFixedMultiDynSelectFormItemPtr	theDynSelectPtr;
#endif
#endif

	theItemPtr = theDataPtr->fCurrentHttpRequestPtr->
						fCurrentFormDescriptionPtr->fItemsArrayPtr;
#if RpHtmlCheckbox || RpHtmlCheckboxDynamic || RpHtmlSelectFixedMulti
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
#endif

	while (theItemPtr->fType != eRpItemType_LastItemInList) {
		switch (theItemPtr->fType) {
#if RpHtmlCheckbox
			case eRpItemType_FormCheckbox:
				theCheckboxPtr = (rpCheckboxFormItemPtr) theItemPtr->fAddress;
				SetBooleanItem(theDataPtr,
								theCheckboxPtr->fSetPtr,
								theCheckboxPtr->fSetPtrType,
								False,
								theCheckboxPtr->fNamePtr,
								theIndexValuesPtr);
				break;
#endif
#if RpHtmlCheckboxDynamic
			case eRpItemType_FormCheckboxDyn:
				theDynCheckboxPtr =
						(rpDynCheckboxFormItemPtr) theItemPtr->fAddress;
				(* (rpResetCheckboxArrayPtr) theDynCheckboxPtr->fResetPtr)
					(theDataPtr, theDynCheckboxPtr->fNamePtr,
					theIndexValuesPtr);
				break;
#endif
#if RpHtmlSelectFixedMulti
			case eRpItemType_FormFixedMultiSelect:
				theSelectPtr =
						(rpFixedMultiSelectFormItemPtr) theItemPtr->fAddress;
				theOptionPtr = theSelectPtr->fFirstOptionPtr;
				while (theOptionPtr != (rpOption_MultiSelectPtr) 0 ) {
					SetBooleanItem(theDataPtr,
									theOptionPtr->fSetPtr,
									theOptionPtr->fSetPtrType,
									False,
									theSelectPtr->fNamePtr,
									theIndexValuesPtr);
					theOptionPtr = theOptionPtr->fNextPtr;
				}
				break;

			case eRpItemType_FormFixedMultiDyn:
				theDynSelectPtr =
						(rpFixedMultiDynSelectFormItemPtr) theItemPtr->fAddress;
				(* (rpResetCheckboxArrayPtr) theDynSelectPtr->fResetPtr)
					(theDataPtr, theDynSelectPtr->fNamePtr, theIndexValuesPtr);
				break;

#endif
#if RpHtmlSelectVariable
			case eRpItemType_FormVariableMultiSelect:
			case eRpItemType_FormVariableMultiDyn:
#endif
#if RpHtmlSelectVarValue
			case eRpItemType_FormVarValueMultiSelect:
			case eRpItemType_FormVarValueMultiDyn:
#endif
#if RpHtmlSelectVariable || RpHtmlSelectVarValue
				((rpVariableSelectFormItemPtr) theItemPtr->fAddress)->
						fResetOptionsPtr(theDataPtr);
				break;
#endif
			default:
				break;
		}
		theItemPtr += 1;
	}
	return;
}

#endif

#if RpHtmlCheckbox || RpHtmlSelectFixedMulti
static void SetBooleanItem(rpDataPtr theDataPtr,
							void * theSetPtr,
							rpVariableType theSetPtrType,
							Boolean theValue,
							char *theNamePtr,
							Signed16Ptr theIndexValuesPtr) {

	if (theSetPtrType == eRpVarType_Function) {
		(* (rpStoreBooleanFuncPtr) theSetPtr)(theValue);
	}
#if AsSnmpAccess
	else if (theSetPtrType == eRpVarType_Snmp) {
		AsStoreSnmpItem(theDataPtr, theNamePtr,
								theIndexValuesPtr,
								eAsTextType_Unsigned8,
								theSetPtr, &theValue);
	}
#endif
#if AsCustomVariableAccess
	else if (theSetPtrType == eRpVarType_Custom) {
		AsStoreCustomItem(theDataPtr, theNamePtr,
								theIndexValuesPtr,
								eAsTextType_Unsigned8,
								&theValue);
	}
#endif
	else if (theSetPtrType == eRpVarType_Complex) {
		(* (rpStoreBooleanComplexPtr) theSetPtr)
				(theDataPtr, theValue, theNamePtr, theIndexValuesPtr);
	}
	else {
		* (Boolean *) theSetPtr = theValue;
	}
	return;
}
#endif

#if RpHtmlRadio || RpHtmlRadioDynamic || RpHtmlSelectFixedSingle
static void SetRadioItem(rpDataPtr theDataPtr,
							void * theSetPtr,
							rpVariableType theSetPtrType,
							rpOneOfSeveral theValue,
							char *theNamePtr,
							Signed16Ptr theIndexValuesPtr) {

	if (theSetPtrType == eRpVarType_Function) {
		(*(rpStoreRadioGroupFuncPtr) theSetPtr)(theValue);
	}
#if AsSnmpAccess
	else if (theSetPtrType == eRpVarType_Snmp) {
		AsStoreSnmpItem(theDataPtr, theNamePtr,
								theIndexValuesPtr,
								eAsTextType_Unsigned8,
								theSetPtr, &theValue);
	}
#endif
#if AsCustomVariableAccess
	else if (theSetPtrType == eRpVarType_Custom) {
		AsStoreCustomItem(theDataPtr, theNamePtr,
								theIndexValuesPtr,
								eAsTextType_Unsigned8,
								&theValue);
	}
#endif
	else if (theSetPtrType == eRpVarType_Complex) {
		(*(rpStoreRadioGroupComplexPtr) theSetPtr)
				(theDataPtr, theValue, theNamePtr, theIndexValuesPtr);
	}
	else {
		*(rpOneOfSeveralPtr) theSetPtr = theValue;
	}
	return;
}
#endif


#if RomPagerUserExit
void RpSetFormObject(void *theTaskDataPtr,
						rpObjectDescriptionPtr theObjectPtr) {
	rpHttpRequestPtr	theRequestPtr;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fCurrentFormDescriptionPtr = theObjectPtr;
}
#endif

void RpReceiveItem(void *theTaskDataPtr, char *theNamePtr,
		char *theValuePtr) {
	rpDataPtr							theDataPtr;
	Signed16Ptr							theIndexValuesPtr;
	asItemError							theItemError;
	rpItemPtr							theItemPtr;
	rpHttpRequestPtr					theRequestPtr;
	rpTextFormItemPtr					theTextItemPtr;
#if RpHtmlCheckbox
	rpCheckboxFormItemPtr				theCheckboxItemPtr;
#endif
#if RpHtmlRadio || RpHtmlRadioDynamic
	rpRadioButtonFormItemPtr			theRadioButtonItemPtr;
#endif
#if RpHtmlSelectFixedMulti
	rpFixedMultiSelectFormItemPtr		theFixedMultiSelectItemPtr;
	rpFixedMultiDynSelectFormItemPtr	theFixedMultiDynSelectItemPtr;
#endif
#if RpHtmlSelectVariable
	rpVariableSelectFormItemPtr			theVariableSelectItemPtr;
#endif
#if RpDynamic
	char *								theQueryPtr;
	Signed8								theSavedQueryIndexLevel;
#endif

	theDataPtr = (rpDataPtr) theTaskDataPtr;
	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theItemPtr = theRequestPtr->fCurrentFormDescriptionPtr->fItemsArrayPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;

#if RpDynamic
	/*
		Check for index values stored at the end of the HTML name
	*/
	theSavedQueryIndexLevel = theDataPtr->fCurrentConnectionPtr->fIndexDepth;

	theQueryPtr = RpFindTokenDelimitedPtr(theNamePtr, kRpIndexCharacter);
	if (theQueryPtr != (char *) 0) {
		/*
			terminate the name and point to the query
		*/
		*theQueryPtr++ = '\0';
		RpStoreQueryValues(theDataPtr, theQueryPtr);
	}
#endif

	theItemPtr = RpFindItemFromName(theItemPtr, theNamePtr, theValuePtr);
#if RpFormFlowDebug
	RP_PRINTF("Name: %s;  Value: %s;  Type: ",theNamePtr, theValuePtr);
#endif
	switch (theItemPtr->fType) {
#if RpHtmlCheckbox
		case eRpItemType_FormCheckbox:
#if RpFormFlowDebug
			RP_PRINTF("Checkbox\n");
#endif
			theCheckboxItemPtr = (rpCheckboxFormItemPtr) theItemPtr->fAddress;
			SetBooleanItem(theDataPtr,
							theCheckboxItemPtr->fSetPtr,
							theCheckboxItemPtr->fSetPtrType,
							True,
							theNamePtr,
							theIndexValuesPtr);
			break;
#endif

#if RpHtmlNamedSubmit || RomPagerJavaScript
#if RpHtmlNamedSubmit
		case eRpItemType_FormNamedSubmit:
#endif
#if RomPagerJavaScript
		case eRpItemType_FormButton:
#endif
#if RpFormFlowDebug
			RP_PRINTF("Button\n");
#endif
			ReceiveItemNamedSubmit(theDataPtr,
					(rpCheckboxFormItemPtr) theItemPtr->fAddress,
					theNamePtr, theValuePtr, theIndexValuesPtr);
			break;
#endif	/* RpHtmlNamedSubmit || RomPagerJavaScript */

		case eRpItemType_FormSubmit:
#if RpFormFlowDebug
			RP_PRINTF("Submit Button\n");
#endif
			ReceiveSubmitButton(theDataPtr,
					(rpButtonFormItemPtr) theItemPtr->fAddress,
					theNamePtr, theValuePtr);
			break;

#if RpHtmlRadio
		case eRpItemType_FormRadioButton:
#endif
#if RpHtmlRadioDynamic
		case eRpItemType_FormRadioButtonDyn:
#endif
#if RpHtmlRadio || RpHtmlRadioDynamic
#if RpFormFlowDebug
			RP_PRINTF("Radio Button\n");
#endif
			theRadioButtonItemPtr = (rpRadioButtonFormItemPtr) theItemPtr->fAddress;

			SetRadioItem(theDataPtr,
							theRadioButtonItemPtr->fRadioGroupPtr->fSetPtr,
							theRadioButtonItemPtr->fRadioGroupPtr->fSetPtrType,
							theRadioButtonItemPtr->fButtonNumber,
							theNamePtr,
							theIndexValuesPtr);
			break;
#endif

#if RpHtmlCheckboxDynamic
		case eRpItemType_FormCheckboxDyn:
#if RpFormFlowDebug
			RP_PRINTF("Dynamic Checkbox\n");
#endif
		ReceiveItemCheckboxDynamic(theDataPtr,
					(rpDynCheckboxFormItemPtr) theItemPtr->fAddress,
					theNamePtr,
					theValuePtr,
					theIndexValuesPtr);
			break;
#endif

#if RpHtmlRadioDynamic
		case eRpItemType_FormRadioGroupDyn:
#if RpFormFlowDebug
			RP_PRINTF("eRpItemType_FormRadioGroupDyn\n");
#endif
			ReceiveItemRadioGroupDynamic(theDataPtr,
					(rpRadioGroupInfoPtr) theItemPtr->fAddress, theNamePtr,
					theValuePtr, theIndexValuesPtr);
			break;
#endif

#if RpHtmlTextFormDynamic
		case eRpItemType_FormTextDyn:
		case eRpItemType_FormPasswordDyn:
		case eRpItemType_FormHiddenDyn:
#endif
		case eRpItemType_FormAsciiText:
		case eRpItemType_FormPasswordText:
		case eRpItemType_FormHiddenText:
#if RpFormFlowDebug
			RP_PRINTF("Text\n");
#endif
			theTextItemPtr = (rpTextFormItemPtr) theItemPtr->fAddress;
			theItemError = RpReceiveText(theDataPtr, 
										theValuePtr,
										theTextItemPtr->fSetPtrType, 
										theTextItemPtr->fSetPtr,
										theTextItemPtr->fTextType, 
										theTextItemPtr->fFieldMaxLength,
										theNamePtr, 
										theIndexValuesPtr);

			/*
				Test fItemError in the request block before overwriting
				it because the user may have called RpSetUserErrorMessage
				from their own conversion routine.
			*/
			if (theRequestPtr->fItemError == eAsItemError_NoError) {
				theRequestPtr->fItemError = theItemError;
			}
			break;

#if RpHtmlSelectFixedSingle
		case eRpItemType_FormFixedSingleSelect:
		case eRpItemType_FormFixedSingleDyn:
#if RpFormFlowDebug
			RP_PRINTF("Fixed Single Select\n");
#endif
			ReceiveItemFixedSingleSelect(theDataPtr,
					(rpFixedSingleSelectFormItemPtr) theItemPtr->fAddress,
					theNamePtr,
					theValuePtr,
					theIndexValuesPtr);
			break;
#endif

#if RpHtmlSelectVariable
		case eRpItemType_FormVariableSingleSelect:
		case eRpItemType_FormVariableMultiSelect:
		case eRpItemType_FormVariableSingleDyn:
		case eRpItemType_FormVariableMultiDyn:
#if RpFormFlowDebug
			RP_PRINTF("Variable Select\n");
#endif
			theVariableSelectItemPtr =
					(rpVariableSelectFormItemPtr) theItemPtr->fAddress;
			theItemError = RpReceiveText(theDataPtr, theValuePtr,
					theVariableSelectItemPtr->fSetPtrType,
					theVariableSelectItemPtr->fSetPtr,
					theVariableSelectItemPtr->fTextType,
					theVariableSelectItemPtr->fFieldMaxLength,
					theNamePtr, theIndexValuesPtr);

			/*
				Test fItemError in the request block before overwriting
				it because the user may have called RpSetUserErrorMessage
				from their own conversion routine.
			*/
			if (theRequestPtr->fItemError == eAsItemError_NoError) {
				theRequestPtr->fItemError = theItemError;
			}
			break;
#endif

#if RpHtmlSelectVarValue
		case eRpItemType_FormVarValueSingleSelect:
		case eRpItemType_FormVarValueMultiSelect:
		case eRpItemType_FormVarValueSingleDyn:
		case eRpItemType_FormVarValueMultiDyn:
#if RpFormFlowDebug
			RP_PRINTF("Variable Value Select\n");
#endif
			ReceiveItemVariableValueSelect(theDataPtr, theValuePtr,
					(rpVariableSelectFormItemPtr) theItemPtr->fAddress,
					theNamePtr, theIndexValuesPtr);
			break;
#endif


#if RpHtmlSelectFixedMulti
		case eRpItemType_FormFixedMultiSelect:
#if RpFormFlowDebug
			RP_PRINTF("eRpItemType_FormFixedMultiSelect\n");
#endif
			theFixedMultiSelectItemPtr = (rpFixedMultiSelectFormItemPtr) theItemPtr->fAddress;
			ReceiveItemFixedMultiSelect(theDataPtr,
					theFixedMultiSelectItemPtr->fFirstOptionPtr,
					theNamePtr,
					theValuePtr,
					theIndexValuesPtr);
			break;

		case eRpItemType_FormFixedMultiDyn:
#if RpFormFlowDebug
			RP_PRINTF("eRpItemType_FormFixedMultiDyn\n");
#endif
			theFixedMultiDynSelectItemPtr = (rpFixedMultiDynSelectFormItemPtr) theItemPtr->fAddress;
			ReceiveItemFixedMultiSelect(theDataPtr,
					theFixedMultiDynSelectItemPtr->fFirstOptionPtr,
					theNamePtr,
					theValuePtr,
					theIndexValuesPtr);
			break;
#endif

#if RpHtmlTextArea
		case eRpItemType_FormTextArea:
#if RpFormFlowDebug
			RP_PRINTF("eRpItemType_FormTextArea\n");
#endif
			break;
#endif

#if RpHtmlTextAreaBuf
		case eRpItemType_FormTextAreaBuf:
#if RpFormFlowDebug
			RP_PRINTF("eRpItemType_FormTextAreaBuf\n");
#endif
			ReceiveItemTextAreaBuf(theDataPtr,
					(rpTextAreaFormItemPtr) theItemPtr->fAddress,
					theNamePtr, theValuePtr, theIndexValuesPtr);
			break;
#endif

#if RpHtmlFormImage
			case eRpItemType_FormImage:
#if RpFormFlowDebug
			RP_PRINTF("eRpItemType_FormImage\n");
#endif
			ReceiveItemImage(theDataPtr,
					(rpImageFormItemPtr) theItemPtr->fAddress,
					theNamePtr, theValuePtr, theIndexValuesPtr);
			break;
#endif

		case eRpItemType_LastItemInList:
			theRequestPtr->fItemError = eAsItemError_ItemNotFound;
			break;

		default:
#if RpFormFlowDebug
			RP_PRINTF("Unknown Item Type\n");
#endif
			/*
				an unknown item type?!?
			*/
			break;
	}	/* end switch */
#if RpDynamic
	theDataPtr->fCurrentConnectionPtr->fIndexDepth = theSavedQueryIndexLevel;
#endif

	return;
}


asItemError RpReceiveText(rpDataPtr theDataPtr,
							char *theValuePtr,
							rpVariableType theSetPtrType,
							void *theSetPtr,
							rpTextType theTextType,
							Unsigned8 theFieldSize,
							char *theNamePtr,
							Signed16Ptr theIndexValuesPtr) {
	asItemError			theItemError;
	size_t				theValueLength;

	theItemError = eAsItemError_NoError;

	switch (theTextType) {

		case eRpTextType_ASCII_Fixed:
		case eRpTextType_ASCII_Extended:
			/*
				Non-HTML sources (ie. Java applets) may submit forms without 
				checking data lengths.  We'd like to check the incoming value 
				length against the field size and report an error if the data 
				won't fit. We can't necessarily do this since PageBuilder and
				VarBuilder didn't always let the user supply the field size.
			*/
			theValueLength = RP_STRLEN(theValuePtr);

			if (theFieldSize > 0 && theValueLength > theFieldSize) {
				/* Arthur Chow 2005.11.02 - delete the extra chars that longer than theFieldSize */
				int i=theFieldSize-1;
				int finish=0;
				int count=0;
				while (!finish)
				{
					char ch=theValuePtr[i];
					if(ch>0x7e || ch<0x20)
					{
						count++;
						i--;
						if (i<0)
							finish=1;
					}
					else
					{
						finish=1;
					}
				}
				theValuePtr[theFieldSize]=0;
				if (count%2)
					theValuePtr[theFieldSize-1]=0;
				/*
				theItemError = eAsItemError_TooManyCharacters;
				*/
			}
			else if (theTextType == eRpTextType_ASCII_Fixed) {
				theItemError = AsCheckAsciiString(theValuePtr, 
						(Unsigned16) theValueLength);
			}
			if (theItemError == eAsItemError_NoError) {

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
					if (theTextType == eRpTextType_ASCII_Fixed) {
							RP_MEMCPY((char *) theSetPtr, 
										theValuePtr, 
										theFieldSize);
					}
					else {
						RP_STRCPY((char *) theSetPtr, theValuePtr);
					}
				}
			}
			break;

		default:
			 theItemError = AsReceiveText(theDataPtr,
										theValuePtr, 
										theSetPtrType, 
										theSetPtr, 
										(asTextType) theTextType, 
										theFieldSize, 
										theNamePtr, 
#if AsUseEnums
										(asEnumElementPtr) 0,
#endif
										theIndexValuesPtr);		
			break;
	}
	return theItemError;
}


static void ReceiveSubmitButton(rpDataPtr theDataPtr,
									rpButtonFormItemPtr theItemPtr,
									char *theNamePtr,
									char *theValuePtr) {

	RpStrLenCpy(theDataPtr->fCurrentHttpRequestPtr->fSubmitButtonValue,
			theValuePtr, kMaxStringLength);
	return;
}

#if RpHtmlNamedSubmit || RomPagerJavaScript
static void ReceiveItemNamedSubmit(rpDataPtr theDataPtr,
									rpCheckboxFormItemPtr theItemPtr,
									char *theNamePtr,
									char *theValuePtr,
									Signed16Ptr theIndexValuesPtr) {
	void *			theSetPtr;
	rpVariableType	theSetPtrType;

	theSetPtr = theItemPtr->fSetPtr;
	theSetPtrType = theItemPtr->fSetPtrType;
	if (theSetPtrType == eRpVarType_Function) {
		(*(rpStoreAsciiTextFuncPtr) theSetPtr)(theValuePtr);
	}
#if AsSnmpAccess
	else if (theSetPtrType == eRpVarType_Snmp) {
		AsStoreSnmpItem(theDataPtr, theNamePtr,
								theIndexValuesPtr,
								eAsTextType_ASCII,
								theSetPtr, theValuePtr);
	}
#endif
#if AsCustomVariableAccess
	else if (theSetPtrType == eRpVarType_Custom) {
		AsStoreCustomItem(theDataPtr, theNamePtr,
								theIndexValuesPtr,
								eAsTextType_ASCII,
								theValuePtr);
	}
#endif
	else if (theSetPtrType == eRpVarType_Complex) {
		(*(rpStoreAsciiTextComplexPtr) theSetPtr)(theDataPtr,
				theValuePtr, theNamePtr, theIndexValuesPtr);
	}
	else {
		RP_STRCPY((char *) theSetPtr, theValuePtr);
	}
	return;
}
#endif

#if RpHtmlCheckboxDynamic
static void ReceiveItemCheckboxDynamic(rpDataPtr theDataPtr,
										rpDynCheckboxFormItemPtr theItemPtr,
										char *theNamePtr,
										char *theValuePtr,
										Signed16Ptr theIndexValuesPtr) {
	Signed8		theQueryIndexLevel;
	void *		theSetPtr;

	theSetPtr = theItemPtr->fSetPtr;
	theValuePtr++;		/* skip past the question mark */
	theQueryIndexLevel = theDataPtr->fCurrentConnectionPtr->fIndexDepth;
	RpStoreQueryValues(theDataPtr, theValuePtr);
	if (theItemPtr->fSetPtrType == eRpVarType_Complex) {
		(*(rpStoreBooleanComplexPtr) theSetPtr)(theDataPtr, True,
				theNamePtr, theIndexValuesPtr);
	}
	theDataPtr->fCurrentConnectionPtr->fIndexDepth = theQueryIndexLevel;
	return;
}
#endif


#if RpHtmlRadioDynamic
static void ReceiveItemRadioGroupDynamic(rpDataPtr theDataPtr,
											rpRadioGroupInfoPtr theItemPtr,
											char *theNamePtr,
											char *theValuePtr,
											Signed16Ptr theIndexValuesPtr) {
	Unsigned8		theButtonValue;

	theButtonValue = RpConvertStringToSigned8(theDataPtr, theValuePtr);
	SetRadioItem(theDataPtr,
					theItemPtr->fSetPtr,
					theItemPtr->fSetPtrType,
					theButtonValue,
					theNamePtr,
					theIndexValuesPtr);
	return;
}
#endif


#if RpHtmlSelectFixedSingle

static void ReceiveItemFixedSingleSelect(rpDataPtr theDataPtr,
		rpFixedSingleSelectFormItemPtr theItemPtr,
		char *theNamePtr,
		char *theValuePtr,
		Signed16Ptr theIndexValuesPtr) {
	rpHttpRequestPtr		theRequestPtr;
	Unsigned8				theConvertedUnsigned8;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theConvertedUnsigned8 =
			RpConvertStringToUnsigned8(theDataPtr, theValuePtr);
	if (theRequestPtr->fItemError == eAsItemError_NoError) {
		SetRadioItem(theDataPtr,
						theItemPtr->fSetPtr,
						theItemPtr->fSetPtrType,
						theConvertedUnsigned8,
						theNamePtr,
						theIndexValuesPtr);
	}
	return;
}
#endif


#if RpHtmlSelectFixedMulti
static void ReceiveItemFixedMultiSelect(rpDataPtr theDataPtr,
										rpOption_MultiSelectPtr theFirstOptionPtr,
										char *theNamePtr,
										char *theValuePtr,
										Signed16Ptr theIndexValuesPtr) {
	rpHttpRequestPtr		theRequestPtr;
	rpOption_MultiSelectPtr	theOptionPtr;
	Boolean					theValueFoundFlag;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theValueFoundFlag = False;
	theOptionPtr = theFirstOptionPtr;
	while (theOptionPtr != (rpOption_MultiSelectPtr) 0 && !theValueFoundFlag) {
		theValueFoundFlag =
				RpExpandedMatch(theRequestPtr, theValuePtr,
								theOptionPtr->fValuePtr);
		if (theValueFoundFlag) {
			SetBooleanItem(theDataPtr,
							theOptionPtr->fSetPtr,
							theOptionPtr->fSetPtrType,
							True,
							theNamePtr,
							theIndexValuesPtr);
		}
		else {
			theOptionPtr = theOptionPtr->fNextPtr;
		}
	}
	if (!theValueFoundFlag) {
		theRequestPtr->fItemError = eAsItemError_MultiSelectionOptionNotFound;
	}
	return;
}
#endif

#if RpHtmlSelectVarValue
static void ReceiveItemVariableValueSelect(rpDataPtr theDataPtr,
		char * theValuePtr, rpVariableSelectFormItemPtr theItemPtr,
		char *theNamePtr, Signed16Ptr theIndexValuesPtr) {
	void *			theSetPtr;
	rpVariableType	theSetPtrType;
	Unsigned32		theValue;

	theSetPtr = theItemPtr->fSetPtr;
	theSetPtrType = theItemPtr->fSetPtrType;
	RpConvertHexString(theDataPtr, (char *) &theValue, theValuePtr, 4, '\0');
	if (theDataPtr->fCurrentHttpRequestPtr->fItemError == eAsItemError_NoError) {
		if (theSetPtrType == eRpVarType_Function) {
			(*(rpStoreUnsigned32FuncPtr) theSetPtr)(theValue);
		}
		else if (theSetPtrType == eRpVarType_Complex) {
			(*(rpStoreUnsigned32ComplexPtr) theSetPtr)(theDataPtr,
					theValue, theNamePtr, theIndexValuesPtr);
		}
		else {
			*(Unsigned32Ptr) theSetPtr = theValue;
		}
	}
	return;

}
#endif

#if RpHtmlTextAreaBuf
static void ReceiveItemTextAreaBuf(rpDataPtr theDataPtr,
									rpTextAreaFormItemPtr theItemPtr,
									char *theNamePtr,
									char *theValuePtr,
									Signed16Ptr theIndexValuesPtr) {
	asItemError		theItemError;
	void *			theSetPtr;
	rpVariableType	theSetPtrType;

	theItemError = AsCheckAsciiString(theValuePtr, 
			(Unsigned16) RP_STRLEN(theValuePtr));
	if (theItemError == eAsItemError_NoError) {
		theSetPtrType = theItemPtr->fSetPtrType;
		if (theSetPtrType != eRpVarType_Direct) {
			theSetPtr = theItemPtr->fSetPtr;
			if (theSetPtrType == eRpVarType_Function) {
				(*(rpStoreAsciiTextFuncPtr) theSetPtr)(theValuePtr);
			}
#if AsSnmpAccess
			else if (theSetPtrType == eRpVarType_Snmp) {
				AsStoreSnmpItem(theDataPtr, theNamePtr,
										theIndexValuesPtr,
										eAsTextType_ASCII,
										theSetPtr, theValuePtr);
			}
#endif
#if AsCustomVariableAccess
			else if (theSetPtrType == eRpVarType_Custom) {
				AsStoreCustomItem(theDataPtr, theNamePtr,
										theIndexValuesPtr,
										eAsTextType_ASCII,
										theValuePtr);
			}
#endif
			else {
				(*(rpStoreAsciiTextComplexPtr) theSetPtr)(theDataPtr,
						theValuePtr, theItemPtr->fNamePtr, theIndexValuesPtr);
			}
		}
	}
	else {
		theDataPtr->fCurrentHttpRequestPtr->fItemError = theItemError;
	}
	return;
}
#endif


#if RpHtmlFormImage
static void ReceiveItemImage(rpDataPtr theDataPtr,
									rpImageFormItemPtr theItemPtr,
									char *theNamePtr,
									char *theValuePtr,
									Signed16Ptr theIndexValuesPtr) {
	rpHttpRequestPtr	theRequestPtr;
	void *				theSetPtr;
	rpVariableType		theSetPtrType;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	if (!theRequestPtr->fFormImageDetectedFlag) {
		RpStrLenCpy(theRequestPtr->fSubmitButtonValue, theNamePtr,
				kMaxStringLength);
		theSetPtr = theItemPtr->fSetPtr;
		theSetPtrType = theItemPtr->fSetPtrType;
		if (theSetPtrType == eRpVarType_Function) {
			(*(rpStoreAsciiTextFuncPtr) theSetPtr)(theNamePtr);
		}
#if AsSnmpAccess
		else if (theSetPtrType == eRpVarType_Snmp) {
			AsStoreSnmpItem(theDataPtr, theNamePtr,
									theIndexValuesPtr,
									eAsTextType_ASCII,
									theSetPtr, theNamePtr);
		}
#endif
#if AsCustomVariableAccess
		else if (theSetPtrType == eRpVarType_Custom) {
			AsStoreCustomItem(theDataPtr, theNamePtr,
									theIndexValuesPtr,
									eAsTextType_ASCII,
									theNamePtr);
		}
#endif
		else if (theSetPtrType == eRpVarType_Complex) {
			(*(rpStoreAsciiTextComplexPtr) theSetPtr)(theDataPtr,
					theNamePtr, theNamePtr, theIndexValuesPtr);
		}
		else {
			RP_STRCPY((char *) theSetPtr, theNamePtr);
		}
		theRequestPtr->fFormImageDetectedFlag = True;
	}
	return;
}
#endif


void RpConvertHexString(void *theTaskDataPtr, char *theHexPtr,
		char *theValuePtr, Unsigned8 theOutputCharCount, char theSeparator) {
	rpHttpRequestPtr	theRequestPtr;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = AsStrToHex(theValuePtr, theSeparator,
										theHexPtr, theOutputCharCount);
	return;
}


/*
	This routine converts an ASCII dot form string into an array of
	hex bytes.

	Inputs:
		theTaskDataPtr		- pointer to the tasks data structure
		theDotFormPtr		- pointer to the hex byte array for the
							  conversion result
		theValuePtr			- pointer to the ASCII dot form string
							  to be converted
		theOutputCharCount	- size of output byte array

	Returns:
		none

	Note that for historical reasons, this routine is called with the
	dot form source string being pointed to by theValuePtr and the
	result hex string being pointed to by theDotFormPtr!
*/

void RpConvertDotFormString(void *theTaskDataPtr, char *theDotFormPtr,
		char *theValuePtr, Unsigned8 theOutputCharCount) {
	rpHttpRequestPtr	theRequestPtr;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;

	/*
		The theDotFormPtr and theValuePtr reversal is intentional and correct.
	*/
	theRequestPtr->fItemError =
			AsDotFormStrToHex(theValuePtr, theDotFormPtr, theOutputCharCount);

	return;
}


Signed8 RpConvertStringToSigned8(void *theTaskDataPtr, char *theValuePtr) {
	rpHttpRequestPtr	theRequestPtr;
	Signed8				theSigned8;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = AsStrToSigned8(theValuePtr, &theSigned8);

	return theSigned8;
}


Signed16 RpConvertStringToSigned16(void *theTaskDataPtr, char *theValuePtr) {
	rpHttpRequestPtr	theRequestPtr;
	Signed16			theSigned16;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = AsStrToSigned16(theValuePtr, &theSigned16);

	return theSigned16;
}


Unsigned8 RpConvertStringToUnsigned8(void *theTaskDataPtr, char *theValuePtr) {
	rpHttpRequestPtr	theRequestPtr;
	Unsigned8			theUnsigned8;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = AsStrToUnsigned8(theValuePtr, &theUnsigned8);

	return theUnsigned8;
}


Unsigned16 RpConvertStringToUnsigned16(void *theTaskDataPtr, char *theValuePtr) {
	rpHttpRequestPtr	theRequestPtr;
	Unsigned16			theUnsigned16;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = AsStrToUnsigned16(theValuePtr, &theUnsigned16);

	return theUnsigned16;
}

#endif /* RomPagerForms */


Signed32 RpConvertStringToSigned32(void *theTaskDataPtr, char *theValuePtr) {
	rpHttpRequestPtr	theRequestPtr;
	Signed32			theSigned32;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = AsStrToSigned32(theValuePtr, &theSigned32);

	return theSigned32;
}


Unsigned32 RpConvertStringToUnsigned32(void *theTaskDataPtr,
										char *theValuePtr) {
	rpHttpRequestPtr	theRequestPtr;
	Unsigned32			theUnsigned32;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = AsStrToUnsigned32(theValuePtr, &theUnsigned32);

	return theUnsigned32;
}


#if AsUse64BitIntegers

Signed64 RpConvertStringToSigned64(void *theTaskDataPtr, char *theValuePtr) {
	rpHttpRequestPtr	theRequestPtr;
	Signed64			theSigned64;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = AsStrToSigned64(theValuePtr, &theSigned64);

	return theSigned64;
}


Unsigned64 RpConvertStringToUnsigned64(void *theTaskDataPtr,
		char *theValuePtr) {
	rpHttpRequestPtr	theRequestPtr;
	Unsigned64			theUnsigned64;

	theRequestPtr = ((rpDataPtr) theTaskDataPtr)->fCurrentHttpRequestPtr;
	theRequestPtr->fItemError = AsStrToUnsigned64(theValuePtr, &theUnsigned64);

	return theUnsigned64;
}

#endif	/* AsUse64BitIntegers */


#if RomPagerImageMapping

void RpProcessImageMap(rpDataPtr theDataPtr) {
	Boolean					theDoneFlag;
	Signed16Ptr				theIndexValuesPtr;
	rpImageMapFormItemPtr	theItemPtr;
	rpMapLocation			theLocation;
	rpImageMapLocationPtr	theLocationItemPtr;
	rpItemPtr				theMapItemPtr;
	rpObjectDescriptionPtr	thePagePtr = (rpObjectDescriptionPtr) 0;
	rpHttpRequestPtr		theRequestPtr;
	void *					theSetPtr;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
	theMapItemPtr = theRequestPtr->fCurrentFormDescriptionPtr->fItemsArrayPtr;
	theItemPtr = (rpImageMapFormItemPtr) theMapItemPtr->fAddress;
	theRequestPtr->fMapHorizontal = theIndexValuesPtr[0] + 1;
	theRequestPtr->fMapVertical = theIndexValuesPtr[1] + 1;

	theDoneFlag = False;
	theLocation = 0;
	theRequestPtr->fObjectPtr = theItemPtr->fDefaultPagePtr;
	theLocationItemPtr = theItemPtr->fFirstLocationPtr;
	while (!theDoneFlag) {
		switch (theLocationItemPtr->fType) {
			case eRpLocationType_Rectangle:
				theDoneFlag =
						MapRectangleMatches(theRequestPtr, theLocationItemPtr);
				break;

			case eRpLocationType_Circle:
				theDoneFlag =
						MapCircleMatches(theRequestPtr, theLocationItemPtr);
				break;

			case eRpLocationType_Polygon:
				theDoneFlag =
						MapPolygonMatches(theRequestPtr, theLocationItemPtr);
				break;
		}
		if (theDoneFlag) {
			theLocation = theLocationItemPtr->fLocation;
			thePagePtr = theLocationItemPtr->fPagePtr;
		}
		else {
			theLocationItemPtr = theLocationItemPtr->fNextPtr;
			if (theLocationItemPtr == 0) {
				theDoneFlag = True;
			}
		}
	}

	theSetPtr = theItemPtr->fSetPtr;
	if (theItemPtr->fSetPtrType == eRpVarType_Function) {
		(*(rpStoreLocationPtr) theSetPtr)(theLocation);
	}
	else if (theItemPtr->fSetPtrType == eRpVarType_Complex) {
		(*(rpStoreLocationComplexPtr) theSetPtr)(theDataPtr, theLocation,
				kQuestion, theIndexValuesPtr);
	}
	else {
		*(rpMapLocationPtr) theSetPtr = theLocation;
	}
	if (theLocation != 0) {
		theRequestPtr->fObjectPtr = thePagePtr;
	}
	return;
}


static Boolean MapRectangleMatches(rpHttpRequestPtr theRequestPtr,
		rpImageMapLocationPtr theItemPtr) {
	Boolean	theMatchFlag;

	if (	theRequestPtr->fMapHorizontal >= theItemPtr->fLeft   &&
			theRequestPtr->fMapHorizontal <= theItemPtr->fRight  &&
			theRequestPtr->fMapVertical   >= theItemPtr->fTop    &&
			theRequestPtr->fMapVertical   <= theItemPtr->fBottom ) {
		theMatchFlag = True;
	}
	else {
		theMatchFlag = False;
	}

	return theMatchFlag;
}


static Boolean MapCircleMatches(rpHttpRequestPtr theRequestPtr,
		rpImageMapLocationPtr theItemPtr) {
	Signed16	theDistance;

	theDistance =	(theRequestPtr->fMapHorizontal - theItemPtr->fLeft) *
					(theRequestPtr->fMapVertical   - theItemPtr->fTop);
	if (theDistance < 0) {
		theDistance = -theDistance;
	}

	return theDistance <= theItemPtr->fRight * theItemPtr->fRight;
}


/*
	The polygon finding algorithm is:

	int pnpoly(int npol, float *xp, float *yp, float x, float y)
	{
		int i, j, c = 0;

		for (i = 0, j = npol-1; i < npol; j = i++) {
			if ((   ((yp[i] <= y) && (y < yp[j])) ||
					((yp[j] <= y) && (y < yp[i]))) &&
					(x < (xp[j] - xp[i]) * (y - yp[i]) /
					(yp[j] - yp[i]) + xp[i]))
						c = !c;
		}
		return c;
	}

	got that?
*/

static Boolean MapPolygonMatches(rpHttpRequestPtr theRequestPtr,
		rpImageMapLocationPtr theItemPtr) {
	int			i;
	int			j;
	Unsigned16	thePointCount;
	rpPointPtr	thePointsPtr;
	Boolean		theResult;

	theResult     = False;
	thePointCount = theItemPtr->fRight;
	thePointsPtr  = theItemPtr->fPolyPointPtr;
	for (i = 0, j = thePointCount - 1; i < thePointCount; j = i++) {
		if (
			(
				((thePointsPtr[i].fY <= theRequestPtr->fMapVertical) &&
				 (theRequestPtr->fMapVertical < thePointsPtr[j].fY)) ||
				((thePointsPtr[j].fY <= theRequestPtr->fMapVertical) &&
				 (theRequestPtr->fMapVertical < thePointsPtr[i].fY))
			) &&
			(theRequestPtr->fMapHorizontal <
				(thePointsPtr[j].fX - thePointsPtr[i].fX)  *
				(theRequestPtr->fMapVertical - thePointsPtr[i].fY)      /
				(thePointsPtr[j].fY - thePointsPtr[i].fY)  +
				thePointsPtr[i].fX
			)
		) {
			theResult = !theResult;
		}
	}

	return theResult;
}

#endif /* RomPagerImageMapping */

#if RomPagerForms || RomPagerSoftPages
rpItemPtr RpFindItemFromName(rpItemPtr theItemArrayPtr, char *theNamePtr,
		char *theValuePtr) {
	Boolean						theFoundItemFlag;
	rpItemPtr					theItemPtr;
	rpTextFormItemPtr			theTextFormItemPtr;
#if RpHtmlRadio || RpHtmlRadioDynamic
	rpRadioButtonFormItemPtr	theRadioButtonFormItemPtr;
#endif
#if RpHtmlFormImage
	rpImageFormItemPtr			theImageItemPtr;
	char						theImageName[kMaxStringLength];
	char *						theNameEndPtr;
#endif

	theFoundItemFlag = False;
#if RpHtmlFormImage
	RpStrLenCpy(theImageName, theNamePtr, kMaxStringLength);
	theNameEndPtr = theImageName + RP_STRLEN(theImageName) - 2;
	if (*theNameEndPtr++ == kAscii_Dot &&
			(*theNameEndPtr == kAscii_x || *theNameEndPtr == kAscii_y) ) {
		*--theNameEndPtr = '\0';
	}
#endif
	theItemPtr = theItemArrayPtr;
	while (!theFoundItemFlag &&
			theItemPtr->fType != eRpItemType_LastItemInList) {
		switch (theItemPtr->fType) {
#if RpHtmlRadio
			case eRpItemType_FormRadioButton:
#endif
#if RpHtmlRadioDynamic
			case eRpItemType_FormRadioButtonDyn:
#endif
#if RpHtmlRadio || RpHtmlRadioDynamic
				/*
					If we have a radio button, the name is stored in the
					RadioGroup structure.
				*/
				theRadioButtonFormItemPtr =
						(rpRadioButtonFormItemPtr) theItemPtr->fAddress;
				theFoundItemFlag = (Boolean) (RP_STRCMP(theNamePtr,
						theRadioButtonFormItemPtr->fRadioGroupPtr->fNamePtr) == 0 &&
						RP_STRCMP(theValuePtr, theRadioButtonFormItemPtr->fValue) == 0);
				break;
#endif
#if RomPagerForms
			case eRpItemType_FormSubmit:
				/*
					Submit buttons without names have an
					implied name of "Submit".
				*/
				theFoundItemFlag = (Boolean) (RP_STRCMP(theNamePtr, 
						kHtmlSubmitLower) == 0);
				break;
#endif

#if RpHtmlFormImage
			case eRpItemType_FormImage:
				theImageItemPtr = (rpImageFormItemPtr) theItemPtr->fAddress;
				if (RP_STRCMP(theImageName, theImageItemPtr->fNamePtr) == 0) {
					RpStrLenCpy(theNamePtr, theImageName, kMaxStringLength);
					theFoundItemFlag = True;
				}
				break;
#endif

			default:
				/*
					the other form item structures begin with the HTML name,
					so just coerce the pointer as if it were a
					eRpItemType_FormAsciiText.
				*/
				theTextFormItemPtr = (rpTextFormItemPtr) theItemPtr->fAddress;
				theFoundItemFlag = (Boolean) (RP_STRCMP(theNamePtr,
						theTextFormItemPtr->fNamePtr) == 0);
				break;
		}
		if (!theFoundItemFlag) {
			theItemPtr += 1;
		}
	}
	return theItemPtr;
}
#endif /* RomPagerForms || RomPagerSoftPages */

#endif	/* RomPagerServer */
