/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    MotionDetect_HW.c

Abstract:

    The routines of Motion detection HW acceleration.


Environment:

        ARM RealView Developer Suite

Revision History:

    2010/07/21  Lucian Yuan  Create
*/


#include "general.h"
#include "board.h"
#include "task.h"
#include "i2capi.h"
#include "fsapi.h"
#include "dcfapi.h"
#include "gpioapi.h"  //lisa 070514
#include "sysapi.h"
#include "uiapi.h"
#include "iisapi.h"
#include "isuapi.h"
#include "timerapi.h"
#include "MotionDetect_API.h"
#include "GlobalVariable.h"
#include "siuapi.h"
#include "MDreg.h"
#include "dmaapi.h"

#if (HW_MD_SUPPORT && (MOTIONDETEC_ENA == 0) )
/*
 *********************************************************************************************************
 *   Constant
 *********************************************************************************************************
 */
#define DRAW_RECT_ON   1
#define DRAW_RECT_OFF  0

#define MDU_TIMEOUT    50
#define VMDSW_DEBUG    0
/*
 *********************************************************************************************************
 *   Variable
 *********************************************************************************************************
 */
int MDTxNewVMDSupport=0;
s8  SIUMODE=-1;
s8  isPIRsenSent[MAX_RFIU_UNIT];
static u32 Thr_PixelDiff[2] ;       //(10*4)   //  sum(YYCbCr1-YYCbCr2) > THR_PIXELDIFF ==> 則判斷該點為有改變.
static u32 Thr_BlockMargin[2];
static u32 gMD_Win_Height;
static u32 gMD_Win_Width;

#if VMDSW
    u32 VMD_CNT=0; //Amon : TEST (141124)
    s8 PNXYshift[8]={7, 6, 5, 4, 3, 2, 1, 0}; //Amon : TEST (141128) positive XY and negative XY of shift bit

    #if INTERPOLATION
    static s8 WindowSize= 3;
    #else
    static s8 WindowSize= 3;
    #endif
    static u32 BlockSize=1;
    static u8  PeriodThr = PeriodTime - PeriodTime/2;
    static u8  VMDSWDiffThr;
    static u8  MD_Weight=0x08;

    u8 VMDSWSense;

    u8 MDImageAccum[(MEAN_Width)*(MEAN_Height)];
    u8 MDImageDiff[(MEAN_Width)*(MEAN_Height)];
    u8 MDImageSalient[(MEAN_Width)*(MEAN_Height)];

    u32 FilterDiffCnt;
    u32 SalientCnt;
    u32 VMDDiffCnt;
    u32 MDWidth;
    u32 MDHeight;
    u32 MDWidth_INTER;
    u32 MDHeight_INTER;

    #if NEW_VMDSW_TEST
    u8 MDImageDiffTEST[(MEAN_Width)*(MEAN_Height)]; // new vmd
    #endif
#endif

u8 MD_blk_Mask_VGA[MC_CH_MAX][MD_BLOCK_NUM_MAX];
u8 MD_blk_Mask_HD[MC_CH_MAX][MD_BLOCK_NUM_MAX];


u8 MD_blk_Wnum[MC_CH_MAX][MD_BLOCK_NUM_MAX];
#if (NEW_VMDSW_TEST || NEW_VMDSW)
unsigned int MD_Blk_MeanTab[MC_CH_MAX][MD_BLOCK_NUM_MAX/2];  //Lucian: Word Unit.
#else
unsigned int MD_Blk_MeanTab[MC_CH_MAX][MD_BLOCK_NUM_MAX/4];  //Lucian: Word Unit.
#endif

DEF_MD_RECT_POS MD_RectPos[MD_DRAW_RECT_MAX+2];

u8  MD_RectNum;
u8  MD_Draw_ID;
u8  MotionDetect_en;
u32  MD_period_Preview;  //Lucian: 設定幾個frame 後,做一次motion detection
u32  MD_period_Video;
u32 MD_CHRun[MC_CH_MAX];
s32 MD_Diff=0;

OS_FLAG_GRP  *gMduFlagGrp;
OS_EVENT     *gMduOpenSem;

u8  MD_trigger = 0;     /* trigger 1:on 0:off */
u8  MD_level = 20;      /* level high : 10, medium : 20, low : 30 */
u8  MD_status = 0;      /* York 通知, 1:ON 0:OFF */

//{PixelDiff,BlockMargin}
#if(MD_SENSITIVITY_LEVEL == 5)
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={
        {10, 1},
        {20, 5},    
        {30,10},   
        {40,15},   
        {40,20}   
    };
#elif(MD_SENSITIVITY_LEVEL == 3)
  #if(PassiveIR_SensControl  == PassiveIR_SS004) // for ZN220
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={
        {5 , 2},
        {5 , 5},
        {5 , 7}
    };
  
    int PIR_SensitivityConfTab[MD_SENSITIVITY_LEVEL]={
        900,
        700,
        400
    };
  #elif(PassiveIR_SensControl  == PassiveIR_SS0041P) // for ZN220
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={
        {5 , 2},
        {5 , 5},
        {5 , 9}
    };
  
    int PIR_SensitivityConfTab[MD_SENSITIVITY_LEVEL]={
        50,
        75,
        100
    };
  #elif(PassiveIR_SensControl  == PassiveIR_PYD1588) // for ZN220, Alro PIR
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={ //set 5 2, make VMD pass easier in PIR trigger within 2sec 
        {7 , 2},
        {7 , 2},
        {7 , 2}
    };
  
    int PIR_SensitivityConfTab[MD_SENSITIVITY_LEVEL]={
        20, // 10M
        40, //  8M
        70  //  5M
        //60, //  7M
        //100  // 4M
    };  
  #elif(SW_APPLICATION_OPTION  == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={
        {5 , 5},
        {5 , 7},
        {5 , 12}
    };
  #elif(HW_BOARD_OPTION == MR9100_TX_RDI_CA811 && (PROJ_OPT == 5 || PROJ_OPT == 8)) // //M936G vs CL894CS, M936R vs CA814G
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={ //L 4,5M ~~ M 7,8M ~~H NA
        {4 , 4},
        {4 , 5},
        {5 , 10}
    };
  #else
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={
        {5 , 2},
        {5 , 4},
        {5 , 6}
    };
  #endif
#endif

    /*These var are used by RX, but we need it to avoid build error for TX*/
    int PIR_SensitivityConfTab_indoor[MD_SENSITIVITY_LEVEL]={
        20, // 8M
        40, // 6M
        70  //4M
    }; 
    int PIR_SensitivityConfTab_outdoor[MD_SENSITIVITY_LEVEL]={
        20, // 8M 
        40, // 6M
        70  //4M 
    }; 

#if(HW_BOARD_OPTION == MR9100_TX_RDI_CA811 && (PROJ_OPT == 5 || PROJ_OPT == 8)) 
    u8 MD_SensitivityConfTab_Night[MD_SENSITIVITY_LEVEL][2]={ //L 2,3M ~~ M 5,6M ~~ H NA
        {4 , 2},
        {5 , 2},
        {5 , 9}
    };
 #else
    u8 MD_SensitivityConfTab_Night[MD_SENSITIVITY_LEVEL][2]={
        {5 , 2},
        {5 , 4},
        {5 , 6}
    };
 #endif
/*
 *********************************************************************************************************
 *   Extern Variable
 *********************************************************************************************************
 */
extern u8 siuOpMode;
extern u32 MotionlessSecond;
extern volatile s32 isu_idufrmcnt;
extern u8 sysTVOutOnFlag;
extern s32 isu_avifrmcnt;
extern u32 ciu_idufrmcnt_ch1;
extern u32 ciu_idufrmcnt_ch2;
extern u32 ciu_idufrmcnt_ch3;
extern u32 ciu_idufrmcnt_ch4;
extern u32 ciu_idufrmcnt_ch5;

extern u8  video_double_field_flag;

#if VMDSW
extern u8* VMDMeanBuf1;
extern u8* VMDMeanBuf2;
extern u8* VMDPositiveCnt_X;
extern u8* VMDPositiveCnt_Y;
extern u8* VMDNegativeCnt_X;
extern u8* VMDNegativeCnt_Y;
extern u8* VMDPositiveBuf_X[10];
extern u8* VMDPositiveBuf_Y[10];
extern u8* VMDNegativeBuf_X[10];
extern u8* VMDNegativeBuf_Y[10];
extern u8* VMDFilterPX;
extern u8* VMDFilterNX;
extern u8* VMDFilterPY;
extern u8* VMDFilterNY;
extern u8* VMDFlterMap;
extern s8* VMDMotionPos_X;
extern s8* VMDMotionPos_Y;

#endif

extern  u8 *PNBuf_sub1[4];
extern  u8 *PNBuf_sub2[4];
extern  u8 sysVideoInCHsel;

/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
void mduMotionDetect_init(void);
void DrawRectangularFrame(u32 OSD_w,u32 UL_x,u32 UL_y,u32 DR_x,u32 DR_y,u8 buf_idx );
void ClearRectangularFrame(u32 OSD_w,u32 UL_x,u32 UL_y,u32 DR_x,u32 DR_y,u8 buf_idx );
void DrawMotionArea_OnTV(unsigned int Diff);

void Mark_Xaxis_Pos_OnTV(u8 Y_Start,u8 Y_End);
void Mark_Xaxis_Pos_OnPanel(u8 Y_Start,u8 Y_End);
s32 mduMotionDetect(int ID,u32 MDWinWidth,u32 MDWinHeight,u32 DrawRectEnble);

extern u32 uiGetOSDBufAdr(u8 buf_idx);
int mduRegConfig(int ID, unsigned int DiffThr,unsigned char *Yaddr,int TotalBlock,int ImgWidth,int DStype);

s8 getPrevMotion_Bit(u8* MotionBuf[10], u8 FrameCnt, u32 ByteCnt, u8 BitCnt);
void VMD_Interpolation(void);
void VMDMethodProcess(void);
void VMDSW_Sensitivity_Config(u8 SenseLevel);

/*
 *********************************************************************************************************
 * Function Body
 *********************************************************************************************************
 */

void mdIntHandler()
{
      u32 intStat;
      unsigned char err;

      intStat=MD_INT_STA;
      OSFlagPost(gMduFlagGrp, intStat , OS_FLAG_SET, &err);


}



void MotionDetect_API(int ID)
{
    u32 DetectWidth,DetectHeight;

#if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
      (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) ||\
      (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM) ||\
      (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
#else
int DrawRectOn;
#endif

    //=================//
    DetectWidth = mpeg4Width;
    DetectHeight = mpeg4Height;

    //Lucian: 偵測到啟動錄影後即關掉偵測機制.直到錄影結束再重啟偵測
    if((sysPIPMain == PIP_MAIN_CH1) && (ID == 1))
        return ;


  #if ( (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_SUBSTREAM) ||\
        (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9211_DUALMODE_TX5) ||\
        (SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD) || (SW_APPLICATION_OPTION == MR9100_AHDINREC_TX5) || (SW_APPLICATION_OPTION == MR8110_RFCAM_TX1) ||\
        (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM)  ||\
        (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    if(sysVideoInCHsel == ID)
    {
       MD_Draw_ID = ID;
    }
        MD_Diff = mduMotionDetect(ID,DetectWidth,DetectHeight,1);
     #if 0
        if(MD_Diff>0)
           DEBUG_SIU("====>Motion Detected:%d\n",MD_Diff);
     #endif
  #else
    if(sysVideoInCHsel == ID)
    {
       DrawRectOn = DRAW_RECT_ON;
       MD_Draw_ID = ID;
    }
    else
       DrawRectOn = DRAW_RECT_OFF;
    if(siuOpMode == SIUMODE_MPEGAVI)
    {
        if(!MD_Diff)
        {
            MD_Diff = mduMotionDetect(ID,DetectWidth,DetectHeight,DrawRectOn);
        }
        else
        {
             MD_Diff = mduMotionDetect(ID,DetectWidth,DetectHeight,DrawRectOn);
        }
        if(MD_Diff)
        {
            MotionlessSecond = 0 ;
        }
    }
    else
    {
        MD_Diff = mduMotionDetect(ID,DetectWidth,DetectHeight,DrawRectOn);
    #if PREVIEW_MD_TRIGGER_REC
        if(MD_Diff)
        {
            uiCaptureVideo();
        }
    #endif
    }
  #endif

#if (CIU1_BOB_REPLACE_MPEG_DF && CIU1_BOB_AUTO_MD)
  #if ( MULTI_CHANNEL_VIDEO_REC)  // multichannl
    if(sysPIPMain == PIP_MAIN_CH1)
    {
        if(MD_Diff > 15)
        {
            if(VideoClipOption[1].video_double_field_flag == 0)
                DEBUG_CIU("Ch%d video_double_field_flag = 1\n", 1);
            VideoClipOption[1].video_double_field_flag = 1;
        } else {
            if(VideoClipOption[1].video_double_field_flag == 1)
                DEBUG_CIU("Ch%d video_double_field_flag = 0\n", 1);
            VideoClipOption[1].video_double_field_flag = 0;
        }
    }
    else
    {
        if(MD_Diff > 20)
        {
            if(VideoClipOption[ID].video_double_field_flag == 0)
                DEBUG_CIU("Ch%d video_double_field_flag = 1\n", ID);
            VideoClipOption[ID].video_double_field_flag = 1;
            if(VideoClipOption[ID].asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA)
                VideoClipOption[ID].MD_Diff             = 1;
        } else {
            if(VideoClipOption[ID].video_double_field_flag == 1)
                DEBUG_CIU("Ch%d video_double_field_flag = 0\n", ID);
            VideoClipOption[ID].video_double_field_flag = 0;
        }
    }
  #else     // single channel
    if(MD_Diff > 20)
    {
        if(video_double_field_flag == 0)
            DEBUG_CIU("video_double_field_flag = 1\n");
        video_double_field_flag = 1;
    } else {
        if(video_double_field_flag == 1)
            DEBUG_CIU("video_double_field_flag = 0\n");
        video_double_field_flag =0;
    }
  #endif
#else
  #if (MULTI_CHANNEL_VIDEO_REC)  // multichannl
    if(MD_Diff > 20)
    {
        VideoClipOption[ID].video_double_field_flag = 1;
        if(VideoClipOption[ID].asfCaptureMode & ASF_CAPTURE_EVENT_MOTION_ENA)
            VideoClipOption[ID].MD_Diff             = 1;
    } else {
        VideoClipOption[ID].video_double_field_flag = 0;
    }
  #endif
#endif

}

void mduMotionDetect_init(void)
{
    u32 i,j;
    unsigned char err;

    for(j=0;j<MC_CH_MAX;j++)
    {
        for(i=0;i<MD_BLOCK_NUM_MAX;i++)
        {
            MD_blk_Mask_VGA[j][i]=0;  //Lucian: 目前初始值都設enable. 未來開機初始值,應由Nand flash 讀出(前次設定).
            MD_blk_Mask_HD[j][i]=0;
        }
    }
    
    #if (sysPIPMain == PIP_MAIN_CH1)
    #if (NEW_VMDSW_TEST || NEW_VMDSW)
    i=(MD_BLOCK_NUM_MAX/2);
    #else
    i=(MD_BLOCK_NUM_MAX/4);
    #endif
    while(i<MD_BLOCK_NUM_MAX)
    {
        if(i%20==0)
            i+=10;
        else
            i++;
        MD_blk_Mask_VGA[0][i]=1;
        MD_blk_Mask_HD[0][i]=1;
    }
    #endif

  
    for(i=0;i<MC_CH_MAX;i++)
    {
        MD_CHRun[i] = 1;
        #if (NEW_VMDSW_TEST || NEW_VMDSW)
        for(j=0;j<MD_BLOCK_NUM_MAX/2;j++)
        #else
        for(j=0;j<MD_BLOCK_NUM_MAX/4;j++)
        #endif
        MD_Blk_MeanTab[i][j]=0;
    }
    MD_RectNum=0;
    MD_Draw_ID=0;

    //Thr_PixelDiff=20;
    //Thr_BlockMargin=1;

    MD_period_Preview=8;  //must be 2^n
    MD_period_Video = 2; //must be 2^n

#if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    mduMotionDetect_Sensitivity_Config(1);
    //DEBUG_CIU("MDLV=%d\n",iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
#endif

    gMduFlagGrp = OSFlagCreate(0x00000000, &err);
    gMduOpenSem = OSSemCreate(1);


}


s32 mduMotionDetect(int ID, u32 MDWinWidth,u32 MDWinHeight,u32 DrawRectEnble)
{
    int i;
    unsigned char *F_curr = PNBuf_sub2[ (ciu_idufrmcnt_ch2-1) & 0x03];
    int Diff,WholeScreenDiff;
    int BlockNum =0;
    //----//
    gMD_Win_Width =MDWinWidth;
    gMD_Win_Height=MDWinHeight;

    memset(MD_blk_Wnum[ID],0,MD_BLOCK_NUM_MAX);
    switch(ID)
    {
       case MD_SIU_ID:
          F_curr=PNBuf_Y[(isu_avifrmcnt-1) & 0x03];
          break;
       case MD_CIU1_ID:
          F_curr=PNBuf_sub1[ (ciu_idufrmcnt_ch1-1) & 0x03];
          break;

       case MD_CIU2_ID:
          #if SWAP_MULTI_STREAM_SUPPORT
            F_curr=PNBuf_sub2[ (ciu_idufrmcnt_ch2-1) & 0x03] + VIDEODISPBUF_OFFSET;
          #else
            F_curr=PNBuf_sub2[ (ciu_idufrmcnt_ch2-1) & 0x03];
          #endif
          break;

       case MD_CIU3_ID:
          F_curr=PNBuf_sub3[ (ciu_idufrmcnt_ch3-1) & 0x03];
          break;

       case MD_CIU4_ID:
          F_curr=PNBuf_sub4[ (ciu_idufrmcnt_ch4-1) & 0x03];
          break;

       case MD_CIU5_ID:
          #if SWAP_MULTI_STREAM_SUPPORT
            F_curr=PNBuf_sub5[ (ciu_idufrmcnt_ch5-1) & 0x03] + VIDEODISPBUF_OFFSET;
          #else
          F_curr=PNBuf_sub5[ (ciu_idufrmcnt_ch5-1) & 0x03];
          #endif
          break;   

    }

    //--Use HW-MD --//
#if SWAP_MULTI_STREAM_SUPPORT
    #if (NEW_VMDSW_TEST || NEW_VMDSW)
        MDWidth=(640/16);
        MDHeight=(360/16);
        MDWidth_INTER = MDWidth*2;
        MDHeight_INTER = MDHeight*2;

        BlockNum=((640/16) * (360/16));
//        printf("ID = %d, %d %d,BlockNum=%d ,MDWinWidth =%d \n",ID,Thr_PixelDiff[SIUMODE],SIUMODE,BlockNum,MDWinWidth);
        mduRegConfig(ID,Thr_PixelDiff[SIUMODE],F_curr,BlockNum,640,1);
    #else
    #if VMDSW
        MDWidth=(640/32);
        MDHeight=(360/32);
        MDWidth_INTER = MDWidth*2-1;
        MDHeight_INTER = MDHeight*2-1;
    #endif
    BlockNum=((640/32) * (360/32));
    mduRegConfig(ID,Thr_PixelDiff[SIUMODE],F_curr,BlockNum,MDWinWidth,2);
    #endif
#else
    if(gMD_Win_Width <=360)
    {
       #if VMDSW
       MDWidth = (gMD_Win_Width/16);
       MDHeight = (gMD_Win_Height/16);
       MDWidth_INTER = MDWidth*2-1;
       MDHeight_INTER = MDHeight*2-1;
       #endif
       BlockNum=((gMD_Win_Width/16) * (gMD_Win_Height/16));
       mduRegConfig(ID,Thr_PixelDiff[SIUMODE],F_curr,BlockNum,MDWinWidth,1);
    }
    else if(gMD_Win_Width <=720)
    {
       #if (NEW_VMDSW_TEST || NEW_VMDSW)
           MDWidth=(gMD_Win_Width/16);
           MDHeight=(gMD_Win_Height/16);
           MDWidth_INTER = MDWidth*2;
           MDHeight_INTER = MDHeight*2;

           BlockNum=((gMD_Win_Width/16) * (gMD_Win_Height/16));
           mduRegConfig(ID,Thr_PixelDiff[SIUMODE],F_curr,BlockNum,MDWinWidth,1);
       #else
           #if VMDSW
           MDWidth=(gMD_Win_Width/32);
           MDHeight=(gMD_Win_Height/32);
           MDWidth_INTER = MDWidth*2-1;
           MDHeight_INTER = MDHeight*2-1;
           #endif
           BlockNum=((gMD_Win_Width/32) * (gMD_Win_Height/32));
           mduRegConfig(ID,Thr_PixelDiff[SIUMODE],F_curr,BlockNum,MDWinWidth,2);
       #endif
    }
    else if(gMD_Win_Width <=1280)
    {
       #if VMDSW
       MDWidth=(gMD_Win_Width/64);
       MDHeight=(gMD_Win_Height/64);
       MDWidth_INTER = MDWidth*2-1;
       MDHeight_INTER = MDHeight*2-1;
       #endif
       BlockNum=((gMD_Win_Width/64) * (gMD_Win_Height/64));
       mduRegConfig(ID,Thr_PixelDiff[SIUMODE],F_curr,BlockNum,MDWinWidth,4);
    }
    else if(gMD_Win_Width <=1920)
    {
       #if VMDSW
       MDWidth=(gMD_Win_Width/96);
       MDHeight=(gMD_Win_Height/96);
       MDWidth_INTER = MDWidth*2-1;
       MDHeight_INTER = MDHeight*2-1;
       #endif
       BlockNum=((gMD_Win_Width/96) * (gMD_Win_Height/96));
       F_curr += gMD_Win_Width*188;
       mduRegConfig(ID,Thr_PixelDiff[SIUMODE],F_curr,BlockNum,MDWinWidth,4);
    }
#endif
    if(MD_CHRun[ID]<3)
    {
        MD_CHRun[ID] ++;
        return 0;
    }
    //----------Check motion block------------//
    Diff=0;
    WholeScreenDiff=0;

    for(i=0;i<BlockNum;i++)
    {
       if( MD_blk_Wnum[ID][i] > 0)
       {
          WholeScreenDiff ++;
          if(MD_blk_Mask_HD[ID][i]==0)
          {
            MD_blk_Wnum[ID][i]=1;
            Diff ++;
            //DEBUG_CIU("[%d,%d]",ID,i);
          }
          else
          {
            MD_blk_Wnum[ID][i]=0;
          }
       }
       else
          MD_blk_Wnum[ID][i]=0;
    }

    if(WholeScreenDiff == BlockNum)
    {  //表示Whole screen move, 可能為關燈,地震...  則不列入
       DEBUG_CIU("Whole screen move!\n");
       Diff=0;
    }

    //-----Draw Motion Area-----//
#if 0    
    if(DrawRectEnble)
    {
        if( (Diff != 0) || (MD_RectNum != 0))
        {
           sysbackSetEvt(SYS_BACK_DRAW_MOTION_AREA_ONTV, Diff);
        }
    }
#endif    
    #if VMDSW
    if(SalientCnt && VMDSW_DEBUG)
        DEBUG_CIU("VMDSW SC(%2d)FD(%2d)DC(%2d)TB(%d)VS(%d)BS(%d)WS(%d)DT(%d)PT(%d) \n",SalientCnt,FilterDiffCnt,VMDDiffCnt,Thr_BlockMargin[SIUMODE],VMDSWSense,BlockSize,WindowSize,VMDSWDiffThr,PeriodThr);
    #endif

    if(Diff >= Thr_BlockMargin[SIUMODE])
    {
    #if MD_DEBUG_ENA   
       DEBUG_CIU("MDNum=%d,%d,%d,%d\n",ID,Diff,Thr_BlockMargin[SIUMODE],Thr_PixelDiff[SIUMODE]);
    #endif
       return Diff;
    }   
    else
       return 0;
}

void DrawMotionArea_OnTV(unsigned int Diff)
{
   u32 i,j;
   u8 *pp;
   u8 Y_Start=0;
   u8 Y_End;
   u8 FindOut_Y;
   u8 MD_Y_mark;

   if( (MD_RectNum != 0) && (MD_RectNum<=MD_DRAW_RECT_MAX))
   {
      for(i=0;i<MD_RectNum;i++)
      {
		if (sysTVOutOnFlag)
            ClearRectangularFrame(gMD_Win_Width/2,  //OSD_X = 320;
	                              MD_RectPos[i].X_Start,MD_RectPos[i].Y_Start,
	                              MD_RectPos[i].X_End,MD_RectPos[i].Y_End,
	                              2);
		else
			ClearRectangularFrame(gMD_Win_Width,
                                  MD_RectPos[i].X_Start,MD_RectPos[i].Y_Start,
                                  MD_RectPos[i].X_End,MD_RectPos[i].Y_End,
                                  2);
      }

      MD_RectNum=0;
   }


   if(Diff == 0)  //No motion, 則不畫圖.
      return;


   FindOut_Y=0;

   //===Mark_Yaxis_Pos===//

   for(j=0;j<(gMD_Win_Height/MD_BLOCK_HEIGHT);j++)
   {
       MD_Y_mark=0;
       pp=MD_blk_Wnum[MD_Draw_ID] + j*(gMD_Win_Width/MD_BLOCK_WIDTH);
       for(i=0;i<(gMD_Win_Width/MD_BLOCK_WIDTH);i++)
       {
          if(*pp == 1)
          {
            MD_Y_mark=1;
            if(FindOut_Y==0)
            {
              Y_Start=j;
              FindOut_Y=1;
            }
            break;
          }

          pp ++;
       }

       if(MD_Y_mark==0)
       {
           if(FindOut_Y==1)
           {  //代表找到ㄧ各段落
              Y_End=j-1;
              FindOut_Y=0;
              Mark_Xaxis_Pos_OnTV(Y_Start,Y_End);
           }
       }
       else //MD_Y_mark==1, boundary condition.
       {
           if( (FindOut_Y==1) && (j==(gMD_Win_Height/MD_BLOCK_HEIGHT)-1))
           {
              Y_End=j;
              FindOut_Y=0;
              Mark_Xaxis_Pos_OnTV(Y_Start,Y_End);
           }
       }

   }


}



void Mark_Xaxis_Pos_OnTV(u8 Y_Start,u8 Y_End)
{
    u32 i,j;
    u8 *pp;
    u8 X_Start=0;
    u8 X_End;
    u8 FindOut_X;
    u8 MD_X_mark;

    FindOut_X=0;
    for(i=0;i<(gMD_Win_Width/MD_BLOCK_WIDTH);i++)
    {
       MD_X_mark=0;
       pp=MD_blk_Wnum[MD_Draw_ID] + Y_Start*(gMD_Win_Width/MD_BLOCK_WIDTH) + i;
       for(j=Y_Start;j<=Y_End;j++)
       {
          if(*pp == 1)
          {
            MD_X_mark=1;
            if(FindOut_X==0)
            {
              X_Start=i;
              FindOut_X=1;
            }
            break;
          }

          pp += (gMD_Win_Width/MD_BLOCK_WIDTH);
       }

       if(MD_X_mark==0)
       {
           if(FindOut_X==1)
           {  //代表找到ㄧ各段落
              X_End=i-1;
              FindOut_X=0;
              if(MD_RectNum<MD_DRAW_RECT_MAX)
              { //預存將來要抹掉
                if (sysTVOutOnFlag)
                {
                    MD_RectPos[MD_RectNum].X_Start= X_Start*MD_BLOCK_WIDTH/2;
                    MD_RectPos[MD_RectNum].Y_Start= Y_Start*MD_BLOCK_HEIGHT/2;
                    MD_RectPos[MD_RectNum].X_End  =(X_End+1)*MD_BLOCK_WIDTH/2-1;
                    MD_RectPos[MD_RectNum].Y_End  =(Y_End+1)*MD_BLOCK_HEIGHT/2-1;
                    //DEBUG_SIU("(%d,%d)->(%d,%d)\n",X_Start,Y_Start,X_End,Y_End);

                    DrawRectangularFrame(gMD_Win_Width/2,
                                         MD_RectPos[MD_RectNum].X_Start,MD_RectPos[MD_RectNum].Y_Start,
                                         MD_RectPos[MD_RectNum].X_End,MD_RectPos[MD_RectNum].Y_End,
                                         2);
                }
				else
				{
					MD_RectPos[MD_RectNum].X_Start= X_Start*MD_BLOCK_WIDTH;
					MD_RectPos[MD_RectNum].Y_Start= Y_Start*MD_BLOCK_HEIGHT;
					MD_RectPos[MD_RectNum].X_End  =(X_End+1)*MD_BLOCK_WIDTH-1;
					MD_RectPos[MD_RectNum].Y_End  =(Y_End+1)*MD_BLOCK_HEIGHT-1;
	                DrawRectangularFrame(gMD_Win_Width,
	                                     MD_RectPos[MD_RectNum].X_Start,MD_RectPos[MD_RectNum].Y_Start,
	                                     MD_RectPos[MD_RectNum].X_End,MD_RectPos[MD_RectNum].Y_End,
	                                     2);
				}
                MD_RectNum++;

              }
           }
       }
       else //MD_X_mark==1, boundary condition.
       {
           if( (FindOut_X==1) && (i==(gMD_Win_Width/MD_BLOCK_WIDTH)-1))
           {
              X_End=i;
              FindOut_X=0;
              if(MD_RectNum<MD_DRAW_RECT_MAX)
              { //預存將來要抹掉
                if (sysTVOutOnFlag)
                {
                    MD_RectPos[MD_RectNum].X_Start= X_Start*MD_BLOCK_WIDTH/2;
                    MD_RectPos[MD_RectNum].Y_Start= Y_Start*MD_BLOCK_HEIGHT/2;
                    MD_RectPos[MD_RectNum].X_End  =(X_End+1)*MD_BLOCK_WIDTH/2-1;
                    MD_RectPos[MD_RectNum].Y_End  =(Y_End+1)*MD_BLOCK_HEIGHT/2-1;
                    //DEBUG_SIU("(%d,%d)->(%d,%d)\n",X_Start,Y_Start,X_End,Y_End);

                    DrawRectangularFrame(gMD_Win_Width/2,
                                         MD_RectPos[MD_RectNum].X_Start,MD_RectPos[MD_RectNum].Y_Start,
                                         MD_RectPos[MD_RectNum].X_End,MD_RectPos[MD_RectNum].Y_End,
                                         2);
                }
				else
				{
					MD_RectPos[MD_RectNum].X_Start= X_Start*MD_BLOCK_WIDTH;
					MD_RectPos[MD_RectNum].Y_Start= Y_Start*MD_BLOCK_HEIGHT;
					MD_RectPos[MD_RectNum].X_End  =(X_End+1)*MD_BLOCK_WIDTH-1;
					MD_RectPos[MD_RectNum].Y_End  =(Y_End+1)*MD_BLOCK_HEIGHT-1;
					DrawRectangularFrame(gMD_Win_Width,
										 MD_RectPos[MD_RectNum].X_Start,MD_RectPos[MD_RectNum].Y_Start,
										 MD_RectPos[MD_RectNum].X_End,MD_RectPos[MD_RectNum].Y_End,
										 2);
				}
                MD_RectNum++;

              }
           }
       }

     }
}




void DrawRectangularFrame(u32 OSD_w,u32 UL_x,u32 UL_y,u32 DR_x,u32 DR_y,u8 buf_idx )
{
   u8 *addr,*addr1,*addr2;
   u32 i;

   addr = (u8 *)uiGetOSDBufAdr(buf_idx);

   //---畫橫線---//
   addr1 = addr + (UL_y*OSD_w + UL_x);
   addr2 = addr + (DR_y*OSD_w + UL_x);

   memset(addr1,0xc0,DR_x-UL_x);
   memset(addr2,0xc0,DR_x-UL_x);

   //---畫直線---//
   addr1 = addr + (UL_y*OSD_w + UL_x);
   addr2 = addr + (UL_y*OSD_w + DR_x);

   for(i=UL_y;i <= DR_y; i++ )
   {
     *addr1= 0xc0;
     *addr2= 0xc0;
      addr1 += OSD_w;
      addr2 += OSD_w;
   }

}

void ClearRectangularFrame(u32 OSD_w,u32 UL_x,u32 UL_y,u32 DR_x,u32 DR_y,u8 buf_idx )
{
   u8 *addr,*addr1,*addr2;
   u32 i;

   addr = (u8 *)uiGetOSDBufAdr(buf_idx);

   //---畫橫線---//
   addr1 = addr + (UL_y*OSD_w + UL_x);
   addr2 = addr + (DR_y*OSD_w + UL_x);

   memset(addr1,0x00,DR_x-UL_x);
   memset(addr2,0x00,DR_x-UL_x);

   //---畫直線---//
   addr1 = addr + (UL_y*OSD_w + UL_x);
   addr2 = addr + (UL_y*OSD_w + DR_x);

   for(i=UL_y;i <= DR_y; i++ )
   {
     *addr1= 0x00;
     *addr2= 0x00;
      addr1 += OSD_w;
      addr2 += OSD_w;
   }

}


void mdu_TestSensitivity(char *cmd)
{
    u32 Diff,BlockMargin;

    sscanf((char*)cmd, "%d %d", &Diff,&BlockMargin);
    Thr_PixelDiff[SIUMODE]=Diff;
    Thr_BlockMargin[SIUMODE]=BlockMargin;

    DEBUG_CIU("Thr_PixelDiff = %d,Thr_BlockMargin=%d\n", Thr_PixelDiff[SIUMODE],Thr_BlockMargin[SIUMODE]);
}


#if 1
void mduMotionDetect_Sensitivity_Config(u8 SenseLevel)
{
    if(SenseLevel < MD_SENSITIVITY_LEVEL)
    {
        Thr_PixelDiff[SIU_DAY_MODE]     = MD_SensitivityConfTab[SenseLevel][0];
        Thr_BlockMargin[SIU_DAY_MODE]   = MD_SensitivityConfTab[SenseLevel][1];
        Thr_PixelDiff[SIU_NIGHT_MODE]   = MD_SensitivityConfTab_Night[SenseLevel][0];
        Thr_BlockMargin[SIU_NIGHT_MODE] = MD_SensitivityConfTab_Night[SenseLevel][1];
        #if VMDSW
        VMDSWSense = SenseLevel;
//        VMDSW_Sensitivity_Config( SenseLevel);
        #endif
    }
}
#else
void mduMotionDetect_Sensitivity_Config(u8 SenseLevel)
{
#if(MD_SENSITIVITY_LEVEL == 5)
   switch(SenseLevel)
   {
     case 0: //Hight
       Thr_PixelDiff=10;
       Thr_BlockMargin=1;
       break;

     case 1: //Midum
       Thr_PixelDiff=20;
       Thr_BlockMargin=5;
       break;

     case 2: //Midum
       Thr_PixelDiff=30;
       Thr_BlockMargin=10;
       break;

     case 3: //Midum
       Thr_PixelDiff=40;
       Thr_BlockMargin=15;
       break;

     case 4: //Low
       Thr_PixelDiff=40;
       Thr_BlockMargin=20;
       break;
   }

#elif(MD_SENSITIVITY_LEVEL == 3)
   switch(SenseLevel)
   {
     case 0: //Hight
       Thr_PixelDiff=15;
       Thr_BlockMargin=5;
       break;

     case 1: //Midum
       Thr_PixelDiff=25;
       Thr_BlockMargin=10;
       break;

     case 2: //Low
       Thr_PixelDiff=35;
       Thr_BlockMargin=20;
       break;
   }
#endif
    //DEBUG_CIU("Thr_PixelDiff = %d\n", Thr_PixelDiff);
}
#endif

void mduMotionDetect_NoiseMargin_Config(u8 Level)
{
   //Lucian: 取消此設定.
}
void mduMotionDetect_Velocity_Config(u8 SpeedLevel)
{
   /*
      Lucian: 分為 HIGH,NORMAL,LOW
              HIGH:  每200ms 做一次motion detection.
              NORMAL:  200ms
              LOW:     400ms


   */
   switch(SpeedLevel)
   {
     case MOTION_DETECT_SPEED_HIGH: //Hight speed:
       MD_period_Preview = 4; //must be 2^n
       MD_period_Video = 8;   //must be 2^n
       break;

     case MOTION_DETECT_SPEED_NORMAL: //normal speed
       MD_period_Preview=4;   //must be 2^n
       MD_period_Video=16;    //must be 2^n
       break;

     case MOTION_DETECT_SPEED_LOW: //Low speed
       MD_period_Preview=8;   //must be 2^n
       MD_period_Video=32;    //must be 2^n
       break;
   }

}

void mduMotionDetect_Mask_Config(u8 *uiMask)
{

}

void mduMotionDetect_ONOFF(u8 onoff)
{
    int i;

    if(onoff)
    {
       //DEBUG_CIU("Enable Motion Detection\n");
       MotionDetect_en = 1;

       for(i=0 ; i< MC_CH_MAX ; i++)
          MD_CHRun[i]    = 1;
    }
    else
    {
       //DEBUG_CIU("Disable Motion Detection\n");
       MotionDetect_en=0;
	}

    MD_Diff=0;
}

//---------------MDU driver-----------------//
void mduRst()
{
    int i;

    MD_CTRL = MD_CTRL_RST;
    for(i=0;i<10;i++);
    MD_CTRL = 0;
}

int mduRegConfig(int ID, unsigned int DiffThr,unsigned char *Yaddr,int TotalBlock,int ImgWidth,int DStype)
{
    int i,count;
    unsigned char err;
    #if VMDSW
    #else
    unsigned int *pMap;
    unsigned int map;
    #endif
    unsigned int *pp;
    #if VMDSW    
    u8* VMDMeantmp;
    #endif

    //----------------------//
    OSSemPend(gMduOpenSem, OS_IPC_WAIT_FOREVER, &err);

    pp = (unsigned int *)REG_MD_MEANSRAM;
    #if (NEW_VMDSW_TEST || NEW_VMDSW)
    for(i=0;i<MD_BLOCK_NUM_MAX/2;i++)
    #else
    for(i=0;i<MD_BLOCK_NUM_MAX/4;i++)
    #endif
    {
        *pp = MD_Blk_MeanTab[ID][i];
        pp ++;
    }

    OSFlagPost(gMduFlagGrp, 0x01 , OS_FLAG_CLR, &err);

    #if VMDSW
    VMDSWDiffThr = DiffThr;
    #endif
    #if (NEW_VMDSW_TEST || NEW_VMDSW)
    MD_THRESHOLD = (DiffThr | (MD_Weight<<16));
    #else
    MD_THRESHOLD = DiffThr;
    #endif
    MD_IMGWIDTH  = ImgWidth;
    MD_BLOCKNUM  = TotalBlock;
    MD_Y_BASEADDR= (unsigned int)Yaddr;
    MD_INT_ENA   = MD_INTENA_ON;

    switch(DStype)
    {
        case 1:
            MD_CTRL = MD_CTRL_DS1x1 | MD_CTRL_TRIG;
            break;

        case 2:
            MD_CTRL = MD_CTRL_DS2x2 | MD_CTRL_TRIG;
            break;

        case 4:
            MD_CTRL = MD_CTRL_DS4x4 | MD_CTRL_TRIG;
            break;

        default:
            DEBUG_CIU("Warning! Down Sample factor is inValid.\n");
            break;
    }

    OSFlagPend(gMduFlagGrp,0x01, OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, MDU_TIMEOUT, &err);

    pp = (unsigned int *)REG_MD_MEANSRAM;
    #if VMDSW
        if(VMD_CNT % 2 == 0)
            VMDMeantmp = VMDMeanBuf1;
        else
            VMDMeantmp = VMDMeanBuf2;
    #endif
    #if (NEW_VMDSW_TEST || NEW_VMDSW)
    for(i=0;i<MD_BLOCK_NUM_MAX/2;i++)
    #else
    for(i=0;i<MD_BLOCK_NUM_MAX/4;i++)
    #endif
    {
        MD_Blk_MeanTab[ID][i] = *pp;
        pp ++;

        #if VMDSW
//            if (MDTxNewVMDSupport == 1 && VMDSWSense != MOTION_DETECT_SPEED_HIGH)
                #if (NEW_VMDSW_TEST || NEW_VMDSW)
                if(i>=TotalBlock/2)
                #else
                if(i>=TotalBlock/4)
                #endif
                    break;
                #if INTERPOLATION
                    *VMDMeantmp = (char) (MD_Blk_MeanTab[ID][i]         & 0xff);VMDMeantmp+=2;
                    *VMDMeantmp = (char) (MD_Blk_MeanTab[ID][i]>>8      & 0xff);VMDMeantmp+=2;
                    *VMDMeantmp = (char) (MD_Blk_MeanTab[ID][i]>>16     & 0xff);VMDMeantmp+=2;
                    *VMDMeantmp = (char) (MD_Blk_MeanTab[ID][i]>>24     & 0xff);VMDMeantmp+=2;
                    
                    if (i != 0 && (i-4)%5 == 0)
                    {
                        VMDMeantmp+=(MDWidth_INTER-1);
                    }
                #else
                    #if (NEW_VMDSW_TEST || NEW_VMDSW)
                    *VMDMeantmp = (char) (MD_Blk_MeanTab[ID][i]         & 0xff);VMDMeantmp++;
                    *VMDMeantmp = (char) (MD_Blk_MeanTab[ID][i]>>16     & 0xff);VMDMeantmp++;
                    #else
                    *VMDMeantmp = (char) (MD_Blk_MeanTab[ID][i]         & 0xff);VMDMeantmp++;
                    *VMDMeantmp = (char) (MD_Blk_MeanTab[ID][i]>>8      & 0xff);VMDMeantmp++;
                    *VMDMeantmp = (char) (MD_Blk_MeanTab[ID][i]>>16     & 0xff);VMDMeantmp++;
                    *VMDMeantmp = (char) (MD_Blk_MeanTab[ID][i]>>24     & 0xff);VMDMeantmp++;
                    #endif
                #endif
        #endif
    }

    #if VMDSW
        VMD_Interpolation();
        VMDMethodProcess();
    #endif
    
    MD_INT_ENA   = MD_INTENA_OFF;
    if (err != OS_NO_ERR)
    {
       DEBUG_CIU("Error! Wait MDU Unit Timeout!\n");
       return 0;    // error
    }
    //-----//
    #if VMDSW
    #else
    pMap=(unsigned int *)(MDUCtrlBase + 0x0020);
    #endif
    count=0;
    while(count<TotalBlock)
    {
        #if VMDSW
            MD_blk_Wnum[ID][count] = MDImageSalient[count];
            count ++;
            
            if(count >= TotalBlock)
               break;
        #else
            map= *pMap;
            for(i=0;i<32;i++)
            {
            MD_blk_Wnum[ID][count] = map & 0x01;
                map = map>>1;
                count ++;
                if(count >= TotalBlock)
                    break;
                pMap ++;
            }
        #endif
    }

    OSSemPost(gMduOpenSem);

    return 1;
}

/**********************VMDSW************************/
#if VMDSW

    void VMDSW_Reset()
    {
        static u32 MSGCnt = 0;
        
        if ( (VMD_CNT > 30) || (MSGCnt > 3)) // 減少MSG次數
        {
            DEBUG_CIU("VMDSW_Reset() diff=%d \n",VMDDiffCnt);
            MSGCnt = 0;
        }
        MSGCnt++;
        VMD_CNT =0;
        memset_hw((void*)MDImageSalient, 0, VMD_BUF_SIZE);

    }
    void VMDSW_Sensitivity_Config(u8 SenseLevel)
    {
        if(SenseLevel < MD_SENSITIVITY_LEVEL)
        {
            VMDSWSense = SenseLevel;
        }
    }

    void VMDMultiFusion(u32 PosCnt, u8 i, u8 j)
    {
        MDImageSalient[PosCnt] = MDImageDiff[PosCnt] & *(VMDFlterMap + PosCnt);
         
        if (MDImageDiff[PosCnt])
        VMDDiffCnt ++;
        if(*(VMDFlterMap + PosCnt))
        {
            FilterDiffCnt++;
    //        DEBUG_CIU("(%d,%d)\n ",i ,j );
        }
        if(MDImageSalient[PosCnt])
        SalientCnt ++;        
    }
    void VMDRegionGrowing(u32 PosCnt)
    {
            
    }

    void VMDMotionSaveCnt2(u32 PosCnt,u8 i, u8 j, s8 dx, s8 dy)
    {
        u8  BitCnt;
        u32 ByteCnt;


        BitCnt  = PosCnt%8;
        ByteCnt = PosCnt/8;

        if (VMD_CNT >= PeriodTime)
        {
            *(VMDPositiveCnt_X + PosCnt) -= getPrevMotion_Bit(VMDPositiveBuf_X,VMD_CNT%PeriodTime,ByteCnt,BitCnt);
            *(VMDNegativeCnt_X + PosCnt) -= getPrevMotion_Bit(VMDNegativeBuf_X,VMD_CNT%PeriodTime,ByteCnt,BitCnt);
            *(VMDPositiveCnt_Y + PosCnt) -= getPrevMotion_Bit(VMDPositiveBuf_Y,VMD_CNT%PeriodTime,ByteCnt,BitCnt);
            *(VMDNegativeCnt_Y + PosCnt) -= getPrevMotion_Bit(VMDNegativeBuf_Y,VMD_CNT%PeriodTime,ByteCnt,BitCnt);
        }
        *(VMDPositiveBuf_X[VMD_CNT%PeriodTime] + ByteCnt) &= ~(1<<PNXYshift[BitCnt]);
        *(VMDNegativeBuf_X[VMD_CNT%PeriodTime] + ByteCnt) &= ~(1<<PNXYshift[BitCnt]);
        *(VMDPositiveBuf_Y[VMD_CNT%PeriodTime] + ByteCnt) &= ~(1<<PNXYshift[BitCnt]);
        *(VMDNegativeBuf_Y[VMD_CNT%PeriodTime] + ByteCnt) &= ~(1<<PNXYshift[BitCnt]);
        
        if ( dx >0)
            *(VMDPositiveBuf_X[VMD_CNT%PeriodTime] + ByteCnt) |=(1<<PNXYshift[BitCnt]);
        if ( dx <0)
            *(VMDNegativeBuf_X[VMD_CNT%PeriodTime] + ByteCnt) |=(1<<PNXYshift[BitCnt]);
        if ( dy >0)
            *(VMDPositiveBuf_Y[VMD_CNT%PeriodTime] + ByteCnt) |=(1<<PNXYshift[BitCnt]);
        if ( dy <0)
            *(VMDNegativeBuf_Y[VMD_CNT%PeriodTime] + ByteCnt) |=(1<<PNXYshift[BitCnt]);

        
        if ( dx >0)
            *(VMDPositiveCnt_X + PosCnt) +=1 ;
        if ( dx <0)
            *(VMDNegativeCnt_X + PosCnt) +=1 ;
        if ( dy >0)
            *(VMDPositiveCnt_Y + PosCnt) +=1 ;
        if ( dy <0)
            *(VMDNegativeCnt_Y + PosCnt) +=1 ;

        
#if 0
        if (PosCnt == 150 && VMD_CNT%10 == 0 )
        {
            DEBUG_CIU("%2d,%d,%d,%d,%d\n", VMD_CNT, *(VMDPositiveCnt_X + PosCnt), *(VMDPositiveCnt_Y + PosCnt), *(VMDNegativeCnt_X + PosCnt), *(VMDNegativeCnt_Y + PosCnt) );
        }
#endif
#if 0

        if (*(VMDPositiveCnt_X + PosCnt) > PeriodThr ) 
            DEBUG_CIU("PX(%d,%d)\n",i ,j );
        if (*(VMDNegativeCnt_X + PosCnt) > PeriodThr ) 
            DEBUG_CIU("NX(%d,%d)\n",i ,j );
        if (*(VMDPositiveCnt_Y + PosCnt) > PeriodThr ) 
            DEBUG_CIU("\tPY(%d,%d)\n",i ,j );
        if (*(VMDNegativeCnt_Y + PosCnt) > PeriodThr ) 
            DEBUG_CIU("\tNY(%d,%d)\n ",i ,j );
#endif
    }

    void VMDMotionResetPos(u32 PosCnt, s8 dx, s8 dy)
    {
        static u8 MotionX_CNT=0;
        static u8 MotionY_CNT=0;
        
       
        // save Motion chain

#if 1
        if(dx == 0)
            MotionX_CNT ++;
        else
            MotionX_CNT =0;
        
        if(dy == 0)
            MotionY_CNT ++;
        else
            MotionY_CNT = 0;

        if(MotionX_CNT >= 15)
        {
            *(VMDMotionPos_X+ PosCnt) = 0;
        }
        else
        {
            *(VMDMotionPos_X + PosCnt) +=dx;
        }
        if(MotionY_CNT >= 15)
        {
            *(VMDMotionPos_Y+ PosCnt) = 0;
        }
        else
        {
            *(VMDMotionPos_Y + PosCnt) +=dy;
        }
#endif
    }

    void VMDMotionExtraction2(u8 *mean_cur, u8 *mean_prev, u32 PosCnt, u8 i, u8 j, s8 *dx_temp, s8 *dy_temp )
    {
        u8 x,y;
        s8 mx ,my ;
        s8 shift_x,shift_y;
        int minmse,mse;
        u32 Pos,ShiftPos;
        u32 MeanWidthTemp,MeanHeightTemp;

        if (VMD_CNT == 0)
        {
            *dx_temp = 0;
            *dy_temp = 0;
        }
        else
        {
            
            #if INTERPOLATION
                i = i*2;
                j = j*2;
                MeanWidthTemp = MDWidth_INTER;
                MeanHeightTemp = MDHeight_INTER;
            #else
                MeanWidthTemp = MDWidth;
                MeanHeightTemp = MDHeight;
            #endif
            mx = i + *(VMDMotionPos_X + PosCnt);
            if(mx <0)
            {
                mx = 0;
                *(VMDMotionPos_X + PosCnt) = 0; 
            }
            if(mx+BlockSize> MeanWidthTemp)
            {
                mx = MeanWidthTemp - BlockSize ;
                *(VMDMotionPos_X + PosCnt) = mx-i;
            }

            my = j + *(VMDMotionPos_Y + PosCnt);
            if(my <0)
            {
                my = 0;
                *(VMDMotionPos_Y + PosCnt) = 0;
            }
            if(my+BlockSize > MeanHeightTemp)
            {
                my = MeanHeightTemp - BlockSize ;
                *(VMDMotionPos_Y + PosCnt) = my-j;
            }

            minmse = 999999;

    // first check
            mse=0;
            for(y=0;y<BlockSize;y++)
            {
        	 	for(x=0;x<BlockSize;x++)
        		{
                    Pos = (my+y)*MeanWidthTemp + (mx+x);
                    mse+=abs((*(mean_cur + Pos)-*(mean_prev + Pos)));
        		} 
            }
            if(mse<minmse)
            {
                minmse=mse;
                *dx_temp=0;
        	    *dy_temp=0;
            }

    // search window check
            for(shift_y = (-WindowSize); shift_y<=(WindowSize);shift_y++)
            {
                for(shift_x = (-WindowSize); shift_x<=(WindowSize);shift_x++)
                {
                    mse=0;
                    if(my+shift_y>=0 && mx+shift_x>=0 && my+shift_y+BlockSize<=MeanHeightTemp && mx+shift_x+BlockSize<=MeanWidthTemp)
                    {
                        for(y=0;y<BlockSize;y++)
                        {
        				 	for(x=0;x<BlockSize;x++)
        					{
    //    					    mse+=abs((mean_cur[my+y][mx+x]-mean_prev[my+y+shift_y][mx+x+shift_x]));
                                Pos = (my+y)*MeanWidthTemp + (mx+x);
                                ShiftPos = (my+y+shift_y)*MeanWidthTemp + (mx+x+shift_x);
                                mse+=abs((*(mean_cur + Pos)-*(mean_prev + ShiftPos)));
    //                            if (PosCnt == 150)
    //                                DEBUG_CIU("(%d,%d) %d,%d\n",*(mean_cur + Pos),*(mean_prev + ShiftPos),Pos,ShiftPos);
        					}
                        }
                        //DEBUG_CIU("(%d,%d)",mse,minmse);
                        if(mse<minmse && (minmse-mse) >2)
    //                    if(mse<minmse)
                        {
                            minmse=mse;
                            *dx_temp=shift_x;
        				    *dy_temp=shift_y;
                        }
                    }
                }
            }
        }
    }

    void VMDTemporalDiff(u8 *mean_cur, u8 *mean_prev, u32 PosCnt ,u8 i, u8 j)
    {
        u32 Pos;
        if (VMD_CNT == 0)
        {
            MDImageAccum[PosCnt] = 0;
            MDImageDiff[PosCnt] = 0;
        }
        else
        {
            #if INTERPOLATION
                Pos = (j*2)*MDWidth_INTER + i*2;
                MDImageAccum[PosCnt] = MDImageAccum[PosCnt]/2 + abs(*(mean_cur +Pos) - *(mean_prev + Pos))/2;
            #else
                MDImageAccum[PosCnt] = MDImageAccum[PosCnt]/2 + abs(*(mean_cur +PosCnt) - *(mean_prev + PosCnt))/2;
            #endif
            
    //        if (PosCnt == 150)
    //            DEBUG_CIU("%d %d %d %d %d\n", VMD_CNT, MDImageAccum[PosCnt], mean_cur[j][i], mean_prev[j][i], abs(mean_cur[j][i] - mean_prev[j][i])/2);
    //            DEBUG_CIU("%d %d %d %d %d\n", VMD_CNT, MDImageAccum[PosCnt], *(mean_cur +Pos), *(mean_prev + Pos), abs(*(mean_cur +Pos) - *(mean_prev + Pos))/2);

            if (MDImageAccum[PosCnt] > VMDSWDiffThr)
            {
                MDImageDiff[PosCnt] = 1;
    //            DEBUG_CIU("%d %d %d %d %d %d\n", VMD_CNT,PosCnt, MDImageAccum[PosCnt], mean_cur[j][i], mean_prev[j][i], abs(mean_cur[j][i] - mean_prev[j][i])/2);
    //            DEBUG_CIU("%d %d %d %d %d %d\n", VMD_CNT,PosCnt, MDImageAccum[PosCnt], *(mean_cur +PosCnt), *(mean_prev + PosCnt), abs(*(mean_cur +PosCnt) - *(mean_prev + PosCnt))/2);
            }
            else
                MDImageDiff[PosCnt] = 0;
            #if 0
            if(PosCnt%20 == 0)
                printf("\n%4d :",PosCnt);
            if (MDImageDiff[PosCnt])
                DEBUG_CIU(". ");
            else
                DEBUG_CIU("  ");
            #endif        
        }

    }

    //VMDSW2============================================================================================================================
    void VMD_Interpolation(void)
    {
#if INTERPOLATION
        u8 *mean_cur_ptr;
        u32 i,j;
        u32 Pos;
        if(VMD_CNT %2 == 0)
        {
            mean_cur_ptr  = VMDMeanBuf1;
        }
        else
        {
            mean_cur_ptr  = VMDMeanBuf2;
        }
        for(j = 0; j<(MDHeight_INTER) ; j+=2)
        {
            for(i = 1; i<(MDWidth_INTER) ; i+=2)
            {
                Pos  = (j*MDWidth_INTER + i);
                *(mean_cur_ptr+ Pos)= (*(mean_cur_ptr+ Pos-1) + *(mean_cur_ptr+ Pos+1) )/2;
            }
        }
        for(j = 1; j<(MDHeight_INTER) ; j+=2)
        {
            for(i = 0; i<(MDWidth_INTER) ; i+=2)
            {
                Pos  = (j*MDWidth_INTER + i);
                *(mean_cur_ptr+ Pos)= (*(mean_cur_ptr+ Pos-MDWidth_INTER) + *(mean_cur_ptr+ Pos+MDWidth_INTER) )/2;
            }
        }
        for(j = 1; j<(MDHeight_INTER) ; j+=2)
        {
            for(i = 1; i<(MDWidth_INTER) ; i+=2)
            {
                Pos  = (j*MDWidth_INTER + i);
                  *(mean_cur_ptr+ Pos)= (*(mean_cur_ptr+ Pos-1) + *(mean_cur_ptr+ Pos+1) + *(mean_cur_ptr+ Pos-MDWidth_INTER) + *(mean_cur_ptr+ Pos+MDWidth_INTER) )/4;
    //            if(Pos == 446)
    //            {
    //            DEBUG_CIU("%d, %d, %d, %d\n",VMD_CNT, *(mean_cur_ptr+ Pos-1-MEAN_Width_INTER),*(mean_cur_ptr+ Pos-MEAN_Width_INTER),*(mean_cur_ptr+ Pos+1-MEAN_Width_INTER));
    //            DEBUG_CIU("%d, %d, %d, %d\n",VMD_CNT, *(mean_cur_ptr+ Pos-1                 ),*(mean_cur_ptr+ Pos)                 ,*(mean_cur_ptr+ Pos+1                 ));
    //            DEBUG_CIU("%d, %d, %d, %d\n",VMD_CNT, *(mean_cur_ptr+ Pos-1+MEAN_Width_INTER),*(mean_cur_ptr+ Pos+MEAN_Width_INTER),*(mean_cur_ptr+ Pos+1+MEAN_Width_INTER));
    //            }
            }
        }
#endif
    }

    void VMDSW_TestSensitivity(char *cmd)
    {
        u32 block;
        u32 window;
        u32 setting;
        
        if (!strncmp((char*)cmd,"B ", strlen("B ")))
        {
            cmd+=strlen("B ");
            sscanf((char*)cmd, "%d", &block);
            BlockSize=block;
            DEBUG_CIU("BlockSize = %d\n", BlockSize);
        }
        else if (!strncmp((char*)cmd,"W ", strlen("W ")))
        {
            cmd+=strlen("W ");
            sscanf((char*)cmd, "%d", &window);
            WindowSize=window;
            DEBUG_CIU("WindowSize = %d\n",WindowSize);
        }
        else if (!strncmp((char*)cmd,"D ", strlen("D ")))
        {
            cmd+=strlen("D ");
            sscanf((char*)cmd, "%d", &setting);
            VMDSWDiffThr=setting;
            DEBUG_CIU("VMDSWDiffThr = %d\n",VMDSWDiffThr);
        }
        else if (!strncmp((char*)cmd,"P ", strlen("P ")))
        {
            cmd+=strlen("P ");
            sscanf((char*)cmd, "%d", &setting);
            PeriodThr=setting;
            DEBUG_CIU("PeriodThr = %d\n",PeriodThr);
        }
        else if (!strncmp((char*)cmd,"WT ", strlen("WT ")))
        {
            cmd+=strlen("WT ");
            sscanf((char*)cmd, "%x", &setting);
            MD_Weight=setting;
            DEBUG_CIU("MD_Weight = 0x%x \n",MD_Weight);
        }
        VMD_CNT =0;
    /*
        sscanf((char*)cmd, "%d %d %d %d", &block,&window,&diffthr,&perthr);
        BlockSize=block;
        WindowSize=window;
        VMDSWDiffThr=diffthr;
        PeriodThr=perthr;
        VMD_CNT =0;
        DEBUG_CIU("BlockSize = %d,WindowSize=%d VMDSWDiffThr=%d PeriodThr=%d\n", BlockSize,WindowSize,VMDSWDiffThr,PeriodThr);
    */
    }

    s8 getPrevMotion_Bit(u8* MotionBuf[10], u8 FrameCnt, u32 ByteCnt, u8 BitCnt)
    {
        u8 PrevMotion;
        
        PrevMotion = ( *(MotionBuf[FrameCnt] + ByteCnt) >> PNXYshift[BitCnt] ) & 0x01;
    //    DEBUG_CIU("\t(%d) ,%d ,0x%2x>>%d ,%d \n",VMD_CNT-10 ,CNT_temp ,*(CurPos[CNT_temp] + ShiftCnt) ,PNXYshift[BitCnt],PrevBit );
        return PrevMotion;
    }

    void VMDTemporalFilter(u32 PosCnt,u8 i,u8 j)
    {
        
        if (VMD_CNT >= (PeriodTime-1))
        {
            
            if (*(VMDPositiveCnt_X + PosCnt) >= PeriodThr)
                *(VMDFilterPX+ PosCnt) =1;
            else
                *(VMDFilterPX+ PosCnt) =0;
            
            if (*(VMDNegativeCnt_X + PosCnt) >= PeriodThr)
                *(VMDFilterNX+ PosCnt) =1;
            else
                *(VMDFilterNX+ PosCnt) =0;

            if (*(VMDPositiveCnt_Y + PosCnt) >= PeriodThr)
                *(VMDFilterPY+ PosCnt) =1;
            else
                *(VMDFilterPY+ PosCnt) =0;

            if (*(VMDNegativeCnt_Y + PosCnt) >= PeriodThr)
                *(VMDFilterNY+ PosCnt) =1;
            else
                *(VMDFilterNY+ PosCnt) =0;


            *(VMDFlterMap + PosCnt)= ( *(VMDFilterPX+ PosCnt) | *(VMDFilterNX+ PosCnt) | *(VMDFilterPY+ PosCnt) | *(VMDFilterNY+ PosCnt) );
    //        DEBUG_CIU("%d ",*(VMDFlterMap + PosCnt) );
    //        if( *(VMDFlterMap + PosCnt) )
    //            DEBUG_CIU("(%2d,%2d)",i,j);
        }
        
    }

#if NEW_VMDSW_TEST

    void VMDTemporalDiff1(u8 *mean_cur, u8 *mean_prev, u32 PosCnt ,u8 i, u8 j)
    {
        u32 Pos;
        if (VMD_CNT == 0)
        {
            MDImageAccum[PosCnt] = 0;
            MDImageDiff[PosCnt] = 0;
        }
        else
        {
            #if INTERPOLATION
                Pos = (j*2)*MDWidth_INTER + i*2;
                MDImageAccum[PosCnt] = MDImageAccum[PosCnt]/2 + abs(*(mean_cur +Pos) - *(mean_prev + Pos))/2;
            #else
                if(MD_Weight == 0x10)
                    MDImageAccum[PosCnt] = abs(*(mean_cur +PosCnt) - *(mean_prev + PosCnt));
                else
                    MDImageAccum[PosCnt] = MDImageAccum[PosCnt]*(16-MD_Weight)/16 + abs(*(mean_cur +PosCnt) - *(mean_prev + PosCnt))*MD_Weight/16;
            #endif
            

            if (MDImageAccum[PosCnt] >= VMDSWDiffThr)
            {
                MDImageDiff[PosCnt] = 1;
    //            DEBUG_CIU("%d %d %d %d %d %d\n", VMD_CNT,PosCnt, MDImageAccum[PosCnt], mean_cur[j][i], mean_prev[j][i], abs(mean_cur[j][i] - mean_prev[j][i])/2);
    //            DEBUG_CIU("%d %d %d %d %d %d\n", VMD_CNT,PosCnt, MDImageAccum[PosCnt], *(mean_cur +PosCnt), *(mean_prev + PosCnt), abs(*(mean_cur +PosCnt) - *(mean_prev + PosCnt))/2);
            }
            else
                MDImageDiff[PosCnt] = 0;
        #if 0
            if (PosCnt == 0)
            {
                DEBUG_CIU("%d %d %d %d %d\n", VMD_CNT, MDImageAccum[PosCnt],*(mean_cur +PosCnt),*(mean_prev + PosCnt), abs(*(mean_cur +PosCnt) - *(mean_prev + PosCnt)) );
    //            DEBUG_CIU("\n CNT = %d %\n", VMD_CNT);
    //            DEBUG_CIU("c0190020 = %8x \n",*((volatile unsigned *)(0xc0190020)));
                DEBUG_CIU("c0191000 = %8x \n",*((volatile unsigned *)(0xc0191000 +0 *4)));
    //            DEBUG_CIU("c0191004 = %8x \n",*((volatile unsigned *)(0xc0191000 +1 *4)));
    //            DEBUG_CIU("c0191008 = %8x \n",*((volatile unsigned *)(0xc0191000 +2 *4)));
    //            DEBUG_CIU("c019100C = %8x \n",*((volatile unsigned *)(0xc0191000 +3 *4)));
    //            DEBUG_CIU("c0191080 = %8x \n",*((volatile unsigned *)(0xc0191000 +80)));
            }
            if (PosCnt < 0)
            {
                DEBUG_CIU("%d %d %d %d %d\n", VMD_CNT, MDImageAccum[PosCnt],*(mean_cur +PosCnt),*(mean_prev + PosCnt), abs(*(mean_cur +PosCnt) - *(mean_prev + PosCnt)) );
    //            DEBUG_CIU("%d %x \n", VMD_CNT, *((volatile unsigned *)(0xc0191000 +PosCnt *4)));
                if (MDImageDiffTEST[PosCnt] && MDImageDiff[PosCnt])
                    DEBUG_CIU(". ");
                else if(MDImageDiffTEST[PosCnt])
                    DEBUG_CIU("H ");
                else if(MDImageDiff[PosCnt])
                    DEBUG_CIU("S ");
                else
                    DEBUG_CIU("  ");
                printf("\n");
            }
        #endif
        #if 1
        if(PosCnt%40 == 0)
            printf("\n%4d :",PosCnt);
        
        if (MDImageDiffTEST[PosCnt] && MDImageDiff[PosCnt])
            DEBUG_CIU(". ");
        else if(MDImageDiffTEST[PosCnt])
            DEBUG_CIU("H ");
        else if(MDImageDiff[PosCnt])
            DEBUG_CIU("S ");
        else
            DEBUG_CIU("  ");
        #endif        
        }

    }

    void VMDMethodProcess(void)
    {
        u8 i,j;
        s8 dx,dy;
        u8 mean_cur[MEAN_Height][MEAN_Width];
        u8 mean_prev[MEAN_Height][MEAN_Width];
        u8 *mean_cur_ptr;
        u8 *mean_prev_ptr;
        u32 PosCnt;


        int count,TotalBlock;
        unsigned int *pMap;
        unsigned int map;
        
        if(VMD_CNT %2 == 0)
        {
            mean_cur_ptr  = VMDMeanBuf1;
            mean_prev_ptr = VMDMeanBuf2;
        }
        else
        {
            mean_cur_ptr  = VMDMeanBuf2;
            mean_prev_ptr = VMDMeanBuf1;
        }
        if (VMD_CNT == 0)
        {
            memset_hw((void*)VMDPositiveCnt_X, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDPositiveCnt_Y, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDNegativeCnt_X, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDNegativeCnt_Y, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterPX, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterNX, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterPY, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterNY, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFlterMap, 0, VMD_BUF_SIZE);
            memset_hw((void*)MDImageSalient, 0, VMD_BUF_SIZE);
            memset_hw((void*)MDImageDiff, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDMotionPos_X, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDMotionPos_Y, 0, VMD_BUF_SIZE);
        }
        SalientCnt = 0;
        VMDDiffCnt =0;
        FilterDiffCnt = 0;
        
        TotalBlock    = MDWidth*MDHeight;
        pMap=(unsigned int *)(MDUCtrlBase + 0x0020);
        count=0;
        while(count<TotalBlock)
        {
                map= *pMap;
                for(i=0;i<32;i++)
                {
                    MDImageDiff[count] = map & 0x01;
                    MDImageDiffTEST[count] = map & 0x01;
                    map = map>>1;
                    count ++;
                    if(count >= TotalBlock)
                        break;
                }
                pMap ++;
        }
        
        for(j = 0; j<(MDHeight-BlockSize+1); j++)
        {
            for (i =0; i<(MDWidth-BlockSize+1); i++)
            {
                PosCnt  = (j*MDWidth + i);
                // step 1
                VMDTemporalDiff1(mean_cur_ptr, mean_prev_ptr, PosCnt, i, j);
                #if 0
                // step 2
                VMDMotionExtraction2(mean_cur_ptr, mean_prev_ptr, PosCnt,i, j, &dx, &dy);
                // save the motion displacement
                VMDMotionResetPos(PosCnt, dx, dy);
                // save the positive and negative count
                VMDMotionSaveCnt2(PosCnt, i, j, dx, dy);
                // step 3
                VMDTemporalFilter(PosCnt, i, j);
                // step 4
                VMDMultiFusion(PosCnt, i, j);
                #endif
            }
        }
        
        VMD_CNT++;
    }
#elif NEW_VMDSW

    void NewVMDTemporalDiff(u8 *mean_cur, u8 *mean_prev, u32 PosCnt ,u8 i, u8 j)
    {
        u32 Pos;
        if (VMD_CNT == 0)
        {
            MDImageAccum[PosCnt] = 0;
            MDImageDiff[PosCnt] = 0;
        }
        else
        {
            #if INTERPOLATION
                Pos = (j*2)*MDWidth_INTER + i*2;
                MDImageAccum[PosCnt] = MDImageAccum[PosCnt]/2 + abs(*(mean_cur +Pos) - *(mean_prev + Pos))/2;
            #else
                if(MD_Weight == 0x10)
                    MDImageAccum[PosCnt] = abs(*(mean_cur +PosCnt) - *(mean_prev + PosCnt));
                else
                    MDImageAccum[PosCnt] = MDImageAccum[PosCnt]*(16-MD_Weight)/16 + abs(*(mean_cur +PosCnt) - *(mean_prev + PosCnt))*MD_Weight/16;
            #endif
            
    //        if (PosCnt == 150)
    //            DEBUG_CIU("%d %d %d %d %d\n", VMD_CNT, MDImageAccum[PosCnt], mean_cur[j][i], mean_prev[j][i], abs(mean_cur[j][i] - mean_prev[j][i])/2);
    //            DEBUG_CIU("%d %d %d %d %d\n", VMD_CNT, MDImageAccum[PosCnt], *(mean_cur +Pos), *(mean_prev + Pos), abs(*(mean_cur +Pos) - *(mean_prev + Pos))/2);

            if (MDImageAccum[PosCnt] >= VMDSWDiffThr)
            {
                MDImageDiff[PosCnt] = 1;
    //            DEBUG_CIU("%d %d %d %d %d %d\n", VMD_CNT,PosCnt, MDImageAccum[PosCnt], mean_cur[j][i], mean_prev[j][i], abs(mean_cur[j][i] - mean_prev[j][i])/2);
    //            DEBUG_CIU("%d %d %d %d %d %d\n", VMD_CNT,PosCnt, MDImageAccum[PosCnt], *(mean_cur +PosCnt), *(mean_prev + PosCnt), abs(*(mean_cur +PosCnt) - *(mean_prev + PosCnt))/2);
            }
            else
                MDImageDiff[PosCnt] = 0;
            #if 0
            if(PosCnt%20 == 0)
                printf("\n%4d :",PosCnt);
            if (MDImageDiff[PosCnt])
                DEBUG_CIU(". ");
            else
                DEBUG_CIU("  ");
            #endif        
        }

    }

    void VMDMethodProcess(void)
    {
        u8 i,j;
        s8 dx,dy;
        u8 mean_cur[MEAN_Height][MEAN_Width];
        u8 mean_prev[MEAN_Height][MEAN_Width];
        u8 *mean_cur_ptr;
        u8 *mean_prev_ptr;
        u32 PosCnt;

        if(VMD_CNT %2 == 0)
        {
            mean_cur_ptr  = VMDMeanBuf1;
            mean_prev_ptr = VMDMeanBuf2;
        }
        else
        {
            mean_cur_ptr  = VMDMeanBuf2;
            mean_prev_ptr = VMDMeanBuf1;
        }
        if (VMD_CNT == 0)
        {
            memset_hw((void*)VMDPositiveCnt_X, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDPositiveCnt_Y, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDNegativeCnt_X, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDNegativeCnt_Y, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterPX, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterNX, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterPY, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterNY, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFlterMap, 0, VMD_BUF_SIZE);
            memset_hw((void*)MDImageSalient, 0, VMD_BUF_SIZE);
            memset_hw((void*)MDImageDiff, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDMotionPos_X, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDMotionPos_Y, 0, VMD_BUF_SIZE);
        }
        
        SalientCnt = 0;
        VMDDiffCnt =0;
        FilterDiffCnt = 0;
        for(j = 0; j<(MDHeight-BlockSize+1); j++)
        {
            for (i =0; i<(MDWidth-BlockSize+1); i++)
            {
                PosCnt  = (j*MDWidth + i);
                // step 1
                NewVMDTemporalDiff(mean_cur_ptr, mean_prev_ptr, PosCnt, i, j);
                // step 2
                VMDMotionExtraction2(mean_cur_ptr, mean_prev_ptr, PosCnt,i, j, &dx, &dy);
                // save the motion displacement
                VMDMotionResetPos(PosCnt, dx, dy);
                // save the positive and negative count
                VMDMotionSaveCnt2(PosCnt, i, j, dx, dy);
                // step 3
                VMDTemporalFilter(PosCnt, i, j);
                // step 4
                VMDMultiFusion(PosCnt, i, j);
            }
        }
        
        VMD_CNT++;
        if(VMDDiffCnt > (MD_BLOCK_NUM_MAX/3)) // 解決 sensor 自身亮度變化
            VMDSW_Reset();
    }

#else

    void VMDMethodProcess(void)
    {
        u8 i,j;
        s8 dx,dy;
        u8 *mean_cur_ptr;
        u8 *mean_prev_ptr;
        u32 PosCnt;
        
        if(VMD_CNT %2 == 0)
        {
            mean_cur_ptr  = VMDMeanBuf1;
            mean_prev_ptr = VMDMeanBuf2;
        }
        else
        {
            mean_cur_ptr  = VMDMeanBuf2;
            mean_prev_ptr = VMDMeanBuf1;
        }
        if (VMD_CNT == 0)
        {
            memset_hw((void*)VMDPositiveCnt_X, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDPositiveCnt_Y, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDNegativeCnt_X, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDNegativeCnt_Y, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterPX, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterNX, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterPY, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFilterNY, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDFlterMap, 0, VMD_BUF_SIZE);
            memset_hw((void*)MDImageSalient, 0, VMD_BUF_SIZE);
            memset_hw((void*)MDImageDiff, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDMotionPos_X, 0, VMD_BUF_SIZE);
            memset_hw((void*)VMDMotionPos_Y, 0, VMD_BUF_SIZE);
        }
        
        SalientCnt = 0;
        VMDDiffCnt =0;
        FilterDiffCnt = 0;
        for(j = 0; j<(MDHeight-BlockSize+1); j++)
        {
            for (i =0; i<(MDWidth-BlockSize+1); i++)
            {
                PosCnt  = (j*MDWidth + i);
                // step 1
                VMDTemporalDiff(mean_cur_ptr, mean_prev_ptr, PosCnt, i, j);
                // step 2
                VMDMotionExtraction2(mean_cur_ptr, mean_prev_ptr, PosCnt,i, j, &dx, &dy);
    //            if(PosCnt == 150 )
    //                DEBUG_CIU("%8d,%2d,%2d,%2d,%2d\n",VMD_CNT,dx,dy,*(VMDMotionPos_X + PosCnt),*(VMDMotionPos_Y + PosCnt));
                // save the motion displacement
                VMDMotionResetPos(PosCnt, dx, dy);
                // save the positive and negative count
                VMDMotionSaveCnt2(PosCnt, i, j, dx, dy);
                // step 3
                VMDTemporalFilter(PosCnt, i, j);
                // step 4
                VMDMultiFusion(PosCnt, i, j);
            }
        }
        
        VMD_CNT++;
        if(VMDDiffCnt > 100) // 解決 sensor 自身亮度變化
            VMDSW_Reset();
    }
#endif

#endif

//--------------------RX----------------------//
#else
s32 MD_Diff=0;
u8  MotionDetect_en;
s8  SIUMODE=-1;
s8  isPIRsenSent[MAX_RFIU_UNIT];
#if(MD_SENSITIVITY_LEVEL == 5)
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={
        {10, 1},
        {20, 5},    
        {30,10},   
        {40,15},   
        {40,20}   
    };
#elif(MD_SENSITIVITY_LEVEL == 3)
    #if ((HW_BOARD_OPTION == MR9200_RX_TRANWO_D8795R2) || (HW_BOARD_OPTION  == MR9200_RX_TRANWO_D8797R) ||\
         (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8710R) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8796P) ||\
         (HW_BOARD_OPTION == MR9200_RX_TRANWO_D8897H) || (HW_BOARD_OPTION == MR9200_RX_TRANWO_SH8710R))
        #if (MOTION_TYPE_FOR_JAPAN == 0) //US 9M,7M,5M
        u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={
            {5 , 5},
            {5 , 7},
            {5 , 12}
        };
        u8 MD_SensitivityConfTab_Night[MD_SENSITIVITY_LEVEL][2]={
            {5 , 5},
            {5 , 7},
            {5 , 12}
        };
        #elif (MOTION_TYPE_FOR_JAPAN == 1) //JP 7M,5M,3M
        u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={
            {5 , 7},
            {5 , 12},
            {5 , 25}
        };
        u8 MD_SensitivityConfTab_Night[MD_SENSITIVITY_LEVEL][2]={
            {5 , 7},
            {5 , 12},
            {5 , 25}
        };
        #else
        u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={
            {5 , 2},
            {5 , 4},
            {5 , 6}
        };
        u8 MD_SensitivityConfTab_Night[MD_SENSITIVITY_LEVEL][2]={
            {5 , 2},
            {5 , 4},
            {5 , 6}
        };
        #endif
    #elif(HW_BOARD_OPTION == MR9200_RX_RDI_UDR777 && (PROJ_OPT == 10 || PROJ_OPT == 11)) //M936G vs CL894CS, M936R vs CA814G, 
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={ //L 4,5M ~~ M 7,8M ~~H NA
        {4 , 4},
        {4 , 5},
        {5 , 10}
    };
    u8 MD_SensitivityConfTab_Night[MD_SENSITIVITY_LEVEL][2]={ //L 2,3M ~~ M 5,6M ~~ H NA
        {4 , 2},
        {5 , 2},
        {5 , 9}
    };
    #elif(HW_BOARD_OPTION == MR9200_RX_RDI_M906) //M906 vs CL894 day only
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={ //L 4,5M ~~ M 6,7M ~~H 8,9M
        {4 , 5},
        {5 , 8},
        {5 , 10}
    };
    u8 MD_SensitivityConfTab_Night[MD_SENSITIVITY_LEVEL][2]={ //L 2,3M ~~ M 5,6M ~~ H NA
        {4 , 2},
        {5 , 2},
        {5 , 9}
    };
    #else
    u8 MD_SensitivityConfTab[MD_SENSITIVITY_LEVEL][2]={
        {5 , 2},
        {5 , 4},
        {5 , 6}
    };
    u8 MD_SensitivityConfTab_Night[MD_SENSITIVITY_LEVEL][2]={
        {5 , 2},
        {5 , 4},
        {5 , 6}
    };
    #endif
#endif

#if (HW_BOARD_OPTION == MR9200_RX_MAYON_MWM903)
    int PIR_SensitivityConfTab_indoor[MD_SENSITIVITY_LEVEL]={
        20,
        40,
        70
        //40, // 8M 
        //50, // 6M
        //60  //4M 
    };
    int PIR_SensitivityConfTab_outdoor[MD_SENSITIVITY_LEVEL]={
        20,
        40,
        70
    }; 
#else
    int PIR_SensitivityConfTab_indoor[MD_SENSITIVITY_LEVEL]={
        20,
        40,
        70
    }; 
    int PIR_SensitivityConfTab_outdoor[MD_SENSITIVITY_LEVEL]={
        20,
        40,
        70
    }; 
#endif

void mdIntHandler()
{
      u32 intStat;

      intStat=MD_INT_STA;

}
void mduMotionDetect_ONOFF(u8 onoff){}
void DrawMotionArea_OnTV(unsigned int Diff){}
void mduMotionDetect_Sensitivity_Config(u8 SenseLevel){}
#endif


