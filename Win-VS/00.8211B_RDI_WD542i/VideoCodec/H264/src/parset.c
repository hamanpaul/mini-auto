/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	parsetcommon.h

Abstract:

   	Picture and Sequence Parameter set generation and handling.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/02	Lsk	Create	

*/

#include <stdlib.h> 
#include "general.h"
#include "parsetcommon.h"
#include "parset.h"
#include "NAL.h"
#include "NALU.h"
#include "NALUcommon.h"
#include "vlc.h"
#include "H264.h"
#include "H264api.h"
#include "board.h"
#include "iduapi.h"
#include "../idu/inc/idureg.h"

/*
*********************************************************************************************************
* Constant
*********************************************************************************************************
*/

#define BASELINE_PROFILE        66
#define FALSE                    0
#define TRUE                     1


/*
*********************************************************************************************************
* Variable
*********************************************************************************************************
*/

seq_parameter_set_rbsp_t active_sps;
pic_parameter_set_rbsp_t active_pps;
Bitstream datastream;               //SPS,PPS
NALU_t SPS_NALU, PPS_NALU;


extern u8 TvOutMode;
/*
*********************************************************************************************************
* Function prototype
*********************************************************************************************************
*/
void GenerateSequenceParameterSet(H264_ENC_CFG *, seq_parameter_set_rbsp_t *) ;
int GenerateSeq_parameter_set_rbsp (seq_parameter_set_rbsp_t *, unsigned char *);
int GenerateSeq_parameter_set_NALU (NALU_t *);

void GeneratePictureParameterSet(pic_parameter_set_rbsp_t *, seq_parameter_set_rbsp_t *);
int GeneratePic_parameter_set_rbsp (pic_parameter_set_rbsp_t *, unsigned char *);
int GeneratePic_parameter_set_NALU(NALU_t *);
//////////////////////////////////////////////////////////
//
// H264 encoder functions
//
//////////////////////////////////////////////////////////

/*! 
 *************************************************************************************
 * \brief
 *    generates a sequence and picture parameter set and stores these in global
 *    active_sps and active_pps
 * \note
 *
 * \return
 *    A NALU containing the Sequence ParameterSet
 *
 *************************************************************************************
*/
int GenerateParameterSets_SW (H264_ENC_CFG* cfg, u8 *buffer)
{ 

  int length;  
  NALU_t n;

  n.buf = buffer;
  GenerateSequenceParameterSet(cfg, &active_sps);
  length = GenerateSeq_parameter_set_NALU(&n);

  /*	
  memcpy(buffer, n.buf, length);
  pos = length;
  printf("sps length = %d\n", length);
  for(i=0; i<length; i++)
    printf("%02x ",buffer[i]);
  printf("\n");
  */

  n.buf = (buffer+length);
  GeneratePictureParameterSet (&active_pps, &active_sps);
  length += GeneratePic_parameter_set_NALU(&n);
  /*	
  memcpy(buffer+pos, n.buf, length);
  printf("pps length = %d\n", length);
  for(i=0; i<length; i++)
    printf("%02x ",*(buffer+pos+i));  
  printf("\n");  
  pos += length;  	
  return pos;
  */
  return length;
}
/*! 
 *************************************************************************************
 * \brief
 *    generates a sequence and picture parameter set and stores these in global
 *    active_sps and active_pps
 * \note
 *
 * \return
 *    A NALU containing the Sequence ParameterSet
 *
 *************************************************************************************
*/
int Rfiu_GenerateParameterSets_SW (H264_ENC_CFG* cfg, u8 *buffer,NALU_t *n)
{ 

  int length;  


  n->buf = buffer;
  GenerateSequenceParameterSet(cfg, &active_sps);
  length = GenerateSeq_parameter_set_NALU(n);

  /*	
  memcpy(buffer, n.buf, length);
  pos = length;
  printf("sps length = %d\n", length);
  for(i=0; i<length; i++)
    printf("%02x ",buffer[i]);
  printf("\n");
  */

  n->buf = (buffer+length);
  GeneratePictureParameterSet (&active_pps, &active_sps);
  length += GeneratePic_parameter_set_NALU(n);
  /*	
  memcpy(buffer+pos, n.buf, length);
  printf("pps length = %d\n", length);
  for(i=0; i<length; i++)
    printf("%02x ",*(buffer+pos+i));  
  printf("\n");  
  pos += length;  	
  return pos;
  */
  return length;
}

/*! 
*************************************************************************************
* \brief
*    int GenerateSeq_parameter_set_NALU (void);
*
* \note
*    Uses the global variables through GenerateSequenceParameterSet()
*    and GeneratePictureParameterSet
*
* \return
*    A NALU containing the Sequence ParameterSet
*
*************************************************************************************
*/
int GenerateSeq_parameter_set_NALU (NALU_t *n)
{  
  int RBSPlen = 0;
  int NALUlen;
  u8 rbsp[LENGTH_OF_SPS_PPS_NALU];

  RBSPlen = GenerateSeq_parameter_set_rbsp (&active_sps, rbsp);
  NALUlen = RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_SPS, NALU_PRIORITY_HIGHEST);
  return NALUlen;
}


/*! 
*************************************************************************************
* \brief
*    NALU_t *GeneratePic_parameter_set_NALU (int PPS_id);
*
* \note
*    Uses the global variables through GenerateSequenceParameterSet()
*    and GeneratePictureParameterSet
*
* \return
*    A NALU containing the Picture Parameter Set
*
*************************************************************************************
*/
int GeneratePic_parameter_set_NALU(NALU_t *n)
{
  int RBSPlen = 0;
  int NALUlen;
  u8 rbsp[LENGTH_OF_SPS_PPS_NALU];

  RBSPlen = GeneratePic_parameter_set_rbsp (&active_pps, rbsp);
  NALUlen = RBSPtoNALU (rbsp, n, RBSPlen, NALU_TYPE_PPS, NALU_PRIORITY_HIGHEST);
  return NALUlen;
}

/*!
 ************************************************************************
 * \brief
 *    GenerateSequenceParameterSet: extracts info from global variables and
 *    generates sequence parameter set structure
 *
 * \par
 *    Function reads all kinds of values from several global variables,
 *    including input-> and image-> and fills in the sps.  Many
 *    values are current hard-coded to defaults.
 *
 * \Note 
 *    Update level_idc, pic_width_in_mbs_minus1,
 *    pic_height_in_map_units_minus1 from cfg      
 ************************************************************************
 */
void GenerateSequenceParameterSet(H264_ENC_CFG* cfg,seq_parameter_set_rbsp_t *sps) 
{
    u32 i;    
    
    sps->profile_idc = BASELINE_PROFILE; //CCU H264 IP only support baseline profile
    sps->constrained_set0_flag = 0;
    sps->constrained_set1_flag = 0;
    sps->constrained_set2_flag = 0;
    sps->constrained_set3_flag = 0;
    sps->level_idc    = cfg->level_idc;
    sps->seq_parameter_set_id = 0;  // Parameter Set ID hard coded to zero

    if( (sps->profile_idc == 100)||(sps->profile_idc == 110)||(sps->profile_idc == 122) || (sps->profile_idc == 144) )
    {
        //TO DO : CCU H264 IP don't support
        /*
        sps->chroma_format_idc 
        sps->bit_depth_luma_minus8
        sps->bit_depth_chroma_minus8
        sps->seq_scaling_matrix_present_flag = 0;
        for(i=0; i<8; i++)
            sps->seq_scaling_list_present_flag[i] = 0;
        */
    }
        
    sps->log2_max_frame_num_minus4 = 0;             
    sps->pic_order_cnt_type = 2;                    
    
    if(sps->pic_order_cnt_type == 0)
        sps->log2_max_pic_order_cnt_lsb_minus4 = 0; 
    else if(sps->pic_order_cnt_type == 1)
    {
        sps->delta_pic_order_always_zero_flag = 0;  
        sps->offset_for_non_ref_pic = 0;            
        sps->offset_for_top_to_bottom_field = 0;        
        sps->num_ref_frames_in_pic_order_cnt_cycle = 0; 
        for (i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            sps->offset_for_ref_frame[i] = 0;
        }        
    }
        
    sps->num_ref_frames = 1; 
    sps->gaps_in_frame_num_value_allowed_flag = 0;
    sps->pic_width_in_mbs_minus1 = (cfg->pic_width_in_mbs-1);
    sps->pic_height_in_map_units_minus1 = (cfg->pic_height_in_map_unit-1);;
    sps->frame_mbs_only_flag = 1;

    if(!sps->frame_mbs_only_flag)
        sps->mb_adaptive_frame_field_flag = 0;
    
    sps->direct_8x8_inference_flag = 1;
    sps->frame_cropping_flag = FALSE;

    if(sps->frame_cropping_flag == TRUE)        
    {
        sps->frame_cropping_rect_left_offset = 0;
        sps->frame_cropping_rect_top_offset = 0;
        sps->frame_cropping_rect_right_offset = 0;
        sps->frame_cropping_rect_bottom_offset = 0;        
    }
    
    sps->vui_parameters_present_flag = FALSE;
    if(sps->vui_parameters_present_flag == TRUE)
    {
        //TO DO : CCU H264 IP don't support
    }

}   
/*!
 ************************************************************************
 * \brief
 *    GeneratePictureParameterSet: 
 *    Generates a Picture Parameter Set structure
 *
 * \par
 *    Regarding the QP
 *    The previous software versions coded the absolute QP only in the 
 *    slice header.  This is kept, and the offset in the PPS is coded 
 *    even if we could save bits by intelligently using this field.
 *
 * \Note 
 *    Update seq_parameter_set_id, num_ref_idx_l0_active_minus1,
 *    num_ref_idx_l1_active_minus1 from pps      
 ************************************************************************
 */
//Lsk : all value must check with ccu
void GeneratePictureParameterSet( pic_parameter_set_rbsp_t *pps, seq_parameter_set_rbsp_t *sps)
{
    pps->pic_parameter_set_id = 0;
    pps->seq_parameter_set_id = sps->seq_parameter_set_id;
    pps->entropy_coding_mode_flag = 0; //CAVLC, CABAC
    pps->pic_order_present_flag = 0;
    pps->num_slice_groups_minus1 = 0;

    if (pps->num_slice_groups_minus1 > 0)
    {
        //TO DO : CCU H264 IP don't support
        /*        
        pps->slice_group_map_type = 0;

        switch (pps->slice_group_map_type)
        {
        case 0:        		
            for(i=0; i<=pps->num_slice_groups_minus1; i++)
            {
            pps->run_length_minus1[i]=0;
            }        		
            break;
        case 1:            
            break;
        case 2:
            for(i=0; i<pps->num_slice_groups_minus1; i++)
            {
                pps->top_left[i] = 0;
                pps->bottom_right[i] = 0;      
            }
            break;
        case 3:
        case 4:
        case 5:
            pps->slice_group_change_direction_flag = 0;
            pps->slice_group_change_rate_minus1 = 0;
            break;
        case 6:
            pps->pic_size_in_map_units_minus1 = 0;
            for (i=0;i<=pps->pic_size_in_map_units_minus1; i++)
                pps->slice_group_id[i] = 0;
            break;
        default:
        }
        */
    }

    pps->num_ref_idx_l0_active_minus1 = sps->frame_mbs_only_flag ? (sps->num_ref_frames-1) : (2 * sps->num_ref_frames - 1) ;   // set defaults
    pps->num_ref_idx_l1_active_minus1 = sps->frame_mbs_only_flag ? (sps->num_ref_frames-1) : (2 * sps->num_ref_frames - 1) ;   // set defaults
    pps->weighted_pred_flag = 0;
    pps->weighted_bipred_idc = 0;
    pps->pic_init_qp_minus26 = 0;         // hard coded to zero, QP lives in the slice header
    pps->pic_init_qs_minus26 = 0;
    pps->chroma_qp_index_offset = 0; 
    pps->deblocking_filter_control_present_flag = 0; //Lsk note
    pps->constrained_intra_pred_flag = 0;
    pps->redundant_pic_cnt_present_flag = 0;

    /*    
    //if( more_rbsp_data() )
    pps->transform_8x8_mode_flag = 0;
    pps->pic_scaling_matrix_present_flag = 0;
    for(i=0; i<8; i++)
        pps->pic_scaling_list_present_flag[i] = 0;
    */
};


/*! 
 *************************************************************************************
 * \brief
 *    int GenerateSeq_parameter_set_rbsp (seq_parameter_set_rbsp_t *sps, char *rbsp);
 *
 * \param sps
 *    sequence parameter structure
 * \param rbsp
 *    buffer to be filled with the rbsp, size should be at least MAXIMUMPARSETRBSPSIZE
 *
 * \return
 *    size of the RBSP in bytes
 *
 * \note
 *    Sequence Parameter VUI function is called, but the function implements
 *    an exit (-1)
 *************************************************************************************
 */
int GenerateSeq_parameter_set_rbsp (seq_parameter_set_rbsp_t *sps, u8 *rbsp)
{

    int len = 0, LenInBytes;
    unsigned i;
    Bitstream *  bitstream = &datastream;


    bitstream->byte_pos = 0;
    bitstream->bits_to_go = 8;
    bitstream->byte_buf = 0x00;
    bitstream->streamBuffer = rbsp;

    len+=encode_u_v  (8, sps->profile_idc, bitstream);   
    len+=encode_u_1  (sps->constrained_set0_flag, bitstream);  
    len+=encode_u_1  (sps->constrained_set1_flag, bitstream);
    len+=encode_u_1  (sps->constrained_set2_flag, bitstream);
    len+=encode_u_1  (sps->constrained_set3_flag, bitstream);
    len+=encode_u_v  (4, 0, bitstream);
    len+=encode_u_v  (8, sps->level_idc, bitstream);
    len+=encode_ue_v (sps->seq_parameter_set_id, bitstream);

    if( (sps->profile_idc == 100)||(sps->profile_idc == 110)||(sps->profile_idc == 122) || (sps->profile_idc == 144) )
    {
        //TO DO : CCU H264 IP don't support
    }
    len+=encode_ue_v (sps->log2_max_frame_num_minus4, bitstream);
    len+=encode_ue_v (sps->pic_order_cnt_type, bitstream);

    if (sps->pic_order_cnt_type == 0)
        len+=encode_ue_v (sps->log2_max_pic_order_cnt_lsb_minus4, bitstream);
    else if (sps->pic_order_cnt_type == 1)
    {
        len+=encode_u_1  (sps->delta_pic_order_always_zero_flag, bitstream);
        len+=encode_se_v (sps->offset_for_non_ref_pic, bitstream);
        len+=encode_se_v (sps->offset_for_top_to_bottom_field, bitstream);
        len+=encode_ue_v (sps->num_ref_frames_in_pic_order_cnt_cycle, bitstream);
        for (i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
            len+=encode_se_v (sps->offset_for_ref_frame[i], bitstream);
    }
  
    len+=encode_ue_v (sps->num_ref_frames, bitstream);
    len+=encode_u_1  (sps->gaps_in_frame_num_value_allowed_flag, bitstream);
    len+=encode_ue_v (sps->pic_width_in_mbs_minus1, bitstream);
    len+=encode_ue_v (sps->pic_height_in_map_units_minus1, bitstream);
    len+=encode_u_1  (sps->frame_mbs_only_flag, bitstream);
    if (!sps->frame_mbs_only_flag)
    {
        len+=encode_u_1  (sps->mb_adaptive_frame_field_flag, bitstream);
    }
    len+=encode_u_1  (sps->direct_8x8_inference_flag, bitstream);
    len+=encode_u_1  (sps->frame_cropping_flag, bitstream);
    if (sps->frame_cropping_flag)
    {
        len+=encode_ue_v (sps->frame_cropping_rect_left_offset, bitstream);
        len+=encode_ue_v (sps->frame_cropping_rect_right_offset, bitstream);
        len+=encode_ue_v (sps->frame_cropping_rect_top_offset, bitstream);
        len+=encode_ue_v (sps->frame_cropping_rect_bottom_offset, bitstream);
    }

    len+=encode_u_1  (sps->vui_parameters_present_flag, bitstream);

    if(sps->vui_parameters_present_flag == TRUE)
    {
        //TO DO : CCU H264 IP don't support
    }

    SODBtoRBSP(bitstream);     // copies the last couple of bits into the byte buffer

    LenInBytes=bitstream->byte_pos;
    return LenInBytes;
}

/*! 
 ***********************************************************************************************
 * \brief
 *    int GeneratePic_parameter_set_rbsp (pic_parameter_set_rbsp_t *sps, char *rbsp);
 *
 * \param pps
 *    picture parameter structure
 * \param rbsp
 *    buffer to be filled with the rbsp, size should be at least MAXIMUMPARSETRBSPSIZE
 *
 * \return
 *    size of the RBSP in bytes, negative in case of an error
 *
 * \note
 *    Picture Parameter VUI function is called, but the function implements
 *    an exit (-1)
 ************************************************************************************************
 */

int GeneratePic_parameter_set_rbsp (pic_parameter_set_rbsp_t *pps, unsigned char *rbsp)
{
    Bitstream *  bitstream = &datastream;
    int len = 0, LenInBytes;

    bitstream->byte_pos = 0;
    bitstream->bits_to_go = 8;
    bitstream->byte_buf = 0x00;
    bitstream->streamBuffer = rbsp;
    

    len+=encode_ue_v (pps->pic_parameter_set_id, bitstream);
    len+=encode_ue_v (pps->seq_parameter_set_id, bitstream);
    len+=encode_u_1  (pps->entropy_coding_mode_flag, bitstream);
    len+=encode_u_1  (pps->pic_order_present_flag, bitstream);
    len+=encode_ue_v (pps->num_slice_groups_minus1, bitstream);
    len+=encode_ue_v (pps->num_ref_idx_l0_active_minus1, bitstream);
    len+=encode_ue_v (pps->num_ref_idx_l1_active_minus1, bitstream);
    len+=encode_u_1  (pps->weighted_pred_flag, bitstream);
    len+=encode_u_v  (2, pps->weighted_bipred_idc, bitstream);
    len+=encode_se_v (pps->pic_init_qp_minus26, bitstream);
    len+=encode_se_v (pps->pic_init_qs_minus26, bitstream);
    len+=encode_se_v (pps->chroma_qp_index_offset,                    bitstream);
    len+=encode_u_1  (pps->deblocking_filter_control_present_flag,    bitstream);
    len+=encode_u_1  (pps->constrained_intra_pred_flag,               bitstream);
    len+=encode_u_1  (pps->redundant_pic_cnt_present_flag,            bitstream);

    SODBtoRBSP(bitstream);     // copies the last couple of bits into the byte buffer  
    LenInBytes=bitstream->byte_pos;
    return LenInBytes;
}
//////////////////////////////////////////////////////////
//
// H264 decoder functions
//
//////////////////////////////////////////////////////////
int InterpretSPS (seq_parameter_set_rbsp_t *sps) //Lsk : mapping spec 7.3.2.1
{
	u32 reserved_zero;
	Bitstream *s = &datastream; //Lsk : for compiler pass
	u8 err=0;

	sps->profile_idc                               = decode_u_v  (8, "SPS: profile_idc"                           , s);	
	sps->constrained_set0_flag                     = decode_u_1  (   "SPS: constrained_set0_flag"                 , s);
	sps->constrained_set1_flag                     = decode_u_1  (   "SPS: constrained_set1_flag"                 , s);
	sps->constrained_set2_flag                     = decode_u_1  (   "SPS: constrained_set2_flag"                 , s);
	sps->constrained_set3_flag                     = decode_u_1  (   "SPS: constrained_set3_flag"                 , s);
	reserved_zero                                  = decode_u_v  (4, "SPS: reserved_zero_4bits"                   , s);
	

	sps->level_idc                                 = decode_u_v  (8, "SPS: level_idc"                             , s);  
	sps->seq_parameter_set_id                      = decode_ue_v ("SPS: seq_parameter_set_id"                     , s);

	if((sps->profile_idc==100)||(sps->profile_idc==110)||(sps->profile_idc==122)||(sps->profile_idc==144))
	{
		DEBUG_H264("H264 encoder IP not profile_idc\n");
		err = 1;
	}
	
	sps->log2_max_frame_num_minus4                 = decode_ue_v ("SPS: log2_max_frame_num_minus4"                , s);
	sps->pic_order_cnt_type                        = decode_ue_v ("SPS: pic_order_cnt_type"                       , s);

	if (sps->pic_order_cnt_type == 0)
	{
		DEBUG_H264("H264 encoder IP not support pic_order_cnt_type=0\n");
		err = 1;
		//sps->log2_max_pic_order_cnt_lsb_minus4 	   = decode_ue_v ("SPS: log2_max_pic_order_cnt_lsb_minus4"           , s);
	}
	else if (sps->pic_order_cnt_type == 1)
	{
		DEBUG_H264("H264 encoder IP not support pic_order_cnt_type=1\n");		
		err =1;
		//sps->delta_pic_order_always_zero_flag      = decode_u_1  ("SPS: delta_pic_order_always_zero_flag"       , s);
		//sps->offset_for_non_ref_pic                = decode_se_v ("SPS: offset_for_non_ref_pic"                 , s);
		//sps->offset_for_top_to_bottom_field        = decode_se_v ("SPS: offset_for_top_to_bottom_field"         , s);
		//sps->num_ref_frames_in_pic_order_cnt_cycle = decode_ue_v ("SPS: num_ref_frames_in_pic_order_cnt_cycle"  , s);
		//for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
		//	sps->offset_for_ref_frame[i]           = decode_se_v ("SPS: offset_for_ref_frame[i]"              , s);		
	}
	sps->num_ref_frames                        	   = decode_ue_v ("SPS: num_ref_frames"                         , s);
	sps->gaps_in_frame_num_value_allowed_flag  	   = decode_u_1  ("SPS: gaps_in_frame_num_value_allowed_flag"   , s);
	sps->pic_width_in_mbs_minus1               	   = decode_ue_v ("SPS: pic_width_in_mbs_minus1"                , s);
	sps->pic_height_in_map_units_minus1            = decode_ue_v ("SPS: pic_height_in_map_units_minus1"         , s);
	sps->frame_mbs_only_flag                   	   = decode_u_1  ("SPS: frame_mbs_only_flag"                    , s);
	if (!sps->frame_mbs_only_flag)
	{
		DEBUG_H264("H264 encoder IP not support !sps->frame_mbs_only_flag\n");	
		err =1;		
		//sps->mb_adaptive_frame_field_flag          = decode_u_1  ("SPS: mb_adaptive_frame_field_flag"           , s);
	}
	sps->direct_8x8_inference_flag             	   = decode_u_1  ("SPS: direct_8x8_inference_flag"              , s);
	sps->frame_cropping_flag                   	   = decode_u_1  ("SPS: frame_cropping_flag"                , s);

	if (sps->frame_cropping_flag)
	{
		DEBUG_H264("H264 encoder IP not support frame_cropping_flag\n");
		err =1;				
		//sps->frame_cropping_rect_left_offset       = decode_ue_v ("SPS: frame_cropping_rect_left_offset"           , s);
		//sps->frame_cropping_rect_right_offset      = decode_ue_v ("SPS: frame_cropping_rect_right_offset"          , s);
		//sps->frame_cropping_rect_top_offset        = decode_ue_v ("SPS: frame_cropping_rect_top_offset"            , s);
		//sps->frame_cropping_rect_bottom_offset     = decode_ue_v ("SPS: frame_cropping_rect_bottom_offset"         , s);
	}

	sps->vui_parameters_present_flag           	   = decode_u_1  ("SPS: vui_parameters_present_flag"            , s);
	if(sps->vui_parameters_present_flag)
	{		
		DEBUG_H264("H264 encoder IP not support\n");
		err =1;		
	}
	rbsp_trailing_bits();

	#if 0//debug																							//case901
	DEBUG_H264("profile_idc = %d\n"							,sps->profile_idc						  );//66
	DEBUG_H264("constrained_set0_flag = %d\n"				,sps->constrained_set0_flag				  );// 0
	DEBUG_H264("constrained_set1_flag = %d\n"				,sps->constrained_set1_flag				  );// 0
	DEBUG_H264("constrained_set2_flag = %d\n"				,sps->constrained_set2_flag				  );// 0
	DEBUG_H264("constrained_set3_flag = %d\n"				,sps->constrained_set3_flag				  );// 0

	DEBUG_H264("reserved_zero = %d\n"						,reserved_zero							  );// 0
	DEBUG_H264("level_idc = %d\n"							,sps->level_idc							  );// 30
	DEBUG_H264("seq_parameter_set_id = %d\n"				,sps->seq_parameter_set_id				  );// 0
	DEBUG_H264("log2_max_frame_num_minus4 = %d\n"			,sps->log2_max_frame_num_minus4			  );// 0
	DEBUG_H264("pic_order_cnt_type = %d\n"					,sps->pic_order_cnt_type				  );// 2

	DEBUG_H264("num_ref_frames = %d\n"						,sps->num_ref_frames					  );// 1
	DEBUG_H264("gaps_in_frame_num_value_allowed_flag = %d\n",sps->gaps_in_frame_num_value_allowed_flag);// 0
	DEBUG_H264("pic_width_in_mbs_minus1 = %d\n"				,sps->pic_width_in_mbs_minus1			  );// 79
	DEBUG_H264("pic_height_in_map_units_minus1 = %d\n"		,sps->pic_height_in_map_units_minus1	  );// 44
	DEBUG_H264("frame_mbs_only_flag = %d\n"					,sps->frame_mbs_only_flag				  );// 1
	

	DEBUG_H264("direct_8x8_inference_flag = %d\n"			,sps->direct_8x8_inference_flag			  );// 1
	DEBUG_H264("frame_cropping_flag = %d\n"					,sps->frame_cropping_flag				  );// 0
	DEBUG_H264("vui_parameters_present_flag = %d\n"			,sps->vui_parameters_present_flag		  );// 0	 	
	#endif

	return err;
}

int InterpretPPS (pic_parameter_set_rbsp_t *pps) //Lsk : mapping spec 7.3.2.2
{
	Bitstream *s = &datastream; //Lsk : for compiler pass
	u8 err=0;

	pps->pic_parameter_set_id                   = decode_ue_v ("PPS: pic_parameter_set_id"                   , s);
	pps->seq_parameter_set_id                   = decode_ue_v ("PPS: seq_parameter_set_id"                   , s);
	pps->entropy_coding_mode_flag               = decode_u_1  ("PPS: entropy_coding_mode_flag"               , s);
	pps->pic_order_present_flag                 = decode_u_1  ("PPS: pic_order_present_flag"                 , s);
	pps->num_slice_groups_minus1                = decode_ue_v ("PPS: num_slice_groups_minus1"                , s);
	if (pps->num_slice_groups_minus1 > 0)
	{
		DEBUG_H264("H264 encoder IP not support num_slice_groups_minus1\n");	
		err = 1;
	}

	pps->num_ref_idx_l0_active_minus1           = decode_ue_v ("PPS: num_ref_idx_l0_active_minus1"           , s);
	pps->num_ref_idx_l1_active_minus1           = decode_ue_v ("PPS: num_ref_idx_l1_active_minus1"           , s);
	pps->weighted_pred_flag                     = decode_u_1  ("PPS: weighted prediction flag"               , s);
	pps->weighted_bipred_idc                    = decode_u_v  ( 2, "PPS: weighted_bipred_idc"                , s);
	pps->pic_init_qp_minus26                    = decode_se_v ("PPS: pic_init_qp_minus26"                    , s);

	pps->pic_init_qs_minus26                    = decode_se_v ("PPS: pic_init_qs_minus26"                    , s);
	pps->chroma_qp_index_offset                 = decode_se_v ("PPS: chroma_qp_index_offset"                 , s);
	pps->deblocking_filter_control_present_flag = decode_u_1  ("PPS: deblocking_filter_control_present_flag" , s);
	pps->constrained_intra_pred_flag            = decode_u_1  ("PPS: constrained_intra_pred_flag"            , s);
	pps->redundant_pic_cnt_present_flag         = decode_u_1  ("PPS: redundant_pic_cnt_present_flag"         , s);

	if(more_rbsp_data())
	{
		DEBUG_H264("H264 encoder IP not support more_rbsp_data\n");	
		err = 1;
	}
	rbsp_trailing_bits();


	#if 0//debug																								  //case901
	DEBUG_H264("pic_parameter_set_id = %d\n"					,pps->pic_parameter_set_id					);// 0
	DEBUG_H264("seq_parameter_set_id = %d\n"					,pps->seq_parameter_set_id				  	);// 0
	DEBUG_H264("entropy_coding_mode_flag = %d\n"				,pps->entropy_coding_mode_flag				);// 0
	DEBUG_H264("pic_order_present_flag = %d\n"					,pps->pic_order_present_flag				);// 0
	DEBUG_H264("num_slice_groups_minus1 = %d\n"					,pps->num_slice_groups_minus1				);// 0

	DEBUG_H264("num_ref_idx_l0_active_minus1 = %d\n"			,pps->num_ref_idx_l0_active_minus1			);// 0
	DEBUG_H264("num_ref_idx_l1_active_minus1 = %d\n"			,pps->num_ref_idx_l1_active_minus1			);// 0
	DEBUG_H264("weighted_pred_flag = %d\n"						,pps->weighted_pred_flag				  	);// 0
	DEBUG_H264("weighted_bipred_idc = %d\n"						,pps->weighted_bipred_idc			  		);// 0
	DEBUG_H264("pic_init_qp_minus26 = %d\n"						,pps->pic_init_qp_minus26				  	);// 0

	DEBUG_H264("pic_init_qs_minus26 = %d\n"						,pps->pic_init_qs_minus26					);// 0
	DEBUG_H264("chroma_qp_index_offset = %d\n"					,pps->chroma_qp_index_offset				);// 0
	DEBUG_H264("deblocking_filter_control_present_flag = %d\n"	,pps->deblocking_filter_control_present_flag);// 0
	DEBUG_H264("constrained_intra_pred_flag = %d\n"				,pps->constrained_intra_pred_flag	  		);// 0
	DEBUG_H264("redundant_pic_cnt_present_flag = %d\n"			,pps->redundant_pic_cnt_present_flag		);// 0	

	#endif

	return err;
}
void ProcessSPS (NALU_t *nalu)
{
	seq_parameter_set_rbsp_t *sps = &active_sps; 
	InterpretSPS(sps);  
}


void ProcessPPS (NALU_t *nalu)
{  
	pic_parameter_set_rbsp_t *pps = &active_pps; 
	InterpretPPS(pps);  
}

