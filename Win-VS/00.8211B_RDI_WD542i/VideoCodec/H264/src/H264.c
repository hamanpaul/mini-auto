/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

    H264.c

Abstract:

    The routines of H264 encoder/decoder.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2011/08/29  Lsk  Create  

*/
#include "general.h"
#include "board.h"
#include "task.h"
#include "VideoCodec_common.h"
#include "NALUCommon.h"
#include "NALU.h"
#include "H264reg.h"
#include "H264.h"
#include "H264api.h"
#include "fsapi.h"
#include "dcfapi.h"
#include "Parset.h"
#include "vlc.h"
#include "MemoryPool.h"
#include "Header.h"
#include "isuapi.h"
#include "asfapi.h"
#include "sysapi.h"
#include "rfiuapi.h"
#if MULTI_CHANNEL_SUPPORT
#include "ciuapi.h"
#endif
H264_ENC_CFG H264Enc_cfg;
H264_DEC_CFG H264Dec_cfg;
NALU_t rfiu_nalu;

#if(VIDEO_CODEC_OPTION ==  H264_CODEC)
/*
*********************************************************************************************************
* Constant
*********************************************************************************************************
*/



#define FME_NORMAL_MODE   0
#define FME_REDUCE_MODE   2


#define H264_TIMEOUT   20  



const int QP2QUANT[40]=
{
   1, 1, 1, 1, 2, 2, 2, 2,
   3, 3, 3, 4, 4, 4, 5, 6,
   6, 7, 8, 9,10,11,13,14,
  16,18,20,23,25,29,32,36,
  40,45,51,57,64,72,81,91
};

enum 
{
    VIDEO_SIZE_320x240 = 0, //0x00
    VIDEO_SIZE_352x288,     //
    VIDEO_SIZE_640x480,     //
    VIDEO_SIZE_704x480,     //
    VIDEO_SIZE_704x576,     //
    VIDEO_SIZE_720x480,     //0x05
    VIDEO_SIZE_720x576,     //
    VIDEO_SIZE_1280x720,    //
    VIDEO_SIZE_END
};

#define SHIFT_QP 12
/*
*********************************************************************************************************
* Variable
*********************************************************************************************************
*/

u8 SPS_PPS_Buffer[0x20];
u8 SPS_PPS_Length;
int QP;

//Intra
u8 qs_mode[VIDEO_SIZE_END]     = { 0, 0, 1, 1, 1, 1, 1, 2};    
//Inter-IME
u8 fme_algo[VIDEO_SIZE_END]    = { 0, 0, 0, 0, 0, 0, 0, 2};
u8 stage_1_sr[VIDEO_SIZE_END]  = {16,16,16,16,16,16,16,16};
u8 don_spl_rat[VIDEO_SIZE_END] = { 4, 4, 5, 5, 5, 5, 5, 5};
u8 me_cand_num[VIDEO_SIZE_END] = { 3, 3, 2, 2, 2, 2, 2, 1};
//Inter-FME
u8 stage_2_sr[VIDEO_SIZE_END]  = { 2, 2, 2, 2, 2, 2, 2, 2};

u8 EncodeLineStripe = 0; //default 320x240
u8 EncodeDownSample = 0;
u8 DecodeLineStripe = 0;//default 320x240
u8 DecodeDownSample = 1;

//step 1. LineStripe step 2. downsample
u32 FrameWidth = 640;
u32 LineStripeFrameWidth = 320;
u32 H_addr     = 160;
u32 V_addr     = 120;
//u32 LineStripe_YAddr = FrameWidth * V_addr + H_addr;
//u32 LineStripe_UAddr = FrameWidth*(V_addr/2) + H_addr;
//u32 LineStripeOffset = FrameWidth - LineStripeFrameWidth;
/*
*********************************************************************************************************
* Extern Varaibel
*********************************************************************************************************
*/
extern seq_parameter_set_rbsp_t active_sps;
extern pic_parameter_set_rbsp_t active_pps;
extern SLICE_HEADER_t active_slice_header;
extern u32 isu_int_status;
extern u32 IsuIndex;
extern u32 asfCaptureMode;
extern u8  SetIVOP;
extern u32 Cal_FileTime_Start_Idx;
extern u8  H264_stat;
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];
extern OS_EVENT    *mpeg4ReadySemEvt;
/*
*********************************************************************************************************
* Function prototype
*********************************************************************************************************
*/


/*

Routine Description:

    The FIQ handler of H264 encoder/decoder.

Arguments:

    None.

Return Value:

    None.

*/

void H264IntHandler(void)
{    
    u32 enc_intStat;
    u32 dec_intStat;
#if 1
    // DEBUG_H264("-->%d ",H264_stat);
 //   if(H264_stat == 1)  
        enc_intStat = H264ENC_INTERRUPT;    //encode
 //   else if (H264_stat == 2)    
        dec_intStat = H264DEC_AVC_IS;       //decode
    
    if (enc_intStat & 0x00000002)       // encoder finish   
    {   
        H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_CLR_INT; 

        OSSemPost(VideoCpleSemEvt);
        mp4_avifrmcnt++;        
    } 
    else if(dec_intStat & 0x00000008)   // decoder finish
    {   
        H264DEC_AVC_IC 			 = 0x00000008; 
        if(dec_intStat & 0x00000006)
        {
            H264DEC_AVC_IC 			 = 0x00000006; 
            DEBUG_H264("### H264 decoder error. 0x%08x\n\n\n\n\n",dec_intStat);
            return;
        }        
        
        //DEBUG_H264("H264 decoder finish\n");
        VideoPictureIndex++;
        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
        OSSemPost(VideoCpleSemEvt);        
    }
    else if(dec_intStat & 0x00000006)   // decoder error
    {   
        H264DEC_AVC_IC 			 = 0x00000006; 
        DEBUG_H264("@@@ H264 decoder error. 0x%08x\n\n\n\n\n",dec_intStat);
    }
    else
    {
        DEBUG_H264("### H264 encoder error. 0x%08x\n\n\n\n\n",enc_intStat);
        DEBUG_H264("@@@ H264 decoder error. 0x%08x\n\n\n\n\n",dec_intStat);
    }
#else
    enc_intStat = H264ENC_INTERRUPT;
//    DEBUG_H264("P1");

//	dec_intStat = H264DEC_AVC_IS;
  //  DEBUG_H264("Q2");

    if (enc_intStat & 0x00000002)       // encoder finish   
    {   
        H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_CLR_INT; 

        OSSemPost(VideoCpleSemEvt);
        mp4_avifrmcnt++;        
    } 
    else if(dec_intStat & 0x00000008)   // decoder finish
    {   
        H264DEC_AVC_IC 			 = 0x00000008; 
        if(dec_intStat & 0x00000006)
        {
            H264DEC_AVC_IC 			 = 0x00000006; 
            DEBUG_H264("### H264 decoder error. 0x%08x\n\n\n\n\n",dec_intStat);
            return;
        }        
        
        //DEBUG_H264("H264 decoder finish\n");
        VideoPictureIndex++;
        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
        OSSemPost(VideoCpleSemEvt);        
    }
    else if(dec_intStat & 0x00000006)   // decoder error
    {   
        H264DEC_AVC_IC 			 = 0x00000006; 
        DEBUG_H264("@@@ H264 decoder error. 0x%08x\n\n\n\n\n",dec_intStat);
    }
    else
    {
        DEBUG_H264("### H264 encoder error. 0x%08x\n\n\n\n\n",enc_intStat);
        DEBUG_H264("@@@ H264 decoder error. 0x%08x\n\n\n\n\n",dec_intStat);
    }
#endif
}

//////////////////////////////////////////////////////////
//
// H264 encoder functions
//
//////////////////////////////////////////////////////////
int yuv_cnt=0;
void H264Enc_Init(void)
{
	H264ENC_ADDR_ENCODE_CTL0 = 0x00000001;
    H264ENC_ADDR_ENCODE_CTL0 = 0x00000000;
}

u32 H264Enc_SetResolution(H264_ENC_CFG* cfg,u16 width, u16 height)
{
//    H264_ENC_CFG* cfg = &h264_enc_cfg;
    
    cfg->pic_width_in_mbs           = width/16;
	cfg->pic_height_in_map_unit     = height/16;  

    if(width==320 && height==240)
        cfg->resolution = VIDEO_SIZE_320x240;
    else if(width==352 && height==288)
        cfg->resolution = VIDEO_SIZE_352x288;
    else if(width==640 && height==480)
        cfg->resolution = VIDEO_SIZE_640x480;
    else if(width==704 && height==480)
        cfg->resolution = VIDEO_SIZE_704x480;
    else if(width==704 && height==576)
        cfg->resolution = VIDEO_SIZE_704x576;
    else if(width==720 && height==480)
        cfg->resolution = VIDEO_SIZE_720x480;
    else if(width==720 && height==576)
        cfg->resolution = VIDEO_SIZE_720x576;
    else if(width==1280 && height==720)
        cfg->resolution = VIDEO_SIZE_1280x720;

    return 1;
    
}
/*

Routine Description:

    Dump H264 encoder register value.

Arguments:

Return Value:


*/
void H264Enc_DumpRegister(void)
{
    DEBUG_H264("H264 Encoder dump register start..............\n");
    DEBUG_H264("Addr 0x0000 = 0x%08x\n",H264ENC_ADDR_SR0         );
    DEBUG_H264("Addr 0x0008 = 0x%08x\n",H264ENC_ADDR_SH0         );
    DEBUG_H264("Addr 0x000c = 0x%08x\n",H264ENC_ADDR_SH1         );
    DEBUG_H264("Addr 0x0018 = 0x%08x\n",H264ENC_ADDR_ENCODE_CTL0 );
    DEBUG_H264("Addr 0x001c = 0x%08x\n",H264ENC_ADDR_FME_CTL     );
    DEBUG_H264("Addr 0x0020 = 0x%08x\n",H264ENC_ADDR_SLICE_REG0  );
    DEBUG_H264("Addr 0x0024 = 0x%08x\n",H264ENC_ADDR_SLICE_REG1  );
    DEBUG_H264("Addr 0x0028 = 0x%08x\n",H264ENC_ADDR_SLICE_REG2  );
    DEBUG_H264("Addr 0x002c = 0x%08x\n",H264ENC_INTERRUPT        );
    DEBUG_H264("Addr 0x0030 = 0x%08x\n",H264ENC_INT_MASK         );
    DEBUG_H264("Addr 0x0104 = 0x%08x\n",H264ENC_ADDR_BASE_Y_CUR  );
    DEBUG_H264("Addr 0x0108 = 0x%08x\n",H264ENC_ADDR_BASE_U_CUR  );
    DEBUG_H264("Addr 0x010c = 0x%08x\n",H264ENC_ADDR_BASE_V_CUR  );
    DEBUG_H264("Addr 0x0110 = 0x%08x\n",H264ENC_ADDR_BASE_Y_REF  );
    DEBUG_H264("Addr 0x0114 = 0x%08x\n",H264ENC_ADDR_BASE_UV_REF );
    DEBUG_H264("Addr 0x0118 = 0x%08x\n",H264ENC_ADDR_BASE_Y_DP   );
    DEBUG_H264("Addr 0x0124 = 0x%08x\n",H264ENC_ADDR_BASE_UV_DP  );
    DEBUG_H264("Addr 0x0128 = 0x%08x\n",H264ENC_ADDR_BASE_ILF    );
    DEBUG_H264("Addr 0x012c = 0x%08x\n",H264ENC_ADDR_BASE_INTRA  );
    DEBUG_H264("Addr 0x0130 = 0x%08x\n",H264ENC_ADDR_BASE_BS     );
    DEBUG_H264("Addr 0x0134 = 0x%08x\n",H264ENC_ADDR_OFFSET_ILF  );
    DEBUG_H264("Addr 0x0138 = 0x%08x\n",H264ENC_ADDR_OFFSET_INTRA);
    DEBUG_H264("Addr 0x013c = 0x%08x\n",H264ENC_EN_ENC_SWRST     );
    DEBUG_H264("Addr 0x0140 = 0x%08x\n",H264ENC_ADDR_SEL_UV      );
    DEBUG_H264("Addr 0x0144 = 0x%08x\n",H264ENC_ADDR_OFFSET_LINE );
    DEBUG_H264("Addr 0x0148 = 0x%08x\n",H264ENC_ADDR_SPECIAL_CMD);
    DEBUG_H264("Addr 0x0180 = 0x%08x\n",H264ENC_ADDR_TEXTURE_BIT );
    DEBUG_H264("Addr 0x0184 = 0x%08x\n",H264ENC_ADDR_HEADER_BIT  );
    DEBUG_H264("Addr 0x0188 = 0x%08x\n",H264ENC_ADDR_BEST_COST   );
    DEBUG_H264("Addr 0x018c = 0x%08x\n",H264ENC_ADDR_TOTAL_BIT   );
    DEBUG_H264("H264 Encoder dump register end................\n");


}

u32 GetQP(u8 mode)
{
    static int frame_cnt = 0;
    static int local_QP = 15;
    
    
	if(mode == 1) //fixed QP from global QP		
		return QP;
	else if(mode == 2)//rand
	    return (rand()%26)+15;  //15~40	
    else if(mode == 3)//rand
		return (rand()%36)+10;  //10~45		    
    else if(mode == 4)//fixed QP
		return 30;	
    else if(mode == 5)//QP range 10~45, each 100 frame add QP
    {        
        if(frame_cnt == 100)
        {
            frame_cnt = 0;

            if(local_QP >= 46)
                local_QP = 10;
            else
                local_QP++;
            
        //    printf("<%d>",local_QP);
            return local_QP;	
        }
        else
        {
            frame_cnt++;            
        //    printf("<%d>",local_QP);
            return local_QP;	
        }		
    }    
    else if(mode == 6)//QP range 10~42, each frame add QP
    {       
        local_QP += 1;
        if(local_QP > 51)
            local_QP = 0;
   //     printf("<%d>",local_QP);        
        return local_QP;	
    }    
    return 28;
}


/*

Routine Description:

    Set H264 slice coding type.

Arguments:

    frame_idx - the frame idex in a coding sequence.
    Period  - the priod for coding an I frame.

Return Value:

    0 - P frame.
    1 - I frame.

*/

u32 H264Enc_DecSliceType(H264_ENC_CFG* cfg, u32 frame_idx, u32 period)
{
//    H264_ENC_CFG* cfg = &h264_enc_cfg;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
                                
    //calulate time by video time, to get exactly asf file time
    if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && SetIVOP)    
    {
        DEBUG_H264("H264 calulate time, start index = %d\n",VideoBufMngWriteIdx);        
        OS_ENTER_CRITICAL();
        Cal_FileTime_Start_Idx = VideoBufMngWriteIdx; 
        VideoTimeStatistics = 0;
        SetIVOP = 0;
        OS_EXIT_CRITICAL();
        cfg->slice_type = I_FRAME;
        
    }        
    else if((asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && SetIVOP)
    {
        DEBUG_H264("H264 calulate time, start index = %d\n",VideoBufMngWriteIdx);
        
        OS_ENTER_CRITICAL();
        Cal_FileTime_Start_Idx = VideoBufMngWriteIdx; 
        VideoTimeStatistics = 0;
        SetIVOP = 0;
        OS_EXIT_CRITICAL();
        cfg->slice_type = I_FRAME;
    }
    else if((frame_idx % period) == 0)
    {        
        cfg->slice_type = I_FRAME;        
    }
    else
    { 
        cfg->slice_type = P_FRAME;    
    }
#if 0
    if(cfg->slice_type == P_FRAME)
        DEBUG_H264("P");
    else
        DEBUG_H264("I");
#endif
    #if H264_TEST
    cfg->qp         = GetQP(1);
    #else
    cfg->qp         = GetQP(4);
    #endif
	cfg->slice_qp_delta = cfg->qp - 26; 
    
    return cfg->slice_type;
}




/*

Routine Description:    
    Slice header syntax
Arguments:

    

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 H264Enc_SetNALSliceHeader(H264_ENC_CFG* cfg)    
{
    u32 write_data;
    //H264_ENC_CFG* cfg = &h264_enc_cfg;
    
    //NAL header
    if(cfg->frame_num== 0)
    {
        cfg->nal_ref_idc   = NALU_PRIORITY_HIGHEST;
        cfg->nal_unit_type = NALU_TYPE_IDR;
        cfg->idr_pic_id    = 0;  
    }
    else
    {
        cfg->nal_ref_idc   = NALU_PRIORITY_HIGH;
        cfg->nal_unit_type = NALU_TYPE_SLICE;
    }
    write_data = (cfg->nal_ref_idc << 29)|(cfg->nal_unit_type << 24)|(cfg->idr_pic_id << 16)|(cfg->level_idc);    
    H264ENC_ADDR_SH0 = write_data;
    //DEBUG_H264("0x00000008 = 0x%08x\n",write_data);
    
    //Slice header
    write_data = (cfg->slice_qp_delta << 25)|(cfg->frame_num);
    H264ENC_ADDR_SH1 = write_data;
    //DEBUG_H264("1.H264ENC_ADDR_SH1 : %d, %d\n",cfg->slice_qp_delta, cfg->frame_num);
    //DEBUG_H264("0x0000000c = 0x%08x\n",write_data);
	
    write_data = (cfg->pic_width_in_mbs<< 19)|(cfg->pic_height_in_map_unit<< 11)|(cfg->slice_type << 10);
    //DEBUG_H264("1.H264ENC_ADDR_SLICE_REG0 : %d, %d, %d\n",cfg->pic_width_in_mbs, cfg->pic_height_in_map_unit, cfg->slice_type);
	H264ENC_ADDR_SLICE_REG0 = write_data;
    //DEBUG_H264("0x00000020 = 0x%08x\n",write_data);
    return 1;
}
/*

Routine Description:    
    set Input source frame buffer,
    set Input reference frame buffer,
    set Output Reconstruct frame buffer,
    set Encoder local reference buffer.
    set Output Bitstram
Arguments:

    

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 H264Enc_InitBuf(H264_ENC_CFG* cfg)    
{
    //H264_ENC_CFG* cfg = &h264_enc_cfg;
    #if 1
    //static int cnt = 1;
    u8 fin_name[32];
    FS_FILE*                pFile;
    u32 size;
    #endif
    
    //Local file test, use mpeg4 buffer
    //Input source frame buffer
    #if MULTI_CHANNEL_SUPPORT
    #if (MULTI_CHANNEL_SEL & 0x01)
    cfg->CurrRawYAddr  = PNBuf_Y[VideoPictureIndex % 4];
    cfg->CurrRawUAddr  = PNBuf_C[VideoPictureIndex % 4];
    #elif (MULTI_CHANNEL_SEL & 0x02)
    cfg->CurrRawYAddr  = PNBuf_sub1[VideoPictureIndex % 4];
    cfg->CurrRawUAddr  = (PNBuf_sub1[VideoPictureIndex % 4] + PNBUF_SIZE_Y);
    //printf("CIU BBBB\n");
    #elif (MULTI_CHANNEL_SEL & 0x04)
    cfg->CurrRawYAddr  = PNBuf_sub2[VideoPictureIndex % 4];
    cfg->CurrRawUAddr  = (PNBuf_sub2[VideoPictureIndex % 4] + PNBUF_SIZE_Y);
    #endif
    #else   // MULTI_CHANNEL_SUPPORT == 0
    cfg->CurrRawYAddr  = PNBuf_Y[VideoPictureIndex % 4];
    cfg->CurrRawUAddr  = PNBuf_C[VideoPictureIndex % 4];
    #endif

    #if H264_TEST
    cfg->CurrRawYAddr  = PNBuf_Y[3];
    cfg->CurrRawUAddr  = (PNBuf_Y[3]+(cfg->pic_width_in_mbs)*(cfg->pic_height_in_map_unit)*256);
    #endif

    #if 0 //check current frame
    if(yuv_cnt < 1);
    {
        printf(">");
        sprintf(fin_name, "%08d.yuv",yuv_cnt); //file system only filename 08d.03d
        pFile= dcfOpen(fin_name, "w+b");        
        if(dcfWrite(pFile, cfg->CurrRawYAddr, cfg->pic_width_in_mbs*cfg->pic_height_in_map_unit*256, &size) == 0)
        {
            DEBUG_IIS("Rec yuv File error \n");
            return 0;
        } 
        if(dcfWrite(pFile, cfg->CurrRawUAddr, cfg->pic_width_in_mbs*cfg->pic_height_in_map_unit*128, &size) == 0)
        {
            DEBUG_IIS("Rec voice File error \n");
            return 0;
        } 
        dcfClose(pFile); 

        yuv_cnt++;
    } 
    #endif
    //cfg->CurrRawYAddr = PNBuf_Y[VideoPictureIndex % 4];   //0x0104, Input source current image Y address
    //cfg->CurrRawUAddr = PNBuf_C[VideoPictureIndex % 4];   //0x0108, Input source current image U address
    H264ENC_ADDR_SEL_UV     = H264_ENC_UV_INTERLEAVE;
    if(EncodeDownSample)
        H264ENC_ADDR_SEL_UV |= H264_ENC_DOWNSAMPLE;

    if(EncodeLineStripe)
    {
        H264ENC_ADDR_BASE_Y_CUR = ((u32)cfg->CurrRawYAddr) + FrameWidth * V_addr + H_addr;;
        H264ENC_ADDR_BASE_U_CUR = ((u32)cfg->CurrRawUAddr) + FrameWidth*(V_addr/2) + H_addr;
        H264ENC_ADDR_OFFSET_LINE = FrameWidth - LineStripeFrameWidth;;
    }
    else
    {
        H264ENC_ADDR_BASE_Y_CUR = (u32)cfg->CurrRawYAddr;
        H264ENC_ADDR_BASE_U_CUR = (u32)cfg->CurrRawUAddr;
    }    
    //DEBUG_H264("0x00000140 = 0x%08x\n",H264_ENC_UV_INTERLEAVE);
    //DEBUG_H264("0x00000104 = 0x%08x\n",(u32)cfg->CurrRawYAddr);
    //DEBUG_H264("0x00000108 = 0x%08x\n",(u32)cfg->CurrRawUAddr);


    
    //Input reference frame buffer, Output Reconstruct frame buffer
    if(cfg->frame_num & 0x01)
    {
        cfg->RefRawYAddr  = VideoNRefBuf_Y;       //0x0110, Input reference image Y addres
        cfg->RefRawUVAddr = VideoNRefBuf_Cb;      //0x0114, Input reference image UV addres  
        cfg->RecRawYAddr  = VideoPRefBuf_Y;        //0x0118, Output reconstructed image Y address
        cfg->RecRawUVAddr = VideoPRefBuf_Cb;       //0x0124, Output reconstructed image UV address 
    }
    else
    {
        cfg->RefRawYAddr  = VideoPRefBuf_Y;        //0x0110, Input reference image Y addres
        cfg->RefRawUVAddr = VideoPRefBuf_Cb;       //0x0114, Input reference image UV addres  
        cfg->RecRawYAddr  = VideoNRefBuf_Y;       //0x0118, Output reconstructed image Y address
        cfg->RecRawUVAddr = VideoNRefBuf_Cb;      //0x0124, Output reconstructed image UV address 
    }
      
    
    H264ENC_ADDR_BASE_Y_REF  = (u32)cfg->RefRawYAddr;
    H264ENC_ADDR_BASE_UV_REF = (u32)cfg->RefRawUVAddr;
    H264ENC_ADDR_BASE_Y_DP   = (u32)cfg->RecRawYAddr;
    H264ENC_ADDR_BASE_UV_DP  = (u32)cfg->RecRawUVAddr;
    
    //DEBUG_H264("0x00000110 = 0x%08x\n",(u32)cfg->RefRawYAddr);
    //DEBUG_H264("0x00000114 = 0x%08x\n",(u32)cfg->RefRawUVAddr);
    //DEBUG_H264("0x00000118 = 0x%08x\n",(u32)cfg->RecRawYAddr);
    //DEBUG_H264("0x00000124 = 0x%08x\n",(u32)cfg->RecRawUVAddr);



    

    //Encoder local reference buffer 
    cfg->ILFPredBuf       = H264ILFPredBuf;              //0x0128, Output ILF prediction data base address  , FRAME_WIDTH*4*12+0x3F
    cfg->IntraPredBuf     = H264IntraPredBuf;            //0x012c, Output Intra prediction data base address, FRAME_WIDTH*2*12+0x3F
    cfg->ILF_offset       = cfg->pic_width_in_mbs * 32;  //0x0134, ILF prediction data offset in external memory, FRAME_WIDTH*4*2/4   
    cfg->IntraPred_offset = cfg->pic_width_in_mbs * 8;  //0x0138, Intra prediction data offset in external memory, FRAME_WIDTH*2/4  
    
    H264ENC_ADDR_BASE_ILF     =  (u32)cfg->ILFPredBuf;
    H264ENC_ADDR_BASE_INTRA   =  (u32)cfg->IntraPredBuf;
    H264ENC_ADDR_OFFSET_ILF   =  cfg->ILF_offset;
    H264ENC_ADDR_OFFSET_INTRA =  cfg->IntraPred_offset;   
        
    //DEBUG_H264("0x00000128 = 0x%08x\n",(u32)cfg->ILFPredBuf);
    //DEBUG_H264("0x0000012c = 0x%08x\n",(u32)cfg->IntraPredBuf);
    //DEBUG_H264("0x00000134 = 0x%08x\n",cfg->ILF_offset);
    //DEBUG_H264("0x00000138 = 0x%08x\n",cfg->IntraPred_offset);
    
    //Output Bitstram
    //cfg->H264StreamAddr   = VideoBufMng[0].buffer;   //0x0130, Output encoded bitstream base address
    H264ENC_ADDR_BASE_BS  = (u32)cfg->H264StreamAddr;
    //DEBUG_H264("0x00000130 = 0x%08x\n",(u32)cfg->H264StreamAddr);
    return 1;
}



/*

Routine Description:    
    //Intra/Inter Mode Algorithm setting
Arguments:

    

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 H264Enc_SetModeDecisionAlgorithm(H264_ENC_CFG* cfg)    
{
    u32 write_data;
    //H264_ENC_CFG* cfg = &h264_enc_cfg;
    

    //Intra/Inter Mode Algorithm setting    
   cfg->fme_algo = fme_algo[cfg->resolution];      
     
    cfg->me_cand_num    = me_cand_num[cfg->resolution];  
    //cfg->qp             = QPISlice;                 //current QP
    cfg->qs_mode_sel    = qs_mode[cfg->resolution];   //0x0024[13:07]

    if(cfg->qp  >= 12)
	    cfg->lambda = QP2QUANT[cfg->qp-SHIFT_QP];
	else
		cfg->lambda = QP2QUANT[0];
    //cfg->lambda
    cfg->stage_1_sr     = stage_1_sr[cfg->resolution];
    cfg->stage_2_sr     = stage_2_sr[cfg->resolution];
    cfg->don_spl_rat_x  = don_spl_rat[cfg->resolution];
    cfg->don_spl_rat_y  = don_spl_rat[cfg->resolution];
       

    write_data = (cfg->me_cand_num << 29)|(cfg->qp << 23)|(cfg->qs_mode_sel << 7)|(cfg->lambda);
    //DEBUG_H264("H264ENC_ADDR_SLICE_REG1 : %d, %d, %d, %d\n", cfg->me_cand_num, cfg->qp, cfg->qs_mode_sel, cfg->lambda);
    H264ENC_ADDR_SLICE_REG1 = write_data;
    //DEBUG_H264("0x00000024 = 0x%08x\n",write_data);
    
    write_data = (cfg->stage_1_sr  << 12)|(cfg->stage_2_sr  << 8)|(cfg->don_spl_rat_y << 4)|(cfg->don_spl_rat_x);
    //DEBUG_H264("H264ENC_ADDR_SLICE_REG2 : %d, %d, %d, %d\n", cfg->stage_1_sr, cfg->stage_2_sr, cfg->don_spl_rat, cfg->don_spl_rat_x);
    H264ENC_ADDR_SLICE_REG2 = write_data;
    //DEBUG_H264("0x00000028 = 0x%08x\n",write_data);

    H264ENC_ADDR_FME_CTL = cfg->fme_algo;
    //DEBUG_H264("0x0000001c = 0x%08x\n",cfg->fme_algo);


    return 1;
}
/*

Routine Description:

    Initialize H264 encoder.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 H264Enc_InitCfg(H264_ENC_CFG* cfg) //check, qp
{
    //H264_ENC_CFG* cfg = &h264_enc_cfg;

    //NAL setting
    cfg->nal_ref_idc      = NALU_PRIORITY_HIGHEST;       //0x0008[30:29]
    cfg->nal_unit_type    = NALU_TYPE_IDR;               //0x0008[28:24]
    //SPS setting
    
    if(cfg->pic_width_in_mbs <= 80 && cfg->pic_height_in_map_unit < 64)
        cfg->level_idc    = 30;                          //0x0008[07:00]   
    else
        cfg->level_idc    = 40;                          //0x0008[07:00]
    //cfg->pic_width_in_mbs           = FRAME_WIDTH/16;
	//cfg->pic_height_in_map_unit     = FRAME_HEIGHT/16;        
    //PPS setting
    cfg->chroma_qp_index_offset     = 0;                 //0x0020[31:27]  
    //Slice header setting
    cfg->slice_type                 = I_FRAME;           //0x0020[10] 
    cfg->idr_pic_id                 = 0;                 //0x0008[23:16]
    cfg->frame_num                  = 0;                 //0x000c[31:25]
    //cfg->qp                         = 26;          		 //0x0024[28:23]
    //cfg->slice_qp_delta             = cfg->qp - 26;      //0x000c[07:00]
    cfg->Disable_deblocking_filter_idc = 0;              //0x0020[09:08] 
    cfg->slice_alpha_c0_offset_div2    = 0;              //0x0020[07:04] 
    cfg->slice_beta_offset_div2        = 0;              //0x0020[03:00]   
	
    return 1;
}

void H264Enc_GetParameterSets(u8 *buf, u32* length) //Lsk : add to asfHeaderObject, to steam by VLC
{
    memcpy(buf, SPS_PPS_Buffer, SPS_PPS_Length);
    *length = SPS_PPS_Length;
}
s32 H264Enc_GenerateParameterSets_HW(H264_ENC_CFG* cfg) //Lsk : do the same thing with H264Enc_InitCfg
{
    u32 write_data;  	
    //H264_ENC_CFG* cfg = &h264_enc_cfg;
    
    write_data = (0x65 << 24) | (cfg->idr_pic_id << 16) | (cfg->level_idc);
    H264ENC_ADDR_SH0      = write_data;
    //DEBUG_H264("0x00000008 = 0x%08x\n",write_data);

    write_data = (cfg->slice_qp_delta << 25) | (cfg->frame_num);
	H264ENC_ADDR_SH1 = write_data;
    //DEBUG_H264("0x0000000c = 0x%08x\n",write_data);
        
    write_data = (cfg->pic_width_in_mbs  << 19) | (cfg->pic_height_in_map_unit << 11) | 1 << 10;
    //DEBUG_H264("2.H264ENC_ADDR_SLICE_REG0 : %d, %d, %d\n",cfg->pic_width_in_mbs, cfg->pic_height_in_map_unit, cfg->slice_type);
	H264ENC_ADDR_SLICE_REG0 = write_data;
    //DEBUG_H264("0x00000020 = 0x%08x\n",write_data);
	
    //Output Bitstram
    //cfg->H264StreamAddr   = VideoBufMng[0].buffer;   //0x0130, Output encoded bitstream base address
    H264ENC_ADDR_BASE_BS  = (u32)cfg->H264StreamAddr;
    //DEBUG_H264("0x00000130 = 0x%08x\n",(u32)cfg->H264StreamAddr);
    
    // Start to encode SPS/PPS    
    H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_SPS_PPS;
    //DEBUG_H264("0x00000018 = 0x00000010\n");
       
    return 1;
}

extern u32 H264_Special_CMD;

u32 H264Enc_CompressOneFrame(VIDEO_INFO * info,H264_ENC_CFG* cfg)
{
    u8  i;
    u8  err;
    u32 intStat;        
//    H264_ENC_CFG* cfg = &h264_enc_cfg;

#if H264_TEST
#else
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif
    #if MULTI_CHANNEL_SUPPORT
    #if (MULTI_CHANNEL_SEL & 0x01)
    OSSemPend(isuSemEvt, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        isuStop();
        ipuStop();
        siuStop();    
        
        DEBUG_H264("Error: isuSemEvt(video capture mode) is %d.\n", err);
        DEBUG_H264("isu_int_status = 0x%08x,0x%08x\n", isu_int_status,SYS_CTL0);

        //Reset SIU/IPU/ISU
        SYS_RSTCTL  = SYS_RSTCTL | 0x00000058;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00000058); 
        return 1;
    }
    #elif (MULTI_CHANNEL_SEL & 0x02)
    OSSemPend(ciuCapSemEvt_CH1, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        ciu_1_Stop();

        DEBUG_H264("Error: ciuCapSemEvt_CH1(video capture mode) is %d.\n", err);

        //Reset CIU
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        return 1;
    }
    #elif (MULTI_CHANNEL_SEL & 0x04)
    OSSemPend(ciuCapSemEvt_CH2, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        ciu_2_Stop();

        DEBUG_H264("Error: ciuCapSemEvt_CH2(video capture mode) is %d.\n", err);

        //Reset CIU
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        return 1;
    }
    #elif (MULTI_CHANNEL_SEL & 0x08)
    OSSemPend(ciuCapSemEvt_CH3, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        ciu_3_Stop();

        DEBUG_H264("Error: ciuCapSemEvt_CH3(video capture mode) is %d.\n", err);

        //Reset CIU
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        return 1;
    }
    #elif (MULTI_CHANNEL_SEL & 0x10)
    OSSemPend(ciuCapSemEvt_CH4, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        ciu_4_Stop();

        DEBUG_H264("Error: ciuCapSemEvt_CH4(video capture mode) is %d.\n", err);

        //Reset CIU
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        return 1;
    }
    #endif
    #else   // MULTI_CHANNEL_SUPPORT == 0
    OSSemPend(isuSemEvt, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        isuStop();
        ipuStop();
        siuStop();    
        
        DEBUG_H264("Error: isuSemEvt(video capture mode) is %d.\n", err);
        DEBUG_H264("isu_int_status = 0x%08x,0x%08x\n", isu_int_status,SYS_CTL0);

        //Reset SIU/IPU/ISU
        SYS_RSTCTL  = SYS_RSTCTL | 0x00000058;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00000058); 
        return 1;
    }
    #endif  // #if MULTI_CHANNEL_SUPPORT
#endif
    *(info->FrameTime) += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; 
    
    cfg->H264StreamAddr = info->StreamBuf;
    cfg->slice_type     = info->FrameType;
    
    //cfg->frame_num      = info->FrameIdx;
    
    H264ENC_EN_ENC_SWRST = 0x00000000; //reset AVC
    H264ENC_EN_ENC_SWRST = 0x00000000; //reset AVC
    H264ENC_EN_ENC_SWRST = 0x00000000; //reset AVC
    H264ENC_EN_ENC_SWRST = 0x00000001; //enable AVC
    H264ENC_ADDR_SPECIAL_CMD = H264_Special_CMD; //defalut disable special command
    

    if(info->ResetFlag == 1)
	{
	    //printf("new SPS, PPS\n");
		H264Enc_InitCfg(cfg);
        SPS_PPS_Length = GenerateParameterSets_SW(cfg, SPS_PPS_Buffer);
		H264Enc_GenerateParameterSets_HW(cfg);
        info->ResetFlag = 0;        
    }   
    
    H264Enc_SetNALSliceHeader(cfg);
    H264Enc_InitBuf(cfg);
    H264Enc_SetModeDecisionAlgorithm(cfg);

    // Clear IP Interrupt
	H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_CLR_INT; 
    //DEBUG_H264("0x00000018 = 0x%08x\n",H264_ENC_CLR_INT);
    
    // IP start
    //H264Enc_DumpRegister();
    H264ENC_INT_MASK = 0xfffffffb;
    H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_EN_IP;
    //DEBUG_H264("0x00000018 = 0x%08x\n",H264_ENC_EN_IP);    
    
    OSSemPend(VideoCpleSemEvt, H264_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        // reset H264 hardware
        //SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
        //for(i=0;i<10;i++);
        //SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100; 
        
        DEBUG_H264("@@@ Encoder Error: VideoCpleSemEvt is %d.\n", err);
        DEBUG_H264("### H264 encoder error. 0x%08x\n\n\n\n\n",H264ENC_INTERRUPT);
        *(info->pSize) = cfg->H264StreamSize = 0;         
        //DEBUG_H264("VideoPictureIndex = %d\n",VideoPictureIndex);
        //MPEG4_Error = 1;
    }
    else
    {
        //printf("H264ENC_ADDR_SR0 = %d\n",((H264ENC_ADDR_SR0 & 0x0003FFFF) >> 2) << 2);
        //printf("H264ENC_ADDR_TOTAL_BIT = %d\n",((H264ENC_ADDR_TOTAL_BIT) >> 3));
        if(cfg->frame_num == 0)
            cfg->H264StreamSize = ((H264ENC_ADDR_SR0 & 0x0003FFFF) >> 2) << 2;                  
        else
            cfg->H264StreamSize = ((H264ENC_ADDR_TOTAL_BIT) >> 3);

        *(info->pSize) = cfg->H264StreamSize; 
        cfg->frame_num++;  
    }    
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif
    return cfg->H264StreamSize;    
}




//////////////////////////////////////////////////////////
//
// H264 decoder functions
//
//////////////////////////////////////////////////////////
void H264Dec_Init()
{
	u32 status=0;
	
	// clear interrupt, active high reset
    H264DEC_AVC_VSR = 0xFFFFFFFF;
    
    // Interrupt Mask Status
    H264DEC_AVC_IMS = 0xFFFFFFFE;
    DEBUG_H264("1.H264DEC_AVC_IMS = 0x%08x\n",H264DEC_AVC_IMS);
    //Check Parser in Idle status 
    while(status == 0)
    {
      //AVC_DIS
      status = H264DEC_AVC_DIS0;
      status = status & 0x00000001;
    }
}

/*

Routine Description:

    Dump H264 encoder register value.

Arguments:

Return Value:


*/
void H264Dec_DumpRegister(void)
{
    DEBUG_H264("Addr 0x0000 = 0x%08x",H264DEC_AVC_MIB );
    DEBUG_H264("Addr 0x0004 = 0x%08x",H264DEC_AVC_MIO );
    DEBUG_H264("Addr 0x0008 = 0x%08x",H264DEC_AVC_ICB );
    DEBUG_H264("Addr 0x000c = 0x%08x",H264DEC_AVC_ICO );
    DEBUG_H264("Addr 0x001c = 0x%08x",H264DEC_AVC_IPB );
    DEBUG_H264("Addr 0x0010 = 0x%08x",H264DEC_AVC_IPO );
    DEBUG_H264("Addr 0x0014 = 0x%08x",H264DEC_AVC_CFY );
    DEBUG_H264("Addr 0x0018 = 0x%08x",H264DEC_AVC_CFUV);
    DEBUG_H264("Addr 0x001c = 0x%08x",H264DEC_AVC_CFO );
    DEBUG_H264("Addr 0x0038 = 0x%08x",H264DEC_AVC_FCS );
    DEBUG_H264("Addr 0x003c = 0x%08x",H264DEC_AVC_IS  );
    DEBUG_H264("Addr 0x0104 = 0x%08x",H264DEC_AVC_IC  );
    DEBUG_H264("Addr 0x0108 = 0x%08x",H264DEC_AVC_IRS );
    DEBUG_H264("Addr 0x010c = 0x%08x",H264DEC_AVC_IMS );
    DEBUG_H264("Addr 0x0200 = 0x%08x",H264DEC_AVC_VSR );
    DEBUG_H264("Addr 0x0204 = 0x%08x",H264DEC_AVC_SOD );
    DEBUG_H264("Addr 0x0208 = 0x%08x",H264DEC_AVC_SFB );
    DEBUG_H264("Addr 0x0300 = 0x%08x",H264DEC_AVC_BDC );
    DEBUG_H264("Addr 0x0304 = 0x%08x",H264DEC_AVC_BDO );
    DEBUG_H264("Addr 0x0308 = 0x%08x",H264DEC_AVC_UVL );
    DEBUG_H264("Addr 0x030c = 0x%08x",H264DEC_AVC_BCS );
    DEBUG_H264("Addr 0x0310 = 0x%08x",H264DEC_AVC_BSA );
    DEBUG_H264("Addr 0x0314 = 0x%08x",H264DEC_AVC_BSL );
    DEBUG_H264("Addr 0x0328 = 0x%08x",H264DEC_AVC_CBP );
    DEBUG_H264("Addr 0x0400 = 0x%08x",H264DEC_AVC_DIS0);
    DEBUG_H264("Addr 0x0404 = 0x%08x",H264DEC_AVC_DIS1);
    DEBUG_H264("Addr 0x0408 = 0x%08x",H264DEC_AVC_DIS2);
    DEBUG_H264("Addr 0x0500 = 0x%08x",H264DEC_AVC_SL0 );
    DEBUG_H264("Addr 0x0504 = 0x%08x",H264DEC_AVC_SL1 );
    DEBUG_H264("Addr 0x0508 = 0x%08x",H264DEC_AVC_SL2 );
}

s32 H264Dec_InitCfg()
{
#if 0
        u32 PicWidthInPix;
    u32 PicWidthInPix;
    u32 PicWidthInMbs;
    u32 PicWidthInMbs;
    u32 PicSize;  
#endif
    return 1;
}




s32 H264Dec_FetchBitstream(VIDEO_INFO * info, H264_DEC_CFG* cfg)      
{  
	u32 status=0;
#if 0
        u32 PicWidthInPix;
    u32 PicWidthInPix;
    u32 PicWidthInMbs;
    u32 PicWidthInMbs;
                                // **NOTE** : the offset unit is Double-WORD per unit
    u8  *MB_INFO_BASE;          //0x0000     ( (img->width/16)*32*4*(1+active_sps->mb_adaptive_frame_field_flag) + 0xFF)
    u32 MB_INFO_OFFSET;         //0x0004       (img->width/16)*32*4*(1+active_sps->mb_adaptive_frame_field_flag)
    u8  *ILF_COEFF_BASE;        //0x0008     ( img->width*2*(1+active_sps->mb_adaptive_frame_field_flag) + 0xFF)
    u32 ILF_COEFF_OFFSET;       //0x000c       img->width*2*(1+active_sps->mb_adaptive_frame_field_flag)
    u8  *INTRA_PRED_BASE;       //0x0010     ( img->width*2*4*(1+active_sps->mb_adaptive_frame_field_flag) + 0xFF)
    u32 INTRA_PRED_OFFSET;      //0x0014       img->width*2*4*(1+active_sps->mb_adaptive_frame_field_flag)

    //Output Reconstruct frame buffer
    u8  *RecRawYAddr;           //0x0018, Output reconstructed image Y address
    u8  *RecRawUVAddr;          //0x001c, Output reconstructed image UV address

    u8  *StreamAddr;            //0x0310,   Input encoded bitstream base address
    u32 StreamSize;             //0x0314

#endif
    //DEBUG_H264("### 1. start H264Dec_FetchBitstream\n");
	//Input Bitstram
    cfg->H264StreamAddr = info->StreamBuf;
    cfg->H264StreamSize = *(info->pSize);
	H264DEC_AVC_BSA  = (u32)cfg->H264StreamAddr;	
    //DEBUG_H264("Addr 0x0310 0x%08x\n",H264DEC_AVC_BSA );
    //DEBUG_H264("H264StreamSize : %d\n",cfg->H264StreamSize);
	H264DEC_AVC_BSL	= cfg->H264StreamSize+0x000000ff;	
    //DEBUG_H264("Addr 0x0314 0x%08x\n",H264DEC_AVC_BSL );
    //DEBUG_H264("Bddr 0x0314 0x%08x\n",(cfg->H264StreamSize+0x000000ff)&0xffffff80 );


	// Enable IP to fetch the bitstream - AVC_SFB
	H264DEC_AVC_SFB = 0x00000001;
    //DEBUG_H264("Addr 0x0208 0x%08x\n",H264DEC_AVC_SFB );
    
	// check rbsp buffer full already 
	while((status&0x00000020) == 0)
	{		
		status= H264DEC_AVC_BCS;
	}	
    return 1;
}

s32 H264Dec_SetSliceInfo()
{
#if 0
    //
	// write_slice_data() as simple version firmware
	//

  // Sreg0
  write_data = img->PicWidthInMbs;
  write_data = (write_data<<8) | (img->PicHeightInMbs&0x000000FF);  
  write_data = (write_data<<1) | active_sps->frame_mbs_only_flag;
  write_data = (write_data<<1) | img->MbaffFrameFlag;
  write_data = (write_data<<1) | active_sps->direct_8x8_inference_flag;
  write_data = (write_data<<1) | active_pps->entropy_coding_mode_flag;
  write_data = (write_data<<5) | (img->num_ref_idx_l0_active&0x0000001F);
  write_data = (write_data<<5) | (img->num_ref_idx_l1_active&0x0000001F);
  write_data = (write_data<<1) | img->field_pic_flag & 0x01;//dsd.field_pic_flag;
  write_data = (write_data<<1) | img->bottom_field_flag & 0x01; //dsd.bottom_field_flag;
  
  *((volatile unsigned int *)(AHB_SLAVE_BASE+AVC_SL0)) = write_data;
  //DEBUG_H264("SREG0: %x \n", write_data);

  // Sreg1
  //write_data = dec_picture->slice_type & 0x03;
  //Anderson modified
  write_data = img->direct_spatial_mv_pred_flag & 0x01;
  write_data =(write_data<<1) | dec_picture->chroma_format_idc;
  write_data =(write_data<<1) | active_pps->transform_8x8_mode_flag;
  write_data =(write_data<<2) | (img->type & 0x03);
  write_data =(write_data<<5) | (active_pps->second_chroma_qp_index_offset & 0x1F);
  write_data =(write_data<<1) | (active_pps->weighted_pred_flag & 0x01);
  write_data =(write_data<<2) | (active_pps->weighted_bipred_idc& 0x03);
  write_data =(write_data<<6) | (img->qp                & 0x3F);
  write_data =(write_data<<5) | (active_pps->chroma_qp_index_offset & 0x1F);
  write_data =(write_data<<1) | (active_pps->constrained_intra_pred_flag & 0x01);

  *((volatile unsigned int *)(AHB_SLAVE_BASE+AVC_SL1)) = write_data;
  //DEBUG_H264("SREG1: %x \n", write_data);

  // Sreg2
  //Anderson modified
  write_data = img->luma_log2_weight_denom;
  write_data = (write_data<<3) | img->chroma_log2_weight_denom;
  write_data = (write_data<<6);
  
  write_data = (write_data<<1) | use_top_for_frm;   // yiang modify for PAFF
  if(dpb->listXsize[1]==0)
    write_data = (write_data<<2); //| 0;
  else if(listX[LIST_1][0]->structure==FRAME)
    write_data = (write_data<<2)| 0x01; // 2'b01
  else
    write_data = (write_data<<2)| 0x02; // 2'b10
  
  
  //write_data = (write_data<<5);
  //colmb_out_disable: disable col-MB data output when nal_ref_idc == 0 
  // use_top_for_frm  : decided by HW
  //col_pic_is_field = dpb.field_picture[listX1[0]];
  //colpic_coded_frame = dpb.coded_frame[listX1[0]];
  //write_data = (write_data<<1) |dec_picture->coded_frame;
  write_data = (write_data<<5) | img->current_slice_nr;
  write_data = (write_data<<2) | img->model_number; //  & 0x03;
  write_data = (write_data<<2) | (img->currentSlice->LFDisableIdc    & 0x03);
  write_data = (write_data<<4) | (img->currentSlice->LFAlphaC0Offset & 0x0F);
  write_data = (write_data<<4) | (img->currentSlice->LFBetaOffset    & 0x0F);
  *((volatile unsigned int *)(AHB_SLAVE_BASE+AVC_SL2)) = write_data; //0x00000508
  //DEBUG_H264("SREG2: %x \n", write_data);
  #endif

  return 1;
}

/*

Routine Description: 
	parser SPS, PPS, Slice header
    ref JM image.c/read_new_slice()
Arguments:

    

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 H264Dec_ParseHeader()//MP4Dec_Bits* Bits, u32 BitsLen,u8 write2dispbuf_en)
{
	int current_header;
	int ret;
	int BitsUsedByHeader;
	NALU_t nalu;


	while (1) //Lsk TODO : check fail
	{
		// get nal_unit_type
		//DEBUG_H264("### H264 decoder start Get NALU\n");
		ret=GetAnnexbNALU (&nalu);  

		//In some cases, zero_byte shall be present. If current NALU is a VCL NALU, we can't tell
		//whether it is the first VCL NALU at this point, so only non-VCL NAL unit is checked here.
		//CheckZeroByteNonVCL(nalu, &ret);
        //printf("@ nalu->nal_unit_type = %d\n",nalu.nal_unit_type);
		switch (nalu.nal_unit_type)
		{
			//Slice header
		case NALU_TYPE_SLICE:
		case NALU_TYPE_IDR:
			//DEBUG_H264("### H264 decoder start Process Slice header\n");
			BitsUsedByHeader = FirstPartOfSliceHeader();       			
			BitsUsedByHeader = RestOfSliceHeader(&nalu);
			return current_header;		
		case NALU_TYPE_DPA:
		case NALU_TYPE_DPB:
		case NALU_TYPE_DPC:
			DEBUG_H264("@@@ H264 encoder IP not support\n");	
		break;
		case NALU_TYPE_SEI:
			DEBUG_H264("@@@ H264 encoder IP not support\n");	
		break;
		case NALU_TYPE_PPS:
			DEBUG_H264("### H264 decoder start Process PPS\n");
			ProcessPPS(&nalu);
		break;
		case NALU_TYPE_SPS:	
			DEBUG_H264("### H264 decoder start Process SPS\n");
			ProcessSPS(&nalu);
		break;
		case NALU_TYPE_AUD:
			DEBUG_H264("@@@ H264 encoder IP not support\n");
		break;
		case NALU_TYPE_EOSEQ:	    
			DEBUG_H264("@@@ H264 encoder IP not support\n");
		break;
		case NALU_TYPE_EOSTREAM:
			DEBUG_H264("@@@ H264 encoder IP not support\n");
		break;
		case NALU_TYPE_FILL:
			DEBUG_H264("@@@ H264 encoder IP not support\n");
		break;
		default:
		break;  
		}
	}
	return  current_header;
}

s32 H264Dec_InitBuf()
{
	u32 write_data;

    //DEBUG_H264("### 2. start H264Dec_InitBuf\n");
    
	// MB prediction data. 
	write_data = (u32)H264MBPredBuf;
	H264DEC_AVC_MIB = write_data>>3;
	//DEBUG_H264("Addr 0x0000 = 0x%08x\n",H264DEC_AVC_MIB);
	
	write_data = (((active_sps.pic_width_in_mbs_minus1+1)*16*2*4)+0x07) & 0xfffffff8;	
	H264DEC_AVC_MIO = write_data>>3;
	//DEBUG_H264("Addr 0x0004 = 0x%08x\n",H264DEC_AVC_MIO);
	
	// ILF prediction data. 
	write_data = (u32)H264ILFPredBuf;
	H264DEC_AVC_ICB = write_data>>3;  
	//DEBUG_H264("Addr 0x0008 = 0x%08x\n",H264DEC_AVC_ICB);
	
	write_data = (((active_sps.pic_width_in_mbs_minus1+1)*16*2*4)+0x07) & 0xfffffff8;	
	H264DEC_AVC_ICO = write_data>>3;	
	//DEBUG_H264("Addr 0x000c = 0x%08x\n",H264DEC_AVC_ICO );

	// Intra prediction data. 
	write_data = (u32)H264IntraPredBuf;
	H264DEC_AVC_IPB = write_data>>3;
	//DEBUG_H264("Addr 0x0010 = 0x%08x\n",H264DEC_AVC_IPB );
	
	write_data = (((active_sps.pic_width_in_mbs_minus1+1)*16*2)+0x07) & 0xfffffff8;
	H264DEC_AVC_IPO = write_data>>3;  	
	//DEBUG_H264("Addr 0x0014 = 0x%08x\n",H264DEC_AVC_IPO );

    #if 0
	if(active_slice_header.frame_num & 1)
	{
		// Output reconstructed image address	
		write_data = (u32)PNBuf_Y[0];	
		H264DEC_AVC_CFY = write_data>>2;
		//DEBUG_H264("Addr 0x0018 = 0x%08x\n",H264DEC_AVC_CFY );

		write_data = (u32)PNBuf_Y[0]+(active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16;
		H264DEC_AVC_CFUV = write_data>>2;
		//DEBUG_H264("Addr 0x001c = 0x%08x\n",H264DEC_AVC_CFUV);

		write_data = (active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16;
		H264DEC_AVC_CFO = write_data;
		//DEBUG_H264("Addr 0x0038 = 0x%08x\n",H264DEC_AVC_CFO);

		// List0, List1 control (baseline always 3'd0)
		write_data = 0x00000000;
		H264DEC_AVC_FCS = write_data>>2;
		//DEBUG_H264("Addr 0x003c = 0x%08x\n",H264DEC_AVC_FCS);	

		//Reference frame start address
		write_data = (u32)PNBuf_Y[1];
		H264DEC_AVC_FB0 = write_data>>2;
		//DEBUG_H264("Addr 0x0040 = 0x%08x\n",H264DEC_AVC_FB0);	
	}
	else
	{
		// Output reconstructed image address	
		write_data = (u32)PNBuf_Y[1];	
		H264DEC_AVC_CFY = write_data>>2;
		//DEBUG_H264("Addr 0x0018 = 0x%08x\n",H264DEC_AVC_CFY );

		write_data = (u32)PNBuf_Y[1]+(active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16;
		H264DEC_AVC_CFUV = write_data>>2;
		//DEBUG_H264("Addr 0x001c = 0x%08x\n",H264DEC_AVC_CFUV);

		write_data = (active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16;
		H264DEC_AVC_CFO = write_data;
		//DEBUG_H264("Addr 0x0038 = 0x%08x\n",H264DEC_AVC_CFO);

		// List0, List1 control (baseline always 3'd0)
		write_data = 0x00000000;
		H264DEC_AVC_FCS = write_data>>2;
		//DEBUG_H264("Addr 0x003c = 0x%08x\n",H264DEC_AVC_FCS);	

		//Reference frame start address
		write_data = (u32)PNBuf_Y[0];
		H264DEC_AVC_FB0 = write_data>>2;
		//DEBUG_H264("Addr 0x0040 = 0x%08x\n",H264DEC_AVC_FB0);
	}
	#endif
#if (!RFIU_SUPPORT)
    if(DecodeDownSample==1)
    {
        //Output reconstructed image address	
    	write_data = (u32)Sub1Videodisplaybuf[IsuIndex% DISPLAY_BUF_NUM];
        //write_data = (u32)MainVideodisplaybuf[IsuIndex% DISPLAY_BUF_NUM];
    	H264DEC_AVC_CFY= write_data>>2;
    	//DEBUG_H264("Addr 0x0018 = 0x%08x\n",H264DEC_AVC_CFY );

    	//write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM]+(active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16;
    	write_data = (u32)Sub1Videodisplaybuf[IsuIndex % DISPLAY_BUF_NUM]+PNBUF_SIZE_Y;
        //write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM]+PNBUF_SIZE_Y;
        H264DEC_AVC_CFUV= write_data>>2;
    	//DEBUG_H264("Addr 0x001c = 0x%08x\n",H264DEC_AVC_CFUV);
    	
    	write_data = (u32)MainVideodisplaybuf[IsuIndex% DISPLAY_BUF_NUM];	
    	H264DEC_AVC_DSY= write_data>>2;
        //DEBUG_H264("Addr 0x0028 = 0x%08x\n",H264DEC_AVC_DSY );

    	write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM]+PNBUF_SIZE_Y;
    	H264DEC_AVC_DSUV= write_data>>2;
        //DEBUG_H264("Addr 0x002C = 0x%08x\n",H264DEC_AVC_DSUV);
    }
    else
    {
        //Output reconstructed image address	
        write_data = (u32)MainVideodisplaybuf[IsuIndex% DISPLAY_BUF_NUM];
        H264DEC_AVC_CFY= write_data>>2;
        //DEBUG_H264("Addr 0x0018 = 0x%08x\n",H264DEC_AVC_CFY );

        //write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM]+(active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16;
        write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM]+PNBUF_SIZE_Y;
        H264DEC_AVC_CFUV= write_data>>2;
    }
    
	write_data = PNBUF_SIZE_Y;
	H264DEC_AVC_CFO = write_data;
	//DEBUG_H264("Addr 0x0038 = 0x%08x\n",H264DEC_AVC_CFO);

	// List0, List1 control (baseline always 3'd0)
	write_data = 0x00000000;
	H264DEC_AVC_FCS = write_data>>2;
	//DEBUG_H264("Addr 0x003c = 0x%08x\n",H264DEC_AVC_FCS);	

	//Reference frame start address
    if(DecodeDownSample==1)
    {
    	write_data = (u32)Sub1Videodisplaybuf[(IsuIndex + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM];
    	H264DEC_AVC_FB0 = write_data>>2;
    	//DEBUG_H264("Addr 0x0040 = 0x%08x\n",H264DEC_AVC_FB0);	
    }
	else
    {
        write_data = (u32)MainVideodisplaybuf[(IsuIndex + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM];
    	H264DEC_AVC_FB0 = write_data>>2;
    	//DEBUG_H264("Addr 0x0040 = 0x%08x\n",H264DEC_AVC_FB0);	
    }
#endif
	return 0;
}

s32 H264Dec_DecodeOneFrame()
{
	u32 write_data;	
	u8 err;
	u32 intStat;

	H264Dec_InitBuf();

	//Slice information 0
	write_data = (active_sps.pic_width_in_mbs_minus1+1); 
	write_data = (write_data<<8) | ((active_sps.pic_height_in_map_units_minus1+1)&0x000000FF);  
	write_data = (write_data<<1) | active_sps.frame_mbs_only_flag; 
	write_data = (write_data<<1) | 0; //baseline
	write_data = (write_data<<1) | active_sps.direct_8x8_inference_flag;
	write_data = (write_data<<1) | active_pps.entropy_coding_mode_flag;
	write_data = (write_data<<5) | ((active_pps.num_ref_idx_l0_active_minus1+1)&0x0000001F);
	write_data = (write_data<<5) | ((active_pps.num_ref_idx_l1_active_minus1+1)&0x0000001F);
	write_data = (write_data<<1) | (active_slice_header.field_pic_flag & 0x01);
	write_data = (write_data<<1) | (active_slice_header.bottom_field_flag & 0x01);
	H264DEC_AVC_SL0 = write_data;	
	//DEBUG_H264("Addr 0x0500 = 0x%08x\n", H264DEC_AVC_SL0 );

	//Slice information 1
	write_data = 0 & 0x01;//direct_spatial_mv_pred_flag
	write_data =(write_data<<1) | 0;//active_sps.chroma_format_idc;
	write_data =(write_data<<1) | active_pps.transform_8x8_mode_flag;
	write_data =(write_data<<2) | (active_slice_header.slice_type & 0x03);
	write_data =(write_data<<5) | (0 & 0x1F);//second_chroma_qp_index_offset
	write_data =(write_data<<1) | (0 & 0x01);//weighted_pred_flag
	write_data =(write_data<<2) | (0 & 0x03);//weighted_bipred_idc
	write_data =(write_data<<6) | ((active_pps.pic_init_qp_minus26+26+active_slice_header.slice_qp_delta)& 0x3F);
	write_data =(write_data<<5) | (active_pps.chroma_qp_index_offset & 0x1F);
	write_data =(write_data<<1) | (active_pps.constrained_intra_pred_flag & 0x01);
	H264DEC_AVC_SL1 = write_data;
	//DEBUG_H264("Addr 0x0504 = 0x%08x\n", H264DEC_AVC_SL1 );

	//Slice information 2
	write_data = 0x00000000;
	H264DEC_AVC_SL2 = write_data; //0x00000508
	//DEBUG_H264("Addr 0x0508 = 0x%08x\n", H264DEC_AVC_SL2 );
	
	// IP start	
	H264DEC_AVC_SOD = 0x00000001; //any value
	
	if(DecodeDownSample== 1)
        H264DEC_AVC_SOD |= H264_DEC_DOWNSAMPLE_1; //any value

    //DEBUG_H264("Addr 0x0100 = 0x%08x\n", H264DEC_AVC_IS );


	OSSemPend(VideoCpleSemEvt, H264_TIMEOUT, &err);

    if (err != OS_NO_ERR)
    {
        DEBUG_H264("@@@ H264DEC_AVC_DIS0 : 0x%08x\n", H264DEC_AVC_DIS0);
        DEBUG_H264("@@@ H264DEC_AVC_DIS1 : 0x%08x\n", H264DEC_AVC_DIS1);
        DEBUG_H264("@@@ H264DEC_AVC_DIS2 : 0x%08x\n", H264DEC_AVC_DIS2);
        DEBUG_H264("@@@ Decoder Error: VideoCpleSemEvt is %d.\n", err);
    } 

	return 1;
}

s32 H264Dec_DecompressOneFrame(VIDEO_INFO * info, H264_DEC_CFG* cfg)
{
//    H264_DEC_CFG* cfg = &h264_dec_cfg;
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif	
    H264DEC_AVC_VSR = 0x00000000; //reset AVC
    H264DEC_AVC_VSR = 0x00000000; //reset AVC
    H264DEC_AVC_VSR = 0x00000000; //reset AVC

    H264DEC_AVC_VSR = 0x00000001; //enable AVC
    // Interrupt Mask Status
    H264DEC_AVC_IMS = 0xFFFFFFFE;
    //DEBUG_H264("2.H264DEC_AVC_IMS = 0x%08x\n",H264DEC_AVC_IMS);

    if(DecodeLineStripe==1)
        H264DEC_AVC_STRP= 0x00000140;   //Output line stripe offset
    
	H264Dec_FetchBitstream(info, cfg);
	H264Dec_ParseHeader();
	H264Dec_DecodeOneFrame();
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif
	return 1;
}

//////////////////////////////////////////////////////////
//
// H264 test functions
//
//////////////////////////////////////////////////////////

#if H264_TEST
#define FramesNum    6
#define INTRA_PERIOD 30

#define OutputALL   0
s32 H264Enc_LocalTest(H264_ENC_CFG* cfg)//only test RC disable
{
    FS_FILE *in;
    FS_FILE *all, *one;

    int in_pos=0;
    int cnt, tmp, size, i;
    char  fin_name[16];
    char  fout_name[16];
    char  SPS_PPS[64];
    int hdr_size;

    int read_yuv_time=0, copy_yuv_time=0, encode_time=0, write_bitstream_time=0;
    int time_1, time_2;
//    H264_ENC_CFG* cfg = &h264_enc_cfg;
    
    H264Enc_SetResolution(cfg,640, 480);
    dcfChDir("\\DCIM\\100VIDEO");

    VideoCodecInit(); //init VideoBufMng 
    
	for(QP=10; QP<11; QP+=1) 
	{	    		    
        cfg.frame_num = 0;
       	printf("### start QP=%d test ###\n", QP);    		
        
		for(cnt=0; cnt<FramesNum; cnt++)
		{				    
		    if(cnt == 0)
		    {
    		    in = dcfOpen("99999999.yuv", "r+b");
    		    if(in==NULL)
    		        DEBUG_H264("%s : fopen error\n",fin_name);    
		    } 
            
			dcfRead(in, PNBuf_Y[3], ((cfg->pic_width_in_mbs)*(cfg->pic_height_in_map_unit)*128*3), &tmp); //due to SD card cluster equal 512 byte so size must mulptile of 512 byte      
			if(cnt == FramesNum-1)
		        dcfClose(in, &tmp);        
                        
            video_info.StreamBuf = VideoBufMng[0].buffer;
            video_info.FrameType = H264Enc_DecSliceType(cnt,INTRA_PERIOD);
            video_info.FrameIdx  = cnt;  

            if(cnt == 0)
                video_info.ResetFlag = 1;
            else
                video_info.ResetFlag = 0;

            size = H264Enc_CompressOneFrame(&video_info,&cfg);
            
            printf("%03d, ", cnt);
            if(cnt%10==9)
                printf("\n");
            #if OutputALL
            if(cnt == 0)
            {
    			sprintf(fout_name, "11111%03d.264",QP);		    
		        one = dcfOpen(fout_name, "w+b");        
		        if(one==NULL)
		            DEBUG_H264("%s : fopen error\n",fout_name);
            }            
		    dcfWrite(one, video_info.StreamBuf, size, &tmp); 
            if(cnt == FramesNum-1)
		        dcfClose(one, &tmp); 			
            #else
            sprintf(fout_name, "%02d%06d.264",QP, cnt);		    
		    one = dcfOpen(fout_name, "w+b");        
	        if(one==NULL)
	            DEBUG_H264("%s : fopen error\n",fout_name);
		    dcfWrite(one, video_info.StreamBuf, size, &tmp); 
	        dcfClose(one, &tmp); 			
            #endif
		}
        printf("\n### end QP=%d test ###\n\n", QP);    		
	} 
    return 1;
}

s32 H264Dec_LocalTest()//only test RC disable
{
    FS_FILE *in;
    FS_FILE *all, *one;

    u32 file_size;
    u32 in_pos=0;
    u32 cnt, tmp, size, i;
    u8  fin_name[16];
    u8  fout_name[16];


    
    VideoCodecInit(); //init VideoBufMng
    H264Dec_Init();
    dcfChDir("\\DCIM\\100VIDEO");

    //for(QP=29; QP<30; QP++) 
    for(QP=10; QP<11; QP++) 
	{ 
	    //if((QP==4)||(QP==7)||(QP>=20 &&QP<35))
        // continue;
		H264Dec_Init();
		DEBUG_H264("\n\n\n\n\n\n\n\n\n### start QP=%d test ###\n", QP);

        
        for(cnt=0; cnt<FramesNum; cnt++)
		//for(cnt=0; cnt<2; cnt++)
		{
			DEBUG_H264("### start decode frame %03d ###\n\n\n", cnt);
			sprintf(fin_name, "%02d%06d.264",QP, cnt);
		    in = dcfOpen(fin_name, "r+b");
		    if(in==NULL)
		        DEBUG_H264("%s : fopen error\n",fin_name);   
			h264_dec_cfg.H264StreamSize = file_size = in->size;        			    
		    dcfRead(in, VideoBufMng[0].buffer, file_size, &tmp); //due to SD card cluster equal 512 byte so size must mulptile of 512 byte      
		    dcfClose(in, &tmp);    

            video_info.StreamBuf    = VideoBufMng[0].buffer;
            *(video_info.pSize)     = file_size;   
		    size = H264Dec_DecompressOneFrame(&video_info);
            IsuIndex++;
            sprintf(fout_name, "D%02d%05d.yuv",QP, cnt);
            #if OutputALL
            if(cnt == 0)
            {
    			sprintf(fout_name, "11111%03d.yuv",QP);		    
		        one = dcfOpen(fout_name, "w+b");        
		        if(one==NULL)
		            DEBUG_H264("%s : fopen error\n",fout_name);
            }            
            dcfWrite(one, (unsigned char *)(H264DEC_AVC_CFY<<2), ((active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16), &tmp); 		       
            dcfWrite(one, (unsigned char *)(H264DEC_AVC_CFUV<<2), ((active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16)>>1, &tmp); 		                   
            if(cnt == FramesNum-1)
		        dcfClose(one, &tmp); 			
            #else
            one = dcfOpen(fout_name, "w+b");        
            if(one==NULL)
                 DEBUG_H264("%s : fopen error\n",fout_name);
            dcfWrite(one, (unsigned char *)(H264DEC_AVC_CFY<<2), ((active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16), &tmp); 		       
            dcfWrite(one, (unsigned char *)(H264DEC_AVC_CFUV<<2), ((active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16)>>1, &tmp); 		                              
            dcfClose(one, &tmp);  
            #endif
        }        
	}        
    return 1;
}
#endif

//-------------------RF-RX use -------------------------//

/*

Routine Description: 
	parser SPS, PPS, Slice header
    ref JM image.c/read_new_slice()
Arguments:

    

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 rfiuH264Dec_ParseHeader(NALU_t* nalu)//MP4Dec_Bits* Bits, u32 BitsLen,u8 write2dispbuf_en)
{
	int current_header;
	int ret;
	int BitsUsedByHeader;
//	NALU_t nalu;


	while (1) //Lsk TODO : check fail
	{
		// get nal_unit_type
		//DEBUG_H264("### H264 decoder start Get NALU\n");
		ret=GetAnnexbNALU (nalu);  

		//In some cases, zero_byte shall be present. If current NALU is a VCL NALU, we can't tell
		//whether it is the first VCL NALU at this point, so only non-VCL NAL unit is checked here.
		//CheckZeroByteNonVCL(nalu, &ret);
        //printf("@ nalu->nal_unit_type = %d\n",nalu.nal_unit_type);
		switch (nalu->nal_unit_type)
		{
			//Slice header
		case NALU_TYPE_SLICE:
		case NALU_TYPE_IDR:
			//DEBUG_H264("### H264 decoder start Process Slice header\n");
			BitsUsedByHeader = FirstPartOfSliceHeader();       			
			BitsUsedByHeader = RestOfSliceHeader(nalu);
			return current_header;		
		case NALU_TYPE_DPA:
		case NALU_TYPE_DPB:
		case NALU_TYPE_DPC:
			DEBUG_H264("@@@ H264 encoder IP not support\n");	
		break;
		case NALU_TYPE_SEI:
			DEBUG_H264("@@@ H264 encoder IP not support\n");	
		break;
		case NALU_TYPE_PPS:
			DEBUG_H264("### H264 decoder start Process PPS\n");
			ProcessPPS(nalu);
		break;
		case NALU_TYPE_SPS:	
			DEBUG_H264("### H264 decoder start Process SPS\n");
			ProcessSPS(nalu);
		break;
		case NALU_TYPE_AUD:
			DEBUG_H264("@@@ H264 encoder IP not support\n");
		break;
		case NALU_TYPE_EOSEQ:	    
			DEBUG_H264("@@@ H264 encoder IP not support\n");
		break;
		case NALU_TYPE_EOSTREAM:
			DEBUG_H264("@@@ H264 encoder IP not support\n");
		break;
		case NALU_TYPE_FILL:
			DEBUG_H264("@@@ H264 encoder IP not support\n");
		break;
		default:
		break;  
		}
	}
	return  current_header;
}
u32 rfiuH264Encode_I_Header(H264_ENC_CFG* cfg, u32 Width, u32 Height)
{
//    Rfiu_H264_ENC_option* rfiu_cfg = &H264_enc_option;
//    H264_ENC_CFG* cfg = &h264_enc_cfg;
#if 1
    H264Enc_SetResolution(cfg, Width, Height);
    H264Enc_InitCfg(cfg);
    SPS_PPS_Length = Rfiu_GenerateParameterSets_SW(cfg, SPS_PPS_Buffer,&rfiu_nalu);
#else
    rfiu_cfg->pic_width_in_mbs           = Width/16;
	rfiu_cfg->pic_height_in_map_unit     = Height/16;  

    if(Width==320 && Height==240)
        rfiu_cfg->resolution = VIDEO_SIZE_320x240;
    else if(Width==352 && Height==288)
        rfiu_cfg->resolution = VIDEO_SIZE_352x288;
    else if(Width==640 && Height==480)
        rfiu_cfg->resolution = VIDEO_SIZE_640x480;
    else if(Width==704 && Height==480)
        rfiu_cfg->resolution = VIDEO_SIZE_704x480;
    else if(Width==704 && Height==576)
        rfiu_cfg->resolution = VIDEO_SIZE_704x576;
    else if(Width==720 && Height==480)
        rfiu_cfg->resolution = VIDEO_SIZE_720x480;
    else if(Width==720 && Height==576)
        rfiu_cfg->resolution = VIDEO_SIZE_720x576;
    else if(Width==1280 && Height==720)
        rfiu_cfg->resolution = VIDEO_SIZE_1280x720;
    
    //NAL setting
    cfg->nal_ref_idc      = NALU_PRIORITY_HIGHEST;       //0x0008[30:29]
    cfg->nal_unit_type    = NALU_TYPE_IDR;               //0x0008[28:24]
    //SPS setting
    
    if(cfg->pic_width_in_mbs <= 80 && cfg->pic_height_in_map_unit < 64)
        cfg->level_idc    = 30;                          //0x0008[07:00]   
    else
        cfg->level_idc    = 40;                          //0x0008[07:00]
    //cfg->pic_width_in_mbs           = FRAME_WIDTH/16;
	//cfg->pic_height_in_map_unit     = FRAME_HEIGHT/16;        
    //PPS setting
    cfg->chroma_qp_index_offset     = 0;                 //0x0020[31:27]  
    //Slice header setting
    cfg->slice_type                 = I_FRAME;           //0x0020[10] 
    cfg->idr_pic_id                 = 0;                 //0x0008[23:16]
    cfg->frame_num                  = 0;                 //0x000c[31:25]
    //cfg->qp                         = 26;          		 //0x0024[28:23]
    //cfg->slice_qp_delta             = cfg->qp - 26;      //0x000c[07:00]
    cfg->Disable_deblocking_filter_idc = 0;              //0x0020[09:08] 
    cfg->slice_alpha_c0_offset_div2    = 0;              //0x0020[07:04] 
    cfg->slice_beta_offset_div2        = 0;              //0x0020[03:00]  
    SPS_PPS_Length = GenerateParameterSets_SW(rfiu_cfg, SPS_PPS_Buffer);
#endif

    H264Enc_GenerateParameterSets_HW(cfg);
}
u32 rfiuH264Decode_I_Header(H264_ENC_CFG* cfg,H264_DEC_CFG* cfg1)
{
    //H264_ENC_CFG* cfg = &h264_enc_cfg;
    //H264_DEC_CFG* cfg1 = &h264_dec_cfg;
    //NAL setting
    cfg1->nal_ref_idc = cfg->nal_ref_idc;
    cfg1->nal_unit_type = cfg->nal_unit_type;

    //SPS setting
    cfg1->level_idc = cfg->level_idc;
    cfg1->pic_width_in_mbs = cfg->pic_width_in_mbs;
    cfg1->pic_height_in_map_unit = cfg->pic_height_in_map_unit;

    //PPS setting
    cfg1->chroma_qp_index_offset = cfg->chroma_qp_index_offset;       
    cfg1->qp = cfg->qp;       
    rfiuH264Dec_ParseHeader(&rfiu_nalu);
}
u32 rfiuH264Decode(VIDEO_INFO *info,
                        H264_DEC_CFG* cfg,
                          u8* pVopBit, 
                          u32 BitsLen, 
                          int RFUnit,
                          unsigned int Offset,
                          int DispMode,
                          int FieldDecEn)
{
    MP4Dec_Bits Bitstream;
    u32         err;
    u8 *BotFieldBits;
    //H264_DEC_CFG* cfg = &h264_dec_cfg;
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif
    H264DEC_AVC_VSR = 0x00000000; //reset AVC
    H264DEC_AVC_VSR = 0x00000000; //reset AVC
    H264DEC_AVC_VSR = 0x00000000; //reset AVC

    H264DEC_AVC_VSR = 0x00000001; //enable AVC
    // Interrupt Mask Status
    H264DEC_AVC_IMS = 0xFFFFFFFE;
    //DEBUG_H264("2.H264DEC_AVC_IMS = 0x%08x\n",H264DEC_AVC_IMS);

    //if(DecodeLineStripe==1)
    //    H264DEC_AVC_STRP= 0x00000140;   //Output line stripe offset

   	H264Dec_FetchBitstream(info, cfg);
    
    info->Width = cfg->pic_width_in_mbs *16;
	info->Height = cfg->pic_height_in_map_unit *16;  


 	err = rfiuH264Dec_ParseHeader(&rfiu_nalu);
//    err = ParseMPEG4Header(pMp4Dec_opt,&Bitstream, 256); //Lucian 2012/4/26
        
    if(!err) 
	{
        DEBUG_H264("Parse H264 Header error!!!\n");
        info->FrameIdx++;
        //DEBUG_H264("\n======== Mpeg4 Decoding Fatal Error! Reboot!======\n");
        //sysForceWDTtoReboot();
        DEBUG_H264("\n======== H264 Decoding Error! Resync:%d!======\n",RFUnit);
        if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==0)
           sysback_RF_SetEvt(SYS_BACKRF_RFI_FORCERESYNC, RFUnit);
        return  0;
    } 
	else if(err == MPEG4_N_VOP) 
	{
        return  err;
    }

    BotFieldBits=pVopBit + Offset;
    err = rfiuH264Decoding1Frame(info,&Bitstream, 
                                  BitsLen,RFUnit,
                                  DispMode,BotFieldBits,
                                  FieldDecEn);

    if(!err)
    {
        //DEBUG_H264("\n======== Mpeg4 Decoding Fatal Error! Reboot!======\n");
		sysForceWDTtoReboot();
        DEBUG_H264("\n======== H264 Decoding Error! Resync:%d!======\n",RFUnit);
        if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==0)
            sysback_RF_SetEvt(SYS_BACKRF_RFI_FORCERESYNC, RFUnit);
    }
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif    
    return  err;
}
s32 rfiuH264Decoding1Frame(VIDEO_INFO *info,MP4Dec_Bits* Bits, 
	                                 u32 BitsLen,int RFUnit,
	                                 int DispMode,u8 *BotFieldBits,
	                                 int FieldDecEn)
{
    u8  err;
    u32 mbNoSize;   
    u32 pictWidth;  
    u32 MbWidth, MbHeight, MbNum;
	int i,Error;
	u8  *RefBuf_Y, *RefBuf_Cb, *RefBuf_Cr, *McoBuf_Y, *McoBuf_Cb, *McoBuf_Cr;
    u32 VideoOffset=0;
    int DecPos;
    static int MpegErrCnt=0;

	u32 write_data;	
	u32 intStat;
    
    //------------------//
    Error=0;
    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

    SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
    for(i=0;i<5;i++); //delay
    SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);

    // 0x0100
    if(FieldDecEn)
    {
       MbWidth         = (info->Width  + 15) >> 4;
       MbHeight        = (info->Height/2 + 15) >> 4;
       MbNum           = MbWidth * MbHeight;
    }
    else
    {
       MbWidth         = (info->Width  + 15) >> 4;
       MbHeight        = (info->Height + 15) >> 4;
       MbNum           = MbWidth * MbHeight;
    }
	H264Dec_InitBuf();
	//Slice information 0
	write_data = (active_sps.pic_width_in_mbs_minus1+1); 
	write_data = (write_data<<8) | ((active_sps.pic_height_in_map_units_minus1+1)&0x000000FF);  
	write_data = (write_data<<1) | active_sps.frame_mbs_only_flag; 
	write_data = (write_data<<1) | 0; //baseline
	write_data = (write_data<<1) | active_sps.direct_8x8_inference_flag;
	write_data = (write_data<<1) | active_pps.entropy_coding_mode_flag;
	write_data = (write_data<<5) | ((active_pps.num_ref_idx_l0_active_minus1+1)&0x0000001F);
	write_data = (write_data<<5) | ((active_pps.num_ref_idx_l1_active_minus1+1)&0x0000001F);
	write_data = (write_data<<1) | (active_slice_header.field_pic_flag & 0x01);
	write_data = (write_data<<1) | (active_slice_header.bottom_field_flag & 0x01);
	H264DEC_AVC_SL0 = write_data;	
	//DEBUG_H264("Addr 0x0500 = 0x%08x\n", H264DEC_AVC_SL0 );

	//Slice information 1
	write_data = 0 & 0x01;//direct_spatial_mv_pred_flag
	write_data =(write_data<<1) | 0;//active_sps.chroma_format_idc;
	write_data =(write_data<<1) | active_pps.transform_8x8_mode_flag;
	write_data =(write_data<<2) | (active_slice_header.slice_type & 0x03);
	write_data =(write_data<<5) | (0 & 0x1F);//second_chroma_qp_index_offset
	write_data =(write_data<<1) | (0 & 0x01);//weighted_pred_flag
	write_data =(write_data<<2) | (0 & 0x03);//weighted_bipred_idc
	write_data =(write_data<<6) | ((active_pps.pic_init_qp_minus26+26+active_slice_header.slice_qp_delta)& 0x3F);
	write_data =(write_data<<5) | (active_pps.chroma_qp_index_offset & 0x1F);
	write_data =(write_data<<1) | (active_pps.constrained_intra_pred_flag & 0x01);
	H264DEC_AVC_SL1 = write_data;
	//DEBUG_H264("Addr 0x0504 = 0x%08x\n", H264DEC_AVC_SL1 );

	//Slice information 2
	write_data = 0x00000000;
	H264DEC_AVC_SL2 = write_data; //0x00000508
	//DEBUG_H264("Addr 0x0508 = 0x%08x\n", H264DEC_AVC_SL2 );
	
	// IP start	
//	H264DEC_AVC_SOD = 0x00000001; //any value
	
//	if(DecodeDownSample== 1)
//        H264DEC_AVC_SOD |= H264_DEC_DOWNSAMPLE_2; //any value

    //DEBUG_H264("Addr 0x0100 = 0x%08x\n", H264DEC_AVC_IS );
    
    //0x0604  // 0x0608   
    if(DispMode == RFIU_RX_DISP_MAIN) 
    {
       if(info->Height >= 720) //For HD
       {
       #if RFRX_FULLSCR_HD_SINGLE
         VideoOffset=0;//RF_RX_2DISP_WIDTH*((RF_RX_2DISP_HEIGHT-(pMp4Dec_opt->Height/2))/2)*2;
       #else
         VideoOffset=RF_RX_2DISP_WIDTH*((RF_RX_2DISP_HEIGHT-(info->Height/2))/2)*2;
       #endif
       }
       else if(info->Height <= 400) //For QVGA
       {
         VideoOffset= 0;//(RF_RX_2DISP_WIDTH-pMp4Dec_opt->Width)/2*2 + RF_RX_2DISP_WIDTH*((RF_RX_2DISP_HEIGHT-pMp4Dec_opt->Height)/2)*2;
       }
       else //For VGA
       {
         VideoOffset=0;
       }

       H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset;
       H264DEC_AVC_DSUV =(u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset + PNBUF_SIZE_Y;

    }
    else if(DispMode == RFIU_RX_DISP_MASK)
    {
       if(info->Height  >= 720)
       {
         VideoOffset= 800*((RF_RX_2DISP_HEIGHT-(info->Height/2))/2)*2;
       }
       else if(info->Height <= 400) //For QVGA
       {
         VideoOffset= (RF_RX_2DISP_WIDTH-info->Width)/2*2 + 800*((RF_RX_2DISP_HEIGHT-info->Height)/2)*2;
       }
       else //For VGA
       {
         VideoOffset=0;
       }

       H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + VideoOffset;
       H264DEC_AVC_DSUV =(u32)MainVideodisplaybuf[0] + VideoOffset + PNBUF_SIZE_Y;

    }
	else if(DispMode == RFIU_RX_DISP_QUARD)
    {
       if(info->Height >= 720)//For HD
       {
          VideoOffset= RF_RX_2DISP_WIDTH*2*2*((RF_RX_2DISP_HEIGHT-(info->Height/2))/2);
       }
       else if(info->Height <= 400) //For QVGA
       {
       #if UI_GRAPH_QVGA_ENABLE
          VideoOffset=0;
       #else
          if(rfiuRX_OpMode & RFIU_RX_OPMODE_P2P)
             VideoOffset=0;
          else
             VideoOffset=(RF_RX_2DISP_WIDTH-info->Width)/2*2 + RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT-info->Height)/2)*2;
       #endif
       }
       else //For VGA
       {
          VideoOffset=0;
       }
   #if RFRX_HALF_MODE_SUPPORT      
       if(rfiuRX_CamOnOff_Num <= 2)
       {
           if(rfiuRX_CamOnOff_Num==1)
           {
           #if UI_GRAPH_QVGA_ENABLE
               H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
               H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2)+ PNBUF_SIZE_Y;
           #else
               if( (info->Height <= 400) && (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) ) //For QVGA
               {
                    H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
                    H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2)+ PNBUF_SIZE_Y
               }
               else                
                    H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT;
                    H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT + PNBUF_SIZE_Y
           #endif     
           }
           else
           {
               if(rfiuRX_CamOnOff_Sta==0x03)
               {
                   if(RFUnit==0)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else if(rfiuRX_CamOnOff_Sta==0x05)
               {
                   if(RFUnit==0)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else if(rfiuRX_CamOnOff_Sta==0x09)
               {
                   if(RFUnit==0)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else if(rfiuRX_CamOnOff_Sta==0x06)
               {
                   if(RFUnit==1)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else if(rfiuRX_CamOnOff_Sta==0x0a)
               {
                   if(RFUnit==1)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else if(rfiuRX_CamOnOff_Sta==0x0c)
               {
                   if(RFUnit==2)
                     DecPos=0;
                   else
                     DecPos=1;
               }
               else
               {
                   DecPos=0;
               } 
               
            #if UI_GRAPH_QVGA_ENABLE
               switch(DecPos)
               {
                  case 0:
                     H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
                     H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2) + PNBUF_SIZE_Y;
        			 break;

        		  case 1:
                     H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2/2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
                     H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH*2/2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2) + PNBUF_SIZE_Y;

        			 break;
               }
            #else
               if( (info->Height <= 400) && (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) ) //For QVGA
               {
                   switch(DecPos)
                   {
                      case 0:
                         H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
                         H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2)+ PNBUF_SIZE_Y;
            			 break;

            		  case 1:
                         H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH*2/2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2);
                         H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH*2/2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*(RFRX_HALF_MODE_SHIFT/2) + PNBUF_SIZE_Y;
                         break;
                   }
               }
               else
               {
                   switch(DecPos)
                   {
                      case 0:
                         H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT;
                         H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT + PNBUF_SIZE_Y;
                         break;

            		  case 1:
                         H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH*2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT;
                         H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH*2 + VideoOffset + (RF_RX_2DISP_WIDTH*2*2)*RFRX_HALF_MODE_SHIFT + PNBUF_SIZE_Y;
                         break;
                   }
               }
            #endif
            }
       }
       else
    #endif
       {
       #if UI_GRAPH_QVGA_ENABLE
           switch(RFUnit)
           {
              case 0:
                 H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ VideoOffset;
                 H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ VideoOffset + PNBUF_SIZE_Y;
    			 break;

    		  case 1:
                 H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + VideoOffset;
                 H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset + PNBUF_SIZE_Y;
    			 break;

    		  case 2:
                 H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
                 H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset + PNBUF_SIZE_Y;
    			 break;

    		  case 3:
                 H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
                 H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset + PNBUF_SIZE_Y;
    			 break;	 
           }
           
       #else
           if( (info->Height <= 400) && (rfiuRX_OpMode & RFIU_RX_OPMODE_P2P) ) //For QVGA
           {
               switch(RFUnit)
               {
                  case 0:
                     H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ VideoOffset;
                     H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ VideoOffset + PNBUF_SIZE_Y;
        			 break;

        		  case 1:
                     H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + VideoOffset;
                     H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset + PNBUF_SIZE_Y;
        			 break;

        		  case 2:
                     H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
                     H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset + PNBUF_SIZE_Y;
                     break;

        		  case 3:
                     H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
                     H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT/2*RF_RX_2DISP_WIDTH*2*2 + VideoOffset + PNBUF_SIZE_Y;
                     break;	 
               }
           }
           else
           {
               switch(RFUnit)
               {
                  case 0:
                     H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0]+ VideoOffset;
                     H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ VideoOffset + PNBUF_SIZE_Y;
        			 break;

        		  case 1:
                     H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2 + VideoOffset;
                     H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH*2 + VideoOffset + PNBUF_SIZE_Y;
        			 break;

        		  case 2:
                     H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
                     H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2*2 + VideoOffset + PNBUF_SIZE_Y;
                     break;

        		  case 3:
                     H264DEC_AVC_DSY = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH*2 + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2*2 + VideoOffset;
                     H264DEC_AVC_DSUV = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH*2 + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2*2 + VideoOffset + PNBUF_SIZE_Y;
                     break;	 
               }
           }
       #endif
       }
	}
	else if(DispMode == RFIU_RX_DISP_SUB1)
	{
   	   H264DEC_AVC_DSY = (u32)Sub1Videodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM];
       H264DEC_AVC_DSUV = (u32)Sub1Videodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + PNBUF_SIZE_Y;
	}
    
    write_data = PNBUF_SIZE_Y;
	H264DEC_AVC_CFO = write_data;
	//DEBUG_H264("Addr 0x0038 = 0x%08x\n",H264DEC_AVC_CFO);

	// List0, List1 control (baseline always 3'd0)
	write_data = 0x00000000;
	H264DEC_AVC_FCS = write_data>>2;
	//DEBUG_H264("Addr 0x003c = 0x%08x\n",H264DEC_AVC_FCS);	

    RefBuf_Y   = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y;
    RefBuf_Cb  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cb;
    //RefBuf_Cr  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr;
	
    McoBuf_Y   = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
    McoBuf_Cb  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
    //McoBuf_Cr  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
 
    //Exchange address//
	rfiuRxDecBuf[0].mpeg4NRefBuf_Y =RefBuf_Y;
    rfiuRxDecBuf[0].mpeg4NRefBuf_Cb=RefBuf_Cb;
    //rfiuRxDecBuf[0].mpeg4NRefBuf_Cr=RefBuf_Cr;

	rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y =McoBuf_Y;
    rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cb=McoBuf_Cb;
    //rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr=McoBuf_Cr;
    
    /*CY 0907*/
    pictWidth = info->Width;
    H264DEC_AVC_CFY         = (u32) &(RefBuf_Y[0]);

    H264DEC_AVC_CFUV        = (u32) &(RefBuf_Cb[0]);

    H264DEC_AVC_FB0         = (u32) &(McoBuf_Y[0]);
    // 0x0638
//    H264DEC_AVC_DSUV        = (u32) &(McoBuf_Cb[0]);


    // 0x0000
	if( (DispMode == RFIU_RX_DISP_MAIN) )
    {
        if(info->Height >= 720)  //HD
        {
           //Mpeg4SourceStride   = RF_RX_2DISP_WIDTH;
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
		}
		else
		{
		   if(FieldDecEn)
              H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2;
           else
              H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH;
           
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
	    }    
	}
    else if( (DispMode == RFIU_RX_DISP_SUB1))
    {
    #if TV_D1_OUT_FULL
        if(FieldDecEn)
          H264DEC_AVC_STRP   = 704*2;
        else
          H264DEC_AVC_STRP   = 704;

        H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
        //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
    #else
        if(FieldDecEn)
          H264DEC_AVC_STRP   = 640*2;
        else
          H264DEC_AVC_STRP   = 640;

        H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
        //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
    #endif
    }
	else if(DispMode == RFIU_RX_DISP_QUARD)
    {
    #if UI_GRAPH_QVGA_ENABLE
        if(info->Height>= 400)
        {
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_2 | 0x00000001; //any value
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
        }
        else
        {
           H264DEC_AVC_STRP   = RF_RX_2DISP_WIDTH*2;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_DISA;
        }
    #else
        if(info->Height>= 720)
        {
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_2 | 0x00000001; //any value
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
        }
        else
        {
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_DISA;
        }
    #endif
	}
    else if(DispMode == RFIU_RX_DISP_MASK)
    {
        if(info->Height >= 720)  //HD
        {
           H264DEC_AVC_STRP = 800;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
		}
		else
		{
		   if(FieldDecEn)
              H264DEC_AVC_STRP   = 800*2;
           else
              H264DEC_AVC_STRP   = 800;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
	    }    
    }

#if MPEG_DEBUG_ENA_LUCIAN
    gpioSetLevel(0, 1, 1);
#endif


    OSSemPend(VideoCpleSemEvt, MPEG4_TIMEOUT, &err);
    

    if (err != OS_NO_ERR)
    {
        // reset MPEG-4 hardware
        SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
		for(i=0;i<10;i++); //delay
        DEBUG_H264("Decoder Error: VideoCpleSemEvt is %d.\n", err);
        SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);
        
        info->FrameIdx ++;
        Error = 1;

        if(MpegErrCnt > 10)
        {
            DEBUG_H264("H.264 fetal error! Force to Reboot!!\n");
			sysForceWDTtoReboot();
        }
        MpegErrCnt ++;
    }
	else
	{
        info->FrameIdx ++;
        MpegErrCnt=0;
	}
    
    if(MPEG4_Status & 0x04) 
    {
        // reset MPEG-4 hardware
        SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
		for(i=0;i<10;i++); //delay
        DEBUG_H264("H.264 decode frame error!!!\n");
        SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);

        Error = 1;
    }
    //-------Decoding Bottom field------//
    if(FieldDecEn)
    {
        MbWidth         = (info->Width  + 15) >> 4;
        MbHeight        = (info->Height/2 + 15) >> 4;
        MbNum           = MbWidth * MbHeight;

        H264DEC_AVC_DSY         = (u32) &(rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y);

        H264DEC_AVC_DSUV        += (720 * 576);

        pictWidth = info->Width;
        H264DEC_AVC_CFY         += (720 * 576) ;

        H264DEC_AVC_CFUV        += (720 * 576) / 2;

        H264DEC_AVC_FB0         += (720 * 576) + ((720 * 576) / 2);

        //H264DEC_AVC_DSUV        += (720 * 576) / 2;
        if(DispMode == RFIU_RX_DISP_MASK)
        {
           H264DEC_AVC_STRP   = 800*2;
        }
        else
        {
           H264DEC_AVC_STRP   = RF_RX_2DISP_WIDTH*2;
        }
        //Mpeg4Ctrl           = (pMp4Dec_opt->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
        H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
        
        OSSemPend(VideoCpleSemEvt, MPEG4_TIMEOUT, &err);
    
        if (err != OS_NO_ERR)
        {
            // reset MPEG-4 hardware
            SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
    		for(i=0;i<10;i++); //delay
            DEBUG_MP4("Decoder Error: VideoCpleSemEvt is %d.\n", err);
            SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);
            
            Error = 1;
        }
    	
        if(MPEG4_Status & 0x04) 
        {
            // reset MPEG-4 hardware
            SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
    		for(i=0;i<10;i++); //delay
            DEBUG_MP4("H264 decode frame error!!!\n");
            SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);

            Error = 1;
        }
    }
    OSSemPost(mpeg4ReadySemEvt);    // release MPEG4 HW

    if(Error)
        return  MPEG4_ERROR;
    else
        return  MPEG4_NORMAL;   
}

#endif
