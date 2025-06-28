/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    uiapi.h

Abstract:

    The application interface of user interface.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#ifndef __UI_API_H__
#define __UI_API_H__

#include "task.h"
#include "uiKey.h"
#include "fsapi.h"

#if(HOME_RF_SUPPORT)
#include "rfiuapi.h"
#endif

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */
#define FLAGUI_MAIN_INIT_READY          0x00000001
#define FLAGUI_SD_GOTO_FORMAT           0x00000002
#define FLAGUI_UI_COMMAND_FINISH        0x00000004
#define FLAGUI_UI_CHANGE_MODE           0x00000008
#define FLAGUI_CHANGE_REC_MODE          0x00000010
#define FLAGUI_SD_INIT                  0x00000020
#define FLAGUI_FORMATTING               0x00000040
#define FLAGUI_CLK_IMG_DISAED           0x00000080
#define FLAGUI_UI_USE_FLASH             0x00000100
#define FLAGUI_RF_PAIR_SUCCESS          0x00000200
#define FLAGUI_RF_CHANGE_CAMERA         0x00000400
#define FLAGUI_UI_CHANGE_SET            0x00000800

#define UI_VERSION_BIT_LIGHT            0x00000001
#define UI_VERSION_BIT_CA               0x00000002 //CAMERA ALARM

/* OSD Define */
#define OSD_Blk0    0
#define OSD_Blk1    1
#define OSD_Blk2    2
#define OSD_L1Blk0  3
#define OSD_L2Blk0  4

#define OSD_STRING_W    8
#define OSD_STRING_H    16

#define OSD_Width     PANNEL_X
#define OSD_Height    PANNEL_Y

/* UI Invalid value*/
#define UI_INVALID_VAL 0xFF /*for u8 size*/

/*P2P related*/
#define UI_P2P_PSW_MAX_LEN 33

/* Status Flag */
enum
{
    UI_SET_RF_OK = 0,
    UI_SET_RF_BUSY,
    UI_SET_RF_NO_NEED
};


/*Message Type for multi-language*/
#define CENTERED      0xFFFF


enum
{
    alpha_0=0,       /*   0%   */
    alpha_1,         /*  30%   */
    alpha_2,         /*  60%   */
    alpha_3,         /*  100%  */
    alpha_OFF,       /*No Alpha*/
};

enum 
{
    UI_MENU_VIDEO_SIZE_640x480 = 0,
    UI_MENU_VIDEO_SIZE_720x480,         /*  不能使用,A1016硬體限制   */
    UI_MENU_VIDEO_SIZE_720x576,         /*  不能使用,A1016硬體限制   */
    UI_MENU_VIDEO_SIZE_704x480,
    UI_MENU_VIDEO_SIZE_704x576,
    UI_MENU_VIDEO_SIZE_352x288,
    UI_MENU_VIDEO_SIZE_352x240,
    UI_MENU_VIDEO_SIZE_320x240,
    UI_MENU_VIDEO_SIZE_1280X720,
    UI_MENU_VIDEO_SIZE_1920x1072,
    UI_MENU_VIDEO_SIZE_640x352,
    UI_MENU_VIDEO_SIZE_1600x896,
    UI_MENU_VIDEO_SIZE_2688x1520,
};

typedef enum 
{
    UI_OSD_DRAW = 0,
    UI_OSD_CLEAR,
    UI_OSD_NONE,

} UI_OSD_ACT;

typedef enum 
{
    UI_GRAPH_DRAW = 0,
    UI_GRAPH_CLEAR,
    UI_GRAPH_NONE,
} UI_GRAPH_ACT;

enum 
{
    UI_DSP_PLAY_LIST_DIR = 0,
    UI_DSP_PLAY_LIST_FILE,
    UI_DSP_PLAY_LIST_DOOR_PIC,
    UI_DSP_PLAY_LIST_DOOR_ALB,
    UI_DSP_PLAY_LIST_PLAYBACK,
    UI_DSP_PLAY_LIST_DOOR_SELECT,
    
};

/*enum
{
    UI_MENU_SETIDX_SCHEDULE_MOTION_OFF = 0,
    UI_MENU_SETIDX_SCHEDULE_MOTION_ON,
};*/


enum 
{
    UI_SCH_MOTION_OFF = 0,
    UI_SCH_MOTION_ON,
};
enum 
{
    UI_RESOLTUION_HD = 0,
    UI_RESOLTUION_VGA,
    UI_RESOLTUION_QVGA,
    UI_RESOLUTION_D1_480V,
    UI_RESOLUTION_D1_576V,
    UI_RESOLTUION_FHD,
    UI_RESOLTUION_QHD,
    UI_RESOLTUION_1600x896,
    UI_RESOLTUION_4M,
};

enum 
{
    UI_MOTION_OFF = 0,
    UI_MOTION_ON,
};

enum 
{
    UI_PIR_OFF = 0,
    UI_PIR_ON,
};

enum 
{
    UI_NETWORK_UP = 0,
    UI_NETWORK_DOWN,
};


enum 
{
    UI_MOTION_SENSITIVITY_H = 0,
    UI_MOTION_SENSITIVITY_M,
    UI_MOTION_SENSITIVITY_L
};

enum 
{
    UI_P2P_STATUS_NONE = 0,  /*not connect*/
    UI_P2P_STATUS_LEVEL_1,
    UI_P2P_STATUS_LEVEL_2,
    UI_P2P_STATUS_LEVEL_3,
    UI_P2P_STATUS_LEVEL_4,
    UI_P2P_STATUS_LEVEL_5,
};

#if 1
enum 
{
    UI_RF_STATUS_NO_SINGLE = 0,
    UI_RF_STATUS_LINK,
    UI_RF_STATUS_OTHER,
};
#endif


enum
{
    UI_REC_STATUS_NONE =0,
    UI_REC_STATUS_RECING,
};



enum
{
    UI_BATTERY_LV0=0,
    UI_BATTERY_LV1,    
    UI_BATTERY_LV2,
    UI_BATTERY_LV3,
    UI_BATTERY_LV4,
    UI_BATTERY_LV5,
    UI_BATTERY_CLEAR,
    UI_BATTERY_CHARGE,
    UI_BATTERY_SHUTDOWN,
    UI_BATTERY_NONE,
};


enum
{
    UI_ALARM_ON =0,
    UI_ALARM_OFF,
};

enum
{
    UI_PLAY_ICON_PLAY =0,
    UI_PLAY_ICON_PAUSE,
    UI_PLAY_ICON_STOP,
    UI_PLAY_ICON_CLEAN,
};

enum
{
    UI_TIME_STAMP_TYPE_24_YMD =0,
    UI_TIME_STAMP_TYPE_24_DMY,
    UI_TIME_STAMP_TYPE_24_MDY,
    UI_TIME_STAMP_TYPE_12_YMD =8,
    UI_TIME_STAMP_TYPE_12_DMY,
    UI_TIME_STAMP_TYPE_12_MDY,
};

enum
{
    UI_DEGREE_TYPE_CENTIGRADE = 0,
    UI_DEGREE_TYPE_FAHRENHEIT,   
};

#if DOOR_BELL_SUPPORT
enum
{
    BELLINLIST_YES =0,
    BELLINLIST_NO,
    BELLINLIST_FULL,
};

enum
{
    BELL_LIST_1 =0,
    BELL_LIST_2,
    BELL_LIST_3,
    BELL_LIST_4,
    BELL_LIST_5,
    BELL_LIST_ADD,
    BELL_LIST_DEL,
};

enum
{
    BELL_POWER_OFF = 0,
    BELL_NO_SIGNAL,
    BELL_SOS,
    BELL_DOOR,
    BELL_BLANKET,
    BELL_BLANKET2,
};

#endif

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))

	typedef struct
	{
	  u8          file_name[12];         /* Indicate the UI file name */
	  u32        file_length;             /* Indicate the UI file length */
	  u32        len_in_nand;             /* Indicate the file length in NAND flash*/
	  u32        sector_addr;             /* Indicate the real sector address in NAND */
	  u32        fb_magic;
	} FRAME_BUF_OBJECT;

#elif ((FLASH_OPTION == FLASH_SERIAL_EON)|| (FLASH_OPTION == FLASH_SERIAL_SST)||(FLASH_OPTION == FLASH_SERIAL_WINBOND)||(FLASH_OPTION == FLASH_SERIAL_ESMT))

	typedef struct
	{
		u8	stFileName[12]; 	/* Indicate the UI file name */
		u32 stFileLen;			/* Indicate the UI file length */
		u32 stLenInSPI; 		/* Indicate the file length in serial flash*/
		u32 stPageStartAddr; 	/* Indicate the real page address in serial flash */
	} FRAME_BUF_OBJECT;

#else
    typedef struct
	{
		u8	stFileName[12]; 	/* Indicate the UI file name */
		u32 stFileLen;			/* Indicate the UI file length */
		u32 stLenInSPI; 		/* Indicate the file length in serial flash*/
		u32 stPageStartAddr; 	/* Indicate the real page address in serial flash */
	} FRAME_BUF_OBJECT;
#endif

typedef struct
{
		u32	stStart;		/* The start addr of the character's file */
		u32	stCount;		/* The count of the character's file */
}UI_FILE_CHAR_COUNT;


typedef enum Ui_ValueCtrl
{
	UI_VALUE_CURRENT = 0,
	UI_VALUE_ADD,       
	UI_VALUE_SUBTRACT,
	UI_VALUE_CLEAN
} UI_VALUECTRL;

//Added by aher 2013/03/22
typedef struct NetworkInfo{
	u8 IPaddr[4];
	u8 Netmask[4];
	u8 Gateway[4];
    u8 DNS1[4];
    u8 DNS2[4];
	u8 IsStaticIP;
    u8 MACAddr[6];
} UI_NET_INFO;

#if DOOR_BELL_SUPPORT

typedef struct DoorBellInfo {
    u8 DevType;
    u8 DevPower;
    u32 DevID;
    u8	hour;	/* 0 - 23 */
    u8	min;	/* 0 - 59 */
    bool pushLowLV;
} Bell_table;

#endif

/*
 *********************************************************************************************************
 * Extern Global Variable
 *********************************************************************************************************
 */
extern OS_STK uiTaskStack[UI_TASK_STACK_SIZE]; /* Stack of task uiTask() */
extern OS_EVENT* uiSemEvt;         /* semaphore to synchronize event processing */
extern OS_EVENT* message_MboxEvt;
extern OS_EVENT* uiOSDSemEvt;
extern OS_FLAG_GRP  *gUiStateFlagGrp;
extern OS_FLAG_GRP  *gUiKeyFlagGrp;
extern OS_EVENT* uiAlarmSemEvt;	/* semaphore to synchronize event processing of alarm-out buzzer ctrl */
extern u32 UIKey;
extern u32 UISubKey,UISubKey1,UISubKey2;
extern u32 MsgKey;
extern u32 SpecialKey;
extern RTC_DATE_TIME KeyTime;
extern u8 iconflag[UIACTIONNUM];
extern u8 start_iconflag[UIACTIONNUM];
extern u8 batteryflag;
extern u8 CurrLanguage;
extern u8 playbackflag;
extern u8 uiMenuEnable;
extern u8 TvOutMode;
extern u8 UISetRFMode;
extern s8 gsParseDirName[9];

extern u16 OSDDispWidth[];
extern u16 OSDDispHeight[];
extern u16 graphDispWidth[];
extern u16 graphDispHeight[];
extern u8  osdYShift;

extern u8 OSD_WARNING[16*24];
extern s8 defaultvalue[];
extern u8 Current_Alarm_Period;
extern u8 gu8TimeStamp;
extern u8 uiISStatic;
extern u8 uiMenuVideoSizeSetting;
extern u8 uiRFStatue[MAX_RFIU_UNIT];
extern u8 uiP2PMode;
extern u8 uiVersion[32];
extern u8  uiVersionTime[];
extern u32 prev_P2pVideoQuality;
#if UI_LIGHT_SUPPORT
    #if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
    extern u8  uiSetRfLightTimer[MULTI_CHANNEL_MAX];
    #elif (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
    extern u8  uiSetRfLightTimer[MULTI_CHANNEL_MAX][7];
    #endif
extern u8  uiSetRfLightDimmer[MULTI_CHANNEL_MAX];
extern u8  uiSetRfLightDur[MULTI_CHANNEL_MAX];
#endif
extern u8  uiSetRfLightState[MULTI_CHANNEL_MAX];

extern u8  uiEnterScanMode;
#if RFIU_RX_WAKEUP_TX_SCHEME
extern u8 uiPIRScheduleOnOff[MAX_RFIU_UNIT];
#endif
extern u8 showmotion;

extern u8  uiSetRfLightSwitch[MULTI_CHANNEL_MAX];
extern u8  uiSetRfAlarmSwitch[MULTI_CHANNEL_MAX];
extern u8  uiSetRfAlarmTimer[MULTI_CHANNEL_MAX][7];
extern u8  uiSetRfAlarmState[MULTI_CHANNEL_MAX];
extern s8  BoxExtKey;

#if DOOR_BELL_SUPPORT
extern Bell_table DoorBellUnit[DOOR_BELL_NUM];
extern bool BellPairMode;
extern u8 BellListSel[DOOR_BELL_NUM];
extern bool dortest;
extern u32  DorBel_TriID;
#endif

/*
 *********************************************************************************************************
 * Function Prototype
 *********************************************************************************************************
 */
s32 uiMenuAction(s8 setidx);



/*
 *********************************************************************************************************
 * Extern Function Prototype
 *********************************************************************************************************
 */
extern s32 uiCheckSDCD(u8 mode);
#if USB_HOST_MASS_SUPPORT
extern s32 uiCheckUSBCD(u8 mode);
#endif
extern s32 uiInit(void);
extern void uiMenuEnterPreview(u8 mode);
extern void uiMenuOSDFrame(u16 osd_w , u16 icon_w , u16 icon_h , u16 x_pos , u16 y_pos , u8 buf_idx , u32 data);
extern void uiMenuOSDShiftY(u16 osd_w , u16 *sy, u16 *ey , u8 shift_h, u8 buf_idx);
extern void uiMenuOSDReset(void);
extern void osdDrawFlashLight(u8);
extern u8 Get_Node_Total_Index(void);
extern void Read_FB_Setting(void);
extern void Read_UI_Setting(void);
extern s32 Save_UI_Setting(void);
extern s32 Save_UI_Setting_Task(s32 dummy);
extern BOOLEAN uiReadVideoFile(void);
extern BOOLEAN uiPlaybackInit(s32 param, BOOLEAN wait);
extern void uiWaitAnyKey(void);
extern u8 uiCaptureVideoStop(void);
extern void osdDrawFSWaitingBar(u32,  u32, u8, u16, u8);
extern s32 uiMenuSet_REC_MODE(u8);
extern u8 uiPlaybackStop(u8);
extern u32 uiGetMenuMode(void);
extern s8 uiCheckSDCardStatForUsb(void);
extern void uiWaitMainInitReady(void);
extern void osdDrawVideoOn(u8 on);
extern void osdDrawFillEmpty(void);
extern void osdDrawMemFull(u8 act);
extern void osdDrawPreviewIcon(void);
extern void osdDrawPlayIcon(void);
extern void osdDrawProtect(u8 mode);
extern void uiOsdDrawInsertSD(u8 buf_idx);
extern void uiOsdDrawBitRate(u32 value);
extern void uiOsdDrawFrameRate(u32 value);
#if MULTI_CHANNEL_VIDEO_REC
extern void uiOsdDrawSysPerRec(void* pData);
extern void uiOsdDrawSysAfterRec(void* pData);
#endif
extern u8 uiSetGoToFormat(void);
extern void uiClearOSDBuf(u8 blk_idx);
extern s32 uiKeyVideoCapture(void);
extern void uiDiskFreeforVideoClip(char * time_str);
extern s8 uiCompareTwoTime(RTC_DATE_TIME* time1, RTC_DATE_TIME* time2);
extern u8 uiCompareSaveData(void);
extern u8 uiGetSaveChecksum(void);
extern void osdDrawPlaybackArea(u8 mode);
extern void osdDrawFillWait(void);
extern void osdDrawFillUSBMSC(void);
extern u8 uiSentKeyToUi(u32 Key);
extern void uiCheckTVInFormat(void);
extern u32 uiCheckVideoRec(void);
extern u32 uiCheckPIRSchedule(int CamID);
extern void uiFliowHDDRemove(void);
extern void uiFlowRunPerSec(void);
extern u8 uiReadSettingFromFlash(u8 *FlashAddr);
extern void uiFlowSdCardMode(void);
extern void uiReadRFIDFromFlash(u8 *FlashAddr);
extern void uiReadNetworkIDFromFlash(u8 *FlashAddr);
extern void uiReadVersionFromFlash(u8 * FlashAddr);
extern void uiWriteRFIDFromFlash(u8 *FlashAddr);
extern void uiWriteRFCALFromFlash(u8 *FlashAddr);

extern void uiReadRFCALFromFlash(u8 *FlashAddr);

extern void uiFlowEnterMenuMode(u32 Mode);

extern void uiWriteNetworkIDFromFlash(u8 *FlashAddr);
extern u8 uiSetDefaultSetting(void);
extern u8 uiWriteSettingToFlash(u8 *FlashAddr);   
extern u8 uiCaptureVideoStopByChannel(u8 Ch_ID);
extern u8 uiCaptureVideoByChannel(u8 Ch_ID);
extern u32 uiCheckPlayback(void);

extern u8 uiOSDMultiLanguageStrCenter(MSG_SRTIDX str_inx, u8 buf_idx, u8 str_color, u8 bg_color);
extern u8 uiOSDMultiLanguageStrByXY(MSG_SRTIDX str_inx, u16 x_pos, u16 y_pos, u8 buf_idx, u8 str_color, u8 bg_color);
extern u8 uiOSDMultiLanguageStrByY(MSG_SRTIDX str_inx, u16 y_pos, u8 buf_idx, u8 str_color, u8 bg_color);
extern u8 uiOSDMultiLanguageStrByX(MSG_SRTIDX str_inx, u16 x_pos, u8 buf_idx, u8 str_color, u8 bg_color);
extern u8 uiOSDASCIIStringByColorX(u8 *string, u16 x_pos , u8 buf_idx , u8 str_color, u8 bg_color);
extern u8 uiOSDASCIIStringByColorY(u8 *string, u16 y_pos, u8 buf_idx , u8 str_color, u8 bg_color);
extern u8 uiOSDASCIIStringByColorCenter(u8 *string, u8 buf_idx , u8 str_color, u8 bg_color);
extern u8 uiOSDASCIIStringByColor(u8 *string, u16 x_pos , u16 y_pos, u8 buf_idx , u8 str_color, u8 bg_color);
extern u8 uiOSDIconColorByXY(OSD_ICONIDX icon_inx ,u16 x_pos, u16 y_pos, u8 buf_idx, u8 bg_color , u8 alpha);
extern u8 uiOSDIconColorByX(OSD_ICONIDX icon_inx ,u16 x_pos,  u8 buf_idx, u8 bg_color , u8 alpha);
extern u8 uiOSDIconColorByY(OSD_ICONIDX icon_inx , u16 y_pos, u8 buf_idx, u8 bg_color , u8 alpha);
extern u8 uiOSDIcon(u16 osd_w, u16 icon_inx, u16 x_pos, u16 y_pos, u8 buf_idx, u8 bg_color, u8 alpha);
extern void uiOsdEnable(u8 osd);
extern void uiOsdDisable(u8 osd);
extern void uiOsdEnableAll(void);
extern u8 CheckCurrResolution(void);
extern void uiOsdDisableAll(void);
extern void uiOSDIconColorByXYChColor(OSD_ICONIDX icon_inx, u16 x_pos , u16 y_pos , u8 buf_idx, u8 bg_color, u8 alpha, u8 font_old_color, u8 font_new_color);
extern u8 uiGetP2PStatueToRF(void);
extern u8  uiSynRFConfigInP2P(u8 camId);
extern s32 uiSetRfTimeRxToTx(s32 camera);
extern u8 uiSetTalkOnOff(void);

extern u8 uiCaptureVideo(void);
extern void uiGraphGetNetworkInfo(u8* ip, u8* submask, u8* defaultGateway);
extern void osdDrawSDCD(u8 i);
extern void osdDrawFileNum(u32 num);
extern void osdDrawPlayIndicator(u8 type);
extern void osdDrawVideoTime(void);
extern void osdDrawVideoIcon(void);
extern void osdDrawSDIcon(u8 on);
extern void uiOSDPreviewInit(void);
extern void osdDrawRemoteOn(u8 on);
extern void uiOsdDrawBattery(u8 level);
extern void osdDrawMessage(MSG_SRTIDX strIdx, u16 x_pos , u16 y_pos, u8 buf_idx, u8 obj_color, u8 bg_color);
extern void uiOsdDrawNewFile(void);

#if ERASE_SPI
extern s8 	uiEraseSpiWholeChip(void);
#endif
extern void uiDrawTimeOnVideoClip(s32 Param);
extern void osdDrawDelMsg(s8* msg,u32 index);
extern s32 uiMenuSet_VideoSize(s8 setting);
extern s32 uiMenuSet_TX_BRIGHTNESS(s8 setting);
extern s32 uiMenuSet_TX_MOTION(s8 setting,int Day_Level,int Night_Level);
extern s32 uiMenuSet_TX_FLICER(s8 setting);
extern void uiOSDDrawRectangle(u16 osd_w , u16 icon_w , u16 icon_h , u16 x_pos , u16 y_pos , u8 buf_idx, u32 color, u8 thick);
extern u32 uiSetZoomMode(u8 Channel, u32 ZoomX, u32 ZoomY, u8 act);
extern s32 uiMenuSet_IR_Mode(u8 setting);
extern s32 uiMenuSet_Light_Status(u8 setting);
extern s32 uiMenuSet_Light_R(u8 setting);
extern s32 uiMenuSet_Light_G(u8 setting);
extern s32 uiMenuSet_Light_B(u8 setting);
extern s32 uiMenuSet_RecordMode(u8 setting);
extern s32 uiMenuSet_MountMode(u8 setting);
extern void uiOutputRedirection(void);

#if (SUPPORT_TOUCH == 1)
    extern u8  uiFlowCheckTouchKey(int TouchX, int TouchY);
#endif
#if (RFIU_SUPPORT)
    extern u8 uiSetRfResolutionRxToTx(s8 setting,u8 camera);
    extern u8 uiSetRfMotionRxToTx(s8 Enable, u8 dayLev, u8 nightLev, u8 camera);
    extern u8 uiSetRfPIRRxToTx(s8 Enable, u8 camera);
    extern u8 uiCheckRfTalkStatus(void);
    extern u8 uiSetRfBrightnessRxToTx(s8 brivalue,u8 camera);
    extern u8 uiSetRfLightDimmerRxToTx(u8 Val, u8 camera);
    extern u8 uiSetRfLightDurationRxToTx(u8 Val, u8 camera);
    extern u8 uiSetRfTimeStampTypeRxToTx(u8 Type, u8 camera);
#if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_01MIN)
    extern u8 uiSetRfLightTimerRxToTx(u8 Hour1, u8 Min1, u8 Hour2, u8 Min2, u8 Week, u8 camera, u8 isSyn);
#elif (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
    extern u8 uiSetRfLightTimerRxToTx(u8 week, u8 part1, u8 part2, u8 part3, u8 part4, u8 part5, u8 part6, u8 camera, u8 isSyn);
#endif
    extern u8 uiSetRfManualLightingRxToTx(u8 Enable, u8 camera);
    extern u8 uiSetRfMotionMaskArea(u8 camera, u8 Type);
    extern u8 uiSetRfLightingRxToTx(u8 Enable, u8 camera);
    extern u8 uiSetRfFlickerRxToTx(u8 setting);
#endif
#if NIC_SUPPORT
    extern void uiSetP2PImageLevel(u8 CamId, u8 level);
    extern u8 uiSetP2PPassword(u8 *password);
#endif
#if UI_LIGHT_SUPPORT
extern void uiOsdDrawLight(u8 act);
#if (UI_LIGHT_TIME_MIN_FORMAT == UI_TIME_INTERVAL_30MIN)
extern void uiSyncLightTimer2LightInterval(u8 nCamCount, u8 u8Enable);
#endif
#endif
extern void uiSetAudioByCH(u8 channel, u8 level);
extern void uiOsdDrawNetworkLink(u8 LinkUp);
extern void uiOsdDrawSDCardFail(u8 act);
extern void uiOsdDrawRemindDownload(u8 RemindDown);
extern void uiFlowCardReady(u8 CardState);
extern s32 uiClearFfQuadBuf(s32 index);

#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
extern void uiCheckUpgradeFileName(FS_DIRENT *pDirEnt, u32 pDirCount);
#elif (UI_VERSION == UI_VERSION_TRANWO)
extern void uiCheckTXUpgradeFileName(void);
#endif

#if (UI_BAT_SUPPORT) 
extern void uiOsdDrawCameraBatteryLevel(u8 camID,u8 act);
#endif
extern u8 uiFlowSetCmdToUI(u8 Cam, u8 Key, u8 SetVal);
extern u8 uiFlowGetUISetting(u8 Cam, u8 Key, u8 *SetVal);
#if (A1025_GATE_WAY_SERIES)
extern void uiFlowSetDefault(void);
#endif

#if DOOR_BELL_SUPPORT
extern void DoorBellinit(void);
#endif


extern bool showSPKLev;
extern bool PlaybackDBG;

#if UI_CAMERA_LIGHT_SUPPORT
extern u8  uiLightInterval[MULTI_CHANNEL_MAX][7][6]; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */
#endif

#if UI_CAMERA_ALARM_SUPPORT
extern u8  uiCamAlarmInterval[MULTI_CHANNEL_MAX][7][6]; /* 7 days a week, 24 hours a day and divide to [0] = 11111111 : light at 0~4am, ... [5] = 00000000 no light at 8~12pm */
#endif

#endif /*end of #ifndef __UI_API_H__*/
