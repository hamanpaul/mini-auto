#ifndef _APPLICATION_H
#define _APPLICATION_H

#include "project.h"




#if (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1)
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define CIU_SP_REP_MP                   0    //for test

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
 #if CIU2_REPLACE_CIU1
    #define MULTI_CHANNEL_SEL               0x04
 #else
    #define MULTI_CHANNEL_SEL               0x02
 #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define RECORD_SOURCE                   LOCAL_RECORD
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if FPGA_BOARD_A1018_SERIES
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130


    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT        0

    //-----//
    #define TX_FW_UPDATE_SUPPORT           0
    #define TX_SNAPSHOT_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2)  //For Audio Line-in
    #define CHIP1018_ID_CODE                0x08
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    1
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
 #if CIU2_REPLACE_CIU1
    #define MULTI_CHANNEL_SEL               0x04
 #else
    #define MULTI_CHANNEL_SEL               0x02
 #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if FPGA_BOARD_A1018_SERIES
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif

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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130


    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1)  //Baby monitor use
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
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
 #if CIU2_REPLACE_CIU1
    #define MULTI_CHANNEL_SEL               0x04
 #else
    #define MULTI_CHANNEL_SEL               0x02
 #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if FPGA_BOARD_A1018_SERIES
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            1
    #define SWAP_MULTI_STREAM_SUPPORT       1  // MP and SP swap.
    #define RF_TX_OPTIMIZE                  1  // 720P:0, 1080P:1;
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
    #if CIU2_REPLACE_CIU1
        #define MULTI_CHANNEL_SEL               0x04
    #else
        #define MULTI_CHANNEL_SEL               0x02
    #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if FPGA_BOARD_A1018_SERIES
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif

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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0

    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM)
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            1
    #define SWAP_MULTI_STREAM_SUPPORT       1  // MP and SP swap.
    #define HD_SWAP_MPSP_EN                 1
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
    #if CIU2_REPLACE_CIU1
        #define MULTI_CHANNEL_SEL               0x04
    #else
        #define MULTI_CHANNEL_SEL               0x02
    #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if FPGA_BOARD_A1018_SERIES
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif

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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0

    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5)
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
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
    #define MULTI_CHANNEL_SEL               0x20
    #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               6
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1920
    #define RF_RX_DEC_HEIGHT_MAX            1080

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0

    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define TX_SNAPSHOT_SUPPORT             1
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM)
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            1
    #define SWAP_MULTI_STREAM_SUPPORT       1  // MP and SP swap.
    #define RF_TX_OPTIMIZE                  1  // 720P:0, 1080P:1;
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
    #define MULTI_CHANNEL_SEL               0x20
    #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               6
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if FPGA_BOARD_A1018_SERIES
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif

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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0

    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM)
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            1
    #define SWAP_MULTI_STREAM_SUPPORT       1  // MP and SP swap.
    #define HD_SWAP_MPSP_EN                 0
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
    #define MULTI_CHANNEL_SEL               0x20
    #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               6
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if FPGA_BOARD_A1018_SERIES
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif

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
    
    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0

    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD)
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            1
    #define SWAP_MULTI_STREAM_SUPPORT       1  // MP and SP swap.
    #define HD_SWAP_MPSP_EN                 0
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
    #define MULTI_CHANNEL_SEL               0x20
    #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               6
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if FPGA_BOARD_A1018_SERIES
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif

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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0

    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0  //for A7196
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1  //for A7196

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    1  //for A7130

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            1
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0

#elif (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    //------------PWIfI---------------------//
	#define ICOMMWIFI_SUPPORT				1   
    #define PWIFI_SUPPORT                   1     // support P-wifi
    
    #define ICOMMWIFI_AP_MODE               0     //0:STA,   1:AP mode
    #define MAC_ADDR_SEL                    2     //Select test Mac address: 1,2,3
    #define DO_TXRX_CHKSUM                  0     //加入checksum 機制,會降低bitrate 10%
    #define TRANS_PKT_SIZE                  2     // 1 :1k  ;  2 : 2k
    
    #define PWIFI_PKT_EOF_SUPPORT           1     //*1: Send EOF 0: Close EOF*/
    #define PWIFI_PAIR_SUPPORT              0

  #if PWIFI_SUPPORT
    #define ICOM_SPI_USE_POLLING            0  
  #else
    #define ICOM_SPI_USE_POLLING            1  
  #endif
    //--------------------------------------//
    #define MULTI_STREAM_SUPPORT            1
    #define SWAP_MULTI_STREAM_SUPPORT       1  // MP and SP swap.
    #define HD_SWAP_MPSP_EN                 0
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
    #define MULTI_CHANNEL_SEL               0x20
    #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               6
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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

    #define LOGIN_FAIL_REBOOT_DISABLE       1
    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if FPGA_BOARD_A1018_SERIES
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif

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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME       0
    #define RFIU_RX_SHOW_ONLY              0

    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0  //for A7196
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1  //for A7196

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    1  //for A7130

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            1
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
    

#elif (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5)
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
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
    #define MULTI_CHANNEL_SEL               0x20
    #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               6
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1920
    #define RF_RX_DEC_HEIGHT_MAX            1080

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define TX_SNAPSHOT_SUPPORT             1
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD)
    #define CHIP1018_ID_CODE                0x01
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            1
    #define SWAP_MULTI_STREAM_SUPPORT       1  // MP and SP swap.
    #define HD_SWAP_MPSP_EN                 0
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
    #define MULTI_CHANNEL_SEL               0x20
    #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               6
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   1024
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if FPGA_BOARD_A1018_SERIES
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif

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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0

    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0  //for A7196
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1  //for A7196
    
    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  1    //HW Motion detection support
    #define VMDSW                          1
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            1    // 加入TX FW 更新機制.
    #define TX_PIRREC_SUPPORT               1    // Battery cam 預錄機制
    #define TX_PIRREC_VMDCHK                1    // for battery camera, PIR trigger + VMD recheck.
    #define TX_PIRREC_RATE_CONTROL          1    // for battery camera, PIR trigger record rate control
    
    #define ISU_OVERLAY_ENABLE              1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                     0

    #define AUDIO_PCM_8BIT_SIGN             0
    
#elif (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)
   #define CHIP1018_ID_CODE                0x01
   #define DRAM_MEMORY_END                 (0x82000000-4)

   #define CIU_SUPPORT_EN                  1
   #define CIU1_SCUP_EN                    1
   #define CIU2_SCUP_EN                    1
   #define CIU3_SCUP_EN                    1
   #define CIU4_SCUP_EN                    1
   #define CIU5_SCUP_EN                    1
   #define SIU_SCUP_EN                     0

   #define MULTI_STREAM_SUPPORT            1
   #define SWAP_MULTI_STREAM_SUPPORT       1  // MP and SP swap.
   #define HD_SWAP_MPSP_EN                 0
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
   #define MULTI_CHANNEL_SEL               0x20
   #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
   #define MULTI_CHANNEL_MAX               6
   #define RECORD_SOURCE                   RX_RECEIVE
   #define QUARD_MODE_DISP_SUPPORT         0
   #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

   #define VIDEO_BUF_NUM                   1024
   #define IIS_BUF_NUM                     256     // 32.768 second


   //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
   #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
   #define IDU_TV_DISABLE                  1   //disable TV and IDU for power saving.
   #define TVDAC_DISABLE                   0   // 1: disable TV DAC
   //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
   #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
   #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

   #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

   #define RF_RX_2DISP_WIDTH               640
   #define RF_RX_2DISP_HEIGHT              480

   #define RF_RX_DEC_WIDTH_MAX             1280
   #define RF_RX_DEC_HEIGHT_MAX            720

   #define RF_RX_DEC_WIDTH_spite            800
   #define RF_RX_DEC_HEIGHT_spite           480

   //--config RF PHY--//
#if FPGA_BOARD_A1018_SERIES
   #define AMIC7130_PAON                   0
#else
   #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
#endif

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

   //==RX(8CH) 共用一個 RF module==//
   #define RFI_TEST_8TX_1RX_PROTOCOL      0
   
   //==RX(8CH) 共用兩個 RF module==//
   #define RFI_TEST_8TX_2RX_PROTOCOL      0

   //==RF Wrap Config==//
   #define RFIU_RX_AVSYNC                 1
   #define RFIU_RX_AUDIO_ON               1
   #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

   #define RFIU_RX_AUDIO_RETURN           1
   #define RFIU_TX_PIR_TRIG               1

   #define RFIU_RX_WAKEUP_TX_SCHEME          0
   #define RFIU_RX_SHOW_ONLY              0

   //======//
   #define RFIU_DATA_6M_ACK_4M_SUPPORT    0  //for A7196
   #define RFIU_DATA_6M_ACK_3M_SUPPORT    1  //for A7196
   
   #define RFIU_DATA_4M_ACK_2M_SUPPORT    1  //for A7130

   //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
   #define UI_SUPPORT_TREE                0
   #define UI_PREVIEW_OSD                 0
   //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
   #define USB_HOST_MASS_SUPPORT          0
   #define USB_DONGLE_SUPPORT             0
   //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
   #define HDCVIENC_SUPPORT               0
   //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
   #define AHD_NVP6021_SUPPORT            0
   //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
   #define HW_MD_SUPPORT                  1    //HW Motion detection support
   #define VMDSW                          1
   //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
   #define AES_DES_SUPPORT                0

   //-----//
   #define TX_FW_UPDATE_SUPPORT            1    // 加入TX FW 更新機制.
   #define TX_PIRREC_SUPPORT               1    // Battery cam 預錄機制
   #define TX_PIRREC_VMDCHK                0    // for battery camera, PIR trigger + VMD recheck.
   #define TX_PIRREC_RATE_CONTROL          1    // for battery camera, PIR trigger record rate control
   
   #define ISU_OVERLAY_ENABLE              1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
   #define GFU_SUPPORT                     0

   #define AUDIO_PCM_8BIT_SIGN             0

#elif(SW_APPLICATION_OPTION == MR8200_RFCAM_RX1)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x84000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU_SCUP_EN                     0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1088

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480
    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130

    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          1
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2)
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x84000000-4)
	#define CHECK_VIDEO_BITSTREAM			1
    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1088

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
	#define RTC_WEEKDAY_CALCULATE			0 //Add by Paul for RTC weekday calculate

#elif(SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2)
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x84000000-4)
	#define USE_NEW_MEMORY_MAP              1
	#define CHECK_VIDEO_BITSTREAM			1
    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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
#if(HOME_RF_SUPPORT)
    #define A7128_SUPPORT					1
	#define MARSS_SUPPORT					1
#else
    #define A7128_SUPPORT					0
	#define MARSS_SUPPORT					0
#endif

    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1088

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    1  //for A7130
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          1
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT        0


    //-----//
    #define TX_FW_UPDATE_SUPPORT           1
    #define ISU_OVERLAY_ENABLE             0    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
	#define RTC_WEEKDAY_CALCULATE			1//Add by Paul for RTC weekday calculate

#elif(SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x84000000-4)
	#define USE_NEW_MEMORY_MAP              1
	#define CHECK_VIDEO_BITSTREAM			1
    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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
    #define P2P_LV_RATECTL_SUPPORT          0
    #define ENABLE_TUTK_RESEND              0
    #define MAX_CLIENT                      3
    #define MAX_AV_CH                       MAX_RFIU_UNIT
    #define TUTK_RESEND_BUF_SIZE            4096/(MAX_CLIENT*MAX_AV_CH)
    #define DEVSTATUS_ACTIVE_UPDATE         0
    #define P2P_IOCMD_TIMEOUT_HANDLE        1
    #define VIDEO_APPEND_AUDIO_SUPPORT      1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1088

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME       1
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          1
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT        0


    //-----//
    #define TX_FW_UPDATE_SUPPORT           1
    #define ISU_OVERLAY_ENABLE             0    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
	#define RTC_WEEKDAY_CALCULATE			1//Add by Paul for RTC weekday calculate
    
    #define LWIP_BLK_SIZE                  2560
    #define LWIP_NBLKS                     32
    #define LWIP_ALL_BLK_SIZE              (LWIP_NBLKS*LWIP_BLK_SIZE)
    
    #define LWIP2_BLK_SIZE                 256
    #define LWIP2_NBLKS                    6
    #define LWIP2_ALL_BLK_SIZE              (LWIP2_NBLKS*LWIP2_BLK_SIZE)    
    
    #define LWIP3_BLK_SIZE                 (4096+32)
    #define LWIP3_NBLKS                    32
    #define LWIP3_ALL_BLK_SIZE             (LWIP3_NBLKS*LWIP3_BLK_SIZE)        


#elif(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
    #define CHIP1018_ID_CODE                0xff
	#define SHOW_LOGO						1
    #define USE_NEW_MEMORY_MAP              1
	#define CHECK_VIDEO_BITSTREAM			1
	#if(USE_NEW_MEMORY_MAP)
	#define DISPLAY_BUFFER_Y_OFFSET			(1920*180)
	#define DISPLAY_BUFFER_UV_OFFSET		(1920*90)
	#else
	#define DISPLAY_BUFFER_Y_OFFSET			(0)
	#define DISPLAY_BUFFER_UV_OFFSET		(0)
	#endif
    #define DRAM_MEMORY_END                 (0x84000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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
    #define TUTK_SUPPORT                   1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% PWIFI Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    #define ICOMMWIFI_SUPPORT               1
    #define PWIFI_SUPPORT                   1     // support P-wifi
            
    #define ICOMMWIFI_AP_MODE               1     //0:STA,   1:AP mode
    #define MAC_ADDR_SEL                    3     //Select test Mac address: 1,2,3
    #define DO_TXRX_CHKSUM                  0     //加入checksum 機制,會降低bitrate 10%
    #define TRANS_PKT_SIZE                  2     // 1 :1k  ;  2 : 2k

    #define PWIFI_PKT_EOF_SUPPORT           1     //*1: Send EOF 0: Close EOF*/
    #define PWIFI_PAIR_SUPPORT              0


#if PWIFI_SUPPORT
    #define ICOM_SPI_USE_POLLING            0  
#else
    #define ICOM_SPI_USE_POLLING            1  
#endif


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1088

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1        0
    #define RFI_TEST_TX_PROTOCOL_B2        0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1        0
    #define RFI_TEST_RX_PROTOCOL_B2        0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
 #if PWIFI_SUPPORT
    #define RFI_TEST_4TX_2RX_PROTOCOL      0
 #else
    #define RFI_TEST_4TX_2RX_PROTOCOL      1
 #endif   

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
 #if PWIFI_SUPPORT
    #define RFI_TEST_4x_RX_PROTOCOL_B1     1
 #else   
    #define RFI_TEST_4x_RX_PROTOCOL_B1     0
 #endif
    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0
    
    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          1
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    1  //for A7130
    //==//
    #define RFIU_AUTO_UNPAIR               1
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          1
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT        1 //先關閉

    //-----//
    #define TX_FW_UPDATE_SUPPORT            1
    #define RX_SNAPSHOT_SUPPORT             1
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    1

#elif(SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)
    #define CHIP1018_ID_CODE                0xff
	#define SHOW_LOGO						1
    #define USE_NEW_MEMORY_MAP              1
	#define CHECK_VIDEO_BITSTREAM			1
	#if(USE_NEW_MEMORY_MAP)
	#define DISPLAY_BUFFER_Y_OFFSET			(1920*180)
	#define DISPLAY_BUFFER_UV_OFFSET		(1920*90)
	#else
	#define DISPLAY_BUFFER_Y_OFFSET			(0)
	#define DISPLAY_BUFFER_UV_OFFSET		(0)
	#endif
    #define DRAM_MEMORY_END                 (0x84000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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
    #define TUTK_SUPPORT                   1


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                   1
    #define RFIU_TEST                      0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1088

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0
    
    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          1
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          1
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT        1 //先關閉

    //-----//
    #define TX_FW_UPDATE_SUPPORT            1
    #define RX_SNAPSHOT_SUPPORT             0
    #define ISU_OVERLAY_ENABLE              1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                     0

    
#elif(SW_APPLICATION_OPTION == MR8120_RFAVSED_RX1)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL        RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1088

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0

	#define MR8600_DEMO_USE_PKEY           0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          1
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1)
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   1   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL        RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1080

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0

	#define MR8600_DEMO_USE_PKEY           0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               1
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1)
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   1   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL        RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1080

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
	#define MR8600_DEMO_USE_PKEY           0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            1
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR9100_RF_AHDIN_AVSED_RX1)
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   1   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL        RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1080

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
	#define MR8600_DEMO_USE_PKEY           0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            1
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2) //MR8600-216M
    #define CHIP1018_ID_CODE                0x09
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
 #if USB_DONGLE_RX_YOUT_DEBUG_ENA
    #define IDU_TV_DISABLE                  0
 #else
    #define IDU_TV_DISABLE                  1
 #endif
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME       0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             1
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT        0

    //-----//
    #define TX_FW_UPDATE_SUPPORT           0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0

#elif(SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) 
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% PWIFI Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	#define ICOMMWIFI_SUPPORT				1
    #define PWIFI_SUPPORT                   1     // support P-wifi
    
    #define ICOMMWIFI_AP_MODE               1     //0:STA,   1:AP mode
    #define MAC_ADDR_SEL                    2     //Select test Mac address: 1,2,3
    #define DO_TXRX_CHKSUM                  0     //加入checksum 機制,會降低bitrate 10%
    #define TRANS_PKT_SIZE                  2     // 1 :1k  ;  2 : 2k
    
    #define PWIFI_PKT_EOF_SUPPORT           1     //*1: Send EOF 0: Close EOF*/
    #define PWIFI_PAIR_SUPPORT              0
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
 #if USB_DONGLE_RX_YOUT_DEBUG_ENA
    #define IDU_TV_DISABLE                  0
 #else
    #define IDU_TV_DISABLE                  1
 #endif
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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

    #define LOGIN_FAIL_REBOOT_DISABLE       1
    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME       1
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             1
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT        0

    //-----//
    #define TX_FW_UPDATE_SUPPORT           0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
  
#elif(SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) //MR8600-216M
    #define CHIP1018_ID_CODE                0x09
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
 #if USB_DONGLE_RX_YOUT_DEBUG_ENA
    #define IDU_TV_DISABLE                  0
 #else
    #define IDU_TV_DISABLE                  1
 #endif
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      1

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME       1
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             1
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            1
    #define ISU_OVERLAY_ENABLE              1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                     0

    #define AUDIO_PCM_8BIT_SIGN             1

    
#elif(SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1RX2)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  0
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1       //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               2       // max video record channel support, include both local sensor and RF channel
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_VGA

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
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
	#define MR8600_DEMO_USE_PKEY           0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR8600_RFAVSED_RX1)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  0
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1       //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               2       // max video record channel support, include both local sensor and RF channel
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_VGA

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
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
	#define MR8600_DEMO_USE_PKEY           0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR8120_RFCAM_RX1) //MR8120-216M
	#define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x84000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define TUTK_SUPPORT                    0
    #define WEB_SERVER_SUPPORT              0

    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT



    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              360

    #define RF_RX_DEC_WIDTH_MAX             1920
    #define RF_RX_DEC_HEIGHT_MAX            1088

    #define RF_RX_DEC_WIDTH_spite           800
    #define RF_RX_DEC_HEIGHT_spite          480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          1
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) //MR8600-216M
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x84000000-4)
	#define CHECK_VIDEO_BITSTREAM			1


    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1088

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR8110_BABYMONITOR)
	#define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x84000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
    //%%%%%%%%%%%%%%%%%%%%%%%%%Network Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define NIC_SUPPORT                     0
    #define TUTK_SUPPORT                    0
    #define WEB_SERVER_SUPPORT              0

    #define USE_DMA_MODE                    0
    #define USE_CPU_MODE                    1
    #define USE_INTERRUPT                   0
    #define USE_POLLING                     1

    #define GPI_BUF_TX_MODE                 USE_DMA_MODE
    #define GPI_BUF_RX_MODE                 USE_DMA_MODE
    #define GPI_TRG_TX_MODE                 USE_POLLING
    #define GPI_TRG_RX_MODE                 USE_INTERRUPT



    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                    1
    #define RFIU_TEST                       0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              360

    #define RF_RX_DEC_WIDTH_MAX             1920
    #define RF_RX_DEC_HEIGHT_MAX            1088

    #define RF_RX_DEC_WIDTH_spite           800
    #define RF_RX_DEC_HEIGHT_spite          480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
    //=TX=//
    #define RFI_TEST_TX_PROTOCOL_B1         0
    #define RFI_TEST_TX_PROTOCOL_B2         0

    //==RX==//
    #define RFI_TEST_RX_PROTOCOL_B1         0
    #define RFI_TEST_RX_PROTOCOL_B2         0

    //==Rx+Rx(2CH) in the same PCB ==//
    #define RFI_TEST_RXRX_PROTOCOL_B1B2    0
    #define RFI_TEST_4TX_2RX_PROTOCOL      0

    //==Rx(2CH) 共用一個RF module ==//
    #define RFI_TEST_2x_RX_PROTOCOL_B1     0

    //==Rx(4CH) 共用一個RF module ==//
    #define RFI_TEST_4x_RX_PROTOCOL_B1     1

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0

    #define RFIU_RX_SHOW_ONLY              1
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR6730_CARDVR_2CH)  // 216M
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x84000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          1   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
    
#elif(SW_APPLICATION_OPTION == MR6730_CARDVR_1CH)  // 128M
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x84000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
    
#elif(SW_APPLICATION_OPTION == MR8211_IPCAM)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   0    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR9670_DOORPHONE)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x84000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode
    #define CIU_SPLITER                     0   // 4 CCIR MUX IN Enable  0:disable 1: enable

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

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
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

		//==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == DVP_RF_SELFTEST)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

	#define DISPLAY_BUFFER_Y_OFFSET			(0)
	#define DISPLAY_BUFFER_UV_OFFSET		(0)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  1
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x0
    #define MULTI_CHANNEL_LOCAL_MAX         4       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == Standalone_Test)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x00
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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
    #define RFIU_SUPPORT                  1
    #define RFIU_TEST                     0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME       1
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT         1
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            1
    #define RX_SNAPSHOT_SUPPORT             0
    #define ISU_OVERLAY_ENABLE              1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                     0
    
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_CIU)
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0
    #define NEW_CIU1_EN                     1

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
  #if(CHIP_OPTION == CHIP_A1019A)   
    #define MULTI_CHANNEL_SEL               0x20 //A1019 use CIU5
  #else
    #define MULTI_CHANNEL_SEL               0x02 //others use CIU1
  #endif
    
    #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               6
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite           800
    #define RF_RX_DEC_HEIGHT_spite          480

	//--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                  1
    #define RFIU_RX_AUDIO_ON                1
    #define RFIU_RX_TIME_SEL                RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN            0
    #define RFIU_TX_PIR_TRIG                0

    #define RFIU_RX_WAKEUP_TX_SCHEME           0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_USB_HOST)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #if CIU2_REPLACE_CIU1
        #define MULTI_CHANNEL_SEL               0x04
    #else
        #define MULTI_CHANNEL_SEL               0x02
    #endif
    #define MULTI_CHANNEL_LOCAL_MAX         4       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               5
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          1
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_DIU)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #if CIU2_REPLACE_CIU1
        #define MULTI_CHANNEL_SEL               0x04
    #else
        #define MULTI_CHANNEL_SEL               0x02
    #endif
    #define MULTI_CHANNEL_LOCAL_MAX         4       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               5
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_IDU)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #if CIU2_REPLACE_CIU1
        #define MULTI_CHANNEL_SEL               0x04
    #else
        #define MULTI_CHANNEL_SEL               0x02
    #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
    
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_SIU)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     1
    #define NEW_CIU1_EN                     1
    
    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x20
    #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               6
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         1
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite           800
    #define RF_RX_DEC_HEIGHT_spite          480

	//--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                  1
    #define RFIU_RX_AUDIO_ON                1
    #define RFIU_RX_TIME_SEL                RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN            0
    #define RFIU_TX_PIR_TRIG                0

    #define RFIU_RX_WAKEUP_TX_SCHEME           0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    

#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFU)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #if CIU2_REPLACE_CIU1
        #define MULTI_CHANNEL_SEL               0x04
    #else
        #define MULTI_CHANNEL_SEL               0x02
    #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite          800
    #define RF_RX_DEC_HEIGHT_spite         480

	//--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFTX1)
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #if CIU2_REPLACE_CIU1
        #define MULTI_CHANNEL_SEL               0x04
    #else
        #define MULTI_CHANNEL_SEL               0x02
    #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0   //disable TV and IDU for power saving.
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH               640
    #define RF_RX_2DISP_HEIGHT              480

    #define RF_RX_DEC_WIDTH_MAX             1280
    #define RF_RX_DEC_HEIGHT_MAX            720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               1

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFRX1)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
	#define MR8600_DEMO_USE_PKEY           0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_H264)
	#define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #if CIU2_REPLACE_CIU1
        #define MULTI_CHANNEL_SEL               0x04
    #else
        #define MULTI_CHANNEL_SEL               0x02
    #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   1024
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1088

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_H1_H264)
	#define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #if CIU2_REPLACE_CIU1
        #define MULTI_CHANNEL_SEL               0x04
    #else
        #define MULTI_CHANNEL_SEL               0x02
    #endif
    #define MULTI_CHANNEL_LOCAL_MAX         3       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               3
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   1024
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1088

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    

#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_PIP)
	#define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode
    #define HWPIP_SUPPORT                   1   // A1016B & A1018B support

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
    
#elif(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  1
    #define CIU1_SCUP_EN                    1
    #define CIU2_SCUP_EN                    1
    #define CIU3_SCUP_EN                    1
    #define CIU4_SCUP_EN                    1
    #define CIU5_SCUP_EN                    1
    #define SIU_SCUP_EN                     1

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x02
    #define MULTI_CHANNEL_LOCAL_MAX         5       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               6
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             480

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

	//--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           0
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 1
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    1
    
#elif(SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1)
    #define CHIP1018_ID_CODE                0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
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
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   2048
    #define IIS_BUF_NUM                     256     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 1   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  0
    #define TVDAC_DISABLE                   1   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         0    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              0    // 1: support motion trigger reocrd , 0: otherwise

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

    #define RF_RX_FULLVIEW_RESO_SEL        RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1920
    #define RF_RX_DEC_HEIGHT_MAX           1080

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME          0
    #define RFIU_RX_SHOW_ONLY              0
	#define MR8600_DEMO_USE_PKEY           0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    0

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                0
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT          0
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0

    //-----//
    #define TX_FW_UPDATE_SUPPORT            0
    #define ISU_OVERLAY_ENABLE             1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                    0
    
#elif(SW_APPLICATION_OPTION == MR8202_GATEWAYBOX_RX)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)

    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
    #define SIU_SCUP_EN                     0

    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0

    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x00
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode

    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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
	#define A7128_SUPPORT					1
	#define MARSS_SUPPORT					1


    //%%%%%%%%%%%%%%%%%%%%%%%%%%RF Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define RFIU_SUPPORT                  1
    #define RFIU_TEST                     0

    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD

    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360

    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720

    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480

    //--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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

    //==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
    
    //==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    #define RFIU_AUTO_UNPAIR               1

    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO

    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0

    #define RFIU_RX_WAKEUP_TX_SCHEME       1
    #define RFIU_RX_SHOW_ONLY              0
    //======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1

    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
    //%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 0
    //%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT         1
    #define USB_DONGLE_SUPPORT             0
    //%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
    //%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
    //%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
    //%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
    //%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
    //-----//
    #define TX_FW_UPDATE_SUPPORT            1
    #define RX_SNAPSHOT_SUPPORT             0
    #define ISU_OVERLAY_ENABLE              1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                     0
    
#elif(SW_APPLICATION_OPTION == MR8202_AN_KLF08W)
    #define CHIP1018_ID_CODE               0xff
    #define DRAM_MEMORY_END                 (0x82000000-4)
	
    #define CIU_SUPPORT_EN                  0
    #define CIU1_SCUP_EN                    0
    #define CIU2_SCUP_EN                    0
    #define CIU3_SCUP_EN                    0
    #define CIU4_SCUP_EN                    0
    #define CIU5_SCUP_EN                    0
    #define SIU_SCUP_EN                     0
	
    #define MULTI_STREAM_SUPPORT            0
    #define RF_TX_OPTIMIZE                  0
    #define CIU1_BOB_REPLACE_MPEG_DF        0
    #define CIU1_BOB_AUTO_MD                0
	
    #define TV_D1_OUT_FULL                  1
    #define TV_D1_OUT_FULL_HALF             0 //上下黑邊 左右滿頻 (TV_D1_OUT_FULL need 1)
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Preview Path Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_SUPPORT           1    //Valid in PA9003: max t0 2
    #define MULTI_CHANNEL_SEL               0x00
    #define MULTI_CHANNEL_LOCAL_MAX         0       // max local sensor video record channel support
    #define MULTI_CHANNEL_MAX               4
    #define RECORD_SOURCE                   RX_RECEIVE
    #define QUARD_MODE_DISP_SUPPORT         0
    #define DUAL_MODE_DISP_SUPPORT          0   // wired dule desplay mode
	
    #define VIDEO_BUF_NUM                   512
    #define IIS_BUF_NUM                     128     // 32.768 second
	
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%SD Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SD_CARD_DISABLE                 0   // 1: disable SD Card, 0: Otherwise
    #define IDU_TV_DISABLE                  1
    #define TVDAC_DISABLE                   0   // 1: disable TV DAC
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH Recoding%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_VIDEO_REC         1    // 1: support multi channel video reocrd at same time, 0: otherwise
    #define MOTION_TRIGGRT_REC              1    // 1: support motion trigger reocrd , 0: otherwise
	
		//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Multi-CH ASF Packer%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define MULTI_CHANNEL_RF_RX_VIDEO_REC   1    // 1: support multi channel RF Rx video reocrd at same time, 0: otherwise
	#define Audio_mode                      AUDIO_MANUAL
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
    #define RFIU_SUPPORT                  1
    #define RFIU_TEST                     0
	
    #define RF_RX_FULLVIEW_RESO_SEL         RF_RX_RESO_HD
	
    #define RF_RX_2DISP_WIDTH              640
    #define RF_RX_2DISP_HEIGHT             360
	
    #define RF_RX_DEC_WIDTH_MAX            1280
    #define RF_RX_DEC_HEIGHT_MAX           720
	
    #define RF_RX_DEC_WIDTH_spite            800
    #define RF_RX_DEC_HEIGHT_spite           480
	
		//--config RF PHY--//
 #if(FPGA_BOARD_A1018_SERIES)
    #define AMIC7130_PAON                   0
 #else
    #define AMIC7130_PAON                   1              //AMICCOM A7130 PA ON
 #endif
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
	
		//==RX(8CH) 共用一個 RF module==//
    #define RFI_TEST_8TX_1RX_PROTOCOL      0
		
		//==RX(8CH) 共用兩個 RF module==//
    #define RFI_TEST_8TX_2RX_PROTOCOL      0

    //==//
    #define RFIU_AUTO_UNPAIR               1
    
    //==RF Wrap Config==//
    #define RFIU_RX_AVSYNC                 1
    #define RFIU_RX_AUDIO_ON               1
    #define RFIU_RX_TIME_SEL               RFIU_RX_TIME_BY_VIDEO
	
    #define RFIU_RX_AUDIO_RETURN           1
    #define RFIU_TX_PIR_TRIG               0
	
    #define RFIU_RX_WAKEUP_TX_SCHEME       1
    #define RFIU_RX_SHOW_ONLY              0
		//======//
    #define RFIU_DATA_6M_ACK_4M_SUPPORT    0
    #define RFIU_DATA_6M_ACK_3M_SUPPORT    1
	
    #define RFIU_DATA_4M_ACK_2M_SUPPORT    0  //for A7130
		//%%%%%%%%%%%%%%%%%%%%%%%%%%UI Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define UI_SUPPORT_TREE                1
    #define UI_PREVIEW_OSD                 0
		//%%%%%%%%%%%%%%%%%%%%%%%USB Option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USB_HOST_MASS_SUPPORT         1
    #define USB_DONGLE_SUPPORT             0
		//%%%%%%%%%%%%%%% Suport DH9801 HD-CVI encoder %%%%%%%%%%%%%%%%//
    #define HDCVIENC_SUPPORT               0
		//%%%%%%%%%%%%%%% Suport HD-AHD encoder %%%%%%%%%%%%%%%%//
    #define AHD_NVP6021_SUPPORT            0
		//%%%%%%%%%%%%%%% Motion dectect option %%%%%%%%%%%%%%%%//
    #define HW_MD_SUPPORT                  0    //HW Motion detection support
    #define VMDSW                          0
		//%%%%%%%%%%%%%%%%%%%%% AES DES SUPPORT %%%%%%%%%%%%%%%%%%%%%%//
    #define AES_DES_SUPPORT                0
		//%%%%%%%%%%%%%%% System Log option %%%%%%%%%%%%%%%%//
    #define CDVR_SYSTEM_LOG_SUPPORT         0
		//-----//
    #define TX_FW_UPDATE_SUPPORT            1
    #define RX_SNAPSHOT_SUPPORT             0
    #define ISU_OVERLAY_ENABLE              1    // 0: disable time stamp, 1: draw time stamp to video frame, 2: draw time stamp to both video and display frame
    #define GFU_SUPPORT                     0
    #define DEVSTATUS_ACTIVE_UPDATE         1
    #define MAX_CLIENT                      3
    #define P2P_LV_RATECTL_SUPPORT          1
    #define P2P_IOCMD_TIMEOUT_HANDLE        1
#if (P2P_LV_RATECTL_SUPPORT == 1) //Rate control depend on TUTK resend
    #define ENABLE_TUTK_RESEND              1
#endif
    #define TUTK_RESEND_BUF_SIZE           512

    #define LWIP_BLK_SIZE                  2560
    #define LWIP_NBLKS                     32
    #define LWIP_ALL_BLK_SIZE              (LWIP_NBLKS*LWIP_BLK_SIZE)
    
    #define LWIP2_BLK_SIZE                 256
    #define LWIP2_NBLKS                    6
    #define LWIP2_ALL_BLK_SIZE              (LWIP2_NBLKS*LWIP2_BLK_SIZE)    
    
    #define LWIP3_BLK_SIZE                 (4096+32)
    #define LWIP3_NBLKS                    32
    #define LWIP3_ALL_BLK_SIZE             (LWIP3_NBLKS*LWIP3_BLK_SIZE)

#endif



//-----------------------------------------------------------------------------------------------//
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% CPU MMU/Cache Opton %%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    //== MMU and L1 cache enable==//
#if( (CHIP_OPTION == CHIP_A1019A) || (CHIP_OPTION == CHIP_A1021A) )
    #define MMU_SUPPORT
#else  //K310 use
    #undef  MMU_SUPPORT
#endif

    //==L2 cache and TCM enable==//
#if((CHIP_OPTION == CHIP_A1019A) || (CHIP_OPTION == CHIP_A1021A) )
    //#define EXT_CACHE_SUPPORT
    #define TCM_DMA_SUPPORT
#elif(CHIP_OPTION == CHIP_A1020A)
    #define EXT_CACHE_SUPPORT
    #define TCM_DMA_SUPPORT
#else
    #define EXT_CACHE_SUPPORT
    #define TCM_DMA_SUPPORT
#endif

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Motion Detection Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    //Software Motion Option// 20150330
    #if VMDSW
        #define NEW_VMDSW_TEST                 0 //A1019 use
        #if ( (CHIP_OPTION == CHIP_A1019A)||(CHIP_OPTION == CHIP_A1025A) || (CHIP_OPTION == CHIP_A1021A) )
        #define NEW_VMDSW                      1 //A1019 use
        #endif

        #define PeriodTime                     6
        #if (NEW_VMDSW_TEST || NEW_VMDSW)
        #define INTERPOLATION                  0
        #define MEAN_Width                     40
        #define MEAN_Height                    30
        #else
        #define INTERPOLATION                  1
        #define MEAN_Width                     20
        #define MEAN_Height                    15
        #endif

        #if INTERPOLATION
        #define MEAN_Width_INTER               39
        #define MEAN_Height_INTER              29
        #endif
    #endif
    //==SW Motion detection==//
    #define MOTIONDETEC_ENA                 0           //SW Motion detection support
	#define MD_METHOD_OPTION		        MD_METHOD_3 //used in SW MD


    //==MD level==//
    #define MD_SENSITIVITY_LEVEL            3

    #define PREVIEW_MD_TRIGGER_REC          0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% SIU/CIU/IPU/ISU/IDU Config%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//

    #define AE_report                       AE_report_Hard  // use HW AE report

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

   #else
    #define ISUCIU_PREVIEW_PNOUT            1
    #define IDUTV_DISPLAY_PN                1
   #endif



    #define AF_ZOOM_OPTION                   AF_ZOOM_OFF
    #define YUV_CCM_MERGE_ENA                0              // IPU YUV_CCM Merge
    #define SIU_MCLK_48M_CAPTURE             0

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% SD High/Normal speed Option %%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define SUPPORT_SD_HIGHSPEED_MODE       0
    #define SDC_SDCD_ENA                    1
    #define SDC_WRPROT_ENA                  0
    #define SDC_ECC_DETECT                  1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Digital TV singnal format Select %%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define TV_DigiOut_SEL                  TV_DigiOut_601

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Others %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define DRAMCNTRL_SEL                   DRAMCNTRL_DDR2  // use New DDR controller
	#define MEMCPY_DMA                      1               // use HW DMA

    //-------WATCH Dog Config-------//
 #if( (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_SIU) ||  (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_CIU) ||  (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_DIU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_IDU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_H264) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_PIP) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_USB_HOST) || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_H1_H264))
    #define WATCH_DOG_SEL                   WATCHDOG_OFF
 #else
    #define WATCH_DOG_SEL                   WATCHDOG_INTERNAL
 #endif

	//-----------------------  //
	#define HIU_COMMAND 					0
#if ((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM)|| (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
     (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) ||(SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) ||(SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
	#define ISFILE							0
#else
	#define ISFILE							1
#endif    
	#define FORCE_FPS                       0   // 0: nothing, N: insert dummy frames for MediaPlayer show N fps,

	//--SW IR--//
    #define IR_PPM_SUPPORT                  0   //SW IR Rx


	#define SHOW_UI_PROCESS_TIME			0
    #define TVOUT_UI_ENA                    1
    #define PANEL_UI_ENA                    1

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


	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%File System Configuration %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
	//JPEG file App insert option
	#define ADDAPP2TOJPEG                   0
    #define ADDAPP3TOJPEG                   0
    #define ADDAPP1TOJPEG                   0

    #define FILE_SYSTEM_DVF_TEST            1
    #define FS_RW_DIRECT                    0    // 1: 用於壞黨修復,不用Dram cache,以減少暫存於Dram 的資料. 會降低檔案系統的效率.
    #define FILE_REPAIR_AUTO                0
    #define INIT_DISK_INFO                  1    // Initilize Disk label and others

    #define VIDEO_CONST_SIZE                0              // 1: video always constant file size(ASF_SIZE_PER_FILE), 0: otherwise
    #define FILE_REPLACE_NAME               0    // SUNIN 特殊要求


    /* DVR File system selection
     * FILE_SYSTEM_DVR_SUB0  -just one group in 100VIDEO directory in DVR
     * FILE_SYSTEM_DVR_SUB1  -two group in 100VIDEO directory, Axxxxxxx.ASF : Manual; Bxxxxxxx.ASF : Event
     * FILE_SYSTEM_DVR_SUB2	-three group in 100VIDEO directory, Axxxxxxx.ASF : Manual; Bxxxxxxx.ASF : 2 min; Cxxxxxxx.ASF : Enent
    */

	#define SUB_FILE_SYSTEM_DVR_SEL         FILE_SYSTEM_DVR_SUB0
    #define CDVR_REC_WITH_PLAY_SUPPORT      1

	//For FILE_SYSTEM_CDVR
	//CDVR_LOG		: 0, need not create Log file
	//CDVR_LOG		: 1, need to create Log file
	#define CDVR_LOG		                0

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
	#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
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

    #if USB_HOST_MASS_SUPPORT
	    #define USB_HOST                         1
      #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
	    #define AUTO_STORAGE_CHANGE              0
        #define USB_HOST_DVR                     1
      #else
      	#define AUTO_STORAGE_CHANGE              1
        #define USB_HOST_DVR                     0
      #endif
	#elif   USB_HOST_MOUSE
		#define USB_HOST                         1
	    #define AUTO_STORAGE_CHANGE              0
    #else
	    #define USB_HOST                         0
	    #define AUTO_STORAGE_CHANGE              0
    #endif

    #define VERIFY_TEST_USB_HOST             0 //ray add
    #define USB_DEVICE                       0
    #if USB_DONGLE_SUPPORT
    #define USB_HOST                         0
    #define AUTO_STORAGE_CHANGE              0
     #endif
    //%%%%%%%%%%%%%%%%%%%%%%%%%Code Reduce option%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%5
 #if( MULTI_CHANNEL_SEL  & 0x01 )
    #define ISU_SUPPORT                      1
    #define IPU_SUPPORT                      1
 #elif( MULTI_CHANNEL_SEL  & 0x20 )
    #define ISU_SUPPORT                      0
    #define IPU_SUPPORT                      1
 #else
    #define ISU_SUPPORT                      0
    #define IPU_SUPPORT                      0
 #endif
    //%%%%%%%%%%%%%%%%%%%%%%%	File close method %%%%%%%%%%%%%%%%%%%%%%//
    #define TRIGGER_MODE_CLOSE_FILE_METHOD   CLOSE_FILE_BY_SLICE
    #define MANUAL_MODE_CLOSE_FILE_METHOD    CLOSE_FILE_BY_SLICE


    //%%%%%%%%%%%%%%%%%%%%%%%	DDR Control %%%%%%%%%%%%%%%%%%%%%%//
    #define DDR_SLEWRATE_CONTROL_FOR_CLOCK             1       //0:fast output slew Rate, 1:slow output slew Rate

	//%%%%%%%%%%%%%%%%%%%%%%%	Remote File Playback %%%%%%%%%%%%%%%%%%%%%%//
	#define REMOTE_TALK_BACK    0


#endif  // _APPLICATION_H
