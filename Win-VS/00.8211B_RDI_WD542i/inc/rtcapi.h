/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	rtcapi.h

Abstract:

   	The application interface of RTC.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __RTC_API_H__
#define __RTC_API_H__


/* Structure */



/* Count Type */
typedef struct _RTC_COUNT {
	u16 	day;
	u8	hour;
	u8	min;
	u8	sec;
} RTC_COUNT;

typedef struct _RTC_TIME_ZONE {
	u8	operator;	/* 0:+, 1:- */
	u8	hour;	/* 0 - 23 */
	u8	min;	/* 0 /30 */
} RTC_TIME_ZONE;

/* Constant */
#define RTC_IRANIAN_BASE_YEAR           1380

/* Variable */

/* Function prototype */

//extern s32 rtcInit(void);
extern void rtcIntHandler(void);
//extern void rtcTest(void);
extern void rtcInit(void);
extern signed long rtcGetCount(RTC_COUNT*);
extern signed long  rtcUpdateCurr(RTC_DATE_TIME*, RTC_DATE_TIME*, RTC_COUNT*);
extern void RTC_Set_Time(RTC_DATE_TIME*);
extern void RTC_Get_Time(RTC_DATE_TIME*);
extern u8 RTC_Get_Week(RTC_DATE_TIME*);
extern void RTC_Get_GMT_Time(RTC_DATE_TIME*);
extern u8 RTC_Set_TimeZone(RTC_TIME_ZONE *);
extern void RTC_Get_Timezone(RTC_TIME_ZONE *);
extern void RTCTime_Local_To_GMT(RTC_DATE_TIME *, RTC_DATE_TIME *);
extern void RTCTime_Gmt_To_Local(RTC_DATE_TIME *, RTC_DATE_TIME *);
extern void  RTC_Second_To_Time(u32 Sec, RTC_DATE_TIME *time);
extern void RTC_HT1381_W_Byte(u8 Addr, u8 Data);
extern u8   RTC_HT1381_R_Byte(u8 Addr);
extern void RTC_HT1381_W_Burst(u8 *pData);
extern void RTC_HT1381_R_Burst(u8 *pData);
extern void Set_HT1381_RTC(RTC_DATE_TIME* date);
extern BOOLEAN Get_HT1381_RTC(RTC_DATE_TIME *rtc_time);
extern void RTC_HT1381_Init(void);

extern void Set_BQ32000_RTC(RTC_DATE_TIME* date);
extern BOOLEAN Get_BQ32000_RTC(RTC_DATE_TIME *rtc_time);
extern void RTC_BQ32000_Init(void);
extern void rtcGetTmpCount(RTC_COUNT* pCount);
extern void rtcSetTmpCount(RTC_COUNT* pCount);
extern void RTC_Set_Time_With_TmpCnt(RTC_DATE_TIME *cur_time, RTC_COUNT* Count);
extern void RTC_Get_TimeZone(RTC_TIME_ZONE *zone);
extern u8 RTC_Set_TimeZone(RTC_TIME_ZONE *zone);
extern u8 rtcGetDayNum(u16 year, u8 month);
extern u8 RTC_Get_DST(void);
extern void RTC_Set_DST(u8 enable);
extern u32 RTC_Time_To_Second(RTC_DATE_TIME *Source);



extern u32 RTCseconds;
extern RTC_DATE_TIME rtcCurr;
extern RTC_DATE_TIME rtcBase;	/* 2005, Jan. 1, 00:00:00 */ 
extern RTC_COUNT rtcCount;
extern RTC_DATE_TIME SetTime;
extern RTC_TIME_ZONE rtcTimeZone;

extern RTC_DATE_TIME g_LocalTime;
#if USE_BUILD_IN_RTC //use build-in RTC
extern void rtcUpdateRTC_ByTimer(void);
#endif
#if Use_Iranian_time
extern BOOLEAN Jleap(u32 year);
#endif

#endif

