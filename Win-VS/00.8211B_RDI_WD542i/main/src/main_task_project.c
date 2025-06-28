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
#if GFU_SUPPORT
#include "gfuapi.h"
#endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
#include "encrptyapi.h"
#endif

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
#elif (HW_BOARD_OPTION == MR9670_WOAN)
    #include "..\..\ui\inc\ui_woan_project.h"
#endif


#if(HW_BOARD_OPTION == MR6730_AFN)
#include "../../gpio/inc/ir_ppm.h"
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
u32  TutkLoginTaskPri;
u32  TutkRoutineTaskPri;
u32 TutkTransmissionTaskPri;
#if PLAY_MAIN_PAGE_SLIDE
u32 	 LogoTaskStack[LOGO_TASK_STACK_SIZE];
BOOLEAN ShowLogoFinish = FALSE;
#endif
u32 gIDU_clk_div_value=0;

#if (HW_BOARD_OPTION==MR6730_AFN)
#if (WDT_ALIVE_TEST)
u8 gWdtAliveLed_State=0;
u8 gWdtAliveLed_Cnt=0;
#endif  
#endif
u8 RFIU_Link = 0;


OS_MEM *icomBuffer;
OS_MEM *icomBuffer2;
OS_MEM *icomBuffer3;

#if 0
__align(4) INT8U	icommbuf[8][2560];
__align(4) INT8U	icommbuf2[1024][256];
__align(4) INT8U	icommbuf3[10][24576];
#else
extern u8 *icommbuf;/*for icomm 6030p used. by aher 20160627*/
extern u8 *icommbuf2;/*for icomm 6030p used. by aher 20160627*/
extern u8 *icommbuf3;/*for icomm 6030p used. by aher 20160627*/
#endif
/*
 *********************************************************************************************************
 *                                               Extern VARIABLES
 *********************************************************************************************************
 */
 
extern u8 gInsertCard;
extern u8 userClicksnapshot;
extern u8 userClickvideoRec;
extern u8 userClickvoiceRec;
extern u8 userClickFormat;
extern u8 got_disk_info;
extern u8 gInsertNAND;
extern u8 Main_Init_Ready;
extern u8 Main_LCD_Ready;

#if(FACTORY_TOOL == TOOL_ON)
extern u8 configsalix;
#endif
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];
/*
*********************************************************************************************************
*                                               Extern Function
*********************************************************************************************************
*/


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
#if(FACTORY_TOOL == TOOL_ON)
extern s32 uiMenuSetAutoOff(s8);
extern void uiSetEnterHiddenMode(void);
#endif
extern int dm9000_probe(void);
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
void mainIpModuleTest();


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

extern u8 icommInit(void);

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
#if ((HW_BOARD_OPTION == MR9670_DEMO_BOARD) || (HW_BOARD_OPTION == MR8200_RX_JIT) || (HW_BOARD_OPTION == MR8120_RX_JIT_LCD) ||\
    (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM719) || (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM720) || (HW_BOARD_OPTION == MR8120_RX_MAYON_MWM710) ||\
    (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM902) || (HW_BOARD_OPTION  == MR8120_RX_JIT_M703SW4) || (HW_BOARD_OPTION  == MR8200_RX_JIT_M703SN4))
    u32 temp,temp1;

    temp1 = SYS_CLK1;
    if(nPlus == 1)
    {
        gIDU_clk_div_value = SYS_CLK1 & 0x0000ff00;
        temp = gIDU_clk_div_value;
        if(temp & 0x100 == 0x100)
            temp = temp & 0xfffffeff;
        temp = temp >> 1;
    }
    else
    {
        temp = gIDU_clk_div_value;
    }
    SYS_CLK1= (temp1 & (~0x0000ff00)) | temp; // 216MHz /13=16.6 MHz
#endif
}

/*
 *********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

/*

Routine Description:

    The main task routine.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
extern s32 spi_master_out_cpu(u8 *  , u32  );
extern s32 spi_master_in(u8 , u8 * , u32 );
extern u8 WiFi_Mode;           // 1: STA Mode, 0: AP Mode.

void mainTask(void *pData)
{
    u8 err;
    u8 i;
    u8 *ptr;
    u32 sd_sec_num ;
	
    int First=1;
    GPIO_CFG c;
#if MEASURE_BOOT_TIME
    u32 time1,time2;
#endif


 	got_disk_info	 = 0;

#if SD_CARD_DISABLE
	gInsertCard 						= 1;
	global_diskInfo.avail_clusters		= 0x00001000;
	global_diskInfo.bytes_per_sector	= 4096;
	global_diskInfo.sectors_per_cluster = 32;
	global_diskInfo.total_clusters		= 0x00002000;
#else
	gInsertCard 	= 0;
#endif
	gInsertNAND 	= 0;

	// WDT Disable from boot code enable
	WDTctrBase = (8 | WDT_PULSEWIDTH);

	DEBUG_MAIN("mainTask Start, priority end = %d\r\n", MAIN_TASK_PRIORITY_END);
	if(MAIN_TASK_PRIORITY_END > 62)
	{
		DEBUG_WARERR("########################################\r\n");
		DEBUG_WARERR("##	mainTask Priority Overflow %d	##\r\n", MAIN_TASK_PRIORITY_END);
		DEBUG_WARERR("########################################\r\n");
		while(1);
	}
		
#if OS_TASK_STAT_EN > 0
	OSStatInit();					/* Initialize the statistics task */
#endif

#if USE_BUILD_IN_RTC
	DEBUG_MAIN("mainTask: rtcInit()\n");
	rtcInit();
#endif
	
    DEBUG_MAIN("############mainTask: timerInit()\n");
	timerInit();	/* time tick for uCOS-II */
#if(USB_DEVICE==1)
	got_disk_info=0;	//civic 070903
	gInsertCard=0;
	gInsertNAND=0;

    DEBUG_MAIN("mainTask: uiCheckSDCD()\n");
    uiCheckSDCD(1);
    
    //DEBUG_MAIN("mainTask: usbInit()\n");
    //usbInit();  /*CY 1023*/
    printf("# USB Device Init start \n");
    sdcInit();
    sdcMount();

    sdcReadSingleBlock(0, usb_device_buf);
    ptr = usb_device_buf;
    sd_sec_num = usb_device_buf[32] + (usb_device_buf[33]<<8) + (usb_device_buf[34]<<16) + (usb_device_buf[35]<<24);
    
    printf("SD sec num %x\n", sd_sec_num);
    Set_MSC_SD_Sec_Num(sd_sec_num);
    usb_DeviceInit();

    while(1)
    {
    //	WDT_Reset_Count();
  	    OSTimeDly(3);
    }
#endif	

    DEBUG_MAIN("mainTask: gpioInit()\n");
    gpioInit();

#if (HW_BOARD_OPTION==MR6730_AFN)
		
		//raising the reset signal of Video Decoder
		Gpio_Preceding();
		
		
		#if(USE_AUDIO_PATH_SW)
		//if(IsAuPathSw()!=AUPATH_BYPASS_OFF)
		{
			AuPathSw_OnOff(AUPATH_BYPASS_OFF);
		}
		#endif//#if(USE_AUDIO_PATH_SW)
		
		#if (!EXT_IR_CODE)
			#if( (HW_DERIV_MODEL==HW_DEVTYPE_CDVR_YD5150)&&(IR_CODE_TBL_SEL==1))			
			IrKeyCodeTbl_Init(IRKC_TYPE_B);//YD-CP002
			DEBUG_MAIN("\nIR KeyCode Mapping in type(%d)\n",IRKC_TYPE_B);
			
			#else			
			IrKeyCodeTbl_Init(IRKC_TYPE_A);//default
			DEBUG_MAIN("\nIR KeyCode Mapping in default type(%d)\n",IRKC_TYPE_A);
			#endif
		#else			
		IrKeyCodeTbl_Init(IRKC_TYPE_A);//always use IRKC_TYPE_A
		DEBUG_MAIN("\nIR KeyCode Mapping in type(%d)\n",IRKC_TYPE_A);
		#endif

#endif

       mainIpModuleTest(); //Lucian: 所有的測試code請放在這裡.
   
	   DEBUG_MAIN("mainTask: WDT_init()\n");
	   WDT_init();
	#if (HW_BOARD_OPTION==MR6730_AFN)
		#if (WDT_ALIVE_TEST)
		gWdtAliveLed_State=0;
		gWdtAliveLed_Cnt=0;
		#endif  
	#endif
	   WDT_Reset_Count();
	   DEBUG_MAIN("mainTask: i2cInit()\n");
	   i2cInit();

#if(BLE_SUPPORT)
    DEBUG_UI("mainTask: BLEInit()\n");
    BLEInit();
#endif

#if(HDMI_TXIC_SEL ==  HDMI_TX_EP952)
	   DEBUG_MAIN("mainTask: EP_HDMI_952_Init()\n");
	   EP_HDMI_952_Init();
#elif(HDMI_TXIC_SEL ==  HDMI_TX_CAT6613)
   
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

     #if ((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||\
          (HW_BOARD_OPTION == MR8100_RX_RDI_M512))  //Lucian: GPIO 切回LCD,解決閃屏問題.
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
        gpioSetLevel(GPIO_GROUP_LCD_EN, GPIO_BIT_LCD_EN, 0);
        gpioSetLevel(GPIO_GROUP_LCD_RESET, GPIO_BIT_LCD_RESET, 1) ;

        Gpio0Level &= ~0xffff8000;
        Gpio1Level &= ~0x0000007c;
        Gpio3Level &= ~0x18000000;

        Gpio0Ena |= 0xffff8000;
        Gpio1Ena |= 0x0000007c;
        Gpio3Ena |= 0x18000000;

        OSTimeDly(1);
        Gpio0Ena &= ~0xffff8000;
        Gpio1Ena &= ~0x0000007c;
        Gpio3Ena &= ~0x18000000;
	    DEBUG_UI("==LCD RESET==\n");
		//LCD_READY
    #endif
    #if (HW_BOARD_OPTION == MR8200_RX_RDI_M721)  //Lucian: GPIO 切回LCD,解決閃屏問題.
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
        gpioSetLevel(GPIO_GROUP_LCD_EN, GPIO_BIT_LCD_EN, 0);

        Gpio0Level &= ~0xffff8000;
        Gpio1Level &= ~0x0000007c;
        Gpio3Level &= ~0x18000000;

        Gpio0Ena |= 0xffff8000;
        Gpio1Ena |= 0x0000007c;
        Gpio3Ena |= 0x18000000;

        OSTimeDly(1);
        Gpio0Ena &= ~0xffff8000;
        Gpio1Ena &= ~0x0000007c;
        Gpio3Ena &= ~0x18000000;
	    DEBUG_UI("==LCD RESET==\n");
		//LCD_READY
    #endif
    #if (HW_BOARD_OPTION == MR8200_RX_TRANWO_D8589N || HW_BOARD_OPTION == MR8120_RX_TRANWO_D8589)
        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);

        Gpio0Level &= ~0xffff8000;
        Gpio1Level &= ~0x0000007c;
        Gpio3Level &= ~0x18000000;

        Gpio0Ena |= 0xffff8000;
        Gpio1Ena |= 0x0000007c;
        Gpio3Ena |= 0x18000000;

        OSTimeDly(1);
        Gpio0Ena &= ~0xffff8000;
        Gpio1Ena &= ~0x0000007c;
        Gpio3Ena &= ~0x18000000;
	    DEBUG_UI("==LCD RESET==\n");
    #endif
        Main_LCD_Ready=1;
  
#if ((FLASH_OPTION == FLASH_SERIAL_ESMT) || (FLASH_OPTION == FLASH_SERIAL_SST)|| (FLASH_OPTION == FLASH_SERIAL_WINBOND) || (FLASH_OPTION == FLASH_SERIAL_EON))
	   DEBUG_MAIN("mainTask: spiInit();\n");
	   spiInit();
  #if SWAP_SP1_SP3
	   DEBUG_MAIN("============SWAP_SP1_SP3 for Ice mode.===============\n");
  #endif
#endif
   
	   DEBUG_MAIN("mainTask: adcInit()\n");
	   adcInit(0);
	   
   		#if(HW_BOARD_OPTION == MR6730_AFN)
		   adcSetDAC_OutputGain(31);//0:max,31:mute
			#if (USE_EXT_DAC_CTRL)   
			DAC_OutEnBoth(0);
			DEBUG_ADC("\n=DAC=(Init:mute)=\n");
			#endif
		#endif	
	
	   DEBUG_MAIN("mainTask: i2cDeviceInit()\n");
	   i2cDeviceInit();
	   DEBUG_MAIN("mainTask: uiInit();\n");
  #if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) )
   
  #else
	   memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * 3); //clear display buffer., 1 buffer is not enough, at least 2 buffer
  #endif
  #if( (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) )
	   memset_hw_Word(Sub1Videodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.
  #endif
   
   
#if MULTI_CHANNEL_VIDEO_REC
	   InitVideoClipOption();
#endif
   
#if GFU_SUPPORT
	   DEBUG_MAIN("mainTask: gfuInit()\n");
	   gfuInit();
#endif

	   
#if (HW_BOARD_OPTION == MR6730_AFN)
	#if (USE_PWR_ONOFF_KEY && PWKEY_PWRCTRL)
	   
  		#if (HW_IR_SUPPORT == 1)
					 hwIrInit();
					 DEBUG_MAIN(">>>>>>>>>>>IR Init\n");
 		#endif
	   
	#endif
#endif

	   uiInit();
   
#if defined(NEW_UI_ARCHITECTURE)
	   UI_powerOnSequence1();  // Power-on sequence of the COMMAX UI
#endif


   
#if PLAY_MAIN_PAGE_SLIDE
	   DEBUG_MAIN("mainTask: uiMenuSetStartMovie()\n");
    #if ( (HW_BOARD_OPTION ==MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8120_RX_JESMAY ) ||\
          (HW_BOARD_OPTION == MR8120_RX_SKYSUCCESS) || (HW_BOARD_OPTION == MR8200_RX_COMMAX) ||\
          (HW_BOARD_OPTION == MR8200_RX_COMMAX_BOX) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||\
          (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
		   ShowLogoFinish = TRUE;
		   uiMenuSetStartMovie(2);
    #else
		   if (OSTaskCreate(DrawLogoTask, (void *) 0, LOGO_TASK_STACK, DISPLAY_LOGO_PRIORITY)!= OS_NO_ERR)
		   {
			   DEBUG_MAIN_ERR("Logo Task Create Fail!!!!\n");
			   ShowLogoFinish = TRUE;
		   }
    #endif
#endif
   
	   WDT_Reset_Count();
   
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
   
   
#if  (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (HW_BOARD_OPTION == MR6730_AFN) || (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) || (SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
		//RX has no preview mode.
   
#else
    #if PLAY_MAIN_PAGE_SLIDE
		   while(ShowLogoFinish == FALSE)
			   OSTimeDly(10);
    #endif
  #if !IS_COMMAX_DOORPHONE
	   MyHandler.MenuMode = VIDEO_MODE;
	   DEBUG_MAIN("mainTask: sysPreviewInit()\n");
	   sysPreviewInit(0);
  #endif
#endif
   
#if USB2WIFI_SUPPORT
    DEBUG_MAIN("mainTask: usbVcInit()\n");
    usbVcInit();
#endif
   
#if (HW_BOARD_OPTION==MR6730_AFN)
		   
		   {//uiOSDPreviewInit_WithoutDrawIcon
			   u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk2};
    #if (UI_PREVIEW_OSD == 1)
			   if (sysTVOutOnFlag)
			   {
				   uiMenuOSDReset();
				   iduTVOSDDisplay(OSD_BLK[sysTVOutOnFlag], 0, 0, TVOSD_SizeX, TVOSD_SizeY);
				   //without osdDrawPreviewIcon()
				   uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
			   }
			   else
			   {
				   uiMenuOSDReset();
				   iduOSDDisplay1(OSD_BLK[sysTVOutOnFlag], 0, 0, PANNEL_X, PANNEL_Y);
				   //without osdDrawPreviewIcon()
				   uiOsdEnable(OSD_BLK[sysTVOutOnFlag]);
			   }
    #endif
		   }
#else
		   
			   uiOSDPreviewInit();
		   
#endif 
   
#if(HW_BOARD_OPTION == MR6730_AFN)
#if (USE_PWR_ONOFF_KEY && PWKEY_PWRCTRL)
  //already initiated before
#else

  #if (HW_IR_SUPPORT == 1)
	   hwIrInit();
	   DEBUG_MAIN(">>>>>>>>>>>IR Init\n");
  #endif
#endif
#endif
   
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
	   DEBUG_MAIN("mainTask: NIC start();\n");
	   gpiInit();
  #endif
#endif
   
   
#if ( (CHIP_OPTION == CHIP_A1018A)|| (CHIP_OPTION == CHIP_A1018B) )
	   DEBUG_MAIN("mainTask: encryptInit();\n");
	   encryptInit();
#endif
   
#if(HW_BOARD_OPTION != MR6730_AFN)
  #if (HW_IR_SUPPORT == 1)
	   hwIrInit();
  #endif
#endif
   
   
#if PLAY_MAIN_PAGE_SLIDE
	   while(ShowLogoFinish == FALSE)
		   OSTimeDly(10);
	   OSTaskDel(DISPLAY_LOGO_PRIORITY);
#endif
#if (HW_BOARD_OPTION==MR8120_RX_HECHI)
	   PANEL_OFF();
#endif
   
#if RFIU_SUPPORT
	   DEBUG_MAIN("mainTask: RfiuInit()\n");
	   RfiuInit();
       #if ((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)\
        || (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
       sysRFRxInMainCHsel = iconflag[UI_MENU_SETIDX_BOOT_CH];
	   DEBUG_MAIN("===> %d sysRFRxInMainCHsel = %d \r\n", __LINE__, sysRFRxInMainCHsel);
       #endif
#endif
   
	   //===================================While(1)=========================//
	   //----------- Enter mainTask while loop --------------
	   DEBUG_MAIN("mainTask: OSTaskChangePrio.\n");
	   OSTaskChangePrio(MAIN_TASK_PRIORITY_START, MAIN_TASK_PRIORITY_END);

   		#if (HW_BOARD_OPTION==MR6730_AFN)
		UI_KeyLock(UI_LOCK_SEC_MAX);
		DEBUG_MAIN(">>>>>>>>>>>Main_Init_Ready\n");
		#endif
		
#if defined(NEW_UI_ARCHITECTURE) || (HW_BOARD_OPTION==MR9670_WOAN)
	   UI_powerOnSequence2();
#endif
	   IntFiqMask=0x40; //Lucian: 目前有開機FIQ Mask 被修改. 先採取治標作法.
	   sysDeadLockMonitor_Reset();
   
 #if (RFIU_TEST==1)
	   DEBUG_MAIN("\n RFIU Test Start.................................\n");
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
   
 #if RFIU_SUPPORT
	   rfiu_Start();
 #endif
   
	
#if MEASURE_BOOT_TIME
	   time2=OSTimeGet();
	   DEBUG_WARERR("--->Booting Time =%d ms\n",(time2-time1)*50);
#endif

#if ICOMMWIFI_SUPPORT
//aher test for icomm
printf("OSMemCreate : icomm used.\n");
printf("Start pri = %d\n",ICOMM_TASK_01_PRIORITY);

printf("\x1B[96m icommbuf:%08x \x1B[0m\n",icommbuf);

icomBuffer = OSMemCreate(icommbuf, 8, 2560,&err);	//for icomm 6030p used. by aher 20160722
	if(icomBuffer==0)
		printf("OSMemCreate fail.\n");
	else
		printf("OSMemCreate success.\n");
	printf("\x1B[96m icommbuf2:%08x \x1B[0m\n",icommbuf2);

icomBuffer2 = OSMemCreate(icommbuf2, 1024,256,&err);	//for icomm 6030p used. by aher 20160722
	if(icomBuffer2==0)
		printf("OSMemCreate2 fail.\n");
	else
		printf("OSMemCreate2 success.\n");

printf("\x1B[96m icommbuf3:%08x \x1B[0m\n",icommbuf3);

//icomBuffer3 = OSMemCreate(icommbuf3, 10,4096*3,&err);	//for icomm 6030p used. by aher 20160722
icomBuffer3 = OSMemCreate(icommbuf3, 10,4096*6,&err);	//for icomm 6030p used. by sean 20170426

	if(icomBuffer3==0)
		printf("OSMemCreate3 fail.\n");
	else
		printf("OSMemCreate3 success.\n");
	

printf("\x1B[96m icommbuf+ = %08x \x1B[0m\n",((u8 *)icommbuf+8*2560));
printf("\x1B[96m icommbuf2+ = %08x \x1B[0m\n",((u8 *)icommbuf2+1024*256));
printf("\x1B[96m icommbuf3+ = %08x \x1B[0m\n",((u8 *)icommbuf3+10*4096*6));


//#if 1
//	printf("\033[40;31mCreate icomm 6030p task2\n\033[0m");
       //  spi_master_out_cpu(u8 * pucSrc, u32 unSize);

	   gpioSetLevel(0 ,0 ,1);		//icomm power on(TBD default is on)
	printf("\033[40;31mCreate icomm 6030p task2\n\033[0m");
 #if 0 //test for aher, marked by sean.
 j=0;
//while(j<10)
while(j<1)
    {
     	int i;
		u8 tem_buf[5]={0x05,0xc0,0x00,0x00,0x10};
		u8 buf222[50];
Spi2Ctrl |= 0x00000010; 		
        spi_master_out_cpu(tem_buf,5);	
Spi2Ctrl &= ~0x00000010;	 
Spi2Ctrl=0;		
Spi2Ctrl |= 0x00000010; 
		spi_master_in(0x00,buf222, 4);  //DMA receive
Spi2Ctrl &= ~0x00000010;	 
Spi2Ctrl=0;			
		printf("buf44 = ");
		for(i=0;i<4;i++)
		 printf("%x ",buf222[i]);
		printf("\n");

/*
Spi2Ctrl |= 0x00000010; 		
        spi_master_out_cpu(tem_buf,5);	
Spi2Ctrl &= ~0x00000010;	 
Spi2Ctrl=0;


Spi2Ctrl |= 0x00000010; 
		spi_master_in_cpu(0x00,buf222, 4);  //DMA receive
Spi2Ctrl &= ~0x00000010;	 
Spi2Ctrl=0;			
		printf("buf222 = ");
		for(i=0;i<4;i++)
		 printf("%x ",buf222[i]);
		//if((buf222[0] == 0xff) && (buf222[1] == 0xff) && (buf222[2] == 0xff) && (buf222[3] == 0xff))
		//	sysForceWDTtoReboot();
		printf("\n");
*/		
       printf("Gpio1Ena = %X \n",Gpio1Ena);
     //  printf("Gpio1Ena = %X \n",Gpio1Ena);		
      j++;
        }   
#endif
	//OSTaskCreate(ICOMM_INIT_TASK, (void *)0, ICOMM_INIT_TASK_STACK, ICOMM_INIT_TASK_PRIORITY);
    DEBUG_UI("\x1B[96mmainTask: icomminit() \x1B[0m\n");
	icommInit();
//#endif
#endif


#if(HOME_RF_SUPPORT)
	   DEBUG_UI("\x1B[96mmainTask: homeRFInit() \x1B[0m\n");
	   homeRFInit();
#endif

#if(CLOUD_SUPPORT)
	   DEBUG_UI("\x1B[96mmainTask: CloudServiceInit() \x1B[0m\n");
	   CloudServiceInit();
#endif

#if MENU_DONOT_SHARE_BUFFER
    OSTimeDly(60);
    #if RFRX_FULLSCR_HD_SINGLE
      if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight>=720)
         iduPlaybackMode(1280/2,720/2,RF_RX_2DISP_WIDTH);
      else
         iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
    #else
      iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
    #endif
    IduIntCtrl |= IDU_FTCINT_ENA;
    sysEnMenu=0;
    #if SD_CARD_DISABLE

    #elif(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)  //Lucian: @VBM, 卡的出始化放在最後面.
	   DEBUG_MAIN("mainTask: uiCheckSDCD()\n");
	   uiCheckSDCD(1);
    #endif
    Main_Init_Ready = 1;
	OSFlagPost(gUiStateFlagGrp, FLAGUI_MAIN_INIT_READY, OS_FLAG_SET, &err);
    //osdDrawPlaybackArea(2);
    for(i=0;i<4;i++)
        uiRFStatue[i] = UI_RF_STATUS_OTHER;

#else

#if SD_CARD_DISABLE
#else
    DEBUG_MAIN("mainTask: uiCheckSDCD()\n");
    uiCheckSDCD(1);
#endif

	Main_Init_Ready = 1;
	OSFlagPost(gUiStateFlagGrp, FLAGUI_MAIN_INIT_READY, OS_FLAG_SET, &err);
#endif   

#if (TUTK_SUPPORT==1)
    TutkLoginTaskPri = IOTC_LOGIN_TASK_PRIORITY;
    TutkRoutineTaskPri = IOTC_ROUTINE_TASK_PRIORITY;
    //20140210
    TutkTransmissionTaskPri=IOTC_TRANSMISSION_TASK_PRIORITY;
    
#if ICOMMWIFI_SUPPORT
    if(WiFi_Mode == 0)   //AP mode
    	OSTimeDly(150); //7.5s
#endif    	

    tutkSampleInit();
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

    #if 0 //((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP))
    if(RFIU_Link == 0)
    {
        if(gRfiuUnitCntl[0].OpMode == RFIU_TX_MODE)
        {
            gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
            RFIU_Link = 1;
        }
    }
    #endif
    #if((HW_BOARD_OPTION  == MR8120_TX_RDI) ||(HW_BOARD_OPTION == MR8120_TX_RDI_CA671)||\
		   (HW_BOARD_OPTION == MR8120_TX_RDI_CA530)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA672)||\
		   (HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON)  || (HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C)||\
		   (HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||(HW_BOARD_OPTION == MR8120_TX_RDI_CL692) ||\
		   (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))  //打開PIR input
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
   
		   mainCheckChipID(CHIP1016_ID_CODE);
   
	   }
   
   
#if(HW_BOARD_OPTION == MR6730_AFN)
#if (USE_MULCH_REC_SYNC)
	   {
		   INT8U err;
		   
		   gMulChAsfSyncReadySemEvt[0] = OSSemDel(gMulChAsfSyncReadySemEvt[0], OS_DEL_ALWAYS, &err);
		   gMulChAsfSyncReadySemEvt[1] = OSSemDel(gMulChAsfSyncReadySemEvt[1], OS_DEL_ALWAYS, &err);
	   }
#endif 
#endif //#if(HW_BOARD_OPTION == MR6730_AFN)
}

void mainIpModuleTest()
{




//=======================//
#if (FPGA_TEST_TV == 1)
    FPGA_TV_Composite_Test();
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
    EP_HDMI_952_Init();
    while(1)
    {
		EP952Controller_Task();
        EP_HDMI_DumpMessage_952();
  	    //OSTimeDly(1);
    }
#endif

#if FPGA_TEST_DTV_1080I_EP952_HDMI
    FPGA_1080I_Test();
    EP_HDMI_952_Init();
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

        DEBUG_MAIN("mainTask: Core_main()\n");
        Core_main(5, Core_main_argv);
    }
#endif

#if GPI_TEST

    DEBUG_MAIN("\n---GPI Test Start.................................\n");
//     DEBUG_MAIN("mainTask: gpiInit();\n");
//	gpiInit();
	GPITestDm9000();
//	nicTask();
#endif
#if IIS_TEST

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
    sdcInit();
    sdcMount();

    //H264Enc_LocalTest();//only test RC disable
    H264Dec_LocalTest();
    while(1)
        ;
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
    #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
        (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
       #if CF_TEST
          if (cfTest() == 1)
    			DEBUG_MAIN("HDD Access Testing Pass!\n");
    		else
    			DEBUG_MAIN("HDD Access Testing Failed!\n");

       #endif
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
    DEBUG_MAIN("\n MCPU Test Start.................................\n");
    if(marsMcpu_Test())
    {
        DEBUG_MCPU("MCPU check OK!\n");
    }
    else
    {
        DEBUG_MCPU("MCPU check Error!\n");
    }
#endif

#if (USB_HOST == 1)
	printf("# USB Host Init start \n");
    usbHostInit();

    while(1)
    {
  	    OSTimeDly(3);
    }
#endif

#if AUDIO_IN_OUT_SELFTEST
    AudioInOutTest();
#endif
    //===============//


}


void mainCheckChipID(int Code)
{
    int ID,Temp,ICVersion;

#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    // unLuck
#else
  #if((CHIP_OPTION  == CHIP_A1016A)) //A1016 series
        if(Code == 0x0ff)
          return;

        //Check old version ID
        ICVersion= *((volatile unsigned *)(SysCtrlBase + 0X0058));
        if(ICVersion == 0x000c1220)
            return;

        Temp=Gpio2Level >>28;
        ID=(( (Temp>>3) & 0x01 )<<0) | (( (Temp>>2) & 0x01 )<<1) | (( (Temp>>1) & 0x01 )<<2) | (( (Temp>>0) & 0x01 )<<3);

        if(Code != ID)
        {
        #if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
            if( ((Code == 0x07) && (ID == 0x06) ) )  
        #elif(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
            if( ( (Code == 0x0a) && (ID == 0x0d) ) || ( (Code == 0x05) && (ID == 0x0a) ) || ( (Code == 0x05) && (ID == 0x0d) ) )  
        #elif(SW_APPLICATION_OPTION == MR8120_RFCAM_TX1)   
            if( ( (Code == 0x08) && (ID == 0x0d) ) ) 
        #elif(SW_APPLICATION_OPTION == MR8120_RFCAM_RX1)   
            if( ( (Code == 0x01) && (ID == 0x07) ) ) 
        #else
            if( ((Code == 0x03) && (ID == 0x09)) ||  ((Code == 0x01) && (ID == 0x0e)) )
        #endif
            {
                //Exception
            }
            else
            {
               DEBUG_MAIN_ERR("ID mismatch:%d,%d\n",ID,Code);
               sysForceWDTtoReboot();
            }
        }

   #elif(CHIP_OPTION == CHIP_A1020A)
        if(Code == 0x0ff)
          return;

        ID==Gpio2Level >>28;
        
        if(ID == 0x00) //for 1st engineer sample
           return;   

        if(Code != ID)
        {
            DEBUG_MAIN_ERR("ID mismatch:%d,%d\n",ID,Code);
            sysForceWDTtoReboot();
        }
   #elif(CHIP_OPTION == CHIP_A1026A) 
        
   #else

   #endif
#endif
}

