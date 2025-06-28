/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	parsetcommon.h

Abstract:

   	Picture and Sequence Parameter Sets, structures common to encoder and decoder.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/02	Lsk	Create	

*/

#ifndef _PARSETCOMMON_H_
#define _PARSETCOMMON_H_

#define MAXnum_slice_groups_minus1  8
#define MAXnum_ref_frames_in_pic_order_cnt_cycle  256

typedef struct
{    
    u8  pic_parameter_set_id;                               // ue(v)
    u8  seq_parameter_set_id;                               // ue(v)
    u8  entropy_coding_mode_flag;                           // u(1)
    u8  pic_order_present_flag;                             // u(1)
    u8  num_slice_groups_minus1;                            // ue(v)

    //TO DO : CCU H264 IP don't support
    /* 
    //if(num_slice_groups_minus1>0)
    u8  slice_group_map_type;                               // ue(v)
    // if( slice_group_map_type = = 0 )
    u8  run_length_minus1[MAXnum_slice_groups_minus1];      // ue(v)
    // else if( slice_group_map_type = = 2 )
    u8  top_left[MAXnum_slice_groups_minus1];               // ue(v)
    u8  bottom_right[MAXnum_slice_groups_minus1];           // ue(v)
    // else if( slice_group_map_type = = 3 || 4 || 5)
    u8  slice_group_change_direction_flag;                  // u(1)
    u8  slice_group_change_rate_minus1;                     // ue(v)
    // else if( slice_group_map_type = = 6 )
    u8  pic_size_in_map_units_minus1;	                    // ue(v)
    u8  *slice_group_id;                                    // complete MBAmap u(v)
    */

    u32 num_ref_idx_l0_active_minus1;                       // ue(v)
    u32 num_ref_idx_l1_active_minus1;                       // ue(v)
    u8  weighted_pred_flag;                                 // u(1)
    u8  weighted_bipred_idc;                                // u(2)
    u8  pic_init_qp_minus26;                                // se(v)
    u8  pic_init_qs_minus26;                                // se(v)
    u8  chroma_qp_index_offset;                             // se(v)


    u8  deblocking_filter_control_present_flag;             // u(1)
    u8  constrained_intra_pred_flag;                        // u(1)
    u8  redundant_pic_cnt_present_flag;                     // u(1)

    // if( more_rbsp_data() )
    u8  transform_8x8_mode_flag;                            // u(1)
    u8  pic_scaling_matrix_present_flag;                    // u(1)
    // if (pic_scaling_matrix_present_flag)
    u32 pic_scaling_list_present_flag[8];                   // u(1)
} pic_parameter_set_rbsp_t;

typedef struct                                                                                            
{                                                                                                                                                                                                                                                 
    u8    profile_idc;                                      // u(8)                                  
    u8    constrained_set0_flag;                            // u(1)                                     
    u8    constrained_set1_flag;                            // u(1)                                     
    u8    constrained_set2_flag;                            // u(1)                                     
    u8    constrained_set3_flag;                            // u(1)                                     
    u8    level_idc;                                        // u(8)                                 
    u8    seq_parameter_set_id;                             // ue(v)                                
    u8    chroma_format_idc;                                // ue(v)                                

    u8   bit_depth_luma_minus8;                             // ue(v)                                 
    u8   bit_depth_chroma_minus8;                           // ue(v)                                 
                                                                                                            
    u8    seq_scaling_matrix_present_flag;                  // u(1)                                    
    u8    seq_scaling_list_present_flag[12];                // u(1)                                    
                                                                                                          
    u8    log2_max_frame_num_minus4;                        // ue(v)                                 
    u8    pic_order_cnt_type;                                                                        
    // if( pic_order_cnt_type == 0 )                                                                        
    u8    log2_max_pic_order_cnt_lsb_minus4;                // ue(v)                                
    // else if( pic_order_cnt_type == 1 )                                                                   
    u8    delta_pic_order_always_zero_flag;                 // u(1)                                         
    u8    offset_for_non_ref_pic;                           // se(v)                                        
    u8    offset_for_top_to_bottom_field;                   // se(v)                                        
    u8    num_ref_frames_in_pic_order_cnt_cycle;            // ue(v)                                   
    // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )                                         
    u8    offset_for_ref_frame[MAXnum_ref_frames_in_pic_order_cnt_cycle];   // se(v)                        

    u8    num_ref_frames;                                   // ue(v)                                
    u8    gaps_in_frame_num_value_allowed_flag;             // u(1)                                     
    u32   pic_width_in_mbs_minus1;                          // ue(v)                                 
    u32   pic_height_in_map_units_minus1;                   // ue(v)                                 
    u8    frame_mbs_only_flag;                              // u(1)                                     
    // if( !frame_mbs_only_flag )                                                                           
    u8    mb_adaptive_frame_field_flag;                     // u(1)                                       
    u8    direct_8x8_inference_flag;                        // u(1)                                     
    u8    frame_cropping_flag;                              // u(1)                                     
    // if(frame_cropping_flag)                                                                           
    u32   frame_cropping_rect_left_offset;                  // ue(v)                                   
    u32   frame_cropping_rect_right_offset;                 // ue(v)                                   
    u32   frame_cropping_rect_top_offset;                   // ue(v)                                   
    u32   frame_cropping_rect_bottom_offset;                // ue(v)                                   
    u8    vui_parameters_present_flag;                      // u(1)                                         
} seq_parameter_set_rbsp_t;                                                                                                                                                                                         
#endif
