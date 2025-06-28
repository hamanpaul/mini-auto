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
#include "ui_menu_icon.h"
#include "rfiuapi.h"
#include "gpioapi.h"
#include "mpeg4api.h"
#include "GlobalVariable.h"

#if (IS_COMMAX_DOORPHONE) || (IS_HECHI_DOORPHONE) || (HW_BOARD_OPTION == MR6730_AFN)
    #include "MainFlow.h"
	#include "Menu.h"
#endif

/*
 *********************************************************************************************************
 *  Constant
 *********************************************************************************************************
 */
#if (HW_BOARD_OPTION == MR8200_RX_COMMAX)||(HW_BOARD_OPTION == MR9670_DEMO_BOARD)||\
    (HW_BOARD_OPTION == MR8200_RX_DB2)|| (HW_BOARD_OPTION == MR8200_RX_DB3) || (HW_BOARD_OPTION == MR6730_AFN)
#define USE_BIG_OSD    1
#else
#define USE_BIG_OSD    0
#endif

#if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
#undef USE_BIG_OSD
#define USE_BIG_OSD    1
#define UI_PLAYBACK_NUM_PER_PAGE        4
#define UI_PLAYBACK_LOC_MENU_Y          20
#define UI_PLAYBACK_LOC_MENU_FILE_X     4
#define UI_PLAYBACK_LOC_MENU_DIR_X      232
#define UI_PLAYBACK_LOC_MENU_DATE_X     240
#define UI_PLAYBACK_LOC_MENU_TIME_X     448
#define UI_PLAYBACK_LOC_MENU_FILE_Y     80
#define UI_PLAYBACK_LOC_PAGE_X          420
#define UI_PLAYBACK_LOC_PAGE_X_L        16
#define UI_PLAYBACK_LOC_PAGE_Y          400
#if IS_COMMAX_DOORPHONE
    #define UI_PLAYBACK_MENU_LEN            320
#else
    #define UI_PLAYBACK_MENU_LEN            400
#endif
#define UI_PLAYBACK_MENU_WEIGHT         24
#define UI_PLAYBACK_MENU_DIR_LEN        144
#define UI_PLAYBACK_MENU_File_LEN       144
#define UI_PLAYBACK_DOOR_SEL_Y_WEIGHT    50
#define UI_PLAYBACK_LOC_DOOR_SEL_X      300
#define UI_PLAYBACK_LOC_DOOR_SEL_Y      200
#define UI_PLAYBACK_LOC_DOOR_SEL_Y1     (UI_PLAYBACK_LOC_DOOR_SEL_Y+UI_PLAYBACK_DOOR_SEL_Y_WEIGHT)
#define UI_PLAYBACK_LOC_DOOR_SEL_Y2     (UI_PLAYBACK_LOC_DOOR_SEL_Y1+UI_PLAYBACK_DOOR_SEL_Y_WEIGHT)
#elif (USE_BIG_OSD == 1)
#define UI_PLAYBACK_NUM_PER_PAGE        8
#define UI_PLAYBACK_LOC_MENU_Y          40
#define UI_PLAYBACK_LOC_MENU_FILE_X     64
#define UI_PLAYBACK_LOC_MENU_DIR_X      232
#define UI_PLAYBACK_LOC_MENU_DATE_X     240
#define UI_PLAYBACK_LOC_MENU_TIME_X     448
#define UI_PLAYBACK_LOC_MENU_FILE_Y     80
#define UI_PLAYBACK_LOC_PAGE_X          360
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
#else
#define UI_PLAYBACK_NUM_PER_PAGE        8
#define UI_PLAYBACK_LOC_MENU_Y          20
#define UI_PLAYBACK_LOC_MENU_FILE_X     32
#define UI_PLAYBACK_LOC_MENU_DIR_X      116
#define UI_PLAYBACK_LOC_MENU_DATE_X     120
#define UI_PLAYBACK_LOC_MENU_TIME_X     224
#define UI_PLAYBACK_LOC_PAGE_X          180
#define UI_PLAYBACK_LOC_PAGE_X_L        16
#define UI_PLAYBACK_LOC_PAGE_Y          176
#define UI_PLAYBACK_MENU_LEN            264
#define UI_PLAYBACK_MENU_WEIGHT         20
#define UI_PLAYBACK_MENU_DIR_LEN        72
#define UI_PLAYBACK_MENU_File_LEN       72
#define UI_PLAYBACK_DOOR_SEL_Y_WEIGHT   40
#define UI_PLAYBACK_LOC_DOOR_SEL_X      100
#define UI_PLAYBACK_LOC_DOOR_SEL_Y      20
#define UI_PLAYBACK_LOC_DOOR_SEL_Y1     (UI_PLAYBACK_LOC_DOOR_SEL_Y+UI_PLAYBACK_DOOR_SEL_Y_WEIGHT)
#define UI_PLAYBACK_LOC_DOOR_SEL_Y2     (UI_PLAYBACK_LOC_DOOR_SEL_Y1+UI_PLAYBACK_DOOR_SEL_Y_WEIGHT)
#endif

#if (USE_BIG_OSD == 1)
#define UI_OSD_CAM_LOC_X                50//Carrie 200
#else
#define UI_OSD_CAM_LOC_X                100
#endif

#define UI_OSD_CAM_LOC_Y                30
/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */

void (*OSDDisplay[])(u8, u32, u32, u32, u32) = {
    iduOSDDisplay1,
    iduTVOSDDisplay,
};

s32 (*uiPlaybackListNext[])(void) = {
    dcfPlaybackDirForward,
    dcfPlaybackFileNext,
    dcfPlaybackFileNext,
    dcfPlaybackFileNext,
};

s32 (*uiPlaybackListPrev[])(void) = {
    dcfPlaybackDirBackward,
    dcfPlaybackFilePrev,
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
#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
extern u8 PhotoFramenum;
#endif

/*
 *********************************************************************************************************
 * English
 *********************************************************************************************************
 */

u8 InsertSd_En[] = {41, 78, 83, 69, 82, 84, 0, 51, 36, 0, 35, 65, 82, 68, 1};   /*Insert SD Card!*/
u8 SdInit_En[] = {51, 36, 0, 35, 65, 82, 68, 0, 41, 78, 73, 84, 14, 14, 14};    /*SD Card Init...*/
u8 NoFile_En[] = {46, 79, 0, 38, 73, 76, 69, 0, 1};                             /*No File !*/
u8 Waiting_En[] = {55, 65, 73, 84, 73, 78, 71};                                 /*Waiting*/
u8 PlsWait_En[] = {48, 76, 69, 65, 83, 69, 0, 55, 65, 73, 84, 14, 14, 14};      /*Please Wait...*/
u8 REC_En[] = {50, 37, 35};                                                     /*REC*/
u8 Detecting_En[] = {36, 69, 84, 69, 67, 84, 73, 78, 71};                       /*Detecting*/
u8 WriteProt_En[] = {55, 82, 73, 84, 69, 0, 48, 82, 79, 84, 69, 67, 84};        /*Write Protect*/
u8 Formating_En[] = {38, 79, 82, 77, 65, 84, 73, 78, 71, 14, 14, 14};           /*Formating...*/
u8 FormatOk_En[] = {38, 79, 82, 77, 65, 84, 0, 47, 43};                         /*Format OK*/
u8 FormatFail_En[] = {38, 79, 82, 77, 65, 84, 0, 38, 65, 73, 76};               /*Format Fail*/
u8 TimeOut_En[] = {52, 73, 77, 69, 0, 47, 85, 84};                              /*Time Out*/
u8 MemFull_En[] = {45, 69, 77, 79, 82, 89, 0, 38, 85, 76, 76, 0, 1};            /*Memory Full !*/
u8 Volume_En[] = {54, 47, 44, 53, 45, 37};                                      /*VOLUME*/
u8 TimeSetErr_En[] = {52, 73, 77, 69, 0, 51, 69, 84, 0, 37, 82, 82, 79, 82, 1}; /*Time Set Error!*/
u8 UsbMassStorage_En[] = {53, 51, 34, 0, 45, 65, 83, 83, 0, 51, 84, 79, 82, 65, 71, 69, 1}; /*USB Mass Storage!*/
u8 SdError_En[] = {35, 65, 82, 68, 0, 37, 82, 82, 79, 82, 0, 1};                /*Card Error !*/
u8 UpdatePass_En[] = {53, 48, 36, 33, 52, 37, 0, 48, 33, 51, 51, 0, 1};         /*UPDATE PASS !*/
u8 UpdateFail_En[] = {53, 48, 36, 33, 52, 37, 0, 38, 33, 41, 44, 0, 1};         /*UPDATE FAIL !*/
u8 FatErr_En[] = {38, 33, 52, 0, 40, 69, 65, 68, 69, 82, 0, 37, 82, 82, 79, 82, 0, 1};  /*FAT Header Error !*/
u8 PlsReFormat_En[] = {48, 76, 69, 65, 83, 69, 0, 50, 69, 13, 70, 79, 82, 77, 65, 84};  /*Please Re-format*/
u8 CardStillFail_En[] = {35, 65, 82, 68, 0, 51, 84, 73, 76, 76, 0, 38, 65, 73, 76};     /*Card Still Fail*/
u8 PlsChangeCard_En[] = {48, 76, 69, 65, 83, 69, 0, 67, 72, 65, 78, 71, 69, 0, 65, 78, 79, 84, 72, 69, 82, 0, 51, 36, 0, 67, 65, 82, 68};   /*Please change another SD card*/
u8 FsErr_En[] = {38, 51, 0, 47, 80, 69, 82, 65, 84, 73, 79, 78, 0, 37, 82, 82, 79, 82}; /*FS Operation Error*/
u8 ChkWriteProt_En[] = {35, 72, 69, 67, 75, 0, 51, 36, 0, 87, 82, 73, 84, 69, 0, 80, 82, 79, 84, 69, 67, 84};   /*Check SD write protect*/
u8 SdHWErr_En[] = {51, 36, 0, 35, 65, 82, 68, 0, 40, 15, 55, 0, 37, 82, 82, 79, 82};    /*SD Card H/W Error*/
u8 FwUpdatePass_En[] = {38, 55, 0, 53, 80, 68, 65, 84, 69, 68, 0, 48, 65, 83, 83, 1};   /*FW Updated Pass!*/
u8 FwUpdateFail_En[] = {38, 55, 0, 53, 80, 68, 65, 84, 69, 68, 0, 38, 65, 73, 76, 1};   /*FW Updated Fail!*/
u8 ChkUiLib_En[] = {35, 72, 69, 67, 75, 0, 53, 41, 0, 44, 73, 66, 82, 65, 82, 89, 1};   /*Check UI Library!*/
u8 UiUpdatePass_En[] = {53, 41, 0, 53, 80, 68, 65, 84, 69, 68, 0, 48, 65, 83, 83, 1};   /*UI Updated Pass!*/
u8 UiUpdateFail_En[] = {53, 41, 0, 53, 80, 68, 65, 84, 69, 68, 0, 38, 65, 73, 76, 1};   /*UI Updated Fail!*/
u8 NoUiLib_En[] = {46, 79, 0, 53, 41, 0, 44, 73, 66, 82, 65, 82, 89, 12, 0, 37, 83, 67, 65, 80, 69, 0, 53, 80, 68, 65, 84, 73, 78, 71, 1};  /*No UI Library, Escape Updating!*/
u8 FileErr_En[] = {38, 73, 76, 69, 0, 37, 82, 82, 79, 82, 1};       /*File Error!*/
u8 FileName_En[] = {38, 73, 76, 69, 0, 46, 65, 77, 69};             /*File Name*/
u8 Date_En[] = {36, 65, 84, 69};                                    /*Date*/
u8 StartTime_En[] = {51, 84, 65, 82, 84, 0, 52, 73, 77, 69};        /*Start Time*/
u8 Page_En[] = {48, 65, 71, 69};                                    /*Page*/
u8 FolderName_En[] = {38, 79, 76, 68, 69, 82, 0, 46, 65, 77, 69};     /*Folder Name*/
u8 EndName_En[] = {37, 78, 68, 0, 52, 73, 77, 69};                    /*End Time*/
u8 DateTime_En[] = {36, 65, 84, 69, 0, 52, 73, 77, 69};               /*Date Time*/
u8 TimeZone_En[] = {52, 73, 77, 69, 0, 58, 79, 78, 69};               /*Time Zone*/
u8 Abnormal_En[] = {33, 66, 78, 79, 82, 77, 65, 76, 0, 47, 80, 69, 82, 65, 84, 73, 79, 78, 1};     /*Abnormal Operation!*/
u8 NoSignal_En[] = {46, 79, 0, 51, 73, 71, 78, 65, 76};         /*No Signal*/
u8 ChangeResolution_En[] = {35, 40, 33, 46, 39, 37, 0, 50, 37, 51, 47, 44, 53, 52, 41, 47, 46, 14, 14, 14}; /*CHANGE RESOLUTION...*/



/*====================OSD Menu =======================*/

UI_MULT_ICON OSD_CHANNEL_Icon_1M[UI_MULT_ICON_END] =
{
    {OSD_SPLIT_0,48,48},
    {OSD_SPLIT_0,48,48},
};
UI_MULT_ICON OSD_CHANNEL_Icon_2M[UI_MULT_ICON_END] =
{
    {OSD_SPLIT_1,48,48},
    {OSD_SPLIT_1,48,48},
};

UI_MULT_ICON OSD_SPLIT_Icon_1M[UI_MULT_ICON_END] =
{
    {OSD_SPLIT_0,48,48},
    {OSD_SPLIT_0,48,48},
};

UI_MULT_ICON OSD_SPLIT_Icon_2M[UI_MULT_ICON_END] =
{
    {OSD_SPLIT_1,48,48},
    {OSD_SPLIT_1,48,48},
};
UI_MULT_ICON OSD_FULL_Icon_1M[UI_MULT_ICON_END] =
{
    {OSD_FULL_0,48,48},
    {OSD_FULL_0,48,48},
};
UI_MULT_ICON OSD_FULL_Icon_2M[UI_MULT_ICON_END] =
{
    {OSD_FULL_1,48,48},
    {OSD_FULL_1,48,48},
};
UI_MULT_ICON OSD_ZOOM_Icon_1M[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_0,48,48},
    {OSD_ZOOM_0,48,48},
};

UI_MULT_ICON OSD_ZOOM_Icon_2M[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_1,48,48},
    {OSD_ZOOM_1,48,48},
};

UI_MULT_ICON OSD_SETTING_1M[UI_MULT_ICON_END] =
{
    {OSD_SETTING_0,48,48},
    {OSD_SETTING_0,48,48},
};

UI_MULT_ICON OSD_SETTING_2M[UI_MULT_ICON_END] =
{
    {OSD_SETTING_1,48,48},
    {OSD_SETTING_1,48,48},
};

UI_MULT_ICON OSD_SOUND_Icon_1M[UI_MULT_ICON_END] =
{
    {OSD_SOUND_0,48,48},
    {OSD_SOUND_0,48,48},
};

UI_MULT_ICON OSD_SOUND_Icon_2M[UI_MULT_ICON_END] =
{
    {OSD_SOUND_1,48,48},
    {OSD_SOUND_1,48,48},
};


UI_MULT_ICON OSD_FOLDER_Icon_1M[UI_MULT_ICON_END] =
{
    {OSD_FOLDER_0,48,48},
    {OSD_FOLDER_0,48,48},
};

UI_MULT_ICON OSD_FOLDER_Icon_2M[UI_MULT_ICON_END] =
{
    {OSD_FOLDER_1,48,48},
    {OSD_FOLDER_1,48,48},
};


UI_MULT_ICON OSD_IMG_SET_Icon_1M[UI_MULT_ICON_END] =
{
    {OSD_IMG_SET_0,48,48},
    {OSD_IMG_SET_0,48,48},
};

UI_MULT_ICON OSD_IMG_SET_Icon_2M[UI_MULT_ICON_END] =
{
    {OSD_IMG_SET_1,48,48},
    {OSD_IMG_SET_1,48,48},
};
UI_MULT_ICON OSD_CH1_1M[UI_MULT_ICON_END] =
{
    {ch11,48,48},
    {ch11,48,48},
};
UI_MULT_ICON OSD_CH1_2M[UI_MULT_ICON_END] =
{
    {ch12,48,48},
    {ch12,48,48},
};
UI_MULT_ICON OSD_CH2_1M[UI_MULT_ICON_END] =
{
    {ch21,48,48},
    {ch21,48,48},
};
UI_MULT_ICON OSD_CH2_2M[UI_MULT_ICON_END] =
{
    {ch22,48,48},
    {ch22,48,48},
};
UI_MULT_ICON OSD_CH3_1M[UI_MULT_ICON_END] =
{
    {ch31,48,48},
    {ch31,48,48},
};
UI_MULT_ICON OSD_CH3_2M[UI_MULT_ICON_END] =
{
    {ch32,48,48},
    {ch32,48,48},
};
UI_MULT_ICON OSD_CH4_1M[UI_MULT_ICON_END] =
{
    {ch41,48,48},
    {ch41,48,48},
};
UI_MULT_ICON OSD_CH4_2M[UI_MULT_ICON_END] =
{
    {ch42,48,48},
    {ch42,48,48},
};

UI_MULT_ICON OSD_ZOOM_IN_Icon_1M[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_0,48,48},
    {OSD_ZOOM_0,48,48},
};

UI_MULT_ICON OSD_ZOOM_IN_Icon_2M[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_1,48,48},
    {OSD_ZOOM_1,48,48},
};

UI_MULT_ICON OSD_ZOOM_OUT_Icon_1M[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_0,48,48},
    {OSD_ZOOM_0,48,48},
};

UI_MULT_ICON OSD_ZOOM_OUT_Icon_2M[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_1,48,48},
    {OSD_ZOOM_1,48,48},
};


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

UI_MULT_LAN_STR ChangeResoStr[UI_MULT_LANU_END] =
{
    {ChangeResolution_En, sizeof(ChangeResolution_En)},
    {ChangeResolution_En, sizeof(ChangeResolution_En)},
    {ChangeResolution_En, sizeof(ChangeResolution_En)},
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
    {MSG_CHANGE_RESOLUTION,     ChangeResoStr},
};

UI_MULT_ICON WarningIcon[UI_MULT_ICON_END] =
{
    {WAR_OSD,48,48},
    {OSD_WARNING,16,24},
};

UI_MULT_ICON VRecIcon[UI_MULT_ICON_END] =
{
    {OSD_VREC,16,24},
    {REC_OSD2,72,64},
};

UI_MULT_ICON FFIcon[UI_MULT_ICON_END] =
{
    {FF_OSD,48,48},
    {OSD_FF,12,16},
};

UI_MULT_ICON RewIcon[UI_MULT_ICON_END] =
{
    {REW_OSD,48,48},
    {OSD_REW,12,16},
};

UI_MULT_ICON DVIcon[UI_MULT_ICON_END] =
{
    {OSD_DV_ICON,20,40},
    {OSD_DV_ICON,20,40},
};

UI_MULT_ICON MIcon[UI_MULT_ICON_END] =
{
    {OSD_M_ICON,20,25},
    {OSD_M_ICON,20,25},
};

UI_MULT_ICON PlayIcon[UI_MULT_ICON_END] =
{
    {OSD_PLAY_ICON,16,25},
    {PLAY_OSD,48,48},
};

UI_MULT_ICON PlaybackIcon[UI_MULT_ICON_END] =
{
    {OSD_PLAYBACK_ICON,20,25},
    {OSD_PLAYBACK_ICON,20,25},
};

UI_MULT_ICON SDIcon[UI_MULT_ICON_END] =
{
    {OSD_SD_ICON,20,40},
    {SD_OSD,44,40},
};

UI_MULT_ICON StopIcon[UI_MULT_ICON_END] =
{
    {OSD_STOP,12,16},
    {STOP_OSD,48,48},
};

UI_MULT_ICON ZoomplusIcon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_plus,16,24},
    {OSD_ZOOM_plus,16,24},
};

UI_MULT_ICON ZoonminusIcon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_minus,16,24},
    {OSD_ZOOM_minus,16,24},
};

UI_MULT_ICON Zoom1Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[0],12,66},
    {OSD_ZOOM_ICON[0],12,66},
};

UI_MULT_ICON Zoom2Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[1],12,66},
    {OSD_ZOOM_ICON[1],12,66},
};

UI_MULT_ICON Zoom3Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[2],12,66},
    {OSD_ZOOM_ICON[2],12,66},
};

UI_MULT_ICON Zoom4Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[3],12,66},
    {OSD_ZOOM_ICON[3],12,66},
};

UI_MULT_ICON Zoom5Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[4],12,66},
    {OSD_ZOOM_ICON[4],12,66},
};

UI_MULT_ICON Zoom6Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[5],12,66},
    {OSD_ZOOM_ICON[5],12,66},
};

UI_MULT_ICON Zoom7Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[6],12,66},
    {OSD_ZOOM_ICON[6],12,66},
};

UI_MULT_ICON Zoom8Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[7],12,66},
    {OSD_ZOOM_ICON[7],12,66},
};

UI_MULT_ICON Zoom9Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[8],12,66},
    {OSD_ZOOM_ICON[8],12,66},
};

UI_MULT_ICON Zoom10Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[9],12,66},
    {OSD_ZOOM_ICON[9],12,66},
};

UI_MULT_ICON Zoom11Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[10],12,66},
    {OSD_ZOOM_ICON[10],12,66},
};

UI_MULT_ICON Zoom12Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[11],12,66},
    {OSD_ZOOM_ICON[11],12,66},
};

UI_MULT_ICON Zoom13Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[12],12,66},
    {OSD_ZOOM_ICON[12],12,66},
};

UI_MULT_ICON Zoom14Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[13],12,66},
    {OSD_ZOOM_ICON[13],12,66},
};

UI_MULT_ICON Zoom15Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[14],12,66},
    {OSD_ZOOM_ICON[14],12,66},
};

UI_MULT_ICON Zoom16Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[15],12,66},
    {OSD_ZOOM_ICON[15],12,66},
};

UI_MULT_ICON Zoom17Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[16],12,66},
    {OSD_ZOOM_ICON[16],12,66},
};

UI_MULT_ICON Zoom18Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[17],12,66},
    {OSD_ZOOM_ICON[17],12,66},
};

UI_MULT_ICON Zoom19Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[18],12,66},
    {OSD_ZOOM_ICON[18],12,66},
};

UI_MULT_ICON Zoom20Icon[UI_MULT_ICON_END] =
{
    {OSD_ZOOM_ICON[19],12,66},
    {OSD_ZOOM_ICON[19],12,66},
};

UI_MULT_ICON AtennaIcon[UI_MULT_ICON_END] =
{
    {OSD_ATENNA_ICON,12,24},
    {OSD_ATENNA_ICON,12,24},
};

UI_MULT_ICON AtennaSginal0Icon[UI_MULT_ICON_END] =
{
    {OSD_ANTENNA0_B,48,20},
    {OSD_ATENNA_SIGNAL_ICON[0],16,24},
};

UI_MULT_ICON AtennaSginal1Icon[UI_MULT_ICON_END] =
{
    {OSD_ANTENNA1_B,48,20},
    {OSD_ATENNA_SIGNAL_ICON[1],16,24},
};

UI_MULT_ICON AtennaSginal2Icon[UI_MULT_ICON_END] =
{
    {OSD_ANTENNA2_B,48,20},
    {OSD_ATENNA_SIGNAL_ICON[2],16,24},
};

UI_MULT_ICON AtennaSginal3Icon[UI_MULT_ICON_END] =
{
    {OSD_ANTENNA3_B,48,20},
    {OSD_ATENNA_SIGNAL_ICON[3],16,24},
};

UI_MULT_ICON AtennaSginal4Icon[UI_MULT_ICON_END] =
{
    {OSD_ANTENNA4_B,48,20},
    {OSD_ATENNA_SIGNAL_ICON[4],16,24},
};
UI_MULT_ICON PauseIcon[UI_MULT_ICON_END] =
{
    {OSD_PAUSE_ICON,20,25},
    {OSD_PAUSE_ICON,20,25},
};
#if (HW_BOARD_OPTION == MR6730_AFN)
UI_MULT_ICON MotionIcon[UI_MULT_ICON_END] =
{
    {OSD_MOTION_ICON,20,40},
    {OSD_MOTION_ICON,20,40},
};
#endif 

UI_OSDICON_TAB OsdIcon[] =
{
    {OSD_ICON_WARNING,    WarningIcon},
    {OSD_ICON_VREC,       VRecIcon},
    {OSD_ICON_FF,         FFIcon},
    {OSD_ICON_REW,        RewIcon},
    {OSD_ICON_DV,         DVIcon},
    {OSD_ICON_M,          MIcon},
    {OSD_ICON_PLAY,       PlayIcon},
    {OSD_ICON_PLAYBACK,   PlaybackIcon},
    {OSD_ICON_SD,         SDIcon},
    {OSD_ICON_STOP,       StopIcon},
    {OSD_ICON_ZOOMplus,   ZoomplusIcon},
    {OSD_ICON_ZOOMminus,  ZoonminusIcon},
    {OSD_ICON_ZOOM1,      Zoom1Icon},
    {OSD_ICON_ZOOM2,      Zoom2Icon},
    {OSD_ICON_ZOOM3,      Zoom3Icon},
    {OSD_ICON_ZOOM4,      Zoom4Icon},
    {OSD_ICON_ZOOM5,      Zoom5Icon},
    {OSD_ICON_ZOOM6,      Zoom6Icon},
    {OSD_ICON_ZOOM7,      Zoom7Icon},
    {OSD_ICON_ZOOM8,      Zoom8Icon},
    {OSD_ICON_ZOOM9,      Zoom9Icon},
    {OSD_ICON_ZOOM10,     Zoom10Icon},
    {OSD_ICON_ZOOM11,     Zoom11Icon},
    {OSD_ICON_ZOOM12,     Zoom12Icon},
    {OSD_ICON_ZOOM13,     Zoom13Icon},
    {OSD_ICON_ZOOM14,     Zoom14Icon},
    {OSD_ICON_ZOOM15,     Zoom15Icon},
    {OSD_ICON_ZOOM16,     Zoom16Icon},
    {OSD_ICON_ZOOM17,     Zoom17Icon},
    {OSD_ICON_ZOOM18,     Zoom18Icon},
    {OSD_ICON_ZOOM19,     Zoom19Icon},
    {OSD_ICON_ZOOM20,     Zoom20Icon},
    {OSD_ICON_CHANNEL_1M, OSD_CHANNEL_Icon_1M },
    {OSD_ICON_CHANNEL_2M, OSD_CHANNEL_Icon_2M },
    {OSD_ICON_SPLIT_1M,   OSD_SPLIT_Icon_1M },
    {OSD_ICON_SPLIT_2M,   OSD_SPLIT_Icon_2M },
    {OSD_ICON_FULL_1M,    OSD_FULL_Icon_1M },
    {OSD_ICON_FULL_2M,    OSD_FULL_Icon_2M },
    {OSD_ICON_ZOOM_1M,    OSD_ZOOM_Icon_1M },
    {OSD_ICON_ZOOM_2M,    OSD_ZOOM_Icon_2M },
    {OSD_ICON_SPLIT_1M,   OSD_SETTING_1M },
    {OSD_ICON_SPLIT_2M,   OSD_SETTING_2M },
    {OSD_ICON_MENU_1M,    OSD_SETTING_1M },
    {OSD_ICON_MENU_2M,    OSD_SETTING_2M },
    {OSD_ICON_PLAYBACK_1M,OSD_FOLDER_Icon_1M},
    {OSD_ICON_PLAYBACK_2M,OSD_FOLDER_Icon_2M},
    {OSD_ICON_SOUND_1M,   OSD_SOUND_Icon_1M},
    {OSD_ICON_SOUND_2M,   OSD_SOUND_Icon_2M },
    {OSD_ICON_IMG_SET_1M, OSD_IMG_SET_Icon_1M },
    {OSD_ICON_IMG_SET_2M, OSD_IMG_SET_Icon_2M },
    {OSD_ICON_CH1_1M,     OSD_CH1_1M },
    {OSD_ICON_CH1_2M,     OSD_CH1_2M },
    {OSD_ICON_CH2_1M,     OSD_CH2_1M },
    {OSD_ICON_CH2_2M,     OSD_CH2_2M },
    {OSD_ICON_CH3_1M,     OSD_CH3_1M },
    {OSD_ICON_CH3_2M,     OSD_CH3_2M },
    {OSD_ICON_CH4_1M,     OSD_CH4_1M },
    {OSD_ICON_CH4_2M,     OSD_CH4_2M },
    {OSD_ICON_ZOOM_IN_1M,  OSD_ZOOM_Icon_1M},
    {OSD_ICON_ZOOM_IN_2M,  OSD_ZOOM_Icon_2M},
    {OSD_ICON_ZOOM_OUT_1M,  OSD_ZOOM_Icon_1M},
    {OSD_ICON_ZOOM_OUT_2M,  OSD_ZOOM_Icon_2M},
    {OSD_ICON_ATENNA,     AtennaIcon},
    {OSD_ICON_ATENNA_Signal_0,  AtennaSginal0Icon},
    {OSD_ICON_ATENNA_Signal_1,  AtennaSginal1Icon},
    {OSD_ICON_ATENNA_Signal_2,  AtennaSginal2Icon},
    {OSD_ICON_ATENNA_Signal_3,  AtennaSginal3Icon},
    {OSD_ICON_ATENNA_Signal_4,  AtennaSginal4Icon},
#if (HW_BOARD_OPTION == MR6730_AFN)
	{OSD_ICON_MOTION,  MotionIcon},
#endif    
	{OSD_ICON_PAUSE,  PauseIcon},
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
extern u8  video_playback_speed;//for asf player level control
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

/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */

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

    diskInfo = &global_diskInfo;

    IduVideo_ClearPKBuf(0);
    IduVidBuf0Addr = (u32)PKBuf0;
    #if NEW_IDU_BRI
        BRI_IADDR_Y = IduVidBuf0Addr;
        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
    #endif
    (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk1, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_Blk1);
    uiOsdEnable(OSD_Blk1);
    if(gInsertCard==0)
    {
        uiOsdDrawInsertSD(OSD_Blk1);
        OSTimeDly(20);
        uiClearOSDBuf(OSD_Blk1);
        return;
    }
    general_MboxEvt->OSEventPtr=(void *)0;

    unSectorNum = diskInfo->total_clusters * diskInfo->sectors_per_cluster; /* SECTOR Size is fixed to be 512 Bytes, */

    if (unSectorNum < 1049576)  /* 1048576 = Sectors of 512MB*/
        ucPercLevel = 4;
    else if ((unSectorNum >= 1049576) && (unSectorNum < 2097152))   /* 512 MB <= capacity < 1GB*/
        ucPercLevel = 5;
    else if ((unSectorNum >= 2097152) && (unSectorNum < 4194304))   /* 1GB <= capacity < 2GB*/
        ucPercLevel = 7;
    else if ((unSectorNum >= 4194304) && (unSectorNum < 8388608))   /* 2GB <= capacity < 4GB*/
        ucPercLevel = 16;
    else if ((unSectorNum >= 8388608) && (unSectorNum < 16777216))  /* 4GB <= capacity < 8GB*/
        ucPercLevel = 24;
    else if ((unSectorNum >= 16777216) && (unSectorNum < 33554432)) /* 8GB <= capacity < 16GB*/
        ucPercLevel = 48;
    else if ((unSectorNum >= 33554432) && (unSectorNum < 67108864)) /* 16GB <= capacity < 32GB*/
        ucPercLevel = 100;
    else if (unSectorNum >= 67108864)   /* 32GB <= capacity */
        ucPercLevel = 200;

    osdDrawMessage(MSG_FORMATING, CENTERED, 80+osdYShift/2, OSD_Blk1, 0xC0, 0x00);
    osdDrawMessage(MSG_PLEASE_WAIT, CENTERED, 110+osdYShift/2, OSD_Blk1, 0xC0, 0x00);

    /* Draw progress bar */
    unPercTemp = 100 / ucPercLevel;
    do
    {
        count++;
        if(count <= ucPercLevel)
        {
            percent = (u32)(count * unPercTemp);

            sprintf ((char *)percentstring, "%d %%", percent);
            osdDrawFSWaitingBar(count,ucPercLevel,1,(OSDIconEndY-40),0);
            uiOSDASCIIStringByColor(percentstring, 40, (OSDIconEndY-60), OSD_Blk1, 0xc0, 0x00);
        }
        msg=OSMboxPend(general_MboxEvt, 40, &err);
        if(count>500)
            break;

    }while(!msg);

    /* Draw the last progress bar */
#if 0
    for(count=count+1;count<=ucPercLevel;count++)
    {
        percent = (u32)(count * unPercTemp);

        sprintf ((char *)percentstring, "%d %%", percent);
        osdDrawFSWaitingBar(count,ucPercLevel,1,(OSDIconEndY-40),0);
        uiOSDASCIIStringByColor(percentstring, 40, (OSDIconEndY-60), OSD_Blk1, 0xc0, 0x00);
    }
    OSTimeDly(2);
#endif

    uiClearOSDBuf(OSD_Blk1);

    if (!strcmp((const char*)msg, "PASS"))
        osdDrawMessage(MSG_FORMAT_OK, CENTERED, 100+osdYShift/2, OSD_Blk1, 0xC0, 0x00);
    else if (!strcmp((const char*)msg, "FAIL"))
        osdDrawMessage(MSG_FORMAT_FAIL, CENTERED, 100+osdYShift/2, OSD_Blk1, 0xC0, 0x00);
    else
        osdDrawMessage(MSG_TIME_OUT, CENTERED, 100+osdYShift/2, OSD_Blk1, 0xC0, 0x00);

    if (MyHandler.MenuMode == GOTO_FORMAT_MODE)
        OSMboxPost(speciall_MboxEvt, msg);
    OSTimeDlyHMSM(0, 0, 0, 300);
    uiClearOSDBuf(OSD_Blk1);        /* Clear the OSD Buf */
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

void uiOsdDrawPlaybackMenuTitle(u8 type)
{
#if (USE_BIG_OSD == 1)
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};

    uiOsdDisableAll();
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    switch (type)
    {
        case UI_DSP_PLAY_LIST_FILE:
        case UI_DSP_PLAY_LIST_DOOR_ALB:
        case UI_DSP_PLAY_LIST_DOOR_PIC:
            #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
            #else
				#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
				//"FILE_PLAYBACK_MENU" only
				uiOSDMultiLanguageStrByXY(MSG_FILE_NAME, UI_PLAYBACK_LOC_MENU_FILE_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
				uiOSDMultiLanguageStrByXY(MSG_DATE, UI_PLAYBACK_LOC_MENU_DATE_X+32, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
				uiOSDMultiLanguageStrByXY(MSG_START_TIME, UI_PLAYBACK_LOC_MENU_TIME_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);

				#else
                uiOSDMultiLanguageStrByXY(MSG_FILE_NAME, UI_PLAYBACK_LOC_MENU_FILE_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
                uiOSDMultiLanguageStrByXY(MSG_DATE, UI_PLAYBACK_LOC_MENU_DATE_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
                uiOSDMultiLanguageStrByXY(MSG_START_TIME, UI_PLAYBACK_LOC_MENU_TIME_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
				#endif
            #endif
            break;

        case UI_DSP_PLAY_LIST_DIR:
            #if(HW_BOARD_OPTION == MR6730_AFN)
				#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
				//"FILE_PLAYBACK_MENU" only							
                uiOSDMultiLanguageStrByXY(MSG_FOLDER_NAME, UI_PLAYBACK_LOC_MENU_FILE_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
	        	//uiOSDMultiLanguageStrByXY(MSG_DATE, UI_PLAYBACK_LOC_MENU_DATE_X+32, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
	        	uiOSDMultiLanguageStrByXY(MSG_DATE, UI_PLAYBACK_LOC_MENU_DATE_X+16, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
                uiOSDMultiLanguageStrByXY(MSG_START_TIME, UI_PLAYBACK_LOC_MENU_TIME_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);

				#else
                uiOSDMultiLanguageStrByXY(MSG_FOLDER_NAME, UI_PLAYBACK_LOC_MENU_FILE_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
	        	uiOSDMultiLanguageStrByXY(MSG_DATE, UI_PLAYBACK_LOC_MENU_DATE_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
                uiOSDMultiLanguageStrByXY(MSG_START_TIME, UI_PLAYBACK_LOC_MENU_TIME_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
				#endif
            #else
                uiOSDMultiLanguageStrByXY(MSG_FOLDER_NAME, UI_PLAYBACK_LOC_MENU_DIR_X, UI_PLAYBACK_LOC_MENU_Y, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
            #endif
            break;

        case UI_DSP_PLAY_LIST_DOOR_SELECT:
            uiOSDASCIIStringByColor("MOVIE", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y, OSD_BLK[sysTVOutOnFlag],  0xC0, 0xC1);
            uiOSDASCIIStringByColor("PICTURE", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y1, OSD_BLK[sysTVOutOnFlag],  0xC0, 0xC1);
            //uiOSDASCIIStringByColor("ALBUM", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y2, OSD_BLK[sysTVOutOnFlag],  0xC0, 0xC1);
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
            uiOSDMultiLanguageStrByXY(MSG_FILE_NAME, UI_PLAYBACK_LOC_MENU_FILE_X, UI_PLAYBACK_LOC_MENU_Y, OSD_Blk0, 0xC0, 0xC1);
            if(OSDIconEndX >= 320)
            {
                uiOSDMultiLanguageStrByXY(MSG_DATE, UI_PLAYBACK_LOC_MENU_DATE_X, UI_PLAYBACK_LOC_MENU_Y, OSD_Blk0, 0xC0, 0xC1);
                uiOSDMultiLanguageStrByXY(MSG_START_TIME, UI_PLAYBACK_LOC_MENU_TIME_X, UI_PLAYBACK_LOC_MENU_Y, OSD_Blk0, 0xC0, 0xC1);
            }
            break;

        case UI_DSP_PLAY_LIST_DIR:
            if(OSDIconEndX >= 320)
                uiOSDMultiLanguageStrByXY(MSG_FOLDER_NAME, UI_PLAYBACK_LOC_MENU_DIR_X, UI_PLAYBACK_LOC_MENU_Y, OSD_Blk0, 0xC0, 0xC1);
            else
                uiOSDMultiLanguageStrByXY(MSG_FOLDER_NAME, UI_PLAYBACK_LOC_MENU_FILE_X, UI_PLAYBACK_LOC_MENU_Y, OSD_Blk0, 0xC0, 0xC1);
            break;

        case UI_DSP_PLAY_LIST_DOOR_SELECT:
            uiOSDASCIIStringByColor("MOVIE", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y, OSD_Blk1,  0xC0, 0xC1);
            uiOSDASCIIStringByColor("PICTURE", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y1, OSD_Blk1,  0xC0, 0xC1);
            //uiOSDASCIIStringByColor("ALBUM", UI_PLAYBACK_LOC_DOOR_SEL_X, UI_PLAYBACK_LOC_DOOR_SEL_Y2, OSD_Blk1,  0xC0, 0xC1);
            break;
    }
#endif
}


extern u8 UI_OSD_4BIT_EN;   // defined in ui_osd.c

void uiOsdDrawPlaybackMenuFrame(u8 type, u8 select, u8 clean)
{
#if (USE_BIG_OSD == 1)
    u32  RowCol, RightCol, LeftCol;
    u16  RowLen, ColLen;
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32  StrH;
    u32  startX, startY = UI_PLAYBACK_LOC_MENU_FILE_Y-4, LineH =16;

#if IS_COMMAX_DOORPHONE
    return;
#endif

    StrH = uiOsdGetFontHeight(CurrLanguage);
    LineH += StrH;
    if (clean == 1)
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
        #if IS_HECHI_DOORPHONE
        RowCol  = 0x22222222;
        RightCol= 0x22222222;
        LeftCol = 0x22222222;
        #endif
    }

    if (type == UI_DSP_PLAY_LIST_DIR)
    {
        #if(HW_BOARD_OPTION == MR6730_AFN)
            RowLen = UI_PLAYBACK_MENU_LEN;
            startX = UI_PLAYBACK_LOC_MENU_FILE_X;
            ColLen = UI_PLAYBACK_MENU_WEIGHT;
        #else
            RowLen = UI_PLAYBACK_MENU_DIR_LEN;
            startX = UI_PLAYBACK_LOC_MENU_DIR_X;
            ColLen = UI_PLAYBACK_MENU_WEIGHT;
        #endif
    }
    else
    {
        if (OSDIconEndX >= 320)
            RowLen = UI_PLAYBACK_MENU_LEN;
        else
            RowLen = UI_PLAYBACK_MENU_File_LEN;
        #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
            startY = 0;
            startX = UI_PLAYBACK_LOC_MENU_FILE_X+RowLen*(select%2);
            select = select/2;
            LineH = 236;
            StrH = 232;
            ColLen = 240;
        #else
            startX = UI_PLAYBACK_LOC_MENU_FILE_X;
            ColLen = UI_PLAYBACK_MENU_WEIGHT;
        #endif
    }
    #if IS_HECHI_DOORPHONE
    UI_OSD_4BIT_EN = 1;
    #endif
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

    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, UI_PLAYBACK_MENU_WEIGHT, UI_PLAYBACK_LOC_MENU_FILE_X-4, 8+select*(OSD_STRING_H+4), OSD_Blk1, LeftCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 4, UI_PLAYBACK_MENU_WEIGHT, UI_PLAYBACK_LOC_MENU_FILE_X+RowLen-8, 8+select*(OSD_STRING_H+4), OSD_Blk1, RightCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], RowLen, 2, UI_PLAYBACK_LOC_MENU_FILE_X-4, 8+select*(OSD_STRING_H+4), OSD_Blk1, RowCol);
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], RowLen, 2, UI_PLAYBACK_LOC_MENU_FILE_X-4, 28+select*(OSD_STRING_H+4), OSD_Blk1, RowCol);
#endif
}

void uiOsdDrawPlaybackMenuPage(u32 Total, u32 Current)
{
#if (USE_BIG_OSD == 1)
    u8  pageInfo[13];    /*Page xxx/xxx*/
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};

    sprintf((char*)pageInfo, "Page %04d/%04d",Current, Total);
    uiOSDASCIIStringByColor(pageInfo, UI_PLAYBACK_LOC_PAGE_X , UI_PLAYBACK_LOC_PAGE_Y, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
    return;
#else
    u8  pageInfo[13];    /*Page xxx/xxx*/

    sprintf((char*)pageInfo, "Page %04d/%04d",Current, Total);
    if(OSDIconEndX >= 320)
        uiOSDASCIIStringByColor(pageInfo, UI_PLAYBACK_LOC_PAGE_X , UI_PLAYBACK_LOC_PAGE_Y, OSD_Blk1 , 0xC0, 0xC1);
    else
        uiOSDASCIIStringByColor(pageInfo, UI_PLAYBACK_LOC_PAGE_X_L , UI_PLAYBACK_LOC_PAGE_Y, OSD_Blk1 , 0xC0, 0xC1);
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
    u32  startY, LineH =16;

    startY = UI_PLAYBACK_LOC_MENU_FILE_Y;

    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], UI_PLAYBACK_MENU_LEN, (UI_PLAYBACK_LOC_PAGE_Y-startY+4), UI_PLAYBACK_LOC_MENU_FILE_X-4, startY-4, OSD_BLK[sysTVOutOnFlag], 0x00000000);
    StrH = uiOsdGetFontHeight(CurrLanguage);
    LineH += StrH;
    for (i = 0; i < DirCnt; i++)
    {
        if (dcfPlaybackCurDir== NULL)
            return;
        strncpy((char*)prt_str, dcfPlaybackCurDir->pDirEnt->d_name, 8);
        prt_str[8] = '\0';
        #if(HW_BOARD_OPTION == MR6730_AFN)
            uiOSDASCIIStringByColor(prt_str, UI_PLAYBACK_LOC_MENU_FILE_X, startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
            if(OSDIconEndX >= 320){
				#if(TIME_FORMAT_TYPE==1)
				//DMY
                sprintf((char*)prt_str, "%02d/%02d/%02d",                                                  
                         (dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD & 0x001F),
                         (dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5,                         
                         (dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD >> 9)+1980
                       );
				#else				
                sprintf((char*)prt_str, "%02d/%02d/%02d",
                         (dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD >> 9)+1980,
                         (dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5,
                         (dcfPlaybackCurDir->pDirEnt->fsFileCreateDate_YMD & 0x001F)
                       );
				#endif
				#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
				//"FILE_PLAYBACK_MENU" only				
				uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_DATE_X+16 , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
				#else
                uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_DATE_X , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
				#endif
                sprintf( (char*)prt_str, "%02d:%02d:%02d",
                         (dcfPlaybackCurDir->pDirEnt->fsFileCreateTime_HMS>>11),
                         (dcfPlaybackCurDir->pDirEnt->fsFileCreateTime_HMS & 0x07E0)>>5,
                         (dcfPlaybackCurDir->pDirEnt->fsFileCreateTime_HMS & 0x001F)<<1
                        );
                uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_TIME_X , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
            }
        #else
            if(OSDIconEndX >= 320)
                uiOSDASCIIStringByColor(prt_str, UI_PLAYBACK_LOC_MENU_DIR_X, startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
            else
                uiOSDASCIIStringByColor(prt_str, UI_PLAYBACK_LOC_MENU_FILE_X, startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
        #endif
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
            uiOSDASCIIStringByColor(prt_str, UI_PLAYBACK_LOC_MENU_DIR_X, 12+i*(OSD_STRING_H+4), OSD_Blk1, 0xC0, 0xC1);
        else
            uiOSDASCIIStringByColor(prt_str, UI_PLAYBACK_LOC_MENU_FILE_X, 12+i*(OSD_STRING_H+4), OSD_Blk1, 0xC0, 0xC1);
        if (i != DirCnt-1)
            (*uiPlaybackListPrev[PlayListDspType])();
    }
#endif
}

void uiOsdDrawPlaybackMenuFile(u8 FileCnt)
{
#if (USE_BIG_OSD == 1)
    u8 i;
#if(HW_BOARD_OPTION == MR6730_AFN)
	u8	prt_str[16];
#else
    u8  prt_str[9];
#endif
    u8   OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32  startY, StrH, LineH =16;

    startY = UI_PLAYBACK_LOC_MENU_FILE_Y;

    StrH = uiOsdGetFontHeight(CurrLanguage);
    LineH += StrH;
    //uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
#if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
    #if IS_HECHI_DOORPHONE
        IduVideo_ClearPKBuf(0);
    #endif
#else
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], UI_PLAYBACK_MENU_LEN, (UI_PLAYBACK_LOC_PAGE_Y-startY+4), UI_PLAYBACK_LOC_MENU_FILE_X-4, startY-4, OSD_BLK[sysTVOutOnFlag], 0x00000000);
#endif

    for (i = 0; i < FileCnt; i++)
    {
#if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
    #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
        void Menu4Split_setFilename(int split, const char* filename);

        Menu4Split_setFilename(i, dcfPlaybackCurFile->pDirEnt->d_name);
    #endif
        filecon=i;
        if (sysReadFile() == 0)
        {
            system_busy_flag=0;
            return;
        }

#else
        if (dcfPlaybackCurFile == NULL)
            return;

		#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
			/*
			#if 1//gdcfFileType_Info[0]="ASF",gdcfFileType_Info[1]="JPG"
			if (sysThumnailPtr->type != 1)//sysThumnailPtr->type ( 0:JPG, 1:ASF/AVI, 2:WAV )
			#else
			if(dcfPlaybackCurFile->fileType != DCF_FILE_TYPE_ASF)
			#endif
			*/
			strncpy((char*)prt_str, dcfPlaybackCurFile->pDirEnt->d_name, 12);
			prt_str[12] = '\0';
		#else
        strncpy((char*)prt_str, dcfPlaybackCurFile->pDirEnt->d_name, 8);
        prt_str[8] = '\0';
		#endif
        uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_FILE_X , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
        if(OSDIconEndX >= 320)
        {
        #if(TIME_FORMAT_TYPE==1)
		//DMY
            sprintf((char*)prt_str, "%02d/%02d/%02d",
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD & 0x001F),
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5,                     
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD >> 9)+1980                   
                   );		
		#else        
            sprintf((char*)prt_str, "%02d/%02d/%02d",
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD >> 9)+1980,
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5,
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD & 0x001F)
                   );
		#endif
			#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
			//"FILE_PLAYBACK_MENU" only
			uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_DATE_X+32 , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
			#else
            uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_DATE_X , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
			#endif
            sprintf( (char*)prt_str, "%02d:%02d:%02d",
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS>>11),
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS & 0x07E0)>>5,
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS & 0x001F)<<1
                    );
			#if 0//( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
			//"FILE_PLAYBACK_MENU" only
			uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_TIME_X+32 , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
			#else
            uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_TIME_X , startY+i*(LineH+4), OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
			#endif
        }
#endif

        if (i != FileCnt-1)
            (*uiPlaybackListPrev[PlayListDspType])();
    }
#else
    u8 i;
    u8  prt_str[9];

    uiClearOSDBuf(OSD_Blk1);
    for (i = 0; i < FileCnt; i++)
    {
        if (dcfPlaybackCurFile == NULL)
            return;
        strncpy((char*)prt_str, dcfPlaybackCurFile->pDirEnt->d_name, 8);
        prt_str[8] = '\0';
        uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_FILE_X , 12+i*(OSD_STRING_H+4), OSD_Blk1 , 0xC0, 0xC1);
        if(OSDIconEndX >= 320)
        {
            sprintf((char*)prt_str, "%02d/%02d/%02d",
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD >> 9)+1980,
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD & 0x01E0)>>5,
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateDate_YMD & 0x001F)
                   );
            uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_DATE_X , 12+i*(OSD_STRING_H+4), OSD_Blk1 , 0xC0, 0xC1);
            sprintf( (char*)prt_str, "%02d:%02d:%02d",
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS>>11),
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS & 0x07E0)>>5,
                     (dcfPlaybackCurFile->pDirEnt->fsFileCreateTime_HMS & 0x001F)<<1
                    );
            uiOSDASCIIStringByColor(prt_str,  UI_PLAYBACK_LOC_MENU_TIME_X , 12+i*(OSD_STRING_H+4), OSD_Blk1 , 0xC0, 0xC1);
        }
        if (i != FileCnt-1)
            (*uiPlaybackListPrev[PlayListDspType])();
    }
#endif
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
    u8          OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};
    UI_MULT_ICON *iconInfo;
    u32         StrH, warnY;
#endif

#if (USE_BIG_OSD == 1)
    uiOsdGetIconInfo(OSD_ICON_WARNING,&iconInfo);
    StrH = uiOsdGetFontHeight(CurrLanguage);
    warnY = OSDIconMidY-(iconInfo->Icon_h+8+StrH)/2;
    uiOSDIconColorByY(OSD_ICON_WARNING , warnY , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    osdDrawMessage(MSG_NO_FILE, CENTERED, (warnY+iconInfo->Icon_h+8), OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
#else
    uiOSDIconColorByXY(OSD_ICON_WARNING ,(OSDIconMidX-8) , (OSDIconMidY-68) , OSD_Blk1, 0x00 , alpha_3);
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
                playbackItem = 1;
            else
                playbackItem--;
            uiOsdDrawPlaybackMenuDoorSel(playbackItem, 0);
            break;

        case UI_KEY_DOWN:
            uiOsdDrawPlaybackMenuDoorSel(playbackItem, 1);
            if (playbackItem == 1)
                playbackItem = 0;
            else
                playbackItem++;
            uiOsdDrawPlaybackMenuDoorSel(playbackItem, 0);
            break;

        case UI_KEY_ENTER:
            if (playbackItem == 0)
                PlayListDspType = UI_DSP_PLAY_LIST_DIR;
            else
            {
                PlayListDspType = playbackItem+1;
                #if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
                dcfDoorChangeDir(uiDoorDirIndex[PlayListDspType-1]);
                #endif
                #if IS_COMMAX_DOORPHONE  || IS_HECHI_DOORPHONE
                    if(PlayListDspType==UI_DSP_PLAY_LIST_DOOR_ALB)
                    {
                        PhotoFramenum=0;
                        splitmenu=0;
                        return ;
                    }
                #endif
            }
            uiOsdDrawPlaybackMenu(0);
            break;

        case UI_KEY_MODE:
            #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
                IduVideo_ClearPKBuf(0);
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

void uiOsdDrawPlaybackMenu(u8 key)
{
    u8          i, display_num, move;
    static u32  totalPage = 0, playbackItem = 0, currPage, playbackDir, LastType;
    u32         TotalNum, tmp_page, curLocation;

#if (USE_BIG_OSD == 1)
    u8          OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    UI_MULT_ICON *iconInfo;
    u32         StrH, warnY;
#endif

    switch (PlayListDspType)
    {
        case UI_DSP_PLAY_LIST_DIR:
            TotalNum = global_totaldir_count;
            curLocation = playbackDir;
            break;

        case UI_DSP_PLAY_LIST_FILE:
            TotalNum = global_totalfile_count;
            curLocation = playback_location;
            break;


#if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
        case UI_DSP_PLAY_LIST_DOOR_PIC:
            TotalNum = global_total_Pic_file_count;
            curLocation = playback_location;
            break;

        case UI_DSP_PLAY_LIST_DOOR_ALB:
            TotalNum = global_total_Alb_file_count;
            curLocation = playback_location;
            break;
#endif
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
                TotalNum = global_totaldir_count;
                curLocation = playbackDir;
            #elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
                PlayListDspType = UI_DSP_PLAY_LIST_FILE;
                TotalNum = global_totalfile_count;
                curLocation = playback_location;
            #endif
            uiOsdDrawPlaybackMenuTitle(PlayListDspType);
            if(TotalNum == 0 || gInsertCard == 0)
            {
                uiOsdDrawPlaybackMenuNoFile();
                return;
            }
            dcfPlaybackCurDir = dcfListDirEntTail;
            #if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
                LastType = PlayListDspType; // for back from page-delete
                if (PlayListDspType == UI_DSP_PLAY_LIST_DOOR_PIC)
                {
                    dcfPlaybackCurFile = dcfListPicFileEntTail;
                }
                else if (PlayListDspType == UI_DSP_PLAY_LIST_DOOR_ALB)
                    dcfPlaybackCurFile = dcfListAlbFileEntTail;
                else
                    dcfPlaybackCurFile = dcfListFileEntTail;
            #else
                dcfPlaybackCurFile = dcfListFileEntTail;
            #endif

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
        #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
            uiOsdDrawPlaybackMenuPage(totalPage, currPage);
        #endif
            #if RFIU_SUPPORT
                uiSetRfDisplayMode(UI_MENU_RF_ENTER_PLAYBACK);
            #endif
            break;

        case UI_KEY_UP:
            if (TotalNum == 0 || gInsertCard==0)
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
                #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
                uiOsdDrawPlaybackMenuPage(totalPage, currPage);
                #endif
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
            if (TotalNum == 0 || gInsertCard==0)
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
                #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
                uiOsdDrawPlaybackMenuPage(totalPage, currPage);
                #endif
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
            if(TotalNum == 0 || gInsertCard==0)
                break;
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
            #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
            uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            #endif
            break;

        case UI_KEY_LEFT:
            /*Page UP*/
            if(TotalNum == 0 || gInsertCard==0)
                break;
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
            #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
            uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            #endif
            break;

        case UI_KEY_ENTER:
            if(PlayListDspType == UI_DSP_PLAY_LIST_DIR)
            {
                if(TotalNum == 0 || gInsertCard == 0)
                    break;
                DEBUG_UI("Enter folder %s\r\n",dcfPlaybackCurDir->pDirEnt->d_name);
                PlayListDspType = UI_DSP_PLAY_LIST_FILE;
                uiOsdDrawPlaybackMenuTitle(PlayListDspType);
                if(TotalNum == 0 || gInsertCard == 0)
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
                TotalNum = global_totalfile_count;
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
            #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
                uiOsdDrawPlaybackMenuPage(totalPage, currPage);
            #endif
            }
            else
            {
                if(TotalNum == 0 || gInsertCard == 0)
                    break;
                DEBUG_UI("Change to Playback Mode %s\r\n", dcfPlaybackCurFile->pDirEnt->d_name);
                LastType = PlayListDspType; // for back from single-view playback
                PlayListDspType = UI_DSP_PLAY_LIST_PLAYBACK;    /*playback mode*/
                playbackflag = 2;
                uiMenuEnable = 0x41;
            #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
                #if IS_COMMAX_DOORPHONE
                    splitmenu = 2;    //Toby130306; 640*480 in 800*480
                #else
                    splitmenu = 0;    //Toby130306;
                #endif
                Iframe_flag=0;  // 1: We just need I-frame 0: play whole MP4
            #else
                Iframe_flag=1;  // 1: We just need I-frame 0: play whole MP4
            #endif
                siuOpMode = 0;
                sysCameraMode = SYS_CAMERA_MODE_PLAYBACK;
                uiMenuOSDReset();

				#if( HW_BOARD_OPTION == MR6730_AFN)
				OSTimeDlyHMSM(0, 0, 0, 100);//delay for screen ready
			   	#endif
				
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
			
			#if( HW_BOARD_OPTION == MR6730_AFN)
				if (strstr(dcfPlaybackCurFile->pDirEnt->d_name, "ASF") != NULL){
					sysThumnailPtr->type = 1;
				}else if (strstr(dcfPlaybackCurFile->pDirEnt->d_name, "JPG") != NULL){
					sysThumnailPtr->type = 0;
				}
			#endif

			#if( (HW_BOARD_OPTION == MR6730_AFN)&&(USE_PLAYBACK_AUTONEXT) )
			if(setUI.SYS_PlayOrder==1)	
			{//turn-on auto-play when everytime entering playback
				//if(dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_JPG)	
				{
					gAutoPlay_Ceased=0;//reset	
					DEBUG_UI("==AutoPlay ON==\n");
				}
			}
			#endif	

            #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
                osdDrawPlayIcon();
            #endif
                uiReadVideoFile();
                MyHandler.MenuMode = PLAYBACK_MODE;
            }
            break;

        case UI_KEY_MENU:
            #if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
                if ((PlayListDspType == UI_DSP_PLAY_LIST_DIR)||
                    (PlayListDspType == UI_DSP_PLAY_LIST_DOOR_PIC)||
                    (PlayListDspType == UI_DSP_PLAY_LIST_DOOR_ALB))
                {
                    DEBUG_UI("Playback list to Door Select\r\n");
                    uiOsdDrawPlaybackMenuDoor(UI_KEY_MODE);
                    break;
                }
            #endif
            #if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
                if(PlayListDspType == UI_DSP_PLAY_LIST_DIR)
            #elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
                if(PlayListDspType == UI_DSP_PLAY_LIST_FILE)
            #elif (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
                if (PlayListDspType == UI_DSP_PLAY_LIST_DOOR_SELECT)
            #endif
            {
                #if (UI_SUPPORT_TREE == 1)
                DEBUG_UI("Playback list to Setup mode\r\n");
                MyHandler.MenuMode = SETUP_MODE;
                uiOsdDisableAll();
                #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_D1) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
                #else
                if(sysTVOutOnFlag)
                    TV_reset();
                #endif
                uiEnterMenu(UI_MENU_NODE_PLAYBACK_SEARCH_ALL);
                uiGraphDrawMenu();
                dcfPlaybackCurDir = dcfListDirEntTail;
                dcfScanFileOnPlaybackDir();
                #else
                DEBUG_UI("Not Support Setup mode\r\n");
                #endif
                break;
            }
            if(PlayListDspType == UI_DSP_PLAY_LIST_FILE)
            {
                DEBUG_UI("File list to Folder List %d, %d\r\n",global_totaldir_count, playbackDir);
                PlayListDspType = UI_DSP_PLAY_LIST_DIR;
                TotalNum = global_totaldir_count;
                curLocation = playbackDir;
            }
            else if(PlayListDspType == UI_DSP_PLAY_LIST_PLAYBACK)
            {
                PlayListDspType = LastType;
                DEBUG_UI("Playback return to File List %d\r\n", PlayListDspType);
                if (PlayListDspType == UI_DSP_PLAY_LIST_FILE)
                {
                    TotalNum = global_totalfile_count;
                    curLocation = playback_location;
                }
            #if (FILE_SYSTEM_SEL == FILE_SYSTEM_DOOR)
                else if (PlayListDspType == UI_DSP_PLAY_LIST_DOOR_PIC)
                {
                    TotalNum = global_total_Pic_file_count;
                    curLocation = playback_location;
                }
                else if (PlayListDspType == UI_DSP_PLAY_LIST_DOOR_ALB)
                {
                    TotalNum = global_total_Alb_file_count;
                    curLocation = playback_location;
                }
            #endif
                #if (FILE_PLAYBACKFILE_SET != FILE_PLAYBACK_SPLITMENU)
                IduVideo_ClearPKBuf(0);
                #endif
                IduVidBuf0Addr = (u32)PKBuf0;
            #if NEW_IDU_BRI
                BRI_IADDR_Y = IduVidBuf0Addr;
                BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
            #endif
            }

        #if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
            Iframe_flag=1;
            splitmenu=1;
        #endif
            uiOsdDrawPlaybackMenuTitle(PlayListDspType);
            if(TotalNum == 0 || gInsertCard == 0)
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

        case UI_KEY_DELETE:
			#if ( (HW_BOARD_OPTION == MR6730_AFN && UI_PBMENU_M2) )
			#if 1//(UI_PBMENU_M2_DEBUG)
			DEBUG_UI("UI_KEY_DELETE,curLocation=%d,TotalNum=%d\n",curLocation,TotalNum);
			#endif
			#endif
            IduVideo_ClearPKBuf(0);
            uiOsdDrawPlaybackMenuTitle(PlayListDspType);
			#if ( !(HW_BOARD_OPTION == MR6730_AFN && UI_PBMENU_M2) )
            if( curLocation > TotalNum)
            {
                curLocation = 1;
            }
			#else
			curLocation--;//move to previous item
			if (curLocation == 0)
			{
				curLocation = 1;//focus on 1st item
				(*uiPlaybackListPrev[PlayListDspType])();
			}
			#endif
            if(TotalNum == 0 || gInsertCard == 0)
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

void uiOsdDrawAtenna(u8 signal)
{
#if(HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS )
    u16 signal_x_start=25;  // 訊號圖示X座標起始位置
#elif (HW_BOARD_OPTION == MR8120_RX_RDI)
    u16 signal_x_start=260;  // 訊號圖示X座標起始位置
#else
    u16 signal_x_start=50;  // 訊號圖示X座標起始位置
#endif
    u8 i;
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};
    u8  bgColor=0x00;
    #if (UI_PREVIEW_OSD == 0)
        return;
    #endif
    if(uiIsRFBroken == 1)
    {
        bgColor=0xC1;
    }
    else if(uiIsRFBroken == 0)
    {
        bgColor=0x00;
    }

    //uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], AtennaSginal0Icon[0].Icon_w, AtennaSginal0Icon[0].Icon_h, signal_x_start, 19+osdYShift, OSD_Blk2, 0);
    #if (OSD_SIZE_X2_DISABLE == 0)
    uiOSDIconColorByXY(OSD_ICON_ATENNA_Signal_0+signal, signal_x_start, 17,  OSD_BLK[sysTVOutOnFlag], bgColor, alpha_3);
    #else
    uiOSDIconColorByXY(OSD_ICON_ATENNA_Signal_0+signal, signal_x_start, 17,  OSD_BLK[sysTVOutOnFlag], bgColor, alpha_3);
    #endif


}
void osdDrawMenuPreview(void)
{
u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};
#if (UI_PREVIEW_OSD == 0)
    return;
#endif
#if (USE_BIG_OSD == 1)
    #if(HW_BOARD_OPTION == MR6730_AFN )
	#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)
		return;
	#endif		
	 //uiOSDIconColorByXY(OSD_ICON_DV ,(OSDIconEndX-60), 30, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
	 	#if (TVOUT_27MHZ)
        uiOSDIconColorByXY(OSD_ICON_DV ,(OSDDispWidth[sysTVOutOnFlag]-90), 25, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
		#else
        uiOSDIconColorByXY(OSD_ICON_DV ,(OSDDispWidth[sysTVOutOnFlag]-60), 25, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
		#endif
    #else
        uiOSDIconColorByXY(OSD_ICON_DV ,(OSDIconEndX-40), 9, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    #endif
#else
    uiOSDIconColorByXY(OSD_ICON_DV ,292, 9, OSD_Blk2, 0x00 , alpha_3);
    if(gInsertCard == 1)
        uiOSDIconColorByXY(OSD_ICON_SD ,(OSDIconEndX-28), (OSDIconEndY-40), OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    else
        uiOsdDrawInsertSD(OSD_BLK[sysTVOutOnFlag]);
#endif
}
#if ((HW_BOARD_OPTION == MR8120_RX_JESMAY) || (HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS))
void uiOSDDrawNoSignal(void)
{
    INT8U err;
    char camera[5];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};

    sprintf(camera,"CAM%d",(sysRFRxInMainCHsel+1));

    DEBUG_UI("enter draw no signal %x\n",iconflag[UI_MENU_SETIDX_FULL_SCREEN]);
#if(RFIU_SUPPORT)
    #if((HW_BOARD_OPTION == MR8120_RX_JESMAY) || (HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS) )
        if (uiIsRFBroken == 1)
        {
            DEBUG_UI("enter draw no signal2 \n");
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],OSDDispWidth[sysTVOutOnFlag],OSDDispHeight[sysTVOutOnFlag],0,0, OSD_BLK[sysTVOutOnFlag],0xC1C1C1C1);
            uiOSDMultiLanguageStrCenter(MSG_NO_SIGNAL, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
        }
    #else
        if (iconflag[UI_MENU_SETIDX_FULL_SCREEN] == UI_MENU_RF_FULL)
        {
            DEBUG_UI("enter draw no signal2 \n");
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],OSDDispWidth[sysTVOutOnFlag],OSDDispHeight[sysTVOutOnFlag],0,0, OSD_BLK[sysTVOutOnFlag],0xC1C1C1C1);
            uiOSDMultiLanguageStrCenter(MSG_NO_SIGNAL, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
            memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM);
        }
    #endif


#endif

}
#else
void uiOsdDrawNoSignal(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    char camera[5];

    if (sysVideoInCHsel == 0)
        sprintf(camera,"CAM%d",(sysRFRxInMainCHsel+1));
    else
        sprintf(camera,"CAM%d",sysVideoInCHsel);

    if (act == UI_OSD_DRAW)
    {
        DEBUG_UI("Camera %d No Signal\r\n",sysRFRxInMainCHsel);
        uiMenuOSDReset();
        (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
        uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag],OSDDispWidth[sysTVOutOnFlag],OSDDispHeight[sysTVOutOnFlag],0,0, OSD_BLK[sysTVOutOnFlag],0xC1C1C1C1);
        osdDrawMessage(MSG_NO_SIGNAL, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0xC1);
        uiOsdDrawAtenna(0);
        uiOSDASCIIStringByColor(camera, UI_OSD_CAM_LOC_X, UI_OSD_CAM_LOC_Y,  OSD_BLK[sysTVOutOnFlag], 0xf8, 0x00);
    }
    else
    {
        uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
        osdDrawPreviewIcon();
    }
    memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE*DISPLAY_BUF_NUM);
}

#endif
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
    #if(HW_BOARD_OPTION == MR6730_AFN )
		/*
        if (curr_playback_speed > UI_PLAYBACK_SPEED_LEVEL/2)
            uiOSDIconColorByXY(OSD_ICON_FF ,50, 30, OSD_Blk2, 0x00 , alpha_3);
        else if(curr_playback_speed < UI_PLAYBACK_SPEED_LEVEL/2)
            uiOSDIconColorByXY(OSD_ICON_REW ,50, 30, OSD_Blk2, 0x00 , alpha_3);
        else
	    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 12 , 16 , 50 , 30, OSD_Blk2 , 0);

        uiOSDASCIIStringByColor(Playback_speed[curr_playback_speed], 70 , 30, OSD_Blk2 ,  0xC0, 0x41);
		*/


        u8 DispBuff=OSD_Blk0;
	
				uiOsdEnable(DispBuff);
		
        if (curr_playback_speed > UI_PLAYBACK_SPEED_LEVEL/2)
            uiOSDIconColorByXY(OSD_ICON_FF ,50, 30, DispBuff, 0x00 , alpha_3);
        else if(curr_playback_speed < UI_PLAYBACK_SPEED_LEVEL/2)
            uiOSDIconColorByXY(OSD_ICON_REW ,50, 30, DispBuff, 0x00 , alpha_3);
        else
				//uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 12 , 16 , 50 , 30, DispBuff , 0);
				uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 180 , 120 , 50 , 30, DispBuff , 0);
				
				//uiOSDASCIIStringByColor(Playback_speed[curr_playback_speed], 70 , 30, DispBuff ,  0xC0, 0x41);
				//uiOSDASCIIStringByColor(Playback_speed[curr_playback_speed], 100 , 45 , DispBuff ,  0xC0, 0x41);
				uiOSDASCIIStringByColor(Playback_speed[curr_playback_speed], 100 , 45 , DispBuff ,  0xC0, 0x00);
				
    #else
    if (curr_playback_speed > UI_PLAYBACK_SPEED_LEVEL/2)
        uiOSDIconColorByXY(OSD_ICON_FF ,40, 12, OSD_Blk0, 0x00 , alpha_3);
    else if(curr_playback_speed < UI_PLAYBACK_SPEED_LEVEL/2)
        uiOSDIconColorByXY(OSD_ICON_REW ,40, 12, OSD_Blk0, 0x00 , alpha_3);
    else
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 12 , 16 , 40 , 12 , OSD_Blk0 , 0);
    uiOSDASCIIStringByColor(Playback_speed[curr_playback_speed], 12 , 12, OSD_Blk0 ,  0xC0, 0x41);
    #endif
}

void uiOsdDrawMaskArea(u8 key)
{
    u8  space_x, space_y;
    u8  OSD_BLK[2] = {IDU_OSD_L1_WINDOW_0, IDU_OSD_L0_WINDOW_0};
    u8  maskMaxColNum = MASKAREA_MAX_COLUMN, maskMaxRowNum;
    u32 LinColar;
    u16 i, j, drawX, tmpX, drawY, drawW;

    switch(key)
    {
        case 0: /*first enter mask area*/
            maskMaxRowNum = MASKAREA_MAX_ROW;
            playbackflag = 0;
            uiMenuEnable=0;
            uiMenuOSDReset();
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
    return;
}

void uiOsdDrawCardInfo(void)
{
    FS_DISKFREE_T *diskInfo;
    u8  tmp_str[10];
    u32 remain;
    u8  osdxShift = 0, shiftY=0;
    u8 color;

#if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_D1) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF))
    osdxShift = 12;
#endif
    if(TvOutMode == UI_MENU_SETTING_TV_OUT_PAL)
        shiftY = 1;
    (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk0, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_Blk0);
    uiOsdEnable(OSD_Blk0);
        color=0xC0;
    if((gInsertCard==1)&&(got_disk_info!= 0))
    {
        diskInfo=&global_diskInfo;
        sprintf((char*)tmp_str, "%dMB",diskInfo->total_clusters*diskInfo->sectors_per_cluster/(1024*2));  /*1MB =2048 sector*/
        uiOSDASCIIStringByColor(tmp_str, 180+osdxShift , 98+osdYShift/2, OSD_Blk0, color, 0x00);
        remain = diskInfo->avail_clusters*1000/diskInfo->total_clusters;
        sprintf((char*)tmp_str, "%d.%d%%",(1000-remain)/10, (1000-remain)%10);
        uiOSDASCIIStringByColor(tmp_str, 180+osdxShift, 122+osdYShift/2+shiftY*3, OSD_Blk0, color, 0x00);
        sprintf((char*)tmp_str, "%d.%d%%",remain/10, remain%10);
        uiOSDASCIIStringByColor(tmp_str, 180+osdxShift, 144+osdYShift/2+shiftY*10, OSD_Blk0, color, 0x00);
    }
    else
    {
        uiOSDASCIIStringByColor("0MB", 180+osdxShift, 100+osdYShift/2, OSD_Blk0, color, 0x00);
        uiOSDASCIIStringByColor("0%", 180+osdxShift, 120+osdYShift/2+shiftY*3, OSD_Blk0, color, 0x00);
        uiOSDASCIIStringByColor("0%", 180+osdxShift, 140+osdYShift/2+shiftY*10, OSD_Blk0, color, 0x00);
    }
}

/*osd not ready*/
void uiOsdDrawNetwork(u8 key)
{

    switch(key)
    {
        case 0:
            break;
        case UI_KEY_MODE:   /*leave*/
            uiCurrNode = uiCurrNode->parent;
            MyHandler.MenuMode = SETUP_MODE;
            uiOsdDisable(OSD_Blk0);
            uiGraphDrawMenu();
            break;

        default:
            DEBUG_UI("uiOsdDrawNetwork error key %d\r\n",key);
            return;
    }
}

void uiOsdDrawLifeTimePerSec(void)
{
    RTC_DATE_TIME   localTime;
    u8  DateTime[40];   /*YYYY/MM/DD  HH:mm:ss CHX*/
    u8  strlength=0;
    
#if((HW_BOARD_OPTION == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA671)|| (HW_BOARD_OPTION == MR8120_TX_RDI_CA672)||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA532)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))

    #if  ((UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_MDY_APM) || (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_YMD_APM) ||(UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_DMY_APM))
    u8  loc_x_vga=8;
    u8  loc_y_vga=22;
    u8  loc_x_hd=8;
    u8  loc_y_hd=19;
    u8  loc_x_qvga=8;
    u8  loc_y_qvga=13;
    #else
    u8  loc_x_vga=10;
    u8  loc_y_vga=22;
    u8  loc_x_hd=10;
    u8  loc_y_hd=19;
    u8  loc_x_qvga=10;
    u8  loc_y_qvga=13;
    #endif    
#elif((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
      (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
    u8  loc_x_vga=2;
    u8  loc_y_vga=27;
    u8  loc_x_hd=2;
    u8  loc_y_hd=28;
#elif((HW_BOARD_OPTION == MR8120_TX_JIT) || (HW_BOARD_OPTION == MR8120_TX_JIT_AV))
    u8  loc_x_vga=6;
    u8  loc_y_vga=25;
    u8  loc_x_hd=6;
    u8  loc_y_hd=26;
#elif (HW_BOARD_OPTION == MR6730_AFN)
	u8	loc_x_vga=2;
	u8	loc_y_vga=27;
	u8	loc_x_hd=0;
	u8	loc_y_hd=16;
	
#else
    u8  loc_x_vga=2;
    u8  loc_y_vga=27;
    u8  loc_x_hd=2;
    u8  loc_y_hd=28;

#endif


    
#if((CIU1_OSD_EN)&& (TIMESTAMP_ENABLE))
    RTC_Get_GMT_Time(&localTime);  


    #if((HW_BOARD_OPTION == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA671)|| (HW_BOARD_OPTION == MR8120_TX_RDI_CA672)||\
        (HW_BOARD_OPTION == MR8120_TX_RDI_CA532)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
        #if (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_MDY)
            sprintf ((char *)DateTime, "%02d/%02d/20%02d %02d:%02d:%02d",  localTime.month, localTime.day, localTime.year, localTime.hour, localTime.min, localTime.sec);
            strlength=20;
        #elif (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_YMD)
            sprintf ((char *)DateTime, "20%02d/%02d/%02d %02d:%02d:%02d", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);
            strlength=20;
        #elif  (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_DMY_APM)

            if (localTime.hour > 12)
                sprintf ((char *)DateTime, "%02d/%02d/20%02d %02d:%02d:%02d PM", localTime.day, localTime.month, localTime.year, localTime.hour-12, localTime.min, localTime.sec);
            else if (localTime.hour == 12)
                sprintf ((char *)DateTime, "%02d/%02d/20%02d %02d:%02d:%02d PM", localTime.day, localTime.month, localTime.year, localTime.hour, localTime.min, localTime.sec);
            else if (localTime.hour == 0)
                sprintf ((char *)DateTime, "%02d/%02d/20%02d %02d:%02d:%02d AM", localTime.day, localTime.month, localTime.year, localTime.hour+12, localTime.min, localTime.sec);
            else 
                sprintf ((char *)DateTime, "%02d/%02d/20%02d %02d:%02d:%02d AM", localTime.day, localTime.month, localTime.year, localTime.hour, localTime.min, localTime.sec);
            strlength=23;            
        #elif  (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_MDY_APM)

            if (localTime.hour > 12)
                sprintf ((char *)DateTime, "%02d/%02d/20%02d %02d:%02d:%02d PM", localTime.month, localTime.day, localTime.year, localTime.hour-12, localTime.min, localTime.sec);
            else if (localTime.hour == 12)
                sprintf ((char *)DateTime, "%02d/%02d/20%02d %02d:%02d:%02d PM", localTime.month, localTime.day, localTime.year, localTime.hour, localTime.min, localTime.sec);
            else if (localTime.hour == 0)
                sprintf ((char *)DateTime, "%02d/%02d/20%02d %02d:%02d:%02d AM", localTime.month, localTime.day, localTime.year, localTime.hour+12, localTime.min, localTime.sec);
            else 
                sprintf ((char *)DateTime, "%02d/%02d/20%02d %02d:%02d:%02d AM", localTime.month, localTime.day, localTime.year, localTime.hour, localTime.min, localTime.sec);
            strlength=23;
        #elif  (UI_SHOW_TIME_FORMAT == UI_SHOW_TIME_FORMAT_YMD_APM)

            if (localTime.hour > 12)
                sprintf ((char *)DateTime, "20%02d/%02d/%02d %02d:%02d:%02d PM", localTime.year, localTime.month, localTime.day, localTime.hour-12, localTime.min, localTime.sec);
            else if (localTime.hour == 12)
                sprintf ((char *)DateTime, "20%02d/%02d/%02d %02d:%02d:%02d PM", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);
            else if (localTime.hour == 0)
                sprintf ((char *)DateTime, "20%02d/%02d/%02d %02d:%02d:%02d AM", localTime.year, localTime.month, localTime.day, localTime.hour+12, localTime.min, localTime.sec);
            else 
                sprintf ((char *)DateTime, "20%02d/%02d/%02d %02d:%02d:%02d AM", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);
            strlength=23;
        #endif
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, DateTime,
                        32,ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                        loc_x_hd, loc_y_hd, loc_x_hd+strlength,loc_y_hd+1);  
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {

        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, DateTime,
                        32,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                        loc_x_vga,loc_y_vga,loc_x_vga+strlength,loc_y_vga+1);     
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_320x240)
    {

        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, DateTime,
                        32,ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                        loc_x_qvga,loc_y_qvga,loc_x_qvga+strlength,loc_y_qvga+1);     
    }
    #elif((HW_BOARD_OPTION ==MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
          (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
        #if(UI_PROJ_OPT == 1)
            sprintf ((char *)DateTime, "20%02d/%02d/%02d      %02d:%02d", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min);

            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
            {

                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, DateTime,
                                32,ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                                loc_x_hd, loc_y_hd, loc_x_hd+16,loc_y_hd+2);  
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
            {

                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, DateTime,
                                32,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                                loc_x_vga,loc_y_vga,loc_x_vga+16,loc_y_vga+2);     
            }
        #else
            sprintf ((char *)DateTime, "20%02d/%02d/%02d      %02d:%02d:%02d", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);

            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
            {

                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, DateTime,
                                32,ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                                loc_x_hd, loc_y_hd, loc_x_hd+16,loc_y_hd+2);  
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
            {

                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, DateTime,
                                32,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                                loc_x_vga,loc_y_vga,loc_x_vga+16,loc_y_vga+2);     
            }
        #endif
    
    #else

	#if(TIME_FORMAT_TYPE==1)
	//DMY
    sprintf ((char *)DateTime, "%02d/%02d/20%02d      %02d:%02d:%02d", localTime.day, localTime.month, localTime.year, localTime.hour, localTime.min, localTime.sec);	
	#else
	
    sprintf ((char *)DateTime, "20%02d/%02d/%02d      %02d:%02d:%02d", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);
	#endif

    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {

        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, DateTime,
                        32,ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                        loc_x_hd, loc_y_hd, loc_x_hd+16,loc_y_hd+2);  
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
    {

        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, DateTime,
                        32,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                        loc_x_vga,loc_y_vga,loc_x_vga+16,loc_y_vga+2);     
    }
    
    #endif
    

#endif

    if(showTime == 1)
    {
        RTC_Get_GMT_Time(&localTime);
		#if(TIME_FORMAT_TYPE==1)
		//DMY
        sprintf ((char *)DateTime, "%02d/%02d/20%02d %02d:%02d:%02d", localTime.day, localTime.month, localTime.year, localTime.hour, localTime.min, localTime.sec);
		
		#else
		
        sprintf ((char *)DateTime, "20%02d/%02d/%02d %02d:%02d:%02d", localTime.year, localTime.month, localTime.day, localTime.hour, localTime.min, localTime.sec);
		#endif
        DEBUG_UI("Current Time %s\r\n", DateTime);
    }
}

void uiOsdDrawRecPerSec(void)
{
    static u8 recOn = UI_OSD_DRAW;
    u32 waitFlag;
    u8  err;

#if  (HW_BOARD_OPTION == MR6730_AFN )
	#if 0
     osdDrawVideoOn(recOn);
     recOn = (recOn + 1) % UI_OSD_NONE;
	#else


	osdDrawVideoOn(setUI.recOn);	
	setUI.recOn = (setUI.recOn + 1) % UI_OSD_NONE;	

	#endif //#if 0 
#endif

#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    waitFlag = (FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 << sysRFRxInMainCHsel);
    if (OSFlagAccept(gRfRxVideoPackerSubReadyFlagGrp, waitFlag, OS_FLAG_WAIT_CLR_ANY, &err) >0)
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
#endif
}


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

//#if (DERIVATIVE_MODEL==MODEL_TYPE_PUSH)
#if(USE_PLAYBACK_AUTONEXT)
void uiOsdClearVideoPlayTime(u8 IsDraw)
{// clear area of playtime+videotime 
	u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
	u32 OSDH;
	u16 ico_w,ico_h;
	u16 pos_x,pos_y;
	u32 ColorData=0;
	u8 txt_color=0;
	char timeForRecord1[10+10];
	OSDH = 470;
	if(sysTVinFormat == TV_IN_NTSC){
		OSDH = ((OSDH*480)/576);
	}
#if (USE_BIG_OSD == 1)
	#if 0//(DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)
	#if(IDUOSD_TIMESTAMP)
	OSDH+=20;
	#else
	OSDH+=7;
	#endif		 
	#endif
	//uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 128 , 20 , 408 , OSDH, OSD_BLK[sysTVOutOnFlag] , 0);
	ico_w=128+16+128;
	ico_h=20;
	pos_x=408-128;
	pos_y=OSDH;

	//DEBUG_UI("ClearVideoPlayTime(%d,%d,%d,%d) on osd[%d]\n",ico_w,ico_h,pos_x,pos_y,OSD_BLK[sysTVOutOnFlag]);
	
	//uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 128+16+128 , 20 , 408-128 , OSDH, OSD_BLK[sysTVOutOnFlag] , 0);
	if(IsDraw==UI_OSD_DRAW)
    {//for debug
    	sprintf (timeForRecord1, "%02d:%02d:%02d/%02d:%02d:%02d", 0, 0, 0, 0, 0, 0);		
		txt_color=0xc2;//0xc0;
		//ColorData=0xC4C4C4C4; //color for debug
	}else
	{
		_SPACEs(timeForRecord1, (8+1+8));
		timeForRecord1[(8+1+8)+1]=0;	
	
		txt_color=0xc0;//0;
		//ColorData=0;
	}
	//
	
	uiOSDASCIIStringByColor((u8*)timeForRecord1, pos_x , pos_y, OSD_BLK[sysTVOutOnFlag] , txt_color, 0x00);
	//uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , ico_w , ico_h , pos_x , pos_y, OSD_BLK[sysTVOutOnFlag] , ColorData);
	
#endif	

}//
#endif
//#endif


void uiOsdDrawPlayTime(u8 type, u32 time_unit)
{
    u8 h,m,s;
    char  timeForRecord[9];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32 StrH;

#if( HW_BOARD_OPTION == MR6730_AFN )
    u32 OSDH;
    OSDH = 470;
    if(sysTVinFormat == TV_IN_NTSC){
        OSDH = ((470*480)/576);
    }
#endif

    StrH = uiOsdGetFontHeight(CurrLanguage)+8;
    if(type==1)
    {
        h = time_unit / 3600;
        m = (time_unit - h*3600) / 60;
        s = time_unit - h*3600 - m*60;

        sprintf (timeForRecord, "%02d:%02d:%02d", h, m, s);
        #if (USE_BIG_OSD == 1)
	    #if( HW_BOARD_OPTION == MR6730_AFN )
			#if(DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)
			#if(IDUOSD_TIMESTAMP)
			OSDH+=20;
			#else
			OSDH+=7;
			#endif
			#endif
	        uiOSDASCIIStringByColor((u8*)timeForRecord, 408 , OSDH, OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
	    #else
            uiOSDASCIIStringByColor((u8*)timeForRecord, 408 , (OSDIconEndY-StrH) , OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
	    #endif

        #else
            uiOSDASCIIStringByColor((u8*)timeForRecord, 208 , 7 , OSD_Blk0 , 0xc0, 0x00);
        #endif
    }
    else    /*clean time*/
    {
        #if (USE_BIG_OSD == 1)
	     #if( HW_BOARD_OPTION == MR6730_AFN )
		 	#if(DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)
			#if(IDUOSD_TIMESTAMP)
			OSDH+=20;
			#else
			OSDH+=7;
			#endif		 
			#endif
	     	uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 128 , 20 , 408 , OSDH, OSD_BLK[sysTVOutOnFlag] , 0);
	     #else
        	uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 64 , 16 , 408 , (OSDIconEndY-StrH) , OSD_BLK[sysTVOutOnFlag] , 0);
	     #endif

        #else
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 64 , 16 , 208 , 77 , OSD_Blk0 , 0);
        #endif
    }
}

void uiOsdVolumeControl(u8 mode, UI_VALUECTRL value)
{
    u8  i;
    u8  tempstring[4];
    u8  DisBuf;
    u16 draw_x, draw_y;
#if(HW_BOARD_OPTION == MR6730_AFN )
	#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)
		#if(VOL_CTRL_MAX==31)
		#define VOL_BAR_FRAME_W				400
		#define VOL_BAR_FRAME_H				100	
		#define VOL_BAR_GRID_W				6
		#define VOL_BAR_GRID_H				16
		#define VOL_BAR_GRID_SP				6
		#define VOL_BAR_GRID_STEP			(VOL_BAR_GRID_W+VOL_BAR_GRID_SP)// 12		
		#define VOL_BAR_MID_X_OFST_L		180// 150
		#define VOL_BAR_MID_X_OFST_C		(4+VOL_BAR_GRID_SP)// 4
		#define VOL_BAR_MID_X_OFST_R		180// 148


		#elif(VOL_CTRL_MAX==16)
		#define VOL_BAR_FRAME_W				400
		#define VOL_BAR_FRAME_H				100		
		#define VOL_BAR_GRID_W				8
		#define VOL_BAR_GRID_H				16
		#define VOL_BAR_GRID_SP				8
		#define VOL_BAR_GRID_STEP			(VOL_BAR_GRID_W+VOL_BAR_GRID_SP)// 12	
		//ASCII font is 16x20 when #if (USE_BIG_OSD == 1)
		#define VOL_BAR_MID_X_OFST_L		((VOL_CTRL_MAX/2)*VOL_BAR_GRID_STEP)+16// 150
		#define VOL_BAR_MID_X_OFST_C		(VOL_BAR_GRID_STEP/2)// 4
		#define VOL_BAR_MID_X_OFST_R		((VOL_CTRL_MAX/2)*VOL_BAR_GRID_STEP)-16// 148
		

		#else
		#error"VOL_BAR_XXX undefined"
		#endif
	#endif
#endif

	

    /*need draw 152*56 icon*/
    if(mode == VIDEO_MODE)
    {
        DisBuf = OSD_Blk2;
        draw_y = (OSDIconEndY-56)/2;
    }
    else
    {
	#if(HW_BOARD_OPTION == MR6730_AFN )
		DisBuf = OSD_Blk0;
		draw_y = (OSDIconEndY-56)/2;
        if(sysTVinFormat == TV_IN_NTSC){
            draw_y = ((draw_y*480)/576);
        }
		DEBUG_UI("y[%d]\n", draw_y);
	#else
        DisBuf = OSD_Blk1;
        uiOsdEnable(1);
        draw_y = (PbkOSD1High-56)/2;
	#endif
    }

    if(value == UI_VALUE_ADD)
    {
    #if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
        if(sysVolumnControl == (VOL_CTRL_MAX-1))
            sysVolumnControl = (VOL_CTRL_MAX-1);
	#else
        if(sysVolumnControl == 9)
            sysVolumnControl = 9;	
	#endif
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
	#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
		u16 x_offset=0;
        if( (VOL_CTRL_MAX==32)||(VOL_CTRL_MAX==16) )
        {
			x_offset=VOL_BAR_MID_X_OFST_L;
			uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], VOL_BAR_FRAME_W, VOL_BAR_FRAME_H, (OSDIconMidX-x_offset), draw_y, DisBuf, 0);				
		}else
		{
			x_offset=80;
			uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 300, 100, (OSDIconMidX-x_offset), draw_y, DisBuf, 0);				
		}
	#else	    
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 300, 100, (OSDIconMidX-80), draw_y, DisBuf, 0);
	#endif
        volumeflag = 0;
        return;
    }


    DEBUG_UI("draw volume sysVolumnControl %d\r\n",sysVolumnControl);
    uiOSDASCIIStringByColor("VOLUME", (OSDIconMidX-40), draw_y, DisBuf, 0xC0, 0x41);  //24->50
    draw_y += 20;
	#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
        if( (VOL_CTRL_MAX==32)||(VOL_CTRL_MAX==16) )
        {
			draw_x = OSDIconMidX-VOL_BAR_MID_X_OFST_L;
			uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], (VOL_BAR_GRID_STEP*VOL_CTRL_MAX), VOL_BAR_GRID_H, draw_x, draw_y, DisBuf, 0);

			if(VOL_CTRL_MAX==32)
				draw_x += (VOL_BAR_GRID_STEP-VOL_BAR_GRID_W);//fine adjust
			else if(VOL_CTRL_MAX==16)
				draw_x += 4;//draw_x += (VOL_BAR_GRID_STEP/2);//draw_x += (VOL_BAR_GRID_STEP/2);//fine adjust
				
		}else
		{
			draw_x = OSDIconMidX-60;  //76->70
			uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 152, 16, draw_x, draw_y, DisBuf, 0);
		}
	#else	
    draw_x = OSDIconMidX-60;  //76->70
    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 152, 16, draw_x, draw_y, DisBuf, 0);
    #endif
    
    for(i=0;i<sysVolumnControl;i++) 
    {
	#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
        if( (VOL_CTRL_MAX==32)||(VOL_CTRL_MAX==16) )
        {
			uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], VOL_BAR_GRID_W, VOL_BAR_GRID_H, draw_x, draw_y, DisBuf, 0xC6C6C6C6);
			draw_x+=VOL_BAR_GRID_STEP; 
		}
		else
		{
	        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 8, 16, draw_x, draw_y, DisBuf, 0xC6C6C6C6);
	        draw_x+=16;			
		}
	#else
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 8, 16, draw_x, draw_y, DisBuf, 0xC6C6C6C6);
        draw_x+=16;
	#endif	
    }
    draw_y += 20;
	#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
    if( (VOL_CTRL_MAX==32)||(VOL_CTRL_MAX==16) )
    {	
	    uiOSDASCIIStringByColor("-", (OSDIconMidX-VOL_BAR_MID_X_OFST_L) , draw_y , DisBuf , 0xC0, 0x41);  //76->70
	    //sprintf((char*)tempstring,"%-2d",sysVolumnControl);
	    sprintf((char*)tempstring,"%02d",sysVolumnControl);
	    uiOSDASCIIStringByColor(tempstring, (OSDIconMidX-VOL_BAR_MID_X_OFST_C) , draw_y , DisBuf , 0xC0, 0x41);
	    uiOSDASCIIStringByColor("+", (OSDIconMidX+VOL_BAR_MID_X_OFST_R) , draw_y, DisBuf , 0xC0, 0x41);
    }else
	{
	    uiOSDASCIIStringByColor("-", (OSDIconMidX-70) , draw_y , DisBuf , 0xC0, 0x41);  //76->70
	    sprintf((char*)tempstring,"%d",sysVolumnControl);
	    uiOSDASCIIStringByColor(tempstring, (OSDIconMidX-4) , draw_y , DisBuf , 0xC0, 0x41);
	    uiOSDASCIIStringByColor("+", (OSDIconMidX+68) , draw_y, DisBuf , 0xC0, 0x41);
	}
	#else
		uiOSDASCIIStringByColor("-", (OSDIconMidX-70) , draw_y , DisBuf , 0xC0, 0x41);	//76->70
		sprintf((char*)tempstring,"%d",sysVolumnControl);
		uiOSDASCIIStringByColor(tempstring, (OSDIconMidX-4) , draw_y , DisBuf , 0xC0, 0x41);
		uiOSDASCIIStringByColor("+", (OSDIconMidX+68) , draw_y, DisBuf , 0xC0, 0x41);
	#endif
    volumeflag = 1;
	#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
	iconflag[UI_MENU_SETIDX_VOLUME] = (VOL_CTRL_MAX-1)- sysVolumnControl;
	#else
    iconflag[UI_MENU_SETIDX_VOLUME] = 9- sysVolumnControl;
	#endif
    uiMenuAction(UI_MENU_SETIDX_VOLUME);
	
    #if(HW_BOARD_OPTION == MR6730_AFN )
		DEBUG_UI("=DAC= uiOsdVolumeControl(%d) \n",sysVolumnControl);	
        g_menus[MENU_PBVOLUME].setItem = sysVolumnControl;
		UI_saveSettings();
    #endif		
}

void uiOsdDrawMenuData(void)
{
    u32 i;
    UI_MENU_NODE *draw_data;
    static UI_NODE_DATA  last_data;
    u8 len;
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
        uiOSDIconColorByXY(draw_data->item.NodeData->IconData[i].IconIndex, draw_data->item.NodeData->IconData[i].Location_x,draw_data->item.NodeData->IconData[i].Location_y, OSD_Blk0 ,0 , alpha_3);

    }
    memcpy(&last_data, draw_data->item.NodeData, sizeof(UI_NODE_DATA));
}

void uiOsdDrawZoomSet(u8 chanel, u8 key)
{

}

void uiOsdDrawImageSet(u8 chanel, u8 key)
{

}

void uiOsdDrawMenu(void)
{
    UI_MENU_NODE *paretn;
    u8  channel;
#if(SHOW_UI_PROCESS_TIME == 1)
    u32 time1;
#endif

    if(uiCurrNode->parent)
        paretn = uiCurrNode->parent;
    else
        paretn = uiCurrNode;
    DEBUG_UI("current node %s action %d\r\n",uiCurrNode->item.NodeData->Node_Name, uiCurrNode->item.NodeData->Action_no);
    DEBUG_UI("paretn node %s action %d\r\n",paretn->item.NodeData->Node_Name, paretn->item.NodeData->Action_no);

    switch(paretn->item.NodeData->Action_no)
    {
        case UI_MENU_SETIDX_CH1ZOOM:
        case UI_MENU_SETIDX_CH2ZOOM:
        case UI_MENU_SETIDX_CH3ZOOM:
        case UI_MENU_SETIDX_CH4ZOOM:
            channel = paretn->item.NodeData->Action_no-UI_MENU_SETIDX_CH1ZOOM;
            DEBUG_UI("Enter ch %d Zoom in Setting \r\n", channel);
            MyHandler.MenuMode = SET_MASK_AREA;
            uiOsdDrawZoomSet(channel, 0);
            break;

        case UI_MENU_SETIDX_CH1IMG:
        case UI_MENU_SETIDX_CH2IMG:
        case UI_MENU_SETIDX_CH3IMG:
        case UI_MENU_SETIDX_CH4IMG:
            channel = paretn->item.NodeData->Action_no-UI_MENU_SETIDX_CH1IMG;
            DEBUG_UI("Enter ch %d Image Setting \r\n", channel);
            MyHandler.MenuMode = SET_MASK_AREA;
            uiOsdDrawImageSet(channel, 0);
            break;

        case UI_MENU_SETIDX_VOLUME:
            DEBUG_UI("Enter Volume Setting \r\n");
            uiOsdVolumeControl(VIDEO_MODE, UI_VALUE_CURRENT);
            break;

        default:
            uiOsdDrawMenuData();
            break;
    }

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    printf("UI Show Menu Time 1 =%d (x50ms)\n",time1);
#endif
}
void uiOsdEnterOsdMenuMode(void)
{
    uiMenuOSDReset();
    (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk0, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiOsdEnable(OSD_Blk0);
    uiOsdDrawMenu();
}




void uiOsdDrawAtennaByTX(u8 signal,u8 camera)
{
    u8  OSDStr[32];
    u8  camStr='\0';
    u8  signalStr='\0';

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

void uiOsdDrawCamera(void)
{
    char camera[5];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};

    sprintf(camera,"CAM%d",(sysRFRxInMainCHsel+1));
    uiOSDASCIIStringByColor(camera, UI_OSD_CAM_LOC_X, UI_OSD_CAM_LOC_Y,  OSD_BLK[sysTVOutOnFlag], 0xf8, 0x00);
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
    u32 waitFlag = 0, cnt = 15;
    u8 SecString[15];

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
            #if((HW_BOARD_OPTION == MR8120_TX_TRANWO) || (HW_BOARD_OPTION == MR8120_TX_TRANWO2) ||\
                (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
            gpioTimerCtrLed(LED_ON);
            #endif

            #if((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
                (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
            OSTimeDly(20);  /*避免配對完成又收到配對按鍵: wait 1 sec*/
            #else
            OSTimeDly(100);  /*避免配對完成又收到配對按鍵: wait 5 sec*/
            #endif
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
            #if((HW_BOARD_OPTION == MR8120_TX_TRANWO)|| (HW_BOARD_OPTION == MR8120_TX_TRANWO2) ||\
                (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
            gpioTimerCtrLed(LED_ON);
            #endif

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

/*
 *********************************************************************************************************
 * Called by other moudle
 *********************************************************************************************************
 */

void osdDrawPlaybackArea(u8 mode)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    uiMenuOSDReset();

    #if( HW_BOARD_OPTION == MR6730_AFN )
    if(mode == 1)
    #else
    if(mode == 2)   /*rf playback*/
    #endif
    {
        (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
        uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
        osdDrawPreviewIcon();
    }
    else
    {
        (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk0, 0, 0, OSDDispWidth[sysTVOutOnFlag], PbkOSD1);
        (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk1, 0, PbkOSD1, OSDDispWidth[sysTVOutOnFlag], PbkOSD2);
        (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk2, 0, PbkOSD2, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
        uiOsdEnable(OSD_Blk0);
        uiOsdEnable(OSD_Blk2);
    }
}

void osdDrawProtect(u8 mode)
{
    u8 str[] = "Write Protect";

   if( gInsertCard==1)
   {
        if(mode==1) /*Playback*/
        {
            (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk1, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
            uiOsdEnable(OSD_Blk1);
            uiClearOSDBuf(OSD_Blk1);
            uiOSDIconColorByXY(OSD_ICON_WARNING ,(OSDIconMidX-8), (OSDIconMidY-68), OSD_Blk1, 0x00 , alpha_3);
            uiOSDASCIIStringByColor(str, (OSDIconMidX-strlen(str)*OSD_STRING_W/2) , (OSDIconMidY-40), OSD_Blk1 , 0xC0, 0x00);
            OSTimeDly(20);
        }
        else if(mode ==2)  // Preview
        {
            uiOSDIconColorByXY(OSD_ICON_WARNING ,(OSDIconMidX-8), (OSDIconMidY-68), OSD_Blk2, 0x00 , alpha_3);
            uiOSDASCIIStringByColor(str, (OSDIconMidX-strlen(str)*OSD_STRING_W/2) , (OSDIconMidY-40), OSD_Blk2 , 0xC0, 0x00);
            OSTimeDly(20);
            uiClearOSDBuf(OSD_Blk2);
            osdDrawPreviewIcon();
        }
    }
}

void osdDrawFlashLight(u8 idx)
{
}

void osdDrawSDIcon(u8 on)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};
    UI_MULT_ICON *iconInfo;

#if( HW_BOARD_OPTION == MR6730_AFN )
    u32 OSDH;
	u32 OSDW;
	#if (HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)
	OSDH = 500;//PAL
	if(sysTVinFormat == TV_IN_NTSC){
		OSDH = ((OSDH*480)/576);
	}
	#else
    OSDH = 460;
    if(sysTVinFormat == TV_IN_NTSC){
        OSDH = ((460*480)/576);
    }
	#endif
#endif

    if (Main_Init_Ready == 0)
        return;

    uiOsdGetIconInfo(OSD_ICON_SD,&iconInfo);

    if (on == UI_OSD_DRAW){
	#if( HW_BOARD_OPTION == MR6730_AFN )
	#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH && NO_OSD_SD_ICON)
		return;
	#endif	

		//uiOSDIconColorByXY(OSD_ICON_SD ,(OSDIconEndX-60), OSDH, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
		#if (TVOUT_27MHZ)
		//OSDW=520;
		OSDW=(OSDDispWidth[sysTVOutOnFlag]-90);		
		uiOSDIconColorByXY(OSD_ICON_SD , OSDW, OSDH, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
		#else
		uiOSDIconColorByXY(OSD_ICON_SD ,(OSDDispWidth[sysTVOutOnFlag]-60), OSDH, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
		#endif
	#else
		uiOSDIconColorByXY(OSD_ICON_SD ,(OSDIconEndX-iconInfo->Icon_w-8), (OSDIconEndY-iconInfo->Icon_h), OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
	#endif
    }else
    {
        uiOsdGetIconInfo(OSD_ICON_SD,&iconInfo);
	 #if( HW_BOARD_OPTION == MR6730_AFN )
	   /*
	     uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, iconInfo->Icon_h,
                    (OSDIconEndX-60) , OSDH , OSD_BLK[sysTVOutOnFlag] , 0);
            */
		#if (TVOUT_27MHZ)
		//if(sysTVinFormat == TV_IN_NTSC)
		//OSDW=520;
		OSDW=(OSDDispWidth[sysTVOutOnFlag]-90);	
		uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, iconInfo->Icon_h,
						OSDW, OSDH , OSD_BLK[sysTVOutOnFlag] , 0); 			
		#else
		uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, iconInfo->Icon_h,
				(OSDDispWidth[sysTVOutOnFlag]-60), OSDH , OSD_BLK[sysTVOutOnFlag] , 0);             
		#endif
	 #else
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, iconInfo->Icon_h,
                (OSDIconEndX-iconInfo->Icon_w-8) , (OSDIconEndY-iconInfo->Icon_h) , OSD_BLK[sysTVOutOnFlag] , 0);
	 #endif
    }
}


#if( HW_BOARD_OPTION == MR6730_AFN )

void osdDrawMotionIcon(u8 on)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};
    UI_MULT_ICON *iconInfo;
    u32 OSDH;
	u32 OSDW;
#if (HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)
	OSDH = 500;//PAL
	if(sysTVinFormat == TV_IN_NTSC){
		OSDH = ((OSDH*480)/576);
	}
	
#else
    OSDH = 460;
    if(sysTVinFormat == TV_IN_NTSC){
        OSDH = ((460*480)/576);
    }
#endif
    uiOsdGetIconInfo(OSD_ICON_MOTION,&iconInfo);

    if (on == UI_OSD_DRAW){
		#if (TVOUT_27MHZ)
		OSDW=(OSDDispWidth[sysTVOutOnFlag]-120);
		uiOSDIconColorByXY(OSD_ICON_MOTION , OSDW, OSDH, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
		#else
        uiOSDIconColorByXY(OSD_ICON_MOTION ,(OSDDispWidth[sysTVOutOnFlag]-90), OSDH, OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
		#endif
	 //DEBUG_UI("----------Draw MotionIcon[%d]------------\n", OSD_ICON_MOTION);
    }else{
		#if (TVOUT_27MHZ)
		OSDW=(OSDDispWidth[sysTVOutOnFlag]-120);
		uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, iconInfo->Icon_h,
                OSDW, OSDH , OSD_BLK[sysTVOutOnFlag] , 0);
		#else      
		uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, iconInfo->Icon_h,
                (OSDDispWidth[sysTVOutOnFlag]-90), OSDH , OSD_BLK[sysTVOutOnFlag] , 0);
		#endif
	 //DEBUG_UI("----------Clear MotionIcon------------");
    }

}

#endif


void osdDrawPreviewIcon(void)
{
    char camera[5];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};

#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    return;
#endif

#if (UI_PREVIEW_OSD == 0)
    return;
#endif

#if (HW_BOARD_OPTION==MR6730_AFN) 

			
		
		if(gInsertCard == 1)
			osdDrawSDIcon(UI_OSD_DRAW);
			
		osdDrawMenuPreview();
	
		#if(HW_DERIV_MODEL!=HW_DEVTYPE_CDVR_PUSH)
		if(sysDualModeDisp)
		{//gather 2 sensor screen in one display
			#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN && CUSTOM_OSD_CAM_TEXT)
			sprintf(camera,"%s","A");
			#else		
			sprintf(camera,"CAM%d",sysVideoInCHsel);
			#endif
			uiOSDASCIIStringByColor(camera, UI_OSD_CAM_LOC_X, UI_OSD_CAM_LOC_Y,  OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);	

		#if DUAL_MODE_DISP_SUPPORT
			if(uiMenuVideoSizeSetting==UI_MENU_VIDEO_SIZE_640x480)
			{
			#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN && CUSTOM_OSD_CAM_TEXT)
				sprintf(camera,"%s","B");
			#else
				sprintf(camera,"CAM%d",sysVideoInCHsel+1);
			#endif	
				//uiOSDASCIIStringByColor(camera, UI_OSD_CAM_LOC_X+(640/2), UI_OSD_CAM_LOC_Y,  OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);	
				//uiOSDASCIIStringByColor(camera, UI_OSD_CAM_LOC_X+(OsdBuf_Width()/2), UI_OSD_CAM_LOC_Y,	OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);	
				uiOSDASCIIStringByColor(camera, (OsdBuf_Width()/2)+32, UI_OSD_CAM_LOC_Y,	OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);	
			}
		#endif 
		}
		else
		{			
		#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN && CUSTOM_OSD_CAM_TEXT)
			sprintf(camera,"%s",(sysVideoInCHsel==1)?"A":"B");
		#else
			if (sysVideoInCHsel == 0)
				sprintf(camera,"CAM%d",(sysRFRxInMainCHsel+1));
			else
				sprintf(camera,"CAM%d",sysVideoInCHsel);
		#endif
			uiOSDASCIIStringByColor(camera, UI_OSD_CAM_LOC_X, UI_OSD_CAM_LOC_Y,  OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);	
	
	
		#if DUAL_MODE_DISP_SUPPORT
			#if(!MULTI_CH_DEGRADE_1CH)
			//clear CAM2 mark on screen
			if(uiMenuVideoSizeSetting==UI_MENU_VIDEO_SIZE_640x480)
			{
				//sprintf(camera,"    ");//clear previous osd font 
				_SPACEs(camera,4)

				//uiOSDASCIIStringByColor(camera, UI_OSD_CAM_LOC_X+(640/2), UI_OSD_CAM_LOC_Y,  OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);	
				//uiOSDASCIIStringByColor(camera, UI_OSD_CAM_LOC_X+(OsdBuf_Width()/2), UI_OSD_CAM_LOC_Y,	OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);	
				uiOSDASCIIStringByColor(camera, (OsdBuf_Width()/2)+32, UI_OSD_CAM_LOC_Y,	OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);	
			}
			#endif
		#endif 
		}
		#endif	
		
		#if(DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)
		if(UI_isPreviewMode())
		{
		#if (IDUOSD_TIMESTAMP)
			extern s8 OverwriteStringEnable;
			if(OverwriteStringEnable)
			{	
				if(gPreviewInitDone==1)
				{								
				UI_IduOsd_DrawTimeStamp(UI_OSD_DRAW);				
				}				
			}
		#endif

		#if (SHOW_RES_ON_OSD)

			switch(setUI.ResolSel)
			{
			case UI_MENU_SETTING_RESOLUTION_HD:
				sprintf(camera,"%3s","HD");//sprintf(camera,"HD");
				break;
			case UI_MENU_SETTING_RESOLUTION_VGA:
				sprintf(camera,"%3s","VGA");//sprintf(camera,"VGA");
				break;
			default:
				sprintf(camera,"---");
				break;				
			}
			//uiOSDASCIIStringByColor(camera, (640-90), (480-48),  OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);	
			#if (TVOUT_27MHZ)
			uiOSDASCIIStringByColor(camera, 520, 432,  OSD_BLK[sysTVOutOnFlag], 0xC4, 0x00);
			#else			
			uiOSDASCIIStringByColor(camera, 550, 432,  OSD_BLK[sysTVOutOnFlag], 0xC4, 0x00);
			#endif
		#endif		
		}
		#endif


		
#else
		//orig-code

    if(MyHandler.MenuMode!=VIDEO_MODE)
        return;

    if (sysVideoInCHsel == 0)
        sprintf(camera,"CAM%d",(sysRFRxInMainCHsel+1));
    else
        sprintf(camera,"CAM%d",sysVideoInCHsel);

    if(gInsertCard == 1)
        osdDrawSDIcon(UI_OSD_DRAW);

    osdDrawMenuPreview();
    uiOSDASCIIStringByColor(camera, UI_OSD_CAM_LOC_X, UI_OSD_CAM_LOC_Y,  OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);
#endif 


#if(HW_BOARD_OPTION == MR6730_AFN)
    if( (setUI.RecMode == RECMODE_MOTION)  && (setUI.StopRec == 0 )  ){
        osdDrawMotionIcon(UI_OSD_DRAW);
    }else{
        osdDrawMotionIcon(UI_OSD_CLEAR);
    }
	/*
	    if( (MemoryFullFlag == TRUE) ){
	        osdDrawMemFull(UI_OSD_DRAW);
	    }
	*/
#endif

}

void osdDrawVideoIcon(void)
{
#if(LCM_OPTION == LCM_P_RGB_888_Innolux)//Lucian: have bug on OSD piority A1016. close it for test,7/11
   if (sysTVOutOnFlag==0)
	  uiOsdDisableAll();
#endif
	osdDrawMenuPreview();
}


void osdDrawVideoTime(void)
{
    char  timeForRecord1[10];
    char  timeForRecord2[10];
    u8    h, m, s;
    u16 x[2]={72,200};
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32 StrH;
    static u32 tmpRTCseconds = 0;
    static s64 tmpVideoNextPresentTime = 0;
    static  s64 Vtime;
    static  int i;

#if( HW_BOARD_OPTION == MR6730_AFN )
#if 1//(USE_PLAYBACK_AUTONEXT)
#define V_DUR_STA_MAX				5
	static u32 VideoPresnTm_prev=0;	
	static u32 VideoPresnTm_stat_cnt=0;	
	//	
	static u32 VideoPresnTm_Curr=0;
#endif
    u32 OSDH;
    OSDH = 470;
    if(sysTVinFormat == TV_IN_NTSC){
        OSDH = ((470*480)/576);
    }
#endif

    StrH = uiOsdGetFontHeight(CurrLanguage)+8;

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
	 #if( HW_BOARD_OPTION == MR6730_AFN )
	     uiOSDASCIIStringByColor((u8*)timeForRecord1, x[sysTVOutOnFlag] , 30 , OSD_Blk2 , 0xc0, 0x00);
	     DEBUG_UI("-----> REC : video time");
	 #else
        uiOSDASCIIStringByColor((u8*)timeForRecord1, x[sysTVOutOnFlag] , 11 , OSD_Blk2 , 0xc0, 0x00);
	 #endif
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
        #if IS_HECHI_DOORPHONE
        return;
        #endif
        #if IS_COMMAX_DOORPHONE
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
	#if( HW_BOARD_OPTION == MR6730_AFN )
		#if(DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)
		#if(IDUOSD_TIMESTAMP)
		OSDH+=20;
		#else
		OSDH+=7;
		#endif
		#endif
		uiOSDASCIIStringByColor((u8*)timeForRecord1, 408 , OSDH, OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
	#else
		uiOSDASCIIStringByColor((u8*)timeForRecord1, 408 , (OSDIconEndY-StrH), OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
	#endif
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


	
		#if (HW_BOARD_OPTION == MR6730_AFN)
		#if 1//(USE_PLAYBACK_AUTONEXT)
		//if(setUI.SYS_PlayOrder==1)
		//{

		//to recover X1 when fast forward/backward of playback
		if(video_playback_speed != (UI_PLAYBACK_SPEED_LEVEL/2))
		{

			VideoPresnTm_Curr=(u32)(VideoNextPresentTime/1000000);
			
			if((VideoPresnTm_prev==VideoPresnTm_Curr))
			{
			DEBUG_UI("[%s]-x-,(%d,%d)\n",(video_playback_speed < (UI_PLAYBACK_SPEED_LEVEL/2))?"REV":"FF",VideoPresnTm_Curr,(VideoDuration-3));
			
				if(VideoDuration)
				{
					u8 REV_Stop=0;
					u8 FF_Stop=0;

					FF_Stop=_CRITERIONS_CHK((video_playback_speed > (UI_PLAYBACK_SPEED_LEVEL/2)) && ((VideoPresnTm_Curr+10) >= (VideoDuration-3)));
					REV_Stop=_CRITERIONS_CHK((video_playback_speed < (UI_PLAYBACK_SPEED_LEVEL/2)) && (h==0&&m==0) );
					
					if( FF_Stop || REV_Stop )	
					{
					DEBUG_UI("(REV_Stop=%d,FF_Stop=%d)\n",REV_Stop,FF_Stop);
						VideoPresnTm_stat_cnt++;
						//DEBUG_UI("VideoDuration_stat_cnt=%d\n",VideoDuration_stat_cnt);
					}
				}

			}				
			else
			{
				VideoPresnTm_stat_cnt=0;
			}

			if( (VideoPresnTm_stat_cnt==V_DUR_STA_MAX) )
	        {		
	        	VideoPresnTm_stat_cnt=0;//reset
	        	
				DEBUG_UI("== X1 ==\n");

				if(sysPlaybackVideoStart)
				{
					//back to X1
					#if (PLAYBACK_FF_REV)
						if(curr_playback_speed != PLAYBACK_SPEED_MID)	
						{
							DEBUG_UI("UI playback speed return to X1 \r\n");
							//set playback speed to X1								
							curr_playback_speed = PLAYBACK_SPEED_MID;
							uiOsdDrawPlaybackPlaySpeed();  
						}
					#else
					if(curr_playback_speed != (UI_PLAYBACK_SPEED_LEVEL/2))	
						curr_playback_speed = (UI_PLAYBACK_SPEED_LEVEL/2);
					#endif	
				}

			}


		}

		//_APP_ENTER_CS_;
		VideoPresnTm_prev=VideoPresnTm_Curr;
		//_APP_EXIT_CS_;

		//}//if(setUI.SYS_PlayOrder==1)			
		#endif
		#endif


			
        if((video_playback_speed == 5) && ((VideoNextPresentTime / 1000000 + 4) >= VideoDuration) && (i > 1))
        {
            strcpy(timeForRecord2, &timeForRecord1[1]);
			

			//DEBUG_UI("i=%d\n",i);


			#if( (HW_BOARD_OPTION == MR6730_AFN)&&(USE_PLAYBACK_AUTONEXT) )
			if(i == 6) // wait 3 sec (500ms*6)
			{
				if(setUI.SYS_PlayOrder==1)
				{
					//if((sysPlaybackVideoStart||sysPlaybackVideoPause)&&sysPlaybackVideoStop==0)
					if((sysPlaybackVideoStart==1)&&sysPlaybackVideoStop==0)
					{
						u8 rty=3;
			            while(sysPlaybackVideoStart&&(rty--))
			            {
			           		//stop current file first
							if(uiPlaybackStop(GLB_ENA) == 1)
				            {
				                osdDrawPlayIndicator(100);
				                uiOsdVolumeControl(PLAYBACK_MODE, UI_VALUE_CLEAN);
				            }			            
							//
							if(sysPlaybackVideoStart==0)
								break;
			            	OSTimeDlyHMSM(0,0,0,200);			            
						}
						
						#if (PLAYBACK_FF_REV)
							if(curr_playback_speed != PLAYBACK_SPEED_MID)	
							{
								DEBUG_UI("UI playback speed return to X1 \r\n");
								//set playback speed to X1								
								curr_playback_speed = PLAYBACK_SPEED_MID;
								uiOsdDrawPlaybackPlaySpeed();  
							}
						#else
							curr_playback_speed = (UI_PLAYBACK_SPEED_LEVEL/2);
						#endif						
					
					}

					
					DEBUG_UI(">>UI_KEY_B_RIGHT<<\n");
					uiSentKeyToUi(UI_KEY_B_RIGHT);
					/*
					UIKey = UI_KEY_B_RIGHT;  //play next file
					OSSemPost(uiSemEvt);
					*/
				}else
				{
					uiSentKeyToUi(UI_KEY_STOP);				
				}
			}
			#else
			if(i == 6)   // 停三秒回到 playback list			
				uiSentKeyToUi(UI_KEY_STOP);
			#endif            
                
        }
        else if((video_playback_speed > 5) && ((((VideoNextPresentTime / 1000000 + 6) >= VideoDuration) && (i > 1)) || (((VideoNextPresentTime / 1000000 + 9) >= VideoDuration) && (i > 4))))
        {
            strcpy(timeForRecord2, &timeForRecord1[1]);
        }
        else
        {
            sprintf (timeForRecord2, "%02d:%02d:%02d", h, m, s);
        }

#if ( HW_BOARD_OPTION == MR6730_AFN)
	//#if (HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)	
	#if(USE_PLAYBACK_AUTONEXT)
	if(setUI.SYS_PlayOrder==1)		
	{
		if(MyHandler.MenuMode==PLAYBACK_MODE)
		{
			if (PlayListDspType == UI_DSP_PLAY_LIST_PLAYBACK)
			{
				//DEBUG_UI("== V_TM %d,%d,%d ==\n",sysPlaybackVideoStart,sysPlaybackVideoStop,uiCheckPlayback());
				if( sysPlaybackVideoStart==0 || (uiCheckPlayback()==0) )
				{
				DEBUG_UI("== SKIP osdDrawVideoTime ==\n");
				return;
				}				
			}
		}
	}			
	#endif
#endif


		
#if (USE_BIG_OSD == 1)
	#if( HW_BOARD_OPTION == MR6730_AFN )
		uiOSDASCIIStringByColor((u8*)timeForRecord2, 408-128 , OSDH , OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
	#else
		uiOSDASCIIStringByColor((u8*)timeForRecord2, 408-128 , (OSDIconEndY-StrH) , OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
	#endif
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
    if (type == 100)   /* Playback Video 結束時, PAUSE 符號消失*/
    {
        uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 20 , 25 , 70 , 30 , OSD_BLK[sysTVOutOnFlag] , 0);
        uiOSDIconColorByXY(OSD_ICON_STOP ,70 , 30 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    }
    else
    {
        if(Iframe_flag==1)
        {
            uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 20 , 25 , 70 , 30 , OSD_BLK[sysTVOutOnFlag] , 0);
            uiOSDIconColorByXY(OSD_ICON_STOP ,70 , 30 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        }

        else if(Iframe_flag==0  && sysPlaybackVideoPause==0)
            uiOSDIconColorByXY(OSD_ICON_PLAYBACK ,70 , 30 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
        else if(Iframe_flag==0  && sysPlaybackVideoPause==1)
             uiOSDIconColorByXY(OSD_ICON_PAUSE ,70 , 30 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
    }
}
void osdDrawFileNum(u32 num)  // TVOUT-flag
{
    u8 filestring[20];
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32 StrH;

#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
     return;
#endif

#if( HW_BOARD_OPTION == MR6730_AFN )
    u32 OSDH;
    OSDH = 470;
    if(sysTVinFormat == TV_IN_NTSC){
        OSDH = ((470*480)/576);
    }
#endif

    StrH = uiOsdGetFontHeight(CurrLanguage)+8;
    if(global_totalfile_count<=9999)
        sprintf((char*)filestring,"%04d/%04d",num,global_totalfile_count);
    else if((global_totalfile_count>=10000)&&(global_totalfile_count<=99999))
        sprintf((char*)filestring,"%05d/%05d",num,global_totalfile_count);
    else if((global_totalfile_count>=100000)&&(global_totalfile_count<=999999))
        sprintf((char*)filestring,"%06d/%06d",num,global_totalfile_count);

#if (USE_BIG_OSD == 1)
    #if( HW_BOARD_OPTION == MR6730_AFN )
	#if(DERIVATIVE_MODEL==MODEL_TYPE_AFN720PSEN)
	#if(IDUOSD_TIMESTAMP)
	OSDH+=20;
	#else
	OSDH+=7;
	#endif
	#endif
	uiOSDASCIIStringByColor(filestring, 50 , OSDH, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
    #else
    uiOSDASCIIStringByColor(filestring, 12 , (OSDIconEndY-StrH), OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
    #endif
#else
    uiOSDASCIIStringByColor(filestring, 12 , 36, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
#endif
}

void osdDrawPlayIcon(void)
{
     u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
     return;
#endif
#if (USE_BIG_OSD == 1)
    #if( HW_BOARD_OPTION == MR6730_AFN)
	//uiOSDIconColorByXY(OSD_ICON_PLAY ,(OSDDispWidth[sysTVOutOnFlag]-60), 30, OSD_BLK[sysTVOutOnFlag], 0xC0 , alpha_3);
    {
    	char txtbuf[8]="";
		
	#if(USE_PLAYBACK_AUTONEXT)
	if(setUI.SYS_PlayOrder==1)		
	{
		if(MyHandler.MenuMode==PLAYBACK_MODE)
		{
			//DEBUG_UI("-mode-%d\n",MyHandler.MenuMode);
			if (PlayListDspType == UI_DSP_PLAY_LIST_PLAYBACK)
			{
				//DEBUG_UI("-ccc-\n");

				if(dcfPlaybackCurFile->pDirEnt->d_name)
				{
					if(dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_JPG)
					{
						if(sysThumnailPtr->type!=0)
						{
							sysThumnailPtr->type=0;//"JPG"
							//DEBUG_UI("sysThumnailPtr->type changed(%d)\n",sysThumnailPtr->type);
						}
					}
					else if(dcfPlaybackCurFile->fileType == DCF_FILE_TYPE_ASF)
					{
						if(sysThumnailPtr->type!=1)
						{
							sysThumnailPtr->type=1;//"ASF"	
							//DEBUG_UI("sysThumnailPtr->type changed(%d)\n",sysThumnailPtr->type);
						}
					}
				}
			
			}
		}
	}			
	#endif
	
		if(sysThumnailPtr->type==0)//"JPG"
		{
			sprintf(txtbuf,"PIC");
			//uiOSDASCIIStringByColor(txtbuf, (OSDDispWidth[sysTVOutOnFlag]-96), 36,  OSD_BLK[sysTVOutOnFlag], 0xC3, alpha_3);	
			uiOSDASCIIStringByColor(txtbuf, (OSDDispWidth[sysTVOutOnFlag]-96), 36, OSD_BLK[sysTVOutOnFlag], 0xC0, alpha_3);	
		}
		else
		{//it should be "ASF"
			sprintf(txtbuf,"MOV");
			//uiOSDASCIIStringByColor(txtbuf, (OSDDispWidth[sysTVOutOnFlag]-96), 36,  OSD_BLK[sysTVOutOnFlag], 0xC4, alpha_3);	
			uiOSDASCIIStringByColor(txtbuf, (OSDDispWidth[sysTVOutOnFlag]-96), 36, OSD_BLK[sysTVOutOnFlag], 0xC0, alpha_3);	
		}
    }
    #else
    uiOSDIconColorByXY(OSD_ICON_PLAY ,(OSDDispWidth[sysTVOutOnFlag]-24), 12, OSD_BLK[sysTVOutOnFlag], 0xC0 , alpha_3);
    #endif
#else
    uiOSDIconColorByXY(OSD_ICON_PLAY ,(OSDDispWidth[sysTVOutOnFlag]-24), 12, OSD_Blk0, 0xC0 , alpha_3);
#endif
    osdDrawSDIcon(UI_OSD_DRAW);
    osdDrawFileNum(playback_location);
    uiOsdDrawPlayTime(sysThumnailPtr->type,VideoDuration-3);
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
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};
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
}

void osdDrawDelMsg(s8* msg,u32 index)        /* index 0 means delete one */
{
    u8 num_string[12];
    u8  OSD_BLK[2] = {OSD_Blk1, OSD_Blk0};
    if(index!=0)
    {
        sprintf((char*)num_string,"%05d/%05d",index,global_totalfile_count);
        uiOSDASCIIStringByColor(num_string, 16 , 128, OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
    }
    else
    {
        sprintf((char*)num_string,"%05d/%05d",1,1);
        uiOSDASCIIStringByColor(num_string, 16 , 128, OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
    }
    uiOSDASCIIStringByColor((u8*)msg, 16 , 148+(osdYShift/2), OSD_BLK[sysTVOutOnFlag] , 0xc0, 0x00);
}

void osdDrawVideoOn(u8 on)
{
    u8 OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};
	

	#if( HW_BOARD_OPTION == MR6730_AFN )
	#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)
	u16 Pos_x=0,Pos_y=0;
	u32 LineColor=0;
	u16 font_h=0;
	u16 font_len=0;
	u16 line_w=4;

	////for VGA
	Pos_x=520+30;
	Pos_y=40;
	font_h=uiOsdGetFontHeight(0);
	font_len=uiOsdGetFontWidth(0);
	if(font_len)
		font_len*=3;//o+REC
	
	#endif
	#endif

    switch(on)
    {
        case UI_OSD_CLEAR:
            #if (UI_PREVIEW_OSD == 1)
                #if (USE_BIG_OSD == 1)
		      #if( HW_BOARD_OPTION == MR6730_AFN )
				#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)
				osdDrawMessage(MSG_REC, Pos_x, 40, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);	
				LineColor=0x00000000;
				uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 14, 14, Pos_x-4-14, Pos_y+3, OSD_BLK[sysTVOutOnFlag], LineColor);

				#else
				#if (TVOUT_27MHZ)
				uiOSDIconColorByXY(OSD_ICON_VREC ,470 , 30 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_0);
				osdDrawMessage(MSG_REC, 490, 30, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);				
				#else				
				uiOSDIconColorByXY(OSD_ICON_VREC ,500 , 30 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_0);
				osdDrawMessage(MSG_REC, 520, 30, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
				#endif
				#endif
		      #else
                          uiOSDIconColorByXY(OSD_ICON_VREC ,380 , 25 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_0);
                          osdDrawMessage(MSG_REC, 400, 25, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
		      #endif
                #else
                    uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , 92 , 24 , 8 , 180+osdYShift , OSD_BLK[sysTVOutOnFlag] , 0);
                #endif
            #endif
            break;

        case UI_OSD_DRAW:
            #if (UI_PREVIEW_OSD == 1)
                #if (USE_BIG_OSD == 1)
		      #if( HW_BOARD_OPTION == MR6730_AFN )
				#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)
				osdDrawMessage(MSG_REC, Pos_x, 40, OSD_BLK[sysTVOutOnFlag], 0xC2, 0x00);			
				LineColor=0xC2C2C2C2;
				uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], 14, 14, Pos_x-4-14, Pos_y+3, OSD_BLK[sysTVOutOnFlag], LineColor);
				
				#else
				#if (TVOUT_27MHZ)
				uiOSDIconColorByXY(OSD_ICON_VREC ,470 , 30 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
				osdDrawMessage(MSG_REC, 490, 30, OSD_BLK[sysTVOutOnFlag], 0xC2, 0x00);				
				#else
				uiOSDIconColorByXY(OSD_ICON_VREC ,500 , 30 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
				osdDrawMessage(MSG_REC, 520, 30, OSD_BLK[sysTVOutOnFlag], 0xC2, 0x00);
				#endif
				#endif
		      #else
                          uiOSDIconColorByXY(OSD_ICON_VREC ,380 , 25 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
                          osdDrawMessage(MSG_REC, 400, 25, OSD_BLK[sysTVOutOnFlag], 0xC2, 0x00);
		      #endif
                #else
                    uiOSDIconColorByXY(OSD_ICON_VREC ,8 , 180+osdYShift , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
                    osdDrawMessage(MSG_REC, 28, 180+osdYShift, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
                #endif
            #endif
            break;

        case UI_OSD_NONE:
            #if (UI_PREVIEW_OSD == 1)
                uiOSDIconColorByXY(OSD_ICON_VREC ,8 , 180+osdYShift , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
                osdDrawMessage(MSG_DETECTING, 28, 180+osdYShift, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
            #endif
       //     RTCseconds = 0; /*in detecting do not record and set rec seconds = 0*/
            break;
    }
}

void osdDrawMemFull(u8 act)
{
#if (HW_BOARD_OPTION == MR6730_AFN)
	
		UI_MULT_ICON *iconInfo;
	
		if (act == UI_OSD_DRAW)
		{
			uiOSDIconColorByXY(OSD_ICON_WARNING ,(OSDDispWidth[sysTVOutOnFlag]-16)/2, (OSDIconMidY-22), OSD_Blk2, 0x00 , alpha_3);
	 #if( HW_BOARD_OPTION == MR6730_AFN)
			osdDrawMessage(MSG_MEMORY_FULL, (OSDIconMidX-66), (OSDIconMidY+50), OSD_Blk2, 0xC0, 0x00);
	 #else
			osdDrawMessage(MSG_MEMORY_FULL, 28, 126+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
	 #endif
		}else
		if( act == UI_OSD_CLEAR ){
		 //clear wraning icon
		 uiOsdGetIconInfo(OSD_ICON_WARNING,&iconInfo);
			uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , iconInfo->Icon_w, iconInfo->Icon_h,
					(OSDDispWidth[sysTVOutOnFlag]-16)/2, (OSDIconMidY-22) , OSD_Blk2 , 0);
	
		//clear memfull
		//uiOSDIconColorByXY(MSG_MEMORY_FULL ,500 , 30 , OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_0);
		   osdDrawMessage(MSG_MEMORY_FULL, (OSDIconMidX-66), (OSDIconMidY+50), OSD_Blk2, 0x00, 0x00);
		}
	
	
#else
	//orig-code

    if (act == UI_OSD_DRAW)
    {
        uiOSDIconColorByXY(OSD_ICON_WARNING ,(OSDDispWidth[sysTVOutOnFlag]-16)/2, (OSDIconMidY-22), OSD_Blk2, 0x00 , alpha_3);
	 #if( HW_BOARD_OPTION == MR6730_AFN)
		osdDrawMessage(MSG_MEMORY_FULL, (OSDIconMidX-66), (OSDIconMidY+50), OSD_Blk2, 0xC0, 0x00);
	 #else
		osdDrawMessage(MSG_MEMORY_FULL, 28, 126+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
	 #endif
    }
#endif		
}

void osdDrawSDCD(u8 i)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};

#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    return;
#endif
    if(i == 1)
    {
        uiClearOSDBuf(2);
        osdDrawMessage(MSG_SD_INIT, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x41);
    }
    else
        osdDrawMessage(MSG_SD_INIT, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0x00, 0x00);
}

void osdDrawFillEmpty(void)
{
   (*OSDDisplay[sysTVOutOnFlag])(OSD_Blk0, 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_Blk0);
    uiOsdEnable(OSD_Blk0);
    uiOSDIconColorByXY(OSD_ICON_WARNING ,(OSDIconMidX-8), (OSDIconMidY-22), OSD_Blk0, 0x00 , alpha_3);
    uiOSDMultiLanguageStrCenter(MSG_NO_FILE, OSD_Blk1 , 0xC0, 0xC1);
    uiOSDASCIIStringByColor("0000/0000", (OSDIconEndX-80), (OSDIconEndY-20), OSD_Blk0, 0xc0, 0x00);
    if(gInsertCard==1)
        uiOSDIconColorByXY(OSD_ICON_SD ,(OSDIconEndX-28), (OSDIconEndY-40), OSD_Blk0, 0x00 , alpha_3);
    else
        uiOSDIconColorByXY(OSD_ICON_M ,(OSDIconEndX-28), (OSDIconEndY-25), OSD_Blk0, 0xCC , alpha_3);
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
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};

    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    osdDrawPreviewIcon();
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    uiOSDIconColorByXY(OSD_ICON_WARNING ,(OSDIconMidX-8), (OSDIconMidY-32), OSD_BLK[sysTVOutOnFlag], 0x00 , alpha_3);
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

void uiOsdDrawInsertSD(u8 buf_idx)
{
    uiOSDIconColorByXY(OSD_ICON_WARNING ,(OSDIconMidX-8), (OSDIconMidY-22), buf_idx, 0x00 , alpha_3);
    osdDrawMessage(MSG_INSERT_SD_CARD, CENTERED, 126+osdYShift/2, buf_idx, 0xC0, 0x41);
}

void uiOsdDrawBitRate(u32 value)
{
    u8 bitRateStr[6];
    u8 signal=0;
    u8 camera=1;
    u8 i;
    u8 maxbitrate[]={6,3,2,1,5};
#if(SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M)    
    const u8 TX2RX1TABLE[5]={0,1,4,7,15};
    const u8 TX1RX1TABLE[5]={0,5,11,17,32};
#else
    const u8 TX2RX1TABLE[5]={0,1,4,7,11};
    const u8 TX1RX1TABLE[5]={0,5,11,17,23};
#endif
    
#if(HW_BOARD_OPTION == MR8120_RX_DEMO_BOARD)
    signal=value/maxbitrate[MAX_BITRATE_2TX];
    uiOsdDrawCamera();//for demo
#elif(HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C)
    for(i=0; i< 5; i++)
    {
        if(value >TX2RX1TABLE[i])
            signal = i;
    }
#else
    for(i=0; i< 5; i++)
    {
        if(value >TX1RX1TABLE[i])
            signal = i;
    }
#endif

    sprintf ((char *)bitRateStr, "B:%d.%d", (value/10), (value%10));
#if (HW_BOARD_OPTION == MR8200_RX_ZINWELL)
    uiOSDASCIIStringByColor(bitRateStr, 20, 180, OSD_Blk2, 0xc6, 0x00);
#endif

#if((HW_BOARD_OPTION == MR8120_TX_RDI2)|| (HW_BOARD_OPTION == MR8120_TX_RDI3)||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_AV) || (HW_BOARD_OPTION == MR8120_TX_GCT) ||\
    (HW_BOARD_OPTION == MR8120_TX_JIT2) || (HW_BOARD_OPTION == MR8120_TX_FRESHCAM)||\
    (HW_BOARD_OPTION == MR8120_TX_MAYON_LW604)||(HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C))
    if(rfiuRX_CamPerRF == 2)
    {
        for(i=0; i< 5; i++)
        {
            if(value >TX2RX1TABLE[i])
                signal = i;
        }        
    }
    else
    {
        for(i=0; i< 5; i++)
        {
            if(value >TX1RX1TABLE[i])
                signal = i;
        }     
    }

     
    uiOsdDrawAtennaByTX(signal,camera);
#else
    uiOsdDrawAtenna(signal);
#endif


}

void uiOsdDrawFrameRate(u32 value)
{
    u8 FrameRateStr[6];

    sprintf ((char *)FrameRateStr, "F:%2d", value);
    #if (HW_BOARD_OPTION == MR8200_RX_ZINWELL)
    uiOSDASCIIStringByColor(FrameRateStr, 20, 200, OSD_Blk2, 0xc6, 0x00);
    #endif
}


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
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};
    #if (UI_PREVIEW_OSD == 1)
        if (sysTVOutOnFlag)
        {
            uiMenuOSDReset();
            iduTVOSDDisplay(OSD_BLK[sysTVOutOnFlag], 0, 0, TVOSD_SizeX, TVOSD_SizeY);
						#if (HW_BOARD_OPTION==MR6730_AFN)
							UI_Adjust_TvOSD2Pos(1);
						#endif	            
            #if((HW_BOARD_OPTION != MR8120_RX_JESMAY ) && (HW_BOARD_OPTION != MR8120_RX_SKYSUCCESS)  && (HW_BOARD_OPTION != MR8120_RX_RDI))
            osdDrawPreviewIcon();
            #endif
            uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
        }
        else
        {
            uiMenuOSDReset();
            iduOSDDisplay1(OSD_BLK[sysTVOutOnFlag], 0, 0, PANNEL_X, PANNEL_Y);
            osdDrawPreviewIcon();
            uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);

        }
    #endif
}
#if (RFIU_SUPPORT)

void uiOsdDrawChangeResolution(u8 act)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
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

void osdDrawQuadIcon(void)
{

}

void uiOsdDrawQuadNoSignal(u8 act, u8 Camid)
{
    u8 i, OnNum = 4, right = 0;
    u16 DrawX, DrawY;
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
    u32 DrawColor;

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
        OnNum -= (iconflag[UI_MENU_SETIDX_CH1_ON+i]-UI_MENU_SETTING_CAMERA_ON);
        if ((i > Camid)&&(iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON))
            right = 1;
    }
    if (OnNum > 2 ) /*quad mode*/
    {
        if ((Camid%2) == 0)
            DrawX = 0;
        else
            DrawX = OSDIconMidX;
        if (Camid < 2)
            DrawY = 0;
        else
            DrawY = OSDIconMidY;
        //uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], OSDIconMidX, OSDIconMidY, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag],DrawColor);
    }
    else    /*half*/
    {
        if ((right == 1)|| (OnNum == 1))  /*camera in left*/
            DrawX = 0;
        else
            DrawX = OSDIconMidX;
        DrawY = 0;
        //uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag], OSDIconMidX, OSDIconMidY, DrawX, DrawY, OSD_BLK[sysTVOutOnFlag],DrawColor);
    }
    if (act == UI_OSD_DRAW)
        uiClearFfQuadBuf(Camid);
    else
        osdDrawQuadIcon();
}
#endif

void osdDrawRemoteOn(u8 on)
{

}

void uiOsdDrawBattery(u8 level)
{
}

void uiOsdDrawNewFile(void)

{
}

void uiOsdDrawNetworkLink(u8 LinkUp)
{

}

void uiOsdDrawAllPreviewIcon(void)
{
}

void uiOsdDrawSDCardFail(u8 act)
{

}

void uiOsdDrawSDCardFULL(u8 act)
{

}


//------------------------------------------------------------------------------
#if(HW_BOARD_OPTION == MR6730_AFN)

void uiOsdDrawSysAfterRec(void* pData)
{
    u8  RecId, i;
    VIDEO_CLIP_OPTION* pVideoClipOption;

    pVideoClipOption = (VIDEO_CLIP_OPTION*)pData;
    if (pVideoClipOption->VideoChannelID > MULTI_CHANNEL_LOCAL_MAX)
    {
        DEBUG_UI("uiOsdDrawSysAfterRec Error ID %d\r\n",pVideoClipOption->VideoChannelID);
        return;
    }

    //DEBUG_UI("clear osdDrawVideoOn");
    if ((MyHandler.MenuMode == VIDEO_MODE) )//|| (MyHandler.MenuMode == ZOOM_MODE))
    {
         osdDrawVideoOn(UI_OSD_CLEAR);
				//DEBUG_UI("clear osdDrawVideoOn---2");
    }
	#if(HW_BOARD_OPTION == MR6730_AFN)
	 if( UI_isMemoryFull() || ( gInsertCard==0)	){ // || (setUI.RecMode == RECMODE_MOTION ) ){
	#else
	 if( (setUI.RecMode == RECMODE_MOTION) || (MemoryFullFlag == TRUE) ){
	#endif 
    
        if (pVideoClipOption->VideoChannelID == 1){
	     setUI.CH1Recing = 0;
	 }
	 if (pVideoClipOption->VideoChannelID == 2){
	     setUI.CH2Recing = 0;
	 }
    }

}

//-------------------------------------------------------------------------------

void uiOsdPrintStates(void)
{
     switch(MyHandler.MenuMode)
    {
        case VIDEO_MODE:
            if(UI_isVideoRecording()==false){
		  if( setUI.RecMode == RECMODE_MOTION ){
		      printf("[[ Detecting ]]\n");
		  }else{
                    printf("[[ preview ]]\n");
		  }
            }else{
                printf("[[ REC ]]\n");                    
            }
            break;
            
        case PLAYBACK_MENU_MODE:
	     if( MenuMode_isActive()){
	          printf("[[ menu -- %s ]]\n", MenuMode_nowMenuName() );
	     }else
	     if( UI_isSDFormatMode() ){
	         printf("[[ SD Format ]]\n");
	     }else{
                printf("[[ playbacklist %s ]]\n",dcfPlaybackCurFile->pDirEnt->d_name);
	     }
            break;

        case PLAYBACK_MODE:
            printf("[[ %s ]]\n",dcfPlaybackCurFile->pDirEnt->d_name);
            break;
                       
        default:
            break;
            }
}


void uiOsdDrawPowerOff(void)
{
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
   
    (*OSDDisplay[sysTVOutOnFlag])(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
    uiClearOSDBuf(OSD_BLK[sysTVOutOnFlag]);
    uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
    osdDrawMessage(MSG_POWER_OFF, CENTERED, CENTERED, OSD_BLK[sysTVOutOnFlag], 0xC0, 0x00);    
}



void uiOsdDrawPlaybackMenuDeleteMsg(u8 act)
{
#if (USE_BIG_OSD == 1)
	u8	TxtMsg[11+1]="";  /*deleting...*/
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
	
	if (act == UI_OSD_DRAW){
		sprintf(TxtMsg, "%s", "deleting...");
		uiOSDASCIIStringByColor(TxtMsg, 60 , 408, OSD_BLK[sysTVOutOnFlag] , 0xC0, 0xC1);
	}
	else{
		u32 StrW = uiOsdGetFontWidth(CurrLanguage)+2;
		u32 StrH = uiOsdGetFontHeight(CurrLanguage)+8;		
		uiMenuOSDFrame(OSDDispWidth[sysTVOutOnFlag] , StrW*11 , StrH , 60 , 408, OSD_BLK[sysTVOutOnFlag] , 0);

	}
	OSTimeDlyHMSM(0,0,0,200);//<-- for human visual feeling

#else
  //...
#endif
}



#endif //#if 0
//-------------------------------------------------------------------------------
