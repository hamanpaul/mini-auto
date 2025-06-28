/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	SDK record.h

Abstract:

   	The declarations of SDK record.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2015/12/16  Toby  Create  

*/

#ifndef __SDK_RECORD_API__
#define __SDK_RECORD_API__

extern u8 Schedule_Status[7][MULTI_CHANNEL_MAX][48];
extern u8 Schedule_changemode[7][MULTI_CHANNEL_MAX][48];

extern BOOLEAN SDK_Record_InitScheduleTime(u8 mode, u8 Ch_ID, u8 Day ,u8 index);
extern BOOLEAN SDK_Record_SaveScheduleTime(u8 mode, u8 Ch_ID, u8 Day ,u8 index);
extern BOOLEAN SDK_Record_CaptureVideoByChannel(u8 Ch_ID);
extern BOOLEAN SDK_Record_CaptureVideoStopByChannel(u8 Ch_ID);
extern BOOLEAN SDK_Record_ScheduleMode(u8 mode, u8 Ch_ID, u8 Day ,u8 index);

#define SDK_SCHEDULE_OFF 	0x00
#define SDK_SCHEDULE_REC 	0x01
#define SDK_SCHEDULE_MOTION 0x02
#endif
