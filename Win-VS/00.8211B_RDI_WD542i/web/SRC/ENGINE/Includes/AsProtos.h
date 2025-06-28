/*
 *	File:		AsProtos.h
 *
 *	Contains:	Scheduler routine prototypes for the RomPager product family.
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
 *      12/18/02    amp     add eAsSshServer for RomCliSecure
 * * * * Release 4.12  * * *
 * * * * Release 4.00  * * *
 *		05/22/01	amp		add C++ compatibility
 *		07/25/00	rhb		support SSL/TLS
 *		01/18/00	bva		add multi-port server prototypes
 * * * * Release 3.06  * * *
 * * * * Release 3.0 * * * *
 *		12/31/98	pjr		rename file AsProtos.h (was RpProtos.h)
 *		12/10/98	pjr		Entry point names change from 
 *								RomPagerXxxx -> AllegroXxxx
 * * * * Release 2.2 * * * *
 * * * * Release 2.1 * * * *
 *		04/29/98	bva		add RomPagerConnectionTask, RomPagerTimerTask
 * * * * Release 2.0 * * * *
 *		07/20/97	bva		RomPagerMainTask now returns number of active TCP tasks
 * * * * Release 1.6 * * * *
 *		04/26/97	bva		RomPagerMainTask now returns number of active HTTP tasks
 *		01/26/97	rhb		eliminate "Boolean" and RomPagerDeInit return
 * * * * Release 1.5 * * * *
 *		09/24/96	rhb		add data pointer
 * * * * Release 1.4 * * * *
 * * * * Release 1.0 * * * *
 *		12/07/95	bva	created
 *
 *	To Do:
 */


#ifndef	_RPPROTOS_
#define	_RPPROTOS_

#ifdef __cplusplus
extern "C" {
#endif

/*
	RomPager family server protocols.
*/

typedef enum {
	eAsHttpServer,
	eAsIppServer,
	eAsTlsServer,
	eAsTelnetServer,
    eAsSshServer
} asProtocolType;


/*
	Main entry points to the Allegro task.
*/

extern void * 	AllegroTaskInit(int sessionID);
extern void 	AllegroTaskDeInit(void *theTaskDataPtr);
extern int	 	AllegroMainTask(void *theTaskDataPtr,
					int *theHttpTasks,
					int *theTcpTasks);
extern int	 	AllegroConnectionTask(void *theTaskDataPtr,
									int theConnection,
									int *theHttpActiveFlag, 
									int *theTcpActiveFlag, int acceptfd);
extern int	 	AllegroTimerTask(void *theTaskDataPtr, int fd);
extern void * 	AllegroTaskInitMemory(int sessionID);
extern int		AllegroTaskInitPort(void *theTaskDataPtr,
									int thePort, 
									asProtocolType theProtocol);
extern int	 	AllegroTaskInitStart(void *theTaskDataPtr, int sessionID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _RPPROTOS_ */
