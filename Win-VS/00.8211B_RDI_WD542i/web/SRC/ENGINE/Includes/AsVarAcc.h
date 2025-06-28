/*
 *	File:		AsVarAcc.h
 *
 *	Contains:	Variable Access Definitions
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
 * * * * Release 4.30  * * *
 *		08/08/03	nam		support floating point for Variable Access
 *		07/08/03	bva		rework AsVariableAccess ifdefs
 * * * * Release 4.21  * * *
 *		02/03/02	rhb		support enums for access through Variable Access
 * * * * Release 4.20  * * *
 *		01/03/03	rhb		get Size and MaxLength right
 *		11/09/02	rhb		move AsStrToxxx from AsEngine.h
 *		08/09/02	rhb		move Phase Dictionary declarations from RomPager.h
 *		08/08/02	rhb		support enums
 * * * * Release 4.12  * * *
 * * * * Release 4.10  * * *
 *		06/13/02	rhb		move AsExpandString prototype from AsEngine.h
 * * * * Release 4.07  * * *
 * * * * Release 4.00  * * *
 *		02/20/01	bva		add variable index prototypes 
 *		02/16/01	rhb		move rpVariableType and asSnmpAccessItem (was 
 *								rpSnmpAccessItem) here from RpPages.h 
 *		02/13/01	rhb		rename RomPagerUse64BitIntegers to AsUse64BitIntegers
 *		02/12/01	rhb		moved from AsEngine.h to make public
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */


#ifndef	_ASVARACC_
#define	_ASVARACC_

#if AsVariableAccess || RomPagerServer

/*
	The variable access item is the general purpose item that is used as a
	bridge between the various Allegro products (RomPager, RomCli, RomPlug) 
	and the user variables. The rpVariableType fields indicate the type of
	access method to use and the asTextType field indicates the kind of data
	conversion to perform. The contents of the fGetPtr and fSetPtr fields are
	dependent on the values of the rpVariableType fields.
*/

typedef enum {
	eRpVarType_Direct,
	eRpVarType_Function,
	eRpVarType_Complex,
	eRpVarType_Snmp,
	eRpVarType_Custom
} rpVariableType;


/*
	The beginning of this list must match with the beginning of rpTextType as 
	defined in RpPages.h. This is so that RomPager can use AsReceiveText().
*/

typedef enum {
	eAsTextType_ASCII,				/* ASCII null-terminated string				*/
	eAsTextType_Hex,
	eAsTextType_HexColonForm,		/* Hex display with colons between chars	*/
	eAsTextType_DotForm,			/* Dotted decimal - nnn.nnn.nnn.nnn			*/
	eAsTextType_Signed8,
	eAsTextType_Signed16,
	eAsTextType_Signed32,
#if AsUse64BitIntegers
	eAsTextType_Signed64,
	eAsTextType_Unsigned64,
#endif
	eAsTextType_Unsigned8,
	eAsTextType_Unsigned16,
	eAsTextType_Unsigned32,
#if AsUseFloatingPoint
	eAsTextType_Float,
	eAsTextType_Double,
#endif
#if AsUseEnums
	eAsTextType_Enum1,				/* Enumerated type with 1 byte values		*/
	eAsTextType_Enum2,				/* Enumerated type with 2 byte values		*/
	eAsTextType_Enum4,				/* Enumerated type with 4 byte values		*/
#endif
	eAsTextType_BoolString,
	eAsTextType_BoolValue
#if 0
	eAsTextType_ASCII_Extended,		/* ASCII null-terminated - no expansion		*/
	eAsTextType_ASCII_Fixed			/* ASCII chars with length specified		*/
#endif
} asTextType;


#if AsSnmpAccess
/*
	The SNMP access definition.
*/

typedef struct {
	Unsigned32		fArray[32];		/*	OID array storage			*/
	Signed32		fCount;			/*	Item count in OID array 	*/
	Signed8			fGetNextIndex;	/*	Index for GetNext OID storage
											-1 means Get
											>= 0 means GetNext		*/
} asSnmpAccessItem, *asSnmpAccessItemPtr;
#endif


typedef struct {
	char *				fNamePtr;			/*	The HTML name of the text
												form item.                    */
	void *			fGetPtr;			/*	Pointer to either the memory 
											location that the Get value is 
											stored in, or a function to 
											retrieve the Get value.      */
	void *			fSetPtr;			/*	Pointer to either the memory 
											location that the Set value is 
											stored in, or a function to 
											store the Set value.         */
	rpVariableType	fGetPtrType;	
	rpVariableType	fSetPtrType;	
	asTextType		fTextType;			/*	Used to indicate what type of 
											conversion is necessary to turn 
												variable to and from text.    */
#if AsUseEnums
	asEnumElementPtr	fEnumElementPtr;	/*	Pointer to the list of enum 
												elements for this item.       */
#endif
	Unsigned8		fFieldMaxLength;	/*	Number of characters to 
											represent this item.         */
} asVarAccessItem, *asVarAccessItemPtr, **asVarAccessItemHdl;


/*********************************************************************
	eRpItemType_DisplayText,
	eRpItemType_NamedDisplayText,
 	eRpItemType_FormAsciiText,
	eRpItemType_FormPasswordText,
	eRpItemType_FormHiddenText,
	eRpItemType_FormTextDyn,
	eRpItemType_FormPasswordDyn,
	eRpItemType_FormHiddenDyn

		GetType = Function
		TextType = ASCII, ASCIIFixed, Hex, HexColonForm, DotForm
*/

typedef char * (*rpFetchBytesFuncPtr)(void);
	 
/*
		GetType = Complex
		TextType = ASCII, ASCIIFixed, Hex, HexColonForm, DotForm
*/
	 
typedef char * (*rpFetchBytesComplexPtr)(void *theTaskDataPtr, 
							char *theNamePtr, Signed16Ptr theIndexValuesPtr);

/*
		GetType = Function
		TextType = Unsigned8, Unsigned16, Unsigned32, Unsigned64
*/
	 
typedef Unsigned8 	(*rpFetchUnsigned8FuncPtr)(void);
typedef Unsigned16 	(*rpFetchUnsigned16FuncPtr)(void);
typedef Unsigned32 	(*rpFetchUnsigned32FuncPtr)(void);
#if AsUse64BitIntegers
typedef Unsigned64 	(*rpFetchUnsigned64FuncPtr)(void);
#endif

/*
		GetType = Complex
		TextType = Unsigned8, Unsigned16, Unsigned32, Unsigned64
*/
	 
typedef Unsigned8 	(*rpFetchUnsigned8ComplexPtr)(void *theTaskDataPtr, 
							char *theNamePtr, Signed16Ptr theIndexValuesPtr);
typedef Unsigned16 	(*rpFetchUnsigned16ComplexPtr)(void *theTaskDataPtr, 
							char *theNamePtr, Signed16Ptr theIndexValuesPtr);
typedef Unsigned32 	(*rpFetchUnsigned32ComplexPtr)(void *theTaskDataPtr, 
							char *theNamePtr, Signed16Ptr theIndexValuesPtr);
#if AsUse64BitIntegers
typedef Unsigned64 	(*rpFetchUnsigned64ComplexPtr)(void *theTaskDataPtr, 
							char *theNamePtr, Signed16Ptr theIndexValuesPtr);
#endif

/*
		GetType = Function
		TextType = Signed8, Signed16, Signed32, Signed64
*/
	 
typedef Signed8 	(*rpFetchSigned8FuncPtr)(void);
typedef Signed16 	(*rpFetchSigned16FuncPtr)(void);
typedef Signed32 	(*rpFetchSigned32FuncPtr)(void);
#if AsUse64BitIntegers
typedef Signed64 	(*rpFetchSigned64FuncPtr)(void);
#endif

/*
		GetType = Complex
		TextType = Signed8, Signed16, Signed32, Signed64
*/
	 
typedef Signed8  (*rpFetchSigned8ComplexPtr)(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
typedef Signed16 (*rpFetchSigned16ComplexPtr)(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
typedef Signed32 (*rpFetchSigned32ComplexPtr)(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
#if AsUse64BitIntegers
typedef Signed64 (*rpFetchSigned64ComplexPtr)(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
#endif

#if AsUseFloatingPoint
/*
		GetType = Function
		TextType = float, double
*/
typedef float	(*rpFetchFloatFuncPtr)(void);
typedef double	(*rpFetchDoubleFuncPtr)(void);

/*
		GetType = Complex
		TextType = float, double
*/
typedef float	(*rpFetchFloatComplexPtr)(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
typedef double	(*rpFetchDoubleComplexPtr)(void *theTaskDataPtr, 
						char *theNamePtr, Signed16Ptr theIndexValuesPtr);
#endif	/* AsUseFloatingPoint */

/*
		SetType = Function
		TextType = ASCII, ASCIIFixed, Hex, HexColonForm, DotForm
*/
typedef void	(*rpStoreAsciiTextFuncPtr)(char *theValuePtr);

/*
		SetType = Complex
		TextType = ASCII, ASCIIFixed, Hex, HexColonForm, DotForm
*/
typedef void	(*rpStoreAsciiTextComplexPtr)(void *theTaskDataPtr, 
						char *theValuePtr, char * theNamePtr, 
						Signed16Ptr theIndexValuesPtr);

/*
		SetType = Function
		TextType = Unsigned8, Unsigned16, Unsigned32, Unsigned64
*/
typedef void	(*rpStoreUnsigned8FuncPtr)(Unsigned8 theValue);
typedef void	(*rpStoreUnsigned16FuncPtr)(Unsigned16 theValue);
typedef void	(*rpStoreUnsigned32FuncPtr)(Unsigned32 theValue);
#if AsUse64BitIntegers
typedef void	(*rpStoreUnsigned64FuncPtr)(Unsigned64 theValue);
#endif
	 
/*
		SetType = Complex
		TextType = Unsigned8, Unsigned16, Unsigned32, Unsigned64
*/
typedef void	(*rpStoreUnsigned8ComplexPtr)(void *theTaskDataPtr, 
						Unsigned8 theValue, char *theNamePtr, 
						Signed16Ptr theIndexValuesPtr);
typedef void	(*rpStoreUnsigned16ComplexPtr)(void *theTaskDataPtr, 
						Unsigned16 theValue, char *theNamePtr, 
						Signed16Ptr theIndexValuesPtr);
typedef void	(*rpStoreUnsigned32ComplexPtr)(void *theTaskDataPtr, 
						Unsigned32 theValue, char *theNamePtr, 
						Signed16Ptr theIndexValuesPtr);
#if AsUse64BitIntegers
typedef void	(*rpStoreUnsigned64ComplexPtr)(void *theTaskDataPtr, 
						Unsigned64 theValue, char *theNamePtr, 
						Signed16Ptr theIndexValuesPtr);
#endif

/*
		SetType = Function
		TextType = Signed8, Signed16, Signed32, Signed64
*/
typedef void	(*rpStoreSigned8FuncPtr)(Signed8 theValue);
typedef void	(*rpStoreSigned16FuncPtr)(Signed16 theValue);
typedef void	(*rpStoreSigned32FuncPtr)(Signed32 theValue);
#if AsUse64BitIntegers
typedef void	(*rpStoreSigned64FuncPtr)(Signed64 theValue);
#endif

/*
		SetType = Complex
		TextType = Signed8, Signed16, Signed32, Signed64
*/
typedef void	(*rpStoreSigned8ComplexPtr)(void *theTaskDataPtr, 
					Signed8 theValue, char *theNamePtr, 
					Signed16Ptr theIndexValuesPtr);
typedef void	(*rpStoreSigned16ComplexPtr)(void *theTaskDataPtr, 
					Signed16 theValue, char *theNamePtr, 
					Signed16Ptr theIndexValuesPtr);
typedef void	(*rpStoreSigned32ComplexPtr)(void *theTaskDataPtr, 
					Signed32 theValue, char *theNamePtr, 
					Signed16Ptr theIndexValuesPtr);
#if AsUse64BitIntegers
typedef void	(*rpStoreSigned64ComplexPtr)(void *theTaskDataPtr, 
					Signed64 theValue, char *theNamePtr, 
					Signed16Ptr theIndexValuesPtr);
#endif

#if AsUseFloatingPoint
/*
		SetType = Function
		TextType = float, double
*/
typedef void	(*rpStoreFloatFuncPtr)(float theValue);
typedef void	(*rpStoreDoubleFuncPtr)(double theValue);

/*
		SetType = Complex
		TextType = float, double
*/
typedef void	(*rpStoreFloatComplexPtr)(void *theTaskDataPtr, 
						float theValue, char *theNamePtr, 
						Signed16Ptr theIndexValuesPtr);
typedef void	(*rpStoreDoubleComplexPtr)(void *theTaskDataPtr, 
						double theValue, char *theNamePtr, 
						Signed16Ptr theIndexValuesPtr);
#endif	/* AsUseFloatingPoint */

/*
	Variable Index call back routines
	
	These call back routines are used to modify the variable index state.  The
	AsPushVariableIndex routine places a new value on the variable index stack and 
	the AsPopVariableIndex routine pops a value off the stack and returns it to
	the caller.  These routines may be used to control the indices that are
	used by the variable access routines.  The values used in these routines are 
	internal index values and therefore are 0-relative.
	
	The AsGetVariableIndexLevel routine reports back the current index level in 
	use starting at 0.  If there are no index levels in use, the value returned
	will be -1.
*/

extern Signed16 	AsPopVariableIndex(void *theTaskDataPtr);
extern void 		AsPushVariableIndex(void *theTaskDataPtr, 
						Signed16 theMewIndexValue);
extern Signed8 		AsGetVariableIndexLevel(void *theTaskDataPtr);

/*
	The AsExpandString routine uses the phrase dictionaries to expand strings. 
	The routine takes a string to be expanded and a buffer and returns the 
	expanded string in the buffer. It returns eAsItemError_TooManyCharacters 
	if the buffer is too small.
*/

extern asItemError	AsExpandString(void *theTaskDataPtr, 
						char *theInputPtr,
						char *theOutputPtr, 
						Unsigned16 theBufferLength);

extern char *				gUserPhrasesEnglish[];
// +++ 12/13/2003, jacob_shih removed
#if 0
extern char *				gUserPhrasesFrench[];
extern char *				gUserPhrasesGerman[];
extern char *				gUserPhrasesItalian[];
extern char *				gUserPhrasesPortuguese[];
extern char *				gUserPhrasesSpanish[];
#endif
// --- 12/13/2003, jacob_shih removed

#endif	/* AsVariableAccess || RomPagerServer */

#if AsVariableAccess || RomPagerServer || RxConversions

/*
	String to number conversion routines
	
	The string to number routines are used internally by Variable Access and 
	RomXML. These routines may also be useful for applications that need to 
	convert strings to binary representations of variables.
*/

extern asItemError	AsDotFormStrToHex(char *theDotFormPtr,
						char *theHexPtr, Unsigned8 theOutputCharCount);

extern asItemError	AsStrToHex(char *theStringPtr, char theSeparator,
						char *theHexPtr, Unsigned8 theOutputByteCount);

extern asItemError	AsStrToSigned8(char *theStringPtr,
						Signed8Ptr theSigned8Ptr);

extern asItemError	AsStrToSigned16(char *theStringPtr,
						Signed16Ptr theSigned16Ptr);

extern asItemError	AsStrToSigned32(char *theStringPtr,
						Signed32Ptr theSigned32Ptr);

extern asItemError	AsStrToUnsigned8(char *theStringPtr,
						Unsigned8Ptr theUnsigned8Ptr);

extern asItemError	AsStrToUnsigned16(char *theStringPtr,
						Unsigned16Ptr theUnsigned16Ptr);

extern asItemError	AsStrToUnsigned32(char *theStringPtr,
						Unsigned32Ptr theUnsigned32Ptr);

#if AsUse64BitIntegers
extern asItemError	AsStrToSigned64(char *theStringPtr,
						Signed64Ptr theSigned64Ptr);

extern asItemError	AsStrToUnsigned64(char *theStringPtr,
						Unsigned64Ptr theUnsigned64Ptr);
#endif

#endif	/* AsVariableAccess || RomPagerServer || RxConversions */

#if AsVariableAccess || RxConversions

#if AsUseEnums
extern asItemError	AsStrToEnum8(char *theStringPtr, 
						asEnumElementPtr theEnumTablePtr, 
						Unsigned8Ptr theValuePtr);

extern asItemError	AsStrToEnum16(char *theStringPtr, 
						asEnumElementPtr theEnumTablePtr, 
						Unsigned16Ptr theValuePtr);

extern asItemError	AsStrToEnum32(char *theStringPtr, 
						asEnumElementPtr theEnumTablePtr, 
						Unsigned32Ptr theValuePtr);
#endif	/* AsUseEnums */

#endif	/* AsVariableAccess || RomPagerServer || RxConversions */


#endif	/* _ASVARACC_ */
