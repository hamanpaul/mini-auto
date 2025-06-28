/*
 *	File:		AsDate.c
 *
 *	Contains:	Miscellaneous Date and Time routines.
 *				These routines may need to be modified for a given
 *				operating environment
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Copyright:	© 1995-2003 by Allegro Software Development Corporation
 *  All rights reserved.
 *
 *  This module contains confidential, unpublished, proprietary 
 *  source code of Allegro Software Development Corporation.
 *
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 *
 *  License is granted for specific uses only under separate 
 *  written license by Allegro Software Development Corporation.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 * * * * Release 4.30  * * *
 *		06/24/03	amp		improve PSOS support for Philips
 * * * * Release 4.21  * * *
 *		05/14/03	pjr		add PSOS support for Philips
 * * * * Release 4.20  * * *
 * * * * Release 4.01  * * *
 *		08/24/01	bva		remove test code
 * * * * Release 4.00  * * *
 *		11/07/00	pjr		eliminate global data pointer (gRpDataPtr)
 *		05/26/00	rhb/amp	if RomPagerCalendarTime, base time on 1/1/1900
 *							add RpComputeEpochReference
 *		05/15/00	rhb		remove #include string.h and stdlib.h
 *		02/09/00	bva		use AsEngine.h
 * * * * Release 3.10  * * *
 * * * * Release 3.02  * * *
 *		05/05/99	bva		rework Precise version of RpGetSysTimeInSeconds
 * * * * Release 3.0 * * * *
 *		01/22/99	pjr		RpDate.c -> AsDate.c
 *		12/31/98	pjr		create from routines in RpUtil.c
 * * * * Release 2.2 * * * *
 *		11/14/98	bva		RpGetRomTimeInSeconds -> RpGetMonthDayYearInSeconds
 *		11/09/98	bva		use macro abstraction for stdlib calls
 *							remove RpFree, RpAlloc routines
 *		07/18/98	bva		add time support for target OS environments
 * * * * Release 2.1 * * * *
 *		04/20/98	bva		moved include of time.h
 *		04/16/98	bva		use memset in RpAlloc
 *		02/13/98	rhb		change parameter to RpCatUnsigned32ToString
 * * * * Release 2.0 * * * *
 *		12/08/97	pjr		change RpBuildDateString to generate a 2 digit
 *							day number.  some servers do not parse a single
 *							digit day number in a date string properly.
 * * * * Release 1.6 * * * *
 *		04/04/97	bva		cleanup warnings
 *		12/12/96	rhb		use Jan 1, 1901 as start for date calculations 
 *								and do date calculations better
 * * * * Release 1.5 * * * *
 * * * * Release 1.3 * * * *
 *		06/13/96	bva		added CRLF to end of RpBuildDateString to save 
 *							calls in BuildDateHeaders
 * * * * Release 1.2 * * * *
 *		06/01/96	bva		added RomPagerUseStandardTime and internal
 *								date/time routines
 * * * * Release 1.1 * * * *
 * * * * Release 1.0 * * * *
 *		01/10/96	bva		created	
 *
 *	To Do:
 */
#include "general.h"
#include "AsEngine.h"

#if RomPagerUseStandardTime
	#include <time.h>
#elif (RpTargetOS == eRpTargetTestNoCal) || (RpTargetOS == eRpTargetTestNoStd)
	#include <time.h>
#elif (RpTargetOS == eRpTargetPSOS)
	#include <psos.h>
	#include <time.h>
#elif (RpTargetOS == eRpTargetNucleus) 
	#include <nucleus.h>
	#include <target.h>
#elif (RpTargetOS == eRpTargetPrecise) 
	#include "mqx.h"
#endif


/*
	Data tables.
*/

const char *	gRpMonthTable[] =
					{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
						"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

#if (RomPagerUseStandardTime == 0)
static const char *gRpDayName[] =
				{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const Unsigned16	gRpMonthDays[] =
				{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
static const Unsigned16	gRpMonthDaysLeapYear[] =
				{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };
#endif

/*
	RpComputeEpochReference
	Compute the reference offset which is used to adjust times
	between the Allegro Epoch (1/1/1900)
	and the stdio epoch (which is system dependent).
	This function should be called during initialization.
*/
#if RomPagerUseStandardTime && RomPagerCalendarTime
Unsigned32 RpComputeEpochReference(void) {
	Unsigned32	theEpochReference;
	struct tm	theReferenceTime;
	time_t		theReferenceTimeSeconds;

	RP_MEMSET(&theReferenceTime, 0, sizeof(theReferenceTime));
	theReferenceTime.tm_mday = 1;
	theReferenceTime.tm_year = 100;	/* Reference date is 1/1/2000 */
	theReferenceTimeSeconds = mktime(&theReferenceTime);
	theEpochReference = 
		(Unsigned32)100 * kSecsPerYear + 24 * kSecsPerDay -	
		theReferenceTimeSeconds;
	return theEpochReference;
}
#endif	/* RomPagerUseStandardTime && RomPagerCalendarTime */

/*
 	RpGetSysTimeInSeconds 
 	
 	This routine is called once during every pass of the RomPagerMainTask 
 	routine.  In most cases, the host operating system will provide some 
 	timer mechanism.  In many cases, this will be modeled after the standard 
 	C Library calls.     

	If the standard C time library routines are used, this call returns 
	the current date and time in seconds since a time base.  If calendar
	time is supported, the time base is typically 1/1/1900, otherwise
	the time base is zeroed when the system is booted. The standard time 
	routines assume the same base when creating date strings.
	
	If the Allegro provided time library routines are used, the time base 
	should be 1/1/1900 if calendar time is supported, otherwise the 
 	value should represent the number of seconds since device boot.
 		
 	Returns:	the current system time in seconds.
*/

#if (RpTargetOS == eRpTargetTestNoCal) 
static Boolean		theFirstFlag = True;
static Unsigned32	theFirstTime = 0;
#endif

#if (RpTargetOS == eRpTargetPSOS) 
static Unsigned32	gCurrentTime = 0;
static Unsigned32	gLastSeconds = 0;
#endif

Unsigned32 RpGetSysTimeInSeconds(void *theTaskDataPtr) {
	Unsigned32	theCurrentTime;
#if (RpTargetOS == eRpTargetPSOS)
	Unsigned32		theDate, theTime, theTicks, theStatus;
	Unsigned32		theSeconds, theMinutes, theHours, theDays, theYear;
	Unsigned32		theLeapYears;
#endif

	theCurrentTime = 0;
#if RomPagerUseStandardTime
	time((time_t *) &theCurrentTime);
#if RomPagerCalendarTime
	/*
		Adjust to time since 1/1/1900.
	*/
	theCurrentTime += ((rpDataPtr) theTaskDataPtr)->fEpochReference;
#endif
#elif (RpTargetOS == eRpTargetTestNoStd)
	/*
		This call returns the current date and time in seconds since 
		1/1/1904, so adjust it to a base of 1/1/1900.
	*/
	time(&theCurrentTime);					
	theCurrentTime += 4 * kSecsPerYear;		
#elif (RpTargetOS == eRpTargetTestNoCal) 
	if (theFirstFlag) {
		time(&theFirstTime);
		theFirstFlag = False;
	}
	time(&theCurrentTime);
	theCurrentTime -= theFirstTime;
#elif (RpTargetOS == eRpTargetNucleus) 
	#if RomPagerCalendarTime
		#error "Need seconds since January 1, 1900"
	#else
		theCurrentTime = NU_Retrieve_Clock()/TICKS_PER_SECOND;
	#endif
#elif (RpTargetOS == eRpTargetPrecise) 
        TIME_STRUCT  mqx_time;
#ifndef MQX_VERSION
#define _time_get_elapsed _Get_kernel_time
#define _time_get _Get_time
#endif
    #if RomPagerCalendarTime
        _time_get(&mqx_time);
        theCurrentTime = mqx_time.SECONDS+2208988800;
    #else
        _time_get_elapsed(&mqx_time);
        theCurrentTime = mqx_time.SECONDS;
    #endif
#elif (RpTargetOS == eRpTargetPSOS)
	#if RomPagerCalendarTime
		#error "Need seconds since January 1, 1901"
	#else
		theStatus = (Unsigned32) tm_get(&theDate, &theTime, &theTicks);
		theSeconds = theTime & 0x3F;
		theMinutes = (theTime >> 8) & 0x3F;
		theHours = (theTime >> 16) & 0x1F;
		theDays = (theDate & 0x3F) + (theDate >> 8) * 31;
		theYear = (theDate >> 16);
		if (theYear > 1900) {
			theYear -= 1900;
		}
		theLeapYears = theYear >> 2;
		theCurrentTime = theSeconds + (theMinutes * 60) + (theHours * 3600)
				+ theDays * 86400 + theYear * 365 * 86400;
		theCurrentTime += theLeapYears * 86400;
	#endif
#elif (RpTargetOS == eRpTargetUCOS2)
        theCurrentTime = OSTimeGet()/TICKS_PER_SECOND;
#else
	#if RomPagerCalendarTime
		#error "Need seconds since January 1, 1900"
	#else
		#error "Need seconds since system boot"
	#endif
#endif
	return theCurrentTime;
}


/*
 	RpGetMonthDayYearInSeconds 
 	
 	This routine is called at initialization to set up the rom and
 	expired dates into system time format.
 	
 		Returns:	the rom time in system time format in seconds.
*/

Unsigned32 RpGetMonthDayYearInSeconds(void *theTaskDataPtr,
										Unsigned32 theMonth,
										Unsigned32 theDay,
										Unsigned32 theYear) {
	rpDate		theTimeStruct;
	
	theTimeStruct.fSeconds = 0;	
	theTimeStruct.fMinutes = 0;	
	theTimeStruct.fHours = 0;	
	theTimeStruct.fDay = theDay;	
	theTimeStruct.fMonth = theMonth - 1;	
	theTimeStruct.fYear = theYear - 1900;	
	return RpGetDateInSeconds((rpDataPtr) theTaskDataPtr, &theTimeStruct);
}


/*
 	RpGetDateInSeconds 
 	
 	This routine is called to turn a RomPager format date structure
 	into the internal systems seconds format.
 	
		Input:		date/time in rpDate structure 	
 		Returns:	the time in system time format in seconds.
*/

Unsigned32 RpGetDateInSeconds(rpDataPtr theDataPtr, rpDatePtr theDatePtr) {
	Unsigned32	theDateInSeconds;
#if RomPagerUseStandardTime
	struct	tm	theTimeStruct;
	
	theTimeStruct.tm_sec = theDatePtr->fSeconds;	
	theTimeStruct.tm_min = theDatePtr->fMinutes;	
	theTimeStruct.tm_hour = theDatePtr->fHours;	
	theTimeStruct.tm_mday = theDatePtr->fDay;	
	theTimeStruct.tm_mon = theDatePtr->fMonth;	
	theTimeStruct.tm_year = theDatePtr->fYear;	

	theDateInSeconds = mktime(&theTimeStruct);
#if RomPagerCalendarTime
	/*
		Adjust to time since 1/1/1900.
	*/
	theDateInSeconds += theDataPtr->fEpochReference;
#endif

#else
	Unsigned32	theDays;
	/*	
		No validity checking is done on the date values, since they
		either come from internal constants or from values passed in
		from the browser.  
	*/

	theDateInSeconds = theDatePtr->fSeconds                  + 
						 theDatePtr->fMinutes  * kSecsPerMin  +
						 theDatePtr->fHours    * kSecsPerHour + 
						(theDatePtr->fDay - 1) * kSecsPerDay;
	
	theDays = gRpMonthDays[theDatePtr->fMonth];
	if (theDatePtr->fMonth > 1 && (theDatePtr->fYear % 4 == 0)) {
		/*
			Add a day for the current leap year
		*/
		theDays += 1;						
	}
	/*
		Add in the leap days for the past years.
		1900 was not a leap year.
	*/
	theDays += (theDatePtr->fYear - 1) / 4;	
	theDateInSeconds += theDays * kSecsPerDay;
	theDateInSeconds += theDatePtr->fYear * kSecsPerYear;	/* since 1900 */
#endif
	return theDateInSeconds;
}


/*
 	RpBuildDateString 
 	
 	This routine is called from various routines to convert an internal time 
 	stored in seconds into a date/time string.  In systems with support for 
 	calendar date/time, the seconds are stored internally as the seconds since 
 	January 1, 1900.  In other systems, this will be an arbitrary number of 
 	seconds based on a rom date plus number of seconds the system has run.  
 	The format of date string should correspond to HTTP/Unix system time 
 	formats.  A sample date/time in this format is:
		
		"Fri, 15 Dec 1995 12:00:00 GMT"
 		
		Input:		time in seconds 	
 		Returns:	a date/time string.
*/
 	
#if RomPagerUseStandardTime

void RpBuildDateString(rpDataPtr theDataPtr, char *theDateString,
						Unsigned32 theSeconds) {
	struct	tm		*theTimeStruct;

#if RomPagerCalendarTime
	theSeconds -= theDataPtr->fEpochReference;
#endif	/* RomPagerCalendarTime */
	
	/*
		Not all systems support the gmtime call, so we use the localtime call.
		If timezone support is added make sure that the values returned are in
		GMT in order for caching to work properly.
	*/
	theTimeStruct = localtime((time_t *) &theSeconds);
	strftime(theDateString, 80, "%a, %d %b %Y %H:%M:%S GMT", theTimeStruct);
	RP_STRCAT(theDateString, kCRLF);
	return;
}

#else

void RpBuildDateString(rpDataPtr theDataPtr, char *theDateString,
						Unsigned32 theDateSeconds) {
	Unsigned32		theDayOfYear;
	Unsigned32		theDayOfWeek;
	Unsigned32		theDayOfMonth;
	Unsigned32		theMonthIndex;
	Unsigned32		theYear;
	Unsigned32		theHours;
	Unsigned32		theMinutes;
	Unsigned32		theSeconds;
	Unsigned32			theLeapYears;
	Unsigned16 const *	theMonthDaysPtr;
	Unsigned16 const *	theMonthPtr;
		
	/*
		1900 was not a leap year - Work with 1901 as a convenient base year.
	*/
	theDateSeconds -= kSecsPerYear;

	/*
		First, compute the day of the week.
	*/
	theDayOfWeek   = (theDateSeconds / kSecsPerDay + 2) % 7;

	/*
		Break apart the seconds into date components starting with years.
		Compute the number of groups of 4 years since 1 Jan 1901 and the 
		remaining seconds (the seconds since the end of the last group).
	*/
	theLeapYears   = theDateSeconds / kSecsPerFourYears;
	theDateSeconds = theDateSeconds % kSecsPerFourYears;

	/*
		Compute the number of years since the end of the last group of 4 years.
		The remaining seconds will be more than 4 times the number of seconds
		in a year if the date is December 31 of a leap year since leap years 
		are the last of a group of 4 years. Then computine the remaining 
		seconds (the seconds since the start of the year). 
	*/
	theYear        = theDateSeconds / kSecsPerYear;
	if (theYear == 4) {
		theYear = 3;
	}
	theDateSeconds -= theYear * kSecsPerYear;

	/*
		Compute the total number of years since 1 Jan 1900 by adding 
		4 times the number of groups of 4 years since 1901 and 1 for 1900.
	*/
	theYear       += 4 * theLeapYears + 1;

	theDayOfYear   = theDateSeconds / kSecsPerDay;	
	theDateSeconds = theDateSeconds % kSecsPerDay;	

	theHours       = theDateSeconds / kSecsPerHour;	
	theDateSeconds = theDateSeconds % kSecsPerHour;	

	theMinutes     = theDateSeconds / kSecsPerMin;	
	theSeconds     = theDateSeconds % kSecsPerMin;	
	
	/*
		Handle the leap year.  2000 is a leap year, so this works.
		2100 isn't, but neither you or I will be around to worry about it then.
	*/
	theMonthDaysPtr = (theYear % 4 == 0) ? gRpMonthDaysLeapYear : gRpMonthDays;
	theMonthPtr = theMonthDaysPtr;
	while (theDayOfYear >= *theMonthPtr) {
		theMonthPtr += 1; 
	}
	theMonthPtr -= 1;
	theMonthIndex = theMonthPtr - theMonthDaysPtr;

	theDayOfMonth = theDayOfYear - *theMonthPtr + 1;
	
	/*
		Now, turn the components into a string.
	*/
	RP_STRCPY(theDateString, (char *) gRpDayName[theDayOfWeek]);
	RP_STRCAT(theDateString, kCommaSpace);

	RpCatUnsigned32ToString(theDayOfMonth, theDateString, 2);
	RP_STRCAT(theDateString, kSpace);

	RP_STRCAT(theDateString, (char *) gRpMonthTable[theMonthIndex]);
	RP_STRCAT(theDateString, kSpace);

	RpCatUnsigned32ToString(theYear + 1900, theDateString, 0);
	RP_STRCAT(theDateString, kSpace);

	RpCatUnsigned32ToString(theHours, theDateString, 2);
	RP_STRCAT(theDateString, kColon);

	RpCatUnsigned32ToString(theMinutes, theDateString, 2);
	RP_STRCAT(theDateString, kColon);

	RpCatUnsigned32ToString(theSeconds, theDateString, 2);
	RP_STRCAT(theDateString, kSpaceGMT);
	RP_STRCAT(theDateString, kCRLF);

	return;
}

#endif	/* RomPagerUseStandardTime */


// +++ _Alphanetworks_Patch_, 12/20/2003, jacob_shih
/*
	RpGetRomDateInSeconds
 	
	This routine converts the pre-defined ROM date string (2003/12/20 06:30PM)
	into the internal systems seconds format.

	Input:		pre-defined ROM date string
	Returns:	the time in system time format in seconds.
*/

#define kRomDate_Year	2003
#define kRomDate_Month	12
#define kRomDate_Day	31
#define kRomDate_Hour	7
#define kRomDate_Minute	8
Unsigned32 RpGetRomDateInSeconds(rpDataPtr theDataPtr, const char* theRomDate)
{
	asItemError	theError;
	Unsigned32	theYear;
	Unsigned32	theMonth;
	Unsigned32	theDay;
	Unsigned32	theHour;
	Unsigned32	theMinute;
	Unsigned32	theValue;
	Unsigned16	theTokenLength;
	char		theDate[kMaxStringLength];	// kMaxStringLength(64)
	char *		theDatePtr;

	RP_STRCPY(theDate, theRomDate);
	theDatePtr= theDate;

	theYear= kRomDate_Year;
	theMonth= kRomDate_Minute;
	theDay= kRomDate_Day;
	theHour= kRomDate_Hour;
	theMinute= kRomDate_Minute;

	theTokenLength= RpFindTokenDelimited(theDatePtr, kAscii_Slash);
	if(theTokenLength>0)
	{
		*(theDatePtr+theTokenLength)= '\0';
		theError = AsStrToUnsigned32(theDatePtr, &theValue);
		if (theError == eAsItemError_NoError) 
		{
			theYear= theValue;
		}
		theDatePtr= theDatePtr+theTokenLength+1;
	}

	theTokenLength= RpFindTokenDelimited(theDatePtr, kAscii_Slash);
	if(theTokenLength>0)
	{
		*(theDatePtr+theTokenLength)= '\0';
		theError = AsStrToUnsigned32(theDatePtr, &theValue);
		if (theError == eAsItemError_NoError) 
		{
			theMonth= theValue;
		}
		theDatePtr= theDatePtr+theTokenLength+1;
	}

	theTokenLength= RpFindTokenDelimited(theDatePtr, kAscii_Space);
	if(theTokenLength>0)
	{
		*(theDatePtr+theTokenLength)= '\0';
		theError = AsStrToUnsigned32(theDatePtr, &theValue);
		if (theError == eAsItemError_NoError) 
		{
			theDay= theValue;
		}
		theDatePtr= theDatePtr+theTokenLength+1;
	}

	theTokenLength= RpFindTokenDelimited(theDatePtr, kAscii_Colon);
	if(theTokenLength>0)
	{
		*(theDatePtr+theTokenLength)= '\0';
		theError = AsStrToUnsigned32(theDatePtr, &theValue);
		if (theError == eAsItemError_NoError) 
		{
			theHour= theValue;
		}
		theDatePtr= theDatePtr+theTokenLength+1;
	}
	if(RP_STRLEN(theDatePtr) > 0)
	{
		if(RP_STRLEN(theDatePtr) > 2)
		{
			if(RP_STRCMP(theDatePtr+2, "AM")==0)	// check AM/PM
			{
				if(theHour==12)		// 12:00AM...12:59AM -> 0:00AM...0:59AM
					theHour=0;
			}
			else
			{
				if(theHour!=12)		// change to 24 hours
					theHour+=12;
			}
			*(theDatePtr+2)= '\0';
		}
		theError = AsStrToUnsigned32(theDatePtr, &theValue);
		if (theError == eAsItemError_NoError) 
		{
			theMinute= theValue;
		}
	}
	theValue= RpGetMonthDayYearInSeconds(theDataPtr,
				theMonth, theDay, theYear);
	theValue+= (theMinute * 60 + theHour * 60 * 60);
	return theValue;
}
// --- _Alphanetworks_Patch_, 12/20/2003, jacob_shih
