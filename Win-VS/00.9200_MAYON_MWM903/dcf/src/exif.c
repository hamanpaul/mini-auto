/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    exif.c

Abstract:

    The routines of EXIF.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2005/08/26  David Tsai  Create  

*/

#include "general.h"
#include "board.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "jpegapi.h"
#include "isuapi.h"/*BJ 0523 S*/
#include "iduapi.h"/*BJ 0523 S*/
#include "ipuapi.h"
#include "siuapi.h"
#include "sysapi.h"
#include "dcfapi.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */
 
/* define debug print */
#define EXIF_IFDS_EXIF_IFD      0
#define EXIF_IFDS_GPS_INFO_IFD      1
#define EXIF_IFDS_INTEROP_IFD       2
#define EXIF_IFDS_MAX_IFD       3   

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

/*BJ 0523 S*/

/*BJ 0523 E*/

s8 exifDecChar[] = "0123456789";
 
u32 exifIfdsPointer[EXIF_IFDS_MAX_IFD];
u32 exifThumbnailPointer;
u32 exifThumbnailLength;
u8  exifAPP3VGAImg_flag;

// for JPEG decoder quantization
u8  Qtable[4][64];
u8  TQ[4], C[3];

/*BJ 0523 S*/
u8 displaybuf_idx = 0;
/*BJ 0523 E*/

u16   gJPGDecodedWidth, gJPGDecodedHeight;
u16   gJPGValidWidth, gJPGValidHeight;
	
	
/*Peter 0829 S*/
extern const u8 jpegZigzag[64];
extern u8 playbackflag;
extern u8 uiMenuEnable;
extern FS_DISKFREE_T global_diskInfo;
extern u8 scaler2sd;
extern u8 TvOutMode;

extern u32 Global_APP3VGAImageSize;

#if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
u32	EXIF_DRI_SIZE = sizeof(EXIF_DRI);
u32	EXIF_DQT_SIZE = sizeof(EXIF_DQT);
u32	EXIF_SOF0_SIZE= sizeof(EXIF_SOF0);
u32	EXIF_SOS_SIZE = sizeof(EXIF_SOS);

#endif    
/*Peter 0829 E*/

/*CY 0601 S*/
__align(4) EXIF_THUMBNAIL_IMAGE exifThumbnailImage =
{
    EXIF_MARKER_SOI,                /* SOI */           /* bSwap16() */
    {                       /* app1 */
        EXIF_MARKER_APP1,               /* APP1 */      /* bSwap16() */
        0,                      /* Lp - TBD */      /* bSwap16() */
        {                       /* identifier[5] */
           'E',  'x',  'i',  'f', '\0',
        },
        0x00,                       /* pad */
        {                       /* tiffHeader */
            EXIF_TIFF_LITTLE_ENDIAN,            /* byteOrder */
            EXIF_TIFF_VERSION_NUMBER,           /* versionNumber */
            EXIF_TIFF_OFFS_TO_IFD0,             /* offsetToIfd0 */
        },
        {                       /* ifd0 */
            EXIF_IFD0_NUM_INTEROP,          /* numInterop */
            {                       /* imageDescription */
                EXIF_TAG_IMAGE_DESCRIPTION,         /* tag */ 
                EXIF_TYPE_ASCII,                /* type */
                EXIF_IFD0_IMAGE_DESCRIPTION_VALUE_COUNT,    /* count */
                EXIF_IFD0_IMAGE_DESCRIPTION_VALUE_OFFSET,   /* valueOffset */                   
            },       
            {                       /* make */
                EXIF_TAG_MAKE,                  /* tag */ 
                EXIF_TYPE_ASCII,                /* type */
                EXIF_IFD0_MAKE_VALUE_COUNT,         /* count */
                EXIF_IFD0_MAKE_VALUE_OFFSET,            /* valueOffset */
            },      
            {                       /* model */
                EXIF_TAG_MODEL,                 /* tag */ 
                EXIF_TYPE_ASCII,                /* type */
                EXIF_IFD0_MODEL_VALUE_COUNT,            /* count */
                EXIF_IFD0_MODEL_VALUE_OFFSET,           /* valueOffset */
            },      
            {                       /* orientation */
                EXIF_TAG_ORIENTATION,               /* tag */ 
                EXIF_TYPE_SHORT,                /* type */
                0x00000001,                 /* count */
                EXIF_VAL_ORIENTATION_TOP_LEFT,      /* valueOffset */
            },      
            {                       /* xResolution */
                EXIF_TAG_X_RESOLUTION,              /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD0_X_RESOLUTION_VALUE_OFFSET,        /* valueOffset */
            },
            {                       /* yResolution */
                EXIF_TAG_Y_RESOLUTION,              /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD0_Y_RESOLUTION_VALUE_OFFSET,        /* valueOffset */
            },              
            {                       /* resolutionUnit */
                EXIF_TAG_RESOLUTION_UNIT,           /* tag */ 
                EXIF_TYPE_SHORT,                /* type */
                0x00000001,                 /* count */
                EXIF_VAL_RESOLUTION_UNIT_INCHES,        /* valueOffset */
            },  
            {                       /* dateTime */
                EXIF_TAG_DATE_TIME,             /* tag */ 
                EXIF_TYPE_ASCII,                /* type */
                EXIF_IFD0_DATE_TIME_VALUE_COUNT,        /* count */
                EXIF_IFD0_DATE_TIME_VALUE_OFFSET,       /* valueOffset */
            },  
            {                       /* yCbCrPositioning */
                EXIF_TAG_YCbCr_POSITIONING,         /* tag */ 
                EXIF_TYPE_SHORT,                /* type */
                0x00000001,                 /* count */
                EXIF_VAL_YCbCr_POSITIONING_COSITED,     /* valueOffset */
            },  
            {                       /* copyright */
                EXIF_TAG_COPYRIGHT,             /* tag */ 
                EXIF_TYPE_ASCII,                /* type */
                EXIF_IFD0_COPYRIGHT_VALUE_COUNT,        /* count */
                EXIF_IFD0_COPYRIGHT_VALUE_OFFSET,       /* valueOffset */
            },  
            {                       /* exifIfdPointer */
                EXIF_TAG_EXIF_IFD_POINTER,          /* tag */ 
                EXIF_TYPE_LONG,                 /* type */
                0x00000001,                 /* count */
                EXIF_IFD0_EXIF_IFD_POINTER,         /* valueOffset */
            },  
            EXIF_IFD0_NEXT_IFD_OFFSET,          /* nextIfdOffset */
        },
        {                       /* ifd0Value */
            {                       /* imageDescriptionValue[] */
               'H',  'i',  'm',  'a',  'x',  ' ',  'i',  'm',   
               'a',  'g',  'e', '\0', '\0', '\0',
            },
            {                       /* makeValue[] */
               'S',  'A',  'L',  'I',  'X',  ' ',  'I',  'n',
               'c', '\0',   
            },
            {   
                                /* modelValue[] */
			   'M',  'R',  '6',  '7',  '2',  '0',  '\0', '\0',
            },  
            { 0x00000048, 0x00000001 },         /* xResolutionValue[] */
            { 0x00000048, 0x00000001 },         /* yResolutionValue[] */
            {                       /* dateTimeValue[] */
               '2',  '0',  '0',  '6',  ':',  '1',  '2',  ':',
               '2',  '5',  ' ',  '0',  '0',  ':',  '0',  '0',
               ':',  '0',  '0', '\0',
            }, 
            {                       /* copyrightValue[] */
               'S',  'A',  'L',  'I',  'X',  ' ',  ' ',  'S',
               'D',  'V',  '-',  '2',  '_',  '0',  '8',
               '1',  '0',  '0', '1', '_', '1','\0',
            }, 
        },  
        {                       /* ifd0e */
            EXIF_IFD0E_NUM_INTEROP,         /* numInterop */
            {                       /* exposureTime */
                EXIF_TAG_EXPOSURE_TIME,             /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD0E_EXPOSURE_TIME_VALUE_OFFSET,      /* valueOffset */                   
            },  
            {                       /* fNumber */
                EXIF_TAG_F_NUMBER,              /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD0E_F_NUMBER_VALUE_OFFSET,       /* valueOffset */                   
            },
            {                       /* exifVersion */
                EXIF_TAG_EXIF_VERSION,              /* tag */ 
                EXIF_TYPE_UNDEFINED,                /* type */
                0x00000004,                 /* count */
                EXIF_VAL_EXIF_VERSION,              /* valueOffset */                   
            },      
            {                       /* dateTimeOriginal */
                EXIF_TAG_DATE_TIME_ORIGINAL,            /* tag */ 
                EXIF_TYPE_ASCII,                /* type */
                EXIF_IFD0E_DATE_TIME_ORIGINAL_VALUE_COUNT,  /* count */
                EXIF_IFD0E_DATE_TIME_ORIGINAL_VALUE_OFFSET, /* valueOffset */                   
            },      
            {                       /* dateTimeOriginal */
                EXIF_TAG_DATE_TIME_DIGITIZED,           /* tag */ 
                EXIF_TYPE_ASCII,                /* type */
                EXIF_IFD0E_DATE_TIME_DIGITIZED_VALUE_COUNT, /* count */
                EXIF_IFD0E_DATE_TIME_DIGITIZED_VALUE_OFFSET,    /* valueOffset */                   
            },  
            {                       /* componentsConfiguration */
                EXIF_TAG_COMPONENTS_CONFIGURATION,      /* tag */ 
                EXIF_TYPE_UNDEFINED,                /* type */
                0x00000004,                 /* count */
                EXIF_VAL_COMPONENTS_CONFIGURATION_YCbCr,        /* valueOffset */                   
            },  
            {                       /* compressedBitsPerPixel */
                EXIF_TAG_COMPRESSED_BITS_PER_PIXEL,     /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD0E_COMPRESSED_BITS_PER_PIXEL_VALUE_OFFSET, /* valueOffset */                    
            },
            {                       /* shutterSpeedValueValue */
                EXIF_TAG_SHUTTER_SPEED_VALUE,           /* tag */ 
                EXIF_TYPE_SRATIONAL,                /* type */
                0x00000001,                 /* count */
                EXIF_IFD0E_SHUTTER_SPEED_VALUE_VALUE_OFFSET,    /* valueOffset */                   
            },
            {                       /* apertureValueValue */
                EXIF_TAG_APERTURE_VALUE,            /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD0E_APERTURE_VALUE_VALUE_OFFSET,     /* valueOffset */                   
            },
            {                       /* brightnessValueValue */
                EXIF_TAG_BRIGHTNESS_VALUE,          /* tag */ 
                EXIF_TYPE_SRATIONAL,                /* type */
                0x00000001,                 /* count */
                EXIF_IFD0E_BRIGHTNESS_VALUE_VALUE_OFFSET,   /* valueOffset */                   
            },
            {                       /* exposureBiasValueValue */
                EXIF_TAG_EXPOSURE_BIAS_VALUE,           /* tag */ 
                EXIF_TYPE_SRATIONAL,                /* type */
                0x00000001,                 /* count */
                EXIF_IFD0E_EXPOSURE_BIAS_VALUE_VALUE_OFFSET,    /* valueOffset */                   
            },
            {                       /* maxApertureValueValue */
                EXIF_TAG_MAX_APERTURE_RATIO_VALUE,      /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD0E_MAX_APERTURE_VALUE_VALUE_OFFSET,     /* valueOffset */                   
            },
            {                       /* subjectDistanceValue */
                EXIF_TAG_SUBJECT_DISTANCE,          /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD0E_SUBJECT_DISTANCE_VALUE_OFFSET,   /* valueOffset */                   
            },
            {                       /* meteringMode */
                EXIF_TAG_METERING_MODE,             /* tag */ 
                EXIF_TYPE_SHORT,                /* type */
                0x00000001,                 /* count */
                EXIF_VAL_METERING_MODE_CENTER_WEIGHT_AVERAGE,   /* valueOffset */                   
            },
            {                       /* lightSource */
                EXIF_TAG_LIGHT_SOURCE,              /* tag */ 
                EXIF_TYPE_SHORT,                /* type */
                0x00000001,                 /* count */
                EXIF_VAL_LIGHT_SOURCE_DAYLIGHT,         /* valueOffset */                   
            },
            {                       /* flash */
                EXIF_TAG_FLASH,                 /* tag */ 
                EXIF_TYPE_SHORT,                /* type */
                0x00000001,                 /* count */
                EXIF_VAL_FLASH_FIRED_NO,            /* valueOffset */                   
            },
            {                       /* focalLength */
                EXIF_TAG_FOCAL_LENGTH,              /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD0E_FOCAL_LENGTH_VALUE_OFFSET,       /* valueOffset */                   
            },
            {                       /* userComment */
                EXIF_TAG_USER_COMMENT,              /* tag */ 
                EXIF_TYPE_UNDEFINED,                /* type */
                EXIF_IFD0E_USER_COMMENT_VALUE_COUNT,        /* count */
                EXIF_IFD0E_USER_COMMENT_VALUE_OFFSET,       /* valueOffset */                   
            },  
            {                       /* subsecTime */
                EXIF_TAG_SUBSEC_TIME,               /* tag */ 
                EXIF_TYPE_ASCII,                /* type */
                0x00000004,                 /* count */
                0x00303030,                 /* valueOffset - "000\0" */
            },
            {                       /* subsecTimeOriginal */
                EXIF_TAG_SUBSEC_TIME_ORIGINAL,          /* tag */ 
                EXIF_TYPE_ASCII,                /* type */
                0x00000004,                 /* count */
                0x00303030,                 /* valueOffset - "000\0" */
            },
            {                       /* subsecTimeDigitized */
                EXIF_TAG_SUBSEC_TIME_DIGITIZED,         /* tag */ 
                EXIF_TYPE_ASCII,                /* type */
                0x00000004,                 /* count */
                0x00303030,                 /* valueOffset - "000\0" */
            },
            {                       /* flashpixVersion */
                EXIF_TAG_FLASHPIX_VERSION,          /* tag */ 
                EXIF_TYPE_UNDEFINED,                /* type */
                0x00000004,                 /* count */
                EXIF_VAL_FLASHPIX_VERSION,          /* valueOffset */                   
            },  
            {                       /* colorSpace */
                EXIF_TAG_COLOR_SPACE,               /* tag */ 
                EXIF_TYPE_SHORT,                /* type */
                0x00000001,                 /* count */
                EXIF_VAL_COLOR_SPACE_sRGB,          /* valueOffset */                   
            },
            {                       /* pixelXDimension */
                EXIF_TAG_PIXEL_X_DIMENSION,         /* tag */ 
                EXIF_TYPE_LONG,                 /* type */
                0x00000001,                 /* count */
                0x00000640,                 /* valueOffset - TBD */ 
            },
            {                       /* pixelYDimension */
                EXIF_TAG_PIXEL_Y_DIMENSION,         /* tag */ 
                EXIF_TYPE_LONG,                 /* type */
                0x00000001,                 /* count */
                0x000004b0,                 /* valueOffset - TBD */ 
            },
            EXIF_IFD0E_NEXT_IFD_OFFSET,         /* nextIfdOffset */
        },  
        {                       /* ifd0eValue */
            { 0x00000001, 0x0000003c },         /* exposureTimeValue[] */
            { 0x0000001c, 0x0000000a },         /* fNumberValue[] */
            {                       /* dateTimeOriginalValue[] */
               '2',  '0',  '0',  '7',  ':',  '1',  '2',  ':',   
               '2',  '5',  ' ',  '0',  '0',  ':',  '0',  '0',
               ':',  '0',  '0', '\0',
            },
            {                       /* dateTimeDigitizedValue[] */
               '2',  '0',  '0',  '5',  ':',  '1',  '2',  ':',   
               '2',  '5',  ' ',  '0',  '0',  ':',  '0',  '0',
               ':',  '0',  '0', '\0',
            },
            { 0x00000005, 0x00000001 },         /* compressedBitsPerPixelValue[] */
            { 0x00000006, 0x00000001 },         /* shutterSpeedValueValue[] */
            { 0x00000003, 0x00000001 },         /* apertureValueValue[] */
            { 0x00000000, 0x00000001 },         /* brightnessValueValue[] */
            { 0x00000000, 0x00000001 },         /* exposureBiasValueValue[] */
            { 0x0000000a, 0x0000004b },         /* maxApertureValueValue[]:1/7.5 sec */
            { 0x00000003, 0x00000001 },         /* subjectDistanceValue[]: 3M */
            { 0x00000023, 0x00000001 },         /* focalLengthValue[]: 35 mm */
            {                       /* userCommentValue[] */
               'A',  'S',  'C',  'I',  'I', '\0', '\0', '\0',   
               'H',  'i',  'm',  'a',  'x',  ' ',  'T',  'e',
               'c',  'h',  'n',  'o',  'l',  'o',  'g',  'y',
               ',',  ' ',  'I',  'n',  'c',  '.', '\0', '\0',
              '\0', '\0', '\0', '\0', '\0', '\0', '\0',
            },
        },
        {                       /* ifd1 */
            EXIF_IFD1_NUM_INTEROP,          /* numInterop */
            {                       /* compression */
                EXIF_TAG_COMPRESSION,               /* tag */ 
                EXIF_TYPE_SHORT,                /* type */
                0x00000001,                 /* count */
                EXIF_VAL_COMPRESSION_JPEG,          /* valueOffset */                   
            },
            {                       /* xResolution */
                EXIF_TAG_X_RESOLUTION,              /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD1_X_RESOLUTION_VALUE_OFFSET,        /* valueOffset */
            },
            {                       /* yResolution */
                EXIF_TAG_Y_RESOLUTION,              /* tag */ 
                EXIF_TYPE_RATIONAL,             /* type */
                0x00000001,                 /* count */
                EXIF_IFD1_Y_RESOLUTION_VALUE_OFFSET,        /* valueOffset */
            },              
            {                       /* resolutionUnit */
                EXIF_TAG_RESOLUTION_UNIT,           /* tag */ 
                EXIF_TYPE_SHORT,                /* type */
                0x00000001,                 /* count */
                EXIF_VAL_RESOLUTION_UNIT_INCHES,        /* valueOffset */
            },  
            {                       /* jpegInterchangeFormat */
                EXIF_TAG_JPEG_INTERCHANGE_FORMAT,       /* tag */ 
                EXIF_TYPE_LONG,                 /* type */
                0x00000001,                 /* count */
                EXIF_IFD1_JPEG_INTERCHANGE_FORMAT,          /* valueOffset */                   
            },
            {                       /* jpegInterchangeFormatLength */
                EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH,    /* tag */ 
                EXIF_TYPE_LONG,                 /* type */
                0x00000001,                 /* count */
                0x00000000,                 /* valueOffset - TBD */                 
            },      
            {                       /* yCbCrPositioning */
                EXIF_TAG_YCbCr_POSITIONING,         /* tag */ 
                EXIF_TYPE_SHORT,                /* type */
                0x00000001,                 /* count */
                EXIF_VAL_YCbCr_POSITIONING_COSITED,     /* valueOffset */
            },  
        },  
        {                       /* ifd1Value */
            { 0x00000048, 0x00000001 },         /* xResolutionValue[] */
            { 0x00000048, 0x00000001 },         /* yResolutionValue[] */
        },  
        {                   /* thumbnail */
            EXIF_MARKER_SOI,            /* SOI */       /* bSwap16() */
            {                   /* dqt */
                EXIF_MARKER_DQT,            /* DQT */   /* bSwap16() */
                0x8400,                 /* Lq */    /* bSwap16() */
                0x00,                   /* lumPqTq */
                {                   /* lumQ[64] = Q50 */
                    16,  11,  10,  16,  24,  40,  51,  61,
                    12,  12,  14,  19,  26,  58,  60,  55,
                    14,  13,  16,  24,  40,  57,  69,  56,
                    14,  17,  22,  29,  51,  87,  80,  62,
                    18,  22,  37,  56,  68, 109, 103,  77,
                    24,  35,  55,  64,  81, 104, 113,  92,
                    49,  64,  78,  87, 103, 121, 120, 101,
                    72,  92,  95,  98, 112, 100, 103,  99,
                },
                0x01,                   /* chrPqTq */
                {                   /* chrQ[64] = Q50 */
                    17,  18,  24,  47,  99,  99,  99,  99,
                    18,  21,  26,  66,  99,  99,  99,  99,
                    24,  26,  56,  99,  99,  99,  99,  99,
                    47,  66,  99,  99,  99,  99,  99,  99,
                    99,  99,  99,  99,  99,  99,  99,  99,
                    99,  99,  99,  99,  99,  99,  99,  99,
                    99,  99,  99,  99,  99,  99,  99,  99,
                    99,  99,  99,  99,  99,  99,  99,  99, 
                }     
            },
            {                   /* dht */
                EXIF_MARKER_DHT,            /* DHT */   /* bSwap16() */
                0xa201,                 /* Lh */    /* bSwap16() */
                0x00,                   /* lumDcTcTh */
                {                   /* lumDcL[16] */
                    0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 
                    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                },
                {                   /* lumDcV[12] */
                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0a, 0x0b, 
                },
                0x10,                   /* lumAcTcTh */
                {                   /* lumAcL[16] */
                    0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
                    0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,     
                },
                {                   /* lumAcV[162] */
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
                },
                0x01,                   /* chrDcTcTh */
                {                   /* chrDcL[16] */
                    0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                },
                {                       /* chrDcV[12] */
                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0a, 0x0b,
                },
                0x11,                   /* chrAcTcTh */
                {                   /* chrAcL[16] */
                    0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
                    0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
                },
                {                   /* chrAcV[162] */       
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
                },
            },  
            {                   /* sof0 */
                EXIF_MARKER_SOF0,           /* SOF0 */  /* bSwap16() */
                0x1100,                 /* Lf */    /* bSwap16() */
                0x08,                   /* P */ 
                0xe001,                 /* Y */     /* bSwap16() */
                0x8002,                 /* X */     /* bSwap16() */
                0x03,                   /* Nf - Y, Cb, Cr */
                0x01, 0x21, 0x00,           /* yC, yHV, yTq */
                0x02, 0x11, 0x01,           /* cbC, cbHV, cbTq */
                0x03, 0x11, 0x01,           /* crC, crHV, crTq */
            },
            {                   /* sos */
                EXIF_MARKER_SOS,            /* SOS */   /* bSwap16() */
                0x0c00,                 /* Ls */    /* bSwap16() */
                0x03,                       /* Ns - Y, Cb, Cr */
                0x01, 0x00,             /* yCs, yTdTa */
                0x02, 0x11,             /* cbCs, cbTdTa */
                0x03, 0x11,             /* crCs, crTdTa */
                0x00,                   /* Ss */
                0x3f,                   /* Se */
                0x00,                   /* AhAl */
            },
        },
    },
    0                           /* bitStream[] */
};  
u32 exifThumbnailImageHeaderSize = (sizeof(u16) + sizeof(EXIF_APP1));   /* SOI + app1 */
    
__align(4) EXIF_PRIMARY_IMAGE exifPrimaryImage =
{                           
    {                       /* primary */
        {                       /* com */
            EXIF_MARKER_COM,                /* COM */   /* bSwap16() */
            0x1200,                         /* Lc */    /* bSwap16() */
            {                               /* Cm[16] */
                //'M',  'a',  'r',  's',  '-',  'S ',  'e',  'm',
                77, 97, 114, 115, 45, 32, 101, 109,
                //'i', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                105, 0, 0, 0, 0, 0, 0, 0,           
            },
            //{0}
        },
#if JPEG_DRI_ENABLE
        {                       /* dri */
            EXIF_MARKER_DRI,            /* DRI */   /* bSwap16() */
            0x0400,                     /* Lr */    /* bSwap16() */
            0x0000,                     /* Ri */    /* bSwap16() */
        },
#endif
        {                       /* dqt */
            EXIF_MARKER_DQT,                /* DQT */   /* bSwap16() */
            0x8400,                     /* Lq */    /* bSwap16() */
            0x00,                       /* lumPqTq */
            {                       /* lumQ[64] = Q50 */
                16,  11,  10,  16,  24,  40,  51,  61,
                12,  12,  14,  19,  26,  58,  60,  55,
                14,  13,  16,  24,  40,  57,  69,  56,
                14,  17,  22,  29,  51,  87,  80,  62,
                18,  22,  37,  56,  68, 109, 103,  77,
                24,  35,  55,  64,  81, 104, 113,  92,
                49,  64,  78,  87, 103, 121, 120, 101,
                72,  92,  95,  98, 112, 100, 103,  99,
            },
            0x01,                       /* chrPqTq */
            {                       /* chrQ[64] = Q50 */
                17,  18,  24,  47,  99,  99,  99,  99,
                18,  21,  26,  66,  99,  99,  99,  99,
                24,  26,  56,  99,  99,  99,  99,  99,
                47,  66,  99,  99,  99,  99,  99,  99,
                99,  99,  99,  99,  99,  99,  99,  99,
                99,  99,  99,  99,  99,  99,  99,  99,
                99,  99,  99,  99,  99,  99,  99,  99,
                99,  99,  99,  99,  99,  99,  99,  99, 
            },    
        },
        {                       /* dht */
            EXIF_MARKER_DHT,                /* DHT */   /* bSwap16() */
            0xa201,                     /* Lh */    /* bSwap16() */
            0x00,                       /* lumDcTcTh */
            {                       /* lumDcL[16] */
                0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 
                0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            },
            {                       /* lumDcV[12] */
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0a, 0x0b, 
            },
            0x10,                       /* lumAcTcTh */
            {                       /* lumAcL[16] */
                0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
                0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,     
            },
            {                       /* lumAcV[162] */
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
            },
            0x01,                       /* chrDcTcTh */
            {                       /* chrDcL[16] */
                0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
            },
            {                       /* chrDcV[12] */
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0a, 0x0b,
            },
            0x11,                       /* chrAcTcTh */
            {                       /* chrAcL[16] */
                0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
                0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
            },
            {                       /* chrAcV[162] */       
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
            },
        },              
        {                       /* sof0 */
            EXIF_MARKER_SOF0,               /* SOF0 */  /* bSwap16() */
            0x1100,                     /* Lf */    /* bSwap16() */
            0x08,                       /* P */ 
            0xb004,                     /* Y - TBD */       /* bSwap16() */
            0x4006,                     /* X - TBD */       /* bSwap16() */
            0x03,                       /* Nf - Y, Cb, Cr */
            0x01, 0x21, 0x00,               /* yC, yHV, yTq */
            0x02, 0x11, 0x01,               /* cbC, cbHV, cbTq */
            0x03, 0x11, 0x01,               /* crC, crHV, crTq */
        },
        {                       /* sos */
            EXIF_MARKER_SOS,                /* SOS */   /* bSwap16() */
            0x0c00,                     /* Ls */    /* bSwap16() */
            0x03,                           /* Ns - Y, Cb, Cr */
            0x01, 0x00,                 /* yCs, yTdTa */
            0x02, 0x11,                 /* cbCs, cbTdTa */
            0x03, 0x11,                 /* crCs, crTdTa */
            0x00,                       /* Ss */
            0x3f,                       /* Se */
            0x00,                       /* AhAl */
        },
    },
    0                          /* bitStream[] */
};  
u32 exifPrimaryImageHeaderSize = sizeof(EXIF_PRIMARY);          /* primary */

#if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
__align(4) EXIF_MJPG exifMJPGImage =
{
    #if MJPG_APP0_ENABLE
    /***** app0 *****/
    {
        EXIF_MARKER_APP0,
        0x1100,    
        {0x4a, 0x46, 0x49, 0x46, 0x00},
        {0x01, 0x02},
        0x00,
        0x0000,
        0x0000,
        0x00,
        0x00,
        0x00
    },
    #endif
    /***** com *****/
    {                       
        EXIF_MARKER_COM,                /* COM */   /* bSwap16() */
        0x1200,                         /* Lc */    /* bSwap16() */
        {                               /* Cm[16] */
            'M',  'a',  'r',  's',  '-',  'S ',  'e',  'm',  
            'i', '\0', '\0', '\0', '\0', '\0', '\0', '\0',            
        },

    },	
    #if MJPG_DRI_ENABLE    
    /***** dri *****/
	{                      
	    EXIF_MARKER_DRI,            /* DRI */   /* bSwap16() */
        0x0400,                     /* Lr */    /* bSwap16() */
        0x0000,                     /* Ri */    /* bSwap16() */
    },
    #endif
    /***** dqt *****/
    {                       
        EXIF_MARKER_DQT,            /* DQT */   /* bSwap16() */
        0x8400,                     /* Lq */    /* bSwap16() */
        0x00,                       /* lumPqTq */
        {                           /* lumQ[64] = Q50 */
            16,  11,  10,  16,  24,  40,  51,  61,
            12,  12,  14,  19,  26,  58,  60,  55,
            14,  13,  16,  24,  40,  57,  69,  56,
            14,  17,  22,  29,  51,  87,  80,  62,
            18,  22,  37,  56,  68, 109, 103,  77,
            24,  35,  55,  64,  81, 104, 113,  92,
            49,  64,  78,  87, 103, 121, 120, 101,
            72,  92,  95,  98, 112, 100, 103,  99,
        },
        0x01,                       /* chrPqTq */
        {                       /* chrQ[64] = Q50 */
            17,  18,  24,  47,  99,  99,  99,  99,
            18,  21,  26,  66,  99,  99,  99,  99,
            24,  26,  56,  99,  99,  99,  99,  99,
            47,  66,  99,  99,  99,  99,  99,  99,
            99,  99,  99,  99,  99,  99,  99,  99,
            99,  99,  99,  99,  99,  99,  99,  99,
            99,  99,  99,  99,  99,  99,  99,  99,
            99,  99,  99,  99,  99,  99,  99,  99, 
        },    
    },
    /***** dht *****/
    {
        EXIF_MARKER_DHT,        /* DHT */   /* bSwap16() */
        0xa201,                 /* Lh */    /* bSwap16() */
        0x00,                   /* lumDcTcTh */
        {                       /* lumDcL[16] */
            0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 
            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        },
        {                       /* lumDcV[12] */
            0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x08, 0x09, 0x0a, 0x0b, 
        },
        0x10,                   /* lumAcTcTh */
        {                       /* lumAcL[16] */
            0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
            0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,     
        },
        {                       /* lumAcV[162] */
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
        },
        0x01,                   /* chrDcTcTh */
        {                       /* chrDcL[16] */
                0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
            },
            {                       /* chrDcV[12] */
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0a, 0x0b,
            },
            0x11,                   /* chrAcTcTh */
            {                       /* chrAcL[16] */
                0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
                0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
            },
            {                       /* chrAcV[162] */       
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
            },
    },
    /***** sof0 *****/
    {
        EXIF_MARKER_SOF0,           /* SOF0 */  /* bSwap16() */
        0x1100,                     /* Lf */    /* bSwap16() */
        0x08,                       /* P */ 
        0xb004,                     /* Y - TBD */       /* bSwap16() */
        0x4006,                     /* X - TBD */       /* bSwap16() */
        0x03,                       /* Nf - Y, Cb, Cr */
        0x01, 0x21, 0x00,           /* yC, yHV, yTq */
        0x02, 0x11, 0x01,           /* cbC, cbHV, cbTq */
        0x03, 0x11, 0x01,           /* crC, crHV, crTq */
    },
    /***** sos *****/
    {
        EXIF_MARKER_SOS,            /* SOS */   /* bSwap16() */
        0x0c00,                     /* Ls */    /* bSwap16() */
        0x03,                       /* Ns - Y, Cb, Cr */
        0x01, 0x00,                 /* yCs, yTdTa */
        0x02, 0x11,                 /* cbCs, cbTdTa */
        0x03, 0x11,                 /* crCs, crTdTa */
        0x00,                       /* Ss */
        0x3f,                       /* Se */
        0x00,                       /* AhAl */
    },	
};

u32 exifMJPGImageHeaderSize = sizeof(EXIF_MJPG);
#endif

/*CY 0601 E*/
    DEF_APP4PREFIX exifAPP4Prefix;

#if ADDAPP3TOJPEG 
    DEF_APP3PREFIX exifAPP3Prefix;
    __align(4) EXIF_PRIMARY_IMAGE exifAPP3VGAImage =
    {                           
        {                       /* primary */
            {                       /* com */
                EXIF_MARKER_COM,                /* COM */   /* bSwap16() */
                0x1200,                         /* Lc */    /* bSwap16() */
                {                               /* Cm[16] */
                    'H',  'i',  'm',  'a',  'x',  ' ',  'I',  'n',  
                        'c', '\0', '\0', '\0', '\0', '\0', '\0', '\0',           
                },
                //{0}
            },
    #if JPEG_DRI_ENABLE
            {                       /* dri */
                EXIF_MARKER_DRI,            /* DRI */   /* bSwap16() */
                0x0400,                     /* Lr */    /* bSwap16() */
                0x2800,                     /* Ri */    /* bSwap16() */
            },
    #endif
            {                       /* dqt */
                EXIF_MARKER_DQT,                /* DQT */   /* bSwap16() */
                0x8400,                     /* Lq */    /* bSwap16() */
                0x00,                       /* lumPqTq */
                {                       
                    // lumQ[64] = Q50 //
                    /*
                    16,  11,  10,  16,  24,  40,  51,  61,
                    12,  12,  14,  19,  26,  58,  60,  55,
                    14,  13,  16,  24,  40,  57,  69,  56,
                    14,  17,  22,  29,  51,  87,  80,  62,
                    18,  22,  37,  56,  68, 109, 103,  77,
                    24,  35,  55,  64,  81, 104, 113,  92,
                    49,  64,  78,  87, 103, 121, 120, 101,
                    72,  92,  95,  98, 112, 100, 103,  99,
                    */
                    // lumQ[64] = Q86 //
                    0x01,  0x01,  0x01,  0x02,  0x01,  0x01,  0x02,  0x02,
                    0x02,  0x02,  0x03,  0x02,  0x02,  0x03,  0x03,  0x06,
                    0x04,  0x03,  0x03,  0x03,  0x03,  0x07,  0x05,  0x08,
                    0x04,  0x06,  0x08,  0x08,  0x0a,  0x09,  0x08,  0x07,
                    0x0b,  0x08,  0x0a,  0x0e,  0x0d,  0x0b,  0x0a,  0x0a,
                    0x0c,  0x0a,  0x08,  0x08,  0x0b,  0x10,  0x0c,  0x0c,
                    0x0d,  0x0f,  0x0f,  0x0f,  0x0f,  0x09,  0x0b,  0x10,
                    0x11,  0x0f,  0x0e,  0x11,  0x0d,  0x0e,  0x0e,  0x0e,
                },
                0x01,                       /* chrPqTq */
                {   
                    // chrQ[64] = Q50 //
                    /*
                    17,  18,  24,  47,  99,  99,  99,  99,
                    18,  21,  26,  66,  99,  99,  99,  99,
                    24,  26,  56,  99,  99,  99,  99,  99,
                    47,  66,  99,  99,  99,  99,  99,  99,
                    99,  99,  99,  99,  99,  99,  99,  99,
                    99,  99,  99,  99,  99,  99,  99,  99,
                    99,  99,  99,  99,  99,  99,  99,  99,
                    99,  99,  99,  99,  99,  99,  99,  99, 
                    */
                    // chrQ[64] = Q86 //
                    0x04,  0x04,  0x04,  0x05,  0x04,  0x05,  0x09,  0x05,
                    0x05,  0x09,  0x0f,  0x0a,  0x08,  0x0a,  0x0f,  0x1a,
                    0x13,  0x09,  0x09,  0x13,  0x1a,  0x1a,  0x1a,  0x1a,
                    0x0d,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,
                    0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,
                    0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,
                    0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,
                    0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,  0x1a,
                },    
            },
            {                       /* dht */
                EXIF_MARKER_DHT,                /* DHT */   /* bSwap16() */
                0xa201,                     /* Lh */    /* bSwap16() */
                0x00,                       /* lumDcTcTh */
                {                       /* lumDcL[16] */
                    0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 
                    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                },
                {                       /* lumDcV[12] */
                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0a, 0x0b, 
                },
                0x10,                       /* lumAcTcTh */
                {                       /* lumAcL[16] */
                    0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03,
                    0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d,     
                },
                {                       /* lumAcV[162] */
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
                },
                0x01,                       /* chrDcTcTh */
                {                       /* chrDcL[16] */
                    0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                    0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
                },
                {                       /* chrDcV[12] */
                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0a, 0x0b,
                },
                0x11,                       /* chrAcTcTh */
                {                       /* chrAcL[16] */
                    0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04,
                    0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
                },
                {                       /* chrAcV[162] */       
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
                },
            },              
            {                       /* sof0 */
                EXIF_MARKER_SOF0,               /* SOF0 */  /* bSwap16() */
                0x1100,                     /* Lf */    /* bSwap16() */
                0x08,                       /* P */ 
                0xf000,                     /* Y - TBD */       /* bSwap16() */
                //0xa000,                       /* X - TBD */       /* bSwap16() */
                0x4001,                     /* X - TBD */       /* bSwap16() */
                0x03,                       /* Nf - Y, Cb, Cr */
                0x01, 0x21, 0x00,               /* yC, yHV, yTq */
                0x02, 0x11, 0x01,               /* cbC, cbHV, cbTq */
                0x03, 0x11, 0x01,               /* crC, crHV, crTq */
            },
            {                       /* sos */
                EXIF_MARKER_SOS,                /* SOS */   /* bSwap16() */
                0x0c00,                     /* Ls */    /* bSwap16() */
                0x03,                           /* Ns - Y, Cb, Cr */
                0x01, 0x00,                 /* yCs, yTdTa */
                0x02, 0x11,                 /* cbCs, cbTdTa */
                0x03, 0x11,                 /* crCs, crTdTa */
                0x00,                       /* Ss */
                0x3f,                       /* Se */
                0x00,                       /* AhAl */
            },
        },
        { 0 },                          /* bitStream[] */
    };  
    //u32 exifAPP3VGAHeaderSize = sizeof(EXIF_PRIMARY);         /* primary */
#endif

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 exifFileInit(void);
s32 exifFileParse(u8, u8*, u32, u32*, u16*, u16*);
s32 exifApp1Init(EXIF_APP1*);
s32 exifThumbnailInit(EXIF_THUMBNAIL*);
s32 exifPrimaryInit(EXIF_PRIMARY*);
s32 exifIntToStr(u32, s8*, s32);

s32 exifApp1Parse(u8*);
s32 exifIfdParse(EXIF_IFD*, u8*);
s32 exifIfdTag(u16);
s32 exifIfdType(u16, u32*);
s32 exifIfdsName(u8);
s32 exifComParse(u8*);
s32 exifDqtParse(u8*);
s32 exifDhtParse(u8*);
s32 exifSof0Parse(u8*, u16*, u16*, u8*);
s32 exifDriParse(u8*, u16*);
s32 exifSosParse(u8*);

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

    EXIF file initialization.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifFileInit(void)
{
    exifApp1Init(&exifThumbnailImage.app1);
    exifPrimaryInit(&exifPrimaryImage.primary);
    
#if ADDAPP3TOJPEG
    exifAPP3VGAImage.bitStream=exifAPP3VGABitstream;
#endif

    exifThumbnailImage.bitStream    = exifThumbnailBitstream;
    exifPrimaryImage.bitStream      = exifPrimaryBitstream;
    
    return 1;
}

/*

Routine Description:

    EXIF APP1 initialization.

Arguments:

    app1 - APP1.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifApp1Init(EXIF_APP1* app1)
{   
    app1->Lp = bSwap16((u16)sizeof(EXIF_APP1));

    app1->ifd1.jpegInterchangeFormatLength.valueOffset = 0;
    
    exifThumbnailInit(&app1->thumbnail); 
    
    return 1;   
}

/*

Routine Description:

    EXIF thumbnail initialization.

Arguments:

    thumbnail - Thumbnail.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifThumbnailInit(EXIF_THUMBNAIL* thumbnail)
{
    //jpegSetQuantizationQuality(JPEG_IMAGE_THUMBNAIL, 60);
    jpegSetQuantizationQuality(JPEG_IMAGE_THUMBNAIL, 80);   /*CY 0907*/
    
    thumbnail->sof0.X = bSwap16((u16)160);
    thumbnail->sof0.Y = bSwap16((u16)120); 
     
    return 1;
}

/*

Routine Description:

    EXIF primary initialization.

Arguments:

    primary - Primary.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifPrimaryInit(EXIF_PRIMARY* primary)
{
    return 1;
}

/*

Routine Description:

    Set quantization table.

Arguments:

    type - Image type.
    lumQ - Luminance quantization table.
    chrQ - Chrominance quantization table.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetQuantizationTable(u8 type, const u8* lumQ, const u8* chrQ)
{   
/*Peter 0829 S*/
    u32 i;
    u8  *pLumQ, *pChrQ;
    
    if (type == JPEG_IMAGE_THUMBNAIL)
    {
        pLumQ   = (u8*)exifThumbnailImage.app1.thumbnail.dqt.lumQ;
        pChrQ   = (u8*)exifThumbnailImage.app1.thumbnail.dqt.chrQ;
    }
    else // if (type == JPEG_IMAGE_PRIMARY)
    {       
        pLumQ   = (u8*)exifPrimaryImage.primary.dqt.lumQ;
        pChrQ   = (u8*)exifPrimaryImage.primary.dqt.chrQ;
    }
    for(i = 0; i < 64; i++) {
        pLumQ[i]    = lumQ[jpegZigzag[i]];
        pChrQ[i]    = chrQ[jpegZigzag[i]];
    }
/*Peter 0829 E*/
            
    return 1;   
}

/*

Routine Description:

    Set image resolution.

Arguments:

    width - Image width.
    height - Image height.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetImageResolution(u16 width, u16 height)
{
    exifThumbnailImage.app1.ifd0e.pixelXDimension.valueOffset = (u32)width;
    exifThumbnailImage.app1.ifd0e.pixelYDimension.valueOffset = (u32)height;
    exifPrimaryImage.primary.sof0.X = bSwap16(width);
    exifPrimaryImage.primary.sof0.Y = bSwap16(height); 
    
#if JPEG_DRI_ENABLE
    exifPrimaryImage.primary.dri.Ri = bSwap16(width / 16); 
#endif

    return 1;
}

/*

Routine Description:

    Set thumbnail size.

Arguments:

    size - Thumbnail size.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetThumbnailSize(u32 size)
{
    /* cytsai: for testing only */ /* cytsai: 0315 */ /* cytsai: 0418 */
    //size = 0x10c8;
        
    exifThumbnailImage.app1.Lp = bSwap16((u16)(sizeof(EXIF_APP1) + size - 2));
    exifThumbnailImage.app1.ifd1.jpegInterchangeFormatLength.valueOffset = sizeof(EXIF_THUMBNAIL) + size;   
    
    return 1;
}

/*

Routine Description:

    Set compressed bits per pixel.

Arguments:

    compressedBitsPerPixel - Compressed bits per pixel (compressedBitsPerPixel = Compressed bits per pixel * 10)
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetCompressedBitsPerPixel(u32 compressedBitsPerPixel)
{
    exifThumbnailImage.app1.ifd0eValue.compressedBitsPerPixelValue[0] = compressedBitsPerPixel;
    exifThumbnailImage.app1.ifd0eValue.compressedBitsPerPixelValue[1] = 10;
    
    return 1;
}

s32 exifSetCopyRightVersion(s8 *copyright)
{
    memcpy((s8 *)exifThumbnailImage.app1.ifd0Value.copyrightValue+7,copyright,14);
    
    return 1;
}
/*

Routine Description:

    Set date time.

Arguments:

    dateTime - Date time.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetDateTime(RTC_DATE_TIME* pDateTime)
{
    s8 dateTimeStr[EXIF_IFD0_DATE_TIME_VALUE_COUNT];
    s8* pStr = dateTimeStr;
    
    exifIntToStr(pDateTime->year + 2000, pStr, 4);
    pStr += 4;
    *pStr++ = ':';
    exifIntToStr(pDateTime->month, pStr, 2);
    pStr += 2;
    *pStr++ = ':';
    exifIntToStr(pDateTime->day, pStr, 2);
    pStr += 2;
    *pStr++ = ' ';
    exifIntToStr(pDateTime->hour, pStr, 2);
    pStr += 2;
    *pStr++ = ':';
    exifIntToStr(pDateTime->min, pStr, 2);
    pStr += 2;
    *pStr++ = ':';
    exifIntToStr(pDateTime->sec, pStr, 2);
    pStr += 2;
    *pStr++ = '\0';
            
    strcpy((char*)exifThumbnailImage.app1.ifd0Value.dateTimeValue, (const char*)dateTimeStr);
    strcpy((char*)exifThumbnailImage.app1.ifd0eValue.dateTimeOriginalValue, (const char*)dateTimeStr);
    strcpy((char*)exifThumbnailImage.app1.ifd0eValue.dateTimeDigitizedValue, (const char*)dateTimeStr);
    
    return 1;
}

/*

Routine Description:

    Convert an integer to a string.

Arguments:

    val - The integer value to be converted.
    tkn - The token.
    cnt - The character count of the token.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifIntToStr(u32 val, s8* str, s32 cnt)
{ 
    s32 i;
    s8 dig[8];
    
    for (i = 0; i < cnt; i++)
    {
        dig[i] = exifDecChar[val % 10];
        val /= 10;
    }   
    
    for (i = (cnt - 1); i >= 0; i--)
        *str++ = dig[i];

    *str = '\0';
    
    return 1;
}

/*

Routine Description:

    Set flash.

Arguments:

    status - Flash status.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetFlash(u16 status)
{
    exifThumbnailImage.app1.ifd0e.flash.valueOffset = (u32)status;
    
    return 1;
}

/*

Routine Description:

    Set light source.

Arguments:

    lightSource - Light source.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetLightSource(u16 lightSource)
{
    exifThumbnailImage.app1.ifd0e.lightSource.valueOffset = (u32)lightSource;   
                
    return 1;
}   
    
/*

Routine Description:

    Set exposure bias value.

Arguments:

    expBiasValue - Exposure bias value. (expBiasValue = Exposure Bias Value * 10)
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetExposureBiasValue(s32 expBiasValue)
{
    exifThumbnailImage.app1.ifd0eValue.exposureBiasValueValue[0] = expBiasValue;
    exifThumbnailImage.app1.ifd0eValue.exposureBiasValueValue[1] = 10;
    
    return 1;
}

/*

Routine Description:

    Set aperture value.

Arguments:

    pFNumber - F number.
    pApertureValue - Aperture value.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetApertureValue(RATIONAL* pFNumber, RATIONAL* pApertureValue)
{
    exifThumbnailImage.app1.ifd0eValue.fNumberValue[0] = pFNumber->numerator;
    exifThumbnailImage.app1.ifd0eValue.fNumberValue[1] = pFNumber->denominator;
    
    exifThumbnailImage.app1.ifd0eValue.apertureValueValue[0] = pApertureValue->numerator;
    exifThumbnailImage.app1.ifd0eValue.apertureValueValue[1] = pApertureValue->denominator;
    
    exifThumbnailImage.app1.ifd0eValue.maxApertureValueValue[0] = pApertureValue->numerator;
    exifThumbnailImage.app1.ifd0eValue.maxApertureValueValue[1] = pApertureValue->denominator;
    
    return 1;
}   
    
/*

Routine Description:

    Set shutter speed value.

Arguments:

    pExposureTime - Exposure time.
    pShutterSpeedValue - Shutter speed value.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetShutterSpeedValue(RATIONAL* pExposureTime, RATIONAL* pShutterSpeedValue)
{
    exifThumbnailImage.app1.ifd0eValue.exposureTimeValue[0] = pExposureTime->numerator;
    exifThumbnailImage.app1.ifd0eValue.exposureTimeValue[1] = pExposureTime->denominator;
    
    exifThumbnailImage.app1.ifd0eValue.shutterSpeedValueValue[0] = pShutterSpeedValue->numerator;
    exifThumbnailImage.app1.ifd0eValue.shutterSpeedValueValue[1] = pShutterSpeedValue->denominator;
    
    return 1;
}

/*

Routine Description:

    Set brightness value.

Arguments:

    pBrightnessValue - Brightness value. 
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetBrightnessValue(RATIONAL* pBrightnessValue)
{
    exifThumbnailImage.app1.ifd0eValue.brightnessValueValue[0] = pBrightnessValue->numerator;
    exifThumbnailImage.app1.ifd0eValue.brightnessValueValue[1] = pBrightnessValue->denominator;
    
    return 1;
}

/*

Routine Description:

    Set subject distance.

Arguments:

    subjectDistance - Subject distance (cm).
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetSubjectDistance(u32 subjectDistance)
{
    exifThumbnailImage.app1.ifd0eValue.subjectDistanceValue[0] = subjectDistance;
    exifThumbnailImage.app1.ifd0eValue.subjectDistanceValue[1] = 100;
            
    return 1;
}

/*

Routine Description:

    Set focal length.

Arguments:

    focalLength - Focal length (mm).
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSetFocalLength(u32 focalLength)
{
    exifThumbnailImage.app1.ifd0eValue.focalLengthValue[0] = focalLength;
    exifThumbnailImage.app1.ifd0eValue.focalLengthValue[1] = 1;
            
    return 1;
}

/*

Routine Description:

    EXIF file parsing.

Arguments:

    type - Image type.
    pBuf - EXIF file buffer.
    size - Size.
    pSizeUsed - Size consumed.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifFileParse(u8 type, u8* pBuf, u32 size, u32* pSizeUsed, u16* pWidth, u16* pHeight)
{       
    u8 lengthField;
    u16 marker, length;
    u32 bufStart = (u32)pBuf;
    u32 bufEnd = (u32)(pBuf+size);
    u8 eoi = 0;
    /*BJ 0523 S*/
    u8 FindBS = 0; /*BJ*/
    u16 applength;
    u8  format;
    u16 ri  = 0;
    u32 APP3_ID;
    u8 var8;
    u8 *pBuf_temp;
    
    *pSizeUsed  = 0;
        
    exifAPP3VGAImg_flag=0;
    while ((((u32)pBuf) < bufEnd) && (eoi == 0) && (FindBS == 0)) /*BJ 0523 S*/
    {
        /* marker */
        marker = *((__packed u16*)pBuf);
        pBuf += 2;
        
        /* default set to with length field */
        lengthField = 1;
        
        switch (marker)
        {
            /* Start Of Frame markers, non-differential, Huffman coding */
            case EXIF_MARKER_SOF0:
                //DEBUG_DCF("Trace: SOF0\n");
                if (exifSof0Parse(pBuf, pWidth, pHeight, &format) == 0) {
                    *pSizeUsed  = ((u32)pBuf) - bufStart;
                    DEBUG_DCF("Error: exifSof0Parse fail!!!\n");
                    return 0;
                }
                break;
                
            case EXIF_MARKER_SOF1:
                //DEBUG_DCF("Trace: SOF1\n");
                break;
                
            case EXIF_MARKER_SOF2:
                //DEBUG_DCF("Trace: SOF2\n");
                break;
                
            case EXIF_MARKER_SOF3:
                //DEBUG_DCF("Trace: SOF3\n");
                break;

            /* Start Of Frame markers, differential, Huffman coding */
            case EXIF_MARKER_SOF5:
                //DEBUG_DCF("Trace: SOF5\n");
                break;
                
            case EXIF_MARKER_SOF6:
                //DEBUG_DCF("Trace: SOF6\n");
                break;
                
            case EXIF_MARKER_SOF7:
                //DEBUG_DCF("Trace: SOF7\n");
                break;
                
            /* Start Of Frame markers, non-differential, arithmetic coding */
            case EXIF_MARKER_JPG:
                //DEBUG_DCF("Trace: JPG\n");
                break;
                
            case EXIF_MARKER_SOF9:
                //DEBUG_DCF("Trace: SOF9\n");
                break;
                
            case EXIF_MARKER_SOF10:
                //DEBUG_DCF("Trace: SOF10\n");
                break;
                
            case EXIF_MARKER_SOF11:
                //DEBUG_DCF("Trace: SOF11\n");
                break;
                
            /* Start Of Frame markers, differential, arithmetic coding */
            case EXIF_MARKER_SOF13:
                //DEBUG_DCF("Trace: SOF13\n");
                break;
                
            case EXIF_MARKER_SOF14:
                //DEBUG_DCF("Trace: SOF14\n");
                break;
                
            case EXIF_MARKER_SOF15:
                //DEBUG_DCF("Trace: SOF15\n");
                break;
                
            /* Huffman table specification */
            case EXIF_MARKER_DHT:
                //DEBUG_DCF("Trace: DHT\n");
                if (exifDhtParse(pBuf) == 0) {
                    *pSizeUsed  = ((u32)pBuf) - bufStart;
                    DEBUG_DCF("Error: exifDhtParse fail!!!\n");
                    return 0;
                }
                break;

            /* Arithmetic coding conditioning specification */
            case EXIF_MARKER_DAC:
                //DEBUG_DCF("Trace: DAC\n");
                break;
                
            /* Restart interval termination */
            case EXIF_MARKER_RST0:
                //DEBUG_DCF("Trace: RST0\n");
                lengthField = 0;
                break;
                
            case EXIF_MARKER_RST1:
                //DEBUG_DCF("Trace: RST1\n");
                lengthField = 0;
                break;
                
            case EXIF_MARKER_RST2:
                //DEBUG_DCF("Trace: RST2\n");
                lengthField = 0;
                break;
                
            case EXIF_MARKER_RST3:
                //DEBUG_DCF("Trace: RST3\n");
                lengthField = 0;
                break;
                
            case EXIF_MARKER_RST4:
                //DEBUG_DCF("Trace: RST4\n");
                lengthField = 0;
                break;
                
            case EXIF_MARKER_RST5:
                //DEBUG_DCF("Trace: RST5\n");
                lengthField = 0;
                break;
                
            case EXIF_MARKER_RST6:
                //DEBUG_DCF("Trace: RST6\n");
                lengthField = 0;
                break;
                
            case EXIF_MARKER_RST7:
                //DEBUG_DCF("Trace: RST7\n");
                lengthField = 0;
                break;
                
            /* Other markers */
            case EXIF_MARKER_SOI:
                //DEBUG_DCF("Trace: SOI\n");
                lengthField = 0;
                break;
                
            case EXIF_MARKER_EOI:
                //DEBUG_DCF("Trace: EOI\n");
                lengthField = 0;
                eoi = 1;
                break;
                
            case EXIF_MARKER_SOS:
                //DEBUG_DCF("Trace: SOS\n");
                jpegSetRestartInterval(ri);
                jpegSetDataFormat(format);
                jpegSetImageResolution(*pWidth, *pHeight);
                if (exifSosParse(pBuf) == 0) {
                    *pSizeUsed  = ((u32)pBuf) - bufStart;
                    DEBUG_DCF("Error: exifSosParse fail!!!\n");
                    return 0;
                }
                FindBS = 1;/*BJ 0523 S*/
                break;
                
            case EXIF_MARKER_DQT:
                //DEBUG_DCF("Trace: DQT\n");
                if (exifDqtParse(pBuf) == 0) {
                    *pSizeUsed  = ((u32)pBuf) - bufStart;
                    DEBUG_DCF("Error: exifDqtParse fail!!!\n");
                    return 0;
                }
                break;
                
            case EXIF_MARKER_DNL:
                //DEBUG_DCF("Trace: DNL\n");
                break;
                
            case EXIF_MARKER_DRI:
                //DEBUG_DCF("Trace: DRI\n");
                if (exifDriParse(pBuf, &ri) == 0) {
                    *pSizeUsed  = ((u32)pBuf) - bufStart;
                    DEBUG_DCF("Error: exifDriParse fail!!!\n");
                    return 0;
                }
                break;
                
            case EXIF_MARKER_DHP:
                //DEBUG_DCF("Trace: DHP\n");
                break;
                
            case EXIF_MARKER_EXP:
                //DEBUG_DCF("Trace: EXP\n");
                break;
                
            case EXIF_MARKER_APP0:
                //DEBUG_DCF("Trace: APP0\n");
                break;
                
            case EXIF_MARKER_APP1:
                //DEBUG_DCF("Trace: APP1\n");               
            #if(CHIP_OPTION != CHIP_A1016A) //Lucian: A1016 內部無D2D SC engine. 故 無法 看Thumbnail.
                if(type == JPEG_IMAGE_THUMBNAIL)
                   exifApp1Parse(pBuf);
            #endif                 
                applength = (*(pBuf++)<<8);
                applength = applength|*(pBuf++);
                pBuf += applength-2;
                        
                lengthField = 0;
                
                break;
                            
            case EXIF_MARKER_APP3:              
                applength = (*(pBuf++)<<8);
                applength = applength|*(pBuf++);
                
                APP3_ID=pBuf[0] | (pBuf[1]<<8) | (pBuf[2]<<16) | (pBuf[3]<<24);
                
                if( (APP3_ID == 0x12345678) && (type == JPEG_IMAGE_THUMBNAIL) )
                {
                  memcpy(exifDecBuf,pBuf+4,applength-2-4);
                  exifAPP3VGAImg_flag=1;
                }
                pBuf += applength-2;
                        
                lengthField = 0;
                break;
                    
            case EXIF_MARKER_APP2:
//                DEBUG_DCF("Trace: EXIF_MARKER_APP2\n");
//                if (exifApp2Parse(pBuf))
                pBuf_temp = pBuf;
                applength = (*(pBuf++)<<8);
                applength = applength|*(pBuf++);
                var8 = 0;
                if (*(pBuf++) == 'Y')
                    if (*(pBuf++) == 'U')
                        if (*(pBuf++) == 'V')
                            if (*(pBuf++) == '4')
                                if (*(pBuf++) == '2')    
                                    if (*(pBuf++) == '0')
                                        var8 = 1;
//                *(pBuf++);*(pBuf++);*(pBuf++);*(pBuf++);*(pBuf++);*(pBuf++);
                if (var8)
                {
                    *pSizeUsed  = ((u32)pBuf) - bufStart;
                    return 2;
                }
                else
                {
                    pBuf = pBuf_temp;
                    applength = (*(pBuf++)<<8);
                    applength = applength|*(pBuf++);
                    pBuf += applength-2;
                    lengthField = 0;
                }

                break;
            case EXIF_MARKER_APP4:
            case EXIF_MARKER_APP5:
            case EXIF_MARKER_APP6:
            case EXIF_MARKER_APP7:
            case EXIF_MARKER_APP8:
            case EXIF_MARKER_APP9:      
            case EXIF_MARKER_APP10:
            case EXIF_MARKER_APP11:
            case EXIF_MARKER_APP12:
            case EXIF_MARKER_APP13:
            case EXIF_MARKER_APP14:
            case EXIF_MARKER_APP15:
                applength = (*(pBuf++)<<8);
                applength = applength|*(pBuf++);
                pBuf += applength-2;
                        
                lengthField = 0;
                
                break;
                
            case EXIF_MARKER_JPG0:
                //DEBUG_DCF("Trace: JPG0\n");
                break;
                
            case EXIF_MARKER_JPG1:
                //DEBUG_DCF("Trace: JPG1\n");
                break;
                
            case EXIF_MARKER_JPG2:
                //DEBUG_DCF("Trace: JPG2\n");
                break;
                
            case EXIF_MARKER_JPG3:
                //DEBUG_DCF("Trace: JPG3\n");
                break;
                
            case EXIF_MARKER_JPG4:
                //DEBUG_DCF("Trace: JPG4\n");
                break;
                
            case EXIF_MARKER_JPG5:
                //DEBUG_DCF("Trace: JPG5\n");
                break;
                
            case EXIF_MARKER_JPG6:
                //DEBUG_DCF("Trace: JPG6\n");
                break;
                
            case EXIF_MARKER_JPG7:
                //DEBUG_DCF("Trace: JPG7\n");
                break;
                
            case EXIF_MARKER_JPG8:
                //DEBUG_DCF("Trace: JPG8\n");
                break;
                
            case EXIF_MARKER_JPG9:
                //DEBUG_DCF("Trace: JPG9\n");
                break;
                
            case EXIF_MARKER_JPG10:
                //DEBUG_DCF("Trace: JPG10\n");
                break;
                
            case EXIF_MARKER_JPG11:
                //DEBUG_DCF("Trace: JPG11\n");
                break;
                
            case EXIF_MARKER_JPG12:
                //DEBUG_DCF("Trace: JPG12\n");
                break;
                
            case EXIF_MARKER_JPG13:
                //DEBUG_DCF("Trace: JPG13\n");
                break;
                
            case EXIF_MARKER_COM:
                //DEBUG_DCF("Trace: COM\n");
                if (exifComParse(pBuf) == 0) {
                    *pSizeUsed  = ((u32)pBuf) - bufStart;
                    return 0;
                }
                break;
                
            /* Reserved markers */
            case EXIF_MARKER_TEM:
                //DEBUG_DCF("Trace: TEM\n");
                break;
                
            case EXIF_MARKER_RES:
                //DEBUG_DCF("Trace: RES\n");
                break;
                 
            default:
                lengthField = 0;
                pBuf--;
                continue;
        }
        
        if (lengthField)
        {
            length =  (*pBuf++) << 8;
            length |= (*pBuf++);
            pBuf += (length - 2);
        }       
    }            
    
    *pSizeUsed = ((u32)pBuf) - bufStart;
    
    return 1;
}
    
/*

Routine Description:

    EXIF APP1 parsing.

Arguments:

    pBuf - APP1 buffer.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifApp1Parse(u8* pBuf)
{   
    u8 var8;
    u16 var16;
    u32 var32;
    u16 i;
    u16 ifdNum = 0;
    u8* pDesc;
    u32 nextIfdOffset = 0;
    //u32 nextIfdsOffset = 0;
    u32 sizeUsed;
    u16 sizeX,sizeY;
    
    var32 = var32;  /* avoid warning */
    
    /*---- length ----*/
    var16 =  (*pBuf++) << 8;
    var16 |= (*pBuf++);
    //DEBUG_DCF("length = 0x%04x\n", var16);
    
    /* identifier[5] and pad */
    var8 = 0;
    if (*pBuf++ == 'E')
        if (*pBuf++ == 'x')
            if (*pBuf++ == 'i')
                if (*pBuf++ == 'f')
                    if (*pBuf++ == '\0')    
                        if (*pBuf++ == '\0')
                            var8 = 1;
    if (var8 ==0)
    {
        DEBUG_DCF("Error: identifier+pad is not found.\n");
        return 0;
    }               
    //DEBUG_DCF("identifier+pad = \"Exif\\0\\0\"\n");       
    
    /* save the start address of the description */
    pDesc = pBuf;
            
    /*---- tiffHeader ----*/
    //DEBUG_DCF("tiffHeader\n");
    
    /* byteOrder */
    var8 = 0;
    if (*pBuf++ == 'I')
        if (*pBuf++ == 'I')
            var8 = 1;
    if (var8 ==0)
    {
        DEBUG_DCF("Error: byteOrder is not found\n");
        return 0;
    }               
    //DEBUG_DCF("byteOrder = \"II\"\n");    
    
    /* versionNumber */
    var16 = *((__packed u16*)pBuf);
    pBuf += 2;
    //DEBUG_DCF("versionNumber = 0x%04x\n", var16);                 
    
    /* offsetToIfd0 */
    var32 = *((__packed u32*)pBuf); 
    pBuf += 4;
    //DEBUG_DCF("offsetToIfd0 = 0x%08x\n", var32);
    
    do 
    {
        if (ifdNum == 0)
        {
            /* initialize EXIF specific IFDs pointer */
            for (i = 0; i < EXIF_IFDS_MAX_IFD; i++)
                exifIfdsPointer[i] = 0;
        }
                
        if (ifdNum == 1)
        {
            /* initialize thumbnail */
            exifThumbnailPointer = 0;
            exifThumbnailLength = 0;
        }
            
        /*---- IFDn ----*/
        //DEBUG_DCF("ifd%d\n", ifdNum);
        
        /* numInterop */
        var16 = *((__packed u16*)pBuf);
        pBuf += 2;
        //DEBUG_DCF("numInterop = 0x%04x\n", var16);    
    
        /* IFD */
        for (i = 0; i < var16; i++)
        {
            exifIfdParse((__packed EXIF_IFD*)pBuf, pDesc);
            pBuf += sizeof(EXIF_IFD);
        }
        
        nextIfdOffset = *((__packed u32*)pBuf);
        
        if (ifdNum == 0)
        {
            for (i = 0; i < EXIF_IFDS_MAX_IFD; i++)
            {
                /* EXIF specific IFDs */
                if (exifIfdsPointer[i])
                    pBuf = pDesc + exifIfdsPointer[i];
                
                if (exifIfdsName(i) == 0)
                    continue;
        
                /* numInterop */
                var16 = *((__packed u16*)pBuf);
                pBuf += 2;
                //DEBUG_DCF("numInterop = 0x%04x\n", var16);    
    
                /* IFD */
                for (i = 0; i < var16; i++)
                {
                    exifIfdParse((__packed EXIF_IFD*)pBuf, pDesc);
                    pBuf += sizeof(EXIF_IFD);
                }
        
                //nextIfdsOffset = *((__packed u32*)pBuf);  
            }   
        }
        
        if (ifdNum == 1)
        {
            /* thumbnail */
            if (exifThumbnailPointer && exifThumbnailLength)
            {
                /* calculate start adress of thumbnail stream */
                pBuf = pDesc + exifThumbnailPointer;
                
                /* decompress thumbnail */
                if(exifFileParse(JPEG_IMAGE_PRIMARY, pBuf, exifThumbnailLength, &sizeUsed, &sizeX, &sizeY) == 0) {
                    DEBUG_DCF("Error: thumbnail exifFileParse fail!!!\n");
                    return 0;
                }
                
                if(jpegDecompression(pBuf + sizeUsed, PKBuf) == 0)
                    DEBUG_DCF("Thumbnail JPEG decode error!!!\n");
 
                if((playbackflag == 2)&&(uiMenuEnable == 0x41))
                {              
                    if(sysTVOutOnFlag)
                    {
                      isuPlayback(PKBuf,Jpeg_displaybuf[1], sizeX , sizeY);
                      iduPlaybackFrame(Jpeg_displaybuf[1]);
                    }
                    else
                    {             
                      isuPlayback(PKBuf,Jpeg_displaybuf[displaybuf_idx], sizeX , sizeY);
                      iduPlaybackFrame(Jpeg_displaybuf[displaybuf_idx]);                   
                    }
                }
                else
                    isuPlayback(PKBuf,Jpeg_displaybuf[2], sizeX , sizeY);
            }
        }
        
        pBuf = pDesc + nextIfdOffset;
        ifdNum++;
    } while (nextIfdOffset != 0);
    
    return 1;
}   

s32 exifApp2Parse(u8* pBuf)
{   
    u8 var8;
    u16 var16;
    u32 var32;
    //u32 nextIfdsOffset = 0;
    
    var32 = var32;  /* avoid warning */
    
    /*---- length ----*/
    var16 =  (*pBuf++) << 8;
    var16 |= (*pBuf++);
    //DEBUG_DCF("length = 0x%04x\n", var16);
    
    /* identifier[6] and pad */
    var8 = 0;
    if (*pBuf++ == 'Y')
        if (*pBuf++ == 'U')
            if (*pBuf++ == 'V')
                if (*pBuf++ == '4')
                    if (*pBuf++ == '2')    
                        if (*pBuf++ == '0')    
                            if (*pBuf++ == '\0')
                                    var8 = 1;
    if (var8 ==0)
    {
        DEBUG_DCF("Error: identifier+pad is not found.\n");
        return 0;
    }               
    
    return 1;
}   
/*

Routine Description:

    IFD parsing.

Arguments:

    pIfd - IFD.
    pDesc - The start address of the description related to offset. 

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifIfdParse(EXIF_IFD* pIfd, u8* pDesc)
{
    u32 unit, count;
    u32 i;
    u8* pVal;
    
    if (exifIfdTag(pIfd->tag) == 0)
        return 0;
    
    if (exifIfdType(pIfd->type, &unit) == 0)
        return 0;
    
    count = pIfd->count;
    //DEBUG_DCF("0x%08x\n", count);
     
    if ((unit * count) <= 4)
    {
        /* EXIF IFD specific */
        if (pIfd->tag == EXIF_TAG_EXIF_IFD_POINTER)
            exifIfdsPointer[EXIF_IFDS_EXIF_IFD] = pIfd->valueOffset;
        if (pIfd->tag == EXIF_TAG_GPS_INFO_IFD_POINTER)
            exifIfdsPointer[EXIF_IFDS_GPS_INFO_IFD] = pIfd->valueOffset;
        if (pIfd->tag == EXIF_TAG_INTEROP_IFD_POINTER)
            exifIfdsPointer[EXIF_IFDS_INTEROP_IFD] = pIfd->valueOffset;
            
        /* thumbnail specific */    
        if (pIfd->tag == EXIF_TAG_JPEG_INTERCHANGE_FORMAT)
            exifThumbnailPointer = pIfd->valueOffset;
        if (pIfd->tag == EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH)
            exifThumbnailLength = pIfd->valueOffset;    
                
        pVal = (u8*)&(pIfd->valueOffset);   
    }
    else
    {
        //DEBUG_DCF("0x%08x -> ", pIfd->valueOffset);   
        pVal = pDesc + pIfd->valueOffset;
    }   
        
    switch (unit)
    {
        case 1:
            if (pIfd->type == EXIF_TYPE_ASCII)
                for (i = 0; i < count; i++, pVal++)
                {
                    if (*pVal == '\0')
                    {
                        break;
                    }   
                    //DEBUG_DCF("%c", *pVal);
                }
            else
                for (i = 0; i < count; i++, pVal++) 
                {
                    //DEBUG_DCF("0x%02x ", *pVal);
                }   
            break;
            
        case 2:
            for (i = 0; i < count; i++, pVal+=2)
            {
                //DEBUG_DCF("0x%04x ", *((__packed u16*)pVal));
            }   
            break;
                
        case 4:
            for (i = 0; i < count; i++, pVal+=4)
            {
                //DEBUG_DCF("0x%08x ", *((__packed u32*)pVal));
            }
            break;
                
        case 8:
            for (i = 0; i < count; i++)
            {
                //DEBUG_DCF("0x%08x ", *((__packed u32*)pVal));
                pVal += 4;  
                //DEBUG_DCF("0x%08x ", *((__packed u32*)pVal));
                pVal += 4;  
            }   
            break;
                
        default:
            break;                                  
    }           
    //DEBUG_DCF("\n");  
        
    return 1;   
}

/*

Routine Description:

    IFD tag.

Arguments:

    tag - IFD tag.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifIfdTag(u16 tag)
{
    switch (tag)
    {
        /*---- EXIF specific IFD ----*/
        
        case EXIF_TAG_EXIF_IFD_POINTER:
            //DEBUG_DCF("exifIfdPointer\n");
            break;
        
        case EXIF_TAG_GPS_INFO_IFD_POINTER:
            //DEBUG_DCF("gpsInfoIfdPointer\n");
            break;
            
        case EXIF_TAG_INTEROP_IFD_POINTER:
            //DEBUG_DCF("interopIfdPointer\n");
            break;
        
        /*---- TIFF Rev. 6.0 Attribute Information Used in EXIF ----*/

        /* Tags relating to image data structure */
        case EXIF_TAG_IMAGE_WIDTH:
            //DEBUG_DCF("imageWidth\n");
            break;
            
        case EXIF_TAG_IMAGE_HEIGHT:
            //DEBUG_DCF("imageHeight\n");
            break;
            
        case EXIF_TAG_BITS_PER_SAMPLE:
            //DEBUG_DCF("bitsPerSample\n");
            break;
            
        case EXIF_TAG_COMPRESSION:
            //DEBUG_DCF("compression\n");
            break;
            
        case EXIF_TAG_PHOTOMETRIC_INTERPRETATION:
            //DEBUG_DCF("photometricInterpretation\n");
            break;
            
        case EXIF_TAG_ORIENTATION:
            //DEBUG_DCF("orientation\n");
            break;
            
        case EXIF_TAG_SAMPLES_PER_PIXEL:
            //DEBUG_DCF("samplesPerPixel\n");
            break;
            
        case EXIF_TAG_PLANAR_CONFIGURATION:
            //DEBUG_DCF("planarConfiguration\n");
            break;
            
        case EXIF_TAG_YCbCr_SUBSAMPLING:
            //DEBUG_DCF("yCbCrSubsampling\n");
            break;
            
        case EXIF_TAG_YCbCr_POSITIONING:
            //DEBUG_DCF("yCbCrPositioning\n");
            break;
            
        case EXIF_TAG_X_RESOLUTION:
            //DEBUG_DCF("xResolution\n");
            break;
            
        case EXIF_TAG_Y_RESOLUTION:
            //DEBUG_DCF("yResolution\n");
            break;
            
        case EXIF_TAG_RESOLUTION_UNIT:
            //DEBUG_DCF("resolutionUnit\n");
            break;
            
        /* Tags relating to recording offset */
        case EXIF_TAG_STRIP_OFFSETS:
            //DEBUG_DCF("stripOffsets\n");
            break;
            
        case EXIF_TAG_ROWS_PER_STRIP:
            //DEBUG_DCF("rowsPerStrip\n");
            break;
            
        case EXIF_TAG_STRIP_BYTE_COUNTS:
            //DEBUG_DCF("stripByteCounts\n");
            break;
            
        case EXIF_TAG_JPEG_INTERCHANGE_FORMAT:
            //DEBUG_DCF("jpegInterchangeFormat\n");
            break;
            
        case EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH:
            //DEBUG_DCF("jpegInterchangeFormatLength\n");
            break;

        /* Tags relating to image data characteristics */
        case EXIF_TAG_TRANSFER_FUNCTION:
            //DEBUG_DCF("transferFunction\n");
            break;
            
        case EXIF_TAG_WHITE_POINT:
            //DEBUG_DCF("whitePoint\n");
            break;
            
        case EXIF_TAG_PRIMARY_CHROMATICITIES:
            //DEBUG_DCF("primaryChromaticities\n");
            break;
            
        case EXIF_TAG_YCbCr_COEFFICIENTS:
            //DEBUG_DCF("yCbCrCoeffecients\n");
            break;
            
        case EXIF_TAG_REFERENCE_BLACK_WHITE:
            //DEBUG_DCF("referenceBlackWhite\n");
            break;
            
        /* Other tags */
        case EXIF_TAG_DATE_TIME:
            //DEBUG_DCF("dateTime\n");
            break;
            
        case EXIF_TAG_IMAGE_DESCRIPTION:
            //DEBUG_DCF("imageDescription\n");
            break;
            
        case EXIF_TAG_MAKE:
            //DEBUG_DCF("make\n");
            break;
            
        case EXIF_TAG_MODEL:
            //DEBUG_DCF("model\n");
            break;
            
        case EXIF_TAG_SOFTWARE:
            //DEBUG_DCF("software\n");
            break;
            
        case EXIF_TAG_ARTIST:
            //DEBUG_DCF("artist\n");
            break;
            
        case EXIF_TAG_COPYRIGHT:
            //DEBUG_DCF("copyright\n");
            break;
            
        /*---- EXIF IFD Attribute Information ----*/

        /* Tags relating to version */
        case EXIF_TAG_EXIF_VERSION:
            //DEBUG_DCF("exifVersion\n");
            break;
            
        case EXIF_TAG_FLASHPIX_VERSION:
            //DEBUG_DCF("flashpixVersion\n");
            break;
            
        /* Tags relating to image data characteristics */
        case EXIF_TAG_COLOR_SPACE:
            //DEBUG_DCF("colorSpace\n");
            break;

        /* Tags relating to image configuration */
        case EXIF_TAG_COMPONENTS_CONFIGURATION:
            //DEBUG_DCF("componentsConfiguration\n");
            break;
            
        case EXIF_TAG_COMPRESSED_BITS_PER_PIXEL:
            //DEBUG_DCF("comperssedBitsPerPixel\n");
            break;
            
        case EXIF_TAG_PIXEL_X_DIMENSION:
            //DEBUG_DCF("pixelXDimension\n");
            break;
            
        case EXIF_TAG_PIXEL_Y_DIMENSION:
            //DEBUG_DCF("pixelYDimension\n");
            break;

        /* Tags relating to user information */
        case EXIF_TAG_MAKER_NOTE:
            //DEBUG_DCF("markerNote\n");
            break;
            
        case EXIF_TAG_USER_COMMENT:
            //DEBUG_DCF("userComment\n");
            break;

        /* Tags relating to related file information */
        case EXIF_TAG_RELATED_SOUND_FILE:
            //DEBUG_DCF("relatedSoundFile\n");
            break;

        /* Tags relating to date and time */
        case EXIF_TAG_DATE_TIME_ORIGINAL:
            //DEBUG_DCF("dateTimeOriginal\n");
            break;
            
        case EXIF_TAG_DATE_TIME_DIGITIZED:
            //DEBUG_DCF("dateTimeDigitized\n");
            break;
            
        case EXIF_TAG_SUBSEC_TIME:
            //DEBUG_DCF("subsecTime\n");
            break;
            
        case EXIF_TAG_SUBSEC_TIME_ORIGINAL:
            //DEBUG_DCF("subsecTimeOriginal\n");
            break;
            
        case EXIF_TAG_SUBSEC_TIME_DIGITIZED:
            //DEBUG_DCF("subsecTimeDigitized\n");
            break;
            
        /* Tags relating to picture-taking conditions */
        case EXIF_TAG_EXPOSURE_TIME:
            //DEBUG_DCF("exposureTime\n");
            break;
            
        case EXIF_TAG_F_NUMBER:
            //DEBUG_DCF("fNumber\n");
            break;
            
        case EXIF_TAG_EXPOSURE_PROGRAM:
            //DEBUG_DCF("exposureProgram\n");
            break;
            
        case EXIF_TAG_SPECTRAL_SENSITIVITY:
            //DEBUG_DCF("spectralSensitivity\n");
            break;
            
        case EXIF_TAG_ISO_SPEED_RATINGS:
            //DEBUG_DCF("isoSpeedRatings\n");
            break;
            
        case EXIF_TAG_OECF:
            //DEBUG_DCF("optoElectricConversionFactor\n");
            break;
            
        case EXIF_TAG_SHUTTER_SPEED_VALUE:
            //DEBUG_DCF("shutterSpeedValue\n");
            break;
            
        case EXIF_TAG_APERTURE_VALUE:
            //DEBUG_DCF("apertureValue\n");
            break;
            
        case EXIF_TAG_BRIGHTNESS_VALUE:
            //DEBUG_DCF("brightnessValue\n");
            break;
            
        case EXIF_TAG_EXPOSURE_BIAS_VALUE:
            //DEBUG_DCF("exposureBiasValue\n");
            break;
            
        case EXIF_TAG_MAX_APERTURE_RATIO_VALUE:
            //DEBUG_DCF("maxApertureValue\n");
            break;
            
        case EXIF_TAG_SUBJECT_DISTANCE:
            //DEBUG_DCF("subjectDistance\n");
            break;
            
        case EXIF_TAG_METERING_MODE:
            //DEBUG_DCF("meteringMode\n");
            break;
            
        case EXIF_TAG_LIGHT_SOURCE:
            //DEBUG_DCF("lightSource\n");
            break;
            
        case EXIF_TAG_FLASH:
            //DEBUG_DCF("flash\n");
            break;
            
        case EXIF_TAG_FOCAL_LENGTH:
            //DEBUG_DCF("focalLength\n");
            break;
            
        case EXIF_TAG_SUBJECT_AREA:
            //DEBUG_DCF("subjectArea\n");
            break;
            
        case EXIF_TAG_FLASH_ENERGY:
            //DEBUG_DCF("flashEnergy\n");
            break;
            
        case EXIF_TAG_SPATIAL_FREQUENCY_RESPONSE:
            //DEBUG_DCF("spatialFrequencyResponse\n");
            break;
            
        case EXIF_TAG_FOCAL_PLANE_X_RESOLUTION:
            //DEBUG_DCF("focalPlaneXResolution\n");
            break;
            
        case EXIF_TAG_FOCAL_PLANE_Y_RESOLUTION:
            //DEBUG_DCF("focalPlaneYResolution\n");
            break;
            
        case EXIF_TAG_FOCAL_PLANE_RESOLUTION_UNIT:
            //DEBUG_DCF("focalPlaneResolutionUnit\n");
            break;
            
        case EXIF_TAG_SUBJECT_LOCATION:
            //DEBUG_DCF("subjectLocation\n");
            break;
            
        case EXIF_TAG_EXPOSURE_INDEX:
            //DEBUG_DCF("exposureIndex\n");
            break;
            
        case EXIF_TAG_SENSING_METHOD:
            //DEBUG_DCF("sensingMethod\n");
            break;
            
        case EXIF_TAG_FILE_SOURCE:
            //DEBUG_DCF("fileSource\n");
            break;
            
        case EXIF_TAG_SCENE_TYPE:
            //DEBUG_DCF("sceneType\n");
            break;
            
        case EXIF_TAG_CFA_PATTERN:
            //DEBUG_DCF("cfaPattern\n");
            break;
            
        case EXIF_TAG_CUSTOM_RENDERED:
            //DEBUG_DCF("customRendered\n");
            break;
            
        case EXIF_TAG_EXPOSURE_MODE:
            //DEBUG_DCF("exposureMode\n");
            break;
            
        case EXIF_TAG_WHITE_BALANCE:
            //DEBUG_DCF("whiteBalance\n");
            break;
            
        case EXIF_TAG_DIGITAL_ZOOM_RATIO:
            //DEBUG_DCF("digitalZoomRatio\n");
            break;
            
        case EXIF_TAG_FOCAL_LENGTH_35mm_FILE:
            //DEBUG_DCF("focalLength35mmFile\n");
            break;
            
        case EXIF_TAG_SCENE_CAPTURE_TYPE:
            //DEBUG_DCF("sceneCaptureType\n");
            break;
            
        case EXIF_TAG_GAIN_CONTROL:
            //DEBUG_DCF("gainControl\n");
            break;
            
        case EXIF_TAG_CONTRAST:
            //DEBUG_DCF("contrast\n");
            break;
            
        case EXIF_TAG_SATURATION:
            //DEBUG_DCF("saturation\n");
            break;
            
        case EXIF_TAG_SHARPNESS:
            //DEBUG_DCF("sharpness\n");
            break;
            
        case EXIF_TAG_DEVICE_SETTING_DESCRIPTION:
            //DEBUG_DCF("deviceSettingDescription\n");
            break;
            
        case EXIF_TAG_SUBJECT_DISTANCE_RANGE:
            //DEBUG_DCF("subjectDistanceRange\n");
            break;

        /* Other tags */
        case EXIF_TAG_IMAGE_UNIQUE_ID:
            //DEBUG_DCF("imageUniqueId\n");
            break;

        /*---- GPS Attribute Information ----*/

        /* Tags relating to GPS */
        case EXIF_TAG_GPS_VERSION_ID:
            //DEBUG_DCF("gpsVersionId\n");
            break;
            
        case EXIF_TAG_GPS_LATITUDE_REF:
            //DEBUG_DCF("gpsLatitudeRef\n");
            break;
            
        case EXIF_TAG_GPS_LATITUDE:
            //DEBUG_DCF("gpsLatitude\n");
            break;
            
        case EXIF_TAG_GPS_LONGITUDE_REF:
            //DEBUG_DCF("gpsLongitudeRef\n");
            break;
            
        case EXIF_TAG_GPS_LONGITUDE:
            //DEBUG_DCF("gpsLongitude\n");
            break;
            
        case EXIF_TAG_GPS_ALTITUDE_REF:
            //DEBUG_DCF("gpsAltitudeRef\n");
            break;
            
        case EXIF_TAG_GPS_ALTITUDE:
            //DEBUG_DCF("gpsAltitude\n");
            break;
            
        case EXIF_TAG_GPS_TIME_STAMP:
            //DEBUG_DCF("gpsTimeStamp\n");
            break;
            
        case EXIF_TAG_GPS_SATELLITES:
            //DEBUG_DCF("gpsSatellites\n");
            break;
            
        case EXIF_TAG_GPS_STATUS:
            //DEBUG_DCF("gpsStatus\n");
            break;
            
        case EXIF_TAG_GPS_MEASURE_MODE:
            //DEBUG_DCF("gpsMeasureMode\n");
            break;
            
        case EXIF_TAG_GPS_DOP:
            //DEBUG_DCF("gpsDop\n");
            break;
            
        case EXIF_TAG_GPS_SPEED_REF:
            //DEBUG_DCF("gpsSpeedref\n");
            break;
            
        case EXIF_TAG_GPS_SPEED:
            //DEBUG_DCF("gpsSpeed\n");
            break;
            
        case EXIF_TAG_GPS_TRACK_REF:
            //DEBUG_DCF("gpsTrackRef\n");
            break;
            
        case EXIF_TAG_GPS_TRACK:
            //DEBUG_DCF("gpsTrack\n");
            break;
            
        case EXIF_TAG_GPS_IMG_DIRECTION_REF:
            //DEBUG_DCF("gpsImgDirectionRef\n");
            break;
            
        case EXIF_TAG_GPS_IMG_DIRECTION:
            //DEBUG_DCF("gpsImgDirection\n");
            break;
            
        case EXIF_TAG_GPS_MAP_DATUM:
            //DEBUG_DCF("gpsMapDatum\n");
            break;
            
        case EXIF_TAG_GPS_DEST_LATITUDE_REF:
            //DEBUG_DCF("gpsDestLatitudeRef\n");
            break;
            
        case EXIF_TAG_GPS_DEST_LATITUDE:
            //DEBUG_DCF("gpsDestLatitude\n");
            break;
            
        case EXIF_TAG_GPS_DEST_LONGITUDE_REF:
            //DEBUG_DCF("gpsDestLongitudeRef\n");
            break;
            
        case EXIF_TAG_GPS_DEST_LONGITUDE:
            //DEBUG_DCF("gpsDestLongitude\n");
            break;
            
        case EXIF_TAG_GPS_DEST_BEARING_REF:
            //DEBUG_DCF("gpsDestBearingRef\n");
            break;
            
        case EXIF_TAG_GPS_DEST_BEARING:
            //DEBUG_DCF("gpsDestBearing\n");
            break;
            
        case EXIF_TAG_GPS_DEST_DISTANCE_REF:
            //DEBUG_DCF("gpsDestDistanceRef\n");
            break;
            
        case EXIF_TAG_GPS_DEST_DISTANCE:
            //DEBUG_DCF("gpsDestDistance\n");
            break;
            
        case EXIF_TAG_GPS_PROCESSING_METHOD:
            //DEBUG_DCF("gpsProcessingMethod\n");
            break;
            
        case EXIF_TAG_GPS_AREA_INFORMATION:
            //DEBUG_DCF("gpsAreaInformation\n");
            break;
            
        case EXIF_TAG_GPS_DATE_STAMP:
            //DEBUG_DCF("gpsDateStamp\n");
            break;
            
        case EXIF_TAG_GPS_DIFFERENTIAL:
            //DEBUG_DCF("gpsDifferential\n");
            break;
        
        /* Other tag */ 
        default:
            DEBUG_DCF("Error: Unknown tag.\n");
            return 0;
    }
    
    return 1;   
}

/*

Routine Description:

    IFD type.

Arguments:

    type - IFD type.
    pUnit - Bytes per unit.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifIfdType(u16 type, u32* pUnit)
{
    switch (type)
    {
        case EXIF_TYPE_BYTE:
            //DEBUG_DCF("byte\n");
            *pUnit = 1;
            break;
            
        case EXIF_TYPE_ASCII:
            //DEBUG_DCF("ascii\n");
            *pUnit = 1;
            break;
            
        case EXIF_TYPE_SHORT:
            //DEBUG_DCF("short\n");
            *pUnit = 2;
            break;
            
        case EXIF_TYPE_LONG:
            //DEBUG_DCF("long\n");
            *pUnit = 4;
            break;
            
        case EXIF_TYPE_RATIONAL:
            //DEBUG_DCF("rational\n");
            *pUnit = 8;
            break;
            
        case EXIF_TYPE_UNDEFINED:
            //DEBUG_DCF("undefined\n");
            *pUnit = 1;
            break;
            
        case EXIF_TYPE_SLONG:
            //DEBUG_DCF("slong\n");
            *pUnit = 4;
            break;
            
        case EXIF_TYPE_SRATIONAL:
            //DEBUG_DCF("srational\n");
            *pUnit = 8;
            break;
        
        default:
            //DEBUG_DCF("unknownType\n");
            *pUnit = 1;
            return 0;
    }           
    
    return 1;
}

/*

Routine Description:

    IFDs name.

Arguments:

    nameIndex - Name index.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifIfdsName(u8 nameIndex)
{
    switch (nameIndex)
    {
        case EXIF_IFDS_EXIF_IFD:
            //DEBUG_DCF("exifIfd\n");
            break;
                    
        case EXIF_IFDS_GPS_INFO_IFD:
            //DEBUG_DCF("gpsInfoIfd\n");
            break;
                        
        case EXIF_IFDS_INTEROP_IFD:
            //DEBUG_DCF("interopIfd\n");
            break;
                        
        default:
            //DEBUG_DCF("unknownIfd\n");
            return 0;       
    };          
    
    return 1;   
}   

/*

Routine Description:

    EXIF COM parsing.

Arguments:

    pBuf - COM buffer.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifComParse(u8* pBuf)
{   
    u16 lc;
    u16 i;
        
    /* lc */
    lc =  (*pBuf++) << 8;
    lc |= (*pBuf++);
    //DEBUG_DCF("Lc = 0x%04x\n", lc);
    
    /* dump */
    //DEBUG_DCF("Cm = \"");
    for (i = 0; i < (lc - 2); i++)
    {
        if (*pBuf != '\0')
        {
            //DEBUG_DCF("%c", *pBuf++);
        }   
    }       
    //DEBUG_DCF("\"\n");    
    
    return 1;
}

/*

Routine Description:

    EXIF DQT parsing.

Arguments:

    pBuf - DQT buffer.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifDqtParse(u8* pBuf)
{   
    u16 lq;
    u8 pqTq, pq, tq;
            
    /* lq */
    lq =  (*pBuf++) << 8;
    lq |= (*pBuf++);
    //DEBUG_DCF("Lq = 0x%04x\n", lq);
    
    /* dqt */
    lq -= 2;
    do 
    {
        pqTq = *pBuf++;
        pq = pqTq & 0xf0;
        tq = pqTq & 0x0f;
            
        if(pq) {
            DEBUG_DCF("Error: Don't support 16-bit Q value precision!!!\n");
            return 0;
        }
            
        /*            
        switch (tq) // only two tables (Y and Cb/Cr) are supported
        {
            case JPEG_COMPONENT_Y:
            case JPEG_COMPONENT_Cb:
            case JPEG_COMPONENT_Cr:
                if (jpegSetQuantizationTable(tq, pq, pBuf, 1) == 0)
                    return 0;
                break;
                
            default:
                return 0;
        }               
        */
        
        memcpy((void*)Qtable[tq], pBuf, 64);
        
        /* dump */      
        /*
        DEBUG_DCF("Tq = 0x%02x -> ", tq);
        for (i = 0; i < 64; i++)
        {
            //DEBUG_DCF("0x%02x ", *(pBuf + i));
        }   
        */
        //DEBUG_DCF("\n");
        
        pBuf += 64; 
        lq -= 65;   
    } while (lq > 0);   

    return 1;
}

/*

Routine Description:

    EXIF DHT parsing.

Arguments:

    pBuf - DHT buffer.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifDhtParse(u8* pBuf)
{   
    u16 lh;
    u8 tcTh, tc, th;
    u16 size;
    u16 i;
        
    /* lh */
    lh =  (*pBuf++) << 8;
    lh |= (*pBuf++);
    //DEBUG_DCF("Lh = 0x%04x\n", lh);
    
    /* dht */
    lh -= 2;
    do 
    {
        tcTh = *pBuf++;
        tc = tcTh & 0xf0;
        th = tcTh & 0x0f;
        
        switch (th) /* only two tables (Y and Cb/Cr) are supported */
        {
            case JPEG_COMPONENT_Y:
            case JPEG_COMPONENT_Cb:
            case JPEG_COMPONENT_Cr:
                if (jpegSetHuffmanTable(th, tc, pBuf, pBuf + 16,1) == 0)/*BJ 0523 S*/
                    return 0;
                break;
                
            default:
                return 0;
        }               
        
        for (i = 0, size = 16; i < 16; i++)
            size += *(pBuf + i);
        
        /* dump */
        //DEBUG_DCF("Th = 0x%02x, Tc = 0x%02x -> ", th, tc);
        //DEBUG_DCF("L: ");
        for (i = 0; i < 16; i++)
        {
            //DEBUG_DCF("0x%02x ", *(pBuf + i));
        }
        //DEBUG_DCF("V: ");
        for (i = 0; i < (size - 16); i++)
        {
            //DEBUG_DCF("0x%02x ", *(pBuf + 16 + i));   
        }   
        //DEBUG_DCF("\n");
        
        pBuf += size;       
        lh -= (1 + size);   
    } while (lh > 0);   

    return 1;
}

/*

Routine Description:

    EXIF SOF0 parsing.

Arguments:

    pBuf - SOF0 buffer.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSof0Parse(u8* pBuf, u16* pWidth, u16* pHeight, u8* pformat)
{   
    u16 lf;
    u8 p;
    u16 y, x;
    u8  nf, /*c[3],*/ hv[3]/*, tq[3]*/;
    u8 format;
    u8 i;
        
    /* lf */
    lf =  (*pBuf++) << 8;
    lf |= (*pBuf++);
    //DEBUG_DCF("Lf = 0x%04x\n", lf);
    
    /* sof0 */
    p = *pBuf++;        /* precision */
    if (p != 0x08) {     /* only 8 bit supported */
        DEBUG_DCF("Don't support %d-bit sample precision!!!\n", p);
        return 0;
    }
    y =  (*pBuf++) << 8;    /* y */
    y |= (*pBuf++);
    x =  (*pBuf++) << 8;    /* x */
    x |= (*pBuf++);
    
    gJPGValidWidth  = x;
    gJPGValidHeight = y; 
    
    nf = *pBuf++;       /* Y/Cb/Cr supported only */
    if (nf != 3) {
        DEBUG_DCF("Don't support %d image components!!!\n", nf);
        return 0;
    }
    for (i = 0; i < 3; i++)
    {   
        C[i] = *pBuf++;         
        hv[i] = *pBuf++;
        TQ[i] = *pBuf++;
    }   
    if ((C[0] != 0x01)) {            /* Y component */
        DEBUG_DCF("Y component id is 0x%02x!!!\n", C[0]);
        //return 0;
    }   
    if (hv[0] == 0x21) {
        format = JPEG_FORMAT_YCbCr_422;
        x       = (x + 15) & ~15;
        y       = (y +  7) & ~7;
        //DEBUG_DCF("YUV422\n");
    } else if (hv[0] == 0x22) {
        format = JPEG_FORMAT_YCbCr_420;
        x       = (x + 15) & ~15;
        y       = (y + 15) & ~15;
        //DEBUG_DCF("YUV420\n");
    } else if (hv[0] == 0x12) {
        format  = JPEG_FORMAT_YCbCr_440;
        x       = (x +  7) & ~7;
        y       = (y + 15) & ~15;
        //DEBUG_DCF("YUV440\n");
    } else if (hv[0] == 0x11) {
        format  = JPEG_FORMAT_YCbCr_444;
        x       = (x +  7) & ~7;
        y       = (y +  7) & ~7;
        //DEBUG_DCF("YUV444\n");
    } else {
        return 0;   
    }
    if ((C[1] != 0x02) || (hv[1] != 0x11)) {  /* Cb component */
        DEBUG_DCF("U component id is 0x%02x!!!\n", C[1]);
        //return 0;
    }
    if ((C[2] != 0x03) || (hv[2] != 0x11)) {  /* Cr component */
        DEBUG_DCF("V component id is 0x%02x!!!\n", C[2]);
        //return 0;   
    }
    
    //jpegSetDataFormat(format);
    //jpegSetImageResolution(x, y);/*BJ 0523 S*/
    
    *pWidth     = x;
    *pHeight    = y;
    *pformat    = format;
    
    gJPGDecodedWidth  = x;
    gJPGDecodedHeight = y;
    /* dump */
    //DEBUG_DCF("P = 0x%02x, Y = 0x%04x, X = 0x%04x, Nf = 0x%02x, ", p, y, x, nf);  
    //DEBUG_DCF("C1 = 0x%02x, H1V1 = 0x%02x, Tq1 = 0x%02x, ", C[0], hv[0], tq[0]);
    //DEBUG_DCF("C2 = 0x%02x, H2V2 = 0x%02x, Tq2 = 0x%02x, ", C[1], hv[1], tq[1]);
    //DEBUG_DCF("C3 = 0x%02x, H3V3 = 0x%02x, Tq3 = 0x%02x\n", C[2], hv[2], tq[2]);
    
    return 1;
}   

/*

Routine Description:

    EXIF DRI parsing.

Arguments:

    pBuf - DRI buffer.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifDriParse(u8* pBuf, u16* ri)
{   
    u16 lr;
    //u16 ri;
    u16 tmp;
    
    /* lr */
    lr =  (*pBuf++) << 8;
    lr |= (*pBuf++);
    //DEBUG_DCF("Lr = 0x%04x\n", lr);
    
    /* dri */
    tmp     = (*pBuf++) << 8;
    *ri     = tmp | (*pBuf++);
    
    //jpegSetRestartInterval(ri);

    /* dump */
    //DEBUG_DCF("Ri = 0x%04x\n", ri);
    
    return 1;
}

/*

Routine Description:

    EXIF SOS parsing.

Arguments:

    pBuf - SOS buffer.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifSosParse(u8* pBuf)
{   
    u16 ls;
    u8 ns, cs[3], tdta[3], ss, se, ahal;
    u8 i;
    
    /* ls */
    ls =  (*pBuf++) << 8;
    ls |= (*pBuf++);
    //DEBUG_DCF("Ls = 0x%04x\n", ls);
    
    /* sos */
    ns = *pBuf++;
    if (ns != 3)
        return 0;
    for (i = 0; i < 3; i++)
    {   
        cs[i] = *pBuf++;
        tdta[i] = *pBuf++;
    }   
    /*
    if ((cs[0] != 0x01) || (tdta[0] != 0x00))
        return 0;
    if ((cs[1] != 0x02) || (tdta[1] != 0x11))
        return 0;
    if ((cs[2] != 0x03) || (tdta[2] != 0x11))
        return 0;
        */
    if ((cs[0] != C[0]) || (tdta[0] != 0x00)) {
        DEBUG_DCF("Cs1 = 0x%02x, Td1Ta1 = 0x%02x\n", cs[0], tdta[0]);
        return 0;
    }
    if ((cs[1] != C[1]) || (tdta[1] != 0x11)) {
        DEBUG_DCF("Cs2 = 0x%02x, Td2Ta2 = 0x%02x\n", cs[1], tdta[1]);
        return 0;
    }
    if ((cs[2] != C[2]) || (tdta[2] != 0x11)) {
        DEBUG_DCF("Cs3 = 0x%02x, Td3Ta3 = 0x%02x\n", cs[2], tdta[2]);
        return 0;
    }

    ss = *pBuf++;
    se = *pBuf++;
    ahal = *pBuf++;
    if ((ss != 0x00) || (se != 0x3f) || (ahal != 0x00)) {
        DEBUG_DCF("Ss = 0x%02x, Se = 0x%02x, AhAl = 0x%02x\n", ss, se, ahal);
        return 0;   
    }
    //DEBUG_DCF("Ns = 0x%02x, ", ns);
    //DEBUG_DCF("Cs1 = 0x%02x, Td1Ta1 = 0x%02x, ", cs[0], tdta[0]);
    //DEBUG_DCF("Cs2 = 0x%02x, Td2Ta2 = 0x%02x, ", cs[1], tdta[1]);
    //DEBUG_DCF("Cs3 = 0x%02x, Td3Ta3 = 0x%02x, ", cs[2], tdta[2]);
    //DEBUG_DCF("Ss = 0x%02x, Se = 0x%02x, AhAl = 0x%02x\n", ss, se, ahal);

    // Fill Quantization SRAM
    for(i = 0; i < 3; i++) {
        jpegSetQuantizationTable(i, 0, (u8*)Qtable[TQ[i]], 1);
    }

    return 1;
}

s32 RawWriteFile(u32 size , u8 * buf)
{
    FS_FILE* pFile;
    u32 ret;    
	u8  tmp;
	
    /* create next file */
    if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_RAW, 0)) == NULL)
        return 0;
    if (dcfWrite(pFile, buf, size , &ret) == 0)
    {
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }
    dcfCloseFileByIdx(pFile, 0, &tmp);
    return 1;
}

/*

Routine Description:

    Write file.

Arguments:

    thumbnailImageSize  - Thumbnail image size.
    primaryImageSize    - Primary image size.
    ChannelID           - Image channel ID.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifWriteFile(u32 thumbnailImageSize, u32 primaryImageSize, u8 ChannelID)
{
    u32 size;
    FS_FILE* pFile;
    u32 app4_size,data_size;
    u32 TotalSize;
	u8  tmp;
	
    TotalSize = 0;
    /* create next file */
    //DEBUG_DCF("exifWriteFile: dcfCreateNextFile()\n");
    if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_JPG, ChannelID)) == NULL) {
        DEBUG_DCF("dcfCreateNextFile(DCF_FILE_TYPE_JPG, %d) error!!!\n", ChannelID);
        return 0;
    }

#if ADDAPP1TOJPEG    
    /* write thumbnail to file */
    if (dcfWrite(pFile, (unsigned char*)&exifThumbnailImage, exifThumbnailImageHeaderSize, &size) == 0)
    {
        DEBUG_DCF("dcfWrite exifThumbnailImageHeader error, size = %d!!!\n", size);
        dcfCloseFileByIdx(pFile, ChannelID, &tmp);
        return 0;
    }
    TotalSize += exifThumbnailImageHeaderSize;
    
    if (dcfWrite(pFile, (unsigned char*)exifThumbnailImage.bitStream, thumbnailImageSize, &size) == 0)
    {
        DEBUG_DCF("dcfWrite thumbnailImage error, size = %d!!!\n", size);
        dcfCloseFileByIdx(pFile, ChannelID, &tmp);
        return 0;
    }
    TotalSize += thumbnailImageSize;
#else
	//write EXIF_MARKER_SOI(FF D8)
	if (dcfWrite(pFile, (unsigned char*)&exifThumbnailImage, 2, &size) == 0)
	{
		DEBUG_DCF("dcfWrite exifThumbnailImageHeader error, size = %d!!!\n", size);
		dcfCloseFileByIdx(pFile, ChannelID, &tmp);
		return 0;
	}
    TotalSize += 2;
	
#endif

#if ADDAPP2TOJPEG
    /*write app2 to file*/
    if (dcfWrite(pFile, (unsigned char*)exifApp2Data, sizeof(DEF_APPENDIXINFO), &size) == 0)
    {
        DEBUG_DCF("dcfWrite thumbnailImage error, size = %d!!!\n", size);
        dcfCloseFileByIdx(pFile, ChannelID, &tmp);
        return 0;
    }
    TotalSize += sizeof(DEF_APPENDIXINFO);
#endif

    /*write app3 to file*/
#if ADDAPP3TOJPEG
    if(sysInsertAPP3_ena)
    {
        if (dcfWrite(pFile, (unsigned char*)&exifAPP3Prefix, sizeof(DEF_APP3PREFIX), &size) == 0)
        {
            DEBUG_DCF("dcfWrite thumbnailImage error, size = %d!!!\n", size);
            dcfCloseFileByIdx(pFile, ChannelID, &tmp);
            return 0;
        }
        TotalSize += sizeof(DEF_APP3PREFIX);
        
        if (dcfWrite(pFile, (unsigned char*)&exifAPP3VGAImage, sizeof(EXIF_PRIMARY), &size) == 0)
        {
            DEBUG_DCF("dcfWrite thumbnailImage error, size = %d!!!\n", size);
            dcfCloseFileByIdx(pFile, ChannelID, &tmp);
            return 0;
        }
        TotalSize += sizeof(EXIF_PRIMARY);
        
        if (dcfWrite(pFile, (unsigned char*)exifAPP3VGAImage.bitStream, Global_APP3VGAImageSize, &size) == 0)
        {
            DEBUG_DCF("dcfWrite primaryImage error, size = %d!!!\n", size);
            dcfCloseFileByIdx(pFile, ChannelID, &tmp);
            return 0;
        }
        TotalSize += Global_APP3VGAImageSize;
     }
#endif

    // Write dummy to APP4 for align address
    app4_size=(TotalSize+ exifPrimaryImageHeaderSize);
    app4_size &= 0x0F ;
    if(app4_size!=0)
    {
        app4_size=( TotalSize+ exifPrimaryImageHeaderSize + sizeof(DEF_APP4PREFIX) ) & 0x0F ;
        if(app4_size==0)    // Add marker length
            app4_size=0x10;
        else
            app4_size=0x10-app4_size;
        exifAPP4Prefix.APP4Marker= EXIF_MARKER_APP4;
        data_size= app4_size+2;
        exifAPP4Prefix.APP4Size= (data_size<<8)|(data_size>>8); 
        if (dcfWrite(pFile, (unsigned char*)&exifAPP4Prefix, sizeof(DEF_APP4PREFIX), &size) == 0)
        {
            DEBUG_DCF("dcfWrite thumbnailImage error, size = %d!!!\n", size);
            dcfCloseFileByIdx(pFile, ChannelID, &tmp);
            return 0;
        }

        if (dcfWrite(pFile, (unsigned char*)PKBuf, app4_size, &size) == 0)//Insert Dummy data
        {
            DEBUG_DCF("dcfWrite thumbnailImage error, size = %d!!!\n", size);
            dcfCloseFileByIdx(pFile, ChannelID, &tmp);
            return 0;
        }            
    }        

    /* write primary to file */
    if (dcfWrite(pFile, (unsigned char*)&exifPrimaryImage, exifPrimaryImageHeaderSize, &size) == 0)
    {
        DEBUG_DCF("dcfWrite exifPrimaryImageHeader error, size = %d!!!\n", size);
        dcfCloseFileByIdx(pFile, ChannelID, &tmp);
        return 0;
    }

    if (dcfWrite(pFile, (unsigned char*)exifPrimaryImage.bitStream, primaryImageSize, &size) == 0)
    {
        DEBUG_DCF("dcfWrite primaryImage error, size = %d!!!\n", size);
        dcfCloseFileByIdx(pFile, ChannelID, &tmp);
        return 0;
    }

/*CY 0629 E*/   
    
    /* close file */
    dcfCloseFileByIdx(pFile, ChannelID, &tmp);
    return 1;   
}

/*

Routine Description:

    Read file.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifReadFile(void)
{
/*BJ 0523 S*/
    u32 sizeUsed,size;
    s32 fsize,rsize;
    u16 sizeX,sizeY;
    u8 *bitsbuf;
    u8  tmp;
    
/*BJ 0523 E*/
    FS_FILE* pFile;
       
    /* open file */
    if ((pFile = dcfOpen((char *)dcfPlaybackCurFile->pDirEnt->d_name, "r")) == NULL)
        return 0;
    /*BJ 0523 S*/
    fsize = pFile->size;
    
    /* Peter 070104 */
    if(fsize <= EXIFDECBUF_SIZE) 
    {
        //==For Thumbnail==//
        if(fsize<=EXIF_THUMBNAIL_MAX+640*480) //Lucian: 縮短讀檔時間
        {
           dcfRead(pFile, exifDecBuf, fsize, &size);
           rsize=0;
        }
        else
        {
           dcfRead(pFile, exifDecBuf, EXIF_THUMBNAIL_MAX+640*480, &size);
           rsize=fsize-(EXIF_THUMBNAIL_MAX+640*480);
        }
        bitsbuf=exifDecBuf+size; 
#if DINAMICALLY_POWER_MANAGEMENT
        sysJPEG_enable();
#endif
        exifFileParse(JPEG_IMAGE_THUMBNAIL, exifDecBuf, size, &sizeUsed, &sizeX, &sizeY);
#if DINAMICALLY_POWER_MANAGEMENT
        sysJPEG_disable();
#endif              
        //==>if detect next or previous, dcfClose(pFile); return 1;
        if(sysCheckNextEvtIsPrevOrNext())
        {
           dcfClose(pFile, &tmp);
           return 1;
        }

        if(((u32)exifDecBuf + fsize) >= DRAM_MEMORY_END) {
            DEBUG_DCF("Error: JPEG file too large for decode, last address is 0x%08x!!!\n", ((u32)exifDecBuf + fsize));
            return 1;
        }
        //==For Primary JPEG==//
        if(exifAPP3VGAImg_flag)
        {}
        else
        {
            //將剩餘的讀完
            while(rsize > 256*1024)     
            {
               dcfRead(pFile, bitsbuf, 256*1024, &size);
               bitsbuf +=256*1024;
               rsize -= 256*1024;
               //==>if detect next or previous, dcfClose(pFile); return 1;
               if(sysCheckNextEvtIsPrevOrNext())
               {
                  dcfClose(pFile, &tmp);
                  return 1;
               }
            }
            
            dcfRead(pFile, bitsbuf, rsize, &size);
            //==>if detect next or previous, dcfClose(pFile); return 1;
            if(sysCheckNextEvtIsPrevOrNext())
            {
               dcfClose(pFile, &tmp);
               return 1;
            }
        }

#if DINAMICALLY_POWER_MANAGEMENT
        sysJPEG_enable();
#endif      
#if(JPEG_DECMODE_OPTION == JPEG_DECODE_MODE_SLICE)
     if(exifAPP3VGAImg_flag)
     {
        exifFileParse(JPEG_IMAGE_PRIMARY, exifDecBuf, size, &sizeUsed, &sizeX, &sizeY);
        if(jpegDecompression(exifDecBuf + sizeUsed, PKBuf0) == 0)    
           DEBUG_DCF("Primary JPEG decode error!!!\n");
     }
     else
     {
     #if DINAMICALLY_POWER_MANAGEMENT
          sysISU_enable();
     #endif
         if(jpegDecompressionSlice(exifDecBuf + sizeUsed, PKBuf0) == 0)
             DEBUG_DCF("Primary JPEG decode error!!!\n");
      #if DINAMICALLY_POWER_MANAGEMENT
         sysISU_disable();
      #endif
         sizeX   = 640;  //SliceJpegdecoding  固定解出640x480 resolution.
         sizeY   = 480;
     }
     
#else
     if(exifAPP3VGAImg_flag)
     {
        exifFileParse(JPEG_IMAGE_PRIMARY, exifDecBuf, size, &sizeUsed, &sizeX, &sizeY);
     }
     #if (CHIP_OPTION == CHIP_A1018A)
     if(jpegDecompression(exifDecBuf + sizeUsed, PKBuf0) == 0)    
        DEBUG_DCF("Primary JPEG decode error!!!\n");
     #else
     if(jpegDecompressionYUV420(exifDecBuf + sizeUsed, PKBuf0,PKBuf0 + PNBUF_SIZE_Y,1,TVOUT_X) == 0)    
        DEBUG_DCF("Primary JPEG decode error!!!\n");
     #endif
#endif        
#if DINAMICALLY_POWER_MANAGEMENT
    sysJPEG_disable();
#endif          
        //==>if detect next or previous, dcfClose(pFile); return 1;
        if(sysCheckNextEvtIsPrevOrNext())
        {
           dcfClose(pFile, &tmp);
           return 1;
        }
    } 
    else 
    {
        DEBUG_DCF("JPEG file %s size biger than exifDecBuf size!!!\n", dcfPlaybackCurFile->pDirEnt->d_name);
    }

    dcfClose(pFile, &tmp);

    if((splitmenu==1) || (splitmenu==2))
        iduPlaybackFrame(PKBuf0);
    else
        iduPlaybackFrame(PKBuf0);
    return 1;
}

/*

Routine Description:

    Read Thumbnail file.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 exifReadThumbnailFile(void)
{
    u32 sizeUsed,size;
    s32 fsize;
    u16 sizeX,sizeY;
    FS_FILE* pFile;
    u8  tmp;
	
    /* open file */
    if ((pFile = dcfOpen((char *)dcfPlaybackCurFile->pDirEnt->d_name, "r")) == NULL)
        return 0;
    
    /*BJ 0523 S*/
    fsize = pFile->size;
    
    /* Peter 070104 */
    if(fsize <= EXIFDECBUF_SIZE) 
    {
        
        //==For Thumbnail==//
        dcfRead(pFile, exifDecBuf, EXIF_THUMBNAIL_MAX, &size);
#if DINAMICALLY_POWER_MANAGEMENT
        sysJPEG_enable();
#endif
        exifFileParse(JPEG_IMAGE_THUMBNAIL, exifDecBuf, size, &sizeUsed, &sizeX, &sizeY);
#if DINAMICALLY_POWER_MANAGEMENT
        sysJPEG_disable();
#endif
    dcfClose(pFile, &tmp);
    }
    return 1;
}

s32 exifDecodeJPEGToYUV(u8 *cYUVRawData, u8 *JpegBitStream, u32 uFilesize, u32 *uWidth, u32 *uHeight)
{
/*BJ 0523 S*/
    u32 sizeUsed;
    u16 sizeX, sizeY;

    
    /* (VCC 20070115) S */
#if DINAMICALLY_POWER_MANAGEMENT
     sysJPEG_enable();
#endif
    exifFileParse(JPEG_IMAGE_PRIMARY, JpegBitStream, uFilesize, &sizeUsed, &sizeX, &sizeY);
    
    if(uFilesize > 35000)
        DEBUG_DCF("uFilesize > IMAGE_SIZE_320x240\n");        
    if(jpegDecompression(JpegBitStream + sizeUsed, cYUVRawData) == 0)
        DEBUG_DCF("UI JPEG decode error!!!\n");
    
  
    *uWidth     = sizeX;
    *uHeight    = sizeY;
    
    if(!sysTVOutOnFlag)
    jpegSetImageResolution(*uWidth, *uHeight);
    
#if DINAMICALLY_POWER_MANAGEMENT
        sysJPEG_disable();
#endif        
    return 1;
   
}

