/*
 *	File:		AsGlobal.c
 *
 *	Contains:	Global Data for RomPager Product Family
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
 * * * * Release 4.00  * * *
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		05/26/00	bva		add gRpDataPtr
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 *		01/22/99	pjr		RpGlobal.c -> AsGlobal.c
 * * * * Release 2.2 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.6 * * * *
 *		04/25/97	bva		remove gRpStatus
 * * * * Release 1.5 * * * *
 *		11/04/96	bva		move dates and phrase dictionary to RpData.c
 *		10/10/96	rhb		add phrase dictionary
 *		09/24/96	rhb		support dynamically allocated engine data 
 *		09/20/96	rhb		allow more than one HTTP request
 * * * * Release 1.4 * * * *
 *		08/16/96	bva		add gRpSubmitButtonValue
 * * * * Release 1.3 * * * *
 *		07/08/96	bva		add gRpIndexDepth, gRpIndexValues
 *		07/03/96	bva		add gRpBoxNameText
 *		07/02/96	bva		add gRpStatus
 *		06/28/96	bva		rework connection structure
 *		06/13/96	bva		add access control variables
 * * * * Release 1.2 * * * *
 *		05/31/96	bva		add gRpMonthDays
 *		05/30/96	bva		align variable sizes to eliminate warnings
 * * * * Release 1.1 * * * *
 *		04/25/96	bva		rework top level multi-tasking
 * * * * Release 1.0 * * * *
 *		03/03/96	bva		added image mapping support
 *		11/08/95	bva		created
 *
 *	To Do:
 */

#include "AsEngine.h"

#if !RomPagerDynamicGlobals
// +++ _Alphanetworks_Patch_, 10/08/2003, jacob_shih
#if defined(WIN32)
rpData		gRpData;
#else
rpData		gRpData[kNumberOfTaskData];
#endif	// WIN32
// --- _Alphanetworks_Patch_, 11/04/2003, jacob_shih
#endif
