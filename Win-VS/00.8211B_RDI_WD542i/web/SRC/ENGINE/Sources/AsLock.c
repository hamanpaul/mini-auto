/*
 *	File:		AsLock.c
 *
 *	Contains:	Resource Lock Support
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2003 by Allegro Software Development Corporation
 *	All rights reserved.
 *
 *	This module contains confidential, unpublished, proprietary
 *	source code of Allegro Software Development Corporation.
 *
 *	The copyright notice above does not evidence any actual or intended
 *	publication of such source code.
 *
 *	License is granted for specific uses only under separate
 *	written license by Allegro Software Development Corporation.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 * * * * Release 4.30  * * *
 *		09/21/03	bva		force unsupported OS to fail AsCreateLocks
 *		06/03/03	amp		created
 * * * * Release 4.20  * * *
 * * * * Release 4.00  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#include "AsEngine.h"

/*
	AsLock.c defines an API for locking critical resources to
	prevent contention problems.
	The implementation is operating system specific.
*/

#if AsResourceLocks

#if (RpTargetOS == eRpTargetWin32)
#include "windows.h"
typedef	HANDLE		ResourceLock, *ResourceLockPtr;
#elif (RpTargetOS == eRpTargetPSOS)
#include <psos.h>
typedef	unsigned long	ResourceLock, *ResourceLockPtr;
#elif (RpTargetOS == eRpTargetLinux)
#include <pthread.h>
#include <errno.h>
typedef	pthread_mutex_t	ResourceLock, *ResourceLockPtr;
#endif

#define	kAsNumResourceLocks	((Unsigned16) eLockIdNumLocks)

#if !RomPagerDynamicGlobals
static ResourceLock	gAsLocks[kAsNumResourceLocks];
#endif

/*
	AsCreateLocks is called by the engine at startup.
	The number of resource locks to create is indicated by kAsNumResourceLocks.
*/
void * AsCreateLocks(void *theTaskDataPtr) {
	Boolean				theFailedFlag;
	Unsigned32			theIndex;
	ResourceLockPtr		theResourceLockPtr;
#if (RpTargetOS == eRpTargetPSOS)
	char				theName[4];
	unsigned long		theStatus;
#endif

	theFailedFlag = False;
#if RomPagerDynamicGlobals
	/*
		Allocate memory for the ResourceLocks.
	*/
	theResourceLockPtr = (ResourceLockPtr)
			RP_ALLOC(kAsNumResourceLocks * sizeof(ResourceLock));
	if (theResourceLockPtr == (ResourceLockPtr) 0) {
		theFailedFlag = True;
	}
#else 
	/*
		Use memory allocated at build-time.
	*/
	theResourceLockPtr = gAsLocks;
#endif

	/*
		Create an array of resource locks.
	*/
	theIndex = 0;
	while (!theFailedFlag && theIndex < kAsNumResourceLocks) {
#if (RpTargetOS == eRpTargetWin32)
		theResourceLockPtr[theIndex] = CreateMutex(NULL, False, NULL);
		if (theResourceLockPtr[theIndex] == NULL) {
			theFailedFlag = True;
		}
#elif (RpTargetOS == eRpTargetPSOS)
		RP_STRCPY(theName, "AS");
		theName[2] = '0' + theIndex;
		theName[3] = '\0';
		theStatus = sm_create(theName,
							1,
							SM_LOCAL | SM_PRIOR,
							&theResourceLockPtr[theIndex]);
		if (theStatus != 0) {
			theFailedFlag = True;
		}
#elif (RpTargetOS == eRpTargetLinux)
		/*
			pthread_mutex_init  initializes the mutex object pointed to by mutex 
			according to the mutex attributes specified in mutexattr.  If 
			mutexattr is NULL, default attributes are used instead.

	 		pthread_mutex_init always returns 0.
		*/
		pthread_mutex_init(&theResourceLockPtr[theIndex], NULL);
#else
		theFailedFlag = True;
#endif
		theIndex++;
	}

	if (theFailedFlag) {
		theResourceLockPtr = (void *) 0;
#if RomPagerDynamicGlobals
		/*
			Free the ResourceLocks memory .
		*/
		RP_FREE(theResourceLockPtr);
#endif
		/*
			Return a null pointer on failure.
		*/
	}
	return (void *) theResourceLockPtr;
}


/*
	AsDestroyLocks is called by the engine at shutdown.
*/
void AsDestroyLocks(void *theTaskDataPtr, void *theLockDataPtr) {
	Unsigned32		theIndex;
	ResourceLockPtr	theResourceLockPtr;
#if (RpTargetOS == eRpTargetLinux)
	int theErrorCode;
#endif

	theIndex = 0;
	theResourceLockPtr = (ResourceLockPtr) theLockDataPtr;
	while (theResourceLockPtr != (ResourceLockPtr) 0 &&
			theIndex < kAsNumResourceLocks) {
#if (RpTargetOS == eRpTargetWin32)
		CloseHandle(theResourceLockPtr[theIndex]);
#elif (RpTargetOS == eRpTargetPSOS)
		sm_delete(theResourceLockPtr[theIndex]);
#elif (RpTargetOS == eRpTargetLinux)
		/*
			pthread_mutex_destroy destroys a mutex object, freeing the 
			resources it might hold. The mutex must be unlocked on entrance.
			In the LinuxThreads implementation, no resources are associated
			with mutex objects, thus pthread_mutex_destroy actually does
			nothing except checking that the mutex is unlocked.
		*/
		theErrorCode = pthread_mutex_destroy(&theResourceLockPtr[theIndex]);
		if theErrorCode == EBUSY) {
			/* 
				the mutex is currently locked.
			*/
		}
#endif
		theIndex++;
	}

#if RomPagerDynamicGlobals
	/*
		Free the ResourceLocks memory .
	*/
	if (theResourceLockPtr != (ResourceLockPtr) 0) {
		RP_FREE(theResourceLockPtr);
	}
#endif

	return;
}


/*
	AsLockResource is called to lock a resource.
*/
void AsLockResource(void *theTaskDataPtr, asLockId theId) {
	ResourceLockPtr		theResourceLockPtr;
	rpDataPtr			theRpDataPtr;
#if (RpTargetOS == eRpTargetLinux)
	int theErrorCode;
#endif

	theRpDataPtr = (rpDataPtr) theTaskDataPtr;
	theResourceLockPtr = (ResourceLockPtr) theRpDataPtr->fLockDataPtr;

#if (RpTargetOS == eRpTargetWin32)
	WaitForSingleObject(theResourceLockPtr[(Unsigned8) theId], INFINITE);
#elif (RpTargetOS == eRpTargetPSOS)
	sm_p(theResourceLockPtr[(Unsigned8) theId], SM_WAIT, 0);
#elif (RpTargetOS == eRpTargetLinux)
	/*
		pthread_mutex_lock locks the given mutex. If the mutex is currently 
		unlocked, it becomes locked and owned by the calling thread, and 
		pthread_mutex_lock returns immediately. If the mutex is already 
		locked by another thread, pthread_mutex_lock suspends the calling 
		thread until the mutex is unlocked.
	*/
	theErrorCode = pthread_mutex_lock(theResourceLockPtr);
	if (theErrorCode == EINVAL) {
		/* 
			the mutex was not been properly initialized.
		*/
#if AsDebug
		RP_PRINTF("Mutex was not been properly initialized\n");
#endif
	}
#endif
	return;
}


/*
	AsReleaseResource is called to release a locked resource.
*/
void AsReleaseResource(void *theTaskDataPtr, asLockId theId) {
	ResourceLockPtr		theResourceLockPtr;
	rpDataPtr			theRpDataPtr;
#if (RpTargetOS == eRpTargetLinux)
	int theErrorCode;
#endif
	theRpDataPtr = (rpDataPtr) theTaskDataPtr;
	theResourceLockPtr = (ResourceLockPtr) theRpDataPtr->fLockDataPtr;

#if (RpTargetOS == eRpTargetWin32)
	ReleaseMutex(theResourceLockPtr[(Unsigned8) theId]);
#elif (RpTargetOS == eRpTargetPSOS)
	sm_v(theResourceLockPtr[(Unsigned8) theId]);
#elif (RpTargetOS == eRpTargetLinux)
	/*
		pthread_mutex_unlock unlocks the given mutex.  The mutex is assumed
		to be locked and owned by the calling thread on entrance to
		pthread_mutex_unlock. If the mutex is of the "fast" kind, 
		pthread_mutex_unlock always returns it to the unlocked state. If it 
		is of the "recursive" kind, it decrements the locking count of the
		mutex (number of pthread_mutex_lock operations performed on it by the 
		calling thread), and only when this count reaches zero is the mutex 
		actually unlocked.

		On  "error checking" mutexes, pthread_mutex_unlock actually checks at 
		run-time that the mutex is locked on entrance, and that it was locked 
		by the same thread that is now calling pthread_mutex_unlock.  If these 
		conditions are not met, an error code is returned and the mutex remains
		unchanged.  "Fast" and "recursive" mutexes perform no such checks, thus 
		allowing a locked mutex to be unlocked by a thread other than its owner.
		This is non-portable behavior and must not be relied upon.
	*/

	theErrorCode = pthread_mutex_unlock(&theResourceLockPtr[(Unsigned8) theId]);
	if (theErrorCode == EINVAL) {
		/*
			The mutex was not properly initialized.
		*/
#if AsDebug
		RP_PRINTF("Mutex was not been properly initialized\n");
#endif
	}
#endif
	return;
}

#endif	/* RomPlugAdvanced */
