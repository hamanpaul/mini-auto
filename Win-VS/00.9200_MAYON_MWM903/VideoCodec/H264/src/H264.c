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
#include "ciuapi.h"
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
#define H264_Skipmode_size 11


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
    VIDEO_SIZE_1920x1072,    //
    VIDEO_SIZE_1920x1088,    //
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
u8 qs_mode[VIDEO_SIZE_END]     = { 0, 0, 1, 1, 1, 1, 1, 2, 2, 2};    
//Inter-IME
#if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5)  || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) ||\
     (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5))
u8 fme_algo[VIDEO_SIZE_END]    = { 0, 0, 0, 0, 0, 0, 0, 2, 2, 2};
#else
u8 fme_algo[VIDEO_SIZE_END]    = { 0, 0, 2, 0, 0, 0, 0, 2, 2, 2};
#endif
u8 stage_1_sr[VIDEO_SIZE_END]  = {16,16,16,16,16,16,16,16,16,16};
u8 don_spl_rat[VIDEO_SIZE_END] = { 4, 4, 5, 5, 5, 5, 5, 5, 5, 5};
u8 me_cand_num[VIDEO_SIZE_END] = { 3, 3, 2, 2, 2, 2, 2, 1, 1, 1};
//Inter-FME
u8 stage_2_sr[VIDEO_SIZE_END]  = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

u8 EncodeLineStripe = 0; //default 320x240
u8 EncodeDownSample = 0;
u8 DecodeLineStripe = 0;//default 320x240
#if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) 
u8 DecodeDownSample = 0;
#else
u8 DecodeDownSample = 1;
#endif

//step 1. LineStripe step 2. downsample
u32 FrameWidth = 640;
u32 LineStripeFrameWidth = 320;
u32 H_addr     = 160;
u32 V_addr     = 120;

u32 H264_IFlag_Index[MAX_RFIU_UNIT];
u32 H264_FrameError[MAX_RFIU_UNIT];
extern u8 rfiu_resetflag[MAX_RFIU_UNIT]; // 0: RF 第一次傳送 , 之後設為1.
#if MULTI_STREAM_SUPPORT
u8 streamtype = 1;  // 0 : small stream, 1: big stream;
#endif
u8 PlaybackTHB_NUM;

//u32 LineStripe_YAddr = FrameWidth * V_addr + H_addr;
//u32 LineStripe_UAddr = FrameWidth*(V_addr/2) + H_addr;
//u32 LineStripeOffset = FrameWidth - LineStripeFrameWidth;
/*
*********************************************************************************************************
* Extern Varaibel
*********************************************************************************************************
*/
extern u32 VideoTimeStatistics;
extern seq_parameter_set_rbsp_t active_sps;
extern pic_parameter_set_rbsp_t active_pps;
extern Bitstream datastream;               //SPS,PPS
extern SLICE_HEADER_t active_slice_header;
extern u32 isu_int_status;
extern u32 IsuIndex;
extern u32 asfCaptureMode;
extern u8  SetIVOP;
extern u32 Cal_FileTime_Start_Idx;
extern u8  H264_stat;
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];
extern OS_EVENT    *mpeg4ReadySemEvt;
extern u8 TvOutMode;
extern u32 H264_Special_CMD;
#if VIDEO_STARTCODE_DEBUG_ENA
extern int monitor_decode[MAX_RFIU_UNIT];
#endif
extern VIDEO_BUF_MNG rfiuRxVideoBufMng[MAX_RFIU_UNIT][VIDEO_BUF_NUM];

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
#if H1_264TEST_ENC
void H264IntHandler(void)
{    
    u32 enc_intStat;
    u32 dec_intStat;
    enc_intStat = H1H264_SWREG1;    //encode
    
    if (enc_intStat & 0x00000004)   // encoder finish   
    {
		H1H264_SWREG1 =enc_intStat;  // write one clear  
        OSSemPost(VideoCpleSemEvt);
    #if MULTI_STREAM_SUPPORT
        if(streamtype == 1)
    #endif
        mp4_avifrmcnt++;        
    } 
    else
    {
		H1H264_SWREG1 =enc_intStat;  // write one clear  
        DEBUG_H264("3.### H264 encoder error. 0x%08x\n\n\n\n\n",enc_intStat);
    }

}
#else
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
    #if H264_DEBUG_ENA
    gpioSetLevel(2, 10, 0);
    #endif  
        H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_CLR_INT; 

        OSSemPost(VideoCpleSemEvt);
    #if MULTI_STREAM_SUPPORT
        if(streamtype == 1)
    #endif
        mp4_avifrmcnt++;        
    } 
    else if(dec_intStat & 0x00000008)   // decoder finish
    {   
        H264DEC_AVC_IC 			 = 0x00000008; 
        if(dec_intStat & 0x00000006)
        {
            H264DEC_AVC_IC 			 = 0x00000006; 
            DEBUG_H264("1.### H264 decoder error. 0x%08x\n\n\n\n\n",dec_intStat);
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
        DEBUG_H264("2.@@@ H264 decoder error. 0x%08x\n\n\n\n\n",dec_intStat);
    }
    else
    {
        DEBUG_H264("3.### H264 encoder error. 0x%08x\n\n\n\n\n",enc_intStat);
        DEBUG_H264("4.@@@ H264 decoder error. 0x%08x\n\n\n\n\n",dec_intStat);
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
#endif

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
    else if(width==640 && height==352)
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
    else if(width==1920 && height==1072)
        cfg->resolution = VIDEO_SIZE_1920x1072;
    else if(width==1920 && height==1088)
        cfg->resolution = VIDEO_SIZE_1920x1088;
    else
    {
        DEBUG_H264("Error resolution %d %d\n",width,height);
        return 0;
    }
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
#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
    (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) ||\
    (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    u32 H264Enc_DecSliceType(H264_ENC_CFG* cfg, u32 frame_idx, u32 period)
{}
#else
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

#endif


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
   #if MULTI_STREAM_SUPPORT
    if(streamtype == 1)
    {
        if(cfg->frame_num == 0)
        {
            cfg->nal_ref_idc   = NALU_PRIORITY_HIGHEST;
            cfg->nal_unit_type = NALU_TYPE_IDR;
            cfg->idr_pic_id    = 0;  
        }   
        else
        {
            cfg->nal_ref_idc   = NALU_PRIORITY_HIGH;
            if(cfg->slice_type == I_FRAME)
            {
            //    cfg->nal_ref_idc   = NALU_PRIORITY_HIGHEST;
                cfg->nal_unit_type = NALU_TYPE_IDR;
                cfg->idr_pic_id    = 0;  
            }
            else
            cfg->nal_unit_type = NALU_TYPE_SLICE;
        }
    }
    else if(streamtype == 0)
    {
        if(cfg->small_frame_num == 0)
        {
            cfg->nal_ref_idc   = NALU_PRIORITY_HIGHEST;
            cfg->nal_unit_type = NALU_TYPE_IDR;
            cfg->idr_pic_id    = 0;  
        }
        else
        {
            cfg->nal_ref_idc   = NALU_PRIORITY_HIGH;
            if(cfg->slice_type == I_FRAME)
            {
            //    cfg->nal_ref_idc   = NALU_PRIORITY_HIGHEST;
                cfg->nal_unit_type = NALU_TYPE_IDR;
                cfg->idr_pic_id    = 0;  
            }
            else
            cfg->nal_unit_type = NALU_TYPE_SLICE;
        }
    }
    #else
    if(cfg->frame_num == 0)
    {
        cfg->nal_ref_idc   = NALU_PRIORITY_HIGHEST;
        cfg->nal_unit_type = NALU_TYPE_IDR;
        cfg->idr_pic_id    = 0;  
    }
    else
    {
        cfg->nal_ref_idc   = NALU_PRIORITY_HIGH;
        if(cfg->slice_type == I_FRAME)
        {
        //    cfg->nal_ref_idc   = NALU_PRIORITY_HIGHEST;
            cfg->nal_unit_type = NALU_TYPE_IDR;
            cfg->idr_pic_id    = 0;  
        }
        else
        cfg->nal_unit_type = NALU_TYPE_SLICE;
    }    
    #endif
    write_data = (cfg->nal_ref_idc << 29)|(cfg->nal_unit_type << 24)|(cfg->idr_pic_id << 16)|(cfg->level_idc);    
    H264ENC_ADDR_SH0 = write_data;
    //DEBUG_H264("0x00000008 = 0x%08x\n",write_data);
    
    //Slice header
    #if MULTI_STREAM_SUPPORT
    if(streamtype == 1)
        write_data = (cfg->slice_qp_delta << 25)|(cfg->frame_num);
    else if(streamtype == 0)
        write_data = (cfg->small_slice_qp_delta << 25)|(cfg->small_frame_num);
    #else
        write_data = (cfg->slice_qp_delta << 25)|(cfg->frame_num);
    #endif
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
    #if 0
    //static int cnt = 1;
    u8 fin_name[32];
    FS_FILE*                pFile;
    //u32 size;
    #endif
    //Local file test, use mpeg4 buffer
    //Input source frame buffer
    #if (MULTI_CHANNEL_SEL & 0x01)
    cfg->CurrRawYAddr  = PNBuf_Y[VideoPictureIndex % 4];
    cfg->CurrRawUAddr  = PNBuf_C[VideoPictureIndex % 4];
    #elif (MULTI_CHANNEL_SEL & 0x02)
      #if HW_DEINTERLACE_CIU1_ENA
        #if MULTI_STREAM_SUPPORT
        if(streamtype == 0)
        {
            cfg->CurrRawYAddr  = PNBuf_sub1[(VideoPictureIndex-1) % 4] + VIDEODISPBUF_OFFSET;
            cfg->CurrRawUAddr  = (PNBuf_sub1[(VideoPictureIndex-1) % 4] + VIDEODISPBUF_OFFSET + PNBUF_SP_SIZE_Y);
        }
        else if (streamtype == 1)
        {
            cfg->CurrRawYAddr  = PNBuf_sub1[(VideoPictureIndex-1) % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub1[(VideoPictureIndex-1) % 4] + PNBUF_SIZE_Y);
        }

        #else
        cfg->CurrRawYAddr  = PNBuf_sub1[(VideoPictureIndex-1) % 4];
        cfg->CurrRawUAddr  = (PNBuf_sub1[(VideoPictureIndex-1) % 4] + PNBUF_SIZE_Y);
        #endif
      #else
        #if MULTI_STREAM_SUPPORT
        if(streamtype == 0)
        {
            cfg->CurrRawYAddr  = PNBuf_sub1[VideoPictureIndex % 4] + VIDEODISPBUF_OFFSET;
            cfg->CurrRawUAddr  = (PNBuf_sub1[VideoPictureIndex % 4] + VIDEODISPBUF_OFFSET + PNBUF_SP_SIZE_Y);
        }
        else if (streamtype == 1)
        {
            cfg->CurrRawYAddr  = PNBuf_sub1[VideoPictureIndex % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub1[VideoPictureIndex % 4] + PNBUF_SIZE_Y);
        }

        #else
        cfg->CurrRawYAddr  = PNBuf_sub1[VideoPictureIndex % 4];
        cfg->CurrRawUAddr  = (PNBuf_sub1[VideoPictureIndex % 4] + PNBUF_SIZE_Y);
        #endif
     #endif
    //printf("CIU BBBB\n");
    #elif (MULTI_CHANNEL_SEL & 0x04)
      #if HW_DEINTERLACE_CIU2_ENA
        #if MULTI_STREAM_SUPPORT
        if(streamtype == 0)
        {
            cfg->CurrRawYAddr  = PNBuf_sub2[(VideoPictureIndex-1) % 4] + VIDEODISPBUF_OFFSET;
            cfg->CurrRawUAddr  = (PNBuf_sub2[(VideoPictureIndex-1) % 4] + VIDEODISPBUF_OFFSET + PNBUF_SP_SIZE_Y);
        }
        else if (streamtype == 1)
        {
            cfg->CurrRawYAddr  = PNBuf_sub2[(VideoPictureIndex-1) % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub2[(VideoPictureIndex-1) % 4] + PNBUF_SIZE_Y);
        }

        #else
        cfg->CurrRawYAddr  = PNBuf_sub2[(VideoPictureIndex-1) % 4];
        cfg->CurrRawUAddr  = (PNBuf_sub2[(VideoPictureIndex-1) % 4] + PNBUF_SIZE_Y);
        #endif
      #else
        #if MULTI_STREAM_SUPPORT
        if(streamtype == 0)
        {
            cfg->CurrRawYAddr  = PNBuf_sub2[VideoPictureIndex % 4] + VIDEODISPBUF_OFFSET;
            cfg->CurrRawUAddr  = (PNBuf_sub2[VideoPictureIndex % 4] + VIDEODISPBUF_OFFSET + PNBUF_SP_SIZE_Y);
        }
        else if (streamtype == 1)
        {
            cfg->CurrRawYAddr  = PNBuf_sub2[VideoPictureIndex % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub2[VideoPictureIndex % 4] + PNBUF_SIZE_Y);
        }

        #else
        cfg->CurrRawYAddr  = PNBuf_sub2[VideoPictureIndex % 4];
        cfg->CurrRawUAddr  = (PNBuf_sub2[VideoPictureIndex % 4] + PNBUF_SIZE_Y);
        #endif
      #endif
    #elif (MULTI_CHANNEL_SEL & 0x20)
      #if HW_DEINTERLACE_CIU5_ENA
        #if MULTI_STREAM_SUPPORT
        if(streamtype == 0)
        {
            cfg->CurrRawYAddr  = PNBuf_sub5[(VideoPictureIndex-1) % 4] + VIDEODISPBUF_OFFSET;
            cfg->CurrRawUAddr  = (PNBuf_sub5[(VideoPictureIndex-1) % 4] + VIDEODISPBUF_OFFSET + PNBUF_SP_SIZE_Y);
        }
        else if (streamtype == 1)
        {
            cfg->CurrRawYAddr  = PNBuf_sub5[(VideoPictureIndex-1) % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub5[(VideoPictureIndex-1) % 4] + PNBUF_SIZE_Y);
        }

        #else
        cfg->CurrRawYAddr  = PNBuf_sub5[(VideoPictureIndex-1) % 4];
        cfg->CurrRawUAddr  = (PNBuf_sub5[(VideoPictureIndex-1) % 4] + PNBUF_SIZE_Y);
        #endif
      #else
        #if MULTI_STREAM_SUPPORT
        if(streamtype == 0)
        {
            cfg->CurrRawYAddr  = PNBuf_sub5[VideoPictureIndex % 4] + VIDEODISPBUF_OFFSET;
            cfg->CurrRawUAddr  = (PNBuf_sub5[VideoPictureIndex % 4] + VIDEODISPBUF_OFFSET + PNBUF_SP_SIZE_Y);
        }
        else if (streamtype == 1)
        {
            cfg->CurrRawYAddr  = PNBuf_sub5[VideoPictureIndex % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub5[VideoPictureIndex % 4] + PNBUF_SIZE_Y);
        }

        #else
        cfg->CurrRawYAddr  = PNBuf_sub5[VideoPictureIndex % 4];
        cfg->CurrRawUAddr  = (PNBuf_sub5[VideoPictureIndex % 4] + PNBUF_SIZE_Y);
        #endif
     #endif
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

    #if MULTI_STREAM_SUPPORT
    if(streamtype == 0)
    {
        if(cfg->small_frame_num & 0x01)
        {
            cfg->RefRawYAddr  = VideoNRefBuf_Y + VIDEODISPBUF_OFFSET;       //0x0110, Input reference image Y addres
            cfg->RefRawUVAddr = VideoNRefBuf_Y + VIDEODISPBUF_OFFSET + PNBUF_SP_SIZE_Y;      //0x0114, Input reference image UV addres  
            cfg->RecRawYAddr  = VideoPRefBuf_Y + VIDEODISPBUF_OFFSET;        //0x0118, Output reconstructed image Y address
            cfg->RecRawUVAddr = VideoPRefBuf_Y + VIDEODISPBUF_OFFSET + PNBUF_SP_SIZE_Y;       //0x0124, Output reconstructed image UV address 
        }
        else
        {
            cfg->RefRawYAddr  = VideoPRefBuf_Y + VIDEODISPBUF_OFFSET;       //0x0110, Input reference image Y addres
            cfg->RefRawUVAddr = VideoPRefBuf_Y + VIDEODISPBUF_OFFSET + PNBUF_SP_SIZE_Y;      //0x0114, Input reference image UV addres  
            cfg->RecRawYAddr  = VideoNRefBuf_Y + VIDEODISPBUF_OFFSET;        //0x0118, Output reconstructed image Y address
            cfg->RecRawUVAddr = VideoNRefBuf_Y + VIDEODISPBUF_OFFSET + PNBUF_SP_SIZE_Y;       //0x0124, Output reconstructed image UV address 
        }
    }
    else if(streamtype == 1)
    {
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
    }
    #else
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
    #endif
      
    
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
    #if MULTI_STREAM_SUPPORT
    if(streamtype == 1)
    {
        if(cfg->qp  >= 12)
    	    cfg->lambda = QP2QUANT[cfg->qp-SHIFT_QP];
    	else
    		cfg->lambda = QP2QUANT[0];
    }
    else if(streamtype == 0)
    {
        if(cfg->small_qp  >= 12)
    	    cfg->lambda = QP2QUANT[cfg->small_qp-SHIFT_QP];
    	else
    		cfg->lambda = QP2QUANT[0];
    }
    #else
    if(cfg->qp  >= 12)
	    cfg->lambda = QP2QUANT[cfg->qp-SHIFT_QP];
	else
		cfg->lambda = QP2QUANT[0];
    #endif
    //cfg->lambda
    cfg->stage_1_sr     = stage_1_sr[cfg->resolution];
    cfg->stage_2_sr     = stage_2_sr[cfg->resolution];
    cfg->don_spl_rat_x  = don_spl_rat[cfg->resolution];
    cfg->don_spl_rat_y  = don_spl_rat[cfg->resolution];
    #if MULTI_STREAM_SUPPORT
    if(streamtype == 1)
        write_data = (cfg->me_cand_num << 29)|(cfg->qp << 23)|(cfg->qs_mode_sel << 7)|(cfg->lambda);
    else if(streamtype == 0)
        write_data = (cfg->me_cand_num << 29)|(cfg->small_qp << 23)|(cfg->qs_mode_sel << 7)|(cfg->lambda);
    #else
    write_data = (cfg->me_cand_num << 29)|(cfg->qp << 23)|(cfg->qs_mode_sel << 7)|(cfg->lambda);
    #endif
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
#if MULTI_STREAM_SUPPORT
    if(streamtype == 1)
        write_data = (cfg->slice_qp_delta << 25) | (cfg->frame_num);
    else if(streamtype == 0)
        write_data = (cfg->small_slice_qp_delta << 25) | (cfg->small_frame_num);
#else
        write_data = (cfg->slice_qp_delta << 25) | (cfg->frame_num);
#endif
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

unsigned char H264_Skipmode_config[H264_Skipmode_size] =    
{
    0x00, 0x00, 0x00, 0x01, 0x41, 0x9A, 
    0x02, 0x00, 0x1F, 0x69, 0x80
};

void H264PutDummyHeader(u32 Width,u32 Height,u8 *pBuf, u32 *byteno,u8 frame_num)
{
    if(frame_num < 0x8)
        H264_Skipmode_config[0x05] = 0x9A;
    else
        H264_Skipmode_config[0x05] = 0x9B;

	H264_Skipmode_config[0x06] = 0x02 | (frame_num<<5);
	
    
    //mb_skip_run
    if((Width == 640) && (Height == 352))
    {
        H264_Skipmode_config[0x08] = 0xDC;    	
        H264_Skipmode_config[0x09] = 0x60;
        H264_Skipmode_config[0x0A] = 0x00; //Lsk: dummy, only 10 bytes
    }
	if((Width == 640) && (Height == 368))
    {
        H264_Skipmode_config[0x08] = 0xE6;    	
        H264_Skipmode_config[0x09] = 0x60;
        H264_Skipmode_config[0x0A] = 0x00; //Lsk: dummy, only 10 bytes
    }
    else if((Width == 1280) && (Height == 720))
    {
        H264_Skipmode_config[0x08] = 0x38;    	
        H264_Skipmode_config[0x09] = 0x46;
        H264_Skipmode_config[0x0A] = 0x00; //Lsk: dummy, only 10 bytes
    }

    else if((Width == 1920) && (Height == 1072))
    {
        H264_Skipmode_config[0x08] = 0x1F;    	
        H264_Skipmode_config[0x09] = 0x69;
        H264_Skipmode_config[0x0A] = 0x80;
    }
	else if((Width == 1920) && (Height == 1088))
    {
        H264_Skipmode_config[0x08] = 0x1F;    	
        H264_Skipmode_config[0x09] = 0xE1;
        H264_Skipmode_config[0x0A] = 0x80;
    }

    *byteno = H264_Skipmode_size;	
    memcpy(pBuf,H264_Skipmode_config,H264_Skipmode_size);    
}

#if H1_264TEST_ENC
/* H.264 intra mode selection favors */
static const u32 h264Intra16Favor[52] = {
    24, 24, 24, 26, 27, 30, 32, 35, 39, 43, 48, 53, 58, 64, 71, 78,
    85, 93, 102, 111, 121, 131, 142, 154, 167, 180, 195, 211, 229,
    248, 271, 296, 326, 361, 404, 457, 523, 607, 714, 852, 1034,
    1272, 1588, 2008, 2568, 3318, 4323, 5672, 7486, 9928, 13216,
    17648
};
static const u32 h264PrevModeFavor[52] = {
    7, 7, 8, 8, 9, 9, 10, 10, 11, 12, 12, 13, 14, 15, 16, 17, 18,
    19, 20, 21, 22, 24, 25, 27, 29, 30, 32, 34, 36, 38, 41, 43, 46,
    49, 51, 55, 58, 61, 65, 69, 73, 78, 82, 87, 93, 98, 104, 110,
    117, 124, 132, 140
};

/* H.264 motion estimation parameters */
static const u32 h264InterFavor[52] = {
    4,   4,   5,   6,   6,   7,   8,   9,  10,  12,  13,  15,  17,  19,  
    21,  24,  26,  30,  34,  38,  42,  48,  53,  60,  68,  76,  85,  96, 
    107, 121, 136, 152, 171, 192, 215, 242, 272, 305, 342, 384, 431, 484, 
    544, 610, 685, 769, 863, 969, 1088, 1221, 1370, 1538
};

/* Penalty factor in 1/256 units for skip mode */
static const u32 h264SkipSadPenalty[52] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 240, 224, 208, 192, 176, 160, 144, 128, 112,
    96,  80,  64,  60,  56,  52,  48,  44,  40,  36,
    32,  28,  24,  20,  18,  16,  16,  15,  15,  14,
    14,  13,  13,  12,  12,  11,  11,  10,  10,  9,
    9, 8
};
/* sqrt(2^((qp-12)/3))*8 */
static const u32 h264DiffMvPenalty[52] =
    { 2, 2, 3, 3, 3, 4, 4, 4, 5, 6,
    6, 7, 8, 9, 10, 11, 13, 14, 16, 18,
    20, 23, 26, 29, 32, 36, 40, 45, 51, 57,
    64, 72, 81, 91, 102, 114, 128, 144, 161, 181,
    203, 228, 256, 287, 323, 362, 406, 456, 512, 575,
    645, 724
};

#define ENCH1_AXI_READ_ID_C0                              1
#define ENCH1_AXI_READ_ID_C1                              2
#define ENCH1_AXI_READ_ID_C2                              3
#ifndef ENCH1_AXI_READ_ID_EN
#define ENCH1_AXI_READ_ID_EN                              1
#endif
void H1_H264Dec_DumpRegister(void)
{
    int i;
    u32 u32addr;
    u32 lastRegAddr = 0x48C;
    for(i = 0; i <= lastRegAddr; i += 4)
    {
        u32addr =H264Enc_CtrlBase +i;
        printf("read Adress: 0x%08X = 0x%08x\n", u32addr, *((volatile unsigned *)(u32addr)));
    }
}

u32 H264Enc_CompressOneFrame(VIDEO_INFO * info,H264_ENC_CFG* cfg,u8 small_type)
{
    u8  err;
    u32 HEncChrOffset           = 0;
    u32 HEncLumOffset           = 0;
    u32 ppsId                   = 0;
    u32 HEncIPPrevModeFavor     = 0x1D;
    u32 sliceSize               = 0;    // H.264 Slice size. mbRowPerSlice (mb rows) [0..127] 0=one slice per picture
    u32 HEncDisableQPMV         = 0;    // H.264 Disable quarter pixel MVs. disableQuarterPixelMv
    u32 HEncTransform8x8        = 0;    // H.264 Transform 8x8 enable. High Profile H.264. transform8x8Mode
    u32 HEncCabacInitIdc        = 0;    // H.264 CABAC initial IDC. [0..2]
    u32 HEncCabacEnable         = 0;    // H.264 CABAC / VP8 boolenc enable. entropyCodingMode. 0=CAVLC (Baseline Profile H.264). 1=CABAC (Main Profile H.264)
    u32 HEncInter4Restrict      = 1;    // H.264 Inter 4x4 mode restriction. restricted4x4Mode
    u32 HEncStreamMode          = 0;    // H.264 Stream mode. byteStream. 0=NAL unit stream. 1=Byte stream
    u32 HEncChromaSwap          = 0;    // H1H264_SWREG19[31] Swap order of chroma bytes in semiplanar input format.
    u32 HEncSplitMv             = 1;    // H1H264_SWREG19[30] Enable using more than 1 MV per macroblock.
    u32 HEncMadThreshold        = 0x10;
    u32 HEncMaxQp               = 0x33;
    u32 HEncMinQp               = 0x05;
    u32 HEncCPDist              = 0;
    u32 colorConversionCoeffA   = 19589;
    u32 colorConversionCoeffB   = 38443;
    u32 colorConversionCoeffC   = 7504;
    u32 colorConversionCoeffE   = 37008;
    u32 colorConversionCoeffF   = 16740;
    u32 HEncIntraAreaLeft       = 255;
    u32 HEncIntraAreaRight      = 255;    
    u32 HEncIntraAreaTop        = 255;    
    u32 HEncIntraAreaBottom     = 255;
    u32 HEncRoi1Left            = 255;
    u32 HEncRoi1Right           = 255;
    u32 HEncRoi1Top             = 255;
    u32 HEncRoi1Bottom          = 255;
    u32 HEncRoi2Left            = 255;
    u32 HEncRoi2Right           = 255;
    u32 HEncRoi2Top             = 255;
    u32 HEncRoi2Bottom          = 255;
    u32 HEncZeroMvFavor         = 10;
    u32 HEncSplitPenalty4x4     = 0;
    u32 HEncMvcViewId           = 1;

    u32 HEncIntTimeout          ;//0x038 [31]
    u32 HEncMvWrite             ;//0x038 [30]
    u32 HEncNalSizeWrite        ;//0x038 [29]
    u32 HEncIntSliceReady       ;//0x038 [28]
    u32 HEncWidth               ;//0x038 [27:19]
    u32 HEncHeight              ;//0x038 [18:10]
    u32 HEncRecWriteBuffer      ;//0x038 [7]
    u32 HEncRecWriteDisable     ;//0x038 [6]
    u32 HEncPictureType         ;//0x038 [4:3]
    u32 HEncEncodingMode        ;//0x038 [2:1]
    u32 HEncEnable              ;//0x038 [0]

    u32 HEncMBComplexityOffset  ;

    u32 test_tmp;
   
    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    #if 1
	H264Enc_InitCfg(cfg);
    printf("1 SPS_PPS %d %x %x \n",SPS_PPS_Length,SPS_PPS_Buffer,&SPS_PPS_Buffer);
    cfg->H264StreamAddr = info->StreamBuf;
    cfg->slice_type     = info->FrameType;
    SPS_PPS_Length = GenerateParameterSets_SW(cfg, SPS_PPS_Buffer, &active_pps, &active_sps, &datastream);
//    memcpy(cfg->H264StreamAddr,SPS_PPS_Buffer,SPS_PPS_Length);    
    #else
    cfg->H264StreamAddr = info->StreamBuf;
    cfg->slice_type     = info->FrameType;
    SPS_PPS_Length =0;
    #endif
    
    cfg->sizeTblBase=sizeTblBase;
    //cfg->CurrRawYAddr  = PNBuf_sub5[(VideoPictureIndex-1) % 4];
    //cfg->CurrRawUAddr  = (PNBuf_sub5[(VideoPictureIndex-1) % 4] + PNBUF_SIZE_Y);
    cfg->CurrRawYAddr  = PNBuf_Y[3];
    cfg->CurrRawUAddr  = (PNBuf_Y[3]+(cfg->pic_width_in_mbs)*(cfg->pic_height_in_map_unit)*256);

    // Base address for output stream data
    H1H264_SWREG5   = (u32)cfg->H264StreamAddr;
//    H1H264_SWREG6   = (u32)cfg->H264StreamAddr;//NAL size?
    H1H264_SWREG6   = (u32)cfg->sizeTblBase;
    printf("H1H264_SWREG6 0x%x\n",H1H264_SWREG6);
    H1H264_SWREG7   = (u32)cfg->RefRawYAddr;
    H1H264_SWREG8   = (u32)cfg->RefRawUVAddr;
    H1H264_SWREG9   = (u32)cfg->RecRawYAddr;
    H1H264_SWREG10  = (u32)cfg->RecRawUVAddr;
    H1H264_SWREG11  = (u32)cfg->CurrRawYAddr;
    H1H264_SWREG12  = (u32)cfg->CurrRawUAddr;
  H1H264_SWREG51  = (u32)H1test1; // Base address for cabac context tables (H264) or probability tables (VP8)
  H1H264_SWREG52  = (u32)H1test2; // Base address for MV output writing

    H1H264_SWREG1   = 0;
    H1H264_SWREG2   = 0x0001d00f;
  H1H264_SWREG3   = 0x01800000; // HW setting
//    H1H264_SWREG13  = (u32)cfg->CurrRawVAddr;
  H1H264_SWREG15  = ((HEncChrOffset << 29) | (HEncLumOffset << 26) | (cfg->pic_width_in_mbs*16 << 12) | (0x01)<<2); // 0x01 input format
    H1H264_SWREG16  = ((cfg->qp << 26) | (cfg->slice_alpha_c0_offset_div2 << 22) | (cfg->slice_beta_offset_div2<< 18) | (cfg->chroma_qp_index_offset << 13) | (cfg->idr_pic_id << 1));
    H1H264_SWREG17  = ((ppsId << 24) | (h264PrevModeFavor[cfg->qp] << 16) | (cfg->frame_num));
    H1H264_SWREG18  = ((cfg->Disable_deblocking_filter_idc << 30) | (sliceSize << 23) | (HEncDisableQPMV << 22) | (HEncTransform8x8 << 21) | (HEncCabacInitIdc << 19) | (HEncCabacEnable << 18) | (HEncInter4Restrict << 17) | (HEncStreamMode << 16) | (h264Intra16Favor[cfg->qp]));
    H1H264_SWREG19  = ((HEncChromaSwap << 31) | (HEncSplitMv << 30));
  H1H264_SWREG20  = 0x00100803;
    H1H264_SWREG21  = ((h264SkipSadPenalty[cfg->qp] << 24) | (h264InterFavor[cfg->qp]) );
    H1H264_SWREG22  = 0;
    H1H264_SWREG23  = 0;
  H1H264_SWREG24  = 0x000a9000; //HW setting Stream buffer limit (64bit addresses) / output stream size (bits). HWStreamDataCount. If limit is reached buffer full IRQ is given.
    H1H264_SWREG25  = ((cfg->slice_qp_delta << 28) | (HEncMadThreshold << 22));
    H1H264_SWREG27  = ((cfg->qp << 26) | (HEncMaxQp << 20) | (HEncMinQp << 14) | HEncCPDist);
    H1H264_SWREG28  = 0;
    H1H264_SWREG29  = 0;
    H1H264_SWREG30  = 0;
    H1H264_SWREG31  = 0;
    H1H264_SWREG32  = 0;
    H1H264_SWREG33  = 0;
    H1H264_SWREG34  = 0;
    H1H264_SWREG35  = (h264DiffMvPenalty[cfg->qp]/2 << 24);
    H1H264_SWREG36 = 0x40000000;
    H1H264_SWREG53  = ((colorConversionCoeffB << 16) | (colorConversionCoeffA));
    H1H264_SWREG54  = ((colorConversionCoeffE << 16) | (colorConversionCoeffC));
    H1H264_SWREG55  = (colorConversionCoeffF); // RGB mode need setting bits 20:30 
    H1H264_SWREG56  = ((HEncIntraAreaLeft << 24) | (HEncIntraAreaRight << 16) | (HEncIntraAreaTop << 8) | (HEncIntraAreaBottom));
    H1H264_SWREG57  = 0;
    H1H264_SWREG58  = 0;
    H1H264_SWREG59  = 0;
    H1H264_SWREG60  = ((HEncRoi1Left << 24) | (HEncRoi1Right << 16) | (HEncRoi1Top << 8) | (HEncRoi1Bottom));
    H1H264_SWREG61  = ((HEncRoi2Left << 24) | (HEncRoi2Right << 16) | (HEncRoi2Top << 8) | (HEncRoi2Bottom));
    H1H264_SWREG62  = ((HEncZeroMvFavor << 28) | (HEncSplitPenalty4x4 << 19) | (HEncMvcViewId << 10));
    H1H264_SWREG237 = 0x08000000;
  H1H264_SWREG238 = 0x04000000;
  
    H1H264_SWREG260 = 0x01D00000;
    H1H264_SWREG261 = 0x000A7000;
    H1H264_SWREG262 = (((h264DiffMvPenalty[cfg->qp]/2)&0xff << 24) | (h264SkipSadPenalty[cfg->qp] << 16) | (h264InterFavor[cfg->qp]));
    H1H264_SWREG264 = ((h264DiffMvPenalty[cfg->qp] << 20));
    H1H264_SWREG272 = 0x01D00000;
    H1H264_SWREG273 = 0x000A7000;
    H1H264_SWREG274 = (((h264DiffMvPenalty[cfg->qp]/2)&0xff << 24) | (h264SkipSadPenalty[cfg->qp] << 16) | (h264InterFavor[cfg->qp]));
    H1H264_SWREG276 = (h264DiffMvPenalty[cfg->qp] << 20);
    H1H264_SWREG284 = 0x01D00000;
    H1H264_SWREG285 = 0x000A7000;
    H1H264_SWREG286 = (((h264DiffMvPenalty[cfg->qp]/2)&0xff << 24) | (h264SkipSadPenalty[cfg->qp] << 16) | (h264InterFavor[cfg->qp]));
    H1H264_SWREG288 = (h264DiffMvPenalty[cfg->qp] << 20);

    if (cfg->slice_type)
        HEncMBComplexityOffset = 17;
    else
        HEncMBComplexityOffset = 15;
    H1H264_SWREG410 = (HEncMBComplexityOffset << 7);
    H1H264_SWREG411 = 0x00000199;
    H1H264_SWREG422 = ((ENCH1_AXI_READ_ID_C0 << 24) | (ENCH1_AXI_READ_ID_C1 << 16) | (ENCH1_AXI_READ_ID_C2 << 8) | (ENCH1_AXI_READ_ID_EN));

    HEncIntTimeout          = 1;
    HEncMvWrite             = 1;
    HEncNalSizeWrite        = 1;
    HEncIntSliceReady       = 0;
    HEncWidth               = cfg->pic_width_in_mbs;
    HEncHeight              = cfg->pic_height_in_map_unit;
    HEncRecWriteBuffer      = 0;
    HEncRecWriteDisable     = 0;
    HEncPictureType         = cfg->slice_type;
    HEncEncodingMode        = 3;
    HEncEnable              = 1;
    H1H264_SWREG40  = 00000000;
    test_tmp  = ( (HEncIntTimeout     << 31) |\
                        (HEncMvWrite        << 30) |\
                        (HEncNalSizeWrite   << 29) |\
                        (HEncIntSliceReady  << 28) |\
                        (HEncWidth          << 19) |\
                        (HEncHeight         << 10) |\
                        (HEncRecWriteBuffer <<  7) |\
                        (HEncRecWriteDisable<<  6) |\
                        (HEncPictureType    <<  3) |\
                        (HEncEncodingMode   <<  1) |\
                        (HEncEnable)
                      );
    //H1_H264Dec_DumpRegister();
    //DEBUG_H264("read Adress: 0xC0150038 = 0x%08x\n",test_tmp);

    H1H264_SWREG14 = test_tmp;

    //printf("@@@ Encoder %d %d %d %s \n",VideoCpleSemEvt->OSEventType,VideoCpleSemEvt->OSEventGrp,VideoCpleSemEvt->OSEventCnt,VideoCpleSemEvt->OSEventName[0]);
    OSSemPend(VideoCpleSemEvt, OS_IPC_WAIT_FOREVER, &err);

    
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
        cfg->H264StreamSize= (H1H264_SWREG24 & 0x7FFFFFFC);                  		
        *(info->pSize) = cfg->H264StreamSize; 
        info->poffset = cfg->poffset = cfg->H264StreamSize;
        cfg->frame_num++;  
    }    


#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif

    OSSemPost(mpeg4ReadySemEvt);
    DEBUG_H264("@@@ Encoder finish \n");
    return cfg->H264StreamSize; 
}
#else
u32 H264Enc_CompressOneFrame(VIDEO_INFO * info,H264_ENC_CFG* cfg,u8 small_type)
{
    u8  err;
    
    #if MULTI_STREAM_SUPPORT
    
    #else
    u32 write_data;
    static int intra_str = 0;
    #endif
    
    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

#if H264_TEST
#else
 #if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
 #endif
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
        /*
        SYS_RSTCTL  = SYS_RSTCTL | 0x00000058;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00000058); 
        */
        SYSReset(SYS_RSTCTL_SIU_RST | SYS_RSTCTL_IPU_RST | SYS_RSTCTL_ISU_RST);
        OSSemPost(mpeg4ReadySemEvt);
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
        /*
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        */
        SYSReset(SYS_RSTCTL_CIU_RST);
        OSSemPost(mpeg4ReadySemEvt);
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
        /*
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        */
        SYSReset(SYS_RSTCTL_CIU2_RST);
        OSSemPost(mpeg4ReadySemEvt);
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
        /*
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        */
        SYSReset_EXT(SYS_CTL0_EXT_CIU3_RST);
        OSSemPost(mpeg4ReadySemEvt);
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
        /*
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        */
        SYSReset_EXT(SYS_CTL0_EXT_CIU4_RST);
        OSSemPost(mpeg4ReadySemEvt);
        return 1;
    }
    #elif (MULTI_CHANNEL_SEL & 0x20)
    OSSemPend(ciuCapSemEvt_CH5, 30, &err);
    if (err != OS_NO_ERR)
    {
        MPEG4_Error = 1;
        ciu_5_Stop();
        ipuStop();
        siuStop();   

        DEBUG_H264("Error: ciuCapSemEvt_CH5(video capture mode) is %d.\n", err);

        //Reset CIU
        /*
        SYS_RSTCTL  = SYS_RSTCTL | 0x00400000;
        for(i=0;i<10;i++);
        SYS_RSTCTL  = SYS_RSTCTL & (~0x00400000); 
        */
        SYSReset(SYS_RSTCTL_SIU_RST | SYS_RSTCTL_IPU_RST | SYS_RSTCTL_ISU_RST);
        OSSemPost(mpeg4ReadySemEvt);
        return 1;
    }
    #endif
#endif

#if MULTI_STREAM_SUPPORT
    
    ///////////// big stream start //////////
    streamtype = 1;    
    H264Enc_SetResolution(cfg, mpeg4Width, mpeg4Height);
    *(info->FrameTime) += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM];
    cfg->H264StreamAddr = info->StreamBuf;
    
    if(info->FrameType == I_FRAME)
        cfg->frame_num = 0;
    cfg->slice_type     = info->FrameType ;
    
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
        SPS_PPS_Length = GenerateParameterSets_SW(cfg, SPS_PPS_Buffer, &active_pps, &active_sps, &datastream);
		H264Enc_GenerateParameterSets_HW(cfg);
        if(mpeg4MultiStreamEnable == 1)
            info->ResetFlag = 1;
        else if(mpeg4MultiStreamEnable == 0)
            info->ResetFlag = 0;
    }   
    //DEBUG_H264("#1 %d %d\n",(cfg->pic_width_in_mbs)*16,(cfg->pic_height_in_map_unit)*16);
    H264Enc_SetNALSliceHeader(cfg);
    H264Enc_InitBuf(cfg);
    H264Enc_SetModeDecisionAlgorithm(cfg);

    // Clear IP Interrupt
	H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_CLR_INT; 
    //DEBUG_H264("0x00000018 = 0x%08x\n",H264_ENC_CLR_INT);
    
    // IP start
    //H264Enc_DumpRegister();
    H264ENC_INT_MASK = 0xfffffffb;
#if H264_DEBUG_ENA
    gpioSetLevel(2, 10, 1);
#endif    
    H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_EN_IP;
    //DEBUG_H264("0x00000018 = 0x%08x\n",H264_ENC_EN_IP);    

    //DEBUG_H264("#1 %d %d %d\n",streamtype,(cfg->pic_width_in_mbs)*16,(cfg->pic_height_in_map_unit)*16);

    OSSemPend(VideoCpleSemEvt, H264_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_H264("@@@ Encoder Error: VideoCpleSemEvt is %d.\n", err);
        DEBUG_H264("### H264 encoder error. 0x%08x\n\n\n\n\n",H264ENC_INTERRUPT);
        *(info->pSize) = cfg->H264StreamSize = 0;         
    }
    else
    {
	        info->poffset= (H264ENC_ADDR_SR0 & 0x7FFFFFFC);        
	        *(info->pSize) = cfg->poffset= info->poffset;
    }  
    cfg->frame_num++;
    ///////////// big stream end //////////
    ///////////// small stream start //////////
    if(small_type == 1)
    {
        streamtype = 0;
        H264Enc_SetResolution(cfg, (SP_MAX_WIDTH) & 0xFF0, (SP_MAX_HEIGHT) & 0xFF0);
        //*(info->Small_FrameTime) += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM];
        cfg->H264StreamAddr = info->StreamBuf + info->poffset;
        cfg->slice_type     = info->FrameType ;
        
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
            SPS_PPS_Length = GenerateParameterSets_SW(cfg, SPS_PPS_Buffer, &active_pps, &active_sps, &datastream);
    		H264Enc_GenerateParameterSets_HW(cfg);
            info->ResetFlag = 0;        
        }   
            //DEBUG_H264("#1 %d %d\n",(cfg->pic_width_in_mbs)*16,(cfg->pic_height_in_map_unit)*16);
        H264Enc_SetNALSliceHeader(cfg);
        H264Enc_InitBuf(cfg);
        H264Enc_SetModeDecisionAlgorithm(cfg);

        // Clear IP Interrupt
    	H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_CLR_INT; 
        //DEBUG_H264("0x00000018 = 0x%08x\n",H264_ENC_CLR_INT);
        
        // IP start
        //H264Enc_DumpRegister();
        H264ENC_INT_MASK = 0xfffffffb;
    #if H264_DEBUG_ENA
        gpioSetLevel(2, 10, 1);
    #endif    
        H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_EN_IP;
        //DEBUG_H264("0x00000018 = 0x%08x\n",H264_ENC_EN_IP);    

        //DEBUG_H264("#1 %d %d %d\n",streamtype,(cfg->pic_width_in_mbs)*16,(cfg->pic_height_in_map_unit)*16);
      
        OSSemPend(VideoCpleSemEvt, H264_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            DEBUG_H264("@@@ Encoder Error: VideoCpleSemEvt is %d.\n", err);
            DEBUG_H264("### H264 encoder error. 0x%08x\n\n\n\n\n",H264ENC_INTERRUPT);
            *(info->pSize) = cfg->H264StreamSize = 0;         
            }
            else
            {
            	cfg->H264StreamSize= (H264ENC_ADDR_SR0 & 0x7FFFFFFC);                  
                cfg->H264StreamSize += info->poffset;
                *(info->pSize) = cfg->H264StreamSize;
        }    
        cfg->small_frame_num++;
    }
    ///////////// small stream end //////////
#else

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
        SPS_PPS_Length = GenerateParameterSets_SW(cfg, SPS_PPS_Buffer, &active_pps, &active_sps, &datastream);
		H264Enc_GenerateParameterSets_HW(cfg);
        info->ResetFlag = 0;        
    }   
//        DEBUG_H264("#2 %d %d\n",info->Height,info->Width);
    H264Enc_SetNALSliceHeader(cfg);
    H264Enc_InitBuf(cfg);
    H264Enc_SetModeDecisionAlgorithm(cfg);
           
    write_data = cfg->pic_width_in_mbs;
    //H264ENC_ADDR_INTRA_COUNT = write_data;
    #if REFRESH_TNTRA_MODE   
    if(cfg->pic_height_in_map_unit == 36)
    {
        if(cfg->frame_num % 4 == 0)
        {
            if(intra_str > (cfg->pic_height_in_map_unit-1))
                intra_str = 0;
            else
                intra_str += 4;
            
            write_data = H264_ENC_I_MB_REFRESH_MODE_EN | H264_ENC_SLICE_SIZE_EQUAL_4 | (intra_str << 8);
            H264ENC_ADDR_INTRA_MB = write_data;
//            printf("QHD %d 0x%x\n",intra_str,H264ENC_ADDR_INTRA_MB);
        }
    }
    else if(cfg->pic_height_in_map_unit == 45)
    {
        if(cfg->frame_num % 2 == 0)
        {
            if(intra_str >= (cfg->pic_height_in_map_unit-1))
                intra_str = 0;
            else
                intra_str += 2;
            
            write_data = H264_ENC_I_MB_REFRESH_MODE_EN | H264_ENC_SLICE_SIZE_EQUAL_2 | (intra_str << 8);
            H264ENC_ADDR_INTRA_MB = write_data;
//            printf("HD %d 0x%x\n",intra_str,H264ENC_ADDR_INTRA_MB);
        }
    }
    #endif
        
    // Clear IP Interrupt
	H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_CLR_INT; 
    //DEBUG_H264("0x00000018 = 0x%08x\n",H264_ENC_CLR_INT);
    
    // IP start
    //H264Enc_DumpRegister();
    H264ENC_INT_MASK = 0xfffffffb;
    #if H264_DEBUG_ENA
    gpioSetLevel(2, 10, 1);
    #endif    
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
        cfg->H264StreamSize= (H264ENC_ADDR_SR0 & 0x7FFFFFFC);                  		
        *(info->pSize) = cfg->H264StreamSize; 
        info->poffset = cfg->poffset = cfg->H264StreamSize;
        cfg->frame_num++;  
    }    

#endif

#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif

    OSSemPost(mpeg4ReadySemEvt);

    return cfg->H264StreamSize;    
}
#endif



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
    DEBUG_H264("Addr 0x0000 = 0x%08x\n",H264DEC_AVC_MIB );
    DEBUG_H264("Addr 0x0004 = 0x%08x\n",H264DEC_AVC_MIO );
    DEBUG_H264("Addr 0x0008 = 0x%08x\n",H264DEC_AVC_ICB );
    DEBUG_H264("Addr 0x000c = 0x%08x\n",H264DEC_AVC_ICO );
    DEBUG_H264("Addr 0x0010 = 0x%08x\n",H264DEC_AVC_IPB );
    DEBUG_H264("Addr 0x0014 = 0x%08x\n",H264DEC_AVC_IPO );
    DEBUG_H264("Addr 0x0018 = 0x%08x\n",H264DEC_AVC_CFY );
    DEBUG_H264("Addr 0x001c = 0x%08x\n",H264DEC_AVC_CFUV);
    DEBUG_H264("Addr 0x0038 = 0x%08x\n",H264DEC_AVC_CFO );
    DEBUG_H264("Addr 0x003c = 0x%08x\n",H264DEC_AVC_FCS );
    DEBUG_H264("Addr 0x0100 = 0x%08x\n",H264DEC_AVC_IS  );
    DEBUG_H264("Addr 0x0104 = 0x%08x\n",H264DEC_AVC_IC  );
    DEBUG_H264("Addr 0x0108 = 0x%08x\n",H264DEC_AVC_IRS );
    DEBUG_H264("Addr 0x010c = 0x%08x\n",H264DEC_AVC_IMS );
    DEBUG_H264("Addr 0x0200 = 0x%08x\n",H264DEC_AVC_VSR );
    DEBUG_H264("Addr 0x0204 = 0x%08x\n",H264DEC_AVC_SOD );
    DEBUG_H264("Addr 0x0208 = 0x%08x\n",H264DEC_AVC_SFB );
    DEBUG_H264("Addr 0x0300 = 0x%08x\n",H264DEC_AVC_BDC );
    DEBUG_H264("Addr 0x0304 = 0x%08x\n",H264DEC_AVC_BDO );
    DEBUG_H264("Addr 0x0308 = 0x%08x\n",H264DEC_AVC_UVL );
    DEBUG_H264("Addr 0x030c = 0x%08x\n",H264DEC_AVC_BCS );
    DEBUG_H264("Addr 0x0310 = 0x%08x\n",H264DEC_AVC_BSA );
    DEBUG_H264("Addr 0x0314 = 0x%08x\n",H264DEC_AVC_BSL );
    DEBUG_H264("Addr 0x0328 = 0x%08x\n",H264DEC_AVC_CBP );
    DEBUG_H264("Addr 0x0400 = 0x%08x\n",H264DEC_AVC_DIS0);
    DEBUG_H264("Addr 0x0404 = 0x%08x\n",H264DEC_AVC_DIS1);
    DEBUG_H264("Addr 0x0408 = 0x%08x\n",H264DEC_AVC_DIS2);
    DEBUG_H264("Addr 0x0500 = 0x%08x\n",H264DEC_AVC_SL0 );
    DEBUG_H264("Addr 0x0504 = 0x%08x\n",H264DEC_AVC_SL1 );
    DEBUG_H264("Addr 0x0508 = 0x%08x\n",H264DEC_AVC_SL2 );
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
	H264DEC_AVC_BSL	= (cfg->H264StreamSize+0x000000ff);
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
	int ret = 1;
	int BitsUsedByHeader = 0;
	NALU_t nalu;

	while (ret) //Lsk TODO : check fail
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
    			return !BitsUsedByHeader;		
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
	return !BitsUsedByHeader;
}

s32 H264Dec_InitBuf()
{
	u32 write_data,height;
    u8  *H264RefBuf_Y, *H264RefBuf_Cb,*H264McoBuf_Y, *H26McoBuf_Cb;
    u32 VideoOffset=0, VideoOffset_uv=0;

	#if ( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
	static int swap_flag=0;
	#endif

//  u32 VideoOffset=0, VideoOffset_uv=0;
//	u32 i;

    //DEBUG_H264("### 2. start H264Dec_InitBuf\n");
    
    height =(active_sps.pic_height_in_map_units_minus1+1)*16;
    
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

    if(DecodeDownSample==1)
    {    	
    	/*******************************************************
		Lsk: HDMI Resolution = 1280x720,
		1. image resolution <= 1280x720, H264_DEC_DOWNSAMPLE_1
		2. image resolution > 1280x720, H264_DEC_DOWNSAMPLE_2
    	*******************************************************/    	
        //Lsk: step1. reconstructed image address
        #if ( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
		if(swap_flag==0)
		{				
			swap_flag = 1;
			H264RefBuf_Y   = MainVideodisplaybuf[DISPLAY_BUF_NUM];
	        H264RefBuf_Cb  = MainVideodisplaybuf[DISPLAY_BUF_NUM]+((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));
	    	
	        H264McoBuf_Y   = MainVideodisplaybuf[DISPLAY_BUF_NUM + 1];
	        H26McoBuf_Cb   = MainVideodisplaybuf[DISPLAY_BUF_NUM + 1]+((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));
		}
		else
		{
			swap_flag = 0;
			H264RefBuf_Y   = MainVideodisplaybuf[DISPLAY_BUF_NUM + 1];
	        H264RefBuf_Cb  = MainVideodisplaybuf[DISPLAY_BUF_NUM + 1]+((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));
	    	
	        H264McoBuf_Y   = MainVideodisplaybuf[DISPLAY_BUF_NUM];
	        H26McoBuf_Cb   = MainVideodisplaybuf[DISPLAY_BUF_NUM]+((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));

		}
		#else
    	H264RefBuf_Y   = mpeg4PRefBuf_Y;
	    H264RefBuf_Cb  = mpeg4PRefBuf_Cb;
	    H264McoBuf_Y   = mpeg4NRefBuf_Y;
	    H26McoBuf_Cb  = mpeg4NRefBuf_Cb;

	    //Exchange address//
	    mpeg4PRefBuf_Y  = H264McoBuf_Y;
	    mpeg4PRefBuf_Cb = H26McoBuf_Cb;
	    mpeg4NRefBuf_Y = H264RefBuf_Y;
	    mpeg4NRefBuf_Cb= H264RefBuf_Cb;
	    #endif

        if(sysTVOutOnFlag)
        {
            if(height > 720)
            {
                if (isCap1920x1080I() == 1) 
                {
                    VideoOffset = (1920)*((1080-536)/2);//1920*180;
                    VideoOffset_uv = VideoOffset/2;//1920*180/2; 
                }
                else
                {
                    VideoOffset = (1280)*((720-536)/2);//1920*180;
                    VideoOffset_uv = VideoOffset/2;
                    //VideoOffset = (1280)*((720-536)/2)+(1280-1024)/8;//1920*180;
                    //VideoOffset_uv = ((1280)*((720-536)/2)+(1280-1024)/4)/2;
                }
            }
        }
        else
        {
            if(sysPlaybackThumbnail == 2)
            {
                if(PlaybackTHB_NUM == 0)
                {
                    VideoOffset =  180 + (RF_RX_2DISP_WIDTH*2)*80;
                    VideoOffset_uv = 180 + (RF_RX_2DISP_WIDTH*2)*80/2;
                }
                else if (PlaybackTHB_NUM == 1)
                {
                    VideoOffset = 180 + (RF_RX_2DISP_WIDTH*2)*160;
                    VideoOffset_uv = 180 + (RF_RX_2DISP_WIDTH*2)*160/2;
                }
                else if (PlaybackTHB_NUM == 2)
                {
                    VideoOffset = 180 + (RF_RX_2DISP_WIDTH*2)*240;
                    VideoOffset_uv = 180 + (RF_RX_2DISP_WIDTH*2)*240/2;
                }
                else if (PlaybackTHB_NUM == 3)
                {
                    VideoOffset = 180 + (RF_RX_2DISP_WIDTH*2)*320;
                    VideoOffset_uv = 180 + (RF_RX_2DISP_WIDTH*2)*320/2;
                }
                else if (PlaybackTHB_NUM == 4)
                {
                    VideoOffset = 180 + (RF_RX_2DISP_WIDTH*2)*400;
                    VideoOffset_uv = 180 + (RF_RX_2DISP_WIDTH*2)*400/2;
                }
            }
        }
        
	    write_data = (u32) & H264RefBuf_Y[0];
	    H264DEC_AVC_CFY         = write_data>>2;
	    write_data = (u32) & H264RefBuf_Cb[0];
	    H264DEC_AVC_CFUV        = write_data>>2;
		write_data = ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));
		H264DEC_AVC_CFO = write_data;

        //Lsk: step2. reference image address			
	    write_data = (u32) & H264McoBuf_Y[0];
	    H264DEC_AVC_FB0         = write_data>>2;
        if(sysPlaybackThumbnail == 2)
        {
    		//Lsk: step3. downsample image address	
    		write_data = (u32)MainVideodisplaybuf[1] + VideoOffset;	
        	H264DEC_AVC_DSY= write_data>>2;
            //DEBUG_H264("Addr 0x0028 = 0x%08x\n",H264DEC_AVC_DSY );

        	write_data = (u32)MainVideodisplaybuf[1] + VideoOffset_uv + PNBUF_SIZE_Y;
        	H264DEC_AVC_DSUV= write_data>>2;
        }
        else
        {
			//Lsk: step3. downsample image address	
			write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + VideoOffset;	
	    	H264DEC_AVC_DSY= write_data>>2;
	        //DEBUG_H264("Addr 0x0028 = 0x%08x\n",H264DEC_AVC_DSY );

	    	write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + VideoOffset_uv + PNBUF_SIZE_Y;
	    	H264DEC_AVC_DSUV= write_data>>2;
        }
        //DEBUG_H264("Addr 0x002C = 0x%08x\n",H264DEC_AVC_DSUV);
        if(sysPlaybackThumbnail == 2)
        {
			write_data = 1024-160;
			H264DEC_AVC_STRP= write_data;   //Output line stripe offset
			H264DEC_AVC_SOD |= H264_DEC_DOWNSAMPLE_4; 			
        }
        else
        {
			if(height > 720)
			{	
				write_data = (RF_RX_2DISP_WIDTH*2);
				H264DEC_AVC_STRP= write_data>>2;   //Output line stripe offset
		        H264DEC_AVC_SOD |= H264_DEC_DOWNSAMPLE_2; 			
			}
		    else 
		    {
		        H264DEC_AVC_SOD |= H264_DEC_DOWNSAMPLE_1; //any value
		    }		
    	}

    }
    else
    {    	    	
    	
   	  	#if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
	  	#if(USE_NEW_MEMORY_MAP)		
		/******************************************************************************
		1920x1080 add upper 1920x180(black), lower 1920x180(black) => 1920x1440
		idu bridge scaler:
		a. horizontal:            1920->1280
		b. vertical(line skip):   1440->720
		******************************************************************************/    		
		//Lsk: step1. reconstructed image address
		//VideoOffset = 1920*180;
		//VideoOffset_uv = 1920*180/2;
        write_data = (u32)MainVideodisplaybuf[IsuIndex% DISPLAY_BUF_NUM] + (1920*180);
        H264DEC_AVC_CFY= write_data>>2;
        //DEBUG_H264("Addr 0x0018 = 0x%08x\n",H264DEC_AVC_CFY );
    
        write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + 1920*1440 + (1920*180/2);
        H264DEC_AVC_CFUV= write_data>>2;
    	//DEBUG_H264("Addr 0x001c = 0x%08x\n",H264DEC_AVC_CFUV);	

		write_data = (1920*1440+(1920*180/2))-(1920*180);
        H264DEC_AVC_CFO = write_data;	

		//Lsk: step2. reference image address			
        write_data = (u32)MainVideodisplaybuf[(IsuIndex + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM]+ (1920*180);
        H264DEC_AVC_FB0 = write_data>>2;    	
	  	#else			  
    	if(height> 720) //1920x1080
		{	
			/******************************************************************************
			1920x1080 add upper 1920x180(black), lower 1920x180(black) => 1920x1440
			idu bridge scaler:
			a. horizontal:            1920->1280
			b. vertical(line skip):   1440->720
			******************************************************************************/    		
			//Lsk: step1. reconstructed image address
//			VideoOffset = 1920*180;
//	        VideoOffset_uv = 1920*180/2;
            write_data = (u32)MainVideodisplaybuf[IsuIndex% DISPLAY_BUF_NUM] + (1920*180);
            H264DEC_AVC_CFY= write_data>>2;
            //DEBUG_H264("Addr 0x0018 = 0x%08x\n",H264DEC_AVC_CFY );
        
            write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + (1920*180/2) + 1920*1440;;
            H264DEC_AVC_CFUV= write_data>>2;
        	//DEBUG_H264("Addr 0x001c = 0x%08x\n",H264DEC_AVC_CFUV);	

    		write_data = (1920*1440+(1920*180/2))-(1920*180);
            H264DEC_AVC_CFO = write_data;	

    		//Lsk: step2. reference image address			
            write_data = (u32)MainVideodisplaybuf[(IsuIndex + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM]+ (1920*180);
            H264DEC_AVC_FB0 = write_data>>2;
    	}
		else
		{
//			VideoOffset = 0;
//	        VideoOffset_uv = 0;	
            write_data = (u32)MainVideodisplaybuf[IsuIndex% DISPLAY_BUF_NUM];
            H264DEC_AVC_CFY= write_data>>2;
            //DEBUG_H264("Addr 0x0018 = 0x%08x\n",H264DEC_AVC_CFY );
        
            write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + 1920*1440;
            H264DEC_AVC_CFUV= write_data>>2;
        	//DEBUG_H264("Addr 0x001c = 0x%08x\n",H264DEC_AVC_CFUV);	

    		write_data = (1920*1440);
            H264DEC_AVC_CFO = write_data;	

    		//Lsk: step2. reference image address			
            write_data = (u32)MainVideodisplaybuf[(IsuIndex + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM];
            H264DEC_AVC_FB0 = write_data>>2;
		}
		#endif
		//Lsk: step3. downsample image address	
		H264DEC_AVC_SOD |= H264_DEC_DOWNSAMPLE_DIS;
      #else		
		//Lsk: step1. reconstructed image address
        write_data = (u32)MainVideodisplaybuf[IsuIndex% DISPLAY_BUF_NUM];
        H264DEC_AVC_CFY= write_data>>2;
        //DEBUG_H264("Addr 0x0018 = 0x%08x\n",H264DEC_AVC_CFY );

        write_data = (u32)MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM] + PNBUF_SIZE_Y;
        H264DEC_AVC_CFUV= write_data>>2;
    	//DEBUG_H264("Addr 0x001c = 0x%08x\n",H264DEC_AVC_CFUV);	

		write_data = PNBUF_SIZE_Y;
		H264DEC_AVC_CFO = write_data;
		//DEBUG_H264("Addr 0x0038 = 0x%08x\n",H264DEC_AVC_CFO);
		
		//Lsk: step2. reference image address	
		write_data = (u32)MainVideodisplaybuf[(IsuIndex + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM];
    	H264DEC_AVC_FB0 = write_data>>2;

		//Lsk: step3. downsample image address	
		H264DEC_AVC_SOD |= H264_DEC_DOWNSAMPLE_DIS;		
	  #endif			
    }
    
	

	// List0, List1 control (baseline always 3'd0)
	write_data = 0x00000000;
	H264DEC_AVC_FCS = write_data>>2;
	//DEBUG_H264("Addr 0x003c = 0x%08x\n",H264DEC_AVC_FCS);	
	
	return 0;
}

s32 H264Dec_DecodeOneFrame(VIDEO_INFO * info)
{
	u32 write_data;	
	u8 err;
	u8 i,j;
    u32 VideoOffset=0, VideoOffset_uv=0;
    u32 VideoOffset1=0, VideoOffset1_uv=0;
	H264Dec_InitBuf();
    //height =(active_sps.pic_height_in_map_units_minus1+1)*16;
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
    #if H1_264TEST
	write_data =(write_data<<5) | (active_pps.second_chroma_qp_index_offset & 0x1F);
    #else
	write_data =(write_data<<5) | (0 & 0x1F);//second_chroma_qp_index_offset
	#endif
	write_data =(write_data<<1) | (0 & 0x01);//weighted_pred_flag
	write_data =(write_data<<2) | (0 & 0x03);//weighted_bipred_idc
	write_data =(write_data<<6) | ((active_pps.pic_init_qp_minus26+26+active_slice_header.slice_qp_delta)& 0x3F);
	write_data =(write_data<<5) | (active_pps.chroma_qp_index_offset & 0x1F);
	write_data =(write_data<<1) | (active_pps.constrained_intra_pred_flag & 0x01);
	H264DEC_AVC_SL1 = write_data;
	//DEBUG_H264("Addr 0x0504 = 0x%08x\n", H264DEC_AVC_SL1 );

    //Slice information 2
    #if H1_264TEST
	write_data =(active_slice_header.LFDisableIdc & 0x03);
	write_data =(write_data<<4) | (active_slice_header.LFAlphaC0Offset & 0xF);
	write_data =(write_data<<4) | (active_slice_header.LFBetaOffset & 0xF) ;
	H264DEC_AVC_SL2 = write_data; //0x00000508
    #else	
	write_data = 0x00000000;
	H264DEC_AVC_SL2 = write_data; //0x00000508
	#endif
	//DEBUG_H264("Addr 0x0508 = 0x%08x\n", H264DEC_AVC_SL2 );

#if H264_DEBUG_ENA
    gpioSetLevel(2, 10, 0);
#endif

	//H264Dec_DumpRegister();
	// IP start	
	H264DEC_AVC_SOD |= 0x00000001; //any value
	//DEBUG_H264("Addr H264DEC_AVC_SOD = 0x%08x\n", H264DEC_AVC_SOD );
	//DEBUG_H264("#### %d %d \n",height,TvOutMode);        
    //DEBUG_H264("Addr 0x0100 = 0x%08x\n", H264DEC_AVC_IS );

	OSSemPend(VideoCpleSemEvt, H264_TIMEOUT, &err);
#if H264_DEBUG_ENA
    gpioSetLevel(2, 10, 1);
#endif
    if (err != OS_NO_ERR)
    {
    	// reset MPEG-4 hardware
        SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
		for(i=0;i<10;i++); //delay		
        SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);
        
        DEBUG_H264("@@@ H264DEC_AVC_DIS0 : 0x%08x\n", H264DEC_AVC_DIS0);
        DEBUG_H264("@@@ H264DEC_AVC_DIS1 : 0x%08x\n", H264DEC_AVC_DIS1);
        DEBUG_H264("@@@ H264DEC_AVC_DIS2 : 0x%08x\n", H264DEC_AVC_DIS2);
        DEBUG_H264("@@@ Decoder Error: VideoCpleSemEvt is %d.\n", err);
		DEBUG_H264("Playback Decoder Error: VideoCpleSemEvt is %d, Size:<%d, %d>, Type:<%d, %d>.\n", err, info->poffset, *(info->pSize), info->FrameType, info->Small_FrameType);
		VideoPictureIndex++;
        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
    } 
    if(sysPlaybackThumbnail == 2)
    {
        if(PlaybackTHB_NUM == 0)
        {
            VideoOffset =  180 + (RF_RX_2DISP_WIDTH*2)*80;
            VideoOffset_uv = 180 + (RF_RX_2DISP_WIDTH*2)*80/2;
            VideoOffset1 =  60 + (RF_RX_2DISP_WIDTH*2)*150;
            VideoOffset1_uv = 60 + (RF_RX_2DISP_WIDTH*2)*150/2;            
            for(i=10;i<78;i++)
                memcpy_hw(MainVideodisplaybuf[0]+VideoOffset1+(1024*i)-512, MainVideodisplaybuf[1]+VideoOffset+(1024*i), 160);
            for(i=5;i<39;i++)
                memcpy_hw(MainVideodisplaybuf[0]+VideoOffset1_uv+PNBUF_SIZE_Y+(1024*i)-768, 
                    MainVideodisplaybuf[1]+ VideoOffset_uv+PNBUF_SIZE_Y+(1024*i), 160);

        }
        else if (PlaybackTHB_NUM == 1)
        {
            VideoOffset = 180 + (RF_RX_2DISP_WIDTH*2)*160;
            VideoOffset_uv = 180 + (RF_RX_2DISP_WIDTH*2)*160/2;
            VideoOffset1 =  60 + (RF_RX_2DISP_WIDTH*2)*220;
            VideoOffset1_uv = 60 + (RF_RX_2DISP_WIDTH*2)*220/2;            
            for(i=10;i<78;i++)
                memcpy_hw(MainVideodisplaybuf[0]+VideoOffset1+(1024*i), MainVideodisplaybuf[1]+VideoOffset+(1024*i), 160);
            for(i=5;i<39;i++)
                memcpy_hw(MainVideodisplaybuf[0]+VideoOffset1_uv+PNBUF_SIZE_Y+(1024*i)-512, 
                    MainVideodisplaybuf[1]+ VideoOffset_uv+PNBUF_SIZE_Y+(1024*i), 160);

        }
        else if (PlaybackTHB_NUM == 2)
        {
            VideoOffset = 180 + (RF_RX_2DISP_WIDTH*2)*240;
            VideoOffset_uv = 180 + (RF_RX_2DISP_WIDTH*2)*240/2;
            VideoOffset1 =  60 + (RF_RX_2DISP_WIDTH*2)*290;
            VideoOffset1_uv = 60 + (RF_RX_2DISP_WIDTH*2)*290/2;            
            for(i=10;i<78;i++)
                memcpy_hw(MainVideodisplaybuf[0]+VideoOffset1+(1024*i)-512, MainVideodisplaybuf[1]+VideoOffset+(1024*i), 160);
            for(i=5;i<39;i++)
                memcpy_hw(MainVideodisplaybuf[0]+VideoOffset1_uv+PNBUF_SIZE_Y+(1024*i)-256, 
                    MainVideodisplaybuf[1]+ VideoOffset_uv+PNBUF_SIZE_Y+(1024*i), 160);
        }
        else if (PlaybackTHB_NUM == 3)
        {
            VideoOffset = 180 + (RF_RX_2DISP_WIDTH*2)*320;
            VideoOffset_uv = 180 + (RF_RX_2DISP_WIDTH*2)*320/2;
            VideoOffset1 =  60 + (RF_RX_2DISP_WIDTH*2)*360;
            VideoOffset1_uv = 60 + (RF_RX_2DISP_WIDTH*2)*360/2;            
            for(i=10;i<78;i++)
                memcpy_hw(MainVideodisplaybuf[0]+VideoOffset1+(1024*i), MainVideodisplaybuf[1]+VideoOffset+(1024*i), 160);
            for(i=5;i<39;i++)
                memcpy_hw(MainVideodisplaybuf[0]+VideoOffset1_uv+PNBUF_SIZE_Y+(1024*i), 
                    MainVideodisplaybuf[1]+ VideoOffset_uv+PNBUF_SIZE_Y+(1024*i), 160);
        }
    }	
	return 1;
}

u32 H264Dec_DecompressOneFrame(VIDEO_INFO * info, H264_DEC_CFG* cfg)
{
    u8 err;
//    H264_DEC_CFG* cfg = &h264_dec_cfg;

    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

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
    
    if((*(info->StreamBuf) == 0x00) && (*(info->StreamBuf+1) == 0x00) 
        && (*(info->StreamBuf+2) == 0x00) && (*(info->StreamBuf+3) == 0x01))
    {
        err = H264Dec_ParseHeader();
    }
    else
    {
        DEBUG_H264("Playback Parse H264 Header Error!\n\n\n");
        err = 0;
    }
    
    if(!err)
    {
        DEBUG_H264("\n========Playback Parse H264 Header Error! Resync!======\n");
        
      #if DINAMICALLY_POWER_MANAGEMENT
        sysMPEG_disable();
      #endif
        OSSemPost(mpeg4ReadySemEvt);
        return  err;
    }
    
    H264Dec_DecodeOneFrame(info);
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif

#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
    (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) ||\
    (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))

#else
    if((sysCameraMode == SYS_CAMERA_MODE_PLAYBACK) && (sysTVOutOnFlag == SYS_OUTMODE_PANEL) && (BRI_SCCTRL_MODE & 0x4))
    {
        iduTVColorbar_onoff(0); 
    }
#endif    
    OSSemPost(mpeg4ReadySemEvt);

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
    u8  tmp;
	
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

            size = H264Enc_CompressOneFrame(&video_info,&cfg,0);
            
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
    u8  tmp;


    
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

#if H1_264TEST
#define FramesNum    6
#define INTRA_PERIOD 30

H264_ENC_CFG h264_enc_cfg;
H264_DEC_CFG h264_dec_cfg;

s32 H1_H264Dec_LocalTest()
{
    FS_FILE *in;
    FS_FILE *all, *one;

    u32 file_size;
    u32 in_pos=0;
    u32 cnt, size, i;
    u8  fin_name[16];
    u8  fout_name[16];
    u8  tmp;
    u32 tmp1;


#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif	
    VideoCodecInit(); //init VideoBufMng
    H264Dec_Init();
    dcfChDir("\\DCIM\\100VIDEO");

    //for(QP=29; QP<30; QP++) 
	{ 
		H264Dec_Init();
		DEBUG_H264("\n\n\n\n\n\n\n\n\n### start QP=%d test ###\n", QP);
        
        for(cnt=0; cnt<FramesNum; cnt++)
		{
			DEBUG_H264("### start decode frame %03d ###\n\n\n", cnt);
			sprintf(fin_name, "D00%05d.264",cnt);
		    in = dcfOpen(fin_name, "r+b");
		    if(in==NULL)
		        DEBUG_H264("%s : fopen error\n",fin_name);   
			h264_dec_cfg.H264StreamSize = file_size = in->size;        			    
		    dcfRead(in, VideoBufMng[0].buffer, file_size, &tmp1); //due to SD card cluster equal 512 byte so size must mulptile of 512 byte      
		    dcfClose(in, &tmp);    

            video_info.StreamBuf    = VideoBufMng[0].buffer;
            *(video_info.pSize)     = file_size;   
		    size = H264Dec_DecompressOneFrame(&video_info,&h264_dec_cfg);
            sprintf(fout_name, "D%02d%05d.yuv",QP, cnt);

            one = dcfOpen(fout_name, "w+b");        
            if(one==NULL)
                 DEBUG_H264("%s : fopen error\n",fout_name);
//            dcfWrite(one, (unsigned char *)(H264DEC_AVC_CFY<<2), ((active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16), &tmp1); 		       
//            dcfWrite(one, (unsigned char *)(H264DEC_AVC_CFUV<<2), ((active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16)>>1, &tmp1); 		                              
            dcfWrite(one, MainVideodisplaybuf[(IsuIndex) % DISPLAY_BUF_NUM], ((active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16), &tmp1); 		                              
            dcfWrite(one, MainVideodisplaybuf[(IsuIndex) % DISPLAY_BUF_NUM]+PNBUF_SIZE_Y, ((active_sps.pic_width_in_mbs_minus1+1)*16*(active_sps.pic_height_in_map_units_minus1+1)*16)>>1, &tmp1); 		                              
            dcfClose(one, &tmp);  
            IsuIndex++;
        }        
	}        
    return 1;
}
#endif
#if H1_264TEST_ENC
#define FramesNum    1
#define INTRA_PERIOD 30

H264_ENC_CFG h264_enc_cfg;
H264_DEC_CFG h264_dec_cfg;

s32 H1_H264Enc_LocalTest()
{
    FS_FILE *in;
    FS_FILE *all, *one;

    int in_pos=0;
    int size, i;
    char  fin_name[16];
    char  fout_name[16];
    char  SPS_PPS[64];
    int hdr_size;

    int read_yuv_time=0, copy_yuv_time=0, encode_time=0, write_bitstream_time=0;
    int time_1, time_2;
    u32 cnt;
    u8  tmp;
    u32 tmp1;

    H264_ENC_CFG* cfg = &h264_enc_cfg;
    
    H264Enc_SetResolution(cfg,640, 480);
    dcfChDir("\\DCIM\\100VIDEO");

    VideoCodecInit(); //init VideoBufMng
    VideoCpleSemEvt     = OSSemCreate(0);
    for(QP=10; QP<11; QP+=1)
    {
        cfg->frame_num = 0;
           printf("### start QP=%d test ###\n", QP);
        
        for(cnt=0; cnt<FramesNum; cnt++)
        {
            if(cnt == 0)
            {
                in = dcfOpen("99999999.yuv", "r+b");
                if(in==NULL)
                {
                    DEBUG_H264("%s : fopen error\n",fin_name);
                    return 0;
                }
            }
            
            dcfRead(in, PNBuf_Y[3], ((cfg->pic_width_in_mbs)*(cfg->pic_height_in_map_unit)*128*3), &tmp1); //due to SD card cluster equal 512 byte so size must mulptile of 512 byte
            if(cnt == FramesNum-1)
                dcfClose(in, &tmp);
            
            video_info.StreamBuf = VideoBufMng[0].buffer;
            video_info.FrameType = H264Enc_DecSliceType(cfg,cnt,INTRA_PERIOD);
            video_info.FrameIdx  = cnt;
            
            if(cnt == 0)
                video_info.ResetFlag = 1;
            else
                video_info.ResetFlag = 0;
            
            size = H264Enc_CompressOneFrame(&video_info,cfg,0);
            
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
            dcfWrite(one, video_info.StreamBuf, size, &tmp1);
            if(cnt == FramesNum-1)
                dcfClose(one, &tmp);
            
            #else
            sprintf(fout_name, "%02d%06d.264",QP, cnt);
            one = dcfOpen(fout_name, "w+b");
            if(one==NULL)
                DEBUG_H264("%s : fopen error\n",fout_name);
            //dcfWrite(one, SPS_PPS_Buffer, SPS_PPS_Length, &tmp1);
            dcfWrite(one, video_info.StreamBuf, size, &tmp1);
            dcfClose(one, &tmp);
            
            #endif
        }
        printf("\n### end QP=%d test ###\n\n", QP);
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
s32 rfiuH264Dec_ParseHeader()//MP4Dec_Bits* Bits, u32 BitsLen,u8 write2dispbuf_en)
{
	int ret = 1;
	int BitsUsedByHeader = 0;
	NALU_t nalu;

	while (ret) //Lsk TODO : check fail
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
    			return !BitsUsedByHeader;		
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
    			//ProcessPPS(&nalu);
    		break;
    		case NALU_TYPE_SPS:	
    			DEBUG_H264("### H264 decoder start Process SPS\n");
    			//ProcessSPS(&nalu);
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
	return  !BitsUsedByHeader;
}
void rfiuH264Encode_I_Header(H264_ENC_CFG* cfg, u32 Width, u32 Height)
{
    H264Enc_SetResolution(cfg, Width, Height);
    H264Enc_InitCfg(cfg);
    
}
void rfiuH264Decode_I_Header(H264_ENC_CFG* cfg,H264_DEC_CFG* cfg1)
{
    u32 i;

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
    
/// sps
    active_sps.profile_idc = BASELINE_PROFILE; //CCU H264 IP only support baseline profile
    active_sps.constrained_set0_flag = 0;
    active_sps.constrained_set1_flag = 0;
    active_sps.constrained_set2_flag = 0;
    active_sps.constrained_set3_flag = 0;
    active_sps.seq_parameter_set_id = 0;  // Parameter Set ID hard coded to zero

    if( (active_sps.profile_idc == 100)||(active_sps.profile_idc == 110)||(active_sps.profile_idc == 122) || (active_sps.profile_idc == 144) )
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
        
    active_sps.log2_max_frame_num_minus4 = 0;             
    active_sps.pic_order_cnt_type = 2;                    
    
    if(active_sps.pic_order_cnt_type == 0)
        active_sps.log2_max_pic_order_cnt_lsb_minus4 = 0; 
    else if(active_sps.pic_order_cnt_type == 1)
    {
        active_sps.delta_pic_order_always_zero_flag = 0;  
        active_sps.offset_for_non_ref_pic = 0;            
        active_sps.offset_for_top_to_bottom_field = 0;        
        active_sps.num_ref_frames_in_pic_order_cnt_cycle = 0; 
        for (i=0; i<active_sps.num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            active_sps.offset_for_ref_frame[i] = 0;
        }        
    }
        
    active_sps.num_ref_frames = 1; 
    active_sps.gaps_in_frame_num_value_allowed_flag = 0;
    active_sps.frame_mbs_only_flag = 1;

    if(!active_sps.frame_mbs_only_flag)
        active_sps.mb_adaptive_frame_field_flag = 0;
    
    active_sps.direct_8x8_inference_flag = 1;
    
    if(cfg->resolution == VIDEO_SIZE_1920x1088)
        active_sps.frame_cropping_flag = TRUE;    
    else
        active_sps.frame_cropping_flag = FALSE;

    if(active_sps.frame_cropping_flag == TRUE)        
    {
        active_sps.frame_cropping_rect_left_offset = 0;
        active_sps.frame_cropping_rect_top_offset = 0;
        active_sps.frame_cropping_rect_right_offset = 0;
        active_sps.frame_cropping_rect_bottom_offset = 4;        
    }
    
    active_sps.vui_parameters_present_flag = FALSE;
    if(active_sps.vui_parameters_present_flag == TRUE)
    {
        //TO DO : CCU H264 IP don't support
    }

///// pps
    active_pps.pic_parameter_set_id = 0;
    active_pps.seq_parameter_set_id = active_sps.seq_parameter_set_id;
    active_pps.entropy_coding_mode_flag = 0; //CAVLC, CABAC
    active_pps.pic_order_present_flag = 0;
    active_pps.num_slice_groups_minus1 = 0;
    active_pps.num_ref_idx_l0_active_minus1 = active_sps.frame_mbs_only_flag ? (active_sps.num_ref_frames-1) : (2 * active_sps.num_ref_frames - 1) ;   // set defaults
    active_pps.num_ref_idx_l1_active_minus1 = active_sps.frame_mbs_only_flag ? (active_sps.num_ref_frames-1) : (2 * active_sps.num_ref_frames - 1) ;   // set defaults
    active_pps.weighted_pred_flag = 0;
    active_pps.weighted_bipred_idc = 0;
    active_pps.pic_init_qp_minus26 = 0;         // hard coded to zero, QP lives in the slice header
    active_pps.pic_init_qs_minus26 = 0;
    active_pps.chroma_qp_index_offset = 0; 
    active_pps.deblocking_filter_control_present_flag = 0; //Lsk note
    active_pps.constrained_intra_pred_flag = 0;
    active_pps.redundant_pic_cnt_present_flag = 0;
    
}

u32 rfiuH264Decode(VIDEO_INFO *info,
                   H264_DEC_CFG* cfg,
                   u8* pVopBit, 
                   u32 BitsLen, 
                   int RFUnit,
                   unsigned int Offset,
                   int DispMode,
                   int FieldDecEn,
                   int QuadDispOut)
{
    MP4Dec_Bits Bitstream;
    u32         err;
    u8 *BotFieldBits,err1;
    

    //H264_DEC_CFG* cfg = &h264_dec_cfg;
    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err1);
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif

    active_sps.level_idc    = cfg->level_idc;
    active_sps.pic_width_in_mbs_minus1 = (cfg->pic_width_in_mbs-1);
    active_sps.pic_height_in_map_units_minus1 = (cfg->pic_height_in_map_unit-1);;
    
    H264DEC_AVC_VSR = 0x00000000; //reset AVC
    H264DEC_AVC_VSR = 0x00000000; //reset AVC
    H264DEC_AVC_VSR = 0x00000000; //reset AVC

    H264DEC_AVC_VSR = 0x00000001; //enable AVC
    // Interrupt Mask Status
    H264DEC_AVC_IMS = 0xFFFFFFFE;
    //DEBUG_H264("2.H264DEC_AVC_IMS = 0x%08x\n",H264DEC_AVC_IMS);

   	H264Dec_FetchBitstream(info, cfg);

    if((*(info->StreamBuf) == 0x00) && (*(info->StreamBuf+1) == 0x00) 
        && (*(info->StreamBuf+2) == 0x00) && (*(info->StreamBuf+3) == 0x01))
    {
        err = H264Dec_ParseHeader();
    }
    else
    {
        DEBUG_H264("Parse H264 Header Error!\n\n\n");
        err = 0;
    }
    if(!err)
    {
        DEBUG_H264("\n========Parse H264 Header Error! Resync:%d!======\n",RFUnit);
        
        if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==0)
            sysback_RF_SetEvt(SYS_BACKRF_RFI_FORCERESYNC, RFUnit);
      #if DINAMICALLY_POWER_MANAGEMENT
        sysMPEG_disable();
      #endif
        OSSemPost(mpeg4ReadySemEvt);
        return  err;
    }
    
    info->Width = cfg->pic_width_in_mbs *16;
    info->Height = cfg->pic_height_in_map_unit *16;  
    
    BotFieldBits=pVopBit + Offset;
    err = rfiuH264Decoding1Frame(info,&Bitstream, 
                                  BitsLen,RFUnit,
                                  DispMode,BotFieldBits,
                                  FieldDecEn,QuadDispOut);
    rfiu_resetflag[RFUnit] = 1;
    if(!err)
    {
        //DEBUG_H264("\n======== Mpeg4 Decoding Fatal Error! Reboot!======\n");
        //sysForceWDTtoReboot();
        DEBUG_H264("\n======== H264 Decoding Error! Resync:%d!======\n",RFUnit);
        if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==0)
            sysback_RF_SetEvt(SYS_BACKRF_RFI_FORCERESYNC, RFUnit);
    }
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif    
    OSSemPost(mpeg4ReadySemEvt);
    return  err;
}

u32 H264DecodeLastFrame(VIDEO_INFO *info,
                   H264_DEC_CFG* cfg,
                   u8* pVopBit, 
                   u32 BitsLen, 
                   int RFUnit,
                   unsigned int Offset,
                   int DispMode,
                   int FieldDecEn,
                   int QuadDispOut)
{
    MP4Dec_Bits Bitstream;
    u32         err;
    u8 *BotFieldBits,err1;
    

    //H264_DEC_CFG* cfg = &h264_dec_cfg;
    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err1);
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif

    active_sps.level_idc    = cfg->level_idc;
    active_sps.pic_width_in_mbs_minus1 = (cfg->pic_width_in_mbs-1);
    active_sps.pic_height_in_map_units_minus1 = (cfg->pic_height_in_map_unit-1);;
    
    H264DEC_AVC_VSR = 0x00000000; //reset AVC
    H264DEC_AVC_VSR = 0x00000000; //reset AVC
    H264DEC_AVC_VSR = 0x00000000; //reset AVC

    H264DEC_AVC_VSR = 0x00000001; //enable AVC
    // Interrupt Mask Status
    H264DEC_AVC_IMS = 0xFFFFFFFE;
    //DEBUG_H264("2.H264DEC_AVC_IMS = 0x%08x\n",H264DEC_AVC_IMS);

   	H264Dec_FetchBitstream(info, cfg);

    if((*(info->StreamBuf) == 0x00) && (*(info->StreamBuf+1) == 0x00) 
        && (*(info->StreamBuf+2) == 0x00) && (*(info->StreamBuf+3) == 0x01))
    {
        err = H264Dec_ParseHeader();
    }
    else
    {
        DEBUG_H264("Parse H264 Header Error!\n\n\n");
        err = 0;
    }
    if(!err)
    {
        DEBUG_H264("\n========Parse H264 Last Frame Header Error! Resync:%d!======\n",RFUnit);
        
      #if DINAMICALLY_POWER_MANAGEMENT
        sysMPEG_disable();
      #endif
        OSSemPost(mpeg4ReadySemEvt);
        return  err;
    }
    
    info->Width = cfg->pic_width_in_mbs *16;
    info->Height = cfg->pic_height_in_map_unit *16;  
    
    BotFieldBits=pVopBit + Offset;
    err = rfiuH264Decoding1Frame(info,&Bitstream, 
                                  BitsLen,RFUnit,
                                  DispMode,BotFieldBits,
                                  FieldDecEn,QuadDispOut);
    rfiu_resetflag[RFUnit] = 1;
    if(!err)
    {
        DEBUG_H264("\n======== H264 Decoding Last Frame Error! Resync:%d======\n",RFUnit);
#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
    (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) ||\
    (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )

#else
        Idu_ClearBuf(6);
#endif
        H264_FrameError[RFUnit] = 1;
    }
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif    
    OSSemPost(mpeg4ReadySemEvt);
    return  err;
}

s32 rfiuH264Decoding1Frame(VIDEO_INFO *info,MP4Dec_Bits* Bits, 
	                                 u32 BitsLen,int RFUnit,
	                                 int DispMode,u8 *BotFieldBits,
	                                 int FieldDecEn,int QuadDispOut)
{
    u8  err;
//    u32 pictWidth;  
//    u32 MbNum;
	int i,Error;
	u8  *RefBuf_Y, *RefBuf_Cb, *McoBuf_Y, *McoBuf_Cb;
    u32 VideoOffset=0, VideoOffset_uv=0;
    int DecPos;
    static int H264ErrCnt=0;

	u32 write_data;

	#if ( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
	static int swap_flag=0;
	#endif
//	u32 intStat;
    
    //------------------//
    Error=0;
    //SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
    //for(i=0;i<5;i++); //delay
    //SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);

    // 0x0100
//    if(FieldDecEn)
//    {
//       MbWidth         = (info->Width  + 15) >> 4;
//       MbHeight        = (info->Height/2 + 15) >> 4;
//       MbNum           = ((info->Width  + 15) >> 4) * ((info->Height/2 + 15) >> 4);
//    }
//    else
//    {
//       MbWidth         = (info->Width  + 15) >> 4;
//       MbHeight        = (info->Height + 15) >> 4;
//       MbNum           = ((info->Width  + 15) >> 4) * ((info->Height + 15) >> 4);
//    }
	//H264Dec_InitBuf();
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
    if(DispMode == RFIU_RX_DISP_MAIN_VGA) 
    {
       if(info->Height >= 720) //For HD
       {
       #if RFRX_FULLSCR_HD_SINGLE
         VideoOffset=0;
       #else
         //VideoOffset=RF_RX_2DISP_WIDTH*((RF_RX_2DISP_HEIGHT-(info->Height/2))/2);
         VideoOffset=0;
       #endif
       }
       else if(info->Height <= 400) //For QVGA
       {
         VideoOffset= 0;
       }
       else //For VGA
       {
         VideoOffset=0;
       }
       write_data = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset;
       H264DEC_AVC_DSY= write_data>>2;
       write_data =(u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset/2 + PNBUF_SIZE_Y;
       H264DEC_AVC_DSUV= write_data>>2;

    }
    else if(DispMode == RFIU_RX_DISP_MAIN_HD) 
    {
       if(info->Height  >= 860)
       {
       #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
         VideoOffset = 0;//1920*180;
         VideoOffset_uv = 0;//1920*180/2; 
       #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
         if(sysTVOutOnFlag)
         {
            if(isCap1920x1080I() == 1) 
            {
                VideoOffset = (1920)*((1080-536)/2);//1920*180;
                VideoOffset_uv = VideoOffset/2;//1920*180/2; 
            }
            else
            {
                VideoOffset = (1280)*((720-536)/2);//1920*180;
                VideoOffset_uv = VideoOffset/2;
                //VideoOffset = (1280)*((720-536)/2)+(1280-1024)/8;//1920*180;
                //VideoOffset_uv = ((1280)*((720-536)/2)+(1280-1024)/4)/2;
            }
         }
         else
         {
            VideoOffset = 0;//1920*180;
            VideoOffset_uv = 0;//1920*180/2; 
         }
       #else
         VideoOffset = 0;
         VideoOffset_uv = 0; 
       #endif
       }
       else if(info->Height  >= 720)
       {
         VideoOffset = 0;
         VideoOffset_uv = 0; 
       }
       else
       {
         VideoOffset = 0;
         VideoOffset_uv = 0; 
       }
        
       write_data = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset;
       H264DEC_AVC_DSY= write_data>>2;
       write_data =(u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset_uv + PNBUF_SIZE_Y;
       H264DEC_AVC_DSUV= write_data>>2;
    }
    else if(DispMode == RFIU_RX_DISP_PIP) 
    {
        VideoOffset = 0;
        VideoOffset_uv = 0; 
        
        write_data = (u32)PKBuf_PIP0Y+ VideoOffset;
        H264DEC_AVC_DSY= write_data>>2;
        write_data = (u32)PKBuf_PIP0CbCr;
        H264DEC_AVC_DSUV= write_data>>2;
    }
    else if(DispMode == RFIU_RX_DISP_MASK)
    {
       if(sysTVOutOnFlag)
       {
           if(info->Height >= 860)
           {
              VideoOffset = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height/2))/2) + 0;
              VideoOffset_uv = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height/2))/4) + 0; 
           }
           else if(info->Height  >= 720)
           {
              VideoOffset = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height/2))/2) + (960-(info->Width/2))/2;
              VideoOffset_uv = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height/2))/4) + (960-(info->Width/2))/2; 
           }
           else
           {
              #if (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)
              VideoOffset = RF_RX_2DISP_WIDTH*2*(200) + 464;
              VideoOffset_uv = RF_RX_2DISP_WIDTH*2*(200/2) + 464; 
              #else
              VideoOffset = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height))/2) + (960-(info->Width))/2;
              VideoOffset_uv = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height))/4) + (960-(info->Width))/2; 
              #endif
           }
       }
       else
       {
       #if(LCM_OPTION == VGA_1024X768_60HZ)
           if(info->Height >= 860)
           {
              VideoOffset = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height/2))/2) + 0;
              VideoOffset_uv = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height/2))/4) + 0; 
           }
           else if(info->Height  >= 720)
           {
              VideoOffset = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height/2))/2) + (1024-(info->Width/2))/2;
              VideoOffset_uv = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height/2))/4) + (1024-(info->Width/2))/2; 
           }
           else
           {
              VideoOffset = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height))/2) + (1024-(info->Width))/2;
              VideoOffset_uv = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT*2-(info->Height))/4) + (1024-(info->Width))/2; 
           }
       #else
           if(info->Height >= 860)
           {
              VideoOffset = RF_RX_2DISP_WIDTH*2*((600-(info->Height/2))/2) + 0;
              VideoOffset_uv = RF_RX_2DISP_WIDTH*2*((600-(info->Height/2))/4) + 0; 
           }
           else if(info->Height  >= 720)
           {
              VideoOffset = RF_RX_2DISP_WIDTH*2*((600-(info->Height/2))/2) + (1024-(info->Width/2))/2;
              VideoOffset_uv = RF_RX_2DISP_WIDTH*2*((600-(info->Height/2))/4) + (1024-(info->Width/2))/2; 
           }
           else
           {
              VideoOffset = RF_RX_2DISP_WIDTH*2*((600-(info->Height))/2) + (1024-(info->Width))/2;
              VideoOffset_uv = RF_RX_2DISP_WIDTH*2*((600-(info->Height))/4) + (1024-(info->Width))/2; 
           }

       #endif
       }
       
	   #if((SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) && USE_NEW_MEMORY_MAP)
	   write_data = (u32)MainVideodisplaybuf[0] + VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
       H264DEC_AVC_DSY= write_data>>2;
       write_data = (u32)MainVideodisplaybuf[0] + VideoOffset_uv + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
       H264DEC_AVC_DSUV= write_data>>2;
	   #else
       write_data = (u32)MainVideodisplaybuf[0] + VideoOffset;
       H264DEC_AVC_DSY= write_data>>2;
       write_data = (u32)MainVideodisplaybuf[0] + VideoOffset_uv + PNBUF_SIZE_Y;
       H264DEC_AVC_DSUV= write_data>>2;
	   #endif
    }
	else if(DispMode == RFIU_RX_DISP_QUARD_HD)
    {
       if(info->Height >= 860) //for FHD
       {
          VideoOffset = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT-(info->Height/4))/2)+ (RF_RX_2DISP_WIDTH*2 - (info->Width/2))/4;
          VideoOffset_uv = RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT-(info->Height/4))/4)+ (RF_RX_2DISP_WIDTH*2 - (info->Width/2))/4;
       }
       else if(info->Height >= 720)//For HD
       {
          VideoOffset= RF_RX_2DISP_WIDTH*2*((RF_RX_2DISP_HEIGHT-(info->Height/2))/2);
       }
       else if(info->Height == 352) //For QHD
       {
          VideoOffset = RF_RX_2DISP_WIDTH*2*8;
          VideoOffset_uv = RF_RX_2DISP_WIDTH*2*4;
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
                write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2)*RFRX_HALF_MODE_SHIFT;
                H264DEC_AVC_DSY= write_data>>2;
                write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2)*RFRX_HALF_MODE_SHIFT/2 + PNBUF_SIZE_Y;
                H264DEC_AVC_DSUV= write_data>>2;
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

               if( info->Height >= 860)
               { 
                   switch(DecPos)
                   {
                      case 0:
                         write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2)*RFRX_HALF_MODE_SHIFT;
                         H264DEC_AVC_DSY= write_data>>2;
                         write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset_uv + (RF_RX_2DISP_WIDTH*2)*RFRX_HALF_MODE_SHIFT/2 + PNBUF_SIZE_Y;
                         H264DEC_AVC_DSUV= write_data>>2;
                         break;

            		  case 1:
                         write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset + (RF_RX_2DISP_WIDTH*2)*RFRX_HALF_MODE_SHIFT;
                         H264DEC_AVC_DSY= write_data>>2;
                         write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset_uv + (RF_RX_2DISP_WIDTH*2)*RFRX_HALF_MODE_SHIFT/2 + PNBUF_SIZE_Y;
                         H264DEC_AVC_DSUV= write_data>>2;
                         break;
                   }
               }
               else
               {
                   switch(DecPos)
                   {
                      case 0:
                         write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset + (RF_RX_2DISP_WIDTH*2)*RFRX_HALF_MODE_SHIFT;
                         H264DEC_AVC_DSY= write_data>>2;
                         write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset/2 + (RF_RX_2DISP_WIDTH*2)*RFRX_HALF_MODE_SHIFT/2 + PNBUF_SIZE_Y;
                         H264DEC_AVC_DSUV= write_data>>2;
                         break;

            		  case 1:
                         write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset + (RF_RX_2DISP_WIDTH*2)*RFRX_HALF_MODE_SHIFT;
                         H264DEC_AVC_DSY= write_data>>2;
                         write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset/2 + (RF_RX_2DISP_WIDTH*2)*RFRX_HALF_MODE_SHIFT/2 + PNBUF_SIZE_Y;
                         H264DEC_AVC_DSUV= write_data>>2;
                         break;
                   }
               }
            }
       }
       else
    #endif
       {
           if( info->Height >= 860)
           {
           	   #if((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)&&(USE_NEW_MEMORY_MAP))
			   switch(RFUnit)
               {
                  case 0:
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset_uv + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 1:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset_uv + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 2:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 + VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0] + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + VideoOffset_uv + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;

        		  case 3:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 + VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + VideoOffset_uv + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;	 
               }
			   #else			  
               switch(RFUnit)
               {
                  case 0:
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset_uv+ PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 1:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + VideoOffset;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset_uv + PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 2:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 + VideoOffset;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0] + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + VideoOffset_uv + PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;

        		  case 3:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 + VideoOffset;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + VideoOffset_uv + PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;	 
               }
			   #endif
           }
           else if(info->Height >= 720)
           {
           	   #if((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)&&(USE_NEW_MEMORY_MAP))
			   switch(RFUnit)
               {
                  case 0:
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset/2 + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 1:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset/2 + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 2:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 + VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0] + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + VideoOffset/2 + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;

        		  case 3:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 + VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + VideoOffset/2 + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;	 
               }
			   #else
               switch(RFUnit)
               {
                  case 0:
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset/2 + PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 1:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + VideoOffset;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset/2 + PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 2:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 + VideoOffset;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0] + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + VideoOffset/2 + PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;

        		  case 3:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 + VideoOffset;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + VideoOffset/2 + PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;	 
               }
			   #endif
           }
           else 
           {
               switch(RFUnit)
               {
               	  #if((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)&&(USE_NEW_MEMORY_MAP))
				  case 0:
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset_uv + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 1:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + VideoOffset + DISPLAY_BUFFER_Y_OFFSET;;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset_uv + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 2:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0] + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;

        		  case 3:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 + DISPLAY_BUFFER_Y_OFFSET;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;	
				  #else
                  case 0:
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ VideoOffset_uv + PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 1:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + VideoOffset;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + VideoOffset_uv + PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
        			 break;

        		  case 2:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0] + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2 + PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;

        		  case 3:
                     write_data = (u32)MainVideodisplaybuf[0] + RF_RX_2DISP_WIDTH + RF_RX_2DISP_HEIGHT*RF_RX_2DISP_WIDTH*2 ;
                     H264DEC_AVC_DSY= write_data>>2;
                     write_data = (u32)MainVideodisplaybuf[0]+ RF_RX_2DISP_WIDTH + (RF_RX_2DISP_HEIGHT/2)*RF_RX_2DISP_WIDTH*2+ PNBUF_SIZE_Y;
                     H264DEC_AVC_DSUV= write_data>>2;
                     break;	 
				  #endif	 
               }
           }

       }
	}
	else if(DispMode == RFIU_RX_DISP_SUB1)
	{
   	   write_data = (u32)Sub1Videodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM];
       H264DEC_AVC_DSY= write_data>>2;
       write_data = (u32)Sub1Videodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + PNBUF_SIZE_Y;
       H264DEC_AVC_DSUV= write_data>>2;
	}
    write_data = ((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));
	H264DEC_AVC_CFO = write_data;
	//DEBUG_H264("Addr 0x0038 = 0x%08x\n",H264DEC_AVC_CFO);

	// List0, List1 control (baseline always 3'd0)
	write_data = 0x00000000;
	H264DEC_AVC_FCS = write_data>>2;
	//DEBUG_H264("Addr 0x003c = 0x%08x\n",H264DEC_AVC_FCS);	
    if( DispMode == RFIU_RX_DISP_PIP )
    {
        RefBuf_Y   = PKBuf_PIP1Y;
        RefBuf_Cb  = PKBuf_PIP1CbCr;
        //RefBuf_Cr  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr;
    	
        McoBuf_Y   = PKBuf_PIP2Y;
        McoBuf_Cb  = PKBuf_PIP2CbCr;
        //McoBuf_Cr  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
     
        //Exchange address//
    	PKBuf_PIP2Y    = RefBuf_Y;
        PKBuf_PIP2CbCr = RefBuf_Cb;
        //rfiuRxDecBuf[0].mpeg4NRefBuf_Cr=RefBuf_Cr;

    	PKBuf_PIP1Y    = McoBuf_Y;
        PKBuf_PIP1CbCr = McoBuf_Cb;
        //rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr=McoBuf_Cr;
        
        /*CY 0907*/
        write_data = 640*352;
        H264DEC_AVC_CFO = write_data;

    }
    else
    {
    	#if ( ((SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2)) && (USE_NEW_MEMORY_MAP))
		if(DispMode == RFIU_RX_DISP_MAIN_HD)
		{
			//DEBUG_RED("swap_flag = %d\n", swap_flag);
			if(swap_flag==0)
			{				
				swap_flag = 1;
				RefBuf_Y   = MainVideodisplaybuf[DISPLAY_BUF_NUM];
		        RefBuf_Cb  = MainVideodisplaybuf[DISPLAY_BUF_NUM]+((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));
		    	
		        McoBuf_Y   = MainVideodisplaybuf[DISPLAY_BUF_NUM + 1];
		        McoBuf_Cb  = MainVideodisplaybuf[DISPLAY_BUF_NUM + 1]+((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));
			}
			else
			{
				swap_flag = 0;
				RefBuf_Y   = MainVideodisplaybuf[DISPLAY_BUF_NUM + 1];
		        RefBuf_Cb  = MainVideodisplaybuf[DISPLAY_BUF_NUM + 1]+((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));
		    	
		        McoBuf_Y   = MainVideodisplaybuf[DISPLAY_BUF_NUM];
		        McoBuf_Cb  = MainVideodisplaybuf[DISPLAY_BUF_NUM]+((RF_RX_DEC_WIDTH_MAX + 0) * (RF_RX_DEC_HEIGHT_MAX + 0));

			}
		}
		else
		{
			RefBuf_Y   = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y;
	        RefBuf_Cb  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cb;
	        //RefBuf_Cr  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr;
	    	
	        McoBuf_Y   = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
	        McoBuf_Cb  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
	        //McoBuf_Cr  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
	     
	        //Exchange address//
	    	rfiuRxDecBuf[0].mpeg4NRefBuf_Y  = RefBuf_Y;
	        rfiuRxDecBuf[0].mpeg4NRefBuf_Cb = RefBuf_Cb;
	        //rfiuRxDecBuf[0].mpeg4NRefBuf_Cr=RefBuf_Cr;

	    	rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y  = McoBuf_Y;
	        rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cb = McoBuf_Cb;
		}
		#else		
        RefBuf_Y   = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y;
        RefBuf_Cb  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cb;
        //RefBuf_Cr  = rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr;
    	
        McoBuf_Y   = rfiuRxDecBuf[0].mpeg4NRefBuf_Y;
        McoBuf_Cb  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cb;
        //McoBuf_Cr  = rfiuRxDecBuf[0].mpeg4NRefBuf_Cr;
     
        //Exchange address//
    	rfiuRxDecBuf[0].mpeg4NRefBuf_Y  = RefBuf_Y;
        rfiuRxDecBuf[0].mpeg4NRefBuf_Cb = RefBuf_Cb;
        //rfiuRxDecBuf[0].mpeg4NRefBuf_Cr=RefBuf_Cr;

    	rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y  = McoBuf_Y;
        rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cb = McoBuf_Cb;
		#endif
        //rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Cr=McoBuf_Cr;
        
    }
    /*CY 0907*/
//    pictWidth = info->Width;
	#if((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)&&(USE_NEW_MEMORY_MAP))
    write_data = (u32) & McoBuf_Y [0] + DISPLAY_BUFFER_Y_OFFSET;
    H264DEC_AVC_CFY         = write_data>>2 ;

	write_data = (u32) & McoBuf_Cb[0] + DISPLAY_BUFFER_UV_OFFSET;;
    H264DEC_AVC_CFUV        = write_data>>2;

	write_data = ULTRA_FHD_SIZE_Y - DISPLAY_BUFFER_Y_OFFSET + DISPLAY_BUFFER_UV_OFFSET;
	H264DEC_AVC_CFO = write_data;	
	
    write_data = (u32) & RefBuf_Y[0] + DISPLAY_BUFFER_Y_OFFSET;;
    H264DEC_AVC_FB0         = write_data>>2;
	#else
    write_data = (u32) & McoBuf_Y [0];
    H264DEC_AVC_CFY         = write_data>>2;
    write_data = (u32) & McoBuf_Cb[0];
    H264DEC_AVC_CFUV        = write_data>>2;
    write_data = (u32) & RefBuf_Y[0];
    H264DEC_AVC_FB0         = write_data>>2;
	#endif
        
#if H264_DEBUG_ENA
    gpioSetLevel(2, 10, 1);
#endif
    // 0x0000
	if( (DispMode == RFIU_RX_DISP_MAIN_VGA) )
    {
        if(info->Height >= 860)
        {
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH - info->Width/4;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_4 | 0x00000001; //any value
        }
        else if(info->Height >= 720)  //HD
        {
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH - info->Width/2;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_2 | 0x00000001; //any value
		}
		else
		{
		   if(FieldDecEn)
              H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - info->Width;
           else
              H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH - info->Width;
           
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_1 | 0x00000001; //any value
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
	    }    
	}
    else if( DispMode == RFIU_RX_DISP_MAIN_HD )
    {
        if(RF_RX_2DISP_WIDTH*2 == info->Width) //HD
        {        	
          #if ((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)&&(USE_NEW_MEMORY_MAP))
            VideoOffset=0;       
            write_data = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset + DISPLAY_BUFFER_Y_OFFSET;
            H264DEC_AVC_CFY= write_data>>2;
            write_data =(u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset/2 + ULTRA_FHD_SIZE_Y + DISPLAY_BUFFER_UV_OFFSET;
            H264DEC_AVC_CFUV= write_data>>2;
            
            write_data = ULTRA_FHD_SIZE_Y - DISPLAY_BUFFER_Y_OFFSET + DISPLAY_BUFFER_UV_OFFSET;
            H264DEC_AVC_CFO = write_data;

            write_data = 0x00000000;
            H264DEC_AVC_FCS = write_data>>2;

            //Reference frame start address
            write_data = (u32)MainVideodisplaybuf[(rfiuVideoBufFill_idx[RFUnit] + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM] + DISPLAY_BUFFER_Y_OFFSET;
            H264DEC_AVC_FB0 = write_data>>2;
            H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - info->Width;
            H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS| 0x00000001; //any value
			
		  #else
            VideoOffset=0;       
            write_data = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset;
            H264DEC_AVC_CFY= write_data>>2;
            write_data =(u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset/2 + PNBUF_SIZE_Y;
            H264DEC_AVC_CFUV= write_data>>2;
            
            write_data = PNBUF_SIZE_Y;
            H264DEC_AVC_CFO = write_data;

            write_data = 0x00000000;
            H264DEC_AVC_FCS = write_data>>2;

            //Reference frame start address
            write_data = (u32)MainVideodisplaybuf[(rfiuVideoBufFill_idx[RFUnit] + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM];
            H264DEC_AVC_FB0 = write_data>>2;
            H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - info->Width;
            H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS| 0x00000001; //any value
          #endif
        }
        else if(RF_RX_2DISP_WIDTH*2 < info->Width)//FULL HD 1920*1440
        {
        #if(SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2)			
            VideoOffset = 1920*180;
            VideoOffset_uv = 1920*180/2;
            write_data = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset;
            H264DEC_AVC_CFY = write_data>>2;
            write_data = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset_uv + 1920*1440;
            H264DEC_AVC_CFUV = write_data>>2;

            write_data = (1920*1440+VideoOffset_uv)-VideoOffset;
            H264DEC_AVC_CFO = write_data;

            write_data = 0x00000000;
            H264DEC_AVC_FCS = write_data>>2;

            //Reference frame start address
            write_data = (u32)MainVideodisplaybuf[(rfiuVideoBufFill_idx[RFUnit] + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM]+ VideoOffset;
            H264DEC_AVC_FB0 = write_data>>2;
            H264DEC_AVC_STRP = 0;
            H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS| 0x00000001; //any value            
         #elif((SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1))
            VideoOffset = 1920*4;
            VideoOffset_uv = VideoOffset/2;
            write_data = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset;
            H264DEC_AVC_CFY = write_data>>2;
            write_data = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset_uv + PNBUF_SIZE_Y;
            H264DEC_AVC_CFUV = write_data>>2;

            write_data = (PNBUF_SIZE_Y+VideoOffset_uv)-VideoOffset;
            H264DEC_AVC_CFO = write_data;

            write_data = 0x00000000;
            H264DEC_AVC_FCS = write_data>>2;

            //Reference frame start address
            write_data = (u32)MainVideodisplaybuf[(rfiuVideoBufFill_idx[RFUnit] + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM]+ VideoOffset;
            H264DEC_AVC_FB0 = write_data>>2;
            H264DEC_AVC_STRP = 0;
            H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS| 0x00000001; //any value
          #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
            if (sysTVOutOnFlag)
            {
                if (isCap1920x1080I() == 1)
                {
                    VideoOffset = 1920*4;
                    VideoOffset_uv = VideoOffset/2;
                    write_data = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset;
                    H264DEC_AVC_CFY = write_data>>2;
                    write_data = (u32)MainVideodisplaybuf[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] + VideoOffset_uv + PNBUF_SIZE_Y;
                    H264DEC_AVC_CFUV = write_data>>2;

                    write_data = (PNBUF_SIZE_Y+VideoOffset_uv)-VideoOffset;
                    H264DEC_AVC_CFO = write_data;

                    write_data = 0x00000000;
                    H264DEC_AVC_FCS = write_data>>2;

                    //Reference frame start address
                    write_data = (u32)MainVideodisplaybuf[(rfiuVideoBufFill_idx[RFUnit] + DISPLAY_BUF_NUM - 1) % DISPLAY_BUF_NUM]+ VideoOffset;
                    H264DEC_AVC_FB0 = write_data>>2;
                    
                    H264DEC_AVC_STRP = 0;
                    H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS| 0x00000001; //any value
                }
                else
                {
                    H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - (info->Width/2);
                    H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_2 | 0x00000001; //any value
                }
            }
            else
            {
                H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - (info->Width/2);
                H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_2 | 0x00000001; //any value
            }
        #else
            H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - (info->Width/2);
            H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_2 | 0x00000001; //any value
        #endif
        }
        else  //QHD
        {
            H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - info->Width;
            H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_1 | 0x00000001; //any value

        }
    }
    else if( DispMode == RFIU_RX_DISP_PIP )
    {
        H264DEC_AVC_STRP = 0;
        H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_2 | 0x00000001; //any value
    }
    else if( (DispMode == RFIU_RX_DISP_SUB1))
    {
    #if TV_D1_OUT_FULL
        if(FieldDecEn)
          H264DEC_AVC_STRP   = 704*2 - info->Width;
        else
          H264DEC_AVC_STRP   = 704 - info->Width;

        H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
        //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
    #else
        if(FieldDecEn)
          H264DEC_AVC_STRP   = 640*2 - info->Width;
        else
          H264DEC_AVC_STRP   = 640 - info->Width;

        H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_1 | 0x00000001; //any value
        //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
    #endif
    }
	else if(DispMode == RFIU_RX_DISP_QUARD_HD)
    {
        if(info->Height>= 860)
        {
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - info->Width/4;
           if(QuadDispOut)
              H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_4 | 0x00000001; //any value
           else
              H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value

        }
        else if(info->Height>= 720)
        {
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - info->Width/2;
           if(QuadDispOut)
              H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_2 | 0x00000001; //any value
           else
              H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
        }
        else
        {
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - info->Width;
           if(QuadDispOut)
              H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_1 | 0x00000001; //any value
           else
              H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_DIS | 0x00000001; //any value
        }
	}
    else if(DispMode == RFIU_RX_DISP_MASK)
    {
        if(info->Height>= 860)  //FHD
        {
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - (info->Width/2);
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_2 | 0x00000001; //any value
        }
        else if(info->Height >= 720)  //HD
        {
           H264DEC_AVC_STRP = RF_RX_2DISP_WIDTH*2 - info->Width/2;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_2 | 0x00000001; //any value
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG | MPEG4_DS_ENA;
		}
		else
		{
		   if(FieldDecEn)
              H264DEC_AVC_STRP   = RF_RX_2DISP_WIDTH*2 - info->Width;
           else
              H264DEC_AVC_STRP   = RF_RX_2DISP_WIDTH*2 - info->Width;
           H264DEC_AVC_SOD  = H264_DEC_DOWNSAMPLE_1 | 0x00000001; //any value
           //Mpeg4Ctrl           = (info->Quant_type << 2) | MPEG4_DBG_DISA | MPEG4_DEC_TRG;
	    }    
    }

#if MPEG_DEBUG_ENA_LUCIAN
    gpioSetLevel(0, 1, 1);
#endif

    OSSemPend(VideoCpleSemEvt, MPEG4_TIMEOUT, &err);

#if H264_DEBUG_ENA
    gpioSetLevel(2, 10, 0);
#endif

    if (err != OS_NO_ERR)
    {
        // reset MPEG-4 hardware
        SYS_RSTCTL  = SYS_RSTCTL | SYS_RSTCTL_MPEG4_RST;
		for(i=0;i<10;i++); //delay
		DEBUG_H264("Decoder Error: VideoCpleSemEvt is %d, Size:<%d, %d>, Type:<%d, %d>.\n", err, info->poffset, *(info->pSize), info->FrameType, info->Small_FrameType);
        SYS_RSTCTL  = SYS_RSTCTL & (~SYS_RSTCTL_MPEG4_RST);
        
        info->FrameIdx ++;
        Error = 1;

        if(H264ErrCnt > 10)
        {
            DEBUG_H264("H.264 fetal error! Force to Reboot!!\n");
			sysForceWDTtoReboot();
        }
        H264ErrCnt ++;
    }
	else
	{
        info->FrameIdx ++;
        H264ErrCnt=0;
		#if VIDEO_STARTCODE_DEBUG_ENA
		monitor_decode[RFUnit]++;
		#endif
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
//        MbWidth         = (info->Width  + 15) >> 4;
//        MbHeight        = (info->Height/2 + 15) >> 4;
//        MbNum           = ((info->Width  + 15) >> 4) * ((info->Height/2 + 15) >> 4);

        H264DEC_AVC_DSY         = (u32) &(rfiuRxDecBuf[RFUnit].mpeg4PRefBuf_Y);

        H264DEC_AVC_DSUV        += (720 * 576);

//        pictWidth = info->Width;
        H264DEC_AVC_CFY         += (720 * 576) ;

        H264DEC_AVC_CFUV        += (720 * 576) / 2;

        H264DEC_AVC_FB0         += (720 * 576) + ((720 * 576) / 2);

        //H264DEC_AVC_DSUV        += (720 * 576) / 2;
        if(DispMode == RFIU_RX_DISP_MASK)
        {
           if(info->Height>= 860)  //FHD
              H264DEC_AVC_STRP   = RF_RX_2DISP_WIDTH*2 - info->Width/2;
           else if(info->Height >= 720)  //HD
           {
              H264DEC_AVC_STRP   = RF_RX_2DISP_WIDTH*2 - info->Width/2;
           }
           else
              H264DEC_AVC_STRP   = RF_RX_2DISP_WIDTH*2 - info->Width;
        }        
        else
        {
           H264DEC_AVC_STRP   = RF_RX_2DISP_WIDTH*2 - info->Width;
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
#if 0
    if((sysTVOutOnFlag == SYS_OUTMODE_PANEL) && (BRI_SCCTRL_MODE & 0x4))
    {
        iduTVColorbar_onoff(0); 
    }
#endif
    if(Error)
        return  MPEG4_ERROR;
    else
        return  MPEG4_NORMAL;   
}

u32 H264_Decode_One_I_frame(int RFUnit)
{
    u32 Error;
    u32 *pVideoSize;
    
    H264_ENC_CFG H264Enc;
    H264_DEC_CFG H264Dec;
	VIDEO_INFO video_info;  
   #if SB_DECORD_SUPPORT
    H264_ENC_CFG H264Enc_sub;
    H264_DEC_CFG H264Dec_sub;
	VIDEO_INFO video_info_sub;
   #endif
   //DEBUG_H264("START H264_Decode_One_I_frame[%d][%d] \n",RFUnit,H264_IFlag_Index[RFUnit]);

    if((H264_IFlag_Index[RFUnit] == 0) || H264_FrameError[RFUnit] == 1)
    {
        DEBUG_H264("END H264_Decode_One_I_frame[%d][%d][%d] \n",RFUnit,H264_IFlag_Index[RFUnit],H264_FrameError[RFUnit]);
        return  MPEG4_NORMAL;
    }
//    DEBUG_H264("[%d] H264_Decode_One_I_frame[%d][%d] \n",__LINE__,RFUnit,H264_IFlag_Index[RFUnit]);
        
    if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
    {
        Error = H264Enc_SetResolution(&H264Enc,gRfiuUnitCntl[RFUnit].TX_PicWidth,gRfiuUnitCntl[RFUnit].TX_PicHeight);
        if(Error == 0)
            return MPEG4_ERROR;

        rfiuH264Encode_I_Header(&H264Enc,
                                gRfiuUnitCntl[RFUnit].TX_PicWidth,
                                gRfiuUnitCntl[RFUnit].TX_PicHeight);
        rfiuH264Decode_I_Header(&H264Enc,&H264Dec); 

        pVideoSize       = &(rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].size);
        video_info.pSize     = pVideoSize;            
        video_info.poffset   = rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].offset;
        video_info.StreamBuf = rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].buffer;
        Error = H264DecodeLastFrame(&video_info,
                                    &H264Dec,
                                    rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].buffer, 
                                    *pVideoSize, 
                                    RFUnit,
                                    rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].offset,
                                    RFIU_RX_DISP_MAIN_HD,
                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                    1);
//        DEBUG_H264("[%d] H264_Decode_One_I_frame[%d][%d] \n",__LINE__,RFUnit,H264_IFlag_Index[RFUnit]);
    }
    else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
    {
      #if SB_DECORD_SUPPORT
        if((gRfiuUnitCntl[RFUnit].RFpara.TX_SubStreamSupport == 0) )
        {
            Error = H264Enc_SetResolution(&H264Enc,gRfiuUnitCntl[RFUnit].TX_PicWidth,gRfiuUnitCntl[RFUnit].TX_PicHeight);
            if(Error == 0)
                return MPEG4_ERROR;
            if(H264Enc.pic_width_in_mbs <= 80 && H264Enc.pic_height_in_map_unit < 64)
                H264Dec.level_idc    = 30;                          //0x0008[07:00]   
            else
                H264Dec.level_idc    = 40;                          //0x0008[07:00]

            rfiuH264Decode_I_Header(&H264Enc,&H264Dec);
            
            pVideoSize       = &(rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].size);
            video_info.pSize     = pVideoSize;            
            video_info.poffset   = rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].offset;
            video_info.StreamBuf = rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].buffer;
            Error = H264DecodeLastFrame(&video_info,
                                        &H264Dec,
                                         video_info.StreamBuf, 
                                        *pVideoSize, 
                                        RFUnit,
                                        video_info.poffset,
                                        RFIU_RX_DISP_QUARD_HD,
                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                        1);
//            DEBUG_H264("[%d] H264_Decode_One_I_frame[%d][%d] \n",__LINE__,RFUnit,H264_IFlag_Index[RFUnit]);
        }
        else
        {
            H264Enc_SetResolution(&H264Enc,640,352);
            if(H264Enc.pic_width_in_mbs <= 80 && H264Enc.pic_height_in_map_unit < 64)
                H264Dec.level_idc    = 30;                          //0x0008[07:00]   
            else
                H264Dec.level_idc    = 40;                          //0x0008[07:00]

            rfiuH264Decode_I_Header(&H264Enc,&H264Dec);

            pVideoSize       = &(rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].size);
            video_info.pSize     = pVideoSize; 
            video_info.poffset   = rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].offset;
            
//            DEBUG_H264("[%d] size %d, offset %d \n",__LINE__,*pVideoSize,video_info.poffset);
          #if CHECK_VIDEO_BITSTREAM
			if((*pVideoSize-video_info.poffset) != 4) //End_code
            {
                
                video_info.StreamBuf = rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].buffer+video_info.poffset;
           
                Error = H264DecodeLastFrame(&video_info,
                                            &H264Dec,
                                            rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].buffer, 
                                            (*pVideoSize-video_info.poffset), 
                                            RFUnit,
                                            video_info.poffset,
                                            RFIU_RX_DISP_QUARD_HD,
                                            (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                            1);
//                DEBUG_H264("[%d] H264_Decode_One_I_frame[%d][%d] \n",__LINE__,RFUnit,H264_IFlag_Index[RFUnit]);
            }
		  #else
            if((*pVideoSize-video_info.poffset) != 0)
            {
                video_info.StreamBuf = rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].buffer+video_info.poffset;
           
                Error = H264DecodeLastFrame(&video_info,
                                            &H264Dec,
                                            rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].buffer, 
                                            (*pVideoSize-video_info.poffset), 
                                            RFUnit,
                                            video_info.poffset,
                                            RFIU_RX_DISP_QUARD_HD,
                                            (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                            1);
//                DEBUG_H264("[%d] H264_Decode_One_I_frame[%d][%d] \n",__LINE__,RFUnit,H264_IFlag_Index[RFUnit]);
            }
		  #endif
            else
            {
                DEBUG_H264("No substream!\n");
            }
        }
      #else 
        rfiuH264Encode_I_Header(&H264Enc,
                                gRfiuUnitCntl[RFUnit].TX_PicWidth,
                                gRfiuUnitCntl[RFUnit].TX_PicHeight);
        rfiuH264Decode_I_Header(&H264Enc,&H264Dec); 
        
        pVideoSize       = &(rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].size);
        video_info.pSize     = pVideoSize;            
        video_info.poffset   = rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].offset;
        video_info.StreamBuf = rfiuRxVideoBufMng[RFUnit][H264_IFlag_Index[RFUnit]].buffer;
        Error = H264DecodeLastFrame(&video_info,
                                    &H264Dec,
                                    video_info.StreamBuf, 
                                    *pVideoSize, 
                                    RFUnit,
                                    video_info.poffset,
                                    RFIU_RX_DISP_QUARD_HD,
                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                    1);
      #endif
    }
//    DEBUG_H264("END H264_Decode_One_I_frame[%d][%d] \n",RFUnit,H264_IFlag_Index[RFUnit]);
    if(Error)
        return  MPEG4_ERROR;
    else
        return  MPEG4_NORMAL;   
    
}
s32 h264SetVideoResolution(u16 width,u16 height)
{
    mpeg4Width = (u32)width;
    mpeg4Height= (u32)height;
    H264Enc_SetResolution(&H264Enc_cfg,width,height);
}

#endif
