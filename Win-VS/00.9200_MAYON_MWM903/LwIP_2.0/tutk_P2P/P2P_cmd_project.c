/*

    Copyright (c) 2008 Mars Semiconductor Corp.

    Module Name:

        P2P_cmd_project.c

    Abstract:

        The routines of user interface action.

    Environment:

        ARM RealView Developer Suite

    Revision History:

        2018/07/13   Hank Wang  Create

*/

#include "general.h"
#include "rfiuapi.h"
#include "sysapi.h"
#include "ispapi.h"
#include "fsapi.h"
#include "dcfapi.h"
#if ((TUTK_SUPPORT) && !ICOMMWIFI_SUPPORT)
#include "TUTKIOTCAPI.h"
#include "AVAPIs.h"
#include "AVIOCTRLDEFs.h"
#include "uiapi.h"
#endif
#include "rtcapi.h"
#include "..\..\ui\inc\uiact_project.h"
/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */

extern u8 *gUID;
extern s8  BoxExtKey;
extern u32 gPairAvIndex;
extern u32 gSystemStorageReady;
extern RTC_DATE_TIME SetTime;
extern RTC_TIME_ZONE SetZone;
extern UI_NET_INFO UINetInfo;
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

void P2PCmd_SetCamPair(int avIndex, char *buf);
void P2PCmd_GetCamLinkStatus(int avIndex, char *buf);
void P2PCmd_SetCamOnOff(int avIndex, char *buf);
void P2PCmd_GetCamBrightness(int avIndex, char *buf);
void P2PCmd_SetCamBrightness(int avIndex, char *buf);
void P2PCmd_GetCamFrequence(int avIndex, char *buf);
void P2PCmd_SetCamFrequence(int avIndex, char *buf);
void P2PCmd_GetCamRecStatus(int avIndex, char *buf);
void P2PCmd_GetLiveDataRate(char *buf); /*Func undefine*/
void P2PCmd_SetCamManualRec(int avIndex, char *buf);
void P2PCmd_GetCamMotionSensitivity(int avIndex, char *buf);
void P2PCmd_SetCamMotionSensitivity(int avIndex, char *buf);
void P2PCmd_GetCamRecScheduleTable(int avIndex, char *buf); /*Func Need to Chk*/
void P2PCmd_SetCamRecScheduleTable(int avIndex, char *buf); /*Func Need to Chk*/
void P2PCmd_GetRecSection(int avIndex, char *buf);
void P2PCmd_SetRecSection(int avIndex, char *buf);
void P2PCmd_SetFormat(int avIndex, char *buf); /*iconflag remove Chk*/
void P2PCmd_GetOverwiteStatus(int avIndex, char *buf);
void P2PCmd_SetOverwite(int avIndex, char *buf);
void P2PCmd_GetDateTime(int avIndex, char *buf); /*Func Need to Chk*/
void P2PCmd_SetDateTime(int avIndex, char *buf); /*Func Need to Chk*/
void P2PCmd_SetDefault(int avIndex, char *buf); /*Func Need to Chk, iconflag remove Chk*/
void P2PCmd_GetLanguage(int avIndex, char *buf);
void P2PCmd_SetLanguage(int avIndex, char *buf);
void P2PCmd_GetNetworkInfo(int avIndex, char *buf);
void P2PCmd_SetNetworkInfo(int avIndex, char *buf); /*Func undefine*/
void P2PCmd_GetLEDStatus(int avIndex, char *buf); /*Func Need to Chk*/
void P2PCmd_SetLEDStatus(int avIndex, char *buf); /*Func Need to Chk*/
void P2PCmd_GetPlaybackFPS(int avIndex, char *buf); /*Func undefine*/

/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */
#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetCamPair(int avIndex, char *buf)
{
    int size =	sizeof(SMsgAVIoctrlSetCameraPairResp);
    SMsgAVIoctrlSetCameraPairReq *p = (SMsgAVIoctrlSetCameraPairReq *)buf;
    SMsgAVIoctrlSetCameraPairResp *q = (SMsgAVIoctrlSetCameraPairResp *)buf;

#if (HOME_RF_SUPPORT)
    gPairAvIndex = avIndex;
#endif

    q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_CAM_PAIR, UI_INVALID_VAL) == 0)? 1 : 0;
    DEBUG_P2P("IOTYPE_WALMART_SETCAMERAPAIR_REQ OK:%d %d\n\n", p->channel, q->result);
    
    #if 0
    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETCAMERAPAIR_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_SETCAMERAPAIR_RESP OK\n\n");
    }
    #endif
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetCamLinkStatus(int avIndex, char *buf)
{
    int size = sizeof(SMsgAVIoctrlGetCameraStatusResp);
    SMsgAVIoctrlGetCameraStatusReq *p = (SMsgAVIoctrlGetCameraStatusReq *)buf;
    SMsgAVIoctrlGetCameraStatusResp *q = (SMsgAVIoctrlGetCameraStatusResp *)buf;
    u8 value[4]; 
    u8 i, result;

    //DEBUG_YELLOW("%d %s %d\n",__LINE__, __FILE__, rfiuRX_CamPair_Sta);
    //DEBUG_YELLOW("%d %s %d\n",__LINE__, __FILE__, rfiuRX_CamOnOff_Sta);
    for (i = 0;i < 4; i++)
    {
        if (uiPairing[i] == TRUE)
        {
            result = 0;
        }
        else
        {
            uiFlowGetUISetting(i, UI_KEY_CAM_ON, &result);
        }
        
        if (((rfiuRX_CamPair_Sta >> i) & 0x01) && (result == 1))
        {
            value[i] = 1;
        }
        else
        {
            value[i] = 0;
        }
    }
    //DEBUG_YELLOW("%d %s %d %d %d %d\n",__LINE__, __FILE__, value[0],value[1],value[2],value[3]);

    q->channel1[0] = value[0];
    q->channel2[0] = value[1];
    q->channel3[0] = value[2];
    q->channel4[0] = value[3];

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETCAMERASTASUS_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
        DEBUG_P2P("IOTYPE_WALMART_GETCAMERASTASUS_RESP OK\n\n");
    }
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetCamOnOff(int avIndex, char *buf)
{
    int size = sizeof(SMsgAVIoctrlSetCameraStatusResp);
    SMsgAVIoctrlSetCameraStatusReq *p = (SMsgAVIoctrlSetCameraStatusReq *)buf;
    SMsgAVIoctrlSetCameraStatusResp *q = (SMsgAVIoctrlSetCameraStatusResp *)buf;

    q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_CAM_ON, p->status) == 0)? 1 : 0;
    DEBUG_P2P("IOTYPE_WALMART_SETCAMERASTASUS_REQ OK,%d %d\n\n",p->channel, q->result);

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETCAMERASTASUS_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_SETCAMERASTASUS_RESP OK\n\n");
    }	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetCamBrightness(int avIndex, char *buf)
{
    int size = sizeof(SMsgAVIoctrlGetCameraBrightnessResp);
    SMsgAVIoctrlGetCameraBrightnessReq *p = (SMsgAVIoctrlGetCameraBrightnessReq *)buf;
    SMsgAVIoctrlGetCameraBrightnessResp *q = (SMsgAVIoctrlGetCameraBrightnessResp *)buf;

    DEBUG_P2P("IOTYPE_WALMART_GETCAMERABRIT_REQ OK:%d\n\n",p->channel);

    uiFlowGetUISetting(p->channel, UI_KEY_CAM_BRIGHTNESS, &(q->status));

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETCAMERABRIT_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
        DEBUG_P2P("IOTYPE_WALMART_GETCAMERABRIT_RESP OK\n\n");
    }	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetCamBrightness(int avIndex, char *buf)
{
	int size = sizeof(SMsgAVIoctrlSetCameraBrightnessResp);
	SMsgAVIoctrlSetCameraBrightnessReq *p = (SMsgAVIoctrlSetCameraBrightnessReq *)buf;
	SMsgAVIoctrlSetCameraBrightnessResp *q = (SMsgAVIoctrlSetCameraBrightnessResp *)buf;

	q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_CAM_BRIGHTNESS, p->status) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETCAMERABRIT_REQ OK:%d %d\n\n",p->channel,q->result);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETCAMERABRIT_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETCAMERABRIT_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetCamFrequence(int avIndex, char *buf)
{
    int size =sizeof(SMsgAVIoctrlGetCameraFreqResp);
    SMsgAVIoctrlGetCameraFreqReq *p = (SMsgAVIoctrlGetCameraFreqReq *)buf;
    SMsgAVIoctrlGetCameraFreqResp *q = (SMsgAVIoctrlGetCameraFreqResp *)buf;

    DEBUG_P2P("IOTYPE_WALMART_GETCAMERAFREQ_REQ OK\n\n");

    uiFlowGetUISetting(UI_INVALID_VAL, UI_KEY_CAM_FLICKER, &(q->status));

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETCAMERAFREQ_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
        DEBUG_P2P("IOTYPE_WALMART_GETCAMERAFREQ_RESP OK\n\n");
    }
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetCamFrequence(int avIndex, char *buf)
{
    int size = sizeof(SMsgAVIoctrlSetCameraFreqResp);
    SMsgAVIoctrlSetCameraFreqReq *p = (SMsgAVIoctrlSetCameraFreqReq *)buf;
    SMsgAVIoctrlSetCameraFreqResp *q = (SMsgAVIoctrlSetCameraFreqResp *)buf;

    q->result = (uiFlowSetCmdToUI(UI_INVALID_VAL , UI_KEY_CAM_FLICKER, p->status) == 0)? 1 : 0;
    DEBUG_P2P("IOTYPE_WALMART_SETCAMERAFREQ_REQ %d\n\n", q->result);

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETCAMERAFREQ_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_SETCAMERAFREQ_RESP OK\n\n");
    }
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetCamRecStatus(int avIndex, char *buf)
{
    int size =sizeof(SMsgAVIoctrlGetManualRecordResp);
    SMsgAVIoctrlGetManualRecordReq *p = (SMsgAVIoctrlGetManualRecordReq *)buf;
    SMsgAVIoctrlGetManualRecordResp *q = (SMsgAVIoctrlGetManualRecordResp *)buf;

    DEBUG_P2P("IOTYPE_WALMART_GETRECORDMANUAL_REQ OK\n\n");

    q->status = (MultiChannelGetCaptureVideoStatus(p->channel) == 1) ? 1 : 0;

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETRECORDMANUAL_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_GETRECORDMANUAL_RESP OK\n\n");
    }	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetLiveDataRate(char *buf)
{
	int size =	sizeof(SMsgAVIoctrlGetLiveViewDataRateReq);
	SMsgAVIoctrlGetLiveViewDataRateReq *p = (SMsgAVIoctrlGetLiveViewDataRateReq *)buf;

	DEBUG_P2P("IOTYPE_WALMART_GETLIVEVIEWDATARATE_REQ OKl bitRate=%d, index=%d\n\n", p->bitRate, p->lastFrameIndex);

}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetCamManualRec(int avIndex, char *buf)
{
	int size = sizeof(SMsgAVIoctrlSetManualRecordResp);
	SMsgAVIoctrlSetManualRecordReq *p = (SMsgAVIoctrlSetManualRecordReq *)buf;
	SMsgAVIoctrlSetManualRecordResp *q = (SMsgAVIoctrlSetManualRecordResp *)buf;

	if (gSystemStorageReady == 1)
	{
	    q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_CAM_MANUAL_REC, p->status) == 0)? 1 : 0;
	}
	else
    {
        q->result = 1; //SD card Not ready.
    }

	DEBUG_P2P("IOTYPE_WALMART_SETRECORDMANUAL_REQ OK %d\n\n",p->status);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETRECORDMANUAL_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETRECORDMANUAL_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetCamMotionSensitivity(int avIndex, char *buf)
{
	int size =sizeof(SMsgAVIoctrlGetCameraMotionLevelResp);
	SMsgAVIoctrlGetCameraMotionLevelReq *p = (SMsgAVIoctrlGetCameraMotionLevelReq *)buf;
	SMsgAVIoctrlGetCameraMotionLevelResp *q = (SMsgAVIoctrlGetCameraMotionLevelResp *)buf;

	DEBUG_P2P("IOTYPE_WALMART_GETRECORDMOTIONLEV_REQ OK\n\n");

    uiFlowGetUISetting(p->channel, UI_KEY_CAM_SENSITIVITY, &(q->status));

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETRECORDMOTIONLEV_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETRECORDMOTIONLEV_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetCamMotionSensitivity(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetCameraMotionLevelResp);
	SMsgAVIoctrlSetCameraMotionLevelReq *p = (SMsgAVIoctrlSetCameraMotionLevelReq *)buf;
	SMsgAVIoctrlSetCameraMotionLevelResp *q = (SMsgAVIoctrlSetCameraMotionLevelResp *)buf;

	q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_CAM_SENSITIVITY, p->status) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETRECORDMOTIONLEV_REQ %d\n\n", q->result);
    
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETRECORDMOTIONLEV_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETRECORDMOTIONLEV_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetCamRecScheduleTable(int avIndex, char *buf)
{
    u8 k, i, week;
    u16 j;
    int size =	sizeof(SMsgAVIoctrlGetCameraRecordScheduleResp);
    SMsgAVIoctrlGetCameraRecordScheduleReq *p = (SMsgAVIoctrlGetCameraRecordScheduleReq *)buf;
    SMsgAVIoctrlGetCameraRecordScheduleResp *q = (SMsgAVIoctrlGetCameraRecordScheduleResp *)buf;

    DEBUG_P2P("IOTYPE_WALMART_GETRECORDSCHEDULE_REQ OK,%d\n\n",p->channel);

    q->channel = p->channel; 

    i = 0;
    k = 0;
    for (j = 0; j < 336; j++)
    {
        if (i == 0)
        {
            week = 6;
        }
        else
        {
            week = i - 1;
        }
        
    	q->status[j] = uiScheduleTime[week][p->channel][k]; 
        k++;
        if (k == 48)
        {
            i++;
            k = 0;
        }
    }

	#if 0
	for(j=0;j<7;j++)
	{
		for(k=0;k<UI_SCHEDULE_HOUR;k++)
			DEBUG_GREEN("%d %d value %d\n",j,k,uiScheduleTime[j][p->channel][k]);
	}
    #endif
    
    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETRECORDSCHEDULE_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_GETRECORDSCHEDULE_RESP OK\n\n");
    }	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetCamRecScheduleTable(int avIndex, char *buf)
{
    u8 k, i, week;
    u16 j;
	int size =	sizeof(SMsgAVIoctrlSetCameraRecordScheduleResp);
	SMsgAVIoctrlSetCameraRecordScheduleReq *p = (SMsgAVIoctrlSetCameraRecordScheduleReq *)buf;
	SMsgAVIoctrlSetCameraRecordScheduleResp *q = (SMsgAVIoctrlSetCameraRecordScheduleResp *)buf;

	DEBUG_P2P("IOTYPE_WALMART_SETRECORDSCHEDULE_REQ OK,%d\n\n",p->channel);

    i = 0;
    k = 0;
	for (j = 0; j < 336; j++)
	{
	    if (i == 0)
        {
            week = 6;
        }
        else
        {
            week = i - 1;
        }
        
		uiScheduleTime[week][p->channel][k] = p->status[j];
        k++;
        if (k == 48)
        {
            i++;
            k = 0;
        }
	}
	
	#if 0
	for(j=0;j<7;j++)
	{
		for(k=0;k<UI_SCHEDULE_HOUR;k++)
			DEBUG_GREEN("%d %d value %d\n",j,k,uiScheduleTime[j][p->channel][k]);
	}
	#endif

	q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_CAM_SCHEDULE, UI_INVALID_VAL) == 0)? 1 : 0;

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETRECORDSCHEDULE_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETRECORDSCHEDULE_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetRecSection(int avIndex, char *buf)
{
	int size = sizeof(SMsgAVIoctrlGetCameraRecordDurationResp);
	SMsgAVIoctrlGetCameraRecordDurationResp *q = (SMsgAVIoctrlGetCameraRecordDurationResp *)buf;

	DEBUG_P2P("IOTYPE_WALMART_GETRECORDDURATION_REQ OK\n\n");

    uiFlowGetUISetting(UI_INVALID_VAL, UI_KEY_CAM_INTERVAL, &(q->status));

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETRECORDDURATION_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETRECORDDURATION_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetRecSection(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetCameraRecordDurationResp);
	SMsgAVIoctrlSetCameraRecordDurationReq *p = (SMsgAVIoctrlSetCameraRecordDurationReq *)buf;
	SMsgAVIoctrlSetCameraRecordDurationResp *q = (SMsgAVIoctrlSetCameraRecordDurationResp *)buf;

	q->result = (uiFlowSetCmdToUI(UI_INVALID_VAL, UI_KEY_CAM_INTERVAL, p->status) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETRECORDDURATION_REQ OK %d\n\n", q->result);
    
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETRECORDDURATION_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETRECORDDURATION_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetFormat(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetCameraSDFormatResp);
	SMsgAVIoctrlSetCameraSDFormatResp *q = (SMsgAVIoctrlSetCameraSDFormatResp *)buf;

	// Stop action of Recording.
	uiCaptureVideoStop();
#if MULTI_CHANNEL_VIDEO_REC
	while(MultiChannelCheckRecordChannel() != 0)
	{
		OSTimeDly(1);
	}
#endif
#if 0	// Sys task evt
	q->result = (uiFlowSetCmdToUI(UI_INVALID_VAL, UI_KEY_SD_FORMAT, UI_INVALID_VAL) == 0)? 1 : 0;
#else	// Execute evt directly
	q->result = sysPlaybackFormat();
#endif	
	DEBUG_P2P("IOTYPE_WALMART_SETSDFORMAT_REQ OK %d\n\n", q->result);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETSDFORMAT_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETSDFORMAT_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetOverwiteStatus(int avIndex, char *buf)
{
	int size = sizeof(SMsgAVIoctrlGetCameraSDOverWriteResp);
	SMsgAVIoctrlGetCameraSDOverWriteResp *q = (SMsgAVIoctrlGetCameraSDOverWriteResp *)buf;

	DEBUG_P2P("IOTYPE_WALMART_GETSDOVERWRITE_REQ OK\n\n");

    uiFlowGetUISetting(UI_INVALID_VAL, UI_KEY_SD_OVERWRITE, &(q->status));

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETSDOVERWRITE_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETSDOVERWRITE_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetOverwite(int avIndex, char *buf)
{
	int size = sizeof(SMsgAVIoctrlSetCameraSDOverWriteResp);
	SMsgAVIoctrlSetCameraSDOverWriteReq *p = (SMsgAVIoctrlSetCameraSDOverWriteReq *)buf;
	SMsgAVIoctrlSetCameraSDOverWriteResp *q = (SMsgAVIoctrlSetCameraSDOverWriteResp *)buf;

	q->result = (uiFlowSetCmdToUI(UI_INVALID_VAL, UI_KEY_SD_OVERWRITE, p->status) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETSDOVERWRITE_REQ OK %d\n\n", q->result);
    
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETSDOVERWRITE_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETSDOVERWRITE_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetDateTime(int avIndex, char *buf)
{
	int size = sizeof(SMsgAVIoctrlGetCameraSystemTimeResp);
	SMsgAVIoctrlGetCameraSystemTimeResp *q = (SMsgAVIoctrlGetCameraSystemTimeResp *)buf;
	RTC_DATE_TIME localTime;

	DEBUG_P2P("IOTYPE_WALMART_GETSYSTIME_REQ OK\n\n");

	RTC_Get_Time(&localTime);		
	q->time[0] = localTime.year;
	q->time[1] = localTime.month;
	q->time[2] = localTime.day;
	q->time[3] = localTime.hour;
	q->time[4] = localTime.min;
	//DEBUG_GREEN("%d %d %d %d %d\n\n",localTime.year,localTime.month,localTime.day,localTime.hour,localTime.min);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETSYSTIME_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETSYSTIME_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetDateTime(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetCameraSystemTimeResp);
	SMsgAVIoctrlSetCameraSystemTimeReq *p = (SMsgAVIoctrlSetCameraSystemTimeReq *)buf;
	SMsgAVIoctrlSetCameraSystemTimeResp *q = (SMsgAVIoctrlSetCameraSystemTimeResp *)buf;
	//DEBUG_GREEN("%d %d %d %d %d\n\n",p->time[0],p->time[1],p->time[2],p->time[3],p->time[4]);

	SetTime.year 	= p->time[0] - 30;
	SetTime.month 	= p->time[1];
	SetTime.day		= p->time[2];
	SetTime.hour	= p->time[3];
	SetTime.min		= p->time[4];
	SetTime.sec		= 0;
	//DEBUG_GREEN("%d %d %d %d %d\n\n",SetTime.year,SetTime.month,SetTime.day,SetTime.hour,SetTime.min);
	q->result = (uiFlowSetCmdToUI(UI_INVALID_VAL, UI_KEY_SYS_TIME, UI_INVALID_VAL) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETSYSTIME_REQ %d\n\n", q->result);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETSYSTIME_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETSYSTIME_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetDefault(int avIndex, char *buf)
{
	int size = sizeof(SMsgAVIoctrlSetCameraSystemTimeResp);
	SMsgAVIoctrlSetCameraDefaultResp *q = (SMsgAVIoctrlSetCameraDefaultResp *)buf;

	q->result = (uiFlowSetCmdToUI(UI_INVALID_VAL, UI_KEY_SYS_DEFAULT, UI_INVALID_VAL) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETDEFAULT_REQ OK %d\n\n", q->result);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETDEFAULT_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETDEFAULT_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetLanguage(int avIndex, char *buf)
{
    int size = sizeof(SMsgAVIoctrlGetCameraLanguageResp);
    SMsgAVIoctrlGetCameraLanguageResp *q = (SMsgAVIoctrlGetCameraLanguageResp *)buf;

    DEBUG_P2P("IOTYPE_WALMART_GETLANG_REQ OK\n\n");

    uiFlowGetUISetting(UI_INVALID_VAL, UI_KEY_SYS_LANGUAGE, &(q->status));

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETLANG_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_GETLANG_RESP OK\n\n");
    }	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetLanguage(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetCameraLanguageResp);
	SMsgAVIoctrlSetCameraLanguageReq *p=(SMsgAVIoctrlSetCameraLanguageReq *)buf;
	SMsgAVIoctrlSetCameraLanguageResp *q=(SMsgAVIoctrlSetCameraLanguageResp *)buf;

	q->result = (uiFlowSetCmdToUI(UI_INVALID_VAL, UI_KEY_SYS_LANGUAGE, p->status) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETLANG_REQ OK %d\n\n", q->result);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETLANG_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETLANG_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetNetworkInfo(int avIndex, char *buf)
{
	int size = sizeof(SMsgAVIoctrlGetCameraNetStatusResp);
	SMsgAVIoctrlGetCameraNetStatusResp *q = (SMsgAVIoctrlGetCameraNetStatusResp *)buf;
	UI_NET_INFO info;

	DEBUG_P2P("IOTYPE_WALMART_GETNETSTATUS_REQ OK\n\n");

	GetNetworkInfo(&info);
	sprintf(q->ipAddress,"%d.%d.%d.%d", info.IPaddr[0], info.IPaddr[1], info.IPaddr[2], info.IPaddr[3]);
	sprintf(q->gateway,"%d.%d.%d.%d", info.Gateway[0], info.Gateway[1], info.Gateway[2], info.Gateway[3]);		
	sprintf(q->mask,"%d.%d.%d.%d", info.Netmask[0], info.Netmask[1], info.Netmask[2], info.Netmask[3]);
	sprintf(q->MAC,"%02X-%02X-%02X-%02X-%02X-%02X", uiMACAddr[0], uiMACAddr[1], uiMACAddr[2], uiMACAddr[3], uiMACAddr[4], uiMACAddr[5]);
	sprintf(q->UID,"%s", gUID);

	memcpy(&q->RX_fw_version, &uiVersion, sizeof(uiVersion));
	memcpy(&q->TX1_fw_version, &gRfiuUnitCntl[0].RFpara.TxCodeVersion, strlen(gRfiuUnitCntl[0].RFpara.TxCodeVersion));
	memcpy(&q->TX2_fw_version, &gRfiuUnitCntl[1].RFpara.TxCodeVersion, strlen(gRfiuUnitCntl[1].RFpara.TxCodeVersion));
	memcpy(&q->TX3_fw_version, &gRfiuUnitCntl[2].RFpara.TxCodeVersion, strlen(gRfiuUnitCntl[2].RFpara.TxCodeVersion));
	memcpy(&q->TX4_fw_version, &gRfiuUnitCntl[3].RFpara.TxCodeVersion, strlen(gRfiuUnitCntl[3].RFpara.TxCodeVersion));

	DEBUG_P2P("IP=%s\n", q->ipAddress);
	DEBUG_P2P("Netmask=%s\n", q->mask);
	DEBUG_P2P("Gateway=%s\n", q->gateway);
	DEBUG_P2P("MAC=%s\n", q->MAC);
	DEBUG_P2P("UID=%s\n", q->UID);
	DEBUG_P2P("RX=%s\n", q->RX_fw_version);
	DEBUG_P2P("TX1=%s\n", q->TX1_fw_version);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETNETSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETNETSTATUS_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetNetworkInfo(int avIndex, char *buf)
{
    u32  ip[4], i;
	int size = sizeof(SMsgAVIoctrlSetCameraNetStatusResp);
	SMsgAVIoctrlSetCameraNetStatusReq *p = (SMsgAVIoctrlSetCameraNetStatusReq *)buf;
	SMsgAVIoctrlSetCameraNetStatusResp *q = (SMsgAVIoctrlSetCameraNetStatusResp *)buf;

	DEBUG_P2P("IOTYPE_WALMART_SETNETSTATUS_REQ OK\n\n");
    if (p->mode == 1)
    {
        sscanf((char*)p->ipAddress, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);
        for (i = 0; i < 4; i++)
        {
            if (ip[i] > 255)
            {
                DEBUG_P2P("IP Address %d Error value %u!!!\n", i, ip[i]);
            }
            else
                UINetInfo.IPaddr[i] = ip[i];
        }
        sscanf((char*)p->gateway, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);
        for (i = 0; i < 4; i++)
        {
            if (ip[i] > 255)
            {
                DEBUG_P2P("Gateway Address %d Error value %u!!!\n", i, ip[i]);
            }
            else
                UINetInfo.Gateway[i] = ip[i];
        }
        sscanf((char*)p->mask, "%u.%u.%u.%u", &ip[0], &ip[1], &ip[2], &ip[3]);
        for (i = 0; i < 4; i++)
        {
            if (ip[i] > 255)
            {
                DEBUG_P2P("Mask Address %d Error value %u!!!\n", i, ip[i]);
            }
            else
                UINetInfo.Netmask[i] = ip[i];
        }
    }
	q->result = (uiFlowSetCmdToUI(UI_INVALID_VAL, UI_KEY_SYS_NET_SET, p->mode) == 0)? 1 : 0;
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETNETSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETNETSTATUS_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetLEDStatus(int avIndex, char *buf)
{
    u8 LED_level;
	int size = sizeof(SMsgAVIoctrlGetLEDResq);
	SMsgAVIoctrlGetLEDResq *q = (SMsgAVIoctrlGetLEDResq *)buf;

	DEBUG_P2P("IOTYPE_WALMART_SETLED_REQ OK\n\n");

	//gpioGetLevel(0, 31, &LED_level);
	//q->status = LED_level;
	
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETLED_RESQ, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETLED_RESQ OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetLEDStatus(int avIndex, char *buf)
{
	int size = sizeof(SMsgAVIoctrlSetLEDResq);
	SMsgAVIoctrlSetLEDReq *p = (SMsgAVIoctrlSetLEDReq *)buf;
	SMsgAVIoctrlSetLEDResq *q = (SMsgAVIoctrlSetLEDResq *)buf;

	DEBUG_P2P("IOTYPE_WALMART_SETLED_REQ OK\n\n");

	//q->result = !(gpioSetLevel(0, 31, p->status));
	
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETLED_RESQ, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETLED_RESQ OK\n\n");
	}	
}
#endif

void P2PCmd_GetPlaybackFPS(int avIndex, char *buf)
{	
	SMsgAVIoctrlGetPlayBackFPSReq *p = (SMsgAVIoctrlGetPlayBackFPSReq *)buf;

	DEBUG_P2P("IOTYPE_WALMART_GETPLAYBACKFPS_REQ OK:%d\n\n", p->FPS);

//			app_report_fps = p->FPS;
}


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetTimeZone(int avIndex, char *buf)
{	
    int size =  sizeof(SMsgAVIoctrlGetTimeZoneResp);
    SMsgAVIoctrlGetTimeZoneResp *q = (SMsgAVIoctrlGetTimeZoneResp *)buf;
    RTC_TIME_ZONE zone;
    //char timezone_des[256]="Taiwan";
    
    DEBUG_P2P("IOTYPE_WALMART_GETTIMEZONE_REQ OK \n");
    
    RTC_Get_TimeZone(&zone);
    q->t_operator = zone.operator;
    q->hour= zone.hour;
    q->min = zone.min;
    uiFlowGetUISetting(UI_INVALID_VAL, UI_KEY_SYS_TIME_ZONE, &(q->index));
    OSTimeDly(2);
    uiFlowGetUISetting(UI_INVALID_VAL, UI_KEY_SYS_DST, &(q->dst));
    //DEBUG_GREEN("IOTYPE_WALMART_SETTIMEZONE_REQ %d %d %d %d\n",  q->t_operator, q->hour ,q->min ,q->index);

    if(avSendIOCtrl(avIndex, IOTYPE_WALMART_GETTIMEZON_RESP, (char *)q, size)== AV_ER_NoERROR)
    {
        DEBUG_P2P("IOTYPE_WALMART_GETTIMEZONE_RESP OK\n\n");
    }
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetTimeZone(int avIndex, char *buf)
{	
    int size =  sizeof(SMsgAVIoctrlSetTimeZoneResp);
    SMsgAVIoctrlSetTimeZoneReq *p = (SMsgAVIoctrlSetTimeZoneReq *)buf;
	SMsgAVIoctrlSetTimeZoneResp *q = (SMsgAVIoctrlSetTimeZoneResp *)buf;
    
    SetZone.operator= p->t_operator;
    SetZone.hour = p->hour;
    SetZone.min = p->min;
    q->result = (uiFlowSetCmdToUI(UI_INVALID_VAL, UI_KEY_SYS_TIME_ZONE, p->index) == 0)? 1 : 0;
    OSTimeDly(2);
    q->result = uiFlowSetCmdToUI(UI_INVALID_VAL, UI_KEY_SYS_DST, p->dst);
    DEBUG_P2P("IOTYPE_WALMART_SETTIMEZONE_REQ %d\n", q->result);//only timezone, q->result = 1 success
    
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETTIMEZONE_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETTIMEZONE_RESP OK\n\n");
	}	
}
#endif


void P2PCmd_GetFwUpgrage(int avIndex, char *buf)
{
    u8 UpgradeInfoBuf[1024];
    u8 Src;
    u32 ReadSize, StorageIndex;
    int size = sizeof(SMsgAVIoctrlGetFWUpgrageResp);
    SMsgAVIoctrlGetFWUpgrageReq *p = (SMsgAVIoctrlGetFWUpgrageReq *)buf;
    SMsgAVIoctrlGetFWUpgrageResp *q = (SMsgAVIoctrlGetFWUpgrageResp *)buf;
    
    Src = p->Source;
    memset(q, 0x0, sizeof(SMsgAVIoctrlGetFWUpgrageResp));
    
#if ISP_NEW_UPGRADE_FLOW_SUPPORT

    q->BitList = 0;
    switch(Src)
    {
        case 0: // SD
            StorageIndex = sysGetStorageIndex(SYS_V_STORAGE_SDC);
            if(sysGetStorageStatus(StorageIndex) == SYS_V_STORAGE_NREADY)
            {
                q->status = 0;
                break;
            }
            else
            {
                q->status = 1;
            }

            if(StorageIndex == SYS_I_STORAGE_MAIN)
            {
                if(dcfItemExist(ispUSBFileName, "rb") == 1)
                {
                    strcpy(q->RX_fw_version, ispUSBFileName);
                	if(strlen(q->RX_fw_version) > 0)
                		q->BitList |= (1 << 0);
                }

                if(dcfItemExist(ispTxFWFileName[0], "rb") == 1)
                {
                    strcpy(q->TX1_fw_version, ispTxFWFileName[0]);
                    q->BitList |= (1 << 1);
                }

                if(dcfItemExist(ispTxFWFileName[1], "rb") == 1)
                {
                    strcpy(q->TX2_fw_version, ispTxFWFileName[1]);
                    q->BitList |= (1 << 2);
                }

                if(dcfItemExist(ispTxFWFileName[2], "rb") == 1)
                {
                    strcpy(q->TX3_fw_version, ispTxFWFileName[2]);
                    q->BitList |= (1 << 3);
                }

                if(dcfItemExist(ispTxFWFileName[3], "rb") == 1)
                {
                    strcpy(q->TX4_fw_version, ispTxFWFileName[3]);
                    q->BitList |= (1 << 4);
                }
            }            
            
            break;
            
        case 1: // Net
            if(ispUpdateFirmwareVersion(0x1f) != 1)
        	{
        		DEBUG_ISP("[W] P2P get firmware version info failed.\n");
        		q->status = 0;
        		break;
        	}
        	else
        	{
        	    q->status = 1;
        	}

        	strcpy(q->RX_fw_version, ispGetFWVersionStr(0));
        	if(strlen(q->RX_fw_version) > 0)
        		q->BitList |= (1 << 0);
        		
        	strcpy(q->TX1_fw_version, ispGetFWVersionStr(1));
        	if(strlen(q->TX1_fw_version) > 0)
        		q->BitList |= (1 << 1);
        		
        	strcpy(q->TX2_fw_version, ispGetFWVersionStr(2));
        	if(strlen(q->TX2_fw_version) > 0)
        		q->BitList |= (1 << 2);
        		
        	strcpy(q->TX3_fw_version, ispGetFWVersionStr(3));
        	if(strlen(q->TX3_fw_version) > 0)
        		q->BitList |= (1 << 3);
        		
        	strcpy(q->TX4_fw_version, ispGetFWVersionStr(4));
        	if(strlen(q->TX4_fw_version) > 0)
        		q->BitList |= (1 << 4);
            break;
            
        default:
            break;
    }
#endif

	DEBUG_P2P("IOTYPE_WALMART_GETFWUPGRAGE_REQ OK\n\n");
    
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETFWUPGRAGE_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETFWUPGRAGE_RESP OK\n\n");
	}	
}


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetFwUpgrage(int avIndex, char *buf)
{	
    u32 Src, Dev;
    int ret = 0;
    int size = sizeof(SMsgAVIoctrlSetFWUpgrageResp);
    SMsgAVIoctrlSetFWUpgrageReq  *p = (SMsgAVIoctrlSetFWUpgrageReq *)buf;
    SMsgAVIoctrlSetFWUpgrageResp *q = (SMsgAVIoctrlSetFWUpgrageResp *)buf;
#if ISP_NEW_UPGRADE_FLOW_SUPPORT
    Src = p->Source;
    Dev = p->dev;

    if(sysGetFWUpgradeStatus() == 0)
    {
        if(Src == 1)
        {   
            DEBUG_YELLOW("[I] P2P set upgrade evt %#x\n", (1 << Dev));
            ret = sysSetEvt(SYS_EVT_APP2NET_UPGRADE_EVT, (1 << Dev));
        }
#if 0   // Not provide SD card upgrade by app.
        else if(Src == 0)
        {
            Dev |= (SYS_V_STORAGE_SDC << 28);
            DEBUG_YELLOW("[I] P2P set upgrade evt %#x\n", Dev);
            ret = sysSetEvt(SYS_EVT_APP2DEV_UPGRADE_EVT, Dev);
        }
#endif    
    }

    memset(buf, 0x0, sizeof(SMsgAVIoctrlSetFWUpgrageResp));

    if(ret == 1)
    {
        if((1 << Dev) & 0x1F)
            sysSetFWUpgradeStatus(1);
        q->dev = Dev;
        q->status = 0;
    }
    else
#endif
        q->status = 1;
    
    DEBUG_P2P("IOTYPE_WALMART_SETUPGRAGE_REQ OK\n\n");

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETUPGRAGE_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_SETUPGRAGE_RESP OK\n\n");
    }	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetAlertStatus(int avIndex, char *buf)
{
    int size = sizeof(SMsgAVIoctrlGetAlertStatusResp);
    SMsgAVIoctrlGetAlertStatusReq *p = (SMsgAVIoctrlGetAlertStatusReq *)buf;
    SMsgAVIoctrlGetAlertStatusResp *q = (SMsgAVIoctrlGetAlertStatusResp *)buf;

    DEBUG_P2P("IOTYPE_WALMART_GETALERTSTATUS_REQ OK\n\n");

    q->channel = p->channel;
    uiFlowGetUISetting(p->channel, UI_KEY_SYS_ALARM, &(q->status));

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETALERTSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_GETALERTSTATUS_RESP OK\n\n");
    }	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetAlertStatus(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetAlretstatusResp);
	SMsgAVIoctrlSetAlretstatusReq *p=(SMsgAVIoctrlSetAlretstatusReq *)buf;
	SMsgAVIoctrlSetAlretstatusResp *q=(SMsgAVIoctrlSetAlretstatusResp *)buf;

    q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_SYS_ALARM, p->status) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETALERTSTATUS_REQ %d\n\n", q->result);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETALERTSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETALERTSTATUS_RESP OK\n\n");
	}	
}
#endif


#if UI_CAMERA_LIGHT_SUPPORT
void P2PCmd_GetLightStatus(int avIndex, char *buf)
{
    u8 j, k, bit, value, week;
    int size =	sizeof(SMsgAVIoctrlGetCameralightStatusResp);
    SMsgAVIoctrlGetCameralightStatusReq *p = (SMsgAVIoctrlGetCameralightStatusReq *)buf;
    SMsgAVIoctrlGetCameralightStatusResp *q = (SMsgAVIoctrlGetCameralightStatusResp *)buf;

    DEBUG_P2P("IOTYPE_WALMART_GETCAMERALIGHTSTATUS_REQ OK,%d\n\n",p->channel);

    q->channel = p->channel; 
    uiFlowGetUISetting(p->channel, UI_KEY_SYS_LIGHT_ON, &(q->button));
    for(j = 0; j < 7; j++)/*Day*/
    {        
        for(k = 0; k < 6; k++)
        {        
            for(bit=0;bit<8;bit++)
            {
                value = k * 8 + bit;
                if (j ==0)
                {
                    week = 6;
                }
                else
                {
                    week = j - 1;
                }
            	q->status[(j * 48) + value] = (((uiLightInterval[p->channel][week][k]) & (0x1 << bit)) > 0) ? 1: 0;
                //DEBUG_YELLOW("%d %s %s %d %d %d %d\n",__LINE__, __FILE__,__FUNCTION__,j,value,(j * 48) + value,q->status[j*value]);
            }
        }
    } 

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETCAMERALIGHTSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_GETCAMERALIGHTSTATUS_RESP OK\n\n");
    }	
}

void P2PCmd_SetLightStatus(int avIndex, char *buf)
{
    u8 i, k, bit, value, count, week;
    u16 j;
	int size =	sizeof(SMsgAVIoctrlSetCameralightStatusResp);
	SMsgAVIoctrlSetCameralightStatusReq *p = (SMsgAVIoctrlSetCameralightStatusReq *)buf;
	SMsgAVIoctrlSetCameralightStatusResp *q = (SMsgAVIoctrlSetCameralightStatusResp *)buf;

	DEBUG_P2P("IOTYPE_WALMART_SETCAMERALIGHTSTATUS_REQ OK,%d\n\n",p->channel);

    if (p->button == 1)
    {
        i = 0;
        value = 0;
        for (j = 0; j < 336; j++)
        {
            if (i == 6)
            {
                i = 0;
            }
            //DEBUG_YELLOW("%d %s %s %d %d\n",__LINE__, __FILE__,__FUNCTION__,j,p->status[j]);
            if (p->status[j] == 0) //delete  
                value &= ~(0x80>>(7-(j%8)));
            else
                value |= (0x80>>(7-(j%8)));
            //DEBUG_YELLOW("%d %s %s %d\n",__LINE__, __FILE__,__FUNCTION__,value);

            if (((j+1) % 8) == 0)
            {
                //DEBUG_YELLOW("%d %s %s %d %d %d\n",__LINE__, __FILE__,__FUNCTION__,j/48,i,value);
                if ((j/48) == 0)
                {
                    week = 6;
                }
                else
                {
                    week = (j/48) - 1;
                }
                uiLightInterval[p->channel][week][i++] = value;
                value = 0;
            }
        }         
    }
    q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_SYS_LIGHT_ON, p->button) == 0)? 1 : 0;
    #if 0
        for (j = 0; j < 7; j++)
        {
            for (i = 0; i < 6; i++)
            {
             DEBUG_YELLOW("%d %s %s %d %d %d\n",__LINE__, __FILE__,__FUNCTION__,j,i,uiLightInterval[p->channel][j][i]);
            }
        }         
    #endif
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETCAMERALIGHTSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETCAMERALIGHTSTATUS_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetStroageInfo(int avIndex, char *buf)
{
    FS_DISKFREE_T *diskInfo;
    s64 FREE_SIZE, TOTAL_SIZE;
    int size = sizeof(SMsgAVIoctrlGetStroageInfoResp);
    SMsgAVIoctrlGetStroageInfoResp *q = (SMsgAVIoctrlGetStroageInfoResp *)buf;

    q->SD_status = sysGetStorageStatus(sysGetStorageIndex(SYS_V_STORAGE_SDC));
    if(q->SD_status > 0)
    {
        switch(sysGetStorageIndex(SYS_V_STORAGE_SDC))
        {
            case SYS_I_STORAGE_MAIN:
                diskInfo = &global_diskInfo;
                break;
            case SYS_I_STORAGE_BACKUP:
                diskInfo = &Backup_diskInfo;
                break;
            default:
                diskInfo = NULL;
                break;
        }
        if(diskInfo != NULL)
        {
            FREE_SIZE = (diskInfo->bytes_per_sector * diskInfo->sectors_per_cluster);
            TOTAL_SIZE = FREE_SIZE;

            FREE_SIZE *= diskInfo->avail_clusters;
            TOTAL_SIZE *= diskInfo->total_clusters;

            q->SD_FREE_SIZE[0] = FREE_SIZE & 0xFF;
            q->SD_FREE_SIZE[1] = (FREE_SIZE >> 8) & 0xFF;
            q->SD_FREE_SIZE[2] = (FREE_SIZE >> 16) & 0xFF;
            q->SD_FREE_SIZE[3] = (FREE_SIZE >> 24) & 0xFF;
            q->SD_FREE_SIZE[4] = (FREE_SIZE >> 32) & 0xFF;
            q->SD_FREE_SIZE[5] = (FREE_SIZE >> 40) & 0xFF;
            q->SD_FREE_SIZE[6] = (FREE_SIZE >> 48) & 0xFF;
            q->SD_FREE_SIZE[7] = (FREE_SIZE >> 56) & 0xFF;

            q->SD_TOTAL_SIZE[0] = TOTAL_SIZE & 0xFF;
            q->SD_TOTAL_SIZE[1] = (TOTAL_SIZE >> 8) & 0xFF;
            q->SD_TOTAL_SIZE[2] = (TOTAL_SIZE >> 16) & 0xFF;
            q->SD_TOTAL_SIZE[3] = (TOTAL_SIZE >> 24) & 0xFF;
            q->SD_TOTAL_SIZE[4] = (TOTAL_SIZE >> 32) & 0xFF;
            q->SD_TOTAL_SIZE[5] = (TOTAL_SIZE >> 40) & 0xFF;
            q->SD_TOTAL_SIZE[6] = (TOTAL_SIZE >> 48) & 0xFF;
            q->SD_TOTAL_SIZE[7] = (TOTAL_SIZE >> 56) & 0xFF;
        }
    }

    q->USB_status = sysGetStorageStatus(sysGetStorageIndex(SYS_V_STORAGE_USBMASS));
    if(q->USB_status > 0)
    {
        switch(sysGetStorageIndex(SYS_V_STORAGE_USBMASS))
        {
            case SYS_I_STORAGE_MAIN:
                diskInfo = &global_diskInfo;
                break;
            case SYS_I_STORAGE_BACKUP:
                diskInfo = &Backup_diskInfo;
                break;
            default:
                diskInfo = NULL;
                break;
        }
        if(diskInfo != NULL)
        {
            FREE_SIZE = (diskInfo->bytes_per_sector * diskInfo->sectors_per_cluster);
            TOTAL_SIZE = FREE_SIZE;

            FREE_SIZE *= diskInfo->avail_clusters;
            TOTAL_SIZE *= diskInfo->total_clusters;

            q->USB_FREE_SIZE[0] = FREE_SIZE & 0xFF;
            q->USB_FREE_SIZE[1] = (FREE_SIZE >> 8) & 0xFF;
            q->USB_FREE_SIZE[2] = (FREE_SIZE >> 16) & 0xFF;
            q->USB_FREE_SIZE[3] = (FREE_SIZE >> 24) & 0xFF;
            q->USB_FREE_SIZE[4] = (FREE_SIZE >> 32) & 0xFF;
            q->USB_FREE_SIZE[5] = (FREE_SIZE >> 40) & 0xFF;
            q->USB_FREE_SIZE[6] = (FREE_SIZE >> 48) & 0xFF;
            q->USB_FREE_SIZE[7] = (FREE_SIZE >> 56) & 0xFF;

            q->USB_TOTAL_SIZE[0] = TOTAL_SIZE & 0xFF;
            q->USB_TOTAL_SIZE[1] = (TOTAL_SIZE >> 8) & 0xFF;
            q->USB_TOTAL_SIZE[2] = (TOTAL_SIZE >> 16) & 0xFF;
            q->USB_TOTAL_SIZE[3] = (TOTAL_SIZE >> 24) & 0xFF;
            q->USB_TOTAL_SIZE[4] = (TOTAL_SIZE >> 32) & 0xFF;
            q->USB_TOTAL_SIZE[5] = (TOTAL_SIZE >> 40) & 0xFF;
            q->USB_TOTAL_SIZE[6] = (TOTAL_SIZE >> 48) & 0xFF;
            q->USB_TOTAL_SIZE[7] = (TOTAL_SIZE >> 56) & 0xFF;
        }
    }

    DEBUG_P2P("IOTYPE_WALMART_GETSTROAGEINFO_REQ OK\n\n");


    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETSTROAGEINFO_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_GETSTROAGEINFO_RESP OK\n\n");
    }	
}
#endif


#if 0
void P2PCmd_SetStroageInfo(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetStroageResp);
	SMsgAVIoctrlSetStroageReq *p=(SMsgAVIoctrlSetStroageReq *)buf;
	SMsgAVIoctrlSetStroageResp *q=(SMsgAVIoctrlSetStroageResp *)buf;

	DEBUG_P2P("IOTYPE_WALMART_SETALERTSTATUS_REQ OK\n\n");


	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETALERTSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETALERTSTATUS_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetPushMsgStatus(int avIndex, char *buf)
{
    int size = sizeof(SMsgAVIoctrlGetPushMsgtStatusResp);
    SMsgAVIoctrlGetPushMsgtStatusResp *q = (SMsgAVIoctrlGetPushMsgtStatusResp *)buf;

    uiFlowGetUISetting(UI_INVALID_VAL, UI_KEY_SYS_PUSH_MSG, &(q->status));
    DEBUG_P2P("IOTYPE_WALMART_GETPUSHMSGSTATUS_REQ OK\n\n");
    
    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETPUSHMSGSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_GETPUSHMSGSTATUS_RESP OK\n\n");
    }	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetPushMsgStatus(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetPushMsgstatusResp);
	SMsgAVIoctrlSetPushMsgstatusReq *p=(SMsgAVIoctrlSetPushMsgstatusReq *)buf;
	SMsgAVIoctrlSetPushMsgstatusResp *q=(SMsgAVIoctrlSetPushMsgstatusResp *)buf;

    q->result = (uiFlowSetCmdToUI(UI_INVALID_VAL, UI_KEY_SYS_PUSH_MSG, p->status) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETPUSHMSGSTATUS_REQ %d\n\n",q->result);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETPUSHMSGSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETPUSHMSGSTATUS_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
#if UI_CAMERA_ALARM_SUPPORT 
void P2PCmd_GetAlarmStatus(int avIndex, char *buf)
{
    u8 j, k, bit, value, week;
    int size =	sizeof(SMsgAVIoctrlGetCameraAlarmStatusResp);
    SMsgAVIoctrlGetCameraAlarmStatusReq *p = (SMsgAVIoctrlGetCameraAlarmStatusReq *)buf;
    SMsgAVIoctrlGetCameraAlarmStatusResp *q = (SMsgAVIoctrlGetCameraAlarmStatusResp *)buf;

    DEBUG_P2P("IOTYPE_WALMART_GETCAMERALARMSTATUS_REQ OK,%d\n\n",p->channel);

    q->channel = p->channel; 
    uiFlowGetUISetting(p->channel, UI_KEY_SYS_CAMALARM_ON, &(q->button));
    for(j = 0; j < 7; j++)/*Day*/
    {        
        for(k = 0; k < 6; k++)
        {        
            for(bit=0;bit<8;bit++)
            {
                value = k * 8 + bit;
                if (j ==0)
                {
                    week = 6;
                }
                else
                {
                    week = j - 1;
                }
            	q->status[(j * 48) + value] = (((uiCamAlarmInterval[p->channel][week][k]) & (0x1 << bit)) > 0) ? 1: 0;
                //DEBUG_YELLOW("%d %s %s %d %d %d %d\n",__LINE__, __FILE__,__FUNCTION__,j,value,(j * 48) + value,q->status[j*value]);
            }
        }
    } 

    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETCAMERALARMSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_GETCAMERALARMSTATUS_RESP OK\n\n");
    }	
}

void P2PCmd_SetAlarmStatus(int avIndex, char *buf)
{
    u8 i, k, bit, value, count, week;
    u16 j;
	int size =	sizeof(SMsgAVIoctrlSetCameraAlarmStatusResp);
	SMsgAVIoctrlSetCameraAlarmStatusReq *p = (SMsgAVIoctrlSetCameraAlarmStatusReq *)buf;
	SMsgAVIoctrlSetCameraAlarmStatusResp *q = (SMsgAVIoctrlSetCameraAlarmStatusResp *)buf;

	DEBUG_P2P("IOTYPE_WALMART_SETCAMERALARMSTATUS_REQ OK,%d\n\n",p->channel);

    if (p->button == 1)
    {
        i = 0;
        value = 0;
        for (j = 0; j < 336; j++)
        {
            if (i == 6)
            {
                i = 0;
            }
            //DEBUG_YELLOW("%d %s %s %d %d\n",__LINE__, __FILE__,__FUNCTION__,j,p->status[j]);
            if (p->status[j] == 0) //delete  
                value &= ~(0x80>>(7-(j%8)));
            else
                value |= (0x80>>(7-(j%8)));
            //DEBUG_YELLOW("%d %s %s %d\n",__LINE__, __FILE__,__FUNCTION__,value);

            if (((j+1) % 8) == 0)
            {
                //DEBUG_YELLOW("%d %s %s %d %d %d\n",__LINE__, __FILE__,__FUNCTION__,j/48,i,value);
                if ((j/48) == 0)
                {
                    week = 6;
                }
                else
                {
                    week = (j/48) - 1;
                }
                uiCamAlarmInterval[p->channel][week][i++] = value;
                value = 0;
            }
        }         
    }
    q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_SYS_CAMALARM_ON, p->button) == 0)? 1 : 0;
    #if 0
        for (j = 0; j < 7; j++)
        {
            for (i = 0; i < 6; i++)
            {
             DEBUG_YELLOW("%d %s %s %d %d %d\n",__LINE__, __FILE__,__FUNCTION__,j,i,uiLightInterval[p->channel][j][i]);
            }
        }         
    #endif
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETCAMERALARMSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETCAMERALARMSTATUS_RESP OK\n\n");
	}	
}
#endif
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetAudio(int avIndex, char *buf)
{
    int size = sizeof(SMsgAVIoctrlGetCameraRecordAudioResp);
	SMsgAVIoctrlGetCameraRecordAudioReq *p=(SMsgAVIoctrlGetCameraRecordAudioReq *)buf;
    SMsgAVIoctrlGetCameraRecordAudioResp *q = (SMsgAVIoctrlGetCameraRecordAudioResp *)buf;

    q->channel = p->channel;
    uiFlowGetUISetting(p->channel, UI_KEY_SYS_AUDIO, &(q->status));
    DEBUG_P2P("IOTYPE_WALMART_GETRECORDAUDIO_REQ OK\n\n");
    
    if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETRECORDAUDIO_RESP, (char*)q, size) == AV_ER_NoERROR)
    {
    	DEBUG_P2P("IOTYPE_WALMART_GETRECORDAUDIO_RESP OK\n\n");
    }	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetAudio(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetCameraRecordAudioResp);
	SMsgAVIoctrlSetCameraRecordAudioReq *p=(SMsgAVIoctrlSetCameraRecordAudioReq *)buf;
	SMsgAVIoctrlSetCameraRecordAudioResp *q=(SMsgAVIoctrlSetCameraRecordAudioResp *)buf;

    q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_SYS_AUDIO, p->status) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETRECORDAUDIO_REQ %d\n\n",q->result);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETRECORDAUDIO_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETRECORDAUDIO_RESP OK\n\n");
	}	
}
#endif


unsigned char RfIntensity(u32 CH)
{
    u8 bitRateStr[6];
    u8 signal=0;
    u8 camera=0;
	
 #if 1   
	const u8 TX1RX1TABLE[5]={0,1,7,10,19};
	const u8 TX2RX1TABLE[5]={0,1,7,10,19};
 #else
	const u8 TX1RX1TABLE[5]={1,7,10,19,30};
	const u8 TX2RX1TABLE[5]={1,4, 7,10,19};
 #endif
	
    u8 i,j;
    j=CH;
            camera =0;
 		signal=0;
			
        #if (SW_APPLICATION_OPTION == MR8202_AN_KLF08W) 
            if(j %2 ==0)  /* CH1 & CH3 */
            {
                for(i=0;i<2;i++)
                {
                    if(iconflag[UI_MENU_SETIDX_CH1_ON+i*2]==UI_MENU_SETTING_CAMERA_ON)
                    {
                        camera++;
                    }
                }
            }
            else  /* CH2 & CH4 */
            {
                for(i=0;i<2;i++)
                {
                    if(iconflag[UI_MENU_SETIDX_CH2_ON+i*2]==UI_MENU_SETTING_CAMERA_ON)
                    {
                        camera++;
                    }
                }
            }
        #elif (SW_APPLICATION_OPTION == MR8202_GATEWAYBOX_RX) 
            for(i=0;i<4;i++)
            {
                if(iconflag[UI_MENU_SETIDX_CH1_ON+i]==UI_MENU_SETTING_CAMERA_ON)
                {
                    camera++;
                }
            }
        #endif
            
            if(camera ==0)
            {
                camera =1;
            }  
			
			if(gRfiuUnitCntl[j].BitRate !=0)
			{
				switch(camera)
				{
					case 1:
						for(i=0; i<5 ;i++)
						{
							if(gRfiuUnitCntl[j].BitRate>TX1RX1TABLE[i])
								signal=i;
						}
						break;

					case 2:
						for(i=0; i<5 ;i++)
						{
							if(gRfiuUnitCntl[j].BitRate>TX2RX1TABLE[i])
								signal=i;
						}
						break;

					default:
						for(i=0; i<5 ;i++)
						{
							if(gRfiuUnitCntl[j].BitRate>TX1RX1TABLE[i])
								signal=i;
						}
						break;
				}
			}

      return signal;

}
void P2PCmd_GetRFStatus(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlGetCameraRfIntensityResp);
	SMsgAVIoctrlGetCameraRfIntensityReq*p=(SMsgAVIoctrlGetCameraRfIntensityReq *)buf;
	SMsgAVIoctrlGetCameraRfIntensityResp *q=(SMsgAVIoctrlGetCameraRfIntensityResp *)buf;

       q->channel=p->channel;
       q->level= RfIntensity(p->channel);
	   
	DEBUG_P2P("IOTYPE_WALMART_GETRFINTENSITY_REQ OK%d %d\n\n",q->channel,q->level);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETRFINTENSITY_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETRFINTENSITY_RESP OK\n\n");
	}	
}

void P2PCmd_GetBatCapStatus(int avIndex, char *buf)
{
    int size = sizeof(SMsgAVIoctrlGetCameraBatCapResp);
	SMsgAVIoctrlGetCameraBatCapReq*p=(SMsgAVIoctrlGetCameraBatCapReq *)buf;
	SMsgAVIoctrlGetCameraBatCapResp *q=(SMsgAVIoctrlGetCameraBatCapResp *)buf;
	
       q->channel=p->channel;
       q->level= gRfiuUnitCntl[p->channel].RFpara.TxBatteryLev;
	   
	DEBUG_P2P("IOTYPE_WALMART_GETBATCAP_REQ OK %d %d\n\n",q->channel,q->level);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETBATCAP_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETBATCAP_RESP OK\n\n");
	}
}


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetDeviceStatus(int avIndex, char *buf)
{
//Code move to sysbackqueue to process. Paul
#if (DEVSTATUS_ACTIVE_UPDATE)
    extern OS_FLAG_GRP  *gP2pDevStatusFlagGrp;
    u8 err;

    DEBUG_P2P("IOTYPE_WALMART_GETDEVSTATUS_REQ[%d] OK \n", avIndex);
    OSFlagPost(gP2pDevStatusFlagGrp, 0x1 << avIndex, OS_FLAG_SET, &err);
#else
    int size = sizeof(SMsgAVIoctrlGetDevStatusResp);
	SMsgAVIoctrlGetDevStatusReq*p=(SMsgAVIoctrlGetDevStatusReq *)buf;
	SMsgAVIoctrlGetDevStatusResp *q=(SMsgAVIoctrlGetDevStatusResp *)buf;
	
      q->channel=p->channel;
#if  RFIU_RX_WAKEUP_TX_SCHEME
	if( gRfiuUnitCntl[p->channel].RFpara.BateryCam_support)
		{
		if (gRfiuUnitCntl[p->channel].RFpara.TxBatteryLev >= UI_BATTERY_LV4)
          		q->bat_sta = 3;
	 	else
	   		q->bat_sta= gRfiuUnitCntl[p->channel].RFpara.TxBatteryLev;
		}
	else
#endif
	 q->bat_sta=-1;
      uiFlowGetUISetting(p->channel, UI_KEY_CAM_MANUAL_AREC, &(q->motion_sta)); 	
      q->rec_sta= (MultiChannelGetCaptureVideoStatus(p->channel) == 1) ? 1 : 0;
      q->rf_sta= RfIntensity(p->channel);
#if UI_CAMERA_ALARM_SUPPORT	  
      q->alarm_sta= uiGetAlarmStatusAPP(p->channel);
#else
     q->alarm_sta=-1;
#endif

#if UI_CAMERA_LIGHT_SUPPORT
      q->light_sta= uiGetLightStatusAPP(p->channel);
#else
     q->light_sta=-1;//Should not less then 0, unsigned char
#endif
	//DEBUG_YELLOW("%d %s %s %d %d %d %d\n",__LINE__, __FILE__,__FUNCTION__,q->channel,q->motion_sta,q->alarm_sta,q->light_sta);
	DEBUG_P2P("IOTYPE_WALMART_GETDEVSTATUS_REQ OK \n");

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETDEVSTATUS_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETDEVSTATUS_RESP OK\n\n");
	}
#endif
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_GetMotionStatus(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlGetCameraMotionSwitchResp);
	SMsgAVIoctrlGetCameraMotionSwitchReq *p=(SMsgAVIoctrlGetCameraMotionSwitchReq *)buf;
	SMsgAVIoctrlGetCameraMotionSwitchResp *q=(SMsgAVIoctrlGetCameraMotionSwitchResp *)buf;

    uiFlowGetUISetting(p->channel, UI_KEY_CAM_MANUAL_AREC, &(q->onoff));

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETRECORDMOTIONSW_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETRECORDMOTIONSW_RESP OK\n\n");
	}	
}
#endif


#if (CHIP_OPTION == CHIP_A1025A)
void P2PCmd_SetMotionStatus(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetCameraMotionSwitchResp);
	SMsgAVIoctrlSetCameraMotionSwitchReq *p=(SMsgAVIoctrlSetCameraMotionSwitchReq *)buf;
	SMsgAVIoctrlSetCameraMotionSwitchResp *q=(SMsgAVIoctrlSetCameraMotionSwitchResp *)buf;

    q->result = (uiFlowSetCmdToUI(p->channel, UI_KEY_CAM_MANUAL_AREC, p->onoff) == 0)? 1 : 0;
	DEBUG_P2P("IOTYPE_WALMART_SETRECORDMOTIONSW_REQ %d\n\n",q->result);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETRECORDMOTIONSW_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETRECORDMOTIONSW_RESP OK\n\n");
	}	
}
#endif


#if UI_CAMERA_LIGHT_SUPPORT
void P2PCmd_GetCamLightStatus(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlGetCameralightSwitchResp);
	SMsgAVIoctrlGetCameralightSwitchReq *p=(SMsgAVIoctrlGetCameralightSwitchReq *)buf;
	SMsgAVIoctrlGetCameralightSwitchResp *q=(SMsgAVIoctrlGetCameralightSwitchResp *)buf;

    q->channel = p->channel;
#if UI_CAMERA_LIGHT_SUPPORT
    q->onoff= uiGetLightStatusAPP(p->channel);
#else
    q->onoff = -1;
#endif

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETCAMERALIGHTSW_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETCAMERALIGHTSW_RESP OK\n\n");
	}	
}

void P2PCmd_SetCamLightStatus(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetCameralightSwitchResp);
    unsigned int ret;
	SMsgAVIoctrlSetCameralightSwitchReq *p=(SMsgAVIoctrlSetCameralightSwitchReq *)buf;
	SMsgAVIoctrlSetCameralightSwitchResp *q=(SMsgAVIoctrlSetCameralightSwitchResp *)buf;

    ret = uiSetLightStatusAPP(p->channel, p->onoff) & 0xFF;
    q->result = ret;
    DEBUG_P2P("IOTYPE_WALMART_SETCAMERALIGHTSW_REQ %d\n\n",q->result);
	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETCAMERALIGHTSW_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETCAMERALIGHTSW_RESP OK\n\n");
	}	
}
#endif

#if UI_CAMERA_ALARM_SUPPORT
void P2PCmd_GetCamAlarmStatus(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlGetCameraAlarmSwitchResp);
	SMsgAVIoctrlGetCameraAlarmSwitchReq *p=(SMsgAVIoctrlGetCameraAlarmSwitchReq *)buf;
	SMsgAVIoctrlGetCameraAlarmSwitchResp *q=(SMsgAVIoctrlGetCameraAlarmSwitchResp *)buf;

    q->channel = p->channel;
#if UI_CAMERA_ALARM_SUPPORT	  
    q->onoff = uiGetAlarmStatusAPP(p->channel);
#else
    q->onoff = -1;
#endif

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_GETCAMERALARMSW_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_GETCAMERALARMSW_RESP OK\n\n");
	}	
}

void P2PCmd_SetCamAlarmStatus(int avIndex, char *buf)
{
	int size =	sizeof(SMsgAVIoctrlSetCameraAlarmSwitchResp);
    unsigned int ret;
	SMsgAVIoctrlSetCameraAlarmSwitchReq *p=(SMsgAVIoctrlSetCameraAlarmSwitchReq *)buf;
	SMsgAVIoctrlSetCameraAlarmSwitchResp *q=(SMsgAVIoctrlSetCameraAlarmSwitchResp *)buf;

    ret = uiSetAlarmStatusAPP(p->channel, p->onoff) & 0xFF;
    q->result = ret;
	DEBUG_P2P("IOTYPE_WALMART_SETCAMERALARMSW_REQ %d\n\n",q->result);

	if (avSendIOCtrl(avIndex, IOTYPE_WALMART_SETCAMERALARMSW_RESP, (char*)q, size) == AV_ER_NoERROR)
	{
		DEBUG_P2P("IOTYPE_WALMART_SETCAMERALARMSW_RESP OK\n\n");
	}	
}
#endif

