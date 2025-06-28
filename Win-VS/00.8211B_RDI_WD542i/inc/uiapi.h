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
#define FLAGUI_HOMERF_COMMAND_FINISH    0x00000800
#define FLAGUI_UI_CHANGE_SET            0x00001000
#define FLAGUI_UI_OSD_SET               0x00002000 // for Model:Sem "Center Alert OSD"

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


/*P2P related*/
#define UI_P2P_PSW_MAX_LEN 33


/*Message Type for multi-language*/
#define CENTERED      0xFFFF

/* Status Flag */
enum
{
    UI_SET_RF_OK = 0,
    UI_SET_RF_BUSY,
    UI_SET_RF_NO_NEED,
    UI_SET_RF_SNAP_DONE,
    UI_SET_RF_SNAP_INIT
};

typedef enum 
{
    MSG_INSERT_SD_CARD = 0, /*Insert SD Card!*/
    MSG_SD_INIT,            /*SD Card Init...*/
    MSG_NO_FILE,            /*No File !*/
    MSG_WAITING,            /*Waiting*/
    MSG_PLEASE_WAIT,        /*Please Wait...*/
    MSG_REC,                /*REC*/
    MSG_DETECTING,          /*Detecting*/
    MSG_WRITE_PROTECT,      /*Write Protect*/
    MSG_FORMATING,          /*Formating...*/
    MSG_FORMAT_OK,          /*Format OK*/
    MSG_FORMAT_FAIL,        /*Format Fail*/
    MSG_TIME_OUT,           /*Time Out*/
    MSG_MEMORY_FULL,        /*MEMORY FULL !*/
    MSG_VOLUME,             /*VOLUME*/
    MSG_TIME_ERROR,         /*Time Set Error!*/
    MSG_USB_MASS_STORAGE,   /*USB Mass Storage!*/
    MSG_CARD_ERROR,         /*SD Card Error,*/
    MSG_UPDATE_EXECUTE,     /*SD card upgrade now…*/
    MSG_UPDATE_PASS,        /*UPDATE PASS !*/
    MSG_UPDATE_FAIL,        /*UPDATE FAIL !*/
    MSG_FAT_HEADER_ERROR,   /*FAT Header Error !*/
    MSG_RE_FORMATE,         /*Please Re-format*/
    MSG_CARD_STILL_FAIL,    /*Card Still Fail*/
    MSG_CHANGE_SD_CARD,     /*Please change another SD card*/
    MSG_FS_OPERATION_ERROR, /*FS Operation Error*/
    MSG_CHECK_WRITE_PROTECT,/*Check SD write protect*/
    MSG_SD_HW_ERROR,        /*SD Card H/W Error*/
    MSG_FW_UPDATE_PASS,     /*FW Updated Pass!*/
    MSG_FW_UPDATE_PASSQ,    /*FW Updated Pass Two!*/
    MSG_FW_UPDATE_FAIL,     /*FW Updated Fail!*/
    MSG_CHECK_UI,           /*Check UI Library!*/
    MSG_UI_UPDATE_PASS,     /*UI Updated Pass!*/
    MSG_UI_UPDATE_FAIL,     /*UI Updated Fail!*/
    MSG_NO_UI_LIBRARY,      /*No UI Library, Escape Updating!*/
    MSG_FILE_ERROR,         /*File Error!*/
    MSG_FILE_NAME,          /*File Name*/
    MSG_DATE,               /*Date*/
    MSG_START_TIME,         /*Start Time*/
    MSG_PAGE,               /*PAGE*/
    MSG_WEEKDAY_SUN,            /*WEEKDAY */  
    MSG_WEEKDAY_MON,            /*WEEKDAY */
    MSG_WEEKDAY_TUE,            /*WEEKDAY */
    MSG_WEEKDAY_WED,            /*WEEKDAY *//*40*/
    MSG_WEEKDAY_THU,            /*WEEKDAY */  
    MSG_WEEKDAY_FRI,            /*WEEKDAY */
    MSG_WEEKDAY_SAT,            /*WEEKDAY */        
    MSG_WEEKLY,             /*WEEKLY*/ 
    MSG_BEGIN_TIME,
    MSG_END_TIME,                           
    MSG_ONCE,
    MSG_OFF,                /*OFF*/
    MSG_ABNORMAL_OPERATION,
    MSG_FOLDER_NAME,
    MSG_DATE_TIME,
    MSG_TIME_ZONE,
    MSG_NO_SIGNAL,
    MSG_CARD_USED,
    MSG_CARD_TOTAL,
    MSG_CHANGE_RESOLUTION,    /*CHANGE RESOLUTION...*/
    MSG_POWER_OFF,
    MSG_RESTORE_DEFAULT_SETTINGS, /*RESTORE DEFAULT SETTINGS...*/
    MSG_ENTER_SCAN_MODE,    /*ENTER SCAN MODE*/
    MSG_CHECK_CARD,         /*Please Check or Format the SD Card.*/
#if ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    MSG_CHECK_CARD2,         /*Please Check or Format the SD Card.*/
#endif
    MSG_CHECK_CARD_II,      /*Please Check or Format the SD Card.*/
    MSG_NO_ZOOM_FOR_HD,     /*No Zoom Function for HD Resolution.*/
    MSG_CARD_FAILED,        /*Card Error! Retry please.*/
    MSG_NETWORK_FAILED,     /*Network Connection Failed!*/
    MSG_CHECK_NETWORK,      /*Please check your network connection.*/
    MSG_FW_UPTO_DATE,       /*Your firmware is up to date.*/
    MSG_FW_WAIT,            /*Please wait*/
    MSG_FW_REBOOTS,         /*until the unit reboots*/
    MSG_DOWNLOAD_FAILED,    /*Download failed*/
    MSG_PLEASE_TRY,         /*Please try again*/
    MSG_CARD_FULL,          /*SD Card Full!*/
    MSG_SURE_WANT,          /*ARE YOU SURE YOU WANT TO*/  
    MSG_DELETE_STORED,      /*DELETE ALL STORED FILES?*/   
    MSG_DELETE_FILE,        /*DELETE THIS FILE?*/   
    MSG_NO,                 /*NO*/  
    MSG_YES,                /*YES*/   
    MSG_PRESS_PAIR,         /*PLEASE PRESS PAIR KEY ON*/  
    MSG_CAMEA_SIDE,         /*CAMERA SIDE*/
    MSG_NO_SD,              /*NO SD CARD*/
    MSG_NO_FIRMWARE,        /*No Firmware*/
    MSG_PLZ_INPUT_AGAIN,    /*Wrong Password!Please input again*/
    MSG_PRE_PAIR_KEY_ONSS,    /*Please Press Pair Key on Sensor Side*/
    MSG_ASCII_STR,          /*other string not define*/
    MSG_SYSTEM_REBOOT,
    MSG_SYSTEM_UPDATING,
    MSG_PLZ_RESTART,        /*PLEASE RESTART MONITOR*/
    MSG_AREC_CANCEL,        /*Triggered recording will be canceled*/
#if ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    MSG_AREC_CANCEL_2,      /*Triggered recording will be canceled Line two*/
#endif    
    MSG_VOL_CONTROL,        /**/
#if((HW_BOARD_OPTION == MR8120_RX_RDI_M703)&&(PROJ_OPT == 2))
    MSG_FORMAT,
    MSG_FORMAT2,
#endif
    MSG_DOORBELL_PAIR,
    MSG_DOORBELL_UNPAIR,
    MSG_DOORBELL_PAIRING,
    MSG_MAX_NUM,             /*keep in the last*/
}MSG_SRTIDX;

#if((HW_BOARD_OPTION == MR8100_RX_RDI_SEM) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP) \
    ||(HW_BOARD_OPTION == MR8100_GCT_VM9710) ||(HW_BOARD_OPTION == MR8100_RX_RDI_M512)||\
    (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
enum
{
    UI_SET_ENVIR_PM25_GOOD = 1,
    UI_SET_ENVIR_PM25_NORMAL,
    UI_SET_ENVIR_PM25_BAD
};

enum
{
    UI_SET_MOTOR_ARROW_UP = 0,
    UI_SET_MOTOR_ARROW_CONTINUE_UP,
    UI_SET_MOTOR_ARROW_DOWN,
    UI_SET_MOTOR_ARROW_CONTINUE_DOWN,
    UI_SET_MOTOR_ARROW_RIGHT,
    UI_SET_MOTOR_ARROW_CONTINUE_RIGHT,
    UI_SET_MOTOR_ARROW_LEFT,
    UI_SET_MOTOR_ARROW_CONTINUE_LEFT,
    UI_SET_MOTOR_ARROW_STOP
};

enum
{
    UI_SET_MELODY_VOL_0 = 0,
    UI_SET_MELODY_VOL_1,
    UI_SET_MELODY_VOL_2,
    UI_SET_MELODY_VOL_3,
    UI_SET_MELODY_VOL_4,
    UI_SET_MELODY_VOL_5,
    UI_SET_MELODY_VOL_6,
    UI_SET_MELODY_VOL_7
};

enum
{
    UI_MEMORY_STATE_NORMAL = 0,
    UI_MEMORY_LEFT_PER0,
    UI_MEMORY_LEFT_PER05,
    UI_MEMORY_LEFT_PER10,
    UI_MEMORY_LEFT_PER20,
    UI_MEMORY_UNSUPPORT,
    UI_MEMORY_NO_INSERT,
};

enum
{
    UI_BOOT_IN_NORMAL = 0,
    UI_BOOT_IN_AP_MODE,
};
#endif
#if (HW_BOARD_OPTION == MR8100_RX_RDI_M512)
#else
typedef enum 
{
#if ((HW_BOARD_OPTION == MR8100_RX_RDI_SEM) || (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    OSD_ICON_WARNING = 0,          /*WARNING*/
    OSD_ICON_WARNING_2,
    OSD_ICON_WARNING_3,
    OSD_ICON_LOAD,
    OSD_ICON_ATENNA_1,
    OSD_ICON_ATENNA_2,
    OSD_ICON_ATENNA_Sign0_1,
    OSD_ICON_ATENNA_Sign1_1,
    OSD_ICON_ATENNA_Sign2_1,
    OSD_ICON_ATENNA_Sign3_1,
    OSD_ICON_ATENNA_Sign4_1,
    OSD_ICON_ATENNA_Sign5_1,
    OSD_ICON_BATTERY_LV0_1,
    OSD_ICON_BATTERY_LV0_2,
    OSD_ICON_BATTERY_LV1_1,
    OSD_ICON_BATTERY_LV1_2,
    OSD_ICON_BATTERY_LV2_1,
    OSD_ICON_BATTERY_LV2_2,
    OSD_ICON_BATTERY_LV3_1,
    OSD_ICON_BATTERY_LV3_2,
    OSD_ICON_BATTERY_LV4_1,
    OSD_ICON_BATTERY_LV4_2,
    OSD_ICON_BATTERY_CHARGE_1,
    OSD_ICON_BATTERY_CHARGE_2,
    OSD_ICON_CAM1_1,
    OSD_ICON_CAM2_1,
    OSD_ICON_CAM3_1,
    OSD_ICON_CAM4_1,
    OSD_ICON_CAM_2,
    OSD_ICON_CAM1_ON_1,//33
    OSD_ICON_CAM2_ON_1, 
    OSD_ICON_CAM3_ON_1,
    OSD_ICON_CAM4_ON_1,
    OSD_ICON_CAM_ON_2,
    OSD_ICON_SEQ1_1,
    OSD_ICON_SEQ1_2,
    OSD_ICON_SEQ1_3,
    OSD_ICON_SEQ2_1,
    OSD_ICON_SEQ2_2,
    OSD_ICON_SEQ2_3,
    OSD_ICON_RETURN_1,
    OSD_ICON_RETURN_2,
    OSD_ICON_HOME_1M,
    OSD_ICON_HOME_2M,
    OSD_ICON_TALKBACK1_1,
    OSD_ICON_TALKBACK1_2,
    OSD_ICON_TALKBACK1_3,
    OSD_ICON_TALKBACK2_1,
    OSD_ICON_TALKBACK2_2,
    OSD_ICON_TALKBACK2_3,
    OSD_ICON_PTZ_UP1_1,
    OSD_ICON_PTZ_UP1_2,
    OSD_ICON_PTZ_UP1_3,
    OSD_ICON_PTZ_DOWN1_1,
    OSD_ICON_PTZ_DOWN1_2,
    OSD_ICON_PTZ_DOWN1_3,
    OSD_ICON_PTZ_R1_1,
    OSD_ICON_PTZ_R1_2,
    OSD_ICON_PTZ_R1_3,  
    OSD_ICON_PTZ_L1_1,
    OSD_ICON_PTZ_L1_2,
    OSD_ICON_PTZ_L1_3,    
    OSD_ICON_PTZ_UP2_1,
    OSD_ICON_PTZ_UP2_2,
    OSD_ICON_PTZ_UP2_3,
    OSD_ICON_PTZ_DOWN2_1,
    OSD_ICON_PTZ_DOWN2_2,
    OSD_ICON_PTZ_DOWN2_3,
    OSD_ICON_PTZ_R2_1,
    OSD_ICON_PTZ_R2_2,
    OSD_ICON_PTZ_R2_3,    
    OSD_ICON_PTZ_L2_1,
    OSD_ICON_PTZ_L2_2,
    OSD_ICON_PTZ_L2_3,    
    OSD_ICON_MENU1_1,//78
    OSD_ICON_MENU1_2,
    OSD_ICON_MENU1_3,
    OSD_ICON_MENU2_1,
    OSD_ICON_MENU2_2,
    OSD_ICON_MENU2_3,
    OSD_ICON_ZOOM1_1,
    OSD_ICON_ZOOM2_1,
    OSD_ICON_ZOOM15_1,
    OSD_ICON_ZOOM1_IN_1,
    OSD_ICON_ZOOM1_IN_2,
    OSD_ICON_ZOOM1_IN_3,
    OSD_ICON_ZOOM1_OUT_1,
    OSD_ICON_ZOOM1_OUT_2,
    OSD_ICON_ZOOM1_OUT_3,
    OSD_ICON_ZOOM2_IN_1,
    OSD_ICON_ZOOM2_IN_2,
    OSD_ICON_ZOOM2_IN_3,
    OSD_ICON_ZOOM2_OUT_1,
    OSD_ICON_ZOOM2_OUT_2,
    OSD_ICON_ZOOM2_OUT_3,
    OSD_ICON_VOL_UP1_1,
    OSD_ICON_VOL_UP1_2,
    OSD_ICON_VOL_UP1_3,
    OSD_ICON_VOL_UP2_1,
    OSD_ICON_VOL_UP2_2,
    OSD_ICON_VOL_UP2_3,
    OSD_ICON_VOL_DOWN1_1,
    OSD_ICON_VOL_DOWN1_2,
    OSD_ICON_VOL_DOWN1_3,
    OSD_ICON_VOL_DOWN2_1,
    OSD_ICON_VOL_DOWN2_2,
    OSD_ICON_VOL_DOWN2_3,
    OSD_ICON_AM,
    OSD_ICON_PM,
    OSD_ICON_SOUND_NULL,
    OSD_ICON_SOUND_FULL,
    OSD_ICON_SOUND_MUTE, 
    OSD_ICON_MENU_NIGHT,
    OSD_ICON_VOX_1,
    OSD_ICON_VOX_2,
    OSD_ICON_REMOTE_1, 
    OSD_ICON_REMOTE_2, 
    OSD_ICON_SD_IN_1,
    OSD_ICON_SD_OUT_1, 
    OSD_ICON_SD_FULL_1,
    OSD_ICON_BLUETOOTH,
    OSD_ICON_TIME_COLON,
    OSD_ICON_NUM1,
    OSD_ICON_NUM2,
    OSD_ICON_NUM3,
    OSD_ICON_NUM4,
    OSD_ICON_NUM5,
    OSD_ICON_NUM6,
    OSD_ICON_NUM7,
    OSD_ICON_NUM8,
    OSD_ICON_NUM9,
    OSD_ICON_NUM10,
    OSD_ICON_QUAD1_2CH_1,
    OSD_ICON_QUAD1_2CH_2,
    OSD_ICON_QUAD1_2CH_3,
    OSD_ICON_QUAD2_2CH_1,
    OSD_ICON_QUAD2_2CH_2,
    OSD_ICON_QUAD2_2CH_3,
    OSD_ICON_QUAD1_4CH_1,
    OSD_ICON_QUAD1_4CH_2,
    OSD_ICON_QUAD1_4CH_3,
    OSD_ICON_QUAD2_4CH_1,
    OSD_ICON_QUAD2_4CH_2,
    OSD_ICON_QUAD2_4CH_3,
    OSD_ICON_DELETE1_1,
    OSD_ICON_DELETE1_2,
    OSD_ICON_DELETE1_3,
    OSD_ICON_DELETE2_1,
    OSD_ICON_DELETE2_2,
    OSD_ICON_DELETE2_3,
    OSD_ICON_SONG_1_1,
    OSD_ICON_SONG_1_2,
    OSD_ICON_SONG_2_1,
    OSD_ICON_SONG_2_2,
    OSD_ICON_SONG_3_1,
    OSD_ICON_SONG_3_2,
    OSD_ICON_SONG_4_1,
    OSD_ICON_SONG_4_2,
    OSD_ICON_SONG_ALL_1,
    OSD_ICON_SONG_ALL_2,    
    OSD_ICON_MUSIC_ON1_1,
    OSD_ICON_MUSIC_ON1_2,
    OSD_ICON_MUSIC_ON1_3,
    OSD_ICON_MUSIC_ON2_1,
    OSD_ICON_MUSIC_ON2_2,
    OSD_ICON_MUSIC_ON2_3,
    OSD_ICON_MUSIC_OFF1_1,
    OSD_ICON_MUSIC_OFF1_2,
    OSD_ICON_MUSIC_OFF1_3,
    OSD_ICON_MUSIC_OFF2_1,
    OSD_ICON_MUSIC_OFF2_2,
    OSD_ICON_MUSIC_OFF2_3,
    OSD_ICON_PAIR_1, 
    OSD_ICON_PAIR_2,   
    OSD_ICON_PAIR_3,     
    OSD_ICON_PAIR_4,       
    OSD_ICON_PAIR_5,         
    OSD_ICON_PAIR_6,        
    OSD_ICON_PAIR_7,      
    OSD_ICON_UNPAIR_1,      
    OSD_ICON_UNPAIR_2,     
    OSD_ICON_UNPAIR_3,     
    OSD_ICON_UNPAIR_4,      
    OSD_ICON_UNPAIR_5,       
    OSD_ICON_UNPAIR_6,        
    OSD_ICON_UNPAIR_7,         
    OSD_ICON_FEED_ICON_1,       
    OSD_ICON_FEED_ICON_2,      
    OSD_ICON_PM10_ICON_1,     
    OSD_ICON_PM10_ICON_2,      
    OSD_ICON_HUMIDITY_ICON_1,  
    OSD_ICON_HUMIDITY_ICON_2,   
    OSD_ICON_TEMPERATURE_1,    
    OSD_ICON_TEMPERATURE_2,    
    OSD_ICON_BAD_1,         
    OSD_ICON_BAD_2,           
    OSD_ICON_GOOD_1,          
    OSD_ICON_GOOD_2,           
    OSD_ICON_MID_1,             
    OSD_ICON_MID_2,          
    OSD_ICON_CENTIGRADE,       
    OSD_ICON_FAHRENHEIT,      
    OSD_ICON_PERCENT, 
    OSD_ICON_NO_SIGNAL_1,
    OSD_ICON_NO_SIGNAL_2,
    OSD_ICON_NO_SIGNAL_3,
    OSD_ICON_NO_SIGNAL_4,
    OSD_ICON_NO_SIGNAL_5,
    OSD_ICON_NO_SIGNAL_6,
    OSD_ICON_NO_SIGNAL_7, 
    OSD_ICON_WARMING,
    OSD_ICON_VOL_1,            
    OSD_ICON_VOL_2,             
    OSD_ICON_VOL_3,            
    OSD_ICON_VOL_4,           
    OSD_ICON_VOL_5,  
    OSD_ICON_BRIGHTNESS_1,
    OSD_ICON_BRIGHTNESS_2,
    OSD_ICON_BRIGHTNESS_3,    
    OSD_ICON_BRIGHTNESS_4,    
    OSD_ICON_BRIGHTNESS_5,    
    OSD_ICON_BRIGHTNESS_6,   
    OSD_ICON_BRIGHTNESS_7,    
    OSD_ICON_BRI_BG_1,  
    OSD_ICON_BRI_BG_2,      
    OSD_ICON_BRI_BG_3,       
    OSD_ICON_BRI_BG_4,       
    OSD_ICON_BRI_ADD_1,  
    OSD_ICON_BRI_ADD_2,      
    OSD_ICON_BRI_SUB_1,      
    OSD_ICON_BRI_SUB_2,      
    OSD_ICON_BRI_LEVEL_1,     
    OSD_ICON_BRI_LEVEL_2,     
    OSD_ICON_BRI_LEVEL_3,     
    OSD_ICON_BRI_LEVEL_4,     
    OSD_ICON_BRI_LEVEL_5, 
    OSD_ICON_COUNTDOWN,
    OSD_ICON_FEED_COLON,      
    OSD_ICON_FEED_00,          
    OSD_ICON_FEED_01,         
    OSD_ICON_FEED_02,         
    OSD_ICON_FEED_03,      
    OSD_ICON_FEED_04,        
    OSD_ICON_FEED_05,         
    OSD_ICON_FEED_06,         
    OSD_ICON_FEED_07,          
    OSD_ICON_FEED_08,          
    OSD_ICON_FEED_09,    
    OSD_ICON_UNPAIR_YES1,
    OSD_ICON_UNPAIR_YES2,
    OSD_ICON_UNPAIR_NO1,
    OSD_ICON_UNPAIR_NO2,
    OSD_ICON_PAIR_LINE1_1,
    OSD_ICON_PAIR_LINE1_2,
    OSD_ICON_PAIR_LINE1_3,
    OSD_ICON_PAIR_LINE1_4,
    OSD_ICON_PAIR_LINE1_5,
    OSD_ICON_PAIR_LINE1_6,
    OSD_ICON_PAIR_LINE1_7,
    OSD_ICON_PAIR_LINE1_8,
    OSD_ICON_PAIR_LINE1_9,
    OSD_ICON_PAIR_LINE1_10,
    OSD_ICON_PAIR_LINE2_1,
    OSD_ICON_PAIR_LINE2_2,
    OSD_ICON_PAIR_LINE2_3,
    OSD_ICON_PAIR_LINE2_4,
    OSD_ICON_PAIR_LINE2_5,
    OSD_ICON_PAIR_LINE2_6,
    OSD_ICON_PAIR_LINE2_7,
    OSD_ICON_ALARM_FEED1_1,
    OSD_ICON_ALARM_FEED1_2,                 
    OSD_ICON_ALARM_FEED1_3,                
    OSD_ICON_ALARM_FEED1_4,                 
    OSD_ICON_ALARM_FEED1_5,                 
    OSD_ICON_ALARM_HUMIDITY1_1,           
    OSD_ICON_ALARM_HUMIDITY1_2,            
    OSD_ICON_ALARM_HUMIDITY1_3,            
    OSD_ICON_ALARM_HUMIDITY1_4,             
    OSD_ICON_ALARM_HUMIDITY1_5,            
    OSD_ICON_ALARM_PM101_1,               
    OSD_ICON_ALARM_PM101_2,                
    OSD_ICON_ALARM_PM101_3,                
    OSD_ICON_ALARM_PM101_4,              
    OSD_ICON_ALARM_PM101_5,
    OSD_ICON_ALARM_TEMPERATURE1_1,
    OSD_ICON_ALARM_TEMPERATURE1_2, 
    OSD_ICON_ALARM_TEMPERATURE1_3,    
    OSD_ICON_ALARM_TEMPERATURE1_4,   
    OSD_ICON_ALARM_TEMPERATURE1_5,       
    OSD_ICON_ALARM_FEED2_1,
    OSD_ICON_ALARM_FEED2_2,                 
    OSD_ICON_ALARM_FEED2_3,                
    OSD_ICON_ALARM_FEED2_4,                 
    OSD_ICON_ALARM_FEED2_5,                 
    OSD_ICON_ALARM_HUMIDITY2_1,           
    OSD_ICON_ALARM_HUMIDITY2_2,            
    OSD_ICON_ALARM_HUMIDITY2_3,            
    OSD_ICON_ALARM_HUMIDITY2_4,             
    OSD_ICON_ALARM_HUMIDITY2_5,            
    OSD_ICON_ALARM_PM102_1,               
    OSD_ICON_ALARM_PM102_2,                
    OSD_ICON_ALARM_PM102_3,                
    OSD_ICON_ALARM_PM102_4,              
    OSD_ICON_ALARM_PM102_5,
    OSD_ICON_ALARM_TEMPERATURE2_1,
    OSD_ICON_ALARM_TEMPERATURE2_2, 
    OSD_ICON_ALARM_TEMPERATURE2_3,    
    OSD_ICON_ALARM_TEMPERATURE2_4,   
    OSD_ICON_ALARM_TEMPERATURE2_5,       
    OSD_ICON_UNSD_LINE1_1,     
    OSD_ICON_UNSD_LINE1_2,      
    OSD_ICON_UNSD_LINE1_3,        
    OSD_ICON_UNSD_LINE1_4,       
    OSD_ICON_UNSD_LINE1_5,         
    OSD_ICON_UNSD_LINE1_6,      
    OSD_ICON_UNSD_LINE1_7,    
    OSD_ICON_UNSD_LINE1_8,     
    OSD_ICON_UNSD_LINE1_9,     
    OSD_ICON_UNSD_LINE1_10,       
    OSD_ICON_UNSD_LINE2_1,      
    OSD_ICON_UNSD_LINE2_2,       
    OSD_ICON_UNSD_LINE2_3,        
    OSD_ICON_UNSD_LINE2_4,    
    OSD_ICON_UNSD_LINE2_5,    
    OSD_ICON_REMOTE_LINE1_1,             
    OSD_ICON_REMOTE_LINE1_2,              
    OSD_ICON_REMOTE_LINE1_3,             
    OSD_ICON_REMOTE_LINE1_4,              
    OSD_ICON_REMOTE_LINE1_5,            
    OSD_ICON_REMOTE_LINE1_6,              
    OSD_ICON_REMOTE_LINE1_7,            
    OSD_ICON_REMOTE_LINE1_8,           
    OSD_ICON_REMOTE_LINE1_9,            
    OSD_ICON_REMOTE_LINE1_10,           
    OSD_ICON_REMOTE_LINE1_11,              
    OSD_ICON_REMOTE_LINE1_12,               
    OSD_ICON_REMOTE_LINE1_13,               
    OSD_ICON_REMOTE_LINE1_14,   
    OSD_ICON_REMOTE_LINE2_1,              
    OSD_ICON_REMOTE_LINE2_2,              
    OSD_ICON_REMOTE_LINE2_3,             
    OSD_ICON_REMOTE_LINE2_4,              
    OSD_ICON_REMOTE_LINE2_5,            
    OSD_ICON_REMOTE_LINE2_6,              
    OSD_ICON_REMOTE_LINE2_7,            
    OSD_ICON_REMOTE_LINE2_8,           
    OSD_ICON_REMOTE_LINE2_9,            
    OSD_ICON_REMOTE_LINE2_10,           
    OSD_ICON_REMOTE_LINE3_1,              
    OSD_ICON_REMOTE_LINE3_2,              
    OSD_ICON_REMOTE_LINE3_3,             
    OSD_ICON_REMOTE_LINE3_4,              
    OSD_ICON_REMOTE_LINE3_5,            
    OSD_ICON_REMOTE_LINE3_6,              
    OSD_ICON_REMOTE_LINE3_7,            
    OSD_ICON_ALARM_LINE1_1,                
    OSD_ICON_ALARM_LINE1_2,               
    OSD_ICON_ALARM_LINE1_3,               
    OSD_ICON_ALARM_LINE1_4,                
    OSD_ICON_ALARM_LINE1_5,                
    OSD_ICON_ALARM_LINE1_6,                 
    OSD_ICON_ALARM_LINE1_7,                 
    OSD_ICON_ALARM_LINE1_8,                
    OSD_ICON_ALARM_LINE1_9,                 
    OSD_ICON_ALARM_LINE1_10,                
    OSD_ICON_ALARM_LINE1_11,                
    OSD_ICON_ALARM_LINE1_12,                 
    OSD_ICON_ALARM_LINE2_1,                
    OSD_ICON_ALARM_LINE2_2,               
    OSD_ICON_BIRTH_1,                   
    OSD_ICON_BIRTH_2,                     
    OSD_ICON_BIRTH_3,                   
    OSD_ICON_BIRTH_4,                   
    OSD_ICON_BIRTH_STR_1,                    
    OSD_ICON_BIRTH_STR_2,                     
    OSD_ICON_BIRTH_STR_3,                   
    OSD_ICON_BIRTH_STR_4,                   
    OSD_ICON_BIRTH_STR_5,                   
    OSD_ICON_BIRTH_STR_6,                   
    OSD_ICON_BIRTH_STR_7,                   
    OSD_ICON_BIRTH_STR_8,                    
    OSD_ICON_BIRTH_STR_9,               
    OSD_ICON_BIRTH_STR_10,                  
    OSD_ICON_BIRTH_STR_11,                   
    OSD_ICON_BIRTH_STR_12,                  
    OSD_ICON_BLUETOOTH_LINE1_1,            
    OSD_ICON_BLUETOOTH_LINE1_2,           
    OSD_ICON_BLUETOOTH_LINE1_3,                
    OSD_ICON_BLUETOOTH_LINE1_4,              
    OSD_ICON_BLUETOOTH_LINE1_5,               
    OSD_ICON_BLUETOOTH_LINE1_6,               
    OSD_ICON_BLUETOOTH_LINE1_7,               
    OSD_ICON_BLUETOOTH_LINE2_1,            
    OSD_ICON_BLUETOOTH_LINE2_2,           
    OSD_ICON_BLUETOOTH_LINE2_3,                
    OSD_ICON_BLUETOOTH_LINE2_4,              
    OSD_ICON_BLUETOOTH_LINE2_5,               
    OSD_ICON_BLUETOOTH_LINE2_6,               
    OSD_ICON_BLUETOOTH_LINE2_7,               
    OSD_ICON_DELETE_LINE1_1,              
    OSD_ICON_DELETE_LINE1_2,              
    OSD_ICON_DELETE_LINE1_3,             
    OSD_ICON_DELETE_LINE1_4,             
    OSD_ICON_DELETE_LINE1_5,               
    OSD_ICON_DELETE_LINE1_6,              
    OSD_ICON_DELETE_LINE1_7,             
    OSD_ICON_DELETE_LINE2_1,              
    OSD_ICON_DELETE_LINE2_2,              
    OSD_ICON_DELETE_LINE2_3,             
    OSD_ICON_DELETE_LINE2_4,             
    OSD_ICON_DELETE_LINE2_5,               
    OSD_ICON_DELETE_LINE2_6,              
    OSD_ICON_DELETE_LINE2_7,             
    OSD_ICON_DELETE_LINE3_1,              
    OSD_ICON_DELETE_LINE3_2,              
    OSD_ICON_DELETE_LINE3_3,             
    OSD_ICON_DELETE_LINE3_4,             
    OSD_ICON_DELETE_LINE3_5,               
    OSD_ICON_DELETE_LINE3_6,              
    OSD_ICON_DELETE_LINE3_7,             
    OSD_ICON_DELETE_LINE3_8,               
    OSD_ICON_DELETE_LINE3_9,             
    OSD_ICON_DELETE_LINE3_10, 
    OSD_ICON_DELETE_LINE3_Single1,
    OSD_ICON_DELETE_LINE3_Single2,
    OSD_ICON_DELETE_YES_1,                        
    OSD_ICON_DELETE_YES_2,                        
    OSD_ICON_DELETE_NO_1,                        
    OSD_ICON_DELETE_NO_2,                        
    OSD_ICON_MEMORY_FULL_1,                
    OSD_ICON_MEMORY_FULL_2,               
    OSD_ICON_MEMORY_FULL_3,               
    OSD_ICON_MEMORY_FULL_4,              
    OSD_ICON_MEMORY_FULL_5,               
    OSD_ICON_MEMORY_FULL_6,               
    OSD_ICON_MEMORY_FULL_7,               
    OSD_ICON_MEMORY_FULL_8,             
    OSD_ICON_MEMORY_FULL_9,               
    OSD_ICON_MEMORY_FULL_10,            
    OSD_ICON_MEMORY_FULL_11,            
    OSD_ICON_MEMORY_FULL_12,             
    OSD_ICON_MEMORY_FULL_13,            
    OSD_ICON_MEMORY_FULL_14,             
    OSD_ICON_MEMORY_LEFT_1,            
    OSD_ICON_MEMORY_LEFT_2,            
    OSD_ICON_MEMORY_LEFT_3,               
    OSD_ICON_MEMORY_LEFT_4,                   
    OSD_ICON_MEMORY_LEFT_5,               
    OSD_ICON_MEMORY_LEFT_6,              
    OSD_ICON_MEMORY_LEFT_7,             
    OSD_ICON_MEMORY_LEFT_8,              
    OSD_ICON_MEMORY_LEFT_9,              
    OSD_ICON_MEMORY_LEFT_10,             
    OSD_ICON_MEMORY_LEFT_5_1,            
    OSD_ICON_MEMORY_LEFT_5_2,          
    OSD_ICON_MEMORY_LEFT_10_1,         
    OSD_ICON_MEMORY_LEFT_10_2,           
    OSD_ICON_MEMORY_LEFT_10_3,           
    OSD_ICON_MEMORY_LEFT_20_1,           
    OSD_ICON_MEMORY_LEFT_20_2,            
    OSD_ICON_MEMORY_LEFT_20_3,            
    OSD_ICON_NO_SD_STR_1,                 
    OSD_ICON_NO_SD_STR_2,            
    OSD_ICON_NO_SD_STR_3,               
    OSD_ICON_NO_SD_STR_4,                 
    OSD_ICON_NO_SD_STR_5,               
    OSD_ICON_NO_SD_STR_6,                 
    OSD_ICON_NO_SD_STR_7,                 
    OSD_ICON_NO_SD_STR_8,             
    OSD_ICON_ANGLE_LEFT_TOP, 
    OSD_ICON_ANGLE_LEFT_BOTTOM,    
    OSD_ICON_ANGLE_RIGHT_TOP,     
    OSD_ICON_ANGLE_RIGHT_BOTTOM,   
    OSD_ICON_UNPAIR_LEFT_TOP, 
    OSD_ICON_UNPAIR_LEFT_BOTTOM,    
    OSD_ICON_UNPAIR_RIGHT_TOP,     
    OSD_ICON_UNPAIR_RIGHT_BOTTOM, 
    OSD_ICON_QUAD_NO_SIGNAL_1,
    OSD_ICON_QUAD_NO_SIGNAL_2,
    OSD_ICON_QUAD_NO_SIGNAL_3,
    OSD_ICON_BLT_FAIL_LINE1_1,
    OSD_ICON_BLT_FAIL_LINE1_2,
    OSD_ICON_BLT_FAIL_LINE1_3,
    OSD_ICON_BLT_FAIL_LINE1_4,
    OSD_ICON_BLT_FAIL_LINE2_1,
    OSD_ICON_BLT_FAIL_LINE2_2,
    OSD_ICON_BLT_FAIL_LINE2_3,
    OSD_ICON_BLT_FAIL_LINE2_4,
    OSD_ICON_BLT_FAIL_LINE2_5,
    OSD_ICON_BLT_FAIL_LINE2_6,
    OSD_ICON_BLT_FAIL_LINE2_7,
    OSD_ICON_BLT_FAIL_LINE2_8,
    OSD_ICON_BLT_FAIL_LINE2_9,
    OSD_ICON_BLT_FAIL_LINE3_1,
    OSD_ICON_BLT_FAIL_LINE3_2,
    OSD_ICON_BLT_FAIL_LINE3_3,
    OSD_ICON_BLT_FAIL_LINE3_4,
    OSD_ICON_BLT_FAIL_LINE3_5,
    OSD_ICON_BLT_FAIL_LINE3_6,
    OSD_ICON_BLT_FAIL_LINE3_7,
    OSD_ICON_BLT_FAIL_LINE3_8,
    OSD_ICON_BLT_FAIL_LINE3_9,
    OSD_ICON_BLT_FAIL_LINE3_10,
    OSD_ICON_TAKE_PHOTO_LINE1_1,
    OSD_ICON_TAKE_PHOTO_LINE1_2,
    OSD_ICON_TAKE_PHOTO_LINE1_3,
    OSD_ICON_TAKE_PHOTO_LINE1_4,
    OSD_ICON_TAKE_PHOTO_LINE1_5,
    OSD_ICON_TAKE_PHOTO_LINE1_6,
    OSD_ICON_TAKE_PHOTO_LINE1_7,
    OSD_ICON_TAKE_PHOTO_LINE1_8,
    OSD_ICON_TAKE_PHOTO_LINE1_9,
    OSD_ICON_TAKE_PHOTO_LINE1_10,
    OSD_ICON_TAKE_PHOTO_LINE1_11,
    OSD_ICON_TAKE_PHOTO_LINE1_12,
    OSD_ICON_TAKE_PHOTO_LINE1_13,
    OSD_ICON_TAKE_PHOTO_LINE1_14,
    OSD_ICON_TAKE_PHOTO_LINE2_1,
    OSD_ICON_TAKE_PHOTO_LINE2_2,
    OSD_ICON_TAKE_PHOTO_LINE2_3,
    OSD_ICON_TAKE_PHOTO_LINE2_4,
    OSD_ICON_TAKE_PHOTO_LINE2_5,
    OSD_ICON_TAKE_PHOTO_LINE2_6,
    OSD_ICON_TAKE_PHOTO_LINE2_7,
    OSD_ICON_BL_SEARCH_1,
    OSD_ICON_BL_SEARCH_2,
    OSD_ICON_BL_SEARCH_3,
    OSD_ICON_BL_SEARCH_4,
    OSD_ICON_BL_SEARCH_5,
    OSD_ICON_BL_SEARCH_6,
    OSD_ICON_BL_SEARCH_7,
    OSD_ICON_CAMERA_UNPAIR_1,
    OSD_ICON_CAMERA_UNPAIR_2,
    OSD_ICON_CAMERA_UNPAIR_3,
    OSD_ICON_CAMERA_UNPAIR_4,
    OSD_ICON_CAMERA_UNPAIR,
    OSD_ICON_TEP_CENTIGRADE,
    OSD_ICON_CAM_OFF_S_1,
    OSD_ICON_CAM_OFF_S_2,
    OSD_ICON_CAM_OFF_S_3,
    OSD_ICON_CAM_OFF_1,
    OSD_ICON_CAM_OFF_2,
    OSD_ICON_CAM_OFF_3,
    OSD_ICON_CAM_OFF_4,
    OSD_ICON_LIGHT_ON_1,
    OSD_ICON_LIGHT_ON_2,
    OSD_ICON_LIGHT_OFF_1,
    OSD_ICON_LIGHT_OFF_2,
    OSD_ICON_POWER_OFF_1,
    OSD_ICON_POWER_OFF_2,
    OSD_ICON_POWER_OFF_3,
    OSD_ICON_POWER_OFF_4,
    OSD_ICON_POWER_OFF_5,
    OSD_ICON_POWER_OFF_6,
    OSD_ICON_POWER_OFF_7,
#else
    OSD_ICON_WARNING = 0,          /*WARNING*/
    OSD_ICON_VREC,             /*VREC*/
    OSD_ICON_FF,               /*FF*/
    OSD_ICON_REW,              /*REW*/
    OSD_ICON_DV,  
    OSD_ICON_M,         /* 5 */
    OSD_ICON_PLAY,
    OSD_ICON_PLAYBACK,
    OSD_ICON_SD,
    OSD_ICON_STOP,
    OSD_ICON_ZOOMplus,   /*10*/
    OSD_ICON_ZOOMminus,
    OSD_ICON_ZOOM1,
    OSD_ICON_ZOOM2,
    OSD_ICON_ZOOM3,
    OSD_ICON_ZOOM4,       /*15*/
    OSD_ICON_ZOOM5,
    OSD_ICON_ZOOM6,
    OSD_ICON_ZOOM7,
    OSD_ICON_ZOOM8,
    OSD_ICON_ZOOM9,     /*20*/
    OSD_ICON_ZOOM10,
    OSD_ICON_ZOOM11,
    OSD_ICON_ZOOM12,
    OSD_ICON_ZOOM13,
    OSD_ICON_ZOOM14,    /*25 */
    OSD_ICON_ZOOM15,
    OSD_ICON_ZOOM16,
    OSD_ICON_ZOOM17,
    OSD_ICON_ZOOM18,
    OSD_ICON_ZOOM19,    /*30 */
    OSD_ICON_ZOOM20,
    OSD_ICON_CHANNEL_1M,
    OSD_ICON_CHANNEL_2M,   
    OSD_ICON_SPLIT_1M,
    OSD_ICON_SPLIT_2M,  /*35*/
    OSD_ICON_FULL_1M,
    OSD_ICON_FULL_2M,   
    OSD_ICON_ZOOM_1M,
    OSD_ICON_ZOOM_2M,   
    OSD_ICON_MENU_1M,   /*40*/
    OSD_ICON_MENU_2M,
    OSD_ICON_SOUND_1M,
    OSD_ICON_SOUND_2M,    
    OSD_ICON_PLAYBACK_1M,
    OSD_ICON_PLAYBACK_2M,     /*45*/
    OSD_ICON_IMG_SET_1M,
    OSD_ICON_IMG_SET_2M, 
    OSD_ICON_CH1_1M,
    OSD_ICON_CH1_2M,
    OSD_ICON_CH2_1M,       /*50*/
    OSD_ICON_CH2_2M,
    OSD_ICON_CH3_1M,
    OSD_ICON_CH3_2M,
    OSD_ICON_CH4_1M,
    OSD_ICON_CH4_2M,      /*55*/
    OSD_ICON_ZOOM_IN_1M,
    OSD_ICON_ZOOM_IN_2M,
    OSD_ICON_ZOOM_OUT_1M,
    OSD_ICON_ZOOM_OUT_2M,           
    OSD_ICON_ATENNA,             /*60*/
    OSD_ICON_ATENNA_Signal_0,
    OSD_ICON_ATENNA_Signal_1,
    OSD_ICON_ATENNA_Signal_2,
    OSD_ICON_ATENNA_Signal_3,    
    OSD_ICON_ATENNA_Signal_4,   /*65*/
    OSD_ICON_ATENNA_Signal_5,
    OSD_ICON_SD_FULL,
    OSD_ICON_SD_FULL_1,
    OSD_ICON_TALKBACK,
    OSD_ICON_UNREAD,
    OSD_ICON_MUTE,               
    OSD_ICON_POWER,
    OSD_ICON_CAM1,
    OSD_ICON_CAM2,
    OSD_ICON_CAM3,
    OSD_ICON_CAM4,               
    OSD_ICON_PAUSE,
    OSD_ICON_VOL1,
    OSD_ICON_VOL2,
    OSD_ICON_VOL3,
    OSD_ICON_VOL4,              
    OSD_ICON_VOL5,
    OSD_ICON_VOL6,
    OSD_ICON_SD_OFF,
    OSD_ICON_QUAD,
    OSD_ICON_HD_1,              
    OSD_ICON_VGA_1,
    OSD_ICON_QVGA_1,
    OSD_ICON_HD_2,
    OSD_ICON_VGA_2,
    OSD_ICON_QVGA_2,            
    OSD_ICON_CAMERA,
    OSD_ICON_REC,
    OSD_ICON_VOL,
    OSD_ICON_FRAME_1,
    OSD_ICON_FRAME_2,
    OSD_ICON_FRAME_3,           
    OSD_ICON_FRAME_4,
    OSD_ICON_SET_VOL,
    OSD_ICON_SET_VOL0,
    OSD_ICON_SET_VOL1,
    OSD_ICON_SET_VOL2,          
    OSD_ICON_SET_VOL3,
    OSD_ICON_SET_VOL4,
    OSD_ICON_SET_VOL5,
    OSD_ICON_VOL_FRAME_1,
    OSD_ICON_VOL_FRAME_2,       
    OSD_ICON_VOL_FRAME_3,
    OSD_ICON_VOL_FRAME_4,
    OSD_ICON_VOL_PLUS,
    OSD_ICON_VOL_MINUS,
    OSD_ICON_VOL_BAR_L,         
    OSD_ICON_VOL_BAR_R,
    OSD_ICON_OVERWRITE,
    OSD_ICON_A_REC,
    OSD_ICON_MOTION_DETECT,
    OSD_ICON_RETURN,            
    OSD_ICON_NO,
    OSD_ICON_SIGNAL_1,
    OSD_ICON_SIGNAL_2,
    OSD_ICON_PLAY_FRAME_R,
    OSD_ICON_PLAY_FRAME_L,      
    OSD_ICON_PLAY_L,
    OSD_ICON_PLAY_R,
    OSD_ICON_REW_L,
    OSD_ICON_REW_R,
    OSD_ICON_FF_L,              
    OSD_ICON_FF_R,
    OSD_ICON_STOP_L,
    OSD_ICON_STOP_R,
    OSD_ICON_PAUSE_L,
    OSD_ICON_PAUSE_R,           
    OSD_ICON_PLAYBACK_FOLDER1,
    OSD_ICON_PLAYBACK_FOLDER2,
    OSD_ICON_PLAYBACK_FILE1,
    OSD_ICON_PLAYBACK_FILE2,
    OSD_ICON_VREC2,             
    OSD_ICON_REC2,
    OSD_ICON_REMOTE,
    OSD_ICON_BATTERY_LV0,
    OSD_ICON_BATTERY_LV1,       
    OSD_ICON_BATTERY_LV2,       
    OSD_ICON_BATTERY_LV3,
    OSD_ICON_BATTERY_LV4,
    OSD_ICON_BATTERY_LV5, 
    OSD_ICON_NET_LINK_UP, 
    OSD_ICON_NET_LINK_DOWN,     
    OSD_ICON_BATTERY_CHARGE,
    OSD_ICON_BATTERY_UNCHARGE, /* Adapter without charging */
    OSD_ICON_BATTERY_L,
    OSD_ICON_BATTERY_R,
    OSD_ICON_CAM1_ON,           
    OSD_ICON_CAM2_ON,           
    OSD_ICON_CAM3_ON,           
    OSD_ICON_CAM4_ON,
    OSD_ICON_CAM1_OFF,        
    OSD_ICON_CAM2_OFF,
    OSD_ICON_CAM3_OFF,          
    OSD_ICON_CAM4_OFF,
    OSD_ICON_REC_MANUAL,
    OSD_ICON_REC_MOTION,
    OSD_ICON_REC_SCHEDULE,
    OSD_ICON_REC_NONE,          
    OSD_ICON_CAM_HD,            
    OSD_ICON_CAM_VGA,
    OSD_ICON_CAM_QVGA,
    OSD_ICON_DOWNLOAD,
    OSD_ICON_NET_BROKE,         
    OSD_ICON_HOME_1M,
    OSD_ICON_HOME_2M,
    OSD_ICON_BACK_1M,
    OSD_ICON_BACK_2M,
    OSD_ICON_MAIN_MENU_BAR_1,   
    OSD_ICON_MAIN_MENU_BAR_2,
    OSD_ICON_MAIN_MENU_BAR_3,
    OSD_ICON_MAIN_MENU_BAR_4,
    OSD_ICON_MAIN_MENU_BAR_5,
    OSD_ICON_MAIN_MENU_BAR_6,   
    OSD_ICON_AUTO_SEQUENCE_1,
    OSD_ICON_AUTO_SEQUENCE_2,
    OSD_ICON_VIEW,
    OSD_ICON_KEYBOARD_NUM_0,
    OSD_ICON_KEYBOARD_NUM_1,    
    OSD_ICON_KEYBOARD_NUM_2,
    OSD_ICON_KEYBOARD_NUM_3,
    OSD_ICON_KEYBOARD_NUM_4,
    OSD_ICON_KEYBOARD_NUM_5,
    OSD_ICON_KEYBOARD_NUM_6,    
    OSD_ICON_KEYBOARD_NUM_7,
    OSD_ICON_KEYBOARD_NUM_8,
    OSD_ICON_KEYBOARD_NUM_9,
    OSD_ICON_KEYBOARD_NUM_ENTER,
    OSD_ICON_KEYBOARD_NUM_CENCEL,
    OSD_ICON_NET_P2P_OK,
    OSD_ICON_MENU_PLAYBACK_1,
    OSD_ICON_MENU_PLAYBACK_2,
    OSD_ICON_MENU_PLAY_STR_1,
    OSD_ICON_MENU_PLAY_STR_1_L2,
    OSD_ICON_MENU_PLAY_STR_1_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_PLAY_STR_1_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_PLAY_STR_1_L4,
    OSD_ICON_MENU_PLAY_STR_1_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7)) 
    OSD_ICON_MENU_PLAY_STR_1_L4,
    OSD_ICON_MENU_PLAY_STR_1_L5,
    OSD_ICON_MENU_PLAY_STR_1_L6,
#endif    
    OSD_ICON_MENU_PLAY_STR_2,
    OSD_ICON_MENU_PLAY_STR_2_L2,
    OSD_ICON_MENU_PLAY_STR_2_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_PLAY_STR_2_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_PLAY_STR_2_L4,
    OSD_ICON_MENU_PLAY_STR_2_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))    
    OSD_ICON_MENU_PLAY_STR_2_L4,
    OSD_ICON_MENU_PLAY_STR_2_L5,
    OSD_ICON_MENU_PLAY_STR_2_L6,
#endif    
    OSD_ICON_MENU_PLAY_STR_3,
    OSD_ICON_MENU_PLAY_STR_3_L2,
    OSD_ICON_MENU_PLAY_STR_3_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_PLAY_STR_3_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_PLAY_STR_3_L4,
    OSD_ICON_MENU_PLAY_STR_3_L5,
#elif ((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_PLAY_STR_3_L4,
    OSD_ICON_MENU_PLAY_STR_3_L5,
    OSD_ICON_MENU_PLAY_STR_3_L6,
#endif    
    OSD_ICON_MENU_PLAY_STR_4,
    OSD_ICON_MENU_PLAY_STR_4_L2,
    OSD_ICON_MENU_PLAY_STR_4_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_PLAY_STR_4_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_PLAY_STR_4_L4,
    OSD_ICON_MENU_PLAY_STR_4_L5,
#elif ((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))   
    OSD_ICON_MENU_PLAY_STR_4_L4,
    OSD_ICON_MENU_PLAY_STR_4_L5,
    OSD_ICON_MENU_PLAY_STR_4_L6,
#endif    
    OSD_ICON_MENU_VOLUME_1,
    OSD_ICON_MENU_VOLUME_2,
    OSD_ICON_MENU_VOL_STR_1,
    OSD_ICON_MENU_VOL_STR_1_L2,
    OSD_ICON_MENU_VOL_STR_1_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_VOL_STR_1_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_VOL_STR_1_L4,
    OSD_ICON_MENU_VOL_STR_1_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))    
    OSD_ICON_MENU_VOL_STR_1_L4,
    OSD_ICON_MENU_VOL_STR_1_L5,
    OSD_ICON_MENU_VOL_STR_1_L6,
#endif    
    OSD_ICON_MENU_VOL_STR_2,
    OSD_ICON_MENU_VOL_STR_2_L2,
    OSD_ICON_MENU_VOL_STR_2_L3, 
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_VOL_STR_2_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_VOL_STR_2_L4,
    OSD_ICON_MENU_VOL_STR_2_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_VOL_STR_2_L4,
    OSD_ICON_MENU_VOL_STR_2_L5,
    OSD_ICON_MENU_VOL_STR_2_L6,
#endif    
    OSD_ICON_MENU_VOL_STR_3,
    OSD_ICON_MENU_VOL_STR_3_L2,
    OSD_ICON_MENU_VOL_STR_3_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_VOL_STR_3_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_VOL_STR_3_L4,
    OSD_ICON_MENU_VOL_STR_3_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_VOL_STR_3_L4,
    OSD_ICON_MENU_VOL_STR_3_L5,
    OSD_ICON_MENU_VOL_STR_3_L6,
#endif    
    OSD_ICON_MENU_BRIGHT_1,
    OSD_ICON_MENU_BRIGHT_2,
    OSD_ICON_MENU_BRI_STR_1,
    OSD_ICON_MENU_BRI_STR_1_L2,
    OSD_ICON_MENU_BRI_STR_1_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_BRI_STR_1_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_BRI_STR_1_L4,
    OSD_ICON_MENU_BRI_STR_1_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_BRI_STR_1_L4,
    OSD_ICON_MENU_BRI_STR_1_L5,
    OSD_ICON_MENU_BRI_STR_1_L6,
#endif    
    OSD_ICON_MENU_BRI_STR_2,
    OSD_ICON_MENU_BRI_STR_2_L2,
    OSD_ICON_MENU_BRI_STR_2_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_BRI_STR_2_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_BRI_STR_2_L4,
    OSD_ICON_MENU_BRI_STR_2_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_BRI_STR_2_L4,
    OSD_ICON_MENU_BRI_STR_2_L5,
    OSD_ICON_MENU_BRI_STR_2_L6,
#endif    
    OSD_ICON_MENU_BRI_STR_3,
    OSD_ICON_MENU_BRI_STR_3_L2,
    OSD_ICON_MENU_BRI_STR_3_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_BRI_STR_3_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_BRI_STR_3_L4,
    OSD_ICON_MENU_BRI_STR_3_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_BRI_STR_3_L4,
    OSD_ICON_MENU_BRI_STR_3_L5,
    OSD_ICON_MENU_BRI_STR_3_L6,
#endif    
    OSD_ICON_MENU_GENERAL_1,
    OSD_ICON_MENU_GENERAL_2,
    OSD_ICON_MENU_GEN_STR_1,
    OSD_ICON_MENU_GEN_STR_1_L2,
    OSD_ICON_MENU_GEN_STR_1_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_GEN_STR_1_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_GEN_STR_1_L4,
    OSD_ICON_MENU_GEN_STR_1_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_GEN_STR_1_L4,
    OSD_ICON_MENU_GEN_STR_1_L5,
    OSD_ICON_MENU_GEN_STR_1_L6,
#endif    
    OSD_ICON_MENU_GEN_STR_2,
    OSD_ICON_MENU_GEN_STR_2_L2,
    OSD_ICON_MENU_GEN_STR_2_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_GEN_STR_2_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_GEN_STR_2_L4,
    OSD_ICON_MENU_GEN_STR_2_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_GEN_STR_2_L4,
    OSD_ICON_MENU_GEN_STR_2_L5,
    OSD_ICON_MENU_GEN_STR_2_L6,
#endif    
    OSD_ICON_MENU_GEN_STR_3,
    OSD_ICON_MENU_GEN_STR_3_L2,
    OSD_ICON_MENU_GEN_STR_3_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))    
    OSD_ICON_MENU_GEN_STR_3_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_GEN_STR_3_L4,
    OSD_ICON_MENU_GEN_STR_3_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))    
    OSD_ICON_MENU_GEN_STR_3_L4,
    OSD_ICON_MENU_GEN_STR_3_L5,
    OSD_ICON_MENU_GEN_STR_3_L6,
#endif    
    OSD_ICON_MENU_GEN_STR_4,
    OSD_ICON_MENU_GEN_STR_4_L2,
    OSD_ICON_MENU_GEN_STR_4_L3,
#if (((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 9)))
    OSD_ICON_MENU_GEN_STR_4_L4,
#elif ((HW_BOARD_OPTION == MR8120_RX_RDI_M733)&&(PROJ_OPT == 4))
    OSD_ICON_MENU_GEN_STR_4_L4,
    OSD_ICON_MENU_GEN_STR_4_L5,
#elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_GEN_STR_4_L4,
    OSD_ICON_MENU_GEN_STR_4_L5,
    OSD_ICON_MENU_GEN_STR_4_L6,
#endif    
    OSD_ICON_SUB_ACT_L,
    OSD_ICON_SUB_ACT_R,
    OSD_ICON_SUB_ACT_PLUS,
    OSD_ICON_SUB_ACT_MINUS,
    OSD_ICON_LETTER_UA,
    OSD_ICON_LETTER_UM,
    OSD_ICON_LETTER_UP,
    OSD_ICON_PLAY_ACT_L,
    OSD_ICON_PLAY_ACT_R,
    OSD_ICON_PLAY_ACT_PLAY_1,
    OSD_ICON_PLAY_ACT_PLAY_2,
    OSD_ICON_PLAY_ACT_STOP_1,
    OSD_ICON_PLAY_ACT_STOP_2,
    OSD_ICON_PLAY_ACT_REW_1,
    OSD_ICON_PLAY_ACT_REW_2,
    OSD_ICON_PLAY_ACT_FF_1,
    OSD_ICON_PLAY_ACT_FF_2,
    OSD_ICON_PLAY_ACT_PREV_1,
    OSD_ICON_PLAY_ACT_PREV_2,
    OSD_ICON_PLAY_ACT_NEXT_1,
    OSD_ICON_PLAY_ACT_NEXT_2,
    OSD_ICON_PLAY_ACT_HOME_1,
    OSD_ICON_PLAY_ACT_HOME_2,
    OSD_ICON_PLAY_ACT_BACK_1,
    OSD_ICON_PLAY_ACT_BACK_2,
    OSD_ICON_PLAY_ACT_DEL_1,
    OSD_ICON_PLAY_ACT_DEL_2,
    OSD_ICON_PLAY_BAR_R,
    OSD_ICON_PLAY_BAR_L,
    OSD_ICON_PLAY_BAR_UP,
    OSD_ICON_PLAY_BAR_DOWN,
    OSD_ICON_TALKBACK_2,
    OSD_ICON_DELETE,
    OSD_ICON_LOAD,
    OSD_ICON_HOME_RF_1,
    OSD_ICON_HOME_RF_2,
    OSD_ICON_HOME_RF_3,
    OSD_ICON_CHECK,
    OSD_ICON_HA_ICON_1,
    OSD_ICON_HA_ICON_2,
#if ((HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA))
    OSD_ICON_HA_APP_ICON,
    OSD_ICON_HA_INDOOR_ICON,
    OSD_ICON_HA_OUTDOOR_ICON,
    OSD_ICON_HA_UNLOCK,
#endif
    OSD_ICON_CAMERA1,
#if (HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505)
    OSD_ICON_LUMUTE,
    OSD_ICON_LULLABY1,
    OSD_ICON_LULLABY2,
    OSD_ICON_LULLABY3,
    OSD_ICON_PTZ,
    OSD_ICON_PTZ_RIGHT,
    OSD_ICON_PTZ_RIGHT1,
    OSD_ICON_PTZ_LEFT,
    OSD_ICON_PTZ_LEFT1,
    OSD_ICON_PTZ_UP,
    OSD_ICON_PTZ_UP1,
    OSD_ICON_PTZ_DOWN,
    OSD_ICON_PTZ_DOWN1,
    OSD_ICON_PTZ_CENTER,
#elif ((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM))
    OSD_ICON_LUMUTE,
    OSD_ICON_LULLABY1,
    OSD_ICON_LULLABY2,
    OSD_ICON_LULLABY3,
    OSD_ICON_PTZ,
    OSD_ICON_PTZ_UP,
    OSD_ICON_PTZ_DOWN,
    OSD_ICON_PTZ_LEFT,
    OSD_ICON_PTZ_RIGHT,
    OSD_ICON_PTZ_UP1,
    OSD_ICON_PTZ_DOWN1,
    OSD_ICON_PTZ_LEFT1,
    OSD_ICON_PTZ_RIGHT1,
    OSD_ICON_AM,
    OSD_ICON_PM,
    OSD_ICON_VOX,
    OSD_ICON_CALIBRAT_H,
    OSD_ICON_CALIBRAT_V,
    OSD_ICON_PAIR1,
    OSD_ICON_PAIR2,
    OSD_ICON_UNPAIR1,
    OSD_ICON_UNPAIR2,
    OSD_ICON_FEED,
    OSD_ICON_NO_SIGNAL_1,
    OSD_ICON_NO_SIGNAL_2,
    OSD_ICON_WARMING,
    OSD_ICON_M_VOL_1,
    OSD_ICON_M_VOL_2,
    OSD_ICON_M_VOL_3,
    OSD_ICON_M_VOL_4,
    OSD_ICON_M_VOL_5,
    OSD_ICON_BRIGHTNESS_1,
    OSD_ICON_BRIGHTNESS_2,
    OSD_ICON_BRI_ACT_1,
    OSD_ICON_BRI_ACT_2,
    OSD_ICON_BRI_ACT_PRESS_1,
    OSD_ICON_BRI_ACT_PRESS_2,
    OSD_ICON_B_VOL_1,
    OSD_ICON_B_VOL_2,
    OSD_ICON_B_VOL_3,
    OSD_ICON_B_VOL_4,
    OSD_ICON_B_VOL_5,
    OSD_ICON_COUNTDOWN,
    OSD_ICON_CD_COLON,
    OSD_ICON_CD_00,
    OSD_ICON_CD_01,
    OSD_ICON_CD_02,
    OSD_ICON_CD_03,
    OSD_ICON_CD_04,
    OSD_ICON_CD_05,
    OSD_ICON_CD_06,
    OSD_ICON_CD_07,
    OSD_ICON_CD_08,
    OSD_ICON_CD_09,
    OSD_ICON_UNPAIR_YES1,
    OSD_ICON_UNPAIR_YES2,
    OSD_ICON_UNPAIR_NO1,
    OSD_ICON_UNPAIR_NO2,
    OSD_ICON_PAIR_STR1,
    OSD_ICON_PAIR_STR2,
    OSD_ICON_PAIR_STR3,
#endif

#if (HW_BOARD_OPTION == MR6730_AFN)
	OSD_ICON_MOTION,
#endif

#if (HW_BOARD_OPTION == MR8200_RX_RDI_M706)
    OSD_ICON_MENU_LIGHTING_1,
    OSD_ICON_MENU_LIGHTING_2,
    OSD_ICON_MENU_LIGHT_STR_1,
    OSD_ICON_MENU_LIGHT_STR_1_L2,
    OSD_ICON_MENU_LIGHT_STR_1_L3,   
    #if(((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 9)))
    OSD_ICON_MENU_LIGHT_STR_1_L4,
    #elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_LIGHT_STR_1_L4,
    OSD_ICON_MENU_LIGHT_STR_1_L5,
    OSD_ICON_MENU_LIGHT_STR_1_L6,
    #endif
    OSD_ICON_MENU_LIGHT_STR_2,
    OSD_ICON_MENU_LIGHT_STR_2_L2,
    OSD_ICON_MENU_LIGHT_STR_2_L3,
    #if(((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 9)))
    OSD_ICON_MENU_LIGHT_STR_2_L4,
    #elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_LIGHT_STR_2_L4,
    OSD_ICON_MENU_LIGHT_STR_2_L5,
    OSD_ICON_MENU_LIGHT_STR_2_L6,
    #endif
    OSD_ICON_MENU_LIGHT_STR_3,
    OSD_ICON_MENU_LIGHT_STR_3_L2,
    OSD_ICON_MENU_LIGHT_STR_3_L3,
    #if(((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 9)))
    OSD_ICON_MENU_LIGHT_STR_3_L4,
    #elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_LIGHT_STR_3_L4,
    OSD_ICON_MENU_LIGHT_STR_3_L5,
    OSD_ICON_MENU_LIGHT_STR_3_L6,
    #endif
    OSD_ICON_MENU_LIGHT_STR_4,
    OSD_ICON_MENU_LIGHT_STR_4_L2,
    OSD_ICON_MENU_LIGHT_STR_4_L3,
    #if(((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 5))||((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 9)))
    OSD_ICON_MENU_LIGHT_STR_4_L4,
    OSD_ICON_MENU_LIGHT_STR_5,
    OSD_ICON_MENU_LIGHT_STR_5_L2,
    OSD_ICON_MENU_LIGHT_STR_5_L3,
    OSD_ICON_MENU_LIGHT_STR_5_L4,
    #elif((HW_BOARD_OPTION == MR8200_RX_RDI_M706)&&(PROJ_OPT == 7))
    OSD_ICON_MENU_LIGHT_STR_4_L4,
    OSD_ICON_MENU_LIGHT_STR_4_L5,
    OSD_ICON_MENU_LIGHT_STR_4_L6,
    OSD_ICON_MENU_LIGHT_STR_5,
    OSD_ICON_MENU_LIGHT_STR_5_L2,
    OSD_ICON_MENU_LIGHT_STR_5_L3,
    OSD_ICON_MENU_LIGHT_STR_5_L4,
    OSD_ICON_MENU_LIGHT_STR_5_L5,
    OSD_ICON_MENU_LIGHT_STR_5_L6,

    #endif
    //OSD_ICON_LIGHTING_SETUP_LIGHT,
    //OSD_ICON_LIGHTING_SETUP_LIGHT_MANUAL,
#endif
#if (UI_SHOW_NO_SINGNAL_TYPE == UI_NOSINGANL_TYPE_SHOW_OSDMSG)
    OSD_ICON__MIN_NOSIGNAL_L0,
    OSD_ICON__MIN_NOSIGNAL_L1,
    OSD_ICON__MIN_NOSIGNAL_L2,
    OSD_ICON__MIN_NOSIGNAL_L3,
    OSD_ICON_NOSIGNAL_L0,
    OSD_ICON_NOSIGNAL_L1,
    OSD_ICON_NOSIGNAL_L2,
    OSD_ICON_NOSIGNAL_L3,
    OSD_ICON_NOSIGNAL_L4,
    OSD_ICON_NOSIGNAL_L5,
    OSD_ICON_NOSIGNAL_L6,
    OSD_ICON_NOSIGNAL_L7,
    OSD_ICON__MIN_NOSIGNAL_EN_L0,
    OSD_ICON__MIN_NOSIGNAL_EN_L1,
    OSD_ICON_NOSIGNAL_EN_L0,
    OSD_ICON_NOSIGNAL_EN_L1,
    OSD_ICON_NOSIGNAL_EN_L2,
    OSD_ICON__MIN_NOSIGNAL_ES_L0,
    OSD_ICON__MIN_NOSIGNAL_ES_L1,
    OSD_ICON_NOSIGNAL_ES_L0,
    OSD_ICON_NOSIGNAL_ES_L1,
    OSD_ICON_NOSIGNAL_ES_L2,
#elif (UI_SHOW_NO_SINGNAL_TYPE == UI_NOSINGANL_TYPE_SHOW_OSDICON)
    OSD_ICON_NOSIGNAL_1,
    OSD_ICON_NOSIGNAL_2,
    OSD_ICON_NOSIGNAL_3,
    OSD_ICON_NOSIGNAL_4,
    OSD_ICON_NOSIGNAL_5,
#endif

#if UI_LIGHT_SUPPORT
    OSD_ICON_LIGHTING_SETUP_LIGHT,
    OSD_ICON_LIGHTING_SETUP_LIGHT_MANUAL,
#endif  
    OSD_ICON_UPGRAD_FINISH_1, 
    OSD_ICON_UPGRAD_FINISH_2,   
    OSD_ICON_UPGRAD_FINISH_3, 
    OSD_ICON_UPGRAD_FINISH_4,     
    OSD_ICON_UPGRAD_FINISH_5,  
    OSD_ICON_UPGRAD_FINISH_6,   
    OSD_ICON_UPGRAD_FINISH_7,    
    OSD_ICON_UPGRAD_FINISH_8,    
    OSD_ICON_UPGRAD_FINISH_9,   
    OSD_ICON_UPGRAD_FINISH_10,   
    OSD_ICON_UPGRAD_FINISH_11,   
    OSD_ICON_UPGRAD_FINISH_12,  
    OSD_ICON_UPGRAD_FINISH_13,  
    OSD_ICON_UPGRAD_FINISH_14,     
    OSD_ICON_UPGRAD_FINISH_S_1,  
    OSD_ICON_UPGRAD_FINISH_S_2,   
    OSD_ICON_UPGRAD_FINISH_S_3,   
    OSD_ICON_UPGRAD_FINISH_S_4,    
    OSD_ICON_UPGRAD_FINISH_S_5,    
    OSD_ICON_UPGRAD_FINISH_S_6,    
    OSD_ICON_UPGRAD_FINISH_S_7,    
    OSD_ICON_UPGRAD_FINISH_S_8,    
    OSD_ICON_UPGRAD_FINISH_S_9,    
    OSD_ICON_UPGRAD_FINISH_S_10,    
    OSD_ICON_UPGRAD_FINISH_S_11,    
    OSD_ICON_UPGRAD_FINISH_S_12,   
    OSD_ICON_UPGRAD_FINISH_S_13,    
    OSD_ICON_UPGRAD_FINISH_S_14,    
    OSD_ICON_UPGRAD_FINISH_S_15,    
    OSD_ICON_UPGRAD_FINISH_S_16, 
#if(UI_OTHER_LANGUAGE == 1)    
    OSD_ICON_UPGRAD_FINISH_J_1,     
    OSD_ICON_UPGRAD_FINISH_J_2,     
    OSD_ICON_UPGRAD_FINISH_J_3,     
    OSD_ICON_UPGRAD_FINISH_J_4,     
    OSD_ICON_UPGRAD_FINISH_J_5,     
    OSD_ICON_UPGRAD_FINISH_J_6,     
    OSD_ICON_UPGRAD_FINISH_J_7,     
    OSD_ICON_UPGRAD_FINISH_J_8,     
    OSD_ICON_UPGRAD_FINISH_J_9,     
    OSD_ICON_UPGRAD_FINISH_J_10,    
    OSD_ICON_UPGRAD_FINISH_J_11,    
    OSD_ICON_UPGRAD_FINISH_J_12,    
    OSD_ICON_UPGRAD_FINISH_J_13,    
    OSD_ICON_UPGRAD_FINISH_J_14,     
#elif(UI_OTHER_LANGUAGE == 2)     
    OSD_ICON_UPGRAD_FINISH_FRE_1,
    OSD_ICON_UPGRAD_FINISH_FRE_2,
    OSD_ICON_UPGRAD_FINISH_FRE_3,
    OSD_ICON_UPGRAD_FINISH_FRE_4,
    OSD_ICON_UPGRAD_FINISH_FRE_5,
    OSD_ICON_UPGRAD_FINISH_FRE_6,
    OSD_ICON_UPGRAD_FINISH_FRE_7,
    OSD_ICON_UPGRAD_FINISH_FRE_8,
    OSD_ICON_UPGRAD_FINISH_FRE2_1,
    OSD_ICON_UPGRAD_FINISH_FRE2_2,
    OSD_ICON_UPGRAD_FINISH_FRE2_3,
    OSD_ICON_UPGRAD_FINISH_FRE2_4,
    OSD_ICON_UPGRAD_FINISH_FRE2_5,
    OSD_ICON_UPGRAD_FINISH_FRE2_6,
    OSD_ICON_UPGRAD_FINISH_FRE2_7,
    OSD_ICON_UPGRAD_FINISH_FRE2_8,
    OSD_ICON_UPGRAD_FINISH_FRE2_9,
    OSD_ICON_UPGRAD_FINISH_FRE2_10,
    OSD_ICON_UPGRAD_FINISH_FRE2_11,
    OSD_ICON_UPGRAD_FINISH_FRE2_12,
    OSD_ICON_UPGRAD_FINISH_GER_1,
    OSD_ICON_UPGRAD_FINISH_GER_2,
    OSD_ICON_UPGRAD_FINISH_GER_3,
    OSD_ICON_UPGRAD_FINISH_GER_4,
    OSD_ICON_UPGRAD_FINISH_GER_5,
    OSD_ICON_UPGRAD_FINISH_GER_6,
    OSD_ICON_UPGRAD_FINISH_GER_7,
    OSD_ICON_UPGRAD_FINISH_GER_8,
    OSD_ICON_UPGRAD_FINISH_GER_9,
    OSD_ICON_UPGRAD_FINISH_GER_10,
    OSD_ICON_UPGRAD_FINISH_GER_11,
    OSD_ICON_UPGRAD_FINISH_GER_12,
    OSD_ICON_UPGRAD_FINISH_GER2_1,
    OSD_ICON_UPGRAD_FINISH_GER2_2,
    OSD_ICON_UPGRAD_FINISH_GER2_3,
    OSD_ICON_UPGRAD_FINISH_GER2_4,
    OSD_ICON_UPGRAD_FINISH_GER2_5,
    OSD_ICON_UPGRAD_FINISH_GER2_6,
    OSD_ICON_UPGRAD_FINISH_GER2_7,
    OSD_ICON_UPGRAD_FINISH_GER2_8,
    OSD_ICON_UPGRAD_FINISH_GER2_9,
    OSD_ICON_UPGRAD_FINISH_ITA_1,
    OSD_ICON_UPGRAD_FINISH_ITA_2,
    OSD_ICON_UPGRAD_FINISH_ITA_3,
    OSD_ICON_UPGRAD_FINISH_ITA_4,
    OSD_ICON_UPGRAD_FINISH_ITA_5,
    OSD_ICON_UPGRAD_FINISH_ITA_6,
    OSD_ICON_UPGRAD_FINISH_ITA_7,
    OSD_ICON_UPGRAD_FINISH_ITA_8,
    OSD_ICON_UPGRAD_FINISH_ITA_9,
    OSD_ICON_UPGRAD_FINISH_ITA_10,
    OSD_ICON_UPGRAD_FINISH_ITA_11,
    OSD_ICON_UPGRAD_FINISH_ITA_12,
    OSD_ICON_UPGRAD_FINISH_ITA_13,
    OSD_ICON_UPGRAD_FINISH_ITA_14,
    OSD_ICON_UPGRAD_FINISH_ITA_15,
    OSD_ICON_UPGRAD_FINISH_ITA_16,
    OSD_ICON_UPGRAD_FINISH_ITA_17,
    OSD_ICON_UPGRAD_FINISH_ITA_18,
#endif
#endif 
#if (UI_SHOW_CAMERA_OFF == 1)
    OSD_ICON_CENT_CAMOFF_ICON_1,
    OSD_ICON_CENT_CAMOFF_ICON_2,
    OSD_ICON_CENT_CAMOFF_ICON_3,
    OSD_ICON_CENT_CAMOFF_ICON_4,
    OSD_ICON_CENT_CAMOFF_ICON_5,
#endif
    OSD_ICON_CENTIGRADE,       
    OSD_ICON_FAHRENHEIT,      
    OSD_ICON_TEMP_NUM_0,      
    OSD_ICON_TEMP_NUM_1,      
    OSD_ICON_TEMP_NUM_2,      
    OSD_ICON_TEMP_NUM_3,      
    OSD_ICON_TEMP_NUM_4,      
    OSD_ICON_TEMP_NUM_5,      
    OSD_ICON_TEMP_NUM_6,      
    OSD_ICON_TEMP_NUM_7,      
    OSD_ICON_TEMP_NUM_8,      
    OSD_ICON_TEMP_NUM_9,      

    OSD_ICON_MAX_NUM,       /*Keep in last*/
    OSD_ICON_BLOCK,
    
    
}OSD_ICONIDX;
#endif
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
    UI_MENU_VIDEO_SIZE_320x240,
    UI_MENU_VIDEO_SIZE_1280X720,
    UI_MENU_VIDEO_SIZE_1920x1088,
    UI_MENU_VIDEO_SIZE_352x240
};

enum 
{
    UI_OSD_DRAW = 0,
    UI_OSD_CLEAR,
    UI_OSD_NONE,
    UI_OSD_CHOSE,  /*For M922GD Mask Area*/
    UI_OSD_NORMAL,
    UI_OSD_PRESS,
};


enum 
{
    UI_DSP_PLAY_LIST_DIR = 0,
    UI_DSP_PLAY_LIST_FILE,
    UI_DSP_PLAY_LIST_DOOR_PIC,
    UI_DSP_PLAY_LIST_DOOR_ALB,
    UI_DSP_PLAY_LIST_PLAYBACK,
    UI_DSP_PLAY_LIST_DOOR_SELECT,
    
};

enum 
{
    UI_RESOLTUION_HD = 0,
    UI_RESOLTUION_VGA,
    UI_RESOLTUION_QVGA,
    UI_RESOLUTION_D1_480V,
    UI_RESOLUTION_D1_576V,
    UI_RESOLUTION_352x240,
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
    UI_SCH_MOTION_OFF = 0,
    UI_SCH_MOTION_ON,
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

#if RFIU_SUPPORT
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
    UI_BATTERY_UNCHARGE, /* Adapter without charging */
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
    UI_SD_IN = 0,
    UI_SD_OUT,
    UI_SD_FULL,
    UI_SD_FAIL,
};

enum
{
    UI_DEGREE_TYPE_CENTIGRADE,
    UI_DEGREE_TYPE_FAHRENHEIT,   
};

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


typedef struct
{
	u8	stFileName[16]; 		/* Indicate the wav file name */
	u32 stFileLen;			    /* Indicate the wav file length */
	u32 stLenInSPI; 			/* Indicate the file length in serial flash*/
	u32 stLenCountInSPI; 	    /* Indicate the count of the file length in serial flash*/
	u32 stPageStartAddr;		/* Indicate the real page address in serial flash */
	u32 stMAXFileLen;		    /* IIndicate the MAX wav file length*/
}UI_WAV_OBJECT;

/*the return value of state after change TV-in format */
typedef enum TVFormatStatus
{
    TVFORMAT_NO_CHANGE = 0,     /*theTV-in format is the same, do not need to change*/
    TVFORMAT_RTN_PREVIEW,       /*the TV-in format is changed, back to the preview mode from any other mode*/
    TVFORMAT_STILL_PREVIEW,     /*the TV-in format is changed, but still in preview mode*/
    TVFORMAT_CAPTURE_SUCCESS,   /*the TV-in format is changed, and return to capture video*/
    TVFORMAT_CAPTURE_FAIL       /*the TV-in format is changed, but return to capture video mode fail*/
} TV_FORMAT_STATUS;


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
#if (ROOTWORK==0)
extern s8 gsParseDirName[9];
#endif

extern u16 OSDDispWidth[];
extern u16 OSDDispHeight[];
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
#if (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
extern u32 uiSetLightDuration;
extern u32 uiCurrentLightTime;
extern u8  uiInScheduleLight;
extern u16  uiInManualLight;
extern u8  uiLightTimer[4];    /*10:11~12:14*/
extern u32 uiSetTriggerDimmer;
#endif
extern u8  uiSetRfLightTimer[MAX_RFIU_UNIT];
extern u8  uiSetRfLightDimmer[MAX_RFIU_UNIT];
extern u8  uiSetRfLightDur[MAX_RFIU_UNIT];
extern u8  uiSetRfLightState[MAX_RFIU_UNIT];
extern u8  uiRetrySnapshot[MAX_RFIU_UNIT];
extern u8  uiEnPair2Preview;

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
extern void uiSDStateInit(void);
extern s32 uiInit(void);
extern void uiMenuEnterPreview(u8 mode);
extern void uiMenuOSDFrame(u16 osd_w , u16 icon_w , u16 icon_h , u16 x_pos , u16 y_pos , u8 buf_idx , u32 data);
extern void uiMenuOSDShiftY(u16 osd_w , u16 *sy, u16 *ey , u8 shift_h, u8 buf_idx);
extern void uiMenuOSDReset(void);
extern void osdDrawFlashLight(u8);
extern u8 Get_Node_Total_Index(void);
extern void Read_FB_Setting(void);
extern void Read_UI_Setting(void);
extern void Save_UI_Setting(void);
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
extern void uiOsdDrawSystemUpdating(u8 buf_idx);
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
extern void osdDrawFillWait(void);
extern void osdDrawFillUSBMSC(void);
extern u8 uiSentKeyToUi(u32 Key);
extern void uiCheckTVInFormat(void);
extern u32 uiCheckVideoRec(void);
extern u8 uiReadSettingFromFlash(u8 *FlashAddr);
extern void uiReadRFIDFromFlash(u8 *FlashAddr);
extern void uiReadNetworkIDFromFlash(u8 *FlashAddr);
extern void uiReadVersionFromFlash(u8 * FlashAddr);
extern void uiWriteRFIDFromFlash(u8 *FlashAddr);
extern void uiWriteNetworkIDFromFlash(u8 *FlashAddr);
extern u8 uiSetDefaultSetting(void);
extern u8 uiWriteSettingToFlash(u8 *FlashAddr);   
extern u8 uiCaptureVideoStopByChannel(u8 Ch_ID);
extern u8 uiCaptureVideoByChannel(u8 Ch_ID);

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
extern void uiOsdDisableAll(void);
extern void uiOSDIconColorByXYChColor(OSD_ICONIDX icon_inx, u16 x_pos , u16 y_pos , u8 buf_idx, u8 bg_color, u8 alpha, u8 font_old_color, u8 font_new_color);
extern void osdDrawMessage(MSG_SRTIDX strIdx, u16 x_pos , u16 y_pos, u8 buf_idx, u8 obj_color, u8 bg_color);

extern u8 uiCaptureVideo(void);
extern void uiGraphGetNetworkInfo(u8* ip, u8* submask, u8* defaultGateway);
extern void osdDrawSDCD(u8 i);
extern void osdDrawShowZoom(u8 value);
extern void osdDrawFileNum(u32 num);
extern void osdDrawPlayIndicator(u8 type);
extern void osdDrawVideoTime(void);
extern void osdDrawVideoIcon(void);
extern void osdDrawSDIcon(u8 on);
extern void uiOSDPreviewInit(void);
extern void osdDrawRemoteOn(u8 on);

extern TV_FORMAT_STATUS uiChangeTVFormat(void);

#if ((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)\
    ||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    extern void uiOsdDrawBattery(u8 level, u8 act);
#else
    extern void uiOsdDrawBattery(u8 level);
#endif
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
extern void uiClearFfQuadBuf(u8 index);
extern u8 uiSetTalkOnOff(void);

    #if (SUPPORT_TOUCH == 1)
    extern u8  uiFlowCheckTouchKey(int TouchX, int TouchY);
    #if(HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505)
    extern u8  uiFlowPTZStop();
    #endif
    #endif
    #if (RFIU_SUPPORT)
    extern u8 uiSetRfResolutionRxToTx(s8 setting,u8 camera);
    extern u8 uiSetRfMotionRxToTx(s8 Enable, u8 dayLev, u8 nightLev, u8 camera);
    extern u8 uiSetRfPIRRxToTx(s8 Enable, u8 camera);
    extern u8 uiCheckRfTalkStatus(void);
    extern void uiCheckP2PMode(void);
    extern u8 uiSetTxVolumeRxToTx(s8 level,u8 camera);
    extern u8 uiSetRfBrightnessRxToTx(s8 brivalue,u8 camera);
    extern u8 uiSetTxSnapshotRxToTx(s8 videoSize,u8 camera);
    extern u8 uiSetTxPhotoTimeRxToTx(u8 hour,u8 min,u8 camera);
    extern u8 uiSetTxPushAppMsgRxToTx(u8 msgidx,u8 camera);

    #endif
    #if NIC_SUPPORT
    extern void uiSetP2PImageLevel(u8 CamId, u8 level);
    extern u8 uiSetP2PPassword(u8 *password);
    #endif
    #if (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
    extern u8 uiSetTxMusicRxToTx(s8 opvalue,u8 camera);
    extern void uiFlowStopTouchkeyAction(void);
    #endif
    #if ((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM))
    extern void uiOsdSetNightStatus(u8 Camid, u8 Val);
    #endif
    extern u8 u8BLEexist;

extern void uiSetAudioByCH(u8 channel, u8 level);
extern void uiOsdDrawNetworkLink(u8 LinkUp);
extern void uiOsdDrawSDCardFail(u8 act);
extern void uiOsdDrawRemindDownload(u8 RemindDown);
extern void uiFlowCardReady(u8 CardState);
extern void uiFlowSetRfLightStatus(u8 Camid, u8 Val, u8 Act);

#if RX_SNAPSHOT_SUPPORT
extern void uiRxSnapshot_All(int dummy1,int dummy2,int dummy3,int dummy4);
extern void uiRxSnapshot_One(int RFUnit,int dummy1,int dummy2,int dummy3);
#endif

extern bool UIDEBUG_PRINT;      



#endif


