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
#if USB_DONGLE_SUPPORT
#include "usbapi.h"
#endif

#if (HDMI_TXIC_SEL == HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
#include "uiapi.h"
#endif

#if PWIFI_SUPPORT
#include "pwifiapi.h"
#endif
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

#define RFTX_AUDIO_TIMESHFT   4

/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
OS_STK rfiuWrapTaskStack_Unit0[RFIU_WRAP_TASK_STACK_SIZE_UNIT0]; 
OS_STK rfiuWrapTaskStack_Unit1[RFIU_WRAP_TASK_STACK_SIZE_UNIT1]; 
OS_STK rfiuWrapTaskStack_Unit2[RFIU_WRAP_TASK_STACK_SIZE_UNIT2]; 
OS_STK rfiuWrapTaskStack_Unit3[RFIU_WRAP_TASK_STACK_SIZE_UNIT3]; 
#if (RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL) 
OS_STK rfiuWrapTaskStack_Unit4[RFIU_WRAP_TASK_STACK_SIZE_UNIT4]; 
OS_STK rfiuWrapTaskStack_Unit5[RFIU_WRAP_TASK_STACK_SIZE_UNIT5]; 
OS_STK rfiuWrapTaskStack_Unit6[RFIU_WRAP_TASK_STACK_SIZE_UNIT6]; 
OS_STK rfiuWrapTaskStack_Unit7[RFIU_WRAP_TASK_STACK_SIZE_UNIT7]; 
#endif


OS_STK rfiuDecTaskStack_Unit0[RFIU_DEC_TASK_STACK_SIZE_UNIT0]; 
OS_STK rfiuDecTaskStack_Unit1[RFIU_DEC_TASK_STACK_SIZE_UNIT1]; 
OS_STK rfiuDecTaskStack_Unit2[RFIU_DEC_TASK_STACK_SIZE_UNIT2]; 
OS_STK rfiuDecTaskStack_Unit3[RFIU_DEC_TASK_STACK_SIZE_UNIT3]; 
#if (RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL) 
OS_STK rfiuDecTaskStack_Unit4[RFIU_DEC_TASK_STACK_SIZE_UNIT4]; 
OS_STK rfiuDecTaskStack_Unit5[RFIU_DEC_TASK_STACK_SIZE_UNIT5]; 
OS_STK rfiuDecTaskStack_Unit6[RFIU_DEC_TASK_STACK_SIZE_UNIT6]; 
OS_STK rfiuDecTaskStack_Unit7[RFIU_DEC_TASK_STACK_SIZE_UNIT7]; 
#endif


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
u8 Time_AudioPlayOffset;

#if ASF_DEBUG_ENA
extern u32 RX_time_A, RX_time_V;
extern u32 RX_skip_A, RX_skip_V;
extern u32 RX_sem_A, RX_sem_V;
#endif


#if RFIU_RX_WAKEUP_TX_SCHEME
u32 rfiuBatCamVideo_TotalTime[MAX_RFIU_UNIT];
u32 rfiuBatCam_LiveMaxTime[MAX_RFIU_UNIT];
u32 rfiuBatCam_PIRRecDurationTime=RF_BATCAM_REC_INTERVAL;
#endif

#if TX_PIRREC_SUPPORT 
u32 rfiuStopPIRRecReady=0;
#endif

#if TX_PIRREC_VMDCHK
u32 rfiuPIRRec_VMDTrig=0;
#endif
/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
#if TX_SNAPSHOT_SUPPORT
extern u32 sysRFTXSnapImgRdy;
#endif

#if TX_PIRREC_VMDCHK
extern int rfiuBatCamDcDetect;
extern u8 MotionDetect_en;
#endif

extern int rfiuBatCamPIRTrig;
extern u32 guiSysTimerCnt;
extern u8 TvOutMode;
extern u32 iduQuadDispCnt;
extern u8 EnableStreaming;
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[];
extern INT32U  guiIISCh0RecDMAId, guiIISCh1RecDMAId, guiIISCh2RecDMAId, guiIISCh3RecDMAId;
extern INT32U  guiIISCh0PlayDMAId, guiIISCh1PlayDMAId, guiIISCh2PlayDMAId, guiIISCh3PlayDMAId,guiIISCh4PlayDMAId;


extern OS_EVENT *gRfiuAVCmpSemEvt[MAX_RFIU_UNIT];
extern u32 guiRFTimerID;


extern void mcpu_ByteMemcpy(u8 *DstAddr, u8 *SrcAddr, unsigned int ByteCnt);
extern unsigned int rfiuCalBufRemainCount(unsigned int WritePtr,unsigned int ReadPtr,unsigned int BufSize);
extern void AdjustIISFreq(s32 level);

extern u32 P2PVideoBufReadIdx[];
extern u32 P2PAudioBufReadIdx[];

extern u32 USBVideoBufReadIdx[];
extern u32 USBAudioBufReadIdx[];
#if (VIDEO_CODEC_OPTION == H264_CODEC)
extern u32 H264_IFlag_Index[MAX_RFIU_UNIT];
#endif

#if VIDEO_STARTCODE_DEBUG_ENA
extern int monitor_RX[MAX_RFIU_UNIT];
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

#if PWIFI_SUPPORT
int  rfiu_AudioRetONOFF_IIS(int OnOff)
{
    int i;

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

        for(i=0; i<(PWIFI_AUDRET_PKTMAP_SIZE/32); i++)  
           gRfiuUnitCntl[sysRFRxInMainCHsel].PwifiAudioRetPktMap[i] = 0xffffffff;

        for(i=0; i<PWIFI_AUDRET_PKTMAP_SIZE ;i++)
           gRfiuUnitCntl[sysRFRxInMainCHsel].AudioRetPkt_RetryCnt[i]=0;

        rfiuAudioRetFromApp=0;
        rfiuAudioRetRec_idx=0;
        rfiuAudioRetRead_idx=0;
        gRfiuUnitCntl[sysRFRxInMainCHsel].RXCmd_AudioRetEn=1;
		MicVol_FadeIn();
      #if 1
        if(guiIISCh0RecDMAId==0xff)
        {
            marsDMAOpen(&guiIISCh0RecDMAId, rfiuAudioRet_RecDMA_ISR);
        #if (Audio_mode == AUDIO_AUTO)
            iisSetNextRecDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx % RFI_AUDIO_RET_BUF_NUM], 1024,sysRFRxInMainCHsel);
        #else
            iisSetNextRecDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
        #endif        
            iisStartRec();
        }    
        else
            DEBUG_RFIU_P2("Error! Audio Rec DMA is occurpied!\n");
      #endif    
        
		
    }
    else
    {   
		MicVol_FadeOut();
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

    DEBUG_PWIFI("==rfiu_AudioRetONOFF_IIS:%d,%d==\n",OnOff,sysRFRxInMainCHsel);

    return 1;
}


int rfiu_AudioRetONOFF_APP(int OnOff,int RFUnit)
{
    int i;
    
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
      
        for(i=0; i<(PWIFI_AUDRET_PKTMAP_SIZE/32); i++)  
           gRfiuUnitCntl[sysRFRxInMainCHsel].PwifiAudioRetPktMap[i] = 0xffffffff;

        for(i=0; i<PWIFI_AUDRET_PKTMAP_SIZE ;i++)
           gRfiuUnitCntl[sysRFRxInMainCHsel].AudioRetPkt_RetryCnt[i]=0;
        
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


#else
int  rfiu_AudioRetONOFF_IIS(int OnOff)
{
    int i,FillIdx, delayCnt = 0;
    static u32 lastOffTick = 0;

    if(OnOff)
    {   //Turn on audio return function.
        if(rfiuAudioRetStatus !=RF_AUDIO_RET_OFF)
        {    
            DEBUG_RFIU_P2("Audio-return is occurpied by APP!!\n");
            return 0;
        }

        while((OSTimeGet() <= lastOffTick + 26)&& delayCnt < 26)
        { //after Talk off, need to wait 1.5sec for the Next Talk On
            DEBUG_RFIU_P2("Delay Cur:%d, Last:%d\n", OSTimeGet(), lastOffTick);
            delayCnt++;
            OSTimeDly(1);
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
		MicVol_FadeIn();
		
    }
    else
    {   
		MicVol_FadeOut();
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
        lastOffTick = OSTimeGet();
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

#endif

#if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
    (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) ||\
    (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) ||\
    (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == DVP_RF_SELFTEST) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD) )
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
    //---//
#if TX_PIRREC_VMDCHK
    unsigned int VMDChk_T5=0;
    unsigned int VMDChk_T6=0;
#endif    
    //-------------------//

    
    RFUnit= (int)pData;
#if TX_PIRREC_SUPPORT 
    if(rfiuBatCamPIRTrig)
    {
        VideoReadIdx = 0;
        AudioReadIdx = 0;
    }
    else
    {
        VideoReadIdx = (VideoBufMngWriteIdx) % VIDEO_BUF_NUM;
        AudioReadIdx = (iisSounBufMngWriteIdx) % IIS_BUF_NUM;
    }
#else
    VideoReadIdx = (VideoBufMngWriteIdx) % VIDEO_BUF_NUM;
    VideoBufMngReadIdx = VideoReadIdx;
    AudioReadIdx = (iisSounBufMngWriteIdx) % IIS_BUF_NUM;
#endif    
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

	#if IIS1_REPLACE_IIS5
	if(guiIISCh0PlayDMAId==0xff)
	    marsDMAOpen(&guiIISCh0PlayDMAId, rfiuAudioRet_PlayDMA_ISR);
	else
	    DEBUG_RFIU_P2("1.Error! Audio Play DMA is occurpied!\n");
    #if(Audio_mode == AUDIO_AUTO)
    iis1SetNextPlayDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #else
    iisSetNextPlayDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #endif
	iisStartPlay(0);
	#else
    if(guiIISCh4PlayDMAId==0xff)
	    marsDMAOpen(&guiIISCh4PlayDMAId, rfiuAudioRet_PlayDMA_ISR);
	else
	    DEBUG_RFIU_P2("Error! Audio Play DMA is occurpied!\n");
    #if(Audio_mode == AUDIO_AUTO)
    iis5SetNextPlayDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #else
    iis5SetNextPlayDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #endif
	iisStartPlay(4);
	#endif
#endif
    
    timerCountRead(guiRFTimerID, &Broken_T1);
    Broken_T3=guiSysTimerCnt;
#if TX_PIRREC_VMDCHK
    VMDChk_T5=guiSysTimerCnt;
#endif

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
    #if (INSERT_NOSIGNAL_FRAME == 0)
    LinkBrokenTimeShift = 0;
    #endif
    DEBUG_RFIU_P2("\n===Link Broken time shift=%d ms,Prev AV totaltime=%d,%d ===\n",LinkBrokenTimeShift,gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime,gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime);
 #if RF_AV_SYNCTIME_EN   
    gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime=0;
    gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime=0;
 #endif
    while(1)
    {
        timerCountRead(guiRFTimerID, &Broken_T2);
        Broken_T4=guiSysTimerCnt;
#if TX_PIRREC_VMDCHK
        VMDChk_T6=guiSysTimerCnt;
#endif
        
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

                       VideoBufMng[VideoReadIdx].time=128;
                       
                       OSSemSet(gRfiuAVCmpSemEvt[0], 0, &err);
					   DEBUG_RFIU_P2("Audio Sync: %d,%d\n",VideoBufMngWriteIdx,VideoReadIdx);
                       break;
                    }
                    else
                    {
                       VideoDelay += (unsigned int)(VideoBufMng[VideoReadIdx].time & 0xffffffff);
                    }
					
                    VideoReadIdx = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                    VideoBufMngReadIdx = VideoReadIdx;
					video_value = rfiuCalBufRemainCount(VideoBufMngWriteIdx,VideoReadIdx,VIDEO_BUF_NUM);
                }
            }
            else
                OSTimeDly(1);
        }
        //------------------------------------------------//
    #if TX_PIRREC_SUPPORT     
        else if( (OffsetGrp >= RFI_BUF_SIZE_GRPUNIT*2/3) && (rfiuBatCamPIRTrig==1) )
        {
             //DEBUG_RFIU_P2("$");
             OSTimeDly(1);
        }
        else if( (rfiuBatCamPIRTrig==1) && (video_value==0) && (OffsetGrp < RFI_BUF_SIZE_GRPUNIT/16) && (rfiuStopPIRRecReady==1) )
        {
             DEBUG_RFIU_P2("==PIR REC STOP:%d==\n",OffsetGrp);
             gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=1;
             gRfiuUnitCntl[RFUnit].TX_Task_Stop=1;
             OSTimeDly(1);
             sysSaveAeExpoVal2UISetting();
  	         sysSaveLastBitRate();
        #if TX_PIR_INTERVAL_OFF
             sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 0);
        #else
             sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 1);
        #endif
        }
    #endif
        //------------------ Pause Mode ------------------//
        else if( ( (OffsetGrp >= RFI_BUF_SIZE_GRPUNIT*3/4) || (GoSyncIframe == 1) ) && (rfiuBatCamPIRTrig==0) )
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
                       if(OffsetGrp < RFI_BUF_SIZE_GRPUNIT/4)
                       {
                          GoSyncIframe=0;
                          OnlySendIframe=1;
                          //AudioDelay=VideoDelay;
                       }
                       else
                       {
                          VideoDelay += (unsigned int)(VideoBufMng[VideoReadIdx].time & 0xffffffff);
					      VideoReadIdx = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                          VideoBufMngReadIdx = VideoReadIdx;
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
                    VideoBufMngReadIdx = VideoReadIdx;
                    video_value = rfiuCalBufRemainCount(VideoBufMngWriteIdx,VideoReadIdx,VIDEO_BUF_NUM);
                }
				
            }
            else
                OSTimeDly(1);
        }
		//---------------------Low speed mode------------------------//
        else if( ( (OffsetGrp > RFI_BUF_SIZE_GRPUNIT*2/3) || (OnlySendIframe==1) ) &&  (rfiuBatCamPIRTrig==0) )
        {
            //Lucian: 若 RF 速度過慢,只傳送 I Frame. Audio frame 停止傳送.   
            // ------Streaming audio payload------//
            OnlySendIframe=1;
           #if RFTX_AUDIO_SUPPORT //(SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) //Don't transmit Audio Except baby monitor's application   
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
                        if(OffsetGrp < RFI_BUF_SIZE_GRPUNIT/6)
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
                    VideoBufMngReadIdx = VideoReadIdx;
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
            // ------streaming data payload------//
        #if TX_SNAPSHOT_SUPPORT
            if(sysRFTXSnapImgRdy)
            {
                RemainBufSize=RFI_TOTAL_BUF_SIZE - ((BufWriteAddrOffset+RFI_TOTAL_BUF_SIZE-BufReadAddrOffset) % RFI_TOTAL_BUF_SIZE);
                DataPayloadLen=(sysRFTXDataSize+3) & (~0x03);
                if(RemainBufSize>DataPayloadLen+PS_DATA_HEADERLEN)
                {
                    DEBUG_RFIU_P2("==Packing Date Payload==\n");
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
        #if RFTX_AUDIO_SUPPORT 
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
        #else  //Lucian: 不傳audio
            if( (audio_value > RFTX_AUDIO_TIMESHFT) )
            {
               AudioDelay=0;
               AudioReadIdx = (AudioReadIdx + 1) % IIS_BUF_NUM;
               OSSemAccept(gRfiuAVCmpSemEvt[0]);
            }
        #endif
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
                    VideoBufMngReadIdx = VideoReadIdx;
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
        {
            gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime += (*pp);
        }    
    #endif
       if(*pp > 8000)
           DEBUG_RFIU_P2("Warning!! Video Timestamp is too Long!:%d,%d\n",(unsigned int)pVideoBufMng->time,VideoDelay);
       if(*pp < 20)
           DEBUG_RFIU_P2("Warning!! Video Timestamp is too short!:%d,%d\n",(unsigned int)pVideoBufMng->time,VideoDelay);
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
       {
          gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime += (*pp);
       }   
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
    //=====//

    AudioPlayOffset= (rfiuAudioRetWrite_idx >= rfiuAudioRetPlay_idx) ?  (rfiuAudioRetWrite_idx - rfiuAudioRetPlay_idx) : (rfiuAudioRetWrite_idx +RFI_AUDIO_RET_BUF_NUM - rfiuAudioRetPlay_idx);
    if( (SilenceCnt>12) &&  (AudioPlayOffset != 0) )
    {
       SilenceCnt=0;
       rfiuAudioRetPlay_idx=(rfiuAudioRetWrite_idx-0) % RFI_AUDIO_RET_BUF_NUM;
       AudioPlayOffset=0;
    #if RF_TX_AUDIORET_DEBUG_ENA
       DEBUG_RFIU_P2("R");
    #endif    
   
    }
    
    if(AudioPlayOffset>25)
    {
       rfiuAudioRetPlay_idx=(rfiuAudioRetPlay_idx+1) % RFI_AUDIO_RET_BUF_NUM;
       AudioPlayOffset--;
    #if RF_TX_AUDIORET_DEBUG_ENA
       DEBUG_RFIU_P2("O");
    #endif    
      
    }

    if(AudioPlayOffset > 0)
    {
       Time_AudioPlayOffset = 1;
    #if RFIU_RX_AUDIO_RETURN   
       if(AudioPlayOffset>9)
          AdjustIISFreq(0);
       else if(AudioPlayOffset>7)
          AdjustIISFreq(0);
       else if(AudioPlayOffset>5)
          AdjustIISFreq(3);
       else if(AudioPlayOffset>3)
          AdjustIISFreq(5);
       else
          AdjustIISFreq(10);
    #endif
    
    #if( (CHIP_OPTION ==CHIP_A1016B) || (CHIP_OPTION ==CHIP_A1016A) )
       adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN+ADC_PGA_REDUCE_TALKBACK); //adc gain -6 dB
       iisboost_OFF();
    #else
       gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
    #endif

	   #if IIS1_REPLACE_IIS5
       #if (Audio_mode == AUDIO_AUTO)
       isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,(u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM]);
       #else
       iisSetNextPlayDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
       #endif
	   
	   #else
       #if (Audio_mode == AUDIO_AUTO)
       isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,(u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM]);
       #else
       iis5SetNextPlayDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
       #endif
	   #endif
       rfiuAudioRetPlay_idx = (rfiuAudioRetPlay_idx+1) % RFI_AUDIO_RET_BUF_NUM;
       SilenceCnt=0;
     #if RF_TX_AUDIORET_DEBUG_ENA
        DEBUG_RFIU_P2("%d ",AudioPlayOffset);
     #endif    
       
    }
    else
    {
    #if( (CHIP_OPTION ==CHIP_A1016B) || (CHIP_OPTION ==CHIP_A1016A) )
       adcSetADC_MICIN_PGA_Gain(ADC_PGA_GAIN);
       iisboost_ON();
    #else
        #if(ENABLE_DOOR_BELL)
            gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 1);
        #else
       gpioSetLevel(GPIO_GROUP_SPK_EN, GPIO_BIT_SPK_EN, 0);
    #endif
    #endif
       #if IIS1_REPLACE_IIS5
       #if (Audio_mode == AUDIO_AUTO)
       isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,rfiuAudioZeroBuf);
       #else
       iisSetNextPlayDMA(rfiuAudioZeroBuf, 1024);
       #endif	   
	   #else
       #if (Audio_mode == AUDIO_AUTO)
       isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,rfiuAudioZeroBuf);
       #else
       iis5SetNextPlayDMA(rfiuAudioZeroBuf, 1024);
       #endif
	   #endif
       SilenceCnt ++;
    #if RF_TX_AUDIORET_DEBUG_ENA
      DEBUG_RFIU_P2("S");
    #endif       
    }
    
    #if(Audio_mode != AUDIO_AUTO)
	iisStartPlay(4);
    #endif
}


#if PWIFI_SUPPORT
void pWifi_WrapTx_Task_UnitX(void* pData)
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
    unsigned int FillIdx,WriteIdx;
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
    //---//
#if TX_PIRREC_VMDCHK
    unsigned int VMDChk_T5=0;
    unsigned int VMDChk_T6=0;
#endif    

    //-------------------//   
    RFUnit= (int)pData;
#if TX_PIRREC_SUPPORT 
    if(rfiuBatCamPIRTrig)
    {
        VideoReadIdx = 0;
        AudioReadIdx = 0;
    }
    else
    {
        VideoReadIdx = (VideoBufMngWriteIdx) % VIDEO_BUF_NUM;
        AudioReadIdx = (iisSounBufMngWriteIdx) % IIS_BUF_NUM;
    }
#else
    VideoReadIdx = (VideoBufMngWriteIdx) % VIDEO_BUF_NUM;
    VideoBufMngReadIdx = VideoReadIdx;
    AudioReadIdx = (iisSounBufMngWriteIdx) % IIS_BUF_NUM;
#endif    
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

	#if IIS1_REPLACE_IIS5
	if(guiIISCh0PlayDMAId==0xff)
	    marsDMAOpen(&guiIISCh0PlayDMAId, rfiuAudioRet_PlayDMA_ISR);
	else
	    DEBUG_RFIU_P2("1.Error! Audio Play DMA is occurpied!\n");
    #if(Audio_mode == AUDIO_AUTO)
    iis1SetNextPlayDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #else
    iisSetNextPlayDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #endif
	iisStartPlay(0);
	#else
    if(guiIISCh4PlayDMAId==0xff)
	    marsDMAOpen(&guiIISCh4PlayDMAId, rfiuAudioRet_PlayDMA_ISR);
	else
	    DEBUG_RFIU_P2("Error! Audio Play DMA is occurpied!\n");
    #if(Audio_mode == AUDIO_AUTO)
    iis5SetNextPlayDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #else
    iis5SetNextPlayDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetPlay_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
    #endif
	iisStartPlay(4);
	#endif
#endif
    
    timerCountRead(guiRFTimerID, &Broken_T1);
    Broken_T3=guiSysTimerCnt;
#if TX_PIRREC_VMDCHK
    VMDChk_T5=guiSysTimerCnt;
#endif

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
    #if (INSERT_NOSIGNAL_FRAME == 0)
    LinkBrokenTimeShift = 0;
    #endif
    DEBUG_RFIU_P2("\n===Link Broken time shift=%d ms,Prev AV totaltime=%d,%d ===\n",LinkBrokenTimeShift,gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime,gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime);
 #if RF_AV_SYNCTIME_EN   
    gRfiuUnitCntl[RFUnit].RFpara.VideoTotalTime=0;
    gRfiuUnitCntl[RFUnit].RFpara.AudioTotalTime=0;
 #endif
    while(1)
    {
        timerCountRead(guiRFTimerID, &Broken_T2);
        Broken_T4=guiSysTimerCnt;
    #if TX_PIRREC_VMDCHK
        VMDChk_T6=guiSysTimerCnt;
    #endif        
        if(gRfiuUnitCntl[RFUnit].TX_Wrap_Stop)
        {
           DEBUG_RFIU_P2("@");
           OSTimeDly(1);
           continue;
        }
		video_value = rfiuCalBufRemainCount(VideoBufMngWriteIdx,VideoReadIdx,VIDEO_BUF_NUM);
        audio_value = rfiuCalBufRemainCount(iisSounBufMngWriteIdx,AudioReadIdx,IIS_BUF_NUM);

        OS_ENTER_CRITICAL();
        BufReadAddrOffset = (gRfiuUnitCntl[RFUnit].BufReadPtr & (PWIFI_PKTMAP_SIZE-1) ) << 10;
        OS_EXIT_CRITICAL();
        
        WriteIdx = (BufWriteAddrOffset >> 10); 
        WriteCnt = rfiuCalBufRemainCount(WriteIdx,FillIdx,PWIFI_PKTMAP_SIZE);
        
        OS_ENTER_CRITICAL();  
        for(i=0;i<WriteCnt;i++)
        {
           gRfiuUnitCntl[RFUnit].PktMapGrp[FillIdx/32]  |= ( 0x01 << (FillIdx & (32-1)) );
           FillIdx=(FillIdx + 1) & (PWIFI_PKTMAP_SIZE-1);
        } 
        gRfiuUnitCntl[RFUnit].BufWritePtr= WriteIdx;
        OffsetGrp = rfiuCalBufRemainCount(gRfiuUnitCntl[RFUnit].BufWritePtr,gRfiuUnitCntl[RFUnit].BufReadPtr,PWIFI_PKTMAP_SIZE);
        rfiuTxBufFullness[0]=OffsetGrp;
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

                       VideoBufMng[VideoReadIdx].time=128;
                       
                       OSSemSet(gRfiuAVCmpSemEvt[0], 0, &err);
					   DEBUG_RFIU_P2("Audio Sync: %d,%d\n",VideoBufMngWriteIdx,VideoReadIdx);
                       break;
                    }
                    else
                    {
                       VideoDelay += (unsigned int)(VideoBufMng[VideoReadIdx].time & 0xffffffff);
                    }
					
                    VideoReadIdx = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                    VideoBufMngReadIdx = VideoReadIdx;
					video_value = rfiuCalBufRemainCount(VideoBufMngWriteIdx,VideoReadIdx,VIDEO_BUF_NUM);
                }
            }
            else
                OSTimeDly(1);
        }
        //------------------------------------------------//
    #if TX_PIRREC_SUPPORT     
        else if( (OffsetGrp >= PWIFI_PKTMAP_SIZE*2/3) && (rfiuBatCamPIRTrig==1) )
        {
             //DEBUG_RFIU_P2("$");
             OSTimeDly(1);
        }
        else if( (rfiuBatCamPIRTrig==1) && (video_value==0) && (OffsetGrp < PWIFI_PKTMAP_SIZE/16) && (rfiuStopPIRRecReady==1) )
        {
             DEBUG_RFIU_P2("==PIR REC STOP:%d==\n",OffsetGrp);
             gRfiuUnitCntl[RFUnit].TX_Wrap_Stop=1;
             gRfiuUnitCntl[RFUnit].TX_Task_Stop=1;
             OSTimeDly(1);
             sysSaveAeExpoVal2UISetting();
  	         sysSaveLastBitRate();
        #if TX_PIR_INTERVAL_OFF
             sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 0);
        #else
             sysback_RF_SetEvt(SYS_BACKRF_RFI_ENTER_WOR_B1, 1);
        #endif
        }
    #endif
        //------------------ Pause Mode ------------------//
        else if( ( (OffsetGrp >= PWIFI_PKTMAP_SIZE*3/4) || (GoSyncIframe == 1) ) && (rfiuBatCamPIRTrig==0) )
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
                       if(OffsetGrp < PWIFI_PKTMAP_SIZE/4)
                       {
                          GoSyncIframe=0;
                          OnlySendIframe=1;
                          //AudioDelay=VideoDelay;
                       }
                       else
                       {
                          VideoDelay += (unsigned int)(VideoBufMng[VideoReadIdx].time & 0xffffffff);
					      VideoReadIdx = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                          VideoBufMngReadIdx = VideoReadIdx;
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
                    VideoBufMngReadIdx = VideoReadIdx;
                    video_value = rfiuCalBufRemainCount(VideoBufMngWriteIdx,VideoReadIdx,VIDEO_BUF_NUM);
                }
				
            }
            else
                OSTimeDly(1);
        }
		//---------------------Low speed mode------------------------//
        else if( ( (OffsetGrp > PWIFI_PKTMAP_SIZE*2/3) || (OnlySendIframe==1) ) &&  (rfiuBatCamPIRTrig==0) )
        {
            //Lucian: 若 RF 速度過慢,只傳送 I Frame. Audio frame 停止傳送.   
            // ------Streaming audio payload------//
            OnlySendIframe=1;
           #if RFTX_AUDIO_SUPPORT //(SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) //Don't transmit Audio Except baby monitor's application   
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
                        if(OffsetGrp < PWIFI_PKTMAP_SIZE/6)
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
                    VideoBufMngReadIdx = VideoReadIdx;
                    OSSemAccept(gRfiuAVCmpSemEvt[0]);
                 }
                 else
                 {
                     OSTimeDly(1);
                 }
            }
        }
        //---------------------Normal mode: Audio/Video 正常傳送---------------------------//
        else if( (OffsetGrp < PWIFI_PKTMAP_SIZE-1) && (GoFirst==0) )
        { 
            // ------streaming data payload------//
        #if TX_SNAPSHOT_SUPPORT
            if(sysRFTXSnapImgRdy)
            {
                RemainBufSize=RFI_TOTAL_BUF_SIZE - ((BufWriteAddrOffset+RFI_TOTAL_BUF_SIZE-BufReadAddrOffset) % RFI_TOTAL_BUF_SIZE);
                DataPayloadLen=(sysRFTXDataSize+3) & (~0x03);
                if(RemainBufSize>DataPayloadLen+PS_DATA_HEADERLEN)
                {
                    DEBUG_RFIU_P2("==Packing Date Payload==\n");
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
        #if RFTX_AUDIO_SUPPORT 
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
        #else  //Lucian: 不傳audio
            if( (audio_value > RFTX_AUDIO_TIMESHFT) )
            {
               AudioDelay=0;
               AudioReadIdx = (AudioReadIdx + 1) % IIS_BUF_NUM;
               OSSemAccept(gRfiuAVCmpSemEvt[0]);
            }
        #endif
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
                    VideoBufMngReadIdx = VideoReadIdx;
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
#endif


void pWifi_WrapRx_Task_UnitX(void* pData)
{
}

void rfiuAudioRet_RecDMA_ISR(int dummy)
{}


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
    int IsVideoFrame;
    unsigned int t1,t2,dt;
    int count=0;

    //-------------------//
    RFUnit= (int)pData;
	PsHeader.PreStatus = 0;	
    
#if (UI_BAT_SUPPORT && RFIU_RX_WAKEUP_TX_SCHEME) 
    if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && (gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode==0) )
    {
        if( 0 == sysGetBTCWakeStatus(RFUnit) )    
        {
             DEBUG_RFIU_P2("RF False WakeUp! TX goto sleep\n");
             count=0;
             while( 0 == rfiuCamSleepCmd(RFUnit) )
             {
                 OSTimeDly(10);
                 count ++;
                 if(count > 10)
                 {
                    DEBUG_RFIU_P2("Warning! Sleep comand Fail!!\n");
                    break;
                 }   
             }
             rfiuBatCam_LiveMaxTime[RFUnit]=RF_BATCAM_LIVE_MAXTIME * 10000;
             if(count <= 10)
             {
                gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
                while(gRfiuUnitCntl[RFUnit].RX_Wrap_Stop)
                {
                   //DEBUG_RFIU_P2("*");
                   OSTimeDly(1);
                   continue;
                }
             }   
        }
    }
#endif   
	
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

#if(RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL )
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

#if RFIU_RX_WAKEUP_TX_SCHEME
    rfiuBatCamVideo_TotalTime[RFUnit]=0;
    gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime=rfiuBatCam_PIRRecDurationTime;
#endif
    gRfiuUnitCntl[RFUnit].RFpara.PIR_TrigEvent=0;  //clear trigger event.
    gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=0;

 #if TUTK_SUPPORT 
    OSSemSet(P2PVideoCmpSemEvt[RFUnit],0, &err);  
    OSSemSet(P2PAudioCmpSemEvt[RFUnit],0, &err); 

    P2PVideoBufReadIdx[RFUnit]  = rfiuRxVideoBufMngWriteIdx[RFUnit];; 
    P2PAudioBufReadIdx[RFUnit]  = rfiuRxIIsSounBufMngWriteIdx[RFUnit];
    
    if (err != OS_NO_ERR) {
        DEBUG_RFIU_P2("OSSemSet Error: P2PVideoCmpSemEvt is %d.\n", err);
    }
 #endif

 #if USB_DONGLE_SUPPORT
    OSSemSet(USBVideoCmpSemEvt[RFUnit],0, &err);  
    OSSemSet(USBAudioCmpSemEvt[RFUnit],0, &err); 

    USBVideoBufReadIdx[RFUnit]  = rfiuRxVideoBufMngWriteIdx[RFUnit];; 
    USBAudioBufReadIdx[RFUnit]  = rfiuRxIIsSounBufMngWriteIdx[RFUnit];
    
    if (err != OS_NO_ERR) {
        DEBUG_RFIU_P2("OSSemSet Error: USBVideoCmpSemEvt is %d.\n", err);
    }
 #endif

    VideoFrameCnt=0;
    FrameRate=0;
    timerCountRead(guiRFTimerID, &t1);
    gRfiuUnitCntl[RFUnit].FrameRate=0;
    DEBUG_RFIU_P2("\n============rfiu_WrapRx_Task_UnitX(%d)==========\n",RFUnit);  

	while(1)
    {
        if(gRfiuUnitCntl[RFUnit].RX_Wrap_Stop)
        {
           //DEBUG_RFIU_P2("*");
           if(gRfiuUnitCntl[RFUnit].RX_Wrap_Stop == 1)
              gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=2;
           
           OSTimeDly(1);
           continue;
        }
            
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    	if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
        {
            VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = (u16)gRfiuUnitCntl[RFUnit].TX_PicWidth;
            if(gRfiuUnitCntl[RFUnit].TX_PicHeight == 1088)
                VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 1080;
            else
                VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = (u16)gRfiuUnitCntl[RFUnit].TX_PicHeight;
    	    RfRxVideoPackerSubTaskCreate(RFUnit, 
    	                                 &VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX]);
        }
#endif

#if RFIU_RX_WAKEUP_TX_SCHEME
        //if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
        if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode)
        {
             if(
                    (rfiuBatCamVideo_TotalTime[RFUnit] > (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime+1) * 1000) ||
                    (rfiuBatCamVideo_TotalTime[RFUnit] > RF_BATCAM_LIVE_MAXTIME * 1000)
                )
             {
                 DEBUG_RFIU_P2("\n--BATCAM Recording timeup!(%d,%d)--\n",RFUnit,rfiuBatCamVideo_TotalTime[RFUnit]);
                 if( 1 == rfiuCamSleepCmd(RFUnit) )
                 {
                    rfiuBatCamVideo_TotalTime[RFUnit]=0;
                 }
                 else
                 {
                    rfiuBatCamVideo_TotalTime[RFUnit] -= 2*1000;
                    gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=1;
                 }
             }
        #if(BTC_PIRREC_MODESEL == BTC_PIRRECMODE_ARLO)    
             else if( 
                        (rfiuBatCamVideo_TotalTime[RFUnit] > (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO-1) * 1000) &&
                        (rfiuBatCamVideo_TotalTime[RFUnit] < (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO) * 1000)
                    )
             {
                 gRfiuUnitCntl[RFUnit].RFpara.PIR_TrigEvent=0;
                 gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=0;
             }
             else if(rfiuBatCamVideo_TotalTime[RFUnit] > (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO) * 1000)
             {
                 if(gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn == 0)
                 {
                     if(gRfiuUnitCntl[RFUnit].RFpara.PIR_TrigEvent)
                     {
                         gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime +=PIRREC_TIMEEXTENDSEC_ARLO;
                         gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=1;
                         DEBUG_RFIU_P2("\n--PIR REC Time Extention!(%d,%d,%d)--\n",RFUnit,gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime,rfiuBatCamVideo_TotalTime[RFUnit]/1000);
                     }
                 }
             }
        #elif(BTC_PIRREC_MODESEL == BTC_PIRRECMODE_LOREX)
             else if(rfiuBatCam_LiveMaxTime[RFUnit] > rfiuBatCam_PIRRecDurationTime*5/2 * 10000)
             {
                 DEBUG_RFIU_P2("\n--BATCAM Recording Over Max Time!(%d,%d)--\n",RFUnit,rfiuBatCam_LiveMaxTime[RFUnit]);
                 if(1 == rfiuCamSleepCmd(RFUnit))
                 {
                    rfiuBatCam_LiveMaxTime[RFUnit]=0;
                 }
                 else
                 {
                    rfiuBatCam_LiveMaxTime[RFUnit] -= (2*10000);
                 }
             }
        #elif(BTC_PIRREC_MODESEL == BTC_PIRRECMODE_DOORBELL)
             else if( 
                        (rfiuBatCamVideo_TotalTime[RFUnit] > (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO-1) * 1000) &&
                        (rfiuBatCamVideo_TotalTime[RFUnit] < (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO) * 1000)
                    )
             {
                 gRfiuUnitCntl[RFUnit].RFpara.PIR_TrigEvent=0;
                 gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=0;
             }
             else if(rfiuBatCamVideo_TotalTime[RFUnit] > (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO) * 1000)
             {
                 if(gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn == 0)
                 {
                 #if ENABLE_DOOR_BELL
                     if(sysGetDoorBellState() == SYS_DOOR_TALK)
                     {
                         gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime +=PIRREC_TIMEEXTENDSEC_ARLO;
                         gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=1;
                         DEBUG_RFIU_P2("\n--Doorbell REC Time Extention!(%d,%d,%d)--\n",RFUnit,gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime,rfiuBatCamVideo_TotalTime[RFUnit]/1000);
                     }
                 #endif
                 }
             }
        #endif    
        }

        if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
        {
             if(rfiuBatCam_LiveMaxTime[RFUnit] > RF_BATCAM_LIVE_MAXTIME * 10000)
             {
                 DEBUG_RFIU_P2("\n--BATCAM LiveView timeup!(%d,%d)--\n",RFUnit,rfiuBatCam_LiveMaxTime[RFUnit]);
                 if(1 == rfiuCamSleepCmd(RFUnit) )
                 {
                    rfiuBatCam_LiveMaxTime[RFUnit]=0;
                    rfiuBatCamVideo_TotalTime[RFUnit]=0;
                    gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=0;
                 }
                 else
                 {
                    rfiuBatCam_LiveMaxTime[RFUnit] -= (2*10000);
                 }
             }

        }

#endif

        OS_ENTER_CRITICAL();
        BufWriteAddrOffset=(gRfiuUnitCntl[RFUnit].BufWritePtr<<13)+(gRfiuUnitCntl[RFUnit].WritePtr_Divs<<10);
        OS_EXIT_CRITICAL();
        RemainBufSize=(BufWriteAddrOffset+RFI_TOTAL_BUF_SIZE-BufReadAddrOffset) % RFI_TOTAL_BUF_SIZE;   

        if(RemainBufSize > RFI_TOTAL_BUF_SIZE/2)
        {
        #if RFIU_RX_WAKEUP_TX_SCHEME
            if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode)
            {
                if(RemainBufSize > RFI_TOTAL_BUF_SIZE*4/5)
                {
                    DEBUG_RFIU_P2("--->WrapRX too slow! RemainBufSize=%d\n",RemainBufSize);
        			OS_ENTER_CRITICAL();
                    BufReadAddrOffset= (gRfiuUnitCntl[RFUnit].BufWritePtr<<13)+(gRfiuUnitCntl[RFUnit].WritePtr_Divs<<10); 
                    OS_EXIT_CRITICAL();
        			continue;
                }  
            }
            else
        #endif        
            {
                DEBUG_RFIU_P2("--->WrapRX too slow! RemainBufSize=%d\n",RemainBufSize);
    			OS_ENTER_CRITICAL();
                BufReadAddrOffset= (gRfiuUnitCntl[RFUnit].BufWritePtr<<13)+(gRfiuUnitCntl[RFUnit].WritePtr_Divs<<10); 
                OS_EXIT_CRITICAL();
    			continue;
            }    
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
                     DEBUG_RFIU_P2("Warning! Pack Size not Valid:%d\n",PsHeader.PayLoadSize);
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
                        VideoFrameCnt +=IsVideoFrame;
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
                           t1=t2;
                           VideoFrameCnt=0;
                        }		
                        #if 0 //RF_LETENCY_DEBUG_ENA
                           if(sysRFRxInMainCHsel==RFUnit)
        				      DEBUG_RFIU_P2("WrapBuf=%d\n",RemainBufSize >> 10);
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
       OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_ERROR<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
       DEBUG_MP4("\n======== RF Wrap Error! Resync:%d!======\n",RFUnit);
   #if PWIFI_SUPPORT

   #else
       if(gRfiuUnitCntl[RFUnit].RX_Task_Stop==0)
           sysback_RF_SetEvt(SYS_BACKRF_RFI_FORCERESYNC, RFUnit);
   #endif    
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
#if(RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 ||\
    RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL)
   unsigned char *pp;
   int i;
   int PTS_flag;
   unsigned char *pBuf;
   unsigned int  cpu_sr = 0;  
   int AVFrame;
   //===//
 #if ASF_DEBUG_ENA
   int ASF_time=0;
   int ASF_size=0;
 #endif
 #if VIDEO_STARTCODE_DEBUG_ENA	
   unsigned char *pReadBuf;
   u16            video_value;
 #endif

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
        video_value = pvcoRfiu[RFUnit]->VideoCmpSemEvt->OSEventCnt;
      #endif
		OS_EXIT_CRITICAL();
      #if VIDEO_STARTCODE_DEBUG_ENA
		OS_ENTER_CRITICAL();
		if((pBuf < pReadBuf)
		&&((pBuf+pHeader->PayLoadSize) > pReadBuf)
		)
		{
			DEBUG_ASF("Warning!!! CH%02d RF write fast than ASF read, %d\n", RFUnit, video_value);
			DEBUG_ASF("Warning!!! Buffer range  0x%08x, 0x%08x, %d\n", rfiuRxVideoBuf[RFUnit], rfiuRxVideoBufEnd[RFUnit], pvcoRfiu[RFUnit]->CurrentBufferSize);	
			DEBUG_ASF("Warning!!! AA. CH%02d %d, 0x%08x\n", RFUnit, rfiuRxVideoBufMngWriteIdx[RFUnit], rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer);
			DEBUG_ASF("Warning!!! BB. CH%02d %d, 0x%08x\n", RFUnit, VideoClipOption[RFUnit].VideoBufMngReadIdx, VideoClipOption[RFUnit].VideoBufMng[VideoClipOption[RFUnit].VideoBufMngReadIdx].buffer);							
		}
		
		OS_EXIT_CRITICAL();									
	  #endif
		
		if(pBuf + pHeader->PayLoadSize + EndCodeSize < rfiuRxVideoBufEnd[RFUnit]) 			
		{
			mcpu_ByteMemcpy(pBuf, pp, pHeader->PayLoadSize);
			#if CHECK_VIDEO_BITSTREAM	
			*(unsigned int*)(pBuf + pHeader->PayLoadSize) = 0x00000001;		
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
				*(unsigned int*)(pBuf + pHeader->PayLoadSize) = 0x00000001;
				#endif
			}
			else
			{
				pBuf=rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer=rfiuRxVideoBuf[RFUnit];
				mcpu_ByteMemcpy(pBuf, pp, pHeader->PayLoadSize);
				
				#if CHECK_VIDEO_BITSTREAM	
				*(unsigned int*)(pBuf + pHeader->PayLoadSize) = 0x00000001;
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

           //DEBUG_RFIU_P2("%d ",pHeader->Status & PS_STA_IFRAME);
           OS_ENTER_CRITICAL();
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer=pBuf - rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size +=pHeader->PayLoadSize + EndCodeSize;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].flag  =pHeader->Status & PS_STA_IFRAME;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].asfflag  =pHeader->Status & PS_STA_IFRAME;
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time  =(s64)(pHeader->PTS_L);
           rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].offset=pHeader->BotFldOffset;
       #if 0
           DEBUG_RFIU_P2("V:%d,%d,%d\n",rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size,
                                        rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].flag,
                                       (s32)rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time);
       #endif
#if (VIDEO_CODEC_OPTION == H264_CODEC)
           if(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].flag == 1)
           {
               H264_IFlag_Index[RFUnit] = rfiuRxVideoBufMngWriteIdx[RFUnit];
               H264_FrameError[RFUnit] = 0;
           }
#endif           
           
          #if ASF_DEBUG_ENA
           ASF_time = rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time;
          #endif
          #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
            if((Lose_video_time[RFUnit] + rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time) > (asfSectionTime*1000))
            {
                Lose_video_time[RFUnit] = 0;
                rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time = 66;
            }
            if(ASF_Restart[RFUnit] == 1)
            {
                rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time = 66;
                ASF_Restart[RFUnit] = 0;
            }
          #endif
          //printf("RF size %d\n",rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].offset);
          #if MULTI_CHANNEL_RF_RX_VIDEO_REC
           if((rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time > (asfSectionTime*1000)) || (rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time < 0))
          #else
           if(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time < 0)
          #endif
           {  //Lucian: timestamp若超過 120 sec,預測不正常.
             #if ASF_DEBUG_ENA
           	  RX_skip_V += rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time - 66;
             #endif
              DEBUG_RFIU_P2("Video PTS is invalid:%d,%d\n", (s32)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time),pHeader->PTS_L);
              rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time=66;
           }
          
           if((rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time > 3000) || (rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time < 20))
           {
              DEBUG_RFIU_P2("Video PTS is invalid:%d,%d\n", (s32)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time),pHeader->PTS_L);
           #if RFIU_RX_WAKEUP_TX_SCHEME
              rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time=66;
           #endif
           }   
           //DEBUG_RFIU_P2("%d ",pHeader->PTS_L);
           #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
              Video_totaltime[RFUnit] += (u32)rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time; 
           #endif

           #if RFIU_RX_WAKEUP_TX_SCHEME
              rfiuBatCamVideo_TotalTime[RFUnit] += (u32)rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time;
           #endif
          
           #if MULTI_CHANNEL_RF_RX_VIDEO_REC
            if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
            {         
                if(pvcoRfiu[RFUnit]->PackerTaskCreated && pvcoRfiu[RFUnit]->sysCaptureVideoStart)
                {
                   pvcoRfiu[RFUnit]->VideoTimeStatistics  += rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time;
                   pvcoRfiu[RFUnit]->CurrentVideoSize     += rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].offset;
                   pvcoRfiu[RFUnit]->CurrentBufferSize    += rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size;
                   pvcoRfiu[RFUnit]->CurrentVideoTime     += rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time;
                }
             }
           #endif

		 #if  VIDEO_STARTCODE_DEBUG_ENA
			if(((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer)) == 0x00) && ((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer+1)) == 0x00) 
	        && ((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer+2)) == 0x00) && (((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer+3)) == 0x01))
	        && ((((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer+4)) == 0x41))||(((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer+4)) == 0x45))||(((unsigned char)(*(unsigned char *)(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer+4)) == 0x65)))
	        )
			{
			}
			else
			{
				monitor_RX[RFUnit]++;				
				DEBUG_RFIU_P2("Warning!!! X CH%02d size=%d, UnpackPS_RF MPEG4 start code error - %d\n", RFUnit, rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].size, VideoClipOption[RFUnit].VideoCmpSemEvt->OSEventCnt);
					
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
		   rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].buffer=pBuf + pHeader->PayLoadSize + EndCodeSize;;
		   #endif
		   OS_EXIT_CRITICAL();
            #if USB_DONGLE_SUPPORT
            if(USBEnableStreaming[RFUnit]) 
            {			
            	OSSemPost(USBVideoCmpSemEvt[RFUnit]); 
                OSFlagPost (OSUSB_task_flag, 0x10,OS_FLAG_SET,0);
            }	
            #endif	
            
           //DEBUG_RFIU_P2("Unpack Video: size=%d\n",VideoBufMng[VideoBufMngWriteIdx].size);
           #if MULTI_CHANNEL_RF_RX_VIDEO_REC
            if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
                if(pvcoRfiu[RFUnit]->PackerTaskCreated && pvcoRfiu[RFUnit]->sysCaptureVideoStart)
                {
	                OSSemPost(pvcoRfiu[RFUnit]->VideoCmpSemEvt); 
                  #if ASF_DEBUG_ENA
                    RX_time_V += ASF_time;
                    RX_sem_V++;
//                    DEBUG_RFIU_P2("RF_VideoTime = %d\n",RX_time_V);
                  #endif
                }
				#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
				else
				{
					//Lsk: Debug Can't start from I frame
					if(pvcoRfiu[RFUnit]->ShowDebugMsgFlag)
						DEBUG_RED("%s, %d. CH%d <%d, %d>\n", __FUNCTION__, __LINE__, RFUnit, pvcoRfiu[RFUnit]->PackerTaskCreated, pvcoRfiu[RFUnit]->sysCaptureVideoStart);
				}
				#endif
           #endif
           AVFrame=1;
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

			#if USB_DONGLE_SUPPORT
				if(USBEnableStreaming[RFUnit]) 
				{
	            	OSSemPost(USBAudioCmpSemEvt[RFUnit]);
                    OSFlagPost (OSUSB_task_flag, 0x20,OS_FLAG_SET,0);
				}	
			#endif	

           OS_ENTER_CRITICAL();
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].buffer=pBuf - rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size;
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size +=pHeader->PayLoadSize;
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].flag  =pHeader->Status & PS_STA_IFRAME;
           rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time  =(s64)(pHeader->PTS_L);

           //DEBUG_RFIU_P2("Unpack Audio:%d,%d\n",rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size,rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time);
          #if ASF_DEBUG_ENA
            ASF_time = rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time;
            ASF_size = rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size;
          #endif
          #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
           if((Lose_audio_time[RFUnit] + rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time) > (asfSectionTime*1000))
           {
               Lose_audio_time[RFUnit] = 0;
               rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time = 128;
           }
           if(ASF_Restart[RFUnit] == 1)
           {
               rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time = 128;
               ASF_Restart[RFUnit] = 0;
           }
          #endif
          #if MULTI_CHANNEL_RF_RX_VIDEO_REC
           if((rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time > (asfSectionTime*1000)) || (rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time < 0))
          #else
           if(rfiuRxVideoBufMng[RFUnit][rfiuRxVideoBufMngWriteIdx[RFUnit]].time < 0)
          #endif
           {
             #if ASF_DEBUG_ENA
              RX_skip_A += rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time - 128;
             #endif
              DEBUG_RFIU_P2("Audio PTS is invalid:%d\n",(s32)(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time) );
              rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time=128;
           }

           //DEBUG_RFIU_P2("%d ",pHeader->PTS_L);
           
        #if 1
           if(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size > 3000)
           {
              DEBUG_RFIU_P2("Audio Len is invalid:%d\n",rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size );
              rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size=2048;
           }
        #endif
          #if (INSERT_NOSIGNAL_FRAME && (NOSIGNAL_MODE == 3))
            Audio_totaltime[RFUnit] += (u32)rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].time; 
          #endif
           #if MULTI_CHANNEL_RF_RX_VIDEO_REC
            if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
                if(pvcoRfiu[RFUnit]->PackerTaskCreated && pvcoRfiu[RFUnit]->sysCaptureVideoStart)
                    pvcoRfiu[RFUnit]->CurrentAudioSize     += rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngWriteIdx[RFUnit]].size;
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
           #if MULTI_CHANNEL_RF_RX_VIDEO_REC
            if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
                if(pvcoRfiu[RFUnit]->PackerTaskCreated && pvcoRfiu[RFUnit]->sysCaptureVideoStart)
                {
	                OSSemPost(pvcoRfiu[RFUnit]->iisCmpSemEvt); 
                  #if ASF_DEBUG_ENA
                    RX_time_A += ASF_time;
//                    DEBUG_RFIU_P2("RF_AudioTime = %d\n",RX_time_A);
                    if(ASF_time != 128 || ASF_size!=2048)
                        DEBUG_RFIU_P2("<RRR> 1.over duration <%d, %d>\n", ASF_time, ASF_size);
                    RX_sem_A++;
                  #endif

                }
           #endif
           //AVFrame=2;
        } 
               
   }
   else if( (pHeader->StreamType >> 4) == 0x00 )
   {
        DEBUG_RFIU_P2("Data PayLoad Size=%d\n",pHeader->PayLoadSize);
#if RX_SNAPSHOT_SUPPORT
        pBuf=rfiuRxDataBufMng[RFUnit].buffer;
        mcpu_ByteMemcpy(pBuf, pp, pHeader->PayLoadSize);
        if(pHeader->Status & PS_STA_CURRENT_IND)
        {
           rfiuRxDataBufMng[RFUnit].buffer=pBuf + pHeader->PayLoadSize;
           rfiuRxDataBufMng[RFUnit].size +=pHeader->PayLoadSize;
		}
        else
        {
           rfiuRxDataBufMng[RFUnit].buffer=rfiuRxDataBuf[RFUnit];
           rfiuRxDataBufMng[RFUnit].size +=pHeader->PayLoadSize;
           //===Write to SD ===//
           //sysbackSetEvt(SYS_BACK_RX_DATASAVE, RFUnit);
           sysSetEvt(SYS_EVT_RX_DATASAVE, RFUnit);

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
    u32 *pVideoSize, last_I_size;
    u32  VideoOffset;
    u8 *pVideoBuf;
    u32 VideoDuration;
    u32 DisplaySync;
    u32 smooth_letency, letency_interval;

    u32 Vop_Result = MPEG4_NORMAL;
  #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
	MP4_Option mpeg4Dec;
  #elif (VIDEO_CODEC_OPTION == H264_CODEC)
    H264_ENC_CFG H264Enc;
    H264_DEC_CFG H264Dec;
	VIDEO_INFO video_info;  
   #if SB_DECORD_SUPPORT
    H264_ENC_CFG H264Enc_sub;
    H264_DEC_CFG H264Dec_sub;
	VIDEO_INFO video_info_sub;
   #endif
  #endif
    //===Audio===//
    u32 *pAudioFlag;
    u32 *pAudioSize;
    u8 *pAudioBuf;
    u32 AudioDuration;
	u32 prev_AudioTime;
 #if (HDMI_TXIC_SEL == HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)   
    u8  OSD_BLK[2] = {OSD_L1Blk0, OSD_Blk0};
 #endif
    int VideoLatency;

    //======//
    unsigned int t1,t2,dt,FrameRate,FrameCnt;
    unsigned int t3,t4;
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
    int NoSubStreamCnt=0;
    int audioQueueDepth;
    
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

    //DEBUG_RFIU_P2("\n sysCameraMode=%d\n",sysCameraMode);
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
            if(sysTVOutOnFlag)
            {
                if(BRI_SCCTRL_MODE & 0x4)
                {
                    iduTVColorbar_onoff(0); 
                    //iduTVOSDEnable(IDU_OSD_L0_WINDOW_0);
                }
            }
        }

     #if 1  
        if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
        {
            if(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth > RF_RX_2DISP_WIDTH*2) // Full HD
            {
            #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2))
               //iduPlaybackMode( 1280 & 0xffffff80,720,RF_RX_2DISP_WIDTH*2);
               memDispBufArrage(DISPBUF_9300FHD);
               iduPlaybackMode( (1920-128) & 0xffffff80,720,1920*2);
            #elif ((SW_APPLICATION_OPTION == MR9100_RF_CVI_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHD_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_AHDIN_AVSED_RX1) || (SW_APPLICATION_OPTION == MR9100_RF_HDMI_AVSED_RX1))
               iduPlaybackMode(1920,1080,1920);
            #elif( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
               if(sysTVOutOnFlag)
               {
                   if(sysRFRxInMainCHsel == RFUnit)
                   {
                   #if (HDMI_TXIC_SEL == HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                     #if (UI_SYNCHRONOUS_DUAL_OUTPUT == 1)
                        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
                     #endif
                        Idu_ClearBuf(DISPLAY_BUF_NUM);
                        iduTVColorbar_onoff(1);
                        //iduTVOSDDisable(IDU_OSD_L0_WINDOW_0);
                        sysTVswitchResolutionbyImagesize();
                        iduTVOSDDisplay(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
                        OSTimeDly(2); // wait 2 ticks, because HDMI & IT66121 are not finish transforming.
                      #if (UI_SYNCHRONOUS_DUAL_OUTPUT == 1)
                        if (uiEnterScanMode == 0) //don't light up backlight when powersaving, wish someday remove this
                        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
                      #endif
                        //iduTVColorbar_onoff(0); 
                        //iduTVOSDEnable(IDU_OSD_L0_WINDOW_0);
                   #else
                        if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                          iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                        else
                          iduPlaybackMode(1920,1080,1920);
                   #endif
                   }
                 #if 0
                    if(BRI_SCCTRL_MODE & 0x4)
                    {
                        iduTVColorbar_onoff(0); 
                        iduTVOSDEnable(IDU_OSD_L0_WINDOW_0);
                    }
                 #endif   
               }
               else
                  iduPlaybackMode(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth/2,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight/2,RF_RX_2DISP_WIDTH*2);
            #else
               iduPlaybackMode(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth/2,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight/2,RF_RX_2DISP_WIDTH*2);
            #endif
            }
            else
            {
            #if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) )
               memDispBufArrage(DISPBUF_NORMAL);
            #endif
            #if( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
               if(sysTVOutOnFlag)
               {
                   if(sysRFRxInMainCHsel == RFUnit)
                   {
                   #if (HDMI_TXIC_SEL == HDMI_TX_EP952 || HDMI_TXIC_SEL == HDMI_TX_IT66121)
                     #if (UI_SYNCHRONOUS_DUAL_OUTPUT == 1)
                        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 0);
                     #endif
                        Idu_ClearBuf(DISPLAY_BUF_NUM);
                        iduTVColorbar_onoff(1);
                        //iduTVOSDDisable(IDU_OSD_L0_WINDOW_0);
                        sysTVswitchResolutionbyImagesize();
                        iduTVOSDDisplay(OSD_BLK[sysTVOutOnFlag], 0, 0, OSDDispWidth[sysTVOutOnFlag], OSDDispHeight[sysTVOutOnFlag]);
                        OSTimeDly(2);   // wait 2 ticks, because HDMI & IT66121 are not finish transforming.
                     #if (UI_SYNCHRONOUS_DUAL_OUTPUT == 1)
                        if (uiEnterScanMode == 0) //don't light up backlight when powersaving, wish someday remove this
                        gpioSetLevel(GPIO_GROUP_BACK_LIGHT, GPIO_BIT_BACK_LIGHT, 1);
                     #endif
                        //iduTVColorbar_onoff(0);
                        //iduTVOSDEnable(IDU_OSD_L0_WINDOW_0);
                   #else
                        if ((TvOutMode == SYS_TV_OUT_HD720P60)||(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) )
                          iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
                        else
                          iduPlaybackMode(1920,1080,1920);
                   #endif
                    }
               #if 0
                    if(BRI_SCCTRL_MODE & 0x4)
                    {
                        iduTVColorbar_onoff(0);
                        iduTVOSDEnable(IDU_OSD_L0_WINDOW_0);
                    }
               #endif
               }
               else
               {
               if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==0) && (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight==0) )
                   iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
               else
                   iduPlaybackMode(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight,RF_RX_2DISP_WIDTH*2);
               }
            #else
               if( (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth==0) && (gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight==0) )
                   iduPlaybackMode(RF_RX_2DISP_WIDTH*2,RF_RX_2DISP_HEIGHT*2,RF_RX_2DISP_WIDTH*2);
               else
                   iduPlaybackMode(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight,RF_RX_2DISP_WIDTH*2);
            #endif
            }
        }
        else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR)
        {            
             iduPlaybackMode(gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicWidth,gRfiuUnitCntl[sysRFRxInMainCHsel].TX_PicHeight,RF_RX_2DISP_WIDTH);
        }
      #endif     
	    
	  #if (RFIU_RX_AVSYNC && RFIU_RX_AUDIO_ON)
        if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) || (sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR) )
        {
          #if IIS1_REPLACE_IIS5
            iisStopPlay();
    	    if(guiIISCh0PlayDMAId==0xff)
    	    {
    	        DEBUG_RFIU_P2("--->Open RF Audio:%d!\n",RFUnit);
    	        marsDMAOpen(&guiIISCh0PlayDMAId, rfiuAudioPlayDMA_ISR);
    	    }
    	    else
    	        DEBUG_RFIU_P2("Error! Audio Play DMA is occurpied!\n");
          #else
            iis5StopPlay();
    	    if(guiIISCh4PlayDMAId==0xff)
    	    {
    	        DEBUG_RFIU_P2("--->Open RF Audio:%d!\n",RFUnit);
    	        marsDMAOpen(&guiIISCh4PlayDMAId, rfiuAudioPlayDMA_ISR);
    	    }
    	    else
    	        DEBUG_RFIU_P2("Error! Audio Play DMA is occurpied!\n");
          #endif
        }
        #if RFRX_QUAD_AUDIO_EN
        else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
        {
          #if IIS1_REPLACE_IIS5
            iisStopPlay();
    	    if(guiIISCh0PlayDMAId==0xff)
    	    {
    	        DEBUG_RFIU_P2("--->Open RF Audio:%d!\n",RFUnit);
    	        marsDMAOpen(&guiIISCh0PlayDMAId, rfiuAudioPlayDMA_ISR);
    	    }
    	    else
    	        DEBUG_RFIU_P2("Error! Audio Play DMA is occurpied!\n");
          #else
            iis5StopPlay();
    	    if(guiIISCh4PlayDMAId==0xff)
    	    {
    	        DEBUG_RFIU_P2("--->Open RF Audio:%d!\n",RFUnit);
    	        marsDMAOpen(&guiIISCh4PlayDMAId, rfiuAudioPlayDMA_ISR);
    	    }
    	    else
    	        DEBUG_RFIU_P2("Error! Audio Play DMA is occurpied!\n");
          #endif
        }
        #endif
	  #endif
      
      #if (RFIU_RX_AUDIO_RETURN)
        if(guiIISCh0RecDMAId==0xff)
            marsDMAOpen(&guiIISCh0RecDMAId, rfiuAudioRet_RecDMA_ISR);
        else
            DEBUG_RFIU_P2("Error! Audio Rec DMA is occurpied!\n");

        for(i=0;i<RFI_AUDIO_RET_BUF_NUM;i++)
           rfiuAudioRetDMANextBuf[i] = rfiuOperBuf[sysRFRxInMainCHsel] + RFI_AUDIORETURN1_ADDR_OFFSET*128 + i*1024;

        rfiuAudioRetRec_idx=0;
        rfiuAudioRetRead_idx=0;
        #if (Audio_mode == AUDIO_AUTO)
        iisSetNextRecDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx % RFI_AUDIO_RET_BUF_NUM], 1024,sysRFRxInMainCHsel);
        #else
        iisSetNextRecDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx % RFI_AUDIO_RET_BUF_NUM], 1024);
        #endif        
        iisStartRec();
      #endif
	  
	    if(sysTVOutOnFlag) 
	    {
            if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
               tvTVE_INTC   =TV_INTC_FRMEND__ENA;
            else
               tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
	    }
		else
	    {
	        IduIntCtrl |= IDU_FTCINT_ENA;
		}
    }
	else
    {   //Dual mode and PIP mode use
        rfiuRxSub1VideoPlayStart=0;
		
	    for(i=0;i<DISPLAY_BUF_NUM;i++)
	      rfiuSub1VideoPresentTime[i]=0;
	    rfiuSub1VideoTime=0; 
	    rfiuSub1VideoTime_frac=0;
		
	    if(sysTVOutOnFlag) 
	    {
            if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
               tvTVE_INTC   =TV_INTC_FRMEND__ENA;
            else
               tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
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
  #if SB_DECORD_SUPPORT
    rfiuH264Encode_I_Header(&H264Enc_sub, 640, 352);
    rfiuH264Decode_I_Header(&H264Enc_sub,&H264Dec_sub); 
  #endif
#endif
	
#if RFIU_RX_AVSYNC
    OSTimeDly(1); // 50 ms for buffering
#endif
    timerCountRead(guiRFTimerID, &t1);
    t4=t3=t1;
    FrameRate=0;

    //gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=0;
    DEBUG_RFIU_P2("\n============rfiu_RxMpeg4DecTask_UnitX(%d)==========\n",RFUnit);
    while (1)
    {   
        if(gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop)
        {
           //DEBUG_RFIU_P2("#");
           if(gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop == 1)  
              gRfiuUnitCntl[RFUnit].RX_MpegDec_Stop=2;
           
           OSTimeDly(1);
           continue;
        }
        if(MpegErrCnt>10)
        {
            DEBUG_RFIU_P2("\n======== RFMpegDec-%d is Fatal Error(%d)! Reboot!======\n",RFUnit,MpegErrCnt);
			sysForceWDTtoReboot();
        }
    
        RunVideo=0;
        RunAudio=0;
		
#if RFIU_RX_AUDIO_ON        
        //-----Audio------//
        //support PCM only
        if(sysRFRxInMainCHsel==RFUnit)
        {
            if(gRfiuUnitCntl[RFUnit].BitRate > 30)
                audioQueueDepth = 7;
            else if(gRfiuUnitCntl[RFUnit].BitRate > 25)
                audioQueueDepth = 8;
            else if(gRfiuUnitCntl[RFUnit].BitRate > 20)
                audioQueueDepth = 9;
            else if(gRfiuUnitCntl[RFUnit].BitRate > 15)
                audioQueueDepth = 10;
            else 
                audioQueueDepth = 11;            
        
            AudioBufOffset = rfiuCalBufRemainCount(rfiuRxIIsSounBufMngWriteIdx[RFUnit],rfiuRxIIsSounBufMngReadIdx[RFUnit],IIS_BUF_NUM);
            AudioPlayOffset= rfiuCalAudioplayBufCount(rfiuAudioBufFill_idx[RFUnit], rfiuAudioBufPlay_idx[RFUnit],IISPLAY_BUF_NUM);
            
	        if( (AudioBufOffset>0) && ((SYNC==1) || (AudioPlayOffset>=0)) )
	        {
	            pAudioBuf        = rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngReadIdx[RFUnit]].buffer;  
	            pAudioFlag       = &(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngReadIdx[RFUnit]].flag);    
	            AudioDuration    = (u32)(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngReadIdx[RFUnit]].time); 
	            pAudioSize       = &(rfiuRxIIsSounBufMng[RFUnit][rfiuRxIIsSounBufMngReadIdx[RFUnit]].size); 

                if(AudioDuration >4000) //Lucian: 保護斷線時間 error.
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
                        AdjustIISFreq(0);
                    else if(AudioBufOffset > 3)	
                        AdjustIISFreq(3);
                    else if(AudioBufOffset > 1)	
                        AdjustIISFreq(5);
                    //-----------------------------------//
					else if(AudioPlayOffset > 9)
	                    AdjustIISFreq(5);
					else if(AudioPlayOffset > 8)
						AdjustIISFreq(7);
					else if( AudioPlayOffset > 7 )
					    AdjustIISFreq(7);	
                    else if(AudioPlayOffset < 2)
                        AdjustIISFreq(10);
                    else if(AudioPlayOffset < 3)
                        AdjustIISFreq(10);
                    else if(AudioPlayOffset < 4)
                        AdjustIISFreq(10);
                    else if(AudioPlayOffset < 5)
                        AdjustIISFreq(10);
					else
						AdjustIISFreq(10); // 5,6,7
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

                    if((AudioBufOffset > audioQueueDepth))
	                   rfiuRxIIsSounBufMngReadIdx[RFUnit] = (rfiuRxIIsSounBufMngReadIdx[RFUnit]+2) % IIS_BUF_NUM;
                    else    
	                   rfiuRxIIsSounBufMngReadIdx[RFUnit] = (rfiuRxIIsSounBufMngReadIdx[RFUnit]+1) % IIS_BUF_NUM;
	            #if RFIU_RX_AVSYNC
	                if(First_Audio)
	                {
	                   rfiuRxMainAudioPlayStart=1;
                       if( (sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR) || (sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR) )
                       {
                        #if IIS1_REPLACE_IIS5
                         #if (Audio_mode == AUDIO_AUTO)
	                      iis1SetNextPlayDMA_auto((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                         #else
	                      iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                         #endif
	                      iisStartPlay(0);
                        #else
                         #if (Audio_mode == AUDIO_AUTO)
	                      iis5SetNextPlayDMA_auto((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                         #else
	                      iis5SetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                         #endif
	                      iisStartPlay(4);
                         #endif
                          DEBUG_RFIU_P2("--->RF Audio Start:%d!\n",RFUnit);
                       }
                    #if RFRX_QUAD_AUDIO_EN
                       else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
                       {
                        #if IIS1_REPLACE_IIS5
                         #if (Audio_mode == AUDIO_AUTO)
                          iis1SetNextPlayDMA_auto((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                         #else
                          iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                         #endif
                          iisStartPlay(0);
                        #else
                         #if (Audio_mode == AUDIO_AUTO)
                          iis5SetNextPlayDMA_auto((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                         #else
                          iis5SetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[RFUnit] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                         #endif
                          iisStartPlay(4);
                        #endif
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
    #if 0
        if( (VideoBufOffset != 0) && (VideoDispOffset == 0) )
          DEBUG_RFIU_P2("EP ");
    #endif      
        if( (VideoBufOffset>0) && ((SYNC==1) || (VideoDispOffset>=0)) )
        {
            pVideoBuf        = rfiuRxVideoBufMng[RFUnit][VideoReadIdx].buffer; 
            pVideoFlag       = &(rfiuRxVideoBufMng[RFUnit][VideoReadIdx].flag);    
            VideoDuration    = (u32)(rfiuRxVideoBufMng[RFUnit][VideoReadIdx].time); 
            pVideoSize       = &(rfiuRxVideoBufMng[RFUnit][VideoReadIdx].size);
            VideoOffset      = rfiuRxVideoBufMng[RFUnit][VideoReadIdx].offset;
            vop_type         = (*pVideoFlag)? 0 : 1;

	     if(vop_type == 0)
		 	last_I_size = *pVideoSize;
            
            if(VideoDuration >2000) //Lucian: 保護斷線時間 error.
                VideoDuration=66;


            if( (SYNC==1) && (*pVideoFlag == 0))
            {
                VideoReadIdx  = (VideoReadIdx + 1) % VIDEO_BUF_NUM;
                SyncTime += VideoDuration;
				RunVideo=1;
                //DEBUG_RFIU_P2("SyncTime=%d\n",SyncTime);
                DEBUG_RFIU_P2("S");
            }
            else
            {
                if(SYNC==1)
                {
                   osdDrawPlaybackArea(2);
                   //OSFlagPost(gRfiuStateFlagGrp, RFIU_RX_STA_FRAMESYNC<<(RFUnit*(32/MAX_RFIU_UNIT)), OS_FLAG_SET, &err);
                   VideoDuration=33;
                #if( (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
                   if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
                   {
                       if(sysTVOutOnFlag)
                       {
                            if(BRI_SCCTRL_MODE & 0x4)
                            {
                                iduTVColorbar_onoff(0); 
                                //iduTVOSDEnable(IDU_OSD_L0_WINDOW_0);
                            }
                       }
                   }
                #else
                   if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
                   {
                       if(sysTVOutOnFlag==0)
                       {
                            if(BRI_SCCTRL_MODE & 0x4)
                            {
                                iduTVColorbar_onoff(0); 
                            }
                       }
                   }
               
                #endif
                }
                SYNC = 0;

                //DEBUG_RFIU_P2("V%d>%d ",RFUnit,VideoDuration);
                //========== Single Display mode==========//
				if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
				{
				    if(sysRFRxInMainCHsel==RFUnit)
    				{
    				   if(VideoDispOffset >= 1 )
                          rfiuRxMainVideoPlayStart=1;
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)  
                       #if(RF_RX_FULLVIEW_RESO_SEL == RF_RX_RESO_VGA)
                       Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                        pVideoBuf, 
                                                        *pVideoSize, 
                                                        RFUnit,
                                                        VideoOffset,
                                                        RFIU_RX_DISP_MAIN_VGA,
                                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                       #elif(RF_RX_FULLVIEW_RESO_SEL == RF_RX_RESO_HD)
                       Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                        pVideoBuf, 
                                                        *pVideoSize, 
                                                        RFUnit,
                                                        VideoOffset,
                                                        RFIU_RX_DISP_MAIN_HD,
                                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                       #endif
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)    
                        video_info.StreamBuf = pVideoBuf;
                        video_info.pSize     = pVideoSize;            
                        video_info.poffset   = VideoOffset;
                        #if(RF_RX_FULLVIEW_RESO_SEL == RF_RX_RESO_VGA)
                        Vop_Result = rfiuH264Decode(&video_info,
                                                    &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_MAIN_VGA,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                    1);
                        #elif(RF_RX_FULLVIEW_RESO_SEL == RF_RX_RESO_HD)
                        Vop_Result = rfiuH264Decode(&video_info,
                                                    &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_MAIN_HD,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                    1);
                        #endif
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
                       //DEBUG_RFIU_P2("%d ",VideoDuration);
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
                 #if RX_DISPLAY_KEEP_BUFFERING
			  if(gRfiuUnitCntl[RFUnit].BitRate > 0)
			         smooth_letency = ((last_I_size*8*1000/1024)/(gRfiuUnitCntl[RFUnit].BitRate*100)); //size: kbit,  time: ms
			  else			  
		               smooth_letency = 100;

			   if(smooth_letency > 1000)
			   	smooth_letency = 1000;
			   if(smooth_letency < 100)
			   	smooth_letency = 100;

		         letency_interval = smooth_letency/3;

                       if(VideoLatency > smooth_letency + 500){
                           rfiuMainVideoTime +=500;
                       }
                       else if(VideoLatency > smooth_letency + 400){
                           rfiuMainVideoTime +=64;
                       }
                       else if(VideoLatency > smooth_letency + 300){
                           rfiuMainVideoTime +=32;
                       }
                       else if(VideoLatency > smooth_letency + 200){
                          rfiuMainVideoTime +=16;
                       }
                       else if(VideoLatency > smooth_letency + 100){
                          rfiuMainVideoTime +=8;
                       } 
		          else if(smooth_letency > 300){
	                       if(VideoLatency < (smooth_letency - 2*letency_interval)){
	                          rfiuMainVideoTime -=32;
	                       }
	                       else if(VideoLatency < (smooth_letency - letency_interval)){
	                          rfiuMainVideoTime -=16;
                       }                       
	                       else if(VideoLatency < smooth_letency){
	                          rfiuMainVideoTime -=8;
	                       }
		          }
                      
                 #else   
                       //==Refine Video speed==//
                       if(VideoLatency > 1000)
                           rfiuMainVideoTime +=500;
                       else if((VideoLatency > 600) )
                          rfiuMainVideoTime +=60;
                    #if RF_RX_SMOOTH_PLUS
                       else if( (VideoLatency > 500) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuMainVideoTime +=16;
                          VideoCount=0;
                       }
                       else if( (VideoLatency > 400) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuMainVideoTime +=8;
                          VideoCount=0;
                       }
                       else if(VideoLatency < 300)
                       {
                          //rfiuMainVideoTime -=2;
                       }
                    #else
                       else if( (VideoLatency > 250) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuMainVideoTime +=33;
                          VideoCount=0;
                       }
                       else if( (VideoLatency > 200) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuMainVideoTime +=16;
                          VideoCount=0;
                       } 
                       else if(VideoLatency < 100)
                       {
                          rfiuMainVideoTime -=4;
                       }

                    #endif
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
    				   DEBUG_RFIU_P2("V>%d,%2d:%d,%d\n",VideoDispOffset,VideoBufOffset,VideoLatency,VideoDuration);
    				#endif
                       if(gRfiuUnitCntl[RFUnit].TX_PicWidth > 1280)
                          VideoCount +=4;
                       else
                          VideoCount++;
    				   RunVideo=1;
    				}
                    else if( (sysRFRxInPIPCHsel==RFUnit) && sysRFRXPIP_en ) //Single view and PIP
                    {
    	               rfiuRxSub1VideoPlayStart=1;
    				   rfiuVideoBufPlay_idx[RFUnit] = rfiuVideoBufFill_idx[RFUnit];
    				
                       if(rfiuSub1VideoTime >= rfiuVideoTimeBase[RFUnit])
                       {					  
                        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)    
                          Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                            pVideoBuf, 
                                                            *pVideoSize, 
                                                            RFUnit,
                                                            VideoOffset,
                                                            RFIU_RX_DISP_PIP,
                                                            (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                        #elif (VIDEO_CODEC_OPTION == H264_CODEC)    
                          #if SB_DECORD_SUPPORT
                            if((gRfiuUnitCntl[RFUnit].RFpara.TX_SubStreamSupport == 1) )
                            {
                                if((*pVideoSize-VideoOffset) != 0)
                                {
                                    video_info_sub.StreamBuf = pVideoBuf+VideoOffset;
                                    video_info_sub.pSize     = pVideoSize; 
                                    video_info_sub.poffset   = VideoOffset;
                                    Vop_Result = rfiuH264Decode(&video_info_sub,
                                                                &H264Dec_sub,
                                                            pVideoBuf, 
                                                            *pVideoSize, 
                                                            RFUnit,
                                                            VideoOffset,
                                                            RFIU_RX_DISP_PIP,
                                                            (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                            1);
                                }
                            }
                          #endif
                        #endif
                          //DEBUG_RFIU_P2("P");
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
    					  {
    					   	rfiuVideoTimeBase[RFUnit] += (VideoDuration-20);
                            DEBUG_RFIU_P2("V%d>%d,%d\n",RFUnit,VideoBufOffset,VideoDuration);
    					  }
                          else if(VideoBufOffset > 10)
    					   	rfiuVideoTimeBase[RFUnit] += (VideoDuration-10);
    					  else if(VideoBufOffset > 4)
    	                    rfiuVideoTimeBase[RFUnit] += (VideoDuration-3);
    					  else if(VideoBufOffset > 1)
    	                    rfiuVideoTimeBase[RFUnit] += (VideoDuration-2);
    					  else
    					   	rfiuVideoTimeBase[RFUnit] += (VideoDuration-0);

                          if( (rfiuSub1VideoTime > 0x0fffffff) )
                          {
    						  rfiuSub1VideoTime =0;
    						  for(i=0;i<MAX_RFIU_UNIT;i++)
    						    rfiuVideoTimeBase[i]=rfiuSub1VideoTime;
                          }
    					  RunVideo=1;
    				   }
    				   else
    				   {
                          RunVideo=0;
    				   }				   
    				}
				}
                //========== Quad Display mode==========//
				else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_QUADSCR)
				{
	               rfiuRxMainVideoPlayStart=1;
				   rfiuVideoBufPlay_idx[RFUnit] = rfiuVideoBufFill_idx[RFUnit];
                   //DEBUG_RFIU_P2("V%d>%d ",RFUnit,VideoDuration);
                   timerCountRead(guiRFTimerID, &t3);
                   if(t4>t3)
                      dt=t4-t3;
                   else
                      dt=(t3+TimerGetTimerCounter(TIMER_7))-t4;

                   if(dt==0)
                      dt=1;
                   
                   if( (rfiuMainVideoTime >= rfiuVideoTimeBase[RFUnit]) )
                   {

                      #if SB_DECORD_SUPPORT
                        if((gRfiuUnitCntl[RFUnit].RFpara.TX_SubStreamSupport == 0) )
                        {
                            video_info.StreamBuf = pVideoBuf;
                            video_info.pSize     = pVideoSize;            
                            video_info.poffset   = VideoOffset;
                            #if(VIDEO_CODEC_OPTION == H264_CODEC)  
                            Vop_Result = rfiuH264Decode(&video_info,
                                                        &H264Dec,
                                                        pVideoBuf, 
                                                        *pVideoSize, 
                                                        RFUnit,
                                                        VideoOffset,
                                                        RFIU_RX_DISP_QUARD_HD,
                                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                        1);
                           #endif
                        }
                        else
                        {
							#if CHECK_VIDEO_BITSTREAM
							if((*pVideoSize-VideoOffset) != 4) //End_code
                            {
                           #if(VIDEO_CODEC_OPTION == H264_CODEC)                             
                                video_info_sub.StreamBuf = pVideoBuf+VideoOffset;
                                video_info_sub.pSize     = pVideoSize; 
                                video_info_sub.poffset   = VideoOffset;
                                Vop_Result = rfiuH264Decode(&video_info_sub,
                                                            &H264Dec_sub,
                                                            pVideoBuf, 
                                                            (*pVideoSize-VideoOffset), 
                                                            RFUnit,
                                                            VideoOffset,
                                                            RFIU_RX_DISP_QUARD_HD,
                                                            (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                            1);
                           #endif
                            }
							#else
                            if((*pVideoSize-VideoOffset) != 0)
                            {
                           #if(VIDEO_CODEC_OPTION == H264_CODEC) 
                                video_info_sub.StreamBuf = pVideoBuf+VideoOffset;
                                video_info_sub.pSize     = pVideoSize; 
                                video_info_sub.poffset   = VideoOffset;
                                Vop_Result = rfiuH264Decode(&video_info_sub,
                                                            &H264Dec_sub,
                                                            pVideoBuf, 
                                                            (*pVideoSize-VideoOffset), 
                                                            RFUnit,
                                                            VideoOffset,
                                                            RFIU_RX_DISP_QUARD_HD,
                                                            (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                            1);
                           #endif
                            }
							#endif
                            else
                            {
                                DEBUG_RFIU_P2("No substream!\n");
                                NoSubStreamCnt ++;
                                if(NoSubStreamCnt>60)
                                {
                                    NoSubStreamCnt=0;
                                    sysback_RF_SetEvt(SYS_BACKRF_RFI_RX_SETOPMODE, RFUnit);
                                }
                            }
                        }
                      #else
                        video_info.StreamBuf = pVideoBuf;
                        video_info.pSize     = pVideoSize;            
                        video_info.poffset   = VideoOffset;
                        #if(VIDEO_CODEC_OPTION == H264_CODEC) 
                        Vop_Result = rfiuH264Decode(&video_info,
                                                    &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_QUARD_HD,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                    1);
                      #endif
                      #endif
                      //DEBUG_RFIU_P2("F ");
                      timerCountRead(guiRFTimerID, &t4);
                      
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
                           DEBUG_RFIU_P2("---->MpegErrCnt=%d\n",MpegErrCnt);
                      }
                      else
                           MpegErrCnt=0;

                      if(VideoBufOffset>20)
                          DEBUG_RFIU_P2("V%d>%d,%d,%d,%d\n",RFUnit,VideoBufOffset,VideoDuration,rfiuMainVideoTime-rfiuVideoTimeBase[RFUnit],dt);
                      
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
                else if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_MASKAREA)
				{
	               rfiuRxMainVideoPlayStart=1;
				   rfiuVideoBufPlay_idx[RFUnit] = rfiuVideoBufFill_idx[RFUnit];
				
                   if( (rfiuMainVideoTime >= rfiuVideoTimeBase[RFUnit]) )
                   {
                      #if SB_DECORD_SUPPORT
                            if((gRfiuUnitCntl[RFUnit].RFpara.TX_SubStreamSupport == 1) )
                            {
                                if((*pVideoSize-VideoOffset) != 0)
                                {
                                    #if(VIDEO_CODEC_OPTION == H264_CODEC) 
                                    video_info_sub.StreamBuf = pVideoBuf+VideoOffset;
                                    video_info_sub.pSize     = pVideoSize; 
                                    video_info_sub.poffset   = VideoOffset;
                                    Vop_Result = rfiuH264Decode(&video_info_sub,
                                                                &H264Dec_sub,
                                                                pVideoBuf, 
                                                                *pVideoSize, 
                                                                RFUnit,
                                                                VideoOffset,
                                                                RFIU_RX_DISP_MASK,
                                                                (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                                1);
                                    #endif
                                }
                            }
                      #else
                        video_info.StreamBuf = pVideoBuf;
                        video_info.pSize     = pVideoSize;            
                        video_info.poffset   = VideoOffset;
                        #if(VIDEO_CODEC_OPTION == H264_CODEC) 
                            Vop_Result = rfiuH264Decode(&video_info,
                                                        &H264Dec,
                                                        pVideoBuf, 
                                                        *pVideoSize, 
                                                        RFUnit,
                                                        VideoOffset,
                                                        RFIU_RX_DISP_MASK,
                                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                        1);
                      #endif
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
                       #if(RF_RX_FULLVIEW_RESO_SEL == RF_RX_RESO_VGA)
                       Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                        pVideoBuf, 
                                                        *pVideoSize, 
                                                        RFUnit,
                                                        VideoOffset,
                                                        RFIU_RX_DISP_MAIN_VGA,
                                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                       #elif(RF_RX_FULLVIEW_RESO_SEL == RF_RX_RESO_HD)
                       Vop_Result  = rfiuMpeg4DecodeVOP(&mpeg4Dec,
                                                        pVideoBuf, 
                                                        *pVideoSize, 
                                                        RFUnit,
                                                        VideoOffset,
                                                        RFIU_RX_DISP_MAIN_HD,
                                                        (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC));
                       #endif
                    #elif (VIDEO_CODEC_OPTION == H264_CODEC)    
                        video_info.StreamBuf = pVideoBuf;
                        video_info.pSize     = pVideoSize;
                        video_info.poffset   = VideoOffset;
                       #if(RF_RX_FULLVIEW_RESO_SEL == RF_RX_RESO_VGA)
                       #if(VIDEO_CODEC_OPTION == H264_CODEC) 
                        Vop_Result = rfiuH264Decode(&video_info,
                                                    &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_MAIN_VGA,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                    1);
                       #endif
                       #elif(RF_RX_FULLVIEW_RESO_SEL == RF_RX_RESO_HD)
                       #if(VIDEO_CODEC_OPTION == H264_CODEC) 
                        Vop_Result = rfiuH264Decode(&video_info,
                                                    &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_MAIN_HD,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                    1);
                       #endif
                       #endif
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
                       else if( (VideoLatency > 450) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuMainVideoTime +=16;
                          VideoCount=0;
                       }
                       else if( (VideoLatency > 350) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuMainVideoTime +=8;
                          VideoCount=0;
                       }
                    #else
                       else if( (VideoLatency > 200) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuMainVideoTime +=33;
                          VideoCount=0;
                       }
                       else if( (VideoLatency > 166) && (VideoDispOffset>1) && (VideoCount>8) )
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
                      if(gRfiuUnitCntl[RFUnit].TX_PicWidth > 1280)
                        VideoCount +=4;
                      else
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
                    video_info.StreamBuf = pVideoBuf;
                    video_info.pSize     = pVideoSize;            
                    video_info.poffset   = VideoOffset;
                    #if(VIDEO_CODEC_OPTION == H264_CODEC) 
                        Vop_Result = rfiuH264Decode(&video_info,
                                                    &H264Dec,
                                                    pVideoBuf, 
                                                    *pVideoSize, 
                                                    RFUnit,
                                                    VideoOffset,
                                                    RFIU_RX_DISP_SUB1,
                                                    (gRfiuUnitCntl[RFUnit].TX_Status & RFIU_TX_STA__FIELD_ENC),
                                                    1);
                    #endif
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
                       else if( (VideoLatency > 200) && (VideoDispOffset>1) && (VideoCount>8) )
                       {
                          rfiuSub1VideoTime +=33;
                          VideoCount=0;
                       }
                       else if( (VideoLatency > 166) && (VideoDispOffset>1) && (VideoCount>8) )
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
                       if(gRfiuUnitCntl[RFUnit].TX_PicWidth > 1280)
                         VideoCount +=4;
                       else
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

                   DEBUG_RFIU_P("RF-%d FrameRate=%d\n",RFUnit,FrameRate);
                   //gRfiuUnitCntl[RFUnit].FrameRate=FrameRate;

                   if(sysCameraMode == SYS_CAMERA_MODE_RF_RX_FULLSCR)
                      sysbackSetEvt(SYS_BACK_DRAW_FRAME_RATE, FrameRate);       
                   else if((sysCameraMode == SYS_CAMERA_MODE_RF_RX_DUALSCR) && (sysRFRxInMainCHsel==RFUnit) )
                      sysbackSetEvt(SYS_BACK_DRAW_FRAME_RATE, FrameRate);

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
   iis5SetNextPlayDMA(rfiuAudioRetDMANextBuf[(rfiuAudioRetRec_idx-2) % RFI_AUDIO_RET_BUF_NUM], 1024);
   iisStartPlay(4);
   rfiuMainAudioTime += 64;
   rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] +=1;
}
#else
#if IIS1_REPLACE_IIS5
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
		     if( (rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM] + 32*4) )
		     {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] += 2;
				 rfiuMainAudioTime += 32;
		     }
			 else if(rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM])
			 {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] +=1;
				 rfiuMainAudioTime += 32;
			 }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,rfiuAudioZeroBuf);
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
				 isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] ++;
				 rfiuMainAudioTime += 32;
             }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
			     isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,rfiuAudioZeroBuf);
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
             isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,rfiuAudioZeroBuf);
            #else
			 iisSetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
            #endif
             //DEBUG_RFIU_P2("Z");
          }
          #if(Audio_mode != AUDIO_AUTO)
          iisStartPlay(0);
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
    			 isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] += 2;
				 rfiuMainAudioTime += 32;
		     }
			 else if(rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM])
			 {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] +=1;
				 rfiuMainAudioTime += 32;
			 }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,rfiuAudioZeroBuf);
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
    			 isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iisSetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] ++;
				 rfiuMainAudioTime += 32;
             }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,rfiuAudioZeroBuf);
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
     		 isr_marsDMA_PlayAuto(guiIISCh0PlayDMAId,rfiuAudioZeroBuf);
            #else
			 iisSetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
            #endif
          }
          #if(Audio_mode != AUDIO_AUTO)
          iisStartPlay(0);
          #endif
      }
    }
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
		     if( (rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM] + 32*4) )
		     {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iis5SetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] += 2;
				 rfiuMainAudioTime += 32;
		     }
			 else if(rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM])
			 {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iis5SetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] +=1;
				 rfiuMainAudioTime += 32;
			 }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,rfiuAudioZeroBuf);
                #else			 
                 iis5SetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
                #endif
				 rfiuMainAudioTime += 32;
			 }
		  }
		  else if(AudioPlayOffset > 1)
          {
             if(rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM])
             {
                #if (Audio_mode == AUDIO_AUTO)
				 isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iis5SetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] ++;
				 rfiuMainAudioTime += 32;
             }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
			     isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,rfiuAudioZeroBuf);
                #else			 
                 iis5SetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
                #endif
				 rfiuMainAudioTime += 32;
			 }
          }
          else
          {
             rfiuMainAudioTime = rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM];
             rfiuMainAudioTime_frac=0;
            #if (Audio_mode == AUDIO_AUTO)
             isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,rfiuAudioZeroBuf);
            #else
			 iis5SetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
            #endif
             //DEBUG_RFIU_P2("Z");
          }
          #if(Audio_mode != AUDIO_AUTO)
          iisStartPlay(4);
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
    			 isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iis5SetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] += 2;
				 rfiuMainAudioTime += 32;
		     }
			 else if(rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM])
			 {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iis5SetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] +=1;
				 rfiuMainAudioTime += 32;
			 }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,rfiuAudioZeroBuf);
                #else
                 iis5SetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
                #endif
				 rfiuMainAudioTime += 32;
			 }
		  }
		  else if(AudioPlayOffset > 1)
          {
             if(rfiuMainAudioTime >= rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM])
             {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,(u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM]);
                #else
				 iis5SetNextPlayDMA((u8*)rfiuMainAudioPlayDMANextBuf[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM], IIS_PLAYBACK_SIZE);
                #endif
                 rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] ++;
				 rfiuMainAudioTime += 32;
             }
			 else
			 {
                #if (Audio_mode == AUDIO_AUTO)
    			 isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,rfiuAudioZeroBuf);
                #else
                 iis5SetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
                #endif
				 rfiuMainAudioTime += 32;
			 }
          }
          else
          {
             rfiuMainAudioTime = rfiuMainAudioPresentTime[rfiuAudioBufPlay_idx[sysRFRxInMainCHsel] % IISPLAY_BUF_NUM];
             rfiuMainAudioTime_frac=0;
            #if (Audio_mode == AUDIO_AUTO)
     		 isr_marsDMA_PlayAuto(guiIISCh4PlayDMAId,rfiuAudioZeroBuf);
            #else
			 iis5SetNextPlayDMA(rfiuAudioZeroBuf, IIS_PLAYBACK_SIZE);
            #endif
          }
          #if(Audio_mode != AUDIO_AUTO)
          iisStartPlay(4);
          #endif
      }
    }
}
#endif
#endif

void rfiuAudioRet_RecDMA_ISR(int dummy)
{   
    if(rfiuAudioRetFromApp)
    {
    #if (Audio_mode == AUDIO_AUTO)
       iisSetNextRecDMA_auto((u8*)rfiuAudioDummyBuf, 1024,sysRFRxInMainCHsel);
    #else
       iisSetNextRecDMA((u8*)rfiuAudioDummyBuf, 1024);
    #endif
       iisStartRec();
    }
    else
    {
    #if PWIFI_SUPPORT
        //DEBUG_PWIFI("%d ",rfiuAudioRetRec_idx);
        if( pWifiCalBufRemainCount(rfiuAudioRetRec_idx,rfiuAudioRetRead_idx,PWIFI_AUDRET_PKTMAP_SIZE) >= 0 )
        {
            rfiuAudioRetRec_idx = (rfiuAudioRetRec_idx+1) % RFI_AUDIO_RET_BUF_NUM;
        }
    #else
        rfiuAudioRetRec_idx = (rfiuAudioRetRec_idx+1) % RFI_AUDIO_RET_BUF_NUM;
    #endif
    
    #if (Audio_mode == AUDIO_AUTO)
        iisSetNextRecDMA_auto((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx], 1024,sysRFRxInMainCHsel);
    #else
        iisSetNextRecDMA((u8*)rfiuAudioRetDMANextBuf[rfiuAudioRetRec_idx], 1024);
    #endif
    	iisStartRec();
        
    #if RFIU_RX_AUDIO_RETURN   
      #if PWIFI_SUPPORT
        gRfiuUnitCntl[sysRFRxInMainCHsel].PwifiAudioRetPktMap[rfiuAudioRetRec_idx/32] |= (0x01 << (rfiuAudioRetRec_idx & (32-1)) );
      #else
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
    #endif
    }
}

void pWifi_WrapTx_Task_UnitX(void* pData)
{

}


#if PWIFI_SUPPORT
void pWifi_WrapRx_Task_UnitX(void* pData)
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
    int IsVideoFrame;
    unsigned int t1,t2,dt;
    int count=0;

    //-------------------//
    RFUnit= (int)pData;
	PsHeader.PreStatus = 0;	
    
#if (UI_BAT_SUPPORT && RFIU_RX_WAKEUP_TX_SCHEME) 
    if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && (gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode==0) )
    {
        if( 0 == sysGetBTCWakeStatus(RFUnit) )    
        {
             DEBUG_RFIU_P2("RF False WakeUp! TX goto sleep\n");
             count=0;
             while( 0 == rfiuCamSleepCmd(RFUnit) )
             {
                 OSTimeDly(10);
                 count ++;
                 if(count > 10)
                 {
                    DEBUG_RFIU_P2("Warning! Sleep comand Fail!!\n");
                    break;
                 }   
             }
             rfiuBatCam_LiveMaxTime[RFUnit]=RF_BATCAM_LIVE_MAXTIME * 10000;
             if(count <= 10)
             {
                gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=1;
                while(gRfiuUnitCntl[RFUnit].RX_Wrap_Stop)
                {
                   //DEBUG_RFIU_P2("*");
                   OSTimeDly(1);
                   continue;
                }
             }   
        }
    }
#endif   
	
    OS_ENTER_CRITICAL();
    rfiuRxVideoBufMngWriteIdx[RFUnit]=0;
    rfiuRxIIsSounBufMngWriteIdx[RFUnit]=0;
    
    SyncFlag=0;
    
    VideoReadIdx = (rfiuRxVideoBufMngWriteIdx[RFUnit]) % VIDEO_BUF_NUM;
    
    BufWriteAddrOffset=0;
    BufReadAddrOffset=0;
    BufReadAddrOffset= (gRfiuUnitCntl[RFUnit].BufReadPtr<<10);    

    OS_EXIT_CRITICAL();

#if(RFI_TEST_RX_PROTOCOL_B1 || RFI_TEST_RX_PROTOCOL_B2 || RFI_TEST_RXRX_PROTOCOL_B1B2 || RFI_TEST_2x_RX_PROTOCOL_B1 || RFI_TEST_4x_RX_PROTOCOL_B1 || RFI_TEST_4TX_2RX_PROTOCOL || RFI_TEST_8TX_2RX_PROTOCOL || RFI_TEST_8TX_1RX_PROTOCOL )
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

#if RFIU_RX_WAKEUP_TX_SCHEME
    rfiuBatCamVideo_TotalTime[RFUnit]=0;
    gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime=rfiuBatCam_PIRRecDurationTime;
#endif
    gRfiuUnitCntl[RFUnit].RFpara.PIR_TrigEvent=0;  //clear trigger event.
    gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=0;

 #if TUTK_SUPPORT 
    OSSemSet(P2PVideoCmpSemEvt[RFUnit],0, &err);  
    OSSemSet(P2PAudioCmpSemEvt[RFUnit],0, &err); 

    P2PVideoBufReadIdx[RFUnit]  = rfiuRxVideoBufMngWriteIdx[RFUnit];; 
    P2PAudioBufReadIdx[RFUnit]  = rfiuRxIIsSounBufMngWriteIdx[RFUnit];
    
    if (err != OS_NO_ERR) {
        DEBUG_RFIU_P2("OSSemSet Error: P2PVideoCmpSemEvt is %d.\n", err);
    }
 #endif

 #if USB_DONGLE_SUPPORT
    OSSemSet(USBVideoCmpSemEvt[RFUnit],0, &err);  
    OSSemSet(USBAudioCmpSemEvt[RFUnit],0, &err); 

    USBVideoBufReadIdx[RFUnit]  = rfiuRxVideoBufMngWriteIdx[RFUnit];; 
    USBAudioBufReadIdx[RFUnit]  = rfiuRxIIsSounBufMngWriteIdx[RFUnit];
    
    if (err != OS_NO_ERR) {
        DEBUG_RFIU_P2("OSSemSet Error: USBVideoCmpSemEvt is %d.\n", err);
    }
 #endif

    VideoFrameCnt=0;
    FrameRate=0;
    timerCountRead(guiRFTimerID, &t1);
    gRfiuUnitCntl[RFUnit].FrameRate=0;
    DEBUG_RFIU_P2("\n============pWifi_WrapRx_Task_UnitX(%d)==========\n",RFUnit);  

	while(1)
    {
        if(gRfiuUnitCntl[RFUnit].RX_Wrap_Stop)
        {
           //DEBUG_RFIU_P2("*");
           if(gRfiuUnitCntl[RFUnit].RX_Wrap_Stop == 1)
              gRfiuUnitCntl[RFUnit].RX_Wrap_Stop=2;
           
           OSTimeDly(1);
           continue;
        }
            
#if MULTI_CHANNEL_RF_RX_VIDEO_REC
    	if(RFUnit < (MULTI_CHANNEL_MAX - MULTI_CHANNEL_LOCAL_MAX))
        {
            VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX].asfVopWidth    = (u16)gRfiuUnitCntl[RFUnit].TX_PicWidth;
            if(gRfiuUnitCntl[RFUnit].TX_PicHeight == 1088)
                VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = 1080;
            else
                VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX].asfVopHeight   = (u16)gRfiuUnitCntl[RFUnit].TX_PicHeight;
    	    RfRxVideoPackerSubTaskCreate(RFUnit, 
    	                                 &VideoClipParameter[RFUnit + MULTI_CHANNEL_LOCAL_MAX]);
        }
#endif

#if RFIU_RX_WAKEUP_TX_SCHEME
        if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode)
        {
             if(
                    (rfiuBatCamVideo_TotalTime[RFUnit] > (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime+1) * 1000) ||
                    (rfiuBatCamVideo_TotalTime[RFUnit] > RF_BATCAM_LIVE_MAXTIME * 1000)
                )
             {
                 DEBUG_RFIU_P2("\n--BATCAM Recording timeup!(%d,%d)--\n",RFUnit,rfiuBatCamVideo_TotalTime[RFUnit]);
                 if( 1 == rfiuCamSleepCmd(RFUnit) )
                 {
                    rfiuBatCamVideo_TotalTime[RFUnit]=0;
                 }
                 else
                 {
                    rfiuBatCamVideo_TotalTime[RFUnit] -= 2*1000;
                    gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=1;
                 }
             }
        #if(BTC_PIRREC_MODESEL == BTC_PIRRECMODE_ARLO)    
             else if( 
                        (rfiuBatCamVideo_TotalTime[RFUnit] > (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO-1) * 1000) &&
                        (rfiuBatCamVideo_TotalTime[RFUnit] < (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO) * 1000)
                    )
             {
                 gRfiuUnitCntl[RFUnit].RFpara.PIR_TrigEvent=0;
                 gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=0;
             }
             else if(rfiuBatCamVideo_TotalTime[RFUnit] > (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO) * 1000)
             {
                 if(gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn == 0)
                 {
                     if(gRfiuUnitCntl[RFUnit].RFpara.PIR_TrigEvent)
                     {
                         gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime +=PIRREC_TIMEEXTENDSEC_ARLO;
                         gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=1;
                         DEBUG_RFIU_P2("\n--PIR REC Time Extention!(%d,%d,%d)--\n",RFUnit,gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime,rfiuBatCamVideo_TotalTime[RFUnit]/1000);
                     }
                 }
             }
        #elif(BTC_PIRREC_MODESEL == BTC_PIRRECMODE_LOREX)
             else if(rfiuBatCam_LiveMaxTime[RFUnit] > rfiuBatCam_PIRRecDurationTime*5/2 * 10000)
             {
                 DEBUG_RFIU_P2("\n--BATCAM Recording Over Max Time!(%d,%d)--\n",RFUnit,rfiuBatCam_LiveMaxTime[RFUnit]);
                 if(1 == rfiuCamSleepCmd(RFUnit))
                 {
                    rfiuBatCam_LiveMaxTime[RFUnit]=0;
                 }
                 else
                 {
                    rfiuBatCam_LiveMaxTime[RFUnit] -= (2*10000);
                 }
             }
        #elif(BTC_PIRREC_MODESEL == BTC_PIRRECMODE_DOORBELL)
             else if( 
                        (rfiuBatCamVideo_TotalTime[RFUnit] > (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO-1) * 1000) &&
                        (rfiuBatCamVideo_TotalTime[RFUnit] < (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO) * 1000)
                    )
             {
                 gRfiuUnitCntl[RFUnit].RFpara.PIR_TrigEvent=0;
                 gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=0;
             }
             else if(rfiuBatCamVideo_TotalTime[RFUnit] > (gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime-PIRREC_TIMEEXTENDSEC_ARLO) * 1000)
             {
                 if(gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn == 0)
                 {
                   #if ENABLE_DOOR_BELL
                     if(sysGetDoorBellState() == SYS_DOOR_TALK)
                     {
                         gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime +=PIRREC_TIMEEXTENDSEC_ARLO;
                         gRfiuUnitCntl[RFUnit].RFpara.PIR_RecTimeExtendOn=1;
                         DEBUG_RFIU_P2("\n--Doorbell REC Time Extention!(%d,%d,%d)--\n",RFUnit,gRfiuUnitCntl[RFUnit].RFpara.PIR_RecDurationTime,rfiuBatCamVideo_TotalTime[RFUnit]/1000);
                     }
                   #endif
                 }
             }
        #endif    
        }

        if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support)
        {
             if(rfiuBatCam_LiveMaxTime[RFUnit] > RF_BATCAM_LIVE_MAXTIME * 10000)
             {
                 DEBUG_RFIU_P2("\n--BATCAM LiveView timeup!(%d,%d)--\n",RFUnit,rfiuBatCam_LiveMaxTime[RFUnit]);
                 if(1 == rfiuCamSleepCmd(RFUnit) )
                 {
                    rfiuBatCam_LiveMaxTime[RFUnit]=0;
                    rfiuBatCamVideo_TotalTime[RFUnit]=0;
                    gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode=0;
                 }
                 else
                 {
                    rfiuBatCam_LiveMaxTime[RFUnit] -= (2*10000);
                 }
             }

        }

#endif

        OS_ENTER_CRITICAL();
        BufWriteAddrOffset=(gRfiuUnitCntl[RFUnit].BufWritePtr<<10);
        OS_EXIT_CRITICAL();
        RemainBufSize=(BufWriteAddrOffset+PWIFI_OPERBUF_SIZE-BufReadAddrOffset) % PWIFI_OPERBUF_SIZE;   

        if(RemainBufSize > PWIFI_OPERBUF_SIZE/2)
        {
        #if RFIU_RX_WAKEUP_TX_SCHEME
            if(gRfiuUnitCntl[RFUnit].RFpara.BateryCam_support && gRfiuUnitCntl[RFUnit].RFpara.BatCam_PIRMode)
            {
                if(RemainBufSize > PWIFI_OPERBUF_SIZE*4/5)
                {
                    DEBUG_RFIU_P2("--->WrapRX too slow! RemainBufSize=%d\n",RemainBufSize);
        			OS_ENTER_CRITICAL();
                    BufReadAddrOffset= (gRfiuUnitCntl[RFUnit].BufWritePtr<<10); 
                    OS_EXIT_CRITICAL();
        			continue;
                }  
            }
            else
        #endif        
            {
                DEBUG_RFIU_P2("--->WrapRX too slow! RemainBufSize=%d\n",RemainBufSize);
    			OS_ENTER_CRITICAL();
                BufReadAddrOffset= (gRfiuUnitCntl[RFUnit].BufWritePtr<<10); 
                OS_EXIT_CRITICAL();
    			continue;
            }    
        }

        if(SyncFlag <= 0)
        {
           SyncFlag=SyncPSHeader_RF(RFUnit,&BufReadAddrOffset,RemainBufSize);
        }
        
        if(SyncFlag > 0)
        {
            RemainBufSize=(BufWriteAddrOffset+PWIFI_OPERBUF_SIZE-BufReadAddrOffset) % PWIFI_OPERBUF_SIZE;
            if(RemainBufSize >= PS_MAX_HEADERLEN ) 
            {
                 ParsePSHeader_RF(RFUnit,BufReadAddrOffset,&PsHeader);

                 //Lucian: Stream type bit[7:6] is used for checking 00
                 if( (PsHeader.PayLoadSize > (PWIFI_OPERBUF_SIZE/2)) || (PsHeader.PayLoadSize & 0x03) || (PsHeader.StreamType& 0xc0) )
                 {
                     DEBUG_RFIU_P2("Warning! Pack Size not Valid:%d\n",PsHeader.PayLoadSize);
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
                        VideoFrameCnt +=IsVideoFrame;
                    #endif
                        
                        BufReadAddrOffset += PackSize;
                        if(BufReadAddrOffset>=PWIFI_OPERBUF_SIZE)
                        {
                            if(BufReadAddrOffset!=PWIFI_OPERBUF_SIZE)
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

                           DEBUG_PWIFI("\nRF-%d FrameRate=%d\n",RFUnit,FrameRate);
                           gRfiuUnitCntl[RFUnit].FrameRate=FrameRate;  
                           t1=t2;
                           VideoFrameCnt=0;
                        }		
                        #if 0 //RF_LETENCY_DEBUG_ENA
                           if(sysRFRxInMainCHsel==RFUnit)
        				      DEBUG_RFIU_P2("WrapBuf=%d\n",RemainBufSize >> 10);
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
#endif

#if RFIU_RX_WAKEUP_TX_SCHEME
void rfiuResetBatCamLiveMaxTime(u8 RFUnit)
{   
    rfiuBatCam_LiveMaxTime[RFUnit] = 0;
}
#endif

#endif






















    
