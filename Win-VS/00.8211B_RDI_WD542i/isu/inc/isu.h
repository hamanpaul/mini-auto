/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	ipu.h

Abstract:

   	The declarations of Image Processing Unit.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __ISU_H__
#define __ISU_H__

#include    "isureg.h"




// convert register setting
typedef struct _ISU_CRS {
	u16		stepw;
	u16		steph;
	u16		phasew;
	u16		phaseh;
} ISU_CRS;




#if ISU_OVERLAY_ENABLE
  #if ((ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95) || (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_25) || (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DR900))
    #include "ascii_font_20x20x95.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95)
    #include "ascii_font_32x40x95.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x48x95)
    #include "ascii_font_32x48x95.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x36x95)
    #include "ascii_font_20x36x95.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_1BIT)
	#include "ascii_font_32x40x95_1bit.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_2BIT)
    #include "ascii_font_32x40x95_2bit.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT)
    #include "ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT)
    #include "ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT_DR900)
    #include "ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT_WITH_LIGHT)
    #include "ascii_font_20x20x95_2bit_with_light.h"    
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA)
    #include "ascii_font_64x40x95_2bit_with_antenna.h"      
  #elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_16x22x95_COMMAX_2BIT)
    #include "../isu/inc/ascii_font_16x22x95_2bit.h"
  #else
    #include "../isu/inc/ascii_font_8x16x95_2bit.h"
  #endif

  #if ((ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95) || (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_25) || (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_DR900))
    #include "ascii_font_20x20x95.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x40x95)
    #include "ascii_font_32x40x95.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x48x95)
    #include "ascii_font_32x48x95.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x36x95)
    #include "ascii_font_20x36x95.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x40x95_1BIT)
	#include "ascii_font_32x40x95_1bit.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x40x95_2BIT)
    #include "ascii_font_32x40x95_2bit.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT)
    #include "ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT)
    #include "ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_DOUBLE_FIELD_2BIT_DR900)
    #include "ascii_font_20x20x95_2bit.h"
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x20x95_2BIT_WITH_LIGHT)
    #include "ascii_font_20x20x95_2bit_with_light.h"    
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA)
    #include "ascii_font_64x40x95_2bit_with_antenna.h"      
  #elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_16x22x95_COMMAX_2BIT)
    #include "../isu/inc/ascii_font_16x22x95_2bit.h"     
  #else)
    #include "../isu/inc/ascii_font_8x16x95_2bit.h"
  #endif

  //Lsk 090707 : QVGA
  #if(ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_12x12x95)
    #include "ascii_font_12x12x95.h"  
  #elif(ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_12x12x95_2BIT)  
    #include "ascii_font_12x12x95_2bit.h"  
  #elif((ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_8x16x95_2BIT)  || (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_8x16x95_RDI_2BIT))
    #include "../isu/inc/ascii_font_8x16x95_2bit.h"  
  #endif
  
#endif

#endif
