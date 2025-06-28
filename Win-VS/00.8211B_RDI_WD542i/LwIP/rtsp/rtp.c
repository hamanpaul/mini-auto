#include "sysopt.h"

#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)

#include "general.h"
#include "rtp.h"
#include "api.h"

#include "Net.h"
#include "MPEG4api.h"
#include "IISapi.h"
#include "task.h"
#include "RFiuapi.h"
#include "UIapi.h"

#define RTP_TIMEOUT 20
#define RTP_BUF_NUM 32
//extern CONNECTION  rtsp_conxn[];	  //保存TCP連接時的信息
OS_STK StreamingRTPTaskStack[STREAMING_RTP_TASK_STACK_SIZE]; /* main task stack */
OS_STK WRAPRTPTaskStack[WRAP_RTP_TASK_STACK_SIZE-1];

u8  RTPBuf[RTP_BUF_NUM][1600];
u32 RTPSize[RTP_BUF_NUM];
u32 RTPServerPort[RTP_BUF_NUM];
u32 RTPClientPort[RTP_BUF_NUM];
u8 RTP_dest_hwaddr[RTP_BUF_NUM][6];

u8 RTP_PACKET[MAX_RTP_PAYLOAD_SIZE+12]; //Lsk_1222 TODO
extern u8  outbuf1[];
extern int RTCP_handler(RTP_session *);
RTP_session RTP_session_for_audio, RTP_session_for_video;
u8  EnableStreaming = 0;
u32	RTPVideoBufReadIdx;
u32 RTPVideoPresentTime;
u32	RTPAudioBufReadIdx;
u32 RTPAudioPresentTime;
volatile u8  WriteRTPBufIdx, ReadRTPBufIdx;

OS_EVENT*   RTPTrgSemEvt;
OS_EVENT*   RTPCmpSemEvt;
u8 err;
//aher
int iii=0;

struct netconn *audio_conn, *video_conn;
extern struct ip_addr remote_ip;
struct ip_addr remote_addr;

int AV_source = Local_playback; 
int AV_CH     = 0;

VIDEO_BUF_MNG *video_buf;
IIS_BUF_MNG   *audio_buf; 

extern VIDEO_BUF_MNG rfiuRxVideoBufMng[MAX_RFIU_UNIT][VIDEO_BUF_NUM]; 
extern u32 rfiuRxVideoBufMngWriteIdx[MAX_RFIU_UNIT];

extern IIS_BUF_MNG rfiuRxIIsSounBufMng[MAX_RFIU_UNIT][IIS_BUF_NUM]; 
extern u32 rfiuRxIIsSounBufMngWriteIdx[MAX_RFIU_UNIT];


void Init_RTP_Session(void)    
{
    RTP_session_for_audio.seq     = 1 + (rand() & 0xFFFF);
    RTP_session_for_audio.ssrc    = 1 + (rand() & 0xFFFFFFFF);
    RTP_session_for_audio.rtptime = 1 + (rand() & 0xFFFFFFFF);
    RTP_session_for_audio.rtcp_stats.pkt_count=0;
	RTP_session_for_audio.rtcp_stats.octet_count=0;
		
    RTP_session_for_video.seq     = 1 + (rand() & 0xFFFF);
    RTP_session_for_video.ssrc    = 1 + (rand() & 0xFFFFFFFF);
    RTP_session_for_video.rtptime = 1 + (rand() & 0xFFFFFFFF);
	RTP_session_for_video.rtcp_stats.pkt_count=0;
	RTP_session_for_video.rtcp_stats.octet_count=0;

    WriteRTPBufIdx = 0;
  	ReadRTPBufIdx = 0;
    
    /* Create the semaphore */
    RTPTrgSemEvt    = OSSemCreate(RTP_BUF_NUM); /* guarded for ping-pong buffer */
    RTPCmpSemEvt    = OSSemCreate(0);

	/* Create the task */
//aher
    if(iii==0)
	   OSTaskCreate(STREAMING_RTP_TASK, STREAMING_RTP_TASK_PARAMETER, STREAMING_RTP_TASK_STACK, STREAMING_RTP_TASK_PRIORITY); 
    OSTaskSuspend(STREAMING_RTP_TASK_PRIORITY);
 if(iii==0)
	OSTaskCreate(WRAP_RTP_TASK, WRAP_RTP_TASK_PARAMETER, WRAP_RTP_TASK_STACK, WRAP_RTP_TASK_PRIORITY); 
    OSTaskSuspend(WRAP_RTP_TASK_PRIORITY);
 iii=0;
}

extern u8 uiCaptureVideo(void);

void Start_RTP_Session(void)
{

    
    RTP_session_for_audio.pause=0;
    RTP_session_for_audio.started=1;
	RTP_session_for_video.pause=0;
    RTP_session_for_video.started=1;	
	
    EnableStreaming  = 1;    
	/**************************************
    **** Streaming Audio/Video Payload ****
    **************************************/
    

    if(AV_source == Local_record)
    {
        uiCaptureVideo();
        
       	RTPVideoBufReadIdx  = 0;//VideoBufMngWriteIdx;
    	RTPVideoPresentTime = 0; //Lsk TODO: msut add av time difference 

    	RTPAudioBufReadIdx  = 0;//iisSounBufMngWriteIdx;
    	RTPAudioPresentTime = 0;

        video_buf = VideoBufMng;
        audio_buf = iisSounBufMng; 
    }
    else if(AV_source == Local_playback)
    {
        uiFlowPlaybackMode(UI_KEY_PLAY);//In playback thumbnail, send KE to playfile
        
    	RTPVideoBufReadIdx  = 0;//VideoBufMngWriteIdx;
    	RTPVideoPresentTime = 0; //Lsk TODO: msut add av time difference 

    	RTPAudioBufReadIdx  = 0;//iisSounBufMngWriteIdx;
    	RTPAudioPresentTime = 0;

        video_buf = VideoBufMng;
        audio_buf = iisSounBufMng; 
    }
    else if(AV_source == RX_receive)
    {
    	RTPVideoBufReadIdx  = rfiuRxVideoBufMngWriteIdx[AV_CH];//VideoBufMngWriteIdx;
    	RTPVideoPresentTime = 0; //Lsk TODO: msut add av time difference 

    	RTPAudioBufReadIdx  = rfiuRxIIsSounBufMngWriteIdx[AV_CH];//iisSounBufMngWriteIdx;
    	RTPAudioPresentTime = 0;

        video_buf = rfiuRxVideoBufMng[AV_CH];
        audio_buf = rfiuRxIIsSounBufMng[AV_CH]; 
    }
    else if(AV_source == RX_transcoder)
    {
    	//RTPVideoBufReadIdx  = 0;//VideoBufMngWriteIdx;
    	//RTPVideoPresentTime = 0; //Lsk TODO: msut add av time difference 

    	//RTPAudioBufReadIdx  = 0;//iisSounBufMngWriteIdx;
    	//RTPAudioPresentTime = 0;
    }

    

    DEBUG_RTSP("RTPVideoBufReadIdx = %d\n\n\n",RTPVideoBufReadIdx);
    DEBUG_RTSP("RTPAudioBufReadIdx = %d\n\n\n",RTPAudioBufReadIdx);
    //aher
    if(iii!=111){
    /* Create new UDP connect */
    audio_conn = netconn_new(NETCONN_UDP);     /* create a new connection */
    video_conn = netconn_new(NETCONN_UDP);
    
    netconn_bind(audio_conn, NULL, RTP_session_for_audio.transport.server_ports.RTP);
    netconn_bind(video_conn, NULL, RTP_session_for_video.transport.server_ports.RTP);
    
    netconn_connect(audio_conn, &remote_ip, RTP_session_for_audio.transport.client_ports.RTP);        /* connect the connection to the remote */
    netconn_connect(video_conn, &remote_ip, RTP_session_for_video.transport.client_ports.RTP);  
iii=111;
}	
    OSTaskResume(WRAP_RTP_TASK_PRIORITY);
    OSTaskResume(STREAMING_RTP_TASK_PRIORITY);
}

void Pause_RTP_Session(void)
{
	//printf("aher : PAUSE\n");
    uiFlowPlaybackMode(UI_KEY_PLAY);//In playback thumbnail, send KE to pause
    RTP_session_for_audio.pause=1;
    RTP_session_for_audio.started=0;
	RTP_session_for_video.pause=1;
    RTP_session_for_video.started=0;
	//    /*設置其會話狀態為暫停*/
    EnableStreaming  = 0;   

    OSTaskSuspend(WRAP_RTP_TASK_PRIORITY);
    OSTaskSuspend(STREAMING_RTP_TASK_PRIORITY);
}
void Stop_RTP_Session(void)
{
//aher 
//printf("aher : STOP\n");
uiFlowPlaybackMode(UI_KEY_STOP);
//
    RTP_session_for_audio.pause=1;
    RTP_session_for_audio.started=0;
	RTP_session_for_video.pause=1;
    RTP_session_for_video.started=0;
		   //    /*設置其會話狀態為暫停*/
//	RTCP_handler(&RTP_session_for_audio);
//	RTCP_handler(&RTP_session_for_video);	   
   
    EnableStreaming  = 0;   


    /* dealloc new UDP connect */
    netconn_delete(audio_conn);     
    netconn_delete(video_conn); 
    //aher 
    OSTaskSuspend(WRAP_RTP_TASK_PRIORITY);
    OSTaskSuspend(STREAMING_RTP_TASK_PRIORITY);	
    //
}

/*************************************************************************/
void Pack_RTP(RTP_session *session, unsigned char marker, unsigned char PT, u32 time,
										 unsigned int payload_size, unsigned char* buf)
{
	//ETH_HEADER, IP_HEADER, UDP_HEADER, RTP_header, RTP payload
    //0~13      , 14~33    , 34~42	   , 42~53
   	RTP_header *hdr;
	u8 *StreamingBuf;
	u8 *data;
    //DEBUG_GPIU("W");
	RTPSize[WriteRTPBufIdx] = sizeof(RTP_header)+payload_size; 
	RTPServerPort[WriteRTPBufIdx] = session->transport.server_ports.RTP;
	RTPClientPort[WriteRTPBufIdx] = session->transport.client_ports.RTP; 	

	memcpy(RTP_dest_hwaddr[WriteRTPBufIdx], session->dest_hwaddr, 6);
	StreamingBuf = RTPBuf[WriteRTPBufIdx];

	//RTP level	
	data = (StreamingBuf + sizeof(RTP_header));
	hdr = (RTP_header*)(StreamingBuf);			
    hdr->version   = 2;
    hdr->padding   = 0;
    hdr->extension = 0;
    hdr->csrc_len  = 0;    
    hdr->marker    = marker;
    hdr->payload   = PT;
    hdr->seq_no    = bSwap16(session->seq);
    if(AV_source == Local_playback) //real-time
   	    hdr->timestamp = bSwap32((session->rtptime + (time /1000)) & 0xFFFFFFFF);
    else                            //only chunk-time
	    hdr->timestamp = bSwap32(session->rtptime);
  	hdr->ssrc      = bSwap32(session->ssrc);		
	mcpu_ByteMemcpy(data, buf, payload_size);
	
    session->seq = (session->seq + 1) & 0xFFFF;	
    if(AV_source == Local_playback) //real-time
        ;
    else                            //only chunk-time
    {
        if(marker)        
    	    session->rtptime = (session->rtptime + time) & 0xFFFFFFFF;
    }
    session->rtcp_stats.pkt_count++;
    session->rtcp_stats.octet_count+=payload_size;	
	WriteRTPBufIdx = (WriteRTPBufIdx + 1) % RTP_BUF_NUM;
}

void Pack_PCM_RTP(u32 time, u32 size,	u8* buffer)
{		
    int marker;
    unsigned int payload_size;
	  					    
	/* if frame size too large, split into multiple RTP packet*/
	while(size > 0)
	{
	    OSSemPend(RTPTrgSemEvt, RTP_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            continue;
        }
        if(size > MAX_RTP_PAYLOAD_SIZE)
        {
        	payload_size = MAX_RTP_PAYLOAD_SIZE;
        	size -= MAX_RTP_PAYLOAD_SIZE;
        	marker = 0;				
        }	
        else
        {
        	payload_size = size;
            size = 0;
        	marker = 1;							 
        }		
        Pack_RTP(&RTP_session_for_audio, marker, PT_FOR_PCM_16K, time*(16000/1000), payload_size, buffer);
        buffer += payload_size;
		OSSemPost(RTPCmpSemEvt);
		
		//RTCP_handler(&RTP_session_for_audio);
	}
}
void Pack_mpeg4_RTP(u32 time,	u32	size, u8* buffer)
{		
    int marker;
    unsigned int payload_size;
	  					    
	/* if frame size too large, split into multiple RTP packet*/
	while(size > 0)
	{
	    OSSemPend(RTPTrgSemEvt, RTP_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            //printf("@");
            continue;
        }
	    if(size > MAX_RTP_PAYLOAD_SIZE)
		{
			payload_size = MAX_RTP_PAYLOAD_SIZE;
			size -= MAX_RTP_PAYLOAD_SIZE;
			marker = 0;				
		}	
		else
	    {
	  	    payload_size = size;
            size = 0;
			marker = 1;							 
		}				
		Pack_RTP(&RTP_session_for_video, marker, PT_FOR_MPEG4, time*(90000/1000), payload_size, buffer);
		buffer += payload_size;	
		OSSemPost(RTPCmpSemEvt);
		//RTCP_handler(&RTP_session_for_video);
	}
}

void WrapRTPTask(void* pData)
{	
	u16 video_value;
	u16 video_value_max;
	u32 RTPVideoSkipTime=0;

	u16 audio_value;
	u16 audio_value_max;
	u32 video_skip_duration = 0;

    u32 audio_cnt = 0;
    u32 video_cnt = 0;    
	
    while(EnableStreaming)
    {   
        //printf("<%d,%d>",audio_cnt,video_cnt);
        if(video_value == 0) //avoid VideoTask deadlock
        {
            OSTimeDly(1);
        }		
        #if 1
        // ------Streaming audio payload------//
        if((video_value == 0) || (RTPAudioPresentTime <= RTPVideoPresentTime))
        { 
            audio_value = OSSemAccept(AudioRTPCmpSemEvt[AV_CH]);
            if (audio_value > 0) {
                audio_cnt ++;
                if(audio_value_max < audio_value)
                	{
                    audio_value_max = audio_value;
                    //printf("<A>");
                	}
                if(AV_source == Local_playback) //real-time
                    audio_buf[RTPAudioBufReadIdx].time = RTPAudioPresentTime; // chunk_time fixed
				Pack_PCM_RTP(audio_buf[RTPAudioBufReadIdx].time, audio_buf[RTPAudioBufReadIdx].size, audio_buf[RTPAudioBufReadIdx].buffer);                         
                 if(AV_source == Local_playback) //real-time
                    RTPAudioPresentTime += 128000;// chunk_time fixed
                else                            //only chunk-time
				RTPAudioPresentTime += audio_buf[RTPAudioBufReadIdx].time;
                RTPAudioBufReadIdx = (RTPAudioBufReadIdx + 1) % IIS_BUF_NUM;				
            }
        }
        #endif
        #if 1
        //------ Streaming video payload------//            
        if((audio_value == 0) || (RTPAudioPresentTime >= RTPVideoPresentTime)) 
		{
            video_value = OSSemAccept(VideoRTPCmpSemEvt[AV_CH]);  
            if (video_value > 0) 
            {
                video_cnt++;
                if(video_value_max < video_value)
                	{
                    video_value_max = video_value;
                    //printf("<V>");
                	}
                #if 1 //all frame
                Pack_mpeg4_RTP(video_buf[RTPVideoBufReadIdx].time + RTPVideoSkipTime, video_buf[RTPVideoBufReadIdx].size, video_buf[RTPVideoBufReadIdx].buffer);                        
                RTPVideoSkipTime = 0;
                #endif
                #if 0 //only send I frame
                if(video_buf[RTPVideoBufReadIdx].flag == 1)//I frame
                {
                    Pack_mpeg4_RTP(video_buf[RTPVideoBufReadIdx].time + video_skip_duration, video_buf[RTPVideoBufReadIdx].size, video_buf[RTPVideoBufReadIdx].buffer);                    
                    video_skip_duration = 0;
                }
                else//P frame
                {
                    video_skip_duration += video_buf[RTPVideoBufReadIdx].time;
                }
                #endif
                 if(AV_source == Local_playback) //real-time
                    RTPVideoPresentTime = video_buf[RTPVideoBufReadIdx].time;
                else                            //only chunk-time    
				    RTPVideoPresentTime += video_buf[RTPVideoBufReadIdx].time;
				RTPVideoBufReadIdx = (RTPVideoBufReadIdx + 1) % VIDEO_BUF_NUM;                                                
            }
        }
        #endif
		//------------------- Bitstream buffer control---------------------------------//
        /*
             Lsk: 以Video bitstream buffer 內的index剩餘個數為偵測點,若大於 ASF_DROP_FRAME_THRESHOLD
                     則為網路速度過慢,需drop frame.

        */
        #if 1
    	if(VideoRTPCmpSemEvt[AV_CH]->OSEventCnt > (VIDEO_BUF_NUM - 60))
		{
            do 
			{
                video_value = OSSemAccept(VideoRTPCmpSemEvt[AV_CH]); 
				if (video_value > 0)
				{
                    if(video_value_max < video_value)
                        video_value_max = video_value;

					//RTPSkipVideo                    
                	RTPVideoSkipTime += video_buf[RTPVideoBufReadIdx].time;
                	RTPVideoBufReadIdx = (RTPVideoBufReadIdx + 1) % VIDEO_BUF_NUM;
                    DEBUG_RTSP("*");
                }
                else
                    DEBUG_RTSP("^");
            } while(video_buf[RTPVideoBufReadIdx].flag != FLAG_I_VOP);
        }
        #endif
    }                       
}

void StreamingRTPTask(void* pData)
{        
    struct netbuf *buf;
    u32 audio_cnt = 0;
    u32 video_cnt = 0;    

    while(EnableStreaming)
	{

        //printf("(%d,%d)",audio_cnt,video_cnt);
        
		OSSemPend(RTPCmpSemEvt, RTP_TIMEOUT, &err);
        if (err != OS_NO_ERR)
        {
            printf("@");
            continue;
        }
        //printf("S");
        /* create a new netbuf */
        buf = netbuf_new();
        /* reference the text into the netbuf */
        //printf("<%d,%d>",RTPSize[ReadRTPBufIdx],(u16_t)RTPSize[ReadRTPBufIdx]);
        netbuf_ref(buf, RTPBuf[ReadRTPBufIdx], (u16_t)RTPSize[ReadRTPBufIdx]);
        //printf("1.%d,%s\n",ReadRTPBufIdx,RTPBuf[ReadRTPBufIdx]);        
        //printf("2.%s\n",buf->p->payload);
        /* send the text */        
        if(RTPClientPort[ReadRTPBufIdx] == RTP_session_for_audio.transport.client_ports.RTP)
        {
            //printf("@A@\n");
            //printf("<%d,%d>\n",RTPSize[ReadRTPBufIdx],(u16_t)RTPSize[ReadRTPBufIdx]);
            audio_cnt++;
            netconn_send(audio_conn, buf);
            //netconn_write(audio_conn, RTPBuf[ReadRTPBufIdx], RTPSize[ReadRTPBufIdx],NETCONN_COPY);
        }
        else if(RTPClientPort[ReadRTPBufIdx] == RTP_session_for_video.transport.client_ports.RTP)
        {
            //printf("@V@\n");
            //printf("<%d,%d>\n",RTPSize[ReadRTPBufIdx],(u16_t)RTPSize[ReadRTPBufIdx]);
            video_cnt++;
            netconn_send(video_conn, buf);
            //netconn_write(video_conn, RTPBuf[ReadRTPBufIdx], RTPSize[ReadRTPBufIdx],NETCONN_COPY);
        }
        /* deallocate netbuf */        
        netbuf_delete(buf);
        
		ReadRTPBufIdx = (ReadRTPBufIdx + 1) % RTP_BUF_NUM;
		OSSemPost(RTPTrgSemEvt);
	}
}
#else
#include "general.h"

u8  EnableStreaming = 0;
#endif //#if (NIC_SUPPORT && !ICOMMWIFI_SUPPORT)
