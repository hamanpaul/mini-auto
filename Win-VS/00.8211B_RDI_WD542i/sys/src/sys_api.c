/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    sys.c

Abstract:

    The routines of system control.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "board.h"
#include "task.h"
#include "sysapi.h"
#include "sys.h"
#include "usbapi.h"
#include "adcapi.h"
#include "uiapi.h"
#include "Iduapi.h"
#include "gpioapi.h"
#include "ispapi.h"
#include "iisapi.h"
#include "siuapi.h"
#include "spiapi.h"
#include "fsapi.h"
#include "dcfapi.h"
#include "asfapi.h"
#include "jpegapi.h"
#include "timerapi.h"
#include "uartapi.h"
#include "isuapi.h"
#include "ipuapi.h"
#include "mpeg4api.h"
#include "../../ui/inc/ui.h"
#include "../../ui/inc/ui_project.h"
#include "p2pserver_api.h"
#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
//#include "../LwIP/include/tutk_P2P/AVIOCTRLDEFs.h"

#if (HOME_RF_SUPPORT)
#include "../LwIP/include/tutk_P2P/MR8200def_homeautomation.h"
#endif

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
SYS_CONFIG_SETTING sysConfig;
SYS_CONFIG_SETTING start_sysConfig;
u8 homeRFSensorName[17];


/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern  UI_NET_INFO UINetInfo;
#if ((HW_BOARD_OPTION  == MR8200_RX_JIT) || GATEWAY_BOX || (HW_BOARD_OPTION  == MR8200_RX_JIT_M703SN4))
extern u8  uiScheduleTime[7][4][96];
#endif
extern u8  homeRFSceneAppMode;
#if CDVR_iHome_LOG_SUPPORT
extern u32 dctTotalLogCount;
#endif
#if SYSTEM_DEBUG_SD_LOG
extern u8 DEBUG_SD_BUF1[512];
extern u8 DEBUG_SD_BUF2[512];
extern OS_EVENT    *dcfReadySemEvt;
#endif
#if ((HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA) ||(HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA))
extern u8 UI_HA_Sensor_Beep_Flag;
#endif

/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */
#if GATEWAY_BOX
extern void uiOsdDrawOverwrit(void);
#endif
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

/*-------------------- event function -------------------*/
/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */

void sysSet_DefaultValue(void)
{
    u8 i;
    sysConfig.RecSetting.Overwrite  = 0; //UI_MENU_SETTING_OVERWRITE_NO
    sysConfig.RecSetting.Seccion    = 300; //UI_MENU_SETTING_SECTION_5MIN
    sysConfig.RecSetting.Duration   = 0;
    sysConfig.RecSetting.EventExtendTime = 0;

    sysConfig.AlarmSetting.AlarmEnable  = 0;
    sysConfig.AlarmSetting.AlarmVal     = 0;
    sysConfig.AlarmSetting.AlarmRange   = 0;

    for(i=0;i<MULTI_CHANNEL_MAX;i++)
    {
        sysConfig.CamSetting[i].Brightness  = 4; //UI_MENU_SETTING_BRIGHTNESS_LV5
        sysConfig.CamSetting[i].CamerOnOff  = 1; //UI_MENU_SETTING_CAMERA_ON
        sysConfig.CamSetting[i].RecMode     = 0; //UI_MENU_REC_MODE_MANUAL
        sysConfig.CamSetting[i].Resoultion  = 1; //UI_MENU_SETTING_RESOLUTION_VGA
    }

    for(i=0;i<MULTI_CHANNEL_MAX;i++)
    {
        sysConfig.EventSetting[i].EventRECTime     = 0;
        sysConfig.EventSetting[i].MotionEnable     = 0;
        sysConfig.EventSetting[i].MotionDayLevel   = 0;
        sysConfig.EventSetting[i].MotionNeightLevel = 0;
        sysConfig.EventSetting[i].PIREnable        = 0;
    }

    sysConfig.NetSetting.DHCPEnable = 1;
    for(i=0;i<4;i++)
    {
        sysConfig.NetSetting.IPAddr[i]  = 0;
        sysConfig.NetSetting.Gateway[i] = 0;
        sysConfig.NetSetting.NetMask[i] = 0;
    }
    
    for(i=0;i<MULTI_CHANNEL_MAX;i++)
    {
        sysConfig.SchSetting[i].SchEnable  = 0;
    }

    sysConfig.SysSetting.Flicker    = 0; //UI_MENU_SETTING_FLICKER_60HZ
    sysConfig.SysSetting.Language   = 0; //UI_MENU_SETTING_LANGUAGE_ENGLISH
    sysConfig.SysSetting.TVOut      = 0; //UI_MENU_NTSC
    sysConfig.SysSetting.Volume     = 5; //UI_MENU_VOL_LV5

    sysConfig.TimeSetting.DSTEnable     = 0;
    sysConfig.TimeSetting.TimeZoneSign  = 0;
    sysConfig.TimeSetting.TimeZoneHour  = 0;
    sysConfig.TimeSetting.TimeZoneMin   = 0;
}

void sysGet_Overwrite(u8* pData)
{
    *pData=sysConfig.RecSetting.Overwrite;
}

u8 sysSet_Overwrite(u8 pData) // 1=yes; 0=no
{
    int i;
    sysConfig.RecSetting.Overwrite=pData;
#if GATEWAY_BOX
        if(sysConfig.RecSetting.Overwrite==1)
            iconflag[UI_MENU_SETIDX_OVERWRITE]=0; //UI_MENU_SETTING_OVERWRITE_YES
        else
            iconflag[UI_MENU_SETIDX_OVERWRITE]=1; //UI_MENU_SETTING_OVERWRITE_NO
        uiOsdDrawOverwrit();
#endif

    if (sysConfig.RecSetting.Overwrite==1)//UI_MENU_SETTING_OVERWRITE_YES
    {
        //sysCaptureVideoMode = ASF_CAPTURE_OVERWRITE;
        sysCaptureVideoMode    |= ASF_CAPTURE_OVERWRITE_ENA;
        #if MULTI_CHANNEL_VIDEO_REC
        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
            VideoClipParameter[i].sysCaptureVideoMode  |= ASF_CAPTURE_OVERWRITE_ENA;
        #endif
    }
    else // UI_MENU_SETTING_OVERWRITE_NO
    {
    
        //sysCaptureVideoMode = ASF_CAPTURE_NORMAL;
        sysCaptureVideoMode    &= ~ASF_CAPTURE_OVERWRITE_ENA;
        #if MULTI_CHANNEL_VIDEO_REC
        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
            VideoClipParameter[i].sysCaptureVideoMode  &= ~ASF_CAPTURE_OVERWRITE_ENA;
        #endif
    }
    return 1;
}

void sysGet_Seccion(u32* pData)
{
    *pData=sysConfig.RecSetting.Seccion;
}

u8 sysSet_Seccion(u32 pData) // sec
{
    sysConfig.RecSetting.Seccion=pData;
    asfSetVideoSectionTime(sysConfig.RecSetting.Seccion);
#if ((HW_BOARD_OPTION  == MR8200_RX_JIT) || (HW_BOARD_OPTION  == MR8200_RX_JIT_M703SN4))
    iconflag[UI_MENU_SETIDX_SECTION]=sysConfig.RecSetting.Seccion;
#endif
    return 1;
}

u8 sysSet_Duration(u32 pData)
{
    u8 i;
    sysConfig.RecSetting.Duration=pData;
#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    //iconflag[UI_MENU_SETIDX_DURATION] = sysConfig.RecSetting.Duration;
    if (Main_Init_Ready == 1)
        uiCaptureVideoStop();

    for(i=0; i<MULTI_CHANNEL_MAX; i++)
    {
        VideoClipParameter[i].asfRecTimeLen = sysConfig.RecSetting.Duration;
    }
#endif
    return 1;
}

void sysGet_Duration(u32* pData)
{
    *pData=sysConfig.RecSetting.Duration;
}

u8 sysSet_EventExtendTime(u32 pData)
{
    sysConfig.RecSetting.EventExtendTime=pData;
#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    if (Main_Init_Ready == 1)
        uiCaptureVideoStop();
    asfEventExtendTime = sysConfig.RecSetting.EventExtendTime;
#endif
    return 1;
}

void sysGet_EventExtendTime(u32* pData)
{
    *pData=sysConfig.RecSetting.EventExtendTime;
}

u8 sysSet_RecMode(u8 nCam, u8 pData)
{
    u8 MEnable, nRet=0;
    sysConfig.CamSetting[nCam].RecMode=pData;
#if GATEWAY_BOX
    iconflag[UI_MENU_SETIDX_CH1_REC_MODE+nCam]=sysConfig.CamSetting[nCam].RecMode;
#endif
#if RFIU_SUPPORT
    // MANUAL       0
    // MOTION       1
    // SCHEDULED 2
    // NONE           3
    if(sysConfig.CamSetting[nCam].RecMode == 1)// UI_MENU_REC_MODE_MOTION
    {
        MEnable=1;
    }
    else
    {
        MEnable=0;
    }
    nRet = uiSetRfMotionRxToTx(MEnable, sysConfig.EventSetting[nCam].MotionDayLevel, 
                                 sysConfig.EventSetting[nCam].MotionNeightLevel, nCam);
#endif
    return nRet;
}

void sysGet_RecMode(u8 nCam, u8* pData)
{
    *pData=sysConfig.CamSetting[nCam].RecMode;
}

u8 sysSet_CamerOnOff(u8 nCam, u8 pData)
{
    u8 nRet=0;
    sysConfig.CamSetting[nCam].CamerOnOff=pData;
#if GATEWAY_BOX
    iconflag[UI_MENU_SETIDX_CH1_ON+nCam]=sysConfig.CamSetting[nCam].CamerOnOff;
    nRet=uiMenuSet_TX_CameraOnOff(sysConfig.CamSetting[nCam].CamerOnOff, nCam);
#endif
    return nRet;
}

void sysGet_CamerOnOff(u8 nCam, u8* pData)
{
    *pData=sysConfig.CamSetting[nCam].CamerOnOff;
}

void sysGet_Resoultion(u8 nCam, u8* pData)
{
    *pData=sysConfig.CamSetting[nCam].Resoultion;
}

u8 sysSet_Resoultion(u8 nCam, u8 pData)
{
    u8 nRet=0;
    sysConfig.CamSetting[nCam].Resoultion=pData;
#if GATEWAY_BOX
    iconflag[UI_MENU_SETIDX_CH1_RES+nCam]=sysConfig.CamSetting[nCam].Resoultion;
#endif
#if RFIU_SUPPORT
    nRet=uiSetRfResolutionRxToTx(sysConfig.CamSetting[nCam].Resoultion, nCam);
#endif
    return nRet;
}

void sysGet_Brightness(u8 nCam, u8* pData)
{
    *pData=sysConfig.CamSetting[nCam].Brightness;
}

u8 sysSet_Brightness(u8 nCam, u8 pData)
{
    u8 nRet=0;
    sysConfig.CamSetting[nCam].Brightness=pData;
#if GATEWAY_BOX
    iconflag[UI_MENU_SETIDX_CH1_BRIGHT+nCam]=sysConfig.CamSetting[nCam].Brightness;
#endif
#if RFIU_SUPPORT
    nRet=uiSetRfBrightnessRxToTx(sysConfig.CamSetting[nCam].Brightness, nCam);
#endif
    return nRet;
}

void sysGet_SchEnable(u8 nCam, u8* pData)
{
    *pData=sysConfig.SchSetting[nCam].SchEnable;
}

u8 sysSet_SchEnable(u8 nCam, u8 pData)
{
    sysConfig.SchSetting[nCam].SchEnable=pData;
    return 1;
}

void sysGet_Schedule(u8 nCam, u8 nDay, u8 nHour, u8* pData)
{
#if GATEWAY_BOX
    *pData=uiScheduleTime[nDay][nCam][nHour*2];
#endif
}

u8 sysSet_Schedule(u8 nCam, u8 nDay, u8 nHour, u8 pData)
{
    u8 i;
#if GATEWAY_BOX
    i=nHour*2;
    uiScheduleTime[nDay][nCam][i]=pData;
    uiScheduleTime[nDay][nCam][i+1]=pData;
#endif
    return 1;
}

void sysGet_MotionEnable(u8 nCam, u8* pEnable, u8* pDay, u8* pNight)
{
    *pEnable=sysConfig.EventSetting[nCam].MotionEnable;
    *pDay=sysConfig.EventSetting[nCam].MotionDayLevel;
    *pNight=sysConfig.EventSetting[nCam].MotionNeightLevel;
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    *pDay = MDSensitivity;
#endif
}

u8 sysSet_MotionEnable(u8 nCam, u8 pEnable, u8 pDay, u8 pNight)
{
    u8 nRet=0;
    sysConfig.EventSetting[nCam].MotionEnable=pEnable;
    sysConfig.EventSetting[nCam].MotionDayLevel=pDay;
    sysConfig.EventSetting[nCam].MotionNeightLevel=pNight;
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    if (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] != pDay)
    {
        iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] = pDay;
        Save_UI_Setting();
    }
    MDSensitivity = pDay;
    uiMenuSet_MotionSensitivity(pDay);
    return 1;
#elif GATEWAY_BOX
    iconflag[UI_MENU_SETIDX_CH1_MOTION_DAY+nCam]=sysConfig.EventSetting[nCam].MotionDayLevel;
#endif
#if RFIU_SUPPORT
    nRet=uiSetRfMotionRxToTx(sysConfig.EventSetting[nCam].MotionEnable, sysConfig.EventSetting[nCam].MotionDayLevel, 
                                 sysConfig.EventSetting[nCam].MotionNeightLevel, nCam);
#endif
    return nRet;
}

void sysGet_PIREnable(u8 nCam, u8* pData)
{
    *pData=sysConfig.EventSetting[nCam].PIREnable;
}

u8 sysSet_PIREnable(u8 nCam, u8 pData)
{
    u8 nRet=0;
    sysConfig.EventSetting[nCam].PIREnable=pData;
#if GATEWAY_BOX
//    iconflag[UI_MENU_SETIDX_CH1_PIR+nCam]=sysConfig.EventSetting[nCam].PIREnable;
#endif
#if RFIU_SUPPORT
    nRet=uiSetRfPIRRxToTx(sysConfig.EventSetting[nCam].PIREnable, nCam);
#endif
    return nRet;
}

void sysGet_TimeZone(u8* pSign, u8* pHour, u8* pMin)
{
    *pSign=sysConfig.TimeSetting.TimeZoneSign;
    *pHour=sysConfig.TimeSetting.TimeZoneHour;
    *pMin =sysConfig.TimeSetting.TimeZoneMin;
}

u8 sysSet_TimeZone(u8 pSign, u8 pHour, u8 pMin)
{
    RTC_TIME_ZONE nZone;
    u8 nRet=0;
    
    sysConfig.TimeSetting.TimeZoneSign=pSign;
    sysConfig.TimeSetting.TimeZoneHour=pHour;
    sysConfig.TimeSetting.TimeZoneMin =pMin;

    nZone.hour      = sysConfig.TimeSetting.TimeZoneHour;
    nZone.min       = sysConfig.TimeSetting.TimeZoneMin;
    nZone.operator  = sysConfig.TimeSetting.TimeZoneSign;
    nRet=RTC_Set_TimeZone(&nZone);
    return nRet;
}

void sysGet_Time(RTC_DATE_TIME *pTime)
{
    RTC_Get_Time(pTime);
}

u8 sysSet_Time(RTC_DATE_TIME *pTime)
{
    RTC_Set_Time(pTime);
    return 1;
}

void sysGet_DSTEnable(u8* pData)
{
    *pData=sysConfig.TimeSetting.DSTEnable;
}

u8 sysSet_DSTEnable(u8 pData)
{
    sysConfig.TimeSetting.DSTEnable=pData;
#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    iconflag[UI_MENU_SETIDX_DAY_SAVING_TIME] = sysConfig.TimeSetting.DSTEnable;
    RTC_Set_DST(sysConfig.TimeSetting.DSTEnable);
#endif
    return 1;
}

void sysGet_AlarmEnable(u8* pData)
{
    *pData=sysConfig.AlarmSetting.AlarmEnable;
}

u8 sysSet_AlarmEnable(u8 pData)
{
    sysConfig.AlarmSetting.AlarmEnable=pData;
#if GATEWAY_BOX
    //iconflag[UI_MENU_SETIDX_ALARM]=sysConfig.AlarmSetting.AlarmEnable;
#endif
    //UI_ALARM_ON   0
    //UI_ALARM_OFF  1
    //uiFlowSetAlarm=sysConfig.AlarmSetting.AlarmEnable;
    return 1;
}

void sysGet_AlarmVal(u8* pData)
{
    *pData=sysConfig.AlarmSetting.AlarmVal;
}

u8 sysSet_AlarmVal(u8 pData)
{
    sysConfig.AlarmSetting.AlarmVal=pData;
#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    iconflag[UI_MENU_SETIDX_ALARM_VOL] = sysConfig.AlarmSetting.AlarmVal;
#endif
    return 1;
}

void sysGet_AlarmRange(u8* pData)
{
    *pData=sysConfig.AlarmSetting.AlarmRange;
}

u8 sysSet_AlarmRange(u8 pData)
{
    sysConfig.AlarmSetting.AlarmRange=pData;
    return 1;
}

void sysGet_NetworkData(SYS_CONFIG_NETWORK *sNetData)
{
    memcpy((void *)sNetData, (void *)&sysConfig.NetSetting, sizeof(SYS_CONFIG_NETWORK));
    //DEBUG_SYS("DHCPEnable = %d \n",sysConfig.NetSetting.DHCPEnable);
    //DEBUG_SYS("IP Address     :%u.%u.%u.%u \n",sysConfig.NetSetting.IPAddr[0], sysConfig.NetSetting.IPAddr[1], sysConfig.NetSetting.IPAddr[2], sysConfig.NetSetting.IPAddr[3]);
    //DEBUG_SYS("Subnet Mask    :%u.%u.%u.%u \n",sysConfig.NetSetting.NetMask[0], sysConfig.NetSetting.NetMask[1], sysConfig.NetSetting.NetMask[2], sysConfig.NetSetting.NetMask[3]);
    //DEBUG_SYS("Default Getway :%u.%u.%u.%u \n",sysConfig.NetSetting.Gateway[0], sysConfig.NetSetting.Gateway[1], sysConfig.NetSetting.Gateway[2], sysConfig.NetSetting.Gateway[3]);
}

u8 sysSet_NetworkData(SYS_CONFIG_NETWORK *sNetData)
{
    u8 i;
    memcpy(&sysConfig.NetSetting, sNetData, sizeof(SYS_CONFIG_NETWORK));
    //DEBUG_SYS("DHCPEnable = %d \n",sysConfig.NetSetting.DHCPEnable);
    //DEBUG_SYS("IP Address     :%u.%u.%u.%u \n",sysConfig.NetSetting.IPAddr[0], sysConfig.NetSetting.IPAddr[1], sysConfig.NetSetting.IPAddr[2], sysConfig.NetSetting.IPAddr[3]);
    //DEBUG_SYS("Subnet Mask    :%u.%u.%u.%u \n",sysConfig.NetSetting.NetMask[0], sysConfig.NetSetting.NetMask[1], sysConfig.NetSetting.NetMask[2], sysConfig.NetSetting.NetMask[3]);
    //DEBUG_SYS("Default Getway :%u.%u.%u.%u \n",sysConfig.NetSetting.Gateway[0], sysConfig.NetSetting.Gateway[1], sysConfig.NetSetting.Gateway[2], sysConfig.NetSetting.Gateway[3]);

#if(NIC_SUPPORT)
    if (sysConfig.NetSetting.DHCPEnable == 1)
    {
        DEBUG_SYS("Enable DHCP\r\n");
        UINetInfo.IsStaticIP=0;
        SetNetworkInfo(&UINetInfo);
    }
    else
    {
        DEBUG_SYS("IP Address     :%u.%u.%u.%u \n",sysConfig.NetSetting.IPAddr[0], sysConfig.NetSetting.IPAddr[1], sysConfig.NetSetting.IPAddr[2], sysConfig.NetSetting.IPAddr[3]);
        DEBUG_SYS("Subnet Mask    :%u.%u.%u.%u \n",sysConfig.NetSetting.NetMask[0], sysConfig.NetSetting.NetMask[1], sysConfig.NetSetting.NetMask[2], sysConfig.NetSetting.NetMask[3]);
        DEBUG_SYS("Default Getway :%u.%u.%u.%u \n",sysConfig.NetSetting.Gateway[0], sysConfig.NetSetting.Gateway[1], sysConfig.NetSetting.Gateway[2], sysConfig.NetSetting.Gateway[3]);
        for(i=0;i<4;i++)
        {
            UINetInfo.IPaddr[i]=sysConfig.NetSetting.IPAddr[i];
            UINetInfo.Netmask[i]=sysConfig.NetSetting.NetMask[i];
            UINetInfo.Gateway[i]=sysConfig.NetSetting.Gateway[i];
        }
        UINetInfo.IsStaticIP=1;
        SetNetworkInfo(&UINetInfo);
    }
#endif
    return 1;
}

void sysGet_TVOut(u8* pData)
{
    *pData=sysConfig.SysSetting.TVOut;
}

u8 sysSet_TVOut(u8 pData)
{
    u8 uartCmd[20];
    u8 i;
    int RfBusy = 1, cnt = 0;
    sysConfig.SysSetting.TVOut=pData;
#if ((HW_BOARD_OPTION  == MR8200_RX_JIT) || (HW_BOARD_OPTION  == MR8200_RX_JIT_M703SN4))
    iconflag[UI_MENU_SETIDX_TV_OUT]=sysConfig.SysSetting.TVOut;

    if (Main_Init_Ready == 0)
        return;

    if(uiSetDefault == 0)
    {
        if (MyHandler.MenuMode != VIDEO_MODE)
            uiFlowSetupToPreview();
    }

    if(sysConfig.SysSetting.TVOut == 0) // UI_MENU_NTSC
    {
        uiSetOutputMode(SYS_OUTMODE_TV);
        sysTVinFormat = TV_IN_NTSC;
        iduSwitchNTSCPAL(SYS_TV_OUT_NTSC);
        iconflag[UI_MENU_SETIDX_50HZ_60HZ] = SENSOR_AE_FLICKER_60HZ;
        sprintf((char *)uartCmd,"FLIC %d", SENSOR_AE_FLICKER_60HZ);
        if(sysTVOutOnFlag==1) // Amon: TV out need to update xy (140109)
            uiSetTVOutXY(SYS_TV_OUT_NTSC);
    }
    else // UI_MENU_PAL
    {
        uiSetOutputMode(SYS_OUTMODE_TV);
        DEBUG_UI("TV out change to PAL\r\n");
        sysTVinFormat = TV_IN_PAL;
        iduSwitchNTSCPAL(SYS_TV_OUT_PAL);
        iconflag[UI_MENU_SETIDX_50HZ_60HZ] = SENSOR_AE_FLICKER_50HZ;
        sprintf((char *)uartCmd,"FLIC %d", SENSOR_AE_FLICKER_50HZ);
        if(sysTVOutOnFlag==1) // Amon: TV out need to update xy (140109)
            uiSetTVOutXY(SYS_TV_OUT_PAL);
    }
    if (Main_Init_Ready == 1)
    {
        for (i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            cnt=0;
            DEBUG_UI("========================> cam%d !!!\r\n",i);
            while (RfBusy == 1)
            {
                if (cnt >UI_SET_FR_CMD_RETRY)
                {
                    DEBUG_UI("uiMenuSet_TVout_Format Timeout !!!\r\n");
                    break;
                }
                RfBusy = rfiu_RXCMD_Enc(uartCmd, i);
                cnt++;
                OSTimeDly(1);
            }
            RfBusy = 1;
        }
    }
#endif
    return 1;
}

void sysGet_Language(u8* pData)
{
    *pData=sysConfig.SysSetting.Language;
}

u8 sysSet_Language(u8 pData)
{
    sysConfig.SysSetting.Language=pData;
#if GATEWAY_BOX
    iconflag[UI_MENU_SETIDX_LANGUAGE]=sysConfig.SysSetting.Language;
#endif
    CurrLanguage = sysConfig.SysSetting.Language;
    return 1;
}

void sysGet_Flicker(u8* pData)
{
    *pData=sysConfig.SysSetting.Flicker;
}

u8 sysSet_Flicker(u8 pData)
{
    u8 nRet=0;
    sysConfig.SysSetting.Flicker=pData;
#if ((HW_BOARD_OPTION  == MR8200_RX_JIT) || (HW_BOARD_OPTION  == MR8200_RX_JIT_M703SN4))
    iconflag[UI_MENU_SETIDX_50HZ_60HZ]=sysConfig.SysSetting.Flicker;
#endif
#if RFIU_SUPPORT
    // SENSOR_AE_FLICKER_60HZ       0
    // SENSOR_AE_FLICKER_50HZ       1
    nRet=uiSetRfFlickerRxToTx(sysConfig.SysSetting.Flicker);
#endif
    return nRet;
}

void sysGet_Volume(u8* pData)
{
    *pData=sysConfig.SysSetting.Volume;
}

u8 sysSet_Volume(u8 pData)
{
    sysConfig.SysSetting.Volume=pData;
#if GATEWAY_BOX
    iconflag[UI_MENU_SETIDX_VOLUME]=sysConfig.SysSetting.Volume;
#endif
    adcSetDAC_OutputGain(sysConfig.SysSetting.Volume);
    return 1;
}

void sysGet_EventTime(u8 nCam, u32 *pData)
{
    *pData = sysConfig.EventSetting[nCam].EventRECTime;
}

u8 sysSet_EventTime(u8 nCam, u32 pData)
{
    sysConfig.EventSetting[nCam].EventRECTime = pData;
#if MULTI_CHANNEL_VIDEO_REC
    VideoClipParameter[nCam].asfRecTimeLen = sysConfig.EventSetting[nCam].EventRECTime;
#endif
    return 1;
}


u8 sysSet_Pair(u8 nCam)
{
#if RFIU_SUPPORT
    rfiu_PAIR_Linit(nCam);
#endif
    return 1;
}

u8 sysSet_FormatSD(void)
{
    u8  err;
    
    if (Main_Init_Ready == 0)
        return 0;
    
    if(gInsertCard!=1)
        return 0;
    
    general_MboxEvt->OSEventPtr=(void *)0;

    if(OSFlagAccept(gUiStateFlagGrp, FLAGUI_SD_GOTO_FORMAT, OS_FLAG_WAIT_SET_ANY, &err)>0)
        sysbackSetEvt(SYS_BACK_PLAYBACK_FORMAT, 0);
    else
        sysSetEvt(SYS_EVT_PLAYBACK_FORMAT, 0);
    return 1;
}

u8 sysSet_UpgradeFW(void)
{
    u8  err;

    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_CLR, &err);
    general_MboxEvt->OSEventPtr=(void *)0;
    sysSetEvt(SYS_EVT_UPGRADE_FW, 0);
    return 1;
}

/*System reboot.
return <0 ,if fail
*/

u8 sysSet_Reset(void)
{
    DEBUG_SYS("sysSet_Reset()\n");
    sysForceWDTtoReboot();
    return 1;
}

/*Get current tempture.
\return <0 ,if fail,
\param *temp [out]: The current temperature.
*/
u8 sysGet_CurrTemp(float *temp)
{
#if(THERMOMETER_SEL == THERMO_MLX90615)
    DEBUG_SYS("sysGet_CurrTemp()\n");
    i2cRead_MLX90615_TempO(temp);
    return 1;
#else
	int err;
	return err;
#endif
}

/*Set the night mode(IR).
\return <0 ,if fail,
\param mode[in]: The night mode will be set for IR. //refer ENUM_NIGHT_MODE
*/
u8 sysSetNightMode(unsigned char mode)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysSetNightMode(%d)\n", mode);
  #if 0
	IR_Mode = mode;
	return 0;
  #else
    if (iconflag[UI_MENU_SETIDX_IR_MODE] != mode)
    {
        iconflag[UI_MENU_SETIDX_IR_MODE] = mode;
        Save_UI_Setting();
    }
    uiMenuSet_IR_Mode(mode);
	return 1;
  #endif
#else
	int err;
	return err;
#endif
}

/*Get the night mode(IR).
\return <0 ,if fail,
\param *mode[out]: Read the night mode from IR.  //refer ENUM_NIGHT_MODE
*/
u8 sysGetNightMode(unsigned char *mode)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
	*mode   = IR_Mode;
    DEBUG_SYS("sysGetNightMode(%d)\n", *mode);
	return 1;
#else
	int err;
	return err;
#endif
}

/*Get the status of light.
\return <0 ,if fail,
\param *value[out] : Read the value from light.
\param *status[out] : Read the status from light.	 //0x01: OFF, 0x02: Random, 0x03: value assigned
*/
u8 sysGetLight(unsigned char *CurrentValueR,unsigned char *CurrentValueG,unsigned char *CurrentValueB,unsigned char *CurrentValueL,unsigned char *status)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    *CurrentValueR  = Light_R;
    *CurrentValueG  = Light_G;
    *CurrentValueB  = Light_B;
    *CurrentValueL  = Light_L;
    *status         = Light_Status;
    DEBUG_SYS("sysGetLight(%02x,%02x,%02x,%02x,%02x)\n", *CurrentValueR, *CurrentValueG, *CurrentValueB, *CurrentValueL, *status);
    return 1;
#else
	int err;
	return err;
#endif
}
/*Set the light.
\return <0 ,if fail,
\param value[in] : Set the value for light.
\param status[in] : Set the status for light.	// 0x01: OFF, 0x02: Random, 0x03: value assigned
*/
u8 sysSetLight(unsigned char CurrentValueR,unsigned char CurrentValueG,unsigned char CurrentValueB,unsigned char CurrentValueL,unsigned char status)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysSetLight(%d,%d,%d,%d,%d)\n", CurrentValueR, CurrentValueG, CurrentValueB, CurrentValueL, status);
  #if 0
    Light_R         = CurrentValueR;
    Light_G         = CurrentValueG;
    Light_B         = CurrentValueB;
    Light_L         = CurrentValueL;
    Light_Status    = status;
    switch(status)
    {
        case 0x01:  // OFF
            i2cWrite_Byte(0x26, 0x00, 0x00);    // LED state (2:Random ON 1:LED On, 0: LED Off)
            break;
        case 0x02:  // Random
            i2cWrite_Byte(0x26, 0x00, 0x02);    // LED state (2:Random ON 1:LED On, 0: LED Off)
            break;
        case 0x03:  // Value assigned
            i2cWrite_Byte(0x26, 0x00, 0x01);    // LED state (2:Random ON 1:LED On, 0: LED Off)
            i2cWrite_Byte(0x26, 0x01, Light_R); // LED R duty (256 step)
            i2cWrite_Byte(0x26, 0x02, Light_G); // LED G duty (256 step)
            i2cWrite_Byte(0x26, 0x03, Light_B); // LED B duty (256 step)
            break;
        default:
            DEBUG_SYS("Error: sysSetLight(0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x) status can't supprot\n", CurrentValueR, CurrentValueG, CurrentValueB, CurrentValueL, status);
            return 0;
    }
    return 1;
  #else
    switch(status)
    {
        case 0x01:  // OFF
        case 0x02:  // Random
            iconflag[UI_MENU_SETIDX_LIGHT_STATUS]   = status;
            uiMenuSet_Light_Status(status);
            break;
        case 0x03:  // Value assigned
            iconflag[UI_MENU_SETIDX_LIGHT_R]        = CurrentValueR;
            iconflag[UI_MENU_SETIDX_LIGHT_G]        = CurrentValueG;
            iconflag[UI_MENU_SETIDX_LIGHT_B]        = CurrentValueB;
            iconflag[UI_MENU_SETIDX_LIGHT_L]        = CurrentValueL;
            iconflag[UI_MENU_SETIDX_LIGHT_STATUS]   = status;
            uiMenuSet_Light_R(CurrentValueR);
            uiMenuSet_Light_G(CurrentValueG);
            uiMenuSet_Light_B(CurrentValueB);
            uiMenuSet_Light_Status(status);
            break;
        default:
            DEBUG_SYS("Error: sysSetLight(0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x) status can't supprot\n", CurrentValueR, CurrentValueG, CurrentValueB, CurrentValueL, status);
            return 0;
    }
    Save_UI_Setting();
    return 1;
  #endif
#else
	int err;
	return err;
#endif
}


/*Set the record mode
\return <0 ,if fail,
\param mode[in] : Set the record mode.	//refer ENUM_RECORD_TYPE

*/
u8 sysSetRecordMode(unsigned int mode)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    u8  Status;

    DEBUG_SYS("sysSetRecordMode(%d)\n", mode);

    //RecordMode  = mode;
    if (iconflag[UI_MENU_SETIDX_RECORD_MODE] != mode)
    {
        iconflag[UI_MENU_SETIDX_RECORD_MODE] = mode;
        Save_UI_Setting();
    }
    uiMenuSet_RecordMode(mode);
    switch(mode)
    {
        case AVIOTC_RECORDTYPE_OFF:
            DEBUG_SYS("AVIOTC_RECORDTYPE_OFF\n");
            uiMenuSet_REC_MODE(UI_MENU_REC_MODE_MANUAL);
            MultiChannelSysCaptureVideoStopOneCh(1);
            MultiChannelSysCaptureVideoStopOneCh(2);
            break;
        case AVIOTC_RECORDTYPE_FULLTIME:
            DEBUG_SYS("AVIOTC_RECORDTYPE_FULLTIME\n");
            uiMenuSet_REC_MODE(UI_MENU_REC_MODE_MANUAL);
            sysGetMountSD(&Status);
            if(Status == AVIOCTRL_SDCARD_MOUNT)
            {
                if(MultiChannelGetCaptureVideoStatus(1))
                    MultiChannelSysCaptureVideoStopOneCh(1);
                MultiChannelSysCaptureVideoOneCh(1);
                if(MultiChannelGetCaptureVideoStatus(2))
                    MultiChannelSysCaptureVideoStopOneCh(2);
                MultiChannelSysCaptureVideoOneCh(2);
            }
            break;
        case AVIOTC_RECORDTYPE_ALARM:
            DEBUG_SYS("AVIOTC_RECORDTYPE_ALARM\n");
            uiMenuSet_REC_MODE(UI_MENU_REC_MODE_MOTION);
            if(MultiChannelGetCaptureVideoStatus(1))
                MultiChannelSysCaptureVideoStopOneCh(1);
            MultiChannelSysCaptureVideoOneCh(1);
            if(MultiChannelGetCaptureVideoStatus(2))
                MultiChannelSysCaptureVideoStopOneCh(2);
            MultiChannelSysCaptureVideoOneCh(2);
            break;
        case AVIOTC_RECORDTYPE_MANUAL:
            DEBUG_SYS("AVIOTC_RECORDTYPE_MANUAL\n");
            break;
        default:
            DEBUG_SYS("Error: sysSetRecordMode(%d) can't supprot\n", mode);
            break;
    }
    return 1;
#else
	int err;
	return err;
#endif
}

/*Read the record mode
\return <0 ,if fail,
\param *mode[out] : Read the record mode.	//refer ENUM_RECORD_TYPE

*/
u8 sysGetRecordMode(unsigned int *mode)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysGetRecordMode()\n");
    *mode   = RecordMode;
    DEBUG_SYS("RecordMode = %d\n", RecordMode);
    return 1;
#else
	int err;
	return err;
#endif
}

/*Mount or unmount SD card.
\return <0 ,if fail,
\param mode[in] : Mount/Unmount SD card.	//refer ENUM_SDCARDMUM_MODE
*/
u8 sysSetMountSD(unsigned char mode)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    u8  level;

    DEBUG_SYS("sysSetMountSD(%d)\n", mode);
    switch(mode)
    {
        case AVIOCTRL_SDCARD_MOUNT:
            if (iconflag[UI_MENU_SETIDX_MOUNT_MODE] != mode)
            {
                iconflag[UI_MENU_SETIDX_MOUNT_MODE] = mode;
                Save_UI_Setting();
            }
            uiMenuSet_MountMode(mode);
            level   = sysCheckSDCD();
            if((level == SDC_CD_IN) && (gInsertCard != 0))
            {
                MountMode   = AVIOCTRL_SDCARD_MOUNT;
                sysSetRecordMode(RecordMode);
            }
            else
            {
                DEBUG_SYS("Can't mount SD card, because SD card not insert!!!\n");
            }
            break;
        case AVIOCTRL_SDCARD_UNMOUNT:
            if (iconflag[UI_MENU_SETIDX_MOUNT_MODE] != mode)
            {
                iconflag[UI_MENU_SETIDX_MOUNT_MODE] = mode;
                Save_UI_Setting();
            }
            uiMenuSet_MountMode(mode);
            MountMode   = AVIOCTRL_SDCARD_UNMOUNT;
            uiMenuSet_REC_MODE(UI_MENU_REC_MODE_MANUAL);
            MultiChannelSysCaptureVideoStopOneCh(1);
            MultiChannelSysCaptureVideoStopOneCh(2);
            break;
        case AVIOCTRL_SDCARD_FORMATING:
            level   = sysCheckSDCD();
            if((level == SDC_CD_IN) && (gInsertCard != 0))
            {
                sysSet_FormatSD();
            }
            else
            {
                DEBUG_SYS("Can't format SD card, because SD card not insert!!!\n");
            }
            break;
        default:
            DEBUG_SYS("Error: sysSetMountSD(%d) not supported!!!\n", mode);
            break;
    }
    return 1;
#else
	int err;
	return err;
#endif
}

/*Check wheathe SD card is mount.
\return <0 ,if fail,
\param *mode[out] : Check wheathe SD card is mount.		//refer ENUM_SDCARDMUM_MODE

*/
u8 sysGetMountSD(unsigned char *mode)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    u8  level;

    DEBUG_SYS("sysGetMountSD()\n");
    if(MountMode == AVIOCTRL_SDCARD_MOUNT)
    {
        level   = sysCheckSDCD();
        if((level == SDC_CD_IN) && (gInsertCard != 0))
        {
            *mode   = AVIOCTRL_SDCARD_MOUNT;
            DEBUG_SYS("AVIOCTRL_SDCARD_MOUNT\n");
            return 1;
        }
    }
    *mode   = AVIOCTRL_SDCARD_UNMOUNT;
    DEBUG_SYS("AVIOCTRL_SDCARD_UNMOUNT\n");
    return 0;
#else
	int err;
	return err;
#endif
}

/*Check the status of file overwrite.
\return <0 ,if fail,
\param *status[out] : Check the status of file overwrite..		// 0x01: allow recycled,  0x02:do not allow recycled

*/
u8 sysGetFileRecycle(unsigned char  *status)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
	if(sysCaptureVideoMode & ASF_CAPTURE_OVERWRITE_ENA)
        *status = 1;
    else
        *status = 2;
    DEBUG_SYS("sysGetFileRecycle(%d)\n", *status);
	return 1;
#else
	int err;
	return err;
#endif
}

/*Set file overwrite.
\return <0 ,if fail,
\param status[in] : Set file overwrite status		//0x01: allow recycled,  0x02:do not allow recycled
*/
u8 sysSetFileRecycle(unsigned char  status)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
	int i;

    DEBUG_SYS("sysSetFileRecycle(%d)\n", status);
    if(status == 1)
    {
        if (iconflag[UI_MENU_SETIDX_OVERWRITE] != UI_MENU_SETTING_OVERWRITE_YES)
        {
            iconflag[UI_MENU_SETIDX_OVERWRITE] = UI_MENU_SETTING_OVERWRITE_YES;
            Save_UI_Setting();
        }
        uiMenuSet_Overwrite(UI_MENU_SETTING_OVERWRITE_YES);
        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            VideoClipParameter[i].sysCaptureVideoMode  |= ASF_CAPTURE_OVERWRITE_ENA;
            VideoClipOption[i].asfCaptureMode          |= ASF_CAPTURE_OVERWRITE_ENA;
        }
    }
    else if(status == 2)
    {
        if (iconflag[UI_MENU_SETIDX_OVERWRITE] != UI_MENU_SETTING_OVERWRITE_NO)
        {
            iconflag[UI_MENU_SETIDX_OVERWRITE] = UI_MENU_SETTING_OVERWRITE_NO;
            Save_UI_Setting();
        }
        uiMenuSet_Overwrite(UI_MENU_SETTING_OVERWRITE_NO);
        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            VideoClipParameter[i].sysCaptureVideoMode  &= ~ASF_CAPTURE_OVERWRITE_ENA;
            VideoClipOption[i].asfCaptureMode          &= ~ASF_CAPTURE_OVERWRITE_ENA;
        }
    }

	return 1;
#else
	int err;
	return err;
#endif
}

/*Read firmware version
\return <0 ,if fail,
\param *version[out] : Firmware version.
*/
u8 sysGetFWver(char *version)
{
    DEBUG_SYS("sysGetFWver()\n");
	memcpy(version, &uiVersion, sizeof(uiVersion));
    DEBUG_SYS("Version: %s", uiVersion);
	return 1;
}

/*Get the sensitivity value of motion detect.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *sensitivity[out]: the sensitivity value of motion detect.
*/
u8 sysGetMDSensitivity(unsigned int channel,unsigned int *sensitivity)	
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    *sensitivity    = MDSensitivity;
    DEBUG_SYS("sysGetMDSensitivity(%d,%d)\n", channel, *sensitivity);
    return 1;
#else
	int err;
	return err;
#endif
}

/*Set the sensitivity value for motion detect.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   sensitivity[in]: the sensitivity value of motion detect.
*/
u8 sysSetMDSensitivity(unsigned int channel,unsigned int sensitivity)	
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysSetMDSensitivity(%d,%d)\n", channel, sensitivity);
    if (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] != sensitivity)
    {
        iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] = sensitivity;
        Save_UI_Setting();
    }
    MDSensitivity   = sensitivity;
    uiMenuSet_MotionSensitivity(sensitivity);
    return 1;
#else
	int err;
	return err;
#endif
}
	/*Set the the TV system mode.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   mode[in]: the TV system mode.	//refer to ENUM_FREQUENCY_MODE
*/
u8 sysSetFrequency(unsigned int channel,unsigned char mode)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysSetFrequency(%d,%d)\n", channel, mode);
    switch(mode)
    {
        case AVIOCTRL_FREQUENCY_50HZ:
            uiMenuSet_Frequency(SENSOR_AE_FLICKER_50HZ);
            siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
            break;
        case AVIOCTRL_FREQUENCY_60HZ:
            uiMenuSet_Frequency(SENSOR_AE_FLICKER_60HZ);
            siuSetFlicker50_60Hz(AE_Flicker_50_60_sel);
            break;
        default:
            DEBUG_SYS("Error: sysSetFrequency(%d,%d) not supported!!!\n", channel, mode);
            return 0;
    }
    if (iconflag[UI_MENU_SETIDX_FLICKER_FREQUENCY] != AE_Flicker_50_60_sel)
    {
        iconflag[UI_MENU_SETIDX_FLICKER_FREQUENCY] = AE_Flicker_50_60_sel;
        Save_UI_Setting();
    }
    return 1;
#else
	int err;
	return err;
#endif
}	

/*Get the the TV system mode.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *mode[out]:  the TV system mode.	//refer to ENUM_FREQUENCY_MODE
*/
u8 sysGetFrequency(unsigned int channel,unsigned char *mode)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    switch(AE_Flicker_50_60_sel)
    {
        case SENSOR_AE_FLICKER_50HZ:
            *mode   = AVIOCTRL_FREQUENCY_50HZ;
            break;
        case SENSOR_AE_FLICKER_60HZ:
            *mode   = AVIOCTRL_FREQUENCY_60HZ;
            break;
        default:
            DEBUG_SYS("AE_Flicker_50_60_sel = %d not supported!!!\n", AE_Flicker_50_60_sel);
            return 0;
    }
    DEBUG_SYS("sysGetFrequency(%d)\n", *mode);
    return 1;
#else
	int err;
	return err;
#endif
}
	
/*Reset all of the setting value to default.*/
u8 sysSet_ResetToDef(void)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysSet_ResetToDef()\n");
    return 1;
#else
	int err;
	return err;
#endif
}
/*Get the high margin of temperature.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *temp[out]:  Read  the high margin of temperature.
*/
u8 sysGet_TempHighMargin(unsigned int channel,float *temp)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysGet_TempHighMargin()\n");
    *temp   = TempHighMargin;
    return 1;
#else
	int err;
	return err;
#endif
}

/*Set the high margin of temperature.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   temp[in]:  Set the high margin of temperature.
*/
u8 sysSet_TempHighMargin(unsigned int channel,float temp)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysSet_TempHighMargin()\n");
    if (TempHighMargin != temp)
    {
        TempHighMargin  = temp;
        Save_UI_Setting();
    }
    uiMenuSet_TempHighMargin();
    return 1;
#else
	int err;
	return err;
#endif
}

/*Get the low margin of temperature.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *temp[out]:  Read the low margin of temperature.
*/
u8 sysGet_TempLowMargin(unsigned int channel,float *temp)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysGet_TempLowMargin()\n");
    *temp   = TempLowMargin;
    return 1;
#else
	int err;
	return err;
#endif
}

/*Set the low margin of temperature.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   temp[in]:  Set the low margin of temperature.
*/
u8 sysSet_TempLowMargin(unsigned int channel,float temp)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysSet_TempLowMargin()\n");
    if (TempLowMargin != temp)
    {
        TempLowMargin   = temp;
        Save_UI_Setting();
    }
    uiMenuSet_TempLowMargin();
    return 1;
#else
	int err;
	return err;
#endif
}

/*Get the status of noise alert.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *status[out]:  Read the status of noise alert.	// 0x01: ON,  0x02:OFF 
*/
u8 sysGet_NoiseAlert(unsigned int channel,unsigned char *status)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    *status = NoiseAlert;
    DEBUG_SYS("sysGet_NoiseAlert(%d,%d)\n", channel, *status);
    return 1;
#else
	int err;
	return err;
#endif
}

/*Set noise alert.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   status[in]:  Set noise alert.	// 0x01: ON,  0x02:OFF 
*/
u8 sysSet_NoiseAlert(unsigned int channel,unsigned char status)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysSet_NoiseAlert(%d,%d)\n", channel, status);
    if (iconflag[UI_MENU_SETIDX_NOISE_ALERT] != status)
    {
        iconflag[UI_MENU_SETIDX_NOISE_ALERT] = status;
        Save_UI_Setting();
    }
    uiMenuSet_NoiseAlert(status);
    return 1;
#else
	int err;
	return err;
#endif
}

/*Get the status of temperature alert.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *status[out]:  Read the status of temperature alert.	// 0x01: ON,  0x02:OFF 
*/

u8 sysGet_TempAlert(unsigned int channel,unsigned char *status)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    *status = TempAlert;
    DEBUG_SYS("sysGet_TempAlert(%d,%d)\n", channel, *status);
    return 1;
#else
	int err;
	return err;
#endif
}

/*Set temperature alert.
/*Set noise alert.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   status[in]:  Set temperature alert.	// 0x01: ON,  0x02:OFF 
*/
u8 sysSet_TempAlert(unsigned int channel,unsigned char status)
{
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    DEBUG_SYS("sysSet_TempAlert(%d,%d)\n", channel, status);
    if (iconflag[UI_MENU_SETIDX_TEMP_ALERT] != status)
    {
        iconflag[UI_MENU_SETIDX_TEMP_ALERT] = status;
        Save_UI_Setting();
    }
    uiMenuSet_TempAlert(status);
    return 1;
#else
	int err;
	return err;
#endif
}

s32 uiSysMenuAction(s8 setidx)
{
#if ((HW_BOARD_OPTION  == MR8200_RX_JIT) || (HW_BOARD_OPTION  == MR8200_RX_JIT_M703SN4))
    switch (setidx)
    {
        case UI_MENU_SETIDX_VIDEO_QUALITY:
            //uiMenuSet_VideoQuality(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_VIDEO_FRAMERATE:
            //uiMenuSet_VideoFrameRate(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_RESOLUTION_CH1:
        case UI_MENU_SETIDX_RESOLUTION_CH2:
        case UI_MENU_SETIDX_RESOLUTION_CH3:
        case UI_MENU_SETIDX_RESOLUTION_CH4:
            sysSet_Resoultion(setidx-UI_MENU_SETIDX_RESOLUTION_CH1,sysConfig.CamSetting[setidx-UI_MENU_SETIDX_RESOLUTION_CH1].Resoultion);
            break;
            
        case UI_MENU_SETIDX_CH1_ON:
        case UI_MENU_SETIDX_CH2_ON:
        case UI_MENU_SETIDX_CH3_ON:
        case UI_MENU_SETIDX_CH4_ON:
            sysSet_CamerOnOff(setidx-UI_MENU_SETIDX_CH1_ON,sysConfig.CamSetting[setidx-UI_MENU_SETIDX_CH1_ON].CamerOnOff);
            break;
            
        case UI_MENU_SETIDX_BRIGHTNESS_CH1:
        case UI_MENU_SETIDX_BRIGHTNESS_CH2:
        case UI_MENU_SETIDX_BRIGHTNESS_CH3:
        case UI_MENU_SETIDX_BRIGHTNESS_CH4:
            sysSet_Brightness(setidx-UI_MENU_SETIDX_BRIGHTNESS_CH1,sysConfig.CamSetting[setidx-UI_MENU_SETIDX_BRIGHTNESS_CH1].Brightness);
            break;

        case UI_MENU_SETIDX_REC_MODE_CH1:
        case UI_MENU_SETIDX_REC_MODE_CH2:
        case UI_MENU_SETIDX_REC_MODE_CH3:
        case UI_MENU_SETIDX_REC_MODE_CH4:
            sysSet_RecMode(setidx-UI_MENU_SETIDX_REC_MODE_CH1,sysConfig.CamSetting[setidx-UI_MENU_SETIDX_REC_MODE_CH1].RecMode);
            break;

        case UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1:
        case UI_MENU_SETIDX_MOTION_SENSITIVITY_CH2:
        case UI_MENU_SETIDX_MOTION_SENSITIVITY_CH3:
        case UI_MENU_SETIDX_MOTION_SENSITIVITY_CH4:
            sysSet_MotionEnable(setidx-UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1,sysConfig.EventSetting[setidx-UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1].MotionEnable,sysConfig.EventSetting[setidx-UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1].MotionDayLevel,sysConfig.EventSetting[setidx-UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1].MotionNeightLevel);
            break;
        
        case UI_MENU_SETIDX_OVERWRITE:
            sysSet_Overwrite(sysConfig.RecSetting.Overwrite);
            break;

        case UI_MENU_SETIDX_SECTION:
            sysSet_Seccion(sysConfig.RecSetting.Seccion);
            break;

        case UI_MENU_SETIDX_MOTION_MASK:
            //uiMenuSet_MotionMask();
            break;        

        case UI_MENU_SETIDX_DATE_TIME:
            //uiMenuSet_DateTime(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_FORMAT:
            //uiMenuSet_Format(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_DISPLAY:
            sysSet_TVOut(sysConfig.SysSetting.TVOut);
            break;

        case UI_MENU_SETIDX_VOLUME:
            sysSet_Volume(sysConfig.SysSetting.Volume);
            break;

        case UI_MENU_SETIDX_ALARM:
            sysSet_AlarmEnable(sysConfig.AlarmSetting.AlarmEnable);
            break;

        case UI_MENU_SETIDX_LANGUAGE:
            sysSet_Language(sysConfig.SysSetting.Language);
            break;

        case UI_MENU_SETIDX_DEFAULT:
            //uiMenuSet_Default(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_50HZ_60HZ:
            sysSet_Flicker(sysConfig.SysSetting.Flicker);
            break;
            
    #if(TUTK_SUPPORT)
        case UI_MENU_SETIDX_P2PID:
            //uiMenuSet_P2PID(uiP2PID);
            break;
    #endif

    #if(RFIU_SUPPORT)
        case UI_MENU_SETIDX_RFID:
            //uiMenuSet_RFID(uiRFID);
            break;
        case UI_MENU_SETIDX_RFID_CODE:
            //uiMenuSet_RFID_CODE(uiRFCODE);
            break;
    #endif

    #if(NIC_SUPPORT)
        case UI_MENU_SETIDX_MAC:
            //uiMenuSet_MACAddr(uiMACAddr);
            break;
        case UI_MENU_SETIDX_NETWORK_STATUS:
            //uiMenuSet_Network();
            break;
    #endif

        case UI_MENU_SETIDX_TV_OUT:
            sysSet_TVOut(sysConfig.SysSetting.TVOut);
            break;

        case UI_MENU_SETIDX_UPGRADE_FW:
            //uiMenuSet_UpgradeFW(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_CH1_PIR:
        case UI_MENU_SETIDX_CH2_PIR:
        case UI_MENU_SETIDX_CH3_PIR:
        case UI_MENU_SETIDX_CH4_PIR:
            sysSet_PIREnable(setidx-UI_MENU_SETIDX_CH1_PIR, sysConfig.EventSetting[setidx-UI_MENU_SETIDX_CH1_PIR].PIREnable);
            break;

    #if NIC_SUPPORT
        case UI_MENU_SETIDX_P2P_PASSWORD:
            //uiMenuSet_P2P_Password();
            break;
    #endif
    
    #if 0
        case UI_MENU_SETIDX_DAY_SAVING_TIME:
            sysSet_DSTEnable(sysConfig.TimeSetting.DSTEnable);
            break;

        case UI_MENU_SETIDX_TIMEZONE:
            sysSet_TimeZone(sysConfig.TimeSetting.TimeZoneSign,sysConfig.TimeSetting.TimeZoneHour,sysConfig.TimeSetting.TimeZoneMin);
            break;

        case UI_MENU_SETIDX_DURATION:
            sysSet_Duration(sysConfig.RecSetting.Duration);
            break;

        case UI_MENU_SETIDX_ALARM_VOL:
            sysSet_AlarmVal(sysConfig.AlarmSetting.AlarmVal);
            break;
    #endif
        default:
            return 0;
    }
#endif
    return 1;
}

#if 0 //Not use yet, save code size.
u8 sysSetDeleteEventList(STimeDay date)
{
	//RTC_DATE_TIME convert_time;
	struct search_time search_dir_start,search_dir_end;
	RTC_DATE_TIME convert_time;
	u32	convert_second;
	RTC_TIME_ZONE zone;

	if(!gInsertCard)
	{
		DEBUG_P2P("No SD Card Insert.\n");
		return 1; //Fail
	}

	convert_time.year	=date.year-2000;
	convert_time.month	=date.month;
	convert_time.day	=date.day;
	convert_time.hour	=date.hour;
	convert_time.min	=date.minute;
	convert_time.sec	=date.second;
	convert_second		=RTC_Time_To_Second(&convert_time);

	if(RTC_Get_DST())	//Is DST duration?
	{
		convert_second=convert_second+3600; 
	}

	RTC_Get_TimeZone(&zone);
	if(!zone.operator)
		convert_second=convert_second+zone.hour*60*60+zone.min*60;
	else
		convert_second=convert_second-zone.hour*60*60-zone.min*60;
	RTC_Second_To_Time(convert_second, &convert_time);
	//printf("CONVERT-2 %d-%d-%d  %d:%d:%d\n",convert_time.year,convert_time.month,convert_time.day,convert_time.hour,convert_time.min,convert_time.sec);
	date.year	=convert_time.year+2000;
	date.month	=convert_time.month;
	date.day	=convert_time.day;
	date.hour	=convert_time.hour;
	date.minute	=convert_time.min;
	date.second	=convert_time.sec;	
	
DEBUG_P2P("FILE: %d/%d/%d %d:%d:%d.\n",date.year,date.month,date.day,date.hour,date.minute,date.second);	

	search_dir_start.YMD=(((date.year-1980)& 0x7F) <<9) |((date.month & 0xF) << 5) |((date.day)      & 0x1F);
	search_dir_start.HMS=(( date.hour&0x1F)<<11)        |((date.minute& 0x3F)<< 5) |((date.second/2) & 0x1F);
	search_dir_end.YMD  =(((date.year-1980)& 0x7F) <<9) |((date.month & 0xF) << 5) |((date.day)      & 0x1F);
	search_dir_end.HMS  =(( date.hour&0x1F)<<11)        |((date.minute& 0x3F)<< 5) |((date.second/2) & 0x1F);
	

//printf("search_dir_start.YMD = %d, HMS = %d\n",search_dir_start.YMD, search_dir_start.HMS);	
	dcfPlaybackCurDir = dcfListDirEntHead;

	do
	{
		if(((dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD>=search_dir_start.YMD)&&(dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD<=search_dir_end.YMD)))
		{
			if(dcfScanFileOnPlaybackDir()==0)
			{
				DEBUG_P2P("Enter folder fail!\n");
				dcfPlaybackCurDir=dcfPlaybackCurDir->next;
			}	
			else
			{
				dcfPlaybackCurFile=dcfListReadFileEntHead;
				do //Check the file time.
				{
					//if(((((search_dir_start.YMD==search_dir_end.YMD)&&(search_dir_start.YMD==dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD))&&((search_dir_start.HMS<=dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS)&&(search_dir_end.HMS>=dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS)))
					//				||(((dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD==search_dir_start.YMD)&&(dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD<search_dir_end.YMD))&&(search_dir_start.HMS<=dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS))
					//				||(((dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD>search_dir_start.YMD)&&(dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD==search_dir_end.YMD))&&(search_dir_end.HMS>=dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS))
					//				||((dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD>search_dir_start.YMD)&&(dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD<search_dir_end.YMD))))
					{
						
						if(((dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD >> 9)+1980)==date.year)
			        	if(((dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5)==date.month)
		   	          	if(((dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD & 0x001F))==date.day)
			            if(((dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS>>11))==date.hour)
			            if(((dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS & 0x07E0)>>5)==date.minute)
			            if(((dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS & 0x001F)<<1)==date.second)
			            {
			            	//printf("$");
							//printf("!!!!HMS = %d!!!!\n",dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS);
							dcfPlaybackDel();
							return 0; //Success
							//break;
				        }
					}
					//printf("!!!%d:%d!!!\n",((dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS>>11)),((dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS & 0x07E0)>>5));
					dcfPlaybackCurFile=dcfPlaybackCurFile->next;
				}while((dcfPlaybackCurFile!=dcfListReadFileEntHead));
				dcfPlaybackCurDir=dcfPlaybackCurDir->next;	
			}
		}
		else
			dcfPlaybackCurDir=dcfPlaybackCurDir->next;
	}while(dcfPlaybackCurDir!=dcfListDirEntHead);

//printf("!!!!HMS2 = %d!!!!\n",dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS);
	DEBUG_P2P("Can't Find File.\n");
	return 1; //Fail
}
#endif

#if(HOME_RF_SUPPORT)
u8 sysGetTotalSensor(void)
{
    u8 i=0;
    u8 count=0;
    
    for(i=0; i<HOMERF_SENSOR_MAX; i++)
    {
        if((gHomeRFSensorList->sSensor[i].sID !=0) && (gHomeRFSensorList->sSensor[i].sID !=0xffffffff))
            count++;
        
    }
    return count;
}

u8 sysGetTotalRoom(void)
{
    u8 i=0;
    u8 count=0;
    
    for(i=0; i<HOMERF_ROOM_MAX; i++)
    {
        if(gHomeRFRoomList->sRoom[i].roomID != 0x00)
        {
            count++;
        }
    }
    return count;
}

u8 sysGetTotalScene(void)
{
    u8 i=0;
    u8 count=0;
    
    for(i=0; i<HOMERF_SCENE_MAX; i++)
    {
        if(gHomeRFSceneList->sScene[i].sceneID != 0x00)
        {
            count++;
        }
    }
    return count;
}


u8 sysGetAppSensorType(u8 uiType)
{
    u8 id;
    
    if(uiType == HOMERF_DEVICE_DOOR)
        id=mrst_Door;
    else if(uiType == HOMERF_DEVICE_PIR)
        id=mrst_PIR;
    else if(uiType == HOMERF_DEVICE_SIREN)
        id=mrst_Siren;
    else if(uiType == HOMERF_DEVICE_IR)
        id=mrst_RemoteCtrl;
    else if(uiType == HOMERF_DEVICE_PLUG)
        id=mrst_PowerPlug;
    else if((uiType == HOMERF_DEVICE_TEMP_HUM) || (uiType == HOMERF_DEVICE_TEMP_HYG))
    //else if(uiType == HOMERF_DEVICE_TEMP_HUM)
        id=mrst_Temperature    ;    
    else if(uiType == HOMERF_DEVICE_GAS)
        id=mrst_Gas;
    else if(uiType == HOMERF_DEVICE_SMOKE)
        id=mrst_Smoke;
    else if(uiType == HOMERF_DEVICE_LEAK)
        id=mrst_Leak;
    else if(uiType == HOMERF_DEVICE_VIBRATE)
        id=mrst_Vibrate;
    else if(uiType == HOMERF_DEVICE_PANIC)
        id=mrst_Panic;
    return id;
    
}

u8 sysGetUISensorListIdx(u32 ID)
{
    u8 idx;

    for(idx=0; idx<HOMERF_SENSOR_MAX; idx++)
    {
        if(gHomeRFSensorList->sSensor[idx].sID == ID)
            break;
    }

    return idx;
}

u8 sysGetUIRoomListIdx(u32 ID)
{
    u8 idx;

    for(idx=0; idx<HOMERF_ROOM_MAX; idx++)
    {
        if(gHomeRFRoomList->sRoom[idx].roomID == ID)
            break;
    }

    return idx;
}


u8 sysGetUISceneListIdx(u32 ID)
{
    u8 idx;

    for(idx=0; idx<HOMERF_SCENE_MAX; idx++)
    {
        if(gHomeRFSceneList->sScene[idx].sceneID == ID)
            break;
    }

    return idx;
}


u8 sysGetUISensorSstaus(u8 idx)
{
    u8 status;

    if(gHomeRFSensorList->sSensor[idx].status & HOMERF_SENSOR_STATUS_ALARM)
    {
        status = mrss_Tirggered;
    }
    else if(gHomeRFSensorList->sSensor[idx].status == 0)
    {
        status = mrss_Normal;
    }

    return status;
}


u8 sysGetSensorData(u32 sID, HOMERF_SENSOR_DATA * getData)
{
    u8 idx;

    //spiReadHomeRF(SPI_HOMERF_SENSOR);
    idx=sysGetUISensorListIdx(sID);

    if(idx == HOMERF_SENSOR_MAX )
        return 0;
    memcpy(getData, &gHomeRFSensorList->sSensor[idx], sizeof(HOMERF_SENSOR_DATA));    
    return 1;
}

void sysAppGetSensorName(u32 camidx)
{
    int i,n=0;

    //spiReadHomeRF(SPI_HOMERF_SENSOR);
    for(i=0;i<HOMERF_SENSOR_MAX;i++)
    {
        if(camidx == gHomeRFSensorList->sSensor[i].sID)
            break;
        else
        {
            n++;
            //DEBUG_SYS("uiSensorList[%d].sensorID = %d\n\n",i,gHomeRFSensorList->sSensor[i].sID);
        }
    }
    strcpy(homeRFSensorName, gHomeRFSensorList->sSensor[i].name);
}



void sysAppGetSensorList(SMsgAVIoctrlGetSensorLstResp * response, u8 order)
{
    u8 i, j, totalOrder;

    //spiReadHomeRF(SPI_HOMERF_SENSOR);
    response->nTotalCount=sysGetTotalSensor();
    response->nStartIdx=order*MAXSENSOR_NUM_ONCE;

    if(response->nTotalCount == 0)
    {
        response->nCount = 0;
        return ;
    }
    if((response->nTotalCount % MAXSENSOR_NUM_ONCE) == 0 )
        totalOrder=response->nTotalCount/MAXSENSOR_NUM_ONCE;
    else
        totalOrder=response->nTotalCount/MAXSENSOR_NUM_ONCE +1;

    if(order < (totalOrder-1))
    {
        response->nCount = MAXSENSOR_NUM_ONCE;    
    }
    else if(order == (totalOrder -1))
    {
         if(response->nTotalCount % MAXSENSOR_NUM_ONCE == 0)
            response->nCount = MAXSENSOR_NUM_ONCE;
        else
            response->nCount = response->nTotalCount % MAXSENSOR_NUM_ONCE;   
    }
    else
    {
        return ;
    }

    
    for(i=0; i<response->nCount; i++)
    {   
        j = order*MAXSENSOR_NUM_ONCE + i;
        strcpy(response->sSensors[i].szName,gHomeRFSensorList->sSensor[j].name);				
        response->sSensors[i].bytePushAlarm=gHomeRFSensorList->sSensor[j].pushOnOff;  
        response->sSensors[i].byteBattery=gHomeRFSensorList->sSensor[j].battery; 
        response->sSensors[i].byteStatus=sysGetUISensorSstaus(j);
        response->sSensors[i].nSensorID=gHomeRFSensorList->sSensor[j].sID;
        response->sSensors[i].byteType=sysGetAppSensorType(gHomeRFSensorList->sSensor[j].type);				
				
        
        if(response->sSensors[i].byteType == mrst_Door)
        {
            response->sSensors[i].data.door.nIsOpen= gHomeRFSensorList->sSensor[j].data.door.isOpen;
        }
        else if(response->sSensors[i].byteType == mrst_Temperature )
        {
            response->sSensors[i].data.tp.nTemperature=gHomeRFSensorList->sSensor[j].data.Temp.value ;
            response->sSensors[i].data.tp.nHigh=gHomeRFSensorList->sSensor[j].data.Temp.high;
            response->sSensors[i].data.tp.nLow=gHomeRFSensorList->sSensor[j].data.Temp.low;
            response->sSensors[i].data.tp.nAlarmSwitch=gHomeRFSensorList->sSensor[j].data.Temp.alarmSwitch;
        }
        else if(response->sSensors[i].byteType == mrst_PowerPlug)
        {
            response->sSensors[i].data.plug.byteHasGalvanometer = gHomeRFSensorList->sSensor[j].data.plug.isSupportGal;
            response->sSensors[i].data.plug.nWattage = gHomeRFSensorList->sSensor[j].data.plug.wattage;
            response->sSensors[i].data.plug.nVoltage = gHomeRFSensorList->sSensor[j].data.plug.voltage;
            response->sSensors[i].data.plug.nCurrent = gHomeRFSensorList->sSensor[j].data.plug.current;
            response->sSensors[i].data.plug.nIsPowerOn = gHomeRFSensorList->sSensor[j].data.plug.isPowerOn;
        }
        else if(response->sSensors[i].byteType == mrst_Siren)
        {
            response->sSensors[i].data.siren.nIsRinging = gHomeRFSensorList->sSensor[j].data.siren.isRinging;
        }
        //DEBUG_SYS("UI:  ID:%x NAME:%s Type:%d Alarm %d\n",gHomeRFSensorList->sSensor[j].sID, gHomeRFSensorList->sSensor[j].name, gHomeRFSensorList->sSensor[j].type,gHomeRFSensorList->sSensor[j].pushOnOff);
        //DEBUG_SYS("APP: ID:%x NAME:%s Type:%d Alarm %d\n",response->sSensors[i].nSensorID,
        //          response->sSensors[i].szName, response->sSensors[i].byteType,response->sSensors[i].bytePushAlarm);

    }
    
}


void  sysAppGetRoomList(SMsgAVIoctrlGetRoomLstResp * response, u8 order)
{
    u8 i, j, k, totalOrder;

    //spiReadHomeRF(SPI_HOMERF_ROOM);
    response->nStartIdx=order*MAXROOM_NUM_ONCE;
    response->nTotalCount=sysGetTotalRoom();

    if(response->nTotalCount == 0)
    {
        response->nCount = 0;
        return ;
    }
    if((response->nTotalCount % MAXROOM_NUM_ONCE) == 0 )
        totalOrder=response->nTotalCount/MAXROOM_NUM_ONCE;
    else
        totalOrder=response->nTotalCount/MAXROOM_NUM_ONCE +1;
    if(order < (totalOrder-1))
    {
        response->nCount = MAXROOM_NUM_ONCE;    
    }
    else if(order == (totalOrder -1))
    {
        if(response->nTotalCount % MAXROOM_NUM_ONCE == 0) 
            response->nCount = MAXROOM_NUM_ONCE;
        else
            response->nCount = response->nTotalCount % MAXROOM_NUM_ONCE; 
    }
    else
    {
        return ;
    }
    for(i=0; i<response->nCount ; i++)
    {

        j=order*MAXROOM_NUM_ONCE+i;      
        response->sRooms[i].nRoomID=gHomeRFRoomList->sRoom[j].roomID;
        response->sRooms[i].nSensorCount=gHomeRFRoomList->sRoom[j].scount;

        strcpy(response->sRooms[i].szName,gHomeRFRoomList->sRoom[j].roomNmae);
        for(k=0; k< response->sRooms[i].nSensorCount; k++)
        {
            response->sRooms[i].nSensors[k]=gHomeRFRoomList->sRoom[j].sID[k];
        }
    }
}

void sysAppGetSceneList(SMsgAVIoctrlGetSceneLstResp * response, u8 order)
{
    u8 i, j, totalOrder;
    
	//spiReadHomeRF(SPI_HOMERF_SCENE);
    response->nStartIdx=order*MAXSCENE_NUM_ONCE; 
    response->nTotalCount=sysGetTotalScene();
	response->channel = homeRFSceneAppMode; //170215 Sean: Locate which scene setting now 

    if(response->nTotalCount == 0)
    {
        response->nCount = 0;
        return ;
    }

    if((response->nTotalCount % MAXSCENE_NUM_ONCE) == 0 )
        totalOrder=response->nTotalCount/MAXSCENE_NUM_ONCE;
    else
        totalOrder=response->nTotalCount/MAXSCENE_NUM_ONCE +1;

    if(order < (totalOrder-1))
    {
        response->nCount = MAXSCENE_NUM_ONCE;    
    }
    else if(order == (totalOrder -1))
    {
        if(response->nTotalCount % MAXSCENE_NUM_ONCE == 0)
            response->nCount = MAXSCENE_NUM_ONCE;
        else
            response->nCount = response->nTotalCount % MAXSCENE_NUM_ONCE;     
    }
    else
    {
        return ;
    }

    for(i=0; i < response->nCount; i++)
    {
        j=order*MAXSCENE_NUM_ONCE+i;
        response->sSceneHead[i].nSceneID=gHomeRFSceneList->sScene[j].sceneID; //uiSceneList[j].sceneID;
        strcpy(response->sSceneHead[i].szName,gHomeRFSceneList->sScene[j].sceneName);
    }
}


/* 0: edit success other: edit fail */
u8 sysAppEditSensor(SMsgAVIoctrlSetEditSensorReq *request)
{
    u8  idx;
    
    idx=sysGetUISensorListIdx(request->sSensor.nSensorID);
    //DEBUG_SYS("[APP] Editor sensor ID %x, APPID:%x idx:%d\n",request->sSensor.nSensorID,request->sSensor.nSensorID, idx);
    if(idx == HOMERF_SENSOR_MAX)
        return  1;

	memset(gHomeRFSensorList->sSensor[idx].name, 0, sizeof(gHomeRFSensorList->sSensor[idx].name));
    strcpy(gHomeRFSensorList->sSensor[idx].name,request->sSensor.szName);
    gHomeRFSensorList->sSensor[idx].pushOnOff=request->sSensor.bytePushAlarm;
    gHomeRFSensorList->sSensor[idx].sirenOnOff=request->sSensor.bytePushAlarm;
    if(request->sSensor.byteType==mrst_Temperature)
    {
        gHomeRFSensorList->sSensor[idx].data.Temp.high=request->sSensor.data.tp.nHigh;
        gHomeRFSensorList->sSensor[idx].data.Temp.low=request->sSensor.data.tp.nLow;
        gHomeRFSensorList->sSensor[idx].data.Temp.alarmSwitch=request->sSensor.data.tp.nAlarmSwitch;
        //DEBUG_SYS("[APP] Edit TEMP %d %d %d\n",request->sSensor.data.tp.nHigh,request->sSensor.data.tp.nLow,request->sSensor.data.tp.nAlarmSwitch);
    }
    else if(request->sSensor.byteType == mrst_PowerPlug)
    {
        if(request->sSensor.data.plug.nIsPowerOn != gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn)
        {
            if(gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn == 1)
            {
                homeRFSendToSensor(HOMERF_SEND_PLUG_OFF,idx);
                /*request->sSensor.data.plug.nIsPowerOn is wrong!!! */
                //gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn = request->sSensor.data.plug.nIsPowerOn;
                gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn = 0;
            }
            else if(gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn == 0)
            {
                homeRFSendToSensor(HOMERF_SEND_PLUG_ON,idx);
                /*request->sSensor.data.plug.nIsPowerOn is wrong!!! */
                //gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn = request->sSensor.data.plug.nIsPowerOn;
                gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn = 1;
            }
        }
        gHomeRFSensorList->sSensor[idx].data.plug.isSupportGal = request->sSensor.data.plug.byteHasGalvanometer;
        gHomeRFSensorList->sSensor[idx].data.plug.wattage = request->sSensor.data.plug.nWattage;	
        gHomeRFSensorList->sSensor[idx].data.plug.voltage = request->sSensor.data.plug.nVoltage;	
        gHomeRFSensorList->sSensor[idx].data.plug.current = request->sSensor.data.plug.nCurrent;	
        //gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn = request->sSensor.data.plug.nIsPowerOn;	
        //DEBUG_SYS("uiSensorList[%x].data.power.isPowerOn = %d \n\n",idx,gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn);   
        
    }  
    else if(request->sSensor.byteType == mrst_Siren)
    {
        if(request->sSensor.data.siren.nIsRinging != gHomeRFSensorList->sSensor[idx].data.siren.isRinging)
        {
            if(gHomeRFSensorList->sSensor[idx].data.siren.isRinging == 1)
            {
                homeRFSetSirenStatus(HOMERF_SEND_SIREN_OFF);
            }
            else if(gHomeRFSensorList->sSensor[idx].data.siren.isRinging == 0)
            {
                homeRFSetSirenStatus(HOMERF_SEND_SIREN_ON);
            }
        }
    }
    //DEBUG_SYS("[APP] Editor Name %s, APPName:%s TrigOnOff:%d\n",uiSensorList[idx].sensorName,request->sSensor.szName, uiSensorList[idx].trigerOnOff);
    spiWriteHomeRF(SPI_HOMERF_SENSOR);
    
    return 0;
    
}

/* 0: edit success other: edit fail */
u8 sysAppEditRoom(SMsgAVIoctrlSetEditRoomReq *request)
{
    u8  idx;
    u8  i;
    
    idx=sysGetUIRoomListIdx(request->sRoom.nRoomID);

    if(idx == HOMERF_ROOM_MAX)
        return  1;
    
    strcpy(gHomeRFRoomList->sRoom[idx].roomNmae, request->sRoom.szName);
    gHomeRFRoomList->sRoom[idx].scount=request->sRoom.nSensorCount;
    for(i=0; i<gHomeRFRoomList->sRoom[idx].scount; i++)
    {
        gHomeRFRoomList->sRoom[idx].sID[i]=request->sRoom.nSensors[i];
    }
    spiWriteHomeRF(SPI_HOMERF_ROOM);
    return 0;
    
}


/* 0: edit success other: edit fail */
u8 sysAppEditSceneName(SMsgAVIoctrlSetEditSceneHeadReq *request)
{
    u8  idx;

    idx=sysGetUISceneListIdx(request->sScene.nSceneID);

    if(idx == HOMERF_SCENE_MAX)
        return  1;

    strcpy(gHomeRFSceneList->sScene[idx].sceneName,request->sScene.szName);

    spiWriteHomeRF(SPI_HOMERF_SCENE);
    return 0;
    
}


/* 0: edit success other: edit fail */
u8 sysAppEditScene(SMsgAVIoctrlSetEditSceneReq * request , u8 order)
{
    u8  idx;
    u8  j, totalOrder; 
	s8	i;
    
    idx=sysGetUISceneListIdx(request->nSceneID);

    gHomeRFSceneList->sScene[idx].totalCnt=request->nTotalCount;

    j=0;
    for(i=(request->nCount)-1; i>=0; i--)
    {
        //j=request->nStartIdx+i; 
        gHomeRFSceneList->sScene[idx].sID[i]=request->sSensors[i].nSensorID; 
        gHomeRFSceneList->sScene[idx].isAlarm[i]=request->sSensors[i].byteIsSceneAffect;     
    }

//    homeRFSceneAppMode = 1; //App Mode
// Add by paul for assign default alarm status of custom scene.
    if(idx == homeRFSceneAppMode-1)
		sysAppExecuteScene(request->nSceneID);
    spiWriteHomeRF(SPI_HOMERF_SCENE);
    
    return 0;
}


/* 0: edit success other: edit fail */
u8 sysAppDeleteSensor(SMsgAVIoctrlSetDelSensorReq * request)
{
    u32 ID;
    u8  idx;
    u8  ret;
    ID=request->nSensorID;
    idx=sysGetUISensorListIdx(ID);

    if(idx == HOMERF_SENSOR_MAX)
        return  1;
    
    
    ret=homeRFDeleteSensor(idx);
    if(ret == 0)
    {
        return 1;
    }
    else
    {
        return 0;    
    }
}


/* 0: edit success other: edit fail */
u8 sysAppDeleteRoom(SMsgAVIoctrlSetDelRoomReq * request)
{
    u32 ID;
    u8  idx;
    u8  ret;
    ID=request->nRoomID;
    idx=sysGetUIRoomListIdx(ID);

    if(idx == HOMERF_ROOM_MAX)
        return  1;
    
    
    homeRFDeleteRoom(idx);

    return 0;
}


/* 0: edit success other: edit fail */
u8 sysAppDeleteScene(SMsgAVIoctrlSetDelSceneReq * request)
{
    u32 ID;
    u8  idx;
    u8  ret;
    
    ID=request->nSceneID;
    idx=sysGetUISceneListIdx(ID);
    
    if(idx == HOMERF_SCENE_MAX)
        return  1;

   	if((idx == 0) || (idx == 1) || (idx == 2)) //Sean: 20170804 Alarm, Disarm, InHome CANT be DEL.
   		return 1;
   		
    homeRFDeleteScene(idx);

    return 0;
}






u32 sysCreateID(u8 idType)
{
    u32 ret_ID=0;
    const u32 ROOM_ID=0x30000000;
    const u32 SCENE_ID=0x40000000;
    static u8 id_offset=0;
    s8 i;
    
    id_offset++;

    if(idType == APP_IDTYPE_ROOM)
    {
        ret_ID=ROOM_ID | id_offset;  
        for(i=0; i<HOMERF_ROOM_MAX; i++)
        {
            if(gHomeRFRoomList->sRoom[i].roomID == ret_ID)
            {
                id_offset++;
                ret_ID=ROOM_ID | id_offset;
                i=-1;
            }
                
        }
    }        
    else
    {
        ret_ID=SCENE_ID | id_offset;

        for(i=0; i<HOMERF_SCENE_MAX; i++)
        {
            if(gHomeRFSceneList->sScene[i].sceneID == ret_ID)
            {
                id_offset++;
                ret_ID=SCENE_ID | id_offset;
                i=-1;
            }
                
        }
    }
         

    return ret_ID;
}


u32 sysAppAddRoom(SMsgAVIoctrlSetAddRoomReq * request)
{
    u32 newID;
    u8  i,j;

    newID=sysCreateID(APP_IDTYPE_ROOM);

    for(i=0; i< HOMERF_ROOM_MAX; i++)
    {
        if(gHomeRFRoomList->sRoom[i].roomID == 0)
        {
            gHomeRFRoomList->sRoom[i].roomID =newID;    
            strcpy(gHomeRFRoomList->sRoom[i].roomNmae, request->sRoom.szName);

            gHomeRFRoomList->sRoom[i].scount=request->sRoom.nSensorCount;
            
            for(j=0; j< gHomeRFRoomList->sRoom[i].scount; j++)
            {
                gHomeRFRoomList->sRoom[i].sID[j]=request->sRoom.nSensors[j];
            }
            break ;
        }
    }


    if(i== HOMERF_ROOM_MAX)
        return 0;
    
    spiWriteHomeRF(SPI_HOMERF_ROOM);
    
    return newID;
}


u32 sysAppAddScene(SMsgAVIoctrlSetAddSceneHeadReq * request)
{
    u32 newID;
    u8  i,j;

    newID=sysCreateID(APP_IDTYPE_SCENE);

    for(i=0; i< HOMERF_SCENE_MAX; i++)
    {
        if(gHomeRFSceneList->sScene[i].sceneID == 0)
        {
            gHomeRFSceneList->sScene[i].sceneID=newID;    

            strcpy(gHomeRFSceneList->sScene[i].sceneName,request->sScene.szName);
            break ;
        }
    }


    if(i== HOMERF_SCENE_MAX)
        return 0;
    
    spiWriteHomeRF(SPI_HOMERF_SCENE);
    
    return newID;
}


u8 sysAppGetSensor(u32 sID, SMsgAVIoctrlGetSensorResp * response)
{
    u8 idx;

	//spiReadHomeRF(SPI_HOMERF_SENSOR);
    idx=sysGetUISensorListIdx(sID);
		
    strcpy(response->sSensor.szName,gHomeRFSensorList->sSensor[idx].name);
    response->sSensor.nSensorID=sID;
    
    response->sSensor.bytePushAlarm=gHomeRFSensorList->sSensor[idx].pushOnOff;
    response->sSensor.byteBattery=gHomeRFSensorList->sSensor[idx].battery;
    response->sSensor.byteStatus= sysGetUISensorSstaus(idx);
    response->sSensor.byteType=sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type);		

    if(response->sSensor.byteType == mrst_Door)
    {
        response->sSensor.data.door.nIsOpen= gHomeRFSensorList->sSensor[idx].data.door.isOpen;
    }
    else if(response->sSensor.byteType == mrst_Temperature )
    {
    	#if 1
        response->sSensor.data.tp.nTemperature=gHomeRFSensorList->sSensor[idx].data.Temp.value ;
        response->sSensor.data.tp.nHigh=gHomeRFSensorList->sSensor[idx].data.Temp.high;
        response->sSensor.data.tp.nLow=gHomeRFSensorList->sSensor[idx].data.Temp.low;
        response->sSensor.data.tp.nAlarmSwitch=gHomeRFSensorList->sSensor[idx].data.Temp.alarmSwitch;
        DEBUG_UI("**** TEMP %x  %x\n",response->sSensor.data.tp.nTemperature, gHomeRFSensorList->sSensor[idx].data.Temp.value);
		#else
        response->sSensor.data.temp.Thermometer_H = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Thermometer_H;
        response->sSensor.data.temp.Thermometer_L = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Thermometer_L;
		response->sSensor.data.temp.Hygrometer_H = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Hygrometer_H;
		response->sSensor.data.temp.Hygrometer_L = gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Hygrometer_L;
        DEBUG_UI("**** TEMP %x%x, Hyg %x%x\n",\
        gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Thermometer_H,\
        gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Thermometer_L,\
        gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Hygrometer_H,\
        gHomeRFSensorList->sSensor[idx].data.Temp_HYG.Hygrometer_L);
		#endif
    }
    else if(response->sSensor.byteType == mrst_PowerPlug)
    {
        response->sSensor.data.plug.byteHasGalvanometer = gHomeRFSensorList->sSensor[idx].data.plug.isSupportGal;
        response->sSensor.data.plug.nWattage = gHomeRFSensorList->sSensor[idx].data.plug.wattage;
        response->sSensor.data.plug.nVoltage = gHomeRFSensorList->sSensor[idx].data.plug.voltage;
        response->sSensor.data.plug.nCurrent = gHomeRFSensorList->sSensor[idx].data.plug.current;
        response->sSensor.data.plug.nIsPowerOn = gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn;     
    }
    else if(response->sSensor.byteType == mrst_Siren)
    {
        response->sSensor.data.siren.nIsRinging = gHomeRFSensorList->sSensor[idx].data.siren.isRinging;
    }

    //DEBUG_SYS("SID:  %x  index: %d  isOpen:%d Status:%d \n",sID, idx, gHomeRFSensorList->sSensor[idx].data.door.isOpen,response->sSensor.byteStatus);
    //DEBUG_SYS("UI:  ID:%x NAME:%s Type:%d Alarm %d\n",gHomeRFSensorList->sSensor[idx].sID, gHomeRFSensorList->sSensor[idx].name, gHomeRFSensorList->sSensor[idx].type,gHomeRFSensorList->sSensor[idx].pushOnOff);
    //DEBUG_SYS("APP: ID:%x NAME:%s Type:%d Alarm %d\n",response->sSensor.nSensorID, 
    //              response->sSensor.szName, response->sSensor.byteType,response->sSensor.bytePushAlarm);
}

u8 sysAppGetScene(u32 sID, SMsgAVIoctrlGetSceneResp * response, u8 order)
{
	u8 idx, totalOrder=0, i, j, ret;
	HOMERF_SENSOR_DATA tempSensor;

	spiReadHomeRF(SPI_HOMERF_SCENE);
	idx=sysGetUISceneListIdx(sID);

	if(idx >= HOMERF_SCENE_MAX) //20170531 Sean ADD.
	{
		response->nTotalCount = 0;
		response->nCount = 0;
		return ;
	}
	//response->nTotalCount=gHomeRFSceneList->sScene[idx].totalCnt;
	response->nTotalCount=gHomeRFSensorCnt; //20170531 Sean ADD.
	response->nStartIdx=order*MAXSENSOR_NUM_ONCE;

	if(response->nTotalCount == 0)
	{
		response->nCount = 0;
		return ;
	}
	
	if((response->nTotalCount % MAXSENSOR_NUM_ONCE) == 0 )
		totalOrder=response->nTotalCount/MAXSENSOR_NUM_ONCE;
	else
		totalOrder=response->nTotalCount/MAXSENSOR_NUM_ONCE +1;

	if(order < (totalOrder-1))
	{
		response->nCount = MAXSENSOR_NUM_ONCE;		  
	}
	else if(order == (totalOrder -1))
	{
		if(response->nTotalCount % MAXSENSOR_NUM_ONCE == 0)
			response->nCount = MAXSENSOR_NUM_ONCE;
		else
			response->nCount = response->nTotalCount % MAXSENSOR_NUM_ONCE;			 
	}
	else
	{
		return ;
	}

	//memcpy(response->sSensors, &tempSensor, sizeof(UI_HOMERF_SENSOR));
	
	for(i=0 ;i<response->nCount; i++)
	{
		j=order*MAXSENSOR_NUM_ONCE+i;
		memset(&tempSensor, 0, sizeof(HOMERF_SENSOR_DATA));

		//ret = sysGetSensorData(gHomeRFSceneList->sScene[idx].sID[j],&tempSensor);    
		//if(ret == 0)
		//	  continue;
		response->sSensors[i].byteIsSceneAffect=gHomeRFSceneList->sScene[idx].isAlarm[j];
		//response->sSensors[i].bytePushAlarm=tempSensor.trigerOnOff;
		//response->sSensors[i].byteBattery=tempSensor.battery;
		response->sSensors[i].byteType=sysGetAppSensorType(gHomeRFSensorList->sSensor[i].type);
		response->sSensors[i].nSensorID=gHomeRFSceneList->sScene[idx].sID[j];
		strcpy(response->sSensors[i].szName,gHomeRFSensorList->sSensor[i].name);
	}

	
}



void sysAppAddSensor(SMsgAVIoctrlSetAddSensorResp * response)
{
    u8 result;
    u8 idx;
    
    if(gAppPairFlag==APP_PAIR_SUCCESS)
        response->result=0;
    else
    {
        response->result=1;   
        gAppPairFlag=APP_PAIR_NONE;
        return; 
    }
    
    idx=sysGetUISensorListIdx(gHomeRFSensorID);
    response->sSensor.byteSameOldID=gHomeRFSensorList->sSensor[idx].byteSameOldID; //Sean: 20170612 Add.
    response->sSensor.nSensorID=gHomeRFSensorList->sSensor[idx].sID;
    response->sSensor.bytePushAlarm=gHomeRFSensorList->sSensor[idx].pushOnOff;
    response->sSensor.byteSirenAlarm=FALSE;
    response->sSensor.byteIsSceneAffect=TRUE;
    response->sSensor.byteIsHealthType=FALSE;
    response->sSensor.byteBattery=gHomeRFSensorList->sSensor[idx].battery;
    response->sSensor.byteStatus=gHomeRFSensorList->sSensor[idx].status;
    response->sSensor.byteType=sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type);
    strcpy(response->sSensor.szName,gHomeRFSensorList->sSensor[idx].name);

    if(response->sSensor.byteType == mrst_Door)
    {
        response->sSensor.data.door.nIsOpen= gHomeRFSensorList->sSensor[idx].data.door.isOpen;
    }
    else if(response->sSensor.byteType == mrst_Temperature )
    {
        response->sSensor.data.tp.nTemperature=(gHomeRFSensorList->sSensor[idx].data.Temp.value & 0xff00)>>8;
        response->sSensor.data.tp.nHigh=gHomeRFSensorList->sSensor[idx].data.Temp.high;
        response->sSensor.data.tp.nLow=gHomeRFSensorList->sSensor[idx].data.Temp.low;
    }
    else if(response->sSensor.byteType == mrst_PowerPlug)
    {
        response->sSensor.data.plug.byteHasGalvanometer = gHomeRFSensorList->sSensor[idx].data.plug.isSupportGal;
        response->sSensor.data.plug.nWattage = gHomeRFSensorList->sSensor[idx].data.plug.wattage;
        response->sSensor.data.plug.nVoltage = gHomeRFSensorList->sSensor[idx].data.plug.voltage;
        response->sSensor.data.plug.nCurrent = gHomeRFSensorList->sSensor[idx].data.plug.current;
        response->sSensor.data.plug.nIsPowerOn = gHomeRFSensorList->sSensor[idx].data.plug.isPowerOn;
    }
    else if(response->sSensor.byteType == mrst_Siren)
    {
        response->sSensor.data.siren.nIsRinging = gHomeRFSensorList->sSensor[idx].data.siren.isRinging;
    }
    gAppPairFlag=APP_PAIR_NONE;
    gHomeRFOpMode == HOMERF_OP_NOMAL;
    gHomeRFSensorID=0;
    DEBUG_SYS("ADD UI:  ID:%x NAME:%s Type:%d Alarm %d\n",gHomeRFSensorList->sSensor[idx].sID, gHomeRFSensorList->sSensor[idx].name, gHomeRFSensorList->sSensor[idx].type,gHomeRFSensorList->sSensor[idx].pushOnOff);
    DEBUG_SYS("ADD APP: ID:%x NAME:%s Type:%d Alarm %d\n",response->sSensor.nSensorID, 
          response->sSensor.szName, response->sSensor.byteType,response->sSensor.bytePushAlarm);

   // DEBUG_SYS("*APP ACK* %d, %d %x \n", idx,gHomeRFSensorCnt, uiSensorList[idx].sensorID);
}

void sysAppEnterPairMode(void)
{
    //gHomeRFOpMode= HOMERF_OP_APP_PAIR;
    homeRFSendToSensor(HOMERF_SEND_APP_PAIR, 0);
}



u8 sysAppExecuteScene(u32 sceneID)
{
    u8 i;
    u32 sceneIdx,sensorIdx;

    sceneIdx=sysGetUISceneListIdx(sceneID);
        
    for(i=0; i<gHomeRFSceneList->sScene[sceneIdx].totalCnt; i++)
    {
        sensorIdx=sysGetUISensorListIdx(gHomeRFSceneList->sScene[sceneIdx].sID[i]);
        gHomeRFSensorList->sSensor[sensorIdx].pushOnOff= gHomeRFSceneList->sScene[sceneIdx].isAlarm[i];
        gHomeRFSensorList->sSensor[sensorIdx].sirenOnOff= gHomeRFSceneList->sScene[sceneIdx].isAlarm[i];
    }
    if(sceneIdx == HOMERF_SYS_SCENE_OFF) //Sean: 20170606
    {
        printf("Set Siren OFF\n");
        UI_HA_Sensor_Beep_Flag = 0;
        homeRFSetSirenStatus(HOMERF_SIREN_OFF);
    }
    
    homeRFSceneAppMode = sceneIdx + 1;

    return 0;
}

/*
        HOMERF_SYS_SCENE_ON=0,
        HOMERF_SYS_SCENE_OFF,
        HOMERF_SYS_SCENE_HOME,
*/

void sysSetScene(u8 setting)
{
    u8 i;

    switch(setting){

    case HOMERF_SYS_SCENE_ON: // 1st Scene
        for(i=0;i<HOMERF_SENSOR_MAX;i++)
        {
            gHomeRFSensorList->sSensor[i].pushOnOff = HOMERF_SENSOR_ON;
            gHomeRFSensorList->sSensor[i].sirenOnOff= HOMERF_SENSOR_ON;
        }
    break;

    case HOMERF_SYS_SCENE_OFF: // 2nd Scene
        for(i=0;i<HOMERF_SENSOR_MAX;i++)
        {
            gHomeRFSensorList->sSensor[i].pushOnOff = HOMERF_SENSOR_OFF;
            gHomeRFSensorList->sSensor[i].sirenOnOff= HOMERF_SENSOR_OFF;
        }
    break;

    case HOMERF_SYS_SCENE_HOME: // 3rd Scene

        if(gHomeRFSceneList->sScene[2].sceneID != 0)  
            sysAppExecuteScene(gHomeRFSceneList->sScene[2].sceneID);
    break;  

    default:
    break;
    }
}

#if CDVR_iHome_LOG_SUPPORT
u8 sysAppGetSensorLog(u32 sID, SMsgAVIoctrlGetSensorLogResp * response, u32 order)
{
    u8 idx;
    int a=0,b=0,i=0,j=0,k=0,LogTatal=0, totalOrder=0,nCnt=0,LogIdx=0;
    int Nday,Nblock,Rblock,Tblock;
    int ReadSize,block_cnt,ReadBlock=0;
    RTC_DATE_TIME   convert_time;
    u32 second_num;
    char hex_sec[9],hex_EventID[5],hex_sID[9],temp_char[2];
    
    idx=sysGetUISensorListIdx(sID);

    if(!gInsertCard)
    {	
        //printf("!!!!Enter NO SD LOG!!!!\n");	
        ReadSize = dcfReadLogFile(0,0,&Rblock,&Tblock);
        if(ReadSize && (dcfLogBuf_Rd[63] == HOMERF_EVENT_LOG) && (dcfLogBuf_Rd[0] != 0) && (dcfLogBuf_Rd[1] != 0))
        {
            while(LogTatal<=(order + 20)) /*Search for match Type except Siren*/
            {
                /*A block only have Maximum 512 data*/
                if(LogIdx>=(order+20))
                    goto ReturnData;	    
                if(i>(ReadSize/64))	
                    break;
                if(sysGetAppSensorType(homeRFGetDevice(dcfLogBuf_Rd[i*64+20], dcfLogBuf_Rd[i*64+21])) == sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type))
                {
                    memset(hex_sID,0,sizeof(hex_sID));/*Convert sID*/
                    for(j=3;j>=0;j--)
                    {
                        sprintf(temp_char,"%x",dcfLogBuf_Rd[i*64+22+j]);
                        strcat(hex_sID,temp_char);
                    }
                    hex_sID[9]='\0';
                    if(strtoul(hex_sID,NULL,16) == sID)
                    {
                        if((LogIdx<MAX_LOGRECORD_NUM) && (LogTatal >= order))
                        {
                            memset(hex_sec,0,sizeof(hex_sec));/*Convert Second*/
                            for(j=3;j>=0;j--)
                            {
                                sprintf(temp_char,"%.2x",dcfLogBuf_Rd[i*64+j]);
                                strcat(hex_sec,temp_char);
                            }
                            hex_sec[9]='\0';

                            memset(hex_EventID,0,sizeof(hex_EventID));/*Convert EventStatus*/
                            for(j=1;j>=0;j--)
                            {
                                sprintf(temp_char,"%.2x",dcfLogBuf_Rd[i*64+27+j]);
                                strcat(hex_EventID,temp_char);
                            }
                            hex_EventID[5]='\0';

                            second_num = strtoul(hex_sec,NULL,16);           
                            RTC_Second_To_Time(second_num,&convert_time);
                            printf("%d-%d-%d  %d:%d:%d\n",convert_time.year,convert_time.month,convert_time.day,convert_time.hour,convert_time.min,convert_time.sec);

                            response->sRecords[LogIdx].time.year = convert_time.year+2000;
                            response->sRecords[LogIdx].time.month= convert_time.month;
                            response->sRecords[LogIdx].time.day= convert_time.day;
                            response->sRecords[LogIdx].time.hour= convert_time.hour;
                            response->sRecords[LogIdx].time.minute= convert_time.min;
                            response->sRecords[LogIdx].time.second= convert_time.sec;
                            response->sRecords[LogIdx].nPreDefEventID = strtoul(hex_EventID,NULL,16);
                            strncpy(response->sRecords[LogIdx].szName, homeRFSensorName,16);
                            LogIdx++;
                        }
                        LogTatal++;
                    }
                }
                i++;
            }
        }
    }

    else /*with SD card*/
    {
        for(a=0;a<dctTotalLogCount;a++) /*Search 7 Days*/
        {
            block_cnt = dcfGetLogFileInfo(a);
            for(b=0;b<block_cnt;b++)/*Search block*/
            {
                i = 0;
                ReadSize = dcfReadLogFile(a,b,&Rblock,&Tblock);
                // if(dcfReadLogFile(a,b,&Rblock,&Tblock))        
                if(ReadSize && (dcfLogBuf_Rd[63] == HOMERF_EVENT_LOG) && (dcfLogBuf_Rd[0] != 0) && (dcfLogBuf_Rd[1] != 0))
                {
                    i=1;
                    ReadBlock = ReadSize/64;
                    while(LogTatal<=(order + 20))
                    {
                        //if(i>(ReadSize/64))		/*A block only have Maximum 512 data*/
                        if(LogIdx>=(order+20))
                            goto ReturnData; /*Search for match Type except Siren*/
                        if(i>(ReadSize/64))	
                            break;
                        if((sysGetAppSensorType(homeRFGetDevice(dcfLogBuf_Rd[(ReadBlock-i)*64+20], dcfLogBuf_Rd[(ReadBlock-i)*64+21])) == sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type)) || (sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type) == mrst_Siren))
                        {
                            memset(hex_sID,0,sizeof(hex_sID)); /*Convert sID*/
                            for(j=3;j>=0;j--)
                            {
                                sprintf(temp_char,"%x",dcfLogBuf_Rd[(ReadBlock-i)*64+22+j]);
                                strcat(hex_sID,temp_char);
                            }
                            hex_sID[9]='\0';
                            				/*Search for match sID except Siren*/
                            if((strtoul(hex_sID,NULL,16) == sID) || (sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type) == mrst_Siren))
                            {                     
                                if((LogIdx < MAX_LOGRECORD_NUM) && (LogTatal >= order))
                                {
                                    memset(hex_sec,0,sizeof(hex_sec)); /*Convert Second*/
                                    for(j=3;j>=0;j--)
                                    {
                                        sprintf(temp_char,"%.2x",dcfLogBuf_Rd[(ReadBlock-i)*64+j]);
                                        strcat(hex_sec,temp_char);
                                    }
                                    hex_sec[9]='\0';

                                    memset(hex_EventID,0,sizeof(hex_EventID));/*Convert EventStatus*/
                                    for(j=1;j>=0;j--)
                                    {
                                        sprintf(temp_char,"%.2x",dcfLogBuf_Rd[(ReadBlock-i)*64+27+j]);
                                        strcat(hex_EventID,temp_char);
                                    }
                                    hex_EventID[5]='\0';

                                    second_num = strtoul(hex_sec,NULL,16);           
                                    RTC_Second_To_Time(second_num,&convert_time);
                                    printf("%d-%d-%d  %d:%d:%d\n",convert_time.year,convert_time.month,convert_time.day,convert_time.hour,convert_time.min,convert_time.sec);

                                    response->sRecords[LogIdx].time.year = convert_time.year+2000;
                                    response->sRecords[LogIdx].time.month= convert_time.month;
                                    response->sRecords[LogIdx].time.day= convert_time.day;
                                    response->sRecords[LogIdx].time.hour= convert_time.hour;
                                    response->sRecords[LogIdx].time.minute= convert_time.min;
                                    response->sRecords[LogIdx].time.second= convert_time.sec;
                                    response->sRecords[LogIdx].nPreDefEventID = strtoul(hex_EventID,NULL,16);
                                    strncpy(response->sRecords[LogIdx].szName, homeRFSensorName, 16);
                                    LogIdx++;
                                }
                                LogTatal++;
                            }
                        }
                        i++;
                    }
                }
                else
                    continue;
            }
        }
    }
    
    ReturnData:

    response->nStartIdx = order;	
    response->nTotalCount = LogTatal;

    if(response->nTotalCount == 0)
    {
        response->nCount = 0;			 
        return ;
    }
    if((LogTatal % MAX_LOGRECORD_NUM) == 0)
        totalOrder = LogTatal/MAX_LOGRECORD_NUM;
    else
        totalOrder = LogTatal/MAX_LOGRECORD_NUM + 1;

    if((order/MAX_LOGRECORD_NUM) < (totalOrder-1))
    {
        response->nCount = MAX_LOGRECORD_NUM;
    }
    else if((order/MAX_LOGRECORD_NUM) == (totalOrder -1))
    {
        if(response->nTotalCount % MAX_LOGRECORD_NUM == 0)
        {
            response->nCount = MAX_LOGRECORD_NUM;
        }
        else
        {
            response->nCount = response->nTotalCount % MAX_LOGRECORD_NUM; 
        }
    }
    else
    {
        return ;
    }
/*
printf("!!nStartIdx:%d\n",order);
printf("!!nTotalCount:%d\n",response->nTotalCount);
printf("!!nCount:%d\n",response->nCount);
*/
}

u8 sysGetSensorLog(u8 Idx, HOMERF_SensorLogList * response, unsigned int order) //Idx: 0->On this LogPage FIRST option, 7->On 2nd LogPage FIRST option ; Order : Start from 0, 7, 14,...
{
    int b=0,i=0,j=0,LogTatal=0, totalOrder=0,LogIdx=0,day=0, ReadBlock=0, LogFileInfo=0, ReadLogFileSize=0;
    int Rblock,Tblock;
    u32 ReadSize;
    u16 ReadNonZero = 0;
    RTC_DATE_TIME   convert_time;
    u32 second_num;
    char hex_sec[9],hex_EventID[5],hex_sID[9],temp_char[2];
    u8  nSensorType=0;

    /*換算使用者點入Log頁面對應哪一天 Day*/
    while(day < Idx)
    {
        if(dcfGetLogFileInfo(i)>0)
            day++;
        i++;
    }
    LogFileInfo = dcfGetLogFileInfo(day);
     for(i=0;i<LogFileInfo;i++)
    {    
        ReadLogFileSize = dcfReadLogFile(day,i,&Rblock,&Tblock)/64;
        for(j=0;j<ReadLogFileSize;j++)
        {
            //if((dcfLogBuf_Rd[j*64] == 0) && (dcfLogBuf_Rd[j*64+1] == 0) || (dcfLogBuf_Rd[j*64+62] != 0))
            //    continue;
            if((dcfLogBuf_Rd[j*64] != 0) && (dcfLogBuf_Rd[j*64+1] != 0) && (dcfLogBuf_Rd[j*64+63] == HOMERF_EVENT_LOG))
                ReadNonZero++;
        }
    }
    /*回傳目前該天數有多少筆LOG*/
    response->nTotalCount = ReadNonZero;
    //response->nTotalCount = (dcfGetLogFileInfo(day)-1)*512 + dcfReadLogFile(day,0,&Rblock,&Tblock)/64;

    for(b=0;b<LogFileInfo;b++)/*Search block*/
    {
        ReadSize = dcfReadLogFile(day,b,&Rblock,&Tblock);  

        if(ReadSize <= 0)
            continue;

        ReadBlock = ReadSize/64;
        i = 1;
        
        while((LogIdx<8) && ReadSize>0)  /* 一頁只能顯示7筆資訊 */
        {   
            if(i > ReadBlock)
                break;
            if((dcfLogBuf_Rd[(ReadBlock-i)*64] != 0 && dcfLogBuf_Rd[(ReadBlock-i)*64+1] != 0) && dcfLogBuf_Rd[(ReadBlock-i)*64+63] == HOMERF_EVENT_LOG)
            {
                if(LogTatal >= order)
                {
                    memset(hex_sec,0,sizeof(hex_sec)); /*Convert Second*/
                    for(j=3;j>=0;j--)
                    {
                        sprintf(temp_char,"%.2x",dcfLogBuf_Rd[(ReadBlock-i)*64+j]);
                        strcat(hex_sec,temp_char);
                    }
                    hex_sec[9]='\0';

                    memset(hex_EventID,0,sizeof(hex_EventID));/*Convert EventStatus*/
                    for(j=1;j>=0;j--)
                    {
                        sprintf(temp_char,"%.2x",dcfLogBuf_Rd[(ReadBlock-i)*64+27+j]);
                        strcat(hex_EventID,temp_char);
                    }
                    hex_EventID[5]='\0';

                    memset(homeRFSensorName,0,sizeof(homeRFSensorName));/*Convert SensorName*/
                    for(j=0;j<16;j++)
                    {
                        sprintf(temp_char,"%c",dcfLogBuf_Rd[(ReadBlock-i)*64+4+j]);
                        strcat(homeRFSensorName,temp_char);
                    }
                    homeRFSensorName[17]='\0';                    

                    second_num = strtoul(hex_sec,NULL,16);           
                    RTC_Second_To_Time(second_num,&convert_time);
                    //printf("%d-%d-%d  %d:%d:%d\n",convert_time.year,convert_time.month,convert_time.day,convert_time.hour,convert_time.min,convert_time.sec);

                    response->sRecords[LogIdx].time.year = convert_time.year+2000;
                    response->sRecords[LogIdx].time.month= convert_time.month;
                    response->sRecords[LogIdx].time.day= convert_time.day;
                    response->sRecords[LogIdx].time.hour= convert_time.hour;
                    response->sRecords[LogIdx].time.minute= convert_time.min;
                    response->sRecords[LogIdx].time.second= convert_time.sec;
                    strncpy(response->sRecords[LogIdx].szName, homeRFSensorName, 16);
                    //response->sRecords[LogIdx].nSensorType= homeRFGetDevice(dcfLogBuf_Rd[(ReadBlock-i)*64+20], dcfLogBuf_Rd[(ReadBlock-i)*64+21]);
					nSensorType = homeRFGetDevice(dcfLogBuf_Rd[(ReadBlock-i)*64+20], dcfLogBuf_Rd[(ReadBlock-i)*64+21]);

					if(nSensorType == HOMERF_DEVICE_SIREN)
						response->sRecords[LogIdx].nSensorType = 4;
					else if(nSensorType == HOMERF_DEVICE_IR)
						response->sRecords[LogIdx].nSensorType = 2;
					else
						response->sRecords[LogIdx].nSensorType = nSensorType;
                    if(strtoul(hex_EventID,NULL,16))                    
                        response->sRecords[LogIdx].nPreDefEventID = 0; //Trigger Event
                    else 
                    { 
                        if(dcfLogBuf_Rd[(ReadBlock-i)*64+29] == 0)
                            response->sRecords[LogIdx].nPreDefEventID = 2; //Lost Link                        
                        else if(dcfLogBuf_Rd[(ReadBlock-i)*64+29] <= 20) //Battery Value
                            response->sRecords[LogIdx].nPreDefEventID = 1; //Low Battery
                        else
                            response->sRecords[LogIdx].nPreDefEventID = 0; //Trigger Event
                    }      
                    LogIdx++;
                }
                LogTatal++;
            }
            i++;
        }

    }

    response->nStartIdx = order;

    if(response->nTotalCount == 0)
    {
        response->nCount = 0;			 
        return ;
    }

    if((order+MAX_UI_LOGRECORD_NUM) <= response->nTotalCount)
        response->nCount = MAX_UI_LOGRECORD_NUM;
    else if((order+MAX_UI_LOGRECORD_NUM) > response->nTotalCount)
        response->nCount = response->nTotalCount % MAX_UI_LOGRECORD_NUM; 
    else
        return;
}



u8 sysGetSensorLogDayList(HOMERF_SensorLogDayList * response, u8 order)
{
    int a=0,b=0,i=0,j=0,LogTatal=0, totalOrder=0,LogIdx=0, LogFileInfo=0;
    int Rblock,Tblock;
    u32 ReadSize;
    RTC_DATE_TIME   convert_time;
    u32 second_num;
    char hex_sec[9],hex_EventID[5],hex_sID[9],temp_char[2];
	u8 exist_flag=0;

	if (!gInsertCard)
	{
		DEBUG_SYS("No SD Card Insert.\n");
		response->nTotalCount = 0;
        response->nCount = 0;
		return;
	}		
	
    for(a=0;a<dctTotalLogCount;a++) /*Search 7 Days*/
    {
    	exist_flag=0;
    	LogFileInfo = dcfGetLogFileInfo(a);		
		
		if(LogFileInfo < 0)	/* SD Card may be fail*/
		{
			DEBUG_SYS("SD Card LogFileInfo Fail %d.\n", LogFileInfo);
			response->nTotalCount = 0;
	        response->nCount = 0;
			return;
		}		
		if(LogFileInfo == 0)
			continue;
		
		for(b=0;b<LogFileInfo;b++)
		{
			ReadSize = dcfReadLogFile(a,b,&Rblock,&Tblock);
			//if(ReadSize && (dcfLogBuf_Rd[63] == HOMERF_EVENT_LOG) && (dcfLogBuf_Rd[0] != 0) && (dcfLogBuf_Rd[1] != 0))
	        if(ReadSize)
	        {           
				for(i=0;i<ReadSize/64;i++)
				{
					if((dcfLogBuf_Rd[i*64] == 0) && (dcfLogBuf_Rd[i*64+1] == 0) || (dcfLogBuf_Rd[i*64+63] != HOMERF_EVENT_LOG))
					{
						continue;
					}
					
					else
					{
						if((LogIdx < MAX_UI_LOGRECORD_NUM) && (LogTatal >= order))
						{		
						
							memset(hex_sec,0,sizeof(hex_sec)); /*Convert Second*/
							for(j=3;j>=0;j--)
							{
								sprintf(temp_char,"%.2x",dcfLogBuf_Rd[j+i*64]);
								strcat(hex_sec,temp_char);
							}
							hex_sec[9]='\0';
		
							second_num = strtoul(hex_sec,NULL,16);			 
							RTC_Second_To_Time(second_num,&convert_time);
							response->sRecords[LogIdx].time.year = convert_time.year+2000;
							response->sRecords[LogIdx].time.month= convert_time.month;
							response->sRecords[LogIdx].time.day= convert_time.day;
							response->sRecords[LogIdx].time.hour= convert_time.hour;
							response->sRecords[LogIdx].time.minute= convert_time.min;
							response->sRecords[LogIdx].time.second= convert_time.sec;
							LogIdx++;
							exist_flag=1;
						}
						break;
					}
				}
				if(exist_flag)
					break;
	        }
	        else     
	            continue;
		}
            
        LogTatal++;
    }
    response->nTotalCount = LogTatal;
    
    if(response->nTotalCount == 0)
    {
        response->nCount = 0;			 
        return ;
    }
    if((LogTatal % MAX_UI_LOGRECORD_NUM) == 0)
        totalOrder = LogTatal/MAX_UI_LOGRECORD_NUM;
    else
        totalOrder = LogTatal/MAX_UI_LOGRECORD_NUM + 1;

    if((order/MAX_UI_LOGRECORD_NUM) < (totalOrder-1))
    {
        response->nCount = MAX_UI_LOGRECORD_NUM;
    }
    else if((order/MAX_UI_LOGRECORD_NUM) == (totalOrder -1))
    {
        if(response->nTotalCount % MAX_UI_LOGRECORD_NUM == 0)
        {
            response->nCount = MAX_UI_LOGRECORD_NUM;
        }
        else
        {
            response->nCount = response->nTotalCount % MAX_UI_LOGRECORD_NUM; 
        }
    }
    else
    {
        return ;
    } 
}

void sysDeleteLog(u8 Idx)
{
    int b=0,i=0,j=0,LogTatal=0, totalOrder=0,LogIdx=0,day=0;
    
    /*換算使用者點入Log頁面對應哪一天 Day*/
    while(day < Idx)
    {
        if(dcfGetLogFileInfo(i)>0)
            day++;
        i++;
    }

    dcfLogFileUsrDel(day);
}

void sysDeleteAllLog(void)
{
    int Day=0;
    Day = dctTotalLogCount;
	while(Day>1)
	{
		if(dcfGetLogFileInfo(Day)>0)
        {
            dcfLogFileUsrDel(Day);
			Day--;
        }	
	}
}
#endif

#if SYSTEM_DEBUG_SD_LOG	
void sysUartLogRecord(u8 buf)
{

    FS_FILE* pFile;
    char 	 TargetName[64];
	u8		 tmp;
    u32 	 writesize;
    INT8U    err;

	if (gInsertCard)
	{
		OSSemPend(dcfReadySemEvt, OS_IPC_WAIT_FOREVER, &err);		 

		memset(TargetName, 0, sizeof(TargetName));
		strcpy((char*)TargetName, "\\DEBUGLOG.txt");

		if ((pFile = dcfOpen((s8 *)TargetName, "a+")) == NULL)
		{
			 DEBUG_DCF("!!!Error: create Test Log %s error!!!\n", TargetName);

		}
		if(buf == 0)
			dcfWrite(pFile, DEBUG_SD_BUF1, sizeof(DEBUG_SD_BUF1), &writesize);
		else
			dcfWrite(pFile, DEBUG_SD_BUF2, sizeof(DEBUG_SD_BUF2), &writesize);
		
		dcfClose(pFile, &tmp);
		OSSemPost(dcfReadySemEvt);
	}
}
#endif //end #if SYSTEM_DEBUG_SD_LOG

#endif

