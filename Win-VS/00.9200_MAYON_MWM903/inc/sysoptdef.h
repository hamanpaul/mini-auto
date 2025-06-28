/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    sysoptdef.h

Abstract:

    The system option definition.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#ifndef __SYS_OPT_DEF_H__
#define __SYS_OPT_DEF_H__


/* Hardware board option*/

//-------------A1013----------//
#define A1013_FPGA_BOARD        100
#define A1013_REALCHIP_A        101

//-------------A1016----------//
#define A1016_FPGA_BOARD        200
#define A1016_REALCHIP_A        201  //A1016 EVB and A1016 FPGA 共用
#define A1016B_FPGA_BOARD		202

//-------------A1018A----------//
#define A1018_FPGA_BOARD        501
#define A1018_EVB_128M          502   //A1018 EVB and A1018 FPGA 共用
#define A1018_EVB_256M_TW2866   503   //A1018 EVB and A1018 FPGA 共用+TW8866
#define A1018_EVB_256M_HM1375   504   //A1018 EVB and A1018 FPGA 共用+TW8866

//-------------A1018B----------//
#define A1018B_FPGA_BOARD       505
#define A1018B_EVB_128M         506
#define A1018B_EVB_216M         507
#define A1018B_SKB_128M_TX      508
#define A1018B_SKB_128M_RX      509   //與OPCOM USB dongle 共用
#define A1018B_SKB_128M_BT_RX   510   //Barvotech Usb dongle

#define MR9300_216M_EVB         511


#define MR6790_DB               520

#define MR9211_DB               530

//-------------A1019A----------//
//--Wireless Cam--//
#define A1019A_FPGA_BOARD       550
#define A1019A_SKB_128M_TX      551 // OPCOM FHD USB TX
#define A1019A_SKB_128M_RX      552
#define A1019A_EVB_128M_TX      553
#define A1019A_TX_MA8806        554
#define A1019A_TX_RDI_CA831     555
#define MR9100_TX_RDI_CA811     556
#define MR9100_TX_SKY_AHD       557
#define MR9100_TX_SKY_W_AHD     558
#define MR9100_TX_SKY_USB       559
#define MR9100_TX_DB_AHDIN      560
#define MR9100_TX_TRANWO_D87T   561
#define MR9100_TX_RDI_CA840     562
#define MR9100_TX_OPCOM_CVI     563 // 外掛audio
#define MR9100_TX_OPCOM_HD_USB  564
#define MR9100_TX_RDI_USB       565 // CA826
#define MR9100_TX_OPCOM_CVI_SK  566
#define MR9100_TX_MUXCOM_AHDIN  567
#define MR9100_TX_JIT_C707HW4   568
#define MR9100_TX_MAYON_MWL612  569

//--Battery Cam--//
#define MR9160_TX_DB_BATCAM     580
#define MR9160_TX_OPCOM_BATCAM  581
#define MR9160_TX_MAYON_MWL613  582
#define MR9160_TX_ROULE_BATCAM   583

//----//
#define MR9100_AHDINREC_MUXCOM  590
#define MR9211_TX_MA8806        591  //with icomm wifi dual mode TX

//--------------  TX (A1018) ---------------//
#define MR9120_TX_DB            600
#define MR9120_TX_OPCOM         601
#define MR9120_TX_MA8806        602
#define MR9120_TX_RDI           603
#define MR9120_TX_TRANWO        604
#define MR9120_TX_RDI_CA831     605
#define MR9120_TX_OPCOM_CVI     606
#define MR9120_TX_TRANWO_D87T   607
#define MR9100_TX_RDI           608
#define MR9120_TX_RDI_USB       609
#define MR9120_TX_IQ            610
#define MR9120_TX_RDI_CA840     611
#define MR9120_TX_OPCOM_USB_6M  612
#define MR9120_TX_SKY_USB       613
#define MR9120_TX_SKY_AHD       614
#define MR9120_TX_BT_USB        615



//---------------RX-------------------//
#define MR9600_RX_DB            700
#define MR9600_RX_OPCOM_CVI     701
#define MR9600_RX_RDI_AHD       702
#define MR9600_RX_DB_ETH        703
#define MR9600_RX_SKY_AHD       704
#define MR9120_RX_DB_AHD        705
#define MR9120_RX_DB_HDMI       706
#define MR9120_RX_MUXCOM_AHD    707





#define MR9200_RX_DB                800
#define MR9200_RX_RDI               801		    // For MR9200A-256M Chip
#define MR9200_RX_GTW		        802
#define MR9200_RX_TRANWO		    803
#define MR9200_RX_RDI_LOREX		    804   // For MR9200B-216M Chip
#define MR9200_RX_TRANWO_D8795	    805
#define MR9200_RX_RDI_UNIDEN	    806
#define MR9200_RX_IQ_GTW    	    807
#define MR9200_RX_RDI_UDR777	    808
#define MR9200_RX_TRANWO_D8795R2    809
#define MR9200_RX_TRANWO_D8797R     810
#define MR9200_RX_TRANWO_D8710R     811         // 9200 Tranwo Box
#define MR9200_RX_JIT_M916HN4       812         
#define MR9200_RX_RDI_M906	        813
#define MR9200_RX_MAYON_MWM903	    814
#define MR9200_RX_RDI_M1000	        815
#define MR9200_RX_TRANWO_D8796P     816
#define MR9200_RX_ROULE             817
#define MR9200_RX_TRANWO_D8897H     818
#define MR9200_RX_TRANWO_SH8710R    819
#define MR9200_RX_MAYON_MWM018      820


#define MR9300_RX_DB                900
#define MR9300_RX_RDI               901


//-------------A1022A----------//
#define A1022A_FPGA_BOARD      1000

//-------------A1021A----------//
#define A1021A_FPGA_BOARD      1100

//-------------A1025A----------//
#define A1025A_FPGA_BOARD      1200
#define A1025A_EVB				1201
#define MR8202A_RX_TARNWO_D8530				1202
#define MR8202A_RX_TARNWO_D8730				1203
#define A1025A_EVB_axviwe			1204
#define MR8202A_RX_MAYON			1205
// ALL
#define A1025_GATE_WAY_SERIES   ((HW_BOARD_OPTION == MR8202A_RX_TARNWO_D8530) ||\
	                                                    (HW_BOARD_OPTION  == MR8202A_RX_TARNWO_D8730) || \
	                                                    (HW_BOARD_OPTION  == MR8202A_RX_MAYON)||\
	                                                    (HW_BOARD_OPTION == A1025A_EVB_axviwe))

#define FPGA_BOARD_A1018_SERIES	((HW_BOARD_OPTION  == A1013_FPGA_BOARD) || \
								(HW_BOARD_OPTION  == A1016_FPGA_BOARD) || \
								(HW_BOARD_OPTION  == A1016B_FPGA_BOARD) || \
								(HW_BOARD_OPTION  == A1018_FPGA_BOARD) || \
								(HW_BOARD_OPTION  == A1018B_FPGA_BOARD) || \
								(HW_BOARD_OPTION  == A1019A_FPGA_BOARD) || \
								(HW_BOARD_OPTION  == A1022A_FPGA_BOARD) || \
								(HW_BOARD_OPTION  == A1021A_FPGA_BOARD) || \
								(HW_BOARD_OPTION  == A1025A_FPGA_BOARD))

//-----------------------RF Customer ID-------------------//
#define RFI_CUSTOMER_ID_ALLPASS             0

#define RFI_CUSTOMER_ID_LOREX               1000       //RDI series

#define RFI_CUSTOMER_ID_SWANN               2000       //TRAWO series

#define RFI_CUSTOMER_ID_JIT                 3000
#define RFI_CUSTOMER_ID_MAYON               4000
#define RFI_CUSTOMER_ID_MAYON_8600          4001
#define RFI_CUSTOMER_ID_COMMAX              5000
#define RFI_CUSTOMER_ID_JESMAY              6000
#define RFI_CUSTOMER_ID_HECHI               7000
#define RFI_CUSTOMER_ID_GCT                 8000
#define RFI_CUSTOMER_ID_SKYSUCCESS          9000
#define RFI_CUSTOMER_ID_ROULE              10000

//--------------RF protocol selection-------------------//
#define RFI_PROTOCOL_ISOWIFI  0   //跳頻, 可以最大避免干擾 Wifi. 於RX 偵測RSSI
#define RFI_PROTOCOL_CE_181   1   //跳頻, 符合CE191(LBT) 法規.  於TX偵測RSSI
#define RFI_PROTOCOL_FCC_247  2   //定頻, 於RX 偵測RSSI
#define RFI_PROTOCOL_ORIG     3   //跳頻, 不參考RSSI,使用greedy rule. 適合展場使用

//----------RF TX VMD selection--------//
#define TX_VMD_ORIG          0
#define TX_VMD_NEW           1

//----------LINK TX type selection--------//
#define RFCAM_TX_8200         0
#define RFCAM_TX_9200         1

/* Sensor shift option
 *  SENSOR_DATA_NO_SHIFT - Sensor data is shifted 0 bits.
 *  SENSOR_DATA_SHIFT_2bit - Sensor data is shifted 2 bits.
 *  SENSOR_DATA_SHIFT_4bit - Sensor data is shifted 4 bits.
 */
#define SENSOR_DATA_NO_SHIFT        0
#define SENSOR_DATA_SHIFT_2bit          1
#define SENSOR_DATA_SHIFT_4bit          2

/* MPEG4 file container option
 *  1 - MP4 file.
 *  2 - ASF file.
 *  4 - AVI file.
 *  8 - MOV file.
 */
#define MPEG4_CONTAINER_MP4     0x1
#define MPEG4_CONTAINER_ASF     0x2
#define MPEG4_CONTAINER_AVI     0x4
#define MPEG4_CONTAINER_MOV     0x8

/* Storage memory option
 *  0 - Ram disk.
 *  1 - SD / MMC.
 *  2 - SMC / Nand gate flash.
 */
#define STORAGE_MEMORY_RAMDISK          0
#define STORAGE_MEMORY_SD_MMC           1
#define STORAGE_MEMORY_SMC_NAND         2
#define STORAGE_MEMORY_USB_HOST         3
#define STORAGE_MEMORY_USB_HOST_2       4
#define STORAGE_MEMORY_MAX              5
/* Jpeg encode operation mode option
*   0 - Frame Mode
*   1 - Slice Mode
*/
#define JPEG_OPMODE_FRAME    0
#define JPEG_OPMODE_SLICE    1

/* Jpeg decode operation mode option
*   0 - Frame Mode
*   1 - Slice Mode
*/
#define JPEG_DECODE_MODE_FRAME  0
#define JPEG_DECODE_MODE_SLICE  1

/* Chip option
 *  0 - PA9001A.
 *  1 - PA9001C.
 */
/*Lucian: 請按照時間排序 IC 版本*/
#define CHIP_PA9001A            0
#define CHIP_PA9001C            1
#define CHIP_PA9002A            2

#define CHIP_PA9001D            3
#define CHIP_PA9002B            4
#define CHIP_PA9002C            5
#define CHIP_PA9002D            6

#define CHIP_A1013A             7   // 2010 Suttle
#define CHIP_A1016A             8   // 2012 MP

#define CHIP_A1016B             9	// 2013 Q1, No tape out
#define CHIP_A1018A             10  // 2013 Q4

#define CHIP_A1018B             11  // 2015 Q1
#define CHIP_A1013B             12  // No tape out

#define CHIP_A1019A             13
#define CHIP_A1020A             14

#define CHIP_A1022A             15
#define CHIP_A1021A             16

#define CHIP_A1025A				17

/*
*    Software Application
*/
//--------------------------------Wireless Application-----------------//
//==RF-Cam TX Module(QFP128)==//
#define MR8120_RFCAM_TX1            0     //請使用 1018CopyToUSB32.bat
#define MR8120_RFCAM_TX2            1     //請使用 1018CopyToUSB32.bat
#define MR8110_RFCAM_TX1            2     //請使用 1018CopyToUSB32.bat
#define MR9120_RFCAM_TX1_SUBSTREAM  3     //請使用 1018CopyToUSB32.bat
#define MR9120_RFCAM_TX1_MUTISTREAM 4     //請使用 1018CopyToUSB32.bat

#define MR9120_RFCAM_TX5            5     //請使用 1019CopyToUSB.bat
#define MR9120_RFCAM_TX5_SUBSTREAM  6     //請使用 1019CopyToUSB.bat
#define MR9120_RFCAM_TX5_MUTISTREAM 7     //請使用 1019CopyToUSB.bat

#define MR9120_RFCAM_TX5_MUTISTREAM_FWUPD   8  //請使用 1019CopyToUSB.bat. support TX fw update. A7196 DATA-6M, ACK-3M for TRAWO 9200, 9300
#define MR9120_BATCAM_TX5_MUTISTREAM_FWUPD  9  //請使用 1019CopyToUSB.bat. support TX fw update. A7196 DATA-6M, ACK-3M for Battery Camera use
#define MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD 10  //請使用 1019CopyToUSB.bat. support TX fw update. A7196 DATA-6M, ACK-3M for Doorbell Camera use

                                                //美安寶請用 1019CopyToUSB144M.bat
//==RF 1RX/2RX + P2P(QFP216)==//
#define MR8200_RFCAM_RX1            11    //請使用1018CopyToUSB64.bat
#define MR8200_RFCAM_RX1RX2         12    //請使用1018CopyToUSB64.bat;9200 RDI共用此設定; 

#define MR9200_HDMI_RX1RX2          13    //注意! 務必使用 1018CopyToUSB93.bat; 9200 with HDMI/USB for TRAWO;
#define MR9200_MIXCAM_RX1RX2        14    //注意! 務必使用 1018CopyToUSB93.bat; 兼容 Normal Cam/ Battery Cam/ Door bell  + HDMI output + USB HDD


#define MR9300_RFDVR_RX1RX2         15    //注意! 務必使用 1018CopyToUSB93.bat
#define MR9300_NETBOX_RX1RX2        16    //注意! 務必使用 1018CopyToUSB64.bat/BL18SD64.bin.  DVR box: no LCD display, Preview on APP only.

//==RF 2RX + Dual TV out(QFP128)==//
#define MR8120_RFAVSED_RX1            20   // 1T1R1TV
#define MR8600_RFAVSED_RX1RX2         21   // 2T2R2TV
#define MR8600_RFAVSED_RX1            22   // 2T1R2TV

#define MR9100_RF_CVI_AVSED_RX1       23   // CVI out,RX
#define MR9100_RF_AHD_AVSED_RX1       24   // AHD out,RX

#define MR9100_RF_AHDIN_AVSED_RX1     25   // AHD out,RX

#define MR9100_RF_DONGLE_AVSED_RX1RX2 26   // 4CH Usb dongle,RX,2.4G RF

#define MR9100_RF_HDMI_AVSED_RX1      27   // HDMI out,RX

#define MR9100_RF_DONGLE_AVSED_RX1RX2_8CH      28 // 8CH Usb dongle,RX,2.4G RF

#define MR9100_WIFI_DONGLE_AVSED_RX1  29  // Usb dongel,RX with ICOMM wifi RF.


//==RF 1RX + Pannel/TV(QFP216)==//
#define MR8120_RFCAM_RX1        30   // 1 RX module
#define MR8600_RFCAM_RX1RX2     31   // 2 RX module

//----------------------- Wire Applications------------------------//
//==Car DVR(QFP216)==//
#define MR6730_CARDVR_2CH       40
#define MR6730_CARDVR_1CH       41

//==Door Phone(QFP216)==//
#define MR9670_DOORPHONE        50

//== IP CAM ==//
#define MR8211_IPCAM            60
#define MR9211_DUALMODE_TX5     61   //dual mode IP CAM with Icomm wifi module. No battery 

//==Baby monitor ==//
#define MR8110_BABYMONITOR      70

//==AVIN DVR REC ==//
#define MR9100_AHDINREC_TX5     80

//==GATEWAY BOX ==//
#define MR8202_GATEWAYBOX_RX     90
#define MR8202_AN_KLF08W		91 //MR8202A_RX_MAYON,A1025A_EVB_axviwe

//--Only for Test,請自行定義--//
#define DVP_RF_SELFTEST             100    //請使用 1018CopyToUSB32.bat
#define Standalone_Test             101    //For A1025 test
#define FPGA_VERIFY_TEST_CIU        102    //test preview path, no playback.
#define FPGA_VERIFY_TEST_IDU        103
#define FPGA_VERIFY_TEST_SIU        104    // CIU5
#define FPGA_VERIFY_TEST_RFU        105
#define FPGA_VERIFY_TEST_H264	    106
#define FPGA_VERIFY_TEST_USB_HOST	107
#define FPGA_VERIFY_TEST_MAC	    108
#define FPGA_VERIFY_TEST_DUAL_CPU	109
#define FPGA_VERIFY_TEST_DIU        110

#define FPGA_VERIFY_TEST_RFTX1      111
#define FPGA_VERIFY_TEST_RFRX1      112
#define FPGA_VERIFY_TEST_PIP	    113
#define FPGA_VERIFY_TEST_GFU	    114
#define FPGA_VERIFY_TEST_H1_H264    115



/* LCM option
 *  0 - HX8312
 *  1 - HX8224.
 */
/*LCD Module*/
/*
   LCD: AV-Pannel,     S-RGB,P-RGB interface
   LCM: Mobile-Pannel, MPU/I80/M68 interface
*/
#define LCM_HX8312                 0x00  //(LCM)
#define LCM_HX8224                 0x01
#define LCM_HX5073_RGB             0x02
#define LCM_HX5073_YUV             0x03
#define LCM_COASIA                 0x04
#define LCM_TPG105                 0x05
#define LCM_HX8224_SRGB            0x06
#define LCM_A015AN04               0x07
#define LCM_TD020THEG1             0x08
#define LCM_GPG48238QS4            0x09
#define LCM_A024CN02               0x0A
#define LCM_HX8224_601             0x0B
#define LCM_HX8817_RGB             0x0C
#define LCM_CCIR601_640x480P       0x0D
#define LCM_TG200Q04               0x0E  //(LCM)
#define LCM_TMT035DNAFWU24_320x240 0x0F
#define LCM_HX8257_RGB666_480x272  0x10
#define LCM_TD036THEA3_320x240     0x11
#define LCM_TD024THEB2             0x12
#define LCM_TD024THEB2_SRGB        0x13
#define LCM_TJ015NC02AA            0x14
#define LCM_LQ035NC111             0x15
#define LCM_HX8224_656             0x16
#define LCM_P_RGB_888_HannStar     0x17
#define LCM_TM024HDH29             0x18
#define LCM_P_RGB_888_Innolux      0x19
#define LCM_HX8257_SRGB_480x272    0x1A
#define LCM_sRGB_HD15_HDMI         0x1B
#define LCM_HX8257_P_RGB_480x272   0x1C
#define LCM_P_RGB_888_AT070TN90    0x1D
#define LCM_P_RGB_888_SY700BE104   0x1E
#define LCM_P_RGB_888_ZSX900B50BL  0x1F
#define LCM_P_RGB_888_FC070227     0x20
#define LCM_P_RGB_888_ZSX70DT5011  0x21
#define LCM_P_RGB_PZX090IV042002   0x22
#define LCM_P_RGB_888_ILI6122      0x23
#define LCM_CCIR601_1280x720p30    0x24
#define LCM_NONE_HD                0x25 // Amon : Tvout use
#define LCM_NONE_FHD               0x26 // Amon : Tvout use


//Lucian: 由於VGA 需精確的clock,故clock source 都由 DPLL調整.
#define VGA_640X480_60HZ           0x50  // 25 MHz    600/24=25
#define VGA_800X600_60HZ           0x51  // 40 MHz    520/13=40
#define VGA_1024X768_60HZ          0x52  // 65 MHz:   520/8=65
#define VGA_1280X800_60HZ          0x53  // 83.46` MHz: 498/6=83



/*BJ 0821 E*/

/*Lisa 5M S*/
#define Sensor_MI_5M                      0
#define Sensor_MI_2M                      1
#define Sensor_COASIA_VGA                 2
#define Sensor_OV7725_VGA                 3
#define Sensor_CCIR656                    4
#define Sensor_CCIR601                    5      //Lucian: TV in,不支援CCIR601. 請改用CCIR656
#define Sensor_MI1320_YUV601              6
#define Sensor_OV7725_YUV601              7
#define Sensor_OV7740_YUV601              8
#define Sensor_MI1320_RAW                 9
#define Sensor_CCIR601_MIX_OV7740YUV      10
#define Sensor_OV7740_RAW                 11
#define Sensor_MI9V136_YUV601             12
#define Sensor_PC1089_YUV601              13
#define Sensor_OV2643_YUV601              14
#define Sensor_MT9M131_YUV601             15
#define Sensor_HM1375_YUV601              16
#define Sensor_NT99141_YUV601             17
#define Sensor_HM5065_YUV601              18
#define Sensor_PO3100K_YUV601             19
#define Sensor_NT99340_YUV601             20
#define Sensor_NT99230_YUV601             21
#define Sensor_PO2210K_YUV601             22
#define Sensor_XC7021_SC2133              23 // ISP XC7021 + snesor SC2133
#define Sensor_XC7021_GC2023              24
#define Sensor_ZN220_YUV601               25
#define Sensor_NONE_HD                    26
#define Sensor_NONE_FHD                   27
/*Lisa 5M E*/

/*Lisa 5M AF_ZOOM S*/
#define MI_5M_AF_ZOOM           0
#define AF_ZOOM_OFF             1
/*Lisa 5M AF_ZOOM E*/

/*AUDIO Option, AUDIO_record_playback*/
#define AUDIO_IIS_IIS       0
#define AUDIO_IIS_DAC       1
#define AUDIO_ADC_DAC       2
#define AUDIO_ADC_IIS       3

/*AUDIO device*/
#define AUDIO_NULL          0
#define AUDIO_AC97_ALC203   1
#define AUDIO_IIS_WM8974    2
#define AUDIO_IIS_ALC5621   3
#define AUDIO_IIS_WM8940    4

/* AUDIO Codec */
#define AUDIO_CODEC_PCM         0
#define AUDIO_CODEC_MS_ADPCM    1
#define AUDIO_CODEC_IMA_ADPCM   2

/* ADC subboard */
#define ADC_SUBBOARD_JUSTEK   1
#define ADC_SUBBOARD_Fuji     2
#define ADC_SUBBOARD_Veri     3

/* Audio mode */
#define AUDIO_AUTO      0
#define AUDIO_MANUAL    1

/* Factory Tool */
#define TOOL_OFF    0
#define TOOL_ON     1

#define AE_report_Soft   0
#define AE_report_Hard  1

/*Video resolution*/
#define VIDEO_VGA_IN_VGA_OUT              0    // 640x480
#define VIDEO_HD_IN_VGA_OUT               1
#define VIDEO_HD_IN_HD_OUT                2    // 1280x720
#define VIDEO_1920x1440DECI3x3TOVGA       3    // 1920x1440 -->640x480 (only SIU-out valid)

#define VIDEO_FULL_HD                     4    // 1920x1080
#define VIDEO_FULLHD_IN_VGA_OUT           5    // 1280x720
#define VIDEO_FULLHD_IN_HD_OUT            6    // 1280x720



/*TV-OUT Video resolution*/
#define TV_QVGA          0
#define TV_VGA           1
#define TV_D1            2
#define TV_CIF           3
#define TV_HD720P60      4
#define TV_HD720P30      5
#define TV_HD720P25      6
#define TV_FHD1080I60    7
#define TV_FHD1080P30    8
#define TV_FHD1080P25    9

#define TV_HD720P60_37M  10

/*TV-IN Mode Selection*/
#define TV_IN_NTSC 0
#define TV_IN_PAL  1

#define TV_IN_60HZ 0
#define TV_IN_50HZ 1


/* HDMI TX selection*/
#define HDMI_TX_NONE      0
#define HDMI_TX_EP952     1  //no test pattern, read EDID by our i2c
#define HDMI_TX_CAT6613   2
#define HDMI_TX_IT66121   3  //support 480p test pattern, read EDID by DDC channel
#define HDMI_TX_CH7038    4

/*Video-In source selection*/
#define VIDEO_IN_SENSOR  0
#define VIDEO_IN_TV      1

/*SALIX SDV selection*/
#define SSDV_1  0
#define SSDV_2  1

/*File system selection*/
#define FILE_SYSTEM_DSC   0   //not support now
#define FILE_SYSTEM_DVR   1   //not support now
#define FILE_SYSTEM_CDVR  2
#define FILE_SYSTEM_DOOR  3

/*File Playback List selection*/
#define FILE_PLAYBACK_MENU          0
#define FILE_PLAYBACK_SPLITMENU     1

/*Sub-File system selection*/
#define FILE_SYSTEM_DVR_SUB0   0        //just one group in 100VIDEO directory
#define FILE_SYSTEM_DVR_SUB1   1        // two group in 100VIDEO directory, Axxxxxxx.ASF : Manual; Bxxxxxxx.ASF : Event
#define FILE_SYSTEM_DVR_SUB2   2        // three group in 100VIDEO directory, Axxxxxxx.ASF : Manual; Bxxxxxxx.ASF : 2 min; Cxxxxxxx.ASF : Enent
#define FILE_SYSTEM_DVR_SUB3   3        // four group in 100VIDEO directory,  Axxxx.asf for CH1, Bxxx.asf for CH2, Cxxx.asf for CH3, Dxxxx.asf for CH4.


/*Flash Light Type Select*/
#define FLASH_LIGHT_NONE  0
#define FLASH_LIGHT_PWM   1
#define FLASH_LIGHT_SRC   2

//Lsk : timer2 is useless in 9002D
#define NO_USE_FINE_TIME_STAMP          0
#define USE_TIMER1_FINE_TIME_STAMP      1
#define USE_TIMER2_FINE_TIME_STAMP      2

#define AUDIO_FOLLOW_VIDEO              0
#define VIDEO_FOLLOW_AUDIO              1             //Lsk : Forward and Backward, and solve frame flutter

#define PLAYBACK_IN_IIS_TASK            0
#define PLAYBACK_IN_IIS_ISR             1
/* Flash Type */
#define FIRMWARE_SIZE_1M	1
#define FIRMWARE_SIZE_2M	2
#define FIRMWARE_SIZE_8M	3
#define FIRMWARE_SIZE_16M	4
#define FIRMWARE_SIZE_32M	5

//--SPI flash--//
#define FLASH_SERIAL_ESMT           0x00
#define FLASH_SERIAL_WINBOND        0x01
#define FLASH_SERIAL_EON            0x02
#define FLASH_SERIAL_SST            0x03
#define FLASH_SERIAL_RESERVED2      0x04
#define FLASH_SERIAL_RESERVED3      0x05
#define FLASH_SERIAL_RESERVED4      0x06
#define FLASH_SERIAL_RESERVED5      0x07

//--NAND flash--//
#define FLASH_NAND_9001_NORMAL      0x08
#define FLASH_NAND_9002_NORMAL      0x09
#define FLASH_NAND_9002_ADV         0x0A
#define FLASH_NAND_9001_RESERVED1   0x0B
#define FLASH_NAND_9001_RESERVED2   0x0C
#define FLASH_NAND_9001_RESERVED3   0x0D
#define FLASH_NAND_9001_RESERVED4   0x0E
#define FLASH_NAND_9001_RESERVED5   0x0F

#define FLASH_NO_DEVICE             0x10
/* Motion Detection Methods */
#define MD_METHOD_1     0x00   //by Lucian
#define MD_METHOD_2     0x01   //by Allen
#define MD_METHOD_3     0x02   //by Lucian

/* Video Codec Option */
#define MJPEG_CODEC  1  //must enable JPEG_DRI_ENABLE / JPEG_OPMODE_FRAME
#define MPEG4_CODEC  2
#define H264_CODEC   3

/* TV decoder Option */
#define TVDEC_NONE   0
#define TI5150       1
#define BIT1605      2
#define MI9V136      3
#define WT8861       4
#define TW9900       5
#define CJC5150      6
#define TW9910       7
#define DM5900       8
#define TW2866       9
#define AHD6124B    10


/* Echo*/
#define FM1288       1

/* TV Encoder Option */
#define TVENC_NONE   0
#define CH7025       1
#define CS8556       2
#define BIT1201G     3


/* TV TCon Option */
#define TCON_NONE    0
#define TCON_MST717C 1

/* RTC Select*/
#define RTC_USE_EXT_RTC     0
#define RTC_USE_BUILD_IN    1
#define RTC_USE_TIMER_RTC   2

/* External RTC Option */
#define RTC_ISL1208    1
#define RTC_HT1381     2
#define RTC_BQ32000    3
#define RTC_PCF8563    4
#define RTC_PT7C43390  5
#define RTC_SD2068     6
#define RTC_HM8563     7 //Should the same as PCF8563, but not test, Paul add@2017.12.29
/* G Sensor Option */
#define G_SENSOR_NONE       0   // No G sensor
#define G_SENSOR_LIS302DL   1   // ST LIS302DL
#define G_SENSOR_H30CD      2   // Hitachi H30CD
#define G_SENSOR_DMARD03    3   // Domintech DMARD03
#define G_SENSOR_BMA150     4   // Bosch BMA150

/* Thermometer Option */
#define THERMO_NONE         0   // No Thermometer
#define THERMO_MLX90615     1   // MLX90615


#define MJPEG_WIDTH                 640
#define MJPEG_HEIGHT                480
#define MJPG_APP0_ENABLE            1
#define MJPG_DRI_ENABLE             1
#define ASF_FORMATE                 0
/* 0 : 6.4 can play, other version pc must have MJPG codec */
/* 1 : only 9 can play */

/* USB Mass Storage Function Option */
#define USB_MSC_READ_WRITE     0
#define USB_MSC_READ_ONLY      1

/* file close method */
#define CLOSE_FILE_BY_SIZE   0
#define CLOSE_FILE_BY_SLICE  1

/* De-interlace method */
#define DOUBLE_FIELD_ON       0   //always use double-field. 只壓一個field. 可去除移動中鋸齒毛邊,但犧牲解析度
#define DOUBLE_FIELD_OFF      1   //alway no-use double-field. 壓兩個field(one frame), 會有鋸齒毛邊, 但有較佳解析度
#define DOUBLE_FIELD_AUTO     2   // auto use double field. 偵測移動,決定是否使用. 可有以上兩者優點. 但切換時會有影像跳動現象.

/* Watch dog*/
#define WATCHDOG_OFF       0
#define WATCHDOG_INTERNAL  1
#define WATCHDOG_EXTERNAL  2

/* TV-in Format detection */
#define TV_IN_FORMAT_DETECT_AUTO    0     //每秒偵測.
#define TV_IN_FORMAT_DETECT_ONCE    1     //只有在開機時偵測.

#define TV_IN_DETECT_BY_DECODER     0     //TV-in 制式由TV decoder決定
#define TV_IN_DETECT_BY_PCB         1     //TV-in 制式由外部電阻決定, High:NTSC, Low: PAL

/*Touch Panel Driver Type Select*/
#define TOUCH_PANEL_DRIVER_NONE         0
#define TOUCH_PANEL_DRIVER_TSC2046      1
#define TOUCH_PANEL_DRIVER_TSC2003      2
#define TOUCH_PANEL_DRIVER_INTERNAL     3
#define TOUCH_PANEL_DRIVER_CAPACITIVE   4
#define TOUCH_PANEL_DRIVER_GT9271       5

/*Touch Key Driver Type Select*/
#define TOUCH_KEY_NONE              0
#define TOUCH_KEY_BS83B12           1
#define TOUCH_KEY_MA86P03           2

/*IO expand Driver Type Select*/
#define IO_EXPAND_NONE              0
#define IO_EXPAND_WT6853            1

/*Volume control Driver Type Select*/
#define Volume_Control_NONE         0
#define Volume_Control_PT2257       1

/*Volume Processer Driver Type Select*/
#define Video_Processer_NONE         0
#define Video_Processer_VX9988       1

/* PIR IC Select for adapted sensitivity PIR */
#define PassiveIR_NONE              0
#define PassiveIR_SS004             1
#define PassiveIR_PYD1588           2
#define PassiveIR_SS0041P           3

#define PassiveIR_TOUBOU            4

/*Video SP resolution*/
#define SP_1x1                      0
#define SP_2x1                      1
#define SP_2x2                      2
#define SP_4x2                      3
#define SP_4x4                      4

/*UI Version Select*/
#define UI_VERSION_OLD              0   /*for salix, dh500...*/
#define UI_VERSION_ONE_TASK         1   /*for ebell...*/
#define UI_VERSION_THREE_TASK       2   /*for opcom...*/
#define UI_VERSION_CUSTOM           3   /*A1016 UI 客人自己定義*/
#define UI_VERSION_ST_1             4   /*A1016 UI.黑底*/
#define UI_VERSION_ST_2             5   /*A1016 UI.藍底*/
#define UI_VERSION_RDI              6   /*RDI UI Lorex*/
#define UI_VERSION_TRANWO           7   /*TRANWO 客人自己定義*/
#define UI_VERSION_GCT              8   /*GCT 客人自己定義*/
#define UI_VERSION_HECHI            9   /*HECHI 客人自己定義*/
#define UI_VERSION_COMMAX          10   /*FOR 8200 COMMAX 客人自己定義*/
#define UI_VERSION_RDI_2           11   /*RDI UI 中性*/
#define UI_VERSION_RDI_3           12   /*RDI UI UDR*/
#define UI_VERSION_MUXCOM          13   /*MUXCOM FILE NAME*/
#define UI_VERSION_TX              14   /*TX 沒有UI*/
#define UI_VERSION_MAYON           15   /*MAYON 客人自己定義*/
#define UI_VERSION_ROULE           16   /*ROULE 客人自己定義*/

/*UI Display Time Format*/
#define UI_SHOW_TIME_FORMAT_NONE    0
#define UI_SHOW_TIME_FORMAT_YMD     1
#define UI_SHOW_TIME_FORMAT_MDY     2
#define UI_SHOW_TIME_FORMAT_MDY_APM 3
#define UI_SHOW_TIME_FORMAT_YMD_APM 4
#define UI_SHOW_TIME_FORMAT_DMY     5

#define UI_TIME_INTERVAL_01MIN      0
#define UI_TIME_INTERVAL_15MIN      1
#define UI_TIME_INTERVAL_30MIN      2
#define UI_TIME_INTERVAL_OFF        3 /* No Light Schedule Timer Setting*/


/*UI Graph Select*/
#define UI_GRAPH_BMP                0
#define UI_GRAPH_JPG                1

/*UI Graph Size*/
#define UI_GRAPH_SIZE_NONE          0
#define UI_GRAPH_SIZE_2MB           1
#define UI_GRAPH_SIZE_4MB           2
#define UI_GRAPH_SIZE_8MB           3
#define UI_GRAPH_SIZE_16MB          4
#define UI_GRAPH_SIZE_HA_2MB        5
#define UI_GRAPH_SIZE_HA_4MB        6
#define UI_GRAPH_SIZE_HA_8MB        7
#define UI_GRAPH_SIZE_HA_16MB       8
#define UI_GRAPH_SIZE_2030p_2MB     9
#define UI_GRAPH_SIZE_2030p_4MB    10
#define UI_GRAPH_SIZE_2030p_8MB    11
#define UI_GRAPH_SIZE_2030p_16MB   12
#define UI_GRAPH_SIZE_2208p_2MB    13
#define UI_GRAPH_SIZE_2208p_4MB    14
#define UI_GRAPH_SIZE_2208p_8MB    15
#define UI_GRAPH_SIZE_2208p_16MB   16

/*UI TX OSD Order Select*/
#define TX_OSD_ORDER_NONE           0
#define TX_OSD_ORDER_SCDT           1  // single,channel,date,time
#define TX_OSD_ORDER_SCTD           2  // single,channel,time,date
#define TX_OSD_ORDER_Single         3  // single
#define TX_OSD_ORDER_DT             4  // date,time

/* UI ISP Bin Code Size Allocation */
#define UI_BIN_CODE_SIZE_1M         0
#define UI_BIN_CODE_SIZE_1D5M       1 /* 擴增為1.5M, 此外必須設定=>(UI_GRAPH_SIZE == UI_GRAPH_SIZE_2208p_16MB) */


/*RFIC select*/
#define RFIC_NORDIG_2M               0

#define RFIC_A7130_2M                1
#define RFIC_A7130_3M                2
#define RFIC_A7130_4M                3

#define RFIC_A7196_4M                4
#define RFIC_A7196_6M                5

#define RFIC_MV400_2M                6
#define RFIC_MV400_4M                7

#define RFIC_NONE_5M                 8

/*RF RX time source selection*/
#define RFIU_RX_TIME_BY_VIDEO        0
#define RFIU_RX_TIME_BY_AUDIO        1

/*RF RX Display mode*/
#define RFIU_RX_DISP_MAIN_VGA        0
#define RFIU_RX_DISP_QUARD_HD        1
#define RFIU_RX_DISP_SUB1            2
#define RFIU_RX_DISP_MASK            3

#define RFIU_RX_DISP_MAIN_HD         4
#define RFIU_RX_DISP_MAIN_FULHD      5
#define RFIU_RX_DISP_QUARD_FULLHD    6

#define RFIU_RX_DISP_PIP             7


/*RF RX Output Resolution*/
#define RF_RX_RESO_VGA               0
#define RF_RX_RESO_HD                1
#define RF_RX_RESO_FULHD             2

/*DRAM Controller select*/
#define DRAMCNTRL_SDRAM              0
#define DRAMCNTRL_DDR2               1

/*TV Digital-out Format select*/
#define TV_DigiOut_656               0
#define TV_DigiOut_601               1
#define TV_DigiOut_YUV               2

/* MULTI_CHANNEL_SEL option */
#define CH_SEL_SIU                   0x01
#define CH_SEL_CIU1                  0x02
#define CH_SEL_CIU2                  0x04
#define CH_SEL_CIU3                  0x08
#define CH_SEL_CIU4                  0x10
#define CH_SEL_CIU5                  0x20



/*Sensor Flicker Config*/
#define SENSOR_AE_FLICKER_60HZ       0
#define SENSOR_AE_FLICKER_50HZ       1

/*Home automation sensor */
#define HOME_SENSOR_NONE             0
#define HOME_SENSOR_HOPERF           1
#define HOME_SENSOR_SWANN            2
#define HOME_SENSOR_MARS             3
#define HOME_SENSOR_BARVOTECH        4
#define HOME_SENSOR_TRANWO          5

#define QUALITY_MODE                0
#define BALANCE_MODE                1
#define SMOOTHFRAME_MODE            2

/*BTC PIR Recoding Mode*/
#define BTC_PIRRECMODE_LOREX        0
#define BTC_PIRRECMODE_ARLO         1
#define BTC_PIRRECMODE_DOORBELL     2

/*For Tranwo No Signal Type*/
#define UI_NOSINGANL_TYPE_SHOW_FULLFRAME 0 /*Show Full Back Frame, nothing notify*/
#define UI_NOSINGANL_TYPE_SHOW_OSDMSG    1 /*Show Msg OSD*/
#define UI_NOSINGANL_TYPE_SHOW_OSDICON   2 /*Show Pure OSD*/

#endif
