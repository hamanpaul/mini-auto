/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    jpeg.c

Abstract:

    The routines of JPEG encoder/decoder.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "board.h"
#include "jpeg.h"
#include "jpegreg.h"
#include "jpegapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "sysapi.h"
#include "siuapi.h"
#if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
#include "task.h"       //Lsk 030912
#include "mpeg4api.h"	//Lsk 030912
#include "asfapi.h"		//Lsk 030912
#include "isuapi.h"		//Lsk 030912
#include "osapi.h"		//Lsk 030912
#endif
#define JPEG_FORMAT_422     0
#define JPEG_FORMAT_420     1

#define JPEG_SYSCTL_RST     0x00000080

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
//#define jpegDebugPrint            printf

#define JPEG_TIMEOUT            100  /*CY 1023*/
#define JPEG_DEBLOCK_DIV        4
#define JPEG_TIME_STATISTIC     0   // JPEG process time statistic, 0: disable, 1: enable.

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

OS_EVENT* jpegSemEvt;
extern u8 siuOpMode;
extern u8 ResetPlayback;
extern u32 IsuIndex;
extern s64 Videodisplaytime[DISPLAY_BUF_NUM];
u8  jpegSemDec_flag;/*BJ 0523 S*/
u8  jpegDecErr_flag;
u32 JpegCtrlTemp;   // for fix PA9002C bug, Peter 071018
u32 MJPEG_Pend;

u8 Debug_jpeg_en; //Amon 20150709	

#if (VIDEO_CODEC_OPTION == MJPEG_CODEC)
u32 MJPG_Task_Go;
/* Lsk: Motion-JPEG Part begin*/
u32 MJPG_Mode;    					//0: record, 1: playback
u8 ReadOnce;
/* task and event related */
OS_STK mjpgTaskStack[MJPG_TASK_STACK_SIZE];
#endif

/*---- Quantization table ----*/
/* Q98 */
/* Luminance */
const u8 jpegDqtLumQ98[64] =
{
    0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,
    0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x01,  0x02,
    0x01,  0x01,  0x01,  0x01,  0x01,  0x02,  0x02,  0x03,
    0x01,  0x02,  0x03,  0x03,  0x03,  0x03,  0x03,  0x02,
    0x04,  0x03,  0x03,  0x05,  0x05,  0x04,  0x03,  0x04,
    0x04,  0x03,  0x03,  0x03,  0x04,  0x06,  0x04,  0x04,
    0x05,  0x05,  0x05,  0x05,  0x06,  0x03,  0x04,  0x06,
    0x06,  0x05,  0x05,  0x06,  0x05,  0x05,  0x05,  0x05,
};
/* Chrominance */
const u8 jpegDqtChrQ98[64] =
{
    0x01,  0x01,  0x01,  0x02,  0x01,  0x02,  0x04,  0x02,
    0x02,  0x04,  0x06,  0x04,  0x03,  0x04,  0x06,  0x0b,
    0x08,  0x04,  0x04,  0x08,  0x0b,  0x0b,  0x0b,  0x0b,
    0x05,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,
    0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,
    0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,
    0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,
    0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,  0x0b,
};

/* Q95 */
/* Luminance */
const u8 jpegDqtLumQ95[64] =
{
     2,   1,   1,   2,   2,   4,   5,   6,
     1,   1,   1,   2,   3,   6,   6,   6,
     1,   1,   2,   2,   4,   6,   7,   6,
     1,   2,   2,   3,   5,   9,   8,   6,
     2,   2,   4,   6,   7,  11,  10,   8,
     2,   4,   6,   6,   8,  10,  11,   9,
     5,   6,   8,   9,  10,  12,  12,  10,
     7,   9,  10,  10,  11,  10,  10,  10,
};
/* Chrominance */
const u8 jpegDqtChrQ95[64] =
{
     2,   2,   2,   5,  10,  10,  10,  10,
     2,   2,   3,   7,  10,  10,  10,  10,
     2,   3,   6,  10,  10,  10,  10,  10,
     5,   7,  10,  10,  10,  10,  10,  10,
    10,  10,  10,  10,  10,  10,  10,  10,
    10,  10,  10,  10,  10,  10,  10,  10,
    10,  10,  10,  10,  10,  10,  10,  10,
    10,  10,  10,  10,  10,  10,  10,  10,
};

/* Q92 */
/* Luminance */
const u8 jpegDqtLumQ92[64] =
{
     3,   2,   2,   3,   4,   6,   8,  10,
     2,   2,   2,   3,   4,   9,  10,   9,
     2,   2,   3,   4,   6,   9,  11,   9,
     2,   3,   4,   5,   8,  14,  13,  10,
     3,   4,   6,   9,  11,  17,  16,  12,
     4,   6,   9,  10,  13,  17,  18,  15,
     8,  10,  12,  14,  16,  19,  19,  16,
    12,  15,  15,  16,  18,  16,  16,  16,
};
/* Chrominance */
const u8 jpegDqtChrQ92[64] =
{
     3,   3,   4,   8,  16,  16,  16,  16,
     3,   3,   4,  11,  16,  16,  16,  16,
     4,   4,   9,  16,  16,  16,  16,  16,
     8,  11,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
    16,  16,  16,  16,  16,  16,  16,  16,
};

/* Q90 */
/* Luminance */
const u8 jpegDqtLumQ90[64] =
{
     3,   2,   2,   3,   5,   8,  10,  12,
     2,   2,   3,   4,   5,  12,  12,  11,
     3,   3,   3,   5,   8,  11,  14,  11,
     3,   3,   4,   6,  10,  17,  16,  12,
     4,   4,   7,  11,  14,  22,  21,  15,
     5,   7,  11,  13,  16,  21,  23,  18,
    10,  13,  16,  17,  21,  24,  24,  20,
    14,  18,  19,  20,  22,  20,  21,  20,
};
/* Chrominance */
const u8 jpegDqtChrQ90[64] =
{
     3,   4,   5,   9,  20,  20,  20,  20,
     4,   4,   5,  13,  20,  20,  20,  20,
     5,   5,  11,  20,  20,  20,  20,  20,
     9,  13,  20,  20,  20,  20,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20,
    20,  20,  20,  20,  20,  20,  20,  20,
};

/* Q88 */
/* Luminance */
const u8 jpegDqtLumQ88[64] =
{
     4,   3,   2,   4,   6,  10,  12,  15,
     3,   3,   3,   5,   6,  14,  14,  13,
     3,   3,   4,   6,  10,  14,  17,  13,
     3,   4,   5,   7,  12,  21,  19,  15,
     4,   5,   9,  13,  16,  26,  25,  18,
     6,   8,  13,  15,  19,  25,  27,  22,
    12,  15,  19,  21,  25,  29,  29,  24,
    17,  22,  23,  24,  27,  24,  25,  24,
};
/* Chrominance */
const u8 jpegDqtChrQ88[64] =
{
     4,   4,   6,  11,  24,  24,  24,  24,
     4,   5,   6,  16,  24,  24,  24,  24,
     6,   6,  13,  24,  24,  24,  24,  24,
    11,  16,  24,  24,  24,  24,  24,  24,
    24,  24,  24,  24,  24,  24,  24,  24,
    24,  24,  24,  24,  24,  24,  24,  24,
    24,  24,  24,  24,  24,  24,  24,  24,
    24,  24,  24,  24,  24,  24,  24,  24,
};

/* Q86 */
/* Luminance */
const u8 jpegDqtLumQ86[64] =
{
    0x01,  0x01,  0x01,  0x02,  0x01,  0x01,  0x02,  0x02,
    0x02,  0x02,  0x03,  0x02,  0x02,  0x03,  0x03,  0x06,
    0x04,  0x03,  0x03,  0x03,  0x03,  0x07,  0x05,  0x08,
    0x04,  0x06,  0x08,  0x08,  0x0a,  0x09,  0x08,  0x07,
    0x0b,  0x08,  0x0a,  0x0e,  0x0d,  0x0b,  0x0a,  0x0a,
    0x0c,  0x0a,  0x08,  0x08,  0x0b,  0x10,  0x0c,  0x0c,
    0x0d,  0x0f,  0x0f,  0x0f,  0x0f,  0x09,  0x0b,  0x10,
    0x11,  0x0f,  0x0e,  0x11,  0x0d,  0x0e,  0x0e,  0x0e,
};
/* Chrominance */
const u8 jpegDqtChrQ86[64] =
{
    0x04,  0x04,  0x04,  0x05,  0x04,  0x05,  0x09,  0x05,
    0x05,  0x09,  0x0f,  0x0a,  0x08,  0x0a,  0x0f,  0x1a,
    0x13,  0x09,  0x09,  0x13,  0x1a,  0x1a,  0x1a,  0x1a,
    0x0d,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,
    0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,
    0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,
    0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,
    0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,
};

/* Q85 */
/* Luminance */
const u8 jpegDqtLumQ85[64] =
{
     5,   3,   3,   5,   7,  12,  15,  18,
     4,   4,   4,   6,   8,  17,  18,  17,
     4,   4,   5,   7,  12,  17,  18,  17,
     4,   5,   7,   9,  15,  26,  24,  19,
     5,   7,  11,  17,  20,  33,  31,  23,
     7,  11,  17,  19,  24,  31,  34,  28,
    15,  19,  23,  26,  31,  36,  36,  30,
    22,  28,  29,  29,  34,  30,  31,  30,
};
/* Chrominance */
const u8 jpegDqtChrQ85[64] =
{
     5,   5,   7,  14,  30,  30,  30,  30,
     5,   6,   8,  20,  30,  30,  30,  30,
     7,   8,  17,  30,  30,  30,  30,  30,
    14,  20,  30,  30,  30,  30,  30,  30,
    30,  30,  30,  30,  30,  30,  30,  30,
    30,  30,  30,  30,  30,  30,  30,  30,
    30,  30,  30,  30,  30,  30,  30,  30,
    30,  30,  30,  30,  30,  30,  30,  30,
};

/* Q80 */
/* Luminance */
const u8 jpegDqtLumQ80[64] =
{
     6,   4,   4,   6,  10,  16,  20,  24,
     5,   5,   6,   8,  10,  23,  24,  22,
     6,   5,   6,  10,  16,  23,  28,  22,
     6,   7,   9,  12,  20,  35,  32,  25,
     7,   9,  15,  22,  27,  44,  41,  31,
    10,  14,  22,  26,  32,  42,  45,  37,
    20,  26,  31,  35,  41,  48,  48,  40,
    29,  37,  38,  39,  45,  40,  41,  40,
};
/* Chrominance */
const u8 jpegDqtChrQ80[64] =
{
     7,   7,  10,  19,  40,  40,  40,  40,
     7,   8,  10,  26,  40,  40,  40,  40,
    10,  10,  22,  40,  40,  40,  40,  40,
    19,  26,  40,  40,  40,  40,  40,  40,
    40,  40,  40,  40,  40,  40,  40,  40,
    40,  40,  40,  40,  40,  40,  40,  40,
    40,  40,  40,  40,  40,  40,  40,  40,
    40,  40,  40,  40,  40,  40,  40,  40,
};

/* Q70 */
/* Luminance */
const u8 jpegDqtLumQ70[64] =
{
    10,   7,   6,  10,  14,  24,  31,  37,
     7,   7,   8,  11,  16,  35,  36,  33,
     8,   8,  10,  14,  24,  34,  41,  34,
     8,  10,  13,  17,  31,  52,  48,  37,
    11,  13,  22,  34,  41,  65,  62,  46,
    14,  21,  33,  38,  49,  62,  68,  55,
    29,  38,  47,  52,  62,  73,  72,  61,
    43,  55,  57,  59,  67,  60,  62,  59,
 };
/* Chrominance */
const u8 jpegDqtChrQ70[64] =
{
    10,  11,  14,  28,  59,  59,  59,  59,
    11,  13,  16,  40,  59,  59,  59,  59,
    14,  16,  34,  59,  59,  59,  59,  59,
    28,  40,  59,  59,  59,  59,  59,  59,
    59,  59,  59,  59,  59,  59,  59,  59,
    59,  59,  59,  59,  59,  59,  59,  59,
    59,  59,  59,  59,  59,  59,  59,  59,
    59,  59,  59,  59,  59,  59,  59,  59,
};

/* Q60 */
/* Luminance */
const u8 jpegDqtLumQ60[64] =
{
    0x09,  0x06,  0x06,  0x08,  0x06,  0x05,  0x09,  0x08,
    0x07,  0x08,  0x0a,  0x09,  0x09,  0x0b,  0x0d,  0x16,
    0x0f,  0x0d,  0x0c,  0x0c,  0x0d,  0x1c,  0x13,  0x15,
    0x10,  0x16,  0x21,  0x1d,  0x23,  0x22,  0x21,  0x1c,
    0x20,  0x1f,  0x24,  0x29,  0x34,  0x2c,  0x24,  0x27,
    0x31,  0x27,  0x1e,  0x1f,  0x2d,  0x3d,  0x2d,  0x31,
    0x36,  0x37,  0x3a,  0x3a,  0x3a,  0x22,  0x2a,  0x3f,
    0x44,  0x3e,  0x38,  0x42,  0x33,  0x37,  0x39,  0x36,
};
/* Chrominance */
const u8 jpegDqtChrQ60[64] =
{
    0x09,  0x09,  0x09,  0x0c,  0x0a,  0x0c,  0x14,  0x0c,
    0x0c,  0x14,  0x0f,  0x0a,  0x0a,  0x0a,  0x0f,  0x1a,
    0x1a,  0x0a,  0x0a,  0x1a,  0x1a,  0x4f,  0x1a,  0x1a,
    0x1a,  0x1a,  0x1a,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,
    0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,
    0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,
    0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,
    0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,  0x4f,
};

/* Q50 */
/* Luminance */
const u8 jpegDqtLumQ50[64] =
{
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68, 109, 103,  77,
    24,  35,  55,  64,  81, 104, 113,  92,
    49,  64,  78,  87, 103, 121, 120, 101,
    72,  92,  95,  98, 112, 100, 103,  99,
};
/* Chrominance */
const u8 jpegDqtChrQ50[64] =
{
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
};

/* Q40 */
/* Luminance */
const u8 jpegDqtLumQ40[64] =
{
    20, 13,  12,  20,  30,  50,  63,  76,
    15, 15,  17,  23,  32,  72,  75,  68,
    17, 16,  20,  30,  50,  71,  86,  70,
    17, 21,  27,  36,  63,  108, 100, 77,
    22, 27,  46,  70,  85,  136, 128, 96,
    30, 43,  68,  80,  101, 130, 141, 115,
    61, 80,  97,  108, 128, 151, 150, 126,
    90, 115, 118, 122, 140, 125, 128, 123,
};
/* Chrominance */
const u8 jpegDqtChrQ40[64] =
{
    21,  22,  30,  58,  123, 123, 123, 123,
    22,  26,  32,  82,  123, 123, 123, 123,
    30,  32,  70,  123, 123, 123, 123, 123,
    58,  82,  123, 123, 123, 123, 123, 123,
    123, 123, 123, 123, 123, 123, 123, 123,
    123, 123, 123, 123, 123, 123, 123, 123,
    123, 123, 123, 123, 123, 123, 123, 123,
    123, 123, 123, 123, 123, 123, 123, 123,
};

/* Q30 */
/* Luminance */
const u8 jpegDqtLumQ30[64] =
{
    26,  18,  16,  26,  40,  66,  85,  101,
    20,  20,  23,  31,  43,  96,  100, 91,
    23,  21,  26,  40,  66,  95,  115, 93,
    23,  28,  36,  48,  85,  145, 133, 103,
    30,  36,  61,  93,  113, 181, 171, 128,
    40,  58,  91,  106, 135, 173, 188, 153,
    81,  106, 130, 145, 171, 201, 200, 168,
    120, 153, 158, 163, 186, 166, 171, 165,
};
/* Chrominance */
const u8 jpegDqtChrQ30[64] =
{
    28,  30,  40,  78,  165, 165, 165, 165,
    30,  35,  43,  110, 165, 165, 165, 165,
    40,  43,  93,  165, 165, 165, 165, 165,
    78,  110, 165, 165, 165, 165, 165, 165,
    165, 165, 165, 165, 165, 165, 165, 165,
    165, 165, 165, 165, 165, 165, 165, 165,
    165, 165, 165, 165, 165, 165, 165, 165,
    165, 165, 165, 165, 165, 165, 165, 165,
};

/* Q20 */
/* Luminance */
const u8 jpegDqtLumQ20[64] =
{
    40,  27,  25,  40,  60,  100, 127, 152,
    30,  30,  35,  47,  65,  145, 150, 137,
    35,  32,  40,  60,  100, 142, 172, 140,
    35,  42,  55,  72,  127, 217, 200, 155,
    45,  55,  92,  140, 170, 255, 255, 192,
    60,  87,  137, 160, 202, 255, 255, 230,
    122, 160, 195, 217, 255, 255, 255, 252,
    180, 230, 237, 245, 255, 250, 255, 247,
};
/* Chrominance */
const u8 jpegDqtChrQ20[64] =
{
    42,  45,  60,  117, 247, 247, 247, 247,
    45,  52,  65,  165, 247, 247, 247, 247,
    60,  65,  140, 247, 247, 247, 247, 247,
    117, 165, 247, 247, 247, 247, 247, 247,
    247, 247, 247, 247, 247, 247, 247, 247,
    247, 247, 247, 247, 247, 247, 247, 247,
    247, 247, 247, 247, 247, 247, 247, 247,
    247, 247, 247, 247, 247, 247, 247, 247,
};

/* Q10 */
/* Luminance */
const u8 jpegDqtLumQ10[64] =
{
    80,  55,  50,  80,  120, 200, 255, 255,
    60,  60,  70,  95,  130, 255, 255, 255,
    70,  65,  80,  120, 200, 255, 255, 255,
    70,  85,  110, 145, 255, 255, 255, 255,
    90,  110, 185, 255, 255, 255, 255, 255,
    120, 175, 255, 255, 255, 255, 255, 255,
    245, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
};
/* Chrominance */
const u8 jpegDqtChrQ10[64] =
{
    85,  90,  120, 235, 255, 255, 255, 255,
    90,  105, 130, 255, 255, 255, 255, 255,
    120, 130, 255, 255, 255, 255, 255, 255,
    235, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
};

/* Q0 */
/* Luminance */
const u8 jpegDqtLumQ0[64] =
{
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
};
/* Chrominance */
const u8 jpegDqtChrQ0[64] =
{
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
};

const u8 *jpegDqtLum, *jpegDqtChr;
//u8 jpegDqtScale = 0; /*CY 0907*/
s16 jpegRestart;
u8 jpegColorFmat;

/* Zizag scan */
const u8 jpegZigzag[64] =
{
     0,   1,   8,  16,   9,   2,   3,  10,
    17,  24,  32,  25,  18,  11,   4,   5,
    12,  19,  26,  33,  40,  48,  41,  34,
    27,  20,  13,   6,   7,  14,  21,  28,
    35,  42,  49,  56,  57,  50,  43,  36,
    29,  22,  15,  23,  30,  37,  44,  51,
    58,  59,  52,  45,  38,  31,  39,  46,
    53,  60,  61,  54,  47,  55,  62,  63,
};
/*BJ 0523 S*/
const short zz[64] =
{
     0,  1,  5,  6, 14, 15, 27, 28,
     2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,
     9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};
/*BJ 0523 E*/
/*---- Huffman table ----*/
/* Luminance DC length */
const u8 jpegDhtLumDcL[16] =
{
    0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* Luminance DC value */
const u8 jpegDhtLumDcV[12] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
};

/* Luminance AC length */
const u8 jpegDhtLumAcL[16] =
{
    0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
    0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,
};

/* Luminance AC value */
const u8 jpegDhtLumAcV[162] =
{
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
    0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
    0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
    0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
    0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
    0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
    0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
    0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
    0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa,
};

/* Chrominance DC length */
const u8 jpegDhtChrDcL[16] =
{
    0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/* Chrominance DC value */
const u8 jpegDhtChrDcV[12] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b,
};

/* Chrominance AC length */
const u8 jpegDhtChrAcL[16] =
{
    0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
    0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
};

/* Chrominance AC value */
const u8 jpegDhtChrAcV[162] =
{
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
    0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
    0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
    0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
    0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
    0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
    0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa,
};

const u32 jpegHuffEnc[384] =
{
    0x0200, 0x0302, 0x0303, 0x0304, 0x0305, 0x0306, 0x040e, 0x051e,
    0x063e, 0x077e, 0x08fe, 0x09fe, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0200, 0x040c, 0x051c, 0x063a, 0x063b, 0x077a, 0x077b, 0x08fa,
    0x09f8, 0x09f9, 0x09fa, 0x0af9, 0x0afa, 0x0bf8, 0x00eb, 0x00f5,
    0x0201, 0x051b, 0x08f9, 0x09f7, 0x0af8, 0x0bf7, 0x0cf6, 0x0cf7,
    0x0fc0, 0x00be, 0x00c7, 0x00d0, 0x00d9, 0x00e2, 0x00ec, 0x00f6,
    0x0304, 0x0779, 0x0af7, 0x0cf5, 0x0096, 0x009e, 0x00a6, 0x00ae,
    0x00b6, 0x00bf, 0x00c8, 0x00d1, 0x00da, 0x00e3, 0x00ed, 0x00f7,
    0x040b, 0x09f6, 0x0cf4, 0x008f, 0x0097, 0x009f, 0x00a7, 0x00af,
    0x00b7, 0x00c0, 0x00c9, 0x00d2, 0x00db, 0x00e4, 0x00ee, 0x00f8,
    0x051a, 0x0bf6, 0x0089, 0x0090, 0x0098, 0x00a0, 0x00a8, 0x00b0,
    0x00b8, 0x00c1, 0x00ca, 0x00d3, 0x00dc, 0x00e5, 0x00ef, 0x00f9,
    0x0778, 0x0084, 0x008a, 0x0091, 0x0099, 0x00a1, 0x00a9, 0x00b1,
    0x00b9, 0x00c2, 0x00cb, 0x00d4, 0x00dd, 0x00e6, 0x00f0, 0x00fa,
    0x08f8, 0x0085, 0x008b, 0x0092, 0x009a, 0x00a2, 0x00aa, 0x00b2,
    0x00ba, 0x00c3, 0x00cc, 0x00d5, 0x00de, 0x00e7, 0x00f1, 0x00fb,
    0x0af6, 0x0086, 0x008c, 0x0093, 0x009b, 0x00a3, 0x00ab, 0x00b3,
    0x00bb, 0x00c4, 0x00cd, 0x00d6, 0x00df, 0x00e8, 0x00f2, 0x00fc,
    0x0082, 0x0087, 0x008d, 0x0094, 0x009c, 0x00a4, 0x00ac, 0x00b4,
    0x00bc, 0x00c5, 0x00ce, 0x00d7, 0x00e0, 0x00e9, 0x00f3, 0x00fd,
    0x0083, 0x0088, 0x008e, 0x0095, 0x009d, 0x00a5, 0x00ad, 0x00b5,
    0x00bd, 0x00c6, 0x00cf, 0x00d8, 0x00e1, 0x00ea, 0x00f4, 0x00fe,
    0x040a, 0x0bf9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0200, 0x0201, 0x0202, 0x0306, 0x040e, 0x051e, 0x063e, 0x077e,
    0x08fe, 0x09fe, 0x0afe, 0x0bfe, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0201, 0x040b, 0x051a, 0x051b, 0x063a, 0x063b, 0x0779, 0x077a,
    0x08f9, 0x09f7, 0x09f8, 0x09f9, 0x09fa, 0x0bf9, 0x0ee0, 0x0fc3,
    0x0304, 0x0639, 0x08f7, 0x08f8, 0x09f6, 0x0af9, 0x0bf7, 0x0bf8,
    0x00b7, 0x00c0, 0x00c9, 0x00d2, 0x00db, 0x00e4, 0x00ed, 0x00f6,
    0x040a, 0x08f6, 0x0af7, 0x0af8, 0x0097, 0x009f, 0x00a7, 0x00af,
    0x00b8, 0x00c1, 0x00ca, 0x00d3, 0x00dc, 0x00e5, 0x00ee, 0x00f7,
    0x0518, 0x09f5, 0x0cf6, 0x0cf7, 0x0098, 0x00a0, 0x00a8, 0x00b0,
    0x00b9, 0x00c2, 0x00cb, 0x00d4, 0x00dd, 0x00e6, 0x00ef, 0x00f8,
    0x0519, 0x0bf6, 0x0fc2, 0x0091, 0x0099, 0x00a1, 0x00a9, 0x00b1,
    0x00ba, 0x00c3, 0x00cc, 0x00d5, 0x00de, 0x00e7, 0x00f0, 0x00f9,
    0x0638, 0x0cf5, 0x008c, 0x0092, 0x009a, 0x00a2, 0x00aa, 0x00b2,
    0x00bb, 0x00c4, 0x00cd, 0x00d6, 0x00df, 0x00e8, 0x00f1, 0x00fa,
    0x0778, 0x0088, 0x008d, 0x0093, 0x009b, 0x00a3, 0x00ab, 0x00b3,
    0x00bc, 0x00c5, 0x00ce, 0x00d7, 0x00e0, 0x00e9, 0x00f2, 0x00fb,
    0x09f4, 0x0089, 0x008e, 0x0094, 0x009c, 0x00a4, 0x00ac, 0x00b4,
    0x00bd, 0x00c6, 0x00cf, 0x00d8, 0x00e1, 0x00ea, 0x00f3, 0x00fc,
    0x0af6, 0x008a, 0x008f, 0x0095, 0x009d, 0x00a5, 0x00ad, 0x00b5,
    0x00be, 0x00c7, 0x00d0, 0x00d9, 0x00e2, 0x00eb, 0x00f4, 0x00fd,
    0x0cf4, 0x008b, 0x0090, 0x0096, 0x009e, 0x00a6, 0x00ae, 0x00b6,
    0x00bf, 0x00c8, 0x00d1, 0x00da, 0x00e3, 0x00ec, 0x00f5, 0x00fe,
    0x0200, 0x0afa, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

const u32 jpegQMapping[256] =
{
    0x0000, 0x07ff, 0x0400, 0x02ab, 0x0200, 0x0b33, 0x0155, 0x0a49,
    0x0100, 0x09c7, 0x00cd, 0x12e9, 0x0955, 0x093b, 0x1249, 0x0911,
    0x0080, 0x08f1, 0x11c7, 0x11af, 0x08cd, 0x08c3, 0x08ba, 0x08b2,
    0x1155, 0x251f, 0x113b, 0x1a5f, 0x1a49, 0x1a35, 0x1111, 0x2421,
    0x0880, 0x23e1, 0x10f1, 0x10ea, 0x19c7, 0x19bb, 0x19af, 0x10d2,
    0x10cd, 0x231f, 0x10c3, 0x197d, 0x10ba, 0x10b6, 0x10b2, 0x22b9,
    0x1955, 0x229d, 0x10a4, 0x2d05, 0x193b, 0x1935, 0x225f, 0x1095,
    0x2249, 0x223f, 0x2235, 0x2c57, 0x1911, 0x2219, 0x1084, 0x1082,
    0x1080, 0x18fc, 0x18f8, 0x21e9, 0x18f1, 0x21db, 0x18ea, 0x2b9b,
    0x21c7, 0x21c1, 0x21bb, 0x21b5, 0x21af, 0x2b53, 0x18d2, 0x367b,
    0x18cd, 0x2b29, 0x18c8, 0x218b, 0x18c3, 0x2b03, 0x217d, 0x2af1,
    0x18ba, 0x18b8, 0x18b6, 0x18b4, 0x18b2, 0x2ac1, 0x2ab9, 0x2159,
    0x2155, 0x3547, 0x2a9d, 0x214b, 0x18a4, 0x2a89, 0x2141, 0x189f,
    0x213b, 0x189c, 0x2135, 0x34c9, 0x2a5f, 0x2a59, 0x1895, 0x349d,
    0x2a49, 0x1891, 0x2a3f, 0x211d, 0x2a35, 0x188c, 0x2a2b, 0x344d,
    0x2111, 0x343b, 0x2a19, 0x2a15, 0x1884, 0x3419, 0x1882, 0x1881,
    0x1880, 0x20fe, 0x20fc, 0x33e9, 0x20f8, 0x3fb3, 0x29e9, 0x33cb,
    0x20f1, 0x33bd, 0x29db, 0x33af, 0x20ea, 0x29d1, 0x20e7, 0x3395,
    0x29c7, 0x20e2, 0x29c1, 0x20df, 0x29bb, 0x20dc, 0x29b5, 0x20d9,
    0x29af, 0x3359, 0x3353, 0x29a7, 0x20d2, 0x3343, 0x299f, 0x20ce,
    0x20cd, 0x2997, 0x3329, 0x20c9, 0x20c8, 0x298d, 0x298b, 0x3311,
    0x20c3, 0x3e0f, 0x3303, 0x3dfd, 0x297d, 0x297b, 0x2979, 0x32ed,
    0x20ba, 0x3dc9, 0x20b8, 0x20b7, 0x20b6, 0x20b5, 0x20b4, 0x20b3,
    0x20b2, 0x3d89, 0x20b0, 0x32bd, 0x295d, 0x3d6b, 0x2959, 0x2957,
    0x2955, 0x32a7, 0x20a9, 0x20a8, 0x20a7, 0x3299, 0x294b, 0x3293,
    0x20a4, 0x20a3, 0x3289, 0x2943, 0x2941, 0x3cff, 0x209f, 0x3279,
    0x293b, 0x3273, 0x209c, 0x326d, 0x2935, 0x3ccf, 0x2099, 0x3cc3,
    0x325f, 0x2097, 0x3259, 0x3cad, 0x2095, 0x3251, 0x2927, 0x2093,
    0x3249, 0x3c8d, 0x2091, 0x3c83, 0x323f, 0x3c79, 0x291d, 0x3c6f,
    0x3235, 0x3c65, 0x208c, 0x2917, 0x208b, 0x3229, 0x3227, 0x3c49,
    0x2911, 0x2088, 0x290f, 0x3c37, 0x3219, 0x3217, 0x3215, 0x3c25,
    0x2084, 0x3c1d, 0x2083, 0x2905, 0x2082, 0x2903, 0x2081, 0x2901
};

#if 0   // marked by Peter
/* Start of frame 0 - Baseline DCT */
u8 jpegSof0[162] =
{
    0x08,           /* P */
    0x04, 0xb0,     /* Y */
    0x06, 0x40,     /* X */
    0x03,           /* Nf - Y, Cb, Cr */
    0x01, 0x21, 0x00,   /* yC, yHV, yTq */
    0x02, 0x11, 0x01,   /* cbC, cbHV, cbTq */
    0x03, 0x11, 0x01,   /* crC, crHV, crTq */
};

/* Start of scan */
u8 jpegSos[] =
{
    0x03,               /* Ns - Y, Cb, Cr */
    0x01, 0x00,     /* yCs, yTdTa */
    0x02, 0x11,     /* cbCs, cbTdTa */
    0x03, 0x11,     /* crCs, crTdTa */
    0x00,           /* Ss */
    0x3f,           /* Se */
    0x00,           /* AhAl */
};
#endif

/* image pixel count */
u32 jpegImagePixelCount = 0;
JPEG_SIZE    jpegSize;

#if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
/* Lsk: Motion-JPEG Part begin*/
u32 MJPG_Mode;    					//0: record, 1: playback
/* task and event related */
OS_STK mjpgTaskStack[MJPG_TASK_STACK_SIZE];


/* buffer management */
extern u32 VideoBufMngReadIdx;
extern u32 VideoBufMngWriteIdx;
extern u32 VideoPictureIndex;
extern u8 VideoRecFrameRate;
extern u32 isu_int_status;
#if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
extern volatile s32 isu_idufrmcnt;
#endif

u32 MJPG_Task_Go;
s32 mjpgCoding1Frame(u8*, u32*);
s32 mjpgDecoding1Frame(u8*, u32, u8);
s32 mjpgSuspendTask(void);

#endif

/* encoding quantization table */
/*
u8 jpegPrimaryLumQ[64];
u8 jpegPrimaryChrQ[64];
u8 jpegThumbnailLumQ[64];
u8 jpegThumbnailChrQ[64];
*/



/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 jpegCheckCrQuantizationTable(u8*);
s32 jpegCheckCrHuffmanTable(u8, u8*, u8*);
s32 JPEG_Enc(u16, u16, s16, s16, s16, u8*, u8*, u8*, u8*, u32, u32);
void JPEG_PutHdr(s8, u32, u8*);
s32 jpegSetHuffmanEncodeTable(void);
s32 jpegSetHuffmanDecodeTable(u8 id, u8 dcAc, u8* Bit, u8* HuffVal);/*BJ 0523 S*/
void isuGetImgOutResolution(u16*, u16*);

#if (VIDEO_CODEC_OPTION == MJPEG_CODEC)
s32 mjpgCoding1Frame(u8*, u32*);
s32 mjpgSuspendTask(void);
#endif


/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

    Initialize JPEG encoder/decoder.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegInit(void)
{
    u32 i;

    SYS_CLK1    = (SYS_CLK1 & ~0x0c000000) | 0x04000000; //JPEG VLC Clock Divisor. The clock divisor value = Divisor + 1.
        // JPEG software reset
    sysJpegRst();

        // create JPEG Sem.
    jpegSemEvt = OSSemCreate(0);
    return 1;
}

/*

Routine Description:

    The FIQ handler of JPEG encoder/decoder.

Arguments:

    None.

Return Value:

    None.

*/
void jpegIntHandler(void)
{
    u32 intStat = JpegIntStat;
#if MULTI_CHANNEL_SUPPORT
    if ((intStat & 0x00000001) )  // JPEG encoder interrupt
    {
        OSSemPost(jpegSemEvt);
    }
#else
    if ((intStat & 0x00000001)  && (siuOpMode == SIUMODE_CAPTURE))  // JPEG encoder interrupt
    {
        OSSemPost(jpegSemEvt);
    }
#endif

#if (VIDEO_CODEC_OPTION == MJPEG_CODEC)
  #if MULTI_CHANNEL_SUPPORT
    else if ((intStat & 0x00000001))  // JPEG encoder interrupt
    {
        AHB_ARBCtrl &= ~(SYS_ARBHIPIR_JPGVLC | SYS_ARBHIPIR_JPGDCT);  //AHB ARBITER control register
        OSSemPost(jpegSemEvt);
        mp4_avifrmcnt++;
    }
  #else
    else if ((intStat & 0x00000001) && (siuOpMode == SIUMODE_MPEGAVI))  // JPEG encoder interrupt
    {
        AHB_ARBCtrl &= ~(SYS_ARBHIPIR_JPGVLC | SYS_ARBHIPIR_JPGDCT);  //AHB ARBITER control register
        OSSemPost(jpegSemEvt);
        mp4_avifrmcnt++;
    }
  #endif
    else if ((intStat & 0x0000000e) && (jpegSemDec_flag & 1))  // JPEG decoder interrupt
    {
        AHB_ARBCtrl &= ~(SYS_ARBHIPIR_JPGVLC | SYS_ARBHIPIR_JPGDCT);  //AHB ARBITER control register
        jpegSemDec_flag = 0;
        jpegDecErr_flag = intStat & 0x0c;

        VideoPictureIndex++;
        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;

        if (jpegDecErr_flag) {// reset JEPG hardware
            sysJpegRst();
        }

        OSSemPost(jpegSemEvt);
    }
#endif
    else if((intStat & 0x0000000e) && (jpegSemDec_flag & 1))   // JPEG decoder interrupt
    {
        jpegSemDec_flag = 0;
        jpegDecErr_flag = intStat & 0x0c;

        if (jpegDecErr_flag) {// reset JEPG hardware
            sysJpegRst();
        }

        OSSemPost(jpegSemEvt);
    }
    else if((intStat & 0x00000080) && (jpegSemDec_flag & 2))   // JPEG decoder block mode interrupt
    {
        jpegSemDec_flag = 0;
        jpegDecErr_flag = intStat & 0x0c;

        if (jpegDecErr_flag) {// reset JEPG hardware
            sysJpegRst();
        }

        OSSemPost(jpegSemEvt);
    }
/*BJ 0523 E*/
}

/*

Routine Description:

    The test routine of JPEG encoder/decoder.

Arguments:

    None.

Return Value:

    None.

*/
void jpegTest(void)
{

}

/*

Routine Description:

    Set image resolution.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegSetImageResolution(u16 width, u16 height)
{
    u32 McuNumber;

    jpegImagePixelCount = (u32)width * (u32)height;

    /* set width and height */
    jpegSize.width = width;
    jpegSize.height= height;


    switch (JpegCtrlTemp & JPEG_IMG_444) {
    case JPEG_IMG_422:
        //DEBUG_JPEG("YUV422\n");
        McuNumber   = ((width + 15) / 16) * ((height +  7) /  8);
        break;
    case JPEG_IMG_420:
        //DEBUG_JPEG("YUV420\n");
        McuNumber   = ((width + 15) / 16) * ((height + 15) / 16);
        break;
    case JPEG_IMG_440:
        //DEBUG_JPEG("YUV440\n");
        McuNumber   = ((width +  7) /  8) * ((height + 15) / 16);
        break;
    case JPEG_IMG_444:
        //DEBUG_JPEG("YUV444\n");
        McuNumber   = ((width +  7) /  8) * ((height +  7) /  8);
        break;
    }
    JpegCtrlTemp    = (JpegCtrlTemp & 0x000000ff) |
                      ((McuNumber & 0xffff) << 16) |
                      ((McuNumber & 0x003f0000) >> 8);

    //DEBUG_JPEG("Width   = %d, Height = %d\n", width, height) ;
    JpegImageSize = width | (height << 16);

    return 1;
}

u32 GetJpegImagePixelCount(void)
{
   return jpegImagePixelCount;
}

void isuGetImgOutResolution(u16 *width, u16 *height)
{
    *width = jpegSize.width;
    *height= jpegSize.height;
}

/*

Routine Description:

    Set quantization quality.

Arguments:

    type - Image type.
    quality - Quantization quality.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegSetQuantizationQuality(u8 type, u8 quality)
{
    const u8* lumQ;
    const u8* chrQ;

    switch (quality)
    {
        case 98:
            lumQ = jpegDqtLumQ98;
            chrQ = jpegDqtChrQ98;
            //jpegDqtScale = 98;    /*CY 0907*/
            break;

        case 95:
            lumQ = jpegDqtLumQ95;
            chrQ = jpegDqtChrQ95;
            //jpegDqtScale = 95;    /*CY 0907*/
            break;

        case 92:
            lumQ = jpegDqtLumQ92;
            chrQ = jpegDqtChrQ92;
            //jpegDqtScale = 92;    /*CY 0907*/
            break;

        case 90:
            lumQ = jpegDqtLumQ90;
            chrQ = jpegDqtChrQ90;
            //jpegDqtScale = 90;    /*CY 0907*/
            break;

        case 88:
            lumQ = jpegDqtLumQ88;
            chrQ = jpegDqtChrQ88;
            //jpegDqtScale = 88;    /*CY 0907*/
            break;

        case 86:
            lumQ = jpegDqtLumQ86;
            chrQ = jpegDqtChrQ86;
            //jpegDqtScale = 86;    /*CY 0907*/
            break;

        case 85:
            lumQ = jpegDqtLumQ85;
            chrQ = jpegDqtChrQ85;
            //jpegDqtScale = 85;    /*CY 0907*/
            break;

        case 80:
            lumQ = jpegDqtLumQ80;
            chrQ = jpegDqtChrQ80;
            //jpegDqtScale = 80;    /*CY 0907*/
            break;

        case 70:
            lumQ = jpegDqtLumQ70;
            chrQ = jpegDqtChrQ70;
            //jpegDqtScale = 70;    /*CY 0907*/
            break;

        case 60:
            lumQ = jpegDqtLumQ60;
            chrQ = jpegDqtChrQ60;
            //jpegDqtScale = 60;    /*CY 0907*/
            break;

        case 40:
            lumQ = jpegDqtLumQ40;
            chrQ = jpegDqtChrQ40;
            //jpegDqtScale = 40;    /*CY 0907*/
            break;

        case 30:
            lumQ = jpegDqtLumQ30;
            chrQ = jpegDqtChrQ30;
            //jpegDqtScale = 30;    /*CY 0907*/
            break;

        case 20:
            lumQ = jpegDqtLumQ20;
            chrQ = jpegDqtChrQ20;
            //jpegDqtScale = 20;    /*CY 0907*/
            break;

        case 10:
            lumQ = jpegDqtLumQ10;
            chrQ = jpegDqtChrQ10;
            //jpegDqtScale = 10;    /*CY 0907*/
            break;

        case 0:
            lumQ = jpegDqtLumQ0;
            chrQ = jpegDqtChrQ0;
            //jpegDqtScale = 10;    /*CY 0907*/
            break;

        case 50:
        default:
            lumQ = jpegDqtLumQ50;
            chrQ = jpegDqtChrQ50;
            //jpegDqtScale = 50;    /*CY 0907*/
            break;
    }

    jpegDqtLum = lumQ;
    jpegDqtChr = chrQ;
    /*BJ 0523 S*/
    //jpegSetQuantizationTable(JPEG_COMPONENT_Y, JPEG_QUANTIZATION_PRECISION_8, jpegDqtLum,0);  /*CY 0907*/
    //jpegSetQuantizationTable(JPEG_COMPONENT_Cb, JPEG_QUANTIZATION_PRECISION_8, jpegDqtChr,0); /*CY 0907*/
    /*BJ 0523 E*/
    exifSetQuantizationTable(type, lumQ, chrQ);

    return 1;
}


/*

Routine Description:

    Set quantization table.

Arguments:

    id - Quantization identifier.
    precision - Quantization precision.
    pQuant - Quantization table, zigzag scan order.
    mode - 0:encoder  1:decoder
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegSetQuantizationTable(u8 id, u8 precision, u8* pQuant , u8 mode)/*BJ 0523 S*/
{
    u32* dstQ;
    u8 i;

    if (precision != JPEG_QUANTIZATION_PRECISION_8) /* only 8 bit quantization table element is supported */
            return 0;

    if(mode)    // JPEG decoder
    {
        //memcpy((void*)dstQ, pQuant, 64);
        if (id == JPEG_COMPONENT_Y) {
            dstQ = (u32*)&JpegQuantLum;
            for (i = 0; i < 64; i++) {
                dstQ[i] = (u32) pQuant[zz[i]];
            }
        } else if (id == JPEG_COMPONENT_Cb) {
            dstQ = (u32*)&JpegQuantChr;
            for (i = 0; i < 64; i++) {
                dstQ[i] = (dstQ[i] & 0xff00) | (u32)pQuant[zz[i]];
            }
        } else if (id == JPEG_COMPONENT_Cr) {
            dstQ = (u32*)&JpegQuantChr;
            for (i = 0; i < 64; i++) {
                dstQ[i] = (dstQ[i] & 0x00ff) | ((u32)pQuant[zz[i]] << 8);
            }
        }
    } else {    // JPEG encoder
        if (id == JPEG_COMPONENT_Y) {
            dstQ = (u32*)&JpegQuantLum;
        } else {
            dstQ = (u32*)&JpegQuantChr;
        }
        for (i = 0; i < 64; i++) {
            //dstQ[i] = (u32) jpegQMapping[pQuant[i]];
            dstQ[i] = (u32) jpegQMapping[pQuant[zz[i]]];    /* Peter 1219 */
            //*((u32*)(dstQ + i * 4)) = (u32) jpegQMapping[pQuant[i]];
        }
    }

    return 1;
}

/*

Routine Description:

    Check Cr quantization table is equal to Cb.

Arguments:

    pQuant - Quantization table.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegCheckCrQuantizationTable(u8* pQuant)
{
    if (memcmp((void*)JpegQuantChr, (void*)pQuant, 64) != 0)
        return 0;

    return 1;
}


s32 jpegSetAllHuffmanTable(void)
{
    EXIF_DHT* pPrimaryDht = &exifPrimaryImage.primary.dht;

    /*BJ 0523 S*/
    jpegSetHuffmanTable(JPEG_COMPONENT_Y,  JPEG_HUFFMAN_DC, (u8*)pPrimaryDht->lumDcL, (u8*)pPrimaryDht->lumDcV,0);
    jpegSetHuffmanTable(JPEG_COMPONENT_Y,  JPEG_HUFFMAN_AC, (u8*)pPrimaryDht->lumAcL, (u8*)pPrimaryDht->lumAcV,0);
    jpegSetHuffmanTable(JPEG_COMPONENT_Cb, JPEG_HUFFMAN_DC, (u8*)pPrimaryDht->chrDcL, (u8*)pPrimaryDht->chrDcV,0);
    jpegSetHuffmanTable(JPEG_COMPONENT_Cb, JPEG_HUFFMAN_AC, (u8*)pPrimaryDht->chrAcL, (u8*)pPrimaryDht->chrAcV,0);
    /*BJ 0523 E*/

    return 1;
}


/*

Routine Description:

    Set huffman table.

Arguments:

    id - Huffman identifier.
    dcAc - DC / AC.
    pLength - Number of huffman codes of length i.
    pValue - Value associated with each huffman code.
    mode - 0:encoder 1:decoder

Return Value:

    0 - Failure.
    1 - Success.

*/
/*BJ 0523 S*/
s32 jpegSetHuffmanTable(u8 id, u8 dcAc, u8* pLength, u8* pValue , u8 mode)
{
    if(mode == 1)
    {   /* Decode */
        jpegSetHuffmanDecodeTable(id,dcAc, pLength, pValue);
    }
    else
    {   /* Encode */
        jpegSetHuffmanEncodeTable();
    }

    return 1;
}
/*BJ 0523 E*/
/*BJ 0523 S*/
s32 jpegSetHuffmanDecodeTable(u8 id, u8 dcAc, u8* Bit, u8* HuffVal)
{
    Huffman_table Hufftable[16];
    unsigned short HuffSize[256];
    //unsigned short HuffCode[256];
    //unsigned short EHUFCO[256];
    //unsigned short EHUFSI[256];
    //unsigned short LASTK;
    int K=0,I=1,J=1;
    int SI;
    int code = 0;
    u8 pos = 0;
    u32 temp;
    u32 * reg_ptr;

    do
    {
        while(J <= Bit[I-1])
        {
            HuffSize[K] = I;
            K = K+1;
            J = J+1;
        }
        I = I+1;
        J = 1;
    }while(I <= 16);
    HuffSize[K] = 0;
    //LASTK = K;

    SI = HuffSize[0];
    K = 0;
    code = 0;

    while (pos < HuffSize[0]) //Huffman table for register
    {
        Hufftable[pos].min = code;
        switch(id)
        {
            case JPEG_COMPONENT_Cr:
            case JPEG_COMPONENT_Cb:
                if(dcAc == JPEG_HUFFMAN_DC)
                    Hufftable[pos].diff= K-code + 192;
                else
                    Hufftable[pos].diff= K-code + 208;
                break;
            case JPEG_COMPONENT_Y:
                if(dcAc == JPEG_HUFFMAN_DC)
                    Hufftable[pos].diff= K-code + 0;
                else
                    Hufftable[pos].diff= K-code + 16;
                break;
        }
        pos++;
    }

    do
    {
        do
        {
            //HuffCode[K] = code; // No used
            code = code + 1;
            K = K+1;
        }while(HuffSize[K] == SI);
        if(HuffSize[K] == 0)
            break;
        do
        {
            code = code << 1;

            //Huffman table for register
            Hufftable[pos].min = code;
            switch(id)
            {
                case JPEG_COMPONENT_Cr:
                case JPEG_COMPONENT_Cb:
                    if(dcAc == JPEG_HUFFMAN_DC)
                        Hufftable[pos].diff= K-code + 192;
                    else
                        Hufftable[pos].diff= K-code + 208;
                    break;
                case JPEG_COMPONENT_Y:
                    if(dcAc == JPEG_HUFFMAN_DC)
                        Hufftable[pos].diff= K-code + 0;
                    else
                        Hufftable[pos].diff= K-code + 16;
                    break;
            }
            pos++;

            SI = SI+1;
        }while(HuffSize[K] != SI);
    }while(HuffSize[K] == SI);

    for (I= pos; I < 16; I++){
        Hufftable[I].min = 0xff;
        //Hufftable[I].diff = 0x00;
        switch(id)  // fix bug by Peter
        {
            case JPEG_COMPONENT_Cr:
            case JPEG_COMPONENT_Cb:
                if(dcAc == JPEG_HUFFMAN_DC)
                    Hufftable[I].diff   = 0;
                else
                    Hufftable[I].diff   = 208;
                break;
            case JPEG_COMPONENT_Y:
                if(dcAc == JPEG_HUFFMAN_DC)
                    Hufftable[I].diff   = 0;
                else
                    Hufftable[I].diff   = 16;
                break;
        }
    }

/*
    K = 0;
    do
    {
        I = HuffVal[K];
        EHUFCO[I] = HuffCode[K];
        EHUFSI[I] = HuffSize[K];
        K = K+1;
    }while(K < LASTK);
*/ //Not used

    /* write Huffman table to register */
    //DEBUG_JPEG("min :");
    for(pos =0 ; pos < 16 ; pos++)
    {
        //DEBUG_JPEG("%x ",Hufftable[pos].min);
    }
    //DEBUG_JPEG("\n");
    //DEBUG_JPEG("diff :");
    for(pos =0 ; pos < 16 ; pos++)
    {
        //DEBUG_JPEG("%x ",Hufftable[pos].diff);
    }
    //DEBUG_JPEG("\n");

    switch(id)
    {
        case JPEG_COMPONENT_Y:
            if (dcAc == JPEG_HUFFMAN_DC)
            {
                //DEBUG_JPEG("Lum_DC\n");
            }
            else
            {
                //DEBUG_JPEG("Lum_AC\n");
            }
            break;
        case JPEG_COMPONENT_Cb:
        case JPEG_COMPONENT_Cr:
            if (dcAc == JPEG_HUFFMAN_DC)
            {
                //DEBUG_JPEG("Chrom_DC\n");
            }
            else
            {
                //DEBUG_JPEG("Chrom_AC\n");
            }
            break;
    }

    for(pos =0 ; pos < 16 ; pos+=4)
    {
        temp =  (Hufftable[pos+3].min & 0xFF)<<24 |
                (Hufftable[pos+2].min & 0xFF)<<16 |
                (Hufftable[pos+1].min & 0xFF)<<8 |
                (Hufftable[pos].min & 0xFF);
        switch (id)
        {
            case JPEG_COMPONENT_Y:
                if (dcAc == JPEG_HUFFMAN_DC)
                {
                    /* DC of Y */
                    //(*(u32 *)(JpegHufAc0MinTblAddr0+pos)) = temp;
                    //JpegHufAc0MinTblAddr0[pos] = temp;
                    reg_ptr = (u32 *)&JpegHufDc0MinTblAddr0;
                    reg_ptr[pos/4] = temp;
                }
                else
                {
                    /* AC of Y */
                    //(*(u32 *)(JpegHufDc0MinTblAddr0+pos)) = temp;
                    //JpegHufDc0MinTblAddr0[pos] = temp;
                    reg_ptr = (u32 *)&JpegHufAc0MinTblAddr0;
                    reg_ptr[pos/4] = temp;
                }
                break;

            case JPEG_COMPONENT_Cb:
            case JPEG_COMPONENT_Cr:
                if (dcAc == JPEG_HUFFMAN_DC)
                {
                    /* DC of Cb */
                    //(*(u32 *)(JpegHufAc1MinTblAddr0+pos)) = temp;
                    //JpegHufAc1MinTblAddr0[pos] = temp;
                    reg_ptr = (u32 *)&JpegHufDc1MinTblAddr0;
                    reg_ptr[pos/4] = temp;
                }
                else
                {
                    /* AC of Cb */
                    //(*(u32 *)(JpegHufDc1MinTblAddr0+pos)) = temp;
                    //JpegHufDc1MinTblAddr0[pos[ = temp;
                    reg_ptr = (u32 *)&JpegHufAc1MinTblAddr0;
                    reg_ptr[pos/4] = temp;
                }
                break;
            default:
                return 0;
        }
    }
    for(pos =0 ; pos < 16 ; pos+=2)
    {
        temp =  (Hufftable[pos+1].diff & 0xFFFF)<<16 |
                (Hufftable[pos].diff & 0xFFFF);

        switch (id)
        {
            case JPEG_COMPONENT_Y:
                if (dcAc == JPEG_HUFFMAN_DC)
                {
                    /* DC of Y */
                    //(*(u32 *)(JpegHufAc0DifTblAddr0+pos*2)) = temp;
                    reg_ptr = (u32 *)&JpegHufDc0DifTblAddr0;
                    *(reg_ptr+pos/2) = temp;
                }
                else
                {
                    /* AC of Y */
                    //(*(u32 *)(JpegHufDc0DifTblAddr0+pos*2)) = temp;
                    reg_ptr = (u32 *)&JpegHufAc0DifTblAddr0;
                    //reg_ptr[pos/2] = temp;
                    *(reg_ptr+pos/2) = temp;
                }
                break;

            case JPEG_COMPONENT_Cb:
            case JPEG_COMPONENT_Cr:
                if (dcAc == JPEG_HUFFMAN_DC)
                {
                    /* DC of Cb */
                    //(*(u32 *)(JpegHufAc1DifTblAddr0+pos*2)) = temp;
                    reg_ptr = (u32 *)&JpegHufDc1DifTblAddr0;
                    //reg_ptr[pos/2] = temp;
                    *(reg_ptr+pos/2) = temp;
                }
                else
                {
                    /* AC of Cb */
                    //(*(u32 *)(JpegHufDc1DifTblAddr0+pos*2)) = temp;
                    reg_ptr = (u32 *)&JpegHufAc1DifTblAddr0;
                    //reg_ptr[pos/2] = temp;
                    *(reg_ptr+pos/2) = temp;
                }
                break;
            default:
                return 0;
        }
    }

    pos = 0;
    for(I = 0; I < 16 ; I++)
        pos += Bit[I];
    for(I = 0 ; I < pos ; I++)
        switch (id)
        {
            case JPEG_COMPONENT_Y:
                if (dcAc == JPEG_HUFFMAN_DC)
                {
                    //*((u32 *)(JPEG_HUFF_DEC_DC0_BASE+4*I)) = (u32)HuffVal[I];
                    reg_ptr = (u32 *)&JPEG_HUFF_DEC_DC0_BASE;
                    reg_ptr[I] = (u32)HuffVal[I];
                }
                else
                {
                    //*((u32 *)(JPEG_HUFF_DEC_AC0_BASE+4*I)) = (u32)HuffVal[I];
                    reg_ptr = (u32 *)&JPEG_HUFF_DEC_AC0_BASE;
                    reg_ptr[I] = (u32)HuffVal[I];
                }
                break;
            case JPEG_COMPONENT_Cb:
            case JPEG_COMPONENT_Cr:
                if (dcAc == JPEG_HUFFMAN_DC)
                {
                    //*((u32 *)(JPEG_HUFF_DEC_DC1_BASE+4*I)) = (u32)HuffVal[I];
                    reg_ptr = (u32 *)&JPEG_HUFF_DEC_DC1_BASE;
                    reg_ptr[I] = (u32)HuffVal[I];
                }
                else
                {
                    //*((u32 *)(JPEG_HUFF_DEC_AC1_BASE+4*I)) = (u32)HuffVal[I];
                    reg_ptr = (u32 *)&JPEG_HUFF_DEC_AC1_BASE;
                    reg_ptr[I] = (u32)HuffVal[I];
                }
                break;
            default:
                return 0;
        }
        return 1;
}
/*BJ 0523 E*/

s32 jpegSetHuffmanEncodeTable(void)
{
    u16 i;
    u32 *jpegenc_ptbl;

    jpegenc_ptbl = (u32*)&JPEGHuffmanEncTb1Addr;

    for (i=0; i<384; i++)
        jpegenc_ptbl[i] = jpegHuffEnc[i];

    //memcpy((void* ) JPEGHuffmanEncTb1Addr, (void* )jpegHuffEnc, 384*4);
    return 1;
}


/*

Routine Description:

    Check Cr huffman table is equal to Cb.

Arguments:

    dcAc - DC / AC.
    pLength - Number of huffman codes of length i.
    pValue - Value associated with each huffman code.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegCheckCrHuffmanTable(u8 dcAc, u8* pLength, u8* pValue)
{
    if (dcAc == JPEG_HUFFMAN_DC)
    {
        /* DC of Cr */
    }
    else
    {
        /* AC of Cr */
    }

    return 1;
}

/*

Routine Description:

    Set data format.

Arguments:

    format - Data format.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegSetDataFormat(u8 format)
{
    //JpegCtrlTemp = 0;/*BJ 0609 S*/
    /* set data format YCbCr 422 or 420 or 440 or 444 */
    JpegCtrlTemp = format;

    return 1;
}

/*

Routine Description:

    Set restart interval.

Arguments:

    restartInterval - Restart interval.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegSetRestartInterval(u16 restartInterval)
{
    /* set restart interval */
/*Peter 0829 S*/
    if(restartInterval) {
        JpegRestartInterval = restartInterval | JPEG_DRI_EN;
    } else {
        JpegRestartInterval = 0;
    }
/*Peter 0829 E*/

    return 1;
}



/*

Routine Description:

    Set default data format.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegSetDefaultDataFormat(void)
{


    return 1;
}

/*

Routine Description:

    Capture primary.

Arguments:

    pBuf - Bitstream buffer.
    pSize - Bitstream size.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegCapturePrimary(u8* pBuf, s16 JpegOpMode)
{
    u32 MCU_no;

    EXIF_DQT* pPrimaryDqt = &exifPrimaryImage.primary.dqt; /*CY 0907*/
    #if (VIDEO_CODEC_OPTION == MJPEG_CODEC)
    u8* buffer[3];  //idu buffer address
	buffer[0]=PKBuf0;
	buffer[1]=PKBuf1;
	buffer[2]=PKBuf2;
    #endif

    /* set encode quantization table of thumbnail image */  /*CY 0907*/
    jpegSetQuantizationTable(JPEG_COMPONENT_Y, JPEG_QUANTIZATION_PRECISION_8, (u8*)pPrimaryDqt->lumQ, 0);
    jpegSetQuantizationTable(JPEG_COMPONENT_Cb, JPEG_QUANTIZATION_PRECISION_8, (u8*)pPrimaryDqt->chrQ, 0);

    //Lucian: Avoid Limitation of MCU
    MCU_no  = jpegSize.width * jpegSize.height / (16 * 8);
    if(MCU_no > 65535)
    {
      jpegColorFmat                     = JPEG_FORMAT_420;
      exifPrimaryImage.primary.sof0.yHV = 0x22;
    }
    else
    {
      jpegColorFmat                     = JPEG_FORMAT_422;
      exifPrimaryImage.primary.sof0.yHV = 0x21;
    }

#if JPEG_DRI_ENABLE
    jpegRestart = bSwap16(exifPrimaryImage.primary.dri.Ri);
#else
    jpegRestart = 0;
#endif

    // set huffman tables
    jpegSetHuffmanEncodeTable();


    // start JPEG hardware compression
    JPEG_Enc( jpegSize.width,               // Image width
              jpegSize.height,              // Image height
              jpegColorFmat,                // Bitstream format, 0: YUV422, 1: YUV420
              jpegRestart,                  // Restart interval
              JpegOpMode,                   // 1: Slice mode, 0: frame mode
              PKBuf,                        // Image address 0, for both frame mode and slice mode, must align 32. Y data, if DataType == YUV420.
              PKBuf1,                       // Image address 1, for slice mode only, must align 32. UV data, if DataType == YUV420.
              PKBuf2,                       // Image address 2, for slice mode only, must align 32.
              pBuf,                         // Bitstream address, must align 4
              0,                            // for A1018, image data type, 0:YUV422, 1:YUV420.
              jpegSize.width * 2);          // for A1018, stride in bytes.
    return 1;
}

#if ADDAPP3TOJPEG
s32 jpegCaptureAPP3Image(u8* pBitsBuf,u8* pImgBuf, s16 JpegOpMode,u32 SrcStride)
{
    u32 MCU_no;

    EXIF_DQT* pPrimaryDqt = &exifAPP3VGAImage.primary.dqt; /*CY 0907*/

    /* set encode quantization table of thumbnail image */  /*CY 0907*/
    jpegSetQuantizationTable(JPEG_COMPONENT_Y, JPEG_QUANTIZATION_PRECISION_8, (u8*)pPrimaryDqt->lumQ, 0);
    jpegSetQuantizationTable(JPEG_COMPONENT_Cb, JPEG_QUANTIZATION_PRECISION_8, (u8*)pPrimaryDqt->chrQ, 0);

  #if JPEG_DRI_ENABLE
    jpegRestart = bSwap16(exifAPP3VGAImage.primary.dri.Ri);
  #else
    jpegRestart = 0;
  #endif

    // set huffman tables
    jpegSetHuffmanEncodeTable();

    /*
       Lucian: 因APP3有64KB大小限制,若拍攝複雜影像會超過限制,故修改之前儲存VGA大小的Image,至Pannel大小(160x240)
    */
    JPEG_Enc( JPGAPP3_WIDTH,                // Image width
              JPGAPP3_HEIGHT,               // Image height
              0,                            // Bitstream format, 0: YUV422, 1: YUV420
              jpegRestart,                  // Restart interval
              JpegOpMode,                   // 1: Slice mode, 0: frame mode
              pImgBuf,                      // Image address 0, for both frame mode and slice mode, must align 32. Y data, if DataType == YUV420.
              PKBuf1,                       // Image address 1, for slice mode only, must align 32. UV data, if DataType == YUV420.
              PKBuf2,                       // Image address 2, for slice mode only, must align 32.
              pBitsBuf,                     // Bitstream address, must align 4
              0,                            // for A1018, image data type, 0:YUV422, 1:YUV420.
              SrcStride);                     // for A1018, stride in bytes.

    return 1;
}
#endif

//
s32 jpegCapturePreviewImg(u8* pBitsBuf,u8* pImgBuf, s16 JpegOpMode,u16 ImgWidth,u16 ImgHeight)
{

    EXIF_DQT* pPrimaryDqt = &exifPrimaryImage.primary.dqt; /*CY 0907*/

    //jpegSetQuantizationQuality(JPEG_IMAGE_PRIMARY, 98);
    /* set encode quantization table of thumbnail image */  /*CY 0907*/
    jpegSetQuantizationTable(JPEG_COMPONENT_Y, JPEG_QUANTIZATION_PRECISION_8, (u8*)pPrimaryDqt->lumQ, 0);
    jpegSetQuantizationTable(JPEG_COMPONENT_Cb, JPEG_QUANTIZATION_PRECISION_8, (u8*)pPrimaryDqt->chrQ, 0);

    jpegColorFmat = JPEG_FORMAT_422;
    exifPrimaryImage.primary.sof0.yHV   = 0x21;
    #if JPEG_DRI_ENABLE
    jpegRestart = bSwap16(exifPrimaryImage.primary.dri.Ri);
    #else
    jpegRestart = 0;
    #endif
    // set huffman tables
    jpegSetHuffmanEncodeTable();

    // start JPEG hardware compression
    JPEG_Enc(ImgWidth,              // Image width
             ImgHeight,             // Image height
             jpegColorFmat,         // Bitstream format, 0: YUV422, 1: YUV420
             jpegRestart,           // Restart interval
             JpegOpMode,            // 1: Slice mode, 0: frame mode
             pImgBuf,               // Image address 0, for both frame mode and slice mode, must align 32. Y data, if DataType == YUV420.
             PKBuf1,                // Image address 1, for slice mode only, must align 32. UV data, if DataType == YUV420.
             PKBuf2,                // Image address 2, for slice mode only, must align 32.
             pBitsBuf,              // Bitstream address, must align 4
             0,                     // for A1018, image data type, 0:YUV422, 1:YUV420.
             ImgWidth * 2);         // for A1018, stride in bytes.

    return 1;
}

s32 jpegCapturePreviewImg420(u8* pBitsBuf, u8* pImgBufY, u8* pImgBufUV, s16 JpegOpMode, u16 ImgWidth, u16 ImgHeight, u32 Stride)
{

    EXIF_DQT* pPrimaryDqt = &exifPrimaryImage.primary.dqt; /*CY 0907*/

    /* set encode quantization table of thumbnail image */  /*CY 0907*/
    jpegSetQuantizationTable(JPEG_COMPONENT_Y, JPEG_QUANTIZATION_PRECISION_8, (u8*)pPrimaryDqt->lumQ, 0);
    jpegSetQuantizationTable(JPEG_COMPONENT_Cb, JPEG_QUANTIZATION_PRECISION_8, (u8*)pPrimaryDqt->chrQ, 0);

    jpegColorFmat = JPEG_FORMAT_420;
    exifPrimaryImage.primary.sof0.yHV   = 0x22;
    #if JPEG_DRI_ENABLE
    jpegRestart = bSwap16(exifPrimaryImage.primary.dri.Ri);
    #else
    jpegRestart = 0;
    #endif
    // set huffman tables
    jpegSetHuffmanEncodeTable();

    // start JPEG hardware compression
    JPEG_Enc(ImgWidth,              // Image width
             ImgHeight,             // Image height
             jpegColorFmat,         // Bitstream format, 0: YUV422, 1: YUV420
             jpegRestart,           // Restart interval
             JpegOpMode,            // 1: Slice mode, 0: frame mode
             pImgBufY,              // Image address 0, for both frame mode and slice mode, must align 32. Y data, if DataType == YUV420.
             pImgBufUV,             // Image address 1, for slice mode only, must align 32. UV data, if DataType == YUV420.
             PKBuf2,                // Image address 2, for slice mode only, must align 32.
             pBitsBuf,              // Bitstream address, must align 4
             1,                     // for A1018, image data type, 0:YUV422, 1:YUV420.
             Stride);               // for A1018, stride in bytes.

    return 1;
}

//




void JPEG_PutHdr(s8 n, u32 val, u8 *bitstrmBuf)
{
    s8 i;

    for(i = n - 1; i >= 0; i--)
        *bitstrmBuf++   = (u8)((val >> (8 * i)) & 0xff);
}


s32 JPEG_Enc( u16 Width,        // Image width
                  u16 Height,       // Image height
                  s16 Format,       // Bitstream format, 0: YUV422, 1: YUV420
                  s16 restart,      // Restart interval
                  s16 slice_mode,   // 1: Slice mode, 0: frame mode
                  u8  *bufAddr_0,   // Image address 0, for both frame mode and slice mode, must align 32. Y data, if DataType == YUV420.
                  u8  *bufAddr_1,   // Image address 1, for slice mode only, must align 32. UV data, if DataType == YUV420.
                  u8  *bufAddr_2,   // Image address 2, for slice mode only, must align 32.
                  u8  *poutBuf,     // Bitstream address, must align 4
                  u32 DataType,     // for A1018, image data type, 0:YUV422, 1:YUV420.
                  u32 Stride)       // for A1018, stride in bytes.
{
    s32     jpegCmpSize;
    u32     mcuSize;

    sysJpegRst();

#if(CHIP_OPTION >= CHIP_A1018A)
    JPEG_BRI_CTRL   = JPEG_BRI_MODE_ENC |
                      (DataType << 2) |
                      (Format << 3);
    JPEG_BRI_ADDR_Y = (u32)bufAddr_0;
    JPEG_BRI_ADDR_C = (u32)bufAddr_1;
    JPEG_BRI_STRIDE = Stride;
    JPEG_BRI_SIZE   = Width | (Height << 16);
#endif

    // Set image size
    JpegImageSize   = (Height << JPEG_IMAGE_SIZE_Y_SHFT) | Width;

    // Set and calculate the number of MCU
    jpegCmpSize = (u32) Height * (u32) Width;
    mcuSize = jpegCmpSize >> 7;
    if (Format == JPEG_FORMAT_420)
       mcuSize = mcuSize >> 1;
    JpegCtrlTemp    = (Format << JPEG_IMG_SHFT) |
                      (slice_mode << JPEG_OPBUF_SHFT)|
                      (mcuSize << JPEG_MCU_NO_SHFT);

    if (Format == JPEG_FORMAT_420)
    {
    //    DEBUG_JPEG("Coding format is YUV420 \n");
    }
    else if (Format == JPEG_FORMAT_422)
    {
        //DEBUG_JPEG("Coding format is YUV422 \n") ;
    }
    else
    {
        DEBUG_JPEG("Error: Coding format is unknown.\n") ;
        exit(0) ;
    }

    //DEBUG_JPEG("Width   = %d, 0x%x\n", Width, Width) ;
    //DEBUG_JPEG("Height  = %d, 0x%x\n", Height, Height) ;
    //DEBUG_JPEG("Restart = %d, 0x%x\n", restart, restart) ;

    // Set restart interval
/*Peter 0829 S*/
    if(restart) {
        JpegRestartInterval = (restart & 0x0000ffff) | JPEG_DRI_EN;
    } else {
        JpegRestartInterval = 0;
    }
/*Peter 0829 E*/

    // set JPEG bitstream starting address and default size
    JpegStreamAddr  = (u32) poutBuf;
    JpegStreamSize  = jpegCmpSize;

    // set input image atarting address
    JpegImageAddr0  = (u32) bufAddr_0;
    JpegImageAddr1  = (u32) bufAddr_1;
    JpegImageAddr2  = (u32) bufAddr_2;

    // set JPEG interrupt when coding an image is complete
    JpegIntStat = 0;
    JpegIntEna = JPEG_CMP_INT_ENA;

    // start JPEG hardware
    JpegCtrl        = JpegCtrlTemp;
    JpegCtrl        = JpegCtrlTemp | JPEG_ENC_ENA;

    return 1;
}

s32 WaitJpegEncComplete(void)
{
    INT8U err;
    s32 outLen;

    OSSemPend(jpegSemEvt, JPEG_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_JPEG("Error: jpegSemEvt is %d.\n", err);
        sysJpegRst();

        return 0;
    }
    outLen = (s32) JpegStreamSize;

    return outLen;

}


/*

Routine Description:

    Capture thumbnail.

Arguments:

    pBuf - Bitstream buffer.
    pSize - Bitstream size.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 jpegCaptureThumbnail(u8* pBuf, u32* pSize)
{
    u32 mod;
    EXIF_DQT* pThumbnailDqt = &exifThumbnailImage.app1.thumbnail.dqt;

    /* set encode quantization table of thumbnail image */
    jpegSetQuantizationTable(JPEG_COMPONENT_Y, JPEG_QUANTIZATION_PRECISION_8, (u8*)pThumbnailDqt->lumQ,0);
    jpegSetQuantizationTable(JPEG_COMPONENT_Cb, JPEG_QUANTIZATION_PRECISION_8, (u8*)pThumbnailDqt->chrQ,0);

    /* compress jpeg bitstream */
    jpegColorFmat = JPEG_FORMAT_422;
    jpegRestart = 0;

    exifThumbnailImage.app1.thumbnail.sof0.yHV  = 0x21 + jpegColorFmat;

    // set huffman tables
    jpegSetHuffmanEncodeTable();

    // start JPEG hardware compression
    JPEG_Enc( 160,                      // Image width
              120,                      // Image height
              JPEG_FORMAT_422,          // Bitstream format, 0: YUV422, 1: YUV420
              jpegRestart,              // Restart interval
              JPEG_OPMODE_FRAME,        // 1: Slice mode, 0: frame mode
              PKBuf1,                   // Image address 0, for both frame mode and slice mode, must align 32. Y data, if DataType == YUV420.
              PKBuf2,                   // Image address 1, for slice mode only, must align 32. UV data, if DataType == YUV420.
              PKBuf2,                   // Image address 2, for slice mode only, must align 32.
              pBuf,                     // Bitstream address, must align 4
              0,                        // for A1018, image data type, 0:YUV422, 1:YUV420.
              160 * 2);                 // for A1018, stride in bytes.

    *pSize=WaitJpegEncComplete();

    mod = *pSize % 4;
    if (mod != 0)
        *pSize += (4 - mod);

    /* set related EXIF IFD ... */
    exifSetThumbnailSize(*pSize);

    return 1;
}

/*

Routine Description:

    Decompression.

Arguments:

    pBuf - Bitstream buffer.

Return Value:

    0 - Failure.
    1 - Success.

*/
/*BJ 0523 S*/
s32 jpegDecompression(u8* pBuf , u8* pResult)
{

    INT8U err;
    u16 size;
    u32 count,u,i,downsmple;
    u32	*pucDstAddr, *pucSrcAddr;
	u8	*pucDst1Addr,*pucSrc1Addr, *pucDst2Addr,*pucDst3Addr,*pucDst4Addr,cc;

#if(CHIP_OPTION >= CHIP_A1018A) // for JPEG_BRI_DAT_TYPE_YUV422
    switch (JpegCtrlTemp & JPEG_IMG_444) {
    case JPEG_IMG_422:
        //DEBUG_JPEG("YUV422\n");
        JPEG_BRI_CTRL   = JPEG_BRI_MODE_DEC |
                          JPEG_BRI_DAT_TYPE_YUV422 |
                          JPEG_BRI_DAT_FMT_YUV422;
        break;
    case JPEG_IMG_420:
        //DEBUG_JPEG("YUV420\n");
        JPEG_BRI_CTRL   = JPEG_BRI_MODE_DEC |
                          JPEG_BRI_DAT_TYPE_YUV422 |
                          JPEG_BRI_DAT_FMT_YUV420;
        break;
    case JPEG_IMG_440:
        //DEBUG_JPEG("YUV440\n");
        JPEG_BRI_CTRL   = JPEG_BRI_MODE_DEC |
                          JPEG_BRI_DAT_TYPE_YUV422 |
                          JPEG_BRI_DAT_FMT_YUV440;
        break;
    case JPEG_IMG_444:
        //DEBUG_JPEG("YUV444\n");
        JPEG_BRI_CTRL   = JPEG_BRI_MODE_DEC |
                          JPEG_BRI_DAT_TYPE_YUV422 |
                          JPEG_BRI_DAT_FMT_YUV444;
        break;
    }

    JPEG_BRI_ADDR_Y = (u32)pResult;
    JPEG_BRI_ADDR_C = (u32)pResult + jpegSize.width + jpegSize.height;
    if(JPEG_BRI_CTRL & JPEG_BRI_DAT_TYPE_YUV420)
        JPEG_BRI_STRIDE = jpegSize.width;
    else
        JPEG_BRI_STRIDE = jpegSize.width * 2;
    JPEG_BRI_SIZE   = jpegSize.width | (jpegSize.height << 16);
    //DEBUG_JPEG("(%d,%d)\n", jpegSize.width, jpegSize.height);
#endif

    /* decompression */
    JpegStreamAddr = (u32)(pBuf);
    JpegImageAddr0 = (u32)(pResult);

    JpegIntEna = 0x07; // All interrupt enable

    jpegSemDec_flag = 1;
    jpegDecErr_flag = 0;

    //trigger
    JpegCtrl        = JpegCtrlTemp;
    JpegCtrl        = JpegCtrlTemp | 0x0002; // trigger decoder.

#if IS_COMMAX_DOORPHONE
    OSSemPend(jpegSemEvt, 20, &err);    // JPEG_TIMEOUT/5
    ///< to avoid the abnormal delay after abnomal timeout
#else
    OSSemPend(jpegSemEvt, JPEG_TIMEOUT, &err);
#endif
   #if(HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
    if(jpegSize.width == 704)
        IDU_TVLayer_Stride(800 ,pResult, 704, 480, 0, 0, 704, uiMenuBuf2);
   #endif
    #if 1
    if(splitmenu==1)
    {
        DEBUG_JPEG("filecon=%2d \n",filecon);
        (u32)pucSrcAddr=(u32)pResult;
   #if(HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
        (u32)pucDstAddr=(u32)uiMenuBuf2;
   #else
        (u32)pucDstAddr=(u32)PKBuf0;
   #endif
        if(filecon==1)
            pucDstAddr+=160;
        else if (filecon==2)
            #if(HW_BOARD_OPTION == MR9670_HECHI) || (HW_BOARD_OPTION == MR8120_RX_HECHI) 
            pucDstAddr+=1280*60; // 800*480
            #else
            pucDstAddr+=1600*60; // 640*480
            #endif
        else if (filecon==3)
            #if(HW_BOARD_OPTION == MR9670_HECHI) || (HW_BOARD_OPTION == MR8120_RX_HECHI) 
            pucDstAddr+=1280*60+160;// 800*480
            #else
            pucDstAddr+=1600*60+160; // 640*480
            #endif
        size=160*2;
        count=160*480*2;
        i=0;
        while(i!=count)
        {
            if((i%size==0)&&(i!=0))
            {
                i+=size;
                pucSrcAddr+=size;
            #if(HW_BOARD_OPTION == MR9670_HECHI) || (HW_BOARD_OPTION == MR8120_RX_HECHI) 
                pucDstAddr+=160; // 800*480
            #else 
                pucDstAddr+=240; // 640*480
            #endif
                if(i==count)
                break;
            }
			pucDstAddr[0]= pucSrcAddr[0];
		//	pucDstAddr[1]= pucSrcAddr[1];
		//	pucDstAddr[2]= pucSrcAddr[2];
		//	pucDstAddr[3]= pucSrcAddr[3];

        //    memcpy_hw(pucDstAddr,pucSrcAddr,4);
            i+=2;
            pucDstAddr+=1;
            pucSrcAddr+=2;
        }
     //    memcpy_hw(PKBuf,PKBuf1,800*480*2);
    }
    else if(splitmenu==2)
    {
        DEBUG_JPEG("jpeg decode \n\n");
        pucSrc1Addr=pResult;
   #if(HW_BOARD_OPTION == MR8100_RX_RDI_SEM)
        pucDst1Addr=uiMenuBuf2;
   #else
        pucDst1Addr=PKBuf0;
   #endif
        size=800*2;
        count=800*480*2;
        while (count)
        {
            memcpy_hw(pucDst1Addr,pucSrc1Addr, 1280);
            pucDst1Addr += size;
            pucSrc1Addr   += 1280;
            count  -= size;
        }
        //memcpy_hw(PKBuf,PKBuf0,800*480*2);
    }

    #else
    if(splitmenu==1)
    {
    DEBUG_JPEG("filecon=%2d \n",filecon);
    (u32)pucSrcAddr=(u32)pResult;
    (u32)pucDstAddr=(u32)PKBuf0;
    size=800*2;

    count=800*480*2;
    while (count)
    {
            memcpy_hw(pucDstAddr,pucSrcAddr, 1280);
            pucDstAddr += size;
            pucSrcAddr   += 1280;
            count  -= size;
    }
    (u32)pucDst1Addr=(u32)PKBuf0;
    (u32)pucDst2Addr=(u32)PKBuf1;
    count=800*480*2;
    i=0;
    while(i!=count)
    {
        if((i%size==0)&&(i!=0))
        {
            i+=size;
            pucDst1Addr+=1600;
            pucDst2Addr+=800;
            if(i==count)
            break;
        }
        memcpy_hw(pucDst2Addr,pucDst1Addr,4);//Y0
        i+=8;
        pucDst2Addr+=4;
        pucDst1Addr+=8;

    }
    switch(filecon)
    {
        case 0:
            DEBUG_JPEG("case 0 \n\n");
            memcpy_hw( PKBuf2,PKBuf1,800*480*2 );
        break;

        case 1:
            DEBUG_JPEG("case 1 \n\n");
            pucDst3Addr = PKBuf1;
            pucDst4Addr = PKBuf2;
            pucDst4Addr += 640;
            while (count)
            {
                memcpy_hw(pucDst4Addr,pucDst3Addr, 640);
                pucDst3Addr  += 1600;
                pucDst4Addr  += 1600;
                count  -= size;
            }
        break;

        case 2:
            DEBUG_JPEG("case 2 \n\n");
            pucDst3Addr = PKBuf1;
            pucDst4Addr = PKBuf2;
            pucDst4Addr += 1600*240;
            while (count)
            {
                memcpy_hw(pucDst4Addr,pucDst3Addr, 640);
                pucDst3Addr  += 1600;
                pucDst4Addr  += 1600;
                count  -= size;
            }
        break;

        case 3:
            DEBUG_JPEG("case 3 \n\n");
            pucDst3Addr = PKBuf1;
            pucDst4Addr = PKBuf2;
            pucDst4Addr += 1600*240+640;
            while (count)
            {
                memcpy_hw(pucDst4Addr,pucDst3Addr, 640);
                pucDst3Addr  += 1600;
                pucDst4Addr  += 1600;
                count  -= size;
            }
        break;
    }
    iduPlaybackFrame(PKBuf0);
    memcpy_hw( pResult,PKBuf0,800*480*2 );

    }
    #endif

    sysJpegRst();

    if (err != OS_NO_ERR)
    {
        DEBUG_JPEG("Error: jpegSemEvt(decode) is %d.\n", err);
        return 0;
    }

    if(jpegDecErr_flag != 0) {
        DEBUG_JPEG("Error: JPEG decode error!!! jpegDecErr_flag is 0x%02x.\n", jpegDecErr_flag);
        return 0;
    }

    return 1;
}

#if(CHIP_OPTION >= CHIP_A1018B) // for JPEG_BRI_DAT_TYPE_YUV422
/*

Routine Description:

    Decompression to YUV420.

Arguments:

    pBuf        - Bitstream buffer.
    pResultY    - Output image address, must align 32. Y data, if DataType == YUV420. YUV422 data, if DataType == YUV422.
    pResultUV   - Output image address, must align 32. UV data, if DataType == YUV420. Invalid when DataType == YUV422.
    DataType    - for A1018, image data type, 0:YUV422, 1:YUV420.
    Stride      - for A1018, stride in bytes. Only valid when DataType == YUV420.

Return Value:

    0 - Failure.
    1 - Success.

*/
/*BJ 0523 S*/
s32 jpegDecompressionYUV420(u8* pBuf,       // Bitstream address, must align 4
                                        u8* pResultY,   // Output image address, must align 32. Y data, if DataType == YUV420. YUV422 data, if DataType == YUV422.
                                        u8* pResultUV,  // Output image address, must align 32. UV data, if DataType == YUV420. Invalid when DataType == YUV422.
                                        u32 DataType,   // for A1018, image data type, 0:YUV422, 1:YUV420.
                                        u32 Stride)     // for A1018, stride in bytes. Only valid when DataType == YUV420.
{

    INT8U err;
    u16 size;
    u32 count,u,i,downsmple;
    u32	*pucDstAddr, *pucSrcAddr;
	u8	*pucDst1Addr,*pucSrc1Addr, *pucDst2Addr,*pucDst3Addr,*pucDst4Addr,cc;

    switch (JpegCtrlTemp & JPEG_IMG_444) {
    case JPEG_IMG_422:
        //DEBUG_JPEG("YUV422\n");
        JPEG_BRI_CTRL   = JPEG_BRI_MODE_DEC |
                          (DataType << 2) |
                          JPEG_BRI_DAT_FMT_YUV422;
        break;
    case JPEG_IMG_420:
        //DEBUG_JPEG("YUV420\n");
        JPEG_BRI_CTRL   = JPEG_BRI_MODE_DEC |
                          (DataType << 2) |
                          JPEG_BRI_DAT_FMT_YUV420;
        break;
    case JPEG_IMG_440:
        //DEBUG_JPEG("YUV440\n");
        JPEG_BRI_CTRL   = JPEG_BRI_MODE_DEC |
                          (DataType << 2) |
                          JPEG_BRI_DAT_FMT_YUV440;
        break;
    case JPEG_IMG_444:
        //DEBUG_JPEG("YUV444\n");
        JPEG_BRI_CTRL   = JPEG_BRI_MODE_DEC |
                          (DataType << 2) |
                          JPEG_BRI_DAT_FMT_YUV444;
        break;
    }

    JPEG_BRI_ADDR_Y = (u32)pResultY;
    JPEG_BRI_ADDR_C = (u32)pResultUV;
    JPEG_BRI_STRIDE = Stride;
    JPEG_BRI_SIZE   = jpegSize.width | (jpegSize.height << 16);

    /* decompression */
    JpegStreamAddr  = (u32)(pBuf);
    JpegImageAddr0  = (u32)(pResultY);

    JpegIntEna      = 0x07; // All interrupt enable

    jpegSemDec_flag = 1;
    jpegDecErr_flag = 0;

    //trigger
    JpegCtrl        = JpegCtrlTemp;
    JpegCtrl        = JpegCtrlTemp | 0x0002; // trigger decoder.

    OSSemPend(jpegSemEvt, JPEG_TIMEOUT, &err);

    sysJpegRst();

    if (err != OS_NO_ERR)
    {
        DEBUG_JPEG("Error: jpegSemEvt(decode) is %d.\n", err);
        return 0;
    }

    if(jpegDecErr_flag != 0) {
        DEBUG_JPEG("Error: JPEG decode error!!! jpegDecErr_flag is 0x%02x.\n", jpegDecErr_flag);
        return 0;
    }

    return 1;
}
#endif

/*

Routine Description:

    JPEG decompression with slice mode for big image.

Arguments:

    pBuf    - Bitstream buffer.
    pResult - Decompression image.

Return Value:

    0 - Failure.
    1 - Success.

*/
/*Peter 071022 S*/
s32 jpegDecompressionSlice(u8* pBuf , u8* pResult)
{

    INT8U   err;
    u8      *pResultTmp[2];
    s32     width, height, RemainderMcuNum, DecBlkMcuNum;
    s32     SliceHeight, RemainderHeight, i, format, SliceNum, TotalPix;
#if JPEG_TIME_STATISTIC
    INT32U  Begin_Time, End_Time, User_Time;
#endif

    DEBUG_JPEG("Trigger JPEG deocder with slice mode.\n");

#if JPEG_TIME_STATISTIC
    Begin_Time      = OSTimeGet();
#endif

    width           = jpegSize.width;
    height          = jpegSize.height;

    //memset(pResult, 0, width * height * 2);

    TotalPix        = width * height;
    if(TotalPix < 1048576)
        SliceNum    = 1;
    else if(TotalPix <  2097152)
        SliceNum    = 2;
    else if(TotalPix <  4194304)
        SliceNum    = 4;
    else if(TotalPix <  8388608)
        SliceNum    = 8;
    else if(TotalPix < 16777216)
        SliceNum    = 16;
    else
        SliceNum    = 32;
    SliceHeight     = height / SliceNum;

    // set decompress block mode mcu number
    format  = JpegCtrlTemp & JPEG_IMG_444;
    if (format == JPEG_IMG_420) {   // 420 mode
        width               = (width  + 15) & ~15;
        height              = (height + 15) & ~15;
        SliceHeight         = (SliceHeight + 15) & ~15;
        DecBlkMcuNum        = (width >> 4) * (SliceHeight >> 4);
        RemainderMcuNum     = (width >> 4) * (height >> 4);
    } else if (format == JPEG_IMG_422) {                            // 422 mode
        width               = (width  + 15) & ~15;
        height              = (height +  7) & ~7;
        SliceHeight         = (SliceHeight + 7) & ~7;
        DecBlkMcuNum        = (width >> 4) * (SliceHeight >> 3);
        RemainderMcuNum     = (width >> 4) * (height >> 3);
    } else if (format == JPEG_IMG_440) {                            // 440 mode
        width               = (width  +  7) & ~7;
        height              = (height + 15) & ~15;
        SliceHeight         = (SliceHeight + 15) & ~15;
        DecBlkMcuNum        = (width >> 3) * (SliceHeight >> 4);
        RemainderMcuNum     = (width >> 3) * (height >> 4);
    } else if (format == JPEG_IMG_444) {                            // 444 mode
        width               = (width  + 7) & ~7;
        height              = (height + 7) & ~7;
        SliceHeight         = (SliceHeight + 7) & ~7;
        DecBlkMcuNum        = (width >> 3) * (SliceHeight >> 3);
        RemainderMcuNum     = (width >> 3) * (height >> 3);
    }
    if(DecBlkMcuNum > 0x7fff) {
        DEBUG_JPEG("Error: DecBlkMcuNum is 0x%x > 0x7fff!!!\n", DecBlkMcuNum);
        sysJpegRst();
        return  0;
    } else {
        JpegRestartInterval    |= (DecBlkMcuNum << 16);
    }

    pResultTmp[0]   = pResult + 640 * 480 * 2;
    //pResultTmp[0]   = pResult;
    pResultTmp[1]   = pResultTmp[0] + width * SliceHeight * 2;
    //pResultTmp[2]   = pResultTmp[1] + width * SliceHeight * 2;
    //pResultTmp[3]   = pResultTmp[2] + width * SliceHeight * 2;

    /* decompression */
    JpegStreamAddr  = (u32)(pBuf);
    JpegImageAddr0  = (u32)(pResultTmp[0]);

    JpegIntEna      = 0x87; // All interrupt enable

    jpegSemDec_flag = 3;
    jpegDecErr_flag = 0;

    //trigger
    JpegCtrlTemp   |= JPEG_DEBLK_MODE_ENA;  // decode block mode
    JpegCtrl        = JpegCtrlTemp;
    JpegCtrl        = JpegCtrlTemp | JPEG_DEC_ENA; // trigger decoder.

    OSSemPend(jpegSemEvt, JPEG_TIMEOUT, &err);

    i   = 0;
    if (err != OS_NO_ERR) {
        DEBUG_JPEG("Error: jpegSemEvt(decode) is %d. i = %d\n", err, i);
        //JpegRestartInterval    &= 0x8000ffff;   // fix lose interrupt bug when disable slice decoder
        JpegCtrlTemp            = 0;
        sysJpegRst();   // reset JPEG hardware for fix PA9002C JPEG slice mode decoder bug
        return 0;
    }

    if(jpegDecErr_flag != 0) {
        DEBUG_JPEG("Error: JPEG decode error!!! jpegDecErr_flag is 0x%02x. i = %d\n", jpegDecErr_flag, i);
        sysJpegRst();   // reset JPEG hardware for fix PA9002C JPEG slice mode decoder bug
        return 0;
    }

    // scalar decompress image to 640 x 480 target
    isuScalar_D2D_Deblock(pResultTmp[0] , pResult, width, SliceHeight, 640, 480 * SliceHeight / height);
    pResult        += 640 * (480 * SliceHeight / height) * 2;
    RemainderHeight = 480 - 480 * SliceHeight / height;

    for(i = 1; (i < SliceNum && RemainderMcuNum > DecBlkMcuNum); i++) {
        // set decompress block mode mcu number
        RemainderMcuNum    -= DecBlkMcuNum;
        RemainderHeight     = 480 * SliceHeight / height;
        if(RemainderMcuNum < DecBlkMcuNum) {
            //DEBUG_JPEG("JPEG height(%d) / SliceNum(%d) is not multiple of %d.\n",
            //                height, SliceNum, (JpegCtrlTemp & JPEG_IMG_420) ? 16 : 8);
            DecBlkMcuNum        = RemainderMcuNum;
            JpegRestartInterval = (JpegRestartInterval & 0x8000ffff) | (DecBlkMcuNum << 16);
            SliceHeight         = height - SliceHeight * i;
        }

        /* decompression */
        JpegImageAddr0  = (u32)(pResultTmp[i & 1]);
        //JpegImageAddr0  = (u32)(pResultTmp[i]);

        JpegIntEna      = 0x87; // All interrupt enable

        jpegSemDec_flag = 3;
        jpegDecErr_flag = 0;

        //trigger
        JpegCtrlTemp   &= ~JPEG_DEC_ENA;  // clear decoder triger
        JpegCtrl        = JpegCtrlTemp | JPEG_DECBLK_TRG; // trigger decoder.

        OSSemPend(jpegSemEvt, JPEG_TIMEOUT, &err);

        if (err != OS_NO_ERR) {
            DEBUG_JPEG("Error: jpegSemEvt(decode) is %d. i = %d\n", err, i);
            //JpegRestartInterval    &= 0x8000ffff;   // fix lose interrupt bug when disable slice decoder
            JpegCtrlTemp            = 0;
            sysJpegRst();   // reset JPEG hardware for fix PA9002C JPEG slice mode decoder bug
            return 0;
        }

        if(jpegDecErr_flag != 0) {
            DEBUG_JPEG("Error: JPEG decode error!!! jpegDecErr_flag is 0x%02x. i = %d\n", jpegDecErr_flag, i);
            sysJpegRst();   // reset JPEG hardware for fix PA9002C JPEG slice mode decoder bug
            return 0;
        }

        // waiting for last time isu finish
        OSSemPend(isuSemEvt, ISU_TIMEOUT ,&err);
        if (err != OS_NO_ERR) {
            DEBUG_ISU("Error: isuSemEvt(isuScalar_D2D_Deblock) is %d. i = %d\n", err, i);
        }

        if(RemainderMcuNum == DecBlkMcuNum) {
            isuScalar_D2D_Deblock(pResultTmp[i & 1] , pResult, width, SliceHeight, 640, RemainderHeight);
            pResult    += 640 * RemainderHeight * 2;
        } else {
            isuScalar_D2D_Deblock(pResultTmp[i & 1] , pResult, width, SliceHeight, 640, 480 * SliceHeight / height);
            pResult    += 640 * (480 * SliceHeight / height) * 2;
        }
    }

    // waiting for last time isu finish
    OSSemPend(isuSemEvt, ISU_TIMEOUT ,&err);
    if (err != OS_NO_ERR) {
        DEBUG_ISU("Error: isuSemEvt(isuScalar_D2D_Deblock) is %d.\n", err);
    }

#if JPEG_TIME_STATISTIC
    End_Time    = OSTimeGet();
    User_Time   = End_Time - Begin_Time;
    DEBUG_JPEG("Trace: jpegDecompressionSlice() time = %d.\n", User_Time);
#endif

    //JpegRestartInterval    &= 0x8000ffff;   // fix lose interrupt bug when disable slice decoder
    JpegCtrlTemp            = 0;
    sysJpegRst();   // reset JPEG hardware for fix PA9002C JPEG slice mode decoder bug

    return 1;
}
#if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
/*

Routine Description:

    Initialize MJPEG encoder/decoder.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mjpgInit(void)
{
    int i;

    jpegInit();

    /* initialize video buffer */
    for(i = 0; i < VIDEO_BUF_NUM; i++) {         //暫時與mpeg4共用
        VideoBufMng[i].buffer = VideoBuf;

    }

    /* Create the semaphore */
    VideoTrgSemEvt = OSSemCreate(VIDEO_BUF_NUM - 2); /* guarded for ping-pong buffer */
	VideoCmpSemEvt = OSSemCreate(0);
    VideoCpleSemEvt= OSSemCreate(0);

    /* Create the task */
	OSTaskCreate(MJPG_TASK, MJPG_TASK_PARAMETER, MJPG_TASK_STACK, MJPG_TASK_PRIORITY);
	mjpgSuspendTask();

    return 1;
}


/*

Routine Description:

    The MJPG task.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
/*Lsk 090311 S*/
void mjpgTask(void* pData)
{
	u32* pFlag;
    s64* pTime;
    s64  Time;
    u32* pSize;
    u32  bitpos; /*BJ 0530 S*/
    u8   *pBuf, *pReadBuf;
    u8   err;
    u32  NextIdx;
    u32  TotalSAD;
    u32  outLen;
    u8   write2dispbuf_en;
    s32  i,DropFrame;
    u32  tempReg;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    /*** Set JPEG VLC Clock Divisor ***/
    jpegSetImageResolution(MJPEG_WIDTH, MJPEG_HEIGHT);  //Lsk : TV out size
    MJPG_Task_Go   = 1;    // 0: never run, 1: ever run



	while (1)
	{
        MJPEG_Pend  = 1;
		OSSemPend(VideoTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
        MJPEG_Pend  = 0;
		if(MJPG_Mode) {
            pBuf        = VideoBufMng[VideoBufMngReadIdx].buffer;  /*BJ 0530 S*/
            pFlag       = &VideoBufMng[VideoBufMngReadIdx].flag;
            Time        = VideoBufMng[VideoBufMngReadIdx].time;
            pSize       = &VideoBufMng[VideoBufMngReadIdx].size;

            write2dispbuf_en    = 1;
			#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
			if(!ResetPlayback && (sysPlaybackForward == 0))
			#elif (AVSYNC == VIDEO_FOLLOW_AUDIO)
            if((sysPlaybackForward == SYS_PLAYBACK_FORWARD_X1) ||
               ((sysPlaybackForward != SYS_PLAYBACK_FORWARD_X1) && (Vop_Type == I_VOP)))
			#endif
            {
                mjpgDecoding1Frame(pBuf, *pSize, write2dispbuf_en);

                if(write2dispbuf_en)//不啟動scaler engine. Mpeg decoding 直接decoding to display buffer.
                {
                    isuStatus_OnRunning = 0;
                    IsuIndex++;
                }
                else
                {
                    // scaling MPEG-4 image to fit display format
                    if(IsuIndex != 0)
                    {
                        OSSemPend(isuSemEvt, 10 ,&err);
                     #if DINAMICALLY_POWER_MANAGEMENT
                        sysISU_disable();
                     #endif
                        if (err != OS_NO_ERR) {
                            DEBUG_MP4("Error: isuSemEvt(playback mode) is %d.\n", err);
                        }
                    }
                }

                //DEBUG_MP4("IsuIndex = %d\n", IsuIndex);
				#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
				while(((IsuIndex >= (MainVideodisplaybuf_idx + DISPLAY_BUF_NUM - 2)) || (isuStatus_OnRunning == 1)) && !ResetPlayback && sysPlaybackVideoStop == 0) /*Peter 1113 S*/    //Lsk 090410 check it    //Lsk 090417 : avoid deadlock when press stop playback
				#elif (AVSYNC == VIDEO_FOLLOW_AUDIO)
                while((IsuIndex >= (MainVideodisplaybuf_idx + DISPLAY_BUF_NUM - 2)) || (isuStatus_OnRunning == 1)) /*Peter 1113 S*/
				#endif
                {
                    OSTimeDly(1);
                }
                Videodisplaytime[IsuIndex % DISPLAY_BUF_NUM]    = Time;
            }

		}
		else {
			pBuf        = VideoBufMng[VideoBufMngWriteIdx].buffer;  /*BJ 0530 S*/
            pTime       = &VideoBufMng[VideoBufMngWriteIdx].time;
            pSize       = &VideoBufMng[VideoBufMngWriteIdx].size;
            pFlag       = &VideoBufMng[VideoBufMngReadIdx].flag;
            *pFlag = 1;//FLAG_I_VOP

			//------Video FIFO Management: 避免 Read/Write pointer overlay. ------//
            //if((asfCaptureMode == ASF_CAPTURE_OVERWRITE) || (asfCaptureMode == ASF_CAPTURE_NORMAL)) {
            if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
                if((WantChangeFile == 1) && (GetLastVideo == 0) && (GetLastAudio)) {
                    OS_ENTER_CRITICAL();
                    LastVideo       	= VideoBufMngWriteIdx;
                    GetLastVideo        = 1;
                    OS_EXIT_CRITICAL();
                }
            }

            pReadBuf    = VideoBufMng[VideoBufMngReadIdx].buffer;
            if(VideoPictureIndex != 0 && (pReadBuf >= pBuf))
            {
                while(((pReadBuf - pBuf) < MPEG4_MIN_BUF_SIZE) &&
                      (VideoCmpSemEvt->OSEventCnt > 2) &&
                      (sysCaptureVideoStop == 0))
                {

                    //DEBUG_JPEG("MP4 buffer = %d bytes\n", pReadBuf - pBuf);
                    DEBUG_MP4("j");
                    OSTimeDly(1);
                    pReadBuf    = VideoBufMng[VideoBufMngReadIdx].buffer;
                }
            }

			//---------- Video Encoding---------//
            *pTime=0; *pSize = 0;

        	//Lucian: frame rate control
            if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_30)
            {
               DropFrame=0;
            }
            else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_15)
            {
               DropFrame=1;
            }
            else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_5)
            {
               DropFrame=5;
            }

            for(i=0;i<DropFrame;i++)
            {
                OSSemPend(isuSemEvt, 10 ,&err);
                if(err != OS_NO_ERR)
                    DEBUG_JPEG("Pend isuSemEvt error\n");
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM];
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }

            mjpgCoding1Frame(pBuf, pSize);
            *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM];
            //-------------Video FIFO management: 計算下一個Video frame start address-------------//
            OS_ENTER_CRITICAL();
            CurrentVideoSize   += *pSize;
            OS_EXIT_CRITICAL();

            NextIdx = (VideoBufMngWriteIdx + 1) % VIDEO_BUF_NUM;

            pBuf    = (u8*)(((u32)pBuf + *pSize + 3) & ~3);

            if(pBuf > mpeg4VideBufEnd)
            {
                DEBUG_JPEG("VideoBuf overflow!!!!\n");
            }

            if(pBuf < (mpeg4VideBufEnd - MPEG4_MIN_BUF_SIZE))
                VideoBufMng[NextIdx].buffer = pBuf;
            else
                VideoBufMng[NextIdx].buffer = VideoBuf;

            VideoBufMngWriteIdx = NextIdx;

            VideoPictureIndex++;
#if ISU_OVERLAY_ENABLE
            sysbackSetEvt(SYS_BACK_DRAWTIMEONVIDEOCLIP, 0);
#endif
		}
		OSSemPost(VideoCmpSemEvt);
	}
}

s32 MJPG_Enc(u16 Width,        // Image width
                  u16 Height,       // Image height
                  s16 Format,       // 0: YUV422, 1: YUV420
                  s16 restart,      // Restart interval
                  s16 slice_mode,   // 1: Slice mode, 0: frame mode
                  u8  *bufAddr_0,   // Image address 0, for both frame mode and slice mode, must align 32. Y data, if DataType == YUV420.
                  u8  *bufAddr_1,   // Image address 1, for slice mode only, must align 32. UV data, if DataType == YUV420.
                  u8  *bufAddr_2,   // Image address 2, for slice mode only, must align 32
                  u8  *poutBuf,     // Bitstream address, must align 4
                  u32 DataType,     // for A1018, image data type, 0:YUV422, 1:YUV420.
                  u32 Stride)       // for A1018, stride in bytes.
{
    s32     jpegCmpSize;
    u32     mcuSize;

    sysJpegRst();

#if(CHIP_OPTION >= CHIP_A1018A)
    JPEG_BRI_CTRL   = JPEG_BRI_MODE_ENC |
                      (DataType << 2) |
                      (Format << 3);
    JPEG_BRI_ADDR_Y = (u32)bufAddr_0;
    JPEG_BRI_ADDR_C = (u32)bufAddr_1;
    JPEG_BRI_STRIDE = Stride;
    JPEG_BRI_SIZE   = Width | (Height << 16);
#endif

    // Set image size
    JpegImageSize = (Height<<JPEG_IMAGE_SIZE_Y_SHFT) | Width;

    // Set and calculate the number of MCU
    jpegCmpSize = (u32) Height * (u32) Width;
    mcuSize = jpegCmpSize>>7;
    if (Format == JPEG_FORMAT_420)
       mcuSize = mcuSize>>1;
    JpegCtrlTemp    = (Format << JPEG_IMG_SHFT) |
                      (slice_mode<<JPEG_OPBUF_SHFT)|
                      (mcuSize<<JPEG_MCU_NO_SHFT);

    if (Format == JPEG_FORMAT_420)
    {
        //DEBUG_JPEG("Coding format is YUV420 \n");
    }
    else if (Format == JPEG_FORMAT_422)
    {
        //DEBUG_JPEG("Coding format is YUV422 \n") ;
    }
    else
    {
        DEBUG_JPEG("Error: Coding format is unknown.\n") ;
        exit(0) ;
    }


    // Set restart interval
/*Peter 0829 S*/
    if(restart) {
        JpegRestartInterval = (restart & 0x0000ffff) | JPEG_DRI_EN;
    } else {
        JpegRestartInterval = 0;
    }
/*Peter 0829 E*/

    // set JPEG bitstream starting address and default size
    JpegStreamAddr  = (u32) poutBuf;
    JpegStreamSize  = jpegCmpSize;

    // set input image atarting address
    JpegImageAddr0  = (u32) bufAddr_0;
    JpegImageAddr1  = (u32) bufAddr_1;
    JpegImageAddr2  = (u32) bufAddr_2;

    // set JPEG interrupt when coding an image is complete
    JpegIntStat = 0;
    JpegIntEna = JPEG_CMP_INT_ENA;

    // start JPEG hardware
    JpegCtrl        = JpegCtrlTemp;
    AHB_ARBCtrl |= (SYS_ARBHIPIR_JPGVLC | SYS_ARBHIPIR_JPGDCT);     //AHB ARBITER control register

    JpegCtrl        = JpegCtrlTemp | JPEG_ENC_ENA;

    return 1;
}
s32 mjpgCapturePreviewImg(u8* pBitsBuf,u8* pImgBuf, s16 JpegOpMode,u16 ImgWidth,u16 ImgHeight)
{

    EXIF_DQT* pPrimaryDqt = &exifMJPGImage.dqt; /*CY 0907*/

    /* set encode quantization table of thumbnail image */  /*CY 0907*/
    jpegSetQuantizationTable(JPEG_COMPONENT_Y, JPEG_QUANTIZATION_PRECISION_8, (u8*)pPrimaryDqt->lumQ, 0);
    jpegSetQuantizationTable(JPEG_COMPONENT_Cb, JPEG_QUANTIZATION_PRECISION_8, (u8*)pPrimaryDqt->chrQ, 0);

    jpegColorFmat = JPEG_FORMAT_420;
    exifMJPGImage.sof0.yHV   = 0x21 + jpegColorFmat;
    #if MJPG_DRI_ENABLE
    jpegRestart = bSwap16(exifMJPGImage.dri.Ri);
    #else
    jpegRestart = 0;
    #endif
    // set huffman tables
    jpegSetHuffmanEncodeTable();

    // start JPEG hardware compression
    MJPG_Enc( ImgWidth,                     // Image width
              ImgHeight,                    // Image height
              jpegColorFmat,                // 0: YUV422, 1: YUV420
              jpegRestart,                  // Restart interval
              JpegOpMode,                   // 1: Slice mode, 0: frame mode
              pImgBuf,                      // Image address 0, for both frame mode and slice mode, must align 32. Y data, if DataType == YUV420.
              PKBuf1,                       // Image address 1, for slice mode only, must align 32. UV data, if DataType == YUV420.
              PKBuf2,                       // Image address 2, for slice mode only, must align 32
              pBitsBuf,                     // Bitstream address, must align 4
              0,                            // for A1018, image data type, 0:YUV422, 1:YUV420.
              ImgWidth * 2);                // for A1018, stride in bytes.

    return 1;
}

s32 mjpgDecoding1Frame(u8* pBuf, u32 size, u8 write2dispbuf_en)
{

    INT8U err;
    u32 sizeUsed;
    u16 sizeX, sizeY;

    /*** Header parse ***/
    if(ReadOnce==0)
    {
        if(exifFileParse(JPEG_IMAGE_PRIMARY, pBuf, size, &sizeUsed, &sizeX, &sizeY) == 0) {
            DEBUG_DCF("Error: exifFileParse fail!!!\n");
            return 0;
        }
        ReadOnce = 1;
    }
    else
    {
        sizeUsed = 634; //fixed header size
    }

    /* decompression */
    JpegStreamAddr = (u32)(pBuf+sizeUsed);
    // set display buffer
    if(write2dispbuf_en)
    {
       JpegImageAddr0 = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM];
    }
    else
    {
       JpegImageAddr0 = (u32) mpeg4outputbuf[VideoPictureIndex % 3];
    }



    JpegIntEna = JPEG_DCMP_INT_ENA | JPEG_DCMP_ERR_INT_ENA; // All interrupt enable

    jpegSemDec_flag = 1;
    jpegDecErr_flag = 0;

    AHB_ARBCtrl |= (SYS_ARBHIPIR_JPGVLC | SYS_ARBHIPIR_JPGDCT);     //AHB ARBITER control register
    //trigger
    JpegCtrl        = JpegCtrlTemp;
    JpegCtrl        = JpegCtrlTemp | JPEG_DEC_ENA; // trigger decoder.

    OSSemPend(jpegSemEvt, JPEG_TIMEOUT, &err);
    //sysJpegRst();

    if (err != OS_NO_ERR)
    {
        DEBUG_JPEG("Error: jpegSemEvt(decode) is %d.\n", err);
        DEBUG_JPEG("JpegStreamSize = %d\n", JpegStreamSize);
        DEBUG_JPEG("SYS_CTL0 = 0x%08x\n", SYS_CTL0);
        return 0;
    }

    if(jpegDecErr_flag != 0) {
        DEBUG_JPEG("Error: JPEG decode error!!! jpegDecErr_flag is 0x%02x.\n", jpegDecErr_flag);
        return 0;
    }

    return 1;
}
/*

Routine Description:

    Capture image.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 mjpgCoding1Frame(u8* FrameBuf, u32* CmpSize)
{
	u8 err;
    s32 ZoomFactor = 0;
	u32 primaryImageSize;
    u32 thumbnailImageSize;

    u32 JpegImagePixelCount;
    FS_DISKFREE_T* diskInfo;  //civic 070903
    u32 used_cluster,i;
    u32 bytes_per_cluster;
    u16 data;
    u32 iduWinReg;

    u8 word[4];

    //u8 quality_control=0x9;
    RTC_DATE_TIME curDateTime = { ' ', ' ', ' ', ' ', ' ', ' ' }; /* 2005, Dec. 24, 23:59:59 */

    u32 Begin_Time, End_Time;

#if DINAMICALLY_POWER_MANAGEMENT
    u32     sys_ctl0_status;
#endif

    u8* buffer[3];  //idu buffer address
	buffer[0]=PKBuf0;
	buffer[1]=PKBuf1;
	buffer[2]=PKBuf2;

	OSSemPend(isuSemEvt, 10 ,&err);

	if (err != OS_NO_ERR)
    {
        DEBUG_JPEG("Error: isuSemEvt(video capture mode) is %d.\n", err);
        DEBUG_JPEG("isu_int_status = 0x%08x\n", isu_int_status);
    }
    /*avoid warning message*/
    if (iduWinReg || i){}

    Begin_Time=OS_tickcounter;

    FrameBuf[0]=0xff; FrameBuf[1]=0xd8;//EXIF_MARKER_SOI
	*CmpSize = 2;

    exifMJPGImage.sof0.X = bSwap16(jpegSize.width);
    exifMJPGImage.sof0.Y = bSwap16(jpegSize.height);
    #if MJPG_DRI_ENABLE
    exifMJPGImage.dri.Ri = bSwap16((jpegSize.width) / 16);
    #endif

	CopyMemory((u8*)(FrameBuf+(*CmpSize)),(unsigned char*)&exifMJPGImage, exifMJPGImageHeaderSize);
	*CmpSize += exifMJPGImageHeaderSize;
    //In encoder, if BITSTREAM_BASE isn't multiple of 4, the lower bytes in first word will be filled zero.
    if(((*CmpSize)%4) != 0)
    {
        for(i=0; i<((*CmpSize)%4); i++)
        {
            word[i] = *(u8*)(FrameBuf+(*CmpSize)-1-i);
            //DEBUG_JPEG("1.word = 0x%02x ",word[i]);
        }
        //DEBUG_JPEG("\n");
    }


    mjpgCapturePreviewImg((u8*)(FrameBuf+(*CmpSize)), buffer[(isu_idufrmcnt+2) % 3], JPEG_OPMODE_FRAME, jpegSize.width, jpegSize.height);
    //mjpgCapturePreviewImg((u8*)(FrameBuf+(*CmpSize)), buffer[(VideoPictureIndex+2) % 3], JPEG_OPMODE_FRAME, jpegSize.width, jpegSize.height);
    primaryImageSize=WaitJpegEncComplete();

    //In encoder, if BITSTREAM_BASE isn't multiple of 4, the lower bytes in first word will be filled zero.
    if(((*CmpSize)%4) != 0)
    {
        for(i=0; i<((*CmpSize)%4); i++)
        {
            *(u8*)(FrameBuf+(*CmpSize)-1-i) = word[i];
            //DEBUG_JPEG("2.word = 0x%02x ",*(u8*)(FrameBuf+(*CmpSize)-1-i));
        }
        //DEBUG_JPEG("\n");
    }

    *CmpSize += primaryImageSize;
    JpegImagePixelCount=GetJpegImagePixelCount();

    End_Time=OS_tickcounter;
    Begin_Time=End_Time;

    End_Time=OS_tickcounter;
    Begin_Time=End_Time;


    return 1;
}

/*Lsk 090311 E*/
/*

Routine Description:

    Resume MJPG task.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mjpgResumeTask(void)
{
    OSTaskResume(MJPG_TASK_PRIORITY);
    return 1;
}

/*

Routine Description:

    Suspend MJPG task.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mjpgSuspendTask(void)
{
    /* Suspend the task */
  	MJPG_Task_Go   = 0;    // 0: never run, 1: ever run  //Lsk 090622
    OSTaskSuspend(MJPG_TASK_PRIORITY);
    return 1;
}

#endif
/*Peter 071022 E*/
