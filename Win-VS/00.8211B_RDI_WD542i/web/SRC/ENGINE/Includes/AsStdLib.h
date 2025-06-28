/*
 *	File:		AsStdLib.h
 *
 *	Contains:	abstraction of standard C library definitions for the RomPager
 *				product family
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
 * * * * Release 4.20  * * *
 *		10/04/02	amp		enable RP_STRCASECMP for RomPagerSecure
 *		10/04/02	amp		add RP_MEMMOVE
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *		07/18/02	pjr		enable strcasecmp routines for RomWebClient
 * * * * Release 4.10  * * *
 * * * * Release 4.07  * * *
 *		11/12/01	rhb		always #define RP_REALLOC
 * * * * Release 4.06  * * *
 * * * * Release 4.00  * * *
 *		05/24/01	rhb		rework RP_OFFSETOF
 *		03/06/01	bva		rework RP_STRCASECMP, RP_STRNCASECMP
 *		02/09/01	dts		add RP_STRNCMP
 *		09/14/00	rhb		add RP_OFFSETOF for RomXML and RomIPP
 *		09/01/00	dts		add RP_STRCASECMP, RP_STRNCASECMP
 *		07/25/00	rhb		add RP_REALLOC for SSL Plus 3 & BSAFE
 * * * * Release 3.0 * * * *
 *		03/16/99	pjr		rework debug code
 *		02/06/99	bva		RpStdLib.h -> AsStdLib.h
 *		01/06/99	bva		add RP_ATOI
 *		12/31/98	pjr		add RP_MEMCMP
 * * * * Release 2.2 * * * *
 *		12/01/98	bva		add RP_ATOL
 *		11/05/98	bva		created
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */


#ifndef	_AS_STDLIB_
#define	_AS_STDLIB_

/*	C Headers  */
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#if AsDebug
#include <stdio.h>
#endif


#define RP_MEMCMP(theDestination, theSource, theLength) \
		(memcmp(theDestination, theSource, theLength))  

#define RP_MEMCPY(theDestination, theSource, theLength) \
		(memcpy(theDestination, theSource, theLength))  

#define RP_MEMMOVE(theDestination, theSource, theLength) \
		(memmove(theDestination, theSource, theLength))

#define RP_MEMSET(theDestination, theValue, theLength) \
		(memset(theDestination, theValue, theLength))  

#define RP_STRCPY(theDestination, theSource) \
		(strcpy(theDestination, theSource))  

#define RP_STRCAT(theDestination, theSource) \
		(strcat(theDestination, theSource))  

#define RP_STRCMP(theFirstString, theSecondString) \
		(strcmp(theFirstString, theSecondString))  

#define RP_STRNCMP(theFirstString, theSecondString, theLength) \
		(strncmp(theFirstString, theSecondString, theLength))  

#define RP_STRLEN(theString) \
		(strlen(theString))  

#define RP_ALLOC(theLength) \
		(malloc(theLength))  

#define RP_FREE(theMemoryBlockPtr) \
		(free(theMemoryBlockPtr))  

#define RP_ATOL(theStringPtr) \
		(atol(theStringPtr))  

#define RP_ATOI(theStringPtr) \
		(atoi(theStringPtr))  

#define RP_REALLOC(theMemoryBlockPtr, theNewLength) \
		(realloc(theMemoryBlockPtr, theNewLength))  

#define RP_OFFSETOF(type, member) \
		(offsetof(type, member))

#if RomCli || RomWebClient || RomPagerSecure || RomWebClientSecure
/*
	These macros provide hooks to the strcasecmp and strncasecmp routines 
	that are included in some C system libraries. If the C system libraries
	do not include these routines, then the ones in AsCommon.c may be used.
*/
#define RP_STRCASECMP(theFirstString, theSecondString) \
		(strcasecmp(theFirstString, theSecondString))  

#define RP_STRNCASECMP(theFirstString, theSecondString, theLength) \
		(strncasecmp(theFirstString, theSecondString, theLength))
#endif	/* RomCli || RomWebClient || RomPagerSecure || RomWebClientSecure */

#if AsDebug
#define RP_PRINTF printf  
#endif


#endif
