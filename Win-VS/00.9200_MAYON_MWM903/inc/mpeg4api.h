/*

Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

	mpeg4api.h

Abstract:

   	The application interface of the MPEG4 encoder/decoder.

Environment:

    	ARM RealView Developer Suite

Revision History:
	
	2005/08/26	David Tsai	Create	

*/

#ifndef __MPEG4_API_H__
#define __MPEG4_API_H__


/* define MPEG4 header marker */
#define VIDOBJ_START_CODE       0x00000100  /* ..0x0000011f  */
#define VIDOBJLAY_START_CODE    0x00000120  /* ..0x0000012f */
#define VISOBJSEQ_START_CODE    0x000001b0
#define VISOBJSEQ_STOP_CODE     0x000001b1  /* ??? */
#define USERDATA_START_CODE     0x000001b2
#define GRPOFVOP_START_CODE     0x000001b3
#define VISOBJ_START_CODE       0x000001b5

#define VISOBJ_TYPE_VIDEO               1

#define VIDOBJLAY_TYPE_SIMPLE           1
#define VIDOBJLAY_TYPE_CORE             3
#define VIDOBJLAY_TYPE_MAIN             4

#define VIDOBJLAY_AR_EXTPAR             15

#define VIDOBJLAY_SHAPE_RECTANGULAR     0
#define VIDOBJLAY_SHAPE_BINARY          1
#define VIDOBJLAY_SHAPE_BINARY_ONLY     2
#define VIDOBJLAY_SHAPE_GRAYSCALE       3

#define VISUAL_OBJECT_SEQ_START_CODE 0x1b0
#define VISUAL_OBJECT_START_CODE     0x1b5
#define VO_START_CODE  0x8
#define VOL_START_CODE 0x12
#define VOP_START_CODE 0x1b6


#define MPEG4_TIMEOUT   5  /*CY 1023*/


#define I_VOP		0		/* vop coding modes */
#define P_VOP		1
#define B_VOP		2
#define D_VOP       3    


#define MPEG4_VDPACKET_SIZE     (3200 >> 3)
#define MPEG4_INTRAINTER_THD        512
#define MPEG4_ONEFOURMV_THD         200 
#define MPEG4_BIASSAD_16            60
#define MPEG4_BIASSAD_8                 260

#define VOP_TIME_INCREMENT_RESOLUTION   30000  
#define VOP_TIME_INCREMENT      (VOP_TIME_INCREMENT_RESOLUTION/30)

#define MPEG4_REC_QVGA      0
#define MPEG4_REC_QHD       1
#define MPEG4_REC_HD        2
#define MPEG4_REC_FULLHD    3


/*----------------- flag of frame-----------------*/
#define FLAG_I_VOP			0x00000001
#define FLAG_P_VOP			0x00000000

#define MPEG4_VIDEO_QUALITY_HIGH    0
#define MPEG4_VIDEO_QUALITY_MEDIUM  1
#define MPEG4_VIDEO_QUALITY_LOW     2

#define MPEG4_VIDEO_FRAMERATE_30    0
#define MPEG4_VIDEO_FRAMERATE_15    1
#define MPEG4_VIDEO_FRAMERATE_5     2
#define MPEG4_VIDEO_FRAMERATE_10    3
#define MPEG4_VIDEO_FRAMERATE_60    4


#define MPEG4_ERROR                 0
#define MPEG4_NORMAL                1
#define MPEG4_N_VOP                 2

#define MPEG_BITRATE_LEVEL_100      0
#define MPEG_BITRATE_LEVEL_80       1
#define MPEG_BITRATE_LEVEL_60       2

/*----------------- type definition --------------*/

typedef struct _MPEG4_BUF_MNG
{
	u32	flag;
	u32	asfflag;
	s64	time;
	u32	size;
    u32 offset;  //Lucian: record the offset of  bottom filed bitstream start position.
	u8* buffer;	
} VIDEO_BUF_MNG;

typedef struct _MPEG4_RF_BUF_MNG
{
    u32	flag;
	s64	time;
	u32	size;
	u32 buffer;	

} MPEG4_RF_BUF_MNG;

typedef struct 
{
    u8  *rdptr;
    s32 Read_bytecnt;
    s32 Bigbitpos; 
} MP4Dec_Bits;

typedef struct 
{
    int Height;
    int Width;
    int PictureType;
    int fcode_for;
    int RTYPE;
    int InitQP;
    int Quant_type;
    int intra_dc_vlc_thr;
    int resync_marker_disable;
    int time_increment_resolution;
    int fix_vop_timeincr_length;
    int time_inc;

    //---新增部分---//
	unsigned int VideoPictureIndex;
} MP4_Option;

typedef struct 
{
   //--Mpeg4 Inner buffer--//
   u8 *mpeg4MVBuf;
 
   u8 *mpeg4PRefBuf_Y;
   u8 *mpeg4PRefBuf_Cb;
   u8 *mpeg4PRefBuf_Cr;
   u8 *mpeg4NRefBuf_Y;
   u8 *mpeg4NRefBuf_Cb;
   u8 *mpeg4NRefBuf_Cr;
   
   //--I/O buffer index--//
}DEF_RF_MP4DEC_BUF;		
/*----------------- variable ---------------------*/

extern u32 IVOP_PERIOD;
extern u8  mpeg4SliceMask[];
extern const u32 PutBitsMask[];
extern u32 HeaderBuf;
extern u32 mpeg4VopTimeInc;
extern u32 mpegflag;

extern u8 filecon;
extern u8 splitmenu;
#if FORCE_FPS
extern u8  Stuffing[];
#endif

extern OS_EVENT* VideoTrgSemEvt;
extern OS_EVENT* VideoCmpSemEvt;
extern OS_EVENT* VideoRTPCmpSemEvt[];
extern OS_EVENT* P2PVideoCmpSemEvt[];
extern OS_EVENT* P2PVideoPlaybackCmpSemEvt; //Toby 130815
extern OS_EVENT* USBVideoCmpSemEvt[];
extern OS_EVENT* USBVideoPlaybackCmpSemEvt; //Toby 130815
extern OS_EVENT* VideoCpleSemEvt;  /*Peter 1109 S*/
extern u32 VideoBufMngReadIdx;
extern u32 VideoBufMngWriteIdx;
extern VIDEO_BUF_MNG VideoBufMng[];
extern u32	CurrentVideoSize;
extern VIDEO_BUF_MNG P2PVideoBufMng[]; //Toby 130815
extern VIDEO_BUF_MNG P2PBusyVideoBufMng[]; //Toby 130815
extern u32 MPEG4_Mode;     // 0: record, 1: playback
extern u32 MPEG4_Status;
extern u32 MPEG4_Task_Go;  // 0: never run, 1: ever run
extern s32 mp4_avifrmcnt; /*BJ 0530 S*/	
extern u32 MPEG4_Error;
extern u8  Video_Pend;
extern u8  dftMpeg4Quality;  //設定Video Clip Quality. Set Qp.
extern u8  mpeg4VideoRecQulity;
extern u8  VideoRecFrameRate;
extern MP4_Option  Mp4Dec_opt;
extern u32 mpeg4Width,  mpeg4Height;        /*CY 0907*/
extern s8 P2PEnableStreaming[];
extern s8 P2PEnableplaybackStreaming; //Toby 130815
extern s8 USBEnableStreaming[];
extern s8 USBEnableplaybackStreaming; //Toby 130815
extern u32 mpeg4MultiStreamEnable; 
extern u32 mpeg4MultiStreamStart;


/*------------------ function prototype ----------------*/

extern s32 mpeg4SetVideoResolution(u16, u16);
extern s32 mpeg4GetVideoResolution(int *pwidth, int *pheight);
extern s32 mpeg4SetVideoQuality(u8);
extern s32 mpeg4SetVideoFrameRate(u8 framerate);

extern void mpeg4ConfigQualityFrameRate(int BitRateLevel);


extern s32 mpeg4EncodeVolHeader(u8*, u32*);    /* Peter: 0711 */

extern s32 mpeg4ResumeTask(void);
extern s32 mpeg4SuspendTask(void);

extern s32 mpeg4Init(void);
extern void mpeg4IntHandler(void);
extern void mpeg4Test(void);


u32 mpeg4DecodeVolHeader(MP4_Option*,u8*, u32);     /* Peter: 0711 */
u32 mpeg4DecodeVOP(u8*, u32, u8, u8);
u32 mpeg4PutDummyVOPHeader(u32 Width,u32 Height,u8 *pBuf, u32 *byteno);


#if CDVR_LOG
void ChangeLogFileStartAddress(void);
#endif

extern s32 rfiuMpeg4EncodeVolHeader(u8* pHeader, u32* pHeaderSize, u32 Width, u32 Height,int Use_MPEG_Q);    
extern u32 rfiuMpeg4DecodeVOP(MP4_Option *pMp4Dec_opt, 
                              u8* pVopBit, 
                              u32 BitsLen, 
                              int RFUnit,
                              unsigned int Offset,
                              int DispMode,
                              int FieldDecEn
                             );
extern s32 rfiuMpeg4Decoding1Frame(MP4_Option *pMp4Dec_opt,MP4Dec_Bits* Bits, 
	                                         u32 BitsLen,int RFUnit,
	                                         int DispMode,u8 *BotFieldBits,
	                                         int FieldDecEn);
#endif
