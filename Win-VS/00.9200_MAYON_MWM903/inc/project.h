#ifndef _PROJECT_H
#define _PROJECT_H

#if(HW_BOARD_OPTION  == MR9200_RX_MAYON_MWM903)
    #define PROJ_OPT                        3 /* 0:英意法 + 歐洲跳頻
                                                 1:英德法 + 歐洲跳頻
                                                 2:英德法 + 美國定頻
                                                 3:英日中 + 日本跳頻
                                                 */
    #define RFRX_HALF_MODE_SUPPORT          1
    #define RFRX_FULSCR_QVGA_QUAD           1
    #define RFRX_QUAD_AUDIO_EN              0
    #define RFRX_FULLSCR_HD_SINGLE          0
    #define RFTX_AUDIO_SUPPORT              1
    #define UI_CAMERA_UPGRADE               0
    #define SD_TASK_INSTALL_FLOW_SUPPORT	1
    #define SB_DECORD_SUPPORT               1   //Quad mode :大小碼流切換

	#define MAX_ADC_PGA_GAIN				0x0028 //Mic Fade-in/Fade-out.
    #define ADC_PGA_GAIN                    0x0028
    #define DAC_PGA_GAIN                    0x002E
    #define ADC_PGA_REDUCE_TALKBACK         0

    #define NOSIGNAL_MODE                   0
    #define INSERT_NOSIGNAL_FRAME           0
    #define SENSOR_FLICKER50_60_SEL         SENSOR_AE_FLICKER_60HZ
    
    //=== Turn On/Off CIU OSD===//
    #define CIU1_OSD_EN                     1
    #define CIU2_OSD_EN                     1
    #define CIU3_OSD_EN                     1
    #define CIU4_OSD_EN                     1
    #define CIU5_OSD_EN                     0

    #define CIU2_REPLACE_CIU1               0  // CIU1 會掉 interrupt
    #define MPEG4_FIELD_ENC_ENA             0  //Lucian: 僅能使用於8600-TX 系列, TV(CCIR656) input,不帶錄影儲存.
    #define USE_MPEG_QUANTIZATION           0

    #define MULTI_STREAM_START_ON           0      //開機是否進入multi-stream encoding
    #define TARGET_BR_SUBSTREAM_H           384    //Unit: Kbps
    #define TARGET_BR_SUBSTREAM_M           256    //Unit: Kbps
    #define TARGET_BR_SUBSTREAM_L           128     //Unit: Kbps
    #define TARGET_FR_SUBSTREAM             4

    //%%%%%%%%%%%%%%%%%%%%%%% Mpeg4 double filed %%%%%%%%%%%%%%%%%%%%%%//
    #define DE_INTERLACE_SEL        DOUBLE_FIELD_AUTO  //Lucian: Only valid on TV-in
    #define RFIU_IVOP_PERIOD        60
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Pannel select %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%/
    #define LCM_OPTION           LCM_P_RGB_PZX090IV042002

    #define UART_PTZ485_COMMAND_RX          0
    #define UART_PTZ485_COMMAND_TX          0
    #define UART_GPS_COMMAND                0
    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TV Decoder select %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define TV_DECODER                       TVDEC_NONE
    #define CIU_SPLITER                      0   // 4 CCIR MUX IN Enable  0:disable 1: enable

    #define TV_ENCODER                       TVENC_NONE
    #define TVOUT_CRYSTAL_24MHZ              1

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Audio formate select %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define Audio_formate_In                nomo_8bit_16k
    #define Audio_formate_Out               nomo_8bit_16k

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% RTC selection %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define USE_BUILD_IN_RTC               RTC_USE_EXT_RTC
    #define EXT_RTC_SEL                    RTC_SD2068

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
  #if( (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_RFU) )
    #define Sensor_OPTION                  Sensor_OV7725_YUV601
    #define CIU1_OPTION                      Sensor_OV7725_YUV601
    #define CIU2_OPTION                      Sensor_OV7725_YUV601
    #define CIU3_OPTION                      Sensor_OV7725_YUV601
    #define CIU4_OPTION                      Sensor_OV7725_YUV601
    #define CIU5_OPTION                      Sensor_OV7725_YUV601
  #else
    #define Sensor_OPTION                    Sensor_NONE_FHD
    #define CIU1_OPTION                      Sensor_NONE_FHD
    #define CIU2_OPTION                      Sensor_NONE_FHD
    #define CIU3_OPTION                      Sensor_NONE_FHD
    #define CIU4_OPTION                      Sensor_NONE_FHD
    #define CIU5_OPTION                      Sensor_NONE_FHD
  #endif

  //----Deinterlace ON/OFF-----//
#if(CIU1_OPTION ==  Sensor_CCIR656)
    #define HW_DEINTERLACE_CIU1_ENA          0
#else
    #define HW_DEINTERLACE_CIU1_ENA          0
#endif

#if(CIU2_OPTION ==  Sensor_CCIR656)
    #define HW_DEINTERLACE_CIU2_ENA          0
#else
    #define HW_DEINTERLACE_CIU2_ENA          0
#endif

#if(CIU3_OPTION ==  Sensor_CCIR656)
    #define HW_DEINTERLACE_CIU3_ENA          0
#else
    #define HW_DEINTERLACE_CIU3_ENA          0
#endif

#if(CIU4_OPTION ==  Sensor_CCIR656)
    #define HW_DEINTERLACE_CIU4_ENA          0
#else
    #define HW_DEINTERLACE_CIU4_ENA          0
#endif

#if(CIU5_OPTION ==  Sensor_CCIR656)
    #define HW_DEINTERLACE_CIU5_ENA          0
#else
    #define HW_DEINTERLACE_CIU5_ENA          0
#endif

    #define SENSOR_ROW_COL_MIRROR            0

  	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% SD card select %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%//
  	#define SDC_FS_SIN_TO_MUL_WRITE 0

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
    #define TOUCH_PANEL                      TOUCH_PANEL_DRIVER_GT9271
    #define SUPPORT_TOUCH                    1

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

    //%%%%%%%%%%%%%%%%%%%%%%%%%%% Volume control %%%%%%%%%%%%%%%%%%%%%%%%%%//
    /* Volume control Driver Type Select*/
    #define Volume_Control                 Volume_Control_NONE 
    
    #define LWIP2_SUPPORT                   1

   //%%%%%%%%%%%%%%%%%%%%%%%%%%%% GPIO Config  %%%%%%%%%%%%%%%%%%%%%%%%%%//
    #define GPIO_RST_SENSOR_GROUP           4
    #define GPIO_RST_SENSOR_GROUP2          4
    #define GPIO_RST_SENSOR_GROUP3          4
    #define GPIO_RST_SENSOR_GROUP4          4
    #define GPIO_RST_SENSOR                 44
    #define GPIO_RST_SENSOR2                44
    #define GPIO_RST_SENSOR3                44
    #define GPIO_RST_SENSOR4                44

    #define GPIO_ENA_SENSOR_GROUP           4
    #define GPIO_ENA_SENSOR_GROUP2          4
    #define GPIO_ENA_SENSOR_GROUP3          4
    #define GPIO_ENA_SENSOR_GROUP4          4
    #define GPIO_ENA_SENSOR                 44
    #define GPIO_ENA_SENSOR2                44
    #define GPIO_ENA_SENSOR3                44
    #define GPIO_ENA_SENSOR4                44
    
    #define SDC_POWER_ON_RESET_ENA          1   //SDC power-off/on reset
    #define GPIO_POWEROFFBIT_SDC            10
    #define GPIO_POWEROFFGRP_SDC            2
    #define GPIO_POWEROFF_ACT               0   // 1: Active High, 0: Active Low
    #define GPIO_SD_WRITE_PROTECT_ENA       0   // 1: Enable SD write protect, 0: GPIO
    
    #define GPIO_CHECKBIT_PWRSW             444

    #define GPIO_DV1_RESET_GROUP          4
    #define GPIO_DV1_RESET                30
    #define GPIO_DV2_VIDEO_RESET_GROUP      4
    #define GPIO_DV2_VIDEO_RESET            0
    
     //Tounch Pannel IC interrupt
    #define GPIO_GROUP_TOUCH_PANNEL_RST     2
    #define GPIO_BIT_TOUCH_PANNEL_RST       2
    #define GPIO_GROUP_TOUCH_PANNEL_INT     1
    #define GPIO_BIT_TOUCH_PANNEL_INT       17

    //LCD back light  
    #define GPIO_GROUP_BACK_LIGHT			2
    #define GPIO_BIT_BACK_LIGHT				9

    #define GPIO_GROUP_LCD_EN               2
    #define GPIO_BIT_LCD_EN                 8

    //GPIO I2C
    #define GPIO_GROUP_I2C_SCK              1
    #define GPIO_GROUP_I2C_SDA              1

    #define GPIO_BIT_I2C_SCK                0
    #define GPIO_BIT_I2C_SDA                1

    //POWER
    #define GPIO_GROUP_POWER_OFF            0
    #define GPIO_BIT_POWER_OFF              0

    //Battery
    #define GPIO_GROUP_BATTERY_CHARGE       2
    #define GPIO_BIT_BATTERY_CHARGE         11
    
    //LED
    #define GPIO_GROUP_LED                  8
    #define GPIO_BIT_LED_6                  8
    #define GPIO_BIT_LED_7                  8
    #define GPIO_BIT_LED_8                  8
    #define GPIO_BIT_LED_9                  8

    #define GPIO_GROUP_TV_LED               1
    #define GPIO_BIT_TV_LED                 9

    #define GPIO_RF_TURBO_GROUP             2
    #define GPIO_RF_TURBO                   11
    
    //RFI
    #define GIIO_CHECKBIT_PAIR              2

    #define GPIO_GROUP_RFI1CONF_SCLK        3
    #define GPIO_GROUP_RFI1CONF_SDATA       3
    #define GPIO_GROUP_RFI1CONF_nSS         3

    #define GPIO_BIT_RFI1_SCLK              5
    #define GPIO_BIT_RFI1_SDATA             2
    #define GPIO_BIT_RFI1_nSS               6

    #define GPIO_GROUP_RFI2CONF_SCLK        0
    #define GPIO_GROUP_RFI2CONF_SDATA       0
    #define GPIO_GROUP_RFI2CONF_nSS         0

    #define GPIO_BIT_RFI2_SCLK             7
    #define GPIO_BIT_RFI2_SDATA            8
    #define GPIO_BIT_RFI2_nSS               9

    //RF PA pin control
    #define GPIO_GROUP_RFI1CONF_TXSW        3           // AMICCOM RF front end PA/LNA select pin
    #define GPIO_BIT_RFI1_TXSW              0

    #define GPIO_GROUP_RFI1CONF_RXSW       3            // AMICCOM RF front end PA/LNA select pin
    #define GPIO_BIT_RFI1_RXSW              1


    #define GPIO_GROUP_RFI2CONF_TXSW        3           // AMICCOM RF front end PA/LNA select pin
    #define GPIO_BIT_RFI2_TXSW              4

    #define GPIO_GROUP_RFI2CONF_RXSW        3           // AMICCOM RF front end PA/LNA select pin
    #define GPIO_BIT_RFI2_RXSW              3


    //NIC
    #define NET_STATUS_POLLING				 1
    #define INT_USE_GROUP_0                  0
    #define INT_USE_GROUP_1                  1
    #define GPIO1_ETH_INT                    19
    #define GPIO0_ETH_INT                    0
    #define NIC_INT_ROULE                    INT_USE_GROUP_1
    #define GPIO_GROUP_Dm9000B_RST           1
    #define GPIO_BIT_Dm9000B_RST             31


    //APP
    #define SET_NTPTIME_TO_RTC               1
    #define NIC_TIMEZONE_DISABLE             0
    #define REMOTE_FILE_PLAYBACK             1
    #define GATEWAY_BOX                      0
    #define APP_KEEP_ALIVE					 1
    #define AutoNTPupdate					 0

    //GPIO  pin control
    #define GPIO_GROUP_TALK                 0
    #define GPIO_BIT_TALK                  2

    #define GPIO_GROUP_SPK_EN               1
    #define GPIO_BIT_SPK_EN                 19

    //SPI Flash
    #define GPIO_GROUP_SPI                  0
    #define GPIO_PIN_SPI_CLK                3
    #define GPIO_PIN_SPI_MOSI               4

    /*HW IR*/
    #define HW_IR_SUPPORT                   0
    #define IR_CUSTOM_CODE_ID               0

    /*UI */
    #define PLAY_MAIN_PAGE_SLIDE            1   // 1: enable UI play main page slide, 0: otherwise
    #define UI_BOOT_FROM_PANEL              1   /* 1: boot panel out, 0:boot TV out*/
    #define OSD_SIZE_X2_DISABLE              1
    #define UI_VERSION                       UI_VERSION_MAYON
    #define BOOT_REC_ENABLE                  0
    #define UI_GRAPH_QVGA_ENABLE             0  // 0: HD/VGA    1:VGA/QVGA
    #define UI_RX_PWRON_QUAD_ENA             1
    #define OSD_SIZE_D1_ENABLE               0
    #define FILE_SYSTEM_SEL                  FILE_SYSTEM_CDVR
    #define FILE_PLAYBACKFILE_SET            FILE_PLAYBACK_MENU
    #define TIMESTAMP_ENABLE                 0
    #define UI_SHOW_TIME_FORMAT              UI_SHOW_TIME_FORMAT_DMY
    #define UI_SUPPORT_HDD                   0
    #define UI_GRAPH_SIZE                    UI_GRAPH_SIZE_8MB
    #define UI_ICONFLAG_BACKUP               1
    #define UI_USE_DEMO_UI                   0
    #define UI_FCC_TEST                            0    /*FCC Test Mode */
    #define SD_AUTO_UPGRADE                  0
    #define UI_OTHER_LANGUAGE                1   /*0: two langugage, 1:three language, 2: five language */
    #define UI_LIGHT_SUPPORT                 1
    #define UI_LIGHT_TIME_MIN_FORMAT         UI_TIME_INTERVAL_30MIN
    #define UI_CAMERA_ALARM_SUPPORT          1
    #define DCF_RECORD_TYPE_API              1
    #define UI_BAT_SUPPORT                   1
    #define UI_CALENDAR_SUPPORT              1
    #define UI_FW_UPGRADE_ICON_ENABLE        1
    #define MASS_STORAGE_INSERT_SHOW         1 //enable two keys: UI_KEY_USB_INSERT and UI_KEY_USB_READY
    
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

    /*Network*/
    #define CHECK_FW_VER_BOOTING            1
    #define NET_FW_TEST                     0
    #define FW_SERVER                       "p2pupdate.mars-semi.com.tw"
    #if (PROJ_OPT == 0)
   	#define FW_PATH							"/Download/Mars/MAYON/MR9200_RX_MWM903/903-EIF-C/"
    #elif (PROJ_OPT == 1)
   	#define FW_PATH							"/Download/Mars/MAYON/MR9200_RX_MWM903/903-EDF-C/"
    #elif (PROJ_OPT == 2)
   	#define FW_PATH							"/Download/Mars/MAYON/MR9200_RX_MWM903/903-EDF-F/"
    #elif (PROJ_OPT == 3)
   	#define FW_PATH							"/Download/Mars/MAYON/MR9200_RX_MWM903/903-EJC-I/"
    #endif
    #if (NET_FW_TEST == 1)
    #define FW_PATH							"/Download/Mars/MAYON/MR9200_RX_MWM903/"
    #endif
	#define Model_name						"MR9200_RX_903"
    #define PORT_FW                         80
    #define TPNS_SERVER                     "p2pserver1.mars-semi.com.tw"
    #define TPNS_SERVER2                    "p2pserver2.mars-semi.com.tw"
    #define PORT_TPNS                       8888
    
    /* Thermometer Option */
    #define THERMOMETER_SEL                 THERMO_NONE

    /* SW IIC */
	#define GPIO_I2C_ENA                    0               // use SW IIC
    #define RF_PWR_TURBO_SUPPORT           0
    #define RF_CUSTOMERID_SET              RFI_CUSTOMER_ID_MAYON
    
    #define FORECE_MPEG_DROP_1_10_FPS    0    //Lucian: 用8600 TX D1 encoding.由於1016X peformace 不足. 30-->27, 25-->23 fps
	#define RF_TX_SMOOTH_PLUS              0    //Lucian: 讓TX 更smooth,會犧牲Quality
    #define RF_RX_SMOOTH_PLUS              1    //Lucian: 讓RX display smooth,會增加一點點Letency
    #define RX_DISPLAY_KEEP_BUFFERING      1    // Ted: 讓RX display更smooth, 會增加 1~2sec Letency
    #define RF_TX_LOWBITRATE_5FPS          0
    //-----RFI Protocol selection---------//
	#define RFIC_SEL                        RFIC_A7196_6M
    #define AMIC7196_USE_EXT_PA             0

    #if ((PROJ_OPT == 0) || (PROJ_OPT == 1))
    #define RFIU_PROTOCOL_SEL               RFI_PROTOCOL_CE_181
    #elif (PROJ_OPT == 2)
    #define RFIU_PROTOCOL_SEL               RFI_PROTOCOL_FCC_247
    #else
    #define RFIU_PROTOCOL_SEL               RFI_PROTOCOL_ISOWIFI
    #endif
   
   #if(RFIC_SEL == RFIC_A7130_4M)
    #define RFIU_RSSI_THR                   135
   #else
    #define RFIU_RSSI_THR                   110
   #endif
    #define RFIU_VOX_SUPPORT                0
    #define TX_VMD_ALGSEL                   TX_VMD_ORIG

    #define RFIU_FORCEUSE_ECC_RS12          1

    /* AUDIO cfg
     *  AUDIO_IIS_IIS - IIS COMPATIBLE.
     *  AUDIO_AC97_ALC203 - REALTEK AC'97 ACL203.Lucian: 不建議使用.
     *  AUDIO_IIS_WM8974 -Wolfson WM8974
     *  AUDIO_ADC_DAC     -HIMAX build_in ADC_DAC
     *  AUDIO_IIS_ALC5621 - REALTEK IIS ALC5621
     *  AUDIO_IIS_WM8940 -Wolfson WM8940          
     */
	#define IIS1_REPLACE_IIS5               0  // IIS1 out	
    #define AUDIO_OPTION		            AUDIO_IIS_DAC
    #define AUDIO_DEVICE		            AUDIO_IIS_WM8940
    #define AUDIO_BYPASS                    0               // 1: AUDIO_IIS_ALC5621 bypass MIC to HP
    #define AUDIO_IN_TO_OUT		            0               // 1: Playback the received audio data immediately, 0: Otherwise
	#define AUDIO_ALC_ENABLE	0
	
    /* AUDIO codec
     *  AUDIO_CODEC_PCM         - PCM format.
     *  AUDIO_CODEC_MS_ADPCM    - Microsoft ADPCM format
     *  AUDIO_CODEC_IMA_ADPCM   - IMA ADPCM format
     */
    #define AUDIO_CODEC                     AUDIO_CODEC_PCM
    #define ADC_SUBBOARD                    ADC_SUBBOARD_Veri

    //Select PIR recording mode
    #define BTC_PIRREC_MODESEL              BTC_PIRRECMODE_ARLO

    
#endif

#endif // _PROJECT_H

