/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	timer.c

Abstract:

   	The routines of timer.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

/*CY 0718*/

#include "general.h"
#include "board.h"
#include "sysapi.h"

#include "timerapi.h"
#include "timer.h"
//#include "timerreg.h"
#include "task.h"
#include "../sys/inc/sys.h"	

#include "siuapi.h"
#include "gpioapi.h"	
#include "../board/inc/intreg.h"
#include "rtcapi.h"
#include "adcapi.h"
#include "memorypool.h"
#include "uiapi.h"
#include "asfapi.h"
/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 * Global Variable
 *********************************************************************************************************
 */
#ifdef NEW_ADC_MECHANISM
u32 Tirq_cnt=0; 
#endif
#if MARSS_SUPPORT
extern OS_FLAG_GRP *gMarssFlagGrp;
#endif

/* cytsai: 0315 - total file */ 
 
TIMER_CFG timerCfg_48M[] =
{ 	/* clockDivisor,          runMode         */
	{  TIMER0_CLOCK_DIVISOR_48M,  TIMER0_RUN_MODE  },
	{  TIMER1_CLOCK_DIVISOR_48M,  TIMER1_RUN_MODE  },
	{  TIMER5_CLOCK_DIVISOR_48M,  TIMER5_RUN_MODE  },
	{  TIMER6_CLOCK_DIVISOR_48M,  TIMER6_RUN_MODE  },
	{  TIMER7_CLOCK_DIVISOR_48M,  TIMER7_RUN_MODE  },
};

TIMER_CFG timerCfg_64M[] =
{ 	/* clockDivisor,          runMode         */
	{  TIMER0_CLOCK_DIVISOR_64M,  TIMER0_RUN_MODE  },
	{  TIMER1_CLOCK_DIVISOR_64M,  TIMER1_RUN_MODE  },
	{  TIMER5_CLOCK_DIVISOR_64M,  TIMER5_RUN_MODE  },
	{  TIMER6_CLOCK_DIVISOR_64M,  TIMER6_RUN_MODE  },
	{  TIMER7_CLOCK_DIVISOR_64M,  TIMER7_RUN_MODE  },
};

TIMER_PWM_CFG timerPwmCfg[3] =
{ 	/* preScale          pulseWidth           clockDivisor           pwmEnable           pwmMode           runMode         */
	{  TIMER2_PRESCALE,  TIMER2_PULSE_WIDTH,  TIMER2_CLOCK_DIVISOR,  TIMER2_PWM_ENABLE,  TIMER2_PWM_MODE,  TIMER2_RUN_MODE  },
	{  TIMER3_PRESCALE,  TIMER3_PULSE_WIDTH,  TIMER3_CLOCK_DIVISOR,  TIMER3_PWM_ENABLE,  TIMER3_PWM_MODE,  TIMER3_RUN_MODE  },
	{  TIMER4_PRESCALE,  TIMER4_PULSE_WIDTH,  TIMER4_CLOCK_DIVISOR,  TIMER4_PWM_ENABLE,  TIMER4_PWM_MODE,  TIMER4_RUN_MODE  }
};	

OS_STK SysTimerTaskStack[SYSTIMER_TASK_STACK_SIZE];
OS_STK SysTickTaskStack[SYSTICK_TASK_STACK_SIZE];
#if MARSS_SUPPORT
OS_STK MarssTimerTaskStack[MARSS_TIMER_TASK_STACK_SIZE];
#endif
u32  guiSysTimerCnt=0;
u32  guiSysTimerId, guiIRTimerId, guiRFTimerID, guiIR_TXTimerId;
#if MARSS_SUPPORT
u32  guiMarssTimerCnt=0;
u32  guiMarssNextTick=0;
u32  guiMarssTimerId;//Use INTR to check time
u32  guiMarssCntTimerId; //Use CNT for calculate time
#endif
u32  MotionlessSecond=0;
#if (TIMER_TEST == 1)
u32  Timer5Id, Timer6Id, Timer7Id;
#endif
OS_EVENT* timerUpdateBaseEvt;	/* update base */ //Lucian: Á×§K¦P®Éupdate RTC
OS_FLAG_GRP  *gTimerIR_TXFlagGrp;
/*
 *********************************************************************************************************
 * Extern Global Variable
 *********************************************************************************************************
 */
extern u32 sys_frequency;
extern u16 Count1ms;


 /*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

#if (USB_HOST == 1)
extern int poll_hub(void);
extern int poll_hid(void);
#endif 

void WDT_off(void);
/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */


void WDT_init(void)     //4// 4 S :   (32768*16) /(16*8192) == 4 s
{  
#if(WATCH_DOG_SEL ==  WATCHDOG_INTERNAL)

    sysWDT_disable(); 
    SYS_CLK1       &= ~((uint32_t)0xFFFF << 16);
    SYS_CLK1       |= WDT_CLOCK_DIVISOR;    //Set Divisor so that WDT can run near 32768Hz: 27M/824=32767

    sysWDT_enable(); 
    // initial WDT
    WDTctrBase      = (8 | WDT_EN | WDT_PULSEWIDTH);
    //WDTctrBase     |= WDT_TEST;
    //WDTctrBase     |= WDT_SW_RST;
#else
    WDT_off();
#endif      
}

void WDT_off(void)
{  
#if(WATCH_DOG_SEL ==  WATCHDOG_INTERNAL)
//Set WDT ctl Base to default value to disable WDT
    WDTctrBase      = 0x0;
    sysWDT_disable(); 
#endif      
}

void WDT_Reset_Count(void)
{
#if(WATCH_DOG_SEL == WATCHDOG_EXTERNAL)
    static u32 Level=0;
#endif
//======================//

    if(TimerProjectWDTResetCount(1) == 1)
    {
    #if(WATCH_DOG_SEL == WATCHDOG_EXTERNAL)
        //gpioSetLevel(0, GPIO_EXT_WDT,0x00 ); // External WDT
    #endif
        return;
    }

#if(WATCH_DOG_SEL == WATCHDOG_EXTERNAL)
    Level ++;
    //gpioSetLevel(0, GPIO_EXT_WDT,Level & 0x01 ); // External WDT
#elif(WATCH_DOG_SEL == WATCHDOG_INTERNAL)
    WDTctrBase      = (8 | WDT_EN | WDT_PULSEWIDTH);
#endif

}


/*

Routine Description:

	Initialize the timer.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 timerInit(void)
{
    u8 err;
    
#if REDUCE_HCLK_TO_128M
    sys_frequency=128000000;
#elif REDUCE_HCLK_TO_144M
    sys_frequency=144000000;
#endif
    
    marsTimerOpen((INT32U *)(&guiSysTimerId), NULL);
#if (TIMER_TEST == 1)
    marsTimerOpen(&guiIRTimerId, timer1_IntHandler);
    marsTimerOpen(&Timer5Id, timer5_IntHandler);
    marsTimerOpen(&Timer6Id, timer6_IntHandler);
    marsTimerOpen(&Timer7Id, timer7_IntHandler);
#else
    marsTimerOpen((INT32U *)(&guiIRTimerId), NULL);
    marsTimerOpen((INT32U *)(&guiRFTimerID), NULL);
#if MARSS_SUPPORT
    marsTimerOpen((INT32U *)(&guiMarssTimerId), NULL);
#if A7128_TIMER_DEBUG
    gpioSetDir(0, 0 ,0);
    gpioSetLevel(0, 0, 0);
	gpioGetLevel(0, 0, &err);
    printf("[paul] Gpio0Dir = 0x%X, level = 0x%X\n",Gpio0Dir, err);
#endif
#endif
#endif    
    TimerProjectTimerInit(1);
    gTimerIR_TXFlagGrp = OSFlagCreate(0x00000000, &err);
	timerUpdateBaseEvt = OSSemCreate(1);
#if CPU_PERFORMANCE_TEST
    OSTaskCreate(SYS_TICK_TASK, SYSTICK_TASK_PARAMETER, SYSTICK_TASK_STACK, TIMER_TICK_TASK_PRIORITY);
#else    
    OSTaskCreate(SYSTIMER_TASK, SYSTIMER_TASK_PARAMETER, SYSTIMER_TASK_STACK, SYSTIMER_TASK_PRIORITY);
    OSTaskCreate(SYS_TICK_TASK, SYSTICK_TASK_PARAMETER, SYSTICK_TASK_STACK, TIMER_TICK_TASK_PRIORITY);
#endif

#if MARSS_SUPPORT
    err = OSTaskCreate(MARSS_TIMER_TASK, MARSS_TIMER_TASK_PARAMETER, MARSS_TIMER_TASK_STACK, MARSS_TIMER_TASK_PRIORITY);
	printf("[PAUL] Task crea result %d, PRIO=%d\n", err);
#endif
    return 1;	
}



/*

Routine Description:

	The test routine of timer.

Arguments:

	None.

Return Value:

	None.

*/
void timerTest(void)
{
	INT32U count;
	
	timerInit();
	
	marsTimerCountWrite(0, 0x00111111);
	marsTimerCountWrite(1, 0x00444444);
	marsTimerCountWrite(2, 0x00888888);
	marsTimerCountWrite(3, 0x00cccccc);
	marsTimerCountWrite(4, 0x00ffffff);
	marsTimerCountWrite(5, 0x00222222);
	marsTimerCountWrite(6, 0x00444444);
	marsTimerCountWrite(7, 0x00444444);
	
	marsTimerCountEnable(0, 1);
	marsTimerCountEnable(1, 1);
	marsTimerCountEnable(5, 1);
	marsTimerCountEnable(6, 1);
	marsTimerCountEnable(7, 1);
	marsTimerPwmCountEnable(2, 1);
	marsTimerPwmCountEnable(3, 1);
	marsTimerPwmCountEnable(4, 1);
	
	marsTimerCountPause(0, 1);
	marsTimerCountPause(1, 1);
	marsTimerCountPause(2, 1);
	marsTimerCountPause(3, 1);
	marsTimerCountPause(4, 1);
	marsTimerCountPause(5, 1);
	marsTimerCountPause(6, 1);
	marsTimerCountPause(7, 1);
	
	marsTimerCountRead(0, &count);
	marsTimerCountRead(1, &count);
	marsTimerCountRead(2, &count);
	marsTimerCountRead(3, &count);
	marsTimerCountRead(4, &count);
	marsTimerCountRead(5, &count);
	marsTimerCountRead(6, &count);
	marsTimerCountRead(7, &count);
	
	marsTimerCountPause(0, 0);
	marsTimerCountPause(1, 0);
	marsTimerCountPause(2, 0);
	marsTimerCountPause(3, 0);
	marsTimerCountPause(4, 0);
	marsTimerCountPause(5, 0);
	marsTimerCountPause(6, 0);
	marsTimerCountPause(7, 0);

	marsTimerCountEnable(0, 0);
	marsTimerCountEnable(1, 0);
	marsTimerCountEnable(5, 0);
	marsTimerCountEnable(6, 0);
	marsTimerCountEnable(7, 0);
	marsTimerPwmCountEnable(2, 0);
	marsTimerPwmCountEnable(3, 0);
	marsTimerPwmCountEnable(4, 0);
}

/*

Routine Description:

	The configuration of timer.

Arguments:

	number - The timer number.
	pCfg - The timer configuration.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 timerConfig(u8 number, TIMER_CFG* pCfg)
{
    return (marsTimerConfig(number, pCfg));
}

/*

Routine Description:

	The count enable of the timer.

Arguments:

	number - The timer number.
	enable - The count enable of the timer.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 timerCountEnable(u8 number, u8 enable)
{
    return (marsTimerCountEnable(number, enable));
}

/*

Routine Description:

	Pause the count of the timer.

Arguments:

	number - The timer number.
	pause - Pause/continue the count of the timer.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 timerCountPause(u8 number, u8 pause)
{
    return (marsTimerCountPause(number, pause));
}

/*

Routine Description:

	The interrupt enable of the timer.

Arguments:

	number - The timer number.
	enable - Enable/disable the interrupt of the timer.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 timerInterruptEnable(u8 number, u8 enable)
{
    return (marsTimerInterruptEnable(number, enable));
}
	
/*

Routine Description:

	The count of the timer to read.

Arguments:

	number - The timer number.
	pCount - The count of the timer to read.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 timerCountRead(u8 number, u32* pCount)
{
    return (marsTimerCountRead(number, (INT32U *)pCount));
}

/*

Routine Description:

	The count of the timer to write.

Arguments:

	number - The timer number.
	count - The count of the timer to write.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 timerCountWrite(u8 number, u32 count)
{
    return (marsTimerCountWrite(number, count));
}

/*

Routine Description:

	The configuration of timer pwm.

Arguments:

	number - The timer number.
	pCfg - The timer configuration.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 timerPwmConfig(u8 number, TIMER_PWM_CFG* pCfg)
{
    return (marsTimerPwmConfig(number, pCfg));
}
/*

Routine Description:

	The count enable of the timer pwm.

Arguments:

	number - The timer number.
	enable - The count enable of the timer.

Return Value:

	0 - Failure.
	1 - Success.

*/

s32 timerPwmCountEnable(u8 number, u8 enable)
{
    return (marsTimerPwmCountEnable(number, enable));
}



#if FINE_TIME_STAMP
void timer2Setting(void)
{
    marsTimerPwmConfig(2, &timerPwmCfg[0]);
    marsTimerCountWrite(2, TIMER2_COUNT);
    marsTimerPwmCountEnable(2, 1);
}
#endif

void timer3Setting(void)
{
    marsTimerPwmConfig(3, &timerPwmCfg[1]);
    marsTimerCountWrite(3, TIMER3_COUNT);
    marsTimerPwmCountEnable(3, 0);
    marsTimerInterruptEnable(3,1);
}

void timer4Setting(void)
{
    marsTimerPwmConfig(4, &timerPwmCfg[2]);
    marsTimerCountWrite(4, TIMER4_COUNT);
    marsTimerPwmCountEnable(4, 0);
    marsTimerInterruptEnable(4,1);
}

void SysTimerTickTask(void* pData)
{
    static u8 Cnt = 0;
    while (1)
    {
        marsTimerWaitForTickInt();
        if (Cnt)
        {
            OSTimeTick();   // used for OS tick (50ms)
        }
        Cnt ^= 1;
    }
}

#if MARSS_SUPPORT
void MarssTimerTask(void* pData)
{
	static u8 val=0;
    printf("[PAUL] A7128TimerTask created\n");
	while(1)
	{
        marsTimerWaitForInt(guiMarssTimerId);  // wait 1ms timer int
        guiMarssTimerCnt++;
		if((guiMarssTimerCnt % 200) == 0) {//BEACON_TICK 200ms
			val = 1;
			
		}
		
		if((guiMarssNextTick !=0) && ((guiMarssTimerCnt % guiMarssNextTick) == 0)) {
			guiMarssNextTick = 0;
			val = 1;
		}

		if(val)
			OSFlagPost (gMarssFlagGrp, 0x2,OS_FLAG_SET,0);//should postINT
		val = 0;
#if A7128_TIMER_DEBUG
        if(guiMarssTimerCnt %50 == 0)
        {

			val = val & 0x01;
			val = (val==1)?0:1;
//			gpioSetLevel(0, 0, val);
			printf("[paul]level = 0x%X\n",val);
			val = 0;
        }
#endif
	}
}
#endif


void SysTimerTask(void* pData)
{
    while (1)
    {
        marsTimerWaitForInt(guiSysTimerId);  // wait 50ms timer int
        guiSysTimerCnt++;
        #if(TIMER0_INTERVAL == 25)
            //===========================
            // 25 ms
            //===========================
            timer_25ms_IntHandler();
        #elif(TIMER0_INTERVAL == 10)
            //===========================
            // 10 ms
            //===========================
            timer_10ms_IntHandler();
        #endif
        
	#if (USB_HOST == 1)
		poll_hub();
		poll_hid();
	#endif        
        //===========================
        // 50 ms
        //===========================
        if((guiSysTimerCnt % (50/ TIMER0_INTERVAL)) == 0)
        {
            timer_50ms_IntHandler();
        }
        
        //===========================
        // 100ms
        //===========================
        if((guiSysTimerCnt % (100/TIMER0_INTERVAL)) == 0)
        {
            timer_100ms_IntHandler();
        }
		if((guiSysTimerCnt % (200/TIMER0_INTERVAL)) == 0)
        {
            timer_200ms_IntHandler();
        }
        
        //===========================
        // 500ms
        //===========================
        if((guiSysTimerCnt % (500/TIMER0_INTERVAL)) == 0)
        {
            timer_500ms_IntHandler();
        }

        //==============================
        // 1s
        //==============================
        if((guiSysTimerCnt % (1000/TIMER0_INTERVAL)) == 0)
        {
        #if DEADLOCK_MONITOR_ENA
            sysDeadLockMonitor();
        #endif
            sysLifeTime ++;
            timer_1s_IntHandler();  
            // It will cause DM9000 init fail. aher 20130410
            /*
			#if NIC_SUPPORT
				EMAC_link_status();
			#endif
	    */			
        }
        
    } // end of while (1)
}

u32 TimerGetTimerCounter(u8 timer)
{
    switch(timer)
    {
        case TIMER_0:
            return TIMER0_COUNT;

        case TIMER_1:
            return TIMER1_COUNT;

        case TIMER_2:
            return TIMER2_COUNT;

        case TIMER_3:
            return TIMER3_COUNT;

        case TIMER_4:
            return TIMER4_COUNT;

        case TIMER_5:
            return TIMER5_COUNT;

        case TIMER_6:
            return TIMER6_COUNT;

        case TIMER_7:
            return TIMER7_COUNT;
        default:
            return 0;                
    }
}

void Timer_IR_TX(void)
{
    u8 err; 

    marsTimerOpen((INT32U*)(&guiIR_TXTimerId), timer_zeropointfivems_IntHandler);
    DEBUG_TIMER("%d \n",guiIR_TXTimerId);
    marsTimerConfig(guiIR_TXTimerId, &timerCfg_48M[5]);
    marsTimerCountWrite(guiIR_TXTimerId, TIMER5_COUNT);
    marsTimerInterruptEnable(guiIR_TXTimerId, 1);       //Enable while using
    marsTimerCountPause(guiIR_TXTimerId, 0);
    marsTimerCountEnable(guiIR_TXTimerId, 1);
    
    OSFlagPend(gTimerIR_TXFlagGrp, FLAGTIME_READY_IR, OS_FLAG_WAIT_SET_ANY, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_TIMER("Pend 0x%08x\n",gTimerIR_TXFlagGrp->OSFlagFlags);
    marsTimerClose(guiIR_TXTimerId);
}

