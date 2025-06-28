/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    WrapRf.c

Abstract:

    The routines of FR Interface Unit.
    
    
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2012/2/9  Lucian Yuan  Create  
*/

#include "general.h"
#include "board.h"
#include "task.h"
#include "sysapi.h"
#include "gpioapi.h"
#include "timerapi.h"
#include "mpeg4api.h"
#include "iisapi.h"
#include "rfiuapi.h"
#include "rfiureg.h"
#include "rfiu.h"
#include "H264api.h"
#include "adcapi.h"
#include "VideoCodecAPI.h"
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
#include "GlobalVariable.h"
#endif
#include "uiapi.h"
/*
 *********************************************************************************************************
 *  SYS Constant
 *********************************************************************************************************
 */
#define PS_START_CODE_PREFIX       0x00ff7f7e


//----------Only use bit 5~0 ------------//
//-----Data-------//
#define PS_IMAGETYPE_JPEG          0x00
//-----Video------//
#define PS_VIDEOTYPE_MPEG4         0x10
#define PS_VIDEOTYPE_H264          0x11
#define PS_VIDEOTYPE_MJPG          0x12

//-----Audio------//
#define PS_AUDIOTYPE_8K8B_PCM      0x20
#define PS_AUDIOTYPE_16K8B_PCM     0x21
#define PS_AUDIOTYPE_16K16B_PCM    0x22

#define PS_AUDIOTYPE_8K8B_ADPCM    0x30
#define PS_AUDIOTYPE_16K8B_ADPCM   0x31
#define PS_AUDIOTYPE_16K16B_ADPCM  0x32
//Lucian: bit6~7 is reserved for check
//-------------------------------------------//

#define PS_STA_IFRAME              0x01
#define PS_STA_CURRENT_IND         0x02
#define PS_STA_LASTONE             0x04
#define PS_STA_PTS                 0x08
#define PS_STA_SCRAMBLE            0x10


#define PS_MAX_HEADERLEN           16
#define PS_VIDEO_HEADERLEN         16
#define PS_AUDIO_HEADERLEN         16

#define PS_DATA_HEADERLEN          8

typedef struct {
    int StreamType;
    int PayLoadSize;
    int HeaderSize;
    int Status;
	int PreStatus;
    int SeqNum;
    int PTS_L;
    int BotFldOffset;
} DEF_RF_PS_HEADER;
/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
OS_STK rfiuWrapTaskStack_Unit0[RFIU_WRAP_TASK_STACK_SIZE_UNIT0]; 
OS_STK rfiuWrapTaskStack_Unit1[RFIU_WRAP_TASK_STACK_SIZE_UNIT1]; 
OS_STK rfiuWrapTaskStack_Unit2[RFIU_WRAP_TASK_STACK_SIZE_UNIT2]; 
OS_STK rfiuWrapTaskStack_Unit3[RFIU_WRAP_TASK_STACK_SIZE_UNIT3]; 

OS_STK rfiuDecTaskStack_Unit0[RFIU_DEC_TASK_STACK_SIZE_UNIT0]; 
OS_STK rfiuDecTaskStack_Unit1[RFIU_DEC_TASK_STACK_SIZE_UNIT1]; 
OS_STK rfiuDecTaskStack_Unit2[RFIU_DEC_TASK_STACK_SIZE_UNIT2]; 
OS_STK rfiuDecTaskStack_Unit3[RFIU_DEC_TASK_STACK_SIZE_UNIT3]; 

VIDEO_BUF_MNG rfiuRxVideoBufMng[MAX_RFIU_UNIT][VIDEO_BUF_NUM]; 
u32 rfiuRxVideoBufMngWriteIdx[MAX_RFIU_UNIT];



IIS_BUF_MNG rfiuRxIIsSounBufMng[MAX_RFIU_UNIT][IIS_BUF_NUM]; 
u32 rfiuRxIIsSounBufMngWriteIdx[MAX_RFIU_UNIT];
u32 rfiuRxIIsSounBufMngReadIdx[MAX_RFIU_UNIT];

#if RX_SNAPSHOT_SUPPORT
DATA_BUF_MNG rfiuRxDataBufMng[MAX_RFIU_UNIT]; 
#endif


u32 rfiuAudioBufPlay_idx[MAX_RFIU_UNIT];
u32 rfiuAudioBufFill_idx[MAX_RFIU_UNIT];
u8 *rfiuMainAudioPlayDMANextBuf[IISPLAY_BUF_NUM];
u8 rfiuAudioZeroBuf[RFI_AUIDIO_SILENCE_SIZE];
u8 rfiuAudioDummyBuf[RFI_AUIDIO_SILENCE_SIZE];
// for 語音雙向
u8 *rfiuAudioRetDMANextBuf[RFI_AUDIO_RET_BUF_NUM];
u32 rfiuAudioRetRec_idx;
u32 rfiuAudioRetRead_idx;

u32 rfiuAudioRetPlay_idx;
u32 rfiuAudioRetWrite_idx;

u32 rfiuAudioRetFromApp;   

u32 rfiuAudioRetStatus;

#if DETECT_RX_VOLUME
u32 RxVolume[MAX_RFIU_UNIT];
#endif
#if ASF_DEBUG_ENA
extern u32 RX_time_A, RX_time_V;
extern u32 RX_skip_A, RX_skip_V;
extern u32 RX_sem_A, RX_sem_V;
#endif
#if AUDIO_DEBUG_ENA
u8  RF_Audiosync = 0;
#endif

u8 rfAudioEnable = 0;
u8 rfMuteVox = 0;
#if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
u32 Video_totaltime[MAX_RFIU_UNIT]={0,0,0,0};
u32 Audio_totaltime[MAX_RFIU_UNIT]={0,0,0,0};
u32 Lose_video_time[MAX_RFIU_UNIT]={0,0,0,0};
u32 Lose_audio_time[MAX_RFIU_UNIT]={0,0,0,0};
#endif

#if( (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) || (SW_APPLICATION_OPTION  ==  MR8100_BABYMONITOR) )
u32 rfiuPrevWifiStatus[MAX_RFIU_UNIT]={0,0,0,0};
#endif
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
extern OS_EVENT    *gRfiuSWReadySemEvt;
 
#if TX_SNAPSHOT_SUPPORT
extern u32 sysRFTXSnapImgRdy;
#endif
extern u32 guiSysTimerCnt;
extern u8 EnableStreaming;
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[];
extern u32 guiIISPlayDMAId;
extern u32 guiIISRecDMAId;
extern u32 asfSectionTime;

extern OS_EVENT *gRfiuAVCmpSemEvt[MAX_RFIU_UNIT];
extern u32 guiRFTimerID;


extern void mcpu_ByteMemcpy(u8 *DstAddr, u8 *SrcAddr, unsigned int ByteCnt);
extern unsigned int rfiuCalBufRemainCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize);
extern void AdjustIISFreq(s32 level);

extern u32 P2PVideoBufReadIdx[];
extern u32 P2PAudioBufReadIdx[];
#if (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
extern u8  nMotorTime;
#elif (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
extern u8  nMotorTime;
#endif

#if( (SW_APPLICATION_OPTION  == MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
#if MENU_DONOT_SHARE_BUFFER
extern u8 sysEnMenu;
extern u8 sysEnZoom;
extern u8 sysEnSnapshot;
#endif
#endif
#if Melody_SNC7232_ENA
extern u8 Melody_audio_level;
extern u8 Melody_play;
#endif
#if ICOMMWIFI_SUPPORT
extern u8 iComm_speak;
extern u32 iCommAudioRetPlay_idx;
extern u32  iCommAudioRetWrite_idx;
#endif
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */ 
int Pack_VideoPs_RF(
                        int RFUnit,
                        int StreamType,
                        unsigned int BufWriteAddrOffset,
                        unsigned int VideoPayLoadLen,
                        VIDEO_BUF_MNG *pVideoBufMng,
                        unsigned char *pBuf,
                        int current_next_indicator,
                        int LastOne,
                        unsigned int PsSeqNum,
                        unsigned int VideoDelay
                    );


int Pack_AudioPs_RF(
                            int RFUnit,
                            int StreamType,
                            unsigned int BufWriteAddrOffset,
                            unsigned int AudioPayLoadLen,
                            IIS_BUF_MNG *pAudioBufMng,
                            unsigned char *pBuf,
                            int current_next_indicator,
                            int LastOne,
                            unsigned int PsSeqNum,
                            unsigned int AudioDelay
                           );

#if TX_SNAPSHOT_SUPPORT
int Pack_DataPs_RF(
                            int RFUnit,
                            int StreamType,
                            unsigned int BufWriteAddrOffset,
                            unsigned int DataPayLoadLen,
                            unsigned char *pBuf,
                            int current_next_indicator,
                            int LastOne,
                            unsigned int PsSeqNum
                          );
#endif

int SyncPSHeader_RF(int RFUnit,
                            unsigned int *pBufReadAddrOffset,
                            unsigned int RemainBufSize
                           );

int ParsePSHeader_RF(int RFUnit,
                             unsigned int BufReadAddrOffset,
                             DEF_RF_PS_HEADER *pHeader
                            );

int UnpackPS_RF(
                        int RFUnit,
                        unsigned int BufReadAddrOffset,
                        DEF_RF_PS_HEADER *pHeader
                     );

void rfiuAudioPlayDMA_ISR(int dummy);
void rfiuAudioRet_RecDMA_ISR(int dummy);
void rfiuAudioRet_PlayDMA_ISR(int dummy);

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */ 
 

#define RFTX_AUDIO_TIMESHFT   4

int  rfiu_AudioRetONOFF_IIS(int OnOff)
{
    int i,FillIdx;

    if(OnOff)
    {   //Turn on audio return function.
        if(rfiuAudioRetStatus !=RF_AUDIO_RET_OFF)
        {    
            DEBUG_RFIU_P2("Audio-return is occurpied by APP!!\n");
            return 0;
        }
        rfiuAudioRetStatus=RF_AUDIO_RET_RX_USE;
        for(i=0;i<MAX_RFIU_UNIT;i++)
           gRfiuUnitCntl[i].RXCmd_AudioRetEn=0;
            
		for(i=0;i<RFI_AUDIO_RET_BUF_NUM;i++)
           rfiuAudioRetDMANextBuf[i] = rfiuOperBuf[sysRFRxInMainCHsel] + RFI_AUDIORETURN1_ADDR_OFFSET*128 + i*1024;

        FillIdx=RFI_AUDIORETURN1_ADDR_OFFSET >>6;
        for(i=0;i<8;i++)
        {
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[FillIdx].PktCount=64;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[FillIdx].RetryCount=0;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[FillIdx].WriteDiv=8;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[FillIdx].ReadDiv =0;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[FillIdx].PktMap0 =0xffffffff;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[FillIdx].PktMap1 =0xffffffff;

            FillIdx ++;
        }

        rfiuAudioRetFromApp=0;
        rfiuAudioRetRec_idx=0;
        rfiuAudioRetRead_idx=0;
        gRfiuUnitCntl[sysRFRxInMainCHsel].RXCmd_AudioRetEn=1;
    }
    else
    {   
        if(rfiuAudioRetStatus !=RF_AUDIO_RET_RX_USE)
        {    
            DEBUG_RFIU_P2("Close Audio-return is Fail!!\n");
            return 0;
        }
        for(i=0;i<MAX_RFIU_UNIT;i++)
           gRfiuUnitCntl[i].RXCmd_AudioRetEn=0;
        //Turn off audio return funtion.
        rfiuAudioRetStatus=RF_AUDIO_RET_OFF;
        rfiuAudioRetFromApp=0;

    }

    return 1;
}


int rfiu_AudioRetONOFF_APP(int OnOff,int RFUnit)
{
    int i,FillIdx;
    
    if(OnOff)
    {   //Turn on audio return function.
        if(rfiuAudioRetStatus != RF_AUDIO_RET_OFF)
        {    
            DEBUG_RFIU_P2("Audio-return is occurpied by RX!!\n");
            return 0;
        }
        rfiuAudioRetStatus=RF_AUDIO_RET_APP_USE;
        for(i=0;i<MAX_RFIU_UNIT;i++)
           gRfiuUnitCntl[i].RXCmd_AudioRetEn=0;        

		for(i=0;i<RFI_AUDIO_RET_BUF_NUM;i++)
           rfiuAudioRetDMANextBuf[i] = rfiuOperBuf[RFUnit] + RFI_AUDIORETURN1_ADDR_OFFSET*128 + i*1024;
      
        FillIdx=RFI_AUDIORETURN1_ADDR_OFFSET >>6;
        for(i=0;i<8;i++)
        {
            gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktCount=64;
            gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].RetryCount=0;
            gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].WriteDiv=8;
            gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].ReadDiv =0;
            gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktMap0 =0xffffffff;
            gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktMap1 =0xffffffff;

            FillIdx ++;
        }
        
        rfiuAudioRetFromApp=1;
        rfiuAudioRetRec_idx=0;
        rfiuAudioRetRead_idx=0;
        gRfiuUnitCntl[RFUnit].RXCmd_AudioRetEn=1;
    }
    else
    {   
        if(rfiuAudioRetStatus != RF_AUDIO_RET_APP_USE)
        {    
            DEBUG_RFIU_P2("Close Audio-return is Fail!!\n");
            return 0;
        }
        //Turn off audio return funtion.
        for(i=0;i<MAX_RFIU_UNIT;i++)
           gRfiuUnitCntl[i].RXCmd_AudioRetEn=0;        
        rfiuAudioRetStatus=RF_AUDIO_RET_OFF;
        rfiuAudioRetFromApp=1;      
    }

    return 1;
}

u8 rfiu_RfSwAudio_DualMode(void)
{
    gRfiuUnitCntl[0].RX_MpegDec_Stop=1;
    gRfiuUnitCntl[1].RX_MpegDec_Stop=1;
    OSTimeDly(1);

   sysRFRxInMainCHsel= (sysRFRxInMainCHsel+1) & 0x1;
   rfiuRxIIsSounBufMngReadIdx[sysRFRxInMainCHsel]=rfiuRxIIsSounBufMngWriteIdx[sysRFRxInMainCHsel] % IIS_BUF_NUM;
   rfiuAudioBufPlay_idx[sysRFRxInMainCHsel]=0;
   rfiuAudioBufFill_idx[sysRFRxInMainCHsel]=0;
   rfiuAudioTimeBase=0;
   rfiuMainAudioTime=0;
   rfiuMainAudioTime_frac=0;

   gRfiuUnitCntl[0].RX_MpegDec_Stop=0;
   gRfiuUnitCntl[1].RX_MpegDec_Stop=0;

   return 1;
}

void rfiu_StopRXMpegDec(int RFUnit)
{
    gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=1;
}

void rfiu_ChangeDispSrcWidthHeight(int Width,int Height)
{
    BRI_IN_SIZE = (Height<<16) | Width; 
}

void rfiu_ChangeDispSrcStride(int Stride)
{

    BRI_STRIDE = Stride;
}

unsigned int rfiu_GetTxFrameRate(int RFUnit)
{

    return gRfiuUnitCntl[RFUnit].FrameRate;
}

unsigned int rfiu_GetTxBitRate(int RFUnit)
{

    return gRfiuUnitCntl[RFUnit].BitRate;
}

unsigned int rfiu_GetTxFlickerSet(int RFUnit)
{

    return gRfiuUnitCntl[RFUnit].RFpara.TxSensorAE_FlickSetting;
}

unsigned int rfiu_GetTxBrightness(int RFUnit)
{

    return gRfiuUnitCntl[RFUnit].RFpara.TX_SensorBrightness;
}

char* rfiu_GetTxCodeVersion(int RFUnit)
{
    return gRfiuUnitCntl[RFUnit].RFpara.TxCodeVersion;
}

int rfiu_ForceTXSetSameResolution(int RFUnit)
{
    int RfBusy;
    int cnt;
    u8 uartCmd[20];
    //-----------------//

 #if ( UI_GRAPH_QVGA_ENABLE && (SW_APPLICATION_OPTION !=MR8100_DUALMODE_VBM) )
    if(gRfiuUnitCntl[RFUnit].TX_PicWidth==640)
        sprintf((char*)uartCmd,"RESO %d %d",320,240);
    else
        sprintf((char*)uartCmd,"RESO %d %d",640,480);
 #else
    if(gRfiuUnitCntl[RFUnit].TX_PicWidth==640)
        sprintf((char*)uartCmd,"RESO %d %d",1280,720);
    else
        sprintf((char*)uartCmd,"RESO %d %d",640,480);
 #endif
    cnt=0;
    RfBusy=1;

    while(RfBusy != 0)
    {
        RfBusy = rfiu_RXCMD_Enc(uartCmd, RFUnit);
        if (cnt >3)
        {
            DEBUG_UI("rfiu_ForceTXSetSameResolution Timeout:%d !!!\n",RFUnit);
            return 0;
        }
        cnt++;
        OSTimeDly(1);
    }

    return 1;
}


#if( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) )
void rfiu_iisTX_TalkPlay(u32 RFUnit)
{
    int i;   
#if RFIU_RX_AUDIO_RETURN    
    rfiuAudioRetPlay_idx=0;
    rfiuAudioRetWrite_idx=0;
    for(i=0;i<RFI_AUDIO_RET_BUF_NUM;i++)
        rfiuAudioRetDMANextBuf[i] = rfiuOperBuf[RFUnit] + RFI_AUDIORETURN1_ADDR_OFFSET*128 + i*1024;
    memset(rfiuAudioRetDMANextBuf[0],0x80,8192);

    if(guiIISPlayDMAId==0xff)
	    marsDMAOpen(&guiIISPlayDMAId, rfiuAudioRet_PlayDMA_ISR);
	else
	    DEBUG_RFIU_P2("Error! Audio Play DMA is occurpied!\n");
    #if(Audio_mode == AUDIO_AUTO)
    iisSetNextPlayDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #else
    iisSetNextPlayDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #endif
	iisStartPlay();
#endif
}

void rfiu_WrapTx_Task_UnitX(void* pData)
{
    int RFUnit;
    int GoFirst,GoSyncIframe,OnlySendIframe;
    unsigned int VideoReadIdx,AudioReadIdx;
    int audio_value,video_value;
    unsigned int BufWriteAddrOffset,BufReadAddrOffset;
    unsigned int Size2BufEnd,RemainBufSize;
    unsigned int AudioPsLen,VideoPsLen;
    unsigned int AudioPayloadLen,VideoPayLoadLen;
#if TX_SNAPSHOT_SUPPORT
    unsigned int DataPsLen;
    unsigned int DataPayloadLen;
#endif
    unsigned int FillIdx,WriteIdx,WriteDiv;
    int WriteCnt,i,OffsetGrp;
    unsigned int PsSeqNum;
    unsigned int cpu_sr = 0;   
    unsigned int VideoDelay;
    unsigned int AudioDelay;
	int SyncTime;
    unsigned char err;

    //---計算TX斷線時間--//
    static int PowOnFirst=1;
    static unsigned int Broken_T1=0;
    static unsigned int Broken_T2=0;

    static unsigned int Broken_T3=0;
    static unsigned int Broken_T4=0;
   
    int LinkBrokenTimeShift;  //ms unit
    int temp;
    //-------------------//

    
    RFUnit= (int)pData;
    
    VideoReadIdx = (VideoBufMngWriteIdx) % VIDEO_BUF_NUM;
    AudioReadIdx = (iisSounBufMngWriteIdx) % IIS_BUF_NUM;
    OSSemSet(gRfiuAVCmpSemEvt[0], 0, &err);
    
    GoFirst=1;
    GoSyncIframe=0;
    
	OnlySendIframe=0;
    BufWriteAddrOffset=0;
    BufReadAddrOffset=0;
    FillIdx=0;
    PsSeqNum=0;
    VideoDelay=0;
    AudioDelay=0;
	SyncTime=0;

#if RFIU_RX_AUDIO_RETURN    
    rfiuAudioRetPlay_idx=0;
    rfiuAudioRetWrite_idx=0;
    for(i=0;i<RFI_AUDIO_RET_BUF_NUM;i++)
        rfiuAudioRetDMANextBuf[i] = rfiuOperBuf[RFUnit] + RFI_AUDIORETURN1_ADDR_OFFSET*128 + i*1024;
    memset(rfiuAudioRetDMANextBuf[0],0x80,8192);

    if(guiIISPlayDMAId==0xff)
	    marsDMAOpen(&guiIISPlayDMAId, rfiuAudioRet_PlayDMA_ISR);
	else
	    DEBUG_RFIU_P2("Error! Audio Play DMA is occurpied!\n");
    #if(Audio_mode == AUDIO_AUTO)
    iisSetNextPlayDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #else
    iisSetNextPlayDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #endif
	iisStartPlay();
#endif

    timerCountRead(guiRFTimerID, &Broken_T1);
    Broken_T3=guiSysTimerCnt;
    if(PowOnFirst)
    {
        LinkBrokenTimeShift=0;
        PowOnFirst=0;
    }
    else
    {
        if(Broken_T2 >= Broken_T1)
          LinkBrokenTimeShift=Broken_T2-Broken_T1; // 100 us unit
        else
          LinkBrokenTimeShift=(Broken_T2+TimerGetTimerCounter(TIMER_7))-Broken_T1;

        if(Broken_T3 >= Broken_T4)
           temp=Broken_T3-Broken_T4;
        else
           temp=Broken_T3 + (0xffffffff-Broken_T4);

        temp=temp*25; // 25ms unit 
        
        if(temp > 100000)
           LinkBrokenTimeShift=temp;
        else
           LinkBrokenTimeShift=LinkBrokenTimeShift/10; //change to ms.
    }
    DEBUG_RFIU_P2("\n======Link Broken time shift=%d ms ======\n",LinkBrokenTimeShift);

 #if RF_AV_SYNCTIME_EN   
    gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime=0;
    gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime=0;
 #endif

    while(1)
    {
        timerCountRead(guiRFTimerID, &Broken_T2);
        Broken_T4=guiSysTimerCnt;
        if(gRfiuUnitCntl[RFUnit].TX_Wrap_Stop)
        {
           DEBUG_RFIU_P2("@");
           OSTimeDly(1);
           continue;
        }
 
		video_value = rfiuCalBufRemainCount(VideoBufMngWriteIdx,VideoReadIdx,VIDEO_BUF_NUM);
        audio_value = rfiuCalBufRemainCount(iisSounBufMngWriteIdx,AudioReadIdx,IIS_BUF_NUM);

		
        OS_ENTER_CRITICAL();
        BufReadAddrOffset = (gRfiuUnitCntl[RFUnit].BufReadPtr & RFI_BUF_SIZE_MASK) << 13;
        OS_EXIT_CRITICAL();
        
        WriteIdx = BufWriteAddrOffset >> 13; 
        WriteDiv = (BufWriteAddrOffset >> 10) & 0x07;
        WriteCnt = rfiuCalBufRemainCount(WriteIdx,FillIdx,RFI_BUF_SIZE_GRPUNIT);
        
        OS_ENTER_CRITICAL();  
        
    #if RFI_LOW_DELAY_ON        
        if(WriteCnt==0)
        {
           gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktCount=64;
           gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].RetryCount=0;
           gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].WriteDiv=WriteDiv;
           gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktMap0 =0xffffffff;
           gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktMap1 =0xffffffff;
           FillIdx=(FillIdx + 1) % RFI_BUF_SIZE_GRPUNIT;
        } 
        else if(WriteCnt > 0)
        {
           for(i=0;i<WriteCnt;i++)
           {
              gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktCount=64;
              gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].RetryCount=0;
              gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].WriteDiv=8;
              gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktMap0 =0xffffffff;
              gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktMap1 =0xffffffff;
              FillIdx=(FillIdx + 1) % RFI_BUF_SIZE_GRPUNIT;
           }
           gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktCount=64;
           gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].RetryCount=0;
           gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].WriteDiv=WriteDiv;
           gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktMap0 =0xffffffff;
           gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktMap1 =0xffffffff;
           FillIdx=(FillIdx + 1) % RFI_BUF_SIZE_GRPUNIT;
        }
     #else
        for(i=0;i<WriteCnt;i++)
        {
          gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktCount=64;
          gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].RetryCount=0;
          gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].WriteDiv=8;
          gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktMap0 =0xffffffff;
          gRfiuUnitCntl[RFUnit].TxPktMap[FillIdx].PktMap1 =0xffffffff;
          FillIdx=(FillIdx + 1) % RFI_BUF_SIZE_GRPUNIT;
        }
     #endif   
        gRfiuUnitCntl[RFUnit].BufWritePtr= WriteIdx;
        OffsetGrp = rfiuCalBufRemainCount(gRfiuUnitCntl[RFUnit].BufWritePtr,gRfiuUnitCntl[RFUnit].BufReadPtr,RFI_BUF_SIZE_GRPUNIT);
        OS_EXIT_CRITICAL();

        if( (video_value <= 0) && (audio_value <= RFTX_AUDIO_TIMESHFT) )
        {
            OSSemSet(gRfiuAVCmpSemEvt[0], 0, &err);
			OSSemPend(gRfiuAVCmpSemEvt[0], 6, &err);
            //OSTimeDly(1);
        }
        else if(GoFirst==1)
        {        
            if(video_value>0)
            {
                while(video_value>0)
                {
                    //DEBUG_RFIU_P2("V:%d,%d A:%d,%d\n",VideoBufMngWriteIdx,VideoReadIdx,iisSounBufMngWriteIdx,AudioReadIdx);
                    if(VideoBufMng[VideoReadIdx].flag == FLAG_I_VOP)
                    {
                       AudioReadIdx = (iisSounBufMngWriteIdx-RFTX_AUDIO_TIMESHFT) % IIS_BUF_NUM;
                       AudioDelay=VideoDelay;
                       GoFirst=0;
                       VideoDelay +=LinkBrokenTimeShift;
                       AudioDelay +=LinkBrokenTimeShift;
                       LinkBrokenTimeShift=0;
                       OSSemSet(gRfiuAVCmpSemEvt[0], 0, &err);
                       VideoBufMng[VideoReadIdx].time=128;
					   DEBUG_RFIU_P2("Audio Sync: %d,%d\n",VideoBufMngWriteIdx,VideoReadIdx);
                       break;
                    }
                    else
                    {
                       VideoDelay += (unsigned int)(VideoBufMng[VideoReadIdx].time & 0xffffffff);
                    }
					
                    VideoReadIdx = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
					video_value = rfiuCalBufRemainCount(VideoBufMngWriteIdx,VideoReadIdx,VIDEO_BUF_NUM);
                }
            }
            else
                OSTimeDly(1);
        }

        //------------------ Pause Mode ------------------//
        else if( (OffsetGrp >= RFI_BUF_SIZE_GRPUNIT*3/4) || (GoSyncIframe == 1) )
        {
            GoSyncIframe=1;
        #if 1
            while( audio_value > RFTX_AUDIO_TIMESHFT )
            {
                 AudioDelay += (unsigned int)(iisSounBufMng[AudioReadIdx].time & 0xffffffff);
                 AudioReadIdx = (AudioReadIdx + 1) % IIS_BUF_NUM;
                 OSSemAccept(gRfiuAVCmpSemEvt[0]);                 
                 audio_value = rfiuCalBufRemainCount(iisSounBufMngWriteIdx,AudioReadIdx,IIS_BUF_NUM);
            }
        #endif     
            
            if(video_value>0)
            {
                while(video_value>0)
                {
                    if(VideoBufMng[VideoReadIdx].flag == FLAG_I_VOP)
                    {
                       DEBUG_RFIU_P2("===== Enter Pause Mode:%d=====\n",video_value);
                       AudioReadIdx = (iisSounBufMngWriteIdx-RFTX_AUDIO_TIMESHFT) % IIS_BUF_NUM;
                       if(OffsetGrp < RFI_BUF_SIZE_GRPUNIT/8)
                       {
                          GoSyncIframe=0;
                          OnlySendIframe=1;
                          //AudioDelay=VideoDelay;
                       }
                       else
                       {
                          VideoDelay += (unsigned int)(VideoBufMng[VideoReadIdx].time & 0xffffffff);
					      VideoReadIdx = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                       }   
					   SyncTime=0;
                       OSSemSet(gRfiuAVCmpSemEvt[0], 0, &err);
                       OSTimeDly(1);
                    #if RF_LETENCY_DEBUG_ENA
                       DEBUG_RFIU_P2("V>%d\n",OffsetGrp);
                    #endif                           
                       break;
                    }
                    else
                    {
                       VideoDelay += (unsigned int)(VideoBufMng[VideoReadIdx].time & 0xffffffff);
                    }
					
                    VideoReadIdx = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                    video_value = rfiuCalBufRemainCount(VideoBufMngWriteIdx,VideoReadIdx,VIDEO_BUF_NUM);
                }
				
            }
            else
                OSTimeDly(1);
        }
		//---------------------Low speed mode------------------------//
        else if( (OffsetGrp > RFI_BUF_SIZE_GRPUNIT*2/4) || (OnlySendIframe==1) )
        {
            //Lucian: 若 RF 速度過慢,只傳送 I Frame. Audio frame 停止傳送.   
            // ------Streaming audio payload------//
            OnlySendIframe=1;
           #if 1//(SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) //Don't transmit Audio Except baby monitor's application   
            if( audio_value > RFTX_AUDIO_TIMESHFT )
            {
                RemainBufSize=RFI_TOTAL_BUF_SIZE - ((BufWriteAddrOffset+RFI_TOTAL_BUF_SIZE-BufReadAddrOffset) % RFI_TOTAL_BUF_SIZE);
                AudioPayloadLen=(iisSounBufMng[AudioReadIdx].size+3) & (~0x03);
                if(RemainBufSize>AudioPayloadLen+PS_AUDIO_HEADERLEN)
                {
                    Size2BufEnd = RFI_TOTAL_BUF_SIZE - BufWriteAddrOffset;
                    AudioPsLen=AudioPayloadLen + PS_AUDIO_HEADERLEN;
                    if(Size2BufEnd >= AudioPsLen)
                    {
                       if(Size2BufEnd <= AudioPsLen + PS_AUDIO_HEADERLEN)
                       {
                          AudioPayloadLen += (AudioPsLen-Size2BufEnd);
                          Pack_AudioPs_RF(RFUnit,PS_AUDIOTYPE_8K8B_PCM,BufWriteAddrOffset,
                                          AudioPayloadLen,&iisSounBufMng[AudioReadIdx],
                                          iisSounBufMng[AudioReadIdx].buffer,0,1,0,AudioDelay);
                          PsSeqNum ++;
                          BufWriteAddrOffset = 0;
                       }
                       else
                       {
                          Pack_AudioPs_RF(RFUnit,PS_AUDIOTYPE_8K8B_PCM,BufWriteAddrOffset,
                                          AudioPayloadLen,&iisSounBufMng[AudioReadIdx],
                                          iisSounBufMng[AudioReadIdx].buffer,0,0,0,AudioDelay);
                          PsSeqNum ++;
                          BufWriteAddrOffset += AudioPsLen;
                       }
                    }
                    else
                    {
                       
        		       Pack_AudioPs_RF(RFUnit,PS_AUDIOTYPE_8K8B_PCM,BufWriteAddrOffset,
                                       Size2BufEnd-PS_AUDIO_HEADERLEN,&iisSounBufMng[AudioReadIdx],
                                       iisSounBufMng[AudioReadIdx].buffer,1,1,0,AudioDelay);
                       PsSeqNum ++;
                       
        		       BufWriteAddrOffset = 0; 
                       Pack_AudioPs_RF(RFUnit,PS_AUDIOTYPE_8K8B_PCM,BufWriteAddrOffset,
                                      AudioPsLen-Size2BufEnd,&iisSounBufMng[AudioReadIdx],
                                      iisSounBufMng[AudioReadIdx].buffer+Size2BufEnd-PS_AUDIO_HEADERLEN,0,0,0,AudioDelay);
                       PsSeqNum ++;
                       BufWriteAddrOffset = AudioPsLen-Size2BufEnd+PS_AUDIO_HEADERLEN; 
                    }
                    AudioDelay=0;
                    AudioReadIdx = (AudioReadIdx + 1) % IIS_BUF_NUM;
                    OSSemAccept(gRfiuAVCmpSemEvt[0]);
                 }
                 else
                 {
                     OSTimeDly(1);
                 }
            }
          #else //Lucian: 不傳audio
            if( audio_value > RFTX_AUDIO_TIMESHFT )
            {
                 AudioDelay += (unsigned int)(iisSounBufMng[AudioReadIdx].time & 0xffffffff);

                 AudioReadIdx = (AudioReadIdx + 1) % IIS_BUF_NUM;
                 OSSemAccept(gRfiuAVCmpSemEvt[0]);
            }
          #endif
            //------ Streaming video payload------// 
            if( video_value > 0)
            {  
                RemainBufSize=RFI_TOTAL_BUF_SIZE - ((BufWriteAddrOffset+RFI_TOTAL_BUF_SIZE-BufReadAddrOffset) % RFI_TOTAL_BUF_SIZE);
                if(RemainBufSize>VideoBufMng[VideoReadIdx].size + PS_VIDEO_HEADERLEN)
                {
                    if(VideoBufMng[VideoReadIdx].flag == FLAG_I_VOP)  //此時僅傳送 I Frame以縮小資料量,直到 RF buffer 累積縮小.
                    {
                        DEBUG_RFIU_P2("===Enter Low Speed Mode:%d===\n",video_value);
                        if(OffsetGrp < RFI_BUF_SIZE_GRPUNIT/16)
                            OnlySendIframe=0;
                        Size2BufEnd = RFI_TOTAL_BUF_SIZE - BufWriteAddrOffset;
                        VideoPayLoadLen=(VideoBufMng[VideoReadIdx].size + 3) & (~0x03);
                        VideoPsLen=VideoPayLoadLen + PS_VIDEO_HEADERLEN;
                        if(Size2BufEnd >= VideoPsLen)
                        {
                           if(Size2BufEnd <= VideoPsLen + PS_VIDEO_HEADERLEN)
                           {
                              VideoPayLoadLen += (Size2BufEnd-VideoPsLen);
                              Pack_VideoPs_RF(RFUnit,PS_VIDEOTYPE_MPEG4,BufWriteAddrOffset,
                                              VideoPayLoadLen,&VideoBufMng[VideoReadIdx],
                                              VideoBufMng[VideoReadIdx].buffer,
                                              0,1,0,VideoDelay);
                              PsSeqNum ++;
                              BufWriteAddrOffset = 0;
                           }
                           else
                           {
                              Pack_VideoPs_RF(RFUnit,PS_VIDEOTYPE_MPEG4,BufWriteAddrOffset,
                                              VideoPayLoadLen,&VideoBufMng[VideoReadIdx],
                                              VideoBufMng[VideoReadIdx].buffer,
                                              0,0,0,VideoDelay);
                              PsSeqNum ++;
                              BufWriteAddrOffset += VideoPsLen;
                           }
                        }
                        else
                        {
            		       Pack_VideoPs_RF(RFUnit,PS_VIDEOTYPE_MPEG4,BufWriteAddrOffset,
                                           Size2BufEnd-PS_VIDEO_HEADERLEN,&VideoBufMng[VideoReadIdx],
                                           VideoBufMng[VideoReadIdx].buffer,
                                           1,1,0,VideoDelay);
                           PsSeqNum ++;
            		       BufWriteAddrOffset = 0; 
                           Pack_VideoPs_RF(RFUnit,PS_VIDEOTYPE_MPEG4,BufWriteAddrOffset,
                                           VideoPsLen-Size2BufEnd,&VideoBufMng[VideoReadIdx],
                                           VideoBufMng[VideoReadIdx].buffer+Size2BufEnd-PS_VIDEO_HEADERLEN,
                                           0,0,0,VideoDelay);
                           PsSeqNum ++;
                           BufWriteAddrOffset = VideoPsLen-Size2BufEnd+PS_VIDEO_HEADERLEN; 
                        }
                        VideoDelay=0;
                    #if RF_LETENCY_DEBUG_ENA
                       DEBUG_RFIU_P2("V>%d\n",OffsetGrp);
                    #endif
    
                    }
                    else
                    {
                         VideoDelay += (unsigned int)(VideoBufMng[VideoReadIdx].time & 0xffffffff);
                    }
                    VideoReadIdx = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                    OSSemAccept(gRfiuAVCmpSemEvt[0]);
                 }
                 else
                 {
                     OSTimeDly(1);
                 }
            }
        }
        //---------------------Normal mode: Audio/Video 正常傳送---------------------------//
        else if( (OffsetGrp < RFI_BUF_SIZE_GRPUNIT-1) && (GoFirst==0) )
        { 
        #if TX_SNAPSHOT_SUPPORT
            if(sysRFTXSnapImgRdy)
            {
                RemainBufSize=RFI_TOTAL_BUF_SIZE - ((BufWriteAddrOffset+RFI_TOTAL_BUF_SIZE-BufReadAddrOffset) % RFI_TOTAL_BUF_SIZE);
                DataPayloadLen=(sysRFTXDataSize+3) & (~0x03);
                if(RemainBufSize>DataPayloadLen+PS_DATA_HEADERLEN)
                {
                    DEBUG_RFIU_P2("==Packing Date Payload:%d==\n",RFUnit);
                    Size2BufEnd = RFI_TOTAL_BUF_SIZE - BufWriteAddrOffset;
                    DataPsLen=DataPayloadLen + PS_DATA_HEADERLEN;
                    if(Size2BufEnd >= DataPsLen)
                    {
                       if(Size2BufEnd <= DataPsLen + PS_DATA_HEADERLEN)
                       {
                          DataPayloadLen += (Size2BufEnd-DataPsLen);
                          Pack_DataPs_RF(RFUnit,PS_IMAGETYPE_JPEG,BufWriteAddrOffset,
                                         DataPayloadLen,
                                         sysRFTXImgData,
                                         0,1,0);
                          PsSeqNum ++;
                          BufWriteAddrOffset = 0;
                       }
                       else
                       {
                          Pack_DataPs_RF(RFUnit,PS_IMAGETYPE_JPEG,BufWriteAddrOffset,
                                          DataPayloadLen,
                                          sysRFTXImgData,
                                          0,0,0);
                          PsSeqNum ++;
                          BufWriteAddrOffset += DataPsLen;
                       }
                    }
                    else
                    {
                       if(Size2BufEnd<=PS_DATA_HEADERLEN)
                          DEBUG_RFIU_P2("---->Waring!! Size2BufEnd is invalid:%d\n",Size2BufEnd);
        		       Pack_DataPs_RF(RFUnit,PS_IMAGETYPE_JPEG,BufWriteAddrOffset,
                                      Size2BufEnd-PS_DATA_HEADERLEN,
                                      sysRFTXImgData,
                                      1,1,0);
                       PsSeqNum ++;
        		       BufWriteAddrOffset = 0; 
                       Pack_DataPs_RF(RFUnit,PS_IMAGETYPE_JPEG,BufWriteAddrOffset,
                                      DataPsLen-Size2BufEnd,
                                      sysRFTXImgData+Size2BufEnd-PS_DATA_HEADERLEN,
                                      0,0,0);
                       PsSeqNum ++;
                       BufWriteAddrOffset = DataPsLen-Size2BufEnd+PS_DATA_HEADERLEN; 
                    }
                }
                else
                {
                    OSTimeDly(1);
                }

                sysRFTXSnapImgRdy=0;
            }
        #endif        
            // ------Streaming audio payload------//
            //DEBUG_RFIU_P2("(%d,%d)",iisSounBufMngWriteIdx,iisSounBufMngReadIdx);
            if( (audio_value > RFTX_AUDIO_TIMESHFT) )
            {
                RemainBufSize=RFI_TOTAL_BUF_SIZE - ((BufWriteAddrOffset+RFI_TOTAL_BUF_SIZE-BufReadAddrOffset) % RFI_TOTAL_BUF_SIZE);
                AudioPayloadLen=(iisSounBufMng[AudioReadIdx].size+3) & (~0x03);
                if(RemainBufSize>AudioPayloadLen+PS_AUDIO_HEADERLEN)
                {
                    Size2BufEnd = RFI_TOTAL_BUF_SIZE - BufWriteAddrOffset;
                    AudioPsLen=AudioPayloadLen + PS_AUDIO_HEADERLEN;
                    if(Size2BufEnd >= AudioPsLen)
                    {
                       if(Size2BufEnd <= AudioPsLen + PS_AUDIO_HEADERLEN)
                       {
                          AudioPayloadLen += (Size2BufEnd-AudioPsLen);
                          Pack_AudioPs_RF(RFUnit,PS_AUDIOTYPE_8K8B_PCM,BufWriteAddrOffset,
                                          AudioPayloadLen,&iisSounBufMng[AudioReadIdx],
                                          iisSounBufMng[AudioReadIdx].buffer,0,1,0,AudioDelay);
                          PsSeqNum ++;
                          BufWriteAddrOffset = 0;
                       }
                       else
                       {
                          Pack_AudioPs_RF(RFUnit,PS_AUDIOTYPE_8K8B_PCM,BufWriteAddrOffset,
                                          AudioPayloadLen,&iisSounBufMng[AudioReadIdx],
                                          iisSounBufMng[AudioReadIdx].buffer,0,0,0,AudioDelay);
                          PsSeqNum ++;
                          BufWriteAddrOffset += AudioPsLen;
                       }
                    }
                    else
                    {
                       if(Size2BufEnd<=PS_AUDIO_HEADERLEN)
                          DEBUG_RFIU_P2("---->Waring!! Size2BufEnd is invalid:%d\n",Size2BufEnd);
        		       Pack_AudioPs_RF(RFUnit,PS_AUDIOTYPE_8K8B_PCM,BufWriteAddrOffset,
                                       Size2BufEnd-PS_AUDIO_HEADERLEN,&iisSounBufMng[AudioReadIdx],
                                       iisSounBufMng[AudioReadIdx].buffer,1,1,0,AudioDelay);
                       PsSeqNum ++;
        		       BufWriteAddrOffset = 0; 
                       Pack_AudioPs_RF(RFUnit,PS_AUDIOTYPE_8K8B_PCM,BufWriteAddrOffset,
                                       AudioPsLen-Size2BufEnd,&iisSounBufMng[AudioReadIdx],
                                       iisSounBufMng[AudioReadIdx].buffer+Size2BufEnd-PS_AUDIO_HEADERLEN,0,0,0,AudioDelay);
                       PsSeqNum ++;
                       BufWriteAddrOffset = AudioPsLen-Size2BufEnd+PS_AUDIO_HEADERLEN; 
                    }

                    //DEBUG_RFIU_P2("AudioDelay=%d\n",AudioDelay);
                    AudioDelay=0;
                    AudioReadIdx = (AudioReadIdx + 1) % IIS_BUF_NUM;
                    OSSemAccept(gRfiuAVCmpSemEvt[0]);
					//DEBUG_RFIU_P2("A");
                }
                else
                {
                    OSTimeDly(1);
                }
            }
            //------ Streaming video payload------// 
            if( video_value > 0 )
            {
                RemainBufSize=RFI_TOTAL_BUF_SIZE - ((BufWriteAddrOffset+RFI_TOTAL_BUF_SIZE-BufReadAddrOffset) % RFI_TOTAL_BUF_SIZE);
                VideoPayLoadLen=(VideoBufMng[VideoReadIdx].size+3) & (~0x3);
                if(RemainBufSize>VideoPayLoadLen + PS_VIDEO_HEADERLEN)
                {
                    Size2BufEnd = RFI_TOTAL_BUF_SIZE - BufWriteAddrOffset;
                    VideoPsLen=VideoPayLoadLen + PS_VIDEO_HEADERLEN;
                    if(Size2BufEnd >= VideoPsLen)
                    {
                       if(Size2BufEnd <= VideoPsLen + PS_VIDEO_HEADERLEN)
                       {
                          VideoPayLoadLen += (Size2BufEnd-VideoPsLen);
                          Pack_VideoPs_RF(RFUnit,PS_VIDEOTYPE_MPEG4,BufWriteAddrOffset,
                                          VideoPayLoadLen,&VideoBufMng[VideoReadIdx],
                                          VideoBufMng[VideoReadIdx].buffer,
                                          0,1,0,VideoDelay);
                          PsSeqNum ++;
                          BufWriteAddrOffset = 0;
                       }
                       else
                       {
                          Pack_VideoPs_RF(RFUnit,PS_VIDEOTYPE_MPEG4,BufWriteAddrOffset,
                                          VideoPayLoadLen,&VideoBufMng[VideoReadIdx],
                                          VideoBufMng[VideoReadIdx].buffer,
                                          0,0,0,VideoDelay);
                          PsSeqNum ++;
                          BufWriteAddrOffset += VideoPsLen;
                       }
                    }
                    else
                    {
                       if(Size2BufEnd<=PS_VIDEOTYPE_MPEG4)
                          DEBUG_RFIU_P2("---->Waring!! Size2BufEnd is invalid:%d\n",Size2BufEnd);
        		       Pack_VideoPs_RF(RFUnit,PS_VIDEOTYPE_MPEG4,BufWriteAddrOffset,
                                       Size2BufEnd-PS_VIDEO_HEADERLEN,&VideoBufMng[VideoReadIdx],
                                       VideoBufMng[VideoReadIdx].buffer,
                                       1,1,0,VideoDelay);
                       PsSeqNum ++;
        		       BufWriteAddrOffset = 0; 
                       Pack_VideoPs_RF(RFUnit,PS_VIDEOTYPE_MPEG4,BufWriteAddrOffset,
                                       VideoPsLen-Size2BufEnd,&VideoBufMng[VideoReadIdx],
                                       VideoBufMng[VideoReadIdx].buffer+Size2BufEnd-PS_VIDEO_HEADERLEN,
                                       0,0,0,VideoDelay);
                       PsSeqNum ++;
                       BufWriteAddrOffset = VideoPsLen-Size2BufEnd+PS_VIDEO_HEADERLEN; 
                    }
                    VideoDelay=0;
                    //DEBUG_RFIU_P2("%d ",VideoBufMng[VideoReadIdx].flag);

                    VideoReadIdx = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                    OSSemAccept(gRfiuAVCmpSemEvt[0]);
                 #if RF_LETENCY_DEBUG_ENA
                    DEBUG_RFIU_P2("V>%d\n",OffsetGrp);
                 #endif
                 }
                 else
                 {
                    OSTimeDly(1);
                 }
            }
            
        }
        else
        {            
            OSTimeDly(1);
        }
    }

}

void rfiu_WrapRx_Task_UnitX(void* pData)
{

}

int Pack_VideoPs_RF(
                            int RFUnit,
                            int StreamType,
                            unsigned int BufWriteAddrOffset,
                            unsigned int VideoPayLoadLen,
                            VIDEO_BUF_MNG *pVideoBufMng,
                            unsigned char *pBuf,
                            int current_next_indicator,
                            int LastOne,
                            unsigned int PsSeqNum,
                            unsigned int VideoDelay
                   )
{
    unsigned int *pp;
    unsigned int status;
    int PTS_flag;

    //-----//
    pp = (unsigned int *)(rfiuOperBuf[RFUnit]+ BufWriteAddrOffset);

    //--start code prefix + stream type--//
    *pp = (PS_START_CODE_PREFIX<<8) | StreamType;
    pp ++;

    //--payload length + status---//
    status=0;
    PTS_flag=1;
    if(pVideoBufMng->flag)
        status |= PS_STA_IFRAME;
    if(current_next_indicator)
    {
        status |= PS_STA_CURRENT_IND;
    }
    if(LastOne)
        status |= PS_STA_LASTONE;
    if(PTS_flag)
        status |= PS_STA_PTS;

    *pp = ((PsSeqNum & 0x0f)<<28) | ( (VideoPayLoadLen & 0x0fffff)<<8) | (status & 0x0ff);
    pp ++;

    if(VideoPayLoadLen & 0x03)
        DEBUG_RFIU_P2("Video Payload not word alignment!\n");
    //---PTS---//
    if(PTS_flag)
    {
       *pp = (unsigned int)pVideoBufMng->time + VideoDelay;
    #if RF_AV_SYNCTIME_EN   
       if(current_next_indicator==0)
          gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime += (*pp);
    #endif       
       pp ++;
       *pp = pVideoBufMng->offset;
       //printf("%d ",*pp);
       pp ++;
    }
    //---Payload--//
    mcpu_ByteMemcpy( (u8 *)pp, pBuf, VideoPayLoadLen);
    
    
}

int Pack_AudioPs_RF(
                            int RFUnit,
                            int StreamType,
                            unsigned int BufWriteAddrOffset,
                            unsigned int AudioPayLoadLen,
                            IIS_BUF_MNG *pAudioBufMng,
                            unsigned char *pBuf,
                            int current_next_indicator,
                            int LastOne,
                            unsigned int PsSeqNum,
                            unsigned int AudioDelay
                           )
{
    unsigned int *pp;
    unsigned int status;
    int PTS_flag;

    //-----//
    pp = (unsigned int *)(rfiuOperBuf[RFUnit]+ BufWriteAddrOffset);

    //--start code prefix + stream type--//
    *pp = (PS_START_CODE_PREFIX<<8) | StreamType;
    pp ++;

    //--payload length + status---//
    status=0;
    PTS_flag=1;
    if(pAudioBufMng->flag)
        status |= PS_STA_IFRAME;
    if(current_next_indicator)
    {
        status |= PS_STA_CURRENT_IND;
    }
    if(LastOne)
        status |= PS_STA_LASTONE;
    if(PTS_flag)
        status |= PS_STA_PTS;

    *pp = ((PsSeqNum & 0x0f)<<28) | ( (AudioPayLoadLen & 0x0fffff)<<8) | (status & 0x0ff);
    pp ++;

    if(AudioPayLoadLen & 0x03)
        DEBUG_RFIU_P2("Audio Payload not word alignment!\n");

    //---PTS---//
    if(PTS_flag)
    {
       *pp = (unsigned int)pAudioBufMng->time + AudioDelay;
    #if RF_AV_SYNCTIME_EN   
       if(current_next_indicator==0)
         gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime += (*pp);
    #endif

        if(*pp > 8000)
           DEBUG_RFIU_P2("Warning!! Audio Timestamp is too Long!:%d,%d\n",(unsigned int)pAudioBufMng->time,AudioDelay);

       pp ++;
       *pp = 0;
       pp ++;
    }
    //---Payload--//
    mcpu_ByteMemcpy( (u8 *)pp, pBuf, AudioPayLoadLen);
}

#if TX_SNAPSHOT_SUPPORT
int Pack_DataPs_RF(
                            int RFUnit,
                            int StreamType,
                            unsigned int BufWriteAddrOffset,
                            unsigned int DataPayLoadLen,
                            unsigned char *pBuf,
                            int current_next_indicator,
                            int LastOne,
                            unsigned int PsSeqNum
                           )
{
    unsigned int *pp;
    unsigned int status;
    int PTS_flag;

    //-----//
    pp = (unsigned int *)(rfiuOperBuf[RFUnit]+ BufWriteAddrOffset);

    //--start code prefix + stream type--//
    *pp = (PS_START_CODE_PREFIX<<8) | StreamType;
    pp ++;

    //--payload length + status---//
    status=0;
    PTS_flag=0;
    if(current_next_indicator)
    {
        status |= PS_STA_CURRENT_IND;
    }
    if(LastOne)
        status |= PS_STA_LASTONE;
    if(PTS_flag)
        status |= PS_STA_PTS;

    *pp = ((PsSeqNum & 0x0f)<<28) | ( (DataPayLoadLen & 0x0fffff)<<8) | (status & 0x0ff);
    pp ++;

    if(DataPayLoadLen & 0x03)
        DEBUG_RFIU_P2("Data Payload not word alignment!\n");

    //---Payload--//
    mcpu_ByteMemcpy( (u8 *)pp, pBuf, DataPayLoadLen);
}
#endif


void rfiu_RxMpeg4DecTask_UnitX(void* pData)
{
}

void rfiuAudioRet_PlayDMA_ISR(int dummy)
{
    int AudioPlayOffset;
    static unsigned int SilenceCnt=0;
    #if (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
    static u8 melodyflag=0;
    static u8 mCount=5;
    #endif
    static u8 VoxCnt=0;
    //=====//
#if ICOMMWIFI_SUPPORT
    if(iComm_speak)
        AudioPlayOffset= (iCommAudioRetWrite_idx >= iCommAudioRetPlay_idx) ?  (iCommAudioRetWrite_idx - iCommAudioRetPlay_idx) : (iCommAudioRetWrite_idx +RFI_AUDIO_RET_BUF_NUM - iCommAudioRetPlay_idx);
    else
        AudioPlayOffset= (rfiuAudioRetWrite_idx >= rfiuAudioRetPlay_idx) ?  (rfiuAudioRetWrite_idx - rfiuAudioRetPlay_idx) : (rfiuAudioRetWrite_idx +RFI_AUDIO_RET_BUF_NUM - rfiuAudioRetPlay_idx);
#else
    AudioPlayOffset= (rfiuAudioRetWrite_idx >= rfiuAudioRetPlay_idx) ?  (rfiuAudioRetWrite_idx - rfiuAudioRetPlay_idx) : (rfiuAudioRetWrite_idx +RFI_AUDIO_RET_BUF_NUM - rfiuAudioRetPlay_idx);
#endif
    if( (SilenceCnt>12) &&  (AudioPlayOffset != 0) )
    {
        SilenceCnt=0;
#if ICOMMWIFI_SUPPORT
        if(iComm_speak)
           iCommAudioRetPlay_idx=(iCommAudioRetWrite_idx-0) % RFI_AUDIO_RET_BUF_NUM;
        else
           rfiuAudioRetPlay_idx=(rfiuAudioRetWrite_idx-0) % RFI_AUDIO_RET_BUF_NUM;
#else
        rfiuAudioRetPlay_idx=(rfiuAudioRetWrite_idx-0) % RFI_AUDIO_RET_BUF_NUM;
#endif
        AudioPlayOffset=0;
      #if RF_TX_AUDIORET_DEBUG_ENA
        if (rfAudioEnable == 1)
        		DEBUG_RFIU_P2("R");
      #endif    
      #if (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
        mCount=0;
      #endif
    }
    
    if(AudioPlayOffset>13)
    {
#if ICOMMWIFI_SUPPORT
        if(iComm_speak)
            iCommAudioRetPlay_idx=(iCommAudioRetWrite_idx-0) % RFI_AUDIO_RET_BUF_NUM;
        else
            rfiuAudioRetPlay_idx=(rfiuAudioRetWrite_idx-0) % RFI_AUDIO_RET_BUF_NUM;
#else
        rfiuAudioRetPlay_idx=(rfiuAudioRetWrite_idx-0) % RFI_AUDIO_RET_BUF_NUM;
#endif
        AudioPlayOffset=0;
    #if RF_TX_AUDIORET_DEBUG_ENA
		if (rfAudioEnable == 1)
       		DEBUG_RFIU_P2("O");
    #endif    
      
    }

    if(AudioPlayOffset > 0)
    {

    #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||\
         (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
        gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
    #elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
        if(iconflag[UI_MENU_SETIDX_MELODY_VOL] != 0)
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
        else
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_LO);
    #elif (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
        #if SPK_CONTROL
         #if (Melody_SNC7232_ENA)
         if (Melody_audio_level != 0)
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_HI);
         #endif
		else
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,GPIO_LEVEL_LO);
        #endif
    #endif
    uiInitDAC_Play();

    #if RFIU_RX_AUDIO_RETURN   
       if(AudioPlayOffset>11)
          AdjustIISFreq(-3);
       else if(AudioPlayOffset>9)
          AdjustIISFreq(-2);
       else if(AudioPlayOffset>7)
          AdjustIISFreq(-1);
       else if(AudioPlayOffset<6)
          AdjustIISFreq(1);
	   else if(AudioPlayOffset<5)
          AdjustIISFreq(2);
       else
          AdjustIISFreq(0);
    #endif
    #if 1
       //adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN+ADC_PGA_REDUCE_TALKBACK); //adc gain -6 dB
       #if (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
       if(nMotorTime>0)
           adcSetADC_MICIN_PGA_Gain(31);
       else
           adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN+ADC_PGA_REDUCE_TALKBACK); //adc gain -6 dB
       if(melodyflag!=2)
       {
           sysbackSetEvt(SYS_BACK_TURN_SPK_GPIO, 0);
           melodyflag=2;
       }
       mCount=0;
       #elif (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
       if(nMotorTime>0)
          adcSetADC_MICIN_PGA_Gain(31);
       else
          adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN+ADC_PGA_REDUCE_TALKBACK); //adc gain -6 dB
       #else
          adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN+ADC_PGA_REDUCE_TALKBACK); //adc gain -6 dB
       #endif
       iisboost_OFF();
    #endif
#if ICOMMWIFI_SUPPORT
	   if(iComm_speak)
	   {
         #if (Audio_mode == AUDIO_AUTO)
	       isr_marsDMA_PlayAuto(guiIISPlayDMAId,(u8*)iCommAudioRetBuf[iCommAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM]);
         #else
	       iisSetNextPlayDMA((u8*)iCommAudioRetBuf[iCommAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
		 #endif
	       iCommAudioRetPlay_idx = (iCommAudioRetPlay_idx+1) % RFI_AUDIO_RET_BUF_NUM;
	   }
	   else
	   {
         #if (Audio_mode == AUDIO_AUTO)
           isr_marsDMA_PlayAuto(guiIISPlayDMAId,(u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM]);
         #else
           iisSetNextPlayDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
         #endif
           rfiuAudioRetPlay_idx = (rfiuAudioRetPlay_idx+1) % RFI_AUDIO_RET_BUF_NUM;
       }
#else
       #if (Audio_mode == AUDIO_AUTO)
       isr_marsDMA_PlayAuto(guiIISPlayDMAId,(u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM]);
       #else
       iisSetNextPlayDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
       #endif
       rfiuAudioRetPlay_idx = (rfiuAudioRetPlay_idx+1) % RFI_AUDIO_RET_BUF_NUM;
#endif
       SilenceCnt=0;
    #if RF_TX_AUDIORET_DEBUG_ENA
		if (rfAudioEnable == 1)
        	DEBUG_RFIU_P2("%d ",AudioPlayOffset);
     #endif
        rfMuteVox = 1;
        VoxCnt = 50;
    }
    else
    {
    #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||\
         (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
        gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 0); //SPK off
    #elif (HW_BOARD_OPTION  == MR8100_GCT_VM9710) 
        #if SPK_CONTROL
            #if (Melody_SNC7232_ENA)
        	//DEBUG_RFIU_P2("Melody_play %d \n\n",Melody_play);
            if(Melody_play == 0)
                gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,0);
            #endif
        #endif
    #elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
        if(iconflag[UI_MENU_SETIDX_MELODY] == GPIO_MUSIC_STOP)
            gpioSetLevel(GPIO_GROUP_SPK_EN,GPIO_BIT_SPK_EN,0);
    #endif
    
    #if (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505)
       if(nMotorTime>0)
           adcSetADC_MICIN_PGA_Gain(31);
       else
           adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN);
       if(mCount>5)
       {
           if(melodyflag!=1)
           {
               sysbackSetEvt(SYS_BACK_TURN_SPK_GPIO, 1);
               melodyflag=1;
           }
       }
       else
        mCount++;
    #elif(HW_BOARD_OPTION  == MR8100_GCT_VM9710)
       if(nMotorTime>0)
          adcSetADC_MICIN_PGA_Gain(31);
       else if(Melody_play != 0)
          adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN+3);
       else
          adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN);
       adcInitDAC_Play(31);
    #elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
       if(iconflag[UI_MENU_SETIDX_MELODY] != GPIO_MUSIC_STOP)
          adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN+5);
       else
          adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN);
       adcInitDAC_Play(31);
    #else
       adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN);
    #endif
    
    #if ((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
        (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
        iisboost_OFF(); //外部電路放大器 
    #else
        iisboost_ON();
    #endif
    #if (Audio_mode == AUDIO_AUTO)
       isr_marsDMA_PlayAuto(guiIISPlayDMAId,rfiuAudioZeroBuf);
    #else
       iisSetNextPlayDMA(rfiuAudioZeroBuf, 1024);
    #endif
       SilenceCnt ++;
    #if RF_TX_AUDIORET_DEBUG_ENA
		if (rfAudioEnable == 1)
      		DEBUG_RFIU_P2("S");
    #endif
        if (VoxCnt <= 0)
            rfMuteVox = 0;
        else
            VoxCnt--;
    }
    #if(Audio_mode != AUDIO_AUTO)
	iisStartPlay();
    #endif
}

#else
void rfiu_WrapTx_Task_UnitX(void* pData)
{

}

void rfiu_WrapRx_Task_UnitX(void* pData)
{
    int RFUnit;
    unsigned int VideoReadIdx;
    unsigned int BufWriteAddrOffset,BufReadAddrOffset;
    unsigned int RemainBufSize;
    unsigned int AudioPsLen,VideoPsLen;
    unsigned int AudioPayloadLen,VideoPayLoadLen;
    unsigned int StreamType;
    DEF_RF_PS_HEADER PsHeader;
    int i;
    int SyncFlag,PackSize;
    unsigned int  cpu_sr = 0;
    u8      err;
    int VideoFrameCnt,FrameRate;
    int AudioFrameCnt;
    int IsVideoFrame;
    unsigned int t1,t2,dt;

	PsHeader.PreStatus = 0;
	
    //-------------------//
    RFUnit= (int)pData;
	
    OS_ENTER_CRITICAL();
    rfiuRxVideoBufMngWriteIdx[RFUnit]=0;
    rfiuRxIIsSounBufMngWriteIdx[RFUnit]=0;
    
    SyncFlag=0;
    
    VideoReadIdx = (rfiuRxVideoBufMngWriteIdx[RFUnit]) % VIDEO_BUF_NUM;
    
    BufWriteAddrOffset=0;
    BufReadAddrOffset=0;
    //BufReadAddrOffset= (gRfiuUnitCntl[RFUnit].BufWritePtr<<13)+(gRfiuUnitCntl[RFUnit].WritePtr_Divs<<10);    
    BufReadAddrOffset= (gRfiuUnitCntl[RFUnit].BufReadPtr<<13);    

    OS_EXIT_CRITICAL();

#if(RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL)
    for(i = 0; i < VIDEO_BUF_NUM; i++) 
    {
        rfiuRxVideoBufMng[RFUnit][i].buffer   =rfiuRxVideoBuf[RFUnit];
        rfiuRxVideoBufMng[RFUnit][i].size     =0;
    }

    for(i = 0; i < IIS_BUF_NUM; i++) 
    {
        rfiuRxIIsSounBufMng[RFUnit][i].buffer = rfiuRxAudioBuf[RFUnit];
        rfiuRxIIsSounBufMng[RFUnit][i].size   = 0; 
    }
#endif

#if RX_SNAPSHOT_SUPPORT
    rfiuRxDataBufMng[RFUnit].buffer=rfiuRxDataBuf[RFUnit];
    rfiuRxDataBufMng[RFUnit].size  = 0; 
#endif


 #if TUTK_SUPPORT 
    OSSemSet(P2PVideoCmpSemEvt[RFUnit],0, &err);  
    OSSemSet(P2PAudioCmpSemEvt[RFUnit],0, &err); 

    P2PVideoBufReadIdx[RFUnit]  = rfiuRxVideoBufMngWriteIdx[RFUnit];; 
    P2PAudioBufReadIdx[RFUnit]  = rfiuRxIIsSounBufMngWriteIdx[RFUnit];
    
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: P2PVideoCmpSemEvt is %d.\n", err);
    }
 #endif

    VideoFrameCnt=0;
    AudioFrameCnt=0;
    FrameRate=0;
    timerCountRead(guiRFTimerID, &t1);
    gRfiuUnitCntl[RFUnit].FrameRate=0;
    #if AUDIO_DEBUG_ENA
    RF_Audiosync = 0;
    #endif
    DEBUG_RFIU_P2("\n============rfiu_WrapRx_Task_UnitX(%d)(%d)(%d)(%d)==========\n",RFUnit,gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime,gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime,asfSectionTime);

    if((gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime == 0xa55aaa55) && (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime == 0xa55aaa55))
    {
        gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime = 0;
        gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime = 0;
    }
  #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
    if(gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime > gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime)
    {
        gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime = gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime;
    }
    else if (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime > gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime)
    {
        gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime = gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime;
    }
    if((gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime == 0) && (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime == 0))
    {
        Video_totaltime[RFUnit] = 0;
        Audio_totaltime[RFUnit] = 0;
    }

    if(Lose_video_time[RFUnit] == 0)
    {
        Lose_video_time[RFUnit] = gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime - Video_totaltime[RFUnit] ;
    }
    else
    {
        Lose_video_time[RFUnit] += (gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime - Video_totaltime[RFUnit] );
    }
    DEBUG_RFIU_P2("\n============(Vlose %ld,Tx %ld,RX %ld)==========\n",Lose_video_time[RFUnit],gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime,Video_totaltime[RFUnit]);
    Video_totaltime[RFUnit] = 0;
    
    if(Lose_audio_time[RFUnit] == 0)
    {
        Lose_audio_time[RFUnit] = gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime - Audio_totaltime[RFUnit] ;
    }
    else
    {
        Lose_audio_time[RFUnit] += (gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime - Audio_totaltime[RFUnit] );
    }
    DEBUG_RFIU_P2("\n============(Alose %ld,Tx %ld,RX %ld)==========\n",Lose_audio_time[RFUnit],gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime,Audio_totaltime[RFUnit]);
    Audio_totaltime[RFUnit] = 0;
    
  #endif
	while(1)
    {
        if(gRfiuUnitCntl[RFUnit].RX_Wrap_Stop)
        {
           DEBUG_RFIU_P2("*");
           OSTimeDly(1);
           continue;
        }
            
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    	if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
        {
            VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = (u16)gRfiuUnitCntl[RFUnit].TX_PicWidth;
            VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = (u16)gRfiuUnitCntl[RFUnit].TX_PicHeight;
    	    RfRxVideoPackerSubTaskCreate(RFUnit, 
	                                 &VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX]);
        }
#endif

        OS_ENTER_CRITICAL();
        BufWriteAddrOffset=(gRfiuUnitCntl[RFUnit].BufWritePtr<<13)+(gRfiuUnitCntl[RFUnit].WritePtr_Divs<<10);
        OS_EXIT_CRITICAL();
        RemainBufSize=(BufWriteAddrOffset+RFI_TOTAL_BUF_SIZE-BufReadAddrOffset) % RFI_TOTAL_BUF_SIZE;   

        if(RemainBufSize > RFI_TOTAL_BUF_SIZE/2)
        {
            DEBUG_RFIU_P2("--->WrapRX too slow! RemainBufSize=%d\n",RemainBufSize);
			OS_ENTER_CRITICAL();
            BufReadAddrOffset= (gRfiuUnitCntl[RFUnit].BufWritePtr<<13)+(gRfiuUnitCntl[RFUnit].WritePtr_Divs<<10); 
            OS_EXIT_CRITICAL();
			continue;
        }

        if(SyncFlag <= 0)
        {
           SyncFlag=SyncPSHeader_RF(RFUnit,&BufReadAddrOffset,RemainBufSize);
        }
        
        if(SyncFlag > 0)
        {
            RemainBufSize=(BufWriteAddrOffset+RFI_TOTAL_BUF_SIZE-BufReadAddrOffset) % RFI_TOTAL_BUF_SIZE;
            if(RemainBufSize >= PS_MAX_HEADERLEN ) 
            {
                 ParsePSHeader_RF(RFUnit,BufReadAddrOffset,&PsHeader);

                 //Lucian: Stream type bit[7:6] is used for checking 00
                 if( (PsHeader.PayLoadSize > (RFI_TOTAL_BUF_SIZE/2)) || (PsHeader.PayLoadSize & 0x03) || (PsHeader.StreamType& 0xc0) )
                 {
                     DEBUG_RFIU_P2("Warning! Pack Size not Valid:%d,0x%x\n",PsHeader.PayLoadSize,PsHeader.StreamType);
                     BufReadAddrOffset += PS_MAX_HEADERLEN;
                     SyncFlag=0;
                     continue;
                 }
                 
                 PackSize=PsHeader.HeaderSize + PsHeader.PayLoadSize;
                 if(RemainBufSize >=  PackSize) 
                 {
                        SyncFlag=0;
                    #if(RFI_TEST_WRAP_OnPROTOCOL && RFI_SELF_TEST_TXRX_PROTOCOL)  
                        //do nothing for test.
                    #else
                        IsVideoFrame=UnpackPS_RF(RFUnit,BufReadAddrOffset,&PsHeader); 
                        VideoFrameCnt += (IsVideoFrame & 0x01);
                        AudioFrameCnt += ((IsVideoFrame>>1) & 0x01);
                    #endif
                        
                        BufReadAddrOffset += PackSize;
                        if(BufReadAddrOffset>=RFI_TOTAL_BUF_SIZE)
                        {
                            if(BufReadAddrOffset!=RFI_TOTAL_BUF_SIZE)
                                DEBUG_RFIU_P2("Warning! not match Buffer end!\n");
                            if( (PsHeader.Status & PS_STA_LASTONE) == 0)
                                DEBUG_RFIU_P2("Warning! not Last one packet!\n");

                            BufReadAddrOffset=0;
                            DEBUG_RFIU_P("=====>Turn arround Buffer start!\n");
                        }

                        //====== Cal Frame Rate======//
                        if(VideoFrameCnt>30)
                        {
                           timerCountRead(guiRFTimerID, &t2);
                           if(t1>t2)
                              dt=t1-t2;
                           else
                              dt=(t1+TimerGetTimerCounter(TIMER_7))-t2;

                           if(dt==0)
                              dt=1;

                           FrameRate = VideoFrameCnt*10000/dt;
                           if(FrameRate>30)
                              FrameRate=30;

                           //DEBUG_RFIU_P2("RF-%d FrameRate=%d\n",RFUnit,FrameRate);
                           gRfiuUnitCntl[RFUnit].FrameRate=FrameRate;  
                           if(AudioFrameCnt==0)
                           {
                              rfiu_ForceTXSetSameResolution(RFUnit);
                              DEBUG_RFIU_P2("-->Warning!! No Audio is received. Rstart TX!!\n");
                           }
                           t1=t2;
                           VideoFrameCnt=0;
                           AudioFrameCnt=0;
                        }		
                        #if RF_LETENCY_DEBUG_ENA
                           if(sysRFRxInMainCHsel==RFUnit)
        				      DEBUG_RFIU_P("WrapBuf=%d\n",RemainBufSize >> 10);
        				#endif

                 }
                 else 
                     OSTimeDly(1);                 
            }
            else 
            {
               OSTimeDly(1);             
            }
        }
        else
        {
            OSTimeDly(1);
        }
                
    }

}

int SyncPSHeader_RF(int RFUnit,
                            unsigned int *pBufReadAddrOffset,
                            unsigned int RemainBufSize
                           )
{
   unsigned int *pp,*qq;
   int i;
   u8 err;
   unsigned int Size2BufEnd;
   //===//
   
   if(RemainBufSize < PS_MAX_HEADERLEN)
      return 0;
   
   pp = (unsigned int *)(rfiuOperBuf[RFUnit] + *pBufReadAddrOffset);
   
#if 0   
   qq = (unsigned int *)(rfiuOperBuf[0] + *pBufReadAddrOffset);
   if((*pp >> 8) !=  PS_START_CODE_PREFIX)
      DEBUG_RFIU_P2("===>Warning! PREFIX mismatch: 0x%x\n",*pp);
#endif   
   for(i=0;i<RemainBufSize/4;i++)
   {   //Lucian: Stream type bit[7:6] is used for checking 00
       if( ((*pp >> 8) ==  PS_START_CODE_PREFIX) && ( (*pp & 0xc0) == 0) )
       {
           Size2BufEnd = RFI_TOTAL_BUF_SIZE - (*pBufReadAddrOffset);
           if(Size2BufEnd <= PS_MAX_HEADERLEN)
           {
               DEBUG_RFIU_P2("---->Waring!!RX Size2BufEnd is invalid:%d\n",Size2BufEnd);
           }
           else
           {
               rfiuRXWrapSyncErrCnt[RFUnit]=0;
               if(i != 0)
               {
                  DEBUG_RFIU_P2("===>Sync:%d\n",i);
                  return 2;
               }
               else
               {
                  return 1;
               }
           }
       }

       *pBufReadAddrOffset = *pBufReadAddrOffset + 4;   
       if(*pBufReadAddrOffset >=RFI_TOTAL_BUF_SIZE)
       {
          *pBufReadAddrOffset=0;
          pp=(unsigned int *)(rfiuOperBuf[RFUnit]);
       }
       else
       {
          pp ++;
       }
   }

   rfiuRXWrapSyncErrCnt[RFUnit] ++;
   DEBUG_RFIU_P2("===>No Sync:%d\n",i);

   if(rfiuRXWrapSyncErrCnt[RFUnit] > 8)
   {
       OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_ERROR<<(RFUnit*8), OS_FLAG_SET, &err);
       //DEBUG_RFIU_P2("======== RF-%d Fatal Error! Reboot!======\n",RFUnit);
       //sysForceWdttoReboot();
       DEBUG_MP4("\n======== RF Wrap Error! Resync:%d!======\n",RFUnit);
       if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==0)
           sysback_RF_SetEvt(SYS_BACKRF_RFI_FORCERESYNC, RFUnit);
   }

   return -1;
}

int ParsePSHeader_RF( int RFUnit,
                             unsigned int BufReadAddrOffset,
                             DEF_RF_PS_HEADER *pHeader
                            )
{
   unsigned int *pp;
   int i;
   int PTS_flag;
   //===//
   
   pp = (unsigned int *)(rfiuOperBuf[RFUnit] + BufReadAddrOffset);

   pHeader->StreamType =  pp[0] & 0x0ff;
   pHeader->PayLoadSize= (pp[1] >> 8) & 0x0fffff;
   pHeader->Status     =  pp[1] & 0x0ff;
   pHeader->SeqNum     = (pp[1] >> 28) & 0x0f;

   if(pHeader->Status & PS_STA_PTS)
   {
      pHeader->PTS_L          = pp[2];
      pHeader->BotFldOffset   = pp[3];

      pHeader->HeaderSize=16;
   }
   else
   {
      pHeader->PTS_L          = 0xffffffff;
      pHeader->BotFldOffset   = 0xffffffff;

      pHeader->HeaderSize=8;
   }
  
   return 0;
}

int UnpackPS_RF(
                        int RFUnit,
                        unsigned int BufReadAddrOffset,
                        DEF_RF_PS_HEADER *pHeader
                      )
{
#if(RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL)
   unsigned char *pp;
   int i;
   int PTS_flag;
   unsigned char *pBuf;   
   unsigned int  cpu_sr = 0;  
   int AVFrame;
 #if ASF_DEBUG_ENA
   int ASF_time=0;
   int ASF_size=0;
 #endif
 #if DETECT_RX_VOLUME
   u8   *pBuf1;
   u32  Sum;
   u32  Size;
 #endif
 #if AUDIO_DEBUG_ENA
   u32 RFiiscnt = 0;
   u32 TXiiscnt =0;
 #endif

   #if VIDEO_STARTCODE_DEBUG_ENA	
   unsigned char *pReadBuf;
   u16            video_value;
   #endif
   //===//

	#if CHECK_VIDEO_BITSTREAM	
	unsigned int EndCodeSize = 4;	
	#else
	unsigned int EndCodeSize = 0;
	#endif
   
   AVFrame=0;
   pp = (unsigned char *)(rfiuOperBuf[RFUnit] + BufReadAddrOffset + pHeader->HeaderSize);
   
   if( (pHeader->StreamType >> 4) == 0x01)
   {    //Video
        OS_ENTER_CRITICAL();
        pBuf=rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer;
		#if VIDEO_STARTCODE_DEBUG_ENA
		pReadBuf=VideoClipOption[RFUnit].VideoBufMng[VideoClipOption[RFUnit].VideoBufMngReadIdx].buffer; 
		video_value	= (pvcoRfiu[RFUnit]->VideoCmpSemEvt->OSEventCnt);
		#endif
		OS_EXIT_CRITICAL();		

		#if VIDEO_STARTCODE_DEBUG_ENA
		OS_ENTER_CRITICAL();
		if((pBuf < pReadBuf)
		&&((pBuf + pHeader->PayLoadSize + EndCodeSize) > pReadBuf)
		)
		{
			DEBUG_ASF("Warning!!! CH%02d RF write fast than ASF read, %d, %d\n", RFUnit, video_value, VideoClipOption[RFUnit].CurrentVideoSize);
			DEBUG_ASF("Warning!!! AA. CH%02d %d, 0x%08x, BB. CH%02d %d, 0x%08x\n", RFUnit, rfiuRxVideoBufMngWriteIdx[RFUnit], rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer,
			RFUnit, VideoClipOption[RFUnit].VideoBufMngReadIdx, VideoClipOption[RFUnit].VideoBufMng[VideoClipOption[RFUnit].VideoBufMngReadIdx].buffer);							
		}		
		OS_EXIT_CRITICAL();									
		#endif

		if(pBuf + pHeader->PayLoadSize + EndCodeSize < rfiuRxVideoBufEnd[RFUnit]) 
		{
			mcpu_ByteMemcpy(pBuf, pp, pHeader->PayLoadSize);

			#if CHECK_VIDEO_BITSTREAM	
			*(unsigned int*)(pBuf + pHeader->PayLoadSize) = 0x000001B6;
			#endif
		}
		else //Not enough
		{						
			//case: Pre is PS_STA_CURRENT_IND
			if(pHeader->PreStatus & PS_STA_CURRENT_IND)
			{
				//copy A to head
				mcpu_ByteMemcpy(rfiuRxVideoBuf[RFUnit], pBuf - rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size, rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size);
				pBuf = rfiuRxVideoBuf[RFUnit] + rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size; 
				//copy B
				mcpu_ByteMemcpy(pBuf, pp, pHeader->PayLoadSize);				

				#if CHECK_VIDEO_BITSTREAM	
				*(unsigned int*)(pBuf + pHeader->PayLoadSize) = 0x000001B6;
				#endif
			}
			else
			{
				pBuf=rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer=rfiuRxVideoBuf[RFUnit];
				mcpu_ByteMemcpy(pBuf, pp, pHeader->PayLoadSize);

				#if CHECK_VIDEO_BITSTREAM	
				*(unsigned int*)(pBuf + pHeader->PayLoadSize) = 0x000001B6;
				#endif				
			}
		}

        pHeader->PreStatus = pHeader->Status;
		
        if(pHeader->Status & PS_STA_CURRENT_IND)
        {
           OS_ENTER_CRITICAL();
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer=pBuf + pHeader->PayLoadSize;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size +=pHeader->PayLoadSize;
           OS_EXIT_CRITICAL();		  
		}
        else
        {
           #if NIC_SUPPORT
		   if(EnableStreaming)
	        OSSemPost(VideoRTPCmpSemEvt[RFUnit]); 
            #if TUTK_SUPPORT 
            if(P2PEnableStreaming[RFUnit]) 
                OSSemPost(P2PVideoCmpSemEvt[RFUnit]);  
            #endif
           #endif

          // DEBUG_RFIU_P2("%d ",pHeader->Status & PS_STA_IFRAME);
           OS_ENTER_CRITICAL();
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer =pBuf - rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size  +=pHeader->PayLoadSize + EndCodeSize;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].flag   =pHeader->Status & PS_STA_IFRAME;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].asfflag=pHeader->Status & PS_STA_IFRAME;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time   =(s64)pHeader->PTS_L;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].offset =pHeader->BotFldOffset;
          #if ASF_DEBUG_ENA
           ASF_time = rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time;
          #endif
          #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
            if((Lose_video_time[RFUnit] + rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time) > (asfSectionTime*1000))
            {
                Lose_video_time[RFUnit] = 0;
                rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time = 66;
            }
          #endif
           if((rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time > (asfSectionTime*1000)) || (rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time < 0))
           {
             #if ASF_DEBUG_ENA
           	  RX_skip_V += rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time - 66;
             #endif
              DEBUG_RFIU_P2("Video PTS is invalid:%d\n",rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time);
              rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time = 66;
           }
          #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
            Video_totaltime[RFUnit] += (u32)rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time; 
          #endif
           #if MULTI_CHANNEL_RF_RX_VIDEO_REC
            if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
            {         
                if(pvcoRfiu[RFUnit]->PackerTaskCreated && pvcoRfiu[RFUnit]->sysCaptureVideoStart)
                {
                   pvcoRfiu[RFUnit]->VideoTimeStatistics  += rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time;
                   pvcoRfiu[RFUnit]->CurrentVideoSize     += rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size;
                   pvcoRfiu[RFUnit]->CurrentVideoTime     += rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time;
                }
             }
           #endif

			#if VIDEO_STARTCODE_DEBUG_ENA
			if(((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer)) == 0x00) && ((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer+1)) == 0x00) 
	        && ((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer+2)) == 0x01) && (((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer+3)) == 0xB6)))
			{
			}
			else
			{
				DEBUG_ASF("Warning!!! X CH%02d size=%d, UnpackPS_RF MPEG4 start code error - %d\n", RFUnit, rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size, VideoClipOption[RFUnit].VideoCmpSemEvt->OSEventCnt);
			}
			#endif

           rfiuRxVideoBufMngWriteIdx[RFUnit] = (rfiuRxVideoBufMngWriteIdx[RFUnit] + 1) % VIDEO_BUF_NUM;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].offset=0;			
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size=0;

		   #if 0
		   if(pBuf + pHeader->PayLoadSize < rfiuRxVideoBufEnd[RFUnit]-(MPEG4_MIN_BUF_SIZE>>1)) //Lsk: avoid RX/ASF buffer overwrite
           {
              rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer=pBuf + pHeader->PayLoadSize;
           }
           else
           {
              rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer=rfiuRxVideoBuf[RFUnit];
           }
		   #else
		   rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer=pBuf + pHeader->PayLoadSize + EndCodeSize;
		   #endif
		   OS_EXIT_CRITICAL();
           //DEBUG_RFIU_P2("Unpack Video: size=%d\n",VideoBufMng[VideoBufMngWriteIdx].size);
           #if MULTI_CHANNEL_RF_RX_VIDEO_REC
            if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
                if(pvcoRfiu[RFUnit]->PackerTaskCreated && pvcoRfiu[RFUnit]->sysCaptureVideoStart)
                {
	                OSSemPost(pvcoRfiu[RFUnit]->VideoCmpSemEvt);
                  #if ASF_DEBUG_ENA
                    RX_time_V += ASF_time;
                    RX_sem_V++;
                    DEBUG_RFIU_P2("RF_VideoTime = %d\n",RX_time_V);
                  #endif
                }
           #endif
           AVFrame |=0x1;
        }        
   }
   else if( ((pHeader->StreamType >> 4) == 0x02) || ((pHeader->StreamType >> 4) == 0x03))
   {    //Audio
        OS_ENTER_CRITICAL();
        pBuf=rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer;
		OS_EXIT_CRITICAL();
        mcpu_ByteMemcpy(pBuf, pp, pHeader->PayLoadSize);
        if(pHeader->Status & PS_STA_CURRENT_IND)
        {
           OS_ENTER_CRITICAL();
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer=pBuf + pHeader->PayLoadSize;
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size +=pHeader->PayLoadSize;
           OS_EXIT_CRITICAL();
		}
        else
        {
           #if NIC_SUPPORT 
	         #if TUTK_SUPPORT 
	           if(P2PEnableStreaming[RFUnit]) 
	             OSSemPost(P2PAudioCmpSemEvt[RFUnit]);  
	         #endif
               if(EnableStreaming)
                 OSSemPost(AudioRTPCmpSemEvt[RFUnit]);    
           #endif

           OS_ENTER_CRITICAL();
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer=pBuf - rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size;
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size +=pHeader->PayLoadSize;
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].flag  =pHeader->Status & PS_STA_IFRAME;
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time  =pHeader->PTS_L;
          #if ASF_DEBUG_ENA
           ASF_time = rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time;
           ASF_size = rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size;
          #endif
          #if AUDIO_DEBUG_ENA
           if(RF_Audiosync == 0)
           {
               RFiiscnt = *rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer;
               RFiiscnt = RFiiscnt<<8;
   
               RFiiscnt += *(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer+1);
               RFiiscnt = RFiiscnt<<8;
   
               RFiiscnt += *(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer+2);
               RFiiscnt = RFiiscnt<<8;
   
               RFiiscnt += *(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer+3);
           }
           else
           { 
               RFiiscnt++;
   
               TXiiscnt = *rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer;
               TXiiscnt = TXiiscnt<<8;
   
               TXiiscnt += *(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer+1);
               TXiiscnt = TXiiscnt<<8;
   
               TXiiscnt += *(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer+2);
               TXiiscnt = TXiiscnt<<8;
   
               TXiiscnt += *(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer+3);
   
               if(RFiiscnt != TXiiscnt)
                   DEBUG_RFIU_P2("\nRF ERROR index = %d,cnt %ld,streamcnt %ld,address 0x%08x\n",rfiuRxIIsSounBufMngWriteIdx[RFUnit],RFiiscnt,TXiiscnt,&rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer);
           }
           RF_Audiosync = 1;
          #endif
          #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
           if((Lose_audio_time[RFUnit] + rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time) > (asfSectionTime*1000))
           {
               Lose_audio_time[RFUnit] = 0;
               rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time = 128;
           }
          #endif
           if((rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time > (asfSectionTime*1000)) || (rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time<0))
           {
              DEBUG_RFIU_P2("Audio PTS is invalid:%d\n",rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time);
           }
          #if(INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
            Audio_totaltime[RFUnit] += (u32)rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time; 
          #endif

           #if MULTI_CHANNEL_RF_RX_VIDEO_REC
            if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
                if(pvcoRfiu[RFUnit]->PackerTaskCreated && pvcoRfiu[RFUnit]->sysCaptureVideoStart)
                    pvcoRfiu[RFUnit]->CurrentAudioSize     += rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size;
           #endif

        #if DETECT_RX_VOLUME
           pBuf1    = rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer;
           Size     = rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size;
           Sum      = 0;
           for(i = 0; i < Size; i++, pBuf1++)
           {
               if(*pBuf1 >= 0x80)
                   Sum    += *pBuf1 - 0x80;
               else
                   Sum    += 0x80 - *pBuf1;
           }
           RxVolume[RFUnit] = Sum / Size;
           //DEBUG_IIS("ch%d ,%02x\n",RFUnit, RxVolume[RFUnit]);
        #endif

           rfiuRxIIsSounBufMngWriteIdx[RFUnit] = (rfiuRxIIsSounBufMngWriteIdx[RFUnit] + 1) % IIS_BUF_NUM;
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size=0;
           if(pBuf + pHeader->PayLoadSize < (rfiuRxAudioBufEnd[RFUnit]-IIS_CHUNK_SIZE*2) )
           {
              rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer=pBuf + pHeader->PayLoadSize;
           }
           else
           {
              rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer=rfiuRxAudioBuf[RFUnit];
           }
		   OS_EXIT_CRITICAL();
           //DEBUG_RFIU_P2("Unpack Audio: size=%d\n",iisSounBufMng[iisSounBufMngWriteIdx].size);
           #if MULTI_CHANNEL_RF_RX_VIDEO_REC
            if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
                if(pvcoRfiu[RFUnit]->PackerTaskCreated && pvcoRfiu[RFUnit]->sysCaptureVideoStart)
                {
	                OSSemPost(pvcoRfiu[RFUnit]->iisCmpSemEvt);
                  #if ASF_DEBUG_ENA
                    RX_time_A += ASF_time;
                    DEBUG_RFIU_P2("RF_AudioTime = %d\n",RX_time_A);
                    if(ASF_time != 128 || ASF_size!=2048)
                        DEBUG_RFIU_P2("<RRR> 1.over duration <%d, %d>\n", ASF_time, ASF_size);
                    RX_sem_A++;
                  #endif
                }
           #endif
           AVFrame |=0x2;
        }        
        
   }
   else if( (pHeader->StreamType >> 4) == 0x00 )
   {
        DEBUG_RFIU_P2("Data PayLoad Size(%d)=%d\n",RFUnit,pHeader->PayLoadSize);
#if RX_SNAPSHOT_SUPPORT
        pBuf=rfiuRxDataBufMng[RFUnit].buffer;
        mcpu_ByteMemcpy(pBuf, pp, pHeader->PayLoadSize);
        if(pHeader->Status & PS_STA_CURRENT_IND)
        {
           rfiuRxDataBufMng[RFUnit].buffer=pBuf + pHeader->PayLoadSize;
           rfiuRxDataBufMng[RFUnit].size +=pHeader->PayLoadSize;
           DEBUG_RFIU_P2("\n==Warning! Data Payload is illegle!==\n");
		}
        else
        {
           rfiuRxDataBufMng[RFUnit].buffer=rfiuRxDataBuf[RFUnit];
           rfiuRxDataBufMng[RFUnit].size +=pHeader->PayLoadSize;
           //===Write to SD ===//
        #if(SD_CARD_DISABLE==0)
           sysSetEvt(SYS_EVT_RX_DATASAVE, RFUnit);
        #endif
           uiRetrySnapshot[RFUnit] = UI_SET_RF_SNAP_DONE;
        } 
#endif       
   }

   return AVFrame;
   
#else
   #if 0
   unsigned int *pp,*qq;
   int i;
   //====//
   pp = (unsigned int *)(rfiuOperBuf[0] + BufReadAddrOffset + pHeader->HeaderSize);
   qq = (unsigned int *)(rfiuOperBuf[1] + BufReadAddrOffset + pHeader->HeaderSize);

   for(i=0;i<pHeader->PayLoadSize/4;i++)
   {
      if(*pp != *qq)
      {
        DEBUG_RFIU_P2("==>Compare error=0x%x,0x%x,Len=%d\n",*pp,*qq,pHeader->PayLoadSize);
        return 0;
      }

      pp ++;
      qq ++;
   }

   return 0;
   #endif
#endif   


}

void rfiu_RxMpeg4DecTask_UnitX(void* pData)
{
    //===Video===//
    u32 *pVideoFlag;
    u32 *pVideoSize;
    u32  VideoOffset;
    u8 *pVideoBuf;
    u32 VideoDuration;

    u32 Vop_Result;
  #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
	MP4_Option mpeg4Dec;
  #elif (VIDEO_CODEC_OPTION == H264_CODEC)
    H264_ENC_CFG H264Enc;
    H264_DEC_CFG H264Dec;
	VIDEO_INFO video_info;  
  #endif
    //===Audio===//
    u32 *pAudioFlag;
    u32 *pAudioSize;
    u8 *pAudioBuf;
    u32 AudioDuration;
	u32 prev_AudioTime;

    int VideoLatency;

    //======//
    unsigned int t1,t2,dt,FrameRate,FrameCnt;
    u8 err;
    s32 i;
    int RFUnit;
  #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    u8 VolHeader[32];
    u32 VolHeaderSize;
  #endif
    unsigned int vop_type;
    int SYNC=1;
    u32 VideoReadIdx;
    int VideoBufOffset,AudioBufOffset;
    int VideoDispOffset,AudioPlayOffset;
    u32 SyncTime;
    unsigned int  cpu_sr = 0; 
    int RunVideo,RunAudio;
    int First_Audio=0;
	int VideoCount;
    int MpegErrCnt=0;
	
    //-----------------------//
    FrameCnt=0;
  #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
    memset(&mpeg4Dec,0,sizeof(MP4_Option));
  #elif (VIDEO_CODEC_OPTION == H264_CODEC)
    memset(&video_info,0,sizeof(VIDEO_INFO));
  #endif
    RFUnit= (int)pData;
    VideoReadIdx=rfiuRxVideoBufMngWriteIdx[RFUnit];
    
    rfiuRxIIsSounBufMngReadIdx[RFUnit]=rfiuRxIIsSounBufMngWriteIdx[RFUnit] % IIS_BUF_NUM;

    SyncTime=0;
	prev_AudioTime=0;
	VideoCount=0;
        	
    rfiuVideoTimeBase[RFUnit]=0;
    rfiuVideoBufFill_idx[RFUnit]=0;
	rfiuVideoBufPlay_idx[RFUnit]=0;

	rfiuAudioBufPlay_idx[RFUnit]=0;
	rfiuAudioBufFill_idx[RFUnit]=0;

    //DEBUG_RFIU_P2("\n=====>Debug:sysRFRxInMainCHsel=%d\n",sysRFRxInMainCHsel);
    if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
    {
        OS_ENTER_CRITICAL();
        if(sysRFRxInMainCHsel==0x0ff)
        {
            sysRFRxInMainCHsel=RFUnit;
        }
        OS_EXIT_CRITICAL();
    }
		
	if(sysRFRxInMainCHsel==RFUnit)
    {
        First_Audio=1;
	    rfiuRxMainVideoPlayStart=0;
	    rfiuRxMainAudioPlayStart=0;
	    for(i=0;i<DISPLAY_BUF_NUM;i++)
	      rfiuMainVideoPresentTime[i]=0;

	    rfiuMainVideoTime=0; 
	    rfiuMainVideoTime_frac=0;
		
	    rfiuMainAudioTime=0;
        rfiuAudioTimeBase=0;
	    rfiuMainAudioTime_frac=0;

		if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
        {
		  for(i=0;i<MAX_RFIU_UNIT;i++)
		    rfiuVideoTimeBase[i]=0;
	    }
        
     #if 1  
        if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
        {
        #if( (SW_APPLICATION_OPTION  ==  MR8100_BABYMONITOR) || (SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM) )
           OSSemPend(gRfiuSWReadySemEvt, OS_IPC_WAIT_FOREVER, &err);
           if( (sysEnMenu==0) && (sysEnSnapshot==0) )
           {
                if(  rfiuPrevWifiStatus[sysRFRxInMainCHsel] != gRfiuUnitCntl[sysRFRxInMainCHsel].RFpara.WifiLinkOn )
                {
                    DEBUG_RFIU_P2("==Wifi Status Change==\n");
                    memset_hw_Word(MainVideodisplaybuf[0], 0x80800000, VIDEODISPBUF_SIZE * DISPLAY_BUF_NUM); //clear display buffer.        
                }    
                if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight>=720)
                {

                    if(sysEnZoom)
                    {
                    #if RFRX_FULLSCR_HD_SINGLE
                      iduPlaybackMode(1280/2,720/2,RF_RX_2DISP_WIDTH*2);
                    #else
                      iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
                    #endif
                    }
                    else
                    {
                    #if RFRX_FULLSCR_HD_SINGLE
                      iduPlaybackMode(1280/2,720/2,RF_RX_2DISP_WIDTH);
                    #else
                      iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
                    #endif
                    }
                }
                else if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight == 240)
                {
                      if(sysEnZoom)
                         iduPlaybackMode(320,240,RF_RX_2DISP_WIDTH*2);
                      else
                         iduPlaybackMode(320,240,RF_RX_2DISP_WIDTH);
                }
                else
                {
                   if(sysEnZoom)
                      iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH*2);
                   else
                      iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
                }

                rfiuPrevWifiStatus[sysRFRxInMainCHsel]=gRfiuUnitCntl[sysRFRxInMainCHsel].RFpara.WifiLinkOn;
            }
            OSSemPost(gRfiuSWReadySemEvt);
        #else
                if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight>=720)
                {
                #if RFRX_FULLSCR_HD_SINGLE
                      iduPlaybackMode(1280/2,720/2,RF_RX_2DISP_WIDTH);
                #else
                      iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
                #endif
                }
                else if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight == 240)
                {
                      iduPlaybackMode(320,240,RF_RX_2DISP_WIDTH);
                }
                else
                {
                   iduPlaybackMode(RF_RX_2DISP_WIDTH,RF_RX_2DISP_HEIGHT,RF_RX_2DISP_WIDTH);
                }
        #endif
        }
        else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR)
        {            
             iduPlaybackMode(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight,RF_RX_2DISP_WIDTH);
        }
      #endif     
	    
	  #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
        if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) || (sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR) )
        {
            iisStopPlay();
    	    if(guiIISPlayDMAId==0xff)
    	    {
    	        DEBUG_RFIU_P2("--->Open RF Audio:%d!\n",RFUnit);
    	        marsDMAOpen(&guiIISPlayDMAId, rfiuAudioPlayDMA_ISR);
    	    }
    	    else
    	        DEBUG_RFIU_P2("Error! Audio Play DMA is occurpied!\n");
          #if ((UI_VERSION == UI_VERSION_RDI) || (UI_VERSION == UI_VERSION_RDI_2) || (UI_VERSION == UI_VERSION_RDI_3))
            if (((uiGetMenuMode() == VIDEO_MODE) || (uiGetMenuMode() == ZOOM_MODE)) && (uiGetVolumeLevel() != 0))
            {
                gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
            }
          #endif
        }
        #if RFRX_QUAD_AUDIO_EN
        else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
        {
            iisStopPlay();
    	    if(guiIISPlayDMAId==0xff)
    	    {
    	        DEBUG_RFIU_P2("--->Open RF Audio:%d!\n",RFUnit);
    	        marsDMAOpen(&guiIISPlayDMAId, rfiuAudioPlayDMA_ISR);
    	    }
    	    else
    	        DEBUG_RFIU_P2("Error! Audio Play DMA is occurpied!\n");
        }
        #endif
	  #endif
      
      #if (RFIU_RX_AUDIO_RETURN)
        if(guiIISRecDMAId==0xff)
            marsDMAOpen(&guiIISRecDMAId, rfiuAudioRet_RecDMA_ISR);
        else
            DEBUG_RFIU_P2("Error! Audio Rec DMA is occurpied!\n");

        for(i=0;i<RFI_AUDIO_RET_BUF_NUM;i++)
           rfiuAudioRetDMANextBuf[i] = rfiuOperBuf[sysRFRxInMainCHsel] + RFI_AUDIORETURN1_ADDR_OFFSET*128 + i*1024;

        rfiuAudioRetRec_idx=0;
        rfiuAudioRetRead_idx=0;
        #if (Audio_mode == AUDIO_AUTO)
        iisSetNextRecDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
        #else
        iisSetNextRecDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
        #endif
        iisStartRec();
      #endif
	  
	  #if TV_DISP_BY_IDU
	    if(sysTVOutOnFlag) 
	    {
	    #if(TV_DISP_BY_TV_INTR)
	        tvTVE_INTC = TV_INTC_BOTFDSTART_ENA;
	    #else
	        tvTVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control * 
	    #endif
	    }
		else
	    {
	        IduIntCtrl |= IDU_FTCINT_ENA;
		}
	  #endif
    }
	else
    {
        rfiuRxSub1VideoPlayStart=0;
		
	    for(i=0;i<DISPLAY_BUF_NUM;i++)
	      rfiuSub1VideoPresentTime[i]=0;
	    rfiuSub1VideoTime=0; 
	    rfiuSub1VideoTime_frac=0;
		
	    if(sysTVOutOnFlag) 
	    {
	    #if(TV_DISP_BY_TV_INTR)
	        tv2TVE_INTC = TV_INTC_BOTFDSTART_ENA;
	    #else
	        tv2TVE_INTC  = TV_INTC_FRMEND__ENA;    //TV interrupt control * 
	    #endif
	    }
		
	}
    
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
	rfiuMpeg4EncodeVolHeader(VolHeader, 
                             &VolHeaderSize,
                             gRfiuUnitCntl[RFUnit].TX_PicWidth,
                             gRfiuUnitCntl[RFUnit].TX_PicHeight,
                             gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA_MPEG_Q);    
	mpeg4DecodeVolHeader(&mpeg4Dec,VolHeader, 0x15); 
#elif(VIDEO_CODEC_OPTION == H264_CODEC)
    rfiuH264Encode_I_Header(&H264Enc,
                            gRfiuUnitCntl[RFUnit].TX_PicWidth,
                            gRfiuUnitCntl[RFUnit].TX_PicHeight);
    rfiuH264Decode_I_Header(&H264Enc,&H264Dec); 

#endif
	
#if RFIU_RX_AVSYNC
    OSTimeDly(1); // 50 ms for buffering
#endif

    timerCountRead(guiRFTimerID, &t1);
    FrameRate=0;

    DEBUG_RFIU_P2("\n============rfiu_RxMpeg4DecTask_UnitX(%d)==========\n",RFUnit);
    while (1)
    {   
        if(gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop)
        {
           DEBUG_RFIU_P2("#");
           OSTimeDly(1);
           continue;
        }
        if(MpegErrCnt>5)
        {
            DEBUG_RFIU_P2("\n======== RFMpegDec-%d is Fatal Error! Reboot!======\n",RFUnit);
            sysForceWDTtoReboot();
        }
    
        RunVideo=0;
        RunAudio=0;
		
#if RFIU_RX_AUDIO_ON        
        //-----Audio------//
        //support PCM only
        if(sysRFRxInMainCHsel==RFUnit)
        {
            AudioBufOffset = rfiuCalBufRemainCount(rfiuRxIIsSounBufMngWriteIdx[RFUnit],rfiuRxIIsSounBufMngReadIdx[RFUnit],IIS_BUF_NUM);
            AudioPlayOffset= rfiuCalAudioplayBufCount(rfiuAudioBufFill_idx[RFUnit], rfiuAudioBufPlay_idx[RFUnit],IISPLAY_BUF_NUM);
	        if( (AudioBufOffset>0) && ((SYNC==1) || (AudioPlayOffset>=0)) )
	        {
	            pAudioBuf        = rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngReadIdx[RFUnit]].buffer;  
	            pAudioFlag       = &(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngReadIdx[RFUnit]].flag);    
	            AudioDuration    = (u32)(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngReadIdx[RFUnit]].time); 
	            pAudioSize       = &(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngReadIdx[RFUnit]].size); 
                    
                if(AudioDuration >2000) //Lucian: 保護斷線時間 error.
                   AudioDuration=128;

	            if(SYNC==1)
	            {
	                if( (SyncTime>=AudioDuration) && (AudioBufOffset>0))
	                {
	                   rfiuRxIIsSounBufMngReadIdx[RFUnit]  = (rfiuRxIIsSounBufMngReadIdx[RFUnit] + 1) % IIS_BUF_NUM;
	                   SyncTime -= AudioDuration;
	                }
	                RunAudio=0;
					//DEBUG_RFIU_P2("SyncTime=%d,%d\n",SyncTime,AudioDuration);
	            }
	            else
	            {
			    #if 1 
                    if(AudioBufOffset > 5)	
                        AdjustIISFreq(-3);
                    else if(AudioBufOffset > 3)	
                        AdjustIISFreq(-3);
                    else if(AudioBufOffset > 1)	
                        AdjustIISFreq(-2);
                    //-----------------------------------//
					else if(AudioPlayOffset > 9)
	                    AdjustIISFreq(-2);
					else if(AudioPlayOffset > 8)
						AdjustIISFreq(-1);
					else if( AudioPlayOffset > 7 )
					    AdjustIISFreq(-1);	
                    else if(AudioPlayOffset < 2)
                        AdjustIISFreq(1);
                    else if(AudioPlayOffset < 3)
                        AdjustIISFreq(1);
                    else if(AudioPlayOffset < 4)
                        AdjustIISFreq(1);
                    else if(AudioPlayOffset < 5)
                        AdjustIISFreq(1);
					else
						AdjustIISFreq(0); // 5,6,7
				 #else
                    if(AudioBufOffset > 5)	
                        AdjustIISFreq(-4);
                    else if(AudioBufOffset > 3)	
                        AdjustIISFreq(-4);
                    else if(AudioBufOffset > 1)	
                        AdjustIISFreq(-4);
                    //-----------------------------------//
					else if(AudioPlayOffset > 9)
	                    AdjustIISFreq(-3);
					else if(AudioPlayOffset > 8)
						AdjustIISFreq(-2);
					else if( AudioPlayOffset > 7 )
					    AdjustIISFreq(-1);	
                    else if(AudioPlayOffset < 2)
                        AdjustIISFreq(4);
                    else if(AudioPlayOffset < 3)
                        AdjustIISFreq(3);
                    else if(AudioPlayOffset < 4)
                        AdjustIISFreq(2);
                    else if(AudioPlayOffset < 5)
                        AdjustIISFreq(1);
					else
						AdjustIISFreq(0); // 5,6,7                    
				 #endif

	            #if RFIU_RX_AVSYNC
	                for(i=0;i<IIS_CHUNK_SIZE/IIS_PLAYBACK_SIZE;i++)
	                {
	                    rfiuMainAudioPlayDMANextBuf[rfiuAudioBufFill_idx[RFUnit] % IISPLAY_BUF_NUM]=pAudioBuf;
	                    rfiuMainAudioPresentTime[rfiuAudioBufFill_idx[RFUnit] % IISPLAY_BUF_NUM]=rfiuAudioTimeBase;
	                    rfiuAudioBufFill_idx[RFUnit] ++;
	                    pAudioBuf += IIS_PLAYBACK_SIZE;
	                    rfiuAudioTimeBase += (AudioDuration /(IIS_CHUNK_SIZE/IIS_PLAYBACK_SIZE));
	                    
	                }
	            #else
	                rfiuMainAudioPlayDMANextBuf[rfiuAudioBufFill_idx[RFUnit] % IISPLAY_BUF_NUM]=pAudioBuf;
	                rfiuAudioBufPlay_idx[RFUnit] = rfiuAudioBufFill_idx[RFUnit];
	                rfiuAudioBufFill_idx[RFUnit] ++;
	            #endif

                    OS_ENTER_CRITICAL();
	                if( (rfiuAudioBufFill_idx[RFUnit] > 65536 + IISPLAY_BUF_NUM) && (rfiuAudioBufPlay_idx[RFUnit] > 65536 + IISPLAY_BUF_NUM) )
	                {
	                   rfiuAudioBufFill_idx[RFUnit] -= 65536;
	                   rfiuAudioBufPlay_idx[RFUnit] -=65536;
	                }
	                
				#if(RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO)
	                if( (rfiuAudioTimeBase > 0x1ffff) && (rfiuVideoTimeBase[RFUnit] > 0x1ffff) && (rfiuMainAudioTime > 0x1ffff))
	                {	                   
					   if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
                       {
                          rfiuMainAudioTime -=0x0ffff;
	                      rfiuAudioTimeBase -=0x0ffff;
						  for(i=0;i<MAX_RFIU_UNIT;i++)
						    rfiuVideoTimeBase[i]=rfiuMainAudioTime;
					   }
					   else
					   {
					     rfiuMainAudioTime -=0x0ffff;
	                     rfiuAudioTimeBase -=0x0ffff;
					     rfiuVideoTimeBase[RFUnit] -=0x0ffff;
					   
                         for(i=0;i<IISPLAY_BUF_NUM;i++)
	                      rfiuMainAudioPresentTime[i] -=0x0ffff;
	                     for(i=0;i<DISPLAY_BUF_NUM;i++)
	                      rfiuMainVideoPresentTime[i] -=0x0ffff;
					   }
	                   
	                }
				#elif(RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO)
	                if( (rfiuAudioTimeBase > 0x1ffff) && (rfiuMainAudioTime > 0x1ffff))
	                {
	                   rfiuMainAudioTime -=0x0ffff;
	                   rfiuAudioTimeBase -=0x0ffff;
	                   for(i=0;i<IISPLAY_BUF_NUM;i++)
	                      rfiuMainAudioPresentTime[i] -=0x0ffff;
	                }
			    #endif
				
	                OS_EXIT_CRITICAL();
                
                    if(AudioBufOffset > 8)
	                   rfiuRxIIsSounBufMngReadIdx[RFUnit] = (rfiuRxIIsSounBufMngReadIdx[RFUnit]+4) % IIS_BUF_NUM;
                    else if(AudioBufOffset > 4)
	                   rfiuRxIIsSounBufMngReadIdx[RFUnit] = (rfiuRxIIsSounBufMngReadIdx[RFUnit]+2) % IIS_BUF_NUM;
                    else
	                rfiuRxIIsSounBufMngReadIdx[RFUnit] = (rfiuRxIIsSounBufMngReadIdx[RFUnit]+1) % IIS_BUF_NUM;
                    
	            #if RFIU_RX_AVSYNC
	                if(First_Audio)
	                {
	                   rfiuRxMainAudioPlayStart=1;
                       if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) || (sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR) )
                       {
                        #if (Audio_mode == AUDIO_AUTO)
	                      iisSetNextPlayDMA_auto((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                        #else
	                      iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                        #endif
	                      iisStartPlay();
                          DEBUG_RFIU_P2("--->RF Audio Start:%d!\n",RFUnit);
                       }
                    #if RFRX_QUAD_AUDIO_EN
                       else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
                       {
                        #if (Audio_mode == AUDIO_AUTO)
	                      iisSetNextPlayDMA_auto((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                        #else
                          iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                        #endif
	                      iisStartPlay();
                          DEBUG_RFIU_P2("--->RF Audio Start:%d!\n",RFUnit);
                       }
                    #endif
	                   First_Audio=0;
	                }
	            #endif
	                RunAudio=1;
	            }
	            
	            //DEBUG_RFIU_P2("A>%d,%2d:%d\n",AudioPlayOffset,AudioBufOffset,AudioDuration);
	        }
        }
		else
	    {
		}
#endif
        //-----Video-----//
        VideoBufOffset = rfiuCalBufRemainCount(rfiuRxVideoBufMngWriteIdx[RFUnit],VideoReadIdx,VIDEO_BUF_NUM);
        VideoDispOffset= rfiuCalVideoDisplayBufCount(rfiuVideoBufFill_idx[RFUnit], rfiuVideoBufPlay_idx[RFUnit],DISPLAY_BUF_NUM);
     
        if( (VideoBufOffset>0) && ((SYNC==1) || (VideoDispOffset>=0)) )
        {
            pVideoBuf        = rfiuRxVideoBufMng[RFUnit][VideoReadIdx].buffer; 
            pVideoFlag       = &(rfiuRxVideoBufMng[RFUnit][VideoReadIdx].flag);    
            VideoDuration    = (u32)(rfiuRxVideoBufMng[RFUnit][VideoReadIdx].time); 
            pVideoSize       = &(rfiuRxVideoBufMng[RFUnit][VideoReadIdx].size);
            VideoOffset      = rfiuRxVideoBufMng[RFUnit][VideoReadIdx].offset;
            vop_type         = (*pVideoFlag)? 0 : 1;

            if(VideoDuration >2000) //Lucian: 保護斷線時間 error.
                VideoDuration=66;

            if( (SYNC==1) && (*pVideoFlag == 0))
            {
                VideoReadIdx  = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                SyncTime += VideoDuration;
				RunVideo=1;
                //DEBUG_RFIU_P2("SyncTime=%d\n",SyncTime);
            }
            else
            {
                if(SYNC==1)
                {
                #if(SW_APPLICATION_OPTION == MR8100_DUALMODE_VBM)
                   if(sysEnSnapshot == 0)
                   {
                      osdDrawPlaybackArea(2);
                      OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(RFUnit*8), OS_FLAG_SET, &err);
                   }
                #else        
                   osdDrawPlaybackArea(2);
                   OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(RFUnit*8), OS_FLAG_SET, &err);
                #endif
                   VideoDuration=33;
                }
                SYNC = 0;
                //========== Single Display mode==========//
				if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
				{
				   if(VideoDispOffset >= 1 )
                      rfiuRxMainVideoPlayStart=1;
                #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
                   Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_MAIN,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                #elif (VIDEO_CODEC_OPTION == H264_CODEC)    
                    Vop_Result = rfiuH264Decode(&video_info,
                                                    &H264Dec,
                                                pVideoBuf, 
                                                *pVideoSize, 
                                                RFUnit,
                                                VideoOffset,
                                                RFIU_RX_DISP_MAIN,
                                                (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                #endif
                   FrameCnt ++;

				   if(Vop_Result == MPEG4_N_VOP) 
				   {
                      OSSemPost(VideoCmpSemEvt);
                      continue;
                   }
                   if(Vop_Result == MPEG4_ERROR)
                   {
                       MpegErrCnt ++;
                   }
                   else
                       MpegErrCnt=0;

				   //==調整Video 播放速度,減低delay==//
                #if (RFIU_RX_AUDIO_ON && (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO))
			       rfiuVideoTimeBase[RFUnit] += VideoDuration;
				#else
                    rfiuVideoTimeBase[RFUnit] += (VideoDuration);
				#endif

				   OS_ENTER_CRITICAL();
	            #if RFIU_RX_AVSYNC
	               rfiuMainVideoPresentTime[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM]=rfiuVideoTimeBase[RFUnit];
	            #else
	               rfiuVideoBufPlay_idx[RFUnit]=rfiuVideoBufFill_idx[RFUnit];
	            #endif

                   VideoLatency = rfiuMainVideoPresentTime[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] - rfiuMainVideoPresentTime[rfiuVideoBufPlay_idx[RFUnit] % DISPLAY_BUF_NUM];  
                   for(i=1;i<=VideoBufOffset;i++)
                   {
                      VideoLatency += (u32)(rfiuRxVideoBufMng[RFUnit][(VideoReadIdx+i) % VIDEO_BUF_NUM].time); 
                   }
                   if(VideoLatency<0)
                       VideoLatency=0;
                 
	               rfiuVideoBufFill_idx[RFUnit]++; 
                   VideoReadIdx  = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                   //==Refine Video speed==//
                   if(VideoLatency > 1000)
                       rfiuMainVideoTime +=500;
                   else if((VideoLatency > 500) )
                      rfiuMainVideoTime +=100;
                #if RF_RX_SMOOTH_PLUS
                   else if( (VideoLatency > 350) && (VideoDispOffset>1) && (VideoCount>8) )
                   {
                      rfiuMainVideoTime +=16;
                      VideoCount=0;
                   }
                   else if( (VideoLatency > 300) && (VideoDispOffset>1) && (VideoCount>8) )
                   {
                      rfiuMainVideoTime +=8;
                      VideoCount=0;
                   }
                #else
                   else if( (VideoLatency > 200) && (VideoDispOffset>1) && (VideoCount>4) )
                   {
                      rfiuMainVideoTime +=33;
                      VideoCount=0;
                   }
                   else if( (VideoLatency > 150) && (VideoDispOffset>1) && (VideoCount>4) )
                   {
                      rfiuMainVideoTime +=16;
                      VideoCount=0;
                   }  
                #endif
				   
                       
                   //======================//               
                   
	               if( (rfiuVideoBufFill_idx[RFUnit] > 65536+DISPLAY_BUF_NUM) && (rfiuVideoBufPlay_idx[RFUnit] > 65536+DISPLAY_BUF_NUM) )
	               {
	                  rfiuVideoBufFill_idx[RFUnit] -= 65532;
	                  rfiuVideoBufPlay_idx[RFUnit] -=65532;
	               }
					
	            #if( (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO) || (RFIU_RX_AUDIO_ON==0) )
	               if( (rfiuVideoTimeBase[RFUnit] > 0x1ffff) && (rfiuMainVideoTime > 0x1ffff))
	               {
	                  rfiuMainVideoTime -=0x0ffff;
	                  rfiuVideoTimeBase[RFUnit] -=0x0ffff;
	                  for(i=0;i<DISPLAY_BUF_NUM;i++)
	                     rfiuMainVideoPresentTime[i] -=0x0ffff;
	               }
				#endif
	               OS_EXIT_CRITICAL();
				#if  RF_LETENCY_DEBUG_ENA
                   if(sysRFRxInMainCHsel==RFUnit)
				      DEBUG_RFIU_P2("V>%d,%2d:%d,%d\n",VideoDispOffset,VideoBufOffset,VideoLatency,VideoDuration);
				#endif
                   VideoCount++;
				   RunVideo=1;
				}

                //========== Quad Display mode==========//
				else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
				{
	               rfiuRxMainVideoPlayStart=1;
				   rfiuVideoBufPlay_idx[RFUnit] = rfiuVideoBufFill_idx[RFUnit];
				#if ( RFIU_RX_AUDIO_ON && (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO) )   
                   if(rfiuMainAudioTime >= rfiuVideoTimeBase[RFUnit])
                   {
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
                      Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                         pVideoBuf, 
                                                         *pVideoSize, 
                                                         RFUnit,
                                                         VideoOffset,
                                                         RFIU_RX_DISP_QUARD,
                                                         (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)    
                        Vop_Result = rfiuH264Decode(&video_info,
                                                        &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_MAIN,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                    #endif
                      VideoReadIdx  = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                      FrameCnt ++;

					  if(Vop_Result == MPEG4_N_VOP) 
					  {
	                     OSSemPost(VideoCmpSemEvt);
	                     continue;
	                  }
                      if(Vop_Result == MPEG4_ERROR)
                      {
                         MpegErrCnt ++;
                      }
                      else
                         MpegErrCnt=0;
            
			          rfiuVideoTimeBase[RFUnit] += VideoDuration;
					  RunVideo=1;
				   }
				   else
				   {
                      RunVideo=0;
				   }
			    #else 
                   if(rfiuMainVideoTime >= rfiuVideoTimeBase[RFUnit])
                   {					  
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
                      Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                        pVideoBuf, 
                                                        *pVideoSize, 
                                                        RFUnit,
                                                        VideoOffset,
                                                        RFIU_RX_DISP_QUARD,
                                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)    
                        Vop_Result = rfiuH264Decode(&video_info,
                                                        &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_MAIN,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                    #endif
                      VideoReadIdx  = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                      FrameCnt ++;

					  if(Vop_Result == MPEG4_N_VOP) 
					  {
	                     OSSemPost(VideoCmpSemEvt);
	                     continue;
	                  }
                      if(Vop_Result == MPEG4_ERROR)
                      {
                           MpegErrCnt ++;
                      }
                      else
                           MpegErrCnt=0;
            
					  if(VideoBufOffset > 15)
					   	rfiuVideoTimeBase[RFUnit] += (VideoDuration-20);
                      else if(VideoBufOffset > 10)
					   	rfiuVideoTimeBase[RFUnit] += (VideoDuration-10);
					  else if(VideoBufOffset > 4)
	                    rfiuVideoTimeBase[RFUnit] += (VideoDuration-3);
					  else if(VideoBufOffset > 1)
	                    rfiuVideoTimeBase[RFUnit] += (VideoDuration-2);
					  else
					   	rfiuVideoTimeBase[RFUnit] += (VideoDuration-0);

                      if( (rfiuMainVideoTime>0x0fffffff) )
                      {
						  rfiuMainVideoTime =0;
						  for(i=0;i<MAX_RFIU_UNIT;i++)
						    rfiuVideoTimeBase[i]=rfiuMainVideoTime;
                      }
					  RunVideo=1;
				   }
				   else
				   {
				      if(VideoBufOffset > 10)
					   	rfiuVideoTimeBase[RFUnit] = rfiuMainVideoTime;
                      RunVideo=0;
				   }				   
				#endif
				}
                else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA)
				{
	               rfiuRxMainVideoPlayStart=1;
				   rfiuVideoBufPlay_idx[RFUnit] = rfiuVideoBufFill_idx[RFUnit];
				
                   if(rfiuMainVideoTime >= rfiuVideoTimeBase[RFUnit])
                   {					  
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
                      Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                        pVideoBuf, 
                                                        *pVideoSize, 
                                                        RFUnit,
                                                        VideoOffset,
                                                        RFIU_RX_DISP_MASK,
                                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)    
                        Vop_Result = rfiuH264Decode(&video_info,
                                                        &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_MAIN,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                    #endif
                      VideoReadIdx  = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                      FrameCnt ++;

					  if(Vop_Result == MPEG4_N_VOP) 
					  {
	                     OSSemPost(VideoCmpSemEvt);
	                     continue;
	                  }
                      if(Vop_Result == MPEG4_ERROR)
                      {
                         MpegErrCnt ++;
                      }
                      else
                         MpegErrCnt=0;
            
					  if(VideoBufOffset > 15)
					   	rfiuVideoTimeBase[RFUnit] += (VideoDuration-20);
                      else if(VideoBufOffset > 10)
					   	rfiuVideoTimeBase[RFUnit] += (VideoDuration-10);
					  else if(VideoBufOffset > 4)
	                    rfiuVideoTimeBase[RFUnit] += (VideoDuration-3);
					  else if(VideoBufOffset > 1)
	                    rfiuVideoTimeBase[RFUnit] += (VideoDuration-2);
					  else
					   	rfiuVideoTimeBase[RFUnit] += (VideoDuration-0);

                      if( (rfiuMainVideoTime>0x0fffffff) )
                      {
						  rfiuMainVideoTime =0;
						  for(i=0;i<MAX_RFIU_UNIT;i++)
						    rfiuVideoTimeBase[i]=rfiuMainVideoTime;
                      }
					  RunVideo=1;
				   }
				   else
				   {
				      if(VideoBufOffset > 10)
					   	rfiuVideoTimeBase[RFUnit] = rfiuMainVideoTime;
                      RunVideo=0;
				   }				   
				}
                //========== Dual TV Display mode==========//
				else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR)
				{  
				   if( 0==RFUnit ) 
				   {
				       //======Main channel=====//
					   if(VideoDispOffset >= 1 )
	                      rfiuRxMainVideoPlayStart=1;
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
                       Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                        pVideoBuf, 
                                                        *pVideoSize, 
                                                        RFUnit,
                                                        VideoOffset,
                                                        RFIU_RX_DISP_MAIN,
                                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)    
                        Vop_Result = rfiuH264Decode(&video_info,
                                                        &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_MAIN,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                    #endif

                       FrameCnt ++;

					   if(Vop_Result == MPEG4_N_VOP) 
					   {
	                      OSSemPost(VideoCmpSemEvt);
	                      continue;
	                   }
                       if(Vop_Result == MPEG4_ERROR)
                       {
                           MpegErrCnt ++;
                       }
                       else
                           MpegErrCnt=0;
                       
	                #if ( RFIU_RX_AUDIO_ON && (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO) )
				       rfiuVideoTimeBase[RFUnit] += VideoDuration;
					#else
                        rfiuVideoTimeBase[RFUnit] += (VideoDuration);
					#endif  
                    
                   	   OS_ENTER_CRITICAL();
		            #if RFIU_RX_AVSYNC
		               rfiuMainVideoPresentTime[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM]=rfiuVideoTimeBase[RFUnit];
		            #else
		               rfiuVideoBufPlay_idx[RFUnit]=rfiuVideoBufFill_idx[RFUnit];
		            #endif

                       VideoLatency = rfiuMainVideoPresentTime[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] - rfiuMainVideoPresentTime[rfiuVideoBufPlay_idx[RFUnit] % DISPLAY_BUF_NUM];
                       for(i=1;i<=VideoBufOffset;i++)
                       {
                          VideoLatency += (u32)(rfiuRxVideoBufMng[RFUnit][(VideoReadIdx+i) % VIDEO_BUF_NUM].time); 
                       }
                       if(VideoLatency<0)
                           VideoLatency=0;
                    
		               rfiuVideoBufFill_idx[RFUnit]++; 
	                   VideoReadIdx  = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                       
                       //==Refine Video speed==//
                       if(VideoLatency > 1000)
                           rfiuMainVideoTime +=500;
                       else if((VideoLatency > 500) )
                          rfiuMainVideoTime +=100;
                    #if RF_RX_SMOOTH_PLUS
                       else if( (VideoLatency > 350) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuMainVideoTime +=16;
                          VideoCount=0;
                       }
                       else if( (VideoLatency > 300) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuMainVideoTime +=8;
                          VideoCount=0;
                       }
                    #else
                       else if( (VideoLatency > 200) && (VideoDispOffset>1) && (VideoCount>4) )
                       {
                          rfiuMainVideoTime +=33;
                          VideoCount=0;
                       }
                       else if( (VideoLatency > 150) && (VideoDispOffset>1) && (VideoCount>4) )
                       {
                          rfiuMainVideoTime +=16;
                          VideoCount=0;
                       }  
                    #endif
                       
                       //======================//
                       
		               if( (rfiuVideoBufFill_idx[RFUnit] > 65536+DISPLAY_BUF_NUM) && (rfiuVideoBufPlay_idx[RFUnit] > 65536+DISPLAY_BUF_NUM) )
		               {
		                  rfiuVideoBufFill_idx[RFUnit] -= 65532;
		                  rfiuVideoBufPlay_idx[RFUnit] -=65532;
		               }
						
		            #if( (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO) || (RFIU_RX_AUDIO_ON==0) )
		               if( (rfiuVideoTimeBase[RFUnit] > 0x1ffff) && (rfiuMainVideoTime > 0x1ffff))
		               {
		                  rfiuMainVideoTime -=0x0ffff;
		                  rfiuVideoTimeBase[RFUnit] -=0x0ffff;
		                  for(i=0;i<DISPLAY_BUF_NUM;i++)
		                     rfiuMainVideoPresentTime[i] -=0x0ffff;
		               }
					#endif

		               OS_EXIT_CRITICAL();
				    #if RF_LETENCY_DEBUG_ENA
					  if( (VideoCount & 0x03) ==0)
				         DEBUG_RFIU_P2("V1>%d,%2d:%d,%d\n",VideoDispOffset,VideoBufOffset,VideoLatency,VideoDuration);
				    #endif
					  VideoCount++;

				   }
				   else
				   {
				       //======Sub channel=====//
					   if(VideoDispOffset >= 1 )
	                      rfiuRxSub1VideoPlayStart=1;
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
	                   Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                        pVideoBuf, 
                                                        *pVideoSize, 
                                                        RFUnit,
                                                        VideoOffset,
                                                        RFIU_RX_DISP_SUB1,
                                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)    
                        Vop_Result = rfiuH264Decode(&video_info,
                                                        &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_MAIN,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                    #endif
                       FrameCnt ++;

					   if(Vop_Result == MPEG4_N_VOP) 
					   {
	                      OSSemPost(VideoCmpSemEvt);
	                      continue;
	                   }
                       if(Vop_Result == MPEG4_ERROR)
                       {
                           MpegErrCnt ++;
                       }
                       else
                           MpegErrCnt=0;
	            
					#if (RFIU_RX_AUDIO_ON && (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO))
				       rfiuVideoTimeBase[RFUnit] += VideoDuration;
					#else      
                       rfiuVideoTimeBase[RFUnit] += (VideoDuration);
					#endif     

					   OS_ENTER_CRITICAL();
		            #if RFIU_RX_AVSYNC
		               rfiuSub1VideoPresentTime[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM]=rfiuVideoTimeBase[RFUnit];
		            #else
		               rfiuVideoBufPlay_idx[RFUnit]=rfiuVideoBufFill_idx[RFUnit];
		            #endif

                       VideoLatency = rfiuSub1VideoPresentTime[rfiuVideoBufFill_idx[RFUnit] % DISPLAY_BUF_NUM] - rfiuSub1VideoPresentTime[rfiuVideoBufPlay_idx[RFUnit] % DISPLAY_BUF_NUM];
                       for(i=1;i<=VideoBufOffset;i++)
                       {
                          VideoLatency += (u32)(rfiuRxVideoBufMng[RFUnit][(VideoReadIdx+i) % VIDEO_BUF_NUM].time); 
                       }

                       if(VideoLatency<0)
                           VideoLatency=0;
                       
		               rfiuVideoBufFill_idx[RFUnit]++; 
	                   VideoReadIdx  = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                       
                       //==Refine Video speed==//
                       if(VideoLatency > 1000)
                           rfiuSub1VideoTime +=500;
                       else if((VideoLatency > 500) )
                          rfiuSub1VideoTime +=100;
                    #if RF_RX_SMOOTH_PLUS
                       else if( (VideoLatency > 350) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuSub1VideoTime +=16;
                          VideoCount=0;
                       }
                       else if( (VideoLatency > 300) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuSub1VideoTime +=8;
                          VideoCount=0;
                       }
                    #else
                       else if( (VideoLatency > 200) && (VideoDispOffset>1) && (VideoCount>4) )
                       {
                          rfiuSub1VideoTime +=33;
                          VideoCount=0;
                       }
                       else if( (VideoLatency > 150) && (VideoDispOffset>1) && (VideoCount>4) )
                       {
                          rfiuSub1VideoTime +=16;
                          VideoCount=0;
                       }  
                    #endif
					   
                       //======================//
                       
		               if( (rfiuVideoBufFill_idx[RFUnit] > 65536+DISPLAY_BUF_NUM) && (rfiuVideoBufPlay_idx[RFUnit] > 65536+DISPLAY_BUF_NUM) )
		               {
		                  rfiuVideoBufFill_idx[RFUnit] -= 65532;
		                  rfiuVideoBufPlay_idx[RFUnit] -=65532;
		               }
						
		            #if( (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_VIDEO) || (RFIU_RX_AUDIO_ON==0) )
		               if( (rfiuVideoTimeBase[RFUnit] > 0x1ffff) && (rfiuSub1VideoTime > 0x1ffff))
		               {
		                  rfiuSub1VideoTime -=0x0ffff;
		                  rfiuVideoTimeBase[RFUnit] -=0x0ffff;
		                  for(i=0;i<DISPLAY_BUF_NUM;i++)
		                     rfiuSub1VideoPresentTime[i] -=0x0ffff;
		               }
					#endif
		               OS_EXIT_CRITICAL();
					
					#if RF_LETENCY_DEBUG_ENA
					   if( (VideoCount & 0x03) ==0)
				         DEBUG_RFIU_P2("V2>%d,%2d:%d,%d\n",VideoDispOffset,VideoBufOffset,VideoLatency,VideoDuration);
				    #endif
					   VideoCount++;
				   }
				   RunVideo=1;

                   
				}

				//====== Cal Frame Rate======//
                if(FrameCnt>30)
                {
                   timerCountRead(guiRFTimerID, &t2);
                   if(t1>t2)
                      dt=t1-t2;
                   else
                      dt=(t1+TimerGetTimerCounter(TIMER_7))-t2;

                   if(dt==0)
                      dt=1;

                   FrameRate = FrameCnt*10000/dt;
                   if(FrameRate>30)
                      FrameRate=30;

                   gRfiuUnitCntl[RFUnit].FrameRate=FrameRate;
                #if UI_RX_OSDDRAW_FRAMERATE   
                   DEBUG_RFIU_P2("RF-%d FrameRate=%d\n",RFUnit,FrameRate);
                   sysbackSetEvt(SYS_BACK_DRAW_FRAME_RATE, RFUnit);
                #endif
                   t1=t2;
                   FrameCnt=0;
                }									
            }

         }
		 else
		 {
            if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR) || (sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA) )
            {
            #if ( RFIU_RX_AUDIO_ON && (RFIU_RX_TIME_SEL == RFIU_RX_TIME_BY_AUDIO) )
			   rfiuVideoTimeBase[RFUnit]=rfiuMainAudioTime;
			   rfiuMainVideoTime=rfiuMainAudioTime;
			#else
               rfiuVideoTimeBase[RFUnit]=rfiuMainVideoTime;
			#endif
			}
		 }

         if( (RunAudio==0) && (RunVideo==0))
         {
            OSTimeDly(1);
         }
		 else
		 {
		    DEBUG_RFIU_P("%d ",RFUnit);
		 }
		 
      }
}

#if RF_RX_AUDIO_SELFTEST
void rfiuAudioPlayDMA_ISR(int dummy)
{
   iisSetNextPlayDMA(rfiuAudioRetDMANextBuf[(rfiuAudioRetRec_idx-2) % RFI_AUDIO_RET_BUF_NUM], 1024);
   iisStartPlay();
   rfiuMainAudioTime += 64;
   rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] +=1;
}
#else
void rfiuAudioPlayDMA_ISR(int dummy)
{
    int AudioPlayOffset;
    
    if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) || (sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR) )
    {  
       if(rfiuRxMainAudioPlayStart)
       {
          //DEBUG_RFIU_P2("A");
          AudioPlayOffset= (rfiuAudioBufFill_idx[sysRFRxInMainCHsel] >= rfiuAudioBufPlay_idx[sysRFRxInMainCHsel]) ?  (rfiuAudioBufFill_idx[sysRFRxInMainCHsel] -rfiuAudioBufPlay_idx[sysRFRxInMainCHsel]) : (rfiuAudioBufFill_idx[sysRFRxInMainCHsel] +IISPLAY_BUF_NUM - rfiuAudioBufPlay_idx[sysRFRxInMainCHsel]);

		  if(AudioPlayOffset > 2)
		  {
             if( (rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM] + 32*6)  )
             {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISPlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] += 2;
				 rfiuMainAudioTime += 32;
                 //DEBUG_RFIU_P2("J");

		     }
			 else if(rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM])
			 {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISPlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
			 	 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] +=1;
				 rfiuMainAudioTime += 32;
			 }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISPlayDMAId,rfiuAudioZeroBuf);
                #else
                 iisSetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
                #endif
				 rfiuMainAudioTime += 32;
                //DEBUG_RFIU_P2("Z");
			 }
		  }
		  else if(AudioPlayOffset > 1)
          {
             if(rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM])
             {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISPlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] ++;
				 rfiuMainAudioTime += 32;
             }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISPlayDMAId,rfiuAudioZeroBuf);
                #else
                 iisSetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
                #endif
				 rfiuMainAudioTime += 32;
                //DEBUG_RFIU_P2("Z");
			 }
          }
          else
          {
             rfiuMainAudioTime = rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM];
             rfiuMainAudioTime_frac=0;
            #if (Audio_mode == AUDIO_AUTO)
			 isr_marsDMA_PlayAuto(guiIISPlayDMAId,rfiuAudioZeroBuf);
            #else
			 iisSetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
            #endif
             //DEBUG_RFIU_P2("Z");
          }
          #if(Audio_mode != AUDIO_AUTO)
          iisStartPlay();
          #endif
       }
    }
    else if( sysCameraMode==SYS_CAMERA_MODE_RF_RX_QUADSCR )
    {
       if(rfiuRxMainAudioPlayStart)
       {
          AudioPlayOffset= (rfiuAudioBufFill_idx[sysRFRxInMainCHsel] >= rfiuAudioBufPlay_idx[sysRFRxInMainCHsel]) ?  (rfiuAudioBufFill_idx[sysRFRxInMainCHsel] -rfiuAudioBufPlay_idx[sysRFRxInMainCHsel]) : (rfiuAudioBufFill_idx[sysRFRxInMainCHsel] +IISPLAY_BUF_NUM - rfiuAudioBufPlay_idx[sysRFRxInMainCHsel]);

		  if(AudioPlayOffset > 2)
		  {
		     if( (rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM] + 32*2) )
		     {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISPlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] += 2;
				 rfiuMainAudioTime += 32;
		     }
			 else if(rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM])
			 {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISPlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
			 	 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] +=1;
				 rfiuMainAudioTime += 32;
			 }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISPlayDMAId,rfiuAudioZeroBuf);
                #else
                 iisSetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
                #endif
				 rfiuMainAudioTime += 32;
			 }
		  }
		  else if(AudioPlayOffset > 1)
          {
             if(rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM])
             {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISPlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] ++;
				 rfiuMainAudioTime += 32;
             }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISPlayDMAId,rfiuAudioZeroBuf);
                #else
                 iisSetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
                #endif
				 rfiuMainAudioTime += 32;
			 }
          }
          else
          {
             rfiuMainAudioTime = rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM];
             rfiuMainAudioTime_frac=0;
            #if (Audio_mode == AUDIO_AUTO)
			 isr_marsDMA_PlayAuto(guiIISPlayDMAId,rfiuAudioZeroBuf);
            #else
			 iisSetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
            #endif
          }
          #if(Audio_mode != AUDIO_AUTO)
          iisStartPlay();
          #endif
      }
    }
}

#endif

void rfiuAudioRet_RecDMA_ISR(int dummy)
{   

    if(rfiuAudioRetFromApp)
    {
    #if (Audio_mode == AUDIO_AUTO)
       iisSetNextRecDMA_auto((u8*)rfiuAudioDummyBuf, 1024);
       iisStartRec();
       //isr_marsDMA_RecAuto(guiIISRecDMAId,(u8*)rfiuAudioDummyBuf);
    #else
       iisSetNextRecDMA((u8*)rfiuAudioDummyBuf, 1024);
       iisStartRec();
    #endif
    }
    else
    {
        rfiuAudioRetRec_idx = (rfiuAudioRetRec_idx+1) % RFI_AUDIO_RET_BUF_NUM;
    #if (Audio_mode == AUDIO_AUTO)
        iisSetNextRecDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx], 1024);
        iisStartRec();
        //isr_marsDMA_RecAuto(guiIISRecDMAId,(u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx]);
    #else
        iisSetNextRecDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx], 1024);
    	iisStartRec();
    #endif
        
    #if RFIU_RX_AUDIO_RETURN    
        if((rfiuAudioRetRec_idx & 0x07)==0)
        {
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].PktCount=64;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].RetryCount=0;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].WriteDiv=8;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].ReadDiv =0;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].PktMap0 =0xffffffff;
            gRfiuUnitCntl[sysRFRxInMainCHsel].TxPktMap[RFI_AUDIORETURN1_ADDR_OFFSET/RFI_GRP_INPKTUNIT + (rfiuAudioRetRec_idx>>3)].PktMap1 =0xffffffff;
        }
    #endif
    }
}


#endif
