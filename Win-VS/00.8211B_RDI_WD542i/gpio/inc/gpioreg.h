/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	gpioreg.h

Abstract:

   	The declarations of general purpose I/O.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __GPIO_REG_H__
#define __GPIO_REG_H__

/* GpioEna */
#define GPIO_DISA		0
#define GPIO_ENA		1

/* GpioDir */
#define GPIO_DIR_OUT		0
#define GPIO_DIR_IN		1

/* GpioLevel */
#define GPIO_LEVEL_LO		0
#define GPIO_LEVEL_HI		1

/* GpioInPullUp */
#define GPIO_IN_PULLUP_ENA	0
#define GPIO_IN_PULLUP_DISA	1

/* GpioIntEna */
#define GPIO_INT_DISA		0
#define GPIO_INT_ENA		1

/* GpioIntStat */
#define GPIO_INT_NOT_OCCUR	0
#define GPIO_INT_OCCUR		1

/* GpioInIntFallEdge */
#define GPIO_IN_INT_FALL_DISA	0
#define GPIO_IN_INT_FALL_ENA	1

/* GpioInIntRiseEdge */
#define GPIO_IN_INT_RISE_DISA	0
#define GPIO_IN_INT_RISE_ENA	1

#if ( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
#define GPIO_IR_ENA         0x00000001
#define GPIO_IR_RESET       0x00000002
#define GPIO_IR_INT_ENA     0x00000001
#endif  /*end of #if (CHIP_OPTION == CHIP_A1016A)*/
#endif  /*end of #ifndef __GPIO_REG_H__*/
