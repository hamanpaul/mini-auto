/*

Copyright (c) 2009  Mars Technologies, Inc.

Module Name:

    gpio_project.c

Abstract:

    The routines of general purpose I/O.

Environment:

        ARM RealView Developer Suite

Revision History:

    2010/06/28  Elsa Lee  Create

*/

#include "general.h"
#include "board.h"
#include "gpio.h"
#include "gpioreg.h"
#include "gpioapi.h"
#include "sysapi.h"
#include "Usbapi.h"
#include "uiapi.h"
#include "uiKey.h"
#include "rfiuapi.h"

#if IR_PPM_SUPPORT
#include "ir_ppm.h"
#endif

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */
#define GPIO_TEST   0
 /*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */

u8  AlarmDetect=0;    // 1: Alarm trigger, 0: none  , 2: Alarm finish
u32 keydetect = 0;

#if IR_PPM_SUPPORT
IR_ISR_FP hx_ir_isr=0;
#endif


GPIO_CFG gpioCfg[GPIO_GROUP_COUNT][GPIO_PIN_COUNT] =
{
/* SW 1214 S */
    {   /* GPIO group 0 */
            /*  ena,        dir     level,      inPullUp           */
        {   GPIO_ENA,    GPIO_DIR_OUT,     GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  0 */        //VBUS_EN              	
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  1 */      //SPK_BEEP  
        {   GPIO_ENA,    GPIO_DIR_IN,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  2 */		//MENU
        {   GPIO_DISA,    GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  3 */		
        {   GPIO_DISA,    GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  4 */
        {   GPIO_DISA,   GPIO_DIR_IN,     GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  5 */
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  6 */
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  7 */        //RF2_SCLK
        {   GPIO_ENA,    GPIO_DIR_OUT,	 GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  8 */	//RF2_SDA
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  9 */        //RF2_SCS
        {   GPIO_DISA,   GPIO_DIR_OUT,     GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 10 */         
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 11 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 12 */
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 13 */
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 14 */
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 15 */
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 16 */
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 17 */
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 18 */
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 19 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 20 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 21 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 22 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 23 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 24 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 25 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 26 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 27 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 28 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 29 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 30 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 31 */
    },
    {       /* GPIO group 1 */
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  0 */     
        {   GPIO_DISA,    GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  1 */      
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  2 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  3 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  4 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  5 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  6 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  7 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  8 */
        {   GPIO_ENA,    GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_DISA, },  /*  9 */	//GPIO1-9
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 10 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 11 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 12 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 13 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 14 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 15 */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 16 */
        {   GPIO_ENA,    GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_DISA, },  /* 17 */  //TOUCH_INT for pwron seq
        {   GPIO_DISA,    GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 18 */		
        {   GPIO_ENA,    GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 19 */	//SPK_EN
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 20 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 21 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 22 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 23 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 24 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 25 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 26 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 27 */
        {   GPIO_DISA,    GPIO_DIR_IN,  GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 28 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 29 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 30 */
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 31 */        //PHY_RST
    },
    {   /* GPIO group 2 */
            /*  ena,        dir     level,      inPullUp           */
        {   GPIO_DISA,  GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  0 */		
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  1 */		
        {   GPIO_ENA,    GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  2 */		//TOUCH_RST
        {   GPIO_ENA,   GPIO_DIR_IN,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  3 */		    //RTC_INT
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  4 */		
        {   GPIO_DISA,  GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  5 */		
        {   GPIO_DISA,  GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  6 */		
        {   GPIO_DISA,  GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  7 */		
        {   GPIO_ENA,  GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  8 */		//LCD_EN
        {   GPIO_ENA,  GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  9 */		//LCD_BL
        {   GPIO_ENA,  GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 10 */		//SD_OFF
        {   GPIO_ENA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 11 */		//CHARGE_STATUS
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 12 */
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 13 */		
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 14 */		
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 15 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 16 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 17 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 18 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 19 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 20 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 21 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 22 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 23 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 24 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 25 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 26 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 27 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 28 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 29 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 30 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 31 */
    },
    {   /* GPIO group 3 */
            /*  ena,        dir     level,      inPullUp           */
        {   GPIO_ENA,  GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  0 */         //RF1_TXSW
        {   GPIO_ENA,    GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  1 */		//RF1_RXSW
        {   GPIO_ENA,    GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  2 */		//RF1_SDA
        {   GPIO_ENA,    GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  3 */		//RF2_RXSW
        {   GPIO_ENA,    GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  4 */		//RF2_TXSW
        {   GPIO_ENA,  GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  5 */         //RF1_SCK
        {   GPIO_ENA,  GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  6 */         //RF1_SCS
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  7 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  8 */
        {   GPIO_DISA,    GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  9 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 10 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 11 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 12 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 13 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 14 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 15 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 16 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 17 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 18 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 19 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 20 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 21 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 22 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 23 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 24 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 25 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 26 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 27 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 28 */
        {   GPIO_DISA,   GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 29 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 30 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 31 */
    },
};

     //GPIO Key: 6,7 10,14,20,21
GPIO_INT_CFG gpio0IntCfg[GPIO_PIN_COUNT] =
{
    /*  intEna,     intFallEdgeEna      intRiseEdgeEna  */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA    },  /*  0 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  1 */
    {   GPIO_INT_ENA,   GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA    },  /*  2 */    //Pair key
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  3 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  4 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  5 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA    },  /*  6 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA    },  /*  7 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  8 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  9 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA    },  /* 10 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 11 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 12 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 13 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA    },  /* 14 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 15 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 16 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 17 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 18 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 19 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_ENA,   GPIO_IN_INT_FALL_ENA    },  /* 20 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_ENA,   GPIO_IN_INT_FALL_ENA    },  /* 21 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_ENA,   GPIO_IN_INT_FALL_ENA    },  /* 22 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_ENA,   GPIO_IN_INT_FALL_ENA    },  /* 23 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_ENA,   GPIO_IN_INT_FALL_ENA    },  /* 24 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_ENA,   GPIO_IN_INT_FALL_ENA    },  /* 25 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_ENA,   GPIO_IN_INT_FALL_ENA    },  /* 26 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_ENA,   GPIO_IN_INT_FALL_ENA    },  /* 27 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 28 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA    },  /* 29 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_ENA,   GPIO_IN_INT_FALL_ENA    },  /* 30 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_ENA,   GPIO_IN_INT_FALL_ENA    },  /* 31 */

};

GPIO_INT_CFG gpio1IntCfg[GPIO_PIN_COUNT] =
{
    /*  intEna,         intFallEdgeEna          intRiseEdgeEna  */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  0 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  1 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  2 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  3 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  4 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  5 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  6 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  7 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  8 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  9 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 10 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 11 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 12 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 13 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 14 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 15 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 16 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 17 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 18 */
    {   GPIO_INT_DISA,   GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA    },  /* 19 */   //Eth Int
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 20 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 21 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 22 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 23 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 24 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 25 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 26 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 27 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 28 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 29 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 30 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 31 */
};

GPIO_INT_CFG gpio2IntCfg[GPIO_PIN_COUNT] =
{
    /*  intEna,         intFallEdgeEna          intRiseEdgeEna  */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  0 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  1 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  2 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  3 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  4 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  5 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  6 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  7 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  8 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  9 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 10 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 11 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 12 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 13 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 14 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 15 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 16 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 17 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 18 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA    },  /* 19 */		//Talk SW
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 20 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 21 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 22 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 23 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 24 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 25 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 26 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 27 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 28 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 29 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 30 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 31 */
};


GPIO_INT_CFG gpio3IntCfg[GPIO_PIN_COUNT] =
{
    /*  intEna,         intFallEdgeEna          intRiseEdgeEna  */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  0 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  1 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  2 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  3 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  4 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  5 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  6 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  7 */			//Menu SW
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  8 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  9 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 10 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 11 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 12 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 13 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 14 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 15 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 16 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 17 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 18 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 19 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 20 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 21 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 22 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 23 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 24 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 25 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 26 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 27 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 28 */
    {   GPIO_INT_ENA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA		},  /* 29 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 30 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 31 */
};


u8 GPIO1_LVTRG_CFG[32] =
{
    GPIO_LVTRG_NONE,    /*0*/
    GPIO_LVTRG_NONE,    /*1*/
    GPIO_LVTRG_NONE,    /*2*/
    GPIO_LVTRG_NONE,    /*3*/
    GPIO_LVTRG_NONE,    /*4*/
    GPIO_LVTRG_NONE,    /*5*/
    GPIO_LVTRG_NONE,    /*6*/
    GPIO_LVTRG_NONE,    /*7*/
    GPIO_LVTRG_NONE,    /*8*/
    GPIO_LVTRG_NONE,    /*9*/
    GPIO_LVTRG_NONE,    /*10*/
    GPIO_LVTRG_NONE,    /*11*/
    GPIO_LVTRG_NONE,    /*12*/
    GPIO_LVTRG_NONE,    /*13*/
    GPIO_LVTRG_NONE,    /*14*/
    GPIO_LVTRG_NONE,    /*15*/
    GPIO_LVTRG_NONE,    /*16*/
    GPIO_LVTRG_NONE,    /*17*/
    GPIO_LVTRG_NONE,    /*18*/
    GPIO_LVTRG_NONE,    /*19*/
    GPIO_LVTRG_NONE,    /*20*/
    GPIO_LVTRG_NONE,    /*21*/
    GPIO_LVTRG_NONE,    /*22*/
    GPIO_LVTRG_NONE,    /*23*/
    GPIO_LVTRG_NONE,    /*24*/
    GPIO_LVTRG_NONE,    /*25*/
    GPIO_LVTRG_NONE,    /*26*/
    GPIO_LVTRG_NONE,    /*27*/
    GPIO_LVTRG_NONE,    /*28*/
    GPIO_LVTRG_NONE,    /*29*/
    GPIO_LVTRG_NONE,    /*30*/
    GPIO_LVTRG_NONE,    /*31*/
};

u8 GPIO2_LVTRG_CFG[32] =
{
    GPIO_LVTRG_NONE,    /*0*/
    GPIO_LVTRG_NONE,    /*1*/
    GPIO_LVTRG_NONE,    /*2*/
    GPIO_LVTRG_NONE,    /*3*/
    GPIO_LVTRG_NONE,    /*4*/
    GPIO_LVTRG_NONE,    /*5*/
    GPIO_LVTRG_NONE,    /*6*/
    GPIO_LVTRG_NONE,    /*7*/
    GPIO_LVTRG_NONE,    /*8*/
    GPIO_LVTRG_NONE,    /*9*/
    GPIO_LVTRG_NONE,    /*10*/
    GPIO_LVTRG_NONE,    /*11*/
    GPIO_LVTRG_NONE,    /*12*/
    GPIO_LVTRG_NONE,    /*13*/
    GPIO_LVTRG_NONE,    /*14*/
    GPIO_LVTRG_NONE,    /*15*/
    GPIO_LVTRG_NONE,    /*16*/
    GPIO_LVTRG_NONE,    /*17*/
    GPIO_LVTRG_NONE,    /*18*/
    GPIO_LVTRG_NONE,    /*19*/
    GPIO_LVTRG_NONE,    /*20*/
    GPIO_LVTRG_NONE,    /*21*/
    GPIO_LVTRG_NONE,    /*22*/
    GPIO_LVTRG_NONE,    /*23*/
    GPIO_LVTRG_NONE,    /*24*/
    GPIO_LVTRG_NONE,    /*25*/
    GPIO_LVTRG_NONE,    /*26*/
    GPIO_LVTRG_NONE,    /*27*/
    GPIO_LVTRG_NONE,    /*28*/
    GPIO_LVTRG_NONE,    /*29*/
    GPIO_LVTRG_NONE,    /*30*/
    GPIO_LVTRG_NONE,    /*31*/
};

u8 GPIO3_LVTRG_CFG[32] =
{
    GPIO_LVTRG_NONE,    /*0*/
    GPIO_LVTRG_NONE,    /*1*/
    GPIO_LVTRG_NONE,    /*2*/
    GPIO_LVTRG_NONE,    /*3*/
    GPIO_LVTRG_NONE,    /*4*/
    GPIO_LVTRG_NONE,    /*5*/
    GPIO_LVTRG_NONE,    /*6*/
    GPIO_LVTRG_NONE,    /*7*/
    GPIO_LVTRG_NONE,    /*8*/
    GPIO_LVTRG_NONE,    /*9*/
    GPIO_LVTRG_NONE,    /*10*/
    GPIO_LVTRG_NONE,    /*11*/
    GPIO_LVTRG_NONE,    /*12*/
    GPIO_LVTRG_NONE,    /*13*/
    GPIO_LVTRG_NONE,    /*14*/
    GPIO_LVTRG_NONE,    /*15*/
    GPIO_LVTRG_NONE,    /*16*/
    GPIO_LVTRG_NONE,    /*17*/
    GPIO_LVTRG_NONE,    /*18*/
    GPIO_LVTRG_NONE,    /*19*/
    GPIO_LVTRG_NONE,    /*20*/
    GPIO_LVTRG_NONE,    /*21*/
    GPIO_LVTRG_NONE,    /*22*/
    GPIO_LVTRG_NONE,    /*23*/
    GPIO_LVTRG_NONE,    /*24*/
    GPIO_LVTRG_NONE,    /*25*/
    GPIO_LVTRG_NONE,    /*26*/
    GPIO_LVTRG_NONE,    /*27*/
    GPIO_LVTRG_NONE,    /*28*/
    GPIO_LVTRG_NONE,    /*29*/
    GPIO_LVTRG_NONE,    /*30*/
    GPIO_LVTRG_NONE,    /*31*/
};


/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */

/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */
extern void isuOutputAddrArrange_TV(void);
extern void ethIntHandler(void);
extern void Ethernet_Exception(void);

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */

/*

Routine Description:

    The IRQ handler of general purpose I/O.

Arguments:

    None.

Return Value:

    None.

*/
void gpioIntHandler(void)
{
    u32 stat = Gpio0IntStat;
    u32 IntEnaSet = Gpio0IntEna;
    u8 pin = 0;
    u8 key = 0xff;
    u8 level = 0;
    u32 bitset;

    /* restricted by h/w design, to solve this issue, intr stat should be checked with intr ena first */
    stat &= IntEnaSet;

    while (stat != 0)
    {
        if (stat & 0x00000001)
        {
            /* Interrupt of GPIO[pin] occurs */
            switch (pin)
            {
                case GIIO_CHECKBIT_PAIR:
                    key = UI_KEY_MENU;
                    DEBUG_GPIO("UI_KEY_MENU!!\n\r");
                    break;

                case GPIO0_ETH_INT:
                    Ethernet_Exception();
                    break;
					
                default:
                    break;
            }
        }

        stat >>= 1;
        pin++;
    }

    if (key != 0xff)
    {
        uiSentKeyToUi(key);
    }
}

void gpio_1_IntHandler(void)
{
    u32 stat = Gpio1IntStat;
    u8 pin = 0;

    while (stat != 0)
    {
        if (stat & 0x00000001)
        {
            /* Interrupt of GPIO[pin] occurs */
            switch (pin)
            {
                case GPIO1_ETH_INT:
                    Ethernet_Exception();
                    break;

                default:
                    DEBUG_GPIO("GPIO INT pin %d!!\n\r", pin);
                    break;
            }
        }

        stat >>= 1;
        pin++;
    }
}

void gpioTimerCtrLed(LED_STATUS state)
{
    static u8 led=0;        
    static u8 count;
    static u8 led_status=LED_NONE;
    
    count++;
    if(state != LED_NONE)
        led_status=state;
    //DEBUG_GPIO("*****%d %d \n",led_status, led);
    switch(led_status)
    {
        case LED_ON:
            gpioSetLevel(GPIO_GROUP_TV_LED, GPIO_BIT_TV_LED, GPIO_LEVEL_LO);
            break;

        case LED_OFF:
            gpioSetLevel(GPIO_GROUP_TV_LED, GPIO_BIT_TV_LED, GPIO_LEVEL_HI);
            break;
            
        case LED_FLASH: 
            if((count % 5) == 0)
            {
                led^=1;
                gpioSetLevel(GPIO_GROUP_TV_LED, GPIO_BIT_TV_LED, led);
            }
            
            break;
            
        default:
            break;
    }
}

