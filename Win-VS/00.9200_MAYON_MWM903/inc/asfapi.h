/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	asfapi.h

Abstract:

   	The application interface of the ASF file.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __ASF_API_H__
#define __ASF_API_H__

#include "fsapi.h"


/* definition */

#define SWITCH_TO_HDMI 1
#define SWITCH_TO_PANEL 0

#define ASF_FILE_SIZE_10MB				( 10*1024*1024)
#define ASF_FILE_SIZE_20MB				( 20*1024*1024)
#define ASF_FILE_SIZE_50MB				( 50*1024*1024)
#define ASF_FILE_SIZE_100MB				(100*1024*1024)

/* cytsai: 0418 */
/* set ASF_PADDING_THRESHOLD to facilitate the utilization of data packet */
#define ASF_PADDING_THRESHOLD           0x00000020  /* new data packet if remained byte in data packet is less than threshold */
/* set (ASF_DATA_PACKET_SIZE > ASF_AUDIO_VIRTUAL_PACKET_SIZE) to facilitate (span = 1) of audio stream properties object */

#define ASF_DATA_PACKET_SIZE            0x00008000  /* 32kB byte/data_packet*/   

#define ASF_HEADER_SIZE_CEIL            (ASF_DATA_PACKET_SIZE*2)  /* header size ceil up to 512 byte boundary */ 

#define ASF_AUDIO_VIRTUAL_PACKET_SIZE       0x000000c8  /* 200 byte/audio_virtual_packet */

/* Peter 070104 */
//#define ASF_IDX_SIMPLE_INDEX_ENTRY_MAX        (6000)  /* 6000 simple index entry for 6000 sec audio/video sequence */
//#define ASF_IDX_SIMPLE_INDEX_ENTRY_MAX      (10800) /* 10800 simple index entry for 3 hours audio/video sequence */
#define ASF_IDX_SIMPLE_INDEX_ENTRY_MAX      (MPEG4_INDEX_BUF_SIZE / 6) /* index entry for 8 hours 10 minutes audio/video sequence */
#define ASF_MASS_WRITE                  1           /* 0: write packet by packet, 1: collect packets for mass write */
#define ASF_MASS_WRITE_HEADER           1           /* 0: write asf header by header, 1: asf header mass write */
#define ASF_MASS_WRITE_SIZE             ASF_HEADER_SIZE_CEIL
/* Peter 070104 */
#define MAX_PAYLOAD_IN_PACKET           63  //Lsk 090410
#define ASF_DROP_FRAME_THRESHOLD        300  // default 10 frames


// Capture mode
#define ASF_CAPTURE_NORMAL              0x00000000
#define ASF_CAPTURE_OVERWRITE_ENA       0x00000001
#define ASF_CAPTURE_OVERWRITE_DIS       0x00000000
#define ASF_CAPTURE_EVENT_GSENSOR_ENA   0x00000002
#define ASF_CAPTURE_EVENT_GSENSOR_DIS   0x00000000
#define ASF_CAPTURE_EVENT_MOTION_ENA    0x00000004
#define ASF_CAPTURE_EVENT_MOTION_DIS    0x00000000
#define ASF_CAPTURE_EVENT_ALARM_ENA     0x00000008
#define ASF_CAPTURE_EVENT_ALARM_DIS     0x00000000
#define ASF_CAPTURE_EVENT_DUMMY_ENA     0x00000010  // 想進錄影模式但又不想真的寫檔時用
#define ASF_CAPTURE_EVENT_DUMMY_DIS     0x00000000
#define ASF_CAPTURE_SCHEDULE_ENA        0x00000020
#define ASF_CAPTURE_SCHEDULE_DIS        0x00000000
#define ASF_CAPTURE_PIRTIGGER_ENA       0x00000040
#define ASF_CAPTURE_PIRTIGGER_DIS       0x00000000

#define ASF_REC_TIME_LEN	150

#define ASF_CAPTURE_EVENT_ALL           (ASF_CAPTURE_EVENT_GSENSOR_ENA | ASF_CAPTURE_EVENT_MOTION_ENA | ASF_CAPTURE_EVENT_ALARM_ENA | ASF_CAPTURE_EVENT_DUMMY_ENA)

#if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION == MR9200_RX_JIT_M916HN4) ||\
    (HW_BOARD_OPTION  == MR9200_RX_ROULE) ||\
     (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM903) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
     (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM018))
#define DCF_OVERWRITE_THR_KBYTE         (900 * 1024)  //Lucian: 剩餘空間小於 XX, 則開始overwrirte  //Lucian: 由於目前用6M RF, 10 min file size 超過300M
#else
#define DCF_OVERWRITE_THR_KBYTE         (500 * 1024)  //Lucian: 剩餘空間小於 XX, 則開始overwrirte  //Lucian: 由於目前用6M RF, 10 min file size 超過300M
#endif

/* enable debugging / audio or not */ 
#define ASF_AUDIO /* Peter 070104 */

#define PREROLL                 0x0bb8         //Lsk 090309 3000 msec


/* Global variable */
extern u16 asfVopWidth;
extern u16 asfVopHeight;
extern u32 asfVopCount;
extern u32 asfVidePresentTime;
extern u8  CloseFlag;
extern u32 asfAudiChunkCount;
extern u32 asfAudiPresentTime;
extern u32 WantChangeFile;
extern u32 LastAudio;
extern u32 LastVideo;
extern u8  GetLastAudio;
extern u8  GetLastVideo;
extern u32 asfCaptureMode;
extern u32 asfSectionTime;
extern u8  asfIndexTableRead;
extern char  timeForRecord1[20];
extern char  timeForRecord2[20];
extern u32 curr_free_space;  //Lsk 090422 : for overwrite information
extern u32 curr_record_space;  //Lsk 090422 : for overwrite information
extern u32 VideoDuration;  //Lsk 090507
extern u32 asfRecTimeLen;
extern u32 asfRecTimeLenTotal;
extern u32 PreRecordTime;
extern u32 asfEventExtendTime;
extern u32 asfEventInterval;    // 前一次Event經過多久後才允許下次Event進來, Second unit.
extern u32 EventTrigger;
extern u32 asfIndexTableCount;
extern u8  curr_playback_speed;
extern u8  curr_slow_speed;
extern u32 VideoDuration;  //Lsk 090507
extern u8  video_playback_speed;
extern u32 IsuIndex;
extern u32 P2PVideoBufMngWriteIdx;
#if CDVR_LOG
#define LOG_INDEX_NUM   512
extern  u8*             szLogFile;
extern  u32             LogFileLength;
extern  OS_EVENT*       LogFileSemEvt;
extern  u8*             LogFileIndex[LOG_INDEX_NUM];
extern  u16             LogFileStart;
extern  u16             LogFileNextStart;
extern  u16             LogFileCurrent;
extern  u8*             pLogFileEnd;
extern  u8*             pLogFileMid;
#endif
extern u8  AsfAudioZeroBuf[IIS_CHUNK_SIZE];

extern unsigned char thumbnailID[8];
enum 
{
    CAPTURE_STATE_WAIT = 0,
    CAPTURE_STATE_TRIGGER,    
    CAPTURE_STATE_TIMEUP,        
    CAPTURE_STATE_TEMP,        
};

/* function prototype */
extern s32 asfInit(void);
extern void asfSetVideoSectionTime(u32 second);
extern void asfSetRecFileNum(s32 FileNum);
extern u32 GetVideoDuration(s8* pFileName);
extern u32 GetVideoResolutionsetTV(s8* pFileName);
extern u32 GetVideoWidth(s8* pFileName);
extern u32 GetVideoHeight(s8* pFileName);
extern u32 GetCaptureVideoStatus(void);
extern s32 asfPausePlayback(u8 *Srcbuf , u8 *Dstbuf , u8 flag);
extern s32 asfCaptureVideoStop(void);
extern s32 asfPlaybackVideoStop(void);
extern u32 asfRemainRecSecForDiskFreeSpace(void);
extern s32 asfSetVideoResolution(u16 width, u16 height);
extern s64 VideoNextPresentTime;
extern s32 asfReadFile(u32 u32PacketOffset);
extern s32 asfRepairFile(signed char* file_name);
extern s32 asfSplitFile(u32 u32PacketOffset);
extern s32 FileBackup(u32 MergeBeginSec, u32 MergeEndSec);

typedef __packed struct _ASF_IDX_SIMPLE_INDEX_ENTRY 
{
    u32 packet_number;          /* 0x00000000, = the closest key frame (I-VOP) prior to current time interval */
    u16 packet_count;           /* 0x0000, = number of packets spans for the closest key frame (I-VOP). */
} ASF_IDX_SIMPLE_INDEX_ENTRY;

#endif

