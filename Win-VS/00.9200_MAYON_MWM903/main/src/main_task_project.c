/*

Copyright (c) 2009  Mars Technologies, Inc.

Module Name:

    main_alm.c

Abstract:

    The routines of main function.

Environment:

        ARM RealView Developer Suite

Revision History:

    2009/08/26  Elsa Lee  Create

*/

#include "general.h"

#include "task.h"
#include "main.h"
#include "stmemapi.h"
#include "intapi.h"
#include "debugapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "dpofapi.h"
#include "sysapi.h"

#include "mpeg4api.h"                   /* FIQ */
#include "jpegapi.h"
#include "siuapi.h"
#include "ipuapi.h"
#include "isuapi.h"
#include "iduapi.h"
#include "rfiuapi.h"
#include "hdmiapi.h"
#include "ciuapi.h"

#include "timerapi.h"                   /* IRQ */
//#include "dmaapi.h"
#include "gpioapi.h"
#include "smcapi.h"
#include "sdcapi.h"
#include "usbapi.h"
#include "rtcapi.h"
#include "uartapi.h"
#include "i2capi.h"
#include "iisapi.h"
#include "adcapi.h"
#include "hiapi.h"
#include "spiapi.h"
#include "uiapi.h"
#include "mp4api.h"
#include "asfapi.h"
#include "aviapi.h" /* Peter 0704 */
#include "mcpuapi.h"
#include "webapi.h"
#include "board.h"
#include "uiKey.h"  // VIDEO_MODE
#include "gfuapi.h"
#include "encrptyapi.h"

#include "../../ui/inc/ui.h"
#if CPU_PERFORMANCE_TEST
#include "../inc/coremark.h"
#endif

#include "intapi.h"
#include <mars_int.h>
#include <mars_dma.h>
#include <mars_timer.h>

#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif

#if defined(NEW_UI_ARCHITECTURE)
    #include "..\..\ui\inc\MainFlow.h"
#endif

#if PWIFI_SUPPORT
  #include "pwifiapi.h"
#endif

/*
 *********************************************************************************************************
 *                                               CONSTANTS
 *********************************************************************************************************
 */


/*
 *********************************************************************************************************
 *                                               VARIABLES
 *********************************************************************************************************
 */
#if (MD_METHOD_OPTION == MD_METHOD_2)
u8	AvgFrmReady = 0;	/* system ready flag when boot has beed completed for a while period ; 1->Ready, 0-> Not ready */
u32	TimeCnt;	/* Time Counter */
#endif

/* disk vol. name, it could assign the specific name by each project. Maximum is 11 characters.*/
char strVolName[11] = "           ";	/* It could assign the specific name of disk vol. by each project. Maximum is 11 characters. */
#if !LWIP2_SUPPORT
u32  TutkLoginTaskPri;
u32  TutkRoutineTaskPri;
#endif

#if !LWIP2_SUPPORT
u32 TutkTransmissionTaskPri;
#endif
#if PLAY_MAIN_PAGE_SLIDE
u32 	 LogoTaskStack[LOGO_TASK_STACK_SIZE];
BOOLEAN ShowLogoFinish = FALSE;
#endif
u32 gIDU_clk_div_value=0;

#if  LWIP2_SUPPORT == 1

void lwipBufInit(void);

OS_MEM *lwipBuffer = 0;
OS_MEM *lwipBuffer2 = 0;
OS_MEM *lwipBuffer3 = 0;
#endif

#if ICOMMWIFI_SUPPORT
OS_MEM *icomBuffer;
OS_MEM *icomBuffer2;
OS_MEM *icomBuffer3;

extern u8 *icommbuf;/*for icomm 6030p used. by aher 20160627*/
extern u8 *icommbuf2;/*for icomm 6030p used. by aher 20160627*/
extern u8 *icommbuf3;/*for icomm 6030p used. by aher 20160627*/
#endif

/*
 *********************************************************************************************************
 *                                               Extern VARIABLES
 *********************************************************************************************************
 */

extern u8 userClicksnapshot;
extern u8 userClickvideoRec;
extern u8 userClickvoiceRec;
extern u8 userClickFormat;
extern u8 got_disk_info;
extern u8 gInsertNAND;
extern u8 Main_Init_Ready;
extern u8 ADC_Init_Ready ;
#if (AUDIO_DEVICE == AUDIO_IIS_ALC5621)
extern u8 ALC5623_test;
#endif
/*
*********************************************************************************************************
*                                               Extern Function
*********************************************************************************************************
*/
void tutk_av_sample(void);
void RX_PowerSavingConfig(void);

#if SDRAM_TEST
extern u8 SDRAM_Test(u32* test_base);
#endif
extern s32 sysPreviewInit(s32);
#if PLAY_MAIN_PAGE_SLIDE
extern s32 uiMenuSetStartMovie(s8);
#endif
extern void WDT_init(void);
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
extern char smcWriteBackFAT(void);
#endif
#if IIS_TEST
extern void iisTest(void);
#endif
extern s8	spiTest(void);

#if DMA_TEST
extern int marsDMA_Test();
#endif

#if FPGA_TEST_DTV_YUV656
extern void FPGA_YUV601_Test(void);
#endif

#if SDC_WRITE_READ_TEST
s32 MemoryTestFunction(void);
#endif

void ExtCacheEnable(int burst8);
void ExtCacheDisable(void);
void mainCheckChipID(int Code);
void mainIpModuleTest(void);


#if H264_TEST
extern s32 H264Enc_LocalTest();//only test RC disable
extern s32 H264Dec_LocalTest();//only test RC disable
#endif

#if FPGA_TEST_DTV_720P_EP952_HDMI
void FPGA_720P_Test(void);
#endif

#if FPGA_TEST_DTV_1080I_EP952_HDMI
void FPGA_1080I_Test(void);
#endif

#if AUDIO_IN_OUT_SELFTEST
extern int AudioInOutTest(void);
#endif
/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

#if PLAY_MAIN_PAGE_SLIDE
void DrawLogoTask(void* pData)
{
    uiMenuSetStartMovie(2);
    ShowLogoFinish = TRUE;
    while (1)
    {
        OSTimeDly(1000);
    }
}
#endif

void IDU_clock_switch(u8 nPlus)
{
}


void RX_PowerSavingConfig(void)
{
    u32     sys_ctl0_status;

    sys_ctl0_status = SYS_CTL0;
    sys_ctl0_status &= ~SYS_CTL0_H264_CKEN  &
                       ~SYS_CTL0_JPEG_CKEN  &
                       ~SYS_CTL0_RF1012_CKEN &
                       ~SYS_CTL0_SIU_CKEN &
                       ~SYS_CTL0_IPU_CKEN &
                       ~SYS_CTL0_ISU_CKEN &
                    #if SD_CARD_DISABLE   
                       ~SYS_CTL0_SD_CKEN &
                    #endif
                       ~SYS_CTL0_MD_CKEN &
                       ~SYS_CTL0_SCUP_CKEN &
                       ~SYS_CTL0_CIU_CKEN &
                       ~SYS_CTL0_CIU2_CKEN;
     SYS_CTL0 = sys_ctl0_status;    
}
/*

Routine Description:

    The main task routine.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/

void mainTask(void *pData)
{
    u8 err;
    int i;
#if ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840) ||(HW_BOARD_OPTION == MR9100_TX_RDI_CA811)||\
        (HW_BOARD_OPTION == MR9100_TX_MAYON_MWL612))
    int First=1;
    GPIO_CFG c;
#endif

#if MEASURE_BOOT_TIME
    u32 time1,time2;
#endif
#if (USE_BUILD_IN_RTC == RTC_USE_BUILD_IN)
    RTC_COUNT   RTCCount1, RTCCount2;
#endif
#if (HDMI_TXIC_SEL == HDMI_TX_IT66121)
    u8 u8HdmiCnt = 0;
#endif
    //============================//
#if (AUDIO_DEVICE == AUDIO_IIS_ALC5621)
    ALC5623_test=0;
#endif
    got_disk_info   = 0;
#if SD_CARD_DISABLE
    sysSetStorageStatus(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_READY);
    global_diskInfo.avail_clusters      = 0x00001000;
    global_diskInfo.bytes_per_sector    = 4096;
    global_diskInfo.sectors_per_cluster = 32;
    global_diskInfo.total_clusters      = 0x00002000;
#else
    sysSetStorageStatus(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_NREADY);
#endif
    gInsertNAND     = 0;
    sysSetStorageStatus(SYS_I_STORAGE_BACKUP, SYS_V_STORAGE_NREADY);

    DEBUG_MAIN("mainTask Start, priority end = %d\r\n", MAIN_TASK_PRIORITY_END);

#if OS_TASK_255_SUPPORT
    if(MAIN_TASK_PRIORITY_END > OS_LOWEST_PRIO-1)
#else
	if(MAIN_TASK_PRIORITY_END > 62)
#endif
    {
        DEBUG_WARERR("########################################\r\n");
        DEBUG_WARERR("##    mainTask Priority Overflow %d   ##\r\n", MAIN_TASK_PRIORITY_END);
        DEBUG_WARERR("########################################\r\n");
        while(1);
    }

#if OS_TASK_STAT_EN > 0
    OSStatInit();                   /* Initialize the statistics task */
#endif

#if (USE_BUILD_IN_RTC == RTC_USE_BUILD_IN)
    DEBUG_MAIN("mainTask: rtcInit()\n");
    rtcInit();
    rtcGetCount(&RTCCount1);
#endif

    DEBUG_MAIN("mainTask: timerInit()\n");
    timerInit();    /* time tick for uCOS-II */

#if MEASURE_BOOT_TIME
    time1=OSTimeGet();
#endif


    DEBUG_MAIN("mainTask: gpioInit()\n");
    gpioInit();

    DEBUG_MAIN("mainTask: encryptInit();\n");
    encryptInit();



    DEBUG_MAIN("mainTask: WDT_init()\n");
    mainIpModuleTest(); //Lucian: 所有的測試code請放在這裡.
    
    WDT_init();
    WDT_Reset_Count();

#if( (SW_APPLICATION_OPTION  == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
	{//preset default date & time
		RTC_Get_Time(&SetTime);
		if(SetTime.year==0)//if(SetTime.year==0 && SetTime.month==1 && SetTime.day==1)
		{
			extern void DateTime_setDefault(void);
			extern void DateTime_setDefault_TZ(u8 Opt);
			extern void DateTime_setDefault_DST(u8 Opt);
			
			//DEBUG_MAIN("RTC_Get_Time,SetTime.year=%d\n",SetTime.year);
			DEBUG_MAIN("RTC_Get_Time year=0 (%d/%d/%d %d:%d:%d)\r\n",SetTime.year+2000, SetTime.month, SetTime.day,SetTime.hour, SetTime.min, SetTime.sec); 		

			DateTime_setDefault_DST(0);
			DEBUG_MAIN("##RTC_preset 1\n");
			DateTime_setDefault_TZ(0);
			DEBUG_MAIN("##RTC_preset 2\n");
			DateTime_setDefault();
			DEBUG_MAIN("##RTC_preset 3\n");
			
			RTC_Get_Time(&SetTime);
			DEBUG_MAIN("get SetTime %d/%d/%d %d:%d:%d\r\n",SetTime.year+2000, SetTime.month, SetTime.day,SetTime.hour, SetTime.min, SetTime.sec);		
		}
	}
#endif

    DEBUG_MAIN("mainTask: i2cInit()\n");
    i2cInit();

#if(HDMI_TXIC_SEL ==  HDMI_TX_EP952)
    DEBUG_MAIN("mainTask: EP_HDMI_952_Init()\n");
  #if(TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_HD720P60)
    EP_HDMI_952_Init(EP952_SF_32000Hz,EP952_VDO_1280x720P);
  #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_HD720P60_37M)
    EP_HDMI_952_Init(EP952_SF_32000Hz,EP952_VDO_1280x720P);
  #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_HD720P30)
    EP_HDMI_952_Init(EP952_SF_32000Hz,EP952_VDO_1280x720P);
  #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_HD720P25)
    EP_HDMI_952_Init(EP952_SF_32000Hz,EP952_VDO_1280x720P);
  #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_FHD1080I60)
    EP_HDMI_952_Init(EP952_SF_32000Hz,EP952_VDO_1920x1080I);
  #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_FHD1080P30)
    EP_HDMI_952_Init(EP952_SF_32000Hz,EP952_VDO_1920x1080P);
  #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_FHD1080P25)
    EP_HDMI_952_Init(EP952_SF_32000Hz,EP952_VDO_1920x1080P);
  #endif
#elif(HDMI_TXIC_SEL ==  HDMI_TX_CAT6613)

#elif(HDMI_TXIC_SEL == HDMI_TX_IT66121)
  #if(TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_HD720P60)
    IT66121_init(EP952_VDO_1280x720P);
  #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_FHD1080I60)
    IT66121_init(EP952_VDO_1920x1080I);
  #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE ==TV_FHD1080P30)
    IT66121_init(EP952_VDO_1920x1080P);
  #elif(TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_VGA)
    IT66121_init(EP952_VDO_720x480W);
  #endif
#endif


    DEBUG_MAIN("mainTask: jpegInit();\n");
    jpegInit(); /* 0425 : BJ*/

    DEBUG_MAIN("mainTask: sysInit();\n");
    sysInit();
    sys_background_init();  //civic 2007 0822 For Background R/W I/O
    sys_backLowTask_init();
    sys_back_RF_Task_init();  //處理RF task 所發出的Event
#if (NIC_SUPPORT == 1)
    sys_back_Network_Task_init();
#endif


    DEBUG_MAIN("mainTask: LCM_IDUInit();\n");
    LCM_IDUInit();  /*CY 0907*/
    IDU_clock_switch(1);
    LCD_PwrOnSeq();

    iduLockVideoAllColor(0x80800000); //LCD_orange_line, show logo前右邊一條橘線, 黑色OSD test color使得橘線變黑線, 要放置於uiinit前
    iduUnlockVideoAllColor();

#if ((FLASH_OPTION == FLASH_SERIAL_ESMT) || (FLASH_OPTION == FLASH_SERIAL_SST)|| (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON))
    DEBUG_MAIN("mainTask: spiInit();\n");
    spiInit();
  #if SWAP_SP1_SP3
    DEBUG_MAIN("============SWAP_SP1_SP3 for Ice mode.===============\n");
  #endif
#endif

    DEBUG_MAIN("mainTask: adcInit()\n");
    adcInit(0);
    ADC_Init_Ready = 1;//==========================
    DEBUG_MAIN("mainTask: i2cDeviceInit()\n");
    i2cDeviceInit();
  #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
       (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == Standalone_Test) ||\
       (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD)  || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
       (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)||\
       (SW_APPLICATION_OPTION == MR8202_GATEWAYBOX_RX) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) )
  
  #else
  	#if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && USE_NEW_MEMORY_MAP)
	Idu_ClearBuf_ULTRA_FHD(0);
	for(i=1; i<DISPLAY_BUF_NUM; i++)
		Idu_ClearBuf_ULTRA_FHD_BOUNDARY(i);	
	#else	
	Idu_ClearBuf(DISPLAY_BUF_NUM);
	#endif
  #endif
  #if( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) )
    memset_hw_Word(Sub1Videodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
  #endif

#if MULTI_CHANNEL_VIDEO_REC
    InitVideoClipOption();
#endif

#if GFU_SUPPORT
    DEBUG_MAIN("mainTask: gfuInit()\n");
    gfuInit();
#endif    

    DEBUG_MAIN("mainTask: uiInit();\n");
    uiInit();
    DEBUG_MAIN("mainTask: uiOSDPreviewInit()\n");
    uiOSDPreviewInit();

#if(SW_APPLICATION_OPTION  == MR9300_RFDVR_RX1RX2 || UI_BOOT_FROM_PANEL == 0)
    for(i=0;i<10;i++)
    {
    #if(HDMI_TXIC_SEL ==  HDMI_TX_EP952)
        EP952Controller_Task();
        //EP_HDMI_DumpMessage_952();
        OSTimeDly(1);
    #elif(HDMI_TXIC_SEL == HDMI_TX_CAT6613)

    #elif(HDMI_TXIC_SEL == HDMI_TX_IT66121)
        IT66121_Task();
        OSTimeDly(1);
    #endif
    }
#endif

#if defined(NEW_UI_ARCHITECTURE)
    UI_powerOnSequence1();  // Power-on sequence of the COMMAX UI
#endif
#if PLAY_MAIN_PAGE_SLIDE
    DEBUG_MAIN("mainTask: uiMenuSetStartMovie()\n");
    if (OSTaskCreate(DrawLogoTask, (void *) 0, (OS_STK *)LOGO_TASK_STACK, DISPLAY_LOGO_PRIORITY)!= OS_NO_ERR)
    {
        DEBUG_MAIN("Logo Task Create Fail!!!!\n");
        ShowLogoFinish = TRUE;
    }
#endif

    WDT_Reset_Count();
    DEBUG_MAIN("mainTask: i2cDeviceInit2()\n");
    i2cDeviceInit2();

#if (MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_MP4)
    DEBUG_MAIN("mainTask: mp4Init()\n");
    mp4Init();
#elif (MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_ASF)
    DEBUG_MAIN("mainTask: asfInit()\n");
    asfInit();
#elif (MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_AVI)   /*Peter 0704 S*/
    DEBUG_MAIN("mainTask: aviInit()\n");
    aviInit();
#endif                          /*Peter 0704 E*/

    DEBUG_MAIN("mainTask: CiuInit()\n");
    CiuInit();

    DEBUG_MAIN("mainTask: siuInit();\n");
    siuInit();

    DEBUG_MAIN("mainTask: isuInit();\n");
    isuInit();  /* 0425 : BJ*/

    init_serial_A();
#if UART_COMMAND
    DEBUG_MAIN("mainTask: uartCmdInit()\n");
    uartCmdInit();
#endif

#if (WEB_SERVER_SUPPORT == 1)
    DEBUG_MAIN("mainTask: WEB_Init()\n");
    WEB_Init();
#endif


#if USB_DONGLE_SUPPORT
    DEBUG_MAIN("mainTask: usbVcInit()\n");
    usbVcInit();
#endif

#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
    DEBUG_MAIN("mainTask: FTMAC110_main()\n");
	FTMAC110_main();
#endif

#if (TUTK_SUPPORT==1)
    
    #if LWIP2_SUPPORT
    tutk_av_sample();
    #else
    TutkLoginTaskPri = IOTC_LOGIN_TASK_PRIORITY;
    TutkRoutineTaskPri = IOTC_ROUTINE_TASK_PRIORITY;
	TutkTransmissionTaskPri=IOTC_TRANSMISSION_TASK_PRIORITY;
    tutkSampleInit();
    #endif
#endif


#if (HW_IR_SUPPORT == 1)
    hwIrInit();
#endif


#if PLAY_MAIN_PAGE_SLIDE
    while(ShowLogoFinish == FALSE)
        OSTimeDly(10);
    OSTaskDel(DISPLAY_LOGO_PRIORITY);
#endif

#if MARSS_SUPPORT
	MARSS_Init();
#endif

#if RFIU_SUPPORT
    DEBUG_MAIN("mainTask: RfiuInit()\n");
    RfiuInit();
#endif

#if (SD_TASK_INSTALL_FLOW_SUPPORT == 0)
	FS_Init(); // initialize file system: create semephore
	
#if SD_CARD_DISABLE
#else
    DEBUG_MAIN("mainTask: uiCheckSDCD()\n");
    uiCheckSDCD(1);
#endif

#if USB_HOST_MASS_SUPPORT
  #if 1
    DEBUG_MAIN("mainTask: uiCheckUSBCD()\n");
    usbHostInit();  //Initialize UsbHost controller and create semephore.
  #endif
   if (HCPortSC & 0x01) //usb Dev in
    {
    	sysKeepSetStorageSel(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_USBMASS, SYS_I_STORAGE_BACKUP);
    }
#endif
#endif

#if(  (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1) ||\
      (SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION == Standalone_Test) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) ||\
      (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1)  || (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) ||\
      (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) ||\
      (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8110_BABYMONITOR) ||(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1) ||\
      (SW_APPLICATION_OPTION == MR8202_GATEWAYBOX_RX) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1)  )
     //RX has no preview mode.
     RX_PowerSavingConfig();
#else
    MyHandler.MenuMode = VIDEO_MODE;
    DEBUG_MAIN("mainTask: sysPreviewInit()\n");
    sysPreviewInit(0);
#endif

    //----------------------------------//
 #if (RFIU_TEST==1)
    DEBUG_MAIN("\n RFIU Test Start.................................\n");
    DEBUG_MAIN("mainTask: OSTaskChangePrio.\n");
    OSTaskChangePrio(MAIN_TASK_PRIORITY_START, MAIN_TASK_PRIORITY_END);
    sysDeadLockMonitor_OFF();
    Main_Init_Ready = 1;
    OSFlagPost(gUiStateFlagGrp, FLAGUI_MAIN_INIT_READY, OS_FLAG_SET, &err);
    err=marsRfiu_Test();
    if(err==0)
    {
        DEBUG_MAIN("marsRfiu_Test FAIL!\n");
    }
    else
      DEBUG_MAIN("\n RFIU Test Finish.................................\n");

 #elif(RFIU_TEST==2)
    OSTaskCreate(RFIU_TXRX_TEST_TASK, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0);
 #endif

   IDU_clock_switch(0);

#if ICOMMWIFI_SUPPORT
    DEBUG_MAIN("OSMemCreate : icomm used.\n");
    DEBUG_MAIN("Start pri = %d\n",ICOMM_TASK_01_PRIORITY);  
    DEBUG_MAIN("\x1B[96m icommbuf:%08x \x1B[0m\n",icommbuf);
   
    icomBuffer = OSMemCreate(icommbuf, 8, 2560,&err);   //for icomm 6030p used. by aher 20160722
    if(icomBuffer==0)
	   DEBUG_MAIN("OSMemCreate fail.\n");
    else
	   DEBUG_MAIN("OSMemCreate success.\n");
    DEBUG_MAIN("\x1B[96m icommbuf2:%08x \x1B[0m\n",icommbuf2);
   
    icomBuffer2 = OSMemCreate(icommbuf2, 1024,256,&err);    //for icomm 6030p used. by aher 20160722
    if(icomBuffer2==0)
	   DEBUG_MAIN("OSMemCreate2 fail.\n");
    else
	   DEBUG_MAIN("OSMemCreate2 success.\n");
   
    DEBUG_MAIN("\x1B[96m icommbuf3:%08x \x1B[0m\n",icommbuf3);
   
    icomBuffer3 = OSMemCreate(icommbuf3, 10,4096*6,&err);   //for icomm 6030p used. by sean 20170426
   
    if(icomBuffer3==0)
	   DEBUG_MAIN("OSMemCreate3 fail.\n");
    else
	   DEBUG_MAIN("OSMemCreate3 success.\n");
	   
    DEBUG_MAIN("\x1B[96m icommbuf+ = %08x \x1B[0m\n",((u8 *)icommbuf+8*2560));
    DEBUG_MAIN("\x1B[96m icommbuf2+ = %08x \x1B[0m\n",((u8 *)icommbuf2+1024*256));
    DEBUG_MAIN("\x1B[96m icommbuf3+ = %08x \x1B[0m\n",((u8 *)icommbuf3+10*4096*6));
      
	DEBUG_MAIN("\033[40;31mCreate icomm 6030p task2\n\033[0m");
	DEBUG_MAIN("\x1B[96mmainTask: icomminit() \x1B[0m\n");

	DEBUG_MAIN("GpioActFlashSelect=0x%x\n",GpioActFlashSelect);
	//GpioActFlashSelect |= (1<<12);	
	icommInit();
  #if PWIFI_SUPPORT  
    DEBUG_MAIN("mainTask: pWifi_Init() \n");
    pWifi_Init();
  #endif
#endif
#if(HOME_RF_SUPPORT)
	DEBUG_UI("mainTask: homeRFInit() \n");
	homeRFInit();
#endif

#if PWM_BEEP_TASK_SUPPORT
	DEBUG_UI("mainTask: pwmBeepTaskInit() \n");
	pwmBeepTaskInit();
#endif
   //-----------------------//

#if MEASURE_BOOT_TIME
    time2=OSTimeGet();
    DEBUG_WARERR("--->Booting Time =%d ms\n",(time2-time1)*50);
#endif

#if defined(NEW_UI_ARCHITECTURE)
#if(SW_APPLICATION_OPTION!=MR9300_RFDVR_RX1RX2)
    DEBUG_UI("mainTask: UI_powerOnSequence2() \n");
    UI_powerOnSequence2();
#endif
#endif

#if RFIU_SUPPORT
    #if PWIFI_SUPPORT
    DEBUG_UI("mainTask: pWifi_Start() \n");    
    pWifi_Start();
    #else
    DEBUG_UI("mainTask: rfiu_Start() \n");    
    rfiu_Start();
    #endif
#endif


#if defined(NEW_UI_ARCHITECTURE)
#if(SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)
    DEBUG_UI("mainTask: UI_powerOnSequence2() \n");
    UI_powerOnSequence2();
#endif
#endif

#if SD_TASK_INSTALL_FLOW_SUPPORT
	FS_Init(); // initialize file system: create semephore
	
  #if SD_CARD_DISABLE
  #else
	DEBUG_MAIN("mainTask: uiCheckSDCD()\n");
	sdcTaskInit();
  #endif
	
  #if USB_HOST_MASS_SUPPORT
	DEBUG_MAIN("mainTask: uiCheckUSBCD()\n");
	usbHostInit();	//Initialize UsbHost controller and create semephore.
	if(HCPortSC & 0x01) //usb Dev in
		sysKeepSetStorageSel(SYS_I_STORAGE_MAIN, SYS_V_STORAGE_USBMASS, SYS_I_STORAGE_BACKUP);
  #endif
#endif

    //===================================While(1)=========================//
    //----------- Enter mainTask while loop --------------
    DEBUG_MAIN("mainTask: OSTaskChangePrio.\n");
    OSTaskChangePrio(MAIN_TASK_PRIORITY_START, MAIN_TASK_PRIORITY_END);
    Main_Init_Ready = 1;
    OSFlagPost(gUiStateFlagGrp, FLAGUI_MAIN_INIT_READY, OS_FLAG_SET, &err);
    IntFiqMask=0x40; //Lucian: 目前有開機FIQ Mask 被修改. 先採取治標作法.
    sysDeadLockMonitor_Reset();

    #if (USE_BUILD_IN_RTC == RTC_USE_BUILD_IN)
    /*檢查RTC是否會跳, RTCCount2與RTCCount1至少要相差1秒*/
    rtcGetCount(&RTCCount2);
    if (memcmp(&RTCCount1, &RTCCount2, sizeof(RTC_COUNT)) == 0)
    {
        rtcSetDefaultTime();
    }
    #endif

    DEBUG_MAIN("mainTask: while(1)\n");
    while (1)
    {
    #if UART_COMMAND
        // release cpu to uart command
        OSTimeDly(1);
    #endif

    #if DEADLOCK_MONITOR_ENA
        sysLifeTime_prev=sysLifeTime;
        //DEBUG_MAIN("$");
    #endif

    #if ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840) ||(HW_BOARD_OPTION == MR9100_TX_RDI_CA811)||\
        (HW_BOARD_OPTION == MR9100_TX_MAYON_MWL612))
        if(sysLifeTime>50)
        {
            if(First)
            {
                c.ena = GPIO_ENA;
                c.dir = GPIO_DIR_IN;
            #if (PIR_TRIGER_ACT_HIGH == 1)
                c.level = GPIO_LEVEL_LO;
            #else
                c.level = GPIO_LEVEL_HI;
            #endif
                c.inPullUp = GPIO_IN_PULLUP_DISA;

                gpioConfig(GPIO_GROUP_PIR,GPIO_BIT_PIR,&c);
                DEBUG_MAIN("-->PIR GPIO enable\n");
                First=0;
            }
        }
    #endif
    
    #if(HDMI_TXIC_SEL ==  HDMI_TX_EP952)
        EP952Controller_Task();
        //EP_HDMI_DumpMessage_952();
    #elif(HDMI_TXIC_SEL ==  HDMI_TX_CAT6613)

    #elif(HDMI_TXIC_SEL == HDMI_TX_IT66121)
        if((u8HdmiCnt % 8 ) == 0){ //while 1 uart cmd OSTimeDly(1), 量測間距約16tick檢查一次
            IT66121_Task();
        }
        u8HdmiCnt++;
    #endif

        if (sysPowerOffFlag)
            sysPowerOff(1);

        if ((userClicksnapshot || userClickvideoRec || userClickvoiceRec || userClickFormat) && gInsertNAND)
        {
            userClicksnapshot=0;
            userClickvideoRec=0;
            userClickvoiceRec=0;
            userClickFormat=0;
        }

        mainCheckChipID(CHIP1018_ID_CODE);

    }

}

void mainIpModuleTest(void)
{
//=======================//
#if (FPGA_TEST_TV == 1)
    FPGA_TV_Composite_Test();
#endif

#if (USB_DEVICE == 1)
    DEBUG_MAIN("# USB Device Init start \n");
    usb_DeviceInit();

    while(1)
    {
  	    OSTimeDly(3);
    }
#endif

#if FPGA_TEST_DTV_YUV656
    DEBUG_MAIN("mainTask: rtcInit()\n");
    FPGA_YUV601_Test();
    while(1);
#endif

#if TEST_Parallel_RGB
    DEBUG_MAIN("Test Parallel RGB\n");
    Parallel_RGB_888();
    while(1);
#endif

#if FPGA_TEST_DTV_720P_EP952_HDMI
    FPGA_720P_Test();
    EP_HDMI_952_Init(EP952_SF_32000Hz,EP952_VDO_1280x720P);
    while(1)
    {
		EP952Controller_Task();
        EP_HDMI_DumpMessage_952();
  	    //OSTimeDly(1);
    }
#endif

#if FPGA_TEST_DTV_1080I_EP952_HDMI
    FPGA_1080I_Test();
    EP_HDMI_952_Init(EP952_SF_32000Hz,EP952_VDO_1920x1080I);
    while(1)
    {
		EP952Controller_Task();
        EP_HDMI_DumpMessage_952();
  	    //OSTimeDly(1);
    }
#endif
#if CPU_PERFORMANCE_TEST
    {
        char    *Core_main_argv[5] = { "Core_main", "100", "10", "40", "0"};

        DEBUG_MAIN2("mainTask: Core_main()\n");
        Core_main(5, Core_main_argv);
        DEBUG_MAIN2("CPU TEST END\n");
        while(1);
    }
#endif

#if GPI_TEST

    DEBUG_MAIN("\n---GPI Test Start.................................\n");
//     DEBUG_MAIN("mainTask: gpiInit();\n");
//	gpiInit();
	GPITestDm9000();
//	nicTask();
#endif
#if 0 //IIS_TEST
	FS_Init(); // initialize file system: create semephore
    sdcInit();
    sdcMount();


    #if 0
    //case 0 : normal-mode, 8bit-8kHZ
    Audio_formate = nomo_8bit_8k;
    iisTest();
    #endif

    #if 0
    //case 1 : NonStop-mode, 8bit-8kHZ
    Audio_formate = nomo_8bit_8k;
    iisTest_NonStopMode();
    #endif

    #if 0
    //case 2 : normal-mode, 10 format
    for(Audio_formate=nomo_8bit_8k; Audio_formate< 0X0A;Audio_formate++)
        iisTest();
    #endif

    #if 1
    //case 3 : NonStop-mode, 10 format
    for(Audio_formate=nomo_8bit_8k; Audio_formate< 0X0A;Audio_formate++)
        iisTest_NonStopMode();
    #endif

    #if 0
    //case 4 : 2ch record
    iisTest_2ch();
    #endif


    while(1);
#endif

#if H264_TEST
	FS_Init(); // initialize file system: create semephore
    sdcInit();
    sdcMount();

    //H264Enc_LocalTest();//only test RC disable
    H264Dec_LocalTest();
    while(1);
#endif
#if H1_264TEST
	FS_Init(); // initialize file system: create semephore
    sdcInit();
    sdcMount();

    H1_H264Dec_LocalTest();
    while(1);
#endif
#if H1_264TEST_ENC
	FS_Init(); // initialize file system: create semephore
    sdcInit();
    sdcMount();

    H1_H264Enc_LocalTest();
    while(1);
#endif

#if SDRAM_TEST
    DEBUG_MAIN("\nSDRAM Test Start.................................\n");
  #ifdef MMU_SUPPORT
    if(0==SDRAM_Test((u32 *)(MEMORY_POOL_START_ADDR+1024*20)))
  #else
    if(0==SDRAM_Test((u32 *)MEMORY_POOL_START_ADDR))
  #endif
    {
        DEBUG_MAIN("DRAM Check Error!!\n");
        return;

    }
    else
    {
        DEBUG_MAIN("DRAM Check OK!!\n");
    }
    DEBUG_MAIN("\nSDRAM Test Finish.................................\n");
#endif

#if DMA_TEST
    DEBUG_MAIN("\n DMA Test Start.................................\n");
    if(0==marsDMA_Test() )
    {
        DEBUG_MAIN("DMA Check Error!!\n");
        return;

    }
    else
    {
        DEBUG_MAIN("DMA Check OK!!\n");
    }
    DEBUG_MAIN("\n DMA Test Finish.................................\n");
#endif

#if TCM_DMA_TEST
    TCM_DMATest();
#endif

#if 0
{
    int     i;
    u32     time1, time2, time3;

    DEBUG_MAIN("\nDisable External cache:\n");
    ExtCacheDisable();
    time1   = OSTime;
    for(i = 0; i < 10000000; i++);
    time2   = OSTime;
    time3   = time2 - time1;
    DEBUG_MAIN("time1   = %d\n", time1);
    DEBUG_MAIN("time2   = %d\n", time2);
    DEBUG_MAIN("time3   = %d\n\n", time3);

    DEBUG_MAIN("Enable External cache with burst 16:\n");
    ExtCacheEnable(0);
    time1   = OSTime;
    for(i = 0; i < 10000000; i++);
    time2   = OSTime;
    time3   = time2 - time1;
    DEBUG_MAIN("time1   = %d\n", time1);
    DEBUG_MAIN("time2   = %d\n", time2);
    DEBUG_MAIN("time3   = %d\n\n", time3);

    DEBUG_MAIN("Enable External cache with burst 8:\n");
    ExtCacheEnable(1);
    time1   = OSTime;
    for(i = 0; i < 10000000; i++);
    time2   = OSTime;
    time3   = time2 - time1;
    DEBUG_MAIN("time1   = %d\n", time1);
    DEBUG_MAIN("time2   = %d\n", time2);
    DEBUG_MAIN("time3   = %d\n\n", time3);
}
#endif

#if INTERNAL_STORAGE_TEST

    #if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_NORMAL)||(FLASH_OPTION == FLASH_NAND_9002_ADV))
		if (smcTest() == 1)
			DEBUG_MAIN("NAND gate Flash Access Testing Pass!\n");
		else
			DEBUG_MAIN("NAND gate Flash Access Testing Failed!\n");
    #endif
    #if (FLASH_OPTION == FLASH_SERIAL_EON)
		if (spiTest() == 1)
			DEBUG_MAIN("Serial Flash Access Testing Pass!\n");
		else
			DEBUG_MAIN("Serial Flash Access Testing Failed!\n");
    #endif
       #if CF_TEST
          if (cfTest() == 1)
    			DEBUG_MAIN("HDD Access Testing Pass!\n");
    		else
    			DEBUG_MAIN("HDD Access Testing Failed!\n");

       #endif
#endif

#if FPGA_TEST_DTV_720P_HDMI
		iduPlaybackMode(1280,720,1280);
		DEBUG_MAIN("mainTask: iduPlaybackMode();\n");
		FPGA_720P_HDMI_Test(1280,720);
		DEBUG_MAIN("mainTask: FPGA_720P_HDMI_Test();\n");
		//while(1)
		{
			OSTimeDly(1);
		}
		DEBUG_MAIN("mainTask: FPGA_TEST_DTV_720P_HDMI() test end;\n");
#endif

#if SDC_WRITE_READ_TEST
    MemoryTestFunction();
    while(1);
#endif

#if MCPU_TEST
    DEBUG_MCPU("\n MCPU Test Start.................................\n");
    if(marsMcpu_Test())
    {
        DEBUG_MCPU("MCPU check OK!\n");
    }
    else
    {
        DEBUG_MCPU("MCPU check Error!\n");
    }
#endif

#if AESDES_TEST
   TestEncrypt_2();
#endif



#if AUDIO_IN_OUT_SELFTEST
    AudioInOutTest();
#endif
    //===============//


}


void mainCheckChipID(int Code)
{
    int ID;

#if(FPGA_BOARD_A1018_SERIES)
    // unLuck
#else
  #if(CHIP_OPTION  == CHIP_A1018B)
        if( Code == 0x0ff )
          return;

        ID=Gpio2Level >>26;

        if(Code != ID)
        {
            if( (ID == 0x00) || ((Code==0x09) && (ID==0x01)) ) //工程品
            {
                //Exception
            }
            else
            {
               DEBUG_MAIN("ID mismatch:%d,%d\n",ID,Code);
               sysForceWDTtoReboot();
            }
        }   
   #elif((CHIP_OPTION == CHIP_A1019A) ||(CHIP_OPTION == CHIP_A1025A))   

   #elif(CHIP_OPTION == CHIP_A1021A)

   
   #endif
#endif
}


