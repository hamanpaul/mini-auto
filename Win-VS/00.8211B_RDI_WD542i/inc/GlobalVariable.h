#ifndef __GLOBAL_VARIABLE_H__
#define __GLOBAL_VARIABLE_H__


#include "general.h"

extern u8  GMotionTrigger[MULTI_CHANNEL_MAX];
#if MULTI_CHANNEL_VIDEO_REC

#include "board.h"
#include "task.h"
#include "mp4api.h"
#include "asfapi.h"
#include "aviapi.h"
#include "mpeg4api.h"
#include "../VideoCodec/mpeg4/inc/mpeg4.h"
#include "../VideoCodec/mpeg4/inc/mpeg4reg.h"
#include "isuapi.h"
#include "iduapi.h"
#include "gpioapi.h"
#include "siuapi.h"
#include "sysapi.h"
#include "osapi.h"
#include "rtcapi.h"
#include "../VideoCodec/mpeg4/inc/Mp4RateControl.h"
#include "i2capi.h"
#include "sysapi.h"
#include "uiapi.h"
#include "MemoryPool.h"
#include "iisapi.h"

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

//  AV_Source
#define LOCAL_RECORD   0 
#define RX_RECEIVE     1



/*
 *********************************************************************************************************
 * Type Definition
 *********************************************************************************************************
 */

typedef struct _VIDEO_CLIP_PARAMETER
{
    u16 asfVopWidth;
    u16 asfVopHeight;
    u32 sysCaptureVideoMode;
    u32 asfRecTimeLen;          // Total recording time of event trigger per video, Second unit.
} VIDEO_CLIP_PARAMETER, *PVIDEO_CLIP_PARAMETER;

typedef struct _VIDEO_CLIP_OPTION
{
    //======== Video Clip Channel ==========================================================================================
    u8              VideoChannelID;
    OS_EVENT*       PackerTaskSemEvt;   // 避免create task 和 destroy task打架
    u8              PackerTaskCreated;

    //======== MPEG-4 ======================================================================================================
    u32             mpeg4VopTimeInc;
    u32             mpegflag;
    
    /* task and event related */
    OS_EVENT*       VideoTrgSemEvt;
    OS_EVENT*       VideoCmpSemEvt;
    OS_EVENT*       VideoRTPCmpSemEvt;

    /* buffer management */
    u32             VideoBufMngReadIdx;
    u32             VideoBufMngWriteIdx;
    //VIDEO_BUF_MNG   VideoBufMng[VIDEO_BUF_NUM]; 
    VIDEO_BUF_MNG   *VideoBufMng; 
    u32             CurrentVideoSize;   // video size in DRAM that don't write to SD card
    u32             CurrentVideoTime;   // video time in DRAM that don't write to SD card, millisecond unit.
    u8				SkipToIframe;
    u8              video_double_field_flag;
    u8              *mpeg4RefBuf_Y, *mpeg4RefBuf_Cb, *mpeg4RefBuf_Cr, *mpeg4McoBuf_Y, *mpeg4McoBuf_Cb, *mpeg4McoBuf_Cr;
    s32             mp4_avifrmcnt;
    s32             mpeg4MBRef;
    u32             Vop_Type;       // 0: I, 1: P
    u32             MPEG4_Mode;     // 0: record, 1: playback
    u32             MPEG4_Status;
    u32             MPEG4_Task_Go;  // 0: never run, 1: ever run
    u32             MPEG4_Error;
    MP4_Option      Mp4Dec_opt;

    /* playback related */

    u8              dftMpeg4Quality;    //設定Video Clip Quality. Set Qp.
    //u8              mpeg4VideoRecQulity;
    //u8              mpeg4VideoRecFrameRate;
    u8              VideoRecFrameRate;
    u32             Mpeg4EncCnt; //Lucina: for internal use: roundtype, reconstruct frame switch.

    //u8              Video_60fps_flag;    //Lucian: QVGA 60 fps, preview 只呈現30 fps. 特例處理
    //u8              show_flag;
    
    /* picture index */
    u32             VideoPictureIndex;
    //u32             NVOPCnt;
    u32             mpeg4Width;
    u32             mpeg4Height;
    u32             mpeg4Stride;
    u32             pnbuf_size_y;

    u32                     Cal_FileTime_Start_Idx;
    DEF_RATECONTROL_PARA    mpRateControlParam;
    u8                      ASF_set_interlace_flag; 
    int                     double_field_cnt;

    //======== IIS ======================================================================================================
    // for 語音雙向
//aher test
	u8 *rfiuAudioRetDMANextBuf[RFI_AUDIO_RET_BUF_NUM];
    u8              AudioChannelID;
    
    /* task and event related */
    OS_STK          iisTaskStack[IIS_TASK_STACK_SIZE]; /* Stack of task iisTask() */
    OS_EVENT*       iisTrgSemEvt;
    OS_EVENT*       iisCmpSemEvt;

    u32             guiIISRecDMAId, guiIISPlayDMAId;
    u8              gucIISPlayDMAStarting, gucIISPlayDMAPause, gucIISStartPlayBefore;
    u8              gucIISRecDMAStarting, gucIISRecDMAPause, gucIISStartRecBefore;
    volatile u8     *gpIISPlayDMANextBuf[16];
    volatile u8     gucIISPlayDMACurrBufIdx, gucIISPlayDMANextBufIdx;
    volatile u8     *gpIISRecDMANextBuf[16];
    volatile u8     gucIISRecDMACurrBufIdx, gucIISRecDMANextBufIdx;
    OS_EVENT        *gIISPlayUseSem;
    OS_EVENT        *gIISRecUseSem;
    u32             iisPlayDMACnt; //Lsk 090925 : count DMA finish
    u32             iisRecDMACnt; //Lsk 090925 : count DMA finish
    u32             IIS_Task_Pend;
    u32             IIS_PlayTask_Pend;
    u32             IIS_PlaybackTask_Stop;
    u32             IIS_Task_Stop;
    OS_EVENT*       iisplayCmpEvt;
    u32             IIS_Task_Go;    // 0: never run, 1: ever run
    
    #if AUDIO_IN_TO_OUT
    //OS_STK          iisPlaybackTaskStack[IIS_PLAYBACK_TASK_STACK_SIZE];
    //OS_EVENT*       iisPlaybackSemEvt;
    //u32             iisSounBufMngPlayIdx;
    //u8              iisPreviewI2ORunning;
    #endif

    /* Buffer Management */
    u32             iisSounBufMngReadIdx;
    u32             iisSounBufMngWriteIdx;
    u32             iisBufferCmpCount;  //civic 070829 
    //IIS_BUF_MNG     iisSounBufMng[IIS_BUF_NUM]; 
    IIS_BUF_MNG     *iisSounBufMng; 
    u32             CurrentAudioSize;
    
    u32             IISMode;        // 0: record, 1: playback, 2: receive and playback audio in preview mode
    s64             IISTime;        // Current IIS playback time(micro second)
    #if FINE_TIME_STAMP
    s32             IISTimeOffset;  // Current IIS playback time offset(micro second)
    #endif
    u32             IISTimeUnit;    // IIS playback time per DMA(micro second)
    //u32             iisPlayCount;   // IIS played chunk number
    //u32             iisTotalPlay;   // IIS total trigger playback number
    
    u8              MuteRec;
    u8              Audio_formate;

    //======== MemoryPool ======================================================================================================
    //u8              *mpeg4displaybuf[DISPLAY_BUF_NUM];
    //u8              *Jpeg_displaybuf[3];
    u8              *mpeg4MVBuf;
    
    u8              *mpeg4PRefBuf_Y;
    u8              *mpeg4PRefBuf_Cb;
    u8              *mpeg4PRefBuf_Cr;
    u8              *mpeg4NRefBuf_Y;
    u8              *mpeg4NRefBuf_Cb;
    u8              *mpeg4NRefBuf_Cr;
    
 #if(VIDEO_CODEC_OPTION == H264_CODEC)
    u8              *H264MBPredBuf;
    u8              *H264ILFPredBuf;
    u8              *H264IntraPredBuf;
 #endif
 
    u8              *PNBuf_Y0, *PNBuf_C0, *PNBuf_Y1, *PNBuf_C1, *PNBuf_Y2, *PNBuf_C2; /*Lucian 070531*/
    u8              *PNBuf_Y3, *PNBuf_C3; 
    u8              *PNBuf_Y[4], *PNBuf_C[4];
    //u8              *mpeg4outputbuf[3];
    u8              *VideoBuf;
    u8              *mpeg4VideBufEnd;

	u8 				*P2PPBVideoBuf; //Toby 130815
 	u8 				*P2PPBVideBufEnd;
    u8              *mpeg4IndexBuf;
#if (CDVR_LOG || CDVR_TEST_LOG)
    u8              *LogFileBuf;
    u8              *LogFileBufEnd;
#endif
 
#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    u8              *ImaAdpcmBuf;
    u8              *ImaAdpcmBufEnd;
#endif

    u8              *iisSounBuf[IIS_BUF_NUM]; 
    u8              sysVoiceRecStart;
    u8              sysVoiceRecStop;

    //======== SYS ======================================================================================================
    /* task and event related */
    //OS_STK                  syssubTaskStack[SYS_TASK_STACK_SIZE]; /* Stack of task sysTask() */
    OS_EVENT*               syssubSemEvt;
    SYS_EVT                 syssubEvt;

    u8                      sysCaptureVideoStart;
    u8                      sysCaptureVideoStop;
    u8                      sysReady2CaptureVideo;

    VIDEO_CLIP_PARAMETER    *pVideoClipParameter;

    //======== ASF ======================================================================================================
    /* common */ 
    u32             AV_Source;
    FS_FILE*        pFile;
    u32             asfVideoFrameCount;
    u32             asfHeaderSize, asfDataSize, asfIndexSize;
    u32             asfHeaderPaddingSize;
    u32             asfDataPacketCount;
    u32             asfDataPacketOffset;
    u32             asfDataPacketSendTime;
    u32             asfDataPacketPreSendTime;//Lsk : 090309
    u32             asfDataPacketNumPayload;    
    u32             asfDataPacketLeftSize;
    u8              asfDataPacketFormatFlag; //Lsk : indicate packet contain video payload or not 
    #if ASF_MASS_WRITE    /* Peter 070104 */
    //__align(16) u8  asfMassWriteData[ASF_MASS_WRITE_SIZE];
    //u8              asfMassWriteData[ASF_MASS_WRITE_SIZE];
    u8*             asfMassWriteData;
    u8*             asfMassWriteDataPoint;
    #endif

    //__align(4) static u8 paddingBytes[ASF_DATA_PACKET_SIZE]; //Lsk 090309
    u8              paddingBytes[ASF_DATA_PACKET_SIZE]; //Lsk 090309

    /* video related */
    u8              asfVideHeader[0x20];
    u32             asfVideHeaderSize;
    u16             asfVopWidth;
    u16             asfVopHeight;
    
    u32             asfVopCount;
    u32             asfVidePresentTime;
    u32             sysVidePresentTime;
    u8              CloseFlag;

#if 1//FORCE_FPS
    u32             asfDummyVopCount;
    u32             DummyChunkTime;
#endif

    /* audio related */
    //ASF_AUDIO_FORMAT    asfAudiFormat;
    u32             asfAudiChunkCount;
    u32             asfAudiPresentTime;
    u32             sysAudiPresentTime;
    /* index related */
    ASF_IDX_SIMPLE_INDEX_ENTRY  *asfIndexTable;
    u32                         asfIndexTableIndex;
    u32                         asfIndexTableCount;
    u32                         asfIndexEntryTime;
    u32                         asfIndexEntryPacketNumber;
    u16                         asfIndexEntryPacketCount;
    u8                          asfIndexTableRead;
    u32                         Last_VideoBufMngReadIdx;
    /*** to get exactly file duration ***/
    u32                         asfTimeStatistics;
    u8                          Start_asfTimeStatistics; //remove
    u32                         VideoTimeStatistics;
    //u8  Start_MPEG4TimeStatistics=0;
    u8                          DirectlyTimeStatistics;  // start asfTimeStatistics from pre-record time or not
    u32                         LocalTimeInSec;         // 怕audio壞掉時間不準,另外弄個計時器當備案.

    u32                 WantChangeFile;
    u32                 LastAudio;
    u32                 LastVideo;
    u8                  GetLastAudio;
    u8                  GetLastVideo;
    u32                 asfCaptureMode;         // ASF_CAPTURE_NORMAL, ASF_CAPTURE_OVERWRITE, ASF_CAPTURE_EVENT

    u8                  DoCaptureVideo;     // long term capture video in event mode, 0: don't capture video, 1: capture video, 2: want to stop capture video.
    //u32                 AudioPlayback; 
    //u8                  mpeg4taskrun;
    u32                 EventTrigger;
    u8                  AlarmDetect;        // 1: Alarm trigger, 0: none  , 2: Alarm finish
    s32                 MD_Diff;
#if(G_SENSOR_DETECT)
    u8                  GSensorEvent;
#endif
    u8                  OpenFile;
    u8                  WantToExitPreRecordMode;
    #if CDVR_LOG
    u8*                 szLogFile;
    u32                 LogFileLength;
    OS_EVENT*           LogFileSemEvt;
    u8*                 LogFileIndex[LOG_INDEX_NUM];
    u16                 LogFileStart;
    u16                 LogFileNextStart;
    u16                 LogFileCurrent;
    u8*                 pLogFileEnd;
    u8*                 pLogFileMid;
    #endif

    u32                 asfRecTimeLen;          // Total recording time of event trigger per video, Second unit.
    u32                 asfRecTimeLenTotal;     // Total recording time of event trigger per video with extend time, Second unit.
    //u32                 PreRecordTime;          // Recording time before event trigger per video, Second unit.
    //u32                 asfEventExtendTime;     // 在 Event mode下錄影時,再被觸發時要延伸多少錄影時間才關檔, Second unit.
    //u32                 MotionlessRecTimeLen;   //Max Motionless period
    u32                 asfEventTriggerTime;    // 前一次收到有效Event的時間,搭配 asfEventInterval, Second unit.

    u8                  tempbuf[ASF_DATA_PACKET_SIZE];  //Lsk 090309
    char                timeForRecord1[20];
    char                timeForRecord2[20];

    u32                 curr_record_space;  //Lsk 090422 : for overwrite information
    //u32                 VideoDuration;  //Lsk 090507

    u8                  ResetPayloadPresentTime;
    u8                  PayloadType ;   //Lsk 090522 : To indicate payload type when single payload in packet

    /*
    u32                 PreRecordTime       = 10;
    u32                 AV_TimeBase         = PREROLL; //For capture
    */
    //u32                 PreRecordTime;

    u32                 AV_TimeBase;        //For capture
    //u8                  RepairASF;          //Lsk 090814
    u8                  BandWidthControl;
    u8                  SetIVOP;
    u8                  CaptureErr;         // flag to record capture fail

    u32                 start_idx;
    u32                 end_idx;

    u8                  FreeSpaceControl;

    #if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    IIS_BUF_MNG         iisBufMngTemp;
    u8                  iisBufTemp[IIS_CHUNK_SIZE];
    s32                 PcmBytesForAdpcm;
    #endif

    //======== ISU ======================================================================================================
    u32                 ISUFrameDuration[ISU_FRAME_DURATION_NUM];

    //======== Timer ======================================================================================================
    u32                 RTCseconds;

    //======== RFIU ======================================================================================================
    s32                 RFUnit;

} VIDEO_CLIP_OPTION, *PVIDEO_CLIP_OPTION;


/*
 *********************************************************************************************************
 * Function Prototype
 *********************************************************************************************************
 */

void InitVideoClipOption(void);

/*
 *********************************************************************************************************
 * External Variable
 *********************************************************************************************************
 */

extern  VIDEO_CLIP_OPTION       VideoClipOption[MULTI_CHANNEL_MAX];
extern  VIDEO_CLIP_PARAMETER    VideoClipParameter[MULTI_CHANNEL_MAX];
#if(MULTI_CHANNEL_LOCAL_MAX)
extern  VIDEO_BUF_MNG           MultiChannelVideoBufMng[MULTI_CHANNEL_LOCAL_MAX][VIDEO_BUF_NUM];
extern  IIS_BUF_MNG             MultiChanneliisSounBufMng[MULTI_CHANNEL_LOCAL_MAX][IIS_BUF_NUM];
#else
extern  VIDEO_BUF_MNG           **MultiChannelVideoBufMng;
extern  IIS_BUF_MNG             **MultiChanneliisSounBufMng;
#endif
extern  PVIDEO_CLIP_OPTION      pvcoRfiu[MAX_RFIU_UNIT];
extern  u32                     RfRxVideoRecordEnable;
extern  OS_EVENT                *OverWriteDelReadySemEvt;
#if(G_SENSOR_DETECT)
extern  u8                      GSensorEvent;
#endif

/*
 *********************************************************************************************************
 * External Function
 *********************************************************************************************************
 */

s32 MultiChannelMPEG4EncoderTaskCreate(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelMPEG4EncoderTaskDestroy(VIDEO_CLIP_OPTION *pVideoClipOption);
void MultiChannelMPEG4IntHandler(void);
u32 MultiChannelDeterminePictureType(u32 frame_idx, u32 period, VIDEO_CLIP_OPTION *pVideoClipOption);
u32 MultiChannelMPEG4PutVOPHeader(u8 *pBuf, u32 pFlag, s64 *pTime, u32 *byteno,u32 FrameIdx, VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelMPEG4Coding1Frame(u8* FrameBuf, u32 FrameType, s64* FrameTime, u32* CmpSize, u32 FrameIdx, u32 mpeg4StartBits, VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelMPEG4Output1Frame(u8* pBuf, s64* pTime, u32* pSize, u32* Mpeg4EncCnt, VIDEO_CLIP_OPTION *pVideoClipOption);
void MultiChannelMPEG4EncoderTask(void* pData);
s32 MultiChannelMpeg4EncodeVolHeader(VIDEO_CLIP_OPTION *pVideoClipOption);
void MultiChannelMpeg4ConfigQualityFrameRate(int BitRateLevel, VIDEO_CLIP_OPTION *pVideoClipOption);
int MultiChannelMpeg4ModifyTargetBitRate(int NewBitRate, VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelMpeg4SetVideoFrameRate(u8 framerate, VIDEO_CLIP_OPTION *pVideoClipOption);
void MultiChannelMpeg4DropFrame(s64* pTime, VIDEO_CLIP_OPTION *pVideoClipOption);

s32 MultiChannelIISRecordTaskCreate(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelIISRecordTaskDestroy(VIDEO_CLIP_OPTION *pVideoClipOption);
void MultiChanneliisSetNextRecDMA(u8* buf, u32 siz, VIDEO_CLIP_OPTION *pVideoClipOption);
void MultiChanneliisRecDMA_ISR(int DMAId);
void MultiChannelIISRecordTask(void* pData);
s32 MultiChannelIIsStopRec(VIDEO_CLIP_OPTION *pVideoClipOption);

s32 MultiChannelAsfCaptureVideo(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteHeaderObject(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteFilePropertiesObjectPre(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteFilePropertiesObjectPost(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteVideStreamPropertiesObject(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteAudiStreamPropertiesObject(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteHeaderExtensionObject(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteCodecListObject(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteContentDescriptionObject(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteHdrPaddingObject(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteDataObjectPre(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteDataObjectPost(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteDataPacketPre(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfWriteDataPacketPost(VIDEO_CLIP_OPTION *pVideoClipOption, u32 paddingLength);
#if (AUDIO_CODEC == AUDIO_CODEC_PCM)
s32 MultiChannelAsfWriteAudiPayload(VIDEO_CLIP_OPTION *pVideoClipOption, IIS_BUF_MNG* pMng,int flag);
#elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
s32 MultiChannelAsfWriteAudiPayload_IMA_ADPCM_1Payload(VIDEO_CLIP_OPTION *pVideoClipOption, IIS_BUF_MNG* pMng1, IIS_BUF_MNG* pMng2, s32 *pPcmOffset);
s32 MultiChannelAsfWriteAudiPayload_IMA_ADPCM(VIDEO_CLIP_OPTION *pVideoClipOption, IIS_BUF_MNG* pMng, s32 *pPcmOffset);
#endif
s32 MultiChannelAsfWriteVidePayload(VIDEO_CLIP_OPTION *pVideoClipOption, VIDEO_BUF_MNG* pMng,int flag);
#if 1//FORCE_FPS
s32 MultiChannelAsfWriteDummyVidePayload(VIDEO_CLIP_OPTION *pVideoClipOption,int time);
#endif
s32 MultiChannelAsfWriteIndexObject(VIDEO_CLIP_OPTION *pVideoClipOption);
FS_FILE* MultiChannelAsfCreateFile(u8 flag, VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfCaptureVideo(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfCloseFile(VIDEO_CLIP_OPTION *pVideoClipOption);
s32 MultiChannelAsfCaptureVideoStop(VIDEO_CLIP_OPTION *pVideoClipOption);
void MultiChannelAsfCaptureVideoStopAll(void);

int MultiChannelSysCaptureVideo(s32 ZoomFactor, u32 Mode);
int sysCaptureVideoSubTaskCreate(int VideoChannelID, VIDEO_CLIP_PARAMETER *pVideoClipParameter);
s32 sysCaptureVideoSubTaskDestroy(int VideoChannelID);
int MultiChannelSysCaptureVideoOneCh(int VideoChannelID);
int MultiChannelSysCaptureVideoStopOneCh(int VideoChannelID);
int MultiChannelGetCaptureVideoStatus(int VideoChannelID);

#if MULTI_CHANNEL_RF_RX_VIDEO_REC
void RfRxVideoPackerSubTask(void* pData);
int RfRxVideoPackerSubTaskCreate(int RFUnit, VIDEO_CLIP_PARAMETER *pVideoClipParameter);
s32 RfRxVideoPackerSubTaskDestroy(int RFUnit);
s32 RfRxVideoPackerEnable(void);
s32 RfRxVideoPackerDisable(void);
s32 RfRxVideoPackerEnableOneCh(int RFUnit);
s32 RfRxVideoPackerDisableOneCh(int RFUnit);
#endif

#endif  // #if MULTI_CHANNEL_VIDEO_REC

#endif  // __GLOBAL_VARIABLE_H__

