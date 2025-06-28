

#ifndef __MARS_TIMERREG_H__
#define __MARS_TIMERREG_H__

#include "board.h"

#define REG_TIMER_BASE        TimerCtrlBase

#define REG_TIMER0COUNT       (REG_TIMER_BASE)
#define REG_TIMER0CTRL 	      (REG_TIMER_BASE + 0x0004)
#define REG_TIMER0INTENA      (REG_TIMER_BASE + 0x0008)
#define REG_TIMER0INTSTA      (REG_TIMER_BASE + 0x000c)
#define REG_TIMER1COUNT       (REG_TIMER_BASE + 0x0010)
#define REG_TIMER1CTRL        (REG_TIMER_BASE + 0x0014)
#define REG_TIMER1INTENA      (REG_TIMER_BASE + 0x0018)
#define REG_TIMER1INTSTA      (REG_TIMER_BASE + 0x001c)
#define REG_TIMER2COUNT       (REG_TIMER_BASE + 0x0020)
#define REG_TIMER2CTRL        (REG_TIMER_BASE + 0x0024)
#define REG_TIMER2INTENA      (REG_TIMER_BASE + 0x0028)
#define REG_TIMER2INTSTA      (REG_TIMER_BASE + 0x002c)
#define REG_TIMER3COUNT       (REG_TIMER_BASE + 0x0030)
#define REG_TIMER3CTRL        (REG_TIMER_BASE + 0x0034)
#define REG_TIMER3INTENA      (REG_TIMER_BASE + 0x0038)
#define REG_TIMER3INTSTA      (REG_TIMER_BASE + 0x003c)
#define REG_TIMER4COUNT       (REG_TIMER_BASE + 0x0040)
#define REG_TIMER4CTRL        (REG_TIMER_BASE + 0x0044)
#define REG_TIMER4INTENA      (REG_TIMER_BASE + 0x0048)
#define REG_TIMER4INTSTA      (REG_TIMER_BASE + 0x004c)
#define REG_TIMER0123INTSTA   (REG_TIMER_BASE + 0x0050)
#define REG_TIMER5COUNT       (REG_TIMER_BASE + 0x0060)
#define REG_TIMER5CTRL        (REG_TIMER_BASE + 0x0064)
#define REG_TIMER5INTENA      (REG_TIMER_BASE + 0x0068)
#define REG_TIMER6COUNT       (REG_TIMER_BASE + 0x0070)
#define REG_TIMER6CTRL        (REG_TIMER_BASE + 0x0074)
#define REG_TIMER6INTENA      (REG_TIMER_BASE + 0x0078)
#define REG_TIMER7COUNT       (REG_TIMER_BASE + 0x0080)
#define REG_TIMER7CTRL        (REG_TIMER_BASE + 0x0084)
#define REG_TIMER7INTENA      (REG_TIMER_BASE + 0x0088)

/* TimerCount */

/* TimerCtrl */
#define TIMER_CLOCK_DIV_SHFT            0

//#define TIMER_FIXED_PERIOD              0x00000000
//#define TIMER_FREE_RUN                  0x00000040

#define TIMER_COUNT_DISA                0x00000000
#define TIMER_COUNT_ENA                 0x00000080

/* TimerPause */ 
#define TIMER_COUNT_PAUSE               0x00000001
#define TIMER_INT_ENA                   0x00000002

/* TimerPwmCount */

/* TimerPwmCtrl */
#define TIMER_PWM_PULSE_WIDTH_SHFT      16

#define TIMER_PWM_CLOCK_DIV_1           0x00000000
#define TIMER_PWM_CLOCK_DIV_2           0x01000000

#define TIMER_PWM_DISA                  0x00000000
#define TIMER_PWM_ENA                   0x02000000

#define TIMER_PWM_TONE                  0x00000000
#define TIMER_PWM_PWM                   0x04000000

#define TIMER_PWM_FIXED_PERIOD          0x00000000
#define TIMER_PWM_FREE_RUN              0x08000000

#define TIMER_PWM_COUNT_DISA            0x00000000
#define TIMER_PWM_COUNT_ENA             0x10000000

/* TimerPwmPause */ 

/* TimerIntStat */
#define TIMER_INT_STAT_0                0x00000001
#define TIMER_INT_STAT_1                0x00000002
#define TIMER_INT_STAT_2                0x00000004
#define TIMER_INT_STAT_3                0x00000008
#define TIMER_INT_STAT_4                0x00000010
#define TIMER_INT_STAT_5                0x00000020
#define TIMER_INT_STAT_6                0x00000040
#define TIMER_INT_STAT_7                0x00000080

#define FLAGTIMER_INT_0   0x00000001
#define FLAGTIMER_INT_1   0x00000002
#define FLAGTIMER_INT_2   0x00000004
#define FLAGTIMER_INT_3   0x00000008
#define FLAGTIMER_INT_4   0x00000010
#define FLAGTIMER_INT_5   0x00000020
#define FLAGTIMER_INT_6   0x00000040
#define FLAGTIMER_INT_7   0x00000080
#define FLAGTIMER_INT_8   0x00000100
#define FLAGTIMER_INT_9   0x00000200
#define FLAGTIMER_INT_10   0x00000400
#define FLAGTIMER_INT_11   0x00000800
#define FLAGTIMER_INT_12   0x00001000

#define FLAGTIMER_TICK    0x80000000

//=================================================================

#endif    // __MARS_TIMERREG_H__
