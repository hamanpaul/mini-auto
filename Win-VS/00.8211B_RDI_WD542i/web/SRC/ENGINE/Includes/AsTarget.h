/*
 *	File:		AsTarget.h
 *
 *	Contains:	Operating Environment identifiers for RomPager
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
 * * * * Release 4.07  * * *
 *		05/15/02	bva		add eRpTargetLinux, eRpTargetThreadX, 
 *							eRpTargetIntegrity
 * * * * Release 4.06  * * *
 * * * * Release 4.00  * * *
 *		05/15/01	pjr		add eRpTargetEz80 and eRpTargetOSE
 * * * * Release 3.0 * * * *
 *		02/06/99	bva		RpTarget.h -> AsTarget.h
 *		12/14/98	bva		added SNMP definitions
 * * * * Release 2.2 * * * *
 *		07/18/98	bva		created
 * * * * Release 2.1 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_AS_TARGET_
#define	_AS_TARGET_

/* 
	RomPager target operating systems 
*/

#define eRpTargetGeneric 		0
#define eRpTargetWin32 			(eRpTargetGeneric + 1)
#define eRpTargetUnix			(eRpTargetWin32 + 1)
#define eRpTargetMacOS			(eRpTargetUnix + 1)
#define eRpTargetTestNoStd		(eRpTargetMacOS + 1)
#define eRpTargetTestNoCal		(eRpTargetTestNoStd + 1)
#define eRpTargetElmic			(eRpTargetTestNoCal + 1)
#define eRpTargetHelios			(eRpTargetElmic + 1)
#define eRpTargetLynx			(eRpTargetHelios + 1)
#define eRpTargetNucleus		(eRpTargetLynx + 1)
#define eRpTargetOS9			(eRpTargetNucleus + 1)
#define eRpTargetPrecise		(eRpTargetOS9 + 1)
#define eRpTargetPSOS			(eRpTargetPrecise + 1)
#define eRpTargetQNX			(eRpTargetPSOS + 1)
#define eRpTargetVRTX			(eRpTargetQNX + 1)
#define eRpTargetVxWorks		(eRpTargetVRTX + 1)
#define eRpTargetEz80			(eRpTargetVxWorks + 1)
#define eRpTargetOSE			(eRpTargetEz80 + 1)
#define eRpTargetLinux			(eRpTargetOSE + 1)
#define eRpTargetThreadX		(eRpTargetLinux + 1)
#define eRpTargetIntegrity		(eRpTargetThreadX + 1)
#define eRpTargetUCOS2		    (eRpTargetIntegrity + 1)

//#define RpTargetOS	eRpTargetMacOS
#define RpTargetOS	eRpTargetUCOS2

/* 
	RomPager target SNMP environments 
*/

#define eRpSnmpGeneric 			0
#define eRpSnmpResearch 		(eRpSnmpGeneric + 1)
#define eRpSnmpEpilogue			(eRpSnmpResearch + 1)

#define RpTargetSnmp	eRpSnmpResearch


#endif
