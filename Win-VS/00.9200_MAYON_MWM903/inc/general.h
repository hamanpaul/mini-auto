/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    general.h

Abstract:

    The general declaration of types and contants.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#ifndef __GENERAL_H__
#define __GENERAL_H__

/* general include file */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#define OS_APIs

/*---------------- type definition ------------------*/
typedef uint8_t u8;     /* Unsigned  8 bit quantity                     */
typedef int8_t s8;      /* Signed    8 bit quantity                     */
typedef uint16_t u16;   /* Unsigned 16 bit quantity                     */
typedef int16_t s16;    /* Signed   16 bit quantity                     */
typedef uint32_t u32;   /* Unsigned 32 bit quantity                     */
typedef int32_t s32;    /* Signed   32 bit quantity                     */
typedef int64_t s64;    /* Signed   64 bit quantity                     */
typedef float f32;      /* Single precision floating point              */
typedef double d64;     /* Double precision floating point              */

typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint8_t BYTE;
typedef signed long LONG;

#define FARCHARPTR char *

typedef __packed struct _u64
{
    u32 lo;
    u32 hi;
} u64;

typedef __packed struct _u128
{
    u8  octet[16];
} u128;

typedef __packed struct _s128
{
    s8  octet[16];
} s128;

typedef struct _RATIONAL
{
    u32 numerator;
    u32 denominator;
} RATIONAL;

typedef struct _SRATIONAL
{
    s32 numerator;
    s32 denominator;
} SRATIONAL;

/* Base Type */
typedef struct _RTC_DATE_TIME
{
    u8	year;	/* year since 2000 - up to 2063 */
    u8	month;	/* 1 - 12 */
    u8 	day;	/* 1 - 31 */
    u8	hour;	/* 0 - 23 */
    u8	min;	/* 0 - 59 */
    u8	sec;	/* 0 - 59 */
    u8	week;	/* 0 - 6 day of week*/
} RTC_DATE_TIME;

typedef struct _REC_TIME
{
    u8	min;	/* 0 - 99 */
    u8	sec;	/* 0 - 59 */
} REC_TIME;

/* constant definition */
#define TRUE                1
#define FALSE               0

/* macro definition */
#define BSWAP16(data)       ((((data) & 0xff00) >> 8 ) | (((data) & 0xff) << 8))
#define BSWAP32(data)       ((((data) & 0x000000ff) << 24) | \
                             (((data) & 0x0000ff00) << 8)  | \
                             (((data) & 0x00ff0000) >> 8)  | \
                             (((data) & 0xff000000) >> 24))

/* function prototype */
extern u16 bSwap16(u16);
extern u32 bSwap32(u32);
extern u64 bSwap64(u64);

/* system option */ /*CY 1023*/
#include "sysopt.h"

/* camera mode */
#define SYS_CAMERA_MODE_PREVIEW        0x00
#define SYS_CAMERA_MODE_PLAYBACK       0x01
#define SYS_CAMERA_MODE_UNKNOWN        0x02
#define SYS_CAMERA_MODE_CAPTURE        0x03

#define SYS_CAMERA_MODE_RF_RX_FULLSCR  0x04
#define SYS_CAMERA_MODE_RF_RX_QUADSCR  0x05
#define SYS_CAMERA_MODE_RF_RX_DUALSCR  0x06
#define SYS_CAMERA_MODE_RF_RX_MASKAREA 0x07

#define SYS_CAMERA_MODE_CIU_QUADSCR    0x08
#define SYS_CAMERA_MODE_GFU_TESTSCR    0x09 //for GFU test only

#define SYS_CAMERA_MODE_UI    		   0x0A

//======TV format====//
#define SYS_TV_OUT_NTSC             0x00
#define SYS_TV_OUT_PAL              0x01

#define SYS_TV_OUT_HD720P60         0x02
#define SYS_TV_OUT_HD720P30         0x03
#define SYS_TV_OUT_HD720P25         0x04

#define SYS_TV_OUT_FHD1080I60       0x05
#define SYS_TV_OUT_FHD1080P30       0x06
#define SYS_TV_OUT_FHD1080P25       0x08

#define SYS_TV_OUT_HD720P60_37M     0x09   //Lucian: 用720P60 timing, 但clock(減半)37.125MHz, fps=30

#define TVOUT_OSDx1_VDOx1   0
#define TVOUT_OSDx2_VDOx1   1 // A1018B不使用 改用OSD BRI
#define TVOUT_OSDx1_VDOx2   2
#define TVOUT_OSDx2_VDOx2   3

#define SYS_RUN_PREVIEW     0
#define SYS_RUN_PLAYBACK    1

#define GLB_DISA			0
#define GLB_ENA				1

#define SYS_OUTMODE_PANEL	0
#define SYS_OUTMODE_TV		1


//=======Main Storage select====//
#define SYS_MAINSTORAGE_SDC       0
#define SYS_MAINSTORAGE_USBMASS   1


// define SIU operation mode
#define SIUMODE_START         0
#define SIUMODE_PREVIEW       1
#define SIUMODE_CAPTURE       2
#define SIUMODE_MPEGAVI       3
#define SIUMODE_MPEGAVI_ZOOM  4
#define SIUMODE_PREVIEW_ZOOM  5
#define SIUMODE_CAP_RREVIEW   6
#define SIUMODE_PREVIEW_MENU  7 // Preview in Menu mode

/* define Jpeg operation mode */
//#define JPEG_OPMODE_FRAME 0
//#define JPEG_OPMODE_SLICE 1



/* For memory pool */ /*Lucian 070419*/

#define ISU_TIMEOUT     30  /*CY 1023*/

#undef JUST_UPDATE_PICT        //define this will just update PICT without code


//--------Define CDRV director name--------//
#if (FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR)
#define GS_DCFDIRNAME "MFG"
#elif(FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)
#define GS_DCFDIRNAME "MFG"
#endif

#if (CDVR_iHome_LOG_SUPPORT || CDVR_SYSTEM_LOG_SUPPORT)
#define GS_DCF_LOGDIR_NAME "LOG"
#endif

#if RX_SNAPSHOT_SUPPORT
#define GS_DCF_PHOTODIR_NAME "PHOTO"
#endif
//--------------Debug Message ON/OFF-------------------//
#define ALL_DEBUG       0
//-----------------------//
#if(FPGA_BOARD_A1018_SERIES)
#define MAIN_DEBUG      1
#elif(HW_BOARD_OPTION == A1025A_EVB)
#define MAIN_DEBUG      1
#elif(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
#define MAIN_DEBUG      1
#elif( (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) )
#define MAIN_DEBUG      1

#else
#define MAIN_DEBUG      0
#endif
#define SPI_DEBUG       1

#define UI_DEBUG        1
#define UI_DEBUG2       0
#define MARSS_DEBUG     0
#define CIU_DEBUG       1
#define ISP_DEBUG       1
#define IIS_DEBUG       1
//-----------------------//
#define ADC_DEBUG       1

#define ASF_DEBUG       1
#define ASF_DEBUG2      0


#define AVI_DEBUG       1

#define DCF_DEBUG       1
#if( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
#define DCF_DEBUG2      1
#else
#define DCF_DEBUG2      1
#endif


#define DMA_DEBUG       1
#define DPOF_DEBUG      1

#define FS_DEBUG        1
#define FS_DEBUG2       0

#define GPIO_DEBUG      1
#define HIU_DEBUG       1
#define I2C_DEBUG       1
#define IDU_DEBUG       1
#define IPU_DEBUG       1
#define ISU_DEBUG       0
#define JPEG_DEBUG      1
#define MP4_DEBUG       1
#define H264_DEBUG      1
#define RTC_DEBUG       1
#define SDC_DEBUG       1
#define SIU_DEBUG       1
#define SYS_DEBUG       1
#if( (SW_APPLICATION_OPTION == MR9100_RF_DONGLE_AVSED_RX1RX2_8CH) || (SW_APPLICATION_OPTION == MR9100_WIFI_DONGLE_AVSED_RX1) )
#define USB_DEBUG       1
#else
#define USB_DEBUG       0
#endif
#define SMC_DEBUG       1
#define TMP_DEBUG       1
#define SDRAM_DEBUG     1
#define UART_DEBUG      1
#define HIU_DEBUG       1
#define CF_DEBUG        1
#define RFIU_DEBUG      1
#define GPIU_DEBUG      1
#define MCPU_DEBUG      1
#define WEB_DEBUG       1
#define BOARD_DEBUG     1
#define TIMER_DEBUG     1
#if( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
#define UHOST_DEBUG     1
#else
#define UHOST_DEBUG     0
#endif
#define UHUB_DEBUG      1
#define USTORAGE_DEBUG  0
#define P2P_DEBUG       1
#define ENCRYPT_DEBUG   1
#define HDMI_DEBUG      1
#define WARERR_DEBUG    1

#if(WARERR_DEBUG || ALL_DEBUG)
#define DEBUG_WARERR(fmt...)   printf(fmt)
#else
#define DEBUG_WARERR(fmt...)   //do { } while (0)
#endif


#if(ADC_DEBUG || ALL_DEBUG)
#define DEBUG_ADC(fmt...)   printf(fmt)
#else
#define DEBUG_ADC(fmt...)   //do { } while (0)
#endif

#if(ASF_DEBUG || ALL_DEBUG)
#define DEBUG_ASF(fmt...)   printf(fmt)
#else
#define DEBUG_ASF(fmt...)   //do { } while (0)
#endif

#if(ASF_DEBUG2 || ALL_DEBUG)
#define DEBUG_ASF2(fmt...)   printf(fmt)
#else
#define DEBUG_ASF2(fmt...)   //do { } while (0)
#endif



#if(AVI_DEBUG || ALL_DEBUG)
#define DEBUG_AVI(fmt...)       printf(fmt)
#else
#define DEBUG_AVI(fmt...)       //do { } while (0)
#endif

#if(DCF_DEBUG || ALL_DEBUG)
#define DEBUG_DCF(fmt...)   printf(fmt)
#else
#define DEBUG_DCF(fmt...)   //do { } while (0)
#endif

#if(DCF_DEBUG2 || ALL_DEBUG)
#define DEBUG_DCF2(fmt...)   printf(fmt)
#else
#define DEBUG_DCF2(fmt...)   //do { } while (0)
#endif

#if(DMA_DEBUG || ALL_DEBUG)
#define DEBUG_DMA(fmt...)   printf(fmt)
#else
#define DEBUG_DMA(fmt...)   //do { } while (0)
#endif

#if(DPOF_DEBUG || ALL_DEBUG)
#define DEBUG_DPOF(fmt...)  printf(fmt)
#else
#define DEBUG_DPOF(fmt...)  //do { } while (0)
#endif

#if(FS_DEBUG || ALL_DEBUG)
#define DEBUG_FS(fmt...)        printf(fmt)
#else
#define DEBUG_FS(fmt...)        //do { } while (0)
#endif

#if(FS_DEBUG2 || ALL_DEBUG)
#define DEBUG_FS2(fmt...)        printf(fmt)
#else
#define DEBUG_FS2(fmt...)        //do { } while (0)
#endif


#if(GPIO_DEBUG || ALL_DEBUG)
#define DEBUG_GPIO(fmt...)  printf(fmt)
#else
#define DEBUG_GPIO(fmt...)  //do { } while (0)
#endif

#if(HIU_DEBUG || ALL_DEBUG)
#define DEBUG_HIU(fmt...)       printf(fmt)
#else
#define DEBUG_HIU(fmt...)       //do { } while (0)
#endif

#if(I2C_DEBUG || ALL_DEBUG)
#define DEBUG_I2C(fmt...)       printf(fmt)
#else
#define DEBUG_I2C(fmt...)       //do { } while (0)
#endif

#if(IDU_DEBUG || ALL_DEBUG)
#define DEBUG_IDU(fmt...)       printf(fmt)
#else
#define DEBUG_IDU(fmt...)       //do { } while (0)
#endif

#if(IIS_DEBUG || ALL_DEBUG)
#define DEBUG_IIS(fmt...)       printf(fmt)
#else
#define DEBUG_IIS(fmt...)       //do { } while (0)
#endif

#if(IPU_DEBUG || ALL_DEBUG)
#define DEBUG_IPU(fmt...)       printf(fmt)
#else
#define DEBUG_IPU(fmt...)       //do { } while (0)
#endif

#if(ISP_DEBUG || ALL_DEBUG)
#define DEBUG_ISP(fmt...)       printf(fmt)
#else
#define DEBUG_ISP(fmt...)       //do { } while (0)
#endif

#if(ISU_DEBUG || ALL_DEBUG)
#define DEBUG_ISU(fmt...)       printf(fmt)
#else
#define DEBUG_ISU(fmt...)       //do { } while (0)
#endif

#if(JPEG_DEBUG || ALL_DEBUG)
#define DEBUG_JPEG(fmt...)  printf(fmt)
#else
#define DEBUG_JPEG(fmt...)  //do { } while (0)
#endif

#if(MAIN_DEBUG || ALL_DEBUG)
#define DEBUG_MAIN(fmt...)  printf(fmt)
#else
#define DEBUG_MAIN(fmt...)  //do { } while (0)
#endif

#if(MP4_DEBUG || ALL_DEBUG)
#define DEBUG_MP4(fmt...)   printf(fmt)
#else
#define DEBUG_MP4(fmt...)   //do { } while (0)
#endif

#if(H264_DEBUG || ALL_DEBUG)
#define DEBUG_H264(fmt...)   printf(fmt)
#else
#define DEBUG_H264(fmt...)   //do { } while (0)
#endif

#if(RTC_DEBUG || ALL_DEBUG)
#define DEBUG_RTC(fmt...)       printf(fmt)
#else
#define DEBUG_RTC(fmt...)       //do { } while (0)
#endif

#if(SDC_DEBUG || ALL_DEBUG)
#define DEBUG_SDC(fmt...)   printf(fmt)
#else
#define DEBUG_SDC(fmt...)   //do { } while (0)
#endif

#if(SIU_DEBUG || ALL_DEBUG)
#define DEBUG_SIU(fmt...)       printf(fmt)
#else
#define DEBUG_SIU(fmt...)       //do { } while (0)
#endif

#if(CIU_DEBUG || ALL_DEBUG)
#define DEBUG_CIU(fmt...)       printf(fmt)
#else
#define DEBUG_CIU(fmt...)       //do { } while (0)
#endif

#if(UI_DEBUG || ALL_DEBUG)
#define DEBUG_UI(fmt...)        printf(fmt)
#else
#define DEBUG_UI(fmt...)        //do { } while (0)
#endif

#if(MARSS_DEBUG || ALL_DEBUG)
#define DEBUG_MARSS(fmt...)        printf(fmt)
#else
#define DEBUG_MARSS(fmt...)        //do { } while (0)
#endif

#if(UI_DEBUG2 || ALL_DEBUG)
#define DEBUG_UI2(fmt...)        printf(fmt)
#else
#define DEBUG_UI2(fmt...)        //do { } while (0)
#endif


#if(SYS_DEBUG || ALL_DEBUG)
#define DEBUG_SYS(fmt...)       printf(fmt)
#else
#define DEBUG_SYS(fmt...)       //do { } while (0)
#endif

#if(USB_DEBUG || ALL_DEBUG)
#define DEBUG_USB(fmt...)   printf(fmt)
#else
#define DEBUG_USB(fmt...)       //do { } while (0)
#endif

#if(SMC_DEBUG || ALL_DEBUG)
#define DEBUG_SMC(fmt...)   printf(fmt)
#else
#define DEBUG_SMC(fmt...)   //do { } while (0)
#endif

#if(TMP_DEBUG || ALL_DEBUG)
#define DEBUG_TMP(fmt...)   printf(fmt)
#else
#define DEBUG_TMP(fmt...)   //do { } while (0)
#endif

#if(SDRAM_DEBUG || ALL_DEBUG)
#define DEBUG_SDRAM(fmt...)   printf(fmt)
#else
#define DEBUG_SDRAM(fmt...)   //do { } while (0)
#endif

#if(SPI_DEBUG || ALL_DEBUG)
#define DEBUG_SPI(fmt...)   printf(fmt)
#else
#define DEBUG_SPI(fmt...)   //do { } while (0)
#endif

#if(UART_DEBUG || ALL_DEBUG)
#define DEBUG_UART(fmt...)	printf(fmt)
#else
#define DEBUG_UART(fmt...)	//do { } while (0)
#endif

#if(HIU_DEBUG || ALL_DEBUG)
#define DEBUG_HIU(fmt...)	printf(fmt)
#else
#define DEBUG_HIU(fmt...)	//do { } while (0)
#endif

#if(CF_DEBUG || ALL_DEBUG)
#define DEBUG_CF(fmt...)   printf(fmt)
#else
#define DEBUG_CF(fmt...)   //do { } while (0)
#endif

#if(RFIU_DEBUG || ALL_DEBUG)
#define DEBUG_RFIU(fmt...)     //printf(fmt)
#else
#define DEBUG_RFIU(fmt...)   //do { } while (0)
#endif

#if(GPIU_DEBUG || ALL_DEBUG)
#define DEBUG_GPIU(fmt...)   printf(fmt)
#else
#define DEBUG_GPIU(fmt...)   //do { } while (0)
#endif

#if(MCPU_DEBUG || ALL_DEBUG)
#define DEBUG_MCPU(fmt...)   printf(fmt)
#else
#define DEBUG_MCPU(fmt...)   //do { } while (0)
#endif

#if(WEB_DEBUG || ALL_DEBUG)
#define DEBUG_WEB(fmt...)   printf(fmt)
#else
#define DEBUG_WEB(fmt...)   //do { } while (0)
#endif

#if(BOARD_DEBUG || ALL_DEBUG)
#define DEBUG_BOARD(fmt...)   printf(fmt)
#else
#define DEBUG_BOARD(fmt...)   //do { } while (0)
#endif

#if(TIMER_DEBUG || ALL_DEBUG)
#define DEBUG_TIMER(fmt...)   printf(fmt)
#else
#define DEBUG_TIMER(fmt...)   //do { } while (0)
#endif

#if (UHOST_DEBUG || ALL_DEBUG)
#define DEBUG_UHOST(fmt...)   printf(fmt)
#else
#define DEBUG_UHOST(fmt...)   //do { } while (0)
#endif

#if (USTORAGE_DEBUG || ALL_DEBUG)
#define DEBUG_STORAGE(fmt...)   printf(fmt)
#else
#define DEBUG_STORAGE(fmt...)   //do { } while (0)
#endif

#if (UHUB_DEBUG || ALL_DEBUG)
#define DEBUG_UHUB(fmt...)   printf(fmt)
#else
#define DEBUG_UHUB(fmt...)   //do { } while (0)
#endif

#if(P2P_DEBUG || ALL_DEBUG)
#define DEBUG_P2P(fmt...)   printf(fmt)
#else
#define DEBUG_P2P(fmt...)   //do { } while (0)
#endif

#if(ENCRYPT_DEBUG || ALL_DEBUG)
#define DEBUG_ENCRYPT(fmt...)   printf(fmt)
#else
#define DEBUG_ENCRYPT(fmt...)   //do { } while (0)
#endif

#if(HDMI_DEBUG || ALL_DEBUG)
#define DEBUG_HDMI(fmt...)   printf(fmt)
#else
#define DEBUG_HDMI(fmt...)   //do { } while (0)
#endif

#define DEBUG_RED(fmt...)		do{printf("\x1B[31m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_GREEN(fmt...)		do{printf("\x1B[32m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_YELLOW(fmt...)	do{printf("\x1B[33m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_BLUE(fmt...)		do{printf("\x1B[34m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_MAGENTA(fmt...)	do{printf("\x1B[35m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_CYAN(fmt...)		do{printf("\x1B[36m");printf(fmt);printf("\x1B[0m");}while(0)

#define DEBUG_LIGHTGRAY(fmt...)		do{printf("\x1B[37m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_DARKGARY(fmt...)		do{printf("\x1B[90m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_LIGHTRED(fmt...)		do{printf("\x1B[91m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_LIGHTGREEN(fmt...)	do{printf("\x1B[92m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_LIGHTYELLOW(fmt...)	do{printf("\x1B[93m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_LIGHTBLUE(fmt...)		do{printf("\x1B[94m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_LIGHTMAGENTA(fmt...)	do{printf("\x1B[95m");printf(fmt);printf("\x1B[0m");}while(0)
#define DEBUG_LIGHTCYAN(fmt...)		do{printf("\x1B[96m");printf(fmt);printf("\x1B[0m");}while(0)


#define DEBUG_SHOW_START() 		printf("\x1B[31m")
#define DEBUG_SHOW_END() 		printf("\x1B[0m")

#include "AWBdef.h"
#include "osapi.h"
#include "MemoryPool.h"

extern void PrintBin (u32 hex);
extern u8* hex2bin(char ch);

#endif
