#ifndef _APPLICATION_H
#define _APPLICATION_H

#include "project.h"





#if (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1)
  #if(CHIP_OPTION == CHIP_A1020A)
    #define CHIP1016_ID_CODE                0x01
  #else
    #define CHIP1016_ID_CODE                0x08
  #endif   
    #define CIU_SUPPORT 					1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0
    
    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #if(CIU1_SCUP_EN)
      #define CIU1_BOB_REPLACE_MPEG_DF      1
    #else
      #define CIU1_BOB_REPLACE_MPEG_DF      0
    #endif
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise

#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD) || \
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))

	#define Audio_mode                      AUDIO_MANUAL
#else
	#define Audio_mode                      AUDIO_AUTO
#endif
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_DEMO_RESOLUTION              0
    #define RF_TX_WIDTH                     640
    #define RF_TX_HEIGHT                    480

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                        RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) ||(HW_BOARD_OPTION  == A1020A_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0

    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        1
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0


    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_TX_WAKEUP_SCHEME          1

    #define RFIU_RX_SHOW_ONLY              0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //%%%%%%%%%%%%%%%%%%Software Motion Option%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define VMDSW                          1
    
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M)
    #define CHIP1016_ID_CODE                0x08

    #define CIU_SUPPORT 					1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #if(CIU1_SCUP_EN)
      #define CIU1_BOB_REPLACE_MPEG_DF      1
    #else
      #define CIU1_BOB_REPLACE_MPEG_DF      0
    #endif
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise

	#define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_DEMO_RESOLUTION              0
    #define RF_TX_WIDTH                     640
    #define RF_TX_HEIGHT                    480

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                        RFIC_A7196_6M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) ||(HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        1
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0


    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_TX_WAKEUP_SCHEME          1

    #define RFIU_RX_SHOW_ONLY              0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //%%%%%%%%%%%%%%%%%%Software Motion Option%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define VMDSW                          1

    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2)  //For Audio Line-in
    #define CHIP1016_ID_CODE               0x08

    #define CIU_SUPPORT 					1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #if(CIU1_SCUP_EN)
      #define CIU1_BOB_REPLACE_MPEG_DF      1
    #else
      #define CIU1_BOB_REPLACE_MPEG_DF      0
    #endif
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
    #define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_DEMO_RESOLUTION              0
    #define RF_TX_WIDTH                     640
    #define RF_TX_HEIGHT                    480

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                        RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        1
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0


    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_TX_WAKEUP_SCHEME          0

    #define RFIU_RX_SHOW_ONLY              0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    #define CDVR_REC_WITH_PLAY_SUPPORT     1

    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1)
   #if(CHIP_OPTION == CHIP_A1020A)
    #define CHIP1016_ID_CODE                0x04
   #else
    #define CHIP1016_ID_CODE                0x02
   #endif
	#define CIU_SUPPORT 					1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  1

    #if(CIU1_SCUP_EN)
      #define CIU1_BOB_REPLACE_MPEG_DF      1
    #else
      #define CIU1_BOB_REPLACE_MPEG_DF      0
    #endif
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise

	#define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_DEMO_RESOLUTION              0
    #define RF_TX_WIDTH                     640
    #define RF_TX_HEIGHT                    480

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                        RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0

    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        1
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0


    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_TX_WAKEUP_SCHEME          1

    #define USE_704x480_RESO               1

    #define RFIU_RX_SHOW_ONLY              0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //%%%%%%%%%%%%%%%%%%Software Motion Option%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define VMDSW                          1

    //---------------//
    #define TX_SNAPSHOT_SUPPORT            1

    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
    #if(CHIP_OPTION == CHIP_A1020A)
        #if (USB2WIFI_SUPPORT)
            #define CHIP1016_ID_CODE                0x0c    /*MR8211B: A1020 with USB WIFI*/
        #else
            #define CHIP1016_ID_CODE                0x0a    /*MR8212B: A1020 without USB WIFI*/
        #endif
    #else //A1016 series
        #if (USB2WIFI_SUPPORT)
            #define CHIP1016_ID_CODE                0x0a    /*MR8211A: A1016 with USB WIFI*/
        #else
            #define CHIP1016_ID_CODE                0x05    /*MR8212A: A1016 without USB WIFI*/
        #endif
    #endif
    
    #define CIU_SUPPORT 				    1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

	#define ICOMMWIFI_SUPPORT				1
    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  1
    
    #if(CIU1_SCUP_EN)
      #define CIU1_BOB_REPLACE_MPEG_DF      1
    #else
      #define CIU1_BOB_REPLACE_MPEG_DF      0
    #endif
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise

	#define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                      1
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_DEMO_RESOLUTION              0
    #define RF_TX_WIDTH                     640
    #define RF_TX_HEIGHT                    480

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite           800
    #define RF_RX_DEC_HEIGHT_spite          480

    #define RF_TX_AUTOPOWER                 0

    //--config RF PHY--//
	#define RFIC_SEL                        RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)||\
       (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD) ||\
       (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
       (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0

    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        1
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0


    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_TX_WAKEUP_SCHEME          1

    #define USE_704x480_RESO               1

    #define RFIU_RX_SHOW_ONLY              0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //%%%%%%%%%%%%%%%%%%Software Motion Option%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define VMDSW                          1

    //---------------//
    #define TX_SNAPSHOT_SUPPORT            1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   1
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif(SW_APPLICATION_OPTION == MR8200_RFCAM_RX1)
    #define CHIP1016_ID_CODE               0x0f

    #define CIU_SUPPORT 					1
    #define CIU_SCUP_EN                     0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0


    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
    #define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #if(HW_BOARD_OPTION == MR8100_GCT_LCD)
    #define NIC_SUPPORT                   0
    #else
    #define NIC_SUPPORT                   1
    #endif
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #if(HW_BOARD_OPTION == MR8100_GCT_LCD)
    #define TUTK_SUPPORT                  0
    #else
    #define TUTK_SUPPORT                  1
    #endif


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480
    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA            0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     1

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0

    #define RFIU_RX_SHOW_ONLY              0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    #if(HW_BOARD_OPTION == MR8100_GCT_LCD)
    #define CDVR_REC_WITH_PLAY_SUPPORT     0
    #else
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    #endif
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif(SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2)
    #define CHIP1016_ID_CODE               0x0f
 
    #define CIU_SUPPORT                     0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0


    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #if((HW_BOARD_OPTION == MR8200_RX_TRANWO_BOX) || (HW_BOARD_OPTION == MR8200_RX_MAYON_MWM014))
    #define TV_D1_OUT_FULL_HALF             1 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    #else
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    #endif
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
    #define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     1
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    1


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA            0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      1

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          1

    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1)
    #define CHIP1016_ID_CODE               0x08

    #define CIU_SUPPORT 					1
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1       //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4       // max video record channel support, include both local sensor and RF channel
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        1
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
	#define MR8600_DEMO_USE_PKEY           0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2)
    #define CHIP1016_ID_CODE               0x0d

    #define CIU_SUPPORT 					1
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0
 #if(HW_BOARD_OPTION ==  MR8600_RX_GCT)
    #define TV_D1_OUT_FULL                  0 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
 #else
    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
 #endif
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1       //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               2       // max video record channel support, include both local sensor and RF channel
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

 #if TV_D1_OUT_FULL
    #define RF_RX_2DISP_WIDTH              704
    #define RF_RX_2DISP_HEIGHT             480
 #else
    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480 
 #endif

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA            0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    1
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
	#define MR8600_DEMO_USE_PKEY           0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1)
    #define CHIP1016_ID_CODE               0x0d

    #define CIU_SUPPORT 					1
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1       //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               2       // max video record channel support, include both local sensor and RF channel
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL

    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

 #if TV_D1_OUT_FULL
    #define RF_RX_2DISP_WIDTH              704
    #define RF_RX_2DISP_HEIGHT             480
 #else
    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480 
 #endif

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     1

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
	#define MR8600_DEMO_USE_PKEY           0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif(SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1_6M)
    #define CHIP1016_ID_CODE               0x0d

    #define CIU_SUPPORT 					1
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1       //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               2       // max video record channel support, include both local sensor and RF channel
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

 #if TV_D1_OUT_FULL
    #define RF_RX_2DISP_WIDTH              704
    #define RF_RX_2DISP_HEIGHT             480
 #else
    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480 
 #endif

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7196_6M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     1

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
	#define MR8600_DEMO_USE_PKEY           0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) //MR8120-216M
	#define CHIP1016_ID_CODE               0x01

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1       //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4       // max video record channel support, include both local sensor and RF channel
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_DEMO_RESOLUTION              0
    #define RF_TX_WIDTH                     640
    #define RF_TX_HEIGHT                    480

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

 #if (HW_BOARD_OPTION == MR8120_RX_HECHI)
    #define RF_RX_DEC_WIDTH_spite           640
 #else
    #define RF_RX_DEC_WIDTH_spite           800
 #endif
    #define RF_RX_DEC_HEIGHT_spite          480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     1

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          1
    #define RFIU_RX_SHOW_ONLY              0
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    
    #define RX_SNAPSHOT_SUPPORT            0
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif(SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) //MR8600-216M
    #define CHIP1016_ID_CODE               0x01

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1       //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4       // max video record channel support, include both local sensor and RF channel
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      1

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == MR8100_BABYMONITOR)
    #define CHIP1016_ID_CODE               0x07

    #define CIU_SUPPORT 					1
	#define CIU_SCUP_EN                     0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0


    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
    #define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||\
        (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    #define NIC_SUPPORT                   0
    #else
    #define NIC_SUPPORT                   1
    #endif
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||\
        (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    #define TUTK_SUPPORT                  0
    #else
    #define TUTK_SUPPORT                  1
    #endif


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0


    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

 #if USE_704x480_RESO
    #define RF_RX_2DISP_WIDTH              704
 #else
    #define RF_RX_2DISP_WIDTH              640
 #endif
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480
    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA            0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     1

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0

    #define RFIU_RX_SHOW_ONLY              1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    #if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||\
        (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    #else
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    #endif

    #define RX_SNAPSHOT_SUPPORT            0
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
    #define CHIP1016_ID_CODE               0x07

    #define CIU_SUPPORT 					1
	#define CIU_SCUP_EN                     0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0


    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
 #if (HW_BOARD_OPTION == MR8100_RX_RDI_M512)
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
 #else 
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
 #endif
    #define IDU_TV_DISABLE                  1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
    #define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||\
        (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    #define NIC_SUPPORT                   0
    #else
    #define NIC_SUPPORT                   1
    #endif
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||\
        (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    #define TUTK_SUPPORT                  0
    #else
    #define TUTK_SUPPORT                  1
    #endif


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0


    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

 #if USE_704x480_RESO
    #define RF_RX_2DISP_WIDTH              704
 #else
    #define RF_RX_2DISP_WIDTH              640
 #endif
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480
    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA            0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     1

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0

    #define RFIU_RX_SHOW_ONLY              1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    #if((HW_BOARD_OPTION == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||\
        (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    #else
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    #endif

 #if (HW_BOARD_OPTION == MR8100_RX_RDI_M512)
    #define RX_SNAPSHOT_SUPPORT            0
 #else
    #define RX_SNAPSHOT_SUPPORT            1
 #endif
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

    
#elif(SW_APPLICATION_OPTION == MR6730_CARDVR_2CH)  // 216M
    #define CHIP1016_ID_CODE                0x0c

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

	#if (HW_BOARD_OPTION != MR6730_AFN)
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0
	
	#else
	
		#if (CIU_BOB_MODE)
		#define CIU1_BOB_REPLACE_MPEG_DF		1
		#define CIU1_BOB_AUTO_MD				0// 1
		
		#else
		#define CIU1_BOB_REPLACE_MPEG_DF		0
		#define CIU1_BOB_AUTO_MD                0
		
		#endif
		
	#endif//#if (HW_BOARD_OPTION == MR6730_AFN)

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #if ((HW_DERIV_MODEL == HW_DEVTYPE_CDVR_AFN720PSEN))
    #define MULTI_CHANNEL_SEL               0x02
    #else
    #define MULTI_CHANNEL_SEL               0x06
    #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #if ((HW_DERIV_MODEL == HW_DEVTYPE_CDVR_AFN720PSEN))
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode
    #else
    #define DUAL_MODE_DISP_SUPPORT          1   // wired dule desplay mode
    #endif

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
   #if(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_PUSH || HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN) 
	//one channel project doesn't need audio auto mode
	#define Audio_mode                      AUDIO_AUTO	//AUDIO_MANUAL	
   #else	
	#define Audio_mode                      AUDIO_AUTO
   #endif

    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   0
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     0
    //%%%%%%%%%%%%%%%%%%Software Motion Option%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define VMDSW                          1

    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif(SW_APPLICATION_OPTION == MR6730_CARDVR_1CH)  // 128M
    #define CHIP1016_ID_CODE               0x02

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   0
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     0
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == MR8211_IPCAM)
    #define CHIP1016_ID_CODE               0x00

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x06
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_AUTO
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   1
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  1


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   0
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == MR9670_DOORPHONE)
    #define CHIP1016_ID_CODE               0x03

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #if (HW_BOARD_OPTION == MR9670_COMMAX)  // for PIP
        #define CIU2_SCUP_EN                    1
    #else
        #define CIU2_SCUP_EN                    0
    #endif
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        1
    #define CIU1_BOB_AUTO_MD                1

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x06
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #if (HW_BOARD_OPTION==MR9670_WOAN)
    #define DUAL_MODE_DISP_SUPPORT          1   // wired dule desplay mode
    #else
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode
    #endif
    #define CIU_SPLITER                     0   // 4 CCIR MUX IN Enable  0:disable 1: enable

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
		//%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0


		//%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                  0
    #define RFIU_TEST                     0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

#if IS_COMMAX_DOORPHONE
    #define RF_RX_DEC_WIDTH_spite            800
#else
    #define RF_RX_DEC_WIDTH_spite            640
#endif
    #define RF_RX_DEC_HEIGHT_spite           480
		//--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
		//=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

		//==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

		//==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

		//==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

		//==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

		//==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #if (HW_BOARD_OPTION==MR9670_WOAN)
    #define UI_SUPPORT_TREE                1
    #else
    #define UI_SUPPORT_TREE                0
    #endif
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
    #define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0



    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      1

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                  0
  #else  
    #define AMIC7130_PAON                  1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA            0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    #define CDVR_REC_WITH_PLAY_SUPPORT     1

    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif(SW_APPLICATION_OPTION == Standalone_Test)
    #define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                  0
    #define RFIU_TEST                     0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_CIU)
    #define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         4       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               5
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   0
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //%%%%%%%%%%%%%%%%%%Software Motion Option%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define VMDSW                          1

    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_DIU)
    #define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         4       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               5
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   0
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_IDU)
    #define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   0
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_SIU)
    #define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     1

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x01
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    0
    #define RFIU_TEST                       0

    #define RF_DEMO_RESOLUTION              0
    #define RF_TX_WIDTH                     640
    #define RF_TX_HEIGHT                    480

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite           800
    #define RF_RX_DEC_HEIGHT_spite          480

	//--config RF PHY--//
	#define RFIC_SEL                        RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1         0
    #define RFI_TEST_TX_PROTOCOL_B2         0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1         0
    #define RFI_TEST_RX_PROTOCOL_B2         0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2     0
    #define RFI_TEST_4TX_2RX_PROTOCOL       0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1      0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                  1
    #define RFIU_RX_AUDIO_ON                1
    #define RFIU_RX_TIME_SEL                RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN            0
    #define RFIU_TX_PIR_TRIG                0

    #define RFIU_TX_WAKEUP_SCHEME           0
    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                 0
    #define UI_PREVIEW_OSD                  1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFU)
    #define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #if(CIU1_SCUP_EN)
      #define CIU1_BOB_REPLACE_MPEG_DF      1
    #else
      #define CIU1_BOB_REPLACE_MPEG_DF      0
    #endif
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      1      //For Self-test,check rfiuapi.h

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480


#elif (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFTX1)
    #define CHIP1016_ID_CODE                0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #if(CIU1_SCUP_EN)
      #define CIU1_BOB_REPLACE_MPEG_DF      1
    #else
      #define CIU1_BOB_REPLACE_MPEG_DF      0
    #endif
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  0 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0   //disable TV and IDU for power saving.

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_DEMO_RESOLUTION              0
    #define RF_TX_WIDTH                     640
    #define RF_TX_HEIGHT                    480

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                        RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        1
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0


    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    #define CDVR_REC_WITH_PLAY_SUPPORT     1

    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFRX1)
    #define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1       //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4       // max video record channel support, include both local sensor and RF channel
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT

    #define WEB_SERVER_SUPPORT              0
    #define TUTK_SUPPORT                    0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) ||(HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        1
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    
	#define MR8600_DEMO_USE_PKEY           0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480
 
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_H264)
	#define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         2       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               2
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   1024
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                  0
    #define RFIU_TEST                     0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480
    
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_VMD)
    #define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         4       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               5
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   0
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //%%%%%%%%%%%%%%%%%%Software Motion Option%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define VMDSW                          1

    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_PIP)
	#define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x06
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode
    #define HWPIP_SUPPORT                   1   // A1016B & A1018B support

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                  0
    #define RFIU_TEST                     0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU)
    #define CHIP1016_ID_CODE               0xff

    #define CIU_SUPPORT 					1
	#define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0

    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1 
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x06
    #define MULTI_CHANNEL_LOCAL_MAX         4       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               5
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                   0
    #define USE_DMA_MODE                  0
    #define USE_CPU_MODE                  1
    #define USE_INTERRUPT                 0
    #define USE_POLLING                   1

    #define GPI_BUF_TX_MODE               USE_DMA_MODE
    #define GPI_BUF_RX_MODE               USE_DMA_MODE
    #define GPI_TRG_TX_MODE               USE_POLLING
    #define GPI_TRG_RX_MODE               USE_INTERRUPT

    #define WEB_SERVER_SUPPORT            0
    #define TUTK_SUPPORT                  0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   0
    #define RFIU_TEST                      0

    #define RF_DEMO_RESOLUTION             0
    #define RF_TX_WIDTH                    640
    #define RF_TX_HEIGHT                   480

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
	#define RFIC_SEL                       RFIC_A7130_4M
  #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AMIC7130_PAON                   0
  #else  
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
  #endif
    #define AMIC7196_USE_EXT_PA             0
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_TX_WAKEUP_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    #define CDVR_REC_WITH_PLAY_SUPPORT     1
    //=====JPEG file App insert option=====//
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define JPGAPP3_WIDTH                   704
    #define JPGAPP3_HEIGHT                  480

#endif



//-----------------------------------------------------------------------------------------------//
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% CPU MMU/Cache Opton %%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    //== MMU and L1 cache enable==//
#if(CHIP_OPTION == CHIP_A1013A)  //Lucian: A1013A 開 MMU會造成系統當機.
    #undef MMU_SUPPORT
#elif (CHIP_OPTION == CHIP_A1013B)  //K310 use
    #undef MMU_SUPPORT
#else  //K310 use
    #undef  MMU_SUPPORT
#endif

    //==L2 cache and TCM enable==//
#if( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1020A) ||\
(CHIP_OPTION == CHIP_A1026A))
    #define EXT_CACHE_SUPPORT
    #define TCM_DMA_SUPPORT
#elif(CHIP_OPTION == CHIP_A1013B)
    #define EXT_CACHE_SUPPORT
    #define TCM_DMA_SUPPORT
#elif( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    #define EXT_CACHE_SUPPORT
    #define TCM_DMA_SUPPORT
#else
    #undef  EXT_CACHE_SUPPORT
    #undef  TCM_DMA_SUPPORT
#endif



    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Motion Detection Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    //==SW Motion detection==//
    //Software Motion Option// 20170310
    #if VMDSW
        #define NEW_VMDSW_TEST                 0 //A1020 use
        #if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
        #define NEW_VMDSW                      1 //A1020/A1026 use
        #endif

        #if (NEW_VMDSW_TEST || NEW_VMDSW)
        #define INTERPOLATION                  0
        #define MEAN_Width                     40
        #define MEAN_Height                    30
        #else
        #define INTERPOLATION                  1
        #define MEAN_Width                     20
        #define MEAN_Height                    15
        #endif
        #define PeriodTime                     6
        
        #if INTERPOLATION
        #define MEAN_Width_INTER               39
        #define MEAN_Height_INTER              29
        #endif
    #endif    
    #define MOTIONDETEC_ENA                 0           //SW Motion detection support
	#define MD_METHOD_OPTION		        MD_METHOD_3 //used in SW MD

    //==HW Motion detection==//
#if( CHIP_OPTION == CHIP_A1013A )
    #define HW_MD_SUPPORT                   0    //HW Motion detection support, only valid in A1013B,A1016
#elif(CHIP_OPTION == CHIP_A1013B)
    #define HW_MD_SUPPORT                   0
#elif( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1020A) ||\
(CHIP_OPTION == CHIP_A1026A))
    #define HW_MD_SUPPORT                   1    //HW Motion detection support, only valid in A1013B,A1016
#elif(CHIP_OPTION == CHIP_A1016B)
    #define HW_MD_SUPPORT                   0    //HW Motion detection support, only valid in A1013B,A1016
#elif(CHIP_OPTION == CHIP_A1018A)
    #define HW_MD_SUPPORT                   1    //HW Motion detection support, only valid in A1013B,A1016
#elif(CHIP_OPTION == CHIP_A1018B)
    #define HW_MD_SUPPORT                   1    //HW Motion detection support, only valid in A1013B,A1016
#else
    #define HW_MD_SUPPORT                   0    //HW Motion detection support, only valid in A1013B,A1016
#endif

    //==MD level==//
    #define MD_SENSITIVITY_LEVEL            3

    #define PREVIEW_MD_TRIGGER_REC          0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% SIU/CIU/IPU/ISU/IDU Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

    #define AE_report                       AE_report_Hard  // use HW AE report

#if( (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1) ||\
     (SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )  /* Roy: Reduce memory usage*/
    #define ISU_OVERLAY_ENABLE              0    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
#else
    #define ISU_OVERLAY_ENABLE              1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
#endif
    #define FINE_TIME_STAMP                 USE_TIMER1_FINE_TIME_STAMP// 0: only use IIS time to calculate frame time, 1: use IIS time + Timer3 to calculate frame time
    #undef  NEW_ADC_MECHANISM                    // 7 times sampling per 25ms

    #define SIU_AWBCALIB_ENA                0
    #define SIU_OB_DPIX_CALIB_ENA           0

    #define PA9002D_AWB_EN                  1    //Lucian: 用於PA9002D, AWB gain 作用於 SIU pre-gamma.
    #define PA9003_AWB_GAIN_IN_SIU          1    //Lucian: AWB gain 作用於 SIU pre-gamma digital gain. 可避免白色變成粉紅色(9003 bug).
	#define DROP_BOTTOMFIELD   				0	/* drop bottom field in isu */

   #if CIU_TEST
    #define ISUCIU_PREVIEW_PNOUT            1
    #define IDUTV_DISPLAY_PN                1

   #elif SUB_TV_TEST
    #define ISUCIU_PREVIEW_PNOUT            0
    #define IDUTV_DISPLAY_PN                0

   #elif MULTI_CHANNEL_SUPPORT
    #define ISUCIU_PREVIEW_PNOUT            1
    #define IDUTV_DISPLAY_PN                1
   #else
    #define ISUCIU_PREVIEW_PNOUT            0
    #define IDUTV_DISPLAY_PN                0
   #endif



    #define TV_DISP_BY_IDU                   1  //解決TV out full resolution 畫面晃動問題.
    #define TV_DISP_BY_TV_INTR               1  //Lucian: 在PA9003 , 新增 TV field intr, 可取代timer-3,於bottom field 切換frame..
    #define AF_ZOOM_OPTION                   AF_ZOOM_OFF
    #define NUMS_PHOTOS_PREVIEW              4
    #define FACTORY_TOOL                     TOOL_OFF
    #define YUV_CCM_MERGE_ENA                0              // IPU YUV_CCM Merge
    #define SIU_MCLK_48M_CAPTURE             0

#if(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_DIU)
    #define HW_DEINTERLACE_CIU1_ENA          1
    #define HW_DEINTERLACE_CIU2_ENA          0
    #define HW_DEINTERLACE_CIU3_ENA          0
    #define HW_DEINTERLACE_CIU4_ENA          0
#else
    #define HW_DEINTERLACE_CIU1_ENA          0
    #define HW_DEINTERLACE_CIU2_ENA          0
    #define HW_DEINTERLACE_CIU3_ENA          0
    #define HW_DEINTERLACE_CIU4_ENA          0
#endif
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% SD High/Normal speed Option %%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SUPPORT_SD_HIGHSPEED_MODE       0

	#define SDC_SDCD_ENA                    1
    
    #define SDC_WRPROT_ENA                  0
    #define SDC_ECC_DETECT                  1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Digital TV singnal format Select %%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #if((HW_BOARD_OPTION == MR8600_RX_TRANWO ) || (HW_BOARD_OPTION == MR8600_RX_RDI2) || (HW_BOARD_OPTION == MR8600_RX_SKYSUCCESS) || (HW_BOARD_OPTION == MR8600_RX_GCT) )
    #define TV_DigiOut_SEL                  TV_DigiOut_656
    #else
    #define TV_DigiOut_SEL                  TV_DigiOut_601
    #endif
    #if(HW_BOARD_OPTION == MR8600_RX_SKYSUCCESS)
    #define TV_BT656_54M_ENA                1   /* 0:nothing 1: TV1, TV2 output 54M */
    #else
    #define TV_BT656_54M_ENA                0   /* 0:nothing 1: TV1, TV2 output 54M */
    #endif
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Others %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define DRAMCNTRL_SEL                   DRAMCNTRL_DDR2  // use New DDR controller
	#define MEMCPY_DMA                      1               // use HW DMA

    //-------WATCH Dog Config-------//
 #if( (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_SIU) ||  (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_CIU) ||  (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_DIU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_IDU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_H264) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_PIP) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
    #define WATCH_DOG_SEL                   WATCHDOG_OFF
 #elif( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
  (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) ||(HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
  (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))   
  
     #define WATCH_DOG_SEL                   WATCHDOG_OFF 
 #else
    #define WATCH_DOG_SEL                   WATCHDOG_INTERNAL
 #endif

	//-----------------------  //
	#define HIU_COMMAND 					0
	#define ISFILE							1
	#define FORCE_FPS                       0   // 0: nothing, N: insert dummy frames for MediaPlayer show N fps,

	//--SW IR--//
    #define IR_PPM_SUPPORT                  0   //SW IR Rx


	#define SHOW_UI_PROCESS_TIME			0
    #define TVOUT_UI_ENA                    1
    #define PANEL_UI_ENA                    1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Audio Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    /* AUDIO option
     *  AUDIO_IIS - IIS COMPATIBLE.
     *  AUDIO_AC97_ALC203 - REALTEK AC'97 ACL203.Lucian: 不建議使用.
     *  AUDIO_IIS_WM8974 -Wolfson WM8974
     *  AUDIO_ADC_DAC     -HIMAX build_in ADC_DAC
     *  AUDIO_IIS_ALC5621 - REALTEK IIS ALC5621
     */
 #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD)|| \
 (HW_BOARD_OPTION  == A1016B_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| \
 (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) ||(HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    #define AUDIO_OPTION		AUDIO_IIS
    #define ADC_SUBBOARD        ADC_SUBBOARD_Fuji

 #elif(HW_BOARD_OPTION  == A1020A_FPGA_BOARD)
    #define AUDIO_OPTION		AUDIO_ADC_DAC
    #define ADC_SUBBOARD        ADC_SUBBOARD_Fuji
 #else
    #define AUDIO_OPTION		AUDIO_ADC_DAC
    #define ADC_SUBBOARD        ADC_SUBBOARD_JUSTEK
 #endif
    #define AUDIO_BYPASS        0               // 1: AUDIO_IIS_ALC5621 bypass MIC to HP
    #define AUDIO_IN_TO_OUT		0               // 1: Playback the received audio data immediately, 0: Otherwise
    /* AUDIO codec
     *  AUDIO_CODEC_PCM         - PCM format.
     *  AUDIO_CODEC_MS_ADPCM    - Microsoft ADPCM format
     *  AUDIO_CODEC_IMA_ADPCM   - IMA ADPCM format
     */
    #define AUDIO_CODEC         AUDIO_CODEC_PCM
   



	//%%%%%%%%%%%%%%%%%%%%%%%%%%%UART COMMAND%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UART_COMMAND                    1

    #if(UART_GPS_COMMAND == 1)
        #define GPS_PROCESS_RMC     1
        #define GPS_PROCESS_VTG     0
        #define GPS_PROCESS_GGA     0
        #define GPS_PROCESS_GLL     0
        #define GPS_PROCESS_GSA     0
        #define GPS_PROCESS_GSV     0
    #else
        #define GPS_PROCESS_RMC     0
        #define GPS_PROCESS_VTG     0
        #define GPS_PROCESS_GGA     0
        #define GPS_PROCESS_GLL     0
        #define GPS_PROCESS_GSA     0
        #define GPS_PROCESS_GSV     0
    #endif


    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Key trigger option %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    /* Key trigger option
     *  KEY_TRIGGER_FPGA_ONBOARD - For FPGA onboard keypad.
     *  KEY_TRIGGER_EV2_ONBOARD - For EV2 onboard keypad.
     *  KEY_TRIGGER_EV2_EXTENDED - For EV2 extended keypad.
     *  KEY_TRIGGER_DVP_256_EXTENDED - For DVP_256 extended keypad
     *  KEY_TRIGGER_EV3_388_EXTENDED - For EV3_388 extended keypad
     *  KEY_TRIGGER_DVP_256_EXTENDED_ADCMIXGPIO - For DVP_256 extended keypad Adc mix GPIO
     *  KEY_TRIGGER_HIU_ADC_EXTENDED - For HIU_256 extended keypad Adc
     *  KEY_TRIGGER_AIPTEK_V5o       - For AIPTEK_V5o keypad Adc mix GPIO
     *  KEY_TRIGGER_SYS_9002D_256P   - For System Demo Board of 9002D-256P
     */
    #define KEY_TRIGGER_OPTION        KEY_TRIGGER_SYS_9002D_256P


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%RTC Configuration %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  #if((HW_BOARD_OPTION  == MR8100_GCT_LCD) || (HW_BOARD_OPTION == MR8100_RX_RDI_SEM)||\
        (HW_BOARD_OPTION == MR8100_RX_RDI_M512))
    #define USE_BUILD_IN_RTC                0
    #define EXT_RTC_SEL                     RTC_SD2068
  #elif( (HW_BOARD_OPTION == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I) || (HW_BOARD_OPTION == MR8120_RX_GCT_SC7700))
    #define USE_BUILD_IN_RTC                RTC_USE_TIMER_RTC
    #define EXT_RTC_SEL                     RTC_NULL
  #else
    #define USE_BUILD_IN_RTC                1
    #define EXT_RTC_SEL                     RTC_NULL
  #endif

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%File System Configuration %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

    #define FILE_SYSTEM_DVF_TEST            1
    #define FS_RW_DIRECT                    0    // 1: 用於壞黨修復,不用Dram cache,以減少暫存於Dram 的資料. 會降低檔案系統的效率.
    #define FILE_REPAIR_AUTO                0
    #define INIT_DISK_INFO                  1    // Initilize Disk label and others

    #define VIDEO_CONST_SIZE                0              // 1: video always constant file size(ASF_SIZE_PER_FILE), 0: otherwise
    #define FILE_REPLACE_NAME               0    // SUNIN 特殊要求

		#if (HW_BOARD_OPTION == MR6730_AFN)
		//move this definition to "ui\inc\MainFlow.h" 
		#else
		#define Get_sametime					0	 // 同時啟動錄影,開檔時間一致.
		#endif

    /* DVR File system selection
     * FILE_SYSTEM_DVR_SUB0  -just one group in 100VIDEO directory in DVR
     * FILE_SYSTEM_DVR_SUB1  -two group in 100VIDEO directory, Axxxxxxx.ASF : Manual; Bxxxxxxx.ASF : Event
     * FILE_SYSTEM_DVR_SUB2	-three group in 100VIDEO directory, Axxxxxxx.ASF : Manual; Bxxxxxxx.ASF : 2 min; Cxxxxxxx.ASF : Enent
    */

	#define SUB_FILE_SYSTEM_DVR_SEL FILE_SYSTEM_DVR_SUB0
    //#define CDVR_REC_WITH_PLAY_SUPPORT     1

	//For FILE_SYSTEM_CDVR
	//CDVR_LOG		: 0, need not create Log file
	//CDVR_LOG		: 1, need to create Log file
	#define CDVR_LOG		0

  #if(HOME_RF_SUPPORT)
    #define CDVR_iHome_LOG_SUPPORT          1
  #else
    #define CDVR_iHome_LOG_SUPPORT          0
  #endif


	//For FILE_SYSTEM_CDVR
	//ROOTWORK		: 0, Video Directory is not in Root Directory
	//ROOTWORK		: 1, Video Directory is in Root Directory
	#if ((HW_BOARD_OPTION == MR8200_RX_COMMAX) || (HW_BOARD_OPTION == MR8200_RX_COMMAX_BOX) || (HW_BOARD_OPTION == MR8211_ZINWELL) || (HW_BOARD_OPTION  == MR6730_AFN))
	#define ROOTWORK		1
    #else
    #define ROOTWORK		0
    #endif

    //%%%%%%%%%%%%%%%%%%%%%%% Dead Lock Monitor %%%%%%%%%%%%%%%%%%%%%%//
  #if( (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFU) )
    #define DEADLOCK_MONITOR_ENA            0
    #define SYSTEM_LIFETIME_MAX             (1*365*24*60*60) // 最大持續開機時間(second unit)
    #define SYSTEM_DEADLOCKTIME_MAX         (30) // Main task 最大死當時間(second unit)
  #else
    #define DEADLOCK_MONITOR_ENA            1
    #define SYSTEM_LIFETIME_MAX             (1*365*24*60*60) // 最大持續開機時間(second unit)
    #define SYSTEM_DEADLOCKTIME_MAX         (30) // Main task 最大死當時間(second unit)
  #endif

	//%%%%%%%%%%%%%%%%%%%%%%%	TV-in format detect Option %%%%%%%%%%%%%%%%%%%%%%//
	#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN)
    #define TVIN_FORMAT_DETECT_MODE         TV_IN_FORMAT_DETECT_ONCE//決定偵測的頻率
    #else
	#define TVIN_FORMAT_DETECT_MODE         TV_IN_FORMAT_DETECT_AUTO//決定偵測的頻率
	#endif
    #define TVIN_FORMAT_DETECT_METHOD       TV_IN_DETECT_BY_DECODER      //決定偵測的方法


	//%%%%%%%%%%%%%%%%%%%%%%%	USB Functions Option %%%%%%%%%%%%%%%%%%%%%%//
	/*	USB Mass Storage Access Option
	 *
	 *  USB_MSC_READ_WRITE	-	Surpport READ and WRITE.
	 *  USB_MSC_READ_ONLY	-	Surpport READ ONLY, WRITE function is prohibited.
	 */
	#define USB_MSC_FUNC_OPTION			     USB_MSC_READ_WRITE
    #define USB_HOST                         0
    #define USB_DEVICE                       0



    //%%%%%%%%%%%%%%%%%%%%%%%	File close method %%%%%%%%%%%%%%%%%%%%%%//
    #define TRIGGER_MODE_CLOSE_FILE_METHOD   CLOSE_FILE_BY_SLICE
    #define MANUAL_MODE_CLOSE_FILE_METHOD    CLOSE_FILE_BY_SLICE


    //%%%%%%%%%%%%%%%%%%%%%%%	DDR Control %%%%%%%%%%%%%%%%%%%%%%//
    #define DDR_SLEWRATE_CONTROL_FOR_CLOCK             1       //0:fast output slew Rate, 1:slow output slew Rate

	//%%%%%%%%%%%%%%%%%%%%%%%	Remote File Playback %%%%%%%%%%%%%%%%%%%%%%//


    #if (HW_BOARD_OPTION == MR8211_ZINWELL)
	#define REMOTE_TALK_BACK    1
    #else
	#define REMOTE_TALK_BACK    0
    #endif

#endif  // _APPLICATION_H
