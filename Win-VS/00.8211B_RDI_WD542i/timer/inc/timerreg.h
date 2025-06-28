/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	timerreg.h

Abstract:

   	The declarations of timer.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __TIMER_REG_H__
#define __TIMER_REG_H__

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
#define WDT_EN                  0x00000010
#define WDT_TEST                0x00000100
#define WDT_PULSEWIDTH          (100<<16)
#define WDT_CLOCK_DIVISOR       0x03370000
#endif
