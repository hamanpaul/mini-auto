/*

  Copyright (c) 2008 Mars Semiconductor Corp.

   Module Name:

   ui_flow_project.c

   Abstract:

   The routines of user interface.

   Environment:

   ARM RealView Developer Suite

   Revision History:

   2010/12/27   Elsa Lee  Create

   */
#include "general.h"
#include "uiapi.h"
#include "ui.h"
#include "sysapi.h"
#include "usbapi.h"
#include "ui_project.h"
#include "siuapi.h"
#include "isuapi.h"
#include "asfapi.h"
#include "uartapi.h"
#include "dcfapi.h"
#include "board.h"
#include "aviapi.h"
#include "timerapi.h"
#include "ipuapi.h"
#include "spiapi.h"
#include "osd_draw_project.h"
#include "gpioapi.h"
#include "iisapi.h"
#if(CHIP_OPTION >= CHIP_A1013A)
#include "rfiuapi.h"
#endif

#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
#if (HW_BOARD_OPTION==MR6730_FINEDIGITAL_LCD)
#include "i2capi.h"
#endif


#if(UART_GPS_COMMAND == 1)
#include "gpsapi.h"
#endif
#if (HW_BOARD_OPTION == MR6730_WINEYE)
#include "MotionDetect_API.h"
#endif

#if (HW_BOARD_OPTION == MR9670_COMMAX) || (HW_BOARD_OPTION == MR9670_COMMAX_WI2)
    #include "MainFlow.h"
#endif
#include "P2pserver_api.h"
#include "numeric.h"

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */
#define UI_SET_MD_TIME          30
#define UI_MD_NOT_REC           UI_SET_MD_TIME+1
#define UI_TALK_DLY_TIME        20
#define UI_RETRY_REC_TIME       10
#define UI_RETRY_REC_NONE       (UI_RETRY_REC_TIME+1)
#define UI_SET_CLOSE_PANEL_TIME 50
#define UI_CHECK_NETWORK_TIME   60
/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
static u8 GS[23]={0};
UI_HANDLER MyHandler;
u8 PlayListDspType = UI_DSP_PLAY_LIST_FILE;
u8 osdYShift = 0;      /*for NTSC osdYShift = 0, PAL osdYShift = 76*/
u8 TvOutMode = 0;       /*NTSC*/
#if 1
u8  uiStartMaskArea[MULTI_CHANNEL_MAX][9] = {0};
u8  uiMaskArea[MULTI_CHANNEL_MAX][9] = {0};
#else
u8 MotionMaskArea[MASKAREA_MAX_ROW][MASKAREA_MAX_COLUMN]={0};
u8 StartMotMask[MASKAREA_MAX_ROW][MASKAREA_MAX_COLUMN] = {0};
#endif
u8 showTime = 0;
u8 uiStartP2PID[P2PID_LENGTH]={0};
u8 uiP2PID[P2PID_LENGTH];
u16 OSDIconEndX,OSDIconEndY, OSDIconMidX,OSDIconMidY;
u16 PbkOSD1, PbkOSD2, PbkOSD1High, PbkOSD2High;
u32 TV_X,TV_Y;
u32 Display_X, Display_Y;
u32 uiStartRFID[RFID_MAX_WORD]={0};
u32 uiStratRFCode[RFID_MAX_WORD]={0};
u32 uiRFID[RFID_MAX_WORD];
u32 uiRFCODE[RFID_MAX_WORD];
u8  uiReturnPreview = 0;
u8  uiStartMACAddr[MAC_LENGTH]={0};
u8  uiMACAddr[MAC_LENGTH];
s8  OverwriteStringEnable   = 1;
s8 gsParseDirName[9]="MFG";
RTC_DATE_TIME ScheduledTimeFrom = { 9, 1, 1, 0, 0, 0 }, ScheduledTimeTo = { 9, 2, 1, 0, 0, 0 };
RTC_DATE_TIME StartSchTimeFrom = { 9, 1, 1, 0, 0, 0 }, StartSchTimeTo = { 9, 2, 1, 0, 0, 0 };
RTC_DATE_TIME SetTime;
RTC_TIME_ZONE SetZone;
RTC_TIME_ZONE StartTimeZone;
RTC_TIME_ZONE UIDefaultTimeZone = {0,8,0};
#if (NIC_SUPPORT)
UI_NET_INFO UINetInfo;
UI_NET_INFO StartUINetInfo;
#endif
u8  UICheckSchedule = 48;      /*Per day check week,reset sch*/
u32 UICheckScheduleMin = 0;   /*Per 30 Min check time*/
u32 UICheckScheduleGetTime = 10;   /*Per 10 Sec Get RTC time*/

#if ISU_OVERLAY_ENABLE
u8  szVideoOverlay1[MAX_OVERLAYSTR];
u8  szVideoOverlay2[MAX_OVERLAYSTR];
u8  szLogString[MAX_OVERLAYSTR];
#endif
u8  uiMotionSecTime[4] = {UI_MD_NOT_REC, UI_MD_NOT_REC, UI_MD_NOT_REC, UI_MD_NOT_REC};
u8 UI_Default_P2P_PSW[UI_P2P_PSW_MAX_LEN] = {"000000"};
u8  uiCurRecStatus[4]={UI_REC_TYPE_NONE,UI_REC_TYPE_NONE,UI_REC_TYPE_NONE,UI_REC_TYPE_NONE};
u32 playbackDir;
u8 ScanRf;
u8  lastMenuMode;
u8  uiDrawSDFail = UI_OSD_CLEAR;
u8  uiRetryRecTime = UI_RETRY_REC_NONE;

#if(SUPPORT_TOUCH)
s8 TouchExtKey = -1;
#endif

u8 gSchRecStop = 0;    /*schedule state */
u8  uiScheduleTime[7][4][48];
u8  uiStartScheduleTime[7][4][48]={0};
u8  uitempScheduleTime[7][4][6];

#if UI_LIGHT_SUPPORT
u8  uiLightTimer[MULTI_CHANNEL_MAX][4];    /*10:11~12:14*/
u8  uiStartLightTimer[MULTI_CHANNEL_MAX][4];    /*10:11~12:14*/
u8  uiLightWeek[MULTI_CHANNEL_MAX];
u8  uiLightInterval[MULTI_CHANNEL_MAX][7][6]={0}; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */
u8  uiStartLightInterval[MULTI_CHANNEL_MAX][7][6]={0}; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */

u8  uiSetRfLightSwitch[MULTI_CHANNEL_MAX] = {UI_SET_RF_BUSY};
u8  uiSetRfLSTimer[MULTI_CHANNEL_MAX][7] = {UI_SET_RF_BUSY};
#endif

#if UI_CAMERA_ALARM_SUPPORT
u8  uiCamAlarmInterval[MULTI_CHANNEL_MAX][7][6]={0}; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */
u8  uiStartCamAlarmInterval[MULTI_CHANNEL_MAX][7][6]={0}; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */

u8  uiSetRfAlarmSwitch[MULTI_CHANNEL_MAX] = {UI_SET_RF_BUSY};
u8  uiSetRfAlarmTimer[MULTI_CHANNEL_MAX][7] = {UI_SET_RF_BUSY};
u8  uiSetRfAlarmState[MULTI_CHANNEL_MAX] = {UI_SET_RF_BUSY};
#endif

int UiGetTouchX, UiGetTouchY;
u8  isManualRec=UI_OSD_CLEAR;// rec icon
u8  isMotionRec=UI_OSD_CLEAR;// motion icon
u8  uiConTouchPress = 0;
u8  uiShowOSDTime[MULTI_CHANNEL_MAX] = {0};// 1 link
u8  uiCheckPower = TRUE;//true:open
u8  uiCloPowSec = 0;
u8  openPower=FALSE;

#if UI_BAT_SUPPORT
u8  uiBatteryInterval[MULTI_CHANNEL_MAX][7][6]={0}; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */
u8  uiStartBatteryInterval[MULTI_CHANNEL_MAX][7][6]={0}; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */

u8  uiBatChangeMode=FALSE;
u8  _uiBatType[4]={UI_CAM_BATTERY,UI_CAM_BATTERY,UI_CAM_BATTERY,UI_CAM_BATTERY};
u8  _uiDoor=0;
u8  gUiPIRSch[4] = {1,1,1,1};
u8  _uiCheckBatterySch = 48;      /*Per day check week,reset sch*/
#endif

#if USB_HOST_MASS_SUPPORT
u8  gUiDeviceRemove = 0;      /*Per day check week,reset sch*/
#endif
u8 gUishowFailTime = 0;
u8 gUiParing = 0;
u8 gUiLeaveKaypad = 0;
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern u32 prev_P2pVideoQuality;
extern u8 uiMenuVideoSizeSetting;
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];
extern u8 uiIPAddr[4];
extern u8 uiSubnetMask[4];
extern u8 uiDefaultGateway[4];
extern u8 uiP2PMode;
extern  u32 *CiuOverlayImg1_Top;
extern  u32 *CiuOverlayImg1_Bot;
extern  u8  OSDTimestameLevel;
extern u8 UI_SDLastLevel;
extern u8 SD_detect_status; //sd status 1:OK 0 :not
extern u8  uiStartScheduleTime[7][4][48];
extern u8 uiFlowSetAlarm;
#if(SUPPORT_TOUCH)
extern bool touch_press;
#endif
extern SYS_CONFIG_SETTING sysConfig;
extern SYS_CONFIG_SETTING start_sysConfig;
extern BOOLEAN ShowLogoFinish;
#if MESUARE_BTCWAKEUP_TIME
extern u32 guiSysTimerCnt;
#endif
extern u8  uiLightTest;
#if USB_HOST_MASS_SUPPORT
extern u8 removeHDDStatus; /*0: Failes, 1: Successed*/
#endif
extern s8  isPIRsenSent[MAX_RFIU_UNIT];
extern u16 gSysSyncBTCTime;
extern u8 showPIRMsg;
/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */
extern void osdDrawRecPreview(u8 act);
extern void osdDrawMotionPreview(u8 act);
extern void uiOsdDrawNetworkLinkUp(void);
extern void uiOsdDrawSDCardFail(u8 act);
extern void osdDrawClearRemoteMsg(u8 act);
extern u8 sysGetBTCWakeStatus(u8 channel_id);

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
u8 uiCheckMotionByCamType(u8 camID);
#if UI_BAT_SUPPORT
void uiCheckBatterySchdule(void);
void uiCheckBTCSetting(u8 camID,u8 linkType);
void uiCheckBTCAlarm(u8 camID);
#endif
#if UI_CAMERA_ALARM_SUPPORT
u8 uiGetAlarmStatusAPP(u8 Ch_ID);
u8 uiSetAlarmStatusAPP(u8 ch, u8 status);
u8 uiGetAlarmOnOffAPP(u8 ch);
u8 uiSetAlarmOnOffAPP(u8 ch, u8 status);
u8 uiGetSuptAlarmAPP(u8 ch);
#endif
u8 uiGetMotionStatusAPP(u8 Ch_ID);
u8 uiSetMotionStatusAPP(u8 ch, u8 status);
#if UI_LIGHT_SUPPORT
u8 uiGetLightStatusAPP(u8 Ch_ID);
u8 uiSetLightStatusAPP(u8 ch, u8 status);
u8 uiGetLightOnOffAPP(u8 ch);
u8 uiSetLightOnOffAPP(u8 ch, u8 status);
u8 uiGetSuptLightAPP(u8 ch);
#endif    
/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */ 
 /*Act = 0: Rf Cmd
        1: UI
  Val = 0: Light off
        1: Trigger on
        2: Manual On
*/
#if (UI_LIGHT_SUPPORT)
void uiFlowSetLightStatus(u8 Camid, u8 Val, u8 Act)
{
    u8  err;
    static u8 checkCnt[MULTI_CHANNEL_MAX] = {0};

	if (uiLightTest == 1)
	    DEBUG_GREEN("%d %s %s cam %d val %d value %d\n",__LINE__, __FILE__,__FUNCTION__,Camid,Val,iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid]);

  	switch (Val)
    {
        case 2://manual on 
            if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] != UI_LIGHT_MANUAL_ON)
            {
                uiSetRfLightState[Camid] = uiSetRfManualLightingRxToTx(UI_TX_LIGHT_MANUAL_OFF, Camid);
            }
            checkCnt[Camid] = 0;
            return;

        case 1://trigger on / sch on 
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            if ((iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] == UI_LIGHT_OFF) || (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] == UI_LIGHT_MANUAL_OFF) || (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] == UI_LIGHT_MANUAL_ON))
            {
                iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] = UI_LIGHT_TRIGGER_ON;
                #if(NIC_SUPPORT)
                UpdateMAYONLightStatus(Camid);
                #endif
                Save_UI_Setting();
            }
            checkCnt[Camid] = 0;
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_CLR, &err);
            break;

        case 0://sch off / trigger off  
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] == UI_LIGHT_TRIGGER_ON)
            {
                iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] = UI_LIGHT_OFF;
                #if(NIC_SUPPORT)
                UpdateMAYONLightStatus(Camid);
                #endif
                Save_UI_Setting();
                checkCnt[Camid] = 0;
            }
            else if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] == UI_LIGHT_MANUAL_ON)
            {
            	if (uiLightTest == 1)
            		DEBUG_GREEN("@@@ CAM = %d,checkCnt = %d\n",Camid,checkCnt[Camid]);	

                if (checkCnt[Camid] == 0)
                    checkCnt[Camid]++;
                else
                {
                    iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] = UI_LIGHT_MANUAL_OFF;
                    Save_UI_Setting();
                    checkCnt[Camid] = 0;
                }
            }
            else
                checkCnt[Camid] = 0;
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_CLR, &err);
            break;

        default:
            checkCnt[Camid] = 0;
            break;
    }

    if ((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE))
        return;
    
	if (uiLightTest == 1)
	    DEBUG_GREEN("%d %s %s cam %d val %d value %d\n",__LINE__, __FILE__,__FUNCTION__,Camid,Val,iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid]);

    if (MyHandler.MenuMode == VIDEO_MODE)
    {
        if (sysRFRxInMainCHsel != Camid)
            return;
        
        if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] < 2)
        {
 		    uiOsdDrawLightManual(UI_OSD_CLEAR);
        }
        else 
        {
 		    uiOsdDrawLightManual(UI_OSD_DRAW);
        }
    }
    else if (MyHandler.MenuMode == QUAD_MODE)
    {
        if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] < 2)
        {
 		    uiOsdDrawQuadLightManual(Camid,UI_OSD_CLEAR);
        }
        else 
        {
 		    uiOsdDrawQuadLightManual(Camid,UI_OSD_DRAW);
        }
    }
}

void uiCheckLightManualSwitchStatus(u8 Camid)
{
    u8  err;

    if (gRfiu_Op_Sta[Camid] != RFIU_RX_STA_LINK_OK)
        return ;
    
    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

	if (uiLightTest == 1)
        DEBUG_GREEN("%d %s %s cam %d value %d\n",__LINE__, __FILE__,__FUNCTION__,Camid,iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid]);

    if ((iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] == UI_LIGHT_MANUAL_OFF) || (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] == UI_LIGHT_OFF))
    {
        DEBUG_UI("Manual Light On Channel %d \r\n", Camid);
        if ((MyHandler.MenuMode == VIDEO_MODE) && (sysRFRxInMainCHsel == Camid))
            uiOsdDrawLightManual(UI_OSD_DRAW);
        else if (MyHandler.MenuMode == QUAD_MODE)
            uiOsdDrawQuadLightManual(Camid,UI_OSD_DRAW);
        iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] = UI_LIGHT_MANUAL_ON;
        uiSetRfLightState[Camid] = uiSetRfManualLightingRxToTx(UI_TX_LIGHT_MANUAL_ON, Camid);
    }
    else
    {
        DEBUG_UI("Manual Light Off Channel %d \r\n", Camid);
        if ((MyHandler.MenuMode == VIDEO_MODE) && (sysRFRxInMainCHsel == Camid))
            uiOsdDrawLightManual(UI_OSD_CLEAR);
        else if (MyHandler.MenuMode == QUAD_MODE)
            uiOsdDrawQuadLightManual(Camid,UI_OSD_CLEAR);
        iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Camid] = UI_LIGHT_MANUAL_OFF;
        uiSetRfLightState[Camid] = uiSetRfManualLightingRxToTx(UI_TX_LIGHT_MANUAL_OFF, Camid);
    } 
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_CLR, &err);
    #if(NIC_SUPPORT)
    UpdateMAYONLightStatus(Camid);
    #endif
    Save_UI_Setting();
}

#if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN) 
void uiSyncLightTimer2LightInterval(u8 nCamCount, u8 u8Enable)
{
    u8 i;
    u8 u8Idx;
    u8 u8StartGrid = 0, u8EndGrid = 0;
    int i32OverNightGrid = -1; // the position among 48 grids
    const u8 u8Idx4Sunday = 0;
    const u8 u8Idx4Saturday = 6;
    u8StartGrid = (uiLightTimer[nCamCount][0] << 1) + (uiLightTimer[nCamCount][1] >= 30 ? 1 : 0);
    u8EndGrid = (uiLightTimer[nCamCount][2] << 1) + (uiLightTimer[nCamCount][3] >= 30 ? 1: 0);

    //DEBUG_UI("[Light] uiSyncLightTimer2LightInterval nCamCount = %d\n", nCamCount);
    //DEBUG_UI("[Light] uiSyncLightTimer2LightInterval StartGrid = %d, EndGrid = %d \n", u8StartGrid, u8EndGrid);

    if(u8EndGrid < u8StartGrid)
    {
        i32OverNightGrid = u8EndGrid;
        u8EndGrid = 48; //the last grid(0~47)
    }

    i = 0;
    memset(&uiLightInterval[nCamCount], 0, sizeof(uiLightInterval[nCamCount])); //is there any good way to sizeof?

    while(i < 7)
    {

        if (uiLightWeek[nCamCount] & (1 << i)) 
        {
            u8Idx = u8StartGrid;
            while(u8Idx <u8EndGrid)
            {
                if((u8Idx & 0x7) == 0 && (u8EndGrid - u8Idx) > 8)
                {
                    uiLightInterval[nCamCount][i][u8Idx >> 3] = 0xFF;
                    u8Idx += 8;
                }
                else
                {
                    uiLightInterval[nCamCount][i][u8Idx >> 3] |= 1 << (7 - (u8Idx & 0x7)); //u8Idx >> 3 => u8Idx / 8bit a set, % 8 => & 0x7, fill up 1
                    u8Idx++;
                }
            }
        }
        
        if(i > u8Idx4Sunday) //check whether it was overnight yesterday (Mon.~Sat.)
        {
            if((i32OverNightGrid != -1) && (uiLightWeek[nCamCount] & (1 << (i-1))))
            {
                u8Idx = 0;
                while(u8Idx < i32OverNightGrid)
                {
                    if((u8Idx & 0x7) == 0 && (u8EndGrid - u8Idx) > 8)
                    {
                        uiLightInterval[nCamCount][i][u8Idx >> 3] = 0xFF;
                        u8Idx += 8;
                    }
                    else
                    {
                        uiLightInterval[nCamCount][i][u8Idx >> 3] |= 1 << (7 - (u8Idx & 0x7)); //u8Idx >> 3 => u8Idx / 8bit a set, % 8 => & 0x7, fill up 1
                        u8Idx++;
                    }
                }
            }
        }
        else //check whether it was overnight on Sat. for i = Sunday
        {
            if((i32OverNightGrid != -1) && (uiLightWeek[nCamCount] & (1 << u8Idx4Saturday)))
            {
                u8Idx = 0;
                while(u8Idx < i32OverNightGrid)
                {
                    if((u8Idx & 0x7) == 0 && (u8EndGrid - u8Idx) > 8)
                    {
                        uiLightInterval[nCamCount][i][u8Idx >> 3] = 0xFF;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
                        u8Idx += 8;                               
                    }
                    else
                    {
                        uiLightInterval[nCamCount][i][u8Idx >> 3] |= 1 << (7 - (u8Idx & 0x7)); //u8Idx >> 3 => u8Idx / 8bit a set, % 8 => & 0x7, fill up 1
                        u8Idx++;
                    }
                }
            }
        }
        uiSetRfLightTimerRxToTx(i, uiLightInterval[nCamCount][i][0],uiLightInterval[nCamCount][i][1],uiLightInterval[nCamCount][i][2],uiLightInterval[nCamCount][i][3],uiLightInterval[nCamCount][i][4],uiLightInterval[nCamCount][i][5], nCamCount, u8Enable);

        i++;
    }
}
#endif

#if (NIC_SUPPORT)
u8 uiGetLightStatusAPP(u8 Ch_ID)
{
    if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+Ch_ID] < 2)
        return FALSE;//close        
    else
        return TRUE;//open        
}

u8 uiSetLightStatusAPP(u8 ch, u8 status)
{
    if (status == 0) //status 0:off  1:on
    {
        if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+ch] > 1)//open
        {
            uiCheckLightManualSwitchStatus(ch);//will be closed
        }
        else
            UpdateMAYONLightStatus(ch);
        
        uiFlowSetLightStatus(ch, 2, 2);
        
        if(iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+ch] < 2)
            return 0;//success
        else
            return 1;
    }
    else
    {
        if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+ch] < 2)
        {
            uiCheckLightManualSwitchStatus(ch);
        }
        else
            UpdateMAYONLightStatus(ch);
        
        uiFlowSetLightStatus(ch, 0, 2);

        if(iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+ch] > 1)
            return 0;//success
        else
            return 1;
    }
    
}
#endif

#endif

#if UI_CAMERA_ALARM_SUPPORT
void uiFlowSetCAStatus(u8 Camid, u8 Val, u8 Act)
{
    u8  err;
    static u8 checkCnt[MULTI_CHANNEL_MAX] = {0};

	if (uiLightTest == 1)
	    DEBUG_GREEN("%d %s %s cam %d val %d value %d\n",__LINE__, __FILE__,__FUNCTION__,Camid,Val,iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid]);

  	switch (Val)
    {
        case 5:
            if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] != UI_LIGHT_MANUAL_ON)
            {
                uiSetRfAlarmState[Camid] = uiSetRfManualLightingRxToTx(UI_TX_ALARM_MANUAL_OFF, Camid);
            }
            checkCnt[Camid] = 0;
            return;

        case 4:
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            if ((iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] == UI_LIGHT_OFF) || (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] == UI_LIGHT_MANUAL_OFF) || (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] == UI_LIGHT_MANUAL_ON))
            {
                iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] = UI_LIGHT_TRIGGER_ON;
                #if(NIC_SUPPORT)
                UpdateMAYONAlarmStatus(Camid);
                #endif
                Save_UI_Setting();
            }
            checkCnt[Camid] = 0;
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_CLR, &err);
            break;

        case 3:
            OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
            if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] == UI_LIGHT_TRIGGER_ON)
            {
                iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] = UI_LIGHT_OFF;
                #if(NIC_SUPPORT)
                UpdateMAYONAlarmStatus(Camid);
                #endif
                Save_UI_Setting();
                checkCnt[Camid] = 0;
            }
            else if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] == UI_LIGHT_MANUAL_ON)
            {
            	if (uiLightTest == 1)
            		DEBUG_GREEN("@@@ CAM = %d,checkCnt = %d\n",Camid,checkCnt[Camid]);	

                if (checkCnt[Camid] == 0)
                    checkCnt[Camid]++;
                else
                {
                    iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] = UI_LIGHT_MANUAL_OFF;
                    Save_UI_Setting();
                    checkCnt[Camid] = 0;
                }
            }
            else
                checkCnt[Camid] = 0;
            OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_CLR, &err);
            break;

        default:
            checkCnt[Camid] = 0;
            break;
    }

    if ((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE))
        return;
    
	if (uiLightTest == 1)
	    DEBUG_GREEN("%d %s %s cam %d val %d value %d\n",__LINE__, __FILE__,__FUNCTION__,Camid,Val,iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid]);

    if (MyHandler.MenuMode == VIDEO_MODE)
    {
        if (sysRFRxInMainCHsel != Camid)
            return;
        
        if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] < 2)
        {
 		    uiOsdDrawCamreaAlarmManual(UI_OSD_CLEAR);
        }
        else 
        {
 		    uiOsdDrawCamreaAlarmManual(UI_OSD_DRAW);
        }
    }
    else if (MyHandler.MenuMode == QUAD_MODE)
    {
        if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] < 2)
        {
 		    uiOsdDrawQuadCamreaAlarmManual(Camid,UI_OSD_CLEAR);
        }
        else
        {
 		    uiOsdDrawQuadCamreaAlarmManual(Camid,UI_OSD_DRAW);
        }
    }
}

void uiCheckAlarmManualSwitchStatus(u8 Camid)
{
    u8  err;

    if (gRfiu_Op_Sta[Camid] != RFIU_RX_STA_LINK_OK)
        return ;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

	if (uiLightTest == 1)
        DEBUG_GREEN("%d %s %s cam %d value %d\n",__LINE__, __FILE__,__FUNCTION__,Camid,iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid]);

    if ((iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] == UI_CAMERA_ALARM_MANUAL_OFF) || (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] == UI_CAMERA_ALARM_OFF))
    {
        DEBUG_UI("Manual Alarm On Channel %d \r\n", Camid);
        if ((MyHandler.MenuMode == VIDEO_MODE) && (sysRFRxInMainCHsel == Camid))
            uiOsdDrawCamreaAlarmManual(UI_OSD_DRAW);
        else if (MyHandler.MenuMode == QUAD_MODE)
            uiOsdDrawQuadCamreaAlarmManual(Camid,UI_OSD_DRAW);
        iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] = UI_CAMERA_ALARM_MANUAL_ON;
        uiSetRfAlarmState[Camid] = uiSetRfManualLightingRxToTx(UI_TX_ALARM_MANUAL_ON, Camid);
    }
    else
    {
        DEBUG_UI("Manual Alarm Off Channel %d \r\n", Camid);
        if ((MyHandler.MenuMode == VIDEO_MODE) && (sysRFRxInMainCHsel == Camid))
            uiOsdDrawCamreaAlarmManual(UI_OSD_CLEAR);
        else if (MyHandler.MenuMode == QUAD_MODE)
            uiOsdDrawQuadCamreaAlarmManual(Camid,UI_OSD_CLEAR);
        iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Camid] = UI_CAMERA_ALARM_MANUAL_OFF;
        uiSetRfAlarmState[Camid] = uiSetRfManualLightingRxToTx(UI_TX_ALARM_MANUAL_OFF, Camid);
    } 
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_CLR, &err);
    #if(NIC_SUPPORT)
    UpdateMAYONAlarmStatus(Camid);
    #endif
    Save_UI_Setting();
}

#if (NIC_SUPPORT)
u8 uiGetAlarmStatusAPP(u8 Ch_ID)
{
    if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+Ch_ID] < 2)
        return FALSE;//close        
    else
        return TRUE;//open        
}

u8 uiSetAlarmStatusAPP(u8 ch, u8 status)
{
    
    if (status == 0) //status 0:off  1:on
    {
        if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+ch] > 1)
        {
            uiCheckAlarmManualSwitchStatus(ch);
        }
        else
            UpdateMAYONAlarmStatus(ch);
        
        uiFlowSetCAStatus(ch,5,2);
        
        if(iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+ch] < 2)
            return 0;//success
        else
            return 1;
    }
    else
    {
        if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+ch] < 2)
        {
            uiCheckAlarmManualSwitchStatus(ch);
        }
        else
            UpdateMAYONAlarmStatus(ch);
        
        uiFlowSetCAStatus(ch,3,2);

        if(iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+ch] > 1)
            return 0;//success
        else
            return 1;
    }
    
}
#endif
#endif

void uiFlowSetRfLightStatus(u8 Camid, u8 Val, u8 Act)
{

    if (Val>2)
    {
        #if UI_CAMERA_ALARM_SUPPORT
        uiFlowSetCAStatus(Camid,Val,Act);
        #endif
    }
    else
    {
        #if UI_LIGHT_SUPPORT
        uiFlowSetLightStatus(Camid,Val,Act);
        #endif
    }

}

void uiResetAlarmSetting(u8 camId,u8 type,u8 day)
{    

    if (gRfiu_Op_Sta[camId] != RFIU_RX_STA_LINK_OK)
        return;

    switch(type)
    {
#if UI_LIGHT_SUPPORT   
        case UI_MENU_SETIDX_CH1_LS_TIMER:
            uiSetCfg |= (0<<5);
            uiSetRfLSTimer[camId][day] = uiSetRfLightTimerRxToTx(day, uiLightInterval[camId][day][0],uiLightInterval[camId][day][1],uiLightInterval[camId][day][2],uiLightInterval[camId][day][3],
                                   uiLightInterval[camId][day][4],uiLightInterval[camId][day][5], camId, uiSetCfg);
            break;
#endif

#if UI_CAMERA_ALARM_SUPPORT 
        case UI_MENU_SETIDX_CH1_CA_TIMER:
            uiSetCfg |= (1<<5);
            uiSetRfAlarmTimer[camId][day] = uiSetRfLightTimerRxToTx(day, uiCamAlarmInterval[camId][day][0],uiCamAlarmInterval[camId][day][1],uiCamAlarmInterval[camId][day][2],uiCamAlarmInterval[camId][day][3],
                                   uiCamAlarmInterval[camId][day][4],uiCamAlarmInterval[camId][day][5], camId, uiSetCfg);
            break;
#endif
    }
}

#if RFIU_SUPPORT
void uiSynRfConfig(u8 camId)
{
    u8  i,level;
    u8  uartCmd[16];
    int RfBusy = 1;

    // compare TX & RX resolution, and sync it
    //DEBUG_UI("========== Video Size is %d ==========\r\n",iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+camId]);
    if (uiP2PMode == 0)
    {
        switch(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+camId])
        {
            case UI_MENU_SETTING_RESOLUTION_HD:
                if((gRfiuUnitCntl[camId].TX_PicWidth != 1280) || (gRfiuUnitCntl[camId].TX_PicHeight != 720))
                    uiMenuSet_TX_CameraResolution(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+camId],camId);
                break;

            case UI_MENU_SETTING_RESOLUTION_QHD:
                if((gRfiuUnitCntl[camId].TX_PicWidth != 640) || (gRfiuUnitCntl[camId].TX_PicHeight != 480))
                    uiMenuSet_TX_CameraResolution(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+camId],camId);
                break;

            case UI_MENU_SETTING_RESOLUTION_QVGA:
                if((gRfiuUnitCntl[camId].TX_PicWidth != 320) || (gRfiuUnitCntl[camId].TX_PicHeight != 240))
                    uiMenuSet_TX_CameraResolution(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+camId],camId);
                break;
            case UI_MENU_SETTING_RESOLUTION_1920x1088:
                if((gRfiuUnitCntl[camId].TX_PicWidth != 1920) || ((gRfiuUnitCntl[camId].TX_PicHeight != 1072)&&(gRfiuUnitCntl[camId].TX_PicHeight != 1088)))
                    uiMenuSet_TX_CameraResolution(iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+camId],camId);
                break;
        }
    }
    else
    {
        if((gRfiuUnitCntl[camId].RFpara.BateryCam_support == 1) && (gRfiuUnitCntl[camId].RFpara.BatCam_PIRMode == 1))
        {}
        else
        {
            level = uiGetP2PStatueToRF();

            if(level == 1)
                uiSetRfResolutionRxToTx(UI_RESOLTUION_FHD, camId);
            else
                uiSetRfResolutionRxToTx(UI_RESOLTUION_HD, camId);

            sprintf((char*)uartCmd,"MODE %d %d", rfiuRX_OpMode, level);
            while(RfBusy != 0)
            {
                RfBusy = rfiu_RXCMD_Enc(uartCmd, camId);
                if (cnt >UI_SET_FR_CMD_RETRY)
                {
                    DEBUG_UI("uiSetP2PStatueToRF Timeout:%d !!!\n",RfBusy);
                    break;
                }
                cnt++;
                OSTimeDly(1);
            }
        }
        //uiSynRFConfigInP2P(camId);
    }
    uiMenuAction((UI_MENU_SETIDX_BRIGHTNESS_CH1+camId));
    uiMenuAction((UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1+camId));
    uiMenuAction(UI_MENU_SETIDX_50HZ_60HZ);    
    uiSetRfTimeRxToTx(camId);
    
    #if UI_LIGHT_SUPPORT
        if (gRfiu_Op_Sta[camId] == RFIU_RX_STA_LINK_OK)
        {
            if (uiSetRfLightSwitch[camId] != UI_SET_RF_OK)
                uiMenuAction((UI_MENU_SETIDX_CH1_LS_ONOFF+camId));

            if (uiSetRfLightState[camId] != UI_SET_RF_OK)
                uiMenuAction((UI_MENU_SETIDX_CH1_LS_STATUS+camId));
        }
        #if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
        if (uiSetRfLightTimer[camId] == UI_SET_RF_BUSY)
            uiMenuAction((UI_MENU_SETIDX_CH1_LS_TIMER+camId));
        #elif (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
        for (i = 0; i < 7; i++) // 7 days a week
            if (uiSetRfLSTimer[camId][i] != UI_SET_RF_OK)
                uiResetAlarmSetting(camId,UI_MENU_SETIDX_CH1_LS_TIMER,i);
        #endif
    #endif

    #if UI_CAMERA_ALARM_SUPPORT
        if (gRfiu_Op_Sta[camId] == RFIU_RX_STA_LINK_OK)
        {
            if (uiSetRfAlarmSwitch[camId] != UI_SET_RF_OK)
                uiMenuAction((UI_MENU_SETIDX_CH1_CA_ONOFF+camId));

            if (uiSetRfAlarmState[camId] != UI_SET_RF_OK)
                uiMenuAction((UI_MENU_SETIDX_CH1_CA_STATUS+camId));
        }
    
        for (i = 0; i < 7; i++) // 7 days a week
            if (uiSetRfAlarmTimer[camId][i] != UI_SET_RF_OK)
                uiResetAlarmSetting(camId,UI_MENU_SETIDX_CH1_CA_TIMER,i);
    #endif   
    
}
#endif

void uiFlowCheckCurRecState(u8 camID)
{
    u8 isPIR = FALSE, isMot = FALSE, result = FALSE;
    
    if (VideoClipParameter[camID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode & ASF_CAPTURE_PIRTIGGER_ENA)
        isPIR = TRUE;

    if (VideoClipParameter[camID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode & ASF_CAPTURE_EVENT_MOTION_ENA)
        isMot = TRUE;
    
    switch(uiCurRecStatus[camID])
    {
        case UI_REC_TYPE_MANUAL:
        case UI_REC_TYPE_SCHEDULE:
            if ((isPIR == FALSE) && (isMot == FALSE))
                result = TRUE;
            break;
            
        case UI_REC_TYPE_MOTION:
            if ((isPIR == FALSE) && (isMot == TRUE))
                result = TRUE;
            break;
            
#if UI_BAT_SUPPORT            
        case UI_REC_TYPE_PIR:
            if ((isPIR == TRUE) && (isMot == FALSE))
                result = TRUE;
            break;
#endif

        default:
            return;
    }
    
    if ((result == FALSE) && (MultiChannelGetCaptureVideoStatus(camID + MULTI_CHANNEL_LOCAL_MAX) == UI_REC_STATUS_RECING))
    {
        uiCaptureVideoStopByChannel(camID);
        DEBUG_YELLOW("Stop Current Rec %d, isPIR %d  isMot %d\n", camID, isPIR, isMot); 
    }
}

void uiFlowCheckRecState(void)
{
    u8  i;
    
    if (Main_Init_Ready == 0)
        return;

    if ((sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY) || ((SysOverwriteFlag == FALSE) && (MemoryFullFlag == TRUE)))
        return;

    if((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE))
        return;
        
    for (i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        if(gRfiu_Op_Sta[i] == RFIU_RX_STA_LINK_BROKEN)
            continue;

        uiFlowCheckCurRecState(i);

        if (uiCurRecStatus[i] == UI_REC_TYPE_MANUAL)
        {
            if (MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == UI_REC_STATUS_NONE)
            {
                DEBUG_UI("Camera %d Continue REC\n", i);
                VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
                uiCaptureVideoByChannel(i);
            }
        }
        else if (uiCurRecStatus[i] == UI_REC_TYPE_MOTION)
        {
            if (MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == UI_REC_STATUS_NONE)
            {
                DEBUG_UI("Camera %d Continue Motion\n",i);
                VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_EVENT_MOTION_ENA;
                uiCaptureVideoByChannel(i);
            }
        }
#if(UI_BAT_SUPPORT)
        else if (uiCurRecStatus[i] == UI_REC_TYPE_PIR)
        {
            if (_uiBatType[i] == UI_CAM_BATTERY)
            {
                if (gRfiuUnitCntl[i].RFpara.BatCam_PIRMode == 1)
                {
                    if (MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == UI_REC_STATUS_NONE)
                    {
                        DEBUG_UI("Camera %d PIR REC\n",i);
                        VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_PIRTIGGER_ENA;
                        VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
                        uiCaptureVideoByChannel(i);
                    }
                }
                else
                {
                    if (MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == UI_REC_STATUS_NONE)
                    {
                        VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                        VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
                        if (isManualRec == UI_OSD_DRAW)
                        {
                            uiCaptureVideoByChannel(i);
                            DEBUG_UI("Camera %d manual rec in PIR\n",i);
                        }
                        else
                        {
                            uiCaptureVideoStopByChannel(i);
                            DEBUG_UI("Camera %d STOP REC In PIR\n",i);
                        }
                    }
                }
            }
        }
#endif
        else if (uiCurRecStatus[i] == UI_REC_TYPE_SCHEDULE)
        {
            UICheckSchedule = 48;
        }
        else if (uiCurRecStatus[i] == UI_REC_TYPE_NONE)
        {
            VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
            VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
        }
    }
}

void uiFlowCheckRetryRec(void)
{
    //u8 i,check=0;

    if (uiDrawSDFail == UI_OSD_DRAW)
    {
        if (uiRetryRecTime == UI_RETRY_REC_NONE)
            uiRetryRecTime = UI_RETRY_REC_TIME;
        else
            uiRetryRecTime --;
        if (uiRetryRecTime == 0)
        {
            uiOsdDrawSDCardFail(UI_OSD_CLEAR);
            uiFlowCheckRecState();
            uiRetryRecTime = UI_RETRY_REC_NONE;
        }
    }
}

#if UI_BAT_SUPPORT 
/** Sync Alarm Status between TX and RX After BTC Sleep. */
void uiCheckBTCAlarm(u8 camID)
{
    u8 err;
    
    if (gRfiuUnitCntl[camID].WakeUpTxEn == 1)
        return;

    if(!((MyHandler.MenuMode == VIDEO_MODE ) || (MyHandler.MenuMode == QUAD_MODE )))
        return;

    if (_uiBatType[camID] == UI_CAM_NORMAL)
        return;

    if (sysGetBTCTimer(camID) > 0)
        return;

    OSFlagPend(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    //close alarm
    #if UI_LIGHT_SUPPORT 
    if (!((iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+camID] == UI_LIGHT_MANUAL_OFF) || (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+camID] == UI_LIGHT_OFF)))
    {
        DEBUG_UI("Manual Light Off Channel %d \r\n", camID);
        if ((MyHandler.MenuMode == VIDEO_MODE) && (sysRFRxInMainCHsel == camID))
            uiOsdDrawLightManual(UI_OSD_CLEAR);
        else if (MyHandler.MenuMode == QUAD_MODE)
            uiOsdDrawQuadLightManual(camID,UI_OSD_CLEAR);
        iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+camID] = UI_LIGHT_MANUAL_OFF;
    }    
    #endif

    #if UI_CAMERA_ALARM_SUPPORT 
    if (!((iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+camID] == UI_CAMERA_ALARM_MANUAL_OFF) || (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+camID] == UI_CAMERA_ALARM_OFF)))
    {
        DEBUG_UI("Manual Alarm Off Channel %d \r\n", camID);
        if ((MyHandler.MenuMode == VIDEO_MODE) && (sysRFRxInMainCHsel == camID))
            uiOsdDrawCamreaAlarmManual(UI_OSD_CLEAR);
        else if (MyHandler.MenuMode == QUAD_MODE)
            uiOsdDrawQuadCamreaAlarmManual(camID,UI_OSD_CLEAR);
        iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+camID] = UI_CAMERA_ALARM_MANUAL_OFF;
    }
    #endif
    
    OSFlagPost(gUiStateFlagGrp, FLAGUI_UI_CHANGE_SET, OS_FLAG_CLR, &err);
    Save_UI_Setting();

}

u8 uiCheckMotionByCamType(u8 camID)
{
    if (_uiBatType[camID]==UI_CAM_NORMAL)
    {
        if (iconflag[UI_MENU_SETIDX_REC_MODE_CH1 + camID] == UI_MENU_REC_MODE_MOTION)
        {
            return TRUE;
        }
    }
    else
    {
        if (!sysBatteryCam_isSleeping(camID))
        {
            return TRUE;
        }
    } 
    return FALSE;
}

void uiFlowCheckRecPIR(u8 RFID)
{
    if (Main_Init_Ready == 0)
        return;

    if ((sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY) || ((SysOverwriteFlag == FALSE) && (MemoryFullFlag == TRUE)))
        return;

    if(_uiBatType[RFID] == UI_CAM_BATTERY)
    {
        if(gRfiuUnitCntl[RFID].RFpara.BatCam_PIRMode == 1)
        {
            if (MultiChannelGetCaptureVideoStatus(RFID+MULTI_CHANNEL_LOCAL_MAX) == UI_REC_STATUS_NONE)
            {
                DEBUG_UI("Camera %d PIR REC\n",RFID);
                VideoClipParameter[RFID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_PIRTIGGER_ENA;
                VideoClipParameter[RFID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
                uiCaptureVideoByChannel(RFID);
            }
        }
    }
}

void uiOsdDrawBtcPaly(void)
{
    static u8 countDarwPlay=0;
    u8 i;
    
    if (countDarwPlay > 2)
    {
        for (i = 0; i < 4; i++)
        {
            osdDrawCamLiveView(i);
        }
        countDarwPlay = 0;
    }
    else
    {
        countDarwPlay += 1;
    }
}

void uiCheckBatterySchdule(void)
{
    u8 i=0,j=0,tmpSch,cam,week,enable, change = 0;
    RTC_DATE_TIME   localTime;
    static u8 lastSchedule=0xff, first = 1;
    
    if (Main_Init_Ready == 0)
        return ;

    RTC_Get_Time(&localTime);
    
    tmpSch = localTime.hour*2;
    if (localTime.min >= 30)
        tmpSch++;

    if ((tmpSch < _uiCheckBatterySch)||(_uiCheckBatterySch == 48))
    {
        if (_uiCheckBatterySch == 48)
            lastSchedule = 0xFF;
    }
    _uiCheckBatterySch = tmpSch;

    if (lastSchedule == _uiCheckBatterySch)
        return;
    //DEBUG_UI("_uiCheckBatterySch %d lastSchedule %d \n",_uiCheckBatterySch,lastSchedule);   
    
    i = tmpSch>>3;
    j = tmpSch%8;

    if (localTime.week==0)
        week=6;
    else
        week=localTime.week-1;

    for (cam=0; cam<MULTI_CHANNEL_MAX; cam++)
    {
        enable = 0;
        if (uiBatteryInterval[cam][week][i] & (0x1 << j))
        {
            if (iconflag[UI_MENU_SETIDX_SET_AREC] & (0x01 << cam))
            {
                enable = 1;
            }
        }

        if (gUiPIRSch[cam] != enable)
            change = 1;
        
        gUiPIRSch[cam] = enable;
    
        if (iconflag[UI_MENU_SETIDX_CH1_ON + cam] == UI_MENU_SETTING_CAMERA_OFF)
            continue;
        
        if (gRfiuUnitCntl[cam].RFpara.BateryCam_support == 0)
            continue;
        //DEBUG_UI("cam %d pir %d %d\n",cam,gUiPIRSch[cam],enable);
        //DEBUG_YELLOW("%d %s %s sch %d %d enable %d\n",__LINE__, __FILE__,__FUNCTION__,cam,_uiBatPIRStatus[cam],enable);

        if ((change) || (first))
        {
            if (sysBatteryCam_isSleeping(cam))
                sysBatteryCam_wake(cam,sysGetBTCWakeTime());
            else
                uiMenuAction((UI_MENU_SETIDX_CH1_PIR+cam));
            DEBUG_UI("CAM %d SYNC PIR %d IN SCHDULE\n",cam,gUiPIRSch[cam]);
        }
    }
    lastSchedule = tmpSch;
    first = 0;
}

void uiFlowSetPir(u8 cam, u8 manual)
{
    u8 i=0,j=0,tmpSch,week,enable;
    RTC_DATE_TIME   localTime;
    
    if (Main_Init_Ready == 0)
        return;

    if (manual == 0)
    {
        iconflag[UI_MENU_SETIDX_SET_AREC] |= (0x1 << cam);
        Save_UI_Setting();
    }

    if  (gRfiuUnitCntl[cam].RFpara.BateryCam_support == 1)
    {
        RTC_Get_Time(&localTime);

        tmpSch = localTime.hour*2;
        if (localTime.min >= 30)
            tmpSch++;

        i = tmpSch>>3;
        j = tmpSch%8;

        if (localTime.week==0)
            week=6;
        else
            week=localTime.week-1;

        enable = 0;

        
        if (uiBatteryInterval[cam][week][i] & (0x1 << j))
        {
            if (iconflag[UI_MENU_SETIDX_SET_AREC] & (0x01 << cam))
            {
                enable = 1;
            }
        }

        gUiPIRSch[cam] = enable;

        uiMenuAction((UI_MENU_SETIDX_CH1_PIR + cam));
        if ((sysRFRxInMainCHsel == cam) && (manual == 0))
        {
            osdDrawMotionPreview(UI_OSD_DRAW);
        }
    }
    else
    {
        uiMenuAction((UI_MENU_SETIDX_CH1_PIR + cam));
    }
}

void uiCheckBTCRec(u8 camID,u8 linkType)
{
    u8 newType;
    
    newType = gRfiuUnitCntl[camID].RFpara.BateryCam_support;  
        
    DEBUG_UI("cam %d link %d pir %d batlv %d oldType %d newType %d \n",camID,linkType,gRfiuUnitCntl[camID].RFpara.BatCam_PIRMode,gRfiuUnitCntl[camID].RFpara.TxBatteryLev,_uiBatType[camID],newType);

    /* 20190117 */
    if (iconflag[UI_MENU_SETIDX_REC_MODE_CH1 + camID] == UI_MENU_REC_MODE_MANUAL)
    {
        iconflag[UI_MENU_SETIDX_REC_MODE_CH1 + camID] = UI_MENU_REC_MODE_MOTION;
    }

    if (_uiBatType[camID] != newType)
    {
        _uiBatType[camID] = newType;
        UpdateMAYONTXStatus(camID);

        switch(newType)
        {
            case UI_CAM_BATTERY:
                uiCurRecStatus[camID] = UI_REC_TYPE_PIR;
                break;
                
            case UI_CAM_NORMAL:
                if (isManualRec == UI_OSD_DRAW)
                    uiCurRecStatus[camID] = UI_REC_TYPE_MANUAL;
                else if (iconflag[UI_MENU_SETIDX_REC_MODE_CH1 + camID] == UI_MENU_REC_MODE_MOTION)
                    uiCurRecStatus[camID] = UI_REC_TYPE_MOTION;  
                else //schdule
                    uiCurRecStatus[camID] = UI_REC_TYPE_NONE;
                UICheckSchedule = 48;
                break;
        }
    }
    else
    {
        if (linkType == UI_RF_STATUS_NO_SINGLE)
        {
            UpdateMAYONTXStatus(camID);
        }
    }

    uiFlowCheckRecState();

    if (linkType == UI_RF_STATUS_LINK)
    {
        uiFlowSetPir(camID, 0);
        uiFlowCheckRecPIR(camID);
    }
    else if (linkType == UI_RF_STATUS_NO_SINGLE)
    {
        uiMotionSecTime[camID] = UI_MD_NOT_REC;
        GMotionTrigger[camID+MULTI_CHANNEL_LOCAL_MAX] = 0;
    }
    
    #if (UI_LIGHT_SUPPORT || UI_CAMERA_ALARM_SUPPORT)
    uiCheckBTCAlarm(camID);
    #endif
    
}

void uiCheckBTCTimer(u8 camID,u8 linkType)
{    
    if (_uiBatType[camID] == UI_CAM_BATTERY)
    {
        if (linkType != UI_RF_STATUS_NO_SINGLE)
        {
            if (_uiBatType[camID] == UI_CAM_BATTERY)
            {
                if (sysGetBTCWakeStatus(camID) != SYS_BTC_WAKEUP_NO)
                {
                    sysSetBTCTimer(camID,sysGetBTCWakeTime());
                    DEBUG_UI("Cam %d Wake Up Start\n",camID);
                }
                else
                {
                    /*手動換醒後斷線又因為PIR而連線，造成PIR連線中會被上個手動倒數而關掉，所以這邊要清COUNTER避免倒數完會SLEEP CMD*/
                    if(sysGetBTCTimer(camID) != 0)
                    {
                        sysSetBTCTimer(camID,0);
                        DEBUG_UI("Cam %d Stop WakeUp to PIR\n",camID);
                    }
                }
            }
        }
    }
}

void uiCheckBTCSetting(u8 camID,u8 linkType)
{
    uiCheckBTCRec(camID, linkType);
    uiCheckBTCTimer(camID, linkType);
}
#endif

void uiSetZoomOut(void)
{
    #if GET_SIU_RAWDATA_PURE //Lucian: For debug!
    {
        u8 MSG[64];
        #if SELECT_ADJUST_EL_AGC
            siuAdjustSW -=10;
            if(siuAdjustSW<1)
                siuAdjustSW=1;
            sprintf((char*)MSG,"EL=%d",siuAdjustSW);
        #else
            siuAdjustAGC --;
            if(siuAdjustAGC<1)
                siuAdjustAGC=1;
            sprintf((char*)MSG,"AGC=%d",siuAdjustAGC);
        #endif
            uiClearOSDBuf(2);
            uiOSDASCIIStringByColor(MSG, (OSDIconMidX-srtlen(MSG)*OSD_STRING_W/2) , (OSDIconMidX-8) , OSD_Blk2 , 0xc0, 0x00);
    }
    #else
        if(sysCameraMode == SYS_CAMERA_MODE_PREVIEW)
        {
            sysPreviewZoomFactor --;

            if(siuOpMode == SIUMODE_MPEGAVI)
            {
                if(sysPreviewZoomFactor<0)
                   sysPreviewZoomFactor=0;
                else
                   sysbackSetEvt(SYS_BACK_VIDEOZOOMINOUT, sysPreviewZoomFactor);
            }
            else
            {
                if(sysPreviewZoomFactor<0)
                   sysPreviewZoomFactor=0;
                else
                   sysSetEvt(SYS_EVT_PREVIEWZOOMINOUT, sysPreviewZoomFactor);
            }
        }
    #endif
}

void uiPlaybackMoveForward(void)
{
#if (UI_VERSION == UI_VERSION_TX)
    return 0;
#else
    u32 FileNum = 0;

    FileNum = dcfGetCurDirFileCount();
    asfIndexTableCount=0;

    if (FileNum == 0)
    {
        IduVideo_ClearBuf();
        if (sysTVOutOnFlag)
            iduSetVidBufAddr(0, PKBuf0);
        else
            iduSetVidBufAddr(0, iduvideobuff);
        osdDrawFillEmpty();
        return;
    }

    playback_location--;
    if (!playback_location)
        playback_location = FileNum;

    if (dcfPlaybackCurFile == NULL)
        return;

    dcfPlaybackFileNext();
    osdDrawPlayIcon();
    if(uiPlaybackStop(GLB_ENA)== 1)
    {
        osdDrawPlayIndicator(100);
    }
    else
    {
        osdDrawPlayIndicator(UI_PLAY_ICON_PLAY);
    }
    uiReadVideoFile();

#endif
}

void uiPlaybackMoveBackward(void)
{
#if (UI_VERSION == UI_VERSION_TX)
    return 0;
#else
    u32 FileNum = 0;

    FileNum = dcfGetCurDirFileCount();
    asfIndexTableCount = 0;

    if (FileNum == 0)
    {
        IduVideo_ClearBuf();
        if (sysTVOutOnFlag)
            iduSetVidBufAddr(0, PKBuf0);
        else
            iduSetVidBufAddr(0, iduvideobuff);
        osdDrawFillEmpty();
        return;
    }

    playback_location++;
    if (playback_location > FileNum)
        playback_location = 1;

    // DEBUG_SYS("playback_location %d \n",playback_location);
    if (dcfPlaybackCurFile == NULL)
        return;
    
    dcfPlaybackFilePrev();
    osdDrawPlayIcon();
    if(uiPlaybackStop(GLB_ENA)== 1)
    {
        osdDrawPlayIndicator(100);
    }
    else
    {
        osdDrawPlayIndicator(UI_PLAY_ICON_PLAY);
    }
    uiReadVideoFile();
#endif
}

void uiSetZoomIn(void)
{
    #if GET_SIU_RAWDATA_PURE //Lucian: For debug!
    {
        u8 MSG[64];
        #if SELECT_ADJUST_EL_AGC
            siuAdjustSW +=10;
            sprintf((char*)MSG,"EL=%d",siuAdjustSW);
        #else
            siuAdjustAGC ++;
            sprintf((char*)MSG,"AGC=%d",siuAdjustAGC);
        #endif
        uiClearOSDBuf(2);
        uiOSDASCIIStringByColor(MSG, (OSDIconMidX-srtlen(MSG)*OSD_STRING_W/2) , (OSDIconMidX-8) , OSD_Blk2 , 0xc0, 0x00);
    }
    #else
        if(sysCameraMode == SYS_CAMERA_MODE_PREVIEW)
        {
            sysPreviewZoomFactor ++;
            if(siuOpMode == SIUMODE_MPEGAVI)
            {
                if(sysPreviewZoomFactor > (MAX_VIDEOCLIP_ZOOM_FACTOR-1) )
                    sysPreviewZoomFactor=MAX_VIDEOCLIP_ZOOM_FACTOR-1;
                else
                    sysbackSetEvt(SYS_BACK_VIDEOZOOMINOUT, sysPreviewZoomFactor);
            }
            else
            {
                if(sysPreviewZoomFactor > (MAX_PREVIEW_ZOOM_FACTOR-1) )
                    sysPreviewZoomFactor=MAX_PREVIEW_ZOOM_FACTOR-1;
                else
                    sysSetEvt(SYS_EVT_PREVIEWZOOMINOUT, sysPreviewZoomFactor);
            }
        }
    #endif
}

void uiFlowPlayback_Delete_File(void)
{
    u8  CamId, Find = 0;
    DCF_LIST_DIRENT*    TmpDir;
    
    IduVideo_ClearPKBuf(0);
    if(Write_protet() && (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY))
    {
        DEBUG_UI("Write_protet.....\r\n");
        osdDrawProtect(1);
    }
    else
    {
        osdDrawFillWait();
        if (PlayListDspType == UI_DSP_PLAY_LIST_DIR)
        {
            DEBUG_UI("Delete Folder\r\n");
            {
            	dcfPlaybackDirForward();
                playbackDir--;
                if (!playbackDir)
                    playbackDir = dcfGetTotalDirCount();
            }
        }
#if UI_CALENDAR_SUPPORT
        else if (PlayListDspType == UI_DSP_PLAY_LIST_FILE)
        {
            DEBUG_UI("Delete File\r\n");
            CamId = dcfPlaybackCurFile->pDirEnt->d_name[7]-'1';
            dcfPlaybackDel();
            totalPlaybackFileNumADay--;
            if (totalPlaybackFileNumADay == 0)
            {
                DEBUG_UI("delete change to Calendar mode\r\n");
                MyHandler.MenuMode = PLAYBACK_MENU_MODE;
                PlayListDspType = UI_DSP_PLAY_LIST_DIR;
                uiEnterMenu(UI_MENU_NODE_PLAYBACK_CALENDAR);
                uiGraphDrawPlaybackList(UI_KEY_MODE, 1);
                uiOsdDisableAll();
                return;
            }
            if (dcfPlaybackCurDir->ChTotal[CamId] == 0)
            {
                TmpDir = dcfPlaybackCurDir;
                dcfPlaybackCurDir = dcfPlaybackCurDir->playbackPrev;
                while (dcfPlaybackCurDir != TmpDir)
                {
                    if(dcfScanFileOnPlaybackDir() == 0)
                        break;
                    while(Find == 0)
                    {
                        Find = uiOsdPlaybackFileFind();
                        DEBUG_UI("Prev Dir %s file %s %d\n",dcfPlaybackCurDir->pDirEnt->d_name, dcfPlaybackCurFile->pDirEnt->d_name, Find);
                        if (Find == 1)
                        {
                            break;
                        }
                        if (dcfPlaybackCurFile  == dcfGetPlaybackFileListTail())
                            break;
                        dcfPlaybackFilePrev();
                    }
                    dcfPlaybackCurDir = dcfPlaybackCurDir->playbackPrev;
                }                
            }
            playback_location--;
            if (!playback_location)
                playback_location = totalPlaybackFileNumADay;
            if (Find == 0)
            {
                do
                {
                    Find = uiOsdPlaybackFileFind();
                    if (Find == 1)
                        break;
                    dcfPlaybackFilePrev();
                }while(dcfPlaybackCurFile != dcfGetPlaybackFileListTail());
            }
        }
#else
        else if (PlayListDspType == UI_DSP_PLAY_LIST_FILE)
        {
            DEBUG_UI("Delete File\r\n");
            dcfPlaybackDel();
            playback_location--;
            if (!playback_location)
                playback_location = dcfGetCurDirFileCount();
        }
#endif
    }
    MyHandler.MenuMode = PLAYBACK_MENU_MODE;
    uiEnterMenu(UI_MENU_NODE_SET_PLAYBACK);
    uiOsdDrawPlaybackMenu(UI_KEY_DELETE);
    DEBUG_UI("delete change to Playback list\r\n");

}

void uiFlowEnterMenuMode(u32 Mode)
{
    u8 i;
    
    MyHandler.MenuMode = Mode;
    siuOpMode       = SIUMODE_START;
    sysCameraMode   = SYS_CAMERA_MODE_PLAYBACK;
    for (i=0;i<MULTI_CHANNEL_MAX;i++)
    {
        uiMotionSecTime[i] = UI_MD_NOT_REC;
        GMotionTrigger[i+MULTI_CHANNEL_LOCAL_MAX] = 0;
    }
    UICheckScheduleGetTime = 0;    
    iduLockVideoAllColor(0x80800000);
    iduMenuMode(PANNEL_X,PANNEL_Y,PANNEL_X);
    uiMenuOSDReset();
    #if RFIU_SUPPORT
        uiSetRfDisplayMode(UI_MENU_RF_ENTER_PLAYBACK);
    #endif
    uiGraphDrawMenu();
    iduUnlockVideoAllColor();
}

void uiFlowVideoMode(u8 WhichKey)
{
    u8 reSetRec=0, SetVal, temp;

#if RFIU_SUPPORT
    if (WhichKey != UI_KEY_TALK)
    {
        if (uiCheckRfTalkStatus() == 1)
        {
            DEBUG_UI("Talk off in Preview Mode\r\n");
            uiSetTalkOnOff();
            uiOsdDrawTalkBack(UI_OSD_CLEAR);
            OSTimeDly(UI_TALK_DLY_TIME);
            gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
            return;
        }
    }
#endif

    if (volumeflag != 0)
    {
        if ((WhichKey != UI_KEY_UP) && (WhichKey != UI_KEY_DOWN))
        {       
            volumeflag  = 4 ;
            return;
        }
    }

    switch(WhichKey)
    {
        case UI_KEY_REC:
            GMotionTrigger[sysRFRxInMainCHsel + MULTI_CHANNEL_LOCAL_MAX] = 0;
            uiMotionSecTime[sysRFRxInMainCHsel] = UI_MD_NOT_REC;
            if(gSchRecStop == 2)
                gSchRecStop = 1;

            if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
            {
                uiOsdDrawInsertSD(UI_OSD_DRAW);
                return;
            }

            if (MultiChannelGetCaptureVideoStatus(sysRFRxInMainCHsel + MULTI_CHANNEL_LOCAL_MAX) == 1)
            {
                uiCaptureVideoStopByChannel(sysRFRxInMainCHsel);
                
                #if(UI_BAT_SUPPORT)
                if (_uiBatType[sysRFRxInMainCHsel] == UI_CAM_NORMAL)
                #endif    
                {
                    uiCurRecStatus[sysRFRxInMainCHsel] = UI_REC_TYPE_NONE;
                    if (iconflag[UI_MENU_SETIDX_REC_MODE_CH1 + sysRFRxInMainCHsel] == UI_MENU_REC_MODE_MOTION)
                    {
                        uiCurRecStatus[sysRFRxInMainCHsel] = UI_REC_TYPE_MOTION;
                        VideoClipParameter[sysRFRxInMainCHsel + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                        VideoClipParameter[sysRFRxInMainCHsel + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_EVENT_MOTION_ENA;
                        uiCaptureVideoByChannel(sysRFRxInMainCHsel);
                        DEBUG_UI("Cam %d Start Motion\n",sysRFRxInMainCHsel);
                    }
                }
            }
            else
            {
                if (uiShowOSDTime[sysRFRxInMainCHsel] == 0)
                    return;
                
                #if(UI_BAT_SUPPORT)
                if (_uiBatType[sysRFRxInMainCHsel] == UI_CAM_NORMAL)
                #endif
                {
                    uiCurRecStatus[sysRFRxInMainCHsel] = UI_REC_TYPE_MANUAL;
                }
                if (MultiChannelGetCaptureVideoStatus(sysRFRxInMainCHsel + MULTI_CHANNEL_LOCAL_MAX) != 0)
                    uiCaptureVideoStopByChannel(sysRFRxInMainCHsel);
                VideoClipParameter[sysRFRxInMainCHsel + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                VideoClipParameter[sysRFRxInMainCHsel + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
                uiCaptureVideoByChannel(sysRFRxInMainCHsel);
                DEBUG_UI("Cam %d Start REC\n", sysRFRxInMainCHsel);
            }
            break;

        case UI_KEY_AREC:
            if (_uiBatType[sysRFRxInMainCHsel] == UI_CAM_NORMAL)
            {
                break;
            }
            
            if (gRfiu_Op_Sta[sysRFRxInMainCHsel] != RFIU_RX_STA_LINK_OK)
            {
                DEBUG_YELLOW("[Set Arec] Cam %d is brocken\n",sysRFRxInMainCHsel);
                break;
            }

            SetVal = 1;
            if (iconflag[UI_MENU_SETIDX_SET_AREC] & (0x01 << sysRFRxInMainCHsel))
            {
                SetVal = 0;
            }

            if (SetVal == 1) /* OPEN */
            {
                iconflag[UI_MENU_SETIDX_SET_AREC] |= (SetVal << sysRFRxInMainCHsel);
                osdDrawMotionPreview(UI_OSD_DRAW);
            }
            else
            {
                temp = (0x01 << sysRFRxInMainCHsel);
                iconflag[UI_MENU_SETIDX_SET_AREC] &= ~temp;
                osdDrawMotionPreview(UI_OSD_CLEAR);
                osdDrawMotionMsg(UI_OSD_DRAW);
            }
            GMotionTrigger[sysRFRxInMainCHsel+MULTI_CHANNEL_LOCAL_MAX] = 0;
            uiMotionSecTime[sysRFRxInMainCHsel] = UI_MD_NOT_REC;
            if(gSchRecStop == 2)
                gSchRecStop = 1;
            isPIRsenSent[sysRFRxInMainCHsel] = 0;
            Save_UI_Setting();
            uiFlowSetPir(sysRFRxInMainCHsel , 1);
            break;
            
        case UI_KEY_MENU:
            if(gSchRecStop == 1)
                gSchRecStop = 2;
            
            #if (UI_SUPPORT_TREE == 1)
                MyHandler.MenuMode = SETUP_MODE;
                lastMenuMode = VIDEO_MODE;
                //uiCaptureVideoStop();
                //uiSetRfDisplayMode(UI_MENU_RF_ENTER_SETUP);
                DEBUG_UI("change to set up mode\r\n");
                uiEnterMenu(UI_MENU_NODE_REC_SET);
                playbackflag = 0;
                uiMenuEnable=0;
                uiReturnPreview = UI_MENU_TO_PRV;
                uiFlowEnterMenuMode(SETUP_MODE);
            #else
                DEBUG_UI("Do not have Menu\r\n");
            #endif
            break;
            
        case UI_KEY_CH1:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_1);
            break;
        case UI_KEY_CH2:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_2);
            break;
        case UI_KEY_CH3:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_3);
            break;
        case UI_KEY_CH4:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_4);
            break;

        case UI_KEY_TALK:
            if (uiCheckRfTalkStatus()== 1)
            {
                DEBUG_UI("Talk off in Preview Mode\r\n");
                if (uiSetTalkOnOff() == 0)
                {
                    uiOsdDrawTalkBack(UI_OSD_CLEAR);
                    OSTimeDly(UI_TALK_DLY_TIME);
                    gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
                }
            }
            else if(uiCheckRfTalkStatus()== 0)
            {
                DEBUG_UI("Talk in Preview Mode\r\n");
                if (uiSetTalkOnOff() == 1)
                {
                    uiOsdDrawTalkBack(UI_OSD_DRAW);
                    gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 0);
                }
            }
            break;
            
        case UI_KEY_VOL: 
            uiOsdVolumeControl(VIDEO_MODE, UI_VALUE_CURRENT);
            break;    
            
        case UI_KEY_UP:
            if (volumeflag > 0)
                uiOsdVolumeControl(VIDEO_MODE, UI_VALUE_ADD);
            else
                uiFlowVideoMode(UI_KEY_MODE);
            break;

        case UI_KEY_DOWN:
            if (volumeflag > 0)
                uiOsdVolumeControl(VIDEO_MODE, UI_VALUE_SUBTRACT);
            else
                uiFlowVideoMode(UI_KEY_MODE);
            break;
            
        case UI_KEY_RIGHT:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_ADD);
            break; 
            
        case UI_KEY_LEFT:
            uiSetRfChangeChannel(UI_MENU_CHANNEL_SUB);
            break;

        #if(UI_BAT_SUPPORT)
        case UI_KEY_PLAY:
            if (ScanRf!=0)
                return;
            
            if (iconflag[UI_MENU_SETIDX_CH1_ON + sysRFRxInMainCHsel] == UI_MENU_SETTING_CAMERA_OFF)
                return;
            
            if ((_uiBatType[sysRFRxInMainCHsel] == UI_CAM_BATTERY) && (sysBatteryCam_isSleeping(sysRFRxInMainCHsel)))
            {
                sysStartBatteryCam(sysRFRxInMainCHsel);
            }
            else
            {
                uiFlowVideoMode(UI_KEY_MODE);
            }
            break;
        #endif    
    
        case UI_KEY_TVOUT_DET:
            if (sysTVOutOnFlag == SYS_OUTMODE_TV)
            {
                DEBUG_UI("PANNEL MODE\r\n");
                //gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT,GPIO_LEVEL_HI );
                sysSetOutputMode(SYS_OUTMODE_PANEL);
                gpioTimerCtrLed(LED_OFF);
            }
            else
            {   /* switch to TV-out */
                DEBUG_UI("TV MODE\r\n");
                //gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT,GPIO_LEVEL_LO );
                sysSetOutputMode(SYS_OUTMODE_TV);
                gpioTimerCtrLed(LED_ON);
            }
            break;

        case UI_KEY_MODE:
        case UI_KEY_ENTER:   //QUAD Mode       
            if (uiQuadDisplay == 0) /*single mode*/
            {
                DEBUG_UI("Only One Camera On can not Enter Quad Mode\r\n");
                return;
            }

            iconflag[UI_MENU_SETIDX_FULL_SCREEN]=UI_MENU_RF_QUAD; 
            DEBUG_UI("UI_MENU_RF_QUAD \n");
            
            MyHandler.MenuMode = QUAD_MODE;
            lastMenuMode = VIDEO_MODE;
            gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
            uiOsdDrawBlackAll(UI_OSD_DRAW);
            IduVideoEnable(0);
            uiSetRfDisplayMode(UI_MENU_RF_QUAD);
            uiOsdDrawBlackAll(UI_OSD_CLEAR);
            //osdDrawQuadIcon();
            uiOSDPreviewInit();
            #if(RFRX_QUAD_AUDIO_EN == 1)
            osdDrawQuadFrame(sysRFRxInMainCHsel+2,UI_OSD_DRAW);
            #endif
            uiEnterMenu(UI_MENU_NODE_QUAD_MODE);
            if (!sysTVOutOnFlag)
                gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
            break;
            
#if UI_CAMERA_ALARM_SUPPORT           
        case UI_KEY_ALARM:
            #if(UI_BAT_SUPPORT)
            if ((_uiBatType[sysRFRxInMainCHsel]==UI_CAM_BATTERY) && (sysBatteryCam_isSleeping(sysRFRxInMainCHsel) == true))
                return;
            #endif
            
            if (iconflag[UI_MENU_SETIDX_CH1_CA_ONOFF+sysRFRxInMainCHsel] == UI_MENU_CAMERA_ALARM_OFF)
            {
                uiFlowVideoMode(UI_KEY_MODE);
                return;
            }

            uiCheckAlarmManualSwitchStatus(sysRFRxInMainCHsel);
            /*避免執行Key連續發送*/
            OSTimeDly(UI_TOUCH_ACT_DELAY);
            break;
#endif

#if UI_LIGHT_SUPPORT
        case UI_KEY_LIGHT:
            #if(UI_BAT_SUPPORT)
            if ((_uiBatType[sysRFRxInMainCHsel]==UI_CAM_BATTERY) && (sysBatteryCam_isSleeping(sysRFRxInMainCHsel) == true))
                return;
            #endif
            
            if (iconflag[UI_MENU_SETIDX_CH1_LS_ONOFF+sysRFRxInMainCHsel] == UI_MENU_CAMERA_ALARM_OFF)
            {
                uiFlowVideoMode(UI_KEY_MODE);
                return;
            }
            
            uiCheckLightManualSwitchStatus(sysRFRxInMainCHsel);
            /*避免執行Key連續發送*/
            OSTimeDly(UI_TOUCH_ACT_DELAY);
            break;
#endif

        default:
            DEBUG_UI("Error Key %d In Video Mode\r\n",WhichKey);
            break;        
    }
}

void uiFlowPlaybackMode(u8 WhichKey)
{
    u32  playStatus;
        
    switch(WhichKey)
    {
        case UI_KEY_ENTER:   /*play or pause*/
            if(dcfPlaybackCurFile->fileType != DCF_FILE_TYPE_ASF)
            {
                DEBUG_UI("Not Asf File\r\n");
                return;
            }
            #if(NIC_SUPPORT)
            if (Remote_play== 1)
                return;
            #endif
            osdDrawClearRemoteMsg(UI_OSD_CLEAR);
            if(sysPlaybackVideoPause==1)    /*pause -> play*/
            {
                DEBUG_UI("UI playback pause -> play\r\n");
                curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
                uiOsdDrawPlaybackPlaySpeed();
                timerCountPause(1, 0);
                sysPlaybackVideoPause=0;
                osdDrawPlayIndicator(UI_PLAY_ICON_PLAY);
                break;
            }
            if(sysPlaybackVideoStart == 1)  /*play -> pause*/
            {
                if((VideoDuration-3) != (VideoNextPresentTime/1000000))
                {
                    if (curr_playback_speed != UI_PLAYBACK_SPEED_LEVEL/2)
                    {
                        DEBUG_UI("UI playback Speed return Normal\r\n");
                        curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
                        uiOsdDrawPlaybackPlaySpeed();
                        osdDrawPlayIndicator(UI_PLAY_ICON_PLAY);
                        break;
                    }
                    DEBUG_UI("UI playback play -> pause\r\n");
                    //timerCountPause(1, 1);
                    sysPlaybackVideoPause = 1;
                    OSTimeDly(3); // Delay for protect system
                    osdDrawPlayIndicator(UI_PLAY_ICON_PAUSE);
                    /*wait for pause finish*/
                    OSTimeDly(2);
                    break;
                }
                else
                {
                    /* playback finish, and re-play*/
                    uiPlaybackStop(GLB_DISA);
                }
            }
            DEBUG_UI("UI playback play....\r\n");
            curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
            uiOsdDrawPlaybackPlaySpeed();
            Iframe_flag = 0;  // 1: We just need I-frame 0: play whole MP4
            playbackflag = 2;
            uiMenuEnable = 0x41;
            //uiOsdDrawPlayTime(0, 0);
            uiReadVideoFile();
            //sysSetEvt(SYS_EVT_ReadFile, playbackflag);
            osdDrawPlayIndicator(UI_PLAY_ICON_PLAY);
            break;

        case UI_KEY_STOP:
            #if(NIC_SUPPORT)
            if (Remote_play == 1)
                return;
            #endif
            osdDrawClearRemoteMsg(UI_OSD_CLEAR);
            if(uiPlaybackStop(GLB_ENA) == 1)
            {
                curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
                uiOsdDrawPlaybackPlaySpeed();
                osdDrawPlayIndicator(100);
                //uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_CLEAN);
            }
            break;

        case UI_KEY_RIGHT: /*FF*/
            #if(NIC_SUPPORT)
            if (Remote_play == 1)
                return;
            #endif
            osdDrawClearRemoteMsg(UI_OSD_CLEAR);
            playStatus = uiCheckPlayback();
            if(playStatus == 0)
            {
                DEBUG_UI("Play next file\n");
                curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
                Iframe_flag = 0;  // 1: We just need I-frame 0: play whole MP4
                uiPlaybackMoveForward();
                //sysSetEvt(SYS_EVT_PLAYBACK_MOVE_FORWARD, 0);
            }
            else if ((playStatus & FLAGSYS_RDYSTAT_PLAY_START) && (Iframe_flag == 0))
            {
                DEBUG_UI("UI playback FF....\r\n");
                if (curr_playback_speed < UI_PLAYBACK_SPEED_LEVEL-1)
                    curr_playback_speed++;
                uiOsdDrawPlaybackPlaySpeed();
            }
            else
                DEBUG_UI("Playback not ready in playback mode key %d\n",WhichKey);
            break;

        case UI_KEY_LEFT:   /*REW*/
            #if(NIC_SUPPORT)
            if (Remote_play == 1)
                return;
            #endif
            osdDrawClearRemoteMsg(UI_OSD_CLEAR);
            playStatus = uiCheckPlayback();
            if(playStatus == 0)
            {
                DEBUG_UI("Play prev file\n");
                if(uiPlaybackStop(GLB_ENA)== 1)
                    osdDrawPlayIndicator(100);
                curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
                Iframe_flag=0;  // 1: We just need I-frame 0: play whole MP4
                uiPlaybackMoveBackward();
                //sysSetEvt(SYS_EVT_PLAYBACK_MOVE_BACKWARD, 0);
            }
            else if ((playStatus & FLAGSYS_RDYSTAT_PLAY_START) && (Iframe_flag == 0))
            {
                DEBUG_UI("UI playback REW....\r\n");
                if (curr_playback_speed > 0)
                    curr_playback_speed--;
                uiOsdDrawPlaybackPlaySpeed();
            }
            else
                DEBUG_UI("Playback not ready in playback mode key %d\n",WhichKey);
            break;


        case UI_KEY_MENU:
            #if(NIC_SUPPORT)
            if ((Remote_play == 0) && (Fileplaying == 1))
            {
                Fileplaying = 0;
                return;
            }
            #endif
            uiPlaybackStop(GLB_ENA);
            uiOsdDisableAll();
            iduLockVideoAllColor(0x80800000);
            iduMenuMode(PANNEL_X,PANNEL_Y, PANNEL_X);
            curr_playback_speed = UI_PLAYBACK_SPEED_LEVEL/2;
            MyHandler.MenuMode = PLAYBACK_MENU_MODE;
            PlayListDspType = UI_DSP_PLAY_LIST_FILE;
            uiEnterMenu(UI_MENU_NODE_SET_PLAYBACK);
            uiOsdDrawPlaybackMenu(UI_KEY_DELETE);
            iduUnlockVideoAllColor();
            DEBUG_UI("change to Playback list\r\n");
            break;

        case UI_KEY_UP:  /*vol +*/
            if((sysPlaybackVideoStart == 1)&&(sysPlaybackVideoPause==0)&&(Iframe_flag == 0))
                uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_ADD);
            break;

        case UI_KEY_DOWN:   /*vol -*/
            if((sysPlaybackVideoStart == 1)&&(sysPlaybackVideoPause==0)&&(Iframe_flag == 0))
                uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_SUBTRACT);
            break;

        default:
            DEBUG_UI("Error Key %d In Playback Mode\r\n",WhichKey);
            break;
    }
}

void uiFlowSetupToPreview(void)
{

    playbackflag = 0;
    uiMenuEnable=0;
    
    iduLockVideoAllColor(0x80800000);
    uiMenuOSDReset();
    iduPreviewMode(PANNEL_X,PANNEL_Y, PANNEL_X);

    IduVideo_ClearPKBuf(0);
    IduVideo_ClearPKBuf(2);
    #if ((Sensor_OPTION == Sensor_OV7725_YUV601) || (Sensor_OPTION == Sensor_OV7725_VGA)) //Lsk 090518 : for RDI project, sensor input 640*480
    if(sysTVOutOnFlag && TvOutMode==SYS_TV_OUT_PAL)
    {
        IduVideo_ClearPartPKBuf(0, 61440, 1);
        IduVideo_ClearPartPKBuf(675840,737280,1);
    }
    #endif

    iduSetVideoBuf0Addr(PKBuf0);

    if ((lastMenuMode == QUAD_MODE) && (uiQuadDisplay != 0))
    {
        MyHandler.MenuMode = QUAD_MODE;
        DEBUG_UI("change to QUAD mode\r\n");
        uiOsdDrawBlackAll(UI_OSD_DRAW);
    #if RFIU_SUPPORT
        uiSetRfDisplayMode(UI_MENU_RF_QUAD);
    #endif 
        uiOsdDrawBlackAll(UI_OSD_CLEAR);
        //osdDrawQuadIcon();
        uiOSDPreviewInit();
        uiEnterMenu(UI_MENU_NODE_QUAD_MODE);    
    }
    else /*Video Mode*/
    {
        MyHandler.MenuMode = VIDEO_MODE;
        DEBUG_UI("change to preview mode\r\n");
    #if RFIU_SUPPORT
        uiSetRfDisplayMode(UI_MENU_RF_FULL);
    #endif 
        uiEnterMenu(UI_MENU_NODE_PREVIEW_MODE);
    
        uiOSDPreviewInit();
    }

    if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
    {
        uiFlowCheckRecState();
    }
}

void uiFrowGoToLastNode()
{
    uiCurrNode = uiCurrNode->parent;
}

void uiFlowRunAction(void)
{
    s8 action;
    u8 set_val,i,tmpVal;
    u8 result, SetAct = 0;

    set_val = uiCurrNode->item.index;
    uiCurrNode->parent->child = uiCurrNode;
    uiCurrNode = uiCurrNode->parent;
    action = uiCurrNode->item.NodeData->Action_no;
    DEBUG_UI("Enter Run Action %d %d\r\n",action,set_val);

    switch(action)
    {
        case UI_MENU_SETIDX_DATE_TIME:
            set_val = 1;
            break;

        case UI_MENU_SETIDX_UPGRADE_FW:
        case UI_MENU_SETIDX_UPGRADE_FW_NET:
            DEBUG_UI("Upgrade %d\r\n", set_val);
            if (set_val == UI_MENU_UPDATE_NO)
            {                
                uiGraphDrawMenu();
                return;
            }
            
            if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
            {
                IduVideo_ClearPKBuf(0);
                uiGraphDrawMenu();
                return;
            }
            break;
            

        case UI_MENU_SETIDX_FORMAT:
            if (set_val == UI_MENU_FORMAT_NO)
            {
                if (MyHandler.MenuMode == GOTO_FORMAT_MODE)
                {
                    OSMboxPost(speciall_MboxEvt, "NO");
                }
                else
                {
                    if (MyHandler.MenuMode == SETUP_MODE)
                        uiGraphDrawMenu();
                }
                return;
            }

            if(sysGetStorageInserted(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
            {
                uiOsdDrawInit();
                uiOsdDrawInsertSD(UI_OSD_DRAW);
                OSTimeDly(40);
                uiOsdDrawInsertSD(UI_OSD_CLEAR);
                uiGraphDrawMenu();
                return;
            }

            if(Write_protet() && sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
            {
                IduVideo_ClearPKBuf(0);
                osdDrawProtect(1);
                uiOsdDisable(1);
                uiGraphDrawMenu();
                return;
            }
            break;

        case UI_MENU_SETIDX_PLAYBACK:
            if (set_val == UI_MENU_DELETE_YES)
            {
                uiFlowPlayback_Delete_File();
            }
            else
            {
                DEBUG_UI("change to playback menu mode\r\n");
                MyHandler.MenuMode = PLAYBACK_MENU_MODE;
                uiEnterMenu(UI_MENU_NODE_SET_PLAYBACK);
                uiOsdDrawPlaybackMenu(UI_KEY_DELETE);
            }
            return;

        case UI_MENU_SETIDX_PAIRING:
            gUiParing = 1;
            #if(UI_BAT_SUPPORT)
            sysSetBTCWakeStatus(set_val,SYS_BTC_WAKEUP_PAIR,TRUE);
            #endif

            tmpVal = iconflag[UI_MENU_SETIDX_CH1_ON+set_val];
            
            if (iconflag[UI_MENU_SETIDX_CH1_ON+set_val] == UI_MENU_SETTING_CAMERA_OFF)
            {
                DEBUG_UI("Pair Mode Set Cam %d ON\r\n",set_val);
                tmpVal = iconflag[UI_MENU_SETIDX_CH1_ON+set_val];
                iconflag[UI_MENU_SETIDX_CH1_ON+set_val] = UI_MENU_SETTING_CAMERA_ON;
                IduVideo_ClearPKBuf(0);
                uiMenuAction(UI_MENU_SETIDX_CH1_ON+set_val);
            }
            
            #if MULTI_CHANNEL_VIDEO_REC
            if (MultiChannelGetCaptureVideoStatus(set_val+MULTI_CHANNEL_LOCAL_MAX) != 0)
            {
                uiCaptureVideoStopByChannel(set_val);
            }
            #endif
            #if RFIU_SUPPORT
            result = uiOsdDrawPairInMenu(set_val);
            #endif
            DEBUG_UI("Pair Cam %d Mode to Preview %d\r\n",set_val,result);
            
            playbackflag = 0;
            uiMenuEnable=0;
            IduVideo_ClearPKBuf(0);
            #if ((Sensor_OPTION == Sensor_OV7725_YUV601) || (Sensor_OPTION == Sensor_OV7725_VGA)) //Lsk 090518 : for RDI project, sensor input 640*480
            if(sysTVOutOnFlag && TvOutMode==SYS_TV_OUT_PAL)
            {
                IduVideo_ClearPartPKBuf(0, 61440, 1);
                IduVideo_ClearPartPKBuf(675840,737280,1);
            }
            #endif
            iduSetVideoBuf0Addr(PKBuf0);
            //uiMenuEnterPreview(0);
            #if RFIU_SUPPORT

            if (result == 1)
            {
                #if UI_LIGHT_SUPPORT
                    for (i = 0; i < 7; i++)
                        uiSetRfLSTimer[set_val][i] = UI_SET_RF_BUSY;
                    uiSetRfLightSwitch[set_val] = UI_SET_RF_BUSY;
                #endif

                #if UI_CAMERA_ALARM_SUPPORT
                    for (i = 0; i < 7; i++)
                        uiSetRfAlarmTimer[set_val][i] = UI_SET_RF_BUSY;
                    uiSetRfAlarmSwitch[set_val] = UI_SET_RF_BUSY; 
                #endif
                uiSetTxTimeStamp[set_val] = UI_SET_RF_BUSY;
                //uiSynRfConfig(set_val);
                uiSetRfChangeChannel(UI_MENU_CHANNEL_1+set_val);
            }
            else
            {
                if (tmpVal == UI_MENU_SETTING_CAMERA_OFF)
                {
                    iconflag[UI_MENU_SETIDX_CH1_ON + set_val] = UI_MENU_SETTING_CAMERA_OFF;
                    uiMenuAction(UI_MENU_SETIDX_CH1_ON + set_val);
                }
                DEBUG_UI("Change to Original Channel\r\n");
            }
            #if(UI_BAT_SUPPORT)
            sysSetBTCWakeStatus(set_val, SYS_BTC_WAKEUP_PAIR, FALSE);
            #endif
            gUiParing = 0;
            Save_UI_Setting();
            MyHandler.MenuMode = VIDEO_MODE;
            uiFlowSetupToPreview();
            #endif
            return;

        case UI_MENU_SETIDX_MOTION_MASK:
            DEBUG_UI("Enter Mask Area Setting\r\n");
            MyHandler.MenuMode = SET_MASK_AREA;
            uiOsdDrawMaskArea(0);
            return;        


        case UI_MENU_SETIDX_TV_OUT:
            if (set_val == iconflag[action])
            {
                DEBUG_UI("TV_OUT Setting do not change\r\n");
                uiGraphDrawMenu();
                return;
            }
            
#if USB_HOST_MASS_SUPPORT
        case UI_MENU_SETIDX_HDD_REMOVE:
            if (set_val == UI_MENU_DEFAULT_NO)
            {
                uiGraphDrawMenu();
                return;
            }

            if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
            {
                uiOsdDrawInsertHDD(UI_OSD_DRAW);
                OSTimeDly(40);
                uiOsdDrawInsertHDD(UI_OSD_CLEAR);
                uiGraphDrawMenu();
                return;
            }
            else
            {
                if (sysGetStorageSel(SYS_I_STORAGE_MAIN) != SYS_V_STORAGE_USBMASS)
                {
                    osdDrawNotHDD(UI_OSD_DRAW);
                    OSTimeDly(40);
                    osdDrawNotHDD(UI_OSD_CLEAR);
                    uiGraphDrawMenu();
                    return;
                }
            }
			gUiDeviceRemove = 1;
            osdDrawHDDUninstallMsg(UI_OSD_DRAW);
            break;
#endif

        default:
            if (action < 0)
                return;
            break;
    }
    if (action < UI_MENU_SETIDX_LAST)
    {
        if (iconflag[action] == set_val)
        {
            if (SetAct == 0)
            {
                uiGraphDrawMenu();
                return;
            }
        }
        iconflag[action] = set_val;
    }    
    uiMenuAction(action);
    switch(action)
    {
        case UI_MENU_SETIDX_FORMAT:
            osdDrawRunFormat();
            iconflag[action] = UI_MENU_FORMAT_NO;
            if (MyHandler.MenuMode == GOTO_FORMAT_MODE)
                return;
            break;
            
        case UI_MENU_SETIDX_UPGRADE_FW:
        case UI_MENU_SETIDX_UPGRADE_FW_NET:
            osdDrawUpgradeFW();
            break;

        case UI_MENU_SETIDX_TV_OUT:
            /*enter to preview, do not need draw menu again*/
            return;
            
        case UI_MENU_SETIDX_NETWORK_STATUS:
            Save_UI_Setting();
            osdDrawSystemReboot();
			sysForceWDTtoReboot();
            uiCurrNode = uiCurrNode->parent;
            uiGraphDrawMenu();
            return;
            
#if USB_HOST_MASS_SUPPORT
        case UI_MENU_SETIDX_HDD_REMOVE:
            osdDrawHDDUninstallStatusMsg(UI_OSD_DRAW, removeHDDStatus);
            OSTimeDly(40);
            osdDrawHDDUninstallStatusMsg(UI_OSD_CLEAR, removeHDDStatus);
            gUiDeviceRemove = 0;
            break;
#endif

        default:
            break;
    }
        
    if (MyHandler.MenuMode == SETUP_MODE)
        uiGraphDrawMenu();
    
    Save_UI_Setting();
  
    
}

void uiFlowSetupMode(u8 WhichKey)
{
	u8 rowNum;

    switch(WhichKey)
    {
        case UI_KEY_LEFT:
            if(uiCurrNode->item.depth == 1)
				uiCurrNode = uiCurrNode->left;
			else
            {
                rowNum = uiCurrNode->item.index % UI_MENU_MAX_COLUMN;
                do
                {
                    uiCurrNode = uiCurrNode->left;
                }
                while((uiCurrNode->item.index % UI_MENU_MAX_COLUMN) != rowNum);
		    }
            uiGraphDrawMenu();
            break;

        case UI_KEY_RIGHT:
            if (TouchExtKey != -1)
            {
                while (uiCurrNode->item.index != TouchExtKey)
                {
                    DEBUG_UI("Touch %d search to right node %d\r\n",TouchExtKey, uiCurrNode->item.index);
                    uiCurrNode = uiCurrNode->right;
                }
            }
            else
            {
                if(uiCurrNode->item.depth == 1)
    				uiCurrNode = uiCurrNode->right;
    			else
                {
                    rowNum = uiCurrNode->item.index % UI_MENU_MAX_COLUMN;
                    do
                    {
                        uiCurrNode = uiCurrNode->right;
                    }
                    while((uiCurrNode->item.index % UI_MENU_MAX_COLUMN) != rowNum);
    		    }
            }
            uiGraphDrawMenu();
            break;

        case UI_KEY_UP:
            if(uiCurrNode->item.depth == 1)
                return;
            uiCurrNode = uiCurrNode->left;
            uiGraphDrawMenu();
            break;

        case UI_KEY_DOWN:
            if(uiCurrNode->item.depth == 1)
                return;
            uiCurrNode = uiCurrNode->right;
            uiGraphDrawMenu();
            break;
            
        case UI_KEY_ENTER:
            if (TouchExtKey != -1)
            {
                if (uiCurrNode->item.count <= TouchExtKey)
                    return;
                
                while (uiCurrNode->item.index != TouchExtKey)
                    uiCurrNode = uiCurrNode->right;
            }
            
            if (uiCurrNode->item.NodeData->Action_no == UI_MENU_SETIDX_PLAYBACK)
            {
                DEBUG_UI("change to playback menu mode\r\n");
                MyHandler.MenuMode = PLAYBACK_MENU_MODE;
                uiCurrNode = uiCurrNode->child;
                IduVideo_ClearPKBuf(0);
                uiGraphDrawMenu();
                #if UI_CALENDAR_SUPPORT
                uiGraphDrawPlaybackList(0, 0);
                #else
                uiOsdDrawPlaybackMenu(0);
                #endif
                break;
            }
            
            if (uiCurrNode->child == NULL)
            {
                uiFlowRunAction();
                break;
            }
            else if ((uiCurrNode->item.NodeData->Action_no == UI_MENU_SETIDX_UPGRADE_FW) ||
                    (uiCurrNode->item.NodeData->Action_no == UI_MENU_SETIDX_UPGRADE_FW_NET))
            {
                uiCurrNode = uiCurrNode->child->right;
                while(uiCurrNode->item.index != UI_MENU_UPDATE_NO)
                    uiCurrNode = uiCurrNode->right;
            }
            else
            {
                if ((uiCurrNode->item.NodeData->Action_no > 0) && 
                    (uiCurrNode->child->item.index != iconflag[uiCurrNode->item.NodeData->Action_no])&&
                    (uiCurrNode->item.NodeData->Action_no != UI_MENU_SETIDX_RESOLUTION_CH1)&&
                    (uiCurrNode->item.NodeData->Action_no != UI_MENU_SETIDX_CH1_ON)&&
                    (uiCurrNode->item.NodeData->Action_no != UI_MENU_SETIDX_BRIGHTNESS_CH1)&&
                    (uiCurrNode->item.NodeData->Action_no != UI_MENU_SETIDX_REC_MODE_CH1)&&
                    (uiCurrNode->item.NodeData->Action_no != UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1)&&
                    (uiCurrNode->item.NodeData->Action_no != UI_MENU_SETIDX_DATE_TIME)&&
                    (uiCurrNode->item.NodeData->Action_no != UI_MENU_SETIDX_CH1_LS_ONOFF)&&
                    (uiCurrNode->item.NodeData->Action_no != UI_MENU_SETIDX_CH1_CA_ONOFF)&&
                    (uiCurrNode->item.NodeData->Action_no < UI_MENU_SETIDX_LAST))
                {
                    /*get current setting*/
                    uiCurrNode = uiCurrNode->child->right;
                    while(uiCurrNode->item.index != iconflag[uiCurrNode->parent->item.NodeData->Action_no])
                        uiCurrNode = uiCurrNode->right;
                    
                    uiCurrNode->parent->child = uiCurrNode;
                }
                else
                {
                    uiCurrNode = uiCurrNode->child;
                }

            }
            
            if (uiCurrNode->item.NodeData->Action_no != UI_MENU_SETIDX_PLAYBACK)
                uiGraphDrawMenu();
            break;

        case UI_KEY_MENU:            
            /*to last page*/
            #if(FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
            splitmenu=0;
            #endif
            //if (uiCurrNode->parent != NULL)
            //DEBUG_UI("===> uiCurrNode->item.depth = %d\r\n",uiCurrNode->item.depth);
            if(uiCurrNode->item.depth > 1)
            {
                uiFrowGoToLastNode();
                uiGraphDrawMenu();
            }
            else
            {
                DEBUG_UI("@@Setup Mode change to preview mode\r\n");
                uiFlowSetupToPreview();
            }
            break;

        case UI_KEY_ENTER_PRV:
            uiFlowSetupToPreview();
            break;
            
        default:
            DEBUG_UI("Error Key %d In Setup Mode\r\n",WhichKey);
            break;
    }   
}

u8 uiCheckQuadTouchChannel(u8  key)
{
    u8  CamId = 0xFF, i;
    u8  left;

    if (uiQuadDisplay == 2)
    {
        if ((key == UI_KEY_UP) || (key == UI_KEY_DOWN))
            left = 1;
        else
            left = 0;
        for (i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
            {
                if (left == 1)
                    return i;
                else
                    left = 1;
            }
        }
    }
    else
    {
        if ((key == UI_KEY_UP) && (iconflag[UI_MENU_SETIDX_CH1_ON] == UI_MENU_SETTING_CAMERA_ON))
            CamId = 0;
        else if ((key == UI_KEY_RIGHT) && (iconflag[UI_MENU_SETIDX_CH2_ON] == UI_MENU_SETTING_CAMERA_ON))
            CamId = 1;
        else if ((key == UI_KEY_DOWN) && (iconflag[UI_MENU_SETIDX_CH3_ON] == UI_MENU_SETTING_CAMERA_ON))
            CamId = 2;
        else if ((key == UI_KEY_LEFT) && (iconflag[UI_MENU_SETIDX_CH4_ON] == UI_MENU_SETTING_CAMERA_ON))
            CamId = 3;
            
    }
    return CamId;
}

#if RFIU_SUPPORT
void uiFlowQuadSwitchChannel(u8 CamId)
{
    iconflag[UI_MENU_SETIDX_FULL_SCREEN]=UI_MENU_RF_FULL;
    DEBUG_UI("CHANNEL %d in Quad \n",CamId);
    gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
    uiOsdDrawBlackAll(UI_OSD_DRAW);
    MyHandler.MenuMode = VIDEO_MODE;
    lastMenuMode = QUAD_MODE;
    sysRFRxInMainCHsel = 0;
    uiSetRfChangeChannel(UI_MENU_CHANNEL_1+CamId);
    uiEnterMenu(UI_MENU_NODE_PREVIEW_MODE);
    uiOsdDrawBlackAll(UI_OSD_CLEAR);
    uiOSDPreviewInit();
    if (!sysTVOutOnFlag)
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
}
#endif

void uiFlowQuadMode(u8 WhichKey)
{
    u8 CamId,i,left,checkCam=0,allBroken=1;

#if RFIU_SUPPORT
    switch(WhichKey)
    {
    #if(RFRX_QUAD_AUDIO_EN == 1)
        case UI_KEY_RIGHT:
            uiSetRfChangeAudio_QuadMode(UI_MENU_CHANNEL_ADD);   // 使用這個function 會清掉OSD
            break;
            
        case UI_KEY_LEFT:
            uiSetRfChangeAudio_QuadMode(UI_MENU_CHANNEL_SUB);                 
            break;
            
        case UI_KEY_UP:
            uiOsdVolumeControl(QUAD_MODE, UI_VALUE_ADD);           
            break;
            
        case UI_KEY_DOWN:
            uiOsdVolumeControl(QUAD_MODE, UI_VALUE_SUBTRACT);
           
            break;
            
    #else
        case UI_KEY_UP:
        case UI_KEY_DOWN:
        case UI_KEY_LEFT:
        case UI_KEY_RIGHT:
            CamId = uiCheckQuadTouchChannel(WhichKey);
            if (CamId == 0xFF)
            {
                DEBUG_UI("Channel Off\r\n");
                break;
            }
            uiFlowQuadSwitchChannel(CamId);
            break;
    #endif
        
        case UI_KEY_ENTER:  /* VIDEO */
            iconflag[UI_MENU_SETIDX_FULL_SCREEN]=UI_MENU_RF_FULL; 
            DEBUG_UI("UI_MENU_RF_FULL \n");
            gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
            uiOsdDrawBlackAll(UI_OSD_DRAW);
            IduVideoEnable(0);
            uiSetRfDisplayMode(UI_MENU_RF_FULL);
            MyHandler.MenuMode = VIDEO_MODE;
            lastMenuMode = QUAD_MODE;
            uiEnterMenu(UI_MENU_NODE_PREVIEW_MODE);
            uiOsdDrawBlackAll(UI_OSD_CLEAR);
            uiOSDPreviewInit();
            if (!sysTVOutOnFlag)
                gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
            break;

        case UI_KEY_REC:
            for (i = 0; i < MULTI_CHANNEL_MAX; i++)
            {
                GMotionTrigger[i+MULTI_CHANNEL_LOCAL_MAX] = 0;
                uiMotionSecTime[i] = UI_MD_NOT_REC;
                if(gSchRecStop == 2)
                    gSchRecStop = 1;
            }

            if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
            {
                uiOsdDrawInsertSD(UI_OSD_DRAW);
                return;
            }

            checkCam = 0;
            for (i = 0; i < MULTI_CHANNEL_MAX; i++)//close recording status
            {
                if (MultiChannelGetCaptureVideoStatus(i + MULTI_CHANNEL_LOCAL_MAX) == 1)
                {
                    uiCaptureVideoStopByChannel(i);
                    
                    #if(UI_BAT_SUPPORT)
                    if (_uiBatType[i] == UI_CAM_NORMAL)
                    #endif
                    {
                        uiCurRecStatus[i] = UI_REC_TYPE_NONE;
                        if (iconflag[UI_MENU_SETIDX_REC_MODE_CH1 + i] == UI_MENU_REC_MODE_MOTION)
                        {
                            uiCurRecStatus[i] = UI_REC_TYPE_MOTION;
                            VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                            VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_EVENT_MOTION_ENA;
                            uiCaptureVideoByChannel(i);
                            DEBUG_UI("Cam %d Start Motion in Quad\r\n", i);
                        }
                    }                    
                    checkCam = 1;
                }
            }

            if (checkCam == 0)
            {
                for (i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if (uiShowOSDTime[i] == 0)
                        continue;
                    
                    DEBUG_UI("Cam %d Start REC in Quad\r\n", i);
                    
                    if (MultiChannelGetCaptureVideoStatus(i + MULTI_CHANNEL_LOCAL_MAX) != 0)
                        uiCaptureVideoStopByChannel(i);
                    
                    #if(UI_BAT_SUPPORT)
                    if (_uiBatType[i] == UI_CAM_NORMAL)
                    #endif
                    {
                        uiCurRecStatus[i] = UI_REC_TYPE_MANUAL;
                    }
                    VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                    VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
                    uiCaptureVideoByChannel(i);
                }
            }
            break;

        case UI_KEY_AREC:
            for (i = 0; i < MULTI_CHANNEL_MAX; i++)
            {
                GMotionTrigger[i+MULTI_CHANNEL_LOCAL_MAX] = 0;
                uiMotionSecTime[i] = UI_MD_NOT_REC;
                if(gSchRecStop == 2)
                    gSchRecStop = 1;
            }
            
            if(isMotionRec == UI_OSD_DRAW)
            {
                for (i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    #if(UI_BAT_SUPPORT)
                    if (_uiBatType[i]==UI_CAM_BATTERY)
                        continue;
                    #endif
                    
                    if (VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode & ASF_CAPTURE_EVENT_MOTION_ENA)
                    {
                        uiCurRecStatus[i]=UI_REC_TYPE_NONE;
                        uiCaptureVideoStopByChannel(i);
                        
                        if (iconflag[UI_MENU_SETIDX_REC_MODE_CH1+i] == UI_MENU_REC_MODE_MOTION)
                        {
                            uiCurRecStatus[i]=UI_REC_TYPE_MOTION;
                            VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_EVENT_MOTION_ENA;
                            uiCaptureVideoByChannel(i);
                            DEBUG_UI("Cam %d Start Motion in Quad\r\n", i);
                        }
                    }
                }
            }
            else
            {
                for (i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    #if(UI_BAT_SUPPORT)
                    if (_uiBatType[i]==UI_CAM_BATTERY)
                        continue;
                    #endif
                    
                    if (MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == 0)
                    {
                        uiCurRecStatus[i]=UI_REC_TYPE_MOTION;
                        VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_EVENT_MOTION_ENA;
                        uiCaptureVideoByChannel(i);
                        DEBUG_UI("Cam %d Start Motion in Quad\r\n", i);
                    }
                }
            }
            isMotionRec ^= 1 ;
            osdDrawMotionPreview(isMotionRec);
            break;
            
        #if(UI_BAT_SUPPORT)
        case UI_KEY_PLAY:
            checkCam = TouchExtKey;
            if (uiQuadDisplay == 2) /*dual mode*/
            {
                if (!((UiGetTouchY >= 140) && (UiGetTouchY <= 300)))
                {
                    if (TouchExtKey == 0)
                    {
                        uiFlowQuadMode(UI_KEY_UP);
                    }
                    else
                    {
                        uiFlowQuadMode(UI_KEY_LEFT);
                    }
                    break;
                }
                
                for (i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
                    {
                        if (checkCam == 0)
                        {
                            CamId = i;
                            break;
                        }
                        else
                            checkCam = 0;
                    }
                }
            }
            else
            {
                if (TouchExtKey == 0)
                {
                    if (UiGetTouchY >= 300)
                    {
                        if ((UiGetTouchY >= 300) && (UiGetTouchY <= 400))
                        {
                            
                            CamId = 2;
                        }
                        else
                        {
                            uiFlowQuadMode(UI_KEY_DOWN);
                            break;
                        }
                    }
                    else
                    {
                        if ((UiGetTouchY >= 60) && (UiGetTouchY <= 160))
                        {
                            
                            CamId = 0;
                        }
                        else
                        {
                            uiFlowQuadMode(UI_KEY_UP);
                            break;
                        }
                    }
                }
                else
                {
                    if (UiGetTouchY >= 300)
                    {
                        if ((UiGetTouchY >= 300) && (UiGetTouchY <= 400))
                        {
                            CamId = 3;
                        }
                        else
                        {
                            uiFlowQuadMode(UI_KEY_LEFT);
                            break;
                        }
                    }
                    else
                    {
                        if ((UiGetTouchY >= 60) && (UiGetTouchY <= 160))
                        {
                            CamId = 1;
                        }
                        else
                        {
                            uiFlowQuadMode(UI_KEY_RIGHT);
                            break;
                        }
                    }  
                }
            }

            if (iconflag[UI_MENU_SETIDX_CH1_ON + CamId] == UI_MENU_SETTING_CAMERA_OFF)
                return;

            if ((_uiBatType[CamId]==UI_CAM_BATTERY) && (sysBatteryCam_isSleeping(CamId)))
            {
                sysStartBatteryCam(CamId);
            }
            else
            {
                uiFlowQuadSwitchChannel(CamId);
            }
            break;
        #endif
        
        case UI_KEY_MENU:
            if(gSchRecStop == 1)
                gSchRecStop = 2;
            
            MyHandler.MenuMode = SETUP_MODE;
            lastMenuMode = QUAD_MODE;
            //uiCaptureVideoStop();
            //uiSetRfDisplayMode(UI_MENU_RF_ENTER_SETUP);
            DEBUG_UI("Quad change to set up mode\r\n");
            uiEnterMenu(UI_MENU_NODE_REC_SET);
            playbackflag = 0;
            uiMenuEnable=0;
            uiReturnPreview = UI_MENU_TO_PRV;
            uiFlowEnterMenuMode(SETUP_MODE);
            break;
            
        case UI_KEY_TVOUT_DET:
            if (sysTVOutOnFlag == SYS_OUTMODE_TV)
            {
                DEBUG_UI("PANNEL MODE\r\n");
                //gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT,GPIO_LEVEL_HI );
                sysSetOutputMode(SYS_OUTMODE_PANEL);
                gpioTimerCtrLed(LED_OFF);
            }
            else
            {   /* switch to TV-out */
                DEBUG_UI("TV MODE\r\n");
                //gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT,GPIO_LEVEL_LO );
                sysSetOutputMode(SYS_OUTMODE_TV);
                gpioTimerCtrLed(LED_ON);
            }
            break;
            
        case UI_KEY_RF_CH:        
            uiFlowQuadMode(UI_KEY_ENTER);
            break;

#if UI_CAMERA_ALARM_SUPPORT           
        case UI_KEY_ALARM: 
            if (uiQuadDisplay == 2) /*dual mode*/
            {
                if ((TouchExtKey == 0) || (TouchExtKey == 2))
                    left = 1;
                else
                    left = 0;

                for (i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
                    {
                        if (left == 1)
                        {
                            CamId = i;
                            break;
                        }
                        else
                        {
                            left = 1;
                        }
                    }
                }
            }
            else if (uiQuadDisplay == 1) /*quad mode*/
            {
                if (iconflag[UI_MENU_SETIDX_CH1_ON+TouchExtKey] == UI_MENU_SETTING_CAMERA_ON)
                    CamId= TouchExtKey;
                else    
                {
                    uiFlowQuadSwitchChannel(CamId);
                    return;

                }
            }
            
            if (iconflag[UI_MENU_SETIDX_CH1_CA_ONOFF+CamId] == UI_MENU_CAMERA_ALARM_OFF)
            {
                uiFlowQuadSwitchChannel(CamId);
                return;

            }

            #if(UI_BAT_SUPPORT)
            if ((_uiBatType[CamId]==UI_CAM_BATTERY) && (sysBatteryCam_isSleeping(CamId) == true))
                return;
            #endif
            
            uiCheckAlarmManualSwitchStatus(CamId);
            /*避免執行Key連續發送*/
            OSTimeDly(UI_TOUCH_ACT_DELAY);
            break;
#endif

#if UI_LIGHT_SUPPORT
        case UI_KEY_LIGHT:
            if (uiQuadDisplay == 2) /*dual mode*/
            {
                if ((TouchExtKey == 0) || (TouchExtKey == 2))
                    left = 1;
                else
                    left = 0;

                for (i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
                    {
                        if (left == 1)
                        {
                            CamId = i;
                            break;
                        }
                        else
                        {
                            left = 1;
                        }
                    }
                }
            }
            else if (uiQuadDisplay == 1) /*quad mode*/
            {
                if (iconflag[UI_MENU_SETIDX_CH1_ON+TouchExtKey] == UI_MENU_SETTING_CAMERA_ON)
                    CamId = TouchExtKey;
                else    
                {
                    uiFlowQuadSwitchChannel(CamId);
                    return;

                }
            }
            
            if (iconflag[UI_MENU_SETIDX_CH1_LS_ONOFF+CamId] == UI_MENU_CAMERA_ALARM_OFF)
            {
                uiFlowQuadSwitchChannel(CamId);
                return;

            }

            #if(UI_BAT_SUPPORT)
            if ((_uiBatType[CamId]==UI_CAM_BATTERY) && (sysBatteryCam_isSleeping(CamId) == true))
                return;
            #endif
            
            uiCheckLightManualSwitchStatus(CamId);
            /*避免執行Key連續發送*/
            OSTimeDly(UI_TOUCH_ACT_DELAY);
            break;
#endif

        default:
            DEBUG_UI("Error Key %d In Quad Mode\r\n",WhichKey);
            break;
    }
#endif
}

void uiFlowPlaybackListMode(u8 WhichKey)
{
    switch(WhichKey)
    {
        case UI_KEY_ENTER:
        case UI_KEY_UP:
        case UI_KEY_DOWN:
        case UI_KEY_RIGHT:
        case UI_KEY_LEFT:
        case UI_KEY_MENU:
            if (PlayListDspType == UI_DSP_PLAY_LIST_DOOR_SELECT)
                uiOsdDrawPlaybackMenuDoor(MyHandler.WhichKey);
            #if UI_CALENDAR_SUPPORT
            else if (PlayListDspType == UI_DSP_PLAY_LIST_DIR)
                uiGraphDrawPlaybackList(WhichKey, TouchExtKey);//CALENDAR
            #endif
            else
                uiOsdDrawPlaybackMenu(MyHandler.WhichKey);//LIST
            break;

        case UI_KEY_REC:
        case UI_KEY_DELETE:
        case UI_KEY_RF_QUAD: 
            if (PlayListDspType == UI_DSP_PLAY_LIST_DIR)
                break;

            if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
                break;
            
            if ((PlayListDspType == UI_DSP_PLAY_LIST_FILE)&& (dcfGetCurDirFileCount() == 0))
                break;
            
            else if ((PlayListDspType == UI_DSP_PLAY_LIST_DIR)&& (dcfGetTotalDirCount() == 0))
                break;
            
            MyHandler.MenuMode = SET_CONFIRM;
            uiEnterMenu(UI_MENU_NODE_DELETE_YES);
            uiOsdDrawConfirmSelect(0, 2);
            DEBUG_UI("change to Set UP mode for delete\r\n");
            break;

        case UI_KEY_PLAY:
            uiFlowSetupToPreview();
            DEBUG_UI("Playback list change to preview\r\n");
            break;

        default:
            DEBUG_UI("Error Key %d In Playback List Mode\r\n",WhichKey);
            break;
    }
}

void uiFlowMaskAreaMode(u8 WhichKey, s8 ExtKey)
{
    switch(WhichKey)
    {
        case UI_KEY_MENU:   /*leave*/
            DEBUG_UI("return to set up mode\r\n");
            playbackflag = 0;
            uiMenuEnable=0;
            uiCurrNode = uiCurrNode->parent;
            uiOsdDrawMaskArea(MyHandler.WhichKey);
            uiMenuAction(uiCurrNode->item.NodeData->Action_no);
            uiFlowEnterMenuMode(SETUP_MODE);
            Save_UI_Setting();
            break;
            
        default:
            //uiOsdDrawMaskAreaSetting(WhichKey, uiSetMaskAreaCam);
            break;            
    }
}

void uiEventHandler(void)
{
    DEBUG_UI("uiEventHandler Mode %d Key %d TouchExtKey %d\n",MyHandler.MenuMode, MyHandler.WhichKey, TouchExtKey);
    switch(MyHandler.MenuMode)
    {
        case VIDEO_MODE:
            uiFlowVideoMode(MyHandler.WhichKey);
            break;
            
        case PLAYBACK_MODE:
            uiFlowPlaybackMode(MyHandler.WhichKey);
            break;
            
        case SETUP_MODE:
            if (MyHandler.WhichKey == UI_KEY_ENTER_PRV)
            {
                if (gUiDeviceRemove == 1)
                    break;
                
                gUiLeaveKaypad = 0;
                uiFlowSetupToPreview();
                break;
            }             
            if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_CH1_ON) 
                uiGraphDrawCameraOnOff(MyHandler.WhichKey);
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_CH1_LS_ONOFF) 
                uiGraphDrawCameraAlarmOnOff(MyHandler.WhichKey);
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_CH1_CA_ONOFF) 
                uiGraphDrawCameraAlarmOnOff(MyHandler.WhichKey);
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_RESOLUTION_CH1) 
                uiGraphDrawResolution(MyHandler.WhichKey);
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_BRIGHTNESS_CH1) 
                uiGraphDrawBrightness(MyHandler.WhichKey);

            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_REC_MODE_CH1)
                uiGraphDrawRECMode(MyHandler.WhichKey);
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1) 
                uiGraphDrawMotionSensitivity(MyHandler.WhichKey);
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_SCHEDULED) 
                uiGraphDrawScheduled(MyHandler.WhichKey);
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_SCHEDULED_SET) 
                uiGraphDrawScheduledSetting(MyHandler.WhichKey);
            
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_DATE_TIME) 
                uiGraphDrawDateTime(MyHandler.WhichKey);
        #if SET_NTPTIME_TO_RTC
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_NTP) 
                uiGraphDrawTimezone(MyHandler.WhichKey);
        #endif
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_CARDINFO) 
                uiGraphDrawCardInfo(MyHandler.WhichKey);
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_VERSION_INFO) 
                uiGraphDrawVersionInfo(MyHandler.WhichKey);
        #if (NIC_SUPPORT)
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_ST_IP_SET) //DHCP
                uiGraphDrawStaticIP(MyHandler.WhichKey);
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_NETWORK_INFO) 
                uiGraphDrawNetworkInfo(MyHandler.WhichKey);
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_NETWORK_KEYPAD) //STATIC IP
                uiGraphDrawKeypad(MyHandler.WhichKey);
        #endif
            else if(uiCurrNode->parent->item.NodeData->Action_no == UI_MENU_SETIDX_APP_INFO) 
                uiGraphDrawAPPInfo(MyHandler.WhichKey);
            else
                uiFlowSetupMode(MyHandler.WhichKey);
                   
            break;

        case SET_CONFIRM:
            if (MyHandler.WhichKey == UI_KEY_ENTER_PRV)
            {
                uiFlowSetupToPreview();
                break;
            }
            uiOsdDrawConfirmSelect(MyHandler.WhichKey, 2);
            break;
            
        case PLAYBACK_MENU_MODE:
            uiFlowPlaybackListMode(MyHandler.WhichKey);
            break;

        case SET_MASK_AREA:
            uiFlowMaskAreaMode(MyHandler.WhichKey,TouchExtKey);
            break;

        case SET_NUMBER_MODE:
            break;

        case GOTO_FORMAT_MODE:
            switch(MyHandler.WhichKey)
            {
                case UI_KEY_MENU:
                case UI_KEY_ENTER_PRV:
                    OSMboxPost(speciall_MboxEvt, "NO");
                    break;
                    
                case UI_KEY_RIGHT:
                    while (uiCurrNode->item.index != TouchExtKey)
                    {
                        DEBUG_UI("Touch %d search to right node %d\r\n",TouchExtKey, uiCurrNode->item.index);
                        uiCurrNode = uiCurrNode->right;
                    }
                    uiGraphDrawMenu();
                    break;

                case UI_KEY_ENTER:
                    uiFlowRunAction();
                    break;
            }
            break;
            
        case QUAD_MODE:
            uiFlowQuadMode(MyHandler.WhichKey);
            break;
            
        default:
            return;
    }

}

u8 uiChangeResolution(u8 camId)
{
    u8 result = FALSE;

    switch (iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+camId])
    {
        case UI_MENU_SETTING_RESOLUTION_HD:
            if((gRfiuUnitCntl[camId].TX_PicWidth != 1280) || (gRfiuUnitCntl[camId].TX_PicHeight != 720))
                result = TRUE;
            break;

        case UI_MENU_SETTING_RESOLUTION_QHD:
            if((gRfiuUnitCntl[camId].TX_PicWidth != 640) || (gRfiuUnitCntl[camId].TX_PicHeight != 480))
                result = TRUE;
            break;

        case UI_MENU_SETTING_RESOLUTION_QVGA:
            if((gRfiuUnitCntl[camId].TX_PicWidth != 320) || (gRfiuUnitCntl[camId].TX_PicHeight != 240))
                result = TRUE;
            break;
            
        case UI_MENU_SETTING_RESOLUTION_1920x1088:
            if((gRfiuUnitCntl[camId].TX_PicWidth != 1920) || ((gRfiuUnitCntl[camId].TX_PicHeight != 1072)&&(gRfiuUnitCntl[camId].TX_PicHeight != 1088)))
                result = TRUE;
            break;
    }
    
    return result;

}

#if(NIC_SUPPORT)
void uiFlowCheckP2PMode(void)
{
    u8 i;
    u8  uartCmd[16];

    if (uiP2PRestoreCfgTime == 10)
    {
        uiP2PMode = 0;
        prev_P2pVideoQuality=2;
        rfiuRX_OpMode = rfiuRX_OpMode & (~RFIU_RX_OPMODE_P2P);
        sprintf((char*)uartCmd,"MODE %d 0", rfiuRX_OpMode);
        ClearP2PConnection();
        for (i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            if (uiChangeResolution(i) == TRUE)
            {
                #if UI_BAT_SUPPORT
                if ((gRfiuUnitCntl[i].RFpara.BateryCam_support == 1) && (sysBatteryCam_isSleeping(i) == TRUE))
                {
                    sysStartBatteryCam(i);
                }
                #endif
                uiMenuAction(UI_MENU_SETIDX_RESOLUTION_CH1+i);
            }            
            rfiu_RXCMD_Enc(uartCmd,i);
        }
    
        if( (rfiuRX_OpMode & RFIU_RX_OPMODE_QUAD) && (sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR) )
        {
            Idu_ClearBuf(6);
            #if RFRX_HALF_MODE_SUPPORT   
                if(rfiuRX_CamOnOff_Num <= 2)
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT+RFRX_HALF_MODE_SHIFT*2,RF_RX_2DISP_WIDTH*2);
                else
            #endif
                  iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
          

          #if TV_DISP_BY_IDU
    	    if(sysTVOutOnFlag) 
    	    {
    	    #if(TV_DISP_BY_TV_INTR)
    	        tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
    	    #else
    	        tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control * 
    	    #endif
    	    }
    		else
    	    {
    	        IduIntCtrl |= IDU_FTCINT_ENA;
    		}
    	  #endif
            OSTimeDly(10);
            osdDrawQuadIcon();
        }

    }
    if (uiP2PRestoreCfgTime != 0)
        uiP2PRestoreCfgTime--;
}
#endif
/*
 *********************************************************************************************************
 * Called by UI in other file
 *********************************************************************************************************
 */

void uiSetTVOutXY(u8 mode)
{
    if(mode == UI_MENU_SETTING_TV_OUT_PAL)
    {
        Display_X = TVOUT_X;
        Display_Y = TVOUT_Y_PAL;
        osdYShift = 76;
        OSDIconMidY = OSDDispHeight[sysTVOutOnFlag]/2;
        OSDIconEndY = OSDDispHeight[sysTVOutOnFlag];
        PbkOSD1 = 80;
        PbkOSD2 = 180;
    }
    else if(mode == UI_MENU_SETTING_TV_OUT_NTSC)//NTSC
    {
        Display_X = TVOUT_X;
        Display_Y = TVOUT_Y_NTSC;
        osdYShift = 0;
        OSDIconMidY = TVOUT_Y_NTSC/2;
        OSDIconEndY = TVOUT_Y_NTSC;
        PbkOSD1 = 80;
        PbkOSD2 = 180;
    }
    else    /*Panel*/
    {
        Display_X = PANNEL_X;
        Display_Y = PANNEL_Y;
        osdYShift = 0;
        OSDIconMidY = OSDDispHeight[sysTVOutOnFlag]/2;
        OSDIconEndY = OSDDispHeight[sysTVOutOnFlag];
        PbkOSD1 = Display_Y/3;
        PbkOSD2 = Display_Y*3/4;
    }
    OSDIconEndX = OSDDispWidth[sysTVOutOnFlag];
    OSDIconMidX = OSDDispWidth[sysTVOutOnFlag]/2;
    PbkOSD1High = PbkOSD2-PbkOSD1;
    PbkOSD2High = OSDIconEndY - PbkOSD2;
}

void uiFlowSdCardMode(void)
{
    u8  level;

    switch(MyHandler.MenuMode)
    {
        case VIDEO_MODE:
        case QUAD_MODE:
            level = sysGetStorageStatus(SYS_I_STORAGE_MAIN);
            if (level == SYS_V_STORAGE_NREADY)
                osdDrawSDIcon(UI_OSD_CLEAR);
            else
                osdDrawSDIcon(UI_OSD_DRAW);
            uiOsdDrawStorageNReady(UI_OSD_CLEAR);
            uiOsdDrawInsertSD(UI_OSD_CLEAR);
            uiOsdDrawSDCardFail(UI_OSD_CLEAR);
            uiFlowCheckRecState();
            break;

        case PLAYBACK_MODE:
            uiPlaybackStop(GLB_ENA);
            uiFlowSetupToPreview();
            break;

        case SETUP_MODE:
        case SET_NETWORK:
        case SET_DATETIME_MODE:
        case GOTO_FORMAT_MODE:
            if ((gUiDeviceRemove == 0) && (gUiParing == 0))
            {
                MyHandler.MenuMode = VIDEO_MODE;
                uiFlowSetupToPreview();
            }
            break;

        case PLAYBACK_MENU_MODE:
            uiFlowPlaybackListMode(UI_KEY_PLAY);
            break;

        case SET_MASK_AREA:
            MyHandler.MenuMode = VIDEO_MODE;
            uiOSDPreviewInit();
            break;

        default:
            DEBUG_UI("SD Card key in Mode %d not ready\n",MyHandler.MenuMode);
            break;

    }
}

s32 uiKeyParse(u8 key)
{
	void* msg;
    u8 err;
    u8 uartCmd[20];
    u8 level;
    u8 ui_chack_SD;
    
    if((Main_Init_Ready == 0)&& key!=UI_KEY_SDCD)
    {    // show busy message
        DEBUG_UI("Busy\n");
        return 0;
    }

    gpioGetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, &level);
    if(level == GPIO_LEVEL_LO)
        key=UI_KEY_LCD_BL;

    if (uiCheckPower == FALSE)
    {
        openPower = FALSE;
        uiCloPowSec = 0;
        uiCheckPower = TRUE;
    }
    
    if (uiReturnPreview != 0)
        uiReturnPreview = UI_MENU_TO_PRV;
    
    DEBUG_UI("uiKeyParse get key %d\n",key);   
    switch (key)
    {
        case UI_KEY_UP:
        case UI_KEY_LEFT:
        case UI_KEY_DOWN:
        case UI_KEY_RIGHT:
        case UI_KEY_ENTER:
        case UI_KEY_DELETE:
        case UI_KEY_PLAY:
        case UI_KEY_MENU:
        case UI_KEY_MODE:
        case UI_KEY_STOP:
        case UI_KEY_RF_QUAD:
        case UI_KEY_RF_CH:
        case UI_KEY_TALK:
        case UI_KEY_PARK:
        case UI_KEY_REC:
        case UI_KEY_FWUP:
        case UI_KEY_FORMAT:
        case UI_KEY_RESET:
        case UI_KEY_BUZZER:
        case UI_KEY_CH1:
        case UI_KEY_CH2:
        case UI_KEY_CH3:
        case UI_KEY_CH4:
        case UI_KEY_TVOUT_DET:
	    case UI_KEY_MONITOR:
	    case UI_KEY_UNLOCK:
        case UI_KEY_STANDBY:
        case UI_KEY_TALK_OFF:
        case UI_KEY_MAIN:
        case UI_KEY_GOTO:
        case UI_KEY_Gate:
        case UI_KEY_LONG_MONITOR:
        case UI_KEY_ENTER_PRV:
        case UI_KEY_LIGHT:
        case UI_KEY_ALARM:
        case UI_KEY_VOL:            
        case UI_KEY_AREC:            
            MyHandler.WhichKey = key;
            uiEventHandler();
            break;

#if MASS_STORAGE_INSERT_SHOW
        case UI_KEY_USB_INSERT:
            osdDrawSDIcon(UI_OSD_CLEAR);
            uiOsdDrawStorageNReady(UI_OSD_DRAW);
            break;
#endif

#if SD_TASK_INSTALL_FLOW_SUPPORT 
        case UI_KEY_DEVCD:
        	if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
        	{
        		uiCaptureVideoStop();
        		uiPlaybackStop(GLB_DISA);
        		MemoryFullFlag = FALSE;
        		//DEBUG_GREEN("PASS\n");
        	}
        	else
        	{
        		//DEBUG_GREEN("GOTO_FORMAT_MODE: %#d\n", MyHandler.MenuMode);
        		if((MyHandler.MenuMode == GOTO_FORMAT_MODE))
        		{
        			OSMboxPost(speciall_MboxEvt, "NO");					
        			uiFlowSetupToPreview();
        		}
        		//DEBUG_GREEN("FAIL: %#d\n", MyHandler.MenuMode);
        	}
        	sysUnlockMountSeq();
        	break;
#endif        	

        case UI_KEY_UART:
            msg = OSMboxPend(uart_MboxEvt, 20, &err);
            uiCmdPareCmd(msg);
            uiEventHandler();
            break;

#if RFIU_SUPPORT
    #if (MR8600_DEMO_USE_PKEY == 1)
        case UI_KEY_RF_DUAL:
            uiSetRfDisplayMode(UI_MENU_RF_DUAL);
            break;
	#else
        case UI_KEY_RF_PAIR:

            #if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1 ) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2 ) )
            uiOsdTXWaitPair(0);
            #else
            #if (UI_PREVIEW_OSD == 1)
                uiOsdDrawPair(0);
            #else
                rfiu_PAIR_Linit(0);
            #endif
            #endif
            break;

        case UI_KEY_RF_PAIR1:
            #if (UI_PREVIEW_OSD == 1)
                uiOsdDrawPair(1);
            #else
                rfiu_PAIR_Linit(1);
            #endif
    #endif

#endif

        case UI_KEY_LCD_BL:
            gpioGetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, &level);
            if(level == GPIO_LEVEL_LO)
            {
                gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_HI);
                uiCheckPower = TRUE;
            }
            else
            {
                gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_LO);
                uiCheckPower = FALSE;
            }
            break;

        default:
            return 0;
    }
    TouchExtKey = -1;
    return 1;
}

/*when device power on, some value will always return to default*/
void uiMenuSetBootSetting(void)
{
    u8  i,j;

    DEBUG_UI("uiMenuSetBootSetting\n");
    
#if (RFIU_SUPPORT)
    for (i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        UI_CFG_RES[i] = iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i];
        UI_TMP_RES[i] = UI_CFG_RES[i];
    }

#endif

#if(NIC_SUPPORT)
    memcpy(&StartUINetInfo, &UINetInfo,sizeof(UI_NET_INFO));
#endif
    memcpy ((void *)start_iconflag,(void *)iconflag,  UIACTIONNUM);
    memcpy (&start_sysConfig, &sysConfig, sizeof(SYS_CONFIG_SETTING));
    iconflag[UI_MENU_SETIDX_FORMAT] = UI_MENU_FORMAT_NO;
    memcpy(&uiStartMaskArea[0][0], &uiMaskArea[0][0], MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN);

#if (UI_SUPPORT_TREE == 1)
    uiGraphGetImageID();
#endif

    for(i=0;i<UIACTIONNUM;i++)
        uiMenuAction(i);
    uiMenuAction(UI_MENU_SETIDX_TIMESTAMP);
        
    for ( i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        #if UI_LIGHT_SUPPORT
            for (j = 0; j < 7; j++)
                uiSetRfLSTimer[i][j] = UI_SET_RF_BUSY;
            uiSetRfLightSwitch[i] = UI_SET_RF_BUSY;
            uiSetRfLightState[i] = UI_SET_RF_BUSY;
        #endif

        #if UI_CAMERA_ALARM_SUPPORT
            for (j = 0; j < 7; j++)
                uiSetRfAlarmTimer[i][j] = UI_SET_RF_BUSY;
            uiSetRfAlarmSwitch[i] = UI_SET_RF_BUSY; 
            uiSetRfAlarmState[i] = UI_SET_RF_BUSY;
        #endif
        uiSetTxTimeStamp[i] = UI_SET_RF_BUSY;
    }

}

void uiWaitMainInitReady(void)
{
    u8  err,i;
    u32 waitFlag;

    waitFlag = OSFlagPend(gUiStateFlagGrp, (FLAGUI_MAIN_INIT_READY|FLAGUI_SD_GOTO_FORMAT), OS_FLAG_WAIT_SET_ANY, OS_IPC_WAIT_FOREVER, &err);
    if(waitFlag & FLAGUI_MAIN_INIT_READY)
    {
        #if (UI_RX_PWRON_QUAD_ENA == 0)
            MyHandler.MenuMode = VIDEO_MODE;
            #if (SUPPORT_TOUCH == 1)
                uiEnterMenu(UI_MENU_NODE_PREVIEW_MODE);
            #endif

            #if RFIU_SUPPORT
            if (uiSetRfChangeChannel(UI_MENU_CHANNEL_SCAN) == 1)
                ScanRf = 0;
            else
                ScanRf = 10;
            #endif

        #else
            if (rfiuRX_CamOnOff_Num < 2)
            {
                MyHandler.MenuMode = VIDEO_MODE;
                #if (SUPPORT_TOUCH == 1)
                    uiEnterMenu(UI_MENU_NODE_PREVIEW_MODE);
                #endif
            }
            else
            {
                MyHandler.MenuMode = QUAD_MODE;
                #if (SUPPORT_TOUCH == 1)
                    uiEnterMenu(UI_MENU_NODE_QUAD_MODE);
                #endif
                #if 0 //(AUDIO_DEVICE== AUDIO_IIS_WM8940)
                    WM8940_SpeakerMute();
                #endif
            }        
        #endif
        
        for (i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            #if UI_BAT_SUPPORT
            uiCurRecStatus[i] = UI_REC_TYPE_PIR; 
            if (_uiBatType[i] == UI_CAM_NORMAL)
            #endif                
            {
                if ((iconflag[UI_MENU_SETIDX_REC_MODE_CH1+i] == UI_MENU_REC_MODE_MANUAL) || (iconflag[UI_MENU_SETIDX_REC_MODE_CH1+i] == UI_MENU_REC_MODE_MOTION))
                {
                    uiCurRecStatus[i] = UI_REC_TYPE_MOTION; 
                    if (MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == UI_REC_STATUS_NONE)
                    {
                        VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_EVENT_MOTION_ENA;
                        uiCaptureVideoByChannel(i);
                    }
                }
            }
        }

        #if (UI_RX_PWRON_QUAD_ENA == 1)
        uiOSDPreviewInit();
        #endif
    }
}

void uiEnterTVMode(void)
{
    sysTVOutOnFlag=1;
    if(sysTVinFormat  == TV_IN_PAL)
    {
      TvOutMode = SYS_TV_OUT_PAL;
	  AE_Flicker_50_60_sel = 1 ;
    }
    else
    {
      TvOutMode = SYS_TV_OUT_NTSC;
	  AE_Flicker_50_60_sel = 0;
    }

    iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);

    ResetisuPanel();
    uiSetTVOutXY(TvOutMode);
    playbackflag = 0;
    uiMenuEnable=0;
}

void uiEnterPanelMode(void)
{
    sysTVOutOnFlag=0;
    if(sysTVinFormat  == TV_IN_PAL)
    {
      TvOutMode = SYS_TV_OUT_PAL;
	  AE_Flicker_50_60_sel = 1 ;
    }
    else
    {
      TvOutMode = SYS_TV_OUT_NTSC;
	  AE_Flicker_50_60_sel = 0;
    }
    sysIDU_enable();
    iduPlaybackMode(PANNEL_X,PANNEL_Y, PANNEL_X);
    ResetisuPanel();
    uiSetTVOutXY(UI_MENU_SETTING_PANEL_OUT);
    playbackflag = 0;
    uiMenuEnable=0;
}

void uiMenuTreeInit(void)
{
#if (SUPPORT_TOUCH == 1)
    uiEnterMenu(UI_MENU_NODE_PREVIEW_MODE);
#else
    uiEnterMenu(0);
#endif
#if (UI_BOOT_FROM_PANEL == 0)
    uiEnterTVMode();
#else   /*Boot Enter Panel Out*/
    uiEnterPanelMode();
#endif
//DEBUG_UI("---->mode =%d\n",MyHandler.MenuMode);
}

void uiFlowRFStatus(u32 waitFlag)
{
#if(RFIU_SUPPORT)
    u8 i, err, ChId;

    for(i=0;i<MULTI_CHANNEL_MAX;i++)
    {
        if(waitFlag & (RFIU_RX_STA_LINK_OK << (i*8)))
        {            
            uiShowOSDTime[i] = 1;
            #if(UI_BAT_SUPPORT)
            SysShowPlay |= (0x01 << i);
            uiCheckBTCSetting(i,UI_RF_STATUS_LINK);//BTC轉DC只會跑這個uiCheckBTCSetting
            #else
            uiFlowCheckRecState();
            #endif
            uiSynRfConfig(i);
        }
        else if (waitFlag & (RFIU_RX_STA_LINK_BROKEN << (i*8)))
        {
            uiShowOSDTime[i] = 0;
            #if(UI_BAT_SUPPORT)
            SysShowPlay &= ~(0x01<<i);
            #endif
        }
        
    }

    if((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE) && (MyHandler.MenuMode != STANDBY_MODE))
    {
        for(i=0;i<MULTI_CHANNEL_MAX;i++)
            uiRFStatue[i] = UI_RF_STATUS_OTHER;
        return;
    }
    //DEBUG_UI("========== uiRFStatue[sysRFRxInMainCHsel] is %d ==========\r\n",uiRFStatue[sysRFRxInMainCHsel]);
    OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_WAIT_CLR_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

    // stop recording and closed file when link broken
    for ( i = 0; i < MAX_RFIU_UNIT; i++)
    {
        if (waitFlag & (RFIU_RX_STA_LINK_BROKEN << (i*8)))
        {
            if (gRfiu_Op_Sta[i] != RFIU_RX_STA_LINK_OK)
            {
                if(uiRFStatue[i] != UI_RF_STATUS_NO_SINGLE)
                {
                    MultiChannelAsfLinkBrokenCloseFile(i);
                    DEBUG_UI("Cam %d stop rec in link broken\n",i);                 
                }
                isPIRsenSent[i] = 0; //for next link up, sending PIR sensitivity from RX to TX
            }        
        }
    } 
    
    ChId = sysRFRxInMainCHsel;	
    if (MyHandler.MenuMode == QUAD_MODE)
    {
        for ( i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            if (((waitFlag >> (i*8)) & (RFIU_RX_STA_LINK_BROKEN|RFIU_RX_STA_LINK_OK))==(RFIU_RX_STA_LINK_BROKEN|RFIU_RX_STA_LINK_OK))
            {
                if (gRfiu_Op_Sta[i] == RFIU_RX_STA_LINK_OK)
                    waitFlag &= ~(RFIU_RX_STA_LINK_BROKEN <<(i*8));
                else
                {
                    waitFlag &= ~(RFIU_RX_STA_LINK_OK <<(i*8));
                    uiRFStatue[i] = UI_RF_STATUS_OTHER;
                }
                    
            }
            
            if(waitFlag & (RFIU_RX_STA_LINK_BROKEN << (i*8)))
            {
                if(uiRFStatue[i] != UI_RF_STATUS_NO_SINGLE)
                {
                    uiShowOSDTime[i] = 0;
                    uiRFStatue[i]=UI_RF_STATUS_NO_SINGLE;
                    uiOsdDrawQuadNoSignal(UI_OSD_DRAW, i);
                    
                    #if(UI_BAT_SUPPORT)
                    uiCheckBTCSetting(i,UI_RF_STATUS_NO_SINGLE);
                    #endif

                    DEBUG_UI("========== %d link broken ==========\r\n",i);
                }
            }
            else if(waitFlag & (RFIU_RX_STA_LINK_OK << (i*8)))
            {
                if(uiRFStatue[i] != UI_RF_STATUS_LINK)
                {
                    uiRFStatue[i]=UI_RF_STATUS_LINK;
                    uiShowOSDTime[i] = 1;
                    uiOsdDrawQuadNoSignal(UI_OSD_CLEAR, i);
                    
                    #if(UI_BAT_SUPPORT)
                    uiCheckBTCSetting(i,UI_RF_STATUS_LINK);
                    #endif
                    
                    osdDrawQuadIcon();
                    #if(RFRX_QUAD_AUDIO_EN == 1)
                    
                        #if((HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593))
                            osdDrawQuadFrameBG();
                        #else
                            osdDrawQuadFrame(sysRFRxInMainCHsel+2,UI_OSD_DRAW);   
                        #endif
                         
                    #endif
                    DEBUG_UI("========== %d link OK ==========\r\n",i);
                }
            }
        }

    }
    else
    {
        if (((waitFlag >> (sysRFRxInMainCHsel*8)) & (RFIU_RX_STA_LINK_BROKEN|RFIU_RX_STA_LINK_OK))==(RFIU_RX_STA_LINK_BROKEN|RFIU_RX_STA_LINK_OK))
        {
            if (gRfiu_Op_Sta[sysRFRxInMainCHsel] == RFIU_RX_STA_LINK_OK)
                waitFlag &= ~(RFIU_RX_STA_LINK_BROKEN <<(sysRFRxInMainCHsel*8));
            else
            {
                waitFlag &= ~(RFIU_RX_STA_LINK_OK <<(sysRFRxInMainCHsel*8));
                uiRFStatue[sysRFRxInMainCHsel] = UI_RF_STATUS_OTHER;
            }
        }

        if(waitFlag & (RFIU_RX_STA_LINK_BROKEN << (sysRFRxInMainCHsel*8)))
        {
            if(uiRFStatue[sysRFRxInMainCHsel] != UI_RF_STATUS_NO_SINGLE)
            {
                uiShowOSDTime[sysRFRxInMainCHsel] = 0;
                uiRFStatue[sysRFRxInMainCHsel]=UI_RF_STATUS_NO_SINGLE;
                //DEBUG_RED("%d %s\n",__LINE__, __FILE__);
                uiOsdDrawNoSignal(UI_OSD_DRAW);
                
                #if(UI_BAT_SUPPORT)
                uiCheckBTCSetting(sysRFRxInMainCHsel,UI_RF_STATUS_NO_SINGLE);
                #endif
                DEBUG_UI("========== %d link broken ==========\r\n",sysRFRxInMainCHsel);
            }
        }
        else if(waitFlag & (RFIU_RX_STA_LINK_OK << (sysRFRxInMainCHsel*8)))
        {
            if(uiRFStatue[sysRFRxInMainCHsel] != UI_RF_STATUS_LINK)
            {
                uiRFStatue[sysRFRxInMainCHsel]=UI_RF_STATUS_LINK;
                uiShowOSDTime[sysRFRxInMainCHsel] = 1;
                //DEBUG_RED("%d %s\n",__LINE__, __FILE__);
                uiOsdDrawNoSignal(UI_OSD_CLEAR);
                
                #if(UI_BAT_SUPPORT)
                uiCheckBTCSetting(sysRFRxInMainCHsel,UI_RF_STATUS_LINK);
                #endif
                DEBUG_UI("========== %d link OK ==========\r\n",sysRFRxInMainCHsel);
            }
        }
    }
    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_CHANGE_CAMERA, OS_FLAG_CLR, &err);
#endif
}

void uiFlowCheckMotion(void)
{
    u8 i,level;
   
    for (i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        #if(UI_BAT_SUPPORT)
        if (uiCheckMotionByCamType(i))
        #else
        if (iconflag[UI_MENU_SETIDX_REC_MODE_CH1 + i] == UI_MENU_REC_MODE_MOTION)
        #endif
        {
            if (uiMotionSecTime[i] == UI_MD_NOT_REC)
            {
                if (GMotionTrigger[i+MULTI_CHANNEL_LOCAL_MAX] == 1)
                {
                    DEBUG_UI(" @#$# Motion REC CAM %d \n",i);
                    GMotionTrigger[i+MULTI_CHANNEL_LOCAL_MAX] = 0;
                    #if (NIC_SUPPORT == 1)
                    sysback_Net_SetEvt(SYS_BACKRF_NTE_SEND_EVENT, i, EVENT_MOTIONDECT);
                    #endif
                    if ((uiFlowSetAlarm == UI_ALARM_ON) && (i == sysRFRxInMainCHsel))
                        Beep_function(1,200,60,200,200,0);
                    //if (uiCaptureVideoByChannel(i) == 1)
                        uiMotionSecTime[i] = UI_SET_MD_TIME;
                    if (uiCheckPower == FALSE)
                        openPower=TRUE;
                        
                }
            }
            else
            {
                uiMotionSecTime[i]--;
                if ((uiFlowSetAlarm == UI_ALARM_ON) && (i == sysRFRxInMainCHsel))
                    Beep_function(1,200,60,200,200,0);
                if (uiMotionSecTime[i] == 0)
                {
                    //if (uiCaptureVideoStopByChannel(i) == 1)
                    {
                        GMotionTrigger[i+MULTI_CHANNEL_LOCAL_MAX] = 0;
                        uiMotionSecTime[i] = UI_MD_NOT_REC;
                    }
                }
            }
        }
        
    }
    
    if ((uiCheckPower == FALSE) && openPower)
    {    
        gpioGetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, &level);
        if(level == GPIO_LEVEL_LO)
        {
            gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_HI);
            uiCloPowSec = UI_SET_CLOSE_PANEL_TIME;
        }
        else
        {
            if (uiCloPowSec == 0)
            {
                gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, GPIO_LEVEL_LO);
                openPower = FALSE;
            }
            else
            {
                uiCloPowSec--;
            }        
        }
    }

}

#if (DCF_RECORD_TYPE_API)
u8 uiSetFileType(u8 camID)
{
    u8 result = DCF_I_FILE_NAME_DASH;

    switch(uiCurRecStatus[camID])
    {
        case UI_REC_TYPE_MANUAL:
            result = DCF_I_FILE_NAME_MANUAL;
            break;
            
        case UI_REC_TYPE_MOTION:
            result = DCF_I_FILE_NAME_DYNAMIC;
            break;
            
        case UI_REC_TYPE_SCHEDULE:
            result = DCF_I_FILE_NAME_SCHE;
            break;
            
#if UI_BAT_SUPPORT
        case UI_REC_TYPE_PIR:
            if (VideoClipParameter[camID + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode & ASF_CAPTURE_PIRTIGGER_ENA)
                result = DCF_I_FILE_NAME_DYNAMIC;
            else
                result = DCF_I_FILE_NAME_MANUAL;
            break;
#endif            
    }

    return result;
}
#endif

/*
 *********************************************************************************************************
 * Called by other moudle
 *********************************************************************************************************
 */

#if (NIC_SUPPORT)
u8 uiGetMotionStatusAPP(u8 Ch_ID)
{
    u8 level;
    
    switch(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1+Ch_ID])
    {
        case UI_MENU_SETTING_SENSITIVITY_LOW: 
            level = 0;
            break;
            
        case UI_MENU_SETTING_SENSITIVITY_MID:
            level = 1;
            break;
            
        case UI_MENU_SETTING_SENSITIVITY_HIGHT:
            level = 2;
            break;    
            
        default:
            DEBUG_UI("Error uiMenuSet_MotionSensitivity %d Camera %d\r\n", iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1+Ch_ID], Ch_ID);
            level = 0;
            break;
    }

    return level;
}

u8 uiSetMotionStatusAPP(u8 ch, u8 status)
{
    u8 level;
 
    switch(status)
    {
        case 0: 
            level =UI_MENU_SETTING_SENSITIVITY_LOW;
            break;
            
        case 1:
            level =UI_MENU_SETTING_SENSITIVITY_MID;
            break;
            
        case 2:
            level =UI_MENU_SETTING_SENSITIVITY_HIGHT;
            break;    
            
        default:
            level =UI_MENU_SETTING_SENSITIVITY_LOW;
            break;
    }
    iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1+ch] = level;
    Save_UI_Setting();
    uiMenuAction(UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1+ch);
    
    return 1;
}

#if (UI_LIGHT_SUPPORT)
u8 uiGetLightOnOffAPP(u8 ch)
{
    if (iconflag[UI_MENU_SETIDX_CH1_LS_ONOFF+ch] == UI_MENU_CAMERA_ALARM_ON)
        return TRUE;
    else
        return FALSE;
}

u8 uiSetLightOnOffAPP(u8 ch, u8 status)
{
    if (iconflag[UI_MENU_SETIDX_CH1_LS_ONOFF+ch] == status)
        return 1;

    iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+ch] = UI_LIGHT_MANUAL_OFF;
    iconflag[UI_MENU_SETIDX_CH1_LS_ONOFF+ch] = status;
    Save_UI_Setting();
    uiMenuAction(UI_MENU_SETIDX_CH1_LS_ONOFF+ch);
    uiOsdDrawLightApp(ch);   
    return 1;
}

u8 uiGetSuptLightAPP(u8 ch)
{
    u8 str[] = "613BD";
    u8 len;
    
    len = strlen((char*)str);
    if(!strncmp((char*)gRfiuUnitCntl[ch].RFpara.TxCodeVersion, (const char *)str, len))
    {
        if (gRfiuUnitCntl[ch].RFpara.TxCodeVersion[31] & UI_VERSION_BIT_LIGHT)
            return TRUE;
        else
            return FALSE;
    }
    else
    {
        return FALSE;
    }
        
}
#endif

#if (UI_CAMERA_ALARM_SUPPORT)
u8 uiGetAlarmOnOffAPP(u8 ch)
{
    if (iconflag[UI_MENU_SETIDX_CH1_CA_ONOFF+ch] == UI_MENU_CAMERA_ALARM_ON)
        return TRUE;
    else
        return FALSE;
}

u8 uiSetAlarmOnOffAPP(u8 ch, u8 status)
{
    if (iconflag[UI_MENU_SETIDX_CH1_CA_ONOFF+ch] == status)
        return 1;

    iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+ch]=UI_CAMERA_ALARM_MANUAL_OFF;
    iconflag[UI_MENU_SETIDX_CH1_CA_ONOFF+ch] = status;
    Save_UI_Setting();
    uiMenuAction(UI_MENU_SETIDX_CH1_CA_ONOFF+ch);
    uiOsdDrawAlarmApp(ch); 
    return 1;
}

u8 uiGetSuptAlarmAPP(u8 ch)
{
    u8 str[] = "613BD";
    u8 len;
    
    len = strlen((char*)str);
    if(!strncmp((char*)gRfiuUnitCntl[ch].RFpara.TxCodeVersion, (const char *)str, len))
    {
        if (gRfiuUnitCntl[ch].RFpara.TxCodeVersion[31] & UI_VERSION_BIT_CA)
            return TRUE;
        else
            return FALSE;
    }
    else
    {
        return FALSE;
    }
        
}
#endif
#endif

u8 uiCaptureVideoByChannel(u8 Ch_ID)
{
    u8  temp;

    if (Ch_ID >= MULTI_CHANNEL_MAX)
    {
        DEBUG_UI("Capture Video Error Channel %d\r\n",Ch_ID);
        return 0;
    }
    
    if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
    {
    #if ((UI_BAT_SUPPORT) && (DCF_RECORD_TYPE_API))
        if (uiCurRecStatus[Ch_ID] == UI_REC_TYPE_NONE)
        {
            DEBUG_UI("Cam %d uiCurRecStatus is none\r\n",Ch_ID);
            return 0;
        }

        dcfSetChannelRecType(Ch_ID,uiSetFileType(Ch_ID));
    #endif
    
    #if UI_BAT_SUPPORT
        if(gRfiuUnitCntl[Ch_ID].RFpara.BatCam_PIRMode == 0){
            if (MultiChannelGetCaptureVideoStatus(Ch_ID+MULTI_CHANNEL_LOCAL_MAX) != 0){
            uiCaptureVideoStopByChannel(Ch_ID);
            }
        temp = (u8)MultiChannelSysCaptureVideoOneCh(Ch_ID+MULTI_CHANNEL_LOCAL_MAX);
        }
        else{
            if (MultiChannelGetCaptureVideoStatus(Ch_ID+MULTI_CHANNEL_LOCAL_MAX) == 0){
                temp = (u8)MultiChannelSysCaptureVideoOneCh(Ch_ID+MULTI_CHANNEL_LOCAL_MAX);
            }
        }
    #endif
        return temp;
    }
    else
    {
        DEBUG_UI("No SD Card\r\n");
        if ((MyHandler.MenuMode == VIDEO_MODE) || (MyHandler.MenuMode == QUAD_MODE))
        {
            if (sysRFRxInMainCHsel == Ch_ID)
            {
                uiOsdDrawInsertSD(UI_OSD_DRAW);
            }
        }
        return 0;
    }

}


/*

Routine Description:

	Stop Capture Video

Arguments:

	None.

Return Value:

	0 - Not need to stop.
	1 - Success.

*/
u8 uiCaptureVideoStopByChannel(u8 Ch_ID)
{
    if (Ch_ID >= MULTI_CHANNEL_MAX)
    {
        DEBUG_UI("Stop Capture Video Error Channel %d\r\n",Ch_ID);
        return 0;
    }

    DEBUG_UI("uiCaptureVideoStopByChannel %d\r\n",Ch_ID);
    if ( MultiChannelSysCaptureVideoStopOneCh(Ch_ID+MULTI_CHANNEL_LOCAL_MAX) == 1)
    {
        if ((MyHandler.MenuMode != SETUP_MODE) && (MyHandler.MenuMode != SET_NUMBER_MODE))
        {
            if ((MyHandler.MenuMode == VIDEO_MODE) || (MyHandler.MenuMode == QUAD_MODE))
                osdDrawVideoOn(UI_OSD_CLEAR);
            else if (MyHandler.MenuMode == QUAD_MODE)
                osdDrawQuadVideoOn(Ch_ID, UI_OSD_CLEAR);
        }
        return 1;
    }
    return 0;
    
}

void uiFlowCheckScheduleTime(void)
{
    u8  i, tmpSch;
    RTC_DATE_TIME   localTime;
    static u8 CurWeek, lastSchedule=0xff;
    static u8 cam_status=0;
    u8 cur_status=0;

    if(SD_detect_status == 0)
    {
        lastSchedule=0xff;   
        return;
    }

    if (UICheckScheduleGetTime == 0)
    {
        UICheckScheduleGetTime = 10;
        RTC_Get_Time(&localTime);
        tmpSch = localTime.hour*2+localTime.min/30;
        if ((tmpSch < UICheckSchedule)||(UICheckSchedule == 48))
        {
            if (UICheckSchedule == 48)
                lastSchedule = 0xFF;
            CurWeek = RTC_Get_Week(&localTime);
            if(CurWeek == 0)
                CurWeek = 6;
            else
                CurWeek--;
        }
        UICheckSchedule = tmpSch;
    }
    else
    {
        UICheckScheduleGetTime--;
        return;
    }

    #if 1
    for( i=0; i< MULTI_CHANNEL_MAX ; i++)
    {
        if(iconflag[UI_MENU_SETIDX_REC_MODE_CH1+i]== UI_MENU_REC_MODE_SCHEDULE)
        {
            cur_status+=(i+1);    
        }        
    }
    
    if( cam_status != cur_status)
    {
        cam_status=cur_status;  
        cur_status=0;
    }
        
    if (lastSchedule == UICheckSchedule)
        return;
    #endif
    
    DEBUG_UI("UICheckSchedule %d lastSchedule %d \n", UICheckSchedule, lastSchedule);
    
    cam_status=0;
    for (i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        #if(UI_BAT_SUPPORT)
        if(_uiBatType[i] == UI_CAM_BATTERY)
            continue;
        #endif

        if(uiCurRecStatus[i] == UI_REC_TYPE_MANUAL)
            continue;

        if(gRfiu_Op_Sta[i] == RFIU_RX_STA_LINK_BROKEN)
            continue;
        
        //DEBUG_YELLOW("%d %s %d %d\n",__LINE__, __FILE__,i,iconflag[UI_MENU_SETIDX_REC_MODE_CH1+i]);
        if(iconflag[UI_MENU_SETIDX_REC_MODE_CH1+i]== UI_MENU_REC_MODE_SCHEDULE)//沒有任何一支設sch就會每秒都檢查
        {
            //DEBUG_YELLOW("%d %s %d %d\n",__LINE__, __FILE__,CurWeek,uiScheduleTime[CurWeek][i][UICheckSchedule]);
            cam_status+=(i+1); 
            if( uiScheduleTime[CurWeek][i][UICheckSchedule] == UI_MENU_SCHEDULE_REC)
            {
                uiCurRecStatus[i]=UI_REC_TYPE_SCHEDULE;

                if (uiShowOSDTime[i] == 0)
                    continue;
                
                if((MultiChannelGetCaptureVideoStatus(i) != UI_REC_STATUS_RECING) || ((VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode & ASF_CAPTURE_EVENT_MOTION_ENA) == ASF_CAPTURE_EVENT_MOTION_ENA))
                {
                    DEBUG_UI("@#!Start Capture Channel %d by Check Schedule\n",i);
                    VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                    VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_EVENT_MOTION_ENA;
                    uiCaptureVideoByChannel(i);
                    gSchRecStop = 2;
                }
            }
            else
            {
                if(uiCurRecStatus[i] == UI_REC_TYPE_SCHEDULE)
                {
                    if (MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) != UI_REC_STATUS_NONE)
                    {
                        uiCaptureVideoStopByChannel(i);  
                        //DEBUG_UI("cam %d stop schedule rec\n",i);
                    }
                }
                
                uiCurRecStatus[i] = UI_REC_TYPE_MOTION;
                if (MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == UI_REC_STATUS_NONE)
                {
                    VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                    VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_EVENT_MOTION_ENA;
                    uiCaptureVideoByChannel(i);
                    DEBUG_UI("%d cam %d start motion\n",__LINE__,i);
                }

            }
        }
        else
        {
            uiCurRecStatus[i] = UI_REC_TYPE_MOTION;
            if (MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == UI_REC_STATUS_NONE)
            {
                VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode &= ~ASF_CAPTURE_PIRTIGGER_ENA;
                VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode |= ASF_CAPTURE_EVENT_MOTION_ENA;
                uiCaptureVideoByChannel(i);
                DEBUG_UI("%d cam %d start motion\n",__LINE__,i);
            }
        }
    }
    lastSchedule = UICheckSchedule;
}

u8 uiCompareSaveData(void)
{
    u8 update = 0;

    if (memcmp((void*)start_iconflag, (void*)iconflag, UIACTIONNUM) != 0)
    {
        memcpy ((void *)start_iconflag,(void *)iconflag,  UIACTIONNUM);
        update = 1;
    }
    
#if 1
    if (memcmp(&uiMaskArea[0][0], &uiStartMaskArea[0][0], sizeof(uiMaskArea)) != 0)
    {
        memcpy(&uiStartMaskArea[0][0], &uiMaskArea[0][0], sizeof(uiMaskArea));
        update = 1;
    }
#else
    if (memcmp(&StartMotMask[0][0], &MotionMaskArea[0][0], MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN) != 0)
    {
        memcpy(&StartMotMask[0][0], &MotionMaskArea[0][0],  MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN);
        update = 1;
    }
#endif

    if (memcmp(&uiStartScheduleTime[0][0][0], &uiScheduleTime[0][0][0], sizeof(uiScheduleTime)) != 0)
    {
        memcpy(&uiStartScheduleTime[0][0][0], &uiScheduleTime[0][0][0], sizeof(uiScheduleTime));
        update = 1;
    }
#if(RFIU_SUPPORT)
    if (memcmp(&uiStartRFID, &uiRFID, sizeof(uiRFID)) !=0)
    {
        memcpy(&uiStartRFID, &uiRFID, sizeof(uiRFID));
        update = 1;
    }
    if (memcmp(&uiStratRFCode, &uiRFCODE, sizeof(uiRFCODE)) !=0)
    {
        memcpy(&uiStratRFCode, &uiRFCODE, sizeof(uiRFCODE));
        update = 1;
    }
#endif
    if (memcmp(&UI_CFG_RES, &UI_TMP_RES, sizeof(UI_CFG_RES)) !=0)
    {
        memcpy(&UI_TMP_RES, &UI_CFG_RES, sizeof(UI_CFG_RES));
        update = 1;
    }

#if(NIC_SUPPORT)
    if (memcmp(&UINetInfo, &StartUINetInfo, sizeof(UI_NET_INFO)) !=0)
    {
        memcpy(&StartUINetInfo, &UINetInfo, sizeof(UI_NET_INFO));
        update = 1;
    }
    if (memcmp(&UI_P2P_PSW, &Start_UI_P2P_PSW, sizeof(UI_P2P_PSW)) !=0)
    {
        memcpy(&Start_UI_P2P_PSW, &UI_P2P_PSW, sizeof(UI_P2P_PSW));
        update = 1;   
    }
#endif
    if (memcmp(&SetZone, &StartTimeZone, sizeof(SetZone)) != 0)
    {
        memcpy(&StartTimeZone, &SetZone, sizeof(SetZone));
        update = 1;
    }
    if (memcmp(&start_sysConfig, &sysConfig, sizeof(SYS_CONFIG_SETTING)) != 0)
    {
        memcpy (&start_sysConfig,&sysConfig, sizeof(SYS_CONFIG_SETTING));
        update = 1;
    }
    
#if(HW_BOARD_OPTION  == MR9200_RX_MAYON_MWM903)
#if UI_LIGHT_SUPPORT
    if (memcmp(&uiLightInterval[0][0][0], &uiStartLightInterval[0][0][0], sizeof(uiLightInterval)) != 0)
    {
        memcpy(&uiStartLightInterval[0][0][0], &uiLightInterval[0][0][0], sizeof(uiLightInterval));
        update = 1;
    }
#endif

#if UI_CAMERA_ALARM_SUPPORT
    if (memcmp(&uiCamAlarmInterval[0][0][0], &uiStartCamAlarmInterval[0][0][0], sizeof(uiCamAlarmInterval)) != 0)
    {
        memcpy(&uiStartCamAlarmInterval[0][0][0], &uiCamAlarmInterval[0][0][0], sizeof(uiCamAlarmInterval));
        update = 1;
    }
#endif

#if(UI_BAT_SUPPORT)
    if (memcmp(&uiBatteryInterval[0][0][0], &uiStartBatteryInterval[0][0][0], sizeof(uiBatteryInterval)) != 0)
    {
        memcpy(&uiStartBatteryInterval[0][0][0], &uiBatteryInterval[0][0][0], sizeof(uiBatteryInterval));
        update = 1;
    }
#endif

#else
#if UI_LIGHT_SUPPORT
    if (memcmp(&uiLightTimer[0][0], &uiStartLightTimer[0][0], sizeof(uiLightTimer)) != 0)
    {
        memcpy(&uiStartLightTimer[0][0], &uiLightTimer[0][0], sizeof(uiLightTimer));
        update = 1;
    }
#endif
#endif
    
    return update;
}

u8 uiGetSaveChecksum(void)
{
    u8 i, j, k, check_val = 0;

    /*avoid warning message*/
    if(j){}

    for (i = 0; i < UIACTIONNUM-1; i++)
        check_val += iconflag[i];

#if 1
    for ( i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        for (j = 0; j < 9; j++)
            check_val += uiMaskArea[i][j];
    }
#else
    for ( i = 0; i < MASKAREA_MAX_ROW; i++)
        for (j = 0; j < MASKAREA_MAX_COLUMN; j++)
            check_val += MotionMaskArea[i][j];
#endif

    for (i = 0 ; i < 7; i++)
    {
        for (j = 0; j < MULTI_CHANNEL_MAX; j++)
        {
            for (k = 0; k < 48; k++)
                check_val += uiScheduleTime[i][j][k];
        }
    }

#if UI_LIGHT_SUPPORT
    for (i = 0 ; i < MULTI_CHANNEL_MAX; i++)
    {
        for (j = 0; j < 7; j++)
        {
            for (k = 0; k < 6; k++)
                check_val += uiLightInterval[i][j][k];
        }
    }
#endif

#if UI_CAMERA_ALARM_SUPPORT
    for (i = 0 ; i < MULTI_CHANNEL_MAX; i++)
    {
        for (j = 0; j < 7; j++)
        {
            for (k = 0; k < 6; k++)
                check_val += uiCamAlarmInterval[i][j][k];
        }
    }
#endif

#if UI_BAT_SUPPORT
    for (i = 0 ; i < MULTI_CHANNEL_MAX; i++)
    {
        for (j = 0; j < 7; j++)
        {
            for (k = 0; k < 6; k++)
                check_val += uiBatteryInterval[i][j][k];
        }
    }
#endif

#if(NIC_SUPPORT)
    for (i = 0; i< 3; i++)
    {
        check_val += UINetInfo.IPaddr[i];
        check_val += UINetInfo.Netmask[i];
        check_val += UINetInfo.Gateway[i];
    }
    check_val += UINetInfo.IsStaticIP;
#endif

    check_val += UI_SET_CHECKSUM;
    return check_val;
}

/*

Routine Description:

	Stop Capture Video

Arguments:

	None.

Return Value:

	0 - Fail.
	1 - Success.

*/
u8 uiCaptureVideo(void)
{
    u8 i;
    
    if (MemoryFullFlag == TRUE)
    {
        DEBUG_UI("SD Card FULL!!!!!\r\n");
        return 0;
    }
    
    if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
    {
        #if ((UI_BAT_SUPPORT) && (DCF_RECORD_TYPE_API))
        for(i=0;i<MULTI_CHANNEL_MAX;i++)
        {
            if (uiCurRecStatus[i] == UI_REC_TYPE_NONE)
            {
                DEBUG_UI("Cam %d uiCurRecStatus is none\r\n",i);
                continue;
            }
            dcfSetChannelRecType(i,uiSetFileType(i));
        }
        #endif
        
        DEBUG_UI("UI Start capture video\r\n");
        
#if (MULTI_CHANNEL_VIDEO_REC && MULTI_CHANNEL_RF_RX_VIDEO_REC)
        RfRxVideoPackerEnable();
#endif
        if(uiKeyVideoCapture() == 1)
        {
            return 1;
        }
        else
            DEBUG_UI("CaptureVideo Fail\r\n");
    }
    else
    {
        DEBUG_UI("No SD Card\r\n");
        if ((MyHandler.MenuMode == VIDEO_MODE) || (MyHandler.MenuMode ==QUAD_MODE))
        {
            uiOsdDrawInsertSD(UI_OSD_DRAW);
        }
    }
    return 0;
}

/*

Routine Description:

	Stop Capture Video

Arguments:

	None.

Return Value:

	0 - Not need to stop.
	1 - Success.

*/
u8 uiCaptureVideoStop(void)
{
    u8 err,i;
    u32   waitFlag;

#if (MULTI_CHANNEL_VIDEO_REC && MULTI_CHANNEL_RF_RX_VIDEO_REC)
    RfRxVideoPackerDisable();
    osdDrawVideoOn(UI_OSD_CLEAR);
#endif

    waitFlag = OSFlagAccept(gSysReadyFlagGrp, (FLAGSYS_RDYSTAT_SET_REC|FLAGSYS_RDYSTAT_REC_START), OS_FLAG_WAIT_CLR_ANY, &err);
    if(waitFlag > 0)
    {
        if(waitFlag & FLAGSYS_RDYSTAT_SET_REC)
        {
            DEBUG_UI("Capture do not Start and now Stop %d\r\n",waitFlag);
            sysCaptureVideoStart = 1;
        }
        else
        {
            if(sysCaptureVideoStart == 0)
            {
                DEBUG_UI("Capture Start and now Stop %d\r\n",waitFlag);
                sysCaptureVideoStart = 1;
            }
            else
            {
                DEBUG_UI("UI Wait Stop Capture.....\r\n");
        #if MULTI_CHANNEL_VIDEO_REC
                MultiChannelAsfCaptureVideoStopAll();
        #else
            #if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_ASF)
                while(asfCaptureVideoStop() == 0)
            #elif(MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_AVI)
                while(aviCaptureVideoStop() == 0)
            #elif(MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MOV)
                while(movCaptureVideoStop() == 0)
            #endif
        #endif
                {
                    OSTimeDly(4);
                }
                OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_REC_START, OS_FLAG_WAIT_SET_ANY, OS_IPC_WAIT_FOREVER, &err);
            }
        }
        OSTimeDly(15);
        DEBUG_UI("UI stop capture video Success\r\n");
        if (MyHandler.MenuMode == VIDEO_MODE)
        {
            osdDrawVideoOn(UI_OSD_CLEAR);
        }
        else if(MyHandler.MenuMode == QUAD_MODE)
        {
            for (i = 0; i < 4; i++)
            {
                osdDrawQuadVideoOn(i,UI_OSD_CLEAR);
            }
        }
        return 1;
    }
    else
    {
        DEBUG_UI("Not yet begun capturing\n");
        return 0;
    }
}

u8 uiSetGoToFormat(void)
{
    while(ShowLogoFinish == FALSE)
        OSTimeDly(10);
    gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
    lastMenuMode = MyHandler.MenuMode;
    MyHandler.MenuMode = GOTO_FORMAT_MODE;
    siuOpMode = 0;
    sysCameraMode = SYS_CAMERA_MODE_PLAYBACK;
    playbackflag = 0;
    uiMenuEnable=0;
    uiReturnPreview = UI_MENU_TO_PRV;
    iduMenuMode(PANNEL_X,PANNEL_Y, PANNEL_X);
    uiOsdDisableAll();
    uiSetRfDisplayMode(UI_MENU_RF_ENTER_SETUP);
    uiEnterMenu(UI_MENU_NODE_SET_FORMAT_NO);
    uiGraphDrawMenu();
    gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
    return 1;
}

void uiDrawGetGSensorData(u8 *data)
{
#if (G_SENSOR == G_SENSOR_LIS302DL)
    s8              out_X, out_Y, out_Z;
    s16             Temp;
    u8              X, Y, Z, signX, signY, signZ;
    const   u8      Floating[16][3] = {"00", "06", "13", "19", "25", "31", "38", "44", "50", "56", "63", "69", "75", "81", "88", "94"};
    //const   u8      Floating[16][3] = {"0", "1", "1", "2", "3", "3", "4", "4", "5", "6", "6", "7", "8", "8", "9", "9"};
    i2cGet_LIS302DL_XYZ(&out_X, &out_Y, &out_Z);
    out_X   = (out_X > 127) ? 127 : (out_X < -128) ? -128 : out_X;
    out_Y   = (out_Y > 127) ? 127 : (out_Y < -128) ? -128 : out_Y;
    out_Z   = (out_Z > 127) ? 127 : (out_Z < -128) ? -128 : out_Z;
    X       = abs(out_X);
    Y       = abs(out_Y);
    Z       = abs(out_Z);
    signX   = (out_X >= 0) ? '+' : '-';
    signY   = (out_Y >= 0) ? '+' : '-';
    signZ   = (out_Z >= 0) ? '+' : '-';
  #if G_SENSOR_DETECT
    if((X >= GLimitX) || (Y >= GLimitY) || (Z >= GLimitZ))
    {
        DEBUG_I2C("G Sensor Value(%d, %d, %d) >= G Threshold(%d, %d, %d)\n", out_X, out_Y, out_Z, GLimitX, GLimitY, GLimitZ);
        GSensorEvent    = 1;
    }
  #endif  // #if G_SENSOR_DETECT
    sprintf ((char *)data, "(%c%d.%sG,%c%d.%sG,%c%d.%sG)",
            signX, X >> 4, Floating[X & 0xf], signY, Y >> 4,
            Floating[Y & 0xf], signZ, Z >> 4, Floating[Z & 0xf]);

#elif (G_SENSOR == G_SENSOR_BMA150)
    static  s16     out_X, out_Y, out_Z, out_T;
    u16             X, Y, Z, signX, signY, signZ;

    i2cGet_BMA150_TXYZ(&out_T, &out_X, &out_Y, &out_Z);
    X       = abs(out_X);
    Y       = abs(out_Y);
    Z       = abs(out_Z);
    signX   = (out_X >= 0) ? '+' : '-';
    signY   = (out_Y >= 0) ? '+' : '-';
    signZ   = (out_Z >= 0) ? '+' : '-';
  #if G_SENSOR_DETECT
    if((X >= GLimitX) || (Y >= GLimitY) || (Z >= GLimitZ))
    {
        DEBUG_I2C("G Sensor Value(%d, %d, %d) >= G Threshold(%d, %d, %d)\n", out_X, out_Y, out_Z, GLimitX, GLimitY, GLimitZ);
        GSensorEvent    = 1;
    }
  #endif  // #if G_SENSOR_DETECT
    sprintf ((char *)data, "(%c%d.%02dG,%c%d.%02dG,%c%d.%02dG)",
            signX, X >> 7, ((X & 0x7f) * 100) >> 7, signY, Y >> 7,
            ((Y & 0x7f) * 100) >> 7, signZ, Z >> 7, ((Z & 0x7f) * 100) >> 7);
#endif  // #if (G_SENSOR == G_SENSOR_LIS302DL)

}

void osdDrawGSensorData(void)
{
    if (OSDTimestameLevel == UI_MENU_VIDEO_OVERWRITESTR_ALL)
    {
        uiDrawGetGSensorData(GS);
        //uiMenuOSDString(OSDDispWidth[sysTVOutOnFlag] , GS , 8 , 16 , 12 , 56 , OSD_Blk2 , 2);
        //uiMenuOSDStringByColor(TVOSD_SizeX , GS , 8 , 16 , 12 , 36 , OSD_Blk2, 0xC0, 0x41);
    }
}

void uiDrawManualRec(void)
{
    u8 i,result=UI_OSD_CLEAR;

    switch(MyHandler.MenuMode)
    {
        case VIDEO_MODE:
            if ((MultiChannelGetCaptureVideoStatus(sysRFRxInMainCHsel+MULTI_CHANNEL_LOCAL_MAX) == 1) && 
                (!(uiShowOSDTime[sysRFRxInMainCHsel] == UI_OSD_DRAW)) && 
                (!(VideoClipParameter[sysRFRxInMainCHsel + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode & ASF_CAPTURE_EVENT_MOTION_ENA)))
            {
                #if(UI_BAT_SUPPORT)
                if ((uiCurRecStatus[sysRFRxInMainCHsel] == UI_REC_TYPE_PIR) && 
                    (!(VideoClipParameter[sysRFRxInMainCHsel + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode & ASF_CAPTURE_PIRTIGGER_ENA)))
                    result = UI_OSD_DRAW;
                #endif  

                if (uiCurRecStatus[sysRFRxInMainCHsel] == UI_REC_TYPE_MANUAL)
                    result = UI_OSD_DRAW;
            }
            break;
            
        case QUAD_MODE:
            for ( i = 0; i < MULTI_CHANNEL_MAX; i++)
            {
                if ((MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == 1) && 
                    (!(uiShowOSDTime[i] == UI_OSD_DRAW)) &&
                    (!(VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode & ASF_CAPTURE_EVENT_MOTION_ENA)))
                {
                    #if(UI_BAT_SUPPORT)
                    if ((uiCurRecStatus[i] == UI_REC_TYPE_PIR) &&
                        (!(VideoClipParameter[i + MULTI_CHANNEL_LOCAL_MAX].sysCaptureVideoMode & ASF_CAPTURE_PIRTIGGER_ENA)))
                        result = UI_OSD_DRAW;
                    #endif  

                    if (uiCurRecStatus[i] == UI_REC_TYPE_MANUAL)
                        result = UI_OSD_DRAW;
                }
            }
            break;
            
        default:
            return;
            
    }

    if (isManualRec != result)
    {
        isManualRec = result;
        osdDrawRecPreview(isManualRec);
    }
    
}

void uiFlowDrawSDFailMsg(void)
{
    if (gUishowFailTime > 0)
    {
        uiOsdDrawSDCardFail(UI_OSD_DRAW);
        gUishowFailTime--;
        //DEBUG_YELLOW("%d %s %d\n",__LINE__, __FILE__,gUishowFailTime);
    }
}

void uiFlowRunPerSec(void)
{
    u8 i,err;
    static unsigned int CheckCount=1;
#if (NIC_SUPPORT)
    static u8 NtpFlag = 0, CheckLink = UI_CHECK_NETWORK_TIME;
#endif
    RTC_DATE_TIME   localTime;

    if (OSFlagAccept(gUiStateFlagGrp, (FLAGUI_MAIN_INIT_READY|FLAGUI_SD_GOTO_FORMAT), OS_FLAG_WAIT_SET_ANY, &err)== 0)
    {
        return;
    }

    if (volumeflag > 0)
    {
        volumeflag++;
        /*clean volume osd*/
        if(volumeflag > 3)
        {
            uiOsdVolumeControl(VIDEO_MODE, UI_VALUE_CLEAN);
            if(MyHandler.MenuMode == QUAD_MODE ) // it should be draw quad frame again after clean volume osd
            {
                #if(RFRX_QUAD_AUDIO_EN == 1)
                    #if((HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593))
                    osdDrawQuadFrameBG();
                    #else
                    osdDrawQuadFrame(sysRFRxInMainCHsel+2,UI_OSD_DRAW);
                    #endif
                    
                #endif
            }
            Save_UI_Setting();
        }
    }
 
#if(NIC_SUPPORT)
#if SET_NTPTIME_TO_RTC
    if(iconflag[UI_MENU_SETIDX_DATE_TIME] == UI_MENU_SETIDX_NTP_YES)
    {
        if ((localTime.hour == 12)&&(localTime.min == 0))
        {
            if (NtpFlag == 0)
            {
                NtpFlag = 1;
                DEBUG_UI("NTP Server update Time\r\n");
                sysback_Net_SetEvt(SYS_BACKRF_NTE_NTP_UPDATE, 0, 0);
                for (i = 0; i < MULTI_CHANNEL_MAX; i++)
                {
                    uiSetRfTimeRxToTx(i);
                }
            }
        }
        else
            NtpFlag = 0;
    }
#endif

    uiFlowCheckP2PMode();
    if (uiP2PMode == 0)
    {
        if (CheckLink == 0)
        {
            uiOsdDrawNetworkLinkUp();
            CheckLink = UI_CHECK_NETWORK_TIME;
        }
        CheckLink--;
    }
    else
    {
        CheckLink = 0;
    }
#endif
    uiOsdDrawLifeTimePerSec();
    uiOsdDrawRecPerSec();
    uiFlowCheckMotion();
    uiFlowCheckScheduleTime();
    uiFlowCheckRetryRec();
    uiDrawManualRec();
    uiFlowDrawSDFailMsg();
   
    #if(UI_BAT_SUPPORT)
    uiCheckBatterySchdule();
    uiOsdDrawBtcPaly();
    #endif

    if (showPIRMsg > 0)
    {
        showPIRMsg--;
        if (showPIRMsg == 0)
        {
            osdDrawMotionMsg(UI_OSD_CLEAR);
        }
    }

    switch(MyHandler.MenuMode)
    {
        case VIDEO_MODE:
            if (ScanRf != 0)
            {
                ScanRf--;
                if ((ScanRf%5)==0)
                {
                    if (uiSetRfChangeChannel(UI_MENU_CHANNEL_SCAN) == 1)
                        ScanRf = 0;
                }
            }
            break;
        default:
            ScanRf = 0;
            if (uiReturnPreview > 0)
            {
                uiReturnPreview--;
                if (uiReturnPreview == 0)
                    uiSentKeyToUi(UI_KEY_ENTER_PRV);
            }
            break;
    }
        
    if((CheckCount & 0x07)==0) //Lucian: 每8sec做一次
    {
        //uiFlowCheckPreRecord();
        sysbackLowSetEvt(SYSBACKLOW_EVT_SYN_RF,0,0,0,0);
    }

    CheckCount ++;
}

s32 uiMenuSetStartMovie(s8 setting)
{
    s32 index;
    u16 uiJpgWidth, uiJpgHeight;
    sysTVinFormat = getTVinFormat();

    iduMenuMode(PANNEL_X,PANNEL_Y, PANNEL_X);

#if ((FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON)||(FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_ESMT))
    
    sysSPI_Enable();
    sysJPEG_enable();
    sysISU_enable();
    index = spiGet_UI_FB_Index("logo.jpg");
    if (index < 0)
    {
        DEBUG_UI("uiMenuSetStartMovie Get UI_FB Index Fail\r\n");
        return 0;
    }

    if(uiGraphDrawJpgGraph(index, PKBuf1, &uiJpgWidth, &uiJpgHeight) == 1)
        IDU_TVLayer_Stride(UI_MENU_SIZE_X ,PKBuf1, gJPGValidWidth, gJPGValidHeight, 0,0, uiJpgWidth,PKBuf0);

    else
        DEBUG_UI("File %s Open Fail\r\n","logo.jpg");

    iduSetVideoBuf0Addr(PKBuf0);
    gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);

    OSTimeDly(50); // for display logo.jpg, don't modify

    iduPreviewMode(PANNEL_X,PANNEL_Y, PANNEL_X);
    IduVideo_ClearPKBuf(0);
    #if (UI_RX_PWRON_QUAD_ENA == 1)
        IduVideo_ClearPKBuf(1);
    #endif
    IduVideo_ClearPKBuf(2);
    sysJPEG_disable();
    sysSPI_Disable();
#endif
    return 1;
}

void uiReadScheduleTime(void)
{
    u8 i,j,k,bit;

    for(i=0;i<MULTI_CHANNEL_MAX;i++)
    {
        for(k=0;k<7;k++)
        {
            for(j=0;j<6;j++)
            {
                for(bit=0;bit<8;bit++)
                    {
                    if (uitempScheduleTime[k][i][j] & (0x1 << bit))
                        uiScheduleTime[k][i][(8*j)+bit] = UI_MENU_SCHEDULE_REC; 
                    else                        
                        uiScheduleTime[k][i][(8*j)+bit] = UI_MENU_SCHEDULE_OFF; 
                    }
            }    
        }
    }
}

void uiSaveScheduleTime(void)
{
    u8 i,value,k,bit,count;

    for(i=0;i<MULTI_CHANNEL_MAX;i++)
    {
        for(k=0;k<7;k++)
        {
            count = 0;
            value = uitempScheduleTime[k][i][count];
            for (bit=0; bit < 48; bit++)
            {
                if (uiScheduleTime[k][i][bit] == 0)  
                    value &= ~(0x01<<(bit%8));
                else
                    value |= (0x01<<(bit%8));
                
                if (((bit+1)%8)==0)
                {
                    uitempScheduleTime[k][i][count++] = value;
                    value = uitempScheduleTime[k][i][count];
                }
            }
        }
    }
}

u8 uiReadSettingFromFlash(u8 *FlashAddr)
{
    int i;

    memcpy(iconflag, FlashAddr, UIACTIONNUM);
    FlashAddr += UIACTIONNUM;

    memcpy(&uitempScheduleTime[0][0][0], FlashAddr, sizeof(uitempScheduleTime));
    FlashAddr += sizeof(uitempScheduleTime);
    uiReadScheduleTime();

#if 1
    memcpy(&uiMaskArea[0][0], FlashAddr, sizeof(uiMaskArea));
    FlashAddr += sizeof(uiMaskArea);
#else
    memcpy(&MotionMaskArea[0][0], FlashAddr, MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN);
    FlashAddr += MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN;
#endif

#if UI_LIGHT_SUPPORT
    memcpy(&uiLightInterval[0][0][0], FlashAddr, sizeof(uiLightInterval));
    FlashAddr += sizeof(uiLightInterval);
    
    uiSetCfg = 0x01; //Enable light control(always on)
    for(i =0; i< MULTI_CHANNEL_MAX; i++)
        uiLightWeek[i] = iconflag[UI_MENU_SETIDX_CH1_LS_TIMER + i];
#endif

#if UI_CAMERA_ALARM_SUPPORT
    memcpy(&uiCamAlarmInterval[0][0][0], FlashAddr, sizeof(uiCamAlarmInterval));
    FlashAddr += sizeof(uiCamAlarmInterval);
#endif

#if(UI_BAT_SUPPORT)
    memcpy(&uiBatteryInterval[0][0][0], FlashAddr, sizeof(uiBatteryInterval));
    FlashAddr += sizeof(uiBatteryInterval);
#endif

#if (NIC_SUPPORT)
    memcpy(&UINetInfo, FlashAddr, sizeof(UI_NET_INFO));
    FlashAddr += sizeof(UI_NET_INFO);
    memcpy(UI_P2P_PSW, FlashAddr, sizeof(UI_P2P_PSW));
    FlashAddr += sizeof(UI_P2P_PSW);
#endif

#if(RFIU_SUPPORT)
    for (i = 0; i < MULTI_CHANNEL_MAX; i++)
        UI_CFG_RES[i]=iconflag[UI_MENU_SETIDX_RESOLUTION_CH1+i];
#endif
#if 1
    memcpy(&SetZone, FlashAddr, sizeof(SetZone));
    FlashAddr += sizeof(SetZone);
#endif   
    memcpy (&sysConfig, FlashAddr, sizeof(SYS_CONFIG_SETTING));
    FlashAddr += sizeof(SYS_CONFIG_SETTING);

    return 1;
}

u8 uiSetDefaultSetting(void)
{
    u8 tmp_tvformat,i;

    memcpy(iconflag, defaultvalue, UIACTIONNUM);
    
#if 1
    memset(&uiMaskArea[0][0], 0, sizeof(uiMaskArea));
#else    
    memcpy(&MotionMaskArea[0][0], &StartMotMask[0][0], MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN);
#endif

    memcpy(&ScheduledTimeFrom, &StartSchTimeFrom, sizeof(RTC_DATE_TIME));
    memcpy(&ScheduledTimeTo, &StartSchTimeTo, sizeof(RTC_DATE_TIME));
    memset(&uiScheduleTime[0][0][0], UI_MENU_SCHEDULE_OFF, sizeof(uiScheduleTime));
    
#if UI_LIGHT_SUPPORT
    //memset(&uiLightTimer[0][0], 0, sizeof(uiLightTimer));
    memset(&uiLightInterval[0][0][0], UI_MENU_SCHEDULE_OFF, sizeof(uiLightInterval));
    
    uiSetCfg = 0x01; //Enable light control(always on)    
    for(i =0; i< MULTI_CHANNEL_MAX; i++)
        uiLightWeek[i] = iconflag[UI_MENU_SETIDX_CH1_LS_TIMER + i];
#endif

#if UI_CAMERA_ALARM_SUPPORT
    memset(&uiCamAlarmInterval[0][0][0], UI_MENU_SCHEDULE_OFF, sizeof(uiCamAlarmInterval));
#endif

#if UI_BAT_SUPPORT
    memset(&uiBatteryInterval[0][0][0], 255, sizeof(uiBatteryInterval));
#endif

#if (NIC_SUPPORT)
    memcpy(&UINetInfo, &StartUINetInfo, sizeof(UI_NET_INFO));
    memcpy(UI_P2P_PSW, UI_Default_P2P_PSW, sizeof(UI_P2P_PSW));
#endif
#if 1
    memcpy(&SetZone, &UIDefaultTimeZone, sizeof(SetZone));
#endif
    sysSet_DefaultValue();
    return 1;
}

u8 uiWriteSettingToFlash(u8 *FlashAddr)
{

    memcpy ((void *)FlashAddr,(void *)iconflag,  UIACTIONNUM);
    FlashAddr += UIACTIONNUM;

    uiSaveScheduleTime();
    memcpy ((void *)FlashAddr,(void *)&uitempScheduleTime[0][0][0],  sizeof(uitempScheduleTime));
    FlashAddr += sizeof(uitempScheduleTime);
    
#if 1
    memcpy ((void *)FlashAddr,(void *)&uiMaskArea[0][0],  sizeof(uiMaskArea));
    FlashAddr += sizeof(uiMaskArea);
#else
    memcpy ((void *)FlashAddr, (void *)&MotionMaskArea[0][0],  MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN);
    FlashAddr += MASKAREA_MAX_ROW*MASKAREA_MAX_COLUMN;
#endif

#if UI_LIGHT_SUPPORT
    memcpy((void *)(FlashAddr), (void *)&uiLightInterval[0][0][0], sizeof(uiLightInterval));
    FlashAddr += sizeof(uiLightInterval);
#endif

#if UI_CAMERA_ALARM_SUPPORT
    memcpy((void *)(FlashAddr), (void *)&uiCamAlarmInterval[0][0][0], sizeof(uiCamAlarmInterval));
    FlashAddr += sizeof(uiCamAlarmInterval);
#endif

#if(UI_BAT_SUPPORT)
    memcpy((void *)(FlashAddr), (void *)&uiBatteryInterval[0][0][0], sizeof(uiBatteryInterval));
    FlashAddr += sizeof(uiBatteryInterval);
#endif

#if (NIC_SUPPORT)
    memcpy ((void *)FlashAddr, (void *)&UINetInfo, sizeof(UI_NET_INFO));
    FlashAddr += sizeof(UI_NET_INFO);
    memcpy((void *)(FlashAddr), (void *)&UI_P2P_PSW,sizeof(UI_P2P_PSW));
    FlashAddr += sizeof(UI_P2P_PSW);
#endif

#if 1
    memcpy((void *)(FlashAddr), (void *)&SetZone,sizeof(SetZone));
    FlashAddr += sizeof(SetZone);
#endif
    memcpy ((void *)FlashAddr,(void *)&sysConfig,  sizeof(SYS_CONFIG_SETTING));
    FlashAddr += sizeof(SYS_CONFIG_SETTING);

    return 1;
}

void uiDrawTimeOnVideoClip(s32 Param)
{
#if ISU_OVERLAY_ENABLE

    RTC_DATE_TIME   curDateTime;
  #if CDVR_LOG
    s32             StrLen;
  #endif
  #if UART_GPS_COMMAND
    static  u8      szGPS[48];
  #endif

    uiDrawGetGSensorData(GS);

    RTC_Get_Time(&curDateTime);
  #if (HW_BOARD_OPTION==MR9670_COMMAX) || (HW_BOARD_OPTION == MR9670_COMMAX_WI2)
    sprintf (timeForRecord1, "20%02d-%02d-%02d %02d:%02d:%02d",
        curDateTime.year, curDateTime.month, curDateTime.day,
        curDateTime.hour, curDateTime.min, curDateTime.sec);
  #else
    sprintf (timeForRecord1, "20%02d/%02d/%02d %02d:%02d:%02d",
        curDateTime.year, curDateTime.month, curDateTime.day,
        curDateTime.hour, curDateTime.min, curDateTime.sec);
  #endif
    if (strcmp(timeForRecord1, timeForRecord2))
    {
        strcpy(timeForRecord2, timeForRecord1);

    #if UART_GPS_COMMAND
        if (gGPS_data1.valid == 1)
        {
            //copy data to
            //DEBUG_SYS("T");
            OSSemPend(GPSUpdateEvt, 5, &err);
            memcpy(&gGPS_data2, &gGPS_data1, sizeof(GPS_DATA));
            gGPS_data1.valid    = 0;
            OSSemPost(GPSUpdateEvt);
        }

        if  (gGPS_data2.valid)
        {
            //DEBUG_SYS("O");
            gGPS_data2.valid    = 0;
            sprintf(szGPS, "%c%02d %02d.%03d %c%03d %02d.%03d",
                    gGPS_data2.N_S, (gGPS_data2.Lat_I /100) & 0xff, (gGPS_data2.Lat_I %100) & 0xff, gGPS_data2.Lat_F/10,
                    gGPS_data2.E_W, (gGPS_data2.Lon_I/100) & 0xff, (gGPS_data2.Lon_I %100) & 0xff, gGPS_data2.Lon_F/10);
            DEBUG_SYS("%s\n", szGPS);
        }
        else if(strlen(szGPS) == 0)
        {
            sprintf(szGPS,"%c%02d %02d.%03d %c%03d %02d.%03d",
                    'N', 0, 0, 0,
                    'E', 0, 0, 0);
            DEBUG_SYS("%s\n", szGPS);
        }
    #endif
    }
#if (G_SENSOR!=G_SENSOR_NONE)
    if (1)    // 全部顯示
    {
    #if(ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_8x16x95_2BIT)
      //#if UART_GPS_COMMAND
      #if 0
        sprintf (szVideoOverlay1, "%s %s %s %s %s                %s", VERNUM, timeForRecord1, GS, ((OSTime >> 3) & 1) ? "Rec" : "   ", BuzzerToggle ? "BZ ON " : "BZ OFF", szGPS);
      #else
        sprintf (szVideoOverlay1, "%s %d %s %s %s %s", VERNUM, chacknum, timeForRecord1, GS, ((OSTime >> 3) & 1) ? "Rec" : "   ", BuzzerToggle ? "BZ ON " : "BZ OFF");
      #endif
    #else
        sprintf (szVideoOverlay1, "%s             %s        ", timeForRecord1, GS);
    #endif
    }
    else    // 只秀時間
    {
    #if(ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_8x16x95_2BIT)
      //#if UART_GPS_COMMAND
      #if 0
        sprintf (szVideoOverlay1, "%s %s                        %s %s                %s", VERNUM, timeForRecord1, ((OSTime >> 3) & 1) ? "Rec" : "   ", BuzzerToggle ? "BZ ON " : "BZ OFF", szGPS);
      #else
        sprintf (szVideoOverlay1, "%s %s                        %s %s", VERNUM, timeForRecord1, ((OSTime >> 3) & 1) ? "Rec" : "   ", BuzzerToggle ? "BZ ON " : "BZ OFF");
      #endif
    #else
        sprintf (szVideoOverlay1, "%s                      ", timeForRecord1);
    #endif
    }
#else
    sprintf (szVideoOverlay1, "%s", timeForRecord1);
#endif
#if CDVR_LOG
    OSSemPend(LogFileSemEvt, 10, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SYS("uiDrawTimeOnVideoClip() Error: LogFileSemEvt is %d.\n", err);
        return;
    }
    sprintf(szLogString,"%s                %s", szVideoOverlay1, szGPS);
    strcpy(szLogFile, szLogString);
    StrLen      = strlen(szLogString);
    strcpy(szLogFile + StrLen, "\r\n");
    szLogFile  += StrLen + 2;
    if(szLogFile >= (LogFileBufEnd - MAX_OVERLAYSTR * 2))
    {
        pLogFileMid                     = szLogFile;
        szLogFile                       = LogFileBuf;
    }
    OSSemPost(LogFileSemEvt);
#endif
    if(OSDTimestameLevel != UI_MENU_VIDEO_OVERWRITESTR_NONE)
    {
        if (strcmp(szVideoOverlay1, szVideoOverlay2) && Param)
        {
            strcpy(szVideoOverlay2, szVideoOverlay1);
        #if ISU_OVERLAY_ENABLE
        #if 0
            GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
                            96,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                            1,12,1+24,11+4);
        //ASCII_FONT_32x40x95_2BIT
           GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
            48,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
            1,1,1+16,0+4);
        #else
          #if (HW_BOARD_OPTION == MR9670_COMMAX) || (HW_BOARD_OPTION == MR9670_COMMAX_WI2)
            if(OverwriteStringEnable)
            {
                DEBUG_UI("szVideoOverlay1 = %s\n", szVideoOverlay1);
                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                    10,20,10+24,19+2);
            } else {
                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, "                    ",
                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                    10,20,10+24,19+2);
            }
          #else
            GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, szVideoOverlay1,
                288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                20,40,20+96,39+4);
          #endif
        //ASCII_FONT_8x16x95_2BIT
        #endif
        //    GenerateOverlayImage(ScalarOverlayImage, szVideoOverlay1, MAX_OVERLAYSTR, ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT, 640);
        #endif
        }
    }

#endif
}


#if (SUPPORT_TOUCH == 1)
/*

Routine Description:

	Check Touch Key and send UI key

Arguments:

	None.

Return Value:

	0 - Not in touch range.
	1 - Success.
	2 - UI busy

*/


u8  uiFlowCheckTouchKey(int TouchX, int TouchY)
{
    //uiOsdDrawPosition(TouchX, TouchY);
#if 1
    UITOUCH_NODE_EVENT_TBL  *nodeTouchInfo;
    u8  i;
    u32 GetKey = UI_KEY_READY;
    u32 uiRightDown_X, uiRightDown_Y;
    static u8 cnt=0;

    
    if (UIKey != UI_KEY_READY)
    {
        DEBUG_UI("uiFlowCheckTouchKey busy \n");
        return 2;
    }

    //DEBUG_UI("**current node: %s  \n",uiCurrNode->item.NodeData->Node_Name);

    if ((uiCurrNode->parent->item.NodeData->Action_no != UI_MENU_SETIDX_DATE_TIME) && 
       (uiCurrNode->parent->item.NodeData->Action_no != UI_MENU_SETIDX_SCHEDULED_SET))
    {
        if(touch_press == TRUE)
        {
            cnt++;
            if(cnt > 6)
                cnt=6;
        }
        else
            cnt=0;    
    }

    nodeTouchInfo = uiCurrNode->item.NodeData->TouchData;
    if (nodeTouchInfo == NULL)
    {
        DEBUG_UI("Current Node do not have touch range \n");
        return 0;
    }
    for (i = 0; i < nodeTouchInfo->uiCheckEventNum; i++)
    {
        uiRightDown_X = nodeTouchInfo->pTouchNodeEvent[i].uiIconWidth+nodeTouchInfo->pTouchNodeEvent[i].uiLeftUp_X;
        uiRightDown_Y = nodeTouchInfo->pTouchNodeEvent[i].uiIconHeight+nodeTouchInfo->pTouchNodeEvent[i].uiLeftUp_Y;
#if 0
        DEBUG_UI("\n");
        DEBUG_GREEN("===> nodeTouchInfo->pTouchNodeEvent[i].uiLeftUp_X = %d\n",nodeTouchInfo->pTouchNodeEvent[i].uiLeftUp_X);
        DEBUG_GREEN("===> uiRightDown_X = %d\n",uiRightDown_X);
        DEBUG_GREEN("===> nodeTouchInfo->pTouchNodeEvent[i].uiLeftUp_Y = %d\n",nodeTouchInfo->pTouchNodeEvent[i].uiLeftUp_Y);
        DEBUG_GREEN("===> uiRightDown_Y = %d\n",uiRightDown_Y);
#endif
        if ((TouchX >= nodeTouchInfo->pTouchNodeEvent[i].uiLeftUp_X) && 
            (TouchY >= nodeTouchInfo->pTouchNodeEvent[i].uiLeftUp_Y)&&
            (TouchX <= uiRightDown_X)&&
            (TouchY <= uiRightDown_Y))
        {
            GetKey = nodeTouchInfo->pTouchNodeEvent[i].uiKeyEvent;
            TouchExtKey = nodeTouchInfo->pTouchNodeEvent[i].uiNodeNum;
            UiGetTouchX = TouchX;
            UiGetTouchY = TouchY;
            break;
        }
    }
    if (GetKey == UI_KEY_READY)
    {

        DEBUG_UI("Current Node not in touch range \n");
        return 0;
    }

    if ((uiCurrNode->parent->item.NodeData->Action_no != UI_MENU_SETIDX_DATE_TIME) &&
        (uiCurrNode->parent->item.NodeData->Action_no != UI_MENU_SETIDX_NTP) &&
       (uiCurrNode->parent->item.NodeData->Action_no != UI_MENU_SETIDX_SCHEDULED_SET))
    {
        if(cnt == 0)
        {
            if (uiSentKeyToUi(GetKey) == 0)
            {
                DEBUG_UI("Get Touch key %d Fail\n",GetKey);
                return 2;
            }
            else
            {
                DEBUG_UI("Get Touch key %d Success\n",GetKey);
                return 1;
            }    
        }
    }
    else
    {
        if (uiSentKeyToUi(GetKey) == 0)
        {
            DEBUG_UI("Get Touch key %d Fail\n",GetKey);
            return 2;
        }
        else
        {
            if (touch_press == FALSE)
                uiConTouchPress = 0;
            else
                uiConTouchPress = 1;
            DEBUG_UI("Get Touch key %d Success Node %s\n",GetKey, uiCurrNode->item.NodeData->Node_Name);
            return 1;
        }
    }
    return 0;

#endif
}
#endif

void uiFlowCardReady(u8 CardState)
{
    if (CardState == SDC_CD_IN)
    {
        uiFlowCheckRecState();
    }
}

void uiCheckTXUpgradeFileName(void)
{
    FS_FILE*    pFile;
    u8 i, tmp, changeFlag = FALSE;
    s8 showMsg[32] = {0};
    s8 showMsg2[32] = {0};
    s8 RenameVerStr[MAX_RFIU_UNIT][32] = {
                                         "Fin_TX01.bin",
                                         "Fin_TX02.bin",
                                         "Fin_TX03.bin",
                                         "Fin_TX04.bin"
                                       };
    s8 str[MAX_RFIU_UNIT][32] =  {
                                 "\\TX1.bin",
                                 "\\TX2.bin",
                                 "\\TX3.bin",
                                 "\\TX4.bin"
                                };

    if (Main_Init_Ready  == 0)
        return;

    memcpy(ispTxFWFileName, str, sizeof(ispTxFWFileName));
    for (i=0; i<MAX_RFIU_UNIT; i++)
    {
        if (gRfiu_Op_Sta[i] != RFIU_RX_STA_LINK_OK)
        {
            DEBUG_YELLOW("Cam %d Link Broken In Upgrade \n", i);
            continue;
        }
        
        if ((pFile = dcfOpen((signed char*)ispTxFWFileName[i], "rb")) != NULL)
        {
            dcfClose(pFile, &tmp);
            #if 1
            if (changeFlag == FALSE)
            {
                if ((MyHandler.MenuMode == VIDEO_MODE) || (MyHandler.MenuMode == QUAD_MODE))
                    lastMenuMode = MyHandler.MenuMode;
                uiEnterMenu(UI_MENU_NODE_PREVIEW_MODE);
                uiFlowEnterMenuMode(SETUP_MODE);
                uiOsdDrawInit();
                changeFlag = TRUE;
            }
            #endif
            //DEBUG_UI("Find CAM %d SD Upgrade File !! \r\n", i);
            sprintf(showMsg, "Camera %d FW updating", i + 1);
            uiOSDASCIIStringByColorCenter(showMsg, OSD_L1Blk0 , 0xC0, 0x85);
            
            if (rfiuTxFwUpdateFromSD(i) == TRUE)
            {
                OSTimeDly(400);
                DEBUG_UI("rfiuTxFwUpdateFromSD CAM %d Successed !! \r\n", i);
                sprintf(showMsg2, "Update successful", i + 1);
                dcfRename((signed char*)RenameVerStr[i],(signed char*)ispTxFWFileName[i]);
            }
            else
            {
                DEBUG_UI("rfiuTxFwUpdateFromSD CAM %d Fail !! \r\n", i);
                sprintf(showMsg2, "Update Failed");
            }
            uiOSDASCIIStringByColorCenter(showMsg, OSD_L1Blk0 , 0x00, 0x00);
            uiOSDASCIIStringByColorCenter(showMsg2, OSD_L1Blk0 , 0xC0, 0x85);
            OSTimeDly(40);
        }
        else
            DEBUG_UI("[SD Upgrade] Cam %d No File !! \r\n", i);
    }
    
    if (changeFlag == TRUE)
        uiFlowSetupToPreview();
}


