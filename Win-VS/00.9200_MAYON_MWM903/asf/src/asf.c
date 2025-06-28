/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    asf.c

Abstract:

    The routines of ASF file.

Environment:

        ARM RealView Developer Suite

Revision History:

    2005/08/26  David Tsai  Create

*/

#include "general.h"
#include "board.h"
#include "task.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "asf.h"
#include "asfapi.h"
#include "mpeg4api.h"
#include "iisapi.h"
#include "../../iis/inc/iis.h"  /* Peter 070104 */
#include "iduapi.h" /* BJ: 0718 */
#include "isuapi.h" /* BJ: 0718 */
#include "ipuapi.h"    /* Peter 070104 */
#include "siuapi.h"    /* Peter 070104 */
#include "sysapi.h"
#include "timerapi.h"

#include "osapi.h"
#include "gpioapi.h"
#include "adcapi.h"
#include "dmaapi.h"
#include "uiapi.h"
#include "rfiuapi.h"
#if (VIDEO_CODEC_OPTION == MJPEG_CODEC)
#include "jpegapi.h" //Lsk : 090312
#elif (VIDEO_CODEC_OPTION == H264_CODEC)
#include "H264api.h" //Lsk : 090312
#endif
#include "ciuapi.h"
#include "GlobalVariable.h"
#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
#include "ima_adpcm_api.h"
#endif


//aher test
   s64     tmp1=0;

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
#define asfDebugPrint               DEBUG_ASF /* BJ: 0718 */

/* enable debugging / audio or not */
//#define ASF_AUDIO /*BJ 0530 S*/


#define TRIGGER_MODE_MAX_FILE_SIZE      ASF_FILE_SIZE_100MB  // 100MB
#define MANUAL_MODE_MAX_FILE_SIZE       ASF_FILE_SIZE_100MB  // 100MB

#define STILL_PLAYBACK_WHEN_ERROR       0   // 1: still playback when error occur, 0: stop playback when error occur.

#define ASF_WRITE_NORMAL	0
#define ASF_WRITE_COPY		1  //Lsk: only support ASF_MASS_WRITE_HEADER, ASF_MASS_WRITE

#if( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
#define ASF_WRITE_PATH		ASF_WRITE_COPY
#else
#define ASF_WRITE_PATH		ASF_WRITE_NORMAL
#endif

enum
{
    READFILE_NORMAL_PLAY = 0,
    READFILE_STOP_AT_FIRST_I_FRAME,
    READFILE_THUMBNAIL_PREVIEW,
};

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */
u32 gPlaybackWidth=0;
u32 gPlaybackHeight=0;


/* common */
u32 asfHeaderSize, asfDataSize, asfIndexSize;
u32 asfHeaderPaddingSize;
u32 asfDataPacketCount;
u32 asfDataPacketOffset;
u32 asfDataPacketSendTime;
u32 asfDataPacketPreSendTime;//Lsk : 090309
u32 asfDataPacketNumPayload;
u32 asfDataPacketLeftSize;
u8  asfDataPacketFormatFlag; //Lsk : indicate packet contain video payload or not
u8 ledon = 0;
#if ASF_MASS_WRITE    /* Peter 070104 */
__align(16) u8  asfMassWriteData[ASF_MASS_WRITE_SIZE];
u8* asfMassWriteDataPoint;
#endif

__align(4) static u8 paddingBytes[ASF_HEADER_SIZE_CEIL] = { 0x00 }; //Lsk 090309


/* video related */
u8  asfVideHeader[0x20];
u32 asfVideHeaderSize;
static u8 asfSeekPrevIFrame;

  #if(VIDEO_RESOLUTION_SEL== VIDEO_VGA_IN_VGA_OUT)
    u16 asfVopWidth = 640, asfVopHeight = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_VGA_OUT)
    u16 asfVopWidth = 640, asfVopHeight = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_FULLHD_IN_VGA_OUT)
    u16 asfVopWidth = 640, asfVopHeight = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_HD_IN_HD_OUT)
    u16 asfVopWidth = 1280, asfVopHeight = 720;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_FULLHD_IN_HD_OUT)
    u16 asfVopWidth = 1280, asfVopHeight = 720;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_1920x1440DECI3x3TOVGA)
    u16 asfVopWidth = 640, asfVopHeight = 480;
  #elif(VIDEO_RESOLUTION_SEL== VIDEO_FULL_HD)
    u16 asfVopWidth = 1920, asfVopHeight = 1072;
  #else
    u16 asfVopWidth = 640, asfVopHeight = 480;
  #endif
  
u32 asfVopCount;
u32 asfVidePresentTime;
u8  CloseFlag;

#if FORCE_FPS
u32 asfDummyVopCount;
u32 DummyChunkTime;
#endif

/* audio related */
ASF_AUDIO_FORMAT asfAudiFormat;
u32 asfAudiChunkCount;
u32 asfAudiPresentTime;

/* index related */
ASF_IDX_SIMPLE_INDEX_ENTRY *asfIndexTable;
u32 asfIndexTableIndex;
u32 asfIndexTableCount;
u32 asfIndexEntryTime;
u32 asfIndexEntryPacketNumber;
u16 asfIndexEntryPacketCount;
u8  asfIndexTableRead;
u32 Last_VideoBufMngReadIdx;
/*** to get exactly file duration ***/
u32 asfTimeStatistics;
u8  Start_asfTimeStatistics=0; //remove
//u8  Start_MPEG4TimeStatistics=0;
u8  DirectlyTimeStatistics=0;  // start asfTimeStatistics from pre-record time or not

u32 WantChangeFile;
u32 LastAudio;
u32 LastVideo;
u8  GetLastAudio;
u8  GetLastVideo;
u32 asfCaptureMode;         // ASF_CAPTURE_NORMAL, ASF_CAPTURE_OVERWRITE, ASF_CAPTURE_EVENT
u8  curr_playback_speed = 5;  //for UI level control
u8  curr_slow_speed = 0;
u8  pre_playback_speed = 5;   //for asf player level control
u8  video_playback_speed  = 5;//for asf player level control
u32 asfSectionTime = 0xffffffff;         // section time per video captured file.
s32 asfRecFileNum   = -1;


u8  DoCaptureVideo;     // long term capture video in event mode, 0: don't capture video, 1: capture video, 2: want to stop capture video.
u32 AudioPlayback;
u8 mpeg4taskrun;
u32 EventTrigger;
u8  OpenFile=0;
#if CDVR_LOG
u8*         szLogFile;
u32         LogFileLength;
OS_EVENT*   LogFileSemEvt;
u8*         LogFileIndex[LOG_INDEX_NUM];
u16         LogFileStart;
u16         LogFileNextStart;
u16         LogFileCurrent;
u8*         pLogFileEnd;
u8*         pLogFileMid;
#endif



u32 asfRecTimeLen           = ASF_REC_TIME_LEN;     // Total recording time of event trigger per video, Second unit.
u32 asfRecTimeLenTotal      = 0;                    // Total recording time of event trigger per video with extend time, Second unit.
u32 PreRecordTime           = 10;                   // Recording time before event trigger per video, Second unit.
u32 asfEventExtendTime      = 0;                    // 在 Event mode下錄影時,再被觸發時要延伸多少錄影時間才關檔, Second unit.
#if ((UI_VERSION == UI_VERSION_RDI) ||(UI_VERSION == UI_VERSION_RDI_2) ||(UI_VERSION == UI_VERSION_RDI_3))
u32 asfEventInterval        = 8;                    // 前一次Event經過多久後才允許下次Event進來, Second unit.
#elif (UI_VERSION == UI_VERSION_MUXCOM)
u32 asfEventInterval        = 4;                    // 前一次Event經過多久後才允許下次Event進來, Second unit.
#else
u32 asfEventInterval        = 0;                    // 前一次Event經過多久後才允許下次Event進來, Second unit.
#endif

u32 asfLocalTimeInSec      = 0;
u32 asfEventTriggerTime    = 0;
u32 MotionlessRecTimeLen    = 10;  //Max Motionless period


u8 tempbuf[ASF_HEADER_SIZE_CEIL];  //Lsk 090309
char  timeForRecord1[20];
char  timeForRecord2[20];

u32 curr_free_space;  //Lsk 090422 : for overwrite information
u32 curr_record_space;  //Lsk 090422 : for overwrite information

u32 VideoDuration;  //Lsk 090507

u8  ResetPayloadPresentTime;
u8  PayloadType ;   //Lsk 090522 : To indicate payload type when single payload in packet


static FS_DISKFREE_T   *diskInfo;  //Lsk 090715
static u32             free_size;
static u32             bytes_per_cluster;

u8 RepairASF=0; //Lsk 090814
u8 BandWidthControl = 0;

#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
s64 Video_timebase ;        //For playback
u8  StartPlayBack;
u8 ResetPlayback;  			//Lsk 090401 : reset playback system
#endif
   u8  CaptureErr = 0;     /*flag to record capture fail*/
u32 AV_TimeBase = PREROLL; //For capture

u32 start_idx;
u32 end_idx;

u8 FreeSpaceControl = 0;


#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
IIS_BUF_MNG iisBufMngTemp;
u8          iisBufTemp[IIS_CHUNK_SIZE];
s32         PcmBytesForAdpcm;
#if (IIS_SAMPLE_RATE == 8000)
const u8    AdpcmPayloadTimeLose[16]    = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1};
#elif (IIS_SAMPLE_RATE == 16000)
const u8    AdpcmPayloadTimeLose[16]    = {0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1};
#endif
#endif
u8 Video_Pend   = 1;

u32 P2PVideoBufMngWriteIdx; 
unsigned int P2PPlaybackVideoStop;
u8 P2Ptempbuf[ASF_HEADER_SIZE_CEIL];
u8 P2P_playback_go;
u8 P2P_check;
u8 Get_p2p_playback;
u8      H264_stat; // 1 : encode , 2 : decode


u8  AsfAudioZeroBuf[IIS_CHUNK_SIZE];
#define H264_SPSPPS_length 0x18
#define ASF_STUCK_THRESHOLD 128
static unsigned char H264_config[H264_SPSPPS_length] =    
{
    0x00, 0x00, 0x00, 0x01, 0x67,
    0x42, 0x00, 0x1E, 0xDA, 0x01,
    0x40, 0x16, 0xE4, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01,
    0x68, 0xCE, 0x38, 0x80,
};

unsigned char thumbnailID[8] = {0x12, 0x13, 0x14, 0x15, 0x88, 0x89, 0x90, 0x91};
/*
 *********************************************************************************************************
 * Extern Variable
 *********************************************************************************************************
 */
extern u8 SetIVOP; 
extern u32 VideoBufMngWriteIdx;
extern u32 VideoTimeStatistics;
extern unsigned int asfVideoFrameCount;

 
extern u8 uiMenuVideoSizeSetting;
extern u8 sysDeleteFATLinkOnRunning;
extern u32 guiIISCh4PlayDMAId;

extern s64 VideoNextPresentTime;

#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
extern s64 IDUInterruptTime;  //Lsk 090326
#endif

extern u8 IDUSlowMotionSpeed;
extern s32 MicroSecPerFrame;
extern u32 IsuIndex;

extern u32 mpegflag;
extern u32 siuFrameNumInMP4, siuSkipFrameRate; /* Peter 20061106 */
extern u32 mpeg4Width, mpeg4Height; /*Peter 1116 S*/
/* audio related */
extern WAVEFORMAT iisPlayFormat;
extern WAVEFORMAT iisRecFormat;
extern u32 IISMode;        // 0: record, 1: playback, 2: receive and playback audio in preview mode
extern s64 IISTime;        // Current IIS playback time(micro second)
extern u32 IISTimeUnit;    // IIS playback time per DMA(micro second)
extern u32 iisPlayCount;   // IIS played chunk number
extern u32 iisTotalPlay;   // IIS total trigger playback number
extern u8  GetMD_Diff;		//Flag of Get Motion Detection Diff



extern u32 VideoPictureIndex;
extern u32 NVOPCnt;
extern u32  Vop_Type;            // 0: Intra frame, 1: Inter frame
//extern u8   sysCaptureVideoStop;

extern u8 sysCaptureVideoStart;
extern u8 sysCaptureVideoStop;
extern u8 pwroff;
extern u32 OS_tickcounter;


extern MP4_Option  Mp4Dec_opt;

extern  u8  sysPlaybackVideoStop;
extern  u8  sysPlaybackVideoStopDone;

extern  s64 Videodisplaytime[DISPLAY_BUF_NUM];
extern  volatile s32 isu_avifrmcnt;
extern  s32 isu_unusedframe;
extern  u32 unMp4FrameDuration[SIU_FRAME_DURATION_NUM]; /* mh@2006/11/22: for time stamp control */ /* Peter 070403 */

extern FS_DISKFREE_T global_diskInfo;


extern u32 RTCseconds;
extern u8 TvOutMode;

extern u8 system_busy_flag;



#if ISU_OVERLAY_ENABLE
extern u32 *ScalarOverlayImage;
#endif

extern BOOLEAN MemoryFullFlag;
extern u8 TVout_Generate_Pause_Frame;
extern u8 ASF_set_interlace_flag;
extern u8 VideoRecFrameRate; //Lsk 090702 : use to calculate Pre-record length

extern u32 MotionlessSecond;

#if (MOTIONDETEC_ENA || HW_MD_SUPPORT || MOTION_TRIGGRT_REC)
extern s32 MD_Diff;
extern u32 MD_FullTVRun;
extern u32 MD_HalfTVRun;
extern u32 MD_PanelRun;
#endif
extern u8 Video_60fps_flag;
extern u8 show_flag;


#if OSD_LANGUAGE_ENA
extern u8 CurrLanguage;
#endif
extern u32  IVOP_PERIOD;

extern u32 Cal_FileTime_Start_Idx;

#if NIC_SUPPORT
extern u8 EnableStreaming;
extern u32 P2PPauseTime;
#endif

extern u32 IIS_Task_Pend;
extern u32 IIS_PlayTask_Pend;
extern u32 IIS_PlaybackTask_Stop;
extern u32 IIS_Task_Stop;
u8 Video_Mode;

#if (VIDEO_CODEC_OPTION == H264_CODEC)
extern u8 EncodeDownSample;
extern u8 EncodeLineStripe;
extern u8 DecodeDownSample;
extern u8 DecodeLineStripe;

#endif

extern unsigned int gRfiu_TX_Sta[MAX_RFIU_UNIT];
extern unsigned int gRfiu_WrapEnc_Sta[MAX_RFIU_UNIT];
extern unsigned int gRfiu_MpegEnc_Sta[MAX_RFIU_UNIT];
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];

#if (AUDIO_DEVICE== AUDIO_IIS_ALC5621)
extern u8 ALC5623_test;
#endif

extern u8  videoPlayNext;
extern int sysPlaybackHeight;
extern u8 Iframe_flag;
extern OS_FLAG_GRP  *gSysReadyFlagGrp;
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
#if ISU_OVERLAY_ENABLE
extern s32 GenerateOverlayImage(u32 *YUV422Image, char *szString, int MaxStrLen, int FontW, int FontH, int VideoW);
#endif

#if (AUDIO_DEVICE == AUDIO_IIS_ALC5621)
extern void Close_IIS_ALC5621(void);
#endif

//#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
extern s32 dcfOverWriteDel(void);
//#endif




/* function prototype */
FS_FILE* asfCreateFile(u8 flag);
s32 asfCloseFile(FS_FILE* pFile);
u8* asfReadPacketHeader(u8                          *buf,
                               ASF_FILE_PROPERTIES_OBJECT  *asfFilePropertiesObject,
                               u32                         *PacketUsedSize,
                               u8                          *flag2,
                               u8                          *Mul_Payload,
                               u32                         *PacketLength,
                               u32							*PaddingLength,//Lsk 090304
                               u64                         *UsedSize);
u8* asfReadPayloadHeader(u8  *buf,
                                u8  *StreamNum,
                                u8  *KeyFrame,
                                u8  flag2,
                                u8  Restart1,
                                u8  Restart2,
                                u8  PayloadFlag,
                                u32 *Offset2Media,
                                u32 *PayloadLength,
                                u32 *MediaObjectSize,  ////Lsk 090622
                                s64 *asfPresentTime,
                                u32 *asfVideoIndex,
                                u32 *asfAudioIndex,
                                u64 *UsedSize,
                                u8  Mul_Payload, //Lsk 090304
                                u32	PacketLength,
                                u32 PaddingLength);
u8* asfReadHeaderObject(u8                                  *buf,
                               ASF_HEADER_OBJECT                   *asfHeaderObject,
                               ASF_FILE_PROPERTIES_OBJECT          *asfFilePropertiesObject,
                               ASF_AUDI_STREAM_PROPERTIES_OBJECT   *asfAudiStreamPropertiesObject,
                               u8                                  *VideoStreamNum,
                               u8                                  *AudioStreamNum);
u8* asfReadVideoPayload(u8  *buf,
                               u8  *VideoStreamBuf,
                               u8  KeyFrame,
                               u8  *StartFrame,
                               u8  *Restart1,
                               u32 *VideoStreamLength,
                               u32 Offset2Media,
                               u32 PayloadLength,
                               u32 MediaObjectSize,   //Lsk 090622
                               s64 asfPresentTime,
                               u32 *asfVideoIndex,
                               u8      *mpeg4taskrun,
                               u32     *u32PacketOffset,
                               u64     *UsedSize,
                               FS_FILE *pFile,
                               ASF_FILE_PROPERTIES_OBJECT *asfFilePropertiesObject,
                               u64     *PacketCount);
u8* asfReadAudioPayload(u8  *buf,
                               u8  *AudioStreamBuf,
                               u8  *StartAudio,
                               u8  *Restart2,
                               u32 *AudioStreamLength,
                               u32 Offset2Media,
                               u32 PayloadLength,
                               s64 asfPresentTime,
                               u32 *asfAudioIndex,
                               u32 *AudioPlayback);
u8* asfReadAudioPayload_IMA_ADPCM(u8      *buf,
                                                 u8      *AudioStreamBuf,
                                                 u8      *StartAudio,
                                                 u8      *Restart2,
                                                 u32     *AudioStreamLength,
                                                 u32     Offset2Media,
                                                 u32     PayloadLength,
                                                 u32     *asfAudioIndex,
                                                 u32     *AudioPlayback,
                                                 ASF_AUDI_STREAM_PROPERTIES_OBJECT   *asfAudiStreamPropertiesObject);
s32 asfBurstReadIndexObject(FS_FILE* pFile, u32 IndexObjectSize);
s32 asfWriteFilePadding(FS_FILE* pFile, u32 DummySize);

s32 asfCaptureVideoFile(s32);

/* Audio */
s32 asfAudioInit(void);

extern s32 iisSetPlayFormat(WAVEFORMAT*);
extern s32 iisSetRecFormat(WAVEFORMAT*);
extern s32 iisStartPlay(void);
extern s32 iisStopPlay(void);
extern s32 iisStartRec(void);
extern s32 iisStopRec(void);
extern s32 iisSetPlayDma(u8*, u32);
extern s32 iisSetRecDma(u8*, u32);
extern s32 iisCheckPlayDmaReady(void);
extern s32 iisCheckRecDmaReady(void);

extern s32 iis5SetPlayFormat(WAVEFORMAT*);


/* Header object */
s32 asfWriteHeaderObject(FS_FILE*);
s32 asfWriteFilePropertiesObjectPre(FS_FILE*);
s32 asfWriteFilePropertiesObjectPost(FS_FILE*);
s32 asfWriteVideStreamPropertiesObject(FS_FILE*);
s32 asfWriteAudiStreamPropertiesObject(FS_FILE*);
s32 asfWriteHeaderExtensionObject(FS_FILE*);
s32 asfWriteCodecListObject(FS_FILE*);
s32 asfWriteContentDescriptionObject(FS_FILE*);
s32 asfWriteHdrPaddingObject(FS_FILE*);

/* Data object */
s32 asfWriteDataObjectPre(FS_FILE*);
s32 asfWriteDataObjectPost(FS_FILE*);
s32 asfWriteDataPacketPre(FS_FILE*);
s32 asfWriteDataPacketPost(FS_FILE*, u32);
s32 asfWriteAudiPayload(FS_FILE*, IIS_BUF_MNG*);
s32 asfWriteVidePayload(FS_FILE*, VIDEO_BUF_MNG*);
s32 asfWriteDummyVidePayload(FS_FILE* pFile);
s32 asfWriteVirtualVidePayload(VIDEO_BUF_MNG* pMng);
s32 asfWriteVirtualAudiPayload(IIS_BUF_MNG* pMng);
#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
s32 asfWriteAudiPayload_IMA_ADPCM(FS_FILE* pFile, IIS_BUF_MNG* pMng, s32 *pPcmOffset);
#endif


/* Index object */
s32 asfWriteIndexObject(FS_FILE*);
u32 GetVideoDuration(s8* pFileName);
u32 GetVideoWidth(s8* pFileName);
u32 GetVideoHeight(s8* pFileName);
u32 GetCaptureVideoStatus(void);
/* for debug */
void Output_Sem(void);

/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */
 /*

Routine Description:

    Get ASF file video width.

Arguments:

    file name.

Return Value:

    video width

*/

u32 GetVideoWidth(s8* pFileName)
{
	FS_FILE*                pFile;
	u32                     size, width;
	u8  tmp;
	
	if ((pFile = dcfOpen(pFileName, "r")) == NULL) {
        DEBUG_ASF("ASF open file error!!!\n");
        return 0;
    }

	if(dcfRead(pFile, tempbuf, 224 , &size) == 0) {
    	DEBUG_ASF("ASF read file error!!!\n");
        return 0;
    }

	if(dcfClose(pFile, &tmp) == 0) {
        DEBUG_ASF("dcfClose() error!!!\n");
    }

	memcpy(&width, (tempbuf + 0xd4), 4);

	printf("width = %d\n", width);
	return width;
}

/*

Routine Description:

    Get ASF file video height.

Arguments:

    file name.

Return Value:

    video height

*/

u32 GetVideoHeight(s8* pFileName)
{
	FS_FILE*                pFile;
	u32                     size, height;
	u8  tmp;
	
	if ((pFile = dcfOpen(pFileName, "r")) == NULL) {
        DEBUG_ASF("ASF open file error!!!\n");
        return 0;
    }

	if(dcfRead(pFile, tempbuf, 224 , &size) == 0) {
    	DEBUG_ASF("ASF read file error!!!\n");
        return 0;
    }

	if(dcfClose(pFile, &tmp) == 0) {
        DEBUG_ASF("dcfClose() error!!!\n");
    }

	memcpy(&height, (tempbuf + 0xd8), 4);

	printf("height = %d\n", height);

	return height;
}

/*

Routine Description:

    Get ASF file video duration.

Arguments:

    file name.

Return Value:

    video duration

*/

u32 GetVideoDuration(s8* pFileName)
{
	FS_FILE*                pFile;
	u32                     size, Duration;
	s64 temp;
	u64 duration;
	u8  tmp;

	if ((pFile = dcfOpen(pFileName, "r")) == NULL) {
        DEBUG_ASF("ASF open file error!!!\n");
        return 0;
    }

	if(dcfRead(pFile, tempbuf, 102 , &size) == 0) {
    	DEBUG_ASF("ASF read file error!!!\n");
        return 0;
    }

	if(dcfClose(pFile, &tmp) == 0) {
        DEBUG_ASF("dcfClose() error!!!\n");
    }

	memcpy(&duration, (tempbuf + 94), 8);

	temp = duration.hi;
	temp = ((temp << 32) +  duration.lo);
	
//  DEBUG_ASF("%ld %ld %ld\n",temp,duration.hi,duration.lo);
    if(Get_p2p_playback==1)
        Duration = ((u32)(temp / 1000)) - 30000;
    else
	    Duration = ((u32)(temp / 10000000)) - 3;
    
//  DEBUG_ASF("%ld\n",Duration);
	return Duration;
}

u32 GetCaptureVideoStatus(void)
{
    return OpenFile;
}
/*

Routine Description:

    Initialize ASF file.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfInit(void)
{
   /* initialize video */
    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    mpeg4Init();
    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
    VideoCodecInit();
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
    mjpgInit();
    #endif
	
	memset_hw(AsfAudioZeroBuf, 0x80, IIS_CHUNK_SIZE);
	
#ifdef ASF_AUDIO
    /* initialize audio */
    asfAudioInit();

    /* initialize audio */
    iisInit();
  #if !IIS1_REPLACE_IIS5
    iis5Init();
  #endif
  #if (AUDIO_DEVICE == AUDIO_IIS_ALC5621)
    Init_IIS_ALC5621_rec();
    ALC5623_test = 1;
    iisInit();
    iis5Init();
  #endif
#endif

#if CDVR_LOG
    LogFileSemEvt   = OSSemCreate(1);
#endif

    return 1;
}

/*

Routine Description:

    The test routine of ASF file.

Arguments:

    None.

Return Value:

    None.

*/
void asfTest(void)
{

}

/*

Routine Description:

    Capture video.

Arguments:

    ZoomFactor  - Zoom factor.
    Mode        - ASF_CAPTURE_NORMAL        // normal mode
                  ASF_CAPTURE_OVERWRITE     // fixed length overwrite mode
                  ASF_CAPTURE_EVENT         // event mode

Return Value:

    0 - Failure.
    1 - Success.

*/
/* Peter 070104 */
#if (!MULTI_CHANNEL_VIDEO_REC)
s32 asfCaptureVideo(s32 ZoomFactor, u32 Mode)
{
    int     i, Result;
    u8      err;
    if (Mode == ASF_CAPTURE_NORMAL)
        DEBUG_ASF("ASF_CAPTURE_NORMAL\n");
    if (Mode & ASF_CAPTURE_OVERWRITE_ENA)
        DEBUG_ASF("ASF_CAPTURE_OVERWRITE_ENA\n");
    if (Mode & ASF_CAPTURE_EVENT_GSENSOR_ENA)
        DEBUG_ASF("ASF_CAPTURE_EVENT_GSENSOR_ENA\n");
    if (Mode & ASF_CAPTURE_EVENT_MOTION_ENA)
        DEBUG_ASF("ASF_CAPTURE_EVENT_MOTION_ENA\n");
    if (Mode & ASF_CAPTURE_EVENT_ALARM_ENA)
        DEBUG_ASF("ASF_CAPTURE_EVENT_ALARM_ENA\n");
    if (Mode & ASF_CAPTURE_SCHEDULE_ENA)
        DEBUG_ASF("ASF_CAPTURE_SCHEDULE_ENA\n");

    /* video */
    asfVideoFrameCount=0;

    asfDataPacketPreSendTime    = 0;    //Lsk 090309
    asfDataPacketSendTime       = 0;
    asfDataPacketFormatFlag     = 0;
    VideoPictureIndex           = 0;
    VideoBufMngReadIdx      = 0;
    VideoBufMngWriteIdx     = 0;
    asfVopCount                 = 0;
#if FORCE_FPS
    asfDummyVopCount            = 0;
    DummyChunkTime              = 0;
#endif
    asfVidePresentTime          = PREROLL; //Lsk 090309
    siuSkipFrameRate            = 0;
    MPEG4_Mode                  = 0;    // 0: record, 1: playback
    Video_Mode                  = 0;    // 0: record, 1: playback
    CurrentVideoSize            = 0;
    asfCaptureMode              = Mode;
    asfIndexTable               = (ASF_IDX_SIMPLE_INDEX_ENTRY*)(mpeg4IndexBuf+sizeof(ASF_SIMPLE_INDEX_OBJECT));
    for(i = 0; i < VIDEO_BUF_NUM; i++) {
        VideoBufMng[i].buffer   = VideoBuf;
    }
  
    // reset MPEG-4 hardware
    SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
    SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100;
    mpeg4SetVideoResolution(mpeg4Width, mpeg4Height);   /*Peter 1116 S*/
    memset(unMp4FrameDuration, 0, sizeof(unMp4FrameDuration));
    memset(ISUFrameDuration, 0, sizeof(ISUFrameDuration));

#ifdef ASF_AUDIO
    /* audio */
    asfAudiChunkCount       = 0;
    asfAudiPresentTime      = PREROLL; //Lsk 090309
    iisSounBufMngReadIdx    = 0;
    iisSounBufMngWriteIdx   = 0;
  #if AUDIO_IN_TO_OUT
    iisSounBufMngPlayIdx    = 0;
  #endif
    IISMode                 = 0;    // 0: record, 1: playback
    IISTime                 = 0;    /* Peter 070104 */
    IISTimeUnit             = (IIS_RECORD_SIZE * 1000) / IIS_SAMPLE_RATE;  /* milliscends */ /* Peter 070104 */
    CurrentAudioSize        = 0;
    /* initialize sound buffer */
    for(i = 0; i < IIS_BUF_NUM; i++) {
        iisSounBufMng[i].buffer     = iisSounBuf[i];
    }
#endif  // ASF_AUDIO

    /* file */
    asfDataPacketCount  = 0;
    asfIndexTableIndex  = 0;
    asfIndexEntryTime   = 0;

#if CDVR_LOG
    LogFileStart                = 0;
    LogFileNextStart            = 0;
    LogFileCurrent              = 0;
    pLogFileEnd                 = (u8*)0;
    pLogFileMid                 = (u8*)0;
    LogFileIndex[LogFileStart]  = LogFileBuf;
#endif


    /* refresh semaphore state */
    Output_Sem();
    /*
    while(VideoCmpSemEvt->OSEventCnt > 0) {
        OSSemAccept(VideoCmpSemEvt);
    }
    */
    OSSemSet(VideoCmpSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoCmpSemEvt is %d.\n", err);
    }

    for(i=0; i<MULTI_CHANNEL_MAX; i++)
    {
        OSSemSet(VideoRTPCmpSemEvt[i], 0, &err);
        if (err != OS_NO_ERR) {
            DEBUG_ASF("OSSemSet Error: VideoRTPCmpSemEvt is %d.\n", err);
        }
    }
    Output_Sem();

    while(VideoTrgSemEvt->OSEventCnt > (VIDEO_BUF_NUM - 2)) {
        OSSemAccept(VideoTrgSemEvt);
    }
    while(VideoTrgSemEvt->OSEventCnt < (VIDEO_BUF_NUM - 2)) {
        OSSemPost(VideoTrgSemEvt);
    }
    //OSSemSet(VideoTrgSemEvt, VIDEO_BUF_NUM - 2, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
    }
    Output_Sem();


#ifdef  ASF_AUDIO
    /*
    while(iisCmpSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisCmpSemEvt);
    }
    */
    OSSemSet(iisCmpSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: iisCmpSemEvt is %d.\n", err);
    }
    for(i=0; i<MULTI_CHANNEL_MAX; i++)
    {
    	OSSemSet(AudioRTPCmpSemEvt[i], 0, &err);
       	if (err != OS_NO_ERR) {
            DEBUG_ASF("OSSemSet Error: AudioRTPCmpSemEvt is %d.\n", err);
        }
    }
    Output_Sem();
    while(iisTrgSemEvt->OSEventCnt > (IIS_BUF_NUM - 2)) {
        OSSemAccept(iisTrgSemEvt);
    }
    while(iisTrgSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
        OSSemPost(iisTrgSemEvt);
    }
    Output_Sem();
  #if AUDIO_IN_TO_OUT
    while(iisPlaybackSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisPlaybackSemEvt);
    }
  #endif
#if ((AUDIO_OPTION == AUDIO_ADC_DAC) || (AUDIO_OPTION == AUDIO_ADC_IIS))
    adcInit(1);
    //#if AUDIO_IN_TO_OUT
    //uiInitDAC_Play();
    //#endif
#endif  // AUDIO_ADC_DAC
#endif

/*CY 0629 S*/
#if RFIU_SUPPORT
   #if RFI_TEST_TX_PROTOCOL_B1
        //--TX--//
        gRfiu_MpegEnc_Sta[0]=RFI_MPEGENC_TASK_RUNNING;

        if(gRfiu_TX_Sta[0]==RFI_TX_TASK_NONE)
        {
           DEBUG_ASF("====RFIU1_TASK Create====\n");
           OSTaskCreate(RFIU_TX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_0, RFIU_TASK_STACK_UNIT0, RFIU_TASK_PRIORITY_UNIT0);
           gRfiu_TX_Sta[0]=RFI_TX_TASK_RUNNING;
        }
    #elif RFI_TEST_TX_PROTOCOL_B2
        //--TX--//
        gRfiu_MpegEnc_Sta[1]=RFI_MPEGENC_TASK_RUNNING;

        if(gRfiu_TX_Sta[1]==RFI_TX_TASK_NONE)
        {
           DEBUG_ASF("====RFIU2_TASK Create====\n");
           OSTaskCreate(RFIU_TX_TASK_UNITX, RFIU_TASK_PARAMETER_UNIT_1, RFIU_TASK_STACK_UNIT1, RFIU_TASK_PRIORITY_UNIT1);
           gRfiu_TX_Sta[1]=RFI_TX_TASK_RUNNING;
        }
    #endif
#endif

    /* write video file */
    Result  = asfCaptureVideoFile(ZoomFactor);
    if (Result == 0)    // asfCaptureVideoFile Error
    {
        /* reset the capture control if error */
        sysCaptureVideoStart = 0;
        sysCaptureVideoStop = 1;
        CaptureErr = 1;
    }

/*CY 0629 E*/

/*CY 0613 S*/

#ifdef  ASF_AUDIO

    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }

    OSSemSet(iisTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
    }

  #if AUDIO_IN_TO_OUT
    while(iisPlaybackSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisPlaybackSemEvt);
    }
  #endif
#endif

    /*
    while(VideoTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(VideoTrgSemEvt);
    }
    */
    OSSemSet(VideoTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
    }

#if RFIU_SUPPORT
   #if RFI_TEST_TX_PROTOCOL_B1
        //--TX--//
        gRfiu_MpegEnc_Sta[0]=RFI_MPEGENC_TASK_NONE;

        if(gRfiu_TX_Sta[0]==RFI_TX_TASK_RUNNING)
        {
           gRfiuUnitCntl[0].TX_Task_Stop=1;
           OSTimeDly(4);
           OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_TASK_PRIORITY_UNIT0);
           gRfiu_TX_Sta[0]=RFI_TX_TASK_NONE;
           gRfiuUnitCntl[0].TX_Task_Stop=0;
           DEBUG_ASF("====RFIU1_TASK Delete====\n");
        }

        if(gRfiu_WrapEnc_Sta[0] == RFI_WRAPENC_TASK_RUNNING)
        {
        #if RFIU_RX_AUDIO_RETURN
            if(guiIISCh4PlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISCh4PlayDMAId);
                guiIISCh4PlayDMAId = 0xFF;
            }
        #endif
           gRfiuUnitCntl[0].TX_Wrap_Stop=1;
           OSTimeDly(3);
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT0);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT0);
           gRfiu_WrapEnc_Sta[0]=RFI_WRAPENC_TASK_NONE;
           gRfiuUnitCntl[0].TX_Wrap_Stop=0;
           DEBUG_ASF("====RFIU1_WRAP Delete====\n");
         }

    #elif RFI_TEST_TX_PROTOCOL_B2
        //--TX--//
        gRfiu_MpegEnc_Sta[1]=RFI_MPEGENC_TASK_NONE;

        if(gRfiu_TX_Sta[1]==RFI_TX_TASK_RUNNING)
        {
           gRfiuUnitCntl[1].TX_Task_Stop=1;
           OSTimeDly(4);
           OSTaskSuspend(RFIU_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_TASK_PRIORITY_UNIT1);
           gRfiu_TX_Sta[0]=RFI_TX_TASK_NONE;
           gRfiuUnitCntl[1].TX_Task_Stop=0;
           DEBUG_ASF("====RFIU2_TASK Delete====\n");
        }

        if(gRfiu_WrapEnc_Sta[1] == RFI_WRAPENC_TASK_RUNNING)
        {
        #if RFIU_RX_AUDIO_RETURN
            if(guiIISCh4PlayDMAId != 0xFF)
            {
                marsDMAClose(guiIISCh4PlayDMAId);
                guiIISCh4PlayDMAId = 0xFF;
            }
        #endif

           gRfiuUnitCntl[1].TX_Wrap_Stop=1;
           OSTimeDly(3);
           OSTaskSuspend(RFIU_WRAP_TASK_PRIORITY_UNIT1);
           OSTaskDel(RFIU_WRAP_TASK_PRIORITY_UNIT1);
           gRfiu_WrapEnc_Sta[1]=RFI_WRAPENC_TASK_NONE;
           gRfiuUnitCntl[1].TX_Wrap_Stop=0;
           DEBUG_ASF("====RFIU2_WRAP Delete====\n");
        }
    #endif
#endif



    /* delay until mpeg4 and IIS task reach pend state */
    OSTimeDly(6);
    /* suspend mpeg4 task */
    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
	mpeg4SuspendTask();
    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
	VideoTaskSuspend();
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
	mjpgSuspendTask();
    #endif
#ifdef ASF_AUDIO
	for(i=0,IIS_Task_Stop=1; (!IIS_Task_Pend)&&(i<30); i++) {

        OSTimeDly(1);
    }
    iisStopRec_ch(0);
    iisSuspendTask();
  #if AUDIO_IN_TO_OUT
	for(i=0,IIS_PlaybackTask_Stop=1; (!IIS_PlayTask_Pend)&&(i<30); i++) {
        OSTimeDly(1);
    }
    iisStopPlay();
    iisSuspendPlaybackTask();
  #endif
#endif

    /* refresh semaphore state */
    Output_Sem();
    /*
    while(VideoCmpSemEvt->OSEventCnt > 0) {
    OSSemAccept(VideoCmpSemEvt);
    }
    */
    OSSemSet(VideoCmpSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoCmpSemEvt is %d.\n", err);
    }
    for(i=0; i<MULTI_CHANNEL_MAX; i++)
    {
        OSSemSet(VideoRTPCmpSemEvt[i], 0, &err);
        if (err != OS_NO_ERR) {
            DEBUG_ASF("OSSemSet Error: VideoRTPCmpSemEvt is %d.\n", err);
        }
    }
    Output_Sem();

    /*
    while(VideoTrgSemEvt->OSEventCnt < (VIDEO_BUF_NUM - 2)) {
    OSSemPost(VideoTrgSemEvt);
    }
    */
    OSSemSet(VideoTrgSemEvt, VIDEO_BUF_NUM - 2, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
    }
    Output_Sem();


#ifdef ASF_AUDIO
    /*
    while(iisCmpSemEvt->OSEventCnt > 0) {
    OSSemAccept(iisCmpSemEvt);
    }
    */
    OSSemSet(iisCmpSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: iisCmpSemEvt is %d.\n", err);
    }
    for(i=0; i<MULTI_CHANNEL_MAX; i++)
    {
        OSSemSet(AudioRTPCmpSemEvt[i], 0, &err);
        if (err != OS_NO_ERR) {
            DEBUG_ASF("OSSemSet Error: AudioRTPCmpSemEvt is %d.\n", err);
        }
    }
    Output_Sem();
    /*
    while(iisTrgSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
    OSSemPost(iisTrgSemEvt);
    }
    */
    OSSemSet(iisTrgSemEvt, IIS_BUF_NUM - 2, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
    }
    Output_Sem();
  #if AUDIO_IN_TO_OUT
    while(iisPlaybackSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisPlaybackSemEvt);
    }
  #endif
#endif
/*CY 0613 E*/

    //DEBUG_ASF("ASF file captured - VOP count = %d\n", asfVopCount);
#ifdef  ASF_AUDIO
    //DEBUG_ASF("ASF file captured - Audio count = %d\n", asfAudiChunkCount);
#endif
    //DEBUG_ASF("ASF file captured - File size = %d bytes\n", asfHeaderSize + asfDataSize + asfIndexSize);
    //DEBUG_ASF("asfVidePresentTime = %d, IISTime = %d\n", asfVidePresentTime, IISTime);

    // reset IIS hardware
    iisReset(IIS_SYSPLL_SEL_48M);

    return  Result;
}
#endif

/*

Routine Description:
    if close file by size, get the max packet count

Arguments:

Return Value:
    max packet count

*/
u32 TriggerModeGetMaxPacketCount(void)
{
    return (TRIGGER_MODE_MAX_FILE_SIZE / ASF_DATA_PACKET_SIZE);
}

/*

Routine Description:
    if close file by size, get the max packet count

Arguments:

Return Value:
    max packet count

*/
u32 ManualModeGetMaxPacketCount(void)
{
        return (MANUAL_MODE_MAX_FILE_SIZE / ASF_DATA_PACKET_SIZE);
}

/*

Routine Description:
    FSM : Check if Event trigger occur

Arguments:

Return Value:

*/
void CheckEventTrigger(void)
{
    /***********************************
    *** Check if Event trigger occur ***
    ***********************************/
    if((asfCaptureMode & ASF_CAPTURE_EVENT_ALARM_ENA) && AlarmDetect)
    {
        DEBUG_ASF("Alarm detect event trigger\n");
        EventTrigger    = CAPTURE_STATE_TRIGGER;
    }
    #if (MOTIONDETEC_ENA || HW_MD_SUPPORT || MOTION_TRIGGRT_REC)
    else if((asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA) && MD_Diff)
    {
        DEBUG_ASF("Motion detect event trigger\n");
        EventTrigger    = CAPTURE_STATE_TRIGGER;
        MotionlessSecond = 0;
    }
    #endif
    #if G_SENSOR_DETECT
    else if((asfCaptureMode & ASF_CAPTURE_EVENT_GSENSOR_ENA) && GSensorEvent)
    {
        DEBUG_ASF("G-Sensor detect event trigger\n");
        EventTrigger    = CAPTURE_STATE_TRIGGER;
    }
    #endif


    if(EventTrigger == CAPTURE_STATE_TRIGGER)
    {
        //Start_MPEG4TimeStatistics = 1;
        /***********************
        *** Reset time count ***
        ***********************/
        asfTimeStatistics   = 0;
        RTCseconds          = 0;
        MotionlessSecond    = 0;
        asfRecTimeLenTotal  = asfRecTimeLen;
        asfLocalTimeInSec   = g_LocalTimeInSec;
        asfEventTriggerTime = 0;
    }
}

/*

Routine Description:
    FSM : Check if Time's up

Arguments:

Return Value:

*/
void CheckRecordTimeUP(void)
{
    u32 CurrentTime, LocalTimeInSec;
    /*********************************************************
    ***             延長 trigger 後的錄影時間              ***
    *********************************************************/
    CurrentTime     = asfTimeStatistics / 1000;
    if(g_LocalTimeInSec >= asfLocalTimeInSec)
        LocalTimeInSec  = g_LocalTimeInSec  - asfLocalTimeInSec;
    else
        LocalTimeInSec  = 0;
    if((asfCaptureMode & ASF_CAPTURE_EVENT_ALARM_ENA) && AlarmDetect)
    {
        if( (LocalTimeInSec >= asfEventInterval) && (CurrentTime >= (asfEventTriggerTime + asfEventInterval)) && ((CurrentTime + asfEventExtendTime) > asfRecTimeLenTotal) )
        {
            asfEventTriggerTime   = CurrentTime;
            asfRecTimeLenTotal    = CurrentTime + asfEventExtendTime;
            if(asfRecTimeLenTotal > asfSectionTime)
                asfRecTimeLenTotal    = asfSectionTime;
            DEBUG_ASF("Extend record time to %d sec\n", asfRecTimeLenTotal);
        }
        AlarmDetect           = 0;
    }
    #if (MOTIONDETEC_ENA || HW_MD_SUPPORT || MOTION_TRIGGRT_REC)
    else if((asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA) && MD_Diff)
    {
        if( (LocalTimeInSec >= asfEventInterval) && (CurrentTime >= (asfEventTriggerTime + asfEventInterval)) && ((CurrentTime + asfEventExtendTime) > asfRecTimeLenTotal) )
        {
            asfEventTriggerTime   = CurrentTime;
            asfRecTimeLenTotal    = CurrentTime + asfEventExtendTime;
            if(asfRecTimeLenTotal > asfSectionTime)
                asfRecTimeLenTotal    = asfSectionTime;
            DEBUG_ASF("Ch extend record time to %d sec\n", asfRecTimeLenTotal);
        }
        MD_Diff               = 0;
    }
    #endif
    #if G_SENSOR_DETECT
    else if((asfCaptureMode & ASF_CAPTURE_EVENT_GSENSOR_ENA) && GSensorEvent)
    {
        if( (LocalTimeInSec >= asfEventInterval) && (CurrentTime >= (asfEventTriggerTime + asfEventInterval)) && ((CurrentTime + asfEventExtendTime) > asfRecTimeLenTotal) )
        {
            asfEventTriggerTime   = CurrentTime;
            asfRecTimeLenTotal    = CurrentTime + asfEventExtendTime;
            if(asfRecTimeLenTotal > asfSectionTime)
                asfRecTimeLenTotal    = asfSectionTime;
            DEBUG_ASF("Ch extend record time to %d sec\n", asfRecTimeLenTotal);
        }
        GSensorEvent          = 0;
    }
    #endif

    /*********************************************************
    *** Time's up!  Store current MPEG4 Encoder WriteIndex ***
    *********************************************************/
    #if(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_ASF)
        if((CurrentTime >= asfRecTimeLenTotal) || (LocalTimeInSec >= (asfRecTimeLenTotal + 5)))
        {
            EventTrigger = CAPTURE_STATE_TIMEUP;
        }
    #elif(MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_AVI)
        if((MotionlessSecond > MotionlessRecTimeLen)||(RTCseconds > asfRecTimeLen))
        {
            Last_VideoBufMngReadIdx = VideoBufMngWriteIdx;
            EventTrigger = CAPTURE_STATE_TIMEUP;
        }
    #endif
}

/*

Routine Description:
    FSM : Check if Write finish

Arguments:

Return Value:

*/
void CheckWriteFinish(void)
{

    EventTrigger    = CAPTURE_STATE_WAIT;
    AlarmDetect     = 0;

    #if (MOTIONDETEC_ENA)
        MD_Diff         = 0;
        MD_FullTVRun    = 1;
        MD_HalfTVRun    = 1;
        MD_PanelRun     = 1;
    #elif HW_MD_SUPPORT
        MD_Diff         = 0;
    #elif MOTION_TRIGGRT_REC
        MD_Diff         = 0;
    #endif

    #if (G_SENSOR_DETECT)
        GSensorEvent    = 0;
    #endif

    #if PREVIEW_MD_TRIGGER_REC
        uiCaptureVideoStop();
    #endif




    if(EventTrigger == CAPTURE_STATE_WAIT)
    {
        end_idx = VideoBufMngReadIdx;
    }
}



/*

Routine Description:
    Indicate record status
Arguments:
    timetick
Return Value:
    timetick
*/
u32 IndicateRecordStatus(u32 timetick)
{
    if((OS_tickcounter - timetick) > 40)  // 1S
    {
        ledon      ^= 1;

        if(ledon)
        {
            if((!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) || EventTrigger)
            {
                osdDrawVideoOn(UI_OSD_DRAW);  // Rec
            }
            else
            {
                osdDrawVideoOn(UI_OSD_NONE);  // Detecting
            }
        }
        else
        {
		    osdDrawVideoOn(UI_OSD_CLEAR);      // Clear
        }
        timetick = OS_tickcounter;

    }
    return timetick;
}

/*

Routine Description:
    project special seeting when enter asf capture process
Arguments:

Return Value:

*/
void EnterASFCapture(void)
{
    AlarmDetect = 0;
    /*** set GOP ***/
    IVOP_PERIOD = 60;
}

/*

Routine Description:
    project special seeting when exit asf capture process
Arguments:

Return Value:

*/
void ExitASFCapture(void)
{
}

/*

Routine Description:
    project special OSD seeting when enter asf capture process
Arguments:

Return Value:

*/
void CaptureModeOSDSetting(void)
{

    #if (UI_PREVIEW_OSD == 1)
        if(sysTVOutOnFlag) //Tv out
        {
            //DEBUG_ASF("Trace4: Capture video...%x\n",tvFRAME_CTL);
            uiOSDPreviewInit();

            if((!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))
            {
                osdDrawVideoOn(UI_OSD_DRAW);  // Rec
            }
            else
            {
                osdDrawVideoOn(UI_OSD_NONE);  // Detecting
            }
        }
    #endif

        sysVoiceRecStart    = 2;
        RTCseconds          = 0;

        timerCountPause(1, 0);
        timerCountEnable(1,1);
        timerInterruptEnable(1,1);  // Using RTC record file time,timer2 for for counting
}

#if SDC_WRITE_READ_TEST
s32 MemoryTestFunction(void)
{
    u32             i, k;
    u8              j;
    u32             writesize;
    FS_FILE*        pFile;
	u8  tmp;
	
    // for disk full control
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit


    while((free_size > (MPEG4_MAX_BUF_SIZE * 5)/1024) || (sysCaptureVideoStop == 0))
    {
    #if 0
        DEBUG_ASF("SDRAM Check...\n");
        //uiMenuOSDStringByColor(OSD_Width , "SDRAM Check...      ", 8 , 16, 8 , 72+16 , 2 , 0xc0, 0x00);
        if(0==SDRAM_Test((u32 *)PKBuf))
        {
            DEBUG_ASF("SDRAM FAIL\n");
            //uiMenuOSDStringByColor(OSD_Width , "SDRAM FAIL        ", 8 , 16, 8 , 72+16 , 2 , 0xc0, 0x00);
            return 0;
        }
        else
        {
            DEBUG_ASF("SDRAM OK\n");
            //uiMenuOSDStringByColor(OSD_Width , "SDRAM OK        ", 8 , 16, 8 , 72+16 , 2 , 0xc0, 0x00);
        }
    #endif

    #if 1

        if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_ASF, 0)) == NULL) {
            DEBUG_ASF("ASF create file error!!!\n");
            //uiMenuOSDStringByColor(OSD_Width , "ASF create file error!!!   ", 8 , 16, 8 , 40 , 2 , 0xc0, 0x00);
            return 0;
        }

        DEBUG_ASF("Write test file\n");

        //uiMenuOSDStringByColor(OSD_Width , "Write test file      ", 8 , 16, 8 , 40 , 2 , 0xc0, 0x00);
        //uiMenuOSDStringByColor(OSD_Width , "                     ", 8 , 16, 8 , 56 , 2 , 0xc0, 0x00);
        //uiMenuOSDStringByColor(OSD_Width , "Check...        ", 8 , 16, 8 , 72 , 2 , 0xc0, 0x00);

        for(k = 0; k < 5; k++) {
            for(i = 0, j = 0; i < MPEG4_MAX_BUF_SIZE; i++, j++) {
                VideoBuf[i] = j;
            }
	        dcfWrite(pFile, VideoBuf, MPEG4_MAX_BUF_SIZE, &writesize);
            if (writesize != MPEG4_MAX_BUF_SIZE) {
                DEBUG_ASF("dcfWrite() error!!!\n");
                //uiMenuOSDStringByColor(OSD_Width , "dcfWrite() error!!!     ", 8 , 16, 8 , 40 , 2 , 0xc0, 0x00);
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            }
	    }
	    dcfCloseFileByIdx(pFile, 0, &tmp);

        DEBUG_ASF("Compare test file %s ", dcfGetPlaybackFileListTail()->pDirEnt->d_name);
        //uiMenuOSDStringByColor(OSD_Width , "Compare test file       ", 8 , 16, 8 , 40 , 2 , 0xc0, 0x00);
        //uiMenuOSDStringByColor(OSD_Width , dcfGetPlaybackFileListTail()->pDirEnt->d_name, 8 , 16, 8 , 56 , 2 , 0xc0, 0x00);
	    if ((pFile = dcfOpen((s8 *)dcfGetPlaybackFileListTail()->pDirEnt->d_name, "r")) == NULL)
	    {
	        DEBUG_ASF("dcfOpen() error!!!\n");
            //uiMenuOSDStringByColor(OSD_Width , "dcfOpen() error!!!      ", 8 , 16, 8 , 40 , 2 , 0xc0, 0x00);
	    	return NULL;
	    }

        for(k = 0; k < 5; k++) {
            memset(VideoBuf, 0, MPEG4_MAX_BUF_SIZE);
            if(dcfRead(pFile, VideoBuf, MPEG4_MAX_BUF_SIZE, &writesize) != 1) {
                DEBUG_ASF("dcfRead() error!!!\n");
                //uiMenuOSDStringByColor(OSD_Width , "dcfRead() error!!!     ", 8 , 16, 8 , 40 , 2 , 0xc0, 0x00);
                dcfClose(pFile, &tmp);
                return 0;
            }
            for(i = 0, j = 0; i < MPEG4_MAX_BUF_SIZE; i++, j++) {
                if(VideoBuf[i] != j) {
                    DEBUG_ASF("dcfRead() data error!!! [0x%x]0x%x != 0x%x\n", k * MPEG4_MAX_BUF_SIZE + i, VideoBuf[i], j);
                    //uiMenuOSDStringByColor(OSD_Width , "SD R/W Err!      ", 8 , 16, 8 , 40 , 2 , 0xc0, 0x00);
                    dcfClose(pFile, &tmp);
                    return 0;
                }
            }
	    }
	    dcfClose(pFile, &tmp);
        DEBUG_ASF("Success\n");
        //uiMenuOSDStringByColor(OSD_Width , "Check OK        ", 8 , 16, 8 , 72 , 2 , 0xc0, 0x00);
     #endif


        // for disk full control
        diskInfo            = &global_diskInfo;
        bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
        free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit
    }

    DEBUG_ASF("Disk full!!!\n");
    //uiMenuOSDStringByColor(OSD_Width , "Disk full!!!     ", 8 , 16, 8 , 40 , 2 , 0xc0, 0x00);
    
    return 0;
}
#endif


/*
Routine Description:

    Trigger mode capture video file.

Arguments:

    ZoomFactor - Zoom factor.

Return Value:

    0 - Failure.
    1 - Success.

Capture file core, don't add any project special purpose
*/
#if( ((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) || (!MULTI_CHANNEL_VIDEO_REC))
s32 asfCaptureVideoFile(s32 ZoomFactor)
{
    FS_FILE*        pFile=NULL;
    u16             video_value;
    u16             video_value_max;
    u32             monitor_value;
    u32             timetick;


#ifdef  ASF_AUDIO
    u16             audio_value;
    u16             audio_value_max;
#endif

    u32             CurrentFileSize;

	#if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
    u8              err;
	#endif
	
#if FINE_TIME_STAMP
    s32             TimeOffset;
#endif

#if(SHOW_UI_PROCESS_TIME == 1)
    u32             time1;
#endif

#if (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
    s32             PcmOffset   = 0;
#endif


    s32             SkipFrameNum;
    u32             PreRecordFrameNum=0;

	#if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
    u32             MaxPacketCount;
	#endif
    u8              TriggerModeFirstFileFlag = 0;
	#if AUDIO_IN_TO_OUT
    u8              InitDACPlayFlag = 0;
	#endif
    u32             SingleFrameSize;
    u32             FreeSpaceThreshold=0;
    u32             FreeSpace=0;
    u32             timeoutcnt=0;
    u8  level;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    int i;
	u8  tmp;
   //--------------------------//
    VideoTimeStatistics = 0;
    asfTimeStatistics = 0;
    asfLocalTimeInSec   = g_LocalTimeInSec;
    DirectlyTimeStatistics = 0;

    #if SDC_WRITE_READ_TEST
    return MemoryTestFunction();
    #endif


    #if MOTIONDETEC_ENA
    MD_FullTVRun    = 1;
    MD_HalfTVRun    = 1;
    MD_PanelRun     = 1;
    #endif

    EnterASFCapture();

    /***********************************************************
    *** calculate max packet count : file size / packet size ***
    ***********************************************************/
    if(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) {
        #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
        MaxPacketCount = TriggerModeGetMaxPacketCount();
        #endif
    } else {
        #if(MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
        MaxPacketCount = ManualModeGetMaxPacketCount();
        #endif
    }


    /****************************************************************
    *** calculate how many frames need for pre-record             ***
    *** PreRecordFrameNum = PreRecordTime * FPS                   ***
    ****************************************************************/
    if(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)
    {

        if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_30)
            PreRecordFrameNum = PreRecordTime * 30;
        else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_15)
            PreRecordFrameNum = PreRecordTime * 15;
        else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_5)
            PreRecordFrameNum = PreRecordTime * 5;
        else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_60)
            PreRecordFrameNum = PreRecordTime * 60;
        else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_10)
            PreRecordFrameNum = PreRecordTime * 10;
    }


    /*********************
    *** reset variable ***
    *********************/
    ResetPayloadPresentTime = 1;
    WantChangeFile  = 0;
    LastAudio       = 0;
    LastVideo       = 0;
    GetLastAudio    = 0;
    GetLastVideo    = 0;
    EventTrigger    = CAPTURE_STATE_WAIT;
    MPEG4_Error     = 0;


    sysReady2CaptureVideo=0;


#if G_SENSOR_DETECT
    GSensorEvent    = 0;
#if (G_SENSOR == G_SENSOR_LIS302DL)
    i2cPolling_LIS302DL();
#elif (G_SENSOR == G_SENSOR_H30CD)
    i2cPolling_H30CD();
#elif (G_SENSOR == G_SENSOR_DMARD03)
    i2cPolling_DMARD03();
#endif
#endif

#if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
    // Enable timer2 for fine tune frame time
    timer2Setting();
#endif


    // for disk full control
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit


    if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
        SetIVOP = 1;
        if((pFile = asfCreateFile(1)) == 0) {
            return 0;
        }
    }

    //DEBUG_ASF("asfSectionTime = %d\n", asfSectionTime);
    //DEBUG_ASF("VideoCmpSemEvt = %d\n", VideoCmpSemEvt->OSEventCnt);
    //DEBUG_ASF("VideoTrgSemEvt = %d\n", VideoTrgSemEvt->OSEventCnt);
    //DEBUG_ASF("VideoCpleSemEvt = %d\n", VideoCpleSemEvt->OSEventCnt);


    video_value     = 0;
    video_value_max = 0;
    monitor_value   = 0;
#ifdef  ASF_AUDIO
    //DEBUG_ASF("iisCmpSemEvt = %d\n", iisCmpSemEvt->OSEventCnt);
    //DEBUG_ASF("iisTrgSemEvt = %d\n", iisTrgSemEvt->OSEventCnt);
    audio_value     = 0;
    audio_value_max = 0;
#endif

    iduCaptureVideo(mpeg4Width,mpeg4Height);

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
    DEBUG_ASF("Asf Capture Time =%d (x50ms)\n",time1);
#endif

	//Lsk 090409 : Let siuCaptureVideo() start immediately after iisResumeTask
	#if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
	    timerCountRead(2, (u32*) &TimeOffset);
    	IISTimeOffset   = TimeOffset >> 8;
	#elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
		timerCountRead(1, (u32*) &TimeOffset);
	    IISTimeOffset   = TimeOffset / 100;
	#endif

	#ifdef ASF_AUDIO
       iisResumeTask();
       //iisStartRec();
       #if AUDIO_IN_TO_OUT
       iisResumePlaybackTask();
	   #endif
    #endif  // #ifdef ASF_AUDIO

  #if (MULTI_CHANNEL_SEL & 0x01)
    isuCaptureVideo(ZoomFactor);
    ipuCaptureVideo();
    siuCaptureVideo(ZoomFactor);
  #elif (MULTI_CHANNEL_SEL & 0x02)
    #if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )
       if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
       {
           if(sysTVinFormat == TV_IN_PAL)
              ciuCaptureVideo_CH1(640, 576/2, mpeg4Width, mpeg4Height/2, CIU1_OSD_EN, mpeg4Width * 2);
           else
              ciuCaptureVideo_CH1(640, 480/2, mpeg4Width, mpeg4Height/2, CIU1_OSD_EN, mpeg4Width * 2);
       }
       else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) )
       {
           if(sysTVinFormat == TV_IN_PAL)
              ciuCaptureVideo_CH1(704, 576/2, mpeg4Width, mpeg4Height/2, CIU1_OSD_EN, mpeg4Width * 2);
           else
              ciuCaptureVideo_CH1(704, 480/2, mpeg4Width, mpeg4Height/2, CIU1_OSD_EN, mpeg4Width * 2);
       }
       else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576) )
       {
           if(sysTVinFormat == TV_IN_PAL)
              ciuCaptureVideo_CH1(720, 576/2, mpeg4Width, mpeg4Height/2, CIU1_OSD_EN, mpeg4Width * 2);
           else
              ciuCaptureVideo_CH1(720, 480/2, mpeg4Width, mpeg4Height/2, CIU1_OSD_EN, mpeg4Width * 2);
       }

    #else
       ciuCaptureVideo_CH1(isuSrcImg.w, isuSrcImg.h, mpeg4Width, mpeg4Height, CIU1_OSD_EN, mpeg4Width);
    #endif
  #elif (MULTI_CHANNEL_SEL & 0x04)
    #if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
        {
            if(sysTVinFormat == TV_IN_PAL)
               ciuCaptureVideo_CH2(640, 576/2, mpeg4Width, mpeg4Height/2, CIU2_OSD_EN, mpeg4Width*2);
            else
               ciuCaptureVideo_CH2(640, 480/2, mpeg4Width, mpeg4Height/2, CIU2_OSD_EN, mpeg4Width*2);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) )
        {
            if(sysTVinFormat == TV_IN_PAL)
               ciuCaptureVideo_CH2(704, 576/2, mpeg4Width, mpeg4Height/2, CIU2_OSD_EN, mpeg4Width*2);
            else
               ciuCaptureVideo_CH2(704, 480/2, mpeg4Width, mpeg4Height/2, CIU2_OSD_EN, mpeg4Width*2);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576) )
        {
            if(sysTVinFormat == TV_IN_PAL)
               ciuCaptureVideo_CH2(720, 576/2, mpeg4Width, mpeg4Height/2, CIU2_OSD_EN, mpeg4Width*2);
            else
               ciuCaptureVideo_CH2(720, 480/2, mpeg4Width, mpeg4Height/2, CIU2_OSD_EN, mpeg4Width*2);
        }
    #else
    ciuCaptureVideo_CH2(isuSrcImg.w, isuSrcImg.h, mpeg4Width, mpeg4Height, CIU2_OSD_EN, mpeg4Width);
    #endif
  #elif (MULTI_CHANNEL_SEL & 0x20)
    #if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )
        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
        {
            if(sysTVinFormat == TV_IN_PAL)
               ciuCaptureVideo_CH5(640, 576/2, mpeg4Width, mpeg4Height/2, CIU5_OSD_EN, mpeg4Width*2);
            else
               ciuCaptureVideo_CH5(640, 480/2, mpeg4Width, mpeg4Height/2, CIU5_OSD_EN, mpeg4Width*2);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576) )
        {
            if(sysTVinFormat == TV_IN_PAL)
               ciuCaptureVideo_CH5(704, 576/2, mpeg4Width, mpeg4Height/2, CIU5_OSD_EN, mpeg4Width*2);
            else
               ciuCaptureVideo_CH5(704, 480/2, mpeg4Width, mpeg4Height/2, CIU5_OSD_EN, mpeg4Width*2);
        }
        else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576) )
        {
            if(sysTVinFormat == TV_IN_PAL)
               ciuCaptureVideo_CH5(720, 576/2, mpeg4Width, mpeg4Height/2, CIU5_OSD_EN, mpeg4Width*2);
            else
               ciuCaptureVideo_CH5(720, 480/2, mpeg4Width, mpeg4Height/2, CIU5_OSD_EN, mpeg4Width*2);
        }
    #else
        ciuCaptureVideo_CH5(isuSrcImg.w, isuSrcImg.h, mpeg4Width, mpeg4Height, CIU5_OSD_EN, mpeg4Width);
    #endif
        ipuCaptureVideo();
        siuCaptureVideo(ZoomFactor);
  #endif


    CaptureModeOSDSetting();

    /************   Start to REC   ************/
    #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
            timeoutcnt=0;
          #if(MULTI_CHANNEL_SEL & 0x01)
            while(isu_avifrmcnt < 4)

          #elif(MULTI_CHANNEL_SEL & 0x02)
            while(ciu_idufrmcnt_ch1 < 4)

          #elif(MULTI_CHANNEL_SEL & 0x04)
            while(ciu_idufrmcnt_ch2 < 4)

          #elif(MULTI_CHANNEL_SEL & 0x08)
            while(ciu_idufrmcnt_ch3 < 4)

          #elif(MULTI_CHANNEL_SEL & 0x10)
            while(ciu_idufrmcnt_ch4 < 4)

          #elif(MULTI_CHANNEL_SEL & 0x20)
            while(ciu_idufrmcnt_ch5 < 4)  

          #else
            while(isu_avifrmcnt < 4)
          #endif
            {
               DEBUG_ASF("asf w1\n");

               OSTimeDly(1);
               timeoutcnt++;
               if ( timeoutcnt>10)
               {
                   DEBUG_ASF("asf t1\n");
                   break;
               }

            }

          #if(MULTI_CHANNEL_SEL & 0x01)
            isu_avifrmcnt           = 0;
            mp4_avifrmcnt           = 0;
            OSSemSet(isuSemEvt, 0, &err);
            sysReady2CaptureVideo   = 1;
            isu_avifrmcnt           = 0;
            timeoutcnt              = 0;
            while(isu_avifrmcnt < 1)
          #elif(MULTI_CHANNEL_SEL & 0x02)
            ciu_idufrmcnt_ch1       = 0;
            mp4_avifrmcnt           = 0;
            OSSemSet(ciuCapSemEvt_CH1, 0, &err);
            sysReady2CaptureVideo   = 1;
            ciu_idufrmcnt_ch1       = 0;
            timeoutcnt              = 0;
            while(ciu_idufrmcnt_ch1 < 1)
          #elif(MULTI_CHANNEL_SEL & 0x04)
            ciu_idufrmcnt_ch2       = 0;
            mp4_avifrmcnt           = 0;
            OSSemSet(ciuCapSemEvt_CH2, 0, &err);
            sysReady2CaptureVideo   = 1;
            ciu_idufrmcnt_ch2       = 0;
            timeoutcnt              = 0;
            while(ciu_idufrmcnt_ch2 < 1)
          #elif(MULTI_CHANNEL_SEL & 0x08)
            ciu_idufrmcnt_ch3       = 0;
            mp4_avifrmcnt           = 0;
            OSSemSet(ciuCapSemEvt_CH3, 0, &err);
            sysReady2CaptureVideo   = 1;
            ciu_idufrmcnt_ch3       = 0;
            timeoutcnt              = 0;
            while(ciu_idufrmcnt_ch3 < 1)
          #elif(MULTI_CHANNEL_SEL & 0x10)
            ciu_idufrmcnt_ch4       = 0;
            mp4_avifrmcnt           = 0;
            OSSemSet(ciuCapSemEvt_CH4, 0, &err);
            sysReady2CaptureVideo   = 1;
            ciu_idufrmcnt_ch4       = 0;
            timeoutcnt              = 0;
            while(ciu_idufrmcnt_ch4 < 1)

          #elif(MULTI_CHANNEL_SEL & 0x20)
            ciu_idufrmcnt_ch5       = 0;
            mp4_avifrmcnt           = 0;
            OSSemSet(ciuCapSemEvt_CH5, 0, &err);
            sysReady2CaptureVideo   = 1;
            ciu_idufrmcnt_ch5       = 0;
            timeoutcnt              = 0;
            while(ciu_idufrmcnt_ch5 < 1)  
          #else
            isu_avifrmcnt           = 0;
            mp4_avifrmcnt           = 0;
            OSSemSet(isuSemEvt, 0, &err);
            sysReady2CaptureVideo   = 1;
            isu_avifrmcnt           = 0;
            timeoutcnt              = 0;
            while(isu_avifrmcnt < 1)
          #endif
            {
                DEBUG_ASF("asf w2\n");

                OSTimeDly(1);
                timeoutcnt++;
                if ( timeoutcnt>5)
                {
                    DEBUG_ASF("asf t2\n");
                    DEBUG_ASF("Error: timeout 2\n");
                    break;
                }
            }

    #else
            sysReady2CaptureVideo=1;
            timeoutcnt=0;
          #if(MULTI_CHANNEL_SEL & 0x01)
            while(isu_avifrmcnt < 1)
          #elif(MULTI_CHANNEL_SEL & 0x02)
            while(ciu_idufrmcnt_ch1 < 1)
          #elif(MULTI_CHANNEL_SEL & 0x04)
            while(ciu_idufrmcnt_ch2 < 1)
          #elif(MULTI_CHANNEL_SEL & 0x08)
            while(ciu_idufrmcnt_ch3 < 1)
          #elif(MULTI_CHANNEL_SEL & 0x10)
            while(ciu_idufrmcnt_ch4 < 1)
          #elif(MULTI_CHANNEL_SEL & 0x20)
            while(ciu_idufrmcnt_ch5 < 1)      
          #else
            while(isu_avifrmcnt < 1)
          #endif
            {
                OSTimeDly(1);
                timeoutcnt++;
                if ( timeoutcnt>10)
                {
                    DEBUG_ASF("\nError!! Wait-CIU/SIU Time Out!! Reboot!:0x%x\n",GpioActFlashSelect);
					sysForceWDTtoReboot();
                    break;
                }
            }
    #endif


    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        if(MPEG4_Task_Go) {
            OSSemAccept(VideoTrgSemEvt);
        }
        mpeg4ResumeTask();
    #elif (VIDEO_CODEC_OPTION == H264_CODEC)
        if(Video_Task_Go) {
            OSSemAccept(VideoTrgSemEvt);
        }
        VideoTaskResume();
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
      	if(MJPG_Task_Go) {
            OSSemAccept(VideoTrgSemEvt);
        }
        mjpgResumeTask();
    #endif

    /*CY 0613 S*/
    DEBUG_ASF("\n------------------Start to REC------------------\n");
    timetick = OS_tickcounter;
    ledon = 1;
    while (sysCaptureVideoStop == 0)
    /*CY 0613 E*/
    {
        if(MPEG4_Error ==1)
        {
            asfCloseFile(pFile);
            return 0;
        }

        #if AUDIO_IN_TO_OUT
        if(IISTime >= IISTimeUnit*32 && InitDACPlayFlag==0)
        {
            InitDACPlayFlag=1;
            uiInitDAC_Play();
        }
        #endif

        /**********************************************************************************************************************
        *** Trigger mode FSM                                                                                                ***
        *** CAPTURE_STATE_WAIT --------> CAPTURE_STATE_TRIGGER --------> CAPTURE_STATE_TIMEUP --------> CAPTURE_STATE_WAIT  ***
        *** WAIT    -> TRIGGER : event trigger, start wrtite A/V bitstream                                                  ***
        *** TRIGGER -> TIMEUP  : Time's up, store lastest video payload index                                               ***
        *** TIMEUP  -> WAIT    : Write Finish film slice, return to wating state                                            ***
        **********************************************************************************************************************/
        if(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) {
            if(EventTrigger == CAPTURE_STATE_WAIT)
    		{
    		    //Start_MPEG4TimeStatistics = 0;
    		    CheckEventTrigger();
                if(EventTrigger == CAPTURE_STATE_TRIGGER)
                {
                    DEBUG_ASF("\n\nFSM : CAPTURE_STATE_TRIGGER\n\n");
                    /****************************************************************
                    *** Calculate how many VOP in SDRAM need to drop              ***
                    ****************************************************************/
                    OS_ENTER_CRITICAL();
                    if(VideoBufMngWriteIdx >= VideoBufMngReadIdx)
                    {
                        //---R---s--W---
                        //if((VideoBufMngWriteIdx - PreRecordFrameNum >= VideoBufMngReadIdx)&&(VideoBufMngWriteIdx - PreRecordFrameNum <= VideoBufMngWriteIdx))
                        if(VideoBufMngWriteIdx >= (VideoBufMngReadIdx + PreRecordFrameNum))
                        {
                            SkipFrameNum = VideoBufMngWriteIdx - PreRecordFrameNum - VideoBufMngReadIdx;
                        }
                        //---R---W--s---
                        //---s---R--W---
                        else
                            SkipFrameNum = 0;
                    }
                    else
                    {
                        //---s---w--R---
                        //---w---R--s---
                        if( (VIDEO_BUF_NUM + VideoBufMngWriteIdx - PreRecordFrameNum) >= VideoBufMngReadIdx)
                        {
                            SkipFrameNum = VIDEO_BUF_NUM + VideoBufMngWriteIdx - PreRecordFrameNum - VideoBufMngReadIdx ;
                        }
                        //---w---s--R---
                        else
                            SkipFrameNum = 0;
                    }
                    OS_EXIT_CRITICAL();

                    /******************************************
                    *** Force Mpeg4 Engine compress I frame ***
                    ******************************************/
                    if(DirectlyTimeStatistics == 0)
                    {
                        SetIVOP = 1;
                        OSTimeDly(2);
                    }
                    /***************************
                    *** drop Audio and video ***
                    ***************************/
                    if(CurrentVideoSize && FreeSpaceControl)
                    {
                        SingleFrameSize = CurrentVideoSize / (VideoCmpSemEvt->OSEventCnt);
                        FreeSpaceThreshold = 4 * 30 * SingleFrameSize + MPEG4_MIN_BUF_SIZE;
                        FreeSpace = MPEG4_MAX_BUF_SIZE - CurrentVideoSize;
                        DEBUG_ASF("FreeSpace control (%d,%d)\n", FreeSpaceThreshold, FreeSpace);
                    }
                    else
                        FreeSpaceControl = 0;

                    DEBUG_ASF("SkipFrameNum = %d\n", SkipFrameNum);

                    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
                    {
                       DEBUG_ASF("Main storage not ready.\n");
                       return 0;
                    }

                    while((VideoBufMng[VideoBufMngReadIdx].flag != FLAG_I_VOP) || (SkipFrameNum > 0)
                        || (FreeSpaceControl && (FreeSpace < FreeSpaceThreshold)))
                    {
                        SkipFrameNum--;
                        video_value = OSSemAccept(VideoCmpSemEvt);
                        if (video_value > 0) {
                            if(video_value_max < video_value)
                                video_value_max = video_value; ////Lsk : for what ?
                            #if CDVR_LOG
                            if(VideoBufMng[VideoBufMngReadIdx].flag == FLAG_I_VOP)
                                LogFileStart    = (LogFileStart + 1) % LOG_INDEX_NUM;
                            #endif
                            asfWriteVirtualVidePayload(&VideoBufMng[VideoBufMngReadIdx]);
                            VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                            OSSemPost(VideoTrgSemEvt);
                            if(FreeSpaceControl)
                                FreeSpace = MPEG4_MAX_BUF_SIZE - CurrentVideoSize;
                        } else {
        				    DEBUG_ASF("\n\nCan't start from I frame!!!\n\n");
                            EventTrigger = CAPTURE_STATE_WAIT;
                            break;
                        }

                        if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
	                    {
	                       DEBUG_ASF("Main storage not ready.\n");
	                       return 0;
	                    }
                    }
                    start_idx = VideoBufMngReadIdx;

                    if(start_idx != end_idx)
                    {
                        DEBUG_ASF("\n Warning!!! lose video slice....\n");
                    }
               #ifdef ASF_AUDIO

                    if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
                    {
                       DEBUG_ASF("Main storage not ready.\n");
                       return 0;
                    }
                    
                    while ((asfAudiPresentTime + IIS_CHUNK_TIME) <= asfVidePresentTime)
                    {
                        audio_value = OSSemAccept(iisCmpSemEvt);
                        if (audio_value > 0) {
                            if(audio_value_max < audio_value)
                                audio_value_max = audio_value; //Lsk : for what ?
                            asfWriteVirtualAudiPayload(&iisSounBufMng[iisSounBufMngReadIdx]);
                            iisSounBufMngReadIdx    = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                            OSSemPost(iisTrgSemEvt);
                        } else {
                            break;
                        }

                        if (sysGetStorageStatus(SYS_I_STORAGE_MAIN) == SYS_V_STORAGE_NREADY)
	                    {
	                       DEBUG_ASF("Main storage not ready.\n");
	                       return 0;
	                    }
                    }
                #endif
                }

                /*******************************************
                *** seek to I fram, open a new asf file  ***
                *******************************************/

                if(EventTrigger == CAPTURE_STATE_TRIGGER)
                {
                    #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
                    if(TriggerModeFirstFileFlag == 0)
                    {
                        AV_TimeBase  = PREROLL;
                        if((pFile = asfCreateFile(0)) == 0) {
                            return 0;
                        }
                        TriggerModeFirstFileFlag = 1;
                    }
                    #elif (TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
                    if(TriggerModeFirstFileFlag == 0)
                    {
                        if((pFile = asfCreateFile(0)) == 0) {
                            return 0;
                        }
                        TriggerModeFirstFileFlag = 1;
                    }
                    else
                    {
                        /*** Reset Audio/Video time biase ***/
                        ResetPayloadPresentTime = 0; //reset video time base
                        AV_TimeBase  = PREROLL;
                        if((pFile = asfCreateFile(0)) == 0) {
                            return 0;
                        }
                    }
                    #endif
                }
            }
            if(EventTrigger == CAPTURE_STATE_TRIGGER)
            {
                /*** check record time period ***/
                CheckRecordTimeUP();

                /*** TODO ***/
                if(EventTrigger == CAPTURE_STATE_TIMEUP)
                {
                    DEBUG_ASF("\n\nFSM : CAPTURE_STATE_TIMEUP\n\n");
                }
            }
            if(EventTrigger == CAPTURE_STATE_TIMEUP)
            {
                CheckWriteFinish();

                if(EventTrigger == CAPTURE_STATE_WAIT)
                {
                    monitor_value = 0;

                    /*** Reset Audio/Video time biase ***/
                    #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
                    ResetPayloadPresentTime = 0; //reset video time base
                    if(asfVidePresentTime >= asfAudiPresentTime )
                    {
                		AV_TimeBase = asfVidePresentTime + 100; // 0.1s suspend
                    }
                    else
                    {
                        AV_TimeBase = asfAudiPresentTime + 100; // 0.1s suspend
                    }
                    /*** Close ASF file ***/
                    #elif(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
                    if(asfCloseFile(pFile) == 0) {
                        return 0;
                    }
                    #endif
                    DEBUG_ASF("\n\nFSM : CAPTURE_STATE_WAIT\n\n");
                }
            }
        }
        /**********************************
        **** Write Audio/Video Payload ****
        **********************************/
        if( (EventTrigger == CAPTURE_STATE_TRIGGER) || (EventTrigger == CAPTURE_STATE_TIMEUP) || (!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))
        {
            #ifdef ASF_AUDIO
            // ------Write audio payload------//
            if((video_value == 0) || (asfAudiPresentTime <= asfVidePresentTime))
            {
                {
                    audio_value = OSSemAccept(iisCmpSemEvt);
                    if (audio_value > 0) {
                        if(audio_value_max < audio_value)
                            audio_value_max = audio_value;

                    #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
                        if (asfWriteAudiPayload(pFile, &iisSounBufMng[iisSounBufMngReadIdx]) == 0)
                    #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
                        if (asfWriteAudiPayload_IMA_ADPCM(pFile, &iisSounBufMng[iisSounBufMngReadIdx], &PcmOffset) == 0)
                    #endif
                        {
                            DEBUG_ASF("ASF write audio payload error!!!\n");

                             /* write header object post */
                            if (asfWriteFilePropertiesObjectPost(pFile) == 0) {
                                DEBUG_ASF("ASF write file properties object post error!!!\n");
                                dcfCloseFileByIdx(pFile, 0, &tmp);
                                return 0;
                            }

                            /* write data object post */
                            if (asfWriteDataObjectPost(pFile) == 0) {
                                DEBUG_ASF("ASF write data object post error!!!\n");
                                dcfCloseFileByIdx(pFile, 0, &tmp);
                                return 0;
                            }
                            dcfCloseFileByIdx(pFile, 0, &tmp);
                            return 0;
                        }

                        iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                        OSSemPost(iisTrgSemEvt);
                    }
                }
            }
            //------ Write video payload------//
            if((audio_value == 0) || (asfAudiPresentTime >= asfVidePresentTime)) {
            #endif      // ASF_AUDIO
                {
                    video_value = OSSemAccept(VideoCmpSemEvt);

                    if (video_value > 0)
                    {
                        if(video_value_max < video_value)
                            video_value_max = video_value;


                        //Start asfTimeStatistics at create file (manual/trigger mode)
                        if(!Start_asfTimeStatistics)
                        {
                             /*** TODO
                                calculat pre-record part VideoTimeStatistics
                                if(Cal_FileTime_Start_Idx == VideoBufMngReadIdx)
                                {
                                }
                             ***/
                            Start_asfTimeStatistics = 1;
                            asfTimeStatistics = 0;
                            asfLocalTimeInSec   = g_LocalTimeInSec;
                        }

                        if (asfWriteVidePayload(pFile, &VideoBufMng[VideoBufMngReadIdx]) == 0)
                        {
                            DEBUG_ASF("ASF write video payload error!!!\n");
                             /* write header object post */
                            if (asfWriteFilePropertiesObjectPost(pFile) == 0) {
                                DEBUG_ASF("ASF write file properties object post error!!!\n");
                                dcfCloseFileByIdx(pFile, 0, &tmp);
                                return 0;
                            }

                            /* write data object post */
                            if (asfWriteDataObjectPost(pFile) == 0) {
                                DEBUG_ASF("ASF write data object post error!!!\n");
                                dcfCloseFileByIdx(pFile, 0, &tmp);
                                return 0;
                            }
                            dcfCloseFileByIdx(pFile, 0, &tmp);
                            DEBUG_ASF("Leave dcfclose!\n");
                            return 0;
                        }

                        VideoBufMngReadIdx = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                        //DEBUG_ASF("Trace: MPEG4 frame written.\n");
                        OSSemPost(VideoTrgSemEvt);
                    }

                    #if FORCE_FPS
                    if(((asfVidePresentTime - PREROLL) * FORCE_FPS) > ((asfVopCount + asfDummyVopCount) * 1000))
                    {
                        if (asfWriteDummyVidePayload(pFile) == 0)
                        {
                            DEBUG_ASF("ASF write video dummy payload error!!!\n");
                             /* write header object post */
                            if (asfWriteFilePropertiesObjectPost(pFile) == 0) {
                                DEBUG_ASF("ASF write file properties object post error!!!\n");
                                dcfCloseFileByIdx(pFile, 0, &tmp);
                                return 0;
                            }

                            /* write data object post */
                            if (asfWriteDataObjectPost(pFile) == 0) {
                                DEBUG_ASF("ASF write data object post error!!!\n");
                                dcfCloseFileByIdx(pFile, 0, &tmp);
                                return 0;
                            }
                            dcfCloseFileByIdx(pFile, 0, &tmp);
                            DEBUG_ASF("Leave dcfclose!\n");
                            return 0;
                        }
                    }
                    #endif
                }

            #ifdef  ASF_AUDIO
            }
            // Skip siu frames for release bandwidth to SD card writing
            monitor_value   = (video_value > audio_value) ? video_value : audio_value;
            #else
            monitor_value   = video_value;
            #endif

            if(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) {

                #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
                /**********************************
                **** Check File Size           ****
                **********************************/
                if(VideoBufMng[VideoBufMngReadIdx].flag == FLAG_I_VOP)
                {
                    if(asfDataPacketCount  > MaxPacketCount)
                    {
                        DEBUG_ASF("\n\n\n File Size reach limit\n\n\n");
                        asfCloseFile(pFile);
                        /*** Reset Audio/Video time biase ***/
                        ResetPayloadPresentTime = 0; //reset video time base
                        AV_TimeBase  = PREROLL;
                        //DEBUG_ASF("MPEG4 UseSem :%04d, IIS UseSem :%04d\n", VideoCmpSemEvt->OSEventCnt,iisCmpSemEvt->OSEventCnt);
                        //DEBUG_ASF("=====================================\n");
                        if((pFile = asfCreateFile(0)) == 0)
                            return 0;
                    }
                }
                #endif
            }
        }   // if((asfCaptureMode != ASF_CAPTURE_EVENT) || (EventTrigger == 2))

        if(video_value < 3)
        {
          #if TEMP_DEBUG_ENA
             gpioSetLevel(0, 12, 1);
          #endif
             OSTimeDly(1);  //Lucian: release resource to low piority task.
          #if TEMP_DEBUG_ENA
             gpioSetLevel(0, 12, 0);
          #endif
        }

        //------------------- Bitstream buffer control---------------------------------//
        /*
             Lucian: 以Audio/Video bitstream buffer 內的index剩餘個數為偵測點,若大於 ASF_DROP_FRAME_THRESHOLD
                     則為SD 寫入速度過慢,需drop frame.

        */
        if(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) //Event trigger mode
        {
            if(EventTrigger == CAPTURE_STATE_WAIT && (
                CurrentVideoSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE * 4) ||
                VideoCmpSemEvt->OSEventCnt > (VIDEO_BUF_NUM - 60) ||
                iisCmpSemEvt->OSEventCnt > (IIS_BUF_NUM - 16))) {
                do {
                    video_value = OSSemAccept(VideoCmpSemEvt);
                    if (video_value > 0) {
                        if(video_value_max < video_value)
                            video_value_max = video_value;
                        asfWriteVirtualVidePayload(&VideoBufMng[VideoBufMngReadIdx]);
                        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                        OSSemPost(VideoTrgSemEvt);
                    } else {
                        break;
                    }
                } while(VideoBufMng[VideoBufMngReadIdx].flag != FLAG_I_VOP);

                #if CDVR_LOG
                LogFileStart    = (LogFileStart + 1) % LOG_INDEX_NUM;
                #endif

                #ifdef  ASF_AUDIO
                while ((asfAudiPresentTime + IIS_CHUNK_TIME) <= asfVidePresentTime) {
                    audio_value = OSSemAccept(iisCmpSemEvt);
                    if (audio_value > 0) {
                        if(audio_value_max < audio_value)
                            audio_value_max = audio_value;
                        asfWriteVirtualAudiPayload(&iisSounBufMng[iisSounBufMngReadIdx]);
                        iisSounBufMngReadIdx    = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                        OSSemPost(iisTrgSemEvt);
                    } else {
                        break;
                    }
                }
                #endif
            }
            else if(EventTrigger == CAPTURE_STATE_WAIT)
            {
                OSTimeDly(1);  //Lucian: release resource to low piority task.
            }
        }
        else // Normal mode or overwrite mode
        {   // asfCaptureMode != ASF_CAPTURE_EVENT
        #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )

            if (monitor_value < ASF_DROP_FRAME_THRESHOLD)
            {
                siuSkipFrameRate    = 0;
            }
            else
            {
               //DEBUG_ASF("z",monitor_value);
               DEBUG_ASF("z");
            }
        #else
            if (monitor_value < ASF_DROP_FRAME_THRESHOLD)
            {       // not skip siu frame
                if(siuSkipFrameRate != 0)
                {
                    siuSkipFrameRate    = 0;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 10)) {
                if(siuSkipFrameRate != 2) {
                    siuSkipFrameRate    = 2;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 20)) {
                if(siuSkipFrameRate != 4) {
                    siuSkipFrameRate    = 4;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 30)) {
                if(siuSkipFrameRate != 6) {
                    siuSkipFrameRate    = 6;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 40)) {
                if(siuSkipFrameRate != 8) {
                    siuSkipFrameRate    = 8;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 50)) {
                if(siuSkipFrameRate != 10) {
                    siuSkipFrameRate    = 10;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 60)) {
                if(siuSkipFrameRate != 12) {
                    siuSkipFrameRate    = 12;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 70)) {
                if(siuSkipFrameRate != 16) {
                    siuSkipFrameRate    = 16;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 80)) {
                if(siuSkipFrameRate != 20) {
                    siuSkipFrameRate    = 20;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 90)) {
                if(siuSkipFrameRate != 24) {
                    siuSkipFrameRate    = 24;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (ASF_DROP_FRAME_THRESHOLD + 100)) {
                if(siuSkipFrameRate != 28) {
                    siuSkipFrameRate    = 28;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else {
                if(siuSkipFrameRate != 32) {
                    siuSkipFrameRate    = 32;
                    DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            }
        #endif

        }   // asfCaptureMode != ASF_CAPTURE_EVENT

        /********************************************************
        *** asf index table detection / SD capacity detection ***
        ********************************************************/
        if( (EventTrigger == CAPTURE_STATE_TRIGGER) || (EventTrigger == CAPTURE_STATE_TIMEUP) || (!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))
        {
            //-------------Detect ASF index table: 因 ASF index table 是暫存於DRAM有容量限制; 錄影結束後才寫入SD card. ------//
            #ifdef  ASF_AUDIO
            if((asfIndexTableIndex + VIDEO_BUF_NUM + IIS_BUF_NUM) > ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
            #else
            if((asfIndexTableIndex + VIDEO_BUF_NUM) > ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
            #endif
    		{
                DEBUG_ASF("asfIndexTableIndex =  %d, index buffer limit to %d, finish!!!\n", asfIndexTableIndex, ASF_IDX_SIMPLE_INDEX_ENTRY_MAX);
                sysCaptureVideoStart    = 0;
                sysCaptureVideoStop     = 1;
                break;
            }

            //-------------Detect Disk Full---------------------//
            // for disk full control
            if(!(asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA)) {
                asfDataSize     = sizeof(ASF_DATA_OBJECT) +
                                  asfDataPacketCount * ASF_DATA_PACKET_SIZE;
                asfIndexSize    = sizeof(ASF_SIMPLE_INDEX_OBJECT) +
                                  asfIndexTableIndex * sizeof(ASF_IDX_SIMPLE_INDEX_ENTRY);
                CurrentFileSize = asfHeaderSize + asfDataSize + asfIndexSize;


                if((((CurrentFileSize + CurrentVideoSize + CurrentAudioSize)/1024) >= (free_size - (MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE*IIS_BUF_NUM)/1024)) )
                {
                    DEBUG_ASF("Disk full!!!\n");
                    DEBUG_ASF("free_size     = %d K-bytes, CurrentFileSize = %d bytes.\n", free_size, CurrentFileSize);
                    DEBUG_ASF("asfHeaderSize = %d bytes.\n", asfHeaderSize);
                    DEBUG_ASF("asfDataSize   = %d bytes.\n", asfDataSize);
                    DEBUG_ASF("asfIndexSize  = %d bytes.\n", asfIndexSize);
                    sysCaptureVideoStart    = 0;
                    sysCaptureVideoStop     = 1;
                    MemoryFullFlag          = TRUE;
					
                    osdDrawMemFull(UI_OSD_DRAW);

                    #if Auto_Video_Test
                    memset(&Video_Auto, 0, sizeof(Video_Auto));
                    Video_Auto.VideoTest_Mode = 2;
                    #endif
                    break;
                }
            }
        }

        /************************************
        *** Change file by size or slice ***
        ************************************/
        if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
            #if (MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
            if((WantChangeFile == 0) && (asfDataPacketCount  > MaxPacketCount))
            #elif (MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
            if((WantChangeFile == 0) && (asfTimeStatistics >= asfSectionTime * 1000))
            #endif
            {
                if(asfRecFileNum != 0)
                {
                    WantChangeFile          = 1;
                    DEBUG_ASF("Time's up!!!\n");
                    DEBUG_ASF("RTCseconds == %d\n", RTCseconds);
                    DEBUG_ASF("asfRecFileNum  = %d\n", asfRecFileNum);

                    if(asfCloseFile(pFile) == 0) {
                        return 0;
                    }

                    /*** Reset Audio/Video time biase ***/
                    ResetPayloadPresentTime = 0; //reset video time base
                    AV_TimeBase  = PREROLL;

                    if((pFile = asfCreateFile(0)) == 0) {
                        return 0;
                    }

                    OS_ENTER_CRITICAL();
                    WantChangeFile  = 0;
                    LastAudio       = 0;
                    LastVideo       = 0;
                    GetLastAudio    = 0;
                    GetLastVideo    = 0;
                    OS_EXIT_CRITICAL();
                    CurrentFileSize = 0;
                }
                else
                {
                    sysCaptureVideoStart    = 0;
                    sysCaptureVideoStop     = 1;
                    DEBUG_ASF("asfRecFileNum  = %d\n", asfRecFileNum);
                    break;
                }
            }
        }
        //-------------Check Power-off: 偵測到Power-off,須結束錄影 ------------------------//
        if(pwroff == 1) {   //prepare for power off
            sysCaptureVideoStart    = 0;
            sysCaptureVideoStop     = 1;
            break;
        }

        //------- Indicator of REC (LED ON/OFF): 以LED 閃爍提示---------------//
        timetick =  IndicateRecordStatus(timetick);
    }   // while (sysCaptureVideoStop == 0)

    ExitASFCapture();

#ifdef  ASF_AUDIO
    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }
#endif

    while(VideoTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(VideoTrgSemEvt);
    }

    /* delay until mpeg4 and IIS task reach pend state */
#if(CHIP_OPTION == CHIP_PA9001D)
    OSTimeDly(2);
#else
    OSTimeDly(6);
#endif



    if((asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && ((EventTrigger == CAPTURE_STATE_WAIT) || (EventTrigger == CAPTURE_STATE_TEMP)))
	{
	    #if(TRIGGER_MODE_CLOSE_FILE_METHOD==CLOSE_FILE_BY_SIZE)
	    if(OpenFile == 1)
        {
            asfCloseFile(pFile);
        }
        #endif
        DEBUG_ASF("Event mode finish!!\n");
        DEBUG_ASF("video_value_max = %d\n", video_value_max);
#ifdef  ASF_AUDIO
        DEBUG_ASF("audio_value_max = %d\n", audio_value_max);
#endif
        return 1;
    }

#ifdef  ASF_AUDIO
    // write redundance audio payload data
    while(iisCmpSemEvt->OSEventCnt > 0) {
        Output_Sem();
        audio_value = OSSemAccept(iisCmpSemEvt);
        Output_Sem();
        if (audio_value > 0) {
            if(audio_value_max < audio_value)
                audio_value_max = audio_value;
        #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
            if (asfWriteAudiPayload(pFile, &iisSounBufMng[iisSounBufMngReadIdx]) == 0)
        #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
            if (asfWriteAudiPayload_IMA_ADPCM(pFile, &iisSounBufMng[iisSounBufMngReadIdx], &PcmOffset) == 0)
        #endif
            {
                DEBUG_ASF("ASF write audio payload error!!!\n");
                 /* write header object post */
                 if (asfWriteFilePropertiesObjectPost(pFile) == 0) {
                 DEBUG_ASF("ASF write file properties object post error!!!\n");
                 dcfCloseFileByIdx(pFile, 0, &tmp);
                 return 0;
                 }

                 /* write data object post */
                 if (asfWriteDataObjectPost(pFile) == 0) {
                 DEBUG_ASF("ASF write data object post error!!!\n");
                 dcfCloseFileByIdx(pFile, 0, &tmp);
                 return 0;
                 }
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            }
            iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
            //OSSemPost(iisTrgSemEvt);
        }
    }
#endif

    while(VideoCmpSemEvt->OSEventCnt > 0) {
        Output_Sem();
        video_value = OSSemAccept(VideoCmpSemEvt);
        Output_Sem();
        if (video_value > 0) {
            if(video_value_max < video_value)
                video_value_max = video_value;
            if (asfWriteVidePayload(pFile, &VideoBufMng[VideoBufMngReadIdx]) == 0)
            {
                DEBUG_ASF("ASF write video payload error!!!\n");
                /* write header object post */
                 if (asfWriteFilePropertiesObjectPost(pFile) == 0) {
                 DEBUG_ASF("ASF write file properties object post error!!!\n");
                 dcfCloseFileByIdx(pFile, 0, &tmp);
                 return 0;
                 }

                 /* write data object post */
                 if (asfWriteDataObjectPost(pFile) == 0) {
                 DEBUG_ASF("ASF write data object post error!!!\n");
                 dcfCloseFileByIdx(pFile, 0, &tmp);
                 return 0;
                 }
                dcfCloseFileByIdx(pFile, 0, &tmp);
                DEBUG_ASF("Leave dcfclose!\n");
                return 0;
            }
            VideoBufMngReadIdx = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
            //OSSemPost(VideoTrgSemEvt);
        }
    }


    asfCloseFile(pFile);

    DEBUG_ASF("video_value_max = %d\n", video_value_max);
#ifdef  ASF_AUDIO
    DEBUG_ASF("audio_value_max = %d\n", audio_value_max);
#endif

    DEBUG_ASF("isu_avifrmcnt     = %d\n", isu_avifrmcnt);

    return 1;
}








/*

Routine Description:

    Create ASF file.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
FS_FILE* asfCreateFile(u8 flag)
{
    FS_FILE* pFile;
  #if FILE_SYSTEM_DVF_TEST
    int time1,time2;
  #endif
  #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
  #endif
    int i;
	u8  tmp;

    DEBUG_ASF("Create ASF file\n");

    OS_ENTER_CRITICAL();
    RTCseconds                  = 0;



    OS_EXIT_CRITICAL();
    Start_asfTimeStatistics     = 0;
    if(EventTrigger == CAPTURE_STATE_WAIT)
    {
        asfTimeStatistics       = 0;
        asfLocalTimeInSec       = g_LocalTimeInSec;
    }

    asfDataPacketCount          = 0;
    asfIndexTableIndex          = 0;
    asfIndexEntryTime           = 0;
    asfVopCount                 = 0;
#if FORCE_FPS
    asfDummyVopCount            = 0;
    DummyChunkTime              = 0;
#endif
	asfDataPacketPreSendTime    = 0;    //Lsk 090519
    asfDataPacketSendTime       = 0;
    asfDataPacketFormatFlag     = 0;
	if(flag)  //Lsk 090519  : reset audio,video timestamp for pre-reord
	{
		asfVidePresentTime  = PREROLL;
	    asfAudiPresentTime  = PREROLL;
		ResetPayloadPresentTime = 1;
	}
	else
		ResetPayloadPresentTime = 0;
    asfAudiChunkCount   = 0;
    MPEG4_Error         = 0;




    //------------- for disk full control-------------//
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit

    if(!(asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA) && (free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE*IIS_BUF_NUM))/1024)) //Notice: K-Byte unit
    {
        DEBUG_ASF("2.Disk full!!!\n");
        system_busy_flag=1;
        //uiOSDIconColorByXY(OSD_ICON_WARNING ,152 , 98+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
        //osdDrawMessage(MSG_MEMORY_FULL, CENTERED, 126+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
        osdDrawMemFull(UI_OSD_DRAW);
        MemoryFullFlag = TRUE;
        OSTimeDly(15);
    #if Auto_Video_Test
        memset(&Video_Auto, 0, sizeof(Video_Auto));
        Video_Auto.VideoTest_Mode = 2;
    #endif
        return 0;
    }
    //----Storge capacity control------//
    if(asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA)
    {
        free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
        //Check filesystem capacity
        while( (free_size < DCF_OVERWRITE_THR_KBYTE) || (dcfGetTotalDirCount() > (DCF_DIRENT_MAX-2)))
        {   // Find the oldest file pointer and delete it
            if(dcfOverWriteDel()==0)
            {
                DEBUG_DCF("Over Write delete fail!!\n");
                return 0;
            }
            else
            {
                //DEBUG_ASF("Over Write delete Pass!!\n");
            }
            free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
            DEBUG_DCF2("Free Space=%d (KBytes) \n",free_size);
        }
    }
  	curr_free_space = free_size;
	curr_record_space = 0;
    /*------------ create next file------------*/

#if FILE_SYSTEM_DVF_TEST
    time1=OSTimeGet();
#endif

	#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
	{
		RTC_DATE_TIME localTime;
		
		RTC_Get_Time(&localTime);  
        if ((pFile = dcfCreateNextBackupFile(DCF_FILE_TYPE_ASF, 0, DCF_REC_TYPE_MANUAL, &localTime)) == NULL)
		{			
	        DEBUG_ASF("ASF create file error!!!\n");
	        return 0;    
        }
	}
	#else
    if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_ASF, 0)) == NULL) {
        DEBUG_ASF("ASF create file error!!!\n");
        return 0;
    }
	#endif
#if FILE_SYSTEM_DVF_TEST
    time2=OSTimeGet();
    DEBUG_ASF("\n--->dcfCreateNextFile Time=%d (x50ms)\n",time2-time1);
#endif

    /* write header object */
    if (asfWriteHeaderObject(pFile) == 0) {
		#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
		dcfBackupClose(pFile);
		#else
        dcfCloseFileByIdx(pFile, 0, &tmp);
		#endif
        return 0;
    }


    /* write data object pre */
    if (asfWriteDataObjectPre(pFile) == 0) {
		#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
		dcfBackupClose(pFile);
		#else
        dcfCloseFileByIdx(pFile, 0, &tmp);
		#endif
        return 0;
    }

    /* capture and write data */

    /* write first data packet */
    if (asfWriteDataPacketPre(pFile) == 0) {
		#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
		dcfBackupClose(pFile);
		#else
        dcfCloseFileByIdx(pFile, 0, &tmp);
		#endif
        return 0;
    }
    OpenFile = 1;
    return pFile;
}

/*

Routine Description:

    Close ASF file.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfCloseFile(FS_FILE* pFile)
{
	#if VIDEO_CONST_SIZE
    u32     CurrentFileSize;
    s32     DummySize;
	#endif

#if FILE_SYSTEM_DVF_TEST
    u32     time1,time2;
#endif
#if (CDVR_TEST_LOG)
    FS_FILE*    pfLogFile;
    u32         writesize;
    unsigned int err;
    u32 *pp;
    char        LogFileName[32];
    static u32  LogFileSize=0;
#endif
#if CDVR_LOG
    FS_FILE*    pfLogFile;
    u32         writesize;	
    //if(sysCaptureVideoStop || (EventTrigger == 3))
    if((sysCaptureVideoStop && (WantChangeFile == 0)) ||
       ((EventTrigger == CAPTURE_STATE_WAIT) && (asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))  //trigger_mode_close_by_size may be error
    {
        ChangeLogFileStartAddress();
    }
#endif
	u8  tmp;

	


    DEBUG_ASF("Close ASF file....\n");

    /* write data packet post */
    if (asfWriteDataPacketPost(pFile, asfDataPacketLeftSize) == 0) {
        DEBUG_ASF("ASF write data packet post error!!!\n");
		#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
		dcfBackupClose(pFile);
		#else
        dcfCloseFileByIdx(pFile, 0, &tmp);
		#endif
        return 0;
    }

    /* write header object post */
    if (asfWriteFilePropertiesObjectPost(pFile) == 0) {
        DEBUG_ASF("ASF write file properties object post error!!!\n");
		#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
		dcfBackupClose(pFile);
		#else
        dcfCloseFileByIdx(pFile, 0, &tmp);
		#endif
        return 0;
    }

    /* write data object post */
    if (asfWriteDataObjectPost(pFile) == 0) {
        DEBUG_ASF("ASF write data object post error!!!\n");
		#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
		dcfBackupClose(pFile);
		#else
        dcfCloseFileByIdx(pFile, 0, &tmp);
		#endif
        return 0;
    }
  #if FILE_SYSTEM_DVF_TEST
     time1=OSTimeGet();
  #endif
    // write index object //
    if (asfWriteIndexObject(pFile) == 0) {
        DEBUG_ASF("ASF write index object error!!!\n");
		#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
		dcfBackupClose(pFile);
		#else
        dcfCloseFileByIdx(pFile, 0, &tmp);
		#endif
        return 0;
    }

 #if FILE_SYSTEM_DVF_TEST
    time2=OSTimeGet();
    DEBUG_ASF("--->Write Index Time=%d (x50ms)\n",time2-time1);
 #endif
    // for NEWKEN NK2400
#if VIDEO_CONST_SIZE
    CurrentFileSize = asfHeaderSize + asfDataSize + asfIndexSize;
    DummySize       = ASF_SIZE_PER_FILE - CurrentFileSize;
    if(DummySize > 0) {
        DEBUG_ASF("Write asf dummy data begin....");
        if(asfWriteFilePadding(pFile, DummySize) == 0) {
            DEBUG_ASF("Error: Write asf dummy data fail.\n");
            dcfCloseFileByIdx(pFile, 0, &tmp);
            return 0;
        }
    } else {
        DEBUG_ASF("CurrentFileSize > %d!!!\n", ASF_SIZE_PER_FILE);
    }

    DEBUG_ASF("finish!!\n");
    DEBUG_ASF("asfHeaderSize = %d\n", asfHeaderSize);
    DEBUG_ASF("asfDataSize = %d\n", asfDataSize);
    DEBUG_ASF("asfIndexSize = %d\n", asfIndexSize);
    DEBUG_ASF("CurrentFileSize = %d\n", CurrentFileSize);
    DEBUG_ASF("DummySize = %d\n", DummySize);
#endif

    /* close file */
#if FILE_SYSTEM_DVF_TEST
    time1=OSTimeGet();
#endif

    
	#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
	if(dcfBackupClose(pFile) == 0) {
        DEBUG_ASF("Close file error!!!\n");
        return 0;
    }
	#else
    if(dcfCloseFileByIdx(pFile, 0, &tmp) == 0) {
        DEBUG_ASF("Close file error!!!\n");
        return 0;
    }
	#endif



#if (CDVR_TEST_LOG &&(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
        LogFileSize +=512;
        if(LogFileSize >LOG_FILE_MAX_SIZE)
            LogFileSize= 1024;

        DEBUG_ASF("----dcfCreatLogFile----\n");
        pp= (u32 *)LogFileBuf;
        for(i=0;i<LogFileSize/4;i++)
        {
            pp[i]= i;
        }

        if(pfLogFile = dcfCreatLogFile())
        {
            if(dcfWrite(pfLogFile,(u8 *)pp , LogFileSize,&writesize) == 0)
	        {
	            DEBUG_ASF("Log file dcfWrite error 1!!!\n");
	            if(dcfClose(pfLogFile, &tmp) == 0)
	                DEBUG_ASF("Log file dcfClose error!!!\n");
	            return 0;
	        }

            if(dcfClose(pfLogFile, &tmp) == 0)
            {
                DEBUG_DCF("Log file dcfClose error!!!\n");
                return 0;
            }
            DEBUG_ASF("success!!!\n");
        }
        else
            DEBUG_ASF("fail!!!\n");


    #if 1
        //---比對Log file ----//
        while(sysDeleteFATLinkOnRunning ==1)
            OSTimeDly(2);
        strncpy(LogFileName, (s8*)dcfGetPlaybackFileListTail()->pDirEnt->d_name, 9);
	    LogFileName[9]  = 0;
	    strcat(LogFileName, "LOG");

	    DEBUG_ASF("---->Check log file: %s:", LogFileName);

        if ((pfLogFile = dcfOpen((s8 *)LogFileName, "rb")) == 0)
    	{	/* create next file error */
    	    DEBUG_ASF("dcfOpen(%s) error!!!\n", LogFileName);
    		return 0;
    	}
        if(dcfRead(pfLogFile,(u8 *)pp, LogFileSize,&writesize) == 0)
        {
            DEBUG_ASF("Log file dcfReade error 1!!!\n");
            if(dcfClose(pfLogFile, &tmp) == 0)
                DEBUG_ASF("Log file dcfClose error!!!\n");
            return 0;
        }

        if(dcfClose(pfLogFile, &tmp) == 0)
        {
            DEBUG_DCF("Log file dcfClose error!!!\n");
            return 0;
        }

         err=0;
         for(i=0;i<LogFileSize/4;i++)
         {
             if( pp[i] !=i )
             {
                 err=1;
                 break;
             }
         }

         if(err)
            DEBUG_ASF("Error,%d\n",i);
         else
             DEBUG_ASF("PASS\n");
    #endif
#endif

#if CDVR_LOG
    dcfWriteFromBuffer  = 1;
    DEBUG_ASF("dcfCreatLogFile...");
    if(LogFileIndex[LogFileStart] <= pLogFileEnd)
    {
        DEBUG_ASF("1..");
        if(pfLogFile = dcfCreatLogFile())
        {
            //DEBUG_ASF("dcfWrite(0x%08x, 0x%08x, 0x%08x,..)\n", (u32)pfLogFile, (u32)(LogFileIndex[LogFileStart]), (u32)(pLogFileEnd - LogFileIndex[LogFileStart]));
	        if(dcfWrite(pfLogFile, LogFileIndex[LogFileStart], pLogFileEnd - LogFileIndex[LogFileStart], &writesize) == 0)
	        {
	            DEBUG_ASF("Log file dcfWrite error 1!!!\n");
	            DEBUG_ASF("pfLogFile        = 0x%08x\n", pfLogFile);
	            DEBUG_ASF("LogFileIndex[%d] = 0x%08x\n", LogFileStart, LogFileIndex[LogFileStart]);
	            DEBUG_ASF("pLogFileEnd      = 0x%08x\n", pLogFileEnd);
	            DEBUG_ASF("size             = %d\n", pLogFileEnd - LogFileIndex[LogFileStart]);
	            DEBUG_ASF("writesize        = %d\n", writesize);
	            if(dcfClose(pfLogFile, &tmp) == 0)
	                DEBUG_ASF("Log file dcfClose error!!!\n");
	            dcfWriteFromBuffer  = 0;
	            return 0;
	        }

	        if(dcfClose(pfLogFile, &tmp) == 0)
	        {
	            DEBUG_DCF("Log file dcfClose error!!!\n");
	            dcfWriteFromBuffer  = 0;
	        	return 0;
	        }
            DEBUG_ASF("success!!!\n");
        }
        else
            DEBUG_ASF("fail!!!\n");
    }
    else
    {
        DEBUG_ASF("2..");
        if(pfLogFile = dcfCreatLogFile())
        {
            //DEBUG_ASF("dcfWrite(0x%08x, 0x%08x, 0x%08x,..)\n", (u32)pfLogFile, (u32)(LogFileIndex[LogFileStart]), (u32)(pLogFileMid - LogFileIndex[LogFileStart]));
	        if(dcfWrite(pfLogFile, LogFileIndex[LogFileStart], pLogFileMid - LogFileIndex[LogFileStart], &writesize) == 0)
	        {
	            DEBUG_ASF("Log file dcfWrite error 2!!!\n");
	            DEBUG_ASF("pfLogFile        = 0x%08x\n", pfLogFile);
	            DEBUG_ASF("LogFileIndex[%d] = 0x%08x\n", LogFileStart, LogFileIndex[LogFileStart]);
	            DEBUG_ASF("pLogFileMid      = 0x%08x\n", pLogFileMid);
	            DEBUG_ASF("pLogFileEnd      = 0x%08x\n", pLogFileEnd);
	            DEBUG_ASF("szLogFile        = 0x%08x\n", szLogFile);
	            DEBUG_ASF("size             = %d\n", pLogFileMid - LogFileIndex[LogFileStart]);
	            DEBUG_ASF("writesize        = %d\n", writesize);
	            if(dcfClose(pfLogFile, &tmp) == 0)
	                DEBUG_ASF("Log file dcfClose error!!!\n");
	            dcfWriteFromBuffer  = 0;
	            return 0;
	        }

            //DEBUG_ASF("dcfWrite(0x%08x, 0x%08x, 0x%08x,..)\n", (u32)pfLogFile, (u32)LogFileBuf, (u32)(pLogFileEnd - LogFileBuf));
	        if(dcfWrite(pfLogFile, LogFileBuf, pLogFileEnd - LogFileBuf, &writesize) == 0)
	        {
	            DEBUG_ASF("Log file dcfWrite error 3!!!\n");
	            DEBUG_ASF("pfLogFile        = 0x%08x\n", pfLogFile);
	            DEBUG_ASF("LogFileBuf       = 0x%08x\n", LogFileBuf);
	            DEBUG_ASF("pLogFileEnd      = 0x%08x\n", pLogFileEnd);
	            DEBUG_ASF("size             = %d\n", pLogFileEnd - LogFileBuf);
	            DEBUG_ASF("writesize        = %d\n", writesize);
	            if(dcfClose(pfLogFile, &tmp) == 0)
	                DEBUG_ASF("Log file dcfClose error!!!\n");
	            dcfWriteFromBuffer  = 0;
	            return 0;
	        }

	        if(dcfClose(pfLogFile, &tmp) == 0)
	        {
	            DEBUG_DCF("Log file dcfClose error!!!\n");
	            dcfWriteFromBuffer  = 0;
	        	return 0;
	        }
            DEBUG_ASF("success!!!\n");
        }
        else
            DEBUG_ASF("fail!!!\n");
    }

#if CDVR_TEST_LOG_FILE   // 撿查連續錄影時log檔是否中間資料遺失導致尾部資料增加, Peter
{
    FS_FILE*    pFile;
    char        LogFileName[32];
    u32         readsize;
    s32         position;
    char        Data[2];

	strncpy(LogFileName, (s8*)dcfGetPlaybackFileListTail()->pDirEnt->d_name, 9);
	LogFileName[9]  = 0;
	strcat(LogFileName, "LOG");

	DEBUG_ASF("Check log file: %s...", LogFileName);

	if ((pFile = dcfOpen((s8 *)LogFileName, "rb")) == 0)
	{	/* create next file error */
	    DEBUG_ASF("dcfOpen(%s) error!!!\n", LogFileName);
	    dcfWriteFromBuffer  = 0;
		return 0;
	}
	Data[0] = Data[1] = 0;
	if(dcfSeek(pFile, pFile->size - 2, FS_SEEK_SET) == 0)
	    DEBUG_ASF("dcfSeek(%s, %d, 0x%08x) error!!!\n", LogFileName, pFile->size - 2, pFile->size - 2);
	if(dcfRead(pFile, (u8*)Data, 2, &readsize) == 0)
	    DEBUG_ASF("dcfRead(%s) error!!!\n", LogFileName);
	dcfClose(pFile, &tmp);
	if((Data[0] != 0x0d) || (Data[1] != 0x0a)) {
	    DEBUG_ASF("Log file (%s, 0x%02x, 0x%02x) read Error!!!!!!!!!\n", LogFileName, Data[0], Data[1]);
	    DEBUG_ASF("Log memory (0x%02x, 0x%02x)\n", *(pLogFileEnd - 2), *(pLogFileEnd - 1));

        DEBUG_ASF("Dump debug.log...");
        if(LogFileIndex[LogFileStart] <= pLogFileEnd)
        {
            DEBUG_ASF("1..");
            if(pfLogFile = dcfOpen("debug.log", "w-"))
            {
                DEBUG_ASF("dcfWrite(0x%08x, 0x%08x, 0x%08x,..)\n", (u32)pfLogFile, (u32)(LogFileIndex[LogFileStart]), (u32)(pLogFileEnd - LogFileIndex[LogFileStart]));
	            if(dcfWrite(pfLogFile, LogFileIndex[LogFileStart], pLogFileEnd - LogFileIndex[LogFileStart], &writesize) == 0)
	            {
	                DEBUG_ASF("Log file dcfWrite error 1!!!\n");
	                DEBUG_ASF("pfLogFile        = 0x%08x\n", pfLogFile);
	                DEBUG_ASF("LogFileIndex[%d] = 0x%08x\n", LogFileStart, LogFileIndex[LogFileStart]);
	                DEBUG_ASF("pLogFileEnd      = 0x%08x\n", pLogFileEnd);
	                DEBUG_ASF("size             = %d\n", pLogFileEnd - LogFileIndex[LogFileStart]);
	                DEBUG_ASF("writesize        = %d\n", writesize);
	                if(dcfClose(pfLogFile, &tmp) == 0)
	                    DEBUG_ASF("Log file dcfClose error!!!\n");
	                dcfWriteFromBuffer  = 0;
	                return 0;
	            }

	            if(dcfClose(pfLogFile, &tmp) == 0)
	            {
	                DEBUG_DCF("Log file dcfClose error!!!\n");
	                dcfWriteFromBuffer  = 0;
	            	return 0;
	            }
                DEBUG_ASF("success!!!\n");
            }
            else
                DEBUG_ASF("fail!!!\n");
        }
        else
        {
            DEBUG_ASF("2..");
            if(pfLogFile = dcfOpen("debug.log", "w-"))
            {
                DEBUG_ASF("dcfWrite(0x%08x, 0x%08x, 0x%08x,..)\n", (u32)pfLogFile, (u32)(LogFileIndex[LogFileStart]), (u32)(pLogFileMid - LogFileIndex[LogFileStart]));
	            if(dcfWrite(pfLogFile, LogFileIndex[LogFileStart], pLogFileMid - LogFileIndex[LogFileStart], &writesize) == 0)
	            {
	                DEBUG_ASF("Log file dcfWrite error 2!!!\n");
	                DEBUG_ASF("pfLogFile        = 0x%08x\n", pfLogFile);
	                DEBUG_ASF("LogFileIndex[%d] = 0x%08x\n", LogFileStart, LogFileIndex[LogFileStart]);
	                DEBUG_ASF("pLogFileMid      = 0x%08x\n", pLogFileMid);
	                DEBUG_ASF("pLogFileEnd      = 0x%08x\n", pLogFileEnd);
	                DEBUG_ASF("szLogFile        = 0x%08x\n", szLogFile);
	                DEBUG_ASF("size             = %d\n", pLogFileMid - LogFileIndex[LogFileStart]);
	                DEBUG_ASF("writesize        = %d\n", writesize);
	                if(dcfClose(pfLogFile, &tmp) == 0)
	                    DEBUG_ASF("Log file dcfClose error!!!\n");
	                dcfWriteFromBuffer  = 0;
	                return 0;
	            }

                DEBUG_ASF("dcfWrite(0x%08x, 0x%08x, 0x%08x,..)\n", (u32)pfLogFile, (u32)LogFileBuf, (u32)(pLogFileEnd - LogFileBuf));
	            if(dcfWrite(pfLogFile, LogFileBuf, pLogFileEnd - LogFileBuf, &writesize) == 0)
	            {
	                DEBUG_ASF("Log file dcfWrite error 3!!!\n");
	                DEBUG_ASF("pfLogFile        = 0x%08x\n", pfLogFile);
	                DEBUG_ASF("LogFileBuf       = 0x%08x\n", LogFileBuf);
	                DEBUG_ASF("pLogFileEnd      = 0x%08x\n", pLogFileEnd);
	                DEBUG_ASF("size             = %d\n", pLogFileEnd - LogFileBuf);
	                DEBUG_ASF("writesize        = %d\n", writesize);
	                if(dcfClose(pfLogFile, &tmp) == 0)
	                    DEBUG_ASF("Log file dcfClose error!!!\n");
	                dcfWriteFromBuffer  = 0;
	                return 0;
	            }

	            if(dcfClose(pfLogFile, &tmp) == 0)
	            {
	                DEBUG_DCF("Log file dcfClose error!!!\n");
	                dcfWriteFromBuffer  = 0;
	            	return 0;
	            }
                DEBUG_ASF("success!!!\n");
            }
            else
                DEBUG_ASF("fail!!!\n");
        }

        // 出錯就進無窮迴圈
	    while(1) {
	        OSTimeDly(1);
	    }
	}
	DEBUG_ASF("pass\n");
}
#endif  // 撿查連續錄影時log檔是否中間資料遺失導致尾部資料增加
    dcfWriteFromBuffer  = 0;
    LogFileStart        = LogFileNextStart;
#endif  // #if CDVR_LOG

#if FILE_SYSTEM_DVF_TEST
    time2=OSTimeGet();
    DEBUG_ASF("--->Close ASFFile Time=%d\n",time2-time1);
#endif
    DEBUG_ASF("asfDataPacketCount = %d\n", asfDataPacketCount);
    DEBUG_ASF("asfIndexTableIndex = %d\n", asfIndexTableIndex);
    DEBUG_ASF("asfIndexEntryTime  = %d\n", asfIndexEntryTime);
    DEBUG_ASF("asfVidePresentTime = %d\n", asfVidePresentTime);
    DEBUG_ASF("asfAudiPresentTime = %d\n", asfAudiPresentTime);
    DEBUG_ASF("asfVopCount        = %d\n", asfVopCount);
#if FORCE_FPS
    DEBUG_ASF("asfDummyVopCount   = %d\n", asfDummyVopCount);
#endif
    DEBUG_ASF("asfAudiChunkCount  = %d\n", asfAudiChunkCount);

    DEBUG_ASF("finish\n");
#if (FILE_REPLACE_NAME == 1) /*G-Sensor Triger時更改檔名 */
{
    u8 NewName[12];
    u8 OldName[12];
    u8 status;
    memset((s8*)NewName, 0, sizeof(NewName));
    memset((s8*)OldName, 0, sizeof(OldName));
    if (GSensorEvent == 1)
    {
        dcfGetCurFileName(OldName);
        dcfGetCurFileName(NewName);
        NewName[6]  ='G';
    	NewName[7]  ='S';

        DEBUG_ASF("NewName = %s\r\n", NewName);
    	status=dcfRename(NewName, OldName);
        if(status > 0)
    	    DEBUG_ASF("Rename Success!\n");
    	else
    	    DEBUG_ASF("Rename Fail!\n");
        GSensorEvent = 0;
    }
}
#endif
    //----Storge capacity control------//
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit
  
    if(asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA)
    {
        free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
        //Check filesystem capacity
        DEBUG_DCF2("Free Space=%d (KBytes) \n",free_size);
        while(free_size < DCF_OVERWRITE_THR_KBYTE)
        {   // Find the oldest file pointer and delete it
            if(dcfOverWriteDel()==0)
            {
                DEBUG_DCF("Over Write delete fail!!\n");
                return 0;
            }
            else
            {
                //DEBUG_ASF("Over Write delete Pass!!\n");
            }
            free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
            DEBUG_DCF2("Free Space=%d (KBytes) \n",free_size);
        }
    }

    OpenFile = 0;
    return 1;
}
#endif
/*

Routine Description:

    Set video resolution.

Arguments:

    width - Video width.
    height - Video height.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 asfSetVideoResolution(u16 width, u16 height)
{
    int i;

    DEBUG_ASF("asfSetVideoResolution(%d, %d)\n", width, height);
    #if (VIDEO_CODEC_OPTION == H264_CODEC)
    asfVopWidth     = width;    /* cytsai: 0418 */
    asfVopHeight    = height;
    if(EncodeLineStripe)
    {
        asfVopWidth  = width  = 320;
        asfVopHeight = height = 240;
    }
    if(EncodeDownSample)
    {
        asfVopWidth  = (width >> EncodeDownSample);    /* cytsai: 0418 */
        asfVopHeight = (height >> EncodeDownSample);    /* cytsai: 0418 */
    }
    mpeg4Width      = (u32) asfVopWidth;
    mpeg4Height     = (u32) asfVopHeight;
    H264Enc_SetResolution(&H264Enc_cfg, asfVopWidth, asfVopHeight);
    //(VideoCodecSetResolution)(asfVopWidth, asfVopHeight);
    #else
    asfVopWidth     = width;    /* cytsai: 0418 */
    asfVopHeight    = height;
    mpeg4SetVideoResolution(width, height);
    mpeg4ConfigQualityFrameRate(MPEG_BITRATE_LEVEL_100);
    #endif

#if MULTI_CHANNEL_VIDEO_REC
    // Peter: 暫時每個channel都用同一個設定,要各channel獨立時再把這段code拿掉
    for(i = 0; i < MULTI_CHANNEL_MAX; i++)
    {
        VideoClipParameter[i].asfVopWidth   = asfVopWidth;
        VideoClipParameter[i].asfVopHeight  = asfVopHeight;
    }
#endif

    return  1;
}

/* BJ: 0718 S*/
u8 matchGuid(u8 *buf)
{
    u32 Guid1,Guid2,Guid3,Guid4;
    //Guid = (u32)(*((u32 *)buf));
    Guid1 = *(buf+0)  + ((*(buf+1))<<8)  + ((*(buf+2))<<16)  + ((*(buf+3))<<24);
    Guid2 = *(buf+4)  + ((*(buf+5))<<8)  + ((*(buf+6))<<16)  + ((*(buf+7))<<24);
    Guid3 = *(buf+8)  + ((*(buf+9))<<8)  + ((*(buf+10))<<16) + ((*(buf+11))<<24);
    Guid4 = *(buf+12) + ((*(buf+13))<<8) + ((*(buf+14))<<16) + ((*(buf+15))<<24);
    switch(Guid1)
    {
        case 0x75b22630:
            //if( (*((u32 *)(buf+4)) == 0x11CF668E) && (*((u32 *)(buf+8)) == 0xAA00D9A6) && (*((u32 *)(buf+12)) == 0x6CCE6200) ){
            if( (Guid2 == 0x11CF668E) && (Guid3 == 0xAA00D9A6) && (Guid4 == 0x6CCE6200) ){
                return ASF_Header_Object_Guid;
            }
            else
                return NoMatch_Guid;
        case 0x75b22636:
            //if( (*((u32 *)(buf+4)) == 0x11CF668E) && (*((u32 *)(buf+8)) == 0xAA00D9A6) && (*((u32 *)(buf+12)) == 0x6CCE6200) ){
            if( (Guid2 == 0x11CF668E) && (Guid3 == 0xAA00D9A6) && (Guid4 == 0x6CCE6200) ){
                return ASF_Data_Object_Guid;
            }
            else
                return NoMatch_Guid;
        case 0x33000890:
            //if( (*((u32 *)(buf+4)) == 0x11CFE5B1) && (*((u32 *)(buf+8)) == 0xA000F489) && (*((u32 *)(buf+12)) == 0xCB4903C9) ){
            if( (Guid2 == 0x11CFE5B1) && (Guid3 == 0xA000F489) && (Guid4 == 0xCB4903C9) ){
                return ASF_Simple_Index_Object_Guid;
            }
            else
                return NoMatch_Guid;
        default:
            return NoMatch_Guid;
    }
}

u8 matchHeaderObjectGuid(u8 *buf)
{
    u32 Guid1,Guid2,Guid3,Guid4;
    //Guid = (u32)(*((u32 *)buf));
    Guid1 = *(buf+0)  + ((*(buf+1))<<8)  + ((*(buf+2))<<16)  + ((*(buf+3))<<24);
    Guid2 = *(buf+4)  + ((*(buf+5))<<8)  + ((*(buf+6))<<16)  + ((*(buf+7))<<24);
    Guid3 = *(buf+8)  + ((*(buf+9))<<8)  + ((*(buf+10))<<16) + ((*(buf+11))<<24);
    Guid4 = *(buf+12) + ((*(buf+13))<<8) + ((*(buf+14))<<16) + ((*(buf+15))<<24);
    switch(Guid1)
    {
        case 0x8CABDCA1:
            //if( (*((u32 *)(buf+4)) == 0x11CFA947) && (*((u32 *)(buf+8)) == 0x00C08EE4) && (*((u32 *)(buf+12)) == 0x6553200C)){
            //if( (*((u32 *)(buf+4)) == 0x11CFA947) && (*((u32 *)(buf+8)) == 0xC000E48E) && (*((u32 *)(buf+12)) == 0x6553200C)){
            if( (Guid2 == 0x11CFA947) && (Guid3 == 0xC000E48E) && (Guid4 == 0x6553200C)){
                return ASF_File_Properties_Object_GUID;
            }
            else
                return NoMatch_Guid;
        case 0xB7DC0791:
            //if( (*((u32 *)(buf+4)) == 0x11CFA9B7) && (*((u32 *)(buf+8)) == 0x00C08EE6) && (*((u32 *)(buf+12)) == 0x6553200C)){
            if( (Guid2 == 0x11CFA9B7) && (Guid3 == 0xC000E68E) && (Guid4 == 0x6553200C)){
                return ASF_Stream_Properties_Object_GUID;
            }
            else
                return NoMatch_Guid;
        case 0x5FBF03B5:
            //if( (*((u32 *)(buf+4)) == 0x11CFA92E) && (*((u32 *)(buf+8)) == 0x00C08EE3) && (*((u32 *)(buf+12)) == 0x6553200C)){
            if( (Guid2 == 0x11CFA92E) && (Guid3 == 0xC000E38E) && (Guid4 == 0x6553200C)){
                return ASF_Header_Extension_Object_GUID;
            }
            else
                return NoMatch_Guid;
        case 0x86D15240:
            //if( (*((u32 *)(buf+4)) == 0x11D0311D) && (*((u32 *)(buf+8)) == 0x00A0A3A4) && (*((u32 *)(buf+12)) == 0xF648039C)){
            if( (Guid2 == 0x11D0311D) && (Guid3 == 0xA000A4A3) && (Guid4 == 0xF64803C9)){
                return ASF_Codec_List_Object_GUID;
            }
            else
                return NoMatch_Guid;
        case 0x75B22633:
            //if( (*((u32 *)(buf+4)) == 0x11CF668E) && (*((u32 *)(buf+8)) == 0x00AAA6D9) && (*((u32 *)(buf+12)) == 0x6CCE6200)){
            if( (Guid2 == 0x11CF668E) && (Guid3 == 0xAA00D9A6) && (Guid4 == 0x6CCE6200)){
                return ASF_Content_Description_Object_GUID;
            }
            else
                return NoMatch_Guid;
        //case 0x74D40618:
        case 0x1806D474:
            //if( (*((u32 *)(buf+4)) == 0x4509CADF) && (*((u32 *)(buf+8)) == 0x9AABA4BA) && (*((u32 *)(buf+12)) == 0xE8AA96CB)){
            if( (Guid2 == 0x4509CADF) && (Guid3 == 0xAB9ABAA4) && (Guid4 == 0xE8AA96CB)){
                return ASF_Padding_Object_GUID;
            }
            else
                return NoMatch_Guid;
        default:
            return NoMatch_Guid;
    }
}

u8 matchStreamTypeGuid(u8 * buf)
{
    u32 Guid1,Guid2,Guid3,Guid4;
    //Guid = (u32)(*((u32 *)buf));
    Guid1 = *(buf+0)  + ((*(buf+1))<<8)  + ((*(buf+2))<<16)  + ((*(buf+3))<<24);
    Guid2 = *(buf+4)  + ((*(buf+5))<<8)  + ((*(buf+6))<<16)  + ((*(buf+7))<<24);
    Guid3 = *(buf+8)  + ((*(buf+9))<<8)  + ((*(buf+10))<<16) + ((*(buf+11))<<24);
    Guid4 = *(buf+12) + ((*(buf+13))<<8) + ((*(buf+14))<<16) + ((*(buf+15))<<24);
    switch(Guid1)
    {
            case 0xF8699E40:
                //if( (*((u32 *)(buf+4)) == 0x11CF5B4D) && (*((u32 *)(buf+8)) == 0x0080A8FD) && (*((u32 *)(buf+12)) == 0x2B445C5F)){
                if( (Guid2 == 0x11CF5B4D) && (Guid3 == 0x8000FDA8) && (Guid4 == 0x2B445C5F)){
                    return ASF_Audio_Media_GUID;
                }
                else
                    return NoMatch_Guid;

            case 0xBC19EFC0:
                //if( (*((u32 *)(buf+4)) == 0x11CF5B4D) && (*((u32 *)(buf+8)) == 0x0080A8FD) && (*((u32 *)(buf+12)) == 0x2B445C5F)){
                if( (Guid2 == 0x11CF5B4D) && (Guid3 == 0x8000FDA8) && (Guid4 == 0x2B445C5F)){
                    return ASF_Video_Media_GUID;
                }
                else
                    return NoMatch_Guid;
            default:
                break;

    }
    return NoMatch_Guid;
}
/* BJ: 0718 E*/

/*

Routine Description:

    To solve video tremble when pause video playback, use top filed of next frame to generate Pause frame.

Arguments:
	SrcBuf + (flag - 1) = Dstbuf


Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfPausePlayback(u8 *Srcbuf , u8 *Dstbuf , u8 flag)
{
#if(CHIP_OPTION < CHIP_A1013A)
	u8 err;
	#if DINAMICALLY_POWER_MANAGEMENT
    sysISU_enable();
    #endif

    isuGenPauseFrame(Srcbuf, Dstbuf, Mp4Dec_opt.Width, Mp4Dec_opt.Height);

  	OSSemPend(isuSemEvt, 10 ,&err);
    #if DINAMICALLY_POWER_MANAGEMENT
    sysISU_disable();
    #endif
    if (err != OS_NO_ERR) {
    	DEBUG_MP4("Error: isuTVout_Pause isuSemEvt(playback mode) is %d.\n", err);
		return 0;
    }
#endif

	//PAUSE frame sotre in MainVideodisplaybuf_idx,  we must update MainVideodisplaybuf_idx+1. then timerIntHandler can correct display PAUSE frame
	//DEBUG_ASF("(%ld,",MainVideodisplaybuf_idx);
	MainVideodisplaybuf_idx += flag;
	//DEBUG_ASF("%ld)\n",MainVideodisplaybuf_idx);
	IsuIndex--; //clear
	
    return 1;
}

/*

Routine Description:

    Repair file when shut down abnormally.

Arguments:

    file_name - file name.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 asfRepairFile(signed char* file_name)
{
    FS_FILE*    pInFile;

    ASF_FILE_PROPERTIES_OBJECT asfFilePropertiesObject;

    u8          flag2, PayloadFlag=0, PayloadNum, StreamNum, KeyFrame;
    u32         ReadSize, size, PacketUsedSize, PacketLength, PaddingLength, Offset2Media, PayloadLength, MediaObjectSize,asfVideoIndex, asfAudioIndex;
    s64         asfPresentTime;
    u8          *buf;

    u8          Error=0;
    u8          Mul_Payload = 0;
    u64         UsedSize;

    u32 offset=0;
	u8  tmp;
	
    asfDataPacketCount = 0;
    asfIndexTable               = (ASF_IDX_SIMPLE_INDEX_ENTRY*)(mpeg4IndexBuf+sizeof(ASF_SIMPLE_INDEX_OBJECT));
    asfIndexTableIndex = 0;
    asfIndexEntryTime  = 0;

    /* open file and scan file*/
    if ((pInFile = dcfOpen(file_name, "r")) == NULL) {
        DEBUG_ASF("ASF open file error!!!\n");
        Error = 1;
        goto ExitAsfRepairFile;
    }

    /*** skip first packet - Header object ***/
    if(dcfRead(pInFile, tempbuf, ASF_HEADER_SIZE_CEIL, &size) == 0) {
        DEBUG_ASF("ASF read file error!!!\n");
        Error = 1;
        goto ExitAsfRepairFile;
    }

    ReadSize = (u32)ASF_HEADER_SIZE_CEIL;
    asfFilePropertiesObject.maximum_data_packet_size = asfFilePropertiesObject.minimum_data_packet_size = ASF_DATA_PACKET_SIZE; //asfReadPacketHeader


    while(ReadSize < pInFile->size)
    {
        if(dcfRead(pInFile, tempbuf, ASF_DATA_PACKET_SIZE , &size) != 1) {
            DEBUG_ASF("ASF read file error!!!\n");
            break;
        }

        offset = dcfTell(pInFile); //scan address

        if(tempbuf[0]!=0x82) //error_correction_flags = 0x82 : check if a correct packet
        {
           offset -= ASF_DATA_PACKET_SIZE;
           DEBUG_ASF("This Packet is not a correct Packet!!!\n");
           break;
        }

        asfDataPacketCount +=1;
        ReadSize += (u32)ASF_DATA_PACKET_SIZE;

        buf = tempbuf;
        buf =  asfReadPacketHeader(buf,
                                   &asfFilePropertiesObject,
                                   &PacketUsedSize,
                                   &flag2,
                                   &Mul_Payload,
                                   &PacketLength,
                                   &PaddingLength,//Lsk 090304
                                   &UsedSize);
        if(buf == 0)
        {
            break;
        }

        if(Mul_Payload)
        {
            PayloadFlag = *((u8 *)buf);
            PayloadNum = (PayloadFlag & 0x3F);
            buf += 1; // Payload Flag
            UsedSize.lo += 1; // Payload Flag
        }
        else
           	PayloadNum = 1;

        while(PayloadNum != 0)
        {
            PayloadNum--;

            buf = asfReadPayloadHeader(buf,
									  &StreamNum,
                                      &KeyFrame,
                                      flag2,
                                      0,
                                      0,
                                      PayloadFlag,
                                      &Offset2Media,
                                      &PayloadLength,
                                      &MediaObjectSize,   //Lsk 090622
                                      &asfPresentTime,
                                      &asfVideoIndex,
                                      &asfAudioIndex,
                                      &UsedSize,
                                      Mul_Payload,    //Lsk 090304
                                      PacketLength,
                                      PaddingLength);
            if(StreamNum == 1) //video stream
            {
                if((asfPresentTime + PREROLL) < asfVidePresentTime)
                {
                   offset -= ASF_DATA_PACKET_SIZE;
                   DEBUG_ASF("This payload is not a correct payload!!!\n");
                   break;
                }
                asfVidePresentTime = (asfPresentTime + PREROLL);
                // Generate Index table
                if(KeyFrame)
                {

                    if(Offset2Media==0)
                    {
                        asfIndexEntryPacketNumber = asfDataPacketCount -1;
                        asfIndexEntryPacketCount  = 1;
                    }
                    else
                        asfIndexEntryPacketCount++;
                }
                /* set index entry on each 1 sec interval */
                while ((asfVidePresentTime >= asfIndexEntryTime) && (asfIndexTableIndex < ASF_IDX_SIMPLE_INDEX_ENTRY_MAX))    //Lsk 090309 preroll index object
                {
                    asfIndexTable[asfIndexTableIndex].packet_number     = asfIndexEntryPacketNumber;
                    asfIndexTable[asfIndexTableIndex++].packet_count    = asfIndexEntryPacketCount;
                    asfIndexEntryTime                                   += 1000; /* next index after 1 s = 1000 ms */
                }
                if (asfIndexTableIndex >= ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
                {
                    DEBUG_ASF("Trace: Video time (%d sec) reaches limit.\n", asfIndexTableIndex);
                    goto ExitAsfRepairFile;
                }
            }

            buf += PayloadLength;
            UsedSize.lo += PayloadLength;
        }

        sysDeadLockMonitor_Reset();
    }

ExitAsfRepairFile:

    if(Error)
    {
        if(dcfClose(pInFile, &tmp) == 0) {
            DEBUG_ASF("dcfClose() error!!!\n");
        }
        return 0;
    }
    else
    {


        if(dcfClose(pInFile, &tmp) == 0) {
            DEBUG_ASF("Close file error!!!\n");
            return 0;
        }

        if(asfDataPacketCount)
        {
            /*** open file, correct header object, add index table ***/
            RepairASF = 1;
            if ((pInFile = dcfOpen(file_name, "a")) == NULL) {
                DEBUG_ASF("ASF open file error!!!\n");
                RepairASF = 0;
                return 0;
            }
            asfHeaderSize = ASF_DATA_PACKET_SIZE - sizeof(ASF_DATA_OBJECT) ;
            /* write header object post */
            if (asfWriteFilePropertiesObjectPost(pInFile) == 0) {
                DEBUG_ASF("ASF write file properties object post error!!!\n");
                dcfClose(pInFile, &tmp);
                RepairASF = 0;
                return 0;
            }
            /* write data object post */
            if (asfWriteDataObjectPost(pInFile) == 0) {
                DEBUG_ASF("ASF write data object post error!!!\n");
                dcfClose(pInFile, &tmp);
                RepairASF = 0;
                return 0;
            }

            dcfSeek(pInFile, offset, FS_SEEK_SET);
            DEBUG_ASF("ASF write index object !!\n");
            // write index object //
            if (asfWriteIndexObject(pInFile) == 0) {
                DEBUG_ASF("ASF write index object error!!!\n");
                dcfClose(pInFile, &tmp);
                RepairASF = 0;
                return 0;
            }
            if(dcfClose(pInFile, &tmp) == 0) {
                DEBUG_ASF("Close file error!!!\n");
                RepairASF = 0;
                return 0;
            }
            RepairASF = 0;
            DEBUG_ASF("Repair %s successful finish\n",file_name);
            return 1;
        }
    }
	return 1;
}

/*

Routine Description:

    Switch source, between HDMI and panel

Arguments:

    u8 source - 
        0: switch to HDMI
        1: switch to panel

Return Value:

    0 - Failure.
    1 - Success.
*/

u32 asfSwitchSource(u8 source){
/* TODO: check source if it is the same source as current
    if (source == current_source)
        return 0;
*/
#if UI_SYNCHRONOUS_DUAL_OUTPUT
  #if (VIDEO_CODEC_OPTION == H264_CODEC)
    switch(source)
    {
        case SWITCH_TO_HDMI:
            if(isCap1920x1080I() == 1)
                DecodeDownSample = 0;
            else
                DecodeDownSample = 1;
            DEBUG_ASF("[ASF] Switch to HDMI %d\n",DecodeDownSample);
        break;
        
        case SWITCH_TO_PANEL:
            DecodeDownSample = 1;
            DEBUG_ASF("[ASF] Switch to PANEL\n");         
        break;
    }
  #endif
#else
  #if (VIDEO_CODEC_OPTION == H264_CODEC)
    switch(source)
    {
        case SWITCH_TO_HDMI:
            if(isCap1920x1080I() == 1)
                DecodeDownSample = 0;
            else
                DecodeDownSample = 1;
            DEBUG_ASF("[ASF] Switch to HDMI\n");
        break;
        
        case SWITCH_TO_PANEL:
            DecodeDownSample = 1;
            DEBUG_ASF("[ASF] Switch to PANEL\n");         
        break;
    }
  #endif
#endif
    if(Iframe_flag == 0)
    {
        ResetPlayback = 1;
        asfSeekPrevIFrame = 1;
    }

    return 1;
}

u32 GetVideoResolutionsetTV(s8* pFileName)
{
	FS_FILE*	pFile;
	u32	size;
	u8  tmp;

	if ((pFile = dcfOpen(pFileName, "r")) == NULL) {
        DEBUG_ASF("ASF open file error!!!\n");
        return 0;
    }

	if(dcfRead(pFile, tempbuf, 224 , &size) == 0) {
    	DEBUG_ASF("ASF read file error!!!\n");
        return 0;
    }

	if(dcfClose(pFile, &tmp) == 0) {
        DEBUG_ASF("dcfClose() error!!!\n");
    }
	
	gPlaybackWidth = (*(tempbuf+213)<<8)+ *(tempbuf+212);
	gPlaybackHeight = (*(tempbuf+217)<<8)+ *(tempbuf+216);
	asfVopWidth  = gPlaybackWidth;
	asfVopHeight = gPlaybackHeight;
	
	if(sysTVOutOnFlag)
	{
		asfSwitchSource(SYS_OUTMODE_TV);
		sysTVswitchResolutionbyImagesize();
		if(asfVopWidth == 640)
			iduPlaybackMode(640,352,640);
		else if (asfVopWidth == 1280)
			iduPlaybackMode(1280,720,1280);
		else if (asfVopWidth == 1920)
		{
			if(isCap1920x1080I() == 1)
				iduPlaybackMode(1920,1080,1920);
			else
			iduPlaybackMode(960,720,1280);
		}

		if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) ||\
			(TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) ||\
			(TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
		   tvTVE_INTC	=TV_INTC_FRMEND__ENA;
		else
		   tvTVE_INTC	=TV_INTC_BOTFDSTART_ENA;
	}

	//DEBUG_ASF("%x %x %x %x\n",*(tempbuf+213),*(tempbuf+212),*(tempbuf+216),*(tempbuf+217));
	//DEBUG_ASF("W %d,H %d\n",gPlaybackWidth,gPlaybackHeight);
    
	return 1;
}

/*

Routine Description:

    Read file.

Arguments:

    u32PacketOffset - Packet offset for playback start position.

Return Value:

    0 - Failure.
    1 - Success.

*/
/* Peter 070104 */
s32 asfReadFile(u32 u32PacketOffset)  //Lsk 090415 : AV Sync rule : audio follow video
{
    FS_FILE*                pFile;
    /* BJ: 0718 S*/
    u64                     UsedSize,PacketCount;
    u8                      flag2;
    ASF_HEADER_OBJECT       asfHeaderObject;
    ASF_DATA_OBJECT         asfDataObject;
    ASF_SIMPLE_INDEX_OBJECT asfSimpleIndexObject;

    ASF_FILE_PROPERTIES_OBJECT asfFilePropertiesObject;
    /*Payload Parsing Info*/
    u8  Mul_Payload = 0, PayloadFlag=0;
    u32 PacketLength, PaddingLength, SendTime, Offset2Media;//, MediaObjectNumber;
    u16 Duration;

    u32 PayloadLength, PacketUsedSize, VideoStreamLength, size;
    s64 asfPresentTime;
	u32 MediaObjectSize;  ////Lsk 090622
    u8  StreamNum, KeyFrame, VideoStreamNum, AudioStreamNum, PayloadNum,  Restart1, Restart2;
    u8  *buf;

    /* Peter 070104 */
    u32 asfVideoIndex, asfAudioIndex, i, Offset;
    u8  err;    
    ASF_AUDI_STREAM_PROPERTIES_OBJECT asfAudiStreamPropertiesObject;
#ifdef  ASF_AUDIO
    u8  *AudioStreamBuf;
    u32 AudioStreamLength;
    u8  StartAudio  = 1;//VopUsedByteSize,
#endif

    u8  StartFrame  = 1;//VopUsedByteSize,
    u8  *VideoStreamBuf;
    u8  Error       = 0;

	//u8  pre_playback_speed = 5;
	s16 CurrrentIndex = 0;  ////Lsk 090414 : add index range
	u16 PlayIndex;
	u8  KeyFramePacketCount=0;
	u8 	GetFirstAudioPayload = 0;  //Lsk 090417 : AVSync timebase
    u8  GetFirstVideoPayload = 1;  //Lsk 090417 : Fastward,Backward timebase
	u8  tmp;
    u8  stuck_cnt = 0;
    u32  asfCheckHeaderSize;
	
	StartPlayBack = 0;
	ResetPlayback = 0;
	Video_timebase = 0;
    #if( (SW_APPLICATION_OPTION != MR9300_RFDVR_RX1RX2) && (SW_APPLICATION_OPTION != MR9300_NETBOX_RX1RX2) )
    VideoNextPresentTime  = 0;
    #endif
    asfSeekPrevIFrame     = 0;
	PacketCount.lo  = 0;

    /* avoid compile warning */
    asfSimpleIndexObject    = asfSimpleIndexObject;
    PaddingLength           = PaddingLength;
    SendTime                = SendTime;
    Duration                = Duration;
    VideoStreamNum          = VideoStreamNum;
    AudioStreamNum          = AudioStreamNum;

    PacketCount.lo          = 0;
//    sysPlaybackThumbnail = READFILE_THUMBNAIL_PREVIEW; //ted testing

	if(u32PacketOffset)
	{
        Restart1    = 1;    // video restart flag
        Restart2    = 1;    // audio restart flag
        CurrrentIndex = u32PacketOffset +3 +1; // +1 for the 0.xxx sec, ex 12.5 sec, PacketOffset will be 12, and we should go for 13
        ResetPlayback = 1;
		asfIndexTableRead = 0;
	}
	else
	{
        Restart1    = 0;
        Restart2    = 0;
		if(sysPlaybackThumbnail == READFILE_STOP_AT_FIRST_I_FRAME || sysPlaybackThumbnail == READFILE_THUMBNAIL_PREVIEW)
			asfIndexTableRead = 1;
    }

	
    /* BJ: 0718 E*/

    /* open file */
    if ((pFile = dcfOpen((signed char*)dcfPlaybackCurFile->pDirEnt->d_name, "r")) == NULL) {
        DEBUG_ASF("ASF open file error!!!\n");
        return 0;
    }

    /* BJ: 0718 S*/
    /* video */
    VideoPictureIndex       = 0;    /* cytsai: for armulator only */
    NVOPCnt                 = 0;
    VideoBufMngReadIdx  = 0;
    VideoBufMngWriteIdx = 0;
    MainVideodisplaybuf_idx     = 0;
    asfVideoIndex           = 0;
    IsuIndex                = 0;
	IDUInterruptTime		= 0;    //Lsk 090326
    IDUSlowMotionSpeed      = 0;
    MicroSecPerFrame        = 0;
    mpegflag                = 0;
    mpeg4taskrun            = 0;
    isuStatus_OnRunning     = 0;
    MPEG4_Mode              = 1;    // 0: record, 1: playback
    Video_Mode              = 1;    // 0: record, 1: playback
    CloseFlag               = 1;
    MPEG4_Error             = 0;


    for(i = 0; i < VIDEO_BUF_NUM; i++) {
        VideoBufMng[i].buffer   = VideoBuf;
    }
    VideoStreamBuf          = (u8*)VideoBufMng[0].buffer;
    for(i = 0; i < DISPLAY_BUF_NUM; i++) {
        Videodisplaytime[i] = 0;
    }

#ifdef  ASF_AUDIO
    /* audio */
    asfAudiChunkCount       = 0;
    asfAudiPresentTime      = 0;
    iisSounBufMngReadIdx    = 0;
    iisSounBufMngWriteIdx   = 0;
    asfAudioIndex           = 0;
    AudioStreamLength       = 0;
    AudioPlayback           = 0;    // 0: audio not play, 1: audio playing
    IISMode                 = 1;    // 0: record, 1: playback, 2: receive and playback audio in preview mode
    IISTime                 = 0;    // Current IIS playback time(micro second)
    IISTimeUnit             = 0;    // IIS playback time per DMA(micro second)
    iisPlayCount            = 0;    // IIS played chunk number
    iisTotalPlay            = 0;    // IIS total trigger playback number
    /* initialize sound buffer */
    for(i = 0; i < IIS_BUF_NUM; i++) {
        iisSounBufMng[i].buffer = iisSounBuf[i];
    }
    AudioStreamBuf          = (u8*)iisSounBufMng[0].buffer;
#endif
    /* refresh semaphore state */
    Output_Sem();
    /*
    while(VideoTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(VideoTrgSemEvt);
    }
    */
    OSSemSet(VideoTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
    }
    Output_Sem();
    //while(VideoCmpSemEvt->OSEventCnt < (DISPLAY_BUF_NUM - 2)) {
    while(VideoCmpSemEvt->OSEventCnt > (VIDEO_BUF_NUM - 2)) {
        OSSemAccept(VideoCmpSemEvt);
    }
    while(VideoCmpSemEvt->OSEventCnt < (VIDEO_BUF_NUM - 2)) {
        OSSemPost(VideoCmpSemEvt);
    }
    Output_Sem();
#ifdef  ASF_AUDIO
    /*
    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }
    */
    OSSemSet(iisTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
    }
    Output_Sem();
    while(iisCmpSemEvt->OSEventCnt > (IIS_BUF_NUM - 2)) {
        OSSemAccept(iisCmpSemEvt);
    }
    while(iisCmpSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
        OSSemPost(iisCmpSemEvt);
    }
    Output_Sem();
#endif

    /*
    DEBUG_ASF("0. VideoBufMngWriteIdx = %d\n", VideoBufMngWriteIdx);
    DEBUG_ASF("0. VideoBufMngReadIdx = %d\n", VideoBufMngReadIdx);
    DEBUG_ASF("0. asfVideoIndex = %d\n", asfVideoIndex);
    DEBUG_ASF("0. VideoPictureIndex = %d\n", VideoPictureIndex);
    DEBUG_ASF("0. IsuIndex = %d\n", IsuIndex);
    DEBUG_ASF("0. MainVideodisplaybuf_idx = %d\n", MainVideodisplaybuf_idx);
    DEBUG_ASF("0. mpeg4: VideoTrgSemEvt = %d\n", VideoTrgSemEvt->OSEventCnt);
    DEBUG_ASF("0. mpeg4: VideoCmpSemEvt = %d\n", VideoCmpSemEvt->OSEventCnt);
    */



	

    if(dcfRead(pFile, tempbuf, ASF_HEADER_SIZE_CEIL, &size) != 1) {
        DEBUG_ASF("ASF read file error!!!\n");
        //dcfClose(pFile);
        //return 0;
        Error   = 1;
        goto ExitAsfReadFile;
    }
    buf = tempbuf;
    if(matchGuid(buf)!=ASF_Header_Object_Guid)  //Lsk 090817 file start must be ASF_Header_Object_Guid
    {
        Error   = 1;
        goto ExitAsfReadFile;
    }


    while(CloseFlag)
    {
        switch(matchGuid(buf))
        {
            case ASF_Header_Object_Guid:			
				buf = asfReadHeaderObject(buf,
                                          &asfHeaderObject,
                                          &asfFilePropertiesObject,
                                          &asfAudiStreamPropertiesObject,
                                          &VideoStreamNum,
                                          &AudioStreamNum);
				
                if(asfFilePropertiesObject.file_size.lo==0 && asfFilePropertiesObject.file_size.hi==0) //file size empty
                {
                    Error   = 1;
                   goto ExitAsfReadFile;
                }

                if(buf == 0) {
                    DEBUG_ASF("asfReadHeaderObject error!!!\n");
                    //dcfClose(pFile);
                    //return 0;
                    Error   = 1;
                    goto ExitAsfReadFile;
                }

				#if (THUMBNAIL_PREVIEW_ENA == 1)
                if(sysPlaybackThumbnail == READFILE_THUMBNAIL_PREVIEW) {
                    printf("Thumbnail I frame has been read, exit\n");
                    goto ExitAsfReadFile;
                }
				#endif

				if((asfVopWidth == 640)&&(asfVopHeight == 352))
				{
				    H264_config[0x07] = 0x1E;
				    H264_config[0x08] = 0xDA;
				    H264_config[0x09] = 0x02;
				    H264_config[0x0A] = 0x80;
				    H264_config[0x0B] = 0xB6;
				    H264_config[0x0C] = 0x40;
				}
				else if((asfVopWidth == 1280)&&(asfVopHeight == 720))
				{
				    H264_config[0x07] = 0x1E;
				    H264_config[0x08] = 0xDA;
				    H264_config[0x09] = 0x01;
				    H264_config[0x0A] = 0x40;
				    H264_config[0x0B] = 0x16;
				    H264_config[0x0C] = 0xE4;
				}
				else if((asfVopWidth == 1920)&&(asfVopHeight == 1072))
				{
				    H264_config[0x07] = 0x28;
				    H264_config[0x08] = 0xDA;
				    H264_config[0x09] = 0x01;
				    H264_config[0x0A] = 0xE0;
				    H264_config[0x0B] = 0x08;
				    H264_config[0x0C] = 0x79;
				}
                else if((asfVopWidth == 1920)&&(asfVopHeight == 1080))
				{
				    H264_config[0x07] = 0x28;
				    H264_config[0x08] = 0xDA;
				    H264_config[0x09] = 0x01;
				    H264_config[0x0A] = 0xE0;
				    H264_config[0x0B] = 0x08;
				    H264_config[0x0C] = 0x9F;
				    H264_config[0x0D] = 0x95;
				}

				DEBUG_ASF("File asfVopWidth = %d, asfVopHeight = %d\n", asfVopWidth, asfVopHeight);
				
                break;
            case ASF_Data_Object_Guid:
                //DEBUG_ASF("Match ASF_Data_Object_Guid\n");
                memcpy(&asfDataObject,buf,sizeof(ASF_DATA_OBJECT));
                buf += sizeof(ASF_DATA_OBJECT);
                UsedSize.lo = sizeof(ASF_DATA_OBJECT);
			/*** Lsk 090420 : Read Index Table ***/
			if(asfIndexTableRead == 0)
			{
				s32  CurrentOffset = dcfTell(pFile);
				u32     IndexSize;
                             if (asfHeaderObject.object_size.lo <= ASF_DATA_PACKET_SIZE)
                                asfCheckHeaderSize = ASF_DATA_PACKET_SIZE;
                             else
                                asfCheckHeaderSize = ASF_HEADER_SIZE_CEIL;

                             CurrentOffset = asfCheckHeaderSize;
                                 
			    if(dcfSeek(pFile, asfCheckHeaderSize + (asfFilePropertiesObject.data_packets_count.lo * asfFilePropertiesObject.maximum_data_packet_size), FS_SEEK_SET) == 0)  {
                		DEBUG_ASF("ASF file seek to IndexOffset error!!!\n");
                        Error   = 1;
                    #if (STILL_PLAYBACK_WHEN_ERROR == 0)
                        goto ExitAsfReadFile;
                    #endif
		           	} else {
    					IndexSize =  ((asfFilePropertiesObject.file_size.lo - (u32)(dcfTell(pFile)) + 511)/512)*512;
    					if(asfBurstReadIndexObject(pFile,IndexSize) == 0) {
    						DEBUG_ASF("Find index objet header error!!!\n");
                            Error   = 1;
                        #if (STILL_PLAYBACK_WHEN_ERROR == 0)
                            goto ExitAsfReadFile;
                        #endif
    		            }

						if(u32PacketOffset)
						{
							if(CurrrentIndex > (asfIndexTableCount-1))
							{
								DEBUG_ASF("ASF file seek warning. CurrrentIndex > (asfIndexTableCount-1)!!!\n");
                            	CurrrentIndex = (asfIndexTableCount-1) ;							
							}
                            /*Find Next I frame*/
                            while(asfIndexTable[CurrrentIndex].packet_number == asfIndexTable[CurrrentIndex+1].packet_number){
                                CurrrentIndex++;
                                if(CurrrentIndex >= (asfIndexTableCount-2))
                                    break;
                            }
                            CurrrentIndex++;

							DEBUG_ASF("ASF file seek CurrrentIndex = %d!!!\n", CurrrentIndex);
							GetFirstAudioPayload = 1;
							GetFirstVideoPayload = 0;                            
						}						
                    }
    				if(dcfSeek(pFile, CurrentOffset, FS_SEEK_SET) == 0) {
                        DEBUG_ASF("ASF file seek to CurrentOffset error!!!\n");
                        Error   = 1;
                        goto ExitAsfReadFile;
    				}
	    			asfIndexTableRead = 1;
	    		}
            #if STILL_PLAYBACK_WHEN_ERROR
                while(sysPlaybackVideoStop == 0 && !(sysPlaybackThumbnail == READFILE_STOP_AT_FIRST_I_FRAME && IsuIndex > 0) && !(sysPlaybackThumbnail == READFILE_THUMBNAIL_PREVIEW))
            #else
                while(sysPlaybackVideoStop == 0 && !Error && !MPEG4_Error && 
                    !(sysPlaybackThumbnail == READFILE_STOP_AT_FIRST_I_FRAME && IsuIndex > 0) && !(sysPlaybackThumbnail == READFILE_THUMBNAIL_PREVIEW))
            #endif
                {
                    if(curr_slow_speed != IDUSlowMotionSpeed){   // get slow speed from UI
                        IDUSlowMotionSpeed = curr_slow_speed;
                        if(IDUSlowMotionSpeed == 0)
                            IISTime = iisSounBufMng[iisSounBufMngReadIdx].time - Video_timebase; //when resume play, IIStime sync to Video
                    }
                	if((video_playback_speed != curr_playback_speed) && mpeg4taskrun )//|| sysPlaybackVideoPause) //Lsk 090406 : play speed change //Lsk 090414: pause
                	                                                                  //Lsk : 091216 to avoid mpeg4SuspendTask twice
					{
						video_playback_speed = curr_playback_speed;  //get new speed from UI
						sysPlaybackForward  = video_playback_speed - 5;
						sysPlaybackBackward = 4 -  video_playback_speed;
						Video_timebase = VideoNextPresentTime;
						IDUInterruptTime = 0;

						if((video_playback_speed == 5) ||((pre_playback_speed <= 5) && (video_playback_speed > 5 )) )//||sysPlaybackVideoPause)			// 2x->1x;-1x->1x;1x->2x //Lsk 090414: pause
						{
							CurrrentIndex = ((Video_timebase/1000) + (1000 + PREROLL ))/1000;  //Lsk : search asfIndexTable to get next I fame packet number
                            if((CurrrentIndex < (asfIndexTableCount-1)) || (video_playback_speed == 5 && (CurrrentIndex == (asfIndexTableCount-1))))
                            {
                                ResetPlayback=1;
                                DEBUG_ASF("Reach last I and press Play.\n press FF/Play\n");
                            }
						}
						else if((pre_playback_speed >= 5) && (video_playback_speed < 5 ) )	 // 1x->-1x
						{
							CurrrentIndex = ((Video_timebase/1000) + (1000 + PREROLL ))/1000;       //Lsk : search asfIndexTable to get prev I fame packet number
							if(CurrrentIndex > (asfIndexTableCount-1))
                                CurrrentIndex = (asfIndexTableCount-1) ;

                            if(CurrrentIndex <= 4)//(1000 + PREROLL )/1000
                                DEBUG_ASF("Reach file head and press RF\n");
                            else
							    ResetPlayback=1;
						}

                        pre_playback_speed =video_playback_speed;
						if(video_playback_speed == 5)
						{
							GetFirstAudioPayload = 0;
							GetFirstVideoPayload = 1;
						}
						else if(ResetPlayback)
						{
							GetFirstAudioPayload = 1;
							GetFirstVideoPayload = 0;
						}
					}

                    /* If the whole file doesn't exist any I frame, Exit Playback*/
                    if(!mpeg4taskrun && (PacketCount.lo == asfFilePropertiesObject.data_packets_count.lo && sysPlaybackForward >=0 ))
                    {
						DEBUG_ASF("Not exists any I frame, error file !!!\n");
                        if(sysPlaybackThumbnail == READFILE_NORMAL_PLAY) //if thumbnail == 1, and return error, UI will stuck
                    		Error   = 1;
	                    goto ExitAsfReadFile;                        
                    }

					//if( (sysPlaybackThumbnail == 0) && ((CurrrentIndex >= asfIndexTableCount && sysPlaybackForward >=0 ) || (CurrrentIndex == 0 && sysPlaybackBackward >=0 )|| (PacketCount.lo == asfFilePropertiesObject.data_packets_count.lo && sysPlaybackForward >=0 )))
					if( ResetPlayback ==0 && mpeg4taskrun && ((CurrrentIndex <= 4 && sysPlaybackBackward >=0 )|| 
                        ((PacketCount.lo == asfFilePropertiesObject.data_packets_count.lo || (CurrrentIndex >= (asfIndexTableCount-1))) && sysPlaybackForward >=0 )))
					{
						if(sysTVOutOnFlag && TVout_Generate_Pause_Frame)
						{
							//Lsk 090511 : use top field of Current frame to generate PAUSE frame, and store in the next frame address so set flag=1
							if( (MainVideodisplaybuf_idx  == VideoBufMngReadIdx) && (MainVideodisplaybuf_idx != 0))
							{
								asfPausePlayback(MainVideodisplaybuf[(MainVideodisplaybuf_idx-1) % DISPLAY_BUF_NUM],
  					   			 				 MainVideodisplaybuf[(MainVideodisplaybuf_idx) % DISPLAY_BUF_NUM],
					  				   			1);

							}
						}

                        #if 0//Lsk 090507 : current file stop, continue next file
    						if(sysPlaybackForward == 0 && iisTrgSemEvt->OSEventCnt == 0)       //Lsk : normal speed, waiting all video frame finish
    						{
                                sysPlaybackVideoStop = 1;
                                VideoNextPresentTime = (VideoDuration-3)*1000000;
    						}
						#else //Lsk: exit current file
						if(videoPlayNext)
						{
    						if(sysPlaybackForward == 0 && iisTrgSemEvt->OSEventCnt == 0)       //Lsk : normal speed, waiting all video frame finish
    						{
                                sysPlaybackVideoStop = 1;
                                VideoNextPresentTime = (VideoDuration-3)*1000000;
								printf("Auto exit\n");
    						}
							//Lsk : FF speed, waiting all video frame finish
	                        if(sysPlaybackForward > 0)
	                        {
	                            PlayIndex = ((VideoNextPresentTime/1000) + (1000 + PREROLL ))/1000;  
	                            DEBUG_ASF("<%d,%d>\n",PlayIndex,asfIndexTableCount -1);

                                if(MainVideodisplaybuf_idx >= IsuIndex - 1) // to avoid the end of file's frame is all error, our playIndex can't reach the EOF threshold
                                    stuck_cnt++;
                                else
                                    stuck_cnt = 0;
                                
	                            if( (PlayIndex >= asfIndexTableCount - 5) || (asfIndexTableCount <= 5) || (stuck_cnt > (ASF_STUCK_THRESHOLD >>sysPlaybackForward))) //due to IVOP = 60, so index table have same content
	                            {
	                                VideoNextPresentTime = (VideoDuration-3)*1000000;
	                                sysPlaybackVideoStop = 1;
	                                DEBUG_ASF("stop cycle playback FF, stuck:%d\n", (stuck_cnt > (ASF_STUCK_THRESHOLD >>sysPlaybackForward)));
	                            }                                
	                        }
	                        //Lsk : RF speed, waiting all video frame finish
	                        if(sysPlaybackBackward >=0)
	                        {                                                                   
	                            PlayIndex = ((VideoNextPresentTime/1000) + (1000 + PREROLL ))/1000;
	                            DEBUG_ASF("<%d,%d>\n",PlayIndex,asfIndexTableCount -2);                                    
                                
                                if(MainVideodisplaybuf_idx >= IsuIndex - 1) // to avoid the head of file's frame is all error, our playIndex can't reach the HOF threshold
                                    stuck_cnt++;
                                else
                                    stuck_cnt = 0;
	                            if((PlayIndex <= 4+2) || (stuck_cnt > (ASF_STUCK_THRESHOLD >> sysPlaybackBackward)))
	                            {
	                                VideoNextPresentTime = 0;
	                                sysPlaybackVideoStop = 1;
	                                DEBUG_ASF("stop cycle playback RF, stuck: %d\n", (stuck_cnt > (ASF_STUCK_THRESHOLD >> sysPlaybackBackward)));                                    
	                            }
	                        }	
						}																	                       						
                        #endif

                        /*** check short file ***/
						if(sysPlaybackThumbnail == READFILE_NORMAL_PLAY &&AudioPlayback == 0 && video_playback_speed == 5)
						{
						    DEBUG_ASF("Detect current file is very short\n");
							AudioPlayback       = 1;
							#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
							IDUInterruptTime = 0;
							StartPlayBack = 1;
							#endif
				            iisResumeTask();
						}
						OSTimeDly(1);//Lsk 090505
						continue;
					}
					if(ResetPlayback)       //Lsk 090406 : clear buffer when ResetPlayback
					{
					    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_FINISH, OS_FLAG_SET, &err);
						if(!sysTVOutOnFlag)  //Lsk 090507 : for pannel FF, BF
						{
							if(video_playback_speed == 5)
							{
								IduIntCtrl &= 0xfffffeff;
								DEBUG_ASF("disable IDU interrupt\n");
							}
							else
							{
								IduIntCtrl |= 0x00000100;
								DEBUG_ASF("enable IDU interrupt\n");
							}
						}
						/*** Stop Playback System ***/
						#ifdef  ASF_AUDIO
    					OSSemSet(iisTrgSemEvt, 0, &err);
						if (err != OS_NO_ERR) {
							DEBUG_ASF("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
    					}
						#endif

					    OSSemSet(VideoTrgSemEvt, 0, &err);
						if (err != OS_NO_ERR) {
							DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
						}

						/* delay until mpeg4 and IIS task reach pend state */
						for(i = 0; (!Video_Pend); i++) {
							OSTimeDly(1);
					    }

						/* suspend mpeg4 task */
					    mpeg4SuspendTask();

						#ifdef  ASF_AUDIO
						for(i=0,IIS_Task_Stop=1; (!IIS_Task_Pend)&&(i<30); i++) {
					        OSTimeDly(1);
					    }
						iis5StopPlay();
						iisSuspendTask();
						#endif

				    	OSSemSet(isuSemEvt, 0, &err);
					    if (err != OS_NO_ERR) {
					        DEBUG_ASF("OSSemSet Error: isuSemEvt is %d.\n", err);
					    }

					    // reset MPEG-4 hardware
					    SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
						SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100;

					    // reset IIS hardware
					    iis5Reset(IIS_SYSPLL_SEL_48M);

						/*** Reset Playback System ***/
						VideoPictureIndex       = 0;
                        NVOPCnt                 = 0;
    					VideoBufMngReadIdx  = 0;
    					VideoBufMngWriteIdx = 0;
					    MainVideodisplaybuf_idx     = 0;
					    asfVideoIndex           = 0;
					    IsuIndex                = 0;
					    MicroSecPerFrame        = 0;
					    mpegflag                = 0;
						mpeg4taskrun            = 0;
					    isuStatus_OnRunning     = 0;
					    MPEG4_Mode              = 1;
                        Video_Mode              = 1;
					    CloseFlag               = 1;
					    MPEG4_Error             = 0;
    					for(i = 0; i < VIDEO_BUF_NUM; i++) {
							VideoBufMng[i].buffer   = VideoBuf;
					    }
						VideoStreamBuf          = (u8*)VideoBufMng[0].buffer;
					    for(i = 0; i < DISPLAY_BUF_NUM; i++) {
							Videodisplaytime[i] = 0;
    					}
						#ifdef  ASF_AUDIO
					    /* audio */
					    asfAudiChunkCount       = 0;
					    asfAudiPresentTime      = 0;
					    iisSounBufMngReadIdx    = 0;
						iisSounBufMngWriteIdx   = 0;
    					asfAudioIndex           = 0;
					    AudioStreamLength       = 0;
					    AudioPlayback           = 0;    // 0: audio not play, 1: audio playing
					    IISMode                 = 1;    // 0: record, 1: playback, 2: receive and playback audio in preview mode
					    IISTime                 = 0;    // Current IIS playback time(micro second)
					    iisPlayCount            = 0;    // IIS played chunk number
					    iisTotalPlay            = 0;    // IIS total trigger playback number
						//    GetFrameFlag            = 0;
					    /* initialize sound buffer */
    					for(i = 0; i < IIS_BUF_NUM; i++) {
				        	iisSounBufMng[i].buffer = iisSounBuf[i];
						}
					    AudioStreamBuf          = (u8*)iisSounBufMng[0].buffer;
						#endif

					    /* refresh semaphore state */
					    Output_Sem();
					    /*
					    while(VideoTrgSemEvt->OSEventCnt > 0) {
				        OSSemAccept(VideoTrgSemEvt);
					    }
					    */
					    OSSemSet(VideoTrgSemEvt, 0, &err);
					    if (err != OS_NO_ERR) {
					        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
					    }
					    Output_Sem();
					    while(VideoCmpSemEvt->OSEventCnt > (VIDEO_BUF_NUM - 2)) {
					        OSSemAccept(VideoCmpSemEvt);
					    }
					    while(VideoCmpSemEvt->OSEventCnt < (VIDEO_BUF_NUM - 2)) {
					        OSSemPost(VideoCmpSemEvt);
					    }
					    Output_Sem();
						#ifdef  ASF_AUDIO

						    while(iisTrgSemEvt->OSEventCnt > 0) {
					        OSSemAccept(iisTrgSemEvt);
					    }

    					OSSemSet(iisTrgSemEvt, 0, &err);
					    if (err != OS_NO_ERR) {
					        DEBUG_ASF("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
					    }
					    Output_Sem();
					    while(iisCmpSemEvt->OSEventCnt > (IIS_BUF_NUM - 2)) {
				        OSSemAccept(iisCmpSemEvt);
					    }
					    while(iisCmpSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
					        OSSemPost(iisCmpSemEvt);
					    }
					    Output_Sem();
						#endif

                        /*For FF, to avoid go back and forth, so find next I frame, not prev I frame*/
                        if(sysPlaybackForward > 0 && !asfSeekPrevIFrame){
                            while(asfIndexTable[CurrrentIndex].packet_number == asfIndexTable[CurrrentIndex+1].packet_number){
                                CurrrentIndex++;
                                if(CurrrentIndex >= (asfIndexTableCount-2))
                                    break;
                            }
                            CurrrentIndex++;
                        }
						/*** Reset File Position***/
						Offset      = (asfIndexTable[CurrrentIndex].packet_number - PacketCount.lo)* asfFilePropertiesObject.maximum_data_packet_size;
						PacketCount.lo  = asfIndexTable[CurrrentIndex].packet_number;
						if(dcfSeek(pFile, Offset, FS_SEEK_CUR)==0) {
								DEBUG_ASF("2.seek error!!!\n");
	                    		Error   = 1;
			                    goto ExitAsfReadFile;
    	    		    }
						StartFrame  = 1;  //Lsk 090406 : To confirm start from I payload
						Restart1 = 1;
						KeyFramePacketCount = asfIndexTable[CurrrentIndex].packet_count;
						StartPlayBack = 0;
						ResetPlayback = 0;


						/*** Lsk 090415 : Enable ADC/DAC ***/
						if(video_playback_speed == 5)
						{
							#if ((AUDIO_OPTION== AUDIO_IIS_DAC) || (AUDIO_OPTION== AUDIO_ADC_DAC))
  							 #if(AUDIO_DEVICE == AUDIO_NULL)
      						 init_DAC_play(1);
  							 #endif
                            #elif ((AUDIO_OPTION== AUDIO_IIS_IIS) || (AUDIO_OPTION== AUDIO_ADC_IIS))
  							 #if (AUDIO_DEVICE== AUDIO_IIS_WM8974)
	        				Init_IIS_WM8974_play();
							 #elif (AUDIO_DEVICE== AUDIO_IIS_WM8940)
	        				Init_IIS_WM8940_play();
  							 #elif(AUDIO_DEVICE == AUDIO_IIS_ALC5621)
					        Init_IIS_ALC5621_play();
  							 #elif(AUDIO_DEVICE == AUDIO_AC97_ALC203)
					        ac97SetupALC203_play();
                             #endif
							#endif
						}
					}

                    /*When switch source, find prev I frame for H264 downsampling*/
                    if(asfSeekPrevIFrame == 1){
                        printf("seek frame %lld\n", VideoNextPresentTime);
 						GetFirstAudioPayload = 1;
                        GetFirstVideoPayload = 0;                        
                        if(video_playback_speed != 5){
        					Video_timebase = VideoNextPresentTime;
        					IDUInterruptTime = 0;
                        }
                        
                        asfSeekPrevIFrame = 0;
                        CurrrentIndex = ((VideoNextPresentTime/1000) + (1000 + PREROLL ))/1000;
        				if(CurrrentIndex > (asfIndexTableCount-1))
                            CurrrentIndex = (asfIndexTableCount-1) ;                        
                        printf("seek CurrrentIndex %d\n", CurrrentIndex);
                        Offset      = (asfIndexTable[CurrrentIndex].packet_number - PacketCount.lo)* asfFilePropertiesObject.maximum_data_packet_size;
						if(dcfSeek(pFile, Offset, FS_SEEK_CUR)==0) {
							DEBUG_ASF("3.seek error!!!\n");
	                    	Error   = 1;
			                goto ExitAsfReadFile;
    	    		    }                        
                        PacketCount.lo = asfIndexTable[CurrrentIndex].packet_number;
                        KeyFramePacketCount = asfIndexTable[CurrrentIndex].packet_count;
                    }

					if(KeyFramePacketCount==0 && (sysPlaybackBackward >= 0))  //Lsk 090406: fseek to get prev I frame packet number
					{
						CurrrentIndex -= 2;
						if(CurrrentIndex < 4){
							CurrrentIndex = 4;
							DEBUG_ASF("FB Read Frame Reach Head, CurrrentIndex is %d\n",CurrrentIndex);
						}
						KeyFramePacketCount = asfIndexTable[CurrrentIndex].packet_count;
						Offset      = (asfIndexTable[CurrrentIndex].packet_number - PacketCount.lo)* asfFilePropertiesObject.maximum_data_packet_size;
						if(dcfSeek(pFile, Offset, FS_SEEK_CUR)==0) {
							DEBUG_ASF("3.seek error!!!\n");
	                    	Error   = 1;
			                goto ExitAsfReadFile;
    	    		    }
						//DEBUG_ASF("CurrrentIndex is %d\n",CurrrentIndex);
						PacketCount.lo = asfIndexTable[CurrrentIndex].packet_number;
					}

					if(KeyFramePacketCount==0 && (sysPlaybackForward > 0))  //Lsk 090406: fseek to get Next I frame packet number
					{
						CurrrentIndex += 2;
						if(CurrrentIndex > (asfIndexTableCount-1)){
							CurrrentIndex = (asfIndexTableCount-1);   
							DEBUG_ASF("FF Read Frame Reach End, CurrrentIndex is %d\n",CurrrentIndex);
        				}
						KeyFramePacketCount = asfIndexTable[CurrrentIndex].packet_count;
						Offset      = (asfIndexTable[CurrrentIndex].packet_number - PacketCount.lo)* asfFilePropertiesObject.maximum_data_packet_size;
						if(dcfSeek(pFile, Offset, FS_SEEK_CUR)==0) {
							DEBUG_ASF("3.seek error!!!\n");
							Error   = 1;
							goto ExitAsfReadFile;
						}
						//DEBUG_ASF("CurrrentIndex is %d\n",CurrrentIndex);
						PacketCount.lo = asfIndexTable[CurrrentIndex].packet_number;
					}
                    
					KeyFramePacketCount--;
                    if(dcfRead(pFile, tempbuf, asfFilePropertiesObject.maximum_data_packet_size , &size) == 0) {
                        Error                   = 1;
                        sysPlaybackVideoStop    = 1;
                        break;
                    }
                    buf = tempbuf;

                    buf =  asfReadPacketHeader(buf,
                                               &asfFilePropertiesObject,
                                               &PacketUsedSize,
                                               &flag2,
                                               &Mul_Payload,
                                               &PacketLength,
                                               &PaddingLength,//Lsk 090304
                                               &UsedSize);
                    if(buf == 0) {
                        Error                   = 1;
                        sysPlaybackVideoStop    = 1;
                        break;
                    }

                    if(Mul_Payload)
                    {
                    	PayloadFlag = *((u8 *)buf);
                        PayloadNum = (PayloadFlag & 0x3F);
                        buf += 1; // Payload Flag
                        UsedSize.lo += 1; // Payload Flag
                    }
                    else
                    	PayloadNum = 1;

                    if(PayloadNum == 4)//???
                    {
                    	PayloadNum = PayloadNum;
                    }

					//DEBUG_RED("Packet info : PacketLength=%d, PayloadNum=%d\n", PacketLength, PayloadNum);
					
                    while(PayloadNum != 0)
                    {

                    	PayloadNum--;

                        buf = asfReadPayloadHeader(buf,
												   &StreamNum,
                                                   &KeyFrame,
                                                   flag2,
                                                   Restart1,
                                                   Restart2,
                                                   PayloadFlag,
                                                   &Offset2Media,
                                                   &PayloadLength,
                                                   &MediaObjectSize,   //Lsk 090622
                                                   &asfPresentTime,
                                                   &asfVideoIndex,
                                                   &asfAudioIndex,
                                                   &UsedSize,
                                                   Mul_Payload,    //Lsk 090304
                                                   PacketLength,
                                                   PaddingLength);

						//DEBUG_RED("Payload info : StreamNum=%d, asfPresentTime=%08d, PayloadLength=%06d\n", StreamNum, (u32)asfPresentTime, PayloadLength);
						
                        if(StreamNum == 1 && (sysPlaybackForward ==0 ||KeyFrame)) // video stream
                        {
                        	if(GetFirstVideoPayload == 0) //Lsk 090415 : reset Video_timebase, for AV sync
							{
								Video_timebase = asfPresentTime * 1000;    //Lsk 090415 : Because IDUInterruptTime and IISTime start , refresh video_timebase
								GetFirstVideoPayload = 1;
							}
							VideoStreamBuf  = asfReadVideoPayload(buf,
                                                                  VideoStreamBuf,
                                                                  KeyFrame,
                                                                  &StartFrame,
                                                                  &Restart1,
                                                                  &VideoStreamLength,
                                                                  Offset2Media,
                                                                  PayloadLength,
                                                                  MediaObjectSize,   //Lsk 090622
                                                                  asfPresentTime,
                                                                  &asfVideoIndex,
                                                                  &mpeg4taskrun,
                                                                  &u32PacketOffset,
                                                                  &UsedSize,
                                                                  pFile,
                                                                  &asfFilePropertiesObject,
                                                                  &PacketCount);

							//if(sysPlaybackForward != 0 && (IsuIndex >= (DISPLAY_BUF_NUM - 2)))  //Lsk 090422 : Don't need to waitting buffer full
							//{
							//	StartPlayBack = 1;
							//}

                        }
#ifdef  ASF_AUDIO
                        else if(StreamNum == 2 && sysPlaybackForward == 0 && StartFrame == 0) // audio stream
                        {
                        	if(GetFirstAudioPayload == 0) //Lsk 090415 : reset IISTime, for AV sync
							{
								Video_timebase = asfPresentTime * 1000;    //Lsk 090415 : Because IDUInterruptTime and IISTime start , refresh video_timebase
								GetFirstAudioPayload = 1;
							}

						#if (AUDIO_CODEC == AUDIO_CODEC_PCM)
                        	AudioStreamBuf  = asfReadAudioPayload(buf,
                                                                  AudioStreamBuf,
                                                                  &StartAudio,
                                                                  &Restart2,
                                                                  &AudioStreamLength,
                                                                  Offset2Media,
                                                                  PayloadLength,
                                                                  asfPresentTime,
                                                                  &asfAudioIndex,
                                                                  &AudioPlayback);
                        #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
                        	AudioStreamBuf  = asfReadAudioPayload_IMA_ADPCM(buf,
                                                                  AudioStreamBuf,
                                                                  &StartAudio,
                                                                  &Restart2,
                                                                  &AudioStreamLength,
                                                                  Offset2Media,
                                                                  PayloadLength,
                                                                  &asfAudioIndex,
                                                                  &AudioPlayback,
                                                                  &asfAudiStreamPropertiesObject);
                        #endif
                        }
#endif
                        buf += PayloadLength;
                        UsedSize.lo += PayloadLength;
                    }
                    PacketCount.lo++;
                    PacketUsedSize = ((u32)buf) - PacketUsedSize;
                    if(PacketUsedSize != PacketLength)
                    {
                        buf += PacketLength - PacketUsedSize;
                        UsedSize.lo += PacketLength - PacketUsedSize;
                    }
                }


				// Stop video playback if user press stop or backward button or thumbnail playback
                if(sysPlaybackVideoStop == 1 ||
                   sysPlaybackThumbnail == READFILE_STOP_AT_FIRST_I_FRAME ||
                   sysPlaybackThumbnail == READFILE_THUMBNAIL_PREVIEW ||
                   Error ||
                   MPEG4_Error) {
                    if((UsedSize.lo == asfDataObject.object_size.lo) &&
                       !(Error || MPEG4_Error || mpeg4taskrun)) {
                        //OSSemPost(VideoTrgSemEvt);
                        if(asfVideoIndex <= 1) {
                            if(MPEG4_Task_Go) {
                                OSSemAccept(VideoTrgSemEvt);
                            }
                            mpeg4ResumeTask();
                            mpeg4taskrun    = 1;
                        }
                        OSTimeDly(1);
                    }
                    if(!(Error || MPEG4_Error)) {
                        for(i = 0; (i < 5) && ((VideoPictureIndex+NVOPCnt) == 0); i++) {
                            OSTimeDly(1);
                        }
                    }
                    if(sysPlaybackThumbnail == READFILE_STOP_AT_FIRST_I_FRAME)
                        iduPlaybackFrame(PKBuf0);
                    OSSemSet(VideoTrgSemEvt, 0, &err);
                    OSSemSet(iisTrgSemEvt, 0, &err);
                    CloseFlag   = 0;
                    break;
                }
                /*
                asfVideoIndex++;
                asfVopCount = asfVideoIndex;
                VideoBufMng[VideoBufMngWriteIdx].size   = VideoStreamLength;
                VideoBufMngWriteIdx = (VideoBufMngWriteIdx + 1) % VIDEO_BUF_NUM;
                //DEBUG_ASF("asfVideoIndex = %d\n", asfVideoIndex);
                //if(asfVideoIndex >= asfVopCount) {
                DEBUG_ASF("Read ASF bitstream finish, playing...\n");
                //}
                OSSemPost(VideoTrgSemEvt);
                */
                if(asfVideoIndex <= 1) {
                    if(MPEG4_Task_Go) {
                        OSSemAccept(VideoTrgSemEvt);
                    }
                    mpeg4ResumeTask();
                    mpeg4taskrun    = 1;
                }
                OSTimeDly(1);

#ifdef  ASF_AUDIO
                asfAudioIndex++;
                asfAudiChunkCount   = asfAudioIndex;
                iisSounBufMng[iisSounBufMngWriteIdx].size   = AudioStreamLength;
                iisSounBufMngWriteIdx = (iisSounBufMngWriteIdx + 1) % IIS_BUF_NUM;
                OSSemPost(iisTrgSemEvt);
                if((AudioPlayback == 0) && (asfAudioIndex >= 1) ) {    // only trigger IIS one time
                    AudioPlayback           = 1;
					IDUInterruptTime = 0;
                    iisResumeTask();
                  	IDUInterruptTime = 0;
        			StartPlayBack = 1;
                }
#else
                // display video without IIS timing
                if(MainVideodisplaybuf_idx < IsuIndex) {
                    if(MainVideodisplaybuf_idx == 0)
                        iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
                    else
                        iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
                    MainVideodisplaybuf_idx++;
                }
#endif


				CloseFlag   = 0;

                break;
//          case ASF_Simple_Index_Object_Guid:
//              memcpy(&asfSimpleIndexObject,buf,sizeof(ASF_SIMPLE_INDEX_OBJECT));
//              buf += sizeof(ASF_SIMPLE_INDEX_OBJECT);
//              CloseFlag = 0;
//              break;
            case NoMatch_Guid:
            default:
                break;
        }
    }
        /* BJ: 0718 E*/

    if(sysPlaybackVideoStop) {
		if(!sysTVOutOnFlag)
			IduIntCtrl &= 0xfffffeff;
		else if(TVout_Generate_Pause_Frame)
		{	//Lsk 090511 : use top field of Current frame to generate PAUSE frame, and store in the next frame address so set flag=1
		    if(MainVideodisplaybuf_idx == 0)
                asfPausePlayback(MainVideodisplaybuf[0],
  				   			 MainVideodisplaybuf[1],
  				   			 2);
            else
            	asfPausePlayback(MainVideodisplaybuf[(MainVideodisplaybuf_idx-1) % DISPLAY_BUF_NUM],
  				   			 MainVideodisplaybuf[(MainVideodisplaybuf_idx) % DISPLAY_BUF_NUM],
  				   			 1);
		}
        sysPlaybackVideoStopDone = 1;
        DEBUG_ASF("3.Video playback stop finish!!!\n");
    } else if (sysPlaybackThumbnail) {
        DEBUG_ASF("Video playback thumbnail finish!!! Mode:%d \n", sysPlaybackThumbnail);
        //while(IsuIndex == 0) {
        for(i = 0; i < 5 && IsuIndex == 0; i++) {
            OSTimeDly(1);
        }

        if(sysTVOutOnFlag && TVout_Generate_Pause_Frame)
		{
    		 if(MainVideodisplaybuf_idx == 0)
                asfPausePlayback(MainVideodisplaybuf[0],
  				   			 MainVideodisplaybuf[1],
  				   			 2);
            else
            	asfPausePlayback(MainVideodisplaybuf[(MainVideodisplaybuf_idx-1) % DISPLAY_BUF_NUM],
  				   			 MainVideodisplaybuf[(MainVideodisplaybuf_idx) % DISPLAY_BUF_NUM],
  				   			 1);
		}
        else
        {
        	#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
         
			if(sysPlaybackHeight > 720)   	   
			{		 				
				#if(USE_NEW_MEMORY_MAP)
				#else
				Idu_ClearBuf_9300FHD_part_Idx(0);
				#endif
				
				IduVidBuf0Addr = (u32)MainVideodisplaybuf[0];
		        #if NEW_IDU_BRI
				BRI_IADDR_Y = IduVidBuf0Addr;
				BRI_IADDR_C = IduVidBuf0Addr + (1920*1440);		
				BRI_IN_SIZE=(720<<16) | (1920-128);
				BRI_STRIDE=1920*2;
				#endif

				#if(USE_NEW_MEMORY_MAP)
				#else
				for(i = 1; i < DISPLAY_BUF_NUM; i++) {
					Idu_ClearBuf_9300FHD_part_Idx(i);
				}
				#endif
			}
			else
			{				
				#if(USE_NEW_MEMORY_MAP)
				IduVidBuf0Addr = (u32)MainVideodisplaybuf[0] + 1920*180;
				
		        #if NEW_IDU_BRI
				BRI_IADDR_Y = (u32)MainVideodisplaybuf[0] + 1920*180;
				BRI_IADDR_C = (u32)MainVideodisplaybuf[0] + (1920*1440) + 1920*90;		
				//BRI_IN_SIZE=(720<<16) | (1920-128);
				//BRI_STRIDE=1920*2;
				#endif				
				#else
				IduVidBuf0Addr = (u32)MainVideodisplaybuf[0];
				
		        #if NEW_IDU_BRI
				BRI_IADDR_Y = IduVidBuf0Addr;
				BRI_IADDR_C = IduVidBuf0Addr + (1920*1440);		
				//BRI_IN_SIZE=(720<<16) | (1920-128);
				//BRI_STRIDE=1920*2;
				#endif
				#endif
			}
			#else
			{
				IduVidBuf0Addr = (u32)MainVideodisplaybuf[0];
		        #if NEW_IDU_BRI
			    BRI_IADDR_Y = IduVidBuf0Addr;
			    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
		        #endif
			}
			#endif
        }

    } else if(!MPEG4_Error && !Error){
#ifdef  ASF_AUDIO
        DEBUG_ASF("Waiting for audio playback finish... %d/%d\n", iisPlayCount, asfAudiChunkCount);
        while(iisPlayCount < asfAudiChunkCount && sysPlaybackVideoStop == 0 && !MPEG4_Error && !sysPlaybackForward && !sysPlaybackBackward) {
            OSTimeDly(1);
        }

        if(sysPlaybackVideoStop == 0 ) {
            DEBUG_ASF("Audio playback finish!!!\n");
        }



#endif
        if(sysPlaybackVideoStop == 0 && !MPEG4_Error) {
            DEBUG_ASF("MainVideodisplaybuf_idx = %d, IsuIndex = %d\n", MainVideodisplaybuf_idx, IsuIndex);
			while(MainVideodisplaybuf_idx < IsuIndex)  //Lsk : waiting all video frame finish
				OSTimeDly(1);
			DEBUG_ASF("MainVideodisplaybuf_idx = %ld, IsuIndex = %ld\n", MainVideodisplaybuf_idx, IsuIndex);
            DEBUG_ASF("Video playback finish!!!\n");

        } else {
            OSSemSet(VideoTrgSemEvt, 0, &err);
            if (err != OS_NO_ERR) {
                DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
            }
            OSSemSet(iisTrgSemEvt, 0, &err);
            if (err != OS_NO_ERR) {
                DEBUG_ASF("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
            }
            DEBUG_ASF("4.Video playback stop finish!!!\n");
        }
        if(MainVideodisplaybuf_idx == 0)
            asfPausePlayback(MainVideodisplaybuf[0], MainVideodisplaybuf[1], 2);
        else
            asfPausePlayback(MainVideodisplaybuf[(MainVideodisplaybuf_idx-1) % DISPLAY_BUF_NUM],
  				   			 MainVideodisplaybuf[(MainVideodisplaybuf_idx) % DISPLAY_BUF_NUM],
  				   			 1);
    }


ExitAsfReadFile:

	OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_FINISH, OS_FLAG_SET, &err);

#ifdef  ASF_AUDIO
    // for fix PA9002D bug
    //iisDisInt();
#endif



    /* close file */
    if(dcfClose(pFile, &tmp) == 0) {
        DEBUG_ASF("dcfClose() error!!!\n");
    }

#ifdef  ASF_AUDIO

    OSSemSet(iisTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
    }
#endif

    OSSemSet(VideoTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
    }
    sysPlaybackVideoStop = 1;
    /* delay until mpeg4 and IIS task reach pend state */
    //for(i = 0; (i < 30) && !Video_Pend; i++) {
    for(i = 0; !Video_Pend; i++) {
        OSTimeDly(1);
    }


    /* suspend mpeg4 task */
    mpeg4SuspendTask();

#ifdef  ASF_AUDIO

#if (FILE_PLAYBACKFILE_SET == FILE_PLAYBACK_SPLITMENU)
	for(i = 0, IIS_Task_Stop = 1; (!IIS_Task_Pend) && (i < 30) && (!sysPlaybackThumbnail); i++) {
#else
	#if( (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
	for(i = 0, IIS_Task_Stop = 1; (!IIS_Task_Pend) && (i < 30) && (!sysPlaybackThumbnail); i++) {
	#else
	for(i = 0, IIS_Task_Stop = 1; (!IIS_Task_Pend) && (i < 30)&& (sysPlaybackThumbnail != READFILE_THUMBNAIL_PREVIEW); i++) {
	#endif	
#endif
        OSTimeDly(1);
    }
    iis5StopPlay();
    iisSuspendTask();

#endif

	OSSemSet(isuSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: isuSemEvt is %d.\n", err);
    }
    DEBUG_ASF("ASF file playback finish, total %d/%d/%d frames, %d/%d audio chunk!!!\n",
            MainVideodisplaybuf_idx, IsuIndex, asfVopCount, iisPlayCount, asfAudiChunkCount);

    // reset MPEG-4 hardware
    SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
    SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100;

    // reset IIS hardware
    iis5Reset(IIS_SYSPLL_SEL_48M);

    if(Error || MPEG4_Error) { // Video playback fail
        return 0;
    } else {    // Video playback success
        return 1;
    }

}

#if 1

FS_FILE* 	pVideoFile;
u32 video_cnt = 0;
u8 p2p_play = 0;
u8 p2p_play_1 = 0;
u8 p2p_30PS = 0;
u8* asfSplitVideoPayload(u8      *buf,
                        u8      *VideoStreamBuf,
                        u8      KeyFrame,
                        u8      *StartFrame,
                        u8      *Restart1,
                        u32     *VideoStreamLength,
                        u32     Offset2Media,
                        u32     PayloadLength,
                        u32     MediaObjectSize,   //Lsk 090622
                        s64     asfPresentTime,
                        u32     *P2PasfVideoIndex,
                        u8      *mpeg4taskrun,
                        u32     *u32PacketOffset,
                        u64     *UsedSize,
                        FS_FILE *pFile,
                        ASF_FILE_PROPERTIES_OBJECT *asfFilePropertiesObject,
                        u64     *PacketCount)
{
    u8      *pBuf;
    u32     time1,time2,time4=0;
    static u32 time3; 
    /* Lsk : read video payload first time, check I frame */

    if(p2p_play_1 == 0)
    {
        time2 = (asfPresentTime*1000 - tmp1) / 1000;
        if(time2 < 50)
            p2p_30PS=1;
        else
            p2p_30PS=0;
        p2p_play_1=1;
    }  
    if(Offset2Media == 0 && *StartFrame == 1 && (*Restart1 == 0 || (*Restart1 == 1 && KeyFrame == 1)))
    {
        *StartFrame         = 0;
        *Restart1           = 0;
        VideoStreamBuf      = P2PVideoBufMng[P2PVideoBufMngWriteIdx].buffer;
        P2PVideoBufMng[P2PVideoBufMngWriteIdx].flag   = KeyFrame;
        if(p2p_30PS == 1)
            asfPresentTime=asfPresentTime*2;
        P2PVideoBufMng[P2PVideoBufMngWriteIdx].time   = asfPresentTime *1000;
        *VideoStreamLength  = PayloadLength;

        if((VideoStreamBuf + PayloadLength) >= P2PPBVideBufEnd)
		{
			VideoStreamBuf = P2PPBVideoBuf;
		}
		
        memcpy(VideoStreamBuf, buf, PayloadLength);
    }
    /* Lsk : read video payload, and offset in media is 0 */
    else if(Offset2Media == 0 && *StartFrame == 0)
    {
        (*P2PasfVideoIndex)++;
        pBuf    = P2PVideoBufMng[P2PVideoBufMngWriteIdx].buffer;
		P2PVideoBufMngWriteIdx = (P2PVideoBufMngWriteIdx + 1) % (VIDEO_BUF_NUM-10);
		pBuf    = (u8*)(((u32)pBuf + (*VideoStreamLength) + 3) & ~3);
		if((pBuf + PayloadLength + MediaObjectSize) < P2PPBVideBufEnd) {
            P2PVideoBufMng[P2PVideoBufMngWriteIdx].buffer = pBuf;
        } else {
            P2PVideoBufMng[P2PVideoBufMngWriteIdx].buffer = P2PPBVideoBuf;
        }
        VideoStreamBuf      = P2PVideoBufMng[P2PVideoBufMngWriteIdx].buffer;

        P2PVideoBufMng[P2PVideoBufMngWriteIdx].flag   = KeyFrame;
        if(p2p_30PS == 1)
            asfPresentTime=asfPresentTime*2;
        P2PVideoBufMng[P2PVideoBufMngWriteIdx].time   = asfPresentTime *1000;
        *VideoStreamLength   = PayloadLength;
        memcpy(VideoStreamBuf, buf, PayloadLength);
    }
    /* Lsk : offset in media is not 0 */
    else if(*Restart1 == 0 || (*Restart1 == 1 && KeyFrame == 1))
    {
    	if((VideoStreamBuf + PayloadLength+ *VideoStreamLength) >= P2PPBVideBufEnd)
		{
			VideoStreamBuf = P2PPBVideoBuf;
		}
		
        memcpy(VideoStreamBuf + *VideoStreamLength, buf, PayloadLength);
        *VideoStreamLength    = (*VideoStreamLength) + PayloadLength;
    }
 
	if(*VideoStreamLength == MediaObjectSize)   //Lsk 090622 : Read a full frame, tell MPEG4 decode
	{
	//===== fixed TUTK app playback is not refer to frame time stamp 2014.03.07  start ===== 
//aher test
        time2 = (P2PVideoBufMng[P2PVideoBufMngWriteIdx].time - tmp1) / 1000;
    //    DEBUG_ASF("## 3 %d, %d \n",(u32)P2PVideoBufMng[P2PVideoBufMngWriteIdx].time,tmp1);
        //time2 = (P2PVideoBufMng[P2PVideoBufMngWriteIdx].time-P2PVideoBufMng[P2PVideoBufMngWriteIdx-1].time) / 1000; //每個 frame timer diff
    //    if(time2 < 50)
    //        {
    //        P2PVideoBufMng[P2PVideoBufMngWriteIdx].time=P2PVideoBufMng[P2PVideoBufMngWriteIdx].time*2;
    //        DEBUG_ASF("## 4 %d \n",(u32)P2PVideoBufMng[P2PVideoBufMngWriteIdx].time);
    //        }
        time1 = OSTimeGet();    // 系統時間
      #if NIC_SUPPORT        
        if(P2PPauseTime != 0)
        {
            //DEBUG_ASF("P2PPauseTime %d %d\n",time1* 50,P2PPauseTime* 50);
            time4=P2PPauseTime*50;
            //time3 += P2PPauseTime;
            P2PPauseTime = 0;
        }
      #endif
        //time1 = (time1-time4);
        if(p2p_play == 0)
        {
            time3 = time1 * 50 + time2+time4;     // 系統時間 + diff
            p2p_play=1;
    //        DEBUG_ASF(" %d\n",p2p_play);
        }
        time3 += (time2+time4);
        //DEBUG_ASF("%d, %d, %d %d \n",time2,time1* 50,time3,time4);
        while((time3 ) > (time1 * 50) && (P2PPlaybackVideoStop == 0))   //比較時間 送出時間是否大於系統時間
        {
            time1 = OSTimeGet();    // 更新系統時間
            OSTimeDly(1);
    //        DEBUG_ASF("g");
        }
//===== fixed TUTK app playback is not refer to frame time stamp 2014.03.07 End =====     
	    P2PVideoBufMng[P2PVideoBufMngWriteIdx].size   = *VideoStreamLength;
     //   dcfWrite(pVideoFile, VideoStreamBuf, P2PVideoBufMng[P2PVideoBufMngWriteIdx].size, &tmp);
	#if NIC_SUPPORT
		if(EnableStreaming)
	        OSSemPost(VideoRTPCmpSemEvt[0]);  
		#if TUTK_SUPPORT
		//DEBUG_ASF("P2PEnableplaybackStreaming =%d \n",P2PEnableplaybackStreaming);
		//OSTimeDly(1);
		if(P2PEnableplaybackStreaming) //Local_Record, Local_Playback
		{
			//while( P2PVideoPlaybackCmpSemEvt->OSEventCnt > 30 )
                //OSTimeDly(1);

			//P2P_check=1;
			OSSemPost(P2PVideoPlaybackCmpSemEvt);
            //DEBUG_ASF("g");
			while(P2PVideoPlaybackCmpSemEvt->OSEventCnt > 5)
			//while(P2P_check == 1)
			{
				OSTimeDly(1);
                if (P2PPlaybackVideoStop == 1)
                    break;
			}
		}
		#endif
	#endif        
	//	DEBUG_ASF("<%d>",video_cnt++);
//aher test
tmp1=P2PVideoBufMng[P2PVideoBufMngWriteIdx].time;	

	}
    return  VideoStreamBuf;
}

u8* asfSplitAudioPayload(u8      *buf,
                        u8      *AudioStreamBuf,
                        u8      *StartAudio,
                        u8      *Restart2,
                        u32     *AudioStreamLength,
                        u32     Offset2Media,
                        u32     PayloadLength,
                        u32     *asfAudioIndex,
                        u32     *AudioPlayback)
{
    // 第一次讀取Audio payload
    if(Offset2Media == 0 && (*StartAudio) == 1)
    {
        *StartAudio         = 0;
        *Restart2           = 0;
        AudioStreamBuf      = P2PiisSounBufMng[P2PiisSounBufMngWriteIdx].buffer;
        *AudioStreamLength  = PayloadLength;
        memcpy(AudioStreamBuf, buf, PayloadLength);

    }
    // 讀取Audio payload,除了一個檔案的第一個payload次之外(無offset時),所有Object的第一個payload都會走這邊.
    else if(Offset2Media == 0 && *StartAudio == 0)
    {
        (*asfAudioIndex)++;

        P2PiisSounBufMng[P2PiisSounBufMngWriteIdx].size   = *AudioStreamLength;
		P2PiisSounBufMngWriteIdx = (P2PiisSounBufMngWriteIdx + 1) % IIS_BUF_NUM;

        AudioStreamBuf      = P2PiisSounBufMng[P2PiisSounBufMngWriteIdx].buffer;
        *AudioStreamLength  = PayloadLength;
        memcpy(AudioStreamBuf, buf, PayloadLength);
    }
    // 除了所有Object的第一個payload之外,其餘payload會走到這邊,但通常一個audio object只有一個payload,所以不會跑到這裡.
    else if(*Restart2 == 0)
    {
        //memcpy(&(AudioStreamBuf[Offset2Media]), buf, PayloadLength);
        //*AudioStreamLength = Offset2Media + PayloadLength;
        memcpy(AudioStreamBuf + *AudioStreamLength, buf, PayloadLength);
        *AudioStreamLength  = (*AudioStreamLength) + PayloadLength;
    }
	P2PiisSounBufMng[P2PiisSounBufMngWriteIdx].size   = *AudioStreamLength;	
//	dcfWrite(pAudioFile, AudioStreamBuf, iisSounBufMng[iisSounBufMngWriteIdx].size, &tmp); 
#if NIC_SUPPORT
	if(EnableStreaming)
		OSSemPost(P2PAudioPlaybackCmpSemEvt);    
	#if TUTK_SUPPORT 
		if(P2PEnableplaybackStreaming) 
			OSSemPost(P2PAudioPlaybackCmpSemEvt);  
	#endif
#endif
    return  AudioStreamBuf;
}
s32 asfSplitFile(u32 u32PacketOffset)  //Lsk 120831 : asf file split to video/audio stream
{
    FS_FILE*                p2pFile;
    /* BJ: 0718 S*/
    u64                     UsedSize,PacketCount;
    u8                      flag2;
    ASF_HEADER_OBJECT       asfHeaderObject;
    ASF_DATA_OBJECT         asfDataObject;
    ASF_SIMPLE_INDEX_OBJECT asfSimpleIndexObject;

    ASF_FILE_PROPERTIES_OBJECT asfFilePropertiesObject;
    /*Payload Parsing Info*/
    u8  Mul_Payload = 0, PayloadFlag=0;
    u32 PacketLength, PaddingLength, SendTime, Offset2Media;//, MediaObjectNumber;
    u16 Duration;

    u32 PayloadLength, PacketUsedSize, VideoStreamLength, size;
    s64 asfPresentTime;
	u32 MediaObjectSize;  ////Lsk 090622
    u8  StreamNum, KeyFrame, VideoStreamNum, AudioStreamNum, PayloadNum,  Restart1, Restart2;
    u8  *buf;

    /* Peter 070104 */
    u32 P2PasfVideoIndex, asfAudioIndex, i;
    u8  err;
    ASF_AUDI_STREAM_PROPERTIES_OBJECT asfAudiStreamPropertiesObject;
#ifdef  ASF_AUDIO
    u8  *AudioStreamBuf;
    u32 AudioStreamLength;
    u8  StartAudio  = 1;//VopUsedByteSize,
#endif

    u8  StartFrame  = 1;//VopUsedByteSize,
    u8  *VideoStreamBuf;
    u8  P2PError       = 0;

	//u8  pre_playback_speed = 5;
	//u16 PlayIndex;
	//u8  KeyFramePacketCount;
	//u8 	GetFirstAudioPayload = 0;  //Lsk 090417 : AVSync timebase
    //u8  GetFirstVideoPayload = 1;  //Lsk 090417 : Fastward,Backward timebase
	u8  tmp;
	
	StartPlayBack = 0;
	ResetPlayback = 0;
	Video_timebase = 0;
	PacketCount.lo  = 0;

	P2PVideoBufMngWriteIdx=0;
	P2PiisSounBufMngWriteIdx=0;
    /* avoid compile warning */
    asfSimpleIndexObject    = asfSimpleIndexObject;
    PaddingLength           = PaddingLength;
    SendTime                = SendTime;
    Duration                = Duration;
    VideoStreamNum          = VideoStreamNum;
    AudioStreamNum          = AudioStreamNum;
    
    PacketCount.lo          = 0;
    if(u32PacketOffset) {
        Restart1    = 1;    // video restart flag
        Restart2    = 1;    // audio restart flag
    } else {
        Restart1    = 0;
        Restart2    = 0;
    }
    /* BJ: 0718 E*/
    p2p_play = 0;
    p2p_play_1 = 0;
    tmp1 =0;
    /* open file */
    if ((p2pFile = dcfOpen((signed char*)dcfPlaybackCurFile->pDirEnt->d_name, "r")) == NULL) {
        DEBUG_ASF("ASF open file error!!!\n");
        return 0;
    }
#if 0
    length = strlen(dcfPlaybackCurFile->pDirEnt->d_name);
    memcpy_hw(file_name, dcfPlaybackCurFile->pDirEnt->d_name, length);
    file_name[length-1] = '4';
    file_name[length-2] = '6';
    file_name[length-3] = '2';
    printf("pVideoFile = %s\n",file_name);
    if ((pVideoFile = dcfOpen(file_name, "r")) == NULL) {
        DEBUG_ASF("ASF open file error!!!\n");
    //    return 0;
    }
#endif

    for(i = 0; i < (VIDEO_BUF_NUM-10); i++) {
        P2PVideoBufMng[i].buffer   = P2PPBVideoBuf;
    }
    VideoStreamBuf          = (u8*)P2PVideoBufMng[0].buffer;
	for(i = 0; i < IIS_BUF_NUM; i++) {
		P2PiisSounBufMng[i].buffer = iisSounBuf[i];
    }
    AudioStreamBuf          = (u8*)P2PiisSounBufMng[0].buffer;

    if(dcfRead(p2pFile, P2Ptempbuf, ASF_HEADER_SIZE_CEIL, &size) != 1) {
        DEBUG_ASF("ASF read file error!!!\n");
        P2PError   = 1;
        goto ExitAsfReadFile;
    }
    buf = P2Ptempbuf;
    if(matchGuid(buf)!=ASF_Header_Object_Guid)  //Lsk 090817 file start must be ASF_Header_Object_Guid
    {
        P2PError   = 1;
        goto ExitAsfReadFile;
    }

    CloseFlag = 1;
    while(CloseFlag)
    {
        printf("@");
        switch(matchGuid(buf))
        {
            case ASF_Header_Object_Guid:
                buf = asfReadHeaderObject(buf,
                                          &asfHeaderObject,
                                          &asfFilePropertiesObject,
                                          &asfAudiStreamPropertiesObject,
                                          &VideoStreamNum,
                                          &AudioStreamNum);
                if(asfFilePropertiesObject.file_size.lo==0 && asfFilePropertiesObject.file_size.hi==0) //file size empty
                {
                    P2PError   = 1;
                   goto ExitAsfReadFile;
                }

                if(buf == 0) {
                    DEBUG_ASF("asfReadHeaderObject error!!!\n");
                    P2PError   = 1;
                    goto ExitAsfReadFile;
                }
                break;
            case ASF_Data_Object_Guid:
                //DEBUG_ASF("Match ASF_Data_Object_Guid\n");
                memcpy(&asfDataObject,buf,sizeof(ASF_DATA_OBJECT));
                buf += sizeof(ASF_DATA_OBJECT);
                UsedSize.lo = sizeof(ASF_DATA_OBJECT);

                while(P2PPlaybackVideoStop == 0)
                {
                    printf("#");
                    if(dcfRead(p2pFile, P2Ptempbuf, asfFilePropertiesObject.maximum_data_packet_size , &size) == 0) {
                        P2PError                = 1;
                        P2PPlaybackVideoStop    = 1;
                        break;
                    }
                    buf = P2Ptempbuf;

                    buf =  asfReadPacketHeader(buf,
                                               &asfFilePropertiesObject,
                                               &PacketUsedSize,
                                               &flag2,
                                               &Mul_Payload,
                                               &PacketLength,
                                               &PaddingLength,//Lsk 090304
                                               &UsedSize);
                    if(buf == 0) {
                        P2PError                = 1;
                        P2PPlaybackVideoStop    = 1;
                        break;
                    }

                    if(Mul_Payload)
                    {
                    	PayloadFlag = *((u8 *)buf);
                        PayloadNum = (PayloadFlag & 0x3F);
                        buf += 1; // Payload Flag
                        UsedSize.lo += 1; // Payload Flag
                    }
                    else
                    	PayloadNum = 1;

                    while(PayloadNum != 0)
                    {
                    	PayloadNum--;
                        buf = asfReadPayloadHeader(buf,
												   &StreamNum,
                                                   &KeyFrame,
                                                   flag2,
                                                   Restart1,
                                                   Restart2,
                                                   PayloadFlag,
                                                   &Offset2Media,
                                                   &PayloadLength,
                                                   &MediaObjectSize,   //Lsk 090622
                                                   &asfPresentTime,
                                                   &P2PasfVideoIndex,
                                                   &asfAudioIndex,
                                                   &UsedSize,
                                                   Mul_Payload,    //Lsk 090304
                                                   PacketLength,
                                                   PaddingLength);
							P2P_playback_go=1;

                        if(StreamNum == 1 ) // video stream
                        {
							VideoStreamBuf  = asfSplitVideoPayload(buf,
                                                                  VideoStreamBuf,
                                                                  KeyFrame,
                                                                  &StartFrame,
                                                                  &Restart1,
                                                                  &VideoStreamLength,
                                                                  Offset2Media,
                                                                  PayloadLength,
                                                                  MediaObjectSize,   //Lsk 090622
                                                                  asfPresentTime,
                                                                  &P2PasfVideoIndex,
                                                                  &mpeg4taskrun,
                                                                  &u32PacketOffset,
                                                                  &UsedSize,
                                                                  p2pFile,
                                                                  &asfFilePropertiesObject,
                                                                  &PacketCount);

                        }
                        else if(StreamNum == 2 && sysPlaybackForward == 0) // audio stream
                        {
                        	AudioStreamBuf  = asfSplitAudioPayload(buf,
                                                                  AudioStreamBuf,
                                                                  &StartAudio,
                                                                  &Restart2,
                                                                  &AudioStreamLength,
                                                                  Offset2Media,
                                                                  PayloadLength,
                                                                  &asfAudioIndex,
                                                                  &AudioPlayback);

                        }
                        buf += PayloadLength;
                        UsedSize.lo += PayloadLength;
                    }
                    PacketCount.lo++;
                    PacketUsedSize = ((u32)buf) - PacketUsedSize;
                    if(PacketUsedSize != PacketLength)
                    {
                        buf += PacketLength - PacketUsedSize;
                        UsedSize.lo += PacketLength - PacketUsedSize;
                    }
                }
				CloseFlag   = 0;
                break;
            case NoMatch_Guid:
            default:
                break;
        }
    }
        /* BJ: 0718 E*/

ExitAsfReadFile:


    DEBUG_ASF("ASF split file finish!!!\n");
    /* close file */

    if(dcfClose(p2pFile, &tmp) == 0) {
        DEBUG_ASF("dcfClose() error!!!\n");
    }
    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_FINISH, OS_FLAG_CLR, &err);

#if 0	
    if(dcfClose(pVideoFile) == 0) {
        DEBUG_ASF("dcfClose() error!!!\n", err);
    }
#endif
	if(P2PError)
		return 0;

	return 1;
}
#endif
#ifdef ASF_AUDIO

/*

Routine Description:

    Audio initialization.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfAudioInit(void)
{
    if(IISMode == 1)
        Audio_formate = Audio_formate_Out;
    else
        Audio_formate = Audio_formate_In;

#if (AUDIO_CODEC == AUDIO_CODEC_PCM)

    asfAudiFormat.codec_id_format_tag       = ASF_WAVE_FORMAT_PCM;  /* PCM */
    asfAudiFormat.number_of_channels        = 0x0001;               /* 1 channel */
  #if(IIS_SAMPLE_RATE == 8000)
    asfAudiFormat.samples_per_second        = 0x00001f40;          /* 8000 sample/sec */
    asfAudiFormat.avg_num_of_bytes_per_sec  = 0x00001f40;          /* 8000 byte/sec */
  #elif(IIS_SAMPLE_RATE == 16000)
    asfAudiFormat.samples_per_second        = 0x00003e80;          /* 16000 sample/sec */
    asfAudiFormat.avg_num_of_bytes_per_sec  = 0x00003e80;          /* 16000 byte/sec */
  #endif
    asfAudiFormat.block_alignment           = 0x0001;               /* 1 byte/sample */
    asfAudiFormat.bits_per_sample           = 0x0008;               /* 8 bit/sample */

#elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)

    IMA_ADPCM_Init();
    asfAudiFormat.codec_id_format_tag       = ASF_WAVE_FORMAT_DVI_ADPCM;    /* IMA_ADPCM */
    asfAudiFormat.number_of_channels        = 0x0001;                       /* 1 channel */
  #if(IIS_SAMPLE_RATE == 8000)
    asfAudiFormat.samples_per_second        = 0x00001f40;          /* 8000 sample/sec */
    asfAudiFormat.avg_num_of_bytes_per_sec  = 0x00000fa0;          /* 4000 byte/sec */
  #elif(IIS_SAMPLE_RATE == 16000)
    asfAudiFormat.samples_per_second        = 0x00003e80;          /* 16000 sample/sec */
    asfAudiFormat.avg_num_of_bytes_per_sec  = 0x00001f40;          /* 8000 byte/sec */
  #endif
    asfAudiFormat.block_alignment           = IMA_ADPCM_BLOCK_SIZE;         /* bytes per block */
    asfAudiFormat.bits_per_sample           = 0x0004;               /* 4 bit/sample */
    switch (Audio_formate) {
        case nomo_8bit_8k:
        case nomo_8bit_16k:
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK;
            break;
        case nomo_16bit_8k:
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK * 2;
            break;
        default:
            DEBUG_ASF("Don't support Audio_formate %d\n", Audio_formate);
    }
    iisBufMngTemp.buffer                    = (u8*)iisBufTemp;

#endif

    return 1;
}

#endif

/*-------------------------------------------------------------------------*/
/* Header object                               */
/*-------------------------------------------------------------------------*/

/*

Routine Description:

    Write header object.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
#if( ((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) || (!MULTI_CHANNEL_VIDEO_REC))
s32 asfWriteHeaderObject(FS_FILE* pFile)
{
    //u32 size;
    u32 hdrSize;
    __align(4) ASF_HEADER_OBJECT  asfHeaderObject =
    {
        {0x30, 0x26, 0xb2, 0x75,    /* object_id = ASF_Header_Object */
         0x8e, 0x66, 0xcf, 0x11,
         0xa6, 0xd9, 0x00, 0xaa,
         0x00, 0x62, 0xce, 0x6c},
        {0x00000000, 0x00000000},   /* object_size = 0x???????????????? */
        0x00000007,         /* number_of_header_object = 0x00000007 (video only)        */
                        /*               0x00000008 (video and audio)   */
        0x01,               /* reserved1 = 0x01 */
        0x02,               /* reserved2 = 0x02 */
    };

    #if (VIDEO_CODEC_OPTION == H264_CODEC)
    asfVideHeaderSize=0;
    #elif (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    mpeg4EncodeVolHeader(asfVideHeader, &asfVideHeaderSize);   /* Peter: 0711 */
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
    asfVideHeaderSize=0;
    #endif

    hdrSize =
        sizeof(ASF_HEADER_OBJECT) +
        sizeof(ASF_FILE_PROPERTIES_OBJECT) +
        sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT) + asfVideHeaderSize +
#ifdef ASF_AUDIO
        sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT) +
#endif
        #if( (VIDEO_CODEC_OPTION == H264_CODEC)||(VIDEO_CODEC_OPTION == MPEG4_CODEC))
        sizeof(ASF_HEADER_EXTENSION_OBJECT) + sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS) + //Lsk 090515
        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
        sizeof(ASF_HEADER_EXTENSION_OBJECT) +
        #endif
        sizeof(ASF_CODEC_LIST_OBJECT) +
        sizeof(ASF_HDR_VIDE_CODEC_ENTRY) +
#ifdef ASF_AUDIO
        sizeof(ASF_HDR_AUDI_CODEC_ENTRY) +
#endif
        sizeof(ASF_CONTENT_DESCRIPTION_OBJECT) +
        sizeof(ASF_HDR_PADDING_OBJECT);
    asfHeaderPaddingSize = ASF_HEADER_SIZE_CEIL - hdrSize - sizeof(ASF_DATA_OBJECT);
    asfHeaderObject.object_size.lo =
        asfHeaderSize =
        hdrSize + asfHeaderPaddingSize;
#ifdef ASF_AUDIO
    //asfHeaderObject.number_of_header_object = 0x00000008;
    asfHeaderObject.number_of_header_object = 0x00000007;   /* Peter 070104 */
#else
    //asfHeaderObject.number_of_header_object = 0x00000007;
    asfHeaderObject.number_of_header_object = 0x00000006;   /* Peter 070104 */
#endif

#if ASF_MASS_WRITE_HEADER  /* Peter 070104 */
    CopyMemory(asfMassWriteData, &asfHeaderObject, sizeof(ASF_HEADER_OBJECT));
    asfMassWriteDataPoint   = asfMassWriteData + sizeof(ASF_HEADER_OBJECT);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfHeaderObject, sizeof(ASF_HEADER_OBJECT), &size) == 0)
            return 0;
#endif

    if (asfWriteFilePropertiesObjectPre(pFile) == 0)
            return 0;
    if (asfWriteVideStreamPropertiesObject(pFile) == 0)
            return 0;
#ifdef ASF_AUDIO
    if (asfWriteAudiStreamPropertiesObject(pFile) == 0)
            return 0;
#endif
    if (asfWriteHeaderExtensionObject(pFile) == 0)
            return 0;
    if (asfWriteCodecListObject(pFile) == 0)
            return 0;
    if (asfWriteContentDescriptionObject(pFile) == 0)
            return 0;
    if (asfWriteHdrPaddingObject(pFile) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write file properties object pre.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteFilePropertiesObjectPre(FS_FILE* pFile)
{
    //u32 size;
    __align(4) ASF_FILE_PROPERTIES_OBJECT asfFilePropertiesObject =
    {
        {0xa1, 0xdc, 0xab, 0x8c,    /* object_id = ASF_File_Properties_Object */
         0x47, 0xa9, 0xcf, 0x11,
         0x8e, 0xe4, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x00000068, 0x00000000},   /* object_size = 0x0000000000000068 */
        {0x00, 0x00, 0x00, 0x00,    /* file_id = file_id of asf_data_object and asf_simple_index_object */
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00},
        {0x00000000, 0x00000000},   /* file_size = 0x???????????????? */
        {0xffffffff, 0xffffffff},   /* creation_date = 0x???????????????? */
        {0x00000000, 0x00000000},   /* data_packets_count= 0x???????????????? */
        {0x00000000, 0x00000000},   /* play_duration = 0x???????????????? in unit of 100 nanosecond */
        {0x00000000, 0x00000000},   /* send_duration = 0x???????????????? in unit of 100 nanosecond */
        {0x00000bb8, 0x00000000},   /* preroll = 0x0bb8000000000000 */  //Lsk : preroll 3sec
        0x00000002,         /* flags = 0x02, BroadcastFlag = 0, SeekableFlag = 1 */
        0x00000800,         /* minimum_data_packet_size = 0x???????? */
        0x00000800,         /* maximum_data_packet_size = 0x???????? */
        //0x00012c00,           /* maximum_bitrate = 384K * 2 */
        0x002547AE,         /* maximum_bitrate = 0 kbps, for Media Player 6.4 */ /* Peter 070104 */
    };

    asfFilePropertiesObject.minimum_data_packet_size =
        asfFilePropertiesObject.maximum_data_packet_size =
            ASF_DATA_PACKET_SIZE;
    asfFilePropertiesObject.object_size.lo = sizeof(ASF_FILE_PROPERTIES_OBJECT);

#if ASF_MASS_WRITE_HEADER
    CopyMemory(asfMassWriteDataPoint, &asfFilePropertiesObject, sizeof(ASF_FILE_PROPERTIES_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_FILE_PROPERTIES_OBJECT);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfFilePropertiesObject, sizeof(ASF_FILE_PROPERTIES_OBJECT), &size) == 0)
            return 0;
#endif

    return 1;
}
#endif //( ((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) || (!MULTI_CHANNEL_VIDEO_REC))
/*

Routine Description:

    Write file properties object post.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/

s32 asfWriteFilePropertiesObjectPost(FS_FILE* pFile)
{
    u32 size;
    u32 offset;
    __align(4) ASF_FILE_PROPERTIES_OBJECT_POST asfFilePropertiesObjectPost =
    {
        {0x00000000, 0x00000000},   /* file_size = 0x???????????????? */
        {0xb22f4000, 0x01c3d255},   /* creation_date = 00:00:00 2004/01/01 */
        {0x00000000, 0x00000000},   /* data_packets_count= 0x???????????????? */
        {0x00000000, 0x00000000},   /* play_duration = 0x???????????????? in unit of 100 nanosecond */
        {0x00000000, 0x00000000},   /* send_duration = 0x???????????????? in unit of 100 nanosecond */
    };

    offset = dcfTell(pFile);

    dcfSeek(pFile, 0x46, FS_SEEK_SET);

    asfDataSize =
        sizeof(ASF_DATA_OBJECT) +
        asfDataPacketCount * ASF_DATA_PACKET_SIZE;
    asfIndexSize =
        sizeof(ASF_SIMPLE_INDEX_OBJECT) +
        asfIndexTableIndex * sizeof(ASF_IDX_SIMPLE_INDEX_ENTRY);
    asfFilePropertiesObjectPost.file_size.lo = asfHeaderSize + asfDataSize + asfIndexSize;
    asfFilePropertiesObjectPost.data_packets_count.lo = asfDataPacketCount;
    //Lsk 090309
    asfFilePropertiesObjectPost.play_duration.lo = asfVidePresentTime * 10000;
    asfFilePropertiesObjectPost.play_duration.hi = ((asfVidePresentTime >> 4) * (10000 >> 4)) >> 24;

    asfFilePropertiesObjectPost.send_duration.lo = (asfVidePresentTime - PREROLL) * 10000;
    asfFilePropertiesObjectPost.send_duration.hi = (((asfVidePresentTime - PREROLL) >> 4) * (10000 >> 4)) >> 24;
    /*** encrypt File ***/

	#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
    if (dcfBackupWrite(pFile, (unsigned char*)&asfFilePropertiesObjectPost, sizeof(ASF_FILE_PROPERTIES_OBJECT_POST), &size) == 0)
	   return 0;
	#else
    if (dcfWrite(pFile, (unsigned char*)&asfFilePropertiesObjectPost, sizeof(ASF_FILE_PROPERTIES_OBJECT_POST), &size) == 0)
        return 0;
	#endif
    dcfSeek(pFile, offset, FS_SEEK_SET);

    return 1;
}

/*

Routine Description:

    Write video stream properties object.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
#if( ((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) || (!MULTI_CHANNEL_VIDEO_REC))
#if (VIDEO_CODEC_OPTION == H264_CODEC)
s32 asfWriteVideStreamPropertiesObject(FS_FILE* pFile)
{
    //u32 size;
    __align(4) ASF_VIDE_STREAM_PROPERTIES_OBJECT asfVideStreamPropertiesObject =
    {
        {0x91, 0x07, 0xdc, 0xb7,    /* object_id = ASF_Stream_Properties_Object */
         0xb7, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000009e, 0x00000000},   /* object_size */
        {0xc0, 0xef, 0x19, 0xbc,    /* steam_type = ASF_Video_Media */
         0x4d, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00, 0x57, 0xfb, 0x20,    /* error_correction_type = ASF_No_Error_Correction */
         0x55, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00000000, 0x00000000},   /* time_offset */
        0x00000050,         /* type_specific_data_length */
        0x00000000,         /* error_correction_data_length */
        0x0001,             /* flags = 0x0001, StreamNumber = 1, EncryptedContentFlag = 0 */
        0x00000000,         /* reserved */
        {
            0x00000000,         /* encoded_image_width = 0x???????? */
            0x00000000,         /* encoded_image_height = 0x???????? */
            0x02,               /* reserved_flags */
            0x0045,             /* format_data_size */
            {
                0x00000045,         /* format_data_size */
                0x00000000,         /* image_width = 0x???????? */
                0x00000000,         /* image_height = 0x???????? */
                0x0001,             /* reserved */
                0x0018,             /* bits_per_pixel_count */
                0x34363248,         /* compression_id = "462H" (H264) */
                0x00000000,         /* image_size = 0x???????? = image_width * image_height * 3 */
                0x00000000,         /* horz_pixels_per_meter */
                0x00000000,         /* vert_pixels_per_meter */
                0x00000000,         /* colors_used_count */
                0x00000000,         /* important_colors_count */                
            },
        },
    };
    asfVideStreamPropertiesObject.object_size.lo =
        sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT) + asfVideHeaderSize;
    asfVideStreamPropertiesObject.type_specific_data_length =
        sizeof(ASF_HDR_VIDE_TYPE_SPECIFIC_DATA) + asfVideHeaderSize;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.format_data_size =
        (u16)(sizeof(ASF_HDR_VIDE_FORMAT_DATA) + asfVideHeaderSize);
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.format_data_size =
        sizeof(ASF_HDR_VIDE_FORMAT_DATA) + asfVideHeaderSize;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_width =

        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_width =
            asfVopWidth;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_height =
        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_height =
            asfVopHeight;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_size =
        asfVopWidth * asfVopHeight * 3;

#if ASF_MASS_WRITE_HEADER   /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT);
    CopyMemory(asfMassWriteDataPoint, &asfVideHeader, asfVideHeaderSize);
    asfMassWriteDataPoint  += asfVideHeaderSize;
#else
    if (dcfWrite(pFile, (unsigned char*)&asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT), &size) == 0)
        return 0;
    if (dcfWrite(pFile, (unsigned char*)&asfVideHeader, asfVideHeaderSize, &size) == 0)
        return 0;
#endif

    return 1;
}
#elif (VIDEO_CODEC_OPTION == MPEG4_CODEC)
s32 asfWriteVideStreamPropertiesObject(FS_FILE* pFile)
{
    u32 size;
    __align(4) ASF_VIDE_STREAM_PROPERTIES_OBJECT asfVideStreamPropertiesObject =
    {
        {0x91, 0x07, 0xdc, 0xb7,    /* object_id = ASF_Stream_Properties_Object */
         0xb7, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000009e, 0x00000000},   /* object_size */
        {0xc0, 0xef, 0x19, 0xbc,    /* steam_type = ASF_Video_Media */
         0x4d, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00, 0x57, 0xfb, 0x20,    /* error_correction_type = ASF_No_Error_Correction */
         0x55, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00000000, 0x00000000},   /* time_offset */
        0x00000050,         /* type_specific_data_length */
        0x00000000,         /* error_correction_data_length */
        0x0001,             /* flags = 0x0001, StreamNumber = 1, EncryptedContentFlag = 0 */
        0x00000000,         /* reserved */
        {
            0x00000000,         /* encoded_image_width = 0x???????? */
            0x00000000,         /* encoded_image_height = 0x???????? */
            0x02,               /* reserved_flags */
            0x0045,             /* format_data_size */
            {
                0x00000045,         /* format_data_size */
                0x00000000,         /* image_width = 0x???????? */
                0x00000000,         /* image_height = 0x???????? */
                0x0001,             /* reserved */
                0x0018,             /* bits_per_pixel_count */
                0x3253344d,         /* compression_id = "2S4M" (M4S2) */
                0x00000000,         /* image_size = 0x???????? = image_width * image_height * 3 */
                0x00000000,         /* horz_pixels_per_meter */
                0x00000000,         /* vert_pixels_per_meter */
                0x00000000,         /* colors_used_count */
                0x00000000,         /* important_colors_count */
                0x00, 0x00, 0x01, 0xb0,     /* codec_specific_data[0x0a] */
                //0x01,
                0x03,                       /* Simple profile level 3 */ /* Peter 070104 */
                0x00, 0x00, 0x01, 0xb5,
                0x09,
                        /* 0x00, 0x00, 0x00, 0x00, */
                /* 0x00, 0x00, 0x00, 0x00, */
                /* 0x00, 0x00, 0x00, 0x00, */
                /* 0x00, 0x00, 0x00, 0x00, */
                /* 0x00, 0x00, 0x00,       */
            },
        },
    };
    asfVideStreamPropertiesObject.object_size.lo =
        sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT) + asfVideHeaderSize;
    asfVideStreamPropertiesObject.type_specific_data_length =
        sizeof(ASF_HDR_VIDE_TYPE_SPECIFIC_DATA) + asfVideHeaderSize;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.format_data_size =
        (u16)(sizeof(ASF_HDR_VIDE_FORMAT_DATA) + asfVideHeaderSize);
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.format_data_size =
        sizeof(ASF_HDR_VIDE_FORMAT_DATA) + asfVideHeaderSize;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_width =

        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_width =
            asfVopWidth;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_height =
        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_height =
            asfVopHeight;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_size =
        asfVopWidth * asfVopHeight * 3;

#if ASF_MASS_WRITE_HEADER   /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT);
    CopyMemory(asfMassWriteDataPoint, &asfVideHeader, asfVideHeaderSize);
    asfMassWriteDataPoint  += asfVideHeaderSize;
#else
    if (dcfWrite(pFile, (unsigned char*)&asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT), &size) == 0)
        return 0;
    if (dcfWrite(pFile, (unsigned char*)&asfVideHeader, asfVideHeaderSize, &size) == 0)
        return 0;
#endif

    return 1;
}
#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
s32 asfWriteVideStreamPropertiesObject(FS_FILE* pFile)
{
#if(ASF_FORMATE==0)
    u32 size;
    __align(4) ASF_VIDE_STREAM_PROPERTIES_OBJECT asfVideStreamPropertiesObject =
    {
        {0x91, 0x07, 0xdc, 0xb7,    /* object_id = ASF_Stream_Properties_Object */
         0xb7, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000009e, 0x00000000},   /* object_size */
        /*** steam_type test start***/
        /***  Lsk pc : media player 11 ok, media player 6.4 ok,
            other pc : fail ***/
        #if 1
        {0xc0, 0xef, 0x19, 0xbc,    /* steam_type = ASF_Video_Media */
         0x4d, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        #if 0
		{0xe0, 0x7d, 0x90,0x35,		 //steam_type = ASF_degradable_JPEG_Media
         0x15, 0xe4, 0xcf, 0x11,
         0xa9, 0x17, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        /***  Lsk pc : media player 9 no video, media player 6.4 no video,
          ivy/Lab pc : media player 9 OK, media player 6.4 bug message,***/
		#if 0
		{0x00, 0xe1,0x1b,0xb6,		 //steam_type = ASF_JFIF_Media
         0x4e, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd,0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        /*** steam_type test end***/
        {0x00, 0x57, 0xfb, 0x20,    /* error_correction_type = ASF_No_Error_Correction */
         0x55, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00000000, 0x00000000},   /* time_offset */
        0x00000000,         /* type_specific_data_length */
        0x00000000,         /* error_correction_data_length */
        0x0001,             /* flags = 0x0001, StreamNumber = 1, EncryptedContentFlag = 0 */
        0x00000000,         /* reserved */
        /* type specific data */
        {
            0x00000000,         /* encoded_image_width = 0x???????? */
            0x00000000,         /* encoded_image_height = 0x???????? */
            0x02,               /* reserved_flags */
            0x0045,             /* format_data_size */
            {
                0x00000045,         /* format_data_size */
                0x00000000,         /* image_width = 0x???????? */
                0x00000000,         /* image_height = 0x???????? */
                0x0001,             /* reserved */
                0x0018,             /* bits_per_pixel_count */

                /* compression_id =
	            0x67706a6d,         //mjpg
	            0x47504a4d,         //MJPG
   	            0x47504a6d,         //mJPG
                0x4649464a,         //JFIF
                0x4745504a,         //JPEG
                */
	            0x47504a4d,         //MJPG

                0x00000000,         /* image_size = 0x???????? = image_width * image_height * 3 */
                0x00000000,         /* horz_pixels_per_meter */
                0x00000000,         /* vert_pixels_per_meter */
                0x00000000,         /* colors_used_count */
                0x00000000,         /* important_colors_count */
            },
        },
    };
    asfVopWidth  = MJPEG_WIDTH;
	asfVopHeight = MJPEG_HEIGHT;

    asfVideStreamPropertiesObject.object_size.lo =
        sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT) + asfVideHeaderSize;
    asfVideStreamPropertiesObject.type_specific_data_length =
        sizeof(ASF_HDR_VIDE_TYPE_SPECIFIC_DATA) + asfVideHeaderSize;

    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.format_data_size =
        (u16)(sizeof(ASF_HDR_VIDE_FORMAT_DATA) + asfVideHeaderSize);



    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.format_data_size =
        sizeof(ASF_HDR_VIDE_FORMAT_DATA) + asfVideHeaderSize;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_width =

        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_width =
            asfVopWidth;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_height =
        asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_height =
            asfVopHeight;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.asf_hdr_vide_format_dta.image_size =
        asfVopWidth * asfVopHeight * 3;
#endif

#if(ASF_FORMATE==1)
    u32 size;
    __align(4) ASF_VIDE_STREAM_PROPERTIES_OBJECT asfVideStreamPropertiesObject =
    {
        {0x91, 0x07, 0xdc, 0xb7,    /* object_id = ASF_Stream_Properties_Object */
         0xb7, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000009e, 0x00000000},   /* object_size */
        /*** steam_type test start***/
        /***  Lsk pc : media player 11 ok, media player 6.4 ok,
            other pc : fail ***/
        #if 0
        {0xc0, 0xef, 0x19, 0xbc,    /* steam_type = ASF_Video_Media */
         0x4d, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        #if 0
		{0xe0, 0x7d, 0x90,0x35,		 //steam_type = ASF_degradable_JPEG_Media
         0x15, 0xe4, 0xcf, 0x11,
         0xa9, 0x17, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        /***  Lsk pc : media player 9 no video, media player 6.4 no video,
          ivy/Lab pc : media player 9 OK, media player 6.4 bug message,***/
		#if 1
		{0x00, 0xe1,0x1b,0xb6,		 //steam_type = ASF_JFIF_Media
         0x4e, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd,0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        #endif
        /*** steam_type test end***/
        {0x00, 0x57, 0xfb, 0x20,    /* error_correction_type = ASF_No_Error_Correction */
         0x55, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x00000000, 0x00000000},   /* time_offset */
        0x00000000,         /* type_specific_data_length */
        0x00000000,         /* error_correction_data_length */
        0x0001,             /* flags = 0x0001, StreamNumber = 1, EncryptedContentFlag = 0 */
        0x00000000,         /* reserved */
        /* type specific data */
        {
            0x00000000,         /* encoded_image_width = 0x???????? */
            0x00000000,         /* encoded_image_height = 0x???????? */
            0x00000000,               /* reserved_flags */
        },
    };
    asfVopWidth  = MJPEG_WIDTH;
	asfVopHeight = MJPEG_HEIGHT;

    asfVideStreamPropertiesObject.object_size.lo =
        sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT);
    asfVideStreamPropertiesObject.type_specific_data_length =
        sizeof(ASF_HDR_VIDE_TYPE_SPECIFIC_DATA);
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_width =
        asfVopWidth;
    asfVideStreamPropertiesObject.asf_hdr_vide_type_specific_dta.encoded_image_height =
        asfVopHeight;
#endif


#if ASF_MASS_WRITE_HEADER   /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfVideStreamPropertiesObject, sizeof(ASF_VIDE_STREAM_PROPERTIES_OBJECT), &size) == 0)
            return 0;
    if (dcfWrite(pFile, (unsigned char*)&asfVideHeader, asfVideHeaderSize, &size) == 0)
            return 0;
#endif

    return 1;
}
#endif
#ifdef ASF_AUDIO

/*

Routine Description:

    Write audio stream properties object.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteAudiStreamPropertiesObject(FS_FILE* pFile)
{
    //u32 size;
    __align(4) ASF_AUDI_STREAM_PROPERTIES_OBJECT asfAudiStreamPropertiesObject =
    {
        {0x91, 0x07, 0xdc, 0xb7,    /* object_id = ASF_Stream_Properties_Object */
         0xb7, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x00000068, 0x00000000},   /* object_size */
        {0x40, 0x9e, 0x69, 0xf8,    /* steam_type = ASF_Audio_Media */
         0x4d, 0x5b, 0xcf, 0x11,
         0xa8, 0xfd, 0x00, 0x80,
         0x5f, 0x5c, 0x44, 0x2b},
        {0x50, 0xcd, 0xc3, 0xbf,    /* error_correction_type = ASF_No_Audio_Spread */
         0x8f, 0x61, 0xcf, 0x11,
         0x8b, 0xb2, 0x00, 0xaa,
         0x00, 0xb4, 0xe2, 0x20},
        {0x00000000, 0x00000000},   /* time_offset */
        0x00000012,         /* type_specific_data_length */
        0x00000008,         /* error_correction_data_length */
        0x0002,             /* flags = 0x0001, StreamNumber = 2, EncryptedContentFlag = 0 */
        0x00000000,         /* reserved */
        {
            0x0001,             /* codec_id_format_tag = 0x0001 (ASF_WAVE_FORMAT_PCM) */
            0x0001,             /* number_of_channels = 0x0001 channel */
            0x00001f40,         /* samples_per_second = 0x00001F40 = 8000 sample/sec */
            0x00001f40,         /* avg_num_of_bytes_per_sec = 8000 byte/sec */
            0x0001,             /* block_alignment = 0x0001 byte */
            0x0008,             /* bits_per_sample = 0x0008 bit */
#if (AUDIO_CODEC == AUDIO_CODEC_PCM)
            0x0000,             /* codec_specific_data_size */
                                /* codec_specific_data[0x00] */
#elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
            0x0002,             /* codec_specific_data_size */
            IMA_ADPCM_SAMPLE_PER_BLOCK, /* wSamplesPerBlock */
#endif
        },
        {
            0x01,               /* span */
            0x00c8,             /* virtual_packet_length */
            0x00c8,             /* virtual_chunk_length */
            0x0001,             /* silence_data_length */
            0x00,               /* silence_data */
        },
    };

    asfAudiStreamPropertiesObject.object_size.lo =
        sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT);
    asfAudiStreamPropertiesObject.type_specific_data_length =
        sizeof(ASF_HDR_AUDI_TYPE_SPECIFIC_DATA);
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.codec_id_format_tag =
        asfAudiFormat.codec_id_format_tag;
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.number_of_channels =
        asfAudiFormat.number_of_channels;
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.samples_per_second =
        asfAudiFormat.samples_per_second;
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.avg_num_of_bytes_per_sec =
        asfAudiFormat.avg_num_of_bytes_per_sec;
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.block_alignment =
        asfAudiFormat.block_alignment;
    asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.bits_per_sample =
        asfAudiFormat.bits_per_sample;
    asfAudiStreamPropertiesObject.asf_hdr_audi_error_correction_dta.virtual_packet_length =
        asfAudiStreamPropertiesObject.asf_hdr_audi_error_correction_dta.virtual_chunk_length =
            //ASF_AUDIO_VIRTUAL_PACKET_SIZE;
            IIS_CHUNK_SIZE; /* Peter 070104 */

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfAudiStreamPropertiesObject, sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfAudiStreamPropertiesObject, sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT), &size) == 0)
        return 0;
#endif

    return 1;
}

#endif

/*

Routine Description:

    Write header extension object.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
#if (VIDEO_CODEC_OPTION == H264_CODEC)
s32 asfWriteHeaderExtensionObject(FS_FILE* pFile)
{
    //u32 size;
    __align(4) ASF_HEADER_EXTENSION_OBJECT asfHeaderExtensionObject =
    {
        {0xb5, 0x03, 0xbf, 0x5f,    /* object_id = ASF_Header_Extension_Object */
         0x2e, 0xa9, 0xcf, 0x11,
         0x8e, 0xe3, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000002e, 0x00000000},   /* object_size */
        {0x11, 0xd2, 0xd3, 0xab,    /* reserved_field_1 = ASF_Reserved_1 */
         0xba, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        0x0006,             /* reserved_field_2 */
        0x00000000,         /* header_extension_data_size */
    };


    __align(4) ASF_HDR_EXT_STREAM_PROPERTY asfHeaderExtStreamPropertyObject =
    {
        {0xCB, 0xA5, 0xE6, 0x14, 	/* object_id = ASF_Extended_Stream_Properties_Object */
         0x72, 0xC6, 0x32, 0x43,
         0x83, 0x99, 0xA9, 0x69,
         0x52, 0x06, 0x5B, 0x5A},
        {0x00000000, 0x00000000},   /* object_size */
		{0x00000000, 0x00000000},   /* start_time */
		{0x00000000, 0x00000000},   /* end_time */
		 0x00000000,				/* data_bitrate = 350000 */
		 //0x004c4b40,				/* data_bitrate = 350000 */
		 0x00000bb8,				/* buffer_size */
		 0x00000000,				/* initial_buffer_fullness */
		 0x00000000,
		 //0x004c4b40,				/* alternate_data_bitrate */
		 0x00000bb8,				/* alternate_buffer_size */
		 0x00000000,				/* alternate_initial_buffer_fullness */
		 0x00000000,
		 //0x00008b11,				/* max_object_size */
		 //0xffffffff,				/* max_object_size */
		 0x00000002,				/* flag */
		 0x0001,					/* stream_num */
		 0x0000,					/* stream_language */
		 {0x00051615, 0x00000000},  /* average_time_per_frame */
		 0x0000,					/* stream_name_count */
		 0x0001,					/* payload_ext_sys_cnt */
	};

	__align(4) ASF_PAYLOAD_EXT_SYSTEMS asfPayloadExtSystem =
	{
		{0x20, 0xDC, 0x90, 0xD5,    /* object_id = ASF_Payload_Extension_System_Content_Type    */
         0xBC, 0x07, 0x6C, 0x43,
         0x9C, 0xF7, 0xF3, 0xBB,
         0xFB, 0xF1, 0xA4, 0xDC},
         0x0001,					/* data_size */
         0x00000000, 				/* info_length */
	};
    asfHeaderExtensionObject.object_size.lo = sizeof(ASF_HEADER_EXTENSION_OBJECT) + sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);
	asfHeaderExtensionObject.header_extension_data_size = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

	asfHeaderExtStreamPropertyObject.object_size.lo = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_HEADER_EXTENSION_OBJECT);

	CopyMemory(asfMassWriteDataPoint, &asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY));
    asfMassWriteDataPoint  += sizeof(ASF_HDR_EXT_STREAM_PROPERTY);

	CopyMemory(asfMassWriteDataPoint, &asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS));
    asfMassWriteDataPoint  += sizeof(ASF_PAYLOAD_EXT_SYSTEMS);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT), &size) == 0)
        return 0;
	if (dcfWrite(pFile, (unsigned char*)&asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY), &size) == 0)
        return 0;
	if (dcfWrite(pFile, (unsigned char*)&asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS), &size) == 0)
        return 0;
#endif

    return 1;
}
#elif (VIDEO_CODEC_OPTION == MPEG4_CODEC)
s32 asfWriteHeaderExtensionObject(FS_FILE* pFile)
{
    u32 size;
    __align(4) ASF_HEADER_EXTENSION_OBJECT asfHeaderExtensionObject =
    {
        {0xb5, 0x03, 0xbf, 0x5f,    /* object_id = ASF_Header_Extension_Object */
         0x2e, 0xa9, 0xcf, 0x11,
         0x8e, 0xe3, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000002e, 0x00000000},   /* object_size */
        {0x11, 0xd2, 0xd3, 0xab,    /* reserved_field_1 = ASF_Reserved_1 */
         0xba, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        0x0006,             /* reserved_field_2 */
        0x00000000,         /* header_extension_data_size */
    };


    __align(4) ASF_HDR_EXT_STREAM_PROPERTY asfHeaderExtStreamPropertyObject =
    {
        {0xCB, 0xA5, 0xE6, 0x14, 	/* object_id = ASF_Extended_Stream_Properties_Object */
         0x72, 0xC6, 0x32, 0x43,
         0x83, 0x99, 0xA9, 0x69,
         0x52, 0x06, 0x5B, 0x5A},
        {0x00000000, 0x00000000},   /* object_size */
		{0x00000000, 0x00000000},   /* start_time */
		{0x00000000, 0x00000000},   /* end_time */
		 0x00000000,				/* data_bitrate = 350000 */
		 //0x004c4b40,				/* data_bitrate = 350000 */
		 0x00000bb8,				/* buffer_size */
		 0x00000000,				/* initial_buffer_fullness */
		 0x00000000,
		 //0x004c4b40,				/* alternate_data_bitrate */
		 0x00000bb8,				/* alternate_buffer_size */
		 0x00000000,				/* alternate_initial_buffer_fullness */
		 0x00000000,
		 //0x00008b11,				/* max_object_size */
		 //0xffffffff,				/* max_object_size */
		 0x00000002,				/* flag */
		 0x0001,					/* stream_num */
		 0x0000,					/* stream_language */
		 {0x00051615, 0x00000000},  /* average_time_per_frame */
		 0x0000,					/* stream_name_count */
		 0x0001,					/* payload_ext_sys_cnt */
	};

	__align(4) ASF_PAYLOAD_EXT_SYSTEMS asfPayloadExtSystem =
	{
		{0x20, 0xDC, 0x90, 0xD5,    /* object_id = ASF_Payload_Extension_System_Content_Type    */
         0xBC, 0x07, 0x6C, 0x43,
         0x9C, 0xF7, 0xF3, 0xBB,
         0xFB, 0xF1, 0xA4, 0xDC},
         0x0001,					/* data_size */
         0x00000000, 				/* info_length */
	};
    asfHeaderExtensionObject.object_size.lo = sizeof(ASF_HEADER_EXTENSION_OBJECT) + sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);
	asfHeaderExtensionObject.header_extension_data_size = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

	asfHeaderExtStreamPropertyObject.object_size.lo = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_HEADER_EXTENSION_OBJECT);

	CopyMemory(asfMassWriteDataPoint, &asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY));
    asfMassWriteDataPoint  += sizeof(ASF_HDR_EXT_STREAM_PROPERTY);

	CopyMemory(asfMassWriteDataPoint, &asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS));
    asfMassWriteDataPoint  += sizeof(ASF_PAYLOAD_EXT_SYSTEMS);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT), &size) == 0)
        return 0;
	if (dcfWrite(pFile, (unsigned char*)&asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY), &size) == 0)
        return 0;
	if (dcfWrite(pFile, (unsigned char*)&asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS), &size) == 0)
        return 0;
#endif

    return 1;
}
#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
s32 asfWriteHeaderExtensionObject(FS_FILE* pFile)
{
    u32 size;
    __align(4) ASF_HEADER_EXTENSION_OBJECT asfHeaderExtensionObject =
    {
        {0xb5, 0x03, 0xbf, 0x5f,    /* object_id = ASF_Header_Extension_Object */
         0x2e, 0xa9, 0xcf, 0x11,
         0x8e, 0xe3, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        {0x0000002e, 0x00000000},   /* object_size */
        {0x11, 0xd2, 0xd3, 0xab,    /* reserved_field_1 = ASF_Reserved_1 */
         0xba, 0xa9, 0xcf, 0x11,
         0x8e, 0xe6, 0x00, 0xc0,
         0x0c, 0x20, 0x53, 0x65},
        0x0006,             /* reserved_field_2 */
        0x00000000,         /* header_extension_data_size */
    };

   asfHeaderExtensionObject.object_size.lo = sizeof(ASF_HEADER_EXTENSION_OBJECT) ;//+ sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

   //asfHeaderExtensionObject.header_extension_data_size = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

	//asfHeaderExtStreamPropertyObject.object_size.lo = sizeof(ASF_HDR_EXT_STREAM_PROPERTY) + sizeof(ASF_PAYLOAD_EXT_SYSTEMS);

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_HEADER_EXTENSION_OBJECT);

	//CopyMemory(asfMassWriteDataPoint, &asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY));
    //asfMassWriteDataPoint  += sizeof(ASF_HDR_EXT_STREAM_PROPERTY);

	//CopyMemory(asfMassWriteDataPoint, &asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS));
    //asfMassWriteDataPoint  += sizeof(ASF_PAYLOAD_EXT_SYSTEMS);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfHeaderExtensionObject, sizeof(ASF_HEADER_EXTENSION_OBJECT), &size) == 0)
        return 0;
	//if (dcfWrite(pFile, (unsigned char*)&asfHeaderExtStreamPropertyObject, sizeof(ASF_HDR_EXT_STREAM_PROPERTY), &size) == 0)
    //    return 0;
	//if (dcfWrite(pFile, (unsigned char*)&asfPayloadExtSystem, sizeof(ASF_PAYLOAD_EXT_SYSTEMS), &size) == 0)
    //    return 0;
#endif

    return 1;
}
#endif
/*

Routine Description:

    Write codec list object.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteCodecListObject(FS_FILE* pFile)
{
    //u32 size;
    __align(4) ASF_CODEC_LIST_OBJECT asfCodecListObject =
    {
        {0x40, 0x52, 0xd1, 0x86,    /* object_id = ASF_Codec_List_Object */
         0x1d, 0x31, 0xd0, 0x11,
         0xa3, 0xa4, 0x00, 0xa0,
         0xc9, 0x03, 0x48, 0xf6},
        {0x0000004e, 0x00000000},   /* object_size = 0x000000000000004e (video only)    */
                        /*         = 0x000000000000008a (video and audio)   */
        {0x41, 0x52, 0xD1, 0x86,    /* reserved = = ASF_Reserved_2 */
         0x1d, 0x31, 0xd0, 0x11,
         0xa3, 0xa4, 0x00, 0xa0,
         0xc9, 0x03, 0x48, 0xf6},
        0x00000001,         /* codec_entries_count = 0x00000001 (video only)    */
                        /*             = 0x00000002 (video and audio)   */
    };
    __align(4) ASF_HDR_VIDE_CODEC_ENTRY asfHdrVideCodecEntry =
    {
        0x0001,             /* type = video codec */
        0x000b,             /* codec_name_length */
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        0x0049, 0x0053,         /* codec_name[0x0B] = "ISO MPEG-4\0" */
        0x004f, 0x0020,
        0x004d, 0x0050,
        0x0045, 0x0047,
        0x002d, 0x0034,
        0x0000,
        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
   		0x0000, 0x0000,         /* codec_name[0x0B] = "ISO MPEG-4\0" */
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000,
        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
   		0x0048, 0x002E,         /* codec_name[0x0B] = "H.264 " */
        0x0032, 0x0036,
        0x0034, 0x0020,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000,
        #endif
        0x0000,             /* codec_description_length */
                            /* codec_description[0x00] */
        0x0004,             /* codec_information_length */
        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
        0x4d, 0x34, 0x53, 0x32,     /* codec_information[0x04] = "M4S2" */
        #elif (VIDEO_CODEC_OPTION == H264_CODEC)
        0x48, 0x32, 0x36, 0x34,     /* codec_information[0x04] = "H264" */
        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
   		//0x6d,0x6a,0x70,0x67,  //mjpg
		0x4d,0x4a,0x50,0x47,  //MJPG
    	//0x6d,0x4a,0x50,0x47,  //mJPG
		//0x4a,0x46,0x49,0x46,  //JFIF
		//0x4a,0x50,0x45,0x47,    //JPEG

        #endif

    };
#ifdef ASF_AUDIO
    #if(IIS_SAMPLE_RATE == 8000)
    __align(4) ASF_HDR_AUDI_CODEC_ENTRY asfHdrAudiCodecEntry =
    {
        0x0002,             /* type = audio codec */
        0x0006,
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x0050, 0x0043,         /* codec_name[0x06] = "PCM\0\0\0" */
        0x004d, 0x0000,
        0x0000, 0x0000,
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x0041, 0x0044,         /* codec_name[0x06] = "ADPCM\0" */
        0x0050, 0x0043,
        0x004d, 0x0000,
      #endif
        0x0013,             /* codec_description_length */
        0x0036, 0x0034,         /* codec_description[0x13] = "64kb/s, 8kHz, Mono\0" */
        0x006b, 0x0062,
        0x002f, 0x0073,
        0x002c, 0x0020,
        0x0038, 0x006b,
        0x0048, 0x007a,
        0x002c, 0x0020,
        0x004d, 0x006f,
        0x006e, 0x006f,
        0x0000,
        0x0002,             /* codec_information_length */
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x01, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_PCM */
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x11, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_DVI_ADPCM */
      #endif
    };
    #elif(IIS_SAMPLE_RATE==16000)
    __align(4) ASF_HDR_AUDI_CODEC_ENTRY asfHdrAudiCodecEntry =
    {
        0x0002,             /* type = audio codec */
        0x0006,
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x0050, 0x0043,         /* codec_name[0x06] = "PCM\0\0\0" */
        0x004d, 0x0000,
        0x0000, 0x0000,
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x0041, 0x0044,         /* codec_name[0x06] = "ADPCM\0" */
        0x0050, 0x0043,
        0x004d, 0x0000,
      #endif
        0x0013,             /* codec_description_length */
        0x0031, 0x0032,     /* codec_description[0x13] = "128kb/s,16kHz,Mono\0" */
        0x0038, 0x006b,
        0x0062, 0x002f,
        0x0073, 0x002c,
        0x0031, 0x0036,
        0x006b, 0x0048,
        0x007a, 0x002c,
        0x004d, 0x006f,
        0x006e, 0x006f,
        0x0000,
        0x0002,             /* codec_information_length */
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x01, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_PCM */
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x11, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_DVI_ADPCM */
      #endif
    };
    #elif(IIS_SAMPLE_RATE==32000)
    __align(4) ASF_HDR_AUDI_CODEC_ENTRY asfHdrAudiCodecEntry =
    {
        0x0002,             /* type = audio codec */
        0x0006,
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x0050, 0x0043,         /* codec_name[0x06] = "PCM\0\0\0" */
        0x004d, 0x0000,
        0x0000, 0x0000,
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x0041, 0x0044,         /* codec_name[0x06] = "ADPCM\0" */
        0x0050, 0x0043,
        0x004d, 0x0000,
      #endif
        0x0013,             /* codec_description_length */
        0x0032, 0x0035,     /* codec_description[0x13] = "256kb/s,32kHz,Mono\0" */
        0x0036, 0x006b,
        0x0062, 0x002f,
        0x0073, 0x002c,
        0x0033, 0x0032,
        0x006b, 0x0048,
        0x007a, 0x002c,
        0x004d, 0x006f,
        0x006e, 0x006f,
        0x0000,
        0x0002,             /* codec_information_length */
      #if (AUDIO_CODEC == AUDIO_CODEC_PCM)
        0x01, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_PCM */
      #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)
        0x11, 0x00,         /* codec_information[0x02] = ASF_WAVE_FORMAT_DVI_ADPCM */
      #endif
    };
    #endif
#endif

    asfCodecListObject.object_size.lo =
        sizeof(ASF_CODEC_LIST_OBJECT) +
            sizeof(ASF_HDR_VIDE_CODEC_ENTRY)
#ifdef ASF_AUDIO
            +
            sizeof(ASF_HDR_AUDI_CODEC_ENTRY);
#else
            ;
#endif
    asfCodecListObject.codec_entries_count =
#ifdef ASF_AUDIO
        0x00000002;
#else
        0x00000001;
#endif

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfCodecListObject, sizeof(ASF_CODEC_LIST_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_CODEC_LIST_OBJECT);
    CopyMemory(asfMassWriteDataPoint, &asfHdrVideCodecEntry, sizeof(ASF_HDR_VIDE_CODEC_ENTRY));
    asfMassWriteDataPoint  += sizeof(ASF_HDR_VIDE_CODEC_ENTRY);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfCodecListObject, sizeof(ASF_CODEC_LIST_OBJECT), &size) == 0)
        return 0;
    if (dcfWrite(pFile, (unsigned char*)&asfHdrVideCodecEntry, sizeof(ASF_HDR_VIDE_CODEC_ENTRY), &size) == 0)
        return 0;
#endif

#ifdef ASF_AUDIO
    #if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfHdrAudiCodecEntry, sizeof(ASF_HDR_AUDI_CODEC_ENTRY));
    asfMassWriteDataPoint  += sizeof(ASF_HDR_AUDI_CODEC_ENTRY);
    #else
    if (dcfWrite(pFile, (unsigned char*)&asfHdrAudiCodecEntry, sizeof(ASF_HDR_AUDI_CODEC_ENTRY), &size) == 0)
        return 0;
#endif
#endif

    return 1;
}

/*

Routine Description:

    Write content description object.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteContentDescriptionObject(FS_FILE* pFile)
{
    //u32 size;
    __align(4) ASF_CONTENT_DESCRIPTION_OBJECT asfContentDescriptionObject =
    {
        {0x33, 0x26, 0xb2, 0x75,    /* object_id = ASF_Content_Description_Object */
         0x8e, 0x66, 0xcf, 0x11,
         0xa6, 0xd9, 0x00, 0xaa,
         0x00, 0x62, 0xce, 0x6c},
        {0x00000062, 0x00000000},   /* object_size */
        0x0000,             /* title_length */
        0x0000,             /* author_length */
        0x0000,             /* copyright_length */
        0x0040,             /* description_length */
        0x0000,             /* rating_length */
                        /* title[0x00] */
                        /* author[0x00] */
                        /* copyright[0x00] */
        0x0000, 0x0000,         /* description[0x20] = "HIMAX PA9001\0..." */
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
        0x0000, 0x0000,
                        /* rating[0x00] */
    };

    asfContentDescriptionObject.object_size.lo = sizeof(ASF_CONTENT_DESCRIPTION_OBJECT);

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfContentDescriptionObject, sizeof(ASF_CONTENT_DESCRIPTION_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_CONTENT_DESCRIPTION_OBJECT);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfContentDescriptionObject, sizeof(ASF_CONTENT_DESCRIPTION_OBJECT), &size) == 0)
        return 0;
#endif

    return 1;
}

/*

Routine Description:

    Write header padding object.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteHdrPaddingObject(FS_FILE* pFile)
{
    //u32 size;
    //__align(4) u8 paddingBytes[0x200] = { 0x00 };
    __align(4) ASF_HDR_PADDING_OBJECT asfHdrPaddingObject =
    {
        {0x74, 0xd4, 0x06, 0x18,    /* object_id = ASF_Padding_Object */
         0xdf, 0xca, 0x09, 0x45,
         0xa4, 0xba, 0x9a, 0xab,
         0xcb, 0x96, 0xaa, 0xe8},
        {0x00000000, 0x00000000},   /* object_size = 0x???????????????? byte */
    };

    asfHdrPaddingObject.object_size.lo =
        sizeof(ASF_HDR_PADDING_OBJECT) +
        asfHeaderPaddingSize;

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfHdrPaddingObject, sizeof(ASF_HDR_PADDING_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_HDR_PADDING_OBJECT);
    CopyMemory(asfMassWriteDataPoint, paddingBytes, asfHeaderPaddingSize);
    asfMassWriteDataPoint  += asfHeaderPaddingSize;
#else
    if (dcfWrite(pFile, (unsigned char*)&asfHdrPaddingObject, sizeof(ASF_HDR_PADDING_OBJECT), &size) == 0)
            return 0;
    if (dcfWrite(pFile, (unsigned char*)paddingBytes, asfHeaderPaddingSize, &size) == 0)
            return 0;
#endif

    return 1;
}

/*-------------------------------------------------------------------------*/
/* Data object                                 */
/*-------------------------------------------------------------------------*/

/*

Routine Description:

    Write data object pre.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteDataObjectPre(FS_FILE* pFile)
{
    u32 size;
    __align(4) ASF_DATA_OBJECT asfDataObject =
    {
        {0x36, 0x26, 0xb2, 0x75,    /* object_id = ASF_Data_Object */
         0x8e, 0x66, 0xcf, 0x11,
         0xa6, 0xd9, 0x00, 0xaa,
         0x00, 0x62, 0xce, 0x6c},
        {0xffffffff, 0xffffffff},   /* object_size = 0x???????????????? */
        {0x00, 0x00, 0x00, 0x00,    /* file_id = = file_id of asf_data_object and asf_simple_index_object */
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00},
        {0xffffffff, 0xffffffff},   /* total_data_packets = 0x???????????????? */
        0x0101,             /* reserved */
                        /* asf_dta_data_pkt[0x????????????????] */
    };

#if ASF_MASS_WRITE_HEADER    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfDataObject, sizeof(ASF_DATA_OBJECT));
    asfMassWriteDataPoint  += sizeof(ASF_DATA_OBJECT);

	#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
	if (dcfBackupWrite(pFile, (unsigned char*)asfMassWriteData, asfMassWriteDataPoint - asfMassWriteData, &size) == 0) {
		DEBUG_ASF("Write file error!!!\n");
        return 0;
    }
	#else
    /*** encrypt File ***/	
    if (dcfWrite(pFile, (unsigned char*)asfMassWriteData, asfMassWriteDataPoint - asfMassWriteData, &size) == 0) {
        DEBUG_ASF("Write file error!!!\n");
        return 0;
    }
	#endif
#else
    if (dcfWrite(pFile, (unsigned char*)&asfDataObject, sizeof(ASF_DATA_OBJECT), &size) == 0)
        return 0;
#endif

    return 1;
}
#endif //( ( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) ) || (!MULTI_CHANNEL_VIDEO_REC))
/*

Routine Description:

    Write data object post.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteDataObjectPost(FS_FILE* pFile)
{
    u32 size;
    u32 offset;
    u64 objectSize = {0x00000000, 0x00000000};
    u64 totalDataPackets = {0x00000000, 0x00000000};

    offset = dcfTell(pFile);

    dcfSeek(pFile, asfHeaderSize+16, FS_SEEK_SET);

    objectSize.lo = asfDataSize;
    /*** encrypt File ***/
	#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
	if (dcfBackupWrite(pFile, (unsigned char*)&objectSize, sizeof(u64), &size) == 0)
            return 0;
	#else
    if (dcfWrite(pFile, (unsigned char*)&objectSize, sizeof(u64), &size) == 0)
            return 0;
	#endif
	
    dcfSeek(pFile, asfHeaderSize+40, FS_SEEK_SET);

    totalDataPackets.lo = asfDataPacketCount;
    /*** encrypt File ***/
	#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
	if (dcfBackupWrite(pFile, (unsigned char*)&totalDataPackets, sizeof(u64), &size) == 0)
            return 0;
	#else
    if (dcfWrite(pFile, (unsigned char*)&totalDataPackets, sizeof(u64), &size) == 0)
            return 0;
	#endif
    dcfSeek(pFile, offset, FS_SEEK_SET);

    return 1;
}

/*

Routine Description:

    Write data packet pre.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
#if( ( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) || (!MULTI_CHANNEL_VIDEO_REC)  )
s32 asfWriteDataPacketPre(FS_FILE* pFile)
{
	#if ASF_MASS_WRITE
	#else
    u32 size;
	#endif
    __align(4) ASF_DATA_PACKET_MULTIPAYLOAD asfDtaDataPacket =
    {
        {
            0x82,               /* error_correction_flags, ErrorCorrectionDataLength = 2,   */
                            /*             OpaqueDataPresent = 0,       */
                            /*             ErrorCorrectionLengthType = 0,   */
                            /*             ErrorCorrectionPresent = 1,      */
            0x00, 0x00,         /* error_correction_data[0x02], Type.Type = 0 (data is uncorrected),    */
                            /*              Type.Number = 0,            */
                            /*              Cycle = 0,              */
        },
        {
            0x11,               /* payload_flag, MultiplePayloadsPresent = 1,   */
                            /*       SequenceType = 0 (X),      */
                            /*       PaddingLengthType = 2 (WORD),  */
                            /*       PacketLengthType = 0 (X),  */
                            /*       ErrorCorrectionPresent = 0,            */
            0x5d,               /* property_flags, ReplicatedDataLength = 1 (BYTE),     */
                            /*         OffsetIntoMediaObjectLengthType = 3 (DWORD), */
                            /*         MediaObjectNumberLengthType = 1 (BYTE),  */
                            /*         StreamNumberLengthType = 1 (BYTE),       */
                            /* packet_length (X) */
                            /* sequence (X) */
            0xffff,             /* padding_length = 0x???? */
            0x00000000,         /* send_time = 0x???????? */
            0xffff,             /* duration = 0x???? */
        },
        {
            0xff,               /* payload_flags = 0x??, NumberOfPayloads,      */
                            /*           PayloadLengthType = 2 (WORD),  */
                            /* asf_dta_payld[0x??] */
        },
    };


#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(asfMassWriteData, &asfDtaDataPacket, sizeof(ASF_DATA_PACKET_MULTIPAYLOAD));
    asfMassWriteDataPoint   = asfMassWriteData + sizeof(ASF_DATA_PACKET_MULTIPAYLOAD);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfDtaDataPacket, sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), &size) == 0)
            return 0;
#endif

    asfDataPacketCount++;
#if ASF_MASS_WRITE    /* Peter 070104 */
#else
    asfDataPacketOffset = dcfTell(pFile) - sizeof(ASF_DATA_PACKET_MULTIPAYLOAD);
#endif
    if(asfDataPacketFormatFlag)
    {
        asfDataPacketPreSendTime = asfDataPacketSendTime;   //Lsk 090309
        asfDataPacketFormatFlag = 0;                        //Lsk 090309
        asfDataPacketSendTime = asfVidePresentTime-PREROLL; //Lsk 090309
    }
    asfDataPacketNumPayload = 0;
    asfDataPacketLeftSize = ASF_DATA_PACKET_SIZE - sizeof(ASF_DATA_PACKET_MULTIPAYLOAD);
    return 1;
}

/*

Routine Description:

    Write data packet post.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
// Lsk 090309
s32 asfWriteDataPacketPost(FS_FILE* pFile, u32 paddingLength)
{
    u32 size;
    //u32 offset;
    u32 padding_offset=0;
    //u16 payload_length;
    //u32 tempBufLeftSize;
    u32 timestamp;

//

    //u8 paddingBytes[0x200] = { 0x00 };
    __align(4) ASF_DATA_PACKET_MULTIPAYLOAD asfDtaDataPacket =
    {
        {
            0x82,               /* error_correction_flags, ErrorCorrectionDataLength = 2,   */
                            /*             OpaqueDataPresent = 0,       */
                            /*             ErrorCorrectionLengthType = 0,   */
                            /*             ErrorCorrectionPresent = 1,      */
            0x00, 0x00,         /* error_correction_data[0x02], Type.Type = 0 (data is uncorrected),    */
                            /*              Type.Number = 0,            */
                            /*              Cycle = 0,              */
        },
        {
            0x11,               /* payload_flag, MultiplePayloadsPresent = 1,   */
                            /*       SequenceType = 0 (X),      */
                            /*       PaddingLengthType = 2 (WORD),  */
                            /*       PacketLengthType = 0 (X),  */
                            /*       ErrorCorrectionPresent = 0,            */
            0x5d,               /* property_flags, ReplicatedDataLength = 1 (BYTE),     */
                            /*         OffsetIntoMediaObjectLengthType = 3 (DWORD), */
                            /*         MediaObjectNumberLengthType = 1 (BYTE),  */
                            /*         StreamNumberLengthType = 1 (BYTE),       */
                            /* packet_length (X) */
                            /* sequence (X) */
            0x0000,             /* padding_length = 0x???? */
            0x00000000,         /* send_time = 0x???????? */
            0x0000,             /* duration = 0x???? */
        },
        {
            0x00,               /* payload_flags = 0x??, NumberOfPayloads,      */
                            /*           PayloadLengthType = 2 (WORD),  */
                            /* asf_dta_payld[0x??] */
        },
    };

    __align(4) ASF_DATA_PACKET_SINGLEPAYLOAD asfDtaDataPacket_single =
    {
        {
            0x82,               /* error_correction_flags, ErrorCorrectionDataLength = 2,   */
                            /*             OpaqueDataPresent = 0,       */
                            /*             ErrorCorrectionLengthType = 0,   */
                            /*             ErrorCorrectionPresent = 1,      */
            0x00, 0x00,         /* error_correction_data[0x02], Type.Type = 0 (data is uncorrected),    */
                            /*              Type.Number = 0,            */
                            /*              Cycle = 0,              */
        },
        {
            0x10,               /* payload_flag, MultiplePayloadsPresent = 1,   */
                            /*       SequenceType = 0 (X),      */
                            /*       PaddingLengthType = 2 (WORD),  */
                            /*       PacketLengthType = 0 (X),  */
                            /*       ErrorCorrectionPresent = 0,            */
            0x5d,               /* property_flags, ReplicatedDataLength = 1 (BYTE),     */
                            /*         OffsetIntoMediaObjectLengthType = 3 (DWORD), */
                            /*         MediaObjectNumberLengthType = 1 (BYTE),  */
                            /*         StreamNumberLengthType = 1 (BYTE),       */
                            /* packet_length (X) */
                            /* sequence (X) */
            0x0000,             /* padding_length = 0x???? */
            0x00000000,         /* send_time = 0x???????? */
            0x0000,             /* duration = 0x???? */
        }
    };

    //DEBUG_ASF("A");
    if(asfDataPacketFormatFlag)
        timestamp = asfDataPacketSendTime;
    else
        timestamp = asfDataPacketPreSendTime;
	if(PayloadType == 1)
		padding_offset = sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)-sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE);
	else if(PayloadType == 2)
    	padding_offset = sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)-sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE);

#if ASF_MASS_WRITE
    if(asfDataPacketNumPayload > 1)
    {
        asfDtaDataPacket.asf_dta_payload_parsing_inf.padding_length = paddingLength;
        asfDtaDataPacket.asf_dta_payload_parsing_inf.send_time = timestamp;
        asfDtaDataPacket.asf_dta_payload_parsing_inf.duration = 1;
        asfDtaDataPacket.asf_dta_m_payload_dta.payload_flags = 0x80 | asfDataPacketNumPayload;
        memset_hw(asfMassWriteDataPoint, 0, asfDataPacketLeftSize);
        asfMassWriteDataPoint  += asfDataPacketLeftSize;
        CopyMemory(asfMassWriteData, &asfDtaDataPacket, sizeof(ASF_DATA_PACKET_MULTIPAYLOAD));
		#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
	    if (dcfBackupWrite(pFile, (unsigned char*)asfMassWriteData, asfMassWriteDataPoint - asfMassWriteData, &size) == 0) {
            DEBUG_ASF("Write file error!!!\n");
            return 0;
        }
		#else
        if (dcfWrite(pFile, (unsigned char*)asfMassWriteData, asfMassWriteDataPoint - asfMassWriteData, &size) == 0) {
            DEBUG_ASF("Write file error!!!\n");
            return 0;
        }
		#endif
    }
    else
    {
        /* re-write fields of data packet header */
        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.padding_length = paddingLength + padding_offset;
        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.send_time = timestamp;
        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.duration = 1;

        CopyMemory(asfMassWriteData, &asfDtaDataPacket_single, sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD));

        if(PayloadType == 1)
        {
            CopyMemory(asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD), asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE)); //ASF_DTA_VIDEO_PAYLOAD_SINGLE shift left (sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD) byte.
            CopyMemory(asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE), asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD), (int)(asfMassWriteDataPoint-(asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD))));//Lsk : payload data shift left
            //CopyMemory(asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE), asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD), (int)(asfMassWriteDataPoint-(asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE))));//Lsk : payload data shift left
        }
		else if(PayloadType == 2)
		{
            CopyMemory(asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD), asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE)); //ASF_DTA_AUDIO_PAYLOAD_SINGLE shift left (sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD) byte.
            CopyMemory(asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE), asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD), (int)(asfMassWriteDataPoint-(asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD))));//Lsk : payload data shift left
            //CopyMemory(asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE), asfMassWriteData+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD), (int)(asfMassWriteDataPoint-(asfMassWriteData+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE))));//Lsk : payload data shift left
		}
        memset_hw(asfMassWriteDataPoint-padding_offset, 0, asfDataPacketLeftSize+padding_offset); //Lsk : padding data
        asfMassWriteDataPoint  += asfDataPacketLeftSize;
		#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
        if (dcfBackupWrite(pFile, (unsigned char*)asfMassWriteData, asfMassWriteDataPoint - asfMassWriteData, &size) == 0) {
            DEBUG_ASF("Write file error!!!\n");
            return 0;
        }		
		#else
        if (dcfWrite(pFile, (unsigned char*)asfMassWriteData, asfMassWriteDataPoint - asfMassWriteData, &size) == 0) {
            DEBUG_ASF("Write file error!!!\n");
            return 0;
        }
		#endif
    }
#else
    if(asfDataPacketNumPayload > 1)
    {
        asfDtaDataPacket.asf_dta_payload_parsing_inf.padding_length = paddingLength;
        asfDtaDataPacket.asf_dta_payload_parsing_inf.send_time = timestamp;
        asfDtaDataPacket.asf_dta_payload_parsing_inf.duration = 1;
        asfDtaDataPacket.asf_dta_m_payload_dta.payload_flags = 0x80 | asfDataPacketNumPayload;
        if (dcfWrite(pFile, (unsigned char*)paddingBytes, asfDataPacketLeftSize, &size) == 0)
            return 0;
        offset = dcfTell(pFile);
        dcfSeek(pFile, asfDataPacketOffset, FS_SEEK_SET);
        if (dcfWrite(pFile, (unsigned char*)&asfDtaDataPacket, sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), &size) == 0)
            return 0;
        dcfSeek(pFile, offset, FS_SEEK_SET);
    }
    else
    {
        offset = dcfTell(pFile);
        if (dcfWrite(pFile, (unsigned char*)paddingBytes, asfDataPacketLeftSize, &size) == 0)
            return 0;
        dcfSeek(pFile, asfDataPacketOffset, FS_SEEK_SET);

        if (dcfRead(pFile, (unsigned char*)(tempbuf), ASF_DATA_PACKET_SIZE, &size) == 0)
            return 0;

        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.padding_length = paddingLength + padding_offset;
        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.send_time = timestamp;
        asfDtaDataPacket_single.asf_dta_payload_parsing_inf.duration = 1;

		if(PayloadType == 1)
		{
			payload_length = offset-asfDataPacketOffset-sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DTA_VIDEO_PAYLOAD);
			CopyMemory(tempbuf, (u8*)(&asfDtaDataPacket_single), sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD));
			CopyMemory(tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD), tempbuf+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE)); //ASF_DTA_VIDEO_PAYLOAD_SINGLE shift left (sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD) byte.
			CopyMemory(tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE), tempbuf+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD), payload_length);//Lsk : payload data shift left
			memset_hw((u8*)(tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE)+payload_length), 0, padding_offset); //Lsk : padding data
		}
		else if(PayloadType == 2)
		{
        	payload_length = offset-asfDataPacketOffset-sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DTA_AUDIO_PAYLOAD);
			CopyMemory(tempbuf, (u8*)(&asfDtaDataPacket_single), sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD));
			CopyMemory(tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD), tempbuf+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD), sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE)); //ASF_DTA_AUDIO_PAYLOAD_SINGLE shift left (sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)-sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD) byte.
			CopyMemory(tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE), tempbuf+sizeof(ASF_DATA_PACKET_MULTIPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD), payload_length);//Lsk : payload data shift left
			memset_hw((u8*)(tempbuf+sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD)+sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE)+payload_length), 0, padding_offset); //Lsk : padding data
		}

        dcfSeek(pFile, asfDataPacketOffset, FS_SEEK_SET);
        if (dcfWrite(pFile, (unsigned char*)tempbuf, ASF_DATA_PACKET_SIZE, &size) == 0) {
            DEBUG_ASF("Write file error!!!\n");
            return 0;
        }
    }
#endif

    if(asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA)
    {
    	diskInfo            = &global_diskInfo;
	    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;

        //due to only update global_diskInfo when clos file, so we must calculate when open file
		//curr_free_space -= (ASF_DATA_PACKET_SIZE / 512) / 2;
		//curr_record_space += (ASF_DATA_PACKET_SIZE / 512) / 2;

        while(curr_free_space < DCF_OVERWRITE_THR_KBYTE)
		{
            if(dcfOverWriteDel()==0)
            {
                DEBUG_DCF("Over Write delete fail!!\n");
                return 0;
            }
            else
            {
                //DEBUG_ASF("Over Write delete Pass!!\n");
            }
            //due to only update global_diskInfo when clos file, so we must calculate when open file
            free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
			curr_free_space = free_size - curr_record_space;
		}
	}
    return 1;
}

#if (AUDIO_CODEC == AUDIO_CODEC_PCM)

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
s32 asfWriteAudiPayload(FS_FILE* pFile, IIS_BUF_MNG* pMng)
{
    //u32 size;
    u32 chunkTime, chunkSize;
    u8* pChunkBuf;
    u32 payloadSize;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif


    __align(4) ASF_DTA_AUDIO_PAYLOAD asfDtaPayload =
    {
        0x02,               /* stream_number = 0x02, StreamNumber = 2, KeyFrameBit = 0, (audio) */
        0x00,               /* media_object_number = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video)    */
                        /*             = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio)    */
        0x00000000,         /* offset_into_media_object = 0x00000000 =              */
                        /*      byte offset of Vn/An corresponding to start of this payload     */
                        /*  non-zero when Vn/An across multiple payloads            */
        0x08,               /* replicated_data_length = 0x08 */
        {0x00000000, 0x00000000},   /* replicated_data = {0x????????, 0x????????} =                     */
                        /*  {size of Vn/An, byte offset of of Vn/An}                    */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads    */
        0x0000,             /* payload_length = 0x???? = size of this payload   */
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads    */
                        /* payload_data[0x????????] */
    };

	if(ResetPayloadPresentTime == 0)
	{

        //DEBUG_ASF("1. asfAudiPresentTime = %010d\n",asfAudiPresentTime);
        //DEBUG_ASF("1. asfVidePresentTime = %010d\n",asfVidePresentTime);
        ResetPayloadPresentTime = 1;
	    if(asfAudiPresentTime <= asfVidePresentTime)
        {
    		asfVidePresentTime -= (asfAudiPresentTime - AV_TimeBase);
	    	asfAudiPresentTime = AV_TimeBase;
        }
        else
        {
            asfAudiPresentTime     -= (asfVidePresentTime - AV_TimeBase);
    	    asfVidePresentTime      = AV_TimeBase;
        }
        //DEBUG_ASF("2. asfAudiPresentTime = %010d\n",asfAudiPresentTime);
        //DEBUG_ASF("2. asfVidePresentTime = %010d\n",asfVidePresentTime);
	}
    chunkTime = pMng->time;
    chunkSize = pMng->size;
    pChunkBuf = pMng->buffer;

    if(MuteRec)
        memset_hw(pChunkBuf, 0x80, chunkSize);

    payloadSize = sizeof(ASF_DTA_AUDIO_PAYLOAD) + chunkSize;
    if (asfDataPacketLeftSize < payloadSize || asfDataPacketNumPayload == MAX_PAYLOAD_IN_PACKET) //Lsk 090410
    {
        if (asfWriteDataPacketPost(pFile, asfDataPacketLeftSize) == 0) /* finish data packet with padding */
                return 0;
        if (asfWriteDataPacketPre(pFile) == 0) /* new a data packet */
                return 0;
    }

    asfDataPacketNumPayload++;
    /* write audio payload header */
    asfDtaPayload.stream_number = 0x02;
    asfDtaPayload.media_object_number = (u8) asfAudiChunkCount;
    asfDtaPayload.offset_into_media_object = 0x00000000;
    asfDtaPayload.replicated_data_length = 0x08;
    asfDtaPayload.replicated_data.lo = chunkSize;
    asfDtaPayload.replicated_data.hi = asfAudiPresentTime;
    asfDtaPayload.payload_length = (u16) chunkSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_AUDIO_PAYLOAD));
    asfMassWriteDataPoint  += sizeof(ASF_DTA_AUDIO_PAYLOAD);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_AUDIO_PAYLOAD), &size) == 0)
            return 0;
#endif
    asfDataPacketLeftSize -= sizeof(ASF_DTA_AUDIO_PAYLOAD);
    /* payload data */
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, pChunkBuf, chunkSize);
    asfMassWriteDataPoint                  += chunkSize;
#else
    if (dcfWrite(pFile, (unsigned char*)pChunkBuf, chunkSize, &size) == 0)
            return 0;
#endif
    asfDataPacketLeftSize -= chunkSize;

    /* advance the index */
    asfAudiChunkCount++;
    asfAudiPresentTime += chunkTime;

    //if(asfCaptureMode == ASF_CAPTURE_OVERWRITE) {
        OS_ENTER_CRITICAL();
        CurrentAudioSize   -= chunkSize;
        OS_EXIT_CRITICAL();
    //}

    return 1;
}

#elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)

/*

Routine Description:

    Write ima adpcm audio payload.

Arguments:

    pFile - File handle.
    pMng1 - Buffer manager 1.
    pMng2 - Buffer manager 2.

Return Value:

    0 - Failure.
    1 - Success and only using pMng1 PCM data.
    2 - Success and using both pMng1 and pMng2 PCM data.

*/
s32 asfWriteAudiPayload_IMA_ADPCM_1Payload(FS_FILE* pFile, IIS_BUF_MNG* pMng1, IIS_BUF_MNG* pMng2, s32 *pPcmOffset)
{
    u32 size;
    u32 chunkTime, chunkSize;
    u8* pChunkBuf;
    u32 payloadSize;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    IMA_ADPCM_Option    ImaAdpcmOpt;
    IMA_ADPCM_Option    *pImaAdpcmOpt;
    s32                 PcmBytesForAdpcm;
    s32                 PcmOffset;
	u8  tmp;
	
    if(IISMode == 1)
        Audio_formate = Audio_formate_Out;
    else
        Audio_formate = Audio_formate_In;

    __align(4) ASF_DTA_AUDIO_PAYLOAD asfDtaPayload =
    {
        0x02,               /* stream_number = 0x02, StreamNumber = 2, KeyFrameBit = 0, (audio) */
        0x00,               /* media_object_number = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video)    */
                        /*             = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio)    */
        0x00000000,         /* offset_into_media_object = 0x00000000 =              */
                        /*      byte offset of Vn/An corresponding to start of this payload     */
                        /*  non-zero when Vn/An across multiple payloads            */
        0x08,               /* replicated_data_length = 0x08 */
        {0x00000000, 0x00000000},   /* replicated_data = {0x????????, 0x????????} =                     */
                        /*  {size of Vn/An, byte offset of of Vn/An}                    */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads    */
        0x0000,             /* payload_length = 0x???? = size of this payload   */
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads    */
                        /* payload_data[0x????????] */
    };

	if(ResetPayloadPresentTime == 0)
	{

        //DEBUG_ASF("1. asfAudiPresentTime = %010d\n",asfAudiPresentTime);
        //DEBUG_ASF("1. asfVidePresentTime = %010d\n",asfVidePresentTime);
        ResetPayloadPresentTime = 1;
	    if(asfAudiPresentTime <= asfVidePresentTime)
        {
    		asfVidePresentTime -= (asfAudiPresentTime - AV_TimeBase);
	    	asfAudiPresentTime = AV_TimeBase;
        }
        else
        {
            asfAudiPresentTime     -= (asfVidePresentTime - AV_TimeBase);
    	    asfVidePresentTime      = AV_TimeBase;
        }
        //DEBUG_ASF("2. asfAudiPresentTime = %010d\n",asfAudiPresentTime);
        //DEBUG_ASF("2. asfVidePresentTime = %010d\n",asfVidePresentTime);
	}
    //chunkTime = pMng->time;
    //chunkSize = pMng->size;
    //pChunkBuf = pMng->buffer;
    chunkTime   = (IMA_ADPCM_SAMPLE_PER_BLOCK * 1000 / IIS_SAMPLE_RATE) + AdpcmPayloadTimeLose[asfAudiChunkCount & 0xf];
    chunkSize   = IMA_ADPCM_BLOCK_SIZE;
    pChunkBuf   = (u8*)ImaAdpcmBuf;

    payloadSize = sizeof(ASF_DTA_AUDIO_PAYLOAD) + chunkSize;
    if (asfDataPacketLeftSize < payloadSize || asfDataPacketNumPayload == MAX_PAYLOAD_IN_PACKET) //Lsk 090410
    {
        if (asfWriteDataPacketPost(pFile, asfDataPacketLeftSize) == 0) /* finish data packet with padding */
                return 0;
        if (asfWriteDataPacketPre(pFile) == 0) /* new a data packet */
                return 0;
    }

    pImaAdpcmOpt                        = &ImaAdpcmOpt;
    pImaAdpcmOpt->AdpcmAddress          = ImaAdpcmBuf;
    pImaAdpcmOpt->AdpcmSize             = IMA_ADPCM_BLOCK_SIZE;
    pImaAdpcmOpt->AdpcmSamplePerBlock   = IMA_ADPCM_SAMPLE_PER_BLOCK;
    pImaAdpcmOpt->PcmStrWord            = 0;
    pImaAdpcmOpt->AdpcmStrWord          = 0;
    pImaAdpcmOpt->AdpcmSample           = 0;
    pImaAdpcmOpt->AdpcmIndex            = 0;
    switch (Audio_formate) {
        case nomo_8bit_8k:
        case nomo_8bit_16k:
            pImaAdpcmOpt->PcmBitPerSample   = 8;
            pImaAdpcmOpt->PcmSigned         = 0;
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK;
            break;
        case nomo_16bit_8k:
            pImaAdpcmOpt->PcmBitPerSample   = 16;
            pImaAdpcmOpt->PcmSigned         = 1;
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK * 2;
            break;
        default:
            pImaAdpcmOpt->PcmBitPerSample   = 8;
            pImaAdpcmOpt->PcmSigned         = 0;
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK;
            DEBUG_ASF("Don't support Audio_formate %d\n", Audio_formate);
    }
    pImaAdpcmOpt->PcmTotalSize      = PcmBytesForAdpcm;
    PcmOffset                       = *pPcmOffset;
    if(PcmOffset <= (IIS_CHUNK_SIZE - PcmBytesForAdpcm)) {
        pImaAdpcmOpt->PcmAddress1   = pMng1->buffer + PcmOffset;
        pImaAdpcmOpt->PcmAddress2   = 0;
        pImaAdpcmOpt->PcmSize1      = PcmBytesForAdpcm;
        pImaAdpcmOpt->PcmSize2      = 0;
        *pPcmOffset                += PcmBytesForAdpcm;
    } else {
        pImaAdpcmOpt->PcmAddress1   = pMng1->buffer + PcmOffset;
        pImaAdpcmOpt->PcmAddress2   = pMng2->buffer;
        pImaAdpcmOpt->PcmSize1      = pMng1->size - PcmOffset;
        pImaAdpcmOpt->PcmSize2      = PcmBytesForAdpcm - pImaAdpcmOpt->PcmSize1;
        *pPcmOffset                 = pImaAdpcmOpt->PcmSize2;
    }
    if(IMA_ADPCM_Encode_Block_HW(pImaAdpcmOpt) == 0) {
        DEBUG_IIS("Wav Write Data error!!!\n");
        sysVoiceRecStop     = 1;
        sysVoiceRecStart    = 0;
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }

    asfDataPacketNumPayload++;
    /* write audio payload header */
    asfDtaPayload.stream_number = 0x02;
    asfDtaPayload.media_object_number = (u8) asfAudiChunkCount;
    asfDtaPayload.offset_into_media_object = 0x00000000;
    asfDtaPayload.replicated_data_length = 0x08;
    asfDtaPayload.replicated_data.lo = chunkSize;
    asfDtaPayload.replicated_data.hi = asfAudiPresentTime;
    asfDtaPayload.payload_length = (u16) chunkSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_AUDIO_PAYLOAD));
    asfMassWriteDataPoint  += sizeof(ASF_DTA_AUDIO_PAYLOAD);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_AUDIO_PAYLOAD), &size) == 0)
            return 0;
#endif
    asfDataPacketLeftSize -= sizeof(ASF_DTA_AUDIO_PAYLOAD);
    /* payload data */
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, pChunkBuf, chunkSize);
    asfMassWriteDataPoint                  += chunkSize;
#else
    if (dcfWrite(pFile, (unsigned char*)pChunkBuf, chunkSize, &size) == 0)
            return 0;
#endif
    asfDataPacketLeftSize -= chunkSize;

    /* advance the index */
    asfAudiChunkCount++;
    asfAudiPresentTime += chunkTime;

    //if(asfCaptureMode == ASF_CAPTURE_OVERWRITE) {
        OS_ENTER_CRITICAL();
        CurrentAudioSize   -= chunkSize;
        OS_EXIT_CRITICAL();
    //}

    if(pImaAdpcmOpt->PcmSize2)
        return 2;
    else
        return 1;
}

/*

Routine Description:

    Write ima adpcm audio payload.

Arguments:

    pFile - File handle.
    pMng - Buffer manager.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteAudiPayload_IMA_ADPCM(FS_FILE* pFile, IIS_BUF_MNG* pMng, s32 *pPcmOffset)
{
    s32 nRtn, PcmBytesForAdpcm;
    if(IISMode == 1)
        Audio_formate = Audio_formate_Out;
    else
        Audio_formate = Audio_formate_In;

    switch (Audio_formate) {
        case nomo_8bit_8k:
        case nomo_8bit_16k:
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK;
            break;
        case nomo_16bit_8k:
            PcmBytesForAdpcm                = IMA_ADPCM_SAMPLE_PER_BLOCK * 2;
            break;
        default:
            DEBUG_ASF("Don't support Audio_formate %d\n", Audio_formate);
    }
    if(*pPcmOffset == 0)    // no temp payload
    {
        do {
            nRtn    = asfWriteAudiPayload_IMA_ADPCM_1Payload(pFile, pMng, 0, pPcmOffset);
        } while ((nRtn == 1) && ((*pPcmOffset + PcmBytesForAdpcm) <= IIS_CHUNK_SIZE));
        if(nRtn == 1)
        {
            memcpy(iisBufMngTemp.buffer + *pPcmOffset, pMng->buffer + *pPcmOffset, IIS_CHUNK_SIZE - *pPcmOffset);
            iisBufMngTemp.size  = pMng->size;
            iisBufMngTemp.time  = pMng->time;
            return 1;
        }
    } else {    // 前一個audio chunk data還沒編完
        nRtn    = asfWriteAudiPayload_IMA_ADPCM_1Payload(pFile, &iisBufMngTemp, pMng, pPcmOffset);
        if((nRtn == 2) && ((*pPcmOffset + PcmBytesForAdpcm) <= IIS_CHUNK_SIZE))
        {
            do {
                nRtn    = asfWriteAudiPayload_IMA_ADPCM_1Payload(pFile, pMng, 0, pPcmOffset);
            } while ((nRtn == 1) && (*pPcmOffset + PcmBytesForAdpcm) <= IIS_CHUNK_SIZE);
        }
        if(nRtn)
        {
            memcpy(iisBufMngTemp.buffer + *pPcmOffset, pMng->buffer + *pPcmOffset, IIS_CHUNK_SIZE - *pPcmOffset);
            iisBufMngTemp.size  = pMng->size;
            iisBufMngTemp.time  = pMng->time;
            return 1;
        }
    }
    return 0;
}

#endif  // #if (AUDIO_CODEC == AUDIO_CODEC_PCM)

/*

Routine Description:

    Write video payload.

Arguments:

    pFile - File handle.
    pMng - Buffer manager.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteVidePayload(FS_FILE* pFile, VIDEO_BUF_MNG* pMng)
{
    //u32 size;
    u32 chunkFlag, chunkTime, chunkSize;
    u8* pChunkBuf;
    u32 payloadSize;
    u32 subPayloadSize;
    //u32 writeSize;
    u32 offsetIntoMediaObject;
    u8  streamNumber;
    u32 SPS_PPS_WriteSize   = 0;
    u8  SPS_PPS_NotReady    = 0;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    __align(4) ASF_DTA_VIDEO_PAYLOAD asfDtaPayload =
    {
        0x81,               /* stream_number = 0x81, StreamNumber = 1, KeyFrameBit = 1, (video) */
        0x00,               /* media_object_number = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video)    */
                        /*             = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio)    */
        0x00000000,         /* offset_into_media_object = 0x00000000 =              */
                        /*      byte offset of Vn/An corresponding to start of this payload     */
                        /*  non-zero when Vn/An across multiple payloads            */
        0x09,               /* replicated_data_length = 0x08 */
        {0x00000000, 0x00000000},   /* replicated_data = {0x????????, 0x????????} =                     */
                        /*  {size of Vn/An, byte offset of of Vn/An}                    */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads    */


		0x00,           /* interlace and top field first */


        0x0000,         /* payload_length = 0x???? = size of this payload   */
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads    */
                        /* payload_data[0x????????] */
    };

    if(ASF_set_interlace_flag)
    {
        asfDtaPayload.flag =  0xc0; 			/* interlace and top field first */
    }

	PayloadType = 1;
    asfDataPacketFormatFlag = 1;                        //Lsk 090309
    chunkFlag   = pMng->flag;
    chunkTime   = pMng->time;
#if FORCE_FPS
    if(DummyChunkTime && (chunkTime > DummyChunkTime))
    {
        chunkTime      -= DummyChunkTime;
        DummyChunkTime  = 0;
    }
#endif
    chunkSize = pMng->size;
    pChunkBuf = pMng->buffer;


    if((asfVopCount == 0) && (*(pMng->buffer+4) != 0x67))
    {
    	if((mpeg4Width == 1280)&&(mpeg4Height == 720))
    	{
    	    H264_config[0x07] = 0x1E;
    	    H264_config[0x08] = 0xDA;
    	    H264_config[0x09] = 0x01;
    	    H264_config[0x0A] = 0x40;
    	    H264_config[0x0B] = 0x16;
    	    H264_config[0x0C] = 0xE4;
    	}
    	else if((mpeg4Width == 1920)&&(mpeg4Height == 1072))
    	{
    	    H264_config[0x07] = 0x28;
    	    H264_config[0x08] = 0xDA;
    	    H264_config[0x09] = 0x01;
    	    H264_config[0x0A] = 0xE0;
    	    H264_config[0x0B] = 0x08;
    	    H264_config[0x0C] = 0x79;
    	}
        else if((mpeg4Width == 1920)&&(mpeg4Height == 1088))
    	{
    	    H264_config[0x07] = 0x28;
    	    H264_config[0x08] = 0xDA;
    	    H264_config[0x09] = 0x01;
    	    H264_config[0x0A] = 0xE0;
    	    H264_config[0x0B] = 0x08;
    	    H264_config[0x0C] = 0x9F;
    	    H264_config[0x0D] = 0x95;
    	}        
        chunkSize += H264_SPSPPS_length; //SPS_PPS_Length
    }

    
	if(ResetPayloadPresentTime == 0)
	{
        //DEBUG_ASF("3. asfAudiPresentTime = %010d\n",asfAudiPresentTime);
        //DEBUG_ASF("3. asfVidePresentTime = %010d\n",asfVidePresentTime);

		ResetPayloadPresentTime = 1;
	    if(asfAudiPresentTime <= asfVidePresentTime)
        {
    		asfVidePresentTime -= (asfAudiPresentTime - AV_TimeBase);
	    	asfAudiPresentTime = AV_TimeBase;
        }
        else
        {
            asfAudiPresentTime     -= (asfVidePresentTime - AV_TimeBase);
    		asfVidePresentTime      = AV_TimeBase;
        }
        //DEBUG_ASF("4. asfAudiPresentTime = %010d\n",asfAudiPresentTime);
        //DEBUG_ASF("4. asfVidePresentTime = %010d\n",asfVidePresentTime);

	}
	asfVidePresentTime += chunkTime;    //Lsk 090409 : add chunkTime before write video payload
    asfTimeStatistics      += chunkTime;

    /* corresponding to payload header, if left size of data packet is not efficiently used, just padding the left size */
    if (asfDataPacketLeftSize < (sizeof(ASF_DTA_VIDEO_PAYLOAD) + ASF_PADDING_THRESHOLD) || asfDataPacketNumPayload == MAX_PAYLOAD_IN_PACKET)  //Lsk 090410
    {
        /* new a data packet */
        if (asfWriteDataPacketPost(pFile, asfDataPacketLeftSize) == 0) /* finish data packet with padding */
            return 0;
        if (asfWriteDataPacketPre(pFile) == 0) /* new a data packet */
            return 0;
    }

    /* save next index entry of 1 sec boundary */
    streamNumber = 0x01;
    #if (VIDEO_CODEC_OPTION == MJPEG_CODEC)
    chunkFlag = 1;
    #endif
    if (chunkFlag & FLAG_I_VOP)
    {
        asfIndexEntryPacketNumber   = asfDataPacketCount - 1; /* data packet index */
        asfIndexEntryPacketCount    = 1;
        streamNumber = 0x81;
        //DEBUG_ASF("I");
    }

    asfDataPacketNumPayload++;
    /* write video payload */
    payloadSize             = sizeof(ASF_DTA_VIDEO_PAYLOAD) + chunkSize;
    subPayloadSize          = (payloadSize <= asfDataPacketLeftSize) ? payloadSize : asfDataPacketLeftSize; /* subpayload size with header */
    payloadSize            -= subPayloadSize; /* left size of payload */
    subPayloadSize         -= sizeof(ASF_DTA_VIDEO_PAYLOAD); /* subpayload size without header */
    offsetIntoMediaObject   = 0;
    /* write video payload header */
    asfDtaPayload.stream_number = streamNumber;
#if FORCE_FPS
    asfDtaPayload.media_object_number       = (u8) (asfVopCount + asfDummyVopCount);
#else
    asfDtaPayload.media_object_number       = (u8) asfVopCount;
#endif
    asfDtaPayload.offset_into_media_object  = offsetIntoMediaObject;
    asfDtaPayload.replicated_data_length    = 0x09;
    asfDtaPayload.replicated_data.lo        = chunkSize;
    asfDtaPayload.replicated_data.hi        = asfVidePresentTime;
    asfDtaPayload.payload_length            = (u16) subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD));
    asfMassWriteDataPoint                  += sizeof(ASF_DTA_VIDEO_PAYLOAD);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD), &size) == 0)
        return 0;
#endif
    asfDataPacketLeftSize                  -= sizeof(ASF_DTA_VIDEO_PAYLOAD);
    /* write video payload data */
    offsetIntoMediaObject                  += subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    if((asfVopCount == 0) && (*(pMng->buffer+4) != 0x67))
    {
        if(subPayloadSize >= H264_SPSPPS_length)
        {
            CopyMemory(asfMassWriteDataPoint, H264_config, H264_SPSPPS_length);
            asfMassWriteDataPoint    += H264_SPSPPS_length;
            CopyMemory(asfMassWriteDataPoint, pChunkBuf, subPayloadSize - H264_SPSPPS_length);
            asfMassWriteDataPoint    += subPayloadSize - H264_SPSPPS_length;
            pChunkBuf                                  += subPayloadSize - H264_SPSPPS_length;
        }
        else
        {
            CopyMemory(asfMassWriteDataPoint, H264_config, subPayloadSize);
            asfMassWriteDataPoint    += subPayloadSize;
            SPS_PPS_NotReady                            = 1;
            SPS_PPS_WriteSize                           = subPayloadSize;
        }
    }
    else
    {
        CopyMemory(asfMassWriteDataPoint, pChunkBuf, subPayloadSize);
        asfMassWriteDataPoint                  += subPayloadSize;
        pChunkBuf                              += subPayloadSize;
    }
#else
    if ((asfVopCount == 0) && (*(pMng->buffer+4) != 0x67))
    {
        if(subPayloadSize >= H264_SPSPPS_length)
        {
            if (dcfWrite(pFile, (unsigned char*)H264_config, H264_SPSPPS_length, &size) == 0)
                return 0;
            if (dcfWrite(pFile, (unsigned char*)pChunkBuf, subPayloadSize - H264_SPSPPS_length, &size) == 0)
                return 0;
            pChunkBuf                                  += subPayloadSize - H264_SPSPPS_length;
        }
        else
        {
            if (dcfWrite(pFile, (unsigned char*)H264_config, subPayloadSize, &size) == 0)
                return 0;
            SPS_PPS_NotReady                            = 1;
            SPS_PPS_WriteSize                           = subPayloadSize;
        }
    }
    else
    {
        if (dcfWrite(pFile, (unsigned char*)pChunkBuf, subPayloadSize, &size) == 0)
            return 0;
        pChunkBuf                              += subPayloadSize;

    }
#endif
    asfDataPacketLeftSize                  -= subPayloadSize;

    while (payloadSize > 0)
    {
        /* new a data packet */
        if (asfWriteDataPacketPost(pFile, asfDataPacketLeftSize) == 0) /* finish data packet with padding */
            return 0;
        if (asfWriteDataPacketPre(pFile) == 0) /* new a data packet */
            return 0;
        asfDataPacketFormatFlag             = 1;                        //Lsk 090309
        if (chunkFlag & FLAG_I_VOP)
            asfIndexEntryPacketCount++;

        asfDataPacketNumPayload++;
        /* write subsequent video payload */
        payloadSize    += sizeof(ASF_DTA_VIDEO_PAYLOAD);
        subPayloadSize  = (payloadSize <= asfDataPacketLeftSize) ? payloadSize : asfDataPacketLeftSize; /* subpayload size with header */
        payloadSize    -= subPayloadSize; /* left size of payload */
        subPayloadSize -= sizeof(ASF_DTA_VIDEO_PAYLOAD); /* subpayload size without header */
        /* write video payload header */

        asfDtaPayload.stream_number             = streamNumber;
#if FORCE_FPS
        asfDtaPayload.media_object_number       = (u8) (asfVopCount + asfDummyVopCount);
#else
        asfDtaPayload.media_object_number       = (u8) asfVopCount;
#endif
        asfDtaPayload.offset_into_media_object  = offsetIntoMediaObject;
        asfDtaPayload.replicated_data_length    = 0x09;
        asfDtaPayload.replicated_data.lo        = chunkSize;
        asfDtaPayload.replicated_data.hi        = asfVidePresentTime;
        asfDtaPayload.payload_length            = (u16) subPayloadSize;

#if ASF_MASS_WRITE
        CopyMemory(asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD));
        asfMassWriteDataPoint                  += sizeof(ASF_DTA_VIDEO_PAYLOAD);
#else
        if (dcfWrite(pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD), &size) == 0)
            return 0;
#endif

        asfDataPacketLeftSize                  -= sizeof(ASF_DTA_VIDEO_PAYLOAD);


        /* write video payload data */
        offsetIntoMediaObject                  += subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
        if(SPS_PPS_NotReady)    // 無線的第一張 I frame 需加上 NAL header
        {
            CopyMemory(asfMassWriteDataPoint, H264_config + SPS_PPS_WriteSize, H264_SPSPPS_length - SPS_PPS_WriteSize);
            asfMassWriteDataPoint    += H264_SPSPPS_length - SPS_PPS_WriteSize;
            CopyMemory(asfMassWriteDataPoint, pChunkBuf, subPayloadSize - (H264_SPSPPS_length - SPS_PPS_WriteSize));
            asfMassWriteDataPoint    += subPayloadSize - (H264_SPSPPS_length - SPS_PPS_WriteSize);
            pChunkBuf                                  += subPayloadSize - (H264_SPSPPS_length - SPS_PPS_WriteSize);
            SPS_PPS_NotReady                            = 0;
        }
        else
        {
            CopyMemory(asfMassWriteDataPoint, pChunkBuf, subPayloadSize);
            asfMassWriteDataPoint                  += subPayloadSize;
            pChunkBuf                              += subPayloadSize;
        }
#else
        if(SPS_PPS_NotReady)    // 無線的第一張 I frame 需加上 NAL header
        {
            if (dcfWrite(pFile, (unsigned char*)H264_config + SPS_PPS_WriteSize, H264_SPSPPS_length - SPS_PPS_WriteSize, &size) == 0)
                return 0;
            if (dcfWrite(pFile, (unsigned char*)pChunkBuf, subPayloadSize - (H264_SPSPPS_length - SPS_PPS_WriteSize), &size) == 0)
                return 0;
            pChunkBuf                                  += subPayloadSize - (H264_SPSPPS_length - SPS_PPS_WriteSize);
            SPS_PPS_NotReady                            = 0;
        }
        else
        {
            if (dcfWrite(pFile, (unsigned char*)pChunkBuf, subPayloadSize, &size) == 0)
                return 0;
            pChunkBuf                              += subPayloadSize;
        }
#endif
        asfDataPacketLeftSize                  -= subPayloadSize;
    }

    /* set index entry on each 1 sec interval */
    while (asfVidePresentTime >= asfIndexEntryTime)    //Lsk 090309 preroll index object
    {
        asfIndexTable[asfIndexTableIndex].packet_number     = asfIndexEntryPacketNumber;
        asfIndexTable[asfIndexTableIndex++].packet_count    = asfIndexEntryPacketCount;
        asfIndexEntryTime                                  += 1000; /* next index after 1 s = 1000 ms */
/*CY 0629 S*/
        if (asfIndexTableIndex >= ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
        {
            DEBUG_ASF("Trace: Video time (%d sec) reaches limit.\n", asfIndexTableIndex);
            return 0;
        }
/*CY 0629 E*/
    }

    /* advance the index */
    asfVopCount++;
    //asfVidePresentTime += chunkTime;    //Lsk 090409
    //if(chunkTime <= 1)    /* Peter 070104 */
    //if(chunkTime < IISTimeUnit)    /* Peter 070104 */
        //DEBUG_ASF("Video chunkTime == %d, asfVopCount = %d\n", chunkTime, asfVopCount);

    //if(asfCaptureMode == ASF_CAPTURE_OVERWRITE) {
    
        OS_ENTER_CRITICAL();
    	if(((asfVopCount -1) == 0) && (*(pMng->buffer+4) != 0x67))
            CurrentVideoSize   -= (chunkSize - H264_SPSPPS_length);
        else
            CurrentVideoSize   -= chunkSize;
        OS_EXIT_CRITICAL();
    //}

    return 1;
}

#if FORCE_FPS

/*

Routine Description:

    Write dummy video payload with MPEG-4 no coded frame.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteDummyVidePayload(FS_FILE* pFile)
{
    u32 size;
    u32 chunkFlag, chunkTime, chunkSize;
    u8* pChunkBuf;
    u32 payloadSize;
    u32 subPayloadSize;
    //u32 writeSize;
    u32 offsetIntoMediaObject;
    u8  streamNumber;

    __align(4) ASF_DTA_VIDEO_PAYLOAD asfDtaPayload =
    {
        0x81,               /* stream_number = 0x81, StreamNumber = 1, KeyFrameBit = 1, (video) */
        0x00,               /* media_object_number = 0x??, n for Vn, e.g. 0x00 for V0, 0x01 for V1, ..., (video)    */
                        /*             = 0x??, n for An, e.g. 0x00 for A0, 0x01 for A1, ..., (audio)    */
        0x00000000,         /* offset_into_media_object = 0x00000000 =              */
                        /*      byte offset of Vn/An corresponding to start of this payload     */
                        /*  non-zero when Vn/An across multiple payloads            */
        0x09,               /* replicated_data_length = 0x08 */
        {0x00000000, 0x00000000},   /* replicated_data = {0x????????, 0x????????} =                     */
                        /*  {size of Vn/An, byte offset of of Vn/An}                    */
                        /*  only existed at 1st payload of Vn/An when Vn/An across multiple payloads    */

		0x00,           /* interlace and top field first */

        0x0000,         /* payload_length = 0x???? = size of this payload   */
                        /*  different with 1st field of replicated_data only when Vn/An across multiple payloads    */
                        /* payload_data[0x????????] */
    };

#if 0
    u8 mpeg4VOPHeader[7] =
    {
        0x00, 0x00, 0x01, 0xB6,
        0x5e, 0xa6, 0x13
    };
#else
    u8 mpeg4VOPHeader[256] =
    {
        0x00
    };
#endif

    DEBUG_ASF("D");

    chunkSize   = 0;
    mpeg4PutDummyVOPHeader(mpeg4Width, mpeg4Height,(u8*)mpeg4VOPHeader, &chunkSize);

    if(ASF_set_interlace_flag==0)
    {
        asfDtaPayload.flag =  0xc0; 			/* interlace and top field first */
    }

    PayloadType             = 1;
    asfDataPacketFormatFlag = 1;                        //Lsk 090309
    chunkFlag               = !FLAG_I_VOP;
    chunkTime               = 1;
    //chunkSize               = sizeof(mpeg4VOPHeader);
    pChunkBuf               = (u8*)mpeg4VOPHeader;
	if(ResetPayloadPresentTime == 0)
	{
		ResetPayloadPresentTime = 1;
	    if(asfAudiPresentTime <= asfVidePresentTime)
        {
    		asfVidePresentTime -= (asfAudiPresentTime - AV_TimeBase);
	    	asfAudiPresentTime = AV_TimeBase;
        }
        else
        {
            asfAudiPresentTime     -= (asfVidePresentTime - AV_TimeBase);
    		asfVidePresentTime      = AV_TimeBase;
        }
	}
	asfVidePresentTime     += chunkTime;    //Lsk 090409 : add chunkTime before write video payload

    /* corresponding to payload header, if left size of data packet is not efficiently used, just padding the left size */
    if (asfDataPacketLeftSize < (sizeof(ASF_DTA_VIDEO_PAYLOAD) + ASF_PADDING_THRESHOLD) || asfDataPacketNumPayload == MAX_PAYLOAD_IN_PACKET)  //Lsk 090410
    {
        /* new a data packet */
        if (asfWriteDataPacketPost(pFile, asfDataPacketLeftSize) == 0) /* finish data packet with padding */
            return 0;
        if (asfWriteDataPacketPre(pFile) == 0) /* new a data packet */
            return 0;
    }

    streamNumber = 0x01;

    asfDataPacketNumPayload++;
    /* write video payload */
    payloadSize                             = sizeof(ASF_DTA_VIDEO_PAYLOAD) + chunkSize;
    subPayloadSize                          = (payloadSize <= asfDataPacketLeftSize) ? payloadSize : asfDataPacketLeftSize; /* subpayload size with header */
    payloadSize                            -= subPayloadSize; /* left size of payload */
    subPayloadSize                         -= sizeof(ASF_DTA_VIDEO_PAYLOAD); /* subpayload size without header */
    offsetIntoMediaObject                   = 0;
    /* write video payload header */
    asfDtaPayload.stream_number             = streamNumber;
    asfDtaPayload.media_object_number       = (u8) (asfVopCount + asfDummyVopCount);
    asfDtaPayload.offset_into_media_object  = offsetIntoMediaObject;
    asfDtaPayload.replicated_data_length    = 0x09;
    asfDtaPayload.replicated_data.lo        = chunkSize;
    asfDtaPayload.replicated_data.hi        = asfVidePresentTime;
    asfDtaPayload.payload_length            = (u16) subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD));
    asfMassWriteDataPoint                  += sizeof(ASF_DTA_VIDEO_PAYLOAD);
#else
    if (dcfWrite(pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD), &size) == 0)
        return 0;
#endif
    asfDataPacketLeftSize                  -= sizeof(ASF_DTA_VIDEO_PAYLOAD);
    /* write video payload data */
    offsetIntoMediaObject                  += subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
    CopyMemory(asfMassWriteDataPoint, pChunkBuf, subPayloadSize);
    asfMassWriteDataPoint                  += subPayloadSize;
#else
    if (dcfWrite(pFile, (unsigned char*)pChunkBuf, subPayloadSize, &size) == 0)
        return 0;
#endif
    pChunkBuf                              += subPayloadSize;
    asfDataPacketLeftSize                  -= subPayloadSize;

    while (payloadSize > 0)
    {
        /* new a data packet */
        if (asfWriteDataPacketPost(pFile, asfDataPacketLeftSize) == 0) /* finish data packet with padding */
            return 0;
        if (asfWriteDataPacketPre(pFile) == 0) /* new a data packet */
            return 0;
        asfDataPacketFormatFlag = 1;                        //Lsk 090309
        if (chunkFlag & FLAG_I_VOP)
            asfIndexEntryPacketCount++;

        asfDataPacketNumPayload++;
        /* write subsequent video payload */
        payloadSize                        += sizeof(ASF_DTA_VIDEO_PAYLOAD);
        subPayloadSize                      = (payloadSize <= asfDataPacketLeftSize) ? payloadSize : asfDataPacketLeftSize; /* subpayload size with header */
        payloadSize                        -= subPayloadSize; /* left size of payload */
        subPayloadSize                     -= sizeof(ASF_DTA_VIDEO_PAYLOAD); /* subpayload size without header */
        /* write video payload header */

        asfDtaPayload.stream_number             = streamNumber;
        asfDtaPayload.media_object_number       = (u8) (asfVopCount + asfDummyVopCount);
        asfDtaPayload.offset_into_media_object  = offsetIntoMediaObject;
        asfDtaPayload.replicated_data_length    = 0x09;
        asfDtaPayload.replicated_data.lo        = chunkSize;
        asfDtaPayload.replicated_data.hi        = asfVidePresentTime;
        asfDtaPayload.payload_length            = (u16) subPayloadSize;

#if ASF_MASS_WRITE
        CopyMemory(asfMassWriteDataPoint, &asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD));
        asfMassWriteDataPoint  += sizeof(ASF_DTA_VIDEO_PAYLOAD);
#else
        if (dcfWrite(pFile, (unsigned char*)&asfDtaPayload, sizeof(ASF_DTA_VIDEO_PAYLOAD), &size) == 0)
            return 0;
#endif

        asfDataPacketLeftSize  -= sizeof(ASF_DTA_VIDEO_PAYLOAD);


        /* write video payload data */
        offsetIntoMediaObject  += subPayloadSize;
#if ASF_MASS_WRITE    /* Peter 070104 */
        CopyMemory(asfMassWriteDataPoint, pChunkBuf, subPayloadSize);
        asfMassWriteDataPoint  += subPayloadSize;
#else
        if (dcfWrite(pFile, (unsigned char*)pChunkBuf, subPayloadSize, &size) == 0)
            return 0;
#endif
        pChunkBuf              += subPayloadSize;
        asfDataPacketLeftSize  -= subPayloadSize;
    }

    /* set index entry on each 1 sec interval */
    while (asfVidePresentTime >= asfIndexEntryTime)    //Lsk 090309 preroll index object
    {
        asfIndexTable[asfIndexTableIndex].packet_number     = asfIndexEntryPacketNumber;
        asfIndexTable[asfIndexTableIndex++].packet_count    = asfIndexEntryPacketCount;
        asfIndexEntryTime                                  += 1000; /* next index after 1 s = 1000 ms */
/*CY 0629 S*/
        if (asfIndexTableIndex >= ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
        {
            DEBUG_ASF("Trace: Video time (%d sec) reaches limit.\n", asfIndexTableIndex);
            return 0;
        }
/*CY 0629 E*/
    }

    /* advance the index */
    asfDummyVopCount++;
    DummyChunkTime++;

    return 1;
}

#endif  // FORCE_FPS
#endif //( ((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2)|| (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) || (!MULTI_CHANNEL_VIDEO_REC))
/*-------------------------------------------------------------------------*/
/* Index object                                */
/*-------------------------------------------------------------------------*/

/*

Routine Description:

    Write index object.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteIndexObject(FS_FILE* pFile)
{
    u32 size;
    u32 length;
    __align(4) ASF_SIMPLE_INDEX_OBJECT asfSimpleIndexObject =
    {
        {0x90, 0x08, 0x00, 0x33,    /* object_id = ASF_Simple_Index_Object */
         0xb1, 0xe5, 0xcf, 0x11,
         0x89, 0xf4, 0x00, 0xa0,
         0xc9, 0x03, 0x49, 0xcb},
        {0x00000000, 0x00000000},   /* object_size = 0x???????????????? */
        {0x00, 0x00, 0x00, 0x00,    /* file_id = file_id of asf_data_object and asf_simple_index_object */
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00},
        {0x00989680, 0x00000000},   /* index_entry_time_interval = 0x0000000000989680 100-nanosecond = 1 second */
        0x00000005,         /* max_packet_count = 0x00000005 = maximum packet_count */
        0x00000000,         /* index_entry_count = 0x???????? */
                        /* asf_idx_simple_index_ent[0x????????] */
    };

    asfSimpleIndexObject.object_size.lo = asfIndexSize;
    asfSimpleIndexObject.index_entry_count = asfIndexTableIndex;
#if 0
    if (dcfWrite(pFile, (unsigned char*)&asfSimpleIndexObject, sizeof(ASF_SIMPLE_INDEX_OBJECT), &size) == 0)
            return 0;
    if (dcfWrite(pFile, (unsigned char*)asfIndexTable, asfIndexTableIndex * sizeof(ASF_IDX_SIMPLE_INDEX_ENTRY), &size) == 0) //Lsk 090303
            return 0;
#else //Lucian optimize: 將index table 一次寫入,以避免破碎寫入
    memcpy((unsigned char*)mpeg4IndexBuf, (unsigned char*)&asfSimpleIndexObject, sizeof(ASF_SIMPLE_INDEX_OBJECT));
    length=asfIndexTableIndex * sizeof(ASF_IDX_SIMPLE_INDEX_ENTRY)+sizeof(ASF_SIMPLE_INDEX_OBJECT);
    memset((mpeg4IndexBuf+length),0,512-(length%512));
    length= ((length+511)/512)*512;  //sector alignment
	#if(ASF_WRITE_PATH==ASF_WRITE_COPY)
    if (dcfBackupWrite(pFile, (unsigned char*)mpeg4IndexBuf, length, &size) == 0) //Lsk 090303
            return 0;	
	#else
    if (dcfWrite(pFile, (unsigned char*)mpeg4IndexBuf, length, &size) == 0) //Lsk 090303
            return 0;
	#endif
#endif
    return 1;
}


/*

Routine Description:

    Read index object.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfReadIndexObject(FS_FILE* pFile)
{
    u32 size;
    __align(4) ASF_SIMPLE_INDEX_OBJECT asfSimpleIndexObject;


    if(dcfRead(pFile, (u8*)&asfSimpleIndexObject, sizeof(ASF_SIMPLE_INDEX_OBJECT), &size) != 1) {
        DEBUG_ASF("Read file to asfSimpleIndexObject fail!!!\n");
        return 0;
    }

    asfIndexTable       = (ASF_IDX_SIMPLE_INDEX_ENTRY*)mpeg4IndexBuf;
    asfIndexSize        = asfSimpleIndexObject.object_size.lo;
    asfIndexTableCount  = asfSimpleIndexObject.index_entry_count;

    if(dcfRead(pFile, (u8*)asfIndexTable, asfIndexSize - sizeof(ASF_SIMPLE_INDEX_OBJECT), &size) != 1) {
        DEBUG_ASF("Read file to asfIndexTable fail!!!\n");
        return 0;
    }

    return 1;
}

/*

Routine Description:

    Burst read index object.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfBurstReadIndexObject(FS_FILE* pFile, u32 IndexObjectSize)
{

	/*
    u32 size;
    ASF_SIMPLE_INDEX_OBJECT* asfSimpleIndexObject;

	if(dcfRead(pFile, mpeg4IndexBuf, IndexObjectSize, &size) != 1) {
        DEBUG_ASF("Read file to asfSimpleIndexObject fail!!!\n");
        return 0;
    }

	asfSimpleIndexObject = (ASF_SIMPLE_INDEX_OBJECT *)mpeg4IndexBuf;
	asfIndexSize        = asfSimpleIndexObject->object_size.lo;
    asfIndexTableCount  = asfSimpleIndexObject->index_entry_count;
	asfIndexTable       = (ASF_IDX_SIMPLE_INDEX_ENTRY*)(mpeg4IndexBuf + sizeof(ASF_SIMPLE_INDEX_OBJECT));
	*/

	u32 size;
    ASF_SIMPLE_INDEX_OBJECT* asfSimpleIndexObject;

	if(dcfRead(pFile, mpeg4IndexBuf, IndexObjectSize, &size) != 1) {
        DEBUG_ASF("Read file to asfSimpleIndexObject fail!!!\n");
        return 0;
    }
	if(matchGuid(mpeg4IndexBuf) == ASF_Simple_Index_Object_Guid) {
		asfSimpleIndexObject = (ASF_SIMPLE_INDEX_OBJECT *)mpeg4IndexBuf;
		asfIndexSize        = asfSimpleIndexObject->object_size.lo;
    	asfIndexTableCount  = asfSimpleIndexObject->index_entry_count;
		asfIndexTable       = (ASF_IDX_SIMPLE_INDEX_ENTRY*)(mpeg4IndexBuf + sizeof(ASF_SIMPLE_INDEX_OBJECT));
		return 1;
	}
	else
		return 0;

}

/*

Routine Description:

    Find index object in the asf file.

Arguments:

    pFile - File handle.

Return Value:

    0         - Failure.
    Otherwise - The offset value.

*/
s32 asfFindIndexObject(FS_FILE* pFile)
{
    u32         size, match;
    s32      CurrentOffset, IndexOffset;
    ASF_HEADER  asfHeader;
    u8          *buf;

    CurrentOffset   = dcfTell(pFile);
    IndexOffset     = 0;
    match           = 0;

    while(1) {
        if(dcfSeek(pFile, IndexOffset, FS_SEEK_SET)==0) {
            DEBUG_ASF("ASF file seek to IndexOffset(%d) error!!!\n", IndexOffset);
            break;
        }

        if(dcfRead(pFile, (u8*)&asfHeader, sizeof(ASF_HEADER), &size) == 0) {
            DEBUG_ASF("Read file to asfHeader fail!!!\n");
            break;
        }
        buf = (u8*)&asfHeader;

        if(matchGuid(buf) == ASF_Simple_Index_Object_Guid) {
            if(dcfSeek(pFile, IndexOffset, FS_SEEK_SET)==0) {
                DEBUG_ASF("ASF file seek to IndexOffset(%d) error!!!\n", IndexOffset);
                break;
            }
            if(asfReadIndexObject(pFile)) {
                match   = 1;
            }
            break;
        } else {
            if(asfHeader.object_size.lo == 0) {
                DEBUG_ASF("asfHeader.object_size.lo == 0!!!\n");
                break;
            }
            IndexOffset    += asfHeader.object_size.lo;
        }
    }

    if(dcfSeek(pFile, CurrentOffset, FS_SEEK_SET)==0) {
        DEBUG_ASF("ASF file seek to CurrentOffset(%d) error!!!\n", CurrentOffset);
        return 0;
    }

    if(match) {
        return  IndexOffset;
    } else {
        DEBUG_ASF("Can't find index object header!!!\n");
        return  0;
    }
}

/*

Routine Description:

    Read header object.

Arguments:

    buf                             - Point to header start address.
    asfHeaderObject                 - Point to header object.
    asfFilePropertiesObject         - Point to file properties object.
    asfAudiStreamPropertiesObject   - Point to audi stream properties object.
    VideoStreamNum                  - Point to video stream number.
    AudioStreamNum                  - Point to audio stream number.

Return Value:

    0           - Failure.
    Otherwise   - Point to header end.

*/
u8* asfReadHeaderObject(u8                                  *buf,
                        ASF_HEADER_OBJECT                   *asfHeaderObject,
                        ASF_FILE_PROPERTIES_OBJECT          *asfFilePropertiesObject,
                        ASF_AUDI_STREAM_PROPERTIES_OBJECT   *asfAudiStreamPropertiesObject,
                        u8                                  *VideoStreamNum,
                        u8                                  *AudioStreamNum)
{
    u64 Size, UsedSize;
    u32 *pp;    
    u32 i, thumbnailSize;
	#if(VIDEO_CODEC_OPTION == H264_CODEC)
	#if !(TUTK_SUPPORT)
    u32 Width = 0;
    u32 Height = 0;
	#endif
	#endif
    //DEBUG_ASF("Match ASF_Header_Object_Guid\n");
    memcpy(asfHeaderObject, buf, sizeof(ASF_HEADER_OBJECT));
    buf            += sizeof(ASF_HEADER_OBJECT);
    UsedSize.lo     = sizeof(ASF_HEADER_OBJECT);
    while(UsedSize.lo < asfHeaderObject->object_size.lo)
    {
        switch(matchHeaderObjectGuid(buf))
        {
            case ASF_File_Properties_Object_GUID:
                //DEBUG_ASF("Match ASF_File_Properties_Object_GUID\n");
                Size.lo         = sizeof(ASF_FILE_PROPERTIES_OBJECT);
                memcpy(asfFilePropertiesObject, buf, Size.lo);
                buf            += Size.lo;// + (Size.hi<<32);
                UsedSize.lo    += Size.lo;// + (Size.hi<<32);
               	{//Lsk 090507 : calculate file time
                s64 temp;
				temp = asfFilePropertiesObject->play_duration.hi;
				temp = ((temp << 32) +  asfFilePropertiesObject->play_duration.lo);
				VideoDuration = (u32)(temp / 10000000);
               	}
                if(asfFilePropertiesObject->maximum_data_packet_size > ASF_DATA_PACKET_SIZE) {
                    DEBUG_ASF("ASF_File_Properties_Object_GUID.maximum_data_packet_size(0x%x) > ASF_DATA_PACKET_SIZE(0x%x)\n",
                                asfFilePropertiesObject->maximum_data_packet_size, ASF_DATA_PACKET_SIZE);
                    return  0;
                }
                break;
            case ASF_Stream_Properties_Object_GUID:
                //DEBUG_ASF("Match ASF_Stream_Properties_Object_GUID\n");
                //Parsing stream properties ==> video , audio ... ... etc

                switch(matchStreamTypeGuid(buf + 16 + 8))
                {
                    case ASF_Audio_Media_GUID:

                        /* Peter 070104 */
                        memcpy(asfAudiStreamPropertiesObject, buf, sizeof(ASF_AUDI_STREAM_PROPERTIES_OBJECT));
                    #if (AUDIO_CODEC == AUDIO_CODEC_PCM )
                        iisPlayFormat.wFormatTag        = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.codec_id_format_tag;
                        iisPlayFormat.nChannels         = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.number_of_channels;
                        iisPlayFormat.nSamplesPerSec    = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.samples_per_second;
                        iisPlayFormat.nAvgBytesPerSec   = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.avg_num_of_bytes_per_sec;
                        iisPlayFormat.nBlockAlign       = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.block_alignment;
                        iisPlayFormat.wBitsPerSample    = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.bits_per_sample;
                        iis5SetPlayFormat(&iisPlayFormat);
                        if(iisPlayFormat.nAvgBytesPerSec == 0) {
                            DEBUG_ASF("asfAudiStreamPropertiesObject.asf_hdr_audi_type_specific_dta.avg_num_of_bytes_per_sec == 0\n");
                            return 0;
                        }
                        //IISTimeUnit                     = IIS_CHUNK_SIZE * 1000000 / iisPlayFormat.nAvgBytesPerSec;
                        IISTimeUnit                     = (IIS_PLAYBACK_SIZE * 1000000) / iisPlayFormat.nAvgBytesPerSec;
                        //asfAudiChunkCount               = AviHeader->StrHdr_aud.Length;
                    #elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM )
                        iisPlayFormat.wFormatTag        = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.codec_id_format_tag;
                        iisPlayFormat.nChannels         = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.number_of_channels;
                        iisPlayFormat.nSamplesPerSec    = IIS_SAMPLE_RATE;
                        iisPlayFormat.nAvgBytesPerSec   = IIS_SAMPLE_RATE;
                        iisPlayFormat.nBlockAlign       = 1;
                        iisPlayFormat.wBitsPerSample    = 8;
                        iisSetPlayFormat(&iisPlayFormat);
                        IISTimeUnit                     = (IIS_PLAYBACK_SIZE * 1000000) / iisPlayFormat.nAvgBytesPerSec;
                    #endif

                        //AudioStreamNum = (*(buf+72) & 0x7F);
                        *AudioStreamNum = asfAudiStreamPropertiesObject->flags & 0x7F;
                        //DEBUG_ASF("Audio : %d\n",AudioStreamNum);

                        break;
                    case ASF_Video_Media_GUID:
                        *VideoStreamNum = (*(buf+72) & 0x7F);
                        //DEBUG_ASF("Video : %d\n",VideoStreamNum);
                        // buf + 0x8F // VOL Header
                        #if ASF_SPLIT_FILE
                            ;
                        #else
                        #if(VIDEO_CODEC_OPTION == H264_CODEC)
                          #if TUTK_SUPPORT
                            gPlaybackWidth = (*(buf+94)<<8)+ *(buf+93);
                            gPlaybackHeight = (*(buf+98)<<8)+ *(buf+97);

							asfVopWidth  = gPlaybackWidth;
							asfVopHeight = gPlaybackHeight;

                            if(sysPlaybackVideoStart == 1)
                            {
                                if(sysTVOutOnFlag)
                                {
                                #if TVOUT_DYNAMIC
                                  #if UI_SYNCHRONOUS_DUAL_OUTPUT
                                   #if PLAYBACK_FIRST_IFRAME
                                    if(Iframe_flag == 1)
                                   #else
                                    if (1)
                                   #endif
                                    {
                                        asfSwitchSource(SYS_OUTMODE_TV);
                                        sysTVswitchResolutionbyImagesize();
                                        if(asfVopWidth == 640)
                                            iduPlaybackMode(640,352,640);
                                        else if (asfVopWidth == 1280)
                                            iduPlaybackMode(1280,720,1280);
                                        else if (asfVopWidth == 1920)
                                        {
                                            if(isCap1920x1080I() == 1)
                                                iduPlaybackMode(1920,1080,1920);
                                            else
                                            iduPlaybackMode(960,720,1280);
                                        }
                                        
                                        if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
                                           tvTVE_INTC   =TV_INTC_FRMEND__ENA;
                                        else
                                           tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
                                    }
                                  #else
                                        if(asfVopWidth == 640)
                                            iduPlaybackMode(640,352,640);
                                        else if (asfVopWidth == 1280)
                                            iduPlaybackMode(1280,720,1280);
                                        else if (asfVopWidth == 1920)
                                        {
                                            if(isCap1920x1080I() == 1)
                                                iduPlaybackMode(1920,1080,1920);
                                            else
                                                iduPlaybackMode(960,720,1280);
                                        }
                                        if( (TvOutMode == SYS_TV_OUT_HD720P60) || (TvOutMode == SYS_TV_OUT_HD720P60_37M) || (TvOutMode == SYS_TV_OUT_HD720P30) || (TvOutMode == SYS_TV_OUT_HD720P25) || (TvOutMode == SYS_TV_OUT_FHD1080P30) || (TvOutMode == SYS_TV_OUT_FHD1080P25) )
                                           tvTVE_INTC   =TV_INTC_FRMEND__ENA;
                                        else
                                           tvTVE_INTC   =TV_INTC_BOTFDSTART_ENA;
                                  #endif
                                #endif
                                }
                                else
                                {
                                    if(sysPlaybackThumbnail == READFILE_THUMBNAIL_PREVIEW)
                                    {

                                    }
                                    else
                                    {
                                    if(gPlaybackWidth == 640)
                                        iduPlaybackMode(640,352,640);
                                    else if (gPlaybackWidth == 1280)
                                        iduPlaybackMode(1280,720,1280);
                                    else if ((asfVopWidth == 1920)&&(asfVopHeight == 1072))
                                        iduPlaybackMode(960,536,1280);
                                    else if ((asfVopWidth == 1920)&&(asfVopHeight == 1080))
                                        iduPlaybackMode(960,540,1280);
                                    IduIntCtrl |= 0x00000100;
                                }
                            }
                            }
                          #else
                            Width = (*(buf+94)<<8)+ *(buf+93);
                            Height = (*(buf+98)<<8)+ *(buf+97);

							asfVopWidth  = Width;
							asfVopHeight = Height;							
							
                            if((sysPlaybackVideoStart == 1) && (!sysTVOutOnFlag))
                            {
                                if(Width == 640)
                                    iduPlaybackMode(640,352,640);
                                else if (Width == 1280)
                                    iduPlaybackMode(1280,720,1280);
                                else if ((Width == 1920)&&(Height == 1072))
                                    iduPlaybackMode(960,536,1280);
                                else if ((Width == 1920)&&(Height == 1080))
                                    iduPlaybackMode(960,540,1280);
                                IduIntCtrl |= 0x00000100;
                            }
                          #endif
                        #elif(VIDEO_CODEC_OPTION == MPEG4_CODEC)
                        if(mpeg4DecodeVolHeader(&Mp4Dec_opt,buf + 0x8B, 0x15) == 0) {
                            return 0;
                        }
                        #endif
                        #endif
                        break;
                    default:
                        return 0;
                        //CloseFlag = 0;
                }

                Size            = *((u64 *)(buf + 16));
                buf            += Size.lo;// + (Size.hi<<32);
                UsedSize.lo    += Size.lo;// + (Size.hi<<32);
                break;
            case ASF_Header_Extension_Object_GUID:
                //DEBUG_ASF("Match ASF_Header_Extension_Object_GUID\n");
                Size            = *((u64 *)(buf+16)); // Object size
                buf            += Size.lo;// + (Size.hi<<32);
                UsedSize.lo    += Size.lo;// + (Size.hi<<32);
                break;
            case ASF_Codec_List_Object_GUID:
                //DEBUG_ASF("Match ASF_Codec_List_Object_GUID\n");
                Size            = *((u64 *)(buf+16)); // Object size
                buf            += Size.lo;// + (Size.hi<<32);
                UsedSize.lo    += Size.lo;// + (Size.hi<<32);
                break;
            case ASF_Content_Description_Object_GUID:
                //DEBUG_ASF("Match ASF_Content_Description_Object_GUID\n");
                Size            = *((u64 *)(buf+16)); // Object size
                buf            += Size.lo;// + (Size.hi<<32);
                UsedSize.lo    += Size.lo;// + (Size.hi<<32);
                break;
            case ASF_Padding_Object_GUID:
                //DEBUG_ASF("Match ASF_Padding_Object_GUID\n");
                Size             = *((u64 *)(buf+16)); // Object size
//                Video_60fps_flag = *((u8*)(buf+24)); // Video_60fps_flag
//                show_flag = 0;

                #if (THUMBNAIL_PREVIEW_ENA == 1) //TED
                if(sysPlaybackThumbnail == READFILE_THUMBNAIL_PREVIEW)                
                {
                                     
                    if((buf[24] == thumbnailID[0]) && (buf[25] == thumbnailID[1]) 
                        && (buf[26] == thumbnailID[2]) && (buf[27] == thumbnailID[3])
                        && (buf[28] == thumbnailID[4]) && (buf[29] == thumbnailID[5]) 
                        && (buf[30] == thumbnailID[6]) && (buf[31] == thumbnailID[7]))
                    {
                        thumbnailSize = (((buf[35])<<24), ((buf[34])<<16), ((buf[33])<<8)+ ((buf[32])));
                        if(thumbnailSize >= 10)
                        {
                            VideoBufMng[VideoBufMngWriteIdx].flag = 1; //must be I frame
                            VideoBufMng[VideoBufMngWriteIdx].time = 3000; //don't care
                            VideoBufMng[VideoBufMngWriteIdx].size = sizeof(H264_config) + thumbnailSize;                    

                            CopyMemory(VideoBufMng[VideoBufMngWriteIdx].buffer, (buf+36), VideoBufMng[VideoBufMngWriteIdx].size);
                            OSSemPost(VideoTrgSemEvt);
                            mpeg4ResumeTask();
                        }
                    }
                }
                #endif
                
                buf            += Size.lo;// + (Size.hi<<32);
                UsedSize.lo    += Size.lo;// + (Size.hi<<32);
                break;
            default:
                break;
        }
    }

    return buf;
}

/*

Routine Description:

    Read packet header.

Arguments:

    buf                     - Point to header start address.
    asfFilePropertiesObjec  - Point to file properties object.
    PacketUsedSize          - Point to file packet used size.
    flag2                   - Point to payload parsing info.
    Mul_Payload             - Point to multiple payload flag.
    PacketLength            - Point to packet length.
    UsedSize                - Point to used size.

Return Value:

    0           - Failure.
    Otherwise   - Point to header end.

*/
u8* asfReadPacketHeader(u8                          *buf,
                        ASF_FILE_PROPERTIES_OBJECT  *asfFilePropertiesObject,
                        u32                         *PacketUsedSize,
                        u8                          *flag2,
                        u8                          *Mul_Payload,
                        u32                         *PacketLength,
                        u32                         *PaddingLength,//Lsk 090304
                        u64                         *UsedSize)
{
    u8      flag, length;
    //u32     SendTime;
    //u16     Duration;

    *PacketUsedSize = (u32)buf;
    flag            = *(buf);
    if(flag && 0x80)
    { // Error correction present
        // Error Correct Data
        length          = (flag & 0x0F);
        buf            += length + 1;
        UsedSize->lo   += length + 1;
        flag            = *(buf);
    }
    // Payload Parsing Info
    *flag2          = *(buf + 1);
    //flag  => Length Type Flags  flag2 => Property Flags
    buf            += 2; // flag & flag2
    UsedSize->lo   += 2; // flag & flag2

    if(flag & 0x01)
    {
        //DEBUG_ASF("Multi-Payload not support\n");
        *Mul_Payload    = 1;
        //return;
    }
    else   //Lsk 090304
    {
        *Mul_Payload    = 0;
    }

    // PacketLength
    switch((flag & 0x60) >> 5)
    {
        case 0:
            if(asfFilePropertiesObject->maximum_data_packet_size == asfFilePropertiesObject->minimum_data_packet_size)
                *PacketLength   = asfFilePropertiesObject->maximum_data_packet_size;
            else
            {
                //DEBUG_ASF("Packet length not specification\n");
                return 0;
                //CloseFlag = 0;
            }
            break;
        case 1:
            *PacketLength   = *buf;
            buf            += 1;
            UsedSize->lo   += 1;
            break;
        case 2:
            *PacketLength   = *buf + *(buf + 1) << 8;
            buf            += 2;
            UsedSize->lo   += 2;
            break;
        case 3:
            *PacketLength   = *buf + (*(buf + 1) << 8) + (*(buf + 2) << 16) + (*(buf + 3) << 24);
            buf            += 4;
            UsedSize->lo   += 4;
            break;
    }
    //DEBUG_ASF("Packet Length: %d\n",PacketLength);
    //Sequence
    switch((flag & 0x06)>>1)
    {
        case 0:
            break;
        case 1:
        case 2:
        case 3:
            //DEBUG_ASF("Sequence should be reserved\n");
            break;
    }

    // PaddingLength
    switch((flag & 0x18)>>3)
    {
        case 0:
            //DEBUG_ASF("Not PaddingLength\n");
            break;
        case 1:
            *PaddingLength   = *buf;
            buf            += 1;
            UsedSize->lo   += 1;
            break;
        case 2:
            *PaddingLength   = *buf + (*(buf + 1) << 8);
            buf            += 2;
            UsedSize->lo   += 2;
            break;
        case 3:
            *PaddingLength   = *buf + (*(buf + 1) << 8) + (*(buf + 2) << 16) + (*(buf + 3) << 24);
            buf            += 4;
            UsedSize->lo   += 4;
            break;
    }
    //DEBUG_ASF("PaddingLength: %d\n",PaddingLength);

    //SendTime =  // 32 bit
    //SendTime        = *buf + (*(buf+1)<<8) + (*(buf+2)<<16) + (*(buf+3)<<24);
    //DEBUG_ASF("SendTime: %d\n",SendTime);
    buf            += 4;
    UsedSize->lo   += 4;
    
    //Duration        = *buf + (*(buf+1)<<8);
    //DEBUG_ASF("Duration: %d\n",Duration);
    buf            += 2;
    UsedSize->lo   += 2;

    return  buf;
}


/*

Routine Description:

    Read payload header.

Arguments:

    buf             - Point to header start address.
    StreamNum       - Point to stream number.
    KeyFrame        - Point to key frame flag.
    flag2           - flag2.
    Restart1        - 1: video restart, 0: otherwise.
    Restart2        - 1: audio restart, 0: otherwise.
    PayloadFlag     - payload flag.
    Offset2Media    - Point to byte offset to media.
    PayloadLength   - Point to payload length.
    asfPresentTime  - Point to asf present time.
    asfVideoIndex   - Point to video index.
    asfAudioIndex   - Point to audio index.
    UsedSize        - Point to used size.

Return Value:

    0           - Failure.
    Otherwise   - Point to header end.

*/
u8* asfReadPayloadHeader(u8     *buf,
                         u8     *StreamNum,
                         u8     *KeyFrame,
                         u8     flag2,
                         u8     Restart1,
                         u8     Restart2,
                         u8     PayloadFlag,
                         u32    *Offset2Media,
                         u32    *PayloadLength,
                         u32    *MediaObjectSize,  ////Lsk 090622
                         s64    *asfPresentTime,
                         u32    *asfVideoIndex,
                         u32    *asfAudioIndex,
                         u64    *UsedSize,
                         u8     Mul_Payload,   //Lsk 090304
                         u32    PacketLength,
                         u32    PaddingLength)
{
    //u32 MediaObjectNumber;
	u32	ReplicatedDataLength;

    *StreamNum      = ((*buf) & 0x7F);
    *KeyFrame       = ((*buf) & 0x80) ? 1 : 0;
    //DEBUG_ASF("StreamNum : %d\n",StreamNum);
    buf            += 1; // Stream number
    UsedSize->lo   += 1; // Stream number

    switch((flag2 & 0x30) >> 4) // Media Object Number Length
    {
        case 0:
            //MediaObjectNumber   = 0;
            break;
        case 1:
            //MediaObjectNumber   = *buf;
            //DEBUG_ASF("Media Object Number 1: %x\n",(*buf));
            buf                += 1;
            UsedSize->lo       += 1;
            break;
        case 2:
            //MediaObjectNumber   = (*buf) + ((*(buf + 1)) << 8);
            //DEBUG_ASF("Media Object Number 2: %x\n",(*buf) + ((*(buf+8))<<8));
            buf                += 2;
            UsedSize->lo       += 2;
            break;
        case 3:
            //MediaObjectNumber   = (*buf) + ((*(buf + 1)) << 8) + ((*(buf + 2)) << 16) + ((*(buf + 3)) << 24);
            //DEBUG_ASF("Media Object Number 3: %x\n",(*buf) + ((*(buf+8))<<8) + ((*(buf+16))<<16) + ((*(buf+24))<<24));
            buf                += 4;
            UsedSize->lo       += 4;
            break;
    }

    if(Restart1 && *StreamNum == 1) {
        //asfVideoIndex   = MediaObjectNumber;
        *asfVideoIndex  = 0;
    }
    if(Restart2 && *StreamNum == 2) {
        //asfAudioIndex   = MediaObjectNumber;
        *asfAudioIndex  = 0;
    }

    switch((flag2 & 0x0C) >> 2) // Offset Into Media Object
    {
        case 0:
            break;
        case 1:
            *Offset2Media   = (*buf);
            //DEBUG_ASF("Offset Into Media Object: %x\n",Offset2Media);
            buf            += 1;
            UsedSize->lo   += 1;
            break;
        case 2:
            *Offset2Media   = (*buf)+ ((*(buf+1))<<8);
            //DEBUG_ASF("Offset Into Media Object: %x\n",Offset2Media);
            buf            += 2;
            UsedSize->lo   += 2;
            break;
        case 3:
            *Offset2Media   = (*buf)+ ((*(buf+1))<<8) + ((*(buf+2))<<16) + ((*(buf+3))<<24);
            //DEBUG_ASF("Offset Into Media Object: %x\n",Offset2Media);
            buf            += 4;
            UsedSize->lo   += 4;
            break;
    }
    switch((flag2 & 0x03) >> 0) // Replicated Data Length
    {
        case 0:
            ReplicatedDataLength = 0;
            break;
        case 1:
            ReplicatedDataLength = *((u8 *)buf);
            buf                 += 1;
            UsedSize->lo        += 1;
            // ReplicatedData
            if(ReplicatedDataLength == 8) {
				*MediaObjectSize = *(buf + 0) |
                                   (*(buf + 1) << 8) |
                                   (*(buf + 2) << 16) |
                                   (*(buf + 3) << 24);
                *asfPresentTime = *(buf + 4) |
                                  (*(buf + 5) << 8) |
                                  (*(buf + 6) << 16) |
                                  (*(buf + 7) << 24);
                *asfPresentTime -= PREROLL;  //Lsk 090303
                /* marked by Ted, IISTime should always restart with 0, because video_timebase has already tuned the offset
                if(Restart2 && *StreamNum == 2) {
                    IISTime = *asfPresentTime * 1000;
                }
                */
            }
			if(ReplicatedDataLength == 9) { //video
				*MediaObjectSize = *(buf + 0) |
                                   (*(buf + 1) << 8) |
                                   (*(buf + 2) << 16) |
                                   (*(buf + 3) << 24);
                *asfPresentTime = *(buf + 4) |
                                  (*(buf + 5) << 8) |
                                  (*(buf + 6) << 16) |
                                  (*(buf + 7) << 24);
                *asfPresentTime -= PREROLL;  //Lsk 090602 : for new video payload
            }
            buf            += ReplicatedDataLength;
            UsedSize->lo   += ReplicatedDataLength;
            break;
        case 2:
            ReplicatedDataLength = *buf + (*(buf+1)<<8);
            buf                 += 2;
            UsedSize->lo        += 2;
            // ReplicatedData
            buf                 += ReplicatedDataLength;
            UsedSize->lo        += ReplicatedDataLength;
            break;
        case 3:
            ReplicatedDataLength    = *buf + (*(buf+1)<<8) + (*(buf+2)<<16) + (*(buf+3)<<24);
            buf                    += 4;
            UsedSize->lo           += 4;
            // ReplicatedData
            buf                    += ReplicatedDataLength;
            UsedSize->lo           += ReplicatedDataLength;
            break;
    }

    if(Mul_Payload)
    {
        switch((PayloadFlag & 0xC0) >> 6) // Payload Data Length
        {
            case 0:
                *PayloadLength  = 0;
                break;
            case 1:
                *PayloadLength  = *((u8 *)buf);
                buf            += 1;
                UsedSize->lo   += 1;
                break;
            case 2:
                *PayloadLength  = *buf + (*(buf + 1) << 8);
                buf            += 2;
                UsedSize->lo   += 2;
                break;
            case 3:
                *PayloadLength  = *buf + (*(buf + 1) << 8) + (*(buf + 2) << 16) + (*(buf + 3) << 24);
                buf            += 4;
                UsedSize->lo   += 4;
                break;
        }
    }
    else  //Lsk 090304
    {
    	if(*StreamNum == 1)
			*PayloadLength = PacketLength - sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD) - sizeof(ASF_DTA_VIDEO_PAYLOAD_SINGLE) - PaddingLength;
		else if(*StreamNum == 2)
	        *PayloadLength = PacketLength - sizeof(ASF_DATA_PACKET_SINGLEPAYLOAD) - sizeof(ASF_DTA_AUDIO_PAYLOAD_SINGLE) - PaddingLength;
    }

    return  buf;
}

u8 asfCheckIFrame(u8   *buf)
{
    if(buf[0] == 0x0 &&
       buf[1] == 0x0 &&
       buf[2] == 0x0 &&
       buf[3] == 0x1)
    {
        if(buf[4] == 0x65 || buf[4] == 0x45)
            return TRUE;
    }
    return FALSE;
}

/*

Routine Description:

    Read video payload.

Arguments:

    buf                 - Point to video payload start address.
    VideoStreamBuf      - Point to mpeg-4 bitstream buffer.
    KeyFrame            - Key frame flag, 1: key frame, 0: otherwise.
    StartFrame          - Point to start frame flag, 1: start frame, 0: otherwise.
    Restart1            - Point to video restart flag, 1: video restart, 0: otherwise.
    VideoStreamLength   - Point to video stream length.
    Offset2Media        - Byte offset to media.
    PayloadLength       - Payload length.
    asfPresentTime      - Asf present time.
    asfVideoIndex       - Point to video index.
    mpeg4taskrun        - Point to mpeg4 task run flag, 1: run, 0: otherwise.

Return Value:

    0           - Failure.
    Otherwise   - Point to VideoStreamBuf.

*/
u8* asfReadVideoPayload(u8      *buf,
                        u8      *VideoStreamBuf,
                        u8      KeyFrame,
                        u8      *StartFrame,
                        u8      *Restart1,
                        u32     *VideoStreamLength,
                        u32     Offset2Media,
                        u32     PayloadLength,
                        u32     MediaObjectSize,   //Lsk 090622
                        s64     asfPresentTime,
                        u32     *asfVideoIndex,
                        u8      *mpeg4taskrun,
                        u32     *u32PacketOffset,
                        u64     *UsedSize,
                        FS_FILE *pFile,
                        ASF_FILE_PROPERTIES_OBJECT *asfFilePropertiesObject,
                        u64     *PacketCount)
{
	static int add_sps_pps=0;
    u8      *pBuf;
    //u16     video_value;

    //for the case still not having any I frame, skip payload
    if(Offset2Media != 0 && *StartFrame == 1){
        printf("No I frame, Skip useless payload\n");
        return VideoStreamBuf;
    }

    /* Lsk : read video payload first time, check I frame */
    if(Offset2Media == 0 && *StartFrame == 1 && (*Restart1 == 0 || (*Restart1 == 1 && KeyFrame == 1)))
    {
    	
    	if(*Restart1 == 1 && KeyFrame == 1)
			add_sps_pps = 1;	
		
		
        *StartFrame         = 0;
        *Restart1           = 0;
        VideoStreamBuf      = VideoBufMng[VideoBufMngWriteIdx].buffer;
        VideoBufMng[VideoBufMngWriteIdx].flag   = KeyFrame;
        VideoBufMng[VideoBufMngWriteIdx].time   = asfPresentTime * 1000;
        *VideoStreamLength  = PayloadLength;

        if(asfCheckIFrame(buf+24)){  // got I frame at start frame
             // do nth
        }
        else if (asfCheckIFrame(buf)){ // got I frame, but not at the first frame, add sps_pps
            add_sps_pps = 1;
        }
        else{ // still not getting any I frame, skip frame
            *StartFrame         = 1;
            return VideoStreamBuf;
        }

		#if 1 //add sps_pps	
		if(add_sps_pps == 1)
		{
			DEBUG_ASF("add_sps_pps, \n");
			CopyMemory(VideoStreamBuf, H264_config, sizeof(H264_config));
	        CopyMemory(VideoStreamBuf+sizeof(H264_config), buf, PayloadLength);
		}
		else
		{
			memcpy(VideoStreamBuf, buf, PayloadLength);				
		}
		#else
		memcpy(VideoStreamBuf, buf, PayloadLength);
		#endif

        // for debug
        //DEBUG_ASF("%d\n", VideoBufMng[VideoBufMngWriteIdx].time);
        /*
        DEBUG_ASF("0: VideoStreamLength = %d, Offset2Media = %d, PayloadLength = %d\n",
                *VideoStreamLength, Offset2Media, PayloadLength);
                */
    }
    /* Lsk : read video payload, and offset in media is 0 */
    else if(Offset2Media == 0 && *StartFrame == 0)
    {
#ifdef  ASF_AUDIO
#else
        // display video without IIS timing
        if(MainVideodisplaybuf_idx < IsuIndex) {
            if(MainVideodisplaybuf_idx == 0)
                iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
            else
                iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
            MainVideodisplaybuf_idx++;
            //FrameCount++;
            //DEBUG_ASF("Frame count : %x\n",FrameCount);
        }
#endif
        (*asfVideoIndex)++;

        pBuf    = VideoBufMng[VideoBufMngWriteIdx].buffer;
        //VideoBufMng[VideoBufMngWriteIdx].size   = *VideoStreamLength;
        if((pBuf + *VideoStreamLength) >= mpeg4VideBufEnd) {
            DEBUG_ASF("VideoBuf overflow!!!!\n");
        }
        if((pBuf < VideoBufMng[VideoBufMngReadIdx].buffer) &&
           //((pBuf + *VideoStreamLength) > VideoBufMng[VideoBufMngReadIdx].buffer)) {
           ((pBuf + *VideoStreamLength) >= VideoBufMng[VideoBufMngReadIdx].buffer)) {
            DEBUG_ASF("VideoBuf write point over read point!!!!\n");
        }
        VideoBufMngWriteIdx = (VideoBufMngWriteIdx + 1) % VIDEO_BUF_NUM;

        //OSSemPost(VideoTrgSemEvt); //Lsk 090622


		/* Lsk : show first frame immediately */
        if((MainVideodisplaybuf_idx == 0) && (IsuIndex > 1)) {
            if (sysPlaybackThumbnail == READFILE_NORMAL_PLAY) {
			    if(sysTVOutOnFlag && TVout_Generate_Pause_Frame)
    			{
	    			/* Lsk 090511 : use top field of Current frame to generate PAUSE frame,
	    			                and store in the next frame address so set flag */
		    		asfPausePlayback(MainVideodisplaybuf[0],
  			    	       			 MainVideodisplaybuf[1],
  				       	    		 0);
        			asfPausePlayback(MainVideodisplaybuf[1],
  	    				   			 MainVideodisplaybuf[0],
  		    			   			 0);
                }
		    	else
                	iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM]);
            }
            
            MainVideodisplaybuf_idx++;
            VideoNextPresentTime    = Videodisplaytime[MainVideodisplaybuf_idx % DISPLAY_BUF_NUM];
        }

        pBuf    = (u8*)(((u32)pBuf + (*VideoStreamLength) + 3) & ~3);

        // firmware have to fix 1K boundary bug, if this chip is older than PA9001D.
#if ((CHIP_OPTION == CHIP_PA9001A) || (CHIP_OPTION == CHIP_PA9001C) || (CHIP_OPTION == CHIP_PA9002A))
        if(((int)pBuf & 0x3ff) > 0x3b0) {    // fix first burst 8 over 1 K bytes boundary bug
            pBuf    = (u8*)(((int)pBuf + 0x400) & ~0x3ff);
        }
#endif

        if((pBuf + PayloadLength) < mpeg4VideBufEnd) {
        //if((pBuf + MPEG4_MIN_BUF_SIZE) < mpeg4VideBufEnd) {
            VideoBufMng[VideoBufMngWriteIdx].buffer = pBuf;
        } else {
            VideoBufMng[VideoBufMngWriteIdx].buffer = VideoBuf;
        }
        VideoStreamBuf      = VideoBufMng[VideoBufMngWriteIdx].buffer;



        VideoBufMng[VideoBufMngWriteIdx].flag   = KeyFrame;
        VideoBufMng[VideoBufMngWriteIdx].time   = asfPresentTime * 1000;
        *VideoStreamLength   = PayloadLength;

        //DEBUG_ASF("%d\n", VideoBufMng[VideoBufMngWriteIdx].time);

        //while((video_value = OSSemAccept(VideoCmpSemEvt)) == 0) {
        while((OSSemAccept(VideoCmpSemEvt)) == 0) {
            OSTimeDly(1);
        }

        /* Lsk 091116 : read video payload fast than MPEG4 decode and display,
           Lsk 090417 : avoid deadlock when press stop playback                 */
        while((*asfVideoIndex >= ((VideoPictureIndex+NVOPCnt) + VIDEO_BUF_NUM - 2)) && sysPlaybackVideoStop == 0 && ResetPlayback == 0) {  //Lsk 090417 : avoid deadlock when press stop playback
            OSTimeDly(1);
        }

        /* Lsk 091116 : write buffer addresss is less than read buffer address,
                        and than writing current payload will cause write buffer
                        addresss is large than read buffer address.
           Lsk 090417 : avoid deadlock when press stop playback
        */
        //while((VideoStreamBuf < VideoBufMng[VideoBufMngReadIdx].buffer) &&
        while((VideoStreamBuf <= VideoBufMng[VideoBufMngReadIdx].buffer) &&
              (*asfVideoIndex > (VideoPictureIndex+NVOPCnt)) &&
           //((VideoStreamBuf + PayloadLength) > VideoBufMng[VideoBufMngReadIdx].buffer)) {
           ((VideoStreamBuf + PayloadLength + MPEG4_MIN_BUF_SIZE) > VideoBufMng[VideoBufMngReadIdx].buffer)
           && sysPlaybackVideoStop == 0 && ResetPlayback == 0) {
            OSTimeDly(1);
        }

        memcpy(VideoStreamBuf, buf, PayloadLength);
    }
    /* Lsk : offset in media is not 0 */
    else if(*Restart1 == 0)// (*Restart1 == 1 && KeyFrame == 1 && *StartFrame == 0)
    {
        if((VideoStreamBuf + (*VideoStreamLength) + PayloadLength) >= mpeg4VideBufEnd) {
            //while((VideoBuf + (*VideoStreamLength) + PayloadLength) > VideoBufMng[VideoBufMngReadIdx].buffer) {
            while(((VideoBuf + (*VideoStreamLength) + PayloadLength + MPEG4_MIN_BUF_SIZE) > VideoBufMng[VideoBufMngReadIdx].buffer)
			&& sysPlaybackVideoStop == 0 && ResetPlayback == 0) {//Lsk 090417 : avoid deadlock when press stop playback
                OSTimeDly(1);
            }
            memcpy(VideoBuf, VideoStreamBuf, *VideoStreamLength);
            VideoBufMng[VideoBufMngWriteIdx].buffer = VideoBuf;
            VideoStreamBuf                                  = VideoBuf;
            //DEBUG_ASF("Some thing maybe wrong!! VideoBufMngWriteIdx = %d\n", VideoBufMngWriteIdx);
        }
        /* Lsk 091116 : write buffer addresss is less than read buffer address,
                        and than writing current payload will cause write buffer
                        addresss is large than read buffer address.
           Lsk 090417 : avoid deadlock when press stop playback
        */
        while((VideoStreamBuf < VideoBufMng[VideoBufMngReadIdx].buffer) &&
           //((VideoStreamBuf + (*VideoStreamLength) + PayloadLength) > VideoBufMng[VideoBufMngReadIdx].buffer)) {
           ((VideoStreamBuf + (*VideoStreamLength) + PayloadLength + MPEG4_MIN_BUF_SIZE) > VideoBufMng[VideoBufMngReadIdx].buffer)
           && sysPlaybackVideoStop == 0 && ResetPlayback == 0) {//Lsk 090417 : avoid deadlock when press stop playback
            OSTimeDly(1);
        }
        //memcpy(&(VideoStreamBuf[Offset2Media]), buf, PayloadLength);
        //*VideoStreamLength = Offset2Media + PayloadLength;
		if(add_sps_pps == 1)		
			CopyMemory(VideoStreamBuf + sizeof(H264_config) + *VideoStreamLength, buf, PayloadLength);
		else	
			CopyMemory(VideoStreamBuf + *VideoStreamLength, buf, PayloadLength);	
	        
        *VideoStreamLength    = (*VideoStreamLength) + PayloadLength;
    }

	if(*VideoStreamLength == MediaObjectSize)   //Lsk 090622 : Read a full frame, tell MPEG4 decode
	{
		if(add_sps_pps == 1)
		{		
			#if 0
			FS_FILE *tFile;
			RTC_DATE_TIME localTime;
			int size;
		
			RTC_Get_Time(&localTime);
			tFile = dcfCreateNextBackupFile(DCF_FILE_TYPE_ASF, 0, DCF_REC_TYPE_MANUAL, &localTime);
			dcfBackupWrite(tFile, (unsigned char*)VideoBufMng[VideoBufMngWriteIdx].buffer, sizeof(H264_config) + *VideoStreamLength, &size);
			dcfBackupClose(tFile);
			#endif
			
			VideoBufMng[VideoBufMngWriteIdx].size = sizeof(H264_config) + *VideoStreamLength;	
			*VideoStreamLength += sizeof(H264_config);		
			add_sps_pps = 0;  			
		}
		else	
	    	VideoBufMng[VideoBufMngWriteIdx].size = *VideoStreamLength;	
        
		OSSemPost(VideoTrgSemEvt);
        if(*asfVideoIndex == 0) {
            if(MPEG4_Task_Go) {
                OSSemAccept(VideoTrgSemEvt);
            }
            mpeg4ResumeTask();
			*mpeg4taskrun    = 1;
			#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
            /* Lsk : start display Video frame in FF/RF*/
			if(sysPlaybackForward != 0)
				StartPlayBack = 1;
			#endif
        }
	}
    return  VideoStreamBuf;
}

#if (AUDIO_CODEC == AUDIO_CODEC_PCM)

/*

Routine Description:

    Read audio payload.

Arguments:

    buf                 - Point to audio payload start address.
    AudioStreamBuf      - Point to audio bitstream buffer.
    StartAudio          - Point to start audio flag, 1: start frame, 0: otherwise.
    Restart2            - Point to audio restart flag, 1: video restart, 0: otherwise.
    AudioStreamLength   - Point to audio stream length.
    Offset2Media        - Byte offset to media.
    PayloadLength       - Payload length.
    asfAudioIndex       - Point to audio index.
    AudioPlayback       - Point to audio playback flag, 1: audio playbacked, 0: otherwise.

Return Value:

    0           - Failure.
    Otherwise   - Point to VideoStreamBuf.

*/
u8* asfReadAudioPayload(u8      *buf,
                        u8      *AudioStreamBuf,
                        u8      *StartAudio,
                        u8      *Restart2,
                        u32     *AudioStreamLength,
                        u32     Offset2Media,
                        u32     PayloadLength,
                        s64     asfPresentTime,
                        u32     *asfAudioIndex,
                        u32     *AudioPlayback)
{
    //u16     audio_value=0;

    // 第一次讀取Audio payload
    if(Offset2Media == 0 && (*StartAudio) == 1)
    {
        *StartAudio         = 0;
        *Restart2           = 0;
        AudioStreamBuf      = iisSounBufMng[iisSounBufMngWriteIdx].buffer;
        *AudioStreamLength  = PayloadLength;
        memcpy(AudioStreamBuf, buf, PayloadLength);

        // for debug
        /*
        DEBUG_ASF("0: AudioStreamLength = %d, Offset2Media = %d, PayloadLength = %d\n",
                *AudioStreamLength, Offset2Media, PayloadLength);
                */
    }
    // 讀取Audio payload,除了一個檔案的第一個payload次之外(無offset時),所有Object的第一個payload都會走這邊.
    else if(Offset2Media == 0 && *StartAudio == 0)
    {
        (*asfAudioIndex)++;
		
        //while(((audio_value = OSSemAccept(iisCmpSemEvt)) == 0)&& sysPlaybackVideoStop==0 ) {
        while(((OSSemAccept(iisCmpSemEvt)) == 0)&& sysPlaybackVideoStop==0 && ResetPlayback == 0) {
			if(*AudioPlayback == 0) 
			{
		        *AudioPlayback       = 1;
				#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
				IDUInterruptTime = 0;
				StartPlayBack = 1;
				#endif				
		        iisResumeTask();
		    }
            OSTimeDly(1);
        }
        iisSounBufMng[iisSounBufMngWriteIdx].time   = asfPresentTime * 1000;
        iisSounBufMng[iisSounBufMngWriteIdx].size   = *AudioStreamLength;
        iisSounBufMngWriteIdx = (iisSounBufMngWriteIdx + 1) % IIS_BUF_NUM;
        OSSemPost(iisTrgSemEvt);
       #if ((SW_APPLICATION_OPTION == MR8200_RFCAM_RX1)  || (SW_APPLICATION_OPTION == MR8600_RFCAM_RX1RX2) || (SW_APPLICATION_OPTION == MR8200_RFCAM_RX1RX2) ||\
            (SW_APPLICATION_OPTION == MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) || (SW_APPLICATION_OPTION == MR9200_HDMI_RX1RX2) ||\
            (SW_APPLICATION_OPTION == MR9200_MIXCAM_RX1RX2) )
        if((*AudioPlayback == 0) &&
            (*asfAudioIndex >= 12) ){
           //(*asfAudioIndex >= 16) &&
          // (IsuIndex >= (DISPLAY_BUF_NUM - 3))) {  // only trigger IIS one time
       #else
        if((*AudioPlayback == 0) &&
            (*asfAudioIndex >= 12) &&
           //(*asfAudioIndex >= 16) &&
           (IsuIndex >= (DISPLAY_BUF_NUM - 3))) {  // only trigger IIS one time
       #endif
            *AudioPlayback       = 1;
			#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
			IDUInterruptTime = 0;
			StartPlayBack = 1;
			#endif
            iisResumeTask();
        }
        AudioStreamBuf      = iisSounBufMng[iisSounBufMngWriteIdx].buffer;
        *AudioStreamLength  = PayloadLength;
        memcpy(AudioStreamBuf, buf, PayloadLength);
    }
    // 除了所有Object的第一個payload之外,其餘payload會走到這邊,但通常一個audio object只有一個payload,所以不會跑到這裡.
    else if(*Restart2 == 0)
    {
        //memcpy(&(AudioStreamBuf[Offset2Media]), buf, PayloadLength);
        //*AudioStreamLength = Offset2Media + PayloadLength;
        memcpy(AudioStreamBuf + *AudioStreamLength, buf, PayloadLength);
        *AudioStreamLength  = (*AudioStreamLength) + PayloadLength;
    }

    return  AudioStreamBuf;
}

#elif (AUDIO_CODEC == AUDIO_CODEC_IMA_ADPCM)

/*

Routine Description:

    Read audio payload.

Arguments:

    buf             - Point to audio payload start address.
    pMng1           - Buffer manager 1.
    pMng2           - Buffer manager 2.
    pPcmOffset      - Offset to PCM Start Point.
    pImaAdpcmOpt    - IMA ADPCM option.

Return Value:

    0 - Failure.
    1 - Success and only using pMng1 PCM data.
    2 - Success and using both pMng1 and pMng2 PCM data.

*/
u8 asfReadAudioPayload_IMA_ADPCM_1Payload(u8                *buf,
                                                             IIS_BUF_MNG       *pMng1,
                                                             IIS_BUF_MNG       *pMng2,
                                                             s32               *pPcmOffset,
                                                             IMA_ADPCM_Option  *pImaAdpcmOpt)
{
    IMA_ADPCM_BLOCK     *pbuff;

    pbuff                               = (IMA_ADPCM_BLOCK*)buf;
    pImaAdpcmOpt->PcmAddress1           = pMng1->buffer + *pPcmOffset;
    pImaAdpcmOpt->PcmAddress2           = pMng2->buffer;
    pImaAdpcmOpt->PcmSize1              = IIS_CHUNK_SIZE - *pPcmOffset;
    pImaAdpcmOpt->PcmSize2              = PcmBytesForAdpcm - pImaAdpcmOpt->PcmSize1;
    pImaAdpcmOpt->PcmStrWord            = *(u32*)((u32)(pImaAdpcmOpt->PcmAddress1) & ~3);
    pImaAdpcmOpt->AdpcmStrWord          = 0;
    pImaAdpcmOpt->AdpcmSample           = pbuff->sample0;
    pImaAdpcmOpt->AdpcmIndex            = pbuff->index;
    pImaAdpcmOpt->PcmBitPerSample       = 8;
    pImaAdpcmOpt->PcmSigned             = 0;
    if(IMA_ADPCM_Decode_Block_HW(pImaAdpcmOpt) == 0)
        return 0;
    *pPcmOffset                        += PcmBytesForAdpcm;

    if(pImaAdpcmOpt->PcmSize2)
        return 2;
    else
        return 1;
}

/*

Routine Description:

    Read audio payload.

Arguments:

    buf                 - Point to audio payload start address.
    AudioStreamBuf      - Point to audio bitstream buffer.
    StartAudio          - Point to start audio flag, 1: start frame, 0: otherwise.
    Restart2            - Point to audio restart flag, 1: video restart, 0: otherwise.
    AudioStreamLength   - Point to audio stream length.
    Offset2Media        - Byte offset to media.
    PayloadLength       - Payload length.
    asfAudioIndex       - Point to audio index.
    AudioPlayback       - Point to audio playback flag, 1: audio playbacked, 0: otherwise.

Return Value:

    0           - Failure.
    Otherwise   - Point to VideoStreamBuf.

*/
u8* asfReadAudioPayload_IMA_ADPCM(u8      *buf,
                                                 u8      *AudioStreamBuf,
                                                 u8      *StartAudio,
                                                 u8      *Restart2,
                                                 u32     *AudioStreamLength,
                                                 u32     Offset2Media,
                                                 u32     PayloadLength,
                                                 u32     *asfAudioIndex,
                                                 u32     *AudioPlayback,
                                                 ASF_AUDI_STREAM_PROPERTIES_OBJECT   *asfAudiStreamPropertiesObject)
{
    u16                         audio_value;
    static  u32                 iisNextIdx, PcmOffset;
    static  IMA_ADPCM_Option    *pImaAdpcmOpt;

    // 第一次讀取Audio payload
    if(Offset2Media == 0 && (*StartAudio) == 1)
    {
        *StartAudio         = 0;
        *Restart2           = 0;
        //AudioStreamBuf      = iisSounBufMng[iisSounBufMngWriteIdx].buffer;
        AudioStreamBuf      = ImaAdpcmBuf;
        *AudioStreamLength  = PayloadLength;
        memcpy(AudioStreamBuf, buf, PayloadLength);

        // for debug
        /*
        DEBUG_ASF("0: AudioStreamLength = %d, Offset2Media = %d, PayloadLength = %d\n",
                *AudioStreamLength, Offset2Media, PayloadLength);
                */
        iisNextIdx                          = (iisSounBufMngWriteIdx + 1) % IIS_BUF_NUM;
        PcmOffset                           = 0;
        PcmBytesForAdpcm                    = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.wSamplesPerBlock;
        pImaAdpcmOpt                        = &ImaAdpcmOpt;
        pImaAdpcmOpt->PcmAddress1           = iisSounBufMng[iisSounBufMngWriteIdx].buffer;
        pImaAdpcmOpt->PcmAddress2           = 0;
        pImaAdpcmOpt->AdpcmAddress          = ImaAdpcmBuf;
        pImaAdpcmOpt->PcmSize1              = IIS_CHUNK_SIZE;
        pImaAdpcmOpt->PcmSize2              = 0;
        pImaAdpcmOpt->PcmTotalSize          = IIS_CHUNK_SIZE;
        pImaAdpcmOpt->AdpcmSize             = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.block_alignment;
        pImaAdpcmOpt->AdpcmSamplePerBlock   = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.wSamplesPerBlock;
        pImaAdpcmOpt->PcmStrWord            = 0;
        pImaAdpcmOpt->AdpcmStrWord          = 0;
        pImaAdpcmOpt->AdpcmSample           = 0;
        pImaAdpcmOpt->AdpcmIndex            = 0;
        pImaAdpcmOpt->PcmBitPerSample       = 8;
        pImaAdpcmOpt->PcmSigned             = 0;
        PcmBytesForAdpcm                    = asfAudiStreamPropertiesObject->asf_hdr_audi_type_specific_dta.wSamplesPerBlock;
    }
    // 讀取Audio payload,除了一個檔案的第一個payload次之外(無offset時),所有Object的第一個payload都會走這邊.
    else if(Offset2Media == 0 && *StartAudio == 0)
    {
        if(PcmOffset >= IIS_CHUNK_SIZE)
        {
            (*asfAudioIndex)++;
            while(((audio_value = OSSemAccept(iisCmpSemEvt)) == 0)&& sysPlaybackVideoStop==0 ) {
                OSTimeDly(1);
            }
            while(iisCmpSemEvt->OSEventCnt <= 1) {
                OSTimeDly(1);
            }
            iisSounBufMng[iisSounBufMngWriteIdx].size   = IIS_CHUNK_SIZE;
            iisSounBufMngWriteIdx                       = iisNextIdx;
            iisNextIdx                                  = (iisSounBufMngWriteIdx + 1) % IIS_BUF_NUM;
            PcmOffset                                  %= IIS_CHUNK_SIZE;
            OSSemPost(iisTrgSemEvt);
        }
        if((*AudioPlayback == 0) &&
            (*asfAudioIndex >= 12) &&
           //(*asfAudioIndex >= 16) &&
           (IsuIndex >= (DISPLAY_BUF_NUM - 2))) {    // only trigger IIS one time
            *AudioPlayback       = 1;
			#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
			IDUInterruptTime = 0;
			StartPlayBack = 1;
			#endif
            iisResumeTask();
        }
        //AudioStreamBuf      = iisSounBufMng[iisSounBufMngWriteIdx].buffer;
        AudioStreamBuf      = ImaAdpcmBuf;
        *AudioStreamLength  = PayloadLength;
        memcpy(AudioStreamBuf, buf, PayloadLength);
    }
    // 除了所有Object的第一個payload之外,其餘payload會走到這邊,但通常一個audio object只有一個payload,所以不會跑到這裡.
    else if(*Restart2 == 0)
    {
        //memcpy(&(AudioStreamBuf[Offset2Media]), buf, PayloadLength);
        //*AudioStreamLength = Offset2Media + PayloadLength;
        memcpy(AudioStreamBuf + *AudioStreamLength, buf, PayloadLength);
        *AudioStreamLength  = (*AudioStreamLength) + PayloadLength;
    }
    while(iisSounBufMngReadIdx == iisNextIdx) {
        OSTimeDly(1);
    }
    asfReadAudioPayload_IMA_ADPCM_1Payload(AudioStreamBuf,
                                           &iisSounBufMng[iisSounBufMngWriteIdx],
                                           &iisSounBufMng[iisNextIdx],
                                           &PcmOffset,
                                           pImaAdpcmOpt);
    return  AudioStreamBuf;
}

#endif

/*

Routine Description:

    Write dummy padding data to file.

Arguments:

    pFile   - File handle.
    size    - Padding data size.

Return Value:

    0 - Failure.
    1 - Success.

*/
/* Peter 070104 */
#if( ((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2)) || (!MULTI_CHANNEL_VIDEO_REC))
s32 asfWriteFilePadding(FS_FILE* pFile, u32 DummySize)
{
    u32 remainder, BufferSize, size;
    u8  *pBuf;

    remainder   = DummySize;

    BufferSize  = MPEG4_INDEX_BUF_SIZE;
    pBuf        = (u8*)asfIndexTable;
    memset_hw(pBuf, 0, BufferSize);

    while(remainder > BufferSize) {
        if (dcfWrite(pFile, pBuf, BufferSize, &size) == 0) {
            return 0;
        }
        remainder  -= BufferSize;
    }
    if(remainder) {
        if (dcfWrite(pFile, pBuf, remainder, &size) == 0) {
            return 0;
        }
    }

    return 1;
}

/*

Routine Description:

    Write virtual video payload.

Arguments:

    pMng - Buffer manager.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteVirtualVidePayload(VIDEO_BUF_MNG* pMng)
{
    u32 chunkTime, chunkSize;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    chunkTime = pMng->time;
    chunkSize = pMng->size;

    /* advance the index */
    asfVidePresentTime += chunkTime;

    OS_ENTER_CRITICAL();
    CurrentVideoSize   -= chunkSize;
    OS_EXIT_CRITICAL();

    return 1;
}

/*

Routine Description:

    Write virtual audio payload.

Arguments:

    pMng - Buffer manager.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 asfWriteVirtualAudiPayload(IIS_BUF_MNG* pMng)
{
    u32 chunkTime, chunkSize;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    chunkTime = pMng->time;
    chunkSize = pMng->size;

    /* advance the index */
    asfAudiChunkCount++;
    asfAudiPresentTime += chunkTime;

    //if(asfCaptureMode == ASF_CAPTURE_OVERWRITE) {
        OS_ENTER_CRITICAL();
        CurrentAudioSize   -= chunkSize;
        OS_EXIT_CRITICAL();
    //}

    return 1;
}
#endif
/*

Routine Description:

    Set video capture section time.

Arguments:

    second - section time per video captured file.

Return Value:

    None.

*/
void asfSetVideoSectionTime(u32 second)
{
    if(second > ASF_IDX_SIMPLE_INDEX_ENTRY_MAX)
    {
        DEBUG_ASF("Warning: Video section time(%d sec) > ASF_IDX_SIMPLE_INDEX_ENTRY_MAX(%d sec)!!!\n", second, ASF_IDX_SIMPLE_INDEX_ENTRY_MAX);
    }
    asfSectionTime  = second;
}

/*

Routine Description:

    Set the Previous record time of the video capture .

Arguments:

    second - the Previous record time of the video capture .

Return Value:

    None.

*/
void asfSetPreviousRecordTime(u32 second)
{
    if(second > 10)
    {
        DEBUG_ASF("Warning: Previous record(%d sec) > 10 Seconds!!!\n", second);
    }
    PreRecordTime  = second;
}


/*

Routine Description:

    Stop video capture.

Arguments:

    Void.

Return Value:

    1: Success.
    2: Not in video capture mode
    0: Otherwise

*/
s32 asfCaptureVideoStop(void)
{
    if(sysCaptureVideoStart && !sysCaptureVideoStop)
    {
        if(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)
        {
            if(!EventTrigger && !DoCaptureVideo)
            {
                sysCaptureVideoStart    = 0;
                sysCaptureVideoStop     = 1;
                DEBUG_ASF("asfCaptureVideoStop() success!!!\n");
                DEBUG_ASF("EventTrigger = %d, DoCaptureVideo = %d\n", EventTrigger, DoCaptureVideo);
                return  1;
            }
			else
			{
                if(DoCaptureVideo)
                {
                    if(DoCaptureVideo == 1)
                        DoCaptureVideo  = 2;
                }
				else if(EventTrigger == CAPTURE_STATE_TRIGGER)
				{
                    Last_VideoBufMngReadIdx = VideoBufMngWriteIdx;
                    EventTrigger                = CAPTURE_STATE_TIMEUP;
                }
                DEBUG_ASF("EventTrigger = %d, DoCaptureVideo = %d\n", EventTrigger, DoCaptureVideo);
            }
        }
		else if(asfVopCount > 10 && asfAudiChunkCount > 12)
		{

            sysCaptureVideoStart    = 0;
            sysCaptureVideoStop     = 1;
            DEBUG_ASF("asfCaptureVideoStop() success!!!\n");
            return  1;
        }
    }
    else if(!sysCaptureVideoStart && sysCaptureVideoStop)
    {
        DEBUG_ASF("Not in video capture mode!!!\n");
        return  2;
    }
    DEBUG_ASF("asfCaptureVideoStop() fail!!!\n");
    DEBUG_ASF("sysCaptureVideoStart = %d\n", sysCaptureVideoStart);
    DEBUG_ASF("sysCaptureVideoStop  = %d\n", sysCaptureVideoStop);
    DEBUG_ASF("asfCaptureMode       = %d\n", asfCaptureMode);
    DEBUG_ASF("EventTrigger         = %d\n", EventTrigger);
    DEBUG_ASF("asfVopCount          = %d\n", asfVopCount);
    DEBUG_ASF("asfAudiChunkCount    = %d\n", asfAudiChunkCount);
    return 0;
}



/*

Routine Description:

    Stop video playback.

Arguments:

    Void.

Return Value:

    1: Success.
    2: Not in video capture mode
    0: Otherwise

*/
s32 asfPlaybackVideoStop(void)
{
    u32 wait_cnt = 0, i;
    if(sysPlaybackVideoStart && !sysPlaybackVideoStop)
    {
        //if((mpeg4taskrun==1 && AudioPlayback==1 && video_playback_speed==5) || (mpeg4taskrun==1  && video_playback_speed!=5)) 
        if((mpeg4taskrun==1 && video_playback_speed==5) || (mpeg4taskrun==1  && video_playback_speed!=5)) //no audio file bug 2014/09/12
        {
            sysPlaybackVideoStart    = 0;
            sysPlaybackVideoStop     = 1;
            sysPlaybackVideoStopDone = 0;

#if( (SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2) )
            while(sysPlaybackVideoStopDone==0 && wait_cnt < 20){ //wait up to 1 second
                wait_cnt++;
                OSTimeDly(1);
            }
            if(sysPlaybackVideoStopDone==1){
                for(i=0;i<DISPLAY_BUF_NUM;i++){
                    Idu_ClearBuf_ULTRA_FHD(i);
                }
            }
#endif
            DEBUG_ASF("asfPlaybackVideoStop() success!!! wait:%d ms\n", wait_cnt*50);
            return  1;
        }
    }
    else if(!sysPlaybackVideoStart && sysPlaybackVideoStop)
    {
        DEBUG_ASF("Not in video capture mode!!!\n");
        return  2;
    }else if(!sysVoicePlayStop) //wave stop
	{
		sysVoicePlayStop=1;
	 	DEBUG_ASF("wave stop success!!!\n");
        return  3;
	}
    return 0;
}

u32 asfRemainRecSecForDiskFreeSpace(void)
{
    u32 total_free_secs;
    u32 remain_rec_sec;
    u32 sectors_per_sec_in_mp4  = 247;

    total_free_secs     = global_diskInfo.sectors_per_cluster * global_diskInfo.avail_clusters;

    //DEBUG_UI("Total clusters now:      %lu\n", global_diskInfo.total_clusters);
    //DEBUG_UI("Available clusters now:  %lu\n", global_diskInfo.avail_clusters);
    //DEBUG_UI("Sectors per cluster now: %u\n",  global_diskInfo.sectors_per_cluster);
    //DEBUG_UI("Bytes per sector now:    %u\n",  global_diskInfo.bytes_per_sector);
    //Lsk 090525 : adjust remain_rec_sec by FrameRate and Image Qulity setting
    switch(VideoRecFrameRate)
    {
        case MPEG4_VIDEO_FRAMERATE_60:
            switch(mpeg4VideoRecQulity)
            {
                case MPEG4_VIDEO_QUALITY_HIGH:
                    sectors_per_sec_in_mp4  = 730; // 2.99 Mb/sec
                    break;

                case MPEG4_VIDEO_QUALITY_MEDIUM:
                    sectors_per_sec_in_mp4  = 539; // 2.21 Mb/sec
                    break;

                case MPEG4_VIDEO_QUALITY_LOW:
                    sectors_per_sec_in_mp4  = 325; // 1.33 Mb/sec
                    break;
            }
            break;

   	    case MPEG4_VIDEO_FRAMERATE_30:
        	switch(mpeg4VideoRecQulity)
            {
                case MPEG4_VIDEO_QUALITY_HIGH:
                    sectors_per_sec_in_mp4  = 730; // 2.99 Mb/sec
                    break;

                case MPEG4_VIDEO_QUALITY_MEDIUM:
                    sectors_per_sec_in_mp4  = 539; // 2.21 Mb/sec
                    break;

                case MPEG4_VIDEO_QUALITY_LOW:
                    sectors_per_sec_in_mp4  = 325; // 1.33 Mb/sec
                    break;
            }
            break;

        case MPEG4_VIDEO_FRAMERATE_15:
            switch(mpeg4VideoRecQulity)
            {
                case MPEG4_VIDEO_QUALITY_HIGH:
                    sectors_per_sec_in_mp4  = 247;   // 1.01 Mb/sec
                    break;

                case MPEG4_VIDEO_QUALITY_MEDIUM:
                    sectors_per_sec_in_mp4  = 191;
                    break;

                case MPEG4_VIDEO_QUALITY_LOW:
                    sectors_per_sec_in_mp4  = 127;
                    break;
            }
            break;

        case MPEG4_VIDEO_FRAMERATE_5:
            switch(mpeg4VideoRecQulity)
            {
                case MPEG4_VIDEO_QUALITY_HIGH:
                    sectors_per_sec_in_mp4  = 85;
                    break;

                case MPEG4_VIDEO_QUALITY_MEDIUM:
                    sectors_per_sec_in_mp4  = 63;
                    break;

                case MPEG4_VIDEO_QUALITY_LOW:
                    sectors_per_sec_in_mp4  = 43;
                    break;
            }
            break;

	    case MPEG4_VIDEO_FRAMERATE_10:
    		switch(mpeg4VideoRecQulity)
    		{
        		case MPEG4_VIDEO_QUALITY_HIGH:
        			sectors_per_sec_in_mp4  = 170;
        			break;

        		case MPEG4_VIDEO_QUALITY_MEDIUM:
        			sectors_per_sec_in_mp4  = 126;
        			break;

        		case MPEG4_VIDEO_QUALITY_LOW:
        			sectors_per_sec_in_mp4  = 86;
        			break;
		    }
		    break;
    }
    if((mpeg4Width == 320) || (mpeg4Width == 352) ) //Lsk 090703 : QVGA frames
    {
        remain_rec_sec = total_free_secs / sectors_per_sec_in_mp4;
        remain_rec_sec *= 3;
    }
    else
    {
        remain_rec_sec = total_free_secs / sectors_per_sec_in_mp4;
    }


    return remain_rec_sec;
}

#if((SW_APPLICATION_OPTION==MR9300_RFDVR_RX1RX2) || (SW_APPLICATION_OPTION == MR9300_NETBOX_RX1RX2))
DCF_LIST_FILEENT* dcfBackupCurFile;
FS_FILE*          pMergeFile;

u8* asfExtractVideoPayload(u8      *buf, 
                        u8      *VideoStreamBuf,
                        u8      KeyFrame,
                        u8      *StartFrame,
                        u8      *Restart1,
                        u32     *VideoStreamLength,
                        u32     Offset2Media,
                        u32     PayloadLength,
                        u32     MediaObjectSize,   //Lsk 090622
                        s64		*PreAudioPresentTime,                        
                        s64     *PreVideoPresentTime,                        
                        s64     *asfBaseTime,                        
                        s64		asfPresentTime,
                        u32     *asfVideoIndex,
                        u8      *mpeg4taskrun,
                        u64     *UsedSize,
                        FS_FILE *pFile,
                        ASF_FILE_PROPERTIES_OBJECT *asfFilePropertiesObject,
                        u64     *PacketCount,
                        u8      *GetFirstPayload)
{
	static int add_sps_pps=0;
	int direct_copy = 0; //Lsk: complete frame
    u8      *pBuf;
    

    /* Lsk : read video payload first time, check I frame */
    if(Offset2Media == 0 && *StartFrame == 1 && (*Restart1 == 0 || (*Restart1 == 1 && KeyFrame == 1)))
    {
    	if(*Restart1 == 1 && KeyFrame == 1)
    	{
			add_sps_pps = 1;                 //Lsk: add sps_pps before I frame.
			DEBUG_ASF("Copy: add_sps_pps\n");
    	}
		
        *StartFrame         = 0;
        *Restart1           = 0;
        VideoStreamBuf      = VideoBufMng[0].buffer;
        VideoBufMng[0].flag   = KeyFrame;
        //VideoBufMng[0].time   = VideoPresentTime * 1000;
        *VideoStreamLength  = PayloadLength;


		if(add_sps_pps ==1)
		{
			CopyMemory(VideoStreamBuf, H264_config, sizeof(H264_config));
			CopyMemory(VideoStreamBuf+sizeof(H264_config), buf, PayloadLength);
		}
		else
		{
			if(*VideoStreamLength == MediaObjectSize)
				direct_copy = 1;
			else
				CopyMemory(VideoStreamBuf , buf, PayloadLength);
		}         
    }
    /* Lsk : read video payload, and offset in media is 0 */
    else if(Offset2Media == 0 && *StartFrame == 0)
    {
        (*asfVideoIndex)++;        
        pBuf    = VideoBufMng[0].buffer;                                                          
        VideoStreamBuf      = VideoBufMng[0].buffer;
        
        
        
        VideoBufMng[0].flag   = KeyFrame;
        //VideoBufMng[0].time   = VideoPresentTime * 1000;
        *VideoStreamLength   = PayloadLength;

		if(*VideoStreamLength == MediaObjectSize)
			direct_copy = 1;
		else
			CopyMemory(VideoStreamBuf , buf, PayloadLength);			  
    }
    /* Lsk : offset in media is not 0 */
    else if(*Restart1 == 0 || (*Restart1 == 1 && KeyFrame == 1))
    {   
    	if(add_sps_pps == 1)
	        CopyMemory(VideoStreamBuf + sizeof(H264_config) + *VideoStreamLength, buf, PayloadLength);			
		else	
	        CopyMemory(VideoStreamBuf + *VideoStreamLength, buf, PayloadLength);			
        *VideoStreamLength    = (*VideoStreamLength) + PayloadLength;
    }

	#if 1
	if(*VideoStreamLength == MediaObjectSize)   //Lsk 090622 : Read a full frame, tell MPEG4 decode
	{		
		if(*GetFirstPayload == 0)
		{
			*asfBaseTime         = asfPresentTime;			  
			*PreAudioPresentTime = asfPresentTime;			
			*PreVideoPresentTime = asfPresentTime;						
			*GetFirstPayload = 1;

			DEBUG_ASF("Copy: asfBaseTime: %d\n", (u32)(*asfBaseTime));									
		}	
		VideoBufMng[0].time = ((u32)asfPresentTime - (u32)(*PreVideoPresentTime));	//Lsk: get chunkTime					
		//printf("v ChunkTime: %d - %d = %d\n", (u32)asfPresentTime, (u32)(*PreVideoPresentTime), (u32)(VideoBufMng[0].time));									
		*PreVideoPresentTime = asfPresentTime;							

		if(direct_copy)
		{
			u8* buffer;

			buffer = VideoBufMng[0].buffer;
			VideoBufMng[0].buffer = buf;
			VideoBufMng[0].size = *VideoStreamLength;
			asfWriteVidePayload(pMergeFile, &VideoBufMng[0]);
			VideoBufMng[0].buffer = buffer;					
		}
		else
		{			
			if(add_sps_pps == 1)
			{
				add_sps_pps = 0;
				VideoBufMng[0].size = sizeof(H264_config) + *VideoStreamLength;	
			}
			else	
		    	VideoBufMng[0].size = *VideoStreamLength;	
			asfWriteVidePayload(pMergeFile, &VideoBufMng[0]);			
		}		
	}
    #endif
    return  VideoStreamBuf;
}

u8* asfExtractAudioPayload(u8      *buf, 
                        u8      *AudioStreamBuf,
                        u8      *StartAudio,
                        u8      *Restart2,
                        u32     *AudioStreamLength,
                        u32     Offset2Media,
                        u32     PayloadLength,
                        u32     MediaObjectSize,   //Lsk 090622
                        u32     *asfAudioIndex,
                        u32     *AudioPlayback,
						s64		*PreAudioPresentTime,                          
                        s64		*asfBaseTime,
                        s64     asfPresentTime,
                        u8      GetFirstPayload)
{
	int direct_copy = 0; //Lsk: complete frame
	//u16     audio_value;;

    // 第一次讀取Audio payload
    if(Offset2Media == 0 && (*StartAudio) == 1)
    {
        *StartAudio         = 0;
        *Restart2           = 0;
        AudioStreamBuf      = iisSounBufMng[0].buffer;
        *AudioStreamLength  = PayloadLength;

		if(*AudioStreamLength == MediaObjectSize)
			direct_copy = 1;
		else				
	        CopyMemory(AudioStreamBuf, buf, PayloadLength);
        
    }
    // 讀取Audio payload,除了一個檔案的第一個payload次之外(無offset時),所有Object的第一個payload都會走這邊.
    else if(Offset2Media == 0 && *StartAudio == 0)
    {
        (*asfAudioIndex)++;

		AudioStreamBuf      = iisSounBufMng[0].buffer;
        *AudioStreamLength  = PayloadLength;

		if(*AudioStreamLength == MediaObjectSize)
			direct_copy = 1;
		else				
	        CopyMemory(AudioStreamBuf, buf, PayloadLength);
		
    }
    // 除了所有Object的第一個payload之外,其餘payload會走到這邊,但通常一個audio object只有一個payload,所以不會跑到這裡.
    else if(*Restart2 == 0)
    {
        //memcpy(&(AudioStreamBuf[Offset2Media]), buf, PayloadLength);
        //*AudioStreamLength = Offset2Media + PayloadLength;
        CopyMemory(AudioStreamBuf + *AudioStreamLength, buf, PayloadLength);
        *AudioStreamLength  = (*AudioStreamLength) + PayloadLength;
    }

	if(*AudioStreamLength == MediaObjectSize)   //Lsk 090622 : Read a full frame, tell MPEG4 decode
	{
		#if 1
        iisSounBufMng[0].size   = *AudioStreamLength;                        

		if(GetFirstPayload == 1)  //寫前一個完整audio frame.
		{
			if(asfPresentTime > (*asfBaseTime))
			{
				iisSounBufMng[0].time = ((u32)asfPresentTime - (u32)*PreAudioPresentTime);
				//printf("A Time: %d - %d = %d\n", (u32)asfPresentTime, (u32)(*PreAudioPresentTime), (u32)(iisSounBufMng[0].time));									
				*PreAudioPresentTime = asfPresentTime;	

				if(direct_copy)
				{
					u8* buffer;
					buffer = iisSounBufMng[0].buffer;
					iisSounBufMng[0].buffer = buf;			
					asfWriteAudiPayload(pMergeFile, &iisSounBufMng[0]);
					iisSounBufMng[0].buffer = buffer;									
				}
				else
				{
					asfWriteAudiPayload(pMergeFile, &iisSounBufMng[0]);				
				}
			}
		}
		#endif

	}
	
    return  AudioStreamBuf;
}
s32 asfExtractFile(s32 BeginSec, s32 EndSec)  //Lsk 120831 : asf file split to video/audio stream
{
    FS_FILE*                pFile;
    FS_FILE*                pAudioFile;

    u8  file_name[64];
    
    /* BJ: 0718 S*/
    u64                     Size,UsedSize,PacketCount;
    u8                      flag,length,flag2;
    ASF_HEADER_OBJECT       asfHeaderObject;
    ASF_DATA_OBJECT         asfDataObject;
    ASF_SIMPLE_INDEX_OBJECT asfSimpleIndexObject;

    ASF_FILE_PROPERTIES_OBJECT asfFilePropertiesObject;
    /*Payload Parsing Info*/
    u8  Mul_Payload = 0, PayloadFlag;
    u32 PacketLength, PaddingLength, SendTime, Offset2Media, MediaObjectNumber;
    u16 Duration;
    
    u32 ReplicatedDataLength, PayloadLength, PacketUsedSize, VideoStreamLength, size;
    s64 asfBaseTime, asfPresentTime, PreAudioPresentTime, PreVideoPresentTime;
	u32 MediaObjectSize;  ////Lsk 090622
    u8  StreamNum, KeyFrame, VideoStreamNum, AudioStreamNum, PayloadNum,  Restart1, Restart2;
    u8  *buf;
    
    /* Peter 070104 */
    u32 IndexNum, asfVideoIndex, asfAudioIndex, i, Offset;
    u8  err;
    u8  *pBuf;
    u16 video_value;
    ASF_AUDI_STREAM_PROPERTIES_OBJECT asfAudiStreamPropertiesObject;
#ifdef  ASF_AUDIO
    u16 audio_value;
    u8  *AudioStreamBuf;
    u32 AudioStreamLength;
    u8  StartAudio  = 1;//VopUsedByteSize,
#endif
    
    u8  StartFrame  = 1;//VopUsedByteSize,
    u8  *VideoStreamBuf;
    u8  Error       = 0;

	//u8  pre_playback_speed = 5;
	s16 CurrrentIndex = 0;  ////Lsk 090414 : add index range
	u16 PlayIndex;
	u8  KeyFramePacketCount;
	u8 	GetFirstPayload = 0;  //Lsk 090417 : AVSync timebase
    //u8  GetFirstVideoPayload = 1;  //Lsk 090417 : Fastward,Backward timebase 
    u8 tmp;
   
	StartPlayBack = 0;
	ResetPlayback = 0;
	Video_timebase = 0;
	PacketCount.lo  = 0;

    
    
    /* avoid compile warning */
    asfSimpleIndexObject    = asfSimpleIndexObject;
    PaddingLength           = PaddingLength;
    SendTime                = SendTime;
    Duration                = Duration;
    VideoStreamNum          = VideoStreamNum;
    AudioStreamNum          = AudioStreamNum;
    
    PacketCount.lo          = 0;
	//printf("<B,E>=<%d,%d>\n", BeginSec, EndSec);
	
	if(BeginSec)
	{
        Restart1    = 1;    // video restart flag
        Restart2    = 1;    // audio restart flag
        CurrrentIndex = BeginSec +3;
        ResetPlayback = 1;
		asfIndexTableRead = 0;
	}
	else
	{
        Restart1    = 0;
        Restart2    = 0;
		if(sysPlaybackThumbnail == READFILE_STOP_AT_FIRST_I_FRAME)
			asfIndexTableRead = 1;
    }
	
    /* BJ: 0718 E*/


    /* open file */
    if ((pFile = dcfOpen((signed char*)dcfBackupCurFile->pDirEnt->d_name, "r")) == NULL) {
        DEBUG_ASF("ASF open file error!!!\n");
        return 0;
    }

	//printf("@@@@ 2\n");
	/*
	
    length = strlen(dcfPlaybackCurFile->pDirEnt->d_name);
    memcpy_hw(file_name, dcfPlaybackCurFile->pDirEnt->d_name, length);
    file_name[length-1] = '4';
    file_name[length-2] = '6';
    file_name[length-3] = '2';    
    printf("pVideoFile = %s\n",file_name);
    if ((pVideoFile = dcfOpen(file_name, "w")) == NULL) {
        DEBUG_ASF("ASF open file error!!!\n");
        return 0;
    }

    */
       
    VideoBufMng[0].buffer   = VideoBuf;
    VideoStreamBuf          = (u8*)VideoBufMng[0].buffer;
       
    iisSounBufMng[0].buffer = iisSounBuf[0];
    AudioStreamBuf          = (u8*)iisSounBufMng[0].buffer;


    


    
	if(sysPlaybackThumbnail == READFILE_STOP_AT_FIRST_I_FRAME)
		asfIndexTableRead = 1;

    if(dcfRead(pFile, tempbuf, ASF_HEADER_SIZE_CEIL, &size) != 1) {
        DEBUG_ASF("ASF read file error!!!\n");        
        Error   = 1;
        goto ExitAsfReadFile;
    }
	//printf("@@@@ 3\n");
    buf = tempbuf;
    if(matchGuid(buf)!=ASF_Header_Object_Guid)  //Lsk 090817 file start must be ASF_Header_Object_Guid
    {
        Error   = 1;
        goto ExitAsfReadFile;
    }

    CloseFlag = 1;
	//printf("@@@@ 4\n");
    while(CloseFlag)
    {
        //printf("@");
        switch(matchGuid(buf))
        {
            case ASF_Header_Object_Guid:
                buf = asfReadHeaderObject(buf, 
                                          &asfHeaderObject, 
                                          &asfFilePropertiesObject, 
                                          &asfAudiStreamPropertiesObject, 
                                          &VideoStreamNum, 
                                          &AudioStreamNum);
                if(asfFilePropertiesObject.file_size.lo==0 && asfFilePropertiesObject.file_size.hi==0) //file size empty
                {
                    Error   = 1;
                   goto ExitAsfReadFile;
                }
                
                if(buf == 0) {
                    DEBUG_ASF("asfReadHeaderObject error!!!\n");
                    Error   = 1;
                    goto ExitAsfReadFile;
                }
                break;
            case ASF_Data_Object_Guid:
                //DEBUG_ASF("Match ASF_Data_Object_Guid\n");
                memcpy(&asfDataObject,buf,sizeof(ASF_DATA_OBJECT));
                buf += sizeof(ASF_DATA_OBJECT);
                UsedSize.lo = sizeof(ASF_DATA_OBJECT);
				/*** Lsk 090420 : Read Index Table ***/
				if(asfIndexTableRead == 0)
				{
					s32  CurrentOffset = dcfTell(pFile);
					u32     IndexSize;

					if(dcfSeek(pFile, ASF_HEADER_SIZE_CEIL + (asfFilePropertiesObject.data_packets_count.lo * asfFilePropertiesObject.maximum_data_packet_size), FS_SEEK_SET) == 0)  {
                		DEBUG_ASF("ASF file seek to IndexOffset error!!!\n");
                        Error   = 1;
                    #if (STILL_PLAYBACK_WHEN_ERROR == 0)
                        goto ExitAsfReadFile;
                    #endif
		           	} else {
    					IndexSize =  ((asfFilePropertiesObject.file_size.lo - (u32)(dcfTell(pFile)) + 511)/512)*512;
    					if(asfBurstReadIndexObject(pFile,IndexSize) == 0) {
    						DEBUG_ASF("Find index objet header error!!!\n");
                            Error   = 1;
                        #if (STILL_PLAYBACK_WHEN_ERROR == 0)
                            goto ExitAsfReadFile;
                        #endif
    		            }

						if(BeginSec)
						{
							if(CurrrentIndex > (asfIndexTableCount-1))
                            	CurrrentIndex = (asfIndexTableCount-1) ;							
						}						
                    }
    				if(dcfSeek(pFile, CurrentOffset, FS_SEEK_SET) == 0) {
                        DEBUG_ASF("ASF file seek to CurrentOffset error!!!\n");
                        Error   = 1;
                        goto ExitAsfReadFile;
    				}
	    			asfIndexTableRead = 1;
	    		}
				if(ResetPlayback)       //Lsk 090406 : clear buffer when ResetPlayback
				{
				    OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_FINISH, OS_FLAG_SET, &err);
					
					/*** Reset File Position***/
					Offset      = (asfIndexTable[CurrrentIndex].packet_number - PacketCount.lo)* asfFilePropertiesObject.maximum_data_packet_size;
					PacketCount.lo  = asfIndexTable[CurrrentIndex].packet_number;

					printf("Offset = %d\n", Offset);
					printf("CurrrentIndex = %d\n", CurrrentIndex);
					printf("asfIndexTable[%d].packet_number = %d\n", CurrrentIndex, asfIndexTable[CurrrentIndex].packet_number);
					printf("PacketCount.lo = %d\n", PacketCount.lo);
					//printf("sysPlaybackVideoStop = %d\n", sysPlaybackVideoStop);
					
					if(dcfSeek(pFile, Offset, FS_SEEK_CUR)==0) {
							printf("#### %d\n", __LINE__);
							DEBUG_ASF("2.seek error!!!\n");
                    		Error   = 1;
		                    goto ExitAsfReadFile;
	    		    }
					StartFrame  = 1;  //Lsk 090406 : To confirm start from I payload
					Restart1 = 1;
					KeyFramePacketCount = asfIndexTable[CurrrentIndex].packet_count;
					StartPlayBack = 0;
					ResetPlayback = 0;
				}


                while(sysPlaybackVideoStop == 0)                    
                {                				
                    //printf("# 1\n");
                    if(dcfRead(pFile, tempbuf, asfFilePropertiesObject.maximum_data_packet_size , &size) == 0) {
						printf("@222222\n");
                        Error                   = 1;
                        sysPlaybackVideoStop    = 1;
                        break;
                    }
                    buf = tempbuf;
                    //printf("# 2\n");
                    buf =  asfReadPacketHeader(buf, 
                                               &asfFilePropertiesObject, 
                                               &PacketUsedSize,
                                               &flag2,
                                               &Mul_Payload,
                                               &PacketLength,
                                               &PaddingLength,//Lsk 090304
                                               &UsedSize);
                    if(buf == 0) {
                        Error                   = 1;
                        sysPlaybackVideoStop    = 1;
                        break;
                    }
                    //printf("# 3\n");                        
                    if(Mul_Payload)
                    {
                    	PayloadFlag = *((u8 *)buf);
                        PayloadNum = (PayloadFlag & 0x3F);
                        buf += 1; // Payload Flag
                        UsedSize.lo += 1; // Payload Flag
                    }
                    else
                    	PayloadNum = 1;

                    while(PayloadNum != 0)
                    {
                    	
                    	PayloadNum--;
                        //printf("# 4\n");    
                        buf = asfReadPayloadHeader(buf,
												   &StreamNum, 
                                                   &KeyFrame,
                                                   flag2,
                                                   Restart1,
                                                   Restart2,
                                                   PayloadFlag,
                                                   &Offset2Media,
                                                   &PayloadLength,
                                                   &MediaObjectSize,   //Lsk 090622
                                                   &asfPresentTime,
                                                   &asfVideoIndex,
                                                   &asfAudioIndex,
                                                   &UsedSize,
                                                   Mul_Payload,    //Lsk 090304
                                                   PacketLength,
                                                   PaddingLength); 

						
							
						
                        if(StreamNum == 1 ) // video stream
                        {
                	       // printf("# 5\n");
                	        //VideoPresentTime = asfPresentTime;
							VideoStreamBuf  = asfExtractVideoPayload(buf, 
                                                                  VideoStreamBuf,
                                                                  KeyFrame,
                                                                  &StartFrame,
                                                                  &Restart1,
                                                                  &VideoStreamLength,
                                                                  Offset2Media,
                                                                  PayloadLength,
                                                                  MediaObjectSize,   //Lsk 090622
                                                                  &PreAudioPresentTime,
                                                                  &PreVideoPresentTime,                                                                                                                                    
                                                                  &asfBaseTime,
                                                                  asfPresentTime,
                                                                  &asfVideoIndex,
                                                                  &mpeg4taskrun,
                                                                  &UsedSize,
                                                                  pFile,
                                                                  &asfFilePropertiesObject,
                                                                  &PacketCount,
                                                                  &GetFirstPayload);	
                                                        
                        }
                        else if(StreamNum == 2 && sysPlaybackForward == 0) // audio stream
                        {                        	
                	       // printf("# 6\n");	
                   	        //AudioPresentTime = asfPresentTime;
                        	AudioStreamBuf  = asfExtractAudioPayload(buf, 
                                                                  AudioStreamBuf,
                                                                  &StartAudio,
                                                                  &Restart2,
                                                                  &AudioStreamLength,
                                                                  Offset2Media,
                                                                  PayloadLength,
                                                                  MediaObjectSize,   //Lsk 090622
                                                                  &asfAudioIndex,
                                                                  &AudioPlayback,
                                                                  &PreAudioPresentTime,                                                                  
                                                                  &asfBaseTime,
                                                                  asfPresentTime,

                                                                  GetFirstPayload);
                        
                        }

						if(asfPresentTime > (EndSec)*1000)
						{
							printf("asfPresentTime = %d, (EndSec+3)*1000 = %d\n", (u32)asfPresentTime, (u32)((EndSec+3)*1000));
							//printf("PacketCount = %d, data_packets_count = %d\n", PacketCount.lo, asfFilePropertiesObject.data_packets_count.lo);
							sysPlaybackVideoStop = 1;
							break;
						}
                        buf += PayloadLength;
                        UsedSize.lo += PayloadLength;
                    }
                    PacketCount.lo++;

					if(PacketCount.lo == asfFilePropertiesObject.data_packets_count.lo)
					{
						//printf("asfPresentTime = %d, (EndSec+3)*1000 = %d\n", (u32)asfPresentTime, (u32)((EndSec+3)*1000));
						printf("PacketCount = %d, data_packets_count = %d\n", PacketCount.lo, asfFilePropertiesObject.data_packets_count.lo);
						sysPlaybackVideoStop = 1;
						//break;
					}
					
                    PacketUsedSize = ((u32)buf) - PacketUsedSize;
                    if(PacketUsedSize != PacketLength)
                    {
                        buf += PacketLength - PacketUsedSize;
                        UsedSize.lo += PacketLength - PacketUsedSize;
                    }
                }
				
				CloseFlag   = 0;                
                break;
            case NoMatch_Guid:
            default:
                break;
        }
    }
        /* BJ: 0718 E*/



ExitAsfReadFile:

	//OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_FINISH, OS_FLAG_SET, &err);
    
    DEBUG_ASF("ASF Extract file finish!!!\n", err);    
    /* close file */            
    if(dcfClose(pFile, &tmp) == 0) {
        DEBUG_ASF("dcfClose() error!!!\n", err);
    }    
	sysPlaybackVideoStop = 0;	
}



//Lsk Note: 1.video has different resoultion. FHD/HD
//          aduio has different codec. PCM/ADPCM
//
//          2. duration > section 
//
//			3. UI Playback_save()
u32 AsfGetVideoTime(unsigned short Time_HMS)
{
	u8    hour, min, sec;
	u32   tmp;
	
	hour = (Time_HMS>>11);
	min  = (Time_HMS & 0x07E0)>>5;
	sec  = (Time_HMS & 0x001F)<<1;
		
	return (hour*60*60+min*60+sec);
	
}

u32 sec_to_hhmmss(u32 Sec)
{
	//File creat time add video VideoNextPresentTime
	u8    h1, m1, s1;
	u32   tmp;
	
	
	h1 = Sec/3600;
    m1 = (Sec - h1*3600) / 60;
    s1 = Sec - h1*3600 - m1*60;

	return h1*10000+m1*100+s1;
	
}


s32 FileBackup(u32 MergeBeginSec, u32 MergeEndSec)
{
	s32 ExtractBeginSec, ExtractEndSec;
	s32 FileCreatTime, FileModifyTime;
    u8 CreatedFile = 0;
	
	DEBUG_SHOW_START();	

	DEBUG_ASF("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");	
	DEBUG_ASF("UI : MergeBeginSec=%d, MergeEndSec=%d\n", MergeBeginSec, MergeEndSec);
	DEBUG_ASF("ASF: MergeBeginSec=%d, MergeEndSec=%d\n", sec_to_hhmmss(MergeBeginSec), sec_to_hhmmss(MergeEndSec));
	
	//dcSearchFileOnPlaybackDir(BIT_MASK(0), 'A', beginSec/60, endSec/60);
	//dcSearchFileOnPlaybackDir(1<<(ch), 'A', MergeBeginSec/60, MergeEndSec/60);	
		
    dcfBackupCurFile = dcfPlaybackCurFile;

	if(dcfBackupCurFile == NULL)
	{
		DEBUG_ASF("[Error][File Backup] Backup range cannot find any file\n");
		return 0;
	}

	
	//Lsk: creat a new backup file.
	asfVopWidth  = GetVideoWidth(dcfBackupCurFile->pDirEnt->d_name);
	asfVopHeight = GetVideoHeight(dcfBackupCurFile->pDirEnt->d_name);

	DEBUG_ASF("dcfBackupCurFile = %s , w,h=%d, %d\n", dcfBackupCurFile->pDirEnt->d_name, asfVopWidth, asfVopHeight);	
	if((asfVopWidth == 640)&&(asfVopHeight == 352))
	{
	    H264_config[0x07] = 0x1E;
	    H264_config[0x08] = 0xDA;
	    H264_config[0x09] = 0x02;
	    H264_config[0x0A] = 0x80;
	    H264_config[0x0B] = 0xB6;
	    H264_config[0x0C] = 0x40;
	}
	else if((asfVopWidth == 1280)&&(asfVopHeight == 720))
	{
	    H264_config[0x07] = 0x1E;
	    H264_config[0x08] = 0xDA;
	    H264_config[0x09] = 0x01;
	    H264_config[0x0A] = 0x40;
	    H264_config[0x0B] = 0x16;
	    H264_config[0x0C] = 0xE4;
	}
	else if((asfVopWidth == 1920)&&(asfVopHeight == 1072))
	{
	    H264_config[0x07] = 0x28;
	    H264_config[0x08] = 0xDA;
	    H264_config[0x09] = 0x01;
	    H264_config[0x0A] = 0xE0;
	    H264_config[0x0B] = 0x08;
	    H264_config[0x0C] = 0x79;
	}
	else if((asfVopWidth == 1920)&&(asfVopHeight == 1080))
	{
	    H264_config[0x07] = 0x28;
	    H264_config[0x08] = 0xDA;
	    H264_config[0x09] = 0x01;
	    H264_config[0x0A] = 0xE0;
	    H264_config[0x0B] = 0x08;
	    H264_config[0x0C] = 0x9F;
	    H264_config[0x0D] = 0x95;
	}

		
	while((dcfBackupCurFile!=NULL) && (MergeBeginSec < MergeEndSec))
	{
	    DEBUG_ASF("Merge dcfBackupCurFile start = %s\n", dcfBackupCurFile->pDirEnt->d_name);
		FileCreatTime  = AsfGetVideoTime(dcfBackupCurFile->pDirEnt->fsFileCreateTime_HMS);
		FileModifyTime = AsfGetVideoTime(dcfBackupCurFile->pDirEnt->fsFileModifiedTime_HMS);

		if(MergeBeginSec < FileCreatTime)
			MergeBeginSec = FileCreatTime; 
		ExtractBeginSec = MergeBeginSec - FileCreatTime;  //the first file may need to copy from the middle, but the rest should all Begin at 0 sec
		
		if(MergeEndSec > FileModifyTime)
			ExtractEndSec = FileModifyTime - FileCreatTime;	//"this file end time" not reach "the needed end time", copy the file to the end
		else
			ExtractEndSec = MergeEndSec - FileCreatTime;    //this file is the end one, choose the needed part of the file

		DEBUG_ASF("@@@@ Merge<%d,%d>, File<%d,%d>, Extract<%d,%d>\n\n", MergeBeginSec, MergeEndSec,
				  FileCreatTime, FileModifyTime, ExtractBeginSec, ExtractEndSec);

        if(ExtractEndSec < 0){
            printf("No More File Match the Time Range... Copy End\n");
            break;
        }

		sysPlaybackVideoStop = 0;
			
        if(CreatedFile == 0)
        {
        	if((pMergeFile = asfCreateFile(1)) == 0)
        	{
        		DEBUG_ASF("[Error][File Backup] Creat a new backup file fail\n");
        		return 0;
            }
            CreatedFile = 1;
        }

		asfExtractFile(ExtractBeginSec, ExtractEndSec);
		MergeBeginSec += (ExtractEndSec - ExtractBeginSec);		
        
		if(dcfBackupCurFile == dcfGetPlaybackFileListTail())
			dcfBackupCurFile = NULL;	
		else
			dcfBackupCurFile = dcfBackupCurFile->next;	

		DEBUG_ASF("Merge dcfBackupCurFile end = %s\n", dcfBackupCurFile->pDirEnt->d_name);
	}

    if(CreatedFile == 1)
    {
	if(asfCloseFile(pMergeFile) == 0)
	{
		DEBUG_ASF("[Error][File Backup] Close a new backup file fail\n");	
        return 0;
    }  
    	printf("Close file, copy ok\n\n\n\n\n");        
    }
	DEBUG_SHOW_END();
	
	return 0;
}
#endif

