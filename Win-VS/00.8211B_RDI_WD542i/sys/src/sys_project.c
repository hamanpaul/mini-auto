/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    sys.c

Abstract:

    The routines of system control.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "board.h"
#include "task.h"
#include "sysapi.h"
#include "sys.h"
#include "usbapi.h"
#include "adcapi.h"
#include "uiapi.h"
#include "Iduapi.h"
#include "gpioapi.h"
#include "ispapi.h"
#include "iisapi.h"
#include "siuapi.h"
#include "spiapi.h"
#include "fsapi.h"
#include "dcfapi.h"
#include "asfapi.h"
#include "jpegapi.h"
#include "timerapi.h"
#include "uartapi.h"
#include "isuapi.h"
#include "ipuapi.h"
#include "mpeg4api.h"
#include "rfiuapi.h"
#include "../ui/inc/ui.h"
#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
//#if IS_COMMAX_DOORPHONE
#if IS_COMMAX_DOORPHONE || (HW_BOARD_OPTION == MR6730_AFN)	
    #include "../../ui/inc/MainFlow.h"  // UI_gotoStandbyMode
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
u8 pwm2st = 0;
s32 CaptureVideoResult;     // 0 - Failure, 1 - Success.
#if INSERT_NOSIGNAL_FRAME
u8 Record_flag[MULTI_CHANNEL_MAX] = {0,0,0,0}; // 0 - OK, 1 - NG.
#endif

#if MULTI_CHANNEL_VIDEO_REC
OS_STK          sysSubTaskStack0[SYS_TASK_STACK_SIZE]; /* Stack of task sysCaptureVideoSubTask() */
OS_STK          sysSubTaskStack1[SYS_TASK_STACK_SIZE]; /* Stack of task sysCaptureVideoSubTask() */
OS_STK          sysSubTaskStack2[SYS_TASK_STACK_SIZE]; /* Stack of task sysCaptureVideoSubTask() */
OS_STK          sysSubTaskStack3[SYS_TASK_STACK_SIZE]; /* Stack of task sysCaptureVideoSubTask() */
OS_FLAG_GRP     *gSysSubReadyFlagGrp;
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
OS_FLAG_GRP     *gRfRxVideoPackerSubReadyFlagGrp;
#endif
#endif


/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern u8 uiMenuVideoSizeSetting;
extern u8 uiMenuEnable;
extern BOOLEAN MemoryFullFlag;
extern u8 configsalix;
extern u8 osdYShift;
extern u8 siuOpMode; //Lucian: 用於CapturePreviewImg();
extern u8 iconflag[UIACTIONNUM];
extern u8 system_busy_flag;
extern u8 ucasfWritetingSD;
extern u8  MonitorToStandby;
extern OS_EVENT* general_MboxEvt;
extern u32 sys_frequency;
extern u8  VideoClipOnTV;
extern s32 ZoomFactorBackup;
extern u8 Main_Init_Ready;
#if((HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M721) && (PROJ_OPT == 5)) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 8))) //Sean: 20170621 add. //20171208 add M732HT
extern u8 FW_UPGRAD_flag;
#endif

#if MOTIONDETEC_ENA
  extern u8 MotionDetect_en;
#endif

#if HW_MD_SUPPORT
  extern u8 MotionDetect_en;
#endif


extern u32 mpeg4Width;
#if VMDSW
extern u32 VMD_CNT;
#endif

// rfiu
extern VIDEO_BUF_MNG rfiuRxVideoBufMng[MAX_RFIU_UNIT][VIDEO_BUF_NUM];
extern u32 rfiuRxVideoBufMngWriteIdx[MAX_RFIU_UNIT];
extern IIS_BUF_MNG rfiuRxIIsSounBufMng[MAX_RFIU_UNIT][IIS_BUF_NUM];
extern u32 rfiuRxIIsSounBufMngWriteIdx[MAX_RFIU_UNIT];
extern u32 rfiuAudioBufPlay_idx[MAX_RFIU_UNIT];
extern u32 rfiuAudioBufFill_idx[MAX_RFIU_UNIT];
extern u8 *rfiuMainAudioPlayDMANextBuf[IISPLAY_BUF_NUM];
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];

/*
 **********************************************************************************************************
 * External Functions
 **********************************************************************************************************
 */
extern s32 sysCaptureImage_Burst_OnPreview(s32 ZoomFactor, u8 BurstNum,u8 ScalingFactor);
extern s32 sysCaptureImage_One_OnPreview_SIU(s32 ZoomFactor);
extern s32 sysCaptureImage_One_OnPreview_CIU1(s32 ZoomFactor);
extern s32 sysCaptureImage_One_OnPreview_CIU2(s32 ZoomFactor);
//extern s32 sysCaptureImage_One_OnPreview_CIU3(s32 ZoomFactor);


extern s32 iduSwitchPreview_TV(int src_W,int src_H);
extern void osdDrawISPStatus(s8 status);
extern s32 ispUpdate_UILIB(void);
extern s32 sysGetDiskFree(s32);
extern s32 sysDoWavePlay(s32 mode);
extern char Write_protet(void);
extern void IduVideo_ClearPKBuf(u8 bufinx);
extern s32 asfCaptureVideo(s32 ZoomFactor, u32 Mode);
#if (HW_BOARD_OPTION==ES_LIGHTING)
extern s32 ispUpdateWaveFile(void);
extern void adcDacOutputZero(void);
extern void uiInitDAC_Play(void);
#endif

#if (HW_BOARD_OPTION==MR6730_AFN)	
#if (Get_sametime == 1)
extern u32 RTC_Time_To_Second(RTC_DATE_TIME *Source);
#endif
#endif

#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
extern int rfiuSetSensorLightCtrl_TX(u8 setting);
#endif

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
s32 sysDoRecord(s32 mode);
s32 sysDoWavePlay(s32 mode);


/*-------------------- event function -------------------*/
/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */

u8 sysProjectSysInit(u8 Step)
{
    switch(Step)
    {
        case 1:
        //------Lucian: Set SD Card/SDRAM PAD Driving-------//
                //SYS_DDR_PADCTL1= (SYS_DDR_PADCTL1 & (~0xe0000000)) | 0x20000000; //no use in A1016
                //SYS_DDR_PADCTL1= (SYS_DDR_PADCTL1 & (~0x0000000e)) | 0x00000002; //SDAM:4 mA
#if IS_COMMAX_DOORPHONE
				SYS_PAD_SR_CTL = 0; //Slew Rate Toby 130730
#endif
            #if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
                SYS_DRV_E4_CTL |= 0x01;
                SYS_DRV_E8_CTL |= 0x01;

                //Set SD PAD driving, sdclk: 12 mA, data: 8 mA
                SYS_DRV_E4_CTL = (SYS_DRV_E4_CTL & (~0x60) ) | 0x40;
                SYS_DRV_E8_CTL = (SYS_DRV_E8_CTL & (~0x60) ) | 0x20;
            #elif (HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
                SYS_DRV_E4_CTL |= 0x80;
                SYS_DRV_E8_CTL |= 0x80;
            #endif
            #if(HW_BOARD_OPTION == MR8200_RX_TRANWO_D2505)
                SYS_PAD_SR_CTL = SYS_PAD_SR_CTL & (~0x100);
            #endif
			
			#if(HW_BOARD_OPTION  == MR8100_GCT_VM9710) // senser
                SYS_DRV_E4_CTL = (SYS_DRV_E4_CTL & ~0x1000);
                SYS_DRV_E8_CTL = (SYS_DRV_E8_CTL & ~0x1000);
			#endif
			
			#if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)) // senser
                SYS_DRV_E4_CTL = (SYS_DRV_E4_CTL & ~0x1000);
                SYS_DRV_E8_CTL = (SYS_DRV_E8_CTL & ~0x1000);
				SYS_PAD_SR_CTL = 0; //Slew Rate 
            #endif
            #if ((SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1))
                SYS_PAD_SR_CTL &= (~0x20000);
                SYS_DRV_E4_CTL |= 0x20000;
            #endif
            break;

        default:
            break;
    }
    return 0;
}



u8 sysProjectExifWrite(u8 Step)
{
    switch (Step)
    {
        case 1:
            //Lucian: 低電關機下:拍照模式,寫卡是在background task內做. 須等到寫卡結束後,才能Power-off.
            //osdDrawLowBattery() 內 check 狀態, 此為舊式寫法. 用於下列project 內.

            break;

        case 2: /*for Factory tool in some project*/

            break;

        default:
            break;
    }
    return 0;
}

u8 sysProjectPreviewInit(u8 Step)
{
    switch(Step)
    {
        case SYS_PREVIEW_INIT_RESET: /*preview reset*/
            sysPreviewReset(0);
            break;

        case SYS_PREVIEW_INIT_OSDDRAWICON: /*draw preview OSD Icon*/
            #if (UI_PREVIEW_OSD == 1)
	        	#if( HW_BOARD_OPTION == MR6730_AFN)	
		  		#else			
                uiOSDPreviewInit();
				#endif 
            #endif
            break;

        default:
            break;
    }
    return 0;
}

u8 sysProjectPreviewReset(u8 Step)
{
    u32     sys_ctl0_status;

    switch(Step)
    {
        case SYS_PREVIEW_RESET_PWRMAG:
		  #if ( (Sensor_OPTION  == Sensor_MI1320_YUV601) || (Sensor_OPTION  == Sensor_CCIR601) || (Sensor_OPTION  == Sensor_CCIR656) || (Sensor_OPTION  == Sensor_OV7725_YUV601) || (Sensor_OPTION  == Sensor_OV7740_YUV601) || (Sensor_OPTION == Sensor_MI9V136_YUV601) || (Sensor_OPTION == Sensor_PC1089_YUV601) || (Sensor_OPTION == Sensor_OV2643_YUV601) || (Sensor_OPTION == Sensor_MT9M131_YUV601) || (Sensor_OPTION == Sensor_HM5065_YUV601) || (Sensor_OPTION == Sensor_HM1375_YUV601) || (Sensor_OPTION == Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_PO3100K_YUV601) )
            sys_ctl0_status 	= SYS_CTL0;
            sys_ctl0_status    |= SYS_CTL0_SIU_CKEN    |	   //SIU enable
            					  SYS_CTL0_ISU_CKEN    |	   //ISU enable
                              #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                                  SYS_CTL0_SCUP_CKEN   |
                                  SYS_CTL0_CIU_CKEN |
                                  SYS_CTL0_RF1013_CKEN |   //yugo
                              #elif( (CHIP_OPTION == CHIP_A1016A) ||(CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
                                  SYS_CTL0_SCUP_CKEN   |
                                  SYS_CTL0_CIU_CKEN |
                                  SYS_CTL0_RF1013_CKEN |   //yugo
                                  SYS_CTL0_MD_CKEN     |
                              #endif
            					  SYS_CTL0_GPIO1_CKEN  |
            					  SYS_CTL0_GPIO2_CKEN  |
            					  SYS_CTL0_GPIO3_CKEN  |    //GPIO3 enable,Lucian: DH500使用GPIO3 需打開
            					  SYS_CTL0_TIMER4_CKEN |	   //PWM enable
            					  SYS_CTL0_IIS_CKEN    |		//IIS enable
                              #if CIU_SPLITER
                                  SYS_CTL0_CIU2_CKEN   |   // Amon (140613)
                              #endif
    					      #if IS_COMMAX_DOORPHONE
    						      SYS_CTL0_TVE_CKEN;
    					      #else
            					  SYS_CTL0_TVE_CKEN |
            					  SYS_CTL0_SER_MCKEN;		   //sensor master clock output enable
		       			      #endif

            sys_ctl0_status    &= ~SYS_CTL0_JPEG_CKEN  &	//JPEG disable
                              #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                                  ~SYS_CTL0_H264_CKEN  &	//H264 disable
                              #else
            					  ~SYS_CTL0_MPEG4_CKEN &	//MPEG disable
                              #endif
            					  ~SYS_CTL0_HIU_CKEN   &	  //HIU disable.
            					  ~SYS_CTL0_IPU_CKEN   &
            					  ~SYS_CTL0_SRAM_CKEN  &
                          #if (FLASH_OPTION < FLASH_NAND_9001_NORMAL)
            					  ~SYS_CTL0_NAND_CKEN  &
            			  #endif

            			  #if USE_BUILD_IN_RTC	  //use build-in RTC
            			  #else
            					  ~SYS_CTL0_RTC_CKEN&
            			  #endif
                                   0xffffffff;


     //   	if (gucUsbPlugInStat & (USB_PLUGIN_STAT_NONE|USB_PLUGIN_STAT_ONLY_POWER))	/* Can only disable usb clk if usb mass storage doesn't work. */
            #if (USB2WIFI_SUPPORT == 0)
            sys_ctl0_status		&= ~SYS_CTL0_USB_CKEN;
            #endif

            SYS_CTL0			= sys_ctl0_status;
           #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
			SYS_CTL0_EXT        = SYS_CTL0_EXT1_RF1013_CKEN | SYS_CTL0_EXT2_RF1013_CKEN ; 
                #if CIU_SPLITER 
                SYS_CTL0_EXT |= SYS_CTL0_EXT_CIU3_CKEN | SYS_CTL0_EXT_CIU4_CKEN; // Amon (140613)
                #endif
           #endif
        #else
            sys_ctl0_status     = SYS_CTL0;

            sys_ctl0_status    |= SYS_CTL0_SIU_CKEN    |       //SIU enable
                                  SYS_CTL0_IPU_CKEN    |       //IPU enable
                                  SYS_CTL0_SRAM_CKEN   |       //IPU SRAM enable
                                  SYS_CTL0_ISU_CKEN    |       //ISU enable
                              #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B )|| (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                                  SYS_CTL0_SCUP_CKEN   |
                                  SYS_CTL0_CIU_CKEN |
                              #elif( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
                                  SYS_CTL0_SCUP_CKEN   |
                                  SYS_CTL0_CIU_CKEN |
                                  SYS_CTL0_CIU2_CKEN |
                              #endif
                                  SYS_CTL0_GPIO1_CKEN  |
                                  SYS_CTL0_GPIO2_CKEN  |
                                  SYS_CTL0_GPIO3_CKEN  |       //GPIO3 enable,Lucian: DH500使用GPIO3 需打開
                                  SYS_CTL0_TIMER4_CKEN |       //PWM enable
                                  SYS_CTL0_IIS_CKEN |          //IIS enable
                              #if IS_COMMAX_DOORPHONE
                				  SYS_CTL0_TVE_CKEN ;          //sensor master clock output enable
                			  #else
                                  SYS_CTL0_TVE_CKEN |
                                  SYS_CTL0_SER_MCKEN;          //sensor master clock output enable
                              #endif

            sys_ctl0_status    &= ~SYS_CTL0_JPEG_CKEN  &    //JPEG disable
                              #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                                  ~SYS_CTL0_H264_CKEN  &	//H264 disable
                              #else
                                  ~SYS_CTL0_MPEG4_CKEN &    //MPEG disable
                              #endif
                                  ~SYS_CTL0_HIU_CKEN &      //HIU disable.
                          #if USE_BUILD_IN_RTC    //use build-in RTC
                          #else
                                  ~SYS_CTL0_RTC_CKEN&
                          #endif
                                  0xffffffff;


    		//if (gucUsbPlugInStat & (USB_PLUGIN_STAT_NONE|USB_PLUGIN_STAT_ONLY_POWER))	/* Can only disable usb clk if usb mass storage doesn't work. */
            #if (USB2WIFI_SUPPORT == 0)
    			sys_ctl0_status		&= ~SYS_CTL0_USB_CKEN;
            #endif

            SYS_CTL0            = sys_ctl0_status;
		#endif
            break;

        case SYS_PREVIEW_RESET_TV_VIDEOOFF:

            break;

        default:
            break;
    }
    return 0;
}

u8 sysProjectPreviewStop(u8 Step)
{
    u32     sys_ctl0_status;

    switch(Step)
    {
        case 1:

                // disable unused module for reduce power consumption
                sys_ctl0_status     = SYS_CTL0;
                sys_ctl0_status    |= SYS_CTL0_SIU_CKEN    |       //SIU enable
                                      SYS_CTL0_IPU_CKEN    |       //IPU enable
                                      SYS_CTL0_SRAM_CKEN   |       //IPU SRAM enable
                                      SYS_CTL0_ISU_CKEN    |       //ISU enable
                                   #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                                    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                                    (CHIP_OPTION == CHIP_A1026A))
                                      SYS_CTL0_SCUP_CKEN   |
                                   #endif
                                      SYS_CTL0_GPIO1_CKEN  |
                                      SYS_CTL0_GPIO2_CKEN  |
                                      SYS_CTL0_GPIO3_CKEN  |    //GPIO3 enable,Lucian: DH500使用GPIO3 需打開
                                      SYS_CTL0_TIMER4_CKEN |       //PWM enable

                                   #if IS_COMMAX_DOORPHONE
				      SYS_CTL0_IIS_CKEN    ;       //IIS enable
                                   #else
                                      SYS_CTL0_IIS_CKEN    |       //IIS enable
                                      SYS_CTL0_SER_MCKEN;          //sensor master clock output enable
   				   #endif

                sys_ctl0_status    &= ~SYS_CTL0_JPEG_CKEN  &    //JPEG disable
                              #if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
                                      ~SYS_CTL0_H264_CKEN  &	//H264 disable
                              #else
                                      ~SYS_CTL0_MPEG4_CKEN &    //MPEG disable
                              #endif
                                      ~SYS_CTL0_HIU_CKEN &      //HIU disable.
                              #if USE_BUILD_IN_RTC    //use build-in RTC
                              #else
                                      ~SYS_CTL0_RTC_CKEN&
                              #endif

                                      0xffffffff;


                    SYS_CTL0            = sys_ctl0_status;

            break;

        case 2: /*draw preview icon*/
                if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
                   iduSwitchPreview_TV(640,480);
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576))
                {
                   if(sysTVinFormat == TV_IN_NTSC)
                      iduSwitchPreview_TV(704,480);
                   else
                      iduSwitchPreview_TV(704,576);
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x288))
                {
                   if(sysTVinFormat == TV_IN_NTSC)
                      iduSwitchPreview_TV(352,240);
                   else
                      iduSwitchPreview_TV(352,288);
                }
                else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576))
                {
                   if(sysTVinFormat == TV_IN_NTSC)
                      iduSwitchPreview_TV(720,480);
                   else
                      iduSwitchPreview_TV(720,576);
                }
                else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                {
                   iduSwitchPreview_TV(1280,720);
                }
                else
                   iduSwitchPreview_TV(640,480);

            break;

        default:
            break;
    }
    return 0;
}

u8 sysProjectSnapshot(u8 Step)
{
    switch(Step)
    {
        case 1:
            break;

        default:
            break;
    }
    return 0;
}

#if IS_COMMAX_DOORPHONE
    extern s8 OverwriteStringEnable;    // ui_flow_project.c
#endif

u8 sysProjectSnapshotOnPreview(u8 Step, s32 ScalingFactor)
{
    int i;

    switch(Step)
    {
        case 1: /*for some project to do self timer*/
            #if IS_COMMAX_DOORPHONE
                // enable date-time stamp
                //OverwriteStringEnable = 1;
                //uiDrawTimeOnVideoClip(1);
            #endif
            break;

        case 2:

                #if MULTI_CHANNEL_SUPPORT
                     switch(sysVideoInCHsel)
                     {
                         case 0:
                            sysCaptureImage_One_OnPreview_SIU(sysPreviewZoomFactor);
                            break;
                         case 1:
                            //for(i=0;i<1000;i++)
                        #if(CHIP_OPTION >= CHIP_A1018A)
                            sysCaptureImage_One_OnPreview420_CIU1(sysPreviewZoomFactor);
                        #else
                            sysCaptureImage_One_OnPreview_CIU1(sysPreviewZoomFactor);
                        #endif
                            break;
                         case 2:
                            //for(i=0;i<1000;i++)
                        #if(CHIP_OPTION >= CHIP_A1018A)
                            sysCaptureImage_One_OnPreview420_CIU2(sysPreviewZoomFactor);
                        #else
                            sysCaptureImage_One_OnPreview_CIU2(sysPreviewZoomFactor);
                        #endif
                            break;
                     }
                #else
			         sysCaptureImage_One_OnPreview_SIU(sysPreviewZoomFactor);
                #endif

            break;


        default:
            #if IS_COMMAX_DOORPHONE
                // disable date-time stamp
                //OverwriteStringEnable = 0;
                //uiDrawTimeOnVideoClip(1);
            #endif
            break;
    }
    return 0;
}

u8 sysProjectVideoCaptureRoot(u8 Step)
{
    switch(Step)
    {
        case 1: /*turn on led for write protet*/

            break;

        case 2: /*turn on led for start cpature*/
            break;

        case 3: /*after capture video*/

            #if MULTI_CHANNEL_SUPPORT
              #if(MULTI_CHANNEL_SEL & 0x01)
                sysPreviewReset(sysPreviewZoomFactor);
              #endif

              #if (MULTI_CHANNEL_SEL & 0x02)
                sysCiu_1_PreviewReset(sysPreviewZoomFactor);
              #endif

              #if(MULTI_CHANNEL_SEL & 0x04)
                sysCiu_2_PreviewReset(sysPreviewZoomFactor);
              #endif

              #if(MULTI_CHANNEL_SEL & 0x08)
                sysCiu_3_PreviewReset(sysPreviewZoomFactor);
              #endif

              #if(MULTI_CHANNEL_SEL & 0x10)
                sysCiu_4_PreviewReset(sysPreviewZoomFactor);
              #endif
        	#else
                sysPreviewReset(sysPreviewZoomFactor);/*BJ 0530 S*/
        	#endif
            break;

        case 4: /*draw preview icon*/
            #if (UI_PREVIEW_OSD == 1)
                uiOSDPreviewInit();
            #endif
            break;

        default:
            break;
    }
    return 0;
}

u8 sysProjectPowerOff(u8 Step)
{
    switch(Step)
    {
        case 1: /*Check if low battery*/

            break;

        case 2: /*Disable LCD Backlight*/



            break;

        case 3: /*set gpio for power off*/


        default:
            break;
    }
    return 0;
}

u8 sysProjectMacro(u8 Step)
{
    switch(Step)
    {
        case 1: /*Set Macro*/

            break;

        default:
            break;
    }
    return 0;
}

u8 sysProjectSDCD_IN(u8 Step)
{
    u8 ucLevel;
#if ((HW_BOARD_OPTION == MR8200_RX_COMMAX) || (HW_BOARD_OPTION == MR8200_RX_COMMAX_BOX) || IS_COMMAX_DOORPHONE || (HW_BOARD_OPTION == MR8200_RX_JIT)||\
    (HW_BOARD_OPTION == MR8200_RX_DB2) || (HW_BOARD_OPTION == MR8200_RX_DB3)|| (HW_BOARD_OPTION == MR8120_RX_HECHI)||\
    (UI_VERSION == UI_VERSION_TRANWO) ||(HW_BOARD_OPTION == MR8600_RX_JESMAY_LCD)||\
    (HW_BOARD_OPTION == MR8200_RX_GCT_LCD)||(HW_BOARD_OPTION == MR8200_RX_JIT_BOX)||(UI_VERSION == UI_VERSION_RDI)||(HW_BOARD_OPTION == MR8200_RX_JIT_BOX_AV)||\
    (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM719)||(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM720)||(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902)||\
    (HW_BOARD_OPTION == MR8120_RX_MAYON_MWM011)||(HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014)||\
    (HW_BOARD_OPTION == MR8120_RX_MAYON_MWM710)||(UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3)||\
    (HW_BOARD_OPTION == MR8120_RX_JIT_LCD) ||(HW_BOARD_OPTION == MR8120_RX_JIT_BOX) ||\
    (HW_BOARD_OPTION == MR8120_RX_JIT_M703SW4) || (HW_BOARD_OPTION == MR8120_RX_JIT_D808SW3) ||\
    (HW_BOARD_OPTION == MR8200_RX_JIT_D808SN4) || (HW_BOARD_OPTION == MR8200_RX_JIT_M703SN4))
    static u8 uISPFlag = 0;
#else
    static u8 uISPFlag = 1;
#endif
    s32 isp_return;
    u8  ledon=0, i, temp, err;
	FS_DISKFREE_T   *diskInfo;
    u32             free_size;
    u32             bytes_per_cluster;

    /*avoid warning message*/
    if (uISPFlag || isp_return || i || ledon || temp || err)
    {}
    #if (((UI_VERSION == UI_VERSION_TRANWO) || (UI_VERSION == UI_VERSION_RDI_4)) && (SD_AUTO_UPGRADE == 1))
        uISPFlag = 1;
    #endif
    switch(Step)
    {
        case 1: /*Set busy led*/

            break;

        case 2: /*set sd led*/

            break;

        case 3:

            break;

        case 4: /*when sd error set led*/

            break;

        case 5: /*display fat error message*/
            #if (UI_PREVIEW_OSD == 1)
                uiOSDPreviewInit();
 
				#if(HW_BOARD_OPTION == MR6730_AFN)
					uiOSDIconColorByXY(OSD_ICON_WARNING ,152 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
					osdDrawMessage(MSG_FAT_HEADER_ERROR, CENTERED, 140+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
					osdDrawMessage(MSG_RE_FORMATE, CENTERED, 160+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
				#else
                    uiOSDIconColorByXY(OSD_ICON_WARNING ,152 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
                    osdDrawMessage(MSG_FAT_HEADER_ERROR, CENTERED, 116+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
                    osdDrawMessage(MSG_RE_FORMATE, CENTERED, 136+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
				#endif
                    OSTimeDly(20);
                    uiClearOSDBuf(2);

            #endif
            break;

        case 6: /*Sd FAT error and USB plug in*/

            break;

        case 7: /*USBIN ,change to next UI mode and set led*/

                MyHandler.MenuMode = VIDEO_MODE;
                uiMenuOSDReset();
                uiMenuEnterPreview(0);      /* return to preview */

            break;

        case 8: /*set led when USBIN*/

            break;

        case 9: /*go to format fail, change to next UI mode*/

                MyHandler.MenuMode = VIDEO_MODE;
                uiMenuOSDReset();
                uiMenuEnterPreview(0);


            break;

        case 10:    /*show format fail message*/

                uiOSDIconColorByXY(OSD_ICON_WARNING ,152 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
                osdDrawMessage(MSG_CARD_STILL_FAIL, CENTERED, 116+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
                osdDrawMessage(MSG_CHANGE_SD_CARD, CENTERED, 136+osdYShift/2, OSD_Blk2, 0xC0, 0x00);

            break;

        case 11:    /*set led when go to format fail*/

            break;

        case 12:    /*go to format success, set UI mode*/

                uiMenuOSDReset();
                uiMenuEnterPreview(0);

            break;

        case 13:    /*show sd unknow error message*/


                uiOSDIconColorByXY(OSD_ICON_WARNING ,152 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
                osdDrawMessage(MSG_FS_OPERATION_ERROR, CENTERED, 116+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
                osdDrawMessage(MSG_CHECK_WRITE_PROTECT, CENTERED, 136+osdYShift/2, OSD_Blk2, 0xC0, 0x00);

                osdDrawPreviewIcon();

            break;

        case 14:    /*set unknow error led*/

            break;

        case 15:    /*no error when check sd*/

            break;

        case 16:    /*after make bin*/
            break;

        case 17:    /*Update firmware*/

                if (uISPFlag==1)
                {
                    #if (SD_AUTO_UPGRADE == 1)
                    if (Main_Init_Ready == 0)
                        break;
                    if (dcfOpen((signed char*)ispUSBFileName, "rb") == NULL)
                        break;
                        
					#if((HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M721) && (PROJ_OPT == 5)) || ((HW_BOARD_OPTION == MR8200_RX_RDI_M706) && (PROJ_OPT == 8))) //Sean: 20170621 add. //20171208 add M732HT
					FW_UPGRAD_flag = 1;
					#endif
                    uiFlowEnterMenuMode(SETUP_MODE);
                    IduVideo_ClearPKBuf(0);
                    osdDrawISPStatus(3);
                    for ( i = RFIU_TASK_PRIORITY_HIGH; i < MAIN_TASK_PRIORITY_END; i++)
                    {
                        if ((i == SYSTIMER_TASK_PRIORITY) ||
                            (i == UARTCMD_TASK1_PRIORITY) ||
                            (i == TIMER_TICK_TASK_PRIORITY) ||
                        #if(HOME_RF_SUPPORT)
                            (i == UARTCMD_TASK2_PRIORITY) ||
                        #endif
                            (i == SYS_TASK_PRIORITY))
                            
                        {
                            continue;
                        }
                        DEBUG_UI("UI OSTaskDel %d!\n",i);
                        OSTaskSuspend(i);
                        OSTaskDel(i);
                    }
                    if(!sysUpgradeFW(0))
                    {
                        osdDrawISPStatus(2);
						sysDeadLockMonitor_OFF();
						while(1);
					}
                    osdDrawISPStatus(1);
                    OSTimeDly(100);
                    #if(HW_BOARD_OPTION == MR8200_RX_TRANWO_SMH101_HA)
                        uiOsdDrawUpgradeFinish();
                        sysDeadLockMonitor_OFF();
                        while(1);
                    #else
                        sysForceWDTtoReboot(); 
                    #endif
                    #else
                    #if ((FLASH_OPTION == FLASH_SERIAL_ESMT) || (FLASH_OPTION == FLASH_SERIAL_EON) || (FLASH_OPTION == FLASH_SERIAL_SST)|| (FLASH_OPTION == FLASH_SERIAL_WINBOND))
                        //sysSPI_Disable();
                    #else
                	    sysNAND_Disable();
                    #endif
                    sysSD_Enable();

                    /*disable watch dog when update firmware*/
                    sysDeadLockMonitor_OFF();
        		    #if (MOTIONDETEC_ENA || HW_MD_SUPPORT)
            		    temp=MotionDetect_en;   //暫時關掉motion detection.
            			MotionDetect_en=0;
        	        #endif

        		    if (sysCaptureVideoStart)
                       sysCaptureVideoStop = 1;
					#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
                     (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
                     (CHIP_OPTION == CHIP_A1026A))
					#if (HW_BOARD_OPTION!=MR6730_AFN)
					
                    isp_return=ispUpdateAllload();//usb boot
                    if(isp_return ==0)
                        {
                            ispUpdatebootload();
                            isp_return=ispUpdate(1);		 // Check whether spiFW.bin exists ot not. If exist then update
                        }

					#else//#if (HW_BOARD_OPTION==MR6730_AFN)
						
						#if 0
						DEBUG_SYS("\nIspUpdate...\n");
						if ((isp_return = ispUpdateAllload()) == 0)	  // update f/w from SD card
						{
							   ERR(("F/W update failed"));
						}
						
						DEBUG_SYS("IspUpdate:%d\n",isp_return);

						#else

							isp_return=0;
						
							#if (USE_FW_UPGRADE)
							if (FW_IsInSDCard())//check FW file existence
							{
				
								#if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN)
								if(Main_Init_Ready && (UI_Get_OpMode()!=MODE_Init))
								{//force to goto init mode								
									#if 0//(USE_IDLE_MODE)
									UI_SwitchToMode(MODE_IDLE);	
									while(setUI.PwrCtrl_Phase!=3)
									{
										OSTimeDlyHMSM(0,0,0,50);
									}							
									#endif
									UI_SwitchToMode(MODE_Init);
									while(sysForceWdt2Reboot)
									{
										OSTimeDlyHMSM(0,0,0,50);
									}
								}
								#endif
								
								DEBUG_SYS("UI_TryFwUpgradeFromSD...\n");		
								isp_return=UI_TryFwUpgradeFromSD();	
							}	
							
							#if 1
							//to end this procedure directly
														
								if (isp_return==1)
								{
									uISPFlag=0;
									#if 0
									MACRO_UI_SET_SYSSTAT_BIT_SET(UI_SET_SYSSTAT_BIT_FWUPG_CTRL);	
									#endif
								}


								/*enable watch dog when update firmware finish*/
								sysDeadLockMonitor_ON();
								#if (MOTIONDETEC_ENA || HW_MD_SUPPORT)
								   MotionDetect_en=temp;   //恢復 motion detection 設定.
								#endif
							break;	
							#endif
														
							#endif//#if (USE_FW_UPGRADE)
							
						#endif


					#endif
					#else
                    isp_return=ispUpdate(1);		 // Check whether spiFW.bin exists ot not. If exist then update
                    #endif
                    if (isp_return!=0) 	 // Only need to re-mount SD while return 1 -1 -2
                    {
                        osdDrawISPStatus(isp_return);
                        OSTimeDly(20);
                        uiClearOSDBuf(2);
                        if (isp_return==1)
                            uISPFlag=0;
                    }
                    if (isp_return ==1)     // Only updated UI lib when updated FW sucessfully
                    {

                        uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
                        osdDrawMessage(MSG_CHECK_UI, CENTERED, 110+osdYShift/2, OSD_Blk2, 0xC0, 0x00);

                        isp_return=ispUpdate_UILIB();

                        if (isp_return!=0)      // Only need to re-mount SD while return 1 -1 -2
                        {
                    #if ((FLASH_OPTION == FLASH_SERIAL_ESMT) || (FLASH_OPTION == FLASH_SERIAL_SST)|| (FLASH_OPTION == FLASH_SERIAL_EON) || (FLASH_OPTION == FLASH_SERIAL_WINBOND))
                            //sysSPI_Disable();
                    #else
                			sysNAND_Disable();
                    #endif
                            sysSD_Enable();
                            if (isp_return==1)
                            {

                                uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
                                osdDrawMessage(MSG_UI_UPDATE_PASS, CENTERED, 110+osdYShift/2, OSD_Blk2, 0xC0, 0x00);

                            }
                            else
                            {

                                uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
                                osdDrawMessage(MSG_UI_UPDATE_FAIL, CENTERED, 110+osdYShift/2, OSD_Blk2, 0xC0, 0x00);

                            }

                            if (isp_return==1)
                                uISPFlag=0;
                        }
                        else
                        {

                            uiOSDIconColorByXY(OSD_ICON_WARNING ,138 , 88+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
                            osdDrawMessage(MSG_NO_UI_LIBRARY, CENTERED, 110+osdYShift/2, OSD_Blk2, 0xC0, 0x00);

                        }

                            OSTimeDly(10);

                        /*enable watch dog when update firmware finish*/
                        sysDeadLockMonitor_ON();

                        isp_return=1;




                    }
                    else
                    {
                       /*enable watch dog when update firmware finish*/
                       sysDeadLockMonitor_ON();
                    }
                 #if (MOTIONDETEC_ENA || HW_MD_SUPPORT)
        			MotionDetect_en=temp;   //恢復 motion detection 設定.
        	     #endif
        	     #endif
                }

            break;

        case 18:    /*Check Disk Free Size*/
            sysGetDiskFree(0);
        	diskInfo            = &global_diskInfo;
            bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
            free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit
        #if MULTI_CHANNEL_VIDEO_REC
            if(free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) * MULTI_CHANNEL_MAX) / 1024) //Notice: K-Byte unit
        #else
            if(free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM)) / 1024) //Notice: K-Byte unit
        #endif
	        {
                DEBUG_SYS("Disk Full\n");
                MemoryFullFlag = TRUE;
        		sysProjectDeviceStatus(DEV_SD_FULL);
                #if ((UI_VERSION == UI_VERSION_RDI)||(HW_BOARD_OPTION == MR8200_RX_COMMAX)||\
                    (HW_BOARD_OPTION == MR8200_RX_COMMAX_BOX)||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
                    if (SysOverwriteFlag == FALSE)
                    {
                        osdDrawMemFull(UI_OSD_DRAW);
                    }
                #endif
	        }
    		else
            {
                MemoryFullFlag = FALSE;
				sysProjectDeviceStatus(DEV_SD_NOT_FULL);
            }

            break;

        case 19:    /*Copy APP from Nand to SD for Special purpose*/
            break;

        case 20:    /*clean wait message and draw preview icon*/
		#if (HW_BOARD_OPTION == MR6730_AFN)	
		if(Main_Init_Ready)
		{		
            osdDrawSDCD(0);
            osdDrawPreviewIcon();	
		}
		#else
            osdDrawSDCD(0);
            osdDrawPreviewIcon();
		#endif
            break;

        case 21:    /*SDCD_IN complete and busy LED off*/
        #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
            UI_gotoStandbyMode();
        #endif
	 	#if (HW_BOARD_OPTION == MR6730_AFN)	
		//DEBUG_SYS("---->sysProjectSDCD_IN : not initial mode\n");
		if(Main_Init_Ready)
			UI_gotoPreviewMode();
	 	#endif			
            break;

        default:
            break;
    }
    return 0;
}

u8 sysProjectWhiteLight(u8 Step, u8 On)
{
    switch(Step) //Turn on PWM2
    {
        case 1: /*Set WhiteLight*/

                if (!pwm2st)
                {
                    SYS_CTL0 |= SYS_CTL0_TIMER4_CKEN;
                    Timer4Count = 0x000095f5;
                    Timer4Ctrl = 0x10200008;
                    pwm2st = 1;
                }

                if (On)
                {//On
                    //DEBUG_SYS("Video Light On!\n\r");
                    Timer4Ctrl |= 0x02000000;
                }
                else
                {//Off
                    //DEBUG_SYS("Video Light Off!\n\r");
                    Timer4Ctrl &= ~(0x12000000);
                    pwm2st = 0;
                }

            break;

        default:
            break;
    }
    return 0;

}

u8 sysProjectSDCD_OFF(u8 Step)
{
    switch(Step)
    {
        case 1: /*set off led when sd off*/

            break;

        case 2: /*draw wait message when sd remove message*/

                    osdDrawSDCD(0);

            break;

        case 3: /*draw sd remove message*/
            break;

        case 4: /*draw SD OFF icon */
            osdDrawSDIcon(UI_OSD_CLEAR);
            break;

        case 5: /*Unknown storage memory*/
            break;


    }
    return 0;
}

u8 sysProjectSelfTimer(u8 Step)
{
    u8 i = 0, led = 0;
    u8 tempstring[4];

    /*avoid warning message*/
    if (led || i || tempstring)
    {}
    switch(Step)
    {
        case SYS_SELF_TIMER_DRAW_ICON:

        	    i = SelfTimer;
        	    while (i!=0)
        	    {
        	        sprintf((char*)tempstring,"%d",i);

        	            if (sysTVOutOnFlag)
                            uiOSDASCIIStringByColor(tempstring, 156 , 112 , 2 , 0xc0, 0x00);
        	            else
                            uiOSDASCIIStringByColor(tempstring, 76 , 112 , 2 , 0xc0, 0x00);


        	        OSTimeDly(10);
        	        i--;
        	    }

            break;

        default:
            break;
    }
    return 0;
}

u32 sysProjectCaptureImage(u8 Step)
{
    switch(Step)
    {
        case 1: /*set model name*/
            #if ADDAPP2TOJPEG

                    strcpy((char*)exifApp2Data->ModelName,"MARS(MR9620):????,Ver1.0");

            #endif
            break;

        case 2: /*Display captured frame*/

                if (!sysTVOutOnFlag)
                    iduCapturePrimary(); //Lucian: Display captured frame.

            break;

        case 3:

                return GetJpegImagePixelCount();


        case 4: /*draw OSD*/

            break;
    }
    return 0;
}

u32 sysProjectCaptureImage_Burst_OnPreview(u8 Step, u8 BurstNum, u32 i)
{
    u32 iduWinReg;
#if(HW_BOARD_OPTION==WINNIN_IRDVR)
    static u32 timeStart;
    u32 timeEnd;
#endif

    switch(Step)
    {
        case 1:

                if (sysTVOutOnFlag)
                {
                    IduVideo_ClearPKBuf(0);
                    IduVidBuf0Addr= (u32)PKBuf0;
                #if NEW_IDU_BRI
                    BRI_IADDR_Y = IduVidBuf0Addr;
                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                #endif
                    iduWinReg=IduWinCtrl & 0x0f;
                    IduWinCtrl &= ~0x0000000f;
                }
                else
                {
                    iduWinReg=IduWinCtrl & 0x0f; //Lucian: 插入黑畫面.
                    IduDefBgColor   = 0x00808000;
                    IduWinCtrl &= ~0x0000000f;
                }
                return iduWinReg;


        case 2:

            break;

        case 3:

            break;

        case 4:

            break;

        case 5:
            #if ADDAPP2TOJPEG

                strcpy((char*)exifApp2Data->ModelName,"MARS(PA9002):????,Ver1.0");

            #endif
            break;

        case 6:

                IduVideo_ClearPKBuf(0);

            break;

        case 7:

            break;

        default:
            break;
    }
    return 0;
}

#if MULTI_CHANNEL_VIDEO_REC
/*

Routine Description:

    The system sub task for multiple channel video capture.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
void sysCaptureVideoSubTask(void* pData)
{
    INT8U               err;
    VIDEO_CLIP_OPTION   *pVideoClipOption;

    pVideoClipOption                        = (VIDEO_CLIP_OPTION*)pData;
    pVideoClipOption->sysCaptureVideoStart  = 1;
    pVideoClipOption->sysCaptureVideoStop   = 0;
    //pVideoClipOption->asfCaptureMode        = sysCaptureVideoMode;
    //pVideoClipOption->asfVopWidth           = asfVopWidth;
    //pVideoClipOption->asfVopHeight          = asfVopHeight;

#if (HW_BOARD_OPTION == MR6730_AFN)
	#if (USE_REC_TERM_MON)	
	switch(pVideoClipOption->VideoChannelID)
	{
	case 1:
		{
			//setUI.RecStatus|=UI_SET_STATUS_BITMSK_CH1_ISREC;//bitmap b0:CH1 recording , b1:CH2 recording
			MACRO_UI_SET_STATUS_BIT_SET(UI_SET_STATUS_BIT_CH1_ISREC);
		}
	break;
	case 2:
		{
			//setUI.RecStatus|=UI_SET_STATUS_BITMSK_CH2_ISREC;//bitmap b0:CH1 recording , b1:CH2 recording
			MACRO_UI_SET_STATUS_BIT_SET(UI_SET_STATUS_BIT_CH2_ISREC);
		}
	break;
	}
	#endif
#endif

    // Do capture video here
    MultiChannelAsfCaptureVideo(pVideoClipOption);

#if (HW_BOARD_OPTION == MR6730_AFN)
	#if (USE_REC_TERM_MON)	
	switch(pVideoClipOption->VideoChannelID)
	{
	case 1:
		{
			//if( setUI.CH1SaveEn == 0 )
			{
				 //if( setUI.CH1Recing == 1 )
				 {
				 if( MultiChannelGetCaptureVideoStatus(1) != 1 )
					 setUI.CH1Recing = 0;
				 }
			}
			//setUI.RecStatus&=(~UI_SET_STATUS_BITMSK_CH1_ISREC);
			MACRO_UI_SET_STATUS_BIT_CLR(UI_SET_STATUS_BIT_CH1_ISREC);
		}
		break;
	case 2:
		{
			//if( setUI.CH2SaveEn == 0 )
			{
				// if( setUI.CH2Recing == 1 )
				 {
				 if( MultiChannelGetCaptureVideoStatus(2) != 1 )
					 setUI.CH2Recing = 0;
				 }
			}
			//setUI.RecStatus&=(~UI_SET_STATUS_BITMSK_CH2_ISREC);
			MACRO_UI_SET_STATUS_BIT_CLR(UI_SET_STATUS_BIT_CH2_ISREC);
		}			
		break;
	}//switch() 
	#endif
#endif


    OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH0 << pVideoClipOption->VideoChannelID, OS_FLAG_SET, &err);

    sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, pVideoClipOption->VideoChannelID);

    OSTaskSuspend(OS_PRIO_SELF);
    DEBUG_SYS("OSTaskSuspend(sysCaptureVideoSubTask%d) fail!!!\n", pVideoClipOption->VideoChannelID);
    while(1) {
        OSTimeDly(0xffff);
    }
}

/*

Routine Description:

    Multiple channel capture video.

Arguments:

    ZoomFactor  - Zoom factor.
    Mode        - ASF_CAPTURE_NORMAL        // normal mode
                  ASF_CAPTURE_OVERWRITE     // fixed length overwrite mode
                  ASF_CAPTURE_EVENT         // event mode

Return Value:

    0 - Failure.
    1 - Success.

*/
int MultiChannelSysCaptureVideo(s32 ZoomFactor, u32 Mode)
{
    int     i;
    INT8U   err;
    u8      level;

    // MPEG-4 software reset
    SYS_RSTCTL = 0x00000100;
    for (i = 0; i < 256; i++);
    SYS_RSTCTL = 0;

#if DUAL_MODE_DISP_SUPPORT
    iduCaptureVideo(mpeg4Width * 2, mpeg4Height);
#else
    iduCaptureVideo(mpeg4Width, mpeg4Height);
#endif

    for(i = 0; i < MULTI_CHANNEL_LOCAL_MAX; i++)
    {
        if(MULTI_CHANNEL_SEL & (1 << i))
        {
            //OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH0 << i, OS_FLAG_CLR, &err);
            //VideoClipOption[i].VideoChannelID   = i;
            switch(i)
            {
                case 0:
                    DEBUG_SYS("Create sub task of video record channel %d (%d,%d)\n", i, asfVopWidth, asfVopHeight);
                    OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH0, OS_FLAG_CLR, &err);
                    VideoClipOption[i].VideoChannelID   = i;
                    VideoClipOption[i].AV_Source        = LOCAL_RECORD;
                    VideoClipOption[i].asfCaptureMode   = sysCaptureVideoMode;
                    VideoClipOption[i].asfVopWidth      = asfVopWidth;
                    VideoClipOption[i].asfVopHeight     = asfVopHeight;
                    VideoClipOption[i].mpeg4Width       = asfVopWidth;
                    VideoClipOption[i].mpeg4Height      = asfVopHeight;
                    VideoClipOption[i].VideoBufMng      = &MultiChannelVideoBufMng[i][0];
                    #ifdef ASF_AUDIO
                    VideoClipOption[i].iisSounBufMng    = &MultiChanneliisSounBufMng[i][0];
                    #endif
                    OSTaskCreate(SYS_SUB_TASK, &VideoClipOption[0], SYS_SUB_TASK_STACK0, SYS_SUB_TASK_PRIORITY_UNIT0);
                    break;
                case 1:
                    if(sysPIPMain == PIP_MAIN_CH1)
                        break;
                    DEBUG_SYS("Create sub task of video record channel %d (%d,%d)\n", i, asfVopWidth, asfVopHeight);
                    OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH1, OS_FLAG_CLR, &err);
                    VideoClipOption[i].VideoChannelID   = i;
                    VideoClipOption[i].AV_Source        = LOCAL_RECORD;
                #if (HW_BOARD_OPTION == MR8211_ZINWELL)
                  #if 0
                    if(OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_WAIT_SET_ALL, &err))   // SD card ready
                        VideoClipOption[i].asfCaptureMode   = sysCaptureVideoMode;
                    else    // SD card not ready
                    {
                        VideoClipOption[i].asfCaptureMode   = sysCaptureVideoMode | ASF_CAPTURE_EVENT_DUMMY_ENA;
                        //VideoClipOption[i].FreeSpaceControl = 1;
                    }
                  #else
                    level   = sysCheckSDCD();
                    if ( (level == SDC_CD_OFF) || (gInsertCard == 0) )
                    {
                        DEBUG_P2P("SD card not ready\n");
                        VideoClipOption[i].asfCaptureMode   = sysCaptureVideoMode | ASF_CAPTURE_EVENT_DUMMY_ENA;
                    }
                    else
                    {
                        DEBUG_P2P("SD card ready\n");
                        VideoClipOption[i].asfCaptureMode   = sysCaptureVideoMode;
                    }
                  #endif
                #else
                    VideoClipOption[i].asfCaptureMode   = sysCaptureVideoMode;
                #endif
                    VideoClipOption[i].asfVopWidth      = asfVopWidth;
                    VideoClipOption[i].asfVopHeight     = asfVopHeight;
                    VideoClipOption[i].mpeg4Width       = asfVopWidth;
                    VideoClipOption[i].mpeg4Height      = asfVopHeight;
                    VideoClipOption[i].VideoBufMng      = &MultiChannelVideoBufMng[i][0];
                    #ifdef ASF_AUDIO
                    VideoClipOption[i].iisSounBufMng    = &MultiChanneliisSounBufMng[i][0];
                    #endif
                    OSTaskCreate(SYS_SUB_TASK, &VideoClipOption[1], SYS_SUB_TASK_STACK1, SYS_SUB_TASK_PRIORITY_UNIT1);
                    break;
                case 2:
                    if(sysPIPMain == PIP_MAIN_CH2)
                        break;
                    DEBUG_SYS("Create sub task of video record channel %d\n", i);
                    OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH2, OS_FLAG_CLR, &err);
                    VideoClipOption[i].VideoChannelID   = i;
                    VideoClipOption[i].AV_Source        = LOCAL_RECORD;
                #if (HW_BOARD_OPTION == MR8211_ZINWELL)
                    //if(OSFlagAccept(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_CARD_READY, OS_FLAG_WAIT_SET_ALL, &err))   // SD card ready
                    //    VideoClipOption[i].asfCaptureMode   = sysCaptureVideoMode;
                    //else    // SD card not ready
                    //{
                        VideoClipOption[i].asfCaptureMode   = sysCaptureVideoMode | ASF_CAPTURE_EVENT_DUMMY_ENA;
                        //VideoClipOption[i].FreeSpaceControl = 1;
                    //}
                    VideoClipOption[i].asfVopWidth      = 320;
                    VideoClipOption[i].asfVopHeight     = 240;
                    VideoClipOption[i].mpeg4Width       = 320;
                    VideoClipOption[i].mpeg4Height      = 240;
                #else
                    VideoClipOption[i].asfCaptureMode   = sysCaptureVideoMode;
                    VideoClipOption[i].asfVopWidth      = asfVopWidth;
                    VideoClipOption[i].asfVopHeight     = asfVopHeight;
                    VideoClipOption[i].mpeg4Width       = asfVopWidth;
                    VideoClipOption[i].mpeg4Height      = asfVopHeight;
                #endif
                    VideoClipOption[i].VideoBufMng      = &MultiChannelVideoBufMng[i][0];
                    #ifdef ASF_AUDIO
                    VideoClipOption[i].iisSounBufMng    = &MultiChanneliisSounBufMng[i][0];
                    #endif
                    OSTaskCreate(SYS_SUB_TASK, &VideoClipOption[2], SYS_SUB_TASK_STACK2, SYS_SUB_TASK_PRIORITY_UNIT2);
                    break;
                case 3:
                    DEBUG_SYS("Create sub task of video record channel %d (%d,%d)\n", i, asfVopWidth, asfVopHeight);
                    OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH3, OS_FLAG_CLR, &err);
                    VideoClipOption[i].VideoChannelID   = i;
                    VideoClipOption[i].AV_Source        = LOCAL_RECORD;
                    VideoClipOption[i].asfCaptureMode   = sysCaptureVideoMode;
                    VideoClipOption[i].asfVopWidth      = asfVopWidth;
                    VideoClipOption[i].asfVopHeight     = asfVopHeight;
                    VideoClipOption[i].mpeg4Width       = asfVopWidth;
                    VideoClipOption[i].mpeg4Height      = asfVopHeight;
                    VideoClipOption[i].VideoBufMng      = &MultiChannelVideoBufMng[i][0];
                    #ifdef ASF_AUDIO
                    VideoClipOption[i].iisSounBufMng    = &MultiChanneliisSounBufMng[i][0];
                    #endif
                    OSTaskCreate(SYS_SUB_TASK, &VideoClipOption[3], SYS_SUB_TASK_STACK3, SYS_SUB_TASK_PRIORITY_UNIT3);
                    break;
                default:
                    DEBUG_SYS("Error: MultiChannelSysCaptureVideo() don't support channel %d", i);
            }
        }
    }

    DEBUG_SYS("System task pending for all channel video record finish...\n");
    OSFlagPend(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_ALL, OS_FLAG_WAIT_SET_ALL, OS_IPC_WAIT_FOREVER, &err);
    DEBUG_SYS("All channel video record finish!!!\n");

    // MPEG-4 software reset
    SYS_RSTCTL = 0x00000100;
    for (i = 0; i < 256; i++);
    SYS_RSTCTL = 0;

    DEBUG_SYS("Deleting all sub task of video record...\n");
    for(i = 0; i < MULTI_CHANNEL_LOCAL_MAX; i++)
    {
        if(MULTI_CHANNEL_SEL & (1 << i))
        {
            //OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH0 << i, OS_FLAG_CLR, &err);
            //VideoClipOption[i].VideoChannelID   = i;
            switch(i)
            {
                case 0:
                    DEBUG_SYS("Delete sub task of video record channel %d\n", i);
                    if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT0))
                        DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT0) error!!\n");
                    if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT0))
                        DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT0) error!!\n");
                    break;
                case 1:
                    if(sysPIPMain == PIP_MAIN_CH1)
                        break;
                    DEBUG_SYS("Delete sub task of video record channel %d\n", i);
                    if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT1))
                        DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT1) error!!\n");
                    if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT1))
                        DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT1) error!!\n");
                    break;
                case 2:
                    if(sysPIPMain == PIP_MAIN_CH2)
                        break;
                    DEBUG_SYS("Delete sub task of video record channel %d\n", i);
                    if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT2))
                        DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT2) error!!\n");
                    if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT2))
                        DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT2) error!!\n");
                    break;
                case 3:
                    DEBUG_SYS("Delete sub task of video record channel %d\n", i);
                    if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT3))
                        DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT3) error!!\n");
                    if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT3))
                        DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT3) error!!\n");
                    break;
                default:
                    DEBUG_SYS("Error: MultiChannelSysCaptureVideo() don't support channel %d", i);
            }
        }
    }
    DEBUG_SYS("Deleting all sub task of video record finish!!\n");
    sysCaptureVideoStart    = 0;
    sysCaptureVideoStop     = 1;
}

/*

Routine Description:

    Create video capture sub task.

Arguments:

    VideoChannelID  - Video channel ID.
    Width           - Video width.
    Height          - Video height.

Return Value:

    0 - Failure.
    1 - Success.

*/
int sysCaptureVideoSubTaskCreate(int VideoChannelID, VIDEO_CLIP_PARAMETER *pVideoClipParameter)
{
    int                 i;
    OS_STK              *ptos;
    INT8U               prio, err;
    PVIDEO_CLIP_OPTION  pVideoClipOption;

    DEBUG_SYS("sysCaptureVideoSubTaskCreate(%d, %d, %d)\n", VideoChannelID, pVideoClipParameter->asfVopWidth, pVideoClipParameter->asfVopHeight);

    if(MULTI_CHANNEL_SEL & (1 << VideoChannelID) == 0)
    {
        DEBUG_SYS("Error: Ch%d isn't a valid channel\n", VideoChannelID);
        return 0;
    }

    i                   = VideoChannelID;
    pVideoClipOption    = &VideoClipOption[i];
    OSSemPend(pVideoClipOption->PackerTaskSemEvt, OS_IPC_WAIT_FOREVER, &err);

    if(pVideoClipOption->PackerTaskCreated)
    {
        DEBUG_SYS("Can't repeat create sub task of video record channel %d\n", i);
        OSSemPost(pVideoClipOption->PackerTaskSemEvt);
        return 0;
    }

    pVideoClipOption->pVideoClipParameter   = pVideoClipParameter;
    switch(i)
    {
        case 0:
            DEBUG_SYS("Create sub task of video record channel %d (%d,%d)\n", i, pVideoClipParameter->asfVopWidth, pVideoClipParameter->asfVopHeight);
            OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH0, OS_FLAG_CLR, &err);
            pVideoClipOption->PackerTaskCreated = 1;
            pVideoClipOption->VideoChannelID    = i;
            pVideoClipOption->AV_Source         = LOCAL_RECORD;
            pVideoClipOption->asfCaptureMode    = pVideoClipParameter->sysCaptureVideoMode;
            pVideoClipOption->asfVopWidth       = pVideoClipParameter->asfVopWidth;
            pVideoClipOption->asfVopHeight      = pVideoClipParameter->asfVopHeight;
            pVideoClipOption->mpeg4Width        = pVideoClipParameter->asfVopWidth;
            pVideoClipOption->mpeg4Height       = pVideoClipParameter->asfVopHeight;
            pVideoClipOption->asfRecTimeLen     = pVideoClipParameter->asfRecTimeLen;
            pVideoClipOption->VideoBufMng       = &MultiChannelVideoBufMng[i][0];
            #ifdef ASF_AUDIO
            pVideoClipOption->iisSounBufMng     = &MultiChanneliisSounBufMng[i][0];
            #endif
            OSTaskCreate(SYS_SUB_TASK, &VideoClipOption[0], SYS_SUB_TASK_STACK0, SYS_SUB_TASK_PRIORITY_UNIT0);
            break;
        case 1:
            if(sysPIPMain == PIP_MAIN_CH1)
                break;
            DEBUG_SYS("Create sub task of video record channel %d (%d,%d)\n", i, pVideoClipParameter->asfVopWidth, pVideoClipParameter->asfVopHeight);
            OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH1, OS_FLAG_CLR, &err);
            pVideoClipOption->PackerTaskCreated = 1;
            pVideoClipOption->VideoChannelID    = i;
            pVideoClipOption->AV_Source         = LOCAL_RECORD;
            pVideoClipOption->asfCaptureMode    = pVideoClipParameter->sysCaptureVideoMode;
            pVideoClipOption->asfVopWidth       = pVideoClipParameter->asfVopWidth;
            pVideoClipOption->asfVopHeight      = pVideoClipParameter->asfVopHeight;
            pVideoClipOption->mpeg4Width        = pVideoClipParameter->asfVopWidth;
            pVideoClipOption->mpeg4Height       = pVideoClipParameter->asfVopHeight;
            pVideoClipOption->asfRecTimeLen     = pVideoClipParameter->asfRecTimeLen;
            pVideoClipOption->VideoBufMng       = &MultiChannelVideoBufMng[i][0];
            #ifdef ASF_AUDIO
            pVideoClipOption->iisSounBufMng     = &MultiChanneliisSounBufMng[i][0];
            #endif
            OSTaskCreate(SYS_SUB_TASK, &VideoClipOption[1], SYS_SUB_TASK_STACK1, SYS_SUB_TASK_PRIORITY_UNIT1);
            break;
        case 2:
            if(sysPIPMain == PIP_MAIN_CH2)
                break;
            DEBUG_SYS("Create sub task of video record channel %d (%d,%d)\n", i, pVideoClipParameter->asfVopWidth, pVideoClipParameter->asfVopHeight);
            OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH2, OS_FLAG_CLR, &err);
            pVideoClipOption->PackerTaskCreated = 1;
            pVideoClipOption->VideoChannelID    = i;
            pVideoClipOption->AV_Source         = LOCAL_RECORD;
            pVideoClipOption->asfCaptureMode    = pVideoClipParameter->sysCaptureVideoMode;
            pVideoClipOption->asfVopWidth       = pVideoClipParameter->asfVopWidth;
            pVideoClipOption->asfVopHeight      = pVideoClipParameter->asfVopHeight;
            pVideoClipOption->mpeg4Width        = pVideoClipParameter->asfVopWidth;
            pVideoClipOption->mpeg4Height       = pVideoClipParameter->asfVopHeight;
            pVideoClipOption->asfRecTimeLen     = pVideoClipParameter->asfRecTimeLen;
            pVideoClipOption->VideoBufMng       = &MultiChannelVideoBufMng[i][0];
            #ifdef ASF_AUDIO
            pVideoClipOption->iisSounBufMng     = &MultiChanneliisSounBufMng[i][0];
            #endif
            OSTaskCreate(SYS_SUB_TASK, &VideoClipOption[2], SYS_SUB_TASK_STACK2, SYS_SUB_TASK_PRIORITY_UNIT2);
            break;
        case 3:
            DEBUG_SYS("Create sub task of video record channel %d (%d,%d)\n", i, pVideoClipParameter->asfVopWidth, pVideoClipParameter->asfVopHeight);
            OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH3, OS_FLAG_CLR, &err);
            pVideoClipOption->PackerTaskCreated = 1;
            pVideoClipOption->VideoChannelID    = i;
            pVideoClipOption->AV_Source         = LOCAL_RECORD;
            pVideoClipOption->asfCaptureMode    = pVideoClipParameter->sysCaptureVideoMode;
            pVideoClipOption->asfVopWidth       = pVideoClipParameter->asfVopWidth;
            pVideoClipOption->asfVopHeight      = pVideoClipParameter->asfVopHeight;
            pVideoClipOption->mpeg4Width        = pVideoClipParameter->asfVopWidth;
            pVideoClipOption->mpeg4Height       = pVideoClipParameter->asfVopHeight;
            pVideoClipOption->asfRecTimeLen     = pVideoClipParameter->asfRecTimeLen;
            pVideoClipOption->VideoBufMng       = &MultiChannelVideoBufMng[i][0];
            #ifdef ASF_AUDIO
            pVideoClipOption->iisSounBufMng     = &MultiChanneliisSounBufMng[i][0];
            #endif
            OSTaskCreate(SYS_SUB_TASK, &VideoClipOption[3], SYS_SUB_TASK_STACK3, SYS_SUB_TASK_PRIORITY_UNIT3);
            break;
        default:
            DEBUG_SYS("Error: sysCaptureVideoSubTaskCreate(%d) don't support", i);
    }

    OSSemPost(pVideoClipOption->PackerTaskSemEvt);

    return 1;
}

/*

Routine Description:

    Destroy video capture sub task.

Arguments:

    VideoChannelID  - Video channel ID.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 sysCaptureVideoSubTaskDestroy(int VideoChannelID)
{
    int                 i;
    INT8U               err;
    PVIDEO_CLIP_OPTION  pVideoClipOption;

    DEBUG_SYS("sysCaptureVideoSubTaskDestroy(%d)\n", VideoChannelID);

    if(MULTI_CHANNEL_SEL & (1 << VideoChannelID) == 0)
    {
        DEBUG_SYS("Error: Ch%d isn't a valid channel\n", VideoChannelID);
        return 0;
    }

    i                   = VideoChannelID;
    pVideoClipOption    = &VideoClipOption[i];

    OSSemPend(pVideoClipOption->PackerTaskSemEvt, OS_IPC_WAIT_FOREVER, &err);

    if(pVideoClipOption->PackerTaskCreated != 1)
    {
        DEBUG_SYS("Can't destroy empty sub task of video record channel %d\n", i);
        OSSemPost(pVideoClipOption->PackerTaskSemEvt);
        return 0;
    }

    if(pVideoClipOption->sysCaptureVideoStop == 0)
        MultiChannelAsfCaptureVideoStop(pVideoClipOption);

    DEBUG_SYS("Pending for Ch%d video record finish...\n", pVideoClipOption->VideoChannelID);
    OSFlagPend(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH0 << pVideoClipOption->VideoChannelID, OS_FLAG_WAIT_SET_ALL, OS_IPC_WAIT_FOREVER, &err);

    switch(i)
    {
        case 0:
            DEBUG_SYS("Delete sub task of video record channel %d\n", i);
            if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT0))
                DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT0) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT0))
                DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT0) error!!\n");
            break;
        case 1:
            if(sysPIPMain == PIP_MAIN_CH1)
                break;
            DEBUG_SYS("Delete sub task of video record channel %d\n", i);
            if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT1))
                DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT1) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT1))
                DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT1) error!!\n");
            break;
        case 2:
            if(sysPIPMain == PIP_MAIN_CH2)
                break;
            DEBUG_SYS("Delete sub task of video record channel %d\n", i);
            if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT2))
                DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT2) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT2))
                DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT2) error!!\n");
            break;
        case 3:
            DEBUG_SYS("Delete sub task of video record channel %d\n", i);
            if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT3))
                DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT3) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT3))
                DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT3) error!!\n");
            break;
        default:
            DEBUG_SYS("Error: sysCaptureVideoSubTaskDestroy(%d) don't support", i);
    }

    DEBUG_SYS("Deleting Ch%d sub task of video record finish!!\n", i);

#if (HW_BOARD_OPTION == MR6730_AFN)
	#if (USE_REC_TERM_MON)	
			if(pVideoClipOption->VideoChannelID==1)
			{
				//if(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH1_TSKRDY))
				{	
					MACRO_UI_SET_STATUS_BIT_CLR(UI_SET_STATUS_BIT_CH1_TSKRDY);
				}
			}
		#if(!MULTI_CH_DEGRADE_1CH)	
			if(pVideoClipOption->VideoChannelID==2)
			{			
				//if(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH2_TSKRDY))
				{	
					MACRO_UI_SET_STATUS_BIT_CLR(UI_SET_STATUS_BIT_CH2_TSKRDY);
				}			
			}
		#endif
	#endif
#endif

    // Enter Preview
    switch(i)
    {
        case 0:
        #if(MULTI_CHANNEL_SEL & 0x01)
            sysPreviewReset(sysPreviewZoomFactor);
        #endif
        break;

        case 1:
        #if(MULTI_CHANNEL_SEL & 0x02)
            sysCiu_1_PreviewReset(sysPreviewZoomFactor);
        #endif
        break;

        case 2:
        #if(MULTI_CHANNEL_SEL & 0x04)
            sysCiu_2_PreviewReset(sysPreviewZoomFactor);
        #endif
        break;

        case 3:
        #if(MULTI_CHANNEL_SEL & 0x08)
            sysCiu_3_PreviewReset(sysPreviewZoomFactor);
        #endif
        break;

        case 4:
        #if(MULTI_CHANNEL_SEL & 0x10)
            sysCiu_4_PreviewReset(sysPreviewZoomFactor);
        #endif
        break;
    }

    pVideoClipOption->PackerTaskCreated = 0;
    OSSemPost(pVideoClipOption->PackerTaskSemEvt);

    return 1;
}

/*

Routine Description:

    Multiple channel capture one channel video.

Arguments:

    VideoChannelID  - Video channel ID.

Return Value:

    0 - Failure.
    1 - Success.

*/
int MultiChannelSysCaptureVideoOneCh(int VideoChannelID)
{
#if (HW_BOARD_OPTION==MR6730_AFN)
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
	unsigned int  cpu_sr = 0;					 /* Prevent compiler warning						   */
#endif

	//for debug only
	#if 0
		if(VideoChannelID==1 || VideoChannelID==2)
		{//recording duration of motion detect
			VideoClipParameter[VideoChannelID].asfRecTimeLen=60;//that will make it quickly
		}
	#endif	
	
	#if (MULTI_CH_DEGRADE_1CH)
	if(VideoChannelID>=2)
	{
		#if  1
		//do nothing but just return ok
		#else
		if(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH2_RECON))
		{
			MACRO_UI_SET_STATUS_BIT_CLR(UI_SET_STATUS_BIT_CH2_RECON);
		}
		#endif
		return 1;
	}
	#endif
	
	#if (USE_REC_TERM_MON)
	#if (USE_RTM_DELAY_ACTIVATE)
	OS_ENTER_CRITICAL();
	if(setUI.RecDlyAct==0)
	{
		setUI.RecDlyAct=UI_RTM_DLYACT_CNT;
	}
	OS_EXIT_CRITICAL();
	#endif
	#endif //#if (USE_REC_TERM_MON)		
#endif 

	// If File system isn't ready, it would be reject for recording
	if(gSystemStroageReady != 1)
	{
		// gSystemStroageReady == 0, means init yet
		if((sysCheckSDCD() == SDC_CD_IN) && (gSystemStroageReady != 0))
			uiOsdDrawSDCardFail(UI_OSD_DRAW);
		return 0;
	}

#if (HW_BOARD_OPTION == MR6730_AFN)
	#if (MULTI_CHANNEL_SUPPORT && MULTI_CHANNEL_VIDEO_REC)	// multichannel
		
		#if (Get_sametime == 1)
		if(sametime_Pair)
		{
			int Ch_Id=VideoChannelID;
		#if(MULTI_CH_DEGRADE_1CH)	
			//only one channel supportted
			if(Ch_Id==1)
		#else
			if(Ch_Id==1 || Ch_Id==2)
		#endif
			{
		
				Ch_Id--;
				sametime_UseCnt[Ch_Id]=0;
			#if 0
				memcpy(&same_localTime,&g_LocalTime,sizeof(RTC_DATE_TIME));
			#else
			{
				RTC_DATE_TIME loc_time;
				u32 time_to_sec=0;
				u32 sametime_to_sec=0;
				u32 time_diff=0;
				u8 UpdateTime=0;
		
				
				RTC_Get_Time(&loc_time);
				time_to_sec=RTC_Time_To_Second(&loc_time);
				sametime_to_sec=RTC_Time_To_Second(&same_localTime);
				if(time_to_sec<sametime_to_sec)
				{
					time_diff=(sametime_to_sec-time_to_sec);
				}
				else
				{
					time_diff=(time_to_sec-sametime_to_sec);
				}
				//
				if(time_diff>=REC_SAMETIME_LTDIFF_THRES)//if(time_diff>=REC_SYNC_LTDIFF_THRES)
				{//update new time to "same_localTime" if difference of current time and last time is g.t. 5 seconds
					memcpy(&same_localTime,&loc_time,sizeof(RTC_DATE_TIME));
					//
					DEBUG_SYS("re-init same_localTime:%04d/%02d/%02d-%02d:%02d:%02d\n\n",(same_localTime.year+2000),same_localTime.month,same_localTime.day,same_localTime.hour,same_localTime.min,same_localTime.sec);
					//
					//memcpy(&sametime_CurrTime[Ch_Id],&same_localTime,sizeof(RTC_DATE_TIME));
					memcpy(&sametime_CurrTime[0],&same_localTime,sizeof(RTC_DATE_TIME));
					memcpy(&sametime_CurrTime[1],&same_localTime,sizeof(RTC_DATE_TIME));
					//
				}
			}
			#endif 
			}
		}		
		#endif //#if (Get_sametime == 1)	
		
	#endif	
#endif	
	
#if (HW_BOARD_OPTION == MR6730_AFN)
	if(VideoChannelID < MULTI_CHANNEL_LOCAL_MAX)
	{
		
	#if (USE_REC_TERM_MON)

		DEBUG_SYS("<REC>S%d-setUI.StopRec=%d\n",VideoChannelID,setUI.StopRec);//debug 	
		//if(setUI.StopRec==0)
		{
			switch(VideoChannelID)
			{
			case 1:
				//if(setUI.CH1Recing)//<--- 
				{
					//setUI.RecStatus|=UI_SET_STATUS_BITMSK_CH1_RECON;//bitmap b4:CH1 rec on-off , b5:CH2 rec on-off
					MACRO_UI_SET_STATUS_BIT_SET(UI_SET_STATUS_BIT_CH1_RECON);
				}
			break;
			case 2:
				//if(setUI.CH2Recing)//<---
				{
					//setUI.RecStatus|=UI_SET_STATUS_BITMSK_CH2_RECON;//bitmap b4:CH1 rec on-off , b5:CH2 rec on-off
					MACRO_UI_SET_STATUS_BIT_SET(UI_SET_STATUS_BIT_CH2_RECON);
				}
			break;
			}
			DEBUG_UI("<REC>S-(REC:0x%02X,EN:0x%02X)\n",(setUI.RecStatus&(UI_SET_STATUS_BITMSK_CH1_ISREC|UI_SET_STATUS_BITMSK_CH2_ISREC)),(setUI.RecStatus&(UI_SET_STATUS_BITMSK_CH1_RECON|UI_SET_STATUS_BITMSK_CH2_RECON))>>4);	
		}		
	#endif
	
		return sysCaptureVideoSubTaskCreate(VideoChannelID,
											&VideoClipParameter[VideoChannelID]);	
	
	}
#else
	//orig-code

    if(VideoChannelID < MULTI_CHANNEL_LOCAL_MAX)
        return sysCaptureVideoSubTaskCreate(VideoChannelID,
                                            &VideoClipParameter[VideoChannelID]);
#endif	

#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    else if(VideoChannelID < MULTI_CHANNEL_MAX)
    {
        RfRxVideoPackerEnableOneCh(VideoChannelID - MULTI_CHANNEL_LOCAL_MAX);
        return 1;
    }
#endif
    else
    {
        DEBUG_SYS("Error: MultiChannelSysCaptureVideoOneCh(%d) isn't a valid channel\n", VideoChannelID);
        return 0;
    }
}

/*

Routine Description:

    Multiple channel capture one channel video stop.

Arguments:

    VideoChannelID  - Video channel ID.

Return Value:

    0 - Failure.
    1 - Success.

*/
int MultiChannelSysCaptureVideoStopOneCh(int VideoChannelID)
{
#if (HW_BOARD_OPTION==MR6730_AFN)
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
	unsigned int  cpu_sr = 0;					 /* Prevent compiler warning						   */
#endif

#if (MULTI_CH_DEGRADE_1CH)
if(VideoChannelID>=2)
{
	#if  1
	//do nothing but just return ok
	#else
	if(MACRO_UI_SET_STATUS_BIT_CHK(UI_SET_STATUS_BIT_CH2_RECON))
	{
		MACRO_UI_SET_STATUS_BIT_CLR(UI_SET_STATUS_BIT_CH2_RECON);
	}
	#endif
	return 1;
}
#endif

#if (USE_ALARMIN_REC)	
	if(setUI.AlarmIn_REC)
	{
		setUI.AlarmIn_REC=0;//clear
		DEBUG_SYS("AlarmIn_REC clear\n");
	}
#endif //#if (USE_ALARMIN_REC)
	
	
#if (USE_REC_TERM_MON)
		
		//if(setUI.StopRec)
		{//stop by manual operation
		DEBUG_SYS("<REC>P%d-setUI.StopRec=%d\n",VideoChannelID,setUI.StopRec);//debug 	
			switch(VideoChannelID)
			{
			case 1:
				if(setUI.CH1Recing==0)
				{
					//setUI.RecStatus&=(~UI_SET_STATUS_BITMSK_CH1_RECON);//bitmap b4:CH1 rec on-off , b5:CH2 rec on-off					
					if(setUI.StopRec)//manual stop
						MACRO_UI_SET_STATUS_BIT_CLR(UI_SET_STATUS_BIT_CH1_RECON);										
				}
			break;
			case 2:
				if(setUI.CH2Recing==0)
				{
					//setUI.RecStatus&=(~UI_SET_STATUS_BITMSK_CH2_RECON);//bitmap b4:CH1 rec on-off , b5:CH2 rec on-off
					if(setUI.StopRec)//manual stop
						MACRO_UI_SET_STATUS_BIT_CLR(UI_SET_STATUS_BIT_CH2_RECON);					
				}
			break;
			}
			
			DEBUG_UI("<REC>P-(REC:0x%02X,EN:0x%02X)\n",(setUI.RecStatus&(UI_SET_STATUS_BITMSK_CH1_ISREC|UI_SET_STATUS_BITMSK_CH2_ISREC)),(setUI.RecStatus&(UI_SET_STATUS_BITMSK_CH1_RECON|UI_SET_STATUS_BITMSK_CH2_RECON))>>4);	
		}
	
	#if (USE_RTM_DELAY_ACTIVATE)
		if(setUI.StopRec)//manual stop
		{
			OS_ENTER_CRITICAL();
			if(setUI.RecDlyAct)
				setUI.RecDlyAct=0;
			OS_EXIT_CRITICAL();
		}
	#endif
#endif
#endif //#if (HW_BOARD_OPTION==MR6730_AFN)

	
    if(VideoChannelID < MULTI_CHANNEL_LOCAL_MAX)
        return sysCaptureVideoSubTaskDestroy(VideoChannelID);
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    else if(VideoChannelID < MULTI_CHANNEL_MAX)
    {
        RfRxVideoPackerDisableOneCh(VideoChannelID - MULTI_CHANNEL_LOCAL_MAX);
        return 1;
    }
#endif
    else
    {
        DEBUG_SYS("Error: MultiChannelSysCaptureVideoOneCh(%d) isn't a valid channel\n", VideoChannelID);
        return 0;
    }
}

/*

Routine Description:

    Multiple channel get status of capture one channel video.

Arguments:

    VideoChannelID  - Video channel ID.

Return Value:

    0 - Not in capture video mode.
    1 - In capture video mode.
    2 - In prerecord mode, but not in capture video mode.(in detect mode)

*/
int MultiChannelGetCaptureVideoStatus(int VideoChannelID)
{
    PVIDEO_CLIP_OPTION  pVideoClipOption;
    int                 VideoRecording;
    u8                  err;
    u32                 waitFlag;

    //DEBUG_SYS("\nMultiChannelGetCaptureVideoStatus(%d)\n", VideoChannelID);

    pVideoClipOption    = &VideoClipOption[VideoChannelID];

    if(VideoChannelID < MULTI_CHANNEL_LOCAL_MAX)
    {
        if(MULTI_CHANNEL_SEL & (1 << VideoChannelID) == 0)
        {
            DEBUG_SYS("Error: Ch%d isn't a valid channel\n", VideoChannelID);
            return 0;
        }

        waitFlag = OSFlagAccept(gSysSubReadyFlagGrp, (FLAGSYS_SUB_RDYSTAT_REC_CH0 << VideoChannelID), OS_FLAG_WAIT_SET_ALL, &err);
        /*
        if(err != OS_NO_ERR)
        {
            DEBUG_UI("gSysSubReadyFlagGrp = %d, error = %d\n",gSysSubReadyFlagGrp, err);
            DEBUG_UI("OSFlagAccept(gSysSubReadyFlagGrp, %d) error!!\n", FLAGSYS_SUB_RDYSTAT_REC_CH0 << pVideoClipOption->VideoChannelID);
            return 0;
        }
        */


        //return (pVideoClipOption->PackerTaskCreated || !waitFlag) ? 1 : 0;
    }
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    else if(VideoChannelID < MULTI_CHANNEL_MAX)
    {
        waitFlag = OSFlagAccept(gRfRxVideoPackerSubReadyFlagGrp, (FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 << VideoChannelID), OS_FLAG_WAIT_SET_ALL, &err);
        //DEBUG_SYS("\ngRfRxVideoPackerSubReadyFlagGrp = 0x%08x\n", gRfRxVideoPackerSubReadyFlagGrp->OSFlagFlags);
        //DEBUG_SYS("Ch%d waitFlag = %d\n", VideoChannelID, waitFlag);
        //DEBUG_SYS("Ch%d PackerTaskCreated = %d\n", VideoChannelID, pVideoClipOption->PackerTaskCreated);
        /*
        if(err != OS_NO_ERR)
        {
            DEBUG_UI("gSysSubReadyFlagGrp = %d, error = %d\n",gSysSubReadyFlagGrp, err);
            DEBUG_UI("OSFlagAccept(gSysSubReadyFlagGrp, %d) error!!\n", FLAGSYS_SUB_RDYSTAT_REC_CH0 << pVideoClipOption->VideoChannelID);
            return 0;
        }
        */

        //return (pVideoClipOption->PackerTaskCreated || !waitFlag) ? 1 : 0;
    }
#endif

    if(pVideoClipOption->PackerTaskCreated || !waitFlag)    // 錄影的task還開著
    {
        if(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL)    // prerecord mode
        {
            if(pVideoClipOption->OpenFile)  // record mode
                return 1;
            else                            // detect mode
                return 2;
        }
        else    // menu record mode
            return 1;
    }
    else    // 錄影的task沒開
    {
        return 0;
    }
}




#if (HW_BOARD_OPTION==MR6730_AFN)
void sysCaptureVideo_DryRun(int VideoChannelID)
{

	if(VideoChannelID==1||VideoChannelID==2)
	{
		INT8U               err;
		VIDEO_CLIP_PARAMETER video_parm;
		PVIDEO_CLIP_OPTION  pVideoClipOption;
		int                 i;
		VIDEO_CLIP_PARAMETER *pVideoClipParameter;

		i                   = VideoChannelID;
		memcpy((void*)&video_parm,(void*)&VideoClipParameter[i],sizeof(VIDEO_CLIP_PARAMETER));
		pVideoClipParameter = (VIDEO_CLIP_PARAMETER*)&video_parm;
		pVideoClipOption	= &VideoClipOption[i];
		pVideoClipOption->pVideoClipParameter   = pVideoClipParameter;


		switch(i)
	    {
	    case 1:
			//OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH1, OS_FLAG_CLR, &err);
			//pVideoClipOption->PackerTaskCreated = 1;
			pVideoClipOption->VideoChannelID    = i;
			pVideoClipOption->AV_Source		   = LOCAL_RECORD;
			pVideoClipOption->asfCaptureMode    = pVideoClipParameter->sysCaptureVideoMode;
			pVideoClipOption->asfVopWidth	   = pVideoClipParameter->asfVopWidth;
			pVideoClipOption->asfVopHeight	   = pVideoClipParameter->asfVopHeight;
			pVideoClipOption->mpeg4Width 	   = pVideoClipParameter->asfVopWidth;
			pVideoClipOption->mpeg4Height	   = pVideoClipParameter->asfVopHeight;
			pVideoClipOption->asfRecTimeLen	   = pVideoClipParameter->asfRecTimeLen;
			pVideoClipOption->VideoBufMng	   = &MultiChannelVideoBufMng[i][0];
			#ifdef ASF_AUDIO
			pVideoClipOption->iisSounBufMng	   = &MultiChanneliisSounBufMng[i][0];
			#endif
			//MultiChannelAsfCaptureVideo(pVideoClipOption);
			MultiChannelIISRecordTaskCreate(pVideoClipOption);
			OSTimeDlyHMSM(0,0,0,100);//OSTimeDlyHMSM(0,0,0,500);
			MultiChannelIISRecordTaskDestroy(pVideoClipOption);
		break;
	    case 2:
			//OSFlagPost(gSysSubReadyFlagGrp, FLAGSYS_SUB_RDYSTAT_REC_CH2, OS_FLAG_CLR, &err);
			//pVideoClipOption->PackerTaskCreated = 1;
			pVideoClipOption->VideoChannelID    = i;
			pVideoClipOption->AV_Source		   = LOCAL_RECORD;
			pVideoClipOption->asfCaptureMode    = pVideoClipParameter->sysCaptureVideoMode;
			pVideoClipOption->asfVopWidth	   = pVideoClipParameter->asfVopWidth;
			pVideoClipOption->asfVopHeight	   = pVideoClipParameter->asfVopHeight;
			pVideoClipOption->mpeg4Width 	   = pVideoClipParameter->asfVopWidth;
			pVideoClipOption->mpeg4Height	   = pVideoClipParameter->asfVopHeight;
			pVideoClipOption->asfRecTimeLen	   = pVideoClipParameter->asfRecTimeLen;
			pVideoClipOption->VideoBufMng	   = &MultiChannelVideoBufMng[i][0];
			#ifdef ASF_AUDIO
			pVideoClipOption->iisSounBufMng	   = &MultiChanneliisSounBufMng[i][0];
			#endif
			//MultiChannelAsfCaptureVideo(pVideoClipOption);
			MultiChannelIISRecordTaskCreate(pVideoClipOption);
			OSTimeDlyHMSM(0,0,0,100);//OSTimeDlyHMSM(0,0,0,500);
			MultiChannelIISRecordTaskDestroy(pVideoClipOption);
		break;		
		}

	}
	
}
#endif





#if MULTI_CHANNEL_RF_RX_VIDEO_REC
/*

Routine Description:

    RF Rx video pack sub task.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
void RfRxVideoPackerSubTask(void* pData)
{
    INT8U               err;
    VIDEO_CLIP_OPTION   *pVideoClipOption;
	FS_DISKFREE_T       *diskInfo;
    u32                 free_size;
    u32                 bytes_per_cluster;

    pVideoClipOption                        = (VIDEO_CLIP_OPTION*)pData;
    pVideoClipOption->sysCaptureVideoStart  = 1;
    pVideoClipOption->sysCaptureVideoStop   = 0;
    //pVideoClipOption->asfCaptureMode        = sysCaptureVideoMode;
    pVideoClipOption->AV_Source             = RX_RECEIVE;
    //DEBUG_SYS("sysCaptureVideoMode  = %d\n", sysCaptureVideoMode);
    //asfSectionTime                          = 20;
    //DEBUG_SYS("asfSectionTime       = %d\n", asfSectionTime);

    // Do capture video here
    MultiChannelAsfCaptureVideo(pVideoClipOption);

    DEBUG_SYS("RfRxVideoPackerSubTask(%d) Suspend\n", pVideoClipOption->VideoChannelID);

    OSTimeDly(1);

    OSFlagPost(gRfRxVideoPackerSubReadyFlagGrp, FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 << pVideoClipOption->RFUnit, OS_FLAG_SET, &err);

#if 0
    if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA))
    {
        u32 TotalSize, i;
        // for disk full control
        diskInfo                        = &global_diskInfo;
        bytes_per_cluster               = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        free_size                       = diskInfo->avail_clusters * (bytes_per_cluster / 512) / 2; //KByte unit
        TotalSize                       = 0;
        for(i = 0; i < MULTI_CHANNEL_MAX; i++)
        {
            if(VideoClipOption[i].PackerTaskCreated && (i != pVideoClipOption->VideoChannelID))
            {
                TotalSize  += (VideoClipOption[i].asfDataSize +
                              VideoClipOption[i].asfIndexSize +
                              VideoClipOption[i].asfHeaderSize +
                              VideoClipOption[i].CurrentVideoSize +
                              VideoClipOption[i].CurrentAudioSize) / 1024;
            }
        }
    #if 1
        if((free_size < (MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) * MULTI_CHANNEL_MAX / 1024) ||
            (TotalSize >= (free_size - (MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) * MULTI_CHANNEL_MAX / 1024)))
        {
            DEBUG_ASF("Ch%d Disk full!!!\n", pVideoClipOption->VideoChannelID);
            DEBUG_ASF("free_size        = %d KBytes\n", free_size);
            DEBUG_ASF("TotalSize        = %d bytes.\n", TotalSize);
            DEBUG_ASF("asfHeaderSize    = %d bytes.\n", pVideoClipOption->asfHeaderSize);
            DEBUG_ASF("asfDataSize      = %d bytes.\n", pVideoClipOption->asfDataSize);
            DEBUG_ASF("asfIndexSize     = %d bytes.\n", pVideoClipOption->asfIndexSize);
            DEBUG_ASF("CurrentVideoSize = %d bytes.\n", pVideoClipOption->CurrentVideoSize);
            DEBUG_ASF("CurrentAudioSize = %d bytes.\n", pVideoClipOption->CurrentAudioSize);
            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, pVideoClipOption->VideoChannelID);
        }
    #else
        diskInfo            = &global_diskInfo;
        bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        free_size           = diskInfo->avail_clusters * (bytes_per_cluster / 512) / 2; //KByte unit
        if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA) && (free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM) * MULTI_CHANNEL_MAX) / 1024)) //Notice: K-Byte unit
        {
            DEBUG_ASF("Disk full!!!\n", free_size);
            DEBUG_ASF("free_size = %d KBytes\n", free_size);
            sysSetEvt(SYS_EVT_VIDEOCAPTURE_STOP, pVideoClipOption->VideoChannelID);
       }
    #endif
    }
#endif

    OSTaskSuspend(OS_PRIO_SELF);
    DEBUG_SYS("OSTaskSuspend(RfRxVideoPackerSubTask%d) fail!!!\n", pVideoClipOption->VideoChannelID);
    while(1) {
        OSTimeDly(0xffff);
    }
}


/*

Routine Description:

    Create RF Rx video pack sub task.

Arguments:

    RFUnit      - RF channel ID.
    Width       - Video width.
    Height      Video height.

Return Value:

    0 - Failure.
    1 - Success.

*/
int RfRxVideoPackerSubTaskCreate(int RFUnit, VIDEO_CLIP_PARAMETER *pVideoClipParameter)
{
    int     i;
    OS_STK  *ptos;
    INT8U   prio, err;
    PVIDEO_CLIP_OPTION  pVideoClipOption;

    i                                   = MULTI_CHANNEL_LOCAL_MAX + RFUnit;
    pVideoClipOption                    = &VideoClipOption[i];

#if LIVEVIEW_UNFROZEN
	if(OSSemAccept(pVideoClipOption->PackerTaskSemEvt) == 0x0)
    	return 0;
#else
    OSSemPend(pVideoClipOption->PackerTaskSemEvt, OS_IPC_WAIT_FOREVER, &err);
#endif

    if((RfRxVideoRecordEnable & (1 << RFUnit)) == 0)
    {
        OSSemPost(pVideoClipOption->PackerTaskSemEvt);
        return 0;
    }

    if(pVideoClipOption->PackerTaskCreated)
    {
        //DEBUG_SYS("Can't repeat create RF Rx sub task of video record channel %d\n", i);
        OSSemPost(pVideoClipOption->PackerTaskSemEvt);
        return 0;
    }
    DEBUG_SYS("Create RF Rx sub task of video record channel %d (%d,%d)\n", i, pVideoClipParameter->asfVopWidth, pVideoClipParameter->asfVopHeight);

    pvcoRfiu[RFUnit]                        = pVideoClipOption;
    pVideoClipOption->pVideoClipParameter   = pVideoClipParameter;
    pVideoClipOption->PackerTaskCreated     = 1;
    pVideoClipOption->VideoChannelID        = i;
    pVideoClipOption->RFUnit                = RFUnit;
    pVideoClipOption->VideoBufMng           = rfiuRxVideoBufMng[RFUnit];
    pVideoClipOption->VideoBufMngReadIdx    = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
    pVideoClipOption->VideoBufMngWriteIdx   = rfiuRxVideoBufMngWriteIdx[pVideoClipOption->RFUnit];
    pVideoClipOption->asfCaptureMode        = pVideoClipParameter->sysCaptureVideoMode;
    pVideoClipOption->asfVopWidth           = pVideoClipParameter->asfVopWidth;
    pVideoClipOption->asfVopHeight          = pVideoClipParameter->asfVopHeight;
    pVideoClipOption->mpeg4Width            = pVideoClipParameter->asfVopWidth;
    pVideoClipOption->mpeg4Height           = pVideoClipParameter->asfVopHeight;
    pVideoClipOption->asfRecTimeLen         = pVideoClipParameter->asfRecTimeLen;

    /* Create the semaphore */
    pVideoClipOption->VideoTrgSemEvt        = OSSemCreate(VIDEO_BUF_NUM - 2); /* guarded for ping-pong buffer */
    pVideoClipOption->VideoCmpSemEvt        = OSSemCreate(0);
   	pVideoClipOption->VideoRTPCmpSemEvt     = OSSemCreate(0);

#ifdef ASF_AUDIO
    /* initialize sound buffer */
    pVideoClipOption->iisSounBufMng         = rfiuRxIIsSounBufMng[RFUnit];
    pVideoClipOption->iisSounBufMngReadIdx  = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];
    pVideoClipOption->iisSounBufMngWriteIdx = rfiuRxIIsSounBufMngWriteIdx[pVideoClipOption->RFUnit];

    /* Create the semaphore */
    pVideoClipOption->iisTrgSemEvt          = OSSemCreate(IIS_BUF_NUM - 2); /* guarded for ping-pong buffer */
    pVideoClipOption->iisCmpSemEvt          = OSSemCreate(0);
   	//pVideoClipOption->iisplayCmpEvt     = OSSemCreate(0);
   	//pVideoClipOption->AudioRTPCmpSemEvt = OSSemCreate(0);
    //AudioRTPCmpSemEvt[pVideoClipOption->VideoChannelID] = OSSemCreate(0);
    pVideoClipOption->gIISPlayUseSem        = OSSemCreate(1);
    pVideoClipOption->gIISRecUseSem         = OSSemCreate(1);

	pVideoClipOption->iisRecDMACnt          = 0;
	pVideoClipOption->IIS_Task_Stop         = 0;
#endif
  #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    if(Record_flag[RFUnit] == 1)
    {
        OSSemPost(pVideoClipOption->PackerTaskSemEvt);
        return 1;
    }
  #endif
    OSFlagPost(gRfRxVideoPackerSubReadyFlagGrp, FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 << RFUnit, OS_FLAG_CLR, &err);

    /* Create the task */
    switch(i)
    {
        case    0:
            ptos    = SYS_SUB_TASK_STACK0;
            prio    = SYS_SUB_TASK_PRIORITY_UNIT0;
            break;
        case    1:
            ptos    = SYS_SUB_TASK_STACK1;
            prio    = SYS_SUB_TASK_PRIORITY_UNIT1;
            break;
        case    2:
            ptos    = SYS_SUB_TASK_STACK2;
            prio    = SYS_SUB_TASK_PRIORITY_UNIT2;
            break;
        case    3:
            ptos    = SYS_SUB_TASK_STACK3;
            prio    = SYS_SUB_TASK_PRIORITY_UNIT3;
            break;
        default:
            break;
    }
    OSTaskCreate(RfRxVideoPackerSubTask, pVideoClipOption, ptos, prio);

    OSSemPost(pVideoClipOption->PackerTaskSemEvt);

    return 1;
}

/*

Routine Description:

    Destroy RF Rx video pack sub task.

Arguments:

    RFUnit      - RF channel ID.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 RfRxVideoPackerSubTaskDestroy(int RFUnit)
{
    int     i;
    INT8U   err;
    PVIDEO_CLIP_OPTION  pVideoClipOption;

    pVideoClipOption    = pvcoRfiu[RFUnit];

    OSSemPend(pVideoClipOption->PackerTaskSemEvt, OS_IPC_WAIT_FOREVER, &err);

    if(pVideoClipOption->PackerTaskCreated != 1)
    {
        OSSemPost(pVideoClipOption->PackerTaskSemEvt);
      #if INSERT_NOSIGNAL_FRAME
        Record_flag[RFUnit] = 0;
      #endif
        return 0;
    }
  #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    if(Record_flag[RFUnit] == 0)
    {
        pVideoClipOption->PackerTaskCreated = 0;
    }
  #endif
    DEBUG_SYS("RfRxVideoPackerSubTaskDestroy(%d)\n", RFUnit);
    DEBUG_SYS("Destroy RF Rx sub task of video record channel %d\n", pVideoClipOption->VideoChannelID);

    if(pVideoClipOption->RFUnit != RFUnit)
    {
        DEBUG_SYS("RfRxVideoPackerSubTaskDestroy(%d) fail!!!!\n", RFUnit);
        DEBUG_SYS("pVideoClipOption->RFUnit(%d) != RFUnit(%d)!!!!\n", pVideoClipOption->RFUnit, RFUnit);
        OSSemPost(pVideoClipOption->PackerTaskSemEvt);
        return 0;
    }
  #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    if(VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth != gRfiuUnitCntl[RFUnit].TX_PicWidth )
        Record_flag[RFUnit] = 0;
    if(Record_flag[RFUnit] == 1)
    {
        OSSemPost(pVideoClipOption->PackerTaskSemEvt);
        return 0;
    }
  #endif
    do
    {
        if(pVideoClipOption->sysCaptureVideoStop == 0)
        {
            DEBUG_SYS("MultiChannelAsfCaptureVideoStop(%d)\n", pVideoClipOption->VideoChannelID);
            MultiChannelAsfCaptureVideoStop(pVideoClipOption);
        }
        DEBUG_SYS("Pending for RFUnit(%d) video record finish...\n", RFUnit);
        OSFlagPend(gRfRxVideoPackerSubReadyFlagGrp, FLAGSYS_RF_RX_PACKER_SUB_RDYSTAT_REC_CH0 << RFUnit, OS_FLAG_WAIT_SET_ALL, 200, &err);
        if(OS_TIMEOUT == err)
            DEBUG_SYS("Error: Pending for RFUnit(%d) video record finish time out!!!!\n", RFUnit);
    } while(OS_TIMEOUT == err);

    OSTimeDly(4);

    /* Delete the task */
    switch(pVideoClipOption->VideoChannelID)
    {
        case 0:
            if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT0))
                DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT0) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT0))
                DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT0) error!!\n");
            break;
        case 1:
            if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT1))
                DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT1) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT1))
                DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT1) error!!\n");
            break;
        case 2:
            if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT2))
                DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT2) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT2))
                DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT2) error!!\n");
            break;
        case 3:
            if(OS_NO_ERR !=  OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT3))
                DEBUG_SYS("OSTaskSuspend(SYS_SUB_TASK_PRIORITY_UNIT3) error!!\n");
            if(OS_NO_ERR !=  OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT3))
                DEBUG_SYS("OSTaskDel(SYS_SUB_TASK_PRIORITY_UNIT3) error!!\n");
            break;
        default:
            DEBUG_SYS("Error: MultiChannelMPEG4Destroy() don't support pVideoClipOption->VideoChannelID = %d", pVideoClipOption->VideoChannelID);
    }

    /* Delete the semaphore */
    pVideoClipOption->VideoTrgSemEvt    = OSSemDel(pVideoClipOption->VideoTrgSemEvt, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->VideoTrgSemEvt)
        DEBUG_SYS("OSSemDel(pVideoClipOption->VideoTrgSemEvt) error!!\n");

    pVideoClipOption->VideoCmpSemEvt    = OSSemDel(pVideoClipOption->VideoCmpSemEvt, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->VideoCmpSemEvt)
        DEBUG_SYS("OSSemDel(pVideoClipOption->VideoCmpSemEvt) error!!\n");

   	pVideoClipOption->VideoRTPCmpSemEvt = OSSemDel(pVideoClipOption->VideoRTPCmpSemEvt, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->VideoRTPCmpSemEvt)
        DEBUG_SYS("OSSemDel(pVideoClipOption->VideoRTPCmpSemEvt) error!!\n");

#ifdef ASF_AUDIO
    pVideoClipOption->iisTrgSemEvt      = OSSemDel(pVideoClipOption->iisTrgSemEvt, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->iisTrgSemEvt)
        DEBUG_SYS("OSSemDel(pVideoClipOption->iisTrgSemEvt) error!!\n");

    pVideoClipOption->iisCmpSemEvt      = OSSemDel(pVideoClipOption->iisCmpSemEvt, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->iisCmpSemEvt)
        DEBUG_SYS("OSSemDel(pVideoClipOption->iisCmpSemEvt) error!!\n");

    pVideoClipOption->gIISPlayUseSem    = OSSemDel(pVideoClipOption->gIISPlayUseSem, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->gIISPlayUseSem)
        DEBUG_SYS("OSSemDel(pVideoClipOption->gIISPlayUseSem) error!!\n");

    pVideoClipOption->gIISRecUseSem     = OSSemDel(pVideoClipOption->gIISRecUseSem, OS_DEL_ALWAYS, &err);
    if(pVideoClipOption->gIISRecUseSem)
        DEBUG_SYS("OSSemDel(pVideoClipOption->gIISRecUseSem) error!!\n");
#endif

    DEBUG_MP4("RfRxVideoPackerSubTaskDestroy(%d) finished!!\n", RFUnit);
    pVideoClipOption->PackerTaskCreated = 0;
    OSSemPost(pVideoClipOption->PackerTaskSemEvt);

    return 1;
}

/*

Routine Description:

    Enable all RF Rx video pack.

Arguments:

    None.

Return Value:

    The value of RfRxVideoRecordEnable.

*/
s32 RfRxVideoPackerEnable(void)
{
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    OS_ENTER_CRITICAL();
    RfRxVideoRecordEnable   = (1 << (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX)) - 1;
    OS_EXIT_CRITICAL();

    return RfRxVideoRecordEnable;
}

/*

Routine Description:

    Disable all RF Rx video pack.

Arguments:

    None.

Return Value:

    The value of RfRxVideoRecordEnable.

*/
s32 RfRxVideoPackerDisable(void)
{
    int     i;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    OS_ENTER_CRITICAL();
    RfRxVideoRecordEnable   = 0;
    OS_EXIT_CRITICAL();

    for(i = 0; i < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX); i++)
        RfRxVideoPackerSubTaskDestroy(i);

    return RfRxVideoRecordEnable;
}

/*

Routine Description:

    Enable RF Rx video pack.

Arguments:

    RFUnit      - RF channel ID.

Return Value:

    The value of RfRxVideoRecordEnable.

*/
s32 RfRxVideoPackerEnableOneCh(int RFUnit)
{
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    OS_ENTER_CRITICAL();
    RfRxVideoRecordEnable  |= 1 << RFUnit;
    OS_EXIT_CRITICAL();

    return RfRxVideoRecordEnable;
}

/*

Routine Description:

    Disable RF Rx video pack.

Arguments:

    RFUnit      - RF channel ID.

Return Value:

    The value of RfRxVideoRecordEnable.

*/
s32 RfRxVideoPackerDisableOneCh(int RFUnit)
{
    int     i;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    OS_ENTER_CRITICAL();
    RfRxVideoRecordEnable  &= ~(1 << RFUnit);
    OS_EXIT_CRITICAL();
  #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    Record_flag[RFUnit] = 0;
  #endif
    RfRxVideoPackerSubTaskDestroy(RFUnit);

    return RfRxVideoRecordEnable;
}


#endif  // #if MULTI_CHANNEL_RF_RX_VIDEO_REC



#endif  // #if MULTI_CHANNEL_VIDEO_REC

u8 sysProjectCaptureVideo(u8 Step, s32 ZoomFactor)
{
    switch(Step)
    {
        case SYS_CAPTURE_VIDEO_OVERWRITE_DELETE_PASS:
            break;

        case SYS_CAPTURE_VIDEO_RISE_FREQUENCY:
            break;

        case SYS_CAPTURE_VIDEO_CHANGE_CHANNEL:
            break;

        case SYS_CAPTURE_VIDEO_CHECK_SET_60FPS:
            break;

        case SYS_CAPTURE_VIDEO_WRITE_ASF_FILE:
        #if(HW_BOARD_OPTION==MR6730_WINEYE)
            DEBUG_SYS("###########SYS_CAPTURE_VIDEO_WRITE_ASF_FILE\n");
            CaptureVideoResult  = asfCaptureVideo(ZoomFactor, sysCaptureVideoMode);    /* capture and write asf file */
            if(CaptureVideoResult == 0) // 出錯就一直叫一直閃燈
            {
                gpioBuzzerCtrl(BUZZER_ONF05SOFF1S);
                gpioTimerCtrLed(LED_ALL_FLASH);
                while(1);
            }
        #else
              #if MULTI_CHANNEL_VIDEO_REC
                CaptureVideoResult  = MultiChannelSysCaptureVideo(ZoomFactor, sysCaptureVideoMode);    /* capture and write asf file */
              #else
                #if SD_CARD_DISABLE
                sysCaptureVideoMode = sysCaptureVideoMode | ASF_CAPTURE_EVENT_ALARM_ENA;
                #endif
                CaptureVideoResult  = asfCaptureVideo(ZoomFactor, sysCaptureVideoMode);    /* capture and write asf file */
                #if VMDSW
                   #if HW_MD_SUPPORT
                    VMD_CNT = 0;
                   #endif
                #endif
              #endif
        #endif
            break;

        case SYS_CAPTURE_VIDEO_SET_SIU_MODE:
            #if ISU_OVERLAY_ENABLE
                #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN) || (HW_BOARD_OPTION == MR6730_AFN)
                #else
                isuGenerateScalarOverlayImage   = 0;
                #endif
            #endif
            break;

        case SYS_CAPTURE_VIDEO_FALL_FREQUENCY:
            break;

        case SYS_CAPTURE_VIDEO_SET_LED:
            break;

        case SYS_CAPTURE_VIDEO_POWER_OFF:
            break;
        case SYS_CAPTURE_VIDEO_CLEAR_BUFFER:
            if(sysTVOutOnFlag) //TV-out
            {

            	IduVideo_ClearBuf();

            }
            break;
        default:
            break;
    }
    return 0;
}


u8  sysProjectDeviceStatus(DEV_STATUS status)
{
    switch(status)
    {
        case DEV_SD_FULL:

            break;
		 case DEV_SD_NOT_FULL:
            osdDrawMemFull(UI_OSD_CLEAR);
		    break;
	    case DEV_USB_PLUG_IN:

			break;
        default:
            return 0;
    }
    return 1;
}


void sysProjectSetSensorChrome(u8 level)
{
    siuSetSensorChrome(level);

#if(HW_BOARD_OPTION == MR8120_TX_SKYSUCCESS)    

    if(iconflag[UI_MENU_SETIDX_SENSOR_CHROME] !=level)
    {
        iconflag[UI_MENU_SETIDX_SENSOR_CHROME]=level;
        //Save_UI_Setting();
        sysback_RF_SetEvt(SYS_BACKRF_RFI_SAVE_UI_SETTING,0);
    }
#endif


}

void sysProjectSetAudioVolume(u8 level)
{
    #if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    u8 volVal[6] = {31, 18, 14, 10, 6, 2};
    #endif
    DEBUG_SYS("===> sysProjectSetAudioVolume %d\n", level);
    #if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    if(iconflag[UI_MENU_SETIDX_VOLUME] != volVal[level])
    {
        iconflag[UI_MENU_SETIDX_VOLUME] = volVal[level];
        iconflag[UI_MENU_SETIDX_MELODY_VOL] = level;
        uiMenuAction(UI_MENU_SETIDX_VOLUME);
        uiMenuAction(UI_MENU_SETIDX_MELODY_VOL);
        Save_UI_Setting();
    }
    #else
    if(iconflag[UI_MENU_SETIDX_VOLUME] != level)
    {
        iconflag[UI_MENU_SETIDX_VOLUME] = level;
        uiMenuAction(UI_MENU_SETIDX_VOLUME);
        Save_UI_Setting();
    }
    #endif
}

void sysProjectSetLightOnOff(u8 level)
{
    DEBUG_SYS("===> sysProjectSetLightOnOff %d\n", level);
    #if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    rfiuSetSensorLightCtrl_TX(level);
    #endif
    
}

