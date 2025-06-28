#include "sysopt.h"
#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)

//#include    "INCLUDES.H"
#include	"ucos_ii.h"
#include    "task.h"
#include	"./lwip/lwipsys.h"
#include	"./lwip/def.h"
#include	"./arch/cc.h"
#include	"./arch/sys_arch.h" 
//*------------------------------------- 常量、變量、宏 -------------------------------------------
static ST_LWIP_MBOX __staLwIPMBoxs[MBOX_NB];
PST_LWIP_MBOX pstCurFreeMBox;  //保存郵箱連的首地址。

//* LwIP線程使用的堆棧
OS_STK T_LWIP_THREAD_STK[T_LWIP_THREAD_MAX_NB][T_LWIP_THREAD_STKSIZE];

//* sys_timeouts數組，用於保存timeouts鏈表的首地址
static struct sys_timeouts __staSysTimeouts[T_LWIP_THREAD_MAX_NB + 1];
static struct sys_timeouts timeouts_null;
//*================================================================================================
//*　　　　　　　　　　　　　　　　　　　　　函　數　區
//*================================================================================================
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_sem_new
//* 功能描述 : 建立並返回一個新的信號量
//* 入口參數 : <count>[in] 指定信號量的初始狀態
//* 出口參數 : 返回新的信號量
//*------------------------------------------------------------------------------------------------
sys_sem_t sys_sem_new(u8_t count)
{
	return OSSemCreate(count);
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_sem_signal
//* 功能描述 : 發送信號
//* 入口參數 : <sem>[in] sem指定要發送的信號
//* 出口參數 : 無
//*------------------------------------------------------------------------------------------------
void sys_sem_signal(sys_sem_t sem)
{
	OSSemPost(sem);
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_sem_free
//* 功能描述 : 釋放信號量
//* 入口參數 : <sem>[in] 指定要釋放的信號量
//* 出口參數 : 無
//*------------------------------------------------------------------------------------------------
void sys_sem_free(sys_sem_t sem)
{
	INT8U	__u8Err;
	
	while(NULL != OSSemDel(sem, OS_DEL_ALWAYS, &__u8Err))
		OSTimeDlyHMSM(0, 0, 0, 100);
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : __MSToOSTicks
//* 功能描述 : 將毫秒轉變成時鐘節拍
//* 入口參數 : <u16MS>[in] 節拍數
//* 出口參數 : 毫秒數
//*------------------------------------------------------------------------------------------------
static INT16U __MSToOSTicks(INT16U u16MS)
{
	INT16U	__u16DelayTicks;
	
	if(u16MS != 0)
	{
		__u16DelayTicks = (u16MS * OS_TICKS_PER_SEC)/1000;
		if(__u16DelayTicks < 1)
			__u16DelayTicks = 1;
		else if(__u16DelayTicks > 65535)
			__u16DelayTicks = 65535;
		else;
	}
	else
		__u16DelayTicks = 0;		
		
	return __u16DelayTicks;
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_arch_sem_wait
//* 功能描述 : 等待由參數sem指定的信號並阻塞線程
//* 入口參數 :     <sem>[in] sem指定要發送的信號
//*          : <timeout>[in] 指定等待的最長時間（單位為毫秒）。為0，線程會一直被阻塞直至收到指定的信號；非0，指定線
//*          :               程最長等待時間
//* 出口參數 : -                0: 在指定時間內等到指定信號
//*          : - SYS_ARCH_TIMEOUT: 在指定時間內沒有等到指定信號
//*------------------------------------------------------------------------------------------------
u32_t sys_arch_sem_wait(sys_sem_t sem, u32_t timeout)
{	
	INT8U	__u8RtnVal;	
		
	OSSemPend(sem, __MSToOSTicks(timeout), &__u8RtnVal);

	if(__u8RtnVal == OS_NO_ERR)
		return 0;
	else
		return SYS_ARCH_TIMEOUT;
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_init
//* 功能描述 : 根據sys_arch.txt文件建立，功能為初始化sys_arch層
//* 入口參數 : 無
//* 出口參數 : 無
//*------------------------------------------------------------------------------------------------
void sys_init(void)
{
	INT8U i; 
	
	//* 先把數組清0
	memset(__staLwIPMBoxs, 0, sizeof(__staLwIPMBoxs));
	
	//* 建立鏈表和郵箱
	for(i=0; i<(MBOX_NB-1); i++)
	{
		//* 把數組中的各成員鏈接在一起
		__staLwIPMBoxs[i].pstNext = &__staLwIPMBoxs[i+1];
				
		//* 建立郵箱，系統必須保證郵箱能夠順利建立，如果出現錯誤，那是程序BUG，應該在調試階段排除
		__staLwIPMBoxs[i].hMBox = OSQCreate(__staLwIPMBoxs[i].pvaMsgs, MBOX_SIZE);
	} 
	
	//* 別忘了數組中最後一個元素，它還沒有建立郵箱呢
	__staLwIPMBoxs[MBOX_NB-1].hMBox = OSQCreate(__staLwIPMBoxs[MBOX_NB-1].pvaMsgs, MBOX_SIZE);
	
	//* 保存鏈表首地址
	pstCurFreeMBox = __staLwIPMBoxs;
	
	//* 初始化sys_timeouts數組，將每個數組成員保存的鏈表地址設置為NULL
	for(i=0; i<(T_LWIP_THREAD_MAX_NB + 1); i++)
		__staSysTimeouts[i].next = NULL;
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_mbox_new
//* 功能描述 : 建立一個空的郵箱
//* 入口參數 : 無
//* 出口參數 : - != SYS_MBOX_NULL : 郵箱申請成功，返回一個指向被申請郵箱的指針
//*          : - = SYS_MBOX_NULL  : 郵箱沒有申請成功
//*------------------------------------------------------------------------------------------------
sys_mbox_t sys_mbox_new(void)
{
	PST_LWIP_MBOX	__pstMBox = SYS_MBOX_NULL;
	
#if OS_CRITICAL_METHOD == 3                     
    OS_CPU_SR  		cpu_sr = 0;
#endif	
	
	OS_ENTER_CRITICAL();
	//{
		if(pstCurFreeMBox != NULL)
		{
			__pstMBox = pstCurFreeMBox;
			pstCurFreeMBox = __pstMBox->pstNext;
		}
	//}
	OS_EXIT_CRITICAL();
	
	return __pstMBox;
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_mbox_free
//* 功能描述 : 釋放郵箱，將郵箱歸還給鏈表
//* 入口參數 : <mbox>[in] 要歸還的郵箱
//* 出口參數 : 無
//*------------------------------------------------------------------------------------------------
void sys_mbox_free(sys_mbox_t mbox)
{	
#if OS_CRITICAL_METHOD == 3                     
    OS_CPU_SR  		cpu_sr = 0;
#endif	
	
	//* 為了防止意外情況發生，再主動清空一次郵箱
	OSQFlush(mbox->hMBox);

	OS_ENTER_CRITICAL();
	//{
		mbox->pstNext = pstCurFreeMBox;
		pstCurFreeMBox = mbox;
	//}
	OS_EXIT_CRITICAL();
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_mbox_post
//* 功能描述 : 將消息投遞到指定的郵箱
//* 入口參數 : <mbox>[in] 指定要投遞的郵箱
//*          :  <msg>[in] 指定要投遞的消息
//* 出口參數 : 無
//*------------------------------------------------------------------------------------------------
void sys_mbox_post(sys_mbox_t mbox, void *msg)
{
	int Q_FULL=0;
	int i;
	
	i=0;
	while(OSQPost(mbox->hMBox, msg) == OS_Q_FULL)
	{
		if(Q_FULL==0)
		{	
			i++;
			printf("MSG:OS_Q_FULL-%d.\n",i);
			/*Free mbox*/
			if(i>10)	
			{
				sys_mbox_free(mbox);	
				Q_FULL=1;

			}	
		}
		OSTimeDlyHMSM(0, 0, 0, 100);
	}	
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_arch_mbox_fetch
//* 功能描述 : 在指定的郵箱接收消息，該函數會阻塞線程直至超時或者郵箱至少收到一條消息
//* 入口參數 :    <mbox>[in]  指定接收消息的郵箱
//*          :     <msg>[out] 結果參數，保存接收到的消息指針
//*          : <timeout>[in]  指定等待接收的最長時間，為0表明一直等待直至接收到消息，單位為毫秒
//* 出口參數 : -                0: 在指定時間內收到消息
//*          : - SYS_ARCH_TIMEOUT: 在指定時間內沒有收到消息
//*------------------------------------------------------------------------------------------------
u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout)
{
	INT8U	__u8RtnVal;
	
	if(msg != NULL)
		*msg = OSQPend(mbox->hMBox, __MSToOSTicks(timeout), &__u8RtnVal);
	else 
		OSQPend(mbox->hMBox, __MSToOSTicks(timeout), &__u8RtnVal);

	if(__u8RtnVal == OS_NO_ERR)
		return 0;
	else
		return SYS_ARCH_TIMEOUT;
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_thread_new
//* 功能描述 : 建立一個新線程
//* 入口參數 : <thread>[in] 新線程的入口地址
//*          :    <arg>[in] 傳遞給新線程的參數
//*          :   <prio>[in] 由LwIP指定的新線程優先級，這個優先級從1開始
//* 出口參數 : 返回線程優先級，注意這與prio不同。這個值實際等於T_LWIP_THREAD_START_PRIO + prio，
//*          : 如果建立不成功則返回0
//*------------------------------------------------------------------------------------------------
sys_thread_t sys_thread_new(void(*thread)(void *arg), void *arg, int prio)
{
	INT8U __u8Prio = 0;
	
	//* 如果優先級定義沒有超出系統允許的範圍
	if(prio > 0 && prio <= T_LWIP_THREAD_MAX_NB)
	{
		__u8Prio = T_LWIP_THREAD_START_PRIO + (prio - 1);
	
		if(OS_NO_ERR == OSTaskCreate(thread, arg, &T_LWIP_THREAD_STK[prio-1][T_LWIP_THREAD_STKSIZE-1], __u8Prio))
			return __u8Prio;
		else
            return 0;
	}
	else	
		return 0;
}
//*------------------------------------------------------------------------------------------------
//* 函數名稱 : sys_arch_timeouts
//* 功能描述 : 獲取當前線程使用的sys_timeouts結構的指針
//* 入口參數 : 無
//* 出口參數 : 返回一個指向當前線程使用的sys_timeouts結構的指針
//*------------------------------------------------------------------------------------------------
struct sys_timeouts *sys_arch_timeouts(void)
{
	INT8S __s8Idx;
	
 //   printf("<%d>", OSTCBCur->OSTCBPrio);
        
	timeouts_null.next = NULL;
	//* 減去起始量獲得偏移量，也就是LwIP內部的優先級定義
	__s8Idx = OSTCBCur->OSTCBPrio - T_LWIP_THREAD_START_PRIO;
	
	//* 當前線程在指定的LwIP線程優先級號範圍之內
	if(__s8Idx >= 0 && __s8Idx < T_LWIP_THREAD_MAX_NB)
		return &__staSysTimeouts[__s8Idx];
	
	//* 如果出現意外情況，當前線程並不在指定的LwIP線程優先級資源之內，則返回__staSysTimeouts數組的最後一個成員
	 //  return &__staSysTimeouts[T_LWIP_THREAD_MAX_NB];
	 return &timeouts_null;
}
#endif //#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)