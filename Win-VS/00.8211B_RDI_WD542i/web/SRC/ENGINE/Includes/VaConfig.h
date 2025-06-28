/*
 *	File:		VaConfig.h
 *
 *	Contains:	Variable Access Configuration Definitions
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
 *		08/27/03	rhb		move AsUse64BitIntegers, AsUseFloatingPoint, and 
 *								AsUseEnums here from AsConfig.h, separate out 
 *								kAsHexSeparator, and move conditionals here 
 *		07/08/03	bva		rework ifdefs
 * * * * Release 4.21  * * *
 * * * * Release 4.00  * * *
 *		05/23/01	rhb		move AsUse64BitIntegers to AsConfig.h
 *		04/19/01	bva		created from AsConfig.h
 * * * * Release 3.10 * * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_VACONFIG_
#define	_VACONFIG_


/*
	Extended Conversion Types

	AsUse64BitIntegers, AsUseEnums, AsUseFloatingPoint
	
	If AsUse64BitIntegers is defined as 1, integer variables can be 64-bits.
	
	If AsUseEnums is defined as 1, enumerated type conversion code is generated.

	AsUseFloatingPoint enables floating point conversion for float and double
	data types.
*/

#define AsUse64BitIntegers 				0		/*  1525 bytes  */

#define AsUseEnums		 				0		/*  ??? bytes  */

#define AsUseFloatingPoint		 		0		/*  ??? bytes  */



#if AsVariableAccess || RomPagerServer

/*
	Variable Access
	
	Variables can be accessed from a variety of sources. The various flags below 
	are used to control what sources are available. 
	

	**** **** **** **** **** **** ****

	AsSnmpAccess
	
	If AsSnmpAccess is defined as 1, then SNMP OIDs can by used by the Variable 
	Access routines. The value kAsSnmpGetNextCount defines the  number of 
	simultaneous OIDs to store for GetNext loops.

	**** **** **** **** **** **** ****

	AsCustomVariableAccess
	
	If AsCustomVariableAccess is defined as 1, then the Variable Access routines 
	can use customer defined routines for variable retrieval and storage.

	**** **** **** **** **** **** ****

	kAsIndexQueryDepth
	
	The kAsIndexQueryDepth definition is used to set the size of a number of 
	arrays used for control of nesting HTML, and indexed queries.  In general, 
	it should be set high enough to allow for the deepest indexed reference 
	made in a Set/Get routine.  That is, these are the arrays used to support 
	routines of the form GetVariable(myVariable, i, j, k).  Each index level
	requires about 50 bytes of RAM.  The default setting allows for 5 levels,
	or variables of the form GetVariable(myVariable, i, j, k, l, m).  The 
	nesting depth for item groups (eRpItemType_ItemGroup) is also defined by 
	this definition and is defined as 3 greater than the value of 
	kAsIndexQueryDepth with a default value of 8.

*/

#define AsSnmpAccess					0		/*  1600 bytes  */

#if AsSnmpAccess			
	#define kAsSnmpGetNextCount			10	
#endif

#define AsCustomVariableAccess			0		/*  900 bytes  */

// +++ _Alphanetworks_Patch_, 11/28/2003, jacob_shih
#if 1
#define kAsIndexQueryDepth 				32
#else
#define kAsIndexQueryDepth 				5
#endif
// --- _Alphanetworks_Patch_, 11/28/2003, jacob_shih


/* 
	User Phrase dictionary
	
	kAsCompressionEscape, kAsCompressionEscapeString
		
	The kAsCompressionEscape value defines the character used to escape
	phrase dictionary expansion.  The default value is '\373', so to display
	the word "Espanol" with a n-tilde, the string would be "Espa\373\361ol".
	
	The kAsCompressionEscape value also is used to indicate the start of the 
	range of signal characters used for the user dictionary.  The default 
	setting of '\373' means that the four characters '\374' to '\377' are 
	used to select phrases in the user dictionary.  The first quarter of the 
	phrases are selected with the '\377' character and the last quarter of 
	the phrases are selected with the '\374' character.  A complete
	description of the phrase dictionary is in RpDict.h  
*/

#define	kAsCompressionEscape		'\373'	
#define	kAsCompressionEscapeString	"\373"

#endif	/* AsVariableAccess || RomPagerServer */


#if AsVariableAccess || RomPagerServer || RomXml

/*
	kAsHexSeparator
	
	The kAsHexSeparator character is used to define the character used to 
	separate hex bytes displayed by the text data type 
	eRpTextType_HexColonForm. The default definition is a ':' which can be 
	used to display hex strings of the form "hh:hh:hh:hh".  By changing 
	this value to a '-' for instance, the hex strings would be displayed 
	as "hh-hh-hh-hh".
*/	

#define kAsHexSeparator					':'	

#endif	/* AsVariableAccess || RomPagerServer || RomXml */


#endif	/* _VACONFIG_ */
