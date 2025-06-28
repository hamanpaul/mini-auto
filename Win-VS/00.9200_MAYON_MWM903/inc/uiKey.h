

#ifndef __UIKEY_H__
#define __UIKEY_H__


//=================================================================
/* Key */
enum
{
    UI_KEY_UP = 0x01,
    UI_KEY_DOWN,
    UI_KEY_LEFT,
    UI_KEY_RIGHT,
    UI_KEY_ENTER,
    UI_KEY_MENU,
    UI_KEY_MODE,
    UI_KEY_TVOUT_DET,
    UI_KEY_USB_DET,
    UI_KEY_SDCD,
    UI_KEY_USBCD,	// NEW KEY
    UI_KEY_PWR_OFF,
    UI_KEY_STOP,
    UI_KEY_PLAY,
    UI_KEY_CH1,
    UI_KEY_CH2,
    UI_KEY_CH3,
    UI_KEY_CH4,
    UI_KEY_VGA,
    UI_KEY_HD,
    UI_KEY_CAPTURE_VIDEO,
    UI_KEY_TV_FORMAT_CHANGE,
    UI_KEY_RF_CH,
    UI_KEY_RF_QUAD,
    UI_KEY_RF_DUAL,
    UI_KEY_RF_PAIR,
    UI_KEY_RF_PAIR1,
    UI_KEY_DELETE,
    UI_KEY_UART,
    UI_KEY_TALK,
    UI_KEY_PARK,    /*Enter Parking Mode*/
    UI_KEY_REC,     /*Start Recording*/
    UI_KEY_AREC,     /*Start Recording*/
    UI_KEY_FWUP,    /*F/W Up-Grade From SD card*/
    UI_KEY_FORMAT,  /*Format SD Card*/
    UI_KEY_RESET,   /*RESET & Re-Booting*/
    UI_KEY_BUZZER,	/*BUZZER TOGGLE*/
    UI_KEY_MONITOR, /*monitoring*/
    UI_KEY_UNLOCK,
    UI_KEY_STANDBY,
    UI_KEY_LCD_BL,
    UI_KEY_TALK_OFF,
    UI_KEY_GOTO,
    UI_KEY_MAIN,
    UI_KEY_ENTER_PRV,
    UI_KEY_RF_TURBO,
    UI_KEY_Gate,
    UI_KEY_LONG_MONITOR,
    UI_KEY_ZERO,
    UI_KEY_ONE,
    UI_KEY_TWO,
    UI_KEY_THREE,
    UI_KEY_FOUR,
    UI_KEY_FIVE,
    UI_KEY_SIX,
    UI_KEY_SEVEN,
    UI_KEY_EIGHT,
    UI_KEY_NINE,
    UI_KEY_VOL,
    UI_KEY_LIGHT,
    UI_KEY_ALARM,
    UI_KEY_SDINIT,
#if USB_HOST_MASS_SUPPORT
    UI_KEY_USB_INSERT,
    UI_KEY_USB_READY,
#endif
	UI_KEY_DEVCD,	// NEW KEY for drive install
    UI_KEY_NONE,
};

/*UI Flow Definition*/
enum
{
    KEY_CODE_STOP = 0,  //STOP RECORDING
    KEY_CODE_PARK,     //ENTER PARKING MODE
    KEY_CODE_REC,       //START RECORDING
    KEY_CODE_FWUP,        //FW UP-GRADE FROM SD CARD
    KEY_CODE_FORMAT,    //FORMAT SD CARD
    KEY_CODE_RESET,     //RESET AND RE-BOOTING
    KEY_CODE_BUZZER,    //BUZZER TOGGLE
    KEY_CODE_NORMAL
};
typedef enum BUZZER_status
{
    BUZZER_NONE = 0,
    BUZZER_ON3S,        /*Keep ON for 3 secs (If booting is finished)*/
    BUZZER_ON1S,        /*Keep ON for 1 secs (1 sec ON after save the file)*/
    BUZZER_2ONF05S,     /*ON(500msec)+OFF(500msec) 2 times when detect g-sensor*/
    BUZZER_ON3SOFF,     /*ON for 3 secs when enter parking mode and OFF*/
    BUZZER_ON2SOFF05S,  /*ON(2 secs)+OFF(0.5sec) 2 times and ON(3 secs) when finished booting*/
    BUZZER_ONF1S,       /*ON(1 sec) + OFF(1 sec) total 5 times*/
    BUZZER_ONF05SOFF1S, /*(ON(0.5sec)+OFF(0.5sec)+ON(0.5sec)+OFF(1sec)) x n times until the condition removed*/
    BUZZER_SEONF05SOFF, /*Start : ON(0.5sec)+OFF(0.5sec)+ON(0.5sec)+OFF End : ON(0.5sec)+OFF(0.5sec)+ON(0.5sec)+OFF*/
    BUZZER_NONF05S,     /*(ON(0.5sec)+OFF(0.5sec)) x n times until complete to format*/
    BUZZER_ONF05SOFF,   /*ON(0.5sec)+OFF(0.5sec)+ON(0.5sec)+OFF*/
    BUZZER_OFF,      
} BUZZER_STATUS;
enum
{
    FILE_TYPE_DR    = 0,    // The recorded file on driving
    FILE_TYPE_ED,           // G-Sensor detected recording file on driving(Event during Driving)
    FILE_TYPE_EP,           // G-Sensor detected recording file on parking(Event during Parking)
    FILE_TYPE_EM,           // Recorded file by receiving "Enter Recording" signal(PORT1=0, PORT2=1, PORT3=0) - (Event norMal)
    FILE_TYPE_MD,           // Recorded file by motion detected on parking(Motion Detect)
};

enum
{
    VIDEO_MODE = 0,
    PLAYBACK_MODE,
    SETUP_MODE,
    PLAYBACK_MENU_MODE,
    SET_NETWORK,
    BY_PASS_MODE,
    GOTO_FORMAT_MODE,
    SET_DATETIME_MODE,
    OSD_SETUP_MODE,
    SET_MASK_AREA,
    SET_NUMBER_MODE,
    QUAD_MODE,
    STANDBY_MODE,
    SET_CONFIRM,
};

enum
{
    UI_MENU_SETIDX_CARDINFO = -6,
    UI_MENU_SETIDX_PLAYBACK,
    UI_MENU_SETIDX_NETWORK_INFO,
    UI_MENU_SETIDX_VERSION_INFO,
    UI_MENU_SETIDX_EXIT,
    UI_MENU_SETIDX_NO_ACTION,

    UI_MENU_SETIDX_REC_MODE_CH1,
    UI_MENU_SETIDX_REC_MODE_CH2,
    UI_MENU_SETIDX_REC_MODE_CH3,
    UI_MENU_SETIDX_REC_MODE_CH4,
    UI_MENU_SETIDX_MOTION_SENSITIVITY_CH1,
    UI_MENU_SETIDX_MOTION_SENSITIVITY_CH2,
    UI_MENU_SETIDX_MOTION_SENSITIVITY_CH3,
    UI_MENU_SETIDX_MOTION_SENSITIVITY_CH4,
    UI_MENU_SETIDX_CH1_ON,
    UI_MENU_SETIDX_CH2_ON,
    UI_MENU_SETIDX_CH3_ON,
    UI_MENU_SETIDX_CH4_ON,
    UI_MENU_SETIDX_RESOLUTION_CH1,
    UI_MENU_SETIDX_RESOLUTION_CH2,
    UI_MENU_SETIDX_RESOLUTION_CH3,
    UI_MENU_SETIDX_RESOLUTION_CH4,
    UI_MENU_SETIDX_BRIGHTNESS_CH1,
    UI_MENU_SETIDX_BRIGHTNESS_CH2,
    UI_MENU_SETIDX_BRIGHTNESS_CH3,
    UI_MENU_SETIDX_BRIGHTNESS_CH4,
    UI_MENU_SETIDX_OVERWRITE,
    UI_MENU_SETIDX_SECTION,
    UI_MENU_SETIDX_DATE_TIME,
    UI_MENU_SETIDX_FORMAT,
    UI_MENU_SETIDX_NETWORK_STATUS,//777 UI_MENU_SETIDX_DY_IP_ON
    UI_MENU_SETIDX_DISPLAY,
    UI_MENU_SETIDX_MOTION_MASK,
    UI_MENU_SETIDX_VOLUME,
    UI_MENU_SETIDX_IS_REC,
    UI_MENU_SETIDX_FULL_SCREEN,
    UI_MENU_SETIDX_P2P_LEVEL,
    UI_MENU_SETIDX_50HZ_60HZ, // 0:60 Hz  1:50Hz
    UI_MENU_SETIDX_DEFAULT,
    UI_MENU_SETIDX_ALARM,
    UI_MENU_SETIDX_LANGUAGE,
    UI_MENU_SETIDX_P2P_PASSWORD,
    UI_MENU_SETIDX_TV_OUT,  // 0:NTSC  1:PAL 2: Panel
    UI_MENU_SETIDX_NTP,
    UI_MENU_SETIDX_CH1_LS_STATUS,
    UI_MENU_SETIDX_CH2_LS_STATUS,
    UI_MENU_SETIDX_CH3_LS_STATUS,
    UI_MENU_SETIDX_CH4_LS_STATUS,
    UI_MENU_SETIDX_CH1_LS_ONOFF,
    UI_MENU_SETIDX_CH2_LS_ONOFF,
    UI_MENU_SETIDX_CH3_LS_ONOFF,
    UI_MENU_SETIDX_CH4_LS_ONOFF,
    UI_MENU_SETIDX_CH1_CA_STATUS,
    UI_MENU_SETIDX_CH2_CA_STATUS,
    UI_MENU_SETIDX_CH3_CA_STATUS,
    UI_MENU_SETIDX_CH4_CA_STATUS,
    UI_MENU_SETIDX_CH1_CA_ONOFF,
    UI_MENU_SETIDX_CH2_CA_ONOFF,
    UI_MENU_SETIDX_CH3_CA_ONOFF,
    UI_MENU_SETIDX_CH4_CA_ONOFF,   
    UI_MENU_SETIDX_MOTION_SECTION,
#if USB_HOST_MASS_SUPPORT
    UI_MENU_SETIDX_HDD_REMOVE,
#endif
    UI_MENU_SETIDX_SET_AREC,
    UI_MENU_SETIDX_CHECK,
    UI_MENU_SETIDX_LAST,   /*it should be keep in the last*/

/* By Pass */  
    UI_MENU_SETIDX_PAIRING,
    //UI_MENU_SETIDX_NETWORK,
    UI_MENU_SETIDX_NETWORK_KEYPAD,
    UI_MENU_SETIDX_SCHEDULED,
    UI_MENU_SETIDX_SCHEDULED_SET,
#if UI_LIGHT_SUPPORT
    UI_MENU_SETIDX_SCHEDULED_LIGHT,
    UI_MENU_SETIDX_SCHEDULED_LIGHT_SET,
#endif
    UI_MENU_SETIDX_VIDEO_QUALITY,
    UI_MENU_SETIDX_VIDEO_FRAMERATE,
    UI_MENU_SETIDX_VIDEO_SIZE,
    UI_MENU_SETIDX_CH1_PIR,
    UI_MENU_SETIDX_CH2_PIR,
    UI_MENU_SETIDX_CH3_PIR,
    UI_MENU_SETIDX_CH4_PIR,
    #if(TUTK_SUPPORT)
        UI_MENU_SETIDX_P2PID,
    #endif
    #if(RFIU_SUPPORT)
        UI_MENU_SETIDX_RFID,
        UI_MENU_SETIDX_RFID_CODE,
    #endif
    #if(NIC_SUPPORT)
        UI_MENU_SETIDX_MAC,
    #endif
    UI_MENU_SETIDX_UPGRADE_FW,
    UI_MENU_SETIDX_UPGRADE_FW_NET,
    UI_MENU_SETIDX_APP_INFO,
    UI_MENU_SETIDX_ST_IP_SET,
    UI_MENU_SETIDX_TIMESTAMP,
    UI_MENU_SETIDX_CH1_LS_TIMER,
    UI_MENU_SETIDX_CH2_LS_TIMER,
    UI_MENU_SETIDX_CH3_LS_TIMER,
    UI_MENU_SETIDX_CH4_LS_TIMER,
    UI_MENU_SETIDX_CH1_CA_TIMER,
    UI_MENU_SETIDX_CH2_CA_TIMER,
    UI_MENU_SETIDX_CH3_CA_TIMER,
    UI_MENU_SETIDX_CH4_CA_TIMER,

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
    MSG_MAX_NUM,             /*keep in the last*/
}MSG_SRTIDX;

typedef enum 
{
    OSD_ICON_WARNING_1 = 0,          /*WARNING*/
    OSD_ICON_VREC,             /*VREC*/
    OSD_ICON_VREC2,             
    OSD_ICON_FF,               /*FF*/
    OSD_ICON_REW,              /*REW*/
    OSD_ICON_PLAY,
    OSD_ICON_PLAYBACK,
    OSD_ICON_SD,
    OSD_ICON_STOP,
    OSD_ICON_MENU_1M,   /*8*/
    OSD_ICON_ATENNA_Signal_0,
    OSD_ICON_ATENNA_Signal_1,
    OSD_ICON_ATENNA_Signal_2,
    OSD_ICON_ATENNA_Signal_3,    
    OSD_ICON_ATENNA_Signal_4,   /*13*/
    OSD_ICON_SD_FULL,
    OSD_ICON_TALKBACK,
    OSD_ICON_TALK_ING1,
    OSD_ICON_MUTE,               
    OSD_ICON_CAM1,
    OSD_ICON_CAM2,
    OSD_ICON_CAM3,
    OSD_ICON_CAM4,               
    OSD_ICON_PAUSE,
    OSD_ICON_VOL_TOP,
    OSD_ICON_VOL0,
    OSD_ICON_VOL1,
    OSD_ICON_VOL2,
    OSD_ICON_VOL3,
    OSD_ICON_VOL4,              
    OSD_ICON_VOL5,
    OSD_ICON_VOL_BOTTOM,
    OSD_ICON_QUAD,
    OSD_ICON_VOL,
    OSD_ICON_RETURN,            
    OSD_ICON_PAUSE_L,
    OSD_ICON_BATTERY_LV0,
    OSD_ICON_BATTERY_LV1,       
    OSD_ICON_BATTERY_LV2,       
    OSD_ICON_BATTERY_LV3,
    OSD_ICON_BATTERY_CHARGE,
    OSD_ICON_NET_LINK_UP, 
    OSD_ICON_NET_LINK_DOWN,     
    OSD_ICON_DOWNLOAD,
    OSD_ICON_PLAY_ACT_STOP_1,
    OSD_ICON_PLAY_ACT_REW_1,
    OSD_ICON_PLAY_ACT_FF_1,
    OSD_ICON_PLAY_BAR_UP,
    OSD_ICON_PLAY_BAR_DOWN,
    OSD_ICON_DELETE,
#if UI_LIGHT_SUPPORT
    OSD_ICON_LIGHTING_SETUP_LIGHT,
    OSD_ICON_LIGHTING_SETUP_LIGHT_MANUAL,
#endif    
    OSD_ICON_MENU_TOP,
    OSD_ICON_MENU_MID,
    OSD_ICON_MENU_BOTTOM,
    OSD_ICON_MOTION_OPEN,
    OSD_ICON_MOTION_CLOSE,
    OSD_ICON_REC_BUTTON,
    OSD_ICON_ARROW_UP,
    OSD_ICON_ARROW_DOWN,    
#if UI_CAMERA_ALARM_SUPPORT
    OSD_ICON_MOTION_ALARM,
    OSD_ICON_MOTION_ALARM_MANUAL,
#endif
    OSD_ICON_LCD_BL,
    OSD_ICON_LOAD,

#if UI_BAT_SUPPORT
    OSD_ICON_BATCAM_BATTERY_LV0,
    OSD_ICON_BATCAM_BATTERY_LV1,       
    OSD_ICON_BATCAM_BATTERY_LV2,       
    OSD_ICON_BATCAM_BATTERY_LV3,
    OSD_ICON_BATCAM_BATTERY_CHARGE,
#endif

#if USB_HOST_MASS_SUPPORT
    OSD_ICON_HDD,
#endif

   // no use
    OSD_ICON_OVERWRITE,
    OSD_ICON_REMOTE,
    
    OSD_ICON_MAX_NUM,       /*Keep in last*/
    OSD_ICON_BLOCK,
    
    
}OSD_ICONIDX;

#define UI_MENU_SETIDX_CH1_RES      UI_MENU_SETIDX_VIDEO_SIZE

#define UIACTIONNUM                 UI_MENU_SETIDX_LAST
#define UI_SET_CHECKSUM               UIACTIONNUM

#define UI_KEY_READY                    0
#define UI_KEY_WAIT_KEY                 0xFFFFFFFF

#define VERSION                     "140314_1" 
#define VERNUM                      "140314_1"

#define MASKAREA_MAX_COLUMN             20
#define MASKAREA_MAX_ROW                15

#define P2PID_LENGTH                21
extern u8 uiP2PID[P2PID_LENGTH];

#define RFID_MAX_WORD               MAX_RF_DEVICE
extern u32 uiRFID[RFID_MAX_WORD];

extern u32 uiRFCODE[RFID_MAX_WORD];

#define MAC_LENGTH                  6
extern u8 uiMACAddr[MAC_LENGTH];


#define UI_LIB_LANGUAGE_NUM	        1	/* supported language types */
#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2)||\
    (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2))
    #define UI_LIB_PER_LANGUAGE_SIZE    0xC4D00        /* for SPI 2MB */
#else 
    //#define UI_LIB_PER_LANGUAGE_SIZE    0x2C4D00       /* for SPI 4MB */
    #define UI_LIB_PER_LANGUAGE_SIZE    0x6C4D00     /* for SPI 8MB */
    //#define UI_LIB_PER_LANGUAGE_SIZE    0xEC4D00     /* for SPI 16MB */
#endif

#if UI_ICONFLAG_BACKUP
#define UI_AUDIO_LIB_SIZE    0x31000     // 196KB, 4K for iconflag backup
#else
#define UI_AUDIO_LIB_SIZE    0x32000     // 200KB
#endif

#define UI_SETUP_GUID_SIZE     0x20                  /* Setup.ini each GUID use the size of memory*/
#define UI_SETUP_RFID_SIZE     RFID_MAX_WORD*4       /* Setup.ini each RFID use the size of memory*/
#define UI_SETUP_MAC_SIZE      0x10                  /* Setup.ini each MAC use the size of memory*/
#define UI_SETUP_RFCODE_SIZE   RFID_MAX_WORD*4       /* Setup.ini each RFCODE use the size of memory*/

#define UI_CONFIG_TOTAL_SIZE   0x1000      // 4KB

#endif  /*end of #ifndef __UIKEY_H__*/


