
/*!
 *************************************************************************************
 * \file header.h
 * 
 * \brief
 *    Prototypes for header.c
 *************************************************************************************
 */

#ifndef _HEADER_H_
#define _HEADER_H_

#include "NALUCommon.h"


typedef enum {
  P_SLICE = 0,
  B_SLICE,
  I_SLICE,
  SP_SLICE,
  SI_SLICE
} SliceType;

typedef struct 
{
  u32 first_mb_in_slice;          
  u32 slice_type;
  u32 frame_num;
  u32 slice_qp_delta;
  u32 field_pic_flag;
  u32 bottom_field_flag;

  #if H1_264TEST
  u32 LFDisableIdc;     //!< Disable loop filter on slice
  u32 LFAlphaC0Offset;  //!< Alpha and C0 offset for filtering slice
  u32 LFBetaOffset;     //!< Beta offset for filtering slice
  u32 pic_parameter_set_id;
  #endif
} SLICE_HEADER_t; //only for sps,pps

extern int FirstPartOfSliceHeader(void);
extern int RestOfSliceHeader(NALU_t *nalu);



#endif

