/*

Copyright (c) 2008   2008 Mars Semiconductor Corp.


Module Name:

	avi.c

Abstract:

   	The routines of AVI file.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2006/06/01	Peter Hsu Create	

*/

#include "general.h"
#include "board.h"

#include "task.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "avi.h"
#include "asfapi.h"
#include "aviapi.h"
#include "mpeg4api.h"
#include "iisapi.h"
#include "iduapi.h"
#include "../../iis/inc/iis.h"  /*Peter 1109 S*/
#include "../idu/inc/idureg.h"
/*Peter 0707 S*/
#include "isuapi.h"
#include "ipuapi.h"
#include "siuapi.h"
#include "sysapi.h"
#if (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
#include "jpegapi.h" //Lsk : 090312
#endif
/*Peter 0707 E*/

#if(MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_AVI)

/*
 *********************************************************************************************************
 * Constant
 *********************************************************************************************************
 */

#define AVIFILEINFO_HASINDEX        0x00000010
#define AVIFILEINFO_MUSTUSEINDEX    0x00000020
#define AVIFILEINFO_ISINTERLEAVED   0x00000100
#define AVIFILEINFO_WASCAPTUREFILE  0x00010000
#define AVIFILEINFO_COPYRIGHTED     0x00020000
#define AVIHEADERSIZE               400
#define AVIFRAMEIDXFUFSIZE          14400


/*
 *********************************************************************************************************
 * Variable
 *********************************************************************************************************
 */

/* common */ 
//u32 aviHeaderSize, aviDataSize, aviIndexSize;
u32 AviHeaderBuf[AVIHEADERSIZE / 4];
u32 MoviChunkLen;
u32 RIFFLen;
u32 AviIdxChkPrevOffset;
u32 Idx1_Len,Idx1_mark;
AVI_INDEXENTRY AVIFrameIdxBuf[AVIFRAMEIDXFUFSIZE];
/*Peter 1109 S*/
AVI_INDEXENTRY  *AVIVideoIdx[AVIFRAMEIDXFUFSIZE];
AVI_INDEXENTRY  *AVIAudioIdx[AVIFRAMEIDXFUFSIZE];
/*Peter 1109 E*/
AVI_INDEXENTRY *AVIFrameIdx;

/* video related */
//u8  aviVideHeader[0x20];
u32 aviVideHeaderSize;
u16 aviVopWidth, aviVopHeight;
u32 aviVopCount;
u32 Framerate;
/*Peter 1109 S*/
s64 VideoNextPresentTime;
s32 MicroSecPerFrame;
/* audio related */
u32 aviAudioChunkCount;
u32 aviAudioPresentTime;
u32 aviVideoChunkCount;
u32 aviVideoPresentTime;

u32 aviVideoChunkSize;
u32 aviAudioChunkSize;

    
static FS_DISKFREE_T   *diskInfo;  //Lsk 090715
static u32             free_size;
static u32             bytes_per_cluster;

extern u32 IsuIndex;

extern u32 mpegflag;
extern u32 siuFrameNumInMP4, siuSkipFrameRate; /* Peter 20061106 */
/*Peter 1109 E*/
extern u32 mpeg4Width, mpeg4Height; /*Peter 1116 S*/
extern s32 isu_avifrmcnt;

WAVFORMATCHK    WavInfo;
/*Peter 1109 S*/
extern WAVEFORMAT iisPlayFormat;
extern WAVEFORMAT iisRecFormat;
extern u32 IISMode;        // 0: record, 1: playback, 2: receive and playback audio in preview mode
extern s64 IISTime;        // Current IIS playback time(micro second)
extern u32 IISTimeUnit;    // IIS playback time per DMA(micro second)
extern u32 iisPlayCount;   // IIS played chunk number
extern u32 iisTotalPlay;   // IIS total trigger playback number
/*Peter 1109 E*/

//WAVRIFFCHK      WavRiffChk;
//WAVDATACHK      WavDataChk;


/* index related */
extern u32  VideoPictureIndex;	/* cytsai: for armulator only */
extern u32  Vop_Type;            // 0: Intra frame, 1: Inter frame
extern u8   sysCaptureVideoStop;


extern u8 sysCaptureVideoStart;
extern u8 sysCaptureVideoStop;
/* playback related */
extern s64  Videodisplaytime[DISPLAY_BUF_NUM];

extern u32	DMA0_Busy;



extern u32 asfCaptureMode;
extern u32 WantChangeFile;
extern u32 LastAudio;
extern u32 LastVideo;
extern u8  GetLastAudio;
extern u8  GetLastVideo;
extern u32 asfTimeStatistics;
extern u8  Start_asfTimeStatistics;
extern u32 VideoTimeStatistics;
extern u8 DirectlyTimeStatistics;
extern u8 VideoRecFrameRate; //Lsk 090702 : use to calculate Pre-record length
extern u8  ResetPayloadPresentTime;
extern u32 Cal_FileTime_Start_Idx;
extern u32 AV_TimeBase ; //For capture
extern s32 asfRecFileNum;
#if MOTIONDETEC_ENA 
extern s32 MD_Diff;
extern u32 MD_FullTVRun;
extern u32 MD_HalfTVRun;
extern u32 MD_PanelRun;
#endif
extern u8 pwroff;

extern int dcfLasfEofCluster;
extern u8 system_busy_flag;
extern BOOLEAN MemoryFullFlag;
extern u32 curr_free_space;  //Lsk 090422 : for overwrite information
extern u32 curr_record_space;  //Lsk 090422 : for overwrite information
extern u8      SetIVOP;
extern u8 FreeSpaceControl;
extern u32 start_idx;
extern u32 end_idx;
extern u32 NVOPCnt;
extern u32 VideoDuration;

#if (AVSYNC == AUDIO_FOLLOW_VIDEO)
extern s64 Video_timebase ;        //For playback
extern u8  StartPlayBack;
extern u8 ResetPlayback;  			//Lsk 090401 : reset playback system
extern s64 IDUInterruptTime;  //Lsk 090326
#endif
extern u8 TVout_Generate_Pause_Frame;
extern u8 mpeg4taskrun;
extern u32 AudioPlayback;
extern u8  video_playback_speed;//for asf player level control  
#if (VIDEO_CODEC_OPTION == MJPEG_CODEC)	
extern u32 MJPG_Mode;    					//0: record, 1: playback 
#endif
extern u8 ReadOnce;
extern MP4_Option  Mp4Dec_opt;
#define AVI_DROP_FRAME_THRESHOLD        300  // default 10 frames

/**********************************************************************************************************
* extern Function
***********************************************************************************************************/
extern u32 TriggerModeGetMaxPacketCount(void);
extern u32 ManualModeGetMaxPacketCount(void);
extern void CheckEventTrigger(void);
extern void CheckRecordTimeUP(void);
extern void CheckWriteFinish(void);
extern void Warning_SDFull(void);
extern u32 IndicateRecordStatus(u32 timetick);
extern void EnterASFCapture(void);
extern void ExitASFCapture(void);
extern void CaptureModeOSDSetting(void);    
//#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
extern s32 dcfOverWriteDel(void);
//#endif

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */

/*Peter 1109 S*/
/* Audio */
s32 aviAudioInit(void);
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
/*Peter 1109 E*/

/* Peter: 0727 S*/
s32 aviCaptureVideoFile(s32 ZoomFactor);
/* Peter: 0727 E*/

/* Header chunk */
s32 aviWriteHeaderChunk(FS_FILE*);
s32 aviWriteFileHeaderPost(FS_FILE*);
void WriteAVIHeader(FS_FILE*, u32*, u32, u32, u32, WAVFORMATCHK*);
void WriteToHeader_Int(s32, PUTAVIBS_HEADER*);
s32 aviReadHeaderIndex(FS_FILE*, u32*, FS_i32*, u32*, WAVFORMATCHK*); /* Peter: 0707 */

/* Data chunk */
s32 aviWriteDataChunkPre(FS_FILE*);
s32 aviWriteAudioChunk(FS_FILE*, IIS_BUF_MNG*, u32*);
s32 aviWriteVideoChunk(FS_FILE*, VIDEO_BUF_MNG*, u32*);
s32 aviWriteDummyVideoChunk(FS_FILE* pFile, VIDEO_BUF_MNG* pMng, u32 *size);
s32 aviWriteVirtualAudioChunk(IIS_BUF_MNG*);
s32 aviWriteVirtualVideoChunk(VIDEO_BUF_MNG*);
/* Index chunk */
s32 aviWriteIndexChunk(FS_FILE*);
void aviMakeIndex(s32, s32);
void aviMakeAudioIndex(s32);
void aviMakeVideoIndex(s32, s32);
u32 EstimationFileSize(void);
    
/*Peter 1109 S*/
/* yc: 0814 S ==> Just for test */

void Output_Sem(void)
{
	#if 0
	u32 s_sem;

	/* yc:0814 S */		
	//*((volatile unsigned *)(0xd0020034)) = 0x0;	       /* setup GPIO1 to ouput port */
	/* yc:0814 S */
	
	/* bit0: DMA_CH0_START, bit2..1: iisTrgSemEvt, bit4..3: iisCmpSemEvt, bit6..5: VideoTrgSemEvt, bit8..7: VideoCmpSemEvt */
	/* bit9: DMA_CH1_START */
	s_sem = Gpio1Level;
	//s_sem &= 0xfc00;
	s_sem &= ~0x00001b99;
	//if (DmaCh0Cmd & 0x1)
	if (DMA0_Busy)
		s_sem |= 0x00000001;			/* bit0: DMA_CH0_START */
	
	if (DmaCh1Cmd & 0x1)
		s_sem |= 0x00000200;
	//s_sem |= ((*((volatile unsigned *)(0xc005001c)) & 0x1) << 9);		/* bit0: DMA_CH1_START */
	s_sem |= ((iisTrgSemEvt->OSEventCnt) <<11);
	s_sem |= ((iisCmpSemEvt->OSEventCnt) <<3);

	//if(VideoTrgSemEvt->OSEventCnt)
	//	s_sem ^= 0x00000020;
	//s_sem |= ((VideoTrgSemEvt->OSEventCnt) <<5);
	s_sem |= ((VideoCmpSemEvt->OSEventCnt) <<7);
	Gpio1Level = s_sem;
	#endif
}
/* yc: 0814 E ==> Just for test */
/*Peter 1109 E*/


/*
 *********************************************************************************************************
 * Function body
 *********************************************************************************************************
 */

/*

Routine Description:

	Initialize AVI file.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 aviInit(void)
{
    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    mpeg4Init();
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
    mjpgInit();
    #endif
	
#ifdef  AVI_AUDIO
	/* initialize audio */
	aviAudioInit();
	
	/* initialize audio */
    iisInit();
#endif
	
	return 1;	
}

/*

Routine Description:

	The test routine of AVI file.

Arguments:

	None.

Return Value:

	None.

*/
void aviTest(void)
{
	
}

/*

Routine Description:

	Capture AVI video.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
/*Peter 1109 S*/
s32 aviCaptureVideo(s32 ZoomFactor, u32 Mode)
{
    int     i;
    u8      err;

    asfCaptureMode              = Mode; //Lsk 100623
	/* video */
	VideoPictureIndex       = 0;	/* cytsai: for armulator only */
	VideoBufMngReadIdx  = 0;
	VideoBufMngWriteIdx = 0;
	aviVopCount             = 0;
	siuSkipFrameRate        = 0;
	MPEG4_Mode              = 0;    // 0: record, 1: playback
	for(i = 0; i < VIDEO_BUF_NUM; i++) {
	    VideoBufMng[i].buffer   = VideoBuf;
	}
	mpeg4SetVideoResolution(mpeg4Width, mpeg4Height);   /*Peter 1116 S*/


    IISTimeUnit             = (IIS_RECORD_SIZE * 1000) / IIS_SAMPLE_RATE;  /* milliscends */ /* Peter 070104 */

#ifdef  AVI_AUDIO
	/* audio */
	aviAudioChunkCount       = 0;
	aviAudioPresentTime      = 0;
	iisSounBufMngReadIdx    = 0;
	iisSounBufMngWriteIdx   = 0;
	IISMode                 = 0;    // 0: record, 1: playback, 2: receive and playback audio in preview mode
	/* initialize sound buffer */
	for(i = 0; i < IIS_BUF_NUM; i++) {
		iisSounBufMng[i].buffer     = iisSounBuf[i];
	}
#endif	


    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
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
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
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
    while(VideoTrgSemEvt->OSEventCnt > (VIDEO_BUF_NUM - 2)) {
        OSSemAccept(VideoTrgSemEvt);
    }
    while(VideoTrgSemEvt->OSEventCnt < (VIDEO_BUF_NUM - 2)) {
        OSSemPost(VideoTrgSemEvt);
    }
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
    }
    Output_Sem();

    #endif               

#ifdef  AVI_AUDIO
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
#endif	

/* Peter: 0727 S*/
        /* write video file */
    if (aviCaptureVideoFile(ZoomFactor) == 0)
    {
        /* reset the capture control if error */
        sysCaptureVideoStart = 0;
        sysCaptureVideoStop = 1;       
	}
/* Peter: 0727 E*/

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
    #if AUDIO_IN_TO_OUT
    while(iisPlaybackSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisPlaybackSemEvt);
    }
    #endif
    #endif
    
    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    /*
    while(VideoTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(VideoTrgSemEvt);
    }
    */
    OSSemSet(VideoTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
    }
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
    /*
    while(VideoTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(VideoTrgSemEvt);
    }
    */
  	OSSemSet(VideoTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
    }
    #endif               
    
    

    /* delay until mpeg4 and IIS task reach pend state */
#if(CHIP_OPTION == CHIP_PA9001D)
    OSTimeDly(2);
#else
    OSTimeDly(6);
#endif

/*CY 0613 S*/
	/* delay until mpeg4 task reach pend state */
	OSTimeDly(4);

    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
	/* suspend mpeg4 task */
	mpeg4SuspendTask();
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
    for(i = 0; !MJPEG_Pend; i++)
    {
        OSTimeDly(1);
    }
    mjpgSuspendTask();
    #endif
    
#ifdef  AVI_AUDIO
	iisStopRec();
	iisSuspendTask();
#endif

	#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
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
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
	/* refresh semaphore state */   
	Output_Sem();
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
    #endif               

#ifdef  AVI_AUDIO
	while(iisCmpSemEvt->OSEventCnt > 0) {
	OSSemAccept(iisCmpSemEvt);
	}
	Output_Sem();
	while(iisTrgSemEvt->OSEventCnt < (IIS_BUF_NUM - 2)) {
	OSSemPost(iisTrgSemEvt);
	}
	Output_Sem();
#endif	
/*CY 0613 E*/	

	DEBUG_AVI("AVI file captured - VOP count = %d\n", aviVopCount);
#ifdef  AVI_AUDIO
	DEBUG_AVI("AVI file captured - Audio count = %d\n", aviAudioChunkCount);
#endif	
	DEBUG_AVI("AVI file captured - File size = %d bytes\n\n", RIFFLen);

	// reset IIS hardware
	iisReset(IIS_SYSPLL_SEL_48M);

	return 1;
}
/*Peter 1109 E*/


/*

Routine Description:

    Create AVI file.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
FS_FILE* aviCreateFile(u8 flag)
{
    FS_FILE* pFile;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
	u8 tmp;
    aviVideoChunkSize = 0;
    aviAudioChunkSize = 0;
    aviVideoPresentTime  = 0;
    aviAudioPresentTime  = 0;
    aviVopCount = 0;
    aviAudioChunkCount = 0;
    RIFFLen     = 0;
	AVIFrameIdx = AVIFrameIdxBuf;

    //------------- for disk full control-------------//
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit
    
    if(!(asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA) && (free_size <= ((MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE*IIS_BUF_NUM))/1024)) //Notice: K-Byte unit
    {
        DEBUG_ASF("Disk full!!!\n");
        dcfLasfEofCluster =-1;
        system_busy_flag=1;
        #if 0
        #if ((HW_BOARD_OPTION == CWELL_DVRBOX) ||  ((HW_BOARD_OPTION == JSW_DVRBOX) && HVR_900_JAPEN))
            uiMenuOSDLayerObj(TVOSD_SizeX , OSD_WARNING , 16 , 24 , 152 , 98+osdYShift/2 , OSD_Blk2);
            osdDrawMessage(MSG_MEMORY_FULL, CENTERED, 126+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
        #else
        #if OSD_LANGUAGE_ENA
        osdDrawCWarningMessage(OSD_MSG_MEMORY_FULL, OSD_Blk2, CurrLanguage, TRUE, FALSE);
        #elif((HW_BOARD_OPTION == A1013_FPGA_BOARD) ||  (HW_BOARD_OPTION == A1013_REALCHIP_A) ||  (HW_BOARD_OPTION == A1016_FPGA_BOARD)||\
        (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
        uiOSDIconColorByXY(OSD_ICON_WARNING ,152 , 98+osdYShift/2 , OSD_Blk2, 0x00 , alpha_3);
        osdDrawMessage(MSG_MEMORY_FULL, CENTERED, 126+osdYShift/2, OSD_Blk2, 0xC0, 0x00);
        #else
        osdDrawWarningMessage("MEMORY FULL !",2,TRUE, FALSE);
        #endif
        #endif
        #if (HW_BOARD_OPTION == RDI_CARMREC)
    	 	gpioTimerCtrLed(LED_1_ON);
        #elif(HW_BOARD_OPTION==DTY_IRDVR)
    	 	gpioTimerCtrLed(LED_R_FLASH);
    		gpioTimerCtrLed(LED_B_ON);
            CaptureTrigMode=0;
            system_busy_flag=0;
        #elif(HW_BOARD_OPTION==WINNIN_IRDVR)
            CaptureTrigMode=0;
            system_busy_flag=0;
        #elif(HW_BOARD_OPTION == ALM_DVRBOX)
            gpioTimerCtrLed(LED_SD);
        #elif(HW_BOARD_OPTION == ACT611_DVRBOX)
            gpioTimerCtrLed(GPIO_CHECKBIT_G, LED_H);
            gpioTimerCtrLed(GPIO_CHECKBIT_B, LED_OFF);
            //gpioTimerCtrLed(GPIO_CHECKBIT_Y, LED_M);
        #endif
        #endif
        MemoryFullFlag = TRUE;
        OSTimeDly(15);
        #if 0
        #if Auto_Video_Test
        memset(&Video_Auto, 0, sizeof(Video_Auto));
        Video_Auto.VideoTest_Mode = 2;
        #endif
        #endif
        return 0;
    }


    //----Storge capacity control------//
    //#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
        #if FILE_SYSTEM_DVF_TEST
        if(1)
        #else
        if(asfCaptureMode & (ASF_CAPTURE_OVERWRITE_ENA | ASF_CAPTURE_EVENT_GSENSOR_ENA))
        #endif
        {
            free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;

            //Check filesystem capacity
            #if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR))
            while((free_size < DCF_OVERWRITE_THR_KBYTE)||(global_totalfile_count > (DCF_FILE_PER_DIR-20)))                   
            #else
            while((free_size < DCF_OVERWRITE_THR_KBYTE))                    
            #endif
            {   // Find the oldest file pointer and delete it
                DEBUG_DCF("1.Free Space=%d (KBytes) \n",free_size);            
                if(dcfOverWriteDel()==0)
                {
                    DEBUG_DCF("Over Write delete fail!!\n");
                    return 0;
                }
                else
                {
                    #if 0
                    #if(HW_BOARD_OPTION==WENSHING_SDV)
                    uiOverWriteFlag = 1;
                    #endif
                    #endif
                    //DEBUG_ASF("Over Write delete Pass!!\n");
                }
                free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
                DEBUG_DCF("2.Free Space=%d (KBytes) \n",free_size);
            }
        }
  	    curr_free_space = free_size;
    	curr_record_space = 0;
    //#endif

    OS_ENTER_CRITICAL();
    RTCseconds                  = 0;
    OS_EXIT_CRITICAL();

    /* create next file */
	if ((pFile = dcfCreateNextFile(DCF_FILE_TYPE_AVI, 0)) == NULL) {
		DEBUG_AVI("AVI create file error!!!\n");
		return 0;
	}

  	/* write header object */
	if(aviWriteHeaderChunk(pFile) == 0) {
		DEBUG_AVI("AVI write file header error!!!\n");
	    dcfCloseFileByIdx(pFile, 0, &tmp);
	    return 0;
	}
	
	/* write data object pre */
	aviWriteDataChunkPre(pFile);


    return pFile;
}


/*

Routine Description:

    Close AVI file.

Arguments:

    pFile - File handle.

Return Value:

    0 - Failure.
    1 - Success.

*/
s32 aviCloseFile(FS_FILE* pFile)
{
	u8 tmp;
    /* write index object */
	if(aviWriteIndexChunk(pFile) == 0) {
		DEBUG_AVI("AVI write index chunk error!!!\n");
	    dcfCloseFileByIdx(pFile, 0, &tmp);
	    return 0;
	}
		
	/* write header object post */
	if(aviWriteFileHeaderPost(pFile) == 0) {
		DEBUG_AVI("AVI write file header post error!!!\n");
	    dcfCloseFileByIdx(pFile, 0, &tmp);
	    return 0;
	}
	
	/* write data object post */
	//aviWriteDataObjectPost(pFile);
	
	/* close file */
    if(dcfCloseFileByIdx(pFile, 0, &tmp) == 0) {
        DEBUG_AVI("Close file error!!!\n");
        return 0;
    }
    
    //----Storge capacity control------//
#if 1    
    diskInfo            = &global_diskInfo;
    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
    free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2; //KByte unit
    //#if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
        #if FILE_SYSTEM_DVF_TEST
        if(1)
        #else
        if(asfCaptureMode & (ASF_CAPTURE_OVERWRITE_ENA | ASF_CAPTURE_EVENT_GSENSOR_ENA))
        #endif
        {
            free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
            //Check filesystem capacity
            #if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR))
            while((free_size < DCF_OVERWRITE_THR_KBYTE)||(global_totalfile_count > (DCF_FILE_PER_DIR-20)))                   
            #else
            while((free_size < DCF_OVERWRITE_THR_KBYTE))                    
            #endif
            {   // Find the oldest file pointer and delete it
                DEBUG_AVI("1. Free Space=%d (KBytes) \n",free_size);  
                if(dcfOverWriteDel()==0)
                {
                    DEBUG_DCF("Over Write delete fail!!\n");
                    return 0;
                }
                else
                {
                    #if 0
                    #if(HW_BOARD_OPTION==WENSHING_SDV)
                    uiOverWriteFlag = 1;
                    #endif
                    #endif
                    //DEBUG_ASF("Over Write delete Pass!!\n");
                }
                free_size           = diskInfo->avail_clusters * (bytes_per_cluster/512)/2;
                DEBUG_DCF("2.Free Space=%d (KBytes) \n",free_size);
            }
        }
  	    curr_free_space = free_size;
    	curr_record_space = 0;
    //#endif

    return 1;
#endif
}
/*

Routine Description:

	Capture AVI video file.

Arguments:

	None.

Return Value:
	
	0 - Failure.
	1 - Success.
		
*/
s32 aviCaptureVideoFile(s32 ZoomFactor)
{
    FS_FILE*        pFile;
    u16             video_value;
    u16             video_value_max;
    u32             monitor_value;
    u32             timetick;


#ifdef  ASF_AUDIO
    u16             audio_value;
    u16             audio_value_max;
#endif      
    
    u32             CurrentFileSize;
    u32             DummySize;
    u8              err;
#if FINE_TIME_STAMP
    s32             TimeOffset;
#endif

#if(SHOW_UI_PROCESS_TIME == 1)
    u32             time1;
#endif



    s32             SkipFrameNum;
    u32             PreRecordFrameNum;

    u32             MaxPacketCount;
    u8              TriggerModeFirstFileFlag = 0;
    u8              InitDACPlayFlag = 0;    
    
    u32             SingleFrameSize;
    u32             FreeSpaceThreshold;
    u32             FreeSpace;

	u32 VidoutLen;
	u32 AudOutLen;
    u8  NotWrite = 0;
    u32 Fixed_FPS;
	u8 tmp;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
    
    
    #if (VIDEO_CODEC_OPTION == MJPEG_CODEC)	
    MJPG_Mode = 0;
    #endif
    Framerate   = 8;

    asfSectionTime = 300;
    
#if ((HW_BOARD_OPTION==DTY_IRDVR)||(HW_BOARD_OPTION == ES_LIGHTING))
        u32             timeoutcnt=0;
#endif

    VideoTimeStatistics = 0;
    asfTimeStatistics = 0;
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
    //if(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)
    {
        if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_30)
        {   
            Fixed_FPS = 30;
            PreRecordFrameNum = PreRecordTime * 30;
        }
        else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_15)
        {   
            Fixed_FPS = 15;
            PreRecordFrameNum = PreRecordTime * 15;
        }
        else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_5)
        {   
            Fixed_FPS = 5;
            PreRecordFrameNum = PreRecordTime * 5;
        }
        else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_60)
        {   
            Fixed_FPS = 60;
            PreRecordFrameNum = PreRecordTime * 60;
        }
        else if(VideoRecFrameRate==MPEG4_VIDEO_FRAMERATE_10)
        {   
            Fixed_FPS = 10;
            PreRecordFrameNum = PreRecordTime * 10;
        }
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
        if((pFile = aviCreateFile(1)) == 0) {            
            return 0;
        }
    }

    DEBUG_AVI("asfSectionTime = %d\n", asfSectionTime);
    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    DEBUG_AVI("VideoCmpSemEvt = %d\n", VideoCmpSemEvt->OSEventCnt);
    DEBUG_AVI("VideoTrgSemEvt = %d\n", VideoTrgSemEvt->OSEventCnt);
    DEBUG_AVI("VideoCpleSemEvt = %d\n", VideoCpleSemEvt->OSEventCnt);   
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
    DEBUG_AVI("VideoCmpSemEvt = %d\n", VideoCmpSemEvt->OSEventCnt);
    DEBUG_AVI("VideoTrgSemEvt = %d\n", VideoTrgSemEvt->OSEventCnt);
    DEBUG_AVI("VideoCpleSemEvt = %d\n", VideoCpleSemEvt->OSEventCnt);
    #endif               
    
    video_value     = 0;
    video_value_max = 0;
    monitor_value   = 0;
#ifdef  ASF_AUDIO
    DEBUG_AVI("iisCmpSemEvt = %d\n", iisCmpSemEvt->OSEventCnt);
    DEBUG_AVI("iisTrgSemEvt = %d\n", iisTrgSemEvt->OSEventCnt);
    audio_value     = 0;
    audio_value_max = 0;
#endif      

    iduCaptureVideo(mpeg4Width,mpeg4Height);

#if(SHOW_UI_PROCESS_TIME == 1)
    time1=OSTimeGet();
#endif
    isuCaptureVideo(ZoomFactor);
    
    ipuCaptureVideo();

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
    iisStartRec();
    #if AUDIO_IN_TO_OUT
    iisResumePlaybackTask();
	#endif  	
    #endif  // #ifdef ASF_AUDIO
    siuCaptureVideo(ZoomFactor);
    


#if ( (LCM_OPTION == LCM_HX8224)||(LCM_OPTION == LCM_TMT035DNAFWU24_320x240)||(LCM_OPTION == LCM_HX8224_SRGB) || (LCM_OPTION == LCM_HX8817_RGB)||(LCM_OPTION == LCM_HX8257_RGB666_480x272)||(LCM_OPTION == LCM_HX8257_SRGB_480x272)||(LCM_OPTION == LCM_HX8257_P_RGB_480x272)||(LCM_OPTION == LCM_TD036THEA3_320x240) || (LCM_OPTION == LCM_HX8224_601) || (LCM_OPTION == LCM_HX8224_656) || (LCM_OPTION == LCM_HX5073_RGB) || (LCM_OPTION == LCM_HX5073_YUV) ||  (LCM_OPTION == LCM_TPG105) || (LCM_OPTION ==LCM_A015AN04) || (LCM_OPTION == LCM_TD020THEG1)||(LCM_OPTION == LCM_GPG48238QS4)||(LCM_OPTION == LCM_A024CN02)||(LCM_OPTION == LCM_CCIR601_640x480P)||(LCM_OPTION == LCM_TJ015NC02AA)||(LCM_OPTION == LCM_LQ035NC111))
    SyncIduIsu();
#endif

    CaptureModeOSDSetting();

    /************   Start to REC   ************/
    #if( (ISU_OUT_BY_FID) || (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC) )

        #if ((HW_BOARD_OPTION==DTY_IRDVR)||(HW_BOARD_OPTION == ES_LIGHTING))
            timeoutcnt=0;
            while(isu_avifrmcnt < 4)
            {
                DEBUG_AVI("asf w1\n");

               OSTimeDly(1);
               timeoutcnt++;
               if ( timeoutcnt>10)
               {
                   DEBUG_AVI("asf t1\n");
                   //DEBUG_AVI("Error: timeout 1\n");
                   break;
               }
               
            }

            isu_avifrmcnt=0;
            mp4_avifrmcnt=0;
            OSSemSet(isuSemEvt, 0, &err);    
            sysReady2CaptureVideo=1;
            timeoutcnt=0;
            while(isu_avifrmcnt < 1)
            {
                DEBUG_AVI("asf w2\n");

               OSTimeDly(1);
               timeoutcnt++;
               if ( timeoutcnt>5)
               {
                   DEBUG_AVI("asf t2\n");
                   DEBUG_AVI("Error: timeout 2\n");
                   break;
               }
               
            }

        #else
        while(isu_avifrmcnt < MAX_VIDEO_FRAME_BUFFER)
        {
           OSTimeDly(1);
        }

        isu_avifrmcnt=0;
        mp4_avifrmcnt=0;
        OSSemSet(isuSemEvt, 0, &err);    
        sysReady2CaptureVideo=1;
        while(isu_avifrmcnt < MAX_VIDEO_FRAME_BUFFER)
        {
          OSTimeDly(1);
        }
        isu_avifrmcnt=0;
        mp4_avifrmcnt=0;
        OSSemSet(isuSemEvt, 0, &err);    
        sysReady2CaptureVideo=1;
        while(isu_avifrmcnt < MAX_VIDEO_FRAME_BUFFER)
        {
          OSTimeDly(1);
        }

        isu_avifrmcnt=0;
        mp4_avifrmcnt=0;
        OSSemSet(isuSemEvt, 0, &err);    
        sysReady2CaptureVideo=1;
        while(isu_avifrmcnt < MAX_VIDEO_FRAME_BUFFER)
        {
          OSTimeDly(1);
        }
        #endif
    #else
        sysReady2CaptureVideo=1;
        while(isu_avifrmcnt < 1)
        {
            OSTimeDly(1);
        }
    #endif

    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    if(MPEG4_Task_Go) {
        OSSemAccept(VideoTrgSemEvt);
    }
    mpeg4ResumeTask();    
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
  	if(MJPG_Task_Go) {
        OSSemAccept(VideoTrgSemEvt);
    }
    mjpgResumeTask();
    #endif               
    


    /*CY 0613 S*/
    DEBUG_AVI("\n------------------Start to REC------------------\n");
    timetick = OS_tickcounter;
    while (sysCaptureVideoStop == 0)
    /*CY 0613 E*/
    {    
        if(MPEG4_Error ==1)
        {
            aviCloseFile(pFile);            
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
            #if (HW_BOARD_OPTION == ES_LIGHTING)
            if(EventTrigger == CAPTURE_STATE_TEMP)
            {
               EventTrigger = CAPTURE_STATE_WAIT ;
            }
            #endif
            if(EventTrigger == CAPTURE_STATE_WAIT) 
    		{
    		    //Start_MPEG4TimeStatistics = 0;
    		    CheckEventTrigger();                        
                if(EventTrigger == CAPTURE_STATE_TRIGGER)
                {
                    DEBUG_AVI("\n\nFSM : CAPTURE_STATE_TRIGGER\n\n");
                    /****************************************************************
                    *** Calculate how many VOP in SDRAM need to drop              ***
                    ****************************************************************/
                    OS_ENTER_CRITICAL();                                        
                    if(VideoBufMngWriteIdx >= VideoBufMngReadIdx)
                    {
                        //---R---s--W---
                        if((VideoBufMngWriteIdx - PreRecordFrameNum >= VideoBufMngReadIdx)&&(VideoBufMngWriteIdx - PreRecordFrameNum <= VideoBufMngWriteIdx))
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
                        DEBUG_AVI("FreeSpace control (%d,%d)\n", FreeSpaceThreshold, FreeSpace);                    
                    }
                    else
                        FreeSpaceControl = 0;

                    DEBUG_AVI("SkipFrameNum = %d\n", SkipFrameNum);
        
                    while((VideoBufMng[VideoBufMngReadIdx].flag != FLAG_I_VOP) || (SkipFrameNum > 0) 
                        || (FreeSpaceControl && (FreeSpace < FreeSpaceThreshold))) 
                    {
                        SkipFrameNum--;
                        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                        video_value = OSSemAccept(VideoCmpSemEvt);
                        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
		    			video_value = OSSemAccept(VideoCmpSemEvt);                    
                        #endif 
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
        				    DEBUG_AVI("\n\nCan't start from I frame!!!\n\n");
                            EventTrigger = CAPTURE_STATE_WAIT;
                            break;
                        }
                    }
                     start_idx = VideoBufMngReadIdx;
                    
                    if(start_idx != end_idx)
                    {
                        DEBUG_AVI("\n Warring!!! lose video slice....\n");
                    }                                       
                    #ifdef ASF_AUDIO
                    while ((aviAudioPresentTime + IIS_CHUNK_TIME) <= aviVideoPresentTime) 
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
                        if((pFile = aviCreateFile(0)) == 0) {
                            return 0;
                        }
                        TriggerModeFirstFileFlag = 1;                    
                    }
                    #elif (TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
                    if(TriggerModeFirstFileFlag == 0)
                    {
                        if((pFile = aviCreateFile(0)) == 0) {
                            return 0;
                        }
                        TriggerModeFirstFileFlag = 1;
                    }
                    else
                    {
                        /*** Reset Audio/Video time biase ***/
                        ResetPayloadPresentTime = 0; //reset video time base
                        AV_TimeBase  = PREROLL;
                        if((pFile = aviCreateFile(0)) == 0) {
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
                    DEBUG_AVI("\n\nFSM : CAPTURE_STATE_TIMEUP\n\n");
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
                    if(aviVideoPresentTime >= aviAudioPresentTime )
                    {   		
                		AV_TimeBase = aviVideoPresentTime + 100; // 0.1s suspend               
                    }
                    else
                    {
                        AV_TimeBase = aviAudioPresentTime + 100; // 0.1s suspend
                    }                    
                    /*** Close ASF file ***/
                    #elif(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE)
                    if(aviCloseFile(pFile) == 0) {
                        return 0;
                    }
                    #endif
                    DEBUG_AVI("\n\nFSM : CAPTURE_STATE_WAIT\n\n");                
                }                      
                #if (HW_BOARD_OPTION == ES_LIGHTING)
                EventTrigger = CAPTURE_STATE_TEMP;
                #endif
            }
        }
        /**********************************
        **** Write Audio/Video Payload ****
        **********************************/
        if( (EventTrigger == CAPTURE_STATE_TRIGGER) || (EventTrigger == CAPTURE_STATE_TIMEUP) || (!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))
        {
            #ifdef ASF_AUDIO
            // ------Write audio payload------//
            if((video_value == 0) || (aviAudioPresentTime <= aviVideoPresentTime))
            {
                {
                    audio_value = OSSemAccept(iisCmpSemEvt);
                    if (audio_value > 0) {
                        if(audio_value_max < audio_value)
                            audio_value_max = audio_value;

                        if(aviWriteAudioChunk(pFile, &iisSounBufMng[iisSounBufMngReadIdx], &AudOutLen) == 0) {
            				DEBUG_AVI("AVI write audio chunk error!!!\n");
				            dcfCloseFileByIdx(pFile, 0, &tmp);                            
        				    return 0;
		            	}   
            			aviMakeAudioIndex(AudOutLen);
                        iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
                        OSSemPost(iisTrgSemEvt);
                    }
                }
            }
            //------ Write video payload------//            
            if((audio_value == 0) || (aviAudioPresentTime >= aviVideoPresentTime))
            {
            #endif      // ASF_AUDIO
                {
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                    video_value = OSSemAccept(VideoCmpSemEvt);
                    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
					video_value = OSSemAccept(VideoCmpSemEvt);                    
                    #endif               
                    
                    if (video_value > 0) 
                    {
                        if(video_value_max < video_value)
                            video_value_max = video_value;
                        
                        if(!Start_asfTimeStatistics)
                        {
                            if(Cal_FileTime_Start_Idx== VideoBufMngReadIdx)
                            {
                                Start_asfTimeStatistics = 1;
                                asfTimeStatistics = 0;
                                #if (HW_BOARD_OPTION == ES_LIGHTING)
                                OS_ENTER_CRITICAL();
                                DirectlyTimeStatistics = 1;
                                OS_EXIT_CRITICAL();
                                #endif

                            }
                        }
                        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                        if(1)
                        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
                        if(NotWrite > 1) //the first frame alaways encode error, after system boot                     
                        #endif               
                        {                            
                            if((aviVideoPresentTime * Fixed_FPS) > (aviVopCount * 1000))                            
                            {   
                                DEBUG_AVI("d");
                                if(aviWriteDummyVideoChunk(pFile, &VideoBufMng[VideoBufMngReadIdx], &VidoutLen) == 0)
                                {
                                    DEBUG_AVI("AVI write video chunk error!!!\n");
                    	    		dcfCloseFileByIdx(pFile, 0, &tmp);                                
                    	    	    return 0;
                			    }                            
    	    					aviMakeVideoIndex(VidoutLen, VideoBufMng[VideoBufMngReadIdx].flag & FLAG_I_VOP);                                                                                                
                            }

                            if(aviWriteVideoChunk(pFile, &VideoBufMng[VideoBufMngReadIdx], &VidoutLen) == 0) {
                                DEBUG_AVI("AVI write video chunk error!!!\n");
                	    		dcfCloseFileByIdx(pFile, 0, &tmp);                                
                	    	    return 0;
                			}                            
    						aviMakeVideoIndex(VidoutLen, VideoBufMng[VideoBufMngReadIdx].flag & FLAG_I_VOP);
                            VideoBufMngReadIdx = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                            //DEBUG_AVI("Trace: MPEG4 frame written.\n");
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                            OSSemPost(VideoTrgSemEvt);              
                            #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
    						OSSemPost(VideoTrgSemEvt);                        
                            #endif               
                        }
                        else
                        {
                            NotWrite += 1;
                            VideoBufMngReadIdx = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                            //DEBUG_AVI("Trace: MPEG4 frame written.\n");
                            #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                            OSSemPost(VideoTrgSemEvt);              
                            #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
    						OSSemPost(VideoTrgSemEvt);                        
                            #endif     
                            
                        }
                        
                    }                                                            
                }
                
            #ifdef  ASF_AUDIO
            }
            // Skip siu frames for release bandwidth to SD card writing
            monitor_value   = (video_value > audio_value) ? video_value : audio_value;
            #else
            monitor_value   = video_value;
            #endif            

            #if 0  //Close file by size
            if(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) {                

                #if(TRIGGER_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
                /**********************************
                **** Check File Size           ****
                **********************************/
                if(VideoBufMng[VideoBufMngReadIdx].flag == FLAG_I_VOP)
                {
                    if(asfDataPacketCount  > MaxPacketCount)
                    {
                        DEBUG_AVI("\n\n\n File Size reach limit\n\n\n");
                        asfCloseFile(pFile);
                        /*** Reset Audio/Video time biase ***/
                        ResetPayloadPresentTime = 0; //reset video time base
                        AV_TimeBase  = PREROLL;
                        //DEBUG_AVI("MPEG4 UseSem :%04d, IIS UseSem :%04d\n", VideoCmpSemEvt->OSEventCnt,iisCmpSemEvt->OSEventCnt);  
                        //DEBUG_AVI("=====================================\n");                          
                        if((pFile = asfCreateFile(0)) == 0) 
                            return 0;
                    }                                      
                }
                #endif                     
            }
            #endif
        }   // if((asfCaptureMode != ASF_CAPTURE_EVENT) || (EventTrigger == 2))

    #if ((HW_BOARD_OPTION==DTY_IRDVR)||(HW_BOARD_OPTION==WINNIN_IRDVR)||(HW_BOARD_OPTION==EVERSPRING_DVRBOX)||(HW_BOARD_OPTION == ES_LIGHTING))
        if(video_value < 3)
    #else
        //DEBUG_AVI("(%d) ",monitor_value);
        if(monitor_value < 2)
    #endif
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
             Lucian: 以Audio/Video bitstream buffer 內的index剩餘個數為偵測點,若大於 AVI_DROP_FRAME_THRESHOLD
                     則為SD 寫入速度過慢,需drop frame.

        */
        if(asfCaptureMode & ASF_CAPTURE_EVENT_ALL) //Event trigger mode
        {
            if(EventTrigger == CAPTURE_STATE_WAIT && 
               (CurrentVideoSize > (MPEG4_MAX_BUF_SIZE - MPEG4_MIN_BUF_SIZE * 4) || 
                #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                VideoCmpSemEvt->OSEventCnt > (VIDEO_BUF_NUM - 60) ||
                #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)			
   				VideoCmpSemEvt->OSEventCnt > (VIDEO_BUF_NUM - 60) ||
                #endif                               
                iisCmpSemEvt->OSEventCnt > (IIS_BUF_NUM - 16))) {
                do {
                    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                    video_value = OSSemAccept(VideoCmpSemEvt);
                    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
   					video_value = OSSemAccept(VideoCmpSemEvt);
                    #endif               
                    if (video_value > 0) {
                        if(video_value_max < video_value)
                            video_value_max = video_value;
                        aviWriteVirtualVideoChunk(&VideoBufMng[VideoBufMngReadIdx]);
                        VideoBufMngReadIdx  = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
                        #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
                        OSSemPost(VideoTrgSemEvt);              
                        #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
                        OSSemPost(VideoTrgSemEvt);              						                        
                        #endif                                       
                    } else {
                        break;
                    }
                } while(VideoBufMng[VideoBufMngReadIdx].flag != FLAG_I_VOP);

                #if CDVR_LOG
                LogFileStart    = (LogFileStart + 1) % LOG_INDEX_NUM;
                #endif

                #ifdef  ASF_AUDIO
                while ((aviAudioPresentTime + IIS_CHUNK_TIME) <= aviVideoPresentTime) {
                    audio_value = OSSemAccept(iisCmpSemEvt);
                    if (audio_value > 0) {
                        if(audio_value_max < audio_value)
                            audio_value_max = audio_value;
                        aviWriteVirtualAudioChunk(&iisSounBufMng[iisSounBufMngReadIdx]);
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

            if (monitor_value < AVI_DROP_FRAME_THRESHOLD) 
            {       
                siuSkipFrameRate    = 0;                    
            } 
            else 
            {
               //DEBUG_AVI("z",monitor_value);
               DEBUG_AVI("z");
            }
        #else
            if (monitor_value < AVI_DROP_FRAME_THRESHOLD) 
            {       // not skip siu frame
                if(siuSkipFrameRate != 0) 
                {
                    siuSkipFrameRate    = 0;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (AVI_DROP_FRAME_THRESHOLD + 10)) {
                if(siuSkipFrameRate != 2) {
                    siuSkipFrameRate    = 2;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (AVI_DROP_FRAME_THRESHOLD + 20)) {
                if(siuSkipFrameRate != 4) {
                    siuSkipFrameRate    = 4;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (AVI_DROP_FRAME_THRESHOLD + 30)) {
                if(siuSkipFrameRate != 6) {
                    siuSkipFrameRate    = 6;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (AVI_DROP_FRAME_THRESHOLD + 40)) {
                if(siuSkipFrameRate != 8) {
                    siuSkipFrameRate    = 8;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (AVI_DROP_FRAME_THRESHOLD + 50)) {
                if(siuSkipFrameRate != 10) {
                    siuSkipFrameRate    = 10;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (AVI_DROP_FRAME_THRESHOLD + 60)) {
                if(siuSkipFrameRate != 12) {
                    siuSkipFrameRate    = 12;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (AVI_DROP_FRAME_THRESHOLD + 70)) {
                if(siuSkipFrameRate != 16) {
                    siuSkipFrameRate    = 16;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (AVI_DROP_FRAME_THRESHOLD + 80)) {
                if(siuSkipFrameRate != 20) {
                    siuSkipFrameRate    = 20;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (AVI_DROP_FRAME_THRESHOLD + 90)) {
                if(siuSkipFrameRate != 24) {
                    siuSkipFrameRate    = 24;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else if(monitor_value < (AVI_DROP_FRAME_THRESHOLD + 100)) {
                if(siuSkipFrameRate != 28) {
                    siuSkipFrameRate    = 28;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            } else {
                if(siuSkipFrameRate != 32) {
                    siuSkipFrameRate    = 32;
                    DEBUG_AVI("siuSkipFrameRate =  %d / 32\n", siuSkipFrameRate);
                }
            }
        #endif
            
        }   // asfCaptureMode != ASF_CAPTURE_EVENT

        /********************************************************
        *** SD capacity detection                             ***
        ********************************************************/
        if( (EventTrigger == CAPTURE_STATE_TRIGGER) || (EventTrigger == CAPTURE_STATE_TIMEUP) || (!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)))
        {                    
            //-------------Detect Disk Full---------------------//                       
            CurrentFileSize =  EstimationFileSize();
            if(!(asfCaptureMode & ASF_CAPTURE_OVERWRITE_ENA)) {
                if(((CurrentFileSize/1024) >= (free_size - (MPEG4_MAX_BUF_SIZE + IIS_CHUNK_SIZE*IIS_BUF_NUM)/1024)) ) 
                {
                    DEBUG_ASF("Disk full!!!\n");
                    DEBUG_ASF("free_size     = %d K-bytes, CurrentFileSize = %d bytes.\n", free_size, CurrentFileSize);
                    sysCaptureVideoStart    = 0;
                    sysCaptureVideoStop     = 1;
                    MemoryFullFlag          = TRUE;
                    dcfLasfEofCluster =-1;
                    Warning_SDFull();
                    #if Auto_Video_Test
                    memset(&Video_Auto, 0, sizeof(Video_Auto));
                    Video_Auto.VideoTest_Mode = 2;
                    #endif
                    break;
                }
            }
            else
            {
                #if ((FILE_SYSTEM_SEL == FILE_SYSTEM_DVR)||(FILE_SYSTEM_SEL == FILE_SYSTEM_CDVR))
                #if FILE_SYSTEM_DVF_TEST
                if(1)
                #else
                if(asfCaptureMode & (ASF_CAPTURE_OVERWRITE_ENA | ASF_CAPTURE_EVENT_GSENSOR_ENA))
                #endif
                {

                	diskInfo            = &global_diskInfo;
            	    bytes_per_cluster   = diskInfo->sectors_per_cluster * diskInfo->bytes_per_sector;
                                		
                    while((curr_free_space - ((CurrentFileSize)/1024)) < DCF_OVERWRITE_THR_KBYTE)
            		{               		    
                        DEBUG_AVI("1. curr_free_space, CurrentFileSize = %d, %d\n",curr_free_space,CurrentFileSize/1024);            		
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
            			curr_free_space = free_size;
                        DEBUG_AVI("2. curr_free_space, CurrentFileSize = %d, %d\n",curr_free_space,CurrentFileSize/1024);            		
            		}
            	}                
                #endif
            }
        }
        /************************************
        *** Change file by size or slice ***
        ************************************/        
        if(!(asfCaptureMode & ASF_CAPTURE_EVENT_ALL)) {
            if(WantChangeFile == 1) 
            {
                if ((GetLastAudio == 1) && (((LastAudio + 1) % IIS_BUF_NUM) == iisSounBufMngReadIdx)) {
                    GetLastAudio    = 2;
                }
                if ((GetLastVideo == 1) && (((LastVideo + 1) % VIDEO_BUF_NUM) == VideoBufMngReadIdx)) {
                    GetLastVideo    = 2;
                }
                    
                if((sysCaptureVideoStop == 0) && (GetLastAudio == 2) && (GetLastVideo == 2)) 
                {
                    if(aviCloseFile(pFile) == 0) {
                        return 0;
                    }                    
                    /*** Reset Audio/Video time biase ***/
                    ResetPayloadPresentTime = 0; //reset video time base
                    AV_TimeBase  = PREROLL;

                    //DEBUG_AVI("MPEG4 UseSem :%04d, IIS UseSem :%04d\n", VideoCmpSemEvt->OSEventCnt,iisCmpSemEvt->OSEventCnt);  
                    //DEBUG_AVI("=====================================\n");  
                    if((pFile = aviCreateFile(0)) == 0) {                        
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
            } 
            
            #if (MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SIZE)
            if((WantChangeFile == 0) && (asfDataPacketCount  > MaxPacketCount))
            #elif (MANUAL_MODE_CLOSE_FILE_METHOD == CLOSE_FILE_BY_SLICE) 
            if((WantChangeFile == 0) && (RTCseconds >= asfSectionTime))
            #endif    
            {
                if(asfRecFileNum != 0)
                {
                    WantChangeFile          = 1;
                    DEBUG_AVI("Time's up!!!\n");
                    DEBUG_AVI("RTCseconds == %d\n", RTCseconds);
                    DEBUG_AVI("asfRecFileNum  = %d\n", asfRecFileNum);
                }
                else
                {
                    sysCaptureVideoStart    = 0;
                    sysCaptureVideoStop     = 1;
                    DEBUG_AVI("asfRecFileNum  = %d\n", asfRecFileNum);
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


#ifdef  AVI_AUDIO
    while(iisTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(iisTrgSemEvt);
    }
#endif

    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    while(VideoTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(VideoTrgSemEvt);
    }
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
  	while(VideoTrgSemEvt->OSEventCnt > 0) {
        OSSemAccept(VideoTrgSemEvt);
    }

    #endif               
    
    /* delay until mpeg4 and IIS task reach pend state */
#if(CHIP_OPTION == CHIP_PA9001D)
    OSTimeDly(2);
#else
    OSTimeDly(6);
#endif

    

    if((asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && ((EventTrigger == CAPTURE_STATE_WAIT) || (EventTrigger == CAPTURE_STATE_TEMP)))     
	{
        DEBUG_AVI("Event mode finish!!\n");
        DEBUG_AVI("video_value_max = %d\n", video_value_max);
#ifdef  ASF_AUDIO
        DEBUG_AVI("audio_value_max = %d\n", audio_value_max);
#endif
        return 1;
    }

#ifdef  AVI_AUDIO
    // write redundance audio payload data
    while(iisCmpSemEvt->OSEventCnt > 0) {
        Output_Sem();   
        audio_value = OSSemAccept(iisCmpSemEvt);
        Output_Sem();
        if (audio_value > 0) {   
            if(audio_value_max < audio_value)
                audio_value_max = audio_value;
            if(aviWriteAudioChunk(pFile, &iisSounBufMng[iisSounBufMngReadIdx], &AudOutLen) == 0) {
        		DEBUG_AVI("AVI write audio chunk error!!!\n");
                dcfCloseFileByIdx(pFile, 0, &tmp);                
        	    return 0;
        	}   
        	aviMakeAudioIndex(AudOutLen);
            iisSounBufMngReadIdx = (iisSounBufMngReadIdx + 1) % IIS_BUF_NUM;
        }
    }
#endif  
    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    while(VideoCmpSemEvt->OSEventCnt > 0) {
        Output_Sem();
        video_value = OSSemAccept(VideoCmpSemEvt);
        Output_Sem();
        if (video_value > 0) {   
            if(video_value_max < video_value)
                video_value_max = video_value;

            if(aviWriteVideoChunk(pFile, &VideoBufMng[VideoBufMngReadIdx], &VidoutLen) == 0) {
                DEBUG_AVI("AVI write video chunk error!!!\n");
    			dcfCloseFileByIdx(pFile, 0, &tmp);
    			return 0;
			}				
			aviMakeVideoIndex(VidoutLen, VideoBufMng[VideoBufMngReadIdx].flag & FLAG_I_VOP);
            VideoBufMngReadIdx = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;
        }
    }
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)				  
	while(VideoCmpSemEvt->OSEventCnt > 0) {
        Output_Sem();
        video_value = OSSemAccept(VideoCmpSemEvt);
        Output_Sem();
        if (video_value > 0) {   
            if(video_value_max < video_value)
                video_value_max = video_value;
            
            if(aviWriteVideoChunk(pFile, &VideoBufMng[VideoBufMngReadIdx], &VidoutLen) == 0) {
                DEBUG_AVI("AVI write video chunk error!!!\n");
	    		dcfCloseFileByIdx(pFile, 0, &tmp);
	    		return 0;
			}
				
			aviMakeVideoIndex(VidoutLen, VideoBufMng[VideoBufMngReadIdx].flag & FLAG_I_VOP);
            VideoBufMngReadIdx = (VideoBufMngReadIdx + 1) % VIDEO_BUF_NUM;            
        }
    }    
    #endif               

    aviCloseFile(pFile);

    
    DEBUG_AVI("video_value_max = %d\n", video_value_max);
#ifdef  ASF_AUDIO
    DEBUG_AVI("audio_value_max = %d\n", audio_value_max);
#endif

    DEBUG_AVI("isu_avifrmcnt     = %d\n", isu_avifrmcnt);

    return 1;
}    
/*Peter 1109 E*/

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

s32 aviSetVideoResolution(u16 width, u16 height)
{
	aviVopWidth     = width;	/* Peter Hsu: 0619 */
	aviVopHeight    = height;
	
	mpeg4SetVideoResolution(width, height);
	mpeg4ConfigQualityFrameRate(MPEG_BITRATE_LEVEL_100);
	return  1;
}

/*

Routine Description:

	Read file.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
/*Peter 1109 S*/

s32 aviReadFile(void)
{
	FS_FILE*    pFile;
	FS_i32      MoviPosition;
	u32         size, IndexNum, i, aviVideoIndex, aviAudioIndex;
	s64         aviPresentTime;
    HDRLIST     *AviHeader;
    u8          *pBuf;
	u16         video_value;
    CHUNKINFO   ChunkInfo;
#ifdef  AVI_AUDIO
	u16         audio_value;
#endif
    u8  Error       = 0;
    u8  err;
    u8  tmp;
	
	ReadOnce = 0;
    NVOPCnt                 = 0;
    sysPlaybackForward = 0;
  	Video_timebase = 0;
    ResetPlayback  = 0;    
  	StartPlayBack = 0;
    mpeg4taskrun = 0;
	IDUInterruptTime = 0;
    
    AviHeader  = (HDRLIST*)AviHeaderBuf;
    memset(AviHeaderBuf, 0, sizeof(AviHeaderBuf));

	/* open file */
	if ((pFile = dcfOpen((s8*)dcfPlaybackCurFile->pDirEnt->d_name, "r")) == NULL) {
	    DEBUG_AVI("AVI open file error!!!\n");
		return 0;
	}
		

	/* video */
	VideoPictureIndex       = 0;	/* cytsai: for armulator only */
	VideoBufMngReadIdx  = 0;
	VideoBufMngWriteIdx = 0;
	aviVopCount             = 0;
	MainVideodisplaybuf_idx     = 0;
	aviVideoIndex           = 0;
	IsuIndex                = 0;
	VideoNextPresentTime    = 0;
	aviPresentTime          = 0;
	MicroSecPerFrame        = 0;
	mpegflag                = 0;
	isuStatus_OnRunning     = 0;
	MPEG4_Mode              = 1;    // 0: record, 1: playback
	#if (VIDEO_CODEC_OPTION == MJPEG_CODEC)	
    MJPG_Mode               = 1;	
    #endif
	for(i = 0; i < VIDEO_BUF_NUM; i++) {
	    VideoBufMng[i].buffer   = VideoBuf;
	}

    #ifdef  AVI_AUDIO
	/* audio */
	aviAudioChunkCount       = 0;
	aviAudioPresentTime      = 0;
	iisSounBufMngReadIdx    = 0;
	iisSounBufMngWriteIdx   = 0;
	aviAudioIndex           = 0;
	AudioPlayback           = 0;    // 0: audio not play, 1: audio playing
	IISMode                 = 1;    // 0: record, 1: playback, 2: receive and playback audio in preview mode
    IISTime                 = 0;    // Current IIS playback time(micro second)
    IISTimeUnit             = 0;    // IIS playback time per DMA(micro second)
    iisPlayCount            = 0;    // IIS played chunk number
    iisTotalPlay            = 0;    // IIS total trigger playback number
	/* initialize sound buffer */
	for(i = 0; i < IIS_BUF_NUM; i++) {
		iisSounBufMng[i].buffer     = iisSounBuf[i];
	}
    #endif	

    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
  	/* refresh semaphore state */
	Output_Sem();
	while(VideoTrgSemEvt->OSEventCnt > 0) {
		OSSemAccept(VideoTrgSemEvt);
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
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
  	/* refresh semaphore state */
	Output_Sem();
	while(VideoTrgSemEvt->OSEventCnt > 0) {
		OSSemAccept(VideoTrgSemEvt);
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
    #endif

    #ifdef  AVI_AUDIO
	while(iisTrgSemEvt->OSEventCnt > 0) {
		OSSemAccept(iisTrgSemEvt);
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
    
    if((aviReadHeaderIndex(pFile, AviHeaderBuf, &MoviPosition, &IndexNum, &WavInfo)) == 0)
    {
        DEBUG_AVI("AVI read file header error!!!\n");
        dcfClose(pFile, &tmp);
        return 0;
    }
    if((dcfSeek(pFile, MoviPosition, FS_SEEK_SET))==0)
    {
        DEBUG_AVI("AVI file seek to movi position 0x%08x error!!!\n", MoviPosition);
        dcfClose(pFile, &tmp);
        return 0;
    }
    
    while((sysPlaybackVideoStop == 0) && !(sysPlaybackThumbnail == 1 && IsuIndex > 0))
    {
        /*** Read all video/audio chunk ***/
        if((aviVideoIndex == aviVopCount) && (aviAudioIndex == aviAudioChunkCount))
        {
            OSTimeDly(1);//Lsk 090505
            continue;
        }

        if((dcfRead(pFile, (u8*)(&ChunkInfo), sizeof(CHUNKINFO), &size)) == 0)
            return 0;

        /*** Audio Chunk ***/
        if(ChunkInfo.ckID == 'bw10')
        {
            //DEBUG_AVI("Read Audio Chunk\n");
            /*******************************************************************
            ***  Step 1 : Memory management.                                 ***  
            ***  Check writing data, not overwrite data that is not yet read *** 
            *******************************************************************/
            while(((audio_value = OSSemAccept(iisCmpSemEvt)) == 0)&& sysPlaybackVideoStop==0 ) {
                OSTimeDly(1);
            }
            pBuf    = iisSounBufMng[iisSounBufMngWriteIdx].buffer;
            /*******************************************************************
            ***  Step 2 : Read Bitstream form avi file. And update video     *** 
            ***           frame information                                  *** 
            *******************************************************************/           

            if((dcfRead(pFile, pBuf, ChunkInfo.ckSize, &size)) == 0)
            {
                DEBUG_AVI("AVI read Audio chunk error!!!\n");
                dcfClose(pFile, &tmp);
                return 0;
            }
            
            iisSounBufMng[iisSounBufMngWriteIdx].size   = size;
            aviAudioIndex++;
            if(aviAudioIndex >= aviAudioChunkCount) {
                DEBUG_AVI("Read  %d audio chunk finish!!!\n",aviAudioChunkCount);
            }
            iisSounBufMngWriteIdx = (iisSounBufMngWriteIdx + 1) % IIS_BUF_NUM;            
            /*******************************************************************
            ***  Step 3 : Post semphone to trigger Decoder,                  *** 
            ***           and resume decoder task.                           ***            
            *******************************************************************/                       
            OSSemPost(iisTrgSemEvt);
            if((AudioPlayback == 0) &&
               ((aviAudioIndex >= 4) || (aviAudioIndex == aviAudioChunkCount)) &&
               ((IsuIndex >= (DISPLAY_BUF_NUM - 2)) || (IsuIndex == aviVopCount))) {    // only trigger IIS one time
	            AudioPlayback           = 1;
	            iisResumeTask();
				StartPlayBack = 1;
       			IDUInterruptTime = 0;
                DEBUG_AVI("iisResumeTask\n\n");
	        }
        }
        /*** Video Chunk ***/
        else if(ChunkInfo.ckID == 'cd00')
        {
            //DEBUG_AVI("Read Video Chunk\n");
            /*******************************************************************
            ***  Step 1 : Memory management.                                 ***  
            ***  Check writing data, not overwrite data that is not yet read *** 
            ***  case 1 : ---W---R------                                     *** 
            ***           write buffer addresss is less than read buffer     ***
            ***           address, and than writing current payload will     ***
            ***           cause write buffer addresss is large than read     ***
            ***           buffer address.                                    ***
            ***  case 2 : --R-------W---                                     ***    
            ***           writing current payload will cause write buffer    ***
            ***           rounding, then cause write buffer addresss is      ***
            ***           large than read buffer address.                    ***
            *******************************************************************/

            pBuf    = VideoBufMng[VideoBufMngWriteIdx].buffer;            
            while((pBuf <= VideoBufMng[VideoBufMngReadIdx].buffer) && 
                  (aviVideoIndex > (VideoPictureIndex+NVOPCnt))&&
                  ((pBuf + ChunkInfo.ckSize + MPEG4_MIN_BUF_SIZE) > VideoBufMng[VideoBufMngReadIdx].buffer)
                  && sysPlaybackVideoStop == 0)
            {                 
                 //DEBUG_AVI("case 1\n");
                 OSTimeDly(1);
            }
            
            if((pBuf + ChunkInfo.ckSize) > mpeg4VideBufEnd)
            {
                while(((VideoBuf + ChunkInfo.ckSize + MPEG4_MIN_BUF_SIZE) > VideoBufMng[VideoBufMngReadIdx].buffer)
        			  && sysPlaybackVideoStop == 0)//Lsk 090417 : avoid deadlock when press stop playback
        	    {
                    //DEBUG_AVI("case 2\n");      	        
                    OSTimeDly(1);
                }
            }
            
            if((pBuf + ChunkInfo.ckSize) > mpeg4VideBufEnd) {
                VideoBufMng[VideoBufMngWriteIdx].buffer = VideoBuf;
                pBuf = VideoBuf;
            } else {
                VideoBufMng[VideoBufMngWriteIdx].buffer = pBuf;
            }
            /*******************************************************************
            ***  Step 2 : Read Bitstream form avi file. And update video     *** 
            ***           frame information                                  *** 
            *******************************************************************/           
            if((dcfRead(pFile, pBuf, ChunkInfo.ckSize, &size)) == 0)
            {
                DEBUG_AVI("AVI read video chunk error!!!\n");
                dcfClose(pFile, &tmp);
                return 0;
            }               
            VideoBufMng[VideoBufMngWriteIdx].size   = size;
            VideoBufMng[VideoBufMngWriteIdx].time   = aviPresentTime;  //fixed frame duration
            aviPresentTime         += AviHeader->MainAviHdr.MicroSecPerFrame;
            aviVideoIndex++;
            if(aviVideoIndex >= aviVopCount) {
                DEBUG_AVI("Read video chunk finish!!!\n");
            }
            
            pBuf    = (u8*)(((u32)pBuf + size + 3) & ~3);
            #if ((CHIP_OPTION == CHIP_PA9001A) || (CHIP_OPTION == CHIP_PA9001C) || (CHIP_OPTION == CHIP_PA9002A)) // firmware have to fix 1K boundary bug, if this chip is older than PA9001D.
            if(((int)pBuf & 0x3ff) > 0x3b0) {    // fix first burst 8 over 1 K bytes boundary bug
                pBuf    = (u8*)(((int)pBuf + 0x400) & ~0x3ff);
            }
            #endif
            VideoBufMngWriteIdx = (VideoBufMngWriteIdx + 1) % VIDEO_BUF_NUM;
            VideoBufMng[VideoBufMngWriteIdx].buffer = pBuf;
            /*******************************************************************
            ***  Step 3 : Post semphone to trigger Decoder,                  *** 
            ***           and resume decoder task.                           ***            
            *******************************************************************/           
            #if(VIDEO_CODEC_OPTION == MPEG4_CODEC)				              
            OSSemPost(VideoTrgSemEvt);
            if(aviVideoIndex <= 1) {
                if(MPEG4_Task_Go)
                {
                    OSSemAccept(VideoTrgSemEvt);
                }
       			IDUInterruptTime = 0;                
                mpeg4ResumeTask();
                mpeg4taskrun = 1;                
                DEBUG_AVI("mpeg4ResumeTask StartPlayBack\n\n");
            }
            #elif(VIDEO_CODEC_OPTION == MJPEG_CODEC)				              
            OSSemPost(VideoTrgSemEvt);
            if(aviVideoIndex <= 1) {
                if(MJPG_Task_Go)
                {
                    OSSemAccept(VideoTrgSemEvt);
                }
                DEBUG_AVI("mjpgResumeTask StartPlayBack\n\n");
       			IDUInterruptTime = 0;
                mjpgResumeTask();
                mpeg4taskrun = 1;                

            }
            #endif

            /*** Show first frame immediately ***/
            if((MainVideodisplaybuf_idx == 0) && (IsuIndex > 1)) {
                if (sysPlaybackThumbnail == 0) {
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
        }
        else
        {
            DEBUG_AVI("Read Error Chunk : 0x%04x\n",ChunkInfo.ckID);
            DEBUG_AVI("Error Position = %d\n", dcfTell(pFile));
            break;
        }
    }
    DEBUG_AVI("Read AVI file finish\n\n");

    if(sysPlaybackVideoStop) {
		if(!sysTVOutOnFlag)
			IduIntCtrl &= (~IDU_FTCINT_ENA);
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

        DEBUG_AVI("Video playback stop finish!!!\n");
    } else if (sysPlaybackThumbnail) {
        DEBUG_AVI("Video playback thumbnail finish!!!\n");        
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
            IduVidBuf0Addr = (u32)MainVideodisplaybuf[0];
        #if NEW_IDU_BRI
            BRI_IADDR_Y = IduVidBuf0Addr;
            BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
        #endif
        }
    }
    else if(!MPEG4_Error && !Error)
    {
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

ExitAviReadFile:

	OSFlagPost(gSysReadyFlagGrp, FLAGSYS_RDYSTAT_PLAY_FINISH, OS_FLAG_SET, &err);

#ifdef  ASF_AUDIO
    // for fix PA9002D bug
    //iisDisInt();
#endif

            
    /* close file */
    if(dcfClose(pFile, &tmp) == 0) {
        DEBUG_ASF("dcfClose() error!!!\n", err);
    }

#ifdef  ASF_AUDIO

    OSSemSet(iisTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: iisTrgSemEvt is %d.\n", err);
    }
#endif
        
    #if (VIDEO_CODEC_OPTION == MPEG4_CODEC)
    OSSemSet(VideoTrgSemEvt, 0, &err);
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
    }
    sysPlaybackVideoStop = 1;
    /* delay until mpeg4 and IIS task reach pend state */   
    for(i = 0; !Video_Pend; i++) {
        OSTimeDly(1);
    }

    /* suspend mpeg4 task */
    mpeg4SuspendTask();
    #elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
    
    OSSemSet(VideoTrgSemEvt, 0, &err);
    
    if (err != OS_NO_ERR) {
        DEBUG_ASF("OSSemSet Error: VideoTrgSemEvt is %d.\n", err);
    }
    sysPlaybackVideoStop = 1;
        
    /* delay until mpeg4 and IIS task reach pend state */   
    for(i = 0; !MJPEG_Pend; i++) {        
        OSTimeDly(1);
    }        
    mjpgSuspendTask();    
    #endif
#ifdef  ASF_AUDIO
    iisStopPlay();
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
    iisReset(IIS_SYSPLL_SEL_48M);

    if(Error || MPEG4_Error) { // Video playback fail
        return 0;
    } else {    // Video playback success
        return 1;
    }
}
/*Peter 1109 E*/

#ifdef  AVI_AUDIO

/*

Routine Description:

	Audio initialization.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 aviAudioInit(void)
{
    WavInfo.ID              = 0;
    WavInfo.Size            = 0;
/*Peter 1109 S*/
    WavInfo.FormatTag       = iisRecFormat.wFormatTag;    // WAVE_FORMAT_PCM 
    WavInfo.Channels        = iisRecFormat.nChannels;
    WavInfo.SamplesPerSec   = iisRecFormat.nSamplesPerSec;
    WavInfo.BitsPerSamples  = iisRecFormat.wBitsPerSample;
    WavInfo.AvgBytesPerSec  = iisRecFormat.nAvgBytesPerSec;
    WavInfo.BlockAlign      = iisRecFormat.nBlockAlign;
/*Peter 1109 E*/
	return 1;
}	

#endif

/*-------------------------------------------------------------------------*/
/* Header object							   */ 	
/*-------------------------------------------------------------------------*/

/*

Routine Description:

	Write header chunk.

Arguments:

	pFile - File handle.

Return Value:

	0 - Failure.
	1 - Success.

*/

s32 aviWriteHeaderChunk(FS_FILE* pFile)
{
    u32 size;

    /* Peter: 0727 S*/
    if(dcfWrite(pFile, (u8*)AviHeaderBuf, AVIHEADERSIZE, &size) == 0)
    	return 0;;
    /* Peter: 0727 E*/

    return 1;
}


/*Peter 1109 S*/
void WriteAVIHeader(
                    FS_FILE* pFile,
                    u32 *AviHeaderBuf,
                    u32 MoviChunkLen,
                    u32 RIFFLen,
                    u32 AviHeaderLen,   //for 'RIFF' to 'movi'
                    WAVFORMATCHK *pWavInfo
                   )
{
    PUTAVIBS_HEADER AviPubs;
    s32             i;
    s32             JUNKWord;
    s32             W, H;
    HDRLIST         *AviHeader                  = (HDRLIST*)AviHeaderBuf;

    W                                           = aviVopWidth;
    H                                           = aviVopHeight;

    /*** Main AVIHeader ***/
    // RIFF-Lens-ID (3 word)
    AviHeader->AviRiff.ID                       = 'FFIR'; 
    AviHeader->AviRiff.fileSize                 = RIFFLen;
    AviHeader->AviRiff.fileType                 = ' IVA';
    
    //LIST-Lens-ID (3 word)                     
    AviHeader->hdrl.listID                      = 'TSIL';
    AviHeader->hdrl.listSize                    = AviHeaderLen - 32;    // 32: RIFF-Lens-ID + LIST-Lens + LIST-Lens-movi; //???
    AviHeader->hdrl.listType                    = 'lrdh';
    //CHUNKID-Lens                              
    AviHeader->AviHdrChunk.ckID                 = 'hiva';
    AviHeader->AviHdrChunk.ckSize               = 56;       
    //Initialize Main AVI Header                
    /*Peter 0711 S*/
#ifdef  AVI_AUDIO
    if(aviVopCount == 0) {
    	AviHeader->MainAviHdr.MicroSecPerFrame      = 0;   //micro sec
    } else {
    	AviHeader->MainAviHdr.MicroSecPerFrame      = (aviAudioPresentTime * 1000) / aviVopCount;   //micro sec
    }
#else                                           
    //AviHeader->MainAviHdr.MicroSecPerFrame      = 33367 * 30 / Framerate;   //micro sec
    AviHeader->MainAviHdr.MicroSecPerFrame      = 1000000 / Framerate;   //micro sec
#endif
    MicroSecPerFrame                            = AviHeader->MainAviHdr.MicroSecPerFrame;
    /*Peter 0711 E*/
    AviHeader->MainAviHdr.MaxBytePerSec         = W * H * 3 * 30;
    AviHeader->MainAviHdr.Reverved1             = 0;  //???
    AviHeader->MainAviHdr.Flags                 = 0x00010030;   /* Peter: 0711 */
    AviHeader->MainAviHdr.TotalFrames           = aviVopCount;  //Total Frame   /* Peter: 0711 */
    AviHeader->MainAviHdr.InitialFrames         = 0;
#ifdef  AVI_AUDIO
    AviHeader->MainAviHdr.Streams               = 2;
#else                                           
    AviHeader->MainAviHdr.Streams               = 1;        //how many streams in the AVI file
#endif
    AviHeader->MainAviHdr.SuggestedBufferSize   = W*H*3;    //Frame Buffer Size
    AviHeader->MainAviHdr.Width                 = W;
    AviHeader->MainAviHdr.Height                = H;
    AviHeader->MainAviHdr.Scale                 = 0; //???
    AviHeader->MainAviHdr.Rate                  = 0; //???
    AviHeader->MainAviHdr.Start                 = 0; //???
    AviHeader->MainAviHdr.Length                = 0; //???
    
    AviHeader->strl_vid.listID                  = 'TSIL';
    AviHeader->strl_vid.listSize                = sizeof(AVISTREAMHEADER) + sizeof(DIBINFO) + 20 + AVIEXTINFOLENS;
    AviHeader->strl_vid.listType                = 'lrts';
    //Initialize Stream Header
    AviHeader->strh_vid.ckID                    = 'hrts';
    AviHeader->strh_vid.ckSize                  = sizeof(AVISTREAMHEADER);

    AviHeader->StrHdr_vid.Type                  = 'sdiv';   //'vids'

    #if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
    AviHeader->StrHdr_vid.Handler   = 'GPJM';   // M4S2 Compressor
    #elif(VIDEO_CODEC_OPTION == MPEG4_CODEC)
    /*  // don't remove this marked code by Peter
    switch(mp4opt->Bitstream_Format)
    {
    case 1:
        AviHeader->StrHdr_vid.Handler   = 'xvid';   // DIVX 5.0 Compressor
        break;
    case 2:
    */
        AviHeader->StrHdr_vid.Handler   = '2S4M';   // M4S2 Compressor
    /*  // don't remove this marked code by Peter
        break;
    case 4:
        AviHeader->StrHdr_vid.Handler   = '362S';   // H.263+
        break;
    default:
        break;
    }
    */
    #endif
    AviHeader->StrHdr_vid.Flags                 = 0;
    AviHeader->StrHdr_vid.Reserved1             = 0;
    AviHeader->StrHdr_vid.InitialFrames         = 0;
    /*Peter 0711 S*/
    //AviHeader->StrHdr_vid.Scale                 = 1001 * 30 / Framerate;
    //AviHeader->StrHdr_vid.Scale                 = 100;
    AviHeader->StrHdr_vid.Scale                 = AviHeader->MainAviHdr.MicroSecPerFrame;
    //AviHeader->StrHdr_vid.Rate                  = 30000;
    //AviHeader->StrHdr_vid.Rate                  = Framerate * 100;
    AviHeader->StrHdr_vid.Rate                  = 1000000;
    /*Peter 0711 E*/
    AviHeader->StrHdr_vid.Start                 = 0;
    AviHeader->StrHdr_vid.Length                = aviVopCount;      //Total Frame   /* Peter: 0711 */
    AviHeader->StrHdr_vid.SuggestedBufferSize   = W * H * 3;        /* Peter: 0711 */
    AviHeader->StrHdr_vid.Quality               = 10000;            /* Peter: 0711 */
    AviHeader->StrHdr_vid.SampleSize            = 0;
    AviHeader->StrHdr_vid.Rec_UpLeft            = 0x00000000;   //upper-left corner
    AviHeader->StrHdr_vid.Rec_DownRight         = (H << 16) | W;    //bottom-down corner    /* Peter: 0711 */
    
    //Initialize Stream format
    AviHeader->strf_vid.ckID                    = 'frts';
    AviHeader->strf_vid.ckSize                  = sizeof(DIBINFO) + AVIEXTINFOLENS;

    AviHeader->StrFmt_vid.Size                  = sizeof(DIBINFO) + AVIEXTINFOLENS; /* Peter: 0711 */
    AviHeader->StrFmt_vid.Width                 = W;
    AviHeader->StrFmt_vid.Height                = H;
    AviHeader->StrFmt_vid.Planes                = 1;
    AviHeader->StrFmt_vid.BitCount              = 24;

    #if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
    AviHeader->StrFmt_vid.Compression  = 'GPJM';   // M4S2 Compressor
    #elif(VIDEO_CODEC_OPTION == MPEG4_CODEC)
    /*  // don't remove this marked code by Peter
    switch(mp4opt->Bitstream_Format)
    {
    case 1:
        AviHeader->StrFmt_vid.Compression        = '05XD';   // DIVX 5.0 Compressor
        break;
    case 2:
    */
        AviHeader->StrFmt_vid.Compression       = '2S4M';   // M4S2 Compressor
    /*  // don't remove this marked code by Peter
        break;
    case 4: 
        AviHeader->StrFmt_vid.Compression       = '362S';   // H.263+
        break;
    default:
        break;
    }
    */
    #endif
    AviHeader->StrFmt_vid.SizeImage             = W * H * 3;
    AviHeader->StrFmt_vid.XPelsPerMeter         = 0;
    AviHeader->StrFmt_vid.YPelsPerMeter         = 0;
    AviHeader->StrFmt_vid.ClrUsed               = 0;        // Use max Color
    AviHeader->StrFmt_vid.ClrImportant          = 0;

#ifdef  AVI_AUDIO
    AviHeader->strl_aud.listID                  = 'TSIL';
    AviHeader->strl_aud.listSize                = sizeof(AVISTREAMHEADER) + sizeof(WAVINFO) + 20;
    AviHeader->strl_aud.listType                = 'lrts';
    //Initialize Stream Header
    AviHeader->strh_aud.ckID                    = 'hrts';
    AviHeader->strh_aud.ckSize                  = sizeof(AVISTREAMHEADER);

    AviHeader->StrHdr_aud.Type                  = 'sdua';               //'auds'
    AviHeader->StrHdr_aud.Handler               = pWavInfo->FormatTag;  // Compressor?
    AviHeader->StrHdr_aud.Flags                 = 0;
    AviHeader->StrHdr_aud.Reserved1             = 0;
    AviHeader->StrHdr_aud.InitialFrames         = 0;
    switch(pWavInfo->SamplesPerSec)
    {
    case 8000:
        AviHeader->StrHdr_aud.Scale             = 5;
        AviHeader->StrHdr_aud.Rate              = 40006;
        break;
    case 16000:
        AviHeader->StrHdr_aud.Scale             = 10;
        AviHeader->StrHdr_aud.Rate              = 159801;
        break;
    case 24000:
        AviHeader->StrHdr_aud.Scale             = 10;
        AviHeader->StrHdr_aud.Rate              = 241071;
        break;
    case 32000:
        AviHeader->StrHdr_aud.Scale             = 10;
        AviHeader->StrHdr_aud.Rate              = 321429;
        break;
    case 44100:
        AviHeader->StrHdr_aud.Scale             = 5;
        AviHeader->StrHdr_aud.Rate              = 220588;
        break;
    case 48000:
        AviHeader->StrHdr_aud.Scale             = 1;
        AviHeader->StrHdr_aud.Rate              = 46875;
        break;
    default:
        DEBUG_AVI("Sample Rate is illegal\n");
        break;
    }
    AviHeader->StrHdr_aud.Start                 = 0;
    AviHeader->StrHdr_aud.Length                = aviAudioChunkCount;    /* Peter: 0711 */
    AviHeader->StrHdr_aud.SuggestedBufferSize   = 0;
    AviHeader->StrHdr_aud.Quality               = 0;
    AviHeader->StrHdr_aud.SampleSize            = pWavInfo->BlockAlign;
    AviHeader->StrHdr_aud.Rec_UpLeft            = 0;
    AviHeader->StrHdr_aud.Rec_DownRight         = 0;  
    
    //Initialize Stream format
    AviHeader->strf_aud.ckID                    = 'frts';
    AviHeader->strf_aud.ckSize                  = sizeof(WAVINFO);

    AviHeader->StrFmt_aud.FormatTag             = pWavInfo->FormatTag; 
    AviHeader->StrFmt_aud.Channel               = pWavInfo->Channels;
    AviHeader->StrFmt_aud.SamplesPerSec         = pWavInfo->SamplesPerSec;  //sample rate  //??? error
    AviHeader->StrFmt_aud.AvgBytesPerSec        = pWavInfo->AvgBytesPerSec;
    AviHeader->StrFmt_aud.BlockAlign            = pWavInfo->BlockAlign;
    AviHeader->StrFmt_aud.BitsPerSample         = pWavInfo->BitsPerSamples;
#endif
    //======================Fill into Header Buffer===================//
    memset(AviHeader->ExtInfo,0,AVIEXTINFOLENS);
    #if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
    #elif(VIDEO_CODEC_OPTION == MPEG4_CODEC)
    mpeg4EncodeVolHeader((u8*)AviHeader->ExtInfo, &aviVideHeaderSize); /* Peter: 0711 */
    #endif
    //initialize seek pointer
    AviPubs.wordcnt                             = 0;
    AviPubs.fillPos                             = (s32*)AviHeaderBuf;
    //====//    
    AviPubs.fillPos                            += sizeof(HDRLIST)/4;
    AviPubs.wordcnt                            += sizeof(HDRLIST)/4;        
    //Write JUNK Chunk
    WriteToHeader_Int('KNUJ', &AviPubs);        // 'JUNK'
    JUNKWord                                    = AviHeaderLen / 4 - AviPubs.wordcnt - 1 - 3;
    WriteToHeader_Int(JUNKWord * 4, &AviPubs);

    for(i = 0; i < JUNKWord; i++)
       WriteToHeader_Int(0, &AviPubs);          // length

    //write the LIST_2 'movi' Chunk
    WriteToHeader_Int('TSIL', &AviPubs);        // 'LIST'
    WriteToHeader_Int(MoviChunkLen, &AviPubs);  // length
    WriteToHeader_Int('ivom', &AviPubs);        // 'movi'
    
    //dcfWrite(pFile, (u8*)AviHeaderBuf, AVIHEADERSIZE, &size);
}
/*Peter 1109 E*/

void WriteToHeader_Int(s32 val, PUTAVIBS_HEADER *pPubs)
{
    *(pPubs->fillPos)=val;
    pPubs->fillPos ++;
    pPubs->wordcnt ++;
}
//

/*

Routine Description:

	Write file properties object post.

Arguments:

	pFile - File handle.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 aviWriteFileHeaderPost(FS_FILE* pFile)
{
    u32 size;

    WriteAVIHeader(
                    pFile,
                    AviHeaderBuf,
                    MoviChunkLen,
                    RIFFLen,
                    AVIHEADERSIZE,   //for 'RIFF' to 'movi'
                    &WavInfo
                  );

	
    dcfSeek(pFile, 0, SEEK_SET); 
    
/* Peter: 0727 S*/
    if(dcfWrite(pFile, (u8*)AviHeaderBuf, AVIHEADERSIZE, &size) == 0)
        return 0;
/* Peter: 0727 E*/
	
    return 1;
}

/*Peter 1109 S*/
/*

Routine Description:

	Read header chunk, idx1 chunk and find movi LIST position.

Arguments:

	pFile           - File handle.
	AviHeaderBuf    - A buffer for get header data.
	MoviPosition    - movi LIST file point, vop start address = *MoviPosition + index.ChunkOffset + 4.
	IndexNum        - Total number of index in idx1 chunk.
	pWavInfo        - Wave format of this AVI file.

Return Value:

	0 - Failure.
	1 - Success.

*/

s32 aviReadHeaderIndex(FS_FILE*         pFile,      
                       u32*             AviHeaderBuf,
                       FS_i32*          MoviPosition,  // movi LIST file point
                       u32*             IndexNum,
                       WAVFORMATCHK*    pWavInfo
                       )
{
    u32         size, err, headsize, i, j, k;
    LISTINFO    moviLIST;
    CHUNKINFO   idx1Chunk;
    HDRLIST     *AviHeader  = (HDRLIST*)AviHeaderBuf;

    /*
    err = dcfRead(pFile, (u8*)&AviHeader->AviRiff,      sizeof(RIFFINFO),                               &size);
    err = dcfRead(pFile, (u8*)&AviHeader->hdrl,         sizeof(LISTINFO),                               &size);
    err = dcfRead(pFile, (u8*)&AviHeader->AviHdrChunk,  sizeof(CHUNKINFO),                              &size);
    err = dcfRead(pFile, (u8*)&AviHeader->MainAviHdr,   sizeof(MAINAVIHEADER),                          &size);
    err = dcfRead(pFile, (u8*)&AviHeader->strl_vid,     sizeof(LISTINFO),                               &size);
    err = dcfRead(pFile, (u8*)&AviHeader->strh_vid,     sizeof(CHUNKINFO),                              &size);
    err = dcfRead(pFile, (u8*)&AviHeader->StrHdr_vid,   sizeof(AVISTREAMHEADER),                        &size);
    err = dcfRead(pFile, (u8*)&AviHeader->strf_vid,     sizeof(CHUNKINFO),                              &size);
    err = dcfRead(pFile, (u8*)&AviHeader->StrFmt_vid,   sizeof(DIBINFO),                                &size);
    */
    headsize    = sizeof(RIFFINFO)
                + sizeof(LISTINFO) 
                + sizeof(CHUNKINFO) 
                + sizeof(MAINAVIHEADER)
                + sizeof(LISTINFO)   
                + sizeof(CHUNKINFO)
                + sizeof(AVISTREAMHEADER)
                + sizeof(CHUNKINFO) 
                + sizeof(DIBINFO);
    if( (dcfRead(pFile, (u8*)AviHeader, headsize, &size)) == 0)
        return 0;

    if((dcfRead(pFile, (u8*)AviHeader->ExtInfo, AviHeader->strf_vid.ckSize - sizeof(DIBINFO), &size)) == 0)
        return 0;

    #if(VIDEO_CODEC_OPTION == MJPEG_CODEC)
    
    #elif(VIDEO_CODEC_OPTION == MPEG4_CODEC)    
    // parse VOL header from AviHeader->ExtInfo
    mpeg4DecodeVolHeader(&Mp4Dec_opt,(u8*)AviHeader->ExtInfo, size);    /* Peter: 0711 */
    #endif
    
    Mp4Dec_opt.Width  = AviHeader->MainAviHdr.Width;  
    Mp4Dec_opt.Height = AviHeader->MainAviHdr.Height;  
    aviVopCount         = AviHeader->MainAviHdr.TotalFrames;
    MicroSecPerFrame    = AviHeader->MainAviHdr.MicroSecPerFrame;
    VideoDuration       = 3 + ((aviVopCount * MicroSecPerFrame)/1000000)  ; //add asf preroll time
    DEBUG_AVI("V. MainAviHdr.TotalFrames = %d\n", aviVopCount);
    DEBUG_AVI("V. MicroSecPerFrame = %d\n", MicroSecPerFrame);
    DEBUG_AVI("V. VideoDuration = %d\n", VideoDuration);

#ifdef  AVI_AUDIO
    /*
    err = dcfRead(pFile, (u8*)&AviHeader->strl_aud,     sizeof(LISTINFO),                               &size);
    err = dcfRead(pFile, (u8*)&AviHeader->strh_aud,     sizeof(CHUNKINFO),                              &size);
    err = dcfRead(pFile, (u8*)&AviHeader->StrHdr_aud,   sizeof(AVISTREAMHEADER),                        &size);
    err = dcfRead(pFile, (u8*)&AviHeader->strf_aud,     sizeof(CHUNKINFO),                              &size);
    err = dcfRead(pFile, (u8*)&AviHeader->StrFmt_aud,   sizeof(WAVINFO),                                &size);
    */
    headsize    = sizeof(LISTINFO)
                + sizeof(CHUNKINFO) 
                + sizeof(AVISTREAMHEADER) 
                + sizeof(CHUNKINFO)
                + sizeof(WAVINFO);
    if((dcfRead(pFile, (u8*)&AviHeader->strl_aud, headsize, &size)) == 0)
        return 0;
    
    pWavInfo->FormatTag         = AviHeader->StrHdr_aud.Handler;        // Compressor?
    pWavInfo->BlockAlign        = AviHeader->StrHdr_aud.SampleSize;     
    pWavInfo->FormatTag         = AviHeader->StrFmt_aud.FormatTag;      
    pWavInfo->Channels          = AviHeader->StrFmt_aud.Channel;        
    pWavInfo->SamplesPerSec     = AviHeader->StrFmt_aud.SamplesPerSec;  // sample rate
    pWavInfo->AvgBytesPerSec    = AviHeader->StrFmt_aud.AvgBytesPerSec; 
    pWavInfo->BlockAlign        = AviHeader->StrFmt_aud.BlockAlign;     
    pWavInfo->BitsPerSamples    = AviHeader->StrFmt_aud.BitsPerSample;  
    iisPlayFormat.wFormatTag        = pWavInfo->FormatTag;
    iisPlayFormat.nChannels         = pWavInfo->Channels;
    iisPlayFormat.nSamplesPerSec    = pWavInfo->SamplesPerSec;
    iisPlayFormat.nAvgBytesPerSec   = pWavInfo->AvgBytesPerSec;
    iisPlayFormat.nBlockAlign       = pWavInfo->BlockAlign;
    iisPlayFormat.wBitsPerSample    = pWavInfo->BitsPerSamples;
    DEBUG_AVI("A. FormatTag = %d\n", pWavInfo->FormatTag);
    DEBUG_AVI("A. BlockAlign = %d\n", pWavInfo->BlockAlign);
    DEBUG_AVI("A. FormatTag = %d\n", pWavInfo->FormatTag);
    DEBUG_AVI("A. Channels = %d\n", pWavInfo->Channels);
    DEBUG_AVI("A. SamplesPerSec = %d\n", pWavInfo->SamplesPerSec);
    DEBUG_AVI("A. AvgBytesPerSec = %d\n", pWavInfo->AvgBytesPerSec);
    DEBUG_AVI("A. BlockAlign = %d\n", pWavInfo->BlockAlign);
    DEBUG_AVI("A. BitsPerSamples = %d\n", pWavInfo->BitsPerSamples);

    iisSetPlayFormat(&iisPlayFormat);
    if(iisPlayFormat.nAvgBytesPerSec == 0) {
        DEBUG_AVI("AviHeader->StrFmt_aud.AvgBytesPerSec == 0\n");
    	return 0;
    }

    IISTimeUnit                     = IIS_PLAYBACK_SIZE * 1000000 / iisPlayFormat.nAvgBytesPerSec;
    aviAudioChunkCount               = AviHeader->StrHdr_aud.Length;
    iisTotalPlay                    = AviHeader->StrHdr_aud.Length;
#endif

    err = dcfSeek(pFile, AviHeader->hdrl.listSize + 20, SEEK_SET);   // goto next LIST
    if(err==0) return 0;
    
    // find movi LIST
    do {
        if((dcfRead(pFile, (u8*)&moviLIST, sizeof(LISTINFO), &size)) == 0)
            return 0;

        if (moviLIST.listType != 'ivom') {
            if((dcfSeek(pFile, moviLIST.listSize - 4, FS_SEEK_CUR))==0)
                return 0;
        }
    } while (moviLIST.listType != 'ivom');
    
    // get movi LIST file point, vop start address = *MoviPosition + index.ChunkOffset + 4
    *MoviPosition   = dcfTell(pFile);
    DEBUG_AVI("MoviPosition= %d\n",*MoviPosition);


    /*** Seek to index chunk position***/
    if((dcfSeek(pFile, moviLIST.listSize - 4, FS_SEEK_CUR)) == 0)
        return 0;

    do {

        if((dcfRead(pFile, (u8*)&idx1Chunk, sizeof(CHUNKINFO), &size)) == 0)
            return 0;

        if(idx1Chunk.ckID != '1xdi')
        {
            if((dcfSeek(pFile, idx1Chunk.ckSize, FS_SEEK_CUR))==0)
                return 0;
        }
    } while (idx1Chunk.ckID != '1xdi');

    /*** Read idx1 chunk data ***/
    if((dcfRead(pFile, (u8*)AVIFrameIdxBuf, idx1Chunk.ckSize, &size)) == 0)
        return 0;

    *IndexNum	= idx1Chunk.ckSize / 16;
    DEBUG_AVI("I. IndexNum = %d\n",*IndexNum);
    for(i = 0, j = 0, k = 0; i < *IndexNum; i++) {
        /* video index */
        if(AVIFrameIdxBuf[i].kid == 'cd00') {
            AVIVideoIdx[j]  = &AVIFrameIdxBuf[i];
            j++;
        /* Audio index */
        } else if(AVIFrameIdxBuf[i].kid == 'bw10') {
            AVIAudioIdx[k]  = &AVIFrameIdxBuf[i];
            k++;
        } else {
            DEBUG_AVI("Undefine index flag!!!\n");
        }
    }
    DEBUG_AVI("Read AVI Header Finish\n");
    return 1;
}
/*Peter 1109 E*/

/*-------------------------------------------------------------------------*/
/* Data object								   */
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
s32 aviWriteDataChunkPre(FS_FILE* pFile)
{
    MoviChunkLen          = 4;
    AviIdxChkPrevOffset   = 4;        // + 'movi'
    Idx1_mark             = '1xdi';   //'idx1'
    Idx1_Len              = 0;
    
	return 1;
}
	
/*Peter 1113 S*/
/*

Routine Description:

	Write audio chunk.

Arguments:

	pFile - File handle.
	pMng - Buffer manager.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 aviWriteAudioChunk(FS_FILE* pFile, IIS_BUF_MNG* pMng, u32 *size)
{
    CHUNKINFO   AudioChunk;
	u32         chunkTime;
	u32         chunkSize;
	u8*         pChunkBuf;
    u32         size1;
	
	chunkTime   = pMng->time;
	chunkSize   = pMng->size;
	pChunkBuf   = pMng->buffer;
		
	/* write audio chunk header */
    AudioChunk.ckID     = 'bw10';
    //AudioChunk.ckSize   = (pMng->size + 3) & ~0x03;
    AudioChunk.ckSize   = chunkSize;
/* Peter: 0727 S*/
    if(iisSounBufMngReadIdx == 0) {
        if(dcfWrite(pFile, (u8*)&AudioChunk, sizeof(CHUNKINFO), &size1) == 0)
    	    return 0;
        *size       = size1;
	    if(dcfWrite(pFile, (u8*)pChunkBuf, chunkSize, &size1) == 0)
	        return 0;
        *size      += size1;
    } else {
        pChunkBuf  -= sizeof(CHUNKINFO);
        chunkSize  += sizeof(CHUNKINFO);
        memcpy((u8*)pChunkBuf, (u8*)&AudioChunk, sizeof(CHUNKINFO));
	    if(dcfWrite(pFile, (u8*)pChunkBuf, chunkSize, size) == 0)
	        return 0;
    }
	
	/* advance the index */	
	aviAudioChunkCount++;
	aviAudioPresentTime += chunkTime;
    aviAudioChunkSize += *size;
	return 1;
}
/*Peter 1113 E*/

/*Peter 1113 S*/
/*

Routine Description:

	Write video chunk.

Arguments:

	pFile - File handle.
	pMng - Buffer manager.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 aviWriteVideoChunk(FS_FILE* pFile, VIDEO_BUF_MNG* pMng, u32 *size)
{
    CHUNKINFO   VideoChunk;
	u32         chunkSize, chunkTime;
	u8*         pChunkBuf;
    u32         size1, size2;

	chunkSize   = pMng->size;
	pChunkBuf   = pMng->buffer;
    chunkTime   = pMng->time;
    size1               = 0;

    VideoChunk.ckID     = 'cd00';
    VideoChunk.ckSize   = (chunkSize + 3) & ~0x03;
    
    //DEBUG_AVI("pMng[%d]->size = %d, ckSize = %d, aviVopCount = %d\n", VideoBufMngReadIdx, pMng->size, VideoChunk.ckSize, aviVopCount);

    if(pChunkBuf == VideoBuf) {
        if(dcfWrite(pFile, (u8*)&VideoChunk, sizeof(CHUNKINFO), &size1) == 0)
            return 0;
        if(dcfWrite(pFile, (u8*)pChunkBuf, VideoChunk.ckSize, &size2) == 0)
            return 0;
        *size               = size1 + size2;
    } else {
        pChunkBuf  -= sizeof(CHUNKINFO);
        chunkSize   = VideoChunk.ckSize + sizeof(CHUNKINFO);
        memcpy((u8*)pChunkBuf, (u8*)&VideoChunk, sizeof(CHUNKINFO));
        if(dcfWrite(pFile, (u8*)pChunkBuf, chunkSize, size) == 0)
            return 0;
    }
    if((*size < 0x20) || (*size > MPEG4_MIN_BUF_SIZE)) {
        DEBUG_AVI("size1 = %d, size2 = %d, size = %d, pMng->size = %d, ckSize = %d, aviVopCount = %d\n", size1, size2, *size, pMng->size, VideoChunk.ckSize, aviVopCount);
        DEBUG_AVI("VideoBufMngReadIdx = %d\n", VideoBufMngReadIdx);
        DEBUG_AVI("VideoBufMngWriteIdx = %d\n", VideoBufMngWriteIdx);
        DEBUG_AVI("VideoCmpSemEvt = %d\n", VideoCmpSemEvt->OSEventCnt);
        DEBUG_AVI("VideoTrgSemEvt = %d\n", VideoTrgSemEvt->OSEventCnt);
        DEBUG_AVI("VideoCpleSemEvt = %d\n", VideoCpleSemEvt->OSEventCnt);
    }
    aviVopCount++;
    aviVideoPresentTime += chunkTime;     
    aviVideoChunkSize += *size;    
    return 1;
}
/*Peter 1113 E*/
/*

Routine Description:

	Write video chunk.

Arguments:

	pFile - File handle.
	pMng - Buffer manager.

Return Value:

	0 - Failure.
	1 - Success.

*/
#if (VIDEO_CODEC_OPTION == MPEG4_CODEC)  
s32 aviWriteDummyVideoChunk(FS_FILE* pFile, VIDEO_BUF_MNG* pMng, u32 *size)
{
    CHUNKINFO   VideoChunk;
	u32         chunkSize, chunkTime;
	u8*         pChunkBuf;
    u32         size1, size2;
    u8 mpeg4VOPHeader[256] = 
    { 
        0x00
    };

    pChunkBuf   = pMng->buffer;
	chunkSize   = 0;
    mpeg4PutDummyVOPHeader(mpeg4Width,mpeg4Height,(u8*)mpeg4VOPHeader, &chunkSize);


    VideoChunk.ckID     = 'cd00';
    VideoChunk.ckSize   = (chunkSize + 3) & ~0x03;

    if(dcfWrite(pFile, (u8*)&VideoChunk, sizeof(CHUNKINFO), &size1) == 0)
        return 0;
    if(dcfWrite(pFile, (u8*)mpeg4VOPHeader, VideoChunk.ckSize, &size2) == 0)
            return 0;
    *size               = size1 + size2;
         
    aviVopCount++;
    aviVideoChunkSize += *size;    
    return 1;
}

#elif (VIDEO_CODEC_OPTION == MJPEG_CODEC)
s32 aviWriteDummyVideoChunk(FS_FILE* pFile, VIDEO_BUF_MNG* pMng, u32 *size)
{
    CHUNKINFO   VideoChunk;
	u32         chunkSize, chunkTime;
	u8*         pChunkBuf;
    u32         size1, size2;

	chunkSize   = pMng->size;
	pChunkBuf   = pMng->buffer;
    chunkTime   = pMng->time;
    size1               = 0;

    VideoChunk.ckID     = 'cd00';
    VideoChunk.ckSize   = (chunkSize + 3) & ~0x03;
    
    //DEBUG_AVI("pMng[%d]->size = %d, ckSize = %d, aviVopCount = %d\n", VideoBufMngReadIdx, pMng->size, VideoChunk.ckSize, aviVopCount);

    if(pChunkBuf == VideoBuf) {
        if(dcfWrite(pFile, (u8*)&VideoChunk, sizeof(CHUNKINFO), &size1) == 0)
            return 0;
        if(dcfWrite(pFile, (u8*)pChunkBuf, VideoChunk.ckSize, &size2) == 0)
            return 0;
        *size               = size1 + size2;
    } else {
        pChunkBuf  -= sizeof(CHUNKINFO);
        chunkSize   = VideoChunk.ckSize + sizeof(CHUNKINFO);
        memcpy((u8*)pChunkBuf, (u8*)&VideoChunk, sizeof(CHUNKINFO));
        if(dcfWrite(pFile, (u8*)pChunkBuf, chunkSize, size) == 0)
            return 0;
    }
    if((*size < 0x20) || (*size > MPEG4_MIN_BUF_SIZE)) {
        DEBUG_AVI("size1 = %d, size2 = %d, size = %d, pMng->size = %d, ckSize = %d, aviVopCount = %d\n", size1, size2, *size, pMng->size, VideoChunk.ckSize, aviVopCount);
        DEBUG_AVI("VideoBufMngReadIdx = %d\n", VideoBufMngReadIdx);
        DEBUG_AVI("VideoBufMngWriteIdx = %d\n", VideoBufMngWriteIdx);
        DEBUG_AVI("VideoCmpSemEvt = %d\n", VideoCmpSemEvt->OSEventCnt);
        DEBUG_AVI("VideoTrgSemEvt = %d\n", VideoTrgSemEvt->OSEventCnt);
        DEBUG_AVI("VideoCpleSemEvt = %d\n", VideoCpleSemEvt->OSEventCnt);
    }
    aviVopCount++;
    aviVideoChunkSize += *size;    
    return 1;
}
#endif
/*Peter 1113 E*/

s32 aviWriteVirtualAudioChunk(IIS_BUF_MNG* pMng)
{
    u32 chunkTime, chunkSize;
#if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif

    chunkTime = pMng->time;
    chunkSize = pMng->size;

    /* advance the index */ 
    aviAudioPresentTime += chunkTime;    


    OS_ENTER_CRITICAL();
    CurrentAudioSize   -= chunkSize;
    OS_EXIT_CRITICAL();


    return 1;
}
s32 aviWriteVirtualVideoChunk(VIDEO_BUF_MNG* pMng)
{
    u32 chunkTime, chunkSize;
 #if (OS_CRITICAL_METHOD == 3)                      /* Allocate storage for CPU status register           */
    unsigned int  cpu_sr = 0;                    /* Prevent compiler warning                           */
#endif
   
    chunkTime = pMng->time;
    chunkSize = pMng->size;
    
    /* advance the index */
    aviVideoPresentTime += chunkTime;    

    OS_ENTER_CRITICAL();
    CurrentVideoSize   -= chunkSize;
    OS_EXIT_CRITICAL();

    return 1;
}
void aviMakeIndex(
                    s32 VidoutLen,
                    s32 AudOutLen
                  )
{
    //====Write AVi Index Chunk====//
    /*
    switch(p_mp4opt->Bitstream_Format)
    {
    case 0:
    case 3:
        break;

    case 1:
    case 2:
    case 4:
    */
        //===record Idx1===//
#ifdef  AVI_AUDIO
        //=Audio=//
        if(AudOutLen)
        {
            AVIFrameIdx->kid          = 'bw10';
            AVIFrameIdx->ChunkOffset  = AviIdxChkPrevOffset;    
            AVIFrameIdx->ChunkLength  = AudOutLen - 8;    //exclude the eight bytes for the RIFF heade(##dc and length)
            AVIFrameIdx->Flags        = 16;               //1:I frame , 0: P frame
            AviIdxChkPrevOffset      += AudOutLen;
            AVIFrameIdx++;

            Idx1_Len                 += 16;
            MoviChunkLen             += AudOutLen;
        }
#endif
        //=Video==//
        AVIFrameIdx->kid              = 'cd00';                           //'00dc'
        AVIFrameIdx->Flags            = Vop_Type ^ 0x01;     //1:I frame , 0: P frame
        AVIFrameIdx->ChunkOffset      = AviIdxChkPrevOffset;    
        AVIFrameIdx->ChunkLength      = VidoutLen - 8;                    //exclude the eight bytes for the RIFF heade(##dc and length)
        AviIdxChkPrevOffset          += VidoutLen;
        AVIFrameIdx++;
        Idx1_Len                     += 16;
        MoviChunkLen                 += VidoutLen;
        DEBUG_AVI("MoviChunkLen = %d, VidoutLen = %d\n", MoviChunkLen, VidoutLen);
    /*
        break;
    default:
        break;  
    }   
    */
    
}

void aviMakeAudioIndex(
                        s32 AudOutLen
                      )
{
    //====Write AVi Index Chunk====//

        //===record Idx1===//
#ifdef  AVI_AUDIO
        //=Audio=//
        if(AudOutLen)
        {
            AVIFrameIdx->kid          = 'bw10';
            AVIFrameIdx->ChunkOffset  = AviIdxChkPrevOffset;    
            AVIFrameIdx->ChunkLength  = AudOutLen - 8;    //exclude the eight bytes for the RIFF heade(##dc and length)
            AVIFrameIdx->Flags        = 16;               //1:I frame , 0: P frame
            AviIdxChkPrevOffset      += AudOutLen;
            AVIFrameIdx++;

            Idx1_Len                 += 16;
            MoviChunkLen             += AudOutLen;
        }
#endif
}

void aviMakeVideoIndex(
                         s32 VidoutLen,
                         s32 IsIntraFrame
                      )
{
    //====Write AVi Index Chunk====//
    /*
    switch(p_mp4opt->Bitstream_Format)
    {
    case 0:
    case 3:
        break;

    case 1:
    case 2:
    case 4:
    */
        //===record Idx1===//
        //=Video==//
        AVIFrameIdx->kid              = 'cd00';                           //'00dc'
        //AVIFrameIdx->Flags            = Vop_Type ^ 0x01;     //1:I frame , 0: P frame
        AVIFrameIdx->Flags            = IsIntraFrame;     //1:I frame , 0: P frame
        AVIFrameIdx->ChunkOffset      = AviIdxChkPrevOffset;    
        AVIFrameIdx->ChunkLength      = VidoutLen - 8;                    //exclude the eight bytes for the RIFF heade(##dc and length)
        AviIdxChkPrevOffset          += VidoutLen;
        AVIFrameIdx++;
        Idx1_Len                     += 16;
        MoviChunkLen                 += VidoutLen;
        //DEBUG_AVI("MoviChunkLen = %d, VidoutLen = %d\n", MoviChunkLen, VidoutLen);
        //DEBUG_AVI("IsIntraFrame = %d\n", IsIntraFrame);
    /*
        break;
    default:
        break;  
    }   
    */
    
}

/*-------------------------------------------------------------------------*/
/* Index object								   */
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
s32 aviWriteIndexChunk(FS_FILE* pFile)
{
	u32 size;
    u32 Zero    = 0;

    if(MoviChunkLen & 0x01)//for short alignment
    {
        dcfWrite(pFile, (u8*)&Zero, 1, &size);
        MoviChunkLen     += 1;
    }

/* Peter: 0727 S*/
    if(dcfWrite(pFile, (u8*)&Idx1_mark,     sizeof(s32), &size) == 0)
        return 0;
    if(dcfWrite(pFile, (u8*)&Idx1_Len,      sizeof(s32), &size) == 0)
        return 0;
    if(dcfWrite(pFile, (u8*)AVIFrameIdxBuf, Idx1_Len,    &size) == 0)
        return 0;
/* Peter: 0727 E*/

    RIFFLen   = AVIHEADERSIZE + MoviChunkLen + Idx1_Len + 4;
    
	return 1;
}



/*

Routine Description:

    Estimation current avi file size

Arguments:

    Void.

Return Value:

    size

*/
u32 EstimationFileSize(void)
{
    u32 DataSize = MoviChunkLen;
        
    if(DataSize & 0x01)//for short alignment
        DataSize     += 1;

    return (AVIHEADERSIZE + DataSize + Idx1_Len + 4);
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
s32 aviCaptureVideoStop(void)
{
    if(sysCaptureVideoStart && !sysCaptureVideoStop)
    {
        if(((asfCaptureMode & ASF_CAPTURE_EVENT_ALL) && !EventTrigger) ||
            (aviVopCount > 10 && aviAudioChunkCount > 12))
        {
            sysCaptureVideoStart    = 0;
            sysCaptureVideoStop     = 1;
            DEBUG_AVI("aviCaptureVideoStop() success!!!\n");
            return  1;
        }
    }
    else if(!sysCaptureVideoStart && sysCaptureVideoStop)
    {
        DEBUG_AVI("Not in video capture mode!!!\n");
        return  2;
    }
    DEBUG_AVI("<<%d, %d>>!!!\n",aviVopCount, aviAudioChunkCount);
    DEBUG_AVI("aviCaptureVideoStop() fail!!!\n");
    DEBUG_AVI("sysCaptureVideoStart = %d\n", sysCaptureVideoStart);
    DEBUG_AVI("sysCaptureVideoStop  = %d\n", sysCaptureVideoStop);
    DEBUG_AVI("asfCaptureMode       = %d\n", asfCaptureMode);
    DEBUG_AVI("EventTrigger         = %d\n", EventTrigger);
    DEBUG_AVI("aviVopCount          = %d\n", aviVopCount);
    DEBUG_AVI("aviAudioChunkCount    = %d\n", aviAudioChunkCount);
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
s32 aviPlaybackVideoStop(void)
{    
    if(sysPlaybackVideoStart && !sysPlaybackVideoStop)
    {
        if((mpeg4taskrun==1 && AudioPlayback==1 && video_playback_speed==5) || (mpeg4taskrun==1  && video_playback_speed!=5))
        {
            sysPlaybackVideoStart    = 0;
            sysPlaybackVideoStop     = 1;
            DEBUG_ASF("asfPlaybackVideoStop() success!!!\n");
            return  1;
        }
    }
    else if(!sysPlaybackVideoStart && sysPlaybackVideoStop)
    {
        DEBUG_ASF("Not in video capture mode!!!\n");
        return  2;
    } 
    return 0;
}


#else   // #if(MPEG4_CONTAINER_OPTION & MPEG4_CONTAINER_AVI)

s64 VideoNextPresentTime;
s32 MicroSecPerFrame;

void Output_Sem(void)
{
}

#endif

