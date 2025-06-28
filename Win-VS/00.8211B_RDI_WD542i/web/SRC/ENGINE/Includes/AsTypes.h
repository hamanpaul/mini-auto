/*
 *	File:		AsTypes.h
 *
 *	Contains:	Data types for the RomPager product family
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
 * * * * Release 4.10  * * *
 *		05/21/01	rhb		conditionalize True,False
 * * * * Release 4.07  * * *
 * * * * Release 4.00  * * *
 *		07/26/01	rhb/pjr	make 64-bit definitions compiler dependent
 *		02/13/01	rhb		move 64-bit typedefs back
 *		06/14/99	bva		move 64-bit typedefs to RpConfig.h
 * * * * Release 3.0 * * * *
 *		02/06/99	bva		RpTypes.h -> AsTypes.h
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 *		11/10/97	rhb		Signed64, Signed64Ptr, Unsigned64, and Unsigned64Ptr
 * * * * Release 1.6 * * * *
 *		01/26/97	rhb/bva	move Boolean to RpConfig.h (really!)
 *		01/04/97	bva		return Boolean from RpConfig.h
 * * * * Release 1.5 * * * *
 *		11/05/96	rhb		move Boolean to RpConfig.h
 * * * * Release 1.4 * * * *
 * * * * Release 1.3 * * * *
 *		07/30/96	bva		remove BooleanPtr
 * * * * Release 1.2 * * * *
 * * * * Release 1.0 * * * *
 *		11/01/95	rhb		created
 *
 *	To Do:
 */

#ifndef	_ASTYPES_
#define	_ASTYPES_

#if (RpTargetOS != eRpTargetOSE)
#ifndef True
	#define False			0
	#define True			1
#endif 
#endif 


typedef signed char			Signed8, 	*Signed8Ptr;
typedef signed short		Signed16, 	*Signed16Ptr;
typedef signed long			Signed32, 	*Signed32Ptr;
typedef unsigned char		Unsigned8, 	*Unsigned8Ptr;
typedef unsigned short		Unsigned16, *Unsigned16Ptr;
typedef unsigned long		Unsigned32, *Unsigned32Ptr;

#if AsUse64BitIntegers

/*
	The 64 bit definitions are compiler dependent.  Specific
	definitions have been provided for some compilers.  If the
	compiler being used is not recognized, the definitions
	from the ANSI C99 standard are used.

	If you are using a compiler that does not work with these
	definitions, you can add a definition specific to your
	compiler.  In this case, please notify Allegro of the
	change so that it can be included in future releases.
*/

#if defined(_MSC_VER)
typedef signed __int64		Signed64, 	*Signed64Ptr;
typedef unsigned __int64	Unsigned64, *Unsigned64Ptr;
#define As64Bit(theInteger)	theInteger

#elif defined(__GNUC__)
typedef signed long long	Signed64, 	*Signed64Ptr;
typedef unsigned long long	Unsigned64, *Unsigned64Ptr;
#define As64Bit(theInteger)	theInteger ## LL

#else
typedef signed long long	Signed64, 	*Signed64Ptr;
typedef unsigned long long	Unsigned64, *Unsigned64Ptr;
#define As64Bit(theInteger)	theInteger
#endif

#endif	/* AsUse64BitIntegers */

#endif	/* _ASTYPES_ */
