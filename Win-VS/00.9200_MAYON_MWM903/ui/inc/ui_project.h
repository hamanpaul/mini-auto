/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ui.h

Abstract:

   	The declarations of user interface.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/04/29	JJ Huang	Create	

*/

#ifndef __UI_PROJECT_H__
#define __UI_PROJECT_H__

#include "motiondetect_api.h"
#include "uiact_project.h"
#include "rtcapi.h"


typedef enum Mult_Language
{
    UI_MULT_LANU_EN = 0,    /*English*/ 
    UI_MULT_LANU_FR,        /*French*/
    UI_MULT_LANU_IT,        /*Italian*/
	UI_MULT_LANU_END
	
} MLT_LANGUAGE;

enum
{
    UI_LIGHT_OFF = 0,
    UI_LIGHT_MANUAL_OFF,
    UI_LIGHT_TIMER_ON,
    UI_LIGHT_MANUAL_ON,
    UI_LIGHT_TRIGGER_ON
};

enum
{
    UI_CAMERA_ALARM_OFF = 0,
    UI_CAMERA_ALARM_MANUAL_OFF,
    UI_CAMERA_ALARM_TIMER_ON,
    UI_CAMERA_ALARM_MANUAL_ON,
    UI_CAMERA_ALARM_TRIGGER_ON
};

enum 
{
    UI_CAM_NORMAL = 0,    
    UI_CAM_BATTERY,       
};

enum 
{
    UI_TX_LIGHT_MANUAL_OFF = 0,       
    UI_TX_LIGHT_MANUAL_ON, 
    UI_TX_LIGHT_FUNC_ON, 
    UI_TX_LIGHT_FUNC_OFF,
    UI_TX_ALARM_MANUAL_OFF,       
    UI_TX_ALARM_MANUAL_ON,
    UI_TX_ALARM_FUNC_ON, 
    UI_TX_ALARM_FUNC_OFF,
};

#define UI_SET_FR_CMD_RETRY         10
#define UI_MENU_TO_PRV              30
#define UI_MENU_MAX_COLUMN          4
#define UI_TOUCH_ACT_DELAY          10



extern void uiGraphGetTimePhotoID(void);
extern void uiOsdDrawMaskArea(u8 key);
extern void uiOsdDrawCardInfo(u8 on);
extern u8 uiCmdPareCmd(u8* cmd);
extern void uiOsdDrawPlaybackMenu(u8 key);
extern void uiOsdDrawPlaybackPlaySpeed(void);
extern void uiOsdDrawPlayTime(u8 type, u32 time_unit);
extern void uiOsdVolumeControl(u8 mode, UI_VALUECTRL value);
extern void uiGraphDrawNetwork(u8 key);
extern void uiGraphDrawScheduled(u8 key);
extern void uiGraphGetNetworkInfo(u8* ip, u8* submask, u8* defaultGateway);
extern void uiOsdDrawLifeTimePerSec(void);
extern void osdDrawRunFormat(void);
extern void uiGraphDrawDateTime(u8 key);
extern void uiFlowRedirection(void);
extern void uiOsdEnterOsdMenuMode(void);
extern void uiOsdDrawMenu(void);
#if (NIC_SUPPORT == 1)
extern void uiOsdDrawIPInfo(u8 Mode);
#endif
extern void osdDrawQuadIcon(void);

#if(HW_BOARD_OPTION == MR6730_AFN)
extern void uiOsdDrawPowerOff(void);
extern void uiOsdDrawPlaybackMenuDeleteMsg(u8 act);

//#if (DERIVATIVE_MODEL==MODEL_TYPE_PUSH)
#if(USE_PLAYBACK_AUTONEXT)
void uiOsdClearVideoPlayTime(u8 IsDraw);
#endif
#endif
extern void uiOsdDrawSetting(u8 On);
extern void uiOsdDrawTimeFrame(u8 select , u8 clean);
extern void uiFrowGoToLastNode(void);
extern void osdDrawSystemReboot(void);
extern void uiOsdDrawScheduledFrame(u8 setCursor,u8 clean);
extern void uiOsdDrawFWVersion(u8 On);
extern void GetNetworkInfo(struct NetworkInfo *info);
extern void uiDrawNetworkInfo(UI_NET_INFO *NetInfo, u8 on);
extern s32  uiMenuSet_TX_CameraOnOff(s8 setting, u8 nCamCount);
extern s8   uiMenuSet_TX_CameraResolution(s8 setting,u8 camera);
extern s32  uiMenuSet_TX_VideoBrightness(s8 setting, u8 nCamCount);
extern void uiGraphDrawRECMode(u8 key);
extern void uiGraphDrawMotionSensitivity(u8 key);
extern void uiGraphDrawCameraOnOff(u8 key);
extern void uiGraphDrawResolution(u8 key);
extern void uiGraphDrawBrightness(u8 key);
extern void uiGraphDrawNetworkInfo(u8 key);
extern void uiGraphDrawKeypad(u8 key);
extern u8 uiGraphDrawScheduledSetting(u8 key);
extern void uiGraphDrawCardInfo(u8 key);
extern void uiGraphDrawVersionInfo(u8 key);
extern void uiOsdDrawConfirmSelect(u8 key, u8 type);
extern void ClearP2PConnection(void);
extern s32 uiMenuSet_TVout_Format(u8 setting);
extern u8 uiOsdDrawPair(u8 Camera);
extern void uiGraphGetImageID(void);
extern void  uiOsdDrawQuadNoSignal(u8 act, u8 Camid);
extern void  uiOsdDrawChangeResolution(u8 act);
extern void osdDrawQuadVideoOn(u8 Camid, u8 act);
extern void uiOsdDrawTalkBack(u8 On);
extern void uiOsdDrawBlackAll(u8 act);
extern u8 uiOsdDrawPairInMenu(u8 camera);
extern void  osdDrawUpgradeFW(void);
extern void uiOsdDrawPlaybackMenuDoor(u8 key);
extern void uiOsdDrawRecPerSec(void);
extern void uiGraphDrawPlaybackbusy(void);
extern void uiFlowRunAction(void);
extern void uiOsdDrawAllPreviewIcon(void);
extern void uiFlowSetupToPreview(void);
extern void  uiOsdDrawRestoreDefaltSettings(u8 On);
extern void uiGraphDrawPlaybackMenu(u8 key);
    

extern u8 showTime;
extern s8 defaultvalue[];
extern RTC_DATE_TIME SetTime;
extern RTC_TIME_ZONE SetZone;
#if 1
extern u8 uiMaskArea[MULTI_CHANNEL_MAX][9];
extern u8 uiStartMaskArea[MULTI_CHANNEL_MAX][9];
#else
extern u8 MotionMaskArea[MASKAREA_MAX_ROW][MASKAREA_MAX_COLUMN];
extern u8 StartMotMask[MASKAREA_MAX_ROW][MASKAREA_MAX_COLUMN];
#endif
extern u16 OSDIconEndX,OSDIconEndY, OSDIconMidX,OSDIconMidY;
extern u16 PbkOSD1, PbkOSD2, PbkOSD1High, PbkOSD2High;
extern u8 PlayListDspType;
extern void (*OSDDisplay[])(u8, u32, u32, u32, u32);
extern UI_NET_INFO UINetInfo;
extern u8 volumeflag;
extern u8  uiQuadDisplay;
extern u8 uiSetDefault;
extern u8  uiRFStatue[MAX_RFIU_UNIT];
extern u32 uiPlayPWMFreg;
extern u32 playbackDir;
#if(SUPPORT_TOUCH)
extern s8  TouchExtKey;
extern u16 Touch_X;
extern u16 Touch_Y;
#endif
extern u8 uiReturnPreview;
extern u8 UI_P2P_PSW[UI_P2P_PSW_MAX_LEN];
extern u8 UI_Default_P2P_PSW[UI_P2P_PSW_MAX_LEN];
#if (HW_BOARD_OPTION == MR8211_ZINWELL)
extern  u16 Light;
extern  f32 TEMP;
extern  u8  IR_Mode;    // AVIOCTRL_NIGHT_ON, AVIOCTRL_NIGHT_OFF, AVIOCTRL_NIGHT_AUTO
extern  u8  Light_Status; //0x01: OFF, 0x02: Random, 0x03: value assigned
extern  u8  Light_R;
extern  u8  Light_G;
extern  u8  Light_B;
extern  u8  Light_L;
extern  u8  RecordMode;     // ENUM_RECORD_TYPE
extern  u8  MountMode;      // ENUM_SDCARDMUM_MODE
extern  u8  NoiseAlert;     // 0x01: ON,  0x02:OFF 
extern  u8  TempAlert;      // 0x01: ON,  0x02:OFF 
extern  f32 TempHighMargin;
extern  f32 TempLowMargin;
extern  f32 StartTempHighMargin;
extern  f32 StartTempLowMargin;
extern  u8  MDSensitivity;  // UI_MENU_SETTING_MOTION_SENSITIVITY_H, UI_MENU_SETTING_MOTION_SENSITIVITY_M, UI_MENU_SETTING_MOTION_SENSITIVITY_L
#endif
#if (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
extern u8  uiLightTimer1[4];    /*10:11~12:14*/
extern u8  uiLightTimer2[4];    /*10:11~12:14*/
extern u8  uiLightWeek1[7];     /*1: on, 0:off*/
extern u8  uiLightWeek2[7];     /*1: on, 0:off*/
#endif
extern u8  uiScheduleTime[7][4][48];
extern u8 uiSetCfg;

#if UI_LIGHT_SUPPORT
#if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
extern u8  uiLightTimer[MULTI_CHANNEL_MAX][4];
#elif (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
extern u8 uiLightInterval[MULTI_CHANNEL_MAX][7][6];
#endif
#endif
extern u8 uiLightTest;

#if UI_CAMERA_ALARM_SUPPORT
extern u8  uiCamAlarmWeek[MULTI_CHANNEL_MAX];
extern u8  uiCamAlarmInterval[MULTI_CHANNEL_MAX][7][6]; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */
#endif

extern int UiGetTouchX, UiGetTouchY;
extern u8  uiCurRecStatus[4];
extern u8 isManualRec;
extern u8 isMotionRec;
extern u8 uiConTouchPress;
#if (NIC_SUPPORT == 1)
extern u8 Fileplaying;
extern u8 Remote_play;
#endif
extern u8  uiSetTxTimeStamp[MULTI_CHANNEL_MAX];

extern u32 totalPlaybackFileNumADay;
extern u8 uiShowOSDTime[MULTI_CHANNEL_MAX];

#if(UI_BAT_SUPPORT)
extern u8  _uiBatType[4];//0 battery 1 normal
#endif

#endif

