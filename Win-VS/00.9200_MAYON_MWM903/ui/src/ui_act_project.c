/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	uiact.c

Abstract:

   	The routines of user interface action.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

#include "general.h"
#include "board.h"
#include "uiapi.h"
#include "sysapi.h"
#include "asfapi.h"
#include "siuapi.h"
#include "mpeg4api.h"
#include "ciuapi.h"
#include "iduapi.h"
#include "ui_project.h"
#include "adcapi.h"
#include "isuapi.h"
#include "uiKey.h"
#include "ui.h"
#include "rfiuapi.h"
#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
#include "dcfapi.h"
#include "gpioapi.h"
#include "P2pserver_api.h"

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
u8 gu8TimeStamp = 1;    /*1: Enable, 0: Disable*/
u8 OSDTimestameLevel;   /*0: All On, 1: Time only, 2: ALL off*/
u8 uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_640x480;

u8 Current_REC_Mode = 0;
char wav_on=0;
u8 CurrLanguage = 0;
u8 uiSetDefault=0;;
u8 uiFlowSetAlarm = 0;
#if USB_HOST_MASS_SUPPORT
u8 removeHDDStatus; /*0: Failes, 1: Successed*/
#endif

s8 defaultvalue[] =
{
    UI_MENU_REC_MODE_MOTION,     /*cam 1 REC Mode*/
    UI_MENU_REC_MODE_MOTION,     /*cam 2 REC Mode*/
    UI_MENU_REC_MODE_MOTION,     /*cam 3 REC Mode*/
    UI_MENU_REC_MODE_MOTION,     /*cam 4 REC Mode*/
    UI_MENU_SETTING_SENSITIVITY_MID,     /*cam 1 Sensitivity*/
    UI_MENU_SETTING_SENSITIVITY_MID,     /*cam 2 Sensitivity*/
    UI_MENU_SETTING_SENSITIVITY_MID,     /*cam 3 Sensitivity*/
    UI_MENU_SETTING_SENSITIVITY_MID,     /*cam 4 Sensitivity*/
    UI_MENU_SETTING_CAMERA_ON,  /*cam 1*/
    UI_MENU_SETTING_CAMERA_ON,  /*cam 2*/
    UI_MENU_SETTING_CAMERA_OFF,  /*cam 3*/
    UI_MENU_SETTING_CAMERA_OFF,  /*cam 4*/
    UI_MENU_SETTING_RESOLUTION_HD, /*cam 1*/
    UI_MENU_SETTING_RESOLUTION_HD, /*cam 2*/
    UI_MENU_SETTING_RESOLUTION_HD, /*cam 3*/
    UI_MENU_SETTING_RESOLUTION_HD, /*cam 4*/
    UI_MENU_SETTING_BRIGHTNESS_LV3, /*cam 1*/
    UI_MENU_SETTING_BRIGHTNESS_LV3, /*cam 2*/
    UI_MENU_SETTING_BRIGHTNESS_LV3, /*cam 3*/
    UI_MENU_SETTING_BRIGHTNESS_LV3, /*cam 4*/
    UI_MENU_SETTING_OVERWRITE_NO, /*Overwite*/
    UI_MENU_SETTING_SECTION_10MIN,
    UI_MENU_SETIDX_NTP_NO,  /*Time*/
    UI_MENU_FORMAT_NO,
    UI_MENU_DYNAMIC_IP, /*Network*/
    UI_MENU_PANEL,
    0,  /*mask*/
    UI_MENU_VOL_LV3,  /*volume (0-9)(mute-max)*/
    0,  /*not rec*/
    UI_MENU_RF_FULL,    /*FULL_SCREEN*/
    5,              /*P2P Level*/
#if ((HW_BOARD_OPTION == MR9200_RX_MAYON_MWM903) && (PROJ_OPT == 0)||\
    (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM903) && (PROJ_OPT == 1))
    UI_MENU_SETTING_FLICKER_50HZ,
#else
    UI_MENU_SETTING_FLICKER_60HZ,
#endif
    UI_MENU_DEFAULT_NO,
    UI_MENU_ALARM_NO,
    UI_MENU_SETTING_LANGUAGE_ENGLISH,
    0,  /*P2P Password*/
    UI_MENU_PANEL_OUT,
    UI_MENU_SWITCH_OFF,
#if UI_LIGHT_SUPPORT
    UI_LIGHT_MANUAL_OFF, /* Cam1 */
    UI_LIGHT_MANUAL_OFF, /* Cam2 */
    UI_LIGHT_MANUAL_OFF, /* Cam3 */
    UI_LIGHT_MANUAL_OFF, /* Cam4 */
    UI_MENU_CAMERA_ALARM_OFF, /* Cam1 */
    UI_MENU_CAMERA_ALARM_OFF, /* Cam2 */
    UI_MENU_CAMERA_ALARM_OFF, /* Cam3 */
    UI_MENU_CAMERA_ALARM_OFF, /* Cam4 */
#endif
#if UI_CAMERA_ALARM_SUPPORT
    UI_CAMERA_ALARM_MANUAL_OFF, /* Cam1 */
    UI_CAMERA_ALARM_MANUAL_OFF, /* Cam2 */
    UI_CAMERA_ALARM_MANUAL_OFF, /* Cam3 */
    UI_CAMERA_ALARM_MANUAL_OFF, /* Cam4 */
    UI_MENU_CAMERA_ALARM_OFF, /* Cam1 */
    UI_MENU_CAMERA_ALARM_OFF, /* Cam2 */
    UI_MENU_CAMERA_ALARM_OFF, /* Cam3 */
    UI_MENU_CAMERA_ALARM_OFF, /* Cam4 */
#endif
    UI_MENU_ALERT_PERIOD_15s,
#if USB_HOST_MASS_SUPPORT
    UI_MENU_DEFAULT_NO, /* HDD REMOVE NO */
#endif
    0xf,                  /*AREC*/
    0  /*checksum*/
};




/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */

#if RFIU_SUPPORT
extern unsigned int         gRfiu_WrapDec_Sta[MAX_RFIU_UNIT];
extern unsigned int         gRfiu_MpegDec_Sta[MAX_RFIU_UNIT];
extern unsigned int         gRfiu_WrapEnc_Sta[MAX_RFIU_UNIT];
extern DEF_RFIU_UNIT_CNTL   gRfiuUnitCntl[];
#endif
extern OS_EVENT             *mpeg4ReadySemEvt;
extern u32                  guiIISPlayDMAId;
extern u32                  guiIISRecDMAId;
extern u8  lastMenuMode;
extern u8  UICheckSchedule;
extern u8 Reminder_FW_Upgrade;
#if(UI_BAT_SUPPORT)
extern u8 gUiPIRSch[4];
extern u8 _uiCheckBatterySch;
#endif
extern u8 uiSetRfLSTimer[MULTI_CHANNEL_MAX][7];
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

extern void uiSetTVOutXY(u8 mode);

/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */

s32 uiMenuSet_ChannelControl(u8 setting)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_ChannelControl %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_ChannelControl %d \r\n",setting);
    switch (setting)
    {
        case UI_MENU_RF_QUAD:
            break;
        case UI_MENU_RF_FULL:
            break;
        default:
            return 1;
    }
    return 1;
}

s32 uiMenuSet_Zoom(u8 setting)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Zoom %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_Zoom %d \r\n",setting);
    switch (setting)
    {
        case UI_MENU_CH1:
            break;
        case UI_MENU_CH2:
            break;
        case UI_MENU_CH3:
            break;
        case UI_MENU_CH4:
            break;
        default:
            return 1;
    }
    return 1;
}

/*

Routine Description:

	Set video quality.

Arguments:

	setting - Setting.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_VideoQuality(s8 setting)
{
	u8 quality;

    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_VideoQuality %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_VideoQuality %d \r\n",setting);
    switch (setting)
    {
        case UI_MENU_VIDEO_QUALITY_HIGH:
            quality = MPEG4_VIDEO_QUALITY_HIGH;
            break;

        case UI_MENU_VIDEO_QUALITY_MEDIUM:
            quality = MPEG4_VIDEO_QUALITY_MEDIUM;
            break;

        case UI_MENU_VIDEO_QUALITY_LOW:
            quality = MPEG4_VIDEO_QUALITY_LOW;
            break;

        default:
            quality = MPEG4_VIDEO_QUALITY_HIGH;
            break;
    }

	mpeg4SetVideoQuality(quality);

	return 1;
}

/*

Routine Description:

	Set video quality.

Arguments:

	setting - Setting.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_VideoFrameRate(s8 setting)
{
    u32 FrameRate;
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSetVideoFrameRate %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSetVideoFrameRate %d \r\n",setting);
    switch (setting)
    {
        case UI_MENU_VIDEO_FRAMERATE_30:
            FrameRate = MPEG4_VIDEO_FRAMERATE_30;
            break;

        case UI_MENU_VIDEO_FRAMERATE_15:
            FrameRate = MPEG4_VIDEO_FRAMERATE_15;
            break;

        case UI_MENU_VIDEO_FRAMERATE_5:
            FrameRate = MPEG4_VIDEO_FRAMERATE_5;
            break;

        case UI_MENU_VIDEO_FRAMERATE_10:
            FrameRate = MPEG4_VIDEO_FRAMERATE_10;
            break;

        default:
            FrameRate = MPEG4_VIDEO_FRAMERATE_30;
            break;
    }
    mpeg4SetVideoFrameRate(FrameRate);

    return 1;
}

s32 uiMenuSet_Display(u8 setting)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Display %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_Display %d \r\n",setting);
    return 0;
    switch (setting)
    {
        case UI_MENU_TV:
            if (sysTVOutOnFlag == SYS_OUTMODE_PANEL)
            {
                DEBUG_UI("PANNEL MODE\r\n");
                sysSetOutputMode(SYS_OUTMODE_TV);
            }
            break;

        case UI_MENU_PANEL:
            if (sysTVOutOnFlag == SYS_OUTMODE_TV)
            {
                DEBUG_UI("PANNEL MODE\r\n");
                sysSetOutputMode(SYS_OUTMODE_PANEL);
            }
            break;

        default:
            return 1;
    }
    return 1;
}

// box don't change to panel
#if ((HW_BOARD_OPTION == MR8120_RX_MAYON_MWM011)||(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014)||\
        (HW_BOARD_OPTION == MR8200_RX_JIT_BOX)||(HW_BOARD_OPTION == MR8120_RX_JIT_BOX)||(HW_BOARD_OPTION == MR8200_RX_JIT_BOX_AV) ||\
        (HW_BOARD_OPTION == MR8120_RX_JIT_D808SW3) || (HW_BOARD_OPTION == MR8200_RX_JIT_D808SN4))
s32 uiMenuSet_TVout_Format(u8 setting)
{
    u8 uartCmd[20];
    u8 i;
    int RfBusy = 1, cnt = 0;
#if 0
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_TVout_Format %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_TVout_Format %d \r\n",setting);
    if (Main_Init_Ready == 0)
        return 0;
#endif
    switch (setting)
    {
        case UI_MENU_NTSC:
            //if (sysTVOutOnFlag == TV_IN_PAL)
            {
                if (MyHandler.MenuMode != VIDEO_MODE)
                {
                    lastMenuMode = VIDEO_MODE;
                    uiFlowSetupToPreview();
                }
                sysSetOutputMode(SYS_OUTMODE_TV);
                DEBUG_UI("TV out change to NTSC\r\n");
                sysTVinFormat = TV_IN_NTSC;
                iduSwitchNTSCPAL(SYS_TV_OUT_NTSC);
                iconflag[UI_MENU_SETIDX_50HZ_60HZ] = UI_MENU_NTSC;
                sprintf((char *)uartCmd,"FLIC %d", SENSOR_AE_FLICKER_60HZ);
                if(sysTVOutOnFlag==1) // Amon: TV out need to update xy (140109)
                    uiSetTVOutXY(SYS_TV_OUT_NTSC);
            }            
            Save_UI_Setting();
            break;

        case UI_MENU_PAL:
            //if (sysTVinFormat == TV_IN_NTSC)
            {
                if (MyHandler.MenuMode != VIDEO_MODE)
                {
                    lastMenuMode = VIDEO_MODE;
                    uiFlowSetupToPreview();
                }
                sysSetOutputMode(SYS_OUTMODE_TV);
                DEBUG_UI("TV out change to PAL\r\n");
                sysTVinFormat = TV_IN_PAL;
                iduSwitchNTSCPAL(SYS_TV_OUT_PAL);
                iconflag[UI_MENU_SETIDX_50HZ_60HZ] = SYS_TV_OUT_PAL;
                sprintf((char *)uartCmd,"FLIC %d", SENSOR_AE_FLICKER_50HZ);
                if(sysTVOutOnFlag==1) // Amon: TV out need to update xy (140109)
                    uiSetTVOutXY(SYS_TV_OUT_PAL);
            }            
            Save_UI_Setting();
            break;
#if 0
        case UI_MENU_PANEL_OUT:
            if(sysTVOutOnFlag==1)
                uiSetOutputMode(SYS_OUTMODE_PANEL);
            DEBUG_UI("PANNEL MODE\r\n");
            break;
#endif
        default:
            return 1;
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
    return 1;

}
#else
s32 uiMenuSet_TVout_Format(u8 setting)
{
    u8 uartCmd[20];
    u8 i;
    int RfBusy = 1, cnt = 0;

    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_TVout_Format %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_TVout_Format %d \r\n",setting);
    if (Main_Init_Ready == 0)
        return 0;

    if(uiSetDefault == 0)
    {
        if (MyHandler.MenuMode != VIDEO_MODE)
            uiFlowSetupToPreview();
    }
    
    switch (setting)
    {
        case UI_MENU_NTSC:
            //if (sysTVOutOnFlag == TV_IN_PAL)
            {
                sysSetOutputMode(SYS_OUTMODE_TV);
                DEBUG_UI("TV out change to NTSC\r\n");
                sysTVinFormat = TV_IN_NTSC;
                iduSwitchNTSCPAL(SYS_TV_OUT_NTSC);
                iconflag[UI_MENU_SETIDX_50HZ_60HZ] = UI_MENU_NTSC;
                sprintf((char *)uartCmd,"FLIC %d", SENSOR_AE_FLICKER_60HZ);
                if(sysTVOutOnFlag==1) // Amon: TV out need to update xy (140109)
                    uiSetTVOutXY(SYS_TV_OUT_NTSC);
            }            
            break;

        case UI_MENU_PAL:
            //if (sysTVinFormat == TV_IN_NTSC)
            {
                sysSetOutputMode(SYS_OUTMODE_TV);
                DEBUG_UI("TV out change to PAL\r\n");
                sysTVinFormat = TV_IN_PAL;
                iduSwitchNTSCPAL(SYS_TV_OUT_PAL);
                iconflag[UI_MENU_SETIDX_50HZ_60HZ] = SYS_TV_OUT_PAL;
                sprintf((char *)uartCmd,"FLIC %d", SENSOR_AE_FLICKER_50HZ);
                if(sysTVOutOnFlag==1) // Amon: TV out need to update xy (140109)
                    uiSetTVOutXY(SYS_TV_OUT_PAL);
            }            
            break;

        case UI_MENU_PANEL_OUT:
            if(sysTVOutOnFlag==1)
                sysSetOutputMode(SYS_OUTMODE_PANEL);
            DEBUG_UI("PANNEL MODE\r\n");
            break;

        default:
            return 1;
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
    return 1;

}
#endif

s8 uiMenuSet_TX_CameraResolution(s8 setting,u8 camera)
{
    u8 recState, RfBusy = UI_SET_RF_BUSY;
#if 0    
    if (Main_Init_Ready == 1)
        DEBUG_UI("cam %d uiMenuSet_TX_CameraResolution %d\r\n",camera,setting);
    else
        DEBUG_MAIN("cam %d uiMenuSet_TX_CameraResolution %d\r\n",camera,setting);
#endif

    UI_CFG_RES[camera]=setting;

    if (uiSetDefault == 1)
        return 1;
    if (Main_Init_Ready == 0)
        return 1;
    if (uiP2PMode != 0)
        return 1;

#if MULTI_CHANNEL_VIDEO_REC
    recState = MultiChannelGetCaptureVideoStatus(camera+MULTI_CHANNEL_LOCAL_MAX);
    if (recState != 0)
    {
        DEBUG_UI("cam %d Stop Capture when change resolution\r\n",camera);
        uiCaptureVideoStopByChannel(camera);
    }
#endif

    switch (setting)
    {
        case UI_MENU_SETTING_RESOLUTION_1920x1088:
            RfBusy=uiSetRfResolutionRxToTx(UI_RESOLTUION_FHD, camera);            
            break;

        case UI_MENU_SETTING_RESOLUTION_HD:
            RfBusy=uiSetRfResolutionRxToTx(UI_RESOLTUION_HD, camera);            
            break;

        default:
            DEBUG_UI("Error cam %d uiMenuSet_TX_VideoResolution %d\r\n", camera,setting);
            iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+camera] = defaultvalue[UI_MENU_SETIDX_RESOLUTION_CH1+camera];
            uiMenuSet_TX_CameraResolution(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+camera],(UI_MENU_SETIDX_RESOLUTION_CH1+camera));
            return 0;
    }
    if ((recState != 0) && (MyHandler.MenuMode != SET_NUMBER_MODE) && (MyHandler.MenuMode != SETUP_MODE))
    {
        DEBUG_UI("cam %d Restart Capture\r\n",camera);
        uiCaptureVideoByChannel(camera);
    }

    if (RfBusy == UI_SET_RF_BUSY)
        DEBUG_UI("cam %d uiMenuSet_TX_CameraResolution Timeout:%d !!!\n",camera,setting);

    return 1;        
}

s32 uiMenuSet_Overwrite(u8 setting)
{
    int i;
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Overwrite %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_Overwrite %d \r\n",setting);

    if (Main_Init_Ready == 1)
        uiCaptureVideoStop();
    
    switch (setting)
    {
        case UI_MENU_SETTING_OVERWRITE_YES:
            //sysCaptureVideoMode = ASF_CAPTURE_OVERWRITE;
            sysCaptureVideoMode    |= ASF_CAPTURE_OVERWRITE_ENA;
            SysOverwriteFlag = TRUE;
        #if MULTI_CHANNEL_VIDEO_REC
            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                VideoClipParameter[i].sysCaptureVideoMode  |= ASF_CAPTURE_OVERWRITE_ENA;
        #endif
            break;

        case UI_MENU_SETTING_OVERWRITE_NO:
            //sysCaptureVideoMode = ASF_CAPTURE_NORMAL;
            sysCaptureVideoMode    &= ~ASF_CAPTURE_OVERWRITE_ENA;
            SysOverwriteFlag = FALSE;
        #if MULTI_CHANNEL_VIDEO_REC
            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                VideoClipParameter[i].sysCaptureVideoMode  &= ~ASF_CAPTURE_OVERWRITE_ENA;
        #endif
            break;

        default:
            DEBUG_UI("Error uiMenuSet_Overwrite %d \r\n", setting);
            iconflag[UI_MENU_SETIDX_OVERWRITE] = defaultvalue[UI_MENU_SETIDX_OVERWRITE];
            uiMenuSet_Overwrite(iconflag[UI_MENU_SETIDX_OVERWRITE]);
            return 0;
    }
    return 1;
}

s32 uiMenuSet_Section(u8 setting)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Section %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_Section %d \r\n",setting);
    switch (setting)
    {
        case UI_MENU_SETTING_SECTION_5MIN:
               asfSetVideoSectionTime(5 * 60);
            break;
        case UI_MENU_SETTING_SECTION_10MIN:
               asfSetVideoSectionTime(10 * 60);
            break;
        case UI_MENU_SETTING_SECTION_15MIN:
               asfSetVideoSectionTime(15 * 60);
            break;
            
        default:
            asfSetVideoSectionTime(5 * 60);
            return 1;
    }
    return 1;
}

s32 uiMenuSet_MotionMask(void)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_MotionMask\r\n");
    else
        DEBUG_MAIN("uiMenuSet_MotionMask\r\n");

#if HW_MD_SUPPORT
    //mduMotionDetect_Mask_Config(&MotionMaskArea[0][0]);
    memcpy(UI_HD_MaskArea[CamId], uiMaskArea[CamId], sizeof(UI_HD_MaskArea[CamId]));
    uiSetRfMotionMaskArea(CamId, 0);
#endif

    iconflag[UI_MENU_SETIDX_MOTION_MASK] = 0;
    return 1;
}

s32 uiMenuSet_DateTime(u8 setting)
{
    u8 i;
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_DateTime %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_DateTime %d \r\n",setting);

    if(Main_Init_Ready == 1)
    {
        uiCaptureVideoStop();
        if (uiSetDefault == 0)
        {
            RTC_Set_Time(&SetTime);
            UICheckSchedule = 48;
            #if(UI_BAT_SUPPORT)
            _uiCheckBatterySch = 48;  
            sysResetBTCCheckLev();
            #endif
        }
    }

#if (NIC_SUPPORT)
    if (setting == UI_MENU_SETIDX_NTP_YES)
    { 
        NTP_Switch(NTP_SWITCH_ON);
        if(Main_Init_Ready == 1)
            sysback_Net_SetEvt(SYS_BACKRF_NTE_NTP_UPDATE, 0, 0);
    }
    else
        NTP_Switch(NTP_SWITCH_OFF);
#endif
    OSTimeDly(10);
#if RFIU_SUPPORT
    for (i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        //uiSetTxTime[i] = uiSetRfTimeRxToTx(i);
        uiSetRfTimeRxToTx(i);
    }
#endif
#if 0
    RTC_Set_TimeZone(&SetZone);
#endif
    return 1;
}

/*

Routine Description:

	Set format.

Arguments:

	setting - Setting.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_Format(s8 setting)
{
    u8  err;

    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Format %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_Format %d \r\n",setting);


	if (setting == UI_MENU_FORMAT_YES)
	{
        uiCaptureVideoStop();
		iconflag[UI_MENU_SETIDX_FORMAT] = UI_MENU_FORMAT_NO; /* Reset default to "No" */
        general_MboxEvt->OSEventPtr=(void *)0;

        if(OSFlagAccept(gUiStateFlagGrp, FLAGUI_SD_GOTO_FORMAT, OS_FLAG_WAIT_SET_ANY, &err)>0)
            sysbackSetEvt(SYS_BACK_PLAYBACK_FORMAT, 0);
        else
            sysSetEvt(SYS_EVT_PLAYBACK_FORMAT, 0);
	}

	return 1;
}

#if(NIC_SUPPORT)
s32 uiMenuSet_Network()
{
    if (UINetInfo.IsStaticIP == 0)
    {
        if (Main_Init_Ready == 1)
            DEBUG_UI("Enable DHCP\r\n");
        else
            DEBUG_MAIN("Enable DHCP\r\n");
    }
    else
    {
        if (Main_Init_Ready == 1)
        {
            DEBUG_UI("IP Address       :%u.%u.%u.%u \n",UINetInfo.IPaddr[0], UINetInfo.IPaddr[1], UINetInfo.IPaddr[2], UINetInfo.IPaddr[3]);
            DEBUG_UI("Subnet Mask      :%u.%u.%u.%u \n",UINetInfo.Netmask[0], UINetInfo.Netmask[1], UINetInfo.Netmask[2], UINetInfo.Netmask[3]);
            DEBUG_UI("Default Getway   :%u.%u.%u.%u \n",UINetInfo.Gateway[0], UINetInfo.Gateway[1], UINetInfo.Gateway[2], UINetInfo.Gateway[3]);
        }
        else
        {
            DEBUG_MAIN("IP Address       :%u.%u.%u.%u \n",UINetInfo.IPaddr[0], UINetInfo.IPaddr[1], UINetInfo.IPaddr[2], UINetInfo.IPaddr[3]);
            DEBUG_MAIN("Subnet Mask      :%u.%u.%u.%u \n",UINetInfo.Netmask[0], UINetInfo.Netmask[1], UINetInfo.Netmask[2], UINetInfo.Netmask[3]);
            DEBUG_MAIN("Default Getway   :%u.%u.%u.%u \n",UINetInfo.Gateway[0], UINetInfo.Gateway[1], UINetInfo.Gateway[2], UINetInfo.Gateway[3]);
        }
        SetNetworkInfo(&UINetInfo);
    }

}

s32 uiMenuSet_Dynamic_ON(u8 setting)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Dynamic_ON %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_Dynamic_ON %d \r\n",setting);

    switch (setting)
    {
        case UI_MENU_DYNAMIC_IP:
            UINetInfo.IsStaticIP = 0;
            break;

        case UI_MENU_STATIC_IP:
            UINetInfo.IsStaticIP = 1;
            break;

        default:
            DEBUG_UI("Error uiMenuSet_Dynamic_ON %d \r\n", setting);
            iconflag[UI_MENU_SETIDX_NETWORK_STATUS] = defaultvalue[UI_MENU_SETIDX_NETWORK_STATUS];
            uiMenuSet_Network();
            break;
    }
    SetNetworkInfo(&UINetInfo);
    
}
#endif

s32 uiMenuSet_Language(u8 setting)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Language %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_Language %d \r\n",setting);

    CurrLanguage = setting;
    return 1;
}

s32 uiMenuSet_Alarm(u8 setting)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Alarm %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_Alarm %d \r\n",setting);
    switch (setting)
    {
        case 0: // alarm On
            uiFlowSetAlarm=UI_ALARM_ON;
            break;

        case 1: // alarm Off
            uiFlowSetAlarm=UI_ALARM_OFF;
            
        default:
            break;
    }
    return 1;
}

s32 uiMenuSet_Default(u8 setting)
{
    u32 i;
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Default %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_Default %d \r\n",setting);

    if (Main_Init_Ready == 0)
        return 0;

    switch (setting)
    {
        case UI_MENU_DEFAULT_YES:
            uiCaptureVideoStop();
            uiSetDefault = 1;
            uiOsdDrawRestoreDefaltSettings(1);
            uiSetDefaultSetting();
            uiSetCfg = 0x11;
            for(i=0;i<UIACTIONNUM;i++)
                uiMenuAction(i);
            uiSetCfg = 0;            
            if(!sysTVOutOnFlag)
                uiOsdDrawRestoreDefaltSettings(0);
            _uiCheckBatterySch = 48;
            uiSetDefault = 0;
            break;

        default:
            break;
    }
    return 1;
}

#if UI_LIGHT_SUPPORT
s32 uiMenuSet_LS_Timer(s8 setting, u8 nCamCount)
{
    u8 RfBusy=UI_SET_RF_BUSY;
#if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
    u8  i;
#endif
#if 0
    if (Main_Init_Ready == 1)
        DEBUG_UI("Cam %d uiMenuSet_LS_Timer %d \r\n",nCamCount,setting);
    else
        DEBUG_MAIN("Cam %d uiMenuSet_LS_Timer %d \r\n",nCamCount,setting);
#endif

    #if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
    DEBUG_UI("Cam %d weekval2 = %d \r\n",nCamCount , setting);
    DEBUG_UI("Cam %d uiLightTimer = %02d:%02d-%02d:%02d \r\n",nCamCount ,uiLightTimer[nCamCount][0],uiLightTimer[nCamCount][1],uiLightTimer[nCamCount][2],uiLightTimer[nCamCount][3]);
    uiSetRfLightTimerRxToTx(uiLightTimer[nCamCount][0], uiLightTimer[nCamCount][1], uiLightTimer[nCamCount][2], uiLightTimer[nCamCount][3], setting, nCamCount, uiSetCfg);
    #elif (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
    uiSetCfg |= (0<<5);
    for (i = 0; i < 7; i++) // 7 days a week
    {
        RfBusy=uiSetRfLightTimerRxToTx(i, uiLightInterval[nCamCount][i][0],uiLightInterval[nCamCount][i][1],uiLightInterval[nCamCount][i][2],uiLightInterval[nCamCount][i][3],
                                   uiLightInterval[nCamCount][i][4],uiLightInterval[nCamCount][i][5], nCamCount, uiSetCfg);
        uiSetRfLSTimer[nCamCount][i] = RfBusy;
    }
    #endif
    
    return 1;
}

s32 uiMenuSet_LS_Status(s8 setting, u8 nCamCount)
{

#if 0
    if (Main_Init_Ready == 1)
        DEBUG_UI("Cam %d uiMenuSet_LS_Status %d \r\n",nCamCount,setting);
    else
        DEBUG_MAIN("Cam %d uiMenuSet_LS_Status %d \r\n",nCamCount,setting);
#endif

    if (setting > 4)
    {
        if (Main_Init_Ready == 0)
        {
	        iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+nCamCount] = defaultvalue[UI_MENU_SETIDX_CH1_LS_STATUS+nCamCount];
	        uiMenuSet_LS_Status(iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+nCamCount], (UI_MENU_SETIDX_CH1_LS_STATUS+nCamCount));
        }
        return 0; 
    }
    else
    {
        switch(setting)
        {
            case UI_LIGHT_MANUAL_ON:
                uiSetRfLightState[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_LIGHT_MANUAL_ON, nCamCount);
                break;
            case UI_LIGHT_MANUAL_OFF:
                uiSetRfLightState[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_LIGHT_MANUAL_OFF, nCamCount);
                break;
            case UI_LIGHT_OFF:
                uiSetRfLightState[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_LIGHT_MANUAL_OFF, nCamCount);
                break;
            case UI_LIGHT_TRIGGER_ON:
                uiSetRfLightState[nCamCount] = uiSetRfManualLightingRxToTx(8, nCamCount);
                break;
        }
    }    
    return 1;
}

s32 uiMenuSet_LS_ONOFF(s8 setting, u8 nCamCount)
{
#if 0
    if (Main_Init_Ready == 1)
        DEBUG_UI("Cam %d uiMenuSet_LS_ONOFF %d \r\n",nCamCount,setting);
    else
        DEBUG_MAIN("Cam %d uiMenuSet_LS_ONOFF %d \r\n",nCamCount,setting);
#endif

    switch(setting)
    {
        case UI_MENU_CAMERA_ALARM_OFF:
            uiSetRfLightSwitch[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_LIGHT_FUNC_OFF, nCamCount);
            break;
            
        case UI_MENU_CAMERA_ALARM_ON:
            uiSetRfLightSwitch[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_LIGHT_FUNC_ON, nCamCount);
            break;
                        
        default:
            if (Main_Init_Ready == 0)
            {
    	        iconflag[UI_MENU_SETIDX_CH1_LS_ONOFF+nCamCount] = defaultvalue[UI_MENU_SETIDX_CH1_LS_ONOFF+nCamCount];
    	        uiMenuSet_LS_ONOFF(iconflag[UI_MENU_SETIDX_CH1_LS_ONOFF+nCamCount], (UI_MENU_SETIDX_CH1_LS_ONOFF+nCamCount));
            }
            return 0; 
    }
    
    return 1;
}

#endif

#if UI_CAMERA_ALARM_SUPPORT
s32 uiMenuSet_CameraAlarm_Timer(s8 setting, u8 nCamCount)
{
    u8 RfBusy=UI_SET_RF_BUSY;
#if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
    u8  i;
#endif
#if 1
    if (Main_Init_Ready == 1)
        DEBUG_UI("Cam %d uiMenuSet_CA_Timer %d \r\n",nCamCount,setting);
    else
        DEBUG_MAIN("Cam %d uiMenuSet_CA_Timer %d \r\n",nCamCount,setting);
#endif
    #if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
    DEBUG_UI("Cam %d weekval2 = %d \r\n",nCamCount , setting);
    DEBUG_UI("Cam %d uiLightTimer = %02d:%02d-%02d:%02d \r\n",nCamCount ,uiLightTimer[nCamCount][0],uiLightTimer[nCamCount][1],uiLightTimer[nCamCount][2],uiLightTimer[nCamCount][3]);
    uiSetRfLightTimerRxToTx(uiLightTimer[nCamCount][0], uiLightTimer[nCamCount][1], uiLightTimer[nCamCount][2], uiLightTimer[nCamCount][3], setting, nCamCount, uiSetCfg);
    #elif (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
    uiSetCfg |= (1<<5);
    for (i = 0; i < 7; i++) // 7 days a week
    {
        RfBusy=uiSetRfLightTimerRxToTx(i, uiCamAlarmInterval[nCamCount][i][0],uiCamAlarmInterval[nCamCount][i][1],uiCamAlarmInterval[nCamCount][i][2],uiCamAlarmInterval[nCamCount][i][3],
                                   uiCamAlarmInterval[nCamCount][i][4],uiCamAlarmInterval[nCamCount][i][5], nCamCount, uiSetCfg);
        uiSetRfAlarmTimer[nCamCount][i] = RfBusy;
    }
    
    #endif
    return 1;
}

s32 uiMenuSet_CameraAlarm_Status(s8 setting, u8 nCamCount)
{

#if 0
    if (Main_Init_Ready == 1)
        DEBUG_UI("Cam %d uiMenuSet_CA_Status %d \r\n",nCamCount,setting);
    else
        DEBUG_MAIN("Cam %d uiMenuSet_CA_Status %d \r\n",nCamCount,setting);
#endif

    if (setting > 4)
    {
        if (Main_Init_Ready == 0)
        {
	        iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+nCamCount] = defaultvalue[UI_MENU_SETIDX_CH1_CA_STATUS+nCamCount];
	        uiMenuSet_CameraAlarm_Status(iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+nCamCount], (UI_MENU_SETIDX_CH1_CA_STATUS+nCamCount));
        }
        return 0; 
    }
    #if 0
    else if (setting < 2)
    {
        uiSetRfAlarmState[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_ALARM_MANUAL_OFF, nCamCount);
    }
    else 
    {
        uiSetRfAlarmState[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_ALARM_MANUAL_ON, nCamCount);
    }
    #else
    else
    {
        switch(setting)
        {
            case UI_CAMERA_ALARM_MANUAL_ON:
                uiSetRfAlarmState[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_ALARM_MANUAL_ON, nCamCount);
                break;
            case UI_CAMERA_ALARM_MANUAL_OFF:
                uiSetRfAlarmState[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_ALARM_MANUAL_OFF, nCamCount);
                break;
            case UI_CAMERA_ALARM_TRIGGER_ON:
                uiSetRfAlarmState[nCamCount] = uiSetRfManualLightingRxToTx(8, nCamCount);
                break;
            case UI_CAMERA_ALARM_OFF:
                uiSetRfAlarmState[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_ALARM_MANUAL_OFF, nCamCount);
                break;
        }
    }    
    #endif
    return 1;
}

s32 uiMenuSet_CA_ONOFF(s8 setting, u8 nCamCount)
{
#if 0
    if (Main_Init_Ready == 1)
        DEBUG_UI("Cam %d uiMenuSet_CA_ONOFF %d \r\n",nCamCount,setting);
    else
        DEBUG_MAIN("Cam %d uiMenuSet_CA_ONOFF %d \r\n",nCamCount,setting);
#endif

    switch(setting)
    {
        case UI_MENU_CAMERA_ALARM_OFF:
            uiSetRfAlarmSwitch[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_ALARM_FUNC_OFF, nCamCount);
            break;
            
        case UI_MENU_CAMERA_ALARM_ON:
            uiSetRfAlarmSwitch[nCamCount] = uiSetRfManualLightingRxToTx(UI_TX_ALARM_FUNC_ON, nCamCount);
            break;
                        
        default:
            if (Main_Init_Ready == 0)
            {
    	        iconflag[UI_MENU_SETIDX_CH1_CA_ONOFF+nCamCount] = defaultvalue[UI_MENU_SETIDX_CH1_CA_ONOFF+nCamCount];
    	        uiMenuSet_CA_ONOFF(iconflag[UI_MENU_SETIDX_CH1_CA_ONOFF+nCamCount], (UI_MENU_SETIDX_CH1_CA_ONOFF+nCamCount));
            }
            return 0; 
    }
    
    return 1;
}

#endif

s32 uiMenuSet_Flicker(u8 setting)
{
    u8 flicker;

    //DEBUG_UI("uiMenuSet_Flicker %d \r\n",setting);

    if (Main_Init_Ready == 0)
        return 0;

    switch (setting)
    {
        case 1:
            flicker = SENSOR_AE_FLICKER_60HZ;
            break;

        case 0:
            flicker = SENSOR_AE_FLICKER_50HZ;
            break;
            
        default:
            break;
    }
    uiSetRfFlickerRxToTx(flicker);
    return 1;
}

s32 uiMenuSet_Audio_Vol(s8 setting)
{
#if (AUDIO_DEVICE== AUDIO_IIS_WM8940)
#else
    u32 volVal[10] = {31, 25, 22, 19, 16, 10, 8, 6, 4, 0};
#endif

    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Audio_Vol %d\r\n",setting);
    else
    {
        DEBUG_MAIN("uiMenuSet_Audio_Vol %d\r\n",setting);
#if 0 //(AUDIO_DEVICE== AUDIO_IIS_WM8940)
        return;
#endif
    }

    if ((setting > 9) || (setting < 0))
    {
        DEBUG_UI("uiMenuSet_Audio_Vol %d Fail\r\n",setting);
        return 0;
    }
    sysVolumnControl = setting;
#if (AUDIO_DEVICE== AUDIO_IIS_WM8940)
    if (setting == 0)
        WM8940_SpeakerMute();
    else
        WM8940_AdjustSpeakerVolume((u8)setting+2);
#else
    adcSetDAC_OutputGain(volVal[setting]);
#endif
    return 1;
}


void uiMenuSet_UpgradeFW(s8 setting)
{
    u8  err,i;

    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_UpgradeFW %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_UpgradeFW %d \r\n",setting);

    if((setting == UI_MENU_UPDATE_YES) )
    {
        uiCaptureVideoStop();
        for ( i = RFIU_TASK_PRIORITY_HIGH; i < MAIN_TASK_PRIORITY_END; i++)
        {
            if ((i == SYSTIMER_TASK_PRIORITY) ||
                (i == TIMER_TICK_TASK_PRIORITY) ||
                (i == UI_TASK_PRIORITY) ||
                (i == UARTCMD_TASK1_PRIORITY) ||
                (i == SYS_TASK_PRIORITY))
                
            {
                continue;
            }
            DEBUG_UI("UI OSTaskDel %d!\n",i);
            OSTaskSuspend(i);
            OSTaskDel(i);
        }
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_CLR, &err);
        general_MboxEvt->OSEventPtr=(void *)0;
        sysSetEvt(SYS_EVT_UPGRADE_FW, 0);        
    }
}

void uiMenuSet_UpgradeNet(s8 setting)
{
    s32 isp_return;
    u8  err,i;
    s32 RetVal;

    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_UpgradeFW %d \r\n",setting);
    else
        DEBUG_MAIN("uiMenuSet_UpgradeFW %d \r\n",setting);

    if (Main_Init_Ready == 0)
        return;
    
    if((setting == UI_MENU_UPDATE_YES) )
    {
        uiCaptureVideoStop();
        for ( i = RFIU_TASK_PRIORITY_HIGH; i < MAIN_TASK_PRIORITY_END; i++)
        {
            if ((i == SYSTIMER_TASK_PRIORITY) ||
            #if (NIC_SUPPORT == 1)     
                (i == SYSBACK_NET_TASK_PRIORITY)||
                (i == T_LWIP_THREAD_START_PRIO)||
                (i == TCPIP_THREAD_PRIO_t)||
                (i == T_ETHERNETIF_INPUT_PRIO)||
                (i == T_LWIPENTRY_PRIOR)||                
            #endif
                (i == UI_TASK_PRIORITY) ||
                (i == TIMER_TICK_TASK_PRIORITY) ||
                (i == UARTCMD_TASK1_PRIORITY) ||
                (i == SYS_TASK_PRIORITY))
            {
                continue;
            }
            DEBUG_UI("UI OSTaskDel %d!\n",i);
            OSTaskSuspend(i);
            OSTaskDel(i);
        }
        OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_CLR, &err);
        #if (NIC_SUPPORT == 1)
        RetVal = sysback_Net_SetEvt(SYS_BACKRF_NTE_FW_UPDATE, 0, 0);
        #endif
        #if 1
        if (RetVal == 0)
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_SET, &err);
        #endif
    }
}

#if(TUTK_SUPPORT)
s32 uiMenuSet_P2PID(u8 *p2pID)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_P2PID %s \r\n",p2pID);
    else
        DEBUG_MAIN("uiMenuSet_P2PID %s \r\n",p2pID);
    spiWriteNet();
    return 1;
}
#endif

#if(RFIU_SUPPORT)
s32 uiMenuSet_RFID(u32 *rfID)
{
    u8 i;
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_RFID: \n");
    else
        DEBUG_MAIN("uiMenuSet_RFID: \n");
    for(i=0;i<RFID_MAX_WORD;i++)
        DEBUG_UI("%x \n",rfID[i]);

    spiWriteRF();
    return 1;
}

s32 uiMenuSet_RFID_CODE(u32 *rfID)
{
    u8 i;
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_RFID_CODE: \n");
    else
        DEBUG_MAIN("uiMenuSet_RFID_CODE: \n");
    for(i=0;i<RFID_MAX_WORD;i++)
        DEBUG_UI("%x \n",rfID[i]);

    spiWriteRF();
    return 1;
}
#endif

#if(NIC_SUPPORT)
s32 uiMenuSet_MACAddr(u8 *macAddr)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_MACAddr: %2x-%2x-%2x-%2x-%2x-%2x\n",macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
    else
        DEBUG_MAIN("uiMenuSet_MACAddr: %2x-%2x-%2x-%2x-%2x-%2x\n",macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
    spiWriteNet();
    return 1;
}
#endif
void uiInitDAC_Play(void)
{
	u32	unAudioVol;

	unAudioVol = (u32) iconflag[UI_MENU_SETIDX_VOLUME];

	adcInitDAC_Play(unAudioVol);

}

s32 uiMenuSet_MotionSensitivity(u8 setting, u8 nCamCount)
{
    u8 level = 7;
    u8  MEnable = 3;//Day : 0x01 Night : 0x10
    //DEBUG_GREEN("uiMenuSet_MotionSensitivity %d Camera %d\r\n", setting, nCamCount);    
    if (Main_Init_Ready == 0)
        return 0;

    switch(setting)
    {
        case UI_MENU_SETTING_SENSITIVITY_LOW: 
            level = UI_MOTION_SENSITIVITY_L;
            break;
            
        case UI_MENU_SETTING_SENSITIVITY_MID:
            level = UI_MOTION_SENSITIVITY_M;
            break;
            
        case UI_MENU_SETTING_SENSITIVITY_HIGHT:
            level = UI_MOTION_SENSITIVITY_H;
            break;    
            
        default:
            DEBUG_UI("Error uiMenuSet_MotionSensitivity %d Camera %d\r\n", setting, nCamCount);
            break;
            
    }
    uiSetRfMotionRxToTx(MEnable, level, level, nCamCount);
    return 1;
}


s32 uiMenuSet_REC_MODE_BY_CH(u8 setting, u8 nCamCount)
{
    if (Main_Init_Ready == 0)
        return 0;
    
    if (Main_Init_Ready == 1)
        DEBUG_UI("cam %d uiMenuSet_REC_MODE_BY_CH %d\r\n",nCamCount, setting);
    else
        DEBUG_MAIN("cam %d uiMenuSet_REC_MODE_BY_CH %d\r\n",nCamCount, setting);

    GMotionTrigger[nCamCount+MULTI_CHANNEL_LOCAL_MAX] = 0;

    #if UI_BAT_SUPPORT
    if (uiCurRecStatus[nCamCount] != UI_REC_TYPE_PIR)
        uiCaptureVideoStopByChannel(nCamCount);
    #endif
    
    if(setting > UI_MENU_REC_MODE_SCHEDULE)
    {
        iconflag[UI_MENU_SETIDX_REC_MODE_CH1+nCamCount] = defaultvalue[UI_MENU_SETIDX_REC_MODE_CH1+nCamCount];
        uiMenuSet_REC_MODE_BY_CH(iconflag[UI_MENU_SETIDX_REC_MODE_CH1+nCamCount],(UI_MENU_SETIDX_REC_MODE_CH1+nCamCount));
        return 1;
    }
    else if (setting == UI_MENU_REC_MODE_MANUAL)
    {
        iconflag[UI_MENU_SETIDX_REC_MODE_CH1+nCamCount] = UI_MENU_REC_MODE_MOTION;
        uiMenuSet_REC_MODE_BY_CH(iconflag[UI_MENU_SETIDX_REC_MODE_CH1+nCamCount],(UI_MENU_SETIDX_REC_MODE_CH1+nCamCount));
        return 1;
    }

    #if UI_BAT_SUPPORT
    if (_uiBatType[nCamCount] == UI_CAM_BATTERY)
        return 1;
    #endif
    
    if (setting == UI_MENU_REC_MODE_MOTION)
    {
        #if MULTI_CHANNEL_VIDEO_REC
            #if UI_BAT_SUPPORT
            if (_uiBatType[nCamCount] == UI_CAM_NORMAL)
            #endif
            {
                if (uiCurRecStatus[nCamCount] != UI_REC_TYPE_MANUAL)
                {
                    VideoClipParameter[nCamCount + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode  &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                    VideoClipParameter[nCamCount + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode  |= ASF_CAPTURE_EVENT_MOTION_ENA;
                    uiCurRecStatus[nCamCount] = UI_REC_TYPE_MOTION;
                }
            }
        #endif
    }
    else
    {
        if(setting == UI_MENU_REC_MODE_SCHEDULE)
            uiCurRecStatus[nCamCount] = UI_REC_TYPE_SCHEDULE;

        #if MULTI_CHANNEL_VIDEO_REC
            VideoClipParameter[nCamCount + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode  &= ~ASF_CAPTURE_PIRTIGGER_ENA;
            VideoClipParameter[nCamCount + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode  &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
        #endif
    }
}


s32 uiMenuSet_TX_VideoBrightness(s8 setting, u8 nCamCount)
{
    u32 volVal[6] = { 0, 2, 4, 5, 6, 8};
    u8 setcmdstr[20];

    //DEBUG_UI("cam %d uiMenuSet_TX_VideoBrightness %d\r\n",nCamCount,setting);

    if (uiSetDefault == 1)
        return 1;

    if (Main_Init_Ready == 0)
        return 1;

    if ((setting > 5) || (setting < 0))
    {
        DEBUG_UI("uiMenuSet_TX_VideoBrightness %d Fail\r\n",setting);
        return 1;
    }
#if RFIU_SUPPORT
    sprintf((char *)setcmdstr,"BRIT %d", volVal[setting]);
    uiSetRfBrightnessRxToTx(volVal[setting],nCamCount);
#endif
    return 1;
}

s32 uiMenuSet_TX_CameraOnOff(s8 setting, u8 nCamCount)
{
    static u8 Cur_val=0;
    u8 setcmdstr[20];
    u8 val=0, i;
    u8 CamCnt=0;
    
    if (Main_Init_Ready == 1)
        DEBUG_UI("cam %d uiMenuSet_TX_CameraOnOff %d\r\n",nCamCount,setting);
    else
        DEBUG_MAIN("cam %d uiMenuSet_TX_CameraOnOff %d\r\n",nCamCount,setting);

    //if (start_iconflag[UI_MENU_SETIDX_CH1_ON+nCamCount] == setting)
        //return 1;

    for (i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
        {
            val |= (1<<i);  
            CamCnt++;
        }       
        else
            val &= ~(1<<i);
    }
    
    SysCamOnOffFlag = val;

    if (CamCnt > 2)
        uiQuadDisplay = 1;
    else if (CamCnt == 1)
        uiQuadDisplay = 0;
    else
        uiQuadDisplay = 2;

    if (Main_Init_Ready == 0)
    {
        rfiuRX_CamOnOff_Sta = val;
        rfiuRX_CamOnOff_Num = CamCnt;
        Cur_val = val;
        return 1;
    }
    else
    {
        if (Cur_val != val)
        {
            for (i = 0; i < MULTI_CHANNEL_MAX; i++)
                uiCaptureVideoStopByChannel(i);

            Cur_val = val;
            sprintf((char *)setcmdstr,"%d", val);
            rfiuCamOnOffCmd(setcmdstr);
        }
    }
    return 1;

}

s32 uiMenuSet_TX_MOTION(s8 setting,int Day_Level,int Night_Level)
{
}

s32 uiMenuSet_TX_BRIGHTNESS(s8 setting)
{

}


#if NIC_SUPPORT
s32 uiMenuSet_P2P_Password(void)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_P2P_Password %s \r\n",UI_P2P_PSW);
    else
        DEBUG_MAIN("uiMenuSet_P2P_Password %s \r\n",UI_P2P_PSW);
    uiSetP2PPassword(UI_P2P_PSW);
    return 1;
}
#endif


/*

Routine Description:

	Set video size.

Arguments:

	setting - Setting.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_VideoSize(s8 setting)
{     //Lucian: ¥Ø«e¶È¤ä´©D1,VGA resolution.
    u16 width, height;
            
    switch (setting)
    {   /*CY 0907*/
        case UI_MENU_VIDEO_SIZE_720x480:
            width = 720;
            if(sysTVinFormat  == TV_IN_PAL )
               height = 576;
            else
               height = 480;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_720x480;
            break;

        case UI_MENU_VIDEO_SIZE_704x480:
            width = 704;
            if(sysTVinFormat  == TV_IN_PAL )
               height = 576;
            else
               height = 480;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_704x480;
            break;

        case UI_MENU_VIDEO_SIZE_720x576:
            width = 720;
            if(sysTVinFormat  == TV_IN_PAL )
               height = 576;
            else
               height = 480;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_720x576;
            break;

        case UI_MENU_VIDEO_SIZE_704x576:
            width = 704;
            if(sysTVinFormat  == TV_IN_PAL )
               height = 576;
            else
               height = 480;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_704x576;
            break;
            
        case UI_MENU_VIDEO_SIZE_320x240:
            width   = 320;
            height  = 240;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_320x240;
            break;

        case UI_MENU_VIDEO_SIZE_352x240:
            width   = 352;
            height  = 240;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_352x240;
            break;    

        case UI_MENU_VIDEO_SIZE_1280X720:
            width   = 1280;
            height  = 720;
            uiMenuVideoSizeSetting = UI_MENU_VIDEO_SIZE_1280X720;
            break;

        case UI_MENU_VIDEO_SIZE_640x480:    
            width   = 640;
            height = 480;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_640x480;
            break;

        case UI_MENU_VIDEO_SIZE_1920x1072:
            width  = 1920;
            height = 1088;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_1920x1072;
            break;

        default:
            width   = 640;
            height  = 480;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_640x480;
            break;
    }       
    
    siuSetVideoResolution(width, height);
#if (MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_MP4)     
    mp4SetVideoResolution(width, height);
#elif (MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_ASF) 
    asfSetVideoResolution(width, height);
#elif (MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_AVI)   /*Peter 0704 S*/    
    aviSetVideoResolution(width, height);
#endif                          /*Peter 0704 E*/

    return 1;
}

/* Setting Microphone gain */
s32 uiMenuSet_MIC_Audio_Vol(s8 setting)

{
    if (Main_Init_Ready == 1)
        DEBUG_UI("UI_MENU_SETIDX_MIC_VOLUME %d \n",setting);
    else
        DEBUG_MAIN("UI_MENU_SETIDX_MIC_VOLUME %d \n",setting);
    adcSetADC_MICIN_PGA_Gain(setting);
    
    return 1;
}

s32 uiMenuSet_PIR(s8 setting, u8 nCamCount)
{
    //DEBUG_GREEN("uiMenuSet_PIR %d Camera %d\r\n", setting, nCamCount);
    
    /*RF not connect*/
    if (Main_Init_Ready == 0)
        return 0;

#if(UI_BAT_SUPPORT)
    if (_uiBatType[nCamCount] == UI_CAM_NORMAL)
    {
        uiSetRfPIRRxToTx(1, nCamCount);
        DEBUG_UI("cam %d uiMenuSet_PIR %d\n",nCamCount, 1);
    }
    else 
    {
        uiSetRfPIRRxToTx(setting, nCamCount);
        DEBUG_UI("cam %d uiMenuSet_PIR %d\n",nCamCount, setting);
    }
#else
    uiSetRfPIRRxToTx(setting, nCamCount);
#endif

    return 1;
}

void uiMenuSet_TimestampType(u8 setting)
{
    u8  i;

    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_TimestampType %d \r\n",setting);
    else
    {
        DEBUG_UI("uiMenuSet_TimestampType %d \r\n",setting);
        return;
    }

    for (i = 0; i < MULTI_CHANNEL_MAX; i++)
      uiSetTxTimeStamp[i] =  uiSetRfTimeStampTypeRxToTx(setting, i);
}

s32 uiMenuSet_Motion_Section(u8 setting)
{
    u8 i;
    u32 time_len=15;

#if 0    
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Motion_Section %d\r\n",setting);
    else
        DEBUG_UI("uiMenuSet_Motion_Section %d\r\n",setting);
#endif

    if (Main_Init_Ready == 1)
        uiCaptureVideoStop();
    
    switch (setting)
    {
        case UI_MENU_ALERT_PERIOD_15s:
            time_len = 15;
            break;

        case UI_MENU_ALERT_PERIOD_30s:
            time_len = 30;
            break;

        case UI_MENU_ALERT_PERIOD_60s:
            time_len = 60;
            break;

        default:
            DEBUG_UI("Error uiMenuSet_Motion_Section %d\r\n",setting);
            iconflag[UI_MENU_SETIDX_MOTION_SECTION] = defaultvalue[UI_MENU_SETIDX_MOTION_SECTION];
            uiMenuSet_Motion_Section(iconflag[UI_MENU_SETIDX_MOTION_SECTION]);            
            break;
            
    }
    
    rfiuBatCam_PIRRecDurationTime = time_len;
    asfEventExtendTime = time_len;

    #if MULTI_CHANNEL_VIDEO_REC
    for(i=0; i<MULTI_CHANNEL_MAX; i++)
    {
        VideoClipParameter[i].asfRecTimeLen=time_len;
    }
    #endif

    return 1;
}

#if SET_NTPTIME_TO_RTC
s32 uiMenuSet_Timezone(u8 setting)
{
    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_Timezone %d %d\r\n",setting, uiSetDefault);
    else
        DEBUG_MAIN("uiMenuSet_Timezone %d %d\r\n",setting, uiSetDefault);
    /*the setting value = 1 means the datetime is set by user not by save value*/
    if (Main_Init_Ready == 1)
        uiCaptureVideoStop();
    if (uiSetDefault == 0)
    {
        DEBUG_UI("Set Time Zone %d %d:%d\r\n",SetZone.operator, SetZone.hour, SetZone.min);
        RTC_Set_TimeZone(&SetZone);
    }
    return 1;
}
#endif

#if USB_HOST_MASS_SUPPORT
s32 uiMenuSet_HDDRemove(u8 setting)
{
    if (Main_Init_Ready == 0)
        return 0;

    switch (setting)
    {
        case UI_MENU_DEFAULT_YES:
            removeHDDStatus = FALSE;
    		iconflag[UI_MENU_SETIDX_HDD_REMOVE] = UI_MENU_DEFAULT_NO; /* Reset default to "No" */

            if (sysGetStorageSel(SYS_I_STORAGE_MAIN) != SYS_V_STORAGE_USBMASS)// Insert USB
            {
                DEBUG_UI("Sroage is not HDD\r\n");
                return 0;
            }

            uiCaptureVideoStop();
            removeHDDStatus = usb_stop_remove();
            DEBUG_UI("%d %s %d\n",__LINE__, __FILE__,removeHDDStatus);            
            break;

        default:
            break;
    }
    return 1;

}
#endif

s32 uiMenuAction(s8 setidx)
{
    switch (setidx)
    {
#if SET_NTPTIME_TO_RTC
        case UI_MENU_SETIDX_NTP:
            uiMenuSet_Timezone(iconflag[setidx]);
            break;
#endif

#if USB_HOST_MASS_SUPPORT
        case UI_MENU_SETIDX_HDD_REMOVE:
            uiMenuSet_HDDRemove(iconflag[setidx]);
            break;
#endif

        case UI_MENU_SETIDX_VIDEO_QUALITY:
            uiMenuSet_VideoQuality(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_VIDEO_FRAMERATE:
            uiMenuSet_VideoFrameRate(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_RESOLUTION_CH1:          
        case UI_MENU_SETIDX_RESOLUTION_CH2:
        case UI_MENU_SETIDX_RESOLUTION_CH3:
        case UI_MENU_SETIDX_RESOLUTION_CH4:
            uiMenuSet_TX_CameraResolution(iconflag[setidx],(setidx-UI_MENU_SETIDX_RESOLUTION_CH1));
            break;
            
        case UI_MENU_SETIDX_CH1_ON:
        case UI_MENU_SETIDX_CH2_ON:
        case UI_MENU_SETIDX_CH3_ON:
        case UI_MENU_SETIDX_CH4_ON:
            uiMenuSet_TX_CameraOnOff(iconflag[setidx],(setidx-UI_MENU_SETIDX_CH1_ON));
            break;
            
        case UI_MENU_SETIDX_BRIGHTNESS_CH1:
        case UI_MENU_SETIDX_BRIGHTNESS_CH2:
        case UI_MENU_SETIDX_BRIGHTNESS_CH3:
        case UI_MENU_SETIDX_BRIGHTNESS_CH4:
            uiMenuSet_TX_VideoBrightness(iconflag[setidx], (setidx-UI_MENU_SETIDX_BRIGHTNESS_CH1));
            break;

        case UI_MENU_SETIDX_REC_MODE_CH1:
        case UI_MENU_SETIDX_REC_MODE_CH2:
        case UI_MENU_SETIDX_REC_MODE_CH3:
        case UI_MENU_SETIDX_REC_MODE_CH4:
            uiMenuSet_REC_MODE_BY_CH(iconflag[setidx],(setidx-UI_MENU_SETIDX_REC_MODE_CH1));
            break;

        case UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1:
        case UI_MENU_SETIDX_MOTION_SENSITIVITY_CH2:
        case UI_MENU_SETIDX_MOTION_SENSITIVITY_CH3:
        case UI_MENU_SETIDX_MOTION_SENSITIVITY_CH4:
            uiMenuSet_MotionSensitivity(iconflag[setidx],(setidx-UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1));
            break;
        
        case UI_MENU_SETIDX_OVERWRITE:
            uiMenuSet_Overwrite(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_SECTION:
            uiMenuSet_Section(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_MOTION_MASK:
            uiMenuSet_MotionMask();
            break;        

        case UI_MENU_SETIDX_DATE_TIME:
            uiMenuSet_DateTime(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_FORMAT:
            uiMenuSet_Format(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_DISPLAY:
            uiMenuSet_Display(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_VOLUME:
            uiMenuSet_Audio_Vol(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_ALARM:
            uiMenuSet_Alarm(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_LANGUAGE:
            uiMenuSet_Language(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_DEFAULT:
            uiMenuSet_Default(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_50HZ_60HZ:
            uiMenuSet_Flicker(iconflag[setidx]);
            break;
            
    #if(TUTK_SUPPORT)
        case UI_MENU_SETIDX_P2PID:
            uiMenuSet_P2PID(uiP2PID);
            break;
    #endif

    #if(RFIU_SUPPORT)
        case UI_MENU_SETIDX_RFID:
            uiMenuSet_RFID(uiRFID);
            break;
        case UI_MENU_SETIDX_RFID_CODE:
            uiMenuSet_RFID_CODE(uiRFCODE);
            break;
            
    #endif

    #if(NIC_SUPPORT)
        case UI_MENU_SETIDX_MAC:
            uiMenuSet_MACAddr(uiMACAddr);
            break;
            
        case UI_MENU_SETIDX_NETWORK_STATUS:
            uiMenuSet_Dynamic_ON(iconflag[setidx]);
            break;
            
        case UI_MENU_SETIDX_ST_IP_SET:
            uiMenuSet_Network();
            break;
            
    #endif

        case UI_MENU_SETIDX_TV_OUT:
            uiMenuSet_TVout_Format(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_UPGRADE_FW:
            uiMenuSet_UpgradeFW(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_CH1_PIR:
        case UI_MENU_SETIDX_CH2_PIR:
        case UI_MENU_SETIDX_CH3_PIR:
        case UI_MENU_SETIDX_CH4_PIR:
            #if(UI_BAT_SUPPORT)
            uiMenuSet_PIR(gUiPIRSch[setidx-UI_MENU_SETIDX_CH1_PIR], (setidx-UI_MENU_SETIDX_CH1_PIR));
            #else
            uiMenuSet_PIR(UI_PIR_ON, (setidx-UI_MENU_SETIDX_CH1_PIR));
            #endif
            
            break;

    #if NIC_SUPPORT
        case UI_MENU_SETIDX_P2P_PASSWORD:
            uiMenuSet_P2P_Password();
            break;

        case UI_MENU_SETIDX_UPGRADE_FW_NET:
            if(Reminder_FW_Upgrade == 0)
            {
                DEBUG_UI("FW is last Version\n");
                break;
            }
            uiMenuSet_UpgradeNet(iconflag[setidx]);
    #endif

#if UI_LIGHT_SUPPORT
        case UI_MENU_SETIDX_CH1_LS_TIMER:
        case UI_MENU_SETIDX_CH2_LS_TIMER:
        case UI_MENU_SETIDX_CH3_LS_TIMER:
        case UI_MENU_SETIDX_CH4_LS_TIMER:
            uiMenuSet_LS_Timer(iconflag[setidx], (setidx-UI_MENU_SETIDX_CH1_LS_TIMER));
            break;

        case UI_MENU_SETIDX_CH1_LS_STATUS:
        case UI_MENU_SETIDX_CH2_LS_STATUS:
        case UI_MENU_SETIDX_CH3_LS_STATUS:
        case UI_MENU_SETIDX_CH4_LS_STATUS:
            uiMenuSet_LS_Status(iconflag[setidx], (setidx-UI_MENU_SETIDX_CH1_LS_STATUS));
            break;

        case UI_MENU_SETIDX_CH1_LS_ONOFF:
        case UI_MENU_SETIDX_CH2_LS_ONOFF:
        case UI_MENU_SETIDX_CH3_LS_ONOFF:
        case UI_MENU_SETIDX_CH4_LS_ONOFF:
            uiMenuSet_LS_ONOFF(iconflag[setidx], (setidx-UI_MENU_SETIDX_CH1_LS_ONOFF));
            break;
            
#endif
#if UI_CAMERA_ALARM_SUPPORT
        case UI_MENU_SETIDX_CH1_CA_TIMER:
        case UI_MENU_SETIDX_CH2_CA_TIMER:
        case UI_MENU_SETIDX_CH3_CA_TIMER:
        case UI_MENU_SETIDX_CH4_CA_TIMER:
            uiMenuSet_CameraAlarm_Timer(iconflag[setidx], (setidx-UI_MENU_SETIDX_CH1_CA_TIMER));
            break;

        case UI_MENU_SETIDX_CH1_CA_STATUS:
        case UI_MENU_SETIDX_CH2_CA_STATUS:
        case UI_MENU_SETIDX_CH3_CA_STATUS:
        case UI_MENU_SETIDX_CH4_CA_STATUS:
            uiMenuSet_CameraAlarm_Status(iconflag[setidx], (setidx-UI_MENU_SETIDX_CH1_CA_STATUS));
            break;

        case UI_MENU_SETIDX_CH1_CA_ONOFF:
        case UI_MENU_SETIDX_CH2_CA_ONOFF:
        case UI_MENU_SETIDX_CH3_CA_ONOFF:
        case UI_MENU_SETIDX_CH4_CA_ONOFF:
            uiMenuSet_CA_ONOFF(iconflag[setidx], (setidx-UI_MENU_SETIDX_CH1_CA_ONOFF));
            break;

#endif

        case UI_MENU_SETIDX_TIMESTAMP:
            #if (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_YMD)
            uiMenuSet_TimestampType(UI_TIME_STAMP_TYPE_24_YMD);
            #elif (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_MDY)
            uiMenuSet_TimestampType(UI_TIME_STAMP_TYPE_24_MDY);
            #elif (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_MDY_APM)
            uiMenuSet_TimestampType(UI_TIME_STAMP_TYPE_12_MDY);
            #elif (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_YMD_APM)
            uiMenuSet_TimestampType(UI_TIME_STAMP_TYPE_12_YMD);
            #elif (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_DMY)
            uiMenuSet_TimestampType(UI_TIME_STAMP_TYPE_24_DMY);
            #endif
            break;
            
        case UI_MENU_SETIDX_MOTION_SECTION:
            uiMenuSet_Motion_Section(iconflag[setidx]);
            break;

        default:
            return 0;
    }
    return 1;
}

s32 uiSetSnapshot(void)
{
	if (sysCameraMode != SYS_CAMERA_MODE_PREVIEW)
		return 0;

	sysSetEvt(SYS_EVT_SNAPSHOT, 0);
 	return 1;
}

