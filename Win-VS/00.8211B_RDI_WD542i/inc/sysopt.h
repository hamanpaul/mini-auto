/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    sysopt.h

Abstract:

    The system option.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#ifndef __SYS_OPT_H__
#define __SYS_OPT_H__

#include "sysoptdef.h"

/*
Step 1: 選擇PCB board
========================== Hardware board option===================
*  A1013_FPGA_BOARD     - A1013 FPGA Board
*  A1013_REALCHIP_A     - A1013 Real chip EVB

*  A1016_FPGA_BOARD     - A1016 FPGA Board
*  A1016B_FPGA_BOARD	- A1016B FPGA Board
*  A1016_REALCHIP_A     - A1016 Real chip EVB
*  MR8120_TX_DEMO_BOARD  - A8120 TX 128 PIN Demo board
*  MR8120_TX_RDI        - A8120 TX 128 PIN RDI board
*  MR8200_RX_DEMO_BOARD  - A8200 RX 216 PIN Demo board
*  MR8200_RX_RDI        - A8200 RX 216 PIN RDI board
*  A1018_FPGA_BOARD     - A1018 FPGA Board
*  A1018B_FPGA_BOARD     - A1018B FPGA Board
********************************************************************
*/

#define HW_BOARD_OPTION   MR8211B_TX_RDI_WD542I
/*
Step 2: 選擇IC 內部型號 
================================ IC type ==========================
#CHIP_A1013A             
#CHIP_A1013B             
#CHIP_A1016A             
#CHIP_A1016B             
#CHIP_A1018A           
********************************************************************
*/
#if(HW_BOARD_OPTION == A1013_FPGA_BOARD)
    #define CHIP_OPTION                     CHIP_A1013B 
    #define FIX_A1018A_BUG_RF               0
#elif(HW_BOARD_OPTION == A1018_FPGA_BOARD)
    #define CHIP_OPTION                     CHIP_A1018A
    #define FIX_A1018A_BUG_RF               0
#elif(HW_BOARD_OPTION == A1018B_FPGA_BOARD)
    #define CHIP_OPTION                     CHIP_A1018B
    #define FIX_A1018A_BUG_RF               0    
#elif(HW_BOARD_OPTION == A1016B_FPGA_BOARD)
    #define CHIP_OPTION  					CHIP_A1016B
    #define FIX_A1018A_BUG_RF               0
#elif((HW_BOARD_OPTION == A1018_EVB_128M) || (HW_BOARD_OPTION == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) || (HW_BOARD_OPTION == MR6790_DB) || (HW_BOARD_OPTION == MR9211_DB) || (HW_BOARD_OPTION == MR9120_TX_DB) || (HW_BOARD_OPTION == MR9600_RX_DB) || (HW_BOARD_OPTION == MR9200_RX_DB) || (HW_BOARD_OPTION == MR9300_RX_DB) || (HW_BOARD_OPTION  == MR9600_RX_OPCOM) )
    #define CHIP_OPTION                     CHIP_A1018A
    #define FIX_A1018A_BUG_RF               0    //RF-1 有bug,用RF-2取代
#elif(HW_BOARD_OPTION == A1026A_FPGA_BOARD)
    #define CHIP_OPTION  					CHIP_A1026A
    #define FIX_A1018A_BUG_RF               0
#else
    #define CHIP_OPTION                     CHIP_A1020A
    #define FIX_A1018A_BUG_RF               0
#endif


/*
Step 3: 設定 IC 操作頻率 
========================== IC operation frequency===================           
********************************************************************
*/
  #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
      #define SYS_CPU_CLK_FREQ      32000000
      #define ADC_IIS_CLK_FREQ      32000000
	  #define IDU_CLK_FREQ          32000000
	  #define RFI_CLK_FREQ          32000000
      
  #elif((HW_BOARD_OPTION == A1018_EVB_128M) || (HW_BOARD_OPTION == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) || (HW_BOARD_OPTION == MR6790_DB) || (HW_BOARD_OPTION == MR9211_DB) || (HW_BOARD_OPTION == MR9120_TX_DB) || (HW_BOARD_OPTION == MR9600_RX_DB) || (HW_BOARD_OPTION == MR9200_RX_DB) || (HW_BOARD_OPTION == MR9300_RX_DB) || (HW_BOARD_OPTION  == MR9600_RX_OPCOM) )
    #if 1  //Lucian: sysclk=160,ddrclk=240
      #define SYS_CPU_CLK_FREQ      160000000
      #define ADC_IIS_CLK_FREQ      48000000
	  #define IDU_CLK_FREQ          48000000
	  #define RFI_CLK_FREQ          32000000
    #else  //Lucian: 超頻用, sysclk=180,ddrclk=270
      #define SYS_CPU_CLK_FREQ      180000000
      #define ADC_IIS_CLK_FREQ      54000000
	  #define IDU_CLK_FREQ          54000000
	  #define RFI_CLK_FREQ          36000000
    #endif
  #elif(HW_BOARD_OPTION  == MR8600_RX_DEMO_BOARD)
     #if 1
      #define SYS_CPU_CLK_FREQ      108000000   
      #define ADC_IIS_CLK_FREQ      54000000
	  #define IDU_CLK_FREQ          54000000
	  #define RFI_CLK_FREQ          36000000
	 #else
      #define SYS_CPU_CLK_FREQ      96000000   
      #define ADC_IIS_CLK_FREQ      48000000
	  #define IDU_CLK_FREQ          48000000
	  #define RFI_CLK_FREQ          32000000
	 #endif	

  #elif (HW_BOARD_OPTION  == MR8120_TX_SKYSUCCESS_AV) 
     #if 0
      #define SYS_CPU_CLK_FREQ      108000000   
      #define ADC_IIS_CLK_FREQ      54000000
	  #define IDU_CLK_FREQ          54000000
	  #define RFI_CLK_FREQ          36000000
	 #else
      #define SYS_CPU_CLK_FREQ      96000000   
      #define ADC_IIS_CLK_FREQ      48000000
	  #define IDU_CLK_FREQ          48000000
	  #define RFI_CLK_FREQ          32000000
	 #endif	
     
  #elif( (HW_BOARD_OPTION  == MR8120_RX_JESMAY) || (HW_BOARD_OPTION  == MR8120_TX_JESMAY) || (HW_BOARD_OPTION  == MR8600_RX_SKYSUCCESS) ||\
         (HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION  == MR8120_TX_TRANWO3))
     #if 0
      #define SYS_CPU_CLK_FREQ      108000000   
      #define ADC_IIS_CLK_FREQ      54000000
	  #define IDU_CLK_FREQ          54000000
	  #define RFI_CLK_FREQ          36000000
	 #else
      #define SYS_CPU_CLK_FREQ      96000000   
      #define ADC_IIS_CLK_FREQ      48000000
	  #define IDU_CLK_FREQ          48000000
	  #define RFI_CLK_FREQ          32000000
	 #endif	     
  #else
     #if 1
      #define SYS_CPU_CLK_FREQ	  108000000   
      #define ADC_IIS_CLK_FREQ	  54000000
      #define IDU_CLK_FREQ		  54000000
      #define RFI_CLK_FREQ		  36000000
     #else
      #define SYS_CPU_CLK_FREQ	  96000000	 
      #define ADC_IIS_CLK_FREQ	  48000000
      #define IDU_CLK_FREQ		  48000000
      #define RFI_CLK_FREQ		  32000000
     #endif
  #endif


/*
Step 4: 設定產品應用 
========================== Product application ===================    
#MR8120_RFCAM_TX1  
#MR8120_RFCAM_TX2         

#MR8200_RFCAM_RX1         
#MR8200_RFCAM_RX1RX2

#MR8120_RFAVSED_RX1
#MR8600_RFAVSED_RX1RX2       

#MR6730_CARDVR_2CH           
********************************************************************
*/

#define SW_APPLICATION_OPTION           MR8211_RFCAM_TX1

//==============Performace Option=================//
/*CY 0907*/
#define DINAMICALLY_POWER_MANAGEMENT    1    // 1: dinamically tune clock module by module, 0: otherwise /* Peter */
#define DCF_WRITE_STATISTIC             1    // Lucian:統計DCF write 流量,optimize rate control.


//==============Test Option=================//
//--元件測試: 請放置於mainTask(), while(1)之前--//
#define IIS_TEST                        0
#define IIC_TEST                        0
#define SDRAM_TEST                      0    //SDRAM r/w test when power-on
#define EMBEDED_SRAM_TEST               0
#define INTERNAL_STORAGE_TEST			0		/* Internal storage test */
#define FPGA_TEST_TV                    0       // FPGA test Tv
#define DMA_TEST                        0
#define TCM_DMA_TEST                    0
#define GPI_TEST                        0
#define CIU_TEST                        0
#define SUB_TV_TEST                     0       
#define MCPU_TEST                       0
#define FPGA_TEST_DTV_YUV656            0
#define FPGA_TEST_DTV_720P_EP952_HDMI   0
#define FPGA_TEST_DTV_1080I_EP952_HDMI  0
#define FPGA_TEST_DTV_720P_HDMI		    0
#define TEST_Parallel_RGB               0
#define H264_TEST						0
#define TIMER_TEST                      0
#define CPU_PERFORMANCE_TEST            0
#define MULTI_CH_SDC_WRITE_TEST         0
#define AUDIO_IN_OUT_SELFTEST           0
#define CIU_DECIMATION_TEST             0  // SP+OSD Amon (140521)
#define SIMU1080I_VIA720P               0 
#define IDU_OSD_TEST                    0  // OSD BRI TEST Amon (140721)
//--整合測試--//
#define Auto_Video_Test                 0    // 錄影到卡滿 --> 檔案逐一播放 -->Delete all file.
#define SDC_WRITE_READ_TEST             0    // SD card r/w test, SDRAM r/w test.
#define CDVR_TEST                       0    // Test CDVR file System
#define ASF_SPLIT_FILE                  0
#define RF_RX_AUDIO_SELFTEST            0
#define iHome_LOG_TEST                  0
#define PLAYBEEP_TEST                   0

#include "application.h"
#include "project.h"

//==============Special Purpose =================//
#define MAKE_SPI_BIN	                0	    /* index of Making bin from spi, 1-> Enable, 0-> Disable */
#define ERASE_SPI	                    0		/* Sepcial purpose, 1-> Enable erase whole chip, 0-> Disable */
#define RW_PROTECT_SPI                  1       /* Read and Write protect function , 1 -> Enable, 0 -> Disable */
#if ((HW_BOARD_OPTION == MR8200_RX_RDI_M721) && (PROJ_OPT == 5))
#define FS_NEW_VERSION					1
#define SDC_SEM_REPAIR					1
#else
#define FS_NEW_VERSION					0
#define SDC_SEM_REPAIR					0
#endif
//==================Debug Option ==================//
#define DISABLE_OSDICON                 0    //for saving memory; 1:enable osd icon 0:disable osd icon

#define GET_BAYERDATA_SIU               0    //Get Bayer data only.
#define GET_SIU_RAWDATA_PURE            0    //Disable OB,Lens shading,pre-gamma,AE eanble only,Fix AGC=8;
#define SELECT_ADJUST_EL_AGC            0    // 1:adjust EL; 0: adjust AGC (手動調整 EL/AGC)

#define CAPTURE_IMG_ON_PREVIEW          0    //test mode for capturing image on video

//Lucian use
#define MPEG_DEBUG_ENA_LUCIAN           0   //GPIO1-5: 量測MPEG  operation time
#define IIS_DEBUG_ENA                   0   //GPIO1-6: Measure IIS DMA opteration time
#define FIQ_DEBUG_ENA_LUCIAN            0   //GPIO1-7: Measure FIQ intr op time
#define SDWRI_DEBUG_ENA                 0   //GPIO1-9: Measure SD WRITE DMA op time
#define SW_AE_DEBUG_ENA                 0   //GPIO1-10: Measure Software-AE get Y-avg op time
#define IRQ_DEBUG_ENA_LUCIAN            0   //GPIO1-11: Measure IRQ intr op time
#define TEMP_DEBUG_ENA                  0   //GPIO1-12: Measure mpeg4 header sw op time.
#define SW_MD_DEBUG_ENA                 0   //GPIO1-14: Measure Motion Detect sw op time.
#define IR_DEBUG_ENA                    0   //GPIO1-16: Measure IR interrupt timming.
#define TV_DEBUG_ENA                    0   //GPIO1-18: Measure playback timming on TV-out.
#define SDC_DEBUG_ENA                   0   //GPIO1-24: Measure SD(DCF) write op time.
#define DMA_DEBUG_ENA                   0   //GPIO1-25: Memsure memcpy_hw op time.
#define OVERLAY_DRAW_DEBUG_ENA          0
#define SYSBACK_DEBUG_ENA               0
#define DRAWTIMEONVIDEOClIP_DEBUG_ENA   0
#define WDT_DEBUG_ENA                   0
#define CIU_DEBUG_INTR                  0
#define MD_DEBUG_ENA                    0
#define RF_LETENCY_DEBUG_ENA            0
#define RF_TX_RATECTRL_DEBUG_ENA        0
#define RF_TX_AUDIORET_DEBUG_ENA        1

//Peter use
#define FIQ_DEBUG_ENA                   0
#define IRQ_DEBUG_ENA                   0
#define MPEG_DEBUG_ENA                  0
#define OS_DEBUG_ENA                    0
#define FS_DEBUG_ENA                    0
#define ASF_DEBUG_ENA                   0
#define EBELL_OS_DEBUG_ENA              0
#define EBELL_IRQ_DEBUG_ENA             0
#define AUDIO_DEBUG_ENA                 0

//Allen use
#define DEBUG_PROGRAM_MSG					/* Show Program massage for UI Lib, Code, Wave Files update */
#define MEASURE_BOOT_TIME               1
//#undef	DEBUG_PROGRAM_MSG					/* */

#define VIDEO_STARTCODE_DEBUG_ENA       0


//----DEBUG LED---//
#if(HW_BOARD_OPTION == A1013_FPGA_BOARD)
    #define  DEBUG_SIU_FRAM_STR_INTR_USE_LED6        0
    #define  DEBUG_SIU_BOTFD_END_INTR_USE_LED7       0

    #define  DEBUG_ISU_INTR_USE_LED6                 0

    #define  DEBUG_IDU_INTR_USE_LED6                 0

    #define  DEBIG_CIU_FRAM_END_INTR_USE_LED6        0

    #define  DEBUG_MPEG_OPERRATION_USE_LED6          0
#elif((HW_BOARD_OPTION == A1016_FPGA_BOARD) || (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define  DEBUG_SIU_FRAM_STR_INTR_USE_LED6        0
    #define  DEBUG_SIU_BOTFD_END_INTR_USE_LED7       0

    #define  DEBUG_ISU_INTR_USE_LED6                 0

    #define  DEBUG_IDU_INTR_USE_LED6                 0

    #define  DEBIG_CIU_FRAM_END_INTR_USE_LED6        0

    #define  DEBUG_MPEG_OPERRATION_USE_LED6          0

#elif((HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD))
    #define  DEBUG_SIU_FRAM_STR_INTR_USE_LED6        0
    #define  DEBUG_SIU_BOTFD_END_INTR_USE_LED7       0

    #define  DEBUG_ISU_INTR_USE_LED6                 0

    #define  DEBUG_IDU_INTR_USE_LED6                 0

    #define  DEBIG_CIU_FRAM_END_INTR_USE_LED6        0

    #define  DEBUG_MPEG_OPERRATION_USE_LED6          0
#elif(HW_BOARD_OPTION == A1018_FPGA_BOARD)
    #define  DEBUG_SIU_FRAM_STR_INTR_USE_LED6        0
    #define  DEBUG_SIU_BOTFD_END_INTR_USE_LED7       0

    #define  DEBUG_ISU_INTR_USE_LED6                 0

    #define  DEBUG_IDU_INTR_USE_LED6                 0

    #define  DEBIG_CIU_FRAM_END_INTR_USE_LED6        0

    #define  DEBUG_MPEG_OPERRATION_USE_LED6          0
#elif(HW_BOARD_OPTION == A1018B_FPGA_BOARD)
    #define  DEBUG_SIU_FRAM_STR_INTR_USE_LED6        0
    #define  DEBUG_SIU_BOTFD_END_INTR_USE_LED7       0

    #define  DEBUG_ISU_INTR_USE_LED6                 0

    #define  DEBUG_IDU_INTR_USE_LED6                 0

    #define  DEBIG_CIU_FRAM_END_INTR_USE_LED6        0

    #define  DEBUG_MPEG_OPERRATION_USE_LED6          0
#endif    


//============================Common Option======================//
#if( (LCM_OPTION == LCM_P_RGB_888_HannStar) || (LCM_OPTION == LCM_P_RGB_888_Innolux) || (LCM_OPTION == LCM_P_RGB_888_AT070TN90)|| (LCM_OPTION == LCM_P_RGB_888_SY700BE104) || \
     (LCM_OPTION == LCM_P_RGB_888_ZSX900B50BL)|| (LCM_OPTION == LCM_P_RGB_888_FC070227) || (LCM_OPTION == LCM_P_RGB_888_ILI6122))
   #define NEW_IDU_BRI                     1
   #define NEW_IDU_BRI_PANEL_INTLX         0
#else
   #define NEW_IDU_BRI                     1
   #define NEW_IDU_BRI_PANEL_INTLX         0
#endif


#if TV_DISP_BY_IDU
   #if(SYS_CPU_CLK_FREQ == 32000000)
      #define TV_DISP_DELAY_TIME_NTSC    600
      #define TV_DISP_DELAY_TIME_PAL     750

   #elif(SYS_CPU_CLK_FREQ == 48000000)
      #define TV_DISP_DELAY_TIME_NTSC     1200
      #define TV_DISP_DELAY_TIME_PAL      1500
   #else
      #define TV_DISP_DELAY_TIME_NTSC     1200
      #define TV_DISP_DELAY_TIME_PAL      1500
   #endif
#endif



#if MEMCPY_DMA
    #define CopyMemory memcpy_hw
#else
    #define CopyMemory memcpy
#endif

#define AVSYNC							AUDIO_FOLLOW_VIDEO
#define PLAYBACK_METHOD                 PLAYBACK_IN_IIS_ISR

/* MPEG4 file container option
 *  MPEG4_CONTAINER_MP4 - MP4 file.
 *  MPEG4_CONTAINER_ASF - ASF file.
 *  MPEG4_CONTAINER_AVI - AVI file.
 *  MPEG4_CONTAINER_MOV - MOV file.
 */
#define MPEG4_CONTAINER_OPTION      MPEG4_CONTAINER_ASF


/* Video Codec Option */
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )  
#define VIDEO_CODEC_OPTION          H264_CODEC
#else
#define VIDEO_CODEC_OPTION          MPEG4_CODEC
#endif

#if (VIDEO_CODEC_OPTION == H264_CODEC)	
#define MAX_VIDEO_FRAME_BUFFER    4                  //use 4 PN buffer
#elif (VIDEO_CODEC_OPTION == MPEG4_CODEC)	
#define MAX_VIDEO_FRAME_BUFFER    4                  //use 4 PN buffer
#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)	
#define MAX_VIDEO_FRAME_BUFFER    3                  //use 3 Pk buffer
#endif

/* Jpeg encode opration mode option
*  JPEG_OPMODE_FRAME -Frame mode
*  JPEG_OPMODE_SLICE -slice mode
*/
#define JPEG_ENC_OPMODE_OPTION  JPEG_OPMODE_SLICE

#define JPEG_DRI_ENABLE     0   // 1: Enable JPEG encode DRI, 0: Otherwise

/* Jpeg decode opration mode option
*  JPEG_DECODE_MODE_FRAME - Frame mode
*  JPEG_DECODE_MODE_SLICE - Slice mode, invalid if chip is PA9001D or PA9002B!!
*/
#define JPEG_DECMODE_OPTION  JPEG_DECODE_MODE_FRAME


/* SDRAM size */ /*CY 1023*/
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
#define SDRAM_SIZE          0x04000000  /* 64MB */
#else
#define SDRAM_SIZE          0x02000000  /* 32MB */
#endif

#define TICKS_PER_SECOND    20
/*== TV out resolution ==*/
#if ( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_D1) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
#define TVOUT_X            704
#else
#define TVOUT_X            640
#endif

#define TVOUT_Y_NTSC       480
#define TVOUT_Y_PAL        576

#define FULL_TV_OUT        1
#define HALF_TV_OUT        0

#if TVOUT_CRYSTAL_24MHZ
  #define TV_ACTSTAEND_PAL   0x05EF00ED
  #define TV_ACTSTAEND_NTSC  0x05E000D8
#else
    #if(TV_ENCODER == BIT1201G)         //Roy: 解決8600 RDI TV2右邊會出現1條白線 
        #define TV_ACTSTAEND_NTSC      0x06480148
        #define TV_ACTSTAEND_NTSC_D1   0x067e00fe
    #else        
        #define TV_ACTSTAEND_NTSC      0x064A0148
        #define TV_ACTSTAEND_NTSC_D1   0x067e00fe
    #endif
    #define TV_ACTSTAEND_PAL           0x06600160
    #define TV_ACTSTAEND_PAL_D1        0x069a011a
#endif

//== Pannel out resolution ==//
#if (LCM_OPTION == LCM_HX5073_RGB)

      #define PANNEL_X    320
      #define PANNEL_Y    240


#elif (LCM_OPTION == LCM_HX5073_YUV)

      #define PANNEL_X  640
      #define PANNEL_Y  240
      #define SCALER_X  40
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 8
      #define MIDDLE_LENGTH_X 12
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif (LCM_OPTION == LCM_TPG105)

      #define PANNEL_X  160
      #define PANNEL_Y  240
      #define SCALER_X  40
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 8
      #define MIDDLE_LENGTH_X 12
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif (LCM_OPTION == LCM_TD020THEG1)

      #define PANNEL_X  214
      #define PANNEL_Y  240
      #define SCALER_X  60
      #define SCALER_TV_X   92
      #define SCALER_TV_Y   90
      #define SCALER_Y  88
      #define MARGIN_LENGTH_X 8
      #define MARGIN_TV_LENGTH_X 12
      #define MARGIN_TV_LENGTH_Y 20
      #define MIDDLE_LENGTH_X 8
      #define MARGIN_LENGTH_Y 20
      #define MIDDLE_LENGTH_Y 20

#elif (LCM_OPTION == LCM_HX8312)

      #define PANNEL_X  320
      #define PANNEL_Y  240

#elif (LCM_OPTION == LCM_HX8224)

      #define PANNEL_X  160
      #define PANNEL_Y  240
      #define SCALER_X  40
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 8
      #define MIDDLE_LENGTH_X 12
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif ( (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) )

      #define PANNEL_X  640
      #define PANNEL_Y  240
      #define SCALER_X  40
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 8
      #define MIDDLE_LENGTH_X 12
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif (LCM_OPTION == LCM_CCIR601_640x480P)

      #define PANNEL_X  640
      #define PANNEL_Y  480
      #define SCALER_X  40
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 8
      #define MIDDLE_LENGTH_X 12
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif (LCM_OPTION == LCM_sRGB_HD15_HDMI)
				  
      #define PANNEL_X  1280
      #define PANNEL_Y  720
      #define SCALER_X  40
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 8
      #define MIDDLE_LENGTH_X 12
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif (LCM_OPTION == LCM_HX8817_RGB)
      #define PANNEL_X  480
      #define PANNEL_Y  240
      #define SCALER_X  40
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 8
      #define MIDDLE_LENGTH_X 12
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif (LCM_OPTION == LCM_A015AN04)

      #define PANNEL_X  94
      #define PANNEL_Y  220
      #define SCALER_X  40
      #define SCALER_Y  80
      #define MARGIN_LENGTH_X 4
      #define MIDDLE_LENGTH_X 4
      #define MARGIN_LENGTH_Y (40+4)
      #define MIDDLE_LENGTH_Y 8

#elif (LCM_OPTION == LCM_HX8224_SRGB)

      #define PANNEL_X  480
      #define PANNEL_Y  240
      #define SCALER_X  120
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 24
      #define MIDDLE_LENGTH_X 36
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif((LCM_OPTION == LCM_GPG48238QS4)||(LCM_OPTION == LCM_A024CN02)||(LCM_OPTION == LCM_TJ015NC02AA))

      #define PANNEL_X  160
      #define PANNEL_Y  240
      #define SCALER_X  40
      #define SCALER_Y  60
      #define SIZE_X    56
      #define SIZE_Y    84
      #define MARGIN_LENGTH_X 8
      #define MIDDLE_LENGTH_X 12
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif (LCM_OPTION == LCM_TG200Q04)

      #define PANNEL_X  220
      #define PANNEL_Y  176
      #define SCALER_X  40
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 8
      #define MIDDLE_LENGTH_X 12
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif ((LCM_OPTION == LCM_TMT035DNAFWU24_320x240)||(LCM_OPTION == LCM_LQ035NC111))

      #define PANNEL_X  320
      #define PANNEL_Y  240
      #define SCALER_X  80
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 8
      #define MIDDLE_LENGTH_X 12
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16
#elif (LCM_OPTION == LCM_TD024THEB2)
      #define PANNEL_X  160
      #define PANNEL_Y  240
      #define SCALER_X  120
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 24
      #define MIDDLE_LENGTH_X 36
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16
#elif (LCM_OPTION == LCM_TD024THEB2_SRGB)
      #define PANNEL_X  480
      #define PANNEL_Y  240
      #define SCALER_X  120
      #define SCALER_Y  60
      #define MARGIN_LENGTH_X 24
      #define MIDDLE_LENGTH_X 36
      #define MARGIN_LENGTH_Y (40+12)
      #define MIDDLE_LENGTH_Y 16

#elif ((LCM_OPTION == LCM_HX8257_RGB666_480x272)||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
  #define PANNEL_X  480
  #define PANNEL_Y  272
  #define SCALER_X  40
  #define SCALER_Y  60
  #define MARGIN_LENGTH_X 8
  #define MIDDLE_LENGTH_X 12
  #define MARGIN_LENGTH_Y (40+12)
  #define MIDDLE_LENGTH_Y 16

#elif (LCM_OPTION ==LCM_TD036THEA3_320x240)
  #define PANNEL_X  320
  #define PANNEL_Y  240
  #define SCALER_X  60
  #define SCALER_TV_X   92
  #define SCALER_TV_Y   90
  #define SCALER_Y  88
  #define MARGIN_LENGTH_X 8
  #define MARGIN_TV_LENGTH_X 12
  #define MARGIN_TV_LENGTH_Y 20
  #define MIDDLE_LENGTH_X 8
  #define MARGIN_LENGTH_Y 20
  #define MIDDLE_LENGTH_Y 20

#elif ((LCM_OPTION == LCM_P_RGB_888_HannStar)|| (LCM_OPTION == LCM_P_RGB_888_AT070TN90) || (LCM_OPTION == LCM_P_RGB_888_SY700BE104) || (LCM_OPTION == LCM_P_RGB_888_ZSX900B50BL)|| (LCM_OPTION == LCM_P_RGB_888_FC070227) || (LCM_OPTION == LCM_P_RGB_888_ILI6122))
  #define PANNEL_X  800
  #define PANNEL_Y  480
  #define SCALER_X  60
  #define SCALER_TV_X   92
  #define SCALER_TV_Y   90
  #define SCALER_Y  88
  #define MARGIN_LENGTH_X 8
  #define MARGIN_TV_LENGTH_X 12
  #define MARGIN_TV_LENGTH_Y 20
  #define MIDDLE_LENGTH_X 8
  #define MARGIN_LENGTH_Y 20
  #define MIDDLE_LENGTH_Y 20  

#elif( (LCM_OPTION == LCM_P_RGB_888_Innolux) )
  #define PANNEL_X  800
  #define PANNEL_Y  600
  #define SCALER_X  60
  #define SCALER_TV_X   92
  #define SCALER_TV_Y   90
  #define SCALER_Y  88
  #define MARGIN_LENGTH_X 8
  #define MARGIN_TV_LENGTH_X 12
  #define MARGIN_TV_LENGTH_Y 20
  #define MIDDLE_LENGTH_X 8
  #define MARGIN_LENGTH_Y 20
  #define MIDDLE_LENGTH_Y 20    

#elif (LCM_OPTION == VGA_640X480_60HZ)
  #define PANNEL_X  640
  #define PANNEL_Y  480
  #define SCALER_X  60
  #define SCALER_TV_X   92
  #define SCALER_TV_Y   90
  #define SCALER_Y  88
  #define MARGIN_LENGTH_X 8
  #define MARGIN_TV_LENGTH_X 12
  #define MARGIN_TV_LENGTH_Y 20
  #define MIDDLE_LENGTH_X 8
  #define MARGIN_LENGTH_Y 20
  #define MIDDLE_LENGTH_Y 20      

#elif (LCM_OPTION == VGA_800X600_60HZ)
  #define PANNEL_X  800
  #define PANNEL_Y  600
  #define SCALER_X  60
  #define SCALER_TV_X   92
  #define SCALER_TV_Y   90
  #define SCALER_Y  88
  #define MARGIN_LENGTH_X 8
  #define MARGIN_TV_LENGTH_X 12
  #define MARGIN_TV_LENGTH_Y 20
  #define MIDDLE_LENGTH_X 8
  #define MARGIN_LENGTH_Y 20
  #define MIDDLE_LENGTH_Y 20      

#elif (LCM_OPTION == VGA_1024X768_60HZ)
  #define PANNEL_X  1024
  #define PANNEL_Y  768
  #define SCALER_X  60
  #define SCALER_TV_X   92
  #define SCALER_TV_Y   90
  #define SCALER_Y  88
  #define MARGIN_LENGTH_X 8
  #define MARGIN_TV_LENGTH_X 12
  #define MARGIN_TV_LENGTH_Y 20
  #define MIDDLE_LENGTH_X 8
  #define MARGIN_LENGTH_Y 20
  #define MIDDLE_LENGTH_Y 20      

#elif (LCM_OPTION == VGA_1280X800_60HZ)
  #define PANNEL_X  1280
  #define PANNEL_Y  800
  #define SCALER_X  60
  #define SCALER_TV_X   92
  #define SCALER_TV_Y   90
  #define SCALER_Y  88
  #define MARGIN_LENGTH_X 8
  #define MARGIN_TV_LENGTH_X 12
  #define MARGIN_TV_LENGTH_Y 20
  #define MIDDLE_LENGTH_X 8
  #define MARGIN_LENGTH_Y 20
  #define MIDDLE_LENGTH_Y 20        

#endif

#endif
