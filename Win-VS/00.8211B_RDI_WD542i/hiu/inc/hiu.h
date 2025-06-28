/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	hi.h

Abstract:

   	The declarations of host interface.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __HIU_H__
#define __HIU_H__

/* Type definition */

typedef struct _HIU_CFG {
	u8 		cfg;
} HIU_CFG;

/* SW 1003 S */
typedef struct _HIU_TIME_CFG
{
	u16 CurYear;
	u8 CurMonth;
	u8 CurDate;
	u8 CurHour;
	u8 CurMin;
	u8 CurSec;
}HIU_TIME_CFG;
/* SW 1003 E */



#endif

