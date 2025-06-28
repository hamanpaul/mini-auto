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
    #if (H1_264TEST || H1_264TEST_ENC)
    u8  second_chroma_qp_index_offset;                      // se(v)
    #endif

    u8  deblocking_filter_control_present_flag;             // u(1)
    u8  constrained_intra_pred_flag;                        // u(1)
    u8  redundant_pic_cnt_present_flag;                     // u(1)

    // if( more_rbsp_data() )
    u8  transform_8x8_mode_flag;                            // u(1)
    u8  pic_scaling_matrix_present_flag;                    // u(1)
    // if (pic_scaling_matrix_present_flag)
    u32 pic_scaling_list_present_flag[8];                   // u(1)
} pic_parameter_set_rbsp_t;

#if (H1_264TEST || H1_264TEST_ENC)

#define MAXIMUMVALUEOFcpb_cnt   32

typedef struct
{
  unsigned  cpb_cnt_minus1;                                   // ue(v)
  unsigned  bit_rate_scale;                                   // u(4)
  unsigned  cpb_size_scale;                                   // u(4)
    unsigned  bit_rate_value_minus1 [MAXIMUMVALUEOFcpb_cnt];  // ue(v)
    unsigned  cpb_size_value_minus1 [MAXIMUMVALUEOFcpb_cnt];  // ue(v)
    unsigned  cbr_flag              [MAXIMUMVALUEOFcpb_cnt];  // u(1)
  unsigned  initial_cpb_removal_delay_length_minus1;          // u(5)
  unsigned  cpb_removal_delay_length_minus1;                  // u(5)
  unsigned  dpb_output_delay_length_minus1;                   // u(5)
  unsigned  time_offset_length;                               // u(5)
} hrd_parameters_t;

typedef struct
{
    u8      aspect_ratio_info_present_flag;                   // u(1)
    u8  aspect_ratio_idc;                               // u(8)
    u8  sar_width;                                    // u(16)
    u8  sar_height;                                   // u(16)
    u8      overscan_info_present_flag;                       // u(1)
    u8      overscan_appropriate_flag;                      // u(1)
    u8      video_signal_type_present_flag;                   // u(1)
    u8  video_format;                                   // u(3)
    u8      video_full_range_flag;                          // u(1)
    u8      colour_description_present_flag;                // u(1)
      u8  colour_primaries;                             // u(8)
      u8  transfer_characteristics;                     // u(8)
      u8  matrix_coefficients;                          // u(8)
    u8      chroma_location_info_present_flag;                // u(1)
    u8   chroma_sample_loc_type_top_field;               // ue(v)
    u8   chroma_sample_loc_type_bottom_field;            // ue(v)
    u8      timing_info_present_flag;                         // u(1)
    u8  num_units_in_tick;                              // u(32)
    u8  time_scale;                                     // u(32)
    u8      fixed_frame_rate_flag;                          // u(1)
    u8      nal_hrd_parameters_present_flag;                  // u(1)
    hrd_parameters_t nal_hrd_parameters;                      // hrd_paramters_t
    u8      vcl_hrd_parameters_present_flag;                  // u(1)
    hrd_parameters_t vcl_hrd_parameters;                      // hrd_paramters_t
  // if ((nal_hrd_parameters_present_flag || (vcl_hrd_parameters_present_flag))
    u8      low_delay_hrd_flag;                             // u(1)
    u8      pic_struct_present_flag;                        // u(1)
    u8      bitstream_restriction_flag;                       // u(1)
    u8      motion_vectors_over_pic_boundaries_flag;        // u(1)
    u8   max_bytes_per_pic_denom;                        // ue(v)
    u8   max_bits_per_mb_denom;                          // ue(v)
    u8   log2_max_mv_length_vertical;                    // ue(v)
    u8   log2_max_mv_length_horizontal;                  // ue(v)
    u8   num_reorder_frames;                             // ue(v)
    u8   max_dec_frame_buffering;                        // ue(v)
} vui_seq_parameters_t;
#endif

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
    #if (H1_264TEST || H1_264TEST_ENC)
    vui_seq_parameters_t vui_seq_parameters;                // vui_seq_parameters_t
    #endif
} seq_parameter_set_rbsp_t;                                                                                                                                                                                         
#endif
