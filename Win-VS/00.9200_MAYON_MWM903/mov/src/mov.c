/*

Copyright (c) 2010 Mars Semiconductor Corp.

Module Name:

    mov.c

Abstract:

    The routines of QuickTime MOV file.

Environment:

    ARM RealView Developer Suite

Revision History:
    
    2010/01/11  Peter Hsu Create

*/

#include "general.h"
#include "board.h"

#include "task.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "mov.h"
#include "movapi.h"
#include "asfapi.h"
#include "mpeg4api.h"
#include "iisapi.h"
/* Peter: 0727 S*/
#include "isuapi.h"
#include "iduapi.h"
#include "ipuapi.h"
#include "siuapi.h"
#include "sysapi.h"
#include "../asf/inc/asf.h"
#include "timerapi.h"
#include "osapi.h"
#include "gpioapi.h"
#include "adcapi.h"
#include "dmaapi.h"
#include "uiapi.h"

#if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MOV)  
/* Peter: 0727 E*/

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
 
/* enable debugging / audio or not */ 
//#define MP4_AUDIO

/* max number of table entry */
#define MOV_VIDE_SAMPLE_MAX (30 * 60 * 10)

#define MOV_VIDE_INTRA_FRAME_INTERVAL (10) /* 10 frames */
#define MOV_VIDE_SYNC_SAMPLE_MAX (MOV_VIDE_SAMPLE_MAX / MOV_VIDE_INTRA_FRAME_INTERVAL + 1)

/* mh@2006/11/13: group specified numbers of sample into one chunk */
#define MOV_VIDE_SAMPLES_PER_CHUNK (30) 

#define MOV_AUDI_SAMPLES_PER_CHUK (0x3E8)
/* mh@2006/11/13: END */

/* Peter: 0727 S*/
#define MP4_BOX_NUM_MAX                16
/* Peter: 0727 E*/

/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

/* current file offset */ 
s64 movFileOffset;

/* video related */
u8 movVideHeader[0x20];
u32 movVideHeaderSize;
u16 movVopWidth, movVopHeight;
u32 movVopCount, movIVopCount;
u32 movVideStsdAtomSize;
MOV_VIDE_SAMPLE_SIZE_ENTRY movVideSampleSizeTable[MOV_VIDE_SAMPLE_MAX];
MOV_VIDE_SAMPLE_TIME_ENTRY movVideSampleTimeTable[MOV_VIDE_SAMPLE_MAX];  
MOV_VIDE_SYNC_SAMPLE_NUMBER_ENTRY movVideSyncSampleNumberTable[MOV_VIDE_SYNC_SAMPLE_MAX];

/* mh@2006/11/14: group specified numbers of sample into one chunk */
u32 movChunkOffsetTableEntryCount;
MOV_VIDE_CHUNK_OFFSET_ENTRY movVideChunkOffsetTable[MOV_VIDE_SAMPLE_MAX];
MOV_VIDE_SAMPLE_TO_CHUNK_ENTRY movVideSampleToChunkTable;
/* mh@2006/11/14: END */
    
/* mh@2006/11/14: reduce the entry count of Time to Sample Box */
u32 movTimeToSampleTableEntryCount;

//u32 mp4SampleToChunkTableEntryCount;
/* mh@2006/11/14: END */

/* audio related */
u32 movSounSampleEntryCount, movSounSampleEntryDuration;
u32 movAudisamplesperChunkCnt;
u32 movAudiPresentTime;
u32 movVidePresentTime;
MOV_AUDIO_FORMAT movAudioFormat;
MOV_SOUN_CHUNK_OFFSET_ENTRY movSounChunkOffsetTable[IIS_SOUN_SAMPLE_MAX]; 
MOV_SOUN_SAMPLE_TO_CHUNK_ENTRY movSounSampleChunkTable[IIS_SOUN_SAMPLE_MAX];



extern u32 VideoPictureIndex;
extern s32 isu_avifrmcnt;

/* Peter: 0727 S*/
/* playback related */
/* Peter: 0727 E*/

/* mh@2006/11/21: copied from avi.c to be compatible with Peter's new MPEG4 driver */
extern u32 siuSkipFrameRate;
/* mh@2006/11/21: END */

extern u16 global_Mp4_count;    //civic 070826
extern  u32 unMp4FrameDuration[SIU_FRAME_DURATION_NUM]; /* mh@2006/11/22: for time stamp control */ /* Peter 070403 */

extern u32 CurrentAudioSize;
extern u32 CurrentVideoSize;

extern u32 asfAudiPresentTime;
extern u32 asfVidePresentTime;
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 movCaptureVideoFile(s32);
/* Peter: 0727 S*/
s32 mp4FindSubBoxes(FS_FILE*, FS_i32, u32*, FS_i32*, u32*);
s32 mp4FindMp4Boxes(FS_FILE*, FS_i32*, FS_i32*, FS_i32*);
s32 mp4FindMoovSubBoxes(FS_FILE*, FS_i32, FS_i32*, FS_i32*, FS_i32*);
s32 mp4FindTrakSubBoxes(FS_FILE*, FS_i32, FS_i32*, FS_i32*, FS_i32*);
s32 mp4FindMdiaSubBoxes(FS_FILE*, FS_i32, FS_i32*, FS_i32*, FS_i32*);
s32 mp4FindMinfSubBoxes(FS_FILE*, FS_i32, FS_i32*, FS_i32*, FS_i32*, FS_i32*, FS_i32*);
s32 mp4FindDinfSubBoxes(FS_FILE*, FS_i32, FS_i32*);
s32 mp4FindStblSubBoxes(FS_FILE*, FS_i32, FS_i32*, FS_i32*, FS_i32*, FS_i32*, FS_i32*, FS_i32*);
/* Peter: 0727 E*/

/* MP4 Common */ 
s32 movWriteFtypBox(FS_FILE*);
s32 movWriteWideBox(FS_FILE* );
s32 movWriteMdatBoxPre(FS_FILE*);
s32 movWriteMdatAtomPost(FS_FILE*, u32);
s32 movWriteVidemdat(FS_FILE*, VIDEO_BUF_MNG*);
s32 movWriteAudimdat(FS_FILE*, IIS_BUF_MNG*);
s32 movWriteMoovBox(FS_FILE*);
s32 movWriteMvhdBox(FS_FILE*);
/* Peter: 0727 S*/
s32 mp4ReadFtypBox(FS_FILE*, FS_i32);
s32 mp4ReadMoovBox(FS_FILE*, FS_i32);
s32 mp4ReadMvhdBox(FS_FILE*, FS_i32);
s32 mp4ReadIodsBox(FS_FILE*, FS_i32);
s32 mp4ReadTrakBox(FS_FILE*, FS_i32);
s32 mp4ReadMdiaBox(FS_FILE*, FS_i32);
s32 mp4ReadMdhdBox(FS_FILE*, FS_i32);
s32 mp4ReadHdlrBox(FS_FILE*, FS_i32, u32*);
s32 mp4ReadTkhdBox(FS_FILE*, FS_i32);
/* Peter: 0727 E*/

/* MP4 Video Track */
s32 movWriteVideTrakBox(FS_FILE*);
s32 movWriteVideTkhdBox(FS_FILE*);
/* Peter: 0727 S*/
s32 movWriteVideMdiaBox(FS_FILE*);
/* Peter: 0727 E*/
s32 movWriteVideMdhdBox(FS_FILE*);
s32 movWriteVideHdlrBox(FS_FILE*);
s32 movWriteVideMinfBox(FS_FILE*);
s32 movWriteVideDinfBox(FS_FILE*);
s32 movWriteVideDrefBox(FS_FILE*);
s32 movWriteVideStblBox(FS_FILE*);
s32 movWriteVideStsdBox(FS_FILE*);
s32 movWriteVideStszBox(FS_FILE*);
s32 movWriteVideSttsBox(FS_FILE*);
s32 movWriteVideStscBox(FS_FILE*);
s32 movWriteVideStcoBox(FS_FILE*);
s32 movWriteVideStssBox(FS_FILE*);
s32 movWriteVideVmhdBox(FS_FILE*); 
/* Peter: 0727 S*/
s32 mp4ReadVideMinfBox(FS_FILE*, FS_i32);
s32 mp4ReadVideDinfBox(FS_FILE*, FS_i32);
s32 mp4ReadVideDrefBox(FS_FILE*, FS_i32);
s32 mp4ReadVideStblBox(FS_FILE*, FS_i32);
s32 mp4ReadVideStsdBox(FS_FILE*, FS_i32);
s32 mp4ReadVideStszBox(FS_FILE*, FS_i32);
s32 mp4ReadVideSttsBox(FS_FILE*, FS_i32);
s32 mp4ReadVideStscBox(FS_FILE*, FS_i32);
s32 mp4ReadVideStcoBox(FS_FILE*, FS_i32);
s32 mp4ReadVideStssBox(FS_FILE*, FS_i32);
s32 mp4ReadVideVmhdBox(FS_FILE*, FS_i32); 
/* Peter: 0727 E*/

/* mh@2006/11/21: get visual sample location from sample number, 
                  chunk offset table, and sample size table */
s32 mp4GetVisualSampleLocation(u32, FS_i32*); 
/* mh@2006/11/21: END */

/* MP4 Soun Track */
s32 movAudioInit(void);
s32 movWriteSounTrakAtom(FS_FILE*);
s32 movWriteSounTkhdAtom(FS_FILE*);
/* Peter: 0727 S*/
s32 movWriteSounMdiaAtom(FS_FILE*);
/* Peter: 0727 E*/
s32 movWriteSounMdhdAtom(FS_FILE*);
s32 movWriteSounHdlrAtom(FS_FILE*);
s32 movWriteSounMinfAtom(FS_FILE*);
s32 movWriteSounDinfAtom(FS_FILE*);
s32 movWriteSounDrefAtom(FS_FILE*);
s32 movWriteSounStblAtom(FS_FILE*);
s32 movWriteSounStsdAtom(FS_FILE*);
s32 movWriteSounStszAtom(FS_FILE*);
s32 movWriteSounSttsAtom(FS_FILE*);
s32 movWriteSounStscAtom(FS_FILE*);
s32 movWriteSounStcoAtom(FS_FILE*);
s32 movWriteSounSmhdAtom(FS_FILE*);
/* Peter: 0727 S*/
s32 mp4ReadSounMinfAtom(FS_FILE*, FS_i32);
s32 mp4ReadSounDinfAtom(FS_FILE*, FS_i32);
s32 mp4ReadSounDrefAtom(FS_FILE*, FS_i32);
s32 mp4ReadSounStblAtom(FS_FILE*, FS_i32);
s32 mp4ReadSounStsdAtom(FS_FILE*, FS_i32);
s32 mp4ReadSounStszAtom(FS_FILE*, FS_i32);
s32 mp4ReadSounSttsAtom(FS_FILE*, FS_i32);
s32 mp4ReadSounStscAtom(FS_FILE*, FS_i32);
s32 mp4ReadSounStcoAtom(FS_FILE*, FS_i32);
s32 mp4ReadSounSmhdAtom(FS_FILE*, FS_i32);
/* Peter: 0727 E*/


/* Peter: 0727 E*/
 
/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */ 

/*

Routine Description:

    Initialize 3GPP/MP4 file.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movInit(void)
{

    /* initialize video */
    mpeg4Init();

#ifdef MOV_AUDIO    
    /* initialize audio */
    movAudioInit();

    /* initialize audio */
    iisInit();
#endif
    
    return 1;   
}

s32 movCaptureVideo(s32 ZoomFactor, u32 Mode)
{
    int     i;
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
    VideoPictureIndex               = 0;
    VideoBufMngReadIdx          = 0;
    VideoBufMngWriteIdx         = 0;
    asfVopCount                     = 0;

    siuSkipFrameRate                = 0;
    MPEG4_Mode                      = 0;    // 0: record, 1: playback
    CurrentVideoSize                = 0;
    asfCaptureMode                  = Mode;
    movTimeToSampleTableEntryCount  = 0;
    movVopCount                     = 0;
    movIVopCount                    = 0;
    movChunkOffsetTableEntryCount   = 0;
    movFileOffset                   = 0;
    movSounSampleEntryCount         = 0;
    movAudiPresentTime              = 0;
    movVidePresentTime              = 0;
    movAudisamplesperChunkCnt       = 1;
       
    for(i = 0; i < VIDEO_BUF_NUM; i++) {
        VideoBufMng[i].buffer   = VideoBuf;
    }
    // reset MPEG-4 hardware
    SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
    SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100; 
    mpeg4SetVideoResolution(mpeg4Width, mpeg4Height);   /*Peter 1116 S*/
    memset(unMp4FrameDuration, 0, sizeof(unMp4FrameDuration));
    memset(ISUFrameDuration, 0, sizeof(ISUFrameDuration));

#ifdef MOV_AUDIO
    iisSounBufMngReadIdx    = 0;
    iisSounBufMngWriteIdx   = 0;
    IISMode                 = 0;    // 0: record, 1: playback
    IISTime                 = 0;    /* Peter 070104 */
    IISTimeUnit             = (IIS_RECORD_SIZE * 1000) / 8000;  /* milliscends */ /* Peter 070104 */
    CurrentAudioSize        = 0;
    /* initialize sound buffer */
    for(i = 0; i < IIS_BUF_NUM; i++) {
        iisSounBufMng[i].buffer     = iisSounBuf[i];
    }
#endif  // MOV_AUDIO

    /* file */
    //asfDataPacketCount = 0;
    //asfIndexTableIndex = 0;
    //asfIndexEntryTime  = 0;
    
    /* refresh semaphore state */
    Output_Sem();

    OSSemSet(VideoCmpSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoCmpSemEvt is %d.\n", err);
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
#ifdef  MOV_AUDIO
    /*
    while(iisCmpSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisCmpSemEvt);
    }
    */
    OSSemSet(iisCmpSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: iisCmpSemEvt is %d.\n", err);
    }
    Output_Sem();
    while(iisTrgSemEvt->OSEventCnt > (IIS_BUF_NUM - 2)) {
        OSSemAccept(iisTrgSemEvt);
    }
    while(iisTrgSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
        OSSemPost(iisTrgSemEvt);
    }
    Output_Sem();
#endif  

/*CY 0629 S*/   
    /* write video file */
    if (movCaptureVideoFile(ZoomFactor) == 0)
    {
        /* reset the capture control if error */
        sysCaptureVideoStart    = 0;
        sysCaptureVideoStop     = 1;       
    }
    else
    { //no error
        //global_Mp4_count++;
    }
/*CY 0629 E*/   
    
/*CY 0613 S*/

#ifdef  MOV_AUDIO
    /*
    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }
    */
    OSSemSet(iisTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
    }
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

    /* delay until mpeg4 and IIS task reach pend state */
    OSTimeDly(2);

    /* suspend mpeg4 task */
    mpeg4SuspendTask();
#ifdef MOV_AUDIO    
    iisStopRec();
    iisSuspendTask();
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
#ifdef MOV_AUDIO    /*
    while(iisCmpSemEvt->OSEventCnt > 0) {
    OSSemAccept(iisCmpSemEvt);
    }
    */
    OSSemSet(iisCmpSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: iisCmpSemEvt is %d.\n", err);
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
#endif  
/*CY 0613 E*/   

    DEBUG_ASF("ASF file captured - VOP count = %d\n", asfVopCount);
#ifdef MOV_AUDIO
    DEBUG_ASF("ASF file captured - Audio count = %d\n", asfAudiChunkCount);
#endif  
    //DEBUG_ASF("ASF file captured - File size = %d bytes\n", asfHeaderSize + asfDataSize + asfIndexSize);
    //DEBUG_ASF("siuLifeFrameCount = %d, isu_avifrmcnt = %d, isu_unusedframe = %d\n\n", siuLifeFrameCount, isu_avifrmcnt, isu_unusedframe);

    // reset IIS hardware
    iisReset(IIS_SYSPLL_SEL_48M);
    
    

    return 1;
}

/*

Routine Description:

    Capture video file.

Arguments:

    ZoomFactor - Zoom factor.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movCaptureVideoFile(s32 ZoomFactor) /*BJ 0530 S*/
{
    FS_FILE*    pFile;
    u16         video_value;
    u16         video_value_max;
    u32         monitor_value;
    u32         flag, size;
    u32         unCurrentSampleDuration;
    u32         unPreviousSampleDuration;
    u32         unSampleCount = 0;
    u32         timetick;
    u8*         pBuf;
    u8          ledon = 0;
    u8          err;
#if FINE_TIME_STAMP
    s32         TimeOffset;
#endif      

#ifdef MOV_AUDIO
    u16 audio_value;
    u16 audio_value_max;
#endif  
	u8  tmp;

    WantChangeFile  = 0;
    LastAudio       = 0;
    LastVideo       = 0;
    GetLastAudio    = 0;
    GetLastVideo    = 0;
#if FINE_TIME_STAMP
    // Enable timer3 for fine tune frame time
    timer3Setting();
#endif
    /* create next file */
    if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_MOV, 0)) == NULL)
        return 0;
            
    /* write FTYP Box */
    if (movWriteFtypBox(pFile) == 0)
    {
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }        

    if (movWriteWideBox(pFile) == 0)
    {
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }     
    
    /* write MDAT Box pre*/
    if (movWriteMdatBoxPre(pFile) == 0)
    {
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }        

    /* file */  
    movFileOffset = sizeof(MOV_FTYP_BOX_T) + sizeof(MOV_MDAT_BOX_T)  + sizeof(MOV_WIDE_BOX_T);
    DEBUG_ASF("VideoCmpSemEvt = %d\n", VideoCmpSemEvt->OSEventCnt);
    DEBUG_ASF("VideoTrgSemEvt = %d\n", VideoTrgSemEvt->OSEventCnt);
    DEBUG_ASF("VideoCpleSemEvt = %d\n", VideoCpleSemEvt->OSEventCnt);
            
    video_value     = 0;
    video_value_max = 0;
    monitor_value   = 0;
#ifdef MOV_AUDIO
    DEBUG_ASF("iisCmpSemEvt = %d\n", iisCmpSemEvt->OSEventCnt);
    DEBUG_ASF("iisTrgSemEvt = %d\n", iisTrgSemEvt->OSEventCnt);
    audio_value     = 0;
    audio_value_max = 0;
#endif  

#if FINE_TIME_STAMP
    timerCountRead(2, (u32*) &TimeOffset);
    IISTimeOffset   = TimeOffset >> 8;
#endif

    iduCaptureVideo(mpeg4Width,mpeg4Height);
    isuCaptureVideo(ZoomFactor);
    ipuCaptureVideo();

#if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP)
    timerCountRead(2, (u32*) &TimeOffset);
	IISTimeOffset   = TimeOffset >> 8;
#elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
	timerCountRead(1, (u32*) &TimeOffset);
    IISTimeOffset   = TimeOffset / 100;
#endif	

#ifdef MOV_AUDIO    
    iisResumeTask();
    iisStartRec();
  #if AUDIO_IN_TO_OUT
    iisResumePlaybackTask();
  #endif  	
#endif  // #ifdef ASF_AUDIO
    siuCaptureVideo(ZoomFactor);


    if(!sysTVOutOnFlag) //Pannel out
    {
        {
            uiOSDPreviewInit();
            IduEna |= 0x02;
        }
    }
    else //TV-out
    {
        //DEBUG_ASF("Trace4: Capture video...%x\n",tvFRAME_CTL);        
        uiOSDPreviewInit();
        if(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)
            osdDrawVideoOn(UI_OSD_CLEAR);
        else
            osdDrawVideoOn(UI_OSD_DRAW);    

    }
    sysVoiceRecStart    = 2;
    RTCseconds          = 0;

    timerCountPause(1, 0);
    timerCountEnable(1,1);
    timerInterruptEnable(1,1);  // Using RTC record file time,timer2 for for counting

    //--------------------Start to REC-----------------------//
#if ( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )
    while(isu_avifrmcnt < 4)
    {
       OSTimeDly(1);
    }

    isu_avifrmcnt=0;
    mp4_avifrmcnt=0;
    OSSemSet(isuSemEvt, 0, &err);    
    sysReady2CaptureVideo=1;
    while(isu_avifrmcnt < 1)
    {
      OSTimeDly(1);
    }

#else
        sysReady2CaptureVideo=1;

        while(isu_avifrmcnt < 1)
        {
            OSTimeDly(1);
        }
#endif

    if(MPEG4_Task_Go) {
        OSSemAccept(VideoTrgSemEvt);
    }
    mpeg4ResumeTask();
    
    /*CY 0613 S*/
    timetick = OS_tickcounter;

    /*CY 0613 S*/
    while (sysCaptureVideoStop == 0)
    /*CY 0613 E*/
    {       
#ifdef MOV_AUDIO
            // Write audio payload
        if(WantChangeFile || (video_value == 0) || (movAudiPresentTime <= movVidePresentTime)) 
        {
            if((asfCaptureMode == ASF_CAPTURE_NORMAL) || ((WantChangeFile == 0) || ((GetLastAudio == 1) && (((LastAudio + 1) % IIS_BUF_NUM) != iisSounBufMngReadIdx)))) 
            {
                audio_value = OSSemAccept(iisCmpSemEvt);
                if (audio_value > 0)
                {
                    if(audio_value_max < audio_value)
                        audio_value_max = audio_value;
                    if (movWriteAudimdat(pFile, &iisSounBufMng[iisSounBufMngReadIdx]) == 0) 
                    {
                        DEBUG_ASF("ASF write audio payload error!!!\n");
                        dcfCloseFileByIdx(pFile, 0, &tmp);
                        return 0;
                    }   
                    iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                    //DEBUG_ASF("Trace: IIS frame written.\n");
                    OSSemPost(iisTrgSemEvt);
                }
            }
        }
        
            // Write video payload
        if(WantChangeFile || (audio_value == 0) || (movAudiPresentTime >= movVidePresentTime)) 
        {
#endif      // MOV_AUDIO
            if((asfCaptureMode == ASF_CAPTURE_NORMAL) || ((WantChangeFile == 0) || ((GetLastVideo == 1) && (((LastVideo + 1) % VIDEO_BUF_NUM) != VideoBufMngReadIdx)))) 
            {
                video_value = OSSemAccept(VideoCmpSemEvt);
                if (video_value > 0)
                {
                    if(video_value_max < video_value)
                        video_value_max = video_value;
                    if (movWriteVidemdat(pFile, &VideoBufMng[VideoBufMngReadIdx]) == 0)
                    {
                        DEBUG_ASF("ASF write video payload error!!!\n");
                        dcfCloseFileByIdx(pFile, 0, &tmp);
                        return 0;
                    }   
                    VideoBufMngReadIdx = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                    //DEBUG_ASF("Trace: MPEG4 frame written.\n");
                    OSSemPost(VideoTrgSemEvt);              
                }
            }
        }

#ifdef MOV_AUDIO
        // Skip siu frames for release bandwidth to SD card writing
        monitor_value   = (video_value > audio_value) ? video_value : audio_value;
#else
        monitor_value   = video_value;
#endif

        if (monitor_value < 10) {       // not skip siu frame
            if(siuSkipFrameRate != 0) {
                siuSkipFrameRate    = 0;
                DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
            }
        } else if(monitor_value < 20) {
            if(siuSkipFrameRate != 2) {
                siuSkipFrameRate    = 2;
                DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
            }
        } else if(monitor_value < 30) {
            if(siuSkipFrameRate != 4) {
                siuSkipFrameRate    = 4;
                DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
            }
        } else if(monitor_value < 40) {
            if(siuSkipFrameRate != 6) {
                siuSkipFrameRate    = 6;
                DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
            }
        } else if(monitor_value < 50) {
            if(siuSkipFrameRate != 8) {
                siuSkipFrameRate    = 8;
                DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
            }
        } else if(monitor_value < 60) {
            if(siuSkipFrameRate != 10) {
                siuSkipFrameRate    = 10;
                DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
            }
        } else if(monitor_value < 70) {
            if(siuSkipFrameRate != 12) {
                siuSkipFrameRate    = 12;
                DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
            }
        } else if(monitor_value < 80) {
            if(siuSkipFrameRate != 16) {
                siuSkipFrameRate    = 16;
                DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
            }
        } else if(monitor_value < 90) {
            if(siuSkipFrameRate != 20) {
                siuSkipFrameRate    = 20;
                DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
            }
        } else if(monitor_value < 100) {
            if(siuSkipFrameRate != 24) {
                siuSkipFrameRate    = 24;
                DEBUG_ASF("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
            }
        } else if(monitor_value < 110) {
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
        
        if(pwroff == 1) {   //prepare for power off
            sysCaptureVideoStart    = 0;
          sysCaptureVideoStop     = 1;  
          break; 
    }
        
    }

#ifdef MOV_AUDIO
    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }
#endif
    while(VideoTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(VideoTrgSemEvt);
    }

    /* delay until mpeg4 and IIS task reach pend state */
    OSTimeDly(2);

#ifdef MOV_AUDIO
    // write redundance audio payload data
    while(iisCmpSemEvt->OSEventCnt > 0) {
        Output_Sem();   
        audio_value = OSSemAccept(iisCmpSemEvt);
        Output_Sem();
        if (audio_value > 0) {   
            if(audio_value_max < audio_value)
                audio_value_max = audio_value;
            if (movWriteAudimdat(pFile, &iisSounBufMng[iisSounBufMngReadIdx]) == 0) {
                DEBUG_ASF("ASF write audio payload error!!!\n");
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            }
            iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
            //OSSemPost(iisTrgSemEvt);
        }
    }
#endif  

    // write redundance video payload data
    while(VideoCmpSemEvt->OSEventCnt > 0) {
        Output_Sem();
        video_value = OSSemAccept(VideoCmpSemEvt);
        Output_Sem();
        if (video_value > 0) {   
            if(video_value_max < video_value)
                video_value_max = video_value;
            if (movWriteVidemdat(pFile, &VideoBufMng[VideoBufMngReadIdx]) == 0) {
                DEBUG_ASF("ASF write video payload error!!!\n");
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            }   
            VideoBufMngReadIdx = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
            //OSSemPost(VideoTrgSemEvt);                
        }
    }

    /* write MDAT atom post */
    if (movWriteMdatAtomPost(pFile, movFileOffset) == 00)
    {
            dcfCloseFileByIdx(pFile, 0, &tmp);
            return 0;
    }               
    
    /* write MOOV Box */
    if (movWriteMoovBox(pFile) == 0)
    {
            dcfCloseFileByIdx(pFile, 0, &tmp);
            return 0;
    }        
    
    /* close file */
    dcfCloseFileByIdx(pFile, 0, &tmp);
    DEBUG_ASF("video_value_max = %d\n", video_value_max);
#ifdef MOV_AUDIO
    DEBUG_ASF("audio_value_max = %d\n", audio_value_max);
#endif

    //DEBUG_ASF("siuLifeFrameCount = %d\n", siuLifeFrameCount);
    DEBUG_ASF("isu_avifrmcnt     = %d\n", isu_avifrmcnt);

    return 1;
}
    
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
s32 movSetVideoResolution(u16 width, u16 height)
{
    movVopWidth = width;    /* cytsai: 0418 */
    movVopHeight = height;
    //mp4VopWidth = 352;    /* cytsai: for armulator only */ 
    //mp4VopHeight = 288;   /* cytsai: for armulator only */
    
    mpeg4SetVideoResolution(width, height);

    return  1;
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
s32 movCaptureVideoStop(void)
{
    if(sysCaptureVideoStart && !sysCaptureVideoStop)
    {
    #if 0
        if(((asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && !EventTrigger) ||
        #ifdef  MOV_AUDIO
            (asfVopCount > 10 && asfAudiChunkCount > 12))
        #else
            (asfVopCount > 10))
        #endif
    #endif
        {
            sysCaptureVideoStart    = 0;
            sysCaptureVideoStop     = 1;
            DEBUG_ASF("movCaptureVideoStop() success!!!\n");
            return  1;
        }
    }
    else if(!sysCaptureVideoStart && sysCaptureVideoStop)
    {
        DEBUG_ASF("Not in video capture mode!!!\n");
        return  2;
    }
    DEBUG_ASF("movCaptureVideoStop() fail!!!\n");
    DEBUG_ASF("sysCaptureVideoStart = %d\n", sysCaptureVideoStart);
    DEBUG_ASF("sysCaptureVideoStop  = %d\n", sysCaptureVideoStop);
    DEBUG_ASF("asfCaptureMode       = %d\n", asfCaptureMode);
    DEBUG_ASF("EventTrigger         = %d\n", EventTrigger);
    DEBUG_ASF("asfVopCount          = %d\n", asfVopCount);
    DEBUG_ASF("asfAudiChunkCount    = %d\n", asfAudiChunkCount);
    return 0;
}




/* mh@2006/11/21: API for getting the visual sample location from 
                  sample number, chunk offset table, and sample size table */
/*

Routine Description:

    Get visual sample location

Arguments:

    unCurrentVisualSampleNumber - Current Visual Sample Number.
    pCurrentVisualSampleLocation - Pointer to CurrentVisualSampleLocation.

Return Value:

    0 - Failure.
    1 - Success.

*/
#ifdef READ_ENABLE
s32 mp4GetVisualSampleLocation(u32 unCurrentVisualSampleNumber, FS_i32* pCurrentVisualSampleLocation)
{
    u32 unChunkOffsetTableIndex;
    u32 unPrioriSampleCountWithinChunk;
    u32 unfirstSampleSizeTableIndexWithinChunk;
    u32 i;

    // step 1. calculate the chunk offset table index
    unChunkOffsetTableIndex = unCurrentVisualSampleNumber / MOV_VIDE_SAMPLES_PER_CHUNK;

    // step 2. calculate the priori sample count within the chunk
    unPrioriSampleCountWithinChunk = unCurrentVisualSampleNumber % MOV_VIDE_SAMPLES_PER_CHUNK;

    // step 3. calculate the first sample size table index within the chunk
    unfirstSampleSizeTableIndexWithinChunk = unChunkOffsetTableIndex * MOV_VIDE_SAMPLES_PER_CHUNK;

    // step 4. calculate the sample location
    *pCurrentVisualSampleLocation = mp4VideChunkOffsetTable[unChunkOffsetTableIndex];
    
    for (i=unfirstSampleSizeTableIndexWithinChunk; 
         i<(unfirstSampleSizeTableIndexWithinChunk+unPrioriSampleCountWithinChunk);
         i++)
         {
            *pCurrentVisualSampleLocation += mp4VideSampleSizeTable[i];
         }
    
    return  1;
}
#endif
/* mh@2006/11/21: END */

/*

Routine Description:

    Read file.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movReadFile(void)
{
#ifdef READ_ENABLE
    FS_FILE* pFile;
/* Peter: 0727 S*/
    FS_i32      FtypPosition, MoovPosition, MdatPosition;
    u32         size, i;
    s32         err;
    u8  tmp;
	
/* Peter: 0727 E*/

    FS_i32 nCurrentVisualSampleLocation; /* mh@2006/11/21 */    

    /* open file */
    if ((pFile = dcfOpen((signed char*)dcfPlaybackCurFile->pDirEnt->d_name, "r")) == NULL)
        return 0;
    
/* Peter: 0727 S*/
    /* video */
    VideoPictureIndex       = 0;
    VideoBufMngReadIdx  = 0;
    VideoBufMngWriteIdx = 0;
    mp4VopCount             = 0;
    MainVideodisplaybuf_idx     = 0;
    
#ifdef MOV_AUDIO
    /* audio */
    mp4SounSampleEntryCount = 0;
    iisSounBufMngReadIdx = 0;
    iisSounBufMngWriteIdx = 0;
#endif

    // find main atoms position: FTYP, MDAT, MOOV
    mp4FindMp4Boxes(pFile, &FtypPosition, &MoovPosition, &MdatPosition);
    
    // read FTYP atom
    mp4ReadFtypBox(pFile, FtypPosition);
    
    // read MOOV atom
    mp4ReadMoovBox(pFile, MoovPosition);
    
    for(i =0; i < mp4VopCount; i++) 
    {
        /* mh@2006/11/21: Get visual sample file location */
        mp4GetVisualSampleLocation(i, &nCurrentVisualSampleLocation); 
        /* mh@2006/11/21: END */
            
        err = FS_FSeek(pFile, nCurrentVisualSampleLocation, SEEK_SET);   // goto next LIST

        if (err) return 0;

        err = dcfRead(pFile, (u8*)VideoBufMng[0].buffer, mp4VideSampleSizeTable[i], &size);

        if(err == 0)    return 0;

        mpeg4DecodeVOP(VideoBufMng[0].buffer, mp4VideSampleSizeTable[i],0,0);

#ifdef  MOV_AUDIO
        err = FS_FSeek(pFile, mp4SounChunkOffsetTable[i], SEEK_SET);   // goto next LIST
        if(err) return 0;
        err = dcfRead(pFile, (u8*)iisSounBufMng[0].buffer, mp4SounSampleSizeTable[i], &size);
        if(err == 0)    return 0;
#endif

        //DEBUG_MP4("mp4VopWidth = %d\n", mp4VopWidth);
        //DEBUG_MP4("mp4VopHeight = %d\n", mp4VopHeight);

        isuPlayback(mpeg4outputbuf[MainVideodisplaybuf_idx],
                    MainVideodisplaybuf[MainVideodisplaybuf_idx], 
                    mp4VopWidth, 
                    mp4VopHeight);

        iduPlaybackFrame(MainVideodisplaybuf[MainVideodisplaybuf_idx]);

        MainVideodisplaybuf_idx++;

        if (MainVideodisplaybuf_idx == 3)
            MainVideodisplaybuf_idx = 0;
    }

    /* close file */            
    dcfClose(pFile, &tmp);
    
    DEBUG_MP4("MP4 file playback finish, total %d frames!!!\n", mp4VopCount);
/* Peter: 0727 E*/
#endif
    return 1;
}

/************************************************************************/
/*                           MP4 Common                                 */
/************************************************************************/

/*

Routine Description:

    Write FTYP Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] File Type Box 'ftyp' identifies:
       1. which spec is the 'best use' of the file, and a minor version
          of that spec. (major brand)
       2. a set of other specs to which the file complies. (compatible brand)  
*/
s32 movWriteFtypBox(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MOV_FTYP_BOX_T tMovFtypAtom = 
    {
        0x14000000,     /* size */
        0x70797466,     /* type = "ftyp" */
        0x20207471,     /* major_brand = "qt  " */
        0x00000000,     /* minor_version */
        0x20207471,     /* compatible_brands = "isom" "qt  " */ 
    };  
    
    tMovFtypAtom.size = bSwap32((u32)sizeof(MOV_FTYP_BOX_T)); 

    if (dcfWrite(pFile, (unsigned char*)&tMovFtypAtom, sizeof(MOV_FTYP_BOX_T), &unSize) == 0)
            return 0;
    
    return 1;
}


s32 movWriteWideBox(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MOV_WIDE_BOX_T tMovMideAtom = 
    {
        0x08000000,     /* size */
        0x65646977,     /* type = "wide" */
    };  
    

    if (dcfWrite(pFile, (unsigned char*)&tMovMideAtom, sizeof(MOV_WIDE_BOX_T), &unSize) == 0)
            return 0;
    
    return 1;
}
/*

Routine Description:

    Write MDAT Box pre.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Media Data Box 'mdat' contains the media data.

*/
s32 movWriteMdatBoxPre(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MOV_MDAT_BOX_T tMovMdatVox =
    {
        0xffffffff,     /* size = TBD */
        0x7461646d,     /* type = "mdat" */
    };
    
    if (dcfWrite(pFile, (unsigned char*)&tMovMdatVox, sizeof(MOV_MDAT_BOX_T), &unSize) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Write MDAT atom data.

Arguments:

    pFile - File handle.
    pBuf - Buffer.
    dataSize - Data size.

Return Value:

    0 - Failure.
    1 - Success.

*/


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
s32 movWriteVidemdat(FS_FILE* pFile, VIDEO_BUF_MNG* pMng)
{
    u32 size;
    u32 chunkFlag, chunkTime, chunkSize;
    u8* pChunkBuf;
    u32 payloadSize;
    u32 subPayloadSize;
    //u32 writeSize;
    u32 offsetIntoMediaObject;
    u8  streamNumber;   
    
    chunkFlag = pMng->flag;
    chunkTime = pMng->time;
    chunkSize = pMng->size;
    pChunkBuf = pMng->buffer;

       movVopCount++;

       // record Time to Sample atom entry
       if(movTimeToSampleTableEntryCount==0)
       {
            movTimeToSampleTableEntryCount++;
            movVideSampleTimeTable[movTimeToSampleTableEntryCount-1].sample_count=bSwap32(1);
            movVideSampleTimeTable[movTimeToSampleTableEntryCount-1].sample_duration=bSwap32(chunkTime);

       }
       else
       {
            if(chunkTime== movVideSampleTimeTable[movTimeToSampleTableEntryCount-1].sample_duration)
               bSwap32( movVideSampleTimeTable[movTimeToSampleTableEntryCount-1].sample_count++);
            else
            {
                movTimeToSampleTableEntryCount++;    
                movVideSampleTimeTable[movTimeToSampleTableEntryCount-1].sample_count=bSwap32(1);
                movVideSampleTimeTable[movTimeToSampleTableEntryCount-1].sample_duration=bSwap32(chunkTime);
             }
       }
       
    /* save next index entry of 1 sec boundary */
    if (chunkFlag & FLAG_I_VOP)
    {
        movVideSyncSampleNumberTable[movIVopCount] = bSwap32(movVopCount); /* data packet index */
        movIVopCount++;
    }

      // Record sample size 
      movVideSampleSizeTable[movVopCount-1]=bSwap32(chunkSize);
      //Chunk offset
      movVideChunkOffsetTable[movChunkOffsetTableEntryCount]=bSwap32(movFileOffset);    // Since we are one sample per chunk
      movChunkOffsetTableEntryCount++;

    if (dcfWrite(pFile, (unsigned char*)pChunkBuf, chunkSize, &size) == 0)
            return 0;
    
       movFileOffset+=chunkSize;
       movVidePresentTime+=chunkTime;
       
    asfVopCount++;
       
    return 1;
}




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
s32 movWriteAudimdat(FS_FILE* pFile, IIS_BUF_MNG* pMng)
{
    u32 size;
    u32 chunkFlag, chunkTime, chunkSize;
    u8* pChunkBuf;
    u32 payloadSize;
    u32 subPayloadSize;
    //u32 writeSize;
    u32 offsetIntoMediaObject;
    u8  streamNumber;   
    
    chunkFlag = pMng->flag;
    chunkTime = pMng->time;
    chunkSize = pMng->size;
    pChunkBuf = pMng->buffer;

       // record Time to Sample atom entry
      movSounSampleEntryCount++;

        
    if(chunkSize==IIS_CHUNK_SIZE)
    {
         movSounSampleChunkTable[movAudisamplesperChunkCnt-1].first_chunk=movAudisamplesperChunkCnt; 
         movSounSampleChunkTable[movAudisamplesperChunkCnt-1].samples_per_chunk=MOV_AUDI_SAMPLES_PER_CHUK;
         movSounSampleChunkTable[movAudisamplesperChunkCnt-1].sample_description_id =0x01;   
    }
    else
    {
         movAudisamplesperChunkCnt++;
         movSounSampleChunkTable[movAudisamplesperChunkCnt-1].first_chunk=movSounSampleEntryCount; 
         movSounSampleChunkTable[movAudisamplesperChunkCnt-1].samples_per_chunk=chunkSize;
         movSounSampleChunkTable[movAudisamplesperChunkCnt-1].sample_description_id =0x01;           
     }
     movSounChunkOffsetTable[movSounSampleEntryCount]=movFileOffset;   
    if (dcfWrite(pFile, (unsigned char*)pChunkBuf, chunkSize, &size) == 0)
            return 0;

       movAudiPresentTime+=chunkTime;   
       movFileOffset+=chunkSize;
         
    return 1;
}

/*

Routine Description:

    Write MDAT atom post.

Arguments:

    pFile - File handle.
    atomSize - Atom size.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteMdatAtomPost(FS_FILE* pFile, u32 atomSize)
{
    u32 offset,mdat_size_offset;
    u32 size,mdat_size;     
    offset = FS_FTell(pFile);   // record the original offset

       mdat_size_offset =   sizeof(MOV_FTYP_BOX_T) + sizeof(MOV_WIDE_BOX_T);

       FS_FSeek(pFile, mdat_size_offset, SEEK_SET);
       
       mdat_size = bSwap32((u32)(movFileOffset -mdat_size_offset)); 
       
    if (dcfWrite(pFile, (unsigned char*)&mdat_size, sizeof(mdat_size), &size) == 0) // Updated size
            return 0;

    FS_FSeek(pFile, offset, SEEK_SET);

    return 1;
}

/*

Routine Description:

    Write MOOV Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Movie Box 'moov' stores the metadata for a presentation.

*/
s32 movWriteMoovBox(FS_FILE* pFile)
{
    u32 unMovMoovBoxSize;
    u32 unSize;
    
    __align(4) MOV_MOOV_BOX_T tMovMoovBox =
    {
        0x00000000, /* size */
        0x766f6f6d, /* type = "moov" */
    };

    mpeg4EncodeVolHeader(movVideHeader, &movVideHeaderSize);   /* Peter: 0711 */
    movVideStsdAtomSize =
        sizeof(MOV_VIDE_STSD_BOX_T) +
        sizeof(MOV_VIDE_VISUAL_SAMPLE_ENTRY) +
        sizeof(MOV_VIDE_ESDS_BOX_T) +
        sizeof(MOV_VIDE_ES_DESCRIPTOR) +
        sizeof(MOV_VIDE_DECODER_CONFIG_DESCRIPTOR) +
        sizeof(MOV_VIDE_DECODER_SPECIFIC_INFO) +  movVideHeaderSize +
        sizeof(MOV_VIDE_SL_CONFIG_DESCRIPTOR);
    
    unMovMoovBoxSize =  
        sizeof(MOV_MOOV_BOX_FIXED_SIZE) +

        movVideStsdAtomSize +
        sizeof(MOV_VIDE_STTS_BOX_T) + movTimeToSampleTableEntryCount * sizeof(MOV_VIDE_SAMPLE_TIME_ENTRY) +
              sizeof(MOV_VIDE_STSC_BOX_T)  +
           sizeof(MOV_VIDE_STSZ_BOX_T) + movVopCount * sizeof(MOV_VIDE_SAMPLE_SIZE_ENTRY) +     
        sizeof(MOV_VIDE_STCO_BOX_T) + movChunkOffsetTableEntryCount * sizeof(MOV_VIDE_CHUNK_OFFSET_ENTRY) +
        sizeof(MOV_VIDE_STSS_BOX_T) + movIVopCount * sizeof(MOV_VIDE_SYNC_SAMPLE_NUMBER_ENTRY) +
        
#ifdef MOV_AUDIO
        sizeof(MOV_MOOV_BOX_AFFIXED_SIZE) +/* soun trak */

        sizeof(MOV_SOUN_STSD_ATOM) +
        sizeof(MOV_SOUN_STSZ_ATOM) +  
        sizeof(MOV_SOUN_STTS_ATOM) +
        sizeof(MOV_SOUN_STSC_ATOM) + movAudisamplesperChunkCnt*sizeof(MOV_SOUN_SAMPLE_TO_CHUNK_ENTRY) +
        sizeof(MOV_SOUN_STCO_ATOM) + movSounSampleEntryCount * sizeof(MOV_SOUN_CHUNK_OFFSET_ENTRY);
#endif          
    
    tMovMoovBox.size = bSwap32(unMovMoovBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMovMoovBox, sizeof(MOV_MOOV_BOX_T), &unSize) == 0)
            return 0;

    if (movWriteMvhdBox(pFile) == 0)
            return 0;
    
    if (movWriteVideTrakBox(pFile) == 0)
            return 0;

#ifdef MOV_AUDIO
    if (movWriteSounTrakAtom(pFile) == 0)
            return 0;
#endif

    
    return 1;
}

/*

Routine Description:

    Write MVHD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Movie Header Box 'mvhd' defines overall information which is 
       media-independent, and relevant to the entire presentation
       consider as a whole.

*/
s32 movWriteMvhdBox(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MOV_MVHD_BOX_T tMovMvhdBox =
    {
        0x6c000000,         /* size */
        0x6468766d,         /* type = "mvhd" */
        0x00000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time = 00:00:00 2004/01/01 */
        0xe8030000,         /* time_scale = 1000 time_scale_unit/sec = 1 millisec/time_scale_unit */
        0xffffffff,         /* duration = 0x???????? time_scale_unit */
        0x00000100,         /* preferred_rate = 1.0 (normal rate in 16.16) */
        0x0001,             /* preferred_volume = 1.0 (full volume in 8.8) */
        0x00, 0x00, 0x00, 0x00,     /* reserved[10] */
        0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00,
        0x00000100, 0x00000000, 0x00000000, /*           1.0 0.0 0.0 (in 16.16) */
        0x00000000, 0x00000100, 0x00000000, /* matrix_structure[9] = 0.0 1.0 0.0 (in 16.16) */
        0x00000000, 0x00000000, 0x00000100, /*           0.0 0.0 1.0 (in 2.30) */
        0x00000000,         /* pre_defined[0] */
        0x00000000,         /* pre_defined[1] */        
        0x00000000,         /* pre_defined[2] */     
        0x00000000,         /* pre_defined[3] */
        0x00000000,         /* pre_defined[4] */ 
        0x00000000,         /* pre_defined[5] */ 
        0x03000000,         /* next_track_id */
    };
    
    tMovMvhdBox.size = bSwap32(sizeof(MOV_MVHD_BOX_T));
    tMovMvhdBox.duration = bSwap32(movVidePresentTime);

    if (dcfWrite(pFile, (unsigned char*)&tMovMvhdBox, sizeof(MOV_MVHD_BOX_T), &unSize) == 0)
            return 0;

    return 1;   
}

/************************************************************************/
/*                          MP4 Video Track                             */
/************************************************************************/

/*

Routine Description:

    Write VIDE TRAK Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Video Track Box 'trak' define video track of the movie   

*/
s32 movWriteVideTrakBox(FS_FILE* pFile)
{
    u32 unMovVideTrakBoxSize;
    u32 unSize;
    
    __align(4) MOV_VIDE_TRAK_BOX_T tMovVideTrakBox = 
    {
        0x00000000,         /* size */
        0x6b617274,         /* type = "trak */
    };

    unMovVideTrakBoxSize =  sizeof(MOV_VIDE_TRAK_BOX_T) +   /* vide trak */
        sizeof(MOV_VIDE_TKHD_BOX_T) + 
/* Peter: 0727 S*/
        sizeof(MOV_VIDE_MDIA_BOX_T) +
/* Peter: 0727 E*/
        sizeof(MOV_VIDE_MDHD_BOX_T) +
        sizeof(MOV_VIDE_HDLR_BOX_T) +
        sizeof(MOV_VIDE_MINF_BOX_T) +
        sizeof(MOV_VIDE_VMHD_BOX_T) +
        sizeof(MOV_VIDE_HDLR_BOX_T) +
        sizeof(MOV_VIDE_DINF_BOX_T) +
        sizeof(MOV_VIDE_DREF_BOX_T) +
        sizeof(MOV_VIDE_STBL_BOX_T) +
        movVideStsdAtomSize +
        sizeof(MOV_VIDE_STTS_BOX_T) + movTimeToSampleTableEntryCount * sizeof(MOV_VIDE_SAMPLE_TIME_ENTRY) +
              sizeof(MOV_VIDE_STSC_BOX_T)  +
           sizeof(MOV_VIDE_STSZ_BOX_T) + movVopCount * sizeof(MOV_VIDE_SAMPLE_SIZE_ENTRY) +     
        sizeof(MOV_VIDE_STCO_BOX_T) + movChunkOffsetTableEntryCount * sizeof(MOV_VIDE_CHUNK_OFFSET_ENTRY) +
        sizeof(MOV_VIDE_STSS_BOX_T) + movIVopCount * sizeof(MOV_VIDE_SYNC_SAMPLE_NUMBER_ENTRY);

    tMovVideTrakBox.size = bSwap32(unMovVideTrakBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideTrakBox, sizeof(MOV_VIDE_TRAK_BOX_T), &unSize) == 0)
            return 0;

    if (movWriteVideTkhdBox(pFile) == 0)
            return 0;
    
/* Peter: 0727 S*/
    if (movWriteVideMdiaBox(pFile) == 0)
/* Peter: 0727 E*/
            return 0;

    return 1;
}

/*

Routine Description:

    Write VIDE TKHD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Video Track Header Box specifies the characteristics of video track.

*/
s32 movWriteVideTkhdBox(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MOV_VIDE_TKHD_BOX_T tMovVideTkhdBox = 
    {
        0x5c000000,         /* size */
        0x64686b74,         /* type = "tkhd" */
        0x0f000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time= 00:00:00 2004/01/01 */
        0x01000000,         /* track_id */
        0x00000000,         /* reserved1 */
        0x00000000,         /* duration = 0x???????? time_scale_unit */
        0x00, 0x00, 0x00, 0x00,     /* reserved2[8] */
        0x00, 0x00, 0x00, 0x00, 
        0x0000,             /* layer = spatial priority for overlay */
        0x0000,             /* alternate_group = group of movie tracks for QoS choice */
        0x0000,             /* volume = 0.0 */
        0x0000,             /* reserved3 */
        0x00000100, 0x00000000, 0x00000000, /*           1.0 0.0 0.0 */
        0x00000000, 0x00000100, 0x00000000, /* matrix_structure[9] = 0.0 0.0 0.0 */
        0x00000000, 0x00000000, 0x00000100, /*           0.0 0.0 1.0 */
        0x00000000,         /* track_width = 0x???????? */
        0x00000000,         /* track_height = 0x???????? */
    };

    tMovVideTkhdBox.size = bSwap32(sizeof(MOV_VIDE_TKHD_BOX_T));

    tMovVideTkhdBox.duration = bSwap32(movVidePresentTime);

    tMovVideTkhdBox.track_width = bSwap32((u32)movVopWidth << 16);

    tMovVideTkhdBox.track_height = bSwap32((u32)movVopHeight << 16);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideTkhdBox, sizeof(MOV_VIDE_TKHD_BOX_T), &unSize) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write VIDE MDIA Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] The Media Box 'mdia' contains all the objects that declare information
       about the media data within a track

*/
/* Peter: 0727 S*/
s32 movWriteVideMdiaBox(FS_FILE* pFile)
/* Peter: 0727 E*/
{
    u32 unMovVideMdiaBoxSize;
    u32 unSize;
/* Peter: 0727 S*/

    __align(4) MOV_VIDE_MDIA_BOX_T tMovVideMdiaBox =
    {
        0x00000000,         /* size */
        0x6169646d,         /* type = "mdia" */
    };

    unMovVideMdiaBoxSize =  sizeof(MOV_VIDE_MDIA_BOX_T) +

        sizeof(MOV_VIDE_MDHD_BOX_T) +
        sizeof(MOV_VIDE_HDLR_BOX_T) +
        sizeof(MOV_VIDE_MINF_BOX_T) +
        sizeof(MOV_VIDE_VMHD_BOX_T) +
        sizeof(MOV_VIDE_HDLR_BOX_T) +
        sizeof(MOV_VIDE_DINF_BOX_T) +
        sizeof(MOV_VIDE_DREF_BOX_T) +
        sizeof(MOV_VIDE_STBL_BOX_T) +
        movVideStsdAtomSize +
        sizeof(MOV_VIDE_STTS_BOX_T) + movTimeToSampleTableEntryCount * sizeof(MOV_VIDE_SAMPLE_TIME_ENTRY) +
              sizeof(MOV_VIDE_STSC_BOX_T)  +
           sizeof(MOV_VIDE_STSZ_BOX_T) + movVopCount * sizeof(MOV_VIDE_SAMPLE_SIZE_ENTRY) +     
        sizeof(MOV_VIDE_STCO_BOX_T) + movChunkOffsetTableEntryCount * sizeof(MOV_VIDE_CHUNK_OFFSET_ENTRY) +
        sizeof(MOV_VIDE_STSS_BOX_T) + movIVopCount * sizeof(MOV_VIDE_SYNC_SAMPLE_NUMBER_ENTRY);

    tMovVideMdiaBox.size = bSwap32(unMovVideMdiaBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideMdiaBox, sizeof(MOV_VIDE_MDIA_BOX_T), &unSize) == 0)
            return 0;
/* Peter: 0727 E*/

    if (movWriteVideMdhdBox(pFile) == 0)
            return 0;

    if (movWriteVideHdlrBox(pFile) == 0)
            return 0;

    if (movWriteVideMinfBox(pFile) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write VIDE MDHD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Media Header 'mdhd' declares overall information that is 
       media-independent, and relevant to characteristics of the 
       media in a track

*/
s32 movWriteVideMdhdBox(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MOV_VIDE_MDHD_BOX_T tMovVideMdhdBox = 
    {
        0x20000000,         /* size */
        0x6468646d,         /* type = "mdhd" */
        0x00000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time= 00:00:00 2004/01/01 */
        0xe8030000,         /* time_scale = 1000 time_scale_unit/sec = 1 millisec/time_scale_unit */
        0x00000000,         /* duration = 0x???????? time_scale_unit = 0x???????? millisec */
        0x0000,             /* language */
        0x0000,             /* pre_defined */
    };

    tMovVideMdhdBox.size = bSwap32(sizeof(MOV_VIDE_MDHD_BOX_T));

    tMovVideMdhdBox.duration = bSwap32(movVidePresentTime);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideMdhdBox, sizeof(MOV_VIDE_MDHD_BOX_T), &unSize) == 0)
            return 0;   

    return 1;
}

/*

Routine Description:

    Write VIDE HDLR Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Handler Reference Box 'hdlr' declares the handler type of the media

*/
s32 movWriteVideHdlrBox(FS_FILE* pFile)
{
    u32 unSize;

    __align(4) MOV_VIDE_HDLR_BOX_T tMovVideHdlrBox =
    {
        0x2d000000,             /* size */
        0x726c6468,             /* type = "hdlr" */
        0x00000000,             /* version_flags */
        0x726c686d,             /* component type = "mhlr"*/
        0x65646976,             /* component sub type = "vide" (video) */
        0x00000000,             /* component manufacture */
        0x00000000,             /* component flags */
        0x00000000,             /* component flags mask */
        0x56, 0x69, 0x73, 0x75, /* name[13] = "VisualStream\0" */
        0x61, 0x6C, 0x53, 0x74,
        0x72, 0x65, 0x61, 0x6D,
        0x00, 
    };

    tMovVideHdlrBox.size = bSwap32(sizeof(MOV_VIDE_HDLR_BOX_T));

    if (dcfWrite(pFile, (unsigned char*)&tMovVideHdlrBox, sizeof(MOV_VIDE_HDLR_BOX_T), &unSize) == 0)
            return 0;

    return 1;
}


s32 movWriteminfVideHdlrBox(FS_FILE* pFile)
{
    u32 unSize;

    __align(4) MOV_VIDE_HDLR_BOX_T tMovVideHdlrBox =
    {
        0x2d000000,             /* size */
        0x726c6468,             /* type = "hdlr" */
        0x00000000,             /* version_flags */
        0x726c6864,             /* component type = "dhlr"*/
        0x206c7275,             /* component sub type = "url  */
        0x00000000,             /* component manufacture */
        0x00000000,             /* component flags */
        0x00000000,             /* component flags mask */
        0x56, 0x69, 0x73, 0x75, /* name[13] = "VisualStream\0" */
        0x61, 0x6C, 0x53, 0x74,
        0x72, 0x65, 0x61, 0x6D,
        0x00, 
    };

    tMovVideHdlrBox.size = bSwap32(sizeof(MOV_VIDE_HDLR_BOX_T));

    if (dcfWrite(pFile, (unsigned char*)&tMovVideHdlrBox, sizeof(MOV_VIDE_HDLR_BOX_T), &unSize) == 0)
            return 0;

    return 1;
}
/*

Routine Description:

    Write VIDE MINF Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Media Information Box 'minf' contains all the objects that declare
       characteristic information of the media in the track  

*/

s32 movWriteVideMinfBox(FS_FILE* pFile)
{
    u32 unMovVideMinfBoxSize;
    u32 unSize;
    
    __align(4) MOV_VIDE_MINF_BOX_T tMovVideMinfBox =
    {
        0x00000000,     /* size */
        0x666e696d,     /* type = "minf" */
    };

    unMovVideMinfBoxSize =  sizeof(MOV_VIDE_MINF_BOX_T) +
            sizeof(MOV_VIDE_VMHD_BOX_T) +
            sizeof(MOV_VIDE_HDLR_BOX_T) +
            sizeof(MOV_VIDE_DINF_BOX_T) +
            sizeof(MOV_VIDE_DREF_BOX_T) +
            sizeof(MOV_VIDE_STBL_BOX_T) +
            movVideStsdAtomSize +
            sizeof(MOV_VIDE_STTS_BOX_T) + movTimeToSampleTableEntryCount * sizeof(MOV_VIDE_SAMPLE_TIME_ENTRY) +
                  sizeof(MOV_VIDE_STSC_BOX_T)  +
               sizeof(MOV_VIDE_STSZ_BOX_T) + movVopCount * sizeof(MOV_VIDE_SAMPLE_SIZE_ENTRY) +     
            sizeof(MOV_VIDE_STCO_BOX_T) + movChunkOffsetTableEntryCount * sizeof(MOV_VIDE_CHUNK_OFFSET_ENTRY) +
            sizeof(MOV_VIDE_STSS_BOX_T) + movIVopCount * sizeof(MOV_VIDE_SYNC_SAMPLE_NUMBER_ENTRY);

    tMovVideMinfBox.size = bSwap32(unMovVideMinfBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideMinfBox, sizeof(MOV_VIDE_MINF_BOX_T), &unSize) == 0)
            return 0;

       if (movWriteVideVmhdBox(pFile) == 0)
            return 0;
       
       if (movWriteminfVideHdlrBox(pFile) == 0)
            return 0;       

    if (movWriteVideDinfBox(pFile) == 0)
            return 0;

    if (movWriteVideStblBox(pFile) == 0)
            return 0;
    

    return 1;
}

/*

Routine Description:

    Write VIDE VMHD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Video Media Header 'vmhd' contains general presentation information
       for video media 
*/
s32 movWriteVideVmhdBox(FS_FILE* pFile) 
{
    u32 unSize;
    
    __align(4) MOV_VIDE_VMHD_BOX_T tMovVideVmhdBox =
    {
        0x14000000,         /* size */
        0x64686d76,         /* type = "vmhd" */
        0x01000000,         /* version_flags = no lean ahead */
        0x0000,             /* graphics_mode = copy */
        0x0000,             /* opcolor[3] */ 
        0x0000,
        0x0000,
    };

    tMovVideVmhdBox.size = bSwap32(sizeof(MOV_VIDE_VMHD_BOX_T));

    if (dcfWrite(pFile, (unsigned char*)&tMovVideVmhdBox, sizeof(MOV_VIDE_VMHD_BOX_T), &unSize) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write VIDE DINF Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Data Information Box 'dinf' contains object that declare the 
       location of the media information in a track.

*/
s32 movWriteVideDinfBox(FS_FILE* pFile)
{
    u32 unMovVideDinfBoxSize;
    u32 unSize;

    __align(4) MOV_VIDE_DINF_BOX_T tMovVideDinfBox =
    {
        0x24000000,         /* size */
        0x666e6964,         /* type = "dinf" */
    };

    unMovVideDinfBoxSize =  sizeof(MOV_VIDE_DINF_BOX_T) +
                            sizeof(MOV_VIDE_DREF_BOX_T);
    
    tMovVideDinfBox.size = bSwap32(unMovVideDinfBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideDinfBox, sizeof(MOV_VIDE_DINF_BOX_T), &unSize) == 0)
            return 0;

    if (movWriteVideDrefBox(pFile) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Write VIDE DREF Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Data Reference Box 'dref' declares the location of the media data

*/
s32 movWriteVideDrefBox(FS_FILE* pFile)
{
    u32 unSize;

    __align(4) MOV_VIDE_DREF_BOX_T tMovVideDrefBox =
    {
        0x1c000000,         /* size */
        0x66657264,         /* type = "dref" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x0c000000,     /* size */
            0x206c7275,     /* type = "url " */
            0x01000000,     /* version_flags = 0x00000001 (self reference) */ 
        },
    };

    tMovVideDrefBox.size = bSwap32(sizeof(MOV_VIDE_DREF_BOX_T));
    
    if (dcfWrite(pFile, (unsigned char*)&tMovVideDrefBox, sizeof(MOV_VIDE_DREF_BOX_T), &unSize) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write VIDE STBL Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Sample Table Box 'stbl' contains all the time and data indexing
       of the media samples in a track 

*/
s32 movWriteVideStblBox(FS_FILE* pFile)
{
    u32 unMovVideStblBoxSize;
    u32 unSize;

    __align(4) MOV_VIDE_STBL_BOX_T tMovVideStblBox =
    {
        0x00000000,     /* size */
        0x6c627473,     /* type = "stbl" */
    };

    unMovVideStblBoxSize =  sizeof(MOV_VIDE_STBL_BOX_T) +
       movVideStsdAtomSize +
       sizeof(MOV_VIDE_STTS_BOX_T) + movTimeToSampleTableEntryCount * sizeof(MOV_VIDE_SAMPLE_TIME_ENTRY) +
       sizeof(MOV_VIDE_STSC_BOX_T)  +
       sizeof(MOV_VIDE_STSZ_BOX_T) + movVopCount * sizeof(MOV_VIDE_SAMPLE_SIZE_ENTRY) +     
       sizeof(MOV_VIDE_STCO_BOX_T) + movChunkOffsetTableEntryCount * sizeof(MOV_VIDE_CHUNK_OFFSET_ENTRY) +
       sizeof(MOV_VIDE_STSS_BOX_T) + movIVopCount * sizeof(MOV_VIDE_SYNC_SAMPLE_NUMBER_ENTRY);

    tMovVideStblBox.size = bSwap32(unMovVideStblBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideStblBox, sizeof(MOV_VIDE_STBL_BOX_T), &unSize) == 0)
            return 0;

    if (movWriteVideStsdBox(pFile) == 0)
            return 0;

    if (movWriteVideSttsBox(pFile) == 0)
            return 0;

    if (movWriteVideStssBox(pFile) == 0)
            return 0;
    
       if (movWriteVideStscBox(pFile) == 0)
            return 0;

    if (movWriteVideStszBox(pFile) == 0)
            return 0;
        
    if (movWriteVideStcoBox(pFile) == 0)
            return 0;


    
    return 1;
}

/*

Routine Description:

    Write VIDE STSD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Sample Description Box 'stsd' gives detailed information about
       the coding type used, and any initialization information needed
       for that coding

*/
s32 movWriteVideStsdBox(FS_FILE* pFile)
{
    u32 size;
    
    __align(4) MOV_VIDE_STSD_BOX_T tMovVideStsdBox =
    {
        0xaa000000,         /* size */
        0x64737473,         /* type = "stsd" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        /* mp4VideVisualSampleEntry */
    };
    
    __align(4) MOV_VIDE_VISUAL_SAMPLE_ENTRY movVideVisualSampleEntry =
    {
            0x9a000000,         /* sample_description_size */
            0x7634706d,         /* data_format = "mp4v" */
            0x00, 0x00, 0x00, 0x00,     /* reserved[6] */       
            0x00, 0x00,
            0x0100,             /* data_reference_index = data_reference of this track */
            0x00000000,             /* Version Revision level */
            0x5352414D,         /* Vendor  "MARS"*/
            0x00020000,         /* Temporal quality */
            0x00020000,         /* Spatial quality */
            0x00000000,         /* width_height = 0x???????? */
            0x00004800,         /* horz_res = 72 dpi */
            0x00004800,         /* vert_res = 72 dpi */
            0x00000000,         /* Data Size */
            0x0100,             /* frame_count */
            0x00, 0x00, 0x00, 0x00,     /* compressor_name[32] */
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x1800,             /* video_depth */
            0xffff,             /* pre_defined3 */
            /* tMp4VideEsdsBox */
    };

    __align(4) MOV_VIDE_ESDS_BOX_T tMovVideEsdsBox =
    {
                0x44000000,         /* size */
                0x73647365,         /* type = "esds" */
                0x00000000,         /* version_flags */
                /* mp4VideEsDescriptor */
                /* mp4VideDecoderConfigDescriptor */
                /* mp4VideDecoderSpecificInfo */
                /* mp4VideSlConfigDescriptor */
    };

    __align(4) MOV_VIDE_ES_DESCRIPTOR movVideEsDescriptor =
    {
                    0x03,               /* ES_DescrTag (0x03) */
                    0x36,               /* Total length from ES_ID to the
                                           end of ES_Descriptor */
                    0x0100,             /* ES_ID = 3 */
                    0x00,               /* streamDependenceFlag (bit7) = 0
                                           URL_flag (bit6) = 0
                                           OCRstreamFlag (bit5) = 0
                                           streamPriority (bit4-bit0) = 0 */
                    /* mp4VideDecoderConfigDescriptor */
                    /* mp4VideSlConfigDescriptor */
                    
    };

    /* DecoderConfigDescriptor: provides information about the decoder type and 
                                the required decoder resources needed for 
                                the associated elementary stream */    
    __align(4) MOV_VIDE_DECODER_CONFIG_DESCRIPTOR movVideDecoderConfigDescriptor =
    {                                   
                        0x04,           /* DecoderConfigDescrTag (0x04) */
                        0x2e,           /* Total length from objectTypeIndication
                                           to the end of DecoderConfigDescriptor */
                        0x20,           /* objectTypeIndication = 0x20
                                           ==> Visual ISO 14496-2 */
                        0x00000011,     /* streamType (bit7-bit2) = 4 
                                           ==> Visual Stream, 
                                           upStream (bit1) = 0
                                           ==> not upstream,
                                           reserved (bit0) = 1,
                                           bufferSizeDB (byte3-byte0):
                                           Size of decoding buffer */
                        0x00350c00,     /* dcd_tag_max_bitrate */
                        0x00000000,     /* dcd_tag_avg_bitrate */
                        /* mp4VideDecoderSpecificInfo */
    };

    __align(4) MOV_VIDE_DECODER_SPECIFIC_INFO movVideDecoderSpecificInfo = 
    {
                            0x05,               /* DecSpecificInfoTag (0x05) */
                            0x1f,               /* Total length from next byte
                                                   to the end of DecoderSpecificInfo */
                            0x00, 0x00, 0x01, 0xb0, /* visual_object_sequence_start_code */
                            0x03,                   /* profile_and_level_indication = 0x03
                                                       ==> Simple Profile/Level 3 */
                            0x00, 0x00, 0x01, 0xb5, /* visual_object_start_code */
                            0x09,                   /* [] is_visual_object_identifier (bit7) = 0
                                                          ==> no version identification or 
                                                              priority needs to be specified, 
                                                       [] visual_object_type (bit6-bit3) = 0001
                                                          ==> video ID,                                                
                                                       [] video_signal_type (bit2) = 0,
                                                       [] next_start_code: add 01 */
                                                   
    };

    __align(4) MOV_VIDE_SL_CONFIG_DESCRIPTOR movVideSlConfigDescriptor =
    {
                        0x06,               /* slcd_tag */
                        0x01,               /* slcd_tag_length */
                        0x02,               /* slcd_tag_data */
    };

    movVideVisualSampleEntry.width_height = bSwap32(((u32)movVopWidth << 16) | ((u32)movVopHeight));
    
    if (dcfWrite(pFile, (unsigned char*)&tMovVideStsdBox, sizeof(MOV_VIDE_STSD_BOX_T), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&movVideVisualSampleEntry, sizeof(MOV_VIDE_VISUAL_SAMPLE_ENTRY), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&tMovVideEsdsBox, sizeof(MOV_VIDE_ESDS_BOX_T), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&movVideEsDescriptor, sizeof(MOV_VIDE_ES_DESCRIPTOR), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&movVideDecoderConfigDescriptor, sizeof(MOV_VIDE_DECODER_CONFIG_DESCRIPTOR), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&movVideDecoderSpecificInfo, sizeof(MOV_VIDE_DECODER_SPECIFIC_INFO), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&movVideHeader, movVideHeaderSize, &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&movVideSlConfigDescriptor, sizeof(MOV_VIDE_SL_CONFIG_DESCRIPTOR), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write VIDE STTS Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Decoding Time to Sample Box 'stts' contains a compact version of 
       a table that allows indexing from decoding time to sample number.

*/
s32 movWriteVideSttsBox(FS_FILE* pFile)
{
    u32 unMovVideSttsBoxSize;
    u32 size;

    __align(4) MOV_VIDE_STTS_BOX_T tMovVideSttsBox =
    {
        0x18000000,         /* size */
        0x73747473,         /* type = "stts" */
        0x00000000,         /* version_flags */
        0x00000000,         /* number_of_entries */
    };

    unMovVideSttsBoxSize =  sizeof(MOV_VIDE_STTS_BOX_T) + movTimeToSampleTableEntryCount * sizeof(MOV_VIDE_SAMPLE_TIME_ENTRY);

    tMovVideSttsBox.size = bSwap32(unMovVideSttsBoxSize);
    tMovVideSttsBox.number_of_entries = bSwap32(movTimeToSampleTableEntryCount);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideSttsBox, sizeof(MOV_VIDE_STTS_BOX_T), &size) == 0)
            return 0;

      if (dcfWrite(pFile, (unsigned char*)movVideSampleTimeTable, movTimeToSampleTableEntryCount * sizeof(MOV_VIDE_SAMPLE_TIME_ENTRY), &size) == 0)
                return 0;
    return 1;
}

/*

Routine Description:

    Write VIDE STSC Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Sample to Chunk Box 'stsc' can be used to to find the chunk that
       contain the sample

*/
s32 movWriteVideStscBox(FS_FILE* pFile)
{
    u32 size;
        
    __align(4) MOV_VIDE_STSC_BOX_T tMovVideStscBox =
    {
        0x1c000000,         /* size */
        0x63737473,         /* type = "stsc" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */ 
           {
                0x01000000,             // first chunk
                0x01000000,             // Sample per chunk
                0x01000000,             //chunk id
        },
    };

      size = sizeof(MOV_VIDE_STSC_BOX_T);

    tMovVideStscBox.size = bSwap32(size);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideStscBox, sizeof(MOV_VIDE_STSC_BOX_T), &size) == 0)
            return 0;
    
    return 1;
}


/*

Routine Description:

    Write VIDE STSZ Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Sample Size Box 'stsz' contains the sample size infomration

*/
s32 movWriteVideStszBox(FS_FILE* pFile)
{
    u32 size;
    
    __align(4) MOV_VIDE_STSZ_BOX_T tMovVideStszBox =
    {
        0x00000000,         /* size */
        0x7a737473,         /* type = "stsz" */
        0x00000000,         /* version_flags */
        0x00000000,         /* sample_size */
        0x00000000,         /* number_of_entries = 0x???????? VOP count */
    };

    size = sizeof(MOV_VIDE_STSZ_BOX_T) + movVopCount * sizeof(MOV_VIDE_SAMPLE_SIZE_ENTRY);
    tMovVideStszBox.size = bSwap32(size);
    tMovVideStszBox.number_of_entries = bSwap32(movVopCount);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideStszBox, sizeof(MOV_VIDE_STSZ_BOX_T), &size) == 0)
            return 0;

       if (dcfWrite(pFile, (unsigned char*)movVideSampleSizeTable, movVopCount * sizeof(MOV_VIDE_SAMPLE_SIZE_ENTRY), &size) == 0)
            return 0;

    return 1;
}


/*

Routine Description:

    Write VIDE STCO Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Chunk Offset Box 'stco' gives the file offset of each chunk. 

*/
s32 movWriteVideStcoBox(FS_FILE* pFile)
{
    u32 size;

    __align(4) MOV_VIDE_STCO_BOX_T tMovVideStcoBox =
    {
        0x00000000,         /* size */
        0x6f637473,         /* type = "stco" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
    };

    size = sizeof(MOV_VIDE_STCO_BOX_T) + movChunkOffsetTableEntryCount * sizeof(MOV_VIDE_CHUNK_OFFSET_ENTRY);
    tMovVideStcoBox.size = bSwap32(size);
    tMovVideStcoBox.number_of_entries = bSwap32(movChunkOffsetTableEntryCount);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideStcoBox, sizeof(MOV_VIDE_STCO_BOX_T), &size) == 0)
            return 0;

      if (dcfWrite(pFile, (unsigned char*)movVideChunkOffsetTable, movChunkOffsetTableEntryCount * sizeof(MOV_VIDE_CHUNK_OFFSET_ENTRY), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write VIDE STSS Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteVideStssBox(FS_FILE* pFile)
{
    u32 size;

    __align(4) MOV_VIDE_STSS_BOX_T tMovVideStssBox =
    {
        0x00000000,         /* size */
        0x73737473,         /* type = "stss" */
        0x00000000,         /* version_flags */
        0x00000000,         /* number_of_entries = 0x???????? I-VOP count */
    };

    size =  sizeof(MOV_VIDE_STSS_BOX_T) + movIVopCount * sizeof(MOV_VIDE_SYNC_SAMPLE_NUMBER_ENTRY);
    tMovVideStssBox.size = bSwap32(size);
    tMovVideStssBox.number_of_entries = bSwap32(movIVopCount);

    if (dcfWrite(pFile, (unsigned char*)&tMovVideStssBox, sizeof(MOV_VIDE_STSS_BOX_T), &size) == 0)
            return 0;

       if (dcfWrite(pFile, (unsigned char*)movVideSyncSampleNumberTable, movIVopCount * sizeof(MOV_VIDE_SYNC_SAMPLE_NUMBER_ENTRY), &size) == 0)
          return 0;

    return 1;
}

/************************************************************************/
/* MP4 Soun Track                           */
/************************************************************************/

#ifdef MOV_AUDIO

/*

Routine Description:

    MP4 initialize audio.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movAudioInit(void)
{
    /* audio format */
    movAudioFormat.number_of_channels   = 0x0001;       /* stereo */
    movAudioFormat.sample_size          = 0x0008;       /* 8 bit per sample */
    movAudioFormat.sample_rate          = 0x00001f40;   /* 8000 Hz */

    /* audio fragment */
    movSounSampleEntryDuration          = 0x000003E8;   /* sample per sample_entry */           

    return 1;
}
    
/*

Routine Description:

    Write SOUN TRAK atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounTrakAtom(FS_FILE* pFile)
{
    u32 size;
    MOV_SOUN_TRAK_ATOM movSounTrakAtom = 
    {
        0x00000000,         /* size */
        0x6b617274,         /* type = "trak" */
    };

    size =  sizeof(MOV_SOUN_TRAK_ATOM) +
        sizeof(MOV_SOUN_TKHD_ATOM) + 
        sizeof(MOV_SOUN_MDIA_ATOM) +
        sizeof(MOV_SOUN_MDHD_ATOM) +
        sizeof(MOV_SOUN_HDLR_ATOM) +
        sizeof(MOV_SOUN_MINF_ATOM) +
        sizeof(MOV_SOUN_SMHD_ATOM) +
        sizeof(MOV_SOUN_HDLR_ATOM) +
        sizeof(MOV_SOUN_DINF_ATOM) +
        sizeof(MOV_SOUN_DREF_ATOM) +
        sizeof(MOV_SOUN_STBL_ATOM) +
        sizeof(MOV_SOUN_STSD_ATOM) +
        sizeof(MOV_SOUN_STSZ_ATOM) +  
        sizeof(MOV_SOUN_STTS_ATOM) +
        sizeof(MOV_SOUN_STSC_ATOM) + movAudisamplesperChunkCnt*sizeof(MOV_SOUN_SAMPLE_TO_CHUNK_ENTRY) +
        sizeof(MOV_SOUN_STCO_ATOM) + movSounSampleEntryCount * sizeof(MOV_SOUN_CHUNK_OFFSET_ENTRY);
    
    movSounTrakAtom.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&movSounTrakAtom, sizeof(MOV_SOUN_TRAK_ATOM), &size) == 0)
            return 0;

    if (movWriteSounTkhdAtom(pFile) == 0)
            return 0;
/* Peter: 0727 S*/
    if (movWriteSounMdiaAtom(pFile) == 0)
/* Peter: 0727 E*/
            return 0;

    return 1;
}

/*

Routine Description:

    Write SOUN TKHD atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounTkhdAtom(FS_FILE* pFile)
{
    u32 size, duration;
    __align(4) MOV_SOUN_TKHD_ATOM movSounTkhdAtom = 
    {
        0x5c000000,         /* size */
        0x64686b74,         /* type = "tkhd" */
        0x0f000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time= 00:00:00 2004/01/01 */
        0x02000000,         /* track_id */
        0x00000000,         /* reserved1 */
        0x00000000,         /* duration = 0x???????? time_scale_unit */
        0x00, 0x00, 0x00, 0x00,     /* reserved2[8] */
        0x00, 0x00, 0x00, 0x00, 
        0x0000,             /* layer = spatial priority for overlay */
        0x0000,             /* alternate_group = group of movie tracks for QoS choice */
        0x0001,             /* volume = 1.0 */
        0x0000,             /* reserved3 */
        0x00000100, 0x00000000, 0x00000000, /*           1.0 0.0 0.0 */
            0x00000000, 0x00000100, 0x00000000, /* matrix_structure[9] = 0.0 0.0 0.0 */
            0x00000000, 0x00000000, 0x00000100, /*           0.0 0.0 1.0 */
        0x00000000,     /* track_width = 0x00000000 */
            0x00000000,     /* track_height = 0x00000000 */
    };
    
    duration = movAudiPresentTime; /* millisec per total */

    movSounTkhdAtom.size = bSwap32(sizeof(MOV_SOUN_TKHD_ATOM));
    movSounTkhdAtom.duration = bSwap32(duration);
    if (dcfWrite(pFile, (unsigned char*)&movSounTkhdAtom, sizeof(MOV_SOUN_TKHD_ATOM), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SOUN MDIA atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounMdiaAtom(FS_FILE* pFile)
{
    u32 size;
/* Peter: 0727 S*/
    __align(4) MOV_SOUN_MDIA_ATOM movSounMdiaAtom =
    {
        0x00000000,         /* size */
        0x6169646d,         /* type = "mdia" */
    };

    size =  sizeof(MOV_SOUN_MDIA_ATOM) +
        sizeof(MOV_SOUN_MDHD_ATOM) +
        sizeof(MOV_SOUN_HDLR_ATOM) +
        sizeof(MOV_SOUN_MINF_ATOM) +
        sizeof(MOV_SOUN_SMHD_ATOM) +
        sizeof(MOV_SOUN_HDLR_ATOM) +
        sizeof(MOV_SOUN_DINF_ATOM) +
        sizeof(MOV_SOUN_DREF_ATOM) +
        sizeof(MOV_SOUN_STBL_ATOM) +
        sizeof(MOV_SOUN_STSD_ATOM) +
        sizeof(MOV_SOUN_STSZ_ATOM) + 
        sizeof(MOV_SOUN_STTS_ATOM) +
        sizeof(MOV_SOUN_STSC_ATOM) + movAudisamplesperChunkCnt*sizeof(MOV_SOUN_SAMPLE_TO_CHUNK_ENTRY) +
        sizeof(MOV_SOUN_STCO_ATOM) + movSounSampleEntryCount * sizeof(MOV_SOUN_CHUNK_OFFSET_ENTRY);
        
    movSounMdiaAtom.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&movSounMdiaAtom, sizeof(MOV_SOUN_MDIA_ATOM), &size) == 0)
            return 0;
/* Peter: 0727 E*/

    if (movWriteSounMdhdAtom(pFile) == 0)
            return 0;
    if (movWriteSounHdlrAtom(pFile) == 0)
            return 0;
    if (movWriteSounMinfAtom(pFile) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SOUN MDHD atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounMdhdAtom(FS_FILE* pFile)
{
    u32 size, sampleCount;
    __align(4) MOV_SOUN_MDHD_ATOM movSounMdhdAtom = 
    {
        0x20000000,         /* size */
        0x6468646d,         /* type = "mdhd" */
        0x00000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time= 00:00:00 2004/01/01 */
        0x00000000,         /* time_scale = 0x???????? time_scale_uint/sec = frameRate time_scale_unit/sec */
        0x00000000,         /* duration = 0x???????? time_scale_unit = vopCount time_scale_unit */
        0x0000,             /* language */
        0x0000,             /* quality */
    };

       sampleCount=(movSounSampleChunkTable[1].first_chunk-movSounSampleChunkTable[0].first_chunk) * movSounSampleChunkTable[0].samples_per_chunk;
       sampleCount += movSounSampleChunkTable[1].first_chunk;   
    
    movSounMdhdAtom.size = bSwap32(sizeof(MOV_SOUN_MDHD_ATOM));
    movSounMdhdAtom.time_scale = bSwap32((u32) movAudioFormat.sample_rate);
    movSounMdhdAtom.duration = bSwap32(sampleCount);
    if (dcfWrite(pFile, (unsigned char*)&movSounMdhdAtom, sizeof(MOV_SOUN_MDHD_ATOM), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SOUN HDLR atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounHdlrAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MOV_SOUN_HDLR_ATOM movSounHdlrAtom =
    {
        0x2c000000,         /* size */
        0x726c6468,         /* type = "hdlr" */
        0x00000000,         /* version_flags */
        0x726c686d,         /* component_type */
        0x6e756f73,         /* component_subtype = "soun" (sound) */
        0x00000000,         /* component_manufacturer */
        0x00000000,         /* component_flags */
        0x00000000,         /* component_flags_mask */
        0x41, 0x75, 0x64, 0x69,     /* component_name[0x0D] = "AudioStream\0" */
        0x6F, 0x53, 0x74, 0x72, 
        0x65, 0x61, 0x6D, 0x00, 
    };

    movSounHdlrAtom.size = bSwap32(sizeof(MOV_SOUN_HDLR_ATOM));
    if (dcfWrite(pFile, (unsigned char*)&movSounHdlrAtom, sizeof(MOV_SOUN_HDLR_ATOM), &size) == 0)
            return 0;

    return 1;
}


s32 movWriteSounminfHdlrAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MOV_SOUN_HDLR_ATOM movSounHdlrAtom =
    {
        0x2c000000,         /* size */
        0x726c6468,         /* type = "hdlr" */
        0x00000000,         /* version_flags */
        0x726c6864,             /* component type = "dhlr"*/
        0x206c7275,             /* component sub type = "url  */
        0x00000000,         /* component_manufacturer */
        0x00000000,         /* component_flags */
        0x00000000,         /* component_flags_mask */
        0x41, 0x75, 0x64, 0x69,     /* component_name[0x0D] = "AudioStream\0" */
        0x6F, 0x53, 0x74, 0x72, 
        0x65, 0x61, 0x6D, 0x00, 
    };

    movSounHdlrAtom.size = bSwap32(sizeof(MOV_SOUN_HDLR_ATOM));
    if (dcfWrite(pFile, (unsigned char*)&movSounHdlrAtom, sizeof(MOV_SOUN_HDLR_ATOM), &size) == 0)
            return 0;

    return 1;
}
/*

Routine Description:

    Write SOUN MINF atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounMinfAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MOV_SOUN_MINF_ATOM movSounMinfAtom =
    {
        0x00000000,         /* size */
        0x666e696d,         /* type = "minf" */
    };

    size =  sizeof(MOV_SOUN_MINF_ATOM) +
        sizeof(MOV_SOUN_SMHD_ATOM) +
        sizeof(MOV_SOUN_HDLR_ATOM) +
        sizeof(MOV_SOUN_DINF_ATOM) +
        sizeof(MOV_SOUN_DREF_ATOM) +
        sizeof(MOV_SOUN_STBL_ATOM) +
        sizeof(MOV_SOUN_STSD_ATOM) +
        sizeof(MOV_SOUN_STSZ_ATOM) +  
        sizeof(MOV_SOUN_STTS_ATOM) +
        sizeof(MOV_SOUN_STSC_ATOM) + movAudisamplesperChunkCnt*sizeof(MOV_SOUN_SAMPLE_TO_CHUNK_ENTRY) +
        sizeof(MOV_SOUN_STCO_ATOM) + movSounSampleEntryCount * sizeof(MOV_SOUN_CHUNK_OFFSET_ENTRY);
        
    movSounMinfAtom.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&movSounMinfAtom, sizeof(MOV_SOUN_MINF_ATOM), &size) == 0)
            return 0;
    if (movWriteSounSmhdAtom(pFile) == 0)
            return 0;
        if (movWriteSounminfHdlrAtom(pFile) == 0)
            return 0;
            
    if (movWriteSounDinfAtom(pFile) == 0)
            return 0;
    
    if (movWriteSounStblAtom(pFile) == 0)
            return 0;


    return 1;
}

/*

Routine Description:

    Write SOUN DINF atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounDinfAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MOV_SOUN_DINF_ATOM movSounDinfAtom =
    {
        0x24000000,         /* size */
        0x666e6964,         /* type = "dinf" */
    };

    size =  sizeof(MOV_SOUN_DINF_ATOM) +            
        sizeof(MOV_SOUN_DREF_ATOM);
        
    movSounDinfAtom.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&movSounDinfAtom, sizeof(MOV_SOUN_DINF_ATOM), &size) == 0)
            return 0;

    if (movWriteSounDrefAtom(pFile) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Write SOUN DREF atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounDrefAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MOV_SOUN_DREF_ATOM movSounDrefAtom =
    {
        0x1c000000,         /* size */
        0x66657264,         /* type = "dref" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x0c000000,         /* size */
            0x206c7275,         /* type = "url " */
            0x01000000,         /* version_flags = 0x00000001 (self reference)  */
        },
    };

    movSounDrefAtom.size = bSwap32(sizeof(MOV_SOUN_DREF_ATOM));
    if (dcfWrite(pFile, (unsigned char*)&movSounDrefAtom, sizeof(MOV_SOUN_DREF_ATOM), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SOUN STBL atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounStblAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MOV_SOUN_STBL_ATOM movSounStblAtom =
    {
        0x00000000,         /* size */
        0x6c627473,         /* type = "stbl" */
    };

    size =  sizeof(MOV_SOUN_STBL_ATOM) +
        sizeof(MOV_SOUN_STSD_ATOM) +
        sizeof(MOV_SOUN_STSZ_ATOM) +  
        sizeof(MOV_SOUN_STTS_ATOM) +
        sizeof(MOV_SOUN_STSC_ATOM) + movAudisamplesperChunkCnt*sizeof(MOV_SOUN_SAMPLE_TO_CHUNK_ENTRY) +
        sizeof(MOV_SOUN_STCO_ATOM) + movSounSampleEntryCount * sizeof(MOV_SOUN_CHUNK_OFFSET_ENTRY);
        
    movSounStblAtom.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&movSounStblAtom, sizeof(MOV_SOUN_STBL_ATOM), &size) == 0)
            return 0;

    if (movWriteSounStsdAtom(pFile) == 0)
            return 0;
    if (movWriteSounStszAtom(pFile) == 0)
            return 0;
    if (movWriteSounSttsAtom(pFile) == 0)
            return 0;
    if (movWriteSounStscAtom(pFile) == 0)
            return 0;
    if (movWriteSounStcoAtom(pFile) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Write SOUN STSD atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounStsdAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MOV_SOUN_STSD_ATOM movSounStsdAtom =
    {
        0x34000000,         /* size */
        0x64737473,         /* type = "stsd" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_Channel */
        {
            0x24000000,         /* sample_description_size */
            0x74776f73,         /* data_format = "sowt" */
            0x00, 0x00, 0x00, 0x00,     /* reserved[6] */   
            0x00, 0x00,
            0x0100,             /* data_reference_index = data_reference of this track */
            0x00000000,         /* version_revision */
            0x00000000,         /* vendor */
            0x0100,             /* number_of_channels = 0x???? */
            0x1000,             /* sample_size = 0x???? */
            0x0000,             /* compression_id */
            0x0000,             /* packet_size */
            0x00000000,         /* sample_rate */
        },
    };

    movSounStsdAtom.size = bSwap32(sizeof(MOV_SOUN_STSD_ATOM));
    movSounStsdAtom.mov_soun_audio_sample_entry.number_of_channels = bSwap16(movAudioFormat.number_of_channels);
    movSounStsdAtom.mov_soun_audio_sample_entry.sample_size = bSwap16(movAudioFormat.sample_size);
    movSounStsdAtom.mov_soun_audio_sample_entry.sample_rate = bSwap32(movAudioFormat.sample_rate);
    if (dcfWrite(pFile, (unsigned char*)&movSounStsdAtom, sizeof(MOV_SOUN_STSD_ATOM), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SOUN STSZ atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounStszAtom(FS_FILE* pFile)
{
    u32 size, entry, i;
       u32 sampleCount;
    __align(4) MOV_SOUN_STSZ_ATOM movSounStszAtom =
    {
        0x00000000,         /* size */
        0x7a737473,         /* type = "stsz" */
        0x00000000,         /* version_flags */
        0x01000000,         /* sample_size */
        0x00000000,         /* number_of_entries = 0x???????? VOP count */
    };

    size = sizeof(MOV_SOUN_STSZ_ATOM);
       sampleCount=(movSounSampleChunkTable[1].first_chunk-movSounSampleChunkTable[0].first_chunk) * movSounSampleChunkTable[0].samples_per_chunk;
       sampleCount += movSounSampleChunkTable[1].first_chunk;   
    movSounStszAtom.size = bSwap32(size);
    movSounStszAtom.number_of_entries = bSwap32(sampleCount);

    if (dcfWrite(pFile, (unsigned char*)&movSounStszAtom, sizeof(MOV_SOUN_STSZ_ATOM), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SOUN STTS atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounSttsAtom(FS_FILE* pFile)
{
    u32 size;
       u32 sampleCount; 
    __align(4) MOV_SOUN_STTS_ATOM movSounSttsAtom =
    {
        0x18000000,         /* size */
        0x73747473,         /* type = "stts" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x00000000,         /* sample_count = 0x???????? = mp4SounSampleEntryCount sample_entry */  
            0x01000000,         /* sample_duration = 0x???????? = mp4SounSampleEntryDuration sample/sample_entry */
        },
    };
       sampleCount=(movSounSampleChunkTable[1].first_chunk-movSounSampleChunkTable[0].first_chunk) * movSounSampleChunkTable[0].samples_per_chunk;
       sampleCount += movSounSampleChunkTable[1].first_chunk;
    movSounSttsAtom.size = bSwap32(sizeof(MOV_SOUN_STTS_ATOM));
    movSounSttsAtom.mov_soun_sample_time_entry.sample_count = bSwap32(sampleCount);
    if (dcfWrite(pFile, (unsigned char*)&movSounSttsAtom, sizeof(MOV_SOUN_STTS_ATOM), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SOUN STSC atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounStscAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MOV_SOUN_STSC_ATOM movSounStscAtom =
    {
        0x1c000000,         /* size */
        0x63737473,         /* type = "stsc" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
    };

       size=sizeof(MOV_SOUN_STSC_ATOM) + movAudisamplesperChunkCnt*sizeof(MOV_SOUN_SAMPLE_TO_CHUNK_ENTRY);
    movSounStscAtom.size = bSwap32(size);
       movSounStscAtom.number_of_entries =bSwap32(movAudisamplesperChunkCnt);

    if (dcfWrite(pFile, (unsigned char*)&movSounStscAtom, sizeof(MOV_SOUN_STSC_ATOM), &size) == 0)
            return 0;
    
        if (dcfWrite(pFile, (unsigned char*)&movSounSampleChunkTable[0].first_chunk, movAudisamplesperChunkCnt*sizeof(MOV_SOUN_SAMPLE_TO_CHUNK_ENTRY), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SOUN STCO atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounStcoAtom(FS_FILE* pFile)
{
    u32 size, entry, i;
    __align(4) MOV_SOUN_STCO_ATOM movSounStcoAtom =
    {
        0x00000000,     /* size */
        0x6f637473,     /* type = "stco" */
        0x00000000,     /* version_flags */
        0x00000000,     /* number_of_entries */
    };

    size = sizeof(MOV_SOUN_STCO_ATOM) + movSounSampleEntryCount * sizeof(MOV_SOUN_CHUNK_OFFSET_ENTRY);
    movSounStcoAtom.size = bSwap32(size);
    movSounStcoAtom.number_of_entries = bSwap32(movSounSampleEntryCount);
    if (dcfWrite(pFile, (unsigned char*)&movSounStcoAtom, sizeof(MOV_SOUN_STCO_ATOM), &size) == 0)
            return 0; 
    
    if (dcfWrite(pFile, (unsigned char*)&movSounChunkOffsetTable[0], movSounSampleEntryCount*sizeof(MOV_SOUN_CHUNK_OFFSET_ENTRY), &size) == 0)
                return 0;

    
    return 1;
}

/*

Routine Description:

    Write SOUN SMHD atom.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 movWriteSounSmhdAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MOV_SOUN_SMHD_ATOM movSounSmhdAtom =
    {
        0x10000000,         /* size */
        0x64686d73,         /* type = "smhd" */
        0x01000000,         /* version_flags */
        0x0000,             /* balance = (0.0) */
        0x0000,             /* reserved */
    };

    movSounSmhdAtom.size = bSwap32(sizeof(MOV_SOUN_SMHD_ATOM));
    if (dcfWrite(pFile, (unsigned char*)&movSounSmhdAtom, sizeof(MOV_SOUN_SMHD_ATOM), &size) == 0)
            return 0;

    return 1;
}

#endif


/* Peter: 0727 S*/
/*

Routine Description:

    Find sub box address.

Arguments:

    pFile       - File handle.
    BoxLimit    - Box range.
    pBoxType    - Point to Box type array.
    pPosition   - Point to Box file position array.
    pBoxNum     - Point to Box number to be found.

Return Value:

    0 - Failure.
    1 - Success.

*/
#ifdef READ_ENABLE
s32 mp4FindSubBoxes(FS_FILE*    pFile, 
                    FS_i32      BoxLimit, 
                    u32*        pBoxType, 
                    FS_i32*     pPosition, 
                    u32*        pBoxNum)
{
    u32     size;
    FS_i32  FilePoint, FileCurrent;
    __align(4) MP4_BOX_HEADER_T mp4BoxHeader;
    
    FileCurrent = FS_FTell(pFile);      // record original position
    
    *pBoxNum   = 0;

    while(((FilePoint = FS_FTell(pFile)) < BoxLimit) && (*pBoxNum < MP4_BOX_NUM_MAX)) 
    {
        if(dcfRead(pFile, (u8*)&mp4BoxHeader, sizeof(MP4_BOX_HEADER_T), &size) == 0)
            return 0;
        
        *pBoxType++ = mp4BoxHeader.type;
        *pPosition++ = FilePoint;
        (*pBoxNum)++;
        
        size = bSwap32(mp4BoxHeader.size);
        FS_FSeek(pFile, (s32)size - sizeof(MP4_BOX_HEADER_T), FS_SEEK_CUR);
    }
    
    if(*pBoxNum >= MP4_BOX_NUM_MAX) 
    {
        DEBUG_MP4("Box number too many!!!!\n");
    }
    
    FS_FSeek(pFile, FileCurrent, FS_SEEK_SET);  // return to original position

    return 1;
}

/*

Routine Description:

    Find top level Box position in mp4 file.

Arguments:

    pFile - File handle.
    pFtyp - ftyp file position.
    pMoov - Point to moov Box file position.
    pMdat - Point to mdat Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
/*
s32 mp4FindMp4Boxes(FS_FILE* pFile, FS_i32* pFtyp, FS_i32* pMoov, FS_i32* pMdat)
{
    u32     size;
    FS_i32  FilePoint, FileCurrent;
    __align(4) MP4_BOX_HEADER_T mp4AtomHeader;
    
    FileCurrent = FS_FTell(pFile);      // record original position
    FS_FSeek(pFile, 0, FS_SEEK_SET);
    
    *pFtyp  = 0;
    *pMoov  = 0;
    *pMdat  = 0;
    while(dcfRead(pFile, (u8*)&mp4AtomHeader, sizeof(MP4_BOX_HEADER_T), &size) == 1) {
        FilePoint   = FS_FTell(pFile) - sizeof(MP4_BOX_HEADER_T);
        switch(mp4AtomHeader.type) {
        case 'pytf':    // ftyp
            *pFtyp  = FilePoint;
            break;
        case 'voom':    // moov
            *pMoov  = FilePoint;
            break;
        case 'tadm':    // mdat
            *pMdat  = FilePoint;
            break;
        default:
            break;
        }
        size    = bSwap32(mp4AtomHeader.size);
        FS_FSeek(pFile, (s32)size - sizeof(MP4_BOX_HEADER_T), FS_SEEK_CUR);
    }
    
    FS_FSeek(pFile, FileCurrent, FS_SEEK_SET);  // return to original position

    return 1;
}
*/
s32 mp4FindMp4Boxes(FS_FILE* pFile, FS_i32* pFtyp, FS_i32* pMoov, FS_i32* pMdat)
{
    FS_i32  FilePoint[MP4_BOX_NUM_MAX];
    u32     BoxType[MP4_BOX_NUM_MAX], BoxNum, i;

    *pFtyp  = -1;
    *pMoov  = -1;
    *pMdat  = -1;

    mp4FindSubBoxes(pFile, pFile->size, BoxType, FilePoint, &BoxNum);

    for(i = 0; i < BoxNum; i++) {
        switch(BoxType[i]) {
        case 'pytf':    // ftyp
            *pFtyp  = FilePoint[i];
            break;
        case 'voom':    // moov
            *pMoov  = FilePoint[i];
            break;
        case 'tadm':    // mdat
            *pMdat  = FilePoint[i];
            break;
        default:
            break;
        }
    }

    return 1;
}

/*

Routine Description:

    Find sub box address in moov Box.

Arguments:

    pFile       - File handle.
    MoovLimit   - moov Box range.
    pMvhd       - Point to mvhd Box file position.
    pIods       - Point to iods Box file position.
    pTrak       - Point to trak Box file position array, trak[4].

Return Value:

    0 - Failure.
    1 - Success.

*/
/*
s32 mp4FindMoovSubBoxes(FS_FILE* pFile, FS_i32 MoovLimit, FS_i32* pMvhd, FS_i32* pIods, FS_i32* pTrak)
{
    u32     size;
    FS_i32  FilePoint, FileCurrent;
    __align(4) MP4_BOX_HEADER_T mp4AtomHeader;
    
    FileCurrent = FS_FTell(pFile);      // record original position
    
    while((FilePoint = FS_FTell(pFile)) < MoovLimit) {
        if(dcfRead(pFile, (u8*)&mp4AtomHeader, sizeof(MP4_BOX_HEADER_T), &size) == 0)
            return 0;
        
        switch(mp4AtomHeader.type) {
        case 'dhvm':    // mvhd
            *pMvhd  = FilePoint;
            break;
        case 'sdoi':    // iods
            *pIods  = FilePoint;
            break;
        case 'kart':    // trak
            *pTrak  = FilePoint;
            pTrak++;
            break;
        default:
            break;
        }
        size    = bSwap32(mp4AtomHeader.size);
        FS_FSeek(pFile, (s32)size - sizeof(MP4_BOX_HEADER_T), FS_SEEK_CUR);
    }
    
    FS_FSeek(pFile, FileCurrent, FS_SEEK_SET);  // return to original position

    return 1;
}
*/
s32 mp4FindMoovSubBoxes(FS_FILE* pFile, FS_i32 MoovLimit, FS_i32* pMvhd, FS_i32* pIods, FS_i32* pTrak)
{
    u32     BoxType[MP4_BOX_NUM_MAX], BoxNum, i;
    FS_i32  FilePoint[MP4_BOX_NUM_MAX];

    *pMvhd  = -1;
    *pIods  = -1;
    
    for(i = 0; i < 4; i++) 
    {
        pTrak[i]  = -1;
    }
    
    mp4FindSubBoxes(pFile, MoovLimit, BoxType, FilePoint, &BoxNum);
    
    for(i = 0; i < BoxNum; i++) 
    {
        switch(BoxType[i]) 
        {
            case 'dhvm':    // mvhd
                *pMvhd  = FilePoint[i];
                break;

            case 'sdoi':    // iods
                *pIods  = FilePoint[i];
                break;

            case 'kart':    // trak
                *pTrak  = FilePoint[i];
                pTrak++;
                break;

            default:
                break;
        }
    }

    return 1;
}

/*

Routine Description:

    Find sub Box address in trak Box.

Arguments:

    pFile       - File handle.
    TrakLimit   - trak Box range.
    pTkhd       - Point to tkhd Box file position.
    pTref       - Point to tref Box file position.
    pMvhd       - Point to mvhd Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4FindTrakSubBoxes(FS_FILE*    pFile, 
                        FS_i32      TrakLimit, 
                        FS_i32*     pTkhd, 
                        FS_i32*     pTref, 
                        FS_i32*     pMdia)
{
    u32     BoxType[MP4_BOX_NUM_MAX], BoxNum, i;
    FS_i32  FilePoint[MP4_BOX_NUM_MAX];

    *pTkhd  = -1;
    *pTref  = -1;
    *pMdia  = -1;
    
    mp4FindSubBoxes(pFile, TrakLimit, BoxType, FilePoint, &BoxNum);
    
    for(i = 0; i < BoxNum; i++) 
    {
        switch(BoxType[i]) 
        {
            case 'dhkt':    // tkhd
                *pMdia  = FilePoint[i];
                break;
                
            case 'fert':    // tref
                *pMdia  = FilePoint[i];
                break;
                
            case 'aidm':    // mdia
                *pMdia  = FilePoint[i];
                break;

            default:
                break;
        }
    }

    return 1;
}

/*

Routine Description:

    Find sub Box address in trak box.

Arguments:

    pFile       - File handle.
    MdiaLimit   - mdia Box range.
    pMdhd       - Point to mdhd Box file position.
    pHdlr       - Point to hdlr Box file position.
    pMinf       - Point to minf Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4FindMdiaSubBoxes(FS_FILE*    pFile, 
                        FS_i32      MdiaLimit, 
                        FS_i32*     pMdhd, 
                        FS_i32*     pHdlr, 
                        FS_i32*     pMinf)
{
    u32     BoxType[MP4_BOX_NUM_MAX], BoxNum, i;
    FS_i32  FilePoint[MP4_BOX_NUM_MAX];

    *pMdhd  = -1;
    *pHdlr  = -1;
    *pMinf  = -1;
    
    mp4FindSubBoxes(pFile, MdiaLimit, BoxType, FilePoint, &BoxNum);
    
    for(i = 0; i < BoxNum; i++) 
    {
        switch(BoxType[i]) 
        {
            case 'dhdm':    // mdhd
                *pHdlr  = FilePoint[i];
                break;

            case 'rldh':    // hdlr
                *pHdlr  = FilePoint[i];
                break;

            case 'fnim':    // minf
                *pMinf  = FilePoint[i];
                break;

            default:
                break;
        }
    }

    return 1;
}

/*

Routine Description:

    Find sub Box address in minf box.

Arguments:

    pFile       - File handle.
    MinfLimit   - minf box range.
    pVmhd       - Point to vmhd Box file position.
    pSmhd       - Point to smhd Box file position.
    pNmhd       - Point to nmhd Box file position.
    pDinf       - Point to dinf Box file position.
    pStbl       - Point to stbl Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4FindMinfSubBoxes(FS_FILE*    pFile, 
                        FS_i32      MinfLimit, 
                        FS_i32*     pVmhd, 
                        FS_i32*     pSmhd, 
                        FS_i32*     pNmhd, 
                        FS_i32*     pDinf, 
                        FS_i32*     pStbl)
{
    u32     BoxType[MP4_BOX_NUM_MAX], BoxNum, i;
    FS_i32  FilePoint[MP4_BOX_NUM_MAX];

    *pVmhd  = -1;
    *pSmhd  = -1;
    *pNmhd  = -1;
    *pDinf  = -1;
    *pStbl  = -1;
    
    mp4FindSubBoxes(pFile, MinfLimit, BoxType, FilePoint, &BoxNum);
    
    for(i = 0; i < BoxNum; i++) 
    {
        switch(BoxType[i]) 
        {
            case 'dhmv':    // vmhd
                *pVmhd  = FilePoint[i];
                break;

            case 'dhms':    // smhd
                *pSmhd  = FilePoint[i];
                break;

            case 'dhmn':    // nmhd
                *pNmhd  = FilePoint[i];
                break;

            case 'fnid':    // dinf
                *pDinf  = FilePoint[i];
                break;

            case 'lbts':    // stbl
                *pStbl  = FilePoint[i];
                break;

            default:
                break;
        }
    }

    return 1;
}

/*

Routine Description:

    Find sub atom address in dinf Box.

Arguments:

    pFile       - File handle.
    DinfLimit   - dinf box range.
    pDref       - Point to dref Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4FindDinfSubBoxes(FS_FILE*    pFile, 
                        FS_i32      DinfLimit, 
                        FS_i32*     pDref)
{
    u32     BoxType[MP4_BOX_NUM_MAX], BoxNum, i;
    FS_i32  FilePoint[MP4_BOX_NUM_MAX];

    *pDref  = -1;
    
    mp4FindSubBoxes(pFile, DinfLimit, BoxType, FilePoint, &BoxNum);
    
    for(i = 0; i < BoxNum; i++) 
    {
        switch(BoxType[i]) 
        {
            case 'ferd':    // dref
                *pDref  = FilePoint[i];
                break;

            default:
                break;
        }
    }

    return 1;
}

/*

Routine Description:

    Find sub box address in stbl box.

Arguments:

    pFile       - File handle.
    StblLimit   - stbl box range.
    pStsd       - Point to stsd atom file position.
    pStts       - Point to stts atom file position.
    pStsc       - Point to stsc atom file position.
    pStsz       - Point to stsz atom file position.
    pStco       - Point to stco atom file position.
    pStss       - Point to stss atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4FindStblSubBoxes(FS_FILE*    pFile, 
                        FS_i32      StblLimit, 
                        FS_i32*     pStsd, 
                        FS_i32*     pStts, 
                        FS_i32*     pStsc, 
                        FS_i32*     pStsz,
                        FS_i32*     pStco,
                        FS_i32*     pStss)
{
    u32     BoxType[MP4_BOX_NUM_MAX], BoxNum, i;
    FS_i32  FilePoint[MP4_BOX_NUM_MAX];

    *pStsd  = -1;
    *pStts  = -1;
    *pStsc  = -1;
    *pStsz  = -1;
    *pStco  = -1;
    *pStss  = -1;
    
    mp4FindSubBoxes(pFile, StblLimit, BoxType, FilePoint, &BoxNum);
    
    for(i = 0; i < BoxNum; i++) 
    {
        switch(BoxType[i]) 
        {
            case 'dsts':    // stsd
                *pStsd  = FilePoint[i];
                break;

            case 'stts':    // stts
                *pStts  = FilePoint[i];
                break;

            case 'csts':    // stsc
                *pStsc  = FilePoint[i];
                break;

            case 'zsts':    // stsz
                *pStsz  = FilePoint[i];
                break;

            case 'octs':    // stco
                *pStco  = FilePoint[i];
                break;

            case 'ssts':    // stss
                *pStss  = FilePoint[i];
                break;

            default:
                break;
        }
    }

    return 1;
}

/*

Routine Description:

    Read FTYP Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadFtypBox(FS_FILE* pFile, FS_i32 FtypPosition)
{
    u32 size;
    __align(4) MOV_FTYP_BOX_T tMp4FtypBox;  
    
    FS_FSeek(pFile, FtypPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4FtypBox, sizeof(MOV_FTYP_BOX_T), &size) == 0)
        return 0;

    size = bSwap32(tMp4FtypBox.size); 

    if (sizeof(MOV_FTYP_BOX_T) != size) 
    {
        FS_FSeek(pFile, (s32)size - sizeof(MOV_FTYP_BOX_T), FS_SEEK_CUR);
    }
    
    return 1;
}

/*

Routine Description:

    Read MOOV Box.

Arguments:

    pFile           - File handle.
    MoovPosition    - Point to moov Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadMoovBox(FS_FILE* pFile, FS_i32 MoovPosition)
{
    u32     size, i;
    FS_i32  MoovLimit, MvhdPosition, IodsPosition, TrakPosition[4];
    __align(4) MP4_MOOV_BOX_T    tMp4MoovBox;

    FS_FSeek(pFile, MoovPosition, FS_SEEK_SET);
    
    if (dcfRead(pFile, (unsigned char*)&tMp4MoovBox, sizeof(MP4_MOOV_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4MoovBox.size);
    MoovLimit   = MoovPosition + size;

    mp4FindMoovSubBoxes(pFile, MoovLimit, &MvhdPosition, &IodsPosition, TrakPosition);
    
    if (mp4ReadMvhdBox(pFile, MvhdPosition) == 0)
        return 0;

    if (mp4ReadIodsBox(pFile, IodsPosition) == 0)
        return 0;
        
    for(i = 0; i < 4; i++) 
    {
        if(TrakPosition[i] == -1)
            break;

        mp4ReadTrakBox(pFile, TrakPosition[i]);
    }
    
    return 1;
}

/*

Routine Description:

    Read MVHD Box.

Arguments:

    pFile           - File handle.
    MvhdPosition    - Point to mvhd atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadMvhdBox(FS_FILE* pFile, FS_i32 MvhdPosition)
{
    u32 size;
    
    __align(4) MP4_MVHD_BOX_T tMp4MvhdBox;
    
    FS_FSeek(pFile, MvhdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4MvhdBox, sizeof(MP4_MVHD_BOX_T), &size) == 0)
        return 0;

    return 1;   
}

/*

Routine Description:

    Read IODS Box.

Arguments:

    pFile           - File handle.
    IodsPosition    - Point to iods atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadIodsBox(FS_FILE* pFile, FS_i32 IodsPosition)
{
    u32 size;
    
    __align(4) MP4_IODS_BOX_T tMp4IodsBox;

    FS_FSeek(pFile, IodsPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4IodsBox, sizeof(MP4_IODS_BOX_T), &size) == 0)
        return 0;

    return 1;   
}

/************************************************************************/
/* MP4 Tracks                                                           */
/************************************************************************/

/*

Routine Description:

    Read TRAK Box.

Arguments:

    pFile           - File handle.
    TrakPosition    - Point to trak Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadTrakBox(FS_FILE* pFile, FS_i32 TrakPosition)
{
    u32     size;
    __align(4) MP4_TRAK_BOX_T tMp4TrakBox;
    FS_i32  TrakLimit, TkhdPosition, TrefPosition, MdiaPosition;

    FS_FSeek(pFile, TrakPosition, FS_SEEK_SET);
    
    if (dcfRead(pFile, (unsigned char*)&tMp4TrakBox, sizeof(MP4_TRAK_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4TrakBox.size);
    TrakLimit   = TrakPosition + size;

    mp4FindTrakSubBoxes(pFile, TrakLimit, &TkhdPosition, &TrefPosition, &MdiaPosition);

    if (TkhdPosition >= 0)
        if (mp4ReadTkhdBox(pFile, TkhdPosition) == 0)
            return 0;

    if (TrefPosition >= 0)   // only used in ODSM track
        if (mp4ReadOdsmTrefBox(pFile, TrefPosition) == 0)
            return 0;

    if (MdiaPosition >= 0)
        if (mp4ReadMdiaBox(pFile, MdiaPosition) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Read TKHD Box.

Arguments:

    pFile           - File handle.
    TkhdPosition    - Point to tkhd Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadTkhdBox(FS_FILE* pFile, FS_i32 TkhdPosition)
{
    u32 size;
    __align(4) MP4_TKHD_BOX_T tMp4TkhdBox;

    FS_FSeek(pFile, TkhdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4TkhdBox, sizeof(MP4_TKHD_BOX_T), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read MDIA Box.

Arguments:

    pFile           - File handle.
    MdiaPosition    - Point to mdia Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadMdiaBox(FS_FILE* pFile, FS_i32 MdiaPosition)
{
    u32     size, HdlrType;
    __align(4) MP4_MDIA_BOX_T tMp4MdiaBox;
    FS_i32  MdiaLimit, MdhdPosition, HdlrPosition, MinfPosition;

    FS_FSeek(pFile, MdiaPosition, FS_SEEK_SET);
    
    if (dcfRead(pFile, (unsigned char*)&tMp4MdiaBox, sizeof(MP4_MDIA_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4MdiaBox.size);
    MdiaLimit   = MdiaPosition + size;

    mp4FindMdiaSubBoxes(pFile, MdiaLimit, &MdhdPosition, &HdlrPosition, &MinfPosition);

    if (MdhdPosition >= 0)
        if (mp4ReadMdhdBox(pFile, MdhdPosition) == 0)
            return 0;

    if (HdlrPosition >= 0)
        if (mp4ReadHdlrBox(pFile, HdlrPosition, &HdlrType) == 0)
            return 0;

    if (MinfPosition >= 0) 
    {
        switch (HdlrType) 
        {
            case 'ediv':    // vide
                if (mp4ReadVideMinfBox(pFile, MinfPosition) == 0)
                    return 0;
                break;

            case 'nuos':    // soun

#ifdef MOV_AUDIO
                if (mp4ReadSounMinfAtom(pFile, MinfPosition) == 0)
                    return 0;
#else
                DEBUG_MP4("MP4 sound functions are disabled!!\n");
#endif
                break;

            case 'msdo':    // odsm
                if (mp4ReadOdsmMinfBox(pFile, MinfPosition) == 0)
                    return 0;
                break;

            case 'msds':    // sdsm
                if (mp4ReadSdsmMinfBox(pFile, MinfPosition) == 0)
                    return 0;
                break;

            default:
                DEBUG_MP4("Hdlr atom handler_type 0x%08x isn't a valid value!!", HdlrType);
                break;
        }
    }

    return 1;
}

/*

Routine Description:

    Read MDHD Box.

Arguments:

    pFile           - File handle.
    MdhdPosition    - Point to mdhd Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadMdhdBox(FS_FILE* pFile, FS_i32 MdhdPosition)
{
    u32 size;
    __align(4) MP4_MDHD_BOX_T tMp4MdhdBox;

    FS_FSeek(pFile, MdhdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4MdhdBox, sizeof(MP4_MDHD_BOX_T), &size) == 0)
        return 0;   

    return 1;
}

/*

Routine Description:

    Read HDLR Box.

Arguments:

    pFile           - File handle.
    HdlrPosition    - Point to hdlr Box file position.
    handler_type    - Handler type: "vide", "soun", "odsm", "sdsm"

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadHdlrBox(FS_FILE* pFile, FS_i32 HdlrPosition, u32* handler_type)
{
    u32 size;
    __align(4) MP4_HDLR_BOX_T tMp4HdlrBox;

    FS_FSeek(pFile, HdlrPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4HdlrBox, sizeof(MP4_HDLR_BOX_T), &size) == 0)
        return 0;

    *handler_type   = tMp4HdlrBox.component_subtype;

    return 1;
}


/************************************************************************/
/* MP4 Video Track                                                      */
/************************************************************************/

/*

Routine Description:

    Read VIDE MINF Box.

Arguments:

    pFile           - File handle.
    MinfPosition    - Point to minf Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideMinfBox(FS_FILE* pFile, FS_i32 MinfPosition)
{
    u32     size;
    __align(4) MP4_VIDE_MINF_BOX_T tMp4VideMinfBox;
    FS_i32  MinfLimit, VmhdPosition, SmhdPosition, NmhdPosition, DinfPosition, StblPosition;

    FS_FSeek(pFile, MinfPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4VideMinfBox, sizeof(MP4_VIDE_MINF_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4VideMinfBox.size);
    MinfLimit   = MinfPosition + size;

    mp4FindMinfSubBoxes(pFile, 
                        MinfLimit, 
                        &VmhdPosition, 
                        &SmhdPosition, 
                        &NmhdPosition, 
                        &DinfPosition, 
                        &StblPosition);

    if (DinfPosition >= 0)
        if (mp4ReadVideDinfBox(pFile, DinfPosition) == 0)
            return 0;

    if (StblPosition >= 0)
        if (mp4ReadVideStblBox(pFile, StblPosition) == 0)
            return 0;

    if (VmhdPosition >= 0)
        if (mp4ReadVideVmhdBox(pFile, VmhdPosition) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Read VIDE DINF Box.

Arguments:

    pFile           - File handle.
    DinfPosition    - Point to dinf Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideDinfBox(FS_FILE* pFile, FS_i32 DinfPosition)
{
    u32     size;
    __align(4) MP4_VIDE_DINF_BOX_T tMp4VideDinfBox;
    FS_i32  DinfLimit, DrefPosition;

    FS_FSeek(pFile, DinfPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4VideDinfBox, sizeof(MP4_VIDE_DINF_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4VideDinfBox.size);
    DinfLimit   = DinfPosition + size;
    mp4FindDinfSubBoxes(pFile, DinfLimit, &DrefPosition);

    if (DrefPosition >= 0)
        if (mp4ReadVideDrefBox(pFile, DrefPosition) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Read VIDE DREF Box.

Arguments:

    pFile           - File handle.
    DrefPosition    - Point to dref Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideDrefBox(FS_FILE* pFile, FS_i32 DrefPosition)
{
    u32 size;
    __align(4) MP4_VIDE_DREF_BOX_T tMp4VideDrefBox;

    FS_FSeek(pFile, DrefPosition, FS_SEEK_SET);
    if (dcfRead(pFile, (unsigned char*)&tMp4VideDrefBox, sizeof(MP4_VIDE_DREF_BOX_T), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read VIDE STBL Box.

Arguments:

    pFile           - File handle.
    StblPosition    - Point to stbl Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideStblBox(FS_FILE* pFile, FS_i32 StblPosition)
{
    u32     size;
    __align(4) MP4_VIDE_STBL_BOX_T tMp4VideStblBox;
    FS_i32  StblLimit, StsdPosition, StszPosition, SttsPosition, StscPosition, StcoPosition, StssPosition;

    FS_FSeek(pFile, StblPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4VideStblBox, sizeof(MP4_VIDE_STBL_BOX_T), &size) == 0)
            return 0;

    size        = bSwap32(tMp4VideStblBox.size);
    StblLimit   = StblPosition + size;
    
    mp4FindStblSubBoxes(pFile, 
                        StblLimit, 
                        &StsdPosition, 
                        &SttsPosition, 
                        &StscPosition, 
                        &StszPosition,
                        &StcoPosition,
                        &StssPosition);

    if (StsdPosition >= 0)
        if (mp4ReadVideStsdBox(pFile, StsdPosition) == 0) // SampleDescriptionBox
            return 0;

    if (SttsPosition >= 0)
        if (mp4ReadVideSttsBox(pFile, SttsPosition) == 0) // DecodingTimeToSampleBox
            return 0;

    if (StscPosition >= 0)
        if (mp4ReadVideStscBox(pFile, StscPosition) == 0) // SampleToChunkBox
            return 0;

    if (StszPosition >= 0)
        if (mp4ReadVideStszBox(pFile, StszPosition) == 0) // SampleSizeBox
            return 0;

    if (StcoPosition >= 0)
        if (mp4ReadVideStcoBox(pFile, StcoPosition) == 0) // ChunkOffsetBox
            return 0;

    if (StssPosition >= 0)
        if (mp4ReadVideStssBox(pFile, StssPosition) == 0) // SyncSampleBox
            return 0;
    
    return 1;
}

/*

Routine Description:

    Read VIDE STSD Box.

Arguments:

    pFile           - File handle.
    StsdPosition    - Point to stsd Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideStsdBox(FS_FILE* pFile, FS_i32 StsdPosition)
{
    u32     size, width_height;
    __align(4) MP4_VIDE_STSD_BOX_T tMp4VideStsdBox;
    __align(4) MP4_VIDE_VISUAL_SAMPLE_ENTRY mp4VideVisualSampleEntry;
    __align(4) MP4_VIDE_ESDS_BOX_T tMp4VideEsdsBox;
    __align(4) MP4_VIDE_ES_DESCRIPTOR mp4VideEsDescriptor;
    __align(4) MP4_VIDE_DECODER_CONFIG_DESCRIPTOR mp4VideDecoderConfigDescriptor;
    __align(4) MP4_VIDE_DECODER_SPECIFIC_INFO mp4VideDecoderSpecificInfo;
    __align(4) MP4_VIDE_SL_CONFIG_DESCRIPTOR mp4VideSlConfigDescriptor;

    FS_FSeek(pFile, StsdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4VideStsdBox, sizeof(MP4_VIDE_STSD_BOX_T), &size) == 0)
        return 0;;

    if (dcfRead(pFile, (unsigned char*)&mp4VideVisualSampleEntry, sizeof(MP4_VIDE_VISUAL_SAMPLE_ENTRY), &size) == 0)
        return 0;
    width_height    = bSwap32(mp4VideVisualSampleEntry.width_height);
    mp4VopWidth     = (width_height >> 16) & 0xffff;
    mp4VopHeight    = width_height & 0xffff;

    if (dcfRead(pFile, (unsigned char*)&tMp4VideEsdsBox, sizeof(MP4_VIDE_ESDS_BOX_T), &size) == 0)
        return 0;
    if (dcfRead(pFile, (unsigned char*)&mp4VideEsDescriptor, sizeof(MP4_VIDE_ES_DESCRIPTOR), &size) == 0)
        return 0;
    if (dcfRead(pFile, (unsigned char*)&mp4VideDecoderConfigDescriptor, sizeof(MP4_VIDE_DECODER_CONFIG_DESCRIPTOR), &size) == 0)
        return 0;
    if (dcfRead(pFile, (unsigned char*)&mp4VideDecoderSpecificInfo, sizeof(MP4_VIDE_DECODER_SPECIFIC_INFO), &size) == 0)
        return 0;
        
    mp4VideHeaderSize   = mp4VideDecoderSpecificInfo.dsi_tag_length - 10;
    if (dcfRead(pFile, (unsigned char*)&mp4VideHeader, mp4VideHeaderSize, &size) == 0)
        return 0;
    if(mpeg4DecodeVolHeader(&Mp4Dec_opt,(u8*)mp4VideHeader, mp4VideHeaderSize) == 0)
        return 0;
    
    if (dcfRead(pFile, (unsigned char*)&mp4VideSlConfigDescriptor, sizeof(MP4_VIDE_SL_CONFIG_DESCRIPTOR), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read VIDE STSZ Box.

Arguments:

    pFile           - File handle.
    StszPosition    - Point to stsz Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideStszBox(FS_FILE* pFile, FS_i32 StszPosition)
{
    u32     size, i;
    __align(4) MP4_VIDE_STSZ_BOX_T tMp4VideStszBox;

    FS_FSeek(pFile, StszPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4VideStszBox, sizeof(MP4_VIDE_STSZ_BOX_T), &size) == 0)
        return 0;
    
    mp4VopCount = bSwap32(tMp4VideStszBox.number_of_entries);
    
    if(tMp4VideStszBox.sample_size == 0) 
    {       
/* mh@2006/11/10: Improve metadata-reading performance */
#if 0       
        for (i = 0; i < mp4VopCount; i++) 
        {
            if (dcfRead(pFile, (unsigned char*)&entry, sizeof(MP4_VIDE_SAMPLE_SIZE_ENTRY), &size) == 0)
                return 0;
            mp4VideSampleSizeTable[i] = bSwap32(entry);
        }
#else
        if (dcfRead(pFile, (unsigned char*)&mp4VideSampleSizeTable, mp4VopCount * sizeof(MP4_VIDE_SAMPLE_SIZE_ENTRY), &size) == 0)
                return 0;
#endif
/* mh@2006/11/10: END */
    }

    // Perforam byte swap
    for (i=0; i<mp4VopCount; i++)
    {
        mp4VideSampleSizeTable[i] = bSwap32(mp4VideSampleSizeTable[i]);
    }

    return 1;
}

/*

Routine Description:

    Read VIDE STTS Box.

Arguments:

    pFile           - File handle.
    SttsPosition    - Point to stts Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideSttsBox(FS_FILE* pFile, FS_i32 SttsPosition)
{
    u32 size, i;
    __align(4) MP4_VIDE_STTS_BOX_T tMp4VideSttsBox;

    FS_FSeek(pFile, SttsPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4VideSttsBox, sizeof(MP4_VIDE_STTS_BOX_T), &size) == 0)
        return 0;
    
    mp4TimeToSampleTableEntryCount = bSwap32(tMp4VideSttsBox.number_of_entries);

/* mh@2006/11/10: Improve metadata-reading performance */
#if 0 
    for (i = 0; i < mp4TimeToSampleTableEntryCount; i++)
    {
        if (dcfRead(pFile, (unsigned char*)&entry, sizeof(u32), &size) == 0)
            return 0;
        mp4VideSampleTimeTable[i].sample_count      = bSwap32(entry);
        
        if (dcfRead(pFile, (unsigned char*)&entry, sizeof(u32), &size) == 0)
            return 0;
        mp4VideSampleTimeTable[i].sample_duration   = bSwap32(entry);
    }
#else
    if (dcfRead(pFile, (unsigned char*)mp4VideSampleTimeTable, mp4TimeToSampleTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TIME_ENTRY), &size) == 0)
        return 0;
#endif

    // Perforam byte swap
    for (i=0; i<mp4TimeToSampleTableEntryCount; i++)
    {
        mp4VideSampleTimeTable[i].sample_count = bSwap32(mp4VideSampleTimeTable[i].sample_count);
        mp4VideSampleTimeTable[i].sample_duration = bSwap32(mp4VideSampleTimeTable[i].sample_duration);
    }

/* mh@2006/11/10: END */

    return 1;
}

/*

Routine Description:

    Read VIDE STSC Box.

Arguments:

    pFile           - File handle.
    StscPosition    - Point to stsc Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideStscBox(FS_FILE* pFile, FS_i32 StscPosition)
{
    u32 size, i;
    __align(4) MP4_VIDE_STSC_BOX_T tMp4VideStscBox;

    FS_FSeek(pFile, StscPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4VideStscBox, sizeof(MP4_VIDE_STSC_BOX_T), &size) == 0)
        return 0;

    mp4SampleToChunkTableEntryCount = bSwap32(tMp4VideStscBox.number_of_entries);

    if (dcfRead(pFile, (unsigned char*)mp4VideSampleToChunkTable, mp4SampleToChunkTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY), &size) == 0)
        return 0;

    // Perforam byte swap
    for (i=0; i<mp4SampleToChunkTableEntryCount; i++)
    {
        mp4VideSampleToChunkTable[i].first_chunk = bSwap32(mp4VideSampleToChunkTable[i].first_chunk);
        mp4VideSampleToChunkTable[i].samples_per_chunk = bSwap32(mp4VideSampleToChunkTable[i].samples_per_chunk);
        mp4VideSampleToChunkTable[i].sample_description_id = bSwap32(mp4VideSampleToChunkTable[i].sample_description_id);
    }
        
    return 1;
}

/*

Routine Description:

    Read VIDE STCO Box.

Arguments:

    pFile           - File handle.
    StcoPosition    - Point to stbl Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideStcoBox(FS_FILE* pFile, FS_i32 StcoPosition)
{
    u32 size, i;
    __align(4) MP4_VIDE_STCO_BOX_T tMp4VideStcoBox;

       FS_FSeek(pFile, StcoPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4VideStcoBox, sizeof(MP4_VIDE_STCO_BOX_T), &size) == 0)
        return 0;
    
    mp4ChunkOffsetTableEntryCount = bSwap32(tMp4VideStcoBox.number_of_entries);

/* mh@2006/11/10: Improve metadata-reading performance */
#if 0 
    for (i = 0; i < mp4ChunkOffsetTableEntryCount; i++)
    {
        if (dcfRead(pFile, (unsigned char*)&entry, sizeof(MP4_VIDE_CHUNK_OFFSET_ENTRY), &size) == 0)
            return 0;
        mp4VideChunkOffsetTable[i] = bSwap32(entry);
    }
#else
    if (dcfRead(pFile, (unsigned char*)&mp4VideChunkOffsetTable, mp4ChunkOffsetTableEntryCount * sizeof(MP4_VIDE_CHUNK_OFFSET_ENTRY), &size) == 0)
            return 0;

    // perform byte swap
    for (i = 0; i < mp4ChunkOffsetTableEntryCount; i++)
    {
        mp4VideChunkOffsetTable[i] = bSwap32(mp4VideChunkOffsetTable[i]);
    }
    
#endif
/* mh@2006/11/10: END */

    return 1;
}

/*

Routine Description:

    Read VIDE STSS Box.

Arguments:

    pFile           - File handle.
    StssPosition    - Point to stss Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideStssBox(FS_FILE* pFile, FS_i32 StssPosition)
{
    u32 size, i;
    __align(4) MP4_VIDE_STSS_BOX_T tMp4VideStssBox;

    FS_FSeek(pFile, StssPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4VideStssBox, sizeof(MP4_VIDE_STSS_BOX_T), &size) == 0)
        return 0;

    mp4IVopCount = bSwap32(tMp4VideStssBox.number_of_entries);

/* mh@2006/11/10: Improve metadata-reading performance */
#if 0   
    for (i = 0; i < mp4IVopCount; i++)
    {
        if (dcfRead(pFile, (unsigned char*)&entry, sizeof(MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY), &size) == 0)
            return 0;
        mp4VideSyncSampleNumberTable[i] = bSwap32(entry);
    }
#else
    if (dcfRead(pFile, (unsigned char*)&mp4VideSyncSampleNumberTable, mp4IVopCount * sizeof(MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY), &size) == 0)
            return 0;

    // perform byte swap
    for (i = 0; i < mp4IVopCount; i++)
    {
        mp4VideSyncSampleNumberTable[i] = bSwap32(mp4VideSyncSampleNumberTable[i]);
    }
#endif
/* mh@2006/11/10: END */

    return 1;
}

/*

Routine Description:

    Read VIDE VMHD Box.

Arguments:

    pFile           - File handle.
    VmhdPosition    - Point to vmhd Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadVideVmhdBox(FS_FILE* pFile, FS_i32 VmhdPosition) 
{
    u32 size;
    __align(4) MP4_VIDE_VMHD_BOX_T tMp4VideVmhdBox;

    FS_FSeek(pFile, VmhdPosition, FS_SEEK_SET);
    if (dcfRead(pFile, (unsigned char*)&tMp4VideVmhdBox, sizeof(MP4_VIDE_VMHD_BOX_T), &size) == 0)
        return 0;

    return 1;
}


/************************************************************************/
/* MP4 Soun Track                           */
/************************************************************************/

#ifdef MOV_AUDIO

/*

Routine Description:

    Read SOUN MINF atom.

Arguments:

    pFile           - File handle.
    MinfPosition    - Point to minf atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSounMinfAtom(FS_FILE* pFile, FS_i32 MinfPosition)
{
    u32     size;
    __align(4) MP4_SOUN_MINF_ATOM mp4SounMinfAtom;
    FS_i32  MinfLimit, VmhdPosition, SmhdPosition, NmhdPosition, DinfPosition, StblPosition;

    FS_FSeek(pFile, MinfPosition, FS_SEEK_SET);
    if (dcfRead(pFile, (unsigned char*)&mp4SounMinfAtom, sizeof(MP4_SOUN_MINF_ATOM), &size) == 0)
            return 0;

    size        = bSwap32(mp4SounMinfAtom.size);
    MinfLimit   = MinfPosition + size;
    mp4FindMinfSubBoxes(pFile, 
                        MinfLimit, 
                        &VmhdPosition, 
                        &SmhdPosition, 
                        &NmhdPosition, 
                        &DinfPosition, 
                        &StblPosition);

    if (DinfPosition >= 0)
        if (mp4ReadSounDinfAtom(pFile, DinfPosition) == 0)
            return 0;
    if (StblPosition >= 0)
        if (mp4ReadSounStblAtom(pFile, StblPosition) == 0)
            return 0;
    if (SmhdPosition >= 0)
        if (mp4ReadSounSmhdAtom(pFile, SmhdPosition) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Read SOUN DINF atom.

Arguments:

    pFile           - File handle.
    DinfPosition    - Point to dinf atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSounDinfAtom(FS_FILE* pFile, FS_i32 DinfPosition)
{
    u32     size;
    __align(4) MP4_SOUN_DINF_ATOM mp4SounDinfAtom;
    FS_i32  DinfLimit, DrefPosition;

    FS_FSeek(pFile, DinfPosition, FS_SEEK_SET);
    if (dcfRead(pFile, (unsigned char*)&mp4SounDinfAtom, sizeof(MP4_SOUN_DINF_ATOM), &size) == 0)
        return 0;

    size        = bSwap32(mp4SounDinfAtom.size);
    DinfLimit   = DinfPosition + size;
    mp4FindDinfSubBoxes(pFile, DinfLimit, &DrefPosition);

    if (DrefPosition >= 0)
        if (mp4ReadSounDrefAtom(pFile, DrefPosition) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Read SOUN DREF atom.

Arguments:

    pFile           - File handle.
    DrefPosition    - Point to dref atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSounDrefAtom(FS_FILE* pFile, FS_i32 DrefPosition)
{
    u32     size;
    __align(4) MP4_SOUN_DREF_ATOM mp4SounDrefAtom;

    FS_FSeek(pFile, DrefPosition, FS_SEEK_SET);
    if (dcfRead(pFile, (unsigned char*)&mp4SounDrefAtom, sizeof(MP4_SOUN_DREF_ATOM), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read SOUN STBL atom.

Arguments:

    pFile           - File handle.
    StblPosition    - Point to stbl atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSounStblAtom(FS_FILE* pFile, FS_i32 StblPosition)
{
    u32     size;
    __align(4) MP4_SOUN_STBL_ATOM mp4SounStblAtom;
    FS_i32  StblLimit, StsdPosition, StszPosition, SttsPosition, StscPosition, StcoPosition, StssPosition;

    FS_FSeek(pFile, StblPosition, FS_SEEK_SET);
    if (dcfRead(pFile, (unsigned char*)&mp4SounStblAtom, sizeof(MP4_SOUN_STBL_ATOM), &size) == 0)
        return 0;

    size        = bSwap32(mp4SounStblAtom.size);
    StblLimit   = StblPosition + size;
    mp4FindStblSubBoxes(pFile, 
                        StblLimit, 
                        &StsdPosition, 
                        &SttsPosition, 
                        &StscPosition, 
                        &StszPosition,
                        &StcoPosition,
                        &StssPosition);

    if(StsdPosition >= 0)
        if (mp4ReadSounStsdAtom(pFile, StsdPosition) == 0)
            return 0;
    if(StszPosition >= 0)
        if (mp4ReadSounStszAtom(pFile, StszPosition) == 0)
            return 0;
    if(SttsPosition >= 0)
        if (mp4ReadSounSttsAtom(pFile, SttsPosition) == 0)
            return 0;
    if(StscPosition >= 0)
        if (mp4ReadSounStscAtom(pFile, StscPosition) == 0)
            return 0;
    if(StcoPosition >= 0)
        if (mp4ReadSounStcoAtom(pFile, StcoPosition) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Read SOUN STSD atom.

Arguments:

    pFile           - File handle.
    StsdPosition    - Point to stsd atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSounStsdAtom(FS_FILE* pFile, FS_i32 StsdPosition)
{
    u32     size;
    __align(4) MP4_SOUN_STSD_ATOM mp4SounStsdAtom;

    FS_FSeek(pFile, StsdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&mp4SounStsdAtom, sizeof(MP4_SOUN_STSD_ATOM), &size) == 0)
        return 0;
    mp4AudioFormat.number_of_channels   = bSwap16(mp4SounStsdAtom.mp4_soun_audio_sample_entry[0].number_of_channels);
    mp4AudioFormat.sample_size          = bSwap16(mp4SounStsdAtom.mp4_soun_audio_sample_entry[0].sample_size);
    mp4AudioFormat.sample_rate          = (bSwap32(mp4SounStsdAtom.mp4_soun_audio_sample_entry[0].sample_rate) >> 16) & 0xffff;

    return 1;
}

/*

Routine Description:

    Read SOUN STSZ atom.

Arguments:

    pFile           - File handle.
    StszPosition    - Point to stsz atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSounStszAtom(FS_FILE* pFile, FS_i32 StszPosition)
{
    u32     size, entry, i;
    __align(4) MP4_SOUN_STSZ_ATOM mp4SounStszAtom;

    FS_FSeek(pFile, StszPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&mp4SounStszAtom, sizeof(MP4_SOUN_STSZ_ATOM), &size) == 0)
        return 0;
    mp4SounSampleEntryCount = bSwap32(mp4SounStszAtom.number_of_entries);
    
    if(mp4SounStszAtom.sample_size == 0) {
        for (i = 0; i < mp4SounSampleEntryCount; i++) {
            if (dcfRead(pFile, (unsigned char*)&entry, sizeof(MP4_SOUN_SAMPLE_SIZE_ENTRY), &size) == 0)
                return 0;
            mp4SounSampleSizeTable[i] = bSwap32(entry);
        }
    }

    return 1;
}

/*

Routine Description:

    Read SOUN STTS atom.

Arguments:

    pFile           - File handle.
    SttsPosition    - Point to stts atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSounSttsAtom(FS_FILE* pFile, FS_i32 SttsPosition)
{
    u32 size;
    __align(4) MP4_SOUN_STTS_ATOM mp4SounSttsAtom;

    FS_FSeek(pFile, SttsPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&mp4SounSttsAtom, sizeof(MP4_SOUN_STTS_ATOM), &size) == 0)
        return 0;
    mp4SounSampleEntryCount     = bSwap32(mp4SounSttsAtom.mp4_soun_sample_time_entry[0].sample_count);
    mp4SounSampleEntryDuration  = bSwap32(mp4SounSttsAtom.mp4_soun_sample_time_entry[0].sample_duration);

    return 1;
}

/*

Routine Description:

    Read SOUN STSC atom.

Arguments:

    pFile           - File handle.
    StscPosition    - Point to stsc atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSounStscAtom(FS_FILE* pFile, FS_i32 StscPosition)
{
    u32 size;
    __align(4) MP4_SOUN_STSC_ATOM mp4SounStscAtom;

    FS_FSeek(pFile, StscPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&mp4SounStscAtom, sizeof(MP4_SOUN_STSC_ATOM), &size) == 0)
        return 0;
    mp4SounSampleEntryCount = bSwap32(mp4SounStscAtom.mp4_soun_sample_to_chunk_entry[0].samples_per_chunk);

    return 1;
}

/*

Routine Description:

    Read SOUN STCO atom.

Arguments:

    pFile           - File handle.
    StcoPosition    - Point to stbl atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSounStcoAtom(FS_FILE* pFile, FS_i32 StcoPosition)
{
    u32 size, entry, i;
    __align(4) MP4_SOUN_STCO_ATOM mp4SounStcoAtom;

    FS_FSeek(pFile, StcoPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&mp4SounStcoAtom, sizeof(MP4_SOUN_STCO_ATOM), &size) == 0)
        return 0; 

    mp4SounSampleEntryCount = bSwap32(mp4SounStcoAtom.number_of_entries);

    for (i = 0; i < mp4SounSampleEntryCount; i++)
    {
        if (dcfRead(pFile, (unsigned char*)&entry, sizeof(MP4_SOUN_CHUNK_OFFSET_ENTRY), &size) == 0)
            return 0;
        mp4SounChunkOffsetTable[i]  = bSwap32(entry);
    }
    
    return 1;
}

/*

Routine Description:

    Read SOUN SMHD atom.

Arguments:

    pFile           - File handle.
    SmhdPosition    - Point to smhd atom file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSounSmhdAtom(FS_FILE* pFile, FS_i32 SmhdPosition)
{
    u32 size;
    __align(4) MP4_SOUN_SMHD_ATOM mp4SounSmhdAtom;

    FS_FSeek(pFile, SmhdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&mp4SounSmhdAtom, sizeof(MP4_SOUN_SMHD_ATOM), &size) == 0)
        return 0;

    return 1;
}

#endif  // #ifdef MOV_AUDIO

#endif  // #ifdef READ_ENABLE



#endif  // #if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MOV)
