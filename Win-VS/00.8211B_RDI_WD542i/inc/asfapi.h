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
#if MULTI_CHANNEL_VIDEO_REC
#include "../asf/inc/asf.h"
#endif

/* definition */

#if ((HW_BOARD_OPTION == AURUM_DVRBOX)||(HW_BOARD_OPTION==ELEGANT_KFCDVR)||(HW_BOARD_OPTION==GOS_DVRBOX)\
    ||(HW_BOARD_OPTION==VER100_CARDVR)||(HW_BOARD_OPTION==SIYUAN_CVR)||(HW_BOARD_OPTION==NEW_SIYUAN)\
    ||(HW_BOARD_OPTION==SUNIN_CARDVR)||(HW_BOARD_OPTION==SUNIN1_CARDVR))
// Record mode
#define AURUM_REC_MODE_NONE             0
#define AURUM_REC_MODE_AUTO             1
#define AURUM_REC_MODE_ON               2
#define AURUM_REC_MODE_REC              3
#endif

#define ASF_FILE_SIZE_10MB				( 10*1024*1024)
#define ASF_FILE_SIZE_20MB				( 20*1024*1024)
#define ASF_FILE_SIZE_50MB				( 50*1024*1024)
#define ASF_FILE_SIZE_100MB				(100*1024*1024)

/* cytsai: 0418 */
/* set ASF_PADDING_THRESHOLD to facilitate the utilization of data packet */
#define ASF_PADDING_THRESHOLD           0x00000020  /* new data packet if remained byte in data packet is less than threshold */
/* set (ASF_DATA_PACKET_SIZE > ASF_AUDIO_VIRTUAL_PACKET_SIZE) to facilitate (span = 1) of audio stream properties object */

#define ASF_DATA_PACKET_SIZE            0x00008000  /* 32kB byte/data_packet*/   
#define ASF_HEADER_SIZE_CEIL            ASF_DATA_PACKET_SIZE  /* header size ceil up to 512 byte boundary */ 

#define ASF_AUDIO_VIRTUAL_PACKET_SIZE       0x000000c8  /* 200 byte/audio_virtual_packet */

/* Peter 070104 */
//#define ASF_IDX_SIMPLE_INDEX_ENTRY_MAX        (6000)  /* 6000 simple index entry for 6000 sec audio/video sequence */
//#define ASF_IDX_SIMPLE_INDEX_ENTRY_MAX      (10800) /* 10800 simple index entry for 3 hours audio/video sequence */
#define ASF_IDX_SIMPLE_INDEX_ENTRY_MAX      (MPEG4_INDEX_BUF_SIZE / 6) /* index entry for 8 hours 10 minutes audio/video sequence */
#define ASF_MASS_WRITE                  1           /* 0: write packet by packet, 1: collect packets for mass write */
#define ASF_MASS_WRITE_HEADER           1           /* 0: write asf header by header, 1: asf header mass write */
#define ASF_MASS_WRITE_SIZE             ASF_DATA_PACKET_SIZE
/* Peter 070104 */
#define MAX_PAYLOAD_IN_PACKET           63  //Lsk 090410
#define ASF_DROP_FRAME_THRESHOLD        300  // default 10 frames



#if(HW_BOARD_OPTION==JSW_DVRBOX)
#define SD_FREE_SIZE_THR           (100*1024) //indicate sd-card free space less than 
#endif


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


#if((HW_BOARD_OPTION==DTY_IRDVR))
#define ASF_CAPTURE_EVENT_ALL           (ASF_CAPTURE_EVENT_MOTION_ENA | ASF_CAPTURE_EVENT_ALARM_ENA |ASF_CAPTURE_SCHEDULE_ENA)
#else
#define ASF_CAPTURE_EVENT_ALL           (ASF_CAPTURE_EVENT_GSENSOR_ENA | ASF_CAPTURE_EVENT_MOTION_ENA | ASF_CAPTURE_EVENT_ALARM_ENA | ASF_CAPTURE_EVENT_DUMMY_ENA)
#endif

#if (UI_VERSION == UI_VERSION_TRANWO) 
 #define DCF_OVERWRITE_THR_KBYTE         (500 * 1024)  //Lucian: 剩餘空間小於 XX, 則開始overwrirte  // 500M,解決 Section time=15min, Overwrite delete 內存空間不足的問題.
#else
 #define DCF_OVERWRITE_THR_KBYTE         (300 * 1024)  //Lucian: 剩餘空間小於 XX, 則開始overwrirte  //Lsk 090423 300M
#endif

/* enable debugging / audio or not */ 
#if(HW_BOARD_OPTION==HX_DN336)
#define ASF_AUDIO /* Peter 070104 */
#else
#define ASF_AUDIO /* Peter 070104 */
#endif

#define PREROLL                 0x0bb8         //Lsk 090309 3000 msec


/* Global variable */
extern u16 asfVopWidth;
extern u16 asfVopHeight;
extern u32 asfVopCount;
extern u32 asfVidePresentTime;
extern u8  CloseFlag;
#if MULTI_CHANNEL_VIDEO_REC
extern ASF_AUDIO_FORMAT asfAudiFormat;
#endif
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
#if ( (HW_BOARD_OPTION == MR6730_AFN)&&(CIU_OSD_METHOD_2) )
extern char  timeForRecord1[];//[32]
extern char  timeForRecord2[];//[32]
#else
extern char  timeForRecord1[20];
extern char  timeForRecord2[20];
#endif
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
extern u32 VideoDuration;  //Lsk 090507
#if ((HW_BOARD_OPTION == AURUM_DVRBOX)||(HW_BOARD_OPTION==ELEGANT_KFCDVR)||(HW_BOARD_OPTION==GOS_DVRBOX)\
    ||(HW_BOARD_OPTION==VER100_CARDVR)||(HW_BOARD_OPTION==SIYUAN_CVR)||(HW_BOARD_OPTION==NEW_SIYUAN)\
    ||(HW_BOARD_OPTION==SUNIN_CARDVR)||(HW_BOARD_OPTION==SUNIN1_CARDVR))
extern  u32 RecMode;
#endif
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
#if((HW_BOARD_OPTION==GOS_DVRBOX)||(HW_BOARD_OPTION == VER100_CARDVR))
extern  u8              PowerOnFlag;    // 1: Power on, 0: Power off
#endif
#if((HW_BOARD_OPTION==GOS_DVRBOX)||(HW_BOARD_OPTION==VER100_CARDVR)||(HW_BOARD_OPTION==SIYUAN_CVR)||(HW_BOARD_OPTION==NEW_SIYUAN))
extern  u8  DoCaptureVideo;
#endif
#if(ASF_ENCRYPTION==1)
extern u8 PassWordA[4], PassWordB[4];
#endif

enum 
{
    CAPTURE_STATE_WAIT = 0,
    CAPTURE_STATE_TRIGGER,    
    CAPTURE_STATE_TIMEUP,        
    CAPTURE_STATE_TEMP,        
};

/* function prototype */                               
extern void asfSetVideoSectionTime(u32 second);
extern void asfSetRecFileNum(s32 FileNum);
extern u32 GetVideoDuration(s8* pFileName);
extern s32 asfPausePlayback(u8 *Srcbuf , u8 *Dstbuf , u8 flag);
extern s32 asfCaptureVideoStop(void);
extern s32 asfPlaybackVideoStop(void);
extern u32 asfRemainRecSecForDiskFreeSpace(void);
extern s32 asfSetVideoResolution(u16 width, u16 height);
extern s64 VideoNextPresentTime;
#endif

