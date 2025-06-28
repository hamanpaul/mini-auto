/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    ciu_1.c

Abstract:

    The routines of CCIR656 Interface Unit-CH1.
    1. TV decoder 參數設定:
        a. Preview/Capture/Video clip mode.

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
#include "ciuapi.h"
#include "ciureg.h"
#include "ciu.h"
#include "MDreg.h"
#include "MotionDetect_API.h"
#include "siuapi.h"
#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
#if (TUTK_SUPPORT==1)
#if(LWIP2_SUPPORT)
#include "../LwIP_2.0/include/tutk_P2P/AVIOCTRLDEFs.h"
#else
#include "../LwIP/include/tutk_P2P/AVIOCTRLDEFs.h"
#endif
#endif
#include "gfuapi.h"


/*
 *********************************************************************************************************
 *  SYS Constant
 *********************************************************************************************************
 */

#define CIU_TIMEOUT			20
#define CIU_MAXOSDSTR       128
#define PULSE_TIME          250   //每輸出250次Hight Level 大約 花0.5ms

#define DIU_TIMEOUT         2
#define SP_TEST             0     /*Amon 150305 */

#define CIU_MASKAREA_TEST                        0 /*Amon 150630 */
/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
OS_FLAG_GRP  *ciuFlagGrp_CH1;
OS_FLAG_GRP  *ciuFlagGrp_CH2;
OS_FLAG_GRP  *ciuFlagGrp_CH3;
OS_FLAG_GRP  *ciuFlagGrp_CH4;
OS_FLAG_GRP  *ciuFlagGrp_CH5;


#if (HW_DEINTERLACE_CIU1_ENA || HW_DEINTERLACE_CIU2_ENA || HW_DEINTERLACE_CIU3_ENA || HW_DEINTERLACE_CIU4_ENA || HW_DEINTERLACE_CIU5_ENA)
OS_EVENT *diuReadySemEvt;
OS_EVENT *DIUCpleSemEvt; /*BJ 0530 S*/
#endif

OS_EVENT* ciuCapSemEvt_CH1;     // for Video capture
OS_EVENT* ciuCapSemEvt_CH2;     // for Video capture
OS_EVENT* ciuCapSemEvt_CH3;     // for Video capture
OS_EVENT* ciuCapSemEvt_CH4;     // for Video capture
OS_EVENT* ciuCapSemEvt_CH5;     // for Video capture

OS_STK ciuTaskStack_CH1[CIU_TASK_STACK_SIZE_CH1]; /* Stack of task ciuTask() */
OS_STK ciuTaskStack_CH2[CIU_TASK_STACK_SIZE_CH2]; /* Stack of task ciuTask() */
OS_STK ciuTaskStack_CH3[CIU_TASK_STACK_SIZE_CH3]; /* Stack of task ciuTask() */
OS_STK ciuTaskStack_CH4[CIU_TASK_STACK_SIZE_CH4]; /* Stack of task ciuTask() */
OS_STK ciuTaskStack_CH5[CIU_TASK_STACK_SIZE_CH5]; /* Stack of task ciuTask() */

u32 ciu_idufrmcnt_ch1 = 0;
u32 ciu_idufrmcnt_ch2 = 0;
u32 ciu_idufrmcnt_ch3 = 0;
u32 ciu_idufrmcnt_ch4 = 0;
u32 ciu_idufrmcnt_ch5 = 0;

int diuForceBobMode[MC_CH_MAX];
int diuForceBobLetecy[MC_CH_MAX];

static char ciuszString1[CIU_MAXOSDSTR]  = "";
static char ciuszString2[CIU_MAXOSDSTR]  = "";
static char ciuszString3[CIU_MAXOSDSTR]  = "";
static char ciuszString4[CIU_MAXOSDSTR]  = "";
static char ciuszString5[CIU_MAXOSDSTR]  = "";


static char ciuszString1_SP[CIU_MAXOSDSTR]  = "";
static char ciuszString2_SP[CIU_MAXOSDSTR]  = "";
static char ciuszString5_SP[CIU_MAXOSDSTR]  = "";


u8 ciu_1_OpMode = SIUMODE_PREVIEW;
u8 ciu_2_OpMode = SIUMODE_PREVIEW;
u8 ciu_3_OpMode = SIUMODE_PREVIEW;
u8 ciu_4_OpMode = SIUMODE_PREVIEW;
u8 ciu_5_OpMode = SIUMODE_PREVIEW;


u32 ciu_1_FrameTime;
u32 ciu_2_FrameTime;
u32 ciu_3_FrameTime;
u32 ciu_4_FrameTime;
u32 ciu_5_FrameTime;

u32 ciu_1_OutX;
u32 ciu_2_OutX;
u32 ciu_3_OutX;
u32 ciu_4_OutX;
u32 ciu_5_OutX;


u32 ciu_1_OutY;
u32 ciu_2_OutY;
u32 ciu_3_OutY;
u32 ciu_4_OutY;
u32 ciu_5_OutY;


u32 ciu_1_line_stride;
u32 ciu_2_line_stride;
u32 ciu_3_line_stride;
u32 ciu_4_line_stride;
u32 ciu_5_line_stride;


u32 ciu_1_OutWidth;
u32 ciu_2_OutWidth;
u32 ciu_3_OutWidth;
u32 ciu_4_OutWidth;
u32 ciu_5_OutWidth;


u32 ciu_1_OutHeight;
u32 ciu_2_OutHeight;
u32 ciu_3_OutHeight;
u32 ciu_4_OutHeight;
u32 ciu_5_OutHeight;


u32 ciu_1_pnbuf_size_y;
u32 ciu_2_pnbuf_size_y;
u32 ciu_5_pnbuf_size_y;
//u32 ciu_3_pnbuf_size_y;
//u32 ciu_4_pnbuf_size_y;
u32 ciu_1_pnbuf_min_size_y;
u32 ciu_2_pnbuf_min_size_y;
u32 ciu_5_pnbuf_min_size_y;

u32 ciu_1_PIP_OutWidth;
u32 ciu_2_PIP_OutWidth;
u32 ciu_3_PIP_OutWidth;
u32 ciu_4_PIP_OutWidth;
u32 ciu_5_PIP_OutWidth;

u32 ciu_1_PIP_OutHeight;
u32 ciu_2_PIP_OutHeight;
u32 ciu_3_PIP_OutHeight;
u32 ciu_4_PIP_OutHeight;
u32 ciu_5_PIP_OutHeight;

u32 ciu_1_PIP_OutX;
u32 ciu_2_PIP_OutX;
u32 ciu_3_PIP_OutX;
u32 ciu_4_PIP_OutX;
u32 ciu_5_PIP_OutX;

u32 ciu_1_PIP_OutY;
u32 ciu_2_PIP_OutY;
u32 ciu_3_PIP_OutY;
u32 ciu_4_PIP_OutY;
u32 ciu_5_PIP_OutY;

u32 ciu_1_FPS_Count;
u32 ciu_2_FPS_Count;

u32 ciu_5_FPS_Count;


/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
#if TX_PIRREC_SUPPORT 
extern u32 rfiuStopPIRRecReady;
#endif
#if TX_PIRREC_VMDCHK
extern u32 rfiuPIRRec_VMDTrig;
#endif

#if(Sensor_OPTION  == Sensor_MI_5M)
extern OS_EVENT* siuSemEvt;
#endif

extern s32 MD_Diff;

extern u8 siuOpMode;
extern u32 *CiuOverlayImg1_Top;
extern u32 *CiuOverlayImg1_Bot;

extern u32 *CiuOverlayImg2_Top;
extern u32 *CiuOverlayImg2_Bot;

extern u32 *CiuOverlayImg3_Top;
extern u32 *CiuOverlayImg3_Bot;

extern u32 *CiuOverlayImg4_Top;
extern u32 *CiuOverlayImg4_Bot;

extern u32 *CiuOverlayImg5_Top;
extern u32 *CiuOverlayImg5_Bot;

extern u32 *CiuOverlayImg1_SP_Top;
extern u32 *CiuOverlayImg1_SP_Bot;

extern u32 *CiuOverlayImg2_SP_Top;
extern u32 *CiuOverlayImg2_SP_Bot;

extern u32 *CiuOverlayImg5_SP_Top;
extern u32 *CiuOverlayImg5_SP_Bot;

extern u32  MD_period_Preview;  //Lucian: 設定幾個frame 後,做一次motion detection
extern u32  MD_period_Video;

extern u8 sysReady2CaptureVideo;
extern u8 uiMenuVideoSizeSetting;
extern u8 video_double_field_flag;

extern u8  szVideoOverlay1[MAX_OVERLAYSTR];
extern u8  szVideoOverlay2[MAX_OVERLAYSTR];
extern u8  szLogString[MAX_OVERLAYSTR];
extern u8  IcrOnFlag;
extern u8  IcrOffFlag;
extern s8  SIUMODE;
extern u8 *MaskAreaBuf;

#if HW_MD_SUPPORT
  extern u8 MotionDetect_en;
#endif

extern u32 VideoPictureIndex;
extern u32 VideoSmallPictureIndex;
//extern s32 isu_avifrmcnt;
#if (TV_DECODER == AHD6124B)
extern int chip_id[4];
extern unsigned int nvp6124_cnt ;
#endif
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */
s32 CiuInit(void);

void ciuTask_CH1(void* pData);
void ciuTask_CH2(void* pData);
void ciuTask_CH3(void* pData);
void ciuTask_CH4(void* pData);
void ciuTask_CH5(void* pData);



void ciuIntHandler_CH1(void);
void ciuIntHandler_CH2(void);
void ciuIntHandler_CH3(void);
void ciuIntHandler_CH4(void);
void ciuIntHandler_CH5(void);



s32 ciuPreviewInit_CH1(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
s32 ciuPreviewInit_CH2(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
s32 ciuPreviewInit_CH3(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
s32 ciuPreviewInit_CH4(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
s32 ciuPreviewInit_CH5(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);



s32 GenerateCIU1_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);
s32 GenerateCIU2_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);
s32 GenerateCIU3_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);

s32 GenerateCIU4_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);

s32 GenerateCIU5_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);


s32 GenerateCIU1_SP_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);
s32 GenerateCIU2_SP_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);
s32 GenerateCIU5_SP_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);

/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */

#if CIU_SUPPORT_EN
s32 CiuInit(void)
{
    int i;
    u8 err;

    SYSReset(SYS_RSTCTL_CIU_RST);  // 一定要 RESET SPLITER
#if (MULTI_CHANNEL_SEL & 0x02)
	memset(CiuOverlayImg1_Top, 0, 640 * 80 );
    memset(CiuOverlayImg1_Bot, 0, 640 * 80 );
    
    memset(CiuOverlayImg1_SP_Top, 0, 640 * 80 );
    memset(CiuOverlayImg1_SP_Bot, 0, 640 * 80 );
#endif

#if (MULTI_CHANNEL_SEL & 0x04)
    SYSReset(SYS_RSTCTL_CIU2_RST);
    memset(CiuOverlayImg2_Top, 0, 640 * 80 );
    memset(CiuOverlayImg2_Bot, 0, 640 * 80 );
    
    memset(CiuOverlayImg2_SP_Top, 0, 640 * 80 );
    memset(CiuOverlayImg2_SP_Bot, 0, 640 * 80 );
#endif

#if (MULTI_CHANNEL_SEL & 0x08)
    SYSReset_EXT(SYS_CTL0_EXT_CIU3_RST);
    memset(CiuOverlayImg3_Top, 0, 640 * 80 );
    memset(CiuOverlayImg3_Bot, 0, 640 * 80 );
#endif

#if (MULTI_CHANNEL_SEL & 0x10)
    SYSReset_EXT(SYS_CTL0_EXT_CIU4_RST);
    memset(CiuOverlayImg4_Top, 0, 640 * 80 );
    memset(CiuOverlayImg4_Bot, 0, 640 * 80 );
#endif

#if (MULTI_CHANNEL_SEL & 0x20)
    SYSReset(SYS_RSTCTL_ISU_RST);
    memset(CiuOverlayImg5_Top, 0, 640 * 80 );
    memset(CiuOverlayImg5_Bot, 0, 640 * 80 );
    
    memset(CiuOverlayImg5_SP_Top, 0, 640 * 80 );
    memset(CiuOverlayImg5_SP_Bot, 0, 640 * 80 );
#endif


#if HW_MD_SUPPORT
    mduMotionDetect_init();
#endif

    for(i=0;i<MC_CH_MAX;i++)
    {
       diuForceBobMode[i]=0;
       diuForceBobLetecy[i]=0;
    }
    //----CH1-----//
    // Create the semaphore
    ciuFlagGrp_CH1=OSFlagCreate(0x00000000, &err);
    ciuCapSemEvt_CH1    = OSSemCreate(0);
    OSTaskCreate(CIU_TASK_CH1, CIU_TASK_PARAMETER_CH1, CIU_TASK_STACK_CH1, CIU_TASK_PRIORITY_CH1);

    //----CH2-----//
    // Create the semaphore
    ciuFlagGrp_CH2=OSFlagCreate(0x00000000, &err);
    ciuCapSemEvt_CH2    = OSSemCreate(0);
    OSTaskCreate(CIU_TASK_CH2, CIU_TASK_PARAMETER_CH2, CIU_TASK_STACK_CH2, CIU_TASK_PRIORITY_CH2);

    //----CH3-----//
    // Create the semaphore
    ciuFlagGrp_CH3=OSFlagCreate(0x00000000, &err);
    ciuCapSemEvt_CH3    = OSSemCreate(0);
    OSTaskCreate(CIU_TASK_CH3, CIU_TASK_PARAMETER_CH3, CIU_TASK_STACK_CH3, CIU_TASK_PRIORITY_CH3);

    //----CH4-----//
    // Create the semaphore
    ciuFlagGrp_CH4=OSFlagCreate(0x00000000, &err);
    ciuCapSemEvt_CH4    = OSSemCreate(0);
    OSTaskCreate(CIU_TASK_CH4, CIU_TASK_PARAMETER_CH4, CIU_TASK_STACK_CH4, CIU_TASK_PRIORITY_CH4);

    //----CH5-----//
    // Create the semaphore
    ciuFlagGrp_CH5=OSFlagCreate(0x00000000, &err);
    ciuCapSemEvt_CH5    = OSSemCreate(0);
    OSTaskCreate(CIU_TASK_CH5, CIU_TASK_PARAMETER_CH5, CIU_TASK_STACK_CH5, CIU_TASK_PRIORITY_CH5);

#if (HW_DEINTERLACE_CIU1_ENA || HW_DEINTERLACE_CIU2_ENA || HW_DEINTERLACE_CIU3_ENA || HW_DEINTERLACE_CIU4_ENA || HW_DEINTERLACE_CIU5_ENA)
    diuReadySemEvt = OSSemCreate(1);
    DIUCpleSemEvt  = OSSemCreate(0);
#endif

	return 1;
}

void ciu_1_Stop(void)
{
    CIU_1_CTL2 &= (~(CIU_DATA_OUT_ENA | CIU_INT_ENA_FRAME_END));
    OSTimeDly(4);
    #if CIU_SPLITER
        CIU_1_CTL2 = CIU_SPLITER_ENA;
    #else
        CIU_1_CTL2 = 0;
    #endif
#if ((SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM))

#else
    SYSReset(SYS_RSTCTL_CIU_RST);
#endif
}

void ciu_2_Stop(void)
{
    CIU_2_CTL2 &= (~(CIU_DATA_OUT_ENA | CIU_INT_ENA_FRAME_END));
    OSTimeDly(4);
    #if CIU_SPLITER
        CIU_2_CTL2 = CIU_SPLITER_ENA;
    #else
        CIU_2_CTL2 = 0;
    #endif

#if ((SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM))
    DEBUG_CIU("===ciu_2_Stop===\n");
#else
    DEBUG_CIU("===ciu_2_Stop===\n");
    SYSReset(SYS_RSTCTL_CIU2_RST);
#endif

}

void ciu_3_Stop(void)
{
    CIU_3_CTL2 &= (~(CIU_DATA_OUT_ENA | CIU_INT_ENA_FRAME_END));
    OSTimeDly(4);
    #if CIU_SPLITER
        CIU_3_CTL2 = CIU_SPLITER_ENA;
    #else
        CIU_3_CTL2 = 0;
    #endif
#if ((SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM))

#else    
    SYSReset_EXT(SYS_CTL0_EXT_CIU3_RST);
#endif

}

void ciu_4_Stop(void)
{
    CIU_4_CTL2 &= (~(CIU_DATA_OUT_ENA | CIU_INT_ENA_FRAME_END));
    OSTimeDly(4);
    #if CIU_SPLITER
        CIU_4_CTL2 = CIU_SPLITER_ENA;
    #else
        CIU_4_CTL2 = 0;
    #endif

 #if ((SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_MUTISTREAM) || (SW_APPLICATION_OPTION == MR9120_RFCAM_TX1_SUBSTREAM))

 #else
    SYSReset_EXT(SYS_CTL0_EXT_CIU4_RST);
 #endif

}

void ciu_5_Stop(void)
{
    //CIU_5_CTL2 &= (~(CIU_DATA_OUT_ENA | CIU_INT_ENA_FRAME_END));
    //OSTimeDly(2);    
    //CIU_5_CTL2 = 0; //still cause reboot
    //OSTimeDly(3);    
    CIU_5_CTL2 &= ~CIU_ENA;
    while(CIU_5_CTL2 & 0x1);
    #if (Sensor_OPTION == Sensor_XC7021_SC2133 || Sensor_OPTION == Sensor_XC7021_GC2023) //pin mux off to disable sensor
    GpioActFlashSelect &= ~CHIP_IO_SEN_EN;
    #endif
}

void ciuDayNight_ICR_ctrl(void)
{
#if (HW_BOARD_OPTION == MR9160_TX_DB_BATCAM)
    u32 i;
    u8  level = 0;

    if (gpioGetLevel(GPIO_GROUP_DAY_NIGHT, GPIO_PIN_DAY_NIGHT, &level) == 1)
    {
        level = (level == SIU_DAY_MODE) ? SIU_DAY_MODE : SIU_NIGHT_MODE;
    }

    if(level == SIU_NIGHT_MODE)
    {
        //DEBUG_GPIO("CIU Enter night mode %d\n",level);
        gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0); //ICR on/ off default hi, make a pulse and on-low/ off-hi
        //OSTimeDly(1);
        for(i=0; i<252000; i++); // spec define at least 10ms, 252000 = 10.105ms
        gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)

        //siuSetSensorDayNight(SIU_NIGHT_MODE); 
        SIUMODE = SIU_NIGHT_MODE;
    }
    else
    {
        //DEBUG_GPIO("CIU Enter day mode %d\n",level); //白天使用紅外線濾光, 只讓可見光通過, 使影像清晰
        gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0); //on-hi, off-low
        //OSTimeDly(1);
        for(i=0; i<252000; i++);
        gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)

        //siuSetSensorDayNight(SIU_DAY_MODE);
        SIUMODE = SIU_DAY_MODE;
    }
#elif (HW_BOARD_OPTION == MR9160_TX_OPCOM_BATCAM) 
    u32 i, adcvalue;
    u8  level = 0;

    adcvalue = adcGetValue(2);
    if(adcvalue > 0x960)  // 未fine tune
        level=SIU_NIGHT_MODE;
    else if(adcvalue < 0xAF0)
        level=SIU_DAY_MODE;

    if(level == SIU_NIGHT_MODE)
    {
        //DEBUG_GPIO("CIU Enter night mode %d\n",level);
        gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0); //ICR on/ off default hi, make a pulse and on-low/ off-hi
        //OSTimeDly(1);
        for(i=0; i<252000; i++); // spec define at least 10ms, 252000 = 10.105ms
        gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)

        //siuSetSensorDayNight(SIU_NIGHT_MODE); 
        SIUMODE = SIU_NIGHT_MODE;
    }
    else
    {
        //DEBUG_GPIO("CIU Enter day mode %d\n",level); //白天使用紅外線濾光, 只讓可見光通過, 使影像清晰
        gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0); //on-hi, off-low
        //OSTimeDly(1);
        for(i=0; i<252000; i++);
        gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)

        //siuSetSensorDayNight(SIU_DAY_MODE);
        SIUMODE = SIU_DAY_MODE;
    }
#elif (HW_BOARD_OPTION == MR9160_TX_MAYON_MWL613)

    u32 i, adcvalue = 0;
    u8  level = 0;
    u8 DCshift = 0;

    DCshift = adcGetValue(1) > 0x800 ? 0x30 : 0 ;

    for (i = 0; i < 4; i++)
        adcvalue = adcGetValue(2);
    adcvalue = 0;
    for (i = 0; i < 4; i++)
        adcvalue += adcGetValue(2);
    adcvalue = adcvalue >> 2;

  #if NEW_LIGHT_SENSOR
    adcvalue = (adcvalue < DCshift) ? 0 : (adcvalue - DCshift);
  
    if(adcvalue < 0x30)  // 未fine tune
        level=SIU_NIGHT_MODE;
    else if(adcvalue > 0x130)
        level=SIU_DAY_MODE;
    
  #else
    adcvalue = (adcvalue < 38) ? 0 : (adcvalue - 38); //adc is always higher when boot

    if(adcvalue < 0x50)  // 未fine tune
        level=SIU_NIGHT_MODE;
    else if(adcvalue > 0x180)
        level=SIU_DAY_MODE;
    
  #endif

    if(level == SIU_NIGHT_MODE)
    {
        //DEBUG_GPIO("CIU Enter night mode %d\n",level);
        #if 0 //(IRLED_CONTROLL_BY_SENSOR == 1) //sensor not ready
            i2cWrite_SENSOR(0x0, 0x0);
            i2cWrite_SENSOR(0xE0, 0x9A); //driving
            i2cWrite_SENSOR(0x0, 0x6);
            i2cWrite_SENSOR(0x0F, 0x21); //IR LED on
        #endif

        #if 0
            i2cWrite_SENSOR(0x0, 0x0);
            i2cWrite_SENSOR(0xE0, 0xAA); //driving
            i2cWrite_SENSOR(0x0, 0x6);
            i2cWrite_SENSOR(0x0E, 0x51); //ICR on
            for(i=0; i<252000; i++);
            i2cWrite_SENSOR(0x0E, 0x20); //clear output
        #else
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0); //ICR on/ off default hi, make a pulse and on-low/ off-hi
            //OSTimeDly(1);
            //for(i=0; i<252000; i++); // spec define at least 10ms, 252000 = 10.105ms
            //gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)
        #endif
        //siuSetSensorDayNight(SIU_NIGHT_MODE); 
        SIUMODE = SIU_NIGHT_MODE;
    }
    else
    {
        //DEBUG_GPIO("CIU Enter day mode %d\n",level); //白天使用紅外線濾光, 只讓可見光通過, 使影像清晰
        #if 0//(IRLED_CONTROLL_BY_SENSOR == 1) //sensor not ready
            i2cWrite_SENSOR(0x0, 0x6);
            i2cWrite_SENSOR(0x0F, 0x1); //IR LED off
        #endif
        #if 0
            i2cWrite_SENSOR(0x0, 0x0);
            i2cWrite_SENSOR(0xE0, 0xAA); //driving
            i2cWrite_SENSOR(0x0, 0x6);
            i2cWrite_SENSOR(0x0E, 0x49); //ICR off
            for(i=0; i<252000; i++);
            i2cWrite_SENSOR(0x0E, 0x20); //clear output
        #else
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0); //on-hi, off-low
            //OSTimeDly(1);
            //for(i=0; i<252000; i++);
            //gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)
        #endif
        //siuSetSensorDayNight(SIU_DAY_MODE);
        SIUMODE = SIU_DAY_MODE;
    }
#elif (HW_BOARD_OPTION == MR9160_TX_ROULE_BATCAM)
    
        u32 i, adcvalue = 0;
        u8  level = 0;
        u8 DCshift = 0;
       
        for (i = 0; i < 4; i++)
            adcvalue = adcGetValue(2);
        adcvalue = 0;
        for (i = 0; i < 4; i++)
            adcvalue += adcGetValue(2);
        adcvalue = adcvalue >> 2;

        if(adcvalue > 0x400)  // 未fine tune
            level=SIU_NIGHT_MODE;
        else if(adcvalue < 0x380)
            level=SIU_DAY_MODE;

    
        if(level == SIU_NIGHT_MODE)
        {
            //siuSetSensorDayNight(SIU_NIGHT_MODE); 
            SIUMODE = SIU_NIGHT_MODE;
        }
        else
        {
            //siuSetSensorDayNight(SIU_DAY_MODE);
            SIUMODE = SIU_DAY_MODE;
        }

#endif
}
    
void ciuTask_DayNight_ICR_ctrl(void)
{
#if ( (HW_BOARD_OPTION == A1018B_SKB_128M_TX) || (HW_BOARD_OPTION == MR9120_TX_OPCOM_USB_6M) || (HW_BOARD_OPTION == MR9120_TX_OPCOM_CVI) ||\
      (HW_BOARD_OPTION == MR9120_TX_SKY_USB) || (HW_BOARD_OPTION == MR9120_TX_SKY_AHD) || (HW_BOARD_OPTION == MR9100_TX_SKY_AHD) ||\
      (HW_BOARD_OPTION == A1019A_SKB_128M_TX) || (HW_BOARD_OPTION == MR9100_TX_SKY_W_AHD) || (HW_BOARD_OPTION == MR9100_TX_SKY_USB) ||\
      (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_HD_USB) || (HW_BOARD_OPTION == MR9100_TX_OPCOM_CVI_SK))
    static u8  level = 0;

    #if (PK3100_FORCE2DAYMODE)
        level=SIU_DAY_MODE;
        if(SIUMODE != level)
        {
            if(level == SIU_DAY_MODE)
            {
                //siuSetSensorDayNight(SIU_DAY_MODE);
                gpioSetLevel(GPIO_GROUP_DAY_NIGHT,GPIO_PIN_DAY_NIGHT,0); // led off
                IcrOnFlag = 4; /*在timer 100ms 發 plus  4 : 300ms*/
                SIUMODE    = SIU_DAY_MODE;
            }
        }
    #else
        #if 0 //  修改電路調整 CDS
        if(adcGetValue(2) < 2200) // 20150911
            level=SIU_DAY_MODE;
        else if(adcGetValue(2) > 2600) // 20150911
            level=SIU_NIGHT_MODE;
        #else
        if(adcGetValue(2) < 0x960) // 20150428
            level=SIU_DAY_MODE;
        else if(adcGetValue(2) > 0xAF0) // 20150428
            level=SIU_NIGHT_MODE;
        #endif
        
        if(SIUMODE != level)
        {
            if(level == SIU_DAY_MODE)
            {
                siuSetSensorDayNight(SIU_DAY_MODE);
                gpioSetLevel(GPIO_GROUP_DAY_NIGHT,GPIO_PIN_DAY_NIGHT,0); // led off
                IcrOnFlag = 4; /*在timer 100ms 發 plus  4 : 300ms*/
                SIUMODE    = SIU_DAY_MODE;
            }
            else
            {
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                gpioSetLevel(GPIO_GROUP_DAY_NIGHT,GPIO_PIN_DAY_NIGHT,1); // led on
                IcrOffFlag = 4; /*在timer 100ms 發 plus  4 : 300ms*/
                SIUMODE    = SIU_NIGHT_MODE;
            }
        }
    #endif
#elif (HW_BOARD_OPTION == MR9120_TX_BT_USB)
    static u8  level = 0;

    if(adcGetValue(2) < 3200) // 20160516
        level=SIU_DAY_MODE;
    else if(adcGetValue(2) > 3600) // 20160516
        level=SIU_NIGHT_MODE;


    if(SIUMODE != level)
    {
        if(level == SIU_DAY_MODE)
        {
            siuSetSensorDayNight(SIU_DAY_MODE);
            gpioSetLevel(GPIO_GROUP_DAY_NIGHT,GPIO_PIN_DAY_NIGHT,0); // led off
            IcrOnFlag = 4; /*在timer 100ms 發 plus  4 : 300ms*/
            SIUMODE    = SIU_DAY_MODE;
        }
        else
        {
            siuSetSensorDayNight(SIU_NIGHT_MODE);
            gpioSetLevel(GPIO_GROUP_DAY_NIGHT,GPIO_PIN_DAY_NIGHT,1); // led on
            IcrOffFlag = 4; /*在timer 100ms 發 plus  4 : 300ms*/
            SIUMODE    = SIU_NIGHT_MODE;
        }
    }
#elif( (HW_BOARD_OPTION == MR9120_TX_RDI_USB) ||\
       (HW_BOARD_OPTION == MR9100_TX_RDI) || (HW_BOARD_OPTION == MR9120_TX_RDI) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA840)||\
       (HW_BOARD_OPTION == MR9100_TX_RDI_CA811))
    static u8  level = 0;
    static u8 Mode   = -1;
    static u8 MDdropF = 0;
    static u8 MDdropFCNT = 0;
    u8  mdset;

    if (gpioGetLevel(GPIO_GROUP_DAY_NIGHT, GPIO_PIN_DAY_NIGHT, &level) == 0)
        level = SIU_DAY_MODE;
    
    if(Mode != level)
    {
        #if HW_MD_SUPPORT
        if (level == SIU_DAY_MODE)
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x01)== 0x1 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
        }
        else
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x02)== 0x2 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
            
        }
        mdset = MotionDetect_en;
	    #endif
        if(mdset)
        {
        #if HW_MD_SUPPORT
           mduMotionDetect_ONOFF(0);
	    #endif
        }
       
        if(level == SIU_DAY_MODE)
        {
            DEBUG_SIU("@@enter day \n");
            siuSetSensorDayNight(SIU_DAY_MODE);
            Mode    = SIU_DAY_MODE;
            SIUMODE    = SIU_DAY_MODE;
            #if HW_MD_SUPPORT
                mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
            #endif
        }
        else 
        {
            DEBUG_SIU("##enter night \n");
            #if ( ((HW_BOARD_OPTION == MR9100_TX_RDI_CA840)&&(PROJ_OPT==0)) || (HW_BOARD_OPTION == MR9100_TX_RDI_CA811))
            siuSetSensorDayNight(SIU_NIGHT_MODE);
            #endif
            Mode    = SIU_NIGHT_MODE;
            SIUMODE    = SIU_NIGHT_MODE;
            #if HW_MD_SUPPORT
                mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
            #endif
        }

        if(mdset)
        {
        #if HW_MD_SUPPORT
           MDdropF = 1;
           MDdropFCNT = 0;
//               mduMotionDetect_ONOFF(1);
	    #endif
        }
    }
    #if HW_MD_SUPPORT
        if (MDdropF == 1) // 夜間切換日間 誤觸 MD
        {
            MDdropFCNT ++;
            if (MDdropFCNT >15)
            {
                mduMotionDetect_ONOFF(1);
                MDdropF = 0;
            }
        }
    #endif
#elif ( (HW_BOARD_OPTION == MR9120_TX_RDI_CA831) || (HW_BOARD_OPTION  == A1019A_TX_RDI_CA831) || (HW_BOARD_OPTION  == MR9100_TX_RDI_USB) )
    static u8  level = 0;
    static u8 Mode   = -1;
    static u8 MDdropF = 0;
    static u8 MDdropFCNT = 0;
    u8  mdset;

    if (gpioGetLevel(GPIO_GROUP_DAY_NIGHT, GPIO_PIN_DAY_NIGHT, &level) == 0)
        level = SIU_DAY_MODE;
    
    if(Mode != level)
    {
        #if HW_MD_SUPPORT
        mdset = MotionDetect_en;
        if(mdset)
           mduMotionDetect_ONOFF(0);
	    #endif
        
        if(level == SIU_DAY_MODE)
        {
            DEBUG_SIU("@@enter day \n");
            siuSetSensorDayNight(SIU_DAY_MODE);
            Mode    = SIU_DAY_MODE;
            SIUMODE    = SIU_DAY_MODE;
            #if HW_MD_SUPPORT
                mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
            #endif
        }
        else 
        {
            DEBUG_SIU("##enter night \n");
            siuSetSensorDayNight(SIU_NIGHT_MODE);
            Mode    = SIU_NIGHT_MODE;
            SIUMODE    = SIU_NIGHT_MODE;
            #if HW_MD_SUPPORT
                mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
            #endif
        }

        if(mdset)
        {
        #if HW_MD_SUPPORT
           MDdropF = 1;
           MDdropFCNT = 0;
//               mduMotionDetect_ONOFF(1);
	    #endif
        }
    }
    #if HW_MD_SUPPORT
        if (MDdropF == 1) // 夜間切換日間 誤觸 MD
        {
            MDdropFCNT ++;
            if (MDdropFCNT >15)
            {
                mduMotionDetect_ONOFF(1);
                MDdropF = 0;
            }
        }
    #endif
#elif((HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T)||(HW_BOARD_OPTION == MR9120_TX_TRANWO_D87T))
    u16 i;
    static u8  level = 0;
    static u8 Mode   = -1;
    static u8 MDdropF = 0;
    static u8 MDdropFCNT = 0;
    u8  mdset;

    if (gpioGetLevel(GPIO_GROUP_DAY_NIGHT, GPIO_PIN_DAY_NIGHT, &level) == 0)
        level = SIU_DAY_MODE;

    if(Mode != level)
    {
        #if HW_MD_SUPPORT
        mdset = MotionDetect_en;
        if(mdset)
            mduMotionDetect_ONOFF(0);
        #endif

        if(level == SIU_NIGHT_MODE)
        {
            DEBUG_GPIO("!@##$Enter night mode %d\n",level);
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
            
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1);
            OSTimeDly(4);
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);

            for(i=0;i<12500; i++)
                gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);    
            gpioSetLevel(GPIO_GROUP_IR_POWER, GPIO_PIN_IR_POWER, 1);
            //夜間彩色
            siuSetSensorDayNight(SIU_NIGHT_MODE); 
            Mode=SIU_NIGHT_MODE;
            SIUMODE = SIU_NIGHT_MODE;
        }
        else
        {
            DEBUG_GPIO("Enter day mode %d\n",level);
            gpioSetLevel(GPIO_GROUP_IR_POWER, GPIO_PIN_IR_POWER, 0);
            for(i=0;i<12500; i++)
                gpioSetLevel(GPIO_GROUP_IR_POWER, GPIO_PIN_IR_POWER, 0);    
            
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1);
            OSTimeDly(4);
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
            
            siuSetSensorDayNight(SIU_DAY_MODE);
            Mode=SIU_DAY_MODE;    
            SIUMODE = SIU_DAY_MODE;
        }
        if(mdset)
        {
        #if HW_MD_SUPPORT
           MDdropF = 1;
           MDdropFCNT = 0;
	    #endif
        }

    }
    #if HW_MD_SUPPORT
        if (MDdropF == 1) // 夜間切換日間 誤觸 MD
        {
            MDdropFCNT ++;
            if (MDdropFCNT >15)
            {
                mduMotionDetect_ONOFF(1);
                MDdropF = 0;
            }
        }
    #endif
#elif(HW_BOARD_OPTION == MR9100_TX_JIT_C707HW4) //no day/night gpio pin
    u16 i;
    u32 adcvalue;
    static u8  level = 0;
    static u8 Mode   = -1;
    static u8 MDdropF = 0;
    static u8 MDdropFCNT = 0;
    u8  mdset;

    adcvalue = adcGetValueAverage(2,15);
    if(adcvalue < 0x960)  // 未fine tune
        level=SIU_DAY_MODE;
    else if(adcvalue > 0xAF0)
        level=SIU_NIGHT_MODE;



    if(Mode != level)
    {
        #if HW_MD_SUPPORT
        mdset = MotionDetect_en;
        if(mdset)
            mduMotionDetect_ONOFF(0);
        #endif

        if(level == SIU_NIGHT_MODE)
        {
            DEBUG_GPIO("!@##$Enter night mode %d\n",level);
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
            
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1);
            OSTimeDly(4);
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);

            for(i=0;i<12500; i++)
                gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);    
            gpioSetLevel(GPIO_GROUP_IR_POWER, GPIO_PIN_IR_POWER, 1);
            //夜間彩色
            siuSetSensorDayNight(SIU_NIGHT_MODE); 
            Mode=SIU_NIGHT_MODE;
            SIUMODE = SIU_NIGHT_MODE;
        }
        else
        {
            DEBUG_GPIO("Enter day mode %d\n",level);
            gpioSetLevel(GPIO_GROUP_IR_POWER, GPIO_PIN_IR_POWER, 0);
            for(i=0;i<12500; i++)
                gpioSetLevel(GPIO_GROUP_IR_POWER, GPIO_PIN_IR_POWER, 0);    
            
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1);
            OSTimeDly(4);
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
            
            siuSetSensorDayNight(SIU_DAY_MODE);
            Mode=SIU_DAY_MODE;    
            SIUMODE = SIU_DAY_MODE;
        }
        if(mdset)
        {
        #if HW_MD_SUPPORT
           MDdropF = 1;
           MDdropFCNT = 0;
	    #endif
        }

    }
    #if HW_MD_SUPPORT
        if (MDdropF == 1) // 夜間切換日間 誤觸 MD
        {
            MDdropFCNT ++;
            if (MDdropFCNT >15)
            {
                mduMotionDetect_ONOFF(1);
                MDdropF = 0;
            }
        }
    #endif
#elif(HW_BOARD_OPTION == MR9100_TX_MAYON_MWL612) //no day/night gpio pin
    u16 i;
    u32 adcvalue = 0;
    static u8  level = 0;
    static u8 Mode   = -1;
    static u8 MDdropF = 0;
    static u8 MDdropFCNT = 0;
    u8  mdset = 0;
    u32 u32LastTick = 0;


#if 0
    adcvalue = adcGetValueAverage(2, 5);
    if (adcvalue == 0) return;
    if(adcvalue > 0x960)  // 未fine tune
        level=SIU_DAY_MODE;
    else if(adcvalue < 0xAF0)
        level=SIU_NIGHT_MODE;

#else

    if(u32LastTick == 0)
    {
        u32LastTick = OS_tickcounter;
        adcvalue = adcGetValue(2);
    }
    else if(OS_tickcounter - u32LastTick < 300) //15 sec
    {
        adcvalue = adcGetValue(2);
    }
    else
    {
        adcvalue = adcGetValueAverage(2, 5);
        if (adcvalue == 0)
            return;
    }
        
    if(adcvalue > 0x960)  // 未fine tune
        level=SIU_DAY_MODE;
    else if(adcvalue < 0xAF0)
        level=SIU_NIGHT_MODE;
#endif

    if(Mode != level)
    {
        #if HW_MD_SUPPORT
        if (level == SIU_DAY_MODE)
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x01)== 0x1 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
        }
        else
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x02)== 0x2 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
            
        }
        mdset = MotionDetect_en;
        if(mdset)
            mduMotionDetect_ONOFF(0);
        #endif

        if(level == SIU_NIGHT_MODE)
        {
            DEBUG_GPIO("!@##$Enter night mode %d\n",level);
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_ON, 0); //ICR on/ off default hi, make a pulse and on-low/ off-hi
            OSTimeDly(3);
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_ON, 1); //on-hi, off-hi =>IR cut斷電 (QM224)

            siuSetSensorDayNight(SIU_NIGHT_MODE); 
            Mode=SIU_NIGHT_MODE;
            SIUMODE = SIU_NIGHT_MODE;
        }
        else
        {
            DEBUG_GPIO("Enter day mode %d\n",level); //白天使用紅外線濾光, 只讓可見光通過, 使影像清晰
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_OFF, 0); //on-hi, off-low
            OSTimeDly(3);
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_OFF, 1); //on-hi, off-hi =>IR cut斷電 (QM224)

            siuSetSensorDayNight(SIU_DAY_MODE);
            Mode=SIU_DAY_MODE;    
            SIUMODE = SIU_DAY_MODE;
        }
        if(mdset)
        {
        #if HW_MD_SUPPORT
           MDdropF = 1;
           MDdropFCNT = 0;
	    #endif
        }

    }
    #if HW_MD_SUPPORT
        if (MDdropF == 1) // 夜間切換日間 誤觸 MD
        {
            MDdropFCNT ++;
            if (MDdropFCNT >15)
            {
                mduMotionDetect_ONOFF(1);
                MDdropF = 0;
            }
        }
    #endif
#elif( (HW_BOARD_OPTION == MR9100_TX_DB_AHDIN) || (HW_BOARD_OPTION == MR9100_TX_MUXCOM_AHDIN))

#elif(HW_BOARD_OPTION == MR9160_TX_DB_BATCAM)
    u16 i;
    static u8 level = 0;
    static u8 Mode   = 0;
    static u8 MDdropF = 0;
    static u8 MDdropFCNT = 0;
    static u32 u32LastTick = 0;
    u8  mdset;


    if (gpioGetLevel(GPIO_GROUP_DAY_NIGHT, GPIO_PIN_DAY_NIGHT, &level) == 1)
    {
        level = (level == SIU_DAY_MODE) ? SIU_DAY_MODE : SIU_NIGHT_MODE;
    }

    if(u32LastTick == 0)
    {
        u32LastTick = OS_tickcounter;
    }
    else if(OS_tickcounter - u32LastTick < 40) //2 sec
    {
        return;
    }
 
    if(SIUMODE != level)
    {
        u32LastTick = OS_tickcounter;
        #if HW_MD_SUPPORT
        if (level == SIU_DAY_MODE)
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x01)== 0x1 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
        }
        else
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x02)== 0x2 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
            
        }
        mdset = MotionDetect_en;
        #endif
        if(mdset)
        {
        #if HW_MD_SUPPORT
           mduMotionDetect_ONOFF(0);
        #endif
        }

        if(level == SIU_NIGHT_MODE)
        {
            DEBUG_GPIO("!@##$Enter night mode %d\n",level);
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0); //ICR on/ off default hi, make a pulse and on-low/ off-hi
            OSTimeDly(1);
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)

            siuSetSensorDayNight(SIU_NIGHT_MODE); 
            SIUMODE = SIU_NIGHT_MODE;
             #if HW_MD_SUPPORT
                mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
            #endif
        }
        else
        {
            DEBUG_GPIO("Enter day mode %d\n",level); //白天使用紅外線濾光, 只讓可見光通過, 使影像清晰
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0); //on-hi, off-low
            OSTimeDly(1);
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)

            siuSetSensorDayNight(SIU_DAY_MODE);
            SIUMODE = SIU_DAY_MODE;
            #if HW_MD_SUPPORT
                mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
            #endif
        }
        if(mdset)
        {
        #if HW_MD_SUPPORT
           MDdropF = 1;
           MDdropFCNT = 0;
	    #endif
        }

    }
    else if(Mode == 0) //just pwr on
    {
        #if HW_MD_SUPPORT
           MDdropF = 1;
           MDdropFCNT = 0;
	    #endif
        Mode = 1; 
    }
    #if HW_MD_SUPPORT
    if (MDdropF == 1) // 夜間切換日間 誤觸 MD
    {
        MDdropFCNT ++;
        if (MDdropFCNT > 15)
        {
            mduMotionDetect_ONOFF(1);
            MDdropF = 0;
        }
    }
    #endif
#elif (HW_BOARD_OPTION == MR9160_TX_OPCOM_BATCAM)
    u16 i;
    u32 adcvalue;
    static u8 level = 0;
    static u8 Mode   = 0;
    static u8 MDdropF = 0;
    static u8 MDdropFCNT = 0;
    static u32 u32LastTick = 0;
    u8  mdset;

    adcvalue = adcGetValue(2);

    if(adcvalue > 0x960)  // 未fine tune
        level=SIU_NIGHT_MODE;
    else if(adcvalue < 0xAF0)
        level=SIU_DAY_MODE;
 
    if(SIUMODE != level)
    {
        #if HW_MD_SUPPORT
        if (level == SIU_DAY_MODE)
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x01)== 0x1 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
        }
        else
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x02)== 0x2 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
            
        }
        mdset = MotionDetect_en;
        #endif
        if(mdset)
        {
        #if HW_MD_SUPPORT
           mduMotionDetect_ONOFF(0);
        #endif
        }

        if(level == SIU_NIGHT_MODE)
        {
            DEBUG_GPIO("!@##$Enter night mode %d\n",level);
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0); //ICR on/ off default hi, make a pulse and on-low/ off-hi
            OSTimeDly(1);
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)

            siuSetSensorDayNight(SIU_NIGHT_MODE); 
            SIUMODE = SIU_NIGHT_MODE;
             #if HW_MD_SUPPORT
                mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
            #endif
        }
        else
        {
            DEBUG_GPIO("Enter day mode %d\n",level); //白天使用紅外線濾光, 只讓可見光通過, 使影像清晰
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0); //on-hi, off-low
            OSTimeDly(1);
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)

            siuSetSensorDayNight(SIU_DAY_MODE);
            SIUMODE = SIU_DAY_MODE;
            #if HW_MD_SUPPORT
                mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
            #endif
        }
        if(mdset)
        {
        #if HW_MD_SUPPORT
           MDdropF = 1;
           MDdropFCNT = 0;
	    #endif
        }

    }
    #if HW_MD_SUPPORT
    if (MDdropF == 1) // 夜間切換日間 誤觸 MD
    {
        MDdropFCNT ++;
        if (MDdropFCNT > 15)
        {
            mduMotionDetect_ONOFF(1);
            MDdropF = 0;
        }
    }
    #endif
#elif (HW_BOARD_OPTION == MR9160_TX_MAYON_MWL613)
    u16 i;
    u32 adcvalue;
    static u8 level = 0;
    static u8 Mode   = 0;
    static u8 MDdropF = 0;
    static u8 MDdropFCNT = 0;
    static u32 u32LastTick = 0;
    u8  mdset;
    u8 DCshift = 0;

    DCshift = adcGetValue(1) > 0x800 ? 0x30 : 0 ;

    if(u32LastTick == 0)
    {
        u32LastTick = OS_tickcounter;
        if(SIUMODE == SIU_NIGHT_MODE)
        {
            #if 0 //(IRLED_CONTROLL_BY_SENSOR == 1) //should apply when sensor init not here
                i2cWrite_SENSOR(0x0, 0x0);
                i2cWrite_SENSOR(0xE0, 0x9A); //driving
                i2cWrite_SENSOR(0x0, 0x6);
                i2cWrite_SENSOR(0x0F, 0x21); //IR LED on
            #endif
            gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)
        }
        else
        {
            #if 0 //(IRLED_CONTROLL_BY_SENSOR == 1)
                i2cWrite_SENSOR(0x0, 0x6);
                i2cWrite_SENSOR(0x0F, 0x1); //IR LED off
            #endif
            gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)
        }
        level = SIUMODE;
        return;
    }
    else if(OS_tickcounter - u32LastTick < 80) //4 sec
    {
        return;
    }
    else
    {
#if 1
        adcvalue = adcGetValueAverage(2, 6);
        if (adcvalue == 0)
            return;
        //adcvalue = adcGetValue(2);
        
      #if NEW_LIGHT_SENSOR
        adcvalue = (adcvalue < DCshift) ? 0 : (adcvalue - DCshift);
        if(adcvalue < 0x30)  // 未fine tune
            level=SIU_NIGHT_MODE;
        else if(adcvalue > 0x130)
            level=SIU_DAY_MODE;
        
      #else
        if(adcvalue < 0x50)  // 未fine tune
            level=SIU_NIGHT_MODE;
        else if(adcvalue > 0x180)
            level=SIU_DAY_MODE;
      #endif
#else
    adcvalue = adcGetValue(2);
    if(adcvalue > 0x960)  // 未fine tune
        level=SIU_NIGHT_MODE;
    else if(adcvalue < 0xAF0)
        level=SIU_DAY_MODE;
#endif
        u32LastTick = OS_tickcounter;
    }
    
    if(SIUMODE != level)
    {
        #if HW_MD_SUPPORT
        if (level == SIU_DAY_MODE)
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x01)== 0x1 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
        }
        else
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x02)== 0x2 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
            
        }
        mdset = MotionDetect_en;
        #endif
        if(mdset)
        {
        #if HW_MD_SUPPORT
           mduMotionDetect_ONOFF(0);
        #endif
        }

        if(level == SIU_NIGHT_MODE)
        {
            DEBUG_GPIO("!@##$Enter night mode %d\n",level);
            #if (IRLED_CONTROLL_BY_SENSOR == 1)
                i2cWrite_SENSOR(0x0, 0x0);
                i2cWrite_SENSOR(0xE0, 0xA); //driving
                i2cWrite_SENSOR(0x0, 0x6);
                i2cWrite_SENSOR(0x0F, 0x21); //IR LED on
            #endif
            
            #if 0
                i2cWrite_SENSOR(0x0, 0x0);
                i2cWrite_SENSOR(0xE0, 0xAA); //driving
                i2cWrite_SENSOR(0x0, 0x6);
                i2cWrite_SENSOR(0x0E, 0x51); //ICR on
                OSTimeDly(1);
                i2cWrite_SENSOR(0x0E, 0x20); //clear output
            #else
                gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0); //ICR on/ off default hi, make a pulse and on-low/ off-hi
              #if ICR_CONTRAL_ENABLE
                IcrOnFlag = 3; // make a pulse 200ms
              #else
                OSTimeDly(2);
                gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)                
              #endif
            #endif
            siuSetSensorDayNight(SIU_NIGHT_MODE); 
            SIUMODE = SIU_NIGHT_MODE;
            #if HW_MD_SUPPORT
                mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
            #endif
        }
        else
        {
            DEBUG_GPIO("Enter day mode %d\n",level); //白天使用紅外線濾光, 只讓可見光通過, 使影像清晰
            #if (IRLED_CONTROLL_BY_SENSOR == 1)
                i2cWrite_SENSOR(0x0, 0x6);
                i2cWrite_SENSOR(0x0F, 0x1); //IR LED off
            #endif
            
            #if 0
                i2cWrite_SENSOR(0x0, 0x0);
                i2cWrite_SENSOR(0xE0, 0xAA); //driving
                i2cWrite_SENSOR(0x0, 0x6);
                i2cWrite_SENSOR(0x0E, 0x49); //ICR off
                OSTimeDly(1);
                i2cWrite_SENSOR(0x0E, 0x20); //clear output
            #else
                gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0); //on-hi, off-low
              #if ICR_CONTRAL_ENABLE
                IcrOffFlag = 3; // make a pulse 200ms
              #else
                OSTimeDly(2);
                gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1); //on-hi, off-hi =>IR cut斷電 (BA6208)
              #endif
            #endif
            siuSetSensorDayNight(SIU_DAY_MODE);
            SIUMODE = SIU_DAY_MODE;
            #if HW_MD_SUPPORT
                mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
            #endif
        }
        if(mdset)
        {
        #if HW_MD_SUPPORT
           MDdropF = 1;
           MDdropFCNT = 0;
	    #endif
        }

    }
    #if HW_MD_SUPPORT
    if (MDdropF == 1) // 夜間切換日間 誤觸 MD
    {
        MDdropFCNT ++;
        if (MDdropFCNT > 15)
        {
            mduMotionDetect_ONOFF(1);
            MDdropF = 0;
        }
    }
    #endif
#elif (HW_BOARD_OPTION == MR9160_TX_ROULE_BATCAM)
    u16 i;
    u32 adcvalue;
    static u8 level = 0;
    static u8 Mode   = 0;
    static u8 MDdropF = 0;
    static u8 MDdropFCNT = 0;
    static u32 u32LastTick = 0;
    u8  mdset;

    if(u32LastTick == 0)
    {
        u32LastTick = OS_tickcounter;
        level = SIUMODE;
        return;
    }
    else if(OS_tickcounter - u32LastTick < 80) //4 sec
    {
        return;
    }
    else
    {
        adcvalue = adcGetValueAverage(2, 6);
        if (adcvalue == 0)
            return;
        //adcvalue = adcGetValue(2);
        if(adcvalue > 0x400)  // 未fine tune
            level=SIU_NIGHT_MODE;
        else if(adcvalue < 0x380)
            level=SIU_DAY_MODE;
        u32LastTick = OS_tickcounter;
    }
    
    if(SIUMODE != level)
    {
      #if HW_MD_SUPPORT
        if (level == SIU_DAY_MODE)
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x01)== 0x1 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
        }
        else
        {
            if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x02)== 0x2 )
                MotionDetect_en   = 1;
            else
                MotionDetect_en   = 0;
            
        }
        mdset = MotionDetect_en;
      #endif
        if(mdset)
        {
        #if HW_MD_SUPPORT
           mduMotionDetect_ONOFF(0);
        #endif
        }

        if(level == SIU_NIGHT_MODE)
        {
            DEBUG_GPIO("!@##$Enter night mode %d\n",level);
            gpioSetLevel(GPIO_GROUP_IR_POWER, GPIO_PIN_IR_POWER, GPIO_LEVEL_HI);
            siuSetSensorDayNight(SIU_NIGHT_MODE); 
            SIUMODE = SIU_NIGHT_MODE;
          #if HW_MD_SUPPORT
            mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
          #endif
        }
        else
        {
            DEBUG_GPIO("Enter day mode %d\n",level); //白天使用紅外線濾光, 只讓可見光通過, 使影像清晰
            gpioSetLevel(GPIO_GROUP_IR_POWER, GPIO_PIN_IR_POWER, GPIO_LEVEL_LO);
            siuSetSensorDayNight(SIU_DAY_MODE);
            SIUMODE = SIU_DAY_MODE;
          #if HW_MD_SUPPORT
            mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
          #endif
        }
        if(mdset)
        {
          #if HW_MD_SUPPORT
           MDdropF = 1;
           MDdropFCNT = 0;
          #endif
        }

    }
  #if HW_MD_SUPPORT
    if (MDdropF == 1) // 夜間切換日間 誤觸 MD
    {
        MDdropFCNT ++;
        if (MDdropFCNT > 15)
        {
            mduMotionDetect_ONOFF(1);
            MDdropF = 0;
        }
    }
  #endif


#else
    static u8  level = 0;
    static u8 Mode   = -1;

    level = SIU_DAY_MODE;
    if(Mode != level)
    {
        Mode=SIU_DAY_MODE;    
        SIUMODE = SIU_DAY_MODE;
    }
#endif

}

void ciuTask_CH1(void* pData)
{
    u8  err;
    u32 waitFlag;
#if CIU_PATTERN_TEST
    int TestCnt=0;
#endif
    


    /*show life time*/



	while (1)
	{
        waitFlag = OSFlagPend(ciuFlagGrp_CH1, CIU_EVET_FREAME_END|CIU_EVET_TOPFILD_END ,OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);

        if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: ciuFlagGrp_CH1 is 0x%x.\n", err);
			//return ;
		}
    #if CIU_PATTERN_TEST
        if( (TestCnt & 0x1f) == 0)
        {
           if( ((TestCnt >>5) & 0x03)== 0)
              CIU_1_CTL2 = (CIU_1_CTL2 & (~0x00300)) | CIU_TESTIMG_01;
           if( ((TestCnt >>5) & 0x03)== 1)
              CIU_1_CTL2 = (CIU_1_CTL2 & (~0x00300)) | CIU_TESTIMG_02;
           else if( ((TestCnt >>5) & 0x03)== 2)
              CIU_1_CTL2 = (CIU_1_CTL2 & (~0x00300)) | CIU_TESTIMG_03;
           else
              CIU_1_CTL2 = (CIU_1_CTL2 & (~0x00300)) | CIU_TESTIMG_03;
        }
        TestCnt ++;
    #endif
    
    #if HW_MD_SUPPORT
        if(waitFlag & CIU_EVET_FREAME_END)
        {
            if(MotionDetect_en)
            {
                if(ciu_1_OpMode == SIUMODE_PREVIEW)
                {
                    if( (ciu_idufrmcnt_ch1 & (MD_period_Preview-1)) == 0)
                        MotionDetect_API(MD_CIU1_ID);

                }
                else
                {
                    if( (ciu_idufrmcnt_ch1 & (MD_period_Video-1)) == 0)
                        MotionDetect_API(MD_CIU1_ID);
                }
            }
        }
    #endif

    #if(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU)
        if(waitFlag & CIU_EVET_FREAME_END)
        { 
           if(sysCameraMode == SYS_CAMERA_MODE_GFU_TESTSCR)
              gfuTest_ALL_Cmd();
        }
    #endif

    #if HW_DEINTERLACE_CIU1_ENA
        if(waitFlag & CIU_EVET_TOPFILD_END)
        {
            //DEBUG_CIU("Top intr\n");
            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)   
                diuRegConfig(MD_CIU1_ID, 640,240,640*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
                diuRegConfig(MD_CIU1_ID, 704,240,704*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)
                diuRegConfig(MD_CIU1_ID, 704,288,704*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480)
                diuRegConfig(MD_CIU1_ID, 720,240,720*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576)
                diuRegConfig(MD_CIU1_ID, 720,288,720*2);
            else
                diuRegConfig(MD_CIU1_ID, 640,240,640*2);
        }
    #endif

    #if 0 //(CIU1_OPTION == Sensor_OV7725_YUV601)   //Adjust Saturation
        {
            u16 Again_x16,temp,AECH,AECL;

            i2cRead_SENSOR(0x0, &temp);
            Again_x16= ((temp>>4) + 1)*(16+ (temp & 0x0f));
            i2cRead_SENSOR(0x08, &AECH);
            i2cRead_SENSOR(0x10, &AECL);
            
            if( (Again_x16 < 32) && (level == SIU_DAY_MODE) && (AECH==0) && (AECL<64) )//戶外強光: 1/250 sec
            {
                i2cWrite_SENSOR(0xa7,SENSOR_U_SATURATION_OUTDOOR);  
                i2cWrite_SENSOR(0xa8,SENSOR_V_SATURATION_OUTDOOR);   
            }
            else
            {
                i2cWrite_SENSOR(0xa7,SENSOR_U_SATURATION_INDOOR);  
                i2cWrite_SENSOR(0xa8,SENSOR_V_SATURATION_INDOOR);   
            }
        }
    #endif

    #if ((HW_BOARD_OPTION == MR9120_TX_TRANWO)||(HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T)||(HW_BOARD_OPTION == MR9120_TX_TRANWO_D87T)||(HW_BOARD_OPTION == MR9100_TX_JIT_C707HW4))
        gpioDetectIRLED();
    #endif

	}
}


void ciuTask_CH2(void* pData)
{
	u8 err;
    u32 waitFlag;

#if CIU_PATTERN_TEST
    int TestCnt=0;
#endif


	while (1)
	{
		waitFlag = OSFlagPend(ciuFlagGrp_CH2, CIU_EVET_FREAME_END|CIU_EVET_TOPFILD_END ,OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: ciuFlagGrp_CH2 is %d.\n", err);
			//return ;
		}
    #if CIU_PATTERN_TEST
        if( (TestCnt & 0x1f) == 0)
        {
           if( ((TestCnt >>5) & 0x03)== 0)
              CIU_2_CTL2 = (CIU_2_CTL2 & (~0x00300)) | CIU_TESTIMG_01;
           if( ((TestCnt >>5) & 0x03)== 1)
              CIU_2_CTL2 = (CIU_2_CTL2 & (~0x00300)) | CIU_TESTIMG_02;
           else if( ((TestCnt >>5) & 0x03)== 2)
              CIU_2_CTL2 = (CIU_2_CTL2 & (~0x00300)) | CIU_TESTIMG_03;
           else
              CIU_2_CTL2 = (CIU_2_CTL2 & (~0x00300)) | CIU_TESTIMG_03;
        }
        TestCnt ++;
    #endif
    #if HW_MD_SUPPORT
        if(waitFlag & CIU_EVET_FREAME_END)
        {
            if(MotionDetect_en)
            {
                if(ciu_2_OpMode == SIUMODE_PREVIEW)
                {
                    if( (ciu_idufrmcnt_ch2 & (MD_period_Preview-1)) == 0)
                        MotionDetect_API(MD_CIU2_ID);
                }
                else
                {
                    if( (ciu_idufrmcnt_ch2 & (MD_period_Video-1)) == 0)
                        MotionDetect_API(MD_CIU2_ID);
                }
            }
        }
    #endif
    #if HW_DEINTERLACE_CIU2_ENA
        if(waitFlag & CIU_EVET_TOPFILD_END)
        {
            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)   
                diuRegConfig(MD_CIU2_ID, 640,240,640*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
                diuRegConfig(MD_CIU2_ID, 704,240,704*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)
                diuRegConfig(MD_CIU2_ID, 704,288,704*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480)
                diuRegConfig(MD_CIU2_ID, 720,240,720*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576)
                diuRegConfig(MD_CIU2_ID, 720,288,720*2);
            else
                diuRegConfig(MD_CIU2_ID, 640,240,640*2);
        }
    #endif

    ciuTask_DayNight_ICR_ctrl();
    
	}
}

void ciuTask_CH3(void* pData)
{
	u8 err;
    u32 waitFlag;
#if CIU_PATTERN_TEST
    int TestCnt=0;
#endif
	while (1)
	{
	    waitFlag = OSFlagPend(ciuFlagGrp_CH3, CIU_EVET_FREAME_END|CIU_EVET_TOPFILD_END ,OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: ciuFlagGrp_CH3 is %d.\n", err);
			//return ;
		}
    #if CIU_PATTERN_TEST
        if( (TestCnt & 0x1f) == 0)
        {
           if( ((TestCnt >>5) & 0x03)== 0)
              CIU_3_CTL2 = (CIU_3_CTL2 & (~0x00300)) | CIU_TESTIMG_01;
           if( ((TestCnt >>5) & 0x03)== 1)
              CIU_3_CTL2 = (CIU_3_CTL2 & (~0x00300)) | CIU_TESTIMG_02;
           else if( ((TestCnt >>5) & 0x03)== 2)
              CIU_3_CTL2 = (CIU_3_CTL2 & (~0x00300)) | CIU_TESTIMG_03;
           else
              CIU_3_CTL2 = (CIU_3_CTL2 & (~0x00300)) | CIU_TESTIMG_03;
        }
        TestCnt ++;
    #endif

    #if HW_MD_SUPPORT
        if(waitFlag & CIU_EVET_FREAME_END)
        {
            if(MotionDetect_en)
            {
                if(ciu_3_OpMode == SIUMODE_PREVIEW)
                {
                    if( (ciu_idufrmcnt_ch3 & (MD_period_Preview-1)) == 0)
                        MotionDetect_API(MD_CIU3_ID);
                }
                else
                {
                    if( (ciu_idufrmcnt_ch3 & (MD_period_Video-1)) == 0)
                        MotionDetect_API(MD_CIU3_ID);
                }
            }
        }
    #endif

    #if HW_DEINTERLACE_CIU3_ENA
        if(waitFlag & CIU_EVET_TOPFILD_END)
        {
            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)   
                diuRegConfig(MD_CIU3_ID, 640,240,640*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
                diuRegConfig(MD_CIU3_ID, 704,240,704*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)
                diuRegConfig(MD_CIU3_ID, 704,288,704*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480)
                diuRegConfig(MD_CIU3_ID, 720,240,720*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576)
                diuRegConfig(MD_CIU3_ID, 720,288,720*2);
            else
                diuRegConfig(MD_CIU3_ID, 640,240,640*2);        
         }
    #endif


	}
}


void ciuTask_CH4(void* pData)
{
	u8 err;
    u32 waitFlag;
#if CIU_PATTERN_TEST
    int TestCnt=0;
#endif

	while (1)
	{
	    waitFlag = OSFlagPend(ciuFlagGrp_CH4, CIU_EVET_FREAME_END|CIU_EVET_TOPFILD_END ,OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: ciuFlagGrp_CH4 is %d.\n", err);
			//return ;
		}
    #if CIU_PATTERN_TEST
        if( (TestCnt & 0x1f) == 0)
        {
           if( ((TestCnt >>5) & 0x03)== 0)
              CIU_4_CTL2 = (CIU_4_CTL2 & (~0x00300)) | CIU_TESTIMG_01;
           if( ((TestCnt >>5) & 0x03)== 1)
              CIU_4_CTL2 = (CIU_4_CTL2 & (~0x00300)) | CIU_TESTIMG_02;
           else if( ((TestCnt >>5) & 0x03)== 2)
              CIU_4_CTL2 = (CIU_4_CTL2 & (~0x00300)) | CIU_TESTIMG_03;
           else
              CIU_4_CTL2 = (CIU_4_CTL2 & (~0x00300)) | CIU_TESTIMG_03;
        }
        TestCnt ++;
    #endif
    #if HW_MD_SUPPORT
         if(waitFlag & CIU_EVET_FREAME_END)
         {
            if(MotionDetect_en)
            {
                if(ciu_4_OpMode == SIUMODE_PREVIEW)
                {
                    if( (ciu_idufrmcnt_ch4 & (MD_period_Preview-1)) == 0)
                        MotionDetect_API(MD_CIU4_ID);
                }
                else
                {
                    if( (ciu_idufrmcnt_ch4 & (MD_period_Video-1)) == 0)
                        MotionDetect_API(MD_CIU4_ID);
                }
            }
        }
    #endif

    #if HW_DEINTERLACE_CIU4_ENA
        if(waitFlag & CIU_EVET_TOPFILD_END)
        {
            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)   
                diuRegConfig(MD_CIU4_ID, 640,240,640*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
                diuRegConfig(MD_CIU4_ID, 704,240,704*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)
                diuRegConfig(MD_CIU4_ID, 704,288,704*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480)
                diuRegConfig(MD_CIU4_ID, 720,240,720*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576)
                diuRegConfig(MD_CIU4_ID, 720,288,720*2);
            else
                diuRegConfig(MD_CIU4_ID, 640,240,640*2);                
        }
    #endif



	}
}

void ciuTask_CH5(void* pData)
{
    u8  err;
    u32 waitFlag;
    u32 RunCnt=0;
#if CIU_PATTERN_TEST
    int TestCnt=0;
#endif
    //---------------------//
	while (1)
	{
        waitFlag = OSFlagPend(ciuFlagGrp_CH5, CIU_EVET_FREAME_END|CIU_EVET_TOPFILD_END ,OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        RunCnt++;
        if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: ciuFlagGrp_CH5 is 0x%x.\n", err);
			//return ;
		}
    #if CIU_PATTERN_TEST
        if( (TestCnt & 0x1f) == 0)
        {
           if( ((TestCnt >>5) & 0x03)== 0)
              CIU_5_CTL2 = (CIU_5_CTL2 & (~0x00300)) | CIU_TESTIMG_01;
           if( ((TestCnt >>5) & 0x03)== 1)
              CIU_5_CTL2 = (CIU_5_CTL2 & (~0x00300)) | CIU_TESTIMG_02;
           else if( ((TestCnt >>5) & 0x03)== 2)
              CIU_5_CTL2 = (CIU_5_CTL2 & (~0x00300)) | CIU_TESTIMG_03;
           else
              CIU_5_CTL2 = (CIU_5_CTL2 & (~0x00300)) | CIU_TESTIMG_03;
        }
        TestCnt ++;
    #endif
    #if 0//(TV_DECODER == AHD6124B)
        for(i=0;i<nvp6124_cnt*4;i++)
		{
			if(chip_id[i/4] == 0x86)
				nvp6124b_set_equalizer(i);
//			else
//				nvp6124_set_equalizer(i);
		}
    #endif
    #if HW_MD_SUPPORT

        if(waitFlag & CIU_EVET_FREAME_END)
        {
            if(MotionDetect_en)
            {
                if(ciu_5_OpMode == SIUMODE_PREVIEW)
                {
                    if( (RunCnt > 2) && ((RunCnt & (MD_period_Preview-1)) == 0))
                        MotionDetect_API(MD_CIU5_ID);

                }
                else if((RunCnt > 2) && uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072){ //FULL HD frame rate is 10 only, use all frames to do motion detect
                    MotionDetect_API(MD_CIU5_ID);
                }                
                else
                {
                    if( (RunCnt > 2) && (RunCnt & (MD_period_Video-1)) == 0)
                        MotionDetect_API(MD_CIU5_ID);
                }
            #if TX_PIRREC_VMDCHK
                if(MD_Diff > 0)
                {
                   if(rfiuPIRRec_VMDTrig==0)
                      DEBUG_CIU("MD_Diff=%d,%d,%d\n",MD_Diff,RunCnt,rfiuVideoInFrameRate);
                   rfiuPIRRec_VMDTrig=1;
                   //gpioSetLevel(0, 31, 1);
                }
            #endif
            }
        }
    #endif

    #if(SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU)
        if(waitFlag & CIU_EVET_FREAME_END)
        {    
           gfuTest_ALL_Cmd();
        }
    #endif

    #if HW_DEINTERLACE_CIU1_ENA
        if(waitFlag & CIU_EVET_TOPFILD_END)
        {
            //DEBUG_CIU("Top intr\n");
            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)   
                diuRegConfig(MD_CIU5_ID, 640,240,640*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
                diuRegConfig(MD_CIU5_ID, 704,240,704*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x576)
                diuRegConfig(MD_CIU5_ID, 704,288,704*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480)
                diuRegConfig(MD_CIU5_ID, 720,240,720*2);
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x576)
                diuRegConfig(MD_CIU5_ID, 720,288,720*2);
            else
                diuRegConfig(MD_CIU5_ID, 640,240,640*2);
        }
    #endif

    #if 0//(CIU5_OPTION == Sensor_OV7725_YUV601)   //Adjust Saturation
        {
            u16 Again_x16,temp,AECH,AECL;

            i2cRead_SENSOR(0x0, &temp);
            Again_x16= ((temp>>4) + 1)*(16+ (temp & 0x0f));
            i2cRead_SENSOR(0x08, &AECH);
            i2cRead_SENSOR(0x10, &AECL);
            
            if( (Again_x16 < 32) && (level == SIU_DAY_MODE) && (AECH==0) && (AECL<64) )//戶外強光: 1/250 sec
            {
                i2cWrite_SENSOR(0xa7,SENSOR_U_SATURATION_OUTDOOR);  
                i2cWrite_SENSOR(0xa8,SENSOR_V_SATURATION_OUTDOOR);   
            }
            else
            {
                i2cWrite_SENSOR(0xa7,SENSOR_U_SATURATION_INDOOR);  
                i2cWrite_SENSOR(0xa8,SENSOR_V_SATURATION_INDOOR);   
            }
        }
    #endif

        ciuTask_DayNight_ICR_ctrl();
    #if ((HW_BOARD_OPTION == MR9120_TX_TRANWO)||(HW_BOARD_OPTION == MR9100_TX_TRANWO_D87T)||(HW_BOARD_OPTION == MR9120_TX_TRANWO_D87T)||(HW_BOARD_OPTION == MR9100_TX_JIT_C707HW4))
        gpioDetectIRLED();
    #endif
	}
}

/*

Routine Description:

	The FIQ handler of Sensor Interface Unit.

Arguments:

	None.

Return Value:

	None.

*/
void ciuIntHandler_CH1(void)
{
    u32         intStat;
    u8          err;
#if( DEBIG_CIU_FRAM_END_INTR_USE_LED6 && (FPGA_BOARD_A1018_SERIES) )
    static u32  Debug_count=0;
#endif

#if CIU_DEBUG_INTR
    static u32  Debug_count=0;
#endif

#if FINE_TIME_STAMP
    s32         IISTimeOffsetCIU, TimeOffset;
    u32         IISTime1;
#endif

    u8          **PNBuf_sub;
#if (CIU1_BOB_REPLACE_MPEG_DF && (CHIP_OPTION == CHIP_A1016A) )
    static u8   ClearBob    = 0;
    static u8   EnterBob    = 0;
#endif
    static u32  ciu1_ch_status = 100;
#if MULTI_CHANNEL_VIDEO_REC
    VIDEO_CLIP_OPTION   *pVideoClipOption   = &VideoClipOption[1];
#endif

    intStat = CIU_1_INTRPT;

    //DEBUG_CIU("C");
#if CIU_DEBUG_INTR
    gpioSetLevel(1, 17, Debug_count & 0x01);
    Debug_count ++;
#endif
    ciu_1_FPS_Count++;
    if(intStat & CIU_INT_STAT_FIFO_OVERF)
    {
        DEBUG_CIU("CIU1 FIFO overflow\n");
    }
    if(intStat & CIU_INT_STAT_SP_OVERF)
    {
        DEBUG_CIU("CIU1 SP FIFO overflow\n");
    }

    if(intStat & CIU_INT_STAT_FRAME_END)
    {
        if(ciu_1_OpMode == SIUMODE_PREVIEW_MENU)
            return;

   #if DUAL_MODE_DISP_SUPPORT
        if(sysDualModeDisp && (ciu_1_OpMode != SIUMODE_CAP_RREVIEW))
            if((ciu_idufrmcnt_ch1 & 3) == (ciu_idufrmcnt_ch2 & 3))
                return;
   #endif

   //==========================SW De-interlace=======================//
   #if (MPEG4_FIELD_ENC_ENA && (CIU1_OPTION == Sensor_CCIR656) )

   #elif (CIU1_BOB_REPLACE_MPEG_DF && (CHIP_OPTION == CHIP_A1016A) ) //Lucian: 不用 Mpeg4 double field,起動 ciu mob mode
        // A1016 CIU_MODE_656_BOB 只能在bottom field end時寫入,不然畫面會閃.
        //if((CIU_1_CTL1 & CIU_FIELD_POL_NEG) == 0)  // field上下沒有交換時
		if(ciu_1_OpMode != SIUMODE_CAP_RREVIEW)
        {
         #if MULTI_CHANNEL_VIDEO_REC
           #if( CIU1_OPTION == Sensor_CCIR656 )
            if((CIU_1_CTL1 & CIU_FIELD_POL_NEG) == 0)
            {
                if(pVideoClipOption->video_double_field_flag)
                {
                    CIU_1_CTL1   = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                    CIU_1_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
                } else {
                    CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656;
                }
            } else {    // 若field上下交換,進BOB要掉3張,離開BOB要掉2張
                if(ClearBob)
                {
                    if(ClearBob == 1)
                        CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656;
                    ClearBob--;
                    return;
                }
                if(EnterBob)
                {
                    if(EnterBob == 2)
                    {
                        CIU_1_CTL1   = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                        CIU_1_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
                    }
                    EnterBob--;
                    if(EnterBob == 0)
                        return;
                }
                if(pVideoClipOption->video_double_field_flag && (EnterBob == 0) && (ClearBob == 0))
                {
                    if((CIU_1_CTL1 & CIU_MODE_656_BOB) != CIU_MODE_656_BOB)
                    {
                        EnterBob    = 2;
                        //return;
                    }
                }
                else if((pVideoClipOption->video_double_field_flag == 0) && (EnterBob == 0) && (ClearBob == 0))
                {
                    if((CIU_1_CTL1 & CIU_MODE_656_BOB) == CIU_MODE_656_BOB)
                    {
                        ClearBob    = 1;
                        return;
                    }
                }
            }
           #elif(CIU1_OPTION == Sensor_CCIR601)
            if((CIU_1_CTL1 & CIU_FIELD_POL_NEG) == 0)
            {
                if(pVideoClipOption->video_double_field_flag)
                {
                    CIU_1_CTL1   = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                    CIU_1_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
                } else {
                    CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_601;
                }
            } else {    // 若field上下交換,進BOB要掉3張,離開BOB要掉2張
                if(ClearBob)
                {
                    if(ClearBob == 1)
                        CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_601;
                    ClearBob--;
                    return;
                }
                if(EnterBob)
                {
                    if(EnterBob == 2)
                    {
                        CIU_1_CTL1   = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                        CIU_1_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
                    }
                    EnterBob--;
                    if(EnterBob == 0)
                        return;
                }
                if(pVideoClipOption->video_double_field_flag && (EnterBob == 0) && (ClearBob == 0))
                {
                    if((CIU_1_CTL1 & CIU_MODE_656_BOB) != CIU_MODE_656_BOB)
                    {
                        EnterBob    = 2;
                        //return;
                    }
                }
                else if((pVideoClipOption->video_double_field_flag == 0) && (EnterBob == 0) && (ClearBob == 0))
                {
                    if((CIU_1_CTL1 & CIU_MODE_656_BOB) == CIU_MODE_656_BOB)
                    {
                        ClearBob    = 1;
                        return;
                    }
                }
            }
           #endif
         #else
           #if( CIU1_OPTION == Sensor_CCIR656 )
            if((CIU_1_CTL1 & CIU_FIELD_POL_NEG) == 0)
            {
                if(video_double_field_flag)
                {
                    CIU_1_CTL1   = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                    CIU_1_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
                } else {
                    CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656;
                }
            } else {    // 若field上下交換,進BOB要掉3張,離開BOB要掉2張
                if(ClearBob)
                {
                    if(ClearBob == 1)
                        CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656;
                    ClearBob--;
                    return;
                }
                if(EnterBob)
                {
                    if(EnterBob == 2)
                    {
                        CIU_1_CTL1   = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                        CIU_1_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
                    }
                    EnterBob--;
                    if(EnterBob == 0)
                        return;
                }
                if(video_double_field_flag && (EnterBob == 0) && (ClearBob == 0))
                {
                    if((CIU_1_CTL1 & CIU_MODE_656_BOB) != CIU_MODE_656_BOB)
                    {
                        EnterBob    = 2;
                        //return;
                    }
                }
                else if((video_double_field_flag == 0) && (EnterBob == 0) && (ClearBob == 0))
                {
                    if((CIU_1_CTL1 & CIU_MODE_656_BOB) == CIU_MODE_656_BOB)
                    {
                        ClearBob    = 1;
                        return;
                    }
                }
            }
           #elif(CIU1_OPTION == Sensor_CCIR601)
            if((CIU_1_CTL1 & CIU_FIELD_POL_NEG) == 0)
            {
                if(video_double_field_flag)
                {
                    CIU_1_CTL1   = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                    CIU_1_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
                } else {
                    CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_601;
                }
            } else {    // 若field上下交換,進BOB要掉3張,離開BOB要掉2張
                if(ClearBob)
                {
                    if(ClearBob == 1)
                        CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_601;
                    ClearBob--;
                    return;
                }
                if(EnterBob)
                {
                    if(EnterBob == 2)
                    {
                        CIU_1_CTL1   = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                        CIU_1_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
                    }
                    EnterBob--;
                    if(EnterBob == 0)
                        return;
                }
                if(video_double_field_flag && (EnterBob == 0) && (ClearBob == 0))
                {
                    if((CIU_1_CTL1 & CIU_MODE_656_BOB) != CIU_MODE_656_BOB)
                    {
                        EnterBob    = 2;
                        //return;
                    }
                }
                else if((video_double_field_flag == 0) && (EnterBob == 0) && (ClearBob == 0))
                {
                    if((CIU_1_CTL1 & CIU_MODE_656_BOB) == CIU_MODE_656_BOB)
                    {
                        ClearBob    = 1;
                        return;
                    }
                }
            }
           #endif
         #endif     // #if MULTI_CHANNEL_VIDEO_REC
            //DEBUG_CIU("1\n");
        }
   #endif       // #if (MPEG4_FIELD_ENC_ENA && (CIU1_OPTION == Sensor_CCIR656) )

   //==========================HW Motion Detection/HW Deinterlace(A1018A)=======================//
   #if (HW_MD_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
    if(ciu1_ch_status != ciu_idufrmcnt_ch1)
    {
        OSFlagPost(ciuFlagGrp_CH1, CIU_EVET_FREAME_END, OS_FLAG_SET, &err);
        ciu1_ch_status = ciu_idufrmcnt_ch1;
    }
   #endif

   //==========================SW PIP=======================//
        switch(sysPIPMain)
        {
        case    PIP_MAIN_CH1:
            PNBuf_sub   = (u8**)PNBuf_sub2;
            break;
        default:
            PNBuf_sub   = (u8**)PNBuf_sub1;
            break;
        }

        if((sysPIPMain == PIP_MAIN_NONE) || (sysPIPMain == PIP_MAIN_CH2))
        {
            if(sysTVOutOnFlag)
            {
            }
            else
            {
                if(sysVideoInCHsel == 0x01)
                {
                #if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
                   if( (sysCameraMode == SYS_CAMERA_MODE_CIU_QUADSCR) || (sysCameraMode == SYS_CAMERA_MODE_GFU_TESTSCR) )
                   {
                      IduWinCtrl = (IduWinCtrl & ~0x00003000);
                      IduVidBuf0Addr=(u32)PNBuf_Quad;
                   #if NEW_IDU_BRI
                      BRI_IADDR_Y = IduVidBuf0Addr;
                      BRI_IADDR_C = BRI_IADDR_Y + ciu_1_pnbuf_size_y;
                   #endif
                   }
                   else
                #endif
                   {
                        IduWinCtrl = (IduWinCtrl & ~0x00003000); //| ((ciu_idufrmcnt_ch1 % 3) << 12);
                    #if IS_COMMAX_DOORPHONE  // 顯示拍的照片
                        //if(ciu_1_OpMode != SIUMODE_CAP_RREVIEW) //拍照時要停住以便顯示拍到的畫面一秒鐘
                    #endif

                    #if HW_DEINTERLACE_CIU1_ENA
                        IduVidBuf0Addr  = ((u32)PNBuf_sub[ (ciu_idufrmcnt_ch1-1) & 0x03]);
                    #else
                        IduVidBuf0Addr  = ((u32)PNBuf_sub[ ciu_idufrmcnt_ch1 & 0x03]);
                    #endif

                    #if NEW_IDU_BRI
                        BRI_IADDR_Y = IduVidBuf0Addr;
                        BRI_IADDR_C = BRI_IADDR_Y + ciu_1_pnbuf_size_y;
                      #if DUAL_MODE_DISP_SUPPORT
                        if(sysDualModeDisp)
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_1_OutWidth * 2);
                        else
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_1_OutWidth);
                      #endif
                    #endif
                   }
                }
            }
        }

        if(ciu_1_OpMode == SIUMODE_MPEGAVI)
        {
        #if MULTI_CHANNEL_VIDEO_REC
            if((pVideoClipOption->VideoPictureIndex + 2) >= ciu_idufrmcnt_ch1)
            {
                if((sysPIPMain == PIP_MAIN_NONE) ||
                    ((sysPIPMain == PIP_MAIN_CH2) && ((ciu_idufrmcnt_ch1 + 1) < ciu_idufrmcnt_ch2)) ||
                    ((sysPIPMain == PIP_MAIN_CH1) && (ciu_idufrmcnt_ch1 < (ciu_idufrmcnt_ch2 + 2))))
                {
                    if(pVideoClipOption->sysReady2CaptureVideo)
                    {
                       #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                        timerCountRead(2, (u32*) &IISTimeOffsetCIU);
                        IISTimeOffsetCIU    = IISTimeOffsetCIU >> 8;
                        if(pVideoClipOption->IISTimeOffset >= IISTimeOffsetCIU) {
                            TimeOffset  = pVideoClipOption->IISTimeOffset - IISTimeOffsetCIU;
                        } else {
                            TimeOffset  = pVideoClipOption->IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetCIU;
                        }
                        if(TimeOffset > pVideoClipOption->IISTimeUnit) {
                            TimeOffset  = pVideoClipOption->IISTimeUnit;
                        }
                        IISTime1    = pVideoClipOption->IISTime + TimeOffset;
                        if(IISTime1 > ciu_1_FrameTime) {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch1 % ISU_FRAME_DURATION_NUM]  = IISTime1 - ciu_1_FrameTime;
                            ciu_1_FrameTime                                                                 = IISTime1;
                        }
            		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
            			timerCountRead(1, (u32*) &IISTimeOffsetCIU);
                        IISTimeOffsetCIU    = IISTimeOffsetCIU / 100;
            			if(pVideoClipOption->IISTimeOffset >= IISTimeOffsetCIU) {
            				TimeOffset  = pVideoClipOption->IISTimeOffset - IISTimeOffsetCIU;
            			} else {
            			    TimeOffset  = pVideoClipOption->IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
            			}
            			if(TimeOffset > pVideoClipOption->IISTimeUnit) {
            				TimeOffset  = pVideoClipOption->IISTimeUnit;
            			}
            			IISTime1    = pVideoClipOption->IISTime + TimeOffset;
            			if(IISTime1 > ciu_1_FrameTime) {
            				pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch1 % ISU_FRAME_DURATION_NUM]  = IISTime1 - ciu_1_FrameTime;
                            ciu_1_FrameTime                                                                 = IISTime1;
                        }
                      #else   // only use IIS time to calculate frame time
                        if(pVideoClipOption->IISTime > ciu_1_FrameTime) {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch1 % ISU_FRAME_DURATION_NUM]  = pVideoClipOption->IISTime - ciu_1_FrameTime;
                            ciu_1_FrameTime                                                                 = pVideoClipOption->IISTime;
                        }
                      #endif
                        else
                        {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch1 % ISU_FRAME_DURATION_NUM]  = 1;
                            ciu_1_FrameTime++;
                        }
                    }

                    OSSemPost(ciuCapSemEvt_CH1);
                    ciu_idufrmcnt_ch1 ++;
                    //CIU_1_CTL1  |= CIU_DATA_OUT_ENA;
                }
            }
            else     // frame skip
            {
               //CIU_1_CTL1  &= ~CIU_DATA_OUT_ENA;
            }
        #else
            if((VideoPictureIndex + 2) >= ciu_idufrmcnt_ch1)
            {
                if(sysReady2CaptureVideo)
                {
                   #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                    timerCountRead(2, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU >> 8;
                    if(IISTimeOffset >= IISTimeOffsetCIU) {
                        TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
                    } else {
                        TimeOffset  = IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetCIU;
                    }
                    if(TimeOffset > IISTimeUnit) {
                        TimeOffset  = IISTimeUnit;
                    }
                    IISTime1    = IISTime + TimeOffset;
                    if(IISTime1 > ciu_1_FrameTime) {
                        ISUFrameDuration[ciu_idufrmcnt_ch1 % ISU_FRAME_DURATION_NUM]    = IISTime1 - ciu_1_FrameTime;
                        ciu_1_FrameTime                                                = IISTime1;
                    }
        		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
        			timerCountRead(1, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU / 100;
        			if(IISTimeOffset >= IISTimeOffsetCIU) {
        				TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
        			} else {
        			    TimeOffset  = IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
        			}
        			if(TimeOffset > IISTimeUnit) {
        				TimeOffset  = IISTimeUnit;
        			}
        			IISTime1    = IISTime + TimeOffset;
        			if(IISTime1 > ciu_1_FrameTime) {
        				ISUFrameDuration[ciu_idufrmcnt_ch1 % ISU_FRAME_DURATION_NUM]    = IISTime1 - ciu_1_FrameTime;
                        //DEBUG_CIU("CIU(%d,%d,%d,%d,%d,%d,%d)\n", ISUFrameDuration[ciu_idufrmcnt_ch1 % ISU_FRAME_DURATION_NUM], ciu_1_FrameTime, IISTime1, (u32)IISTime, TimeOffset, IISTimeOffset, IISTimeOffsetCIU);
                        ciu_1_FrameTime                                                 = IISTime1;
                    }
                  #else   // only use IIS time to calculate frame time
                    if(IISTime > ciu_1_FrameTime) {
                        ISUFrameDuration[ciu_idufrmcnt_ch1 % ISU_FRAME_DURATION_NUM]    = IISTime - ciu_1_FrameTime;
                        ciu_1_FrameTime                                                 = IISTime;
                    }
                  #endif
                    else
                    {
                        ISUFrameDuration[ciu_idufrmcnt_ch1 % ISU_FRAME_DURATION_NUM]    = 1;
                        ciu_1_FrameTime++;
                    }
                }

                OSSemPost(ciuCapSemEvt_CH1);
                ciu_idufrmcnt_ch1 ++;
                //CIU_1_CTL1  |= CIU_DATA_OUT_ENA;
            }
            else     // frame skip
            {
                //CIU_1_CTL1  &= ~CIU_DATA_OUT_ENA;
            }
        #endif  // #if MULTI_CHANNEL_VIDEO_REC
       }
       else // ciu_1_OpMode != SIUMODE_MPEGAVI
       {
            if(sysPIPMain == PIP_MAIN_NONE)
            {
                ciu_idufrmcnt_ch1 ++;
            }
            else if(sysPIPMain == PIP_MAIN_CH2)
            {
                if((ciu_idufrmcnt_ch1 + 1) < ciu_idufrmcnt_ch2)
                {
                    ciu_idufrmcnt_ch1 ++;
                    //CiuSetIduAdr    = 1;
                }
            }
            else if(sysPIPMain == PIP_MAIN_CH1)
            {
                u32 temp = (u32)PNBuf_sub[(ciu_idufrmcnt_ch1 + 2) & 3];
                if((ciu_idufrmcnt_ch1 < (ciu_idufrmcnt_ch2 + 2)) && (temp != (u32)IduVidBuf0Addr))
                    ciu_idufrmcnt_ch1 ++;
            }
       }

#if(NEW_CIU1_EN)

    #if ISUCIU_PREVIEW_PNOUT
        #if (MULTI_STREAM_SUPPORT)
            #if SP_TEST
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + (ciu_1_OutY +60)* ciu_1_line_stride + ciu_1_OutX);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + (ciu_1_OutY +60)* ciu_1_line_stride / 2 + ciu_1_OutX);
            #else
                #if SWAP_MULTI_STREAM_SUPPORT
                    if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                    {
                        CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
                        CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
                        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
                    }
                    else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
                    {
                        #if HD_SWAP_MPSP_EN
                        CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
                        CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
                        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
                        #else
                        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
                        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
                        CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
                        #endif
                    }
                    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
                    {
                        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
                        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
                        CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
                    }
                #else
                    CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
                    CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
                    CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX     + VIDEODISPBUF_OFFSET);
                    CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
                #endif
            #endif
        #else
                #if (CIU_SP_REP_MP)
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
                #else
                CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
                CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
                #endif
                #if CIU_DECIMATION_TEST
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
                #endif            
        #endif
    #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + (ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + PNBUF_SIZE_Y);
    #endif


#else
      #if ISUCIU_PREVIEW_PNOUT
        #if SWAP_MULTI_STREAM_SUPPORT
            CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
            CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX     + VIDEODISPBUF_OFFSET);
            CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
         #else
            CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
            #if CIU_DECIMATION_TEST
            CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
            CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
            #endif
            #if MULTI_STREAM_SUPPORT
              #if SP_TEST
              CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + (ciu_1_OutY +60)* ciu_1_line_stride + ciu_1_OutX);
              CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + (ciu_1_OutY +60)* ciu_1_line_stride / 2 + ciu_1_OutX);
              #else
              CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX     + VIDEODISPBUF_OFFSET);
              CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
              #endif
            #endif
        #endif
      #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + (ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y);
      #endif
#endif
      
      //--------------------------------------------------------------------------------------------------------------------//
    
    }

    if(intStat & CIU_INT_STAT_FIELD_END)
    {
    #if HW_DEINTERLACE_CIU1_ENA
        OSFlagPost(ciuFlagGrp_CH1, CIU_EVET_TOPFILD_END, OS_FLAG_SET, &err);
    #endif
    }

}

void ciuIntHandler_CH2(void)
{
    u32 intStat;
#if CIU_DEBUG_INTR
    static u32  Debug_count=0;
#endif

#if FINE_TIME_STAMP
    s32 IISTimeOffsetCIU, TimeOffset;
    u32 IISTime1;
#endif

    u8  **PNBuf_sub;
    u8          err;
    static u32  ciu2_ch_status = 100;

#if MULTI_CHANNEL_VIDEO_REC
    VIDEO_CLIP_OPTION   *pVideoClipOption   = &VideoClipOption[2];
#endif
    //--------------------------------------------------------------//
    intStat = CIU_2_INTRPT;

#if CIU_DEBUG_INTR
    gpioSetLevel(1, 18, Debug_count & 0x01);
    Debug_count ++;
#endif
    ciu_2_FPS_Count++;
    if(intStat & CIU_INT_STAT_FIFO_OVERF)
    {
        DEBUG_CIU("CIU2 FIFO overflow\n");
    }
    if(intStat & CIU_INT_STAT_SP_OVERF)
    {
        DEBUG_CIU("CIU2 SP FIFO overflow\n");
    }

    if(intStat & CIU_INT_STAT_FRAME_END)
    {
        if(ciu_2_OpMode == SIUMODE_PREVIEW_MENU)
            return;

    #if DUAL_MODE_DISP_SUPPORT
        if(sysDualModeDisp && (ciu_2_OpMode != SIUMODE_CAP_RREVIEW))
            if(((ciu_idufrmcnt_ch2 + 2) & 3) == (ciu_idufrmcnt_ch1 & 3))
                return;
    #endif


   //==========================HW Motion Detection/HW Deinterlace(A1018A)=======================//
   #if (HW_MD_SUPPORT)
        if(ciu2_ch_status != ciu_idufrmcnt_ch2)
        {
            OSFlagPost(ciuFlagGrp_CH2, CIU_EVET_FREAME_END, OS_FLAG_SET, &err);
            ciu2_ch_status = ciu_idufrmcnt_ch2;
        }
   #endif

        switch(sysPIPMain)
        {
        case    PIP_MAIN_CH2:
            PNBuf_sub   = (u8**)PNBuf_sub1;
            break;
        default:
            PNBuf_sub   = (u8**)PNBuf_sub2;
            break;
        }

        if((sysPIPMain == PIP_MAIN_NONE) || (sysPIPMain == PIP_MAIN_CH1))
        {
            if(sysTVOutOnFlag)
            {
            }
            else
            {
                if(sysVideoInCHsel == 0x02)
                {
                #if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU))
                   if( (sysCameraMode == SYS_CAMERA_MODE_CIU_QUADSCR) || (sysCameraMode == SYS_CAMERA_MODE_GFU_TESTSCR) )
                   {
                      IduWinCtrl = (IduWinCtrl & ~0x00003000);
                      IduVidBuf0Addr=(u32)PNBuf_Quad;
                   #if NEW_IDU_BRI
                      BRI_IADDR_Y = IduVidBuf0Addr;
                      BRI_IADDR_C = BRI_IADDR_Y + ciu_2_pnbuf_size_y;
                   #endif
                   }
                   else
                #endif
                   {
                        IduWinCtrl = (IduWinCtrl & ~0x00003000); //| ((ciu_idufrmcnt_ch1 % 3) << 12);
                    #if HW_DEINTERLACE_CIU2_ENA
                        IduVidBuf0Addr=((u32)PNBuf_sub[ (ciu_idufrmcnt_ch2-1) & 0x03]);
                    #else
                        IduVidBuf0Addr=((u32)PNBuf_sub[ ciu_idufrmcnt_ch2 & 0x03]);
                    #endif
                    #if NEW_IDU_BRI
                        BRI_IADDR_Y = IduVidBuf0Addr;
                        BRI_IADDR_C = BRI_IADDR_Y + ciu_2_pnbuf_size_y;
                      #if DUAL_MODE_DISP_SUPPORT
                        if(sysDualModeDisp)
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_2_OutWidth * 2);
                        else
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_2_OutWidth);
                      #endif
                    #endif
                   }
                }
            }
        }

       
       if(ciu_2_OpMode == SIUMODE_MPEGAVI)
       {
        #if MULTI_CHANNEL_VIDEO_REC
           if((pVideoClipOption->VideoPictureIndex + 2) >= ciu_idufrmcnt_ch2)
           {
                if((sysPIPMain == PIP_MAIN_NONE) ||
                    ((sysPIPMain == PIP_MAIN_CH1) && ((ciu_idufrmcnt_ch2 + 1) < ciu_idufrmcnt_ch1)) ||
                    ((sysPIPMain == PIP_MAIN_CH2) && (ciu_idufrmcnt_ch2 < (ciu_idufrmcnt_ch1 + 2))))
                {
                    if(pVideoClipOption->sysReady2CaptureVideo)
                    {
                       #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                        timerCountRead(2, (u32*) &IISTimeOffsetCIU);
                        IISTimeOffsetCIU    = IISTimeOffsetCIU >> 8;
                        if(pVideoClipOption->IISTimeOffset >= IISTimeOffsetCIU) {
                            TimeOffset  = pVideoClipOption->IISTimeOffset - IISTimeOffsetCIU;
                        } else {
                            TimeOffset  = pVideoClipOption->IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetCIU;
                        }
                        if(TimeOffset > pVideoClipOption->IISTimeUnit) {
                            TimeOffset  = pVideoClipOption->IISTimeUnit;
                        }
                        IISTime1    = pVideoClipOption->IISTime + TimeOffset;
                        if(IISTime1 > ciu_2_FrameTime) {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch2 % ISU_FRAME_DURATION_NUM]  = IISTime1 - ciu_2_FrameTime;
                            ciu_2_FrameTime                                                                 = IISTime1;
                        }
            		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
            			timerCountRead(1, (u32*) &IISTimeOffsetCIU);
                        IISTimeOffsetCIU    = IISTimeOffsetCIU / 100;
            			if(pVideoClipOption->IISTimeOffset >= IISTimeOffsetCIU) {
            				TimeOffset  = pVideoClipOption->IISTimeOffset - IISTimeOffsetCIU;
            			} else {
            			    TimeOffset  = pVideoClipOption->IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
            			}
            			if(TimeOffset > pVideoClipOption->IISTimeUnit) {
            				TimeOffset  = pVideoClipOption->IISTimeUnit;
            			}
            			IISTime1    = pVideoClipOption->IISTime + TimeOffset;
            			if(IISTime1 > ciu_2_FrameTime) {
            				pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch2 % ISU_FRAME_DURATION_NUM]  = IISTime1 - ciu_2_FrameTime;
                            ciu_2_FrameTime                                                                 = IISTime1;
                        }
                      #else   // only use IIS time to calculate frame time
                        if(pVideoClipOption->IISTime > ciu_2_FrameTime) {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch2 % ISU_FRAME_DURATION_NUM]  = pVideoClipOption->IISTime - ciu_2_FrameTime;
                            ciu_2_FrameTime                                                                 = pVideoClipOption->IISTime;
                        }
                      #endif
                        else
                        {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch2 % ISU_FRAME_DURATION_NUM]  = 1;
                            ciu_2_FrameTime++;
                        }
                    }

                    OSSemPost(ciuCapSemEvt_CH2);
                    ciu_idufrmcnt_ch2 ++;
                    //CIU_2_CTL2  |= CIU_DATA_OUT_ENA;
                }
           }
           else     // frame skip
           {
               //CIU_2_CTL2  &= ~CIU_DATA_OUT_ENA;
               //DEBUG_CIU("%d %d\n", ciu_idufrmcnt_ch2, VideoPictureIndex);
               DEBUG_CIU("==Skip frame==\n");
           }
        #else
           //DEBUG_CIU("[%d,%d,%d] ",VideoPictureIndex & 0x03,ciu_idufrmcnt_ch2 & 0x03,VideoSmallPictureIndex& 0x03);
           if((VideoPictureIndex + 2) >= ciu_idufrmcnt_ch2)
           {
           #if MULTI_CHANNEL_VIDEO_REC
                if(VideoClipOption[2].sysReady2CaptureVideo)
           #else
                if(sysReady2CaptureVideo)
           #endif
                {
                   #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                    timerCountRead(2, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU >> 8;
                    if(IISTimeOffset >= IISTimeOffsetCIU) {
                        TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
                    } else {
                        TimeOffset  = IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetCIU;
                    }
                    if(TimeOffset > IISTimeUnit) {
                        TimeOffset  = IISTimeUnit;
                    }
                    IISTime1    = IISTime + TimeOffset;
                    if(IISTime1 > ciu_2_FrameTime) {
                        ISUFrameDuration[ciu_idufrmcnt_ch2 % ISU_FRAME_DURATION_NUM]    = IISTime1 - ciu_2_FrameTime;
                        ciu_2_FrameTime                                                 = IISTime1;
                    }
        		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
        			timerCountRead(1, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU / 100;
        			if(IISTimeOffset >= IISTimeOffsetCIU) 
                    {
        				TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
        			}
        			else 
                    {
        			    TimeOffset  = IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
        			}
        			if(TimeOffset > IISTimeUnit) 
                    {
        				TimeOffset  = IISTimeUnit;
        			}
        			IISTime1    = IISTime + TimeOffset;
        			if(IISTime1 > ciu_2_FrameTime) 
                    {
        				ISUFrameDuration[ciu_idufrmcnt_ch2 % ISU_FRAME_DURATION_NUM]    = IISTime1 - ciu_2_FrameTime;
                        ciu_2_FrameTime                                                 = IISTime1;
                    }
                  #else   // only use IIS time to calculate frame time
                    if(IISTime > ciu_2_FrameTime) {
                        ISUFrameDuration[ciu_idufrmcnt_ch2 % ISU_FRAME_DURATION_NUM]    = IISTime - ciu_2_FrameTime;
                        ciu_2_FrameTime                                                 = IISTime;
                    }
                  #endif
                    else
                    {
                        ISUFrameDuration[ciu_idufrmcnt_ch2 % ISU_FRAME_DURATION_NUM]    = 1;
                        ciu_2_FrameTime++;
                    }
                }

               OSSemPost(ciuCapSemEvt_CH2);
               ciu_idufrmcnt_ch2 ++;
               //CIU_2_CTL2  |= CIU_DATA_OUT_ENA;
           }
           else     // frame skip
           {
               //CIU_2_CTL2  &= ~CIU_DATA_OUT_ENA;
               //DEBUG_CIU("%d %d\n", ciu_idufrmcnt_ch2, VideoPictureIndex);
               DEBUG_CIU("==Skip frame==\n");
           }
        #endif  // #if MULTI_CHANNEL_VIDEO_REC
       }
       else // ciu_2_OpMode != SIUMODE_MPEGAVI
       {
            if(sysPIPMain == PIP_MAIN_NONE)
            {
                ciu_idufrmcnt_ch2 ++;
            }
            else if(sysPIPMain == PIP_MAIN_CH1)
            {
                if((ciu_idufrmcnt_ch2 + 1) < ciu_idufrmcnt_ch1)
                {
                    ciu_idufrmcnt_ch2 ++;
                    //CiuSetIduAdr    = 1;
                }
            }
            else if(sysPIPMain == PIP_MAIN_CH2)
            {
                u32 temp = (u32)PNBuf_sub[(ciu_idufrmcnt_ch2 + 2) & 3];
                if((ciu_idufrmcnt_ch2 < (ciu_idufrmcnt_ch1 + 2)) && (temp != (u32)IduVidBuf0Addr))
                    ciu_idufrmcnt_ch2 ++;
            }
            //DEBUG_CIU("%d%d ", ciu_idufrmcnt_ch1 & 0x03, ciu_idufrmcnt_ch2 & 0x03);
       }


    #if ISUCIU_PREVIEW_PNOUT            
        #if (MULTI_STREAM_SUPPORT)
            #if SP_TEST
                CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + (ciu_2_OutY +60)* ciu_2_line_stride + ciu_2_OutX);
                CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y + (ciu_2_OutY +60)* ciu_2_line_stride / 2 + ciu_2_OutX);
            #else
                #if SWAP_MULTI_STREAM_SUPPORT
                    if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                    {
                        CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX);
                        CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX);
                        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
                    }
                    else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
                    {
                        #if HD_SWAP_MPSP_EN
                        CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX);
                        CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX);
                        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
                        #else
                        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX);
                        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX);
                        CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
                        #endif
                    }
                #else
                    CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX);
                    CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX);
                    CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX     + VIDEODISPBUF_OFFSET);
                    CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
                #endif
            #endif
        #else
                #if (CIU_SP_REP_MP)
                CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX);
                CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX);
                #else
                CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX);
                CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX);
                #endif
                #if CIU_DECIMATION_TEST
                CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX);
                CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX);
                #endif            
        #endif
    #else
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + (ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX) * 2);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y);
    #endif
      //--------------------------------------------------------------------------------------------------------------------//

   }

   if(intStat & CIU_INT_STAT_FIELD_END)
   {
   #if HW_DEINTERLACE_CIU2_ENA
       OSFlagPost(ciuFlagGrp_CH2, CIU_EVET_TOPFILD_END, OS_FLAG_SET, &err);
   #endif
   }

}

void ciuIntHandler_CH3(void)
{
    u32 intStat;
#if FINE_TIME_STAMP
    s32 IISTimeOffsetCIU, TimeOffset;
    u32 IISTime1;
#endif
    u8  **PNBuf_sub;
    u8          err;
    //==================================//

#if MULTI_CHANNEL_VIDEO_REC
    VIDEO_CLIP_OPTION   *pVideoClipOption   = &VideoClipOption[3];
#endif

    intStat = CIU_3_INTRPT;
    //DEBUG_CIU("CH3 intr\n");
    if(intStat & CIU_INT_STAT_FIFO_OVERF)
    {
        DEBUG_CIU("CIU3 FIFO overflow\n");
    }

    if(intStat & CIU_INT_STAT_FRAME_END)
    {
   //==========================HW Motion Detection/HW Deinterlace(A1018A)=======================//
   #if (HW_MD_SUPPORT)
        OSFlagPost(ciuFlagGrp_CH3, CIU_EVET_FREAME_END, OS_FLAG_SET, &err);
   #endif

        PNBuf_sub   = (u8**)PNBuf_sub3;
           
        if(sysTVOutOnFlag)
        {
        }
        else
        {
            if(sysVideoInCHsel == 0x03)
            {
            #if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
               if( (sysCameraMode == SYS_CAMERA_MODE_CIU_QUADSCR) || (sysCameraMode == SYS_CAMERA_MODE_GFU_TESTSCR) )
               {
                  IduWinCtrl = (IduWinCtrl & ~0x00003000);
                  IduVidBuf0Addr=(u32)PNBuf_Quad;
               #if NEW_IDU_BRI
                  BRI_IADDR_Y = IduVidBuf0Addr;
                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
               #endif
               }
               else
            #endif
               {
                    IduWinCtrl = (IduWinCtrl & ~0x00003000); //| ((ciu_idufrmcnt_ch1 % 3) << 12);
                #if HW_DEINTERLACE_CIU3_ENA
                    IduVidBuf0Addr=((u32)PNBuf_sub[ (ciu_idufrmcnt_ch3-1) & 0x03]);
                #else
                    IduVidBuf0Addr=((u32)PNBuf_sub[ ciu_idufrmcnt_ch3 & 0x03]);
                #endif

                #if NEW_IDU_BRI
                    BRI_IADDR_Y = IduVidBuf0Addr;
                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                #endif
               }
            }
        }

       if(ciu_3_OpMode == SIUMODE_MPEGAVI)
       {
        #if MULTI_CHANNEL_VIDEO_REC
           if((pVideoClipOption->VideoPictureIndex + 2) >= ciu_idufrmcnt_ch3)
           {
                if(pVideoClipOption->sysReady2CaptureVideo)
                {
                   #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                    timerCountRead(2, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU >> 8;
                    if(pVideoClipOption->IISTimeOffset >= IISTimeOffsetCIU) {
                        TimeOffset  = pVideoClipOption->IISTimeOffset - IISTimeOffsetCIU;
                    } else {
                        TimeOffset  = pVideoClipOption->IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetCIU;
                    }
                    if(TimeOffset > pVideoClipOption->IISTimeUnit) {
                        TimeOffset  = pVideoClipOption->IISTimeUnit;
                    }
                    IISTime1    = pVideoClipOption->IISTime + TimeOffset;
                    if(IISTime1 > ciu_3_FrameTime) {
                        pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch3 % ISU_FRAME_DURATION_NUM]  = IISTime1 - ciu_3_FrameTime;
                        ciu_3_FrameTime                                                                 = IISTime1;
                    }
        		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
        			timerCountRead(1, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU / 100;
        			if(pVideoClipOption->IISTimeOffset >= IISTimeOffsetCIU) {
        				TimeOffset  = pVideoClipOption->IISTimeOffset - IISTimeOffsetCIU;
        			} else {
        			    TimeOffset  = pVideoClipOption->IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
        			}
        			if(TimeOffset > pVideoClipOption->IISTimeUnit) {
        				TimeOffset  = pVideoClipOption->IISTimeUnit;
        			}
        			IISTime1    = pVideoClipOption->IISTime + TimeOffset;
        			if(IISTime1 > ciu_3_FrameTime) {
        				pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch3 % ISU_FRAME_DURATION_NUM]  = IISTime1 - ciu_3_FrameTime;
                        ciu_3_FrameTime                                                                 = IISTime1;
                    }
                  #else   // only use IIS time to calculate frame time
                    if(pVideoClipOption->IISTime > ciu_3_FrameTime) {
                        pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch3 % ISU_FRAME_DURATION_NUM]  = pVideoClipOption->IISTime - ciu_3_FrameTime;
                        ciu_3_FrameTime                                                                 = pVideoClipOption->IISTime;
                    }
                  #endif
                    else
                    {
                        pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch3 % ISU_FRAME_DURATION_NUM]  = 1;
                        ciu_3_FrameTime++;
                    }
                }

                OSSemPost(ciuCapSemEvt_CH3);
                ciu_idufrmcnt_ch3 ++;
           }
           else     // frame skip
           {
               //CIU_2_CTL2  &= ~CIU_DATA_OUT_ENA;
               //DEBUG_CIU("%d %d\n", ciu_idufrmcnt_ch2, VideoPictureIndex);
           }
        #else
           if((VideoPictureIndex + 2) >= ciu_idufrmcnt_ch3)
           {
           #if MULTI_CHANNEL_VIDEO_REC
                if(VideoClipOption[3].sysReady2CaptureVideo)
           #else
                if(sysReady2CaptureVideo)
           #endif
                {
                   #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                    timerCountRead(2, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU >> 8;
                    if(IISTimeOffset >= IISTimeOffsetCIU) {
                        TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
                    } else {
                        TimeOffset  = IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetCIU;
                    }
                    if(TimeOffset > IISTimeUnit) {
                        TimeOffset  = IISTimeUnit;
                    }
                    IISTime1    = IISTime + TimeOffset;
                    if(IISTime1 > ciu_3_FrameTime) {
                        ISUFrameDuration[ciu_idufrmcnt_ch3 % ISU_FRAME_DURATION_NUM]    = IISTime1 - ciu_3_FrameTime;
                        ciu_3_FrameTime                                                 = IISTime1;
                    }
        		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
        			timerCountRead(1, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU / 100;
        			if(IISTimeOffset >= IISTimeOffsetCIU) {
        				TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
        			} else {
        			    TimeOffset  = IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
        			}
        			if(TimeOffset > IISTimeUnit) {
        				TimeOffset  = IISTimeUnit;
        			}
        			IISTime1    = IISTime + TimeOffset;
        			if(IISTime1 > ciu_3_FrameTime) {
        				ISUFrameDuration[ciu_idufrmcnt_ch3 % ISU_FRAME_DURATION_NUM]    = IISTime1 - ciu_3_FrameTime;
                        ciu_3_FrameTime                                                 = IISTime1;
                    }
                  #else   // only use IIS time to calculate frame time
                    if(IISTime > ciu_3_FrameTime) {
                        ISUFrameDuration[ciu_idufrmcnt_ch3 % ISU_FRAME_DURATION_NUM]    = IISTime - ciu_3_FrameTime;
                        ciu_3_FrameTime                                                 = IISTime;
                    }
                  #endif
                    else
                    {
                        ISUFrameDuration[ciu_idufrmcnt_ch3 % ISU_FRAME_DURATION_NUM]    = 1;
                        ciu_3_FrameTime++;
                    }
                }

               OSSemPost(ciuCapSemEvt_CH3);
               ciu_idufrmcnt_ch3 ++;
           }
           else     // frame skip
           {
           }
        #endif  // #if MULTI_CHANNEL_VIDEO_REC
       }
       else // ciu_2_OpMode != SIUMODE_MPEGAVI
       {
            ciu_idufrmcnt_ch3 ++;
            //DEBUG_CIU("%d%d ", ciu_idufrmcnt_ch1 & 0x03, ciu_idufrmcnt_ch2 & 0x03);
       }


    #if ISUCIU_PREVIEW_PNOUT
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch3 + 1) & 0x03] + ciu_3_OutY * ciu_3_line_stride + ciu_3_OutX);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch3 + 1) & 0x03] + PNBUF_SIZE_Y + ciu_3_OutY * ciu_3_line_stride / 2 + ciu_3_OutX);
    #else
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch3 + 1) & 0x03] + (ciu_3_OutY * ciu_3_line_stride + ciu_3_OutX) * 2);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch3 + 1) & 0x03] + PNBUF_SIZE_Y);
    #endif

   }

   if(intStat & CIU_INT_STAT_FIELD_END)
   {
   #if HW_DEINTERLACE_CIU3_ENA
       OSFlagPost(ciuFlagGrp_CH3, CIU_EVET_TOPFILD_END, OS_FLAG_SET, &err);
   #endif
   }

}

void ciuIntHandler_CH4(void)
{
    u32 intStat;
#if FINE_TIME_STAMP
    s32 IISTimeOffsetCIU, TimeOffset;
    u32 IISTime1;
#endif
    u8  **PNBuf_sub;
    u8          err;
    //==================================//

#if MULTI_CHANNEL_VIDEO_REC
    VIDEO_CLIP_OPTION   *pVideoClipOption   = &VideoClipOption[4];
#endif

    intStat = CIU_4_INTRPT;

    if(intStat & CIU_INT_STAT_FIFO_OVERF)
    {
        DEBUG_CIU("CIU4 FIFO overflow\n");
    }

    if(intStat & CIU_INT_STAT_FRAME_END)
    {
    //==========================HW Motion Detection/HW Deinterlace(A1018A)=======================//
   #if (HW_MD_SUPPORT )
        OSFlagPost(ciuFlagGrp_CH4, CIU_EVET_FREAME_END, OS_FLAG_SET, &err);
   #endif

        PNBuf_sub   = (u8**)PNBuf_sub4;

        if(sysTVOutOnFlag)
        {
        }
        else
        {
            if(sysVideoInCHsel == 0x04)
            {
            #if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
               if( (sysCameraMode == SYS_CAMERA_MODE_CIU_QUADSCR) || (sysCameraMode == SYS_CAMERA_MODE_GFU_TESTSCR) )
               {
                  IduWinCtrl = (IduWinCtrl & ~0x00003000);
                  IduVidBuf0Addr=(u32)PNBuf_Quad;
               #if NEW_IDU_BRI
                  BRI_IADDR_Y = IduVidBuf0Addr;
                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
               #endif
               }
               else
            #endif
               {
                    IduWinCtrl = (IduWinCtrl & ~0x00003000); //| ((ciu_idufrmcnt_ch1 % 3) << 12);
                #if HW_DEINTERLACE_CIU4_ENA
                    IduVidBuf0Addr=((u32)PNBuf_sub[ (ciu_idufrmcnt_ch4-1) & 0x03]);
                #else
                    IduVidBuf0Addr=((u32)PNBuf_sub[ ciu_idufrmcnt_ch4 & 0x03]);
                #endif

                #if NEW_IDU_BRI
                    BRI_IADDR_Y = IduVidBuf0Addr;
                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                #endif
               }
            }
        }

       if(ciu_4_OpMode == SIUMODE_MPEGAVI)
       {
        #if MULTI_CHANNEL_VIDEO_REC
           if((pVideoClipOption->VideoPictureIndex + 2) >= ciu_idufrmcnt_ch4)
           {
                    if(pVideoClipOption->sysReady2CaptureVideo)
                    {
                       #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                        timerCountRead(2, (u32*) &IISTimeOffsetCIU);
                        IISTimeOffsetCIU    = IISTimeOffsetCIU >> 8;
                        if(pVideoClipOption->IISTimeOffset >= IISTimeOffsetCIU) {
                            TimeOffset  = pVideoClipOption->IISTimeOffset - IISTimeOffsetCIU;
                        } else {
                            TimeOffset  = pVideoClipOption->IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetCIU;
                        }
                        if(TimeOffset > pVideoClipOption->IISTimeUnit) {
                            TimeOffset  = pVideoClipOption->IISTimeUnit;
                        }
                        IISTime1    = pVideoClipOption->IISTime + TimeOffset;
                        if(IISTime1 > ciu_4_FrameTime) {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch4 % ISU_FRAME_DURATION_NUM]  = IISTime1 - ciu_4_FrameTime;
                            ciu_4_FrameTime                                                                 = IISTime1;
                        }
            		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
            			timerCountRead(1, (u32*) &IISTimeOffsetCIU);
                        IISTimeOffsetCIU    = IISTimeOffsetCIU / 100;
            			if(pVideoClipOption->IISTimeOffset >= IISTimeOffsetCIU) {
            				TimeOffset  = pVideoClipOption->IISTimeOffset - IISTimeOffsetCIU;
            			} else {
            			    TimeOffset  = pVideoClipOption->IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
            			}
            			if(TimeOffset > pVideoClipOption->IISTimeUnit) {
            				TimeOffset  = pVideoClipOption->IISTimeUnit;
            			}
            			IISTime1    = pVideoClipOption->IISTime + TimeOffset;
            			if(IISTime1 > ciu_4_FrameTime) {
            				pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch4 % ISU_FRAME_DURATION_NUM]  = IISTime1 - ciu_4_FrameTime;
                            ciu_4_FrameTime                                                                 = IISTime1;
                        }
                      #else   // only use IIS time to calculate frame time
                        if(pVideoClipOption->IISTime > ciu_4_FrameTime) {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch4 % ISU_FRAME_DURATION_NUM]  = pVideoClipOption->IISTime - ciu_4_FrameTime;
                            ciu_4_FrameTime                                                                 = pVideoClipOption->IISTime;
                        }
                      #endif
                        else
                        {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch4 % ISU_FRAME_DURATION_NUM]  = 1;
                            ciu_4_FrameTime++;
                        }
                    }

                    OSSemPost(ciuCapSemEvt_CH4);
                    ciu_idufrmcnt_ch4 ++;
           }
           else     // frame skip
           {
           }
        #else
           if((VideoPictureIndex + 2) >= ciu_idufrmcnt_ch4)
           {
           #if MULTI_CHANNEL_VIDEO_REC
                if(VideoClipOption[4].sysReady2CaptureVideo)
           #else
                if(sysReady2CaptureVideo)
           #endif
                {
                   #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                    timerCountRead(2, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU >> 8;
                    if(IISTimeOffset >= IISTimeOffsetCIU) {
                        TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
                    } else {
                        TimeOffset  = IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetCIU;
                    }
                    if(TimeOffset > IISTimeUnit) {
                        TimeOffset  = IISTimeUnit;
                    }
                    IISTime1    = IISTime + TimeOffset;
                    if(IISTime1 > ciu_4_FrameTime) {
                        ISUFrameDuration[ciu_idufrmcnt_ch4 % ISU_FRAME_DURATION_NUM]    = IISTime1 - ciu_4_FrameTime;
                        ciu_4_FrameTime                                                 = IISTime1;
                    }
        		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
        			timerCountRead(1, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU / 100;
        			if(IISTimeOffset >= IISTimeOffsetCIU) {
        				TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
        			} else {
        			    TimeOffset  = IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
        			}
        			if(TimeOffset > IISTimeUnit) {
        				TimeOffset  = IISTimeUnit;
        			}
        			IISTime1    = IISTime + TimeOffset;
        			if(IISTime1 > ciu_4_FrameTime) {
        				ISUFrameDuration[ciu_idufrmcnt_ch4 % ISU_FRAME_DURATION_NUM]    = IISTime1 - ciu_4_FrameTime;
                        ciu_4_FrameTime                                                 = IISTime1;
                    }
                  #else   // only use IIS time to calculate frame time
                    if(IISTime > ciu_4_FrameTime) {
                        ISUFrameDuration[ciu_idufrmcnt_ch4 % ISU_FRAME_DURATION_NUM]    = IISTime - ciu_4_FrameTime;
                        ciu_4_FrameTime                                                 = IISTime;
                    }
                  #endif
                    else
                    {
                        ISUFrameDuration[ciu_idufrmcnt_ch4 % ISU_FRAME_DURATION_NUM]    = 1;
                        ciu_4_FrameTime++;
                    }
                }

               OSSemPost(ciuCapSemEvt_CH4);
               ciu_idufrmcnt_ch4 ++;
           }
           else     // frame skip
           {
           }
        #endif  // #if MULTI_CHANNEL_VIDEO_REC
       }
       else // ciu_2_OpMode != SIUMODE_MPEGAVI
       {
            ciu_idufrmcnt_ch4 ++;
            //DEBUG_CIU("%d%d ", ciu_idufrmcnt_ch1 & 0x03, ciu_idufrmcnt_ch2 & 0x03);
       }

    #if ISUCIU_PREVIEW_PNOUT
        CIU_4_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch4 + 1) & 0x03] + ciu_4_OutY * ciu_4_line_stride + ciu_4_OutX);
        CIU_4_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch4 + 1) & 0x03] + PNBUF_SIZE_Y + ciu_4_OutY * ciu_4_line_stride / 2 + ciu_4_OutX);
    #else
        CIU_4_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch4 + 1) & 0x03] + (ciu_4_OutY * ciu_4_line_stride + ciu_4_OutX) * 2);
        CIU_4_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch4 + 1) & 0x03] + PNBUF_SIZE_Y);
    #endif

   }

   if(intStat & CIU_INT_STAT_FIELD_END)
   {
   #if HW_DEINTERLACE_CIU4_ENA
       OSFlagPost(ciuFlagGrp_CH4, CIU_EVET_TOPFILD_END, OS_FLAG_SET, &err);
   #endif
   }

}

void ciuIntHandler_CH5(void)
{
    u32 intStat;
#if FINE_TIME_STAMP
    s32 IISTimeOffsetCIU, TimeOffset;
    u32 IISTime1;
#endif
    u8  **PNBuf_sub;
    u8          err;
#if CIU_DEBUG_INTR
    static u32  Debug_count=0;
    u8 level;
#endif
    //==================================//

#if MULTI_CHANNEL_VIDEO_REC
    VIDEO_CLIP_OPTION   *pVideoClipOption   = &VideoClipOption[5];
#endif

    intStat = CIU_5_INTRPT;
#if CIU_DEBUG_INTR
    gpioGetLevel(1,17,&level);
    gpioSetLevel(1, 17, (~(level))&0x01);
#endif

    //DEBUG_CIU("c");

    if(intStat & CIU_INT_STAT_FIFO_OVERF)
    {
        DEBUG_CIU("CIU5 FIFO overflow\n");
    }
    if(intStat & CIU_INT_STAT_SP_OVERF)
    {
#if (Sensor_OPTION == Sensor_XC7021_SC2133 || Sensor_OPTION == Sensor_XC7021_GC2023 || Sensor_OPTION == Sensor_ZN220_YUV601) //避花屏 丟棄frame when sp overflow 
        //DEBUG_CIU("CIU5 SP FIFO overflow\n");
        return;
#else
        DEBUG_CIU("CIU5 SP FIFO overflow\n");
#endif
    }

    if(intStat & CIU_INT_STAT_FRAME_END)
    {
        ciu_5_FPS_Count ++;
    //==========================HW Motion Detection/HW Deinterlace(A1018A)=======================//
   #if (HW_MD_SUPPORT  || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
        OSFlagPost(ciuFlagGrp_CH5, CIU_EVET_FREAME_END, OS_FLAG_SET, &err);
   #endif

        PNBuf_sub   = (u8**)PNBuf_sub5;

        if(sysTVOutOnFlag)
        {
        }
        else
        {
            if(sysVideoInCHsel == 0x05)
            {
            #if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
               if( (sysCameraMode == SYS_CAMERA_MODE_CIU_QUADSCR) || (sysCameraMode == SYS_CAMERA_MODE_GFU_TESTSCR) )
               {
                  IduWinCtrl = (IduWinCtrl & ~0x00003000);
                  IduVidBuf0Addr=(u32)PNBuf_Quad;
               #if NEW_IDU_BRI
                  BRI_IADDR_Y = IduVidBuf0Addr;
                  BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
               #endif
               }
               else
            #endif
               {
                    IduWinCtrl = (IduWinCtrl & ~0x00003000); //| ((ciu_idufrmcnt_ch1 % 3) << 12);
                #if HW_DEINTERLACE_CIU5_ENA
                    IduVidBuf0Addr=((u32)PNBuf_sub[ (ciu_idufrmcnt_ch5-1) & 0x03]);
                #else
                    IduVidBuf0Addr=((u32)PNBuf_sub[ ciu_idufrmcnt_ch5 & 0x03]);
                #endif

                #if NEW_IDU_BRI
                    BRI_IADDR_Y = IduVidBuf0Addr;
                    BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                #endif
               }
            }
        }

       if(ciu_5_OpMode == SIUMODE_MPEGAVI)
       {
        #if MULTI_CHANNEL_VIDEO_REC
           if((pVideoClipOption->VideoPictureIndex + 2) >= ciu_idufrmcnt_ch5)
           {
                    if(pVideoClipOption->sysReady2CaptureVideo)
                    {
                       #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                        timerCountRead(2, (u32*) &IISTimeOffsetCIU);
                        IISTimeOffsetCIU    = IISTimeOffsetCIU >> 8;
                        if(pVideoClipOption->IISTimeOffset >= IISTimeOffsetCIU) {
                            TimeOffset  = pVideoClipOption->IISTimeOffset - IISTimeOffsetCIU;
                        } else {
                            TimeOffset  = pVideoClipOption->IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetCIU;
                        }
                        if(TimeOffset > pVideoClipOption->IISTimeUnit) {
                            TimeOffset  = pVideoClipOption->IISTimeUnit;
                        }
                        IISTime1    = pVideoClipOption->IISTime + TimeOffset;
                        if(IISTime1 > ciu_5_FrameTime) {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch5 % ISU_FRAME_DURATION_NUM]  = IISTime1 - ciu_5_FrameTime;
                            ciu_5_FrameTime                                                                 = IISTime1;
                        }
            		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
            			timerCountRead(1, (u32*) &IISTimeOffsetCIU);
                        IISTimeOffsetCIU    = IISTimeOffsetCIU / 100;
            			if(pVideoClipOption->IISTimeOffset >= IISTimeOffsetCIU) {
            				TimeOffset  = pVideoClipOption->IISTimeOffset - IISTimeOffsetCIU;
            			} else {
            			    TimeOffset  = pVideoClipOption->IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
            			}
            			if(TimeOffset > pVideoClipOption->IISTimeUnit) {
            				TimeOffset  = pVideoClipOption->IISTimeUnit;
            			}
            			IISTime1    = pVideoClipOption->IISTime + TimeOffset;
            			if(IISTime1 > ciu_5_FrameTime) {
            				pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch5 % ISU_FRAME_DURATION_NUM]  = IISTime1 - ciu_5_FrameTime;
                            ciu_5_FrameTime                                                                 = IISTime1;
                        }
                      #else   // only use IIS time to calculate frame time
                        if(pVideoClipOption->IISTime > ciu_5_FrameTime) {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch5 % ISU_FRAME_DURATION_NUM]  = pVideoClipOption->IISTime - ciu_5_FrameTime;
                            ciu_5_FrameTime                                                                 = pVideoClipOption->IISTime;
                        }
                      #endif
                        else
                        {
                            pVideoClipOption->ISUFrameDuration[ciu_idufrmcnt_ch5 % ISU_FRAME_DURATION_NUM]  = 1;
                            ciu_5_FrameTime++;
                        }
                    }

                    OSSemPost(ciuCapSemEvt_CH5);
                #if(Sensor_OPTION == Sensor_MI_5M)    
                    OSSemPost(siuSemEvt);
                #endif
                    ciu_idufrmcnt_ch5 ++;
           }
           else     // frame skip
           {
           }
        #else
           if((VideoPictureIndex + 2) >= ciu_idufrmcnt_ch5)
           {
           #if MULTI_CHANNEL_VIDEO_REC
                if(VideoClipOption[5].sysReady2CaptureVideo)
           #else
                if(sysReady2CaptureVideo)
           #endif
                {
                   #if (FINE_TIME_STAMP == USE_TIMER2_FINE_TIME_STAMP) // use IIS time + Timer3 to calculate frame time
                    timerCountRead(2, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU >> 8;
                    if(IISTimeOffset >= IISTimeOffsetCIU) {
                        TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
                    } else {
                        TimeOffset  = IISTimeOffset + (TIMER2_COUNT >> 8) - IISTimeOffsetCIU;
                    }
                    if(TimeOffset > IISTimeUnit) {
                        TimeOffset  = IISTimeUnit;
                    }
                    IISTime1    = IISTime + TimeOffset;
                    if(IISTime1 > ciu_5_FrameTime) {
                        ISUFrameDuration[ciu_idufrmcnt_ch5 % ISU_FRAME_DURATION_NUM]    = IISTime1 - ciu_5_FrameTime;
                        ciu_5_FrameTime                                                 = IISTime1;
                    }
        		  #elif (FINE_TIME_STAMP == USE_TIMER1_FINE_TIME_STAMP)
        			timerCountRead(1, (u32*) &IISTimeOffsetCIU);
                    IISTimeOffsetCIU    = IISTimeOffsetCIU / 100;
        			if(IISTimeOffset >= IISTimeOffsetCIU) {
        				TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
        			} else {
        			    TimeOffset  = IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
        			}
        			if(TimeOffset > IISTimeUnit) {
        				TimeOffset  = IISTimeUnit;
        			}
        			IISTime1    = IISTime + TimeOffset;
        			if(IISTime1 > ciu_5_FrameTime) {
        				ISUFrameDuration[ciu_idufrmcnt_ch5 % ISU_FRAME_DURATION_NUM]    = IISTime1 - ciu_5_FrameTime;
                        ciu_5_FrameTime                                                 = IISTime1;
                    }
                  #else   // only use IIS time to calculate frame time
                    if(IISTime > ciu_5_FrameTime) {
                        ISUFrameDuration[ciu_idufrmcnt_ch5 % ISU_FRAME_DURATION_NUM]    = IISTime - ciu_5_FrameTime;
                        ciu_5_FrameTime                                                 = IISTime;
                    }
                  #endif
                    else
                    {
                        ISUFrameDuration[ciu_idufrmcnt_ch5 % ISU_FRAME_DURATION_NUM]    = 1;
                        ciu_5_FrameTime++;
                    }
                }

               OSSemPost(ciuCapSemEvt_CH5);
            #if(Sensor_OPTION == Sensor_MI_5M)    
               OSSemPost(siuSemEvt);
            #endif
               
               ciu_idufrmcnt_ch5 ++;
           }
           else     // frame skip
           {
           }
        #endif  // #if MULTI_CHANNEL_VIDEO_REC
       }
       else // ciu_2_OpMode != SIUMODE_MPEGAVI
       {
            ciu_idufrmcnt_ch5 ++;
            //DEBUG_CIU("%d%d ", ciu_idufrmcnt_ch1 & 0x03, ciu_idufrmcnt_ch2 & 0x03);
       }

    #if ISUCIU_PREVIEW_PNOUT
        #if (MULTI_STREAM_SUPPORT)
            #if SP_TEST
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + (ciu_5_OutY +60)* ciu_5_line_stride + ciu_5_OutX);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_pnbuf_size_y + (ciu_5_OutY +60)* ciu_5_line_stride / 2 + ciu_5_OutX);
            #else
                #if SWAP_MULTI_STREAM_SUPPORT
                    if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                    {
                        CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX);
                        CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_pnbuf_size_y + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX);
                        CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
                    }
                    else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
                    {
                        #if HD_SWAP_MPSP_EN
                        CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX);
                        CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_pnbuf_size_y + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX);
                        CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
                        #else
                        CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX);
                        CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_pnbuf_size_y + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX);
                        CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
                        #endif
                    }
                    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
                    {
                        CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX);
                        CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_pnbuf_size_y + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX);
                        CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX     + VIDEODISPBUF_OFFSET);
                        CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
                    }
                #else
                    CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX);
                    CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_pnbuf_size_y + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX);
                    CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX     + VIDEODISPBUF_OFFSET);
                    CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
                #endif
            #endif
        #else
                #if (CIU_SP_REP_MP)
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_pnbuf_size_y + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX);
                #else
                CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX);
                CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_pnbuf_size_y + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX);
                #endif
                #if CIU_DECIMATION_TEST
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + ciu_5_pnbuf_size_y + ciu_5_OutY * ciu_5_line_stride / 2 + ciu_5_OutX);
                #endif            
        #endif
    #else
        CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + (ciu_5_OutY * ciu_5_line_stride + ciu_5_OutX) * 2);
        CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch5 + 1) & 0x03] + PNBUF_SIZE_Y);
    #endif

   }

   if(intStat & CIU_INT_STAT_FIELD_END)
   {
   #if (HW_DEINTERLACE_CIU5_ENA || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU))
       OSFlagPost(ciuFlagGrp_CH5, CIU_EVET_TOPFILD_END, OS_FLAG_SET, &err);
   #endif
   }

}

int ciu1_ChangeInputSize(int InWidth,int InHeight)
{
     CIU_1_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
     return 1;
}

int ciu2_ChangeInputSize(int InWidth,int InHeight)
{
     CIU_2_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
     return 1;
}

int ciu3_ChangeInputSize(int InWidth,int InHeight)
{
     CIU_3_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
     return 1;
}

int ciu4_ChangeInputSize(int InWidth,int InHeight)
{
     CIU_4_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
     return 1;
}

int ciu5_ChangeInputSize(int InWidth,int InHeight)
{
     CIU_5_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
     return 1;
}

#define CIU_OSD_CHARMAX  32

s32 ciuPreviewInit_CH1(u8 mode,u32 InWidth, u32 InHeight, u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en, u32 line_stride)
{
#if NEW_CIU1_EN
{
    u8  **PNBuf_sub;
    u8  err;
    u32 NEWCIU1_CTR2=0;
 #if FPGA_BOARD_A1018_SERIES
    char ciuOSDStr1[CIU_OSD_CHARMAX+16]="CH_1DEFGHIJKLMNO123456789QWERTYU";
 #else
    char ciuOSDStr1[CIU_OSD_CHARMAX+16]="                                ";
 #endif
 #if (SWAP_MULTI_STREAM_SUPPORT || MULTI_STREAM_SUPPORT)
    u32 sp_reso;
 #endif
    //=================================//

	IISTime = 0;
	ciu_1_FrameTime = 0;
    DEBUG_RED("ciuPreviewInit_CH1\n");
    #if SWAP_MULTI_STREAM_SUPPORT
        if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
        {
            sp_reso = CIU_SP_DECI_1X1;
            NEWCIU1_CTR2 = ( CIU_SA_SEL_DOWNSAMPLE | CIU_SP_SEL_SENSOR | CIU_MP_SEL_SCALAR | CIU_DS_MODE_1_2 );
            OutWidth = 640;
            OutHeight = 360;
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH1 MP Scaling dwon 640x360\n");
            #endif
        }
        else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) )
        {
          #if HD_SWAP_MPSP_EN
            sp_reso = CIU_SP_DECI_1X1;
            NEWCIU1_CTR2 = ( CIU_SA_SEL_DOWNSAMPLE | CIU_SP_SEL_SENSOR | CIU_MP_SEL_SCALAR | CIU_DS_MODE_1_2 );
            OutWidth = 640;
            OutHeight = 360;
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH1 MP Scaling dwon 640x360\n");
            #endif
          #else
            sp_reso = CIU_SP_DECI_2X2;
            NEWCIU1_CTR2 = ( CIU_SA_SEL_SENSOR | CIU_SP_SEL_DOWNSAMPLE | CIU_MP_SEL_SCALAR | CIU_DS_MODE_1_2 );
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH1 SP Downsample dwon 640x360 %x\n",sp_reso);
            #endif
          #endif
        }
        else if (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
            sp_reso = CIU_SP_DECI_1X1;
            NEWCIU1_CTR2 = ( CIU_SA_SEL_DOWNSAMPLE | CIU_SP_SEL_DOWNSAMPLE | CIU_MP_SEL_SCALAR | CIU_DS_MODE_1_2 );
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH1 MP SP Downsample dwon 640x360 %x\n",sp_reso);
            #endif
        }
        else if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
        {
            sp_reso = CIU_SP_DECI_1X1;
            DEBUG_CIU("ciuPreviewInit_CH1 SP Downsample dwon 320x240 %x\n",sp_reso);
        }
    #elif (MULTI_STREAM_SUPPORT)
        if (CIU_SP_RESO == SP_1x1)
            sp_reso = CIU_SP_DECI_1X1;
        else if (CIU_SP_RESO == SP_2x1)
            sp_reso = CIU_SP_DECI_2X1;
        else if (CIU_SP_RESO == SP_2x2)
            sp_reso = CIU_SP_DECI_2X2;
        else if (CIU_SP_RESO == SP_4x2)
            sp_reso = CIU_SP_DECI_4X2;
        else if (CIU_SP_RESO == SP_4x4)
            sp_reso = CIU_SP_DECI_4X4 ;
    #endif    
    DEBUG_CIU("ciuPreviewInit_CH1(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n", mode, InWidth, InHeight, OutWidth, OutHeight, OutX, OutY, OSD_en, line_stride);

    ciu_1_pnbuf_size_y  = PNBUF_SIZE_Y;
    #if (MULTI_STREAM_SUPPORT)
    ciu_1_pnbuf_min_size_y = PNBUF_SP_SIZE_Y;
    #endif

    ciu_idufrmcnt_ch1   = 0;  //0;
    OSSemSet(ciuCapSemEvt_CH1, 0, &err);
    ciu_1_OpMode = mode;

    CIU_1_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
    if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
    {
        if ((NEWCIU1_CTR2 & CIU_SA_SEL_DOWNSAMPLE)==CIU_SA_SEL_DOWNSAMPLE)
        {
            CIU_1_Scaling_InSize = ( (InWidth/2)<<CIU_INPUT_SIZE_X_SHFT) | ((InHeight/2)<<CIU_INPUT_SIZE_Y_SHFT);
            CIU_1_InputStride   =  (InWidth/2);
        }
        else
        {
            CIU_1_Scaling_InSize = ( (InWidth)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
            CIU_1_InputStride   =  InWidth;
        }
    }
    else
    {
        if ((NEWCIU1_CTR2 & CIU_SA_SEL_DOWNSAMPLE)==CIU_SA_SEL_DOWNSAMPLE)
        {
            CIU_1_Scaling_InSize = ( (InWidth/2)<<CIU_INPUT_SIZE_X_SHFT) | ((InHeight/2)<<CIU_INPUT_SIZE_Y_SHFT);
            CIU_1_InputStride   =  (InWidth/2);
        }
        else
        {
            CIU_1_Scaling_InSize = ( (InWidth)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
            CIU_1_InputStride   =  InWidth;
        }
    }
 #if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
    #if(TV_DECODER==WT8861)
      CIU_1_IMG_STR       = (((720-InWidth)+30)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
    #else
      CIU_1_IMG_STR       = (((720-InWidth)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
    #endif
 #else
    CIU_1_IMG_STR       = 0;
 #endif
    ciu_1_OutWidth      = OutWidth;
    ciu_1_OutHeight     = OutHeight;
    ciu_1_OutX          = OutX;
    ciu_1_OutY          = OutY;
    ciu_1_line_stride   = line_stride;
    
    PNBuf_sub   = (u8**)PNBuf_sub1;
    #if CIU_DROP_EN
        if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
        {
            CIU_1_DropReg = 0x89;
        }
        else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) )
        {
            CIU_1_DropReg = 0x7F;
        }
    #endif

    if(mode ==SIUMODE_CAP_RREVIEW)
    {
        CIU_1_CTL1       =
                         CIU_NEW_BURST16 |
                   #if(CIU1_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif ((CIU1_OPTION==Sensor_CCIR601) || (TV_DECODER == AHD6124B) )
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                         CIU_OUT_FORMAT_422 |
                   #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((CIU1_OPTION==Sensor_NT99141_YUV601) || (CIU1_OPTION == Sensor_NT99340_YUV601) || (CIU1_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                   #if (CIU_SP_REP_MP)
                         CIU_MAINPATH_DIS |
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_1X1 |
                   #endif
                         CIU_656DATALATCH_RIS;


        CIU_1_FRAME_STRIDE   = line_stride * 2;
    }
    else
    {
        CIU_1_CTL1       =
                         CIU_NEW_BURST16 |
                   #if(CIU1_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif ((CIU1_OPTION==Sensor_CCIR601) || (TV_DECODER == AHD6124B) )
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                   #if ISUCIU_PREVIEW_PNOUT
                         CIU_OUT_FORMAT_420 |
                      #if (QUARD_MODE_DISP_SUPPORT || MULTI_STREAM_SUPPORT)
                         CIU_SP_OUT_FORMAT_420 |
                      #endif
                   #else
                         CIU_OUT_FORMAT_422 |
                      #if (QUARD_MODE_DISP_SUPPORT || MULTI_STREAM_SUPPORT)
                         CIU_SP_OUT_FORMAT_422 |
                      #endif
                   #endif

                   #if (QUARD_MODE_DISP_SUPPORT)
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_2X2 |
                   #endif
                   #if (MULTI_STREAM_SUPPORT)
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_1X1|
                         CIU_VSYNC_ACT_HI | // SP yuv map
                   #endif
                   #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((CIU1_OPTION==Sensor_NT99141_YUV601) || (CIU1_OPTION == Sensor_NT99340_YUV601) || (CIU1_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                   #if CIU_DECIMATION_TEST
                         CIU_MAINPATH_DIS |
                   #endif
                   #if (CIU_SP_REP_MP)
                         CIU_MAINPATH_DIS |
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_1X1|
                   #endif
                         CIU_656DATALATCH_RIS;
#if ISUCIU_PREVIEW_PNOUT
    #if SWAP_MULTI_STREAM_SUPPORT
        if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
        {
            CIU_1_FRAME_STRIDE   = 640;
        }
        else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720))
        {
            #if HD_SWAP_MPSP_EN
            CIU_1_FRAME_STRIDE   = 640;
            #else
            CIU_1_FRAME_STRIDE   = line_stride;
            #endif
        }
        else if (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
            CIU_1_FRAME_STRIDE   = 640;
        }
        else
            CIU_1_FRAME_STRIDE   = line_stride;
    #else
        CIU_1_FRAME_STRIDE   = line_stride;
        #if (CIU_SP_REP_MP)
        CIU_1_SP_FRAME_STRIDE = line_stride;
        #endif
    #endif
    
    #if (QUARD_MODE_DISP_SUPPORT)
        CIU_1_SP_FRAME_STRIDE= line_stride;
    #elif (MULTI_STREAM_SUPPORT)
        #if  SP_TEST
        CIU_1_SP_FRAME_STRIDE= line_stride;
        #else
            if (sp_reso == CIU_SP_DECI_1X1)
                CIU_1_SP_FRAME_STRIDE= line_stride;
            else if ( (sp_reso == CIU_SP_DECI_2X1) || (sp_reso == CIU_SP_DECI_2X2))
                CIU_1_SP_FRAME_STRIDE= line_stride / 2;
            else if ( (sp_reso == CIU_SP_DECI_4X2) || (sp_reso == CIU_SP_DECI_4X4))
                CIU_1_SP_FRAME_STRIDE= line_stride / 4;
        #endif
    #endif
#else
        CIU_1_FRAME_STRIDE   = line_stride * 2;
     #if (QUARD_MODE_DISP_SUPPORT)
        CIU_1_SP_FRAME_STRIDE= line_stride * 2;
     #endif
        #if (CIU_SP_REP_MP)
        CIU_1_SP_FRAME_STRIDE = line_stride;
        #endif
#endif
    }
#if CIU_DROP_EN
    if ((AE_Flicker_50_60_sel==SENSOR_AE_FLICKER_60HZ) && (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) )
        CIU_1_CTL1 |= CIU_FRAMEDROP_EN;
    else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) )
        CIU_1_CTL1 |= CIU_FRAMEDROP_EN;
#endif

#if CIU1_SCUP_EN
    CIU_1_CTL1 |= (CIU_SCA_EN);
    if( (InWidth== OutWidth) && (InHeight==OutHeight) )
    {
        CIU_1_CTL1 |=CIU_SCA_SHAREBUF_EN| CIU_SCA_EN;
    }
#endif

    if(sysPIPMain == PIP_MAIN_CH1)
    {
        CIU_1_CTL1 |= CIU_MODE_656_BOB;
        CIU_1_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }

    CIU_1_OutputSize    = ( OutWidth<<CIU_OUTPUT_SIZE_X_SHFT) | (OutHeight<<CIU_OUTPUT_SIZE_Y_SHFT);

    // 設定第0張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        #if (MULTI_STREAM_SUPPORT)
            #if SWAP_MULTI_STREAM_SUPPORT
            if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
            {
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
            }
            else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
            {
                #if HD_SWAP_MPSP_EN
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
                #else
                CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
                #endif
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
            {
                CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
            }
            #else
            CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
            CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
            
            #endif
        
        #else
            #if (CIU_SP_REP_MP)
            CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
            CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #else
            CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #endif
            
        #endif
    #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y);
    #endif
    }

#if (QUARD_MODE_DISP_SUPPORT)
    CIU_1_SP_STR_YADDR     = (unsigned int)PNBuf_Quad + 0;
    CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_Quad + PNBUF_SIZE_Y + 0);
#endif

    memset((u8*)ciuszString1, 0, sizeof(ciuszString1));
    memset((u8*)ciuszString1_SP, 0, sizeof(ciuszString1_SP));
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        #if HD_SWAP_MPSP_EN
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                              ciuOSDStr1,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              8,3,8+4,3+8);
        #else
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                              ciuOSDStr1,  CIU_OSD_CHARMAX,
                              ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
        #endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
    {
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                              ciuOSDStr1,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
    {
        #if SWAP_MULTI_STREAM_SUPPORT
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                              ciuOSDStr1,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              8,3,8+4,3+8);
        #else
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                              ciuOSDStr1,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
        #endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
    {
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                              ciuOSDStr1,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }
    else//under VGA
    {
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                              ciuOSDStr1,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }
    #if MULTI_STREAM_SUPPORT
        #if SWAP_MULTI_STREAM_SUPPORT
            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
            {
                GenerateCIU1_SP_OSD2Bits(CiuOverlayImg1_SP_Top, CiuOverlayImg1_SP_Bot,
                                        ciuOSDStr1,  CIU_OSD_CHARMAX,
                                        ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                                        8,3,8+4,3+8);
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
            {
                #if HD_SWAP_MPSP_EN
                GenerateCIU1_SP_OSD2Bits(CiuOverlayImg1_SP_Top, CiuOverlayImg1_SP_Bot,
                                        ciuOSDStr1,  CIU_OSD_CHARMAX,
                                        ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                                        8,3,8+4,3+8);
                #else
                GenerateCIU1_SP_OSD2Bits(CiuOverlayImg1_SP_Top, CiuOverlayImg1_SP_Bot,
                                        ciuOSDStr1,  CIU_OSD_CHARMAX,
                                        ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                                        8,3,8+4,3+8);
                #endif
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
            {
                GenerateCIU1_SP_OSD2Bits(CiuOverlayImg1_SP_Top, CiuOverlayImg1_SP_Bot,
                                        ciuOSDStr1,  CIU_OSD_CHARMAX,
                                        ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                                        8,3,8+4,3+8);
            }
        #else
            GenerateCIU1_SP_OSD2Bits(CiuOverlayImg1_SP_Top, CiuOverlayImg1_SP_Bot,
                                    ciuOSDStr1,  CIU_OSD_CHARMAX,
                                    ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                                    8,3,8+4,3+8);
        #endif
    #endif
    if(OSD_en)
    {
        CIU_1_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_1_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_1_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #if (CIU_DECIMATION_TEST || MULTI_STREAM_SUPPORT)
        CIU_1_SP_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_1_SP_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_1_SP_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #endif
        CIU_1_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                    CIU_DATA_OUT_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
 			        CIU_YUVDATALATCH_POS |
                #else
                    CIU_YUVDATALATCH_NEG |
                #endif
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else    
                    CIU_TESTIMG_DISA |
                #endif
                    CIU_MP_SEL_SCALAR |
                    CIU_SA_SEL_SENSOR |
                    CIU_SP_SEL_SENSOR |
                    CIU_DS_MODE_DISA |
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                     CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if HW_DEINTERLACE_CIU5_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                #if CIU_MASKAREA_TEST
                    CIU_MASKAREA_EN |
                #endif
                #if (MULTI_STREAM_SUPPORT)
                    NEWCIU1_CTR2 |
                    CIU_SP_OSD_ENA |    //cuz SP's image size hasn't defined, 20160613
                #endif
                    CIU_OSD_ENA;
    }
    else
    {
        CIU_1_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_1_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_1_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #if (CIU_DECIMATION_TEST || MULTI_STREAM_SUPPORT)
        CIU_1_SP_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_1_SP_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_1_SP_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #endif
        CIU_1_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                    CIU_DATA_OUT_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_YUVDATALATCH_NEG |
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else
                    CIU_TESTIMG_DISA |
                #endif
                    CIU_MP_SEL_SCALAR |
                    CIU_SA_SEL_SENSOR |
                    CIU_SP_SEL_SENSOR |
                    CIU_DS_MODE_DISA |
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                     CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if HW_DEINTERLACE_CIU5_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                #if (MULTI_STREAM_SUPPORT)
                    NEWCIU1_CTR2 |
                #endif
                    CIU_OSD_DISA;
    }
    #if(CHIP_OPTION == CHIP_A1018A)
        CIU_1_INTRPT |=CIU_INT_DATA_CMP ;
    #endif

    // 設定第1張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y + PNBUF_SIZE_Y + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y + PNBUF_SIZE_Y + PNBUF_SIZE_Y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        #if (MULTI_STREAM_SUPPORT)
            #if SWAP_MULTI_STREAM_SUPPORT
            if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
            {
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
            }
            else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
            {
                #if HD_SWAP_MPSP_EN
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
                #else
                CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
                #endif
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
            {
                CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
            }
            #else
            CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
            CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_1_pnbuf_min_size_y);
            
            #endif
        
        #else
            #if (CIU_SP_REP_MP)
            CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
            CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #else
            CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #endif
            
        #endif
    #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + PNBUF_SIZE_Y);
    #endif
    }
    #if CIU_MASKAREA_TEST
    CIU_1_MSAKAREA_Yaddr = (unsigned int)MaskAreaBuf;
    CIU_1_MSAKAREA_COLOR = 0x008080ff;
    #endif
}
#else
{
    u8  **PNBuf_sub;
    u8  err;
#if(FPGA_BOARD_A1018_SERIES)
    char ciuOSDStr1[CIU_OSD_CHARMAX+16]="CH_1DEFGHIJKLMNP123456789QWERTYU";
#elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) )
    char ciuOSDStr1[CIU_OSD_CHARMAX+16]="CH_1DEFGHIJKLMNP123456789QWERTYU";
#else
    char ciuOSDStr1[CIU_OSD_CHARMAX+16]="                                ";
#endif

    DEBUG_CIU("ciuPreviewInit_CH1(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n", mode, InWidth, InHeight, OutWidth, OutHeight, OutX, OutY, OSD_en, line_stride);

    //================//
#if DUAL_MODE_DISP_SUPPORT
    if((mode == SIUMODE_CAP_RREVIEW) || (mode == SIUMODE_PREVIEW_MENU))
    {
        ciu_1_pnbuf_size_y  = PNBUF_SIZE_Y;
    } else {
        line_stride         = line_stride * 2;
        ciu_1_pnbuf_size_y  = PNBUF_SIZE_Y * 2;
    }
#else
    ciu_1_pnbuf_size_y  = PNBUF_SIZE_Y;
    #if MULTI_STREAM_SUPPORT
    ciu_1_pnbuf_min_size_y = PNBUF_SP_SIZE_Y;
    #endif
#endif

    ciu_idufrmcnt_ch1   = 0;  //0;
    OSSemSet(ciuCapSemEvt_CH1, 0, &err);
    if(mode == SIUMODE_MPEGAVI)
        ciu_1_OpMode    = SIUMODE_MPEGAVI;
    else if(mode == SIUMODE_PREVIEW_MENU)
        ciu_1_OpMode    = SIUMODE_PREVIEW_MENU;
    else if(mode == SIUMODE_CAP_RREVIEW)
        ciu_1_OpMode    = SIUMODE_CAP_RREVIEW;
    else
        ciu_1_OpMode    = SIUMODE_PREVIEW;
    CIU_1_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);

#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
  #if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
    #if(TV_DECODER == WT8861)
		CIU_1_IMG_STR 		= (0x98<<CIU_IMG_H_STR_SHFT);
	#elif((TV_DECODER == TW9910) || (TV_DECODER == TW9900))
		CIU_1_IMG_STR 		|= (((720-InWidth)/2*2)<<CIU_IMG_H_STR_SHFT) ;
	#endif
	    if(sysTVinFormat == TV_IN_PAL)
	  	CIU_1_IMG_STR 		|=(0x19<<CIU_IMG_V_STR_SHFT);

  #else
  	CIU_1_IMG_STR		= (((720-InWidth)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
  #endif
#else
    CIU_1_IMG_STR		= 0;
#endif
    ciu_1_OutWidth      = OutWidth;
    ciu_1_OutHeight     = OutHeight;
    ciu_1_OutX          = OutX;
    ciu_1_OutY          = OutY;
    ciu_1_line_stride   = line_stride;
    switch(sysPIPMain)
    {
    case    PIP_MAIN_CH1:
        PNBuf_sub   = (u8**)PNBuf_sub2;
        break;
    default:
        PNBuf_sub   = (u8**)PNBuf_sub1;
        break;
    }

    if((mode == SIUMODE_CAP_RREVIEW) || (mode == SIUMODE_PREVIEW_MENU))
    {
        CIU_1_CTL1       =
                   #if(CIU1_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif(CIU1_OPTION==Sensor_CCIR601)
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                         CIU_OUT_FORMAT_422 |
                   #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((CIU1_OPTION==Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                   #if (CIU_SP_REP_MP)
                         CIU_MAINPATH_DIS |
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_1X1|
                   #endif
                         CIU_656DATALATCH_RIS;

        CIU_1_FRAME_STRIDE   = line_stride*2;
    }
    else
    {
        CIU_1_CTL1       =
                   #if(CIU1_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif(CIU1_OPTION==Sensor_CCIR601)
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                   #if ISUCIU_PREVIEW_PNOUT
                         CIU_OUT_FORMAT_420 |
                      #if (QUARD_MODE_DISP_SUPPORT || MULTI_STREAM_SUPPORT)
                         CIU_SP_OUT_FORMAT_420 |
                      #endif
                   #else
                         CIU_OUT_FORMAT_422 |
                      #if (QUARD_MODE_DISP_SUPPORT || MULTI_STREAM_SUPPORT)
                         CIU_SP_OUT_FORMAT_422 |
                      #endif
                   #endif

                   #if (QUARD_MODE_DISP_SUPPORT)
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_2X2 |
                   #endif

                   #if MULTI_STREAM_SUPPORT
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_4X4 |
                   #endif
                    
                   #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((CIU1_OPTION==Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                   #if CIU_DECIMATION_TEST
                         CIU_MAINPATH_DIS |
                   #endif
                   #if (CIU_SP_REP_MP)
                         CIU_MAINPATH_DIS |
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_1X1|
                   #endif
                         CIU_656DATALATCH_RIS;
#if ISUCIU_PREVIEW_PNOUT
        CIU_1_FRAME_STRIDE   = line_stride;
        #if (CIU_SP_REP_MP)
        CIU_2_SP_FRAME_STRIDE = line_stride;
        #endif
    #if (QUARD_MODE_DISP_SUPPORT)
        CIU_1_SP_FRAME_STRIDE= line_stride;
    #elif (MULTI_STREAM_SUPPORT)
        #if  SP_TEST
        CIU_1_SP_FRAME_STRIDE= line_stride;
        #else
        CIU_1_SP_FRAME_STRIDE= line_stride/4;
        #endif
    #endif
#else
        CIU_1_FRAME_STRIDE   = line_stride*2;
        #if (CIU_SP_REP_MP)
        CIU_2_SP_FRAME_STRIDE = line_stride;
        #endif
    #if (QUARD_MODE_DISP_SUPPORT)
        CIU_1_SP_FRAME_STRIDE= line_stride*2;
    #elif (MULTI_STREAM_SUPPORT)
        CIU_1_SP_FRAME_STRIDE= line_stride*2/4;
    #endif
#endif

    }

#if CIU1_SCUP_EN
    CIU_1_CTL1 |= (CIU_SCA_EN);
    if( (InWidth== OutWidth) && (InHeight==OutHeight) )
    {
        CIU_1_CTL1 |=CIU_SCA_SHAREBUF_EN | CIU_SCA_EN;
    }
#endif

    DEBUG_CIU("sysTVinFormat    = %d\n", sysTVinFormat);
#if IS_COMMAX_DOORPHONE
    if(((sysPIPMain == PIP_MAIN_CH2) || (sysVideoInCHsel == 1)) && (mode != SIUMODE_CAP_RREVIEW) && (mode != SIUMODE_PREVIEW_MENU))
    {
        CIU_1_CTL1 |= CIU_MODE_656_BOB;
        CIU_1_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }
  #if (TV_DECODER == WT8861)
	if((sysTVinFormat == TV_IN_PAL)&& (mode != SIUMODE_PREVIEW_MENU))
    {
        //CIU_1_CTL1 |= CIU_FIELD_POL_NEG;
        CIU_1_CTL1 |= CIU_FIELD_POL_NEG | CIU_656BOB_FSEL_BOTTOM;
    }
  #endif
#else
    if(sysPIPMain == PIP_MAIN_CH2)
    {
        CIU_1_CTL1 |= CIU_MODE_656_BOB;
        CIU_1_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }
#endif

    CIU_1_OutputSize    = ( OutWidth<<CIU_OUTPUT_SIZE_X_SHFT) | (OutHeight<<CIU_OUTPUT_SIZE_Y_SHFT);

    // 設定第0張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y);
    }
    else if(mode == SIUMODE_PREVIEW_MENU)
    {
        CIU_1_STR_YADDR     = (unsigned int)PKBuf0;
        CIU_1_STR_CbCrADDR  = (unsigned int)PKBuf0;
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
    #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y);
    #endif
    }

    #if (QUARD_MODE_DISP_SUPPORT)
       CIU_1_SP_STR_YADDR     = (unsigned int)PNBuf_Quad;
       CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_Quad + ciu_1_pnbuf_size_y);
    #endif
#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE
/*
    GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, ciuOSDStr1,
       					 	CIU_OSD_CHARMAX,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
        					6,20,6+24,19+2);
        					*/
    memset((u8*)szVideoOverlay2, 0, sizeof(szVideoOverlay2));
    uiDrawTimeOnVideoClip(1);
#else
    memset((u8*)ciuszString1, 0, sizeof(ciuszString1));
    memset((u8*)ciuszString1_SP, 0, sizeof(ciuszString1_SP));
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                                ciuOSDStr1,  CIU_OSD_CHARMAX,
                                ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                                0,0,0+8,0+4);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
    {
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                                ciuOSDStr1,  CIU_OSD_CHARMAX,
                                ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                                0,0,0+8,0+4);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
    {
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                                ciuOSDStr1,  CIU_OSD_CHARMAX,
                                ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                                0,0,0+8,0+4);
    }
    else    //under VGA
    {
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                                ciuOSDStr1,  CIU_OSD_CHARMAX,
                                ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                                0,0,0+8,0+4);
    }
    
    #if(CIU_DECIMATION_TEST == 1)
    GenerateCIU1_SP_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                            ciuOSDStr1,  CIU_OSD_CHARMAX,
                            ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                            0,0,0+8,0+4);
    #endif
    #if(MULTI_STREAM_SUPPORT == 1)
    GenerateCIU1_SP_OSD2Bits(CiuOverlayImg1_SP_Top, CiuOverlayImg1_SP_Bot,
                            ciuOSDStr1,  CIU_OSD_CHARMAX,
                            ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                            0,0,0+1,0+1);
    #endif

#endif

    if(OSD_en)
    {
        CIU_1_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_1_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_1_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #if (CIU_DECIMATION_TEST || MULTI_STREAM_SUPPORT)
        CIU_1_SP_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_1_SP_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_1_SP_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #endif

        CIU_1_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                     CIU_DATA_OUT_ENA |
                     CIU_VSYNC_ACT_LO |
                     CIU_HSYNC_ACT_LO |
                     CIU_INT_DISA_FIFO_OVERF |
                     CIU_INT_ENA_FRAME_END |
                #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
 			        CIU_YUVDATALATCH_POS |
 			    //#elif(CIU1_OPTION==Sensor_CCIR656) 
                    //CIU_YUVDATALATCH_POS |
                #else
 			        CIU_YUVDATALATCH_NEG |
 			    #endif
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else
                     CIU_TESTIMG_DISA |
                #endif
                     CIU_INT_DISA_FIELD_END |
                     CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                     CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if HW_DEINTERLACE_CIU1_ENA
                     CIU_INT_ENA_TOPEND |
                #endif
                #if (CIU_DECIMATION_TEST|| MULTI_STREAM_SUPPORT)
                     CIU_SP_OSD_ENA |
                #endif
                     CIU_OSD_ENA;

    }
    else
    {
        CIU_1_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                     CIU_DATA_OUT_ENA |
                     CIU_VSYNC_ACT_LO |
                     CIU_HSYNC_ACT_LO |
                     CIU_INT_DISA_FIFO_OVERF |
                     CIU_INT_ENA_FRAME_END |
                #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
                     CIU_YUVDATALATCH_POS |
                //#elif(CIU1_OPTION==Sensor_CCIR656) 
                    //CIU_YUVDATALATCH_POS |     
                #else
                     CIU_YUVDATALATCH_NEG |
                #endif
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else
                     CIU_TESTIMG_DISA |
                #endif
                     CIU_INT_DISA_FIELD_END |
                     CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                     CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if HW_DEINTERLACE_CIU1_ENA
                     CIU_INT_ENA_TOPEND |
                #endif
                     CIU_OSD_DISA;
    }
    #if(CHIP_OPTION == CHIP_A1018A)  //fix A1018A's bug
        CIU_1_INTRPT |=CIU_INT_DATA_CMP ;
    #endif
    
    // 設定第1張的位址
    if(mode ==SIUMODE_CAP_RREVIEW)
    {
        if((CIU_1_CTL1 & CIU_FIELD_POL_NEG) == 0)   // 若field有交換的話,第一張影像要輸出到第零張的位址
        {
        #if IS_COMMAX_DOORPHONE
            CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + (OutY * line_stride + OutX) * 2);
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y);
        #else
            CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y + (OutY * line_stride + OutX) * 2);
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y);
        #endif
        }
    }
    else if(mode == SIUMODE_PREVIEW_MENU)
    {
        CIU_1_STR_YADDR     = (unsigned int)PKBuf0;
        CIU_1_STR_CbCrADDR  = (unsigned int)PKBuf0;
    }
    else
    {
        /*
        if(sysPIPMain == PIP_MAIN_CH2)
        {
            CIU_1_STR_YADDR     = (unsigned int)PNBuf_sub2[1];
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub2[1]+ciu_1_pnbuf_size_y);
        } else {
            CIU_1_STR_YADDR     = (unsigned int)PNBuf_sub1[1];
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub1[1]+ciu_1_pnbuf_size_y);
        }
        */
      #if ISUCIU_PREVIEW_PNOUT
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
      #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y);
      #endif
    }
}
#endif
    return 1;
}

s32 ciuPreviewInit_CH2(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride)
{

    u8  **PNBuf_sub;
    u8  err;
 #if(FPGA_BOARD_A1018_SERIES)
    char ciuOSDStr2[CIU_OSD_CHARMAX+16]="CH_2DEFGHIJKLMNO123456789QWERTYU";
 #elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375))
    char ciuOSDStr2[CIU_OSD_CHARMAX+16]="CH_2DEFGHIJKLMNP123456789QWERTYU";
 #else
    char ciuOSDStr2[CIU_OSD_CHARMAX+16]="                                ";
 #endif
 #if (SWAP_MULTI_STREAM_SUPPORT || MULTI_STREAM_SUPPORT)
u32 sp_reso;
#endif
    #if SWAP_MULTI_STREAM_SUPPORT
        if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
        {
            sp_reso = CIU_SP_DECI_1X1;
            OutWidth = 640;
            OutHeight = 360;
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH2 MP Scaling dwon 640x360\n");
            #endif
        }
        else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352))
        {
          #if HD_SWAP_MPSP_EN
            sp_reso = CIU_SP_DECI_1X1;
            OutWidth = 640;
            OutHeight = 360;
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH2 MP Scaling dwon 640x360\n");
            #endif
          #else
            sp_reso = CIU_SP_DECI_2X2;
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH2 SP Downsample dwon 640x360 %x\n",sp_reso);
            #endif
          #endif
        }
        else if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
        {
            sp_reso = CIU_SP_DECI_2X2;
            DEBUG_CIU("ciuPreviewInit_CH2 SP Downsample dwon 320x240 %x\n",sp_reso);
        }
    #elif (MULTI_STREAM_SUPPORT)
        if (CIU_SP_RESO == SP_1x1)
            sp_reso = CIU_SP_DECI_1X1;
        else if (CIU_SP_RESO == SP_2x1)
            sp_reso = CIU_SP_DECI_2X1;
        else if (CIU_SP_RESO == SP_2x2)
            sp_reso = CIU_SP_DECI_2X2;
        else if (CIU_SP_RESO == SP_4x2)
            sp_reso = CIU_SP_DECI_4X2;
        else if (CIU_SP_RESO == SP_4x4)
            sp_reso = CIU_SP_DECI_4X4 ;
    #endif
    DEBUG_CIU("ciuPreviewInit_CH2(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n", mode, InWidth, InHeight, OutWidth, OutHeight, OutX, OutY, OSD_en, line_stride);
    //=================================//
    
#if DUAL_MODE_DISP_SUPPORT
    if((mode == SIUMODE_CAP_RREVIEW) || (mode == SIUMODE_PREVIEW_MENU))
    {
        ciu_2_pnbuf_size_y  = PNBUF_SIZE_Y;
    } else 
    {
        line_stride         = line_stride * 2;
        ciu_2_pnbuf_size_y  = PNBUF_SIZE_Y * 2;
    }
#else
    ciu_2_pnbuf_size_y  = PNBUF_SIZE_Y;
    #if (MULTI_STREAM_SUPPORT)
    ciu_2_pnbuf_min_size_y = PNBUF_SP_SIZE_Y;
    #endif
#endif

    ciu_idufrmcnt_ch2   = 0;  //0;
    OSSemSet(ciuCapSemEvt_CH2, 0, &err);
    if(mode == SIUMODE_MPEGAVI)
        ciu_2_OpMode = SIUMODE_MPEGAVI;
    else if(mode == SIUMODE_PREVIEW_MENU)
        ciu_2_OpMode = SIUMODE_PREVIEW_MENU;
    else if(mode == SIUMODE_CAP_RREVIEW)
        ciu_2_OpMode    = SIUMODE_CAP_RREVIEW;
    else
        ciu_2_OpMode    = SIUMODE_PREVIEW;

    CIU_2_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);

 #if( (CIU2_OPTION==Sensor_CCIR656) || (CIU2_OPTION==Sensor_CCIR601) )
  #if(TV_DECODER==WT8861)
    CIU_2_IMG_STR       = (((720-InWidth)+30)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
  #else
    CIU_2_IMG_STR       = (((720-InWidth)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
  #endif
 #else
    CIU_2_IMG_STR       = 0;
 #endif
    ciu_2_OutWidth      = OutWidth;
    ciu_2_OutHeight     = OutHeight;
    ciu_2_OutX          = OutX;
    ciu_2_OutY          = OutY;
    ciu_2_line_stride   = line_stride;
    switch(sysPIPMain)
    {
    case    PIP_MAIN_CH2:
        PNBuf_sub   = (u8**)PNBuf_sub1;
        break;
    default:
        PNBuf_sub   = (u8**)PNBuf_sub2;
        break;
    }

    if((mode ==SIUMODE_CAP_RREVIEW) || (mode == SIUMODE_PREVIEW_MENU))
    {
        CIU_2_CTL1       =
                         CIU_NEW_BURST16 |
                   #if(CIU2_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif(CIU2_OPTION==Sensor_CCIR601)
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                         CIU_OUT_FORMAT_422 |
                   #if((CIU2_OPTION==Sensor_OV7725_YUV601)||(CIU2_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((CIU1_OPTION==Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                   #if (CIU_SP_REP_MP)
                         CIU_MAINPATH_DIS |
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_1X1|
                   #endif
                         CIU_656DATALATCH_RIS;

        CIU_2_FRAME_STRIDE   = line_stride * 2;
    }
    else
    {
        CIU_2_CTL1       =
                         CIU_NEW_BURST16 |
                   #if(CIU2_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif(CIU2_OPTION==Sensor_CCIR601)
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                   #if ISUCIU_PREVIEW_PNOUT
                         CIU_OUT_FORMAT_420 |
                      #if (QUARD_MODE_DISP_SUPPORT || MULTI_STREAM_SUPPORT)
                         CIU_SP_OUT_FORMAT_420 |
                      #endif
                   #else
                         CIU_OUT_FORMAT_422 |
                      #if (QUARD_MODE_DISP_SUPPORT || MULTI_STREAM_SUPPORT)
                         CIU_SP_OUT_FORMAT_422 |
                      #endif
                   #endif

                   #if (QUARD_MODE_DISP_SUPPORT)
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_2X2 |
                   #endif

                   #if (MULTI_STREAM_SUPPORT)
                         CIU_SUBPATH_EN |
                         sp_reso|
                   #endif

                   #if((CIU2_OPTION==Sensor_OV7725_YUV601)||(CIU2_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((CIU1_OPTION==Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                   #if CIU_DECIMATION_TEST
                         CIU_MAINPATH_DIS |
                   #endif
                   #if (CIU_SP_REP_MP)
                         CIU_MAINPATH_DIS |
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_1X1|
                   #endif
                         CIU_656DATALATCH_RIS;
#if ISUCIU_PREVIEW_PNOUT
    #if SWAP_MULTI_STREAM_SUPPORT
        if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
        {
            CIU_2_FRAME_STRIDE   = 640;
        }

        else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
        {
            #if HD_SWAP_MPSP_EN
            CIU_2_FRAME_STRIDE   = 640;
            #else
            CIU_2_FRAME_STRIDE   = line_stride;
            #endif
        }
    #else
        CIU_2_FRAME_STRIDE   = line_stride;
        #if (CIU_SP_REP_MP)
        CIU_2_SP_FRAME_STRIDE = line_stride;
        #endif
    #endif
    #if (QUARD_MODE_DISP_SUPPORT)
        CIU_2_SP_FRAME_STRIDE= line_stride;
    #elif (MULTI_STREAM_SUPPORT)
        #if  SP_TEST
        CIU_2_SP_FRAME_STRIDE= line_stride;
        #else
            if (sp_reso == CIU_SP_DECI_1X1)
                CIU_2_SP_FRAME_STRIDE= line_stride;
            else if ( (sp_reso == CIU_SP_DECI_2X1) || (sp_reso == CIU_SP_DECI_2X2))
                CIU_2_SP_FRAME_STRIDE= line_stride / 2;
            else if ( (sp_reso == CIU_SP_DECI_4X2) || (sp_reso == CIU_SP_DECI_4X4))
                CIU_2_SP_FRAME_STRIDE= line_stride / 4;
        #endif
    #endif
#else
        CIU_2_FRAME_STRIDE   = line_stride * 2;
        #if (CIU_SP_REP_MP)
        CIU_2_SP_FRAME_STRIDE = line_stride;
        #endif
    #if (QUARD_MODE_DISP_SUPPORT)
        CIU_2_SP_FRAME_STRIDE= line_stride * 2;
    #elif (MULTI_STREAM_SUPPORT)
            if (sp_reso == CIU_SP_DECI_1X1)
                CIU_2_SP_FRAME_STRIDE= line_stride * 2;
            else if ( (sp_reso == CIU_SP_DECI_2X1) || (sp_reso == CIU_SP_DECI_2X2))
                CIU_2_SP_FRAME_STRIDE= line_stride * 2 /2;
            else if ( (sp_reso == CIU_SP_DECI_4X2) || (sp_reso == CIU_SP_DECI_4X4))
                CIU_2_SP_FRAME_STRIDE= line_stride * 2 /4;

    #endif
#endif
    }

#if CIU2_SCUP_EN
    CIU_2_CTL1 |= (CIU_SCA_EN);
    if( (InWidth== OutWidth) && (InHeight==OutHeight) )
    {
        CIU_2_CTL1 |=CIU_SCA_SHAREBUF_EN | CIU_SCA_EN;
    }
#endif

    DEBUG_CIU("sysTVinFormat    = %d\n", sysTVinFormat);

    if(sysPIPMain == PIP_MAIN_CH1)
    {
        CIU_2_CTL1 |= CIU_MODE_656_BOB;
        CIU_2_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }


    CIU_2_OutputSize    = ( OutWidth<<CIU_OUTPUT_SIZE_X_SHFT) | (OutHeight<<CIU_OUTPUT_SIZE_Y_SHFT);

    // 設定第0張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y);
    }
    /*else if(mode == SIUMODE_PREVIEW_MENU)  //Amon : 未知
    {
        CIU_2_STR_YADDR     = (unsigned int)PKBuf0;
        CIU_2_STR_CbCrADDR  = (unsigned int)PKBuf0;
    }*/
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        #if (MULTI_STREAM_SUPPORT)
            #if SWAP_MULTI_STREAM_SUPPORT
            if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
            {
                CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
            }
            else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
            {
                #if HD_SWAP_MPSP_EN
                CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
                #else
                CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
                #endif
            }
            #else
            CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
            CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
            CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
            
            #endif
        
        #else
            #if (CIU_SP_REP_MP)
            CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
            CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #else
            CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
            CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #endif
            
        #endif
    #else
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y);
    #endif
    }

    #if (QUARD_MODE_DISP_SUPPORT)
    CIU_2_SP_STR_YADDR     = (unsigned int)PNBuf_Quad + InWidth/2;
    CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_Quad + ciu_2_pnbuf_size_y + InWidth/2);
    #endif
    memset((u8*)ciuszString2, 0, sizeof(ciuszString2));
    memset((u8*)ciuszString2_SP, 0, sizeof(ciuszString2_SP));
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        #if HD_SWAP_MPSP_EN
        GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot,
                              ciuOSDStr2,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              8,3,8+4,3+8);
        #else
        GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot,
                              ciuOSDStr2,  CIU_OSD_CHARMAX,
                              ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
        #endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
    {
        #if SWAP_MULTI_STREAM_SUPPORT
        GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot,
                              ciuOSDStr2,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              8,3,8+4,3+8);
        #else
        GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot,
                              ciuOSDStr2,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
        #endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
    {
        GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot,
                              ciuOSDStr2,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }
    else//under VGA
    {
        GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot,
                              ciuOSDStr2,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }

    #if(CIU_DECIMATION_TEST == 1)
    GenerateCIU2_SP_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot,
                            ciuOSDStr2,  CIU_OSD_CHARMAX,
                            ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                            8,3,8+4,3+8);
    #endif
    #if MULTI_STREAM_SUPPORT
        #if SWAP_MULTI_STREAM_SUPPORT
            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
            {
                GenerateCIU2_SP_OSD2Bits(CiuOverlayImg2_SP_Top, CiuOverlayImg2_SP_Bot,
                                        ciuOSDStr2,  CIU_OSD_CHARMAX,
                                        ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                                        8,3,8+4,3+8);
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
            {
                #if HD_SWAP_MPSP_EN
                GenerateCIU2_SP_OSD2Bits(CiuOverlayImg2_SP_Top, CiuOverlayImg2_SP_Bot,
                                        ciuOSDStr2,  CIU_OSD_CHARMAX,
                                        ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                                        8,3,8+4,3+8);
                #else
                GenerateCIU2_SP_OSD2Bits(CiuOverlayImg2_SP_Top, CiuOverlayImg2_SP_Bot,
                                        ciuOSDStr2,  CIU_OSD_CHARMAX,
                                        ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                                        8,3,8+4,3+8);
                #endif
            }
        #else
            GenerateCIU2_SP_OSD2Bits(CiuOverlayImg2_SP_Top, CiuOverlayImg2_SP_Bot,
                                    ciuOSDStr2,  CIU_OSD_CHARMAX,
                                    ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                                    8,3,8+4,3+8);
        #endif
    #endif


    if(OSD_en)
    {
        CIU_2_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_2_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_2_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #if (CIU_DECIMATION_TEST || MULTI_STREAM_SUPPORT)
        CIU_2_SP_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_2_SP_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_2_SP_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #endif

        CIU_2_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                    CIU_DATA_OUT_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
                     CIU_YUVDATALATCH_POS |
                #else
                     CIU_YUVDATALATCH_NEG |
                #endif
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else
                    CIU_TESTIMG_DISA |
                #endif
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                    CIU_SPLITER_ENA | //enable CCIR4ch
                #endif
                #if CIU2_REPLACE_CIU1
     			    CIU_YUVDATALATCH_POS |
                #endif
                #if HW_DEINTERLACE_CIU2_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                #if (CIU_DECIMATION_TEST|| MULTI_STREAM_SUPPORT)
                     CIU_SP_OSD_ENA |
                #endif
                #if CIU_SP_REP_MP
                    CIU_OSD_DISA;
                #else
                    CIU_OSD_ENA;
                #endif
    }
    else
    {
        CIU_2_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                    CIU_DATA_OUT_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_YUVDATALATCH_NEG |
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else    
                    CIU_TESTIMG_DISA |
                #endif
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                    CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if CIU2_REPLACE_CIU1
     			    CIU_YUVDATALATCH_POS |
                #endif
                #if HW_DEINTERLACE_CIU2_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_DISA;
    }
    #if(CHIP_OPTION == CHIP_A1018A)
        CIU_2_INTRPT |=CIU_INT_DATA_CMP ;
    #endif

    // 設定第1張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        if((CIU_2_CTL1 & CIU_FIELD_POL_NEG) == 0)   // 若field有交換的話,第一張影像要輸出到第零張的位址
        {
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + ciu_2_pnbuf_size_y + (OutY * line_stride + OutX) * 2);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + ciu_2_pnbuf_size_y + ciu_2_pnbuf_size_y);

        }
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        #if (MULTI_STREAM_SUPPORT)
            #if SWAP_MULTI_STREAM_SUPPORT
            if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
            {
                CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
            }
            else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
            {
                #if HD_SWAP_MPSP_EN
                CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
                #else
                CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
                #endif
            }
            #else
            CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
            CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
            CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_2_pnbuf_min_size_y);
            
            #endif
        
        #else
            #if (CIU_SP_REP_MP)
            CIU_2_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
            CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #else
            CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
            CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #endif
            
        #endif
    #else
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + (OutY * line_stride + OutX) * 2);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y);
    #endif
    }
    return 1;
}

s32 ciuPreviewInit_CH3(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride)
{
    u8  **PNBuf_sub;
    u8  err;
 #if(FPGA_BOARD_A1018_SERIES)
    char ciuOSDStr3[CIU_OSD_CHARMAX+16]="CH_3DEFGHIJKLMNO123456789QWERTYU";
 #elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) )
    char ciuOSDStr3[CIU_OSD_CHARMAX+16]="CH_3DEFGHIJKLMNP123456789QWERTYU";
 #else
    char ciuOSDStr3[CIU_OSD_CHARMAX+16]="                                ";
 #endif
    //=================================//

    DEBUG_CIU("ciuPreviewInit_CH3(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n", mode, InWidth, InHeight, OutWidth, OutHeight, OutX, OutY, OSD_en, line_stride);

    ciu_idufrmcnt_ch3   = 0;  //0;
    OSSemSet(ciuCapSemEvt_CH3, 0, &err);
    if(mode == SIUMODE_MPEGAVI)
        ciu_3_OpMode = SIUMODE_MPEGAVI;
    else
        ciu_3_OpMode = SIUMODE_PREVIEW;

    CIU_3_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
 #if( (CIU3_OPTION==Sensor_CCIR656) || (CIU3_OPTION==Sensor_CCIR601) )
  #if(TV_DECODER==WT8861)
    CIU_3_IMG_STR       = (((720-InWidth)+30)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
  #else
    CIU_3_IMG_STR       = (((720-InWidth)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
  #endif
 #else
    CIU_3_IMG_STR       = 0;
 #endif
    ciu_3_OutWidth      = OutWidth;
    ciu_3_OutHeight     = OutHeight;
    ciu_3_OutX          = OutX;
    ciu_3_OutY          = OutY;
    ciu_3_line_stride   = line_stride;
    switch(sysPIPMain)
    {
    case    PIP_MAIN_CH2:
        PNBuf_sub   = (u8**)PNBuf_sub1;
        break;
    default:
        PNBuf_sub   = (u8**)PNBuf_sub3;
        break;
    }

    if(mode ==SIUMODE_CAP_RREVIEW)
    {
        CIU_3_CTL1       =
                   #if(CIU3_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif(CIU3_OPTION==Sensor_CCIR601)
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                         CIU_OUT_FORMAT_422 |
                   #if((CIU3_OPTION==Sensor_OV7725_YUV601)||(CIU3_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((CIU1_OPTION==Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                         CIU_656DATALATCH_RIS;


        CIU_3_FRAME_STRIDE   = line_stride * 2;
    }
    else
    {
        CIU_3_CTL1       =
                   #if(CIU3_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif(CIU3_OPTION==Sensor_CCIR601)
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                   #if ISUCIU_PREVIEW_PNOUT
                         CIU_OUT_FORMAT_420 |
                      #if (QUARD_MODE_DISP_SUPPORT)
                         CIU_SP_OUT_FORMAT_420 |
                      #endif
                   #else
                         CIU_OUT_FORMAT_422 |
                      #if (QUARD_MODE_DISP_SUPPORT)
                         CIU_SP_OUT_FORMAT_422 |
                      #endif
                   #endif

                   #if (QUARD_MODE_DISP_SUPPORT)
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_2X2 |
                   #endif

                   #if((CIU3_OPTION==Sensor_OV7725_YUV601)||(CIU3_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((CIU1_OPTION==Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                         CIU_656DATALATCH_RIS;
#if ISUCIU_PREVIEW_PNOUT
        CIU_3_FRAME_STRIDE   = line_stride;
     #if (QUARD_MODE_DISP_SUPPORT)
        CIU_3_SP_FRAME_STRIDE= line_stride;
     #endif
#else
        CIU_3_FRAME_STRIDE   = line_stride * 2;
     #if (QUARD_MODE_DISP_SUPPORT)
        CIU_3_SP_FRAME_STRIDE= line_stride * 2;
     #endif
#endif
    }

#if CIU3_SCUP_EN
    CIU_3_CTL1 |= (CIU_SCA_EN);
    if( (InWidth== OutWidth) && (InHeight==OutHeight) )
    {
        CIU_3_CTL1 |=CIU_SCA_SHAREBUF_EN| CIU_SCA_EN;
    }
#endif
#if IS_COMMAX_DOORPHONE
    if(((sysPIPMain == PIP_MAIN_CH1) || (sysVideoInCHsel == 2)) && (mode != SIUMODE_CAP_RREVIEW) && (mode != SIUMODE_PREVIEW_MENU))
    {
        CIU_3_CTL1 |= CIU_MODE_656_BOB;
        CIU_3_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }
#else
    if(sysPIPMain == PIP_MAIN_CH1)
    {
        CIU_3_CTL1 |= CIU_MODE_656_BOB;
        CIU_3_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }
#endif

    CIU_3_OutputSize    = ( OutWidth<<CIU_OUTPUT_SIZE_X_SHFT) | (OutHeight<<CIU_OUTPUT_SIZE_Y_SHFT);

    // 設定第0張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y + OutY * line_stride / 2 + OutX);
    #else
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y);
    #endif
    }

#if (QUARD_MODE_DISP_SUPPORT)
    CIU_3_SP_STR_YADDR     = (unsigned int)PNBuf_Quad + (InHeight/2)*line_stride ;
    CIU_3_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_Quad + PNBUF_SIZE_Y + (InHeight/2/2)*line_stride);
#endif

    memset((u8*)ciuszString3, 0, sizeof(ciuszString3));
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        GenerateCIU3_OSD2Bits(CiuOverlayImg3_Top, CiuOverlayImg3_Bot,
                              ciuOSDStr3,  CIU_OSD_CHARMAX,
                              ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
    {
        GenerateCIU3_OSD2Bits(CiuOverlayImg3_Top, CiuOverlayImg3_Bot,
                              ciuOSDStr3,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
    {
        GenerateCIU3_OSD2Bits(CiuOverlayImg3_Top, CiuOverlayImg3_Bot,
                              ciuOSDStr3,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }
    else    // VGA
    {
        GenerateCIU3_OSD2Bits(CiuOverlayImg3_Top, CiuOverlayImg3_Bot,
                              ciuOSDStr3,  CIU_OSD_CHARMAX,
                              ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }

    if(OSD_en)
    {
        CIU_3_OVL_IDXCOLOR_Y   = 0x00ffffff;
        CIU_3_OVL_IDXCOLOR_CB  = 0x00808080;
        CIU_3_OVL_IDXCOLOR_CR  = 0x00808080;

        CIU_3_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                    CIU_DATA_OUT_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_YUVDATALATCH_NEG |
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else    
                    CIU_TESTIMG_DISA |
                #endif
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                     CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if HW_DEINTERLACE_CIU3_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_ENA;
    }
    else
    {
        CIU_3_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                    CIU_DATA_OUT_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_YUVDATALATCH_NEG |
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else
                    CIU_TESTIMG_DISA |
                #endif
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                     CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if HW_DEINTERLACE_CIU3_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_DISA;
    }
    #if(CHIP_OPTION == CHIP_A1018A)
        CIU_3_INTRPT |=CIU_INT_DATA_CMP ;
    #endif

    // 設定第1張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y + PNBUF_SIZE_Y + (OutY * line_stride + OutX) * 2);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y + PNBUF_SIZE_Y + PNBUF_SIZE_Y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + PNBUF_SIZE_Y + OutY * line_stride / 2 + OutX);
    #else
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + (OutY * line_stride + OutX) * 2);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + PNBUF_SIZE_Y);
    #endif
    }
    return 1;
}

s32 ciuPreviewInit_CH4(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride)
{
    u8  **PNBuf_sub;
    u8  err;
 #if(FPGA_BOARD_A1018_SERIES)
    char ciuOSDStr4[CIU_OSD_CHARMAX+16]="CH_4DEFGHIJKLMNO123456789QWERTYU";
 #elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) )
    char ciuOSDStr4[CIU_OSD_CHARMAX+16]="CH_4DEFGHIJKLMNP123456789QWERTYU";
 #else
    char ciuOSDStr4[CIU_OSD_CHARMAX+16]="                                ";
 #endif
    //=================================//

    DEBUG_CIU("ciuPreviewInit_CH4(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n", mode, InWidth, InHeight, OutWidth, OutHeight, OutX, OutY, OSD_en, line_stride);

    ciu_idufrmcnt_ch4   = 0;  //0;
    OSSemSet(ciuCapSemEvt_CH4, 0, &err);
    if(mode == SIUMODE_MPEGAVI)
        ciu_4_OpMode = SIUMODE_MPEGAVI;
    else
        ciu_4_OpMode = SIUMODE_PREVIEW;

    CIU_4_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
 #if( (CIU4_OPTION==Sensor_CCIR656) || (CIU4_OPTION==Sensor_CCIR601) )
  #if(TV_DECODER==WT8861)
    CIU_4_IMG_STR       = (((720-InWidth)+30)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
  #else
    CIU_4_IMG_STR       = (((720-InWidth)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
  #endif
 #else
    CIU_4_IMG_STR       = 0;
 #endif
    ciu_4_OutWidth      = OutWidth;
    ciu_4_OutHeight     = OutHeight;
    ciu_4_OutX          = OutX;
    ciu_4_OutY          = OutY;
    ciu_4_line_stride   = line_stride;
    switch(sysPIPMain)
    {
    case    PIP_MAIN_CH2:
        PNBuf_sub   = (u8**)PNBuf_sub1;
        break;
    default:
        PNBuf_sub   = (u8**)PNBuf_sub4;
        break;
    }

    if(mode ==SIUMODE_CAP_RREVIEW)
    {
        CIU_4_CTL1       =
                   #if(CIU4_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif(CIU4_OPTION==Sensor_CCIR601)
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                         CIU_OUT_FORMAT_422 |
                   #if((CIU4_OPTION==Sensor_OV7725_YUV601)||(CIU4_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((CIU1_OPTION==Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                         CIU_656DATALATCH_RIS;


        CIU_4_FRAME_STRIDE   = line_stride * 2;
    }
    else
    {
        CIU_4_CTL1       =
                   #if(CIU4_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif(CIU4_OPTION==Sensor_CCIR601)
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                   #if ISUCIU_PREVIEW_PNOUT
                         CIU_OUT_FORMAT_420 |
                      #if (QUARD_MODE_DISP_SUPPORT)
                         CIU_SP_OUT_FORMAT_420 |
                      #endif
                   #else
                         CIU_OUT_FORMAT_422 |
                      #if (QUARD_MODE_DISP_SUPPORT)
                         CIU_SP_OUT_FORMAT_422 |
                      #endif
                   #endif

                   #if (QUARD_MODE_DISP_SUPPORT)
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_2X2 |
                   #endif

                   #if((CIU4_OPTION==Sensor_OV7725_YUV601)||(CIU4_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((CIU1_OPTION==Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                         CIU_656DATALATCH_RIS;
#if ISUCIU_PREVIEW_PNOUT
        CIU_4_FRAME_STRIDE   = line_stride;
     #if (QUARD_MODE_DISP_SUPPORT)
        CIU_4_SP_FRAME_STRIDE= line_stride;
     #endif
#else
        CIU_4_FRAME_STRIDE   = line_stride * 2;
     #if (QUARD_MODE_DISP_SUPPORT)
        CIU_4_SP_FRAME_STRIDE= line_stride * 2;
     #endif
#endif
    }

#if CIU4_SCUP_EN
    CIU_4_CTL1 |= (CIU_SCA_EN);
    if( (InWidth== OutWidth) && (InHeight==OutHeight) )
    {
        CIU_4_CTL1 |=CIU_SCA_SHAREBUF_EN| CIU_SCA_EN;
    }
#endif
#if IS_COMMAX_DOORPHONE
    if(((sysPIPMain == PIP_MAIN_CH1) || (sysVideoInCHsel == 2)) && (mode != SIUMODE_CAP_RREVIEW) && (mode != SIUMODE_PREVIEW_MENU))
    {
        CIU_4_CTL1 |= CIU_MODE_656_BOB;
        CIU_4_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }
#else
    if(sysPIPMain == PIP_MAIN_CH1)
    {
        CIU_4_CTL1 |= CIU_MODE_656_BOB;
        CIU_4_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }
#endif

    CIU_4_OutputSize    = ( OutWidth<<CIU_OUTPUT_SIZE_X_SHFT) | (OutHeight<<CIU_OUTPUT_SIZE_Y_SHFT);

    // 設定第0張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_4_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_4_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        CIU_4_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
        CIU_4_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y + OutY * line_stride / 2 + OutX);
    #else
        CIU_4_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_4_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y);
    #endif
    }

#if (QUARD_MODE_DISP_SUPPORT)
    CIU_4_SP_STR_YADDR     = (unsigned int)PNBuf_Quad + InWidth/2 + (InHeight/2)*line_stride;
    CIU_4_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_Quad + PNBUF_SIZE_Y + InWidth/2 + (InHeight/2/2)*line_stride);
#endif

    memset((u8*)ciuszString4, 0, sizeof(ciuszString4));
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        GenerateCIU4_OSD2Bits(CiuOverlayImg4_Top, CiuOverlayImg4_Bot,
                              ciuOSDStr4,  CIU_OSD_CHARMAX,
                              ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
    {
        GenerateCIU4_OSD2Bits(CiuOverlayImg4_Top, CiuOverlayImg4_Bot,
                              ciuOSDStr4,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
    {
        GenerateCIU4_OSD2Bits(CiuOverlayImg4_Top, CiuOverlayImg4_Bot,
                              ciuOSDStr4,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }
    else //Under VGA
    {
        GenerateCIU4_OSD2Bits(CiuOverlayImg4_Top, CiuOverlayImg4_Bot,
                              ciuOSDStr4,  CIU_OSD_CHARMAX,
                              ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    }

    if(OSD_en)
    {
        CIU_4_OVL_IDXCOLOR_Y   = 0x00ffffff;
        CIU_4_OVL_IDXCOLOR_CB  = 0x00808080;
        CIU_4_OVL_IDXCOLOR_CR  = 0x00808080;

        CIU_4_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                    CIU_DATA_OUT_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_YUVDATALATCH_NEG |
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else    
                    CIU_TESTIMG_DISA |
                #endif
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                     CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if HW_DEINTERLACE_CIU4_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_ENA;
    }
    else
    {
        CIU_4_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                    CIU_DATA_OUT_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_YUVDATALATCH_NEG |
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else
                    CIU_TESTIMG_DISA |
                #endif
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                     CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if HW_DEINTERLACE_CIU4_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_DISA;
    }
    #if(CHIP_OPTION == CHIP_A1018A)
        CIU_4_INTRPT |=CIU_INT_DATA_CMP ;
    #endif

    // 設定第1張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_4_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y + PNBUF_SIZE_Y + (OutY * line_stride + OutX) * 2);
        CIU_4_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y + PNBUF_SIZE_Y + PNBUF_SIZE_Y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        CIU_4_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
        CIU_4_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + PNBUF_SIZE_Y + OutY * line_stride / 2 + OutX);
    #else
        CIU_4_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + (OutY * line_stride + OutX) * 2);
        CIU_4_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + PNBUF_SIZE_Y);
    #endif
    }
    return 1;
}

s32 ciuPreviewInit_CH5(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride)
{
    u8  **PNBuf_sub;
    u8  err;
    u32 NEWCIU_CTR2=0;
 #if(FPGA_BOARD_A1018_SERIES)
    char ciuOSDStr5[CIU_OSD_CHARMAX+16]="CH_5DEFGHIJKLMNO123456789QWERTYU";
 #elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) )
    char ciuOSDStr5[CIU_OSD_CHARMAX+16]="CH_5DEFGHIJKLMNP123456789QWERTYU";
 #else
    char ciuOSDStr5[CIU_OSD_CHARMAX+16]="                                ";
 #endif
 #if (SWAP_MULTI_STREAM_SUPPORT || MULTI_STREAM_SUPPORT)
    u32 sp_reso;
 #endif
    //=================================//

 #if (USE_BUILD_IN_RTC == RTC_USE_EXT_RTC)
    RTC_DATE_TIME   localTime;
    u8  timetype;
    s8  DateOSD[10];
    s8  TimeOSD[11];  // 12-hour clock :11 24:8
 #endif

 #if (USE_BUILD_IN_RTC == RTC_USE_EXT_RTC)    
    RTC_Get_GMT_Time(&localTime);
    timetype = iconflag[UI_MENU_SETIDX_CH1_TSP_TYPE];
    uiTXOSDDateTimeFormat((timetype & 0x0F),DateOSD,localTime.year,localTime.month,localTime.day,TimeOSD,localTime.hour,localTime.min,localTime.sec);
    sprintf ((char *)ciuOSDStr5, "%s      %s",DateOSD , TimeOSD);
 #endif

	IISTime = 0;
	ciu_5_FrameTime = 0;
    #if SWAP_MULTI_STREAM_SUPPORT
        if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
        {
            sp_reso = CIU_SP_DECI_1X1;
            NEWCIU_CTR2 = ( CIU_SA_SEL_DOWNSAMPLE | CIU_SP_SEL_SENSOR | CIU_MP_SEL_SCALAR | CIU_DS_MODE_1_2 );
            OutWidth = 640;
            OutHeight = 360;
            //IsuScUpFIFODepth = 0x1d1d1111; //SP當大碼流頻寬會不夠 撥MP scalling頻寬給SP用 提高效能, move to sensor
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH5 MP Scaling dwon 640x360\n");
            #endif
        }
        else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) )
        {
          #if HD_SWAP_MPSP_EN
            sp_reso = CIU_SP_DECI_1X1;
            NEWCIU_CTR2 = ( CIU_SA_SEL_DOWNSAMPLE | CIU_SP_SEL_SENSOR | CIU_MP_SEL_SCALAR | CIU_DS_MODE_1_2 );
            OutWidth = 640;
            OutHeight = 360;
            //IsuScUpFIFODepth = 0x1d1d1111; //SP當大碼流頻寬會不夠 撥MP scalling頻寬給SP用 提高效能, move to sensor
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH5 MP Scaling dwon 640x360\n");
            #endif
          #else
            sp_reso = CIU_SP_DECI_2X2;
            NEWCIU_CTR2 = ( CIU_SA_SEL_SENSOR | CIU_SP_SEL_DOWNSAMPLE | CIU_MP_SEL_SCALAR | CIU_DS_MODE_1_2 );
            //IsuScUpFIFODepth = 0x1a1a1a1a; //default, move to sensor
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH5 SP Downsample dwon 640x360 %x\n",sp_reso);
            #endif
          #endif
        }
        else if (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
            sp_reso = CIU_SP_DECI_1X1;
            NEWCIU_CTR2 = ( CIU_SA_SEL_DOWNSAMPLE | CIU_SP_SEL_DOWNSAMPLE | CIU_MP_SEL_SCALAR | CIU_DS_MODE_1_2 );
            //IsuScUpFIFODepth = 0x1a1a1a1a; //default, move to sensor
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_CIU("ciuPreviewInit_CH5 MP SP Downsample dwon 640x360 %x\n",sp_reso);
            #endif
        }
        else if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
        {
            sp_reso = CIU_SP_DECI_1X1;
            DEBUG_CIU("ciuPreviewInit_CH5 SP Downsample dwon 320x240 %x\n",sp_reso);
        }
    #elif (MULTI_STREAM_SUPPORT)
        if (CIU_SP_RESO == SP_1x1)
            sp_reso = CIU_SP_DECI_1X1;
        else if (CIU_SP_RESO == SP_2x1)
            sp_reso = CIU_SP_DECI_2X1;
        else if (CIU_SP_RESO == SP_2x2)
            sp_reso = CIU_SP_DECI_2X2;
        else if (CIU_SP_RESO == SP_4x2)
            sp_reso = CIU_SP_DECI_4X2;
        else if (CIU_SP_RESO == SP_4x4)
            sp_reso = CIU_SP_DECI_4X4 ;
    #endif   
 #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))  

 #else
    DEBUG_CIU("ciuPreviewInit_CH5(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n", mode, InWidth, InHeight, OutWidth, OutHeight, OutX, OutY, OSD_en, line_stride);
 #endif
#if DUAL_MODE_DISP_SUPPORT
    if((mode == SIUMODE_CAP_RREVIEW) || (mode == SIUMODE_PREVIEW_MENU))
    {
        ciu_5_pnbuf_size_y  = PNBUF_SIZE_Y;
    } else 
    {
        line_stride         = line_stride * 2;
        ciu_5_pnbuf_size_y  = PNBUF_SIZE_Y * 2;
    }
#else
    ciu_5_pnbuf_size_y  = PNBUF_SIZE_Y;
    #if (MULTI_STREAM_SUPPORT)
    ciu_5_pnbuf_min_size_y = PNBUF_SP_SIZE_Y;
    #endif
#endif

    ciu_idufrmcnt_ch5   = 0;  //0;
    OSSemSet(ciuCapSemEvt_CH5, 0, &err);
    ciu_5_OpMode = mode;

    CIU_5_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
    if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
    {
        if ((NEWCIU_CTR2 & CIU_SA_SEL_DOWNSAMPLE)==CIU_SA_SEL_DOWNSAMPLE)
        {
            CIU_5_Scaling_InSize = ( (InWidth/2)<<CIU_INPUT_SIZE_X_SHFT) | ((InHeight/2)<<CIU_INPUT_SIZE_Y_SHFT);
            CIU_5_InputStride   =  (InWidth/2);
        }
        else
        {
            CIU_5_Scaling_InSize = ( (InWidth)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
            CIU_5_InputStride   =  InWidth;
        }
    }
    else
    {
        if ((NEWCIU_CTR2 & CIU_SA_SEL_DOWNSAMPLE)==CIU_SA_SEL_DOWNSAMPLE)
        {
            CIU_5_Scaling_InSize = ( (InWidth/2)<<CIU_INPUT_SIZE_X_SHFT) | ((InHeight/2)<<CIU_INPUT_SIZE_Y_SHFT);
            CIU_5_InputStride   =  (InWidth/2);
        }
        else
        {
            CIU_5_Scaling_InSize = ( (InWidth)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
            CIU_5_InputStride   =  InWidth;
        }
    }
 #if( (CIU5_OPTION==Sensor_CCIR656) || (CIU5_OPTION==Sensor_CCIR601) )
    #if(TV_DECODER==WT8861)
      CIU_5_IMG_STR       = (((720-InWidth)+30)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
    #else
      CIU_5_IMG_STR       = (((720-InWidth)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);
    #endif
 #else
    CIU_5_IMG_STR       = 0;
 #endif
    ciu_5_OutWidth      = OutWidth;
    ciu_5_OutHeight     = OutHeight;
    ciu_5_OutX          = OutX;
    ciu_5_OutY          = OutY;
    ciu_5_line_stride   = line_stride;
    
    PNBuf_sub   = (u8**)PNBuf_sub5;
    #if CIU_DROP_EN
        if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
        {
            CIU_5_DropReg = 0x89;
        }
        else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) )
        {
            CIU_5_DropReg = 0x7F;
        }
    #endif

    if(mode ==SIUMODE_CAP_RREVIEW)
    {
        CIU_5_CTL1       =
                         CIU_NEW_BURST16 |
                   #if(Sensor_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif ((Sensor_OPTION==Sensor_CCIR601) || (TV_DECODER == AHD6124B) )
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                         CIU_OUT_FORMAT_422 |
                   #if((Sensor_OPTION==Sensor_OV7725_YUV601)||(Sensor_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((Sensor_OPTION==Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                   #if (CIU_SP_REP_MP)
                         CIU_MAINPATH_DIS |
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_1X1 |
                   #endif
                         CIU_656DATALATCH_RIS;


        CIU_5_FRAME_STRIDE   = line_stride * 2;
    }
    else
    {
        CIU_5_CTL1       =
                         CIU_NEW_BURST16 |
                   #if(Sensor_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif ((Sensor_OPTION==Sensor_CCIR601) || (TV_DECODER == AHD6124B) )
                         CIU_MODE_601 |
                   #else
                         CIU_MODE_YUV |
                   #endif
                   #if ISUCIU_PREVIEW_PNOUT
                         CIU_OUT_FORMAT_420 |
                      #if (QUARD_MODE_DISP_SUPPORT || MULTI_STREAM_SUPPORT)
                         CIU_SP_OUT_FORMAT_420 |
                      #endif
                   #else
                         CIU_OUT_FORMAT_422 |
                      #if (QUARD_MODE_DISP_SUPPORT || MULTI_STREAM_SUPPORT)
                         CIU_SP_OUT_FORMAT_422 |
                      #endif
                   #endif

                   #if (QUARD_MODE_DISP_SUPPORT)
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_2X2 |
                   #endif
                   #if (MULTI_STREAM_SUPPORT)
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_1X1|
                         CIU_VSYNC_ACT_HI | // SP yuv map
                   #endif
                   #if((Sensor_OPTION==Sensor_OV7725_YUV601)||(Sensor_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif((Sensor_OPTION==Sensor_NT99141_YUV601) || (Sensor_OPTION == Sensor_NT99340_YUV601) || (Sensor_OPTION == Sensor_NT99230_YUV601))
                         CIU_YUVMAP_C9 |
    			   #else
    			         CIU_YUVMAP_36 |
    			   #endif
                         CIU_FIELD_POL_POS |
                         CIU_SCA_DIS |
                   #if (CHIP_OPTION  > CHIP_A1016A)
                         CIU_MP_BUR16_EN |
                         CIU_SP_BUR16_EN |
                   #endif
                   #if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
                         CIU_OSD_ADDR_MODE_FRAME |
                   #else
                         CIU_OSD_ADDR_MODE_LINEAR |
                   #endif
                   #if CIU_DECIMATION_TEST
                         CIU_MAINPATH_DIS |
                   #endif
                   #if (CIU_SP_REP_MP)
                         CIU_MAINPATH_DIS |
                         CIU_SUBPATH_EN |
                         CIU_SP_DECI_1X1|
                   #endif
                         CIU_656DATALATCH_RIS;
#if ISUCIU_PREVIEW_PNOUT
    #if SWAP_MULTI_STREAM_SUPPORT
        if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
        {
            CIU_5_FRAME_STRIDE   = 640;
        }
        else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720))
        {
            #if HD_SWAP_MPSP_EN
            CIU_5_FRAME_STRIDE   = 640;
            #else
            CIU_5_FRAME_STRIDE   = line_stride;
            #endif
        }
        else if (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
        {
            CIU_5_FRAME_STRIDE   = 640;
        }
        else
            CIU_5_FRAME_STRIDE   = line_stride;
    #else
        CIU_5_FRAME_STRIDE   = line_stride;
        #if (CIU_SP_REP_MP)
        CIU_5_SP_FRAME_STRIDE = line_stride;
        #endif
    #endif
    
    #if (QUARD_MODE_DISP_SUPPORT)
        CIU_5_SP_FRAME_STRIDE= line_stride;
    #elif (MULTI_STREAM_SUPPORT)
        #if  SP_TEST
        CIU_5_SP_FRAME_STRIDE= line_stride;
        #else
            if (sp_reso == CIU_SP_DECI_1X1)
                CIU_5_SP_FRAME_STRIDE= line_stride;
            else if ( (sp_reso == CIU_SP_DECI_2X1) || (sp_reso == CIU_SP_DECI_2X2))
                CIU_5_SP_FRAME_STRIDE= line_stride / 2;
            else if ( (sp_reso == CIU_SP_DECI_4X2) || (sp_reso == CIU_SP_DECI_4X4))
                CIU_5_SP_FRAME_STRIDE= line_stride / 4;
        #endif
    #endif
#else
        CIU_5_FRAME_STRIDE   = line_stride * 2;
     #if (QUARD_MODE_DISP_SUPPORT)
        CIU_5_SP_FRAME_STRIDE= line_stride * 2;
     #endif
        #if (CIU_SP_REP_MP)
        CIU_2_SP_FRAME_STRIDE = line_stride;
        #endif
#endif
    }
#if CIU_DROP_EN
    if ((AE_Flicker_50_60_sel==SENSOR_AE_FLICKER_60HZ) && (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) )
        CIU_5_CTL1 |= CIU_FRAMEDROP_EN;
    else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) )
        CIU_5_CTL1 |= CIU_FRAMEDROP_EN;
#endif

#if CIU5_SCUP_EN
    CIU_5_CTL1 |= (CIU_SCA_EN);
    if( (InWidth== OutWidth) && (InHeight==OutHeight) )
    {
        CIU_5_CTL1 |=CIU_SCA_SHAREBUF_EN| CIU_SCA_EN;
    }
#endif

    if(sysPIPMain == PIP_MAIN_CH1)
    {
        CIU_5_CTL1 |= CIU_MODE_656_BOB;
        CIU_5_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }

    CIU_5_OutputSize    = ( OutWidth<<CIU_OUTPUT_SIZE_X_SHFT) | (OutHeight<<CIU_OUTPUT_SIZE_Y_SHFT);

    // 設定第0張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        #if (MULTI_STREAM_SUPPORT)
            #if SWAP_MULTI_STREAM_SUPPORT
            if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
            {
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
            }
            else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
            {
                #if HD_SWAP_MPSP_EN
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
                #else
                CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
                #endif
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
            {
                CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
                CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
            }
            #else
            CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
            CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
            CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
            
            #endif
        
        #else
            #if (CIU_SP_REP_MP)
            CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
            CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #else
            CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
            CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #endif
            
        #endif
    #else
        CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y);
    #endif
    }

#if (QUARD_MODE_DISP_SUPPORT)
    CIU_5_SP_STR_YADDR     = (unsigned int)PNBuf_Quad + 0;
    CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_Quad + PNBUF_SIZE_Y + 0);
#endif

    memset((u8*)ciuszString5, 0, sizeof(ciuszString5));
    memset((u8*)ciuszString5_SP, 0, sizeof(ciuszString5_SP));
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        #if HD_SWAP_MPSP_EN
        GenerateCIU5_OSD2Bits(CiuOverlayImg5_Top, CiuOverlayImg5_Bot,
                              ciuOSDStr5,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              ciu_loc_x_vga,ciu_loc_y_vga,ciu_loc_x_vga+ciu_line1_str_num,ciu_loc_y_vga+ciu_line_num);
        #else
        GenerateCIU5_OSD2Bits(CiuOverlayImg5_Top, CiuOverlayImg5_Bot,
                              ciuOSDStr5,  CIU_OSD_CHARMAX,
                              ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                              ciu_loc_x_hd, ciu_loc_y_hd, ciu_loc_x_hd+ciu_line1_str_num,ciu_loc_y_hd+ciu_line_num);
        #endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
    {
        GenerateCIU5_OSD2Bits(CiuOverlayImg5_Top, CiuOverlayImg5_Bot,
                              ciuOSDStr5,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              ciu_loc_x_vga,ciu_loc_y_vga,ciu_loc_x_vga+ciu_line1_str_num,ciu_loc_y_vga+ciu_line_num);
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
    {
        #if SWAP_MULTI_STREAM_SUPPORT
        GenerateCIU5_OSD2Bits(CiuOverlayImg5_Top, CiuOverlayImg5_Bot,
                              ciuOSDStr5,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              ciu_loc_x_vga,ciu_loc_y_vga,ciu_loc_x_vga+ciu_line1_str_num,ciu_loc_y_vga+ciu_line_num);
        #else
        GenerateCIU5_OSD2Bits(CiuOverlayImg5_Top, CiuOverlayImg5_Bot,
                              ciuOSDStr5,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              ciu_loc_x_fhd, ciu_loc_y_fhd, ciu_loc_x_fhd+ciu_line1_str_num,ciu_loc_y_fhd+ciu_line_num);
        #endif
    }
    else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
    {
        GenerateCIU5_OSD2Bits(CiuOverlayImg5_Top, CiuOverlayImg5_Bot,
                              ciuOSDStr5,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              ciu_loc_x_fhd, ciu_loc_y_fhd, ciu_loc_x_fhd+ciu_line1_str_num,ciu_loc_y_fhd+ciu_line_num);
    }
    else//under VGA
    {
        GenerateCIU5_OSD2Bits(CiuOverlayImg5_Top, CiuOverlayImg5_Bot,
                              ciuOSDStr5,  CIU_OSD_CHARMAX,
                              ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                              ciu_loc_x_vga,ciu_loc_y_vga,ciu_loc_x_vga+ciu_line1_str_num,ciu_loc_y_vga+ciu_line_num);
    }
    #if MULTI_STREAM_SUPPORT
        #if SWAP_MULTI_STREAM_SUPPORT
            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
            {
                GenerateCIU5_SP_OSD2Bits(CiuOverlayImg5_SP_Top, CiuOverlayImg5_SP_Bot,
                                        ciuOSDStr5,  CIU_OSD_CHARMAX,
                                        ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                                            ciu_loc_x_fhd, ciu_loc_y_fhd, ciu_loc_x_fhd+ciu_line1_str_num,ciu_loc_y_fhd+ciu_line_num);
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
            {
                #if HD_SWAP_MPSP_EN
                GenerateCIU5_SP_OSD2Bits(CiuOverlayImg5_SP_Top, CiuOverlayImg5_SP_Bot,
                                        ciuOSDStr5,  CIU_OSD_CHARMAX,
                                        ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                                            ciu_loc_x_hd, ciu_loc_y_hd, ciu_loc_x_hd+ciu_line1_str_num,ciu_loc_y_hd+ciu_line_num);
                #else
                GenerateCIU5_SP_OSD2Bits(CiuOverlayImg5_SP_Top, CiuOverlayImg5_SP_Bot,
                                        ciuOSDStr5,  CIU_OSD_CHARMAX,
                                        ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                                            ciu_loc_x_vga,ciu_loc_y_vga,ciu_loc_x_vga+ciu_line1_str_num,ciu_loc_y_vga+ciu_line_num);
                #endif
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
            {
                GenerateCIU5_SP_OSD2Bits(CiuOverlayImg5_SP_Top, CiuOverlayImg5_SP_Bot,
                                        ciuOSDStr5,  CIU_OSD_CHARMAX,
                                        ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                                            ciu_loc_x_vga,ciu_loc_y_vga,ciu_loc_x_vga+ciu_line1_str_num,ciu_loc_y_vga+ciu_line_num);
            }
        #else
            GenerateCIU5_SP_OSD2Bits(CiuOverlayImg5_SP_Top, CiuOverlayImg5_SP_Bot,
                                    ciuOSDStr5,  CIU_OSD_CHARMAX,
                                    ASCII_SMALL_FONT_WIDTH, ASCII_SMALL_FONT_HEIGHT,
                                    ciu_loc_x_vga,ciu_loc_y_vga,ciu_loc_x_vga+ciu_line1_str_num,ciu_loc_y_vga+ciu_line_num);
        #endif
    #endif
    if(OSD_en)
    {
        CIU_5_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_5_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_5_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #if (CIU_DECIMATION_TEST || MULTI_STREAM_SUPPORT)
        CIU_5_SP_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_5_SP_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_5_SP_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #endif
        CIU_5_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                    CIU_DATA_OUT_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
#if (Sensor_OPTION == Sensor_XC7021_SC2133 || Sensor_OPTION == Sensor_XC7021_GC2023 || Sensor_OPTION == Sensor_ZN220_YUV601) //避花屏 丟棄frame when sp overflow 
                    CIU_INT_ENA_FIFO_OVERF |
#else
                    CIU_INT_DISA_FIFO_OVERF |
#endif
                    CIU_INT_ENA_FRAME_END |
                    CIU_YUVDATALATCH_NEG |
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else    
                    CIU_TESTIMG_DISA |
                #endif
                    CIU_MP_SEL_SCALAR |
                    CIU_SA_SEL_SENSOR |
                    CIU_SP_SEL_SENSOR |
                    CIU_DS_MODE_DISA |
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                     CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if HW_DEINTERLACE_CIU5_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                #if CIU_MASKAREA_TEST
                    CIU_MASKAREA_EN |
                #endif
                #if (MULTI_STREAM_SUPPORT)
                    NEWCIU_CTR2 |
                    CIU_SP_OSD_ENA |    //cuz SP's image size hasn't defined, 20160613
                #endif
                    CIU_OSD_ENA;
    }
    else
    {
        CIU_5_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_5_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_5_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #if (CIU_DECIMATION_TEST || MULTI_STREAM_SUPPORT)
        CIU_5_SP_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_5_SP_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_5_SP_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #endif
        CIU_5_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                    CIU_DATA_OUT_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
#if (Sensor_OPTION == Sensor_XC7021_SC2133 || Sensor_OPTION == Sensor_XC7021_GC2023 || Sensor_OPTION == Sensor_ZN220_YUV601) //避花屏 丟棄frame when sp overflow 
                    CIU_INT_ENA_FIFO_OVERF |
#else
                    CIU_INT_DISA_FIFO_OVERF |
#endif
                    CIU_INT_ENA_FRAME_END |
                    CIU_YUVDATALATCH_NEG |
                #if CIU_PATTERN_TEST
                     CIU_TESTIMG_01 |
                #else
                    CIU_TESTIMG_DISA |
                #endif
                    CIU_MP_SEL_SCALAR |
                    CIU_SA_SEL_SENSOR |
                    CIU_SP_SEL_SENSOR |
                    CIU_DS_MODE_DISA |
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if CIU_SPLITER
                     CIU_SPLITER_ENA | //enable CCIR4ch
                #endif     
                #if HW_DEINTERLACE_CIU5_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                #if (MULTI_STREAM_SUPPORT)
                    NEWCIU_CTR2 |
                #endif
                    CIU_OSD_DISA;
    }
    #if(CHIP_OPTION == CHIP_A1018A)
        CIU_5_INTRPT |=CIU_INT_DATA_CMP ;
    #endif

    // 設定第1張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y + PNBUF_SIZE_Y + (OutY * line_stride + OutX) * 2);
        CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + PNBUF_SIZE_Y + PNBUF_SIZE_Y + PNBUF_SIZE_Y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        #if (MULTI_STREAM_SUPPORT)
            #if SWAP_MULTI_STREAM_SUPPORT
            if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
            {
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
            }
            else if ((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480))
            {
                #if HD_SWAP_MPSP_EN
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
                #else
                CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
                #endif
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352)
            {
                CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
                CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
                CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
                CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
            }
            #else
            CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
            CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX + VIDEODISPBUF_OFFSET);
            CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + OutY * line_stride / 2 + OutX + VIDEODISPBUF_OFFSET + ciu_5_pnbuf_min_size_y);
            
            #endif
        
        #else
            #if (CIU_SP_REP_MP)
            CIU_5_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
            CIU_5_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #else
            CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
            CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_5_pnbuf_size_y + OutY * line_stride / 2 + OutX);
            #endif
            
        #endif
    #else
        CIU_5_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + (OutY * line_stride + OutX) * 2);
        CIU_5_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + PNBUF_SIZE_Y);
    #endif
    }
    #if CIU_MASKAREA_TEST
    CIU_5_MSAKAREA_Yaddr = (unsigned int)MaskAreaBuf;
    CIU_5_MSAKAREA_COLOR = 0x008080ff;
    #endif
    return 1;
}

void ciuPreviewInMenu_CH1(u32 InWidth, u32 InHeight, u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride)
{
    ciuPreviewInit_CH1(SIUMODE_PREVIEW_MENU, InWidth, InHeight, OutWidth, OutHeight, OutX, OutY, CIU1_OSD_EN, line_stride);
}

//Lucian: RF TX 必須使用CIU1,放大倍率固定為x2.由於HW限制,HD輸出無法做Zoom-in
s32 rfiuciu1ZoomInx2(int OnOff,int Xpos,int Ypos)
{

   CIU_1_CTL1 &= (~CIU_SCA_SHAREBUF_EN);

#if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )  //For TV-in
   if(OnOff)
   {
      CIU_1_InputSize     = ( (640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2


      if(Xpos+320>640)
          Xpos=320;
      if(Ypos+120>240)
          Ypos=120;

      CIU_1_IMG_STR       = ((((720-640)/2+Xpos)*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);

   }
   else
   {
      CIU_1_InputSize     = ( (640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x1

      CIU_1_IMG_STR       = (((720-640)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);

   }

#else //For Sensor-in
   if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
   {
       //Cannot use zoom function //
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
   {
       //Cannot use zoom function //
   }
   else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480))
   {
   #if(CHIP_OPTION == CHIP_A1018B)
       //Cannot use zoom function //
   #else
       if(OnOff)
       {
           if(Xpos+640>1280)
              Xpos=640;
           if(Ypos+360>720)
              Ypos=360;

           CIU_1_InputSize     = ( (1280/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((720/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_1_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
       }
       else
       {
           CIU_1_InputSize     = ( (1280*2)<<CIU_INPUT_SIZE_X_SHFT) | ((720)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom off
           CIU_1_IMG_STR=0;
       }
   #endif
   }
   else //VGA/QVGA
   {
       if(OnOff)
       {
           if(Xpos+320>640)
              Xpos=320;
           if(Ypos+240>480)
              Ypos=240;

           CIU_1_InputSize     = ( (640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2

           CIU_1_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
       }
       else
       {
           CIU_1_InputSize     = ( (640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_1_IMG_STR=0;
       }
   }
#endif
    return 1;
}

s32 rfiuciu1ZoomInx3(int OnOff,int Xpos,int Ypos)
{

   CIU_1_CTL1 &= (~CIU_SCA_SHAREBUF_EN);

#if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )  //For TV-in
   if(OnOff)
   {
      CIU_1_InputSize     = ( (640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2


      if(Xpos+320>640)
          Xpos=320;
      if(Ypos+120>240)
          Ypos=120;

      CIU_1_IMG_STR       = ((((720-640)/2+Xpos)*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);

   }
   else
   {
      CIU_1_InputSize     = ( (640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x1

      CIU_1_IMG_STR       = (((720-640)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);

   }

#else //For Sensor-in
   if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
   {
       //Cannot use zoom function //
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
   {
       //Cannot use zoom function //
   }
   else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480))
   {
   #if(CHIP_OPTION == CHIP_A1018B)
       //Cannot use zoom function //
   #else
       if(OnOff)
       {
           if(Xpos+416>1280)
              Xpos=1280-416;
           if(Ypos+240>720)
              Ypos=720-240;

           CIU_1_InputSize     = ( (416*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_1_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
       }
       else
       {
           CIU_1_InputSize     = ( (1280*2)<<CIU_INPUT_SIZE_X_SHFT) | ((720)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom off
           CIU_1_IMG_STR=0;
       }
   #endif
   }
   else //VGA/QVGA
   {
       if(OnOff)
       {
           if(Xpos+320>640)
              Xpos=320;
           if(Ypos+240>480)
              Ypos=240;

           CIU_1_InputSize     = ( (640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2

           CIU_1_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
       }
       else
       {
           CIU_1_InputSize     = ( (640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_1_IMG_STR=0;
       }
   }
#endif
    return 1;

}

s32 rfiuciu2ZoomInx2(int OnOff,int Xpos,int Ypos)
{

   CIU_2_CTL1 &= (~CIU_SCA_SHAREBUF_EN);

#if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )  //For TV-in
   if(OnOff)
   {
      CIU_2_InputSize     = ( (640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2


      if(Xpos+320>640)
          Xpos=320;
      if(Ypos+120>240)
          Ypos=120;

      CIU_2_IMG_STR       = ((((720-640)/2+Xpos)*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);

   }
   else
   {
      CIU_2_InputSize     = ( (640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x1

      CIU_2_IMG_STR       = (((720-640)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);

   }

#else //For Sensor-in
   if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
   {
       //Cannot use zoom function //
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
   {
       //Cannot use zoom function //
   }
   else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480))
   {
   #if(CHIP_OPTION == CHIP_A1018B)
       //Cannot use zoom function //
   #else
       if(OnOff)
       {
           if(Xpos+640>1280)
              Xpos=640;
           if(Ypos+360>720)
              Ypos=360;

           CIU_2_InputSize     = ( (1280/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((720/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_2_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
       }
       else
       {
           CIU_2_InputSize     = ( (1280*2)<<CIU_INPUT_SIZE_X_SHFT) | ((720)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom off
           CIU_2_IMG_STR=0;
       }
   #endif
   }
   else //VGA/QVGA
   {
       if(OnOff)
       {
           if(Xpos+320>640)
              Xpos=320;
           if(Ypos+240>480)
              Ypos=240;

           CIU_2_InputSize     = ( (640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2

           CIU_2_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
       }
       else
       {
           CIU_2_InputSize     = ( (640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_2_IMG_STR=0;
       }
   }
#endif
    return 1;
}

s32 rfiuciu2ZoomInx3(int OnOff,int Xpos,int Ypos)
{

   CIU_2_CTL1 &= (~CIU_SCA_SHAREBUF_EN);

#if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )  //For TV-in
   if(OnOff)
   {
      CIU_2_InputSize     = ( (640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2


      if(Xpos+320>640)
          Xpos=320;
      if(Ypos+120>240)
          Ypos=120;

      CIU_2_IMG_STR       = ((((720-640)/2+Xpos)*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);

   }
   else
   {
      CIU_2_InputSize     = ( (640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x1

      CIU_2_IMG_STR       = (((720-640)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);

   }

#else //For Sensor-in
   if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
   {
       //Cannot use zoom function //
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
   {
       //Cannot use zoom function //
   }
   else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480))
   {
   #if(CHIP_OPTION == CHIP_A1018B)
       //Cannot use zoom function //
   #else
       if(OnOff)
       {
           if(Xpos+416>1280)
              Xpos=1280-416;
           if(Ypos+240>720)
              Ypos=720-240;

           CIU_2_InputSize     = ( (416*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_2_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
       }
       else
       {
           CIU_2_InputSize     = ( (1280*2)<<CIU_INPUT_SIZE_X_SHFT) | ((720)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom off
           CIU_2_IMG_STR=0;
       }
   #endif
   }
   else //VGA/QVGA
   {
       if(OnOff)
       {
           if(Xpos+320>640)
              Xpos=320;
           if(Ypos+240>480)
              Ypos=240;

           CIU_2_InputSize     = ( (640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2

           CIU_2_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
       }
       else
       {
           CIU_2_InputSize     = ( (640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_2_IMG_STR=0;
       }
   }
#endif
    return 1;
}

s32 ciu1ScUpZoom(s32 zoomFactor)
{
   u32 W,H;

   getPreviewZoomSize(zoomFactor,&W,&H);
   CIU_1_CTL1 &= (~CIU_SCA_SHAREBUF_EN);

   CIU_1_InputSize     = ( (W*2)<<CIU_INPUT_SIZE_X_SHFT) | (H<<CIU_INPUT_SIZE_Y_SHFT);

   if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) )
   {
       if( (1280 >= W) && (720 >= H) )
           CIU_1_IMG_STR       = (((1280-W)/2)<<CIU_IMG_H_STR_SHFT) | (((720-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_1_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
   {
       if( (640 >= W) && (480 >= H) )
           CIU_1_IMG_STR       = (((640-W)/2)<<CIU_IMG_H_STR_SHFT) | (((480-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_1_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
   {
       if( (1920 >= W) && (1080 >= H) )
           CIU_1_IMG_STR       = (((1920-W)/2)<<CIU_IMG_H_STR_SHFT) | (((1080-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_1_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
   {
       if( (1600 >= W) && (896 >= H) )
           CIU_1_IMG_STR       = (((1600-W)/2)<<CIU_IMG_H_STR_SHFT) | (((896-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_1_IMG_STR = 0;
   }
   else
   {
       CIU_1_IMG_STR = 0;
   }
   return 1;
}

s32 ciu2ScUpZoom(s32 zoomFactor)
{
   u32 W,H;

   getPreviewZoomSize(zoomFactor,&W,&H);
   CIU_2_CTL1 &= (~CIU_SCA_SHAREBUF_EN);

   CIU_2_InputSize     = ( (W*2)<<CIU_INPUT_SIZE_X_SHFT) | (H<<CIU_INPUT_SIZE_Y_SHFT);

   if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) )
   {
       if( (1280 >= W) && (720 >= H) )
           CIU_2_IMG_STR       = (((1280-W)/2)<<CIU_IMG_H_STR_SHFT) | (((720-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_2_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
   {
       if( (640 >= W) && (480 >= H) )
           CIU_2_IMG_STR       = (((640-W)/2)<<CIU_IMG_H_STR_SHFT) | (((480-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_2_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
   {
       if( (1920 >= W) && (1080 >= H) )
           CIU_2_IMG_STR       = (((1920-W)/2)<<CIU_IMG_H_STR_SHFT) | (((1080-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_2_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
   {
       if( (1600 >= W) && (896 >= H) )
           CIU_2_IMG_STR       = (((1600-W)/2)<<CIU_IMG_H_STR_SHFT) | (((896-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_2_IMG_STR = 0;
   }
   else
   {
       CIU_2_IMG_STR = 0;
   }
   return 1;
}

s32 ciu3ScUpZoom(s32 zoomFactor)
{
   u32 W,H;

   getPreviewZoomSize(zoomFactor,&W,&H);
   CIU_3_CTL1 &= (~CIU_SCA_SHAREBUF_EN);

   CIU_3_InputSize     = ( (W*2)<<CIU_INPUT_SIZE_X_SHFT) | (H<<CIU_INPUT_SIZE_Y_SHFT);


   if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) )
   {
       if( (1280 >= W) && (720 >= H) )
           CIU_3_IMG_STR       = (((1280-W)/2)<<CIU_IMG_H_STR_SHFT) | (((720-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_3_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
   {
       if( (640 >= W) && (480 >= H) )
           CIU_3_IMG_STR       = (((640-W)/2)<<CIU_IMG_H_STR_SHFT) | (((480-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_3_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
   {
       if( (1920 >= W) && (1080 >= H) )
           CIU_3_IMG_STR       = (((1920-W)/2)<<CIU_IMG_H_STR_SHFT) | (((1080-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_3_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
   {
       if( (1600 >= W) && (896 >= H) )
           CIU_3_IMG_STR       = (((1600-W)/2)<<CIU_IMG_H_STR_SHFT) | (((896-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_3_IMG_STR = 0;
   }
   else
   {
       CIU_3_IMG_STR = 0;
   }
   return 1;
}

s32 ciu4ScUpZoom(s32 zoomFactor)
{
   u32 W,H;

   getPreviewZoomSize(zoomFactor,&W,&H);
   CIU_4_CTL1 &= (~CIU_SCA_SHAREBUF_EN);

   CIU_4_InputSize     = ( (W*2)<<CIU_INPUT_SIZE_X_SHFT) | (H<<CIU_INPUT_SIZE_Y_SHFT);

   if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) )
   {
       if( (1280 >= W) && (720 >= H) )
           CIU_4_IMG_STR       = (((1280-W)/2)<<CIU_IMG_H_STR_SHFT) | (((720-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_4_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
   {
       if( (640 >= W) && (480 >= H) )
           CIU_4_IMG_STR       = (((640-W)/2)<<CIU_IMG_H_STR_SHFT) | (((480-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_4_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
   {
       if( (1920 >= W) && (1080 >= H) )
           CIU_4_IMG_STR       = (((1920-W)/2)<<CIU_IMG_H_STR_SHFT) | (((1080-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_4_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
   {
       if( (1600 >= W) && (896 >= H) )
           CIU_4_IMG_STR       = (((1600-W)/2)<<CIU_IMG_H_STR_SHFT) | (((896-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_4_IMG_STR = 0;
   }
   else
   {
       CIU_4_IMG_STR = 0;
   }
   return 1;
}

s32 ciu5ScUpZoom(s32 zoomFactor)
{
   u32 W,H;

   getPreviewZoomSize(zoomFactor,&W,&H);
   CIU_5_CTL1 &= (~CIU_SCA_SHAREBUF_EN);

   CIU_5_InputSize     = ( (W*2)<<CIU_INPUT_SIZE_X_SHFT) | (H<<CIU_INPUT_SIZE_Y_SHFT);

   if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) )
   {
       if( (1280 >= W) && (720 >= H) )
           CIU_5_IMG_STR       = (((1280-W)/2)<<CIU_IMG_H_STR_SHFT) | (((720-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_5_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
   {
       if( (640 >= W) && (480 >= H) )
           CIU_5_IMG_STR       = (((640-W)/2)<<CIU_IMG_H_STR_SHFT) | (((480-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_5_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
   {
       if( (1920 >= W) && (1080 >= H) )
           CIU_5_IMG_STR       = (((1920-W)/2)<<CIU_IMG_H_STR_SHFT) | (((1080-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_5_IMG_STR = 0;
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
   {
       if( (1600 >= W) && (896 >= H) )
           CIU_5_IMG_STR       = (((1600-W)/2)<<CIU_IMG_H_STR_SHFT) | (((896-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_5_IMG_STR = 0;
   }
   else
   {
       CIU_5_IMG_STR = 0;
   }
   return 1;
}

void CiuOSD_OnOff(u8 onoff)
{
#if CIUOSDONOFF
    if (onoff)
        CIU_5_CTL2 |=  (CIU_SP_OSD_ENA |CIU_OSD_ENA);
    else
        CIU_5_CTL2 &= ~(CIU_SP_OSD_ENA |CIU_OSD_ENA);
#endif
}

u8 getCiuOSD_OnOff(void)
{
#if CIUOSDONOFF
    if ((CIU_5_CTL2 & (CIU_SP_OSD_ENA |CIU_OSD_ENA)) == 0x3000)
        return 1;
    else
        return 0;
#endif
}
#if(CIU_OSD_MODE_SEL == CIU_OSD_FRAME_ADDR_MODE)
/*
  Application Notes:
  Lucian:
          1.(FontW/4) * (Font_END_X-Font_STR_X) must be multiples of 32.
          2. OSD 水平起始點必須是4 的倍數,且大於等於4
          3. OSD 水平結束點, 必須小於IMG_WIDTH
*/
s32 GenerateCIU1_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    #if OSD_GREY_UNDERLAY
    int x;
    #endif
    //--------------------//

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSD1 windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSD1 Buffer overflow!\n");
        return -1;
    }

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;
    
#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSD1 width is not multiples of 32 \n");
	}
#else
    if ((char_W*FontW*2/8) != osd_stride)
    {
        DEBUG_CIU("Error! CIU OSD1 stride is not four times as great as width \n");
    }
#endif
#if ( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
    CIU_1_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_1_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_1_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;

#else
    CIU_1_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_1_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH)<<CIU_OVL_WEY_SHFT);
    CIU_1_OVL_MAXBYTECNTLIM= FontW * FontH *  FontSize;
#endif

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }

    CIU_1_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_1_OVL_BOTIADDR     = (unsigned int)OSDImg_top+osd_stride;
#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
    CIU_1_OVL_STRIDE = ( (osd_stride*2)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#else
    CIU_1_OVL_STRIDE = ( (osd_stride)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#endif

#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString1[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH*j + FontW_Byte*i;

                 #if NEW_CIU1_EN
                 {
                     #if SWAP_MULTI_STREAM_SUPPORT
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_SFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                         {
                            #if HD_SWAP_MPSP_EN
                            source  = (u8*)ASCII_SFont[index];
                            #else
                            source  = (u8*)ASCII_Font[index];
                            #endif
                         }
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #else
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_XFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                            source  = (u8*)ASCII_Font[index];
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #endif
                 }
                 #else
                 {
                     if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                        source  = (u8*)ASCII_Font[index];
                     else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
                        source  = (u8*)ASCII_XFont[index];
                     else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
                        source  = (u8*)ASCII_XFont[index];
                     else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_320x240)
                        source  = (u8*)ASCII_SFont[index];
                     else   // VGA
                        source  = (u8*)ASCII_Font[index];
                 }
                 #endif
                     for(y=0;y<FontH;y++)
                     {
                        #if OSD_GREY_UNDERLAY
                        for(x=0; x<FontW_Byte; x++) //改成灰底
                        {
                            if( (*(source+x) & 0xc0) == 0 )
                                *(source+x) |= 0x80;
                            if( (*(source+x) & 0x30) == 0 )
                                *(source+x) |= 0x20;
                            if( (*(source+x) & 0x0c) == 0 )
                                *(source+x) |= 0x08;
                            if( (*(source+x) & 0x03) == 0 )
                                *(source+x) |= 0x02;
                        }
                        #endif
                        memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                        osd1 += osd_stride;
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString1, szString);
#endif
    return  1;
}




/*
  Application Notes:
  Lucian:  (FontW/4) * (Font_END_X-Font_STR_X) must be multiples of 32.
*/
s32 GenerateCIU2_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    #if OSD_GREY_UNDERLAY
    int x;
    #endif
    //--------------------//

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSD2 windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSD2 Buffer overflow!\n");
        return -1;
    }
    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;

#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSD2 width is not multiples of 32 \n");
	}
#else
    if ((char_W*FontW*2/8) != osd_stride)
    {
//        DEBUG_CIU("osd_stride %d (char_W*FontW*2/8/4) %d\n",osd_stride,(char_W*FontW*2/8/4));
        DEBUG_CIU("Error! CIU OSD2 stride is not four times as great as width \n");
    }
#endif
#if( (CIU2_OPTION==Sensor_CCIR656) || (CIU2_OPTION==Sensor_CCIR601) )
    CIU_2_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_2_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_2_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;
#else

    CIU_2_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_2_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH)<<CIU_OVL_WEY_SHFT);
    CIU_2_OVL_MAXBYTECNTLIM= FontW * FontH *  FontSize;
#endif
    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }


    CIU_2_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_2_OVL_BOTIADDR     = (unsigned int)OSDImg_top+osd_stride;
#if( (CIU2_OPTION==Sensor_CCIR656) || (CIU2_OPTION==Sensor_CCIR601) )
    CIU_2_OVL_STRIDE = ( (osd_stride*2)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#else
    CIU_2_OVL_STRIDE = ( (osd_stride)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#endif

#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString2[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH*j + FontW_Byte*i;

                     #if SWAP_MULTI_STREAM_SUPPORT
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_SFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                         {
                            #if HD_SWAP_MPSP_EN
                            source  = (u8*)ASCII_SFont[index];
                            #else
                            source  = (u8*)ASCII_Font[index];
                            #endif
                         }
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #else
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_XFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                            source  = (u8*)ASCII_Font[index];
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #endif
                     for(y=0;y<FontH;y++)
                     {
                        #if OSD_GREY_UNDERLAY
                        for(x=0; x<FontW_Byte; x++) //改成灰底
                        {
                            if( (*(source+x) & 0xc0) == 0 )
                                *(source+x) |= 0x80;
                            if( (*(source+x) & 0x30) == 0 )
                                *(source+x) |= 0x20;
                            if( (*(source+x) & 0x0c) == 0 )
                                *(source+x) |= 0x08;
                            if( (*(source+x) & 0x03) == 0 )
                                *(source+x) |= 0x02;
                        }
                        #endif
                        memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                        osd1 += osd_stride;
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString2, szString);
#endif
    return  1;
}

/*
  Application Notes:
  Lucian:  (FontW/4) * (Font_END_X-Font_STR_X) must be multiples of 32.
*/
s32 GenerateCIU3_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    //--------------------//

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSD3 windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSD3 Buffer overflow!\n");
        return -1;
    }
    
    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;

#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSD3 width is not multiples of 32 \n");
	}
#else
    if ((char_W*FontW*2/8) != osd_stride)
    {
        DEBUG_CIU("Error! CIU OSD3 stride is not four times as great as width \n");
    }
#endif
#if( (CIU3_OPTION==Sensor_CCIR656) || (CIU3_OPTION==Sensor_CCIR601) )
    CIU_3_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_3_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_3_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;
#else
    CIU_3_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_3_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH)<<CIU_OVL_WEY_SHFT);
    CIU_3_OVL_MAXBYTECNTLIM= FontW * FontH *  FontSize;
#endif

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }


    CIU_3_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_3_OVL_BOTIADDR     = (unsigned int)OSDImg_top+osd_stride;
#if( (CIU3_OPTION==Sensor_CCIR656) || (CIU3_OPTION==Sensor_CCIR601) )
    CIU_3_OVL_STRIDE = ( (osd_stride*2)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#else
    CIU_3_OVL_STRIDE = ( (osd_stride)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#endif

#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString3[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH*j + FontW_Byte*i;

                 #if NEW_CIU1_EN
                 {
                     #if SWAP_MULTI_STREAM_SUPPORT
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_SFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                         {
                            #if HD_SWAP_MPSP_EN
                            source  = (u8*)ASCII_SFont[index];
                            #else
                            source  = (u8*)ASCII_Font[index];
                            #endif
                         }
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #else
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_XFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                            source  = (u8*)ASCII_Font[index];
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #endif
                 }
                 #else
                 {
                     if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                        source  = (u8*)ASCII_Font[index];
                     else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
                        source  = (u8*)ASCII_XFont[index];
                     else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
                        source  = (u8*)ASCII_XFont[index];
                     else   // VGA
                        source  = (u8*)ASCII_Font[index];
                 }
                 #endif
                     for(y=0;y<FontH;y++)
                     {
                        memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                        osd1 += osd_stride;
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString3, szString);
#endif
    return  1;
}

s32 GenerateCIU4_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    //--------------------//

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSD4 windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSD4 Buffer overflow!\n");
        return -1;
    }
    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;

#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSD4 width is not multiples of 32 \n");
	}
#else
    if ((char_W*FontW*2/8) != osd_stride)
    {
        DEBUG_CIU("Error! CIU OSD4 stride is not four times as great as width \n");
    }
#endif
#if( (CIU4_OPTION==Sensor_CCIR656) || (CIU4_OPTION==Sensor_CCIR601) )
    CIU_4_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_4_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_4_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;
#else
    CIU_4_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_4_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH)<<CIU_OVL_WEY_SHFT);
    CIU_4_OVL_MAXBYTECNTLIM= FontW * FontH *  FontSize;
#endif

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }


    CIU_4_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_4_OVL_BOTIADDR     = (unsigned int)OSDImg_top+osd_stride;
#if( (CIU4_OPTION==Sensor_CCIR656) || (CIU4_OPTION==Sensor_CCIR601) )
    CIU_4_OVL_STRIDE = ( (osd_stride*2)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#else
    CIU_4_OVL_STRIDE = ( (osd_stride)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#endif

#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString4[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH*j + FontW_Byte*i;

                     if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                        source  = (u8*)ASCII_Font[index];
                     else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072)
                        source  = (u8*)ASCII_XFont[index];
                     else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896)
                        source  = (u8*)ASCII_XFont[index];
                     else   // VGA
                        source  = (u8*)ASCII_Font[index];
                 
                     for(y=0;y<FontH;y++)
                     {
                        memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                        osd1 += osd_stride;
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString4, szString);
#endif
    return  1;
}

s32 GenerateCIU5_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    #if OSD_GREY_UNDERLAY
    int x;
    #endif
    //--------------------//

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSD5 windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSD5 Buffer overflow!\n");
        return -1;
    }
    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;

#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSD5 width is not multiples of 32 \n");
	}
#else
    if ((char_W*FontW*2/8) != osd_stride)
    {
        DEBUG_CIU("Error! CIU OSD5 stride is not four times as great as width \n");
    }
#endif
#if( (CIU5_OPTION==Sensor_CCIR656) || (CIU5_OPTION==Sensor_CCIR601) )
    CIU_5_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_5_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_5_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;
#else
    CIU_5_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_5_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH)<<CIU_OVL_WEY_SHFT);
    CIU_5_OVL_MAXBYTECNTLIM= FontW * FontH *  FontSize;
#endif

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }

//    FontW_Byte = FontW *2 / 8;
//    osd1    = (u8*)OSDImg_top;
//    osd2    = (u8*)OSDImg_bot;
//    osd_stride=FontW_Byte*char_W;

    CIU_5_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_5_OVL_BOTIADDR     = (unsigned int)OSDImg_top+osd_stride;
#if( (CIU5_OPTION==Sensor_CCIR656) || (CIU5_OPTION==Sensor_CCIR601) )
    CIU_5_OVL_STRIDE = ( (osd_stride*2)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#else
    CIU_5_OVL_STRIDE = ( (osd_stride)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#endif

#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString5[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH*j + FontW_Byte*i;

                     #if SWAP_MULTI_STREAM_SUPPORT
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_SFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                         {
                            #if HD_SWAP_MPSP_EN
                            source  = (u8*)ASCII_SFont[index];
                            #else
                            source  = (u8*)ASCII_Font[index];
                            #endif
                         }
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #else
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_XFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                            source  = (u8*)ASCII_Font[index];
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #endif
                     for(y=0;y<FontH;y++)
                     {
                        #if OSD_GREY_UNDERLAY
                        for(x=0; x<FontW_Byte; x++) //改成灰底
                        {
                            if( (*(source+x) & 0xc0) == 0 )
                                *(source+x) |= 0x80;
                            if( (*(source+x) & 0x30) == 0 )
                                *(source+x) |= 0x20;
                            if( (*(source+x) & 0x0c) == 0 )
                                *(source+x) |= 0x08;
                            if( (*(source+x) & 0x03) == 0 )
                                *(source+x) |= 0x02;
                        }
                        #endif
                        memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                        osd1 += osd_stride;
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString5, szString);
#endif
    return  1;
}

s32 GenerateCIU1_SP_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    #if OSD_GREY_UNDERLAY
    int x;
    #endif
    //--------------------//

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSDsp windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSDsp Buffer overflow!\n");
        return -1;
    }
    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;

#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU SP OSD1 width is not multiples of 32 \n");
	}
#else
    if ((char_W*FontW*2/8) != osd_stride)
    {
        DEBUG_CIU("Error! CIU SP OSD1 stride is not four times as great as width \n");
    }
#endif
#if ( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
    CIU_1_SP_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_1_SP_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_1_SP_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;

#else
    CIU_1_SP_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_1_SP_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH)<<CIU_OVL_WEY_SHFT);
    CIU_1_SP_OVL_MAXBYTECNTLIM= FontW * FontH *  FontSize;
#endif

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }

    CIU_1_SP_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_1_SP_OVL_BOTIADDR     = (unsigned int)OSDImg_top+osd_stride;
#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
    CIU_1_SP_OVL_STRIDE = ( (osd_stride*2)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#else
    CIU_1_SP_OVL_STRIDE = ( (osd_stride)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#endif

#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString1_SP[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH*j + FontW_Byte*i;

                 #if 1
                     source  = (u8*)ASCII_Font[index];
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
                 #endif
                     for(y=0;y<FontH;y++)
                     {
                        #if OSD_GREY_UNDERLAY
                        for(x=0; x<FontW_Byte; x++) //改成灰底
                        {
                            if( (*(source+x) & 0xc0) == 0 )
                                *(source+x) |= 0x80;
                            if( (*(source+x) & 0x30) == 0 )
                                *(source+x) |= 0x20;
                            if( (*(source+x) & 0x0c) == 0 )
                                *(source+x) |= 0x08;
                            if( (*(source+x) & 0x03) == 0 )
                                *(source+x) |= 0x02;
                        }
                        #endif
                        memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                        osd1 += osd_stride;
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString1_SP, szString);
#endif
    return  1;
}

s32 GenerateCIU2_SP_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    #if OSD_GREY_UNDERLAY
    int x;
    #endif
    //--------------------//

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSDsp windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSDsp Buffer overflow!\n");
        return -1;
    }
    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;

#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU SP OSD2 width is not multiples of 32 \n");
	}
#else
    if ((char_W*FontW*2/8) != osd_stride)
    {
        DEBUG_CIU("Error! CIU SP OSD2 stride is not four times as great as width \n");
    }
#endif
#if ( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
    CIU_2_SP_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_2_SP_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_2_SP_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;

#else
    CIU_2_SP_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_2_SP_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH)<<CIU_OVL_WEY_SHFT);
    CIU_2_SP_OVL_MAXBYTECNTLIM= FontW * FontH *  FontSize;
#endif

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }

    CIU_2_SP_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_2_SP_OVL_BOTIADDR     = (unsigned int)OSDImg_top+osd_stride;
#if( (CIU2_OPTION==Sensor_CCIR656) || (CIU2_OPTION==Sensor_CCIR601) )
    CIU_2_SP_OVL_STRIDE = ( (osd_stride*2)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#else
    CIU_2_SP_OVL_STRIDE = ( (osd_stride)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#endif

#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString2_SP[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH*j + FontW_Byte*i;

                 #if 1
                     #if SWAP_MULTI_STREAM_SUPPORT
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_XFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                         {
                            #if HD_SWAP_MPSP_EN
                            source  = (u8*)ASCII_Font[index];
                            #else
                            source  = (u8*)ASCII_SFont[index];
                            #endif
                         }
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #else
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_XFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                            source  = (u8*)ASCII_Font[index];
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #endif
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
                 #endif
                     for(y=0;y<FontH;y++)
                     {
                        #if OSD_GREY_UNDERLAY
                        for(x=0; x<FontW_Byte; x++) //改成灰底
                        {
                            if( (*(source+x) & 0xc0) == 0 )
                                *(source+x) |= 0x80;
                            if( (*(source+x) & 0x30) == 0 )
                                *(source+x) |= 0x20;
                            if( (*(source+x) & 0x0c) == 0 )
                                *(source+x) |= 0x08;
                            if( (*(source+x) & 0x03) == 0 )
                                *(source+x) |= 0x02;
                        }
                        #endif
                        memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                        osd1 += osd_stride;
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString2_SP, szString);
#endif
    return  1;
}

s32 GenerateCIU5_SP_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    #if OSD_GREY_UNDERLAY
    int x;
    #endif

    //--------------------//

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSDsp windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSDsp Buffer overflow!\n");
        return -1;
    }
    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;

#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU SP OSD5 width is not multiples of 32 \n");
	}
#else
    if ((char_W*FontW*2/8) != osd_stride)
    {
        DEBUG_CIU("Error! CIU SP OSD5 stride is not four times as great as width \n");
    }
#endif
#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSDsp width is not multiples of 32 \n");
	}
#endif
#if ( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
    CIU_5_SP_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_5_SP_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_5_SP_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;

#else
    CIU_5_SP_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_5_SP_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH)<<CIU_OVL_WEY_SHFT);
    CIU_5_SP_OVL_MAXBYTECNTLIM= FontW * FontH *  FontSize;
#endif

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }

    CIU_5_SP_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_5_SP_OVL_BOTIADDR     = (unsigned int)OSDImg_top+osd_stride;
#if( (CIU2_OPTION==Sensor_CCIR656) || (CIU2_OPTION==Sensor_CCIR601) )
    CIU_5_SP_OVL_STRIDE = ( (osd_stride*2)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#else
    CIU_5_SP_OVL_STRIDE = ( (osd_stride)<<CIU_OVL_STRIDE_SHFT) | ((char_W*FontW*2/8/4)<<CIU_OVL_WIDTH_SHFT);
#endif

#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString2_SP[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH*j + FontW_Byte*i;

                 #if 1
                     #if SWAP_MULTI_STREAM_SUPPORT
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_XFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                         {
                            #if HD_SWAP_MPSP_EN
                            source  = (u8*)ASCII_Font[index];
                            #else
                            source  = (u8*)ASCII_SFont[index];
                            #endif
                         }
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #else
                         if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896))
                            source  = (u8*)ASCII_XFont[index];
                         else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                            source  = (u8*)ASCII_Font[index];
                         else   // VGA
                            source  = (u8*)ASCII_SFont[index];
                     #endif
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
                 #endif
                     for(y=0;y<FontH;y++)
                     {
                        #if OSD_GREY_UNDERLAY
                        for(x=0; x<FontW_Byte; x++) //改成灰底
                        {
                            if( (*(source+x) & 0xc0) == 0 )
                                *(source+x) |= 0x80;
                            if( (*(source+x) & 0x30) == 0 )
                                *(source+x) |= 0x20;
                            if( (*(source+x) & 0x0c) == 0 )
                                *(source+x) |= 0x08;
                            if( (*(source+x) & 0x03) == 0 )
                                *(source+x) |= 0x02;
                        }
                        #endif
                        memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                        osd1 += osd_stride;
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString5_SP, szString);
#endif
    return  1;
}

#else

/*
  Application Notes:
  Lucian:  (FontW/4) * (Font_END_X-Font_STR_X) must be multiples of 32.
*/
s32 GenerateCIU1_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    //--------------------//
    CIU_1_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_1_OVL_BOTIADDR     = (unsigned int)OSDImg_bot;

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSD1 windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSD1 Buffer overflow!\n");
        return -1;
    }



    CIU_1_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_1_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_1_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;
#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString1[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH/2*j + FontW_Byte*i;
                     osd2=(u8*)OSDImg_bot + osd_stride*FontH/2*j + FontW_Byte*i;

                 #if 1
                     source  = (u8*)ASCII_Font[index];
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
                 #endif
                     for(y=0;y<FontH;y++)
                     {
                        if( (y & 0x01)==0 )
                        {
                          memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                          osd1 += osd_stride;
                        }
                        else
                        {
                          memcpy(osd2, source, sizeof(u8) * FontW_Byte); //bottom field
                          osd2 += osd_stride;
                        }
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString1, szString);
#endif
    return  1;
}

/*
  Application Notes:
  Lucian:  (FontW/4) * (Font_END_X-Font_STR_X) must be multiples of 32.
*/
s32 GenerateCIU2_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    //--------------------//
    CIU_2_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_2_OVL_BOTIADDR     = (unsigned int)OSDImg_bot;

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSD2 windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSD2 Buffer overflow!\n");
        return -1;
    }



    CIU_2_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_2_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_2_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;
#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString2[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH/2*j + FontW_Byte*i;
                     osd2=(u8*)OSDImg_bot + osd_stride*FontH/2*j + FontW_Byte*i;

                 #if 1
                     source  = (u8*)ASCII_Font[index];
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
                 #endif
                     for(y=0;y<FontH;y++)
                     {
                        if( (y & 0x01)==0 )
                        {
                          memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                          osd1 += osd_stride;
                        }
                        else
                        {
                          memcpy(osd2, source, sizeof(u8) * FontW_Byte); //bottom field
                          osd2 += osd_stride;
                        }
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString2, szString);
#endif
    return  1;
}


/*
  Application Notes:
  Lucian:  (FontW/4) * (Font_END_X-Font_STR_X) must be multiples of 32.
*/
s32 GenerateCIU3_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    //--------------------//
    CIU_3_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_3_OVL_BOTIADDR     = (unsigned int)OSDImg_bot;

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSD3 windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSD3 Buffer overflow!\n");
        return -1;
    }



    CIU_3_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_3_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_3_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;
#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString3[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH/2*j + FontW_Byte*i;
                     osd2=(u8*)OSDImg_bot + osd_stride*FontH/2*j + FontW_Byte*i;

                 #if 1
                     source  = (u8*)ASCII_Font[index];
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
                 #endif
                     for(y=0;y<FontH;y++)
                     {
                        if( (y & 0x01)==0 )
                        {
                          memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                          osd1 += osd_stride;
                        }
                        else
                        {
                          memcpy(osd2, source, sizeof(u8) * FontW_Byte); //bottom field
                          osd2 += osd_stride;
                        }
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString3, szString);
#endif

    return  1;
}

s32 GenerateCIU4_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    //--------------------//
    CIU_4_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_4_OVL_BOTIADDR     = (unsigned int)OSDImg_bot;

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSD3 windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSD3 Buffer overflow!\n");
        return -1;
    }

    CIU_4_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_4_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_4_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;
#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString4[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH/2*j + FontW_Byte*i;
                     osd2=(u8*)OSDImg_bot + osd_stride*FontH/2*j + FontW_Byte*i;

                 #if 1
                     source  = (u8*)ASCII_Font[index];
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
                 #endif
                     for(y=0;y<FontH;y++)
                     {
                        if( (y & 0x01)==0 )
                        {
                          memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                          osd1 += osd_stride;
                        }
                        else
                        {
                          memcpy(osd2, source, sizeof(u8) * FontW_Byte); //bottom field
                          osd2 += osd_stride;
                        }
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString4, szString);
#endif

    return  1;
}

s32 GenerateCIU5_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
    int strlength, i,j,k ,index, FontW_Byte, y;
    int osd_stride;
    u8  *source, *osd1, *osd2;
    int FontSize;
    int char_W,char_H;
    //--------------------//
    CIU_5_OVL_TOPIADDR     = (unsigned int)OSDImg_top;
    CIU_5_OVL_BOTIADDR     = (unsigned int)OSDImg_bot;

    char_W  = (Font_END_X-Font_STR_X);
    char_H  = (Font_END_Y-Font_STR_Y);

    if( (char_W<1) || (char_H<1))
    {
        DEBUG_CIU("OSD5 windown is invalid!\n");
        return -1;
    }

    FontSize   = char_W*char_H;
    strlength  = strlen(szString);
    if(FontSize > MaxStrLen)
    {
        DEBUG_CIU("OSD5 Buffer overflow!\n");
        return -1;
    }

    CIU_5_OVL_WSP          = ((Font_STR_X * FontW+4)<<CIU_OVL_WSX_SHFT) | ((Font_STR_Y * FontH/2)<<CIU_OVL_WSY_SHFT); //field coor.
    CIU_5_OVL_WEP          = ((Font_END_X * FontW+4)<<CIU_OVL_WEX_SHFT) | ((Font_END_Y * FontH/2)<<CIU_OVL_WEY_SHFT);
    CIU_5_OVL_MAXBYTECNTLIM= FontW * FontH/2 *  FontSize;

    if(FontSize > strlength)
    {
       for(i=strlength;i<FontSize;i++)
         szString[i]=32;
       szString[FontSize]='\0';
       strlength=FontSize;
    }
    else if(FontSize < strlength)
    {
       szString[strlength]='\0';
       strlength=FontSize;
    }

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;
#if ISU_OVERLAY_ENABLE
    for(j=0;j<char_H;j++)
    {
        for(i=0;i<char_W;i++)
        {
             k=j*char_W+i;
             if(szString[k] != ciuszString5[k])
             {
                 index=szString[k] - 32;
                 if( (index<95) && (index>=0) )
                 {
                     osd1=(u8*)OSDImg_top + osd_stride*FontH/2*j + FontW_Byte*i;
                     osd2=(u8*)OSDImg_bot + osd_stride*FontH/2*j + FontW_Byte*i;

                 #if 1
                     source  = (u8*)ASCII_Font[index];
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
                 #endif
                     for(y=0;y<FontH;y++)
                     {
                        if( (y & 0x01)==0 )
                        {
                          memcpy(osd1, source, sizeof(u8) * FontW_Byte); //top field
                          osd1 += osd_stride;
                        }
                        else
                        {
                          memcpy(osd2, source, sizeof(u8) * FontW_Byte); //bottom field
                          osd2 += osd_stride;
                        }
                        source += FontW_Byte;
                     }
                 }
             }

        }
    }
    strcpy(ciuszString5, szString);
#endif

    return  1;
}

#endif

s32 ciuCaptureVideo_CH1(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride)
{
    ciu_1_FrameTime = 0;
    if(sysPIPMain == PIP_MAIN_CH2)
    {
        ciuPreviewInit_CH2(SIUMODE_PREVIEW, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU2_OSD_EN, line_stride);
        ciuPreviewInit_CH1(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth / 2, OutHeight / 2, OutWidth / 2, OutHeight / 2, CIU1_OSD_EN, line_stride);
    }
    else
        ciuPreviewInit_CH1(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU1_OSD_EN, line_stride);
    return 1;
}

s32 ciuCaptureVideo_CH2(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride)
{
    ciu_2_FrameTime = 0;
    if(sysPIPMain == PIP_MAIN_CH1)
    {
        ciuPreviewInit_CH1(SIUMODE_PREVIEW, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU1_OSD_EN, line_stride);
        ciuPreviewInit_CH2(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth / 2, OutHeight / 2, OutWidth / 2, OutHeight / 2, CIU2_OSD_EN, line_stride);
    }
    else
        ciuPreviewInit_CH2(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU2_OSD_EN, line_stride);
    return 1;
}

s32 ciuCaptureVideo_CH3(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride)
{
    ciu_3_FrameTime = 0;    
    ciuPreviewInit_CH3(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU2_OSD_EN, line_stride);
    return 1;
}

s32 ciuCaptureVideo_CH4(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride)
{
    ciu_4_FrameTime = 0;
    ciuPreviewInit_CH4(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU2_OSD_EN, line_stride);
    return 1;
}

s32 ciuCaptureVideo_CH5(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride)
{
    ciuPreviewInit_CH5(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU5_OSD_EN, line_stride);
    return 1;
}

  #if (HW_DEINTERLACE_CIU1_ENA || HW_DEINTERLACE_CIU2_ENA || HW_DEINTERLACE_CIU3_ENA || HW_DEINTERLACE_CIU4_ENA || HW_DEINTERLACE_CIU5_ENA)
  #define DIU_Y_THR  16
  #define DIU_C_THR  20
  
  int diuRegConfig(int ID, u32 Width,u32 Height,u32 Stride)
  {
      u8 err;
      int i;
      u8 *Y_Curr,*C_Curr,*Y_Prev,*C_Prev;
      u32 MotionCnt,Y_Diff,Cb_Diff,Cr_Diff,Avg_Diff;
      //=======//
      OSSemPend(diuReadySemEvt, OS_IPC_WAIT_FOREVER, &err);

      SYS_CTL0_EXT |= SYS_CTL0_EXT_DIU_CKEN;
      switch(ID)
      {
           case MD_CIU1_ID:
              Y_Curr =PNBuf_sub1[ (ciu_idufrmcnt_ch1) & 0x03];
              Y_Prev =PNBuf_sub1[ (ciu_idufrmcnt_ch1-1) & 0x03];
              break;

           case MD_CIU2_ID:
              Y_Curr=PNBuf_sub2[ (ciu_idufrmcnt_ch2) & 0x03];
              Y_Prev=PNBuf_sub2[ (ciu_idufrmcnt_ch2-1) & 0x03];
              break;

           case MD_CIU3_ID:
              Y_Curr=PNBuf_sub3[ (ciu_idufrmcnt_ch3) & 0x03];
              Y_Prev=PNBuf_sub3[ (ciu_idufrmcnt_ch3-1) & 0x03];
              break;

           case MD_CIU4_ID:
              Y_Curr=PNBuf_sub4[ (ciu_idufrmcnt_ch4) & 0x03];
              Y_Prev=PNBuf_sub4[ (ciu_idufrmcnt_ch4-1) & 0x03];
              break;

            case MD_CIU5_ID:
              Y_Curr=PNBuf_sub5[ (ciu_idufrmcnt_ch5) & 0x03];
              Y_Prev=PNBuf_sub5[ (ciu_idufrmcnt_ch5-1) & 0x03];    
              break;
      }
      C_Curr =Y_Curr + PNBUF_SIZE_Y;
      C_Prev =Y_Prev + PNBUF_SIZE_Y;

      DIU_PREVADDR_Y=(unsigned int)Y_Prev;
      DIU_PREVADDR_C=(unsigned int)C_Prev;

      DIU_CURRADDR_Y=(unsigned int)Y_Curr;
      DIU_CURRADDR_C=(unsigned int)C_Curr;

      DIU_FRAME_STRIDE=Stride;

      DIU_WINSIZE= (Width<<DIU_WITH_SHFT) | (Height<<DIU_HEIGHT_SHFT);
      DIU_MOTIONPARAM=0x00120000 | (DIU_C_THR<<8) | DIU_Y_THR;
      
      if( (diuForceBobMode[ID]==1) || (diuForceBobLetecy[i]>0) )
         DIU_CTL=DIU_TRIG | DIU_FORCEBOB;
      else
         DIU_CTL=DIU_TRIG;
      OSSemPend(DIUCpleSemEvt, DIU_TIMEOUT, &err);
      
      if (err != OS_NO_ERR)
      {
            // reset DIU hardware
            //SYS_RSTCTL  = SYS_RSTCTL | 0x00000100;
            //for(i=0;i<10;i++);
            //SYS_RSTCTL  = SYS_RSTCTL & ~0x00000100;
            diuForceBobMode[ID]=0;

            DEBUG_CIU("DIU Timeout:%d\n", err);
      }
      else
      {
           MotionCnt=DIU_MVCOUNT;
           Y_Diff=DIU_Y_DIFSUM;
           Cb_Diff=DIU_CB_DIFSUM;
           Cr_Diff=DIU_CR_DIFSUM;
           if(MotionCnt == 0)
             MotionCnt=1;
           Avg_Diff= (Y_Diff+Cb_Diff*4+Cr_Diff*4)/9/MotionCnt;
           if( (Avg_Diff>12) || (MotionCnt > 40000) )
           {
              diuForceBobMode[ID]=1;
              diuForceBobLetecy[ID]=30;
           }
           else
              diuForceBobMode[ID]=0;

           diuForceBobLetecy[ID] -=1;
           if(diuForceBobLetecy[ID] < 0)
              diuForceBobLetecy[ID]=0;
            
           //DEBUG_CIU("Avg=%d,%d,\n",Avg_Diff,MotionCnt);
      }

      SYS_CTL0_EXT &= (~SYS_CTL0_EXT_DIU_CKEN);

      OSSemPost(diuReadySemEvt);
  }

  void diuIntHandler(void)
  {
     u32 intStat;

     intStat = DIU_INTRPT;
     //DEBUG_CIU("DIU ");
     OSSemPost(DIUCpleSemEvt);
  }

  #endif


u32 ciuCheckFrameRate(void)
{
    static u32 last_count=0;
    static u8 count_error=0;
    u32 fps,fps_temp;

#if TX_PIRREC_SUPPORT 
    if(rfiuStopPIRRecReady)
        return 0;
#endif

    #if (MULTI_CHANNEL_SEL &0x02)
        fps_temp = ciu_1_FPS_Count;
    #elif (MULTI_CHANNEL_SEL &0x04)
        fps_temp = ciu_2_FPS_Count;
    #elif (MULTI_CHANNEL_SEL &0x20)
        fps_temp = ciu_5_FPS_Count;
    #endif
    if(last_count == 0)
    {
        last_count=fps_temp;
        return 0;
    }
    fps=fps_temp - last_count;

    #if (HW_BOARD_OPTION == MR9120_TX_RDI_CA831) //追蹤 FPS=0 沒有reboot
    DEBUG_CIU("fps=%d fps_temp=%d last_count=%d count_error=%d \n",fps,fps_temp,last_count,count_error);
    #endif
    if(fps <3)
    {
        count_error++;
        if(count_error >5)
        {
            #if (MULTI_CHANNEL_SEL &0x02)
            DEBUG_TIMER("CIU1 Frame error %d , system reboot\n", fps);
            #elif (MULTI_CHANNEL_SEL &0x04)
            DEBUG_TIMER("CIU2 Frame error %d , system reboot\n", fps);
            #elif (MULTI_CHANNEL_SEL &0x20)
            DEBUG_TIMER("CIU5 Frame error %d , system reboot\n", fps);
            #endif
			sysForceWDTtoReboot();
        }
        
    }
    else
    {
        count_error=0;
    }
    
    last_count =fps_temp;    
    return fps;
    
}


u8 CIUDrawMaskArea(u16 x,u16 y)
{
    u16 Bpoint;
    u16 Bshift;
    u16 w;
    
    w = 640/4/8;
    Bpoint = x/8 + y*w;
    Bshift = x%8;
    *(MaskAreaBuf +Bpoint) |= 1 << Bshift;
//    printf("CIUDrawMaskArea %d %d\n",Bpoint,Bshift);
    return 1;
}
u8 CIUClearMaskArea(u16 x,u16 y)
{
    u16 Bpoint;
    u16 Bshift;
    u16 w;
    
    w = 640/4/8;
    Bpoint = x/8 + y*w;
    Bshift = x%8;
    *(MaskAreaBuf +Bpoint) &= ~(1 << Bshift);
//    printf("CIUDrawMaskArea %d %d\n",Bpoint,Bshift);
    return 1;
}
u8 CIUClearMaskAreaAll()
{
    u16 x;

    for (x=0;x<2400;x++)
    *(MaskAreaBuf +x) = 0;
//    printf("CIUDrawMaskArea %d %d\n",Bpoint,Bshift);
    return 1;
}
#else
u8 getCiuOSD_OnOff(void) {}
void ciuIntHandler_CH1(void){}
void ciuIntHandler_CH2(void){}
void ciuIntHandler_CH3(void){}
void ciuIntHandler_CH4(void){}
void ciuIntHandler_CH5(void){}
void ciu_1_Stop(void){}
void ciu_2_Stop(void){}
void ciu_3_Stop(void){}
void ciu_4_Stop(void){}
void ciu_5_Stop(void){}
s32 ciu1ScUpZoom(s32 zoomFactor){return 1;}
s32 ciu2ScUpZoom(s32 zoomFactor){return 1;}
s32 ciu3ScUpZoom(s32 zoomFactor){return 1;}
s32 ciu4ScUpZoom(s32 zoomFactor){return 1;}
s32 ciu5ScUpZoom(s32 zoomFactor){return 1;}
s32 ciuPreviewInit_CH1(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuPreviewInit_CH2(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuPreviewInit_CH3(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuPreviewInit_CH4(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuPreviewInit_CH5(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride){return 1;}
s32 CiuInit(void){return 1;}
s32 ciuCaptureVideo_CH1(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuCaptureVideo_CH2(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuCaptureVideo_CH3(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuCaptureVideo_CH4(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuCaptureVideo_CH5(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride){return 1;}
void CiuOSD_OnOff(u8 onoff){}
s32 GenerateCIU1_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,char *szString, int MaxStrLen,int FontW, int FontH,int Font_STR_X,int Font_STR_Y,int Font_END_X,int Font_END_Y){return 1;}
s32 GenerateCIU2_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,char *szString, int MaxStrLen,int FontW, int FontH,int Font_STR_X,int Font_STR_Y,int Font_END_X,int Font_END_Y){return 1;}
s32 GenerateCIU3_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,char *szString, int MaxStrLen,int FontW, int FontH,int Font_STR_X,int Font_STR_Y,int Font_END_X,int Font_END_Y){return 1;}
s32 GenerateCIU4_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,char *szString, int MaxStrLen,int FontW, int FontH,int Font_STR_X,int Font_STR_Y,int Font_END_X,int Font_END_Y){return 1;}
s32 GenerateCIU5_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,char *szString, int MaxStrLen,int FontW, int FontH,int Font_STR_X,int Font_STR_Y,int Font_END_X,int Font_END_Y){return 1;}
#endif


