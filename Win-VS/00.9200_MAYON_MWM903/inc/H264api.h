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
#if (H1_264_ENC || H1_264TEST_ENC)
typedef struct 
{
    
    //H1H264_SWREG7 H1H264_SWREG8 Input reference frame buffer
    u8 * RefRawYAddr;                   //0x001C, Input reference image Y addres
    u8 * RefRawUVAddr;                  //0x0020, Input reference image UV addres

    //H1H264_SWREG9 H1H264_SWREG10 Output Reconstruct frame buffer
    u8 * RecRawYAddr;                   //0x0024, Output reconstructed image Y address
    u8 * RecRawUVAddr;                  //0x0028, Output reconstructed image UV address
    
    //H1H264_SWREG11 H1H264_SWREG12 Input source frame buffer
    u8 * CurrRawYAddr;                  //0x002C, Input source current image Y address
    u8 * CurrRawUAddr;                  //0x0030, Input source current image U address
    u8 * CurrRawVAddr;                  //0x0034, Input source current image V address

    //H1H264_SWREG14
    u8 HEncIntTimeout;                  //0x0038 [31],Enable interrupt for timeout.
    u8 HEncMvWrite;                     //0x0038 [30],Enable writing MV and SAD of each MB to BaseMvWrite.
    u8 HEncNalSizeWrite;                //0x0038 [29],Enable writing size of each NAL unit to BaseControl, nalSizeWriteOut.
    u8 HEncIntSliceReady;               //0x0038 [28],Enable interrupt for slice ready.
    u32 HEncWidth;                      //0x0038 [27:19],Encoded width. lumWidth (macroblocks) H264:[9..255] JPEG:[6..511].
    u32 HEncHeight;                     //0x0038 [18:10],Encoded height. lumHeight (macroblocks) H264:[6..255] JPEG:[2..511].
    u8 HEncRecWriteBuffer;              //0x0038 [7],Size of buffer used when writing reconstructed image. 0=1 MB buffered (write burst for every MB). 1=4 MBs buffered (write burst for every fourth MB).
    u8 HEncRecWriteDisable;             //0x0038 [6],Disable writing of reconstructed image. recWriteDisable.
    u8 HEncPictureType;                 //0x0038 [4:3],Encoded picture type. frameType. 0=INTER. 1=INTRA(IDR). 2=MVC-INTER. 3=MVC-INTER(ref mod).
    u8 HEncEncodingMode;                //0x0038 [2:1],Encoding mode. stream type.1:VP8,2:JPEG,3:H264.

    //H1H264_SWREG15 
    u8 HEncChrOffset;                   //0x003c[31:29]
    u8 HEncLumOffset;                   //0x003c[28:26]

    //H1H264_SWREG16
    u8 picqp;                           //0x0040[31:26],H.264 Pic init qp in PPS [0..51]
    //u8 slice_alpha_c0_offset_div2;      //0x0040[22:25],H.264 Slice filter alpha c0 offset div2 [-6..6]
    //u8 slice_beta_offset_div2;          //0x0040[21:18],H.264 Slice filter beta offset div2 [-6..6]  
    s8 slice_alpha_c0_offset_div2;      //0x0040[22:25],H.264 Slice filter alpha c0 offset div2 [-6..6]
    s8 slice_beta_offset_div2;          //0x0040[21:18],H.264 Slice filter beta offset div2 [-6..6]  
    u8 chroma_qp_index_offset;          //0x0040[17:13],H.264 Chroma qp index offset [-12..12]
    u8 idr_pic_id;                      //0x0040[04:01],H.264 IDR picture ID
    
    //H1H264_SWREG17
    u8  ppsId;                          //0x0044[31:24]
    int frame_num;                      //0x0044[15:00]

    //H1H264_SWREG18
    u8 Disable_deblocking_filter_idc;   //0x0048[31:30]
    
    //H1H264_SWREG25
    u8 slice_qp_delta;                  //0x0064[31:28],MAD based QP adjustment.madQpChange [-8..7]
    //s8 slice_qp_delta;                  //0x0064[31:28],MAD based QP adjustment.madQpChange [-8..7]
    u8 madthreshold;                    //0x0064[27:22],MAD threshold div256
    
    //H1H264_SWREG27
    int qp;                             //0x006c[31:26],H.264 Initial QP.qpLum [0..51]
    int maxqp;                          //0x006c[25:20],H.264 Maximum QP.qpMax [0..51]
    int minqp;                          //0x006c[19:14],H.264 Minimum QP.qpMin [0..51]
    int HEncCPDist;                     //0x006c[12:00],H.264 Checkpoint distance(mb)0=disabled [0..8191],
    

    
    //Encoder local reference buffer 
    u8 * ILFPredBuf;                  //0x0128, Output ILF prediction data base address  , FRAME_WIDTH*4*2
    u8 * IntraPredBuf;                //0x012c, Output Intra prediction data base address, FRAME_WIDTH*2
    u32  ILF_offset;                  //0x0134, ILF prediction data offset in external memory, word aligned, FRAME_WIDTH*4*2/4 
    u32  IntraPred_offset;            //0x0138, Intra prediction data offset in external memory, word aligned, FRAME_WIDTH*2/4


    //Output Bitstram
    u8 * H264StreamAddr;              //0x0130, Output encoded bitstream base address
    u32  H264StreamSize;              //0x0000[17:02]  
    u8 * sizeTblBase;

    //NAL setting
    int nal_ref_idc;                  //0x0008[30:29] //NALU_PRIORITY_xxxx
    int nal_unit_type;                //0x0008[28:24]


    //SPS setting
    int level_idc;                    //0x0008[07:00]
    int pic_width_in_mbs;             //0x0020[26:19]
    int pic_height_in_map_unit;       //0x0020[18:11]


    //Slice header setting  
    int small_frame_num;                    //0x000c[31:25]
    int small_slice_qp_delta;
    int slice_type;                   //0x0020[10] 

    //Intra/Inter Mode Algorithm setting
    int fme_algo;                     //0x001c[01:00]   
    int me_cand_num;                  //0x0024[31:29]
    int small_qp;
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
    u32 poffset;


    
}H264_ENC_CFG;
#else
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
    u8 * sizeTblBase;

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
    int small_frame_num;                    //0x000c[31:25]
    int slice_qp_delta;               //0x000c[07:00]
    int small_slice_qp_delta;
    int slice_type;                   //0x0020[10] 
    int Disable_deblocking_filter_idc;//0x0020[09:08] 
    int slice_alpha_c0_offset_div2;   //0x0020[07:04] 
    int slice_beta_offset_div2;       //0x0020[03:00]   

    //Intra/Inter Mode Algorithm setting
    int fme_algo;                     //0x001c[01:00]   
    int me_cand_num;                  //0x0024[31:29]
    int qp;                           //0x0024[28:23]
    int small_qp;
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
    u32 poffset;
}H264_ENC_CFG;
#endif
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

extern u8  PlaybackTHB_NUM;
extern H264_ENC_CFG H264Enc_cfg;
extern H264_DEC_CFG H264Dec_cfg;
extern u32 H264_FrameError[MAX_RFIU_UNIT];
extern void H264Enc_Init(void);
extern void H264IntHandler(void);
extern u32 H264Enc_DecSliceType(H264_ENC_CFG*, u32, u32);
extern u32 H264Enc_CompressOneFrame(VIDEO_INFO *,H264_ENC_CFG*,u8);
extern u32 H264Dec_DecompressOneFrame(VIDEO_INFO *,H264_DEC_CFG*);
extern u32 H264Enc_SetResolution(H264_ENC_CFG*, u16 , u16);
extern s32 H264Enc_InitCfg(H264_ENC_CFG*);
extern s32 H264Enc_GenerateParameterSets_HW(H264_ENC_CFG*);

extern u32 rfiuH264Decode(VIDEO_INFO *, H264_DEC_CFG*, u8*, u32, int, unsigned int, int, int,int);
extern s32 rfiuH264Decoding1Frame(VIDEO_INFO *,MP4Dec_Bits* , u32 ,int ,int ,u8 *, int,int);
extern void H264PutDummyHeader(u32 ,u32 ,u8*, u32*,u8);
extern void rfiuH264Decode_I_Header(H264_ENC_CFG* ,H264_DEC_CFG* );
extern void rfiuH264Encode_I_Header(H264_ENC_CFG* , u32 , u32 );
extern s32 H264Enc_SetNALSliceHeader(H264_ENC_CFG* ); 
extern s32 H264Enc_SetModeDecisionAlgorithm(H264_ENC_CFG* );   

#endif
