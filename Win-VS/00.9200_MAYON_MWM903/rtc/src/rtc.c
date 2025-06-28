/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    rtc.c

Abstract:

    The routines of RTC.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "board.h"
#include "rtcapi.h"
#include "rtcreg.h"
#include "task.h"
#include "sysapi.h"
#include "i2capi.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */
#define RTC_COLOCK  0x7fff
#define RTC_SEC_INT 0x0001
#define RTC_MIN_INT 0x0002
#define RTC_HOU_INT 0x0004

#if (EXT_RTC_SEL  ==  RTC_HT1381)
#define     RTC_HT1381_TIMEOUT  10  /* Peter 090408*/
#endif
#if (CHIP_OPTION == CHIP_A1016A)
    #define RTC_SET_DELAY 500
#else
    #define RTC_SET_DELAY 1000
#endif

#if RTC_INCREASE_CHECK
#define RTC_TIME_TOLERANCE 25
#endif
/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

OS_STK rtcTaskStack[RTC_TASK_STACK_SIZE];
OS_EVENT* rtcSemEvt; /* semaphore to synchronize event processing */
#if(((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 3))   /*M936*/  ||\
   ((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 5))    /*M736UJ*/||\
   ((HW_BOARD_OPTION == MR9200_RX_RDI_M906) && (PROJ_OPT == 0))      /*M906*/  ||\
   ((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 7))    /*M736UJ*/||\
   ((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 12)))  /*M736U*/
   /*GMT+09:00 Osaka, Sapporo, Tokyo, Seoul*/
    RTC_DATE_TIME rtcBase = { 17, 6, 5, 3, 0, 0, 2};    /* 2017, Jun. 5, 12:00:00 */
#elif((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 4))    /*UD780P*/
    /*GMT+10:00 Eastern Time (USA & Canada)*/
    RTC_DATE_TIME rtcBase = { 17, 6, 5, 2, 0, 0, 2};    /* 2017, Jun. 5, 12:00:00 */
#elif((HW_BOARD_OPTION == MR9200_RX_RDI_UNIDEN)                          /*UDR777 old Version*/ ||\
     ((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 0))     /*UDR777*/ ||\
     ((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 1))     /*UDR777 Demo*/ ||\
     ((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 2)))   /*UDR780*/ 
    /*GMT-05:00 Eastern Time (USA & Canada)*/
    RTC_DATE_TIME rtcBase = { 17, 6, 5, 17, 0, 0, 2};   /* 2017, Jun. 5, 12:00:00 */
#elif(((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 6))  /*M736G*/  ||\
      ((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 8))  /*M936CS*/ )   
    /*GMT+01:00 Amsterdam, Berlin*/
    RTC_DATE_TIME rtcBase = { 17, 6, 5, 11, 0, 0, 2};   /* 2017, Jun. 5, 12:00:00 */
#elif((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 10))/*M936G*/ 
    /*GMT+01:00 Amsterdam, Berlin*/
    RTC_DATE_TIME rtcBase = { 17, 10, 31, 11, 0, 0, 2}; /* 2017, Oct. 31, 12:00:00 */
#elif((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) && (PROJ_OPT == 9))      /*M736SA*/ 
   /*GMT+09:00 Osaka, Sapporo, Tokyo, Seoul*/
    RTC_DATE_TIME rtcBase = { 17, 10, 20, 3, 0, 0, 3};  /* 2017, Oct. 20, 12:00:00 */
#elif (UI_VERSION == UI_VERSION_TRANWO)
    RTC_DATE_TIME rtcBase = { 17, 1, 1, 0, 0, 0, 0};    /* 2017, Jan. 1, 00:00:00 */
#else   
    RTC_DATE_TIME rtcBase = { 17, 6, 5, 12, 0, 0, 2};   /* 2017, Jun. 5, 12:00:00 */
#endif
RTC_DATE_TIME rtcCurr;
RTC_COUNT rtcCount;
RTC_COUNT rtcTmpCount;
RTC_COUNT rtcSetCount;
RTC_TIME_ZONE rtcTimeZone = {0,0,0};
u8  rtcDstEnable = RTC_DST_OFFSET_OFF;
u16 rtcTimeZoneSec = 0;
u32 RTCseconds=0;
OS_CPU_SR  cpu_sr;
#if Use_Iranian_time
/*Iranian */
static const u32 rtcDayMonth[13] = {   0, 31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
#else
                    /* Dummy. Jan. Feb. Mar. Apr. May. Jun. Jul. Aug. Sep. Oct. Nov. Dec. */
static const u32 rtcDayMonth[13] = {   0,  31,  28,  31,  30,  31,  30,  31,  31,  30,  31,  30,  31  };
#endif
#if 0
// no one used it, if you need, plz enable it, 20160811
static const u32 rtcSecYear[64] = {  0        ,  31622400,   63158400,   94694400,   126230400,  157852800,  189388800,  220924800,   /*2000~2007*/
                                     252460800,  284083200,  315619200,  347155200,  378691200,  410313600,  441849600,  473385600,   /*2008~2015*/
                                     504921600,  536544000,  568080000,  599616000,  631152000,  662774400,  694310400,  725846400,   /*2016~2023*/
                                     757382400,  789004800,  820540800,  852076800,  883612800,  915235200,  946771200,  978307200,   /*2024~2031*/
                                     1009843200, 1041465600, 1073001600, 1104537600, 1136073600, 1167696000, 1199232000, 1230768000,   /*2032~2039*/
                                     1262304000, 1293926400, 1325462400, 1356998400, 1388534400, 1420156800, 1451692800, 1483228800,  /*2040~2047*/
                                     1514764800, 1546387200, 1577923200, 1609459200, 1640995200, 1672617600, 1704153600, 1735689600,  /*2048~2055*/
                                     1767225600, 1798848000, 1830384000, 1861920000, 1893456000, 1925078400, 1956614400, 1988150400}; /*2056~2063*/

static const u32 rtcSecMonth[12] = { 0       ,  2678400,  5097600,  7862400,  10454400,  13132800,  /*1~6*/
                                     15724800,  18403200, 21081600, 23673600, 26352000,  28944000};  /*7~12*/
#endif
#if (EXT_RTC_SEL  ==  RTC_HT1381)
OS_EVENT*   rtcHT1381SemEvt;        /* semaphore to synchronize event processing */
#endif

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 rtcSetBase(RTC_DATE_TIME*);
s32 rtcGetBase(RTC_DATE_TIME*);
s32 rtcGetCount(RTC_COUNT*);
void rtcClearCount(void);
u32 RTC_Time_To_Second(RTC_DATE_TIME *Source);

#if RTC_INCREASE_CHECK //Add by Paul for RTC increase check, 2017.06.22
BOOLEAN rtcIsCorrectIncrease(RTC_DATE_TIME *cur_time, RTC_INCREASE_TYPE type);
#endif

#if (USE_BUILD_IN_RTC == RTC_USE_EXT_RTC && EXT_RTC_SEL  ==  RTC_HT1381)
void gpioSetDir(u8 ucGroup, u8 ucPin, u8 ucDir);
#endif

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */


/*

Routine Description:

    The IRQ handler of RTC.

Arguments:

    None.

Return Value:

    None.

*/
void rtcIntEnable(void)
{
    RtcIntCtrl &= ~RTC_SEC_INT & ~RTC_MIN_INT & ~RTC_HOU_INT;
    RtcIntCtrl |= RTC_SEC_INT |RTC_MIN_INT |RTC_HOU_INT;
}

void rtcIntDisable(void)
{
    RtcIntCtrl &= ~RTC_SEC_INT & ~RTC_MIN_INT & ~RTC_HOU_INT;
}

/*

Routine Description:

    Update current Date/Time.

Arguments:

    pCurr - Current Date/Time.
    pBase - Base Date/Time.
    pCount - Count Day/Time

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 rtcUpdateCurr(RTC_DATE_TIME* pCurr, RTC_COUNT* pCount)
{
    u8 sec, min, hour, week = 6;    /*2000/1/1 is Saturday*/
    u32 day, month, year;
    u32 leapDay, dayYear, dayMonth;

    //DEBUG_RTC("rtcCount %02d  %02d:%02d:%02d\n", pCount->day, pCount->hour, pCount->min, pCount->sec);
    sec   =pCount->sec;
    min   = pCount->min;
    hour  = pCount->hour;
    day   = (u32)pCount->day+1;
    month = 1;
    year  = 2000;

    if (sec > 59)
    {
        sec -= 60;
        min++;
    }

    if (min > 59)
    {
        min -= 60;
        hour++;
    }

    if (hour > 23)
    {
        hour -= 24;
        day++;
    }

    week  = (week + day)%7;

    do {
        if ((year % 400) == 0)
                leapDay = 1;
        else if (((year % 4) == 0) && ((year % 100) != 0))
            leapDay = 1;
        else
            leapDay = 0;

        dayYear = 365 + leapDay;
        if (day > dayYear)
        {
            day -= dayYear;
            year++;
        }
        else
            break;
    } while (1);


    do {
        dayMonth = rtcDayMonth[month];
        if (month == 2)
            dayMonth += leapDay;
        if (day > dayMonth)
        {
            day -= dayMonth;
            month++;

            while(month > 12)
            {
                month -= 12;
                year++;
                if ((year % 400) == 0)
                    leapDay = 1;
                else if (((year % 4) == 0) && ((year % 100) != 0))
                    leapDay = 1;
                else
                    leapDay = 0;
            }

        }
        else
            break;
    } while (1);

    pCurr->sec   = (u8)sec;
    pCurr->min   = (u8)min;
    pCurr->hour  = (u8)hour;
    pCurr->day   = (u8)day;
    pCurr->month = (u8)month;
    pCurr->year  = (u8)(year - 2000);
    pCurr->week  = week;

    //DEBUG_RTC("pCurr->sec: %d\n",pCurr->sec);
    //DEBUG_RTC("pCurr->min: %d\n",pCurr->min);
    //DEBUG_RTC("pCurr->hour: %d\n",pCurr->hour);
    //DEBUG_RTC("pCurr->month: %d\n",pCurr->month);
    //DEBUG_RTC("pCurr->year: %d\n",pCurr->year);
    return 1;
}

void rtcSetDefaultTime(void)
{
    DEBUG_RTC("RTC Reset -- Request User Input Date\n");
#if(EXT_RTC_SEL == RTC_PT7C43390)
	PT7C43390_RTC_Init();
#endif
    RTC_Set_GMT_Time(&rtcBase);
}
void rtcGetTmpCount(RTC_COUNT* pCount)
{
    u32 count = RtcTmpCnt;

    pCount->sec  = (u8) ((count >> RTC_COUNT_SEC_SHFT) & 0x3f);
    pCount->min  = (u8) ((count >> RTC_COUNT_MIN_SHFT) & 0x3f);
    pCount->hour = (u8) ((count >> RTC_COUNT_HOUR_SHFT) & 0x1f);
    pCount->day  = (u16)((count >> RTC_COUNT_DAY_SHFT) & 0x7fff);
    //DEBUG_RTC("RTC Get TmpCount: %d days %d:%d:%d\n",pCount->day, pCount->hour, pCount->min, pCount->sec);
}

void rtcSetTmpCount(RTC_COUNT* pCount)
{
    RtcTmpCnt = (((u32)pCount->sec)   << RTC_COUNT_SEC_SHFT)   |
                (((u32)pCount->min)   << RTC_COUNT_MIN_SHFT)   |
                (((u32)pCount->hour)  << RTC_COUNT_HOUR_SHFT)  |
                (((u32)pCount->day)   << RTC_COUNT_DAY_SHFT);

}

void RTC_Set_Time_With_TmpCnt(RTC_DATE_TIME *cur_time, RTC_COUNT* Count)
{
    u8  err;
    u32 j;

    memcpy(&rtcBase, cur_time, sizeof(RTC_DATE_TIME));
    OSSemPend(rtcSemEvt, 5, &err);

    /*Clear The RTC count reg*/
    memset(&rtcCount, 0, sizeof(RTC_COUNT));
    RtcIntCtrl = 0;
    RtcIntCtrl |= RTC_COLOCK << 8;
    RtcCountCtrl &= ~RTC_COUNT_RESET_START;
    for(j=0; j<RTC_SET_DELAY; j++);
    rtcSetTmpCount(Count);
    RtcCountCtrl = RTC_COUNT_RESET_START | RTC_POWER_UP_SRAM | RTC_CLOCK_LOW;
    for(j=0; j<RTC_SET_DELAY; j++);
    rtcGetCount(&rtcCount);

    /* Update rtcBase */
    rtcSetBase(&rtcBase);

    /* Update rtcCurr */
    rtcGetBase(&rtcCurr);

    if (err == OS_NO_ERR)
        OSSemPost(rtcSemEvt);
    
}

void rtcIntHandler(void)
{
    u32 intStat = RtcIntStat;


    if (intStat & RTC_INT_STAT_SEC)
    {
    }

    if (intStat & RTC_INT_STAT_MIN)
    {
        /* minute */
        //RTCseconds+=60;
    }

    if (intStat & RTC_INT_STAT_HOUR)
    {
        /* hour */
        //RTCseconds+=3600;
    }

    if (intStat & RTC_INT_STAT_DAY)
    {
        /* day */
    }

}
#if (USE_BUILD_IN_RTC != RTC_USE_TIMER_RTC)
static BOOLEAN rtcIsTimeValid(RTC_DATE_TIME* ptime)
{
    if ((ptime->year>99) || (ptime->year < 10))
        return FALSE;
    if ((ptime->month>12) || (ptime->month < 1))
        return FALSE;

    if ((ptime->day>31) || (ptime->day < 1))
        return FALSE;

    if (ptime->hour>23)
        return FALSE;

    if (ptime->min>59)
        return FALSE;

    if (ptime->sec>59)
        return FALSE;

    return TRUE;
}
#endif

void rtcStart(void)
{
    //u32 rtc_clock;
    u32 temp;

    memset(&rtcTmpCount, 0, sizeof(RTC_COUNT));
    SYS_CTL0   |= SYS_CTL0_RTC_CKEN;

    RtcBase = 0;
    RtcCountCtrl = RTC_COUNT_RESET_START;
    temp=RtcCount;
    RtcCountCtrl=0;
    RtcIntCtrl |= (RTC_COLOCK << 8);
    RtcTmpCnt=temp;
    RtcCountCtrl = RTC_COUNT_RESET_START;

    // Check colock source is 32768 khz or not
    //rtc_clock=RtcIntCtrl & 0xFFFFFFF0;  // mask y h m s interrupt enable bits
    rtcGetCount(&rtcCount);

    if ((rtcCount.day < 3653) || (rtcCount.hour > 23) || (rtcCount.min > 59) || (rtcCount.sec > 59))
    {
        // RTC no power, reset default
        rtcSetDefaultTime();
    }

    else
    {
        // Compensate the counter to the RTC
        rtcUpdateCurr(&rtcCurr, &rtcCount);
    }
    //The Enable default value is 1 so set 0 -> 1
    // The Colock source should set 0x8000 (32768 KHz)


}

/*

Routine Description:

    Set RTC Base.

Arguments:

    pDateTime - Date/Time.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 rtcSetBase(RTC_DATE_TIME* pDateTime)
{
    return 0;
    #if 0
    RtcBase = (((u32)pDateTime->sec)   << RTC_BASE_SEC_SHFT)   |
              (((u32)pDateTime->min)   << RTC_BASE_MIN_SHFT)   |
              (((u32)pDateTime->hour)  << RTC_BASE_HOUR_SHFT)  |
              (((u32)pDateTime->day)   << RTC_BASE_DAY_SHFT)   |
              (((u32)pDateTime->month) << RTC_BASE_MONTH_SHFT) |
              (((u32)pDateTime->year)  << RTC_BASE_YEAR_SHFT);

    return 1;
    #endif
}

void rtcClearCount(void)
{
    return;
    
    #if 0
    u32 i=0,j;
    
    rtcCount.day=0;
    rtcCount.hour=0;
    rtcCount.min=0;
    rtcCount.sec=0;
    memset(&rtcTmpCount, 0, sizeof(RTC_COUNT));
    //Clear The RTC count reg

    RtcIntCtrl = 0;
    RtcIntCtrl |= RTC_COLOCK << 8;

    RtcCountCtrl &= ~RTC_COUNT_RESET_START;
    for(j=0; j<RTC_SET_DELAY; j++);
    rtcGetTmpCount(&rtcTmpCount);
    RtcCountCtrl = RTC_COUNT_RESET_START | RTC_POWER_UP_SRAM | RTC_CLOCK_LOW;
    for(j=0; j<RTC_SET_DELAY; j++);

    rtcGetCount(&rtcCount);

    while(memcmp(&rtcCount, &rtcTmpCount, sizeof(RTC_COUNT)) != 0)
    {
        RtcCountCtrl &= ~RTC_COUNT_RESET_START;

        for(j=0; j<RTC_SET_DELAY; j++);
        RtcCountCtrl |= RTC_COUNT_RESET_START | RTC_POWER_UP_SRAM | RTC_CLOCK_LOW;

        for(j=0; j<RTC_SET_DELAY; j++);

        rtcGetCount(&rtcCount);
        i++;

        if(i>2)
        {
            //DEBUG_RTC("RTC count initial error \n");
            break;
        }
    }
    #endif
}

#if (USE_BUILD_IN_RTC == RTC_USE_BUILD_IN) //use build-in RTC
/*

Routine Description:

    Update RTC by timer.

Arguments:

    None.

Return Value:

    None.

*/
void rtcUpdateRTC_ByTimer(void)
{
    RTC_DATE_TIME GMTTime;

    /*avoid warning messag*/
    if(GMTTime.day) {}
    //Clear RTC Count
    rtcClearCount();

#if (USE_BUILD_IN_RTC == RTC_USE_BUILD_IN)
     #if UART_GPS_COMMAND  //Lucian: パGPS 丁ㄓタず场RTC
        RTCTime_Local_To_GMT(&g_LocalTime, &GMTTime);

        //Update rtcBase in reg
        rtcSetBase(&GMTTime);
        memcpy(&rtcBase, &GMTTime, sizeof(RTC_DATE_TIME));
     #else  //Lucian: パず场Timmer ㄓタず场RTC
        rtcSetBase(&g_LocalTime);
        /* sync rtcBase by rtcCurr */
        memcpy(&rtcBase, &g_LocalTime, sizeof(RTC_DATE_TIME));
     #endif
#else

#endif


}

#endif

void RTC_TimeToCount(RTC_DATE_TIME *cur_time)
{
    u32 i;

    rtcSetCount.day = 0;
    for (i = 0; i < cur_time->year; i++)
    {
        if (i%4 == 0)   /*leap year*/
            rtcSetCount.day += 366;
        else
            rtcSetCount.day += 365;
    }

    for (i = 1; i < cur_time->month; i++)
    {
        rtcSetCount.day += rtcDayMonth[i];
    }
    if (((cur_time->year%4) == 0) && (cur_time->month > 2))
        rtcSetCount.day++;
    rtcSetCount.day += (cur_time->day-1);
    rtcSetCount.hour =  cur_time->hour;
    rtcSetCount.min = cur_time->min;
    rtcSetCount.sec = cur_time->sec;
}

void  RTC_Set_GMT_Time(RTC_DATE_TIME *cur_time)
{
    //DEBUG_RTC("RTC_Set_GMT_Time 20%02d/%02d/%02d  %02d:%02d:%02d\n",
    //cur_time->year, cur_time->month, cur_time->day, cur_time->hour, cur_time->min, cur_time->sec);
    	BOOLEAN bRet;
#if (USE_BUILD_IN_RTC == RTC_USE_BUILD_IN)
    u16 i;
    u8 err;

    OSSemPend(rtcSemEvt, 5, &err);
    RtcCountCtrl = 0;

    RTC_TimeToCount(cur_time);
    rtcSetTmpCount(&rtcSetCount);
    for(i=0; i<RTC_SET_DELAY; i++);
    //DEBUG_RTC("rtcSetCount %02d  %02d:%02d:%02d\n", rtcSetCount.day, rtcSetCount.hour, rtcSetCount.min, rtcSetCount.sec);
    RtcCountCtrl = RTC_COUNT_RESET_START;
    rtcGetCount(&rtcCount);
    //DEBUG_RTC("rtcCount %02d  %02d:%02d:%02d\n", rtcCount.day, rtcCount.hour, rtcCount.min, rtcCount.sec);
    rtcUpdateCurr(&rtcCurr, &rtcCount);
    if (err == OS_NO_ERR)
        OSSemPost(rtcSemEvt);
#elif (USE_BUILD_IN_RTC == RTC_USE_TIMER_RTC)
    memcpy(&rtcCurr, cur_time, sizeof(RTC_DATE_TIME));
#else
    u8 err;
    OSSemPend(rtcSemEvt, 5, &err);
  #if(EXT_RTC_SEL == RTC_HT1381)
    Set_HT1381_RTC(cur_time);
  #elif(EXT_RTC_SEL == RTC_ISL1208)
    Set_ISL1208_RTC(cur_time);
  #elif(EXT_RTC_SEL == RTC_BQ32000)
    Set_BQ32000_RTC(cur_time);
  #elif(EXT_RTC_SEL == RTC_PCF8563)
    Set_PCF8563_RTC(cur_time);
  #elif(EXT_RTC_SEL == RTC_PT7C43390)
    bRet = Set_PT7C43390_RTC(cur_time);
  #elif(EXT_RTC_SEL == RTC_SD2068)
    Set_SD2068_RTC(cur_time);
  #elif(EXT_RTC_SEL == RTC_HM8563)
    Set_HM8563_RTC(cur_time);
  #endif
    if (err == OS_NO_ERR)
        OSSemPost(rtcSemEvt);
#endif
#if RTC_INCREASE_CHECK
//Update previous time for increase check.
	DEBUG_RTC("Update RTC from %s-%d, ret = %d\n", __func__, __LINE__, (int)bRet);
	rtcIsCorrectIncrease(cur_time, RTC_INCREASE_USER_SET);
#endif
}

void  RTC_Set_Time(RTC_DATE_TIME *cur_time)
{
    RTC_DATE_TIME gmtTime;

//    DEBUG_RTC("RTC_Set_Time 20%02d/%02d/%02d  %02d:%02d:%02d\n",
//    cur_time->year, cur_time->month, cur_time->day, cur_time->hour, cur_time->min, cur_time->sec);
#if(HW_BOARD_OPTION == MR9160_TX_MAYON_MWL613)
	if(rtcIsTimeValid(cur_time) == FALSE)
	{
		DEBUG_RTC("[RTC ERR] Set Invalid Time %02d/%02d/%02d  %02d:%02d:%02d\n",
    			cur_time->year, cur_time->month, cur_time->day, cur_time->hour, cur_time->min, cur_time->sec);
		return;
	}
#endif
    RTCTime_Local_To_GMT(cur_time, &gmtTime);
    RTC_Set_GMT_Time(&gmtTime);
    g_LocalTimeInSec = RTC_Time_To_Second(cur_time);
}

#if RTC_INCREASE_CHECK //Add by Paul for RTC increase check, 2017.06.22
#ifdef HT1381_DEBUG
void BUG_ON()
{
	OS_CPU_SR  cpu_sr;
	sysWDT_disable();
	OS_ENTER_CRITICAL();
	while(1);
	OS_EXIT_CRITICAL();
}
#endif

void delay1ms()
{
unsigned char tick_last;
tick_last = 100;
while(tick_last--);
}

BOOLEAN rtcIsCorrectIncrease(RTC_DATE_TIME *cur_time, RTC_INCREASE_TYPE type)
{
	RTC_DATE_TIME set_time;
	static INT32U pre_tick, pre_time=0;
	static BOOLEAN isFirstErr= TRUE;
	INT32U delta_time=0, delta_tick=0, cur_tick=0;

	cur_tick = OSTimeGet();
	//restore pre-time to cur-time when retry fail
	if(type == RTC_INCREASE_FAIL_TEST)
	{
		pre_time = RTC_Time_To_Second(cur_time);
		DEBUG_RTC("RTC Fail Test\n");
		return TRUE;
	}

	//sync pre-time and cur-time when go back to default time
	if((pre_time== 0) || (type == RTC_INCREASE_USER_SET))
	{
		pre_tick = cur_tick;
		pre_time = RTC_Time_To_Second(cur_time);
		return TRUE;
	}

	//restore pre-time to cur-time when retry fail

	delta_tick = (cur_tick - pre_tick) / 20; //1//1000ms / 50ms = 20, decide by system tick.
	if(type == RTC_INCREASE_RESTORE)
	{
		delta_tick += pre_time;
		RTC_Second_To_Time(delta_tick, &set_time);
		memcpy(cur_time, &set_time, sizeof(RTC_DATE_TIME));
		DEBUG_RTC("RTC restore pre-time %02d/%02d/%02d %02d:%02d:%02d\n",  
	    			set_time.year+2000,set_time.month, set_time.day,(u32)set_time.hour, set_time.min, set_time.sec);
		return TRUE;
	}

	delta_time = RTC_Time_To_Second(cur_time)-pre_time;
//delta_time should same as delta_tick+1, 3TX will get 2 between the 2 value.
	if(( delta_time > delta_tick+1) ||( delta_time < delta_tick))
	{
		RTC_Second_To_Time(pre_time, &set_time);
		DEBUG_RTC("[RTC ERR] delta_time = %d, delta_tick = %d\n", delta_time, delta_tick);
		DEBUG_RTC("[RTC ERR] pre: %02d/%02d/%02d %02d:%02d:%02d\n",  
	    			set_time.year+2000,set_time.month, set_time.day,(u32)set_time.hour, set_time.min, set_time.sec);
		DEBUG_RTC("[RTC ERR] cur: %02d/%02d/%02d %02d:%02d:%02d\n",
					cur_time->year+2000,cur_time->month, cur_time->day,(u32)cur_time->hour, cur_time->min, cur_time->sec);
		if(isFirstErr) {
			//Set trigger ON
			isFirstErr = FALSE;
		} else {
#ifdef HT1381_DEBUG
			DEBUG_RTC("[RTC]%s %d\n", __func__,__LINE__ );
			gpioSetLevel(GPIO_GROUP_SCAN_LED, GPIO_BIT_SCAN_LED, 0);
			BUG_ON();
#endif
		}
//		OSTimeDly(200);//delay 10sec for test verify

		return FALSE;
	}
	if(isFirstErr == FALSE) {
		isFirstErr = TRUE;
		DEBUG_RTC("[RTC ERRONEOUS] cur: %02d/%02d/%02d %02d:%02d:%02d\n",
					cur_time->year+2000,cur_time->month, cur_time->day,(u32)cur_time->hour, cur_time->min, cur_time->sec);
#ifdef HT1381_DEBUG
		gpioSetLevel(GPIO_GROUP_SCAN_LED, GPIO_BIT_SCAN_LED, 0);
		BUG_ON();
	} else {
		gpioSetLevel(GPIO_GROUP_SCAN_LED, GPIO_BIT_SCAN_LED, 0);
		gpioSetLevel(GPIO_GROUP_SCAN_LED, GPIO_BIT_SCAN_LED, 1);
#endif
	}

	pre_tick = cur_tick;
	pre_time = RTC_Time_To_Second(cur_time);//time correct, update pre-time
	return TRUE;
}
#endif

void RTC_Get_GMT_Time(RTC_DATE_TIME *cur_time)
{
    u8 retry=0, error=0, bRet = FALSE;

#if (USE_BUILD_IN_RTC == RTC_USE_BUILD_IN)
    rtcGetCount(&rtcCount);
    rtcUpdateCurr(&rtcCurr, &rtcCount);
    memcpy(cur_time, &rtcCurr, sizeof(RTC_DATE_TIME));

    if(rtcIsTimeValid(cur_time) == FALSE)
    {
        DEBUG_RTC("RTC Get Time Error, %d \n", retry);
        error = 1;
    }
    else
    {
        error = 0;
    }
#elif (USE_BUILD_IN_RTC == RTC_USE_TIMER_RTC)
    memcpy(cur_time, &rtcCurr, sizeof(RTC_DATE_TIME));
    if(cur_time->year > 63)
        cur_time->year=0;
    //DEBUG_RTC("RTC_Get_GMT_Time 20%02d/%02d/%02d  %02d:%02d:%02d\n",
        //cur_time->year, cur_time->month, cur_time->day, cur_time->hour, cur_time->min, cur_time->sec);
#else //本 RTC, Re-try .
    while(retry<5)
    {
      #if(EXT_RTC_SEL == RTC_HT1381)
        bRet = Get_HT1381_RTC(cur_time);
      #elif(EXT_RTC_SEL == RTC_ISL1208)
        bRet = Get_ISL1208_RTC(cur_time);
      #elif(EXT_RTC_SEL == RTC_BQ32000)
        bRet = Get_BQ32000_RTC(cur_time);
      #elif(EXT_RTC_SEL == RTC_PCF8563)
        bRet = Get_PCF8563_RTC(cur_time);
      #elif(EXT_RTC_SEL == RTC_PT7C43390)
        bRet = Get_PT7C43390_RTC(cur_time);
      #elif(EXT_RTC_SEL == RTC_SD2068)
        bRet = Get_SD2068_RTC(cur_time);
      #elif(EXT_RTC_SEL == RTC_HM8563)
        bRet = Get_HM8563_RTC(cur_time);
		bRet = TRUE;
      #endif

        /*check time range*/
        if(bRet == FALSE || rtcIsTimeValid(cur_time) == FALSE
#if RTC_INCREASE_CHECK
			|| rtcIsCorrectIncrease(cur_time,RTC_INCREASE_VERIFY) == FALSE
#endif
			)
        {
            DEBUG_RTC("RTC Get Time Error, %d \n", retry);
            error = 1;
            //OSTimeDly(1);
        }
        else
        {
            error = 0;
            break;
        }

        retry++;
    }
#endif


    if (error==1)
    {
        DEBUG_RTC("RTC recover, %d-%02d/%02d/%02d %02d:%02d:%02d\n",
					retry, cur_time->year+2000,cur_time->month,
			    	cur_time->day,(u32)cur_time->hour, cur_time->min, cur_time->sec);

#if RTC_INCREASE_CHECK
		rtcIsCorrectIncrease(cur_time, RTC_INCREASE_RESTORE);//restore previous time
		RTC_Set_GMT_Time(cur_time);
#else
		rtcSetDefaultTime();
		memcpy(cur_time, &rtcBase, sizeof(RTC_DATE_TIME));
#endif
    }

}

void RTC_Get_Time(RTC_DATE_TIME*loc_time)
{
#if 0  //Lucian: 参パ 1s timer弄
    memcpy(loc_time,&g_LocalTime,sizeof(RTC_DATE_TIME));
#else
    RTC_DATE_TIME gmt;
    RTC_Get_GMT_Time(&gmt);
    RTCTime_Gmt_To_Local(&gmt, loc_time);
#endif
}

void RTC_Set_DST(u8 enable)
{
    if (enable >= RTC_DST_OFFSET_END)
    {
        DEBUG_RTC("RTC_Set_DST Error %d \n", enable);
        return;
    }
    rtcDstEnable = enable;
}

u8 RTC_Get_DST(void)
{
    return rtcDstEnable;
}

#if RTC_WEEKDAY_CALCULATE
//Tomohiko Sakamoto's Algorithm, day of week
u8 RTC_Get_Week(RTC_DATE_TIME*loc_time)
{
  static u8 t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  loc_time->year -= loc_time->month < 3;
  return (loc_time->year + loc_time->year/4 -
  			loc_time->year/100 + loc_time->year/400 +
  			t[loc_time->month-1] + loc_time->day) % 7;
}
#else
u8 RTC_Get_Week(RTC_DATE_TIME*loc_time)
{
    u8 is_leap;
    u8 i;
    u32 days=0;
    u8 weekday;

    is_leap = (loc_time->year%4 == 0) ? 1 : 0;


    /*calculate the week day of the first day of this month*/
    for (i = 0; i < loc_time->year; i++)
    {
        if (i%4 == 0)   /*leap year*/
            days+=366;
        else
            days+=365;
    }
    for (i = 1; i < loc_time->month; i++)
    {
        days+=rtcDayMonth[i];
    }
    if (loc_time->month > 2 && is_leap == 1)  /*leap year*/
        days++;


    days+=loc_time->day;

    weekday = (u8) ((days+5)%7);   /*2000.01.01 is Sat*/

    return weekday;
}
#endif
/*

Routine Description:

    Get RTC Base.

Arguments:

    pDateTime - Date/Time.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 rtcGetBase(RTC_DATE_TIME* pDateTime)
{
    u32 base = RtcBase;

#if 1
    pDateTime->sec   = (u8)((base & RTC_BASE_SEC_MASK) >> RTC_BASE_SEC_SHFT);
    pDateTime->min   = (u8)((base & RTC_BASE_MIN_MASK) >> RTC_BASE_MIN_SHFT);
    pDateTime->hour  = (u8)((base & RTC_BASE_HOUR_MASK)>> RTC_BASE_HOUR_SHFT);
    pDateTime->day   = (u8)((base & RTC_BASE_DAY_MASK) >> RTC_BASE_DAY_SHFT);
    pDateTime->month = (u8)((base & RTC_BASE_MONTH_MASK)>> RTC_BASE_MONTH_SHFT);
    pDateTime->year  = (u8)((base & RTC_BASE_YEAR_MASK)>> RTC_BASE_YEAR_SHFT);
#else
    pDateTime->sec   = (u8)(base >> RTC_BASE_SEC_SHFT);
    pDateTime->min   = (u8)(base >> RTC_BASE_MIN_SHFT);
    pDateTime->hour  = (u8)(base >> RTC_BASE_HOUR_SHFT);
    pDateTime->day   = (u8)(base >> RTC_BASE_DAY_SHFT);
    pDateTime->month = (u8)(base >> RTC_BASE_MONTH_SHFT);
    pDateTime->year  = (u8)(base >> RTC_BASE_YEAR_SHFT);
#endif
    return 1;
}

/*

Routine Description:

    Get RTC Count.

Arguments:

    pDateTime - Date/Time.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 rtcGetCount(RTC_COUNT* pCount)
{
    u32 count = RtcCount;

    pCount->sec  = (u8) ((count >> RTC_COUNT_SEC_SHFT) & 0x3f);
    pCount->min  = (u8) ((count >> RTC_COUNT_MIN_SHFT) & 0x3f);
    pCount->hour = (u8) ((count >> RTC_COUNT_HOUR_SHFT) & 0x1f);
    pCount->day  = (u16)((count >> RTC_COUNT_DAY_SHFT) & 0x7fff);
    //DEBUG_RTC("RTC SEC: %d \n",pCount->sec);
    //DEBUG_RTC("RTC MIN: %d \n",pCount->min);
    //DEBUG_RTC("RTC hour: %d \n",pCount->hour);
    //DEBUG_RTC("RTC day: %d \n",pCount->day);

    return 1;
}


/*

Routine Description:

    Get num of month

Arguments:

    year - current year
    month - current month

Return Value:

    num of month
    

*/

u8 rtcGetDayNum(u16 year, u8 month)
{
    if(((year %4) == 0) && (month == 2))
    {
        return 29;
    }
    else
    {
        return rtcDayMonth[month];
    }
}

void rtcSuspendTask(void)
{
    /* Suspend the task */
    //DEBUG_IIS("Trace: IIS task suspending.\n");
    OSTaskSuspend(RTC_TASK_PRIORITY);


}

//Civic 070919 S
void rtcTask(void* pData)
{
    u8 err;

    OSSemPend(rtcSemEvt, OS_IPC_WAIT_FOREVER, &err);
    rtcGetCount(&rtcCount);
    rtcUpdateCurr(&rtcCurr, &rtcCount);

}

#if (USE_BUILD_IN_RTC == RTC_USE_BUILD_IN)
void rtcInit(void)
{
    rtcStart();
    rtcSemEvt = OSSemCreate(1);
#if(CHIP_OPTION < CHIP_A1013A)
    OSTaskCreate(RTC_TASK, RTC_TASK_PARAMETER, RTC_TASK_STACK, RTC_TASK_PRIORITY);
#endif
}
#endif

#if Use_Iranian_time

 /* Decides if Jalali year a leap one:
 those years that have a remainder of
 1, 5, 9, 13, 17, 22, 26, and 30 when divided by 33 */
 /*Time Base is 1380.01.01  Wednesday*/
BOOLEAN Jleap(u32 year)
{
    u32 tmp;
    u32  Iranian_year;

    Iranian_year = 1380 + year;
    tmp = Iranian_year % 33;
    if (tmp == 1 || tmp == 5||tmp==9||tmp==13||tmp==17||tmp==22||tmp==26||tmp==30)
        return TRUE;
    else
        return FALSE;
}
u32 RTC_Time_To_Second(RTC_DATE_TIME *Source)
{
    u32 i, day = 0, sec;


    if(Source == NULL)
        return 0;

    for (i = 0; i < Source->year; i++)
    {
        if (Jleap(i) == TRUE) /*leap year*/
            day +=366;
        else
            day +=365;
    }

    for (i = 1; i < Source->month; i++)
    {
        day+=rtcDayMonth[i];
    }
    day += (Source->day-1);
    sec = day*86400+Source->hour*3600+Source->min*60+Source->sec;
    return sec;
}

void  RTC_Second_To_Time(u32 Sec, RTC_DATE_TIME *time)
{
    u8 year = 0, mon = 1, i;
    u32 day;

//    DEBUG_RTC("RTC_Second_To_Time sec %d\n", Sec);
    day = Sec/86400;
    Sec = Sec%86400;
    time->hour = Sec/3600;
    Sec = Sec%3600;
    time->min = Sec/60;
    time->sec = Sec%60;

    time->week = (day+2)%7;

    while (day > 366)
    {
        if (Jleap(year) == TRUE) /*leap year*/
            day-=366;
        else
            day-=365;
        year++;
    }
    if((Jleap(year)== FALSE)&&(day == 366))
    {
        year++;
        day-=365;
    }
    time->year = year;
    for (i = 1; i <= 12; i++)
    {
        if(i == 12 && (Jleap(year) == TRUE))
        {
            break;
        }
        else if(day > rtcDayMonth[i])
        {
            mon++;
            day -= rtcDayMonth[i];
        }
        else
            break;
    }
    time->month = mon;
    time->day = day;

}
#else
/*Time Base is 2000.01.01*/
u32 RTC_Time_To_Second(RTC_DATE_TIME *Source)
{
    u32 i, day = 0, sec;

    if(Source == NULL)
        return 0;
    for (i = 0; i < Source->year; i++)
    {
        if (i%4 == 0)   /*leap year*/
            day +=366;
        else
            day +=365;
    }
    for (i = 1; i < Source->month; i++)
    {
        day+=rtcDayMonth[i];
    }
    if ((Source->month > 2) && (Source->year%4 == 0))  /*leap year*/
        day++;
    day += (Source->day-1);
    sec = day*86400+Source->hour*3600+Source->min*60+Source->sec;
    return sec;
}

void RTC_Second_To_Time(u32 Sec, RTC_DATE_TIME *time)
{
    u8 year = 0, mon = 1, i, week;
    u32 day;

//    DEBUG_RTC("RTC_Second_To_Time sec %d\n", Sec);
    day = Sec/86400+1;
    Sec = Sec%86400;
    time->hour = Sec/3600;
    Sec = Sec%3600;
    time->min = Sec/60;
    time->sec = Sec%60;
    week = (day%7)+5;
    if (week >= 7)
        week -= 7;
    while (day > 365)
    {
        if (year%4 == 0)
        {
            if (day == 366)
                break;
            else
                day-=366;
        }
        else
        {
            day-=365;
        }
        year++;
    }
    time->year = year;
    for (i = 1; i <= 12; i++)
    {
        if(i == 2 && year%4 == 0)
        {
            if(day > 29)
            {
                mon++;
                day -= 29;
            }
            else
                break;
        }
        else if(day > rtcDayMonth[i])
        {
            mon++;
            day -= rtcDayMonth[i];
        }
        else
            break;
    }
    time->month = mon;
    time->day = day;
    time->week = week;

}
#endif
u8 RTC_Set_TimeZone(RTC_TIME_ZONE *zone)
{
    if((zone->operator > 1) || (zone->hour > 12) || (zone->min != 0 && zone->min != 30))
        return 0;
    memcpy(&rtcTimeZone, zone, sizeof(RTC_TIME_ZONE));
    rtcTimeZoneSec = zone->hour*3600+zone->min*60;
    return 1;
}

void RTC_Get_TimeZone(RTC_TIME_ZONE *zone)
{
    memcpy(zone, &rtcTimeZone, sizeof(RTC_TIME_ZONE));
    return;
}

void RTCTime_Gmt_To_Local(RTC_DATE_TIME *GMT, RTC_DATE_TIME *Local)
{
    u32 sec;
    memset(Local, 0, sizeof(RTC_DATE_TIME));
    sec = RTC_Time_To_Second(GMT);
    if(rtcTimeZone.operator == 1)
    {
        if(sec < rtcTimeZoneSec)   /*local time is 1999/12/31*/
        {
            sec = sec+86400-rtcTimeZoneSec;
            Local->year = 99;
            Local->month = 12;
            Local->day = 31;
            Local->hour = sec/3600;
            sec = sec%3600;
            Local->min = sec/60;
            Local->sec = sec%60;
            return;
        }
        else
            sec-=rtcTimeZoneSec;
    }
    else
        sec+=rtcTimeZoneSec;

    if (rtcDstEnable == RTC_DST_OFFSET_1HR)
        sec += 3600;
    else if (rtcDstEnable == RTC_DST_OFFSET_2HR)
        sec += 7200;
#if( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
	else
		sec += 0;
#endif	


    RTC_Second_To_Time(sec, Local);

    g_LocalTimeInSec=sec;

    return;
}

void RTCTime_Local_To_GMT(RTC_DATE_TIME *Local, RTC_DATE_TIME *GMT)
{
    u32 sec;

//    DEBUG_RTC("RTCTime_Local_To_GMT Local  20%02d/%02d/%02d  %02d:%02d:%02d\n",
//        Local->year, Local->month, Local->day, Local->hour, Local->min, Local->sec);
    memcpy(GMT, Local, sizeof(RTC_DATE_TIME));
    if((Local->year == 0) && (Local->month== 1) &&(Local->day == 1))
    {
        if(rtcTimeZone.operator == 0)
            return;
        sec = Local->hour*3600+Local->min*60+Local->sec;
        if(sec < (86400-rtcTimeZoneSec))
            return;
        sec-=(86400-rtcTimeZoneSec);
    }
    else
    {
        sec = RTC_Time_To_Second(Local);
        if(rtcTimeZone.operator == 0)
            sec-=rtcTimeZoneSec;
        else
            sec+=rtcTimeZoneSec;
    }
    if (rtcDstEnable == RTC_DST_OFFSET_1HR)
        sec -= 3600;
    if (rtcDstEnable == RTC_DST_OFFSET_2HR)
        sec -= 7200;
    RTC_Second_To_Time(sec, GMT);

//    DEBUG_RTC("RTCTime_Local_To_GMT GMT  20%02d/%02d/%02d  %02d:%02d:%02d\n",
//        GMT->year, GMT->month, GMT->day, GMT->hour, GMT->min, GMT->sec);
    return;
}
//Civic 070919 E


__inline u8 GetBCDtoDecimal(u8 data)
{
    return  ((data >> 4) & 0xF) * 10 + ((data) & 0xF);
}
__inline u8 ConvertDecimaltoBCD(u8 data)
{
    return  ((data / 10) << 4) | (data % 10);
}

#if (USE_BUILD_IN_RTC == RTC_USE_EXT_RTC)//use build-in RTC

#if(EXT_RTC_SEL  ==  RTC_HT1381)

//*********************** HT1381 RTC function begin ******************************

#define RTC_HT1381_SDA_W(Value)     gpioSetLevel(RTC_HT1381_SDA_GROUP, RTC_HT1381_SDA, Value)
#define RTC_HT1381_SDA_R(Value)     gpioGetLevel(RTC_HT1381_SDA_GROUP, RTC_HT1381_SDA, Value)
#define RTC_HT1381_SCK_W(Value)     gpioSetLevel(RTC_HT1381_SCK_GROUP, RTC_HT1381_SCK, Value)
#define RTC_HT1381_RST_W(Value)     gpioSetLevel(RTC_HT1381_RST_GROUP, RTC_HT1381_RST, Value)
#define RTC_HT1381_SDA_DirI()         gpioSetDir(RTC_HT1381_SDA_GROUP, RTC_HT1381_SDA ,1)
#define RTC_HT1381_SDA_DirO()         gpioSetDir(RTC_HT1381_SDA_GROUP, RTC_HT1381_SDA ,0)

void RTC_HT1381_W_Byte(u8 Addr, u8 Data)
{
    s32 i;
    u8  Value;

    // write command
    Value   = 0x80 | (Addr << 1);
	OS_ENTER_CRITICAL();
	RTC_HT1381_RST_W(0);
    RTC_HT1381_SCK_W(0);
    RTC_HT1381_RST_W(1);
    for(i = 0; i < 8; i++)
    {
        RTC_HT1381_SDA_W((Value >> i) & 1);
        RTC_HT1381_SCK_W(1);
        RTC_HT1381_SCK_W(0);
    }

    // write data
    for(i = 0; i < 8; i++)
    {
        RTC_HT1381_SDA_W((Data >> i) & 1);
        RTC_HT1381_SCK_W(1);
        RTC_HT1381_SCK_W(0);
    }

    RTC_HT1381_RST_W(0);
	OS_EXIT_CRITICAL();
}

u8 RTC_HT1381_R_Byte(u8 Addr)
{
    s32 i;
    u8  Data, Value;

    // write command
    Value   = 0x81 | (Addr << 1);
	OS_ENTER_CRITICAL();
	RTC_HT1381_RST_W(0);
    RTC_HT1381_SCK_W(0);
    RTC_HT1381_RST_W(1);
    for(i = 0; i < 8; i++)
    {
        RTC_HT1381_SDA_W((Value >> i) & 1);
        RTC_HT1381_SCK_W(1);
        RTC_HT1381_SCK_W(0);
    }

    // read data
    RTC_HT1381_SDA_W(0);
    RTC_HT1381_SDA_DirI();
    Data    = 0;
    for(i = 0; i < 8; i++)
    {
        RTC_HT1381_SDA_R(&Value);
        Data   |= Value << i;
        RTC_HT1381_SCK_W(1);
        RTC_HT1381_SCK_W(0);
    }
    RTC_HT1381_RST_W(0);
    RTC_HT1381_SDA_DirO();
    RTC_HT1381_SDA_W(0);
	OS_EXIT_CRITICAL();

    return  Data;
}

void RTC_HT1381_W_Burst(u8 *pData)
{
    s32 i, j;
    u8  Value, Data;

    // write command
    Value   = 0xbe;
	OS_ENTER_CRITICAL();
	RTC_HT1381_RST_W(0);
    RTC_HT1381_SCK_W(0);
    RTC_HT1381_RST_W(1);
    for(i = 0; i < 8; i++)
    {
        RTC_HT1381_SDA_W((Value >> i) & 1);
        RTC_HT1381_SCK_W(1);
        RTC_HT1381_SCK_W(0);
    }

    // burst write data
    for(j = 0; j < 8; j++)
    {
        Data    = pData[j];
        for(i = 0; i < 8; i++)
        {
            RTC_HT1381_SDA_W((Data >> i) & 1);
            RTC_HT1381_SCK_W(1);
            RTC_HT1381_SCK_W(0);
        }
    }
    RTC_HT1381_RST_W(0);
	OS_EXIT_CRITICAL();
}

void RTC_HT1381_R_Burst(u8 *pData)
{
    s32 i, j, k =100;
    u8  Data, Value;


	OS_ENTER_CRITICAL();

    // write command
    Value   = 0xbf;
	RTC_HT1381_RST_W(0);
    RTC_HT1381_SCK_W(0);
    RTC_HT1381_RST_W(1);
    for(i = 0; i < 8; i++)
    {
        RTC_HT1381_SDA_W((Value >> i) & 1);
        RTC_HT1381_SCK_W(1);
        RTC_HT1381_SCK_W(0);
    }

    // burst read data
    
	while(Value & k--) {
		RTC_HT1381_SDA_W(0);
		RTC_HT1381_SDA_R(&Value);
//		printf("%c", (Value)?'.':'-');
	}
	if(k == 0) {
		gpioSetLevel(GPIO_GROUP_SCAN_LED, GPIO_BIT_SCAN_LED, 0);
		DEBUG_RTC("HT1381 clear SDA GPIO fail - %d\n", Value);
	}

    RTC_HT1381_SDA_DirI();
	k=100;
	while(k--);// Wait stat change

    for(j = 0; j < 8; j++)
    {
        Data    = 0;
        for(i = 0; i < 8; i++)
        {
            RTC_HT1381_SDA_R(&Value);
            Data   |= Value << i;
            RTC_HT1381_SCK_W(1);
            RTC_HT1381_SCK_W(0);
        }
        pData[j]    = Data;
    }
    RTC_HT1381_RST_W(0);
    RTC_HT1381_SDA_DirO();
    RTC_HT1381_SDA_W(0);
	OS_EXIT_CRITICAL();
}

void Set_HT1381_RTC(RTC_DATE_TIME* date)
{
    u8  data[8];
    u8  err;

    OSSemPend(rtcHT1381SemEvt, RTC_HT1381_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_MP4("Set_HT1381_RTC() Error: rtcHT1381SemEvt is %d.\n", err);
    }


    data[0] = ConvertDecimaltoBCD(date->sec);
    data[1] = ConvertDecimaltoBCD(date->min);
    data[2] = ConvertDecimaltoBCD(date->hour);  //Set to 24H Format
    data[3] = ConvertDecimaltoBCD(date->day);
    data[4] = ConvertDecimaltoBCD(date->month);
    data[6] = ConvertDecimaltoBCD(date->year);

//  DEBUG_RTC("Set_HT1381_RTC Dec:20%02d/%02d/%02d %02d:%02d:%02d\n", date->year, date->month, date->day, date->hour, date->min, date->sec);
//  DEBUG_RTC("Set_HT1381_RTC BCD:20%02x/%02x/%02x %02x:%02x:%02x\n", data[6], data[4], data[3], data[2], data[1], data[0]);
    RTC_HT1381_W_Burst((u8*)data);

    OSSemPost(rtcHT1381SemEvt);
}

BOOLEAN Get_HT1381_RTC(RTC_DATE_TIME *rtc_time)
{
    u8  data[8];
    u8  err;

    OSSemPend(rtcHT1381SemEvt, RTC_HT1381_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_MP4("Get_HT1381_RTC() Error: rtcHT1381SemEvt is %d.\n", err);
    }

    RTC_HT1381_R_Burst((u8*)data);
    rtc_time->sec     = GetBCDtoDecimal(data[0] & 0x7f);
    rtc_time->min     = GetBCDtoDecimal(data[1]);
    if(data[2] & 0x80)  // if 12 hours mode and pm
        rtc_time->hour    = GetBCDtoDecimal(data[2] & 0x1F) + ((data[2] & 0x20) ? 12 : 0);
    else
        rtc_time->hour    = GetBCDtoDecimal(data[2]);

    rtc_time->day     = GetBCDtoDecimal(data[3]);
    rtc_time->month   = GetBCDtoDecimal(data[4]);
    rtc_time->year    = GetBCDtoDecimal(data[6]);

    //DEBUG_RTC("Get_HT1381_RTC Dec:20%02d/%02d/%02d %02d:%02d:%02d\n", time.year, time.month, time.day, time.hour, time.min, time.sec);
    //DEBUG_RTC("Get_HT1381_RTC BCD:20%02x/%02x/%02x %02x:%02x:%02x\n", data[6], data[4], data[3], data[2], data[1], data[0]);

    OSSemPost(rtcHT1381SemEvt);
    return TRUE;
}

void RTC_HT1381_Init(void)
{
    u8              data;
    RTC_DATE_TIME   time, time1;
    u8              err;

    SYS_CTL0   |= SYS_CTL0_GPIO3_CKEN;

    rtcHT1381SemEvt = OSSemCreate(1);

    OSSemPend(rtcHT1381SemEvt, RTC_HT1381_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_MP4("RTC_HT1381_Init() Error: rtcHT1381SemEvt is %d.\n", err);
    }

    // Disable write protect
    RTC_HT1381_W_Byte(7, 0);
    data    = RTC_HT1381_R_Byte(7);
    DEBUG_RTC("HT1381 RTC port %d = %d\n", 7, data);


    // Enable oscillator
    if( RTC_HT1381_R_Byte(0) & 0x80)
    {
        RTC_HT1381_W_Byte(0, ConvertDecimaltoBCD(time.sec));          // enable oscillator
        //RTC_HT1381_W_Byte(0, 0x80 | ConvertDecimaltoBCD(time.sec));   // disable oscillator
        data    = RTC_HT1381_R_Byte(0);
        DEBUG_RTC("HT1381 RTC port %d = %d\n", 0, data);
    }
    OSSemPost(rtcHT1381SemEvt);

    Get_HT1381_RTC(&time);
    DEBUG_RTC("RTC_HT1381 Date:%04d/%02d/%02d %02d:%02d:%02d\n",(time.year + 2000), time.month, time.day, time.hour, time.min, time.sec);

    /*check time range*/
    if(rtcIsTimeValid(&time) == FALSE)
    {
        DEBUG_RTC("RTC Get Time Error!!! Reset RTC!!!\n");
        // Reset time to 2010/01/01 00:00:00
        memcpy(&time, &rtcBase, sizeof(RTC_DATE_TIME));
        Set_HT1381_RTC(&time);
        Get_HT1381_RTC(&time1);
        DEBUG_RTC("RTC_HT1381 Date:%04d/%02d/%02d %02d:%02d:%02d\n", (time1.year + 2000), time1.month, time1.day, time1.hour, time1.min, time1.sec);
    }
    //RTC_Get_GMT_Time(&time);
    RTCTime_Gmt_To_Local(&time, &g_LocalTime);
    DEBUG_RTC("g_LocalTime:    20%02d/%02d/%02d %02d:%02d:%02d\n", g_LocalTime.year, g_LocalTime.month, g_LocalTime.day, g_LocalTime.hour, g_LocalTime.min, g_LocalTime.sec);
}
//*********************** HT1381 RTC function end ******************************

#elif(EXT_RTC_SEL == RTC_BQ32000)

//*********************** BQ32000 RTC function begin ******************************
void Set_BQ32000_RTC(RTC_DATE_TIME* date)
{
    u32 wdata;
    u8  i;
    u8  err;

    wdata   = (ConvertDecimaltoBCD(date->sec)   << 16) |
              (ConvertDecimaltoBCD(date->min)   <<  8) |
               ConvertDecimaltoBCD(date->hour);
    i2cWrite_BQ32000_Word(0, wdata, 3);
    wdata   = (ConvertDecimaltoBCD(date->day)   << 16) |
              (ConvertDecimaltoBCD(date->month) <<  8) |
              ConvertDecimaltoBCD(date->year);
    i2cWrite_BQ32000_Word(4, wdata, 3);

    //DEBUG_RTC("Set_BQ32000_RTC Dec:20%02d/%02d/%02d %02d:%02d:%02d\n", date->year, date->month, date->day, date->hour, date->min, date->sec);
    //DEBUG_RTC("Set_BQ32000_RTC BCD:20%02x/%02x/%02x %02x:%02x:%02x\n", data[6], data[4], data[3], data[2], data[1], data[0]);
}

BOOLEAN Get_BQ32000_RTC(RTC_DATE_TIME *rtc_time)
{
    u32 wdata;

    i2cRead_BQ32000_Word(0, &wdata, 3);
    //DEBUG_RTC("BQ32000: 0x%08x ", wdata);
    rtc_time->hour  = ((wdata >>  4) & 0x03) * 10 + ((wdata >>  0) & 0x0f);
    rtc_time->min   = ((wdata >> 12) & 0x07) * 10 + ((wdata >>  8) & 0x0f);
    rtc_time->sec   = ((wdata >> 20) & 0x07) * 10 + ((wdata >> 16) & 0x0f);

    i2cRead_BQ32000_Word(4, &wdata, 3);
    //DEBUG_RTC("0x%08x\n", wdata);
    rtc_time->year  = ((wdata >>  4) & 0x0f) * 10 + ((wdata >>  0) & 0x0f);
    rtc_time->month = ((wdata >> 12) & 0x01) * 10 + ((wdata >>  8) & 0x0f);
    rtc_time->day   = ((wdata >> 20) & 0x03) * 10 + ((wdata >> 16) & 0x0f);

    //DEBUG_RTC("Get_BQ32000_RTC Dec:20%02d/%02d/%02d %02d:%02d:%02d\n", time.year, time.month, time.day, time.hour, time.min, time.sec);
    //DEBUG_RTC("Get_BQ32000_RTC BCD:20%02x/%02x/%02x %02x:%02x:%02x\n", data[6], data[4], data[3], data[2], data[1], data[0]);
    return TRUE;
}

void RTC_BQ32000_Init(void)
{
    RTC_DATE_TIME   time, time1;

    Get_BQ32000_RTC(&time);
    DEBUG_RTC("RTC_BQ32000 Date:%04d/%02d/%02d %02d:%02d:%02d\n", (time.year + 2000), time.month, time.day, time.hour, time.min, time.sec);

    /*check time range*/
    if(rtcIsTimeValid(&time) == FALSE)
    {
        DEBUG_RTC("RTC Get Time Error!!! Reset RTC!!!\n");
        // Reset time to 2010/01/01 00:00:00
        time.year   = 10;
        time.month  = 1;
        time.day    = 1;
        time.hour   = 0;
        time.min    = 0;
        time.sec    = 0;
        Set_BQ32000_RTC(&time);
        Get_BQ32000_RTC(&time1);
        DEBUG_RTC("RTC_BQ32000 Date:%04d/%02d/%02d %02d:%02d:%02d\n", (time1.year + 2000), time1.month, time1.day, time1.hour, time1.min, time1.sec);
    }
    //RTC_Get_GMT_Time(&time);
    RTCTime_Gmt_To_Local(&time, &g_LocalTime);
    DEBUG_RTC("g_LocalTime: 20%02d/%02d/%02d %02d:%02d:%02d\n", g_LocalTime.year, g_LocalTime.month, g_LocalTime.day, g_LocalTime.hour, g_LocalTime.min, g_LocalTime.sec);

}
//*********************** BQ32000 RTC function end ******************************
#endif
#endif  /*end of #if(USE_BUILD_IN_RTC == RTC_USE_EXT_RTC)*/



