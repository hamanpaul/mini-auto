

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
    UI_KEY_FWUP,    /*F/W Up-Grade From SD card*/
    UI_KEY_FORMAT,  /*Format SD Card*/
    UI_KEY_RESET,   /*RESET & Re-Booting*/
    UI_KEY_BUZZER,	/*BUZZER TOGGLE*/
    UI_KEY_AUDIO_CH1,  
    UI_KEY_AUDIO_CH2,
    UI_KEY_LIGHT,
#if(HW_BOARD_OPTION == MR6730_AFN)	
	UI_KEY_B_RIGHT, /*FF*/	//for continuous play
#endif
#if Melody_SNC7232_ENA
	UI_KEY_Melody_Play,
	UI_KEY_Melody_PlayAll,
	UI_KEY_Melody_PlayNext,
	UI_KEY_Melody_Stop,
	UI_KEY_Melody_Pause,
	UI_KEY_Melody_Start,
	UI_KEY_Melody_StartAll,
	UI_KEY_Melody_Volume,
#endif
    UI_KEY_GCT_LIGHT,
    UI_KEY_TEST_MODE,
    UI_KEY_SEP_NIGHT_LIGHT,
    UI_KEY_SEP_LIGHT_OFF,
    UI_KEY_SEP_LIGHT_ON,
    UI_KEY_NONE
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
    QUAD_MODE,
    SET_NUMBER_MODE,
};

enum
{
    UI_MENU_SETIDX_CARDINFO = -4,
    UI_MENU_SETIDX_PLAYBACK,
    UI_MENU_SETIDX_EXIT,
    UI_MENU_SETIDX_NO_ACTION,

    UI_MENU_SETIDX_REC_MODE,
    UI_MENU_SETIDX_VIDEO_QUALITY,
    UI_MENU_SETIDX_VIDEO_FRAMERATE,
    UI_MENU_SETIDX_VIDEO_SIZE,
    UI_MENU_SETIDX_VIDEO_SIZE_CH2,
    UI_MENU_SETIDX_VIDEO_SIZE_CH3,
    UI_MENU_SETIDX_VIDEO_SIZE_CH4,
    UI_MENU_SETIDX_OVERWRITE,
    UI_MENU_SETIDX_SECTION,
    UI_MENU_SETIDX_CHANNEL,
    UI_MENU_SETIDX_DATE_TIME,
    UI_MENU_SETIDX_FORMAT,
    UI_MENU_SETIDX_NETWORK,
    UI_MENU_SETIDX_DISPLAY,
    UI_MENU_SETIDX_AUDRES,
    UI_MENU_SETIDX_SAMPLING,
    UI_MENU_SETIDX_MOTION_MASK,
    UI_MENU_SETIDX_MOTION_SENSITIVITY,
    UI_MENU_SETIDX_VOLUME,
    UI_MENU_SETIDX_IS_REC,
    UI_MENU_SETIDX_FULL_SCREEN,
    UI_MENU_SETIDX_CH1ZOOM,
    UI_MENU_SETIDX_CH2ZOOM,
    UI_MENU_SETIDX_CH3ZOOM,
    UI_MENU_SETIDX_CH4ZOOM,
    UI_MENU_SETIDX_CH1IMG,
    UI_MENU_SETIDX_CH2IMG,
    UI_MENU_SETIDX_CH3IMG,
    UI_MENU_SETIDX_CH4IMG,
    UI_MENU_SETIDX_TX_BRIGHT,
    UI_MENU_SETIDX_TX_MOTION,
    UI_MENU_SETIDX_TX_RES, 
    UI_MENU_SETIDX_MIC_VOLUME,
    UI_MENU_SETIDX_CH1_ON,
    UI_MENU_SETIDX_CH2_ON,
    UI_MENU_SETIDX_CH3_ON,
    UI_MENU_SETIDX_CH4_ON,
    UI_MENU_SETIDX_P2P_LEVEL,
#if NIC_SUPPORT
    UI_MENU_SETIDX_P2P_PASSWORD,
#endif
#if 0
    UI_MENU_SETIDX_CH1_BRIGHT,
    UI_MENU_SETIDX_CH1_CONTRAST,
    UI_MENU_SETIDX_CH1_FULL,
    UI_MENU_SETIDX_CH1_HUE,
    UI_MENU_SETIDX_CH2_BRIGHT,
    UI_MENU_SETIDX_CH2_CONTRAST,
    UI_MENU_SETIDX_CH2_FULL,
    UI_MENU_SETIDX_CH2_HUE,
    UI_MENU_SETIDX_CH3_BRIGHT,
    UI_MENU_SETIDX_CH3_CONTRAST,
    UI_MENU_SETIDX_CH3_FULL,
    UI_MENU_SETIDX_CH3_HUE,
    UI_MENU_SETIDX_CH4_BRIGHT,
    UI_MENU_SETIDX_CH4_CONTRAST,
    UI_MENU_SETIDX_CH4_FULL,
    UI_MENU_SETIDX_CH4_HUE,
#endif
    UI_MENU_SETIDX_BUZZER,  // Buzzer Toggle: 1: On, 0:Off
    UI_MENU_SETIDX_TV_OUT,  // 0:NTSC  1:PAL
    UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT,
    UI_MENU_SETIDX_PIR,
#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M))
    UI_MENU_SETIDX_TX_FLICKER,
#endif
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    UI_MENU_SETIDX_IR_MODE,
    UI_MENU_SETIDX_LIGHT_STATUS,
    UI_MENU_SETIDX_LIGHT_R,
    UI_MENU_SETIDX_LIGHT_G,
    UI_MENU_SETIDX_LIGHT_B,
    UI_MENU_SETIDX_LIGHT_L,
    UI_MENU_SETIDX_RECORD_MODE,
    UI_MENU_SETIDX_MOUNT_MODE,
    UI_MENU_SETIDX_NOISE_ALERT,
    UI_MENU_SETIDX_TEMP_ALERT,
    UI_MENU_SETIDX_TEMP_HIGH_MARGIN,
    UI_MENU_SETIDX_TEMP_LOW_MARGIN,
    UI_MENU_SETIDX_FLICKER_FREQUENCY,
#elif (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
    UI_MENU_SETIDX_LIGHT_TIMER,
    UI_MENU_SETIDX_LIGHT_DURATION,
    UI_MENU_SETIDX_LIGHT_DIMMER,
#endif
#if (HW_BOARD_OPTION  == MR6730_AFN)
	UI_MENU_SETIDX_PWR_DEFAULT,
#endif
#if ((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
	UI_MENU_SETIDX_MELODY,
	UI_MENU_SETIDX_NIGHT_LIGHT,
	UI_MENU_SETIDX_MELODY_VOL,
#endif
#if (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)
	UI_MENU_SETIDX_AP_SCONFIG_DETECT,
#endif
    UI_MENU_SETIDX_CHECK,
    UI_MENU_SETIDX_LAST,   /*it should be keep in the last*/

/* By Pass */
    UI_MENU_SETIDX_GRAPHIC_SETUP,
    UI_MENU_SETIDX_PAIRING,
    #if(TUTK_SUPPORT)
        UI_MENU_SETIDX_P2PID,
    #endif
    #if(RFIU_SUPPORT)
        UI_MENU_SETIDX_RFID,
    #endif
    #if(NIC_SUPPORT)
        UI_MENU_SETIDX_MAC,
    #endif
    UI_MENU_SETIDX_UPGRADE_FW,
};

#define UI_MENU_SETIDX_CH1_RES      UI_MENU_SETIDX_VIDEO_SIZE

#define UIACTIONNUM                 UI_MENU_SETIDX_LAST
#define UI_SET_CHECKSUM               (UIACTIONNUM+5)

#define UI_KEY_READY                    0
#define UI_KEY_WAIT_KEY                 0xFFFFFFFF

#define VERSION                     "0.04" 
#define VERNUM                      "140102_1"

#define MASKAREA_MAX_COLUMN             (352/16)
#define MASKAREA_MAX_ROW                (288/16)

#define P2PID_LENGTH                21
extern u8 uiP2PID[P2PID_LENGTH];

#define RFID_MAX_WORD               MAX_RF_DEVICE
extern u32 uiRFID[RFID_MAX_WORD];

extern u32 uiRFCODE[RFID_MAX_WORD];

#define MAC_LENGTH                  6
extern u8 uiMACAddr[MAC_LENGTH];


#define UI_LIB_LANGUAGE_NUM	        1	/* supported language types */
#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2)||\
    (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2)||\
    (SW_APPLICATION_OPTION == MR8211_IPCAM))
    #define UI_LIB_PER_LANGUAGE_SIZE    0xC4D00        /* for SPI 2MB */
#else 
    #define UI_LIB_PER_LANGUAGE_SIZE    0x2C4D00       /* for SPI 4MB */
    //#define UI_LIB_PER_LANGUAGE_SIZE    0x6C4D00     /* for SPI 8MB */
    //#define UI_LIB_PER_LANGUAGE_SIZE    0xEC4D00     /* for SPI 16MB */
#endif

#define UI_AUDIO_LIB_SIZE    0x32000     // 200KB

#define UI_SETUP_GUID_SIZE     0x20                  /* Setup.ini each GUID use the size of memory*/
#define UI_SETUP_RFID_SIZE     RFID_MAX_WORD*4       /* Setup.ini each RFID use the size of memory*/
#define UI_SETUP_MAC_SIZE      0x10                  /* Setup.ini each MAC use the size of memory*/
#define UI_SETUP_RFCODE_SIZE   RFID_MAX_WORD*4       /* Setup.ini each RFCODE use the size of memory*/

#define UI_CONFIG_TOTAL_SIZE   0x800      // 2KB
#if (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
#define UI_TRIGGER_DIMMER_TIME  600
#endif

#endif  /*end of #ifndef __UIKEY_H__*/

/*tx staus bit */
#define TXstausbit0 0x01 //TX day :0 night :1
#define TXstausbit1 0x02 //
#define TXstausbit2 0x04 //
#define TXstausbit3 0x08 //
#define TXstausbit4 0x10 //
#define TXstausbit5 0x20 //
#define TXstausbit6 0x40 //
#define TXstausbit7 0x80 //


