/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

    SDK_record.c

Abstract:

    The routines of Record SDK.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2015/12/16  Toby  Create  

*/
#include "general.h"
#include "sysapi.h"
#include "asfapi.h"
#include "Adcapi.h"
#include "dcfapi.h"
#include "GlobalVariable.h"
#include "SDK_recordapi.h"


u8 Schedule_Status[7][MULTI_CHANNEL_MAX][48] = {0};
u8 Schedule_changemode[7][MULTI_CHANNEL_MAX][48] = {0};

BOOLEAN SDK_Record_InitScheduleTime(u8 mode, u8 Ch_ID, u8 Day ,u8 index)
{
    Schedule_Status[Day][Ch_ID][index] = mode;
	
	if(index==0) //Lsk: Across the night
		Schedule_changemode[Day][Ch_ID][index] = 1;
    return TRUE;
}

BOOLEAN SDK_Record_SaveScheduleTime(u8 mode, u8 Ch_ID, u8 Day ,u8 index)
{
    if(Schedule_Status[Day][Ch_ID][index] != mode)
    {
        Schedule_Status[Day][Ch_ID][index] = mode;
        Schedule_changemode[Day][Ch_ID][index] = 1;
    }
    return TRUE;
}

BOOLEAN SDK_Record_ScheduleMode(u8 mode, u8 Ch_ID, u8 Day ,u8 index)
{
    //RTC_DATE_TIME   localTime;
    int Status;
    
//    if(MultiChannelGetCaptureVideoStatus(Ch_ID+MULTI_CHANNEL_LOCAL_MAX) == 1)
//        REC_Status |= (0x1<<Ch_ID);
//    else
//        REC_Status &= ~(0x1<<Ch_ID);

	//if(index==47)	  //cross night
	//	Schedule_changemode[Day][Ch_ID][index] = 1;

    if(Schedule_changemode[Day][Ch_ID][index] == 1)
    {
      #if MULTI_CHANNEL_VIDEO_REC
        Status = MultiChannelGetCaptureVideoStatus(Ch_ID+MULTI_CHANNEL_LOCAL_MAX);
        if ((Status == 1)||(Status == 2))// 1: Recording Mode  2: Detecting Mode
        {
            VideoClipParameter[Ch_ID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
            SDK_Record_CaptureVideoStopByChannel(Ch_ID);
        }
      #endif
		
		Schedule_changemode[Day][Ch_ID][index] = 0;
    }

	#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
	if(VideoClipOption[Ch_ID].ShowDebugMsgFlag)
	    printf("##CH%d %d, %d, %d\n", Ch_ID, Day,index,Schedule_Status[Day][Ch_ID][index]);
	#endif
	
    if(Schedule_Status[Day][Ch_ID][index] == SDK_SCHEDULE_OFF)
    {
      #if MULTI_CHANNEL_VIDEO_REC
        if (MultiChannelGetCaptureVideoStatus(Ch_ID+MULTI_CHANNEL_LOCAL_MAX) == 1)
        {
            VideoClipParameter[Ch_ID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
            SDK_Record_CaptureVideoStopByChannel(Ch_ID);
        }
      #endif
    }
    else if(Schedule_Status[Day][Ch_ID][index] & SDK_SCHEDULE_REC == SDK_SCHEDULE_REC)
    {
      #if MULTI_CHANNEL_VIDEO_REC
        if((MultiChannelGetCaptureVideoStatus(Ch_ID + MULTI_CHANNEL_LOCAL_MAX) == 1) && (VideoClipParameter[Ch_ID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode == ASF_CAPTURE_EVENT_MOTION_ENA))
        {
            VideoClipParameter[Ch_ID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
            SDK_Record_CaptureVideoStopByChannel(Ch_ID);
        }
        if (MultiChannelGetCaptureVideoStatus(Ch_ID + MULTI_CHANNEL_LOCAL_MAX) == 0)
        {
            VideoClipParameter[Ch_ID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_NORMAL;
            MultiChannelSysCaptureVideoOneCh(Ch_ID + MULTI_CHANNEL_LOCAL_MAX);
        }
      #endif
    }
    else if(Schedule_Status[Day][Ch_ID][index] == SDK_SCHEDULE_MOTION)
    {
      #if MULTI_CHANNEL_VIDEO_REC
        if (MultiChannelGetCaptureVideoStatus(Ch_ID + MULTI_CHANNEL_LOCAL_MAX) == 0)
        {
            VideoClipParameter[Ch_ID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_EVENT_MOTION_ENA;
            MultiChannelSysCaptureVideoOneCh(Ch_ID + MULTI_CHANNEL_LOCAL_MAX);
        }				
      #endif
    }
    //printf("REC_Status %x\n",REC_Status);

	return TRUE;
}

BOOLEAN SDK_Record_CaptureVideoByChannel(u8 Ch_ID)
{
    u8  temp;

    if (Ch_ID >= MULTI_CHANNEL_MAX)
    {
        DEBUG_UI("Capture Video Error Channel %d\r\n",Ch_ID);
        return FALSE;
    }
    if ((MemoryFullFlag == TRUE) && (SysOverwriteFlag == FALSE))
    {
        DEBUG_UI("Ch %d SD Card FULL!!!!!\r\n",Ch_ID);
        return FALSE;
    }
    if((sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY))
    {
        DEBUG_UI("SDK_Record_CaptureVideoByChannel %d\r\n",Ch_ID);
      #if MULTI_CHANNEL_VIDEO_REC
        //if (MultiChannelGetCaptureVideoStatus(Ch_ID+MULTI_CHANNEL_LOCAL_MAX) != 0)
        //    SDK_Record_CaptureVideoStopByChannel(Ch_ID);
        temp = (u8)MultiChannelSysCaptureVideoOneCh(Ch_ID + MULTI_CHANNEL_LOCAL_MAX);
      #endif
        return temp;
    }
    else
    {
        DEBUG_UI("No SD Card\r\n");
        return FALSE;
    }
}

BOOLEAN SDK_Record_CaptureVideoStopByChannel(u8 Ch_ID)
{
    if (Ch_ID >= MULTI_CHANNEL_MAX)
    {
        DEBUG_UI("Stop Capture Video Error Channel %d\r\n",Ch_ID);
        return FALSE;
    }

    DEBUG_UI("SDK_Record_CaptureVideoStopByChannel %d\r\n",Ch_ID);
  #if MULTI_CHANNEL_VIDEO_REC
    if ( MultiChannelSysCaptureVideoStopOneCh(Ch_ID+MULTI_CHANNEL_LOCAL_MAX) == 1)
        return TRUE;
  #endif

    return FALSE;
}

