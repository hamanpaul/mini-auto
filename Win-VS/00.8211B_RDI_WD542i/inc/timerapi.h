/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	timerapi.h

Abstract:

   	The application interface of timer.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#ifndef __TIMER_API_H__
#define __TIMER_API_H__
#include "general.h"
#define TIMER_0     0
#define TIMER_1     1
#define TIMER_2     2   /*PWM*/
#define TIMER_3     3   /*PWM*/
#define TIMER_4     4   /*PWM*/
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#define TIMER_5     5
#define TIMER_6     6
#define TIMER_7     7
#endif

#ifndef TIMER_CFG
typedef struct _TIMER_CFG {
    unsigned int  clockDivisor;
    unsigned int  runMode;
} TIMER_CFG;
#endif

#define	TIMER_TIMEOUT	5

/*
Lucian:  用於動態升頻
    TIMERx_CLOCK_DIVISOR_48M: 系統主頻下設定
    TIMERx_CLOCK_DIVISOR_64M: 系統超頻下設定

*/
//-------TIMER 0------//
#define TIMER0_CLOCK_DIVISOR_48M                0x71f    // divisor=480-1
#define TIMER0_CLOCK_DIVISOR_64M                0x93f    // divisor=640-1

#define TIMER0_DIVISOR			                (((TIMER0_CLOCK_DIVISOR_48M & 0xFFFFFF00) >> 2) | (TIMER0_CLOCK_DIVISOR_48M & 0x3f)) /* 0x8000 = 32768 */
#define TIMER0_RUN_MODE			                 TIMER_FIXED_PERIOD
#define TIMER0_COUNT			                ((((SYS_CPU_CLK_FREQ / 1000) * TIMER0_INTERVAL) / (TIMER0_DIVISOR+1) )-1)

//-------TIMER 1------//
//Sofe-IR 讀取time-stamp 用, unit: 10 us,目前不再用Soft-IR
#if (TIMER_TEST == 1)   /*1sec*/
    #define TIMER1_CLOCK_DIVISOR_48M	         0x71f //System:32MHz,Div=480-1
    #define TIMER1_CLOCK_DIVISOR_64M             0x93f //System:48MHz,Div=640-1
#else
   #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define TIMER1_CLOCK_DIVISOR_48M	         0x43f //System:32MHz,Div=320-1
    #define TIMER1_CLOCK_DIVISOR_64M             0x71f //System:48MHz,Div=480-1        
   #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    #if( SYS_CPU_CLK_FREQ == 160000000 )
    #define TIMER1_CLOCK_DIVISOR_48M	         0x183f //System: 96MHz,Div=1600-1
    #else
    #define TIMER1_CLOCK_DIVISOR_48M             0x1c07 //System: 108MHz,Div=1800-1
    #endif
    #define TIMER1_CLOCK_DIVISOR_64M             0x1c07 //System: 108MHz,Div=1800-1
   #else
     #if( SYS_CPU_CLK_FREQ == 108000000 )
        #define TIMER1_CLOCK_DIVISOR_48M	     0x1037 //System:108MHz,Div=1080-1
     #else
        #define TIMER1_CLOCK_DIVISOR_48M	     0x0e3f //System: 96MHz,Div=960-1
	 #endif
     #define TIMER1_CLOCK_DIVISOR_64M            0x1037 //System: 108MHz,Div=1080-1
   #endif
#endif
#define TIMER1_DIVISOR			         (((TIMER1_CLOCK_DIVISOR_48M & 0xFFFFFF00) >> 2) | (TIMER1_CLOCK_DIVISOR_48M & 0x3f))  /* 479 */
#define TIMER1_RUN_MODE			         TIMER_FIXED_PERIOD
#define TIMER1_COUNT			         ((((SYS_CPU_CLK_FREQ / 1000) * TIMER1_INTERVAL) / (TIMER1_DIVISOR+1) )-1)


//-------TIMER 2(PWM)------//
#if FINE_TIME_STAMP // use IIS time + Timer3 to calculate frame time
#define TIMER2_PRESCALE                  0x7e
#define TIMER2_PULSE_WIDTH               16
#define TIMER2_CLOCK_DIVISOR             TIMER_PWM_CLOCK_DIV_1
#define TIMER2_PWM_ENABLE                TIMER_PWM_DISA
#define TIMER2_PWM_MODE                  TIMER_PWM_TONE
#define TIMER2_RUN_MODE                  TIMER_FIXED_PERIOD
#define TIMER2_COUNT                     0xFA000
#else
#define TIMER2_PRESCALE                  0x0
#define TIMER2_PULSE_WIDTH               16
#define TIMER2_CLOCK_DIVISOR             TIMER_PWM_CLOCK_DIV_1
#define TIMER2_PWM_ENABLE                TIMER_PWM_ENA
#define TIMER2_PWM_MODE                  TIMER_PWM_PWM
#define TIMER2_RUN_MODE                  TIMER_FIXED_PERIOD
#define TIMER2_COUNT                     0x000000
#endif

//-------TIMER 3(PWM)------//
#if MOTOR_EN
#if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
//Lucian: used for IDU Time delay.
#define TIMER3_PRESCALE                  239
#define TIMER3_PULSE_WIDTH               16
#define TIMER3_CLOCK_DIVISOR             TIMER_PWM_CLOCK_DIV_2 //x2
#define TIMER3_PWM_ENABLE                TIMER_PWM_ENA
#define TIMER3_PWM_MODE                  TIMER_PWM_TONE
#define TIMER3_RUN_MODE                  TIMER_FIXED_PERIOD
#define TIMER3_COUNT                     1200
#else
#define TIMER3_PRESCALE                  239
#define TIMER3_PULSE_WIDTH               16
#define TIMER3_CLOCK_DIVISOR             TIMER_PWM_CLOCK_DIV_2 //x2
#define TIMER3_PWM_ENABLE                TIMER_PWM_ENA
#define TIMER3_PWM_MODE                  TIMER_PWM_TONE
#define TIMER3_RUN_MODE                  TIMER_FIXED_PERIOD
#define TIMER3_COUNT                     226
#endif
#else
//Lucian: used for IDU Time delay.
#define TIMER3_PRESCALE                  239
#define TIMER3_PULSE_WIDTH               16
#define TIMER3_CLOCK_DIVISOR             TIMER_PWM_CLOCK_DIV_2 //x2
#define TIMER3_PWM_ENABLE                TIMER_PWM_ENA
#define TIMER3_PWM_MODE                  TIMER_PWM_TONE
#define TIMER3_RUN_MODE                  TIMER_FIXED_PERIOD
#define TIMER3_COUNT                     1200
#endif

//-------TIMER 4(PWM)------//
#if (HW_BOARD_OPTION == SUNWAY_SDV)
//sunway used for time delay of TV display
#define TIMER4_PRESCALE                  239
#define TIMER4_PULSE_WIDTH               16
#define TIMER4_CLOCK_DIVISOR             TIMER_PWM_CLOCK_DIV_2
#define TIMER4_PWM_ENABLE                TIMER_PWM_ENA
#define TIMER4_PWM_MODE                  TIMER_PWM_TONE
#define TIMER4_RUN_MODE                  TIMER_FIXED_PERIOD
#define TIMER4_COUNT                     18
#elif ((HW_BOARD_OPTION == SKYBEST_DVRBOX)||(HW_BOARD_OPTION==SUPER_POWER)||(HW_BOARD_OPTION==VEISE_CARDVR))
#define TIMER4_PRESCALE                  239
#define TIMER4_PULSE_WIDTH               16
#define TIMER4_CLOCK_DIVISOR             TIMER_PWM_CLOCK_DIV_2
#define TIMER4_PWM_ENABLE                TIMER_PWM_DISA
#define TIMER4_PWM_MODE                  TIMER_PWM_TONE
#define TIMER4_RUN_MODE                  TIMER_FIXED_PERIOD
#define TIMER4_COUNT                     18
#else
#define TIMER4_PRESCALE                  239
#define TIMER4_PULSE_WIDTH               128
#define TIMER4_CLOCK_DIVISOR             TIMER_PWM_CLOCK_DIV_2
#define TIMER4_PWM_ENABLE                TIMER_PWM_DISA
#define TIMER4_PWM_MODE                  TIMER_PWM_TONE
#define TIMER4_RUN_MODE                  TIMER_FIXED_PERIOD
#define TIMER4_COUNT                     18
#endif

#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
//-------TIMER 5------//
/*2sec*/
#if (TIMER_TEST == 1)
#define TIMER5_CLOCK_DIVISOR_64M          0x71f     //System:32MHz,Div=480-1
#define TIMER5_CLOCK_DIVISOR_48M	      0x93f     //System:48MHz,Div=640-1
#else
   #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
      #define TIMER5_CLOCK_DIVISOR_48M	         0x313f //System:32MHz,Div=3200-1
      #define TIMER5_CLOCK_DIVISOR_64M          0x4a3f //System:48MHz,Div=4800-1   
   #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      #if( SYS_CPU_CLK_FREQ == 160000000 )
         #define TIMER5_CLOCK_DIVISOR_48M	         0xf93f //System: 96MHz,Div=16000-1
      #else
         #define TIMER5_CLOCK_DIVISOR_48M          0x1190f //System: 108MHz,Div=18000-1
      #endif
      #define TIMER5_CLOCK_DIVISOR_64M          0x1190f //System: 108MHz,Div=18000-1
   #else
     #if( SYS_CPU_CLK_FREQ == 108000000 )
        #define TIMER5_CLOCK_DIVISOR_48M	         0xa82f //System:108MHz,Div=10800-1
     #else
        #define TIMER5_CLOCK_DIVISOR_48M	         0x953f //System: 96MHz,Div=9600-1
	 #endif
     #define TIMER5_CLOCK_DIVISOR_64M          0xa82f //System: 108MHz,Div=10800-1
   #endif
#endif
#define TIMER5_DIVISOR			         (((TIMER5_CLOCK_DIVISOR_48M & 0xFFFFFF00) >> 2) | (TIMER5_CLOCK_DIVISOR_48M & 0x3f)) /* 479 */
#define TIMER5_RUN_MODE			         TIMER_FIXED_PERIOD
#define TIMER5_COUNT			         ((((SYS_CPU_CLK_FREQ / 1000) * TIMER5_INTERVAL) / (TIMER5_DIVISOR+1) )-1)

//-------TIMER 6------//
/*3sec*/
#define TIMER6_CLOCK_DIVISOR_64M          0x71f     //System:32MHz,Div=480-1
#define TIMER6_CLOCK_DIVISOR_48M	      0x93f     //System:48MHz,Div=640-1
#define TIMER6_DIVISOR			         (((TIMER6_CLOCK_DIVISOR_48M & 0xFFFFFF00) >> 2) | (TIMER6_CLOCK_DIVISOR_48M & 0x3f)) /* 479 */
#define TIMER6_RUN_MODE			         TIMER_FIXED_PERIOD
#define TIMER6_COUNT			         ((((SYS_CPU_CLK_FREQ / 1000) * TIMER6_INTERVAL) / (TIMER6_DIVISOR+1) )-1)

//-------TIMER 7------//
#if (TIMER_TEST == 1)
/*4sec*/
#define TIMER7_CLOCK_DIVISOR_64M             0x71f     //System:32MHz,Div=480-1
#define TIMER7_CLOCK_DIVISOR_48M	         0x93f     //System:48MHz,Div=640-1
#else
//Used for RF, Unit: 100 us.
   #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
   #define TIMER7_CLOCK_DIVISOR_48M	         0x313f //System:32MHz,Div=3200-1
   #define TIMER7_CLOCK_DIVISOR_64M          0x4a3f //System:48MHz,Div=4800-1   
   #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
      #if( SYS_CPU_CLK_FREQ == 160000000 )
         #define TIMER7_CLOCK_DIVISOR_48M	         0xf93f //System: 96MHz,Div=16000-1
      #else   
         #define TIMER7_CLOCK_DIVISOR_48M          0x1190f //System: 108MHz,Div=18000-1
      #endif
      #define TIMER7_CLOCK_DIVISOR_64M          0x1190f //System: 108MHz,Div=18000-1
   #else
     #if( SYS_CPU_CLK_FREQ == 108000000 )
        #define TIMER7_CLOCK_DIVISOR_48M	         0xa82f //System:108MHz,Div=10800-1
     #else
        #define TIMER7_CLOCK_DIVISOR_48M	         0x953f //System: 96MHz,Div=9600-1
	 #endif
     #define TIMER7_CLOCK_DIVISOR_64M          0xa82f //System: 108MHz,Div=10800-1
   #endif
#endif
#define TIMER7_DIVISOR			         (((TIMER7_CLOCK_DIVISOR_48M & 0xFFFFFF00) >> 2) | (TIMER7_CLOCK_DIVISOR_48M & 0x3f)) /* 479 */
#define TIMER7_RUN_MODE			          TIMER_FIXED_PERIOD

//#define TIMER7_COUNT			         ((((SYS_CPU_CLK_FREQ / 1000) * TIMER7_INTERVAL) / (TIMER7_DIVISOR+1) )-1)
#define TIMER7_COUNT			         (( (SYS_CPU_CLK_FREQ / 1000) / (TIMER7_DIVISOR+1) * TIMER7_INTERVAL)-1)
#endif


//=======================================================//
extern TIMER_CFG timerCfg_48M[];
extern TIMER_CFG timerCfg_64M[];
extern u32 MotionlessSecond;

extern signed long timerInit(void);
extern void timerIntHandler(void);
extern void timerTest(void);
extern void WDT_Reset_Count(void);
extern signed long timerInterruptEnable(unsigned char, unsigned char);
extern signed long timerCountRead(unsigned char number, unsigned long* pCount);
extern void timer2Setting(void);
extern void timer3Setting(void);
extern void timer4Setting(void);

extern signed long timerPwmCountEnable(unsigned char number, unsigned char enable);
extern unsigned long TimerGetTimerCounter(unsigned char timer);
extern s32 timerConfig(u8 number, TIMER_CFG* pCfg);
extern s32 timerCountEnable(u8 number, u8 enable);
extern s32 timerCountPause(u8 number, u8 pause);

#if (SUPPORT_TOUCH_KEY)
extern void TouchKeyPolling(void);
#endif

extern void Beep_function(u8 beep_count,u8 beep_clock_div,u16 beep_on_time,u16 beep_off_time,u16 beep_delay,u8 force_start_flag);
extern u8 gBeep_pause_width;
#define PWM_ALARM_READY		1

#define PWM_BEEP_FORCE_START_FLAG		0x1
#define PWM_BEEP_WAIT_UNTIL_FINISH_FLAG	0x2
extern u8 timerDisableMic;
//extern OS_EVENT* wdtSemEvt;
#endif
