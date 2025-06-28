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

#if (HW_BOARD_OPTION == MR9670_WOAN)
	UI_MULT_LANU_EN = 0,	/*English*/	
	UI_MULT_LANU_ES,		/*Spanish*/
	UI_MULT_LANU_FR,		/*French*/
	UI_MULT_LANU_IT,		/*Italian*/
	UI_MULT_LANU_PT,		/*Portuguese*/
	UI_MULT_LANU_NL,		/*Holland*/
	UI_MULT_LANU_RU,		/*Russian*/
	UI_MULT_LANU_DE,		/*German*/
#elif ((HW_BOARD_OPTION == MR8200_RX_JIT) ||(HW_BOARD_OPTION == MR8200_RX_JIT_BOX)||(HW_BOARD_OPTION == MR8200_RX_JIT_BOX_AV)||\
       (HW_BOARD_OPTION == MR8120_RX_JIT_LCD)||(HW_BOARD_OPTION == MR8120_RX_JIT_BOX) || (HW_BOARD_OPTION == MR8120_RX_JIT_D808SW3) ||\
       (HW_BOARD_OPTION  == MR8120_RX_JIT_M703SW4) || (HW_BOARD_OPTION == MR8200_RX_JIT_D808SN4) || (HW_BOARD_OPTION  == MR8200_RX_JIT_M703SN4))
    UI_MULT_LANU_EN = 0,    /*English*/ 
    UI_MULT_LANU_FR,        /*French*/
    UI_MULT_LANU_IT,        /*Italian*/
    UI_MULT_LANU_ES,        /*Spanish*/
#else
	UI_MULT_LANU_EN = 0,
	UI_MULT_LANU_TC,    /*Traditional Chinese*/
	UI_MULT_LANU_SC,    /*Simple Chinese*/
#endif
    
	UI_MULT_LANU_END
} MLT_LANGUAGE;

#define UI_SET_FR_CMD_RETRY         10
#if(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902)
#define UI_MENU_TO_PRV              9
#else
#define UI_MENU_TO_PRV              31
#endif

extern void uiGraphGetTimePhotoID(void);
extern void uiOsdDrawMaskArea(u8 key);
extern void uiOsdDrawCardInfo(void);
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

extern void uiSetOutputMode(u8 ucMode);
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


extern u8 showTime;
extern s8 defaultvalue[];
extern RTC_DATE_TIME SetTime;
extern RTC_TIME_ZONE SetZone;
extern u8 MotionMaskArea[MASKAREA_MAX_ROW][MASKAREA_MAX_COLUMN];
extern u8 StartMotMask[MASKAREA_MAX_ROW][MASKAREA_MAX_COLUMN];
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

#endif

