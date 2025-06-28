#ifndef __ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA_H__
#define __ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA_H__

#if (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA)
//#define ASCII_Font  ascii_font_64x40x95_2bit_with_antenna
extern  const u32   ASCII_Font[95][160];
#endif

#if (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA)
//#define ASCII_XFont ascii_font_64x40x95_2bit_with_antenna
#define ASCII_XFont ASCII_Font
extern  const u32   ASCII_XFont[95][160];
#endif

#if (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA)
//#define ASCII_SFont ascii_font_64x40x95_2bit_with_antenna
#define ASCII_SFont ASCII_Font
extern  const u32   ASCII_SFont[95][160];
#endif

#endif

