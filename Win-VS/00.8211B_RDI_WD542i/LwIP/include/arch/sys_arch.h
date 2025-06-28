#include "sysopt.h"
#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
#ifndef	__sys_arch_h__
#define	__sys_arch_h__
#include  "ucos_ii.h"

//決定Network layer能夠Queue的數量. modified by aher 2013/12/12
#define	MBOX_SIZE					20//20		//* 指定郵箱能夠接收的消息數量
//aher 
#define	MBOX_NB						200//10		//* 指定郵箱個數，也就是鏈表長度

#define	SYS_MBOX_NULL				(void *)0
#define	SYS_SEM_NULL				(void *)0
//*-------------------------------------- 結構定義 ------------------------------------------------
/* LwIP郵箱結構 */
typedef struct stLwIPMBox{
	struct stLwIPMBox 	*pstNext;
	OS_EVENT*			hMBox;
	void 				*pvaMsgs[MBOX_SIZE];
} ST_LWIP_MBOX, *PST_LWIP_MBOX;
//*------------------------------- 一些自定義的數據類型 -------------------------------------------
typedef OS_EVENT* 		sys_sem_t;
typedef PST_LWIP_MBOX 	sys_mbox_t;		//* LwIP郵箱
typedef u8_t			sys_thread_t;	//* LwIP線程ID

#endif

#endif //#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)