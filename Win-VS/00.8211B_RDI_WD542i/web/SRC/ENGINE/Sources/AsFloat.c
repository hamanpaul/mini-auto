/*
 *  File:       AsFloat.c
 *
 *  Contains:   Double precision floating point using standard 'C' library
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Copyright:  © 1995-2003 by Allegro Software Development Corporation
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
 *  Change History (most recent first):
 *
 * * * * Release 4.30  * * *
 *      09/11/03    bva     fix conditional
 *      06/26/03    nam     created
 * * * * Release 4.20  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *  To Do:
 */


#include "AsEngine.h"

#if AsUseFloatingPoint

#include <stdio.h>

double AsStringToFloat(char *theStringPtr) {
	double	theFloatValue;

	sscanf(theStringPtr, "%lg", &theFloatValue);
	return theFloatValue;
}

Unsigned16 AsFloatToString(double theFloatValue, char *theStringPtr) {

	sprintf(theStringPtr, "%g", theFloatValue);
	return (Unsigned16)RP_STRLEN(theStringPtr);
}

#endif
