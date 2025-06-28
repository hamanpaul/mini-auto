/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	iisapi.h

Abstract:

   	The application interface of IIS.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __IIS_API_H__
#define __IIS_API_H__


/* constant */

#define DEFAULT_WIDTH	1		//8-bits
#define DEFAULT_CHAN	1		// 1 channel
#define DEFAULT_RATE	8000	//frequency
#define DEFAULT_BYTES_PER_SEC	8192	//Bytes

#if(HW_BOARD_OPTION==SALIX_SDV||HW_BOARD_OPTION==HX_DH500)
    #if(SDV_SELECT == SSDV_2)
        #define DEFAULT_BYTES_PER_SHOT_12M	2550	//Bytes 1034*1024
        #define DEFAULT_BYTES_PER_SHOT_5M       1550 //Sector Unit
        #define DEFAULT_BYTES_PER_SEC_MP4        460
    #elif (SDV_SELECT == SSDV_1)
        #define DEFAULT_BYTES_PER_SHOT_VGA       140  // Sector unit
        #define DEFAULT_BYTES_PER_SEC_MP4        540
    #endif
#elif(HW_BOARD_OPTION==ULTMOST_SDV)
		#define DEFAULT_BYTES_PER_SHOT_12M	    3302
		#define DEFAULT_BYTES_PER_SHOT_8M       2796
		#define DEFAULT_BYTES_PER_SHOT_5M       1986
		#define DEFAULT_BYTES_PER_SHOT_3M       974
		#define DEFAULT_BYTES_PER_SHOT_1_3M     396
		#define DEFAULT_BYTES_PER_SHOT_VGA      140
		#define DEFAULT_BYTES_PER_SEC_MP4       460
#elif((HW_BOARD_OPTION==SHUOYING_SDV)||(HW_BOARD_OPTION==SUNWAY_SDV))
		#define DEFAULT_BYTES_PER_SHOT_2M       726
		#define DEFAULT_BYTES_PER_SHOT_1_3M     396
		#define DEFAULT_BYTES_PER_SEC_MP4       460
#elif(HW_BOARD_OPTION==WENSHING_SDV)
        #define DEFAULT_BYTES_PER_SHOT_VGA       140  // Sector unit
        #define DEFAULT_BYTES_PER_SEC_MP4        540
#else
        #define DEFAULT_BYTES_PER_SHOT_12M	    2550	//Bytes 1034*1024
		#define DEFAULT_BYTES_PER_SHOT_5M       1550 //Sector Unit
		#define DEFAULT_BYTES_PER_SEC_MP4        460
#endif


/*Peter 1109 S*/
#define IIS_SAMPLE_RATE     16000  //must modify adc_project.h
#if (IIS_SAMPLE_RATE == 8000)
#define IIS_PLAYBACK_SIZE   256
#define IIS_RECORD_SIZE     256           /* keep IISTimeUnit 32ms, (IIS_RECORD_SIZE * 1000) / IIS_SAMPLE_RATE */
#define IIS_CHUNK_SIZE     1024
#elif (IIS_SAMPLE_RATE == 16000)
#define IIS_PLAYBACK_SIZE   512
#define IIS_RECORD_SIZE     512
#define IIS_CHUNK_SIZE     2048
#endif
#define IIS_CHUNK_TIME      ((IIS_CHUNK_SIZE * 1000) / IIS_SAMPLE_RATE)  /* keep 128 ms */ /* Peter 070104 */
/*Peter 1109 E*/

#define DEFAULT_DATA_POS	0x28	//Default DATA size position

/* max number of table entry */
#define	IIS_SOUN_SAMPLE_MAX		0x00000200

#define IIS_SYSPLL_SEL_48M         0
#define IIS_SYSPLL_SEL_64M         1

/* type definition */

typedef struct _IIS_BUF_MNG
{
	u32	flag;
	u32	time;
	u32	size;
	u8* 	buffer;	
} IIS_BUF_MNG;
		
/* variable */

extern OS_EVENT* iisTrgSemEvt;
extern OS_EVENT* iisCmpSemEvt;
extern OS_EVENT* iisplayCmpEvt; //civic 070829
extern OS_EVENT* AudioRTPCmpSemEvt[];
extern OS_EVENT* P2PAudioCmpSemEvt[];
extern OS_EVENT* P2PAudioPlaybackCmpSemEvt; //Toby
//extern OS_EVENT* UiUpdateEvt;
extern u32 iisSounBufMngReadIdx;
extern u32 iisSounBufMngWriteIdx;
extern IIS_BUF_MNG iisSounBufMng[]; 
extern IIS_BUF_MNG P2PiisSounBufMng[]; //Toby
extern u32 P2PiisSounBufMngWriteIdx;
//extern u8 *iisSounBuf[IIS_BUF_NUM];   /*Peter 1109 S*/
extern u8 MuteRec;
extern u32	CurrentAudioSize;
extern s64 IISTime;        // Current IIS playback time(micro second)
extern s32 IISTimeOffset;  // Current IIS playback time offset(micro second)
extern u32 IISTimeUnit;    // IIS playback time per DMA(micro second)

#if (AUDIO_IN_TO_OUT || REMOTE_TALK_BACK)
extern OS_EVENT*   iisPlaybackSemEvt;
extern u32         iisSounBufMngPlayIdx;
#endif
extern s8 P2PEnableStreaming[];


/* function prototype */

extern s32 iisResumeTask(void);
extern s32 iisSuspendTask(void);
extern s32 iisResumePlaybackTask(void);
extern s32 iisSuspendPlaybackTask(void);
extern s32 iisPreviewI2OBegin(void);
extern s32 iisPreviewI2OEnd(void);

extern s32 iisInit(void);
extern s32 iisReset(u8);      /*Peter 1109 S*/
extern void iisIntHandler(void);
#if IIS_TEST
  extern void iisTest(void);
#endif
extern s32 iisCaptureVideoInit(void);   /* Peter 070104 */
extern s32 wavRecVoice(void);	/* Civic 070829 */
extern s32 wavRecVoiceFile(void);	/* Civic 070829 */
extern s32 wavreadfile(void);
extern s32 ac97SetupALC203_AdjustVolumn(u8 Volumn);
extern void iisEnaInt(void);
extern void iisDisInt(void);
extern void iisMute(u32 Mute);
extern s32 iisStopRec(void);
extern s32 iisStopPlay(void);

#if ((HW_BOARD_OPTION==ES_LIGHTING))

extern s32 iisStartPlay2(void);
extern s32 iisStopPlay2(u8 mode);
extern s32 iisStartRec2(void);
extern s32 iisStopRec2(void);
extern s32 iisSetPlayDma2(u8* buf, u32 siz);
extern s32 iisSetRecDma2(u8* buf, u32 siz);
extern INT32U iisWaitForIntOrCancel(INT32U mode, INT16U Timeout, INT32U* RecordLen);
extern INT32U iisCancelDMA(INT32U mode);
extern s32 iisCloseDMA(u8 mode);
extern s32 iisGetPlayDmaStatue(void);

#endif

/* Audio record mode */
enum
{
    nomo_8bit_8k = 0x00  ,
    nomo_8bit_16k   ,
    nomo_8bit_32k   ,
    nomo_8bit_441k  ,
    nomo_8bit_48k   , 
    
    nomo_16bit_8k   , //0x05
    nomo_16bit_16k  ,
    nomo_16bit_32k  ,
    nomo_16bit_441k ,
    nomo_16bit_48k  ,
    
    stero_8bit_8k   , //0x0A
    stero_8bit_16k  ,    
    stero_8bit_32k  ,
    stero_8bit_441k ,
    stero_8bit_48k  ,
    
    stero_16bit_8k  , //0x0F
    stero_16bit_16k ,
    stero_16bit_32k ,
    stero_16bit_441k,
    stero_16bit_48k ,
    Audio_test_end    //0x14
};
extern u8 Audio_formate;
#endif
