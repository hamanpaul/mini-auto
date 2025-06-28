/*
 *	File:		AsLock.h
 *
 *	Contains:	Resource Locking external definitions 
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
 *		06/30/03	amp		created
 * * * * Release 4.21  * * *
 * * * * Release 4.20  * * *
 * * * * Release 4.00  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_AS_LOCK_
#define	_AS_LOCK_

/*
	Resource Lock definitions
*/
typedef enum {
#if AsResourceLocks
	eLockIdRomPlug,
	eLockIdRomPlugAppData,
#endif
	eLockIdNumLocks
} asLockId;

/*
	AsLock.c
*/
#if AsResourceLocks
extern void		AsLockResource(void *theTaskDataPtr, asLockId theId);
extern void		AsReleaseResource(void *theTaskDataPtr, asLockId theId);
#endif

#endif /* _AS_LOCK_ */
