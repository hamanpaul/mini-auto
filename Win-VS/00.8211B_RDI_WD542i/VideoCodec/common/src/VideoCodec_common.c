/*

Copyright (c) 2011 Mars Semiconductor Corp.

Module Name:

    VideoCodec_common.c

Abstract:

    The common routines of Video Codec.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2011/09/01  Lsk  Create  

*/

#include <stdio.h>
#include <stdlib.h>

#include "general.h"
#include "Task.h"
#include "isuapi.h"
#include "VideoCodec_common.h"
#include "VideoCodecAPI.h"
#include "MPEG4api.h"
#include "asfapi.h"
#include "sysapi.h"
#if MULTI_CHANNEL_SUPPORT
#include "ciuapi.h"
#endif
#include "H264api.h"

/*
 *********************************************************************************************************
 * Function Pointer for Video Codec(MotionJPEG, MPEG4, H264)
 *********************************************************************************************************
 */
int (*VideoCodecModule_SW_Reset)();
u32 (*VideoCodecDecideFrameType)(H264_ENC_CFG *, u32, u32);
u32 (*VideoCodecEncodingOneFrame)(VIDEO_INFO *, H264_ENC_CFG *);
u32 (*VideoCodecDecodingOneFrame)(VIDEO_INFO *, H264_DEC_CFG *);
u32 (*VideoCodecSetResolution)(H264_ENC_CFG *, u16 , u16);

int (*RataControlInit)();
//int (*GetQP)();
int (*UpdatRCModel)();



/*
*********************************************************************************************************
* Variable
*********************************************************************************************************
*/

u32  IntraPeriod = 60; //Int

/* task and event related */
OS_STK      VideoTaskStack[VIDEO_TASK_STACK_SIZE]; 
//OS_EVENT*   VideoTrgSemEvt;
//OS_EVENT*   VideoCmpSemEvt;
//OS_EVENT*   VideoCpleSemEvt; 

u8 Video_Task_Mode;     // 0: record, 1: playback
u8 Video_Task_Error;
u8 Video_Task_Pend;
u8 Video_Task_Go;       // 0: never run, 1: ever run

/* buffer management */
u8* VideoBufEnd;
u8 *VideoNRefBuf_Y;
u8 *VideoNRefBuf_Cb;
u8 *VideoPRefBuf_Y;
u8 *VideoPRefBuf_Cb;

//VIDEO_BUF_MNG VideoBufMng[VIDEO_BUF_NUM];  //Lsk : for compiler pass
//VIDEO_BUF_MNG VideoBufMng[VIDEO_BUF_NUM]; 


extern u8 Video_Mode;     // 0: record, 1: playback
u8 Video_Status;
u8 Video_Task_Go;  // 0: never run, 1: ever run
u8 Video_Error;
extern u8 Video_Pend;

/* buffer management */


/* Contain relate */
//u32 VideoTimeStatistics=0;
extern u32 CurrentVideoSize;

VIDEO_INFO video_info;        
/*
 *********************************************************************************************************
 * Extern Variable
 *********************************************************************************************************
 */
#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
extern u8 ResetPlayback;  			//Lsk 090401 : reset playback system 
#endif

extern u8      SetIVOP;
extern u32 IsuIndex;
extern  s64 Videodisplaytime[DISPLAY_BUF_NUM];
/*
*********************************************************************************************************
* Function prototype
*********************************************************************************************************
*/

/*

Routine Description:

    Initialize Video Codec task.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 VideoCodecInit(void)
{
    u32 i;

    #if (VIDEO_CODEC_OPTION == MJPEG_CODEC)	
    //VideoDecideFrameType = ;
    //VideoEncodingOneFrame = ;
    //VideoRataControlInit = ;
    //VideoDecodingOneFrame = ;
    #elif (VIDEO_CODEC_OPTION == MPEG4_CODEC)	
    //VideoDecideFrameType = ;    
    //VideoEncodingOneFrame = ;
    //VideoRataControlInit = ;
    //VideoDecodingOneFrame = ;   
    #elif (VIDEO_CODEC_OPTION == H264_CODEC)	    
    VideoCodecDecideFrameType  = H264Enc_DecSliceType;    
    VideoCodecSetResolution    = H264Enc_SetResolution;
    VideoCodecEncodingOneFrame = H264Enc_CompressOneFrame;
    VideoCodecDecodingOneFrame = H264Dec_DecompressOneFrame;

    H264Enc_Init();
    //VideoRataControlInit = ;    
    //VideoDecodingOneFrame = ;   
    #endif
    ////Lsk TO DO : software reset
    #if 0  //software reset 
    // MPEG-4 software reset
    SYS_RSTCTL = 0x00000100;
    for (i = 0; i < 256; i++);
    SYS_RSTCTL = 0;
    #endif
    /* initialize video buffer */
    for(i = 0; i < VIDEO_BUF_NUM; i++) {        
        VideoBufMng[i].buffer = VideoBuf;
    }
    
    VideoNRefBuf_Y  = mpeg4NRefBuf_Y;       //0x0110, Input reference image Y addres
    VideoNRefBuf_Cb = mpeg4NRefBuf_Cb;      //0x0114, Input reference image UV addres  
    VideoPRefBuf_Y  = mpeg4PRefBuf_Y;        //0x0118, Output reconstructed image Y address
    VideoPRefBuf_Cb = mpeg4PRefBuf_Cb;
    VideoBufEnd     = mpeg4VideBufEnd;
    
          

    //Lsk TO DO : video codec and Rate control init 
    // initialize rate control parameter
    //memset(&mpRateControlParam,0,sizeof(DEF_RATECONTROL_PARA));
    //RCQ2_init(&mpRateControlParam);
    
    /* Create the semaphore */
    VideoTrgSemEvt  = OSSemCreate(VIDEO_BUF_NUM - 2); /* guarded for ping-pong buffer */
    VideoCmpSemEvt  = OSSemCreate(0);
    VideoCpleSemEvt = OSSemCreate(0); 
    for(i=0; i<MULTI_CHANNEL_MAX; i++)
        VideoRTPCmpSemEvt[i] = OSSemCreate(0);
    
    /* Create Video Codec task */
    OSTaskCreate(VIDEO_TASK, VIDEO_TASK_PARAMETER, VIDEO_TASK_STACK, VIDEO_TASK_PRIORITY); 
    VideoTaskSuspend();        
    return 1;   
}



/*

Routine Description:

    The Video Codec task.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
void VideoTask(void* pData)
{
    u32*        pFlag;
    s64*        pTime;
    s64         Time;    
    u32*        pSize;
    u8          *pBuf, *pReadBuf;
    u8          err;
    u32         NextIdx; 
    u8          write2dispbuf_en;
    s32         i, DropFrame;
    u32         Vop_Result;
    u32         Vop_Type;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

 
    
    Video_Task_Go   = 1;    // 0: never run, 1: ever run
    
    while (1)
    {
        Video_Pend  = 1;
        OSSemPend(VideoTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
        Video_Pend  = 0;
    
        /*CY 0613 E*/
        if (err != OS_NO_ERR)
        {
            DEBUG_H264("Error: H264TrgSemEvt is %d.\n", err);        
        }
        
        if(Video_Mode)
        {   
            pBuf        = VideoBufMng[VideoBufMngReadIdx].buffer;  /*BJ 0530 S*/
            pFlag       = &VideoBufMng[VideoBufMngReadIdx].flag;    
            Time        = VideoBufMng[VideoBufMngReadIdx].time; 
            pSize       = &VideoBufMng[VideoBufMngReadIdx].size;    
            Vop_Type    = (*pFlag)? 0 : 1;
            
            write2dispbuf_en    = 1;
            
			#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
			if(!ResetPlayback && (sysPlaybackForward == 0 ||  Vop_Type == I_FRAME)) 
			#elif (AVSYNC == VIDEO_FOLLOW_AUDIO)
            if((sysPlaybackForward == SYS_PLAYBACK_FORWARD_X1) || 
               ((sysPlaybackForward != SYS_PLAYBACK_FORWARD_X1) && (Vop_Type == I_FRAME))) 
			#endif
            {
                video_info.StreamBuf = pBuf;
                video_info.pSize     = pSize;            
                Vop_Result  = (VideoCodecDecodingOneFrame)(&video_info, &H264Dec_cfg);
                
                if(write2dispbuf_en)//不啟動scaler engine. Mpeg decoding 直接decoding to display buffer.
                {                    
                    isuStatus_OnRunning = 0;
                    IsuIndex++;                                        
                }
                else
                {
                    // scaling Video image to fit display format
                    if(IsuIndex != 0) 
                    {
                        
                        OSSemPend(isuSemEvt, 10 ,&err);
                     #if DINAMICALLY_POWER_MANAGEMENT
                        sysISU_disable();
                     #endif          
                        if (err != OS_NO_ERR) {
                            DEBUG_MP4("Error: isuSemEvt(playback mode) is %d.\n", err);
                        }
                    }
                }
                

				#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
				while(((IsuIndex >= (MainVideodisplaybuf_idx + DISPLAY_BUF_NUM - 2)) || (isuStatus_OnRunning == 1)) && !ResetPlayback && sysPlaybackVideoStop == 0) /*Peter 1113 S*/    //Lsk 090410 check it    //Lsk 090417 : avoid deadlock when press stop playback           
				#elif (AVSYNC == VIDEO_FOLLOW_AUDIO)
                while((IsuIndex >= (MainVideodisplaybuf_idx + DISPLAY_BUF_NUM - 2)) || (isuStatus_OnRunning == 1)) /*Peter 1113 S*/                   
				#endif
                {
                    //DEBUG_MP4("ISU waiting for display, IsuIndex = %d, MainVideodisplaybuf_idx = %d\n", IsuIndex, MainVideodisplaybuf_idx);
                    OSTimeDly(1);
                }
                Videodisplaytime[IsuIndex % DISPLAY_BUF_NUM]    = Time;

                
                if(!write2dispbuf_en)
                {
                #if DINAMICALLY_POWER_MANAGEMENT
                    sysISU_enable();
                #endif          
                    isuPlayback_av(mpeg4outputbuf[IsuIndex % 3],
                                   MainVideodisplaybuf[IsuIndex % DISPLAY_BUF_NUM], 
                                   Mp4Dec_opt.Width, 
                                   Mp4Dec_opt.Height);
                }

            } 
			#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
			else if(Vop_Type == P_FRAME)
			#elif (AVSYNC == VIDEO_FOLLOW_AUDIO)
            else if((sysPlaybackForward != SYS_PLAYBACK_FORWARD_X1) && (Vop_Type == P_FRAME)) 
			#endif
            {
                while((((VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM) == VideoBufMngWriteIdx && CloseFlag) && (sysPlaybackVideoStop == 0) && (ResetPlayback == 0)) //Lsk 090417 : avoid deadlock when press stop playback
                {
                    OSTimeDly(1);
                }
                VideoPictureIndex++;
                VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
            }
        }
        else //--------------------------- MPEG-4 capture Mode------------------------------------
        {  
            pBuf        =  VideoBufMng[VideoBufMngWriteIdx].buffer;  /*BJ 0530 S*/
            pFlag       = &VideoBufMng[VideoBufMngWriteIdx].flag; 
            pTime       = &VideoBufMng[VideoBufMngWriteIdx].time; 
            pSize       = &VideoBufMng[VideoBufMngWriteIdx].size; 
            *pFlag      = (VideoCodecDecideFrameType)(&H264Enc_cfg,VideoPictureIndex, IntraPeriod);   // I: 1, P: 0
                                                                
            //--------Video rate control: Calculate QP-----//            
            //move to H264Enc_CompressOneFrame
            
            //------Video FIFO Management: 避免 Read/Write pointer overlay. ------//
            if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
                if((WantChangeFile == 1) && (GetLastVideo == 0) && (GetLastAudio)) {
                    OS_ENTER_CRITICAL();
                    LastVideo           = VideoBufMngWriteIdx;
                    GetLastVideo        = 1;
                    OS_EXIT_CRITICAL();

                #if CDVR_LOG
                    ChangeLogFileStartAddress();
                #endif
                }
            }

            pReadBuf    = VideoBufMng[VideoBufMngReadIdx].buffer;   
            if(VideoPictureIndex != 0 && (pReadBuf >= pBuf)) 
            {
                while((pReadBuf >= pBuf) &&
                      ((pReadBuf - pBuf) < VIDEO_MIN_BUF_SIZE) && 
                      (VideoCmpSemEvt->OSEventCnt > 2) &&
                      (sysCaptureVideoStop == 0)) 
                {
                    DEBUG_MP4("y");  
                    OSTimeDly(1);
                    pReadBuf    = VideoBufMng[VideoBufMngReadIdx].buffer;

                    // GetLastVideo一直為0, 可能會造成dead lock, 所以要更新GetLastVideo
                    if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
                        if((WantChangeFile == 1) && (GetLastVideo == 0) && (GetLastAudio)) {
                            OS_ENTER_CRITICAL();
                            LastVideo           = VideoBufMngWriteIdx;
                            GetLastVideo        = 1;
                            OS_EXIT_CRITICAL();

                        #if CDVR_LOG
                            ChangeLogFileStartAddress();
                        #endif
                        }
                    }
                }
            }
            //---------- Video Encoding---------//
            *pTime=0;

            #if 1 //Lucian: frame rate control
            if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_30)
            {
                DropFrame=0;
            }
            else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_15)
            {
                DropFrame=1;
            }
            else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_10)
            {
                DropFrame=2;
            }
            else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_5)
            {
                DropFrame=5;
            }
            
            #if MULTI_CHANNEL_SUPPORT
            #if (MULTI_CHANNEL_SEL & 0x01)
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(isuSemEvt, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
            #elif (MULTI_CHANNEL_SEL & 0x02)
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH1, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                
                VideoPictureIndex++;
                mp4_avifrmcnt++;
                
            }
            #elif (MULTI_CHANNEL_SEL & 0x04)
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH2, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
            #elif (MULTI_CHANNEL_SEL & 0x08)
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH3, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
            #elif (MULTI_CHANNEL_SEL & 0x10)
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(ciuCapSemEvt_CH4, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
            #endif
            #else   // MULTI_CHANNEL_SUPPORT == 0
            for(i = 0; i < DropFrame; i++)
            {
                OSSemPend(isuSemEvt, 4 ,&err);
                *pTime += ISUFrameDuration[VideoPictureIndex % ISU_FRAME_DURATION_NUM]; /* Peter 081007 */
                VideoPictureIndex++;
                mp4_avifrmcnt++;
            }
            #endif  // #if MULTI_CHANNEL_SUPPORT
            #endif  //#if 1 //Lucian: frame rate control

            //Lsk TO DO:
            video_info.StreamBuf = pBuf;
            video_info.FrameType = *pFlag;
            video_info.FrameIdx  = VideoPictureIndex;  
            video_info.FrameTime = pTime;
            video_info.pSize     = pSize;            
            *pSize = (VideoCodecEncodingOneFrame)(&video_info, &H264Enc_cfg);
			        
            OS_ENTER_CRITICAL();
            VideoTimeStatistics += *pTime;

            if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && (VideoTimeStatistics >= (asfSectionTime * 1000)))            
            {
                SetIVOP = 1;
                video_info.ResetFlag = 1;
            } 
            if((asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && (VideoTimeStatistics >= (asfRecTimeLen * 1000)))            
            {
                SetIVOP = 1; 
                video_info.ResetFlag = 1;
            }
            OS_EXIT_CRITICAL();

            //--------Rebuild RC model of Rate control ------//
            //move to H264Enc_CompressOneFrame()
            //-------------Video FIFO management: 計算下一個Video frame start address-------------//
            OS_ENTER_CRITICAL();
            CurrentVideoSize   += *pSize;
            OS_EXIT_CRITICAL();
        
            NextIdx = (VideoBufMngWriteIdx + 1) % VIDEO_BUF_NUM;
            
            pBuf    = (u8*)(((u32)pBuf + *pSize + 3) & ~3);
            
            if(pBuf > VideoBufEnd) 
            {
                DEBUG_MP4("VideoBuf overflow!!!!\n");
            }
            
            if(pBuf < (VideoBufEnd - VIDEO_MIN_BUF_SIZE))
                VideoBufMng[NextIdx].buffer = pBuf;
            else
                VideoBufMng[NextIdx].buffer = VideoBuf;
                
            VideoBufMngWriteIdx = NextIdx;
            VideoPictureIndex++;    
        }
        
        OSSemPost(VideoCmpSemEvt);  
    }
}


/*

Routine Description:

    Resume Video Codec task.

Arguments:

    None.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 VideoTaskResume(void)
{
    video_info.ResetFlag = 1;
    OSTaskResume(VIDEO_TASK_PRIORITY);    
    return 1;
}

/*

Routine Description:

    Suspend Video Codec task.

Arguments:

    None.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 VideoTaskSuspend(void)
{
    /* Suspend the task */
  	Video_Task_Go   = 0;    // 0: never run, 1: ever run  //Lsk 090622
    OSTaskSuspend(VIDEO_TASK_PRIORITY);
    return 1;
}



