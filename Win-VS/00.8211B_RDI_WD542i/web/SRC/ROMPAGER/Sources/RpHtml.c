/*
 *	File:		RpHtml.c
 *
 *	Contains:	Routines to prepare ROM pages for sending
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
 *		09/03/03	pjr		change kMaxPathLength to kMaxSaveHeaderLength in
 *							SendOutImageSource
 *		08/13/03	bva		rework SendFormHeader for URL states,
 *							collapse SendOutUrlState for clarity
 *		08/01/03	bva		make SendFormHeader send relative URLs when possible
 * * * * Release 4.20  * * *
 *		02/25/03	rhb		add casts to char * in RpGetBytesPtr
 *		01/03/03	rhb		get Size and MaxLength right
 *		10/04/02	amp		add kRpObjFlag_ForceHttp and kRpObjFlag_ForceHttps
 *		09/13/02	rhb		fix OSE warnings by adding Boolean casts and others
 *		08/23/02	rhb		fix compiler warning
 * * * * Release 4.12  * * *
 * * * * Release 4.10  * * *
 *		06/08/02	bva		remove RomPagerSlaveIdentity
 *		04/18/02	rhb		fix compiler warnings
 *		04/10/02	bva		fix compile warning
 * * * * Release 4.07  * * *
 *		01/02/02	rhb		use 8 & 16 bit types for 8 & 16 bit Custom accesses
 * * * * Release 4.06  * * *
 * * * * Release 4.04  * * *
 *		12/12/01	bva		fix compiler warning in SendItemBuffer
 *		11/16/01	bva		use Unsigned32 for length in SendItemBuffer
 * * * * Release 4.03  * * *
 *		10/16/01	rhb		fix RpFileInsertItem when empty output buffer
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		04/19/01	bva		add RpPageFlowDebug support for dynamic and 
 *							repeat group items
 *		02/17/01	bva		remove RomPagerPreOneFive
 *		02/16/01	rhb		rename RpRetrieveCustomItem to AsRetrieveCustomItem,
 *								RpRetrieveSnmpItem to AsRetrieveSnmpItem,
 *								rpSnmpAccessItem to asSnmpAccessItem and 
 *								kRpCompressionEscape* to kAsCompressionEscape*
 *		02/15/01	bva		remove dictionary patching,
 *							change RpExpandedMatch parameter
 *		02/15/01	rhb		rename kRpHexSeparator to kAsHexSeparator
 *		02/13/01	rhb		rename RomPagerSnmpAccess to AsSnmpAccess, 
 *								RomPagerUse64BitIntegers to AsUse64BitIntegers,
 *								RpCustomVariableAccess to AsCustomVariableAccess
 *		01/05/01	pjr		add buffer display item type
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		10/27/00	pjr		move index values to the connection structure
 *		08/09/00	rhb		support RpHtmlFormImage
 *		08/07/00	rhb		use kRpIndexCharacter instead of kAscii_Question
 *		06/02/00	bva		use gRpDataPtr
 *		03/28/00	rhb		remove unnecessary code from SendItemFile()
 *		02/09/00	bva		use AsEngine.h
 *		02/02/00	bva		change RpSendDataOutZeroTerminated to fix warnings
 *		01/11/00	bva		NULL -> (char *) 0
 * * * * Release 3.10  * * *
 * * * * Release 3.06  * * *
 *		11/29/99	rhb		save away pointer to next item to fix problem with
 *							large VariableSelect items inside a RpDynamicDisplay
 * * * * Release 3.05  * * *
 * * * * Release 3.02  * * *
 *		04/26/99	bva		add ASCII_Extended for rpTextType
 * * * * Release 3.0 * * * *
 *		03/30/99	rhb		add RpGetDynamicDisplayIndex
 *		03/16/99	pjr		rework debug code
 *		02/01/99	bva		change FixedSingleSelect items to use VALUE
 *		01/14/99	pjr		conditionalize the whole file on RomPagerServer
 *		12/28/98	pjr		move some Server Push routines to RpHttpDy.c,
 *								move chunked code to RpCommon.c
 *		12/28/98	bva		add custom variable access support
 *		12/16/98	bva		merge general SNMP access routines
 *		12/14/98	bva		merge support for eRpItemType_SnmpDisplay
 *		12/11/98	rhb		merge Named Indirect & Soft Pages,
 *								SendItem -> RpSendItem
 *		12/07/98	rhb		fix SendItemVariableSelect, SendItemVarValueSelect
 *								for multiple occurences on the same page
 * * * * Release 2.2 * * * *
 *		11/30/98	bva		fix compile warnings
 *		11/12/98	rhb		give a type to fDataPtr in the request structure
 *		11/10/98	rhb		implement eRpItemType_File and make
 *								SendDataOutZeroTerminated public
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *		10/22/98	pjr		change Base64 prefixes from Rm to Rp
 *		10/16/98	pjr		moved some routines to RpCommon.c
 *		09/04/98	bva		optimize SendItemVariableSelect,
 *							SendItemVarValueSelect
 *		09/03/98	pjr		enable RpHexToString for RomPop
 *		08/31/98	bva		change <CR><LF> definitions
 *		07/31/98	pjr		more compiler warning fixes
 *		07/23/98	pjr		fix compiler warnings
 *		07/16/98	bva		add support for query items on
 *							eRpItemType_NamedDisplayText
 *		07/02/98	bva		add eRpItemType_FormPasswordDyn,
 *								eRpItemType_FormHiddenDyn,
 *								merge checkbox and checkbox dynamic
 *		06/22/98	pjr		space is now included in the content length string
 *		06/16/98	bva		add support for slave server identity
 * * * * Release 2.1 * * * *
 *		05/23/98	bva		rework compile flags
 *		05/20/98	rhb		support VarValue <SELECT> types
 *		05/01/98	bva		remove unused variables
 *		04/20/98	rhb		support multi-buffer Variable Selects
 *		03/09/98	bva		add support for eRpItemType_FormButton
 *		02/15/98	bva		use GetBytesPtr for memory savings
 *		02/13/98	rhb		change parameter to RpCatUnsigned32ToString
 *		02/01/98	bva		make sure SMTP text has canonical
 *							linefeeds (<CR><LF>)
 *		01/23/98	pjr		modify SendDataOutZeroTerminated to handle patched
 *							dictionary phrases (RpPhraseDictPatching)
 *		01/21/98	bva		SendDataOutBase64 -> RmSendDataOutBase64,
 *							SendOutBase64Padding -> RmSendOutBase64Padding
 *		01/13/98	bva		massage SendOutUrlState
 * * * * Release 2.0 * * * *
 *		12/16/97	pjr		add new line character to debug printfs
 *		12/08/97	bva		remove obsolete code
 *		11/21/97	bva		remove HTTP/1.1 test code
 *		11/14/97	pjr		add RomPagerDebug checks for fNestedDepth and
 *							fIndexDepth overflow.
 *		11/10/97	rhb		add support for 64 bit integers
 *		10/17/97	bva		fix double server push headers
 *		10/10/97	bva		fix compiler warnings
 *		10/07/97	pjr		initialize theNestingPtr->fRepeatWhileFunctionPtr
 *							in RpBuildHtmlPageReply for the
 *							eRpItemType_ItemGroup case.
 *		09/30/97	pjr		change the input string for RpHexToString to be
 *							an unsigned char * instead of a char *.  change
 *							NIBBLE_TO_HEX to generate lower case a-f.
 *		09/23/97	bva		add support for eRpItemType_FormNamedSubmit
 *		09/15/97	rhb		add support for eRpItemType_FormRadioButtonDyn,
 *								eRpItemType_FormFixedMultiDyn,
 *								eRpItemType_FormVariableSingleDyn, and
 *								eRpItemType_FormVariableMultiDyn
 *		09/14/97	bva		add eRpItemType_ImageSource support
 *		09/09/97	bva		add support for eRpItemType_FormFixedSingleDyn
 *		08/26/97	pjr		send kCRLF after MIME type string.
 *		08/25/97	bva		add page flow debugging
 *		08/09/97	bva		add termination to RpSendServerPushSeparator
 *		07/31/97	rhb		add eRpItemType_UrlState,
 *								eRpItemType_FormTextAreaBuf
 *		07/31/97	edk		rework user phrase dict to support more compilers
 *		07/25/97	bva		use kRpHexSeparator for eRpTextType_HexColonForm
 *		07/18/97	bva		fix compile errors for JavaScript
 *		07/18/97	rhb		check radio button for
 *							eRpItemType_FormRadioGroupDyn
 *		07/16/97	bva		add support for eRpItemType_FormTextDyn
 *		07/07/97	bva		add SendItemFormFile
 *		07/04/97	bva		fix RpExpandString buffer overflow problem by
 *							reworking SendDataOutZeroTerminated,
 *							RpExpandedMatch
 *		06/26/97	pjr		change SendFormHeader for form based file upload
 *		06/19/97	bva		add support for eRpItemType_RepeatGroupWhile
 *		06/12/97	bva		fURL -> fRefererUrl
 *		06/05/97	bva		change RpExpandString for kRpCompressionEscape
 *		05/25/97	bva		rework page dispatching
 * * * * Release 1.6 * * * *
 *		04/18/97	bva		fix RpExpandString for
 *								!fUserPhrasesCanBeCompressed
 *		04/07/97	bva		add kHtmlValueClose,
 *							change kHtmlChecked, kHtmlInputClose
 *		04/04/97	bva		cleanup warnings
 *		03/24/97	bva		server push debugging
 *		03/15/97	bva		server push
 *		03/05/97	bva		add support for eRpItemType_FormCheckboxDyn
 *		02/22/97	bva		added extended phrase dictionary support
 *		02/02/97	bva		add selectable user phrase dictionary
 *		01/22/97	bva		add SendNamedDisplayTextOut
 *		01/16/97	bva		rework rpFetchTextFuncPtr, rpFetchTextComplexPtr,
 *								rpFetchOptionComplexPtr for consistency
 *							theItemNumber in rpFetchTextComplexPtr
 *							becomes Unsigned16
 *		01/12/97	bva		FlipResponseBuffers -> RpFlipResponseBuffers
 *		12/27/96	bva		support GET for forms
 * * * * Release 1.5 * * * *
 *		11/05/96	bva		add conditional compiles
 *		10/22/96	bva		use RpBuildQueryValues
 *		10/20/96	bva		support complex functions for text area
 *		10/17/96	rhb		check for Html buffers overflow
 *		10/14/96	bva		add Java Script support for form items
 *		10/10/96	bva		add initializations for picky compilers
 *		10/07/96	rhb		add phrase dictionary
 *		09/24/96	rhb		support dynamically allocated engine data
 *		09/24/96	bva		add conditional compile flags for form items
 *		09/20/96	rhb		allow more than one HTTP request
 * * * * Release 1.4 * * * *
 *		08/16/96	bva		add support for eRpItemType_FormRadioGroupDyn
 *		08/13/96	bva		fix SendItemRadioButton for eRpVarType_Function
 *								GetPtr
 * * * * Release 1.3 * * * *
 *		07/30/96	bva		remove BooleanPtr, fAccess from pg/form structures
 *		07/29/96	bva		add SendDataOutFixedText
 *		07/23/96	bva		cleanup compiler warnings
 *		07/13/96	bva		add RepeatGroupDynamic support
 *		07/10/96	bva		add RpCatSigned32ToString and
 *								RpConvertSigned32ToAscii
 *		07/09/96	bva		add RepeatGroup support,
 *							add complex variable functions
 *		07/05/96	bva		merge page and form headers
 *		07/02/96	bva		fix SendDataOutLengthEncoded problem with buffer
 *								switch
 *		06/27/96	bva		added eRpItemType_HtmlReferer
 *		06/26/96	rhb		fix SendDataOutLengthEncoded problems
 *		06/24/96	rhb		added eRpTextType_HexColonForm
 *		06/18/96	bva		rework RpBuildHtmlPageReply to fix boundary
 *								condition and improve clarity
 *		06/13/96	bva		remove SendPasswordItem
 * * * * Release 1.2 * * * *
 *		06/01/96	bva		add RpCatUnsigned32ToString
 *		05/30/96	bva		cleanup to eliminate warnings
 * * * * Release 1.1 * * * *
 *		04/25/96	rhb		split apart fixed variable select types
 *		04/24/96	rhb		convert text items when GetPtr is function
 * * * * Release 1.0 * * * *
 *		03/10/96	bva		added indexed item support
 *		03/09/96	bva		added hidden field support
 *		02/16/96	bva		added html close support
 *		02/13/96	bva		added html title support
 *		01/05/96	bva		added group item support
 *		12/03/96	bva		add SendDisplayTextOut
 *		12/01/96	bva		send out trailing zeros with
 *								SendDataOutDecimalUnsigned
 *		11/08/95	bva		fGciCommandPtr -> fCgiCommandPtr
 *		11/01/95	rhb		created
 *
 *	To Do:
 */

#include "AsEngine.h"


#if RomPagerServer

static void SendDataOutDecimalSigned(rpHttpRequestPtr theRequestPtr,
				Signed32 theData);
#if AsUse64BitIntegers
static void SendDataOutDecimalUnsigned64(rpHttpRequestPtr theRequestPtr,
				Unsigned64 theData);
static void SendDataOutDecimalSigned64(rpHttpRequestPtr theRequestPtr,
				Signed64 theData);
#endif
static void SendDataOutDotForm(rpHttpRequestPtr theRequestPtr,
				Unsigned8 theLength,
				char *theHexDataPtr);
static void SendDataOutFixedText(rpHttpRequestPtr theRequestPtr,
				Unsigned8 theLength,
				char *theTextPtr);
static void SendDataOutHex(rpHttpRequestPtr theRequestPtr,
				Unsigned8 theLength,
				char theSeparator,
				char *theHexPtr);
static void SendDataOutLengthEncoded(rpHttpRequestPtr theRequestPtr,
				Unsigned16 theLength,
				char *theLengthEncodedPtr);
static void SendDisplayTextOut(rpDataPtr theDataPtr,
				rpTextDisplayItemPtr theTextDisplayItemPtr);
static void SendNamedDisplayTextOut(rpDataPtr theDataPtr,
				rpNamedTextDisplayItemPtr theTextDisplayItemPtr);

#if RpHtmlBufferDisplay
static Boolean	SendItemBuffer(rpHttpRequestPtr theRequestPtr,
					rpBufferDisplayItemPtr theBufferItemPtr);
#endif

#if RpHtmlCheckbox || RpHtmlCheckboxDynamic
static void SendItemCheckbox(rpDataPtr theDataPtr,
			char *	theNamePtr,
			void *	theGetPtr,
			rpVariableType theGetPtrType,
#if RomPagerJavaScript
			char *	theJavaScriptPtr,
#endif
			Boolean	theDynamicFlag);
#endif
#if RomPagerSecure
static void SendOutFullUrl(rpHttpRequestPtr theRequestPtr,
				rpFullUrlItemPtr theItemPtr, rpConnectionPtr theConnectionPtr);
static void SendOutHostName(rpHttpRequestPtr theRequestPtr,
				char *theTextPtr, rpConnectionPtr theConnectionPtr);
#endif
#if RpHtmlFormImage
static void SendItemImage(rpHttpRequestPtr theRequestPtr,
				rpImageFormItemPtr theItemPtr);
#endif
#if RpHtmlNamedSubmit || RomPagerJavaScript
static void SendItemNamedButton(rpDataPtr theDataPtr,
				rpCheckboxFormItemPtr theItemPtr,
				Boolean theSubmitFlag);
#endif
#if RpHtmlRadio || RpHtmlRadioDynamic
static void SendItemRadioButton(rpHttpRequestPtr theRequestPtr,
				rpRadioButtonFormItemPtr theItemPtr, Boolean theDynamicFlag);
#endif
#if RpHtmlRadioDynamic
static void SendItemRadioButtonDynamic(rpHttpRequestPtr theRequestPtr,
		rpRadioGroupInfoPtr theRadioGroupInfoPtr);
#endif
#if RomPagerForms
static void SendItemFormText(rpHttpRequestPtr theRequestPtr,
				rpTextFormItemPtr theItemPtr,
				rpItemType theItemType);
static void SendItemButton(rpHttpRequestPtr theRequestPtr,
				rpButtonFormItemPtr theItemPtr,
				Boolean theSubmitFlag);
static void SendFormHeader(rpHttpRequestPtr theRequestPtr,
				rpObjectDescriptionPtr theFormDescriptionPtr);
#endif
#if RpHtmlSelectFixedSingle
static void SendItemFixedSingleSelect(rpDataPtr theDataPtr,
				rpFixedSingleSelectFormItemPtr theItemPtr,
				Boolean theDynamicFlag);
#endif
#if RpHtmlSelectFixedMulti
static void	SendItemFixedMultiSelect(rpDataPtr theDataPtr, char *theNamePtr,
				rpOption_MultiSelectPtr theFirstOptionPtr,
				Unsigned8 theListLength,
#if RomPagerJavaScript
				char *theJavaScriptPtr,
#endif
				Boolean theDynamicFlag);
#endif
#if RpHtmlSelectVariable
static Boolean SendItemVariableSelect(rpDataPtr theDataPtr,
				rpVariableSelectFormItemPtr theItemPtr,
				Boolean theMultipleFlag,
				Boolean theDynamicFlag);
#endif
#if RpHtmlSelectVarValue
static Boolean SendItemVarValueSelect(rpDataPtr theDataPtr,
				rpVariableSelectFormItemPtr theItemPtr,
				Boolean theMultipleFlag,
				Boolean theDynamicFlag);
#endif
#if RpHtmlTextArea || RpHtmlTextAreaBuf
#if RpHtmlTextArea
static void SendItemTextArea(rpDataPtr theDataPtr,
				rpTextAreaFormItemPtr theItemPtr);
#endif
#if RpHtmlTextAreaBuf
static void SendItemTextAreaBuf(rpHttpRequestPtr theRequestPtr,
				rpTextAreaFormItemPtr theItemPtr);
#endif
static void SendItemTextAreaHeaders(rpHttpRequestPtr theRequestPtr,
				rpTextAreaFormItemPtr theItemPtr);
#endif
#if RomPagerFileUpload
static void SendItemFormFile(rpHttpRequestPtr theRequestPtr,
				rpFileFormItemPtr theItemPtr);
#endif
#if RpFileInsertItem
static void SendItemFile(rpHttpRequestPtr theRequestPtr, char *theFileNamePtr);
#endif
static void SendOutImageSource(rpHttpRequestPtr theRequestPtr,
				rpObjectDescriptionPtr theImageDescriptionPtr);
static void SendOutHtmlReferer(rpHttpRequestPtr theRequestPtr,
				char *theTextPtr);
#if RomPagerQueryIndex
static void SendOutIndexDisplay(rpHttpRequestPtr theRequestPtr,
				char *theTextPtr, Unsigned8 theOffset);
static void SendOutQueryDisplay(rpHttpRequestPtr theRequestPtr,
				char *theTextPtr);
static void SendOutQueryValues(rpHttpRequestPtr theRequestPtr,
				Boolean theForUrlFlag);
static rpItemPtr TestRepeatGroupWhile(rpDataPtr theDataPtr,
					rpHttpRequestPtr theRequestPtr,
					rpNestingPtr theNestingPtr);
#endif
#if RpHtmlRadio || RpHtmlRadioDynamic
static void SendOutRadioButtonChecked(rpDataPtr theDataPtr,
				rpRadioGroupInfoPtr theRadioGroupInfoPtr,
				rpOneOfSeveral theButtonNumber);
static void SendOutRadioButtonHeaders(rpHttpRequestPtr theRequestPtr,
				char *theNamePtr, Boolean theDynamicFlag);
#endif

#if RpHtmlSelectVariable || RpHtmlSelectVarValue || \
	RpHtmlSelectFixedSingle || RpHtmlSelectFixedMulti
static void SendSelectStart(rpHttpRequestPtr theRequestPtr,
					char *theNamePtr,
#if RomPagerJavaScript
					char *theJavaScriptPtr,
#endif
					Unsigned8 theListLength,
					Boolean theMultipleFlag,
					Boolean theDynamicFlag);
#endif

static void SendTextOut(rpDataPtr theDataPtr,
					rpVariableType theGetPtrType,
					void *theGetPtr,
					rpTextType theTextType,
					Unsigned8 theFieldSize,
					char *theNamePtr);
/* Arthur Chow 2005-11-03 : Handle the special input char such as (&quot;)" (&lt;)< (&gt;)> */
static void SendTextOut_Alpha_InputTextItem(rpDataPtr theDataPtr,
					rpVariableType theGetPtrType,
					void *theGetPtr,
					rpTextType theTextType,
					Unsigned8 theFieldSize,
					char *theNamePtr);


void RpBuildHtmlPageReply(rpHttpRequestPtr theRequestPtr) {
	rpItemPtr				theItemPtr;
	rpObjectDescriptionPtr	thePageDescriptionPtr;
	rpItemPtr				theNextItemPtr;
#if RpPageFlowDebug
	rpObjectExtensionPtr	theExtensionPtr;
	rpObjectFlags			theObjectFlags;
#endif

	theRequestPtr->fHtmlBufferReady = False;
	thePageDescriptionPtr = theRequestPtr->fObjectPtr;

	if (theRequestPtr->fItemState == eRpHtmlFirst) {
		theItemPtr = thePageDescriptionPtr->fItemsArrayPtr;
		theNextItemPtr = theItemPtr + 1;
		theRequestPtr->fItemState = eRpHtmlNext;
#if RpPageFlowDebug
		theExtensionPtr = thePageDescriptionPtr->fExtensionPtr;
		if (theExtensionPtr != (rpObjectExtensionPtr) 0) {
			theObjectFlags = theExtensionPtr->fFlags;
			if (theObjectFlags & kRpObjFlag_DebugFlow) {
				theRequestPtr->fDebugPageFlow = True;
				RP_PRINTF("\nBegin page - %s\n", thePageDescriptionPtr->fURL);
			}
		}
#endif
	}
	else {
		theItemPtr = theRequestPtr->fItemPtr;
		theNextItemPtr = theRequestPtr->fNextItemPtr;
	}
	RpBuildHtmlItemReply(theRequestPtr, theItemPtr, theNextItemPtr);
	return;
}


void RpBuildHtmlItemReply(rpHttpRequestPtr theRequestPtr,
		rpItemPtr theItemPtr, rpItemPtr theNextItemPtr) {
	rpDataPtr				theDataPtr;
	Boolean					theDoneFlag;
	rpDynamicDisplayItemPtr theDynamicDisplayItemPtr;
	Unsigned8				theIndex;
	rpNestingPtr			theNestingPtr;
#if RomPagerQueryIndex
	Signed16				theIncrement;
	Signed16				theLimit;
	rpRepeatGroupDynItemPtr	theRepeatGroupDynamicPtr;
	rpRepeatGroupItemPtr	theRepeatGroupItemPtr;
	Signed16				theStart;
#endif
#if RomPagerQueryIndex || RomMailer
	rpConnectionPtr			theConnectionPtr;
#endif

	theDoneFlag = False;
	theDataPtr = theRequestPtr->fDataPtr;
#if RomPagerQueryIndex || RomMailer
	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;
#endif

	while (!theDoneFlag) {
#if RomPagerSoftPages
		if (theItemPtr->fType == eRpItemType_NamedIndirect) {
			theItemPtr = &((rpNamedIndirectItemPtr) theItemPtr->fAddress)->fItem;
		}
#if RomPagerDebug
		if (theItemPtr->fType == eRpItemType_NamedIndirect) {
			RP_PRINTF("a Named Indirect item must not point to another Named Indirect item\n");
			break;
		}
#endif
#endif
		switch (theItemPtr->fType) {
			/*
				Handle state change items at this level,
				and handle data items in RpSendItem.
			*/
			case eRpItemType_LastItemInList:
				if (theRequestPtr->fNestedDepth != -1) {
					/*
						We're finishing a nested item.
					*/
					theNestingPtr =
							&theRequestPtr->fNestedItems[
								theRequestPtr->fNestedDepth];
#if RomPagerQueryIndex
					if (theNestingPtr->fRepeatWhileFunctionPtr !=
							(rpRepeatWhileFuncPtr) 0) {
						theItemPtr = TestRepeatGroupWhile(theDataPtr,
												theRequestPtr, theNestingPtr);
					}
					else if (theNestingPtr->fIndexIncrement == 0) {
						/*
							A non-indexed nested item, so just pop the state.
						*/
						theItemPtr = theNestingPtr->fReturnItemPtr;
						theRequestPtr->fNestedDepth -= 1;
					}
					else {
						/*
							We are in a Repeat Group, so see whether to stay
							in the loop or exit.
						*/
						if (theConnectionPtr->fIndexValues
								[theConnectionPtr->fIndexDepth] ==
								theNestingPtr->fIndexLimit) {
							/*
								We reached the limit, so just pop the state.
							*/
							theItemPtr = theNestingPtr->fReturnItemPtr;
							theRequestPtr->fNestedDepth -= 1;
							theConnectionPtr->fIndexDepth -= 1;
						}
						else {
							/*
								We need to repeat this group, so bump
								the index, and reset the item pointer
								to the beginning.
							*/
							theConnectionPtr->fIndexValues
									[theConnectionPtr->fIndexDepth] +=
									theNestingPtr->fIndexIncrement;
							theItemPtr = theNestingPtr->fRepeatItemPtr;
						}
					}
#else
					/*
						a non-indexed nested item, so just pop the state
					*/
					theItemPtr = theNestingPtr->fReturnItemPtr;
					theRequestPtr->fNestedDepth -= 1;
#endif
					theNextItemPtr = theItemPtr + 1;
				}
				else {
					/*
						The page is complete, so set up the transmit info
						for the last buffer.
					*/
					theRequestPtr->fHttpTransactionState =
							eRpHttpResponseComplete;
					theDoneFlag = True;
#if RomMailer
					if (theConnectionPtr->fProtocol == eRpSmtpClient &&
							theRequestPtr->fBase64State != eRpBase64_0) {
						/*
							We need to finish off the Base64 encoded object.
						*/
						RpSendOutBase64Padding(theRequestPtr);
						theRequestPtr->fBase64State = eRpBase64_0;
						theRequestPtr->fBase64LineLength = 0;
					}
#endif
					RpFlipResponseBuffers(theRequestPtr);
				}
				break;

			case eRpItemType_DynamicDisplay:
#if RpPageFlowDebug
				if (theRequestPtr->fDebugPageFlow) {
					RP_PRINTF("Processing eRpItemType_DynamicDisplay\n");
				}
#endif
				theDynamicDisplayItemPtr =
						(rpDynamicDisplayItemPtr) theItemPtr->fAddress;
				theIndex = RpGetDynamicDisplayIndex(theDataPtr,
						theDynamicDisplayItemPtr, (char *) kQuestion);
				/*
					set up the item pointer for the next real item
					and come back through the loop to send it out.
				*/
				if (theIndex < theDynamicDisplayItemPtr->fItemCount) {
					theItemPtr =
							theDynamicDisplayItemPtr->fItemsArrayPtr + theIndex;
				}
				else {
					/*
						The index was bad, so just up for the next item.
					*/
					theItemPtr = theNextItemPtr;
					theNextItemPtr = theItemPtr + 1;
				}
				break;

#if RomPagerQueryIndex
			case eRpItemType_ItemGroup:
			case eRpItemType_RepeatGroup:
			case eRpItemType_RepeatGroupDynamic:
			case eRpItemType_RepeatGroupWhile:
#if RpPageFlowDebug
				if (theRequestPtr->fDebugPageFlow) {
					switch (theItemPtr->fType) {
						case eRpItemType_ItemGroup:
							RP_PRINTF("Processing eRpItemType_ItemGroup\n");
							break;
						case eRpItemType_RepeatGroup:
							RP_PRINTF("Processing eRpItemType_RepeatGroup\n");
							break;
						case eRpItemType_RepeatGroupDynamic:
							RP_PRINTF("Processing eRpItemType_RepeatGroupDynamic\n");
							break;
						case eRpItemType_RepeatGroupWhile:
							RP_PRINTF("Processing eRpItemType_RepeatGroupWhile\n");
						default:
							break;
					}
				}
#endif

#if RomPagerDebug
				if (theRequestPtr->fNestedDepth >= (kAsIndexQueryDepth + 2)) {
					RP_PRINTF("fNestedDepth overflow, aborting item!\n");
					theItemPtr = theNextItemPtr;
					theNextItemPtr = theItemPtr + 1;
					break;
				}
#endif
				theRequestPtr->fNestedDepth += 1;
				theNestingPtr = &theRequestPtr->fNestedItems[
						theRequestPtr->fNestedDepth];
				theNestingPtr->fReturnItemPtr = theNextItemPtr;
				if (theItemPtr->fType == eRpItemType_ItemGroup) {
					theNestingPtr->fRepeatWhileFunctionPtr =
							(rpRepeatWhileFuncPtr) 0;
					theNestingPtr->fIndexIncrement = 0;
					theItemPtr = (rpItemPtr) theItemPtr->fAddress;
				}
				else if (theItemPtr->fType == eRpItemType_RepeatGroupWhile) {
#if RomPagerDebug
					if (theConnectionPtr->fIndexDepth >=
							(kAsIndexQueryDepth - 1)) {
						RP_PRINTF("fIndexDepth overflow, aborting item!\n");
						theItemPtr = theNextItemPtr;
						theNextItemPtr = theItemPtr + 1;
						break;
					}
#endif
					theConnectionPtr->fIndexDepth += 1;
					theConnectionPtr->fIndexValues
							[theConnectionPtr->fIndexDepth] = 0;
					theRepeatGroupDynamicPtr =
							(rpRepeatGroupDynItemPtr) theItemPtr->fAddress;
					theNestingPtr->fRepeatWhileFunctionPtr =
							(rpRepeatWhileFuncPtr)
							theRepeatGroupDynamicPtr->fFunctionPtr;
					theNestingPtr->fRepeatItemPtr =
							theRepeatGroupDynamicPtr->fItemsArrayPtr;

					/*
						In a Repeat Group While, the IndexIncrement is used
						to hold the index value that will be stored in the
						index values array.
					*/
					theNestingPtr->fIndexIncrement = 0;
					theNestingPtr->fRepeatWhileValue = (void *) 0;
					theItemPtr = TestRepeatGroupWhile(theDataPtr,
							theRequestPtr, theNestingPtr);
				}
				else {
#if RomPagerDebug
					if (theConnectionPtr->fIndexDepth >=
							(kAsIndexQueryDepth - 1)) {
						RP_PRINTF("fIndexDepth overflow, aborting item!\n");
						theItemPtr = theNextItemPtr;
						theNextItemPtr = theItemPtr + 1;
						break;
					}
#endif
					theConnectionPtr->fIndexDepth += 1;
					theNestingPtr->fRepeatWhileFunctionPtr =
							(rpRepeatWhileFuncPtr) 0;
					if (theItemPtr->fType == eRpItemType_RepeatGroup) {
						theRepeatGroupItemPtr =
								(rpRepeatGroupItemPtr) theItemPtr->fAddress;
						theIncrement = theRepeatGroupItemPtr->fIndexIncrement;
						theLimit = theRepeatGroupItemPtr->fIndexLimit;
						theStart = theRepeatGroupItemPtr->fIndexStart;
						theItemPtr = theRepeatGroupItemPtr->fItemsArrayPtr;
					}
					else {
						/*
							must be eRpItemType_RepeatGroupDynamic
						*/
						theRepeatGroupDynamicPtr =
								(rpRepeatGroupDynItemPtr) theItemPtr->fAddress;
						(*(rpDynamicRepeatFuncPtr)
								theRepeatGroupDynamicPtr->fFunctionPtr)
								(theDataPtr, &theStart,
								&theLimit, &theIncrement);
						theItemPtr = theRepeatGroupDynamicPtr->fItemsArrayPtr;
					}
					theNestingPtr->fIndexIncrement = theIncrement;

					/*
						Index starts and limits are defined 1-relative and
						stored 0-relative, so that calls to the complex
						Set/Get routines are 0-relative.  External query
						and index displays are 1-relative.
					*/
					theNestingPtr->fIndexLimit = theLimit - 1;
					theConnectionPtr->fIndexValues
							[theConnectionPtr->fIndexDepth] = theStart - 1;
					theNestingPtr->fRepeatItemPtr = theItemPtr;
				}
#else
			case eRpItemType_ItemGroup:
#if RpPageFlowDebug
				if (theRequestPtr->fDebugPageFlow) {
					RP_PRINTF("Processing eRpItemType_ItemGroup\n");
				}
#endif
				theNestingPtr->fIndexIncrement = 0;
				theItemPtr = (rpItemPtr) theItemPtr->fAddress;

#endif	/* RomPagerQueryIndex */
				theNextItemPtr = theItemPtr + 1;
				break;

#if RomPagerSoftPages
			case eRpItemType_EndOfList:
				if (theRequestPtr->fHtmlBufferReady) {
					theRequestPtr->fItemPtr = theItemPtr;
				}
				theDoneFlag = True;
				break;
#endif

#if RpFileInsertItem
			case eRpItemType_File:
				RpSendItem(theRequestPtr, theItemPtr);
				theDoneFlag = True;
					theItemPtr = theNextItemPtr;
					theNextItemPtr = theItemPtr + 1;
				theRequestPtr->fItemPtr = theItemPtr;
				theRequestPtr->fNextItemPtr = theNextItemPtr;
				break;
#endif

			default:
				/*
					Send out the data items.
					RpSendItem returns True if the item was completely
					handled, and False if we need to process the
					item further after sending a filled buffer.
				*/
				if (RpSendItem(theRequestPtr, theItemPtr)) {
					theItemPtr = theNextItemPtr;
					theNextItemPtr = theItemPtr + 1;
				}
				if (theRequestPtr->fHtmlBufferReady) {
					/*
						We have a filled buffer, but the page is not yet
						complete. theRequestPtr->fItemPtr points to the
						next item to be sent when we get to this routine again.
					*/
					theDoneFlag = True;
					theRequestPtr->fItemPtr = theItemPtr;
					theRequestPtr->fNextItemPtr = theNextItemPtr;
				}
				break;
		}	/*	switch	*/
	}	/*	while	*/
	return;
}


Unsigned8 RpGetDynamicDisplayIndex(rpDataPtr theDataPtr,
				rpDynamicDisplayItemPtr theDynamicDisplayItemPtr,
				char *theNamePtr) {
	void *				theGetPtr;
	rpVariableType		theGetPtrType;
	Unsigned8			theIndex;
	Signed16Ptr			theIndexValuesPtr;

	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
	theGetPtrType = theDynamicDisplayItemPtr->fGetPtrType;
	theGetPtr = theDynamicDisplayItemPtr->fGetPtr;
	if (theGetPtrType == eRpVarType_Direct) {
		theIndex = *(Unsigned8Ptr) (theGetPtr);
	}
	else if (theGetPtrType == eRpVarType_Function) {
		theIndex = (*(rpFetchIndexFuncPtr) theGetPtr)();
	}
#if AsSnmpAccess
	else if (theGetPtrType == eRpVarType_Snmp) {
		theIndex = *((Unsigned8Ptr) AsRetrieveSnmpItem(theDataPtr,
												theNamePtr,
												theIndexValuesPtr,
												eAsTextType_Unsigned8,
												theGetPtr));
	}
#endif
#if AsCustomVariableAccess
	else if (theGetPtrType == eRpVarType_Custom) {
		theIndex = *((Boolean *) AsRetrieveCustomItem(theDataPtr,
												theNamePtr,
												theIndexValuesPtr,
												eAsTextType_Unsigned8));
	}
#endif
	else {
		theIndex =
				(*(rpFetchIndexComplexPtr) theGetPtr)(theDataPtr,
				theIndexValuesPtr);
	}

	return theIndex;
}


Boolean RpSendItem(rpHttpRequestPtr theRequestPtr, rpItemPtr theItemPtr) {
	rpDataPtr							theDataPtr;
	Boolean								theItemCompleteFlag;
	Unsigned16							theLength = 0;
#if RpHtmlCheckbox
	rpCheckboxFormItemPtr				theCheckboxItemPtr;
#endif
#if RpHtmlCheckboxDynamic
	rpDynCheckboxFormItemPtr			theDynCheckboxItemPtr;
#endif
#if RpHtmlSelectFixedMulti
	rpFixedMultiSelectFormItemPtr		theFixedMultiItemPtr;
	rpFixedMultiDynSelectFormItemPtr	theFixedMultiDynItemPtr;
#endif

	theDataPtr = theRequestPtr->fDataPtr;
	theItemCompleteFlag = True;

	switch (theItemPtr->fType) {
		case eRpItemType_DataZeroTerminated:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_DataZeroTerminated\n");
			}
#endif
			RpSendDataOutZeroTerminated(theRequestPtr,
					(char *) theItemPtr->fAddress);
			break;

		case eRpItemType_DataLengthEncoded:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_DataLengthEncoded\n");
			}
#endif
			theLength = (*((Unsigned8Ptr) theItemPtr->fAddress) << 8) +
						*((Unsigned8Ptr) theItemPtr->fAddress + 1);
#if RomMailer
			if (theDataPtr->fCurrentConnectionPtr->fProtocol == eRpSmtpClient) {
				RpSendDataOutBase64(theRequestPtr, theLength,
					((char *) theItemPtr->fAddress) + 2 * sizeof(Unsigned8));
			}
			else {
				SendDataOutLengthEncoded(theRequestPtr, theLength,
					((char *) theItemPtr->fAddress) + 2 * sizeof(Unsigned8));
			}
#else
			SendDataOutLengthEncoded(theRequestPtr, theLength,
					((char *) theItemPtr->fAddress) + 2 * sizeof(Unsigned8));
#endif
			break;

		case eRpItemType_ExtendedAscii:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_ExtendedAscii\n");
			}
#endif
			RpSendDataOutExtendedAscii(theRequestPtr,
					(char *) theItemPtr->fAddress);
			break;

		case eRpItemType_DisplayText:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_DisplayText\n");
			}
#endif
			SendDisplayTextOut(theDataPtr,
					(rpTextDisplayItemPtr) theItemPtr->fAddress);
			break;

		case eRpItemType_NamedDisplayText:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_NamedDisplayText\n");
			}
#endif
			SendNamedDisplayTextOut(theDataPtr,
					(rpNamedTextDisplayItemPtr) theItemPtr->fAddress);
			break;

#if RpHtmlCheckbox
		case eRpItemType_FormCheckbox:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormCheckbox\n");
			}
#endif
			theCheckboxItemPtr = (rpCheckboxFormItemPtr) theItemPtr->fAddress;
			SendItemCheckbox(theDataPtr, theCheckboxItemPtr->fNamePtr,
					theCheckboxItemPtr->fGetPtr,
					theCheckboxItemPtr->fGetPtrType,
#if RomPagerJavaScript
					theCheckboxItemPtr->fJavaScriptPtr,
#endif
					False);
			break;
#endif

#if RpHtmlNamedSubmit
		case eRpItemType_FormNamedSubmit:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormNamedSubmit\n");
			}
#endif
			SendItemNamedButton(theDataPtr,
					(rpCheckboxFormItemPtr) theItemPtr->fAddress, True);
			break;
#endif

#if RomPagerJavaScript && RomPagerForms
		case eRpItemType_FormButton:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormButton\n");
			}
#endif
			SendItemNamedButton(theDataPtr,
					(rpCheckboxFormItemPtr) theItemPtr->fAddress, False);
			break;
#endif

#if RpHtmlRadio
		case eRpItemType_FormRadioButton:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormRadioButton\n");
			}
#endif
			SendItemRadioButton(theRequestPtr,
					(rpRadioButtonFormItemPtr) theItemPtr->fAddress, False);
			break;
#endif

#if RpHtmlRadioDynamic
		case eRpItemType_FormRadioButtonDyn:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormRadioButtonDyn\n");
			}
#endif
			SendItemRadioButton(theRequestPtr,
					(rpRadioButtonFormItemPtr) theItemPtr->fAddress, True);
			break;

		case eRpItemType_FormRadioGroupDyn:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormRadioGroupDyn\n");
			}
#endif
			SendItemRadioButtonDynamic(theRequestPtr,
					(rpRadioGroupInfoPtr) theItemPtr->fAddress);
			break;
#endif

#if RpHtmlCheckboxDynamic
		case eRpItemType_FormCheckboxDyn:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormCheckboxDyn\n");
			}
#endif
			theDynCheckboxItemPtr =
					(rpDynCheckboxFormItemPtr) theItemPtr->fAddress;
			SendItemCheckbox(theDataPtr, theDynCheckboxItemPtr->fNamePtr,
					theDynCheckboxItemPtr->fGetPtr,
					theDynCheckboxItemPtr->fGetPtrType,
#if RomPagerJavaScript
					theDynCheckboxItemPtr->fJavaScriptPtr,
#endif
					True);
			break;
#endif

#if RomPagerForms
		case eRpItemType_FormHeader:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormHeader\n");
			}
#endif
			SendFormHeader(theRequestPtr,
					(rpObjectDescriptionPtr) theItemPtr->fAddress);
			break;

#if RpHtmlTextFormDynamic
		case eRpItemType_FormTextDyn:
		case eRpItemType_FormPasswordDyn:
		case eRpItemType_FormHiddenDyn:
#endif
		case eRpItemType_FormAsciiText:
		case eRpItemType_FormPasswordText:
		case eRpItemType_FormHiddenText:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormText\n");
			}
#endif
			SendItemFormText(theRequestPtr,
					(rpTextFormItemPtr) theItemPtr->fAddress,
					theItemPtr->fType);
			break;

		case eRpItemType_FormSubmit:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormSubmit\n");
			}
#endif
			SendItemButton(theRequestPtr,
					(rpButtonFormItemPtr) theItemPtr->fAddress, True);
			break;

		case eRpItemType_FormReset:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormReset\n");
			}
#endif
			SendItemButton(theRequestPtr,
					(rpButtonFormItemPtr) theItemPtr->fAddress, False);
			break;
#endif

#if RpHtmlSelectFixedSingle
		case eRpItemType_FormFixedSingleSelect:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormFixedSingleSelect\n");
			}
#endif
			SendItemFixedSingleSelect(theDataPtr,
					(rpFixedSingleSelectFormItemPtr) theItemPtr->fAddress,
					False);
			break;

		case eRpItemType_FormFixedSingleDyn:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormFixedSingleDyn\n");
			}
#endif
			SendItemFixedSingleSelect(theDataPtr,
					(rpFixedSingleSelectFormItemPtr) theItemPtr->fAddress,
					True);
			break;
#endif

#if RpHtmlSelectFixedMulti
		case eRpItemType_FormFixedMultiSelect:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormFixedMultiSelect\n");
			}
#endif
			theFixedMultiItemPtr =
					(rpFixedMultiSelectFormItemPtr) theItemPtr->fAddress;
			SendItemFixedMultiSelect(theDataPtr,
					theFixedMultiItemPtr->fNamePtr,
					theFixedMultiItemPtr->fFirstOptionPtr,
					theFixedMultiItemPtr->fListLength,
#if RomPagerJavaScript
					theFixedMultiItemPtr->fJavaScriptPtr,
#endif
					False);
			break;

		case eRpItemType_FormFixedMultiDyn:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormFixedMultiDyn\n");
			}
#endif
			theFixedMultiDynItemPtr =
					(rpFixedMultiDynSelectFormItemPtr) theItemPtr->fAddress;
			SendItemFixedMultiSelect(theDataPtr,
					theFixedMultiDynItemPtr->fNamePtr,
					theFixedMultiDynItemPtr->fFirstOptionPtr,
					theFixedMultiDynItemPtr->fListLength,
#if RomPagerJavaScript
					theFixedMultiDynItemPtr->fJavaScriptPtr,
#endif
					True);
			break;
#endif


#if RpHtmlSelectVariable
		case eRpItemType_FormVariableSingleSelect:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormVariableSingleSelect\n");
			}
#endif
			theItemCompleteFlag = SendItemVariableSelect(theDataPtr,
					(rpVariableSelectFormItemPtr) theItemPtr->fAddress,
					False, False);
			break;

		case eRpItemType_FormVariableMultiSelect:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormVariableMultiSelect\n");
			}
#endif
			theItemCompleteFlag = SendItemVariableSelect(theDataPtr,
					(rpVariableSelectFormItemPtr) theItemPtr->fAddress,
					True, False);
			break;

		case eRpItemType_FormVariableSingleDyn:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormVariableSingleDyn\n");
			}
#endif
			theItemCompleteFlag = SendItemVariableSelect(theDataPtr,
					(rpVariableSelectFormItemPtr) theItemPtr->fAddress,
					False, True);
			break;

		case eRpItemType_FormVariableMultiDyn:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormVariableMultiDyn\n");
			}
#endif
			theItemCompleteFlag = SendItemVariableSelect(theDataPtr,
					(rpVariableSelectFormItemPtr) theItemPtr->fAddress,
					True, True);
			break;
#endif

#if RpHtmlSelectVarValue
		case eRpItemType_FormVarValueSingleSelect:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormVarValueSingleSelect\n");
			}
#endif
			theItemCompleteFlag = SendItemVarValueSelect(theDataPtr,
					(rpVariableSelectFormItemPtr) theItemPtr->fAddress,
					False, False);
			break;

		case eRpItemType_FormVarValueMultiSelect:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormVarValueMultiSelect\n");
			}
#endif
			theItemCompleteFlag = SendItemVarValueSelect(theDataPtr,
					(rpVariableSelectFormItemPtr) theItemPtr->fAddress,
					True, False);
			break;

		case eRpItemType_FormVarValueSingleDyn:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormVarValueSingleDyn\n");
			}
#endif
			theItemCompleteFlag = SendItemVarValueSelect(theDataPtr,
					(rpVariableSelectFormItemPtr) theItemPtr->fAddress,
					False, True);
			break;

		case eRpItemType_FormVarValueMultiDyn:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormVarValueMultiDyn\n");
			}
#endif
			theItemCompleteFlag = SendItemVarValueSelect(theDataPtr,
					(rpVariableSelectFormItemPtr) theItemPtr->fAddress,
					True, True);
			break;
#endif

#if RpHtmlTextArea
		case eRpItemType_FormTextArea:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormTextArea\n");
			}
#endif
			SendItemTextArea(theDataPtr,
					(rpTextAreaFormItemPtr) theItemPtr->fAddress);
			break;
#endif

#if RpHtmlTextAreaBuf
		case eRpItemType_FormTextAreaBuf:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormTextAreaBuf\n");
			}
#endif
			SendItemTextAreaBuf(theRequestPtr,
					(rpTextAreaFormItemPtr) theItemPtr->fAddress);
			break;
#endif

#if RomPagerFileUpload
		case eRpItemType_FormFile:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormFile\n");
			}
#endif
			SendItemFormFile(theRequestPtr,
					(rpFileFormItemPtr) theItemPtr->fAddress);
			break;
#endif

#if RpFileInsertItem
		case eRpItemType_File:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_File\n");
			}
#endif
			SendItemFile(theRequestPtr, (char *) theItemPtr->fAddress);
			break;
#endif

#if RomPagerUrlState
		case eRpItemType_UrlState:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_UrlState\n");
			}
#endif
			RpSendDataOutZeroTerminated(theRequestPtr, (char *) theItemPtr->fAddress);
			RpSendDataOutZeroTerminated(theRequestPtr, kUrlStatePrefix);
			RpSendDataOutZeroTerminated(theRequestPtr, theRequestPtr->fUrlState);
			break;
#endif

#if AsSnmpAccess
		case eRpItemType_SnmpDisplay:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_SnmpAccess\n");
			}
#endif
			RpSendOutSnmpDisplay(theRequestPtr,
					(asSnmpAccessItemPtr) theItemPtr->fAddress);
			break;
#endif

#if RomPagerQueryIndex
		case eRpItemType_IndexDisplay_4:
		case eRpItemType_IndexDisplay_3:
		case eRpItemType_IndexDisplay_2:
		case eRpItemType_IndexDisplay_1:
		case eRpItemType_IndexDisplay_0:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_IndexDisplay_n\n");
			}
#endif
			SendOutIndexDisplay(theRequestPtr,
					(char *) theItemPtr->fAddress,
					(Unsigned8) (theItemPtr->fType -
					eRpItemType_IndexDisplay_0));
			break;

		case eRpItemType_QueryDisplay:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_QueryDisplay\n");
			}
#endif
			SendOutQueryDisplay(theRequestPtr,
					(char *) theItemPtr->fAddress);
			break;
#endif


#if RomPagerSecure
		case eRpItemType_HostName:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_HostName\n");
			}
#endif
			SendOutHostName(theRequestPtr,
					(char *) theItemPtr->fAddress,
					theDataPtr->fCurrentConnectionPtr);
			break;

		case eRpItemType_FullUrl:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FullUrl\n");
			}
#endif
			SendOutFullUrl(theRequestPtr,
					(rpFullUrlItemPtr) theItemPtr->fAddress,
					theDataPtr->fCurrentConnectionPtr);
			break;
#endif	/* RomPagerSecure */


#if RpHtmlFormImage
		case eRpItemType_FormImage:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_FormImage\n");
			}
#endif
			SendItemImage(theRequestPtr,
					(rpImageFormItemPtr) theItemPtr->fAddress);
			break;
#endif	/* RpHtmlFormImage */


		case eRpItemType_ImageSource:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_ImageSource\n");
			}
#endif
			SendOutImageSource(theRequestPtr,
					(rpObjectDescriptionPtr) theItemPtr->fAddress);
			break;

		case eRpItemType_HtmlReferer:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_HtmlReferer\n");
			}
#endif
			SendOutHtmlReferer(theRequestPtr, (char *) theItemPtr->fAddress);
			break;

#if RpHtmlBufferDisplay
		case eRpItemType_BufferDisplay:
#if RpPageFlowDebug
			if (theRequestPtr->fDebugPageFlow) {
				RP_PRINTF("Processing eRpItemType_BufferDisplay\n");
			}
#endif
			theItemCompleteFlag = SendItemBuffer(theRequestPtr,
					(rpBufferDisplayItemPtr) theItemPtr->fAddress);
			break;
#endif

		default:
#if RomPagerDebug

			/*
				Set a breakpoint at this innocuous instruction to catch
				incorrectly coded item types.
			*/
			theLength = theLength * 2;
#endif
			break;
	}

	return theItemCompleteFlag;
}


#if RomPagerForms
static void SendFormHeader(rpHttpRequestPtr theRequestPtr,
		rpObjectDescriptionPtr theFormDescriptionPtr) {
	Unsigned8		theFlags;
	char *			theSchemePtr;

	if (theFormDescriptionPtr->fMimeDataType == eRpDataTypeFormMultipart) {
		/*
			This is a multipart form (used for file upload) so send out
			the enclosure type.
		*/
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlFormHeaderMulti);
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlFormHeaderPost);
	}
	else {
		/*
			This is a regular form so send out the form header using the
			default enclosure type.
		*/
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlFormHeader);
		if (theFormDescriptionPtr->fMimeDataType == eRpDataTypeFormGet) {
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlFormHeaderGet);
		}
		else {
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlFormHeaderPost);
		}
	}

	/*
		Send out "Action=". Send scheme and IP address if scheme is forced
		in the object extension.
	*/
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlFormHeaderAction);
	theFlags = theFormDescriptionPtr->fExtensionPtr->fFlags;
	if (theFlags & kRpObjFlag_ForceHttp) {
		/*
			Force http.
		*/
		theSchemePtr = kHttpString;
	}
	else if (theFlags & kRpObjFlag_ForceHttps) {
		/*
			Force https.
		*/
		theSchemePtr = kHttpsString;
	}
	else {
		/*
			No scheme is forced.
		*/
		theSchemePtr = (char *) 0;
	}

	if (theSchemePtr != (char *) 0) {
		/*
			Send the scheme and IP address.
		*/
		RpSendDataOutZeroTerminated(theRequestPtr, theSchemePtr);
		SendDataOutDotForm(theRequestPtr, 4, 
			(char *) &theRequestPtr->fDataPtr->fCurrentConnectionPtr->fIpLocal);
#if RomPagerUrlState
		/*
			Send the URL state if there is one.
		*/
		if (theRequestPtr->fUrlState[0] != '\0') {
			RpSendDataOutZeroTerminated(theRequestPtr, kUrlStatePrefix);
			RpSendDataOutZeroTerminated(theRequestPtr, theRequestPtr->fUrlState);
		}
#endif
		/*
			The ACTION URL has a prefix, so send it out the way it is stored.
		*/
		RpSendDataOutZeroTerminated(theRequestPtr, theFormDescriptionPtr->fURL);
	}
	else {
		/*
			There is no "http" prefix to the URL, so it is a relative URL. So, we
			want to skip past the initial '/' in the pathname when we put it
			out, so that the URL is relative. This makes forms more usable when
			a RomPager server is the target of another RomPager server with
			the Remote Host proxy option.
		*/
#if RomPagerUrlState
		/*
			Send the URL state if there is one.
		*/
		if (theRequestPtr->fUrlState[0] != '\0') {
			RpSendDataOutZeroTerminated(theRequestPtr, kUrlStateRelativePrefix);
			RpSendDataOutZeroTerminated(theRequestPtr, theRequestPtr->fUrlState);
			RpSendDataOutZeroTerminated(theRequestPtr, theFormDescriptionPtr->fURL);
		}
		else {
			RpSendDataOutZeroTerminated(theRequestPtr, theFormDescriptionPtr->fURL + 1);
		}
#else
		RpSendDataOutZeroTerminated(theRequestPtr, theFormDescriptionPtr->fURL + 1);
#endif	/* RomPagerUrlState */
	}
#if RomPagerQueryIndex
	SendOutQueryValues(theRequestPtr, True);
#endif
#if RomPagerJavaScript
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueClose);
	RpSendDataOutZeroTerminated(theRequestPtr,
				theFormDescriptionPtr->fExtensionPtr->fJavaScriptPtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
#else
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueInputClose);
#endif
	return;
}
#endif	/* RomPagerForms */


#if RpHtmlBufferDisplay
static Boolean SendItemBuffer(rpHttpRequestPtr theRequestPtr,
								rpBufferDisplayItemPtr theBufferItemPtr) {
	rpDataPtr				theDataPtr;
	Unsigned16				theLength;
	Boolean					theItemComplete;
	void *					theGetPtr;
	Signed16Ptr				theIndexValuesPtr;

	theDataPtr = theRequestPtr->fDataPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
	theGetPtr = theBufferItemPtr->fGetPtr;
	theItemComplete = False;

	/*
		See if we already have some data.
	*/
	if (theRequestPtr->fBufferItemLength == 0) {
		/*
			Get some buffer data from the user routine.
		*/
		theRequestPtr->fBufferItemLastBuffer =
				(*(rpFetchBufferComplexPtr) theGetPtr)(theDataPtr,
				theBufferItemPtr->fNamePtr, theIndexValuesPtr,
				&theRequestPtr->fBufferItemPtr,
				&theRequestPtr->fBufferItemLength);
	}

	/*
		If we got any data, process it.
	*/
	if (theRequestPtr->fBufferItemLength > 0) {

		if (theRequestPtr->fBufferItemLength >
				theRequestPtr->fFillBufferAvailable) {
			theLength = theRequestPtr->fFillBufferAvailable;
		}
		else {
			theLength = (Unsigned16) theRequestPtr->fBufferItemLength;
		}

		RP_MEMCPY(theRequestPtr->fHtmlFillPtr,
					theRequestPtr->fBufferItemPtr, theLength);
		theRequestPtr->fHtmlFillPtr += theLength;
		theRequestPtr->fFillBufferAvailable -= theLength;
		theRequestPtr->fBufferItemPtr += theLength;
		theRequestPtr->fBufferItemLength -= theLength;

		if (theRequestPtr->fFillBufferAvailable == 0) {
			RpFlipResponseBuffers(theRequestPtr);
		}
	}

	if (theRequestPtr->fBufferItemLastBuffer &&
			theRequestPtr->fBufferItemLength == 0) {
		/*
			This is the last buffer from the user and we've completely
			processed it.
		*/
		theRequestPtr->fBufferItemLastBuffer = False;
		theItemComplete = True;
	}

	return theItemComplete;
}
#endif	/* RpHtmlBufferDisplay */


#if RpHtmlCheckbox || RpHtmlCheckboxDynamic
static void SendItemCheckbox(rpDataPtr theDataPtr,
			char *	theNamePtr,
			void *	theGetPtr,
			rpVariableType theGetPtrType,
#if RomPagerJavaScript
			char *	theJavaScriptPtr,
#endif
			Boolean	theDynamicFlag) {
	Signed16Ptr			theIndexValuesPtr;
	rpHttpRequestPtr	theRequestPtr;
	Boolean				theValueFlag;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputType);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlCheckbox);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlName);
	RpSendDataOutZeroTerminated(theRequestPtr, theNamePtr);
#if RpHtmlCheckboxDynamic
	if (theDynamicFlag) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValue);
		SendOutQueryValues(theRequestPtr, False);
	}
#endif
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueClose);
	if (theGetPtr != 0) {
		if (theGetPtrType == eRpVarType_Direct) {
			theValueFlag = *(Boolean *) theGetPtr;
		}
		else if (theGetPtrType == eRpVarType_Function) {
			theValueFlag = (*(rpFetchBooleanFuncPtr) theGetPtr)();
		}
#if AsSnmpAccess
		else if (theGetPtrType == eRpVarType_Snmp) {
			theValueFlag = *((Boolean *) AsRetrieveSnmpItem(theDataPtr,
							theNamePtr, theIndexValuesPtr,
							eAsTextType_Unsigned8, theGetPtr));
		}
#endif
#if AsCustomVariableAccess
		else if (theGetPtrType == eRpVarType_Custom) {
			theValueFlag = *((Boolean *) AsRetrieveCustomItem(theDataPtr,
							theNamePtr, theIndexValuesPtr,
							eAsTextType_Unsigned8));
		}
#endif
		else {
			theValueFlag = (*(rpFetchBooleanComplexPtr) theGetPtr)(theDataPtr,
							theNamePtr, theIndexValuesPtr);
		}
		if (theValueFlag) {
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlChecked);
		}
	}
#if RomPagerJavaScript
	RpSendDataOutZeroTerminated(theRequestPtr, theJavaScriptPtr);
#endif
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
	return;
}
#endif

#if RpHtmlNamedSubmit || RomPagerJavaScript
static void SendItemNamedButton(rpDataPtr theDataPtr,
								rpCheckboxFormItemPtr theItemPtr,
								Boolean theSubmitFlag) {
	char *				theBytesPtr;
	Signed16Ptr			theIndexValuesPtr;
	rpHttpRequestPtr	theRequestPtr;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;

	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputType);
	if (theSubmitFlag) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSubmit);
	}
	else {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlButton);
	}
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlName);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fNamePtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValue);

	theBytesPtr = RpGetBytesPtr(theDataPtr,
								theItemPtr->fGetPtr,
								theItemPtr->fGetPtrType,
								theItemPtr->fNamePtr,
								theIndexValuesPtr);
	RpSendDataOutZeroTerminated(theRequestPtr, theBytesPtr);

#if RomPagerJavaScript
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueClose);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fJavaScriptPtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
#else
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueInputClose);
#endif
	return;
}
#endif


#if RpHtmlRadio || RpHtmlRadioDynamic
static void SendItemRadioButton(rpHttpRequestPtr theRequestPtr,
		rpRadioButtonFormItemPtr theItemPtr, Boolean theDynamicFlag) {
	rpRadioGroupInfoPtr	theRadioGroupInfoPtr;

	theRadioGroupInfoPtr = theItemPtr->fRadioGroupPtr;
	SendOutRadioButtonHeaders(theRequestPtr, theRadioGroupInfoPtr->fNamePtr,
		theDynamicFlag);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fValue);
	SendOutRadioButtonChecked(theRequestPtr->fDataPtr,
								theRadioGroupInfoPtr,
								theItemPtr->fButtonNumber);
#if RomPagerJavaScript
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fJavaScriptPtr);
#endif
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
	return;
}
#endif

#if RpHtmlRadioDynamic
static void SendItemRadioButtonDynamic(rpHttpRequestPtr theRequestPtr,
		rpRadioGroupInfoPtr theRadioGroupInfoPtr) {
	rpOneOfSeveral		theButtonNumber;
	rpConnectionPtr		theConnectionPtr;

	SendOutRadioButtonHeaders(theRequestPtr, theRadioGroupInfoPtr->fNamePtr,
								False);
	theConnectionPtr = theRequestPtr->fDataPtr->fCurrentConnectionPtr;
	theButtonNumber =
			theConnectionPtr->fIndexValues[theConnectionPtr->fIndexDepth] + 1;
	RpSendDataOutDecimalUnsigned(theRequestPtr, (Unsigned32) theButtonNumber);
	SendOutRadioButtonChecked(theRequestPtr->fDataPtr,
								theRadioGroupInfoPtr,
								theButtonNumber);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
	return;
}
#endif


#if RpHtmlRadio || RpHtmlRadioDynamic
static void SendOutRadioButtonHeaders(rpHttpRequestPtr theRequestPtr,
										char *theNamePtr,
										Boolean theDynamicFlag) {

#if RomPagerDebug && !RpHtmlRadioDynamic
	if (theDynamicFlag) {
		RP_PRINTF("SendOutRadioButtonHeaders needs RpHtmlRadioDynamic\n");
	}
#endif

	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputType);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlRadio);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlName);
	RpSendDataOutZeroTerminated(theRequestPtr, theNamePtr);
#if RpHtmlRadioDynamic
	if (theDynamicFlag) {
		SendOutQueryValues(theRequestPtr, False);
	}
#endif
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValue);
	return;
}


static void SendOutRadioButtonChecked(rpDataPtr theDataPtr,
									rpRadioGroupInfoPtr	theRadioGroupInfoPtr,
									rpOneOfSeveral theButtonNumber) {
	void *				theGetPtr;
	rpVariableType		theGetPtrType;
	Signed16Ptr			theIndexValuesPtr;
	char *				theNamePtr;
	rpHttpRequestPtr	theRequestPtr;
	rpOneOfSeveral		theValue;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueClose);
	theGetPtr = theRadioGroupInfoPtr->fGetPtr;
	theGetPtrType = theRadioGroupInfoPtr->fGetPtrType;
	theNamePtr = theRadioGroupInfoPtr->fNamePtr;
	if (theGetPtr != 0) {
		if (theGetPtrType == eRpVarType_Direct) {
			theValue = *(rpOneOfSeveralPtr) (theGetPtr);
		}
		else if (theGetPtrType == eRpVarType_Function) {
			theValue = (*(rpFetchRadioGroupFuncPtr) theGetPtr)();
		}
#if AsSnmpAccess
		else if (theGetPtrType == eRpVarType_Snmp) {
			theValue = *((rpOneOfSeveralPtr) AsRetrieveSnmpItem(theDataPtr,
								theNamePtr, theIndexValuesPtr,
								eAsTextType_Unsigned8, theGetPtr));
		}
#endif
#if AsCustomVariableAccess
		else if (theGetPtrType == eRpVarType_Custom) {
			theValue = *((rpOneOfSeveralPtr) AsRetrieveCustomItem(theDataPtr,
								theNamePtr, theIndexValuesPtr,
								eAsTextType_Unsigned8));
		}
#endif
		else {
			theValue = (*(rpFetchRadioGroupComplexPtr) theGetPtr)(theDataPtr,
								theNamePtr, theIndexValuesPtr);
		}
		if (theValue == theButtonNumber) {
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlChecked);
		}
	}
	return;
}
#endif


static void SendDisplayTextOut(rpDataPtr theDataPtr,
								rpTextDisplayItemPtr theItemPtr) {

	SendTextOut(theDataPtr, theItemPtr->fGetPtrType,
				theItemPtr->fGetPtr, theItemPtr->fTextType,
				theItemPtr->fFieldMaxLength, (char *) kQuestion);
	return;
}

static void SendNamedDisplayTextOut(rpDataPtr theDataPtr,
									rpNamedTextDisplayItemPtr theItemPtr) {
#if RomPagerQueryIndex
	rpConnectionPtr		theConnectionPtr;
	char *				theQueryPtr;
	Signed8				theSavedQueryIndexLevel;

	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;

	/*
		Check for index values stored at the end of the HTML name (APC).
	*/
	theSavedQueryIndexLevel = theConnectionPtr->fIndexDepth;

	theQueryPtr = RpFindTokenDelimitedPtr(theItemPtr->fNamePtr,
											kRpIndexCharacter);

	if (theQueryPtr != (char *) 0) {
		/*
			Terminate the name and point to the query.
		*/
		*theQueryPtr++ = '\0';
		RpStoreQueryValues(theDataPtr, theQueryPtr);
	}
#endif	/* RomPagerQueryIndex */

	SendTextOut(theDataPtr, theItemPtr->fGetPtrType,
				theItemPtr->fGetPtr, theItemPtr->fTextType,
				theItemPtr->fFieldMaxLength, theItemPtr->fNamePtr);

#if RomPagerQueryIndex
	theConnectionPtr->fIndexDepth = theSavedQueryIndexLevel;
#endif	/* RomPagerQueryIndex */

	return;
}


#if RomPagerForms
static void SendItemFormText(rpHttpRequestPtr theRequestPtr,
		rpTextFormItemPtr theItemPtr, rpItemType theItemType) {
#if RpHtmlTextFormDynamic
	Boolean		theDynamicFlag = False;
#endif

	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputType);
	switch (theItemType) {
#if RpHtmlTextFormDynamic
		case eRpItemType_FormTextDyn:
			theDynamicFlag = True;
#endif
		case eRpItemType_FormAsciiText:
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlText);
			break;

#if RpHtmlTextFormDynamic
		case eRpItemType_FormPasswordDyn:
			theDynamicFlag = True;
#endif
		case eRpItemType_FormPasswordText:
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlPassword);
			break;

#if RpHtmlTextFormDynamic
		case eRpItemType_FormHiddenDyn:
			theDynamicFlag = True;
#endif
		case eRpItemType_FormHiddenText:
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlHidden);
			break;

		default:
			break;

	}
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlName);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fNamePtr);
#if RpHtmlTextFormDynamic
	if (theDynamicFlag) {
		SendOutQueryValues(theRequestPtr, False);
	}
	if (theItemType != eRpItemType_FormHiddenText &&
			theItemType != eRpItemType_FormHiddenDyn) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSize);
		RpSendDataOutDecimalUnsigned(theRequestPtr,
				(Unsigned32) theItemPtr->fFieldSize);
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlMaxLength);
		RpSendDataOutDecimalUnsigned(theRequestPtr,
				(Unsigned32) theItemPtr->fFieldMaxLength);
	}
#else
	if (theItemType != eRpItemType_FormHiddenText) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSize);
		RpSendDataOutDecimalUnsigned(theRequestPtr,
				(Unsigned32) theItemPtr->fFieldSize);
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlMaxLength);
		RpSendDataOutDecimalUnsigned(theRequestPtr,
				(Unsigned32) theItemPtr->fFieldMaxLength);
	}
#endif
	if (theItemPtr->fGetPtr != 0) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValue);
/********************************************************************************/
/* Arthur Chow 2005-11-03 : Handle the special input char such as (&quot;)" (&lt;)< (&gt;)> 
		SendTextOut(theRequestPtr->fDataPtr, theItemPtr->fGetPtrType,
					theItemPtr->fGetPtr, theItemPtr->fTextType,
					theItemPtr->fFieldMaxLength, (char *) theItemPtr->fNamePtr);
*/
		SendTextOut_Alpha_InputTextItem(theRequestPtr->fDataPtr, theItemPtr->fGetPtrType,
					theItemPtr->fGetPtr, theItemPtr->fTextType,
					theItemPtr->fFieldMaxLength, (char *) theItemPtr->fNamePtr);
/********************************************************************************/
	}
#if RomPagerJavaScript
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueClose);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fJavaScriptPtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
#else
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueInputClose);
#endif
	return;
}
#endif


char * RpGetBytesPtr(rpDataPtr theDataPtr,
						void * theGetPtr,
						rpVariableType theGetPtrType,
						char *theNamePtr,
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


static void SendTextOut(rpDataPtr theDataPtr, rpVariableType theGetPtrType,
						void *theGetPtr, rpTextType theTextType,
						Unsigned8 theFieldSize, char *theNamePtr) {
	char *				theBytesPtr;
	Unsigned8			theDataLength;
	Signed16Ptr			theIndexValuesPtr;
	rpHttpRequestPtr	theRequestPtr;
	char				theSeparator;
	Signed32			theSigned32;
	Unsigned32			theUnsigned32;
#if AsUse64BitIntegers
	Signed64			theSigned64;
	Unsigned64			theUnsigned64;
#endif

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;

	switch (theTextType) {

		case eRpTextType_ASCII:
		case eRpTextType_ASCII_Extended:
		case eRpTextType_ASCII_Fixed:
		case eRpTextType_DotForm:
		case eRpTextType_Hex:
		case eRpTextType_HexColonForm:
			theBytesPtr = RpGetBytesPtr(theDataPtr, theGetPtr, theGetPtrType,
										theNamePtr, theIndexValuesPtr);

			switch (theTextType) {
				case eRpTextType_ASCII:
					RpSendDataOutZeroTerminated(theRequestPtr, theBytesPtr);
					break;

				case eRpTextType_ASCII_Extended:
					RpSendDataOutExtendedAscii(theRequestPtr, theBytesPtr);
					break;

				case eRpTextType_ASCII_Fixed:
					SendDataOutFixedText(theRequestPtr,
							theFieldSize, theBytesPtr);
					break;

				case eRpTextType_Hex:
					theSeparator = '\0';
					theDataLength = (theFieldSize + 1) / 2;
					SendDataOutHex(theRequestPtr,
							theDataLength, theSeparator, theBytesPtr);
					break;

				case eRpTextType_HexColonForm:
					theSeparator = kAsHexSeparator;
					theDataLength = (theFieldSize + 2) / 3;
					SendDataOutHex(theRequestPtr,
							theDataLength, theSeparator, theBytesPtr);
					break;

				case eRpTextType_DotForm:
					theDataLength = (theFieldSize + 1) / 4;
					SendDataOutDotForm(theRequestPtr,
							theDataLength, theBytesPtr);
					break;

				default:
					break;
			}
			break;

		case eRpTextType_Signed8:
			if (theGetPtrType == eRpVarType_Direct) {
				theSigned32 = (Signed32) *(Signed8Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theSigned32 =
						(Signed32)(*(rpFetchSigned8FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theSigned32 = (Signed32) *((Signed8Ptr) AsRetrieveSnmpItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned32 = (Signed32) *((Signed8Ptr) AsRetrieveCustomItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theSigned32 =
						(Signed32)(*(rpFetchSigned8ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			SendDataOutDecimalSigned(theRequestPtr, theSigned32);
			break;

		case eRpTextType_Signed16:
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
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned32 = (Signed32) *((Signed16Ptr) AsRetrieveCustomItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theSigned32 =
						(Signed32)(*(rpFetchSigned16ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			SendDataOutDecimalSigned(theRequestPtr, theSigned32);
			break;

		case eRpTextType_Signed32:
			if (theGetPtrType == eRpVarType_Direct) {
				theSigned32 = *(Signed32Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theSigned32 =
						(*(rpFetchSigned32FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theSigned32 = *((Signed32Ptr) AsRetrieveSnmpItem(theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned32 = *((Signed32Ptr) AsRetrieveCustomItem(theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theSigned32 =
						(*(rpFetchSigned32ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			SendDataOutDecimalSigned(theRequestPtr, theSigned32);
			break;

#if AsUse64BitIntegers
		case eRpTextType_Signed64:
			if (theGetPtrType == eRpVarType_Direct) {
				theSigned64 = *(Signed64Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theSigned64 =
						(*(rpFetchSigned64FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theSigned64 = *((Signed64Ptr) AsRetrieveSnmpItem(theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned64 = *((Signed64Ptr) AsRetrieveCustomItem(theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theSigned64 =
						(*(rpFetchSigned64ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			SendDataOutDecimalSigned64(theRequestPtr, theSigned64);
			break;

#endif

		case eRpTextType_Unsigned8:
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
													(asTextType) theTextType,
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
													(asTextType) theTextType));
			}
#endif
			else {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned8ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			RpSendDataOutDecimalUnsigned(theRequestPtr, theUnsigned32);
			break;

		case eRpTextType_Unsigned16:
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
													(asTextType) theTextType,
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
													(asTextType) theTextType));
			}
#endif
			else {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned16ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			RpSendDataOutDecimalUnsigned(theRequestPtr, theUnsigned32);
			break;

		case eRpTextType_Unsigned32:
			if (theGetPtrType == eRpVarType_Direct) {
				theUnsigned32 = *(Unsigned32Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned32FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theUnsigned32 = *((Unsigned32Ptr) AsRetrieveSnmpItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theUnsigned32 = *((Unsigned32Ptr) AsRetrieveCustomItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theUnsigned32 =
						(*(rpFetchUnsigned32ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			RpSendDataOutDecimalUnsigned(theRequestPtr, theUnsigned32);
			break;

#if AsUse64BitIntegers
		case eRpTextType_Unsigned64:
			if (theGetPtrType == eRpVarType_Direct) {
				theUnsigned64 = *(Unsigned64Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theUnsigned64 = (Unsigned64)
						(*(rpFetchUnsigned64FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theUnsigned64 = *((Unsigned64Ptr) AsRetrieveSnmpItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theUnsigned64 = *((Unsigned64Ptr) AsRetrieveCustomItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theUnsigned64 = (*(rpFetchUnsigned64ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			SendDataOutDecimalUnsigned64(theRequestPtr, theUnsigned64);
			break;

#endif	/* AsUse64BitIntegers */

		default:
			break;
	}

	return;
}


/* Arthur Chow 2005-11-03 : Handle the special input char such as (&quot;)" (&lt;)< (&gt;)> */
void sWeb_DisplayString_Reword_Form(char* pString, char* pResult)
{
	int i, j, iLen;

	iLen= strlen(pString);
	for( i=j=0; i< iLen; i++)
	{
#if 1
		if(pString[i]=='<')
		{
			pResult[j++]= '&';
			pResult[j++]= 'l';
			pResult[j++]= 't';
			pResult[j++]= ';';
			continue;
		}
		else if(pString[i]=='>')
		{
			pResult[j++]= '&';
			pResult[j++]= 'g';
			pResult[j++]= 't';
			pResult[j++]= ';';
			continue;
		}
		else if(pString[i]=='&')
		{
			pResult[j++]= '&';
			pResult[j++]= 'a';
			pResult[j++]= 'm';
			pResult[j++]= 'p';
			pResult[j++]= ';';
			continue;
		}
		else if(pString[i]=='"')
		{
			pResult[j++]= '&';
			pResult[j++]= 'q';
			pResult[j++]= 'u';
			pResult[j++]= 'o';
			pResult[j++]= 't';
			pResult[j++]= ';';
			continue;
		}
#endif
		pResult[j++]= pString[i];
	}
	pResult[j++]= '\0';
} 


static void SendTextOut_Alpha_InputTextItem(rpDataPtr theDataPtr, rpVariableType theGetPtrType,
						void *theGetPtr, rpTextType theTextType,
						Unsigned8 theFieldSize, char *theNamePtr) {
	char *				theBytesPtr;
	Unsigned8			theDataLength;
	Signed16Ptr			theIndexValuesPtr;
	rpHttpRequestPtr	theRequestPtr;
	char				theSeparator;
	Signed32			theSigned32;
	Unsigned32			theUnsigned32;
#if AsUse64BitIntegers
	Signed64			theSigned64;
	Unsigned64			theUnsigned64;
#endif
	char szTemp[256];
	int dotform=0;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;

	switch (theTextType) {

		case eRpTextType_DotForm:
			dotform=1;
		case eRpTextType_ASCII:
		case eRpTextType_ASCII_Extended:
		case eRpTextType_ASCII_Fixed:
		case eRpTextType_Hex:
		case eRpTextType_HexColonForm:
			theBytesPtr = RpGetBytesPtr(theDataPtr, theGetPtr, theGetPtrType,
										theNamePtr, theIndexValuesPtr);
			if (!dotform)
			{
				sWeb_DisplayString_Reword_Form(theBytesPtr, (char *)&szTemp);
				theBytesPtr=(char *)&szTemp;
			}
			switch (theTextType) {
				case eRpTextType_ASCII:
					RpSendDataOutZeroTerminated(theRequestPtr, theBytesPtr);
					break;

				case eRpTextType_ASCII_Extended:
					RpSendDataOutExtendedAscii(theRequestPtr, theBytesPtr);
					break;

				case eRpTextType_ASCII_Fixed:
					SendDataOutFixedText(theRequestPtr,
							theFieldSize, theBytesPtr);
					break;

				case eRpTextType_Hex:
					theSeparator = '\0';
					theDataLength = (theFieldSize + 1) / 2;
					SendDataOutHex(theRequestPtr,
							theDataLength, theSeparator, theBytesPtr);
					break;

				case eRpTextType_HexColonForm:
					theSeparator = kAsHexSeparator;
					theDataLength = (theFieldSize + 2) / 3;
					SendDataOutHex(theRequestPtr,
							theDataLength, theSeparator, theBytesPtr);
					break;

				case eRpTextType_DotForm:
					theDataLength = (theFieldSize + 1) / 4;
					SendDataOutDotForm(theRequestPtr,
							theDataLength, theBytesPtr);
					break;

				default:
					break;
			}
			break;

		case eRpTextType_Signed8:
			if (theGetPtrType == eRpVarType_Direct) {
				theSigned32 = (Signed32) *(Signed8Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theSigned32 =
						(Signed32)(*(rpFetchSigned8FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theSigned32 = (Signed32) *((Signed8Ptr) AsRetrieveSnmpItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned32 = (Signed32) *((Signed8Ptr) AsRetrieveCustomItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theSigned32 =
						(Signed32)(*(rpFetchSigned8ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			SendDataOutDecimalSigned(theRequestPtr, theSigned32);
			break;

		case eRpTextType_Signed16:
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
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned32 = (Signed32) *((Signed16Ptr) AsRetrieveCustomItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theSigned32 =
						(Signed32)(*(rpFetchSigned16ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			SendDataOutDecimalSigned(theRequestPtr, theSigned32);
			break;

		case eRpTextType_Signed32:
			if (theGetPtrType == eRpVarType_Direct) {
				theSigned32 = *(Signed32Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theSigned32 =
						(*(rpFetchSigned32FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theSigned32 = *((Signed32Ptr) AsRetrieveSnmpItem(theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned32 = *((Signed32Ptr) AsRetrieveCustomItem(theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theSigned32 =
						(*(rpFetchSigned32ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			SendDataOutDecimalSigned(theRequestPtr, theSigned32);
			break;

#if AsUse64BitIntegers
		case eRpTextType_Signed64:
			if (theGetPtrType == eRpVarType_Direct) {
				theSigned64 = *(Signed64Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theSigned64 =
						(*(rpFetchSigned64FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theSigned64 = *((Signed64Ptr) AsRetrieveSnmpItem(theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theSigned64 = *((Signed64Ptr) AsRetrieveCustomItem(theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theSigned64 =
						(*(rpFetchSigned64ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			SendDataOutDecimalSigned64(theRequestPtr, theSigned64);
			break;

#endif

		case eRpTextType_Unsigned8:
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
													(asTextType) theTextType,
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
													(asTextType) theTextType));
			}
#endif
			else {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned8ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			RpSendDataOutDecimalUnsigned(theRequestPtr, theUnsigned32);
			break;

		case eRpTextType_Unsigned16:
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
													(asTextType) theTextType,
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
													(asTextType) theTextType));
			}
#endif
			else {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned16ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			RpSendDataOutDecimalUnsigned(theRequestPtr, theUnsigned32);
			break;

		case eRpTextType_Unsigned32:
			if (theGetPtrType == eRpVarType_Direct) {
				theUnsigned32 = *(Unsigned32Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theUnsigned32 = (Unsigned32)
						(*(rpFetchUnsigned32FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theUnsigned32 = *((Unsigned32Ptr) AsRetrieveSnmpItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theUnsigned32 = *((Unsigned32Ptr) AsRetrieveCustomItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theUnsigned32 =
						(*(rpFetchUnsigned32ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			RpSendDataOutDecimalUnsigned(theRequestPtr, theUnsigned32);
			break;

#if AsUse64BitIntegers
		case eRpTextType_Unsigned64:
			if (theGetPtrType == eRpVarType_Direct) {
				theUnsigned64 = *(Unsigned64Ptr) theGetPtr;
			}
			else if (theGetPtrType == eRpVarType_Function) {
				theUnsigned64 = (Unsigned64)
						(*(rpFetchUnsigned64FuncPtr) theGetPtr)();
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theUnsigned64 = *((Unsigned64Ptr) AsRetrieveSnmpItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType,
													theGetPtr));
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theUnsigned64 = *((Unsigned64Ptr) AsRetrieveCustomItem(
													theDataPtr,
													theNamePtr,
													theIndexValuesPtr,
													(asTextType) theTextType));
			}
#endif
			else {
				theUnsigned64 = (*(rpFetchUnsigned64ComplexPtr) theGetPtr)
						(theDataPtr, theNamePtr, theIndexValuesPtr);
			}
			SendDataOutDecimalUnsigned64(theRequestPtr, theUnsigned64);
			break;

#endif	/* AsUse64BitIntegers */

		default:
			break;
	}

	return;
}


#if RomPagerForms
static void SendItemButton(rpHttpRequestPtr theRequestPtr,
		rpButtonFormItemPtr theItemPtr, Boolean theSubmitFlag) {

	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputType);
	if (theSubmitFlag) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSubmit);
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlName);
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSubmitLower);
	} else {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlReset);
	}
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValue);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fLabelText);
#if RomPagerJavaScript
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueClose);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fJavaScriptPtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
#else
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueInputClose);
#endif
	return;
}
#endif

#if RpHtmlSelectFixedSingle
static void SendItemFixedSingleSelect(rpDataPtr theDataPtr,
		rpFixedSingleSelectFormItemPtr theItemPtr, Boolean theDynamicFlag) {
	void *					theGetPtr;
	rpVariableType			theGetPtrType;
	Signed16Ptr				theIndexValuesPtr;
	char *					theNamePtr;
	rpOption_OneSelectPtr	theOption_OneSelectPtr;
	rpOneOfSeveral			theOptionValue;
	rpHttpRequestPtr		theRequestPtr;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;

	SendSelectStart(theRequestPtr, theItemPtr->fNamePtr,
#if RomPagerJavaScript
			theItemPtr->fJavaScriptPtr,
#endif
			theItemPtr->fListLength, False, theDynamicFlag);
	theGetPtr = theItemPtr->fGetPtr;
	theGetPtrType = theItemPtr->fGetPtrType;
	theNamePtr = theItemPtr->fNamePtr;
	theOption_OneSelectPtr = theItemPtr->fFirstOptionPtr;
	if (theGetPtrType == eRpVarType_Function) {
		theOptionValue = (*(rpFetchRadioGroupFuncPtr) theGetPtr)();
	}
#if AsSnmpAccess
	else if (theGetPtrType == eRpVarType_Snmp) {
		theOptionValue = *((rpOneOfSeveral *) AsRetrieveSnmpItem(theDataPtr,
														theNamePtr, 
														theIndexValuesPtr,
														eAsTextType_Unsigned8, 
														theGetPtr));
	}
#endif
#if AsCustomVariableAccess
	else if (theGetPtrType == eRpVarType_Custom) {
		theOptionValue = *((rpOneOfSeveral *) AsRetrieveCustomItem(theDataPtr,
														theNamePtr, 
														theIndexValuesPtr,
														eAsTextType_Unsigned8));
	}
#endif
	else if (theGetPtrType == eRpVarType_Complex) {
		theOptionValue = (*(rpFetchRadioGroupComplexPtr) theGetPtr)
							(theDataPtr, theNamePtr, theIndexValuesPtr);
	}
	else {
		theOptionValue = *(rpOneOfSeveralPtr) theGetPtr;
	}
	while (theOption_OneSelectPtr != (rpOption_OneSelectPtr) 0) {
		/*
			Send out the <OPTION> tag and value.
		*/
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlOptionValue);
		RpSendDataOutDecimalUnsigned(theRequestPtr,
				(Unsigned32) theOption_OneSelectPtr->fOptionNumber);
		if (theOptionValue == theOption_OneSelectPtr->fOptionNumber) {
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSelected);
		}
		RpSendDataOutZeroTerminated(theRequestPtr, kCloseAngle);
		/*
			Send out the <OPTION> string.
		*/
		RpSendDataOutZeroTerminated(theRequestPtr,
				theOption_OneSelectPtr->fValuePtr);
		RpSendDataOutZeroTerminated(theRequestPtr, "\n");
		theOption_OneSelectPtr = theOption_OneSelectPtr->fNextPtr;
	}
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSelectEnd);

	return;
}
#endif


#if RpHtmlSelectVariable
static Boolean SendItemVariableSelect(rpDataPtr theDataPtr,
					rpVariableSelectFormItemPtr theItemPtr,
					Boolean theMultipleFlag, Boolean theDynamicFlag) {
	rpFetchOptionFuncPtr	theGetFuncPtr = (rpFetchOptionFuncPtr) 0;
	rpFetchOptionComplexPtr	theGetComplexPtr = (rpFetchOptionComplexPtr) 0;
	Signed16Ptr				theIndexValuesPtr;
	Unsigned8				theItemNumber;
	Boolean					theLoopFlag;
	rpHttpRequestPtr		theRequestPtr;
	Boolean					theSelectedFlag;
	void *					theValuePtr;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
	theItemNumber = theRequestPtr->fItemNumber;

	/*
		On the first item, send out the <SELECT> tag
	*/
	if (theItemNumber == 0) {
		SendSelectStart(theRequestPtr,
				theItemPtr->fNamePtr,
#if RomPagerJavaScript
				theItemPtr->fJavaScriptPtr,
#endif
				theItemPtr->fListLength,
				theMultipleFlag, theDynamicFlag);
	}

	theLoopFlag = True;
	while (theLoopFlag) {
		/*
			Get the value for the <OPTION> item
		*/
		if (theItemPtr->fGetPtrType == eRpVarType_Function) {
			theGetFuncPtr = (rpFetchOptionFuncPtr) theItemPtr->fGetPtr;
			theValuePtr = (*theGetFuncPtr)(theItemNumber, &theSelectedFlag);
		}
		else {
			theGetComplexPtr = (rpFetchOptionComplexPtr) theItemPtr->fGetPtr;
			theValuePtr = (*theGetComplexPtr)(theRequestPtr->fDataPtr,
							theItemPtr->fNamePtr, theIndexValuesPtr,
							theItemNumber, &theSelectedFlag);
		}
		if (theValuePtr == (void *) 0) {
			/*
				We don't have any more items, so exit.
			*/
			theLoopFlag = False;
		}
		else {
			/*
				Send out the <OPTION> tag
			*/
			if (theSelectedFlag) {
				RpSendDataOutZeroTerminated(theRequestPtr, kHtmlOptionSelected);
			} else {
				RpSendDataOutZeroTerminated(theRequestPtr, kHtmlOption);
			}
			/*
				Send out the <OPTION> string
			*/
			SendTextOut(theRequestPtr->fDataPtr, eRpVarType_Direct,
						theValuePtr, theItemPtr->fTextType,
						theItemPtr->fFieldMaxLength, theItemPtr->fNamePtr);
			RpSendDataOutZeroTerminated(theRequestPtr, "\n");

			/*
				Bump for the next item.
			*/
			theItemNumber += 1;

			/*
				If we've filled a buffer exit the loop.  We'll resume
				the <SELECT> list after we send the buffer.
			*/
			if (theRequestPtr->fHtmlBufferReady) {
				theLoopFlag = False;
			}
		}
	}

	/*
		If we are out of items, send the </SELECT> tag
	*/
	if (theValuePtr == (void *) 0) {
		theItemNumber = 0;
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSelectEnd);
	}

	/*
		Save away the next item number, or clear it if there
		aren't any more.
	*/
	theRequestPtr->fItemNumber = theItemNumber;

	/*
		Indicate whether we need to come back or not
	*/
	return (Boolean) (theItemNumber == 0);
}
#endif


#if RpHtmlSelectVarValue
static Boolean SendItemVarValueSelect(rpDataPtr theDataPtr,
		rpVariableSelectFormItemPtr theItemPtr, Boolean theMultipleFlag,
		Boolean theDynamicFlag) {
	rpFetchOptionValueFuncPtr		theGetFuncPtr;
	rpFetchOptionValueComplexPtr	theGetComplexPtr;
	Signed16Ptr						theIndexValuesPtr;
	Unsigned8						theItemNumber;
	Boolean							theLoopFlag;
	void *							theOptionPtr;
	rpHttpRequestPtr				theRequestPtr;
	Boolean							theSelectedFlag;
	Unsigned32						theValue;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
	theGetFuncPtr = (rpFetchOptionValueFuncPtr) 0;
	theGetComplexPtr = (rpFetchOptionValueComplexPtr) 0;
	theItemNumber = theRequestPtr->fItemNumber;

	/*
		On the first item, send out the <SELECT> tag
	*/
	if (theItemNumber == 0) {
		SendSelectStart(theRequestPtr,
				theItemPtr->fNamePtr,
#if RomPagerJavaScript
				theItemPtr->fJavaScriptPtr,
#endif
				theItemPtr->fListLength, theMultipleFlag, theDynamicFlag);
	}
	theLoopFlag = True;

	while (theLoopFlag) {
		/*
			Get the value for the <OPTION> item
		*/
		if (theItemPtr->fGetPtrType == eRpVarType_Function) {
			theGetFuncPtr = (rpFetchOptionValueFuncPtr) theItemPtr->fGetPtr;
			theOptionPtr = (*theGetFuncPtr)
					(theItemNumber, &theSelectedFlag, &theValue);
		}
		else {
			theGetComplexPtr = (rpFetchOptionValueComplexPtr)
					theItemPtr->fGetPtr;
			theOptionPtr = (*theGetComplexPtr) (theRequestPtr->fDataPtr,
							theItemPtr->fNamePtr, theIndexValuesPtr,
							theItemNumber, &theSelectedFlag, &theValue);
		}
		if (theOptionPtr == (void *) 0) {
			/*
				We don't have any more items, so exit.
			*/
			theLoopFlag = False;
		}
		else {
			/*
				Send out the <OPTION> tag and value
			*/
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlOptionValue);
			SendDataOutHex(theRequestPtr, 4, '\0', (char *) &theValue);
			if (theSelectedFlag) {
				RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSelected);
			}
			RpSendDataOutZeroTerminated(theRequestPtr, kCloseAngle);

			/*
				Send out the <OPTION> string
			*/
			SendTextOut(theRequestPtr->fDataPtr, eRpVarType_Direct,
						theOptionPtr, theItemPtr->fTextType,
						theItemPtr->fFieldMaxLength, theItemPtr->fNamePtr);
			RpSendDataOutZeroTerminated(theRequestPtr, "\n");

			/*
				Bump for the next item.
			*/
			theItemNumber += 1;

			/*
				If we've filled a buffer exit the loop.  We'll resume
				the <SELECT> list after we send the buffer.
			*/
			if (theRequestPtr->fHtmlBufferReady) {
				theLoopFlag = False;
			}
		}
	}

	/*
		If we are out of items, send the </SELECT> tag
	*/
	if (theOptionPtr == (void *) 0) {
		theItemNumber = 0;
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSelectEnd);
	}
	/*
		Save away the next item number, or clear it if there
		aren't any more.
	*/

	theRequestPtr->fItemNumber = theItemNumber;

	/*
		Indicate whether we need to come back or not
	*/
	return (Boolean) (theItemNumber == 0);
}
#endif


#if RpHtmlSelectFixedMulti
static void SendItemFixedMultiSelect(rpDataPtr theDataPtr, char *theNamePtr,
		rpOption_MultiSelectPtr theFirstOptionPtr, Unsigned8 theListLength,
#if RomPagerJavaScript
		char *theJavaScriptPtr,
#endif
	Boolean theDynamicFlag) {
	void *					theGetPtr;
	rpVariableType			theGetPtrType;
	Signed16Ptr				theIndexValuesPtr;
	rpOption_MultiSelectPtr	theOption_MultiSelectPtr;
	rpHttpRequestPtr		theRequestPtr;
	Boolean					theSelectedFlag;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
	SendSelectStart(theRequestPtr, theNamePtr,
#if RomPagerJavaScript
			theJavaScriptPtr,
#endif
			theListLength, True, theDynamicFlag);
	theOption_MultiSelectPtr = theFirstOptionPtr;
	while (theOption_MultiSelectPtr != (rpOption_MultiSelectPtr) 0) {
		theGetPtr = theOption_MultiSelectPtr->fGetPtr;
		theGetPtrType = theOption_MultiSelectPtr->fGetPtrType;
		if (theGetPtrType == eRpVarType_Function) {
			theSelectedFlag = (*(rpFetchBooleanFuncPtr) theGetPtr)();
		}
		else if (theGetPtrType == eRpVarType_Complex) {
			theSelectedFlag = (*(rpFetchBooleanComplexPtr) theGetPtr)
					(theDataPtr, theNamePtr, theIndexValuesPtr);
		}
#if AsSnmpAccess
		else if (theGetPtrType == eRpVarType_Snmp) {
			theSelectedFlag = *((Boolean *) AsRetrieveSnmpItem(theDataPtr,
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Unsigned8,
									theGetPtr));
		}
#endif
#if AsCustomVariableAccess
		else if (theGetPtrType == eRpVarType_Custom) {
			theSelectedFlag = *((Boolean *) AsRetrieveCustomItem(theDataPtr,
									theNamePtr,
									theIndexValuesPtr,
									eAsTextType_Unsigned8));
		}
#endif
		else {
			theSelectedFlag = *(Boolean *) theGetPtr;
		}
		if (theSelectedFlag) {
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlOptionSelected);
		} else {
			RpSendDataOutZeroTerminated(theRequestPtr, kHtmlOption);
		}
		RpSendDataOutZeroTerminated(theRequestPtr,
				theOption_MultiSelectPtr->fValuePtr);
		RpSendDataOutZeroTerminated(theRequestPtr, "\n");
		theOption_MultiSelectPtr = theOption_MultiSelectPtr->fNextPtr;
	}
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSelectEnd);

	return;
}
#endif

#if RpHtmlSelectVariable || RpHtmlSelectVarValue || \
	RpHtmlSelectFixedSingle || RpHtmlSelectFixedMulti
static void SendSelectStart(rpHttpRequestPtr theRequestPtr,
							char *theNamePtr,
#if RomPagerJavaScript
							char *theJavaScriptPtr,
#endif
							Unsigned8 theListLength,
							Boolean theMultipleFlag,
							Boolean theDynamicFlag) {

	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSelect);
	if (theMultipleFlag) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlMultiple);
	}
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlName);
	RpSendDataOutZeroTerminated(theRequestPtr, theNamePtr);
	if (theDynamicFlag) {
		SendOutQueryValues(theRequestPtr, False);
	}
	if (theListLength > 0) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSize);
		RpSendDataOutDecimalUnsigned(theRequestPtr,
				(Unsigned32)theListLength);
	}
#if RomPagerJavaScript
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueClose);
	RpSendDataOutZeroTerminated(theRequestPtr, theJavaScriptPtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
#else
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueInputClose);
#endif
	return;
}
#endif

#if RpHtmlTextArea
static void SendItemTextArea(rpDataPtr theDataPtr,
		rpTextAreaFormItemPtr theItemPtr) {
	void *				theGetPtr;
	rpVariableType		theGetPtrType;
	Signed16Ptr			theIndexValuesPtr;
	Unsigned16			theItemNumber;
	char *				theNamePtr;
	rpHttpRequestPtr	theRequestPtr;
	char *				theValuePtr;

	theRequestPtr = theDataPtr->fCurrentHttpRequestPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
	SendItemTextAreaHeaders(theRequestPtr, theItemPtr);
	theGetPtr = theItemPtr->fGetPtr;
	theGetPtrType = theItemPtr->fGetPtrType;
	theNamePtr = theItemPtr->fNamePtr;

	if (theGetPtr != 0 && theGetPtrType != eRpVarType_Direct) {
		theItemNumber = 0;
		theValuePtr = kHtmlTextArea;
		while (theValuePtr != (char *) 0) {
			if (theGetPtrType == eRpVarType_Function) {
				theValuePtr = (*(rpFetchTextFuncPtr) theGetPtr)(theItemNumber);
			}
#if AsSnmpAccess
			else if (theGetPtrType == eRpVarType_Snmp) {
				theValuePtr = (char *) AsRetrieveSnmpItem(theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										eAsTextType_ASCII,
										theGetPtr);
			}
#endif
#if AsCustomVariableAccess
			else if (theGetPtrType == eRpVarType_Custom) {
				theValuePtr = (char *) AsRetrieveCustomItem(theDataPtr,
										theNamePtr,
										theIndexValuesPtr,
										eAsTextType_ASCII);
			}
#endif
			else {
				theValuePtr = (*(rpFetchTextComplexPtr) theGetPtr)
						(theDataPtr,
						theNamePtr,
						theIndexValuesPtr,
						theItemNumber);
			}
			if (theValuePtr != (char *) 0) {
				RpSendDataOutZeroTerminated(theRequestPtr, theValuePtr);
				RpSendDataOutZeroTerminated(theRequestPtr, "\n");
				theItemNumber += 1;
			}
		}
	}
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlTextAreaEnd);
	return;
}
#endif

#if RpHtmlTextAreaBuf
static void SendItemTextAreaBuf(rpHttpRequestPtr theRequestPtr,
		rpTextAreaFormItemPtr theItemPtr) {
	rpDataPtr			theDataPtr;
	void *				theGetPtr;
	rpVariableType		theGetPtrType;
	Signed16Ptr			theIndexValuesPtr;
	char *				theValuePtr;

	theDataPtr = theRequestPtr->fDataPtr;
	theIndexValuesPtr = theDataPtr->fCurrentConnectionPtr->fIndexValues;
	theGetPtr = theItemPtr->fGetPtr;
	theGetPtrType = theItemPtr->fGetPtrType;
	SendItemTextAreaHeaders(theRequestPtr, theItemPtr);
	if (theGetPtr != 0 && theGetPtrType != eRpVarType_Direct) {
		theValuePtr = RpGetBytesPtr(theDataPtr,
									theGetPtr,
									theGetPtrType,
									theItemPtr->fNamePtr,
									theIndexValuesPtr);

		if (theValuePtr != (char *) 0) {
			RpSendDataOutZeroTerminated(theRequestPtr, theValuePtr);
		}
	}
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlTextAreaEnd);
	return;
}
#endif

#if RpHtmlTextArea || RpHtmlTextAreaBuf
static void SendItemTextAreaHeaders(rpHttpRequestPtr theRequestPtr,
		rpTextAreaFormItemPtr theItemPtr) {

	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlTextArea);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlName);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fNamePtr);

	if (theItemPtr->fRows > 0) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlRows);
		RpSendDataOutDecimalUnsigned(theRequestPtr,
				(Unsigned32)theItemPtr->fRows);
	}
	if (theItemPtr->fColumns > 0) {
		RpSendDataOutZeroTerminated(theRequestPtr, kHtmlColumns);
		RpSendDataOutDecimalUnsigned(theRequestPtr,
				(Unsigned32)theItemPtr->fColumns);
	}
#if RomPagerJavaScript
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueClose);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fJavaScriptPtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
#else
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueInputClose);
#endif
	return;
}
#endif

#if RpHtmlFormImage
static void SendItemImage(rpHttpRequestPtr theRequestPtr,
		rpImageFormItemPtr theItemPtr) {

	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputType);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlImage);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlName);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fNamePtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueClose);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fImagePtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
	return;
}
#endif

#if RomPagerSecure
static void SendOutHostName(rpHttpRequestPtr theRequestPtr,
		char *theTextPtr, rpConnectionPtr theConnectionPtr) {

	RpSendDataOutZeroTerminated(theRequestPtr, theTextPtr);
	SendDataOutDotForm(theRequestPtr, 4, (char *) &theConnectionPtr->fIpLocal);
	return;
}

static void SendOutFullUrl(rpHttpRequestPtr theRequestPtr,
		rpFullUrlItemPtr theItemPtr, rpConnectionPtr theConnectionPtr) {
	char *theSchemePtr;

	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlAnchorStart);
	switch (theItemPtr->fUrlScheme) {
		case eRpUrlScheme_Http:
			theSchemePtr = kHttpString;
			break;

		case eRpUrlScheme_Https:
			theSchemePtr = kHttpsString;
			break;
	}
	SendOutHostName(theRequestPtr, theSchemePtr, theConnectionPtr);
	if (theItemPtr->fPort) {
		RpSendDataOutZeroTerminated(theRequestPtr, kColon);
		RpSendDataOutDecimalUnsigned(theRequestPtr,
				(Unsigned32) theItemPtr->fPort);
	}
#if RomPagerUrlState
	if (theRequestPtr->fUrlState[0] != '\0') {
		RpSendDataOutZeroTerminated(theRequestPtr, kUrlStatePrefix);
		RpSendDataOutZeroTerminated(theRequestPtr, theRequestPtr->fUrlState);
	}
#endif
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fPagePtr->fURL);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlAnchorMiddle);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fLinkPtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlAnchorEnd);

	return;
}
#endif

#if RomPagerFileUpload
static void SendItemFormFile(rpHttpRequestPtr theRequestPtr,
		rpFileFormItemPtr theItemPtr) {

	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputType);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlFile);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlName);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fNamePtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlSize);
	RpSendDataOutDecimalUnsigned(theRequestPtr,
			(Unsigned32) theItemPtr->fFieldSize);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlMaxLength);
	RpSendDataOutDecimalUnsigned(theRequestPtr,
			(Unsigned32) theItemPtr->fFieldMaxLength);
#if RomPagerJavaScript
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueClose);
	RpSendDataOutZeroTerminated(theRequestPtr, theItemPtr->fJavaScriptPtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlInputClose);
#else
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlValueInputClose);
#endif
	return;
}
#endif


#if RpFileInsertItem
static void SendItemFile(rpHttpRequestPtr theRequestPtr, char *theFileNamePtr) {

	theRequestPtr->fHttpTransactionState = eRpOpenInsertFileItem;
	theRequestPtr->fFileNamePtr = theFileNamePtr;
	if (theRequestPtr->fFillBufferAvailable == 0) {
		RpFlipResponseBuffers(theRequestPtr);
	}
	return;
}
#endif


#if RomPagerQueryIndex

static void SendOutIndexDisplay(rpHttpRequestPtr theRequestPtr,
								char *theTextPtr, Unsigned8 theOffset) {
	rpConnectionPtr		theConnectionPtr;

	/*
		Send out the value of the index in 1-relative format.
	*/
	RpSendDataOutZeroTerminated(theRequestPtr, theTextPtr);
	theConnectionPtr = theRequestPtr->fDataPtr->fCurrentConnectionPtr;
	RpSendDataOutDecimalUnsigned(theRequestPtr,
			(Unsigned32) theConnectionPtr->fIndexValues
			[theConnectionPtr->fIndexDepth - theOffset] + 1);
	return;
}


static void SendOutQueryDisplay(rpHttpRequestPtr theRequestPtr,
									char *theTextPtr) {

	RpSendDataOutZeroTerminated(theRequestPtr, theTextPtr);
	SendOutQueryValues(theRequestPtr, True);
	return;
}


static void SendOutQueryValues(rpHttpRequestPtr theRequestPtr,
		Boolean theForUrlFlag) {
	char	theQueryString[4 * kAsIndexQueryDepth + 1];

	/*
		Send out the values of the indices in 1-relative format as a query
		"?,i,j,k" so that we can pass them from page to page.
	*/

	theQueryString[0] = '\0';
	RpBuildQueryValues(theRequestPtr, theQueryString, theForUrlFlag);
	RpSendDataOutZeroTerminated(theRequestPtr, theQueryString);
	return;
}


static rpItemPtr TestRepeatGroupWhile(rpDataPtr theDataPtr,
					rpHttpRequestPtr theRequestPtr,
					rpNestingPtr theNestingPtr) {
	rpConnectionPtr		theConnectionPtr;
	rpItemPtr			theItemPtr;

	theConnectionPtr = theDataPtr->fCurrentConnectionPtr;

	/*
		Test the Repeat While function, and see whether
		to go through the loop or exit.
	*/
	(*theNestingPtr->fRepeatWhileFunctionPtr)
			(theDataPtr, &theNestingPtr->fIndexIncrement,
			&theNestingPtr->fRepeatWhileValue);
	if (theNestingPtr->fRepeatWhileValue == (void *) 0) {
		/*
			We are done, so just pop the state
		*/
		theItemPtr = theNestingPtr->fReturnItemPtr;
		theNestingPtr->fRepeatWhileFunctionPtr = (rpRepeatWhileFuncPtr) 0;
		theRequestPtr->fNestedDepth -= 1;
		theConnectionPtr->fIndexDepth -= 1;
	}
	else {
		/*
			We need to go through the repeat group, so save the index,
			and set the item pointer to the beginning of the repeat group.
		*/
		theConnectionPtr->fIndexValues[theConnectionPtr->fIndexDepth] =
				theNestingPtr->fIndexIncrement;
		theItemPtr = theNestingPtr->fRepeatItemPtr;
	}
	return theItemPtr;
}

#endif	/* RomPagerQueryIndex */

#if RmAggregateHtml
static void SendOutImageSource(rpHttpRequestPtr theRequestPtr,
				rpObjectDescriptionPtr theImageDescriptionPtr) {
	char		theHexImageString[kMaxSaveHeaderLength * 2];

	RpSendDataOutZeroTerminated(theRequestPtr, C_oIMG_SRC);
	if (theRequestPtr->fAggregate) {
		/*
			for SMTP messages with embedded images we have to generate
			special image source names and matching SMTP headers.
		*/
		RmAddImageToMessage(theRequestPtr, theImageDescriptionPtr);
		RpSendDataOutZeroTerminated(theRequestPtr, kCid);
		RpHexToString((unsigned char *)theImageDescriptionPtr->fURL,
						theHexImageString,
						(Unsigned16)RP_STRLEN(theImageDescriptionPtr->fURL));
		RpSendDataOutZeroTerminated(theRequestPtr, theHexImageString);
	}
	else {
		RpSendDataOutZeroTerminated(theRequestPtr,
				theImageDescriptionPtr->fURL);
	}
	RpSendDataOutZeroTerminated(theRequestPtr, kQuote);
	return;
}
#else
static void SendOutImageSource(rpHttpRequestPtr theRequestPtr,
				rpObjectDescriptionPtr theImageDescriptionPtr) {

	RpSendDataOutZeroTerminated(theRequestPtr, C_oIMG_SRC);
	RpSendDataOutZeroTerminated(theRequestPtr, theImageDescriptionPtr->fURL);
	RpSendDataOutZeroTerminated(theRequestPtr, kQuote);
	return;
}
#endif

static void SendOutHtmlReferer(rpHttpRequestPtr theRequestPtr,
								char *theTextPtr) {

	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlAnchorStart);
	RpSendDataOutZeroTerminated(theRequestPtr, theRequestPtr->fRefererUrl);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlAnchorMiddle);
	RpSendDataOutZeroTerminated(theRequestPtr, theTextPtr);
	RpSendDataOutZeroTerminated(theRequestPtr, kHtmlAnchorEnd);
	return;
}


static void SendDataOutLengthEncoded(rpHttpRequestPtr theRequestPtr,
		Unsigned16 theLength, char *theLengthEncodedPtr) {
	Unsigned16 theCopyLength;
	Unsigned16 theOffset;

	theOffset = 0;
	while (theLength > 0) {
		theCopyLength = theLength;
		if (theCopyLength > theRequestPtr->fFillBufferAvailable) {
			theCopyLength = theRequestPtr->fFillBufferAvailable;
		}
		RP_MEMCPY(theRequestPtr->fHtmlFillPtr,
				theLengthEncodedPtr + theOffset,
				theCopyLength);
		theRequestPtr->fFillBufferAvailable -= theCopyLength;
		if (theRequestPtr->fFillBufferAvailable == 0) {
			RpFlipResponseBuffers(theRequestPtr);
		}
		else {
			theRequestPtr->fHtmlFillPtr += theCopyLength;
		}
		theLength -= theCopyLength;
		theOffset += theCopyLength;
	}
	return;
}


/*
	The RpSendDataOutZeroTerminated routine is the main routine to place
	null-terminated strings in the output buffers. It is used for both
	direct and indirect user data. All the strings that are sent by
	this routine will look at the string for compression dictionary
	expansion.  A more complete description of the phrase dictionary and
	the system dictionary phrase definitions can be found in RpDict.h.
*/

void RpSendDataOutZeroTerminated(rpHttpRequestPtr theRequestPtr,
		const char *theCharPtr) {
	Unsigned32		theIndex;
	const char *	theInputPtr;
	Unsigned8		theCharacter;
	const char *	thePhrasePtr;
#if RomMailer
	rpDataPtr		theDataPtr;
	Unsigned8		theRoomNeeded;

	theDataPtr = theRequestPtr->fDataPtr;
#endif

	theInputPtr = theCharPtr;
	if (theInputPtr != (const char *) 0) {
		theCharacter = *theInputPtr++;
		while (theCharacter != (char) 0) {
#if RomMailer
			if (theDataPtr->fCurrentConnectionPtr->fProtocol ==
					eRpSmtpClient) {
				/*
					Protocol is SMTP so leave room for expansion,
					possibly <LF> and '.'.
				*/
				theRoomNeeded = 2;
			}
			else {
				/*
					Not SMTP so no extra room is needed.
				*/
				theRoomNeeded = 0;
			}
			if (theRequestPtr->fFillBufferAvailable > theRoomNeeded)
#else
			if (theRequestPtr->fFillBufferAvailable > 0)
#endif
			{
				if (theCharacter <= (Unsigned8) '\177') {
					/*
						Send out a normal character.
					*/
#if RomMailer
					if (theDataPtr->fCurrentConnectionPtr->fProtocol ==
							eRpSmtpClient) {
						/*
							For SMTP messages we have to change all line ends
							to <CR><LF> and make sure that any periods that
							begin a line get an extra period to prevent
							confusion with the end-of-data signal characters.
						*/
						switch (theCharacter) {
							case kAscii_Newline:
								if (theRequestPtr->fLastCharacter
										!= kAscii_Return) {
									/*
										there was a missing <CR>,
										so let's add one
									*/
									*theRequestPtr->fHtmlFillPtr++ =
											kAscii_Return;
									theRequestPtr->fFillBufferAvailable -= 1;
								}
								break;

							case kAscii_Dot:
								if (theRequestPtr->fLastCharacter ==
										kAscii_Return) {
									/*
										The last character was a <CR>,
										and this character is a dot,
										so we need to add a <LF> and a dot.
									*/
									*theRequestPtr->fHtmlFillPtr++ =
											kAscii_Newline;
									*theRequestPtr->fHtmlFillPtr++ =
											kAscii_Dot;
									theRequestPtr->fFillBufferAvailable -= 2;
								}
								if (theRequestPtr->fLastCharacter ==
										kAscii_Newline) {
									/*
										The last character was a NewLine,
										and this character is a dot,
										so add a dot so that the mail
										receiver can strip it out.
									*/
									*theRequestPtr->fHtmlFillPtr++ =
											kAscii_Dot;
									theRequestPtr->fFillBufferAvailable -= 1;
								}
								break;

							default:
								if (theRequestPtr->fLastCharacter ==
										kAscii_Return) {
									/*
										The last character was a <CR>,
										and this character is not a <LF>,
										so we need to add a <LF>.
									*/
									*theRequestPtr->fHtmlFillPtr++ =
											kAscii_Newline;
									theRequestPtr->fFillBufferAvailable -= 1;
								}
								break;
						}
						*theRequestPtr->fHtmlFillPtr++ = theCharacter;
						theRequestPtr->fFillBufferAvailable -= 1;
						theRequestPtr->fLastCharacter = theCharacter;
					}
					else {
						*theRequestPtr->fHtmlFillPtr++ = theCharacter;
						theRequestPtr->fFillBufferAvailable -= 1;
					}
#else	/* !RomMailer */
					*theRequestPtr->fHtmlFillPtr++ = theCharacter;
					theRequestPtr->fFillBufferAvailable -= 1;
#endif	/* RomMailer */
				}
				else if (theCharacter < (Unsigned8) kAsCompressionEscape) {
					/*
						Send out a system dictionary phrase
						(note the recursion).
					*/
					thePhrasePtr = (gRpPhrases[theCharacter -
												(Unsigned8) '\200']);
					RpSendDataOutZeroTerminated(theRequestPtr, thePhrasePtr);
				}
				else if (theCharacter == (Unsigned8) kAsCompressionEscape) {
					/*
						Send out the next character with no expansion.
					*/
					*theRequestPtr->fHtmlFillPtr++ = *theInputPtr++;
					theRequestPtr->fFillBufferAvailable -= 1;
				}
				else {
					/*
						The next character is used to index into the
						section of the user dictionary selected by
						the signal character.
					*/
					theIndex = (Unsigned8) *theInputPtr++;

					theIndex += ((Unsigned8) '\377' - theCharacter) *
									k8_BitsValue;
					thePhrasePtr = theRequestPtr->fUserPhrases[theIndex];

					if (theRequestPtr->fUserPhrasesCanBeCompressed) {
						/*
							Send out a user dictionary phrase
							(note the recursion).
						*/
						RpSendDataOutZeroTerminated(theRequestPtr, thePhrasePtr);
					}
					else {
						/*
							Send out a user dictionary phrase with no
							further expansion (useful for international).
						*/
						RpSendDataOutExtendedAscii(theRequestPtr, thePhrasePtr);
					}
				}
				theCharacter = *theInputPtr++;
			}
			else {
				RpFlipResponseBuffers(theRequestPtr);
			}
		}
	}
	return;
}


void RpSendDataOutDecimalUnsigned(rpHttpRequestPtr theRequestPtr,
		Unsigned32 theData) {
	Unsigned16	theCharsWritten;

	if (theRequestPtr->fFillBufferAvailable < 10) {
		RpFlipResponseBuffers(theRequestPtr);
	}
	theCharsWritten = RpConvertUnsigned32ToAscii(theData,
			theRequestPtr->fHtmlFillPtr);
	theRequestPtr->fHtmlFillPtr += theCharsWritten;
	theRequestPtr->fFillBufferAvailable -= theCharsWritten;

	return;
}

static void SendDataOutDecimalSigned(rpHttpRequestPtr theRequestPtr,
		Signed32 theData) {
	Unsigned16	theCharsWritten;

	if (theRequestPtr->fFillBufferAvailable < 11) {
		RpFlipResponseBuffers(theRequestPtr);
	}
	theCharsWritten = RpConvertSigned32ToAscii(theData,
									theRequestPtr->fHtmlFillPtr);
	theRequestPtr->fHtmlFillPtr += theCharsWritten;
	theRequestPtr->fFillBufferAvailable -= theCharsWritten;

	return;
}

#if AsUse64BitIntegers
static void SendDataOutDecimalUnsigned64(rpHttpRequestPtr theRequestPtr,
		Unsigned64 theData) {
	Unsigned16	theCharsWritten;

	if (theRequestPtr->fFillBufferAvailable < 20) {
		RpFlipResponseBuffers(theRequestPtr);
	}
	theCharsWritten = RpConvertUnsigned64ToAscii(theData,
			theRequestPtr->fHtmlFillPtr);
	theRequestPtr->fHtmlFillPtr += theCharsWritten;
	theRequestPtr->fFillBufferAvailable -= theCharsWritten;

	return;
}

static void SendDataOutDecimalSigned64(rpHttpRequestPtr theRequestPtr,
		Signed64 theData) {
	Unsigned16	theCharsWritten;

	if (theRequestPtr->fFillBufferAvailable < 21) {
		RpFlipResponseBuffers(theRequestPtr);
	}
	theCharsWritten = RpConvertSigned64ToAscii(theData,
									theRequestPtr->fHtmlFillPtr);
	theRequestPtr->fHtmlFillPtr += theCharsWritten;
	theRequestPtr->fFillBufferAvailable -= theCharsWritten;

	return;
}
#endif

static void SendDataOutHex(rpHttpRequestPtr theRequestPtr,
							Unsigned8 theLength,
							char theSeparator,
							char *theHexPtr) {
	Unsigned8	theByte;

	while(theLength > 0) {
		if (theRequestPtr->fFillBufferAvailable >= 3) {
			theByte = *theHexPtr++;
			*theRequestPtr->fHtmlFillPtr++ =
					NIBBLE_TO_HEX((theByte >> 4) & 0x0f);
			*theRequestPtr->fHtmlFillPtr++ =
					NIBBLE_TO_HEX(theByte & 0x0f);
			theRequestPtr->fFillBufferAvailable -= 2;
			theLength -= 1;
			if (theSeparator != '\0' && theLength > 0) {
				*theRequestPtr->fHtmlFillPtr++ = theSeparator;
				theRequestPtr->fFillBufferAvailable -= 1;
			}
		}
		else {
			RpFlipResponseBuffers(theRequestPtr);
		}
	}

	return;
}

static void SendDataOutFixedText(rpHttpRequestPtr theRequestPtr,
					Unsigned8 theLength, char *theTextPtr) {

	while (theLength > 0) {
		if (theRequestPtr->fFillBufferAvailable > 0) {
			*theRequestPtr->fHtmlFillPtr++ = *theTextPtr++;
			theRequestPtr->fFillBufferAvailable -= 1;
			theLength -= 1;
		}
		else {
			RpFlipResponseBuffers(theRequestPtr);
		}
	}
	return;
}

static void SendDataOutDotForm(rpHttpRequestPtr theRequestPtr,
								Unsigned8 theLength,
								char *theHexDataPtr) {

	while (theLength > 0) {
		RpSendDataOutDecimalUnsigned(theRequestPtr,
				(Unsigned32) (Unsigned8) *theHexDataPtr++);
		theLength -= 1;
		if (theLength > 0) {
			/*
				NB: the buffer has space for this '.' since
				SendDataOutDecimalUnsigned makes sure there is room
				for a 32-bit value and we pass in an 8-bit value
			*/
			*theRequestPtr->fHtmlFillPtr++ = '.';
			theRequestPtr->fFillBufferAvailable -= 1;
		}
	}

	return;
}


#if RpHtmlSelectFixedMulti
Boolean RpExpandedMatch(rpHttpRequestPtr theRequestPtr, char *theInputPtr,
						char *theMatchPtr) {
	Boolean				theMatchFlag;

	/*
		This routine relys on the fact that the normal output
		buffers are unused during forms processing.
	*/
	theRequestPtr->fHtmlFillPtr = theRequestPtr->fHtmlResponseBufferOne;
	theRequestPtr->fFillBufferAvailable = kHtmlMemorySize;
	RpSendDataOutZeroTerminated(theRequestPtr, theMatchPtr);
	*(theRequestPtr->fHtmlFillPtr) = (char) 0;
	theMatchFlag = (Boolean) (RP_STRCMP(theInputPtr,
			theRequestPtr->fHtmlResponseBufferOne) == 0);
	return theMatchFlag;
}
#endif

#endif	/* RomPagerServer */
