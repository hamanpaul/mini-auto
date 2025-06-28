/*
 *	File:		AsConvrt.h
 *
 *	Contains:	Error Definitions used by Conversion Routines
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
 *		02/11/03	rhb		add asEnumElementPtrPtr
 * * * * Release 4.20  * * *
 *		07/05/02	rhb		support enum types
 * * * * Release 4.12  * * *
 * * * * Release 4.07  * * *
 *		03/21/02	rhb		add eAsItemError_UnexpectedExtendedCharacter
 * * * * Release 4.06  * * *
 * * * * Release 4.00  * * *
 *		02/14/01	rhb		rename rpItemError and eRpItemError_* to 
 *								asItemError and eAsItemError_*
 *		12/14/00	bva		created
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */


#ifndef	_AS_CONVRT_
#define	_AS_CONVRT_


/*
	Error definitions used by conversion routines. 
	These error codes must remain in this order for RomPager.
*/

typedef enum {
	eAsItemError_UseErrorPagePtr						= -1,
	eAsItemError_NoError								=  0,
	eAsItemError_ItemNotFound,
	eAsItemError_SingleSelectionOptionNotFound,
	eAsItemError_MultiSelectionOptionNotFound,
	eAsItemError_IllegalHexCharacter,
	eAsItemError_OddNumberOfHexCharacters,
	eAsItemError_ExpectingAColonInHexColonForm,
	eAsItemError_TooManyCharacters,
	eAsItemError_Signed8TooSmall,
	eAsItemError_Signed8TooLarge,
	eAsItemError_Signed16TooSmall,
	eAsItemError_Signed16TooLarge,
	eAsItemError_Signed32TooSmall,
	eAsItemError_Signed32TooLarge,
	eAsItemError_Signed64TooSmall,
	eAsItemError_Signed64TooLarge,
	eAsItemError_Unsigned8TooLarge,
	eAsItemError_Unsigned16TooLarge,
	eAsItemError_Unsigned32TooLarge,
	eAsItemError_Unsigned64TooLarge,
	eAsItemError_ExpectingADecimalDigit,
	eAsItemError_ExpectingAnInteger,
	eAsItemError_UnexpectedExtendedCharacter,
#if AsUseEnums
	eAsItemError_NoSuchEnumValue,
	eAsItemError_NoSuchEnumString,
#endif
	eAsItemError_UserError					/* must be last	*/
} asItemError;


#if AsUseEnums
/*
	EnumElement structure for Enum Tables
*/
typedef struct {
	char *		fStringPtr;
	Unsigned32	fValue;
} asEnumElement, *asEnumElementPtr, **asEnumElementPtrPtr;
#endif



#endif	/* _AS_CONVRT_ */
