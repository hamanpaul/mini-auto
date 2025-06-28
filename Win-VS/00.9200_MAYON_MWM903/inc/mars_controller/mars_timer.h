

#ifndef __MARS_TIMER_H__
#define __MARS_TIMER_H__

#include <osapi.h>
#include "sysopt.h"
#ifndef FP_VOID
typedef void   (*FP_VOID)(void);    // Function Point
#endif

#define TIMER_ID_0    0
#define TIMER_ID_1    1
#define TIMER_ID_2    2
#define TIMER_ID_3    3
#define TIMER_ID_4    4
#define TIMER_ID_5    5
#define TIMER_ID_6    6
#define TIMER_ID_7    7
#if ( (CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
#define TIMER_ID_8    8
#define TIMER_ID_9    9
#define TIMER_ID_10    10
#define TIMER_ID_11    11
#define TIMER_ID_12    12
#define TIMER_ID_NUM  13
#else
#define TIMER_ID_NUM  8
#endif
/* TimerCount */

/* TimerCtrl */
#define TIMER_CLOCK_DIV_SHFT		0

#define TIMER_FIXED_PERIOD		0x00000000
#define TIMER_FREE_RUN			0x00000040

#define TIMER_COUNT_DISA		0x00000000
#define TIMER_COUNT_ENA			0x00000080

/* TimerPause */ 
#define TIMER_COUNT_PAUSE		0x00000001
#define TIMER_INT_ENA			0x00000002

/* TimerPwmCount */

/* TimerPwmCtrl */
#define TIMER_PWM_PULSE_WIDTH_SHFT	16

#define TIMER_PWM_CLOCK_DIV_1		0x00000000
#define TIMER_PWM_CLOCK_DIV_2		0x01000000

#define TIMER_PWM_DISA			0x00000000
#define TIMER_PWM_ENA			0x02000000

#define TIMER_PWM_TONE			0x00000000
#define TIMER_PWM_PWM			0x04000000

#define TIMER_PWM_FIXED_PERIOD		0x00000000
#define TIMER_PWM_FREE_RUN		0x08000000

#define TIMER_PWM_COUNT_DISA		0x00000000
#define TIMER_PWM_COUNT_ENA		0x10000000

/* TimerPwmPause */ 

/* TimerIntStat */
#define TIMER_INT_STAT			0x00000001

/*BJ 0828 S*/
#define TIMER_INT_STAT_0		0x00000001
#define TIMER_INT_STAT_1		0x00000002
#define TIMER_INT_STAT_2		0x00000004
#define TIMER_INT_STAT_3		0x00000008
#define TIMER_INT_STAT_4		0x00000010
/*BJ 0828 E*/
#define WDT_CNT_MASK            0xfffffff0
#define WDT_EN                  0x000000f0
#define WDT_TEST                0x00000100
#define WDT_SW_RST              0x00000200
#define WDT_PULSEWIDTH          (0x147<<16)
//#define WDT_CLOCK_DIVISOR       0x00000000  // for WDT clock source selection: 32768Hz(RTC Oscillator)
#define WDT_CLOCK_DIVISOR       0x03370000  // for WDT clock source selection: 27MHz(System Oscillator)



#ifndef TIMER_PWM_CFG
typedef struct _TIMER_PWM_CFG {
    INT32U  preScale;
    INT32U  pulseWidth;
    INT32U  clockDivisor;
    INT32U  pwmEnable;
    INT32U  pwmMode;	
    INT32U  runMode;
} TIMER_PWM_CFG;
#endif


//=================================================================
extern void marsTimerInit(void);
extern void marsTimerOpen(INT32U *pTimerId, FP_VOID pEventHdl);
extern void marsTimerClose(INT32U uiTimerId);
extern void marsTimerPwmOpen(INT32U uiTimerPwmId, FP_VOID pEventHdl);
extern void marsTimerPwmClose(INT32U uiTimerPwmId);

extern INT32U marsTimerConfig(INT32U uiTimerId, TIMER_CFG* pCfg);
extern INT32U marsTimerPwmConfig(INT32U uiTimerId, TIMER_PWM_CFG* pCfg);
extern INT32U marsTimerCountEnable(INT32U uiTimerId, INT32U enable);
extern INT32U marsTimerPwmCountEnable(INT32U uiTimerId, INT32U enable);
extern INT32U marsTimerPwmEnable(INT32U uiTimerId, INT32U enable);
extern INT32U marsTimerCountRead(INT32U uiTimerId, INT32U* pCount);
extern INT32U marsTimerCountWrite(INT32U uiTimerId, INT32U count);
extern INT32U marsTimerCountPause(INT32U uiTimerId, INT32U pause);
extern INT32U marsTimerInterruptEnable(INT32U uiTimerId, INT32U enable);

extern INT32S marsTimerWaitForInt(INT32U uiTimerId);
extern void marsTimerClrIntFlag(INT32U uiTimerId);
extern void marsTimerWaitForTickInt(void);

//=================================================================

#endif    // __MARS_TIMER_H__
