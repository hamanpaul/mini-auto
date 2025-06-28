

/*

Copyright (c) 2005  Himax Technologies, Inc.

Module Name:

    CIU.h

Abstract:

    The structures and constants of CCIR656 Interface.

Environment:

        ARM RealView Developer Suite

Revision History:

    2010/07/22  Lucian Yuan   Create              

*/

#ifndef __CIU_H__
#define __CIU_H__

  #define CIU_OSD_LINEAR_ADDR_MODE   0
  #define CIU_OSD_FRAME_ADDR_MODE    1

  #define  CIU_OSD_MODE_SEL CIU_OSD_FRAME_ADDR_MODE


  #if ((ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95) || (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_25) || (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DR900))
    #include "../isu/inc/ascii_font_20x20x95.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95)
    #include "../isu/inc/ascii_font_32x40x95.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x48x95)
    #include "../isu/inc/ascii_font_32x48x95.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x36x95)
    #include "../isu/inc/ascii_font_20x36x95.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_1BIT)
	#include "../isu/inc/ascii_font_32x40x95_1bit.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_2BIT)
    #include "../isu/inc/ascii_font_32x40x95_2bit.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT)
    #include "../isu/inc/ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT)
    #include "../isu/inc/ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT_DR900)
    #include "../isu/inc/ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA)
    #include "../isu/inc/ascii_font_64x40x95_2bit_with_antenna.h"        
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_16x22x95_COMMAX_2BIT)
    #include "../isu/inc/ascii_font_16x22x95_2bit.h"        
  #else
    #include "../isu/inc/ascii_font_8x16x95_2bit.h"
  #endif

  #if ((ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95) || (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_25) || (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_DR900))
    #include "../isu/inc/ascii_font_20x20x95.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x40x95)
    #include "../isu/inc/ascii_font_32x40x95.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x48x95)
    #include "../isu/inc/ascii_font_32x48x95.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x36x95)
    #include "../isu/inc/ascii_font_20x36x95.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x40x95_1BIT)
	#include "../isu/inc/ascii_font_32x40x95_1bit.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x40x95_2BIT)
    #include "../isu/inc/ascii_font_32x40x95_2bit.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT)
    #include "../isu/inc/ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT)
    #include "../isu/inc/ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT_DR900)
    #include "../isu/inc/ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA)
    #include "../isu/inc/ascii_font_64x40x95_2bit_with_antenna.h"        
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_16x22x95_COMMAX_2BIT)
    #include "../isu/inc/ascii_font_16x22x95_2bit.h"        
  #else
    #include "../isu/inc/ascii_font_8x16x95_2bit.h"
  #endif

  //Lsk 090707 : QVGA
  #if(ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_12x12x95)
    #include "../isu/inc/ascii_font_12x12x95.h"  
  #elif(ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_12x12x95_2BIT)  
    #include "../isu/inc/ascii_font_12x12x95_2bit.h"  
  #elif((ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_8x16x95_2BIT)  || (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_8x16x95_RDI_2BIT))
    #include "../isu/inc/ascii_font_8x16x95_2bit.h"  
  #endif
  

#endif
