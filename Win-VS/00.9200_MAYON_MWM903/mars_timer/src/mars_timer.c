
#include "general.h"
#include "board.h"
#include "sysapi.h"

#include "timerapi.h"
#include "intapi.h"

#include <mars_int.h>
#include <mars_timer.h>
#include <mars_timerreg.h>
#include <mars_system.h>
#include <stdio.h>

extern u32 guiSysTimerId;

void marsTimerIntHandler(void);

INT8U gTimerLockStatus = 0;
OS_EVENT     *gTimerSem;
OS_FLAG_GRP  *gTimerFlagGrp;

FP_VOID gpTimerIsrTbl[TIMER_ID_NUM];

const INT32U gTimerClkEnable[TIMER_ID_NUM] = {
    SYS_CTL0_TIMER0_CKEN,
    SYS_CTL0_TIMER1_CKEN,
    SYS_CTL0_TIMER2_CKEN,
    SYS_CTL0_TIMER3_CKEN,
    SYS_CTL0_TIMER4_CKEN,
    SYS_CTL0_TIMER5_CKEN,
    SYS_CTL0_TIMER6_CKEN,
    SYS_CTL0_TIMER7_CKEN,
#if ( (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
    SYS_CTL0_TIMER8_CKEN,
    SYS_CTL0_TIMER9_CKEN,
    SYS_CTL0_TIMER10_CKEN,
    SYS_CTL0_TIMER11_CKEN,
    SYS_CTL0_TIMER12_CKEN,
 #endif   
};
const INT32U gFlagTimerInt[TIMER_ID_NUM] = {
    FLAGTIMER_INT_0,
    FLAGTIMER_INT_1,
    FLAGTIMER_INT_2,
    FLAGTIMER_INT_3,
    FLAGTIMER_INT_4,
    FLAGTIMER_INT_5,
    FLAGTIMER_INT_6,
    FLAGTIMER_INT_7,
#if ( (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
    FLAGTIMER_INT_8,
    FLAGTIMER_INT_9,
    FLAGTIMER_INT_10,
    FLAGTIMER_INT_11,
    FLAGTIMER_INT_12,
#endif    
};
const INT32U gTimerCtrl[TIMER_ID_NUM] = {
    REG_TIMER0CTRL,
    REG_TIMER1CTRL,
    REG_TIMER2CTRL,
    REG_TIMER3CTRL,
    REG_TIMER4CTRL,
    REG_TIMER5CTRL,
    REG_TIMER6CTRL,
    REG_TIMER7CTRL,
};
const INT32U gTimerCount[TIMER_ID_NUM]  = {
    REG_TIMER0COUNT,
    REG_TIMER1COUNT,
    REG_TIMER2COUNT,
    REG_TIMER3COUNT,
    REG_TIMER4COUNT,
    REG_TIMER5COUNT,
    REG_TIMER6COUNT,
    REG_TIMER7COUNT,
};
const INT32U gTimerIntEna[TIMER_ID_NUM] = {
    REG_TIMER0INTENA,
    REG_TIMER1INTENA,
    REG_TIMER2INTENA,
    REG_TIMER3INTENA,
    REG_TIMER4INTENA,
    REG_TIMER5INTENA,
    REG_TIMER6INTENA,
    REG_TIMER7INTENA,
};

void marsTimerInit(void)
{
    INT32U i;
    INT8U err;
    
    gTimerSem = OSSemCreate(5);        // timer0, 1, 5~7
    gTimerFlagGrp = OSFlagCreate(0x00000000, &err);
    
    marsIntIRQDisable(INT_IRQMASK_TIMER);
    marsIntIRQDefIsr(INT_IRQID_TIMER, marsTimerIntHandler);
    
    for(i=0; i<TIMER_ID_NUM; i++)
        gpTimerIsrTbl[i] = NULL;
}

void marsTimerOpen(INT32U *pTimerId, FP_VOID pEventHdl)
{
    INT8U err;
    INT32U i, uiTimerLockBit;
    
    // get one semaphore
    OSSemPend(gTimerSem, OS_IPC_WAIT_FOREVER, &err);
    
    if(gTimerLockStatus == 0)
        marsIntIRQEnable(INT_IRQMASK_TIMER);
        
    for(i=0; i<TIMER_ID_NUM; i++)
    {
        if((i == TIMER_ID_2)||(i == TIMER_ID_3)||(i == TIMER_ID_4))
            continue;
        uiTimerLockBit = (0x01 << i);
        if((gTimerLockStatus & uiTimerLockBit) == 0)    // this timer is not locked
        {
            gTimerLockStatus |= uiTimerLockBit;
            gpTimerIsrTbl[i] = pEventHdl;
            *pTimerId = i;
            SYSClkEnable(gTimerClkEnable[i]);
            return;
        }
    }
}

void marsTimerClose(INT32U uiTimerId)
{
    INT32U uiTimerLockBit = (0x01 << uiTimerId);

    if((uiTimerId == TIMER_ID_2)||(uiTimerId == TIMER_ID_3)||(uiTimerId == TIMER_ID_4))
        return;
    if((gTimerLockStatus & uiTimerLockBit) == 0)    // this timer is not locked
        return;
    gTimerLockStatus &= (~uiTimerLockBit);
    gpTimerIsrTbl[uiTimerId] = NULL;
    
    // release semaphore
    OSSemPost(gTimerSem);
    //SYSClkDisable(gTimerClkEnable[uiTimerId]);
    if(gTimerLockStatus == 0)
        marsIntIRQDisable(INT_IRQMASK_TIMER);
}

void marsTimerPwmOpen(INT32U uiTimerPwmId, FP_VOID pEventHdl)
{
    INT32U uiTimerLockBit = (0x01 << uiTimerPwmId);
#if ( (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
     if((uiTimerPwmId != TIMER_ID_2)&&(uiTimerPwmId != TIMER_ID_3)&&(uiTimerPwmId != TIMER_ID_4)&&(uiTimerPwmId != TIMER_ID_10))
	 return;	
#else
    if((uiTimerPwmId != TIMER_ID_2)&&(uiTimerPwmId != TIMER_ID_3)&&(uiTimerPwmId != TIMER_ID_4))
	 return;	
#endif		
        
    if(gTimerLockStatus == 0)
        marsIntIRQEnable(INT_IRQMASK_TIMER);
        
    gTimerLockStatus |= uiTimerLockBit;
    gpTimerIsrTbl[uiTimerPwmId] = pEventHdl;
    SYSClkEnable(gTimerClkEnable[uiTimerPwmId]);
}

void marsTimerPwmClose(INT32U uiTimerPwmId)
{
    INT32U uiTimerLockBit = (0x01 << uiTimerPwmId);
    
 #if ( (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
     if((uiTimerPwmId != TIMER_ID_2)&&(uiTimerPwmId != TIMER_ID_3)&&(uiTimerPwmId != TIMER_ID_4)&&(uiTimerPwmId != TIMER_ID_10))
	 return;	
#else
    if((uiTimerPwmId != TIMER_ID_2)&&(uiTimerPwmId != TIMER_ID_3)&&(uiTimerPwmId != TIMER_ID_4))
	 return;	
#endif	
    
    gTimerLockStatus &= (~uiTimerLockBit);
    gpTimerIsrTbl[uiTimerPwmId] = NULL;
    SYSClkDisable(gTimerClkEnable[uiTimerPwmId]);
    
    if(gTimerLockStatus == 0)
        marsIntIRQDisable(INT_IRQMASK_TIMER);
}


INT32U marsTimerConfig(INT32U uiTimerId, TIMER_CFG* pCfg)
{
    volatile INT32U *pTimerCtrl;
    INT32U ctrl = (pCfg->clockDivisor | pCfg->runMode);
    
    if((uiTimerId == TIMER_ID_2)||(uiTimerId == TIMER_ID_3)||(uiTimerId == TIMER_ID_4))
        return 0;
    
    pTimerCtrl = (volatile INT32U *)gTimerCtrl[uiTimerId];	
    *pTimerCtrl = ctrl;
    return 1;		
}

INT32U marsTimerPwmConfig(INT32U uiTimerId, TIMER_PWM_CFG* pCfg)
{
    volatile INT32U *pTimerCtrl;
    INT32U ctrl =( pCfg->preScale |
		   (pCfg->pulseWidth << TIMER_PWM_PULSE_WIDTH_SHFT) |
		   pCfg->clockDivisor |
		   pCfg->pwmEnable |
		   pCfg->pwmMode |	
		   pCfg->runMode);
    
    if((uiTimerId != TIMER_ID_2)&&(uiTimerId != TIMER_ID_3)&&(uiTimerId != TIMER_ID_4))
        return 0;
    
    pTimerCtrl = (volatile INT32U *)gTimerCtrl[uiTimerId];
    *pTimerCtrl = ctrl;
    return 1;		
}

INT32U marsTimerCountEnable(INT32U uiTimerId, INT32U enable)
{
    INT8U err;
    volatile INT32U *pTimerCtrl;
    
    if((uiTimerId == TIMER_ID_2)||(uiTimerId == TIMER_ID_3)||(uiTimerId == TIMER_ID_4))
        return 0;
    
    pTimerCtrl = (volatile INT32U *)gTimerCtrl[uiTimerId];
    if(enable)
    {
        // clear Timer INT-flag occurs before
        OSFlagPost(gTimerFlagGrp, gFlagTimerInt[uiTimerId], OS_FLAG_CLR, &err);
        if (uiTimerId == guiSysTimerId)
            OSFlagPost(gTimerFlagGrp, FLAGTIMER_TICK, OS_FLAG_CLR, &err);
        *pTimerCtrl |= (TIMER_COUNT_ENA);
    }
    else
        *pTimerCtrl &= (~TIMER_COUNT_ENA);	
				 
    return 1;
}

INT32U marsTimerPwmCountEnable(INT32U uiTimerId, INT32U enable)
{
    INT8U err;
    volatile INT32U *pTimerCtrl;
    
    if((uiTimerId != TIMER_ID_2)&&(uiTimerId != TIMER_ID_3)&&(uiTimerId != TIMER_ID_4))
        return 0;
    
    pTimerCtrl = (volatile INT32U *)gTimerCtrl[uiTimerId];
    if(enable)
    {
        // clear Timer INT-flag occurs before
        OSFlagPost(gTimerFlagGrp, gFlagTimerInt[uiTimerId], OS_FLAG_CLR, &err);
        *pTimerCtrl |= (TIMER_PWM_COUNT_ENA);
    }
    else
        *pTimerCtrl &= (~TIMER_PWM_COUNT_ENA);	
				 
    return 1;
}

INT32U marsTimerPwmEnable(INT32U uiTimerId, INT32U enable)
{
    INT8U err;
    volatile INT32U *pTimerCtrl;
    
    if((uiTimerId != TIMER_ID_2)&&(uiTimerId != TIMER_ID_3)&&(uiTimerId != TIMER_ID_4))
        return 0;
    
    pTimerCtrl = (volatile INT32U *)gTimerCtrl[uiTimerId];
    if(enable)
    {
        // clear Timer INT-flag occurs before
        OSFlagPost(gTimerFlagGrp, gFlagTimerInt[uiTimerId], OS_FLAG_CLR, &err);
        *pTimerCtrl |= (TIMER_PWM_ENA);
    }
    else
        *pTimerCtrl &= (~TIMER_PWM_ENA);	
				 
    return 1;
}


INT32U marsTimerCountRead(INT32U uiTimerId, INT32U* pCount)
{
    volatile INT32U *pTimerCount;
    
    if(uiTimerId >= TIMER_ID_NUM)
        return 0;
        
    pTimerCount = (volatile INT32U *)gTimerCount[uiTimerId];
    *pCount  = *pTimerCount;
    return 1;		
}

INT32U marsTimerCountWrite(INT32U uiTimerId, INT32U count)
{
    volatile INT32U *pTimerCount;
    
    if(uiTimerId >= TIMER_ID_NUM)
        return 0;
        
    pTimerCount = (volatile INT32U *)gTimerCount[uiTimerId];
    *pTimerCount = count;
    return 1;		
}

INT32U marsTimerCountPause(INT32U uiTimerId, INT32U pause)
{
    volatile INT32U *pTimerIntEna;
    
    if(uiTimerId >= TIMER_ID_NUM)
        return 0;
        
    pTimerIntEna = (volatile INT32U *)gTimerIntEna[uiTimerId];
    if(pause)
        *pTimerIntEna |= (TIMER_COUNT_PAUSE);
    else
        *pTimerIntEna &= (~TIMER_COUNT_PAUSE);

    return 1;		
}

INT32U marsTimerInterruptEnable(INT32U uiTimerId, INT32U enable)
{
    volatile INT32U *pTimerIntEna;
    
    if(uiTimerId >= TIMER_ID_NUM)
        return 0;
        
    pTimerIntEna = (volatile INT32U *)gTimerIntEna[uiTimerId];
    if(enable)
        *pTimerIntEna |= (TIMER_INT_ENA);
    else
        *pTimerIntEna &= (~TIMER_INT_ENA);

    return 1;		
}

void marsTimerIntHandler(void)
{
    INT8U err;
    INT32U i, CurTimerIntStat, uiTimerChkBit=0x01;
#if 0 //CPU_PERFORMANCE_TEST //Paul add for fixed CPU_PERFORMANCE_TEST hand-on issue
	static INT32U t1=0;
#endif
    volatile INT32U *pTimerIntSta;
    
    pTimerIntSta = (volatile INT32U *)REG_TIMER0123INTSTA;
    CurTimerIntStat = *pTimerIntSta;

    for(i=0; i<TIMER_ID_NUM; i++)
    {
    	if(CurTimerIntStat & uiTimerChkBit)
        {
#if 0 //CPU_PERFORMANCE_TEST //Paul add for fixed CPU_PERFORMANCE_TEST hand-on issue
	        if(0 == i)
    	    {
	    		t1++;
				if((t1 % (50/ TIMER0_INTERVAL)) == 0)
				{
					OSTimeTick();   // used for OS tick (50ms)
//					DC_MAILBOX_5=1;//For dual CPU use
//					DC_C2_Ctrl=1; //Trigger for 2 CPU time tick
				}
			}
#else
    	    OSFlagPost(gTimerFlagGrp, gFlagTimerInt[i], OS_FLAG_SET, &err);
            if (i == guiSysTimerId)
    	        OSFlagPost(gTimerFlagGrp, FLAGTIMER_TICK, OS_FLAG_SET, &err);
#endif
    	    if(gpTimerIsrTbl[i] != NULL)
                gpTimerIsrTbl[i]();
        }
        uiTimerChkBit <<= 1;
    }
}

void marsTimerWaitForTickInt(void)
{
    INT8U err;

    OSFlagPend(gTimerFlagGrp, FLAGTIMER_TICK, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
}

INT32S marsTimerWaitForInt(INT32U uiTimerId)
{
    INT8U err;
    if(uiTimerId >= TIMER_ID_NUM)
    {
        DEBUG_TIMER("Error TimerId %d\n",uiTimerId);
        return 0;
    }
        
    OSFlagPend(gTimerFlagGrp, gFlagTimerInt[uiTimerId], OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
    if (err != OS_NO_ERR)
        DEBUG_TIMER("Error Pend %d\n",err);
    return 1;    // ok
}

void marsTimerClrIntFlag(INT32U uiTimerId)
{
    INT8U err;
    // clear Timer INT-flag occurs before
    OSFlagPost(gTimerFlagGrp, gFlagTimerInt[uiTimerId], OS_FLAG_CLR, &err);
    if (uiTimerId == guiSysTimerId)
        OSFlagPost(gTimerFlagGrp, FLAGTIMER_TICK, OS_FLAG_CLR, &err);
}

