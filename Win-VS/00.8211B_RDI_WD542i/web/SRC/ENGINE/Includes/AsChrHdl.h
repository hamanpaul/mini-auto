/*
 *	File:		AsChrHdl.h
 *
 *	Contains:	Character Handler routines for RomPager product family
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
 *		12/18/02	amp     move rszFunc here
 * * * * Release 4.12  * * *
 * * * * Release 4.10  * * *
 *		04/08/02	dts		change rcvFunc to type Boolean for backpressure
 * * * * Release 4.07  * * *
 * * * * Release 4.00  * * *
 *		09/03/00	dts		updated xmitChar, xmitString
 *		07/31/00	dts		created
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

/*
	The function pointer types defined here are used by RomCli and RomTelnet
	and must not be defined inside either package.
*/

#ifndef _ASCHAR_H_
#define _ASCHAR_H_

/*
	This dummy structure is needed to keep some compilers happy with
	the procedure definitions that follow.
*/
typedef struct {
	struct rpConnection * 	fConnectionPtr;
} tnDummyStruct, *tnDummyStructPtr;

typedef int xmitChar(struct rpConnection *, const char x);
typedef int xmitString(struct rpConnection *, const char *x, Unsigned16 length);
typedef int xmitStatus(struct rpConnection *);
typedef Boolean rcvFunc(struct rpConnection *, char x);
typedef void termFunc(struct rpConnection *);
typedef void rszFunc(struct rpConnection *, Unsigned16, Unsigned16);

#endif
