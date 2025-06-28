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
#define A1020A_FPGA_BOARD		203
#define A1026A_FPGA_BOARD		204

#define MR8120_TX_DEMO_BOARD    210
#define MR8120_TX_RDI           211  //for 8200_RX_RDI  CA670
#define MR8120_TX_RDI2          212  //for 8600_RX_RDI
#define MR8120_TX_RDI3          213  //for 8120_RX_RDI
#define MR8120_TX_RDI4          214  //for 8660_RX_RDI 與MR8700_TX共用PCB
#define MR8120_TX_JESMAY        215  //for TX with TVP5150
#define MR8120_TX_TRANWO        216  //for Sensor_HM1375
#define MR8120_TX_SOCKET        217
#define MR8120_TX_ZINWELL       218
#define MR8120_TX_DB2           219
#define MR8120_TX_COMMAX        220
#define MR8120_TX_TELEFIELDS    221
#define MR8120_TX_SKYSUCCESS    222  //for TX with OV7725
#define MR8120_TX_SKYSUCCESS_AV 223  //for TX with TI5150
#define MR8120_TX_GCT           224
#define MR8120_TX_RDI_AV        225  //for TW with TI5150
#define MR8120_TX_JIT           226  //M703SN4
#define MR8120_TX_GCT_8200      227  //for 8200_RX_GCT
#define MR8120_TX_Alford        228
#define MR8120_TX_HECHI         229
#define MR8120_TX_TWT_FCC       230
#define MR8120_TX_JESMAY_LCD    231
#define MR8120_TX_JESMAY_AV2    232  // for TX with TW9900
#define MR8120_TX_JIT2          233  // for JIT 8600
#define MR8120_TX_SUNIN_820HD   234  //820HD
#define MR8120_TX_TRANWO2       235  // for Sensor_NT99141_YUV601
#define MR8120_TX_RDI_CA671     236  
#define MR8120_TX_MAYON         237  //MWL605 For MR8200A  MWL604 For MR8120A
#define MR8120_TX_RDI_CA530     238
#define MR8120_TX_FRESHCAM      239
#define MR8120_TX_RDI_CA672     240  
#define MR8120_TX_Philio        241
#define MR8120_TX_MAYON_LW604   242  // for 8600 Model with OV7725
#define MR8120_TX_MAYON_MWL605C 243  // for 8600 Model with NT99141
#define MR8120_TX_MA8806	    244
#define MR8120_TX_ALM	        245
#define MR8120_TX_TRANWO_VM2505 246  // VM2505T
#define MR8120_TX_RDI_CA532     247
#define MR8120_TX_RDI_CL692     248
#define MR8120_TX_RDI_CA652     249
#define MR8100_GCT_VM9710       250
#define MR8120_TX_RDI_CA542     251
#define MR8211_TX_RDI_SEP       252
#define MR8120_TX_TRANWO3       253 //based MR8120_TX_TRANWO2 For A1020
#define MR8120_TX_GCT_VM00      254


#define MR8600_RX_DEMO_BOARD    270
#define MR8600_RX_RDI           271  // for BIT12101G
#define MR8600_RX_DB2           272
#define MR8600_RX_GCT           273
#define MR8600_RX_JESMAY_LCD    274  // 8600 216M
#define MR8600_RX_JESMAY        275  // 8600 128M
#define MR8600_RX_JIT           276
#define MR8600_RX_TRANWO        277
#define MR8600_RX_RDI2          278  // for CS8556
#define MR8600_RX_MAYON         279  // MWM015
#define MR8600_RX_SKYSUCCESS    280
#define MR8600_RX_RDI_M904D     281

#define MR8200_RX_DEMO_BOARD        300
#define MR8200_RX_RDI               301
#define MR8200_RX_TRANWO_LCD        302   //D8492
#define MR8200_RX_SOCKET            303
#define MR8200_RX_ZINWELL           304
#define MR8200_RX_DB2               305
#define MR8200_RX_COMMAX            306   //WL-1@CWM-1020AQ
#define MR8200_RX_COMMAX_BOX        307   //WL-1-1_Quad_box@CWS-40QR
#define MR8200_RX_RDI_M930          308
#define MR8200_RX_DB3               309
#define MR8200_RX_JIT               310   //M703SN4
#define MR8200_RX_RDI_M900          311   //RDI1052_M900 Lorex
#define MR8200_RX_Alford_BOX        312
#define MR8200_RX_GCT_LCD           313   //VM7777
#define MR8200_RX_RDI_RX240         314   //RDI1140
#define MR8200_RX_RDI_M721          315   //RDI1161_M721 Lorex CM6772
#define MR8200_RX_RDI_M701          316   //RDI1167_M701 Lorex
#define MR8200_RX_RDI_M920          317   //RDI1165_M920 Lorex CM671922
#define MR8200_RX_JIT_BOX           318   //D808SN4
#define MR8200_RX_MAYON_MWM719      319   //RX:MWM719  TX: MWL605 
#define MR8200_RX_MAYON_MWM720      320   //MWM713 For Touch pannel version
#define MR8200_RX_RDI_M902          321
#define MR8200_RX_TRANWO_D8593      322 
#define MR8200_RX_Philio            323 
#define MR8200_RX_MAYON_MWM014      324   //RX:MWM014 TX: MWL606 GateWay Box version
#define MR8200_RX_TRANWO_BOX        325
#define MR8200_RX_RDI_M742          326   //RDI1209 GT
#define MR8200_RX_RDI_M731          327
#define MR8200_RX_RDI_M712          328
#define MR8200_RX_TRANWO_D2505      329
#define MR8200_RX_TRANWO_D8593_HA 	330
#define MR8200_RX_RDI_M731_HA       331
#define MR8200_RX_RDI_M706          332
#define MR8200_RX_RDI_M742_HA       333
#define MR8200_RX_JIT_BOX_AV        334   //806
#define MR8200_RX_MAYON_MWM902      335   // For Touch key version
#define MR8200_RX_MAYON_MWM903      336   // For Touch Panel version
#define MR8200_RX_JIT_M703SN4       337   //M703SN4 new verison
#define MR8200_RX_JIT_D808SN4       338   //D808SN4 new verison
#define MR8200_RX_TRANWO_SMH101     339
#define MR8200_RX_TRANWO_SMH101_HA  340
#define MR8200_RX_TRANWO_D8593RS    341   //D8593R-SW in SMH101HA-HW
#define MR8200_RX_TRANWO_D8589N     342   //D8589 W/ Nic Support



#define MR8120_RX_DEMO_BOARD        350
#define MR8120_RX_JESMAY            351
#define MR8120_RX_RDI               352
#define MR8120_RX_RDI_M713          353   //RDI1134 Lorex
#define MR8120_RX_RDI_M703          354	//Defender & SVAT
#define MR8120_RX_VTECH             355
#define MR8120_RX_TELEFIELDS        356
#define MR8120_RX_SKYSUCCESS        357
#define MR8120_RX_JIT_LCD           358
#define MR8120_RX_HECHI             359
#define MR8120_RX_SUNIN_820HD       360   //820HD
#define MR8120_RX_MAYON_MWM710      361   //RX: MWM710 TX: MWL604 
#define MR8120_RX_FRESHCAM          362
#define MR8120_RX_TRANWO            363
#define MR8120_RX_JIT_BOX           364   //D808-3
#define MR8120_RX_MAYON_MWM011      365
#define MR8120_RX_RDI_M724          366
#define MR8120_RX_RDI_M733          367
#define MR8120_RX_JIT_M703SW4       368   //M703SW4  new verison
#define MR8120_RX_JIT_D808SW3       369   //D808SW3 new verison
#define MR8120_RX_TRANWO_D8592      370   //D8593 8120 1RF No NIC_SPPORT VERSION
#define MR8120_RX_TRANWO_D8592RS    371   //D8592R-SW in SMH101HA-HW 
#define MR8120_RX_TRANWO_D8589      372
#define MR8120_RX_GCT_SC7700        373   /*IC是用MR8100*/

#define MR9670_DEMO_BOARD       380
#define MR9670_COMMAX           381 // CDV-71AM-VE
#define MR9670_COMMAX_WI2       382 // CDV-70KM
#define MR9670_COMMAX_71UM      383 // CDV-71UM
#define MR9670_WOAN             384
#define MR9670_HECHI            385
#define MR9670_HECHI_4CH        386

#define MR6730_WINEYE           401
#define MR6730_AFN              402

#define MR8211_DEMO_BOARD       430
#define MR8211_ZINWELL          431

#define MR8100_GCT_LCD          440
#define MR8100_RX_RDI_SEM       441
#define MR8100_RX_RDI_M512      442

//------------A1020--------------//
#define MR8120B_TX_MAYON        500  // for A1020
#define MR8120S_TX_GCT_VM00     501
#define MR8211B_TX_RDI_WD542I   502




//-------------A1018----------//
#define A1018_FPGA_BOARD        901
#define A1018_EVB_128M          902   //A1018 EVB and A1018 FPGA 共用
#define A1018_EVB_256M_TW2866   903   //A1018 EVB and A1018 FPGA 共用+TW8866
#define A1018B_FPGA_BOARD       904
#define A1018_EVB_256M_HM1375   905   //A1018 EVB and A1018 FPGA 共用+TW8866


#define MR6790_DB               910

#define MR9211_DB               930

#define MR9120_TX_DB            940

#define MR9600_RX_DB            950
#define MR9600_RX_OPCOM         951

#define MR9200_RX_DB            960

#define MR9300_RX_DB            970
//-----------------------RF Customer ID-------------------//
#define RFI_CUSTOMER_ID_ALLPASS             0

#define RFI_CUSTOMER_ID_LOREX               1000       //RDI series

#define RFI_CUSTOMER_ID_SWANN               2000       //TRAWO series
#define RFI_CUSTOMER_ID_SWANN_A1020         2001       //TRAWO series to Match A1020 TX

#define RFI_CUSTOMER_ID_JIT                 3000
#define RFI_CUSTOMER_ID_MAYON               4000
#define RFI_CUSTOMER_ID_MAYON_8600          4001
#define RFI_CUSTOMER_ID_COMMAX              5000
#define RFI_CUSTOMER_ID_JESMAY              6000
#define RFI_CUSTOMER_ID_HECHI               7000
#define RFI_CUSTOMER_ID_GCT                 8000
#define RFI_CUSTOMER_ID_SKYSUCCESS          9000


//--------------RF protocol selection-------------------//
#define RFI_PROTOCOL_ISOWIFI  0
#define RFI_PROTOCOL_CE_181   1
#define RFI_PROTOCOL_FCC_247  2
#define RFI_PROTOCOL_ORIG     3

//----------RF TX VMD selection--------//
#define TX_VMD_ORIG          0
#define TX_VMD_NEW           1



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
#define STORAGE_MEMORY_RAMDISK      0
#define STORAGE_MEMORY_SD_MMC       1
#define STORAGE_MEMORY_SMC_NAND     2
#define STORAGE_MEMORY_USB_HOST       3
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

#define CHIP_A1016B             9	// 2013
#define CHIP_A1020A             10   // 2016

#define CHIP_A1018A             21  // 2013

#define CHIP_A1018B             22
#define CHIP_A1013B             23
#define CHIP_A1026A             24   // 2017


/*
*    Software Application
*/
//==RF-Cam TX Module(QFP128)==//
#define MR8120_RFCAM_TX1        0
#define MR8120_RFCAM_TX2        1
#define MR8100_RFCAM_TX1        2
#define MR8120_RFCAM_TX1_6M     3

#define MR8211_RFCAM_TX1        4    //used for dual-mode-VBM TX 請使用CopyToUSB_U96.bat

//==RF 1RX/2RX + P2P(QFP216)==//
#define MR8200_RFCAM_RX1        10
#define MR8200_RFCAM_RX1RX2     11


//==RF 2RX + Dual TV out(QFP128)==//
#define MR8120_RFAVSED_RX1      20   // 1T1R1TV
#define MR8600_RFAVSED_RX1RX2   21   // 2T2R2TV
#define MR8600_RFAVSED_RX1      22   // 2T1R2TV
#define MR8600_RFAVSED_RX1_6M   23

//==RF 1RX + Pannel/TV(QFP216)==//
#define MR8120_RFCAM_RX1        30   // 1 RX module
#define MR8600_RFCAM_RX1RX2     31   // 2 RX module

//==Car DVR(QFP216)==//
#define MR6730_CARDVR_2CH       40
#define MR6730_CARDVR_1CH       41

//==Door Phone(QFP216)==//
#define MR9670_DOORPHONE        50

//==IP CAM(QFP216)==//
#define MR8211_IPCAM            60

//==Baby monitor ==//
#define MR8100_BABYMONITOR      70	
#define MR8100_DUALMODE_VBM     71   //Lucian:OK. 請使用CopyToUSB_SEM_WDT.bat


//--Only for Test,請自行定義--//
#define DVP_RF_SELFTEST             100   //RF 對傳測試 or FCC test
#define Standalone_Test             101
#define FPGA_VERIFY_TEST_CIU        102
#define FPGA_VERIFY_TEST_IDU        103
#define FPGA_VERIFY_TEST_SIU        104
#define FPGA_VERIFY_TEST_RFU        105
#define FPGA_VERIFY_TEST_H264	    106
#define FPGA_VERIFY_TEST_USB_HOST	107
#define FPGA_VERIFY_TEST_MAC	    108
#define FPGA_VERIFY_TEST_DUAL_CPU	109
#define FPGA_VERIFY_TEST_DIU        110   //test de-interlace engine.

#define FPGA_VERIFY_TEST_RFTX1      111   //no used
#define FPGA_VERIFY_TEST_RFRX1      112   // no used
#define FPGA_VERIFY_TEST_PIP	    113
#define FPGA_VERIFY_TEST_GFU	    114
#define FPGA_VERIFY_TEST_VMD	    115



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
#define LCM_P_RGB_888_ILI6122      0x21
#define LCM_P_RGB_888_ILI6126C     0x22


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
#define Sensor_NONE                       21

/*Lisa 5M E*/

/*Lisa 5M AF_ZOOM S*/
#define MI_5M_AF_ZOOM           0
#define AF_ZOOM_OFF             1
/*Lisa 5M AF_ZOOM E*/

/*AUDIO Module*/
#define AUDIO_IIS           0
#define AUDIO_AC97_ALC203   1
#define AUDIO_IIS_WM8974    2
#define AUDIO_ADC_DAC       3
#define AUDIO_IIS_ALC5621   4

/* AUDIO Codec */
#define AUDIO_CODEC_PCM         0
#define AUDIO_CODEC_MS_ADPCM    1
#define AUDIO_CODEC_IMA_ADPCM   2

/* ADC subboard */
#define ADC_SUBBOARD_JUSTEK   1
#define ADC_SUBBOARD_Fuji     2

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
#define VIDOE_FULL_HD                     4    // 1920x1080

/*TV-OUT Video resolution*/
#define TV_QVGA     0
#define TV_VGA      1
#define TV_D1       2
#define TV_CIF      3
#define TV_HD720P   4
#define TV_FHD1080I 5
#define TV_FHD1080P 6

/*TV-IN Mode Selection*/
#define TV_IN_NTSC 0
#define TV_IN_PAL  1

#define TV_IN_60HZ 0
#define TV_IN_50HZ 1


/* HDMI TX selection*/
#define HDMI_TX_NONE      0
#define HDMI_TX_EP952     1
#define HDMI_TX_CAT6613   2

/*Video-In source selection*/
#define VIDEO_IN_SENSOR  0
#define VIDEO_IN_TV      1

/*SALIX SDV selection*/
#define SSDV_1  0
#define SSDV_2  1

/*File system selection*/
#define FILE_SYSTEM_DSC   0
#define FILE_SYSTEM_DVR   1
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

#define PLAYBACK_IN_IIS_TASK              0
#define PLAYBACK_IN_IIS_ISR            1             //Lsk : Forward and Backward, and solve frame flutter

/* Flash Type */
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
#define RTC_NULL       0
#define RTC_ISL1208    1
#define RTC_HT1381     2
#define RTC_BQ32000    3
#define RTC_SD2068     4

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
#define TOUCH_PANEL_DRIVER_NONE     0
#define TOUCH_PANEL_DRIVER_TSC2046  1
#define TOUCH_PANEL_DRIVER_TSC2003  2
#define TOUCH_PANEL_DRIVER_CAPACITIVE 3

/*Touch Key Driver Type Select*/
#define TOUCH_KEY_NONE              0
#define TOUCH_KEY_BS83B12           1
#define TOUCH_KEY_MA86P03           2
#define TOUCH_KEY_BS8112A3          3
#define TOUCH_KEY_CBM7320           4

/*IO expand Driver Type Select*/
#define IO_EXPAND_NONE              0
#define IO_EXPAND_WT6853            1


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
#define UI_VERSION_RDI_4           13   /*RDI UI VBM*/

/*UI Display Time Format*/
#define UI_SHOW_TIME_FORMAT_NONE    0
#define UI_SHOW_TIME_FORMAT_YMD     1
#define UI_SHOW_TIME_FORMAT_MDY     2
#define UI_SHOW_TIME_FORMAT_MDY_APM 3
#define UI_SHOW_TIME_FORMAT_YMD_APM 4
#define UI_SHOW_TIME_FORMAT_DMY     5
#define UI_SHOW_TIME_FORMAT_DMY_APM 6

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

/*RFIC select*/
#define RFIC_NORDIG_2M               0

#define RFIC_A7130_2M                1
#define RFIC_A7130_3M                2
#define RFIC_A7130_4M                3

#define RFIC_A7196_4M                4
#define RFIC_A7196_6M                5

#define RFIC_MV400_2M                6
#define RFIC_MV400_4M                7

#define RFIC_NONE                    8

/*RF RX time source selection*/
#define RFIU_RX_TIME_BY_VIDEO        0
#define RFIU_RX_TIME_BY_AUDIO        1

/*RF RX Display mode*/
#define RFIU_RX_DISP_MAIN            0
#define RFIU_RX_DISP_QUARD           1
#define RFIU_RX_DISP_SUB1            2
#define RFIU_RX_DISP_MASK            3

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

/*For Tranwo No Signal Type*/
#define UI_NOSINGANL_TYPE_SHOW_FULLFRAME 0 /*Show Full Back Frame, nothing notify*/
#define UI_NOSINGANL_TYPE_SHOW_OSDMSG    1 /*Show Msg OSD*/
#define UI_NOSINGANL_TYPE_SHOW_OSDICON   2 /*Show Pure OSD*/

#endif
