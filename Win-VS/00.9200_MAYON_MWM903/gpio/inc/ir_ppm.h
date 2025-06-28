#ifndef __IR_PPM_H__
#define __IR_PPM_H__
      #define ENABLE     1
      #define DISABLE   0
      #define FAIL          0

#define HIMAX_ISP_PLATFORM          1
#define IR_REPEATE_ENABLE              0
//#define IR_REMOTE_TYPE              5   /*1: 8 Keys, 2: 18 keys 3:21 keys, 4:eclipse 5: RDI*/
/**************************************************************************
 *  Resurace ussage notes:                                                *
 *    1. Requires 1 timer at 10us resolution (should < 200us)             *
 *    2. GPIO support both postive and negative edge intrrupt           *
 **************************************************************************/
      #define IR_TIMER_ID			1 /* irTimerID: 指定Timer-1 */
      //#define IR_REMOTE_TYPE		4 /* Lucian : 直接指定*/
      //Lucian: 設定IR要使用的GPIO0-X
      #define IR_GPIOGROUPSEL   0   //Use GPIO0




     typedef u32 UINT32;
      typedef void (*IR_ISR_FP)(void);
#endif

