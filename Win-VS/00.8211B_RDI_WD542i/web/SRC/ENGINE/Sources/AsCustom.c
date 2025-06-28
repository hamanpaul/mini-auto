/*
 *	File:		AsCustom.c
 *
 *	Contains:	RomPager shell routines for custom variable access
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
 *		07/08/03	bva		rework ifdefs
 * * * * Release 4.21  * * *
 * * * * Release 4.02  * * *
 *		09/18/01	bva		comment out sample code
 * * * * Release 4.00  * * *
 *		02/16/01	rhb		renamed file and functions from Rp* to As* 
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 *		12/28/98	bva		created
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"

#if AsCustomVariableAccess

/*
	This function is called by the various access routines in AsVarAcc.c.
	The routine is passed in the master pointer to engine information, 
	the HTML name of the item, a pointer to the index array, and the 
	type of the value to be retrieved. It returns a pointer to the value 
	in native format for conversion to string by the AsVarAcc.c routines.
*/ 

void * AsRetrieveCustomItem(void *theTaskDataPtr, 
								char *theHtmlNamePtr, 
								Signed16Ptr theIndexValuesPtr,
								asTextType theItemDataType) {
#if 0
	Signed16	theIndexI;						
	Signed16	theIndexJ;						
	Signed16	theIndexK;						
	Signed16	theIndexL;						
	Signed16	theIndexM;
#endif
	void		*theValuePtr = (void *) 0;						
	
	
#if 0
	theIndexI = *theIndexValuesPtr;
	theIndexJ = *(theIndexValuesPtr + 1);
	theIndexK = *(theIndexValuesPtr + 2);
	theIndexL = *(theIndexValuesPtr + 3);
	theIndexM = *(theIndexValuesPtr + 4);
#endif
	
	return theValuePtr;				
}

/*
	This function is called by the various storage routines in AsVarAcc.c.
	The routine is passed in the master pointer to engine information, 
	the HTML name of the item, a pointer to the index array, the type of 
	the value to be stored and a pointer to the value to be stored.
*/ 

void AsStoreCustomItem(void *theTaskDataPtr, 
							char *theHtmlNamePtr, 
							Signed16Ptr theIndexValuesPtr,
							asTextType theItemDataType,
							void *theItemPtr) {
#if 0
	Signed16	theIndexI;						
	Signed16	theIndexJ;						
	Signed16	theIndexK;						
	Signed16	theIndexL;						
	Signed16	theIndexM;						
	
	theIndexI = *theIndexValuesPtr;
	theIndexJ = *(theIndexValuesPtr + 1);
	theIndexK = *(theIndexValuesPtr + 2);
	theIndexL = *(theIndexValuesPtr + 3);
	theIndexM = *(theIndexValuesPtr + 4);
#endif

	return;				
}


#endif	/* AsCustomVariableAccess */
