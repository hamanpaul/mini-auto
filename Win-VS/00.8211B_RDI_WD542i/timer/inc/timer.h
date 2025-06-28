/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	timer.h

Abstract:

   	The declarations of timer.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __TIMER_H__
#define __TIMER_H__

#include <../inc/mars_controller/mars_timer.h>



extern u32  guiSysTimerId, guiIRTimerId, guiRFTimerID, guiIR_TXTimerId;
#if(TIMER_TEST == 1)
extern u32  Timer5Id, Timer6Id, Timer7Id;
#endif
extern OS_EVENT* timerUpdateBaseEvt;
extern OS_FLAG_GRP  *gTimerIR_TXFlagGrp;

#define FLAGTIME_READY_IR      0x00000001

extern void timer_1s_IntHandler(void);
extern void timer_zeropointfivems_IntHandler(void);
extern void timer1_IntHandler(void);
extern void timer2_IntHandler(void);
extern void timer3_IntHandler(void);
extern void timer4_IntHandler(void);
extern void timer5_IntHandler(void);
extern void timer6_IntHandler(void);
extern void timer7_IntHandler(void);
extern void timer_25ms_IntHandler(void);
extern void timer_10ms_IntHandler(void);
extern void timer_50ms_IntHandler(void);
extern void timer_100ms_IntHandler(void);
extern void timer_200ms_IntHandler(void);
extern void timer_500ms_IntHandler(void);
extern void timer_KeyPolling(void);
extern s32 TimerProjectTimerInit(u8 Step);
extern s32 TimerProjectWDTResetCount(u8 Step);

#endif
