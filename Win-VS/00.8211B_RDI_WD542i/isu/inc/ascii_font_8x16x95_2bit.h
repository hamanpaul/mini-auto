#ifndef __ASCII_FONT_8x16x95_2BIT_H__
#define __ASCII_FONT_8x16x95_2BIT_H__

#if (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_8x16x95_2BIT)
#define ASCII_Font  ascii_font_8x16x95_2bit
extern  const u32   ASCII_Font[95][8];
#elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_16x20x95_2BIT)
#define ASCII_Font  ASCII_Font_16x20_2_bit
extern  const u32   ASCII_Font[95][20];
#elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_20x24x95_2BIT)
#define ASCII_Font  ASCII_Font_20x24_2_bit
extern  const u32   ASCII_Font[95][30];
#elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x28x95_2BIT)
#define ASCII_Font  ASCII_Font_32x28_2_bit
extern  const u32   ASCII_Font[95][56];
#elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_12x16x95_2BIT)
#define ASCII_Font  ASCII_Font_12x16_2_bit
extern  const u32   ASCII_Font[95][12];
#elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_32x36x95_RDI_2BIT)
#define ASCII_Font  ASCII_Font32x36x95_RDI
extern  const u32   ASCII_Font[95][72];
#elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_16x20x95_RDI_2BIT)
#define ASCII_Font  ASCII_Font_16x20_2_bit_RDI
extern  const u32   ASCII_Font[95][20];
#elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_8x16x95_RDI_2BIT)
#define ASCII_Font  ascii_font_8x16x95_2bit_RDI
extern  const u32   ASCII_Font[95][8];
#elif (ISU_OVERLAY_LARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA2)
#define ASCII_Font  ascii_font_64x40x95_2bit_with_antenna2
extern  const u32   ASCII_Font[95][160];
#endif

#if (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_8x16x95_2BIT)
#define ASCII_XFont ascii_font_8x16x95_2bit
extern  const u32   ASCII_XFont[95][8];
#elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_16x20x95_2BIT)
#define ASCII_XFont ASCII_Font_16x20_2_bit
extern  const u32   ASCII_XFont[95][20];
#elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_20x24x95_2BIT)
#define ASCII_XFont ASCII_Font_20x24_2_bit
extern  const u32   ASCII_XFont[95][30];
#elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x28x95_2BIT)
#define ASCII_XFont ASCII_Font_32x28_2_bit
extern  const u32   ASCII_XFont[95][56];
#elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_12x16x95_2BIT)
#define ASCII_XFont ASCII_Font_12x16_2_bit
extern  const u32   ASCII_XFont[95][12];
#elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_32x36x95_RDI_2BIT)
#define ASCII_XFont ASCII_Font32x36x95_RDI
extern  const u32   ASCII_XFont[95][72];
#elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_16x20x95_RDI_2BIT)
#define ASCII_XFont ASCII_Font_16x20_2_bit_RDI
extern  const u32   ASCII_XFont[95][20];
#elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_8x16x95_RDI_2BIT)
#define ASCII_XFont ascii_font_8x16x95_2bit_RDI
extern  const u32   ASCII_XFont[95][8];
#elif (ISU_OVERLAY_XLARGE_FONT_TYPE == ASCII_FONT_64x40x95_2BIT_WITH_ANTENNA2)
#define ASCII_XFont ascii_font_64x40x95_2bit_with_antenna2
extern  const u32   ASCII_XFont[95][160];
#endif


#if (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_8x16x95_2BIT)
#define ASCII_SFont ascii_font_8x16x95_2bit
extern  const u32   ASCII_SFont[95][8];
#elif (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_16x20x95_2BIT)
#define ASCII_SFont ASCII_Font_16x20_2_bit
extern  const u32   ASCII_SFont[95][20];
#elif (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_20x24x95_2BIT)
#define ASCII_SFont ASCII_Font_20x24_2_bit
extern  const u32   ASCII_SFont[95][30];
#elif (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_32x28x95_2BIT)
#define ASCII_SFont ASCII_Font_32x28_2_bit
extern  const u32   ASCII_SFont[95][56];
#elif (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_12x16x95_2BIT)
#define ASCII_SFont ASCII_Font_12x16_2_bit
extern  const u32   ASCII_SFont[95][12];
#elif (ISU_OVERLAY_SMALL_FONT_TYPE == ASCII_FONT_8x16x95_RDI_2BIT)
#define ASCII_SFont ascii_font_8x16x95_2bit_RDI
extern  const u32   ASCII_SFont[95][8];
#endif


#endif

