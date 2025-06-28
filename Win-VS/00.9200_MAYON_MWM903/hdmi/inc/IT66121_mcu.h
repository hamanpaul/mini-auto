///*****************************************
//  Copyright (C) 2009-2015
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <mcu.h>
//   @author Jau-Chih.Tseng@ite.com.tw
//   @date   2015/09/03
//   @fileversion: ITE_HDMITX_SAMPLE_3.21
//******************************************/
#include "general.h"
#include "board.h"

#if(HDMI_TXIC_SEL == HDMI_TX_IT66121)
#ifndef _MCU_H_
#define _MCU_H_

//#include "515xram.h"
#include <stdio.h>
#include <stdlib.h>

//#define MySon
//#define Unixtar

/***************************************/
/* DEBUG INFO define                   */
/**************************************/
//#define Build_LIB
//#define MODE_RS232



/*************************************/
/*Port Using Define                  */
/*************************************/

#define _1PORT_



/************************************/
/* Function DEfine                  */
/***********************************/


#define       _HBR_I2S_

///////////////////////////////////////////////////////////////////////////////
// Include file
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>
//#include "Reg_c51.h" //Amy
///////////////////////////////////////////////////////////////////////////////
// Type Definition
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#ifdef MySon
extern BYTE xdata  PADMOD0;//        _at_  0x0f50;
extern BYTE xdata  PORT4OE;//        _at_  0x0f57;
extern BYTE xdata  PORT40;//             _at_  0x0f58;
extern BYTE xdata  PORT41;//             _at_  0x0f59;
extern BYTE xdata  PORT42;//             _at_  0x0f5A;
extern BYTE xdata  PORT43;//             _at_  0x0f5B;
extern BYTE xdata  PORT44;//             _at_  0x0f5C;
#else
#endif
////////////////////////////////////////////////////////////////////////////////
#define FALSE 0
#define TRUE 1

#define SUCCESS 0
#define FAIL -1

#define ON 1
#define OFF 0

#define LO_ACTIVE TRUE
#define HI_ACTIVE FALSE

#ifndef NULL
#define NULL 0
#endif
///////////////////////////////////////////////////////////////////////////////
// Constant Definition
///////////////////////////////////////////////////////////////////////////////
#define TX0DEV            0
#define TX0ADR           0x98
#define TX0CECADR        0x9C

#define RXADR            0x90

#define EDID_ADR         0xA0    // alex 070321

#define DELAY_TIME        1        // unit=1 us;
#define IDLE_TIME        100        // unit=1 ms;

#define HIGH              1
#define LOW               0

#ifdef _HPDMOS_
    #define HPDON         0
    #define HPDOFF        1
#else
    #define HPDON         1
    #define HPDOFF        0
#endif

///////////////////////////////////////////////////////////////////////////////
// 8051 Definition
///////////////////////////////////////////////////////////////////////////////
/* 
#ifdef _ENCEC_
    #define CECDEV    0
#endif

    #define DevNum    1
    #define LOOPMS    20
    #define SCL_PORT        P1_0
    #define TX0_SDA_PORT    P1_1
    #define Hold_Pin        P1_5

	sbit  EDID_WP0 	= P4^0;
	sbit  EDID_WP1 	= P4^1;
	sbit  gpHPD0 		= P4^2;		// for it6623 port 1 , with MOSFET
	sbit  gpHPD1 		= P4^3;		// for it6633P and it6623 port 0 , without MOSFET
*/

#endif    // _MCU_H_


//For debug

#define HDMI_Debug               1


#if HDMI_Debug
    #define DBG_printf(fmt...)         printf(fmt)

    #define HDMITX_DEBUG_PRINTF(x)  printf x
    #define HDCP_DEBUG_PRINTF(x)  printf x
    #define EDID_DEBUG_PRINTF(x)  printf x
    #define HDMITX_DEBUG_INFO(x) printf x
#else
    #define HDMITX_DEBUG_PRINTF(x)
    #define HDCP_DEBUG_PRINTF(x)
    #define EDID_DEBUG_PRINTF(x)
    #define HDMITX_DEBUG_INFO(x)
#endif


#if HDMI_DEBUG_ENA
    #define HDMITX_DEBUG_PRINTF1(x) printf x
    #define HDCP_DEBUG_PRINTF1(x) printf x
    #define EDID_DEBUG_PRINTF1(x) printf x
#else
    #define HDMITX_DEBUG_PRINTF1(x)
    #define HDCP_DEBUG_PRINTF1(x)
    #define EDID_DEBUG_PRINTF1(x)
#endif

#if HDMI_DEBUG_ENA
    #define HDMITX_DEBUG_PRINTF2(x) printf x
    #define HDCP_DEBUG_PRINTF2(x) printf x
    #define EDID_DEBUG_PRINTF2(x) printf x
#else
    #define HDMITX_DEBUG_PRINTF2(x)
    #define HDCP_DEBUG_PRINTF2(x)
    #define EDID_DEBUG_PRINTF2(x)
#endif

#if HDMI_DEBUG_ENA
    #define HDMITX_DEBUG_PRINTF3(x) printf x
    #define HDCP_DEBUG_PRINTF3(x) printf x
    #define EDID_DEBUG_PRINTF3(x) printf x
#else
    #define HDMITX_DEBUG_PRINTF3(x)
    #define HDCP_DEBUG_PRINTF3(x)
    #define EDID_DEBUG_PRINTF3(x)
#endif

#if HDMI_DEBUG_ENA
    #define HDMITX_DEBUG_PRINTF4(x) printf x
    #define HDCP_DEBUG_PRINTF4(x) printf x
    #define EDID_DEBUG_PRINTF4(x) printf x
#else
    #define HDMITX_DEBUG_PRINTF4(x)
    #define HDCP_DEBUG_PRINTF4(x)
    #define EDID_DEBUG_PRINTF4(x)
#endif

#if HDMI_DEBUG_ENA
    #define HDMITX_DEBUG_PRINTF5(x) printf x
    #define HDCP_DEBUG_PRINTF5(x) printf x
    #define EDID_DEBUG_PRINTF5(x) printf x
#else
    #define HDMITX_DEBUG_PRINTF5(x)
    #define HDCP_DEBUG_PRINTF5(x)
    #define EDID_DEBUG_PRINTF5(x)
#endif

#if HDMI_DEBUG_ENA
    #define HDMITX_DEBUG_PRINTF6(x) printf x
    #define HDCP_DEBUG_PRINTF6(x) printf x
    #define EDID_DEBUG_PRINTF6(x) printf x
#else
    #define HDMITX_DEBUG_PRINTF6(x)
    #define HDCP_DEBUG_PRINTF6(x)
    #define EDID_DEBUG_PRINTF6(x)
#endif

#if HDMI_DEBUG_ENA
    #define HDMITX_DEBUG_PRINTF7(x) printf x
    #define HDCP_DEBUG_PRINTF7(x) printf x
    #define EDID_DEBUG_PRINTF7(x) printf x
#else
    #define HDMITX_DEBUG_PRINTF7(x)
    #define HDCP_DEBUG_PRINTF7(x)
    #define EDID_DEBUG_PRINTF7(x)
#endif


#endif
