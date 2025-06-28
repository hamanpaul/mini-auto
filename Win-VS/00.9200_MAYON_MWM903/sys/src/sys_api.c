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
#include "rfiuapi.h"
#if (TUTK_SUPPORT)
#include "p2pserver_api.h"
#endif

#include "mpeg4api.h"
#include "../../ui/inc/ui.h"
#include "../../ui/inc/ui_project.h"
#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
#if (TUTK_SUPPORT==1)
#if LWIP2_SUPPORT
#include "../LwIP_2.0/include/tutk_P2P/AVIOCTRLDEFs.h"
#else
#include "../LwIP/include/tutk_P2P/AVIOCTRLDEFs.h"
#endif
#endif

#if (HOME_RF_SUPPORT)
#if LWIP2_SUPPORT
#include "../LwIP_2.0/include/tutk_P2P/MR8200def_homeautomation.h"
#else
#include "../LwIP/include/tutk_P2P/MR8200def_homeautomation.h"
#endif
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
u8 homeRFSensorName[18];
#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
u8  SystemLogData[SYSTEM_LOG_DATA_SIZE];
#endif

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern  UI_NET_INFO UINetInfo;
extern u8  homeRFSceneAppMode;
#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
extern u32 dctTotalLogCount;
#endif
/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */
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

    for(i=0; i<MULTI_CHANNEL_MAX; i++)
    {
        sysConfig.CamSetting[i].Brightness  = 4; //UI_MENU_SETTING_BRIGHTNESS_LV5
        sysConfig.CamSetting[i].CamerOnOff  = 1; //UI_MENU_SETTING_CAMERA_ON
        sysConfig.CamSetting[i].RecMode     = 0; //UI_MENU_REC_MODE_MANUAL
        sysConfig.CamSetting[i].Resoultion  = 1; //UI_MENU_SETTING_RESOLUTION_VGA
    }

    for(i=0; i<MULTI_CHANNEL_MAX; i++)
    {
        sysConfig.EventSetting[i].EventRECTime     = 0;
        sysConfig.EventSetting[i].MotionEnable     = 0;
        sysConfig.EventSetting[i].MotionDayLevel   = 0;
        sysConfig.EventSetting[i].MotionNeightLevel = 0;
        sysConfig.EventSetting[i].PIREnable        = 0;
    }

    sysConfig.NetSetting.DHCPEnable = 1;
    for(i=0; i<4; i++)
    {
        sysConfig.NetSetting.IPAddr[i]  = 0;
        sysConfig.NetSetting.Gateway[i] = 0;
        sysConfig.NetSetting.NetMask[i] = 0;
    }

    for(i=0; i<MULTI_CHANNEL_MAX; i++)
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
    return 1;
}

u8 sysSet_Duration(u32 pData)
{
#if MULTI_CHANNEL_VIDEO_REC
    u8 i = 0;
    //i = i; // Avoid warming msg
#endif
    sysConfig.RecSetting.Duration=pData;
#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
    //iconflag[UI_MENU_SETIDX_DURATION] = sysConfig.RecSetting.Duration;
    if (Main_Init_Ready == 1)
        uiCaptureVideoStop();

#if MULTI_CHANNEL_VIDEO_REC
    for(i=0; i<MULTI_CHANNEL_MAX; i++)
    {
        VideoClipParameter[i].asfRecTimeLen = sysConfig.RecSetting.Duration;
    }
#endif
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
}

u8 sysSet_Schedule(u8 nCam, u8 nDay, u8 nHour, u8 pData)
{
    return 1;
}

void sysGet_MotionEnable(u8 nCam, u8* pEnable, u8* pDay, u8* pNight)
{
    *pEnable=sysConfig.EventSetting[nCam].MotionEnable;
    *pDay=sysConfig.EventSetting[nCam].MotionDayLevel;
    *pNight=sysConfig.EventSetting[nCam].MotionNeightLevel;
}

u8 sysSet_MotionEnable(u8 nCam, u8 pEnable, u8 pDay, u8 pNight)
{
    u8 nRet=0;
    sysConfig.EventSetting[nCam].MotionEnable=pEnable;
    sysConfig.EventSetting[nCam].MotionDayLevel=pDay;
    sysConfig.EventSetting[nCam].MotionNeightLevel=pNight;
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
#elif ((UI_VERSION == UI_VERSION_TRANWO) && (UI_SUPPORT_DST_TIME))
    iconflag[UI_MENU_SETIDX_DST_TIME] = sysConfig.TimeSetting.DSTEnable;
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

#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
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
        for(i=0; i<4; i++)
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
    sysConfig.SysSetting.TVOut=pData;
    return 1;
}

void sysGet_Language(u8* pData)
{
    *pData=sysConfig.SysSetting.Language;
}

u8 sysSet_Language(u8 pData)
{
    sysConfig.SysSetting.Language=pData;
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


    if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_SDC)
    {
        if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
            return 0;
    }
    else
    {
        if(sysGetStorageStatus(SYS_I_STORAGE_BACKUP) == SYS_V_STORAGE_NREADY)
            return 0;
    }

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
    int err = 0;
    return err;
}

/*Get the night mode(IR).
\return <0 ,if fail,
\param *mode[out]: Read the night mode from IR.  //refer ENUM_NIGHT_MODE
*/
u8 sysGetNightMode(unsigned char *mode)
{
    int err = 0;
    return err;
}

/*Get the status of light.
\return <0 ,if fail,
\param *value[out] : Read the value from light.
\param *status[out] : Read the status from light.	 //0x01: OFF, 0x02: Random, 0x03: value assigned
*/
u8 sysGetLight(unsigned char *CurrentValueR,unsigned char *CurrentValueG,unsigned char *CurrentValueB,unsigned char *CurrentValueL,unsigned char *status)
{
    int err = 0;
    return err;
}
/*Set the light.
\return <0 ,if fail,
\param value[in] : Set the value for light.
\param status[in] : Set the status for light.	// 0x01: OFF, 0x02: Random, 0x03: value assigned
*/
u8 sysSetLight(unsigned char CurrentValueR,unsigned char CurrentValueG,unsigned char CurrentValueB,unsigned char CurrentValueL,unsigned char status)
{
    int err = 0;
    return err;
}


/*Set the record mode
\return <0 ,if fail,
\param mode[in] : Set the record mode.	//refer ENUM_RECORD_TYPE

*/
u8 sysSetRecordMode(unsigned int mode)
{
    int err = 0;
    return err;
}

/*Read the record mode
\return <0 ,if fail,
\param *mode[out] : Read the record mode.	//refer ENUM_RECORD_TYPE

*/
u8 sysGetRecordMode(unsigned int *mode)
{
    int err = 0;
    return err;
}

/*Mount or unmount SD card.
\return <0 ,if fail,
\param mode[in] : Mount/Unmount SD card.	//refer ENUM_SDCARDMUM_MODE
*/
u8 sysSetMountSD(unsigned char mode)
{
    int err = 0;
    return err;
}

/*Check wheathe SD card is mount.
\return <0 ,if fail,
\param *mode[out] : Check wheathe SD card is mount.		//refer ENUM_SDCARDMUM_MODE

*/
u8 sysGetMountSD(unsigned char *mode)
{
    int err = 0;
    return err;
}

/*Check the status of file overwrite.
\return <0 ,if fail,
\param *status[out] : Check the status of file overwrite..		// 0x01: allow recycled,  0x02:do not allow recycled

*/
u8 sysGetFileRecycle(unsigned char  *status)
{
    int err = 0;
    return err;
}

/*Set file overwrite.
\return <0 ,if fail,
\param status[in] : Set file overwrite status		//0x01: allow recycled,  0x02:do not allow recycled
*/
u8 sysSetFileRecycle(unsigned char  status)
{
    int err = 0;
    return err;
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
    int err = 0;
    return err;
}

/*Set the sensitivity value for motion detect.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   sensitivity[in]: the sensitivity value of motion detect.
*/
u8 sysSetMDSensitivity(unsigned int channel,unsigned int sensitivity)
{
    int err = 0;
    return err;
}
/*Set the the TV system mode.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   mode[in]: the TV system mode.	//refer to ENUM_FREQUENCY_MODE
*/
u8 sysSetFrequency(unsigned int channel,unsigned char mode)
{
    int err = 0;
    return err;
}

/*Get the the TV system mode.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *mode[out]:  the TV system mode.	//refer to ENUM_FREQUENCY_MODE
*/
u8 sysGetFrequency(unsigned int channel,unsigned char *mode)
{
    int err = 0;
    return err;
}

/*Reset all of the setting value to default.*/
u8 sysSet_ResetToDef(void)
{
    int err = 0;
    return err;
}
/*Get the high margin of temperature.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *temp[out]:  Read  the high margin of temperature.
*/
u8 sysGet_TempHighMargin(unsigned int channel,float *temp)
{
    int err = 0;
    return err;
}

/*Set the high margin of temperature.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   temp[in]:  Set the high margin of temperature.
*/
u8 sysSet_TempHighMargin(unsigned int channel,float temp)
{
    int err = 0;
    return err;
}

/*Get the low margin of temperature.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *temp[out]:  Read the low margin of temperature.
*/
u8 sysGet_TempLowMargin(unsigned int channel,float *temp)
{
    int err = 0;
    return err;
}

/*Set the low margin of temperature.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   temp[in]:  Set the low margin of temperature.
*/
u8 sysSet_TempLowMargin(unsigned int channel,float temp)
{
    int err = 0;
    return err;
}

/*Get the status of noise alert.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *status[out]:  Read the status of noise alert.	// 0x01: ON,  0x02:OFF
*/
u8 sysGet_NoiseAlert(unsigned int channel,unsigned char *status)
{
    int err = 0;
    return err;
}

/*Set noise alert.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   status[in]:  Set noise alert.	// 0x01: ON,  0x02:OFF
*/
u8 sysSet_NoiseAlert(unsigned int channel,unsigned char status)
{
    int err = 0;
    return err;
}

/*Get the status of temperature alert.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   *status[out]:  Read the status of temperature alert.	// 0x01: ON,  0x02:OFF
*/

u8 sysGet_TempAlert(unsigned int channel,unsigned char *status)
{
    int err = 0;
    return err;
}

/*
Set temperature alert.
Set noise alert.
\return <0 ,if fail,
\param channel[in]: Camera index.
\	   status[in]:  Set temperature alert.	// 0x01: ON,  0x02:OFF
*/
u8 sysSet_TempAlert(unsigned int channel,unsigned char status)
{
    int err = 0;
    return err;
}

s32 uiSysMenuAction(s8 setidx)
{
    return 1;
}

#if(HOME_RF_SUPPORT)
u8 sysGetTotalSensor(void)
{
    u8 i=0;
    u8 count=0;

    for(i=0; i<HOMERF_SENSOR_MAX; i++)
    {
        if((i != 27) && (i != 26) &&(gHomeRFSensorList->sSensor[i].sID !=0) && (gHomeRFSensorList->sSensor[i].sID !=0xffffffff))
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
    u8 id = 0;

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
//    else if(uiType == HOMERF_DEVICE_TEMP_HYG)
//        id=mrst_TEMP_HYG;
    
    return id;

}

u8 sysGetUISensorListIdx(u32 ID)
{
    u8 idx;

    for(idx=0; idx<HOMERF_SENSOR_MAX; idx++)
    {
//    printf("sID:%d\n", gHomeRFSensorList->sSensor[idx].sID);
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

    switch(gHomeRFSensorList->sSensor[idx].type)
    {

        case HOMERF_DEVICE_DOOR:
            status = (gHomeRFSensorList->sSensor[idx].data.door.isOpen)?
                mrss_Tirggered: mrss_Normal;
            break;
        case HOMERF_DEVICE_SIREN:
            status = (gHomeRFSensorList->sSensor[idx].data.siren.isRinging)?
                mrss_Tirggered: mrss_Normal;
            break;
        case HOMERF_DEVICE_IR:
        case HOMERF_DEVICE_TEMP_HUM:
        case HOMERF_DEVICE_PIR:
        default:
            status = (gHomeRFSensorList->sSensor[idx].status & HOMERF_SENSOR_STATUS_ALARM)?
                mrss_Tirggered: mrss_Normal;
            break;
    }

    return status;
}


u8 sysGetSensorData(u32 sID, HOMERF_SENSOR_DATA * getData)
{
    u8 idx;

    idx=sysGetUISensorListIdx(sID);

    if(idx == HOMERF_SENSOR_MAX )
        return 0;
    memcpy(getData, &gHomeRFSensorList->sSensor[idx], sizeof(HOMERF_SENSOR_DATA));
    return 1;
}

void sysAppGetSensorName(u32 camidx)
{
    int i,n=0;

    for(i=0; i<HOMERF_SENSOR_MAX; i++)
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
//        DEBUG_BLUE("UI:  ID:%x NAME:%s Type:%d Alarm %d\n",gHomeRFSensorList->sSensor[j].sID, gHomeRFSensorList->sSensor[j].name, gHomeRFSensorList->sSensor[j].type,gHomeRFSensorList->sSensor[j].pushOnOff);
        //DEBUG_BLUE("APP: ID:%x NAME:%s Type:%d Alarm %d\n",response->sSensors[i].nSensorID,
        //          response->sSensors[i].szName, response->sSensors[i].byteType,response->sSensors[i].bytePushAlarm);

    }

}


void  sysAppGetRoomList(SMsgAVIoctrlGetRoomLstResp * response, u8 order)
{
    u8 i, j, k, totalOrder;

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

    response->nStartIdx=order*MAXSCENE_NUM_ONCE;
    response->nTotalCount=sysGetTotalScene();

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
u8 sysAppEditSensor(u32 sID, u8 *name)
{
    u8  idx;

    idx=sysGetUISensorListIdx(sID);
    //DEBUG_SYS("[APP] Editor sensor ID %x, APPID:%x idx:%d\n",request->sSensor.nSensorID,request->sSensor.nSensorID, idx);
    if(idx == HOMERF_SENSOR_MAX)
        return  1;

    memset(gHomeRFSensorList->sSensor[idx].name, 0, sizeof(gHomeRFSensorList->sSensor[idx].name));
    strcpy(gHomeRFSensorList->sSensor[idx].name,name);

//printf("name:%s\n",gHomeRFSensorList->sSensor[idx].name);
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
        j=request->nStartIdx+i;
        gHomeRFSceneList->sScene[idx].sID[j]=request->sSensors[i].nSensorID;
        gHomeRFSceneList->sScene[idx].isAlarm[j]=request->sSensors[i].byteIsSceneAffect;
    }

//    homeRFSceneAppMode = 1; //App Mode

    spiWriteHomeRF(SPI_HOMERF_SCENE);

    return 0;
}


/* 0: edit success other: edit fail */
u8 sysAppDeleteSensor(u32 ID)
{
    u8  idx;
    u8  ret;
    //ID=request->nSensorID;
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
        response->sSensor.data.tp.nTemperature=gHomeRFSensorList->sSensor[idx].data.Temp.value ;
        response->sSensor.data.tp.nHigh=gHomeRFSensorList->sSensor[idx].data.Temp.high;
        response->sSensor.data.tp.nLow=gHomeRFSensorList->sSensor[idx].data.Temp.low;
        response->sSensor.data.tp.nAlarmSwitch=gHomeRFSensorList->sSensor[idx].data.Temp.alarmSwitch;
        DEBUG_UI("**** TEMP %x  %x\n",response->sSensor.data.tp.nTemperature, gHomeRFSensorList->sSensor[idx].data.Temp.value);
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
    else if(response->sSensor.byteType == mrst_TEMP_HYG)
    {
//        response->sSensor.data.temp = gHomeRFSensorList->sSensor[idx].data.Temp_HYG;
    }

    DEBUG_SYS("SID:  %x  index: %d  isOpen:%d Status:%d \n",sID, idx, gHomeRFSensorList->sSensor[idx].data.door.isOpen,response->sSensor.byteStatus);
    DEBUG_SYS("UI:  ID:%x NAME:%s Type:%d Alarm %d\n",gHomeRFSensorList->sSensor[idx].sID, gHomeRFSensorList->sSensor[idx].name, gHomeRFSensorList->sSensor[idx].type,gHomeRFSensorList->sSensor[idx].pushOnOff);
    DEBUG_SYS("APP: ID:%x NAME:%s Type:%d Alarm %d\n",response->sSensor.nSensorID,
              response->sSensor.szName, response->sSensor.byteType,response->sSensor.bytePushAlarm);
}

u8 sysAppGetScene(u32 sID, SMsgAVIoctrlGetSceneResp * response, u8 order)
{
    u8 idx, totalOrder=0, i, j, ret;
    HOMERF_SENSOR_DATA tempSensor;

    idx=sysGetUISceneListIdx(sID);

    response->nTotalCount=gHomeRFSceneList->sScene[idx].totalCnt;
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

    for(i=0 ; i<response->nCount; i++)
    {
        j=order*MAXSENSOR_NUM_ONCE+i;
        memset(&tempSensor, 0, sizeof(HOMERF_SENSOR_DATA));

        ret = sysGetSensorData(gHomeRFSceneList->sScene[idx].sID[j],&tempSensor);
        if(ret == 0)
            continue;
        response->sSensors[i].byteIsSceneAffect=gHomeRFSceneList->sScene[idx].isAlarm[j];
        //response->sSensors[i].bytePushAlarm=tempSensor.trigerOnOff;
        //response->sSensors[i].byteBattery=tempSensor.battery;
        response->sSensors[i].byteType=sysGetAppSensorType(tempSensor.type);
        response->sSensors[i].nSensorID=tempSensor.sID;
        strcpy(response->sSensors[i].szName,tempSensor.name);
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
		printf("APP_PAIR_NONE\n");
        return;
    }

    idx=sysGetUISensorListIdx(gHomeRFSensorID);

	printf("gHomeRFSensorID = %d, idx = %d\n",gHomeRFSensorID, idx);
	
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
    //DEBUG_YELLOW("ADD UI:  ID:%x NAME:%s Type:%d Alarm %d\n",gHomeRFSensorList->sSensor[idx].sID, gHomeRFSensorList->sSensor[idx].name, gHomeRFSensorList->sSensor[idx].type,gHomeRFSensorList->sSensor[idx].pushOnOff);
    //DEBUG_YELLOW("ADD APP: ID:%x NAME:%s Type:%d Alarm %d\n",response->sSensor.nSensorID,
    //          response->sSensor.szName, response->sSensor.byteType,response->sSensor.bytePushAlarm);

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

    switch(setting)
    {

        case HOMERF_SYS_SCENE_ON: // 1st Scene
            for(i=0; i<HOMERF_SENSOR_MAX; i++)
            {
                gHomeRFSensorList->sSensor[i].pushOnOff = HOMERF_SENSOR_ON;
                gHomeRFSensorList->sSensor[i].sirenOnOff= HOMERF_SENSOR_ON;
            }
            break;

        case HOMERF_SYS_SCENE_OFF: // 2nd Scene
            for(i=0; i<HOMERF_SENSOR_MAX; i++)
            {
                gHomeRFSensorList->sSensor[i].pushOnOff = HOMERF_SENSOR_OFF;
                gHomeRFSensorList->sSensor[i].sirenOnOff= HOMERF_SENSOR_OFF;
            }
            break;

        case HOMERF_SYS_SCENE_HOME: // 3rd Scene

            if(gHomeRFSceneList->sScene[2].sceneID != 0)//Means have setting
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
    int ReadSize;
    RTC_DATE_TIME   convert_time;
    u32 second_num;
    char hex_sec[10],hex_EventID[6],hex_sID[10],temp_char[10];

    idx=sysGetUISensorListIdx(sID);

    if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
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
                if((sysGetAppSensorType(homeRFGetDevice(dcfLogBuf_Rd[i*64+20], dcfLogBuf_Rd[i*64+21])) == sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type)) || (sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type) == mrst_Siren))
                {
                    memset(hex_sID,0,sizeof(hex_sID));/*Convert sID*/
                    for(j=3; j>=0; j--)
                    {
                        sprintf(temp_char,"%x",dcfLogBuf_Rd[i*64+22+j]);
                        strcat(hex_sID,temp_char);
                    }
                    hex_sID[9]='\0';
                    /*Search for match sID except Siren*/
                    if((strtoul(hex_sID,NULL,16) == sID) || (sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type) == mrst_Siren))
                    {
                        if((LogIdx<MAX_LOGRECORD_NUM) && (LogTatal >= order))
                        {
                            memset(hex_sec,0,sizeof(hex_sec));/*Convert Second*/
                            for(j=3; j>=0; j--)
                            {
                                sprintf(temp_char,"%.2x",dcfLogBuf_Rd[i*64+j]);
                                strcat(hex_sec,temp_char);
                            }
                            hex_sec[9]='\0';

                            memset(hex_EventID,0,sizeof(hex_EventID));/*Convert EventStatus*/
                            for(j=1; j>=0; j--)
                            {
                                sprintf(temp_char,"%.2x",dcfLogBuf_Rd[i*64+27+j]);
                                strcat(hex_EventID,temp_char);
                            }
                            hex_EventID[5]='\0';

                            second_num = strtoul(hex_sec,NULL,16);
                            RTC_Second_To_Time(second_num,&convert_time);
                            printf("%d-%02d-%02d  %02d:%02d:%02d\n",convert_time.year,convert_time.month,convert_time.day,convert_time.hour,convert_time.min,convert_time.sec);

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
        for(a=0; a<7; a++) /*Search 7 Days*/
        {
            //block_cnt = dcfGetLogFileInfo(a);
            for(b=0; b<dcfGetLogFileInfo(a); b++) /*Search block*/
            {
                i = 0;
                ReadSize = dcfReadLogFile(a,b,&Rblock,&Tblock);
                // if(dcfReadLogFile(a,b,&Rblock,&Tblock))
                if(ReadSize && (dcfLogBuf_Rd[63] == HOMERF_EVENT_LOG) && (dcfLogBuf_Rd[0] != 0) && (dcfLogBuf_Rd[1] != 0))
                {
                    i=0;
                    while(LogTatal<=(order + 20))
                    {
                        //if(i>(ReadSize/64))		/*A block only have Maximum 512 data*/
                        if(LogIdx>=(order+20))
                            goto ReturnData; /*Search for match Type except Siren*/
                        if(i>(ReadSize/64))
                            break;
                        if((sysGetAppSensorType(homeRFGetDevice(dcfLogBuf_Rd[i*64+20], dcfLogBuf_Rd[i*64+21])) == sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type)) || (sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type) == mrst_Siren))
                        {
                            memset(hex_sID,0,sizeof(hex_sID)); /*Convert sID*/
                            for(j=3; j>=0; j--)
                            {
                                sprintf(temp_char,"%x",dcfLogBuf_Rd[i*64+22+j]);
                                strcat(hex_sID,temp_char);
                            }
                            hex_sID[9]='\0';
                            /*Search for match sID except Siren*/
                            if((strtoul(hex_sID,NULL,16) == sID) || (sysGetAppSensorType(gHomeRFSensorList->sSensor[idx].type) == mrst_Siren))
                            {
                                if((LogIdx < MAX_LOGRECORD_NUM) && (LogTatal >= order))
                                {
                                    memset(hex_sec,0,sizeof(hex_sec)); /*Convert Second*/
                                    for(j=3; j>=0; j--)
                                    {
                                        sprintf(temp_char,"%.2x",dcfLogBuf_Rd[i*64+j]);
                                        strcat(hex_sec,temp_char);
                                    }
                                    hex_sec[9]='\0';

                                    memset(hex_EventID,0,sizeof(hex_EventID));/*Convert EventStatus*/
                                    for(j=1; j>=0; j--)
                                    {
                                        sprintf(temp_char,"%.2x",dcfLogBuf_Rd[i*64+27+j]);
                                        strcat(hex_EventID,temp_char);
                                    }
                                    hex_EventID[5]='\0';

                                    second_num = strtoul(hex_sec,NULL,16);
                                    RTC_Second_To_Time(second_num,&convert_time);
                                    //printf("%d-%d-%d  %d:%d:%d\n",convert_time.year,convert_time.month,convert_time.day,convert_time.hour,convert_time.min,convert_time.sec);

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
    char hex_sec[10],hex_EventID[6],hex_sID[9],temp_char[10];

    /*換算使用者點入Log頁面對應哪一天 Day*/
    while(day < Idx)
    {
        if(dcfGetLogFileInfo(i)>0)
            day++;
        i++;
    }
    LogFileInfo = dcfGetLogFileInfo(day);
    for(i=0; i<LogFileInfo; i++)
    {
        ReadLogFileSize = dcfReadLogFile(day,i,&Rblock,&Tblock)/64;
        for(j=0; j<ReadLogFileSize; j++)
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

    for(b=0; b<LogFileInfo; b++) /*Search block*/
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
                    for(j=3; j>=0; j--)
                    {
                        sprintf(temp_char,"%.2x",dcfLogBuf_Rd[(ReadBlock-i)*64+j]);
                        strcat(hex_sec,temp_char);
                    }
                    hex_sec[9]='\0';

                    memset(hex_EventID,0,sizeof(hex_EventID));/*Convert EventStatus*/
                    for(j=1; j>=0; j--)
                    {
                        sprintf(temp_char,"%.2x",dcfLogBuf_Rd[(ReadBlock-i)*64+27+j]);
                        strcat(hex_EventID,temp_char);
                    }
                    hex_EventID[5]='\0';

                    memset(homeRFSensorName,0,sizeof(homeRFSensorName));/*Convert SensorName*/
                    for(j=0; j<16; j++)
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
                    response->sRecords[LogIdx].nSensorType= homeRFGetDevice(dcfLogBuf_Rd[(ReadBlock-i)*64+20], dcfLogBuf_Rd[(ReadBlock-i)*64+21]);

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
    char hex_sec[10],hex_EventID[5],hex_sID[9],temp_char[10];
    u8 exist_flag=0;

    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
    {
        DEBUG_SYS("No SD Card Insert.\n");
        response->nTotalCount = 0;
        response->nCount = 0;
        return;
    }

    for(a=0; a<dctTotalLogCount; a++) /*Search 7 Days*/
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

        for(b=0; b<LogFileInfo; b++)
        {
            ReadSize = dcfReadLogFile(a,b,&Rblock,&Tblock);
            //if(ReadSize && (dcfLogBuf_Rd[63] == HOMERF_EVENT_LOG) && (dcfLogBuf_Rd[0] != 0) && (dcfLogBuf_Rd[1] != 0))
            if(ReadSize)
            {
                for(i=0; i<ReadSize/64; i++)
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
                            for(j=3; j>=0; j--)
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
#endif //#if(HOME_RF_SUPPORT)

#if CDVR_SYSTEM_LOG_SUPPORT
u8 sysUIGetSensorLog(u8 day, SYSTEM_EventLogList * response, unsigned int order) //Day: 0->Today, 1->Yesterday... ; Order : Start from 0, 4, 8,...
{
    int k,b=0,i=0,j=0,LogTatal=0, totalOrder=0,LogIdx=0, ReadBlock=0, LogFileInfo=0, ReadLogFileSize=0;
    int Rblock,Tblock;
    u32 ReadSize;
    u16 ReadNonZero = 0;
    RTC_DATE_TIME   convert_time;
    u32 second_num;
    char hex_sec[10],hex_EventID[5],hex_sID[9],temp_char[10];

    LogFileInfo = dcfGetLogFileInfo(day);
    for(i=0; i<LogFileInfo; i++)
    {
        ReadLogFileSize = dcfReadLogFile(day,i,&Rblock,&Tblock)/64;
        for(j=0; j<ReadLogFileSize; j++)
        {
            if((dcfLogBuf_Rd[j*64] != 0) && (dcfLogBuf_Rd[j*64+1] != 0) && (dcfLogBuf_Rd[j*64+63] == SYSTEM_EVENT_LOG))
                ReadNonZero++;
        }
    }

    /*回傳目前該天數有多少筆LOG*/
    response->nTotalCount = ReadNonZero;
    /* 確定第幾天後開始 對該天做parse */

    for(b=0; b<LogFileInfo; b++) /*Search block*/
    {
        ReadSize = dcfReadLogFile(day,b,&Rblock,&Tblock);

        if(ReadSize <= 0)
            continue;

        ReadBlock = ReadSize/64;
        i = 1;

        while((LogIdx<MAX_SYS_LOGRECORD_NUM) && ReadSize>0)  /* 一頁只能顯示4筆資訊 */
        {
            if(i > ReadBlock)
                break;
            if((dcfLogBuf_Rd[(ReadBlock-i)*64] != 0 && dcfLogBuf_Rd[(ReadBlock-i)*64+1] != 0) && dcfLogBuf_Rd[(ReadBlock-i)*64+63] == SYSTEM_EVENT_LOG)
            {
            

                if(LogTatal >= order)
                {

                    memset(hex_sec,0,sizeof(hex_sec)); /*Convert Second*/
                    for(j=3; j>=0; j--)
                    {
                        sprintf(temp_char,"%.2x",dcfLogBuf_Rd[(ReadBlock-i)*64+j]);
                        strcat(hex_sec,temp_char);
                    }
                    hex_sec[9]='\0';

                    second_num = strtoul(hex_sec,NULL,16);
                    RTC_Second_To_Time(second_num,&convert_time);
                    //printf("%d-%d-%d  %d:%d:%d\n",convert_time.year,convert_time.month,convert_time.day,convert_time.hour,convert_time.min,convert_time.sec);
                    response->sRecords[LogIdx].time.year = convert_time.year+2000;
                    response->sRecords[LogIdx].time.month= convert_time.month;
                    response->sRecords[LogIdx].time.day= convert_time.day;
                    response->sRecords[LogIdx].time.hour= convert_time.hour;
                    response->sRecords[LogIdx].time.minute= convert_time.min;
                    response->sRecords[LogIdx].time.second= convert_time.sec;
                    response->sRecords[LogIdx].nSysEventID = dcfLogBuf_Rd[(ReadBlock-i)*64+4];
                    response->sRecords[LogIdx].EventChannel= dcfLogBuf_Rd[(ReadBlock-i)*64+5];                  
                    //printf("\x1B[92m EventChannel=%d,ID=%d \x1B[0m\n",response->sRecords[LogIdx].EventChannel,response->sRecords[LogIdx].nSysEventID);
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
        return 1;
    }

    if((order+MAX_SYS_LOGRECORD_NUM) <= response->nTotalCount)
        response->nCount = MAX_SYS_LOGRECORD_NUM;
    else if((order+MAX_SYS_LOGRECORD_NUM) > response->nTotalCount)
        response->nCount = response->nTotalCount % MAX_SYS_LOGRECORD_NUM;
    else
        return 1;
    
    return 0;	//FINISH
}

#define SYSTEM_LOG_DATA_SIZE   64
u8  SystemLogData[SYSTEM_LOG_DATA_SIZE];

void sysWriteLog(u8 SysEventID, u8 channel, u8 type)
{
    RTC_DATE_TIME   localTime;
    u32 second;
    u8 i;
    u8 *ptr= SystemLogData;
    memset(SystemLogData, 0, SYSTEM_LOG_DATA_SIZE);
    printf("\x1B[96mSysEventID:%d, CH:%d\x1B[0m\n",SysEventID,channel);
    RTC_Get_Time(&localTime);
    second=RTC_Time_To_Second(&localTime);

    memcpy(ptr, &second, sizeof(second));
    ptr += sizeof(second);

    memcpy(ptr, &SysEventID, sizeof(SysEventID));
    ptr += sizeof(SysEventID);

    memcpy(ptr, &channel, sizeof(channel));
    ptr +=sizeof(channel);

    memcpy(ptr, &type, sizeof(type));
    ptr +=sizeof(type);

    SystemLogData[SYSTEM_LOG_DATA_SIZE-1] = SYSTEM_EVENT_LOG;
    printf("%s %d\n", __FUNCTION__, __LINE__);
    dcfWriteLogFile(SystemLogData, SYSTEM_LOG_DATA_SIZE);
    printf("%s %d\n", __FUNCTION__, __LINE__);
}
void sysBack_WriteLog(u8 SysEventID, u8 channel)	//Use For Motion Detection.
{
	u8 ret;

	ret = MultiChannelGetCaptureVideoStatus(channel);	//20171101 Sean: check if CH is recording or not.
	
	if(ret == 0)	//Not recording, return.
		return;

	sysWriteLog(SysEventID, channel, SYSTEM_LOG_MOTION);
}

u8 sysEventDate[31];

u8 sysGetSystemLog_initSearchDate(u8 month)
{
    int a=0,LogTatal=0, LogFileInfo=0;

    if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_SDC)
    {
        if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
        {
            DEBUG_SYS("No SD Card Insert.\n");
            return 0;
        }
    }
    else
    {
        if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
        {
            DEBUG_SYS("No Storage Insert.\n");
            return 0;
        }
    }
    memset(sysEventDate, 0, sizeof(sysEventDate)); //reset sysEventDate array.

    for(a=0; a<dctTotalLogCount; a++) //Search N Days
    {
        LogFileInfo = dcfGetLogFileInfo(a);

        if(LogFileInfo < 0) //SD Card may be fail
        {
            DEBUG_SYS("SD Card LogFileInfo Fail %d.\n", LogFileInfo);
            return 0;
        }
        if(LogFileInfo == 0)
            continue;

        sysEventDate[a] = 1;

        LogTatal++;
    }
}

#define SYSTEM_LOG_Ent_SIZE   1024
DCF_LIST_SYSLOG	*HeadCurLog	= NULL;
DCF_LIST_SYSLOG	*CurLog		= NULL;
DCF_LIST_SYSLOG	*PrevCurLog	= NULL;
DCF_LIST_SYSLOG CurLogListLogEnt[SYSTEM_LOG_Ent_SIZE];

void sysGetSystemLog_initSearch(u32 SearchbeginSec, u32 SearchendSec, u8 type, u8 ch) //beginSec and endSec should be same day
{
    RTC_DATE_TIME   search_time, convert_time, current_time;
    int b=0,i=0,j=0,k=0, totalOrder=0,LogIdx=0, ReadBlock=0, LogFileInfo=0, ReadLogFileSize=0;
    int Rblock,Tblock;
    u32 second_num, ReadSize, CurrentTime_sec, Event_cnt=0;
    u16 ReadNonZero = 0;
    u8	day=0, event_ch;
    char hex_sec[10],hex_EventID[5],hex_sID[9],temp_char[10];

    //First convert Second to Date.
    DEBUG_SYS("sysInitSearch\n");
    RTC_Second_To_Time(SearchbeginSec,&search_time);
    RTC_Get_Time(&current_time);
    CurrentTime_sec = RTC_Time_To_Second(&current_time);

    sysGetSystemLog_initSearchDate(search_time.month);

    //printf("\x1B[96mcurrent_time: %lu\x1B[0m\n",CurrentTime_sec);
    //printf("current_time.year  =%d, search_time.year =%d\n",current_time.year, search_time.year);
    //printf("current_time.month =%d, search_time.month=%d\n",current_time.month, search_time.month);
    //printf("current_time.day =%d, search_time.day=%d\n",current_time.day, search_time.day);
    //printf("day=%d, %d\n",(current_time.day - search_time.day)\
    //					 ,(sysEventDate[current_time.day - search_time.day]));

    if((current_time.year == search_time.year) \
            && (current_time.month == search_time.month) \
            && (sysEventDate[current_time.day - search_time.day]))
    {
        day = current_time.day - search_time.day;
        LogFileInfo = dcfGetLogFileInfo(day);

        for(b=0; b<LogFileInfo; b++) //Search block
        {
            ReadSize = dcfReadLogFile(day,b,&Rblock,&Tblock);

            if(ReadSize <= 0)
                continue;

            ReadBlock = ReadSize/64;
            i = 0;

            while(ReadBlock>0)
            {
                CurLog->next = NULL;
                if(i >= ReadBlock)
                    break;

                if((dcfLogBuf_Rd[i*64] != 0 && dcfLogBuf_Rd[i*64+1] != 0) \
                        && (dcfLogBuf_Rd[i*64+63] == SYSTEM_EVENT_LOG) \
                        && (dcfLogBuf_Rd[i*64+4] == SYSTEM_MOTION_ON) \
                        && (dcfLogBuf_Rd[i*64+5] == ch))
                {
                    memset(hex_sec,0,sizeof(hex_sec)); /*Convert Second*/
                    for(j=3; j>=0; j--)
                    {
                        sprintf(temp_char,"%.2x",dcfLogBuf_Rd[i*64+j]);
                        strcat(hex_sec,temp_char);
                    }
                    hex_sec[9]='\0';

                    second_num = strtoul(hex_sec,NULL,16);
                    if(second_num < SearchbeginSec)
                    {
                        i++;
                        continue;
                    }
                    else if(second_num > SearchendSec)
                        break;

                    event_ch = dcfLogBuf_Rd[i*64+5];

                    CurLog = &CurLogListLogEnt[Event_cnt]; //Allocate CruLog m.m.

                    CurLog->beginSec = second_num;
                    CurLog->channel	 = event_ch;

                    k=1;
                    while(1) //TBD
                    {
                        if(i+k >= ReadBlock)
                        {
                            if (HeadCurLog == NULL)
                                HeadCurLog = CurLog;
                            else
                                PrevCurLog->next = CurLog;

                            PrevCurLog = CurLog;
                            break;
                        }
                        if((dcfLogBuf_Rd[(i+k)*64] != 0 && dcfLogBuf_Rd[(i+k)*64+1] != 0) \
                                && (dcfLogBuf_Rd[(i+k)*64+63] == SYSTEM_EVENT_LOG) \
                                && (dcfLogBuf_Rd[(i+k)*64+5] == event_ch) \
                                && (dcfLogBuf_Rd[(i+k)*64+4] == SYSTEM_MOTION_OFF) \
                        		&& (dcfLogBuf_Rd[i*64+5] == ch))
                        {
                            memset(hex_sec,0,sizeof(hex_sec)); /*Convert Second*/
                            for(j=3; j>=0; j--)
                            {
                                sprintf(temp_char,"%.2x",dcfLogBuf_Rd[(i+k)*64+j]);
                                strcat(hex_sec,temp_char);
                            }
                            hex_sec[9]='\0';

                            second_num = strtoul(hex_sec,NULL,16);
                            event_ch   = dcfLogBuf_Rd[(i+k)*64+5];
                            CurLog->endSec = second_num;

                            if (HeadCurLog == NULL)
                                HeadCurLog = CurLog;
                            else
                                PrevCurLog->next = CurLog;

                            PrevCurLog = CurLog;
                            break;
                        }
                        else if((dcfLogBuf_Rd[(i+k)*64] != 0 && dcfLogBuf_Rd[(i+k)*64+1] != 0) \
                                && (dcfLogBuf_Rd[(i+k)*64+63] == SYSTEM_EVENT_LOG) \
                                && (dcfLogBuf_Rd[(i+k)*64+5] == event_ch) \
                                && (dcfLogBuf_Rd[(i+k)*64+4] == SYSTEM_MOTION_ON))
                        {
                            if (HeadCurLog == NULL)
                                HeadCurLog = CurLog;
                            else
                                PrevCurLog->next = CurLog;

                            PrevCurLog = CurLog;
                            break;
                        }
                        k++;
                    }
                    Event_cnt++;
                }
                i++;
            }
        }
    }
    else
        DEBUG_SYS("System Log Data Not Found.\n");

    CurLog = HeadCurLog;
}

bool sysGetSystemLog_nextSearch(u32* beginSec, u32* endSec, u8* eventType)
{
    if(CurLog != NULL)
    {
        *beginSec	= CurLog->beginSec;
        *endSec		= CurLog->endSec;
        *eventType	= CurLog->eventType;

        CurLog = CurLog->next;
        return TRUE;
    }
    else
        return FALSE;
}

#endif

#if QRCODE_SUPPORT

#include <../ui/inc/pic.h>

void QR_show(char *text, u8 qr_size, u16 x_pos, u16 y_pos)
{
	int x, y, size, border = 0;
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level
	// Make and print the QR Code symbol
	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
	bool ok;

	ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
		qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
	size = qrcodegen_getSize(qrcode);
	if (ok)
	{
		for (y = -1; y < size+1; y++)
		{
			for (x = -1; x < size+1; x++)
			{
				if((y == -1) || (y == size+1))
					OsdBuf_fillRect(x_pos+qr_size*x, y_pos+qr_size*y, qr_size, qr_size, clWHITE_OSD);
				else
				{
					if((x == -1) || (x == size+1))
						OsdBuf_fillRect(x_pos+qr_size*x, y_pos+qr_size*y, qr_size, qr_size, clWHITE_OSD);
					else
					{
						if(qrcodegen_getModule(qrcode, x, y))
							OsdBuf_fillRect(x_pos+qr_size*x, y_pos+qr_size*y, qr_size, qr_size, clBLACK_OSD);
						else
							OsdBuf_fillRect(x_pos+qr_size*x, y_pos+qr_size*y, qr_size, qr_size, clWHITE_OSD);
					}
				}
			}
		}
	}
	else
    	DEBUG_SYS("[ERR] Invalid QR Size.\n");
}

////////////////////////////////////////
// ----------------------
// QR Code Generate Code.
// 20170927 Sean Add.
// ----------------------
////////////////////////////////////////

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifndef QRCODEGEN_TEST
#define testable static  // Keep functions private
#else
// Expose private functions
#ifndef __cplusplus
#define testable
#else
// Needed for const variables because they are treated as implicitly 'static' in C++
#define testable extern
#endif
#endif


/*---- Forward declarations for private functions ----*/

// Regarding all public and private functions defined in this source file:
// - They require all pointer/array arguments to be not null.
// - They only read input scalar/array arguments, write to output pointer/array
//   arguments, and return scalar values; they are "pure" functions.
// - They don't read mutable global variables or write to any global variables.
// - They don't perform I/O, read the clock, print to console, etc.
// - They allocate a small and constant amount of stack memory.
// - They don't allocate or free any memory on the heap.
// - They don't recurse or mutually recurse. All the code
//   could be inlined into the top-level public functions.
// - They run in at most quadratic time with respect to input arguments.
//   Most functions run in linear time, and some in constant time.
//   There are no unbounded loops or non-obvious termination conditions.
// - They are completely thread-safe if the caller does not give the
//   same writable buffer to concurrent calls to these functions.

testable void appendBitsToBuffer(unsigned int val, int numBits, uint8_t buffer[], int *bitLen);

testable void appendErrorCorrection(uint8_t data[], int version, enum qrcodegen_Ecc ecl, uint8_t result[]);
testable int getNumDataCodewords(int version, enum qrcodegen_Ecc ecl);
testable int getNumRawDataModules(int version);

testable void calcReedSolomonGenerator(int degree, uint8_t result[]);
testable void calcReedSolomonRemainder(const uint8_t data[], int dataLen,
                                       const uint8_t generator[], int degree, uint8_t result[]);
testable uint8_t finiteFieldMultiply(uint8_t x, uint8_t y);

testable void initializeFunctionModules(int version, uint8_t qrcode[]);
static void drawWhiteFunctionModules(uint8_t qrcode[], int version);
static void drawFormatBits(enum qrcodegen_Ecc ecl, enum qrcodegen_Mask mask, uint8_t qrcode[]);
testable int getAlignmentPatternPositions(int version, uint8_t result[7]);
static void fillRectangle(int left, int top, int width, int height, uint8_t qrcode[]);

static void drawCodewords(const uint8_t data[], int dataLen, uint8_t qrcode[]);
static void applyMask(const uint8_t functionModules[], uint8_t qrcode[], enum qrcodegen_Mask mask);
static long getPenaltyScore(const uint8_t qrcode[]);

testable bool getModule(const uint8_t qrcode[], int x, int y);
testable void setModule(uint8_t qrcode[], int x, int y, bool isBlack);
testable void setModuleBounded(uint8_t qrcode[], int x, int y, bool isBlack);

testable int calcSegmentBitLength(enum qrcodegen_Mode mode, size_t numChars);
testable int getTotalBits(const struct qrcodegen_Segment segs[], size_t len, int version);
static int numCharCountBits(enum qrcodegen_Mode mode, int version);



/*---- Private tables of constants ----*/

// For checking text and encoding segments.
static const char *ALPHANUMERIC_CHARSET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

// For generating error correction codes.
testable const int8_t ECC_CODEWORDS_PER_BLOCK[4][41] =
{
    // Version: (note that index 0 is for padding, and is set to an illegal value)
    //0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40    Error correction level
    {-1,  7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},  // Low
    {-1, 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28},  // Medium
    {-1, 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},  // Quartile
    {-1, 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},  // High
};

// For generating error correction codes.
testable const int8_t NUM_ERROR_CORRECTION_BLOCKS[4][41] =
{
    // Version: (note that index 0 is for padding, and is set to an illegal value)
    //0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40    Error correction level
    {-1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25},  // Low
    {-1, 1, 1, 1, 2, 2, 4, 4, 4, 5, 5,  5,  8,  9,  9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49},  // Medium
    {-1, 1, 1, 2, 2, 4, 4, 6, 6, 8, 8,  8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68},  // Quartile
    {-1, 1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81},  // High
};

// For automatic mask pattern selection.
static const int PENALTY_N1 = 3;
static const int PENALTY_N2 = 3;
static const int PENALTY_N3 = 40;
static const int PENALTY_N4 = 10;



/*---- High-level QR Code encoding functions ----*/

// Public function - see documentation comment in header file.
bool qrcodegen_encodeText(const char *text, uint8_t tempBuffer[], uint8_t qrcode[],
                          enum qrcodegen_Ecc ecl, int minVersion, int maxVersion, enum qrcodegen_Mask mask, bool boostEcl)
{
    size_t i;
    size_t textLen = strlen(text);
    size_t bufLen;
    struct qrcodegen_Segment seg;
    if (textLen == 0)
        return qrcodegen_encodeSegmentsAdvanced(NULL, 0, ecl, minVersion, maxVersion, mask, boostEcl, tempBuffer, qrcode);
    bufLen= qrcodegen_BUFFER_LEN_FOR_VERSION(maxVersion);

    if (qrcodegen_isNumeric(text))
    {
        if (qrcodegen_calcSegmentBufferSize(qrcodegen_Mode_NUMERIC, textLen) > bufLen)
            goto fail;
        seg = qrcodegen_makeNumeric(text, tempBuffer);
    }
    else if (qrcodegen_isAlphanumeric(text))
    {
        if (qrcodegen_calcSegmentBufferSize(qrcodegen_Mode_ALPHANUMERIC, textLen) > bufLen)
            goto fail;
        seg = qrcodegen_makeAlphanumeric(text, tempBuffer);
    }
    else
    {
        if (textLen > bufLen)
            goto fail;
        for (i = 0; i < textLen; i++)
            tempBuffer[i] = (uint8_t)text[i];
        seg.mode = qrcodegen_Mode_BYTE;
        seg.bitLength = calcSegmentBitLength(seg.mode, textLen);
        if (seg.bitLength == -1)
            goto fail;
        seg.numChars = (int)textLen;
        seg.data = tempBuffer;
    }
    return qrcodegen_encodeSegmentsAdvanced(&seg, 1, ecl, minVersion, maxVersion, mask, boostEcl, tempBuffer, qrcode);

fail:
    qrcode[0] = 0;  // Set size to invalid value for safety
    return false;
}


// Public function - see documentation comment in header file.
bool qrcodegen_encodeBinary(uint8_t dataAndTemp[], size_t dataLen, uint8_t qrcode[],
                            enum qrcodegen_Ecc ecl, int minVersion, int maxVersion, enum qrcodegen_Mask mask, bool boostEcl)
{

    struct qrcodegen_Segment seg;
    seg.mode = qrcodegen_Mode_BYTE;
    seg.bitLength = calcSegmentBitLength(seg.mode, dataLen);
    if (seg.bitLength == -1)
    {
        qrcode[0] = 0;  // Set size to invalid value for safety
        return false;
    }
    seg.numChars = (int)dataLen;
    seg.data = dataAndTemp;
    return qrcodegen_encodeSegmentsAdvanced(&seg, 1, ecl, minVersion, maxVersion, mask, boostEcl, dataAndTemp, qrcode);
}


// Appends the given sequence of bits to the given byte-based bit buffer, increasing the bit length.
testable void appendBitsToBuffer(unsigned int val, int numBits, uint8_t buffer[], int *bitLen)
{
    int i;
    assert(0 <= numBits && numBits <= 16 && (unsigned long)val >> numBits == 0);
    for (i = numBits - 1; i >= 0; i--, (*bitLen)++)
        buffer[*bitLen >> 3] |= ((val >> i) & 1) << (7 - (*bitLen & 7));
}



/*---- Error correction code generation functions ----*/

// Appends error correction bytes to each block of the given data array, then interleaves bytes
// from the blocks and stores them in the result array. data[0 : rawCodewords - totalEcc] contains
// the input data. data[rawCodewords - totalEcc : rawCodewords] is used as a temporary work area
// and will be clobbered by this function. The final answer is stored in result[0 : rawCodewords].
testable void appendErrorCorrection(uint8_t data[], int version, enum qrcodegen_Ecc ecl, uint8_t result[])
{
    // Calculate parameter numbers
    
    int numBlocks = NUM_ERROR_CORRECTION_BLOCKS[(int)ecl][version];
    int blockEccLen = ECC_CODEWORDS_PER_BLOCK[(int)ecl][version];
    int rawCodewords = getNumRawDataModules(version) / 8;
    int dataLen = rawCodewords - blockEccLen * numBlocks;
    int numShortBlocks = numBlocks - rawCodewords % numBlocks;
    int shortBlockDataLen = rawCodewords / numBlocks - blockEccLen;

    // Split data into blocks and append ECC after all data
    uint8_t generator[30];
    int i,j,k,l;
    assert(0 <= (int)ecl && (int)ecl < 4 && qrcodegen_VERSION_MIN <= version && version <= qrcodegen_VERSION_MAX);
    calcReedSolomonGenerator(blockEccLen, generator);
    for (i = 0, j = dataLen, k = 0; i < numBlocks; i++)
    {
        int blockLen = shortBlockDataLen;
        if (i >= numShortBlocks)
            blockLen++;
        calcReedSolomonRemainder(&data[k], blockLen, generator, blockEccLen, &data[j]);
        j += blockEccLen;
        k += blockLen;
    }

    // Interleave (not concatenate) the bytes from every block into a single sequence
    for (i = 0, k = 0; i < numBlocks; i++)
    {
        for (j = 0, l = i; j < shortBlockDataLen; j++, k++, l += numBlocks)
            result[l] = data[k];
        if (i >= numShortBlocks)
            k++;
    }
    for (i = numShortBlocks, k = (numShortBlocks + 1) * shortBlockDataLen, l = numBlocks * shortBlockDataLen;
            i < numBlocks; i++, k += shortBlockDataLen + 1, l++)
        result[l] = data[k];
    for (i = 0, k = dataLen; i < numBlocks; i++)
    {
        for (j = 0, l = dataLen + i; j < blockEccLen; j++, k++, l += numBlocks)
            result[l] = data[k];
    }
}


// Returns the number of 8-bit codewords that can be used for storing data (not ECC),
// for the given version number and error correction level. The result is in the range [9, 2956].
testable int getNumDataCodewords(int version, enum qrcodegen_Ecc ecl)
{
    int v = version, e = (int)ecl;
    assert(0 <= e && e < 4 && qrcodegen_VERSION_MIN <= v && v <= qrcodegen_VERSION_MAX);
    return getNumRawDataModules(v) / 8 - ECC_CODEWORDS_PER_BLOCK[e][v] * NUM_ERROR_CORRECTION_BLOCKS[e][v];
}


// Returns the number of data bits that can be stored in a QR Code of the given version number, after
// all function modules are excluded. This includes remainder bits, so it might not be a multiple of 8.
// The result is in the range [208, 29648]. This could be implemented as a 40-entry lookup table.
testable int getNumRawDataModules(int version)
{
	int result, numAlign;
    assert(qrcodegen_VERSION_MIN <= version && version <= qrcodegen_VERSION_MAX);
    result = (16 * version + 128) * version + 64;
    if (version >= 2)
    {
        numAlign = version / 7 + 2;
        result -= (25 * numAlign - 10) * numAlign - 55;
        if (version >= 7)
            result -= 18 * 2;  // Subtract version information
    }
    return result;
}



/*---- Reed-Solomon ECC generator functions ----*/

// Calculates the Reed-Solomon generator polynomial of the given degree, storing in result[0 : degree].
testable void calcReedSolomonGenerator(int degree, uint8_t result[])
{
    uint8_t root = 1;
    int i,j;
    // Start with the monomial x^0
    assert(1 <= degree && degree <= 30);
    memset(result, 0, degree * sizeof(result[0]));
    result[degree - 1] = 1;

    // Compute the product polynomial (x - r^0) * (x - r^1) * (x - r^2) * ... * (x - r^{degree-1}),
    // drop the highest term, and store the rest of the coefficients in order of descending powers.
    // Note that r = 0x02, which is a generator element of this field GF(2^8/0x11D).
    root = 1;
    for (i = 0; i < degree; i++)
    {
        // Multiply the current product by (x - r^i)
        for (j = 0; j < degree; j++)
        {
            result[j] = finiteFieldMultiply(result[j], root);
            if (j + 1 < degree)
                result[j] ^= result[j + 1];
        }
        root = finiteFieldMultiply(root, 0x02);
    }
}


// Calculates the remainder of the polynomial data[0 : dataLen] when divided by the generator[0 : degree], where all
// polynomials are in big endian and the generator has an implicit leading 1 term, storing the result in result[0 : degree].
testable void calcReedSolomonRemainder(const uint8_t data[], int dataLen,
                                       const uint8_t generator[], int degree, uint8_t result[])
{
int i,j;
    // Perform polynomial division
    assert(1 <= degree && degree <= 30);
    memset(result, 0, degree * sizeof(result[0]));
    for (i = 0; i < dataLen; i++)
    {
        uint8_t factor = data[i] ^ result[0];
        memmove(&result[0], &result[1], (degree - 1) * sizeof(result[0]));
        result[degree - 1] = 0;
        for (j = 0; j < degree; j++)
            result[j] ^= finiteFieldMultiply(generator[j], factor);
    }
}


// Returns the product of the two given field elements modulo GF(2^8/0x11D).
// All inputs are valid. This could be implemented as a 256*256 lookup table.
testable uint8_t finiteFieldMultiply(uint8_t x, uint8_t y)
{
    int i;
    // Russian peasant multiplication
    uint8_t z = 0;
    for (i = 7; i >= 0; i--)
    {
        z = (z << 1) ^ ((z >> 7) * 0x11D);
        z ^= ((y >> i) & 1) * x;
    }
    return z;
}



/*---- Drawing function modules ----*/

// Clears the given QR Code grid with white modules for the given
// version's size, then marks every function module as black.
testable void initializeFunctionModules(int version, uint8_t qrcode[])
{
    // Initialize QR Code
    int i,j;
    int numAlign;
    int qrsize;
    // Fill numerous alignment patterns
    uint8_t alignPatPos[7] = {0};

    qrsize = version * 4 + 17;
    
    memset(qrcode, 0, ((qrsize * qrsize + 7) / 8 + 1) * sizeof(qrcode[0]));
    qrcode[0] = (uint8_t)qrsize;

    // Fill horizontal and vertical timing patterns
    fillRectangle(6, 0, 1, qrsize, qrcode);
    fillRectangle(0, 6, qrsize, 1, qrcode);

    // Fill 3 finder patterns (all corners except bottom right) and format bits
    fillRectangle(0, 0, 9, 9, qrcode);
    fillRectangle(qrsize - 8, 0, 8, 9, qrcode);
    fillRectangle(0, qrsize - 8, 9, 8, qrcode);

    numAlign = getAlignmentPatternPositions(version, alignPatPos);
    for (i = 0; i < numAlign; i++)
    {
        for (j = 0; j < numAlign; j++)
        {
            if ((i == 0 && j == 0) || (i == 0 && j == numAlign - 1) || (i == numAlign - 1 && j == 0))
                continue;  // Skip the three finder corners
            else
                fillRectangle(alignPatPos[i] - 2, alignPatPos[j] - 2, 5, 5, qrcode);
        }
    }

    // Fill version blocks
    if (version >= 7)
    {
        fillRectangle(qrsize - 11, 0, 3, 6, qrcode);
        fillRectangle(0, qrsize - 11, 6, 3, qrcode);
    }
}


// Draws white function modules and possibly some black modules onto the given QR Code, without changing
// non-function modules. This does not draw the format bits. This requires all function modules to be previously
// marked black (namely by initializeFunctionModules()), because this may skip redrawing black function modules.
static void drawWhiteFunctionModules(uint8_t qrcode[], int version)
{
    // Draw horizontal and vertical timing patterns
    int qrsize;
    int i,j,k,l;
    long data;
    int numAlign;
    uint8_t alignPatPos[7] = {0};
    int rem;
    
    qrsize= qrcodegen_getSize(qrcode);
    for (i = 7; i < qrsize - 7; i += 2)
    {
        setModule(qrcode, 6, i, false);
        setModule(qrcode, i, 6, false);
    }

    // Draw 3 finder patterns (all corners except bottom right; overwrites some timing modules)
    for (i = -4; i <= 4; i++)
    {
        for (j = -4; j <= 4; j++)
        {
            int dist = abs(i);
            if (abs(j) > dist)
                dist = abs(j);
            if (dist == 2 || dist == 4)
            {
                setModuleBounded(qrcode, 3 + j, 3 + i, false);
                setModuleBounded(qrcode, qrsize - 4 + j, 3 + i, false);
                setModuleBounded(qrcode, 3 + j, qrsize - 4 + i, false);
            }
        }
    }

    // Draw numerous alignment patterns
    numAlign = getAlignmentPatternPositions(version, alignPatPos);

    for (i = 0; i < numAlign; i++)
    {
        for (j = 0; j < numAlign; j++)
        {
            if ((i == 0 && j == 0) || (i == 0 && j == numAlign - 1) || (i == numAlign - 1 && j == 0))
                continue;  // Skip the three finder corners
            else
            {
                for (k = -1; k <= 1; k++)
                {
                    for (l = -1; l <= 1; l++)
                        setModule(qrcode, alignPatPos[i] + l, alignPatPos[j] + k, k == 0 && l == 0);
                }
            }
        }
    }

    // Draw version blocks
    if (version >= 7)
    {
        // Calculate error correction code and pack bits
        rem = version;  // version is uint6, in the range [7, 40]
        for (i = 0; i < 12; i++)
            rem = (rem << 1) ^ ((rem >> 11) * 0x1F25);
        data = (long)version << 12 | rem;  // uint18
        assert(data >> 18 == 0);

        // Draw two copies
        for (i = 0; i < 6; i++)
        {
            for (j = 0; j < 3; j++)
            {
                k = qrsize - 11 + j;
                setModule(qrcode, k, i, (data & 1) != 0);
                setModule(qrcode, i, k, (data & 1) != 0);
                data >>= 1;
            }
        }
    }
}


// Draws two copies of the format bits (with its own error correction code) based
// on the given mask and error correction level. This always draws all modules of
// the format bits, unlike drawWhiteFunctionModules() which might skip black modules.
static void drawFormatBits(enum qrcodegen_Ecc ecl, enum qrcodegen_Mask mask, uint8_t qrcode[])
{
    // Calculate error correction code and pack bits
    int data;
    int i;
    int qrsize;
    int rem;
    assert(0 <= (int)mask && (int)mask <= 7);
    
    switch (ecl)
    {
    case qrcodegen_Ecc_LOW     :
        data = 1;
        break;
    case qrcodegen_Ecc_MEDIUM  :
        data = 0;
        break;
    case qrcodegen_Ecc_QUARTILE:
        data = 3;
        break;
    case qrcodegen_Ecc_HIGH    :
        data = 2;
        break;
    default:
        assert(false);
    }
    data = data << 3 | (int)mask;  // ecl-derived value is uint2, mask is uint3
    rem = data;
    for (i = 0; i < 10; i++)
        rem = (rem << 1) ^ ((rem >> 9) * 0x537);
    data = data << 10 | rem;
    data ^= 0x5412;  // uint15
    assert(data >> 15 == 0);

    // Draw first copy
    for (i = 0; i <= 5; i++)
        setModule(qrcode, 8, i, ((data >> i) & 1) != 0);
    setModule(qrcode, 8, 7, ((data >> 6) & 1) != 0);
    setModule(qrcode, 8, 8, ((data >> 7) & 1) != 0);
    setModule(qrcode, 7, 8, ((data >> 8) & 1) != 0);
    for (i = 9; i < 15; i++)
        setModule(qrcode, 14 - i, 8, ((data >> i) & 1) != 0);

    // Draw second copy
    qrsize = qrcodegen_getSize(qrcode);
    for (i = 0; i <= 7; i++)
        setModule(qrcode, qrsize - 1 - i, 8, ((data >> i) & 1) != 0);
    for (i = 8; i < 15; i++)
        setModule(qrcode, 8, qrsize - 15 + i, ((data >> i) & 1) != 0);
    setModule(qrcode, 8, qrsize - 8, true);
}


// Calculates the positions of alignment patterns in ascending order for the given version number,
// storing them to the given array and returning an array length in the range [0, 7].
testable int getAlignmentPatternPositions(int version, uint8_t result[7])
{
    int i ,pos;
    int numAlign;
    int step;
    
    if (version == 1)
        return 0;
    numAlign = version / 7 + 2;
    if (version != 32)
    {
        // ceil((size - 13) / (2*numAlign - 2)) * 2
        step = (version * 4 + numAlign * 2 + 1) / (2 * numAlign - 2) * 2;
    }
    else    // C-C-C-Combo breaker!
        step = 26;
    for (i = numAlign - 1, pos = version * 4 + 10; i >= 1; i--, pos -= step)
        result[i] = pos;
    result[0] = 6;
    return numAlign;
}


// Sets every pixel in the range [left : left + width] * [top : top + height] to black.
static void fillRectangle(int left, int top, int width, int height, uint8_t qrcode[])
{
    int dy,dx;
    for (dy = 0; dy < height; dy++)
    {
        for (dx = 0; dx < width; dx++)
            setModule(qrcode, left + dx, top + dy, true);
    }
}



/*---- Drawing data modules and masking ----*/

// Draws the raw codewords (including data and ECC) onto the given QR Code. This requires the initial state of
// the QR Code to be black at function modules and white at codeword modules (including unused remainder bits).
static void drawCodewords(const uint8_t data[], int dataLen, uint8_t qrcode[])
{
    int qrsize = qrcodegen_getSize(qrcode);
    int i = 0;  // Bit index into the data
    int right,vert,j;
    // Do the funny zigzag scan
    for (right = qrsize - 1; right >= 1; right -= 2)    // Index of right column in each column pair
    {
        if (right == 6)
            right = 5;
        for (vert = 0; vert < qrsize; vert++)    // Vertical counter
        {
            for (j = 0; j < 2; j++)
            {
                int x = right - j;  // Actual x coordinate
                bool upward = ((right + 1) & 2) == 0;
                int y = upward ? qrsize - 1 - vert : vert;  // Actual y coordinate
                if (!getModule(qrcode, x, y) && i < dataLen * 8)
                {
                    bool black = ((data[i >> 3] >> (7 - (i & 7))) & 1) != 0;
                    setModule(qrcode, x, y, black);
                    i++;
                }
                // If there are any remainder bits (0 to 7), they are already
                // set to 0/false/white when the grid of modules was initialized
            }
        }
    }
    assert(i == dataLen * 8);
}


// XORs the data modules in this QR Code with the given mask pattern. Due to XOR's mathematical
// properties, calling applyMask(..., m) twice with the same value is equivalent to no change at all.
// This means it is possible to apply a mask, undo it, and try another mask. Note that a final
// well-formed QR Code symbol needs exactly one mask applied (not zero, not two, etc.).
static void applyMask(const uint8_t functionModules[], uint8_t qrcode[], enum qrcodegen_Mask mask)
{
    int x,y;
	bool invert;
    int qrsize;
	bool val;
    assert(0 <= (int)mask && (int)mask <= 7);  // Disallows qrcodegen_Mask_AUTO
    qrsize = qrcodegen_getSize(qrcode);
    for (y = 0; y < qrsize; y++)
    {
        for (x = 0; x < qrsize; x++)
        {
            if (getModule(functionModules, x, y))
                continue;
            switch ((int)mask)
            {
            case 0:
                invert = (x + y) % 2 == 0;
                break;
            case 1:
                invert = y % 2 == 0;
                break;
            case 2:
                invert = x % 3 == 0;
                break;
            case 3:
                invert = (x + y) % 3 == 0;
                break;
            case 4:
                invert = (x / 3 + y / 2) % 2 == 0;
                break;
            case 5:
                invert = x * y % 2 + x * y % 3 == 0;
                break;
            case 6:
                invert = (x * y % 2 + x * y % 3) % 2 == 0;
                break;
            case 7:
                invert = ((x + y) % 2 + x * y % 3) % 2 == 0;
                break;
            default:
                assert(false);
            }
		val = getModule(qrcode, x, y);
            setModule(qrcode, x, y, val ^ invert);
        }
    }
}


// Calculates and returns the penalty score based on state of the given QR Code's current modules.
// This is used by the automatic mask choice algorithm to find the mask pattern that yields the lowest score.
static long getPenaltyScore(const uint8_t qrcode[])
{
    int qrsize = qrcodegen_getSize(qrcode);
    long result = 0;
	int x,y,runX,runY,bits,k;
	int black = 0;
    int total;

    // Adjacent modules in row having same color
    for (y = 0; y < qrsize; y++)
    {
        bool colorX;
        for (x = 0, runX; x < qrsize; x++)
        {
            if (x == 0 || getModule(qrcode, x, y) != colorX)
            {
                colorX = getModule(qrcode, x, y);
                runX = 1;
            }
            else
            {
                runX++;
                if (runX == 5)
                    result += PENALTY_N1;
                else if (runX > 5)
                    result++;
            }
        }
    }
    // Adjacent modules in column having same color
    for (x = 0; x < qrsize; x++)
    {
        bool colorY;
        for (y = 0, runY; y < qrsize; y++)
        {
            if (y == 0 || getModule(qrcode, x, y) != colorY)
            {
                colorY = getModule(qrcode, x, y);
                runY = 1;
            }
            else
            {
                runY++;
                if (runY == 5)
                    result += PENALTY_N1;
                else if (runY > 5)
                    result++;
            }
        }
    }

    // 2*2 blocks of modules having same color
    for (y = 0; y < qrsize - 1; y++)
    {
        for (x = 0; x < qrsize - 1; x++)
        {
            bool  color = getModule(qrcode, x, y);
            if (  color == getModule(qrcode, x + 1, y) &&
                    color == getModule(qrcode, x, y + 1) &&
                    color == getModule(qrcode, x + 1, y + 1))
                result += PENALTY_N2;
        }
    }

    // Finder-like pattern in rows
    for (y = 0; y < qrsize; y++)
    {
        for (x = 0, bits = 0; x < qrsize; x++)
        {
            bits = ((bits << 1) & 0x7FF) | (getModule(qrcode, x, y) ? 1 : 0);
            if (x >= 10 && (bits == 0x05D || bits == 0x5D0))  // Needs 11 bits accumulated
                result += PENALTY_N3;
        }
    }
    // Finder-like pattern in columns
    for (x = 0; x < qrsize; x++)
    {
        for (y = 0, bits = 0; y < qrsize; y++)
        {
            bits = ((bits << 1) & 0x7FF) | (getModule(qrcode, x, y) ? 1 : 0);
            if (y >= 10 && (bits == 0x05D || bits == 0x5D0))  // Needs 11 bits accumulated
                result += PENALTY_N3;
        }
    }

    // Balance of black and white modules
    black = 0;
    for (y = 0; y < qrsize; y++)
    {
        for (x = 0; x < qrsize; x++)
        {
            if (getModule(qrcode, x, y))
                black++;
        }
    }
    total = qrsize * qrsize;
    // Find smallest k such that (45-5k)% <= dark/total <= (55+5k)%
    for (k = 0; black*20L < (9L-k)*total || black*20L > (11L+k)*total; k++)
        result += PENALTY_N4;
    return result;
}



/*---- Basic QR Code information ----*/

// Public function - see documentation comment in header file.
int qrcodegen_getSize(const uint8_t qrcode[])
{
    int result;
    result= qrcode[0];
    //printf("\x1B[96m %d,%d,%d \x1B[0m\n",(qrcodegen_VERSION_MIN * 4 + 17),result,(qrcodegen_VERSION_MAX * 4 + 17));
    assert(qrcode != NULL);
    assert((qrcodegen_VERSION_MIN * 4 + 17) <= result
           && result <= (qrcodegen_VERSION_MAX * 4 + 17));
    return result;
}


// Public function - see documentation comment in header file.
bool qrcodegen_getModule(const uint8_t qrcode[], int x, int y)
{
    int qrsize = qrcode[0];
    assert(qrcode != NULL);
    return (0 <= x && x < qrsize && 0 <= y && y < qrsize) && getModule(qrcode, x, y);
}


// Gets the module at the given coordinates, which must be in bounds.
testable bool getModule(const uint8_t qrcode[], int x, int y)
{
    int qrsize = qrcode[0];
    int index = y * qrsize + x;
    int bitIndex = index & 7;
    int byteIndex = (index >> 3) + 1;
    //printf("qrsize=%d, x=%d, y=%d\n",qrsize,x,y);
    assert(21 <= qrsize && qrsize <= 177 && 0 <= x && x < qrsize && 0 <= y && y < qrsize);
    return ((qrcode[byteIndex] >> bitIndex) & 1) != 0;
}


// Sets the module at the given coordinates, which must be in bounds.
testable void setModule(uint8_t qrcode[], int x, int y, bool isBlack)
{
    int qrsize = qrcode[0];
    int index = y * qrsize + x;
    int bitIndex = index & 7;
    int byteIndex = (index >> 3) + 1;
    assert(21 <= qrsize && qrsize <= 177 && 0 <= x && x < qrsize && 0 <= y && y < qrsize);
    if (isBlack)
        qrcode[byteIndex] |= 1 << bitIndex;
    else
        qrcode[byteIndex] &= (1 << bitIndex) ^ 0xFF;
}


// Sets the module at the given coordinates, doing nothing if out of bounds.
testable void setModuleBounded(uint8_t qrcode[], int x, int y, bool isBlack)
{
    int qrsize = qrcode[0];
    if (0 <= x && x < qrsize && 0 <= y && y < qrsize)
        setModule(qrcode, x, y, isBlack);
}



/*---- Segment handling ----*/

// Public function - see documentation comment in header file.
bool qrcodegen_isAlphanumeric(const char *text)
{
    assert(text != NULL);
    for (; *text != '\0'; text++)
    {
        if (strchr(ALPHANUMERIC_CHARSET, *text) == NULL)
            return false;
    }
    return true;
}


// Public function - see documentation comment in header file.
bool qrcodegen_isNumeric(const char *text)
{
    assert(text != NULL);
    for (; *text != '\0'; text++)
    {
        if (*text < '0' || *text > '9')
            return false;
    }
    return true;
}


// Public function - see documentation comment in header file.
size_t qrcodegen_calcSegmentBufferSize(enum qrcodegen_Mode mode, size_t numChars)
{
    int temp = calcSegmentBitLength(mode, numChars);
    if (temp == -1)
        return SIZE_MAX;
    assert(0 <= temp && temp <= INT16_MAX);
    return ((size_t)temp + 7) / 8;
}


// Returns the number of data bits needed to represent a segment
// containing the given number of characters using the given mode. Notes:
// - Returns -1 on failure, i.e. numChars > INT16_MAX or
//   the number of needed bits exceeds INT16_MAX (i.e. 32767).
// - Otherwise, all valid results are in the range [0, INT16_MAX].
// - For byte mode, numChars measures the number of bytes, not Unicode code points.
// - For ECI mode, numChars must be 0, and the worst-case number of bits is returned.
//   An actual ECI segment can have shorter data. For non-ECI modes, the result is exact.
testable int calcSegmentBitLength(enum qrcodegen_Mode mode, size_t numChars)
{
	int temp;
	int n;
	int result;
    const int LIMIT = INT16_MAX;  // Can be configured as high as INT_MAX
    if (numChars > (unsigned int)LIMIT)
        return -1;
    n = (int)numChars;

    result = -2;
    if (mode == qrcodegen_Mode_NUMERIC)
    {
        // n * 3 + ceil(n / 3)
        if (n > LIMIT / 3)
            goto overflow;
        result = n * 3;
        temp = n / 3 + (n % 3 == 0 ? 0 : 1);
        if (temp > LIMIT - result)
            goto overflow;
        result += temp;
    }
    else if (mode == qrcodegen_Mode_ALPHANUMERIC)
    {
        // n * 5 + ceil(n / 2)
        if (n > LIMIT / 5)
            goto overflow;
        result = n * 5;
        temp = n / 2 + n % 2;
        if (temp > LIMIT - result)
            goto overflow;
        result += temp;
    }
    else if (mode == qrcodegen_Mode_BYTE)
    {
        if (n > LIMIT / 8)
            goto overflow;
        result = n * 8;
    }
    else if (mode == qrcodegen_Mode_KANJI)
    {
        if (n > LIMIT / 13)
            goto overflow;
        result = n * 13;
    }
    else if (mode == qrcodegen_Mode_ECI && numChars == 0)
        result = 3 * 8;
    assert(0 <= result && result <= LIMIT);
    return result;
overflow:
    return -1;
}


// Public function - see documentation comment in header file.
struct qrcodegen_Segment qrcodegen_makeBytes(const uint8_t data[], size_t len, uint8_t buf[])
{
    struct qrcodegen_Segment result;
    assert(data != NULL || len == 0);
    result.mode = qrcodegen_Mode_BYTE;
    result.bitLength = calcSegmentBitLength(result.mode, len);
    assert(result.bitLength != -1);
    result.numChars = (int)len;
    if (len > 0)
        memcpy(buf, data, len * sizeof(buf[0]));
    result.data = buf;
    return result;
}


// Public function - see documentation comment in header file.
struct qrcodegen_Segment qrcodegen_makeNumeric(const char *digits, uint8_t buf[])
{
	int accumCount;
    unsigned int accumData = 0;
	size_t len;
	int bitLen;
    struct qrcodegen_Segment result;

    assert(digits != NULL);
    len = strlen(digits);
    result.mode = qrcodegen_Mode_NUMERIC;
    bitLen = calcSegmentBitLength(result.mode, len);
    assert(bitLen != -1);
    result.numChars = (int)len;
    if (bitLen > 0)
        memset(buf, 0, ((size_t)bitLen + 7) / 8 * sizeof(buf[0]));
    result.bitLength = 0;

    accumData = 0;
    accumCount = 0;
    for (; *digits != '\0'; digits++)
    {
        char c = *digits;
        assert('0' <= c && c <= '9');
        accumData = accumData * 10 + (c - '0');
        accumCount++;
        if (accumCount == 3)
        {
            appendBitsToBuffer(accumData, 10, buf, &result.bitLength);
            accumData = 0;
            accumCount = 0;
        }
    }
    if (accumCount > 0)  // 1 or 2 digits remaining
        appendBitsToBuffer(accumData, accumCount * 3 + 1, buf, &result.bitLength);
    assert(result.bitLength == bitLen);
    result.data = buf;
    return result;
}


// Public function - see documentation comment in header file.
struct qrcodegen_Segment qrcodegen_makeAlphanumeric(const char *text, uint8_t buf[])
{
	int accumCount;
	unsigned int accumData;
	int bitLen;
	size_t len;
    struct qrcodegen_Segment result;

    assert(text != NULL);
    len = strlen(text);
    result.mode = qrcodegen_Mode_ALPHANUMERIC;
    bitLen = calcSegmentBitLength(result.mode, len);
    assert(bitLen != -1);
    result.numChars = (int)len;
    if (bitLen > 0)
        memset(buf, 0, ((size_t)bitLen + 7) / 8 * sizeof(buf[0]));
    result.bitLength = 0;

    accumData = 0;
    accumCount = 0;
    for (; *text != '\0'; text++)
    {
        const char *temp = strchr(ALPHANUMERIC_CHARSET, *text);
        assert(temp != NULL);
        accumData = accumData * 45 + (temp - ALPHANUMERIC_CHARSET);
        accumCount++;
        if (accumCount == 2)
        {
            appendBitsToBuffer(accumData, 11, buf, &result.bitLength);
            accumData = 0;
            accumCount = 0;
        }
    }
    if (accumCount > 0)  // 1 character remaining
        appendBitsToBuffer(accumData, 6, buf, &result.bitLength);
    assert(result.bitLength == bitLen);
    result.data = buf;
    return result;
}


// Public function - see documentation comment in header file.
struct qrcodegen_Segment qrcodegen_makeEci(long assignVal, uint8_t buf[])
{
    struct qrcodegen_Segment result;
    result.mode = qrcodegen_Mode_ECI;
    result.numChars = 0;
    result.bitLength = 0;
    if (0 <= assignVal && assignVal < (1 << 7))
    {
        memset(buf, 0, 1 * sizeof(buf[0]));
        appendBitsToBuffer(assignVal, 8, buf, &result.bitLength);
    }
    else if ((1 << 7) <= assignVal && assignVal < (1 << 14))
    {
        memset(buf, 0, 2 * sizeof(buf[0]));
        appendBitsToBuffer(2, 2, buf, &result.bitLength);
        appendBitsToBuffer(assignVal, 14, buf, &result.bitLength);
    }
    else if ((1 << 14) <= assignVal && assignVal < 1000000L)
    {
        memset(buf, 0, 3 * sizeof(buf[0]));
        appendBitsToBuffer(6, 3, buf, &result.bitLength);
        appendBitsToBuffer(assignVal >> 10, 11, buf, &result.bitLength);
        appendBitsToBuffer(assignVal & 0x3FF, 10, buf, &result.bitLength);
    }
    else
        assert(false);
    result.data = buf;
    return result;
}


// Public function - see documentation comment in header file.
bool qrcodegen_encodeSegments(const struct qrcodegen_Segment segs[], size_t len,
                              enum qrcodegen_Ecc ecl, uint8_t tempBuffer[], uint8_t qrcode[])
{
    return qrcodegen_encodeSegmentsAdvanced(segs, len, ecl,
                                            qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, -1, true, tempBuffer, qrcode);
}


// Public function - see documentation comment in header file.
bool qrcodegen_encodeSegmentsAdvanced(const struct qrcodegen_Segment segs[], size_t len, enum qrcodegen_Ecc ecl,
                                      int minVersion, int maxVersion, int mask, bool boostEcl, uint8_t tempBuffer[], uint8_t qrcode[])
{
	int i,j;
    int version, dataUsedBits;
    int dataCapacityBits;
    int bitLen;
    size_t i2;
    uint8_t padByte;
    int terminatorBits;
    long penalty;
    assert(segs != NULL || len == 0);
    assert(qrcodegen_VERSION_MIN <= minVersion && minVersion <= maxVersion && maxVersion <= qrcodegen_VERSION_MAX);
    assert(0 <= (int)ecl && (int)ecl <= 3 && -1 <= (int)mask && (int)mask <= 7);
    // Find the minimal version number to use
    for (version = minVersion; ; version++)
    {
        int dataCapacityBits = getNumDataCodewords(version, ecl) * 8;  // Number of data bits available
        dataUsedBits = getTotalBits(segs, len, version);
        if (dataUsedBits != -1 && dataUsedBits <= dataCapacityBits)
            break;  // This version number is found to be suitable
        if (version >= maxVersion)    // All versions in the range could not fit the given data
        {
            qrcode[0] = 0;  // Set size to invalid value for safety
            return false;
        }
    }
    assert(dataUsedBits != -1);

    // Increase the error correction level while the data still fits in the current version number
    for (i = (int)qrcodegen_Ecc_MEDIUM; i <= (int)qrcodegen_Ecc_HIGH; i++)
    {
        if (boostEcl && dataUsedBits <= getNumDataCodewords(version, (enum qrcodegen_Ecc)i) * 8)
            ecl = (enum qrcodegen_Ecc)i;
    }

    // Create the data bit string by concatenating all segments
    dataCapacityBits = getNumDataCodewords(version, ecl) * 8;
    memset(qrcode, 0, qrcodegen_BUFFER_LEN_FOR_VERSION(version) * sizeof(qrcode[0]));
    bitLen = 0;
    for (i2 = 0; i2 < len; i2++)
    {
        const struct qrcodegen_Segment *seg = &segs[i2];
        unsigned int modeBits;
        switch (seg->mode)
        {
        case qrcodegen_Mode_NUMERIC     :
            modeBits = 0x1;
            break;
        case qrcodegen_Mode_ALPHANUMERIC:
            modeBits = 0x2;
            break;
        case qrcodegen_Mode_BYTE        :
            modeBits = 0x4;
            break;
        case qrcodegen_Mode_KANJI       :
            modeBits = 0x8;
            break;
        case qrcodegen_Mode_ECI         :
            modeBits = 0x7;
            break;
        default:
            assert(false);
        }
        appendBitsToBuffer(modeBits, 4, qrcode, &bitLen);
        appendBitsToBuffer(seg->numChars, numCharCountBits(seg->mode, version), qrcode, &bitLen);
        for (j = 0; j < seg->bitLength; j++)
            appendBitsToBuffer((seg->data[j >> 3] >> (7 - (j & 7))) & 1, 1, qrcode, &bitLen);
    }

    // Add terminator and pad up to a byte if applicable
    terminatorBits = dataCapacityBits - bitLen;
    if (terminatorBits > 4)
        terminatorBits = 4;
    appendBitsToBuffer(0, terminatorBits, qrcode, &bitLen);
    appendBitsToBuffer(0, (8 - bitLen % 8) % 8, qrcode, &bitLen);

    // Pad with alternate bytes until data capacity is reached
    for (padByte = 0xEC; bitLen < dataCapacityBits; padByte ^= 0xEC ^ 0x11)
        appendBitsToBuffer(padByte, 8, qrcode, &bitLen);
    assert(bitLen % 8 == 0);

    // Draw function and data codeword modules
    appendErrorCorrection(qrcode, version, ecl, tempBuffer);
    initializeFunctionModules(version, qrcode);
    drawCodewords(tempBuffer, getNumRawDataModules(version) / 8, qrcode);
    drawWhiteFunctionModules(qrcode, version);
    initializeFunctionModules(version, tempBuffer);

    // Handle masking
    if (mask == qrcodegen_Mask_AUTO)    // Automatically choose best mask
    {
        long minPenalty = LONG_MAX;
        for (i = 0; i < 8; i++)
        {
            drawFormatBits(ecl, (enum qrcodegen_Mask)i, qrcode);
            applyMask(tempBuffer, qrcode, (enum qrcodegen_Mask)i);
            penalty = getPenaltyScore(qrcode);
            if (penalty < minPenalty)
            {
                mask = (enum qrcodegen_Mask)i;
                minPenalty = penalty;
            }
            applyMask(tempBuffer, qrcode, (enum qrcodegen_Mask)i);  // Undoes the mask due to XOR
        }
    }
    assert(0 <= (int)mask && (int)mask <= 7);
    drawFormatBits(ecl, mask, qrcode);
    applyMask(tempBuffer, qrcode, mask);
    return true;
}


// Returns the number of bits needed to encode the given list of segments at the given version.
// The result is in the range [0, 32767] if successful. Otherwise, -1 is returned if any segment
// has more characters than allowed by that segment's mode's character count field at the version,
// or if the actual answer exceeds INT16_MAX.
testable int getTotalBits(const struct qrcodegen_Segment segs[], size_t len, int version)
{
    int result = 0;
    int numChars;
    int bitLength;
    int ccbits;
    long temp;
    size_t i;
    assert(segs != NULL || len == 0);
    assert(qrcodegen_VERSION_MIN <= version && version <= qrcodegen_VERSION_MAX);
    for (i = 0; i < len; i++)
    {
        numChars = segs[i].numChars;
        bitLength = segs[i].bitLength;
        assert(0 <= numChars && numChars <= INT16_MAX);
        assert(0 <= bitLength && bitLength <= INT16_MAX);
        ccbits = numCharCountBits(segs[i].mode, version);
        assert(0 <= ccbits && ccbits <= 16);
        // Fail if segment length value doesn't fit in the length field's bit-width
        if (numChars >= (1L << ccbits))
            return -1;
        temp = 4L + ccbits + bitLength;
        if (temp > INT16_MAX - result)
            return -1;
        result += temp;
    }
    assert(0 <= result && result <= INT16_MAX);
    return result;
}


// Returns the bit width of the segment character count field for the
// given mode at the given version number. The result is in the range [0, 16].
static int numCharCountBits(enum qrcodegen_Mode mode, int version)
{
    int i;
    assert(qrcodegen_VERSION_MIN <= version && version <= qrcodegen_VERSION_MAX);
    if      ( 1 <= version && version <=  9)  i = 0;
    else if (10 <= version && version <= 26)  i = 1;
    else if (27 <= version && version <= 40)  i = 2;
    else  assert(false);

    switch (mode)
    {
    case qrcodegen_Mode_NUMERIC     :
    {
        const int temp[] = {10, 12, 14};
        return temp[i];
    }
    case qrcodegen_Mode_ALPHANUMERIC:
    {
        const int temp[] = { 9, 11, 13};
        return temp[i];
    }
    case qrcodegen_Mode_BYTE        :
    {
        const int temp[] = { 8, 16, 16};
        return temp[i];
    }
    case qrcodegen_Mode_KANJI       :
    {
        const int temp[] = { 8, 10, 12};
        return temp[i];
    }
    case qrcodegen_Mode_ECI         :
        return 0;
    default:
        assert(false);
    }
}
#endif
