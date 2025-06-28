/**************************************************************************
*                                                                         *
*   PROJECT     : uCOS_LWIP (uC/OS LwIP port)                             *
*                                                                         *
*   MODULE      : SYS_ARCH_OPTS.h                                         *
*                                                                         *
*   AUTHOR      : Michael Anburaj                                         *
*                 URL  : http://geocities.com/michaelanburaj/             *
*                 EMAIL: michaelanburaj@hotmail.com                       *
*                                                                         *
*   PROCESSOR   : Any                                                     *
*                                                                         *
*   TOOL-CHAIN  : Any                                                     *
*                                                                         *
*   DESCRIPTION :                                                         *
*   Module configuration for sys_arch.                                    *
*                                                                         *
**************************************************************************/


#ifndef __SYS_ARCH_OPTS_H__
#define __SYS_ARCH_OPTS_H__


/* ********************************************************************* */
/* Module configuration */

#define LWIP_TASK_MAX  5           /* Number of LwIP tasks */
#define LWIP_STK_SIZE 1024*1      /* Stack size for LwIP tasks */
#define LWIP_START_PRIO	1
#define TCPIP_THREAD_PRIO 1
#define  SYS_LIGHTWEIGHT_PROT	1
#define PPP_THREAD_PRIO 6
#define SLIPIF_THREAD_PRIO 7
#define PPP_DEBUG 0x80

/* Note: 
   Task priorities, LWIP_START_PRIO through (LWIP_START_PRIO+LWIP_MAX_TASKS-1) must be reserved
   for LwIP and must not used by other applications outside. */

#define MAX_QUEUE_ENTRIES 10              /* LwIP queue size */
#define MAX_QUEUES 20              /* Max. LwIP queues */


/* ********************************************************************* */
/* Interface macro & data definition */


/* ********************************************************************* */
/* Interface function definition */

#define MEM_ALIGNMENT 	4
#define ETH_PAD_SIZE	0 //2
#define ARP_QUEUEING	0

#endif /* __SYS_ARCH_OPTS_H__ */

