/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	Header.c

Abstract:

 *    H.264 Slice headers

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/23	Lsk	Create	

*/

#include <stdlib.h>
#include "general.h"
#include "vlc.h"
#include "header.h"
#include "ParsetCommon.h"



extern seq_parameter_set_rbsp_t active_sps;
extern pic_parameter_set_rbsp_t active_pps;
SLICE_HEADER_t active_slice_header;

static u8 ref_pic_list_reordering(void);
static u8 dec_ref_pic_marking(Bitstream *currStream, NALU_t *nalu);


/*!
 ************************************************************************
 * \brief
 *    read the first part of the header (only the pic_parameter_set_id)
 * \return
 *    Length of the first part of the slice header (in bits)
 ************************************************************************
 */
int FirstPartOfSliceHeader(void)
{
	u32 pic_parameter_set_id;
	u32 tmp;
	Bitstream *currStream;

	// Get first_mb_in_slice
	active_slice_header.first_mb_in_slice = decode_ue_v ("SH: first_mb_in_slice", currStream);

	tmp = decode_ue_v ("SH: slice_type", currStream);
	if (tmp>4) tmp -=5;
	active_slice_header.slice_type = tmp;

	pic_parameter_set_id = decode_ue_v ("SH: pic_parameter_set_id", currStream);

	#if 0
	DEBUG_H264("first_mb_in_slice = %d\n"	, active_slice_header.first_mb_in_slice);// 0
	DEBUG_H264("slice_type = %d\n"			, active_slice_header.slice_type	   );// 2
	DEBUG_H264("pic_parameter_set_id = %d\n", pic_parameter_set_id			  	   );// 0
	#endif
	return 1;
}

/*!
 ************************************************************************
 * \brief
 *    read the scond part of the header (without the pic_parameter_set_id 
 * \return
 *    Length of the second part of the Slice header in bits
 ************************************************************************
 */
int RestOfSliceHeader(NALU_t *nalu)
{
	Bitstream *currStream;
	u32 val;
	u32 idr_pic_id;
	u8 	err=0;
	
	active_slice_header.frame_num = decode_u_v (active_sps.log2_max_frame_num_minus4 + 4, "SH: frame_num", currStream);

	if (active_sps.frame_mbs_only_flag)
	{
		active_slice_header.field_pic_flag=0;
	}
	else
	{
		DEBUG_H264("H264 encoder IP not support frame_mbs_only_flag\n");	
		err = 1;
	}

	
	if (nalu->nal_unit_type == 5)//Lsk TODO
	{
		idr_pic_id = decode_ue_v("SH: idr_pic_id", currStream);
	}

	if (active_sps.pic_order_cnt_type == 0)
	{
		DEBUG_H264("H264 encoder IP not support. pic_order_cnt_type == 0\n");
		err = 1;		
	}	
	if( active_sps.pic_order_cnt_type == 1 && !active_sps.delta_pic_order_always_zero_flag ) 
	{
		DEBUG_H264("H264 encoder IP not support. pic_order_cnt_type == 1\n");
		err = 1;		
	}	

	
	if (active_pps.redundant_pic_cnt_present_flag)
	{
		DEBUG_H264("H264 encoder IP not support. redundant_pic_cnt_present_flag\n");
		err = 1;		
	}

	if(active_slice_header.slice_type == B_SLICE)
  	{
  		DEBUG_H264("H264 encoder IP not support. redundant_pic_cnt_present_flag\n");
		err = 1;		
  	}

	if(active_slice_header.slice_type == P_SLICE)// SP_SLICE || B_SLICE
	{
		val = decode_u_1 ("SH: num_ref_idx_override_flag", currStream);
		if(val)
		{
			DEBUG_H264("H264 encoder IP not support. num_ref_idx_override_flag\n");
			err = 1;
		}
	}
	#if 0
	DEBUG_H264("frame_num = %d\n"				 , active_slice_header.frame_num);// 0
	DEBUG_H264("idr_pic_id = %d\n"				 , idr_pic_id				    );// 0
	DEBUG_H264("num_ref_idx_override_flag = %d\n", val			  	   			);// 0
	#endif
	ref_pic_list_reordering();

	if ((active_pps.weighted_pred_flag&&(active_slice_header.slice_type==P_SLICE||active_slice_header.slice_type==SP_SLICE))
		||(active_pps.weighted_bipred_idc==1 && (active_slice_header.slice_type==B_SLICE)))
	{
		DEBUG_H264("H264 encoder IP not support pred_weight_table\n");
		err = 1;
	}

	if (nalu->nal_reference_idc)//Lsk TODO
	    dec_ref_pic_marking(currStream, nalu);

	if (active_pps.entropy_coding_mode_flag && active_slice_header.slice_type!=I_SLICE && active_slice_header.slice_type!=SI_SLICE)
	{
		DEBUG_H264("H264 encoder IP not support cabac_init_idc\n");
		err = 1;
	}
	
	val = decode_se_v("SH: slice_qp_delta", currStream);
	active_slice_header.slice_qp_delta = val;

	if(active_slice_header.slice_type==SP_SLICE || active_slice_header.slice_type == SI_SLICE) 
	{
		DEBUG_H264("H264 encoder IP not support SP_SLICE/SI_SLICE\n");
		err = 1;
	}

    #if H1_264TEST
	if (active_pps.deblocking_filter_control_present_flag)
	{
        active_slice_header.LFDisableIdc = decode_ue_v ("SH: disable_deblocking_filter_idc", currStream);
        if (active_slice_header.LFDisableIdc!=1)
        {
          active_slice_header.LFAlphaC0Offset = decode_se_v("SH: slice_alpha_c0_offset_div2", currStream);
          active_slice_header.LFBetaOffset = decode_se_v("SH: slice_beta_offset_div2", currStream);
        }
        else
        {
          active_slice_header.LFAlphaC0Offset = active_slice_header.LFBetaOffset = 0;
        }
	}
    else
    {
          active_slice_header.LFDisableIdc = active_slice_header.LFAlphaC0Offset = active_slice_header.LFBetaOffset = 0;
    }
    #else
	if (active_pps.deblocking_filter_control_present_flag)
	{
		DEBUG_H264("H264 encoder IP not support. deblocking_filter_control_present_flag\n");
		err = 1;		
	}
    #endif
	
	if (active_pps.num_slice_groups_minus1>0)// && active_pps.slice_group_map_type>=3 && active_pps.slice_group_map_type<=5)
	{
		DEBUG_H264("H264 encoder IP not support. num_slice_groups_minus1\n");
		err = 1;		
	}

	#if 0
	DEBUG_H264("first_mb_in_slice = %d\n"	, active_slice_header.first_mb_in_slice     );
	DEBUG_H264("slice_type = %d\n"			, active_slice_header.slice_type	        );
	DEBUG_H264("pic_parameter_set_id = %d\n", active_slice_header.pic_parameter_set_id  );
	DEBUG_H264("frame_num = %d\n"           , active_slice_header.frame_num             );
	DEBUG_H264("slice_qp_delta = %d\n"      , active_slice_header.slice_qp_delta        );
    #if H1_264TEST
    if (active_pps.deblocking_filter_control_present_flag)
	{
    	DEBUG_H264("LFDisableIdc = %d\n"	, active_slice_header.LFDisableIdc	        );
    	DEBUG_H264("LFAlphaC0Offset = %d\n"	, active_slice_header.LFAlphaC0Offset	    );
    	DEBUG_H264("LFBetaOffset = %d\n"    , active_slice_header.LFBetaOffset          );
	}
    #endif
	#endif

	return err;
}


/*!
 ************************************************************************
 * \brief
 *    read the reference picture reordering information
 ************************************************************************
 */
static u8 ref_pic_list_reordering(void)
{
	u32 val;
	u8 err=0;
	Bitstream *currStream;


	if (active_slice_header.slice_type!=I_SLICE && active_slice_header.slice_type!=SI_SLICE)
	{
		val = decode_u_1 ("SH: ref_pic_list_reordering_flag_l0", currStream);           
		if(val)
		{
			DEBUG_H264("H264 encoder IP not support ref_pic_list_reordering_flag_l0\n");	
			err = 1;
		}
	}

	if (active_slice_header.slice_type==B_SLICE)
	{
		DEBUG_H264("H264 encoder IP not support B_SLICE\n");	
		err = 1;	
	}
	#if 0
	DEBUG_H264("ref_pic_list_reordering_flag_l0 = %d\n", val);// 0	
	#endif
	return err;
}




/*!
 ************************************************************************
 * \brief
 *    read the memory control operations
 ************************************************************************
 */
static u8 dec_ref_pic_marking(Bitstream *currStream, NALU_t *nalu)
{
	u32 no_output_of_prior_pics_flag;
	u32 long_term_reference_flag;
	u32 adaptive_ref_pic_buffering_flag;
	u8 err=0;

	if (nalu->nal_unit_type==5)
	{
		no_output_of_prior_pics_flag = decode_u_1("SH: no_output_of_prior_pics_flag", currStream);
		long_term_reference_flag = decode_u_1("SH: long_term_reference_flag", currStream);
	}
	else
	{
		adaptive_ref_pic_buffering_flag = decode_u_1("SH: adaptive_ref_pic_buffering_flag", currStream);
		if (adaptive_ref_pic_buffering_flag)
		{
			// read Memory Management Control Operation 
			DEBUG_H264("H264 encoder IP not support. adaptive_ref_pic_buffering_flag\n"); 
			err = 1;
		}
	}
	#if 0
	DEBUG_H264("no_output_of_prior_pics_flag = %d\n"   , no_output_of_prior_pics_flag	);// 0	
	DEBUG_H264("long_term_reference_flag = %d\n"	   , long_term_reference_flag		);// 0	
	DEBUG_H264("adaptive_ref_pic_buffering_flag = %d\n", adaptive_ref_pic_buffering_flag);// 0	
	#endif
	return err;
}


