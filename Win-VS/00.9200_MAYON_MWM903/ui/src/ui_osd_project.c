/*

  Copyright (c) 2008 Mars Semiconductor Corp.

   Module Name:

   ui_osd_project.c

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
#include "ui_project.h"
#include "dcfapi.h"
#include "sysapi.h"
#include "board.h"
#include "asfapi.h"
#include "aviapi.h"
#include "osdlib_project.h"
#include "osdlib.h"
#include "isuapi.h"
#include "iduapi.h"
#include "sdcapi.h"
#include "dcfapi.h"
#include "ciuapi.h"
#include "siuapi.h"
#include "osd_draw_project.h"
//#include "ui_menu_icon.h"
#include "rfiuapi.h"
#include "gpioapi.h"
#if (HW_BOARD_OPTION==MR9670_COMMAX) || (HW_BOARD_OPTION == MR9670_COMMAX_WI2)
    #include "MainFlow.h"
    #include "mpeg4api.h"
#endif
#if (NIC_SUPPORT)
#include "p2pserver_api.h"
#endif

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */

#define USE_BIG_OSD    1

#if(FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
#define UI_PLAYBACK_NUM_PER_PAGE        4
#else
#define UI_PLAYBACK_NUM_PER_PAGE        8
#endif
#define UI_PLAYBACK_LOC_MENU_Y          40
#define UI_PLAYBACK_LOC_MENU_FILE_X     164
#define UI_PLAYBACK_LOC_MENU_DIR_X      332
#define UI_PLAYBACK_LOC_MENU_DATE_X     340
#define UI_PLAYBACK_LOC_MENU_TIME_X     548
#define UI_PLAYBACK_LOC_MENU_FILE_Y     80
#define UI_PLAYBACK_LOC_PAGE_X          660
#define UI_PLAYBACK_LOC_PAGE_X_L        16
#define UI_PLAYBACK_LOC_PAGE_Y          412
#define UI_PLAYBACK_MENU_LEN            528
#define UI_PLAYBACK_MENU_WEIGHT         24
#define UI_PLAYBACK_MENU_DIR_LEN        144
#define UI_PLAYBACK_MENU_File_LEN       144
#define UI_PLAYBACK_DOOR_SEL_Y_WEIGHT    50
#define UI_PLAYBACK_LOC_DOOR_SEL_X      300
#define UI_PLAYBACK_LOC_DOOR_SEL_Y      200
#define UI_PLAYBACK_LOC_DOOR_SEL_Y1     (UI_PLAYBACK_LOC_DOOR_SEL_Y+UI_PLAYBACK_DOOR_SEL_Y_WEIGHT)
#define UI_PLAYBACK_LOC_DOOR_SEL_Y2     (UI_PLAYBACK_LOC_DOOR_SEL_Y1+UI_PLAYBACK_DOOR_SEL_Y_WEIGHT)
#define UI_OSD_ASCII_STRING_LOC_X        40
#define UI_OSD_ASCII_STRING_LOC_Y        12


/*------------ TOP ------------*/

#define UI_OSD_BASE_X                   32 // Up left base position of panel
#define UI_OSD_BASE_Y                   12 
#define UI_OSD_BASE_TV_X                80 // Up left base position for TV
#define UI_OSD_BASE_TV_Y_U              30 // Up base position for TV
#define UI_OSD_BASE_TV_Y_D              430// Down base position for TV

#define UI_OSD_ATENNA_LOC_X         	UI_OSD_BASE_X
#define UI_OSD_ATENNA_LOC_Y             UI_OSD_BASE_Y
#define UI_OSD_CAM_LOC_X                UI_OSD_BASE_X + 72 
#define UI_OSD_CAM_LOC_Y                UI_OSD_BASE_Y + 2
#define UI_OSD_REC_LOC_X                188
#define UI_OSD_REC_LOC_Y                UI_OSD_BASE_Y 
#define UI_OSD_NET_LINK_LOC_X           244 
#define UI_OSD_NET_LINK_LOC_Y           UI_OSD_BASE_Y 
#define UI_OSD_SD_FULL_LOC_X            316
#define UI_OSD_SD_FULL_LOC_Y            UI_OSD_BASE_Y
#define UI_SD_ICON_LOC_X                316
#define UI_SD_ICON_LOC_Y                UI_OSD_BASE_Y

#if UI_LIGHT_SUPPORT
#define UI_OSD_LIGHT_LOC_X              800
#define UI_OSD_LIGHT_LOC_Y              UI_OSD_BASE_Y
#endif
#if UI_CAMERA_ALARM_SUPPORT
#define UI_OSD_ALARM_LOC_X              852
#define UI_OSD_ALARM_LOC_Y              UI_OSD_BASE_Y
#endif

#define UI_OSD_DOWNLOAD_LOC_X           908
#define UI_OSD_DOWNLOAD_LOC_Y           UI_OSD_BASE_Y
#define UI_OSD_BATTERY_LOC_X            968
#define UI_OSD_BATTERY_LOC_Y            UI_OSD_BASE_Y


/*------------ LEFT ------------*/

#define UI_OSD_MENU_LOC_X           960
#define UI_OSD_MENU_LOC_Y           20
#define UI_OSD_MENU_BAR_X           0
#define UI_OSD_MENU_BAR_Y           199//212


/*------------ QUAD ------------*/

#define UI_OSD_CAM1_QUAD_LOC_X          UI_OSD_BASE_X
#define UI_OSD_CAM1_QUAD_LOC_Y          UI_OSD_BASE_Y+2
#define UI_OSD_REC_QUAD_LOC_X           108 //UI_OSD_CAM_LOC_X 右邊,CAM 4字元,間隔 10
#define UI_OSD_REC_QUAD_LOC_Y           UI_OSD_BASE_Y 
#if UI_LIGHT_SUPPORT
#define UI_OSD_LIGHT_QUAD_LOC_X         172
#define UI_OSD_LIGHT_QUAD_LOC_Y         UI_OSD_BASE_Y
#endif
#if UI_CAMERA_ALARM_SUPPORT
#define UI_OSD_ALARM_QUAD_LOC_X         232
#define UI_OSD_ALARM_QUAD_LOC_Y         UI_OSD_BASE_Y
#endif

/*------------ BOTTOM ------------*/

#define UI_PREVIEW_TIME_X           360//380
#define UI_PREVIEW_TIME_Y           552
#define UI_OSD_LCD_X                28
#define UI_OSD_LCD_Y                552
#define UI_OSD_MOTION_X             108
#define UI_OSD_MOTION_Y             UI_OSD_LCD_Y                
#define UI_OSD_REC_BUTTON_X         188
#define UI_OSD_REC_BUTTON_Y         UI_OSD_LCD_Y
#define UI_OSD_VOL_LOC_X            816
#define UI_OSD_VOL_LOC_Y            UI_OSD_LCD_Y
#define UI_OSD_TALK_LOC_X           904//896
#define UI_OSD_TALK_LOC_Y           UI_OSD_LCD_Y
#define UI_OSD_QUAD_LOC_X           976
#define UI_OSD_QUAD_LOC_Y           UI_OSD_LCD_Y
#define UI_OSD_VOL_NUM_LOC_X        820
#define UI_OSD_VOL_NUM_LOC_Y        UI_OSD_LCD_Y


#define UI_OSD_REMOTE_LOC_X             UI_OSD_NET_LINK_LOC_X
#define UI_OSD_REMOTE_LOC_Y             UI_OSD_NET_LINK_LOC_Y
#define UI_OSD_RES_LOC_X                UI_OSD_CAM_LOC_X + 16*4 + 10 //UI_OSD_CAM_LOC_X 右邊,CAM 4字元,間隔 10
#define UI_OSD_RES_LOC_Y                UI_OSD_BASE_Y +2
#define UI_OSD_OVERWRITE_LOC_X          596
#define UI_OSD_OVERWRITE_LOC_Y          40
#define UI_OSD_RESOLUTION_LOC_X     60
#define UI_OSD_RESOLUTION_LOC_Y     10
#define UI_OSD_PRE_VOL_LOC_X        120
#define UI_OSD_PRE_VOL_LOC_Y        438
#define UI_OSD_PRE_VOL_MUTE_LOC_X   32
#define UI_OSD_PRE_VOL_MUTE_LOC_Y   443
#define UI_OSD_PRE_VOL_NUM_LOC_X    32
#define UI_OSD_PRE_VOL_NUM_LOC_Y    443
#define UI_OSD_PLAYBACY_PLAY_LOC_X  100
#define UI_OSD_PLAYBACY_PLAY_LOC_Y  90


/*********************for TV*********************/
#define UI_OSD_SD_FULL_LOC_TV_X         UI_OSD_BASE_TV_X + 52
#define UI_OSD_SD_FULL_LOC_TV_Y         UI_OSD_BASE_TV_Y_D
#define UI_OSD_REC_QUAD_LOC_TV_X        UI_OSD_CAM1_QUAD_LOC_TV_X + 16*4 + 10
#define UI_OSD_REC_QUAD_LOC_TV_Y        UI_OSD_BASE_TV_Y_U - 5
#define UI_OSD_OVERWRITE_LOC_TV_X       380
#define UI_OSD_OVERWRITE_LOC_TV_Y       40
#define UI_OSD_NET_LINK_LOC_TV_X        UI_OSD_BASE_TV_X
#define UI_OSD_NET_LINK_LOC_TV_Y        UI_OSD_BASE_TV_Y_D
#define UI_OSD_CAM1_QUAD_LOC_TV_X       UI_OSD_BASE_TV_X
#define UI_OSD_CAM1_QUAD_LOC_TV_Y       UI_OSD_BASE_TV_Y_U
#define UI_OSD_REC_LOC_TV_X             UI_OSD_RES_LOC_TV_X + 16*4 + 10
#define UI_OSD_REC_LOC_TV_Y             UI_OSD_BASE_TV_Y_U 
#define UI_OSD_ATENNA_LOC_TV_X      	UI_OSD_BASE_TV_X 
#define UI_OSD_ATENNA_LOC_TV_Y          UI_OSD_BASE_TV_Y_U
#define UI_OSD_CAM_LOC_TV_X             UI_OSD_BASE_TV_X + 52
#define UI_OSD_CAM_LOC_TV_Y             UI_OSD_BASE_TV_Y_U
#define UI_OSD_RES_LOC_TV_X             UI_OSD_CAM_LOC_TV_X + 16*4 + 10
#define UI_OSD_RES_LOC_TV_Y             UI_OSD_BASE_TV_Y_U
#define UI_OSD_BATTERY_LOC_TV_X         300
#define UI_OSD_BATTERY_LOC_TV_Y         70
#define UI_PLAYBACK_LOC_MENU_TV_Y       40  //未使用
#define UI_PLAYBACK_LOC_MENU_FILE_TV_X  64
#define UI_PLAYBACK_LOC_MENU_DIR_TV_X   280
#define UI_PLAYBACK_LOC_MENU_DATE_TV_X  240
#define UI_PLAYBACK_LOC_MENU_TIME_TV_X  430
#define UI_PLAYBACK_LOC_MENU_FILE_TV_Y  80 //未使用
#define UI_PLAYBACK_LOC_PAGE_TV_X       340
#define UI_PLAYBACK_LOC_PAGE_TV_X_L     16
#define UI_PLAYBACK_LOC_PAGE_TV_Y       430 
#define UI_PLAYBACK_LOC_VIDEO_TIME_X    398//526
#define UI_PLAYBACK_LOC_VIDEO_TIME_Y    12
#define UI_PREVIEW_TIME_TV_X            168
#define UI_PREVIEW_TIME_TV_Y            430
#define UI_OSD_ASCII_STRING_LOC_TV_X    UI_OSD_BASE_TV_X+36 //
#define UI_OSD_ASCII_STRING_LOC_TV_Y    UI_OSD_BASE_TV_Y_U

/************************************************/
#define UI_CONFIRM_FORMATE_X        80
#define UI_CONFIRM_FORMATE_Y        202
#define UI_CONFIRM_DELETE_X         280
#define UI_CONFIRM_DELETE_Y         180
#define UI_CONFIRM_YES_X            500
#define UI_CONFIRM_YES_Y            260
#define UI_CONFIRM_FORMATE_TV_X     200
#define UI_CONFIRM_YES_TV_X         308
#define UI_CONFIRM_DELETE_TV_X      200


#define UI_OSD_VERSION_LOC_X        139
#define UI_OSD_VERSION_LOC_TV_X     268
#define UI_OSD_VERSION_LOC_Y        392

/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
s32 uiOsdPlaybackFileNext(void);
s32 uiOsdPlaybackFilePrev(void);

void (*OSDDisplay[])(u8, u32, u32, u32, u32) = {
    iduOSDDisplay1,
    iduTVOSDDisplay,
};

s32 (*uiPlaybackListNext[])(void) = {
    dcfPlaybackDirForward,
#if UI_CALENDAR_SUPPORT        
    uiOsdPlaybackFileNext,
#else
    dcfPlaybackFileNext,
#endif
    dcfPlaybackFileNext,
    dcfPlaybackFileNext,
};

s32 (*uiPlaybackListPrev[])(void) = {
    dcfPlaybackDirBackward,
#if UI_CALENDAR_SUPPORT        
    uiOsdPlaybackFilePrev,
#else
    dcfPlaybackFilePrev,
#endif
    dcfPlaybackFilePrev,
    dcfPlaybackFilePrev,
};

#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
u8  uiDoorDirIndex[]=
{
    DCF_DOOR_DIR_MOVIE,
    DCF_DOOR_DIR_PICTURE,
    DCF_DOOR_DIR_ALBUM
};
#endif

u8* Playback_speed[UI_PLAYBACK_SPEED_LEVEL]={"16X", "8X ", "4X ", "2X ", "1X ", "   ", "2X ", "4X ", "8X ", "16X"};

s32 MotionMaskCursor = 0;

u8 zoomflag = 0;
u8 zoomcoff = 1;
u8 volumeflag;
u8 uiIsRFBroken = 2;
u8 uiIsRFParing[RFID_MAX_WORD]={0};   // 1:Pairing 0:Not Pairing
#if ((HW_BOARD_OPTION==MR9670_COMMAX) ||(HW_BOARD_OPTION == MR9670_COMMAX_WI2))
extern u8 PhotoFramenum;
#endif
#if APP_KEEP_ALIVE
u8 APPConnectIcon = 0; /* Sean 160729 Earth Icon */
#endif
u32 totalPlaybackFileNumADay;
u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
bool gfileLog = 0;
u8 showPIRMsg = 0;
/*
 *********************************************************************************************************
 * English
 *********************************************************************************************************
 */

u8 InsertSd_En[] = {41, 78, 83, 69, 82, 84, 0, 51, 36, 0, 35, 65, 82, 68};   /*Insert SD Card*/
u8 SdInit_En[] = {51, 36, 0, 35, 65, 82, 68, 0, 41, 78, 73, 84, 14, 14, 14};    /*SD Card Init...*/
u8 NoFile_En[] = {46, 79, 0, 38, 73, 76, 69};                             /*No File*/
u8 Waiting_En[] = {55, 65, 73, 84, 73, 78, 71};                                 /*Waiting*/
u8 PlsWait_En[] = {48, 76, 69, 65, 83, 69, 0, 55, 65, 73, 84, 14, 14, 14};      /*Please Wait...*/
u8 REC_En[] = {50, 37, 35};                                                     /*REC*/
u8 Detecting_En[] = {36, 69, 84, 69, 67, 84, 73, 78, 71};                       /*Detecting*/
u8 WriteProt_En[] = {55, 82, 73, 84, 69, 0, 48, 82, 79, 84, 69, 67, 84};        /*Write Protect*/
u8 Formating_En[] = {38, 79, 82, 77, 65, 84, 73, 78, 71, 14, 14, 14};           /*Formating...*/
u8 FormatOk_En[] = {38, 79, 82, 77, 65, 84, 0, 47, 43};                         /*Format OK*/
u8 FormatFail_En[] = {38, 79, 82, 77, 65, 84, 0, 38, 65, 73, 76};               /*Format Fail*/
u8 TimeOut_En[] = {52, 73, 77, 69, 0, 47, 85, 84};                              /*Time Out*/
u8 MemFull_En[] = {45, 69, 77, 79, 82, 89, 0, 38, 85, 76, 76};            /*Memory Full*/
u8 Volume_En[] = {54, 47, 44, 53, 45, 37};                                      /*VOLUME*/
u8 TimeSetErr_En[] = {52, 73, 77, 69, 0, 51, 69, 84, 0, 37, 82, 82, 79, 82}; /*Time Set Error*/
u8 UsbMassStorage_En[] = {53, 51, 34, 0, 45, 65, 83, 83, 0, 51, 84, 79, 82, 65, 71, 69}; /*USB Mass Storage*/
u8 SdError_En[] = {35, 65, 82, 68, 0, 37, 82, 82, 79, 82};                /*Card Error*/
u8 UpdatePass_En[] = {53, 48, 36, 33, 52, 37, 0, 48, 33, 51, 51};         /*UPDATE PASS*/
u8 UpdateFail_En[] = {53, 48, 36, 33, 52, 37, 0, 38, 33, 41, 44};         /*UPDATE FAIL*/
u8 FatErr_En[] = {38, 33, 52, 0, 40, 69, 65, 68, 69, 82, 0, 37, 82, 82, 79, 82};  /*FAT Header Error*/
u8 PlsReFormat_En[] = {48, 76, 69, 65, 83, 69, 0, 50, 69, 13, 70, 79, 82, 77, 65, 84};  /*Please Re-format*/
u8 CardStillFail_En[] = {35, 65, 82, 68, 0, 51, 84, 73, 76, 76, 0, 38, 65, 73, 76};     /*Card Still Fail*/
u8 PlsChangeCard_En[] = {48, 76, 69, 65, 83, 69, 0, 67, 72, 65, 78, 71, 69, 0, 65, 78, 79, 84, 72, 69, 82, 0, 51, 36, 0, 67, 65, 82, 68};   /*Please change another SD card*/
u8 FsErr_En[] = {38, 51, 0, 47, 80, 69, 82, 65, 84, 73, 79, 78, 0, 37, 82, 82, 79, 82}; /*FS Operation Error*/
u8 ChkWriteProt_En[] = {35, 72, 69, 67, 75, 0, 51, 36, 0, 87, 82, 73, 84, 69, 0, 80, 82, 79, 84, 69, 67, 84};   /*Check SD write protect*/
u8 SdHWErr_En[] = {51, 36, 0, 35, 65, 82, 68, 0, 40, 15, 55, 0, 37, 82, 82, 79, 82};    /*SD Card H/W Error*/
u8 FwUpdatePass_En[] = {38, 55, 0, 53, 80, 68, 65, 84, 69, 68, 0, 48, 65, 83, 83};   /*FW Updated Pass*/
u8 FwUpdateFail_En[] = {38, 55, 0, 53, 80, 68, 65, 84, 69, 68, 0, 38, 65, 73, 76};   /*FW Updated Fail*/
u8 ChkUiLib_En[] = {35, 72, 69, 67, 75, 0, 53, 41, 0, 44, 73, 66, 82, 65, 82, 89};   /*Check UI Library*/
u8 UiUpdatePass_En[] = {53, 41, 0, 53, 80, 68, 65, 84, 69, 68, 0, 48, 65, 83, 83};   /*UI Updated Pass*/
u8 UiUpdateFail_En[] = {53, 41, 0, 53, 80, 68, 65, 84, 69, 68, 0, 38, 65, 73, 76};   /*UI Updated Fail*/
u8 NoUiLib_En[] = {46, 79, 0, 53, 41, 0, 44, 73, 66, 82, 65, 82, 89, 12, 0, 37, 83, 67, 65, 80, 69, 0, 53, 80, 68, 65, 84, 73, 78, 71};  /*No UI Library, Escape Updating*/
u8 FileErr_En[] = {38, 73, 76, 69, 0, 37, 82, 82, 79, 82};       /*File Error*/
u8 FileName_En[] = {38, 73, 76, 69, 0, 46, 65, 77, 69};             /*File Name*/
u8 Date_En[] = {36, 65, 84, 69};                                    /*Date*/
u8 StartTime_En[] = {51, 84, 65, 82, 84, 0, 52, 73, 77, 69};        /*Start Time*/
u8 Page_En[] = {48, 65, 71, 69};                                    /*Page*/
u8 FolderName_En[] = {38, 79, 76, 68, 69, 82, 0, 46, 65, 77, 69};     /*Folder Name*/
u8 EndName_En[] = {37, 78, 68, 0, 52, 73, 77, 69};                    /*End Time*/
u8 DateTime_En[] = {36, 65, 84, 69, 0, 52, 73, 77, 69};               /*Date Time*/
u8 TimeZone_En[] = {52, 73, 77, 69, 0, 58, 79, 78, 69};               /*Time Zone*/
u8 Abnormal_En[] = {33, 66, 78, 79, 82, 77, 65, 76, 0, 47, 80, 69, 82, 65, 84, 73, 79, 78};     /*Abnormal Operation*/
u8 NoSignal_En[] = {46, 79, 0, 51, 73, 71, 78, 65, 76};         /*No Signal*/
u8 ChangeResolution_En[] = {35, 40, 33, 46, 39, 37, 0, 50, 37, 51, 47, 44, 53, 52, 41, 47, 46, 14, 14, 14}; /*CHANGE RESOLUTION...*/
u8 RESTORE_DEFAULT_SETTINGS_En[] = {50, 37, 51, 52, 47, 50, 37, 0, 36, 37, 38, 33, 53, 44, 52, 0, 51, 37, 52, 52, 41, 46, 39, 51, 14, 14, 14 }; /*Restore_Default_Settings...*/
u8 PowerOff_En[] = {48, 47, 55, 37, 50, 41, 46, 39, 0, 47, 38, 38,14, 14, 14}; /*POWERING OFF...*/
u8 Card_Failed_En[] = {35, 65, 82, 68, 0, 37, 82, 82, 79, 82, 1, 50, 69, 84, 82, 89, 0, 48, 76, 69, 65, 83, 69, 14}; /*Card Error!Retry please.*/
u8 Card_Full_En[] = {51, 36, 0, 35, 65, 82, 68, 0, 38, 85, 76, 76, 1};/*SD Card Full!*/
u8 System_Reboot[] = {51, 89, 83, 84, 69, 77, 0, 50, 69, 66, 79, 79, 84, 0, 46, 79, 87, 14};/*System Reboot Now.*/

/*====================OSD Menu =======================*/




/*====================OSD Menu END=======================*/

/*
 *********************************************************************************************************
 * UI_MULT_LAN_STR
 *********************************************************************************************************
 */


UI_MULT_LAN_STR InsertSdStr[UI_MULT_LANU_END] =
{
    {InsertSd_En, sizeof(InsertSd_En)},
    {InsertSd_En, sizeof(InsertSd_En)},
    {InsertSd_En, sizeof(InsertSd_En)},
};

UI_MULT_LAN_STR SdInitStr[UI_MULT_LANU_END] =
{
    {SdInit_En, sizeof(SdInit_En)},
    {SdInit_En, sizeof(SdInit_En)},
    {SdInit_En, sizeof(SdInit_En)},
};

UI_MULT_LAN_STR NoFileStr[UI_MULT_LANU_END] =
{
    {NoFile_En, sizeof(NoFile_En)},
    {NoFile_En, sizeof(NoFile_En)},
    {NoFile_En, sizeof(NoFile_En)},
};

UI_MULT_LAN_STR WaitingStr[UI_MULT_LANU_END] =
{
    {Waiting_En, sizeof(Waiting_En)},
    {Waiting_En, sizeof(Waiting_En)},
    {Waiting_En, sizeof(Waiting_En)},
};

UI_MULT_LAN_STR PlsWaitStr[UI_MULT_LANU_END] =
{
    {PlsWait_En, sizeof(PlsWait_En)},
    {PlsWait_En, sizeof(PlsWait_En)},
    {PlsWait_En, sizeof(PlsWait_En)},
};

UI_MULT_LAN_STR RECStr[UI_MULT_LANU_END] =
{
    {REC_En, sizeof(REC_En)},
    {REC_En, sizeof(REC_En)},
    {REC_En, sizeof(REC_En)},
};

UI_MULT_LAN_STR DetectingStr[UI_MULT_LANU_END] =
{
    {Detecting_En, sizeof(Detecting_En)},
    {Detecting_En, sizeof(Detecting_En)},
    {Detecting_En, sizeof(Detecting_En)},
};

UI_MULT_LAN_STR WriteProtStr[UI_MULT_LANU_END] =
{
    {WriteProt_En, sizeof(WriteProt_En)},
    {WriteProt_En, sizeof(WriteProt_En)},
    {WriteProt_En, sizeof(WriteProt_En)},
};

UI_MULT_LAN_STR FormatingStr[UI_MULT_LANU_END] =
{
    {Formating_En, sizeof(Formating_En)},
    {Formating_En, sizeof(Formating_En)},
    {Formating_En, sizeof(Formating_En)},
};

UI_MULT_LAN_STR FormatOkStr[UI_MULT_LANU_END] =
{
    {FormatOk_En, sizeof(FormatOk_En)},
    {FormatOk_En, sizeof(FormatOk_En)},
    {FormatOk_En, sizeof(FormatOk_En)},
};

UI_MULT_LAN_STR FormatFailStr[UI_MULT_LANU_END] =
{
    {FormatFail_En, sizeof(FormatFail_En)},
    {FormatFail_En, sizeof(FormatFail_En)},
    {FormatFail_En, sizeof(FormatFail_En)},
};

UI_MULT_LAN_STR TimeOutStr[UI_MULT_LANU_END] =
{
    {TimeOut_En, sizeof(TimeOut_En)},
    {TimeOut_En, sizeof(TimeOut_En)},
    {TimeOut_En, sizeof(TimeOut_En)},
};

UI_MULT_LAN_STR MemFullStr[UI_MULT_LANU_END] =
{
    {MemFull_En, sizeof(MemFull_En)},
    {MemFull_En, sizeof(MemFull_En)},
    {MemFull_En, sizeof(MemFull_En)},
};

UI_MULT_LAN_STR VolStr[UI_MULT_LANU_END] =
{
    {Volume_En, sizeof(Volume_En)},
    {Volume_En, sizeof(Volume_En)},
    {Volume_En, sizeof(Volume_En)},
};

UI_MULT_LAN_STR TimeSetErrStr[UI_MULT_LANU_END] =
{
    {TimeSetErr_En, sizeof(TimeSetErr_En)},
    {TimeSetErr_En, sizeof(TimeSetErr_En)},
    {TimeSetErr_En, sizeof(TimeSetErr_En)},
};

UI_MULT_LAN_STR UsbMassStorageStr[UI_MULT_LANU_END] =
{
    {UsbMassStorage_En, sizeof(UsbMassStorage_En)},
    {UsbMassStorage_En, sizeof(UsbMassStorage_En)},
    {UsbMassStorage_En, sizeof(UsbMassStorage_En)},
};

UI_MULT_LAN_STR SdErrorStr[UI_MULT_LANU_END] =
{
    {SdError_En, sizeof(SdError_En)},
    {SdError_En, sizeof(SdError_En)},
    {SdError_En, sizeof(SdError_En)},
};

UI_MULT_LAN_STR UpdatePassStr[UI_MULT_LANU_END] =
{
    {UpdatePass_En, sizeof(UpdatePass_En)},
    {UpdatePass_En, sizeof(UpdatePass_En)},
    {UpdatePass_En, sizeof(UpdatePass_En)},
};

UI_MULT_LAN_STR UpdateFailStr[UI_MULT_LANU_END] =
{
    {UpdateFail_En, sizeof(UpdateFail_En)},
    {UpdateFail_En, sizeof(UpdateFail_En)},
    {UpdateFail_En, sizeof(UpdateFail_En)},
};

UI_MULT_LAN_STR FatErrStr[UI_MULT_LANU_END] =
{
    {FatErr_En, sizeof(FatErr_En)},
    {FatErr_En, sizeof(FatErr_En)},
    {FatErr_En, sizeof(FatErr_En)},
};

UI_MULT_LAN_STR PlsReFormatStr[UI_MULT_LANU_END] =
{
    {PlsReFormat_En, sizeof(PlsReFormat_En)},
    {PlsReFormat_En, sizeof(PlsReFormat_En)},
    {PlsReFormat_En, sizeof(PlsReFormat_En)},
};

UI_MULT_LAN_STR CardStillFailStr[UI_MULT_LANU_END] =
{
    {CardStillFail_En, sizeof(CardStillFail_En)},
    {CardStillFail_En, sizeof(CardStillFail_En)},
    {CardStillFail_En, sizeof(CardStillFail_En)},
};

UI_MULT_LAN_STR PlsChangeCardStr[UI_MULT_LANU_END] =
{
    {PlsChangeCard_En, sizeof(PlsChangeCard_En)},
    {PlsChangeCard_En, sizeof(PlsChangeCard_En)},
    {PlsChangeCard_En, sizeof(PlsChangeCard_En)},
};

UI_MULT_LAN_STR FsErrStr[UI_MULT_LANU_END] =
{
    {FsErr_En, sizeof(FsErr_En)},
    {FsErr_En, sizeof(FsErr_En)},
    {FsErr_En, sizeof(FsErr_En)},
};

UI_MULT_LAN_STR ChkWriteProtStr[UI_MULT_LANU_END] =
{
    {ChkWriteProt_En, sizeof(ChkWriteProt_En)},
    {ChkWriteProt_En, sizeof(ChkWriteProt_En)},
    {ChkWriteProt_En, sizeof(ChkWriteProt_En)},
};

UI_MULT_LAN_STR SdHWErrStr[UI_MULT_LANU_END] =
{
    {SdHWErr_En, sizeof(SdHWErr_En)},
    {SdHWErr_En, sizeof(SdHWErr_En)},
    {SdHWErr_En, sizeof(SdHWErr_En)},
};

UI_MULT_LAN_STR FwUpdatePassStr[UI_MULT_LANU_END] =
{
    {FwUpdatePass_En, sizeof(FwUpdatePass_En)},
    {FwUpdatePass_En, sizeof(FwUpdatePass_En)},
    {FwUpdatePass_En, sizeof(FwUpdatePass_En)},
};

UI_MULT_LAN_STR FwUpdateFailStr[UI_MULT_LANU_END] =
{
    {FwUpdateFail_En, sizeof(FwUpdateFail_En)},
    {FwUpdateFail_En, sizeof(FwUpdateFail_En)},
    {FwUpdateFail_En, sizeof(FwUpdateFail_En)},
};

UI_MULT_LAN_STR ChkUiLibStr[UI_MULT_LANU_END] =
{
    {ChkUiLib_En, sizeof(ChkUiLib_En)},
    {ChkUiLib_En, sizeof(ChkUiLib_En)},
    {ChkUiLib_En, sizeof(ChkUiLib_En)},
};

UI_MULT_LAN_STR UiUpdatePassStr[UI_MULT_LANU_END] =
{
    {UiUpdatePass_En, sizeof(UiUpdatePass_En)},
    {UiUpdatePass_En, sizeof(UiUpdatePass_En)},
    {UiUpdatePass_En, sizeof(UiUpdatePass_En)},
};

UI_MULT_LAN_STR UiUpdateFailStr[UI_MULT_LANU_END] =
{
    {UiUpdateFail_En, sizeof(UiUpdateFail_En)},
    {UiUpdateFail_En, sizeof(UiUpdateFail_En)},
    {UiUpdateFail_En, sizeof(UiUpdateFail_En)},
};

UI_MULT_LAN_STR NoUiLibStr[UI_MULT_LANU_END] =
{
    {NoUiLib_En, sizeof(NoUiLib_En)},
    {NoUiLib_En, sizeof(NoUiLib_En)},
    {NoUiLib_En, sizeof(NoUiLib_En)},
};

UI_MULT_LAN_STR FileErrStr[UI_MULT_LANU_END] =
{
    {FileErr_En, sizeof(FileErr_En)},
    {FileErr_En, sizeof(FileErr_En)},
    {FileErr_En, sizeof(FileErr_En)},
};

UI_MULT_LAN_STR FileNameStr[UI_MULT_LANU_END] =
{
    {FileName_En, sizeof(FileName_En)},
    {FileName_En, sizeof(FileName_En)},
    {FileName_En, sizeof(FileName_En)},
};

UI_MULT_LAN_STR DateStr[UI_MULT_LANU_END] =
{
    {Date_En, sizeof(Date_En)},
    {Date_En, sizeof(Date_En)},
    {Date_En, sizeof(Date_En)},
};

UI_MULT_LAN_STR StartTimeStr[UI_MULT_LANU_END] =
{
    {StartTime_En, sizeof(StartTime_En)},
    {StartTime_En, sizeof(StartTime_En)},
    {StartTime_En, sizeof(StartTime_En)},
};

UI_MULT_LAN_STR PageStr[UI_MULT_LANU_END] =
{
    {Page_En, sizeof(Page_En)},
    {Page_En, sizeof(Page_En)},
    {Page_En, sizeof(Page_En)},
};

UI_MULT_LAN_STR FolderNameStr[UI_MULT_LANU_END] =
{
    {FolderName_En, sizeof(FolderName_En)},
    {FolderName_En, sizeof(FolderName_En)},
    {FolderName_En, sizeof(FolderName_En)},
};

UI_MULT_LAN_STR EndNameStr[UI_MULT_LANU_END] =
{
    {EndName_En, sizeof(EndName_En)},
    {EndName_En, sizeof(EndName_En)},
    {EndName_En, sizeof(EndName_En)},
};

UI_MULT_LAN_STR DateTimeStr[UI_MULT_LANU_END] =
{
    {DateTime_En, sizeof(DateTime_En)},
    {DateTime_En, sizeof(DateTime_En)},
    {DateTime_En, sizeof(DateTime_En)},
};

UI_MULT_LAN_STR TimeZoneStr[UI_MULT_LANU_END] =
{
    {TimeZone_En, sizeof(TimeZone_En)},
    {TimeZone_En, sizeof(TimeZone_En)},
    {TimeZone_En, sizeof(TimeZone_En)},
};

UI_MULT_LAN_STR AbnormalStr[UI_MULT_LANU_END] =
{
    {Abnormal_En, sizeof(Abnormal_En)},
    {Abnormal_En, sizeof(Abnormal_En)},
    {Abnormal_En, sizeof(Abnormal_En)},
};

UI_MULT_LAN_STR NoSignalStr[UI_MULT_LANU_END] =
{
    {NoSignal_En, sizeof(NoSignal_En)},
    {NoSignal_En, sizeof(NoSignal_En)},
    {NoSignal_En, sizeof(NoSignal_En)},
};

UI_MULT_LAN_STR PowerOffStr[UI_MULT_LANU_END] =
{
    {PowerOff_En, sizeof(PowerOff_En)},
    {PowerOff_En, sizeof(PowerOff_En)},
    {PowerOff_En, sizeof(PowerOff_En)},
};

UI_MULT_LAN_STR ChangeResoStr[UI_MULT_LANU_END] =
{
    {ChangeResolution_En, sizeof(ChangeResolution_En)},
    {ChangeResolution_En, sizeof(ChangeResolution_En)},
    {ChangeResolution_En, sizeof(ChangeResolution_En)},
};

UI_MULT_LAN_STR RestoreDefaultSettingsStr[UI_MULT_LANU_END] =
{
    {RESTORE_DEFAULT_SETTINGS_En, sizeof(RESTORE_DEFAULT_SETTINGS_En)},
    {RESTORE_DEFAULT_SETTINGS_En, sizeof(RESTORE_DEFAULT_SETTINGS_En)},
    {RESTORE_DEFAULT_SETTINGS_En, sizeof(RESTORE_DEFAULT_SETTINGS_En)},
};

UI_MULT_LAN_STR CardFailedStr[UI_MULT_LANU_END] =
{
    {Card_Failed_En, sizeof(Card_Failed_En)},
    {Card_Failed_En, sizeof(Card_Failed_En)},
    {Card_Failed_En, sizeof(Card_Failed_En)},
};

UI_MULT_LAN_STR CardFullStr[UI_MULT_LANU_END] =
{
    {Card_Full_En, sizeof(Card_Full_En)},
    {Card_Full_En, sizeof(Card_Full_En)},
    {Card_Full_En, sizeof(Card_Full_En)},
};

UI_MULT_LAN_STR SystemRebootStr[UI_MULT_LANU_END] =
{
    {System_Reboot, sizeof(System_Reboot)},
    {System_Reboot, sizeof(System_Reboot)},
    {System_Reboot, sizeof(System_Reboot)},
};

/*
 *********************************************************************************************************
 * UI_MSG_TAB
 *********************************************************************************************************
 */

UI_MSG_TAB UiMsgStr[] =
{
    {MSG_ASCII_STR,             NULL },
    {MSG_INSERT_SD_CARD,        InsertSdStr},
    {MSG_SD_INIT,               SdInitStr},
    {MSG_NO_FILE,               NoFileStr},
    {MSG_WAITING,               WaitingStr},
    {MSG_PLEASE_WAIT,           PlsWaitStr},
    {MSG_REC,                   RECStr},
    {MSG_DETECTING,             DetectingStr},
    {MSG_WRITE_PROTECT,         WriteProtStr},
    {MSG_FORMATING,             FormatingStr},
    {MSG_FORMAT_OK,             FormatOkStr},
    {MSG_FORMAT_FAIL,           FormatFailStr},
    {MSG_TIME_OUT,              TimeOutStr},
    {MSG_MEMORY_FULL,           MemFullStr},
    {MSG_VOLUME,                VolStr},
    {MSG_TIME_ERROR,            TimeSetErrStr},
    {MSG_USB_MASS_STORAGE,      UsbMassStorageStr},
    {MSG_CARD_ERROR,            SdErrorStr},
    {MSG_UPDATE_PASS,           UpdatePassStr},
    {MSG_UPDATE_FAIL,           UpdateFailStr},
    {MSG_FAT_HEADER_ERROR,      FatErrStr},
    {MSG_RE_FORMATE,            PlsReFormatStr},
    {MSG_CARD_STILL_FAIL,       CardStillFailStr},
    {MSG_CHANGE_SD_CARD,        PlsChangeCardStr},
    {MSG_FS_OPERATION_ERROR,    FsErrStr},
    {MSG_CHECK_WRITE_PROTECT,   ChkWriteProtStr},
    {MSG_SD_HW_ERROR,           SdHWErrStr},
    {MSG_FW_UPDATE_PASS,        FwUpdatePassStr},
    {MSG_FW_UPDATE_FAIL,        FwUpdateFailStr},
    {MSG_CHECK_UI,              ChkUiLibStr},
    {MSG_UI_UPDATE_PASS,        UiUpdatePassStr},
    {MSG_UI_UPDATE_FAIL,        UiUpdateFailStr},
    {MSG_NO_UI_LIBRARY,         NoUiLibStr},
    {MSG_FILE_ERROR,            FileErrStr},
    {MSG_FILE_NAME,             FileNameStr},
    {MSG_DATE,                  DateStr},
    {MSG_START_TIME,            StartTimeStr},
    {MSG_FOLDER_NAME,           FolderNameStr },
    {MSG_END_TIME,              EndNameStr },
    {MSG_DATE_TIME,             DateTimeStr },
    {MSG_TIME_ZONE,             TimeZoneStr },
    {MSG_ABNORMAL_OPERATION,    AbnormalStr },
    {MSG_NO_SIGNAL,             NoSignalStr },
    {MSG_POWER_OFF,             PowerOffStr},
    {MSG_CHANGE_RESOLUTION,     ChangeResoStr},
    {MSG_RESTORE_DEFAULT_SETTINGS, RestoreDefaultSettingsStr},
    {MSG_CARD_FAILED,           CardFailedStr},
    {MSG_CARD_FULL,             CardFullStr},
    {MSG_SYSTEM_REBOOT,         SystemRebootStr},
};

/* ICON */

UI_MULT_ICON WarningIcon[UI_MULT_ICON_END] =
{
    {OSD_WARNING,16,24},
    {OSD_WARNING,16,24},
};
#if 0
UI_MULT_ICON VRecIcon[UI_MULT_ICON_END] =
{
    {OSD_VREC,16,24},
    {OSD_VREC,16,24},
};

UI_MULT_ICON VRecIcon2[UI_MULT_ICON_END] =
{
    {OSD_VREC2,16,24},
    {OSD_VREC2,16,24},
};
#else
UI_MULT_ICON Rec_start_Icon[UI_MULT_ICON_END] =
{
    {OSD_Rec_Start,24,24},
    {OSD_Rec_Start,24,24},
};

UI_MULT_ICON Rec_stop_Icon[UI_MULT_ICON_END] =
{
    {OSD_Rec_Stop,24,24},
    {OSD_Rec_Stop,24,24},
};
#endif
UI_MULT_ICON FFIcon[UI_MULT_ICON_END] =
{
    {OSD_FF,12,16},
    {OSD_FF,12,16},
};

UI_MULT_ICON RewIcon[UI_MULT_ICON_END] =
{
    {OSD_REW,12,16},
    {OSD_REW,12,16},
};


UI_MULT_ICON PlayIcon[UI_MULT_ICON_END] =
{
    {OSD_PLAY_ICON,16,25},
    {OSD_PLAY_ICON,16,25},
};

UI_MULT_ICON PlaybackIcon[UI_MULT_ICON_END] =
{
    {OSD_PLAYBACK_ICON,20,25},
    {OSD_PLAYBACK_ICON,20,25},
};


UI_MULT_ICON StopIcon[UI_MULT_ICON_END] =
{
    {OSD_STOP,12,16},
    {OSD_STOP,12,16},
};

UI_MULT_ICON PauseIcon[UI_MULT_ICON_END] =
{
    {OSD_PAUSE_ICON,20,25},
    {OSD_PAUSE_ICON,20,25},
};

UI_MULT_ICON BatteryIconCharge[UI_MULT_ICON_END] =
{
    {OSD_CHARGE_ICON,44,20},
    {OSD_CHARGE_ICON,44,20},
};

UI_MULT_ICON BatteryIconLV3[UI_MULT_ICON_END] =
{
    {OSD_BAT_ICON[3],44,20},
    {OSD_BAT_ICON[3],44,20},
};

UI_MULT_ICON BatteryIconLV2[UI_MULT_ICON_END] =
{
    {OSD_BAT_ICON[2],44,20},
    {OSD_BAT_ICON[2],44,20},
};

UI_MULT_ICON BatteryIconLV1[UI_MULT_ICON_END] =
{
    {OSD_BAT_ICON[1],44,20},
    {OSD_BAT_ICON[1],44,20},
};

UI_MULT_ICON BatteryIconLV0[UI_MULT_ICON_END] =
{
    {OSD_BAT_ICON[0],44,20},
    {OSD_BAT_ICON[0],44,20},
};

UI_MULT_ICON AtennaSginal0Icon[UI_MULT_ICON_END] =
{
    {OSD_ANTENNA0_B,44,24},
    {OSD_ANTENNA0_B,44,24},
};

UI_MULT_ICON AtennaSginal1Icon[UI_MULT_ICON_END] =
{
    {OSD_ANTENNA1_B,44,24},
    {OSD_ANTENNA1_B,44,24},
};

UI_MULT_ICON AtennaSginal2Icon[UI_MULT_ICON_END] =
{
    {OSD_ANTENNA2_B,44,24},
    {OSD_ANTENNA2_B,44,24},
};

UI_MULT_ICON AtennaSginal3Icon[UI_MULT_ICON_END] =
{
    {OSD_ANTENNA3_B,44,24},
    {OSD_ANTENNA3_B,44,24},
};

UI_MULT_ICON AtennaSginal4Icon[UI_MULT_ICON_END] =
{
    {OSD_ANTENNA4_B,44,24},
    {OSD_ANTENNA4_B,44,24},
};

UI_MULT_ICON SDFullIcon[UI_MULT_ICON_END] =
{
    {OSD_SD_FULL,32,20},
    {OSD_SD_FULL,32,20},
};

UI_MULT_ICON SDIcon[UI_MULT_ICON_END] =
{
    {OSD_SD_ICON,32,28},
    {OSD_SD_ICON,32,28},
};

UI_MULT_ICON LinkUpIcon[UI_MULT_ICON_END] =
{
    {Network_LinkUp,40,24},
    {Network_LinkUp,40,24},
};

UI_MULT_ICON LinkDownIcon[UI_MULT_ICON_END] =
{
    {Network_LinkDown,40,24},
    {Network_LinkDown,40,24},
};

UI_MULT_ICON DownloadIcon[UI_MULT_ICON_END] =
{
    {OSD_Download,32,32},
    {OSD_Download,32,32},
};

UI_MULT_ICON TalkbackIcon[UI_MULT_ICON_END] =
{
    {OSD_TALKBACK,24,32},
    {OSD_TALKBACK,24,32},
};

UI_MULT_ICON TalkIngIcon[UI_MULT_ICON_END] =
{
    {OSD_TALKING,32,64},
    {OSD_TALKING,32,64},
};

UI_MULT_ICON Menu1Icon[UI_MULT_ICON_END] =
{
    {OSD_MENU1,44,24},
    {OSD_MENU1,44,24},
};

UI_MULT_ICON Menu_Top_Icon[UI_MULT_ICON_END] =
{
    {OSD_MENU_Top,28,12},
    {OSD_MENU_Top,28,12},
};

UI_MULT_ICON Menu_Mid_Icon[UI_MULT_ICON_END] =
{
    {OSD_MENU_Mid,28,24},
    {OSD_MENU_Mid,28,24},
};

UI_MULT_ICON Menu_Bottom_Icon[UI_MULT_ICON_END] =
{
    {OSD_MENU_Bottom,28,12},
    {OSD_MENU_Bottom,28,12},
};

UI_MULT_ICON QuadIcon[UI_MULT_ICON_END] =
{
    {OSD_QUAD,32,32},
    {OSD_QUAD,32,32},
};

UI_MULT_ICON Cam1_Icon[UI_MULT_ICON_END] =
{
    {OSD_CAM1,40,20},
    {OSD_CAM1,40,20},
};

UI_MULT_ICON Cam2_Icon[UI_MULT_ICON_END] =
{
    {OSD_CAM2,40,20},
    {OSD_CAM2,40,20},
};

UI_MULT_ICON Cam3_Icon[UI_MULT_ICON_END] =
{
    {OSD_CAM3,40,20},
    {OSD_CAM3,04,20},
};

UI_MULT_ICON Cam4_Icon[UI_MULT_ICON_END] =
{
    {OSD_CAM4,40,20},
    {OSD_CAM4,40,20},
};

UI_MULT_ICON Motion_Open_Icon[UI_MULT_ICON_END] =
{
    {OSD_MOTION_OPEN,36,32},
    {OSD_MOTION_OPEN,36,32},
};

UI_MULT_ICON Motion_Close_Icon[UI_MULT_ICON_END] =
{
    {OSD_MOTION_CLOSE,36,32},
    {OSD_MOTION_CLOSE,36,32},
};

UI_MULT_ICON REC_Button_Icon[UI_MULT_ICON_END] =
{
    {OSD_Rec_Button,40,32},
    {OSD_Rec_Button,40,32},
};

UI_MULT_ICON OsdMuteIcon[UI_MULT_ICON_END] =
{
    {OSD_Mute,40,32},
    {OSD_Mute,40,32},
};

UI_MULT_ICON OsdVolTopIcon[UI_MULT_ICON_END] =
{
    {OSD_Vol_Top,24,24},
    {OSD_Vol_Top,24,24},
};

UI_MULT_ICON OsdVolBottomIcon[UI_MULT_ICON_END] =
{
    {OSD_Vol_Bottom,24,24},
    {OSD_Vol_Bottom,24,24},
};

UI_MULT_ICON OsdVol0Icon[UI_MULT_ICON_END] =
{
    {OSD_Vol0,24,92},
    {OSD_Vol0,24,92},
};

UI_MULT_ICON OsdVol1Icon[UI_MULT_ICON_END] =
{
    {OSD_Vol1,24,92},
    {OSD_Vol1,24,92},
};

UI_MULT_ICON OsdVol2Icon[UI_MULT_ICON_END] =
{
    {OSD_Vol2,24,92},
    {OSD_Vol2,24,92},
};
UI_MULT_ICON OsdVol3Icon[UI_MULT_ICON_END] =
{
    {OSD_Vol3,24,92},
    {OSD_Vol3,24,92},
};

UI_MULT_ICON OsdVol4Icon[UI_MULT_ICON_END] =
{
    {OSD_Vol4,24,92},
    {OSD_Vol4,24,92},
};

UI_MULT_ICON OsdVol5Icon[UI_MULT_ICON_END] =
{
    {OSD_Vol5,24,92},
    {OSD_Vol5,24,92},
};

UI_MULT_ICON OsdLCDBLIcon[UI_MULT_ICON_END] =
{
    {OSD_LCD_BL,40,32},
    {OSD_LCD_BL,40,32},
};

UI_MULT_ICON OsdArrowUpIcon[UI_MULT_ICON_END] =
{
    {OSD_Arrow_Up,36,36},
    {OSD_Arrow_Up,36,36},
};

UI_MULT_ICON OsdArrowDownIcon[UI_MULT_ICON_END] =
{
    {OSD_Arrow_Down,36,36},
    {OSD_Arrow_Down,36,36},
};

UI_MULT_ICON ReturnIcon[UI_MULT_ICON_END] =
{
    {OSD_RETURN,36,36},
    {OSD_RETURN,36,36},
};

UI_MULT_ICON DelIcon[UI_MULT_ICON_END] =
{
    {OSD_DEL,36,24},
    {OSD_DEL,36,24},
};

UI_MULT_ICON UpIcon[UI_MULT_ICON_END] =
{
    {OSD_UP,36,36},
    {OSD_UP,36,36},
};

UI_MULT_ICON DownIcon[UI_MULT_ICON_END] =
{
    {OSD_DOWN,36,36},
    {OSD_DOWN,36,36},
};

UI_MULT_ICON OsdPlayIcon[UI_MULT_ICON_END] =
{
    {OSD_PLAY,36,36},
    {OSD_PLAY,36,36},
};

UI_MULT_ICON OsdStopIcon[UI_MULT_ICON_END] =
{
    {OSD_STOP_ICON,36,36},
    {OSD_STOP_ICON,36,36},
};

UI_MULT_ICON OsdFFIcon[UI_MULT_ICON_END] =
{
    {OSD_FF_ICON,36,36},
    {OSD_FF_ICON,36,36},
};

UI_MULT_ICON OsdRewIcon[UI_MULT_ICON_END] =
{
    {OSD_REWIND_ICON,36,36},
    {OSD_REWIND_ICON,36,36},
};

UI_MULT_ICON OsdPauseIcon[UI_MULT_ICON_END] =
{
    {OSD_PAUSE,36,36},
    {OSD_PAUSE,36,36},
};

UI_MULT_ICON OsdVolIcon[UI_MULT_ICON_END] =
{
    {OSD_VOL,40,32},
    {OSD_VOL,40,32},
};

UI_MULT_ICON NewFWIcon[UI_MULT_ICON_END] =
{
    {New_Firmware,36,30},
    {New_Firmware,36,30},
};

UI_MULT_ICON OsdSpeed1Icon[UI_MULT_ICON_END] =
{
    {OSD_SPPED1,36,36},
    {OSD_SPPED1,36,36},
};

UI_MULT_ICON OsdSpeed2Icon[UI_MULT_ICON_END] =
{
    {OSD_SPPED2,36,36},
    {OSD_SPPED2,36,36},
};

UI_MULT_ICON OsdSpeed4Icon[UI_MULT_ICON_END] =
{
    {OSD_SPPED4,36,36},
    {OSD_SPPED4,36,36},
};

UI_MULT_ICON OsdSpeed8Icon[UI_MULT_ICON_END] =
{
    {OSD_SPPED8,36,36},
    {OSD_SPPED8,36,36},
};

UI_MULT_ICON OsdSpeed16Icon[UI_MULT_ICON_END] =
{
    {OSD_SPPED16,36,36},
    {OSD_SPPED16,36,36},
};

#if UI_LIGHT_SUPPORT
UI_MULT_ICON Lighting_Light_Icon[UI_MULT_ICON_END] =
{
    {OSD_LIGHTING_SETUP_LIGHT, 36, 40},
    {OSD_LIGHTING_SETUP_LIGHT, 36, 40},
};

UI_MULT_ICON Lighting_Light_Manual_Icon[UI_MULT_ICON_END] =
{
    {OSD_LIGHTING_SETUP_LIGHT_MANUAL, 24, 28},
    {OSD_LIGHTING_SETUP_LIGHT_MANUAL, 24, 28},
};
#endif

#if UI_CAMERA_ALARM_SUPPORT
UI_MULT_ICON Motion_Alarm_Manual_Icon[UI_MULT_ICON_END] =
{
    {OSD_MOTION_ALARM_MANUAL, 44, 24},
    {OSD_MOTION_ALARM_MANUAL, 44, 24},
};
#endif 

#if UI_BAT_SUPPORT
UI_MULT_ICON Batcam_Battery_LV0[UI_MULT_ICON_END] =
{
    {OSD_Batcam_Battery_LV0, 44, 44},
    {OSD_Batcam_Battery_LV0, 44, 44},
};

UI_MULT_ICON Batcam_Battery_LV1[UI_MULT_ICON_END] =
{
    {OSD_Batcam_Battery_LV1, 44, 44},
    {OSD_Batcam_Battery_LV1, 44, 44},
};

UI_MULT_ICON Batcam_Battery_LV2[UI_MULT_ICON_END] =
{
    {OSD_Batcam_Battery_LV2, 44, 44},
    {OSD_Batcam_Battery_LV2, 44, 44},
};

UI_MULT_ICON Batcam_Battery_LV3[UI_MULT_ICON_END] =
{
    {OSD_Batcam_Battery_LV3, 44, 44},
    {OSD_Batcam_Battery_LV3, 44, 44},
};

UI_MULT_ICON Batcam_Battery_Charge[UI_MULT_ICON_END] =
{
    {OSD_Batcam_Battery_Charge, 44, 44},
    {OSD_Batcam_Battery_Charge, 44, 44},
};
#endif

#if USB_HOST_MASS_SUPPORT
UI_MULT_ICON HDDIcon[UI_MULT_ICON_END] =
{
    {OSD_HDD_ICON,32,28},
    {OSD_HDD_ICON,32,28},
};
#endif

UI_MULT_ICON RemoteIcon[UI_MULT_ICON_END] =
{
    {OSD_REMOTE_ICON,24,32},
    {OSD_REMOTE_ICON,24,32},
};

UI_OSDICON_TAB OsdIcon[] =
{
    {OSD_ICON_WARNING_1,    WarningIcon},
    {OSD_ICON_VREC,       Rec_start_Icon},
    {OSD_ICON_VREC2,      Rec_stop_Icon},
    {OSD_ICON_FF,         FFIcon},
    {OSD_ICON_REW,        RewIcon},
    {OSD_ICON_PLAY,       OsdPlayIcon},
    {OSD_ICON_PLAYBACK,   PlaybackIcon},
    {OSD_ICON_SD,         SDIcon},
    {OSD_ICON_STOP,       StopIcon},
    {OSD_ICON_MENU_1M,          Menu1Icon},
    {OSD_ICON_ATENNA_Signal_0,  AtennaSginal0Icon},
    {OSD_ICON_ATENNA_Signal_1,  AtennaSginal1Icon},
    {OSD_ICON_ATENNA_Signal_2,  AtennaSginal2Icon},
    {OSD_ICON_ATENNA_Signal_3,  AtennaSginal3Icon},
    {OSD_ICON_ATENNA_Signal_4,  AtennaSginal4Icon},
    {OSD_ICON_SD_FULL,          SDFullIcon},
    {OSD_ICON_TALKBACK,         TalkbackIcon},
    {OSD_ICON_TALK_ING1,        TalkIngIcon},
    {OSD_ICON_MUTE,          OsdMuteIcon},
    {OSD_ICON_CAM1,           Cam1_Icon},
    {OSD_ICON_CAM2,           Cam2_Icon},
    {OSD_ICON_CAM3,           Cam3_Icon},
    {OSD_ICON_CAM4,           Cam4_Icon},
    {OSD_ICON_PAUSE,            PauseIcon},
    {OSD_ICON_VOL_TOP,       OsdVolTopIcon},    
    {OSD_ICON_VOL0,          OsdVol0Icon},
    {OSD_ICON_VOL1,          OsdVol1Icon},
    {OSD_ICON_VOL2,          OsdVol2Icon},
    {OSD_ICON_VOL3,          OsdVol3Icon},
    {OSD_ICON_VOL4,          OsdVol4Icon},
    {OSD_ICON_VOL5,          OsdVol5Icon},
    {OSD_ICON_VOL_BOTTOM,    OsdVolBottomIcon},
    {OSD_ICON_QUAD,             QuadIcon},
    {OSD_ICON_VOL,              OsdVolIcon},
    {OSD_ICON_RETURN,           ReturnIcon},
    {OSD_ICON_PAUSE_L,          OsdPauseIcon},
    {OSD_ICON_BATTERY_LV0,      BatteryIconLV0},
    {OSD_ICON_BATTERY_LV1,      BatteryIconLV1},
    {OSD_ICON_BATTERY_LV2,      BatteryIconLV2},
    {OSD_ICON_BATTERY_LV3,      BatteryIconLV3},
    {OSD_ICON_BATTERY_CHARGE,   BatteryIconCharge},
    {OSD_ICON_NET_LINK_UP,      LinkUpIcon},
    {OSD_ICON_NET_LINK_DOWN,    LinkDownIcon},
    {OSD_ICON_DOWNLOAD,         DownloadIcon},
    {OSD_ICON_PLAY_ACT_STOP_1,  OsdStopIcon},
    {OSD_ICON_PLAY_ACT_REW_1,   OsdRewIcon},
    {OSD_ICON_PLAY_ACT_FF_1,    OsdFFIcon},
    {OSD_ICON_PLAY_BAR_UP,      UpIcon},
    {OSD_ICON_PLAY_BAR_DOWN,    DownIcon},
    {OSD_ICON_DELETE,           DelIcon},
#if UI_LIGHT_SUPPORT
    {OSD_ICON_LIGHTING_SETUP_LIGHT,  			Lighting_Light_Icon},
    {OSD_ICON_LIGHTING_SETUP_LIGHT_MANUAL,  	Lighting_Light_Manual_Icon},
#endif
    {OSD_ICON_MENU_TOP,            Menu_Top_Icon},
    {OSD_ICON_MENU_MID,            Menu_Mid_Icon},
    {OSD_ICON_MENU_BOTTOM,         Menu_Bottom_Icon},
    {OSD_ICON_MOTION_OPEN,         Motion_Open_Icon},
    {OSD_ICON_MOTION_CLOSE,        Motion_Close_Icon},
    {OSD_ICON_REC_BUTTON,          REC_Button_Icon},
    {OSD_ICON_ARROW_UP,            OsdArrowUpIcon},
    {OSD_ICON_ARROW_DOWN,          OsdArrowDownIcon},
#if UI_CAMERA_ALARM_SUPPORT
    {OSD_ICON_MOTION_ALARM_MANUAL,  Motion_Alarm_Manual_Icon},
#endif 
    {OSD_ICON_LCD_BL,           OsdLCDBLIcon},
    {OSD_ICON_LOAD,             PlaybackIcon},
#if UI_BAT_SUPPORT
    {OSD_ICON_BATCAM_BATTERY_LV0,      Batcam_Battery_LV0},
    {OSD_ICON_BATCAM_BATTERY_LV1,      Batcam_Battery_LV1},
    {OSD_ICON_BATCAM_BATTERY_LV2,      Batcam_Battery_LV2},
    {OSD_ICON_BATCAM_BATTERY_LV3,      Batcam_Battery_LV3},
    {OSD_ICON_BATCAM_BATTERY_CHARGE,   Batcam_Battery_Charge},
#endif
#if USB_HOST_MASS_SUPPORT
    {OSD_ICON_HDD,          HDDIcon},
#endif
    {OSD_ICON_REMOTE,       RemoteIcon},

};

OSD_ICONIDX uiOsdIconIdx[OSD_ICON_MAX_NUM];
MSG_SRTIDX uiStrIconIdx[MSG_MAX_NUM];
enum
{
    MAX_BITRATE_1TX=0,//不同的配對組合 最大的bitrate也不同
    MAX_BITRATE_2TX,
    MAX_BITRATE_3TX,
    MAX_BITRATE_4TX
};
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */

extern  u32 *CiuOverlayImg1_Top;
extern  u32 *CiuOverlayImg1_Bot;

extern  u32 *CiuOverlayImg2_Top;
extern  u32 *CiuOverlayImg2_Bot;

extern  u32 *CiuOverlayImg3_Top;
extern  u32 *CiuOverlayImg3_Bot;

extern  u32 *CiuOverlayImg4_Top;
extern  u32 *CiuOverlayImg4_Bot;
#if (NIC_SUPPORT == 1)
extern  u8  my_ipaddr[];
#endif
#if (REMOTE_FILE_PLAYBACK && NIC_SUPPORT)
extern u8 Fileplaying;
#endif
extern DEF_RFIU_UNIT_CNTL   gRfiuUnitCntl[];
extern u8  uiQuadDisplay;  /*1: quad mode, 0: single mode, 2: dual mode*/
extern u8  video_playback_speed;//for asf player level control
extern u8 uiDrawSDFail;
extern u8  _uiDoor;//Lucian status
extern u32 H264_IFlag_Index[MAX_RFIU_UNIT];
extern u8 gUishowFailTime;
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
void uiOsdDrawCameraBatteryLevel(u8 camID,u8 act);
void osdDrawCamLiveView(u8 camID);

/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */
void uiOsdDrawInit(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    
    sysTVOutOnFlag = 0;
    uiMenuOSDReset();
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);

}

void uiOSDCreateWindow(u8 *obj[], u8* Caption , u16 width, u16 height, u16 loc_x, u16 loc_y)
{
    uiOSDASCIIStringByColor(Caption , loc_x+4 , loc_y+4 , 0 , 0xc0 , 0x00);
}

u8 uiOsdGetIconInfo(OSD_ICONIDX icon_inx , UI_MULT_ICON **icon_info)
{
    u32  IconNum, iconIdx;

    if (icon_inx >= OSD_ICON_MAX_NUM)
    {
        DEBUG_UI("ui Get Library Fail by icon index %d\r\n",icon_inx);
        return 0;
    }
    IconNum = sizeof(OsdIcon)/sizeof(UI_OSDICON_TAB);
    iconIdx = uiOsdIconIdx[icon_inx];

    if (iconIdx > IconNum)
    {
        DEBUG_UI("This Icon Index %d not use in this project\r\n",icon_inx);
        return 0;
    }

    *icon_info = &OsdIcon[iconIdx].PicIcon[UI_MULT_ICON_B];
    return 1;
}

void osdDrawMessage(MSG_SRTIDX strIdx, u16 x_pos , u16 y_pos, u8 buf_idx, u8 obj_color, u8 bg_color)
{
    if(x_pos == CENTERED)
    {
        if(y_pos == CENTERED)
            uiOSDMultiLanguageStrCenter(strIdx, buf_idx, obj_color, bg_color);
        else
            uiOSDMultiLanguageStrByY(strIdx, y_pos, buf_idx, obj_color, bg_color);
    }
    else if (y_pos == CENTERED)
        uiOSDMultiLanguageStrByX(strIdx, x_pos, buf_idx, obj_color, bg_color);
    else
        uiOSDMultiLanguageStrByXY(strIdx, x_pos, y_pos, buf_idx, obj_color, bg_color);
}

void uiOsdDrawSDCardFULL(u8 act)
{
    osdDrawMemFull(act);
}

void osdDrawRunFormat(void)
{
    void* msg;
    u32 count = 0, percent = 0;
    u8 err;
    FS_DISKFREE_T *diskInfo;
    u8  ucPercLevel;        /* Percentage level, the total levels which should be divided. */
    u32 unSectorNum;
    u32   unPercTemp;
    u8 percentstring[6];
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u8 LOC_Y[2] = {0, 30};

    diskInfo = &global_diskInfo;

    IduVideo_ClearPKBuf(0);
    iduSetVideoBuf0Addr(PKBuf0);
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    
    if(sysGetStorageInserted(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
    {
        uiOsdDrawInsertSD(UI_OSD_DRAW);
        OSTimeDly(20);
        uiOsdDrawInsertSD(UI_OSD_CLEAR);
        osdDrawSDIcon(UI_OSD_CLEAR);
        if (MyHandler.MenuMode == GOTO_FORMAT_MODE)
        OSMboxPost(speciall_MboxEvt, "NO");
        return;
    }

    osdDrawMessage(MSG_FORMATING, CENTERED, 80+osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    osdDrawMessage(MSG_PLEASE_WAIT, CENTERED, 110+osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);

    do
    {
        uiGraphDrawLoadingGraph(count);
        msg=OSMboxPend(general_MboxEvt, 10, &err);
        //DEBUG_GREEN("Pending msg %x \n",msg);
        if (count == UI_MENU_LOADING_6)
            count = UI_MENU_LOADING_1;
        else
            count++;
    }while(!msg);    

    OSTimeDly(2);
    IduVideo_ClearPKBuf(0);
    iduSetVideoBuf0Addr(PKBuf0);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    //DEBUG_YELLOW("%s\n",msg);
    if (!strcmp((const char*)msg, "PASS"))
        osdDrawMessage(MSG_FORMAT_OK, CENTERED, 100+osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    else if (!strcmp((const char*)msg, "FAIL"))
        osdDrawMessage(MSG_FORMAT_FAIL, CENTERED, 100+osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    else
        osdDrawMessage(MSG_TIME_OUT, CENTERED, 100+osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    //DEBUG_YELLOW("GOTO_FORMAT_MODE %d\n",MyHandler.MenuMode);
    if (MyHandler.MenuMode== GOTO_FORMAT_MODE)
        OSMboxPost(speciall_MboxEvt, msg);
    OSTimeDlyHMSM(0, 0, 2, 0);
    if (MyHandler.MenuMode== SETUP_MODE)
        uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);        /* Clear the OSD Buf */
}

u32 uiOsdGetFontWidth(u8 language)
{
	u32  nFontWidth = 0;
	switch(language)
    {
        case UI_MULT_LANU_EN:
        default:
#if (USE_BIG_OSD == 1)
            nFontWidth = 16;
#else
            nFontWidth = 8;
#endif
            break;
    }
    return nFontWidth;
}

u32 uiOsdGetFontHeight(u8 language)
{
    u32  nFontHeight = 0;
	switch(language)
    {
        case UI_MULT_LANU_EN:
        default:
#if (USE_BIG_OSD == 1)
            nFontHeight = 20;
#else
            nFontHeight = 16;
#endif
            break;
    }
    return nFontHeight;
}

u8  uiOsdPlaybackFileFind(void)
{
    u8  CamId;

    CamId = dcfPlaybackCurFile->pDirEnt->d_name[7]-'1';
    if (sysPlaybackCamList & (0x01 << CamId))
        return 1;
    else
        return 0;
}

s32 uiOsdPlaybackFileNext(void)
{
    u32 cnt = 0;
    u8  retVal = 0;
    DCF_LIST_DIRENT* PlaytmpDir;
    DCF_LIST_FILEENT* playTmpFile;

    if (dcfPlaybackCurFile == NULL)
        return 0;
    playTmpFile = dcfPlaybackCurFile;
    do
    {
        if (dcfPlaybackCurFile  == dcfGetPlaybackFileListTail())
            break;
        dcfPlaybackFileNext();
        retVal = uiOsdPlaybackFileFind();
        if (gfileLog)
            printf("FileNext %s %d\n",dcfPlaybackCurFile->pDirEnt->d_name, retVal);
    }while (retVal == 0);
    
    if (retVal == 1)
    {
        return 1;
    }
    else
    {
        PlaytmpDir = dcfPlaybackCurDir;
        dcfPlaybackCurDir = dcfPlaybackCurDir->playbackNext;
        while (dcfPlaybackCurDir != PlaytmpDir)
        {
            if(dcfScanFileOnPlaybackDir() == 0)
                break;
            dcfPlaybackCurFile = dcfGetPlaybackFileListHead();
            while(retVal == 0)
            {
                retVal = uiOsdPlaybackFileFind();
                DEBUG_UI("next Dir %s file %s %d\n",dcfPlaybackCurDir->pDirEnt->d_name, dcfPlaybackCurFile->pDirEnt->d_name[7], retVal);
                if (retVal == 1)
                {
                    return 1;
                }
                if (dcfPlaybackCurFile  == dcfGetPlaybackFileListTail())
                    break;
                dcfPlaybackFileNext();
            }
            dcfPlaybackCurDir = dcfPlaybackCurDir->playbackNext;
        }
        if (retVal == 0)
        {
            if(dcfScanFileOnPlaybackDir() == 0)
                return 0;
            dcfPlaybackCurFile = dcfGetPlaybackFileListHead();
            while(dcfPlaybackCurFile != playTmpFile)
            {
                retVal = uiOsdPlaybackFileFind();
                DEBUG_UI("next orDir %s file %s %d\n",dcfPlaybackCurDir->pDirEnt->d_name, dcfPlaybackCurFile->pDirEnt->d_name[7], retVal);
                if (retVal == 1)
                    return 1;
                dcfPlaybackFileNext();
            }
        }
    }
    return 0;
}

s32 uiOsdPlaybackFilePrev(void)
{
    u32 cnt = 0;
    u8  retVal = 0;
    DCF_LIST_DIRENT* PlaytmpDir;
    DCF_LIST_FILEENT* playTmpFile;

    if (dcfPlaybackCurFile == NULL)
        return 0;

    playTmpFile = dcfPlaybackCurFile;
    do
    {
        if (dcfPlaybackCurFile  == dcfGetPlaybackFileListHead())
        {
            break;
        }
        dcfPlaybackFilePrev();
        retVal = uiOsdPlaybackFileFind();
        if (gfileLog)
            printf("FilePrev %s %d\n",dcfPlaybackCurFile->pDirEnt->d_name, retVal);
    }
    while (retVal == 0);
    if (retVal == 1)
    {
        return 1;
    }
    else
    {
        PlaytmpDir = dcfPlaybackCurDir;
        dcfPlaybackCurDir = dcfPlaybackCurDir->playbackPrev;
        while (dcfPlaybackCurDir != PlaytmpDir)
        {
            if(dcfScanFileOnPlaybackDir() == 0)
                break;
            while(retVal == 0)
            {
                retVal = uiOsdPlaybackFileFind();
                DEBUG_UI("Prev Dir %s file %s %d\n",dcfPlaybackCurDir->pDirEnt->d_name, dcfPlaybackCurFile->pDirEnt->d_name[7], retVal);
                if (retVal == 1)
                {
                    return 1;
                }
                if (dcfPlaybackCurFile  == dcfGetPlaybackFileListHead())
                    break;
                dcfPlaybackFilePrev();
            }
            dcfPlaybackCurDir = dcfPlaybackCurDir->playbackPrev;
        }
        if (retVal == 0)
        {
            if(dcfScanFileOnPlaybackDir() == 0)
                return 0;
            while(dcfPlaybackCurFile != playTmpFile)
            {
                retVal = uiOsdPlaybackFileFind();
                DEBUG_UI("Prev orDir %s file %s %d\n",dcfPlaybackCurDir->pDirEnt->d_name, dcfPlaybackCurFile->pDirEnt->d_name[7], retVal);
                if (retVal == 1)
                    return 1;
                dcfPlaybackFilePrev();
            }
        }
    }
    return 0;
}

void uiOsdDrawPlaybackMenuTitle(u8 type)
{
#if (USE_BIG_OSD == 1)
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_FILE_X [2] = {UI_PLAYBACK_LOC_MENU_FILE_X,UI_PLAYBACK_LOC_MENU_FILE_TV_X };
    u16 LOC_DATE_X [2] = {UI_PLAYBACK_LOC_MENU_DATE_X,UI_PLAYBACK_LOC_MENU_DATE_TV_X };
    u16 LOC_TIME_X [2] = {UI_PLAYBACK_LOC_MENU_TIME_X,UI_PLAYBACK_LOC_MENU_TIME_TV_X };
    u16 LOC_DIR_X  [2] = {UI_PLAYBACK_LOC_MENU_DIR_X,UI_PLAYBACK_LOC_MENU_DIR_TV_X };


    uiOsdDisableAll();
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    switch (type)
    {
        case UI_DSP_PLAY_LIST_FILE:
        case UI_DSP_PLAY_LIST_DOOR_ALB:
        case UI_DSP_PLAY_LIST_DOOR_PIC:
            #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
            #else
                uiOSDMultiLanguageStrByXY(MSG_FILE_NAME, LOC_FILE_X[sysTVOutOnFlag], UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
                uiOSDMultiLanguageStrByXY(MSG_DATE, LOC_DATE_X[sysTVOutOnFlag], UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
                uiOSDMultiLanguageStrByXY(MSG_START_TIME, LOC_TIME_X[sysTVOutOnFlag], UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
            #endif
            break;

        case UI_DSP_PLAY_LIST_DIR:
            uiOSDMultiLanguageStrByXY(MSG_FOLDER_NAME, LOC_DIR_X[sysTVOutOnFlag], UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
            break;

        case UI_DSP_PLAY_LIST_DOOR_SELECT:
            uiOSDASCIIStringByColor("MOVIE", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y, OSD_BLK[sysTVOutOnFlag],  0xC0, 0xC1);
            uiOSDASCIIStringByColor("PICTURE", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y1, OSD_BLK[sysTVOutOnFlag],  0xC0, 0xC1);
            uiOSDASCIIStringByColor("ALBUM", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y2, OSD_BLK[sysTVOutOnFlag],  0xC0, 0xC1);
            break;

    }
#else
    uiOsdDisable(OSD_Blk2);
    (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk0, 0, 0, OSDDispWidth[sysTVOutOnFlag], 40);
    (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk1, 0, 40, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_Blk0);
    uiClearOSDBuf(OSD_Blk1);
    uiOsdEnable(OSD_Blk0);
    uiOsdEnable(OSD_Blk1);
    switch (type)
    {
        case UI_DSP_PLAY_LIST_FILE:
        case UI_DSP_PLAY_LIST_DOOR_ALB:
        case UI_DSP_PLAY_LIST_DOOR_PIC:
            uiOSDMultiLanguageStrByXY(MSG_FILE_NAME, LOC_FILE_X[sysTVOutOnFlag], UI_PLAYBACK_LOC_MENU_Y, OSD_Blk0, 0xC0, 0xC1);
            if(OSDIconEndX >= 320)
            {
                uiOSDMultiLanguageStrByXY(MSG_DATE,  LOC_DATE_X[sysTVOutOnFlag], UI_PLAYBACK_LOC_MENU_Y, OSD_Blk0, 0xC0, 0xC1);
                uiOSDMultiLanguageStrByXY(MSG_START_TIME, LOC_TIME_X[sysTVOutOnFlag], UI_PLAYBACK_LOC_MENU_Y, OSD_Blk0, 0xC0, 0xC1);
            }
            break;

        case UI_DSP_PLAY_LIST_DIR:
            if(OSDIconEndX >= 320)
                uiOSDMultiLanguageStrByXY(MSG_FOLDER_NAME, LOC_DIR_X[sysTVOutOnFlag], UI_PLAYBACK_LOC_MENU_Y, OSD_Blk0, 0xC0, 0xC1);
            else
                uiOSDMultiLanguageStrByXY(MSG_FOLDER_NAME, LOC_TIME_X[sysTVOutOnFlag], UI_PLAYBACK_LOC_MENU_Y, OSD_Blk0, 0xC0, 0xC1);
            break;

        case UI_DSP_PLAY_LIST_DOOR_SELECT:
            uiOSDASCIIStringByColor("MOVIE", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y, OSD_Blk1,  0xC0, 0xC1);
            uiOSDASCIIStringByColor("PICTURE", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y1, OSD_Blk1,  0xC0, 0xC1);
            uiOSDASCIIStringByColor("ALBUM", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y2, OSD_Blk1,  0xC0, 0xC1);
            break;
    }
#endif
}

void uiOsdDrawPlaybackMenuFrame(u8 type, u8 select, u8 clean)
{
#if (USE_BIG_OSD == 1)
    u32  RowCol, RightCol, LeftCol;
    u16  RowLen, ColLen;
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_FILE_X[2] = {UI_PLAYBACK_LOC_MENU_FILE_X,UI_PLAYBACK_LOC_MENU_FILE_TV_X };
    u16 LOC_DIR_X[2] = {UI_PLAYBACK_LOC_MENU_DIR_X,UI_PLAYBACK_LOC_MENU_DIR_TV_X };
    u32  StrH;
    u32  startX, startY = UI_PLAYBACK_LOC_MENU_FILE_Y-4, LineH =16;

    StrH = uiOsdGetFontHeight(CurrLanguage);
    LineH += StrH;
    if(clean == 1)
    {
        RowCol  = 0;
        RightCol= 0;
        LeftCol = 0;
    }
    else
    {
        RowCol  = 0xC2C2C2C2;
        RightCol= 0xC2C2C2C2;
        LeftCol = 0xC2C2C2C2;
        #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
        RowCol  = 0xC2C2C2C2;
        RightCol= 0xC2C2C2C2;
        LeftCol = 0xC2C2C2C2;
        #endif
    }

    if (type == UI_DSP_PLAY_LIST_DIR)
    {
        RowLen = UI_PLAYBACK_MENU_DIR_LEN;
        startX = LOC_DIR_X[sysTVOutOnFlag];
        ColLen = UI_PLAYBACK_MENU_WEIGHT;
    }
    else
    {
        if (OSDIconEndX >= 320)
        {
        #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
            RowLen = 400;
        #else
            RowLen = UI_PLAYBACK_MENU_LEN;
        #endif
        }
        else
            RowLen = UI_PLAYBACK_MENU_File_LEN;
        #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
            startY = 0;
            startX = 4+RowLen*(select%2);
            select = select/2;
            LineH = 236;
            StrH = 232;
            ColLen = 240;
        #else
            startX = LOC_FILE_X[sysTVOutOnFlag];
            ColLen = UI_PLAYBACK_MENU_WEIGHT;
        #endif
    }

    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, ColLen, startX-4, startY+select*(LineH+4), OSD_BLK[sysTVOutOnFlag], LeftCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, ColLen, startX+RowLen-8, startY+select*(LineH+4), OSD_BLK[sysTVOutOnFlag], RightCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], RowLen, 4, startX-4, startY+select*(LineH+4), OSD_BLK[sysTVOutOnFlag], RowCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], RowLen, 4, startX-4, startY+StrH+4+select*(LineH+4), OSD_BLK[sysTVOutOnFlag], RowCol);

#else
    u32  RowCol, RightCol, LeftCol;
    u16  RowLen;

    if(clean == 1)
    {
        RowCol  = 0;
        RightCol= 0;
        LeftCol = 0;
    }
    else
    {
        RowCol  = 0xC2C2C2C2;
        RightCol= 0xC2C20000;
        LeftCol = 0x0000C2C2;
    }

    if (type == UI_DSP_PLAY_LIST_DIR)
    {
        RowLen = UI_PLAYBACK_MENU_DIR_LEN;
    }
    else
    {
        if (OSDIconEndX >= 320)
            RowLen = UI_PLAYBACK_MENU_LEN;
        else
            RowLen = UI_PLAYBACK_MENU_File_LEN;
    }

    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, UI_PLAYBACK_MENU_WEIGHT, LOC_FILE_X[sysTVOutOnFlag]-4, 8+select*(OSD_STRING_H+4), OSD_Blk1, LeftCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, UI_PLAYBACK_MENU_WEIGHT, LOC_FILE_X[sysTVOutOnFlag]+RowLen-8, 8+select*(OSD_STRING_H+4), OSD_Blk1, RightCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], RowLen, 2, LOC_FILE_X[sysTVOutOnFlag]-4, 8+select*(OSD_STRING_H+4), OSD_Blk1, RowCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], RowLen, 2, LOC_FILE_X[sysTVOutOnFlag]-4, 28+select*(OSD_STRING_H+4), OSD_Blk1, RowCol);
#endif
}

void uiOsdDrawPlaybackMenuPage(u32 Total, u32 Current)
{
#if (USE_BIG_OSD == 1)
    u8  pageInfo[13];    /*Page xxx/xxx*/
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
	u16 LOC_PAGE_X [2] =   {UI_PLAYBACK_LOC_PAGE_X,UI_PLAYBACK_LOC_PAGE_TV_X };
	u16 LOC_PAGE_Y [2] =   {UI_PLAYBACK_LOC_PAGE_Y,UI_PLAYBACK_LOC_PAGE_TV_Y };

    sprintf((char*)pageInfo, "Page %04d/%04d",Current, Total);
    uiOSDASCIIStringByColor(pageInfo, LOC_PAGE_X[sysTVOutOnFlag], LOC_PAGE_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
    return;
#else
    u8  pageInfo[13];    /*Page xxx/xxx*/

    sprintf((char*)pageInfo, "Page %04d/%04d",Current, Total);
    if(OSDIconEndX >= 320)
        uiOSDASCIIStringByColor(pageInfo, LOC_PAGE_X[sysTVOutOnFlag] , LOC_PAGE_Y[sysTVOutOnFlag] + osdYShift, OSD_Blk1 , 0xC0, 0xC1);
    else
        uiOSDASCIIStringByColor(pageInfo, LOC_PAGE_X_L[sysTVOutOnFlag] , LOC_PAGE_Y[sysTVOutOnFlag] + osdYShift, OSD_Blk1 , 0xC0, 0xC1);
    return;
#endif
}

void uiOsdDrawPlaybackMenuFolder(u8 DirCnt)
{
#if (USE_BIG_OSD == 1)
    u8 i;
    u8  prt_str[9];
    u32  StrH;
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
	u16 LOC_FILE_X [2] = {UI_PLAYBACK_LOC_MENU_FILE_X,UI_PLAYBACK_LOC_MENU_FILE_TV_X };
	u16 LOC_DIR_X  [2] = {UI_PLAYBACK_LOC_MENU_DIR_X,UI_PLAYBACK_LOC_MENU_DIR_TV_X };
    u32  startY, LineH =16;

    startY = UI_PLAYBACK_LOC_MENU_FILE_Y;

    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], UI_PLAYBACK_MENU_LEN, (UI_PLAYBACK_LOC_PAGE_Y-startY+4), LOC_FILE_X[sysTVOutOnFlag]-4, startY-4, OSD_BLK[sysTVOutOnFlag], 0x00);
    StrH = uiOsdGetFontHeight(CurrLanguage);
    LineH += StrH;
    for (i = 0; i < DirCnt; i++)
    {
        if (dcfPlaybackCurDir== NULL)
            return;
        strncpy((char*)prt_str, dcfPlaybackCurDir->pDirEnt->d_name, 8);
        prt_str[8] = '\0';
        if(OSDIconEndX >= 320)
            uiOSDASCIIStringByColor(prt_str, LOC_DIR_X[sysTVOutOnFlag], startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
        else
            uiOSDASCIIStringByColor(prt_str, LOC_FILE_X[sysTVOutOnFlag], startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
        if (i != DirCnt-1)
            (*uiPlaybackListPrev[PlayListDspType])();
    }
#else
    u8 i;
    u8  prt_str[9];

    uiClearOSDBuf(OSD_Blk1);
    for (i = 0; i < DirCnt; i++)
    {
        if (dcfPlaybackCurDir== NULL)
            return;
        strncpy((char*)prt_str, dcfPlaybackCurDir->pDirEnt->d_name, 8);
        prt_str[8] = '\0';
        if(OSDIconEndX >= 320)
            uiOSDASCIIStringByColor(prt_str, LOC_DIR_X[sysTVOutOnFlag], 12+i*(OSD_STRING_H+4), OSD_Blk1, 0xC0, 0xC1);
        else
            uiOSDASCIIStringByColor(prt_str, LOC_FILE_X[sysTVOutOnFlag], 12+i*(OSD_STRING_H+4), OSD_Blk1, 0xC0, 0xC1);
        if (i != DirCnt-1)
            (*uiPlaybackListPrev[PlayListDspType])();
    }
#endif
}

void uiOsdDrawPlaybackMenuFile(u8 FileCnt)
{
    u8 i;
    u8  prt_str[9];
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
	u16 LOC_FILE_X [2] = {UI_PLAYBACK_LOC_MENU_FILE_X,UI_PLAYBACK_LOC_MENU_FILE_TV_X };
	u16 LOC_DATE_X [2] = {UI_PLAYBACK_LOC_MENU_DATE_X,UI_PLAYBACK_LOC_MENU_DATE_TV_X };
	u16 LOC_TIME_X [2] = {UI_PLAYBACK_LOC_MENU_TIME_X,UI_PLAYBACK_LOC_MENU_TIME_TV_X };

    u32  startY, StrH, LineH =16;

    startY = UI_PLAYBACK_LOC_MENU_FILE_Y;

    StrH = uiOsdGetFontHeight(CurrLanguage);
    LineH += StrH;
    //uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], UI_PLAYBACK_MENU_LEN, (UI_PLAYBACK_LOC_PAGE_Y-startY+4), LOC_FILE_X[sysTVOutOnFlag]-4, startY-4, OSD_BLK[sysTVOutOnFlag], 0x00000000);

    for (i = 0; i < FileCnt; i++)
    {
        if (dcfPlaybackCurFile == NULL)
            return;
        strncpy((char*)prt_str, dcfPlaybackCurFile->pDirEnt->d_name, 8);
        prt_str[8] = '\0';
        uiOSDASCIIStringByColor(prt_str,  LOC_FILE_X[sysTVOutOnFlag] , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
        if(OSDIconEndX >= 320)
        {
            sprintf((char*)prt_str, "%02d/%02d/%02d",
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD >> 9)+1980,
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5,
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD & 0x001F)
                   );
            uiOSDASCIIStringByColor(prt_str,  LOC_DATE_X[sysTVOutOnFlag] , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
            sprintf( (char*)prt_str, "%02d:%02d:%02d",
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS>>11),
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS & 0x07E0)>>5,
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS & 0x001F)<<1
                    );
            uiOSDASCIIStringByColor(prt_str,  LOC_TIME_X[sysTVOutOnFlag] , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
        }

        if (i != FileCnt-1)
            (*uiPlaybackListPrev[PlayListDspType])();
    }
}

void uiOsdDrawPlaybackMenuDoorSel(u8 select, u8 clean)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u8  StrColor;

    if (clean == 1)
        StrColor = 0xC1;
    else
        StrColor = 0xC0;
    uiOSDASCIIStringByColor("S", (UI_PLAYBACK_LOC_DOOR_SEL_X-40), (UI_PLAYBACK_LOC_DOOR_SEL_Y+UI_PLAYBACK_DOOR_SEL_Y_WEIGHT*select), OSD_BLK[sysTVOutOnFlag],  StrColor, 0xC1);
}

void uiOsdDrawPlaybackMenuNoFile(void)
{
#if (USE_BIG_OSD == 1)
    u8          OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    UI_MULT_ICON *iconInfo;
    u32         StrH, warnY;
#endif

#if (USE_BIG_OSD == 1)
    uiOsdGetIconInfo(OSD_ICON_WARNING_1,&iconInfo);
    StrH = uiOsdGetFontHeight(CurrLanguage);
    warnY = OSDIconMidY-(iconInfo->Icon_h+8+StrH)/2;
    uiOSDIconColorByY(OSD_ICON_WARNING_1 , warnY , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    osdDrawMessage(MSG_NO_FILE, CENTERED, (warnY+iconInfo->Icon_h+8), OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
#else
    uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,(OSDIconMidX-8) , (OSDIconMidY-68) , OSD_Blk1, 0x00 , alpha_3);
    osdDrawMessage(MSG_NO_FILE, CENTERED, 106+osdYShift/2, OSD_Blk1, 0xC0, 0xC1);
#endif

}

void uiOsdDrawPlaybackMenuDoor(u8 key)
{
    static u8 playbackItem;

    switch (key)
    {
        case 0:
            uiOsdDrawPlaybackMenuTitle(PlayListDspType);
            playbackItem = 0;
            uiOsdDrawPlaybackMenuDoorSel(playbackItem, 0);
            break;

        case UI_KEY_UP:
            uiOsdDrawPlaybackMenuDoorSel(playbackItem, 1);
            if (playbackItem == 0)
                playbackItem = 2;
            else
                playbackItem--;
            uiOsdDrawPlaybackMenuDoorSel(playbackItem, 0);
            break;

        case UI_KEY_DOWN:
            uiOsdDrawPlaybackMenuDoorSel(playbackItem, 1);
            if (playbackItem == 2)
                playbackItem = 0;
            else
                playbackItem++;
            uiOsdDrawPlaybackMenuDoorSel(playbackItem, 0);
            break;

        case UI_KEY_ENTER:
        #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)   
            if (playbackItem == 0)
                PlayListDspType = UI_DSP_PLAY_LIST_DIR;
            else
        #endif
            {
                PlayListDspType = playbackItem+1;
                #if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
                dcfDoorChangeDir(uiDoorDirIndex[PlayListDspType-1]);
                #endif
            }
            uiOsdDrawPlaybackMenu(0);
            break;

        case UI_KEY_MODE:
            #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                IduVideo_ClearPKBuf(0);
                splitmenu=0;
            #endif
            uiOsdDrawPlaybackMenuTitle(UI_DSP_PLAY_LIST_DOOR_SELECT);
            if (PlayListDspType == UI_DSP_PLAY_LIST_DIR)
                playbackItem = 0;
            else
                playbackItem = PlayListDspType-1;
            uiOsdDrawPlaybackMenuDoorSel(playbackItem, 0);
            PlayListDspType = UI_DSP_PLAY_LIST_DOOR_SELECT;
            break;

        case UI_KEY_MENU:
            uiOsdDrawPlaybackMenu(UI_KEY_MENU);
            break;


    }
}

void (*uiPlaybackListDrawItem[])(u8) = {
    uiOsdDrawPlaybackMenuFolder,
    uiOsdDrawPlaybackMenuFile,
    uiOsdDrawPlaybackMenuFile,
    uiOsdDrawPlaybackMenuFile,
};

void osdDrawReturn(void)
{
u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
#if (UI_PREVIEW_OSD == 0)
    return;
#endif

    uiOSDIconColorByXY(OSD_ICON_RETURN ,UI_OSD_MENU_LOC_X, UI_OSD_MENU_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
}

void osdDrawVol(void)
{
u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
#if (UI_PREVIEW_OSD == 0)
    return;
#endif

    if (sysVolumnControl == 0)
        uiOSDIconColorByXY(OSD_ICON_MUTE ,UI_OSD_VOL_LOC_X, UI_OSD_VOL_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    else
        uiOSDIconColorByXY(OSD_ICON_VOL ,UI_OSD_VOL_LOC_X, UI_OSD_VOL_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
}

void osdDrawDelete(void)
{
u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
#if (UI_PREVIEW_OSD == 0)
    return;
#endif

    uiOSDIconColorByXY(OSD_ICON_DELETE ,UI_OSD_MENU_LOC_X-100, UI_OSD_MENU_LOC_Y+4, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    uiOSDIconColorByXY(OSD_ICON_PLAY_BAR_UP ,UI_OSD_MENU_LOC_X, UI_OSD_MENU_LOC_Y+80, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    uiOSDIconColorByXY(OSD_ICON_PLAY_BAR_DOWN ,UI_OSD_MENU_LOC_X, UI_OSD_MENU_LOC_Y+300, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
}

#if UI_CALENDAR_SUPPORT
void uiOsdDrawPlaybackMenu(u8 key)
{
    u8          i,j, display_num, move;
    static u32  totalPage = 0, playbackItem = 0, currPage, playbackDir, LastType;
    u32         TotalNum, tmp_page, curLocation;
    static  u8  Local_playback_busy;
#if (USE_BIG_OSD == 1)
    u8          OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
#endif
    DCF_LIST_DIRENT* curDir;
    DCF_LIST_DIRENT* PlaytmpDir;
    
    if(PlayListDspType == UI_DSP_PLAY_LIST_FILE)
    {
        TotalNum = totalPlaybackFileNumADay;
        curLocation = playback_location;
    }
    else
    {
        TotalNum = dcfGetTotalDirCount();
        curLocation = playbackDir;
    }

    DEBUG_UI("Enter File List key %d \r\n",key);
    switch(key)
    {
        case 0:
            uiMenuOSDReset();
            PlayListDspType = UI_DSP_PLAY_LIST_FILE;
            uiOsdDrawPlaybackMenuTitle(PlayListDspType);
            osdDrawReturn();
            osdDrawDelete();

            #if (REMOTE_FILE_PLAYBACK && NIC_SUPPORT)
            if(Local_playback_busy == 1)
            {
                Fileplaying = 0;
                Local_playback_busy = 0;
            }
            #endif
            
            /*Calculate the number of files*/
            totalPlaybackFileNumADay = 0;
            
            // fetch files info of current Dir, reload the files, 170214
            curDir = dcfPlaybackCurDir = dcfPlaybackDayInfo[sysPlaybackDay-1].dirHead;
            if (dcfFileInit(curDir, SCANFILEENTRY) == 0)
            {
            	return;
            }
            
            for(i = 0; i < DCF_MAX_MULTI_FILE; i++)
            {
            	DEBUG_MAGENTA("********** CH %d *************\n",i);
            	DEBUG_MAGENTA("curDir->ChTotal    [%d]: %d\n", i,curDir->ChTotal[i]);
            	DEBUG_MAGENTA("curDir->SumManual  [%d]: %d\n", i,curDir->SumManual[i]);                
            	DEBUG_MAGENTA("curDir->SumSchedule[%d]: %d\n", i,curDir->SumSchedule[i]);                
            	DEBUG_MAGENTA("curDir->SumDetect  [%d]: %d\n", i,curDir->SumDetect[i]);
            	DEBUG_MAGENTA("curDir->SumRing  [%d]: %d\n", i,curDir->SumRing[i]);
            	DEBUG_MAGENTA("******************************\n");
            }
            
            PlaytmpDir = dcfPlaybackDayInfo[sysPlaybackDay-1].dirTail;
            for (i = 0; i < dcfPlaybackDayInfo[sysPlaybackDay-1].DirNum; i++)
            {
                for (j = 0; j < 4; j++)
                {
                    if (sysPlaybackCamList & (0x1 << j))
                    {
                        if (sysPlaybackType & DCF_DISTRIB_MANU)
                            totalPlaybackFileNumADay += PlaytmpDir->SumManual[j];
                        if (sysPlaybackType & DCF_DISTRIB_SCHE)
                            totalPlaybackFileNumADay += PlaytmpDir->SumSchedule[j];
                        if (sysPlaybackType & DCF_DISTRIB_MOTN)
                            totalPlaybackFileNumADay += PlaytmpDir->SumDetect[j];
                        if (sysPlaybackType & DCF_DISTRIB_RING)
                            totalPlaybackFileNumADay += PlaytmpDir->SumRing[j];
                    }
                }
                PlaytmpDir = PlaytmpDir->playbackNext;
            }
            TotalNum = totalPlaybackFileNumADay;
            if(TotalNum == 0 || (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY))
            {
                uiOsdDrawPlaybackMenuNoFile();
                return;
            }

            dcfPlaybackCurDir = dcfPlaybackDayInfo[sysPlaybackDay-1].dirTail;
            if(dcfScanFileOnPlaybackDir() == 0)
            {
                DEBUG_UI("Folder %s inital fail \r\n",dcfPlaybackCurDir->pDirEnt->d_name);
                uiOsdDrawPlaybackMenuNoFile();
                return;
            }
            do
            {
                if (uiOsdPlaybackFileFind() == 1)
                    break;
                dcfPlaybackFilePrev();
            }while(dcfPlaybackCurFile != dcfGetPlaybackFileListHead());
            
            playback_location = 1;
            curLocation = 1;
            totalPage = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
            currPage = 1;
            if (totalPage == currPage)
                display_num = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
            else
                display_num = UI_PLAYBACK_NUM_PER_PAGE;
            playbackItem = 0;
            (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
            uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
            for (i = 0; i < (display_num-1); i++)
                (*uiPlaybackListNext[PlayListDspType])();
                uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            #if RFIU_SUPPORT
                uiSetRfDisplayMode(UI_MENU_RF_ENTER_PLAYBACK);
            #endif
            break;

        case UI_KEY_UP: /* useless */
            if (TotalNum == 0 || (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY))
                break;
            curLocation--;
            if (curLocation == 0)
                curLocation = TotalNum;
            if ((*uiPlaybackListNext[PlayListDspType])() == 0)
                break;
            tmp_page = (curLocation + UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
            if (tmp_page != currPage)
            {
                #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                    IduVideo_ClearPKBuf(0);
                    uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                #endif

                if (totalPage == tmp_page)
                    display_num = (TotalNum + UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                else
                    display_num = UI_PLAYBACK_NUM_PER_PAGE;
                for (i = 0; i < (display_num-1); i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                playbackItem = display_num-1;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                currPage = tmp_page;
                uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            }
            else
            {
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                if (curLocation == TotalNum)
                    playbackItem = (TotalNum + UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE;
                else
                    playbackItem--;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
            }
            break;

        case UI_KEY_DOWN: /* useless */
            if (TotalNum == 0 || (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY))
                break;
            curLocation++;
            if(curLocation > TotalNum)
                curLocation = 1;
            if ((*uiPlaybackListPrev[PlayListDspType])() == 0)
                break;
            tmp_page = (curLocation+UI_PLAYBACK_NUM_PER_PAGE - 1)/UI_PLAYBACK_NUM_PER_PAGE;
            if (tmp_page != currPage)
            {
                #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                    IduVideo_ClearPKBuf(0);
                    uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                #endif
                if (totalPage == tmp_page)
                    display_num = (TotalNum + UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                else
                    display_num = UI_PLAYBACK_NUM_PER_PAGE;
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                /*always show first file in this page*/
                for (i = 0; i < display_num-1; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                currPage = tmp_page;
                uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            }
            else
            {
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                if (curLocation%UI_PLAYBACK_NUM_PER_PAGE == 1)
                    playbackItem = 0;
                else
                    playbackItem++;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
            }
            break;

        case UI_KEY_RIGHT:
            /*Page Down*/
            if(TotalNum == 0 || (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY))
                break;
            #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                IduVideo_ClearPKBuf(0);
            #endif
            if (TotalNum <= UI_PLAYBACK_NUM_PER_PAGE)
            {
                /*only one page, always in first item*/
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                curLocation = 1;
                move = playbackItem;
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                for (i = 0; i < move; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                break;
            }
            currPage++;
            if (currPage > totalPage)
            {
                currPage = 1;
                move = TotalNum - curLocation+1;
                curLocation = 1;
                for (i = 0; i < move; i++)
                    (*uiPlaybackListPrev[PlayListDspType])();
                display_num = UI_PLAYBACK_NUM_PER_PAGE;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                for (i = 0; i < (display_num-1); i++)
                    (*uiPlaybackListNext[PlayListDspType])();
            }
            else
            {
                move = UI_PLAYBACK_NUM_PER_PAGE - playbackItem;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                playbackItem = 0;
                for (i = 0; i < move; i++)
                    (*uiPlaybackListPrev[PlayListDspType])();
                if (currPage == totalPage)
                    display_num = (TotalNum + UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                else
                    display_num = UI_PLAYBACK_NUM_PER_PAGE;
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                curLocation = curLocation+move;
                for (i = 0; i < display_num-1; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
            }
            uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            break;

        case UI_KEY_LEFT:
            /*Page UP*/
            if(TotalNum == 0 || (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY))
                break;
            #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                IduVideo_ClearPKBuf(0);
            #endif
            if (TotalNum <= UI_PLAYBACK_NUM_PER_PAGE)
            {
                /*only one page, always in first item*/
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                curLocation = 1;
                move = playbackItem;
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                for (i = 0; i < move; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                break;
            }
            currPage--;
            if (currPage < 1)
            {
                currPage = totalPage;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                display_num = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                move = playbackItem+display_num;
                for (i = 0; i < move; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                curLocation = TotalNum - display_num+1;
                for (i = 0; i < (display_num-1); i++)
                    (*uiPlaybackListNext[PlayListDspType])();
            }
            else
            {
                display_num = UI_PLAYBACK_NUM_PER_PAGE;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                move = playbackItem+display_num;
                for (i = 0; i < move; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                curLocation = curLocation-move;
                for (i = 0; i < (display_num-1); i++)
                    (*uiPlaybackListNext[PlayListDspType])();
            }
            uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            break;

        case UI_KEY_ENTER:
            if ((TouchExtKey >= 0) && (playbackItem != TouchExtKey))  /*touch select*/
            {
                if (TotalNum == 0 || (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY))
                    break;
                if (totalPage == currPage)
                    display_num = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                else
                    display_num = UI_PLAYBACK_NUM_PER_PAGE;
                if (TouchExtKey >= display_num)
                    break;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                if (playbackItem > TouchExtKey)
                {
                    move = playbackItem-TouchExtKey;
                    curLocation -= move;
                    for (i = 0; i < move; i++)
                        (*uiPlaybackListNext[PlayListDspType])();
                }
                else
                {
                    move = TouchExtKey-playbackItem;
                    curLocation += move;
                    for (i = 0; i < move; i++)
                        (*uiPlaybackListPrev[PlayListDspType])();
                }
                playbackItem = TouchExtKey;
                playback_location = curLocation;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                break;
            }
            else // play
            {
                if(TotalNum == 0 || (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY))
                    break;
                DEBUG_UI("Change to Playback Mode %s\r\n", dcfPlaybackCurFile->pDirEnt->d_name);
                LastType = PlayListDspType;
                PlayListDspType = UI_DSP_PLAY_LIST_PLAYBACK;    /*playback mode*/
                playbackflag = 2;
                uiMenuEnable = 0x41;
                //Iframe_flag = 1;  // 1: We just need I-frame 0: play whole MP4
                siuOpMode = 0;
                sysCameraMode = SYS_CAMERA_MODE_PLAYBACK;
                uiMenuOSDReset();
#if (USE_BIG_OSD == 1)
                (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
                uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
#else
                (*OSDDisplay[sysTVOutOnFlag])(0, 0, 0, OSDDispWidth[sysTVOutOnFlag], 79);
                (*OSDDisplay[sysTVOutOnFlag])(1, 0, 80, OSDDispWidth[sysTVOutOnFlag], 179);
                (*OSDDisplay[sysTVOutOnFlag])(2, 0, 180, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
                uiOsdEnable(OSD_Blk0);
                uiOsdEnable(OSD_Blk2);
#endif
                IduVidBuf0Addr = (u32)MainVideodisplaybuf[0];
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf0Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
                osdDrawPlayIcon();
            #endif
                osdDrawPlayIndicator(UI_PLAY_ICON_PLAY);
            #if (REMOTE_FILE_PLAYBACK && NIC_SUPPORT)
                if(Fileplaying == 1)
                    uiOSDASCIIStringByColorCenter((u8*)"Remote device is busy",OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
                    //uiGraphDrawPlaybackbusy();
                else
                {
                    Local_playback_busy = 1;
                    Fileplaying = 1;
                    uiReadVideoFile();
                }
                #else
                    uiReadVideoFile();
            #endif
                MyHandler.MenuMode = PLAYBACK_MODE;
                uiEnterMenu(UI_MENU_NODE_PLAYBACK_MODE);
                
            }
            break;

        case UI_KEY_MENU:
            PlayListDspType = UI_DSP_PLAY_LIST_DIR;
            uiMenuOSDReset();
            (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
            uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
            uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
            
            uiEnterMenu(UI_MENU_NODE_PLAYBACK_CALENDAR);
            uiGraphDrawPlaybackList(UI_KEY_MODE, 0);
            break;

        case UI_KEY_DELETE:
            IduVideo_ClearPKBuf(0);
            uiOsdDrawPlaybackMenuTitle(PlayListDspType);
            osdDrawReturn();
            osdDrawDelete();
            
            #if (REMOTE_FILE_PLAYBACK && NIC_SUPPORT)
            if(Local_playback_busy == 1)
            {
                Fileplaying = 0;
                Local_playback_busy = 0;
            }
            #endif
            
            if( curLocation > TotalNum)
            {
                curLocation = 1;
            }
            if(TotalNum == 0 || (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY))
            {
                uiOsdDrawPlaybackMenuNoFile();
                return;
            }
            totalPage = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
            currPage = (curLocation+UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
            if (totalPage == currPage)
                display_num = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
            else
                display_num = UI_PLAYBACK_NUM_PER_PAGE;
            playbackItem = (curLocation+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE;
            for (i = 0; i < playbackItem; i++)
                (*uiPlaybackListNext[PlayListDspType])();
            (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
            uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
            for (i = 0; i < (display_num-playbackItem-1); i++)
                (*uiPlaybackListNext[PlayListDspType])();
            #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
            uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            #endif
            break;            

    }

    if(PlayListDspType == UI_DSP_PLAY_LIST_FILE)
    {
        playback_location = curLocation;
    }
    else if(PlayListDspType == UI_DSP_PLAY_LIST_DIR)
    {
        playbackDir = curLocation;
    }
    else if(PlayListDspType == UI_DSP_PLAY_LIST_DOOR_PIC)
    {
        playback_location = curLocation;
    }
    else if(PlayListDspType == UI_DSP_PLAY_LIST_DOOR_ALB)
    {
        playback_location = curLocation;
    }
}
#else
void uiOsdDrawPlaybackMenu(u8 key)
{
    u8          i, display_num, move;
    static u32  totalPage = 0, playbackItem = 0, currPage, playbackDir, LastType;
    u32         TotalNum, tmp_page, curLocation;
    static  u8  Local_playback_busy;
#if (USE_BIG_OSD == 1)
    u8          OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    UI_MULT_ICON *iconInfo;
    u32         StrH, warnY;
#endif

    switch (PlayListDspType)
    {
        case UI_DSP_PLAY_LIST_DIR:
            TotalNum = dcfGetTotalDirCount();
            curLocation = playbackDir;
             DEBUG_YELLOW("%d %s %d %d\n",__LINE__, __FILE__,TotalNum,curLocation);
            break;

        case UI_DSP_PLAY_LIST_FILE:
            TotalNum = dcfGetCurDirFileCount();
            curLocation = playback_location;
            break;
    }

    switch(key)
    {
        case 0:
            #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                Iframe_flag=1;
                splitmenu=1;
            #else
                IduVideo_ClearPKBuf(0);
            #endif
            uiMenuOSDReset();
            #if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
                PlayListDspType = UI_DSP_PLAY_LIST_DIR;
                TotalNum = dcfGetTotalDirCount();
                curLocation = playbackDir;
            #elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
                PlayListDspType = UI_DSP_PLAY_LIST_FILE;
                TotalNum = dcfGetCurDirFileCount();
                curLocation = playback_location;
            #endif
            uiOsdDrawPlaybackMenuTitle(PlayListDspType);
            osdDrawReturn();
            //osdDrawDelete();
            if(TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN) == 0)
            {
                uiOsdDrawPlaybackMenuNoFile();
                return;
            }
            dcfPlaybackCurDir = dcfGetVideoDirListTail();
            dcfPlaybackCurFile = dcfGetPlaybackFileListTail();

            curLocation = 1;
            totalPage = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
            currPage = 1;
            if (totalPage == currPage)
                display_num = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
            else
                display_num = UI_PLAYBACK_NUM_PER_PAGE;
            playbackItem = 0;
            (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
            uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
            for (i = 0; i < (display_num-1); i++)
                (*uiPlaybackListNext[PlayListDspType])();
                uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            #if RFIU_SUPPORT
                uiSetRfDisplayMode(UI_MENU_RF_ENTER_PLAYBACK);
            #endif
            break;

        case UI_KEY_UP:
            if (TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN)==0)
                break;
            curLocation--;
            if (curLocation == 0)
                curLocation = TotalNum;
            if ((*uiPlaybackListNext[PlayListDspType])() == 0)
                break;
            tmp_page = (curLocation + UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
            if (tmp_page != currPage)
            {
                #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                    IduVideo_ClearPKBuf(0);
                    uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                #endif

                if (totalPage == tmp_page)
                    display_num = (TotalNum + UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                else
                    display_num = UI_PLAYBACK_NUM_PER_PAGE;
                for (i = 0; i < (display_num-1); i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                playbackItem = display_num-1;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                currPage = tmp_page;
                uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            }
            else
            {
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                if (curLocation == TotalNum)
                    playbackItem = (TotalNum + UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE;
                else
                    playbackItem--;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
            }
            break;

        case UI_KEY_DOWN:
            if (TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN)==0)
                break;
            curLocation++;
            if(curLocation > TotalNum)
                curLocation = 1;
            if ((*uiPlaybackListPrev[PlayListDspType])() == 0)
                break;
            tmp_page = (curLocation+UI_PLAYBACK_NUM_PER_PAGE - 1)/UI_PLAYBACK_NUM_PER_PAGE;
            if (tmp_page != currPage)
            {
                #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                    IduVideo_ClearPKBuf(0);
                    uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                #endif
                if (totalPage == tmp_page)
                    display_num = (TotalNum + UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                else
                    display_num = UI_PLAYBACK_NUM_PER_PAGE;
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                /*always show first file in this page*/
                for (i = 0; i < display_num-1; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                currPage = tmp_page;
                uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            }
            else
            {
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                if (curLocation%UI_PLAYBACK_NUM_PER_PAGE == 1)
                    playbackItem = 0;
                else
                    playbackItem++;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
            }
            break;

        case UI_KEY_RIGHT:
            /*Page Down*/
            if(TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN)==0)
                break;
            #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                IduVideo_ClearPKBuf(0);
            #endif
            if (TotalNum <= UI_PLAYBACK_NUM_PER_PAGE)
            {
                /*only one page, always in first item*/
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                curLocation = 1;
                move = playbackItem;
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                for (i = 0; i < move; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                break;
            }
            currPage++;
            if (currPage > totalPage)
            {
                currPage = 1;
                move = TotalNum - curLocation+1;
                curLocation = 1;
                for (i = 0; i < move; i++)
                    (*uiPlaybackListPrev[PlayListDspType])();
                display_num = UI_PLAYBACK_NUM_PER_PAGE;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                for (i = 0; i < (display_num-1); i++)
                    (*uiPlaybackListNext[PlayListDspType])();
            }
            else
            {
                move = UI_PLAYBACK_NUM_PER_PAGE - playbackItem;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                playbackItem = 0;
                for (i = 0; i < move; i++)
                    (*uiPlaybackListPrev[PlayListDspType])();
                if (currPage == totalPage)
                    display_num = (TotalNum + UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                else
                    display_num = UI_PLAYBACK_NUM_PER_PAGE;
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                curLocation = curLocation+move;
                for (i = 0; i < display_num-1; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
            }
            uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            break;

        case UI_KEY_LEFT:
            /*Page UP*/
            if(TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN)==0)
                break;
            #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                IduVideo_ClearPKBuf(0);
            #endif
            if (TotalNum <= UI_PLAYBACK_NUM_PER_PAGE)
            {
                /*only one page, always in first item*/
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                curLocation = 1;
                move = playbackItem;
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                for (i = 0; i < move; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                break;
            }
            currPage--;
            if (currPage < 1)
            {
                currPage = totalPage;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                display_num = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                move = playbackItem+display_num;
                for (i = 0; i < move; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                curLocation = TotalNum - display_num+1;
                for (i = 0; i < (display_num-1); i++)
                    (*uiPlaybackListNext[PlayListDspType])();
            }
            else
            {
                display_num = UI_PLAYBACK_NUM_PER_PAGE;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                move = playbackItem+display_num;
                for (i = 0; i < move; i++)
                    (*uiPlaybackListNext[PlayListDspType])();
                (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                playbackItem = 0;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                curLocation = curLocation-move;
                for (i = 0; i < (display_num-1); i++)
                    (*uiPlaybackListNext[PlayListDspType])();
            }
            uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            break;

        case UI_KEY_ENTER:
            if ((TouchExtKey >= 0) && (playbackItem != TouchExtKey))  /*touch select*/
            {
                if (TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN)==0)
                    break;
                if (totalPage == currPage)
                    display_num = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                else
                    display_num = UI_PLAYBACK_NUM_PER_PAGE;
                if (TouchExtKey >= display_num)
                    break;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 1);
                if (playbackItem > TouchExtKey)
                {
                    move = playbackItem-TouchExtKey;
                    curLocation -= move;
                    for (i = 0; i < move; i++)
                        (*uiPlaybackListNext[PlayListDspType])();
                }
                else
                {
                    move = TouchExtKey-playbackItem;
                    curLocation += move;
                    for (i = 0; i < move; i++)
                        (*uiPlaybackListPrev[PlayListDspType])();
                }
                playbackItem = TouchExtKey;
                playback_location = curLocation;
                uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                break;
            }
            else
            {
                if(PlayListDspType == UI_DSP_PLAY_LIST_DIR)
                {
                    if(TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN) == 0)
                        break;
                    DEBUG_UI("Enter folder %s\r\n",dcfPlaybackCurDir->pDirEnt->d_name);
                    PlayListDspType = UI_DSP_PLAY_LIST_FILE;
                    uiOsdDrawPlaybackMenuTitle(PlayListDspType);
                    osdDrawReturn();
                    osdDrawDelete();
                    if(TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN) == 0)
                    {
                        uiOsdDrawPlaybackMenuNoFile();
                        return;
                    }
                    if(dcfScanFileOnPlaybackDir() == 0)
                    {
                        DEBUG_UI("Folder %s inital fail \r\n",dcfPlaybackCurDir->pDirEnt->d_name);
                        uiOsdDrawPlaybackMenuNoFile();
                        return;
                    }
                    TotalNum = dcfGetCurDirFileCount();
                    playback_location = 1;
                    curLocation = playback_location;
                    totalPage = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
                    currPage = 1;
                    if (totalPage == currPage)
                        display_num = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
                    else
                        display_num = UI_PLAYBACK_NUM_PER_PAGE;
                    playbackItem = 0;
                    (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
                    uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
                    for (i = 0; i < (display_num-1); i++)
                        (*uiPlaybackListNext[PlayListDspType])();
                    uiOsdDrawPlaybackMenuPage(totalPage, currPage);
                }
                else
                {
                    if(TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN) == 0)
                        break;
                    DEBUG_UI("Change to Playback Mode %s\r\n", dcfPlaybackCurFile->pDirEnt->d_name);
                    LastType = PlayListDspType;
                    PlayListDspType = UI_DSP_PLAY_LIST_PLAYBACK;    /*playback mode*/
                    playbackflag = 2;
                    uiMenuEnable = 0x41;
                #if(FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                    #if(HW_BOARD_OPTION == MR9670_HECHI) || (HW_BOARD_OPTION == MR8120_RX_HECHI)
                        splitmenu = 0; // Toby131126 (800*480)
                    #else
                        splitmenu = 2; // Toby130306 (640*480)
                    #endif
                        Iframe_flag = 0;  // 1: We just need I-frame 0: play whole MP4
                #else
                        Iframe_flag = 1;  // 1: We just need I-frame 0: play whole MP4
                #endif                
                    siuOpMode = 0;
                    sysCameraMode = SYS_CAMERA_MODE_PLAYBACK;
                    uiMenuOSDReset();
#if (USE_BIG_OSD == 1)
                    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
                    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
#else
                    (*OSDDisplay[sysTVOutOnFlag])(0, 0, 0, OSDDispWidth[sysTVOutOnFlag], 79);
                    (*OSDDisplay[sysTVOutOnFlag])(1, 0, 80, OSDDispWidth[sysTVOutOnFlag], 179);
                    (*OSDDisplay[sysTVOutOnFlag])(2, 0, 180, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
                    uiOsdEnable(OSD_Blk0);
                    uiOsdEnable(OSD_Blk2);
#endif
                    IduVidBuf0Addr = (u32)MainVideodisplaybuf[0];
                #if NEW_IDU_BRI
                    BRI_IADDR_Y = IduVidBuf0Addr;
                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                #endif
                #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
                    osdDrawPlayIcon();
                #endif
                #if (REMOTE_FILE_PLAYBACK && NIC_SUPPORT)
                if(Fileplaying == 1)
                    uiGraphDrawPlaybackbusy();
                else
                {
                    Local_playback_busy = 1;
                    Fileplaying = 1;
                    uiReadVideoFile();
                }
                #else
                    uiReadVideoFile();
                #endif
                    MyHandler.MenuMode = PLAYBACK_MODE;
                    uiEnterMenu(UI_MENU_NODE_PLAYBACK_MODE);
                }
            }
            break;

        case UI_KEY_MENU:
            #if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
                if(PlayListDspType == UI_DSP_PLAY_LIST_DIR)
            #elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
                if(PlayListDspType == UI_DSP_PLAY_LIST_FILE)
            #elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
                if (PlayListDspType == UI_DSP_PLAY_LIST_DOOR_SELECT)
            #endif
            {
                #if (UI_SUPPORT_TREE == 1)
                #if 0
                for(i=0; i<4; i++)
                {
                    if (MultiChannelGetCaptureVideoStatus(i) == UI_REC_STATUS_RECING)
                    {
                        uiCurRecStatus[i]=UI_REC_STATUS_RECING;
                    }
                    else
                        uiCurRecStatus[i]=UI_REC_STATUS_NONE;
                }
                #endif
                DEBUG_UI("Playback list to Setup mode\r\n");
                uiCaptureVideoStop();
                MyHandler.MenuMode = SETUP_MODE;
                uiOsdDisableAll();
                #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_D1) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
                #else
                //if(sysTVOutOnFlag)
                    //TV_reset();
                #endif
                uiEnterMenu(UI_MENU_NODE_PLAYBACK);
                uiGraphDrawMenu();
                dcfPlaybackCurDir = dcfGetVideoDirListTail();
                dcfScanFileOnPlaybackDir();
                #else
                DEBUG_UI("Not Support Setup mode\r\n");
                #endif
                break;
            }
            if(PlayListDspType == UI_DSP_PLAY_LIST_FILE)
            {
                DEBUG_UI("File list to Folder List %d, %d\r\n",dcfGetTotalDirCount(), playbackDir);
                PlayListDspType = UI_DSP_PLAY_LIST_DIR;
                TotalNum = dcfGetTotalDirCount();
                curLocation = playbackDir;
            }
            else if(PlayListDspType == UI_DSP_PLAY_LIST_PLAYBACK)
            {
                PlayListDspType = LastType;
                DEBUG_UI("Playback return to File List %d\r\n", PlayListDspType);
                if (PlayListDspType == UI_DSP_PLAY_LIST_FILE)
                {
                    TotalNum = dcfGetCurDirFileCount();
                    curLocation = playback_location;
                }
                IduVideo_ClearPKBuf(0);
                #if 0
                if (sysTVOutOnFlag)
                {
                    if(TvOutMode == SYS_TV_OUT_PAL) //Lsk 090629 : when Play D1 video than return menu, reset TV-out setting
                    {
                       tvACTSTAEND = TV_ACTSTAEND_PAL;
                       tvTVFB_STRIDE=0x00000140;    // Video Buffer stride = 640
                    }
                    else
                    {
                       tvACTSTAEND = TV_ACTSTAEND_NTSC;
                       tvTVFB_STRIDE=0x00000140;    // Video Buffer stride = 640
                    }
                }
                #endif
                IduVidBuf0Addr = (u32)PKBuf0;
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf0Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            }
            #if(FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
            Iframe_flag=1;
            splitmenu=1;
            #endif
            #if (REMOTE_FILE_PLAYBACK && NIC_SUPPORT)
            if(Local_playback_busy == 1)
            {
                Fileplaying = 0;
                Local_playback_busy = 0;
            }
            #endif
            uiOsdDrawPlaybackMenuTitle(PlayListDspType);
            osdDrawReturn();
            if(PlayListDspType != UI_DSP_PLAY_LIST_DIR)
                osdDrawDelete();
            if(TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN) == 0)
            {
                uiOsdDrawPlaybackMenuNoFile();
                return;
            }
            totalPage = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
            currPage = (curLocation+UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
            if (totalPage == currPage)
                display_num = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
            else
                display_num = UI_PLAYBACK_NUM_PER_PAGE;
            playbackItem = (curLocation+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE;
            for (i = 0; i < playbackItem; i++)
                (*uiPlaybackListNext[PlayListDspType])();
            (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
            uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
            for (i = 0; i < (display_num-playbackItem-1); i++)
                (*uiPlaybackListNext[PlayListDspType])();
            uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            break;

        case UI_KEY_DELETE:
            IduVideo_ClearPKBuf(0);
            uiOsdDrawPlaybackMenuTitle(PlayListDspType);
            osdDrawReturn();
            osdDrawDelete();
            if( curLocation > TotalNum)
            {
                curLocation = 1;
            }
            if(TotalNum == 0 || sysGetStorageStatus(SYS_I_STORAGE_MAIN) == 0)
            {
                uiOsdDrawPlaybackMenuNoFile();
                return;
            }
            totalPage = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
            currPage = (curLocation+UI_PLAYBACK_NUM_PER_PAGE-1)/UI_PLAYBACK_NUM_PER_PAGE;
            if (totalPage == currPage)
                display_num = (TotalNum+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE+1;
            else
                display_num = UI_PLAYBACK_NUM_PER_PAGE;
            playbackItem = (curLocation+UI_PLAYBACK_NUM_PER_PAGE-1)%UI_PLAYBACK_NUM_PER_PAGE;
             DEBUG_YELLOW("%d %s %d %d\n",__LINE__, __FILE__,playbackItem,display_num);
            for (i = 0; i < playbackItem; i++)
                (*uiPlaybackListNext[PlayListDspType])();
             DEBUG_YELLOW("%d %s %s\n",__LINE__, __FILE__,dcfPlaybackCurDir->pDirEnt->d_name);
            (*uiPlaybackListDrawItem[PlayListDspType])(display_num);
            uiOsdDrawPlaybackMenuFrame(PlayListDspType, playbackItem, 0);
            for (i = 0; i < (display_num-playbackItem-1); i++)
                (*uiPlaybackListNext[PlayListDspType])();
            #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
            uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            #endif
            break;

    }

    if(PlayListDspType == UI_DSP_PLAY_LIST_FILE)
    {
        playback_location = curLocation;
    }
    else if(PlayListDspType == UI_DSP_PLAY_LIST_DIR)
    {
        playbackDir = curLocation;
    }
    else if(PlayListDspType == UI_DSP_PLAY_LIST_DOOR_PIC)
    {
        playback_location = curLocation;
    }
    else if(PlayListDspType == UI_DSP_PLAY_LIST_DOOR_ALB)
    {
        playback_location = curLocation;
    }
}
#endif

void uiOsdDrawConfirmSelect(u8 key, u8 type)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16  LOC_X[2] = {UI_CONFIRM_YES_X, UI_CONFIRM_YES_TV_X};
    static u8 CurSel=0, SetType = 0;

    DEBUG_UI("uiOsdDrawConfirmSelect\r\n");
    switch (key)
    {
        case 0:
            SetType = type;
            IduVideo_ClearPKBuf(0);
            uiOsdDisableAll();
            (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
            uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
            uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
            
            if (type == 1)  /*formate*/
            {
                if(sysTVOutOnFlag == 0)
                    uiOSDASCIIStringByColor((u8*)"ARE YOU SURE TO DELETE ALL STORED FILES?", UI_CONFIRM_FORMATE_X , UI_CONFIRM_FORMATE_Y, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
            
                else if(sysTVOutOnFlag == 1)
                {
                    uiOSDASCIIStringByColor((u8*)"ARE YOU SURE TO ", UI_CONFIRM_FORMATE_TV_X , UI_CONFIRM_FORMATE_Y-26, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
                    uiOSDASCIIStringByColor((u8*)"DELETE ALL STORED FILES?", UI_CONFIRM_FORMATE_TV_X-64 , UI_CONFIRM_FORMATE_Y, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
                }
            }   
            else if (type == 2) /*Deltet*/
            {                
                if(sysTVOutOnFlag == 0)
                    uiOSDASCIIStringByColor((u8*)"ARE YOU SURE TO DELETE THIS FILES?", UI_CONFIRM_DELETE_X , UI_CONFIRM_DELETE_Y, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
                        
                else if(sysTVOutOnFlag == 1)
                {
                    uiOSDASCIIStringByColor((u8*)"ARE YOU SURE TO ", UI_CONFIRM_DELETE_TV_X , UI_CONFIRM_DELETE_Y-26, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
                    uiOSDASCIIStringByColor((u8*)"DELETE THIS FILES?", UI_CONFIRM_DELETE_TV_X-16 , UI_CONFIRM_DELETE_Y, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
                }
            }       
            CurSel = 0;
            uiOSDASCIIStringByColor((u8*)"YES",  LOC_X[sysTVOutOnFlag],      UI_CONFIRM_YES_Y,               OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
            uiOSDASCIIStringByColor((u8*)"NO", LOC_X[sysTVOutOnFlag] ,     (UI_CONFIRM_YES_Y+80),          OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
            uiOSDASCIIStringByColor((u8*)">>", (LOC_X[sysTVOutOnFlag]-50) , (UI_CONFIRM_YES_Y+80*CurSel),   OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
            uiOSDASCIIStringByColor((u8*)"OK",  700, 480, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
            osdDrawReturn();
            break;
            
        case UI_KEY_UP:
            uiOSDASCIIStringByColor((u8*)">>", (LOC_X[sysTVOutOnFlag]-50) , (UI_CONFIRM_YES_Y+80*CurSel), OSD_BLK[sysTVOutOnFlag] , 0x00, 0xC1);
            CurSel = TouchExtKey;
            uiOSDASCIIStringByColor((u8*)">>", (LOC_X[sysTVOutOnFlag]-50) , (UI_CONFIRM_YES_Y+80*CurSel), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
            break;

        case UI_KEY_MENU:
            if (SetType == 1)   /*formate*/
            {
                MyHandler.MenuMode = SETUP_MODE;
                uiEnterMenu(UI_MENU_NODE_SET_FORMAT_NO);
                uiOsdDisable(OSD_BLK[sysTVOutOnFlag]);
                uiGraphDrawMenu();
            }
            else if (SetType == 2) /*Deltet*/
            {            
                MyHandler.MenuMode = PLAYBACK_MENU_MODE;
                uiEnterMenu(UI_MENU_NODE_SET_PLAYBACK);
                uiOsdDrawPlaybackMenu(UI_KEY_DELETE);
                DEBUG_UI("change to Playback list\r\n");
            }
            break;

        case UI_KEY_ENTER:
            if (TouchExtKey != -1)
            {
                CurSel = TouchExtKey;
            }
            if (SetType == 1)   /*formate*/
            {
                MyHandler.MenuMode = SETUP_MODE;
                uiOsdDisable(OSD_BLK[sysTVOutOnFlag]);
                if (CurSel == 1)
                {
                    uiEnterMenu(UI_MENU_NODE_SET_FORMAT_NO);
                    uiFlowRunAction();
                }
                else
                {
                    uiEnterMenu(UI_MENU_NODE_SET_FORMAT_NO);
                    uiGraphDrawMenu();
                }
            }
            else if (SetType == 2) /*Delete*/
            {
                MyHandler.MenuMode = PLAYBACK_MENU_MODE;
                if (CurSel == UI_MENU_DELETE_YES)
                {
                    //uiEnterMenu(UI_MENU_NODE_DELETE_YES);
                    uiFlowRunAction();
                }
                else
                {
                    uiEnterMenu(UI_MENU_NODE_SET_PLAYBACK);
                    uiOsdDrawPlaybackMenu(UI_KEY_DELETE);
                    DEBUG_UI("change to Playback list\r\n");
                }
            }
            break;

        default:
            DEBUG_UI("Error Key %d in confirm mode\r\n",key);
            return;
    }
}

#if 0
void uiOsdDrawMaskAreaClean(void)
{
    u8  maskMaxColNum = MASKAREA_MAX_COLUMN, maskMaxRowNum = MASKAREA_MAX_ROW;
    u8  x, y, space_x, space_y;
    u8  OSD_BLK[2] = {IDU_OSD_L1_WINDOW_0, IDU_OSD_L0_WINDOW_0};
    u16 drawX, drawW, drawY;
    u32 LinColar;

    space_x = OSDIconEndX/maskMaxColNum;
    space_y = OSDIconEndY/maskMaxRowNum;

    x = MotionMaskCursor%maskMaxColNum;
    y = MotionMaskCursor/maskMaxColNum;

    drawX = x*space_x;
    drawY = y*space_y+1;
    /*draw left part*/
    switch(drawX&0x0003)
    {
        case 0:
            if(MotionMaskArea[y][x] != 0)
                LinColar = 0xC4C4C4C0;
            else
                LinColar = 0x000000C0;
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), drawX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
            drawX = drawX+4;
            drawW = space_x-4;
            break;

        case 1:
            if((x>0)&&(MotionMaskArea[y][x-1]) != 0)
            {
                if(MotionMaskArea[y][x] != 0)
                    LinColar = 0xC4C4C0C4;
                else
                    LinColar = 0x0000C0C4;
            }
            else
            {
                if(MotionMaskArea[y][x] != 0)
                    LinColar = 0xC4C4C000;
                else
                    LinColar = 0x0000C000;

            }
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), (drawX-1), drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
            drawX = drawX+3;
            drawW = space_x-3;
            break;

        case 2:
            if((x>0)&&(MotionMaskArea[y][x-1]) != 0)
            {
                if(MotionMaskArea[y][x] != 0)
                    LinColar = 0xC4C0C4C4;
                else
                    LinColar = 0x00C0C4C4;
            }
            else
            {
                if(MotionMaskArea[y][x] != 0)
                    LinColar = 0xC4C00000;
                else
                    LinColar = 0x00C00000;
            }
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), (drawX-2), drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
            drawX = drawX+2;
            drawW = space_x-2;
            break;

        case 3:
            drawX = drawX+1;
            drawW = space_x-1;
            break;

    }

    if (MotionMaskArea[y][x] != 0)
        LinColar = 0xC4C4C4C4;
    else
        LinColar = 0;

    if( x == (maskMaxColNum-1))
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], ((OSDDispWidth[sysTVOutOnFlag]-drawX)&0xFFFC), (space_y-1), drawX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
    else
    {
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], (drawW&0xFFFC), (space_y-1), drawX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
        drawX += (drawW&0xFFFC);
        /*draw right part*/
        drawW = drawW&0x0003;
        switch(drawW)
        {
            case 0:
                return;

            case 1:
                if(MotionMaskArea[y][x+1] == 0)
                {
                    if(MotionMaskArea[y][x] == 0)
                        LinColar = 0x0000C000;
                    else
                        LinColar = 0x0000C0C4;
                }
                else
                {
                    if(MotionMaskArea[y][x] == 0)
                        LinColar = 0xC4C4C000;
                    else
                        LinColar = 0xC4C4C0C4;
                }
                break;

            case 2:
                if(MotionMaskArea[y][x+1] == 0)
                {
                    if(MotionMaskArea[y][x] == 0)
                        LinColar = 0x00C00000;
                    else
                        LinColar = 0x00C0C4C4;
                }
                else
                {
                    if(MotionMaskArea[y][x] == 0)
                        LinColar = 0xC4C00000;
                    else
                        LinColar = 0xC4C0C4C4;
                }
                break;

            case 3:
                if(MotionMaskArea[y][x] == 0)
                    LinColar = 0xC0000000;
                else
                    LinColar = 0xC0C4C4C4;
                break;
        }
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), drawX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);

    }

    return;
}
void uiOsdDrawMaskAreaCurrent(void)
{
    u8  x, y, space_x, space_y;
    u8  maskMaxColNum = MASKAREA_MAX_COLUMN, maskMaxRowNum = MASKAREA_MAX_ROW;
    u8  OSD_BLK[2] = {IDU_OSD_L1_WINDOW_0, IDU_OSD_L0_WINDOW_0};
    u16 drawX, drawW, drawY;
    u32 LinColar;

    DEBUG_UI("MotionMaskCursor %d\r\n",MotionMaskCursor);
    space_x = OSDIconEndX/maskMaxColNum;
    space_y = OSDIconEndY/maskMaxRowNum;
    if (MotionMaskCursor < 0)
        MotionMaskCursor += (maskMaxColNum*maskMaxRowNum);
    else if (MotionMaskCursor >= (maskMaxColNum*maskMaxRowNum))
        MotionMaskCursor -= (maskMaxColNum*maskMaxRowNum);

    x = MotionMaskCursor%maskMaxColNum;
    y = MotionMaskCursor/maskMaxColNum;

    drawX = x*space_x;
    drawY = y*space_y+1;
    /*draw left part*/
    switch(drawX&0x0003)
    {
        case 0:
            LinColar = 0xC6C6C6C0;
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), drawX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
            drawX = drawX+4;
            drawW = space_x-4;
            break;

        case 1:
            if((x>0)&&(MotionMaskArea[y][x-1]) != 0)
                LinColar = 0xC6C6C0C4;
            else
                LinColar = 0xC6C6C000;
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), (drawX-1), drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
            drawX = drawX+3;
            drawW = space_x-3;
            break;

        case 2:
            if((x>0)&&(MotionMaskArea[y][x-1]) != 0)
                LinColar = 0xC6C0C4C4;
            else
                LinColar = 0xC6C00000;
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), (drawX-2), drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
            drawX = drawX+2;
            drawW = space_x-2;
            break;

        case 3:
            drawX = drawX+1;
            drawW = space_x-1;
            break;
    }
    LinColar = 0xC6C6C6C6;
    if( x == (maskMaxColNum-1))
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], ((OSDDispWidth[sysTVOutOnFlag]-drawX)&0xFFFC), (space_y-1), drawX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
    else
    {
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], (drawW&0xFFFC), (space_y-1), drawX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
        drawX += (drawW&0xFFFC);
        /*draw right part*/
        drawW = drawW&0x0003;
        switch(drawW)
        {
            case 0:
                return;

            case 1:
                if(MotionMaskArea[y][x+1] == 0)
                    LinColar = 0x0000C0C6;
                else
                    LinColar = 0xC4C4C0C6;
                break;

            case 2:
                if(MotionMaskArea[y][x+1] == 0)
                    LinColar = 0x00C0C6C6;
                else
                    LinColar = 0xC4C0C6C6;
                break;

            case 3:
                LinColar = 0xC0C6C6C6;
                break;
        }
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), drawX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
    }
}
#endif

void uiOsdDrawAtenna(u8 signal)
{
 
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_ATENNA_LOC_X, UI_OSD_ATENNA_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_ATENNA_LOC_Y, UI_OSD_ATENNA_LOC_TV_Y };
    u8  bgColor=0x00;
    #if ((UI_PREVIEW_OSD == 0) )
        return;
    #endif

    if(MyHandler.MenuMode != VIDEO_MODE)
        return;
    
    if(uiIsRFBroken == 1)
    {
        bgColor=0xC1;
    }
    else if(uiIsRFBroken == 0)
    {
        bgColor=0x00;
    }

    #if (OSD_SIZE_X2_DISABLE == 0)
   	uiOSDIconColorByXY(OSD_ICON_ATENNA_Signal_0+signal, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], bgColor, alpha_3);
    #else
   	uiOSDIconColorByXY(OSD_ICON_ATENNA_Signal_0+signal, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], bgColor, alpha_3);
    #endif

}


void uiLedDrawAtenna(u8 signal)
{

    u8  pin=0x01;

    if(MyHandler.MenuMode != VIDEO_MODE)
        return;

#if (HW_BOARD_OPTION == MR8200_RX_GCT_LCD)
    i2cRead_WT6853(addr, &level);
    //DEBUG_UI("read regist:%x, data:%x  \n",addr,data);
    bitSet=0x1f;
    data &= ~bitSet;
    switch (signal)
    {
        case 0:
            data |= 0x1f;
            break;
        
        case 1:
            data |= 0xf;
            break;
        
        case 2:
            data |= 0x7;
            break;
        
        case 3:
            data |= 0x3;
            break;

        case 4:
            data |= 0x1;
            break;

        case 5:
            data |= 0;
            break;
            
        default:
            data |= 0;
            break;
    }
    i2cWrite_WT6853(addr, data);
    //DEBUG_UI("wirte regist:%x, data:%x  \n",addr,data);
    //i2cRead_WT6853(addr, &data);
    //DEBUG_UI("read regist:%x, data:%x  \n",addr,data);
#endif
}

void osdDrawMenuPreview(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 space;
    //u32 color = 0Xdcdcdcdc;
    u32 color = 0X9c9c9c9c;
#if (UI_PREVIEW_OSD == 0)
        return;
#endif

    UI_MULT_ICON *iconInfo;

    uiOSDIconColorByXY(OSD_ICON_MENU_TOP,UI_OSD_MENU_BAR_X, UI_OSD_MENU_BAR_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_2);    
    uiOsdGetIconInfo(OSD_ICON_MENU_TOP,&iconInfo);
    space = UI_OSD_MENU_BAR_Y+iconInfo->Icon_h;
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, 73, UI_OSD_MENU_BAR_X, space, OSD_BLK[sysTVOutOnFlag], color);
    space += 73;
    uiOSDIconColorByXY(OSD_ICON_MENU_MID,UI_OSD_MENU_BAR_X, space, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_2);
    uiOsdGetIconInfo(OSD_ICON_MENU_MID,&iconInfo);
    space += iconInfo->Icon_h;
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, 79, UI_OSD_MENU_BAR_X, space, OSD_BLK[sysTVOutOnFlag], color);
    space += 79;
    uiOSDIconColorByXY(OSD_ICON_MENU_BOTTOM,UI_OSD_MENU_BAR_X, space, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_2);

}

void osdDrawLCDPreview(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 space;
    
#if (UI_PREVIEW_OSD == 0)
        return;
#endif

    UI_MULT_ICON *iconInfo;

    uiOSDIconColorByXY(OSD_ICON_LCD_BL,UI_OSD_LCD_X, UI_OSD_LCD_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);    

}

void osdDrawMotionPreview(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 drawY;
    UI_MULT_ICON *iconInfo;
    
#if (UI_PREVIEW_OSD == 0)
        return;
#endif
    
    if (MyHandler.MenuMode != VIDEO_MODE)
    {
        return;
    }

    drawY = UI_PREVIEW_TIME_Y - 40;

    if (act == UI_OSD_DRAW)
    {
        uiOSDIconColorByXYChColor(OSD_ICON_MOTION_CLOSE,UI_OSD_MOTION_X, UI_OSD_MOTION_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3, 0xe8, 0xc2);    
        showPIRMsg = 0;
        uiOSDASCIIStringByColorY("PIR will be canceled", drawY, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
    }
    else if (act == UI_OSD_CLEAR)
    {
        uiOSDIconColorByXY(OSD_ICON_MOTION_CLOSE,UI_OSD_MOTION_X, UI_OSD_MOTION_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);    
        showPIRMsg = 7;
    }
    
}

void osdDrawMotionMsg(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    UI_MULT_ICON *iconInfo;
    u16 drawY;
    
#if (UI_PREVIEW_OSD == 0)
        return;
#endif
    drawY = UI_PREVIEW_TIME_Y - 40;
    
    if (act == UI_OSD_DRAW)
    {
        uiOSDASCIIStringByColorY("PIR will be canceled", drawY, OSD_BLK[sysTVOutOnFlag], 0xc2, 0x00);
    }
    else if(act == UI_OSD_CLEAR)
    {
        uiOSDASCIIStringByColorY("PIR will be canceled", drawY, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
    }
    
}

void osdDrawRecPreview(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 space;
    u16 drawX, drawY;
    
#if (UI_PREVIEW_OSD == 0)
        return;
#endif

    if (MyHandler.MenuMode == VIDEO_MODE)
    {
        drawX = UI_OSD_REC_BUTTON_X;
        drawY = UI_OSD_REC_BUTTON_Y;
    }
    else
    {
        drawX = UI_OSD_MOTION_X;
        drawY = UI_OSD_MOTION_Y;
    }
    
    if (act == UI_OSD_DRAW)
        uiOSDIconColorByXYChColor(OSD_ICON_REC_BUTTON, drawX, drawY, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3, 0xe8, 0xc2);    
    else
        uiOSDIconColorByXY(OSD_ICON_REC_BUTTON, drawX, drawY, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);    
}

void osdDrawQuadPreview(void)
{
u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
#if (UI_PREVIEW_OSD == 0)
    return;
#endif

    uiOSDIconColorByXY(OSD_ICON_QUAD ,UI_OSD_QUAD_LOC_X, UI_OSD_QUAD_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);

}

void uiOsdDrawSensor(u8 Camid, u8 mode)
{
    char camera[5];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    UI_MULT_ICON *iconInfo;
    u16 DrawX, DrawY;
    u8 i;
    char sensor_name[20];

    if (mode == VIDEO_MODE)
    {
        DrawX = UI_OSD_CAM_LOC_X;
        DrawY = UI_OSD_CAM_LOC_Y;
    }
    else
    {
        if (uiQuadDisplay == 1)
        {
            if ((Camid%2) == 0)
                DrawX = UI_OSD_CAM1_QUAD_LOC_X;
            else
                DrawX = (UI_OSD_CAM1_QUAD_LOC_X+OSDIconMidX);
            if (Camid < 2)
                DrawY = UI_OSD_CAM1_QUAD_LOC_Y;
            else
                DrawY = (UI_OSD_CAM1_QUAD_LOC_Y+OSDIconMidY);
        }
        else
        {
            if (iconflag[UI_MENU_SETIDX_CH1_ON+Camid] == UI_MENU_SETTING_CAMERA_OFF)
                return;
            DrawY = UI_OSD_CAM1_QUAD_LOC_Y;
            for (i = 0; i < 4; i++)
            {
                if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
                {
                    if (i == Camid) /*left*/
                        DrawX = UI_OSD_CAM1_QUAD_LOC_X;
                    else    /*right*/
                        DrawX = UI_OSD_CAM1_QUAD_LOC_X+OSDIconMidX;
                    break;                        
                }
            }
        }
    }
    sprintf(camera,"CAM%d",Camid+1);
    if(sysTVOutOnFlag == 0)
        uiOSDASCIIStringByColor(camera, DrawX, DrawY,  OSD_BLK[sysTVOutOnFlag], 0xc4, 0x00);
    else
        uiOSDASCIIStringByColor(camera, DrawX, DrawY,  OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    if (mode == VIDEO_MODE)
    {
        if (Camid == 0)
        {
            sprintf(sensor_name,"Opening sensor 1",Camid+1);
        }
        else if (Camid == 1)
        {
            sprintf(sensor_name,"Flood sensor 1",Camid+1);
        }
        else if (Camid == 2)
        {
            sprintf(sensor_name,"Shock sensor 1",Camid+1);
        }
        else if (Camid == 3)
        {
            sprintf(sensor_name,"Pir sensor 1",Camid+1);
        }
        uiOSDASCIIStringByColor(sensor_name, DrawX+50, DrawY,  OSD_BLK[sysTVOutOnFlag], 0xc4, 0x00);
    }

}


void uiOsdDrawCamera(u8 Camid, u8 mode, u8 act)
{
    char camera[5];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_CAM_LOC_X,UI_OSD_CAM_LOC_TV_X  };
    u16 LOC_Y [2] = {UI_OSD_CAM_LOC_Y,UI_OSD_CAM_LOC_TV_Y };
    u16 QUAD_LOC_X [2] = {UI_OSD_CAM1_QUAD_LOC_X,UI_OSD_CAM1_QUAD_LOC_TV_X };
    u16 QUAD_LOC_Y [2] = {UI_OSD_CAM1_QUAD_LOC_Y,UI_OSD_CAM1_QUAD_LOC_TV_Y };
    u16 DrawX, DrawY;
    u8 i;

    if (mode == VIDEO_MODE)
    {
        DrawX = LOC_X[sysTVOutOnFlag];
        DrawY = LOC_Y[sysTVOutOnFlag];
    }
    else
    {
        if (uiQuadDisplay == 1)   //quad
        {
            if ((Camid%2) == 0)
                DrawX = QUAD_LOC_X[sysTVOutOnFlag];
            else
                DrawX = (QUAD_LOC_X[sysTVOutOnFlag]+OSDIconMidX);
            if (Camid < 2)
                DrawY = QUAD_LOC_Y[sysTVOutOnFlag];
            else
                DrawY = (QUAD_LOC_Y[sysTVOutOnFlag]+OSDIconMidY);
            
        }
        else
        {
            if (iconflag[UI_MENU_SETIDX_CH1_ON+Camid] == UI_MENU_SETTING_CAMERA_OFF)
                return;
            
            DrawY = QUAD_LOC_Y[sysTVOutOnFlag];
            for (i = 0; i < 4; i++)
            {
                if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
                {
                    if (i == Camid) /*left*/
                        DrawX = QUAD_LOC_X[sysTVOutOnFlag];
                    else    /*right*/
                        DrawX = QUAD_LOC_X[sysTVOutOnFlag]+OSDIconMidX;
                    break;                        
                }
            }
            
        }
    }

    if (act == UI_OSD_DRAW)//blue
    {
        uiOSDIconColorByXY(OSD_ICON_CAM1 + Camid ,DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    }
    else
    {
        uiOSDIconColorByXYChColor(OSD_ICON_CAM1 + Camid ,DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3, 0xe8, 0xc2);
    }
    
}

#if 1
void osdDrawVideoOn(u8 on)
{
    u8 OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_REC_LOC_X,UI_OSD_REC_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_REC_LOC_Y,UI_OSD_REC_LOC_TV_Y };

    if(MyHandler.MenuMode!=VIDEO_MODE)
        return;
    
    switch(on)
    {
        case UI_OSD_NONE:
        case UI_OSD_CLEAR:
            uiOsdDrawCamera(sysRFRxInMainCHsel, VIDEO_MODE, UI_OSD_DRAW);
            break;

        case UI_OSD_DRAW:
            uiOsdDrawCamera(sysRFRxInMainCHsel, VIDEO_MODE, UI_OSD_CLEAR);
            break;

    }
}

void osdDrawQuadVideoOn(u8 Camid, u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_REC_QUAD_LOC_X,UI_OSD_REC_QUAD_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_REC_QUAD_LOC_Y,UI_OSD_REC_QUAD_LOC_TV_Y };
    UI_MULT_ICON *iconInfo;
    u16 DrawX, DrawY;
    u8  i;

    uiOsdGetIconInfo(OSD_ICON_VREC,&iconInfo);

    if (uiQuadDisplay == 1)
    {
        if ((Camid%2) == 0)
            DrawX = LOC_X[sysTVOutOnFlag];
        else
            DrawX = (LOC_X[sysTVOutOnFlag]+OSDIconMidX);
        if (Camid < 2)
            DrawY = LOC_Y[sysTVOutOnFlag];
        else
            DrawY = (LOC_Y[sysTVOutOnFlag]+OSDIconMidY);
    }
    else
    {
        if (iconflag[UI_MENU_SETIDX_CH1_ON+Camid] == UI_MENU_SETTING_CAMERA_OFF)
            return;
        DrawY = LOC_Y[sysTVOutOnFlag];
        for (i = 0; i < 4; i++)
        {
            if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
            {
                if (i == Camid) /*left*/
                    DrawX = LOC_X[sysTVOutOnFlag];
                else    /*right*/
                    DrawX = LOC_X[sysTVOutOnFlag]+OSDIconMidX;
                break;                        
            }
        }
    }
    
    if (act == UI_OSD_DRAW)
    {
        uiOsdDrawCamera(Camid, QUAD_MODE, UI_OSD_CLEAR);
    }
    else
    {
        uiOsdDrawCamera(Camid, QUAD_MODE, UI_OSD_DRAW);
    }
    
    
}

#else
void osdDrawVideoOn(u8 on)
{
    u8 OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_REC_LOC_X,UI_OSD_REC_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_REC_LOC_Y,UI_OSD_REC_LOC_TV_Y };

    if(MyHandler.MenuMode!=VIDEO_MODE)
        return;
    
    switch(on)
    {
        case UI_OSD_NONE:
        case UI_OSD_CLEAR:
            #if (UI_PREVIEW_OSD == 1)
                #if (USE_BIG_OSD == 1)
                    uiOSDIconColorByXY(OSD_ICON_VREC2 ,LOC_X[sysTVOutOnFlag] , LOC_Y[sysTVOutOnFlag] , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
                    //osdDrawMessage(MSG_REC, LOC_X[sysTVOutOnFlag]+20, LOC_Y[sysTVOutOnFlag]+3, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
                #else
                    //uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 92 , 24 , 8 , 180+osdYShift , OSD_BLK[sysTVOutOnFlag] , 0);
                    uiOSDIconColorByXY(OSD_ICON_VREC2 ,8 , 180+osdYShift , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
                    //osdDrawMessage(MSG_REC, 28, 180+osdYShift, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
                #endif
            #endif
            break;

        case UI_OSD_DRAW:
            #if (UI_PREVIEW_OSD == 1)
                #if (USE_BIG_OSD == 1)
                    uiOSDIconColorByXY(OSD_ICON_VREC ,LOC_X[sysTVOutOnFlag] , LOC_Y[sysTVOutOnFlag] , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
                    //osdDrawMessage(MSG_REC, LOC_X[sysTVOutOnFlag]+20, LOC_Y[sysTVOutOnFlag]+3, OSD_BLK[sysTVOutOnFlag], 0xC2, 0x00);
                #else
                    uiOSDIconColorByXY(OSD_ICON_VREC ,8 , 180+osdYShift , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
                    //osdDrawMessage(MSG_REC, 28, 180+osdYShift, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
                #endif
            #endif
            break;

    }
}

void osdDrawQuadVideoOn(u8 Camid, u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_REC_QUAD_LOC_X,UI_OSD_REC_QUAD_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_REC_QUAD_LOC_Y,UI_OSD_REC_QUAD_LOC_TV_Y };
    UI_MULT_ICON *iconInfo;
    u16 DrawX, DrawY;
    u8  i;

    uiOsdGetIconInfo(OSD_ICON_VREC,&iconInfo);

    if (uiQuadDisplay == 1)
    {
        if ((Camid%2) == 0)
            DrawX = LOC_X[sysTVOutOnFlag];
        else
            DrawX = (LOC_X[sysTVOutOnFlag]+OSDIconMidX);
        if (Camid < 2)
            DrawY = LOC_Y[sysTVOutOnFlag];
        else
            DrawY = (LOC_Y[sysTVOutOnFlag]+OSDIconMidY);
    }
    else
    {
        if (iconflag[UI_MENU_SETIDX_CH1_ON+Camid] == UI_MENU_SETTING_CAMERA_OFF)
            return;
        DrawY = LOC_Y[sysTVOutOnFlag];
        for (i = 0; i < 4; i++)
        {
            if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
            {
                if (i == Camid) /*left*/
                    DrawX = LOC_X[sysTVOutOnFlag];
                else    /*right*/
                    DrawX = LOC_X[sysTVOutOnFlag]+OSDIconMidX;
                break;                        
            }
        }
    }
    
    if (act == UI_OSD_DRAW)
    {
        uiOSDIconColorByXY(OSD_ICON_VREC ,DrawX , DrawY , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        //osdDrawMessage(MSG_REC, DrawX+iconInfo->Icon_w, DrawY+3, OSD_BLK[sysTVOutOnFlag], 0xC2, 0x00);
    }
    else
    {
        //uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], iconInfo->Icon_w, iconInfo->Icon_h, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00);
        uiOSDIconColorByXY(OSD_ICON_VREC2 ,DrawX , DrawY , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        //osdDrawMessage(MSG_REC, DrawX+iconInfo->Icon_w, DrawY+3, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
    }
    
    
}

#endif

#if(UI_BAT_SUPPORT)
void uiOsdDrawCameraBatteryLevel(u8 camID,u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 singleX=490,singleY=280;
    u16 quadX[2]={236,236};
    u16 quadY[2]={128,128};
    u16 i,DrawX,DrawY;
    u32 batLevel=gRfiuUnitCntl[camID].RFpara.TxBatteryLev;
      //DEBUG_YELLOW("%d %s %s TxBatteryLev %d %d\n",__LINE__, __FILE__,__FUNCTION__,camID,batLevel);

    if (gRfiuUnitCntl[camID].WakeUpTxEn == 1)
        return;
        
    if(!((MyHandler.MenuMode == VIDEO_MODE ) || (MyHandler.MenuMode == QUAD_MODE )))
        return;

    if (_uiBatType[camID] == UI_CAM_NORMAL)
        return;

    if ((MyHandler.MenuMode == VIDEO_MODE ) && (camID != sysRFRxInMainCHsel))
        return;
    
    if (act==UI_OSD_DRAW)
    {
        if (uiQuadDisplay == 2)
        {
            if (SysCamOnOffFlag & (0x01<<camID))
                H264_Decode_One_I_frame(camID);
        }
        else
            H264_Decode_One_I_frame(camID);      
    }

    if (batLevel == RF_BATCAM_TXBATSTAT_NOSHOW)
        batLevel = 0;
    else if (batLevel >= UI_BATTERY_LV4)
        batLevel = 3;

    if (MyHandler.MenuMode==VIDEO_MODE)
    {
        if (act==UI_OSD_DRAW)
            uiOSDIconColorByXY(OSD_ICON_BATCAM_BATTERY_LV0+batLevel, singleX, singleY, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
        else
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],44,44,singleX,singleY, OSD_BLK[sysTVOutOnFlag],0x3f3f3f3f);
    }
    else
    {
        if (uiQuadDisplay == 1)
        {
            if ((camID%2) == 0)
                DrawX = quadX[sysTVOutOnFlag];
            else
                DrawX = (quadX[sysTVOutOnFlag]+OSDIconMidX);
            if (camID < 2)
                DrawY = quadY[sysTVOutOnFlag];
            else
                DrawY = (quadY[sysTVOutOnFlag]+OSDIconMidY);
        }
        else
        {
            if (iconflag[UI_MENU_SETIDX_CH1_ON+camID] == UI_MENU_SETTING_CAMERA_OFF)
                return;
            
            DrawY = singleY-20;
            for (i = 0; i < 4; i++)
            {
                if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
                {
                    if (i == camID) /*left*/
                        DrawX = quadX[sysTVOutOnFlag];
                    else    /*right*/
                        DrawX = quadX[sysTVOutOnFlag]+OSDIconMidX;
                    break;                        
                }
            }
        }
        if (act==UI_OSD_DRAW)
            uiOSDIconColorByXY(OSD_ICON_BATCAM_BATTERY_LV0+batLevel ,DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        else
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],44,44,DrawX,DrawY, OSD_BLK[sysTVOutOnFlag],0x3f3f3f3f);
    }  
    
}

void osdDrawCamLiveView(u8 camID)
{    
    if(!((MyHandler.MenuMode == VIDEO_MODE ) || (MyHandler.MenuMode == QUAD_MODE )))
        return;
    
    if (_uiBatType[camID] == UI_CAM_BATTERY)
    {        
        if (iconflag[UI_MENU_SETIDX_CH1_ON+camID] == UI_MENU_SETTING_CAMERA_OFF)
        {
            uiOsdDrawCameraBatteryLevel(camID,UI_OSD_DRAW);          
        }        
        else if ((uiShowOSDTime[camID]==0) && (uiRFStatue[camID] == UI_RF_STATUS_NO_SINGLE))
        {
            uiOsdDrawCameraBatteryLevel(camID,UI_OSD_DRAW);          
        }
        else
        {
            uiOsdDrawCameraBatteryLevel(camID,UI_OSD_CLEAR);          
        }
    }
}

#endif

void uiOsdDrawNoSignal(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};

    //memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM);
    
    if (act == UI_OSD_DRAW)
    {
        #if(UI_BAT_SUPPORT)
        if (_uiBatType[sysRFRxInMainCHsel]==UI_CAM_NORMAL)
            Idu_ClearBuf(DISPLAY_BUF_NUM);
        #endif
        
        DEBUG_UI("Camera %d No Signal\r\n",sysRFRxInMainCHsel);
        if(gRfiu_Op_Sta[sysRFRxInMainCHsel] == RFIU_RX_STA_LINK_OK)
        {
            DEBUG_UI("Camera %d is linking in uiOsdDrawNoSignal\r\n",sysRFRxInMainCHsel);
            return;
        }
        
        uiMenuOSDReset();
        (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
        uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
        //uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],OSDDispWidth[sysTVOutOnFlag],OSDDispHeight[sysTVOutOnFlag],0,0, OSD_BLK[sysTVOutOnFlag],0xC1C1C1C1);
        //osdDrawMessage(MSG_NO_SIGNAL, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
        uiOsdDrawAtenna(0);
        osdDrawPreviewIcon();
    }
    else
    {
        uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
        osdDrawPreviewIcon();
    }
}


/*
 *********************************************************************************************************
 * Called by UI in other file
 *********************************************************************************************************
 */
 #if RFIU_SUPPORT
u8 uiOsdDrawPairInMenu(u8 Camera)
{
    u8  err;
    u32 waitFlag = 0, cnt = 30;
    u8  SecString[15];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};


    IduVideo_ClearPKBuf(0);
    iduSetVideoBuf0Addr(PKBuf0);
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    OSFlagPost(gRfiuStateFlagGrp, (RFIU_RX_STA_PAIR_OK<<(Camera*8)), OS_FLAG_CLR, &err);
    rfiu_PAIR_Linit(Camera);
    //uiOSDASCIIStringByColor("Please Press Pair Key On", 40, 100, OSD_Blk1, 0xc0, 0x00);
    //uiOSDASCIIStringByColor("Camera Side 30", 40, 120, OSD_Blk1, 0xc0, 0x00);
    uiOSDASCIIStringByColorY("Please Press Pair Key On", 216, OSD_BLK[sysTVOutOnFlag], 0xc0, 0x00);
    uiOSDASCIIStringByColorY("Camera Side 30", 244, OSD_BLK[sysTVOutOnFlag], 0xc0, 0x00);
    while (waitFlag == 0)
    {
        waitFlag = OSFlagPend(gRfiuStateFlagGrp, (RFIU_RX_STA_PAIR_OK<<(Camera*8)),(OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME), 20, &err);
        if (waitFlag & FLAGUI_RF_PAIR_SUCCESS)
        {
            DEBUG_UI("Wait PAIR Flag\r\n");
            uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
            return 1;

        }
        else if (cnt != 0)
        {
            cnt --;
            sprintf ((char *)SecString, "Camera Side %2d", cnt);
            //uiOSDASCIIStringByColor(SecString, 40+12*8, 120, OSD_Blk1, 0xc0, 0x00);
            uiOSDASCIIStringByColorY(SecString, 244, OSD_BLK[sysTVOutOnFlag], 0xc0, 0x00);
        }
        else
        {
            DEBUG_UI("Draw Pair Timeout\r\n");
            uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
            rfiu_PAIR_Stop(Camera);
            return 0;
        }

    }
    DEBUG_UI("Draw Pair Success\r\n");
    return 1;
}
#endif
void uiOsdDrawPlaybackPlaySpeed(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X[2] = {UI_OSD_ASCII_STRING_LOC_X ,UI_OSD_ASCII_STRING_LOC_TV_X};
    u16 LOC_Y[2] = {UI_OSD_ASCII_STRING_LOC_Y ,UI_OSD_ASCII_STRING_LOC_TV_Y};
    u16 draw_x;
    u32 strW ;

    strW = uiOsdGetFontWidth(UI_MULT_LANU_EN);
    draw_x = (LOC_X[sysTVOutOnFlag]+strW*3+4);

    if (curr_playback_speed > UI_PLAYBACK_SPEED_LEVEL/2)
        uiOSDIconColorByXY(OSD_ICON_FF ,draw_x, LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    else if(curr_playback_speed < UI_PLAYBACK_SPEED_LEVEL/2)
        uiOSDIconColorByXY(OSD_ICON_REW ,draw_x, LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    else
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 12 , 16 , draw_x , LOC_Y[sysTVOutOnFlag] , OSD_BLK[sysTVOutOnFlag] , 0);
    uiOSDASCIIStringByColor(Playback_speed[curr_playback_speed], LOC_X[sysTVOutOnFlag] , LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag] ,  0xC0, 0x41);
    if (curr_playback_speed == 5)
    uiOSDASCIIStringByColor(Playback_speed[curr_playback_speed], LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag] ,  0xC0, 0x00);
}

void uiOsdDrawMaskArea(u8 key) //以後用uiOsdDrawMaskAreaSetting取代
{
    u8  space_x, space_y;
    u8  OSD_BLK[2] = {IDU_OSD_L1_WINDOW_0, IDU_OSD_L0_WINDOW_0};
    u8  maskMaxColNum = MASKAREA_MAX_COLUMN, maskMaxRowNum;
    u32 LinColar;
    u16 i, j, drawX, tmpX, drawY, drawW;
#if 0
    switch(key)
    {
        case 0: /*first enter mask area*/
            maskMaxRowNum = MASKAREA_MAX_ROW;
            playbackflag = 0;
            uiMenuEnable=0;
            uiMenuOSDReset();
            IduVideo_ClearPKBuf(0);
            iduSetVideoBuf0Addr(PKBuf0);
            #if RFIU_SUPPORT
                uiSetRfDisplayMode(UI_MENU_RF_FULL);
            #endif
            uiMenuEnterPreview(0);
            
            
            (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
            uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
            
            space_x = OSDIconEndX/maskMaxColNum;
            space_y = OSDIconEndY/maskMaxRowNum;

            /*draw current select setting and draw row and column*/
            drawY = 1;
            for (i = 0; i < maskMaxRowNum; i++)
            {
                drawX = 0;
                for (j = 0; j < maskMaxColNum; j++)
                {
                    /*draw left part*/
                    switch(drawX&0x0003)
                    {
                        case 0:
                            if (MotionMaskArea[i][j] != 0)
                                LinColar = 0xC4C4C4C0;
                            else
                                LinColar = 0x000000C0;
                            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), drawX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
                            tmpX = drawX+4;
                            drawW = space_x-4;
                            break;

                        case 1:
                            if((j>0)&&(MotionMaskArea[i][j-1]) != 0)
                                LinColar = 0x0000C0C4;
                            else
                                LinColar = 0x0000C000;
                            if (MotionMaskArea[i][j] != 0)
                                LinColar |= 0xC4C40000;
                            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), (drawX-1), drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
                            tmpX = drawX+3;
                            drawW = space_x-3;
                            break;

                        case 2:
                            if((j>0)&&(MotionMaskArea[i][j-1]) != 0)
                                LinColar = 0x00C0C4C4;
                            else
                                LinColar = 0x00C00000;
                            if (MotionMaskArea[i][j] != 0)
                                LinColar |= 0xC4000000;
                            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), (drawX-2), drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
                            tmpX = drawX+2;
                            drawW = space_x-2;
                            break;

                        case 3:
                            if((j>0)&&(MotionMaskArea[i][j-1]) != 0)
                                LinColar = 0xC0C4C4C4;
                            else
                                LinColar = 0xC0000000;
                            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, (space_y-1), (drawX-3), drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
                            tmpX = drawX+1;
                            drawW = space_x-1;
                            break;
                    }

                    if (MotionMaskArea[i][j] != 0)
                    {
                        LinColar = 0xC4C4C4C4;
                        if( j == (maskMaxColNum-1))
                            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], ((OSDDispWidth[sysTVOutOnFlag]-drawW)&0xFFFC), (space_y-1), tmpX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
                        else
                            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], (drawW&0xFFFC), (space_y-1), tmpX, drawY, OSD_BLK[sysTVOutOnFlag], LinColar);
                    }

                    drawX+=space_x;
                }
                uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], OSDDispWidth[sysTVOutOnFlag], 1, 0, (drawY-1), OSD_BLK[sysTVOutOnFlag], 0xC0C0C0C0);
                drawY+=space_y;
            }
            /*draw current cursor*/
            uiOsdDrawMaskAreaCurrent();
            break;

        case UI_KEY_UP:
            uiOsdDrawMaskAreaClean();
            MotionMaskCursor -= maskMaxColNum;
            uiOsdDrawMaskAreaCurrent();
            break;

        case UI_KEY_DOWN:
            uiOsdDrawMaskAreaClean();
            MotionMaskCursor += maskMaxColNum;
            uiOsdDrawMaskAreaCurrent();
            break;

        case UI_KEY_RIGHT:
            uiOsdDrawMaskAreaClean();
            MotionMaskCursor++;
            if (MotionMaskCursor%maskMaxColNum == 0)
                MotionMaskCursor -= maskMaxColNum;
            uiOsdDrawMaskAreaCurrent();
            break;

        case UI_KEY_LEFT:
            uiOsdDrawMaskAreaClean();
            if (MotionMaskCursor%maskMaxColNum == 0)
                MotionMaskCursor += maskMaxColNum;
            MotionMaskCursor--;
            uiOsdDrawMaskAreaCurrent();
            break;

        case UI_KEY_ENTER:
            i = MotionMaskCursor%maskMaxColNum;
            j = MotionMaskCursor/maskMaxColNum;
            MotionMaskArea[j][i] = (MotionMaskArea[j][i] == 1) ? 0 : 1;
            break;

        case UI_KEY_MODE:
            uiOsdDisable(OSD_BLK[sysTVOutOnFlag]);
            break;

        default:
            DEBUG_UI("osdDrawMaskArea error key %d\r\n",key);
            return;
    }
    #endif
    return;
}

void uiOsdDrawCardInfo(u8 on)
{
    FS_DISKFREE_T *diskInfo;
    u8  tmp_str[10];
    u32 remain;
    u16  x_pos = 482, y_pos[3]={160,240,318};
    u8 color;
    u32 total=0;
    
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    
    color=0xC1;
    if(on == UI_OSD_DRAW)
    {
        if((sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)&&(got_disk_info!= 0))
        {
            diskInfo=&global_diskInfo;
            total = diskInfo->total_clusters*diskInfo->sectors_per_cluster/(1024*2);
            //DEBUG_YELLOW("%d %s %d %d %d\n",__LINE__, __FILE__,total,diskInfo->total_clusters,diskInfo->sectors_per_cluster);
            sprintf((char*)tmp_str, "%d.%d",total/1000,(total-((total/1000)*1000))/100);  /*1MB =2048 sector*/
            uiOSDASCIIStringByColor(tmp_str, x_pos , y_pos[0], OSD_BLK[sysTVOutOnFlag], color, 0x00);
            remain = diskInfo->avail_clusters*1000/diskInfo->total_clusters;
            if (remain > 1000) remain = 1000;
            sprintf((char*)tmp_str, "%d.%d%%",(1000-remain)/10, (1000-remain)%10);
            uiOSDASCIIStringByColor(tmp_str, x_pos, y_pos[1], OSD_BLK[sysTVOutOnFlag], color, 0x00);
            sprintf((char*)tmp_str, "%d.%d%%",remain/10, remain%10);
            uiOSDASCIIStringByColor(tmp_str, x_pos, y_pos[2], OSD_BLK[sysTVOutOnFlag], color, 0x00);
        }
        else
        {
            uiOSDASCIIStringByColor("0", x_pos, y_pos[0], OSD_BLK[sysTVOutOnFlag], color, 0x00);
            uiOSDASCIIStringByColor("0%",  x_pos, y_pos[1], OSD_BLK[sysTVOutOnFlag], color, 0x00);
            uiOSDASCIIStringByColor("0%",  x_pos, y_pos[2], OSD_BLK[sysTVOutOnFlag], color, 0x00);
        }
    }
    else
    {
         uiOsdDisable(OSD_BLK[sysTVOutOnFlag]); 
    }
}

/*osd not ready*/

void uiOsdDrawFWVersion(u8 on)
{ 
    u16  LOC_X[2] = {UI_OSD_VERSION_LOC_X, UI_OSD_VERSION_LOC_TV_X};
    u16  y_pos = UI_OSD_VERSION_LOC_Y;
    u16  tx_x_pos[4] = {UI_OSD_VERSION_LOC_X, 644, UI_OSD_VERSION_LOC_X, 644};
    u16  tx_y_pos[4] = {174, 174, 288, 288};
    u16  x,y;
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u8  Version[30];
    u8  i,len=0;
    u8  tmpStr[32];
    
    if(on == UI_OSD_DRAW)
    {
        (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
        uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
        uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
        
        len=strlen(uiVersion);
        
        sprintf((char*)Version, "%s",uiVersion);
        uiOSDASCIIStringByColor(Version, LOC_X[sysTVOutOnFlag], y_pos, OSD_BLK[sysTVOutOnFlag], 0xC1, 0x00);

        for (i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            len = strlen(gRfiuUnitCntl[i].RFpara.TxCodeVersion);
            if (len > 0)
            {
                sprintf((char*)tmpStr,"%s",gRfiuUnitCntl[i].RFpara.TxCodeVersion);
                uiOSDASCIIStringByColor(tmpStr, tx_x_pos[i], tx_y_pos[i], OSD_BLK[sysTVOutOnFlag], 0xC1, 0x00);
            }
            else
                uiOSDASCIIStringByColor((u8*)gRfiuUnitCntl[i].RFpara.TxCodeVersion, tx_x_pos[i], tx_y_pos[i], OSD_BLK[sysTVOutOnFlag], 0xC1, 0x00);
            memset(&tmpStr, 0, sizeof(tmpStr));            
        }
        
    }
    else
    {
         uiOsdDisable(OSD_BLK[sysTVOutOnFlag]); 
    }
}

void uiOsdDrawNetwork(u8 key)
{

    switch(key)
    {
        case 0:
            break;
        case UI_KEY_MODE:   /*leave*/
            uiCurrNode = uiCurrNode->parent;
            MyHandler.MenuMode = SETUP_MODE;
            uiOsdDisable(OSD_BLK[sysTVOutOnFlag]);
            uiGraphDrawMenu();
            break;

        default:
            DEBUG_UI("uiOsdDrawNetwork error key %d\r\n",key);
            return;
    }
}

void uiOsdDrawRestoreDefaltSettings(u8 on)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    if(on == 1)
    {
        IduVideo_ClearPKBuf(0);
        (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
        uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
        uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
        osdDrawMessage(MSG_RESTORE_DEFAULT_SETTINGS, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);    
    }
    else
    {
         uiOsdDisable(OSD_BLK[sysTVOutOnFlag]); 
    }
}

void uiCheckTimerWrong(void)
{
    RTC_DATE_TIME   localTime;
    static u8   TimeCheckHour = 99, TimeCheckMin = 99, TimeCheckSec = 99, TimeClickCnt = 0; /* <---Variable: Reboot RX when an timer exception occurs */

    RTC_Get_Time(&localTime);

    /* Reboot RX when an timer exception occurs */
    if ((TimeCheckHour != localTime.hour) || (TimeCheckHour != localTime.min) || (TimeCheckHour != localTime.sec))
    {
        TimeCheckHour = localTime.hour;
        TimeCheckMin = localTime.min;
        TimeCheckSec = localTime.sec;
        TimeClickCnt = 0;
    }
    else
    {
        TimeClickCnt++;
        if (TimeClickCnt == 5)
        {
            DEBUG_UI("*** Unusual Time Click Reboot!! ***\n\r");
            TimeClickCnt = 0;
			sysForceWDTtoReboot();
        }
    }

}

void uiOsdDrawLifeTimePerSec(void)
{
    RTC_DATE_TIME   localTime;
    u8  DateTime[40];   /*YYYY/MM/DD  HH:mm:ss CHX*/
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_PREVIEW_TIME_X, UI_PREVIEW_TIME_TV_X };
    u16 LOC_Y [2] = {UI_PREVIEW_TIME_Y, UI_PREVIEW_TIME_TV_Y };

    uiCheckTimerWrong();
    
    if(showTime == 1)
    {
        RTC_Get_Time(&localTime);
        sprintf ((char *)DateTime, "20%02d/%02d/%02d %02d:%02d:%02d", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);
        DEBUG_GREEN("Current Time %s\r\n", DateTime);
    }
    
    #if(UI_BAT_SUPPORT)
    if ((MyHandler.MenuMode == VIDEO_MODE) && (_uiBatType[sysRFRxInMainCHsel]== UI_CAM_BATTERY))
        return;
    #endif
    
    if ((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE))
        return;

    if ((MyHandler.MenuMode == VIDEO_MODE) && (uiShowOSDTime[sysRFRxInMainCHsel] == 1))
        return;
    
    if ((MyHandler.MenuMode == VIDEO_MODE) || (MyHandler.MenuMode == QUAD_MODE) )
    {
        RTC_Get_Time(&localTime);
        #if (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_DMY) 
            sprintf ((char *)DateTime, "%02d/%02d/20%02d  %02d:%02d:%02d", localTime.day, localTime.month, localTime.year, localTime.hour, localTime.min, localTime.sec);
        #else
            sprintf ((char *)DateTime, "20%02d/%02d/%02d %02d:%02d:%02d", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);
        #endif
        uiOSDASCIIStringByColorY((u8*)DateTime, LOC_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag] , 0xe8, 0x00);
    }

}

#if 1
void uiOsdDrawRecPerSec(void)
{
    static u8 recOn = UI_OSD_DRAW;
    u32 waitFlag, relFlag;
    u8  err, i;
    
    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
    {
        osdDrawVideoOn(UI_OSD_CLEAR);
        recOn = UI_OSD_NONE;
        return;
    }

#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    if (MyHandler.MenuMode == VIDEO_MODE)
    {
        //waitFlag = (FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 << sysRFRxInMainCHsel);
        //if (OSFlagAccept(gRfRxVideoPackerSubReadyFlagGrp, waitFlag, OS_FLAG_WAIT_CLR_ANY, &err) >0)
        //DEBUG_YELLOW("%d %s %s %d %d\n",__LINE__, __FILE__,__FUNCTION__,sysRFRxInMainCHsel+MULTI_CHANNEL_LOCAL_MAX, MultiChannelGetCaptureVideoStatus(sysRFRxInMainCHsel+MULTI_CHANNEL_LOCAL_MAX));
        if ((MultiChannelGetCaptureVideoStatus(sysRFRxInMainCHsel+MULTI_CHANNEL_LOCAL_MAX) == 1)&&(gRfiu_Op_Sta[sysRFRxInMainCHsel]==RFIU_RX_STA_LINK_OK))
        {
            if (recOn == UI_OSD_NONE)
                recOn = UI_OSD_CLEAR;
            recOn^=1;
            osdDrawVideoOn(recOn);
        }
        else
        {
            if (recOn != UI_OSD_NONE)
            {
                osdDrawVideoOn(UI_OSD_CLEAR);
                recOn = UI_OSD_NONE;
            }
        }
    }
    else if (MyHandler.MenuMode == QUAD_MODE)
    {
        waitFlag = FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_ALL;
        relFlag = OSFlagAccept(gRfRxVideoPackerSubReadyFlagGrp, waitFlag, OS_FLAG_WAIT_CLR_ANY, &err);

        if (relFlag >0)
        {
            if (recOn == UI_OSD_NONE)
                recOn = UI_OSD_CLEAR;
            recOn^=1;
            for (i = 0; i < 4; i++)
            {
                //if (relFlag & (FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 << i))
                if ((MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == 1)&&(gRfiu_Op_Sta[i]==RFIU_RX_STA_LINK_OK))
                    osdDrawQuadVideoOn(i, recOn);
                else
                    osdDrawQuadVideoOn(i, UI_OSD_CLEAR);
            }
        }
        else
        {
            if (recOn != UI_OSD_NONE)
            {
                for (i = 0; i < 4; i++)
                {
                    osdDrawQuadVideoOn(i, UI_OSD_CLEAR);
                }
                recOn = UI_OSD_NONE;
            }
        }
    }
#endif
}
#else
void uiOsdDrawRecPerSec(void)
{
    static u8 recOn = UI_OSD_DRAW;
    u32 waitFlag, relFlag;
    u8  err, i;
    
    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
    {
        osdDrawVideoOn(UI_OSD_CLEAR);
        recOn = UI_OSD_NONE;
        return;
    }

#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    if (MyHandler.MenuMode == VIDEO_MODE)
    {
        //waitFlag = (FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 << sysRFRxInMainCHsel);
        //if (OSFlagAccept(gRfRxVideoPackerSubReadyFlagGrp, waitFlag, OS_FLAG_WAIT_CLR_ANY, &err) >0)
        //DEBUG_YELLOW("%d %s %s %d %d\n",__LINE__, __FILE__,__FUNCTION__,sysRFRxInMainCHsel+MULTI_CHANNEL_LOCAL_MAX, MultiChannelGetCaptureVideoStatus(sysRFRxInMainCHsel+MULTI_CHANNEL_LOCAL_MAX));
        if ((MultiChannelGetCaptureVideoStatus(sysRFRxInMainCHsel+MULTI_CHANNEL_LOCAL_MAX) == 1)&&(gRfiu_Op_Sta[sysRFRxInMainCHsel]==RFIU_RX_STA_LINK_OK))
        {
            if (recOn == UI_OSD_NONE)
                recOn = UI_OSD_CLEAR;
            recOn^=1;
            osdDrawVideoOn(recOn);
        }
        else
        {
            if (recOn != UI_OSD_NONE)
            {
                osdDrawVideoOn(UI_OSD_CLEAR);
                recOn = UI_OSD_NONE;
            }
        }
    }
    else if (MyHandler.MenuMode == QUAD_MODE)
    {
        waitFlag = FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_ALL;
        relFlag = OSFlagAccept(gRfRxVideoPackerSubReadyFlagGrp, waitFlag, OS_FLAG_WAIT_CLR_ANY, &err);

        if (relFlag >0)
        {
            if (recOn == UI_OSD_NONE)
                recOn = UI_OSD_CLEAR;
            recOn^=1;
            for (i = 0; i < 4; i++)
            {
                //if (relFlag & (FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 << i))
                if ((MultiChannelGetCaptureVideoStatus(i+MULTI_CHANNEL_LOCAL_MAX) == 1)&&(gRfiu_Op_Sta[i]==RFIU_RX_STA_LINK_OK))
                    osdDrawQuadVideoOn(i, recOn);
                else
                    osdDrawQuadVideoOn(i, UI_OSD_CLEAR);
            }
        }
        else
        {
            if (recOn != UI_OSD_NONE)
            {
                for (i = 0; i < 4; i++)
                {
                    osdDrawQuadVideoOn(i, UI_OSD_CLEAR);
                }
                recOn = UI_OSD_NONE;
            }
        }
    }
#endif
}
#endif



u8 uiOsdGetStrLibByLanguage(MSG_SRTIDX str_inx, u8 **lib, u8 *font_w, u8 *font_h, UI_MULT_LAN_STR **str_info, u8 language)
{
    u32  MsgNum, msgIdx;

    if(str_inx >= MSG_MAX_NUM)
    {
        DEBUG_UI("ui Get Library Fail by index %d\r\n",str_inx);
        return 0;
    }

    MsgNum = sizeof(UiMsgStr)/sizeof(UI_MSG_TAB);
    msgIdx = uiStrIconIdx[str_inx];
    if (msgIdx > MsgNum)
    {
        DEBUG_UI("This Str Index %d not use in this project\r\n",msgIdx);
        return 0;
    }

    if(msgIdx == 0xFF)
    {
        DEBUG_UI("ui Get Library has Error index %d\r\n", str_inx);
        return 0;
    }

    if(language >= UI_MULT_LANU_END)
    {
        DEBUG_UI("ui Get Library Fail by language %d\r\n",language);
        return 0;
    }
    if (str_inx != MSG_ASCII_STR)
        *str_info = &UiMsgStr[msgIdx].MsgStr[language];
    else
        *str_info = NULL;

    switch(language)
    {
        case UI_MULT_LANU_EN:
        default:
#if (USE_BIG_OSD == 1)
            *lib = &OSD_ASCII_16x20[0][0];
#else
            *lib = &OSD_ASCII_White[0][0];
#endif
            *font_w = uiOsdGetFontWidth(language);
            *font_h = uiOsdGetFontHeight(language);
            break;
    }
    return 1;
}

u8 uiOsdGetStrLib(MSG_SRTIDX str_inx, u8 **lib, u8 *font_w, u8 *font_h, UI_MULT_LAN_STR **str_info)
{
    return uiOsdGetStrLibByLanguage(str_inx, lib, font_w, font_h, str_info, CurrLanguage);
}

void uiOsdDrawPlayTime(u8 type, u32 time_unit)
{
    u8 h,m,s;
    char  timeForRecord[9];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_PLAYBACK_LOC_VIDEO_TIME_X, 416}; //Refer osdDrawVideoTime LOC_X+16
    u16 LOC_Y [2] = {UI_PLAYBACK_LOC_VIDEO_TIME_Y, 430}; //Refer osdDrawVideoTime LOC_Y

    LOC_Y[0] = OSDIconEndY-(uiOsdGetFontHeight(CurrLanguage)+8)-40;
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 16*sizeof("%02d:%02d:%02d/%02d:%02d:%02d") , 20, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag] , 0);
    if(type==1)
    {
        h = time_unit / 3600;
        m = (time_unit - h*3600) / 60;
        s = time_unit - h*3600 - m*60;

        sprintf (timeForRecord, "%02d:%02d:%02d", h, m, s);
        #if (USE_BIG_OSD == 1)
            uiOSDASCIIStringByColor((u8*)timeForRecord, LOC_X[sysTVOutOnFlag]+80 , LOC_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
        #else
            uiOSDASCIIStringByColor((u8*)timeForRecord, 208 , 7 , OSD_Blk0 , 0xc0, 0x00);
        #endif
    }
    else    /*clean time*/
    {
        #if (USE_BIG_OSD == 1)
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 16*sizeof("%02d:%02d:%02d/%02d:%02d:%02d") , 20, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag] , 0);
        #else
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 64 , 16 , 208 , 77 , OSD_Blk0 , 0);
        #endif
    }
}

void uiOsdVolumeControl(u8 mode,UI_VALUECTRL value)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u8  i;
    u16 draw_x, draw_y;
    u8  tempstring[4];
    u32 strW, strH;
    UI_MULT_ICON *iconInfo;

    if ((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != PLAYBACK_MODE))
        return;

    if(sysTVOutOnFlag==0)
        draw_y = UI_OSD_VOL_NUM_LOC_Y-140-20;//140 OSD_ICON_VOL_BAR HIGH
    else
        draw_y = UI_OSD_VOL_NUM_LOC_Y -10 + osdYShift;

    if(value == UI_VALUE_ADD)
    {
        if(sysVolumnControl == 5)
            sysVolumnControl = 5;
        else
            sysVolumnControl++;
    }
    else if(value == UI_VALUE_SUBTRACT)
    {
        if(sysVolumnControl == 0)
            sysVolumnControl = 0;
        else
            sysVolumnControl--;
    }
    else if(value == UI_VALUE_CLEAN)
    {
        DEBUG_GREEN("%d %s sysVolumnControl %d\n",__LINE__,__FILE__,sysVolumnControl);
        if (sysVolumnControl == 0)
            uiOSDIconColorByXY(OSD_ICON_MUTE ,UI_OSD_VOL_LOC_X, UI_OSD_VOL_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        else
            uiOSDIconColorByXY(OSD_ICON_VOL ,UI_OSD_VOL_LOC_X, UI_OSD_VOL_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 24, 140, UI_OSD_VOL_NUM_LOC_X, draw_y, OSD_BLK[sysTVOutOnFlag], 0);
        volumeflag = 0;
        return;
    }
    
    DEBUG_GREEN("%d %s sysVolumnControl %d\n",__LINE__,__FILE__,sysVolumnControl);
    if (sysVolumnControl == 0)
        uiOSDIconColorByXY(OSD_ICON_MUTE ,UI_OSD_VOL_LOC_X, UI_OSD_VOL_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    else
        uiOSDIconColorByXY(OSD_ICON_VOL ,UI_OSD_VOL_LOC_X, UI_OSD_VOL_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);

    uiOSDIconColorByXY(OSD_ICON_VOL_TOP ,UI_OSD_VOL_NUM_LOC_X, draw_y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    uiOsdGetIconInfo(OSD_ICON_VOL_TOP,&iconInfo);
    draw_y += iconInfo->Icon_h;
    uiOSDIconColorByXY(OSD_ICON_VOL0+sysVolumnControl ,UI_OSD_VOL_NUM_LOC_X, draw_y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    uiOsdGetIconInfo(OSD_ICON_VOL0+sysVolumnControl,&iconInfo);
    draw_y += iconInfo->Icon_h;
    uiOSDIconColorByXY(OSD_ICON_VOL_BOTTOM ,UI_OSD_VOL_NUM_LOC_X, draw_y, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);

    volumeflag = 1;
    iconflag[UI_MENU_SETIDX_VOLUME] = sysVolumnControl;
    uiMenuAction(UI_MENU_SETIDX_VOLUME);
    
}

void uiOsdDrawMenuData(void)
{
    u32 i;
    UI_MENU_NODE *draw_data;
    static UI_NODE_DATA  last_data;

    draw_data = uiCurrNode;

    if (draw_data->item.NodeData->IconData == NULL)
    {
        DEBUG_UI("Current node is not osd data\r\n");
        return;
    }

    for (i = 0; i < draw_data->item.NodeData->FileNum; i++)
    {
        if((MyHandler.WhichKey == UI_KEY_LEFT)||(MyHandler.WhichKey == UI_KEY_RIGHT))
        {
            if(draw_data->item.NodeData->IconData[i].IconIndex == last_data.IconData[i].IconIndex)
            {
                continue;
            }
        }

        DEBUG_UI(" icon index %d  x  %d , x %d\r\n",draw_data->item.NodeData->IconData[i].IconIndex,draw_data->item.NodeData->IconData[i].Location_x,draw_data->item.NodeData->IconData[i].Location_y);
        uiOSDIconColorByXY(draw_data->item.NodeData->IconData[i].IconIndex, draw_data->item.NodeData->IconData[i].Location_x,draw_data->item.NodeData->IconData[i].Location_y, OSD_BLK[sysTVOutOnFlag] ,0 , alpha_3);

    }
    memcpy(&last_data, draw_data->item.NodeData, sizeof(UI_NODE_DATA));
}

void uiOsdDrawZoomSet(u8 chanel, u8 key)
{

}

void uiOsdDrawImageSet(u8 chanel, u8 key)
{

}

void uiOsdDrawAtennaByTX(u8 signal,u8 camera)
{
    u8  OSDStr[32];
    u8  signalStr='\0';
    u8  camStr='\0';
    
    switch(camera)
    {
        case 0:
            break;
        case 1:
            camStr='*';
            break;
        case 2:
            camStr='+';
            break;
    }

    switch(signal)
    {
        case 0:
            signalStr='"';

            break;
        case 1:
            signalStr='#';

            break;
        case 2:
            signalStr='$';

            break;
        case 3:
            signalStr='%';

            break;
        case 4:
            signalStr='&';

            break;
    }

    sprintf ((char *)OSDStr, "       %c", signalStr);
    GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, OSDStr,
                            32,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                            1,1,1+8,0+4);
}

void uiOsdDrawCameraByTX(u8 camera)
{
    switch(camera)
    {
        case 1:
            GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, "*",
                            2,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                            4,2,4+2,1+2);
            break;
        case 2:
            GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, "+",
                            2,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                            4,2,4+2,1+2);
            break;
    }


}



#if (NIC_SUPPORT == 1)
void uiOsdDrawIPInfo(u8 Mode)
{
    u8  IpAddr[20];   /*IP :192.168.100.100*/


    #if (UI_PREVIEW_OSD == 0)
        return;
    #endif
    sprintf ((char *)IpAddr, "IP :%u.%u.%u.%u", my_ipaddr[0], my_ipaddr[1], my_ipaddr[2], my_ipaddr[3]);
    if (Mode == VIDEO_MODE)
    {
        uiOSDASCIIStringByColor(IpAddr, 12 , 160+osdYShift , OSD_Blk2, 0xC0, 0x41);
    }
}

void uiOsdDrawNetworkLinkUp(void)
{
#if (NIC_SUPPORT)
    int LinkStatus;

    if ((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE))
        return;

    LinkStatus = Get_network_status();
    uiOsdDrawNetworkLink(LinkStatus);
#endif
}

void uiOsdDrawNetworkLink(u8 LinkUp)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_NET_LINK_LOC_X, UI_OSD_NET_LINK_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_NET_LINK_LOC_Y, UI_OSD_NET_LINK_LOC_TV_Y };
    int  p2pInfo;
    
    if (Main_Init_Ready == 0)
        return;
    if ((MyHandler.MenuMode != VIDEO_MODE) &&(MyHandler.MenuMode != QUAD_MODE))
        return;

    //DEBUG_RED("uiOsdDrawNetworkLink %d\r\n",LinkUp);
    //DEBUG_UI("sysTVOutOnFlag = %d\t TvOutMode = %d\t osdYShift = %d \n",sysTVOutOnFlag,TvOutMode,osdYShift); //Amon: Trace

    if (MyHandler.MenuMode == QUAD_MODE)
        LOC_X[0] = 308;
    
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 40, 32, LOC_X[0], LOC_Y[0], OSD_BLK[sysTVOutOnFlag], 0x00);
    
    if (LinkUp == UI_NETWORK_UP)
    {
        Check_P2P_info(&p2pInfo);
        if (p2pInfo == P2P_STATUS_OK)
            uiOSDIconColorByXY(OSD_ICON_NET_LINK_UP, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
        else
            uiOSDIconColorByXYChColor(OSD_ICON_NET_LINK_UP, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3, 0xc3, 0xc6);
    }
    else
    {
        uiOSDIconColorByXY(OSD_ICON_NET_LINK_DOWN, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
    }
}

#endif

void uiOsdIconInit(void)
{
    u32 i, j;
    u32 AllIdxNum;

    memset(uiOsdIconIdx, 0xFF, sizeof(uiOsdIconIdx));
    memset(uiStrIconIdx, 0xFF, sizeof(uiStrIconIdx));

    AllIdxNum = sizeof(OsdIcon)/sizeof(UI_OSDICON_TAB);
    for (i = 0; i < AllIdxNum; i++)
    {
        for (j = 0; j < OSD_ICON_MAX_NUM; j++)
        {
            if (OsdIcon[i].IconIdx == j)
            {
                uiOsdIconIdx[j] = (OSD_ICONIDX)i;
                break;
            }
        }
    }
    #if 0
    for (i = 0; i < OSD_ICON_MAX_NUM; i++)
    {
        DEBUG_UI("uiOsdIconIdx %d %d\n",i , uiOsdIconIdx[i]);
    }
    #endif
    AllIdxNum = sizeof(UiMsgStr)/sizeof(UI_MSG_TAB);
    for (i = 0; i < AllIdxNum; i++)
    {
        for (j = 0; j < MSG_MAX_NUM; j++)
        {
            if (UiMsgStr[i].MsgIdx == j)
            {
                uiStrIconIdx[j] = (MSG_SRTIDX)i;
                break;
            }
        }
    }
    #if 0
    for (i = 0; i < MSG_MAX_NUM; i++)
    {
        DEBUG_UI("uiStrIconIdx %d %d\n",i , uiStrIconIdx[i]);
    }
    #endif
}
#if RFIU_SUPPORT
u8 uiOsdTXWaitPair(u8 Camera)
{
    u8  err;
    u32 waitFlag = 0, cnt = 10;

    //uiClearOSDBuf(OSD_Blk1);
    //uiOsdEnable(OSD_Blk1);

    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS, OS_FLAG_CLR, &err);
    rfiu_PAIR_Linit(Camera);

    while (waitFlag == 0)
    {
        waitFlag = OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS,OS_FLAG_WAIT_SET_ANY, 20, &err);
        if (waitFlag & FLAGUI_RF_PAIR_SUCCESS)
        {
            DEBUG_UI("Wait PAIR Flag\r\n");

            return 1;

        }
        else if (cnt != 0)
        {
            cnt --;
            //sprintf ((char *)SecString, "Camera Side %2d", cnt);
			//uiOSDASCIIStringByColorY(SecString, 120, OSD_Blk2, 0xc0, 0x00);

        }
        else
        {
            rfiu_PAIR_Stop(Camera);
            DEBUG_UI("No Signal \r\n");


            return 0;
        }

    }

}

u8 uiOsdRXWaitPair(u8 Camera)
{
#if((HW_BOARD_OPTION == MR8600_RX_RDI )  || (HW_BOARD_OPTION == MR8120_RX_RDI) )
    u8  err;
    u32 waitFlag = 0, cnt = 30;
    u8 led=0;

    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS, OS_FLAG_CLR, &err);
    rfiu_PAIR_Linit(Camera);

    DEBUG_UI("enter draw pair \n");
    while (waitFlag == 0)
    {
        waitFlag = OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS,OS_FLAG_WAIT_SET_ANY, 20, &err);
        if (waitFlag & FLAGUI_RF_PAIR_SUCCESS)
        {
            DEBUG_UI("RF PAIR SUCCESS!\r\n");
            gpioTimerCtrLed(LED_OFF);
            return 1;
        }
        else if (cnt != 0)
        {
            led^=1;
            if(Camera==0)
            {
                gpioSetLevel(GPIO_GROUP_LED, GPIO_BIT_LED_6, led);
                gpioSetLevel(GPIO_GROUP_LED_RF1,GPIO_BIT_LED_7,0);
            }
            else
            {
                gpioSetLevel(GPIO_GROUP_LED, GPIO_BIT_LED_6, 0);
                gpioSetLevel(GPIO_GROUP_LED_RF1,GPIO_BIT_LED_7,led);
            }
            gpioTimerCtrLed((LED_R_LONG_FLASH+Camera));
            cnt --;
        }
        else
        {
            DEBUG_UI("No Signal \r\n");
            rfiu_PAIR_Stop(Camera);
            gpioTimerCtrLed(LED_OFF);
            return 0;
        }
    }
#endif
}

void uiOsdDrawSetting(u8 On)
{
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};

    if (On == 1)
    {
        uiMenuOSDReset();
        (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
        uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    }
    else
    {
        uiOsdDisable(OSD_BLK[sysTVOutOnFlag]);
    }
}

u8 uiOsdDrawPair(u8 Camera)
{
    u8  err;
    u32 waitFlag = 0, cnt = 30;
    u8 SecString[15];

    uiIsRFBroken=1;
    OSFlagPost(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS, OS_FLAG_CLR, &err);
    rfiu_PAIR_Linit(Camera);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],OSDDispWidth[sysTVOutOnFlag],OSDDispHeight[sysTVOutOnFlag],0,0,OSD_Blk2,0xC1C1C1C1);
    uiOsdDrawBitRate(0);
    uiOSDASCIIStringByColorY("Please Press Pair Key On", 100, OSD_Blk2, 0xc0, 0xC1);
    uiOSDASCIIStringByColorY("Camera Side 30", 120, OSD_Blk2, 0xc0, 0xC1);

    DEBUG_UI("enter draw pair \n");
    //IduVideo_ClearBuf();
    //IduVideo_ClearPKBuf(0);
    //IduVideo_ClearPKBuf(2);
    while (waitFlag == 0)
    {
        waitFlag = OSFlagPend(gUiStateFlagGrp, FLAGUI_RF_PAIR_SUCCESS,OS_FLAG_WAIT_SET_ANY, 20, &err);
        if (waitFlag & FLAGUI_RF_PAIR_SUCCESS)
        {
            DEBUG_UI("Wait PAIR Flag\r\n");
            uiClearOSDBuf(OSD_Blk2);
            #if (HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS)
                gpioTimerCtrLed(LED_R_ON);
            #endif
            return 1;

        }
        else if (cnt != 0)
        {
            cnt --;
            sprintf ((char *)SecString, "Camera Side %2d", cnt);

			uiOSDASCIIStringByColorY(SecString, 120, OSD_Blk2, 0xc0, 0xC1);
            #if((HW_BOARD_OPTION == MR8120_RX_JESMAY) || (HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS) )

                gpioTimerCtrLed(LED_R_FLASH);

            #endif

        }
        else
        {

            DEBUG_UI("No Signal \r\n");
        #if ((HW_BOARD_OPTION == MR8120_RX_JESMAY) || (HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS))
            uiOSDDrawNoSignal();
        #else
            uiOsdDrawNoSignal(UI_OSD_DRAW);
        #endif
            //uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],200,100,100,240,OSD_Blk2,0xC1C1C1C1);
           // uiOSDASCIIStringByColorY("No Signal", 100, OSD_Blk2, 0xc0, 0xC1);
            //OSTimeDly(40);
            //uiClearOSDBuf(OSD_Blk2);
            #if((HW_BOARD_OPTION == MR8120_RX_JESMAY) || (HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS) )

                gpioTimerCtrLed(LED_R_OFF);

            #endif
            rfiu_PAIR_Stop(Camera);

            return 0;
        }

    }

}
#endif

#define UI_MENU_DATETIME_YEAR_X     148
#define UI_MENU_DATETIME_MONTH_X    296
#define UI_MENU_DATETIME_DAY_X      412
#define UI_MENU_DATETIME_HOUR_X     172
#define UI_MENU_DATETIME_MIN_X      280
#define UI_MENU_DATETIME_SEC_X      384

#define UI_MENU_DATETIME_YEAR_Y     160
#define UI_MENU_DATETIME_HOUR_Y     242

u16 UiDateSetLocX[6]=
{
    UI_MENU_DATETIME_YEAR_X,
    UI_MENU_DATETIME_MONTH_X,
    UI_MENU_DATETIME_DAY_X,
    UI_MENU_DATETIME_HOUR_X,
    UI_MENU_DATETIME_MIN_X,
    UI_MENU_DATETIME_SEC_X,
};

void uiOsdDrawTimeFrame(u8 select , u8 clean)
{
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32  LenCol;
    u16  FrameH, FrameW, StartX, StartY;

    if(clean == 1)
    {
        LenCol  = 0;
    }
    else
    {
        LenCol  = 0xC3C3C3C3;
    }    
#if((HW_BOARD_OPTION == MR8200_RX_JIT)||(HW_BOARD_OPTION == MR8200_RX_JIT_BOX)||(HW_BOARD_OPTION == MR8120_RX_JIT_LCD)||\
    (HW_BOARD_OPTION == MR8120_RX_JIT_BOX))
    if (sysTVOutOnFlag == 0)
    {
        StartX = (UiDateSetLocX[select]*5)>>2;
        
    }
    else
    {
        StartX = UiDateSetLocX[select];
        
    }

    if(select ==0)
    {
        if (sysTVOutOnFlag == 0)
        {
            FrameW = 116;
            FrameH = 48;
            StartY=UI_MENU_DATETIME_YEAR_Y;
        }
        else
        {
            FrameW = 102;
            FrameH = 48;
            if(TvOutMode == 0)  //[0]:NTSC [1]:PAL
        StartY=UI_MENU_DATETIME_YEAR_Y;
            else
                StartY=UI_MENU_DATETIME_YEAR_Y+34;
        }  
        
    }
    else
    {
        if (sysTVOutOnFlag == 0)
        {
            FrameW = 112;
            FrameH = 48;
            if(select <3)
                StartY=UI_MENU_DATETIME_YEAR_Y;
            else
                StartY=UI_MENU_DATETIME_HOUR_Y;
        }
        else
        {
            FrameW = 86;
            FrameH = 48;
        if(select <3)
                if(TvOutMode == 0)  //[0]:NTSC [1]:PAL
            StartY=UI_MENU_DATETIME_YEAR_Y;
        else
                    StartY=UI_MENU_DATETIME_YEAR_Y+34;
            else
                if(TvOutMode == 0)  //[0]:NTSC [1]:PAL
            StartY=UI_MENU_DATETIME_HOUR_Y;   
                else
                    StartY=UI_MENU_DATETIME_HOUR_Y+51;
        }    

        
    }
#else
    if (sysTVOutOnFlag == 0)
    {
        StartX = (UiDateSetLocX[select]*5)>>2;
        
    }
    else
    {
        StartX = UiDateSetLocX[select];
        
    }

    if(select ==0)
    {
        if (sysTVOutOnFlag == 0)
        {
            FrameW = 152;
            FrameH = 52;
        }
        else
        {
            FrameW = 122;
            FrameH = 52;
        }  
        StartY=UI_MENU_DATETIME_YEAR_Y;
    }
    else
    {
        if (sysTVOutOnFlag == 0)
        {
            FrameW = 108;
            FrameH = 52;
        }
        else
        {
            FrameW = 86;
            FrameH = 52;
        }    

        if(select <3)
            StartY=UI_MENU_DATETIME_YEAR_Y;
        else
            StartY=UI_MENU_DATETIME_HOUR_Y;   
    }
#endif

    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, FrameH+8, StartX-4, StartY-4, OSD_BLK[sysTVOutOnFlag], LenCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, FrameH+8, StartX+FrameW, StartY-4, OSD_BLK[sysTVOutOnFlag], LenCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], FrameW, 4, StartX, StartY-4, OSD_BLK[sysTVOutOnFlag], LenCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], FrameW, 4, StartX, StartY+FrameH, OSD_BLK[sysTVOutOnFlag], LenCol);
    
}


#define UI_MENU_SCHEDULE_SET_CAM1_X     68
#define UI_MENU_SCHEDULE_SET_CAM_X_D    136
#define UI_MENU_SCHEDULE_SET_CAM1_Y     100
#define UI_MENU_SCHEDULE_SET_S_H_X      140
#define UI_MENU_SCHEDULE_SET_S_M_X      224
#define UI_MENU_SCHEDULE_SET_E_H_X      360
#define UI_MENU_SCHEDULE_SET_E_M_X      444
#define UI_MENU_SCHEDULE_SET_TIME_Y     235
#define UI_MENU_SCHEDULE_SET_WEEK_X     132
#define UI_MENU_SCHEDULE_SET_WEEK_X_D   56
#define UI_MENU_SCHEDULE_SET_WEEK_Y     320
#define UI_MENU_SCHEDULE_SET_MOTION_X   88
#define UI_MENU_SCHEDULE_SET_MOTION_Y   398
#define UI_MENU_SCHEDULE_SET_SET_X      308
#define UI_MENU_SCHEDULE_SET_SET_Y      402
#define UI_MENU_SCHEDULE_DEL_SET_X      452
#define UI_MENU_SCHEDULE_DEL_SET_Y      402


u32 UiSchSetLocX[17] =
{
    UI_MENU_SCHEDULE_SET_CAM1_X,
    UI_MENU_SCHEDULE_SET_CAM1_X+UI_MENU_SCHEDULE_SET_CAM_X_D,
    UI_MENU_SCHEDULE_SET_CAM1_X+UI_MENU_SCHEDULE_SET_CAM_X_D*2,
    UI_MENU_SCHEDULE_SET_CAM1_X+UI_MENU_SCHEDULE_SET_CAM_X_D*3,
    UI_MENU_SCHEDULE_SET_S_H_X,
    UI_MENU_SCHEDULE_SET_S_M_X,
    UI_MENU_SCHEDULE_SET_E_H_X,
    UI_MENU_SCHEDULE_SET_E_M_X,
    UI_MENU_SCHEDULE_SET_WEEK_X,
    UI_MENU_SCHEDULE_SET_WEEK_X+UI_MENU_SCHEDULE_SET_WEEK_X_D,
    UI_MENU_SCHEDULE_SET_WEEK_X+UI_MENU_SCHEDULE_SET_WEEK_X_D*2,
    UI_MENU_SCHEDULE_SET_WEEK_X+UI_MENU_SCHEDULE_SET_WEEK_X_D*3+4,
    UI_MENU_SCHEDULE_SET_WEEK_X+UI_MENU_SCHEDULE_SET_WEEK_X_D*4+8,
    UI_MENU_SCHEDULE_SET_WEEK_X+UI_MENU_SCHEDULE_SET_WEEK_X_D*5+8,
    UI_MENU_SCHEDULE_SET_WEEK_X+UI_MENU_SCHEDULE_SET_WEEK_X_D*6+8,
  //  UI_MENU_SCHEDULE_SET_MOTION_X,
    UI_MENU_SCHEDULE_SET_SET_X,
    UI_MENU_SCHEDULE_DEL_SET_X
};
u32 UiSchSetLocY[17] =
{
    UI_MENU_SCHEDULE_SET_CAM1_Y,
    UI_MENU_SCHEDULE_SET_CAM1_Y,
    UI_MENU_SCHEDULE_SET_CAM1_Y,
    UI_MENU_SCHEDULE_SET_CAM1_Y,
    UI_MENU_SCHEDULE_SET_TIME_Y,
    UI_MENU_SCHEDULE_SET_TIME_Y,
    UI_MENU_SCHEDULE_SET_TIME_Y,
    UI_MENU_SCHEDULE_SET_TIME_Y,
    UI_MENU_SCHEDULE_SET_WEEK_Y,
    UI_MENU_SCHEDULE_SET_WEEK_Y,
    UI_MENU_SCHEDULE_SET_WEEK_Y,
    UI_MENU_SCHEDULE_SET_WEEK_Y,
    UI_MENU_SCHEDULE_SET_WEEK_Y,
    UI_MENU_SCHEDULE_SET_WEEK_Y,
    UI_MENU_SCHEDULE_SET_WEEK_Y,
   // UI_MENU_SCHEDULE_SET_MOTION_Y,
    UI_MENU_SCHEDULE_SET_SET_Y,
    UI_MENU_SCHEDULE_DEL_SET_Y
};




void uiOsdDrawScheduledFrame(u8 setCursor,u8 clean)
{
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32  LenCol;
    u16  FrameH, FrameW, StartX, StartY;

    if(clean == 1)
    {
        LenCol  = 0;
    }
    else
    {
        LenCol  = 0xC3C3C3C3;
    }
#if((HW_BOARD_OPTION == MR8200_RX_JIT)||(HW_BOARD_OPTION == MR8200_RX_JIT_BOX)||(HW_BOARD_OPTION == MR8120_RX_JIT_LCD)||\
    (HW_BOARD_OPTION == MR8120_RX_JIT_BOX))
    if (sysTVOutOnFlag == 0)
    {
        StartX = (UiSchSetLocX[setCursor]*5)>>2;
        StartY = UiSchSetLocY[setCursor]-4;
    }
    else
    {
        StartX = UiSchSetLocX[setCursor];
        StartY = UiSchSetLocY[setCursor]-4;
    }
    if (setCursor <= 3) /*select camera*/
    {
        if (sysTVOutOnFlag == 0)
        {
            FrameW = 120;
            FrameH = 52;
        }
        else
        {
            if(TvOutMode)
                StartY += 25;
            FrameW = 96;
            FrameH = 48;
        }
    }
    else if (setCursor <= 7)    /*Schedule Time*/
    {
        if (sysTVOutOnFlag == 0)
        {
            StartX -= 8;
            StartY -= 12;
            FrameW = 60;
            FrameH = 52;
        }
        else
        {
            StartX -= 8;
            if(TvOutMode)
                StartY += 35;
            else   
                StartY -= 12;
            FrameW = 48;
            FrameH = 54;
        }
    }
    else if (setCursor <= 14)    /*Schedule Week*/
    {
        if (sysTVOutOnFlag == 0)
        {
            FrameW = 52;
            FrameH = 30;
        }
        else
        {
            if(TvOutMode)
                StartY += 65;
            FrameW = 44;
            FrameH = 24;
        }
    }
    else
    {
        if (sysTVOutOnFlag == 0)
        {
            FrameW = 96;
            FrameH = 40;
        }
        else
        {
            if(TvOutMode)
                StartY += 75;
            FrameW = 78;
            FrameH = 38;
        }
    }

#else
    if (sysTVOutOnFlag == 0)
    {
        StartX = (UiSchSetLocX[setCursor]*5)>>2;
        StartY = UiSchSetLocY[setCursor]-4;
    }
    else
    {
        StartX = UiSchSetLocX[setCursor];
        StartY = UiSchSetLocY[setCursor]-4;
    }
    if (setCursor <= 3) /*select camera*/
    {
        if (sysTVOutOnFlag == 0)
        {
            FrameW = 128;
            FrameH = 52;
        }
        else
        {
            if(TvOutMode)
                StartY += 25;
            FrameW = 100;
            FrameH = 52;
        }
    }
    else if (setCursor <= 7)    /*Schedule Time*/
    {
        StartX -= 8;
        if (sysTVOutOnFlag == 0)
        {
            StartY -= 12;
            FrameW = 84;
            FrameH = 60;
        }
        else
        {
            if(TvOutMode)
                StartY += 35;
            else   
                StartY -= 12;
            FrameW = 68;
            FrameH = 56;
        }
    }
    else if (setCursor <= 14)    /*Schedule Week*/
    {
        if (sysTVOutOnFlag == 0)
        {
            FrameW = 56;
            FrameH = 30;
        }
        else
        {
            if(TvOutMode)
                StartY += 65;
            FrameW = 46;
            FrameH = 24;
        }
    }
    else
    {
        if (sysTVOutOnFlag == 0)
        {
            FrameW = 120;
            FrameH = 52;
        }
        else
        {
            if(TvOutMode)
                StartY += 75;
            FrameW = 96;
            FrameH = 52;
        }
    }
#endif
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, FrameH+8, StartX-4, StartY-4, OSD_BLK[sysTVOutOnFlag], LenCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, FrameH+8, StartX+FrameW, StartY-4, OSD_BLK[sysTVOutOnFlag], LenCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], FrameW, 4, StartX, StartY-4, OSD_BLK[sysTVOutOnFlag], LenCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], FrameW, 4, StartX, StartY+FrameH, OSD_BLK[sysTVOutOnFlag], LenCol);
}

void osdDrawSystemReboot(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};

    IduVideo_ClearPKBuf(0);
    iduSetVideoBuf0Addr(PKBuf0);
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    osdDrawMessage(MSG_SYSTEM_REBOOT, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    OSTimeDly(20);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);        /* Clear the OSD Buf */
}

void osdDrawUpgradeFW(void)
{
    u8  err;
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32 waitFlag = 0, cnt = 0, draw;
    u8 *strLib;
    u8 font_w, font_h;
    UI_MULT_LAN_STR *strInfo;
    u16 drawX, drawY;
    void* msg;

    if (uiOsdGetStrLib(MSG_PLEASE_WAIT, &strLib, &font_w, &font_h, &strInfo) == 0)
        return;

    drawX = (OSDDispWidth[sysTVOutOnFlag] - strInfo->Str_len*font_w)/2+(strInfo->Str_len -3)*font_w;
    drawY = (OSDDispHeight[sysTVOutOnFlag] - font_h)/2;

    DEBUG_UI("enter draw upgrade f/w \n");
    IduVideo_ClearPKBuf(0);
    iduSetVideoBuf0Addr(PKBuf0);
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);


    while (waitFlag == 0)
    {
        waitFlag = OSFlagPend(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_UPDATE_SEC, OS_FLAG_WAIT_SET_ANY, 20, &err);       
        if (waitFlag == 0)
        {
            osdDrawMessage(MSG_PLEASE_WAIT, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
        
            draw = cnt%4;
            if (draw == 0)
                uiOSDASCIIStringByColor("   ", drawX, drawY, OSD_BLK[sysTVOutOnFlag] , 0x00, 0x00);
            else
                uiOSDASCIIStringByColor(".", drawX+(draw-1)*font_w, drawY, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0x00);
        }
        cnt++;
    }
    osdDrawMessage(MSG_PLEASE_WAIT, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
    msg=OSMboxPend(general_MboxEvt, 40, &err);
    if (!strcmp((const char*)msg, "PASS"))
    {
        osdDrawMessage(MSG_FW_UPDATE_PASS, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    }
    else if (!strcmp((const char*)msg, "FAIL"))
        osdDrawMessage(MSG_FW_UPDATE_FAIL, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    else
        osdDrawMessage(MSG_TIME_OUT, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    OSTimeDly(20);
    osdDrawMessage(MSG_SYSTEM_REBOOT, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    OSTimeDly(20);
    sysForceWDTtoReboot();
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);        /* Clear the OSD Buf */
}

void uiOsdDrawMaskAreaSel(u8 SetX, u8 SetY, u8 Act)
{
    u32 selColor;
    u32 LinColar = 0x000000C0;
    u16 DrawX, DrawY;

    if (Act == UI_OSD_DRAW)
    {
        selColor = 0xE8E8E8E8;
        LinColar = 0xE8E8E8C0;
    }
    else
    {
        selColor = 0;
        LinColar = 0x000000C0;
    }

    DrawX = 193+SetX*64;
    DrawY = 121+SetY*72;

    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],  4, 71, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], LinColar);
    DrawX += 4;
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 60, 71, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], selColor);

}

void uiOsdDrawMaskAreaSetting(u8 Key, u8 Camid)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 DrawX, DrawY, startX = 193, StartY = 120;
    u32 LinColar = 0x000000C0;
    u8  i, j, SetX =0, SetY =0,cnt=0;

    switch(Key)
    {
        case 0:
            IduVideoEnable(0);
            IduVideo_ClearPKBuf(0);
            IduVideo_ClearPKBuf(2);
            iduSetVideoBuf0Addr(PKBuf0);
            sysRFRxInMainCHsel=Camid;
            uiSetRfDisplayMode(UI_MENU_RF_ENTER_MASKAREA);
            uiMenuOSDReset();
            (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
            uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
            DrawX = startX;
            DrawY = StartY;
            for ( i = 0; i < 11; i++)//x
            {
                uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, 360, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], LinColar);
                DrawX += 64;
            }
            DrawX = startX;
            for ( i = 0; i < 8; i++)//y
            {
                uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 640, 1, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0xC0C0C0C0);
                DrawY += 64;
            }
            break;

        case UI_KEY_ENTER:           
            SetX = (UiGetTouchX -110)/40;
            SetY = (UiGetTouchY -100)/60;
            i = (SetX+SetY*10)/8;
            j = (SetX+SetY*10)%8;

            if (uiMaskArea[Camid][i] & (1<<j))
            {
                uiOsdDrawMaskAreaSel(SetX, SetY, UI_OSD_CLEAR);
                uiMaskArea[Camid][i] &= ~(1<<j);
            }
            else
            {
                uiOsdDrawMaskAreaSel(SetX, SetY, UI_OSD_DRAW);
                uiMaskArea[Camid][i] |= (1<<j);
            }
            //uiGraphDrawMaskAreaGraph(0,UI_OSD_CLEAR);
            //uiGraphDrawMaskAreaGraph(1,UI_OSD_CLEAR);
            /*避免執行Key連續發送*/
            OSTimeDly(UI_TOUCH_ACT_DELAY);
            break;
            
        case UI_KEY_UP: //Select all Mask Area

            for(i=0; i<7 ;i++)
            {
                if (i == 6)
                {
                    uiMaskArea[Camid][i] = 3;
                }
                else
                    uiMaskArea[Camid][i] = 255;
            }

            for (i=0 ; i<7 ;i++)
            { 
                for(j=0 ; j<8 ; j++)
                {
                    if (uiMaskArea[Camid][i] & (1<<j))
                        uiOsdDrawMaskAreaSel(SetX,SetY,UI_OSD_DRAW);
                    SetX++; 
                    if (SetX >= 10)
                    {
                        SetY++;
                        SetX = 0;
                    }
                }        
            }
            //uiGraphDrawMaskAreaGraph(1,UI_OSD_DRAW);
            //uiGraphDrawMaskAreaGraph(0,UI_OSD_CLEAR);
            break;
            
        case UI_KEY_DOWN:  //Delete All Mask Area
            for(i=0; i<7 ;i++)
            {
                uiMaskArea[Camid][i] = 0;
            }

            for (i=0 ; i<7 ;i++)
            { 
                for(j=0 ; j<8 ; j++)
                {
                    if (i==6 && j==2)
                        break;
                    uiOsdDrawMaskAreaSel(SetX,SetY,UI_OSD_CLEAR);
                    SetX++; 
                    if (SetX >= 10)
                    {
                        SetY++;
                        SetX = 0;
                    }
                }        
            }
            //uiGraphDrawMaskAreaGraph(0,UI_OSD_DRAW);
            //uiGraphDrawMaskAreaGraph(1,UI_OSD_CLEAR);
            break;
            
    }
}

/*Act = 0 : init
        1 : Draw one
        2 : Clean one
        3 : Clean All*/
void uiOsdDrawLoadIcon(u8 index, u8 Act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 DrawX[10] = {36, 56, 66, 66, 56, 36, 16, 4, 4, 16};
    u16 DrawY[10] = {2, 9, 24, 43, 61, 67, 61, 44, 23, 7};
    u8  TmpIndex;
    u16 StartX = 500, StartY = 255; 

    switch(Act)
    {
        case 0: /*init*/
            uiOsdDrawInit();
            IduOsdL1Ctrl &= ~(0x00003000);
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag], 0, 0, OSD_BLK[sysTVOutOnFlag], 0x81818181);
            break;

        case 1: /*Draw one*/
            TmpIndex = (index+6)%10;
            uiOSDIconColorByXYChColor(OSD_ICON_LOAD, StartX+DrawX[TmpIndex], StartY+DrawY[TmpIndex], OSD_BLK[sysTVOutOnFlag], 0x81, alpha_3, 0xde, 0x40);
            TmpIndex = (index+7)%10;
            uiOSDIconColorByXYChColor(OSD_ICON_LOAD, StartX+DrawX[TmpIndex], StartY+DrawY[TmpIndex], OSD_BLK[sysTVOutOnFlag], 0x81, alpha_3, 0xde, 0xdd);
            TmpIndex = (index+8)%10;
            uiOSDIconColorByXYChColor(OSD_ICON_LOAD, StartX+DrawX[TmpIndex], StartY+DrawY[TmpIndex], OSD_BLK[sysTVOutOnFlag], 0x81, alpha_3, 0xde, 0xe5);
            TmpIndex = (index+9)%10;
            uiOSDIconColorByXYChColor(OSD_ICON_LOAD, StartX+DrawX[TmpIndex], StartY+DrawY[TmpIndex], OSD_BLK[sysTVOutOnFlag], 0x81, alpha_3, 0xde, 0xC7);
            uiOSDIconColorByXYChColor(OSD_ICON_LOAD, StartX+DrawX[index], StartY+DrawY[index], OSD_BLK[sysTVOutOnFlag], 0x81, alpha_3, 0xde, 0xC0);
            break;

        case 2: /*Clean one*/
            //uiOSDIconColorByXYChColor(OSD_ICON_LOAD, 344+DrawX[index], 184+DrawY[index], OSD_BLK[sysTVOutOnFlag], 0x81, alpha_3, 0xca, 0x81);
            break;

        case 3: /*Clean All*/
            uiOsdDisable(OSD_BLK[sysTVOutOnFlag]);
            break;            
    }
}

/*
 *********************************************************************************************************
 * Called by other moudle
 *********************************************************************************************************
 */
void osdDrawPlaybackArea(u8 mode)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};

    uiMenuOSDReset();
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    if(mode == 2)   /*rf playback*/
    {
        osdDrawPreviewIcon();
        osdDrawQuadIcon();
    }
    
}

void uiOsdDrawTalkBack(u8 on)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    UI_MULT_ICON *iconInfo;
    u8 talk1_H=0,talk2_H=0;
    
    uiOSDIconColorByXY(OSD_ICON_TALKBACK,UI_OSD_TALK_LOC_X, UI_OSD_TALK_LOC_Y , OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);

    uiOsdGetIconInfo(OSD_ICON_TALK_ING1,&iconInfo);
    talk1_H += iconInfo->Icon_h;
    if (on == UI_OSD_DRAW)
    {
        uiOSDIconColorByXY(OSD_ICON_TALK_ING1,UI_OSD_TALK_LOC_X-4, UI_OSD_TALK_LOC_Y - talk1_H - 20, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
    }
    else
    {
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, talk1_H ,
        UI_OSD_TALK_LOC_X-4 , UI_OSD_TALK_LOC_Y - talk1_H - 20, OSD_BLK[sysTVOutOnFlag] , 0);

    }
}

void uiOsdDrawRemindDownload(u8 RemindDown)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_DOWNLOAD_LOC_X, 520 };
    u16 LOC_Y [2] = {UI_OSD_DOWNLOAD_LOC_Y, 420 };
    UI_MULT_ICON *iconInfo;

    if (Main_Init_Ready == 0)
        return;
    if ((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE))
        return;

    DEBUG_UI("uiOsdDrawRemindDownload %d\r\n",RemindDown);
    if (RemindDown == UI_OSD_DRAW)
        uiOSDIconColorByXY(OSD_ICON_DOWNLOAD ,LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag]+ osdYShift, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    else
    {
        uiOsdGetIconInfo(OSD_ICON_DOWNLOAD,&iconInfo);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, iconInfo->Icon_h,
                LOC_X[sysTVOutOnFlag] , LOC_Y[sysTVOutOnFlag]+ osdYShift , OSD_BLK[sysTVOutOnFlag] , 0);
    }
}

void uiOsdDrawOverwrit(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_OVERWRITE_LOC_X, UI_OSD_OVERWRITE_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_OVERWRITE_LOC_Y, UI_OSD_OVERWRITE_LOC_TV_Y };

    if (iconflag[UI_MENU_SETIDX_OVERWRITE] == UI_MENU_SETTING_OVERWRITE_YES)
        uiOSDIconColorByXY(OSD_ICON_OVERWRITE,LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
    else
        uiOSDIconColorByXY(OSD_ICON_OVERWRITE,LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00, alpha_0);
}

void osdDrawProtect(u8 mode)
{
    u8 str[] = "Write Protect";

   if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
   {
        if(mode==1) /*Playback*/
        {
            (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk1, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
            uiOsdEnable(OSD_Blk1);
            uiClearOSDBuf(OSD_Blk1);
            uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,(OSDIconMidX-8), (OSDIconMidY-68), OSD_Blk1, 0x00 , alpha_3);
            uiOSDASCIIStringByColor(str, (OSDIconMidX-strlen(str)*OSD_STRING_W/2) , (OSDIconMidY-40), OSD_Blk1 , 0xC0, 0x00);
            OSTimeDly(20);
        }
        else if(mode ==2)  // Preview
        {
            uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,(OSDIconMidX-8), (OSDIconMidY-68), OSD_Blk2, 0x00 , alpha_3);
            uiOSDASCIIStringByColor(str, (OSDIconMidX-strlen(str)*OSD_STRING_W/2) , (OSDIconMidY-40), OSD_Blk2 , 0xC0, 0x00);
            OSTimeDly(20);
            uiClearOSDBuf(OSD_Blk2);
            osdDrawPreviewIcon();
        }
    }
}

void uiOsdDrawBlackAll(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};

    if (act == UI_OSD_DRAW)
    {
        uiMenuOSDReset();
        OSD_Black_Color_Bar(1);
        (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
        uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],OSDDispWidth[sysTVOutOnFlag],OSDDispHeight[sysTVOutOnFlag],0,0, OSD_BLK[sysTVOutOnFlag],0xC1C1C1C1);
    }
    else
    {        
        uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
        OSD_Black_Color_Bar(0);
    }

}


void osdDrawFlashLight(u8 idx)
{
}

void osdDrawSDIcon(u8 on)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    UI_MULT_ICON *iconInfo;
    u16 LOC_X [2] = {UI_SD_ICON_LOC_X,560};
    u16 LOC_Y [2] = {UI_SD_ICON_LOC_Y,420};

    if (Main_Init_Ready == 0)
        return;

    if ((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE))
        return;
    
    if (MyHandler.MenuMode == QUAD_MODE)
        LOC_X[0] = 392;

    if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_USBMASS)// Insert USB 
        if (on == UI_OSD_DRAW)
            uiOSDIconColorByXY(OSD_ICON_HDD ,LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        else
        {
            uiOsdGetIconInfo(OSD_ICON_HDD,&iconInfo);
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, iconInfo->Icon_h,
                    LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0);
    	}
    else
        if (on == UI_OSD_DRAW)
            uiOSDIconColorByXY(OSD_ICON_SD ,LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        else
        {
            uiOsdGetIconInfo(OSD_ICON_SD,&iconInfo);
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, iconInfo->Icon_h,
                    LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0);
    	}
}

void uiosdDrawResolution(void)
{
    u8 resolution[5]={0};
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_RES_LOC_X, UI_OSD_RES_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_RES_LOC_Y, UI_OSD_RES_LOC_TV_Y };


    if((gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth == 640) && (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight == 480))
    {
        sprintf(resolution,"VGA");            
    }
    else if((gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth == 1280) && (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight == 720))
    {
        sprintf(resolution,"HD");    
    }
    else if((gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth == 320) && (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight == 240))
    {
        sprintf(resolution,"QVGA");    
    }
    if(sysTVOutOnFlag == 0)    
        uiOSDASCIIStringByColor(resolution, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag],  OSD_BLK[sysTVOutOnFlag], 0xc4, 0x00);
    else
        uiOSDASCIIStringByColor(resolution, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag],  OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
}


void osdDrawVideoIcon(void)
{
#if(LCM_OPTION == LCM_P_RGB_888_Innolux)//Lucian: have bug on OSD piority A1016. close it for test,7/11
   if (sysTVOutOnFlag==0)
	  uiOsdDisableAll();
#endif
	osdDrawMenuPreview();
    osdDrawQuadPreview();
   if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
    osdDrawSDIcon(UI_OSD_DRAW);

}


void osdDrawVideoTime(void)
{
    char  timeForRecord1[10];
    char  timeForRecord2[10];
    u8    h, m, s;
    u16 x[2]={72,200};
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_PLAYBACK_LOC_VIDEO_TIME_X, 400}; //Refer uiOsdDrawPlayTime LOC_X
    u16 LOC_Y [2] = {UI_PLAYBACK_LOC_VIDEO_TIME_Y, 430}; //Refer uiOsdDrawPlayTime LOC_Y
    //u32 StrH;
    static u32 tmpRTCseconds = 0;
    static  s64 Vtime;
    static  int i;

    
    //StrH = uiOsdGetFontHeight(CurrLanguage)+8;
    LOC_Y[0] = OSDIconEndY-(uiOsdGetFontHeight(CurrLanguage)+8)-40;

    if (sysCaptureVideoStart==1)
    {
        if (tmpRTCseconds != RTCseconds)
        {
            tmpRTCseconds = RTCseconds;
        }
        else if (tmpRTCseconds != 0)
            return;
        #if (UI_PREVIEW_OSD == 0)
            return;
        #endif
        h = RTCseconds / 3600;
        m = (RTCseconds - h*3600) / 60;
        s = RTCseconds - h*3600 - m*60;
        sprintf (timeForRecord1, "%02d:%02d:%02d", h, m, s);
        uiOSDASCIIStringByColor((u8*)timeForRecord1, x[sysTVOutOnFlag] , 11 , OSD_Blk2 , 0xc0, 0x00);
    }
    else if (sysPlaybackVideoStart == 1)
    {
        #if 0
        if (tmpVideoNextPresentTime != VideoNextPresentTime)
        {
            tmpVideoNextPresentTime = VideoNextPresentTime;
        }
        else if (tmpVideoNextPresentTime != 0)
            return;
        #endif
        #if (HW_BOARD_OPTION == MR9670_COMMAX) || (HW_BOARD_OPTION == MR9670_COMMAX_WI2)
        {
            void ShowSingleMovieTimeOsd(int now, int total);    // ref. MenuFunc.c

            ShowSingleMovieTimeOsd(VideoNextPresentTime/1000000, VideoDuration-3);
            return;
        }
        #endif

        h = (VideoDuration-3) / 3600;
        m = ((VideoDuration-3) - h*3600) / 60;
        s = (VideoDuration-3) - h*3600 - m*60;
        sprintf (timeForRecord1, "/%02d:%02d:%02d", h, m, s);
#if (USE_BIG_OSD == 1)
        uiOSDASCIIStringByColor((u8*)timeForRecord1, LOC_X[sysTVOutOnFlag]+128 , LOC_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
#else
        uiOSDASCIIStringByColor((u8*)timeForRecord1, x[sysTVOutOnFlag] , 7 , OSD_Blk0 , 0xc0, 0x00);
#endif
        h = VideoNextPresentTime/3600000000;
        m = (VideoNextPresentTime/1000000 - h*3600) / 60;
        s = VideoNextPresentTime/1000000 - h*3600 - m*60;
        if (Vtime != VideoNextPresentTime)
        {
            Vtime   = VideoNextPresentTime;
            i       = 0;
        }
        else if(!sysPlaybackVideoPause)
        {
            i++;
        }
        if((video_playback_speed == 5) && ((VideoNextPresentTime / 1000000 + 4) >= VideoDuration) && (i > 1))
        {
            strcpy(timeForRecord2, &timeForRecord1[1]);
            if(i == 6)   // 停三秒回到 playback list
                uiSentKeyToUi(UI_KEY_STOP);
        }
        else if((video_playback_speed > 5) && ((((VideoNextPresentTime / 1000000 + 6) >= VideoDuration) && (i > 1)) || (((VideoNextPresentTime / 1000000 + 9) >= VideoDuration) && (i > 4))))
        {
            strcpy(timeForRecord2, &timeForRecord1[1]);
        }
        else
        {
            sprintf (timeForRecord2, "%02d:%02d:%02d", h, m, s);
        }
#if (USE_BIG_OSD == 1)
        uiOSDASCIIStringByColor((u8*)timeForRecord2, LOC_X[sysTVOutOnFlag] , LOC_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
#else
        uiOSDASCIIStringByColor((u8*)timeForRecord2, x[sysTVOutOnFlag]-64 , 7 , OSD_Blk0 , 0xc0, 0x00);
#endif
        if (volumeflag > 0)
        {
            volumeflag++;
            /*clean volume osd*/
            if(volumeflag > 3)
            {
                uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_CLEAN);
                Save_UI_Setting();
            }
        }
    }
}

void osdDrawPlayIndicator(u8 type)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X[2] = {16 ,84};
    u16 LOC_Y[2] = {12 ,25};

    switch(type)
    {    
        case 100:/* Playback Video 結束時, PAUSE 符號消失*/
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 20 , 25 , LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag] , 0);
            uiOSDIconColorByXY(OSD_ICON_STOP ,16 , 12 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
            uiOSDIconColorByXY(OSD_ICON_PLAY ,LOC_X[sysTVOutOnFlag]+70+356, LOC_Y[sysTVOutOnFlag]+8, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
            break;
            
        case UI_PLAY_ICON_PAUSE://status : pause , draw :play
            uiOSDIconColorByXY(OSD_ICON_PAUSE , LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
            uiOSDIconColorByXY(OSD_ICON_PLAY ,LOC_X[sysTVOutOnFlag]+70+356, LOC_Y[sysTVOutOnFlag]+8, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
			break;

        case UI_PLAY_ICON_PLAY:
            uiOSDIconColorByXY(OSD_ICON_PLAYBACK , LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
            uiOSDIconColorByXY(OSD_ICON_PAUSE_L ,LOC_X[sysTVOutOnFlag]+70+356, LOC_Y[sysTVOutOnFlag]+8, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
            break;
            
        default:
            DEBUG_UI("osdDrawPlayIndicator %d \n",type);
            break;
    }
}

void uiDrawNetworkInfo(UI_NET_INFO *NetInfo, u8 on)
{
    u16 y_pos[4]   = {158, 238, 320, 398};
    u16 LOC_X[2][4] = {{434, 510, 582, 658},{152, 217, 283, 349}}; //LOC_x[0]:Panel [1]:TV
    u16 x_loc[2] = {422,246} ; // [0]:Panel [1]:TV
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u8  tempStr[4] = {0};
    u8  i,j;
    u8 *ip;

   if(on == UI_OSD_DRAW)
    {
        (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
        uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
        uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
        
        for(j=0; j<3; j++)
        {   
            switch(j)
            {
                case 0:
                    ip = NetInfo->IPaddr;
                    break;

                case 1:
                    ip = NetInfo->Netmask;
                    break;

                case 2:
                    ip = NetInfo->Gateway;
                    break;
            }

            for(i=0; i<4; i++)
            {
                sprintf((u8 *) tempStr ,"%3d", ip[i]);
                uiOSDASCIIStringByColor(tempStr, LOC_X[sysTVOutOnFlag][i] ,y_pos[j] , OSD_BLK[sysTVOutOnFlag] , 0xc1, 0x00);
            }
        }   
        uiOSDASCIIStringByColor(uiP2PID, x_loc[sysTVOutOnFlag] ,y_pos[3] , OSD_BLK[sysTVOutOnFlag] , 0xc1, 0x00);       
    }
    else
    {
        uiOsdDisable(OSD_BLK[sysTVOutOnFlag]); 
    }

}
void osdDrawFileNum(u32 num)  // TVOUT-flag
{
    u8 filestring[20];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_BASE_X, UI_OSD_BASE_TV_X};
    u16 LOC_Y [2] = { 0, 430};
    //u32 StrH;
    //StrH = uiOsdGetFontHeight(CurrLanguage)+8;
    LOC_Y[0] = OSDIconEndY-(uiOsdGetFontHeight(CurrLanguage)+8);
    if(dcfGetCurDirFileCount()<=9999)
        sprintf((char*)filestring,"%04d/%04d",num,dcfGetCurDirFileCount());
    else if((dcfGetCurDirFileCount()>=10000)&&(dcfGetCurDirFileCount()<=99999))
        sprintf((char*)filestring,"%05d/%05d",num,dcfGetCurDirFileCount());
    else if((dcfGetCurDirFileCount()>=100000)&&(dcfGetCurDirFileCount()<=999999))
        sprintf((char*)filestring,"%06d/%06d",num,dcfGetCurDirFileCount());

#if (USE_BIG_OSD == 1)
    uiOSDASCIIStringByColor(filestring, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag] + osdYShift, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
#else
    uiOSDASCIIStringByColor(filestring, 12 , 36, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
#endif
}

void osdDrawPlayIcon(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {372, 560}; //[0]:Panel [1]:TV
    u16 LOC_Y [2] = {UI_OSD_BASE_Y, UI_OSD_BASE_TV_Y_U}; //[0]:Panel [1]:TV
    UI_MULT_ICON *iconInfo;
    u16 DrawX=0,DrawY=0; //[0]:Panel [1]:TV
    u8  storage = OSD_ICON_SD;
    //LOC_X[0] = (OSDDispWidth[0]-24);

#if (USE_BIG_OSD == 1)
    uiOSDIconColorByXY(OSD_ICON_PLAY_ACT_REW_1 ,LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag]+8, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    uiOSDIconColorByXY(OSD_ICON_PLAY ,LOC_X[sysTVOutOnFlag]+70, LOC_Y[sysTVOutOnFlag]+8, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    uiOSDIconColorByXY(OSD_ICON_PLAY_ACT_STOP_1 ,LOC_X[sysTVOutOnFlag]+140, LOC_Y[sysTVOutOnFlag]+8, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    uiOSDIconColorByXY(OSD_ICON_PLAY_ACT_FF_1 ,LOC_X[sysTVOutOnFlag]+210, LOC_Y[sysTVOutOnFlag]+8, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
#else
    uiOSDIconColorByXY(OSD_ICON_PLAY ,LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag]+8, OSD_Blk0, 0xC0 , alpha_3);
#endif
    osdDrawReturn();
    osdDrawFileNum(playback_location);
    //uiOsdDrawPlayTime(sysThumnailPtr->type,VideoDuration-3);

    //draw play sd icon
    if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_USBMASS)// Insert USB 
        storage = OSD_ICON_HDD;
    else
        storage = OSD_ICON_SD;

    uiOsdGetIconInfo(storage,&iconInfo);
    DrawX = OSDIconEndX-iconInfo->Icon_w-8;
    DrawY = OSDIconEndY-iconInfo->Icon_h-8;
    uiOSDIconColorByXY(storage ,DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    
}

void osdDrawFillUSBMSC(void)
{
    (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk1, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_Blk1);
    uiOsdEnable(OSD_Blk1);
    osdDrawMessage(MSG_USB_MASS_STORAGE, CENTERED, CENTERED, OSD_Blk1, 0xC0, 0x00);
}

void osdDrawFillWait(void)
{


    (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk2, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_Blk2);
    uiOsdEnable(OSD_Blk2);
    osdDrawMessage(MSG_PLEASE_WAIT, CENTERED, 72+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
}

void osdDrawShowZoom(u8 value)
{
    #if 0
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    if(value == 1)//+
    {
        if(zoomcoff == 18)
            zoomcoff = 18;
        else
            zoomcoff++;

        uiOSDIconColorByXY(OSD_ICON_ZOOMplus ,8 , 54 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        uiOSDIconColorByXY(OSD_ICON_ZOOM1+zoomcoff ,8 , 84 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        uiOSDIconColorByXY(OSD_ICON_ZOOMminus ,8 , 156 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        zoomflag=1;
    }
    else if(value == 2)//-
    {
        if(zoomcoff == 0)
            zoomcoff = 0;
        else
            zoomcoff--;

        uiOSDIconColorByXY(OSD_ICON_ZOOMplus ,8 , 54 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        uiOSDIconColorByXY(OSD_ICON_ZOOM1+zoomcoff ,8 , 84 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        uiOSDIconColorByXY(OSD_ICON_ZOOMminus ,8 , 156 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        zoomflag=1;
    }
    else
    {
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 16 , 126 , 8 , 54 , OSD_BLK[sysTVOutOnFlag] , 0);
        zoomflag=0;
    }
    #endif
}

void osdDrawDelMsg(s8* msg,u32 index)        /* index 0 means delete one */
{
    u8 num_string[12];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    if(index!=0)
    {
        sprintf((char*)num_string,"%05d/%05d",index,dcfGetCurDirFileCount());
        uiOSDASCIIStringByColor(num_string, 16 , 128, OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
    }
    else
    {
        sprintf((char*)num_string,"%05d/%05d",1,1);
        uiOSDASCIIStringByColor(num_string, 16 , 128, OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
    }
    uiOSDASCIIStringByColor((u8*)msg, 16 , 148+(osdYShift/2), OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
}

void osdDrawMemFull(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_SD_FULL_LOC_X, UI_OSD_SD_FULL_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_SD_FULL_LOC_Y, UI_OSD_SD_FULL_LOC_TV_Y };
    
    if (Main_Init_Ready == 0)
        return;

    if ((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE))
        return;

    if (MyHandler.MenuMode == QUAD_MODE)
        LOC_X[0] = 392;

    if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_USBMASS)// Insert USB
    {
        if(act == UI_OSD_DRAW)
        {
            uiOSDIconColorByXYChColor(OSD_ICON_HDD, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3, 0xe8, 0xc2);
        }
        else
        {
            if (MemoryFullFlag == FALSE)
            {
                uiOSDIconColorByXY(OSD_ICON_HDD, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
            }
        }
    }
    else
    {
        if(act == UI_OSD_DRAW)
        {
            uiOSDIconColorByXYChColor(OSD_ICON_SD, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3, 0xe8, 0xc2);
        }
        else
        {
            if (MemoryFullFlag == FALSE)
            {
                uiOSDIconColorByXY(OSD_ICON_SD, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
            }
        }
    }
}




void osdDrawSDCD(u8 i)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    return;
    if (Main_Init_Ready == 0)
        return;

    if(i == 1)
    {
        uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
        osdDrawMessage(MSG_SD_INIT, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
    }
    else
        osdDrawMessage(MSG_SD_INIT, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
}

void osdDrawFillEmpty(void)
{
    u16 LOC_X [2] = {UI_OSD_SD_FULL_LOC_X, UI_OSD_SD_FULL_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_SD_FULL_LOC_Y, UI_OSD_SD_FULL_LOC_TV_Y };

   (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    
    if (MyHandler.MenuMode == QUAD_MODE)
        LOC_X[0] = 392;

    uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,(OSDIconMidX-8), (OSDIconMidY-22), OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    uiOSDMultiLanguageStrCenter(MSG_NO_FILE, OSD_Blk1 , 0xC0, 0xC1);
    uiOSDASCIIStringByColor("0000/0000", (OSDIconEndX-80), (OSDIconEndY-20), OSD_BLK[sysTVOutOnFlag], 0xc0, 0x00);    
    if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_USBMASS)// Insert USB 
        uiOSDIconColorByXY(OSD_ICON_HDD ,LOC_X [sysTVOutOnFlag], LOC_Y [sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    else
        uiOSDIconColorByXY(OSD_ICON_SD ,LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);        
}

void osdDrawFSWaitingBar(u32 CurrFile, u32 TotalFile, u8 buf_idx, u16 y_pos, u8 clean)
{
    u16  Bar_len, Curr_len;

    Bar_len = (OSDDispWidth[sysTVOutOnFlag]-8*2)/4-2;
    Curr_len = CurrFile*Bar_len/TotalFile;
    if (clean == TRUE)
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , Bar_len*4 , 12 , 8+4, y_pos+4 , buf_idx , 0);

    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], (Bar_len*4+4*2), 4,  8,               y_pos,        buf_idx, 0xC0C0C0C0);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], (Bar_len*4+4*2), 4,  8,               (y_pos+4+12), buf_idx, 0xC0C0C0C0);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4,               12, 8,               (y_pos+4),    buf_idx, 0xC0C0C0C0);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4,               12, (8+Bar_len*4+4), (y_pos+4),    buf_idx, 0xC0C0C0C0);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], Curr_len*4,      12, 8+4,             y_pos+4 ,     buf_idx, 0xC6C6C6C6);

}

void osdDrawISPStatus(s8 status)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};

    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    osdDrawPreviewIcon();
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,(OSDIconMidX-8), (OSDIconMidY-32), OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);

    if (status == 1)
        osdDrawMessage(MSG_UPDATE_PASS, CENTERED, 116, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x85);
    else
        osdDrawMessage(MSG_UPDATE_FAIL, CENTERED, 116, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x85);

}

void osdDrawISPNow(void)
{
    (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk2, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_Blk2);
    osdDrawPreviewIcon();
    uiOsdEnable(OSD_Blk2);
}

void uiOsdDrawInsertSD(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    UI_MULT_ICON *iconInfo;

    if (act == UI_OSD_DRAW)
    {
        uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,(OSDIconMidX - 16), (OSDIconMidY + 125), OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_USBMASS)// Insert USB 
            uiOSDASCIIStringByColor("Insert HDD", OSDIconMidX - 80, OSDIconMidY + 180 + osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
        else
            uiOSDASCIIStringByColor("Insert SD Card", OSDIconMidX - 112, OSDIconMidY + 180+ osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
    }
    else
    {
        uiOsdGetIconInfo(OSD_ICON_WARNING_1,&iconInfo);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], iconInfo->Icon_w, iconInfo->Icon_h, (OSDIconMidX - 16), (OSDIconMidY + 125), OSD_BLK[sysTVOutOnFlag], 0);
        if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_USBMASS)// Insert USB 
            uiOSDASCIIStringByColor("Insert HDD", OSDIconMidX-80, OSDIconMidY + 180 + osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
        else
            uiOSDASCIIStringByColor("Insert SD Card", OSDIconMidX-112, OSDIconMidY + 180 + osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
    }
}

void uiOsdDrawInsertHDD(u8 act)
{
    s8  showMsg[7];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 x_pos;
    u16 y_pos = 480;
    u8 len;
    
    uiOsdDrawInit();

    len = (sizeof(showMsg) / sizeof(showMsg[0]));
    x_pos = (OSDIconEndX - (len* uiOsdGetFontWidth(CurrLanguage)))>>1;
    sprintf(showMsg, "Insert HDD");

    if (act == UI_OSD_DRAW)
    {
        uiOSDASCIIStringByColor(showMsg, x_pos, y_pos, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    }
    else
    {
        uiOSDASCIIStringByColor(showMsg, x_pos, y_pos, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
    }
}

void uiOsdDrawBitRate(u32 value)
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
	//const u8 TX3RX1TABLE[5]={1,3, 7,10,13};
	//const u8 TX4RX1TABLE[5]={1,2, 5, 7,10};
    u8 i,j;

    /*4T2R */
    #if( (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) )
        if(sysRFRxInMainCHsel %2 ==0)  /* CH1 & CH3 */
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

    /* 4T1R */
    #elif ((SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1))
        for(i=0;i<4;i++)
        {
            if(iconflag[UI_MENU_SETIDX_CH1_ON+i]==UI_MENU_SETTING_CAMERA_ON)
            {
                camera++;
            }
        }
    /* 1T1R */
    #else
        camera = 1;
    #endif
       
        if(camera == 0)
        {
            camera = 1;
        }
		if(value != 0)
		{
			switch(camera)
			{
				case 1:
					for(i = 0; i<5; i++)
					{
						if(value >= TX1RX1TABLE[i])
							signal = i;
					}
					break;

				case 2:
					for(i = 0; i<5; i++)
					{
						if(value >= TX2RX1TABLE[i])
							signal = i;
					}
					break;

				default:
					for(i = 0; i < 5; i++)
					{
						if(value >= TX1RX1TABLE[i])
							signal = i;
					}
					break;
			}
	
		}
		
        //uiOsdDrawCamera();//for demo

        sprintf ((char *)bitRateStr, "B:%d.%d", (value/10), (value%10));

        uiOsdDrawAtenna(signal);
        
}

void uiOsdDrawFrameRate(u32 value)
{
    u8 FrameRateStr[6];

    sprintf ((char *)FrameRateStr, "F:%2d", value);
    #if (HW_BOARD_OPTION == MR8200_RX_ZINWELL)
    uiOSDASCIIStringByColor(FrameRateStr, 20, 200, OSD_Blk2, 0xc6, 0x00);
    #endif
}

#if UI_LIGHT_SUPPORT
void uiOsdDrawLight(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};

    if ((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE))
        return;
    #if 0
    if(act == UI_OSD_DRAW)
        uiOSDIconColorByXYChColor(OSD_ICON_LIGHTING_SETUP_LIGHT, UI_OSD_LIGHT_LOC_X, UI_OSD_LIGHT_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3, 0xe8, 0xC2);
    else
    {
        uiOSDIconColorByXY(OSD_ICON_LIGHTING_SETUP_LIGHT, UI_OSD_LIGHT_LOC_X, UI_OSD_LIGHT_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
    }
    #endif
}

void uiOsdDrawLightManual(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 i,DrawX,DrawY;

    if (MyHandler.MenuMode != VIDEO_MODE)
        return;

    if (act != UI_OSD_NONE)
    {
        if (iconflag[UI_MENU_SETIDX_CH1_LS_ONOFF+sysRFRxInMainCHsel] == UI_MENU_CAMERA_ALARM_OFF)
            return;
    }

    if (uiLightTest)
        DEBUG_GREEN("uiOsdDrawLightManual cam %d value %d \n",sysRFRxInMainCHsel,iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+sysRFRxInMainCHsel]);
    
    if(act == UI_OSD_DRAW)
        uiOSDIconColorByXYChColor(OSD_ICON_LIGHTING_SETUP_LIGHT_MANUAL, UI_OSD_LIGHT_LOC_X, UI_OSD_LIGHT_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3, 0xe8, 0xC2);
    else if (act == UI_OSD_CLEAR)
        uiOSDIconColorByXY(OSD_ICON_LIGHTING_SETUP_LIGHT_MANUAL, UI_OSD_LIGHT_LOC_X, UI_OSD_LIGHT_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
    else
        uiOSDIconColorByXY(OSD_ICON_LIGHTING_SETUP_LIGHT_MANUAL, UI_OSD_LIGHT_LOC_X, UI_OSD_LIGHT_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_0);

}

void uiOsdDrawQuadLightManual(u8 cam,u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16  i,DrawX,DrawY;

    if (MyHandler.MenuMode != QUAD_MODE)
        return;

    if (act != UI_OSD_NONE)
    {
        if (iconflag[UI_MENU_SETIDX_CH1_LS_ONOFF+cam] == UI_MENU_CAMERA_ALARM_OFF)
            return;
    }

    if (uiLightTest)
        DEBUG_GREEN("uiOsdDrawQuadLightManual cam %d value %d \n",cam,iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+cam]);
    
    if (uiQuadDisplay == 1)
    {
        if ((cam%2) == 0)
            DrawX = UI_OSD_LIGHT_QUAD_LOC_X;
        else
            DrawX = (UI_OSD_LIGHT_QUAD_LOC_X+OSDIconMidX);
        if (cam < 2)
            DrawY = UI_OSD_LIGHT_QUAD_LOC_Y;
        else
            DrawY = (UI_OSD_LIGHT_QUAD_LOC_Y+OSDIconMidY);

    }
    else
    {
        if (iconflag[UI_MENU_SETIDX_CH1_ON+cam] == UI_MENU_SETTING_CAMERA_OFF)
            return;
        
        DrawY = UI_OSD_LIGHT_QUAD_LOC_Y;
        for (i = 0; i < 4; i++)
        {
            if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
            {
                if (i==cam)
                    DrawX = UI_OSD_LIGHT_QUAD_LOC_X;
                else
                    DrawX = (UI_OSD_LIGHT_QUAD_LOC_X+OSDIconMidX);
                break;                        
            }
        }
        
    }
    
    if(act == UI_OSD_DRAW)
        uiOSDIconColorByXYChColor(OSD_ICON_LIGHTING_SETUP_LIGHT_MANUAL, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3, 0xe8, 0xC2);
    else if (act == UI_OSD_CLEAR)
        uiOSDIconColorByXY(OSD_ICON_LIGHTING_SETUP_LIGHT_MANUAL, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
    else
        uiOSDIconColorByXY(OSD_ICON_LIGHTING_SETUP_LIGHT_MANUAL, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_0);
}

void uiOsdDrawLightIcon(void)
{
    u8 i;
    
    if (MyHandler.MenuMode == VIDEO_MODE)
    {
        if (uiSetRfLightState[sysRFRxInMainCHsel] == UI_SET_RF_OK)
        {
            if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+sysRFRxInMainCHsel] == UI_LIGHT_MANUAL_ON)
                uiOsdDrawLightManual(UI_OSD_DRAW);
            else if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+sysRFRxInMainCHsel] == UI_LIGHT_TRIGGER_ON)
                uiOsdDrawLightManual(UI_OSD_DRAW);
            else if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+sysRFRxInMainCHsel] == UI_LIGHT_MANUAL_OFF)
                uiOsdDrawLightManual(UI_OSD_CLEAR);
            else if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+sysRFRxInMainCHsel] == UI_LIGHT_OFF)
                uiOsdDrawLightManual(UI_OSD_CLEAR);
        }
        else
        {
            uiOsdDrawLightManual(UI_OSD_CLEAR);
        }    
    }
    else if (MyHandler.MenuMode == QUAD_MODE)
    {
        for (i = 0; i < 4; i++)
        {
            if (uiSetRfLightState[i] == UI_SET_RF_OK)
            {
                if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+i] == UI_LIGHT_MANUAL_ON)
                    uiOsdDrawQuadLightManual(i,UI_OSD_DRAW);
                else if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+i] == UI_LIGHT_TRIGGER_ON)
                    uiOsdDrawQuadLightManual(i,UI_OSD_DRAW);
                else if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+i] == UI_LIGHT_MANUAL_OFF)
                    uiOsdDrawQuadLightManual(i,UI_OSD_CLEAR);
                else if (iconflag[UI_MENU_SETIDX_CH1_LS_STATUS+i] == UI_LIGHT_OFF)
                    uiOsdDrawQuadLightManual(i,UI_OSD_CLEAR);
            }
            else
            {
                uiOsdDrawQuadLightManual(i,UI_OSD_CLEAR);
            }
        }
    }
}

void uiOsdDrawLightApp(u8 camID)
{
    if (iconflag[UI_MENU_SETIDX_CH1_LS_ONOFF+camID] == UI_MENU_CAMERA_ALARM_ON)
        uiOsdDrawLightIcon();
    else
    {
        if (MyHandler.MenuMode == VIDEO_MODE)
            uiOsdDrawLightManual(UI_OSD_NONE);
        else
            uiOsdDrawQuadLightManual(camID,UI_OSD_NONE);
    }
}
#endif

#if UI_CAMERA_ALARM_SUPPORT
void uiOsdDrawCamreaAlarmManual(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16  i,DrawX,DrawY;

    if (MyHandler.MenuMode != VIDEO_MODE)
        return;

    if (act != UI_OSD_NONE)
    {
        if (iconflag[UI_MENU_SETIDX_CH1_CA_ONOFF+sysRFRxInMainCHsel] == UI_MENU_CAMERA_ALARM_OFF)
            return;
    }

    if (uiLightTest)
        DEBUG_GREEN("uiOsdDrawCamreaAlarmManual cam %d value %d \n",sysRFRxInMainCHsel,iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+sysRFRxInMainCHsel]);

    if(act == UI_OSD_DRAW)
        uiOSDIconColorByXYChColor(OSD_ICON_MOTION_ALARM_MANUAL, UI_OSD_ALARM_LOC_X, UI_OSD_ALARM_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3, 0xe8, 0xC2);
    else if (act == UI_OSD_CLEAR)
        uiOSDIconColorByXY(OSD_ICON_MOTION_ALARM_MANUAL, UI_OSD_ALARM_LOC_X, UI_OSD_ALARM_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
    else
        uiOSDIconColorByXY(OSD_ICON_MOTION_ALARM_MANUAL, UI_OSD_ALARM_LOC_X, UI_OSD_ALARM_LOC_Y, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_0);

}

void uiOsdDrawQuadCamreaAlarmManual(u8 cam,u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16  i,DrawX,DrawY;

    if (MyHandler.MenuMode != QUAD_MODE)
        return;
    
    if (act != UI_OSD_NONE)
    {
        if (iconflag[UI_MENU_SETIDX_CH1_CA_ONOFF+cam] == UI_MENU_CAMERA_ALARM_OFF)
            return;
    }
    
    if (uiLightTest)
        DEBUG_GREEN("uiOsdDrawQuadCamreaAlarmManual cam %d value %d \n",cam,iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+cam]);

    if (uiQuadDisplay == 1)
    {
        if ((cam%2) == 0)
            DrawX = UI_OSD_ALARM_QUAD_LOC_X;
        else
            DrawX = (UI_OSD_ALARM_QUAD_LOC_X+OSDIconMidX);
        if (cam < 2)
            DrawY = UI_OSD_ALARM_QUAD_LOC_Y;
        else
            DrawY = (UI_OSD_ALARM_QUAD_LOC_Y+OSDIconMidY);
        
    }
    else
    {
        if (iconflag[UI_MENU_SETIDX_CH1_ON+cam] == UI_MENU_SETTING_CAMERA_OFF)
            return;
        
        DrawY = UI_OSD_ALARM_QUAD_LOC_Y;
        for (i = 0; i < 4; i++)
        {
            if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
            {
                if (i==cam)
                    DrawX = UI_OSD_ALARM_QUAD_LOC_X;
                else
                    DrawX = (UI_OSD_ALARM_QUAD_LOC_X+OSDIconMidX);
                break;                        
            }
        }
    
    }
    
    if(act == UI_OSD_DRAW)
        uiOSDIconColorByXYChColor(OSD_ICON_MOTION_ALARM_MANUAL, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3, 0xe8, 0xC2);
    else if (act == UI_OSD_CLEAR)
        uiOSDIconColorByXY(OSD_ICON_MOTION_ALARM_MANUAL, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_3);
    else
        uiOSDIconColorByXY(OSD_ICON_MOTION_ALARM_MANUAL, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag], 0x00, alpha_0);
}

void uiOsdDrawAlarmIcon(void)
{
    u8 i;
    
    if (MyHandler.MenuMode == VIDEO_MODE)
    {
        if (uiSetRfAlarmState[sysRFRxInMainCHsel] == UI_SET_RF_OK)
        {
            if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+sysRFRxInMainCHsel] == UI_CAMERA_ALARM_MANUAL_ON)
                uiOsdDrawCamreaAlarmManual(UI_OSD_DRAW);
            else if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+sysRFRxInMainCHsel] == UI_CAMERA_ALARM_MANUAL_OFF)
                uiOsdDrawCamreaAlarmManual(UI_OSD_CLEAR);
            else if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+sysRFRxInMainCHsel] == UI_CAMERA_ALARM_TRIGGER_ON)
                uiOsdDrawCamreaAlarmManual(UI_OSD_DRAW);
            else if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+sysRFRxInMainCHsel] == UI_CAMERA_ALARM_OFF)
                uiOsdDrawCamreaAlarmManual(UI_OSD_CLEAR);
        }
        else
        {
            uiOsdDrawCamreaAlarmManual(UI_OSD_CLEAR);
        }
    }
    else if (MyHandler.MenuMode == QUAD_MODE)
    {
        for (i = 0; i < 4; i++)
        {
            if (uiSetRfAlarmState[i] == UI_SET_RF_OK)
            {
                if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+i] == UI_CAMERA_ALARM_MANUAL_ON)
                    uiOsdDrawQuadCamreaAlarmManual(i,UI_OSD_DRAW);
                else if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+i] == UI_CAMERA_ALARM_MANUAL_OFF)
                    uiOsdDrawQuadCamreaAlarmManual(i,UI_OSD_CLEAR);
                else if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+i] == UI_CAMERA_ALARM_TRIGGER_ON)
                    uiOsdDrawQuadCamreaAlarmManual(i,UI_OSD_DRAW);
                else if (iconflag[UI_MENU_SETIDX_CH1_CA_STATUS+i] == UI_CAMERA_ALARM_OFF)
                    uiOsdDrawQuadCamreaAlarmManual(i,UI_OSD_CLEAR);
            
            }
            else
            {
                uiOsdDrawQuadCamreaAlarmManual(i,UI_OSD_CLEAR);
            }
        }    
    }
}

void uiOsdDrawAlarmApp(u8 camID)
{
    if (iconflag[UI_MENU_SETIDX_CH1_CA_ONOFF+camID] == UI_MENU_CAMERA_ALARM_ON)
        uiOsdDrawAlarmIcon();
    else
    {
        if (MyHandler.MenuMode == VIDEO_MODE)
            uiOsdDrawCamreaAlarmManual(UI_OSD_NONE);
        else
            uiOsdDrawQuadCamreaAlarmManual(camID,UI_OSD_NONE);
    }
}
#endif

void uiOSDInit(void)
{
    #if (UI_PREVIEW_OSD == 0)
        return;
    #endif
    #if (UI_PREVIEW_OSD == 1)
        if (sysTVOutOnFlag)
        {
            uiMenuOSDReset();
            iduTVOSDDisplay(2, 0, 0, TVOSD_SizeX, TVOSD_SizeY);
            uiOsdEnable(2);
        }
        else
        {
            uiMenuOSDReset();
            iduOSDDisplay1(2, 0, 0, PANNEL_X, PANNEL_Y);
            osdDrawPreviewIcon();
            uiOsdEnable(2);

        }
    #endif
}

void uiOSDPreviewInit(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    #if (UI_PREVIEW_OSD == 1)
        if (sysTVOutOnFlag)
        {
            uiMenuOSDReset();
            iduTVOSDDisplay(OSD_BLK[sysTVOutOnFlag], 0, 0, TVOSD_SizeX, TVOSD_SizeY);           
            if (MyHandler.MenuMode == QUAD_MODE)
                osdDrawQuadIcon();
            else
                osdDrawPreviewIcon();
            uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
        }
        else
        {
            uiMenuOSDReset();
            iduOSDDisplay1(OSD_BLK[sysTVOutOnFlag], 0, 0, PANNEL_X, PANNEL_Y);
            if (MyHandler.MenuMode == QUAD_MODE)
                osdDrawQuadIcon();
            else
                osdDrawPreviewIcon();
            uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);

        }
    #endif
}

void osdDrawLiveViewTalkFrame(void)
{
    if (uiCheckRfTalkStatus() == 1)
    {
        //draw sysRFRxInMainCHsel frame
    }
}

void osdDrawPreviewIcon(void)
{
#if (UI_PREVIEW_OSD == 0)
    return;
#endif

    if (Main_Init_Ready == 0)
        return;

    if(MyHandler.MenuMode!=VIDEO_MODE)
        return;
    
    osdDrawMenuPreview();
    osdDrawLCDPreview();

    if (iconflag[UI_MENU_SETIDX_SET_AREC] & (0x01 << sysRFRxInMainCHsel))
    {
        osdDrawMotionPreview(UI_OSD_DRAW);
    }
    else
    {
        osdDrawMotionPreview(UI_OSD_CLEAR);
    }

    if (isManualRec)
        osdDrawRecPreview(UI_OSD_CLEAR);
    else
        osdDrawRecPreview(UI_OSD_DRAW);
    osdDrawVol();
    uiOsdDrawTalkBack(UI_OSD_CLEAR);
    osdDrawQuadPreview();

    if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
        osdDrawSDIcon(UI_OSD_DRAW);

    if ((SysOverwriteFlag == FALSE) && (MemoryFullFlag == TRUE))
        osdDrawMemFull(UI_OSD_DRAW);

    //uiosdDrawResolution();
    uiOsdDrawCamera(sysRFRxInMainCHsel, VIDEO_MODE, UI_OSD_DRAW);
#if MULTI_CHANNEL_VIDEO_REC
    osdDrawVideoOn(UI_OSD_NONE);
#endif
#if (NIC_SUPPORT)
    if (uiP2PMode == 0)
        osdDrawRemoteOn(UI_OSD_CLEAR);
    else
        osdDrawRemoteOn(UI_OSD_DRAW);

    if (Check_fw_ver_net(Check_by_local) == 1)
        uiOsdDrawRemindDownload(UI_OSD_DRAW);
#endif

#if UI_LIGHT_SUPPORT
    uiOsdDrawLightIcon();
#endif

#if UI_CAMERA_ALARM_SUPPORT
    uiOsdDrawAlarmIcon();
#endif

    uiOsdDrawBattery(batteryflag);
#if(UI_BAT_SUPPORT)
    osdDrawCamLiveView(sysRFRxInMainCHsel);
#endif
    if ((uiCheckRfTalkStatus()== 1) && (gRfiu_Op_Sta[sysRFRxInMainCHsel] == RFIU_RX_STA_LINK_OK))
    {
        uiOsdDrawTalkBack(UI_OSD_DRAW);
    }

}

#if (RFIU_SUPPORT)
void uiOsdDrawChangeResolution(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    //memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM); 
    Idu_ClearBuf(6);

    if (act == UI_OSD_DRAW)
    {
        uiMenuOSDReset();
        (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
        uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],OSDDispWidth[sysTVOutOnFlag],OSDDispHeight[sysTVOutOnFlag],0,0, OSD_BLK[sysTVOutOnFlag],0xC1C1C1C1);
        osdDrawMessage(MSG_CHANGE_RESOLUTION, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
        osdDrawPreviewIcon();    
    }
    else
    {
        uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
        osdDrawPreviewIcon();    
    }
     
}

#if 0
void osdDrawQuadFrameBG(void)
{
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32  LenCol=0Xdcdcdcdc;

    if(MyHandler.MenuMode !=QUAD_MODE)
        return;
    
    if(uiQuadDisplay == 1)  //quad mode
    {
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], OSDIconEndX, 4, 0, OSDIconMidY, OSD_BLK[sysTVOutOnFlag], LenCol);  
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, OSDIconEndY, OSDIconMidX, 0, OSD_BLK[sysTVOutOnFlag], LenCol); 
    }
    else  //half mode
    {

        //uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], OSDIconEndX, 4, 0, 80, OSD_BLK[sysTVOutOnFlag], LenCol);  
        //uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], OSDIconEndX, 4, 0, 400, OSD_BLK[sysTVOutOnFlag], LenCol); 
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, 600, OSDIconMidX, 0, OSD_BLK[sysTVOutOnFlag], LenCol); 
    }
}
#endif

void osdDrawQuadFrame(u8 channel,u8 act)
{
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32  LenCol=0;
    u32  FrameW=OSDIconMidX-4, FrameH=OSDIconMidY-4;
    u32  StartX=0, StartY=0;
    u8   i, right=0;

    if(MyHandler.MenuMode !=QUAD_MODE)
        return;
    
    if(act == UI_OSD_DRAW) 
    {
        LenCol=0xC3C3C3C3;    
    }
    else  
    {
        LenCol=0;            
    }
    for (i = UI_MENU_CHANNEL_1; i < UI_MENU_CHANNEL_4+1; i++)
    {
        if ((i > channel)&&(iconflag[UI_MENU_SETIDX_CH1_ON+(i-UI_MENU_CHANNEL_1)] == UI_MENU_SETTING_CAMERA_ON))
            right = 1;    
    }
    
    if(uiQuadDisplay == 1)  //quad
    {
        switch(channel)
        {          
            case UI_MENU_CHANNEL_1:
                StartX=0;
                StartY=0;
                break;
                
            case UI_MENU_CHANNEL_2:
                StartX=OSDIconMidX;
                StartY=0;
                break;
                
            case UI_MENU_CHANNEL_3:
                StartX=0;
                StartY=OSDIconMidY;
                break;
                
            case UI_MENU_CHANNEL_4:
                StartX=OSDIconMidX;
                StartY=OSDIconMidY;
                break;

            default:
                break;
        }
    
    }
    else  //half
    {
        FrameH=OSDIconEndY;
        StartY=0;
        if( (right == 1) || (uiQuadDisplay==0))  // draw on left 
        {
            StartX=0;               
        }
        else
        {
            StartX=OSDIconMidX;    
        }       
        
    }
    
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, FrameH+4, StartX, StartY, OSD_BLK[sysTVOutOnFlag], LenCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, FrameH+4, StartX+FrameW, StartY, OSD_BLK[sysTVOutOnFlag], LenCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], FrameW, 4, StartX, StartY, OSD_BLK[sysTVOutOnFlag], LenCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], FrameW, 4, StartX, StartY+FrameH, OSD_BLK[sysTVOutOnFlag], LenCol);
    
    
}

void osdDrawQuadIcon(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u8 i,checkcam=0;

    if (Main_Init_Ready == 0)
        return;
    
    if(MyHandler.MenuMode!=QUAD_MODE)
        return;

    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    osdDrawMenuPreview();
    osdDrawLCDPreview();

    if (isManualRec)
        osdDrawRecPreview(UI_OSD_CLEAR);
    else
        osdDrawRecPreview(UI_OSD_DRAW);
    osdDrawQuadPreview();
    
    for (i = 0; i < 4; i++)
    {
        #if(UI_VERSION == UI_VERSION_TRANWO)
            uiOsdDrawCamera(i, QUAD_MODE,0);
        #else
            uiOsdDrawCamera(i, QUAD_MODE, UI_OSD_DRAW);
        #endif
    }    

    if(sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_READY)
        osdDrawSDIcon(UI_OSD_DRAW);
    
    if ((SysOverwriteFlag == FALSE) && (MemoryFullFlag == TRUE))
        osdDrawMemFull(UI_OSD_DRAW);
    
#if (NIC_SUPPORT)
    if (uiP2PMode == 0)
        osdDrawRemoteOn(UI_OSD_CLEAR);
    else
        osdDrawRemoteOn(UI_OSD_DRAW);

    if (Check_fw_ver_net(Check_by_local) == 1)
        uiOsdDrawRemindDownload(UI_OSD_DRAW);
#endif

#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    for (i = 0; i < 4; i++)
    {
        #if 0
        waitFlag = (FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 << i);
        if (OSFlagAccept(gRfRxVideoPackerSubReadyFlagGrp, waitFlag, OS_FLAG_WAIT_CLR_ANY, &err) >0)
        {
            osdDrawQuadVideoOn(i,UI_OSD_DRAW);
        }
        else
            #endif
        {
            osdDrawQuadVideoOn(i,UI_OSD_NONE);
        }
    }
#endif

#if UI_LIGHT_SUPPORT
    uiOsdDrawLightIcon();
#endif

#if UI_CAMERA_ALARM_SUPPORT
    uiOsdDrawAlarmIcon();
#endif

    uiOsdDrawBattery(batteryflag);
#if(UI_BAT_SUPPORT)
    for (i = 0; i < 4; i++)
    {
        osdDrawCamLiveView(i);
    }
#endif    
    osdDrawLiveViewTalkFrame();
    
}

void osdDrawRemoteOn(u8 on)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    UI_MULT_ICON *iconInfo;
    u16 x_pos = UI_OSD_REMOTE_LOC_X, y_pos =UI_OSD_REMOTE_LOC_Y ;
    int LinkStatus;
    
    if((MyHandler.MenuMode != QUAD_MODE) && (MyHandler.MenuMode != VIDEO_MODE))
        return;
    
    if (MyHandler.MenuMode == QUAD_MODE)
        x_pos = 308;

    uiOsdGetIconInfo(OSD_ICON_NET_LINK_UP,&iconInfo);
    switch(on)
    {
        case UI_OSD_CLEAR:
            LinkStatus = Get_network_status();
            uiOsdDrawNetworkLink((u8)LinkStatus);
            break;

        case UI_OSD_DRAW:
#if APP_KEEP_ALIVE
            APPConnectIcon = 1;
#endif
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], iconInfo->Icon_w, iconInfo->Icon_h, x_pos, y_pos, OSD_BLK[sysTVOutOnFlag], 0x00);
            uiOSDIconColorByXY(OSD_ICON_REMOTE ,x_pos + 8 , y_pos , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
            break;
    }
}

void uiOsdDrawQuadNoSignal(u8 act, u8 Camid)
{
    u8 i,right = 0;
    u32 DrawColor = 0;

#if 0
    if (act == UI_OSD_DRAW)
    {
        uiGraphDrawQuadNoSignal(Camid);
        return;

    }
#endif

    if (act == UI_OSD_DRAW)
        DrawColor = 0xC1C1C1C1;
    else
        DrawColor = 0;

    for (i = 0; i < 4; i++)
    {        
        if ((i > Camid)&&(iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON))
            right = 1;
    }
    
    if (uiQuadDisplay !=1 ) /*half mode*/
    {       
        if ((right == 1)|| (uiQuadDisplay == 0))  /*camera in left*/
            Camid = 4;
        else
            Camid = 5;        
    }
    //uiClearFfQuadBuf(Camid);
    #if (1)
    #if(UI_BAT_SUPPORT)
    if (act == UI_OSD_DRAW)
    {
        for (i = 0; i < 4; i++)
        {
            osdDrawCamLiveView(i);
        }
    }
    #endif
    #else
    if (act == UI_OSD_CLEAR)
        osdDrawQuadIcon();
    #if(UI_BAT_SUPPORT)
    else
    {
        for (i = 0; i < 4; i++)
        {
            osdDrawCamLiveView(i);
        }
    }
    #endif
    #endif
}
#endif

void uiOsdDrawBattery(u8 level)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 LOC_X [2] = {UI_OSD_BATTERY_LOC_X, UI_OSD_BATTERY_LOC_TV_X };
    u16 LOC_Y [2] = {UI_OSD_BATTERY_LOC_Y, UI_OSD_BATTERY_LOC_TV_Y };
    u8  bg_color=alpha_3;
    u16 osd_idx=0;
    
    if(level == UI_BATTERY_CLEAR)
    {
        osd_idx=OSD_ICON_BATTERY_LV0;
        bg_color=alpha_0;    
    }
    else if(level == UI_BATTERY_CHARGE)
    {
        osd_idx=OSD_ICON_BATTERY_CHARGE;
        bg_color=alpha_3;
    }
    else
    {
        osd_idx=OSD_ICON_BATTERY_LV0+level;
        bg_color=alpha_3;
    }
    //uiOSDIconColorByXY(osd_idx, LOC_X[sysTVOutOnFlag], LOC_Y[sysTVOutOnFlag], OSD_BLK[sysTVOutOnFlag], 0x00, bg_color);
}

void uiOsdDrawNewFile(void)

{
}
void uiOsdDrawSDCardFail(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    UI_MULT_ICON *iconInfo;

    if (Main_Init_Ready == 0)
        return;
    
    if (!((MyHandler.MenuMode == VIDEO_MODE) || (MyHandler.MenuMode == QUAD_MODE)))
        return;

    if (act == UI_OSD_DRAW)
    {
        if (gUishowFailTime == 0)
            gUishowFailTime = 10;
        DEBUG_UI("######### uiOsdDrawSDCardFail ##########\r\n");
        uiDrawSDFail = UI_OSD_DRAW;
        uiOSDIconColorByXY(OSD_ICON_WARNING_1 ,(OSDIconMidX - 16), (OSDIconMidY + 125), OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_USBMASS)// Insert USB 
            uiOSDASCIIStringByColor("HDD error retry please", OSDIconMidX - 176, OSDIconMidY + 180 + osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0xC2, 0x00);
        else
            uiOSDASCIIStringByColor("Card error retry please", OSDIconMidX - 184, OSDIconMidY + 180 + osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0xC2, 0x00);
    }
    else
    {
        gUishowFailTime = 0;
        uiDrawSDFail = UI_OSD_CLEAR;
        uiOsdGetIconInfo(OSD_ICON_WARNING_1,&iconInfo);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], iconInfo->Icon_w, iconInfo->Icon_h, (OSDIconMidX - 16), (OSDIconMidY + 125), OSD_BLK[sysTVOutOnFlag], 0);
        if(sysGetStorageSel(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_USBMASS)// Insert USB 
            uiOSDASCIIStringByColor("HDD error retry please", OSDIconMidX - 176, OSDIconMidY + 180 + osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
        else
            uiOSDASCIIStringByColor("Card error retry please", OSDIconMidX - 184, OSDIconMidY + 180 + osdYShift/2, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
    }
}

void uiOsdDrawPosition(int X, int Y)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};  
    u8  str[15]={0};

    sprintf(str,"(%03d,%03d)\n",X,Y);

    uiOSDASCIIStringByColor(str, 56, 100,  OSD_BLK[sysTVOutOnFlag], 0xc4, 0x00);
    
}

void uiOsdDrawPowerOff(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
   
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    osdDrawMessage(MSG_POWER_OFF, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);    
}

void uiOsdDrawStorageNReady(u8 act)
{
    u8  str[17]={0};
    u16  drawX,drawY;
    u8  len;
    
    if ((MyHandler.MenuMode != VIDEO_MODE) && (MyHandler.MenuMode != QUAD_MODE))
        return;
    
    sprintf(str,"HDD Init Wait...");

    len = (sizeof(str) / sizeof(str[0]));
    drawX = (OSDIconEndX - (len* uiOsdGetFontWidth(CurrLanguage)))>>1;
    drawY = OSDIconMidY + 120;
        
    if (act == UI_OSD_DRAW)
        uiOSDASCIIStringByColor(str, drawX, drawY, OSD_BLK[sysTVOutOnFlag], 0xc2, 0x00);
    else
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], (len * uiOsdGetFontWidth(CurrLanguage)), uiOsdGetFontHeight(UI_MULT_LANU_EN), drawX, drawY, OSD_BLK[sysTVOutOnFlag], 0);
}

#if USB_HOST_MASS_SUPPORT
void osdDrawNotHDD(u8 act)
{
    s8  showMsg[7];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 x_pos;
    u16 y_pos = 480;
    u8 len;
    
    uiOsdDrawInit();

    len = (sizeof(showMsg) / sizeof(showMsg[0]));
    x_pos = (OSDIconEndX - (len* uiOsdGetFontWidth(CurrLanguage)))>>1;
    sprintf(showMsg, "Not HDD");
    
    if (act == UI_OSD_DRAW)
        uiOSDASCIIStringByColor(showMsg, x_pos, y_pos, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    else
    {
        uiOsdDisable(OSD_BLK[sysTVOutOnFlag]);
    }
}

void osdDrawHDDUninstallStatusMsg(u8 act, u8 status)
{
    static s8 showMsg[20];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 x_pos;
    u16 y_pos = 480;
    u8 len;
    
    uiOsdDrawInit();

    switch(status)
    {
        case 0:         /* Failed*/
            DEBUG_UI("Remove HDD Failed\r\n");
            sprintf(showMsg, "HDD removal failed");
            break;

        case 1:         /* Successed*/
            DEBUG_UI("Remove HDD Safely\r\n");
            sprintf(showMsg, "HDD Safely Removed");
            break;

        case 2:         /*Clear Last Msg*/
            DEBUG_UI("Clear Last HDD Msg\r\n");
            //sprintf(showMsg, "000000000000000000000000");
            break;
    }
    
    len = (sizeof(showMsg) / sizeof(showMsg[0]));
    x_pos = (OSDIconEndX - (len* uiOsdGetFontWidth(CurrLanguage)))>>1;

    if (act == UI_OSD_DRAW)
        uiOSDASCIIStringByColor(showMsg, x_pos, y_pos, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    else
    {
        uiOsdDisable(OSD_BLK[sysTVOutOnFlag]);
    }
}

void osdDrawHDDUninstallMsg(u8 act)
{
    s8  showMsg[20];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u16 x_pos;
    u16 y_pos = 480;
    u8 len;
    
    uiOsdDrawInit();

    len = (sizeof(showMsg) / sizeof(showMsg[0]));
    x_pos = (OSDIconEndX - (len* uiOsdGetFontWidth(CurrLanguage)))>>1;
    sprintf(showMsg, "Remove HDD Wait...");
    
    if (act == UI_OSD_DRAW)
    {
        uiOSDASCIIStringByColor(showMsg, x_pos, y_pos, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
    }
    else
        uiOSDASCIIStringByColor(showMsg, x_pos, y_pos, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
}
#endif

void osdDrawClearRemoteMsg(u8 act)
{
    if (act == UI_OSD_DRAW)
        uiOSDASCIIStringByColorCenter((u8*)"Remote device is busy",OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
    else
        uiOSDASCIIStringByColorCenter((u8*)"Remote device is busy",OSD_BLK[sysTVOutOnFlag] , 0x00, 0x00);
}

void osdDrawSchduleWarnMsg(u8 type, u8 act)
{
    static s8 showMsg[30];
    u16 x_pos = 376;
    u16 y_pos = 124;

    uiOsdDrawInit();

    switch(type)
    {
        case 0:         /* Camera */
            DEBUG_UI("Camera wasnt set up\r\n");
            sprintf(showMsg, "Camera wasnt set up");
            break;

        case 1:         /* Time */
            DEBUG_UI("Datetime wasnt set up\r\n");
            sprintf(showMsg, "Datetime wasnt set up");
            break;

    }

    if (act == UI_OSD_DRAW)
        uiOSDASCIIStringByColor(showMsg, x_pos, y_pos, OSD_BLK[sysTVOutOnFlag], 0xC2, 0x00);
    else
    {
        uiOsdDisable(OSD_BLK[sysTVOutOnFlag]);
    }
}

void uiOsdDrawAllPreviewIcon(void)
{
    u8  i,LinkStatus;
    u8  tmpVal;

    uiOsdDrawAtenna(4);
    for (i = 0; i < 4; i++)
    {
        #if(UI_VERSION == UI_VERSION_TRANWO)
        if(i == sysRFRxInMainCHsel )
        {
            uiOsdDrawCamera(i, QUAD_MODE,1);    
        }
        else
        {
            uiOsdDrawCamera(i, QUAD_MODE,0);
        }
        
        #else
        uiOsdDrawCamera(i, QUAD_MODE, UI_OSD_DRAW);
        #endif        
        osdDrawQuadVideoOn(i, UI_OSD_DRAW);
    }
    osdDrawSDIcon(UI_OSD_DRAW);
    uiOsdDrawTalkBack(UI_OSD_DRAW);
    tmpVal = iconflag[UI_MENU_SETIDX_OVERWRITE];
    iconflag[UI_MENU_SETIDX_OVERWRITE] = UI_MENU_SETTING_OVERWRITE_YES;
    uiOsdDrawOverwrit();
    iconflag[UI_MENU_SETIDX_OVERWRITE] = tmpVal;
    osdDrawMemFull(UI_OSD_DRAW);
    tmpVal = dcfNewFile;
    dcfNewFile = 1;
    uiOsdDrawNewFile();
    dcfNewFile = tmpVal;
    osdDrawShowZoom(UI_OSD_DRAW);
    osdDrawRemoteOn(UI_OSD_DRAW);
    uiOsdDrawBattery(UI_BATTERY_CHARGE);
    
    osdDrawMenuPreview();
    osdDrawLCDPreview();
    osdDrawMotionPreview(UI_OSD_CLEAR);
    osdDrawRecPreview(UI_OSD_CLEAR);
    osdDrawVol();
    osdDrawQuadPreview();
    
#if (NIC_SUPPORT)
    LinkStatus = Get_network_status();
    uiOsdDrawNetworkLink((u8)LinkStatus);

    uiOsdDrawRemindDownload(UI_OSD_DRAW);
#endif

#if UI_LIGHT_SUPPORT
    for (i = 0; i < 4; i++)
    {
        uiOsdDrawQuadLightManual(i,UI_OSD_DRAW);
    }
#endif

#if UI_CAMERA_ALARM_SUPPORT
    for (i = 0; i < 4; i++)
    {
        uiOsdDrawQuadCamreaAlarmManual(i,UI_OSD_DRAW);
    }
#endif
    uiOsdDrawStorageNReady(UI_OSD_DRAW);
}
