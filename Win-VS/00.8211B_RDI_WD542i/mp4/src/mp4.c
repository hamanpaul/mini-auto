/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    mp4.c

Abstract:

    The routines of 3GPP/MP4 file.

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
#include "mp4.h"
#include "mp4api.h"
#include "mpeg4api.h"
#include "iisapi.h"
/* Peter: 0727 S*/
#include "isuapi.h"
#include "iduapi.h"
#include "ipuapi.h"
#include "siuapi.h"
#include "sysapi.h"
/* Peter: 0727 E*/
#if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MP4)
/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

/* define debug print */
#define mp4DebugPrint               printf
 
/* enable debugging / audio or not */ 
//#define MP4_AUDIO

/* max number of table entry */
#define MP4_VIDE_SAMPLE_MAX (30 * 60 * 10)

#define MP4_VIDE_INTRA_FRAME_INTERVAL (10) /* 10 frames */
#define MP4_VIDE_SYNC_SAMPLE_MAX (MP4_VIDE_SAMPLE_MAX / MP4_VIDE_INTRA_FRAME_INTERVAL + 1)

/* mh@2006/11/13: group specified numbers of sample into one chunk */
#define MP4_VIDE_SAMPLES_PER_CHUNK (30) 
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
u32 mp4FileOffset;

/* video related */
u8 mp4VideHeader[0x20];
u32 mp4VideHeaderSize;
u16 mp4VopWidth, mp4VopHeight;
u32 mp4VopCount, mp4IVopCount;
u32 mp4VideDuration;
u32 mp4VideStsdAtomSize;
MP4_VIDE_SAMPLE_SIZE_ENTRY mp4VideSampleSizeTable[MP4_VIDE_SAMPLE_MAX];
MP4_VIDE_SAMPLE_TIME_ENTRY mp4VideSampleTimeTable[MP4_VIDE_SAMPLE_MAX];  
MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY mp4VideSyncSampleNumberTable[MP4_VIDE_SYNC_SAMPLE_MAX];

/* mh@2006/11/14: group specified numbers of sample into one chunk */
u32 mp4ChunkOffsetTableEntryCount;
MP4_VIDE_CHUNK_OFFSET_ENTRY mp4VideChunkOffsetTable[MP4_VIDE_SAMPLE_MAX/MP4_VIDE_SAMPLES_PER_CHUNK + 1];
MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY mp4VideSampleToChunkTable[2];
/* mh@2006/11/14: END */
    
/* mh@2006/11/14: reduce the entry count of Time to Sample Box */
u32 mp4TimeToSampleTableEntryCount;
u32 mp4SampleToChunkTableEntryCount;
/* mh@2006/11/14: END */

/* audio related */
u32 mp4SounSampleEntryCount, mp4SounSampleEntryDuration;
MP4_AUDIO_FORMAT mp4AudioFormat;
MP4_SOUN_SAMPLE_SIZE_ENTRY mp4SounSampleSizeTable[IIS_SOUN_SAMPLE_MAX];
MP4_SOUN_CHUNK_OFFSET_ENTRY mp4SounChunkOffsetTable[IIS_SOUN_SAMPLE_MAX]; 

/* object description */
__align(4) u8 mp4ObjectDescription[0x0a] = 
{
    0x01, 0x08, 0x11, 0x06, 0x00, 0x9f, 0x0f, 0x02,
    0x00, 0x01
};

/* scene description */ 
__align(4) u8 mp4SceneDescription[0x0a] = 
{
    0xc0, 0x11, 0xa4, 0xcd, 0x54, 0x42, 0xa0, 0x14,
    0x41, 0x50
};

extern u32 VideoPictureIndex;
extern s32 isu_avifrmcnt;

/* Peter: 0727 S*/
/* playback related */
/* Peter: 0727 E*/

/* mh@2006/11/21: copied from avi.c to be compatible with Peter's new MPEG4 driver */
extern u32 siuSkipFrameRate;
/* mh@2006/11/21: END */


/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

s32 mp4CaptureVideoFile(s32);
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
s32 mp4WriteFtypBox(FS_FILE*);
s32 mp4WriteMdatBoxPre(FS_FILE*);
s32 mp4WriteMdatAtomData(FS_FILE*, u8*, u32);
s32 mp4WriteMdatAtomPost(FS_FILE*, u32);
s32 mp4WriteMoovBox(FS_FILE*);
s32 mp4WriteMvhdBox(FS_FILE*);
s32 mp4WriteIodsBox(FS_FILE*);
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
s32 mp4WriteVideTrakBox(FS_FILE*);
s32 mp4WriteVideTkhdBox(FS_FILE*);
/* Peter: 0727 S*/
s32 mp4WriteVideMdiaBox(FS_FILE*);
/* Peter: 0727 E*/
s32 mp4WriteVideMdhdBox(FS_FILE*);
s32 mp4WriteVideHdlrBox(FS_FILE*);
s32 mp4WriteVideMinfBox(FS_FILE*);
s32 mp4WriteVideDinfBox(FS_FILE*);
s32 mp4WriteVideDrefBox(FS_FILE*);
s32 mp4WriteVideStblBox(FS_FILE*);
s32 mp4WriteVideStsdBox(FS_FILE*);
s32 mp4WriteVideStszBox(FS_FILE*);
s32 mp4WriteVideSttsBox(FS_FILE*);
s32 mp4WriteVideStscBox(FS_FILE*);
s32 mp4WriteVideStcoBox(FS_FILE*);
s32 mp4WriteVideStssBox(FS_FILE*);
s32 mp4WriteVideVmhdBox(FS_FILE*); 
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
s32 mp4AudioInit(void);
s32 mp4WriteSounTrakAtom(FS_FILE*);
s32 mp4WriteSounTkhdAtom(FS_FILE*);
/* Peter: 0727 S*/
s32 mp4WriteSounMdiaAtom(FS_FILE*);
/* Peter: 0727 E*/
s32 mp4WriteSounMdhdAtom(FS_FILE*);
s32 mp4WriteSounHdlrAtom(FS_FILE*);
s32 mp4WriteSounMinfAtom(FS_FILE*);
s32 mp4WriteSounDinfAtom(FS_FILE*);
s32 mp4WriteSounDrefAtom(FS_FILE*);
s32 mp4WriteSounStblAtom(FS_FILE*);
s32 mp4WriteSounStsdAtom(FS_FILE*);
s32 mp4WriteSounStszAtom(FS_FILE*);
s32 mp4WriteSounSttsAtom(FS_FILE*);
s32 mp4WriteSounStscAtom(FS_FILE*);
s32 mp4WriteSounStcoAtom(FS_FILE*);
s32 mp4WriteSounSmhdAtom(FS_FILE*);
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

/* MP4 Object Descriptor Stream Track */
s32 mp4WriteOdsmTrakBox(FS_FILE*);
s32 mp4WriteOdsmTkhdBox(FS_FILE*);
/* Peter: 0727 S*/
s32 mp4WriteOdsmMdiaBox(FS_FILE*);
/* Peter: 0727 E*/
s32 mp4WriteOdsmMdhdBox(FS_FILE*);
s32 mp4WriteOdsmHdlrBox(FS_FILE*);
s32 mp4WriteOdsmMinfBox(FS_FILE*);
s32 mp4WriteOdsmDinfBox(FS_FILE*);
s32 mp4WriteOdsmDrefBox(FS_FILE*);
s32 mp4WriteOdsmStblBox(FS_FILE*);
s32 mp4WriteOdsmStsdBox(FS_FILE*);
s32 mp4WriteOdsmStszBox(FS_FILE*);
s32 mp4WriteOdsmSttsBox(FS_FILE*);
s32 mp4WriteOdsmStscBox(FS_FILE*);
s32 mp4WriteOdsmStcoBox(FS_FILE*);
s32 mp4WriteOdsmNmhdBox(FS_FILE*);
s32 mp4WriteOdsmTrefBox(FS_FILE*);
/* Peter: 0727 S*/
s32 mp4ReadOdsmMinfBox(FS_FILE*, FS_i32);
s32 mp4ReadOdsmDinfBox(FS_FILE*, FS_i32);
s32 mp4ReadOdsmDrefBox(FS_FILE*, FS_i32);
s32 mp4ReadOdsmStblBox(FS_FILE*, FS_i32);
s32 mp4ReadOdsmStsdBox(FS_FILE*, FS_i32);
s32 mp4ReadOdsmStszBox(FS_FILE*, FS_i32);
s32 mp4ReadOdsmSttsBox(FS_FILE*, FS_i32);
s32 mp4ReadOdsmStscBox(FS_FILE*, FS_i32);
s32 mp4ReadOdsmStcoBox(FS_FILE*, FS_i32);
s32 mp4ReadOdsmNmhdBox(FS_FILE*, FS_i32);
s32 mp4ReadOdsmTrefBox(FS_FILE*, FS_i32);
/* Peter: 0727 E*/

/* MP4 Scene Description Stream Track */
s32 mp4WriteSdsmTrakBox(FS_FILE*);
s32 mp4WriteSdsmTkhdBox(FS_FILE*);
/* Peter: 0727 S*/
s32 mp4WriteSdsmMdiaBox(FS_FILE*);
/* Peter: 0727 E*/
s32 mp4WriteSdsmMdhdBox(FS_FILE*);
s32 mp4WriteSdsmHdlrBox(FS_FILE*);
s32 mp4WriteSdsmMinfBox(FS_FILE*);
s32 mp4WriteSdsmDinfBox(FS_FILE*);
s32 mp4WriteSdsmDrefBox(FS_FILE*);
s32 mp4WriteSdsmStblBox(FS_FILE*);
s32 mp4WriteSdsmStsdBox(FS_FILE*);
s32 mp4WriteSdsmStszBox(FS_FILE*);
s32 mp4WriteSdsmSttsBox(FS_FILE*);
s32 mp4WriteSdsmStscBox(FS_FILE*);
s32 mp4WriteSdsmStcoBox(FS_FILE*);
s32 mp4WriteSdsmNmhdBox(FS_FILE*);
/* Peter: 0727 S*/
s32 mp4ReadSdsmMinfBox(FS_FILE*, FS_i32);
s32 mp4ReadSdsmDinfBox(FS_FILE*, FS_i32);
s32 mp4ReadSdsmDrefBox(FS_FILE*, FS_i32);
s32 mp4ReadSdsmStblBox(FS_FILE*, FS_i32);
s32 mp4ReadSdsmStsdBox(FS_FILE*, FS_i32);
s32 mp4ReadSdsmStszBox(FS_FILE*, FS_i32);
s32 mp4ReadSdsmSttsBox(FS_FILE*, FS_i32);
s32 mp4ReadSdsmStscBox(FS_FILE*, FS_i32);
s32 mp4ReadSdsmStcoBox(FS_FILE*, FS_i32);
s32 mp4ReadSdsmNmhdBox(FS_FILE*, FS_i32);
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
s32 mp4Init(void)
{
    /* initialize video */
    mpeg4Init();

#ifdef MP4_AUDIO    
    /* initialize audio */
    mp4AudioInit();
    
    /* initialize audio */
        iisInit();
#endif
    
    return 1;   
}

/*

Routine Description:

    The test routine of 3GPP/MP4 file.

Arguments:

    None.

Return Value:

    None.

*/
void mp4Test(void)
{
    
}

/*

Routine Description:

    Capture video.

Arguments:

    ZoomFactor - Zoom factor.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4CaptureVideo(s32 ZoomFactor) /*BJ 0530 S*/
{
    u32 i;
    
    /* video */
    VideoPictureIndex = 0;
    VideoBufMngReadIdx = 0;
    VideoBufMngWriteIdx = 0;
    mp4VideDuration = 0;
    mp4VopCount = 0;
    mp4ChunkOffsetTableEntryCount = 0;
    mp4TimeToSampleTableEntryCount = 0;
    mp4SampleToChunkTableEntryCount = 0;

    /* mh@2006/11/21: copied from avi.c to be compatible with Peter's new MPEG4 driver */
    siuSkipFrameRate        = 0;
    MPEG4_Mode              = 0;    // 0: record, 1: playback

    for(i = 0; i < VIDEO_BUF_NUM; i++) 
    {
        VideoBufMng[i].buffer   = VideoBuf;
    }
    /* mh@2006/11/21: END */
    
#ifdef MP4_AUDIO
    /* audio */
    mp4SounSampleEntryCount = 0;
    iisSounBufMngReadIdx = 0;
    iisSounBufMngWriteIdx = 0;
#endif

/*CY 0629 S*/
    /* write video file */
    if (mp4CaptureVideoFile(ZoomFactor) == 0)
    {
        /* reset the capture control if error */
        sysCaptureVideoStart = 0;
        sysCaptureVideoStop = 1;       
    }
/*CY 0629 E*/   
    
/*CY 0613 S*/
    /* delay until mpeg4 task reach pend state */
    OSTimeDly(1);
    
    /* suspend mpeg4 and iis task */
    mpeg4SuspendTask();

#ifdef MP4_AUDIO
    iisSuspendTask();
#endif  
/*CY 0613 E*/

    DEBUG_MP4("Trace: MP4 file captured - VOP count = %d.\n", mp4VopCount);
    
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
s32 mp4CaptureVideoFile(s32 ZoomFactor) /*BJ 0530 S*/
{
    FS_FILE* pFile;
    u16 value;
    u32 flag, size;
    u32 unCurrentSampleDuration;
    u32 unPreviousSampleDuration;
    u32 unSampleCount = 0;
    u8* pBuf;
	u8 tmp;
        
    /* create next file */
    if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_MP4, 0)) == NULL)
        return 0;
            
    /* write FTYP Box */
    if (mp4WriteFtypBox(pFile) == 0)
    {
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }        
    
    /* write MDAT Box pre*/
    if (mp4WriteMdatBoxPre(pFile) == 0)
    {
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }        

    /* file */  
    mp4FileOffset = sizeof(MP4_FTYP_BOX_T) + sizeof(MP4_MDAT_BOX_T);
    
#ifdef MP4_AUDIO    
    iisResumeTask();
#endif

    iduCaptureVideo(mpeg4Width,mpeg4Height);
    isuCaptureVideo(ZoomFactor);
    ipuCaptureVideo();
    siuCaptureVideo(ZoomFactor);
/*BJ 060929 S*/
#if ( (LCM_OPTION == LCM_HX8224) || (LCM_OPTION == LCM_HX5073_RGB) || (LCM_OPTION == LCM_HX5073_YUV) || (LCM_OPTION == LCM_TPG105) ||(LCM_OPTION ==LCM_A015AN04)|| (LCM_OPTION == LCM_TD020THEG1) || (LCM_OPTION == LCM_TD036THEA3_320x240) ||(LCM_OPTION == LCM_GPG48238QS4)||(LCM_OPTION == LCM_A024CN02)||(LCM_OPTION == LCM_TJ015NC02AA))
    SyncIduIsu();
#endif
/*BJ 060929 E*/
    while(isu_avifrmcnt < 1);

/* mh@2006/11/21: copied from avi.c to be compatible with Peter's new MPEG4 driver */

    /* refresh semaphore state */
    //Output_Sem();
    while(VideoCmpSemEvt->OSEventCnt > 0) {
        OSSemAccept(VideoCmpSemEvt);
    }
    //Output_Sem();
    while(VideoTrgSemEvt->OSEventCnt < (VIDEO_BUF_NUM - 2)) {
        OSSemPost(VideoTrgSemEvt);
    }
    //Output_Sem();

/* mh@2006/11/21: END */

    mpeg4ResumeTask();
    
    /*CY 0613 S*/
    while (sysCaptureVideoStop == 0)
    /*CY 0613 E*/
    {       
#ifdef MP4_AUDIO        
        value = OSSemAccept(iisCmpSemEvt);
        if (value > 0)
        {   
            size = iisSounBufMng[iisSounBufMngReadIdx].size;
            pBuf = iisSounBufMng[iisSounBufMngReadIdx].buffer;
                
            mp4SounChunkOffsetTable[mp4SounSampleEntryCount] = mp4FileOffset;
            mp4SounSampleSizeTable[mp4SounSampleEntryCount++] = size;
            mp4FileOffset += size;
                
            if (mp4WriteMdatAtomData(pFile, pBuf, size) == 0)
            {
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            }           
                
            iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % 2;
            
            //DEBUG_MP4("Trace: IIS frame written.\n");
            OSSemPost(iisTrgSemEvt);
        }
#endif
        
        value = OSSemAccept(VideoCmpSemEvt);
        if (value > 0)
        {
            flag = VideoBufMng[VideoBufMngReadIdx].flag;
            unCurrentSampleDuration = VideoBufMng[VideoBufMngReadIdx].time;
            size = VideoBufMng[VideoBufMngReadIdx].size;
            pBuf = VideoBufMng[VideoBufMngReadIdx].buffer;

            mp4VopCount++;
                
            if (flag & FLAG_I_VOP)
            {

                /* mh@2006/11/16: Perform byte swap here to save metadata-storing time */
                mp4VideSyncSampleNumberTable[mp4IVopCount++] = bSwap32(mp4VopCount);
                /* mh@2006/11/16: END */

/*CY 0629 S*/               
                if (mp4IVopCount > MP4_VIDE_SYNC_SAMPLE_MAX)
                {
                    dcfCloseFileByIdx(pFile, 0, &tmp);
                    DEBUG_MP4("Trace: Video IVOP count (%d) reaches limit.\n", mp4IVopCount); 
                    return 0;   
                }   
/*CY 0629 E*/                       
            }   

            /* mh@2006/11/16: Group specified numbers of sample into one chunk */
            if ((mp4VopCount % MP4_VIDE_SAMPLES_PER_CHUNK) == 1)
            {
                mp4VideChunkOffsetTable[mp4ChunkOffsetTableEntryCount++] = bSwap32(mp4FileOffset);
            }
            /* mh@2006/11/16: END */

            /* mh@2006/11/16: reduce the entry count of Decoding Time to Sample Box */
            if (mp4VopCount == 1) // 1st frame
            {
                unPreviousSampleDuration = unCurrentSampleDuration;
                unSampleCount = 1;
            }
            else
            {
                if (unCurrentSampleDuration != unPreviousSampleDuration)
                {
                    mp4VideSampleTimeTable[mp4TimeToSampleTableEntryCount].sample_duration = bSwap32(unPreviousSampleDuration);
                    mp4VideSampleTimeTable[mp4TimeToSampleTableEntryCount++].sample_count = bSwap32(unSampleCount);
                    unSampleCount = 1;
                    unPreviousSampleDuration = unCurrentSampleDuration;
                }
                else
                {
                    unSampleCount++;
                }
            }
            /* mh@2006/11/16: END */
            
            mp4VideDuration += unCurrentSampleDuration;

            mp4VideSampleSizeTable[mp4VopCount-1] = bSwap32(size);
            mp4FileOffset += size;
            
/*CY 0629 S*/               
            if (mp4VopCount >= MP4_VIDE_SAMPLE_MAX)
            {
                /*dcfClose(pFile);*/
                sysCaptureVideoStop = 1;
                DEBUG_MP4("Trace: Video VOP count (%d) reaches limit.\n", mp4VopCount); 
                /*return 0;*/   
            }   
/*CY 0629 E*/       

            if (mp4WriteMdatAtomData(pFile, pBuf, size) == 0)
            {
                dcfCloseFileByIdx(pFile, 0, &tmp);
                return 0;
            }                   

/* mh@2006/11/21: replace the ping-pong buffer by circular buffer */
#if 0               
            VideoBufMngReadIdx = (VideoBufMngReadIdx + 1) % 2;
#else
            VideoBufMngReadIdx = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
#endif
/* mh@2006/11/21: END */

            //DEBUG_MP4("Trace: MPEG4 frame written.\n");
            OSSemPost(VideoTrgSemEvt);  
        }
    }

    /* mh@2006/11/16: reduce the entry count of Decoding Time to Sample Box (last entry) */
    mp4VideSampleTimeTable[mp4TimeToSampleTableEntryCount].sample_duration = bSwap32(unPreviousSampleDuration);
    mp4VideSampleTimeTable[mp4TimeToSampleTableEntryCount++].sample_count = bSwap32(unSampleCount);

    if ((mp4VopCount % MP4_VIDE_SAMPLES_PER_CHUNK) == 0 )
        mp4SampleToChunkTableEntryCount = 1;
    else
        mp4SampleToChunkTableEntryCount = 2;
    /* mh@2006/11/16: END */

    /* write MDAT atom post */
    if (mp4WriteMdatAtomPost(pFile, mp4FileOffset - sizeof(MP4_FTYP_BOX_T)) == 00)
    {
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }               
    
    /* write MOOV Box */
    if (mp4WriteMoovBox(pFile) == 0)
    {
        dcfCloseFileByIdx(pFile, 0, &tmp);
        return 0;
    }        
    
    /* close file */
    dcfCloseFileByIdx(pFile, 0, &tmp);

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
s32 mp4SetVideoResolution(u16 width, u16 height)
{
    mp4VopWidth = width;    /* cytsai: 0418 */
    mp4VopHeight = height;
    //mp4VopWidth = 352;    /* cytsai: for armulator only */ 
    //mp4VopHeight = 288;   /* cytsai: for armulator only */
    
    mpeg4SetVideoResolution(width, height);
    
    return  1;
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
s32 mp4GetVisualSampleLocation(u32 unCurrentVisualSampleNumber, FS_i32* pCurrentVisualSampleLocation)
{
    u32 unChunkOffsetTableIndex;
    u32 unPrioriSampleCountWithinChunk;
    u32 unfirstSampleSizeTableIndexWithinChunk;
    u32 i;

    // step 1. calculate the chunk offset table index
    unChunkOffsetTableIndex = unCurrentVisualSampleNumber / MP4_VIDE_SAMPLES_PER_CHUNK;

    // step 2. calculate the priori sample count within the chunk
    unPrioriSampleCountWithinChunk = unCurrentVisualSampleNumber % MP4_VIDE_SAMPLES_PER_CHUNK;

    // step 3. calculate the first sample size table index within the chunk
    unfirstSampleSizeTableIndexWithinChunk = unChunkOffsetTableIndex * MP4_VIDE_SAMPLES_PER_CHUNK;

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
s32 mp4ReadFile(void)
{
    FS_FILE* pFile;
/* Peter: 0727 S*/
    FS_i32      FtypPosition, MoovPosition, MdatPosition;
    u32         size, i;
    s32         err;
	u8			tmp;
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
    mp4VideDuration         = 0;
    mp4VopCount             = 0;
    MainVideodisplaybuf_idx     = 0;
    
#ifdef MP4_AUDIO
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
            
        err = dcfSeek(pFile, nCurrentVisualSampleLocation, SEEK_SET);   // goto next LIST

        if (err==0) return 0;

        err = dcfRead(pFile, (u8*)VideoBufMng[0].buffer, mp4VideSampleSizeTable[i], &size);

        if(err == 0)    return 0;

        mpeg4DecodeVOP(VideoBufMng[0].buffer, mp4VideSampleSizeTable[i],0,0);

#ifdef  MP4_AUDIO
        err = dcfSeek(pFile, mp4SounChunkOffsetTable[i], SEEK_SET);   // goto next LIST
        if(err==0) return 0;
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
s32 mp4WriteFtypBox(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MP4_FTYP_BOX_T tMp4FtypAtom = 
    {
        0x18000000,     /* size */
        0x70797466,     /* type = "ftyp" */
        0x414d4948,     /* major_brand = "HIMA" */
        0x00000000,     /* minor_version */
        0x6d6f7369,     /* compatible_brands[2] = "isom" "mp42" */ 
        0x3234706d,     
    };  
    
    tMp4FtypAtom.size = bSwap32((u32)sizeof(MP4_FTYP_BOX_T)); 

    if (dcfWrite(pFile, (unsigned char*)&tMp4FtypAtom, sizeof(MP4_FTYP_BOX_T), &unSize) == 0)
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
s32 mp4WriteMdatBoxPre(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MP4_MDAT_BOX_T tMp4MdatVox =
    {
        0xffffffff,     /* size = TBD */
        0x7461646d,     /* type = "mdat" */
    };
    
    if (dcfWrite(pFile, (unsigned char*)&tMp4MdatVox, sizeof(MP4_MDAT_BOX_T), &unSize) == 0)
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
s32 mp4WriteMdatAtomData(FS_FILE* pFile, u8* pBuf, u32 dataSize)
{
    u32 size;       
    
    if (dcfWrite(pFile, (unsigned char*)pBuf, dataSize, &size) == 0)
            return 0;
    
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
s32 mp4WriteMdatAtomPost(FS_FILE* pFile, u32 atomSize)
{
    u32 offset;
    u32 size;       
    
    if (dcfWrite(pFile, (unsigned char*)&mp4ObjectDescription, sizeof(mp4ObjectDescription), &size) == 0)
            return 0;
    if (dcfWrite(pFile, (unsigned char*)&mp4SceneDescription, sizeof(mp4SceneDescription), &size) == 0)
            return 0;
    atomSize += sizeof(mp4ObjectDescription) + sizeof(mp4SceneDescription);
    offset = dcfTell(pFile);
    dcfSeek(pFile, 0x18, SEEK_SET);
    atomSize = bSwap32(atomSize); 
    if (dcfWrite(pFile, (unsigned char*)&atomSize, sizeof(u32), &size) == 0)
            return 0;
    dcfSeek(pFile, offset, SEEK_SET); 
    
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
s32 mp4WriteMoovBox(FS_FILE* pFile)
{
    u32 unMp4MoovBoxSize;
    u32 unSize;
    
    __align(4) MP4_MOOV_BOX_T tMp4MoovBox =
    {
        0x00000000, /* size */
        0x766f6f6d, /* type = "moov" */
    };

    mpeg4EncodeVolHeader(mp4VideHeader, &mp4VideHeaderSize);   /* Peter: 0711 */
    mp4VideStsdAtomSize =
        sizeof(MP4_VIDE_STSD_BOX_T) +
        sizeof(MP4_VIDE_VISUAL_SAMPLE_ENTRY) +
        sizeof(MP4_VIDE_ESDS_BOX_T) +
        sizeof(MP4_VIDE_ES_DESCRIPTOR) +
        sizeof(MP4_VIDE_DECODER_CONFIG_DESCRIPTOR) +
        sizeof(MP4_VIDE_DECODER_SPECIFIC_INFO) +  mp4VideHeaderSize +
        sizeof(MP4_VIDE_SL_CONFIG_DESCRIPTOR);
    
    unMp4MoovBoxSize =  
        sizeof(MP4_MOOV_BOX_T) +
        sizeof(MP4_MVHD_BOX_T) +
        sizeof(MP4_IODS_BOX_T) +
        sizeof(MP4_VIDE_TRAK_BOX_T) +
        sizeof(MP4_VIDE_TKHD_BOX_T) + 
/* Peter: 0727 S*/
        sizeof(MP4_VIDE_MDIA_BOX_T) +
/* Peter: 0727 E*/
        sizeof(MP4_VIDE_MDHD_BOX_T) +
        sizeof(MP4_VIDE_HDLR_BOX_T) +
        sizeof(MP4_VIDE_MINF_BOX_T) +
        sizeof(MP4_VIDE_VMHD_BOX_T) + 
        sizeof(MP4_VIDE_DINF_BOX_T) +
        sizeof(MP4_VIDE_DREF_BOX_T) +
        sizeof(MP4_VIDE_STBL_BOX_T) +
        mp4VideStsdAtomSize +
        sizeof(MP4_VIDE_STTS_BOX_T) + mp4TimeToSampleTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TIME_ENTRY) +
        sizeof(MP4_VIDE_STSC_BOX_T) + mp4SampleToChunkTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY) +
        sizeof(MP4_VIDE_STSZ_BOX_T) + mp4VopCount * sizeof(MP4_VIDE_SAMPLE_SIZE_ENTRY) +        
        sizeof(MP4_VIDE_STCO_BOX_T) + mp4ChunkOffsetTableEntryCount * sizeof(MP4_VIDE_CHUNK_OFFSET_ENTRY) +
        sizeof(MP4_VIDE_STSS_BOX_T) + mp4IVopCount * sizeof(MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY) +
        
#ifdef MP4_AUDIO
        sizeof(MP4_SOUN_TRAK_ATOM) +    /* soun trak */
        sizeof(MP4_SOUN_TKHD_ATOM) + 
/* Peter: 0727 S*/
        sizeof(MP4_SOUN_MDIA_ATOM) +
/* Peter: 0727 E*/
        sizeof(MP4_SOUN_MDHD_ATOM) +
        sizeof(MP4_SOUN_HDLR_ATOM) +
        sizeof(MP4_SOUN_MINF_ATOM) +
        sizeof(MP4_SOUN_DINF_ATOM) +
        sizeof(MP4_SOUN_DREF_ATOM) +
        sizeof(MP4_SOUN_STBL_ATOM) +
        sizeof(MP4_SOUN_STSD_ATOM) +
        sizeof(MP4_SOUN_STSZ_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_SAMPLE_SIZE_ENTRY) +
        sizeof(MP4_SOUN_STTS_ATOM) +
        sizeof(MP4_SOUN_STSC_ATOM) +
        sizeof(MP4_SOUN_STCO_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_CHUNK_OFFSET_ENTRY) +
        sizeof(MP4_SOUN_SMHD_ATOM) +
#endif          
        sizeof(MP4_ODSM_TRAK_BOX_T) +   /* odsm trak */
        sizeof(MP4_ODSM_TKHD_BOX_T) + 
        sizeof(MP4_ODSM_TREF_BOX_T) + 
/* Peter: 0727 S*/
        sizeof(MP4_ODSM_MDIA_BOX_T) +
/* Peter: 0727 E*/
        sizeof(MP4_ODSM_MDHD_BOX_T) +
        sizeof(MP4_ODSM_HDLR_BOX_T) +
        sizeof(MP4_ODSM_MINF_BOX_T) +
        sizeof(MP4_ODSM_NMHD_BOX_T) +
        sizeof(MP4_ODSM_DINF_BOX_T) +
        sizeof(MP4_ODSM_DREF_BOX_T) +
        sizeof(MP4_ODSM_STBL_BOX_T) +
        sizeof(MP4_ODSM_STSD_BOX_T) +
        sizeof(MP4_ODSM_STTS_BOX_T) +
        sizeof(MP4_ODSM_STSC_BOX_T) +
        sizeof(MP4_ODSM_STSZ_BOX_T) +
        sizeof(MP4_ODSM_STCO_BOX_T) +
        sizeof(MP4_SDSM_TRAK_BOX_T) +   /* sdsm trak */
        sizeof(MP4_SDSM_TKHD_BOX_T) + 
/* Peter: 0727 S*/
        sizeof(MP4_SDSM_MDIA_BOX_T) +
/* Peter: 0727 E*/
        sizeof(MP4_SDSM_MDHD_BOX_T) +
        sizeof(MP4_SDSM_HDLR_BOX_T) +
        sizeof(MP4_SDSM_MINF_BOX_T) +
        sizeof(MP4_SDSM_NMHD_BOX_T) +
        sizeof(MP4_SDSM_DINF_BOX_T) +
        sizeof(MP4_SDSM_DREF_BOX_T) +
        sizeof(MP4_SDSM_STBL_BOX_T) +
        sizeof(MP4_SDSM_STSD_BOX_T) +
        sizeof(MP4_SDSM_STTS_BOX_T) +
        sizeof(MP4_SDSM_STSC_BOX_T) +
        sizeof(MP4_SDSM_STSZ_BOX_T) +
        sizeof(MP4_SDSM_STCO_BOX_T);
    
    tMp4MoovBox.size = bSwap32(unMp4MoovBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMp4MoovBox, sizeof(MP4_MOOV_BOX_T), &unSize) == 0)
            return 0;

    if (mp4WriteMvhdBox(pFile) == 0)
            return 0;

    if (mp4WriteIodsBox(pFile) == 0)
            return 0;
    
    if (mp4WriteVideTrakBox(pFile) == 0)
            return 0;

#ifdef MP4_AUDIO
    if (mp4WriteSounTrakAtom(pFile) == 0)
            return 0;
#endif

    if (mp4WriteOdsmTrakBox(pFile) == 0)
            return 0;
    
    if (mp4WriteSdsmTrakBox(pFile) == 0)
            return 0;
    
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
s32 mp4WriteMvhdBox(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MP4_MVHD_BOX_T tMp4MvhdBox =
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
        0x00000000, 0x00000000, 0x00000040, /*           0.0 0.0 1.0 (in 2.30) */
        0x00000000,         /* pre_defined[0] */
        0x00000000,         /* pre_defined[1] */        
        0x00000000,         /* pre_defined[2] */     
        0x00000000,         /* pre_defined[3] */
        0x00000000,         /* pre_defined[4] */ 
        0x00000000,         /* pre_defined[5] */ 
        0x04000000,         /* next_track_id */
    };
    
    tMp4MvhdBox.size = bSwap32(sizeof(MP4_MVHD_BOX_T));
    tMp4MvhdBox.duration = bSwap32(mp4VideDuration);

    if (dcfWrite(pFile, (unsigned char*)&tMp4MvhdBox, sizeof(MP4_MVHD_BOX_T), &unSize) == 0)
            return 0;

    return 1;   
}

/*

Routine Description:

    Write IODS Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] 1. Initial Object Descriptor Box 'iods' contains stream specific
          information.
       2. Not defined in ISO14496-12
       3. Even without it, the movie file still can be played successfully
          by MediaPlayer and QuickTime

*/
s32 mp4WriteIodsBox(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MP4_IODS_BOX_T tMp4IodsBox =
    {
        0x21000000,         /* size */
        0x73646f69,         /* type = "iods" */
        0x00000000,         /* version_flags */
        {
            0x10,               /* MP4_IOD_Tag */
            0x13,               /* Size of the following field */
            0x4f00,             /* ObjectDescriptorID (bit15~6) = 1 
                                   URL_flag (bit5) = 0
                                   IncludeInlineProfileLevelFlag (bit4) = 0
                                   bit(3~0): reserved = 0b1111 */
            0xfe,               /* ODProfileLevelIndication = 0xFE:
                                   no OD profile specified */
            0x01,               /* sceneProfileLevelIndication = 0x01:
                                   Simple2D profile */
            0x02,               /* audioProfileLevelIndication = 0x02:
                                   Main Profile (?) */
            0x01,               /* visualProfileLevelIndication = 0x01:
                                   Simple Profile (Level 3) */
            0xfe,               /* graphicsProfileLevelIndication = 0xFE:
                                   no graphics profile specified */
            0x0e,               /* ES_ID_Inc */
            0x04,               /* odsm_track_id_length */
            0x01000000,         /* odsm_track_id */
            0x0e,               /* ES_ID_Inc */
            0x04,               /* sdsm_trck_id_length */
            0x02000000,         /* sdsm_track_id */
        },
    };

    tMp4IodsBox.size = bSwap32(sizeof(MP4_IODS_BOX_T));

    if (dcfWrite(pFile, (unsigned char*)&tMp4IodsBox, sizeof(MP4_IODS_BOX_T), &unSize) == 0)
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
s32 mp4WriteVideTrakBox(FS_FILE* pFile)
{
    u32 unMp4VideTrakBoxSize;
    u32 unSize;
    
    __align(4) MP4_VIDE_TRAK_BOX_T tMp4VideTrakBox = 
    {
        0x00000000,         /* size */
        0x6b617274,         /* type = "trak */
    };

    unMp4VideTrakBoxSize =  sizeof(MP4_VIDE_TRAK_BOX_T) +   /* vide trak */
        sizeof(MP4_VIDE_TKHD_BOX_T) + 
/* Peter: 0727 S*/
        sizeof(MP4_VIDE_MDIA_BOX_T) +
/* Peter: 0727 E*/
        sizeof(MP4_VIDE_MDHD_BOX_T) +
        sizeof(MP4_VIDE_HDLR_BOX_T) +
        sizeof(MP4_VIDE_MINF_BOX_T) +
        sizeof(MP4_VIDE_VMHD_BOX_T) +
        sizeof(MP4_VIDE_DINF_BOX_T) +
        sizeof(MP4_VIDE_DREF_BOX_T) +
        sizeof(MP4_VIDE_STBL_BOX_T) +
        mp4VideStsdAtomSize +
        sizeof(MP4_VIDE_STTS_BOX_T) + mp4TimeToSampleTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TIME_ENTRY) +
        sizeof(MP4_VIDE_STSC_BOX_T) + mp4SampleToChunkTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY) +
        sizeof(MP4_VIDE_STSZ_BOX_T) + mp4VopCount * sizeof(MP4_VIDE_SAMPLE_SIZE_ENTRY) +
        sizeof(MP4_VIDE_STCO_BOX_T) + mp4ChunkOffsetTableEntryCount * sizeof(MP4_VIDE_CHUNK_OFFSET_ENTRY) +
        sizeof(MP4_VIDE_STSS_BOX_T) + mp4IVopCount * sizeof(MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY); 

    tMp4VideTrakBox.size = bSwap32(unMp4VideTrakBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideTrakBox, sizeof(MP4_VIDE_TRAK_BOX_T), &unSize) == 0)
            return 0;

    if (mp4WriteVideTkhdBox(pFile) == 0)
            return 0;
    
/* Peter: 0727 S*/
    if (mp4WriteVideMdiaBox(pFile) == 0)
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
s32 mp4WriteVideTkhdBox(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MP4_VIDE_TKHD_BOX_T tMp4VideTkhdBox = 
    {
        0x5c000000,         /* size */
        0x64686b74,         /* type = "tkhd" */
        0x01000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time= 00:00:00 2004/01/01 */
        0x03000000,         /* track_id */
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
        0x00000000, 0x00000000, 0x00000040, /*           0.0 0.0 1.0 */
        0x00000000,         /* track_width = 0x???????? */
        0x00000000,         /* track_height = 0x???????? */
    };

    tMp4VideTkhdBox.size = bSwap32(sizeof(MP4_VIDE_TKHD_BOX_T));

    tMp4VideTkhdBox.duration = bSwap32(mp4VideDuration);

    tMp4VideTkhdBox.track_width = bSwap32((u32)mp4VopWidth << 16);

    tMp4VideTkhdBox.track_height = bSwap32((u32)mp4VopHeight << 16);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideTkhdBox, sizeof(MP4_VIDE_TKHD_BOX_T), &unSize) == 0)
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
s32 mp4WriteVideMdiaBox(FS_FILE* pFile)
/* Peter: 0727 E*/
{
    u32 unMp4VideMdiaBoxSize;
    u32 unSize;
/* Peter: 0727 S*/

    __align(4) MP4_VIDE_MDIA_BOX_T tMp4VideMdiaBox =
    {
        0x00000000,         /* size */
        0x6169646d,         /* type = "mdia" */
    };

    unMp4VideMdiaBoxSize =  sizeof(MP4_VIDE_MDIA_BOX_T) +

        sizeof(MP4_VIDE_MDHD_BOX_T) +
        sizeof(MP4_VIDE_HDLR_BOX_T) +
        sizeof(MP4_VIDE_MINF_BOX_T) +
        sizeof(MP4_VIDE_VMHD_BOX_T) +
        sizeof(MP4_VIDE_DINF_BOX_T) +
        sizeof(MP4_VIDE_DREF_BOX_T) +
        sizeof(MP4_VIDE_STBL_BOX_T) +
        mp4VideStsdAtomSize +
        sizeof(MP4_VIDE_STTS_BOX_T) + mp4TimeToSampleTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TIME_ENTRY) +
        sizeof(MP4_VIDE_STSC_BOX_T) + mp4SampleToChunkTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY) +
        sizeof(MP4_VIDE_STSZ_BOX_T) + mp4VopCount * sizeof(MP4_VIDE_SAMPLE_SIZE_ENTRY) +
        sizeof(MP4_VIDE_STCO_BOX_T) + mp4ChunkOffsetTableEntryCount * sizeof(MP4_VIDE_CHUNK_OFFSET_ENTRY) +
        sizeof(MP4_VIDE_STSS_BOX_T) + mp4IVopCount * sizeof(MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY);

    tMp4VideMdiaBox.size = bSwap32(unMp4VideMdiaBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideMdiaBox, sizeof(MP4_VIDE_MDIA_BOX_T), &unSize) == 0)
            return 0;
/* Peter: 0727 E*/

    if (mp4WriteVideMdhdBox(pFile) == 0)
            return 0;

    if (mp4WriteVideHdlrBox(pFile) == 0)
            return 0;

    if (mp4WriteVideMinfBox(pFile) == 0)
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
s32 mp4WriteVideMdhdBox(FS_FILE* pFile)
{
    u32 unSize;
    
    __align(4) MP4_VIDE_MDHD_BOX_T tMp4VideMdhdBox = 
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

    tMp4VideMdhdBox.size = bSwap32(sizeof(MP4_VIDE_MDHD_BOX_T));

    tMp4VideMdhdBox.duration = bSwap32(mp4VideDuration);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideMdhdBox, sizeof(MP4_VIDE_MDHD_BOX_T), &unSize) == 0)
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
s32 mp4WriteVideHdlrBox(FS_FILE* pFile)
{
    u32 unSize;

    __align(4) MP4_VIDE_HDLR_BOX_T tMp4VideHdlrBox =
    {
        0x2d000000,             /* size */
        0x726c6468,             /* type = "hdlr" */
        0x00000000,             /* version_flags */
        0x00000000,             /* pre_defined */
        0x65646976,             /* handler_type = "vide" (video) */
        0x00000000,             /* reserved[0] */
        0x00000000,             /* reserved[1] */
        0x00000000,             /* reserved[2] */
        0x56, 0x69, 0x73, 0x75, /* name[13] = "VisualStream\0" */
        0x61, 0x6C, 0x53, 0x74,
        0x72, 0x65, 0x61, 0x6D,
        0x00, 
    };

    tMp4VideHdlrBox.size = bSwap32(sizeof(MP4_VIDE_HDLR_BOX_T));

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideHdlrBox, sizeof(MP4_VIDE_HDLR_BOX_T), &unSize) == 0)
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

s32 mp4WriteVideMinfBox(FS_FILE* pFile)
{
    u32 unMp4VideMinfBoxSize;
    u32 unSize;
    
    __align(4) MP4_VIDE_MINF_BOX_T tMp4VideMinfBox =
    {
        0x00000000,     /* size */
        0x666e696d,     /* type = "minf" */
    };

    unMp4VideMinfBoxSize =  sizeof(MP4_VIDE_MINF_BOX_T) +
            sizeof(MP4_VIDE_VMHD_BOX_T) +
            sizeof(MP4_VIDE_DINF_BOX_T) +
            sizeof(MP4_VIDE_DREF_BOX_T) +
            sizeof(MP4_VIDE_STBL_BOX_T) +
            mp4VideStsdAtomSize +
            sizeof(MP4_VIDE_STSZ_BOX_T) + mp4VopCount * sizeof(MP4_VIDE_SAMPLE_SIZE_ENTRY) +
            sizeof(MP4_VIDE_STTS_BOX_T) + mp4TimeToSampleTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TIME_ENTRY) +
            sizeof(MP4_VIDE_STSC_BOX_T) + mp4SampleToChunkTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY) +
            sizeof(MP4_VIDE_STCO_BOX_T) + mp4ChunkOffsetTableEntryCount * sizeof(MP4_VIDE_CHUNK_OFFSET_ENTRY) +
            sizeof(MP4_VIDE_STSS_BOX_T) + mp4IVopCount * sizeof(MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY);

    tMp4VideMinfBox.size = bSwap32(unMp4VideMinfBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideMinfBox, sizeof(MP4_VIDE_MINF_BOX_T), &unSize) == 0)
            return 0;

    if (mp4WriteVideVmhdBox(pFile) == 0)
            return 0;

    if (mp4WriteVideDinfBox(pFile) == 0)
            return 0;

    if (mp4WriteVideStblBox(pFile) == 0)
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
s32 mp4WriteVideVmhdBox(FS_FILE* pFile) 
{
    u32 unSize;
    
    __align(4) MP4_VIDE_VMHD_BOX_T tMp4VideVmhdBox =
    {
        0x14000000,         /* size */
        0x64686d76,         /* type = "vmhd" */
        0x01000000,         /* version_flags = no lean ahead */
        0x0000,             /* graphics_mode = copy */
        0x0000,             /* opcolor[3] */ 
        0x0000,
        0x0000,
    };

    tMp4VideVmhdBox.size = bSwap32(sizeof(MP4_VIDE_VMHD_BOX_T));

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideVmhdBox, sizeof(MP4_VIDE_VMHD_BOX_T), &unSize) == 0)
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
s32 mp4WriteVideDinfBox(FS_FILE* pFile)
{
    u32 unMp4VideDinfBoxSize;
    u32 unSize;

    __align(4) MP4_VIDE_DINF_BOX_T tMp4VideDinfBox =
    {
        0x24000000,         /* size */
        0x666e6964,         /* type = "dinf" */
    };

    unMp4VideDinfBoxSize =  sizeof(MP4_VIDE_DINF_BOX_T) +
                            sizeof(MP4_VIDE_DREF_BOX_T);
    
    tMp4VideDinfBox.size = bSwap32(unMp4VideDinfBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideDinfBox, sizeof(MP4_VIDE_DINF_BOX_T), &unSize) == 0)
            return 0;

    if (mp4WriteVideDrefBox(pFile) == 0)
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
s32 mp4WriteVideDrefBox(FS_FILE* pFile)
{
    u32 unSize;

    __align(4) MP4_VIDE_DREF_BOX_T tMp4VideDrefBox =
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

    tMp4VideDrefBox.size = bSwap32(sizeof(MP4_VIDE_DREF_BOX_T));
    
    if (dcfWrite(pFile, (unsigned char*)&tMp4VideDrefBox, sizeof(MP4_VIDE_DREF_BOX_T), &unSize) == 0)
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
s32 mp4WriteVideStblBox(FS_FILE* pFile)
{
    u32 unMp4VideStblBoxSize;
    u32 unSize;

    __align(4) MP4_VIDE_STBL_BOX_T tMp4VideStblBox =
    {
        0x00000000,     /* size */
        0x6c627473,     /* type = "stbl" */
    };

    unMp4VideStblBoxSize =  sizeof(MP4_VIDE_STBL_BOX_T) +
        mp4VideStsdAtomSize +
        sizeof(MP4_VIDE_STSZ_BOX_T) + mp4VopCount * sizeof(MP4_VIDE_SAMPLE_SIZE_ENTRY) +
        sizeof(MP4_VIDE_STTS_BOX_T) + mp4TimeToSampleTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TIME_ENTRY) +
        sizeof(MP4_VIDE_STSC_BOX_T) + mp4SampleToChunkTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY) +
        sizeof(MP4_VIDE_STCO_BOX_T) + mp4ChunkOffsetTableEntryCount * sizeof(MP4_VIDE_CHUNK_OFFSET_ENTRY) +
        sizeof(MP4_VIDE_STSS_BOX_T) + mp4IVopCount * sizeof(MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY);

    tMp4VideStblBox.size = bSwap32(unMp4VideStblBoxSize);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideStblBox, sizeof(MP4_VIDE_STBL_BOX_T), &unSize) == 0)
            return 0;

    if (mp4WriteVideStsdBox(pFile) == 0)
            return 0;

    if (mp4WriteVideSttsBox(pFile) == 0)
            return 0;

    if (mp4WriteVideStscBox(pFile) == 0)
            return 0;

    if (mp4WriteVideStszBox(pFile) == 0)
            return 0;
        
    if (mp4WriteVideStcoBox(pFile) == 0)
            return 0;

    if (mp4WriteVideStssBox(pFile) == 0)
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
s32 mp4WriteVideStsdBox(FS_FILE* pFile)
{
    u32 size;
    
    __align(4) MP4_VIDE_STSD_BOX_T tMp4VideStsdBox =
    {
        0xa8000000,         /* size */
        0x64737473,         /* type = "stsd" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        /* mp4VideVisualSampleEntry */
    };
    
    __align(4) MP4_VIDE_VISUAL_SAMPLE_ENTRY mp4VideVisualSampleEntry =
    {
            0x98000000,         /* sample_description_size */
            0x7634706d,         /* data_format = "mp4v" */
            0x00, 0x00, 0x00, 0x00,     /* reserved[6] */       
            0x00, 0x00,
            0x0100,             /* data_reference_index = data_reference of this track */
            0x0000,             /* pre_defined1 */
            0x0000,             /* reserved1 */
            0x00000000,         /* pre_defined2[0] */
            0x00000000,         /* pre_defined2[1] */
            0x00000000,         /* pre_defined2[2] */
            0x00000000,         /* width_height = 0x???????? */
            0x00004800,         /* horz_res = 72 dpi */
            0x00004800,         /* vert_res = 72 dpi */
            0x00000000,         /* reserved2 */
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

    __align(4) MP4_VIDE_ESDS_BOX_T tMp4VideEsdsBox =
    {
                0x42000000,         /* size */
                0x73647365,         /* type = "esds" */
                0x00000000,         /* version_flags */
                /* mp4VideEsDescriptor */
                /* mp4VideDecoderConfigDescriptor */
                /* mp4VideDecoderSpecificInfo */
                /* mp4VideSlConfigDescriptor */
    };

    __align(4) MP4_VIDE_ES_DESCRIPTOR mp4VideEsDescriptor =
    {
                    0x03,               /* ES_DescrTag (0x03) */
                    0x34,               /* Total length from ES_ID to the
                                           end of ES_Descriptor */
                    0x0300,             /* ES_ID = 3 */
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
    __align(4) MP4_VIDE_DECODER_CONFIG_DESCRIPTOR mp4VideDecoderConfigDescriptor =
    {                                   
                        0x04,           /* DecoderConfigDescrTag (0x04) */
                        0x2c,           /* Total length from objectTypeIndication
                                           to the end of DecoderConfigDescriptor */
                        0x20,           /* objectTypeIndication = 0x20
                                           ==> Visual ISO 14496-2 */
                        0x00280211,     /* streamType (bit7-bit2) = 4 
                                           ==> Visual Stream, 
                                           upStream (bit1) = 0
                                           ==> not upstream,
                                           reserved (bit0) = 1,
                                           bufferSizeDB (byte3-byte0):
                                           Size of decoding buffer */
                        0xd8751900,     /* dcd_tag_max_bitrate */
                        0xe0940b00,     /* dcd_tag_avg_bitrate */
                        /* mp4VideDecoderSpecificInfo */
    };

    __align(4) MP4_VIDE_DECODER_SPECIFIC_INFO mp4VideDecoderSpecificInfo = 
    {
                            0x05,               /* DecSpecificInfoTag (0x05) */
                            0x1d,               /* Total length from next byte
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
#if 0   /* mh@2006/10/20: for furture work */                                                  
                            0x00, 0x00, 0x01, 0x00, /* video_object_start_code */    
                            0x00, 0x00, 0x01, 0x20, /* video_object_layer_start_code */
                            0x00,                   /* [] random_accessible_vol (bit7) = 0
                                                       [] video_object_type_indication (bit6-bit0) = 0b0000000 */
                            0xc4,                   /* [] video_object_type_indication (bit7) = 1,
                                                       [] is_object_layer_identifier (bit6) = 1,
                                                       [] video_object_layer_verid (bit5-bit2) = 0001,
                                                       [] video_object_layer_priority (bit1-bit0) = 00 */
                            0x88,                   /* [] video_object_layer_priority (bit7) = 1,
                                                       [] aspect_ratio_info (bit6-bit3) = 0001
                                                          ==> 1:1,
                                                       [] vol_control_parameters (bit2) = 0,
                                                       [] video_object_layer_shape (bit1-bit0) = 00 
                                                          ==> rectangular */
                            0x81,                   /* [] marker_bit (bit7) = 1,
                                                          vop_time_increment_resolution (16bit)
                                                          = 0b 000 0001 1111 0100 0
                                                          = 1000 */
                            0xf4, 

                            0x50,                   /* [] marker_bit (bit6) = 1,
                                                       [] fixed_vop_rate (bit5) = 0,
                                                       [] marker_bit (bit4) = 1,
                                                       [] video_object_layer_width (13bits) */
                                                       
                            0x00, 
                            
                            0x40,                   /* [] marker_bit (bit6) = 1
                                                       [] video_object_layer_height (13bits) */
                            0x01,                   /* [] marker_bit (bit0) = 1 */ 

                            0x44,                   /* [] interlaced (bit7) = 0
                                                       [] obmc_disable (bit6) = 1,
                                                       [] sprit_enable (bit5) = 0,
                                                       [] not_8_bit (bit4) = 0,
                                                       [] quant_type (bit3) = 0,
                                                       [] complexity_estimation_disable (bit2) = 1,
                                                       [] resync_marker_disable (bit1) = 0,
                                                       [] data_partitioned (bit0) = 0 */
                            0x3f,                   /* [] scalability (bit7) = 0
                                                       [] next_start_code = 0x3F */  
#endif                                                     
    };

    __align(4) MP4_VIDE_SL_CONFIG_DESCRIPTOR mp4VideSlConfigDescriptor =
    {
                        0x06,               /* slcd_tag */
                        0x01,               /* slcd_tag_length */
                        0x02,               /* slcd_tag_data */
    };

    size =  10 +                        /* 0x00, 0x00, 0x01, 0xb0, 0x01, 0x00, 0x00, 0x01, 0xb5, 0x09, */
        mp4VideHeaderSize;              /* mp4VideHeader */
    mp4VideDecoderSpecificInfo.dsi_tag_length = (u8) size; 
    size += 2 +                     /* + sizeof(dsi_tag) + sizeof(dsi_tag_length)  */   
        sizeof(MP4_VIDE_DECODER_CONFIG_DESCRIPTOR) - 2; /* - sizeof(dcd_tag) - sizeof(dcd_tag_length) */
    mp4VideDecoderConfigDescriptor.dcd_tag_length = (u8) size;
    size += 2 +                         /* + sizeof(dcd_tag) + sizeof(dcd_tag_length) */
        sizeof(MP4_VIDE_SL_CONFIG_DESCRIPTOR) +     
        sizeof(MP4_VIDE_ES_DESCRIPTOR) - 2;     /* - sizeof(esd_tag) - sizeof(esd_tag_length) */
    mp4VideEsDescriptor.esd_tag_length = (u8) size;
    size += 2 +                         /* + sizeof(esd_tag) + sizeof(esd_tag_length) */
        sizeof(MP4_VIDE_ESDS_BOX_T);
    tMp4VideEsdsBox.size = bSwap32(size);
    size += sizeof(MP4_VIDE_VISUAL_SAMPLE_ENTRY);
    mp4VideVisualSampleEntry.sample_description_size = bSwap32(size);
    mp4VideVisualSampleEntry.width_height = bSwap32(((u32)mp4VopWidth << 16) | ((u32)mp4VopHeight));

    size += sizeof(MP4_VIDE_STSD_BOX_T);
    tMp4VideStsdBox.size = bSwap32(size);   
    
    if (dcfWrite(pFile, (unsigned char*)&tMp4VideStsdBox, sizeof(MP4_VIDE_STSD_BOX_T), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&mp4VideVisualSampleEntry, sizeof(MP4_VIDE_VISUAL_SAMPLE_ENTRY), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideEsdsBox, sizeof(MP4_VIDE_ESDS_BOX_T), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&mp4VideEsDescriptor, sizeof(MP4_VIDE_ES_DESCRIPTOR), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&mp4VideDecoderConfigDescriptor, sizeof(MP4_VIDE_DECODER_CONFIG_DESCRIPTOR), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&mp4VideDecoderSpecificInfo, sizeof(MP4_VIDE_DECODER_SPECIFIC_INFO), &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&mp4VideHeader, mp4VideHeaderSize, &size) == 0)
            return 0;

    if (dcfWrite(pFile, (unsigned char*)&mp4VideSlConfigDescriptor, sizeof(MP4_VIDE_SL_CONFIG_DESCRIPTOR), &size) == 0)
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
s32 mp4WriteVideSttsBox(FS_FILE* pFile)
{
    u32 unMp4VideSttsBoxSize;
    u32 size;

    __align(4) MP4_VIDE_STTS_BOX_T tMp4VideSttsBox =
    {
        0x18000000,         /* size */
        0x73747473,         /* type = "stts" */
        0x00000000,         /* version_flags */
        0x00000000,         /* number_of_entries */
    };

    unMp4VideSttsBoxSize = sizeof(MP4_VIDE_STTS_BOX_T) +
                           mp4TimeToSampleTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TIME_ENTRY);

    tMp4VideSttsBox.size = bSwap32(unMp4VideSttsBoxSize);
    tMp4VideSttsBox.number_of_entries = bSwap32(mp4TimeToSampleTableEntryCount);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideSttsBox, sizeof(MP4_VIDE_STTS_BOX_T), &size) == 0)
            return 0;

/* mh@2006/11/09: Improve metadata-storing performance */
#if 0
    for (i = 0; i < mp4VopCount; i++)
    {

        
        entry = bSwap32(mp4VideSampleTimeTable[i].sample_count);
        if (dcfWrite(pFile, (unsigned char*)&entry, sizeof(u32), &size) == 0)
                return 0;

        entry = bSwap32(mp4VideSampleTimeTable[i].sample_duration);
        if (dcfWrite(pFile, (unsigned char*)&entry, sizeof(u32), &size) == 0)
                return 0;
    }
#else
    if (dcfWrite(pFile, (unsigned char*)mp4VideSampleTimeTable, mp4TimeToSampleTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TIME_ENTRY), &size) == 0)
                return 0;
#endif
/* mh@2006/11/09: END */


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
s32 mp4WriteVideStscBox(FS_FILE* pFile)
{
    u32 size;
    u32 unSamplePerChunk;
        
    __align(4) MP4_VIDE_STSC_BOX_T tMp4VideStscBox =
    {
        0x1c000000,         /* size */
        0x63737473,         /* type = "stsc" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */ 
    };

    size = sizeof(MP4_VIDE_STSC_BOX_T) + mp4SampleToChunkTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY);

    tMp4VideStscBox.size = bSwap32(size);
    tMp4VideStscBox.number_of_entries = bSwap32(mp4SampleToChunkTableEntryCount);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideStscBox, sizeof(MP4_VIDE_STSC_BOX_T), &size) == 0)
            return 0;

    /*____Write Video Sample to Chunk Table Entry____*/
    mp4VideSampleToChunkTable[0].first_chunk = 0x01000000;
    mp4VideSampleToChunkTable[0].samples_per_chunk = bSwap32(MP4_VIDE_SAMPLES_PER_CHUNK);
    mp4VideSampleToChunkTable[0].sample_description_id = 0x01000000;

    if (mp4SampleToChunkTableEntryCount == 2) 
    {
        mp4VideSampleToChunkTable[1].first_chunk = bSwap32(mp4ChunkOffsetTableEntryCount);
        unSamplePerChunk = mp4VopCount % MP4_VIDE_SAMPLES_PER_CHUNK;
        mp4VideSampleToChunkTable[1].samples_per_chunk = bSwap32(unSamplePerChunk);
        mp4VideSampleToChunkTable[1].sample_description_id = 0x01000000;
    }

    if (dcfWrite(pFile, (unsigned char*)&mp4VideSampleToChunkTable, mp4SampleToChunkTableEntryCount * sizeof(MP4_VIDE_SAMPLE_TO_CHUNK_ENTRY), &size) == 0)
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
s32 mp4WriteVideStszBox(FS_FILE* pFile)
{
    u32 size;
    
    __align(4) MP4_VIDE_STSZ_BOX_T tMp4VideStszBox =
    {
        0x00000000,         /* size */
        0x7a737473,         /* type = "stsz" */
        0x00000000,         /* version_flags */
        0x00000000,         /* sample_size */
        0x00000000,         /* number_of_entries = 0x???????? VOP count */
    };

    size = sizeof(MP4_VIDE_STSZ_BOX_T) + 
           mp4VopCount * sizeof(MP4_VIDE_SAMPLE_SIZE_ENTRY);
    tMp4VideStszBox.size = bSwap32(size);
    tMp4VideStszBox.number_of_entries = bSwap32(mp4VopCount);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideStszBox, sizeof(MP4_VIDE_STSZ_BOX_T), &size) == 0)
            return 0;

/* mh@2006/11/09: Improve metadata-storing performance */
#if 0
    for (i = 0; i < mp4VopCount; i++)
    {
        entry = bSwap32(mp4VideSampleSizeTable[i]);
        if (dcfWrite(pFile, (unsigned char*)&entry, sizeof(MP4_VIDE_SAMPLE_SIZE_ENTRY), &size) == 0)
                return 0;
    }
#else
    if (dcfWrite(pFile, (unsigned char*)mp4VideSampleSizeTable, mp4VopCount * sizeof(MP4_VIDE_SAMPLE_SIZE_ENTRY), &size) == 0)
            return 0;
#endif
/* mh@2006/11/09: END */

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
s32 mp4WriteVideStcoBox(FS_FILE* pFile)
{
    u32 size;

    __align(4) MP4_VIDE_STCO_BOX_T tMp4VideStcoBox =
    {
        0x00000000,         /* size */
        0x6f637473,         /* type = "stco" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
    };

    size = sizeof(MP4_VIDE_STCO_BOX_T) + mp4ChunkOffsetTableEntryCount * sizeof(MP4_VIDE_CHUNK_OFFSET_ENTRY);
    tMp4VideStcoBox.size = bSwap32(size);
    tMp4VideStcoBox.number_of_entries = bSwap32(mp4ChunkOffsetTableEntryCount);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideStcoBox, sizeof(MP4_VIDE_STCO_BOX_T), &size) == 0)
            return 0;

/* mh@2006/11/09: Improve metadata-storing performance */
#if 0
    for (i = 0; i < mp4VopCount; i++)
    {
        entry = bSwap32(mp4VideChunkOffsetTable[i]);

        if (dcfWrite(pFile, (unsigned char*)&entry, sizeof(MP4_VIDE_CHUNK_OFFSET_ENTRY), &size) == 0)
                return 0;
    }
#else
    if (dcfWrite(pFile, (unsigned char*)mp4VideChunkOffsetTable, mp4ChunkOffsetTableEntryCount * sizeof(MP4_VIDE_CHUNK_OFFSET_ENTRY), &size) == 0)
            return 0;
#endif
/* mh@2006/11/09: END */

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
s32 mp4WriteVideStssBox(FS_FILE* pFile)
{
    u32 size;

    __align(4) MP4_VIDE_STSS_BOX_T tMp4VideStssBox =
    {
        0x00000000,         /* size */
        0x73737473,         /* type = "stss" */
        0x00000000,         /* version_flags */
        0x00000000,         /* number_of_entries = 0x???????? I-VOP count */
    };

    size =  sizeof(MP4_VIDE_STSS_BOX_T) + 
        mp4IVopCount * sizeof(MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY);
    tMp4VideStssBox.size = bSwap32(size);
    tMp4VideStssBox.number_of_entries = bSwap32(mp4IVopCount);

    if (dcfWrite(pFile, (unsigned char*)&tMp4VideStssBox, sizeof(MP4_VIDE_STSS_BOX_T), &size) == 0)
            return 0;

/* mh@2006/11/09: Improve metadata-storing performance */
#if 0
    for (i = 0; i < mp4IVopCount; i++)
    {
        entry = bSwap32(mp4VideSyncSampleNumberTable[i]);

        if (dcfWrite(pFile, (unsigned char*)&entry, sizeof(MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY), &size) == 0)
                return 0;
    }
#else
    if (dcfWrite(pFile, (unsigned char*)mp4VideSyncSampleNumberTable, mp4IVopCount * sizeof(MP4_VIDE_SYNC_SAMPLE_NUMBER_ENTRY), &size) == 0)
                return 0;
#endif
/* mh@2006/11/09: END */

    return 1;
}

/************************************************************************/
/* MP4 Soun Track                           */
/************************************************************************/

#ifdef MP4_AUDIO

/*

Routine Description:

    MP4 initialize audio.

Arguments:

    None.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4AudioInit(void)
{
    /* audio format */
    mp4AudioFormat.number_of_channels = 0x0002; /* stereo */
    mp4AudioFormat.sample_size = 0x0010;        /* 16 bit per sample */
    mp4AudioFormat.sample_rate = 0x0000ac44;    /* 44100 Hz */

    /* audio fragment */
    mp4SounSampleEntryDuration = 0x00000400;    /* sample per sample_entry */           
    
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
s32 mp4WriteSounTrakAtom(FS_FILE* pFile)
{
    u32 size;
    MP4_SOUN_TRAK_ATOM mp4SounTrakAtom = 
    {
        0x00000000,         /* size */
        0x6b617274,         /* type = "trak" */
    };

    size =  sizeof(MP4_SOUN_TRAK_ATOM) +
        sizeof(MP4_SOUN_TKHD_ATOM) + 
/* Peter: 0727 S*/
        sizeof(MP4_SOUN_MDIA_ATOM) +
/* Peter: 0727 E*/
        sizeof(MP4_SOUN_MDHD_ATOM) +
        sizeof(MP4_SOUN_HDLR_ATOM) +
        sizeof(MP4_SOUN_MINF_ATOM) +
        sizeof(MP4_SOUN_DINF_ATOM) +
        sizeof(MP4_SOUN_DREF_ATOM) +
        sizeof(MP4_SOUN_STBL_ATOM) +
        sizeof(MP4_SOUN_STSD_ATOM) +
        sizeof(MP4_SOUN_STSZ_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_SAMPLE_SIZE_ENTRY) +
        sizeof(MP4_SOUN_STTS_ATOM) +
        sizeof(MP4_SOUN_STSC_ATOM) +
        sizeof(MP4_SOUN_STCO_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_CHUNK_OFFSET_ENTRY) +
        sizeof(MP4_SOUN_SMHD_ATOM);
    mp4SounTrakAtom.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounTrakAtom, sizeof(MP4_SOUN_TRAK_ATOM), &size) == 0)
            return 0;

    if (mp4WriteSounTkhdAtom(pFile) == 0)
            return 0;
/* Peter: 0727 S*/
    if (mp4WriteSounMdiaAtom(pFile) == 0)
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
s32 mp4WriteSounTkhdAtom(FS_FILE* pFile)
{
    u32 size, duration;
    __align(4) MP4_SOUN_TKHD_ATOM mp4SounTkhdAtom = 
    {
        0x5c000000,         /* size */
        0x64686b74,         /* type = "tkhd" */
        0x01000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time= 00:00:00 2004/01/01 */
        0x04000000,         /* track_id */
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
            0x00000000, 0x00000000, 0x00000040, /*           0.0 0.0 1.0 */
        0x00000000,     /* track_width = 0x00000000 */
            0x00000000,     /* track_height = 0x00000000 */
    };
    
    duration = mp4SounSampleEntryCount * mp4SounSampleEntryDuration * 1000 / mp4AudioFormat.sample_rate; /* millisec per total */

    mp4SounTkhdAtom.size = bSwap32(sizeof(MP4_SOUN_TKHD_ATOM));
    mp4SounTkhdAtom.duration = bSwap32(duration);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounTkhdAtom, sizeof(MP4_SOUN_TKHD_ATOM), &size) == 0)
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
s32 mp4WriteSounMdiaAtom(FS_FILE* pFile)
{
    u32 size;
/* Peter: 0727 S*/
    __align(4) MP4_SOUN_MDIA_ATOM mp4SounMdiaAtom =
    {
        0x00000000,         /* size */
        0x6169646d,         /* type = "mdia" */
    };

    size =  sizeof(MP4_SOUN_MDIA_ATOM) +
        sizeof(MP4_SOUN_MDHD_ATOM) +
        sizeof(MP4_SOUN_HDLR_ATOM) +
        sizeof(MP4_SOUN_MINF_ATOM) +
        sizeof(MP4_SOUN_DINF_ATOM) +
        sizeof(MP4_SOUN_DREF_ATOM) +
        sizeof(MP4_SOUN_STBL_ATOM) +
        sizeof(MP4_SOUN_STSD_ATOM) +
        sizeof(MP4_SOUN_STSZ_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_SAMPLE_SIZE_ENTRY) +
        sizeof(MP4_SOUN_STTS_ATOM) +
        sizeof(MP4_SOUN_STSC_ATOM) +
        sizeof(MP4_SOUN_STCO_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_CHUNK_OFFSET_ENTRY) +
        sizeof(MP4_SOUN_SMHD_ATOM);
        
    mp4SounMdiaAtom.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounMdiaAtom, sizeof(MP4_SOUN_MDIA_ATOM), &size) == 0)
            return 0;
/* Peter: 0727 E*/

    if (mp4WriteSounMdhdAtom(pFile) == 0)
            return 0;
    if (mp4WriteSounHdlrAtom(pFile) == 0)
            return 0;
    if (mp4WriteSounMinfAtom(pFile) == 0)
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
s32 mp4WriteSounMdhdAtom(FS_FILE* pFile)
{
    u32 size, sampleCount;
    __align(4) MP4_SOUN_MDHD_ATOM mp4SounMdhdAtom = 
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

    sampleCount = mp4SounSampleEntryCount * mp4SounSampleEntryDuration; /* number of sample per total */        
    
    mp4SounMdhdAtom.size = bSwap32(sizeof(MP4_SOUN_MDHD_ATOM));
    mp4SounMdhdAtom.time_scale = bSwap32((u32) mp4AudioFormat.sample_rate);
    mp4SounMdhdAtom.duration = bSwap32(sampleCount);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounMdhdAtom, sizeof(MP4_SOUN_MDHD_ATOM), &size) == 0)
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
s32 mp4WriteSounHdlrAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SOUN_HDLR_ATOM mp4SounHdlrAtom =
    {
        0x2c000000,         /* size */
        0x726c6468,         /* type = "hdlr" */
        0x00000000,         /* version_flags */
        0x00000000,         /* component_type */
        0x6e756f73,         /* component_subtype = "soun" (sound) */
        0x00000000,         /* component_manufacturer */
        0x00000000,         /* component_flags */
        0x00000000,         /* component_flags_mask */
        0x41, 0x75, 0x64, 0x69,     /* component_name[0x0D] = "AudioStream\0" */
        0x6F, 0x53, 0x74, 0x72, 
        0x65, 0x61, 0x6D, 0x00, 
    };

    mp4SounHdlrAtom.size = bSwap32(sizeof(MP4_SOUN_HDLR_ATOM));
    if (dcfWrite(pFile, (unsigned char*)&mp4SounHdlrAtom, sizeof(MP4_SOUN_HDLR_ATOM), &size) == 0)
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
s32 mp4WriteSounMinfAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SOUN_MINF_ATOM mp4SounMinfAtom =
    {
        0x00000000,         /* size */
        0x666e696d,         /* type = "minf" */
    };

    size =  sizeof(MP4_SOUN_MINF_ATOM) +
        sizeof(MP4_SOUN_DINF_ATOM) +
        sizeof(MP4_SOUN_DREF_ATOM) +
        sizeof(MP4_SOUN_STBL_ATOM) +
        sizeof(MP4_SOUN_STSD_ATOM) +
        sizeof(MP4_SOUN_STSZ_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_SAMPLE_SIZE_ENTRY) +
        sizeof(MP4_SOUN_STTS_ATOM) +
        sizeof(MP4_SOUN_STSC_ATOM) +
        sizeof(MP4_SOUN_STCO_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_CHUNK_OFFSET_ENTRY) +
        sizeof(MP4_SOUN_SMHD_ATOM);
        
    mp4SounMinfAtom.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounMinfAtom, sizeof(MP4_SOUN_MINF_ATOM), &size) == 0)
            return 0;

    if (mp4WriteSounDinfAtom(pFile) == 0)
            return 0;
    if (mp4WriteSounStblAtom(pFile) == 0)
            return 0;
    if (mp4WriteSounSmhdAtom(pFile) == 0)
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
s32 mp4WriteSounDinfAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SOUN_DINF_ATOM mp4SounDinfAtom =
    {
        0x24000000,         /* size */
        0x666e6964,         /* type = "dinf" */
    };

    size =  sizeof(MP4_SOUN_DINF_ATOM) +            
        sizeof(MP4_SOUN_DREF_ATOM);
        
    mp4SounDinfAtom.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounDinfAtom, sizeof(MP4_SOUN_DINF_ATOM), &size) == 0)
            return 0;

    if (mp4WriteSounDrefAtom(pFile) == 0)
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
s32 mp4WriteSounDrefAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SOUN_DREF_ATOM mp4SounDrefAtom =
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

    mp4SounDrefAtom.size = bSwap32(sizeof(MP4_SOUN_DREF_ATOM));
    if (dcfWrite(pFile, (unsigned char*)&mp4SounDrefAtom, sizeof(MP4_SOUN_DREF_ATOM), &size) == 0)
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
s32 mp4WriteSounStblAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SOUN_STBL_ATOM mp4SounStblAtom =
    {
        0x00000000,         /* size */
        0x6c627473,         /* type = "stbl" */
    };

    size =  sizeof(MP4_SOUN_STBL_ATOM) +
        sizeof(MP4_SOUN_STSD_ATOM) +
        sizeof(MP4_SOUN_STSZ_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_SAMPLE_SIZE_ENTRY) +
        sizeof(MP4_SOUN_STTS_ATOM) +
        sizeof(MP4_SOUN_STSC_ATOM) +
        sizeof(MP4_SOUN_STCO_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_CHUNK_OFFSET_ENTRY);
        
    mp4SounStblAtom.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounStblAtom, sizeof(MP4_SOUN_STBL_ATOM), &size) == 0)
            return 0;

    if (mp4WriteSounStsdAtom(pFile) == 0)
            return 0;
    if (mp4WriteSounStszAtom(pFile) == 0)
            return 0;
    if (mp4WriteSounSttsAtom(pFile) == 0)
            return 0;
    if (mp4WriteSounStscAtom(pFile) == 0)
            return 0;
    if (mp4WriteSounStcoAtom(pFile) == 0)
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
s32 mp4WriteSounStsdAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SOUN_STSD_ATOM mp4SounStsdAtom =
    {
        0x5b000000,         /* size */
        0x64737473,         /* type = "stsd" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x4b000000,         /* sample_description_size */
            0x6134706d,         /* data_format = "mp4a" */
            0x00, 0x00, 0x00, 0x00,     /* reserved[6] */   
            0x00, 0x00,
            0x0100,             /* data_reference_index = data_reference of this track */
            0x00000000,         /* version_revision */
            0x00000000,         /* vendor */
            0x0200,             /* number_of_channels = 0x???? */
            0x1000,             /* sample_size = 0x???? */
            0x0000,             /* compression_id */
            0x0000,             /* packet_size */
            0x00000000,         /* sample_rate */
            {
                0x27000000,         /* size */
                0x73647365,         /* type = "esds" */
                0x00000000,         /* version_flags */
                {   
                    /* ES_Descriptor */
                    0x03,               /* esd_tag */
                    0x19,               /* esd_tag_length */
                    0x0000,             /* esd_tag_es_id */
                    0x00,               /* esd_tag_flag */
                        /* DecoderConfigDescriptor */                                    
                        0x04,               /* dcd_tag */
                        0x11,               /* dcd_tag_length */
                        0x40,               /* dcd_tag_obj_type_ind */
                        0x00c00015,         /* dcd_tag_flag_buf_size */
                        0xa83b0200,         /* dcd_tag_max_bitrate */
                        0x9eef0100,         /* dcd_tag_avg_bitrate */
                            /* DecoderSpecificInfo */
                            0x05,               /* dsi_tag */
                            0x02,               /* dsi_tag_length */
                            0x12, 0x10,         /* dsi_tag_data */
                        /* SLConfigDescriptor */
                        0x06,               /* slcd_tag */
                        0x01,               /* slcd_tag_length */
                        0x02,               /* slcd_tag_data */
                },
            },
        },
    };

    mp4SounStsdAtom.size = bSwap32(sizeof(MP4_SOUN_STSD_ATOM));
    mp4SounStsdAtom.mp4_soun_audio_sample_entry[0].number_of_channels = bSwap16(mp4AudioFormat.number_of_channels);
    mp4SounStsdAtom.mp4_soun_audio_sample_entry[0].sample_size = bSwap16(mp4AudioFormat.sample_size);
    mp4SounStsdAtom.mp4_soun_audio_sample_entry[0].sample_rate = bSwap32(mp4AudioFormat.sample_rate << 16);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounStsdAtom, sizeof(MP4_SOUN_STSD_ATOM), &size) == 0)
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
s32 mp4WriteSounStszAtom(FS_FILE* pFile)
{
    u32 size, entry, i;
    __align(4) MP4_SOUN_STSZ_ATOM mp4SounStszAtom =
    {
        0x00000000,         /* size */
        0x7a737473,         /* type = "stsz" */
        0x00000000,         /* version_flags */
        0x00000000,         /* sample_size */
        0x00000000,         /* number_of_entries = 0x???????? VOP count */
    };

    size = sizeof(MP4_SOUN_STSZ_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_SAMPLE_SIZE_ENTRY);
    mp4SounStszAtom.size = bSwap32(size);
    mp4SounStszAtom.number_of_entries = bSwap32(mp4SounSampleEntryCount);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounStszAtom, sizeof(MP4_SOUN_STSZ_ATOM), &size) == 0)
            return 0;
    for (i = 0; i < mp4SounSampleEntryCount; i++)
    {
        entry = bSwap32(mp4SounSampleSizeTable[i]);
        if (dcfWrite(pFile, (unsigned char*)&entry, sizeof(MP4_SOUN_SAMPLE_SIZE_ENTRY), &size) == 0)
                return 0;
    }

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
s32 mp4WriteSounSttsAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SOUN_STTS_ATOM mp4SounSttsAtom =
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

    mp4SounSttsAtom.size = bSwap32(sizeof(MP4_SOUN_STTS_ATOM));
    mp4SounSttsAtom.mp4_soun_sample_time_entry[0].sample_count = bSwap32(mp4SounSampleEntryCount);
    mp4SounSttsAtom.mp4_soun_sample_time_entry[0].sample_duration = bSwap32(mp4SounSampleEntryDuration);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounSttsAtom, sizeof(MP4_SOUN_STTS_ATOM), &size) == 0)
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
s32 mp4WriteSounStscAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SOUN_STSC_ATOM mp4SounStscAtom =
    {
        0x1c000000,         /* size */
        0x63737473,         /* type = "stsc" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x01000000,         /* first_chunk */
            0x01000000,         /* samples_per_chunk = mp4SounSampleEntryCount  */
            0x01000000,         /* sample_description_id = visual_sample_entry of this track */
        },
    };

    mp4SounStscAtom.size = bSwap32(sizeof(MP4_SOUN_STSC_ATOM));
    //mp4SounStscAtom.mp4_soun_sample_to_chunk_entry[0].samples_per_chunk = bSwap32(mp4SounSampleEntryCount);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounStscAtom, sizeof(MP4_SOUN_STSC_ATOM), &size) == 0)
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
s32 mp4WriteSounStcoAtom(FS_FILE* pFile)
{
    u32 size, entry, i;
    __align(4) MP4_SOUN_STCO_ATOM mp4SounStcoAtom =
    {
        0x00000000,     /* size */
        0x6f637473,     /* type = "stco" */
        0x00000000,     /* version_flags */
        0x00000000,     /* number_of_entries */
    };

    size = sizeof(MP4_SOUN_STCO_ATOM) + mp4SounSampleEntryCount * sizeof(MP4_SOUN_CHUNK_OFFSET_ENTRY);
    mp4SounStcoAtom.size = bSwap32(size);
    mp4SounStcoAtom.number_of_entries = bSwap32(mp4SounSampleEntryCount);
    if (dcfWrite(pFile, (unsigned char*)&mp4SounStcoAtom, sizeof(MP4_SOUN_STCO_ATOM), &size) == 0)
            return 0; 
    for (i = 0; i < mp4SounSampleEntryCount; i++)
    {
        entry = bSwap32(mp4SounChunkOffsetTable[i]);
        if (dcfWrite(pFile, (unsigned char*)&entry, sizeof(MP4_SOUN_CHUNK_OFFSET_ENTRY), &size) == 0)
                return 0;
    }
    
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
s32 mp4WriteSounSmhdAtom(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SOUN_SMHD_ATOM mp4SounSmhdAtom =
    {
        0x10000000,         /* size */
        0x64686d73,         /* type = "smhd" */
        0x01000000,         /* version_flags */
        0x0000,             /* balance = (0.0) */
        0x0000,             /* reserved */
    };

    mp4SounSmhdAtom.size = bSwap32(sizeof(MP4_SOUN_SMHD_ATOM));
    if (dcfWrite(pFile, (unsigned char*)&mp4SounSmhdAtom, sizeof(MP4_SOUN_SMHD_ATOM), &size) == 0)
            return 0;

    return 1;
}

#endif

/************************************************************************/
/*                 MP4 Object Descriptor Stream Track                   */
/************************************************************************/

/*

Routine Description:

    Write ODSM TRAK Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmTrakBox(FS_FILE* pFile)
{
    u32 size;
    
    __align(4) MP4_ODSM_TRAK_BOX_T tMp4OdsmTrakBox = 
    {
        0x00000000,         /* size */
        0x6b617274,         /* type = "trak" */
    };

/* Peter: 0727 S*/
    size =  sizeof(MP4_ODSM_TRAK_BOX_T) +
            sizeof(MP4_ODSM_TKHD_BOX_T) + 
            sizeof(MP4_ODSM_TREF_BOX_T) +
            sizeof(MP4_ODSM_MDIA_BOX_T) +
            sizeof(MP4_ODSM_MDHD_BOX_T) +
            sizeof(MP4_ODSM_HDLR_BOX_T) +
            sizeof(MP4_ODSM_MINF_BOX_T) +
            sizeof(MP4_ODSM_NMHD_BOX_T) +
            sizeof(MP4_ODSM_DINF_BOX_T) +
            sizeof(MP4_ODSM_DREF_BOX_T) +
            sizeof(MP4_ODSM_STBL_BOX_T) +
            sizeof(MP4_ODSM_STSD_BOX_T) +
            sizeof(MP4_ODSM_STTS_BOX_T) +
            sizeof(MP4_ODSM_STSC_BOX_T) +
            sizeof(MP4_ODSM_STSZ_BOX_T) +           
            sizeof(MP4_ODSM_STCO_BOX_T);

    tMp4OdsmTrakBox.size = bSwap32(size);

    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmTrakBox, sizeof(MP4_ODSM_TRAK_BOX_T), &size) == 0)
            return 0;

    if (mp4WriteOdsmTkhdBox(pFile) == 0)
            return 0;

    if (mp4WriteOdsmTrefBox(pFile) == 0)
            return 0;
    
    if (mp4WriteOdsmMdiaBox(pFile) == 0)
            return 0;
    
/* Peter: 0727 E*/

    return 1;
}

/*

Routine Description:

    Write ODSM TKHD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmTkhdBox(FS_FILE* pFile)
{
    u32 size;
    
    __align(4) MP4_ODSM_TKHD_BOX_T tMp4OdsmTkhdBox = 
    {
        0x5c000000,         /* size */
        0x64686b74,         /* type = "tkhd" */
        0x01000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time= 00:00:00 2004/01/01 */
        0x01000000,         /* track_id */
        0x00000000,         /* reserved1 */
        0x00000000,         /* duration */
        0x00, 0x00, 0x00, 0x00,     /* reserved2[8] */
        0x00, 0x00, 0x00, 0x00, 
        0x0000,             /* layer = spatial priority for overlay */
        0x0000,             /* alternate_group = group of movie tracks for QoS choice */
        0x0000,             /* volume = 0.0 */
        0x0000,             /* reserved3 */
        0x00000100, 0x00000000, 0x00000000, /*           1.0 0.0 0.0 */
        0x00000000, 0x00000100, 0x00000000, /* matrix_structure[9] = 0.0 0.0 0.0 */
        0x00000000, 0x00000000, 0x00000040, /*           0.0 0.0 1.0 */ 
        0x00000000,         /* track_width */
        0x00000000,         /* track_height */
    };

    tMp4OdsmTkhdBox.size = bSwap32(sizeof(MP4_ODSM_TKHD_BOX_T));
    tMp4OdsmTkhdBox.duration = bSwap32(mp4VideDuration);
    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmTkhdBox, sizeof(MP4_ODSM_TKHD_BOX_T), &size) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Write ODSM TREF Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] 1. Track Reference Box 'tref' provides a reference from the containing
          track to another track in the presentation
       2. So far, this box contain two referenced tracks:
          vide track (track ID 3) & soun track (track ID 4)
*/
s32 mp4WriteOdsmTrefBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_ODSM_TREF_BOX_T tMp4OdsmTrefBox =
    {
        0x14000000,         /* size */
        0x66657274,         /* type = "tref" */
        {
            0x0c000000,         /* size */
            0x646f706d,         /* type = "mpod" */
            {
                0x03000000,         /* track_ids[0] = vide track id */
                0x04000000,         /* track_ids[1] = soun track id */
            },  
        },
    };

    tMp4OdsmTrefBox.size = bSwap32(sizeof(MP4_ODSM_TREF_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmTrefBox, sizeof(MP4_ODSM_TREF_BOX_T), &size) == 0)
            return 0;

    return 1;
}


/*

Routine Description:

    Write ODSM MDIA Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
/* Peter: 0727 S*/
s32 mp4WriteOdsmMdiaBox(FS_FILE* pFile)
{
    u32 size;

    __align(4) MP4_ODSM_MDIA_BOX_T tMp4OdsmMdiaBox =
    {
        0x00000000,         /* size */
        0x6169646d,         /* type = "mdia" */
    };

    size =  sizeof(MP4_ODSM_MDIA_BOX_T) +
            sizeof(MP4_ODSM_MDHD_BOX_T) +
            sizeof(MP4_ODSM_HDLR_BOX_T) +
            sizeof(MP4_ODSM_MINF_BOX_T) +
            sizeof(MP4_ODSM_NMHD_BOX_T) +
            sizeof(MP4_ODSM_DINF_BOX_T) +
            sizeof(MP4_ODSM_DREF_BOX_T) +
            sizeof(MP4_ODSM_STBL_BOX_T) +
            sizeof(MP4_ODSM_STSD_BOX_T) +
            sizeof(MP4_ODSM_STTS_BOX_T) +
            sizeof(MP4_ODSM_STSC_BOX_T) +
            sizeof(MP4_ODSM_STSZ_BOX_T) +
            sizeof(MP4_ODSM_STCO_BOX_T);
    
    tMp4OdsmMdiaBox.size = bSwap32(size);

    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmMdiaBox, sizeof(MP4_ODSM_MDIA_BOX_T), &size) == 0)
            return 0;

    if (mp4WriteOdsmMdhdBox(pFile) == 0)
            return 0;

    if (mp4WriteOdsmHdlrBox(pFile) == 0)
            return 0;

    if (mp4WriteOdsmMinfBox(pFile) == 0)
            return 0;

    return 1;
}
/* Peter: 0727 E*/

/*

Routine Description:

    Write ODSM MDHD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmMdhdBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_ODSM_MDHD_BOX_T tMp4OdsmMdhdBox = 
    {
        0x20000000,         /* size */
        0x6468646d,         /* type = "mdhd" */
        0x00000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time= 00:00:00 2004/01/01 */
        0xe8030000,         /* time_scale = 1000 time_scale_uint/sec = 1 millisec/time_scale_unit */
        0x00000000,         /* duration = 0 time_scale_unit */
        0x0000,             /* language */
        0x0000,             /* pre_defined */
    };

    tMp4OdsmMdhdBox.size = bSwap32(sizeof(MP4_ODSM_MDHD_BOX_T));
    tMp4OdsmMdhdBox.duration = bSwap32(mp4VideDuration);

    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmMdhdBox, sizeof(MP4_ODSM_MDHD_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write ODSM HDLR Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmHdlrBox(FS_FILE* pFile)
{
    u32 size;
    
    __align(4) MP4_ODSM_HDLR_BOX_T tMp4OdsmHdlrBox =
    {
        0x37000000,         /* size */
        0x726c6468,         /* type = "hdlr" */
        0x00000000,         /* version_flags */
        0x00000000,         /* pre_defined */
        0x6d73646f,         /* handler_type = "odsm" (object descriptor) */
        0x00000000,         /* reserved[0] */
        0x00000000,         /* reserved[1] */
        0x00000000,         /* reserved[2] */
        0x4f, 0x62, 0x6a, 0x65,     /* name[0x017] = "ObjectDescriptorStream\0" */
        0x63, 0x74, 0x44, 0x65,
        0x73, 0x63, 0x72, 0x69,
        0x70, 0x74, 0x6f, 0x72, 
        0x53, 0x74, 0x72, 0x65, 
        0x61, 0x6d, 0x00,
    };

    tMp4OdsmHdlrBox.size = bSwap32(sizeof(MP4_ODSM_HDLR_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmHdlrBox, sizeof(MP4_ODSM_HDLR_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write ODSM MINF Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmMinfBox(FS_FILE* pFile)
{
    u32 size;
    
    __align(4) MP4_ODSM_MINF_BOX_T tMp4OdsmMinfBox =
    {
        0x00000000,         /* size */
        0x666e696d,         /* type = "minf" */
    };

    size =  sizeof(MP4_ODSM_MINF_BOX_T) +
            sizeof(MP4_ODSM_NMHD_BOX_T) +
            sizeof(MP4_ODSM_DINF_BOX_T) +
            sizeof(MP4_ODSM_DREF_BOX_T) +
            sizeof(MP4_ODSM_STBL_BOX_T) +
            sizeof(MP4_ODSM_STSD_BOX_T) +
            sizeof(MP4_ODSM_STTS_BOX_T) +
            sizeof(MP4_ODSM_STSC_BOX_T) +
            sizeof(MP4_ODSM_STSZ_BOX_T) +       
            sizeof(MP4_ODSM_STCO_BOX_T);
    
    tMp4OdsmMinfBox.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmMinfBox, sizeof(MP4_ODSM_MINF_BOX_T), &size) == 0)
            return 0;

    if (mp4WriteOdsmNmhdBox(pFile) == 0)
            return 0;

    if (mp4WriteOdsmDinfBox(pFile) == 0)
            return 0;

    if (mp4WriteOdsmStblBox(pFile) == 0)
            return 0;
    

    return 1;
}

/*

Routine Description:

    Write ODSM NMHD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

[Note] Null Media Header Box 'nmhd' is used by the streams other than
       visual and audio

*/
s32 mp4WriteOdsmNmhdBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_ODSM_NMHD_BOX_T tMp4OdsmNmhdBox =
    {
        0x0c000000,         /* size */
        0x64686d6e,         /* type = "nmhd" */
        0x00000000,         /* version_flags */
    };

    tMp4OdsmNmhdBox.size = bSwap32(sizeof(MP4_ODSM_NMHD_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmNmhdBox, sizeof(MP4_ODSM_NMHD_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write ODSM DINF Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmDinfBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_ODSM_DINF_BOX_T tMp4OdsmDinfBox =
    {
        0x24000000,         /* size */
        0x666e6964,         /* type = "dinf" */
    };

    size =  sizeof(MP4_ODSM_DINF_BOX_T) +           
            sizeof(MP4_ODSM_DREF_BOX_T);

    tMp4OdsmDinfBox.size = bSwap32(size);

    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmDinfBox, sizeof(MP4_ODSM_DINF_BOX_T), &size) == 0)
            return 0;

    if (mp4WriteOdsmDrefBox(pFile) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Write ODSM DREF Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmDrefBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_ODSM_DREF_BOX_T tMp4OdsmDrefBox =
    {
        0x1c000000,         /* size */
        0x66657264,         /* type = "dref" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x0c000000,         /* size */
            0x206c7275,         /* type = "url " */
            0x01000000,         /* version_flags = 0x00000001 (self reference) */ 
        },
    };

    tMp4OdsmDrefBox.size = bSwap32(sizeof(MP4_ODSM_DREF_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmDrefBox, sizeof(MP4_ODSM_DREF_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write ODSM STBL Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmStblBox(FS_FILE* pFile)
{
    u32 size;

    __align(4) MP4_ODSM_STBL_BOX_T tMp4OdsmStblBox =
    {
        0x00000000,         /* size */
        0x6c627473,         /* type = "stbl" */
    };

    size =  sizeof(MP4_ODSM_STBL_BOX_T) +
            sizeof(MP4_ODSM_STSD_BOX_T) +
            sizeof(MP4_ODSM_STTS_BOX_T) +
            sizeof(MP4_ODSM_STSC_BOX_T) +
            sizeof(MP4_ODSM_STSZ_BOX_T) +
            sizeof(MP4_ODSM_STCO_BOX_T);

    tMp4OdsmStblBox.size = bSwap32(size);   

    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmStblBox, sizeof(MP4_ODSM_STBL_BOX_T), &size) == 0)
            return 0;

    if (mp4WriteOdsmStsdBox(pFile) == 0)
            return 0;
    
    if (mp4WriteOdsmSttsBox(pFile) == 0)
            return 0;

    if (mp4WriteOdsmStscBox(pFile) == 0)
            return 0;
    
    if (mp4WriteOdsmStszBox(pFile) == 0)
            return 0;
    
    if (mp4WriteOdsmStcoBox(pFile) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Write ODSM STSD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmStsdBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_ODSM_STSD_BOX_T tMp4OdsmStsdBox =
    {
        0x43000000,         /* size */
        0x64737473,         /* type = "stsd" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x33000000,         /* sample_description_size */
            0x7334706d,         /* data_format = "mp4s" */
            0x00, 0x00, 0x00, 0x00,     /* reserved[6] */   
            0x00, 0x00,
            0x0100,             /* data_reference_index = data_reference of this track */
            {
                0x23000000,     /* size */
                0x73647365,     /* type = "esds" */
                0x00000000,     /* version_flags */
                {
                    /* ES_Descriptor */
                    0x03,           /* esd_tag (ES_DescrTag) */
                    0x15,           /* esd_tag_length */
                    0x0100,         /* esd_tag_es_id */
                    0x00,           /* esd_tag_flag */

                        /* DecoderConfigDescriptor */                                
                        0x04,           /* dcd_tag (DecoderConfigDescrTag) */
                        0x0d,           /* dcd_tag_length */
                        0x02,           /* dcd_tag_obj_type_ind (ObjectTypeIdentification) */
                        0x84000005,     /* dcd_tag_flag_buf_size 
                                           streamType = 1 (ObjectDescriptorStream) */
                        
                        0x00000000,     /* dcd_tag_max_bitrate */
                        0x00000000,     /* dcd_tag_avg_bitrate */

                        /* SLConfigDescriptor */
                        0x06,           /* slcd_tag (SLConfigDescrTag) */
                        0x01,           /* slcd_tag_length */
                        0x02,           /* slcd_tag_data */
                },
            },
        },
    };

    tMp4OdsmStsdBox.size = bSwap32(sizeof(MP4_ODSM_STSD_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmStsdBox, sizeof(MP4_ODSM_STSD_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write ODSM STTS Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmSttsBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_ODSM_STTS_BOX_T tMp4OdsmSttsBox =
    {
        0x18000000,     /* size */
        0x73747473,     /* type = "stts" */
        0x00000000,     /* version_flags */
        0x01000000,     /* number_of_entries */
        {
            0x01000000,     /* sample_count  */
            0x00000000,     /* sample_duration */
        },
    };

    tMp4OdsmSttsBox.size = bSwap32(sizeof(MP4_ODSM_STTS_BOX_T));
    tMp4OdsmSttsBox.mp4_odsm_sample_time_entry[0].sample_duration = bSwap32(mp4VideDuration);
    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmSttsBox, sizeof(MP4_ODSM_STTS_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write ODSM STSC Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmStscBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_ODSM_STSC_BOX_T tMp4OdsmStscBox =
    {
        0x1c000000,         /* size */
        0x63737473,         /* type = "stsc" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x01000000,         /* first_chunk */
            0x01000000,         /* samples_per_chunk */
            0x01000000,         /* sample_description_id = visual_sample_entry of this track */
        },
    };

    tMp4OdsmStscBox.size = bSwap32(sizeof(MP4_ODSM_STSC_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmStscBox, sizeof(MP4_ODSM_STSC_BOX_T), &size) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Write ODSM STSZ Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmStszBox(FS_FILE* pFile)
{
    u32 size;
    
    __align(4) MP4_ODSM_STSZ_BOX_T tMp4OdsmStszBox =
    {
        0x14000000,         /* size */
        0x7a737473,         /* type = "stsz" */
        0x00000000,         /* version_flags */
        0x00000000,         /* sample_size = 0x???????? VOP count */
        0x01000000,         /* number_of_entries */
    };

    tMp4OdsmStszBox.sample_size = bSwap32(mp4VopCount);
    if (dcfWrite(pFile, (unsigned char*)&tMp4OdsmStszBox, sizeof(MP4_ODSM_STSZ_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write ODSM STCO Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteOdsmStcoBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_ODSM_STCO_BOX_T mp4OdsmStcoBox =
    {
        0x14000000,         /* size */
        0x6f637473,         /* type = "stco" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x00000000,         /* offset */
        },
    };

    mp4OdsmStcoBox.size = bSwap32(sizeof(MP4_ODSM_STCO_BOX_T));
    mp4OdsmStcoBox.mp4_odsm_chunk_offset_entry[0] = bSwap32(mp4FileOffset);

    if (dcfWrite(pFile, (unsigned char*)&mp4OdsmStcoBox, sizeof(MP4_ODSM_STCO_BOX_T), &size) == 0)
            return 0;

    return 1;
}





/************************************************************************/
/*                MP4 Scene Description Stream Track                    */
/************************************************************************/

/*

Routine Description:

    Write SDSM TRAK Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmTrakBox(FS_FILE* pFile)
{
    u32 size;
    
    __align(4) MP4_SDSM_TRAK_BOX_T tMp4SdsmTrakBox = 
    {
        0x00000000,         /* size */
        0x6b617274,         /* type = "trak" */
    };

/* Peter: 0727 S*/
    size =  sizeof(MP4_SDSM_TRAK_BOX_T) +
            sizeof(MP4_SDSM_TKHD_BOX_T) + 
            sizeof(MP4_SDSM_MDIA_BOX_T) +
            sizeof(MP4_SDSM_MDHD_BOX_T) +
            sizeof(MP4_SDSM_HDLR_BOX_T) +
            sizeof(MP4_SDSM_MINF_BOX_T) +
            sizeof(MP4_SDSM_NMHD_BOX_T) +
            sizeof(MP4_SDSM_DINF_BOX_T) +
            sizeof(MP4_SDSM_DREF_BOX_T) +
            sizeof(MP4_SDSM_STBL_BOX_T) +
            sizeof(MP4_SDSM_STSD_BOX_T) +
            sizeof(MP4_SDSM_STTS_BOX_T) +
            sizeof(MP4_SDSM_STSC_BOX_T) +
            sizeof(MP4_SDSM_STSZ_BOX_T) +
            sizeof(MP4_SDSM_STCO_BOX_T);

    tMp4SdsmTrakBox.size = bSwap32(size);
    
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmTrakBox, sizeof(MP4_SDSM_TRAK_BOX_T), &size) == 0)
            return 0;

    if (mp4WriteSdsmTkhdBox(pFile) == 0)
            return 0;
    
    if (mp4WriteSdsmMdiaBox(pFile) == 0)
            return 0;

/* Peter: 0727 E*/

    return 1;
}

/*

Routine Description:

    Write SDSM TKHD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmTkhdBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_TKHD_BOX_T tMp4SdsmTkhdBox = 
    {
        0x5c000000,         /* size */
        0x64686b74,         /* type = "tkhd" */
        0x01000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time= 00:00:00 2004/01/01 */
        0x02000000,         /* track_id */
        0x00000000,         /* reserved1 */
        0x00000000,         /* duration */
        0x00, 0x00, 0x00, 0x00,     /* reserved2[8] */
        0x00, 0x00, 0x00, 0x00, 
        0x0000,             /* layer = spatial priority for overlay */
        0x0000,             /* alternate_group = group of movie tracks for QoS choice */
        0x0000,             /* volume = 0.0 */
        0x0000,             /* reserved3 */
        0x00000100, 0x00000000, 0x00000000, /*           1.0 0.0 0.0 */
        0x00000000, 0x00000100, 0x00000000, /* matrix_structure[9] = 0.0 0.0 0.0 */
        0x00000000, 0x00000000, 0x00000040, /*           0.0 0.0 1.0 */
        0x00000000,         /* track_width */
        0x00000000,         /* track_height */
    };

    tMp4SdsmTkhdBox.size = bSwap32(sizeof(MP4_SDSM_TKHD_BOX_T));
    tMp4SdsmTkhdBox.duration = bSwap32(mp4VideDuration);
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmTkhdBox, sizeof(MP4_SDSM_TKHD_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SDSM MDIA Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
/* Peter: 0727 S*/
s32 mp4WriteSdsmMdiaBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_MDIA_BOX_T tMp4SdsmMdiaBox =
    {
        0x00000000,         /* size */
        0x6169646d,         /* type = "mdia" */
    };

    size =  sizeof(MP4_SDSM_MDIA_BOX_T) +
            sizeof(MP4_SDSM_MDHD_BOX_T) +
            sizeof(MP4_SDSM_HDLR_BOX_T) +
            sizeof(MP4_SDSM_MINF_BOX_T) +
            sizeof(MP4_SDSM_NMHD_BOX_T) +
            sizeof(MP4_SDSM_DINF_BOX_T) +
            sizeof(MP4_SDSM_DREF_BOX_T) +
            sizeof(MP4_SDSM_STBL_BOX_T) +
            sizeof(MP4_SDSM_STSD_BOX_T) +
            sizeof(MP4_SDSM_STTS_BOX_T) +
            sizeof(MP4_SDSM_STSC_BOX_T) +
            sizeof(MP4_SDSM_STSZ_BOX_T) +
            sizeof(MP4_SDSM_STCO_BOX_T);
        
    tMp4SdsmMdiaBox.size = bSwap32(size);

    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmMdiaBox, sizeof(MP4_SDSM_MDIA_BOX_T), &size) == 0)
            return 0; 
/* Peter: 0727 E*/

    if (mp4WriteSdsmMdhdBox(pFile) == 0)
            return 0;

    if (mp4WriteSdsmHdlrBox(pFile) == 0)
            return 0;

    if (mp4WriteSdsmMinfBox(pFile) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SDSM MDHD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmMdhdBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_MDHD_BOX_T tMp4SdsmMdhdBox = 
    {
        0x20000000,         /* size */
        0x6468646d,         /* type = "mdhd" */
        0x00000000,         /* version_flags */
        0x801319bc,         /* creation_time = 00:00:00 2004/01/01 */
        0x801319bc,         /* modification_time= 00:00:00 2004/01/01 */
        0xe8030000,         /* time_scale = 1000 time_scale_uint/sec = 1 millisec/time_scale_unit */
        0x00000000,         /* duration = 0 time_scale_unit */
        0x0000,             /* language */
        0x0000,             /* pre_defined */
    };

    tMp4SdsmMdhdBox.size = bSwap32(sizeof(MP4_SDSM_MDHD_BOX_T));
    tMp4SdsmMdhdBox.duration = bSwap32(mp4VideDuration);
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmMdhdBox, sizeof(MP4_SDSM_MDHD_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SDSM HDLR Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmHdlrBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_HDLR_BOX_T tMp4SdsmHdlrBox =
    {
        0x37000000,         /* size */
        0x726c6468,         /* type = "hdlr" */
        0x00000000,         /* version_flags */
        0x00000000,         /* pre_defined */
        0x6d736473,         /* handler_type = "sdsm" (scene descriptor) */
        0x00000000,         /* reserved[0] */
        0x00000000,         /* reserved[1] */
        0x00000000,         /* reserved[2] */
        0x53, 0x63, 0x65, 0x6e,     /* name[0x017] = "SceneDescriptionStream\0" */
        0x65, 0x44, 0x65, 0x73,
        0x63, 0x72, 0x69, 0x70, 
        0x74, 0x69, 0x6f, 0x6e, 
        0x53, 0x74, 0x72, 0x65, 
        0x61, 0x6d, 0x00,
    };

    tMp4SdsmHdlrBox.size = bSwap32(sizeof(MP4_SDSM_HDLR_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmHdlrBox, sizeof(MP4_SDSM_HDLR_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SDSM MINF Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmMinfBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_MINF_BOX_T tMp4SdsmMinfBox =
    {
        0x00000000,         /* size */
        0x666e696d,         /* type = "minf" */
    };

    size =  sizeof(MP4_SDSM_MINF_BOX_T) +
            sizeof(MP4_SDSM_NMHD_BOX_T) +
            sizeof(MP4_SDSM_DINF_BOX_T) +
            sizeof(MP4_SDSM_DREF_BOX_T) +
            sizeof(MP4_SDSM_STBL_BOX_T) +
            sizeof(MP4_SDSM_STSD_BOX_T) +
            sizeof(MP4_SDSM_STTS_BOX_T) +
            sizeof(MP4_SDSM_STSC_BOX_T) +
            sizeof(MP4_SDSM_STSZ_BOX_T) +
            sizeof(MP4_SDSM_STCO_BOX_T);

    tMp4SdsmMinfBox.size = bSwap32(size);

    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmMinfBox, sizeof(MP4_SDSM_MINF_BOX_T), &size) == 0)
            return 0;

    if (mp4WriteSdsmNmhdBox(pFile) == 0)
            return 0;

    if (mp4WriteSdsmDinfBox(pFile) == 0)
            return 0;
    
    if (mp4WriteSdsmStblBox(pFile) == 0)
            return 0;
    

    return 1;
}

/*

Routine Description:

    Write SDSM NMHD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmNmhdBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_NMHD_BOX_T tMp4SdsmNmhdBox =
    {
        0x0c000000,         /* size */
        0x64686d6e,         /* type = "nmhd" */
        0x00000000,         /* version_flags */
    };

    tMp4SdsmNmhdBox.size = bSwap32(sizeof(MP4_SDSM_NMHD_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmNmhdBox, sizeof(MP4_SDSM_NMHD_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SDSM DINF Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmDinfBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_DINF_BOX_T tMp4SdsmDinfBox =
    {
        0x24000000,         /* size */
        0x666e6964,         /* type = "dinf" */
    };

    size =  sizeof(MP4_SDSM_DINF_BOX_T) +
            sizeof(MP4_SDSM_DREF_BOX_T);
    tMp4SdsmDinfBox.size = bSwap32(size);
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmDinfBox, sizeof(MP4_SDSM_DINF_BOX_T), &size) == 0)
            return 0;

    if (mp4WriteSdsmDrefBox(pFile) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Write SDSM DREF Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmDrefBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_DREF_BOX_T tMp4SdsmDrefBox =
    {
        0x1c000000,         /* size */
        0x66657264,         /* type = "dref" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x0c000000,         /* size */
            0x206c7275,         /* type = "url " */
            0x01000000,         /* version_flags = 0x00000001 (self reference) */
        },
    };

    tMp4SdsmDrefBox.size = bSwap32(sizeof(MP4_SDSM_DREF_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmDrefBox, sizeof(MP4_SDSM_DREF_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SDSM STBL Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmStblBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_STBL_BOX_T tMp4SdsmStblBox =
    {
        0x00000000,         /* size */
        0x6c627473,         /* type = "stbl" */
    };

    size =  sizeof(MP4_SDSM_STBL_BOX_T) +
            sizeof(MP4_SDSM_STSD_BOX_T) +
            sizeof(MP4_SDSM_STTS_BOX_T) +
            sizeof(MP4_SDSM_STSC_BOX_T) +
            sizeof(MP4_SDSM_STSZ_BOX_T) +
            sizeof(MP4_SDSM_STCO_BOX_T);

    tMp4SdsmStblBox.size = bSwap32(size);

    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmStblBox, sizeof(MP4_SDSM_STBL_BOX_T), &size) == 0)
            return 0;

    if (mp4WriteSdsmStsdBox(pFile) == 0)
            return 0;

    if (mp4WriteSdsmSttsBox(pFile) == 0)
            return 0;

    if (mp4WriteSdsmStscBox(pFile) == 0)
            return 0;
    
    if (mp4WriteSdsmStszBox(pFile) == 0)
            return 0;

    if (mp4WriteSdsmStcoBox(pFile) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Write SDSM STSD Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmStsdBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_STSD_BOX_T tMp4SdsmStsdBox =
    {
        0x48000000,         /* size */
        0x64737473,         /* type = "stsd" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x38000000,         /* sample_description_size */
            0x7334706d,         /* data_format = "mp4s" */
            0x00, 0x00, 0x00, 0x00,     /* reserved[6] */   
            0x00, 0x00,
            0x0100,             /* data_reference_index = data_reference of this track */
            {
                0x28000000,     /* size */
                0x73647365,     /* type = "esds" */
                0x00000000,     /* version_flags */
                {
                    /* ES_Descriptor */
                    0x03,           /* esd_tag */
                    0x1a,           /* esd_tag_length */
                    0x0200,         /* esd_tag_es_id */
                    0x00,           /* esd_tag_flag */
                        /* DecoderConfigDescriptor */                              
                        0x04,           /* dcd_tag */
                        0x12,           /* dcd_tag_length */
                        0x02,           /* dcd_tag_obj_type_ind */
                        0x1400000d,     /* dcd_tag_flag_buf_size */
                        0x00000000,     /* dcd_tag_max_bitrate */
                        0x00000000,     /* dcd_tag_avg_bitrate */
                            /* DecoderSpecificInfo */
                            0x05,           /* dsi_tag */
                            0x03,           /* dsi_tag_length */
                            0x00, 0x00, 0x40,   /* dsi_tag_data[3] */
                        /* SLConfigDescriptor */
                        0x06,           /* slcd_tag */
                        0x01,           /* slcd_tag_length */
                        0x02,           /* slcd_tag_data */
                },
            },
        },
    };

    tMp4SdsmStsdBox.size = bSwap32(sizeof(MP4_SDSM_STSD_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmStsdBox, sizeof(MP4_SDSM_STSD_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SDSM STTS Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmSttsBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_STTS_BOX_T tMp4SdsmSttsBox =
    {
        0x18000000,         /* size */
        0x73747473,         /* type = "stts" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x01000000,         /* sample_count  */
            0x00000000,         /* sample_duration */
        },
    };

    tMp4SdsmSttsBox.size = bSwap32(sizeof(MP4_SDSM_STTS_BOX_T));
    tMp4SdsmSttsBox.mp4_sdsm_sample_time_entry[0].sample_duration = bSwap32(mp4VideDuration);
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmSttsBox, sizeof(MP4_SDSM_STTS_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SDSM STSC Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmStscBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_STSC_BOX_T tMp4SdsmStscBox =
    {
        0x1c000000,     /* size */
        0x63737473,     /* type = "stsc" */
        0x00000000,     /* version_flags */
        0x01000000,     /* number_of_entries */
        {
            0x01000000,     /* first_chunk */
            0x01000000,     /* samples_per_chunk */
            0x01000000,     /* sample_description_id = visual_sample_entry of this track */
        },
    };

    tMp4SdsmStscBox.size = bSwap32(sizeof(MP4_SDSM_STSC_BOX_T));
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmStscBox, sizeof(MP4_SDSM_STSC_BOX_T), &size) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Write SDSM STSZ Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmStszBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_STSZ_BOX_T tMp4SdsmStszBox =
    {
        0x14000000,         /* size */
        0x7a737473,         /* type = "stsz" */
        0x00000000,         /* version_flags */
        0x00000000,         /* sample_size = 0x???????? VOP count */
        0x01000000,         /* number_of_entries */
    };

    tMp4SdsmStszBox.sample_size = bSwap32(mp4VopCount);
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmStszBox, sizeof(MP4_SDSM_STSZ_BOX_T), &size) == 0)
            return 0;

    return 1;
}


/*

Routine Description:

    Write SDSM STCO Box.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4WriteSdsmStcoBox(FS_FILE* pFile)
{
    u32 size;
    __align(4) MP4_SDSM_STCO_BOX_T tMp4SdsmStcoBox =
    {
        0x14000000,         /* size */
        0x6f637473,         /* type = "stco" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x00000000,         /* offset */
        },
    };

    tMp4SdsmStcoBox.size = bSwap32(sizeof(MP4_SDSM_STCO_BOX_T));
    mp4FileOffset += sizeof(mp4ObjectDescription); 
    tMp4SdsmStcoBox.mp4_sdsm_chunk_offset_entry[0] = bSwap32(mp4FileOffset);
    if (dcfWrite(pFile, (unsigned char*)&tMp4SdsmStcoBox, sizeof(MP4_SDSM_STCO_BOX_T), &size) == 0)
            return 0;

    return 1;
}



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
s32 mp4FindSubBoxes(FS_FILE*    pFile, 
                    FS_i32      BoxLimit, 
                    u32*        pBoxType, 
                    FS_i32*     pPosition, 
                    u32*        pBoxNum)
{
    u32     size;
    FS_i32  FilePoint, FileCurrent;
    __align(4) MP4_BOX_HEADER_T mp4BoxHeader;
    
    FileCurrent = dcfTell(pFile);      // record original position
    
    *pBoxNum   = 0;

    while(((FilePoint = dcfTell(pFile)) < BoxLimit) && (*pBoxNum < MP4_BOX_NUM_MAX)) 
    {
        if(dcfRead(pFile, (u8*)&mp4BoxHeader, sizeof(MP4_BOX_HEADER_T), &size) == 0)
            return 0;
        
        *pBoxType++ = mp4BoxHeader.type;
        *pPosition++ = FilePoint;
        (*pBoxNum)++;
        
        size = bSwap32(mp4BoxHeader.size);
        dcfSeek(pFile, (s32)size - sizeof(MP4_BOX_HEADER_T), FS_SEEK_CUR);
    }
    
    if(*pBoxNum >= MP4_BOX_NUM_MAX) 
    {
        DEBUG_MP4("Box number too many!!!!\n");
    }
    
    dcfSeek(pFile, FileCurrent, FS_SEEK_SET);  // return to original position

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
    
    FileCurrent = dcfTell(pFile);      // record original position
    dcfSeek(pFile, 0, FS_SEEK_SET);
    
    *pFtyp  = 0;
    *pMoov  = 0;
    *pMdat  = 0;
    while(dcfRead(pFile, (u8*)&mp4AtomHeader, sizeof(MP4_BOX_HEADER_T), &size) == 1) {
        FilePoint   = dcfTell(pFile) - sizeof(MP4_BOX_HEADER_T);
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
        dcfSeek(pFile, (s32)size - sizeof(MP4_BOX_HEADER_T), FS_SEEK_CUR);
    }
    
    dcfSeek(pFile, FileCurrent, FS_SEEK_SET);  // return to original position

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
    
    FileCurrent = dcfTell(pFile);      // record original position
    
    while((FilePoint = dcfTell(pFile)) < MoovLimit) {
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
        dcfSeek(pFile, (s32)size - sizeof(MP4_BOX_HEADER_T), FS_SEEK_CUR);
    }
    
    dcfSeek(pFile, FileCurrent, FS_SEEK_SET);  // return to original position

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
    __align(4) MP4_FTYP_BOX_T tMp4FtypBox;  
    
    dcfSeek(pFile, FtypPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4FtypBox, sizeof(MP4_FTYP_BOX_T), &size) == 0)
        return 0;

    size = bSwap32(tMp4FtypBox.size); 

    if (sizeof(MP4_FTYP_BOX_T) != size) 
    {
        dcfSeek(pFile, (s32)size - sizeof(MP4_FTYP_BOX_T), FS_SEEK_CUR);
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

    dcfSeek(pFile, MoovPosition, FS_SEEK_SET);
    
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
    
    dcfSeek(pFile, MvhdPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, IodsPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, TrakPosition, FS_SEEK_SET);
    
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

    dcfSeek(pFile, TkhdPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, MdiaPosition, FS_SEEK_SET);
    
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

#ifdef MP4_AUDIO
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

    dcfSeek(pFile, MdhdPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, HdlrPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, MinfPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, DinfPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, DrefPosition, FS_SEEK_SET);
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

    dcfSeek(pFile, StblPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, StsdPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, StszPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, SttsPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, StscPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, StcoPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, StssPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, VmhdPosition, FS_SEEK_SET);
    if (dcfRead(pFile, (unsigned char*)&tMp4VideVmhdBox, sizeof(MP4_VIDE_VMHD_BOX_T), &size) == 0)
        return 0;

    return 1;
}


/************************************************************************/
/* MP4 Soun Track                           */
/************************************************************************/

#ifdef MP4_AUDIO

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

    dcfSeek(pFile, MinfPosition, FS_SEEK_SET);
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

    dcfSeek(pFile, DinfPosition, FS_SEEK_SET);
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

    dcfSeek(pFile, DrefPosition, FS_SEEK_SET);
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

    dcfSeek(pFile, StblPosition, FS_SEEK_SET);
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

    dcfSeek(pFile, StsdPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, StszPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, SttsPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, StscPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, StcoPosition, FS_SEEK_SET);

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

    dcfSeek(pFile, SmhdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&mp4SounSmhdAtom, sizeof(MP4_SOUN_SMHD_ATOM), &size) == 0)
        return 0;

    return 1;
}

#endif

/************************************************************************/
/* MP4 Object Descriptor Stream Track                   */
/************************************************************************/

/*

Routine Description:

    Read ODSM MINF Box.

Arguments:

    pFile           - File handle.
    MinfPosition    - Point to minf Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmMinfBox(FS_FILE* pFile, FS_i32 MinfPosition)
{
    u32     size;
    __align(4) MP4_ODSM_MINF_BOX_T tMp4OdsmMinfBox;
    FS_i32  MinfLimit, VmhdPosition, SmhdPosition, NmhdPosition, DinfPosition, StblPosition;

    dcfSeek(pFile, MinfPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4OdsmMinfBox, sizeof(MP4_ODSM_MINF_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4OdsmMinfBox.size);
    MinfLimit   = MinfPosition + size;

    mp4FindMinfSubBoxes(pFile, 
                        MinfLimit, 
                        &VmhdPosition, 
                        &SmhdPosition, 
                        &NmhdPosition, 
                        &DinfPosition, 
                        &StblPosition);

    if (DinfPosition >= 0)
        if (mp4ReadOdsmDinfBox(pFile, DinfPosition) == 0)
            return 0;

    if (StblPosition >= 0)
        if (mp4ReadOdsmStblBox(pFile, StblPosition) == 0)
            return 0;

    if (NmhdPosition >= 0)
        if (mp4ReadOdsmNmhdBox(pFile, NmhdPosition) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Read ODSM DINF Box.

Arguments:

    pFile           - File handle.
    DinfPosition    - Point to dinf Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmDinfBox(FS_FILE* pFile, FS_i32 DinfPosition)
{
    u32 size;
    __align(4) MP4_ODSM_DINF_BOX_T tMp4OdsmDinfBox;
    FS_i32  DinfLimit, DrefPosition;

    dcfSeek(pFile, DinfPosition, FS_SEEK_SET);
    if (dcfRead(pFile, (unsigned char*)&tMp4OdsmDinfBox, sizeof(MP4_ODSM_DINF_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4OdsmDinfBox.size);
    DinfLimit   = DinfPosition + size;

    mp4FindDinfSubBoxes(pFile, DinfLimit, &DrefPosition);

    if (DrefPosition >= 0)
        if (mp4ReadOdsmDrefBox(pFile, DrefPosition) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Read ODSM DREF Box.

Arguments:

    pFile           - File handle.
    DrefPosition    - Point to dref Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmDrefBox(FS_FILE* pFile, FS_i32 DrefPosition)
{
    u32 size;
    __align(4) MP4_ODSM_DREF_BOX_T tMp4OdsmDrefBox;

    dcfSeek(pFile, DrefPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4OdsmDrefBox, sizeof(MP4_ODSM_DREF_BOX_T), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read ODSM STBL Box.

Arguments:

    pFile           - File handle.
    StblPosition    - Point to stbl Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmStblBox(FS_FILE* pFile, FS_i32 StblPosition)
{
    u32     size;
    __align(4) MP4_ODSM_STBL_BOX_T tMp4OdsmStblBox;
    FS_i32  StblLimit, StsdPosition, StszPosition, SttsPosition, StscPosition, StcoPosition, StssPosition;

    dcfSeek(pFile, StblPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4OdsmStblBox, sizeof(MP4_ODSM_STBL_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4OdsmStblBox.size);
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
        if (mp4ReadOdsmStsdBox(pFile, StsdPosition) == 0)
            return 0;

    if (StszPosition >= 0)
        if (mp4ReadOdsmStszBox(pFile, StszPosition) == 0)
            return 0;

    if (SttsPosition >= 0)
        if (mp4ReadOdsmSttsBox(pFile, SttsPosition) == 0)
            return 0;

    if (StscPosition >= 0)
        if (mp4ReadOdsmStscBox(pFile, StscPosition) == 0)
            return 0;

    if (StcoPosition >= 0)
        if (mp4ReadOdsmStcoBox(pFile, StcoPosition) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Read ODSM STSD Box.

Arguments:

    pFile           - File handle.
    StsdPosition    - Point to stsd Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmStsdBox(FS_FILE* pFile, FS_i32 StsdPosition)
{
    u32 size;
    __align(4) MP4_ODSM_STSD_BOX_T tMp4OdsmStsdBox;

    dcfSeek(pFile, StsdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4OdsmStsdBox, sizeof(MP4_ODSM_STSD_BOX_T), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read ODSM STSZ Box.

Arguments:

    pFile           - File handle.
    StszPosition    - Point to stsz Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmStszBox(FS_FILE* pFile, FS_i32 StszPosition)
{
    u32 size;
    __align(4) MP4_ODSM_STSZ_BOX_T tMp4OdsmStszBox;

    dcfSeek(pFile, StszPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4OdsmStszBox, sizeof(MP4_ODSM_STSZ_BOX_T), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read ODSM STTS Box.

Arguments:

    pFile           - File handle.
    SttsPosition    - Point to stts Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmSttsBox(FS_FILE* pFile, FS_i32 SttsPosition)
{
    u32 size;
    __align(4) MP4_ODSM_STTS_BOX_T tMp4OdsmSttsBox;

    dcfSeek(pFile, SttsPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4OdsmSttsBox, sizeof(MP4_ODSM_STTS_BOX_T), &size) == 0)
        return 0;

//  mp4VideDuration = bSwap32(tMp4OdsmSttsBox.mp4_odsm_sample_time_entry[0].sample_duration);

    return 1;
}

/*

Routine Description:

    Read ODSM STSC Box.

Arguments:

    pFile           - File handle.
    StscPosition    - Point to stsc Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmStscBox(FS_FILE* pFile, FS_i32 StscPosition)
{
    u32 size;
    __align(4) MP4_ODSM_STSC_BOX_T tMp4OdsmStscBox;

    dcfSeek(pFile, StscPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4OdsmStscBox, sizeof(MP4_ODSM_STSC_BOX_T), &size) == 0)
        return 0;
    
    return 1;
}

/*

Routine Description:

    Read ODSM STCO Box.

Arguments:

    pFile           - File handle.
    StcoPosition    - Point to stbl Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmStcoBox(FS_FILE* pFile, FS_i32 StcoPosition)
{
    u32 size;
    __align(4) MP4_ODSM_STCO_BOX_T mp4OdsmStcoBox =
    {
        0x14000000,         /* size */
        0x6f637473,         /* type = "stco" */
        0x00000000,         /* version_flags */
        0x01000000,         /* number_of_entries */
        {
            0x00000000,         /* offset */
        },
    };

    dcfSeek(pFile, StcoPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&mp4OdsmStcoBox, sizeof(MP4_ODSM_STCO_BOX_T), &size) == 0)
        return 0;

//  mp4FileOffset   = bSwap32(mp4OdsmStcoBox.mp4_odsm_chunk_offset_entry[0]);

    return 1;
}

/*

Routine Description:

    Read ODSM NMHD Box.

Arguments:

    pFile           - File handle.
    NmhdPosition    - Point to nmhd Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmNmhdBox(FS_FILE* pFile, FS_i32 NmhdPosition)
{
    u32 size;
    __align(4) MP4_ODSM_NMHD_BOX_T tMp4OdsmNmhdBox;

    dcfSeek(pFile, NmhdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4OdsmNmhdBox, sizeof(MP4_ODSM_NMHD_BOX_T), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read ODSM TREF Box.

Arguments:

    pFile           - File handle.
    TrefPosition    - Point to tref Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadOdsmTrefBox(FS_FILE* pFile, FS_i32 TrefPosition)
{
    u32 size;
    __align(4) MP4_ODSM_TREF_BOX_T tMp4OdsmTrefBox;

    dcfSeek(pFile, TrefPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4OdsmTrefBox, sizeof(MP4_ODSM_TREF_BOX_T), &size) == 0)
        return 0;

    return 1;
}

/************************************************************************/
/* MP4 Scene Description Stream Track                   */
/************************************************************************/

/*

Routine Description:

    Read SDSM MINF Box.

Arguments:

    pFile           - File handle.
    MinfPosition    - Point to minf Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSdsmMinfBox(FS_FILE* pFile, FS_i32 MinfPosition)
{
    u32     size;
    __align(4) MP4_SDSM_MINF_BOX_T tMp4SdsmMinfBox;
    FS_i32  MinfLimit, VmhdPosition, SmhdPosition, NmhdPosition, DinfPosition, StblPosition;

    dcfSeek(pFile, MinfPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4SdsmMinfBox, sizeof(MP4_SDSM_MINF_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4SdsmMinfBox.size);
    MinfLimit   = MinfPosition + size;

    mp4FindMinfSubBoxes(pFile, 
                        MinfLimit, 
                        &VmhdPosition, 
                        &SmhdPosition, 
                        &NmhdPosition, 
                        &DinfPosition, 
                        &StblPosition);

    if (DinfPosition >= 0)
        if (mp4ReadSdsmDinfBox(pFile, DinfPosition) == 0)
            return 0;

    if (StblPosition >= 0)
        if (mp4ReadSdsmStblBox(pFile, StblPosition) == 0)
            return 0;

    if (NmhdPosition >= 0)
        if (mp4ReadSdsmNmhdBox(pFile, NmhdPosition) == 0)
            return 0;

    return 1;
}

/*

Routine Description:

    Read SDSM DINF Box.

Arguments:

    pFile           - File handle.
    DinfPosition    - Point to dinf Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSdsmDinfBox(FS_FILE* pFile, FS_i32 DinfPosition)
{
    u32     size;
    __align(4) MP4_SDSM_DINF_BOX_T tMp4SdsmDinfBox;
    FS_i32  DinfLimit, DrefPosition;

    dcfSeek(pFile, DinfPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4SdsmDinfBox, sizeof(MP4_SDSM_DINF_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4SdsmDinfBox.size);
    DinfLimit   = DinfPosition + size;

    mp4FindDinfSubBoxes(pFile, DinfLimit, &DrefPosition);

    if (DrefPosition >= 0)
        if (mp4ReadSdsmDrefBox(pFile, DrefPosition) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Read SDSM DREF Box.

Arguments:

    pFile           - File handle.
    DrefPosition    - Point to dref Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSdsmDrefBox(FS_FILE* pFile, FS_i32 DrefPosition)
{
    u32 size;
    __align(4) MP4_SDSM_DREF_BOX_T tMp4SdsmDrefBox;

    dcfSeek(pFile, DrefPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4SdsmDrefBox, sizeof(MP4_SDSM_DREF_BOX_T), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read SDSM STBL Box.

Arguments:

    pFile           - File handle.
    StblPosition    - Point to stbl Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSdsmStblBox(FS_FILE* pFile, FS_i32 StblPosition)
{
    u32     size;
    __align(4) MP4_SDSM_STBL_BOX_T tMp4SdsmStblBox;
    FS_i32  StblLimit, StsdPosition, StszPosition, SttsPosition, StscPosition, StcoPosition, StssPosition;

    dcfSeek(pFile, StblPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4SdsmStblBox, sizeof(MP4_SDSM_STBL_BOX_T), &size) == 0)
        return 0;

    size        = bSwap32(tMp4SdsmStblBox.size);
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
        if (mp4ReadSdsmStsdBox(pFile, StsdPosition) == 0)
            return 0;

    if (StszPosition >= 0)
        if (mp4ReadSdsmStszBox(pFile, StszPosition) == 0)
            return 0;

    if (SttsPosition >= 0)
        if (mp4ReadSdsmSttsBox(pFile, SttsPosition) == 0)
            return 0;

    if (StscPosition >= 0)
        if (mp4ReadSdsmStscBox(pFile, StscPosition) == 0)
            return 0;

    if (StcoPosition >= 0)
        if (mp4ReadSdsmStcoBox(pFile, StcoPosition) == 0)
            return 0;
    
    return 1;
}

/*

Routine Description:

    Read SDSM STSD Box.

Arguments:

    pFile           - File handle.
    StsdPosition    - Point to stsd Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSdsmStsdBox(FS_FILE* pFile, FS_i32 StsdPosition)
{
    u32 size;
    __align(4) MP4_SDSM_STSD_BOX_T tMp4SdsmStsdBox;

    dcfSeek(pFile, StsdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4SdsmStsdBox, sizeof(MP4_SDSM_STSD_BOX_T), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read SDSM STSZ Box.

Arguments:

    pFile           - File handle.
    StszPosition    - Point to stsz Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSdsmStszBox(FS_FILE* pFile, FS_i32 StszPosition)
{
    u32 size;
    __align(4) MP4_SDSM_STSZ_BOX_T tMp4SdsmStszBox;

    dcfSeek(pFile, StszPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4SdsmStszBox, sizeof(MP4_SDSM_STSZ_BOX_T), &size) == 0)
        return 0;
    //mp4VopCount = bSwap32(tMp4SdsmStszBox.sample_size);

    return 1;
}

/*

Routine Description:

    Read SDSM STTS Box.

Arguments:

    pFile           - File handle.
    SttsPosition    - Point to stts Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSdsmSttsBox(FS_FILE* pFile, FS_i32 SttsPosition)
{
    u32 size;
    __align(4) MP4_SDSM_STTS_BOX_T tMp4SdsmSttsBox;

    dcfSeek(pFile, SttsPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4SdsmSttsBox, sizeof(MP4_SDSM_STTS_BOX_T), &size) == 0)
        return 0;

    //mp4VideDuration = bSwap32(tMp4SdsmSttsBox.mp4_sdsm_sample_time_entry[0].sample_duration);

    return 1;
}

/*

Routine Description:

    Read SDSM STSC Box.

Arguments:

    pFile           - File handle.
    StscPosition    - Point to stsc Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSdsmStscBox(FS_FILE* pFile, FS_i32 StscPosition)
{
    u32 size;
    __align(4) MP4_SDSM_STSC_BOX_T tMp4SdsmStscBox;

    dcfSeek(pFile, StscPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4SdsmStscBox, sizeof(MP4_SDSM_STSC_BOX_T), &size) == 0)
        return 0;

    return 1;
}

/*

Routine Description:

    Read SDSM STCO Box.

Arguments:

    pFile           - File handle.
    StcoPosition    - Point to stbl Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSdsmStcoBox(FS_FILE* pFile, FS_i32 StcoPosition)
{
    u32 size;
    __align(4) MP4_SDSM_STCO_BOX_T tMp4SdsmStcoBox;

    dcfSeek(pFile, StcoPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4SdsmStcoBox, sizeof(MP4_SDSM_STCO_BOX_T), &size) == 0)
        return 0;

    //mp4FileOffset = bSwap32(tMp4SdsmStcoBox.mp4_sdsm_chunk_offset_entry[0]);

    return 1;
}

/*

Routine Description:

    Read SDSM NMHD Box.

Arguments:

    pFile           - File handle.
    NmhdPosition    - Point to nmhd Box file position.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 mp4ReadSdsmNmhdBox(FS_FILE* pFile, FS_i32 NmhdPosition)
{
    u32 size;
    __align(4) MP4_SDSM_NMHD_BOX_T tMp4SdsmNmhdBox;

    dcfSeek(pFile, NmhdPosition, FS_SEEK_SET);

    if (dcfRead(pFile, (unsigned char*)&tMp4SdsmNmhdBox, sizeof(MP4_SDSM_NMHD_BOX_T), &size) == 0)
        return 0;

    return 1;
}
/* Peter: 0727 E*/
#endif //#if (MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_MP4)

