/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	rtcreg.h

Abstract:

   	The declarations of RTC.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __RTC_REG_H__
#define __RTC_REG_H__

/* RtcBase */
#define RTC_BASE_SEC_SHFT	0
#define RTC_BASE_SEC_MASK	0x0000003f
#define RTC_BASE_MIN_SHFT	6
#define RTC_BASE_MIN_MASK	0x00000fc0
#define RTC_BASE_HOUR_SHFT	12
#define RTC_BASE_HOUR_MASK	0x0001f000
#define RTC_BASE_DAY_SHFT	17
#define RTC_BASE_DAY_MASK	0x003e0000
#define RTC_BASE_MONTH_SHFT	22
#define RTC_BASE_MONTH_MASK	0x03c00000
#define RTC_BASE_YEAR_SHFT	26
#define RTC_BASE_YEAR_MASK	0xfc000000

/* RtcCount */
#define RTC_COUNT_SEC_SHFT	0
#define RTC_COUNT_SEC_MASK	0x0000003f
#define RTC_COUNT_MIN_SHFT	6
#define RTC_COUNT_MIN_MASK	0x00000fc0
#define RTC_COUNT_HOUR_SHFT	12
#define RTC_COUNT_HOUR_MASK	0x0001f000
#define RTC_COUNT_DAY_SHFT	17
#define RTC_COUNT_DAY_MASK	0xfffe0000

/* RtcCountCtrl */
#define RTC_COUNT_STOP		0x00000000
#define RTC_COUNT_RESET_START	0x00000001

#define RTC_POWER_UP_SRAM	0x00000000
#define RTC_POWER_DOWN_SRAM	0x00000100

#define RTC_CLOCK_LOW		0x00000000
#define RTC_CLOCK_SYS		0x00000200

/* RtcIntCtrl */
#define RTC_INT_ENA_SEC		0x00000001
#define RTC_INT_ENA_MIN		0x00000002
#define RTC_INT_ENA_HOUR	0x00000004
#define RTC_INT_ENA_DAY		0x00000008

#define RTC_COUNT_CLOCK_SHFT	8
#define RTC_COUNT_CLOCK_MASK	0xffffff00		

/* RtcIntStat */
#define RTC_INT_STAT_SEC	0x00000001
#define RTC_INT_STAT_MIN	0x00000002
#define RTC_INT_STAT_HOUR	0x00000004
#define RTC_INT_STAT_DAY	0x00000008

#endif
