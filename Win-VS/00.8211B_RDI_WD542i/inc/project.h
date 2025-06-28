#ifndef _PROJECT_H
#define _PROJECT_H


#if(HW_BOARD_OPTION  == MR8211B_TX_RDI_WD542I)
    #define UI_PROJ_OPT                     0 /*0: WD542i*/
    #define ADC_PGA_GAIN                    0x0a
    #define ADC_PGA_REDUCE_TALKBACK         24

	#define SYSTEM_DEBUG_SD_LOG				0
    
    #define USB2WIFI_SUPPORT                0
    #define Melody_SNC7232_ENA              0
    #define INSERT_NOSIGNAL_FRAME           0
    #define SENSOR_FLICKER50_60_SEL         SENSOR_AE_FLICKER_60HZ
        //=== Turn On/Off CIU OSD===//
    #define CIU1_OSD_EN                     1
    #define CIU2_OSD_EN                     1
    #define CIU3_OSD_EN                     1
    #define CIU4_OSD_EN                     1
    
    #define MPEG4_FIELD_ENC_ENA             0  //Lucian: 僅能使用於8600-TX 系列, TV(CCIR656) input,不帶錄影儲存.
    #define USE_MPEG_QUANTIZATION           0
        //%%%%%%%%%%%%%%%%%%%%%%% Mpeg4 double filed %%%%%%%%%%%%%%%%%%%%%%//
    #define DE_INTERLACE_SEL                DOUBLE_FIELD_AUTO  //Lucian: Only valid on TV-in
    #define RFIU_IVOP_PERIOD                40
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Pannel select %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%/
    #define LCM_OPTION                      LCM_HX8224
    
    #define UART_PTZ485_COMMAND_RX          0
    #define UART_PTZ485_COMMAND_TX          0
    #define UART_GPS_COMMAND                0
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TV Decoder select %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define TV_DECODER                       TVDEC_NONE
    #define CIU_SPLITER                      0   // 4 CCIR MUX IN Enable  0:disable 1: enable
    
    #define TV_ENCODER                       TVENC_NONE
    #define TVOUT_CRYSTAL_24MHZ              1
    
        //%%%%%%%%%%%%%%%%%%%%%%%%% Home Automation protocol  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%/
    #define HOME_RF_SUPPORT                  0
    #define HOME_RF_OPTION                   HOME_SENSOR_NONE
        
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%TV-out Config %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
        /* TV-OUT Video Clip Resolution
        *  TV_QVGA     -  320x240
        *  TV_VGA      -  640x480
        *  TV_D1       -  704x480 (Lucian: Only valid in TV-in, Sensor in not support! 會有頻寬不足現象,不建議使用)
        *  TV_HD720P   -  1280x720
        *  TV_FHD1080I -  1920x1080
        */
	#define TVOUT_RESOLUTION_PREVIEW_CAPTURE		    TV_VGA  //Lucian: Preview mode is full resolution. Only in video clip mode, half-resolution is valid,    
        
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Video Input format  and  Sensor select %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define Sensor_OPTION                    Sensor_HM1375_YUV601
    #define CIU1_OPTION                      Sensor_HM1375_YUV601
    #define CIU2_OPTION                      Sensor_HM1375_YUV601
    #define CIU3_OPTION                      Sensor_HM1375_YUV601
    #define CIU4_OPTION                      Sensor_HM1375_YUV601
    
    #define SENSOR_ROW_COL_MIRROR            0
  #if(Sensor_OPTION == Sensor_CCIR601)
    #define USE_PROGRESSIVE_SENSOR           0
    #define ISU_OUT_BY_VSYNC                 0
    
    #define SDV_SELECT                       SSDV_1
    #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.
    
    
    #define VIDEO_RESOLUTION_SEL  VIDEO_VGA_IN_VGA_OUT
    
    #if(TV_DECODER == MI9V136)
      #define SENSOR_DATA_SHIFT_OPTION    SENSOR_DATA_NO_SHIFT  // bit[7:0]
    #else
      #define SENSOR_DATA_SHIFT_OPTION    SENSOR_DATA_SHIFT_2bit  // bit[7:0]
    #endif
    
  #elif(Sensor_OPTION == Sensor_CCIR656)
    #define USE_PROGRESSIVE_SENSOR           0
    #define ISU_OUT_BY_VSYNC                 0
    #define ISU_OUT_BY_FID                   1   //@9003, 外部FID intr,  由siu intr (Frame start)取代.
    
    #define SDV_SELECT                       SSDV_1
    
    #define VIDEO_RESOLUTION_SEL  VIDEO_VGA_IN_VGA_OUT
    
    #define SENSOR_DATA_SHIFT_OPTION    SENSOR_DATA_SHIFT_2bit  // bit[7:0]
    
  #elif(Sensor_OPTION == Sensor_MI_5M)
    #define USE_PROGRESSIVE_SENSOR           1
    #define ISU_OUT_BY_VSYNC                 0
    #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.
    
    #define SDV_SELECT                       SSDV_2
    
    #define VIDEO_RESOLUTION_SEL             VIDEO_VGA_IN_VGA_OUT
    
    #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_NO_SHIFT
    
  #elif(Sensor_OPTION == Sensor_MI1320_YUV601)
    #define USE_PROGRESSIVE_SENSOR           1
    #define ISU_OUT_BY_VSYNC                 0
    #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.
    
    #define SDV_SELECT                       SSDV_1
    
    #define VIDEO_RESOLUTION_SEL             VIDEO_VGA_IN_VGA_OUT
    
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
    
  #elif(Sensor_OPTION == Sensor_PO3100K_YUV601)
    #define USE_PROGRESSIVE_SENSOR           1
    #define ISU_OUT_BY_VSYNC                 0
    #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.

    #define SDV_SELECT                       SSDV_1

    #define VIDEO_RESOLUTION_SEL             VIDEO_HD_IN_HD_OUT

    #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit

  #elif(Sensor_OPTION == Sensor_OV7740_YUV601)
    #define USE_PROGRESSIVE_SENSOR           1
    #define ISU_OUT_BY_VSYNC                 0
    #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.
    
    #define SDV_SELECT                       SSDV_1
    
    #define VIDEO_RESOLUTION_SEL             VIDEO_VGA_IN_VGA_OUT
    
    #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_SHIFT_2bit
  #else
    #define USE_PROGRESSIVE_SENSOR           1
    #define ISU_OUT_BY_VSYNC                 0
    #define ISU_OUT_BY_FID                   0    //Lucian: 改善CCIR601 input,Video 不平順的問題.
    
    #define SDV_SELECT                       SSDV_1
    
    #define VIDEO_RESOLUTION_SEL             VIDEO_VGA_IN_VGA_OUT
    
    #define SENSOR_DATA_SHIFT_OPTION         SENSOR_DATA_NO_SHIFT
    
  #endif
    
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% SPI flash select %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
        /* FLASH OPTION
        *   FLASH_SERIAL_ESMT           - ESMT serial flash
        *   FLASH_SERIAL_WINBOND        - Winbond serial flash
        *   FLASH_SERIAL_EON            - EON serial flahs
        *   FLASH_SERIAL_SST            - SST serial flash
        *   FLASH_NAND_9001_NORMAL      - NAND Normal TYPE(512 Bytes) with PA9001
        *   FLASH_NAND_9002_NORMAL      - NAND Normal TYPE(512 Bytes) with PA9002
        *   FLASH_NAND_9002_ADV         - NAND Advanced TYPE(2048 Bytes) with PA9002
        *   FLASH_NO_DEVICE             - No any storage
        */
    #define FLASH_OPTION    FLASH_SERIAL_EON
    
    
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%Flash Light select %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
        /*----Flash Light Type Select
         *FLASH_LIGHT_NONE -
         *FLASH_LIGHT_PWM  -
         *FLASH_LIGHT_SRC  -
         */
    #define FLASH_LIGHT_SEL   FLASH_LIGHT_NONE
    
    
        //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%HDMI HW select %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
        /* HDMI selection
        * HDMI_TX_NONE
        * HDMI_TX_EP952
        * HDMI_TX_CAT6613
        */
    #define HDMI_TXIC_SEL   HDMI_TX_NONE
    
    
        //%%%%%%%%%%%%%%%%%%%%%%%%%%% G-Sensor %%%%%%%%%%%%%%%%%%%%%%%%%%//
        /* G_SENSOR : G Sensor Option
        G_SENSOR_NONE       0   // No G sensor
        G_SENSOR_LIS302DL   1   // ST LIS302DL
        G_SENSOR_H30CD      2   // Hitachi H30CD
        */
    #define G_SENSOR                        G_SENSOR_NONE   // 0: none, 1: ST LIS302DL
    #define G_SENSOR_DETECT                 0   // 1: G-sensor detect enable, 0: otherwise
    
    
        //%%%%%%%%%%%%%%%%%%%%%%%%%%% Touch Pannel %%%%%%%%%%%%%%%%%%%%%%%%%%//
        /* Touch Panel Driver Type Select
            *  TOUCH_PANEL_DRIVER_NONE          -  Not support touch panel
            *  TOUCH_PANEL_DRIVER_TSC2046       -  TSC 2046
            *  TOUCH_PANEL_DRIVER_TSC2003       -  TSC 2003
            */
    #define TOUCH_PANEL                      TOUCH_PANEL_DRIVER_NONE
    #define SUPPORT_TOUCH                    0
    
        //%%%%%%%%%%%%%%%%%%%%%%%%%%% Touch Key %%%%%%%%%%%%%%%%%%%%%%%%%%//
        /* Touch Key Driver Type Select
            *  TOUCH_KEY_NONE          -  Not support touch key
            *  TOUCH_KEY_BS83B12       -  BS83B12
            *  TOUCH_KEY_MA86P03       -  MA86P03
            */
    #define TOUCH_KEY                      TOUCH_KEY_NONE
    #define SUPPORT_TOUCH_KEY              0
    
        //%%%%%%%%%%%%%%%%%%%%%%%%%%% IO Expand %%%%%%%%%%%%%%%%%%%%%%%%%%//
        /* IO Expand Driver Type Select
            *  IO EXPAND_NONE          -  Not support io expand
            *  IO EXPAND_WT6853        -  WT6853
            */
    #define IO_EXPAND                      IO_EXPAND_NONE
    #define SUPPORT_IO_EXPAND              0
    
    
    
       //%%%%%%%%%%%%%%%%%%%%%%%%%%%% GPIO Config  %%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define GPIO_RST_SENSOR_GROUP           0
    #define GPIO_RST_SENSOR_GROUP2          0
    #define GPIO_RST_SENSOR_GROUP3          0
    #define GPIO_RST_SENSOR_GROUP4          0
    #define GPIO_RST_SENSOR                 31
    #define GPIO_RST_SENSOR2                31
    #define GPIO_RST_SENSOR3                31
    #define GPIO_RST_SENSOR4                31
    
    #define GPIO_ENA_SENSOR_GROUP           8
    #define GPIO_ENA_SENSOR_GROUP2          8
    #define GPIO_ENA_SENSOR_GROUP3          8
    #define GPIO_ENA_SENSOR_GROUP4          8
    #define GPIO_ENA_SENSOR                 0
    #define GPIO_ENA_SENSOR2                11
    #define GPIO_ENA_SENSOR3                2
    #define GPIO_ENA_SENSOR4                3
    
    #define SDC_POWER_ON_RESET_ENA          0   //SDC power-off/on reset
    #define GPIO_POWEROFFBIT_SDC            8
    #define GPIO_POWEROFFGRP_SDC            8
    #define GPIO_POWEROFF_ACT               1   // 1: Active High, 0: Active Low
    #define GPIO_SD_WRITE_PROTECT_ENA       0   // 1: Enable SD write protect, 0: GPIO
    
    
    #define GPIO_GROUP_TALK                 5
    #define GPIO_BIT_TALK                   33
    
    #define GPIO_CHECKBIT_PWRSW             134
    
        //PIR sensor//
    #define GPIO_GROUP_PIR                  8
    #define GPIO_BIT_PIR                    8
    #define PIR_TRIGER_ACT_HIGH             1   

        //GPIO POP noise
    #define GPIO_GROUP_POP_EN               8
    #define GPIO_BIT_POP_EN                 17
    
        //GPIO SPK
    #define GPIO_GROUP_SPK_EN               1
    #define GPIO_BIT_SPK_EN                 6
    
        //GPIO I2C
    #define GPIO_GROUP_I2C_SCK              8
    #define GPIO_GROUP_I2C_SDA             8
    
    #define GPIO_BIT_I2C_SCK                27
    #define GPIO_BIT_I2C_SDA                28
    
        //LED
    #define GPIO_GROUP_LIGHT_SW             1
    #define GPIO_BIT_LIGHT_SW               11

    #define GPIO_GROUP_LIGHT_LED            8
    #define GPIO_BIT_LIGHT_LED              13

    #define GPIO_GROUP_LED_RF               8
    #define GPIO_BIT_LED_RF                 8
    #if 0
    #define GPIO_GROUP_LED                  4
    #define GPIO_BIT_LED_6                  6
    #define GPIO_BIT_LED_7                  1
    #define GPIO_BIT_LED_8                  8
    #define GPIO_BIT_LED_9                  1
    #endif

    #define GPIO_GROUP_MODE_SEL             8
    #define GPIO_BIT_MODE_SEL               8
        //Day_Night
    
    #define GPIO_GROUP_IR_POWER             3
    #define GPIO_PIN_IR_POWER               28

    //Sensor
    #define GPIO_GROUP_SENSOR_DAYNIGHT      8
    #define GPIO_BIT_SENSOR_DAYNIGHT        3
    
    #define GPIO_POWER_KEY                  12
    #define GPIO_POWER_KEY_GROUP            8
    #define GPIO_POWER_KEEP                 1
    #define GPIO_POWER_KEEP_GROUP           8

    // Motor control
    #define MOTOR_EN                        0
    #define GPIO_MOTOR_GROUP_UP             8   // UP
    #define GPIO_MOTOR_GROUP_DOWN           8   // DOWN
    #define GPIO_MOTOR_GROUP_Left           8   // LEFT
    #define GPIO_MOTOR_GROUP_RIGHT          8   // RIGHT
    #define GPIO_MOTOR_BIT_UP               7   // UP
    #define GPIO_MOTOR_BIT_DOWN             8   // DOWN
    #define GPIO_MOTOR_BIT_Left             9   // LEFT
    #define GPIO_MOTOR_BIT_RIGHT            10  // RIGHT
    
    #define GPIO_GROUP_IR_DETECTION         8
    #define GPIO_PIN_IR_DETECTION           9
    
    #define GPIO_GROUP_ICR_ON               8
    #define GPIO_PIN_ICR_ON                 27
    
    #define GPIO_GROUP_ICR_OFF              8
    #define GPIO_PIN_ICR_OFF                28
    
    //RFI
    #define GPIO_GROUP_CHECKBIT_PAIR        0
    #define GPIO_CHECKBIT_PAIR              2
    
    #define GPIO_GROUP_RFI1CONF_SCLK        1
    #define GPIO_GROUP_RFI1CONF_SDATA       1
    #define GPIO_GROUP_RFI1CONF_nSS         1
    
    #define GPIO_BIT_RFI1_SCLK              13
    #define GPIO_BIT_RFI1_SDATA             14
    #define GPIO_BIT_RFI1_nSS               12
    
    #define GPIO_GROUP_RFI2CONF_SCLK        8
    #define GPIO_GROUP_RFI2CONF_SDATA       8
    #define GPIO_GROUP_RFI2CONF_nSS         8
    
    #define GPIO_BIT_RFI2_SCLK              2
    #define GPIO_BIT_RFI2_SDATA             1
    #define GPIO_BIT_RFI2_nSS               17
    
        //RF PA pin control

    #define GPIO_GROUP_RFI1CONF_TXSW        3          // AMICCOM RF front end PA/LNA select pin
    #define GPIO_BIT_RFI1_TXSW              27 
    
    #define GPIO_GROUP_RFI1CONF_RXSW        1           // AMICCOM RF front end PA/LNA select pin
    #define GPIO_BIT_RFI1_RXSW              2 
    
    #define GPIO_GROUP_RFI2CONF_TXSW        8          // AMICCOM RF front end PA/LNA select pin
    #define GPIO_BIT_RFI2_TXSW              18
    
    #define GPIO_GROUP_RFI2CONF_RXSW        8           // AMICCOM RF front end PA/LNA select pin
    #define GPIO_BIT_RFI2_RXSW              10

    
        //NIC
    //#define GPIO_ETH_INT                    29
    #define GPIO1_ETH_INT                    19
    #define GPIO0_ETH_INT                    0
    #define INT_USE_GROUP_0                  4
    #define INT_USE_GROUP_1                  4
    #define NIC_INT_ROULE                    INT_USE_GROUP_1
    #define GPIO_GROUP_Dm9000B_RST           8
    #define GPIO_BIT_Dm9000B_RST             10
    
        //APP
    #define SET_NTPTIME_TO_RTC               0
    #define NIC_TIMEZONE_DISABLE             1
    #define REMOTE_FILE_PLAYBACK             0
    #define GATEWAY_BOX                      0
	#define APP_KEEP_ALIVE					 0
        
    
    #define GPIO_DV1_RESET_GROUP          4
    #define GPIO_DV1_RESET                30
    #define GPIO_DV2_VIDEO_RESET_GROUP      4
    #define GPIO_DV2_VIDEO_RESET            0
    
    
        //SPI Flash
    #define GPIO_GROUP_SPI                  0
    #define GPIO_PIN_SPI_CLK                3
    #define GPIO_PIN_SPI_MOSI               4

    //GPIO Music IC 
    #define GPIO_MUSIC_GROUP0               0
    #define GPIO_MUSIC_GROUP                1
    #define GPIO_MUSIC_BIT0                 1	// 0-1
    #define GPIO_MUSIC_BIT1                 8	// 1-8
    #define GPIO_MUSIC_BIT2                 9	// 1-9
    
        /*HW IR*/
    #define HW_IR_SUPPORT                   0
    #define IR_CUSTOM_CODE_ID               0
    
    
        /*UI */
    #define SENSOR_PARAMETER                 0   /* 0: 7吋model 1:5吋baby monitor */ 
    #define PLAY_MAIN_PAGE_SLIDE             0   // 1: enable UI play main page slide, 0: otherwise
    #define UI_BOOT_FROM_PANEL               0   /* 1: boot panel out, 0:boot TV out*/
    #define OSD_SIZE_X2_DISABLE              0
    #define UI_VERSION                       UI_VERSION_ST_1
    #define BOOT_REC_ENABLE                  1

    
    #define UI_GRAPH_QVGA_ENABLE             0  // 0: HD/VGA    1:VGA/QVGA
    #define UI_RX_PWRON_QUAD_ENA             0
    #define OSD_SIZE_D1_ENABLE               0
    #define FILE_SYSTEM_SEL                  FILE_SYSTEM_DVR
    #define FILE_PLAYBACKFILE_SET            FILE_PLAYBACK_MENU
    #define TIMESTAMP_ENABLE                 0
    #define UI_GRAPH_SIZE                    UI_GRAPH_SIZE_NONE
    #define SPK_CONTROL                      0
    
        /*TV */
    #define TV_YUV_GAIN                    0x00808080
    #define TV_YUV_GAIN_PAL                0x00808080
    #define CIU_Y_IDX                      0x00ffffff
    #define CIU_CB_IDX                     0x00808080
    #define CIU_CR_IDX                     0x00808080
    
        /* Sensor */
    #define SENSOR_U_SATURATION_INDOOR      0x30
    #define SENSOR_V_SATURATION_INDOOR      0x30
    #define SENSOR_U_SATURATION_OUTDOOR     0x40
    #define SENSOR_V_SATURATION_OUTDOOR     0x40
    
        /* SDCD status*/
    #define SDC_CD_IN                       0
    #define SDC_CD_OFF                      1
    
        /* Thermometer Option */
    #define THERMOMETER_SEL                 THERMO_NONE
    
        /* SW IIC */
	#define GPIO_I2C_ENA                    0               // use SW IIC
    #define RF_PWR_TURBO_SUPPORT           0
    #define RF_CUSTOMERID_SET              RFI_CUSTOMER_ID_SWANN
        
        
    #define FORECE_MPEG_DROP_1_10_FPS    0    //Lucian: 用8600 TX D1 encoding.由於1016X peformace 不足. 30-->27, 25-->23 fps
	#define RF_TX_SMOOTH_PLUS              0    //Lucian: 讓TX 更smooth,會犧牲Quality
    #define RF_RX_SMOOTH_PLUS              1    //Lucian: 讓RX display smooth,會增加一點點Letency
    #define RF_TX_LOWBITRATE_5FPS          0
        //-----RFI Protocol selection---------//
    #define RFIU_PROTOCOL_SEL               RFI_PROTOCOL_CE_181
    #define RFIU_RSSI_THR                   135        
    
    #define RFIU_VOX_SUPPORT                0
    #define TX_VMD_ALGSEL                   TX_VMD_NEW  


    #define CiuLossSemEvtBUG                1  //開機mpeg4 收不到 SemEvt 關閉ciu 

    
	//NET
	#define TX_PUSH_APPMSG                  1
    #define CLOUD_SUPPORT                   0
    #define CHECK_FW_VER_BOOTING            1
    #define FW_SERVER                       "p2pupdate.mars-semi.com.tw"
    #define FW_PATH                         "/Download/Mars/RDI/Test/"
    #define PORT_FW                         80
    #define TPNS_SERVER                     "tpns1.mars-semi.com.tw"
    #define TPNS_SERVER2                    "tpns2.mars-semi.com.tw"
	#define CLOUD_PUSH_SERVER				"pushmsg.mars-cloud.com"
	#define PORT_CLOUD_PUSH 				80
    #define PORT_TPNS                       8888
    #define Model_name                      ""
    #define CLOUD_SERVER                    "192.168.1.55" 
    #define PORT_CLOUD                      20001
#endif
#endif // _PROJECT_H

