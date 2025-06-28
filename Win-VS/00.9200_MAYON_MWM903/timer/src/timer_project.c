/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	timer.c

Abstract:

   	The routines of timer.

Environment:

    	ARM RealView Developer Suite

Revision History:

	2005/08/26	David Tsai	Create

*/

/*CY 0718*/

#include "general.h"
#include "board.h"
#include "sysapi.h"

#include "timerapi.h"
#include "timer.h"
//#include "timerreg.h"
#include "task.h"
#include "../sys/inc/sys.h"

#include "siuapi.h"
#include "gpioapi.h"
#include "../board/inc/intreg.h"
#include "rtcapi.h"
#include "adcapi.h"
#include "memorypool.h"
#include "uiapi.h"
#include "asfapi.h"
#include "i2capi.h"
#include "..\..\ui\inc\ui.h"
#include "Rfiuapi.h"

#if defined(NEW_UI_ARCHITECTURE)
    #include "..\..\ui\inc\MainFlow.h"
#endif
#if CDVR_SYSTEM_LOG_SUPPORT
#include "..\..\ui\inc\LiveViewFlow.h"
#endif
/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 * Global Variable
 *********************************************************************************************************
 */

u8 unstable_idu_frame_cnt=0;
u8 fixed_ISU_IDU_ASYNC=0;
s32 old_Timer_isu_idufrmcnt;
u32	gbeep_on_time_counter;
u32	gbeep_off_time_counter;
u32	gbeep_delay_time_counter;
u16	gbeep_clock_div;
u8 PWM_is_On=0;
u8 PWM_alarm_counter=0;
u8 PWM_alarm_ready=1;
u8 gBeep_pause_width=0x1;
OS_FLAG_GRP *OSPWM_task_flags=0;
#if(SUPPORT_TOUCH)
bool touch_press=FALSE;
#endif
#if DOOR_BELL_SUPPORT
OS_FLAG_GRP *DoorBellFlagGrp = 0;
#endif
#if OS_TASK_STAT_EN
OS_EVENT  *gCpuLoadingSemEvt;
#endif


#if CDVR_SYSTEM_LOG_SUPPORT
int gTXtriggerEvent[4]={0,0,0,0};
s16 TriggerCnt[4]={-1,-1,-1,-1};
#endif

#if PWM_BEEP_TASK_SUPPORT
OS_STK			pwmBeepTaskStack[PWM_BEEP_TASK_STACK_SIZE];
OS_EVENT		*gPwmBeepSem;
#endif

#if (PassiveIR_SensControl == PassiveIR_SS004 || PassiveIR_SensControl == PassiveIR_SS0041P)
enum PIR_SENSITIVE_STATE
{
    PIR_INIT,
    PIR_START,
    PIR_SET_LEVEL,
    PIR_END,
    PIR_DONE
};
#endif
/*
 *********************************************************************************************************
 * Extern Global Variable
 *********************************************************************************************************
 */
extern volatile s32 isu_idufrmcnt;
extern u8 siuOpMode;
extern u8 Main_Init_Ready;
extern u32 sysVideoInSel;
extern u8 siuAeReadyToCapImage;
extern u8  isuGenerateScalarOverlayImage;

extern u8 Time_AudioPlayOffset;

#if (PassiveIR_SensControl == PassiveIR_SS004 || PassiveIR_SensControl == PassiveIR_SS0041P)
extern int PIR_SensitivityConfTab[MD_SENSITIVITY_LEVEL];
#endif

#if Auto_Video_Test
    extern u8 playbackflag;
    extern u8 uiMenuEnable;
    extern OS_EVENT* general_MboxEvt;
    extern u8 Iframe_flag; // Decided get I-frame or play whole file
#endif
    extern u8  IcrOnFlag;
    extern u8  IcrOffFlag;


#if IS_COMMAX_DOORPHONE && (TV_DECODER == TW9910)
u8 i2c_lock;
u8 i2c_check=0;
#endif
#if(TV_DECODER ==WT8861)
extern u8 bVideoResetTimer;
u8 ii=0;
extern u8 cam_check;
extern u8 i2c_mode;
#endif

#if (RFIU_RX_WAKEUP_TX_SCHEME && TUTK_SUPPORT)
extern u8 gAppWakeCamflag;
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];
#endif
/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */
#if ((HW_BOARD_OPTION == MR8202A_RX_TARNWO_D8530)||(HW_BOARD_OPTION == MR8202A_RX_TARNWO_D8730))
#define GPIO_PIN_BEEP              25
#define UI_BEEP_ON	{OS_ENTER_CRITICAL();PWM5_CTLREG=0;PWM5_CVREG=gbeep_on_time_counter;PWM5_PAUSE_INTEN=2;PWM5_CTLREG=0x17000000|gbeep_clock_div|((u32)gBeep_pause_width<<16);Gpio0Ena&=(~(1<<GPIO_PIN_BEEP));OS_EXIT_CRITICAL();}
#define UI_BEEP_OFF	{OS_ENTER_CRITICAL();PWM5_CTLREG=0;PWM5_CVREG=gbeep_off_time_counter;PWM5_PAUSE_INTEN=2;PWM5_CTLREG=0x15000000|gbeep_clock_div|((u32)gBeep_pause_width<<16);Gpio0Ena|=((1<<GPIO_PIN_BEEP));Gpio0Level&=(~(1<<GPIO_PIN_BEEP));OS_EXIT_CRITICAL();}
#define UI_BEEP_LONG_OFF	{OS_ENTER_CRITICAL();PWM5_CTLREG=0;PWM5_CVREG=gbeep_delay_time_counter;PWM5_PAUSE_INTEN=2;PWM5_CTLREG=0x15800000|gbeep_clock_div;Gpio0Ena|=((1<<GPIO_PIN_BEEP));Gpio0Level&=(~(1<<GPIO_PIN_BEEP));OS_EXIT_CRITICAL();}
#define UI_BEEP_STOP	{OS_ENTER_CRITICAL();PWM5_CTLREG=0;PWM5_PAUSE_INTEN=0;PWM5_CTLREG=0;Gpio0Ena|=((1<<GPIO_PIN_BEEP));Gpio0Level&=(~(1<<GPIO_PIN_BEEP));OS_EXIT_CRITICAL();}
#else
#define UI_BEEP_ON	{OS_ENTER_CRITICAL();Timer3Ctrl=0;Timer3Count=gbeep_on_time_counter;Timer3IntEna=2;Timer3Ctrl=0x17000000|gbeep_clock_div|((u32)gBeep_pause_width<<16);Gpio0Ena&=(~(1<<1));OS_EXIT_CRITICAL();}
#define UI_BEEP_OFF	{OS_ENTER_CRITICAL();Timer3Ctrl=0;Timer3Count=gbeep_off_time_counter;Timer3IntEna=2;Timer3Ctrl=0x15000000|gbeep_clock_div|((u32)gBeep_pause_width<<16);Gpio0Ena|=((1<<1));Gpio0Level&=(~(1<<1));OS_EXIT_CRITICAL();}
#define UI_BEEP_LONG_OFF	{OS_ENTER_CRITICAL();Timer3Ctrl=0;Timer3Count=gbeep_delay_time_counter;Timer3IntEna=2;Timer3Ctrl=0x15800000|gbeep_clock_div;Gpio0Ena|=((1<<1));Gpio0Level&=(~(1<<1));OS_EXIT_CRITICAL();}
#define UI_BEEP_STOP	{OS_ENTER_CRITICAL();Timer3Ctrl=0;Timer3IntEna=0;Timer3Ctrl=0;Gpio0Ena|=((1<<1));Gpio0Level&=(~(1<<1));OS_EXIT_CRITICAL();}
#endif
 /*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
void PWM_ISR_handler(void);
void gpioSensorCtrl100ms(void);
void gpioICRCtrl(void);
void DoorBellPolling(void);
#if (PassiveIR_SensControl == PassiveIR_SS004 || PassiveIR_SensControl == PassiveIR_SS0041P)
void gpioPIRSensCtrl(int level);
#endif

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3) || (UI_VERSION == UI_VERSION_TRANWO) )
 extern void gpioKeyPolling(void);
#endif

#if (USE_BUILD_IN_RTC == RTC_USE_TIMER_RTC)
u32 RTC_Time_To_Second(RTC_DATE_TIME *Source);
#endif

#if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
     (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD)  || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
u32 ciuCheckFrameRate(void);
#endif

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */
#if (USB_DONGLE_SUPPORT && ((PROJECT_SELECT == 2)||(PROJECT_SELECT == 3)))
	extern OS_FLAG_GRP *OSUSB_task_flag;
	extern u8  PcamPreviewStart;
#endif


void timer_KeyPolling(void)
{
    OS_tickcounter++;

#if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3) || (UI_VERSION == UI_VERSION_TRANWO) )
    gpioKeyPolling();
    adcKeyPolling();
#elif (HW_BOARD_OPTION == MR9200_RX_ROULE)
    gpioKeyPolling();
#else
   adcKeyPolling();
#endif
}

void timer_1s_IntHandler(void)
{

    RTC_DATE_TIME gmt;
	static u32	ucCount = 0;
#if (HW_IR_SUPPORT == 1)
    u8  irKey;
#endif
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

#if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
     (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    u32 fps;
#endif
#if (USE_BUILD_IN_RTC == RTC_USE_TIMER_RTC)
    u32 sec;
#endif

#if (CDVR_SYSTEM_LOG_SUPPORT || RFIU_RX_WAKEUP_TX_SCHEME)
	u8 i;
#endif

#if (RFIU_RX_WAKEUP_TX_SCHEME && TUTK_SUPPORT)
static u8 battery_cam_timer[MULTI_CHANNEL_MAX] = 0;
#endif

#if OS_TASK_STAT_EN
static u8 needPrint = 0;
#endif
//-------------------Time updating--------------------------------------------//

#if (USE_BUILD_IN_RTC == RTC_USE_BUILD_IN)
    if( (ucCount & 0x0fff) == 0)  //Lucian:每 4096 sec  讀取RTC
    {
        RTC_Get_GMT_Time(&gmt);
        RTCTime_Gmt_To_Local(&gmt, &g_LocalTime);
    }
    else
    {
        OS_ENTER_CRITICAL();
        g_LocalTimeInSec ++;
        OS_EXIT_CRITICAL();
        RTC_Second_To_Time(g_LocalTimeInSec, &g_LocalTime);   //Lucian: 未來可optimize: 不必每次都做sec --> time.

    }
#elif (USE_BUILD_IN_RTC == RTC_USE_TIMER_RTC)
    RTC_Get_GMT_Time(&gmt);
    sec = RTC_Time_To_Second(&gmt);
    sec++;
    RTC_Second_To_Time(sec, &gmt);
    RTC_Set_GMT_Time(&gmt);
#else
   	if( (ucCount & 0x1ff) == 0) //Lucian: 每 512 sec 讀一次RTC
	{
	    RTC_Get_GMT_Time(&gmt);
	    RTCTime_Gmt_To_Local(&gmt, &g_LocalTime);
	}
    else
    {
        OS_ENTER_CRITICAL();
        g_LocalTimeInSec ++;
        OS_EXIT_CRITICAL();
        RTC_Second_To_Time(g_LocalTimeInSec, &g_LocalTime);   //Lucian: 未來可optimize: 不必每次都做sec --> time.

    }
#endif
    ucCount ++;
#if OS_TASK_STAT_EN
    if(OSSemAccept(gCpuLoadingSemEvt) !=0)
        needPrint = (needPrint)?1:0;
        
    if((ucCount %5) && needPrint)
        OSTaskStatHook();
#endif

//Battery cam WakeUp Timer, Wakeup From APP
#if (RFIU_RX_WAKEUP_TX_SCHEME && TUTK_SUPPORT)
for(i=0; i<MULTI_CHANNEL_MAX; i++){
    if(gAppWakeCamflag & (0x01 << i)){
        battery_cam_timer[i] = 30;  //XXX: default: wake up 30 seconds, should set by UI settings
        gAppWakeCamflag &= (!(0x01 << i));
    }

    if(battery_cam_timer[i] > 0){
        battery_cam_timer[i]--;
        if(battery_cam_timer[i] == 0){
            if(gRfiuUnitCntl[i].WakeUpTxEn == 1) //Stop wakeup
            {
               gRfiuUnitCntl[i].WakeUpTxEn = 0; 
            }
            else
            {
               //rfiuCamSleepCmd(i);   
            }
        }
    }
}
#endif

//----Draw Overlay image on video----//
#if (ISU_OVERLAY_ENABLE)
    if(isuGenerateScalarOverlayImage)
        sysbackSetEvt(SYS_BACK_DRAWTIMEONVIDEOCLIP, 1);
#endif


//----------Show time on OSD when video clip----------//
    sysbackSetEvt(SYS_BACK_CHECK_UI_CONTROL, 0);
#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
    sysbackSetEvt(SYS_BACK_SCHEDULEMODE, 0);
#endif

#if (NET_STATUS_POLLING && NIC_SUPPORT)
//----------Check Network status ----------//
	sysbackSetEvt(SYS_BACK_DRAW_NET_ICON, 0);
#endif

#if(HOME_RF_SUPPORT)
//----------Count Home RF status ----------//
    sysbackSetEvt(SYS_BACK_CHECK_HOMERF, 0);
#endif

//------判斷TV-in format--------//
#if( (Sensor_OPTION == Sensor_CCIR601) ||(Sensor_OPTION == Sensor_CCIR656) )
    #if ( TVIN_FORMAT_DETECT_MODE == TV_IN_FORMAT_DETECT_ONCE )
        //Lucian: TV-in format detection 只做開機時,做一次 Detect.
        if((Main_Init_Ready == 1) &&(sysTVInFormatLocked == FALSE))
	         sysbackSetEvt(SYS_BACK_CHECK_TVIN_FORMAT,0);
    #else
        if ( Main_Init_Ready == 1 )
            sysbackSetEvt(SYS_BACK_CHECK_TVIN_FORMAT,0);
    #endif
#else
    if ( (Main_Init_Ready == 1) && (sysVideoInSel==VIDEO_IN_TV) )
            sysbackSetEvt(SYS_BACK_CHECK_TVIN_FORMAT,0);
#endif


   //-----------Battery detetct-------------//
#if((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) || (HW_BOARD_OPTION == MR9200_RX_RDI_M906))
    //move to timer_200ms_IntHandler
#else
    adcBatteryCheck();
#endif

    // Replace the RTC function
    if(sysPlaybackVideoPause == 0)
    {
        OS_ENTER_CRITICAL();
        RTCseconds++;
        OS_EXIT_CRITICAL();
    }
      //--------------For Testing---------------------//
#if (USB_DONGLE_SUPPORT && ((PROJECT_SELECT == 2)||(PROJECT_SELECT == 3)))
		if( PcamPreviewStart )
		{
			OSFlagPost (OSUSB_task_flag, 0x10,OS_FLAG_SET,0);
		}
#endif
#if Auto_Video_Test
        switch(Video_Auto.VideoTest_Mode)
        {
            case 1: /*capture*/

                if (Video_Auto.VideoTest_CurrFileElapse == 0)
                {
                    if ((Video_Auto.VideoTest_FileNum != Video_Auto.VideoTest_CurrFileNum) || Video_Auto.VideoTest_FileNum == 0)
                    {
                        uiKeyVideoCapture();
                        Video_Auto.VideoTest_CurrFileElapse++;
                    }

                }
                else if (Video_Auto.VideoTest_CurrFileElapse == Video_Auto.VideoTest_FileTime)
                {
                    sysCaptureVideoStop = 1;
                    sysCaptureVideoStart = 0;
                    Video_Auto.VideoTest_CurrFileElapse = 0;
                    Video_Auto.VideoTest_CurrFileNum++;
                    if (Video_Auto.VideoTest_CurrFileNum == Video_Auto.VideoTest_FileNum)
                    {
                        memset(&Video_Auto, 0, sizeof(Video_Auto));
                        Video_Auto.VideoTest_Mode = 2;
                    }
                }
                else
                    Video_Auto.VideoTest_CurrFileElapse++;

                break;

            case 2: /*playback Init*/
                playbackflag = 2;
                uiMenuEnable = 0x41;
                Iframe_flag=0;  // 1: We just need I-frame 0: play whole MP4
                general_MboxEvt->OSEventPtr=(void *)0;
                sysSetEvt(SYS_EVT_PLAYBACK_INIT, playbackflag);
                Video_Auto.VideoTest_Mode = 3;
                Video_Auto.VideoTest_CurrFileNum++;
                break;

            case 3: /*playback*/
                if(OSMboxAccept(general_MboxEvt) == 0)
                    break;
                if ((Video_Auto.VideoTest_FileNum != 0) && (sysPlaybackVideoStart == 0))
                {
                    if (Video_Auto.VideoTest_CurrFileNum == Video_Auto.VideoTest_FileNum)
                    {
                        memset(&Video_Auto, 0, sizeof(Video_Auto));
                        Video_Auto.VideoTest_Mode = 4;
                        break;
                    }
                    if (Video_Auto.VideoTest_CurrFileNum == 0)
                    {
                        playbackflag = 2;
                        uiMenuEnable = 0x41;
                    }
                    Iframe_flag=0;  // 1: We just need I-frame 0: play whole MP4
                    general_MboxEvt->OSEventPtr=(void *)0;
                    sysSetEvt(SYS_EVT_PLAYBACK_MOVE_FORWARD,0);
                    //sysSetEvt(SYS_EVT_ReadFile, playbackflag);
                    Video_Auto.VideoTest_CurrFileNum++;
                }
                break;

            case 4: /*delete*/
                general_MboxEvt->OSEventPtr=(void *)0;
                sysSetEvt(SYS_EVT_PLAYBACK_DELETE_ALL, 0);
                #if 0
                Video_Auto.VideoTest_Mode = 0;
                #else
                Video_Auto.VideoTest_Mode = 6;
                #endif
                break;

            case 5: /*stop*/
                if(sysCaptureVideoStart)
                {
                    sysCaptureVideoStop = 1;
                    sysCaptureVideoStart = 0;
                }
                else if (sysPlaybackVideoStart)
                    sysPlaybackVideoStop = 1;
                break;

            case 6: /*test again*/
                /*wait delete all*/
                if(OSMboxAccept(general_MboxEvt) != 0)
                {
                    DEBUG_TIMER("Enter to Preview\n");
                    /*enter preview*/
                    playbackflag    = 0;
                    uiMenuEnable    = 0;
                    siuOpMode       = SIUMODE_PREVIEW;
                    sysSetEvt(SYS_EVT_PREVIEW_INIT, playbackflag);
                    Video_Auto_Test(30, 120);
                }
                break;

            default:
                break;
        }
#endif  // Auto_Video_Test

#if (HW_IR_SUPPORT == 1)
    if ((ucCount & 0x3) == 0)
    {
        irKey = IR_INT_STATUS;
    }
#endif

#if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
     (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    fps=ciuCheckFrameRate();
    if((ucCount % 10)== 0)
    {
        DEBUG_TIMER("FPS:%d \n",fps);
    }
#endif

#if CDVR_SYSTEM_LOG_SUPPORT

	for(i=0;i<MAX_CHANNELS;i++)
	{
		if((gTXtriggerEvent[i] != 0) && (TriggerCnt[i] < 10))
		{
			gTXtriggerEvent[i] = 0; //Reset flag.

			if((TriggerCnt[i] == 0) || (TriggerCnt[i] == -1))
			{
				TriggerCnt[i] = asfEventExtendTime;
				sysback_Net_SetEvt(SYS_BACKRF_WRITE_LOG, SYSTEM_MOTION_ON, i);
			}
			else
			{
				if((TriggerCnt[i]+10) > asfEventExtendTime)
					TriggerCnt[i] += 10;
			}
		}
		if(TriggerCnt[i] > 0)
			TriggerCnt[i]--;

		if(TriggerCnt[i] == 0)	//Trigger End, Recode End Time.
		{
			sysback_Net_SetEvt(SYS_BACKRF_WRITE_LOG, SYSTEM_MOTION_OFF, i);
			TriggerCnt[i] = -1;
		}
	}
#endif

#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
                 usb_keep_on();
#endif


#if (DEVSTATUS_ACTIVE_UPDATE)
    {
        extern OS_FLAG_GRP  *gP2pDevStatusFlagGrp;
        u8 err;
        u32 waitFlag = OSFlagAccept(gP2pDevStatusFlagGrp, 0xFFFFFFFF, OS_FLAG_WAIT_SET_ANY , &err);

        sysback_Net_SetEvt(SYS_BACK_DEVSTATUS_UPDATE, SYS_BACK_DEVSTATUS_CHECK, waitFlag);
    }
#endif

}


void timer2_IntHandler(void)
{
    //used for fine-tune IIS audio clock.
#if (TIMER_TEST == 1)
    RTC_DATE_TIME   localTime;

    RTC_Get_GMT_Time(&localTime);
    DEBUG_TIMER("Timer 2 %d:%d:%d\n",localTime.hour,localTime.min,localTime.sec);
#endif
}

void timer3_IntHandler(void)
{
#if (TIMER_TEST == 1)
    RTC_DATE_TIME   localTime;

    RTC_Get_GMT_Time(&localTime);
    DEBUG_TIMER("Timer 3 %d:%d:%d\n",localTime.hour,localTime.min,localTime.sec);
#else
    //used for time delay of TV display
#endif  /*end of #if (TIMER_TEST == 1)*/
#if (! A1025_GATE_WAY_SERIES)
    PWM_ISR_handler();
#endif
}

void timer4_IntHandler(void)
{
#if (TIMER_TEST == 1)
    RTC_DATE_TIME   localTime;

    RTC_Get_GMT_Time(&localTime);
    DEBUG_TIMER("Timer 4 %d:%d:%d\n",localTime.hour,localTime.min,localTime.sec);
#else

//sunway used for time delay of TV display
#endif  /*end of #if (TIMER_TEST == 1)*/
}

void timerPWM5_IntHandler(void)
{
#if (A1025_GATE_WAY_SERIES)
   PWM_ISR_handler();
#endif
}
void timer_zeropointfivems_IntHandler(void)
{
    u8 err;
    u8 i[8]={1,0,0,1,1,0,0,1};
    static int count=0;
    OSFlagPost(gTimerIR_TXFlagGrp, FLAGTIME_READY_IR, OS_FLAG_CLR, &err);
    if(count <8)
    {
        gpioSetLevel(2, 12, i[count]);
    }
    else if(count == 8)
    {
       DEBUG_TIMER("Timer_IR_TX end %d\n",count);
       marsTimerCountPause(guiIR_TXTimerId, 1);
       marsTimerInterruptEnable(guiIR_TXTimerId, 0);       //Disable mars timer interrupt
    }else
    {
        OSFlagPost(gTimerIR_TXFlagGrp, FLAGTIME_READY_IR, OS_FLAG_SET, &err);
        count=0;
    }
    //DEBUG_TIMER("%d\r",count);
    count ++;
}
void timer1_IntHandler(void)
{
#if (TIMER_TEST == 1)
    RTC_DATE_TIME   localTime;

    RTC_Get_GMT_Time(&localTime);
    DEBUG_TIMER("Timer 1 %d:%d:%d\n",localTime.hour,localTime.min,localTime.sec);
#endif
}

void timer5_IntHandler(void)
{
#if (TIMER_TEST == 1)
    RTC_DATE_TIME   localTime;

    RTC_Get_GMT_Time(&localTime);
    DEBUG_TIMER("Timer 5 %d:%d:%d\n",localTime.hour,localTime.min,localTime.sec);
#endif
}

void timer6_IntHandler(void)
{
#if (TIMER_TEST == 1)
    RTC_DATE_TIME   localTime;

    RTC_Get_GMT_Time(&localTime);
    DEBUG_TIMER("Timer 6 %d:%d:%d\n",localTime.hour,localTime.min,localTime.sec);
#endif
}

void timer7_IntHandler(void)
{
#if (TIMER_TEST == 1)
    RTC_DATE_TIME   localTime;

    RTC_Get_GMT_Time(&localTime);
    DEBUG_TIMER("Timer 7 %d:%d:%d\n",localTime.hour,localTime.min,localTime.sec);
#endif
}

void timer_25ms_IntHandler(void)
{
#if defined(NEW_UI_ARCHITECTURE)
    UI_step25msTimer();
#else
   	//---Do Key Scan---//
    timer_KeyPolling();
#endif

#if (PassiveIR_SensControl == PassiveIR_SS0041P)
    gpioPIRSensCtrl(-1);
#endif
}

void timer_10ms_IntHandler(void)
{
}

void timer_50ms_IntHandler(void)
{
    //OSTimeTick();   // used for OS tick (50ms)

}

void timer_100ms_IntHandler(void)
{
#if (SUPPORT_TOUCH == 1)
        int TouchX, TouchY;

#endif
#if (IS_COMMAX_DOORPHONE && (TV_DECODER == TW9910))
    u8  data;
#endif
    WDT_Reset_Count();
    #if(SUPPORT_TOUCH_KEY)
    TouchKeyPolling();
    #endif

#if DOOR_BELL_SUPPORT
    DoorBellPolling();
#endif

	#if (JPEG_DEBUG_ENA_9300||JPEG_DEBUG_ENA_9200)
    sysbackSetEvt(SYS_BACK_SHOWTIMEONOSD_VIDEOCLIP, 0);
	#endif


#if (SUPPORT_TOUCH == 1)
    #if(HW_BOARD_OPTION !=  A1018B_EVB_216M)
        if (i2c_Touch_getPosition(&TouchX, &TouchY) == TRUE)
        {
            uiFlowCheckTouchKey(TouchX, TouchY);
            DEBUG_TIMER("Touched: x:%d, y:%d\n", TouchX, TouchY);
            touch_press=TRUE;  /*Roy: it should be set TRUE after uiFlowCheckTouchKey(TouchX, TouchY); */
        }
        else
        {
            #if (UI_VERSION == UI_VERSION_TRANWO)
            uiFlowStopTouchkeyAction();
            #endif
            touch_press=FALSE;
        }
    #endif
#endif

    #if (HW_BOARD_OPTION == MR8202A_RX_MAYON)
        gpioTimerCtrLed(0, LED_NONE);
        gpioTimerCtrLed(1, LED_NONE);
        gpioTimerCtrLed(2, LED_NONE);
        gpioTimerCtrLed(3, LED_NONE);
        gpioTimerCtrNetLed(LED_NONE);
        gpioTimerCtrSpeak(SPK_NONE);
    #elif (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018)
        gpioTimerCtrLed(0, LED_NONE);
        gpioTimerCtrLed(1, LED_NONE);
        gpioTimerCtrLed(2, LED_NONE);
        gpioTimerCtrLed(3, LED_NONE);
        gpioTimerCtrNetLed(LED_NONE);
        gpioKeyPolling();
    #elif (HW_BOARD_OPTION == A1025A_EVB_axviwe)
        gpioTimerCtrLed(0, LED_NONE);
        gpioTimerCtrLed(1, LED_NONE);
        gpioTimerCtrLed(2, LED_NONE);
        gpioTimerCtrLed(3, LED_NONE);
        gpioTimerCtrNetLed(LED_NONE);
        gpioSpeakSwitch(SPK_NONE);
    #elif((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3)||\
        (UI_VERSION == UI_VERSION_TRANWO) )
        gpioTimerCtrLed(LED_NONE);
    #elif ((HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T) || (HW_BOARD_OPTION == MR9120_TX_TRANWO_D87T) || (HW_BOARD_OPTION == MR9100_TX_JIT_C707HW4))
        gpioTimerCtrLed(LED_NONE);
        gpioKeyPolling();
    #elif ((HW_BOARD_OPTION == MR9120_TX_RDI_CA840) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811))
        gpioKeyPolling();
    #elif (HW_BOARD_OPTION == MR9100_AHDINREC_MUXCOM)
        gpioTimerCtrLed(LED_NONE);
    #endif
    #if ( (HW_BOARD_OPTION == MR9600_RX_RDI_AHD) || (HW_BOARD_OPTION == MR9600_RX_SKY_AHD) || (HW_BOARD_OPTION == MR9120_RX_DB_AHD) || (HW_BOARD_OPTION == MR9120_RX_MUXCOM_AHD) || (HW_BOARD_OPTION == MR9120_RX_DB_HDMI))
        gpioTimerCtrLed(LED_NONE);
        gpioKeyPolling();
    #endif

#if IS_COMMAX_DOORPHONE && (TV_DECODER == TW9910)

	if((Main_Init_Ready == 1) &&(sysTVInFormatLocked == TRUE)&&(i2c_check==0))
	{
		i2cRead_TW9910(0x1C, &data,I2C_TW9910_RD_SLAV_ADDR);
	    data &= 0x70;
	    if(data==0x00)//NTSC
	    {
			i2cWrite_TW9910(0x01,0x78,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x07,0x02,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x08,0x14,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x09,0xF0,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x0A,0x18,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x1C,0x07,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x1E,0x18,I2C_TW9910_WR_SLAV_ADDR);
	        DEBUG_I2C("NTSC \n\n");
	    }
	    else if(data==0x10)//PAL
	    {
			i2cWrite_TW9910(0x01,0x79,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x07,0x12,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x08,0x18,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x09,0x20,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x0A,0x0E,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x1C,0x17,I2C_TW9910_WR_SLAV_ADDR);
			i2cWrite_TW9910(0x1E,0x08,I2C_TW9910_WR_SLAV_ADDR);
	        DEBUG_I2C("PAL \n\n");
	    }
		if((sysTVInFormatLocked == TRUE)&&(i2c_lock==1))
			sysCiu_1_PreviewReset(0);   // for de-interlace

		i2c_check=1;

	}

#endif

#if ICR_CONTRAL_ENABLE
    gpioICRCtrl();
#endif

#if (PassiveIR_SensControl == PassiveIR_SS004)
    gpioPIRSensCtrl(-1);
#endif

}

void timer_200ms_IntHandler(void)
{

#if (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) ||\
    (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
  #if (AUDIO_DEVICE== AUDIO_IIS_WM8940)
    static u8 timeplay = 0;
    static u8 timerec = 0;
    if(Time_AudioPlayOffset > 0)
    {
        if(timeplay == 0)
        {
    	    i2cWrite_WM8940(0x2D, 0x0021);  //Input PGA volume
    	    i2cWrite_WM8940(0x36, DAC_PGA_GAIN);  //Input PGA volume
    	    timeplay = 1;
            timerec = 0;
	        //printf("play \n");
        }
        Time_AudioPlayOffset = 0;
    }
    else
    {
        if(timerec == 0)
        {
	        i2cWrite_WM8940(0x2D, 0x0031);  //Input PGA volume
    	        i2cWrite_WM8940(0x36, 0x0000);  //Input PGA volume
    	    timerec = 1;
            timeplay = 0;
	        //printf("rec \n");
        }
    }
  #endif
#endif

#if (TV_DECODER == WT8861)
    u8 Status,bVideoResetcont;
    u8 cnt;
	if(bVideoResetTimer &&(ii==0))	//20050630
	{
	//	bVideoResetcont=bVideoResetTimer;
	//	bVideoResetcont=bVideoResetcont-5;
	//	bVideoResetTimer=bVideoResetcont;
		VIDEO_SetupColourStandard();
	}
	i2cRead_WT8861(0x3A, &Status, I2C_WT8861_RD_SLAV_ADDR);
	if (((Status & 0x0F)== 0x0E) &&((i2c_mode==3)||(i2c_mode==4)||(i2c_mode==5)))
		cam_check=1;
    if (MyHandler.MenuMode != VIDEO_MODE)
        return;
#if 0
	i2cRead_WT8861(0x3A, &Status, I2C_WT8861_RD_SLAV_ADDR);
	if ((Status & 0x0F)== 0x0E) {
		if(sysTVinFormat==TV_IN_PAL)
			i2cWrite_WT8861(0xb2,0x50,I2C_WT8861_WR_SLAV_ADDR);
		else
			i2cWrite_WT8861(0xd6,0x00,I2C_WT8861_WR_SLAV_ADDR);
    }
	else
	{
		if(sysTVinFormat==TV_IN_PAL)
			i2cWrite_WT8861(0xb2,0xd0,I2C_WT8861_WR_SLAV_ADDR);
		else
			i2cWrite_WT8861(0xd6,0x83,I2C_WT8861_WR_SLAV_ADDR);
	}
#else
//	DEBUG_TIMER("%d",cam_check);
	if (cam_check==1)
	{
		i2cRead_WT8861(0x3A, &Status, I2C_WT8861_RD_SLAV_ADDR);
		while(((Status&0x0F) != 0x0E) && (cnt < 5))
	    {
	        cnt++;
	        i2cRead_WT8861(0x3A,&Status,I2C_WT8861_RD_SLAV_ADDR);
	    }
	//	DEBUG_TIMER("%d",cnt);
		if(cnt < 5)
		{
			ii=0;
			if(sysTVinFormat==TV_IN_PAL)
				i2cWrite_WT8861(0xb2,0x50,I2C_WT8861_WR_SLAV_ADDR);
			else
				i2cWrite_WT8861(0xd6,0x00,I2C_WT8861_WR_SLAV_ADDR);
		}else
		{
			ii=1;
	//		bVideoResetTimer=0;
			if(sysTVinFormat==TV_IN_PAL)
				i2cWrite_WT8861(0xb2,0xd0,I2C_WT8861_WR_SLAV_ADDR);
			else
				i2cWrite_WT8861(0xd6,0x83,I2C_WT8861_WR_SLAV_ADDR);
		}
	//	if ((Status & 0x0F)!= 0x0E)
	//		return;

	}
#endif
#endif

#if((HW_BOARD_OPTION == MR9200_RX_RDI_UDR777) || (HW_BOARD_OPTION == MR9200_RX_RDI_M906))
    adcBatteryCheck();
#endif
}

void timer_500ms_IntHandler(void)
{
  #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
       (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) ||\
       (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == Standalone_Test) ||\
       (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD)  || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
       (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) ||\
       (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)||\
       (SW_APPLICATION_OPTION == MR8202_GATEWAYBOX_RX) ||(SW_APPLICATION_OPTION ==MR8202_AN_KLF08W) )
       //TX 
  #else
    #if (!JPEG_DEBUG_ENA_9300 && !JPEG_DEBUG_ENA_9200)
    if(sysVoiceRecStart!=0)
        sysbackSetEvt(SYS_BACK_SHOWTIMEONOSD_VIDEOCLIP, 0);
	#endif
  #endif  
  
    #if defined(NEW_UI_ARCHITECTURE)
        UI_step500msTimer();
    #endif
}

s32 TimerProjectTimerInit(u8 Step)
{
    switch(Step)
    {
        case 1:
            marsTimerPwmOpen(2, timer2_IntHandler);
            marsTimerPwmOpen(3, timer3_IntHandler);
            marsTimerPwmOpen(4, timer4_IntHandler);
            marsTimerPwmOpen(10, timerPWM5_IntHandler);

            //當OS ticker. 100ms unit.
            marsTimerConfig(guiSysTimerId, &timerCfg_48M[0]);
            marsTimerCountWrite(guiSysTimerId, TIMER0_COUNT);
            marsTimerCountEnable(guiSysTimerId, 1);
            marsTimerInterruptEnable(guiSysTimerId, 1);

            #if TIMER_TEST
                marsTimerConfig(guiIRTimerId, &timerCfg_48M[1]);
                marsTimerCountWrite(guiIRTimerId, TIMER1_COUNT);
                marsTimerCountEnable(guiIRTimerId, 1);
                marsTimerInterruptEnable(guiIRTimerId, 1);       //Enable while using

                marsTimerConfig(Timer5Id, &timerCfg_48M[2]);
                marsTimerCountWrite(Timer5Id, TIMER5_COUNT);
                marsTimerCountEnable(Timer5Id, 1);
                marsTimerInterruptEnable(Timer5Id, 1);       //Enable while using

                marsTimerConfig(Timer6Id, &timerCfg_48M[3]);
                marsTimerCountWrite(Timer6Id, TIMER6_COUNT);
                marsTimerCountEnable(Timer6Id, 1);
                marsTimerInterruptEnable(Timer6Id, 1);       //Enable while using

                marsTimerConfig(Timer7Id, &timerCfg_48M[4]);
                marsTimerCountWrite(Timer7Id, TIMER7_COUNT);
                marsTimerCountEnable(Timer7Id, 1);
                marsTimerInterruptEnable(Timer7Id, 1);       //Enable while using
                timer2Setting();
                timer3Setting();
                timer4Setting();
            #else
                //Lucian: 當IR 及秒計時器. 10us unit
                marsTimerConfig(guiIRTimerId, &timerCfg_48M[1]);
                marsTimerCountWrite(guiIRTimerId, TIMER1_COUNT);
                marsTimerCountEnable(guiIRTimerId, 1);
                marsTimerInterruptEnable(guiIRTimerId, 0);       //Enable while using
                //Lucian: Used for RF , 100us unit.
                marsTimerConfig(guiRFTimerID, &timerCfg_48M[4]);
                marsTimerCountWrite(guiRFTimerID, TIMER7_COUNT);
                marsTimerCountEnable(guiRFTimerID, 1);
                marsTimerInterruptEnable(guiRFTimerID, 0);       //Enable while using
                #if MARSS_SUPPORT
                //Paul: Used for A7128 polling , 1ms unit.
                marsTimerConfig(guiMarssTimerId, &timerCfg_48M[3]);
                marsTimerCountWrite(guiMarssTimerId, TIMER6_COUNT);
                marsTimerCountEnable(guiMarssTimerId, 1);
                marsTimerInterruptEnable(guiMarssTimerId, 1);       //Enable while using
                #endif
            #endif
            //Lucina: 當time delay for TV out

            break;

        default:
            break;

    }
    return 0;
}

s32 TimerProjectWDTResetCount(u8 Step)
{
    switch (Step)
    {
        case 1: /*check reset WDT count or not*/

            if(sysForceWdt2Reboot)   // 1: Reboot by WDT, 0: otherwise
                return 1;   /*do not reset WDT count*/

            return 0;   /*reset WDT count*/

        default:
            break;

    }
    return 0;
}


//beep_on_time,u16 beep_off_time u16 beep_delay are in unit ms
void Beep_function(u8 beep_count,u8 beep_clock_div,u16 beep_on_time,u16 beep_off_time,u16 beep_delay,u8 flags)
{
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

	if (!OSPWM_task_flags)
	{
		OSPWM_task_flags=OSFlagCreate(PWM_ALARM_READY,0);
	}

	if (flags&PWM_BEEP_FORCE_START_FLAG)
	{
		OS_ENTER_CRITICAL();
		if ((!PWM_is_On)&&(PWM_alarm_counter==0))
			PWM_alarm_ready=1;
		#if ((HW_BOARD_OPTION == MR8202A_RX_TARNWO_D8530)||(HW_BOARD_OPTION == MR8202A_RX_TARNWO_D8730))
	       PWM5_PAUSE_INTEN=0;
		PWM5_CTLREG=0;
		Gpio0Ena|=((1<<GPIO_PIN_BEEP));
		#else
		Timer3IntEna=0;
		Timer3Ctrl=0;
		Gpio0Ena|=((1<<1));
		#endif
		OS_EXIT_CRITICAL();
	}
	if ((PWM_alarm_ready)&&(gBeep_pause_width != 0))/*If gBeep_pause_width is zero ,PWM no pluse*/
	{
		gbeep_clock_div=beep_clock_div;
		gbeep_on_time_counter=(beep_on_time*(SYS_CPU_CLK_FREQ/2000)>>8)/(gbeep_clock_div+1);
		gbeep_off_time_counter=(beep_off_time*(SYS_CPU_CLK_FREQ/2000)>>8)/(gbeep_clock_div+1);
		gbeep_delay_time_counter=(beep_delay*(SYS_CPU_CLK_FREQ/2000)>>8)/(gbeep_clock_div+1);
		//printf("\nON=%x,Off=%x,delay=%x",gbeep_on_time_counter,gbeep_off_time_counter,gbeep_delay_time_counter);

		PWM_alarm_counter=beep_count-1;
#if PWM_BEEP_TASK_SUPPORT
		OSSemPost(gPwmBeepSem);
#else
		PWM_is_On=1;
		PWM_alarm_ready=0;
        #if 0
		OSFlagAnd (OSPWM_task_flags,(~PWM_ALARM_READY) );
        #endif
		UI_BEEP_ON;

        #if 0
		if (flags&PWM_BEEP_WAIT_UNTIL_FINISH_FLAG)
		{
			timeout_val=(beep_on_time+beep_off_time)*beep_count+beep_delay+50;
			timeout_val=timeout_val*OS_TICKS_PER_MS;
			OSFlagPend (OSPWM_task_flags,PWM_ALARM_READY, OS_FLAG_WAIT_SET_ANY,timeout_val,0);
		}
        #endif
#endif
	}
}

void PWM_ISR_handler(void)
{
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    u8  err;
#if 0
    static  s32 Counter = 0;
  #if 0
    if(Counter == 0)
    {
        gpioSetLevel(0, 1, 1);  // 開始對濕度計充電
        Counter++;
    } else if(Counter == 1) {
        Humidity    = (AdcRecData_G3G4 >> 16) ^ 0x0800; // signed->unsigned
        gpioSetLevel(0, 1, 0);  // 停止對濕度計充電
        //Timer3Ctrl  = 0x00000000;
        Counter++;
    } else if(Counter == 14999) {
        Counter = 0;
    } else {
        Counter++;
    }
    //gpioSetLevel(0, 1, Counter++ & 1);
  #else
    Humidity        = (AdcRecData_G3G4 >> 16) ^ 0x0800; // signed->unsigned
    gpioSetLevel(0, 1, 0);  // 停止對濕度計充電
    Timer3Count     = 0x00000000;
    Timer3Ctrl      = 0x00000000;
    Counter++;
  #endif
#else
    if (PWM_is_On)
    {
    //  printf("\nOff");
        if (!PWM_alarm_counter)
        {
            UI_BEEP_LONG_OFF;
            OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PWM_BEEP, OS_FLAG_SET, &err);
        }
        else
        {
            UI_BEEP_OFF;
        }
        //UI_BEEP_OFF;
        PWM_is_On=0;
    }
    else
    {
        if (PWM_alarm_counter)
        {
    //      printf("\nOn");
            PWM_alarm_counter--;
            //gbeep_clock_div-=4;
            UI_BEEP_ON;
            PWM_is_On=1;
        }
        else
        {
    //      printf("\nstop");
#if PWM_BEEP_TASK_SUPPORT
			OSSemPost(gPwmBeepSem);
#else
            PWM_alarm_ready=1;
#endif
        #if 0
            OSFlagPostINT (OSPWM_task_flags, PWM_ALARM_READY);
        #endif
            UI_BEEP_STOP;
        }
    }
#endif
}

#if PWM_BEEP_TASK_SUPPORT
u8 pwmBeepTaskInit(void)
{
    gPwmBeepSem = OSSemCreate(0);
    OSTaskCreate(PWM_BEEP_TASK, PWM_BEEP_TASK_PARAMETER, PWM_BEEP_TASK_STACK, PWM_BEEP_TASK_PRIORITY);
	return 0;
}

void pwmBeepTask(void* pData)
{
    u8  err;
#if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P))
    u8  level;
#endif

#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    while(1)
    {
 		OSSemPend(gPwmBeepSem, OS_IPC_WAIT_FOREVER, &err);
		if(PWM_alarm_ready)
		{ // First beep on
	        gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
	      #if (AUDIO_DEVICE== AUDIO_IIS_WM8940)
	        i2cWrite_WM8940(0x36, 0x0039);
	      #endif
			PWM_alarm_ready = 0;
			UI_BEEP_ON;
			PWM_is_On=1;
			continue;
		}
		if((PWM_alarm_counter <= 0) && (PWM_is_On == 0))
		{ // Beep finish
			PWM_alarm_ready=1;
#if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P))
			gpioGetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, &level);
			DEBUG_TIMER("Set SPK after Beep in Power Saving!! level(%d)\r\n",level);
			if(level == GPIO_LEVEL_LO)
    			SPKERStop();
			else
                SPKERPlay();
#else
            SPKERPlay();
#endif
		}
    }
}
#endif


#if DOOR_BELL_SUPPORT
/*
*********************************************************************************************************
*                                         CheckBellList
*
* Description : Check ID exist in PairList, if power turn to Low level, record localtime for UI task Check
*               low battery alarm
*              
*
* Arguments   : DoorBellUnit -> struct for doorbell s
*
* Returns     : BELLINLIST_YES  -> ID Exist in PairList
*             : BELLINLIST_NO   -> ID don't exist in PairList
*             : BELLINLIST_FULL -> PairList Full  ,Bell number Upper limit is Five
*               
*********************************************************************************************************
*/
u8 CheckBellList(u32 ID,u8 power)
{
    u8 i;
    RTC_DATE_TIME   localTime;


    for(i=0;i<DOOR_BELL_NUM;i++)
    {
        if(DoorBellUnit[i].DevID == ID)
        {
            if((power == 0x01)&&(DoorBellUnit[i].DevPower == 0x0))
            {
                DoorBellUnit[i].DevPower = power;
                RTC_Get_Time(&localTime);
                DoorBellUnit[i].hour = localTime.hour;
                DoorBellUnit[i].min = localTime.min;
                DoorBellUnit[i].pushLowLV = TRUE;
            }
            else
            {
                DoorBellUnit[i].DevPower = power;
            }
            DEBUG_GREEN("BELLINLIST_YES ID 0x%x List[%d] 0x%x\n",ID,i,DoorBellUnit[i].DevID);
            return BELLINLIST_YES;
        }
    }
    if((i == DOOR_BELL_NUM)&&(DoorBellUnit[i-1].DevID==0))
    {
        DEBUG_GREEN("BELLINLIST_NO\n");
        return BELLINLIST_NO;
    }
    else
    {
        DEBUG_GREEN("BELLINLIST_NO\n");
        return BELLINLIST_FULL;
    }
}

/*
*********************************************************************************************************
*                                         DoorBellPolling
*
* Description : Polling TX Type,Power Status,ID ,send alarm to UI task
*              
*
* Arguments   : i2cRead_433RFModule_MCU Read data (5 Byte)
*             : include TX type(4 bit),Power status(4 bit),ID(32 bit)
*             : DoorData[0] include TX type & Power status
*             : DoorData[1~4] for ID
*
* Returns     : NA
*               
*********************************************************************************************************
*/
void DoorBellPolling(void)
{
    u8 DoorData[5] = 0;
    u8 err;
    u8 type;
    u8 power;
    u32 ID;
    u8 i;
    u8 checkList;
    u32 Default_ID = 0x12345678;
    static u8 tempkey = 0;
    
    if(i2cRead_433RFModule_MCU(&DoorData) == 0)
    {
        return;
    }
    if(DoorData[0] == 0xff)
    {
        return;
    }
    else
    {
        type  = DoorData[0] >> 4;
        power = DoorData[0] &0xf;
        ID    = DoorData[1]<<24 | DoorData[2]<<16 | DoorData[3] <<8 | DoorData[4];
        if((ID &~ Default_ID == 0) || (ID == 0x0))
            return;
        checkList = CheckBellList(ID,power);
    }
    
    if (checkList == BELLINLIST_YES)
    {
        DorBel_TriID = ID;
        switch(type)
        {
            case 0x0:/*RF Module Power off*/
                break;
            case 0x1:/*No signal*/
                break;
            case 0x2:/*SOS*/
                tempkey = UI_SOS_ALARM;
                DEBUG_CYAN("sentKeytoUI UI_SOS_ALARM %x\n",type);
                uiSentKeyToUi(tempkey);
                break;
            case 0x3:/*DoorBell*/
                tempkey = UI_DOORBELL_ALARM;
                DEBUG_CYAN("sentKeytoUI UI_DOORBELL_ALARM %x\n",type);
                uiSentKeyToUi(tempkey);
                break;
            case 0x4:/*Bed Blanket*/
                tempkey = UI_MAT_ALARM;
                DEBUG_CYAN("sentKeytoUI UI_MAT_ALARM %x\n",type);
                uiSentKeyToUi(tempkey);
                break;
            case 0x5: /*Bathroom Blanket*/
                tempkey = UI_MAT_ALARM2;
                DEBUG_CYAN("sentKeytoUI UI_MAT_ALARM2 %x\n",type);
                uiSentKeyToUi(tempkey);
                break;
        }
    }
    else if(checkList == BELLINLIST_NO)
    {
        DEBUG_CYAN("checkList == No type--> %x Mode %d\n",type,BellPairMode);
        if(BellPairMode == TRUE)
        {
            /*Enter Pair Mode*/      
            DEBUG_GREEN("Enter Pair Mode \n");
            for(i=0;i<DOOR_BELL_NUM;i++)
            {
                if(DoorBellUnit[i].DevID == 0x0)
                {
                    DoorBellUnit[i].DevID = ID;
                    DoorBellUnit[i].DevPower = power;
                    DoorBellUnit[i].DevType = type;
                    printf("Pair %d %d %d %d \n",i,ID,power,type);
                    OSFlagPost(DoorBellFlagGrp,DOORBELL_PAIR_FLAG, OS_FLAG_SET, &err);
                    BellPairMode = FALSE;
                    DEBUG_GREEN("Post Pair Suc\n");
                    break;
                }
                else
                    printf("No Pair %d %d %d %d \n",i,ID,power,type);
            }
        }
        else
        {
            /*Ignore Receive Data*/
        }
    }
    else
    {
        /*Bell List Full Ignore Receive Data*/
    }
}

/*
*********************************************************************************************************
*                                         DoorBellWrite
*
* Description : Write cmd to DoorBell MCU
*              
*
* Arguments   : i2cWrite_433RFModule_MCU write cmd 
*
* Returns     : NA
*               
*********************************************************************************************************
*/
void DoorBellWrite(u8 cmd)
{
/*
1: Open RF Module Power
2: notification pair
3: Unpair
*/

    if(i2cWrite_433RFModule_MCU(cmd)== 0)
    {
        DEBUG_GREEN("Bell Write Fail %d\n",cmd);
        return ;
    }
}

/*
*********************************************************************************************************
*                                         DoorBellinit
*
* Description : init doorBell Flag
*              
*
* Arguments   : DoorBellFlagGrp for pair counting sec
*
* Returns     : NA
*               
*********************************************************************************************************
*/

void DoorBellinit(void)
{
    u8 err;
    DoorBellFlagGrp = OSFlagCreate(0x00000000,&err);
}
#endif


void gpioICRCtrl(void)
{
#if ICR_CONTRAL_ENABLE
    static u8 IcrOnStep = 0;
    static u8 IcrOffStep = 0;

    #if (HW_BOARD_OPTION == MR9160_TX_MAYON_MWL613)
        u8 prevV = 0;
        u8 nextV = 1;
    #else
        u8 prevV = 1;
        u8 nextV = 0;
    #endif
    
    // ICR ON
    if ((IcrOnFlag > 0))
    {
        if ((IcrOnStep == 0))
        {
            gpioSetLevel(GPIO_GROUP_ICR_ON,GPIO_PIN_ICR_ON, prevV);
//	        DEBUG_GPIO("ICR_ON 1\n");
            IcrOnStep = 1;
        }
        IcrOnFlag--;
    }
    if((IcrOnFlag == 0) && (IcrOnStep ==1 ))
    {
        gpioSetLevel(GPIO_GROUP_ICR_ON,GPIO_PIN_ICR_ON, nextV);
//        DEBUG_GPIO("ICR_ON 0\n");
        IcrOnStep = 0;
    }

    // ICR OFF
    if ((IcrOffFlag > 0))
    {
        if ((IcrOffStep == 0))
        {
            gpioSetLevel(GPIO_GROUP_ICR_OFF,GPIO_PIN_ICR_OFF, prevV);
//	        DEBUG_GPIO("ICR_OFF 1\n");
            IcrOffStep = 1;
        }
        IcrOffFlag--;
    }
    if((IcrOffFlag == 0) && (IcrOffStep ==1 ))
    {
        gpioSetLevel(GPIO_GROUP_ICR_OFF,GPIO_PIN_ICR_OFF, nextV);
//        DEBUG_GPIO("ICR_OFF 0\n");
        IcrOffStep = 0;
    }
#endif
}

#if (PassiveIR_SensControl == PassiveIR_SS004)
/*
gpioPIRSensCtrl for adjust PIR-trigger sensitivity
int level
    0: call by timer
    100~1000: call by ui_cmd_project, 1000ms the most sensitive, 100ms the least
Experiment result:
    400ms-5~6M
    700ms-7~8M
    1000ms-10~11M
PIR can be much more sensitive, make trigger eariler so that YUV sensor get the first frame eariler.
VMD check (TX_PIRREC_VMDCHK) whether PIR false trigger, avoid power consumption.
*/
void gpioPIRSensCtrl(int level)
{
    static u8 first_settings = 1;
    static int PIRlevel;
    static u8 workingState = PIR_INIT;
    static int waitCount = 0;
    static int min_MD_sensitivity;
     //working on every power on
//    static u8 workingState = PIR_DONE; // for ui_cmd test mode, not to do every power on

    if (Main_Init_Ready != 1)
        return;

    if(first_settings){
        min_MD_sensitivity = (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] < iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT])?iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]:iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT];
        PIRlevel = PIR_SensitivityConfTab[min_MD_sensitivity];        
        first_settings = 0;
    }

    if(level >= 0) //uart cmd: KPIRLEVEL 700
    {
        workingState = PIR_INIT;
        PIRlevel = PIR_SensitivityConfTab[level];
    }

    // 300ms Low for start, 1s High, 300ms Low for end
    if(workingState != PIR_DONE) 
    {
        waitCount += 100; //ms
        
        if ( workingState == PIR_INIT )
        {
            waitCount = 0;
            gpioSetLevel(GPIO_GROUP_PIR_SENSITIVE, GPIO_BIT_PIR_SENSITIVE, GPIO_LEVEL_HI);
            workingState = PIR_START;
            //DEBUG_TIMER("PIR sens START level %d\n", PIRlevel);
        }
        else if (workingState == PIR_START)
        {
            if(waitCount >= 300){
                waitCount = 0;
                gpioSetLevel(GPIO_GROUP_PIR_SENSITIVE, GPIO_BIT_PIR_SENSITIVE, GPIO_LEVEL_LO);
                workingState = PIR_SET_LEVEL;
            }
        }
        else if (workingState == PIR_SET_LEVEL )
        {
            if(waitCount >= PIRlevel){
                waitCount = 0;
                gpioSetLevel(GPIO_GROUP_PIR_SENSITIVE, GPIO_BIT_PIR_SENSITIVE, GPIO_LEVEL_HI);
                workingState = PIR_END;
            }
        }
        else if (workingState == PIR_END )
        {
            if(waitCount >= 300){
                waitCount = 0;
                gpioSetLevel(GPIO_GROUP_PIR_SENSITIVE, GPIO_BIT_PIR_SENSITIVE, GPIO_LEVEL_LO);
                workingState = PIR_DONE;
                DEBUG_TIMER("PIR sens END level %d\n", PIRlevel);
            }
        } 
    }
}

#elif (PassiveIR_SensControl == PassiveIR_SS0041P)
void gpioPIRSensCtrl(int level)
{
    static u8 first_settings = 1;
    static int PIRlevel;
    static u8 workingState = PIR_INIT;
    static int waitCount = 0;
    static int min_MD_sensitivity;
    
     //working on every power on
    //static u8 workingState = PIR_DONE; // for ui_cmd test mode, not to do every power on

    if (Main_Init_Ready != 1)
        return;

    if(first_settings){
        min_MD_sensitivity = (iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY] < iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT])?iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]:iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT];
        PIRlevel = PIR_SensitivityConfTab[min_MD_sensitivity];        
        first_settings = 0;
    }

    if(level >= 0) //uart cmd: KPIRLEVEL 0 1 2 or VMD Set_TX_MOTION
    {
        workingState = PIR_INIT;
        PIRlevel = PIR_SensitivityConfTab[level];
        return; //make sure the first 25ms count from the beginning
    }

    // 50ms Low for start, 100ms High, 50ms Low for end
    if(workingState != PIR_DONE) 
    {
        waitCount += 25; //ms
        
        if ( workingState == PIR_INIT )
        {
            waitCount = 0;
            gpioSetLevel(GPIO_GROUP_PIR_SENSITIVE, GPIO_BIT_PIR_SENSITIVE, GPIO_LEVEL_HI);
            workingState = PIR_START;
            //DEBUG_TIMER("PIR sens START level %d\n", PIRlevel);
        }
        else if (workingState == PIR_START)
        {
            if(waitCount >= 50){
                waitCount = 0;
                gpioSetLevel(GPIO_GROUP_PIR_SENSITIVE, GPIO_BIT_PIR_SENSITIVE, GPIO_LEVEL_LO);
                workingState = PIR_SET_LEVEL;
            }
        }
        else if (workingState == PIR_SET_LEVEL)
        {
            if(waitCount >= PIRlevel){
                waitCount = 0;
                gpioSetLevel(GPIO_GROUP_PIR_SENSITIVE, GPIO_BIT_PIR_SENSITIVE, GPIO_LEVEL_HI);
                workingState = PIR_END;
            }
        }
        else if (workingState == PIR_END )
        {
            if(waitCount >= 50){
                waitCount = 0;
                gpioSetLevel(GPIO_GROUP_PIR_SENSITIVE, GPIO_BIT_PIR_SENSITIVE, GPIO_LEVEL_LO);
                workingState = PIR_DONE;
                DEBUG_TIMER("PIR sens END level %d\n", PIRlevel);
            }
        } 
    }
}
#endif

#if (TOUCH_KEY == TOUCH_KEY_BS83B12)



#if(HW_BOARD_OPTION == MR9200_RX_RDI_M906)
    void TouchKeyPolling(void)
    {
        u8 data;
        u8 tempKey;
        u32 UiMode;
        static u8 lastData=0, taklOn = 0, KeyCnt = 0;
        i2cRead_BS83B12(I2C_BS83B12_RD_SLAV_ADDR, &data);
        if (taklOn == 1)
        {
            if (data == 0)
            {
                taklOn = 0;
                uiSentKeyToUi(UI_KEY_TALK_OFF);
            }
        }

        if ((data != 0) &&(lastData == data))
        {
            UiMode = uiGetMenuMode();
            if ((UiMode != SET_NUMBER_MODE)&&(UiMode != PLAYBACK_MENU_MODE)&&(UiMode != SETUP_MODE))
                return;
            KeyCnt++;

            if ((KeyCnt < 20)&&(data == 1))
                return;
            else if(KeyCnt < 5)
                return;
            else
            {
                if(data == 1)
                    KeyCnt = 20;
                else
                    KeyCnt = 5;
            }
            switch (data)
            {
                case 1:
                    tempKey=UI_KEY_GOTO;
                    taklOn = 1;
                    DEBUG_TIMER("Key UI_KEY_GOTO \n");
                    break;

                case 4:
                    tempKey=UI_KEY_CONT_RIGHT;
                    DEBUG_TIMER("Key UI_KEY_CONT_RIGHT \n");
                    break;

                case 5:
                    tempKey=UI_KEY_CONT_DOWN;
                    DEBUG_TIMER("Key UI_KEY_CONT_DOWN \n");
                    break;

                case 7:
                    tempKey=UI_KEY_CONT_LEFT;
                    DEBUG_TIMER("Key UI_KEY_CONT_LEFT \n");
                    break;

                case 11:
                    tempKey=UI_KEY_CONT_UP;
                    DEBUG_TIMER("Key UI_KEY_CONT_UP \n");
                    break;

                default:
                    return;
            }
            uiSentKeyToUi(tempKey);
        }

        else if( (lastData==0) && (data != 0) )
        {
            switch(data)
            {
                case 1:
                    tempKey=UI_KEY_TALK;
                    taklOn = 1;
                    DEBUG_TIMER("Key UI_KEY_TALK \n");
                    break;
                case 2:
                    tempKey=UI_KEY_SCAN;
                    DEBUG_TIMER("Key UI_KEY_SCAN, \n");
                    break;
                case 3:
                    tempKey=UI_KEY_QUAD;
                    DEBUG_TIMER("Key UI_KEY_QUAD \n");
                    break;
                case 4:
                    tempKey=UI_KEY_RIGHT;
                    DEBUG_TIMER("Key UI_KEY_RIGHT \n");
                    break;
                case 5:
                    tempKey=UI_KEY_DOWN;
                    DEBUG_TIMER("Key UI_KEY_DOWN \n");
                    break;
                case 6:
                    tempKey=UI_KEY_MOTION;
                    DEBUG_TIMER("Key UI_KEY_MOTION \n");
                    break;
                case 7:
                    tempKey=UI_KEY_LEFT;
                    DEBUG_TIMER("Key UI_KEY_LEFT \n");
                    break;
                case 8:
                    tempKey=UI_KEY_OK;
                    DEBUG_TIMER("Key UI_KEY_OK \n");
                    break;
                case 9:
                    tempKey=UI_KEY_MENU;
                    DEBUG_TIMER("Key UI_KEY_MENU \n");
                    break;
                case 10:
                    tempKey=UI_KEY_REC;
                    DEBUG_TIMER("Key UI_KEY_REC \n");
                    break;
                case 11:
                    tempKey=UI_KEY_UP;
                    DEBUG_TIMER("Key UI_KEY_UP \n");
                    break;
                default:
                    DEBUG_TIMER("Key Default %d \n",data);

            }
            if(tempKey !=0)
            {
                uiSentKeyToUi(tempKey);
            }
        }
        else if ((data != 0) &&(lastData == data) && (data == 1))
        {
            UiMode = uiGetMenuMode();
            if (UiMode != SETUP_MODE)
                return;
            KeyCnt++;

            if (KeyCnt < 10)
                return;
            else
                KeyCnt = 10;

            switch (data)
            {
                case 1:
                    DEBUG_TIMER("Key UI_KEY_TALK for Default Cnt !!\n");
                    uiSentKeyToUi(UI_KEY_TALK);
                    break;
                default:
                    return;
            }
        }
        else
            KeyCnt = 0;

        lastData=data;


    }
#endif


#endif


