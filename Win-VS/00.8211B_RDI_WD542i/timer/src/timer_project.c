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

#if defined(NEW_UI_ARCHITECTURE)
    #include "..\..\ui\inc\Key.h"
    #include "..\..\ui\inc\MainFlow.h"
#elif (HW_BOARD_OPTION == MR9670_WOAN)    
    #include "..\..\ui\inc\Key.h"
#endif

#if (HW_BOARD_OPTION == MR6730_AFN)
#include "GlobalVariable.h"
#endif
#if(CHIP_OPTION >= CHIP_A1013A)
#include "rfiuapi.h"
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
#if DOOR_BELL_SUPPORT
OS_FLAG_GRP *DoorBellFlagGrp = 0;
#endif

#if(SUPPORT_TOUCH)
bool touch_press=FALSE;
#endif
#if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||\
     (HW_BOARD_OPTION  == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
u8  timerDisableMic = 0;
#endif
#if PLAY_NOAUDIOFILE
u32 noaudio_timer_index=0;
#endif
#if (Melody_SNC7232_ENA)
u8 Melody_play=0;  //0:stop 1:cmd 2:play 3:play all 4:pause
u8 Melody_play_num=0;
u8 Melody_audio_level=3;
u8 fplay=0;
u8 Melody_retry=0;
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
extern u8  gInsertCard;

#if Auto_Video_Test
    extern u8 playbackflag;
    extern u8 uiMenuEnable;
    extern OS_EVENT* general_MboxEvt;
    extern u8 Iframe_flag; // Decided get I-frame or play whole file
#endif



#if(MONITOR_ISU_DISPLAY_BUFFER_IDX)
extern s32 old_ISU_isu_idufrmcnt;
extern s32 old_FID_isu_idufrmcnt;
extern s32 ISU_int_time;
extern s32 FID_int_time;
extern s32 Timer3_int_time;
extern s32 display_idx_timeoffset;
#endif
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

#if (HW_BOARD_OPTION == MR8211_ZINWELL)
extern u16 Humidity;
extern u16 Light;
extern f32 TEMP;
extern s32 TempC;
#endif

#if MOTOR_EN
#if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
//NULL
#else
extern u8  MotorStatusH;   // 0: 停止, 1: 正轉, 2: 負轉
extern u8  MotorStatusV;   // 0: 停止, 1: 正轉, 2: 負轉
#endif
#endif
#if PLAY_NOAUDIOFILE
extern u8 noaudio_flag;
extern  s64 Videodisplaytime[DISPLAY_BUF_NUM];
#endif

#if( (HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I) )  
extern u8  nMotorTime;
#endif
/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */
#define UI_BEEP_ON	{Timer3Ctrl=0;Timer3Count=gbeep_on_time_counter;Timer3IntEna=2;Timer3Ctrl=0x17000000|gbeep_clock_div|((u32)gBeep_pause_width<<16);Gpio0Ena&=(~(1<<1));}
#define UI_BEEP_OFF	{Timer3Ctrl=0;Timer3Count=gbeep_off_time_counter;Timer3IntEna=2;Timer3Ctrl=0x15000000|gbeep_clock_div|((u32)gBeep_pause_width<<16);Gpio0Ena|=((1<<1));Gpio0Level&=(~(1<<1));}
#define UI_BEEP_LONG_OFF	{Timer3Ctrl=0;Timer3Count=gbeep_delay_time_counter;Timer3IntEna=2;Timer3Ctrl=0x15800000|gbeep_clock_div;Gpio0Ena|=((1<<1));Gpio0Level&=(~(1<<1));}
#define UI_BEEP_STOP	{Timer3IntEna=0;Timer3Ctrl=0;Gpio0Ena|=((1<<1));Gpio0Level&=(~(1<<1));}

 /*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
void PWM_ISR_handler(void);
void gpioSensorCtrl100ms(void);
void DoorBellPolling(void);
/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */


void timer_KeyPolling(void)
{
    OS_tickcounter ++;
#if defined(NEW_UI_ARCHITECTURE)
    Key_polling();
#elif(HW_BOARD_OPTION==MR9670_WOAN)
	Key_polling();
#elif((HW_BOARD_OPTION==MR6730_WINEYE) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
    gpioKeyPolling();
#elif ((UI_VERSION == UI_VERSION_RDI) || (HW_BOARD_OPTION==MR8200_RX_GCT_LCD) || (HW_BOARD_OPTION==MR8600_RX_JESMAY_LCD)||\
      (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
    gpioKeyPolling();
    adcKeyPolling();
#else
   adcKeyPolling();
#endif
}

void timer_1s_IntHandler(void)
{
    extern u8 Enter_Wifi_Connect;  // 1: Enter Wifi Connect Mode.

	u8	err;
    RTC_DATE_TIME gmt;
	static u32	ucCount = 0;
#if (HW_IR_SUPPORT == 1)
    u8  irKey;
#endif
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) )
    u32 fps;
    #endif
    u32 sec;
    static u8 retry_count = 0;
   
#if(BLE_SUPPORT)
    if(g_BLEsyncTimeError)
    {
        if(BLEGetStatus() == BLE_CONNECTED)
        {
            if(retry_count % 8 == 0)
                sysbackSetEvt(SYS_BACK_BLE_SYNCTIME, 0);
            retry_count++;
            if(retry_count == 25) //try 7 times the most
            {
                retry_count = 0;
                g_BLEsyncTimeError = 0;
            }
        }
        else
        {
            retry_count = 0;
            g_BLEsyncTimeError = 0;

        }
    }
    else
    {
        retry_count = 0;
    }
    if(u8BLEexist == FALSE && g_BLEmoduleDetCount < 3)
    {
        sysbackSetEvt(SYS_BACK_BLE_SYNCTIME, 3);
    }
#endif
//-------------------Time updating--------------------------------------------//
#if (USE_BUILD_IN_RTC == RTC_USE_BUILD_IN)
    if( (ucCount & 0x0fff) == 0)  //Lucian:每 4096 sec  讀取RTC
    {
        //RTC_Get_GMT_Time(&gmt);
        //RTCTime_Gmt_To_Local(&gmt, &g_LocalTime);
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
   	if( (ucCount & 0x1FF) == 0) //Lucian: 每 512 sec 讀一次RTC
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

//----Draw Overlay image on video----//
#if (ISU_OVERLAY_ENABLE &&(HW_BOARD_OPTION != MR6730_WINEYE))
    if(isuGenerateScalarOverlayImage)
        sysbackSetEvt(SYS_BACK_DRAWTIMEONVIDEOCLIP, 1);
#endif


//----------Show time on OSD when video clip----------//
    sysbackSetEvt(SYS_BACK_CHECK_UI_CONTROL, 0);

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
#elif (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV)
    if ( (Main_Init_Ready == 1) && (sysVideoInSel==VIDEO_IN_TV) )
            sysbackSetEvt(SYS_BACK_CHECK_TVIN_FORMAT,0);

#elif MULTI_CHANNEL_SUPPORT
    if ( (Main_Init_Ready == 1) && (sysVideoInSel==VIDEO_IN_TV) )
            sysbackSetEvt(SYS_BACK_CHECK_TVIN_FORMAT,0);
#endif


   //-----------Battery detetct-------------//
   #if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)\
    ||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
   /*100ms delect Battery*/
   #else
    adcBatteryCheck();
   #endif

#if (HW_BOARD_OPTION == MR8100_GCT_VM9710 && UI_PROJ_OPT == 1)
    adcTemperaturePolling();
#endif

    // Replace the RTC function
    if(sysPlaybackVideoPause == 0)
    {
        OS_ENTER_CRITICAL();
        RTCseconds++;
        OS_EXIT_CRITICAL();
    }
      //--------------For Testing---------------------//
#if( (HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I)  )   
    if(nMotorTime>0)
        nMotorTime --;
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


#if((HW_BOARD_OPTION == MR8120_TX_SKYSUCCESS )||(HW_BOARD_OPTION == MR8120_TX_RDI3) ||\
    (HW_BOARD_OPTION == MR8120_TX_RDI2) || (HW_BOARD_OPTION == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8211_ZINWELL)||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA671)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA672)||\
    (HW_BOARD_OPTION == MR8600_RX_JESMAY_LCD)||(HW_BOARD_OPTION == MR8120_TX_HECHI)||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652) ||\
    (HW_BOARD_OPTION == MR8200_RX_JIT_BOX_AV))
    gpioKeyPolling();

#endif

#if (HW_IR_SUPPORT == 1)
    if ((ucCount & 0x3) == 0)
    {
        irKey = IR_INT_STATUS;
    }
#endif

#if(HW_BOARD_OPTION == MR8120_TX_ZINWELL)
    //adcKeyPolling1();
#endif


    
#if( ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) ) && ((Sensor_OPTION == Sensor_OV7725_YUV601) || (Sensor_OPTION == Sensor_HM1375_YUV601) ))
    fps=ciuCheckFrameRate(); 
    if((ucCount % 10)== 0)
    {
        DEBUG_TIMER("FPS:%d \n",fps);
    }
#endif

#if TX_SNAPSHOT_SUPPORT
    if((ucCount % 30)== 0)
    {
        sysTXSnapshotCheck();        
    }
#endif

#if (HW_BOARD_OPTION == MR6730_AFN)

	if(gApp_Var.UN.MB.SWRST_RestSec)
	{
		_APP_ENTER_CS_;
		gApp_Var.UN.MB.SWRST_RestSec--;
		_APP_EXIT_CS_;
		DEBUG_TIMER("system is going to reboot(%d)\n\n", gApp_Var.UN.MB.SWRST_RestSec);

		if(gApp_Var.UN.MB.SWRST_RestSec==0)
		{
			DEBUG_TIMER("< software reset... >\n\n");
			sysSet_Reset();//sw-reset
		}	
	}

	
	#if (USE_UI_TASK_WDT)
		UI_TaskWdt();
	#endif



	#if 1
	MACRO_UI_SET_SYSSTAT_BIT_SET(UI_SET_SYSSTAT_BIT_TMR_1S);
	OSSemPost(uiSemEvt);//notify uiTask()
	#endif	

#endif
#if (Melody_SNC7232_ENA)

    if(Melody_retry != Melody_NORMAL)
    {
        switch(Melody_retry)
        {
            case Melody_STOP:
                uiSentKeyToUi(UI_KEY_Melody_Stop);
                break;
            case Melody_PAUSE:
                uiSentKeyToUi(UI_KEY_Melody_Pause);
                break;
            case Melody_PLAY:
                uiSentKeyToUi(UI_KEY_Melody_Play);
                break;
            case Melody_PLAYALL:
                uiSentKeyToUi(UI_KEY_Melody_PlayAll);
                break;
            case Melody_START:
                uiSentKeyToUi(UI_KEY_Melody_Start);
                break;
            case Melody_STARTALL:
                uiSentKeyToUi(UI_KEY_Melody_StartAll);
                break;
            case Melody_PLAYNEXT:
                uiSentKeyToUi(UI_KEY_Melody_PlayNext);
                break;
            case Melody_SETVOLUME:
                uiSentKeyToUi(UI_KEY_Melody_Volume);
                break;
            default:
                break;
        }
    }
#endif
#if (HW_BOARD_OPTION == MR8100_GCT_LCD)
    uiFlowCheckBatteryCharge();
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
#if TV_DISP_BY_IDU
  #if(TV_DISP_BY_TV_INTR)

  #else
    if( sysCameraMode == SYS_CAMERA_MODE_PREVIEW)
    {
        marsTimerPwmCountEnable(3, 0);
        if(isu_idufrmcnt != 0)
                IduWinCtrl = (IduWinCtrl & ~0x00003000) | (( (isu_idufrmcnt-1) % 3) << 12);

        #if(MONITOR_ISU_DISPLAY_BUFFER_IDX)
        if((isu_idufrmcnt-1!=old_Timer_isu_idufrmcnt+1)&&(isu_idufrmcnt>10))
            DEBUG_TIMER("3.<%d,%d,%d>\n",isu_idufrmcnt-1,old_Timer_isu_idufrmcnt,isuFrameTime);
        old_Timer_isu_idufrmcnt = isu_idufrmcnt-1;
        #endif
    }
    else
    {
        marsTimerPwmCountEnable(3, 0);
        IduWinCtrl = ( IduWinCtrl & (~0x00003000) );
        if(MainVideodisplaybuf_idx == 0)
            IduVidBuf0Addr= (u32)MainVideodisplaybuf[0];
        else
            IduVidBuf0Addr= (u32)MainVideodisplaybuf[(MainVideodisplaybuf_idx-1) % DISPLAY_BUF_NUM];
        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
    }
  #endif
#endif
#endif  /*end of #if (TIMER_TEST == 1)*/
    PWM_ISR_handler();
}

void timer4_IntHandler(void)
{
#if (TIMER_TEST == 1)
    RTC_DATE_TIME   localTime;

    RTC_Get_GMT_Time(&localTime);
    DEBUG_TIMER("Timer 4 %d:%d:%d\n",localTime.hour,localTime.min,localTime.sec);
#else

//sunway used for time delay of TV display
#if TV_DISP_BY_IDU
  #if(TV_DISP_BY_TV_INTR)

  #else
    if( sysCameraMode == SYS_CAMERA_MODE_PREVIEW)
    {
        marsTimerPwmCountEnable(4, 0);
        IduWinCtrl = (IduWinCtrl & ~0x00003000) | (( (isu_idufrmcnt-1) % 3) << 12);
    }
    else
    {
        marsTimerPwmCountEnable(4, 0);
        IduWinCtrl = ( IduWinCtrl & (~0x00003000) );
        if(MainVideodisplaybuf_idx == 0)
            IduVidBuf0Addr= (u32)MainVideodisplaybuf[0];
        else
            IduVidBuf0Addr= (u32)MainVideodisplaybuf[(MainVideodisplaybuf_idx-1) % DISPLAY_BUF_NUM];
        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
    }
  #endif
#endif
#endif  /*end of #if (TIMER_TEST == 1)*/
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
   	//---Do Key Scan---//

#if (HW_BOARD_OPTION == MR8120_TX_Philio)
    static u8 count=0;

    if(count>40)
        timer_KeyPolling();
    count++;
#else
    timer_KeyPolling();
#endif
#if (HW_BOARD_OPTION == MR6730_AFN)
	IO_Status_Refresh();
#endif 
}

void timer_10ms_IntHandler(void)
{
}

void timer_50ms_IntHandler(void)
{
    //OSTimeTick();   // used for OS tick (50ms)
#if (HW_BOARD_OPTION==MR6730_AFN)
		MACRO_UI_SET_SYSSTAT_BIT_SET(UI_SET_SYSSTAT_BIT_TMR_50MS);
		//OSSemPost(uiSemEvt);//notify uiTask()
#endif
#if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||\
     (HW_BOARD_OPTION  == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    if (timerDisableMic > 0)
    {
        timerDisableMic--;
        if (timerDisableMic == 0)
        {
          #if ((HW_BOARD_OPTION  == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
            if(sys8211TXWifiStat != MR8211_QUIT_WIFI)
                adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN);
          #endif
            adcEMICOnOff(1);
        }
    }
#endif
#if PLAY_NOAUDIOFILE
    if(noaudio_flag)
    {
        iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
        MainVideodisplaybuf_idx++;
        noaudio_timer_index++;
        VideoNextPresentTime    = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
    }
    else
        noaudio_timer_index=0;
#endif

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

#if (SUPPORT_TOUCH == 1)
        if (i2c_Touch_getPosition(&TouchX, &TouchY) == TRUE)
        {
            uiFlowCheckTouchKey(TouchX, TouchY);
            DEBUG_TIMER("Touched: x:%d, y:%d\n", TouchX, TouchY);
            touch_press=TRUE;  /*Roy: it should be set TRUE after uiFlowCheckTouchKey(TouchX, TouchY); */
        }
        else
        {
            #if((HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505) || (HW_BOARD_OPTION == MR8100_GCT_LCD))
            uiFlowPTZStop();
            #elif ((HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
            uiFlowStopTouchkeyAction();
            #endif
            touch_press=FALSE;
        }
#endif

    #if((HW_BOARD_OPTION == MR8120_RX_JESMAY) || (HW_BOARD_OPTION == MR8120_TX_JESMAY) ||\
        (HW_BOARD_OPTION == MR8600_RX_RDI) || (HW_BOARD_OPTION == MR8120_RX_RDI) ||\
        (HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS) || (HW_BOARD_OPTION == MR8120_TX_RDI_AV)||\
        (HW_BOARD_OPTION ==  MR8120_TX_TRANWO) || (HW_BOARD_OPTION == MR8600_RX_TRANWO) ||\
        (HW_BOARD_OPTION ==  MR8120_TX_TRANWO2) ||(HW_BOARD_OPTION == MR8600_RX_RDI2) ||\
        (UI_VERSION == UI_VERSION_RDI) || (HW_BOARD_OPTION ==  MR8600_RX_JESMAY_LCD)||\
        (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM719) || (HW_BOARD_OPTION == MR8120_RX_MAYON_MWM710)||\
        (HW_BOARD_OPTION == MR8120_TX_JESMAY_LCD)||(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014) ||\
        (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3)||\
        (UI_VERSION == UI_VERSION_TRANWO) || (HW_BOARD_OPTION == MR8120_RX_FRESHCAM) ||\
        (HW_BOARD_OPTION == MR8120_TX_FRESHCAM)|| (HW_BOARD_OPTION == MR8120_RX_MAYON_MWM011)||\
        (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505) || (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902)||\
        (HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3)||\
        (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
        gpioTimerCtrLed(LED_NONE);
    #endif
//    #if 0//(HW_BOARD_OPTION == MR8120_TX_JIT)
        //adcKeyPolling1();
//    #endif
    #if((HW_BOARD_OPTION == MR8120_RX_JESMAY) ||  (HW_BOARD_OPTION == MR8600_RX_RDI)  ||\
        (HW_BOARD_OPTION == MR8120_RX_RDI)    ||  (HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS)||\
        (HW_BOARD_OPTION == MR8600_RX_GCT)    ||  (HW_BOARD_OPTION == MR8600_RX_DB2) ||\
        (HW_BOARD_OPTION == MR8120_TX_GCT)    ||  (HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD) ||\
        (HW_BOARD_OPTION == MR8120_TX_TRANWO) ||  (HW_BOARD_OPTION == MR8120_TX_TWT_FCC)||\
        (HW_BOARD_OPTION == MR8600_RX_JESMAY) ||  (HW_BOARD_OPTION == MR8600_RX_JIT) ||\
        (HW_BOARD_OPTION == MR8120_TX_JIT)    ||  (HW_BOARD_OPTION == MR8600_RX_TRANWO)||\
        (HW_BOARD_OPTION ==  MR8120_TX_TRANWO2) ||(HW_BOARD_OPTION == MR8600_RX_RDI2) ||\
        (HW_BOARD_OPTION == MR8120_TX_RDI_CA530) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593) ||\
        (HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) || (HW_BOARD_OPTION == MR8120_TX_FRESHCAM)||\
        (HW_BOARD_OPTION == MR8120_RX_FRESHCAM)||(HW_BOARD_OPTION == MR8600_RX_JESMAY_LCD)||\
        (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM719)||(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM720)||\
        (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014)||(HW_BOARD_OPTION == MR8120_RX_MAYON_MWM710)||\
        (HW_BOARD_OPTION == MR8120_RX_MAYON_MWM011) ||(HW_BOARD_OPTION == MR8600_RX_MAYON)||\
        (HW_BOARD_OPTION == MR8120_TX_MAYON_LW604)||\
        (HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C)||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593_HA)||\
        (HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)||\
        (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)||(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902)||\
        (HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_GCT_VM9710) ||\
        (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA) ||\
        (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS) ||\
        (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592RS) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM) ||\
        (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8100_RX_RDI_M512) ||\
        (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589) || (HW_BOARD_OPTION == MR8120_TX_TRANWO3)||\
        (HW_BOARD_OPTION == MR8120S_TX_GCT_VM00)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)||\
        (HW_BOARD_OPTION == MR8120_TX_GCT_VM00) || (HW_BOARD_OPTION == MR8120_RX_GCT_SC7700) ||\
        (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N))
        gpioKeyPolling();
    #endif

    #if (HW_BOARD_OPTION==MR6730_WINEYE)
        #if ISU_OVERLAY_ENABLE
        {
            sysbackSetEvt(SYS_BACK_DRAWTIMEONVIDEOCLIP, 1);
        /*
            static  u32 i   = 0;
            if((i % 2) == 0)
                sysbackSetEvt(SYS_BACK_DRAWTIMEONVIDEOCLIP, 1);
            i++;
        */
        }
        #endif
    #endif

    /*
    #if (HW_BOARD_OPTION==MR8211_ZINWELL)
        gpioSensorCtrl100ms();
    #endif
    */
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

#if (HW_BOARD_OPTION==MR6730_AFN)
	UI_step100msTimer();
#endif

   //-----------Battery detetct-------------//
   #if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)\
    ||(HW_BOARD_OPTION == MR8100_RX_RDI_M512))
   adcBatteryCheck(); /*100ms delect battery*/
   #endif

}

void timer_200ms_IntHandler(void)
{
u8 Status,bVideoResetcont;
u8 cnt;

#if (TV_DECODER == WT8861)
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
}
void timer_500ms_IntHandler(void)
{
    u8 i,err;

    #if (HW_BOARD_OPTION==MR9670_WOAN)
        sysbackSetEvt(SYS_BACK_SHOWTIMEONOSD_VIDEOCLIP, 0);
        UI_step500msTimer();
        return;
    #endif

 #if 1
    /*Hi Lucain, 如果這個註解掉會造成 RX playback回放時間不顯示, 請知悉 */
    if(sysVoiceRecStart!=0)
        sysbackSetEvt(SYS_BACK_SHOWTIMEONOSD_VIDEOCLIP, 0);
 #endif
    #if 0//((HW_BOARD_OPTION == MR8100_GCT_LCD)||(HW_BOARD_OPTION == MR8100_RX_RDI_SEM))
    //----------Show time on OSD when video clip----------//
    sysbackSetEvt(SYS_BACK_CHECK_UI_CONTROL_500MS, 0);
    #endif
    
    #if defined(NEW_UI_ARCHITECTURE)
        UI_step500msTimer();
    #endif
    #if (Melody_SNC7232_ENA)
        if( (Melody_play ==2) || (Melody_play ==3) )
        {
            gpioGetLevel(1,18,&fplay);
            if(fplay == 0)
            {
                if(Melody_play==3)
                {
                    uiSentKeyToUi(UI_KEY_Melody_PlayNext);
                }
                else // 2
                {
                    uiSentKeyToUi(UI_KEY_Melody_Play);
                }
            }
        }
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
        #if MOTOR_EN
            #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
            //NULL
            #else
            timer3Setting();
            #endif
        #endif

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
                #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                    (CHIP_OPTION == CHIP_A1026A))
                    //Lucian: Used for RF , 100us unit.
                    marsTimerConfig(guiRFTimerID, &timerCfg_48M[4]);
                    marsTimerCountWrite(guiRFTimerID, TIMER7_COUNT);
                    marsTimerCountEnable(guiRFTimerID, 1);
                    marsTimerInterruptEnable(guiRFTimerID, 0);       //Enable while using
                #endif
            #endif
            //Lucina: 當time delay for TV out
            #if TV_DISP_BY_IDU
              #if(TV_DISP_BY_TV_INTR)

              #else
                timer3Setting();
              #endif
            #endif

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

#if (TOUCH_KEY == TOUCH_KEY_BS8112A3)
    #if 0 //(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902) //不確定是否有其他案子使用, 先用案子隔開了; Cliet said not stable, need revert, TBD 20171113
void TouchKeyPolling(void)
{
    u8 data,data1;
    u8 tempKey=UI_KEY_NONE;
    u32 UiMode;
    static u8 lastkey=0;
    static u8 lastData=0, lastData1=0, ledOn = 1, KeyCnt = 0, KeyCnt1 = 0;

    i2cRead_Byte(0xa1, 0x08, &data);
    //DEBUG_TIMER("===> 0x08 = %d \n",data);
    i2cRead_Byte(0xa1, 0x09, &data1);
    //DEBUG_TIMER("===> 0x09 = %d \n",data1);
    if((data == 255) && (data1 == 255) && (ledOn == 1))
    {
        gpioSetLevel(GPIO_GROUP_TV_LED, GPIO_BIT_TV_LED, 0);
        ledOn=0;
    }

    if ((data != 0) && (data != 255))
    {
        if ((lastData != data) || (KeyCnt > 5)) //for touch || longpress
        {
            switch (data)
            {
                case 1:
                    tempKey = UI_KEY_PLAY;
                    DEBUG_TIMER("Key UI_KEY_PLAY \n");
                    break;
                case 2:
                    tempKey = UI_KEY_REC;
                    DEBUG_TIMER("Key UI_KEY_REC \n");
                    break;
                case 4:
                    tempKey = UI_KEY_MENU;
                    DEBUG_TIMER("Key UI_KEY_MENU \n");
                    break;
                case 8:
                    tempKey = UI_KEY_RIGHT;
                    DEBUG_TIMER("Key UI_KEY_RIGHT \n");
                    break;
                case 16:
                    tempKey = UI_KEY_DOWN;
                    DEBUG_TIMER("Key UI_KEY_DOWN \n");
                    break;
                case 32:
                    tempKey = UI_KEY_LCD_BL;
                    DEBUG_TIMER("Key UI_KEY_LCD_BL \n");
                    break;
                case 64:
                    tempKey = UI_KEY_LEFT;
                    DEBUG_TIMER("Key UI_KEY_LEFT \n");
                    break;
                case 128:
                    tempKey = UI_KEY_ENTER;
                    DEBUG_TIMER("Key UI_KEY_ENTER \n");
                    break;
                default:
                    return;
            }
            if(tempKey!=UI_KEY_NONE)
            {
                if(ledOn == 0)
                {
                    gpioSetLevel(GPIO_GROUP_TV_LED, GPIO_BIT_TV_LED, 1);
                    ledOn = 1;
                }
                uiSentKeyToUi(tempKey);
                if (KeyCnt > 5)
                    KeyCnt -= 3; // controll send key freq when long press

            } 
        }
        else
        {
            KeyCnt++;
        }
        
    }
    else
    {
        KeyCnt = 0;
    }

    if ((data1 != 0) && (data1 != 255))
    {
        if ((lastData1 != data1) || (KeyCnt1 > 5))
        {
            switch (data1)
            {
                case 1:
                    tempKey = UI_KEY_RF_QUAD;
                    DEBUG_TIMER("Key UI_KEY_RF_QUAD \n");
                    break;
                case 2:
                    tempKey = UI_KEY_TALK;
                    DEBUG_TIMER("Key UI_KEY_TALK \n");
                    break;
                case 4:
                    tempKey = UI_KEY_UP;
                    DEBUG_TIMER("Key UI_KEY_UP \n");
                    break;
                default:
                    return;
            }
            if(tempKey != UI_KEY_NONE)
            {
                if(ledOn == 0)
                {
                    gpioSetLevel(GPIO_GROUP_TV_LED, GPIO_BIT_TV_LED, 1);
                    ledOn = 1;
                }
                uiSentKeyToUi(tempKey);
                if (KeyCnt1 > 5)
                    KeyCnt1 -= 3;
            }
        }
        else
        {
            KeyCnt1++;
        }
        
    }
    else
    {
        KeyCnt1 = 0;
    }

    lastData = data;
    lastData1 = data1;
}

#else

void TouchKeyPolling(void)
{
    u8 data,data1;
    u8 tempKey=UI_KEY_NONE;
    u32 UiMode;
    static u8 lastkey=0;
    static u8 lastData=0, lastData1=0, ledOn = 1, KeyCnt = 0, KeyCnt1 = 0;

    i2cRead_Byte(0xa1, 0x08, &data);
    //DEBUG_TIMER("===> 0x08 = %d \n",data);
    i2cRead_Byte(0xa1, 0x09, &data1);
    //DEBUG_TIMER("===> 0x09 = %d \n",data1);
    if((data == 255) && (data1 == 255) && (ledOn == 1))
    {
        gpioSetLevel(GPIO_GROUP_TV_LED, GPIO_BIT_TV_LED, 0);
        ledOn=0;
    }
    #if 0
    else if(((data != 255) && (data != 0) && (ledOn == 0)) || ((data1 != 255) && (data1 != 0) && (ledOn == 0)))
    {
        gpioSetLevel(GPIO_GROUP_TV_LED, GPIO_BIT_TV_LED, 1);
        ledOn=1;
    }
    #endif

    if ((data != 0) && (data != 255) &&(lastData == data))
    {
        if (KeyCnt > 0)
            return;
        
        switch (data)
        {
            case 1:
                tempKey=UI_KEY_PLAY;
                DEBUG_TIMER("Key UI_KEY_PLAY \n");
                break;
            case 2:
                tempKey=UI_KEY_REC;
                DEBUG_TIMER("Key UI_KEY_REC \n");
                break;
            case 4:
                tempKey=UI_KEY_MENU;
                DEBUG_TIMER("Key UI_KEY_MENU \n");
                break;
            case 8:
                tempKey=UI_KEY_RIGHT;
                DEBUG_TIMER("Key UI_KEY_RIGHT \n");
                break;
            case 16:
                tempKey=UI_KEY_DOWN;
                DEBUG_TIMER("Key UI_KEY_DOWN \n");
                break;
            case 32:
                tempKey=UI_KEY_LCD_BL;
                DEBUG_TIMER("Key UI_KEY_LCD_BL \n");
                break;
            case 64:
                tempKey=UI_KEY_LEFT;
                DEBUG_TIMER("Key UI_KEY_LEFT \n");
                break;
            case 128:
                tempKey=UI_KEY_ENTER;
                DEBUG_TIMER("Key UI_KEY_ENTER \n");
                break;
            default:
                return;
        }
        if(tempKey!=UI_KEY_NONE)
        {
            if(ledOn == 0)
            {
                gpioSetLevel(GPIO_GROUP_TV_LED, GPIO_BIT_TV_LED, 1);
                ledOn=1;
            }
            uiSentKeyToUi(tempKey);
            KeyCnt=1;
        }
    }
    else
    {
        if(KeyCnt>0)
            KeyCnt--;
        else
            KeyCnt = 0;
    }

    if ((data1 != 0) && (data1 != 255) && (lastData1 == data1))
    {
        if (KeyCnt1 > 0)
            return;
        
        switch (data1)
        {
            case 1:
                tempKey=UI_KEY_RF_QUAD;
                DEBUG_TIMER("Key UI_KEY_RF_QUAD \n");
                break;
            case 2:
                tempKey=UI_KEY_TALK;
                DEBUG_TIMER("Key UI_KEY_TALK \n");
                break;
            case 4:
                tempKey=UI_KEY_UP;
                DEBUG_TIMER("Key UI_KEY_UP \n");
                break;
            default:
                return;
        }
        if(tempKey!=UI_KEY_NONE)
        {
            if(ledOn == 0)
            {
                gpioSetLevel(GPIO_GROUP_TV_LED, GPIO_BIT_TV_LED, 1);
                ledOn=1;
            }
            uiSentKeyToUi(tempKey);
            KeyCnt1=1;
        }
    }
    else
    {
        if(KeyCnt1>0)
            KeyCnt1--;
        else
            KeyCnt1 = 0;
    }

    lastData=data;
    lastData1=data1;
}
    #endif
#endif

#if DOOR_BELL_SUPPORT

void DoorBellPolling(void)
{
    u8 DoorData = 0,err;
    static u8 tempkey = 0;
    if(i2cRead_433RFModule_MCU(&DoorData)==0)
        return;

    switch(DoorData)
    {
        case 0: /*DoorBell_PowerOff*/
            break;
        case 7: /*DoorBell_Idle*/
            break;
        case 6: /*DoorBell_Alarm*/
            tempkey = UI_DOORBELL_ALARM;
            uiSentKeyToUi(tempkey);
            break;
        case 5: /*SOS_Alarm*/
            tempkey = UI_SOS_ALARM;
            uiSentKeyToUi(tempkey);
            break;
        case 4: /*Pair_Success*/
            OSFlagPost(DoorBellFlagGrp,DOORBELL_PAIR_FLAG, OS_FLAG_SET, &err);
            DEBUG_GREEN("Post Pair Suc\n");
            break;
        default :
            //DEBUG_GREEN("Unexpectedly DoorBell Value %d \n",DoorData);
            break;
    }
    
}

void DoorBellWrite(u8 cmd)
{
/*
4: Unpair 
5: SOS Pair Mode
6: Door Bell Pair Mode
7: Leave Pair Mode
*/
    u8 err;

    if(i2cWrite_433RFModule_MCU(cmd)== 0)
    {
        DEBUG_GREEN("Bell Write Fail %d\n",cmd);
        return ;
    }
    switch(cmd)
    {
        case 5:
        case 6:
            DoorBellFlagGrp = OSFlagCreate(0x00000000,&err);
            break;
    }
}

#endif

#if (TOUCH_KEY == TOUCH_KEY_BS83B12)
    #if ((HW_BOARD_OPTION == MR8200_RX_RDI)||(HW_BOARD_OPTION == MR8200_RX_RDI_M900)||\
        (HW_BOARD_OPTION == MR8120_RX_RDI_M713)||(HW_BOARD_OPTION == MR8200_RX_RDI_M902)||\
        (HW_BOARD_OPTION == MR8200_RX_RDI_M712)||(HW_BOARD_OPTION == MR8600_RX_RDI_M904D))


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
            else
                return;
        }

        if ((data != 0) &&(lastData == data))
        {
            UiMode = uiGetMenuMode();
            if ((UiMode != SET_NUMBER_MODE)&&(UiMode != PLAYBACK_MENU_MODE))
                return;
            KeyCnt++;

            if (KeyCnt < 5)
                return;
            else
                KeyCnt = 5;

            switch (data)
            {
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
    #elif ((HW_BOARD_OPTION == MR8200_RX_RDI_M721) || (HW_BOARD_OPTION == MR8120_RX_RDI_M724))
    void TouchKeyPolling(void)
    {
        u8 data;
        u8 tempKey;
        u32 UiMode;
        static u8 lastkey=0;
        static u8 lastData=0, taklOn = 0, KeyCnt = 0;
        i2cRead_BS83B12(I2C_BS83B12_RD_SLAV_ADDR, &data);


        if ((data != 0) &&(lastData == data))
        {
            UiMode = uiGetMenuMode();
            if (UiMode != SETUP_MODE) 
                return;
            KeyCnt++;

            if (KeyCnt < 50)
                return;
            else
                KeyCnt = 50;

            switch (data)
            {
                case 5:
                    tempKey=UI_KEY_L_DEL;
                    DEBUG_TIMER("Key UI_KEY_L_DEL \n");
                    break;
                default:
                    return;
            }
            if(lastkey != tempKey)
            {
            uiSentKeyToUi(tempKey);
                lastkey=tempKey;
            }
        }
        else if( (lastData==0) && (data != 0))
        {
            switch(data)
            {
                case 1:
                    tempKey=UI_KEY_RF_CH;
                    DEBUG_TIMER("Key UI_KEY_RF_CH \n");
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
                    tempKey=UI_KEY_ZOOM;
                    DEBUG_TIMER("Key UI_KEY_ZOOM \n");
                    break;
                case 5:
                    tempKey=UI_KEY_DELETE;
                    DEBUG_TIMER("Key UI_KEY_DELETE \n");
                    break;
                case 9:
                    tempKey=UI_KEY_MENU;
                    DEBUG_TIMER("Key UI_KEY_MENU \n");
                    break;

                default:
                    DEBUG_TIMER("Key Default %d \n",data);

            }
            if(tempKey !=0)
            {
                uiSentKeyToUi(tempKey);
                lastkey=tempKey;
            }
        }
        else
            KeyCnt = 0;
        lastData=data;


    }
    #elif (HW_BOARD_OPTION == MR8200_RX_RDI_M920)
    void TouchKeyPolling(void)
    {
        u8 data;
        u8 tempKey;
        u32 UiMode;
        static u8 lastkey=0;
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

#if((HW_BOARD_OPTION == MR8200_RX_RDI_M920)&&(PROJ_OPT == 10))
        if ((data != 0) &&(lastData == data) && (data != 2))
#else   
        if ((data != 0) &&(lastData == data) && (data != 5))
#endif
        {
            UiMode = uiGetMenuMode();
            if (UiMode != SETUP_MODE) 
                return;
            KeyCnt++;

            if (KeyCnt < 50)
                return;
            else
                KeyCnt = 50;

            switch (data)
            {
                case 3:
                    tempKey=UI_KEY_L_DEL;
                    DEBUG_TIMER("Key UI_KEY_L_DEL \n");
                    break;
                default:
                    return;
            }
            if(lastkey != tempKey)
            {
                uiSentKeyToUi(tempKey);
                lastkey=tempKey;
            }
        }
        else if( (lastData==0) && (data != 0))
        {
            switch(data)
            {
                case 1:
                    if (lastData == 0)
                    {
                        tempKey=UI_KEY_QUAD;
                        DEBUG_TIMER("Key UI_KEY_QUAD \n");
                    }
                    break;
                case 2:
                    if (lastData == 0)
                    {
                        tempKey=UI_KEY_ZOOM;
                        DEBUG_TIMER("Key UI_KEY_ZOOM, \n");
                    }
                    break;
                case 3:
                    if (lastData == 0)
                    {
                        tempKey=UI_KEY_DELETE;
                        DEBUG_TIMER("Key UI_KEY_DELETE \n");
                    }
                    break;
                case 4:
                    if (lastData == 0)
                    {
                        tempKey=UI_KEY_MENU;
                        DEBUG_TIMER("Key UI_KEY_MENU \n");
                    }
                    break;
                case 5:
                    taklOn = 1;
                    tempKey=UI_KEY_TALK;
                    DEBUG_TIMER("Key UI_KEY_TALK \n");
                    break;
                case 6:
                    if (lastData == 0)
                    {
                        tempKey=UI_KEY_MOTION;
                        DEBUG_TIMER("Key UI_KEY_MOTION \n");
                    }
                    break;
                case 7:
                    if (lastData == 0)
                    {
                        tempKey=UI_KEY_REC;
                        DEBUG_TIMER("Key UI_KEY_REC \n");
                    }
                    break;
                case 8:
                    if (lastData == 0)
                    {
                        tempKey=UI_KEY_RF_CH;
                        DEBUG_TIMER("Key UI_KEY_RF_CH \n");
                    }
                    break;

                default:
                    DEBUG_TIMER("Key Default %d \n",data);

            }

            if(tempKey !=0)
            {
                uiSentKeyToUi(tempKey);
                lastkey=tempKey;
            }


        }
#if((HW_BOARD_OPTION == MR8200_RX_RDI_M920)&&(PROJ_OPT == 10))
        else if ((data != 0) &&(lastData == data) && (data == 2))
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
                case 2:
                    DEBUG_TIMER("Key UI_KEY_ZOOM for Default Cnt !!\n");
                    uiSentKeyToUi(UI_KEY_ZOOM);
                    break;
                default:
                    return;
            }
        }
#else        
        else if ((data != 0) &&(lastData == data) && (data == 5))
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
                case 5:
                    DEBUG_TIMER("Key UI_KEY_TALK for Default Cnt !!\n");
                    uiSentKeyToUi(UI_KEY_TALK);
                    break;
                default:
                    return;
            }
        }
#endif        
        else
            KeyCnt = 0;
        lastData=data;


    }

    #elif (HW_BOARD_OPTION == MR8200_RX_GCT_LCD )
    void TouchKeyPolling(void)
    {
        u16 readKey=0;
        u8 datakey=0;
        u8 checksum=0;
        u8 key;

        //i2cRead_BS83B12_2Byte(I2C_BS83B12_RD_SLAV_ADDR, &readKey);
        i2cRead_BS83B12(I2C_BS83B12_RD_SLAV_ADDR, &datakey);
//        DEBUG_TIMER("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \n");
//        DEBUG_TIMER("%% Touch Key check sum error data: %x checksum: %x %% \n",datakey, checksum);
//        DEBUG_TIMER("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \n");
//        datakey = readKey >> 8;
//        checksum = readKey &0x00ff;
        //DEBUG_TIMER(" readdata %x datakey: %x checksum %x \n",readKey, datakey, checksum);
        #if 0
        if((datakey+0xA5) != checksum)
        {
            DEBUG_TIMER("%% Touch Key check sum error data: %x checksum: %x %% \n",datakey, checksum);

            return;
        }
        #endif
        if(datakey==0)
            return;
        switch(datakey)
        {
            case 0x09:
                key = UI_KEY_MENU;
                break;

            case 0x08:
                key = UI_KEY_PLAY;
                break;

            case 0x07:
                key = UI_KEY_REC;
                break;

            case 0x06:
                key = UI_KEY_RF_QUAD;
                break;

            case 0x05:
                key = UI_KEY_ENTER;
                break;

            case 0x04:
                key = UI_KEY_LEFT;
                break;

            case 0x03:
                key = UI_KEY_DOWN;
                break;

            case 0x02:
                key = UI_KEY_UP;
                break;

            case 0x01:
                key = UI_KEY_RIGHT;
                break;
            default:
                DEBUG_TIMER("%% Undfine Key %% \n");
                return;
                break;

        }
        uiSentKeyToUi(key);

    }

    #elif (HW_BOARD_OPTION == MR8600_RX_JESMAY_LCD)
    void TouchKeyPolling(void)
    {
        u16 readKey=0;
        u8 datakey=0;
        u8 checksum=0;
        u8 key;
        static u8 keycount=0;
        static bool talk_on= FALSE;


        i2cRead_BS83B12(I2C_BS83B12_RD_SLAV_ADDR, &datakey);

        if(talk_on == TRUE)
        {
            if(datakey == 0)
            {
                talk_on=FALSE;
                uiSentKeyToUi(UI_KEY_TALK_OFF);
            }
            else
            {
                return;
            }
        }

        if(datakey >0)
        {
            keycount++;
            switch (datakey)
            {
                case 0x08:
                    key= UI_KEY_MENU;
                    break;

                case 0x04:
                    key= UI_KEY_ENTER;
                    break;

                case 0x02:
                    key= UI_KEY_RF_QUAD;
                    break;

                case 0x01:
                    key= UI_KEY_UP;
                    break;

                case 0x10:
                    key= UI_KEY_DOWN;
                    break;

                case 0x20:
                    key= UI_KEY_LEFT;
                    break;

                case 0x40:
                    if(MyHandler.MenuMode == VIDEO_MODE)
                    {
                        talk_on= TRUE;
                        key= UI_KEY_TALK;
                    }
                    else
                    {
                        key= UI_KEY_RIGHT;
                    }

                    break;

                default:
                    DEBUG_TIMER("Touch key error %x \n",datakey);
                    key=UI_KEY_NONE;
                    keycount=0;
                    break;
            }

            if(keycount >10)
            {
                keycount=10;
            }


            if((key != UI_KEY_NONE) && (keycount ==1) || (talk_on== TRUE))
            {
                uiSentKeyToUi(key);

            }
        }
        else
        {
            keycount=0;
        }

    }
    #elif(HW_BOARD_OPTION == MR8120_RX_GCT_SC7700)
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
                    //tempKey=UI_KEY_GOTO;
                    taklOn = 1;
                    DEBUG_TIMER("Key UI_KEY_GOTO \n");
                    break;

                case 4:
                    //tempKey=UI_KEY_CONT_RIGHT;
                    DEBUG_TIMER("Key UI_KEY_CONT_RIGHT \n");
                    break;

                case 5:
                    //tempKey=UI_KEY_CONT_DOWN;
                    DEBUG_TIMER("Key UI_KEY_CONT_DOWN \n");
                    break;

                case 7:
                    //tempKey=UI_KEY_CONT_LEFT;
                    DEBUG_TIMER("Key UI_KEY_CONT_LEFT \n");
                    break;

                case 11:
                    //tempKey=UI_KEY_CONT_UP;
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
                    //tempKey=UI_KEY_SCAN;
                    DEBUG_TIMER("Key UI_KEY_SCAN, \n");
                    break;
                case 3:
                    //tempKey=UI_KEY_QUAD;
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
                    //tempKey=UI_KEY_MOTION;
                    DEBUG_TIMER("Key UI_KEY_MOTION \n");
                    break;
                case 7:
                    tempKey=UI_KEY_LEFT;
                    DEBUG_TIMER("Key UI_KEY_LEFT \n");
                    break;
                case 8:
                    //tempKey=UI_KEY_OK;
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
     #endif//  #if ((HW_BOARD_OPTION == MR8200_RX_RDI)

#elif  (TOUCH_KEY == TOUCH_KEY_MA86P03)

    #if ((HW_BOARD_OPTION == MR8200_RX_TRANWO_LCD) ||(HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593)||\
         (HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505) || (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592)||\
         (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8593RS) || (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8592RS) ||\
         (HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589) || (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N))
    void TouchKeyPolling(void)
    {
        u8 data=0 ;
        u8 key=UI_KEY_NONE;
        i2cReadTouchKey(&data);

        switch(data)
        {
            case 0x81:
                key=UI_KEY_RF_QUAD;
                DEBUG_TIMER("Key UI_KEY_RF_QUAD \n");
                break;

            case 0x82:
                key=UI_KEY_RIGHT;
                DEBUG_TIMER("Key UI_KEY_RIGHT \n");

                break;

            case 0x83:
                key=UI_KEY_ENTER;
                DEBUG_TIMER("Key UI_KEY_ENTER \n");
                break;

            case 0x84:
                key=UI_KEY_UP;
                DEBUG_TIMER("Key UI_KEY_UP \n");
                break;

            case 0x85:
                key=UI_KEY_DOWN;
                DEBUG_TIMER("Key UI_KEY_DOWN \n");
                break;

            case 0x86:
                key=UI_KEY_LEFT;
                DEBUG_TIMER("Key UI_KEY_LEFT \n");
                break;

            case 0x87:
                key=UI_KEY_MENU;
                DEBUG_TIMER("Key UI_KEY_MENU \n");
                break;

            case 0x88:
                key=UI_KEY_REC;
                DEBUG_TIMER("Key UI_KEY_REC \n");
                break;
        }
        if(key != UI_KEY_NONE)
            uiSentKeyToUi(key);
    }
    #endif

#elif(TOUCH_KEY == TOUCH_KEY_CBM7320)

    void TouchKeyPolling(void)
    {
        u16 readKey=0;
        u8 datakey=0,checksum=0;
        static u8 LastKey =0;
        u8 key,level = 0;

        gpioGetLevel(GPIO_GROUP_TOUCH_PANNEL_INT,GPIO_PIN_TOUCH_PANNEL_INT,&level);
        if (level)
            i2cRead_CBM7320_Manual(&datakey);
        else
            LastKey = 0 ;
        //i2cRead_BS83B12_2Byte(I2C_BS83B12_RD_SLAV_ADDR, &readKey);
//        DEBUG_TIMER("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \n");
//        DEBUG_TIMER("%% Touch Key check sum error data: %x checksum: %x %% \n",datakey, checksum);
//        DEBUG_TIMER("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% \n");
//        datakey = readKey >> 8;
//        checksum = readKey &0x00ff;
        //DEBUG_TIMER(" readdata %x datakey: %x checksum %x \n",readKey, datakey, checksum);
        #if 0
        if((datakey+0xA5) != checksum)
        {
            DEBUG_TIMER("%% Touch Key check sum error data: %x checksum: %x %% \n",datakey, checksum);

            return;
        }
        #endif
        
        if((datakey==0) || (datakey==LastKey))
            return;
        
        DEBUG_GREEN("Key %d \n",datakey);
        switch(datakey)
        {
            case 0x01:
                key = UI_KEY_LCD_BL;
                DEBUG_GREEN("Key UI_KEY_LCD_BL\n");
                break;

            case 0x02:
                key = UI_KEY_TALK;
                DEBUG_GREEN("Key UI_KEY_TALK\n");
                break;

            case 0x03:
                key = UI_KEY_RF_QUAD;
                DEBUG_GREEN("Key UI_KEY_RF_QUAD\n");
                break;

            case 0x0B:
                key = UI_KEY_LEFT;
                DEBUG_GREEN("Key UI_KEY_LEFT\n");
                break;

            case 0x0A:
                key = UI_KEY_RIGHT;
                DEBUG_GREEN("Key UI_KEY_RIGHT\n");
                break;

            case 0x09:
                key = UI_KEY_ENTER;
                DEBUG_GREEN("Key UI_KEY_ENTER\n");
                break;

            case 0x08:
                key = UI_KEY_UP;
                DEBUG_GREEN("Key UI_KEY_UP\n");
                break;

            case 0x07:
                key = UI_KEY_DOWN;
                DEBUG_GREEN("Key UI_KEY_DOWN\n");
                break;

            case 0x06:
                key = UI_KEY_MENU;
                DEBUG_GREEN("Key UI_KEY_MENU\n");
                break;

            case 0x05:
                key = UI_KEY_REC;
                DEBUG_GREEN("Key UI_KEY_REC\n");
                break;

            case 0x04:
                key = UI_KEY_PLAY;
                DEBUG_GREEN("Key UI_KEY_PLAY\n");
                break;
            default:
                DEBUG_TIMER("%% Undfine Key %% \n");
                return;
                break;

        }
        LastKey = datakey;
        uiSentKeyToUi(key);

    }



#endif  //  #if(TOUCH_KEY == TOUCH_KEY_BS83B12)

//beep_on_time,u16 beep_off_time u16 beep_delay are in unit ms
void Beep_function(u8 beep_count,u8 beep_clock_div,u16 beep_on_time,u16 beep_off_time,u16 beep_delay,u8 flags)
{
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
	u32 timeout_val;
	if (!OSPWM_task_flags)
	{
		OSPWM_task_flags=OSFlagCreate(PWM_ALARM_READY,0);
	}

	if (flags&PWM_BEEP_FORCE_START_FLAG)
	{
		OS_ENTER_CRITICAL();
		if ((!PWM_is_On)&&(PWM_alarm_counter==0))
			PWM_alarm_ready=1;
		Timer3IntEna=0;
		Timer3Ctrl=0;
		Gpio0Ena|=((1<<1));
		OS_EXIT_CRITICAL();
	}
	if ((PWM_alarm_ready)&&(gBeep_pause_width != 0))/*If gBeep_pause_width is zero ,PWM no pluse*/
	{
		gbeep_clock_div=beep_clock_div;
		gbeep_on_time_counter=(beep_on_time*(SYS_CPU_CLK_FREQ/2000)>>8)/(gbeep_clock_div+1);
		gbeep_off_time_counter=(beep_off_time*(SYS_CPU_CLK_FREQ/2000)>>8)/(gbeep_clock_div+1);
		gbeep_delay_time_counter=(beep_delay*(SYS_CPU_CLK_FREQ/2000)>>8)/(gbeep_clock_div+1);
		printf("\nPWM ON=%x,Off=%x,delay=%x\n",gbeep_on_time_counter,gbeep_off_time_counter,gbeep_delay_time_counter);
		PWM_alarm_counter=beep_count-1;
		PWM_is_On=1;
		PWM_alarm_ready=0;
        #if 0
		OSFlagAnd (OSPWM_task_flags,(~PWM_ALARM_READY) );
        #endif
        OS_ENTER_CRITICAL();
		UI_BEEP_ON;
        OS_EXIT_CRITICAL();

        #if 0
		if (flags&PWM_BEEP_WAIT_UNTIL_FINISH_FLAG)
		{
			timeout_val=(beep_on_time+beep_off_time)*beep_count+beep_delay+50;
			timeout_val=timeout_val*OS_TICKS_PER_MS;
			OSFlagPend (OSPWM_task_flags,PWM_ALARM_READY, OS_FLAG_WAIT_SET_ANY,timeout_val,0);
		}
        #endif
	}
}

void PWM_ISR_handler(void)
{
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    u8  err;

//#if (HW_BOARD_OPTION == MR8211_ZINWELL)
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
#elif MOTOR_EN
    #if((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP))
    #else
    MotorSetting(MotorStatusH, MotorStatusV);
    timer3Setting();
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
            PWM_alarm_ready=1;
        #if 0
            OSFlagPostINT (OSPWM_task_flags, PWM_ALARM_READY);
        #endif
            UI_BEEP_STOP;
        }
    }
#endif

}


