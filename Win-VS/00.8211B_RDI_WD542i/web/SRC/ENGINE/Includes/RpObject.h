/*
 *	File:		RpObject.h
 *
 *	Contains:	Elements and structures used for RomPager object descriptions
 *				in RomPager Basic and RomMailer used without RomPager Advanced
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
 * * * * Release 4.12  * * *
 *		09/12/02	bva		add eRpObjectTypeDynamicClose
 * * * * Release 4.11  * * *
 * * * * Release 4.00  * * *
 *		01/18/00	bva		RomPagerLight -> RomPagerBasic
 * * * * Release 3.0 * * * *
 *		01/22/99	pjr		created from RpPages.h
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */


#ifndef	_RP_OBJECT_
#define	_RP_OBJECT_


/*
	Object Type Definitions for cache control
*/
typedef enum {
	eRpObjectTypeStatic,
	eRpObjectTypeDynamic,
	eRpObjectTypeDynamicClose
} rpObjectType;


/*
	The Object structure definition
*/
typedef struct rpObjectDescription {
	char *					fURL;					/*	Path used to find 
														object                */
	Unsigned32				fLength;				/*	Length of data in 
														object (used for images 
														and applets)          */
	rpDataType				fMimeDataType;			/*	Indicates page type 
														(used for MIME type 
														checking)             */
#if RomPagerBasic
	rpObjectType			fCacheObjectType;		/*	Static, Dynamic and File
														types will trigger
														different HTTP 
														headers to control 
														browser caching       */
#endif
} rpObjectDescription, *rpObjectDescriptionPtr, **rpObjectDescPtrPtr;


#endif	/* _RP_OBJECT_ */
