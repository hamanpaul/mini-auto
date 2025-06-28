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

#define HW_BOARD_OPTION   MR9200_RX_MAYON_MWM903

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
#if(HW_BOARD_OPTION == A1018_FPGA_BOARD)
    #define CHIP_OPTION                   CHIP_A1018A
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0
    
#elif(HW_BOARD_OPTION == A1018B_FPGA_BOARD)
    #define CHIP_OPTION                   CHIP_A1018B
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0
    
#elif(HW_BOARD_OPTION == A1021A_FPGA_BOARD)
    #define CHIP_OPTION                   CHIP_A1021A
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0
    
#elif(HW_BOARD_OPTION == A1022A_FPGA_BOARD)
    #define CHIP_OPTION                   CHIP_A1022A
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0
    
#elif(HW_BOARD_OPTION == A1025A_FPGA_BOARD)
    #define CHIP_OPTION                   CHIP_A1025A
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0


#elif( (HW_BOARD_OPTION == MR9160_TX_DB_BATCAM) || (HW_BOARD_OPTION == MR9160_TX_OPCOM_BATCAM) || (HW_BOARD_OPTION == MR9160_TX_MAYON_MWL613) || (HW_BOARD_OPTION == MR9160_TX_ROULE_BATCAM) )
    #define CHIP_OPTION                   CHIP_A1019A
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           1
    
#elif( (HW_BOARD_OPTION == A1019A_FPGA_BOARD) || (HW_BOARD_OPTION == A1019A_SKB_128M_TX) || (HW_BOARD_OPTION == A1019A_EVB_128M_TX) ||\
       (HW_BOARD_OPTION == A1019A_SKB_128M_RX) || (HW_BOARD_OPTION == A1019A_TX_MA8806) || (HW_BOARD_OPTION == A1019A_TX_RDI_CA831)||\
       (HW_BOARD_OPTION == MR9100_TX_RDI_CA811) || (HW_BOARD_OPTION == MR9100_TX_SKY_AHD) || (HW_BOARD_OPTION == MR9100_TX_SKY_W_AHD) ||\
       (HW_BOARD_OPTION == A1019A_EVB) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_HD_USB) || \
       (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI_SK) || (HW_BOARD_OPTION == MR9100_TX_RDI_USB) || (HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T)||\
       (HW_BOARD_OPTION == MR9100_AHDINREC_MUXCOM) || (HW_BOARD_OPTION == MR9100_TX_MAYON_MWL612) || (HW_BOARD_OPTION == MR9211_TX_MA8806) )
    #define CHIP_OPTION                   CHIP_A1019A
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0
    
#elif(HW_BOARD_OPTION == A1016B_FPGA_BOARD)
    #define CHIP_OPTION  			      CHIP_A1016B
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0
    
#elif((HW_BOARD_OPTION == A1018_EVB_128M) || (HW_BOARD_OPTION == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) \
    || (HW_BOARD_OPTION == MR6790_DB) || (HW_BOARD_OPTION == MR9211_DB) || (HW_BOARD_OPTION == MR9120_TX_DB) ||\
    (HW_BOARD_OPTION == MR9600_RX_DB) ||  (HW_BOARD_OPTION == MR9300_RX_DB) || \
    (HW_BOARD_OPTION == MR9200_RX_RDI) || (HW_BOARD_OPTION == MR9200_RX_TRANWO) ||\
    (HW_BOARD_OPTION == MR9120_TX_TRANWO))
    #define CHIP_OPTION                   CHIP_A1018A
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0
    
#elif(HW_BOARD_OPTION  == MR9120_TX_OPCOM)
    #define CHIP_OPTION                   CHIP_A1018A
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0
    
#elif( (HW_BOARD_OPTION == A1018B_SKB_128M_TX) || (HW_BOARD_OPTION == MR9120_TX_RDI_USB) || (HW_BOARD_OPTION == MR9120_TX_SKY_USB) || (HW_BOARD_OPTION == MR9120_TX_BT_USB))
    #define CHIP_OPTION                   CHIP_A1018B
    #define REDUCE_HCLK_TO_128M           1
    #define REDUCE_HCLK_TO_144M           0
#elif ((HW_BOARD_OPTION == A1025A_EVB)|| (A1025_GATE_WAY_SERIES))
	#define CHIP_OPTION                   CHIP_A1025A
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0
#else
    #define CHIP_OPTION                   CHIP_A1018B
    #define REDUCE_HCLK_TO_128M           0
    #define REDUCE_HCLK_TO_144M           0
    
#endif

/*
Step 3: 設定產品應用 
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
#define SW_APPLICATION_OPTION           MR9200_MIXCAM_RX1RX2
/*
Step 4: 設定 IC 操作頻率 
========================== IC operation frequency===================           
********************************************************************
*/
  #if(FPGA_BOARD_A1018_SERIES)
      #define SYS_CPU_CLK_FREQ      32000000
      #define ADC_IIS_CLK_FREQ      32000000
	  #define IDU_CLK_FREQ          32000000
	  #define RFI_CLK_FREQ          32000000

  #elif( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) )
      #define SYS_CPU_CLK_FREQ      162000000
      #define ADC_IIS_CLK_FREQ      54000000
	  #define IDU_CLK_FREQ          54000000
	  #define RFI_CLK_FREQ          54000000
  #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
      #define SYS_CPU_CLK_FREQ      162000000
      #define ADC_IIS_CLK_FREQ      54000000
	  #define IDU_CLK_FREQ          54000000
	  #define RFI_CLK_FREQ          54000000
  #else
    #if REDUCE_HCLK_TO_128M  //Lucian: sysclk=128,ddrclk=192
      #define SYS_CPU_CLK_FREQ      160000000
      #define ADC_IIS_CLK_FREQ      32000000
	  #define IDU_CLK_FREQ          32000000
	  #define RFI_CLK_FREQ          48000000
    #elif REDUCE_HCLK_TO_144M  //Lucian: sysclk=144,ddrclk=216
      #define SYS_CPU_CLK_FREQ      160000000
      #define ADC_IIS_CLK_FREQ      48000000
	  #define IDU_CLK_FREQ          48000000
	  #define RFI_CLK_FREQ          48000000
    #else  //Lucian: sysclk=160,ddrclk=240
      #define SYS_CPU_CLK_FREQ      160000000
      #define ADC_IIS_CLK_FREQ      48000000
	  #define IDU_CLK_FREQ          48000000
	  #define RFI_CLK_FREQ          48000000
    #endif
  #endif

//==============Performace Option=================//
/*CY 0907*/
#define DINAMICALLY_POWER_MANAGEMENT    1    // 1: dinamically tune clock module by module, 0: otherwise /* Peter */
#define DCF_WRITE_STATISTIC             1    // Lucian:統計DCF write 流量,optimize rate control.


//==============Test Option=================//
//--元件測試: 請放置於mainTask(), while(1)之前--//
#define IIS_TEST                        0
#define IIS_4CH_REC_TEST                0
#define IIC_TEST                        0
#define SDRAM_TEST                      0    //SDRAM r/w test when power-on
#define EMBEDED_SRAM_TEST               0
#define SPI_ENDIAN_TEST					0
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
#define SIMU1080I_VIA720P               0  //此功能在數位輸出(BT1120)無法使用.
#define IDU_OSD_TEST                    0  // OSD BRI TEST Amon (140721)
#define AESDES_TEST                     0
#define FTMAC110_NET_TEST				0
#define NET_LOOPBACK_TEST               0

//--整合測試--//
#define Auto_Video_Test                 0    // 錄影到卡滿 --> 檔案逐一播放 -->Delete all file.
#define SDC_WRITE_READ_TEST             0    // SD card r/w test, SDRAM r/w test.
#define CDVR_TEST                       0    // Test CDVR file System
#define ASF_SPLIT_FILE                  0
#define RF_RX_AUDIO_SELFTEST            0
#define iHome_LOG_TEST                  0
#define PLAYBEEP_TEST                   0   //beep play

#define PIR_FALSETRIG_TEST              0   //RX端錄下 faulse trigger影片.
#define PIR_PYD1588_TEST		      0  //for PIR PYD1588 test, 4 new cmd for settings PIR parameter, please use DC mode for testing

//==============Special Purpose =================//
#define MAKE_SPI_BIN	                0	    /* index of Making bin from spi, 1-> Enable, 0-> Disable */
#define ERASE_SPI	                    0		/* Sepcial purpose, 1-> Enable erase whole chip, 0-> Disable */
#define RW_PROTECT_SPI                  1       /* Read and Write protect function , 1 -> Enable, 0 -> Disable */
#define SDC_SEM_REPAIR					1
#define FS_SD_REMOUNT					1

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
#define RF_TX_AUDIORET_DEBUG_ENA        0
#define CIU_PATTERN_TEST                0
#define USB_DONGLE_RX_YOUT_DEBUG_ENA    0
#define PK3100_FORCE2DAYMODE            0    // always Day
#define HDMI_DEBUG_ENA                  0
#define MESUARE_BTCWAKEUP_TIME          1
#define PWIFI_PROTOCOL_TIMING_DEBUG_ENA 1    //Pwifi protocol timing debug  

//Peter use
#define FIQ_DEBUG_ENA                   0
#define IRQ_DEBUG_ENA                   0
#define MPEG_DEBUG_ENA                  0
#define OS_DEBUG_ENA                    0
#define FS_DEBUG_ENA                    0
#define ASF_DEBUG_ENA                   0
#define EBELL_OS_DEBUG_ENA              0
#define EBELL_IRQ_DEBUG_ENA             0

//Allen use
#define DEBUG_PROGRAM_MSG					/* Show Program massage for UI Lib, Code, Wave Files update */
#define MEASURE_BOOT_TIME               0
//#undef	DEBUG_PROGRAM_MSG					/* */

//Toby use
#define H264_DEBUG_ENA                  0   //GPIO1-5: 量測 H264  operation time
#define VIDEO_STARTCODE_DEBUG_ENA       0

//Aher use
#define P2P_DEBUG_ENA                   0

//Lsk use
#define JPEG_DEBUG_ENA_9300		0
#define JPEG_DEBUG_ENA_9200		0

#define H1_264TEST		0
#define H1_264TEST_ENC  0

#define H1_264_ENC      0
//----DEBUG LED---//
#define  DEBUG_SIU_FRAM_STR_INTR_USE_LED6        0
#define  DEBUG_SIU_BOTFD_END_INTR_USE_LED7       0
#define  DEBUG_ISU_INTR_USE_LED6                 0
#define  DEBUG_IDU_INTR_USE_LED6                 0
#define  DEBIG_CIU_FRAM_END_INTR_USE_LED6        0
#define  DEBUG_MPEG_OPERRATION_USE_LED6          0


#include "application.h"
#include "project.h"

//============================Common Option======================//
    #define NEW_IDU_BRI                     1
    #define NEW_IDU_BRI_PANEL_INTLX         0        


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



#if MEMCPY_DMA
    #define CopyMemory memcpy_hw
#else
    #define CopyMemory memcpy
#endif

#define AVSYNC							AUDIO_FOLLOW_VIDEO
#define PLAYBACK_METHOD					PLAYBACK_IN_IIS_ISR

/* MPEG4 file container option
 *  MPEG4_CONTAINER_MP4 - MP4 file.
 *  MPEG4_CONTAINER_ASF - ASF file.
 *  MPEG4_CONTAINER_AVI - AVI file.
 *  MPEG4_CONTAINER_MOV - MOV file.
 */
#define MPEG4_CONTAINER_OPTION      MPEG4_CONTAINER_ASF


/* Video Codec Option */
#if (HW_BOARD_OPTION == MR8202A_RX_TARNWO_D8530)
#define VIDEO_CODEC_OPTION          H264_CODEC   //MPEG4_CODEC
#else
#define VIDEO_CODEC_OPTION          H264_CODEC
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


#define TICKS_PER_SECOND    20
/*== TV out resolution ==*/
#if ( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_D1) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_CIF) )
#define TVOUT_X            704
#define TVOUT_Y            480
#elif ( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P60) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P30) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P25) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_HD720P60_37M))
#define TVOUT_X            1280
#define TVOUT_Y            720
#elif ( (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080I60) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE == TV_FHD1080P30) )
#define TVOUT_X            1920
#define TVOUT_Y            1080
#else
#define TVOUT_X            640
#define TVOUT_Y            480
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
    #define TV_ACTSTAEND_PAL           0x06620160
    #define TV_ACTSTAEND_PAL_D1        0x069a011a
#endif

typedef enum {
	EP952_SF_None = 0,
	EP952_SF_32000Hz = 1,
	EP952_SF_44100Hz,
	EP952_SF_48000Hz,	
}EP952_AudFreq;

/*
 0: No video input
 1: CEA-861D 640 x 480 			(60Hz or 59.94Hz)	Progressive 4:3
 2: CEA-861D 720 x 480 			(60Hz or 59.94Hz)	Progressive 4:3		<-- 480P  60Hz (4:3)
 3: CEA-861D 720 x 480 			(60Hz or 59.94Hz)	Progressive 16:9	<-- 480P  60Hz (16:9)
 4: CEA-861D 1280 x 720			(60Hz or 59.94Hz)	Progressive 16:9 	<-- 720P  60Hz
 5: CEA-861D 1920 x 1080		(60Hz or 59.94Hz)	Interlaced 16:9		<-- 1080i 60Hz
*/
typedef enum {
	EP952_VDO_None = 0,
	EP952_VDO_640x480,
	EP952_VDO_720x480,
	EP952_VDO_720x480W,
	EP952_VDO_1280x720P,
	EP952_VDO_1920x1080I,
}EP952_VideoSize;

#define EP952_VDO_1920x1080P 34
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

#elif (LCM_OPTION == LCM_CCIR601_1280x720p30)
      
      #define PANNEL_X  1280
      #define PANNEL_Y  720
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

#elif( (LCM_OPTION == LCM_P_RGB_888_ZSX70DT5011) )
  #define PANNEL_X  1024
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
  
#elif( (LCM_OPTION == LCM_P_RGB_PZX090IV042002) )
  #define PANNEL_X  1024
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
  #define PANNEL_Y  720
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
  #define PANNEL_Y  720
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

#if(Sensor_OPTION == Sensor_NONE_HD)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_HD_IN_HD_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit
#elif(Sensor_OPTION == Sensor_NONE_FHD)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_FULL_HD
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit
#elif(Sensor_OPTION == Sensor_CCIR601)
  #define USE_PROGRESSIVE_SENSOR           0
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.
  #define SDV_SELECT                       SSDV_1

  #define VIDEO_RESOLUTION_SEL             VIDEO_VGA_IN_VGA_OUT
  #if(TV_DECODER == MI9V136)
    #define SENSOR_DATA_SHIFT_OPTION       SENSOR_DATA_NO_SHIFT  // bit[7:0]
  #else
    #define SENSOR_DATA_SHIFT_OPTION       SENSOR_DATA_SHIFT_2bit  // bit[7:0]
  #endif

#elif(Sensor_OPTION == Sensor_CCIR656)
  #define USE_PROGRESSIVE_SENSOR           0
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   1   //@9003, 外部FID intr,  由siu intr (Frame start)取代.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_VGA_IN_VGA_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit  // bit[7:0]

#elif(Sensor_OPTION == Sensor_MI_5M)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_2
 #if FPGA_BOARD_A1018_SERIES
  #define VIDEO_RESOLUTION_SEL             VIDEO_FULLHD_IN_VGA_OUT
 #else
  #define VIDEO_RESOLUTION_SEL             VIDEO_FULL_HD
 #endif
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_NO_SHIFT

#elif(Sensor_OPTION == Sensor_MI1320_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_HD_IN_HD_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#elif(Sensor_OPTION == Sensor_OV2643_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_HD_IN_HD_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#elif(Sensor_OPTION == Sensor_MT9M131_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_HD_IN_HD_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#elif(Sensor_OPTION == Sensor_HM1375_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_HD_IN_HD_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#elif(Sensor_OPTION == Sensor_NT99141_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_HD_IN_HD_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#elif(Sensor_OPTION == Sensor_NT99340_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_FULL_HD
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#elif(Sensor_OPTION == Sensor_NT99230_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_FULL_HD
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#elif(Sensor_OPTION == Sensor_HM5065_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.
  
  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_FULL_HD
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#elif(Sensor_OPTION == Sensor_PO3100K_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_HD_IN_HD_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#elif(Sensor_OPTION == Sensor_PO2210K_YUV601 || Sensor_OPTION == Sensor_ZN220_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_FULL_HD
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#elif(Sensor_OPTION == Sensor_OV7740_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_VGA_IN_VGA_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit
  
#elif(Sensor_OPTION == Sensor_OV7725_YUV601)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_VGA_IN_VGA_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit
  
#elif(Sensor_OPTION == Sensor_XC7021_SC2133 || Sensor_OPTION == Sensor_XC7021_GC2023)
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_FULL_HD
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

#else
  #define USE_PROGRESSIVE_SENSOR           1
  #define ISU_OUT_BY_VSYNC                 0
  #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

  #define SDV_SELECT                       SSDV_1
  #define VIDEO_RESOLUTION_SEL             VIDEO_VGA_IN_VGA_OUT
  #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_NO_SHIFT

#endif

#if (SW_APPLICATION_OPTION  == DVP_RF_SELFTEST) //for RF interal test
  #define MPEG4_MAX_WIDTH                1280
  #define MPEG4_MAX_HEIGHT               720

#elif(VIDEO_RESOLUTION_SEL== VIDEO_VGA_IN_VGA_OUT)
  #define MPEG4_MAX_WIDTH                720
  #define MPEG4_MAX_HEIGHT               576
#elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_VGA_OUT)
  #define MPEG4_MAX_WIDTH                720
  #define MPEG4_MAX_HEIGHT               576
#elif(VIDEO_RESOLUTION_SEL== VIDEO_FULLHD_IN_VGA_OUT)
  #define MPEG4_MAX_WIDTH                720
  #define MPEG4_MAX_HEIGHT               576    
#elif(VIDEO_RESOLUTION_SEL== VIDEO_1920x1440DECI3x3TOVGA)
  #define MPEG4_MAX_WIDTH                720
  #define MPEG4_MAX_HEIGHT               576
#elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_HD_OUT)
  #define MPEG4_MAX_WIDTH                1280
  #define MPEG4_MAX_HEIGHT               720
#elif(VIDEO_RESOLUTION_SEL== VIDEO_FULLHD_IN_HD_OUT)
  #define MPEG4_MAX_WIDTH                1280
  #define MPEG4_MAX_HEIGHT               720    
#elif(VIDEO_RESOLUTION_SEL== VIDEO_FULL_HD)
  #define MPEG4_MAX_WIDTH                1920
  #define MPEG4_MAX_HEIGHT               1088
#endif

#if MULTI_STREAM_SUPPORT
  #if SWAP_MULTI_STREAM_SUPPORT
    #define SP_MAX_WIDTH                  640
    #define SP_MAX_HEIGHT                 360
  #else 
    #if (CIU_SP_RESO == SP_1x1)
      #define SP_MAX_WIDTH                  MPEG4_MAX_WIDTH
      #define SP_MAX_HEIGHT                 MPEG4_MAX_HEIGHT
    #elif (CIU_SP_RESO == SP_2x1)
      #define SP_MAX_WIDTH                  (MPEG4_MAX_WIDTH/2)
      #define SP_MAX_HEIGHT                 MPEG4_MAX_HEIGHT
    #elif (CIU_SP_RESO == SP_2x2)
      #define SP_MAX_WIDTH                  (MPEG4_MAX_WIDTH/2)
      #define SP_MAX_HEIGHT                 (MPEG4_MAX_HEIGHT/2)
    #elif (CIU_SP_RESO == SP_4x2)
      #define SP_MAX_WIDTH                  (MPEG4_MAX_WIDTH/4)
      #define SP_MAX_HEIGHT                 (MPEG4_MAX_HEIGHT/2)
    #elif (CIU_SP_RESO == SP_4x4)
      #define SP_MAX_WIDTH                  (MPEG4_MAX_WIDTH/4)
      #define SP_MAX_HEIGHT                 (MPEG4_MAX_HEIGHT/4)
    #endif
    
  #endif // #if SWAP_MULTI_STREAM_SUPPORT
  
#endif // #if MULTI_STREAM_SUPPORT

#endif
