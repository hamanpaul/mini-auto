#ifndef __MEMORY_POOL_H__
#define __MEMORY_POOL_H__

#include "iduapi.h"
#include "mpeg4api.h"
#include "iisapi.h"

/*
 *********************************************************************************************************
 *  SYS Constant
 *********************************************************************************************************
 */
#if (FPGA_BOARD_A1018_SERIES)
    #define HTML_DATASIZE  0x20000   /* 128 KByte */
#endif
#if(Sensor_OPTION == Sensor_MI_2M)
    #define IMG_MAX_WIDTH                  1600
    #define IMG_MAX_HEIGHT                 1200

    #define SENSOR_VALID_SIZE_X            1604
    #define SENSOR_VALID_SIZE_Y            1204

#elif(Sensor_OPTION == Sensor_MI_5M)
    #define IMG_MAX_WIDTH                  2560
    #define IMG_MAX_HEIGHT                 1920

    #define SENSOR_VALID_SIZE_X            2564
    #define SENSOR_VALID_SIZE_Y            1924

#elif( (Sensor_OPTION == Sensor_MI1320_YUV601) || (Sensor_OPTION == Sensor_MI1320_RAW) )
    #define IMG_MAX_WIDTH                  1280
    #define IMG_MAX_HEIGHT                 1024

    #define SENSOR_VALID_SIZE_X            1284
    #define SENSOR_VALID_SIZE_Y            1028
#elif( (Sensor_OPTION == Sensor_OV2643_YUV601) || (Sensor_OPTION == Sensor_MT9M131_YUV601) || (Sensor_OPTION == Sensor_HM1375_YUV601) || (Sensor_OPTION == Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_PO3100K_YUV601) )
    #define IMG_MAX_WIDTH                  1280
    #define IMG_MAX_HEIGHT                 720

    #define SENSOR_VALID_SIZE_X            1284
    #define SENSOR_VALID_SIZE_Y            720
#elif( (Sensor_OPTION == Sensor_HM5065_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601) || (Sensor_OPTION == Sensor_PO2210K_YUV601) || (Sensor_OPTION == Sensor_ZN220_YUV601) || (Sensor_OPTION == Sensor_XC7021_SC2133) || (Sensor_OPTION == Sensor_XC7021_GC2023) )
    #define IMG_MAX_WIDTH                  1920
    #define IMG_MAX_HEIGHT                 1080

    #define SENSOR_VALID_SIZE_X            1920
    #define SENSOR_VALID_SIZE_Y            1080
#elif( (Sensor_OPTION == Sensor_CCIR656) ||(Sensor_OPTION == Sensor_CCIR601) || (Sensor_OPTION == Sensor_CCIR601_MIX_OV7740YUV) )
    #define IMG_MAX_WIDTH                  720
    #define IMG_MAX_HEIGHT                 576

    #define SENSOR_VALID_SIZE_X            724
    #define SENSOR_VALID_SIZE_Y            580

#elif(Sensor_OPTION == Sensor_OV7725_VGA)
    #define IMG_MAX_WIDTH                  640
    #define IMG_MAX_HEIGHT                 480
    //lisa OV7725
    #define SENSOR_VALID_SIZE_X            644
    #define SENSOR_VALID_SIZE_Y            484

#elif(Sensor_OPTION == Sensor_OV7725_YUV601)
    #define IMG_MAX_WIDTH                  640
    #define IMG_MAX_HEIGHT                 480
    #define SENSOR_VALID_SIZE_X            644
    #define SENSOR_VALID_SIZE_Y            484

#elif( (Sensor_OPTION == Sensor_OV7740_YUV601) || (Sensor_OPTION == Sensor_OV7740_RAW) || (Sensor_OPTION == Sensor_MI9V136_YUV601) || (Sensor_OPTION == Sensor_PC1089_YUV601) || (Sensor_OPTION == Sensor_PC1089_YUV601) )
    #define IMG_MAX_WIDTH                  640
    #define IMG_MAX_HEIGHT                 480
    #define SENSOR_VALID_SIZE_X            644
    #define SENSOR_VALID_SIZE_Y            484
#elif(Sensor_OPTION == Sensor_NONE_HD)
    #define IMG_MAX_WIDTH                  1280
    #define IMG_MAX_HEIGHT                 720

    #define SENSOR_VALID_SIZE_X            1284
    #define SENSOR_VALID_SIZE_Y            720
#elif(Sensor_OPTION == Sensor_NONE_FHD)
    #define IMG_MAX_WIDTH                  1920
    #define IMG_MAX_HEIGHT                 1080

    #define SENSOR_VALID_SIZE_X            1920
    #define SENSOR_VALID_SIZE_Y            1080
#endif

#define IPU_LINE_SIZE                  IMG_MAX_WIDTH
#define EXIFDECBUF_SIZE                (IMG_MAX_WIDTH*IMG_MAX_HEIGHT)

#define EXIF_THUMBNAIL_MAX             (160 * 120+ 160*120+512) //Lucian: +App2 appendix information
#define EXIF_THUMBNAIL_WIDTH           160
#define EXIF_THUMBNAIL_HEIGHT            120
#define EXIF_PRIMARY_MAX               (IMG_MAX_WIDTH * IMG_MAX_HEIGHT)

#define PKBUF_SIZE                     (IMG_MAX_WIDTH * IMG_MAX_HEIGHT*2)



#if MULTI_STREAM_SUPPORT
#define VIDEODISPBUF_SIZE              (MPEG4_MAX_WIDTH*MPEG4_MAX_HEIGHT*3/2 + ((MPEG4_MAX_WIDTH/2)*(MPEG4_MAX_HEIGHT/2)*3/2))  /* Amon  150128 */
#define VIDEODISPBUF_OFFSET            (MPEG4_MAX_WIDTH*MPEG4_MAX_HEIGHT*3/2)  /* Amon  150128 */
#else
   #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) )
     #if(USE_NEW_MEMORY_MAP)
       #define VIDEODISPBUF_SIZE              (1920*1440*3/2)  /* Peter 070108 */
     #else
       #define VIDEODISPBUF_SIZE              (MPEG4_MAX_WIDTH*MPEG4_MAX_HEIGHT*3/2)  /* Peter 070108 */
     #endif
   #else
      #define VIDEODISPBUF_SIZE              (MPEG4_MAX_WIDTH*MPEG4_MAX_HEIGHT*3/2)  /* Peter 070108 */
   #endif
#endif

#if(SW_APPLICATION_OPTION == MR8202_AN_KLF08W)
#define MPEG4_MIN_BUF_SIZE             (1280 * 720 / 2) // Minimum bitstream size of single MPEG4 video frame
#else
#define MPEG4_MIN_BUF_SIZE             (MPEG4_MAX_WIDTH * MPEG4_MAX_HEIGHT / 2) // Minimum bitstream size of single MPEG4 video frame
#endif


#ifdef OPCOM_JPEG_GUI
    #define GUI_JPGFILE_MAX_SIZE            0x180000
#else
    #define GUI_JPGFILE_MAX_SIZE            0
#endif

#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) ||\
      (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM))
  #if(VIDEO_RESOLUTION_SEL == VIDEO_HD_IN_HD_OUT)
     #define MAX_MPEG4_BITS_BUF_UNIT             5   //Lucian:決定預錄 bits buffer size
  #elif(VIDEO_RESOLUTION_SEL == VIDEO_FULL_HD)
     #define MAX_MPEG4_BITS_BUF_UNIT             5
  #else
     #define MAX_MPEG4_BITS_BUF_UNIT             20
  #endif

#elif(SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5)
  #if(VIDEO_RESOLUTION_SEL == VIDEO_HD_IN_HD_OUT)
     #define MAX_MPEG4_BITS_BUF_UNIT             2   //Lucian:決定預錄 bits buffer size
  #elif(VIDEO_RESOLUTION_SEL == VIDEO_FULL_HD)
     #define MAX_MPEG4_BITS_BUF_UNIT             2
  #else
     #define MAX_MPEG4_BITS_BUF_UNIT             20
  #endif

#elif(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
  #if(VIDEO_RESOLUTION_SEL == VIDEO_HD_IN_HD_OUT)
     #define MAX_MPEG4_BITS_BUF_UNIT             8   //Lucian:決定預錄 bits buffer size
  #elif(VIDEO_RESOLUTION_SEL == VIDEO_FULL_HD)
     #define MAX_MPEG4_BITS_BUF_UNIT             8
  #else
     #define MAX_MPEG4_BITS_BUF_UNIT             20
  #endif
  
#elif ( SW_APPLICATION_OPTION == MR6730_CARDVR_2CH)
  #if(VIDEO_RESOLUTION_SEL == VIDEO_HD_IN_HD_OUT)
     #define MAX_MPEG4_BITS_BUF_UNIT             15   //Lucian:決定預錄 bits buffer size
  #elif(VIDEO_RESOLUTION_SEL == VIDEO_FULL_HD)
     #define MAX_MPEG4_BITS_BUF_UNIT             7
  #else
     #define MAX_MPEG4_BITS_BUF_UNIT             20
  #endif
#else
  #if(VIDEO_RESOLUTION_SEL == VIDEO_HD_IN_HD_OUT)
     #define MAX_MPEG4_BITS_BUF_UNIT             10   //Lucian:決定預錄 bits buffer size
  #elif(VIDEO_RESOLUTION_SEL == VIDEO_FULL_HD)
     #define MAX_MPEG4_BITS_BUF_UNIT             5
  #else
     #define MAX_MPEG4_BITS_BUF_UNIT             20
  #endif
#endif


         // for 16MB SDRAM
#if (CDVR_LOG || CDVR_TEST_LOG)
    #define LOG_FILE_MAX_SIZE                    1400000
  #if(RFIU_SUPPORT && !(RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2))
    #define MPEG4_MAX_BUF_SIZE              (720*480/2*8)   // Total bitstream size of MPEG4 video: 1,658,880 Byte
  #else
    #define MPEG4_MAX_BUF_SIZE              (MPEG4_MIN_BUF_SIZE * MAX_MPEG4_BITS_BUF_UNIT - LOG_FILE_MAX_SIZE - GUI_JPGFILE_MAX_SIZE)   // Total bitstream size of MPEG4 video: 2,611,200 Bytes
  #endif
#else
  #if (RFIU_SUPPORT && !(RFI_TEST_TX_PROTOCOL_B1 || RFI_TEST_TX_PROTOCOL_B2))
    #if( (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2) || (SW_APPLICATION_OPTION == Standalone_Test) || (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || \
         (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) ||(SW_APPLICATION_OPTION ==MR8202_GATEWAYBOX_RX) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1))     
        #define MPEG4_MAX_BUF_SIZE              (720*480/2*8)
    #elif (SW_APPLICATION_OPTION == MR8202_AN_KLF08W)
        #define MPEG4_MAX_BUF_SIZE              (1280*720/2*4)    // 1,843,200 Bytes
    #else
        #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
            #if(USE_NEW_MEMORY_MAP)
            #define MPEG4_MAX_BUF_SIZE              (1280*720/2*8)  //Lsk: add space for 9300
            #else
            #define MPEG4_MAX_BUF_SIZE              (1280*720/2*8)   // Total bitstream size of MPEG4 video: 1,658,880 Byte
            #endif
        #elif( (SW_APPLICATION_OPTION  == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
            #if(USE_NEW_MEMORY_MAP)
            #define MPEG4_MAX_BUF_SIZE              (1280*720/2*10)   // Total bitstream size of MPEG4 video: 1,658,880 Byte
            #else
            #define MPEG4_MAX_BUF_SIZE              (1280*720/2*8)    // Total bitstream size of MPEG4 video: 1,658,880 Byte
            #endif
        #else
            #if( (SW_APPLICATION_OPTION  == MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) )
            #define MPEG4_MAX_BUF_SIZE              (1280*720/2*11)   // Total bitstream size of MPEG4 video: 1,658,880 Byte
            #else
            #define MPEG4_MAX_BUF_SIZE              (1280*720/2*8)   // Total bitstream size of MPEG4 video: 1,658,880 Byte
            #endif
        #endif
    #endif
  #else
    #define MPEG4_MAX_BUF_SIZE              (MPEG4_MIN_BUF_SIZE * MAX_MPEG4_BITS_BUF_UNIT - GUI_JPGFILE_MAX_SIZE)   // Total bitstream size of MPEG4 video: 4,147,200 Byte
  #endif
#endif

#define MPEG4_MVBUF                              (MPEG4_MAX_WIDTH)
#define MPEG4_INDEX_BUF_SIZE                     ((2 * 60 * 60 + 600) * 6)  // ASF 2 hours 10 minutes index size (bytes)


#define PNBUF_SIZE_Y                             (MPEG4_MAX_WIDTH*MPEG4_MAX_HEIGHT)        /*Lucian 070531*/
#define PNBUF_SIZE_C                             (PNBUF_SIZE_Y/2)

#define PNBUF_SP_SIZE_Y                         ((SP_MAX_WIDTH)*(SP_MAX_HEIGHT))        /*Amon 150129*/
#define PNBUF_SP_SIZE_C                         (PNBUF_SP_SIZE_Y/2)

#define ULTRA_FHD_SIZE_Y                         (1920*1440)        
#define ULTRA_FHD_SIZE_C                         (ULTRA_FHD_SIZE_Y/2)

//Timer interval control
#define TIMER0_INTERVAL                          25		/* for adckey polling, modify timer interval to 25 ms */
#define TIMER1_INTERVAL                          1000

    #if (TIMER_TEST == 1)
        #define TIMER5_INTERVAL               2000
        #define TIMER6_INTERVAL               3000
        #define TIMER7_INTERVAL               4000
    #else
        #define TIMER5_INTERVAL               8
		#if MARSS_SUPPORT
        	#define TIMER6_INTERVAL               1
		#else
        	#define TIMER6_INTERVAL               1000
		#endif
        #define TIMER7_INTERVAL               102400 //102.4 sec
    #endif
/* Peter 070108 E */
#if (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)
#define DISPLAY_BUF_NUM                 5  // 3 or 12 display buffer; Lucian: 12 ->6
#else
#define DISPLAY_BUF_NUM                 6  // 3 or 12 display buffer; Lucian: 12 ->6
#endif
#define IISPLAY_BUF_NUM                 16

#define RFI_AUDIO_RET_BUF_NUM           64


#if( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION==MR8200_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) ||\
     (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) || (SW_APPLICATION_OPTION==MR8200_RFCAM_RX1) )
#define MEMORY_POOL_START_ADDR          0x80500000
#elif((SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
#define MEMORY_POOL_START_ADDR          0x80200000
#elif( (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) )
#define MEMORY_POOL_START_ADDR          0x80500000
#else
#define MEMORY_POOL_START_ADDR          0x80400000
#endif

#define SDC_BLK_LEN                     0x0200      /* SD block length */
#define MMC_BLK_LEN                     0x0200      /* MMC block length */



/* Multi-page test */
#define SMC_MULTI_PAGE                  4
/* Max. SMC page size */
#if 0		/* have been defined in smc.c*/
#if ((FLASH_OPTION == FLASH_NAND_9001_NORMAL) ||(FLASH_OPTION == FLASH_NAND_9002_NORMAL))
#define SMC_MAX_PAGE_SIZE               512
#define SMC_MAX_REDUN_SIZE              16
#elif (FLASH_OPTION == FLASH_NAND_9002_ADV)
#define SMC_MAX_PAGE_SIZE               2048
#define SMC_MAX_REDUN_SIZE              16
#endif
#endif

/* Multi-block test */
#define SDC_MULTI_BLK                   8

/* Buffer for DCF layer */
#define DCF_BUF_SIZE            (SDC_BLK_LEN * 128)
#define MIN_DISK_FREE_SIZE      (((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE * IIS_BUF_NUM)) / 1024)  //Notice: K-Byte unit

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
#define DCF_LOGBUF_SIZE          (SDC_BLK_LEN * 64)  // 32kB
#endif

//added by Albert Lee on 20090611
//FS temp buffer
#define FSTEMP_BUF_SIZE			8192

#define IDUREGNUM                       70
/* BJ 0505 S */
//#if IIS_TEST
#if 1
  #define IIS_BUF_SIZ           256        /* 800bytes/100ms @ Fs=8000Hz */
  #define IIS_REC_TIME          300        /* IIS_REC_TIME*100ms */
#endif

#if(OSD_SIZE_D1_ENABLE  && (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_VGA))
#define TV_MAXOSD_SizeX 704  //for set mask area when TVout resolution CIF/DI
#define TV_MAXOSD_SizeY 576
#elif ( (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_HD720P60) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_HD720P30) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_HD720P25) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_HD720P60_37M))
    #define TV_MAXOSD_SizeX 1280
    #define TV_MAXOSD_SizeY 720
#elif ((TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_FHD1080I60) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_FHD1080P30) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_FHD1080P25) )
    #define TV_MAXOSD_SizeX 1920
    #define TV_MAXOSD_SizeY 1080
#else
#define TV_MAXOSD_SizeX 352  //for set mask area when TVout resolution CIF/DI
#define TV_MAXOSD_SizeY 288

#endif

#if (OSD_SIZE_X2_DISABLE)
    #if(OSD_SIZE_D1_ENABLE && (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_VGA))
        #define TVOSD_SizeX 704
        #define TVOSD_SizeY 576
    #elif ((TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_HD720P60) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_HD720P30) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_HD720P25) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_HD720P60_37M))
        #define TVOSD_SizeX 1280
        #define TVOSD_SizeY 720
    #elif ((TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_FHD1080I60) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_FHD1080P30) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_FHD1080P25) )
        #define TVOSD_SizeX 1920
        #define TVOSD_SizeY 1024
    #else
        #define TVOSD_SizeX 640
        #define TVOSD_SizeY 576
    #endif
#else
    #if( (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_D1) || (TVOUT_RESOLUTION_PREVIEW_CAPTURE==TV_CIF))
        #define TVOSD_SizeX 320
        #define TVOSD_SizeY 288
    #else
        #define TVOSD_SizeX 320
        #define TVOSD_SizeY 288
    #endif
#endif

#if (LCM_OPTION == LCM_HX5073_RGB)
    #define OSD_SizeX 320
    #define OSD_SizeY 240

#elif (LCM_OPTION == LCM_HX5073_YUV)
    #define OSD_SizeX 640
    #define OSD_SizeY 480

#elif ((LCM_OPTION == LCM_HX8312)||(LCM_OPTION == LCM_TMT035DNAFWU24_320x240)||(LCM_OPTION == LCM_LQ035NC111))
    #define OSD_SizeX 320
    #define OSD_SizeY 240

#elif ( (LCM_OPTION == LCM_HX8224) || (LCM_OPTION == LCM_TPG105)||(LCM_OPTION == LCM_TJ015NC02AA))
    #define OSD_SizeX 160
    #define OSD_SizeY 240
#elif ( (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) )
    #define OSD_SizeX 640
    #define OSD_SizeY 240
#elif (LCM_OPTION == LCM_CCIR601_640x480P)
    #define OSD_SizeX 160
    #define OSD_SizeY 240
#elif (LCM_OPTION == LCM_CCIR601_1280x720p30)
    #define OSD_SizeX 160
    #define OSD_SizeY 240
#elif (LCM_OPTION == LCM_HX8817_RGB)
    #define OSD_SizeX 480
    #define OSD_SizeY 240
#elif ((LCM_OPTION == LCM_HX8257_RGB666_480x272)||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272))
    #define OSD_SizeX 480
    #define OSD_SizeY 272
#elif ( (LCM_OPTION ==LCM_TD020THEG1) )
    #define OSD_SizeX 214
    #define OSD_SizeY 240
#elif(LCM_OPTION == LCM_TD036THEA3_320x240)
    #define OSD_SizeX 320
    #define OSD_SizeY 240
#elif (LCM_OPTION == LCM_HX8224_SRGB)
    #define OSD_SizeX 480
    #define OSD_SizeY 240

#elif (LCM_OPTION ==LCM_A015AN04)
    #define OSD_SizeX 94
    #define OSD_SizeY 220

#elif (LCM_OPTION ==LCM_GPG48238QS4)
    #define OSD_SizeX 160
    #define OSD_SizeY 240

#elif (LCM_OPTION ==LCM_TG200Q04)
    #define OSD_SizeX 220
    #define OSD_SizeY 176
#elif (LCM_OPTION == LCM_TD024THEB2)
    #define OSD_SizeX 160
    #define OSD_SizeY 240
#elif (LCM_OPTION == LCM_TD024THEB2_SRGB)
    #define OSD_SizeX 480
    #define OSD_SizeY 240
#elif (LCM_OPTION == LCM_TM024HDH29)
    #define OSD_SizeX 320
    #define OSD_SizeY 240
#elif ((LCM_OPTION == LCM_P_RGB_888_HannStar) || (LCM_OPTION ==LCM_P_RGB_888_ZSX900B50BL) || (LCM_OPTION == LCM_P_RGB_888_FC070227) || (LCM_OPTION == LCM_P_RGB_888_ILI6122))
    #define OSD_SizeX 800
    #define OSD_SizeY 480
#elif (LCM_OPTION == LCM_P_RGB_888_Innolux)
    #define OSD_SizeX 800
    #define OSD_SizeY 600
#elif ((LCM_OPTION == LCM_P_RGB_888_AT070TN90)|| (LCM_OPTION == LCM_P_RGB_888_SY700BE104))
    #define OSD_SizeX 800
    #define OSD_SizeY 480
#elif (LCM_OPTION == LCM_P_RGB_888_ZSX70DT5011)
    #define OSD_SizeX 1024
    #define OSD_SizeY 600
#elif (LCM_OPTION == LCM_P_RGB_PZX090IV042002)
    #define OSD_SizeX 1024
    #define OSD_SizeY 600
#elif (LCM_OPTION == VGA_640X480_60HZ)
    #define OSD_SizeX 640
    #define OSD_SizeY 480
#elif (LCM_OPTION == VGA_800X600_60HZ)
    #define OSD_SizeX 800
    #define OSD_SizeY 600
#elif (LCM_OPTION == VGA_1024X768_60HZ)
    #define OSD_SizeX 1024
    #define OSD_SizeY 768
#elif (LCM_OPTION == VGA_1280X800_60HZ)
    #define OSD_SizeX 800
    #define OSD_SizeY 600
#elif (LCM_OPTION == LCM_sRGB_HD15_HDMI)
    #define OSD_SizeX 1280
    #define OSD_SizeY 720
#endif

 //-----For RFIU use------//
   #define RFI_MPEGDEC_TASK_NONE      0
   #define RFI_MPEGDEC_TASK_RUNNING   1
   #define RFI_MPEGDEC_TASK_SUSPEND   2

   #define RFI_WRAPDEC_TASK_NONE      0
   #define RFI_WRAPDEC_TASK_RUNNING   1
   #define RFI_WRAPDEC_TASK_SUSPEND   2

   #define RFI_RX_TASK_NONE           0
   #define RFI_RX_TASK_RUNNING        1
   #define RFI_RX_TASK_SUSPEND        2


   #define RFI_MPEGENC_TASK_NONE      0
   #define RFI_MPEGENC_TASK_RUNNING   1
   #define RFI_MPEGENC_TASK_SUSPEND   2

   #define RFI_WRAPENC_TASK_NONE      0
   #define RFI_WRAPENC_TASK_RUNNING   1
   #define RFI_WRAPENC_TASK_SUSPEND   2

   #define RFI_TX_TASK_NONE           0
   #define RFI_TX_TASK_RUNNING        1
   #define RFI_TX_TASK_SUSPEND        2

#if (RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL) 
   #define MAX_RFIU_UNIT              8
#else
   #define MAX_RFIU_UNIT              4
#endif   
   #define MAX_RF_DEVICE              MAX_RFIU_UNIT

   #define MAX_PWIFI_UNIT             4
   

   #define PIP_MAIN_NONE              0 // No Picture In Picture function
   #define PIP_MAIN_CH1               1 // PIP enable and CH1 is main channel
   #define PIP_MAIN_CH2               2 // PIP enable and CH2 is main channel

#if (TUTK_SUPPORT)
#if LWIP2_SUPPORT
#if ENABLE_TUTK_RESEND
           #define IOTC_BUF_SIZE ((2*1024*1024 + 4*TUTK_RESEND_BUF_SIZE*1024*MAX_CLIENT)) // 2MB+256K*max_ch*max_clients
#else
           #define IOTC_BUF_SIZE (2 * 1024 * 1024) // 2MB
#endif
#else
   #define IOTC_BUF_SIZE (1024 * 1024) // 1MB
#endif
#endif

   #define GFU_SWQUE_CMDMAX      64
#if VMDSW
   #define VMD_BUF_SIZE         (MEAN_Width)*(MEAN_Height)
   #define VMD_BITBUF_SIZE      (VMD_BUF_SIZE+8)/8 // for 20x15 300bit = 38byte
   #if INTERPOLATION
   #define VMD_BUF_SIZE_INTER   (MEAN_Width_INTER)*(MEAN_Height_INTER)
   #endif
#endif

#define DISPBUF_NORMAL     0
#define DISPBUF_9300FHD    1


/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern u8 *siuRawBuf;

extern u8 *PKBuf;
extern u8 *PKBuf0;
extern u8 *PKBuf1;
extern u8 *PKBuf2;

extern u8 *rfiuOperBuf[MAX_RFIU_UNIT];

extern u8 *rfiuRxVideoBuf[MAX_RFIU_UNIT];
extern u8 *rfiuRxVideoBufEnd[MAX_RFIU_UNIT];

extern u8 *rfiuRxAudioBuf[MAX_RFIU_UNIT];
extern u8 *rfiuRxAudioBufEnd[MAX_RFIU_UNIT];

#if RX_SNAPSHOT_SUPPORT
extern u8 *rfiuRxDataBuf[MAX_RFIU_UNIT];
extern u8 *rfiuRxDataBufEnd[MAX_RFIU_UNIT];
#endif 

#if TX_FW_UPDATE_SUPPORT
extern u8 *rfiuTXFwUpdBuf;
#endif


extern u32 rfiuMainVideoPresentTime[DISPLAY_BUF_NUM];
extern u32 rfiuSub1VideoPresentTime[DISPLAY_BUF_NUM];

extern u32 rfiuVideoTimeBase[MAX_RFIU_UNIT];


extern u32 rfiuMainAudioPresentTime[IISPLAY_BUF_NUM];

extern DEF_RF_MP4DEC_BUF rfiuRxDecBuf[MAX_RFIU_UNIT];


extern u32 rfiuMainVideoTime;
extern u32 rfiuMainVideoTime_frac;
extern u32 rfiuRxMainVideoPlayStart;

extern u32 rfiuSub1VideoTime;
extern u32 rfiuSub1VideoTime_frac;
extern u32 rfiuRxSub1VideoPlayStart;


extern u32 rfiuMainAudioTime;
extern u32 rfiuAudioTimeBase;
extern u32 rfiuMainAudioTime_frac;
extern u32 rfiuRxMainAudioPlayStart;

extern int rfiuVideoInFrameRate;
extern int rfiuTXBitRate[2];
extern int rfiuTxBufFullness[2];

extern u32 rfiuRX_OpMode;
extern u32 rfiuRX_OpModeCmdRetry[MAX_RFIU_UNIT];

extern u32 rfiuRX_P2pVideoQuality;

#if ICOMMWIFI_SUPPORT
extern s32 rfiu_TX_P2pVideoQuality; //RFWIFI_P2P_QUALITY_MEDI
extern s32 rfiu_TX_WifiPower;
extern s32 rfiu_TX_WifiCHNum;

extern s32 Icomm_WifiPower;
extern s32 Icomm_WifiCHNum;
#endif


extern u32 rfiu_TX8BitPCMFmt;

extern u32 rfiuRX_CamOnOff_Sta;
extern u32 rfiuRX_CamPair_Sta;

extern u32 rfiuRX_CamOnOff_Num;
extern u32 rfiuRX_CamPerRF;
extern u32 rfiuRX_CamSleep_Sta;

extern u32 rfiuTxPIR_Trig;

extern u32 rfiuRXWrapSyncErrCnt[MAX_RFIU_UNIT];



extern u32 rfiuVideoBufFill_idx[MAX_RFIU_UNIT];
extern u32 rfiuVideoBufPlay_idx[MAX_RFIU_UNIT];

extern  u8 *PNBuf_sub1[4];
extern  u8 *PNBuf_sub2[4];
extern  u8 *PNBuf_sub3[4];
extern  u8 *PNBuf_sub4[4];
extern  u8 *PNBuf_sub5[4];
extern  u8 *PNBuf_Quad;
extern  u8 *PKBuf_PIP0Y;
extern  u8 *PKBuf_PIP1Y;
extern  u8 *PKBuf_PIP2Y;
extern  u8 *PKBuf_PIP0CbCr;
extern  u8 *PKBuf_PIP1CbCr;
extern  u8 *PKBuf_PIP2CbCr;

extern u8 *ipuDstBuf0;
extern u8 *ipuDstBuf1;
extern u8 *ipuDstBuf2;

#if ( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
extern u8 *MainVideodisplaybuf[DISPLAY_BUF_NUM+2];
#else
extern u8 *MainVideodisplaybuf[DISPLAY_BUF_NUM];
#endif
extern u32 MainVideodisplaybuf_idx;

extern u8 *Sub1Videodisplaybuf[DISPLAY_BUF_NUM];
extern u32 Sub1Videodisplaybuf_idx;

extern u8 *mpeg4MVBuf;

extern u8 * Jpeg_displaybuf[3];

extern u8 *mpeg4PRefBuf_Y;
extern u8 *mpeg4PRefBuf_Cb;
extern u8 *mpeg4PRefBuf_Cr;
extern u8 *mpeg4NRefBuf_Y;
extern u8 *mpeg4NRefBuf_Cb;
extern u8 *mpeg4NRefBuf_Cr;
extern u8 *PNBuf_Y0, *PNBuf_C0, *PNBuf_Y1, *PNBuf_C1, *PNBuf_Y2, *PNBuf_C2;
extern u8 *PNBuf_Y3, *PNBuf_C3;
extern u8 *PNBuf_Y[4], *PNBuf_C[4];

extern u8* mpeg4outputbuf[3];
extern u8* VideoBuf;
extern u8* mpeg4VideBufEnd;
extern u8* mpeg4IndexBuf;
extern u8* P2PPBVideoBuf;
extern u8* P2PPBVideBufEnd;
extern u8* usb_AV_buf;
extern u8* usb_AV_buf_end;

#if(VIDEO_CODEC_OPTION == H264_CODEC)
extern u8 *H264MBPredBuf;
extern u8 *H264ILFPredBuf;
extern u8 *H264IntraPredBuf ;
#endif

extern RTC_DATE_TIME g_LocalTime;
extern u32 g_LocalTimeInSec;
#if UART_GPS_COMMAND
extern u8 g_updatetimeFlag;
#endif

#if (CDVR_LOG || CDVR_TEST_LOG)
extern u8* LogFileBuf;
extern u8* LogFileBufEnd;
#endif

#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
extern  u8* ImaAdpcmBuf;
extern  u8* ImaAdpcmBufEnd;
#endif

#ifdef OPCOM_JPEG_GUI
extern u8* gpGUIJPGFileBuf;
#endif

extern u8 *exifDecBuf;
extern u8 *exifThumbnailBitstream;
extern u8 *exifPrimaryBitstream;


#if ADDAPP3TOJPEG
  extern u8 *exifAPP3VGABitstream;
#endif

#if ADDAPP2TOJPEG
extern DEF_APPENDIXINFO *exifApp2Data;
#endif

extern u8*  usb_device_buf;

#if (USB_HOST == 1)
extern u8*  usb_qh_buf_1;
extern u8*  usb_qh_buf_2;
extern u8*  usb_qh_buf_3;
extern u8*	usb_qtd_buf;
extern u8*  usb_itd_buf_1;
extern u32* usb_Page_buf_0;
#endif

extern u8 *sdcReadBuf;
extern u8 *sdcWriteBuf;

extern u8 *dcfBuf;
#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
extern u8 *dcfLogBuf_Wr;
extern u8 *dcfLogBuf_Rd;
#endif


#if SDC_ECC_DETECT
extern u8 *FatDummyBuf;
#endif
extern u8 *sizeTblBase;
extern u8 *H1test1;
extern u8 *H1test2;

extern u8 *iotcBuf; // for ALL IOTCAPIs use

//extern u8 *iduvideobuff;

extern u8* OSD_buf;
extern u8* SPIConfigBuf;
extern u8* OSD_buf1;
extern u32 OSD_buf1_Size;
extern u8* iduvideobuff;

extern u8 *iisSounBuf[IIS_BUF_NUM];
#if TUTK_SUPPORT
extern u8 *p2plocal_buffer;
extern u8 *p2pbusylocal_buffer;
//for event list use. by aher
extern u8 *p2pEventList;
#endif

#if PIC_OP
    extern u8* picTmpBuf;
#endif

/*------------Declare Global Variable for control------------*/
extern u8  isuStatus_OnRunning;
extern s8  sysCameraMode;

extern u8 sysInsertAPP3_ena; //Lucian: 決定是否加入APP3 to JPEG file
extern u8 sysCaptureVideoStart;
extern u8 sysCaptureVideoStop;
extern u8 sysPowerOffFlag;
extern u8 sysTVOutOnFlag;
extern u8 sysVoiceRecStart;
extern u8 sysVoiceRecStop;
extern u8 sysVoicePlayStop;
extern u8 sysVolumnControl;
extern u8 sysIsAC97Playing;
extern u32 sysPlayBeepFlag;

extern u8 sysVideoInCHsel;
extern u8 sysDualModeDisp;
#if HWPIP_SUPPORT
extern u8 sysOsdInCHsel;
#endif


extern u8 sysRFRxInMainCHsel;
extern u8 sysRFRxInPIPCHsel;  //for PIP sub-channel
extern u8 sysRFRXPIP_en;


extern u8 sysRF_PTZ_CHsel;
extern u8 sysRF_AudioChSw_DualMode;

extern u8 sysPIPMain;

#if IIS_TEST
  extern u8 *iisBuf_play;
  extern u8 *iisBuf_play1;
#elif IIS_4CH_REC_TEST
  extern u8 *iisBuf_play;
  extern u8 *iisBuf_play1;
  extern u8 *iisBuf_play2;
  extern u8 *iisBuf_play3;
#endif

#if GET_SIU_RAWDATA_PURE
  extern s16 siuAdjustSW;
  extern s16 siuAdjustAGC;
#endif

/*Declare Event Variable*/
extern OS_EVENT* isuSemEvt;
extern u32 OS_tickcounter;

extern u32 gfuSwQueWrIdx;
extern u32 gfuSwQueRdIdx;
extern u8 *gfuSwQueAddr;

//===============================//
extern int memDispBufArrage(int mode);
//===============================//
#if TX_SNAPSHOT_SUPPORT
extern u8 *sysRFTXImgData;
extern u32 sysRFTXDataSize;
#endif


#endif
