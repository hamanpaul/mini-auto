
/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    iis.c

Abstract:

    The routines of IIS.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "board.h"

#include "task.h"
#include "iis.h"
#include "iisreg.h"
#include "iisapi.h"
#include <../inc/mars_controller/mars_dma.h>
#include "mp4api.h"
#include "asfapi.h"
#include "sysapi.h"
#include "aviapi.h"
#include "mpeg4api.h"
#include "sysapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "wavlib.h"
#include "adcapi.h"
#include "osapi.h"
#include "timerapi.h"
#include "uiapi.h"
#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
#include "ima_adpcm_api.h"
#endif

#if RF_TX_OPTIMIZE
#include "rfiuapi.h"
#endif


/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

#define SHOW_DEBUG_MSG_AC97    1
/* define debug print */
//#define iisDebugPrint             printf

/*Peter 1109 S*/
/* IIS time out value */
#define IIS_TIMEOUT         30      //civic 070829 10 --> 30

/* chunk time */
/* yc: 2006.07.31: S */
/* max buffer size */
//#define IIS_MAX_BUF_SIZE      0x00000800  /* cytsai: 0418 */
#define IIS_MAX_BUF_SIZE        2000    /* 800 bytes */

//#define IIS_CHUNK_TIME            0x00000019
//#define IIS_CHUNK_TIME            0x00000064  /* 100ms */

/* fixed chunk size for non-compressed samples */
//#define IIS_CHUNK_SIZE            0x000000c8
//#define IIS_CHUNK_SIZE            1000    /* 800 bytes */
/* yc: 2006.07.31: E */
/*Peter 1109 E*/

#ifdef BIG_ENDIAN
#define host_to_le16(x) ((((x)&0xff)<<8)|(((x)&0xff00)>>8))
#define host_to_le32(x) (((x)>>24)|(((x)&0xff0000)>>8)|(((x)&0xff00)<<8)|((x)<<24))
#define le16_to_host(x) ((((x)&0xff)<<8)|(((x)&0xff00)>>8))
#define le32_to_host(x) (((x)>>24)|(((x)&0xff0000)>>8)|(((x)&0xff00)<<8)|((x)<<24))
#else
#define host_to_le16(x) (x)
#define host_to_le32(x) (x)
#define le16_to_host(x) (x)
#define le32_to_host(x) (x)
#endif

#define mask(a,b) ((a)&(~((b)-1)))

#if (HW_BOARD_OPTION == MR8120_TX_RDI_CA652)
#define DETECT_VOLUME   0 /*20160725 - Modity to close Vox FUNC*/
#else
#define DETECT_VOLUME   1
#endif

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
 #if(IIS_SAMPLE_RATE==8000)
WAVEFORMAT iisPlayFormat =
{
        0x0001,     /* wFormatTag */
        0x0001,     /* nChannels */
        0x00001f40, /* nSamplesPerSec */
        0x00001f40, /* nAvgBytesPerSec */
        0x0001,     /* nBlockAlign */
        0x0008      /* wBitsPerSample */
};

WAVEFORMAT iisRecFormat =
{
        0x0001,     /* wFormatTag */
        0x0001,     /* nChannels */
        0x00001f40, /* nSamplesPerSec */
        0x00001f40, /* nAvgBytesPerSec */
        0x0001,     /* nBlockAlign */
        0x0008      /* wBitsPerSample */
};
#elif(IIS_SAMPLE_RATE==16000)
WAVEFORMAT iisPlayFormat =
{
        0x0001,     /* wFormatTag */
        0x0001,     /* nChannels */
        0x00003e80, /* nSamplesPerSec */
        0x00003e80, /* nAvgBytesPerSec */
        0x0001,     /* nBlockAlign */
        0x0008      /* wBitsPerSample */
};

WAVEFORMAT iisRecFormat =
{
        0x0001,     /* wFormatTag */
        0x0001,     /* nChannels */
        0x00003e80, /* nSamplesPerSec */
        0x00003e80, /* nAvgBytesPerSec */
        0x0001,     /* nBlockAlign */
        0x0008      /* wBitsPerSample */
};
#endif
/* Civic 070829 S */

static char riff_magic[] = RIFF_MAGIC;
static char wave_magic[] = WAVE_MAGIC;
static char fmt_magic[]  = FMT_CHUNK_MAGIC;
static char data_magic[] = DATA_CHUNK_MAGIC;

/* Civic 070829 E */

/*Lucian 071019 S*/
const u8 SPKVolumnTable[11]={0x1f,0x11,0x0f,0x0d,
                             0x0b,0x09,0x07,0x05,
                             0x03,0x02,0x00
                       };

const u8 HPVolumnTable[11]={0x1f,0x1d,0x1b,0x19,
                            0x17,0x16,0x15,0x13,
                            0x11,0x09,0x07
                     };
/*Lucian 071019 E*/

/* task and event related */
OS_STK iisTaskStack[IIS_TASK_STACK_SIZE]; /* Stack of task iisTask() */
OS_EVENT* iisTrgSemEvt;
OS_EVENT* iisCmpSemEvt;
#if IIS_TEST
OS_STK iisCh0TaskStack[IIS_CH0_TASK_STACK_SIZE]; /* Stack of task iisTask() */
OS_STK iisCh1TaskStack[IIS_CH1_TASK_STACK_SIZE]; /* Stack of task iisTask() */
OS_EVENT* iisCh0TrgSemEvt;
OS_EVENT* iisCh1TrgSemEvt;
OS_EVENT* iisCh0CmpSemEvt;
OS_EVENT* iisCh1CmpSemEvt;
#endif
OS_EVENT* iisplayCmpEvt;    //civic 070829
OS_EVENT* AudioRTPCmpSemEvt[MULTI_CHANNEL_MAX];
#if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
OS_STK      iisPlaybackTaskStack[IIS_PLAYBACK_TASK_STACK_SIZE];
OS_EVENT*   iisPlaybackSemEvt;
u32         iisSounBufMngPlayIdx;
u8          iisPreviewI2ORunning;
#endif


/* buffer management */
u32 iisSounBufMngReadIdx;
u32 iisSounBufMngWriteIdx;
u32 iisBufferCmpCount;  //civic 070829
IIS_BUF_MNG iisSounBufMng[IIS_BUF_NUM];
IIS_BUF_MNG P2PiisSounBufMng[IIS_BUF_NUM]; //Toby 130815
u32 P2PiisSounBufMngWriteIdx;
u32 CurrentAudioSize;

u8 IISPplyback;
u32 IISMode=3;        // 0: record, 1: playback, 2: receive and playback audio in preview mode
s64 IISTime;        // Current IIS playback time(micro second)
#if FINE_TIME_STAMP
s32 IISTimeOffset;  // Current IIS playback time offset(micro second)
#endif
u32 IISTimeUnit;    // IIS playback time per DMA(micro second)
u32 iisPlayCount;   // IIS played chunk number
u32 iisTotalPlay;   // IIS total trigger playback number

#if IIS_DEBUG_ENA
u32 under_count;
u32 IIS_UnderRunSta;
#endif

u8 MuteRec = 0;
u8 Audio_formate    = nomo_8bit_16k;
//u8 iisDMABurst_ena=0;
//u8 iisDMABurst_Count;
//u8 iisDMABurst_CountMax;
//u8 *iisDMABurst_Addr;
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
    #if(ADC_IIS_CLK_FREQ == 24000000)
    /******************************************************************************************************************
    *** Sample rate = ADC_IIS_CLK_FREQ * (IIS_CLK_DIV_A/(IIS_CLK_DIV_A+1)) / (IIS_CLK_DIV_B+1) / 256                      ***
    *** ADC_Conv_RATE > 15
    ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
    u32 IIS_CLK_DIV_A[20] = {IIS_CLK_DIV_A_15    ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_2     ,IIS_CLK_DIV_A_16     ,IIS_CLK_DIV_A_1     ,
                             IIS_CLK_DIV_A_15    ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_2     ,IIS_CLK_DIV_A_16     ,IIS_CLK_DIV_A_1     ,
                             IIS_CLK_DIV_A_15    ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_2     ,IIS_CLK_DIV_A_16     ,IIS_CLK_DIV_A_1     ,
                             IIS_CLK_DIV_A_15    ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_2     ,IIS_CLK_DIV_A_16     ,IIS_CLK_DIV_A_1     };
    u32 IIS_CLK_DIV_B[20] = {IIS_CLK_DIV_B_10    ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_0     ,
                             IIS_CLK_DIV_B_10    ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_0     ,
                             IIS_CLK_DIV_B_10    ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_0     ,
                             IIS_CLK_DIV_B_10    ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_0     };

    #elif(ADC_IIS_CLK_FREQ == 32000000)
    /******************************************************************************************************************
    *** Sample rate = ADC_IIS_CLK_FREQ * (IIS_CLK_DIV_A/(IIS_CLK_DIV_A+1)) / (IIS_CLK_DIV_B+1) / 256                      ***
    *** ADC_Conv_RATE > 15
    ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
    u32 IIS_CLK_DIV_A[20] = {IIS_CLK_DIV_A_24    ,IIS_CLK_DIV_A_24     ,IIS_CLK_DIV_A_1     ,IIS_CLK_DIV_A_2     ,IIS_CLK_DIV_A_3     ,
                             IIS_CLK_DIV_A_24    ,IIS_CLK_DIV_A_24    ,IIS_CLK_DIV_A_1     ,IIS_CLK_DIV_A_2     ,IIS_CLK_DIV_A_3     ,
                             IIS_CLK_DIV_A_24    ,IIS_CLK_DIV_A_24     ,IIS_CLK_DIV_A_1     ,IIS_CLK_DIV_A_2     ,IIS_CLK_DIV_A_3     ,
                             IIS_CLK_DIV_A_24    ,IIS_CLK_DIV_A_24     ,IIS_CLK_DIV_A_1     ,IIS_CLK_DIV_A_2     ,IIS_CLK_DIV_A_3     };
    u32 IIS_CLK_DIV_B[20] = {IIS_CLK_DIV_B_14    ,IIS_CLK_DIV_B_3     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,
                             IIS_CLK_DIV_B_14    ,IIS_CLK_DIV_B_3     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,
                             IIS_CLK_DIV_B_14    ,IIS_CLK_DIV_B_3     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,
                             IIS_CLK_DIV_B_14    ,IIS_CLK_DIV_B_3     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     ,IIS_CLK_DIV_B_1     };
    #elif(ADC_IIS_CLK_FREQ == 48000000)
    /******************************************************************************************************************
    *** Sample rate = ADC_IIS_CLK_FREQ * (IIS_CLK_DIV_A/(IIS_CLK_DIV_A+1)) / (IIS_CLK_DIV_B+1) / 256                      ***
    *** ADC_Conv_RATE > 15
    ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
    u32 IIS_CLK_DIV_A[20] = {IIS_CLK_DIV_A_53    ,IIS_CLK_DIV_A_24     ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_16   ,IIS_CLK_DIV_A_1     ,
                             IIS_CLK_DIV_A_53    ,IIS_CLK_DIV_A_24     ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_16   ,IIS_CLK_DIV_A_1     ,
                             IIS_CLK_DIV_A_53    ,IIS_CLK_DIV_A_24     ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_16   ,IIS_CLK_DIV_A_1     ,
                             IIS_CLK_DIV_A_53    ,IIS_CLK_DIV_A_24     ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_16   ,IIS_CLK_DIV_A_1     };
    u32 IIS_CLK_DIV_B[20] = {IIS_CLK_DIV_B_22    ,IIS_CLK_DIV_B_5     ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_1     ,
                             IIS_CLK_DIV_B_22    ,IIS_CLK_DIV_B_5     ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_1     ,
                             IIS_CLK_DIV_B_22    ,IIS_CLK_DIV_B_5     ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_1     ,
                             IIS_CLK_DIV_B_22    ,IIS_CLK_DIV_B_5     ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_1     };
    #elif(ADC_IIS_CLK_FREQ == 54000000)
    /******************************************************************************************************************
    *** Sample rate = ADC_IIS_CLK_FREQ * (IIS_CLK_DIV_A/(IIS_CLK_DIV_A+1)) / (IIS_CLK_DIV_B+1) / 256                      ***
    *** ADC_Conv_RATE > 15
    ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
    u32 IIS_CLK_DIV_A[20] = {IIS_CLK_DIV_A_71    ,IIS_CLK_DIV_A_225   ,IIS_CLK_DIV_A_10    ,IIS_CLK_DIV_A_5    ,IIS_CLK_DIV_A_10     ,
                             IIS_CLK_DIV_A_71    ,IIS_CLK_DIV_A_225   ,IIS_CLK_DIV_A_10    ,IIS_CLK_DIV_A_5    ,IIS_CLK_DIV_A_10     ,
                             IIS_CLK_DIV_A_71    ,IIS_CLK_DIV_A_225   ,IIS_CLK_DIV_A_10    ,IIS_CLK_DIV_A_5    ,IIS_CLK_DIV_A_10     ,
                             IIS_CLK_DIV_A_71    ,IIS_CLK_DIV_A_225   ,IIS_CLK_DIV_A_10    ,IIS_CLK_DIV_A_5    ,IIS_CLK_DIV_A_10     };
    u32 IIS_CLK_DIV_B[20] = {IIS_CLK_DIV_B_25    ,IIS_CLK_DIV_B_6     ,IIS_CLK_DIV_B_5     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_3     ,
                             IIS_CLK_DIV_B_25    ,IIS_CLK_DIV_B_6     ,IIS_CLK_DIV_B_5     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_3     ,
                             IIS_CLK_DIV_B_25    ,IIS_CLK_DIV_B_6     ,IIS_CLK_DIV_B_5     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_3     ,
                             IIS_CLK_DIV_B_25    ,IIS_CLK_DIV_B_6     ,IIS_CLK_DIV_B_5     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_3     };

    #else
    /******************************************************************************************************************
    *** Sample rate = ADC_IIS_CLK_FREQ * (IIS_CLK_DIV_A/(IIS_CLK_DIV_A+1)) / (IIS_CLK_DIV_B+1) / 256                      ***
    *** ADC_Conv_RATE > 15
    ******************************************************************************************************************/
                            //8k                 ,16k                 ,32k                 ,44.1K               ,48K
    u32 IIS_CLK_DIV_A[20] = {IIS_CLK_DIV_A_53    ,IIS_CLK_DIV_A_15     ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_16   ,IIS_CLK_DIV_A_1     ,
                             IIS_CLK_DIV_A_53    ,IIS_CLK_DIV_A_15     ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_16   ,IIS_CLK_DIV_A_1     ,
                             IIS_CLK_DIV_A_53    ,IIS_CLK_DIV_A_15     ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_16   ,IIS_CLK_DIV_A_1     ,
                             IIS_CLK_DIV_A_53    ,IIS_CLK_DIV_A_15     ,IIS_CLK_DIV_A_6     ,IIS_CLK_DIV_A_16   ,IIS_CLK_DIV_A_1     };
    u32 IIS_CLK_DIV_B[20] = {IIS_CLK_DIV_B_22    ,IIS_CLK_DIV_B_10     ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_1     ,
                             IIS_CLK_DIV_B_22    ,IIS_CLK_DIV_B_10     ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_1     ,
                             IIS_CLK_DIV_B_22    ,IIS_CLK_DIV_B_10     ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_1     ,
                             IIS_CLK_DIV_B_22    ,IIS_CLK_DIV_B_10     ,IIS_CLK_DIV_B_4     ,IIS_CLK_DIV_B_3    ,IIS_CLK_DIV_B_1     };
    #endif
    u32 Rec_Time[20]= { 300, 600,1200,1653,1800,
                        600,1200,2400,3306,3600,
                        600,1200,2400,3306,3600,
                       1200,2400,4800,6612,7200};
#endif
//-----------Extern Variable---------//
extern FS_DISKFREE_T global_diskInfo; //civic 070829
extern OS_EVENT* general_MboxEvt;
#if(HW_BOARD_OPTION==SALIX_SDV)
  extern u8 sysTVOutOnFlag;
#endif
/*Peter 1109 S*/
extern  u32 g_intStat;

// for debug
extern u32  DMA0_Busy;
/* for display */
extern  s64 VideoNextPresentTime;
extern  u32 IsuIndex;
extern  s64 Videodisplaytime[DISPLAY_BUF_NUM];    /* Peter 070104 */
extern  s32 MicroSecPerFrame;
/*Peter 1109 E*/
extern  u8  sysPlaybackVideoStart;
extern  u8  sysPlaybackVideoStop;
extern  u8  sysPlaybackVideoPause;
//extern OS_EVENT* wdtSemEvt;
extern s64 Video_timebase; //roule ff
extern u8 ResetPlayback;
extern u8 TVout_Generate_Pause_Frame;
#define AC97_SETTING_DELAY      10000
extern const INT32U gDMAReqCmmd[DMA_REQ_MAX];
#if NIC_SUPPORT
extern u8 EnableStreaming;
#endif
#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
extern s64 IDUInterruptTime;   /* Lsk 090326 Playbak */
#endif

extern u8 rfiuVoxEna[MAX_RF_DEVICE];
extern u8 rfiuVoxThresh[MAX_RF_DEVICE];
extern u8 rfiuVoxTrigFlag;
extern u32 rfiuVoxTrigLev;


extern OS_EVENT *gRfiuAVCmpSemEvt[MAX_RFIU_UNIT];

#if RF_TX_OPTIMIZE
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];
#endif

#if(HW_BOARD_OPTION  == MR8100_GCT_VM9710)
extern u8  uiIsVM9710;
#endif
extern u8 rfMuteVox;
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
s32 wavReadVoiceData(FS_FILE*, IIS_BUF_MNG*); //civic 070829 S
s32 wavWriteVoiceData(FS_FILE*, IIS_BUF_MNG*); //civic 070829 S
s32 wavRecVoiceFile(void);      //civic 070829 S
s32 iisSetPlayFormat(WAVEFORMAT*);
s32 iisSetRecFormat(WAVEFORMAT*);
s32 iisStartPlay(void);
s32 iisStopPlay(void);
s32 iisStartRec(void);
s32 iisStopRec(void);
s32 iisSetPlayDma(u8*, u32);
s32 iisSetRecDma(u8*, u32);
s32 iisCheckPlayDmaReady(void);
s32 iisCheckRecDmaReady(void);
#if IIS_TEST
s32 iisStopRec_ch(u8 ch);
s32 iisSetRecDma_ch(u8* buf, u32 siz, u8 ch);
s32 iisCheckRecDmaReady_ch(u8 ch);
#endif
void iisIOPlayDMA_ISR(int);
void iisPlayDMA_ISR(int);
void iisRecDMA_ISR(int);
void iisSetNextPlayDMA(u8* buf, u32 siz);
void iisSetNextRecDMA(u8* buf, u32 siz);
void iisSetNextRecDMA_auto(u8* buf, u32 siz);
/* yc: 20070124: S */
#if(AUDIO_OPTION == AUDIO_AC97_ALC203)
s32 ac97SetupALC203_rec(void);
s32 ac97SetupALC203_play(void);
void ac97SetupALC203_pwd(void);
#endif

u32 Get_Voice_On_Off(void);
void Set_Voice_On_Off(u8 on_off);
/* yc: 20070124: E */

/* yc:0814 S */
void  Output_Sem(void);
/* yc:0814 E */

#ifndef AUDIO_CODEC
    #define AUDIO_CODEC         AUDIO_CODEC_PCM
#endif
#if (AUDIO_CODEC == AUDIO_CODEC_MS_ADPCM)
s32 wavRecVoiceFile_MS_ADPCM(void);
s32 wavReadFile_MS_ADPCM(FS_FILE* pFile);
#endif

 extern u8 uiMenuEnable;    //civic 070903
 u32 guiIISRecDMAId=0xFF, guiIISPlayDMAId=0xFF;
#if ((HW_BOARD_OPTION==ES_LIGHTING))
 u32 guiIISRecDMAId2=0xFF, guiIISPlayDMAId2=0xFF;
 u32 guiIISRecSize=0;
#endif
 u8  gucIISPlayDMAStarting=0, gucIISPlayDMAPause=0, gucIISStartPlayBefore=0;
 u8  gucIISRecDMAStarting=0, gucIISRecDMAPause=0, gucIISStartRecBefore=0;
 volatile u8  *gpIISPlayDMANextBuf[16];
 volatile u8  gucIISPlayDMACurrBufIdx, gucIISPlayDMANextBufIdx;
 volatile u8  *gpIISRecDMANextBuf[16];
 volatile u8  gucIISRecDMACurrBufIdx, gucIISRecDMANextBufIdx;
 OS_EVENT     *gIISPlayUseSem;
 OS_EVENT     *gIISRecUseSem;
 #if IIS_TEST
 u32 guiIISCh0RecDMAId=0xFF, guiIISCh1RecDMAId=0xFF, guiIISCh2RecDMAId=0xFF, guiIISCh3RecDMAId=0xFF;
 OS_EVENT     *gIISCh0RecUseSem;
 OS_EVENT     *gIISCh1RecUseSem;
 OS_EVENT     *gIISCh2RecUseSem;
 OS_EVENT     *gIISCh3RecUseSem;
#endif
u32 iisPlayDMACnt; //Lsk 090925 : count DMA finish
u32 iisRecDMACnt; //Lsk 090925 : count DMA finish
u32 IIS_Task_Pend;
u32 IIS_PlayTask_Pend;
u32 IIS_PlaybackTask_Stop;
u32 IIS_Task_Stop;
/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*Peter 1109 S*/
/*

Routine Description:

    Initialize the IIS.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisInit(void)
{
    u32 iis_n;
    u32 i;

    // reset IIS hardware
    SYS_RSTCTL  = SYS_RSTCTL | 0x00008000;
    SYS_RSTCTL  = SYS_RSTCTL & ~0x00008000;

    #if ( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) || (AUDIO_OPTION == AUDIO_IIS_ALC5621) )
        IisCtrl = IIS_ENA | IIS_CLK_DIV_A[Audio_formate] | IIS_CLK_DIV_B[Audio_formate];   /* 24MHz */
      #if(IIS_SAMPLE_RATE==8000)
        IisMode = IIS_IIS_COMPATIBLE | IIS_WS_LO_LEFT_CH;
      #elif(IIS_SAMPLE_RATE==16000)
        if( (Audio_formate==nomo_8bit_16k) || (Audio_formate==nomo_16bit_16k) )
            IisMode = IIS_IIS_COMPATIBLE | IIS_WS_LO_LEFT_CH | 0x080F0000; //30000*8/15
        else
            IisMode = IIS_IIS_COMPATIBLE | IIS_WS_LO_LEFT_CH;
	  #endif

      #if((CHIP_OPTION == CHIP_A1013A)|| (CHIP_OPTION == CHIP_A1013B)||(CHIP_OPTION == CHIP_A1016A))
        IisAudFormat = ((IIS_INT_WDONE_MASK | IIS_INT_RDONE_MASK)
                        | IIS_INT_OVER_MASK
                        | IIS_INT_UNDER_MASK
                        | IIS_INT_PEND_MASK
                       );
      #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
        IisAudFormat = ((IIS_INT_WDONE_MASK | IIS_INT_RDONE_MASK)
                        | IIS_INT_OVER_MASK
                        | IIS_INT_UNDER_MASK
                        | IIS_INT_PEND_MASK
                       );
        IisAdvance = (IIS_OP_MODE_IIS1_ENA | IIS_OP_MODE_IIS2_DISA | IIS_OP_MODE_IIS3_DISA | IIS_OP_MODE_IIS4_DISA);
      #else
        IisAudFormat = ((IIS_INT_WDONE_MASK | IIS_INT_RDONE_MASK)
                        | IIS_INT_OVER_MASK
                        | IIS_INT_UNDER_MASK
                        | IIS_INT_PEND_MASK
                       );
      #endif

        //DEBUG_IIS("iisinit IisAudFormat= %x\n", IisAudFormat);

    #elif (AUDIO_OPTION == AUDIO_ADC_DAC)
        IisCtrl = IIS_ENA | IIS_CLK_DIV_A[Audio_formate] | IIS_CLK_DIV_B[Audio_formate];   /* 24MHz */

	  #if(IIS_SAMPLE_RATE==8000)
    	IisMode = ( IIS_IIS_COMPATIBLE | IIS_WS_LO_LEFT_CH | OUT_DIGITIAL_FILTER_DISA |OUT_DIGITAL_FILTER_GAINX4 |
    		        OUT_DIGITAL_FILTER_FORMAT_UNSIGNED | R_MUTE_EN_DISA |L_MUTE_EN_DISA);
      #elif(IIS_SAMPLE_RATE==16000)
        if(Audio_formate==nomo_8bit_16k || Audio_formate==nomo_16bit_16k)
		    IisMode = ( IIS_IIS_COMPATIBLE | IIS_WS_LO_LEFT_CH | OUT_DIGITIAL_FILTER_DISA |OUT_DIGITAL_FILTER_GAINX4 |
    		            OUT_DIGITAL_FILTER_FORMAT_UNSIGNED | R_MUTE_EN_DISA |L_MUTE_EN_DISA|0x080F0000);//30000*8/15
        else
            IisMode = ( IIS_IIS_COMPATIBLE | IIS_WS_LO_LEFT_CH | OUT_DIGITIAL_FILTER_DISA |OUT_DIGITAL_FILTER_GAINX4 |
    		            OUT_DIGITAL_FILTER_FORMAT_UNSIGNED | R_MUTE_EN_DISA |L_MUTE_EN_DISA);
      #endif

    #endif

    if (iisSetPlayFormat(&iisPlayFormat) == 0)
        return 0;

    if (iisSetRecFormat(&iisRecFormat) == 0)
        return 0;

    /* initialize sound buffer */
    for(iis_n = 0; iis_n < IIS_BUF_NUM; iis_n++)
        iisSounBufMng[iis_n].buffer = iisSounBuf[iis_n];

    /* Create the semaphore */
    Output_Sem();
    iisTrgSemEvt = OSSemCreate(0);//IIS_BUF_NUM - 2);    /* guarded for ping-pong buffer */
    iisCmpSemEvt = OSSemCreate(0);
    iisplayCmpEvt = OSSemCreate(0); // Peter 20090527
    for(i=0; i<MULTI_CHANNEL_MAX; i++)
        AudioRTPCmpSemEvt[i] = OSSemCreate(0);
    Output_Sem();
    //DEBUG_IIS("iisInit OSSemCreate(%d), iisTrgSemEvt = %d\n",  IIS_BUF_NUM, iisTrgSemEvt->OSEventCnt);
    //DEBUG_IIS("iisInit OSSemCreate(0), iisCmpSemEvt = %d\n", iisCmpSemEvt->OSEventCnt);

    gIISPlayUseSem = OSSemCreate(1);
    gIISRecUseSem = OSSemCreate(1);

  #if IIS_TEST
    gIISCh0RecUseSem = OSSemCreate(1);
    gIISCh1RecUseSem = OSSemCreate(1);
    gIISCh2RecUseSem = OSSemCreate(1);
    gIISCh3RecUseSem = OSSemCreate(1);
  #endif

  #if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
    iisPlaybackSemEvt   = OSSemCreate(0);
    //DEBUG_IIS("iisInit OSSemCreate(0), iisPlaybackSemEvt = %d\n", iisPlaybackSemEvt->OSEventCnt);
  #endif

    /* Create the task */
    //DEBUG_IIS("Trace: IIS task creating.\n");
    OSTaskCreate(IIS_TASK, IIS_TASK_PARAMETER, IIS_TASK_STACK, IIS_TASK_PRIORITY);
    iisSuspendTask();

  #if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
    //DEBUG_IIS("Trace: IIS playback task creating.\n");
    OSTaskCreate(IIS_PLAYBACK_TASK, IIS_PLAYBACK_TASK_PARAMETER, IIS_PLAYBACK_TASK_STACK, IIS_PLAYBACK_TASK_PRIORITY);
    iisSuspendPlaybackTask();
  #endif

  #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    IMA_ADPCM_Init();
  #endif

    return 1;
}
/*Peter 1109 E*/

/* Peter 070104 */
/*

Routine Description:

    Initialize the IIS before video capture.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisCaptureVideoInit(void)
{
    IISTime = 0;

    return 1;
}
/* Peter 070104 */

/*Peter 1109 S*/
/*

Routine Description:

    Reset the IIS.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

    Notice: ADC/DAC/IIS clock  都是吃ADC clock. 不受Sysclk 變動影響.
            MR6720 的 ADC clock=48 MHz
            A1013  的 ADC clock 暫定為 24 MHz

*/
s32 iisReset(u8 clk_sel)
{
    u32 iis_n;

    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) || (AUDIO_OPTION == AUDIO_IIS_ALC5621) )
        // reset IIS hardware
        SYS_RSTCTL  = SYS_RSTCTL | 0x00008000;
        SYS_RSTCTL  = SYS_RSTCTL & ~0x00008000;

        IisCtrl = IIS_ENA | IIS_CLK_DIV_A[Audio_formate] | IIS_CLK_DIV_B[Audio_formate];   /* 24MHz */
	  #if(IIS_SAMPLE_RATE==8000)
        IisMode = IIS_IIS_COMPATIBLE | IIS_WS_LO_LEFT_CH;
	  #elif(IIS_SAMPLE_RATE==16000)
        IisMode = IIS_IIS_COMPATIBLE | IIS_WS_LO_LEFT_CH | 0x080F0000;
	  #endif

    #elif (AUDIO_OPTION == AUDIO_ADC_DAC)
        // reset IIS hardware
        SYS_RSTCTL  = SYS_RSTCTL | 0x00008000;
        SYS_RSTCTL  = SYS_RSTCTL & ~0x00008000;


        IisCtrl = IIS_ENA | IIS_CLK_DIV_A[Audio_formate] | IIS_CLK_DIV_B[Audio_formate];   /* 24MHz */

		#if(IIS_SAMPLE_RATE==8000)
    		IisMode = IIS_IIS_COMPATIBLE | IIS_WS_LO_LEFT_CH |OUT_DIGITAL_FILTER_GAINX4;
        #elif(IIS_SAMPLE_RATE==16000)
			IisMode = IIS_IIS_COMPATIBLE | IIS_WS_LO_LEFT_CH |OUT_DIGITAL_FILTER_GAINX4|0x080F0000;
		#endif

		#if (HW_BOARD_OPTION == MR6730_AFN)
			if(UI_isPlaybackMode())
			{
				DEBUG_IIS("iisReset wo adcInit(0)\n");//skip this to reduce bob sound
			}
			else
				adcInit(0);
			
		#else
		adcInit(0);
		#endif
		init_DAC_play(0);
      #if ((ADC_SUBBOARD == ADC_SUBBOARD_JUSTEK) && !IS_COMMAX_DOORPHONE)
        uiMenuAction(UI_MENU_SETIDX_VOLUME);
      #endif

        IisCtrl &= ~IIS_ENA;           // IisCtrl is the data of IIS Control reg
        IisCtrl |= IIS_ENA;
    #endif


    if (iisSetPlayFormat(&iisPlayFormat) == 0)
        return 0;

    if (iisSetRecFormat(&iisRecFormat) == 0)
        return 0;

    /* initialize sound buffer */
    for(iis_n = 0; iis_n < IIS_BUF_NUM; iis_n++)
        iisSounBufMng[iis_n].buffer = iisSounBuf[iis_n];

    return 1;
}
/*Peter 1109 E*/

/*Peter 1109 S*/
/*

Routine Description:

    The IRQ handler of IIS.

Arguments:

    None.

Return Value:

    None.

*/
void iisIntHandler(void)
{

    u32  INT_SOURCE;

    //gpioSetLevel(0, 0, 1);
    INT_SOURCE = (*((volatile unsigned *)(IisCtrlBase + 0x0008)));
    if(IISMode == 1)
    {   // IIS playback
    #if IIS_DEBUG_ENA
        under_count++;
        //DEBUG_IIS("IIS_UnderRunSta=%d\n",IIS_UnderRunSta);
    #endif
        IisCtrl |= (IIS_EXMT_ENA | IIS_XMT_ENA | IIS_ENA);
    }
    else if(IISMode == 0 || IISMode == 2)
    {   // IIS record or Receive and playback audio in preview mode
        IisCtrl &= ~IIS_RCV_ENA;
        IisCtrl |= IIS_ENA;
    #if (AUDIO_OPTION != AUDIO_ADC_DAC)
        IisCtrl |= IIS_RCV_ENA;
    #endif
    #if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
        IisCtrl |= (IIS_EXMT_ENA | IIS_XMT_ENA | IIS_ENA);
    #endif
    }
    //gpioSetLevel(0, 0, 0);
}
/*Peter 1109 E*/
#if PLAYBEEP_TEST
void iisSetClickSound(u32 select, u8 *pdata, u32 size)
{
    u16 audio_loop;
    u16 loop_times;
    u8 rest_play_data;
    u8 *pBuf;

    sysIIS_enable();
    iisReset(IIS_SYSPLL_SEL_48M);
    init_DAC_play(1);

    switch(select)
    {
        case PLAY_BEEP:
            pBuf=beep_wav;
            loop_times=sizeof(beep_wav) / IIS_BUF_SIZ;
            rest_play_data=sizeof(beep_wav) % IIS_BUF_SIZ;
            rest_play_data=((rest_play_data)/4)*4; //align 4 bytes
            if(rest_play_data)
                loop_times++;
            break;
        case PLAY_POWER_ON:
        case PLAY_POWER_OFF:
            pBuf=pdata;
            loop_times=size / IIS_BUF_SIZ;
            rest_play_data=size % IIS_BUF_SIZ;
            rest_play_data=((rest_play_data)/4)*4; //align 4 bytes
            if(rest_play_data)
                loop_times++;
            break;
        default:
            iisStopPlay();
            sysIIS_disable();
            return;
    }
    for(audio_loop=0; audio_loop < loop_times; audio_loop ++)
    {
        if(audio_loop!=loop_times-1)
        {
                iisSetPlayDma(pBuf, IIS_BUF_SIZ);
                iisStartPlay();
                pBuf += IIS_BUF_SIZ;
        }
        else
        {
            iisSetPlayDma(pBuf, rest_play_data);
                iisStartPlay();
                pBuf += rest_play_data;
        }
        if(iisCheckPlayDmaReady() != 1)
        {
            DEBUG_IIS("Error: iisCheckPlayDmaReady()\n");
            return;
        }
    }
    iisStopPlay();
    sysIIS_disable();

    //if(!sysTVOutOnFlag)
    //    gpioSetLevel(2, 28, 0);  //Speaker Disable
}

s32 playSoundBeep(s32 select)
{
    u16 audio_loop;
    u16 loop_times;
    u8 rest_play_data;
    u8 *pBuf,err;
    u32 status;
    u32 cur_clock;
    //---------------------------//

    sysPlayBeepFlag=1;
    //sysIIS_enable();
    //iisReset(IIS_SYSPLL_SEL_48M);

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_play();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_play();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_play();
#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
    init_DAC_play(1);
#endif
	if(select==1)
    {
        pBuf=snapshoot_wav+44;
        loop_times=(sizeof(snapshoot_wav)-44) / IIS_BUF_SIZ;
        rest_play_data=(sizeof(snapshoot_wav)-44) % IIS_BUF_SIZ;
        rest_play_data=((rest_play_data)/4)*4; //align 4 bytes
        if(rest_play_data)
            loop_times++;
        //DEBUG_IIS("snapshoot_wav size: %x\n",sizeof(snapshoot_wav));
    }
    else if(select==2)
	{
        pBuf=button4_wav+44;
        loop_times=(sizeof(button4_wav)-44) / IIS_BUF_SIZ;
        rest_play_data=(sizeof(button4_wav)-44) % IIS_BUF_SIZ;
        rest_play_data=((rest_play_data)/4)*4; //align 4 bytes
        if(rest_play_data)
            loop_times++;
        //DEBUG_IIS("snapshoot_wav size: %x\n",sizeof(snapshoot_wav));
    }
	else if(select==3)
	{
        pBuf=button1_wav+44;
        loop_times=(sizeof(button1_wav)-44) / IIS_BUF_SIZ;
        rest_play_data=(sizeof(button1_wav)-44) % IIS_BUF_SIZ;
        rest_play_data=((rest_play_data)/4)*4; //align 4 bytes
        if(rest_play_data)
            loop_times++;
        //DEBUG_IIS("snapshoot_wav size: %x\n",sizeof(snapshoot_wav));
    }
	else if(select==4)
	{
        pBuf=DoorOpen_wav+44;
        loop_times=(sizeof(DoorOpen_wav)-44) / IIS_BUF_SIZ;
        rest_play_data=(sizeof(DoorOpen_wav)-44) % IIS_BUF_SIZ;
        rest_play_data=((rest_play_data)/4)*4; //align 4 bytes
        if(rest_play_data)
            loop_times++;
        //DEBUG_IIS("snapshoot_wav size: %x\n",sizeof(snapshoot_wav));
    }
	else if(select==5)
	{
        pBuf=InterphoneCall_wav+44;
        loop_times=(sizeof(InterphoneCall_wav)-44) / IIS_BUF_SIZ;
        rest_play_data=(sizeof(InterphoneCall_wav)-44) % IIS_BUF_SIZ;
        rest_play_data=((rest_play_data)/4)*4; //align 4 bytes
        if(rest_play_data)
            loop_times++;
        //DEBUG_IIS("snapshoot_wav size: %x\n",sizeof(snapshoot_wav));
    }
	else if(select==6)
	{
        pBuf=PowerOn_wav+44;
        loop_times=(sizeof(PowerOn_wav)-44) / IIS_BUF_SIZ;
        rest_play_data=(sizeof(PowerOn_wav)-44) % IIS_BUF_SIZ;
        rest_play_data=((rest_play_data)/4)*4; //align 4 bytes
        if(rest_play_data)
            loop_times++;
        //DEBUG_IIS("snapshoot_wav size: %x\n",sizeof(snapshoot_wav));
    }
    else
    {
        pBuf=beep_wav+44;
        loop_times=(sizeof(beep_wav)-44) / IIS_BUF_SIZ;
        rest_play_data=(sizeof(beep_wav)-44) % IIS_BUF_SIZ;
        rest_play_data=((rest_play_data)/4)*4; //align 4 bytes
        if(rest_play_data)
            loop_times++;
        //DEBUG_IIS("snapshoot_wav size: %x\n",sizeof(snapshoot_wav));
    }

    DEBUG_IIS("===>Play Beep Start\n");
    if(guiIISPlayDMAId == 0xFF)
    {
        marsDMAOpen(&guiIISPlayDMAId, iisPlayDMA_ISR);
    //    DEBUG_IIS("guiIISPlayDMAId = %d \r\n", guiIISPlayDMAId);
		gucIISPlayDMACurrBufIdx=0;
		gucIISPlayDMANextBufIdx=0;
    }
    for(audio_loop=0; audio_loop < loop_times; audio_loop ++)
    {
    	gpIISPlayDMANextBuf[gucIISPlayDMANextBufIdx] = pBuf;
		if(gucIISPlayDMANextBufIdx == 15)
            gucIISPlayDMANextBufIdx=0;
        else
            gucIISPlayDMANextBufIdx++;

		if(gucIISPlayDMAStarting == 0)
        {
        	gucIISPlayDMAStarting = 1;
        //	DEBUG_IIS("gucIISPlayDMAStarting = %d \r\n", gucIISPlayDMAStarting);
            iisSetNextPlayDMA((u8*)gpIISPlayDMANextBuf[gucIISPlayDMACurrBufIdx], IIS_BUF_SIZ);
            iisStartPlay();
            //gucIISPlayDMAStarting = 1;
        }
        if(gucIISPlayDMACurrBufIdx == gucIISPlayDMANextBufIdx)
        {
            while((gucIISPlayDMACurrBufIdx == gucIISPlayDMANextBufIdx)&&(gucIISPlayDMAStarting))
            {  //DEBUG_IIS("4");
               OSTimeDly(1);
            }
        }
		pBuf += IIS_BUF_SIZ;
	#if 0
        if(audio_loop!=loop_times-1)
        {
            iisSetPlayDma(pBuf, IIS_BUF_SIZ);
            iisStartPlay();
            pBuf += IIS_BUF_SIZ;
        }
        else
        {
            iisSetPlayDma(pBuf, rest_play_data);
            iisStartPlay();
            pBuf += rest_play_data;
        }
        if(iisCheckPlayDmaReady() != 1)
        {
            DEBUG_IIS("Error: iisCheckPlayDmaReady()\n");
            return;
        }
	#endif
    }

    iisStopPlay();
    sysPlayBeepFlag=0;

    DEBUG_IIS("===>Play Beep End\n");
     //sysIIS_disable();
}
#endif

#if AUDIO_IN_OUT_SELFTEST
int AudioInOutTest(void)
{
    u32 status;
    u32 i;
    u8 *PlayBuf,*FillBuf,*pTemp;
    int pcmcnt,count;
    //-----------------//

#if UART_COMMAND
    DEBUG_MAIN("mainTask: uartCmdInit()\n");
    uartCmdInit();
#endif

    sysDeadLockCheck_ena=0;
#if DINAMICALLY_POWER_MANAGEMENT
    sysIIS_enable();
#endif
    iisInit();

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_rec();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
//    Init_IIS_ALC5621_rec_for_IISTEST();  //1013 FPGA Line-in left
    Init_IIS_ALC5621_rec();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_rec();
#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
    adcInit(1);
#endif

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_play();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_play();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_play();
#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
    init_DAC_play(1);
#endif

    PlayBuf=iisSounBuf[0];
    FillBuf=iisSounBuf[8];

    //Find the data size
    iisSetRecDma( FillBuf, IIS_CHUNK_SIZE*2 );
    iisStartRec();

    if (iisCheckRecDmaReady() != 1)
    {
        /* DMA error */
        DEBUG_IIS("Error: iisCheckRecDmaReady()\n");
        return;
    }

    count=0;
    while(1)
    {
        pTemp=PlayBuf;
        PlayBuf=FillBuf;
        FillBuf=pTemp;

        iisSetPlayDma(PlayBuf, IIS_CHUNK_SIZE*2);
        iisSetRecDma( FillBuf, IIS_CHUNK_SIZE*2 );
        iisStartPlay();
        iisStartRec();

        DEBUG_IIS(".",count);
        if(iisCheckPlayDmaReady() != 1)
        {
            DEBUG_IIS("Error: iisCheckPlayDmaReady()\n");
            return;
        }

        if (iisCheckRecDmaReady() != 1)
        {
            /* DMA error */
            DEBUG_IIS("Error: iisCheckRecDmaReady()\n");
            return;
        }

        count ++;

    }

    iisStopPlay();
    iisStopRec();

    return 1;
}


#endif
s32 iisPlayWave(u8* SrcBuf, u32 targetSize )
{
    u16 audio_loop;
    u16 loop_times;
    u8 rest_play_data;

    DEBUG_IIS("IIS Play Wave Start...\n");
    sysIIS_enable();
    iisReset(IIS_SYSPLL_SEL_48M);

    #if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_play();
    #elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
        Init_IIS_ALC5621_play();
    #elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
        ac97SetupALC203_play();
    #elif(AUDIO_OPTION == AUDIO_ADC_DAC)
        init_DAC_play(1);
    #endif

    loop_times=sizeof(targetSize) / IIS_BUF_SIZ;
    rest_play_data=sizeof(targetSize) % IIS_BUF_SIZ;
    rest_play_data=((rest_play_data)/4)*4; //align 4 bytes
    if(rest_play_data)
        loop_times++;

    DEBUG_IIS("wav size: %x\, loop time:%d n",targetSize,loop_times);

    for(audio_loop=0; audio_loop < loop_times; audio_loop ++)
    {
        if(audio_loop!=loop_times-1)
        {
            iisSetPlayDma(SrcBuf, IIS_BUF_SIZ);
            iisStartPlay();
            SrcBuf += IIS_BUF_SIZ;
        }
        else
        {
            iisSetPlayDma(SrcBuf, rest_play_data);
            iisStartPlay();
            SrcBuf += rest_play_data;
        }
        if(iisCheckPlayDmaReady() != 1)
        {

            // ichuang 2008/5/28 S
            /* DMA error */
            DEBUG_IIS("Error: iisCheckPlayDmaReady()\n");
            // ichuang 2008/5/28 E
            return;
        }
    }

    iisStopPlay();
    DEBUG_IIS("IIS Play Wave Complete...\n");

    #if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        //Init_IIS_WM8974_reset(); //Lucian: remove
    #elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
        ac97SetupALC203_pwd();
    #elif(AUDIO_OPTION == AUDIO_ADC_DAC)
        init_DAC_play(0);
    #endif
    sysIIS_disable();
    return 1;
}


/*

Routine Description:

    The test routine of IIS.

Arguments:

    None.

Return Value:

    None.

*/
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
extern  DMA_CFG_AUTO MarsdmaCfg_auto[DMA_REQ_MAX];
#endif
#if IIS_TEST
extern const INT32U gDMAReqCmmd[DMA_REQ_MAX];
void DumpWaveFile(void)
{
    u8* pBuf;
    u32 i,audio_loop;

    //wave header
    pBuf = iisBuf_play-44;
    for(i=0; i<44; i+=4)
    {
        DEBUG_IIS("0x%08x, ",*(unsigned int *)(pBuf+i));
    }
    DEBUG_IIS("\n");
    //wave data
    pBuf=iisBuf_play;
    for(audio_loop=0; audio_loop < IIS_REC_TIME; audio_loop ++)
    {
        for(i=0; i<IIS_BUF_SIZ; i+=4)
        {
            DEBUG_IIS("0x%08x, ",*(unsigned int *)(pBuf+i));
        }
        DEBUG_IIS("\n");
        pBuf += IIS_BUF_SIZ;
    }
}
int SaveDataInSD(void)
{
    FS_FILE* pFile;
    riff_wave_hdr_t riff_wave_hdr;

    u16     width, chan;
    u32     rate, data_size;
    u32     size;

    u16 CHAN[20]  = {1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2};
    u16 WIDTH[20] = {1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2};
    u32 RATE[20]  = {8000,16000,32000,44100,48000,8000,16000,32000,44100,48000,8000,16000,32000,44100,48000,8000,16000,32000,44100,48000};




    width = WIDTH[Audio_formate];
    chan  = CHAN[Audio_formate];
    rate  = RATE[Audio_formate];
    data_size = IIS_REC_TIME*IIS_BUF_SIZ;
    riff_create_header (&riff_wave_hdr, chan, rate, width, data_size);
    memcpy((unsigned char*)(iisBuf_play-44),(unsigned char*)&riff_wave_hdr,sizeof(riff_wave_hdr));

    //use byte addr for sd size less than 2G. Use winhex dump 0xC800~0x1f42c data
    sdcWriteMultipleBlock(1+(44+IIS_REC_TIME*IIS_BUF_SIZ)/512, 100*512, (unsigned char*)(iisBuf_play-44));



}

int SaveDataInWaveFile(void)
{
    FS_FILE* pFile;
    riff_wave_hdr_t riff_wave_hdr;

    u16     width, chan;
    u32     rate, data_size;
    u32     size;
	u8		tmp;

    u16 CHAN[20]  = {1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2};
    u16 WIDTH[20] = {1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2};
    u32 RATE[20]  = {8000,16000,32000,44100,48000,8000,16000,32000,44100,48000,8000,16000,32000,44100,48000,8000,16000,32000,44100,48000};
    char  fin_name[16];

    dcfChDir("\\DCIM\\100VIDEO");

    //if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_WAV, 0)) == NULL){
    //    DEBUG_IIS("Wav create file error!!!\n");
    //    return 0;
    //}
    sprintf(fin_name, "%08d.wav", Audio_formate); //file system only filename 08d.03d
    pFile= dcfOpen(fin_name, "w+b");

    width = WIDTH[Audio_formate];
    chan  = CHAN[Audio_formate];
    rate  = RATE[Audio_formate];
    data_size = Rec_Time[Audio_formate]*IIS_BUF_SIZ;
    riff_create_header (&riff_wave_hdr, chan, rate, width, data_size);
    if (dcfWrite(pFile, (unsigned char*)&riff_wave_hdr, sizeof(riff_wave_hdr), &size) == 0)
    {
        DEBUG_IIS("Wav Write header error!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }

    if(dcfWrite(pFile, (unsigned char*)iisBuf_play, data_size, &size) == 0)
    {
        DEBUG_IIS("Rec voice File error \n");
        return 0;
    }
    dcfClose(pFile, &tmp);

}
int SaveDataInWaveFile_ch(u8 ch)
{
    FS_FILE* pFile;
    riff_wave_hdr_t riff_wave_hdr;

    u16     width, chan;
    u32     rate, data_size;
    u32     size;

    u16 CHAN[20]  = {1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2};
    u16 WIDTH[20] = {1,1,1,1,1,2,2,2,2,2,1,1,1,1,1,2,2,2,2,2};
    u32 RATE[20]  = {8000,16000,32000,44100,48000,8000,16000,32000,44100,48000,8000,16000,32000,44100,48000,8000,16000,32000,44100,48000};
    char  fin_name[16];
    u8 *pBuf;
	u8 tmp;

    if(ch==0)
        pBuf = VideoBuf;
    else if(ch==1)
        pBuf = VideoBuf + IIS_BUF_SIZ*Rec_Time[Audio_formate];


    dcfChDir("\\DCIM\\100VIDEO");


    sprintf(fin_name, "%01d%07d.wav", ch, Audio_formate); //file system only filename 08d.03d
    pFile= dcfOpen(fin_name, "w+b");


    width = WIDTH[Audio_formate];
    chan  = CHAN[Audio_formate];
    rate  = RATE[Audio_formate];
    data_size = Rec_Time[Audio_formate]*IIS_BUF_SIZ;
    riff_create_header (&riff_wave_hdr, chan, rate, width, data_size);
    if (dcfWrite(pFile, (unsigned char*)&riff_wave_hdr, sizeof(riff_wave_hdr), &size) == 0)
    {
        DEBUG_IIS("Wav Write header error!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }
    if(dcfWrite(pFile, (unsigned char*)pBuf, data_size, &size) == 0)
    {
        DEBUG_IIS("Rec voice File error \n");
        return 0;
    }
    dcfClose(pFile, &tmp);
}

void iis_ch0_task(void)
{
    u32 audio_loop;
    u8 *pBuf;
    u32 status;
    u32 i;
    u8 err;
    while(1)
    {
        OSSemPend(iisCh0TrgSemEvt, 0, &err);

        pBuf = VideoBuf;

        DEBUG_IIS("Ch0 REC Data addr : 0x%08x\n",(u32)(pBuf));

        for(audio_loop=0; audio_loop < Rec_Time[Audio_formate]; audio_loop ++)
        {
            //DEBUG_IIS("ch0 audio_loop=%d\n",audio_loop);
            iisSetRecDma_ch(pBuf, IIS_BUF_SIZ, 0);
            iisStartRec();
            pBuf += IIS_BUF_SIZ;

            if (iisCheckRecDmaReady_ch(0) != 1)
            {
                /* DMA error */
                DEBUG_IIS("CH0 Error: iisCheckRecDmaReady()\n");
                //return;
            }
        }
        //iisStopRec_ch(0);
        DEBUG_IIS("Ch0 IIS/AC97 Recording Complete...\n");

        OSSemPost(iisCh0CmpSemEvt);
    }
}
void iis_ch1_task(void)
{
    u32 audio_loop;
    u8 *pBuf;
    u32 status;
    u32 i;
    u8 err;

    while(1)
    {
        OSSemPend(iisCh1TrgSemEvt, 0, &err);

        pBuf = VideoBuf + IIS_BUF_SIZ*Rec_Time[Audio_formate];

        DEBUG_IIS("Ch1 REC Data addr : 0x%08x\n",(u32)(pBuf));

        for(audio_loop=0; audio_loop < Rec_Time[Audio_formate]; audio_loop ++)
        {
            //DEBUG_IIS("<%d>",audio_loop);
            iisSetRecDma_ch(pBuf, IIS_BUF_SIZ, 1);
            iisStartRec();
            pBuf += IIS_BUF_SIZ;

            if (iisCheckRecDmaReady_ch(1) != 1)
            {
                /* DMA error */
                DEBUG_IIS("CH1 Error: iisCheckRecDmaReady()\n");
                //return;
            }
        }
        //iisStopRec_ch(1);
        DEBUG_IIS("Ch1 IIS/AC97 Recording Complete...\n");
        //DEBUG_IIS("Ch1 Save data start!!!\n");
        OSSemPost(iisCh1CmpSemEvt);
        //SaveDataInSD();
        //DEBUG_IIS("Ch1 Save data end!!!\n");
    }
}

void iisTest_2ch(void)
{
    u32 audio_loop;
    u8 *pBuf;
    u32 status;
    u32 i;
    u8 err;

#if DINAMICALLY_POWER_MANAGEMENT
      sysIIS_enable();
#endif

    DEBUG_IIS("************************************\n");
    DEBUG_IIS("2 channel record test\n");
    DEBUG_IIS("Audio_formate =  %d \n",Audio_formate);
    DEBUG_IIS("************************************\n");

    if(Audio_formate>= nomo_16bit_8k && nomo_16bit_8k<0x0a)
    {
        iisPlayFormat.wBitsPerSample = 16;
        iisRecFormat.wBitsPerSample = 16;
    }

    iisInit();
    DEBUG_IIS("IIS/AC97 Recording Start...\n");
#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_rec();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
//    Init_IIS_ALC5621_rec_for_IISTEST();  //1013 FPGA Line-in left
    Init_IIS_ALC5621_rec();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_rec();
#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
    adcInit(1);
#endif
    iisInit();

    iisCh0TrgSemEvt = OSSemCreate(1);
    iisCh1TrgSemEvt = OSSemCreate(1);
    iisCh0CmpSemEvt = OSSemCreate(0);
    iisCh1CmpSemEvt = OSSemCreate(0);

    OSTaskCreate(IIS_CH0_TASK, IIS_CH0_TASK_PARAMETER, IIS_CH0_TASK_STACK, IIS_CH0_TASK_PRIORITY);
    OSTaskCreate(IIS_CH1_TASK, IIS_CH1_TASK_PARAMETER, IIS_CH1_TASK_STACK, IIS_CH1_TASK_PRIORITY);

    for(Audio_formate=nomo_8bit_8k; Audio_formate< 0X0A;)
    {
        OSSemPend(iisCh0CmpSemEvt, 0, &err);
        SaveDataInWaveFile_ch(0);
        DEBUG_IIS("Ch0 Save data end!!!\n");

        OSSemPend(iisCh1CmpSemEvt, 0, &err);
        SaveDataInWaveFile_ch(1);
        DEBUG_IIS("Ch1 Save data end!!!\n");

        iisStopRec_ch(0);
        iisStopRec_ch(1);

        Audio_formate++;

        DEBUG_IIS("************************************\n");
        DEBUG_IIS("2 channel record test\n");
        DEBUG_IIS("Audio_formate =  %d \n",Audio_formate);
        DEBUG_IIS("************************************\n");

        iisInit();
        DEBUG_IIS("IIS/AC97 Recording Start...\n");
        #if (AUDIO_OPTION == AUDIO_IIS_WM8974)
        Init_IIS_WM8974_rec();
        #elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
        //Init_IIS_ALC5621_rec_for_IISTEST();  //1013 FPGA Line-in left
        Init_IIS_ALC5621_rec();
        #elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
        ac97SetupALC203_rec();
        #elif(AUDIO_OPTION == AUDIO_ADC_DAC)
        adcInit(1);
        #endif
        iisInit();

        OSSemPost(iisCh0TrgSemEvt);
        OSSemPost(iisCh1TrgSemEvt);
    }

}
void iisTest(void)
{
    u32 audio_loop;
    u8 *pBuf;
    u32 status;
    u32 i;

#if DINAMICALLY_POWER_MANAGEMENT
      sysIIS_enable();
#endif

    DEBUG_IIS("\n\n\n Audio_formate =  %d \n\n\n",Audio_formate);

    if(Audio_formate>= nomo_16bit_8k && nomo_16bit_8k<0x0a)
    {
        iisPlayFormat.wBitsPerSample = 16;
        iisRecFormat.wBitsPerSample = 16;
    }

    iisInit();
    DEBUG_IIS("IIS/AC97 Recording Start...\n");
#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_rec();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
//    Init_IIS_ALC5621_rec_for_IISTEST();  //1013 FPGA Line-in left
    Init_IIS_ALC5621_rec();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_rec();
#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
    adcInit(1);
#endif
    iisInit();


    pBuf=iisBuf_play;
    DEBUG_IIS("REC Data addr : 0x%08x\n",(u32)(pBuf));

    for(audio_loop=0; audio_loop < Rec_Time[Audio_formate]; audio_loop ++)
    {
        iisSetRecDma( pBuf, IIS_BUF_SIZ );
        iisStartRec();
        pBuf += IIS_BUF_SIZ;

        if (iisCheckRecDmaReady() != 1)
        {
            /* DMA error */
            DEBUG_IIS("Error: iisCheckRecDmaReady()\n");
            return;
        }
    }
    iisStopRec();
    DEBUG_IIS("IIS/AC97 Recording Complete...\n");

    DEBUG_IIS("Save data start!!!\n");
    SaveDataInWaveFile();
    //SaveDataInSD();
    DEBUG_IIS("Save data end!!!\n");

    //==========//
    DEBUG_IIS("IIS/AC97 Playback Start...\n");
#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_play();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_play();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_play();
#endif
    pBuf=iisBuf_play;
    for(audio_loop=0; audio_loop < Rec_Time[Audio_formate]; audio_loop ++)
    {
        iisSetPlayDma(pBuf, IIS_BUF_SIZ);
        iisStartPlay();
        pBuf += IIS_BUF_SIZ;

        if(iisCheckPlayDmaReady() != 1)
        {
            /* DMA error */
            DEBUG_IIS("Error: iisCheckPlayDmaReady()\n");
            return;
        }
    }
    iisStopPlay();
    DEBUG_IIS("IIS/AC97 Playback Complete...\n");

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    //Init_IIS_WM8974_reset(); //Lucian: remove
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_pwd();
#endif

}
#if 1
s32 iisSetRecDma_NonStopMode(u8* buf, u32 siz, u32 *DMA_ID)
{
    DMA_CFG_AUTO *pdmaCfg_auto;
    INT8U err;
    u32   ADCRecGrp;
    INT32U uiDMAId,status, uiDMACmmd;
    REGDMA_CFG_AUTO  RegDMACfg_auto;
    REGDMA_CFG_AUTO  *pRegDMACfg_auto = (REGDMA_CFG_AUTO*)&RegDMACfg_auto;


    OSSemPend(gIISRecUseSem, OS_IPC_WAIT_FOREVER, &err);

    marsDMAOpen(&uiDMAId, isr_marsDMAAuto);
    *DMA_ID = uiDMAId;
    pdmaCfg_auto= &MarsdmaCfg_auto[uiDMAId];
    /* set read data dma */
  #if(AUDIO_OPTION == AUDIO_ADC_DAC)
    #if((CHIP_OPTION  == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B))
    ADCRecGrp = ((AdcCtrlReg & 0x0000003C)>>2); //Lsk : must call adcInit() to set adc record group.
    if(ADCRecGrp == 1)
    {
        pdmaCfg_auto->src = (u32)&(AdcRecData_G0);
        uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD];
    }
    else if(ADCRecGrp == 2)
    {
        pdmaCfg_auto->src = (u32)&(AdcRecData_G1);
        uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G1_RECORD];
    }
    else if(ADCRecGrp == 4)
    {
        pdmaCfg_auto->src = (u32)&(AdcRecData_G2);
        uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G2_RECORD];
    }
    else if(ADCRecGrp == 8)
    {
        pdmaCfg_auto->src = (u32)&(AdcRecData_G3);
        uiDMACmmd = gDMAReqCmmd[DMA_REQ_ADC_G3_RECORD];
    }
	#elif((CHIP_OPTION == CHIP_A1016A))
    pdmaCfg_auto->src = (u32)&(AdcRecData_G0);
    uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD];
    #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
    pdmaCfg_auto->src = (u32)&(AdcRecData_G0);
    uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD];
    #else
    pdmaCfg_auto->src = (u32)&(AdcRecData);
    uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD];
    #endif
  #else
    pdmaCfg_auto->src = (u32)&(IisRxData);
    uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD];
  #endif

    pdmaCfg_auto->dst        = (u32)buf;
    pdmaCfg_auto->src_stride = 0;
    pdmaCfg_auto->dst_stride = IIS_BUF_SIZ;
    pdmaCfg_auto->datacnt    = siz / 16; // 4 cycle(burst=1)* 4 bytes(word)
    pdmaCfg_auto->linecnt    = 1;
    pdmaCfg_auto->burst      = 1; /*CY 0917*/

    if(pdmaCfg_auto->burst)
        uiDMACmmd |= DMA_BURST4;

    uiDMACmmd |= DMA_AUTONONSTOP_EN;

    pRegDMACfg_auto->src     = pdmaCfg_auto->src;
    pRegDMACfg_auto->dst     = pdmaCfg_auto->dst;

    pdmaCfg_auto->src       += pdmaCfg_auto->src_stride;
    pdmaCfg_auto->dst       += pdmaCfg_auto->dst_stride;

    pRegDMACfg_auto->src_alt = pdmaCfg_auto->src;
    pRegDMACfg_auto->dst_alt = pdmaCfg_auto->dst;

    pRegDMACfg_auto->datacnt = pdmaCfg_auto->datacnt;
    pRegDMACfg_auto->linecnt = pdmaCfg_auto->linecnt;
    pRegDMACfg_auto->cmmd    = uiDMACmmd;

    marsDMAConfig_auto(uiDMAId, pRegDMACfg_auto);
    return 1;
}
s32 iisSetPlayDma_NonStopMode(u8* buf, u32 siz, u32 *DMA_ID)
{
    DMA_CFG_AUTO *pdmaCfg_auto;
    DMA_CFG dmaCfg;
    INT8U err;
    INT32U uiDMAId,status, uiDMACmmd;
    REGDMA_CFG_AUTO  RegDMACfg_auto;
    REGDMA_CFG_AUTO  *pRegDMACfg_auto = (REGDMA_CFG_AUTO*)&RegDMACfg_auto;


    OSSemPend(gIISPlayUseSem, OS_IPC_WAIT_FOREVER, &err);
    marsDMAOpen(&uiDMAId, isr_marsDMAAuto);
    *DMA_ID = uiDMAId;

    uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_PLAY];
    pdmaCfg_auto= &MarsdmaCfg_auto[uiDMAId];

    pdmaCfg_auto->src        = (u32)buf;
    pdmaCfg_auto->dst        = (u32)&(IisTxData);
    pdmaCfg_auto->src_stride = IIS_BUF_SIZ;
    pdmaCfg_auto->dst_stride = 0;
    pdmaCfg_auto->datacnt    = siz / 16; // 4 cycle(burst=1)* 4 bytes(word)
    pdmaCfg_auto->linecnt    = 1;
    pdmaCfg_auto->burst      = 1; /*CY 0917*/

    if(pdmaCfg_auto->burst)
        uiDMACmmd |= DMA_BURST4;

    uiDMACmmd |= DMA_AUTONONSTOP_EN;

    pRegDMACfg_auto->src     = pdmaCfg_auto->src;
    pRegDMACfg_auto->dst     = pdmaCfg_auto->dst;

    pdmaCfg_auto->src       += pdmaCfg_auto->src_stride;
    pdmaCfg_auto->dst       += pdmaCfg_auto->dst_stride;

    pRegDMACfg_auto->src_alt = pdmaCfg_auto->src;
    pRegDMACfg_auto->dst_alt = pdmaCfg_auto->dst;

    pRegDMACfg_auto->datacnt = pdmaCfg_auto->datacnt;
    pRegDMACfg_auto->linecnt = pdmaCfg_auto->linecnt;
    pRegDMACfg_auto->cmmd    = uiDMACmmd;

    marsDMAConfig_auto(uiDMAId, pRegDMACfg_auto);
    return 1;
}
s32 iisCheckRecDmaReady_NonStopMode(void)
{
    u8 err;

    err = marsDMACheckReady_auto(guiIISRecDMAId);
    guiIISRecDMAId = 0x55;
    OSSemPost(gIISRecUseSem);

    return (err);
}
s32 iisCheckPlayDmaReady_NonStopMode(void)
{
    u8 err;

    err = marsDMACheckReady_auto(guiIISPlayDMAId);
    guiIISPlayDMAId = 0x55;
    OSSemPost(gIISPlayUseSem);
    return (err);
}
void iisTest_NonStopMode(void) // must enable marsDMAIntHandler
{
    u32 audio_loop;
    u8 *pBuf;
    u32 status;
    u32 i;
    u32 uiDMAId;
#if DINAMICALLY_POWER_MANAGEMENT
      sysIIS_enable();
#endif

    DEBUG_IIS("\n\n\n Audio_formate =  %d \n\n\n",Audio_formate);
    if(Audio_formate>= nomo_16bit_8k && nomo_16bit_8k<0x0a)
    {
        iisPlayFormat.wBitsPerSample = 16;
        iisRecFormat.wBitsPerSample = 16;
    }
    iisInit();
    DEBUG_IIS("IIS/AC97 Recording Start...\n");
#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_rec();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_rec();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_rec();
#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
    adcInit(1);
#endif
    iisInit();

    while(iisTrgSemEvt->OSEventCnt < Rec_Time[Audio_formate]) {
        OSSemPost(iisTrgSemEvt);
    }

    pBuf=iisBuf_play;
    iisSetRecDma_NonStopMode(pBuf, IIS_BUF_SIZ, &guiIISRecDMAId);
    iisStartRec();
    iisCheckRecDmaReady_NonStopMode();
    iisStopRec();
    DEBUG_IIS("IIS/AC97 Recording Complete...\n");
    SaveDataInWaveFile();


    //==========//
    DEBUG_IIS("IIS/AC97 Playback Start...\n");
#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_play();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_play();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_play();
#endif
    while(iisTrgSemEvt->OSEventCnt < Rec_Time[Audio_formate]) {
        OSSemPost(iisTrgSemEvt);
    }
    pBuf=iisBuf_play;


    iisSetPlayDma_NonStopMode(pBuf, IIS_BUF_SIZ, &guiIISPlayDMAId);
    iisStartPlay();
    iisCheckPlayDmaReady_NonStopMode();
    iisStopPlay();
    DEBUG_IIS("IIS/AC97 Playback Complete...\n");

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    //Init_IIS_WM8974_reset(); //Lucian: remove
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_pwd();
#endif

}
#endif
#endif
#if((AUDIO_OPTION == AUDIO_ADC_DAC)||(AUDIO_OPTION == AUDIO_IIS_ALC5621))
void ADCRectest(void)
{
    ///ADC REC Test
      sysIIS_enable();
      iisCaptureVideoInit();
      sysVoiceRecStop=0;
      sysVoiceRecStart=1;
	#if(AUDIO_OPTION == AUDIO_IIS_ALC5621)
		Init_IIS_ALC5621_rec();
	#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
		adcInit(1);
	#endif
    DEBUG_IIS("SYS_CTL0 = %#x\n",SYS_CTL0);
      //iisInit();
      wavRecVoice();
      sysIIS_disable();
}
#endif
/*

Routine Description:

    Set playback format.

Arguments:

    pFormat - The playback format.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisSetPlayFormat(WAVEFORMAT* pFormat)
{
    u32 format;


    switch (pFormat->nChannels)
    {
        case 1:         /* Mono */
            if (pFormat->wBitsPerSample == 8)   /* 8 bit */
   				format = IIS_PLAY_8b_MONO | IIS_PLAY_DATA_UNSIGNED;
            else if (pFormat->wBitsPerSample == 16) /* 16 bit */
   				format = IIS_PLAY_16b_MONO | IIS_PLAY_DATA_SIGNED;
            else                    /* Not supported */
                return 0;
            break;

        case 2:         /* Stereo */
            if (pFormat->wBitsPerSample == 16)  /* 16 bit */
                format = IIS_PLAY_16b_STEREO | IIS_PLAY_DATA_SIGNED;  /*Peter 1109 S*/
            else                    /* Not supported */
                return 0;
            break;

        default:
            return 0;   /* Not supported */
    }

#if((CHIP_OPTION == CHIP_A1013A)|| (CHIP_OPTION == CHIP_A1013B)||(CHIP_OPTION == CHIP_A1016A))
    IisAudFormat |= (IIS_INT_OVER_MASK | IIS_INT_UNDER_MASK | IIS_INT_PEND_MASK);
#elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
    IisAudFormat |= (IIS_INT_OVER_MASK | IIS_INT_UNDER_MASK | IIS_INT_PEND_MASK );
    IisAdvance |= (IIS_OP_MODE_IIS1_ENA | IIS_OP_MODE_IIS2_DISA | IIS_OP_MODE_IIS3_DISA | IIS_OP_MODE_IIS4_DISA);
#else
    IisAudFormat &= ~(IIS_INT_OVER_MASK | IIS_INT_UNDER_MASK | IIS_INT_PEND_MASK );
#endif

    IisAudFormat &= (~0x00000007);
    IisAudFormat |= format;



    return 1;
}

/*

Routine Description:

    Set recording format.

Arguments:

    pFormat - The recording format.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisSetRecFormat(WAVEFORMAT* pFormat)
{
    u32 format;

    switch (pFormat->nChannels)
    {
        case 1:         /* Mono */
            if (pFormat->wBitsPerSample == 8)   /* 8 bit */
            {

				#if(CHIP_OPTION == CHIP_PA9001D)
					format = IIS_REC_8b_MONO | IIS_PLAY_DATA_UNSIGNED; /*Peter 1109 S*/
				#else

				#if (AUDIO_OPTION==AUDIO_ADC_DAC)
                    format = IIS_REC_8b_MONO | IIS_REC_DATA_SIGNED;
                #else
                    format = IIS_REC_8b_MONO | IIS_REC_DATA_UNSIGNED;  /*Peter 1109 S*/
                #endif

				#endif
            }
            else if (pFormat->wBitsPerSample == 16) /* 16 bit */
                format = IIS_REC_16b_MONO | IIS_REC_DATA_SIGNED;    /* signed instead? */  /*Peter 1109 S*/
            else                    /* Not supported */
                return 0;
            break;

        /* yc: 2006.07.24: S */
        case 2:         /* Stereo */
            if (pFormat->wBitsPerSample == 16)  /* 16 bit */
                format = IIS_REC_16b_STEREO | IIS_REC_DATA_SIGNED;   /*Peter 1109 S*/
            else                    /* Not supported */
                return 0;
            break;
        /* yc: 2006.07.24: E */

        default:
            return 0;   /* Not supported */
    }

#if((CHIP_OPTION == CHIP_A1013A)|| (CHIP_OPTION == CHIP_A1013B)||(CHIP_OPTION == CHIP_A1016A))
    IisAudFormat |= (IIS_INT_OVER_MASK | IIS_INT_UNDER_MASK | IIS_INT_PEND_MASK );
#elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
    IisAudFormat |= (IIS_INT_OVER_MASK | IIS_INT_UNDER_MASK | IIS_INT_PEND_MASK );
    IisAdvance |= (IIS_OP_MODE_IIS1_ENA | IIS_OP_MODE_IIS2_DISA | IIS_OP_MODE_IIS3_DISA | IIS_OP_MODE_IIS4_DISA);
#else
    IisAudFormat &= ~(IIS_INT_OVER_MASK | IIS_INT_UNDER_MASK | IIS_INT_PEND_MASK );
#endif

    IisAudFormat &= (~0x00000038);
    IisAudFormat |= format;

    return 1;
}

/*

Routine Description:

    Start playback.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisStartPlay(void)
{
    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) || (AUDIO_OPTION == AUDIO_IIS_ALC5621) || (AUDIO_OPTION == AUDIO_ADC_DAC) )
        IisCtrl    |= IIS_EXMT_ENA | IIS_XMT_ENA | IIS_ENA;
    #elif (AUDIO_OPTION == AUDIO_AC97_ALC203)
        Ac97OCC     = 0x00000303;
        Ac97Ctrl   |= AC97_XMT_ENA | AC97_ENA;
    #endif

    //DEBUG_IIS("iisStartPlay: IisCtrl = 0x%08x.\n", IisCtrl);
    gucIISStartPlayBefore   = 1;
    return 1;
}

/*

Routine Description:

    Stop playback.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisStopPlay(void)
{
    volatile INT16U *pIISUseSemCnt = &gIISPlayUseSem->OSEventCnt;
    u8      err;

    if(gucIISStartPlayBefore == 0)
        return;

    //OSSemPend(gIISPlayUseSem, OS_IPC_WAIT_FOREVER, &err);

    if(guiIISPlayDMAId != 0xFF)
    {
        marsDMAClose(guiIISPlayDMAId);
        guiIISPlayDMAId = 0xFF;
    }
    gucIISPlayDMAStarting = 0;
    gucIISPlayDMAPause = 0;

    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) || (AUDIO_OPTION == AUDIO_IIS_ALC5621) || (AUDIO_OPTION == AUDIO_ADC_DAC) )
        IisCtrl  &= ~(IIS_EXMT_ENA | IIS_XMT_ENA | IIS_ENA);
    #elif (AUDIO_OPTION == AUDIO_AC97_ALC203)
        Ac97OCC = 0x00000000;
        Ac97Ctrl &= (~AC97_XMT_ENA);
    #endif

    gucIISStartPlayBefore = 0;
    return 1;
}


/*

Routine Description:

    Start record.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisStartRec(void)
{
    //#if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) ||  (AUDIO_OPTION == AUDIO_IIS_ALC5621) ||(AUDIO_OPTION == AUDIO_ADC_DAC) )
    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) ||  (AUDIO_OPTION == AUDIO_IIS_ALC5621))
        IisCtrl    |= IIS_RCV_ENA | IIS_ENA;
    #elif (AUDIO_OPTION == AUDIO_ADC_DAC)
        IisCtrl    |= IIS_ENA;
    #elif (AUDIO_OPTION == AUDIO_AC97_ALC203)
        Ac97ICC     = 0x00000303;
        Ac97Ctrl   |= AC97_RCV_ENA | AC97_ENA;
    #endif

    return 1;   ;
}

/*

Routine Description:

    Stop record.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisStopRec(void)
{
    volatile INT16U *pIISUseSemCnt = &gIISRecUseSem->OSEventCnt;
    //INT16U *pIISUseSemCnt = &gIISRecUseSem->OSEventCnt;

    while(1)
    {
        if(*pIISUseSemCnt != 0)
            break;
		OSTimeDly(1);
    }
	if(guiIISRecDMAId != 0xFF)
    {
        marsDMAClose(guiIISRecDMAId);
        guiIISRecDMAId = 0xFF;

    }
	gucIISRecDMAStarting = 0;

    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) ||  (AUDIO_OPTION == AUDIO_IIS_ALC5621))
        IisCtrl &= ~IIS_RCV_ENA;
    #elif (AUDIO_OPTION == AUDIO_ADC_DAC)
        #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
            (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
            (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||\
            (CHIP_OPTION == CHIP_A1026A))
           #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
             AdcCtrlReg &= (~ADC_REC_G1);
             //AdcCtrlReg &= (~ADC_REC_CONV_ENA);
           #else
             AdcCtrlReg &= (~ADC_REC_G1);
             //AdcCtrlReg &= (~ADC_REC_CONV_ENA);
           #endif
        #else
        AdcCtrlReg &= ~ADC_RX_ENA;
        #endif
    #elif (AUDIO_OPTION == AUDIO_AC97_ALC203)
        Ac97ICC = 0x00000000;
        Ac97Ctrl &= (~AC97_RCV_ENA);
    #endif

    return 1;
}
#if IIS_TEST
s32 iisStopRec_ch(u8 ch)
{
    volatile INT16U *pIISUseSemCnt;
    //INT16U *pIISUseSemCnt = &gIISRecUseSem->OSEventCnt;

    if(ch==0)
    {
        pIISUseSemCnt = &gIISCh0RecUseSem->OSEventCnt;
        while(1)
        {
            if(*pIISUseSemCnt != 0)
                break;
    		OSTimeDly(1);
        }

    	if(guiIISCh0RecDMAId != 0xFF)
        {
            marsDMAClose(guiIISCh0RecDMAId);
            guiIISCh0RecDMAId = 0xFF;

        }
    	gucIISRecDMAStarting = 0;
    }


    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) ||  (AUDIO_OPTION == AUDIO_IIS_ALC5621))
        IisCtrl &= ~IIS_RCV_ENA;
    #elif (AUDIO_OPTION == AUDIO_ADC_DAC)
        #if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
            (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
            (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
            (CHIP_OPTION == CHIP_A1026A))
           #if ( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
            (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
            (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
             AdcCtrlReg &= (~ADC_REC_G1);
             //AdcCtrlReg &= (~ADC_REC_CONV_ENA);
           #else
             AdcCtrlReg &= (~ADC_REC_G1);
             //AdcCtrlReg &= (~ADC_REC_CONV_ENA);
           #endif
        #else
        AdcCtrlReg &= ~ADC_RX_ENA;
        #endif
    #elif (AUDIO_OPTION == AUDIO_AC97_ALC203)
        Ac97ICC = 0x00000000;
        Ac97Ctrl &= (~AC97_RCV_ENA);
    #endif

    return 1;
}
#endif
/*

Routine Description:

    Set playback dma.

Arguments:

    buf - The buffer to record to.
    siz - The size to record.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisSetPlayDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;
    INT8U err;

    OSSemPend(gIISPlayUseSem, OS_IPC_WAIT_FOREVER, &err);

    /* set write data dma */
    /* yc: 2006.07.24: S */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(IisTxData);
/*Peter 1109 S*/
    dmaCfg.burst    = 1; /*CY 0917*/
    dmaCfg.cnt      = siz / 16;
/*Peter 1109 E*/
    //if (dmaConfig(DMA_REQ_IIS_PLAY, &dmaCfg) == 0)
    //    return 0;
    guiIISPlayDMAId = marsDMAReq(DMA_REQ_IIS_PLAY, &dmaCfg);

    /* yc: 2006.07.24: E */
    return 1;
}

#if ((HW_BOARD_OPTION==ES_LIGHTING))

s32 iisStartPlay2(void)
{
    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) || (AUDIO_OPTION == AUDIO_IIS_ALC5621) || (AUDIO_OPTION == AUDIO_ADC_DAC) )
        IisCtrl    |= IIS_EXMT_ENA | IIS_XMT_ENA | IIS_ENA;
    #elif (AUDIO_OPTION == AUDIO_AC97_ALC203)
        Ac97OCC     = 0x00000303;
        Ac97Ctrl   |= AC97_XMT_ENA | AC97_ENA;
    #endif

    return 1;
}

/*

Routine Description:

    Stop playback.

Arguments:

    mode: 0 : under the record mode.
              1 : not under the record mode
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisStopPlay2(u8 mode)
{


    DEBUG_IIS("iisStopPlay2,  ID=%d \n", guiIISPlayDMAId2);
    if(guiIISPlayDMAId2 != 0xFF)
    {
        marsDMAClose(guiIISPlayDMAId2);
        guiIISPlayDMAId2 = 0xFF;
    }

    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) || (AUDIO_OPTION == AUDIO_IIS_ALC5621) || (AUDIO_OPTION == AUDIO_ADC_DAC) )
        if (mode==0)
            IisCtrl  &= ~(IIS_EXMT_ENA | IIS_XMT_ENA);
        else
            IisCtrl  &= ~(IIS_EXMT_ENA | IIS_XMT_ENA | IIS_ENA);
    #elif (AUDIO_OPTION == AUDIO_AC97_ALC203)
        Ac97OCC = 0x00000000;
        Ac97Ctrl &= (~AC97_XMT_ENA);
    #endif

    return 1;
}


/*

Routine Description:

    Start record.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisStartRec2(void)
{
    //#if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) ||  (AUDIO_OPTION == AUDIO_IIS_ALC5621) ||(AUDIO_OPTION == AUDIO_ADC_DAC) )
    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) ||  (AUDIO_OPTION == AUDIO_IIS_ALC5621))
        IisCtrl    |= IIS_RCV_ENA | IIS_ENA;
    #elif (AUDIO_OPTION == AUDIO_ADC_DAC)
        IisCtrl    |= IIS_ENA;
//        AdcCtrlReg |= ADC_RX_ENA;
    #elif (AUDIO_OPTION == AUDIO_AC97_ALC203)
        Ac97ICC     = 0x00000303;
        Ac97Ctrl   |= AC97_RCV_ENA | AC97_ENA;
    #endif

    return 1;   ;
}

/*

Routine Description:

    Stop record.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisStopRec2(void)
{

    if(guiIISRecDMAId2 != 0xFF)
    {
        marsDMAClose(guiIISRecDMAId2);
        guiIISRecDMAId2 = 0xFF;
    }

    #if( (AUDIO_OPTION == AUDIO_IIS) || (AUDIO_OPTION == AUDIO_IIS_WM8974) ||  (AUDIO_OPTION == AUDIO_IIS_ALC5621))
        IisCtrl &= ~IIS_RCV_ENA;
    #elif (AUDIO_OPTION == AUDIO_ADC_DAC)
        AdcCtrlReg &= ~ADC_RX_ENA;
    #elif (AUDIO_OPTION == AUDIO_AC97_ALC203)
        Ac97ICC = 0x00000000;
        Ac97Ctrl &= (~AC97_RCV_ENA);
    #endif

    return 1;
}

s32 iisSetPlayDma2(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;
    INT8U err;

    /* set write data dma */
    /* yc: 2006.07.24: S */
    dmaCfg.src = (u32)buf;
    dmaCfg.dst = (u32)&(IisTxData);
/*Peter 1109 S*/
    dmaCfg.burst    = 1; /*CY 0917*/
    dmaCfg.cnt      = siz / 16;
/*Peter 1109 E*/
    //if (dmaConfig(DMA_REQ_IIS_PLAY, &dmaCfg) == 0)
    //    return 0;
    marsDMAClearCancelFlag();
    guiIISPlayDMAId2 =0xFF;
    guiIISPlayDMAId2 = marsDMAReq(DMA_REQ_IIS_PLAY, &dmaCfg);

    DEBUG_DMA("ES_LIGHTING DMA=%d: %X, %X, %X \r\n", guiIISPlayDMAId2, dmaCfg.src, dmaCfg.dst, dmaCfg.cnt);
    /* yc: 2006.07.24: E */
    iisStartPlay2();
    return 1;
}

/*

Routine Description:

    Set recording dma.

Arguments:

    buf - The buffer to record to.
    siz - The size to record.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisSetRecDma2(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;

    DEBUG_IIS("iisSetRecDma2:guiIISRecDMAId2=%x\n",guiIISRecDMAId2);
  #if(AUDIO_OPTION == AUDIO_ADC_DAC)
    dmaCfg.src = (u32)&(AdcRecData);
  #else
    dmaCfg.src = (u32)&(IisRxData);
  #endif
    /* yc: 2006.07.24: S */
    dmaCfg.dst = (u32)buf;
/*Peter 1109 S*/
    dmaCfg.burst    = 1; /*CY 0917*/
    dmaCfg.cnt      = siz / 16; // 4 cycle* 4 bytes(word)
/*Peter 1109 S*/
    //if (dmaConfig(DMA_REQ_IIS_RECORD, &dmaCfg) == 0)
    //    return 0;
    marsDMAClearCancelFlag();
    guiIISRecSize = siz;
    guiIISRecDMAId2 = 0xFF ;
    guiIISRecDMAId2 = marsDMAReq(DMA_REQ_IIS_RECORD, &dmaCfg);


    DEBUG_IIS("iisSetRecDma2: guiIISRecDMAId2=%x\n",guiIISRecDMAId2);

    /* yc: 2006.07.24: E */

    return 1;
}

//mode 0: record
//mode 1: Playback
INT32U iisWaitForIntOrCancel(INT32U mode, INT16U Timeout, INT32U* RecordLen)
{
    INT8U err;
    volatile INT32U *pDMARegCmmd;
    volatile INT32U *pDMACycCnt;
    INT32U uiDMAId, uiEet, uiCycleCnt;

    if (mode == 0)
        uiDMAId = guiIISRecDMAId2 ;
    else
        uiDMAId = guiIISPlayDMAId2 ;


    uiEet = marsDMAWaitForIntOrCancel(uiDMAId, Timeout, &uiCycleCnt);

    DEBUG_IIS("iisWaitForIntOrCancel: uiEet=%d, CycleCnt=%X  \n",uiEet , uiCycleCnt);


    if((uiEet == 2) && (mode==0))
    {
        *RecordLen =guiIISRecSize - (uiCycleCnt+1)*16;
        if (*RecordLen >= guiIISRecSize)
        {
            DEBUG_IIS("ERROR: iisWaitForIntOrCancel: RecordLen = 0xX is too large  \n",*RecordLen);
            *RecordLen = guiIISRecSize;
        }
    }

    return uiEet;
}

//mode 0: record
//mode 1: Playback
INT32U iisCancelDMA(INT32U mode)
{

    marsDMACancel();

    return 1;
}

s32 iisCloseDMA(u8 mode)
{

    DEBUG_IIS("iisCloseDMA,  ID=%d \n", guiIISPlayDMAId2);
    if(guiIISPlayDMAId2 != 0xFF)
    {
        marsDMAClose(guiIISPlayDMAId2);
        guiIISPlayDMAId2 = 0xFF;
    }

    return 1;
}

s32 iisGetPlayDmaStatue(void)
{
    INT32U Status;

    if(guiIISPlayDMAId2==0xff)
        return 9;

    Status = marsDMAGetIntSta(guiIISPlayDMAId2);

    if ( Status == 0)
        return 0;
    else    // 1: finish,  2:error
        return 1;

}



#endif
/*

Routine Description:

    Set recording dma.

Arguments:

    buf - The buffer to record to.
    siz - The size to record.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisSetRecDma(u8* buf, u32 siz)
{
    DMA_CFG dmaCfg;
    INT8U err;
    u32   ADCRecGrp;

    OSSemPend(gIISRecUseSem, OS_IPC_WAIT_FOREVER, &err);
    /* yc: 2006.07.24: S */
    /* set read data dma */
  #if(AUDIO_OPTION == AUDIO_ADC_DAC)
    #if((CHIP_OPTION == CHIP_A1016A))
	dmaCfg.src = (u32)&(AdcRecData_G0);
    #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
	dmaCfg.src = (u32)&(AdcRecData_G0);
    #else
	dmaCfg.src = (u32)&(AdcRecData);
    #endif
  #else
    dmaCfg.src = (u32)&(IisRxData);
  #endif
    /* yc: 2006.07.24: S */
    dmaCfg.dst = (u32)buf;
/*Peter 1109 S*/
    dmaCfg.burst    = 1; /*CY 0917*/
    dmaCfg.cnt      = siz / 16; // 4 cycle* 4 bytes(word)
/*Peter 1109 S*/
    //if (dmaConfig(DMA_REQ_IIS_RECORD, &dmaCfg) == 0)
    //    return 0;

	#if(AUDIO_OPTION == AUDIO_ADC_DAC)
    #if((CHIP_OPTION == CHIP_A1016A))
	guiIISRecDMAId = marsDMAReq(DMA_REQ_IIS_RECORD, &dmaCfg);
    #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
	guiIISRecDMAId = marsDMAReq(DMA_REQ_IIS_RECORD, &dmaCfg);
    #else
    guiIISRecDMAId = marsDMAReq(DMA_REQ_IIS_RECORD, &dmaCfg);
    #endif
    #else
    guiIISRecDMAId = marsDMAReq(DMA_REQ_IIS_RECORD, &dmaCfg);
    #endif
    /* yc: 2006.07.24: E */

    return 1;
}
#if IIS_TEST
s32 iisSetRecDma_ch(u8* buf, u32 siz, u8 ch)
{
    DMA_CFG dmaCfg;
    INT8U err;
    u32   ADCRecGrp;

    if(ch==0)
        OSSemPend(gIISCh0RecUseSem, OS_IPC_WAIT_FOREVER, &err);
    else if(ch==1)
        OSSemPend(gIISCh1RecUseSem, OS_IPC_WAIT_FOREVER, &err);
    else if(ch==2)
        OSSemPend(gIISCh2RecUseSem, OS_IPC_WAIT_FOREVER, &err);
    else if(ch==3)
        OSSemPend(gIISCh3RecUseSem, OS_IPC_WAIT_FOREVER, &err);

    /* yc: 2006.07.24: S */
    /* set read data dma */
  #if(AUDIO_OPTION == AUDIO_ADC_DAC)
    #if((CHIP_OPTION == CHIP_A1016A))
    if(ch == 0)
        dmaCfg.src = (u32)&(AdcRecData_G0);
    else if(ch == 1)
        dmaCfg.src = (u32)&(AdcRecData_G1);
    #elif((CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
    if(ch == 0)
        dmaCfg.src = (u32)&(AdcRecData_G0);
    else if(ch == 1)
        dmaCfg.src = (u32)&(AdcRecData_G1);
    #else
	dmaCfg.src = (u32)&(AdcRecData);
    #endif
  #else
    dmaCfg.src = (u32)&(IisRxData);
  #endif
    /* yc: 2006.07.24: S */
    dmaCfg.dst = (u32)buf;
/*Peter 1109 S*/
    dmaCfg.burst    = 1; /*CY 0917*/
    dmaCfg.cnt      = siz / 16; // 4 cycle* 4 bytes(word)
/*Peter 1109 S*/
    //if (dmaConfig(DMA_REQ_IIS_RECORD, &dmaCfg) == 0)
    //    return 0;

	#if(AUDIO_OPTION == AUDIO_ADC_DAC)
    #if((CHIP_OPTION == CHIP_A1016A))
	if(ch == 0)
	{
        guiIISCh0RecDMAId = marsDMAReq(DMA_REQ_IIS_RECORD, &dmaCfg);
        //DEBUG_IIS("Ch0 RecDMAId= %d\n",guiIISCh0RecDMAId);
	}
    else if(ch == 1)
    {
        guiIISCh1RecDMAId = marsDMAReq(DMA_REQ_ADC_G1_RECORD, &dmaCfg);
        //DEBUG_IIS("Ch1 RecDMAId= %d\n",guiIISCh1RecDMAId);
    }
    #elif((CHIP_OPTION == CHIP_A1016B) || \
        (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
	if(ch == 0)
	{
        guiIISCh0RecDMAId = marsDMAReq(DMA_REQ_IIS_RECORD, &dmaCfg);
        //DEBUG_IIS("Ch0 RecDMAId= %d\n",guiIISCh0RecDMAId);
	}
    else if(ch == 1)
    {
        guiIISCh1RecDMAId = marsDMAReq(DMA_REQ_ADC_G1_RECORD, &dmaCfg);
        //DEBUG_IIS("Ch1 RecDMAId= %d\n",guiIISCh1RecDMAId);
    }
    #else
    guiIISRecDMAId = marsDMAReq(DMA_REQ_IIS_RECORD, &dmaCfg);
    #endif
    #else
    guiIISRecDMAId = marsDMAReq(DMA_REQ_IIS_RECORD, &dmaCfg);
    #endif
    /* yc: 2006.07.24: E */

    return 1;
}
#endif
/*

Routine Description:

    Check play dma ready.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisCheckPlayDmaReady(void)
{
    u8 err;

    err = marsDMACheckReady(guiIISPlayDMAId);
    guiIISPlayDMAId = 0x55;
    //DEBUG_IIS("iisCheckPlayDmaReady \r\n");
    OSSemPost(gIISPlayUseSem);
    //DEBUG_IIS("iisPlayready post = %d", gIISPlayUseSem->OSEventCnt);

    return (err);

}

/*

Routine Description:

    Check recording dma ready.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisCheckRecDmaReady(void)
{
    u8 err;

    err = marsDMACheckReady(guiIISRecDMAId);
    guiIISRecDMAId = 0x55;
    OSSemPost(gIISRecUseSem);

    return (err);

}
#if IIS_TEST
s32 iisCheckRecDmaReady_ch(u8 ch)
{
    u8 err;

    if(ch==0)
    {
        err = marsDMACheckReady(guiIISCh0RecDMAId);
        guiIISCh0RecDMAId = 0x55;
        OSSemPost(gIISCh0RecUseSem);
    }
    else if(ch==1)
    {
        err = marsDMACheckReady(guiIISCh1RecDMAId);
        guiIISCh1RecDMAId = 0x55;
        OSSemPost(gIISCh1RecUseSem);
    }
    else if(ch==2)
    {
        err = marsDMACheckReady(guiIISCh2RecDMAId);
        guiIISCh2RecDMAId = 0x55;
        OSSemPost(gIISCh2RecUseSem);
    }
    else if(ch==3)
    {
        err = marsDMACheckReady(guiIISCh3RecDMAId);
        guiIISCh3RecDMAId = 0x55;
        OSSemPost(gIISCh3RecUseSem);
    }
    return (err);
}
#endif
/*Lsk 090327 S*/
/*

Routine Description:

    Adjust IIS frequency

Arguments:

    level  - difference between IISTime and IDUInterruptTime

Return Value:

    None.

*/
//void AdjustIISFreq(s64 Offset)
void AdjustIISFreq(s32 level)
{
	//#define IIS_MODIFY_THRESHOLD   26693
	//s8 level = Offset/IIS_MODIFY_THRESHOLD;

	//DEBUG_IIS("%2d ",level);
	if(level > 10)
		level = 10;
	if(level < -10)
		level = -10;
	#if(IIS_SAMPLE_RATE == 8000)
	switch(level)
	{
        #if(ADC_IIS_CLK_FREQ == 24000000)
        case 0: //7990
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_15 | IIS_CLK_DIV_B_10;
			break;
		case -1: //8021
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_16 | IIS_CLK_DIV_B_10;
			break;
		case 1:  //7955
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_14 | IIS_CLK_DIV_B_10;
			break;
		case -2: //8049
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_17 | IIS_CLK_DIV_B_10;
			break;
		case 2:  //7914
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_13 | IIS_CLK_DIV_B_10;
			break;
		case -3: //8074
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_18 | IIS_CLK_DIV_B_10;
			break;
		case 3:  //7867
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_12 | IIS_CLK_DIV_B_10;
			break;
		case -4: //8097
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_19 | IIS_CLK_DIV_B_10;
			break;
		case 4:  //7813
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_11 | IIS_CLK_DIV_B_10;
			break;
		case -5: //8117
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_20 | IIS_CLK_DIV_B_10;
			break;
		case 5:  //7748
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_10 | IIS_CLK_DIV_B_10;
			break;
        case -6: //8149
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_22 | IIS_CLK_DIV_B_10;
			break;
		case 6:  //7717
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_1 | IIS_CLK_DIV_B_11;
			break;
		case -7: //8194
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_25 | IIS_CLK_DIV_B_10;
			break;
		case 7:  //7677
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_57 | IIS_CLK_DIV_B_11;
			break;
		case -8: //8238
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_29 | IIS_CLK_DIV_B_10;
			break;
		case 8:  //7630
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_42 | IIS_CLK_DIV_B_11;
			break;
		case -9: //8297
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_37 | IIS_CLK_DIV_B_10;
			break;
		case 9:  //7582
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_33 | IIS_CLK_DIV_B_11;
			break;
		case -10: //8364
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_53 | IIS_CLK_DIV_B_10;
			break;
		case 10:  //7518
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_26 | IIS_CLK_DIV_B_11;
			break;
		default: //7990
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_15 | IIS_CLK_DIV_B_10;
        #elif(ADC_IIS_CLK_FREQ == 32000000)
        case 0: //8000
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_24 | IIS_CLK_DIV_B_14;
			break;
		case -1: //8024
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_26 | IIS_CLK_DIV_B_14;
			break;
		case 1:  //7986
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_23 | IIS_CLK_DIV_B_14;
			break;
		case -2: //8035
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_9 | IIS_CLK_DIV_B_13;
			break;
		case 2:  //7954
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_21 | IIS_CLK_DIV_B_14;
			break;
		case -3: //8064
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_30 | IIS_CLK_DIV_B_14;
			break;
		case 3:  //7936
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_8 | IIS_CLK_DIV_B_13;
			break;
		case -4: //8080
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_32 | IIS_CLK_DIV_B_14;
			break;
		case 4:  //7916
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_19 | IIS_CLK_DIV_B_14;
			break;
		case -5: //8101
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_35 | IIS_CLK_DIV_B_14;
			break;
		case 5:  //7894
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_18 | IIS_CLK_DIV_B_14;
			break;
        case -6: //8129
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_40 | IIS_CLK_DIV_B_14;
			break;
		case 6:  //7870
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_17 | IIS_CLK_DIV_B_14;
			break;
		case -7: //8166
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_49 | IIS_CLK_DIV_B_14;
			break;
		case 7:  //7823
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_16 | IIS_CLK_DIV_B_14;
			break;
		case -8: //8219
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_72 | IIS_CLK_DIV_B_14;
			break;
		case 8:  //7776
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_214 | IIS_CLK_DIV_B_15;
			break;
		case -9: //8288
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_183 | IIS_CLK_DIV_B_14;
			break;
		case 9:  //7722
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_86 | IIS_CLK_DIV_B_15;
			break;
		case -10: //8370
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_15 | IIS_CLK_DIV_B_13;
			break;
		case 10:  //7660
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_50 | IIS_CLK_DIV_B_15;
			break;
		default: //8000
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_24 | IIS_CLK_DIV_B_14;
       #elif(ADC_IIS_CLK_FREQ == 48000000)
        case 0: //8001
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_53 | IIS_CLK_DIV_B_22;
			break;
		case -1: //8020
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_61 | IIS_CLK_DIV_B_22;
			break;
		case 1:  //7978
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_46 | IIS_CLK_DIV_B_22;
			break;
		case -2: //8040
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_72 | IIS_CLK_DIV_B_22;
			break;
		case 2:  //7958
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_41 | IIS_CLK_DIV_B_22;
			break;
		case -3: //8060
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_88 | IIS_CLK_DIV_B_22;
			break;
		case 3:  //7937
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_37 | IIS_CLK_DIV_B_22;
			break;
		case -4: //8080
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_112 | IIS_CLK_DIV_B_22;
			break;
		case 4:  //7919
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_34 | IIS_CLK_DIV_B_22;
			break;
		case -5: //8100
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_156 | IIS_CLK_DIV_B_22;
			break;
		case 5:  //7897
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_31 | IIS_CLK_DIV_B_22;
			break;
        case -6: //8135
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_21 | IIS_CLK_DIV_B_21;
			break;
		case 6:  //7867
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_12 | IIS_CLK_DIV_B_21;
			break;
		case -7: //8167
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_23 | IIS_CLK_DIV_B_21;
			break;
		case 7:  //7826
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_24 | IIS_CLK_DIV_B_22;
			break;
		case -8: //8217
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_27 | IIS_CLK_DIV_B_21;
			break;
		case 8:  //7779
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_233 | IIS_CLK_DIV_B_23;
			break;
		case -9: //8290
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_13 | IIS_CLK_DIV_B_20;
			break;
		case 9:  //7724
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_88 | IIS_CLK_DIV_B_23;
			break;
		case -10: //8375
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_57 | IIS_CLK_DIV_B_21;
			break;
		case 10:  //7662
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_51 | IIS_CLK_DIV_B_23;
			break;
		default: //7990
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_15 | IIS_CLK_DIV_B_10;

        //Toby
        #elif(ADC_IIS_CLK_FREQ == 54000000)
        case 0: //8000
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_71 | IIS_CLK_DIV_B_25;
			break;
		case -1: //8024
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_7 | IIS_CLK_DIV_B_22;
			break;
		case 1:  //7973
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_57 | IIS_CLK_DIV_B_25;
			break;
		case -2: //8048
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_124 | IIS_CLK_DIV_B_25;
			break;
		case 2:  //7943
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_47 | IIS_CLK_DIV_B_25;
			break;
		case -3: //8072
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_198 | IIS_CLK_DIV_B_22;
			break;
		case 3:  //7924
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_42 | IIS_CLK_DIV_B_25;
			break;
		case -4: //8099
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_24 | IIS_CLK_DIV_B_24;
			break;
		case 4:  //7904
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_38 | IIS_CLK_DIV_B_25;
			break;
		case -5: //8124
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_26 | IIS_CLK_DIV_B_24;
			break;
		case 5:  //7881
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_34 | IIS_CLK_DIV_B_25;
			break;
 	    case -6: //8124
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_26 | IIS_CLK_DIV_B_24;
			break;
		case 6:  //7881
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_34 | IIS_CLK_DIV_B_25;
			break;
		case -7: //8169
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_30 | IIS_CLK_DIV_B_24;
			break;
		case 7:  //7833
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_28 | IIS_CLK_DIV_B_25;
			break;
		case -8: //8243
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_42 | IIS_CLK_DIV_B_24;
			break;
		case 8:  //7763
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_157 | IIS_CLK_DIV_B_26;
			break;
		case -9: //8342
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_88 | IIS_CLK_DIV_B_24;
			break;
		case 9:  //7670
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_4 | IIS_CLK_DIV_B_21;
			break;
		case -10: //8465
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_12 | IIS_CLK_DIV_B_22;
			break;
		case 10:  //7552
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_29 | IIS_CLK_DIV_B_26;
			break;
		default: //8000
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_71 | IIS_CLK_DIV_B_25;
        #endif
        //Toby

	}
    #elif(IIS_SAMPLE_RATE == 16000)
    switch(level)
	{
    //Toby
    #if(ADC_IIS_CLK_FREQ == 48000000)
        case 0: //16000
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_24 | IIS_CLK_DIV_B_5;
			break;
		case -1: //16048
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_26 | IIS_CLK_DIV_B_5;
			break;
		case 1:  //15952
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_22 | IIS_CLK_DIV_B_5;
			break;
		case -2: //16091
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_28 | IIS_CLK_DIV_B_5;
			break;
		case 2:  //15909
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_21 | IIS_CLK_DIV_B_5;
			break;
		case -3: //16145
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_31 | IIS_CLK_DIV_B_5;
			break;
		case 3:  //15856
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_20 | IIS_CLK_DIV_B_5;
			break;
		case -4: //16190
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_34 | IIS_CLK_DIV_B_5;
			break;
		case 4:  ///15789
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_18| IIS_CLK_DIV_B_5;
			break;
		case -5: //16239
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_38 | IIS_CLK_DIV_B_5;
			break;
		case 5:  //15740
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_17 | IIS_CLK_DIV_B_5;
			break;
  	    case -6: //16339
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_50 | IIS_CLK_DIV_B_5;
			break;
		case 6:  //15686
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_16 | IIS_CLK_DIV_B_5;
			break;
		case -7: //16486
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_91 | IIS_CLK_DIV_B_5;
			break;
		case 7:  //15555
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_14 | IIS_CLK_DIV_B_5;
			break;
		case -8: //16666
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_1 | IIS_CLK_DIV_B_2;
			break;
		case 8:  //15384
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_12 | IIS_CLK_DIV_B_5;
			break;
		case -9: //17142
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_6 | IIS_CLK_DIV_B_4;
			break;
		case 9:  ///15151
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_10 | IIS_CLK_DIV_B_5;
			break;
		case -10: //17500
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_7 | IIS_CLK_DIV_B_4;
			break;
		case 10:  //14814
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_8 | IIS_CLK_DIV_B_5;
			break;
		default: //16071
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_27 | IIS_CLK_DIV_B_5;

        //Toby
    #elif(ADC_IIS_CLK_FREQ == 54000000)
        case 0: //16000=30000*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_225 | IIS_CLK_DIV_B_6;
			break;
		case -1: //16040=30075*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_6 | IIS_CLK_DIV_B_5;
			break;
		case 1:  //15956=29917*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_138 | IIS_CLK_DIV_B_6;
			break;
		case -2: //16080=30150*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_6 | IIS_CLK_DIV_B_5;
			break;
		case 2:  //15916=29842*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_102 | IIS_CLK_DIV_B_6;
			break;
		case -3: //16120=30225*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_7 | IIS_CLK_DIV_B_5;
			break;
		case 3:  //15874=29763*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_80 | IIS_CLK_DIV_B_6;
			break;
		case -4: //16160=30300*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_7 | IIS_CLK_DIV_B_5;
			break;
		case 4:  //15838=29696*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_68 | IIS_CLK_DIV_B_6;
			break;
		case -5: //16200=30375*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_7 | IIS_CLK_DIV_B_5;
			break;
		case 5:  //15794=29613*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_57 | IIS_CLK_DIV_B_6;
			break;
        case -6: //16258
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_7 | IIS_CLK_DIV_B_5;
			break;
        case 6:  //15736
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_47 | IIS_CLK_DIV_B_6;
			break;
		case -7: //16347
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_7 | IIS_CLK_DIV_B_5;
			break;
		case 7:  //15569
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_31 | IIS_CLK_DIV_B_6;
			break;
		case -8: //16470
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_7 | IIS_CLK_DIV_B_5;
			break;
		case 8:  //15535
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_29 | IIS_CLK_DIV_B_6;
			break;
		case -9: //16626
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_8 | IIS_CLK_DIV_B_5;
			break;
		case 9:  ///15401
		    IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_23 | IIS_CLK_DIV_B_6;
			break;
		case -10: //16801
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_3 | IIS_CLK_DIV_B_4;
			break;
		case 10:  //15225
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_18 | IIS_CLK_DIV_B_6;
			break;
		default: //16000=30000*8/15
			IisCtrl = (IisCtrl & 0xff0000ff)  | IIS_CLK_DIV_A_225 | IIS_CLK_DIV_B_6;
    #endif
    }
    #endif
}


/*Peter 1109 S*/
/*

Routine Description:

    The IIS task.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
void iisTask(void* pData)
{
    u32*    pTime;
    u32*    pSize;
    u8*     pBuf;
    u8      err;
    s32     i, j, interval;
    u32     status;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
#if RF_TX_OPTIMIZE  
    int RFUnit;
#endif


    //-------------------------------------//
    interval    = 0;
    pBuf = pBuf;    /* avoid warning */

#if RF_TX_OPTIMIZE      
    iisSounBufMngWriteIdx   = 0;
    iisSounBufMngReadIdx    = 0;
    IISMode                 = 0;    // 0: record, 1: playback
    IISTime                 = 0;    /* Peter 070104 */
    IISTimeUnit             = (IIS_RECORD_SIZE * 1000) / IIS_SAMPLE_RATE;  /* milliscends */ /* Peter 070104 */
    CurrentAudioSize        = 0;
    /* initialize sound buffer */
    for(i = 0; i < IIS_BUF_NUM; i++) {
        iisSounBufMng[i].buffer     = iisSounBuf[i];
    }
    RFUnit= (int)pData;
#endif


    while (1)
    {
    #if RF_TX_OPTIMIZE  
        if(gRfiuUnitCntl[RFUnit].TX_IIS_Stop)
        {
           DEBUG_MP4("&");
           OSTimeDly(1);
           continue;
        }

    #else
    	IIS_Task_Pend = 1;
        OSSemPend(iisTrgSemEvt, OS_IPC_WAIT_FOREVER, &err);
        IIS_Task_Pend = 0;

        if (err != OS_NO_ERR)
        {
            DEBUG_IIS("Error: iisTrgSemEvt is %d.\n", err);
            //return;
        }
    #endif

        if(IISMode == 1) // playback
        {
            OSSemPend(gIISPlayUseSem, OS_IPC_WAIT_FOREVER, &err);
			if(sysTVOutOnFlag) //TV-out
			{
				#if TV_DISP_BY_IDU
    				#if(AVSYNC == AUDIO_FOLLOW_VIDEO)
    				//AdjustIISFreq(IISTime - (IDUInterruptTime / 3));
    				AdjustIISFreq((s32)((IISTime - (IDUInterruptTime / 3)) / 26693));
    				#endif
				#endif
			}
  			pTime = &iisSounBufMng[iisSounBufMngReadIdx].time;
            pSize = &iisSounBufMng[iisSounBufMngReadIdx].size;
            pBuf  = iisSounBufMng[iisSounBufMngReadIdx].buffer;

            // for video playback pause and resume
            if(IISPplyback!=1)
            {
                if(sysPlaybackVideoPause)
                {
                    DEBUG_IIS("Enter video playback pause mode\n");
                    gucIISPlayDMAPause = 1;
					if(sysTVOutOnFlag && TVout_Generate_Pause_Frame)
					{
    					//Lsk 090511 : use top field of next frame to generate PAUSE frame, and store in the current frame address so clear flag
					    if(MainVideodisplaybuf_idx == 0)
                            asfPausePlayback(MainVideodisplaybuf[0], MainVideodisplaybuf[1], 2);
                        else
                        	asfPausePlayback(MainVideodisplaybuf[(MainVideodisplaybuf_idx-1) % DISPLAY_BUF_NUM],
  			                	   			 MainVideodisplaybuf[(MainVideodisplaybuf_idx) % DISPLAY_BUF_NUM],
  				   			                 1);
					}

                    while(sysPlaybackVideoPause && sysPlaybackVideoStart == 1 &&
					      #if (AVSYNC == VIDEO_FOLLOW_AUDIO)
                          sysPlaybackVideoStop == 0 && sysPlaybackBackward == 0)
						  #elif (AVSYNC == AUDIO_FOLLOW_VIDEO)
						  sysPlaybackVideoStop == 0 )
						  #endif
                    {
                        OSTimeDly(1);
                    }
                    //gucIISPlayDMAPause = 0; //Exit video playback pause mode, reset 0
                    DEBUG_IIS("Exit video playback pause mode\n");
                }
            } // if(IISPplyback!=1)
            j   = *pSize / IIS_PLAYBACK_SIZE;
            if(IISPplyback != 1)
            {
                if(sysPlaybackForward != SYS_PLAYBACK_FORWARD_X1)
                {
                    j  /= 4;
                }
                if(sysPlaybackForward == SYS_PLAYBACK_FORWARD_X8)
                {
                    interval    = !interval;
                }
            }  //    if(IISPplyback!=1)

            //iisDMABurst_Count=0;
            //iisDMABurst_ena=0;
            //iisDMABurst_CountMax=(j-1);
            if (sysPlaybackVideoStop==0)
            {
                if(guiIISPlayDMAId == 0xFF)
                {
                    marsDMAOpen(&guiIISPlayDMAId, iisPlayDMA_ISR);
                    DEBUG_IIS("guiIISPlayDMAId = %d \r\n", guiIISPlayDMAId);
                    gucIISPlayDMACurrBufIdx=0;
                    gucIISPlayDMANextBufIdx=0;
                }
                for(i = 0; i < j; i++)
                {
                    if( (sysPlaybackForward != SYS_PLAYBACK_FORWARD_X8) || interval ||(IISPplyback==1) )
                    {
                        //iisSetPlayDma(pBuf, IIS_PLAYBACK_SIZE);
                        //iisStartPlay();
                        gpIISPlayDMANextBuf[gucIISPlayDMANextBufIdx] = pBuf;
                        //DEBUG_IIS("%d", gucIISPlayDMANextBufIdx);
                        /**********************************************
                        *** 1. read more data in, and save pufIdx.  ***
                        *** 2. Use DMA move to IIS in ISR function. ***
                        *** 3. Sync ISR and read data.              ***
                        **********************************************/
                        if(gucIISPlayDMANextBufIdx == 15)
                            gucIISPlayDMANextBufIdx=0;
                        else
                            gucIISPlayDMANextBufIdx++;

                        if(gucIISPlayDMAStarting == 0)
                        {
                            gucIISPlayDMAStarting = 1;
                        	//DEBUG_IIS("gucIISPlayDMAStarting = %d \r\n", gucIISPlayDMAStarting);
                            iisSetNextPlayDMA((u8*)gpIISPlayDMANextBuf[gucIISPlayDMACurrBufIdx], IIS_PLAYBACK_SIZE);
                            iisStartPlay();
                            //gucIISPlayDMAStarting = 1;
                        }

                        if(gucIISPlayDMACurrBufIdx == gucIISPlayDMANextBufIdx)
                        {
                            while((gucIISPlayDMACurrBufIdx == gucIISPlayDMANextBufIdx)&&(gucIISPlayDMAStarting)&&(!IIS_Task_Stop))
                            {  //DEBUG_IIS("4");
                               OSTimeDly(1);
                            }
                        }

                        #if IIS_DEBUG_ENA
                            IIS_UnderRunSta=1;
                            gpioSetLevel(1, 6, 1);
                        #endif




                    }
                    if(IISPplyback == 1)
                    {
                       pBuf   += IIS_PLAYBACK_SIZE;
                    }
                    else if(sysPlaybackForward != SYS_PLAYBACK_FORWARD_X1)
                    {
                        pBuf   += IIS_PLAYBACK_SIZE * 4;
                    }
                    else
                    {
                        pBuf   += IIS_PLAYBACK_SIZE;
                    }

					#if (PLAYBACK_METHOD==PLAYBACK_IN_IIS_TASK)
                    if(IISPplyback==1)
                        IISTime    += IISTimeUnit;
                    if(sysPlaybackForward != SYS_PLAYBACK_FORWARD_X1)
                    {
                        IISTime    += IISTimeUnit * 4;  // microsecond unit
                    }
                    else
                    {
                        IISTime    += IISTimeUnit;  // microsecond unit
                    }
					#endif

					#if( (PLAYBACK_METHOD==PLAYBACK_IN_IIS_TASK))
                    if(IISPplyback!=1)
                    {
                    	if(sysTVOutOnFlag) //TV-out
                        {
                        	#if TV_DISP_BY_IDU
        						#if TV_DEBUG_ENA
            	        	       gpioSetLevel(1, 18, 1);
                		        #endif
                            #else
        						while((IISTime >= VideoNextPresentTime) && (MainVideodisplaybuf_idx < IsuIndex))
        						{
        							#if TV_DEBUG_ENA
        		    		        gpioSetLevel(1, 18, 1);
                             		#endif
                                  	iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
        							#if TV_DEBUG_ENA
            	                    gpioSetLevel(1, 18, 0);
        	  			            #endif
        	        	            MainVideodisplaybuf_idx++;
            	        	        while(VideoNextPresentTime >= Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM] && CloseFlag)
                	        	    {
                	        	    	OSTimeDly(1);
        							}
        	                    	VideoNextPresentTime    = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
        						}
                            #endif
    					}
                        else //Pannel-out
                        {
                        	//while((IISTime >= (VideoNextPresentTime - Video_timebase)) && (MainVideodisplaybuf_idx < IsuIndex)) //Lsk 090507 : when FF or BF speed return normal speed
                        	//while((IISTime >= (VideoNextPresentTime - Video_timebase)) && (MainVideodisplaybuf_idx < IsuIndex - 1 )) //Lsk 090507 : when FF or BF speed return normal speed
                            while((IISTime >= (VideoNextPresentTime - Video_timebase)) && ((MainVideodisplaybuf_idx + 1) < IsuIndex))   						
                            {
                          		iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
    							#if TV_DEBUG_ENA
        	                    gpioSetLevel(1, 18, 0);
    	  			            #endif
    	        	            MainVideodisplaybuf_idx++;
    	        	        #if 0   //Marked by Peter for fixing ROULE_DOORPHONE playback deadlock bug
    	        	            while(VideoNextPresentTime >= Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM] && sysPlaybackVideoStop==0 && CloseFlag && !ResetPlayback)
        	        	        //while(VideoNextPresentTime >= Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM] && CloseFlag && !ResetPlayback)
            	        	    {
            	        	    	OSTimeDly(1);
                    	        }
                    	    #endif
    	                    	VideoNextPresentTime    = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
                            }
    					}
    				}
					#endif
    			}
                //iisDMABurst_ena=0;
                iisPlayCount++;
                iisSounBufMngReadIdx    = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
            }
            else
                OSTimeDly(2);

            //DEBUG_IIS("iisSounBufMngReadIdx=%x \r\n", iisSounBufMngReadIdx);

            //DEBUG_IIS("x %d", gIISPlayUseSem->OSEventCnt);
            OSSemPost(gIISPlayUseSem);
            //DEBUG_IIS("y %d", gIISPlayUseSem->OSEventCnt);
        }
        else if(IISMode == 0) //----------------------Recording-------------------------//
        {        // record
            //iisDMABurst_ena=0;
            pTime = &iisSounBufMng[iisSounBufMngWriteIdx].time;
            pSize = &iisSounBufMng[iisSounBufMngWriteIdx].size;
            pBuf  = iisSounBufMng[iisSounBufMngWriteIdx].buffer;

            *pTime = IIS_CHUNK_TIME;
#if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MP4)
            *pSize = IIS_CHUNK_SIZE;
            *pTime  = (*pSize * 1000) / iisRecFormat.nAvgBytesPerSec;   //  sound length in millisecond unit
#elif (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_ASF)
            *pSize = IIS_CHUNK_SIZE;
            *pTime  = (*pSize * 1000) / iisRecFormat.nAvgBytesPerSec;   //  sound length in millisecond unit
#elif (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_AVI)
            *pSize  = IIS_CHUNK_SIZE;
            *pTime  = (*pSize * 1000) / iisRecFormat.nAvgBytesPerSec;   //  sound length in millisecond unit
#elif (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MOV)
            *pSize = IIS_CHUNK_SIZE;
		    *pTime  = (*pSize * 1000) / iisRecFormat.nAvgBytesPerSec;   //  sound length in millisecond unit
#endif



            /* Peter 070104 */
			if(guiIISRecDMAId == 0xFF)
			{
			    marsDMAOpen(&guiIISRecDMAId, iisRecDMA_ISR);
			    DEBUG_IIS("guiIISRecDMAId = %d \r\n", guiIISRecDMAId);
			    gucIISRecDMACurrBufIdx=0;
			    gucIISRecDMANextBufIdx=0;
			}

            j   = *pSize / IIS_RECORD_SIZE;            
            for(i = 0; i < j; i++)
	        {
				gpIISRecDMANextBuf[gucIISRecDMANextBufIdx] = pBuf;
				pBuf   += IIS_RECORD_SIZE;
				/**********************************************
				*** 1. read more data in, and save pufIdx.  ***
				*** 2. Use DMA move to IIS in ISR function. ***
				*** 3. Sync ISR and read data.              ***
				**********************************************/
				if(gucIISRecDMANextBufIdx == 15)
					gucIISRecDMANextBufIdx=0;
				else
					gucIISRecDMANextBufIdx++;

				if(gucIISRecDMAStarting == 0)
                {
                    gucIISRecDMAStarting = 1;
                	//DEBUG_IIS("1.gucIISRecDMAStarting = %d \r\n", gucIISRecDMAStarting);
                    #if (Audio_mode == AUDIO_AUTO)
                    iisSetNextRecDMA_auto((u8*)gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx], IIS_RECORD_SIZE);
                    #else
                    iisSetNextRecDMA((u8*)gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx], IIS_RECORD_SIZE);
                    #endif
                    iisStartRec();
                }

                if(gucIISRecDMACurrBufIdx == gucIISRecDMANextBufIdx)
                {
                    while((gucIISRecDMACurrBufIdx == gucIISRecDMANextBufIdx)&&(gucIISRecDMAStarting)&&(!IIS_Task_Stop))
                    {
                    	//DEBUG_IIS("#");
                    	OSTimeDly(1);
                    }
                }

            }
            if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL))
            {
                if((WantChangeFile == 1) && (GetLastAudio == 0))
                {
                    OS_ENTER_CRITICAL();
                    LastAudio           = iisSounBufMngWriteIdx;
                    GetLastAudio        = 1;
                    OS_EXIT_CRITICAL();
                }
            }
            OS_ENTER_CRITICAL();
            CurrentAudioSize       += *pSize;
            OS_EXIT_CRITICAL();

            iisSounBufMngWriteIdx = (iisSounBufMngWriteIdx + 1) %  IIS_BUF_NUM;
        }   // end of record
#if AUDIO_IN_TO_OUT
        else if(IISMode == 2)   // Receive and playback and playback audio in preview mode
        {
            pBuf    = iisSounBufMng[iisSounBufMngWriteIdx].buffer;

			if(guiIISRecDMAId == 0xFF)
			{
			    marsDMAOpen(&guiIISRecDMAId, iisRecDMA_ISR);
			    DEBUG_IIS("guiIISRecDMAId = %d \r\n", guiIISRecDMAId);
			    gucIISRecDMACurrBufIdx=0;
			    gucIISRecDMANextBufIdx=0;
			}

            j   = IIS_CHUNK_SIZE / IIS_RECORD_SIZE;
            for(i = 0; i < j; i++)
	        {
	        	//DEBUG_IIS("{%d}",i);
				gpIISRecDMANextBuf[gucIISRecDMANextBufIdx] = pBuf;
				pBuf   += IIS_RECORD_SIZE;
				/**********************************************
				*** 1. read more data in, and save pufIdx.  ***
				*** 2. Use DMA move to IIS in ISR function. ***
				*** 3. Sync ISR and read data.              ***
				**********************************************/
				if(gucIISRecDMANextBufIdx == 15)
					gucIISRecDMANextBufIdx=0;
				else
					gucIISRecDMANextBufIdx++;

				if(gucIISRecDMAStarting == 0)
                {
                    gucIISRecDMAStarting = 1;
                	//DEBUG_IIS("2.gucIISRecDMAStarting = %d \r\n", gucIISRecDMAStarting);
                    iisSetNextRecDMA((u8*)gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx], IIS_RECORD_SIZE);
                    iisStartRec();
                }

                if(gucIISRecDMACurrBufIdx == gucIISRecDMANextBufIdx)
                {
                    while((gucIISRecDMACurrBufIdx == gucIISRecDMANextBufIdx)&&(gucIISRecDMAStarting)&&(!IIS_Task_Stop))
                    {
                    	//DEBUG_IIS("$");
                    	OSTimeDly(1);
                    }
                }
            }
			iisSounBufMngWriteIdx = (iisSounBufMngWriteIdx + 1) %  IIS_BUF_NUM;
        }
#endif
    }
}
/*Peter 1109 E*/

/*

Routine Description:

    Resume IIS task.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisResumeTask(void)
{
    volatile INT16U *pIISUseSemCnt = &gIISPlayUseSem->OSEventCnt;

    if(*pIISUseSemCnt == 0)
    {
        OSSemPost(gIISPlayUseSem);
    }

    /* Resume the task */
    //DEBUG_IIS("Trace: IIS task resuming.\n");
    iisPlayDMACnt = 0;
	iisRecDMACnt  = 0;
	IIS_Task_Stop= 0;
    OSTaskResume(IIS_TASK_PRIORITY);

    return 1;
}

/*

Routine Description:

    Suspend IIS task.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisSuspendTask(void)
{
    /* Suspend the task */
    //DEBUG_IIS("Trace: IIS task suspending.\n");
    OSTaskSuspend(IIS_TASK_PRIORITY);

    return 1;
}

/*********************************** AUDIO_IN_TO_OUT Begin **********************************/

#if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)

/*

Routine Description:

    The IIS playback task for playback the received audio data immediately.

Arguments:

    pData - The task parameter.

Return Value:

    None.

*/
void iisPlaybackTask(void* pData)
{
    u32*    pTime;
    u32*    pSize;
    u8*     pBuf;
    u8      err;
    u8 		i;


    while (1)
    {
    	IIS_PlayTask_Pend = 1;
		OSSemPend(iisPlaybackSemEvt, OS_IPC_WAIT_FOREVER, &err);  //IISTask recording mode
    	IIS_PlayTask_Pend = 0;
	   	if (err != OS_NO_ERR)
	    {
	        DEBUG_IIS("Error: iisPlaybackSemEvt is %d.\n", err);
	    }
#if REMOTE_TALK_BACK
		if(gucIISPlayDMANextBufIdx % (IIS_CHUNK_SIZE / IIS_PLAYBACK_SIZE) == 0)
        {
			pBuf                    = iisSounBufMng[iisSounBufMngReadIdx].buffer;
            iisSounBufMngReadIdx    = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
        }
#else
		if(iisPlayDMACnt % (IIS_CHUNK_SIZE / IIS_PLAYBACK_SIZE) == 0)
			pBuf    = iisSounBufMng[iisSounBufMngPlayIdx].buffer;
#endif
		if(guiIISPlayDMAId == 0xFF)
	    {
	        marsDMAOpen(&guiIISPlayDMAId, iisIOPlayDMA_ISR);
	        DEBUG_IIS("guiIISPlayDMAId = %d \r\n", guiIISPlayDMAId);
			gucIISPlayDMACurrBufIdx = 0;
            gucIISPlayDMANextBufIdx = 0;
	    }
		gpIISPlayDMANextBuf[gucIISPlayDMANextBufIdx] = pBuf;
        pBuf   += IIS_PLAYBACK_SIZE;

        /********************************************************
        *** 1. Save more buf addr in gpIISPlayDMANextBuf.     ***
        *** 2. Update next DMA addr in ISR function. 	      ***
        *** 3. Sync ISR and read data to void data overwrite. ***
        ********************************************************/
        if(gucIISPlayDMANextBufIdx == 15)
            gucIISPlayDMANextBufIdx = 0;
        else
            gucIISPlayDMANextBufIdx++;

		if(gucIISPlayDMAStarting == 0)
        {
            gucIISPlayDMAStarting = 1;
        	//DEBUG_IIS("gucIISPlayDMAStarting = %d \r\n", gucIISPlayDMAStarting);
            iisSetNextPlayDMA((u8*)gpIISPlayDMANextBuf[gucIISPlayDMACurrBufIdx], IIS_PLAYBACK_SIZE);
            iisStartPlay();
        }

		if(gucIISPlayDMACurrBufIdx == gucIISPlayDMANextBufIdx)
        {
            while((gucIISPlayDMACurrBufIdx == gucIISPlayDMANextBufIdx) && (gucIISPlayDMAStarting))
            {
            	//DEBUG_IIS("&");
                OSTimeDly(1);
            }
        }
    }
}
/*

Routine Description:

    Resume IIS Playback task.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisResumePlaybackTask(void)
{
    /* Resume the task */
    //DEBUG_IIS("Trace: IIS Playback task resuming.\n");
    IIS_PlaybackTask_Stop   = 0;
    OSTaskResume(IIS_PLAYBACK_TASK_PRIORITY);

    return 1;
}

/*

Routine Description:

    Suspend IIS Playback task.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisSuspendPlaybackTask(void)
{
    /* Suspend the task */
    //DEBUG_IIS("Trace: IIS Playback task suspending.\n");
    OSTaskSuspend(IIS_PLAYBACK_TASK_PRIORITY);

    return 1;
}

/*

Routine Description:

    Begin received audio data and playback the data immediately in preview mode.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisPreviewI2OBegin(void)
{
    u32     i;
    u8      err;

    if(iisPreviewI2ORunning)
        return 0;

    DEBUG_IIS("iisPreviewI2OBegin()\n", err);

    iisPreviewI2ORunning    = 1;
    iisSounBufMngReadIdx    = 0;
    iisSounBufMngWriteIdx   = 0;
    iisSounBufMngPlayIdx    = 0;
    IISMode                 = 2;    // 0: record, 1: playback, 2: receive and playback audio in preview mode
    IISTime                 = 0;    /* Peter 070104 */
#if REMOTE_TALK_BACK
    iisPlayDMACnt           = 0;
#endif

    for(i = 0; i < IIS_BUF_NUM; i++) {
        iisSounBufMng[i].buffer     = iisSounBuf[i];
    }

    OSSemSet(iisCmpSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_IIS("OSSemSet Error: iisCmpSemEvt is %d.\n", err);
    }
    while(iisTrgSemEvt->OSEventCnt > (IIS_BUF_NUM - 2)) {
        OSSemAccept(iisTrgSemEvt);
    }
    while(iisTrgSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
        OSSemPost(iisTrgSemEvt);
    }
    while(iisPlaybackSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisPlaybackSemEvt);
    }

#if (AUDIO_OPTION == AUDIO_ADC_DAC)
    adcInit(1);
#endif  // AUDIO_ADC_DAC
#if (REMOTE_TALK_BACK == 0)
    iisResumeTask();
#endif
    //iisStartRec();
    //while(iisPlaybackSemEvt->OSEventCnt < 2) {
    //    OSTimeDly(1);
    //}
    iisResumePlaybackTask();
    #if (AUDIO_OPTION == AUDIO_ADC_DAC)
    uiInitDAC_Play();
    #endif  // AUDIO_ADC_DAC


    DEBUG_IIS("IisCtrl = 0x%08x.\n", IisCtrl);
    DEBUG_IIS("IisMode = 0x%08x.\n", IisMode);
    DEBUG_IIS("AdcCtrlReg = 0x%08x.\n", AdcCtrlReg);

    return 1;
}

/*

Routine Description:

    Finish received audio data and playback the data immediately in preview mode.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 iisPreviewI2OEnd(void)
{
    u8 err;
	u8 i;

    if(!iisPreviewI2ORunning)
        return 0;

    DEBUG_IIS("iisPreviewI2OEnd()\n", err);

	iisPreviewI2ORunning = 0;
	//Step 1 : Stop IISTask
    while(iisTrgSemEvt->OSEventCnt > 0) {      //for IIS_Task
        OSSemAccept(iisTrgSemEvt);
    }
	OSSemSet(iisTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_IIS("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
    }
	while(iisCmpSemEvt->OSEventCnt > 0) {      //for asf write
        OSSemAccept(iisCmpSemEvt);
    }


#if (REMOTE_TALK_BACK == 0)
	for(i = 0, IIS_Task_Stop = 1; (!IIS_Task_Pend) && (i < 30); i++) {

        OSTimeDly(1);
    }
	iisStopRec();
    iisSuspendTask();
#endif
	//Step 2 : Stop IISPlayTask
    while(iisPlaybackSemEvt->OSEventCnt > 0) { //for Audio_In_Out
        OSSemAccept(iisPlaybackSemEvt);
    }
	for(i = 0, IIS_PlaybackTask_Stop = 1; (!IIS_PlayTask_Pend) && (i < 30); i++) {
        OSTimeDly(1);
    }
	iisStopPlay();
    iisSuspendPlaybackTask();

	//Step 3 : reset
    OSSemSet(iisCmpSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_IIS("OSSemSet Error: iisCmpSemEvt is %d.\n", err);
    }
    OSSemSet(iisTrgSemEvt, IIS_BUF_NUM - 2, &err);
    if (err != OS_NO_ERR) {
        DEBUG_IIS("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
    }
    while(iisPlaybackSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisPlaybackSemEvt);
    }

#if (REMOTE_TALK_BACK == 0)
    iisReset(IIS_SYSPLL_SEL_48M);
#endif

    return 1;
}

void iisIOPlayDMA_ISR(int dumy)
{
	iisPlayDMACnt++;
	if(iisPlayDMACnt % (IIS_CHUNK_SIZE / IIS_PLAYBACK_SIZE)==0)//Lsk ToDO
	{
	    iisSounBufMngPlayIdx    = (iisSounBufMngPlayIdx + 1) % IIS_BUF_NUM;
    #if (REMOTE_TALK_BACK == 0)
	    if(IISMode == 2)    // Receive and playback audio in preview mode
	    {
	        OSSemAccept(iisCmpSemEvt);
	        if(iisPreviewI2ORunning)
	            OSSemPost(iisTrgSemEvt);
	    }
    #endif
    }
    if( (gucIISPlayDMAPause) || (guiIISPlayDMAId == 0xFF) )
    {
    	gucIISPlayDMAStarting   = 0;
    	gucIISPlayDMAPause      = 0;
        return;
    }

    if(gucIISPlayDMACurrBufIdx == 15)
        gucIISPlayDMACurrBufIdx = 0;
    else
        gucIISPlayDMACurrBufIdx++;

    if(gucIISPlayDMACurrBufIdx == gucIISPlayDMANextBufIdx)
    {
        //DEBUG_IIS("1.Prepare IIS data for DMA slowly\n");
        gucIISPlayDMAStarting   = 0;
        return;
    }
    iisSetNextPlayDMA((u8*)gpIISPlayDMANextBuf[gucIISPlayDMACurrBufIdx], IIS_PLAYBACK_SIZE);
    iisStartPlay();
}

#endif  // #if AUDIO_IN_TO_OUT

/*********************************** AUDIO_IN_TO_OUT End **********************************/

#if(AUDIO_OPTION == AUDIO_AC97_ALC203)
/* yc: 20070124: S */
/*

Routine Description:

    Setup the ALC203 Playback function.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 ac97SetupALC203_rec(void)
{
    s32 ac97_n, ac97_t;

    //============== AC97 Codec setting ==============//
    Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_RESET | 0x00000);
    for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

    //==Set Extended Audio Status and Control(MX2A)==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_EXT_AUD_STA_CON | AC97_EXT_AUD_STA_CON_VRA_ENA);
        //Ac97CRAC = 0x002a0001;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | AC97_CRA_EXT_AUD_STA_CON);
        //Ac97CRAC = 0x802a0000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK_LSB) == AC97_EXT_AUD_STA_CON_VRA_ENA)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting AC97_CRA_EXT_AUD_STA_CON is fail !\n");
        return;
    }
    else
    {
        DEBUG_IIS("AC97_CRA_EXT_AUD_STA_CON : %d\n", ac97_t);
    }
    #endif



    //====Setup PCM ADC Sampling Rate = 8KHz (MX32)===//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_PCM_ADC_RATE | AC97_PCM_ADC_RATE_8K_HZ);
        //Ac97CRAC = 0x00321f40;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | AC97_CRA_PCM_ADC_RATE);
        //Ac97CRAC = 0x80320000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == AC97_PCM_ADC_RATE_8K_HZ)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting AC97_CRA_PCM_ADC_RATE is fail !\n");
        return;
    }
    else
    {
        DEBUG_IIS("AC97_CRA_PCM_ADC_RATE: %d\n", ac97_t);
    }
    #endif


    //==Setup Data flow control(MX6A):PCM-in from original ADC,stereo,Analog-out from DAC only==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | 0x006a0000 | 0x4040);
        //Ac97CRAC = 0x006a4040;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | 0x006a0000);
        //Ac97CRAC = 0x806a0000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == 0x4040)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting Data flow control(MX6A) is fail !\n");
        return;
    }
    else
    {
        DEBUG_IIS("DATA FLOW CONTROL: %d\n", ac97_t);
    }
    #endif

    //==Setup Recording Gain for Stereo ADC(MX1C)=+22.5 dB ==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_REC_GAIN_STEREO_ADC | 0x0f0f);
        //Ac97CRAC = 0x001c0f0f;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | AC97_CRA_REC_GAIN_STEREO_ADC);
        //Ac97CRAC = 0x801c0000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == 0x0f0f)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting AC97_CRA_REC_GAIN_STEREO_ADC is fail !\n");
        return;
    }
    else
    {
        DEBUG_IIS("AC97_CRA_REC_GAIN_STEREO_ADC: %d\n", ac97_t);
    }
    #endif

    //==Setup Recording Source from MIC In(MX1A)==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_REC_SELECT | 0x0000);
        //Ac97CRAC = 0x001a0000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | AC97_CRA_REC_SELECT);
        //Ac97CRAC = 0x801a0000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == 0x0000)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting AC97_CRA_REC_SELECT is fail !\n");
        return;
    }
    else
    {
        DEBUG_IIS("AC97_CRA_REC_SELECT: %d\n", ac97_t);
    }
    #endif

    //==Setup General purpose(MX20): select MIC1==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | 0x00200000 | 0x0000);
        //Ac97CRAC = 0x00200000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | 0x00200000);

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == 0x0000)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting General purpose reg is fail !\n");
        return;
    }
    else
    {
        DEBUG_IIS("GENERAL_PURPOSE REG: %d\n", ac97_t);
    }
    #endif

    //==Setup MIC volume(MX0E): Mic volume(analog-in)= 0dB, Boost enable, Boost gain=+29.5 dB. ==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | 0x000e0000 | 0x0348);
        //Ac97CRAC = 0x000e0348;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | 0x000e0000);
        //Ac97CRAC = 0x800e0000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == 0x0348)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting General purpose reg is fail !\n");
        return;
    }
    else
    {
        DEBUG_IIS("MIC volume REG: %d\n", ac97_t);
    }
    #endif

    return 1;
}

s32 ac97SetupALC203_play(void)
{
    s32 ac97_n, ac97_t;
    u32 PlayBackVolumn;

    PlayBackVolumn=(SPKVolumnTable[sysVolumnControl]<<8) | (HPVolumnTable[sysVolumnControl]);
    sysIsAC97Playing=1;
    //============== AC97 Codec setting ==============//
    //Power-ON
    Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_POWERDOWN | 0x00000);
    for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

    //ALC203 Reset
    Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_RESET | 0x00000);
    for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

    //==Set Extended Audio Status and Control(MX2A)==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_EXT_AUD_STA_CON | AC97_EXT_AUD_STA_CON_VRA_ENA);
        //Ac97CRAC = 0x002a0001;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | AC97_CRA_EXT_AUD_STA_CON);
        //Ac97CRAC = 0x802a0000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK_LSB) == AC97_EXT_AUD_STA_CON_VRA_ENA)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting AC97_CRA_EXT_AUD_STA_CON is fail !\n");
        return;
    }
    else
    {
        //DEBUG_IIS("AC97_CRA_EXT_AUD_STA_CON : %d\n", ac97_t);
    }
    #endif

    //====Setup PCM DAC Sampling Rate = 8KHz (MX2C)===//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_PCM_DAC_RATE | AC97_PCM_DAC_RATE_8K_HZ);
        //Ac97CRAC = 0x002c1f40;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Check AC'97 codec write done interrupt status */
        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | AC97_CRA_PCM_DAC_RATE);
        //Ac97CRAC = 0x802c0000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == AC97_PCM_DAC_RATE_8K_HZ)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting AC97_CRA_PCM_DAC_RATE is fail !\n");
        return;
    }
    else
    {
        //DEBUG_IIS("AC97_CRA_PCM_DAC_RATE : %d\n", ac97_t);
    }
    #endif


#if 0
    //== Setup Line output volume attenuation(MX02) = +0 dB ==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_LINEOUT | 0x02020);

        /* Need to wait for AC'97 CRA */
        for(ac97_n = 0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | AC97_CRA_LINEOUT);

        /* Need to wait for AC'97 CRA */
        for(ac97_n = 0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == 0x02020)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting AC97_CRA_LINEOUT is fail !\n");
        return;
    }
    else
    {
        DEBUG_IIS("AC97_CRA_LINEOUT : %d\n", ac97_t);
    }
    #endif
#else
    //== Setup Headphone output volume attenuation(MX04) = +0 dB ==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_HEADPHONE | 0x0000);
        //Ac97CRAC = 0x00048000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n = 0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | AC97_CRA_HEADPHONE);
        //Ac97CRAC = 0x80040000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n = 0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == 0x00000)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting AC97_CRA_HEADPHONE is fail !\n");
        return;
    }
    else
    {
        //DEBUG_IIS("AC97_CRA_HEADPHONE : %d\n", ac97_t);
    }
    #endif
#endif
    //===Setup PCM output volume gain(MX18)=> L(SPK): + 0dB, R(HP):-13.5dB==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_PCM_OUT_VOLUME | PlayBackVolumn);
        //Ac97CRAC = 0x00181111;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | AC97_CRA_PCM_OUT_VOLUME);
        //Ac97CRAC = 0x80180000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == PlayBackVolumn)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting AC97_CRA_PCM_OUT_VOLUME is fail !\n");
        return;
    }
    else
    {
        //DEBUG_IIS("AC97_CRA_PCM_OUT_VOLUME: %d\n", ac97_t);
    }
    #endif

    //==Setup Data flow control(MX6A):PCM-in from original ADC,stereo,Analog-out from DAC only==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | 0x006a0000 | 0x4040);
        //Ac97CRAC = 0x006a4040;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | 0x006a0000);
        //Ac97CRAC = 0x806a0000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == 0x4040)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting Data flow control(MX6A) is fail !\n");
        return;
    }
    else
    {
        //DEBUG_IIS("DATA FLOW CONTROL: %d\n", ac97_t);
    }
    #endif

    return 1;
}

void ac97SetupALC203_pwd(void)
{
    s32 ac97_n,ac97_t;

    sysIsAC97Playing=0;

    Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_RESET | 0x00000);
    for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);
    //==Power down control(MX26)==//
    Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_POWERDOWN | 0x0ff00);
    for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

    Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_POWERDOWN | 0x0ff00);
    for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

}
#endif

int riff_create_header (riff_wave_hdr_t *buffer, u16 chan, u32 rate, u16 width, u32 data_size) {

    //if (data_size != mask (data_size, chan * width)) {
        //return (-1);
    //}

    /* RIFF header */
    memcpy ((void *)buffer->riff_hdr_hdr.riff_magic, (void *)riff_magic, 4);
    buffer->riff_hdr_hdr.riff_size = host_to_le32(data_size + sizeof (riff_wave_hdr_t) - 8);
    memcpy ((void *)buffer->riff_hdr_hdr.wave_magic, (void *)wave_magic,  4);

    /* fmt chunk */
    memcpy ((void *)buffer->riff_hdr_fmt_chunk_hdr.chunk_magic, (void *)fmt_magic, 4);
    buffer->riff_hdr_fmt_chunk_hdr.chunk_size = host_to_le32(sizeof (riff_wave_fmt_t));
    buffer->riff_hdr_fmt_chunk.fmt_format = host_to_le16(WAVE_FORMAT_PCM);
    buffer->riff_hdr_fmt_chunk.fmt_chan = host_to_le16(chan);
    buffer->riff_hdr_fmt_chunk.fmt_rate = host_to_le32(rate);
    buffer->riff_hdr_fmt_chunk.fmt_bytes_per_second = host_to_le32(rate * chan * width);
    buffer->riff_hdr_fmt_chunk.fmt_bytes_per_sample = host_to_le16(chan * width);
    buffer->riff_hdr_fmt_chunk.fmt_width = host_to_le16(width * 8); /* bits! */

    /* data chunk */
    memcpy ((void *)buffer->riff_hdr_data_chunk_hdr.chunk_magic, (void *)data_magic,  4);
    buffer->riff_hdr_data_chunk_hdr.chunk_size = host_to_le32(data_size);

    return (0);
}

s32 wavRecVoice(void)
{
    int i;

    iisBufferCmpCount       = 0;
    iisSounBufMngReadIdx    = 0;
    iisSounBufMngWriteIdx   = 0;
    IISMode                 = 0;    // 0: record, 1: playback, 2: receive and playback audio in preview mode
    IISTime                 = 0;    /* Peter 070104 */
    IISTimeUnit             = (IIS_RECORD_SIZE * 1000) / 8000;  /* milliscends */ /* Peter 070104 */
    /* initialize sound buffer */
    for(i = 0; i < IIS_BUF_NUM; i++) {
        iisSounBufMng[i].buffer     = iisSounBuf[i];
    }
    /* initialize Voice Semaphore */
    while(iisCmpSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisCmpSemEvt);
    }
    Output_Sem();
    while(iisTrgSemEvt->OSEventCnt > (IIS_BUF_NUM - 2)) {
        OSSemAccept(iisTrgSemEvt);
    }
    while(iisTrgSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
        OSSemPost(iisTrgSemEvt);
    }
    Output_Sem();

#if (AUDIO_CODEC == AUDIO_CODEC_PCM)
    DEBUG_IIS("PCM format\n");
    if (wavRecVoiceFile() == 0)
    {
        /* reset the capture control if error */
        sysVoiceRecStart = 0;
        sysVoiceRecStop = 1;
    }
#elif (AUDIO_CODEC == AUDIO_CODEC_MS_ADPCM)
    DEBUG_IIS("Microsoft ADPCM format\n");
    if (wavRecVoiceFile_MS_ADPCM() == 0)
    {
        /* reset the capture control if error */
        sysVoiceRecStart    = 0;
        sysVoiceRecStop     = 1;
    }
#elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    DEBUG_IIS("IMA ADPCM format\n");
    if (wavRecVoiceFile_IMA_ADPCM() == 0)
    {
        /* reset the capture control if error */
        sysVoiceRecStart    = 0;
        sysVoiceRecStop     = 1;
    }
#endif

    //Stop iisTrgSemEvt semaphore --> User Click Stop Rec
    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }
    // Stop IIS task and Rec
    iisStopRec();
    iisSuspendTask();
    // Recover the Voice Semaphore
    while(iisCmpSemEvt->OSEventCnt > 0) {
    OSSemAccept(iisCmpSemEvt);
    }
    Output_Sem();
    while(iisTrgSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
    OSSemPost(iisTrgSemEvt);
    }
    Output_Sem();

    DEBUG_IIS("Voice file captured - Voice count = %d\n", iisBufferCmpCount);

    // reset IIS hardware
    iisReset(IIS_SYSPLL_SEL_48M);

    return 1;
}

s32 wavRecVoiceFile(void)
{
    FS_FILE* pFile;
    FS_DISKFREE_T* diskInfo;
    u32 free_size;
    u32 bytes_per_cluster;
    u16 audio_value;
    u16 audio_value_max;
    u16     width, chan;
    u32     rate, data_size,data_size_pos;
    u32 size;
    riff_wave_hdr_t riff_wave_hdr;
    double      length = 1.0;
	u8 tmp;
	
    diskInfo    =   &global_diskInfo;
    bytes_per_cluster   =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size= diskInfo->avail_clusters * bytes_per_cluster;

    if(free_size <=0x8000)      //reserve for  redundance voice data
    {
        DEBUG_IIS("Memory full \n");
        diskInfo->avail_clusters=0; //for Tome -> 0
        sysVoiceRecStop=1;
        sysVoiceRecStart=0;

        osdDrawMemFull(UI_OSD_DRAW);
        OSTimeDly(20);

        return 0;
    }

    width = DEFAULT_WIDTH;
    chan  = DEFAULT_CHAN;
    rate  = DEFAULT_RATE;
    data_size_pos= DEFAULT_DATA_POS;
    if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_WAV, 0)) == NULL){
        DEBUG_IIS("Wav create file error!!!\n");
        return 0;
    }

    DEBUG_IIS("iisCmpSemEvt = %d\n", iisCmpSemEvt->OSEventCnt);
    DEBUG_IIS("iisTrgSemEvt = %d\n", iisTrgSemEvt->OSEventCnt);
    audio_value_max = 0;
    // Temoary data size (will update later)
    data_size = mask ((int)(length*rate*width*chan), width*chan);
    riff_create_header (&riff_wave_hdr, chan, rate, width, data_size);

    if (dcfWrite(pFile, (unsigned char*)&riff_wave_hdr, sizeof(riff_wave_hdr), &size) == 0)
    {
        DEBUG_IIS("Wav Write header error!!!\n");
        sysVoiceRecStop=1;
        sysVoiceRecStart=0;
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }
    // Resume IIS task
    iisResumeTask();
    iisStartRec();

    diskInfo    =   &global_diskInfo;
    bytes_per_cluster   =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size= diskInfo->avail_clusters * bytes_per_cluster;
    while (sysVoiceRecStop == 0)
    {

        if(free_size <= 0x8000)     //reserve for  redundance voice data
        {
            DEBUG_IIS("Memory full \n");
            diskInfo->avail_clusters=0; //for Tome -> 0
            sysVoiceRecStop=1;
            sysVoiceRecStart=0;

            osdDrawMemFull(UI_OSD_DRAW);
            OSTimeDly(20);

            break;
        }
        OSTimeDly(1);

        // Return the original semaphore count value
        audio_value = OSSemAccept(iisCmpSemEvt);
        if (audio_value > 0)
            {   //Finish one buffer record
            if(audio_value_max < audio_value)
                audio_value_max = audio_value;
            if (wavWriteVoiceData(pFile, &iisSounBufMng[iisSounBufMngReadIdx]) == 0) {
                DEBUG_IIS("Wav Write Data error!!!\n");
                sysVoiceRecStop=1;
                sysVoiceRecStart=0;
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            }else
                    free_size-=1024;  //Chunk size
            iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
            //asfDebugPrint("Trace: IIS frame written.\n");
            OSSemPost(iisTrgSemEvt);
        }

    }   //end while
    // Revoke the semaphore key
    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }
    // mark below because we decrease avail_clusters in dcfclose
    //diskInfo  =   &global_diskInfo;
    //bytes_per_cluster =   diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    //used_cluster=(iisBufferCmpCount+bytes_per_cluster-1) / bytes_per_cluster;
    //diskInfo->avail_clusters -= used_cluster;


    /* delay until IIS task reach pend state */
    OSTimeDly(3);

    // write redundance voice data
    while(iisCmpSemEvt->OSEventCnt > 0) {
        Output_Sem();
        audio_value = OSSemAccept(iisCmpSemEvt);
        Output_Sem();
        if (audio_value > 0 && free_size > 0x8000)
        {
            if(audio_value_max < audio_value)
                audio_value_max = audio_value;
            if (wavWriteVoiceData(pFile, &iisSounBufMng[iisSounBufMngReadIdx]) == 0) {
                DEBUG_IIS("Wav Write Data error!!!\n");
                sysVoiceRecStop=1;
                sysVoiceRecStart=0;
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            }
            iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
            //OSSemPost(iisTrgSemEvt);
        }
    }   //end while

    // update the wav data size
    dcfSeek(pFile, data_size_pos, FS_SEEK_SET);
    iisBufferCmpCount-=5120; //temp use ..the further root cause should diag out civicbug
    if (dcfWrite(pFile, (char *)&iisBufferCmpCount, 4, &size) == 0)
            return 0;
	DEBUG_IIS("global_Wav_count++\n");
    dcfCloseFileByIdx(pFile, 0, &tmp);
    uiMenuEnable=2;
    //if(sysTVOutOnFlag)
     // uiTVKeyMenu();
    //else
    //uiKeyMenu();

    return 1;
}

/*

Routine Description:

    Write audio payload.

Arguments:

    pFile - File handle.
    pMng - Buffer manager.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 wavWriteVoiceData(FS_FILE* pFile, IIS_BUF_MNG* pMng)
{
    u32 size;
    u32 chunkTime, chunkSize;
    u8* pChunkBuf;


    chunkTime = pMng->time;
    chunkSize = pMng->size;
    pChunkBuf = pMng->buffer;

    if (dcfWrite(pFile, (unsigned char*)pChunkBuf, chunkSize, &size) == 0)
        {
        DEBUG_IIS("Rec voice File error \n");
            return 0;
        }

    /* Record the data chunk */
    // One size for IIS record is 1024 bytes (256x4 bytes (IIS DMA))
    //DEBUG_IIS("chunkSize = %d\n", chunkSize);
    iisBufferCmpCount+=chunkSize;

    return 1;
}

/*  WAVE format:
0         4   ChunkID          Contains the letters "RIFF" in ASCII form
                               (0x52494646 big-endian form).
4         4   ChunkSize        36 + SubChunk2Size, or more precisely:
                               4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
                               This is the size of the rest of the chunk
                               following this number.  This is the size of the
                               entire file in bytes minus 8 bytes for the
                               two fields not included in this count:
                               ChunkID and ChunkSize.
8         4   Format           Contains the letters "WAVE"
                               (0x57415645 big-endian form).

The "WAVE" format consists of two subchunks: "fmt " and "data":
The "fmt " subchunk describes the sound data's format:

12        4   Subchunk1ID      Contains the letters "fmt "
                               (0x666d7420 big-endian form).
16        4   Subchunk1Size    16 for PCM.  This is the size of the
                               rest of the Subchunk which follows this number.
20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
                               Values other than 1 indicate some
                               form of compression.
22        2   NumChannels      Mono = 1, Stereo = 2, etc.
24        4   SampleRate       8000, 44100, etc.
28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
32        2   BlockAlign       == NumChannels * BitsPerSample/8
                               The number of bytes for one sample including
                               all channels. I wonder what happens when
                               this number isn't an integer?
34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
          2   ExtraParamSize   if PCM, then doesn't exist
          X   ExtraParams      space for extra parameters

The "data" subchunk contains the size of the data and the actual sound:

36        4   Subchunk2ID      Contains the letters "data"
                               (0x64617461 big-endian form).
40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
                               This is the number of bytes in the data.
                               You can also think of this as the size
                               of the read of the subchunk following this
                               number.
44        *   Data             The actual sound data.

*/
#if 1  //re-coding 2013/0705 by Lucian
s32 wavReadFile_PCM(FS_FILE* pFile)
{
    u32 data_size_pos,data_size,size;
    u8 *PlayBuf,*FillBuf,*pTemp;
    int pcmcnt,count;
	u8	tmp;
    //============================================//

#if (AUDIO_OPTION == AUDIO_IIS_WM8974)
    Init_IIS_WM8974_play();
#elif(AUDIO_OPTION == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_play();
#elif(AUDIO_OPTION == AUDIO_AC97_ALC203)
    ac97SetupALC203_play();
#elif(AUDIO_OPTION == AUDIO_ADC_DAC)
    init_DAC_play(1);
#endif

    PlayBuf=iisSounBuf[0];
    FillBuf=iisSounBuf[8];

    //Find the data size
    data_size_pos   = DEFAULT_DATA_POS;
    dcfSeek(pFile, data_size_pos, FS_SEEK_SET);
    if (dcfRead(pFile, (char *)&data_size, 4, &size) == 0)
    {
        DEBUG_IIS("Wav Read Data Size error!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }
    DEBUG_IIS("Voice Data Size = %d\n", data_size);

    if(data_size >= IIS_CHUNK_SIZE*2)
        pcmcnt=IIS_CHUNK_SIZE*2;
    else
        pcmcnt=data_size;

    if (dcfRead(pFile, FillBuf, pcmcnt, &size) == 0)
    {
        DEBUG_IIS("Wav Read Data error!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }
    data_size -= pcmcnt;
    count=0;
    while(data_size>0)
    {
        pTemp=PlayBuf;
        PlayBuf=FillBuf;
        FillBuf=pTemp;

        iisSetPlayDma(PlayBuf, pcmcnt);
        iisStartPlay();

        if(data_size >= IIS_CHUNK_SIZE*2)
            pcmcnt=IIS_CHUNK_SIZE*2;
        else
            pcmcnt=data_size;

        if (dcfRead(pFile, FillBuf, pcmcnt, &size) == 0)
        {
            DEBUG_IIS("Wav Read Data error!!!\n");
            dcfClose(pFile, &tmp);
            return 0;
        }
        data_size -= pcmcnt;

        if(iisCheckPlayDmaReady() != 1)
        {
            DEBUG_IIS("Error: iisCheckPlayDmaReady()\n");
            return;
        }
		if(sysVoicePlayStop==1)
			break;
        DEBUG_IIS("Play CHUNK %d\n",count);
        count ++;
    }
	if(sysVoicePlayStop==0)
	{
	    pTemp=PlayBuf;
	    PlayBuf=FillBuf;
	    FillBuf=pTemp;

	    iisSetPlayDma(PlayBuf, pcmcnt);
	    iisStartPlay();

	    if(iisCheckPlayDmaReady() != 1)
	    {
	        DEBUG_IIS("Error: iisCheckPlayDmaReady()\n");
	        return;
	    }
	    DEBUG_IIS("Play CHUNK %d\n",count);
	}
    iisStopPlay();
    dcfClose(pFile, &tmp);
    OSTimeDly(5);
    DEBUG_IIS("Wav Playback End!!!\n");
	sysVoicePlayStop=0;
    return 1;
}
#else
s32 wavReadFile_PCM(FS_FILE* pFile)
{
    int data_size, i, VoicePlayCount    = 0;
    u32 data_size_pos;
    u32 size, rest_key, audio_value;
    u32 AudioPlayback       = 0;
    //============================================//

    iisSounBufMngReadIdx    = 0;
    iisSounBufMngWriteIdx   = 0;

    IISMode                 = 1;    // 0: record, 1: playback, 2: receive and playback audio in preview mode
    IISTime                 = 0;    // Current IIS playback time(micro second)
    IISTimeUnit             = 0;    // IIS playback time per DMA(micro second)
    iisPlayCount            = 0;    // IIS played chunk number
    iisTotalPlay            = 0;    // IIS total trigger playback number

    /* initialize sound buffer */
    for(i = 0; i < IIS_BUF_NUM; i++) {
        iisSounBufMng[i].buffer     = iisSounBuf[i];
    }
    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }

    while(iisplayCmpEvt->OSEventCnt > 0) {
        OSSemAccept(iisplayCmpEvt);
    }
    Output_Sem();
    while(iisCmpSemEvt->OSEventCnt > (IIS_BUF_NUM - 2)) {
        OSSemAccept(iisCmpSemEvt);
    }
    while(iisCmpSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
        OSSemPost(iisCmpSemEvt);
    }
    Output_Sem();

    //Find the data size
    data_size_pos   = DEFAULT_DATA_POS;
    dcfSeek(pFile, data_size_pos, FS_SEEK_SET);
    if (dcfRead(pFile, (char *)&data_size, 4, &size) == 0)
    {
        DEBUG_IIS("Wav Read Data Size error!!!\n");
        dcfClose(pFile);
        return 0;
    }
    DEBUG_IIS("Voice Data Size = %d\n", data_size);
    VoicePlayCount=data_size/IIS_CHUNK_SIZE;

    while(iisPlayCount < VoicePlayCount )
    {
        if(sysVoicePlayStop==1)
            break;
        audio_value = OSSemAccept(iisCmpSemEvt);

        if (audio_value > 0)
        {
            rest_key = OSSemAccept(iisplayCmpEvt);
            if (rest_key==1)
            {
                OSSemPost(iisTrgSemEvt);
            }

            //DCF read
            if (dcfRead(pFile, (u8*)iisSounBufMng[iisSounBufMngWriteIdx].buffer, IIS_CHUNK_SIZE, &size) == 0)
            {
                DEBUG_IIS("Wav Read Data error!!!\n");
                dcfClose(pFile);
                return 0;
            }

            //Advance the buffer pointer
            iisSounBufMng[iisSounBufMngWriteIdx].size   = size;
            iisSounBufMngWriteIdx = (iisSounBufMngWriteIdx + 1) % IIS_BUF_NUM;

            if(AudioPlayback == 0)
            {
                AudioPlayback=1;
                iisResumeTask();
                OSSemPost(iisTrgSemEvt);
            }
        }
    }

    dcfClose(pFile);
    OSTimeDly(5);
    DEBUG_IIS("Wav Playback End!!!\n");
    // reset IIS hardware
    return 1;
}
#endif

s32 wavreadfile(void)
{
    FS_FILE*    pFile;
    u32         size;
    u16         wFormatTag;
	u8	tmp;
    if ((pFile = dcfOpen((signed char*)dcfPlaybackCurFile->pDirEnt->d_name, "r")) == NULL) {
        DEBUG_IIS("WAV open file error!!!\n");
        return 0;
    }

    //Find the data size
    dcfSeek(pFile, 0x14, FS_SEEK_SET);  // read fmt_format(PCM) or wFormatTag(MS ADPCM)
    if (dcfRead(pFile, (char *)&wFormatTag, sizeof(wFormatTag), &size) == 0)
    {
        DEBUG_IIS("Wav Read Data Size error!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }

    if(wFormatTag == WAVE_FORMAT_PCM) {
        DEBUG_IIS("PCM format\n");
        return wavReadFile_PCM(pFile);
#if (AUDIO_CODEC == AUDIO_CODEC_MS_ADPCM)
    } else if(wFormatTag == WAVE_FORMAT_ADPCM) {
        DEBUG_IIS("Microsoft ADPCM format\n");
        return wavReadFile_MS_ADPCM(pFile);
#elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    } else if(wFormatTag == WAVE_FORMAT_IMA_ADPCM) {
        DEBUG_IIS("IMA ADPCM format\n");
        return wavReadFile_IMA_ADPCM(pFile);
#endif
    } else {
        DEBUG_IIS("Error: Unknown wave format!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }
}

s32 wavReadVoiceData(FS_FILE* pFile, IIS_BUF_MNG* pMng)
{
    u32 size;
    u32 chunkTime, chunkSize;
    u8* pChunkBuf;


    chunkTime = pMng->time;
    chunkSize = pMng->size;
    pChunkBuf = pMng->buffer;

    if (dcfRead(pFile, (unsigned char*)pChunkBuf, chunkSize, &size) == 0)
            return 0;

     //iisSounBufMng[iisSounBufMngWriteIdx].size   = IIS_CHUNK_SIZE;
    /* Record the data chunk */
    // One size for IIS record is 1024 bytes (256x4 bytes (IIS DMA))
    //DEBUG_IIS("chunkSize = %d\n", chunkSize);
    //iisBufferCmpCount+=chunkSize;

    return 1;
}

s32 ac97SetupALC203_AdjustVolumn(u8 Volumn)
{
    s32 ac97_n, ac97_t;
    u32 PlayBackVolumn;
    //
    sysVolumnControl=Volumn; //Update system Volumn
    PlayBackVolumn=(SPKVolumnTable[sysVolumnControl]<<8) | (HPVolumnTable[sysVolumnControl]);

    if(sysIsAC97Playing==0)
       return 0;
    //===Setup PCM output volume gain(MX18)=> L(SPK): + 0dB, R(HP):-13.5dB==//
    for(ac97_t = 10; ac97_t > 0; ac97_t --)
    {
        Ac97CRAC = (AC97_WRITE_REGISTER_SELECT | AC97_CRA_PCM_OUT_VOLUME | PlayBackVolumn);
        //Ac97CRAC = 0x00181111;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Read Back the setting value */
        Ac97CRAC = (AC97_READ_REGISTER_SELECT | AC97_CRA_PCM_OUT_VOLUME);
        //Ac97CRAC = 0x80180000;

        /* Need to wait for AC'97 CRA */
        for(ac97_n=0; ac97_n < AC97_SETTING_DELAY;ac97_n ++);

        /* Compare read back value */
        if((Ac97CRAC & AC97_READ_DATA_MASK) == PlayBackVolumn)
            break;
    }

    #if SHOW_DEBUG_MSG_AC97
    if(ac97_t == 0)
    {
        DEBUG_IIS("Setting AC97_CRA_PCM_OUT_VOLUME is fail !\n");
        return 0;
    }
    else
    {
        //DEBUG_IIS("AC97_CRA_PCM_OUT_VOLUME: %d\n", ac97_t);
    }
    #endif
    return 1;
}

void iisEnaInt(void)
{
#if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
    IisAudFormat &= ~(IIS_INT_OVER_MASK | IIS_INT_UNDER_MASK | IIS_INT_PEND_MASK | IIS_INT_WDONE_MASK | IIS_INT_RDONE_MASK );
    IisAdvance &= ~(IIS_OP_MODE_IIS1_ENA | IIS_OP_MODE_IIS2_DISA | IIS_OP_MODE_IIS3_DISA | IIS_OP_MODE_IIS4_DISA);
#else
    IisAudFormat &= ~(IIS_INT_OVER_MASK | IIS_INT_UNDER_MASK | IIS_INT_PEND_MASK | IIS_INT_WDONE_MASK | IIS_INT_RDONE_MASK );
#endif
}

void iisDisInt(void)
{

#if( (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || \
    (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
    IisAudFormat |= IIS_INT_OVER_MASK | IIS_INT_UNDER_MASK | IIS_INT_PEND_MASK | IIS_INT_WDONE_MASK | IIS_INT_RDONE_MASK ;
    IisAdvance |= IIS_OP_MODE_IIS1_ENA | IIS_OP_MODE_IIS2_DISA | IIS_OP_MODE_IIS3_DISA | IIS_OP_MODE_IIS4_DISA;
#else
    IisAudFormat |= IIS_INT_OVER_MASK | IIS_INT_UNDER_MASK | IIS_INT_PEND_MASK | IIS_INT_WDONE_MASK | IIS_INT_RDONE_MASK ;
#endif
}

#if (PLAYBACK_METHOD==PLAYBACK_IN_IIS_ISR)
void PlaybackFrame(void)
{
    if(IISPplyback!=1)
    {
    	if(sysTVOutOnFlag) //TV-out
        {
        	#if TV_DISP_BY_IDU
				#if TV_DEBUG_ENA
        	       gpioSetLevel(1, 18, 1);
		        #endif
            #else
				while((IISTime >= VideoNextPresentTime) && (MainVideodisplaybuf_idx < IsuIndex))
				{							
					//DEBUG_MP4("(&1)");
					#if TV_DEBUG_ENA
    		        gpioSetLevel(1, 18, 1);
             		#endif
                  	iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
					#if TV_DEBUG_ENA
                    gpioSetLevel(1, 18, 0);
			            #endif
    	            MainVideodisplaybuf_idx++;
        	        while(VideoNextPresentTime >= Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM] && CloseFlag)
	        	    {
						//DEBUG_MP4("(&2)");
	        	    	OSTimeDly(1);
					}
                	VideoNextPresentTime    = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
				}
            #endif
		}
        else //Pannel-out
        {
        	
        	//while((IISTime >= (VideoNextPresentTime - Video_timebase)) && (MainVideodisplaybuf_idx < IsuIndex)) //Lsk 090507 : when FF or BF speed return normal speed
        	//while((IISTime >= (VideoNextPresentTime - Video_timebase)) && (MainVideodisplaybuf_idx < IsuIndex - 1 )) //Lsk 090507 : when FF or BF speed return normal speed
        	//Lsk: ((MainVideodisplaybuf_idx + 1) < IsuIndex), 確保VideoNextPresentTime    = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
        	//     所以會有漏播最張一楨     
            while((IISTime >= (VideoNextPresentTime - Video_timebase)) && ((MainVideodisplaybuf_idx + 1) < IsuIndex))   						
            {
				//DEBUG_MP4("(&3)");
               	iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
				#if TV_DEBUG_ENA
                gpioSetLevel(1, 18, 0);
		         #endif
	            MainVideodisplaybuf_idx++;
	        	#if 0   //Marked by Peter for fixing ROULE_DOORPHONE playback deadlock bug
	            while(VideoNextPresentTime >= Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM] && sysPlaybackVideoStop==0 && CloseFlag && !ResetPlayback)
    	        //while(VideoNextPresentTime >= Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM] && CloseFlag && !ResetPlayback)
        	    {
        	    	OSTimeDly(1);
    	        }
    	    	#endif
            	VideoNextPresentTime    = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
            }
		}
    }
}
#endif

void iisPlayDMA_ISR(int dummy)
{
    /* To confirm Audio payload playback finish*/
    if(iisPlayDMACnt % (IIS_CHUNK_SIZE/IIS_PLAYBACK_SIZE)==0)
    {
        Output_Sem();
        OSSemPost(iisCmpSemEvt);
        OSSemPost(iisplayCmpEvt);

        #if NIC_SUPPORT
        if(EnableStreaming)
        	OSSemPost(AudioRTPCmpSemEvt[0]);
        #if TUTK_SUPPORT
        if(P2PEnableStreaming[0])
            OSSemPost(P2PAudioCmpSemEvt[0]);
        #endif
        #endif
    }

	#if (PLAYBACK_METHOD==PLAYBACK_IN_IIS_ISR)
	IISTime    += IISTimeUnit;  // microsecond unit
	PlaybackFrame();
	#endif
	
    iisPlayDMACnt++;
    if( (gucIISPlayDMAPause) || (guiIISPlayDMAId == 0xFF) )
    {
    	gucIISPlayDMAStarting = 0;
    	gucIISPlayDMAPause = 0;
        return;
    }

    if(gucIISPlayDMACurrBufIdx == 15)
        gucIISPlayDMACurrBufIdx=0;
    else
        gucIISPlayDMACurrBufIdx++;


    if(gucIISPlayDMACurrBufIdx == gucIISPlayDMANextBufIdx)
    {
        //DEBUG_IIS("2.Prepare IIS data for DMA slowly\n");
        gucIISPlayDMAStarting = 0;
        return;
    }
	if(sysPlayBeepFlag == 1)
    iisSetNextPlayDMA((u8*)gpIISPlayDMANextBuf[gucIISPlayDMACurrBufIdx], IIS_BUF_SIZ);
	else
	iisSetNextPlayDMA((u8*)gpIISPlayDMANextBuf[gucIISPlayDMACurrBufIdx], IIS_PLAYBACK_SIZE);
    iisStartPlay();
}
void iisRecDMA_ISR(int dummy)
{
#if DETECT_VOLUME
    u32         Sum;
    u32         Avg;
    u8          *pBuf;
    u32         i;
    u32         Volume;
    static u32  MaxVolume   = 0;
    static u32  Counter     = 0;
    u8          bDouble=1;
    static u32   iiscnt =0;
#endif

#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    /* To confirm Audio payload playback finish*/
	#if FINE_TIME_STAMP
    s32     TimeOffset;
	#endif
	#if AUDIO_IN_TO_OUT
    OSSemPost(iisPlaybackSemEvt);
    #endif

	if(guiIISRecDMAId == 0xFF)
    {
    	gucIISRecDMAStarting = 0;
        return;
    }

    #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
    timerCountRead(2, (u32*) &TimeOffset);
    OS_ENTER_CRITICAL();
    IISTime    += IISTimeUnit;  // millisecond unit
    IISTimeOffset   = TimeOffset >> 8;
    OS_EXIT_CRITICAL();
    #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
	timerCountRead(1, (u32*) &TimeOffset);
	OS_ENTER_CRITICAL();
	IISTime    += IISTimeUnit;  // millisecond unit
	IISTimeOffset   = TimeOffset / 100;
	OS_EXIT_CRITICAL();
    #else
	OS_ENTER_CRITICAL();
	IISTime    += IISTimeUnit;  // millisecond unit
	OS_EXIT_CRITICAL();
    #endif

#if DETECT_VOLUME
    if((rfiuVoxEna[0]) && (rfMuteVox == 0))
    {
        pBuf    = (u8*)gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx];
        Sum     = 0;
        
        //for(i = 0; i < 32; i++, pBuf++)
        for(i = 0; i < 32; i++ )
        {
                Sum    += *pBuf;
                pBuf += 16;
        }
        Avg     = Sum / 32;

        pBuf    = (u8*)gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx];
        Sum     = 0;
        
        for(i = 0; i < 32; i++)
        {
            if(*pBuf >= Avg)
                Sum    += *pBuf - Avg;
            else
                Sum    += Avg - *pBuf;
            pBuf += 16;

        }
        Volume  = (Sum*10) / 32;
        //DEBUG_IIS("%02x %02x\n", Avg, Volume);
        if(Volume > MaxVolume)
            MaxVolume   = (Volume >= 0x9f6) ? 0x9f6 : Volume;

        if((Counter++ & 0x1f) == 0)
        {
        #if (HW_BOARD_OPTION  == MR8100_GCT_VM9710)
            if(uiIsVM9710 == 0)
                bDouble=2;
        #endif
            if(MaxVolume >= rfiuVoxThresh[0]*bDouble)
            {
                //DEBUG_IIS("rfiu_SetRXVoxTrig(0x%02x)\n", MaxVolume);
                rfiu_SetRXVoxTrig(MaxVolume/10);
                rfiuVoxTrigFlag = 1;
                rfiuVoxTrigLev=MaxVolume/10;
                MaxVolume   = 0;
            }
        }
    }
#endif
#if AUDIO_DEBUG_ENA
    *gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx] = iiscnt>>32;
    *gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx]++;


    *gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx] = iiscnt>>16;
    *gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx]++;

    *gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx] = iiscnt>>8;
    *gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx]++;
    
    *gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx] = iiscnt;
#endif    
	iisRecDMACnt++;
	if(iisRecDMACnt % (IIS_CHUNK_SIZE/IIS_RECORD_SIZE)==0)
    {
	    Output_Sem();
	    OSSemPost(iisCmpSemEvt);
	    OSSemPost(iisplayCmpEvt);
    #if NIC_SUPPORT
        #if TUTK_SUPPORT
        if(P2PEnableStreaming[0])
            OSSemPost(P2PAudioCmpSemEvt[0]);
        #endif

        if(EnableStreaming)
        	OSSemPost(AudioRTPCmpSemEvt[0]);
    #endif

    #if RFIU_SUPPORT
         OSSemPost(gRfiuAVCmpSemEvt[0]);
    #endif
    #if AUDIO_DEBUG_ENA
        iiscnt++;
    #endif
    }

    if(gucIISRecDMACurrBufIdx == 15)
        gucIISRecDMACurrBufIdx=0;
    else
        gucIISRecDMACurrBufIdx++;

    //DEBUG_IIS("%d", gucIISPlayDMACurrBufIdx);
    if(gucIISRecDMACurrBufIdx == gucIISRecDMANextBufIdx)
    {
        //DEBUG_IIS("Prepare IIS addr for DMA slowly\n");
        gucIISRecDMAStarting = 0;
        return;
    }
    #if (Audio_mode == AUDIO_AUTO)
//    isr_marsDMA_RecAuto(guiIISRecDMAId,(u8*)gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx]);
    iisSetNextRecDMA_auto((u8*)gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx], IIS_RECORD_SIZE);
    iisStartRec();
    #else
    iisSetNextRecDMA((u8*)gpIISRecDMANextBuf[gucIISRecDMACurrBufIdx], IIS_RECORD_SIZE);
    iisStartRec();
    #endif
}
void iisSetNextPlayDMA(u8* buf, u32 siz)
{
    INT32U uiDMAId, uiDMACmmd=0;
    REGDMA_CFG  RegDMACfg;
    REGDMA_CFG  *pRegDMACfg = (REGDMA_CFG*)&RegDMACfg;


    uiDMACmmd = (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|
                 DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_IIS_PLAY|DMA_BURST4);

    pRegDMACfg->src = (u32)buf;
    pRegDMACfg->dst = (u32)&(IisTxData);
    pRegDMACfg->cnt = (siz / 16);
    pRegDMACfg->cmmd = uiDMACmmd;

    marsDMAConfig(guiIISPlayDMAId, pRegDMACfg);
}
void iisSetNextPlayDMA_auto(u8* buf, u32 siz)
{
    DMA_CFG_AUTO *pdmaCfg_auto;

    INT32U uiDMAId, uiDMACmmd=0;
    //REGDMA_CFG  RegDMACfg;
    //REGDMA_CFG  *pRegDMACfg = (REGDMA_CFG*)&RegDMACfg;
    REGDMA_CFG_AUTO  RegDMACfg_auto;
    REGDMA_CFG_AUTO  *pRegDMACfg_auto = (REGDMA_CFG_AUTO*)&RegDMACfg_auto;
	
	if(guiIISPlayDMAId == 0xff)
	{
		printf("guiIISPlayDMAId over MAX, return.\n");
		return;
	}

    isr_marsDMA_PlayAuto(guiIISPlayDMAId,buf);
    pdmaCfg_auto= &MarsdmaCfg_auto[guiIISPlayDMAId];

    uiDMACmmd = (DMA_INT_ENA_FINISH|DMA_INT_ENA_ERROR|DMA_SRC_AHB|DMA_DST_APB|
                 DMA_SRC_INC4|DMA_DST_INC0|DMA_DATA_WORD|DMA_IIS_PLAY|DMA_BURST4);

//    pRegDMACfg->src = (u32)buf;
//    pRegDMACfg->dst = (u32)&(IisTxData);
//    pRegDMACfg->cnt = (siz / 16);
//    pRegDMACfg->cmmd = uiDMACmmd;
    pdmaCfg_auto->src        = (u32)buf;
    pdmaCfg_auto->dst        = (u32)&(IisTxData);
    pdmaCfg_auto->src_stride = IIS_PLAYBACK_SIZE;
    pdmaCfg_auto->dst_stride = 0;
    pdmaCfg_auto->datacnt    = siz / 16; // 4 cycle(burst=1)* 4 bytes(word)
    pdmaCfg_auto->linecnt    = 1;
    pdmaCfg_auto->burst      = 1; /*CY 0917*/

    if(pdmaCfg_auto->burst)
        uiDMACmmd |= DMA_BURST4;

    uiDMACmmd |= DMA_AUTONONSTOP_EN;

    pRegDMACfg_auto->src     = pdmaCfg_auto->src;
    pRegDMACfg_auto->dst     = pdmaCfg_auto->dst;

    pdmaCfg_auto->src       += pdmaCfg_auto->src_stride;
    pdmaCfg_auto->dst       += pdmaCfg_auto->dst_stride;

    pRegDMACfg_auto->src_alt = pdmaCfg_auto->src;
    pRegDMACfg_auto->dst_alt = pdmaCfg_auto->dst;

    pRegDMACfg_auto->datacnt = pdmaCfg_auto->datacnt;
    pRegDMACfg_auto->linecnt = pdmaCfg_auto->linecnt;
    pRegDMACfg_auto->cmmd    = uiDMACmmd;


    marsDMAConfig_auto(guiIISPlayDMAId, pRegDMACfg_auto);
}
void iisSetNextRecDMA(u8* buf, u32 siz)
{
	u32   ADCRecGrp;
    INT32U uiDMAId, uiDMACmmd=0;
    REGDMA_CFG  RegDMACfg;
    REGDMA_CFG  *pRegDMACfg = (REGDMA_CFG*)&RegDMACfg;


	/* set read data dma */
  #if(AUDIO_OPTION == AUDIO_ADC_DAC)
    #if((CHIP_OPTION == CHIP_A1016A))
	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pRegDMACfg->src = (u32)&(AdcRecData_G0);
    #elif( (CHIP_OPTION == CHIP_A1018A) || \
        (CHIP_OPTION == CHIP_A1018B) || \
        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pRegDMACfg->src = (u32)&(AdcRecData_G0);
    #else
   	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pRegDMACfg->src = (u32)&(AdcRecData);
    #endif
  #else
   	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pRegDMACfg->src = (u32)&(IisRxData);
  #endif

	pRegDMACfg->dst = (u32)buf;
	pRegDMACfg->cnt = (siz / 16);
    pRegDMACfg->cmmd = uiDMACmmd;

	marsDMAConfig(guiIISRecDMAId, pRegDMACfg);
}
void iisSetNextRecDMA_auto(u8* buf, u32 siz)
{
    DMA_CFG_AUTO *pdmaCfg_auto;
	u32   ADCRecGrp;
    INT32U uiDMAId, uiDMACmmd=0;
    //REGDMA_CFG  RegDMACfg;
    //REGDMA_CFG  *pRegDMACfg = (REGDMA_CFG*)&RegDMACfg;
    REGDMA_CFG_AUTO  RegDMACfg_auto;
    REGDMA_CFG_AUTO  *pRegDMACfg_auto = (REGDMA_CFG_AUTO*)&RegDMACfg_auto;

	if(guiIISRecDMAId == 0xff)
	{
		printf("guiIISRecDMAId over MAX, return.\n");
		return;
	}

    isr_marsDMA_RecAuto(guiIISRecDMAId,buf);
    pdmaCfg_auto= &MarsdmaCfg_auto[guiIISRecDMAId];
	/* set read data dma */
  #if(AUDIO_OPTION == AUDIO_ADC_DAC)
    #if((CHIP_OPTION == CHIP_A1016A))
	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pdmaCfg_auto->src = (u32)&(AdcRecData_G0);
    #elif( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || \
        (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1020A) ||(CHIP_OPTION == CHIP_A1026A)) // A1020DIFF1016
	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pdmaCfg_auto->src = (u32)&(AdcRecData_G0);
    #else
   	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pdmaCfg_auto->src = (u32)&(AdcRecData);
    #endif
  #else
   	uiDMACmmd = gDMAReqCmmd[DMA_REQ_IIS_RECORD]|DMA_BURST4;
    pdmaCfg_auto->src = (u32)&(IisRxData);
  #endif

//	pRegDMACfg->dst = (u32)buf;
//	pRegDMACfg->cnt = (siz / 16);
//  pRegDMACfg->cmmd = uiDMACmmd;

    pdmaCfg_auto->dst        = (u32)buf;
    pdmaCfg_auto->src_stride = 0;
    pdmaCfg_auto->dst_stride = IIS_RECORD_SIZE;
    pdmaCfg_auto->datacnt    = siz / 16; // 4 cycle(burst=1)* 4 bytes(word)
    pdmaCfg_auto->linecnt    = 1;
    pdmaCfg_auto->burst      = 1; /*CY 0917*/

//    if(pdmaCfg_auto->burst)
//        uiDMACmmd |= DMA_BURST4;

    uiDMACmmd |= DMA_AUTONONSTOP_EN;

    pRegDMACfg_auto->src     = pdmaCfg_auto->src;
    pRegDMACfg_auto->dst     = pdmaCfg_auto->dst;

    pdmaCfg_auto->src       += pdmaCfg_auto->src_stride;
    pdmaCfg_auto->dst       += pdmaCfg_auto->dst_stride;

    pRegDMACfg_auto->src_alt = pdmaCfg_auto->src;
    pRegDMACfg_auto->dst_alt = pdmaCfg_auto->dst;

    pRegDMACfg_auto->datacnt = pdmaCfg_auto->datacnt;
    pRegDMACfg_auto->linecnt = pdmaCfg_auto->linecnt;
    pRegDMACfg_auto->cmmd    = uiDMACmmd;
    
	marsDMAConfig_auto(guiIISRecDMAId, pRegDMACfg_auto);
}
/*

Routine Description:

    Set audio output mute.

Arguments:

    Mute    - 1: Mute, 0: Not Mute.

Return Value:

    None.

*/
void iisMute(u32 Mute)
{
    if(Mute)
        IisMode |= R_MUTE_EN_ENA | L_MUTE_EN_ENA;
    else
        IisMode &= ~R_MUTE_EN_ENA & ~L_MUTE_EN_ENA;
}
void iisboost_ON(void)
{
    DacTxCtrl |= DAC_ENBST_ON;
}
void iisboost_OFF(void)
{
    DacTxCtrl &= ~DAC_ENBST_ON;
}
