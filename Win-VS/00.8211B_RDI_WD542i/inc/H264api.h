/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

	H264api.h

Abstract:

   	The application interface of the H264 encoder/decoder.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2011/09/09	Lsk	Create	

*/

#ifndef __H264_API_H__
#define __H264_API_H__

#include "VideoCodecAPI.h"
/* constant */

#define CCU_IP_FIRMWARE 1
typedef struct 
{
    //Input source frame buffer
    u8 * CurrRawYAddr;                //0x0104, Input source current image Y address
    u8 * CurrRawUAddr;                //0x0108, Input source current image U address
    u8 * CurrRawVAddr;                //0x010c, Input source current image V address

    //Input reference frame buffer
    u8 * RefRawYAddr;                 //0x0110, Input reference image Y addres
    u8 * RefRawUVAddr;                //0x0114, Input reference image UV addres


    //Output Reconstruct frame buffer
    u8 * RecRawYAddr;                 //0x0118, Output reconstructed image Y address
    u8 * RecRawUVAddr;                //0x0124, Output reconstructed image UV address


    //Encoder local reference buffer 
    u8 * ILFPredBuf;                  //0x0128, Output ILF prediction data base address  , FRAME_WIDTH*4*2
    u8 * IntraPredBuf;                //0x012c, Output Intra prediction data base address, FRAME_WIDTH*2
    u32  ILF_offset;                  //0x0134, ILF prediction data offset in external memory, word aligned, FRAME_WIDTH*4*2/4 
    u32  IntraPred_offset;            //0x0138, Intra prediction data offset in external memory, word aligned, FRAME_WIDTH*2/4


    //Output Bitstram
    u8 * H264StreamAddr;              //0x0130, Output encoded bitstream base address
    u32  H264StreamSize;              //0x0000[17:02]  

    //NAL setting
    int nal_ref_idc;                  //0x0008[30:29] //NALU_PRIORITY_xxxx
    int nal_unit_type;                //0x0008[28:24]


    //SPS setting
    int level_idc;                    //0x0008[07:00]
    int pic_width_in_mbs;             //0x0020[26:19]
    int pic_height_in_map_unit;       //0x0020[18:11]

    //PPS setting
    int chroma_qp_index_offset;       //0x0020[31:27]

    //Slice header setting  
    int idr_pic_id;                   //0x0008[23:16]
    int frame_num;                    //0x000c[31:25]
    int slice_qp_delta;               //0x000c[07:00]
    int slice_type;                   //0x0020[10] 
    int Disable_deblocking_filter_idc;//0x0020[09:08] 
    int slice_alpha_c0_offset_div2;   //0x0020[07:04] 
    int slice_beta_offset_div2;       //0x0020[03:00]   

    //Intra/Inter Mode Algorithm setting
    int fme_algo;                     //0x001c[01:00]   
    int me_cand_num;                  //0x0024[31:29]
    int qp;                           //0x0024[28:23]
    int qs_mode_sel;                  //0x0024[13:07]
    int lambda;                        //0x0024[06:00]
    int stage_1_sr;                   //0x0028[17:12]
    int stage_2_sr;                   //0x0028[11:08]
    int don_spl_rat_x;                  //0x0028[07:04]
    int don_spl_rat_y;                //0x0028[03:00]

    //Rate control
    int TotalTextureBit;              //0x0180
    int TotalHeaderBit;               //0x0184
    int TotalSATD;                    //0x0188  
    int TotalBit;                     //0x018c  

    //other
    int resolution;
}H264_ENC_CFG;
typedef struct 
{
	//NAL setting
    int nal_ref_idc;                  
    int nal_unit_type;                

	//SPS setting
    int level_idc;                    
    int pic_width_in_mbs;             
    int pic_height_in_map_unit;       

    //PPS setting
    int chroma_qp_index_offset;       //0x0020[31:27]

	//Slice header
	u32 slice_type;
	u32 qp;
	
    u32 PicWidthInPix;
    u32 PicHeightInPix;
    u32 PicWidthInMbs;
    u32 PicHeightInMbs;
    u32 PicSize;                //0x0038     img->width*img->height;            

								// **NOTE** : the offset unit is Double-WORD per unit                                
    u8  *MB_INFO_BASE;          //0x0000       size : img->width*2*4*(1+active_sps->mb_adaptive_frame_field_flag) byte
    u32 MB_INFO_OFFSET;         //0x0004
    u8  *ILF_COEFF_BASE;        //0x0008       size : img->width*2*4*(1+active_sps->mb_adaptive_frame_field_flag) byte
    u32 ILF_COEFF_OFFSET;       //0x000c       
    u8  *INTRA_PRED_BASE;       //0x0010       size : img->width*2*(1+active_sps->mb_adaptive_frame_field_flag) byte
    u32 INTRA_PRED_OFFSET;      //0x0014       

    //Output Reconstruct frame buffer
    u8  *RecRawYAddr;           //0x0018, Output reconstructed image Y address
    u8  *RecRawUVAddr;          //0x001c, Output reconstructed image UV address

    //Input Bitstram
    u8  *H264StreamAddr;            //0x0310,   Input encoded bitstream base address
    u32 H264StreamSize;             //0x0314

} H264_DEC_CFG;

extern H264_ENC_CFG H264Enc_cfg;
extern H264_DEC_CFG H264Dec_cfg;


extern void H264IntHandler(void);
extern u32 H264Enc_DecSliceType(H264_ENC_CFG*, u32, u32);
extern u32 H264Enc_CompressOneFrame(VIDEO_INFO *,H264_ENC_CFG*);
extern s32 H264Dec_DecompressOneFrame(VIDEO_INFO *,H264_DEC_CFG*);
extern u32 H264Enc_SetResolution(H264_ENC_CFG*, u16 , u16);

extern u32 rfiuH264Encode_I_Header(H264_ENC_CFG*, u32, u32);    
extern u32 rfiuH264Decode(VIDEO_INFO *, H264_DEC_CFG*, u8*, u32, int, unsigned int, int, int);
extern s32 rfiuH264Decoding1Frame(VIDEO_INFO *,MP4Dec_Bits* , u32 ,int ,int ,u8 *, int);

#endif
