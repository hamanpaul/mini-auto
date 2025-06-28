/*
 *	File:		RpPgFunc.h
 *
 *	Contains:	Function call prototypes for Get and Set functions
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
 * * * * Release 4.04  * * *
 *		11/16/01	bva		change rpFetchBufferComplexPtr for Unsigned32 length
 * * * * Release 4.03  * * *
 *		09/14/01	rhb		add rpConnCloseFuncPtr
 * * * * Release 4.02  * * *
 * * * * Release 4.00  * * *
 *		03/30/01	bva		add RpHttpPutComplete
 *		02/13/01	rhb		rename RomPagerUse64BitIntegers to AsUse64BitIntegers
 *		01/05/01	pjr		add rpFetchBufferComplexPtr typedef
 *		01/17/00	bva		theServerDataPtr -> theTaskDataPtr
 *		01/03/00	bva		move RpSoftPageFunction to RpSoftPg.h
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 *		12/11/98	rhb		add RpSoftPageFunction
 *		12/05/98	bva		fix comment warning
 * * * * Release 2.2 * * * *
 *		09/03/98	bva		reorganize
 * * * * Release 2.1 * * * *
 *		05/20/98	rhb		add rpFetchOptionValueFuncPtr and 
 *								rpFetchOptionValueComplexPtr
 *		05/08/98	bva		add rpSessionCloseFuncPtr
 * * * * Release 2.0 * * * *
 *		11/10/97	rhb		add support for 64 bit integers
 *		06/19/97	bva		add rpRepeatWhileFuncPtr
 * * * * Release 1.6 * * * *
 *		03/05/97	bva		add rpResetCheckboxArrayPtr
 *		01/16/97	bva		rework rpFetchTextFuncPtr, rpFetchTextComplexPtr,
 *								rpFetchOptionComplexPtr for consistency, 
 *							theItemNumber in rpFetchTextComplexPtr becomes Unsigned16
 * * * * Release 1.5 * * * *
 *		11/01/96	bva		created from RpPages.h
 * * * * Release 1.4 * * * *
 *
 *	To Do:
 */

#ifndef	_RPFUNC_
#define	_RPFUNC_

typedef Unsigned8	rpOneOfSeveral, *rpOneOfSeveralPtr;
typedef Unsigned8	rpMapLocation, *rpMapLocationPtr;


/*********************************************************************
	eRpItemType_FormCheckbox,
	eRpItemType_FormCheckboxDyn,
	eRpItemType_FormFixedMultiSelect,
	eRpItemType_FormFixedMultiDyn

		GetType = Function
*/
typedef Boolean	(*rpFetchBooleanFuncPtr)(void);

/*
		GetType = Complex
*/
typedef Boolean	(*rpFetchBooleanComplexPtr)(void *theTaskDataPtr, 
							char *theNamePtr, Signed16Ptr theIndexValuesPtr);
/*
		SetType = Function
*/
typedef void	(*rpStoreBooleanFuncPtr)(Boolean theValue);

/*
		SetType = Complex
*/
typedef void	(*rpStoreBooleanComplexPtr)(void *theTaskDataPtr, 
							Boolean theValue, char *theNamePtr, 
							Signed16Ptr theIndexValuesPtr);
/*
		Dynamic reset function
*/

typedef void (*rpResetCheckboxArrayPtr) (void *theTaskDataPtr, 
					char *theNamePtr, Signed16Ptr theIndexValuesPtr);

/*********************************************************************
	eRpItemType_FormRadioButton,
	eRpItemType_FormRadioButtonDyn,
	eRpItemType_FormRadioGroupDyn,
	eRpItemType_FormFixedSingleSelect
	eRpItemType_FormFixedSingleDyn

		GetType = Function
*/

typedef rpOneOfSeveral	(*rpFetchRadioGroupFuncPtr)(void);
/*
		GetType = Complex
*/
typedef rpOneOfSeveral	(*rpFetchRadioGroupComplexPtr)(void *theTaskDataPtr, 
							char *theNamePtr, Signed16Ptr theIndexValuesPtr);
/*
		SetType = Function
*/
typedef void	(*rpStoreRadioGroupFuncPtr)(rpOneOfSeveral theValue);
/*
		SetType = Complex
*/
typedef void	(*rpStoreRadioGroupComplexPtr)(void *theTaskDataPtr, 
							rpOneOfSeveral theValue, char *theNamePtr, 
							Signed16Ptr theIndexValuesPtr);



#if RpHtmlBufferDisplay
/*********************************************************************
	eRpItemType_BufferDisplay
*/

typedef Boolean (*rpFetchBufferComplexPtr)(void *theTaskDataPtr,
					char *theNamePtr, Signed16Ptr theIndexValuesPtr,
					char **theBufferPtr, Unsigned32Ptr theLengthPtr);
#endif


/*********************************************************************
	eRpItemType_FormTextArea

		GetType = Function
*/
	 
typedef char *	(*rpFetchTextFuncPtr)(Unsigned16 theItemNumber);
/*
		GetType = Complex
*/
typedef char *	(*rpFetchTextComplexPtr)(void *theTaskDataPtr,
							char *theNamePtr, Signed16Ptr theIndexValuesPtr,
							Unsigned16 theItemNumber);

/*********************************************************************
	eRpItemType_FormVariableSingleSelect,
	eRpItemType_FormVariableMultiSelect,
	eRpItemType_FormVariableSingleDyn,
	eRpItemType_FormVariableMultiDyn

		GetType = Function
*/
typedef void *	(*rpFetchOptionFuncPtr)(Unsigned8 theItemNumber, 
					Boolean *theOptionSelectedFlag);
/*
		GetType = Complex
*/
typedef void *	(*rpFetchOptionComplexPtr)(void *theTaskDataPtr, 
					char *theNamePtr, Signed16Ptr theIndexValuesPtr,
					Unsigned8 theItemNumber, 
					Boolean *theOptionSelectedFlag);
					

/*********************************************************************
	eRpItemType_FormVarValueSingleSelect,
	eRpItemType_FormVarValueMultiSelect,
	eRpItemType_FormVarValueSingleDyn,
	eRpItemType_FormVarValueMultiDyn

		GetType = Function
*/
typedef void *	(*rpFetchOptionValueFuncPtr)(Unsigned8 theItemNumber, 
					Boolean *theOptionSelectedFlag, 
					Unsigned32Ptr theValuePtr);
/*
		GetType = Complex
*/
typedef void *	(*rpFetchOptionValueComplexPtr)(void *theTaskDataPtr, 
					char *theNamePtr, Signed16Ptr theIndexValuesPtr,
					Unsigned8 theItemNumber, 
					Boolean *theOptionSelectedFlag,
					Unsigned32Ptr theValuePtr);

/*********************************************************************
	eRpItemType_FormVariableMultiSelect,
	eRpItemType_FormVariableMultiDyn
	eRpItemType_FormVarValueMultiSelect,
	eRpItemType_FormVarValueMultiDyn

		Reset Function
*/
typedef void (*rpResetVarSelectPtr)(void *theTaskDataPtr);



/*********************************************************************
	Server-Side Image Map Processing
*/
typedef void	(*rpStoreLocationPtr)(rpMapLocation theValue);
typedef void	(*rpStoreLocationComplexPtr)(void *theTaskDataPtr, 
					rpMapLocation theValue, char *theNamePtr, 
					Signed16Ptr theIndexValuesPtr);


/*********************************************************************
	eRpItemType_DynamicDisplay

		GetType = Function
*/
typedef Unsigned8	(*rpFetchIndexFuncPtr)(void);
/*
		GetType = Complex
*/
typedef Unsigned8	(*rpFetchIndexComplexPtr)(void *theTaskDataPtr, 
							Signed16Ptr theIndexValuesPtr);


/*********************************************************************
	eRpItemType_RepeatGroupDynamic
*/
typedef void (*rpDynamicRepeatFuncPtr)
					(void *theTaskDataPtr, 
					Signed16Ptr theStart, 
					Signed16Ptr theLimit, 
					Signed16Ptr theIncrement);


/*********************************************************************
	eRpItemType_RepeatGroupWhile
*/
typedef void 	(*rpRepeatWhileFuncPtr)(void *theTaskDataPtr, 
							Signed16Ptr theIndexPtr,
							void ** theRepeatGroupValuePtr);


/*********************************************************************
	Page object pre-processing,
	Form object post-processing
*/
typedef void (*rpProcessDataFuncPtr)(void *theTaskDataPtr, 
		Signed16Ptr theIndexValuesPtr);


/*********************************************************************
	Request Close processing
*/
typedef void (*rpProcessCloseFuncPtr)(void *theTaskDataPtr);


/*********************************************************************
	Connection Close processing
*/
typedef void (*rpConnCloseFuncPtr)(void *theTaskDataPtr, 
							void *theConnectionCookie);


/*********************************************************************
	External Password processing
*/
typedef void (*rpSessionCloseFuncPtr)
		(void *theTaskDataPtr, void *theUserCookie);


/*********************************************************************
	Server Push processing
*/
#if RomPagerServerPush
extern void RpUserServerPushExit(void *theTaskDataPtr);
#endif


/*********************************************************************
	The Put Complete Interface call
*/
#if RomPagerPutMethod
extern void RpHttpPutComplete(void *theTaskDataPtr, char *thePathPtr);
#endif


#endif
