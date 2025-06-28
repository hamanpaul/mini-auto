/*

Copyright (c) 2014 Mars Semiconductor Corp.

Module Name:

    MultiChannelH264.c

Abstract:

    The routines of Multiple Channel H264 encoder/decoder.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2014/08/21  Peter Create  

*/
#include "general.h"

#if (MULTI_CHANNEL_VIDEO_REC && (VIDEO_CODEC_OPTION == H264_CODEC))

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
#include "ipuapi.h"
#include "GlobalVariable.h"

/*
*********************************************************************************************************
* Constant
*********************************************************************************************************
*/

/*
*********************************************************************************************************
* Variable
*********************************************************************************************************
*/

u8          Video_Curr_Channel_ID;

/*
*********************************************************************************************************
* Extern Varaibel
*********************************************************************************************************
*/

extern OS_EVENT    *mpeg4ReadySemEvt;

extern u32 sysVideoInSel;
extern u32 EventTrigger;  //用於Buffer moniting.
extern u32 asfVopCount;   //用於Buffer moniting.

extern s32 mp4_avifrmcnt, isu_avifrmcnt;
extern u32 IsuIndex;

extern u8* mpeg4outputbuf[3];
extern u8  sysCaptureVideoStop;
extern u32 unMp4FrameDuration[SIU_FRAME_DURATION_NUM]; /* mh@2006/11/22: for time stamp control */ /* Peter 070403 */
extern u32 isu_int_status;

extern u32 VideoPictureIndex;
extern u8  TVout_Generate_Pause_Frame;

#if NIC_SUPPORT
extern u8 EnableStreaming;
extern u8  LocalChannelSource; // ch0 source
#endif
#if TUTK_SUPPORT
extern s8 P2PEnableStreaming[];
#endif

extern u32 FiqError;

extern u32 H264_Special_CMD;



/*
*********************************************************************************************************
* Function prototype
*********************************************************************************************************
*/


u32 MultiChannelH264GetQP(u8 mode, VIDEO_CLIP_OPTION *pVideoClipOption)
{
	if(mode == 1) //fixed QP from global QP		
		return pVideoClipOption->QP;
	else if(mode == 2)//rand
	    return (rand()%26)+15;  //15~40	
    else if(mode == 3)//rand
		return (rand()%36)+10;  //10~45		    
    else if(mode == 4)//fixed QP
		return 30;	
    else if(mode == 5)//QP range 10~45, each 100 frame add QP
    {        
        if(pVideoClipOption->frame_cnt == 100)
        {
            pVideoClipOption->frame_cnt = 0;

            if(pVideoClipOption->local_QP >= 46)
                pVideoClipOption->local_QP = 10;
            else
                pVideoClipOption->local_QP++;
            
            return pVideoClipOption->local_QP;	
        }
        else
        {
            pVideoClipOption->frame_cnt++;            
            return pVideoClipOption->local_QP;	
        }		
    }    
    else if(mode == 6)//QP range 10~42, each frame add QP
    {       
        pVideoClipOption->local_QP += 1;
        if(pVideoClipOption->local_QP > 51)
            pVideoClipOption->local_QP = 0;
        return pVideoClipOption->local_QP;	
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

u32 MultiChannelH264Enc_DecSliceType(u32 frame_idx, u32 period, VIDEO_CLIP_OPTION *pVideoClipOption)
{
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    H264_ENC_CFG* cfg = &pVideoClipOption->H264Enc_cfg;
                                
    //calulate time by video time, to get exactly asf file time
    if(!(pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && pVideoClipOption->SetIVOP)    
    {
        DEBUG_H264("Ch%d H264 calulate time, start index = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngWriteIdx);        
        OS_ENTER_CRITICAL();
        pVideoClipOption->Cal_FileTime_Start_Idx    = pVideoClipOption->VideoBufMngWriteIdx; 
        pVideoClipOption->VideoTimeStatistics       = 0;
        pVideoClipOption->SetIVOP                   = 0;
        OS_EXIT_CRITICAL();
        cfg->slice_type           = I_FRAME;
        
    }        
    else if((pVideoClipOption->asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && pVideoClipOption->SetIVOP)
    {
        DEBUG_H264("Ch%d H264 calulate time, start index = %d\n", pVideoClipOption->VideoChannelID, pVideoClipOption->VideoBufMngWriteIdx);
        
        OS_ENTER_CRITICAL();
        pVideoClipOption->Cal_FileTime_Start_Idx = pVideoClipOption->VideoBufMngWriteIdx; 
        pVideoClipOption->VideoTimeStatistics = 0;
        pVideoClipOption->SetIVOP = 0;
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
    cfg->qp               = MultiChannelH264GetQP(1, pVideoClipOption);
    #else
    cfg->qp               = MultiChannelH264GetQP(4, pVideoClipOption);
    #endif
	cfg->slice_qp_delta   = cfg->qp - 26; 
    
    return cfg->slice_type;
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

s32 MultiChannelH264Enc_InitBuf(VIDEO_CLIP_OPTION *pVideoClipOption)    
{
    #if 0
    //static int cnt = 1;
    u8 fin_name[32];
    FS_FILE*                pFile;
    u32 size;
    #endif
    H264_ENC_CFG* cfg = &pVideoClipOption->H264Enc_cfg;
    
    //Local file test, use mpeg4 buffer
    //Input source frame buffer
    switch(pVideoClipOption->VideoChannelID)
    {
        case 0:
            cfg->CurrRawYAddr  = PNBuf_Y[pVideoClipOption->VideoPictureIndex % 4];
            cfg->CurrRawUAddr  = PNBuf_C[pVideoClipOption->VideoPictureIndex % 4];
            break;
        case 1:
            cfg->CurrRawYAddr  = PNBuf_sub1[pVideoClipOption->VideoPictureIndex % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub1[pVideoClipOption->VideoPictureIndex % 4] + PNBUF_SIZE_Y);
            break;
        case 2:
            cfg->CurrRawYAddr  = PNBuf_sub2[pVideoClipOption->VideoPictureIndex % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub2[pVideoClipOption->VideoPictureIndex % 4] + PNBUF_SIZE_Y);
            break;
        case 3:
            cfg->CurrRawYAddr  = PNBuf_sub3[pVideoClipOption->VideoPictureIndex % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub3[pVideoClipOption->VideoPictureIndex % 4] + PNBUF_SIZE_Y);
            break;
        case 4:
            cfg->CurrRawYAddr  = PNBuf_sub4[pVideoClipOption->VideoPictureIndex % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub4[pVideoClipOption->VideoPictureIndex % 4] + PNBUF_SIZE_Y);
            break;
        case 5:
            cfg->CurrRawYAddr  = PNBuf_sub5[pVideoClipOption->VideoPictureIndex % 4];
            cfg->CurrRawUAddr  = (PNBuf_sub5[pVideoClipOption->VideoPictureIndex % 4] + PNBUF_SIZE_Y);
            break;    
        default:
            DEBUG_H264("Error: Can't support MultiChannelH264Enc_InitBuf(%d)\n", pVideoClipOption->VideoChannelID);
            return 0;
    }

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
    if(pVideoClipOption->EncodeDownSample)
        H264ENC_ADDR_SEL_UV |= H264_ENC_DOWNSAMPLE;

    if(pVideoClipOption->EncodeLineStripe)
    {
        H264ENC_ADDR_BASE_Y_CUR     = ((u32)cfg->CurrRawYAddr) + pVideoClipOption->FrameWidth * pVideoClipOption->V_addr + pVideoClipOption->H_addr;;
        H264ENC_ADDR_BASE_U_CUR     = ((u32)cfg->CurrRawUAddr) + pVideoClipOption->FrameWidth*(pVideoClipOption->V_addr / 2) + pVideoClipOption->H_addr;
        H264ENC_ADDR_OFFSET_LINE    = pVideoClipOption->FrameWidth - pVideoClipOption->LineStripeFrameWidth;;
    }
    else
    {
        H264ENC_ADDR_BASE_Y_CUR = (u32)cfg->CurrRawYAddr;
        H264ENC_ADDR_BASE_U_CUR = (u32)cfg->CurrRawUAddr;
    }    

    //Input reference frame buffer, Output Reconstruct frame buffer
    if(cfg->frame_num & 0x01)
    {
        cfg->RefRawYAddr  = pVideoClipOption->VideoNRefBuf_Y;       //0x0110, Input reference image Y addres
        cfg->RefRawUVAddr = pVideoClipOption->VideoNRefBuf_Cb;      //0x0114, Input reference image UV addres  
        cfg->RecRawYAddr  = pVideoClipOption->VideoPRefBuf_Y;        //0x0118, Output reconstructed image Y address
        cfg->RecRawUVAddr = pVideoClipOption->VideoPRefBuf_Cb;       //0x0124, Output reconstructed image UV address 
    }
    else
    {
        cfg->RefRawYAddr  = pVideoClipOption->VideoPRefBuf_Y;        //0x0110, Input reference image Y addres
        cfg->RefRawUVAddr = pVideoClipOption->VideoPRefBuf_Cb;       //0x0114, Input reference image UV addres  
        cfg->RecRawYAddr  = pVideoClipOption->VideoNRefBuf_Y;       //0x0118, Output reconstructed image Y address
        cfg->RecRawUVAddr = pVideoClipOption->VideoNRefBuf_Cb;      //0x0124, Output reconstructed image UV address 
    }
      
    H264ENC_ADDR_BASE_Y_REF     = (u32)cfg->RefRawYAddr;
    H264ENC_ADDR_BASE_UV_REF    = (u32)cfg->RefRawUVAddr;
    H264ENC_ADDR_BASE_Y_DP      = (u32)cfg->RecRawYAddr;
    H264ENC_ADDR_BASE_UV_DP     = (u32)cfg->RecRawUVAddr;
    
    //Encoder local reference buffer 
    cfg->ILFPredBuf             = pVideoClipOption->H264ILFPredBuf;              //0x0128, Output ILF prediction data base address  , FRAME_WIDTH*4*12+0x3F
    cfg->IntraPredBuf           = pVideoClipOption->H264IntraPredBuf;            //0x012c, Output Intra prediction data base address, FRAME_WIDTH*2*12+0x3F
    cfg->ILF_offset             = cfg->pic_width_in_mbs * 32;  //0x0134, ILF prediction data offset in external memory, FRAME_WIDTH*4*2/4   
    cfg->IntraPred_offset       = cfg->pic_width_in_mbs * 8;  //0x0138, Intra prediction data offset in external memory, FRAME_WIDTH*2/4  
    
    H264ENC_ADDR_BASE_ILF       =  (u32)cfg->ILFPredBuf;
    H264ENC_ADDR_BASE_INTRA     =  (u32)cfg->IntraPredBuf;
    H264ENC_ADDR_OFFSET_ILF     =  cfg->ILF_offset;
    H264ENC_ADDR_OFFSET_INTRA   =  cfg->IntraPred_offset;   
        
    //Output Bitstram
    //cfg->H264StreamAddr   = VideoBufMng[0].buffer;   //0x0130, Output encoded bitstream base address
    H264ENC_ADDR_BASE_BS  = (u32)cfg->H264StreamAddr;
    //DEBUG_H264("0x00000130 = 0x%08x\n",(u32)cfg->H264StreamAddr);
    return 1;
}

u32 MultiChannelH264Enc_CompressOneFrame(VIDEO_CLIP_OPTION *pVideoClipOption)
{
//  u8              i;
    u8              err;
//  u32             intStat;        
    H264_ENC_CFG*   cfg     = &pVideoClipOption->H264Enc_cfg;
    VIDEO_INFO*     info    = &pVideoClipOption->video_info;

    OSSemPend(mpeg4ReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
    Video_Curr_Channel_ID       = pVideoClipOption->VideoChannelID;

#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_enable();
#endif
    switch(pVideoClipOption->VideoChannelID)
    {
        case 0:
            OSSemPend(isuSemEvt, 30, &err);
            if (err != OS_NO_ERR)
            {
                pVideoClipOption->MPEG4_Error = 1;
                isuStop();
                ipuStop();
                siuStop();    
                
                DEBUG_H264("Error: isuSemEvt(video capture mode) is %d.\n", err);
                DEBUG_H264("isu_int_status = 0x%08x,0x%08x\n", isu_int_status,SYS_CTL0);

                SYSReset(SYS_RSTCTL_SIU_RST | SYS_RSTCTL_IPU_RST | SYS_RSTCTL_ISU_RST);
                OSSemPost(mpeg4ReadySemEvt);
                return 0;
            }
            break;
        case 1:
            OSSemPend(ciuCapSemEvt_CH1, 30, &err);
            if (err != OS_NO_ERR)
            {
                pVideoClipOption->MPEG4_Error = 1;
                ciu_1_Stop();

                DEBUG_H264("Error: ciuCapSemEvt_CH1(video capture mode) is %d.\n", err);

                SYSReset(SYS_RSTCTL_CIU_RST);
                OSSemPost(mpeg4ReadySemEvt);
                return 0;
            }
            break;
        case 2:
            OSSemPend(ciuCapSemEvt_CH2, 30, &err);
            if (err != OS_NO_ERR)
            {
                pVideoClipOption->MPEG4_Error = 1;
                ciu_2_Stop();

                DEBUG_H264("Error: ciuCapSemEvt_CH2(video capture mode) is %d.\n", err);

                SYSReset(SYS_RSTCTL_CIU2_RST);
                OSSemPost(mpeg4ReadySemEvt);
                return 0;
            }
            break;
        case 3:
            OSSemPend(ciuCapSemEvt_CH3, 30, &err);
            if (err != OS_NO_ERR)
            {
                pVideoClipOption->MPEG4_Error = 1;
                ciu_3_Stop();

                DEBUG_H264("Error: ciuCapSemEvt_CH3(video capture mode) is %d.\n", err);

                SYSReset_EXT(SYS_CTL0_EXT_CIU3_RST);
                OSSemPost(mpeg4ReadySemEvt);
                return 0;
            }
            break;
        case 4:
            OSSemPend(ciuCapSemEvt_CH4, 30, &err);
            if (err != OS_NO_ERR)
            {
                pVideoClipOption->MPEG4_Error = 1;
                ciu_4_Stop();

                DEBUG_H264("Error: ciuCapSemEvt_CH4(video capture mode) is %d.\n", err);

                SYSReset_EXT(SYS_CTL0_EXT_CIU4_RST);
                OSSemPost(mpeg4ReadySemEvt);
                return 0;
            }
            break;
        default:
            DEBUG_H264("Error: Can't support MultiChannelH264Enc_CompressOneFrame(%d)\n", pVideoClipOption->VideoChannelID);
            OSSemPost(mpeg4ReadySemEvt);
            return 0;
    }

    *(info->FrameTime) += pVideoClipOption->ISUFrameDuration[pVideoClipOption->VideoPictureIndex % ISU_FRAME_DURATION_NUM]; 
    
    cfg->H264StreamAddr = info->StreamBuf;
    cfg->slice_type     = info->FrameType;
    
    //cfg->frame_num      = info->FrameIdx;
    
    H264ENC_EN_ENC_SWRST        = 0x00000000; //reset AVC
    H264ENC_EN_ENC_SWRST        = 0x00000000; //reset AVC
    H264ENC_EN_ENC_SWRST        = 0x00000000; //reset AVC
    H264ENC_EN_ENC_SWRST        = 0x00000001; //enable AVC
    H264ENC_ADDR_SPECIAL_CMD    = H264_Special_CMD; //defalut disable special command
    

    if(info->ResetFlag == 1)
	{
	    //printf("new SPS, PPS\n");
		H264Enc_InitCfg(cfg);
        pVideoClipOption->SPS_PPS_Length = GenerateParameterSets_SW(cfg, pVideoClipOption->SPS_PPS_Buffer, &pVideoClipOption->active_pps, &pVideoClipOption->active_sps, &pVideoClipOption->datastream);
		H264Enc_GenerateParameterSets_HW(cfg);
        info->ResetFlag = 0;        
    }   
    
    H264Enc_SetNALSliceHeader(cfg);
    MultiChannelH264Enc_InitBuf(pVideoClipOption);
    H264Enc_SetModeDecisionAlgorithm(cfg);

    // Clear IP Interrupt
	H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_CLR_INT; 
    //DEBUG_H264("0x00000018 = 0x%08x\n",H264_ENC_CLR_INT);
    
    // IP start
    //H264Enc_DumpRegister();
    H264ENC_INT_MASK = 0xfffffffb;
#if H264_DEBUG_ENA
    gpioSetLevel(2, 10, 0);
#endif    
    H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_EN_IP;
    //DEBUG_H264("0x00000018 = 0x%08x\n",H264_ENC_EN_IP);    
#if H264_DEBUG_ENA
    gpioSetLevel(2, 10, 1);
#endif    
    OSSemPend(VideoCpleSemEvt, H264_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        // reset H264 hardware
        //SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
        //for(i=0;i<10;i++);
        //SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100; 
        
        DEBUG_H264("@@@ Ch%d Encoder Error: VideoCpleSemEvt is %d.\n", pVideoClipOption->VideoChannelID, err);
        DEBUG_H264("### Ch%d H264 encoder error. 0x%08x\n\n\n\n\n", pVideoClipOption->VideoChannelID, H264ENC_INTERRUPT);
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
        cfg->frame_num++;  
    }    
#if DINAMICALLY_POWER_MANAGEMENT
    sysMPEG_disable();
#endif

    OSSemPost(mpeg4ReadySemEvt);

    return cfg->H264StreamSize;    
}

/*

Routine Description:

    The FIQ handler of Multiple channel H264 encoder/decoder.

Arguments:

    None.

Return Value:

    None.

*/

void MultiChannelH264IntHandler(void)
{    
    u32                 enc_intStat;
    u32                 dec_intStat;
    VIDEO_CLIP_OPTION   *pVideoClipOption;

    pVideoClipOption    = &VideoClipOption[Video_Curr_Channel_ID];

    enc_intStat = H264ENC_INTERRUPT;    //encode
    dec_intStat = H264DEC_AVC_IS;       //decode
    
    if (enc_intStat & 0x00000002)       // encoder finish   
    {   
        H264ENC_ADDR_ENCODE_CTL0 = H264_ENC_CLR_INT; 

        OSSemPost(VideoCpleSemEvt);
        pVideoClipOption->mp4_avifrmcnt++;        
    } 
    else if(dec_intStat & 0x00000008)   // decoder finish
    {   
        H264DEC_AVC_IC      = 0x00000008; 
        if(dec_intStat & 0x00000006)
        {
            H264DEC_AVC_IC  = 0x00000006; 
            DEBUG_H264("### CH%d H264 decoder error. 0x%08x\n\n\n\n\n", Video_Curr_Channel_ID, dec_intStat);
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
        DEBUG_H264("@@@ CH%d  H264 decoder error. 0x%08x\n\n\n\n\n", Video_Curr_Channel_ID, dec_intStat);
    }
    else
    {
        DEBUG_H264("Ch%d MultiChannelH264IntHandler error!!!\n", Video_Curr_Channel_ID);
        DEBUG_H264("### H264 encoder error. 0x%08x\n\n\n\n\n",enc_intStat);
        DEBUG_H264("@@@ H264 decoder error. 0x%08x\n\n\n\n\n",dec_intStat);
    }
}


#endif
