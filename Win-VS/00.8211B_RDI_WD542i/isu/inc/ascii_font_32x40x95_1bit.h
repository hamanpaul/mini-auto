#ifndef __ASCII_FONT_32x40x95_1BIT_H__
#define __ASCII_FONT_32x40x95_1BIT_H__

#if (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x40x95_1BIT)
#define ASCII_Font  ascii_font_32x40x95_1bit
extern  const u32   ASCII_Font[95][40];
#endif

#if (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x40x95_1BIT)
#define ASCII_XFont ascii_font_32x40x95_1bit
extern  const u32   ASCII_XFont[95][40];
#endif

#if (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_32x40x95_1BIT)
#define ASCII_SFont ascii_font_32x40x95_1bit
extern  const u32   ASCII_SFont[95][40];
#endif

#endif

