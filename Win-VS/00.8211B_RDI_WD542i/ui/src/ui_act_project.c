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
#include "rfiuapi.h"
#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
#include "dcfapi.h"
#if (TUTK_SUPPORT==1)
#include "../LwIP/include/tutk_P2P/AVIOCTRLDEFs.h"
#endif

#if (HW_BOARD_OPTION == MR6730_AFN) 
#include "MainFlow.h"
#endif
#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
#include "uartapi.h"
#include "gpioapi.h"
#endif


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
u8 uiSetUiSetting = 0;
u8 gu8TimeStamp = 1;    /*1: Enable, 0: Disable*/
u8 OSDTimestameLevel;   /*0: All On, 1: Time only, 2: ALL off*/
u8 uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_640x480;

u8 Current_REC_Mode = 0;
char wav_on=0;
u8 CurrLanguage = 0;
//u8 uiSetDefault=0;
s8 defaultvalue[] =
{
    UI_MENU_REC_MODE_MANUAL,
#if(HW_BOARD_OPTION == MR8211_ZINWELL)
    UI_MENU_VIDEO_QUALITY_HIGH,
#elif( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    UI_MENU_VIDEO_QUALITY_HIGH,
#else
    UI_MENU_VIDEO_QUALITY_MEDIUM,
#endif
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    UI_MENU_VIDEO_FRAMERATE_15,
#else
    #if(RF_DEMO_SETTING == 1)
    UI_MENU_VIDEO_FRAMERATE_10,
    #elif (HW_BOARD_OPTION == MR8211_ZINWELL)
    UI_MENU_VIDEO_FRAMERATE_30,
    #else
    UI_MENU_VIDEO_FRAMERATE_30,
    #endif
#endif

#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
   #if(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_SIU)
    UI_MENU_SETTING_RESOLUTION_VGA,  /*cam 1*/
   #else
    UI_MENU_SETTING_RESOLUTION_VGA, /*cam 1*/
   #endif
#elif(HW_BOARD_OPTION ==  A1018_EVB_128M)
    UI_MENU_SETTING_RESOLUTION_HD,
#elif( (HW_BOARD_OPTION ==  A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) )
    UI_MENU_SETTING_RESOLUTION_HD,
#else
   #if(SW_APPLICATION_OPTION == MR6730_CARDVR_2CH)
    UI_MENU_SETTING_RESOLUTION_HD,  /*cam 1*/
   #else
    UI_MENU_SETTING_RESOLUTION_VGA, /*cam 1*/
   #endif
#endif
    UI_MENU_SETTING_RESOLUTION_VGA, /*cam 2*/
    UI_MENU_SETTING_RESOLUTION_VGA, /*cam 3*/
    UI_MENU_SETTING_RESOLUTION_VGA, /*cam 4*/
#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    UI_MENU_SETTING_OVERWRITE_YES,
    UI_MENU_SETTING_SECTION_5MIN,
#elif (HW_BOARD_OPTION == MR8211_ZINWELL)
    UI_MENU_SETTING_OVERWRITE_YES,
    UI_MENU_SETTING_SECTION_20SEC,
#elif (HW_BOARD_OPTION == MR6730_AFN)
    UI_MENU_SETTING_OVERWRITE_YES,
    UI_MENU_SETTING_SECTION_5MIN,
#else
    UI_MENU_SETTING_OVERWRITE_NO,
    UI_MENU_SETTING_SECTION_1MIN,
#endif
    UI_MENU_SETTING_CHANNEL_1,
    0,  /*Time*/
    UI_MENU_FORMAT_NO,
    0, /*Network*/
    UI_MENU_PANEL,
    UI_AUDIO_RESOLUTION_8BIT,
    UI_AUDIO_SAMPLING_8K,
    0,  /*mask*/
    UI_MENU_SETTING_MOTION_SENSITIVITY_M,
#if(HW_BOARD_OPTION == MR8120_RX_RDI)
    0, /*volume (0-31)*/
#elif IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    0,  // volume (0-31): loudest (0) ~ quietest (31)
#elif ((HW_BOARD_OPTION==MR8120_TX_COMMAX) || (HW_BOARD_OPTION==MR8211_ZINWELL) || (HW_BOARD_OPTION==MR8120_TX_RDI)||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA671)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA530)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA672)||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA532)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
    0,  // volume (0-31): loudest (0) ~ quietest (31)
#elif (HW_BOARD_OPTION==MR8120_TX_JESMAY_LCD)
    5, /*volume (0-31)*/
#elif ((HW_BOARD_OPTION == MR8120_TX_TRANWO) || (HW_BOARD_OPTION == MR8120_TX_TRANWO2)||\
       (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
    3, /*volume (0-31)*/    
#elif (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
    5, /*volume (0-31)*/    
#elif (HW_BOARD_OPTION == MR8120_TX_DB2)
    0, /*volume (0-31)*/      
#else
    5, /*volume (0-31)*/
#endif
    0,  /*not rec*/
    UI_MENU_RF_FULL,    /*FULL_SCREEN*/
    5, /*zoom1*/
    5, /*zoom2*/
    5, /*zoom3*/
    5, /*zoom4*/
    0, /*image1*/
    0, /*image2*/
    0, /*image3*/
    0, /*image4*/
    4, /*Tx brightness*/
#if(SW_APPLICATION_OPTION == MR9670_DOORPHONE) 
    UI_MENU_TX_MOTION_ON,
#else
    UI_MENU_TX_MOTION_OFF,
#endif
#if (HW_BOARD_OPTION == MR8211_ZINWELL)
    UI_MENU_SETTING_RESOLUTION_HD,
#elif( (HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
    UI_MENU_SETTING_RESOLUTION_VGA,
#elif ( (HW_BOARD_OPTION == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8120_TX_DB2) ||\
    (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
   #if USE_704x480_RESO
    UI_MENU_SETTING_RESOLUTION_D1_480V,
   #else
    UI_MENU_SETTING_RESOLUTION_HD,
   #endif
#elif ( (HW_BOARD_OPTION == MR6730_AFN) && (HW_DERIV_MODEL == HW_DEVTYPE_CDVR_AFN720PSEN) )
    UI_MENU_SETTING_RESOLUTION_HD,
#else
   #if(CIU1_OPTION == Sensor_HM1375_YUV601)
    UI_MENU_SETTING_RESOLUTION_HD,
   #elif(CIU1_OPTION == Sensor_OV7740_YUV601)
    UI_MENU_SETTING_RESOLUTION_D1_480V, 
   #else
    UI_MENU_SETTING_RESOLUTION_VGA,
   #endif
#endif
#if((HW_BOARD_OPTION == MR8120_TX_RDI3) || (HW_BOARD_OPTION == MR8120_TX_RDI_AV) ||(HW_BOARD_OPTION == MR8120_TX_RDI)||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA671)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA530)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA672)||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA532)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
    0, /*MIC volume (0-31)*/
#else
    8, /*MIC volume (0-31)*/
#endif
    UI_MENU_SETTING_CAMERA_ON,  /*cam 1*/
    UI_MENU_SETTING_CAMERA_ON,  /*cam 2*/
    UI_MENU_SETTING_CAMERA_ON,  /*cam 3*/
    UI_MENU_SETTING_CAMERA_ON,  /*cam 4*/
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    3,              /*P2P Level*/
#else
    5,              /*P2P Level*/
#endif
#if NIC_SUPPORT
    0,  /*P2P password*/
#endif
    UI_MENU_SETTING_BUZZER_OFF,
    UI_MENU_NTSC,
    UI_MENU_SETTING_MOTION_SENSITIVITY_M,
    UI_MENU_SETTING_PIR_ON,
#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) ||(SW_APPLICATION_OPTION == MR8120_RFCAM_TX2))
    SENSOR_AE_FLICKER_60HZ,
#endif
#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
    AVIOCTRL_NIGHT_AUTO,    // UI_MENU_SETIDX_IR_MODE
    0x02,                   // UI_MENU_SETIDX_LIGHT_STATUS 0x01: OFF, 0x02: Random, 0x03: value assigned
    0xff,                   // UI_MENU_SETIDX_LIGHT_R
    0xff,                   // UI_MENU_SETIDX_LIGHT_G
    0xff,                   // UI_MENU_SETIDX_LIGHT_B
    0xff,                   // UI_MENU_SETIDX_LIGHT_L
    AVIOTC_RECORDTYPE_OFF,  // UI_MENU_SETIDX_RECORD_MODE
    AVIOCTRL_SDCARD_MOUNT,  // UI_MENU_SETIDX_MOUNT_MODE
    2,                      // NoiseAlert, 0x01: ON,  0x02:OFF 
    2,                      // TempAlert, 0x01: ON,  0x02:OFF 
    40,                     // TempHighMargin
    15,                     // TempLowMargin
#elif (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
    0,  /*Timer week*/
    UI_MENU_SETTING_LS_DUR_1MIN,
    UI_MENU_SETTING_LS_DIMMER_100,
#endif
#if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
    UI_MENU_SETIDX_MELODY_OFF,
#elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    GPIO_MUSIC_STOP,
#endif
#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) ||(HW_BOARD_OPTION  == MR8100_GCT_VM9710)||\
    (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    UI_MENU_SETIDX_LIGHT_OFF,
    UI_SET_MELODY_VOL_5, /*Because the function limits, the default vol 5 actually is the vol 7*/
#endif
#if (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)
    UI_BOOT_IN_NORMAL,
#endif
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
#if (Melody_SNC7232_ENA)
extern u8 Melody_audio_level;
#endif

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
void uiMenuSet_UpgradeServer()
{
    u8          err, i;

    if (Main_Init_Ready == 1)
        DEBUG_UI("uiMenuSet_UpgradeServer \r\n");
    
    uiCaptureVideoStop();
    for ( i = RFIU_TASK_PRIORITY_HIGH; i < MAIN_TASK_PRIORITY_END; i++)
    {
        if ((i == SYSTIMER_TASK_PRIORITY) ||
            (i == UI_TASK_PRIORITY) ||
        #if USB2WIFI_SUPPORT
            (i == USB_TASK_PRIORITY) ||
            (i == USB_DEVICE_TASK_PRIORITY)||
        #endif
            (i == UARTCMD_TASK1_PRIORITY) ||
#if ICOMMWIFI_SUPPORT 
            (i == TIMER_TICK_TASK_PRIORITY)||
            (i == ICOMM_TASK_01_PRIORITY)||
            (i == ICOMM_TASK_02_PRIORITY)||
            (i == ICOMM_TASK_03_PRIORITY)||
            (i == ICOMM_TASK_04_PRIORITY)||
            (i == ICOMM_TASK_05_PRIORITY)||
            (i == ICOMM_TASK_06_PRIORITY)||
            (i == ICOMM_TASK_07_PRIORITY)||
            (i == ICOMM_TASK_08_PRIORITY)||
            (i == ICOMM_TASK_09_PRIORITY)||
            (i == ICOMM_TASK_10_PRIORITY)||
            (i == ICOMM_TASK_11_PRIORITY)||
            (i == ICOMM_TASK_12_PRIORITY)||
            (i == ICOMM_TASK_13_PRIORITY)||
            (i == ICOMM_TASK_14_PRIORITY)
#else
			(i == TIMER_TICK_TASK_PRIORITY)
#endif

                
                
//                (i == TCPIP_THREAD_PRIO_t)||
//                (i == T_ETHERNETIF_INPUT_PRIO)||
//                (i == T_LWIPENTRY_PRIOR)
            )
        {
            continue;
        }
        DEBUG_UI("UI OSTaskDel %d!\n",i);
        OSTaskSuspend(i);
        OSTaskDel(i);
    }
}

s32 uiMenuSet_ChannelControl(u8 setting)
{
    DEBUG_UI("uiMenuSet_ChannelControl %d \r\n",setting);
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
    DEBUG_UI("uiMenuSet_Zoom %d \r\n",setting);
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

s32 uiMenuSet_IMG(u8 setting)
{
    DEBUG_UI("uiMenuSet_IMG %d \r\n",setting);
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

s32 uiMenuSet_REC_MODE(u8 setting)
{
    int i;
    Current_REC_Mode = setting;

    DEBUG_UI("uiMenuSet_REC_MODE %d \r\n",setting);
    switch(Current_REC_Mode)
    {
        case UI_MENU_REC_MODE_MOTION:
            sysCaptureVideoMode = (sysCaptureVideoMode & ASF_CAPTURE_OVERWRITE_ENA) |
                                  ASF_CAPTURE_EVENT_MOTION_ENA;
            #if MOTIONDETEC_ENA
            siuMotionDetect_ONOFF(1);
            #endif

            #if HW_MD_SUPPORT
            #if ( (HW_BOARD_OPTION == MR6730_AFN)&&(USE_MD_EN_ALWAYS_ON) )
            mduMotionDetect_ONOFF(1);//always enabled
            #else
            mduMotionDetect_ONOFF(1);
            #endif
            #endif

            break;
        case UI_MENU_REC_MODE_MANUAL:
            sysCaptureVideoMode = (sysCaptureVideoMode & ASF_CAPTURE_OVERWRITE_ENA);
            #if MOTIONDETEC_ENA
            siuMotionDetect_ONOFF(0);
            #endif

            #if HW_MD_SUPPORT
            #if ( (HW_BOARD_OPTION == MR6730_AFN)&&(USE_MD_EN_ALWAYS_ON) )
            mduMotionDetect_ONOFF(1);//always enabled
            #else
            mduMotionDetect_ONOFF(0);
            #endif
            #endif

            break;
        case UI_MENU_REC_MODE_PARKING:
            if(iconflag[UI_MENU_REC_MODE_MOTION] == UI_MENU_TX_MOTION_ON)
            {
                sysCaptureVideoMode = (sysCaptureVideoMode & ASF_CAPTURE_OVERWRITE_ENA) |
                                      ASF_CAPTURE_EVENT_MOTION_ENA | ASF_CAPTURE_EVENT_GSENSOR_ENA;
                #if MOTIONDETEC_ENA
                siuMotionDetect_ONOFF(1);
                #endif
                #if HW_MD_SUPPORT
                mduMotionDetect_ONOFF(1);
                #endif
                DEBUG_UI("mduMotionDetect_ONOFF\n\n");
            }
            else
            {
                sysCaptureVideoMode = (sysCaptureVideoMode & ASF_CAPTURE_OVERWRITE_ENA) |
                                      ASF_CAPTURE_EVENT_GSENSOR_ENA;
                #if MOTIONDETEC_ENA
                siuMotionDetect_ONOFF(0);
                #endif
                #if HW_MD_SUPPORT
                mduMotionDetect_ONOFF(0);
                #endif
            }
            break;

        case UI_MENU_REC_MODE_DRIVING:
            sysCaptureVideoMode = (sysCaptureVideoMode & ASF_CAPTURE_OVERWRITE_ENA);
            #if MOTIONDETEC_ENA
            siuMotionDetect_ONOFF(0);
            #endif

            #if HW_MD_SUPPORT
            mduMotionDetect_ONOFF(0);
            #endif

            break;
        default:
            #if MOTIONDETEC_ENA
            siuMotionDetect_ONOFF(0);
            #endif

            #if HW_MD_SUPPORT
            mduMotionDetect_ONOFF(0);
            #endif
            break;
    }

#if MULTI_CHANNEL_VIDEO_REC
    // Peter: 暫時每個channel都用同一個設定,要各channel獨立時再把這段code拿掉
    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
        VideoClipParameter[i].sysCaptureVideoMode   = sysCaptureVideoMode;
#endif

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

    DEBUG_UI("uiMenuSet_VideoQuality %d \r\n",setting);
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
    DEBUG_UI("uiMenuSetVideoFrameRate %d\r\n",setting);
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
{    
	u16 width, height;
    

	switch (setting)
	{	/*CY 0907*/
        case UI_MENU_VIDEO_SIZE_720x480:
            width = 720;
            height = 480;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_720x480;
            iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_VGA; //default

			break;

        case UI_MENU_VIDEO_SIZE_704x480:
            width = 704;
            height = 480;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_704x480;
            iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_D1_480V;

            #if((HW_BOARD_OPTION ==MR8120_TX_RDI2) || (HW_BOARD_OPTION ==MR8120_TX_RDI3))
                siu_SetD1_FPS(UI_MENU_VIDEO_SIZE_704x480);
            #endif
            
			break;

        case UI_MENU_VIDEO_SIZE_352x240:
            width = 352;
            height = 240;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_352x240;
            iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_352x240;
            
			break;    

        case UI_MENU_VIDEO_SIZE_720x576:
            width = 720;
            height = 576;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_720x576;
            iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_VGA; //default

			break;

        case UI_MENU_VIDEO_SIZE_704x576:
            width = 704;
            height = 576;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_704x576;
            iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_D1_576V;

            #if((HW_BOARD_OPTION ==MR8120_TX_RDI2) || (HW_BOARD_OPTION ==MR8120_TX_RDI3))
                siu_SetD1_FPS(UI_MENU_VIDEO_SIZE_704x576);
            #endif  
            
			break;

        case UI_MENU_VIDEO_SIZE_320x240:
            width   = 320;
        	height  = 240;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_320x240;
            iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_QVGA;
            break;

        case UI_MENU_VIDEO_SIZE_1280X720:
            width   = 1280;
            height  = 720;
            uiMenuVideoSizeSetting = UI_MENU_VIDEO_SIZE_1280X720;
            iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_HD;
            break;

		case UI_MENU_VIDEO_SIZE_640x480:
            width  = 640;
            height = 480;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_640x480;
            iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_VGA; //default

            break;

        case UI_MENU_VIDEO_SIZE_1920x1088:
            width  = 1920;
            height = 1088;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_1920x1088;
            iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_VGA; //default

            break;

		default:
	        width  = 640;
            height = 480;
            uiMenuVideoSizeSetting=UI_MENU_VIDEO_SIZE_640x480;
            iconflag[UI_MENU_SETIDX_TX_RES] = UI_MENU_SETTING_RESOLUTION_VGA; //default

			break;
	}

	DEBUG_UI("-->uiMenuSet_VideoSize: %d,%d\n",width,height);
	siuSetVideoResolution(width, height);
#if (MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_MP4)
	mp4SetVideoResolution(width, height);
#elif (MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_ASF)
	asfSetVideoResolution(width, height);
#elif (MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_AVI)	/*Peter 0704 S*/
	aviSetVideoResolution(width, height);
#endif							/*Peter 0704 E*/
    Save_UI_Setting();
	return 1;
}

s32 uiMenuSet_Channel(u8 setting)
{
    DEBUG_UI("uiMenuSet_Channel %d \r\n",setting);
#if 1
    return 0;
#else
    switch (setting)
    {
        case UI_MENU_SETTING_CHANNEL_1:
            sysVideoInCHsel= 1;
            DEBUG_UI("Video Channel 1  \r\n",sysVideoInCHsel);
            break;

        case UI_MENU_SETTING_CHANNEL_2:

            sysVideoInCHsel= 2;
            DEBUG_UI("Video Channel 2  \r\n",sysVideoInCHsel);
            break;
        default:
            return 1;
    }
    return 1;
#endif
}


void uiSetOutputMode(u8 ucMode)
{
    //--Check iff under Video recording or playback--//
    uiCaptureVideoStop();
    uiPlaybackStop(GLB_ENA);

	//-- Stop Preview Path--//
#if CIU_TEST
    ciu_1_Stop();
#else
 #if MULTI_CHANNEL_SUPPORT
    #if(MULTI_CHANNEL_SEL & 0x01)
    isuStop();
    ipuStop();
    siuStop();
    #endif

    #if(MULTI_CHANNEL_SEL & 0x02)
    ciu_1_Stop();
    #endif

    #if(MULTI_CHANNEL_SEL & 0x04)
    ciu_2_Stop();
    #endif

    #if(MULTI_CHANNEL_SEL & 0x08)
    ciu_3_Stop();
    #endif
 #else
    isuStop();
    ipuStop();
    siuStop();
 #endif
#endif

    if(ucMode == SYS_OUTMODE_TV)
    {
        //--Back Light off---//
    #if(HW_BOARD_OPTION == MR8120_RX_DEMO_BOARD )
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
    #elif ((HW_BOARD_OPTION == MR8200_RX_DB2) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD))
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
    #endif
        OSTimeDly(2);
        sysTVOutOnFlag=GLB_ENA;
        DEBUG_UI("SET to TV-OUT!!\n\r");
        IduIntCtrl = 0x00000000;    //IDU interrupt control *
        IduWinCtrl &= (~0x0f);
        IduOsdL1Ctrl  &= ~(0x00000010);
        IduOsdL2Ctrl  &= ~(0x00000010);
        OSTimeDly(2);
        IduEna = 0;

    #if ((HW_BOARD_OPTION == MR8200_RX_DB2) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD))
        gpioSetLevel(GPIO_GROUP_LCD_EN, GPIO_BIT_LCD_EN, 0);
    #endif

    #if(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P)
        TvOutMode = SYS_TV_OUT_HD720P;
    #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I)
        TvOutMode = SYS_TV_OUT_FHD1080I;
    #else
        TvOutMode = SYS_TV_OUT_NTSC;
    #endif

        //---Lucian: 設定 CCIR656 Pannel,讓TV out與 656-Panel 同時輸出---//
   #if( (LCM_OPTION == LCM_HX8224) || (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) || (LCM_OPTION == LCM_HX8224_SRGB))
        IDU_Init(0, 1);

        IduDispCfg &= ~0x0000000f;
        IduDispCfg |= 0x00000005;//IDU_INTF_SPI ;
        IduMpuCmdCfg = 0x001e0028;
      #if(TV_DigiOut_SEL ==TV_DigiOut_YUV)
        IduMpuCmd = 0x00006083;
      #else //656
        IduMpuCmd = 0x00006002;
      #endif
        iduWaitCmdBusy();
   #endif

        ResetisuPanel();
        uiSetTVOutXY(TvOutMode);
        uiFlowRedirection();

        //============是否關掉digital TV out===========//
    #if( (LCM_OPTION ==LCM_HX8224_601) || (LCM_OPTION ==LCM_HX8224_656))

    #elif( (HW_BOARD_OPTION ==  A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) || (HW_BOARD_OPTION  == MR9600_RX_OPCOM) )

    #elif(HDMI_TXIC_SEL !=  HDMI_TX_NONE)

    #else
        tvTV_CONF &= (~0x00400000);  //Lucian: Turn off digital TV out.
    #endif

    }
    else
    {
        DEBUG_UI("SET to PANEL-OUT!!\n\r");
        sysTVOutOnFlag=GLB_DISA;

        tvTVE_INTC   =0x00000000;    //TV interrupt control *
        tvFRAME_CTL &= (~0x0f);
        IduOsdL1Ctrl  &= ~(0x00000010);
		IduOsdL2Ctrl  &= ~(0x00000010);
        OSTimeDly(2);
        IduEna = 0;
		iduRst();

        //---Lucian: 設定 CCIR656 Pannel---//
   #if( (LCM_OPTION == LCM_HX8224) || (LCM_OPTION == LCM_HX8224_SRGB))
        IDU_Init(0, 1);
        IduDispCfg &= ~0x0000000f;
        IduDispCfg |= 0x00000005;//IDU_INTF_SPI ;
        IduMpuCmdCfg = 0x001e0028;
        IduMpuCmd = 0x00006000;
        iduWaitCmdBusy();
   #elif((LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656))
        IDU_Init(0, 1);
        IduDispCfg &= ~0x0000000f;
        IduDispCfg |= 0x00000005;//IDU_INTF_SPI ;
        IduMpuCmdCfg = 0x001e0028;
      #if(TV_DigiOut_SEL ==TV_DigiOut_YUV)
        IduMpuCmd = 0x00006083;
      #else //656
        IduMpuCmd = 0x00006002;
      #endif
        iduWaitCmdBusy();
   #endif

    #if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A))
        IduYCbCr2R=0x002C0020;
        IduYCbCr2G=0x00968B20;
        IduYCbCr2B=0x00003820;
    #endif
        ResetisuPanel();
        uiSetTVOutXY(UI_MENU_SETTING_PANEL_OUT);
    #if ((HW_BOARD_OPTION == MR8200_RX_DB2) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD))
        gpioSetLevel(GPIO_GROUP_LCD_EN, GPIO_BIT_LCD_EN, 1);
        OSTimeDly(1);
    #endif
        uiFlowRedirection();

        //----Turn on back light---//
        OSTimeDly(2);
    #if(HW_BOARD_OPTION == MR8120_RX_DEMO_BOARD )
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
    #elif ((HW_BOARD_OPTION == MR8200_RX_DB2) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD))
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
    #endif

    }
}

s32 uiMenuSet_Display(u8 setting)
{
    DEBUG_UI("uiMenuSet_Display %d \r\n",setting);
    return;
    switch (setting)
    {
        case UI_MENU_TV:
            if (sysTVOutOnFlag == SYS_OUTMODE_PANEL)
            {
                DEBUG_UI("PANNEL MODE\r\n");
                uiSetOutputMode(SYS_OUTMODE_TV);
            }
            break;

        case UI_MENU_PANEL:
            if (sysTVOutOnFlag == SYS_OUTMODE_TV)
            {
                DEBUG_UI("PANNEL MODE\r\n");
                uiSetOutputMode(SYS_OUTMODE_PANEL);
            }
            break;

        default:
            return 1;
    }
    return 1;
}

s32 uiMenuSet_TVout_Format(u8 setting)
{
#if(TV_DECODER == TVDEC_NONE)
    DEBUG_UI("uiMenuSet_TVout_Format %d \r\n",setting);

    switch (setting)
    {
        case UI_MENU_NTSC:
            //if (sysTVOutOnFlag == TV_IN_PAL)
            {
                DEBUG_UI("TV out change to NTSC\r\n");
                sysTVinFormat = TV_IN_NTSC;
                iduSwitchNTSCPAL(SYS_TV_OUT_NTSC);
            }
            break;

        case UI_MENU_PAL:
            //if (sysTVinFormat == TV_IN_NTSC)
            {
                DEBUG_UI("TV out change to PAL\r\n");
                sysTVinFormat = TV_IN_PAL;
                iduSwitchNTSCPAL(SYS_TV_OUT_PAL);
            }
            break;

        default:
            return 1;
    }
    return 1;
#endif
}


s32 uiMenuSet_AudRes(u8 setting)
{
    DEBUG_UI("uiMenuSet_AudRes %d \r\n",setting);
    switch (setting)
    {
        case UI_AUDIO_RESOLUTION_8BIT:
            break;
        case UI_AUDIO_RESOLUTION_16BIT:
            break;
        default:
            return 1;
    }
    return 1;
}

s32 uiMenuSet_Sampling(u8 setting)
{
    DEBUG_UI("uiMenuSet_Sampling %d \r\n",setting);
    switch (setting)
    {
        case UI_AUDIO_SAMPLING_8K:
            break;
        case UI_AUDIO_SAMPLING_16K:
            break;
        case UI_AUDIO_SAMPLING_32K:
            break;
        case UI_AUDIO_SAMPLING_44K:
            break;
        case UI_AUDIO_SAMPLING_48K:
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
s32 uiMenuSet_VideoResolution(s8 setting)
{
#if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_D1) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
    setting = UI_MENU_SETTING_RESOLUTION_D1_480V;
#endif
    DEBUG_UI("uiMenuSet_VideoResolution %d\r\n",setting);
    switch (setting)
    {
        case UI_MENU_SETTING_RESOLUTION_D1_480V:
            uiMenuSet_VideoSize(UI_MENU_VIDEO_SIZE_704x480);
            break;

        case UI_MENU_SETTING_RESOLUTION_352x240:
            uiMenuSet_VideoSize(UI_MENU_VIDEO_SIZE_352x240);
            break;
            
        case UI_MENU_SETTING_RESOLUTION_D1_576V:
            uiMenuSet_VideoSize(UI_MENU_VIDEO_SIZE_704x576);
            break;
            
        case UI_MENU_SETTING_RESOLUTION_VGA:
            if(sysTVinFormat  == TV_IN_PAL )
                uiMenuSet_VideoSize(UI_MENU_VIDEO_SIZE_640x480);
            else
                uiMenuSet_VideoSize(UI_MENU_VIDEO_SIZE_640x480);
            break;

        case UI_MENU_SETTING_RESOLUTION_QVGA:
            if(sysTVinFormat  == TV_IN_PAL )
                uiMenuSet_VideoSize(UI_MENU_VIDEO_SIZE_320x240);
            else
                uiMenuSet_VideoSize(UI_MENU_VIDEO_SIZE_320x240);
            break;

        case UI_MENU_SETTING_RESOLUTION_HD:
            uiMenuSet_VideoSize(UI_MENU_VIDEO_SIZE_1280X720);
            break;

        default:
            return 0;
    }
    return 1;
}

void uiMenuSet_RXCameraResolution(s8 setting,u8 camera)
{
#if (RFIU_SUPPORT)
    uiSetRfResolutionRxToTx(setting, camera);
#endif
}

s32 uiMenuSet_Overwrite(u8 setting)
{
    u32 i;
    DEBUG_UI("uiMenuSet_Overwrite %d \r\n",setting);
    switch (setting)
    {
        case UI_MENU_SETTING_OVERWRITE_YES:
            //sysCaptureVideoMode = ASF_CAPTURE_OVERWRITE;
            sysCaptureVideoMode    |= ASF_CAPTURE_OVERWRITE_ENA;
            #if MULTI_CHANNEL_VIDEO_REC
            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                VideoClipParameter[i].sysCaptureVideoMode  |= ASF_CAPTURE_OVERWRITE_ENA;
            #endif
            break;

        case UI_MENU_SETTING_OVERWRITE_NO:
            //sysCaptureVideoMode = ASF_CAPTURE_NORMAL;
            sysCaptureVideoMode    &= ~ASF_CAPTURE_OVERWRITE_ENA;
            #if MULTI_CHANNEL_VIDEO_REC
            for(i = 0; i < MULTI_CHANNEL_MAX; i++)
                VideoClipParameter[i].sysCaptureVideoMode  &= ~ASF_CAPTURE_OVERWRITE_ENA;
            #endif
            break;

        default:
            return 0;
    }
    return 1;
}

s32 uiMenuSet_Section(u8 setting)
{
    DEBUG_UI("uiMenuSet_Section %d \r\n",setting);
    switch (setting)
    {

        case UI_MENU_SETTING_SECTION_20SEC:
            asfSetVideoSectionTime(20);
            break;
        case UI_MENU_SETTING_SECTION_1MIN:
               asfSetVideoSectionTime(1 * 60);
            break;
        case UI_MENU_SETTING_SECTION_5MIN:
               asfSetVideoSectionTime(5 * 60);
            break;
        case UI_MENU_SETTING_SECTION_10MIN:
               asfSetVideoSectionTime(10 * 60);
            break;
        case UI_MENU_SETTING_SECTION_15MIN:
               asfSetVideoSectionTime(15 * 60);
            break;

        case UI_MENU_SETTING_SECTION_30MIN:
            asfSetVideoSectionTime(30 * 60);
            break;

	#if (HW_BOARD_OPTION == MR6730_AFN )
		#if(RECFILELEN_60MIN==1)	
		case UI_MENU_SETTING_SECTION_60MIN:
			asfSetVideoSectionTime(60 * 60);
			break;	
		#endif	
		#if(RECFILELEN_120MIN==1)
		case UI_MENU_SETTING_SECTION_120MIN:
			asfSetVideoSectionTime(120 * 60);
			break;	
		#endif					
	#endif 


        default:
            asfSetVideoSectionTime(150);
            return 1;
    }
    return 1;
}

s32 uiMenuSet_MotionMask(void)
{
    DEBUG_UI("uiMenuSet_MotionMask\r\n");

#if HW_MD_SUPPORT
    mduMotionDetect_Mask_Config(&MotionMaskArea[0][0]);
#endif

    iconflag[UI_MENU_SETIDX_MOTION_MASK] = 0;
    return 1;
}

s32 uiMenuSet_MotionSensitivity(u8 setting)
{
    u8 level;

    DEBUG_UI("Motion Sensitivity %d \r\n",setting);
    switch (setting)
    {
        case UI_MENU_SETTING_MOTION_SENSITIVITY_H:
            level = 0;
            break;

        case UI_MENU_SETTING_MOTION_SENSITIVITY_M:
            level = 1;
            break;

        case UI_MENU_SETTING_MOTION_SENSITIVITY_L:
            level = 2;
            break;

        default:
            return 0;
    }
    #if MOTIONDETEC_ENA
      siuMotionDetect_Sensitivity_Config(level);
    #endif

    #if HW_MD_SUPPORT
      mduMotionDetect_Sensitivity_Config(level);
    #endif

    return 1;
}

s32 uiMenuSet_DateTime(u8 setting)
{
    DEBUG_UI("uiMenuSet_DateTime %d\r\n",setting);
    /*the setting value = 1 means the datetime is set by user not by save value*/
    if(setting == 0)
        return 0;
    RTC_Set_Time(&SetTime);
    iconflag[UI_MENU_SETIDX_DATE_TIME] = 0;
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

    DEBUG_UI("uiMenuSet_Format %d\r\n",setting);
	if (setting == UI_MENU_FORMAT_YES)
	{
		iconflag[UI_MENU_SETIDX_FORMAT] = UI_MENU_FORMAT_NO; /* Reset default to "No" */
    if(OSFlagAccept(gUiStateFlagGrp, FLAGUI_SD_GOTO_FORMAT, OS_FLAG_WAIT_SET_ANY, &err)>0)
        sysbackSetEvt(SYS_BACK_PLAYBACK_FORMAT, 0);
    else
        sysSetEvt(SYS_EVT_PLAYBACK_FORMAT, 0);
	}

	return 1;
}

s32 uiMenuSet_Network(void)
{
#if(NIC_SUPPORT)
    if (UINetInfo.IsStaticIP == 0)
    {
        DEBUG_UI("Enable DHCP\r\n");
        uiISStatic = 0;
    }
    else
    {
        DEBUG_UI("IP Address       :%u.%u.%u.%u \n",UINetInfo.IPaddr[0], UINetInfo.IPaddr[1], UINetInfo.IPaddr[2], UINetInfo.IPaddr[3]);
        DEBUG_UI("Subnet Mask      :%u.%u.%u.%u \n",UINetInfo.Netmask[0], UINetInfo.Netmask[1], UINetInfo.Netmask[2], UINetInfo.Netmask[3]);
        DEBUG_UI("Default Getway   :%u.%u.%u.%u \n",UINetInfo.Gateway[0], UINetInfo.Gateway[1], UINetInfo.Gateway[2], UINetInfo.Gateway[3]);
        uiISStatic = 1;
        SetNetworkInfo(&UINetInfo);
    }
#endif
}

#if (Melody_SNC7232_ENA)
s32 uiMenuSet_SNC7232_AudioVolume_init(s8 setting)
{
    //u8 volVal[6] = {31, 16, 12, 8, 4, 0};
    u8 volVal[6] = {31, 18, 14, 10, 6, 2};
    u8 nlevel;
    
    DEBUG_UI("uiMenuSet_SNC7232_AudioVolume_init %d \r\n",setting);

    switch(setting)
    {
        case 31:
            Melody_audio_level=0;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,0);
            #endif
            break;

        case 18:
            Melody_audio_level=1;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            #endif
            break;
            
        case 14:
            Melody_audio_level=2;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            #endif
            break;
            
        case 10:
            Melody_audio_level=3;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            #endif
            break;
            
        case 6:
            Melody_audio_level=4;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            #endif
            break;
            
        case 2:
            Melody_audio_level=5;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            #endif
            break;
            
        default:
            return 0;
            break;
    }
    if(setting !=31)
		Melody_SNC7232_AudioVolume(Melody_audio_level);

    return 1;
}
s32 uiMenuSet_SNC7232_AudioVolume(s8 setting)
{
    //u8 volVal[6] = {31, 16, 12, 8, 4, 0};
    u8 volVal[6] = {31, 18, 14, 10, 6, 2};
    u8 nlevel;
    
    DEBUG_UI("uiMenuSet_SNC7232_AudioVolume %d \r\n",setting);

    switch(setting)
    {
        case 31:
            Melody_audio_level=0;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,0);
            #endif
            break;

        case 18:
            Melody_audio_level=1;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            #endif
            break;
            
        case 14:
            Melody_audio_level=2;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            #endif
            break;
            
        case 10:
            Melody_audio_level=3;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            #endif
            break;
            
        case 6:
            Melody_audio_level=4;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            #endif
            break;
            
        case 2:
            Melody_audio_level=5;
            #if SPK_CONTROL
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            #endif
            break;
            
        default:
            return 0;
            break;
    }
    if(setting !=31)
        uiSentKeyToUi(UI_KEY_Melody_Volume);

    return 1;
}
#endif
#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
s32 uiMenuSetMelodyVolCtrl(u8 setting)
{
    u8  SendCmd;
  #if(HW_BOARD_OPTION == MR8211_TX_RDI_SEP) // MR8211B_TX_RDI_WD542I setting is zero. 
    if(setting != 0)
        setting += 2;
  #endif
    switch(setting)
    {
        case UI_SET_MELODY_VOL_0:
            SendCmd = 0x30;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,0);
            break;
            
        case UI_SET_MELODY_VOL_1:
            SendCmd = 0x31;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            break;

        case UI_SET_MELODY_VOL_2:
            SendCmd = 0x32;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            break;

        case UI_SET_MELODY_VOL_3:
            SendCmd = 0x33;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            break;

        case UI_SET_MELODY_VOL_4:
            SendCmd = 0x34;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            break;
            
        case UI_SET_MELODY_VOL_5:
            SendCmd = 0x35;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            break;

        case UI_SET_MELODY_VOL_6:
            SendCmd = 0x36;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            break;

        case UI_SET_MELODY_VOL_7:
            SendCmd = 0x37;
            sendchar(RDI_SEP_MCU_UART_ID, &SendCmd);
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,1);
            break;

        default:
            DEBUG_UI("uiMenuSetMelodyVolCtrl_TX Error Setting(%d)\n",setting);
            break;

    }
    DEBUG_UI("uiMenuSetMelodyVolCtrl_TX: Vol(%d),SendCmd(0x%02x)\n",setting , SendCmd);
}
#endif
s32 uiMenuSet_Audio_Vol(s8 setting)
{
    DEBUG_UI("uiMenuSet_Audio_Vol %d\r\n",setting);

    if (Main_Init_Ready == 0)
        sysVolumnControl = setting;

#if (HW_BOARD_OPTION != MR6730_AFN )	
    #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
    adcSetDAC_OutputGain((u32) setting);
      #if Melody_SNC7232_ENA
      if (Main_Init_Ready == 1)
          uiMenuSet_SNC7232_AudioVolume(setting);
      #endif
    #elif((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    adcSetDAC_OutputGain((u32) setting);
    #else
	adcSetDAC_OutputGain((u32) setting);
    #endif

#else	//#if (HW_BOARD_OPTION == MR6730_AFN )	

		if (Main_Init_Ready == 0)
		{
			DEBUG_UI("\nDAC Off before initialization done\n");
		#if (USE_EXT_DAC_CTRL)
				UI_AudioOut_Mute(1);
		#else
			{
				u32 RegVal=DacTxCtrl;
				
				if(RegVal&(DAC_ENDAL_ON|DAC_ENDAR_ON))
				{//make DAC silence
					//adcSetDAC_OutputGain(0);
					adcSetDAC_OutputGain(31);//0:max,31:mute
					_APP_ENTER_CS_;
					DacTxCtrl &= ~(DAC_ENDAL_ON|DAC_ENDAR_ON); // turn off DAC_OUT_L , DAC_OUT_R				
					_APP_EXIT_CS_;
					DEBUG_UI("\nDAC quiet!\n");
				}
			}	
		#endif
		
			return 1;
		}
		else	
		{
			//DEBUG_UI("\nsetting=%d\n",setting);	
		#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH) )
			if(setting<VOL_CTRL_MAX)	//Volume:0~31			
		#else
			if(setting<10)	//Volume:0~9 only		
		#endif
			{
				u8 idx=(u8)(setting&0x001F);//0~31
				u32  db=0;
				
			#if (HW_BOARD_OPTION == MR6730_AFN)
				#if (HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)			
					/*
					//---------------------------------------------
					//Volume to DAC Gain(db) mapping table
					static const u8 DAC_VolMapToGain[10]={
						//0~9 : Vol-Max~Mute
						//mapping to 
						//0~31 : DAC Gain(db) Max~Mute
						0,3,6,10,15,20,24,28,30,31
					};
					//---------------------------------------------
				
					db=DAC_VolMapToGain[idx];
					//DEBUG_UI("DAC Gain:0x%02X(db)\n",db);				
					*/
					
					#if(VOL_CTRL_MAX==32)
						//0~31 : Vol-Max~Mute
						//mapping to 
						//0~31 : DAC Gain(db) Max~Mute
						assert(idx<32);
						db=idx;	
					#elif(VOL_CTRL_MAX==16)
						//0~15 : Vol-Max~Mute
						//mapping to 
						//0~31 : DAC Gain(db) Max~Mute

						#if 1
						static const u8 DAC_VolMapToGain[VOL_CTRL_MAX]={
						0,1,4,7,10,12,13,14,16,18,20,22,24,26,28,31
						};	
						assert(idx<16);
						db=DAC_VolMapToGain[idx];
						//DEBUG_UI("DAC Gain:0x%02X(db)\n",db); 		

						#else
						assert(idx<16);

						if(idx==15)
							db=31;//set to max
						else
							db=idx*2;
						#endif
					#elif(VOL_CTRL_MAX==10)
						//Volume to DAC Gain(db) mapping table
						static const u8 DAC_VolMapToGain[10]={
							//0~9 : Vol-Max~Mute
							//mapping to 
							//0~31 : DAC Gain(db) Max~Mute
							0,3,6,10,15,20,24,28,30,31
						};
						assert(idx<VOL_CTRL_MAX);
						db=DAC_VolMapToGain[idx];
						//DEBUG_UI("DAC Gain:0x%02X(db)\n",db);					
					#else
						assert(idx<VOL_CTRL_MAX);
						db=idx;
					#endif
					
				#else
				//Volume:0~9 only
				
					//Volume to DAC Gain(db) mapping table
					static const u8 DAC_VolMapToGain[10]={
						//0~9 : Vol-Max~Mute
						//mapping to 
						//0~31 : DAC Gain(db) Max~Mute
						//0,3,6,10,15,20,24,28,30,31
						0,2,4,6,9,12,15,18,20,31
					};
					//assert(idx<10);
					if(idx<10)
						db=DAC_VolMapToGain[idx];
					else
						db=(10/2);//default
					//DEBUG_UI("DAC Gain:0x%02X(db)\n",db); 		
				#endif	
				
			#else
				db=idx;//direct use the "sysVolumnControl"
			#endif
			
				adcSetDAC_OutputGain((u32) db);
			}
	
		#if (USE_EXT_DAC_CTRL)		
				//DEBUG_UI("\nDAC:0x%08X (%d)\n",DacTxCtrl,setting);//debug only
			    #if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)
        		if(setting==(VOL_CTRL_MAX-1))
				#else
				if(setting==9)
				#endif
				{					
					UI_AudioOut_Mute(1);
				}
				else
				{
					UI_AudioOut_Mute(0);
				}
		#else
			    #if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH)
        		if(setting==(VOL_CTRL_MAX-1))
				#else		
				if(setting==9)
				#endif	
				{					
					if(DacTxCtrl&(DAC_ENDAL_ON|DAC_ENDAR_ON))
					{
						////DAC_turnOffBoth();
						_APP_ENTER_CS_;
						DacTxCtrl &= ~(DAC_ENDAL_ON|DAC_ENDAR_ON); // turn off DAC_OUT_L , DAC_OUT_R				
						_APP_EXIT_CS_;
						DEBUG_UI("\nDAC Tx Off\n");
					}
				}
				else
				{
					if(!(DacTxCtrl&(DAC_ENDAL_ON|DAC_ENDAR_ON)))
					{
						////DAC_turnOnBoth();
						_APP_ENTER_CS_;
						DacTxCtrl |= (DAC_ENDAL_ON|DAC_ENDAR_ON);  // turn on DAC_OUT_L , DAC_OUT_R 		
						_APP_EXIT_CS_;
						DEBUG_UI("\nDAC Tx On\n");
					}
				}
		#endif //#if (USE_EXT_DAC_CTRL)
		}

	
#endif
    return 1;
}

s32 uiMenuSet_TX_PIR(s8 setting)
{
    DEBUG_UI("uiMenuSet_TX_PIR %d \r\n",setting);

    if (iconflag[UI_MENU_SETIDX_PIR] != setting)
    {
        iconflag[UI_MENU_SETIDX_PIR] = setting;
        //Save_UI_Setting();
         sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
    }
    return 1;
}

s32 uiMenuSet_TX_BRIGHTNESS(s8 setting)
{
    DEBUG_UI("uiMenuSet_Tx_Brightness %d \r\n",setting);

    if((setting > 8) || (setting < 0))
    {
        DEBUG_UI("uiMenuSet_Tx_Brightness(%d) fail!!!\n", setting);
        return 0;
    }

    siuSetExposureValue(setting);  //Lucian: 目前有Bug,暫時拿掉.
    if (iconflag[UI_MENU_SETIDX_TX_BRIGHT] != setting)
    {
        iconflag[UI_MENU_SETIDX_TX_BRIGHT] = setting;
        //Save_UI_Setting();
        sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
    }
}

#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) ||(SW_APPLICATION_OPTION == MR8120_RFCAM_TX2))

s32 uiMenuSet_TX_FLICER(s8 setting)
{
    DEBUG_UI("uiMenuSet_TX_FLICER %d \r\n",setting);
    AE_Flicker_50_60_sel=setting;
    if(iconflag[UI_MENU_SETIDX_TX_FLICKER] != setting)
    {
        iconflag[UI_MENU_SETIDX_TX_FLICKER] = setting;
        //Save_UI_Setting();
         sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
    }
}
#endif

void uiMenuSet_UpgradeFW(void)
{
    s32 isp_return;
    if(gInsertCard == 0)
    {
        DEBUG_UI("uiMenuSet_UpgradeFW No Sd Card\n");
        return;
    }
    DEBUG_UI("uiMenuSet_UpgradeFW \n");
    sysSD_Enable();
    /*disable watch dog when update firmware*/
    sysDeadLockMonitor_OFF();
    isp_return=ispUpdateAllload();//usb boot
    if(isp_return ==0)
    {
        ispUpdatebootload();
        isp_return=ispUpdate(1);		 // Check whether spiFW.bin exists ot not. If exist then update
    }
    /*enable watch dog when update firmware finish*/
    OSTimeDly(1);
    sysDeadLockMonitor_ON();
}
s32 uiMenuSet_TX_MOTION(s8 setting,int Day_Level,int Night_Level)
{
    u8 level=SIU_DAY_MODE;

    DEBUG_UI("uiMenuSet_Tx_Motion: %d,%d,%d \r\n",setting,Day_Level,Night_Level);



   #if((HW_BOARD_OPTION  == MR8120_TX_RDI) || ( (HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT != 4) )||\
       (HW_BOARD_OPTION == MR8120_TX_RDI_CA530)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA672)||\
       (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) ||\
       (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
    if (gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level) == 0)
        level = SIU_DAY_MODE;
   #endif

    #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 5))
        level = SIU_DAY_MODE;
    #endif
    
    if(level == SIU_DAY_MODE)
    {
    #if HW_MD_SUPPORT
      mduMotionDetect_Sensitivity_Config(Day_Level);
    #endif
      switch (setting & 0x01)
      {
        case UI_MENU_TX_MOTION_ON:
            #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 5))
            setting |= 0x02;
            #endif
            #if HW_MD_SUPPORT
                mduMotionDetect_ONOFF(1);
            #endif
            break;

        case UI_MENU_TX_MOTION_OFF:
            #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 5))
            setting &= ~0x02;
            #endif
            #if HW_MD_SUPPORT
                mduMotionDetect_ONOFF(0);
            #endif
            break;

        default:
            DEBUG_UI("uiMenuSet_Tx_Motion(%d) fail!!!\n", setting);
            return 0;
      }
    }
    else
    {
        #if HW_MD_SUPPORT
          mduMotionDetect_Sensitivity_Config(Night_Level);
        #endif
        if (setting & 0x02)
        {
            #if HW_MD_SUPPORT
                mduMotionDetect_ONOFF(1);
            #endif
        }
        else
        {
            #if HW_MD_SUPPORT
                mduMotionDetect_ONOFF(0);
            #endif
        }
    }

    if( (iconflag[UI_MENU_SETIDX_TX_MOTION] != setting) || (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] != Day_Level) || (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT] != Night_Level) )
    {
        iconflag[UI_MENU_SETIDX_TX_MOTION] = setting;
        iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] = Day_Level;
        iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT] = Night_Level;
        //Save_UI_Setting();
         sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
    }
}

#if(TUTK_SUPPORT)
s32 uiMenuSet_P2PID(u8 *p2pID)
{
    DEBUG_UI("uiMenuSet_P2PID %s \r\n",p2pID);
    spiWriteNet();
    return 1;
}
#endif

#if(RFIU_SUPPORT)
s32 uiMenuSet_RFID(u32 *rfID)
{
    u8 i;
    DEBUG_UI("uiMenuSet_RFID: \n");
    for(i=0;i<RFID_MAX_WORD;i++)
        DEBUG_UI("%x \n",rfID[i]);
    spiWriteRF();
    return 1;
}
#endif

#if(NIC_SUPPORT)
s32 uiMenuSet_MACAddr(u8 *macAddr)
{
    DEBUG_UI("uiMenuSet_MACAddr: %2x-%2x-%2x-%2x-%2x-%2x\n",macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
    spiWriteNet();
    return 1;
}

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
void uiInitDAC_Play(void)
{
	u32	unAudioVol;

	unAudioVol = (u32) iconflag[UI_MENU_SETIDX_VOLUME];

	adcInitDAC_Play(unAudioVol);

}

s32 uiMenuSet_TX_CameraOnOff(s8 setting, u8 nCamCount)
{
    static u8 Cur_val=0;
    u8 setcmdstr[20];
    u8 val=0, i;

    DEBUG_UI("cam %d uiMenuSet_TX_CameraOnOff %d\r\n",nCamCount,setting);

    //if (start_iconflag[UI_MENU_SETIDX_CH1_ON+nCamCount] == setting)
        //return 1;

    for (i = 0; i < 4; i++)
    {
        if (iconflag[UI_MENU_SETIDX_CH1_ON+i] == UI_MENU_SETTING_CAMERA_ON)
            val |= (1<<i);
        else
            val &= ~(1<<i);
    }

    if (Main_Init_Ready == 0)
    {
        rfiuRX_CamOnOff_Sta = val;
        Cur_val = val;
        return 1;
    }
    else
    {
        if (Cur_val != val)
        {
            Cur_val = val;
            sprintf((char *)setcmdstr,"%d", val);
            rfiuCamOnOffCmd(setcmdstr);
        }
    }
    return 1;

}

/* Setting Microphone gain */
s32 uiMenuSet_MIC_Audio_Vol(s8 setting)

{
    DEBUG_UI("UI_MENU_SETIDX_MIC_VOLUME %d \n",setting);
    adcSetADC_MICIN_PGA_Gain(setting);

    return 1;
}




#if (HW_BOARD_OPTION  == MR8211_ZINWELL)

/*

Routine Description:

	Set IR mode.

Arguments:

	setting - AVIOCTRL_NIGHT_ON, AVIOCTRL_NIGHT_OFF, AVIOCTRL_NIGHT_AUTO.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_IR_Mode(u8 setting)
{
    DEBUG_UI("uiMenuSet_IR_Mode %d \r\n",setting);
	IR_Mode = setting;
	return 1;
}

/*

Routine Description:

	Set Light Status.

Arguments:

	setting - 0x01: OFF, 0x02: Random, 0x03: value assigned.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_Light_Status(u8 setting)
{
    DEBUG_UI("uiMenuSet_Light_Status(%d)\n", setting);
	Light_Status    = setting;
    switch(setting)
    {
        case 0x01:  // OFF
            i2cWrite_Byte(0x26, 0x00, 0x00);    // LED state (2:Random ON 1:LED On, 0: LED Off)
            break;
        case 0x02:  // Random
            i2cWrite_Byte(0x26, 0x00, 0x02);    // LED state (2:Random ON 1:LED On, 0: LED Off)
            break;
        case 0x03:  // Value assigned
            i2cWrite_Byte(0x26, 0x00, 0x01);    // LED state (2:Random ON 1:LED On, 0: LED Off)
            break;
        default:
            DEBUG_SYS("Error: uiMenuSet_Light_Status(0x%08x) status can't supprot\n", Light_Status);
	        return 0;
    }
	return 1;
}

/*

Routine Description:

	Set Red Light Value.

Arguments:

	setting - Setting.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_Light_R(u8 setting)
{
    DEBUG_UI("uiMenuSet_Light_R(%d)\n", setting);
	Light_R = setting;
    i2cWrite_Byte(0x26, 0x01, Light_R); // LED R duty (256 step)
	return 1;
}

/*

Routine Description:

	Set Green Light Value.

Arguments:

	setting - Setting.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_Light_G(u8 setting)
{
    DEBUG_UI("uiMenuSet_Light_G(%d)\n", setting);
	Light_G = setting;
    i2cWrite_Byte(0x26, 0x02, Light_G); // LED G duty (256 step)
	return 1;
}

/*

Routine Description:

	Set Blue Light Value.

Arguments:

	setting - Setting.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_Light_B(u8 setting)
{
    DEBUG_UI("uiMenuSet_Light_B(%d)\n", setting);
	Light_B = setting;
    i2cWrite_Byte(0x26, 0x03, Light_B); // LED B duty (256 step)
	return 1;
}

/*

Routine Description:

	Set Record Mode.

Arguments:

	setting - ENUM_RECORD_TYPE.
               	AVIOTC_RECORDTYPE_OFF				= 0x00,
               	AVIOTC_RECORDTYPE_FULLTIME			= 0x01,
               	AVIOTC_RECORDTYPE_ALARM				= 0x02,
               	AVIOTC_RECORDTYPE_MANUAL			= 0x03,
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_RecordMode(u8 setting)
{
    DEBUG_UI("uiMenuSet_RecordMode(%d)\n", setting);
	RecordMode  = setting;
	return 1;
}

/*

Routine Description:

	Set Mount Mode.

Arguments:

	setting -   AVIOCTRL_SDCARD_MOUNT
	            AVIOCTRL_SDCARD_UNMOUNT

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_MountMode(u8 setting)
{
    DEBUG_UI("uiMenuSet_RecordMode(%d)\n", setting);
	MountMode   = setting;
	return 1;
}

/*

Routine Description:

	Set Noise Alert Mode.

Arguments:

	setting -   0x01: ON,  0x02:OFF 

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_NoiseAlert(u8 setting)
{
    DEBUG_UI("uiMenuSet_NoiseAlert(%d)\n", setting);
	NoiseAlert  = setting;
	return 1;
}

/*

Routine Description:

	Set Temp Alert Mode.

Arguments:

	setting -   0x01: ON,  0x02:OFF 

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_TempAlert(u8 setting)
{
    DEBUG_UI("uiMenuSet_TempAlert(%d)\n", setting);
	TempAlert   = setting;
	return 1;
}

/*

Routine Description:

	Set Temp High Margin.

Arguments:

	setting -   Setting.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_TempHighMargin(void)
{
    DEBUG_UI("uiMenuSet_TempHighMargin()\n");
	//TempHighMargin  = setting;
	return 1;
}

/*

Routine Description:

	Set Temp Low Margin.

Arguments:

	setting -   Setting.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_TempLowMargin(void)
{
    DEBUG_UI("uiMenuSet_TempLowMargin()\n");
	//TempLowMargin   = setting;
	return 1;
}

/*

Routine Description:

	Set Mount Mode.

Arguments:

	setting -   AVIOCTRL_SDCARD_MOUNT
	            AVIOCTRL_SDCARD_UNMOUNT

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 uiMenuSet_Frequency(u8 setting)
{
    DEBUG_UI("uiMenuSet_Frequency(%d)\n", setting);
	AE_Flicker_50_60_sel    = setting;
    sysTVinFormat           = setting;
    TvOutMode               = setting;
	return 1;
}
#elif (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
s32 uiMenuSet_Light_Duration(u8 setting)
{
    DEBUG_UI("uiMenuSet_Light_Duration %d\n", setting);

    iconflag[UI_MENU_SETIDX_LIGHT_DURATION] = setting;
    switch (setting)
    {
        case UI_MENU_SETTING_LS_DUR_OFF:
            if ((uiInScheduleLight == 0) && (uiInManualLight == 0))
            {
                gpioSetLevel(GPIO_GROUP_W_LED, GPIO_BIT_W_LED, 0);
                rfiu_SetRXLightTrig(0);
                uiSetTriggerDimmer = 0;
            }
            uiSetLightDuration = 0;
            break;

        case UI_MENU_SETTING_LS_DUR_1MIN:
            uiSetLightDuration = 60;
            break;

        case UI_MENU_SETTING_LS_DUR_3MIN:
            uiSetLightDuration = 180;
            break;

        case UI_MENU_SETTING_LS_DUR_10MIN:
            uiSetLightDuration = 600;
            break;
    }
    if (Main_Init_Ready == 1)
         sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
        //Save_UI_Setting();
}

s32 uiMenuSet_Light_Dimmer(u8 setting)
{
    DEBUG_UI("uiMenuSet_Light_Dimmer %d\n", setting);

    if (uiSetUiSetting == 0)
    {
        if (iconflag[UI_MENU_SETIDX_LIGHT_DIMMER] == setting)
            return;
        else
            iconflag[UI_MENU_SETIDX_LIGHT_DIMMER] = setting;
    }
    else
        uiSetUiSetting = 0;
    gpioSetLightDimmer(setting);
    if (Main_Init_Ready == 1)
        Save_UI_Setting();
    if (uiSetTriggerDimmer != 0)
        uiSetTriggerDimmer = UI_TRIGGER_DIMMER_TIME;
}

s32 uiMenuSet_Light_Timer(u8 setting)
{
    u8  i;
    u8  CrossNight = 0;
    DEBUG_UI("uiMenuSet_Light_Timer %d %02d:%02d ~ %02d:%02d\n", setting, uiLightTimer[0], uiLightTimer[1], uiLightTimer[2], uiLightTimer[3]);

    if ((uiLightTimer[0] > uiLightTimer[2]) ||
        ((uiLightTimer[0] == uiLightTimer[2]) && (uiLightTimer[1] > uiLightTimer[3])))
    {
        CrossNight = 1;
        uiLightTimer1[0] = uiLightTimer[0];
        uiLightTimer1[1] = uiLightTimer[1];
        uiLightTimer1[2] = 24;
        uiLightTimer1[3] = 0;
        uiLightTimer2[0] = 0;
        uiLightTimer2[1] = 0;
        uiLightTimer2[2] = uiLightTimer[2];
        uiLightTimer2[3] = uiLightTimer[3];
    }
    else
    {
        CrossNight = 0;
        memcpy(uiLightTimer1, uiLightTimer, sizeof(uiLightTimer));
        memset(uiLightTimer2, 0, sizeof (uiLightTimer2));
    }
    memset(uiLightWeek1, 0, sizeof (uiLightWeek1));
    memset(uiLightWeek2, 0, sizeof (uiLightWeek2));
    
    for ( i = 0; i < 7; i++)
    {
        if (setting & (0x01 << i))
        {
            uiLightWeek1[i] = 1;
            if (CrossNight == 1)
            {
                if (i == 6)
                    uiLightWeek2[0] = 1;
                else
                    uiLightWeek2[i+1] = 1;
            }
        }
    }  
    if (uiInScheduleLight == 2)
        uiInScheduleLight = 0;
}

#endif
#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
s32 uiMenuSet_Night_Light(s8 setting)
{
    u8 sendCmd;

	#if (ICOMMWIFI_SUPPORT == 0)
    DEBUG_UI("uiMenuSet_Night_Light %d \r\n",setting);
    #endif
    switch (setting)
    {
        case UI_MENU_SETIDX_LIGHT_OFF:
            sendCmd = 0x10;
            break;

        case UI_MENU_SETIDX_LIGHT_ON:
            sendCmd = 0x11;
            break;

        default:
            break;
    }

	sendchar(RDI_SEP_MCU_UART_ID, &sendCmd);
	return 1;
}

void uiSet_Light_Cnt(u8 count, u8 interval)
{
	u8 i;

	for(i=0;i<count;i++)
	{
		uiMenuSet_Night_Light(UI_MENU_SETIDX_LIGHT_ON);
		OSTimeDly(interval);
		uiMenuSet_Night_Light(UI_MENU_SETIDX_LIGHT_OFF);
		OSTimeDly(interval);
	}
}
s32 uiMenuSet_Melody(u8 setting)
{
    u8 sendCmd;

    DEBUG_UI("uiMenuSet_Melody %d \r\n",setting);
    gpioMusicCtr(setting);
    return 1;
}
#endif

s32 uiMenuAction(s8 setidx)
{
    switch (setidx)
    {
        case UI_MENU_SETIDX_VIDEO_QUALITY:
            uiMenuSet_VideoQuality(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_VIDEO_FRAMERATE:
            uiMenuSet_VideoFrameRate(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_VIDEO_SIZE:
            uiMenuSet_RXCameraResolution(iconflag[setidx],0);
            break;

        case UI_MENU_SETIDX_TX_RES:
            uiMenuSet_VideoResolution(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_VIDEO_SIZE_CH2:
        case UI_MENU_SETIDX_VIDEO_SIZE_CH3:
        case UI_MENU_SETIDX_VIDEO_SIZE_CH4:
            uiMenuSet_RXCameraResolution(iconflag[setidx],(setidx-UI_MENU_SETIDX_VIDEO_SIZE));
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

        case UI_MENU_SETIDX_MOTION_SENSITIVITY:
            uiMenuSet_MotionSensitivity(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_CHANNEL:
            uiMenuSet_Channel(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_REC_MODE:
        #if (HW_BOARD_OPTION==MR6730_WINEYE)
            uiMenuSet_REC_MODE(Current_REC_Mode);
        #else
            uiMenuSet_REC_MODE(iconflag[setidx]);
        #endif
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

        case UI_MENU_SETIDX_AUDRES:
            uiMenuSet_AudRes(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_SAMPLING:
            uiMenuSet_Sampling(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_VOLUME:
            uiMenuSet_Audio_Vol(iconflag[setidx]);
            break;

#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
        case UI_MENU_SETIDX_MELODY_VOL:
            uiMenuSetMelodyVolCtrl(iconflag[setidx]);
            break;
#endif            
    #if(TUTK_SUPPORT)
        case UI_MENU_SETIDX_P2PID:
            uiMenuSet_P2PID(uiP2PID);
            break;
    #endif

    #if(RFIU_SUPPORT)
        case UI_MENU_SETIDX_RFID:
            uiMenuSet_RFID(uiRFID);
            break;
    #endif

    #if(NIC_SUPPORT)
        case UI_MENU_SETIDX_MAC:
            uiMenuSet_MACAddr(uiMACAddr);
            break;
        case UI_MENU_SETIDX_NETWORK:
            uiMenuSet_Network();
            break;
        case UI_MENU_SETIDX_P2P_PASSWORD:
            uiMenuSet_P2P_Password();
            break;
    #endif

        case UI_MENU_SETIDX_TX_BRIGHT:
            uiMenuSet_TX_BRIGHTNESS(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_TX_MOTION:
            uiMenuSet_TX_MOTION(iconflag[setidx],iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY],iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
            break;
        case UI_MENU_SETIDX_MIC_VOLUME:
            uiMenuSet_MIC_Audio_Vol(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_CH1_ON:
        case UI_MENU_SETIDX_CH2_ON:
        case UI_MENU_SETIDX_CH3_ON:
        case UI_MENU_SETIDX_CH4_ON:
            uiMenuSet_TX_CameraOnOff(iconflag[setidx],(setidx-UI_MENU_SETIDX_CH1_ON));
            break;
        case UI_MENU_SETIDX_TV_OUT:
    #if ((HW_BOARD_OPTION == MR8120_TX_RDI)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA671)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA672)||\
        (HW_BOARD_OPTION == MR8120_TX_RDI_CA532)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
            uiMenuSet_TVout_Format(defaultvalue[setidx]);
    #else
            uiMenuSet_TVout_Format(iconflag[setidx]);
    #endif
            break;

        case UI_MENU_SETIDX_UPGRADE_FW:
            uiMenuSet_UpgradeFW();
            break;

        case UI_MENU_SETIDX_PIR:
            uiMenuSet_TX_PIR(iconflag[setidx]);
            break;
#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) ||(SW_APPLICATION_OPTION == MR8120_RFCAM_TX2))            
        case UI_MENU_SETIDX_TX_FLICKER:
            uiMenuSet_TX_FLICER(iconflag[setidx]); /* 20151119 modified.  All TX will be control by RX(which has 50&60Hz Option)*/
            break;
#endif

#if (HW_BOARD_OPTION  == MR8211_ZINWELL)
        case UI_MENU_SETIDX_IR_MODE:
            uiMenuSet_IR_Mode(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_LIGHT_STATUS:
            uiMenuSet_Light_Status(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_LIGHT_R:
            uiMenuSet_Light_R(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_LIGHT_G:
            uiMenuSet_Light_G(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_LIGHT_B:
            uiMenuSet_Light_B(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_LIGHT_L:
            break;

        case UI_MENU_SETIDX_RECORD_MODE:
            uiMenuSet_RecordMode(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_MOUNT_MODE:
            uiMenuSet_MountMode(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_NOISE_ALERT:
            uiMenuSet_NoiseAlert(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_TEMP_ALERT:
            uiMenuSet_TempAlert(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_TEMP_HIGH_MARGIN:
            uiMenuSet_TempHighMargin();
            break;

        case UI_MENU_SETIDX_TEMP_LOW_MARGIN:
            uiMenuSet_TempLowMargin();
            break;

        case UI_MENU_SETIDX_FLICKER_FREQUENCY:
            uiMenuSet_Frequency(iconflag[setidx]);
            break;
#elif (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
        case UI_MENU_SETIDX_LIGHT_DURATION:
            uiMenuSet_Light_Duration(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_LIGHT_DIMMER:
            uiSetUiSetting = 1;
            uiMenuSet_Light_Dimmer(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_LIGHT_TIMER:
            uiMenuSet_Light_Timer(iconflag[setidx]);
            break;
#endif
#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
        case UI_MENU_SETIDX_NIGHT_LIGHT:
            uiMenuSet_Night_Light(iconflag[setidx]);
            break;

        case UI_MENU_SETIDX_MELODY:
            uiMenuSet_Melody(iconflag[setidx]);
            break;
#endif

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

