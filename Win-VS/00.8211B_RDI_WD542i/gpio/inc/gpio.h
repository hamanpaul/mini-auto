/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	gpio.h

Abstract:

   	The declarations of general purpose I/O.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __GPIO_H__
#define __GPIO_H__

#include "gpioapi.h"    

extern GPIO_CFG gpioCfg[GPIO_GROUP_COUNT][GPIO_PIN_COUNT];
extern GPIO_INT_CFG gpio0IntCfg[GPIO_PIN_COUNT];
extern GPIO_INT_CFG gpio1IntCfg[GPIO_PIN_COUNT];
#if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
extern GPIO_INT_CFG gpio2IntCfg[GPIO_PIN_COUNT];
extern GPIO_INT_CFG gpio3IntCfg[GPIO_PIN_COUNT];
#endif

#if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
extern u8 GPIO1_LVTRG_CFG[32];
extern u8 GPIO2_LVTRG_CFG[32];
extern u8 GPIO3_LVTRG_CFG[32];
#else
extern u8 GPIO1_LVTRG_CFG[16];
#endif

#endif
