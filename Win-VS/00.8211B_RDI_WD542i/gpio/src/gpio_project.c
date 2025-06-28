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
#include "Siuapi.h"
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
u8  icomm_isr_cnt = 0;

#if IR_PPM_SUPPORT
IR_ISR_FP hx_ir_isr=0;
#endif

GPIO_CFG gpioCfg[GPIO_GROUP_COUNT][GPIO_PIN_COUNT] =
{
/* SW 1214 S */
    {   /* GPIO group 0 */
            /*  ena,        dir     level,      inPullUp           */
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  0 */       /*   System Initial setting */ 
		{   GPIO_ENA,	GPIO_DIR_OUT,	GPIO_LEVEL_LO,	GPIO_IN_PULLUP_ENA, },	/*	7 */	   /*  key1 For Lullabies Control */
        //{   GPIO_DISA,   GPIO_DIR_IN,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  1 */       /*  SPI_Int For iTM1021 Wifi Module */ 
        {   GPIO_ENA,   GPIO_DIR_IN,      GPIO_LEVEL_LO,  GPIO_IN_PULLUP_DISA, },  /*  2 */      /*   Pairing For System Pairing*/   
        {   GPIO_DISA,  GPIO_DIR_OUT,     GPIO_LEVEL_HI,  GPIO_IN_PULLUP_DISA, },  /*  3 */      /*  SPI_CLK For System Booting */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,     GPIO_LEVEL_HI,  GPIO_IN_PULLUP_DISA, },  /*  4 */      /*  SPI_MOSI For System Booting */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,     GPIO_LEVEL_HI,  GPIO_IN_PULLUP_DISA, },  /*  5 */      /*  SPI_CSn For System Booting */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,     GPIO_LEVEL_HI,  GPIO_IN_PULLUP_DISA, },  /*  6 */      /*  SPI_MISO For System Booting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  7 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  8 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  9 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 10 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 11 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 12 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 13 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 14 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 15 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 16 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 17 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 18 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 19 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 20 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 21 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 22 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 23 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 24 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 25 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 26 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 27 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 28 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 29 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,     GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 30 */       /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 31 */        /*   Sensor reset For HM1340 Sensor Reset*/  
    },
    {       /* GPIO group 1 */
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  0 */       /*  I2C SCL Serial output clock*/
        {   GPIO_DISA,   GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  1 */       /* I2C SDA Serial data bus  */
			{	GPIO_DISA,	  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  2 */		 //RF SCLK
			{	GPIO_DISA,	  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  3 */		//RF_CSn  
			{	GPIO_DISA,	  GPIO_DIR_IN,	  GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  4 */
			{	GPIO_DISA,	  GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  5 */		//SPKER_MUTE
        {   GPIO_ENA,    GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  6 */     /*  SPK_EN   For Audio Power Amplifier */
		{	GPIO_ENA,	GPIO_DIR_IN,	 GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  7 */		/*	SPI_Int For iTM1021 Wifi Module */ 
     // {   GPIO_ENA,    GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  7 */       /*  key1 For Lullabies Control */
        {   GPIO_ENA,    GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  8 */       /*  key2 For Lullabies Control */
        {   GPIO_ENA,    GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  9 */       /*  Key3 For Lullabies Control */
        {   GPIO_ENA,    GPIO_DIR_IN,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_DISA, },  /* 10 */        /*  WiFi Reset For Wifi Module Reset */
        {   GPIO_DISA,    GPIO_DIR_IN,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 11 */         /*  Light Switch For Light Control */
        {   GPIO_ENA,    GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 12 */    /*  RF1_CS For A7130 Wifi Module */
        {   GPIO_ENA,    GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 13 */    /*  RF1_SCK For A7130 Wifi Module */
        {   GPIO_ENA,    GPIO_DIR_OUT,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 14 */       /*  RF1_SDA For A7130 Wifi Module */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 15 */      /*  UARTA TXD For MCU Communication */
        {   GPIO_DISA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 16 */      /*  UARTA RXD For MCU Communication */
        {   GPIO_DISA,    GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 17 */     /*  UARTB TXD For MCU Communication */
        {   GPIO_DISA,    GPIO_DIR_IN,    GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 18 */     /*  UARTB RXD For MCU Communication */
        {   GPIO_ENA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 19 */         /*   System Initial setting */ 	
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 20 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 21 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 22 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 23 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 24 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 25 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 26 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 27 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 28 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 29 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 30 */         /*   System Initial setting */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 31 */         /*   System Initial setting */ 
    },
    {   /* GPIO group 2 */
            /*  ena,        dir     level,      inPullUp           */
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  0 */		 /*  Sensor Pixel clock output */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  1 */		/*  Sensor Vertical sync output */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  2 */        /*  Sensor Horizontal sync output */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  3 */         /*  Sensor Pixel Data output Bit 7 */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  4 */        /*  Sensor Pixel Data output Bit 6 */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /*  5 */        /*  Sensor Pixel Data output Bit 5 */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  6 */        /*  Sensor Pixel Data output Bit 4 */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  7 */        /*  Sensor Pixel Data output Bit 3 */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  8 */        /*  Sensor Pixel Data output Bit 2 */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  9 */        /*  Sensor Pixel Data output Bit 1 */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 10 */        /*  Sensor Pixel Data output Bit 0 */ 
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 11 */        /*  Sensor Master clock output */ 
        {   GPIO_ENA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 12 */        /*  Wake up For iTM1021 Wifi Module */
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 13 */        /*  RF1_TXD For A7130 Wifi Module */
        {   GPIO_DISA,  GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 14 */        /*  RF1_RXD For A7130 Wifi Module */
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
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  0 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  1 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  2 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  3 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  4 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  5 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  6 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  7 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  8 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /*  9 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 10 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 11 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 12 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 13 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 14 */
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
        {   GPIO_ENA,   GPIO_DIR_OUT,   GPIO_LEVEL_HI,  GPIO_IN_PULLUP_ENA, },  /* 27 */    /*  RF1_TXSW For A7130 Wifi Module */
        {   GPIO_ENA,   GPIO_DIR_OUT,   GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 28 */    /*  IR LED */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 29 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 30 */
        {   GPIO_DISA,  GPIO_DIR_IN,    GPIO_LEVEL_LO,  GPIO_IN_PULLUP_ENA, },  /* 31 */
    },
};

     //GPIO Key: 6,7 10,14,20,21
GPIO_INT_CFG gpio0IntCfg[GPIO_PIN_COUNT] =
{
    /*  intEna,     intFallEdgeEna      intRiseEdgeEna  */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA    },  /*  0 */     
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  1 */
    {   GPIO_INT_ENA,   GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA},  /*  2 */    //Pair key
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  3 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  4 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  5 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA    },  /*  6 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA    },  /*  7 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  8 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /*  9 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA    },  /* 10 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 11 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 12 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 13 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA    },  /* 14 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 15 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 16 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 17 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 18 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 19 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,   GPIO_IN_INT_RISE_DISA    },  /* 20 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,   GPIO_IN_INT_RISE_DISA    },  /* 21 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,   GPIO_IN_INT_RISE_DISA    },  /* 22 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,   GPIO_IN_INT_RISE_DISA    },  /* 23 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,   GPIO_IN_INT_RISE_DISA    },  /* 24 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,   GPIO_IN_INT_RISE_DISA    },  /* 25 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,   GPIO_IN_INT_RISE_DISA    },  /* 26 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,   GPIO_IN_INT_RISE_DISA    },  /* 27 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 28 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA    },  /* 29 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,   GPIO_IN_INT_RISE_DISA    },  /* 30 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,   GPIO_IN_INT_RISE_DISA    },  /* 31 */

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
    {   GPIO_INT_ENA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_ENA   },  /* 10 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 11 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 12 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 13 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 14 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 15 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 16 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 17 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA   },  /* 18 */
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA    },  /* 19 */  
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
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA    },  /* 19 */  
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
    {   GPIO_INT_DISA,  GPIO_IN_INT_FALL_DISA,  GPIO_IN_INT_RISE_DISA    },  /* 19 */  
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

extern void (*gpio_spi_isr)(void);


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

                case GPIO_CHECKBIT_PAIR:
                    key = UI_KEY_RF_PAIR;
                    DEBUG_GPIO("GIIO_CHECKBIT_PAIR!!\n\r");
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
	#include "p2pserver_api.h"

    u32 stat = Gpio1IntStat;
    u8 pin = 0;
    u8 key = 0xff;
    u8 level = 0;

    #if 1
    while (stat != 0)
    {
        if (stat & 0x00000001)
        {
            /* Interrupt of GPIO[pin] occurs */
            switch (pin)
            {
            #if ICOMMWIFI_SUPPORT
				case 7:	/*SPI wifi used.*/
				   gpio_spi_isr();					
						break;	
			#if 0
				case 10:
				
					memset(JOIN_DEFAULT_SSID, 0, sizeof(JOIN_DEFAULT_SSID));
					memset(JOIN_DEFAULT_PSK, 0, sizeof(JOIN_DEFAULT_PSK));
					
					uiSetP2PPassword("000000"); //Reset P2P PW to default.
					Save_UI_Setting();
				
					DEBUG_GPIO("RESET iComm finish, Rebooting...\n");
					
					uiMenuSet_Night_Light(1);
					OSTimeDly(10);
					uiMenuSet_Night_Light(0);
					
					sysForceWDTtoReboot();
			#endif
			#endif
			#if 0
                case GPIO_BIT_LIGHT_SW:
                    gpioGetLevel(GPIO_GROUP_LIGHT_SW, GPIO_BIT_LIGHT_SW, &level);
                    if(level==0)
                    {
                        gpioGetLevel(GPIO_GROUP_LIGHT_LED, GPIO_BIT_LIGHT_LED, &level);
                        if(level==0)
                            level=1;
                        else
                            level=0;
                        gpioSetLevel(GPIO_GROUP_LIGHT_LED, GPIO_BIT_LIGHT_LED, level);
                        DEBUG_GPIO("GPIO_BIT_LIGHT_SW!!\n\r");
                    }
                    break;
			#endif

                default:
                    DEBUG_GPIO("GPIO INT pin %d!!\n\r", pin);
                    break;
            }
        }

        stat >>= 1;
        pin++;
    }
    #else

    	//printf("\x1B[96m stat=%d \x1B[0m\n",stat);
	if (stat & 0x00000001<<7)
	{
		//DEBUG_GPIO("*"); 
		//spi_host_irq_enable(FALSE);
		
		gpio_spi_isr(); 
	}


	
    #endif
}


void gpioDetectIRLED(void)
{
    u8          level=-1,data=0;
    static u8   Mode        = 0xff;

    if(Mode != level)
    {
        siuSetSensorDayNight(SIU_DAY_MODE);
    }

}
u8 gpioMusicCtr(MUSIC_STATUS opvalue)
{
    static u8 KeepMusicStatus = 0xFF;

    if (KeepMusicStatus != opvalue)
    {
        KeepMusicStatus = opvalue;
    }
    else
    {
        DEBUG_GPIO("gpioMusicCtr Keep the same MusicStatus = %d \n", opvalue);
        return 1;
    }

    DEBUG_GPIO("gpioMusicCtr opvalue = %d \n", opvalue);
    switch(opvalue)
    {
        case GPIO_MUSIC_STOP:
            gpioSetLevel(GPIO_MUSIC_GROUP0,    GPIO_MUSIC_BIT0,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT1,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT2,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_LO);
            break;
        case GPIO_PLAY_MUSIC1:
            gpioSetLevel(GPIO_MUSIC_GROUP0,    GPIO_MUSIC_BIT0,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT1,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT2,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
            break;
        case GPIO_PLAY_MUSIC2:
            gpioSetLevel(GPIO_MUSIC_GROUP0,    GPIO_MUSIC_BIT0,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT1,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT2,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
            break;
        case GPIO_PLAY_MUSIC3:
            gpioSetLevel(GPIO_MUSIC_GROUP0,    GPIO_MUSIC_BIT0,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT1,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT2,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
            break;
        case GPIO_PLAY_MUSIC4:
            gpioSetLevel(GPIO_MUSIC_GROUP0,    GPIO_MUSIC_BIT0,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT1,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT2,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
            break;
        case GPIO_PLAY_ALL:
            gpioSetLevel(GPIO_MUSIC_GROUP0,    GPIO_MUSIC_BIT0,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT1,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT2,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
            break;
        case GPIO_PLAY_PAUSE:
            gpioSetLevel(GPIO_MUSIC_GROUP0,    GPIO_MUSIC_BIT0,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT1,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT2,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_LO);
            break;
        case GPIO_PLAY_RESTART:
        case GPIO_PLAY_RESTART_ALL:
            gpioSetLevel(GPIO_MUSIC_GROUP0,    GPIO_MUSIC_BIT0,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT1,    GPIO_LEVEL_HI);
            gpioSetLevel(GPIO_MUSIC_GROUP,    GPIO_MUSIC_BIT2,    GPIO_LEVEL_LO);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
            break;
        default:
            DEBUG_GPIO("gpioMusicCtr got Wrong Command.\n");
            return 0;
    }
    iconflag[UI_MENU_SETIDX_MELODY] = opvalue;
    Save_UI_Setting();
    return 1;
}

void gpioKeyPolling(void)
{
    u8 gpioLev, err;
    static u8  PressCnt = 0, ap_mode = 0;
    static u8 j=0, isSmartLinkMode = 0;

    extern u8 WiFi_Mode;			// 1: STA Mode, 0: AP Mode.
    extern u8 iconflag[UIACTIONNUM];
    extern u8 Enter_Wifi_Connect;  // 1: Enter Wifi Connect Mode.

//    isSmartLinkMode = (strcmp(JOIN_DEFAULT_SSID, ""))?0:1;
	gpioGetLevel(1, 10, &gpioLev);	//WiFi Reset Key.
	if (gpioLev == 0)   /*press*/
	{
		PressCnt++;
		if(PressCnt > 50)
		{
			memset(JOIN_DEFAULT_SSID, 0, sizeof(JOIN_DEFAULT_SSID));
			memset(JOIN_DEFAULT_PSK, 0, sizeof(JOIN_DEFAULT_PSK));
			iconflag[UI_MENU_SETIDX_NIGHT_LIGHT] = UI_MENU_SETIDX_LIGHT_OFF;
            iconflag[UI_MENU_SETIDX_AP_SCONFIG_DETECT] = UI_BOOT_IN_NORMAL;
			uiSetP2PPassword("000000"); //Reset P2P PW to default.
			Save_UI_Setting();

			DEBUG_GPIO("RESET iComm finish, Rebooting...\n");
			uiSet_Light_Cnt(3, 5); //flash light 3 times, 5 ticks(250ms) interval.
            PressCnt=0;
			sysForceWDTtoReboot();
		}
	}
	else if(PressCnt !=0)
	{
        //enter AP mode
        if(strcmp(JOIN_DEFAULT_SSID,"") ==0)
        {
            ap_mode++;
            if(ap_mode %2)
            {
                //smartlink mode
                isSmartLinkMode = 1;
            }
            else
            {
                isSmartLinkMode = 0;
                APmode_default();
            }
        }
			PressCnt=0;
		}
	else
		PressCnt=0;

// Light control
//    DEBUG_GPIO("ap_mode=%d, WiFi_Mode=%d\n",ap_mode,WiFi_Mode);
/*Enter_Wifi_Connect
1: smartlink lock channel
2: smartlink finish
*/

    if( ap_mode == 0 )
        return;
//DEBUG_RED("ap_mode = %d, isSmartLinkMode = %d, Enter_Wifi_Connect = %d\n",
//                        ap_mode, isSmartLinkMode, Enter_Wifi_Connect);
    if((ap_mode %2 == 0) && (j++ % 20 == 0))
    {//AP mode, flash slow
        uiSet_Light_Cnt(1, 10); //flash light forever, 10 ticks(500ms) interval.
    }
    else if((isSmartLinkMode) && (Enter_Wifi_Connect == 1))
    {//Set light when Slink locked channel
        uiMenuSet_Night_Light(1);
    }
    else if((isSmartLinkMode) && (Enter_Wifi_Connect == 0) )
    {//Smartconfig mode, flash fast
        uiSet_Light_Cnt(1, 5); //flash light forever, 5 ticks(250ms) interval.
    }
    else if(Enter_Wifi_Connect == 2)
    {//Into station mode
        ap_mode = 0;
    }
}

