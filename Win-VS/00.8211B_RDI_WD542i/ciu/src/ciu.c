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
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || \
    (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))

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
#include "rfiuapi.h"
#if MULTI_CHANNEL_VIDEO_REC
#include "GlobalVariable.h"
#endif
#if (TUTK_SUPPORT==1)
#include "../LwIP/include/tutk_P2P/AVIOCTRLDEFs.h"
#endif
#if( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
#include "gfuapi.h"
#endif


/*
 *********************************************************************************************************
 *  SYS Constant
 *********************************************************************************************************
 */

#define CIU_TIMEOUT			20
#define CIU_MAXOSDSTR       128
#define PULSE_TIME          250   //每輸出250次Hight Level 大約 花0.5ms

#define DIU_TIMEOUT         2

#define CIU_SP_TEST         0     // For TEST SP, MP+SP on one frame.
#if( (HW_BOARD_OPTION == A1020A_FPGA_BOARD) || (HW_BOARD_OPTION == A1026A_FPGA_BOARD) )
#define NEWCIU1_TEST_EN     1
#define NEWCIU1_SW_MPSP     0
#endif
/*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
OS_FLAG_GRP  *ciuFlagGrp_CH1;
OS_FLAG_GRP  *ciuFlagGrp_CH2;
OS_FLAG_GRP  *ciuFlagGrp_CH3;
OS_FLAG_GRP  *ciuFlagGrp_CH4;

#if (HW_DEINTERLACE_CIU1_ENA || HW_DEINTERLACE_CIU2_ENA || HW_DEINTERLACE_CIU3_ENA || HW_DEINTERLACE_CIU4_ENA)
OS_EVENT *diuReadySemEvt;
OS_EVENT *DIUCpleSemEvt; /*BJ 0530 S*/
#endif

OS_EVENT* ciuCapSemEvt_CH1;     // for Video capture
OS_EVENT* ciuCapSemEvt_CH2;     // for Video capture
OS_EVENT* ciuCapSemEvt_CH3;     // for Video capture
OS_EVENT* ciuCapSemEvt_CH4;     // for Video capture



OS_STK ciuTaskStack_CH1[CIU_TASK_STACK_SIZE_CH1]; /* Stack of task ciuTask() */
OS_STK ciuTaskStack_CH2[CIU_TASK_STACK_SIZE_CH2]; /* Stack of task ciuTask() */
OS_STK ciuTaskStack_CH3[CIU_TASK_STACK_SIZE_CH3]; /* Stack of task ciuTask() */
OS_STK ciuTaskStack_CH4[CIU_TASK_STACK_SIZE_CH4]; /* Stack of task ciuTask() */


u32 ciu_idufrmcnt_ch1 = 0;
u32 ciu_idufrmcnt_ch2 = 0;
u32 ciu_idufrmcnt_ch3 = 0;
u32 ciu_idufrmcnt_ch4 = 0;

int diuForceBobMode[MC_CH_MAX];
int diuForceBobLetecy[MC_CH_MAX];

static char ciuszString1[CIU_MAXOSDSTR]  = "";
static char ciuszString2[CIU_MAXOSDSTR]  = "";
static char ciuszString3[CIU_MAXOSDSTR]  = "";
static char ciuszString4[CIU_MAXOSDSTR]  = "";

#if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
static char ciuszString1_SP[CIU_MAXOSDSTR]  = "";
#endif

u8 ciu_1_OpMode = SIUMODE_PREVIEW;
u8 ciu_2_OpMode = SIUMODE_PREVIEW;
u8 ciu_3_OpMode = SIUMODE_PREVIEW;
u8 ciu_4_OpMode = SIUMODE_PREVIEW;

int ciu1ZoomStart;
int ciu1ZoomOnOff;
int ciu1ZoomXpos;
int ciu1ZoomYpos;


u32 ciu_1_FrameTime;
u32 ciu_2_FrameTime;
u32 ciu_3_FrameTime;
u32 ciu_4_FrameTime;

u32 ciu_1_OutX;
u32 ciu_2_OutX;
u32 ciu_3_OutX;
u32 ciu_4_OutX;

u32 ciu_1_OutY;
u32 ciu_2_OutY;
u32 ciu_3_OutY;
u32 ciu_4_OutY;

u32 ciu_1_line_stride;
u32 ciu_2_line_stride;
u32 ciu_3_line_stride;
u32 ciu_4_line_stride;

u32 ciu_1_OutWidth;
u32 ciu_2_OutWidth;
u32 ciu_3_OutWidth;
u32 ciu_4_OutWidth;

u32 ciu_1_OutHeight;
u32 ciu_2_OutHeight;
u32 ciu_3_OutHeight;
u32 ciu_4_OutHeight;

u32 ciu_1_pnbuf_size_y;
u32 ciu_2_pnbuf_size_y;
//u32 ciu_3_pnbuf_size_y;
//u32 ciu_4_pnbuf_size_y;

u32 ciu_1_PIP_OutWidth;
u32 ciu_2_PIP_OutWidth;
u32 ciu_3_PIP_OutWidth;
u32 ciu_4_PIP_OutWidth;


u32 ciu_1_PIP_OutHeight;
u32 ciu_2_PIP_OutHeight;
u32 ciu_3_PIP_OutHeight;
u32 ciu_4_PIP_OutHeight;


u32 ciu_1_PIP_OutX;
u32 ciu_2_PIP_OutX;
u32 ciu_3_PIP_OutX;
u32 ciu_4_PIP_OutX;


u32 ciu_1_PIP_OutY;
u32 ciu_2_PIP_OutY;
u32 ciu_3_PIP_OutY;
u32 ciu_4_PIP_OutY;

u32 ciu_1_FPS_Count;

/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
#if(RFIU_SUPPORT)
extern DEF_RFIU_UNIT_CNTL   gRfiuUnitCntl[];
#endif
extern u8 siuOpMode;
extern  u32 *CiuOverlayImg1_Top;
extern  u32 *CiuOverlayImg1_Bot;

extern  u32 *CiuOverlayImg2_Top;
extern  u32 *CiuOverlayImg2_Bot;

extern  u32 *CiuOverlayImg3_Top;
extern  u32 *CiuOverlayImg3_Bot;

extern  u32 *CiuOverlayImg4_Top;
extern  u32 *CiuOverlayImg4_Bot;
#if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
extern  u32 *CiuOverlayImg1_SP_Top;
extern  u32 *CiuOverlayImg1_SP_Bot;
#endif
extern u32  MD_period_Preview;  //Lucian: 設定幾個frame 後,做一次motion detection
extern u32  MD_period_Video;

extern u8 sysReady2CaptureVideo;
extern u8 uiMenuVideoSizeSetting;
extern u8 video_double_field_flag;

extern u8  szVideoOverlay1[MAX_OVERLAYSTR];
extern u8  szVideoOverlay2[MAX_OVERLAYSTR];
extern u8  szLogString[MAX_OVERLAYSTR];
extern u8 nightVMDadd ; // Amon 160817

#if HW_MD_SUPPORT
  extern u8 MotionDetect_en;
#endif

extern u32 VideoPictureIndex;
//extern s32 isu_avifrmcnt;

#if (HW_BOARD_OPTION == MR8211_ZINWELL)
extern u16 Light;
extern f32 TEMP;
extern u8  IR_Mode;    // AVIOCTRL_NIGHT_ON, AVIOCTRL_NIGHT_OFF, AVIOCTRL_NIGHT_AUTO
#endif

#if (HW_BOARD_OPTION == MR6730_AFN)
#if (CIU_BOB_MODE)
extern u8 g_ShowBobOnOSD;//replace the "SHOW_BOB_ON_OSD"		

extern u8 g_CIU_BobMode;//b0:CIU1 Bob , b1 CIU2 Bob , ...	
#endif
#endif

#if(HW_BOARD_OPTION == MR8100_GCT_VM9710)
extern u8 uiNightVision;
extern int ciu1MotorXpos;
extern int ciu1MotorYpos;
#else
extern u8 uiNightVision;
int ciu1MotorXpos;
int ciu1MotorYpos;
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


void ciuIntHandler_CH1(void);
void ciuIntHandler_CH2(void);
void ciuIntHandler_CH3(void);
void ciuIntHandler_CH4(void);


s32 ciuPreviewInit_CH1(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
s32 ciuPreviewInit_CH2(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
s32 ciuPreviewInit_CH3(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);
s32 ciuPreviewInit_CH4(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY,u8 OSD_en,u32 line_stride);



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

s32 GenerateCIU1_SP_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y);
/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */
#if CIU_SUPPORT

s32 CiuInit(void)
{
    int i;
    u8 err;

    SYSReset(SYS_RSTCTL_CIU_RST);
    #if (CHIP_OPTION == CHIP_A1018A)
        #if (MULTI_CHANNEL_SEL & 0x04)
        SYSReset(SYS_RSTCTL_CIU2_RST);
        #endif
        #if (MULTI_CHANNEL_SEL & 0x08)
        SYSReset_EXT(SYS_CTL0_EXT_CIU3_RST);
        #endif
        #if (MULTI_CHANNEL_SEL & 0x10)
        SYSReset_EXT(SYS_CTL0_EXT_CIU4_RST);
        #endif
    #endif

	memset(CiuOverlayImg1_Top, 0, 640 * 80 );
    memset(CiuOverlayImg1_Bot, 0, 640 * 80 );

    memset(CiuOverlayImg2_Top, 0, 640 * 80 );
    memset(CiuOverlayImg2_Bot, 0, 640 * 80 );

    memset(CiuOverlayImg3_Top, 0, 640 * 80 );
    memset(CiuOverlayImg3_Bot, 0, 640 * 80 );

    memset(CiuOverlayImg4_Top, 0, 640 * 80 );
    memset(CiuOverlayImg4_Bot, 0, 640 * 80 );

#if HW_MD_SUPPORT
    mduMotionDetect_init();
#endif

    ciu1ZoomStart=0;
    ciu1ZoomOnOff=0;
    ciu1ZoomXpos=0;
    ciu1ZoomYpos=0;

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

#if ( (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
    //----CH4-----//
    // Create the semaphore
    ciuFlagGrp_CH4=OSFlagCreate(0x00000000, &err);
    ciuCapSemEvt_CH4    = OSSemCreate(0);
    OSTaskCreate(CIU_TASK_CH4, CIU_TASK_PARAMETER_CH4, CIU_TASK_STACK_CH4, CIU_TASK_PRIORITY_CH4);
#endif

#if (HW_DEINTERLACE_CIU1_ENA || HW_DEINTERLACE_CIU2_ENA || HW_DEINTERLACE_CIU3_ENA || HW_DEINTERLACE_CIU4_ENA)
    diuReadySemEvt = OSSemCreate(1);
    DIUCpleSemEvt  = OSSemCreate(0);
#endif

	return 1;
}

void ciu_1_Stop(void)
{
    int i;

    //CIU_1_CTL2 &= (~(CIU_DATA_OUT_ENA | CIU_INT_ENA_FRAME_END));
    //OSTimeDly(2);
    //CIU_1_CTL2 = 0;
    CIU_1_CTL2 &= ~CIU_ENA;
    while(CIU_1_CTL2 & 0x1);
}

void ciu_2_Stop(void)
{
    int i;

    CIU_2_CTL2 &= (~(CIU_DATA_OUT_ENA | CIU_INT_ENA_FRAME_END));
    OSTimeDly(2);
    CIU_2_CTL2 = 0;
}

void ciu_3_Stop(void)
{
    int i;

    CIU_3_CTL2 &= (~(CIU_DATA_OUT_ENA | CIU_INT_ENA_FRAME_END));
    OSTimeDly(2);
    CIU_3_CTL2 = 0;
}

void ciu_4_Stop(void)
{
    int i;

    CIU_4_CTL2 &= (~(CIU_DATA_OUT_ENA | CIU_INT_ENA_FRAME_END));
    OSTimeDly(2);
    CIU_4_CTL2 = 0;
}

void ciuTask_CH1(void* pData)
{
    u8  err;
    u32 waitFlag;
    //u8  DateTime[36];   /*YYYY/MM/DD HH:mm:ss*/
    //RTC_DATE_TIME   localTime;
    u32 i;
    u8  mdset;
    int Mode        = -1;
    u8  level       = 0;

    u32 adcvalue;

    static u8 MDdropF=0;
    static u8 MDdropFCNT=0;
#if (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692)
    u8  ledLevel;
#endif
#if ((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION == MR8120S_TX_GCT_VM00) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    u16  light_rty=0;
    u8   GCTlevel       = 1;
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
           gfuTest_ALL_Cmd();
        }
    #endif

    #if HW_DEINTERLACE_CIU1_ENA
        if(waitFlag & CIU_EVET_TOPFILD_END)
        {
            //DEBUG_CIU("Top intr\n");
            diuRegConfig(MD_CIU1_ID, 640,240,640*2);
        }
    #endif
    #if((HW_BOARD_OPTION  == MR8100_GCT_VM9710) )
        if((light_rty>0)&&(light_rty%10 ==0))
            rfiu_SetRXLightTrig(uiNightVision);
        
        if(light_rty>0)
        {
            light_rty--;
        }
        else
        {
            if(gRfiuUnitCntl[0].OpMode != RFIU_TX_MODE)
                light_rty =30;
        }
    #endif
    #if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
        if((light_rty>0)&&(light_rty%10 ==0))
            uiTXStaustoRX(TXstausbit0,uiNightVision);
        
        if(light_rty>0)
        {
            light_rty--;
        }
        else
        {
            if(gRfiuUnitCntl[0].OpMode != RFIU_TX_MODE)
                light_rty =30;
        }    
    #endif
    #if ((CIU1_OPTION == Sensor_OV7725_YUV601)&& ((HW_BOARD_OPTION == MR8120_TX_RDI2) || (HW_BOARD_OPTION == MR8120_TX_RDI3)))   //Adjust Saturation
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

    #if (HW_BOARD_OPTION == MR8211_ZINWELL)
        switch(IR_Mode)
        {
            case AVIOCTRL_NIGHT_ON:
                if(Mode != SIU_NIGHT_MODE)
                {
                    DEBUG_SIU("##enter night \n");
                    gpioSetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, 0);
                    siuSetSensorDayNight(SIU_NIGHT_MODE);
                    Mode    = SIU_NIGHT_MODE;
                }
                break;
            case AVIOCTRL_NIGHT_OFF:
                if(Mode != SIU_DAY_MODE)
                {
                    DEBUG_SIU("@@enter day \n");
                    gpioSetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, 1);
                    siuSetSensorDayNight(SIU_DAY_MODE);
                    Mode    = SIU_DAY_MODE;
                }
                break;
            case AVIOCTRL_NIGHT_AUTO:
                if(Mode != SIU_NIGHT_MODE && Light < 0x130)
                {
                    DEBUG_SIU("##enter night \n");
                    gpioSetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, 0);
                    siuSetSensorDayNight(SIU_NIGHT_MODE);
                    Mode    = SIU_NIGHT_MODE;
                }
                else if(Mode != SIU_DAY_MODE && Light > 0x1C6)
                {
                    DEBUG_SIU("@@enter day \n");
                    gpioSetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, 1);
                    siuSetSensorDayNight(SIU_DAY_MODE);
                    Mode    = SIU_DAY_MODE;
                }
                break;
        }
    #elif((HW_BOARD_OPTION == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA672) || (HW_BOARD_OPTION == MR8120_TX_RDI2) ||\
         (HW_BOARD_OPTION == MR8120_TX_RDI2) || ( (HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT != 4) ))
      
        if (gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level) == 0)
            level = SIU_DAY_MODE;

        if(Mode != level)
        {
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
                #if HW_MD_SUPPORT
                    mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
                #endif
                #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1);
                    OSTimeDly(4);
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
                #endif                   
            }
            else 
            {
                DEBUG_SIU("##enter night \n");
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                Mode    = SIU_NIGHT_MODE;
                #if HW_MD_SUPPORT
                    mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
                #endif
                #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1);
                    OSTimeDly(4);
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
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
      
    #elif ((HW_BOARD_OPTION == MR8120_TX_RDI_CA652) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA542))
        adcvalue = adcGetValueAverage(0,15);
        if( (adcvalue < 620) && (adcvalue != 0)) // 20151111
            level=SIU_DAY_MODE;
        else if(adcvalue > 1400) // 20151111
            level=SIU_NIGHT_MODE;

        if(Mode != level)
        {
//            if (level == SIU_DAY_MODE)
//            {
//                if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x01)== 0x1 )
//                    MotionDetect_en   = 1;
//                else
//                    MotionDetect_en   = 0;
//            }
//            else
//            {
//                if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x02)== 0x2 )
//                    MotionDetect_en   = 1;
//                else
//                    MotionDetect_en   = 0;
//                
//            }
            mdset = MotionDetect_en;
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
                #if HW_MD_SUPPORT
                    mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
                #endif
                #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1);
                    OSTimeDly(4);
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
                #endif                   
            }
            else 
            {
                DEBUG_SIU("##enter night \n");
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                Mode    = SIU_NIGHT_MODE;
                #if HW_MD_SUPPORT
                    mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
                #endif
                #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1);
                    OSTimeDly(4);
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
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
      
    #elif(HW_BOARD_OPTION == MR8120_TX_RDI_CL692)
      
        if (gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level) == 0)
            level = SIU_DAY_MODE;

        if(Mode != level)
        {
     
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
                #if HW_MD_SUPPORT
                    #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CL692) && (UI_PROJ_OPT == 3))
                    nightVMDadd = 0; // Amon 160817
                    #endif
                    mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
                #endif
                #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1);
                    OSTimeDly(4);
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
                #endif                   
            }
            else 
            {
                DEBUG_SIU("##enter night \n");

                siuSetSensorDayNight(SIU_NIGHT_MODE);

                Mode    = SIU_NIGHT_MODE;
                #if HW_MD_SUPPORT
                    #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CL692) && (UI_PROJ_OPT == 3))
                    nightVMDadd = 2; // Amon 160817
                    #endif
                    mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
                #endif
                #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1);
                    OSTimeDly(4);
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
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
    #elif ( (HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||( (HW_BOARD_OPTION == MR8120_TX_RDI_CA671) && (UI_PROJ_OPT == 4) ) )  
        if (gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level) == 0)
            level = SIU_DAY_MODE;

        if(Mode != level)
        {
//            if (level == SIU_DAY_MODE)
//            {
//                if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x01)== 0x1 )
//                    MotionDetect_en   = 1;
//                else
//                    MotionDetect_en   = 0;
//            }
//            else
//            {
//                if ((iconflag[UI_MENU_SETIDX_TX_MOTION] & 0x02)== 0x2 )
//                    MotionDetect_en   = 1;
//                else
//                    MotionDetect_en   = 0;
//                
//            }
            mdset = MotionDetect_en;
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
                #if HW_MD_SUPPORT
                    mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY]);
                #endif
                #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1);
                    OSTimeDly(4);
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
                #endif                   
            }
            else 
            {
                DEBUG_SIU("##enter night \n");
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                Mode    = SIU_NIGHT_MODE;
                #if HW_MD_SUPPORT
                    mduMotionDetect_Sensitivity_Config(iconflag[UI_MENU_SETIDX_MOTION_SENSITIVITY_NIGHT]);
                #endif
                #if ((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1);
                    OSTimeDly(4);
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
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
    #elif((HW_BOARD_OPTION == MR8120_TX_RDI2) || (HW_BOARD_OPTION == MR8120_TX_HECHI) )
      #if SMART_LED
        i   = (AdcRecData_G1G2 >> 16) ^ 0x0800;
        if(i < 0x0800)
            level   = SIU_DAY_MODE;
        else if(i > 0x0A00) 
            level   = SIU_NIGHT_MODE;
      #else
        if (gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level) == 0)
            level = SIU_DAY_MODE;
      #endif

        if(Mode != level)
        {
        #if SMART_LED
            mduMotionDetect_ONOFF(1);
        #else
            mdset   = MotionDetect_en;
            if(mdset)
            {
            #if HW_MD_SUPPORT
               mduMotionDetect_ONOFF(0);
    	    #endif
            }
        #endif
           
            if(level == SIU_DAY_MODE)
            {
                DEBUG_SIU("@@enter day \n");
            #if SMART_LED
                Timer3Ctrl  = 0x06ff0004;   // LED全暗
            #endif
                siuSetSensorDayNight(SIU_DAY_MODE);
                Mode    = SIU_DAY_MODE;
            }
            else 
            {
                DEBUG_SIU("##enter night \n");
            #if SMART_LED
                Timer3Ctrl  = 0x06400004;   // LED全亮
            #endif
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                Mode    = SIU_NIGHT_MODE;
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
      #if SMART_LED
        else if(Mode == SIU_NIGHT_MODE)
        {
            const u32   PWM_Para[4] = {0x06ff0004, 0x06b40004, 0x06770004, 0x06400004}; // 全暗到全亮
            u32         Gain;
            if( (ciu_idufrmcnt_ch1 & (MD_period_Preview-1)) == 0)
            {
                Gain    = siuGetExposureGain();
                DEBUG_CIU("Gain = 0x%08x\n\n", Gain);
                if(Gain < 30)
                    Timer3Ctrl  = PWM_Para[1];   // LED全暗
                else if(Gain < 60)
                    Timer3Ctrl  = PWM_Para[2];
                else
                    Timer3Ctrl  = PWM_Para[3];
            }
        }
      #endif
    #elif(HW_BOARD_OPTION == MR8120_TX_JIT)
        if(Main_Init_Ready) // for 1375
        {
            gpioDetectIRLED();
        }
    #elif ((HW_BOARD_OPTION == MR8120_TX_RDI_CA530))
    {
        gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT,GPIO_BIT_SENSOR_DAYNIGHT, &level);

        if(Mode != level)
        {
            if(level == SIU_DAY_MODE)
            {
                DEBUG_GPIO("@@enter day \n");
                siuSetSensorDayNight(SIU_DAY_MODE);
                Mode =SIU_DAY_MODE;
                gpioSetLevel(GPIO_GROUP_NIGHT_LED, GPIO_BIT_NIGHT_LED, GPIO_LEVEL_LO);
                
            }
            else 
            {
                DEBUG_GPIO("##enter night \n");
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                Mode =SIU_NIGHT_MODE;
                gpioSetLevel(GPIO_GROUP_NIGHT_LED, GPIO_BIT_NIGHT_LED, GPIO_LEVEL_HI);
                
            }
        }    
    }
    #elif ((HW_BOARD_OPTION == MR8120_TX_GCT) || (HW_BOARD_OPTION == MR8120_TX_SKYSUCCESS) ||\
           (HW_BOARD_OPTION == MR8120_TX_FRESHCAM))
        gpioGetLevel(GPIO_GROUP_DAY_NIGHT,GPIO_PIN_DAY_NIGHT, &level);
      
        if(Mode != level)
        {
            if(level == SIU_DAY_MODE)
            {      
                gpioSetLevel(GPIO_GROUP_DAY_PWM, GPIO_PIN_DAY_PWM,0);           
                for(i=0;i<PULSE_TIME*210;i++)
                    gpioSetLevel(GPIO_GROUP_DAY_PWM, GPIO_PIN_DAY_PWM, 1);
                gpioSetLevel(GPIO_GROUP_DAY_PWM, GPIO_PIN_DAY_PWM,0);           
                siuSetSensorDayNight(SIU_DAY_MODE);
                Mode =SIU_DAY_MODE;
            }
            else
            {
                gpioSetLevel(GPIO_GROUP_NIGHT_PWM, GPIO_PIN_NIGHT_PWM, 0);
                for(i=0;i<PULSE_TIME*210;i++)
                    gpioSetLevel(GPIO_GROUP_NIGHT_PWM, GPIO_PIN_NIGHT_PWM, 1);
                gpioSetLevel(GPIO_GROUP_NIGHT_PWM, GPIO_PIN_NIGHT_PWM, 0);
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                Mode =SIU_NIGHT_MODE;
            }

        }
    #elif((HW_BOARD_OPTION == MR8120_TX_TRANWO2) || (HW_BOARD_OPTION == MR8120_TX_MAYON_LW604) ||\
        (HW_BOARD_OPTION == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) || (HW_BOARD_OPTION == MR8120_TX_MAYON_MWL605C)||\
        (HW_BOARD_OPTION == MR8120_TX_MA8806) || (HW_BOARD_OPTION == MR8120_TX_TRANWO_VM2505) ||\
        (HW_BOARD_OPTION == MR8120_TX_TRANWO3))
        gpioDetectIRLED();
    #elif(HW_BOARD_OPTION  == MR8100_GCT_VM9710)
        if (Main_Init_Ready == 1)
        {
            adcvalue = adcGetValueAverage(1,30);
            gpioGetLevel(GPIO_GROUP_MODE_SEL, GPIO_BIT_MODE_SEL, &level);
            if(level==1) // VM9710
            {
                if( (adcvalue > 515) ) // 20150921
                    GCTlevel=SIU_DAY_MODE;
                else if( (adcvalue < 241) && (adcvalue > 0)) // 20150921
                    GCTlevel=SIU_NIGHT_MODE;
            }
            else // VM9700
            {
                if( (adcvalue > 3100) ) // 20151118
                    GCTlevel=SIU_DAY_MODE;
                else if( (adcvalue < 1900) && (adcvalue > 0)) // 20151120
                    GCTlevel=SIU_NIGHT_MODE;
            }
        }
        
        if(Mode != GCTlevel)
        {
            if(GCTlevel == SIU_DAY_MODE)
            {
                siuSetSensorDayNight(SIU_DAY_MODE);
                gpioSetLevel(GPIO_GROUP_IR_POWER,GPIO_PIN_IR_POWER,0); // led off
                Mode    = SIU_DAY_MODE;
                uiNightVision = 0;
                light_rty=50;
            }
            else if(GCTlevel == SIU_NIGHT_MODE)
            {
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                gpioSetLevel(GPIO_GROUP_IR_POWER,GPIO_PIN_IR_POWER,1); // led on
                Mode    = SIU_NIGHT_MODE;
                uiNightVision = 1;
                light_rty=50;
            }
        }
    #elif(HW_BOARD_OPTION  == MR8120S_TX_GCT_VM00)
        if (Main_Init_Ready == 1)
        {
            adcvalue = adcGetValueAverage(1,30);
            if( (adcvalue > 515) ) // 20150921
                GCTlevel=SIU_DAY_MODE;
            else if( (adcvalue < 241) && (adcvalue > 0)) // 20150921
                GCTlevel=SIU_NIGHT_MODE;
        }
        
        if(Mode != GCTlevel)
        {
            if(GCTlevel == SIU_DAY_MODE)
            {
                siuSetSensorDayNight(SIU_DAY_MODE);
                gpioSetLevel(GPIO_GROUP_IR_POWER,GPIO_PIN_IR_POWER,0); // led off
                Mode    = SIU_DAY_MODE;
            }
            else if(GCTlevel == SIU_NIGHT_MODE)
            {
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                gpioSetLevel(GPIO_GROUP_IR_POWER,GPIO_PIN_IR_POWER,1); // led on
                Mode    = SIU_NIGHT_MODE;
            }
        }
    #elif(HW_BOARD_OPTION == MR8211_TX_RDI_SEP)
        if (Main_Init_Ready == 1)
        {
          #if ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) && (UI_PROJ_OPT == 0))    /*RDI SEP*/
            adcvalue = adcGetValueAverage(1,30);
            if( (adcvalue > 3600) ) // Night
                GCTlevel=SIU_NIGHT_MODE;
            else if( (adcvalue < 3000) && (adcvalue > 0)) // day
                GCTlevel=SIU_DAY_MODE;
          #else //SEP 1: WD542, 2:CA542H
            adcvalue = adcGetValueAverage(1,5);
            if( (adcvalue > 3400) ) // Night
                GCTlevel=SIU_NIGHT_MODE;
            else if( (adcvalue < 3200) && (adcvalue > 0)) // day
                GCTlevel=SIU_DAY_MODE;
          #endif
        }
        if(Mode != GCTlevel)
        {
            if(GCTlevel == SIU_DAY_MODE)
            {
                siuSetSensorDayNight(SIU_DAY_MODE);
                gpioSetLevel(GPIO_GROUP_IR_POWER,GPIO_PIN_IR_POWER,0);
                Mode    = SIU_DAY_MODE;
                uiNightVision = 0;
                light_rty=50;
            }
            else if(GCTlevel == SIU_NIGHT_MODE)
            {
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                gpioSetLevel(GPIO_GROUP_IR_POWER,GPIO_PIN_IR_POWER,1);
                Mode    = SIU_NIGHT_MODE;
                uiNightVision = 1;
                light_rty=50;
            }
        }
    #elif(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)
        if (Main_Init_Ready == 1)
        {
            adcvalue = adcGetValueAverage(1,5);
            if( (adcvalue > 3700) ) // Night
                GCTlevel=SIU_NIGHT_MODE;
            else if( (adcvalue < 3400) && (adcvalue > 0)) // day
                GCTlevel=SIU_DAY_MODE;
        }
        if(Mode != GCTlevel)
        {
            if(GCTlevel == SIU_DAY_MODE)
            {
                siuSetSensorDayNight(SIU_DAY_MODE);
                gpioSetLevel(GPIO_GROUP_IR_POWER,GPIO_PIN_IR_POWER,0);
                Mode    = SIU_DAY_MODE;
                uiNightVision = 0;
                light_rty=50;
            }
            else if(GCTlevel == SIU_NIGHT_MODE)
            {
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                gpioSetLevel(GPIO_GROUP_IR_POWER,GPIO_PIN_IR_POWER,1);
                Mode    = SIU_NIGHT_MODE;
                uiNightVision = 1;
                light_rty=50;
            }
        }
//    #elif (HW_BOARD_OPTION == MR8120_TX_RDI_CA532)
//        gpioDetectIRLED();
//        //gpioDetectMIRRORFLIP();
	#elif( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN)&&(USE_SENSOR_DAY_NIGHT_MODE) )
	//if(OS_tickcounter%10==0)//25ms*20=500ms
	if(OS_tickcounter%4==0)//25ms*4=100ms
	{
		gpioGetLevel(GPIO_GROUP_DAY_NIGHT,GPIO_PIN_DAY_NIGHT,&level);
		if(Mode != level)
        {
			if(level == SIU_DAY_MODE)
			{
				DEBUG_SIU("@@enter day \n");
				siuSetSensorDayNight(SIU_DAY_MODE);
                #if (USE_SENSOR_DAY_NIGHT_MODE == 2)
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 1);
                    OSTimeDly(4);
                    gpioSetLevel(GPIO_GROUP_ICR_ON, GPIO_PIN_ICR_ON, 0);
				#elif (USE_SENSOR_DAY_NIGHT_MODE == 1)
				gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
				//...				
                #endif   				
			}
			else 
			{
				DEBUG_SIU("##enter night \n");
				siuSetSensorDayNight(SIU_NIGHT_MODE);
                #if (USE_SENSOR_DAY_NIGHT_MODE == 2)
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1);
                    OSTimeDly(4);
                    gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 0);
				#elif (USE_SENSOR_DAY_NIGHT_MODE == 1)
				gpioSetLevel(GPIO_GROUP_ICR_OFF, GPIO_PIN_ICR_OFF, 1);
				//...				
                #endif  				
			}
			Mode = level;
		}
	}     
    #endif      

	}
}


void ciuTask_CH2(void* pData)
{
	u8 err;
    u32 waitFlag;

	while (1)
	{
		waitFlag = OSFlagPend(ciuFlagGrp_CH2, CIU_EVET_FREAME_END|CIU_EVET_TOPFILD_END ,OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: ciuFlagGrp_CH2 is %d.\n", err);
			//return ;
		}

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
            diuRegConfig(MD_CIU2_ID, 640,240,640*2);
        }
    #endif



	}
}

void ciuTask_CH3(void* pData)
{
	u8 err;
    u32 waitFlag;

	while (1)
	{
	    waitFlag = OSFlagPend(ciuFlagGrp_CH3, CIU_EVET_FREAME_END|CIU_EVET_TOPFILD_END ,OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: ciuFlagGrp_CH3 is %d.\n", err);
			//return ;
		}


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
            diuRegConfig(MD_CIU3_ID, 640,240,640*2);
        }
    #endif


	}
}


void ciuTask_CH4(void* pData)
{
	u8 err;
    u32 waitFlag;

	while (1)
	{
	    waitFlag = OSFlagPend(ciuFlagGrp_CH4, CIU_EVET_FREAME_END|CIU_EVET_TOPFILD_END ,OS_FLAG_WAIT_SET_ANY|OS_FLAG_CONSUME, OS_IPC_WAIT_FOREVER, &err);
        if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: ciuFlagGrp_CH4 is %d.\n", err);
			//return ;
		}

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
            diuRegConfig(MD_CIU4_ID, 640,240,640*2);
        }
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
#if( DEBIG_CIU_FRAM_END_INTR_USE_LED6 &&((HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION  == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD))|| \
    (HW_BOARD_OPTION  == A1018B_FPGA_BOARD) ||(HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
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
    static u8   ClearBob    = 0;
    static u8   EnterBob    = 0;
    static u32  ciu1_ch_status = 100;
#if MULTI_CHANNEL_VIDEO_REC
    VIDEO_CLIP_OPTION   *pVideoClipOption   = &VideoClipOption[1];
#endif
    //------------//

    intStat = CIU_1_INTRPT;
#if CIU_DEBUG_INTR
    gpioSetLevel(0, 1, Debug_count & 0x01);
    Debug_count ++;
#endif
    ciu_1_FPS_Count++;
    if(intStat & CIU_INT_STAT_FIFO_OVERF)
    {
        DEBUG_CIU("CIU1 FIFO overflow\n");
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

					#if (HW_BOARD_OPTION == MR6730_AFN)
 					if ((CIU_1_CTL1 & 0x30) != CIU_MODE_656_BOB)
                    {
                    CIU_1_CTL1   = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                    CIU_1_CTL1  &= (~CIU_SCA_SHAREBUF_EN);				
					#if (CIU_BOB_MODE)
					g_CIU_BobMode|=0x01;
					#endif
					DEBUG_CIU("B");
 					}
					#else
					CIU_1_CTL1   = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
                    CIU_1_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
					#endif
                } else {
                    
					#if (HW_BOARD_OPTION == MR6730_AFN)
                    if ((CIU_1_CTL1 & 0x30) != CIU_MODE_656)
                    {		
                    CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656;
					#if (CIU_BOB_MODE)
					g_CIU_BobMode&=(~0x01);
					#endif
					DEBUG_CIU("b");
                    }
					#else
					CIU_1_CTL1  = (CIU_1_CTL1 & (~0x30)) | CIU_MODE_656;
					#endif					
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
            #if TV_DISP_BY_IDU

            #else
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
                        if((ciu_1_OpMode != SIUMODE_CAP_RREVIEW) || ((CIU_1_CTL1 & CIU_FIELD_POL_NEG) == 0))
                    #endif
                        IduVidBuf0Addr  = ((u32)PNBuf_sub[ ciu_idufrmcnt_ch1 & 0x03]);
                    #if NEW_IDU_BRI
                        BRI_IADDR_Y = IduVidBuf0Addr;
                        BRI_IADDR_C = BRI_IADDR_Y + ciu_1_pnbuf_size_y;
                      #if DUAL_MODE_DISP_SUPPORT
                       #if(HW_BOARD_OPTION == MR9670_WOAN) || (HW_BOARD_OPTION == MR6730_AFN)
                        if(sysDualModeDisp)
                        {
                            BRI_IN_SIZE = (ciu_1_OutWidth * 2) | (ciu_1_OutHeight << 16);
                            BRI_STRIDE  = ciu_1_OutWidth * 4;
                        }
                        else
                        {
                            BRI_IN_SIZE = ciu_1_OutWidth | ((ciu_1_OutHeight * 2) << 16);
                            BRI_STRIDE  = ciu_1_OutWidth * 2;
                        }
                       #else
                        if(sysDualModeDisp)
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_1_OutWidth * 2);
                        else
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_1_OutWidth);
                      #endif
                    #endif
                    #endif
                   }
                }
            #endif
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
                       #if(HW_BOARD_OPTION == MR9670_WOAN) || (HW_BOARD_OPTION == MR6730_AFN)
                        if(sysDualModeDisp)
                        {
                            BRI_IN_SIZE = (ciu_1_OutWidth * 2) | (ciu_1_OutHeight << 16);
                            BRI_STRIDE  = ciu_1_OutWidth * 4;
                        }
                        else
                        {
                            BRI_IN_SIZE = ciu_1_OutWidth | ((ciu_1_OutHeight * 2) << 16);
                            BRI_STRIDE  = ciu_1_OutWidth * 2;
                        }
                       #else
                        if(sysDualModeDisp)
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_1_OutWidth * 2);
                        else
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_1_OutWidth);
                      #endif
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
                //DEBUG_CIU("-->%d ",ciu_idufrmcnt_ch1);
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


      #if ISUCIU_PREVIEW_PNOUT
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
        #if (CIU_DECIMATION_TEST || (CIU_SP_TEST))
        CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX);
        CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y + ciu_1_OutY * ciu_1_line_stride / 2 + ciu_1_OutX);
        #endif
      #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + (ciu_1_OutY * ciu_1_line_stride + ciu_1_OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch1 + 1) & 0x03] + ciu_1_pnbuf_size_y);
      #endif
      //--------------------------------------------------------------------------------------------------------------------//
    #if( (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) )
      if(ciu1ZoomStart) 
      {
         if(ciu1ZoomOnOff == 0)
         {
            ciu1MotorXpos=0;
            ciu1MotorYpos=0;
         }
         rfiuciu1ZoomIn(ciu1ZoomOnOff,(ciu1ZoomXpos+ciu1MotorXpos),(ciu1ZoomYpos+ciu1MotorYpos));
         ciu1ZoomStart=0;
      }
    #endif
    
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
#if FINE_TIME_STAMP
    s32 IISTimeOffsetCIU, TimeOffset;
    u32 IISTime1;
#endif
    u8  **PNBuf_sub;
    u8          err;
    static u32  ciu2_ch_status = 100;
    //==================================//

#if MULTI_CHANNEL_VIDEO_REC
    VIDEO_CLIP_OPTION   *pVideoClipOption   = &VideoClipOption[2];
#endif

    intStat = CIU_2_INTRPT;

    if(intStat & CIU_INT_STAT_FIFO_OVERF)
    {
        DEBUG_CIU("CIU2 FIFO overflow\n");
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

   #if (MPEG4_FIELD_ENC_ENA && (CIU2_OPTION == Sensor_CCIR656) )

   #elif (CIU1_BOB_REPLACE_MPEG_DF && (CHIP_OPTION == CHIP_A1016A) ) //Lucian: 不用 Mpeg4 double field,起動 ciu mob mode
    if(ciu_2_OpMode != SIUMODE_CAP_RREVIEW)
    {
     #if MULTI_CHANNEL_VIDEO_REC
       #if( CIU2_OPTION == Sensor_CCIR656 )
        if(pVideoClipOption->video_double_field_flag)
        {
           CIU_2_CTL1   = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
           CIU_2_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
        }
        else
        {
           CIU_2_CTL1= (CIU_2_CTL1 & (~0x30)) | CIU_MODE_656;
        }
       #elif(CIU2_OPTION == Sensor_CCIR601)
        if(pVideoClipOption->video_double_field_flag)
        {
           CIU_2_CTL1   = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
           CIU_2_CTL1  &= (~CIU_SCA_SHAREBUF_EN);
        }
        else
        {
           CIU_2_CTL1   = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_601;
        }
       #endif
     #else
       #if( CIU2_OPTION == Sensor_CCIR656 )
        if(video_double_field_flag)
        {
           CIU_2_CTL1   = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
           CIU_2_CTL1  &=  (~CIU_SCA_SHAREBUF_EN);
        }
        else
        {
           CIU_2_CTL1   = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_656;
        }
       #elif(CIU2_OPTION == Sensor_CCIR601)
        if(video_double_field_flag)
        {
           CIU_2_CTL1   = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_656_BOB;
           CIU_2_CTL1  &=  (~CIU_SCA_SHAREBUF_EN);
        }
        else
        {
           CIU_2_CTL1   = (CIU_2_CTL1 & (~0x30)) | CIU_MODE_601;
        }
       #endif
     #endif     // #if MULTI_CHANNEL_VIDEO_REC
     }
   #endif       // #if (MPEG4_FIELD_ENC_ENA && (CIU2_OPTION ==Sensor_CCIR656) )

   //==========================HW Motion Detection/HW Deinterlace(A1018A)=======================//
   #if (HW_MD_SUPPORT)
    if(ciu2_ch_status != ciu_idufrmcnt_ch2)
    {
        OSFlagPost(ciuFlagGrp_CH2, CIU_EVET_FREAME_END, OS_FLAG_SET, &err);
        ciu2_ch_status = ciu_idufrmcnt_ch2;
    }
   #endif

   #if MULTI_CHANNEL_SUPPORT
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
            #if TV_DISP_BY_IDU

            #else
                if(sysVideoInCHsel == 0x02)
                {
                #if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
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
                        IduVidBuf0Addr=((u32)PNBuf_sub[ ciu_idufrmcnt_ch2 & 0x03]);
                    #if NEW_IDU_BRI
                        BRI_IADDR_Y = IduVidBuf0Addr;
                        BRI_IADDR_C = BRI_IADDR_Y + ciu_2_pnbuf_size_y;
                      #if DUAL_MODE_DISP_SUPPORT
                       #if(HW_BOARD_OPTION == MR9670_WOAN) || (HW_BOARD_OPTION == MR6730_AFN)
                        if(sysDualModeDisp)
                        {
                            BRI_IN_SIZE = (ciu_2_OutWidth * 2) | (ciu_2_OutHeight << 16);
                            BRI_STRIDE  = ciu_2_OutWidth * 4;
                        }
                        else
                        {
                            BRI_IN_SIZE = ciu_2_OutWidth | ((ciu_2_OutHeight * 2) << 16);
                            BRI_STRIDE  = ciu_2_OutWidth * 2;
                        }
                       #else
                        if(sysDualModeDisp)
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_2_OutWidth * 2);
                        else
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_2_OutWidth);
                      #endif
                    #endif
                    #endif
                   }
                }
            #endif
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
                       #if(HW_BOARD_OPTION == MR9670_WOAN) || (HW_BOARD_OPTION == MR6730_AFN)
                        if(sysDualModeDisp)
                        {
                            BRI_IN_SIZE = (ciu_2_OutWidth * 2) | (ciu_2_OutHeight << 16);
                            BRI_STRIDE  = ciu_2_OutWidth * 4;
                        }
                        else
                        {
                            BRI_IN_SIZE = ciu_2_OutWidth | ((ciu_2_OutHeight * 2) << 16);
                            BRI_STRIDE  = ciu_2_OutWidth * 2;
                        }
                       #else
                        if(sysDualModeDisp)
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_2_OutWidth * 2);
                        else
                            BRI_IN_SIZE = (BRI_IN_SIZE & 0xffff0000) | (ciu_2_OutWidth);
                      #endif
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
           }
        #else
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
        			if(IISTimeOffset >= IISTimeOffsetCIU) {
        				TimeOffset  = IISTimeOffset - IISTimeOffsetCIU;
        			} else {
        			    TimeOffset  = IISTimeOffset + (TimerGetTimerCounter(TIMER_1) / 100) - IISTimeOffsetCIU;
        			}
        			if(TimeOffset > IISTimeUnit) {
        				TimeOffset  = IISTimeUnit;
        			}
        			IISTime1    = IISTime + TimeOffset;
        			if(IISTime1 > ciu_2_FrameTime) {
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
                    //DEBUG_CIU("A");
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
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y + ciu_2_OutY * ciu_2_line_stride / 2 + ciu_2_OutX);
    #else
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + (ciu_2_OutY * ciu_2_line_stride + ciu_2_OutX) * 2);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch2 + 1) & 0x03] + ciu_2_pnbuf_size_y);
    #endif

   #endif
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

   #if MULTI_CHANNEL_SUPPORT
        switch(sysPIPMain)
        {
        case    PIP_MAIN_CH2:
            PNBuf_sub   = (u8**)PNBuf_sub1;
            break;
        default:
            PNBuf_sub   = (u8**)PNBuf_sub3;
            break;
        }

        if((sysPIPMain == PIP_MAIN_NONE) || (sysPIPMain == PIP_MAIN_CH1))
        {
            if(sysTVOutOnFlag)
            {
            #if TV_DISP_BY_IDU

            #else
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
                        IduVidBuf0Addr=((u32)PNBuf_sub[ ciu_idufrmcnt_ch3 & 0x03]);
                    #if NEW_IDU_BRI
                        BRI_IADDR_Y = IduVidBuf0Addr;
                        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                    #endif
                   }
                }
            #endif
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
        }

       if(ciu_3_OpMode == SIUMODE_MPEGAVI)
       {
        #if MULTI_CHANNEL_VIDEO_REC
           if((pVideoClipOption->VideoPictureIndex + 2) >= ciu_idufrmcnt_ch3)
           {
                if((sysPIPMain == PIP_MAIN_NONE) ||
                    ((sysPIPMain == PIP_MAIN_CH1) && ((ciu_idufrmcnt_ch3 + 1) < ciu_idufrmcnt_ch1)) ||
                    ((sysPIPMain == PIP_MAIN_CH2) && (ciu_idufrmcnt_ch3 < (ciu_idufrmcnt_ch1 + 2))))
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
            if(sysPIPMain == PIP_MAIN_NONE)
            {
                ciu_idufrmcnt_ch3 ++;
            }
            else if(sysPIPMain == PIP_MAIN_CH1)
            {
                if((ciu_idufrmcnt_ch3 + 1) < ciu_idufrmcnt_ch1)
                {
                    ciu_idufrmcnt_ch3 ++;
                    //CiuSetIduAdr    = 1;
                    //DEBUG_CIU("A");
                }
            }
            else if(sysPIPMain == PIP_MAIN_CH2)
            {
                u32 temp = (u32)PNBuf_sub[(ciu_idufrmcnt_ch3 + 2) & 3];
                if((ciu_idufrmcnt_ch3 < (ciu_idufrmcnt_ch1 + 2)) && (temp != (u32)IduVidBuf0Addr))
                    ciu_idufrmcnt_ch3 ++;
            }
            //DEBUG_CIU("%d%d ", ciu_idufrmcnt_ch1 & 0x03, ciu_idufrmcnt_ch2 & 0x03);
       }


    #if ISUCIU_PREVIEW_PNOUT
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch3 + 1) & 0x03] + ciu_3_OutY * ciu_3_line_stride + ciu_3_OutX);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch3 + 1) & 0x03] + PNBUF_SIZE_Y + ciu_3_OutY * ciu_3_line_stride / 2 + ciu_3_OutX);
    #else
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch3 + 1) & 0x03] + (ciu_3_OutY * ciu_3_line_stride + ciu_3_OutX) * 2);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch3 + 1) & 0x03] + PNBUF_SIZE_Y);
    #endif

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

   #if MULTI_CHANNEL_SUPPORT
        switch(sysPIPMain)
        {
        case    PIP_MAIN_CH2:
            PNBuf_sub   = (u8**)PNBuf_sub1;
            break;
        default:
            PNBuf_sub   = (u8**)PNBuf_sub4;
            break;
        }

        if((sysPIPMain == PIP_MAIN_NONE) || (sysPIPMain == PIP_MAIN_CH1))
        {
            if(sysTVOutOnFlag)
            {
            #if TV_DISP_BY_IDU

            #else
                if(sysVideoInCHsel == 0x04)
                {
                #if (QUARD_MODE_DISP_SUPPORT || (SW_APPLICATION_OPTION == FPGA_VERIFY_TEST_GFU) )
                   if(  (sysCameraMode == SYS_CAMERA_MODE_CIU_QUADSCR) || (sysCameraMode == SYS_CAMERA_MODE_GFU_TESTSCR) )
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
                        IduVidBuf0Addr=((u32)PNBuf_sub[ ciu_idufrmcnt_ch4 & 0x03]);
                    #if NEW_IDU_BRI
                        BRI_IADDR_Y = IduVidBuf0Addr;
                        BRI_IADDR_C = BRI_IADDR_Y + PNBUF_SIZE_Y;
                    #endif
                   }
                }
            #endif
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
        }

       if(ciu_4_OpMode == SIUMODE_MPEGAVI)
       {
        #if MULTI_CHANNEL_VIDEO_REC
           if((pVideoClipOption->VideoPictureIndex + 2) >= ciu_idufrmcnt_ch4)
           {
                if((sysPIPMain == PIP_MAIN_NONE) ||
                    ((sysPIPMain == PIP_MAIN_CH1) && ((ciu_idufrmcnt_ch4 + 1) < ciu_idufrmcnt_ch1)) ||
                    ((sysPIPMain == PIP_MAIN_CH2) && (ciu_idufrmcnt_ch4 < (ciu_idufrmcnt_ch1 + 2))))
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
            if(sysPIPMain == PIP_MAIN_NONE)
            {
                ciu_idufrmcnt_ch4 ++;
            }
            else if(sysPIPMain == PIP_MAIN_CH1)
            {
                if((ciu_idufrmcnt_ch4 + 1) < ciu_idufrmcnt_ch1)
                {
                    ciu_idufrmcnt_ch4 ++;
                    //CiuSetIduAdr    = 1;
                    //DEBUG_CIU("A");
                }
            }
            else if(sysPIPMain == PIP_MAIN_CH2)
            {
                u32 temp = (u32)PNBuf_sub[(ciu_idufrmcnt_ch4 + 2) & 3];
                if((ciu_idufrmcnt_ch4 < (ciu_idufrmcnt_ch1 + 2)) && (temp != (u32)IduVidBuf0Addr))
                    ciu_idufrmcnt_ch4 ++;
            }
            //DEBUG_CIU("%d%d ", ciu_idufrmcnt_ch1 & 0x03, ciu_idufrmcnt_ch2 & 0x03);
       }


    #if ISUCIU_PREVIEW_PNOUT
        CIU_4_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch4 + 1) & 0x03] + ciu_4_OutY * ciu_4_line_stride + ciu_4_OutX);
        CIU_4_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch4 + 1) & 0x03] + PNBUF_SIZE_Y + ciu_4_OutY * ciu_4_line_stride / 2 + ciu_4_OutX);
    #else
        CIU_4_STR_YADDR     = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch4 + 1) & 0x03] + (ciu_4_OutY * ciu_4_line_stride + ciu_4_OutX) * 2);
        CIU_4_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[(ciu_idufrmcnt_ch4 + 1) & 0x03] + PNBUF_SIZE_Y);
    #endif

   #endif
   }

   if(intStat & CIU_INT_STAT_FIELD_END)
   {
   #if HW_DEINTERLACE_CIU4_ENA
       OSFlagPost(ciuFlagGrp_CH4, CIU_EVET_TOPFILD_END, OS_FLAG_SET, &err);
   #endif
   }

}

int ciu1_ChangeInputSize(int InWidth,int InHeight)
{
     CIU_1_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
}

int ciu2_ChangeInputSize(int InWidth,int InHeight)
{
     CIU_2_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
}

int ciu3_ChangeInputSize(int InWidth,int InHeight)
{
     CIU_3_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
}

int ciu4_ChangeInputSize(int InWidth,int InHeight)
{
     CIU_4_InputSize     = ( (InWidth*2)<<CIU_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_INPUT_SIZE_Y_SHFT);
}

#define CIU_OSD_CHARMAX  32

s32 ciuPreviewInit_CH1(u8 mode,u32 InWidth, u32 InHeight, u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en, u32 line_stride)
{
    u8  **PNBuf_sub;
    u32 temp;
    u8  err;
    u32 newCIU1_tmp = 0;
#if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION  == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD) || \
    (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018B_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    char ciuOSDStr1[CIU_OSD_CHARMAX+16]="CH_1DEFGHIJKLMNP123456789QWERTYU";
#elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) )
    char ciuOSDStr1[CIU_OSD_CHARMAX+16]="CH_1DEFGHIJKLMNP123456789QWERTYU";
#else
    char ciuOSDStr1[CIU_OSD_CHARMAX+16]="                                ";
#endif

#if (NEWCIU1_TEST_EN || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
    #if (QUARD_MODE_DISP_SUPPORT)
        #if NEWCIU1_SW_MPSP
        newCIU1_tmp = CIU_SA_SEL_DS | CIU_SP_SEL_SENSOR| CIU_MP_SEL_SC | CIU_DS_MODE_1_2;
        OutWidth = OutWidth/2;
        OutHeight = OutHeight/2;
        #else
        newCIU1_tmp = CIU_SA_SEL_SENSOR | CIU_SP_SEL_DS | CIU_MP_SEL_SC | CIU_DS_MODE_1_2;
        #endif
    #else
        #if CIU_SP_TEST
        newCIU1_tmp = CIU_SA_SEL_SENSOR | CIU_SP_SEL_DS | CIU_MP_SEL_SC | CIU_DS_MODE_1_2; 
        #else
        newCIU1_tmp = CIU_SA_SEL_SENSOR | CIU_SP_SEL_SENSOR | CIU_MP_SEL_SC | CIU_DS_MODE_DIS; 
        #endif
    #endif
#endif
    DEBUG_CIU("ciuPreviewInit_CH1(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n", mode, InWidth, InHeight, OutWidth, OutHeight, OutX, OutY, OSD_en, line_stride);

    //================//
	IISTime = 0;
	ciu_1_FrameTime = 0;    
#if DUAL_MODE_DISP_SUPPORT
    if((mode == SIUMODE_CAP_RREVIEW) || (mode == SIUMODE_PREVIEW_MENU))
    {
        ciu_1_pnbuf_size_y  = PNBUF_SIZE_Y;
    } 
    else 
    {
        line_stride         = line_stride * 2;
        ciu_1_pnbuf_size_y  = PNBUF_SIZE_Y * 2;
    }
#else
  #if(HW_BOARD_OPTION == MR8211_ZINWELL)
    ciu_1_pnbuf_size_y  = 640 * 480;
  #else
    ciu_1_pnbuf_size_y  = PNBUF_SIZE_Y;
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

#if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
    CIU_1_IMG_STR		= 0;
#else
  #if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
    #if NEWCIU1_TEST_EN
      CIU_1_IMG_STR		= 0;
    #else
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
    #endif
  #else
    CIU_1_IMG_STR		= 0;
  #endif
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
                   #if (NEWCIU1_TEST_EN || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
                         CIU_NEW_BURST16 |
                         CIU_NEW_SP_TIMING |
                   #endif
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
    			   #elif(CIU1_OPTION==Sensor_NT99141_YUV601)
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
                   #if CIU_SP_TEST
                         CIU_SUBPATH_EN |
                   #endif
                         CIU_656DATALATCH_RIS;

        CIU_1_FRAME_STRIDE   = line_stride*2;
    }
    else
    {
        CIU_1_CTL1       =
                   #if (NEWCIU1_TEST_EN || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
                         CIU_NEW_BURST16 |
                         CIU_NEW_SP_TIMING |
                   #endif
                   #if(CIU1_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif(CIU1_OPTION==Sensor_CCIR601)
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

                   #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif(CIU1_OPTION==Sensor_NT99141_YUV601)
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
                   #if CIU_SP_TEST
                         CIU_SUBPATH_EN |
                   #endif
                         CIU_656DATALATCH_RIS;
   #if ISUCIU_PREVIEW_PNOUT
        CIU_1_FRAME_STRIDE   = line_stride;
     #if (QUARD_MODE_DISP_SUPPORT || CIU_SP_TEST)
        CIU_1_SP_FRAME_STRIDE= line_stride;
     #endif
   #else
        CIU_1_FRAME_STRIDE   = line_stride*2;
     #if (QUARD_MODE_DISP_SUPPORT || CIU_SP_TEST)
        CIU_1_SP_FRAME_STRIDE= line_stride*2;
     #endif
   #endif
    }

#if (NEWCIU1_TEST_EN || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
    #if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
        #if NEWCIU1_SW_MPSP
        CIU_1_TotalSize      = line_stride/4;
        #else
        CIU_1_TotalSize      = line_stride/2;
        #endif
    #else
        #if NEWCIU1_SW_MPSP
        CIU_1_TotalSize      = line_stride/2;
        #else
        CIU_1_TotalSize      = InWidth; //line_stride; InWidth

        #endif
    #endif
#endif
#if CIU1_SCUP_EN
  #if (HW_BOARD_OPTION == MR8211_ZINWELL)
    if(0)
  #else
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
  #endif
    {
       CIU_1_CTL1 &= (~CIU_SCA_EN);
    #if((CHIP_OPTION == CHIP_A1016A))
       SdramArbit = (SdramArbit & (~0xff000000)) | 0x01000000;
    #endif
    }
    else
    {
         CIU_1_CTL1 |= (CIU_SCA_EN);
       
	#if((CHIP_OPTION == CHIP_A1016A))
	    #if((SW_APPLICATION_OPTION == MR8120_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX2) || (SW_APPLICATION_OPTION == MR8120_RFCAM_TX1_6M) || (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) )
	        SdramArbit = (SdramArbit & (~0xff000000)) | 0x3f000000;  //Lucian: walk aroudn A1016's bug @TX.
	    #else
		    SdramArbit = (SdramArbit & (~0xff000000)) | 0x01000000;
	    #endif
    #endif
    }
  #if (CHIP_OPTION == CHIP_A1020A || CHIP_OPTION == CHIP_A1026A)

    if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) && (InWidth == OutWidth) && (InHeight == OutHeight) )
    {
        CIU_1_CTL1 |=CIU_SCA_SHAREBUF_EN | CIU_SCA_EN;
    }
  #endif
#endif

    //DEBUG_CIU("sysTVinFormat    = %d\n", sysTVinFormat);
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
#if (NEWCIU1_TEST_EN || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
    #if NEWCIU1_SW_MPSP
    CIU_1_SA_InputSize = ( (OutWidth)<<CIU_SA_INPUT_SIZE_X_SHFT) | (OutHeight<<CIU_SA_INPUT_SIZE_Y_SHFT);
    #else
    //CIU_1_SA_InputSize = ( OutWidth<<CIU_SA_INPUT_SIZE_X_SHFT) | (OutHeight<<CIU_SA_INPUT_SIZE_Y_SHFT);
    CIU_1_SA_InputSize = ( (InWidth)<<CIU_SA_INPUT_SIZE_X_SHFT) | (InHeight<<CIU_SA_INPUT_SIZE_Y_SHFT);
    #endif
#endif
    // 設定第0張的位址
#if MULTI_CHANNEL_SUPPORT
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
        #if CIU_SP_TEST
        CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
        CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
        #endif
    #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y);
    #endif
    }

    #if (QUARD_MODE_DISP_SUPPORT)
       CIU_1_SP_STR_YADDR     = (unsigned int)PNBuf_Quad;
       CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_Quad + ciu_1_pnbuf_size_y);
    #endif
#else
    CIU_1_STR_YADDR     = (unsigned int)PNBuf_Y[0];
    CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_Y[0]+ciu_1_pnbuf_size_y);
#endif
#if IS_COMMAX_DOORPHONE || IS_HECHI_DOORPHONE || (HW_BOARD_OPTION == MR6730_AFN)
/*
    GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, ciuOSDStr1,
       					 	CIU_OSD_CHARMAX,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
        					6,20,6+24,19+2);
*/
    memset((u8*)szVideoOverlay1, 0, sizeof(szVideoOverlay1));
    memset((u8*)szVideoOverlay2, 0, sizeof(szVideoOverlay2));
    memset((u8*)ciuszString1, 0, sizeof(ciuszString1));
    uiDrawTimeOnVideoClip(1);
#elif(HW_BOARD_OPTION == MR9670_WOAN) //|| (HW_BOARD_OPTION == MR6730_AFN)
                GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot, "                    ",
                    288,ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                    10,20,10+24,19+2);
#else
    memset((u8*)ciuszString1, 0, sizeof(ciuszString1));
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                                ciuOSDStr1,  CIU_OSD_CHARMAX,
                                ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                                0,0,0+8,0+4);
    else    // VGA
        GenerateCIU1_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                                ciuOSDStr1,  CIU_OSD_CHARMAX,
                                ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                                0,0,0+8,0+4);

    #if(CIU_DECIMATION_TEST)
    GenerateCIU1_SP_OSD2Bits(CiuOverlayImg1_Top, CiuOverlayImg1_Bot,
                            ciuOSDStr1,  CIU_OSD_CHARMAX,
                            ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                            0,0,0+8,0+4);
    #endif
    #if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
    memset((u8*)ciuszString1_SP, 0, sizeof(ciuszString1_SP));
    #endif
    #if CIU_SP_TEST
    GenerateCIU1_SP_OSD2Bits(CiuOverlayImg1_SP_Top, CiuOverlayImg1_SP_Bot,
                            ciuOSDStr1,  CIU_OSD_CHARMAX/2,
                            ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                            0,0,0+8,0+4);
    #endif
#endif

    if(OSD_en)
    {
        CIU_1_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_1_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_1_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #if (CIU_DECIMATION_TEST || CIU_SP_TEST)
        CIU_1_SP_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_1_SP_OVL_IDXCOLOR_CB  = 0x00ffffff;
        CIU_1_SP_OVL_IDXCOLOR_CR  = CIU_CR_IDX;
        #endif

        CIU_1_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                     CIU_VSYNC_ACT_LO |
                     CIU_HSYNC_ACT_LO |
                     CIU_INT_DISA_FIFO_OVERF |
                     CIU_INT_ENA_FRAME_END |
                     CIU_DATA_OUT_ENA |
                #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
 			         CIU_YUVDATALATCH_POS |
                #else
 			         CIU_YUVDATALATCH_NEG |
 			    #endif
                     CIU_TESTIMG_DISA |
                     //CIU_TESTIMG_02 |
                     CIU_INT_DISA_FIELD_END |
                     CIU_EXT_FIELD_EN |
                #if HW_DEINTERLACE_CIU1_ENA
                     CIU_INT_ENA_TOPEND |
                #endif
                #if (NEWCIU1_TEST_EN || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A))
                     newCIU1_tmp |
                #endif
                #if (CIU_DECIMATION_TEST || CIU_SP_TEST)
                     CIU_SP_OSD_ENA |
                #endif
                     CIU_OSD_ENA;

    }
    else
    {
        CIU_1_CTL2 = CIU_NORMAL |
                     CIU_ENA |
                     CIU_VSYNC_ACT_LO |
                     CIU_HSYNC_ACT_LO |
                     CIU_INT_DISA_FIFO_OVERF |
                     CIU_INT_ENA_FRAME_END |
                     CIU_DATA_OUT_ENA |
                #if((CIU1_OPTION==Sensor_OV7725_YUV601)||(CIU1_OPTION==Sensor_OV2643_YUV601))
 			        CIU_YUVDATALATCH_POS |
                #else
 			        CIU_YUVDATALATCH_NEG |
 			    #endif
                     CIU_TESTIMG_DISA |
                     //CIU_TESTIMG_01 |
                     CIU_INT_DISA_FIELD_END |
                     CIU_EXT_FIELD_EN |
                #if HW_DEINTERLACE_CIU1_ENA
                     CIU_INT_ENA_TOPEND |
                #endif
                #if (NEWCIU1_TEST_EN || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
                     newCIU1_tmp |
                #endif
                     CIU_OSD_DISA;
    }
    #if(TV_DECODER == TW2866)
        CIU_1_CTL2 |= CIU_SPLITER_ENA; //enable CCIR4ch
    #endif
    #if(CHIP_OPTION == CHIP_A1018A)
        CIU_1_INTRPT |=CIU_INT_DATA_CMP ;
    #endif
    
    // 設定第1張的位址
    if(mode ==SIUMODE_CAP_RREVIEW)
    {
    #if MULTI_CHANNEL_SUPPORT
      #if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
        if((CIU_1_CTL1 & CIU_FIELD_POL_NEG) == 0)   // 若field有交換的話,第一張影像要輸出到第零張的位址
        {
        #if IS_COMMAX_DOORPHONE
            CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + (OutY * line_stride + OutX) * 2);
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y);
        #else
            CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y + (OutY * line_stride + OutX) * 2);
            CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y);
        #endif      
        }
      #else
        #if((CHIP_OPTION == CHIP_A1016A))//walk around A1016 CIU's bug
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y);     
        #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y);     
        #endif
      #endif   
        
    #else
       #if((CHIP_OPTION == CHIP_A1016A)) //walk around A1016 CIU's bug
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_Y[0] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_Y[0] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y);
       #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_Y[1] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_Y[1] + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y + ciu_1_pnbuf_size_y);
       #endif
    #endif
    }
    else if(mode == SIUMODE_PREVIEW_MENU)
    {
        CIU_1_STR_YADDR     = (unsigned int)PKBuf0;
        CIU_1_STR_CbCrADDR  = (unsigned int)PKBuf0;
    }
    else
    {
    #if MULTI_CHANNEL_SUPPORT
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
        #if CIU_SP_TEST
        CIU_1_SP_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
        CIU_1_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y + OutY * line_stride / 2 + OutX);
        #endif
      #else
        CIU_1_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + (OutY * line_stride + OutX) * 2);
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_1_pnbuf_size_y);
      #endif
    #else
        CIU_1_STR_YADDR     = (unsigned int)PNBuf_Y[1];
        CIU_1_STR_CbCrADDR  = (unsigned int)(PNBuf_Y[1]+ciu_1_pnbuf_size_y);
    #endif
    }
    #if CIU_SPLITER
    CIU_1_SPILTER_CTL = 0x32100000;
    //DEBUG_I2C("CIU_1_SPILTER_CTL =%x\n",CIU_1_SPILTER_CTL);    
    #endif

}

s32 ciuPreviewInit_CH2(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride)
{

#if MULTI_CHANNEL_SUPPORT
    u8  **PNBuf_sub;
    u32 temp;
    u8  err;
 #if( (HW_BOARD_OPTION  == A1013_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016_FPGA_BOARD) || (HW_BOARD_OPTION  == A1016B_FPGA_BOARD) || \
    (HW_BOARD_OPTION  == A1020A_FPGA_BOARD) || (HW_BOARD_OPTION  == A1018_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018B_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    char ciuOSDStr2[CIU_OSD_CHARMAX+16]="CH_2DEFGHIJKLMNO123456789QWERTYU";
 #elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375))
    char ciuOSDStr2[CIU_OSD_CHARMAX+16]="CH_2DEFGHIJKLMNP123456789QWERTYU";
 #else
    char ciuOSDStr2[CIU_OSD_CHARMAX+16]="                                ";
 #endif

    DEBUG_CIU("ciuPreviewInit_CH2(%d, %d, %d, %d, %d, %d, %d, %d, %d)\n", mode, InWidth, InHeight, OutWidth, OutHeight, OutX, OutY, OSD_en, line_stride);

    //=================================//
#if DUAL_MODE_DISP_SUPPORT
    if((mode == SIUMODE_CAP_RREVIEW) || (mode == SIUMODE_PREVIEW_MENU))
    {
        ciu_2_pnbuf_size_y  = PNBUF_SIZE_Y;
    } else {
        line_stride         = line_stride * 2;
        ciu_2_pnbuf_size_y  = PNBUF_SIZE_Y * 2;
    }
#else
  #if(HW_BOARD_OPTION == MR8211_ZINWELL)
    ciu_2_pnbuf_size_y  = line_stride * OutHeight;
  #else
    ciu_2_pnbuf_size_y  = PNBUF_SIZE_Y;
  #endif
#endif

    ciu_idufrmcnt_ch2   = 0;  //0;
    OSSemSet(ciuCapSemEvt_CH2, 0, &err);
    if(mode == SIUMODE_MPEGAVI)
        ciu_2_OpMode = SIUMODE_MPEGAVI;
    else if(mode == SIUMODE_PREVIEW_MENU)
        ciu_2_OpMode = SIUMODE_PREVIEW;
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

    if(mode ==SIUMODE_CAP_RREVIEW)
    {
        CIU_2_CTL1       =
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
    			   #elif(CIU2_OPTION==Sensor_NT99141_YUV601)
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


        CIU_2_FRAME_STRIDE   = line_stride * 2;
    }
    else
    {
        CIU_2_CTL1       =
                   #if(CIU2_OPTION==Sensor_CCIR656)
                         CIU_MODE_656 |
                   #elif(CIU2_OPTION==Sensor_CCIR601)
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

                   #if((CIU2_OPTION==Sensor_OV7725_YUV601)||(CIU2_OPTION==Sensor_OV2643_YUV601))
    			         CIU_YUVMAP_27 |
    			   #elif(CIU2_OPTION==Sensor_NT99141_YUV601)
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
        CIU_2_FRAME_STRIDE   = line_stride;
     #if (QUARD_MODE_DISP_SUPPORT)
        CIU_2_SP_FRAME_STRIDE= line_stride;
     #endif
#else
        CIU_2_FRAME_STRIDE   = line_stride * 2;
     #if (QUARD_MODE_DISP_SUPPORT)
        CIU_2_SP_FRAME_STRIDE= line_stride * 2;
     #endif
#endif
    }

#if CIU2_SCUP_EN
  #if (HW_BOARD_OPTION == MR8211_ZINWELL)
    if(0)
  #else
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
  #endif
    {
        CIU_2_CTL1 &= (~CIU_SCA_EN);
    #if((CHIP_OPTION == CHIP_A1016A))
        SdramArbit = (SdramArbit & (~0xff000000)) | 0x01000000;
    #endif
    }
    else
    {
        CIU_2_CTL1 |= (CIU_SCA_EN);
    #if((CHIP_OPTION == CHIP_A1016A))
     #if (SW_APPLICATION_OPTION != MR9670_DOORPHONE)
        SdramArbit = (SdramArbit & (~0xff000000)) | 0x3f000000; //Lucian: walk aroudn A1016's bug.
     #endif
    #endif
    }

  #if(CHIP_OPTION >= CHIP_A1018A)
    if( (InWidth== OutWidth) && (InHeight==OutHeight) )
    {
        CIU_2_CTL1 |=CIU_SCA_SHAREBUF_EN | CIU_SCA_EN;
    }
  #endif
#endif
#if IS_COMMAX_DOORPHONE || (HW_BOARD_OPTION == MR9670_WOAN)
    if(((sysPIPMain == PIP_MAIN_CH1) || (sysVideoInCHsel == 2)) && (mode != SIUMODE_CAP_RREVIEW) && (mode != SIUMODE_PREVIEW_MENU))
    {
        CIU_2_CTL1 |= CIU_MODE_656_BOB;
        CIU_2_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }
#else
    if(sysPIPMain == PIP_MAIN_CH1)
    {
        CIU_2_CTL1 |= CIU_MODE_656_BOB;
        CIU_2_CTL1 &=  (~CIU_SCA_SHAREBUF_EN);
    }
#endif

    CIU_2_OutputSize    = ( OutWidth<<CIU_OUTPUT_SIZE_X_SHFT) | (OutHeight<<CIU_OUTPUT_SIZE_Y_SHFT);

    // 設定第0張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
    #else
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y);
    #endif
    }

#if (QUARD_MODE_DISP_SUPPORT)
    CIU_2_SP_STR_YADDR     = (unsigned int)PNBuf_Quad + InWidth/2;
    CIU_2_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_Quad + ciu_2_pnbuf_size_y + InWidth/2);
#endif

#if (HW_BOARD_OPTION == MR6730_AFN)
    memset((u8*)szVideoOverlay1, 0, sizeof(szVideoOverlay1));
    memset((u8*)szVideoOverlay2, 0, sizeof(szVideoOverlay2));
    memset((u8*)ciuszString2, 0, sizeof(ciuszString2));
    uiDrawTimeOnVideoClip(1);
#else
    memset((u8*)ciuszString2, 0, sizeof(ciuszString2));
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot,
                              ciuOSDStr2,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    else    // VGA
        GenerateCIU2_OSD2Bits(CiuOverlayImg2_Top, CiuOverlayImg2_Bot,
                              ciuOSDStr2,  CIU_OSD_CHARMAX,
                              ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
#endif
    if(OSD_en)
    {
        CIU_2_OVL_IDXCOLOR_Y   = CIU_Y_IDX;
        CIU_2_OVL_IDXCOLOR_CB  = CIU_CB_IDX;
        CIU_2_OVL_IDXCOLOR_CR  = CIU_CR_IDX;

        CIU_2_CTL2 = CIU_NORMAL |
                    CIU_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_DATA_OUT_ENA |
                    CIU_YUVDATALATCH_NEG |
                    CIU_TESTIMG_DISA |
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if HW_DEINTERLACE_CIU2_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_ENA;
    }
    else
    {
        CIU_2_CTL2 = CIU_NORMAL |
                    CIU_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_DATA_OUT_ENA |
                    CIU_YUVDATALATCH_NEG |
                    CIU_TESTIMG_DISA |
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if HW_DEINTERLACE_CIU2_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_DISA;
    }
    #if(TV_DECODER == TW2866)
        CIU_2_CTL2 |= CIU_SPLITER_ENA; //enable CCIR4ch
    #endif
    #if(CHIP_OPTION == CHIP_A1018A)
        CIU_2_INTRPT |=CIU_INT_DATA_CMP ;
    #endif

    // 設定第1張的位址
    if(mode == SIUMODE_CAP_RREVIEW)
    {
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + ciu_2_pnbuf_size_y + (OutY * line_stride + OutX) * 2);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + ciu_2_pnbuf_size_y + ciu_2_pnbuf_size_y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + OutY * line_stride + OutX);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
    #else
        CIU_2_STR_YADDR     = (unsigned int)(PNBuf_sub[1] + (OutY * line_stride + OutX) * 2);
        CIU_2_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[1] + ciu_2_pnbuf_size_y);
    #endif
    }
#endif
}

s32 ciuPreviewInit_CH3(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride)
{
#if MULTI_CHANNEL_SUPPORT
    u8  **PNBuf_sub;
    u32 temp;
    u8  err;
 #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018B_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    char ciuOSDStr3[CIU_OSD_CHARMAX+16]="CH_3DEFGHIJKLMNO123456789QWERTYU";
 #elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) )
    char ciuOSDStr3[CIU_OSD_CHARMAX+16]="CH_3DEFGHIJKLMNP123456789QWERTYU";
 #else
    char ciuOSDStr3[CIU_OSD_CHARMAX+16]="                                ";
 #endif
    //=================================//

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
    			   #elif(CIU3_OPTION==Sensor_NT99141_YUV601)
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
    			   #elif(CIU3_OPTION==Sensor_NT99141_YUV601)
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
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        CIU_3_CTL1 &= (~CIU_SCA_EN);
    #if((CHIP_OPTION == CHIP_A1016A))
        SdramArbit = (SdramArbit & (~0xff000000)) | 0x01000000;
    #endif
    }
    else
    {
        CIU_3_CTL1 |= (CIU_SCA_EN);
    #if((CHIP_OPTION == CHIP_A1016A))
        SdramArbit = (SdramArbit & (~0xff000000)) | 0x3f000000; //Lucian: walk aroudn A1016's bug.
    #endif
    }

  #if(CHIP_OPTION >= CHIP_A1018A)
    if( (InWidth== OutWidth) && (InHeight==OutHeight) )
    {
        CIU_3_CTL1 |=CIU_SCA_SHAREBUF_EN| CIU_SCA_EN;
    }
  #endif
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
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y);
    }
    else
    {
    #if ISUCIU_PREVIEW_PNOUT
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + OutY * line_stride + OutX);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y + OutY * line_stride / 2 + OutX);
    #else
        CIU_3_STR_YADDR     = (unsigned int)(PNBuf_sub[0] + (OutY * line_stride + OutX) * 2);
        CIU_3_STR_CbCrADDR  = (unsigned int)(PNBuf_sub[0] + ciu_2_pnbuf_size_y);
    #endif
    }

#if (QUARD_MODE_DISP_SUPPORT)
    CIU_3_SP_STR_YADDR     = (unsigned int)PNBuf_Quad + (InHeight/2)*line_stride ;
    CIU_3_SP_STR_CbCrADDR  = (unsigned int)(PNBuf_Quad + ciu_2_pnbuf_size_y + (InHeight/2/2)*line_stride);
#endif

    memset((u8*)ciuszString3, 0, sizeof(ciuszString3));
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        GenerateCIU3_OSD2Bits(CiuOverlayImg3_Top, CiuOverlayImg3_Bot,
                              ciuOSDStr3,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    else    // VGA
        GenerateCIU3_OSD2Bits(CiuOverlayImg3_Top, CiuOverlayImg3_Bot,
                              ciuOSDStr3,  CIU_OSD_CHARMAX,
                              ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);

    if(OSD_en)
    {
        CIU_3_OVL_IDXCOLOR_Y   = 0x00ffffff;
        CIU_3_OVL_IDXCOLOR_CB  = 0x00808080;
        CIU_3_OVL_IDXCOLOR_CR  = 0x00808080;

        CIU_3_CTL2 = CIU_NORMAL |
                    CIU_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_DATA_OUT_ENA |
                    CIU_YUVDATALATCH_NEG |
                    CIU_TESTIMG_DISA |
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if HW_DEINTERLACE_CIU3_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_ENA;
    }
    else
    {
        CIU_3_CTL2 = CIU_NORMAL |
                    CIU_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_DATA_OUT_ENA |
                    CIU_YUVDATALATCH_NEG |
                    CIU_TESTIMG_DISA |
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if HW_DEINTERLACE_CIU3_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_DISA;
    }
    #if(TV_DECODER == TW2866)
        CIU_3_CTL2 |= CIU_SPLITER_ENA; //enable CCIR4ch
    #endif
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
#endif
}

s32 ciuPreviewInit_CH4(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride)
{
#if MULTI_CHANNEL_SUPPORT
    u8  **PNBuf_sub;
    u32 temp;
    u8  err;
 #if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || \
    (HW_BOARD_OPTION  == A1020A_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| (HW_BOARD_OPTION  == A1018B_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    char ciuOSDStr4[CIU_OSD_CHARMAX+16]="CH_4DEFGHIJKLMNO123456789QWERTYU";
 #elif( (HW_BOARD_OPTION  == A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) )
    char ciuOSDStr4[CIU_OSD_CHARMAX+16]="CH_4DEFGHIJKLMNP123456789QWERTYU";
 #else
    char ciuOSDStr4[CIU_OSD_CHARMAX+16]="                                ";
 #endif
    //=================================//

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
    			   #elif(CIU4_OPTION==Sensor_NT99141_YUV601)
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
    			   #elif(CIU4_OPTION==Sensor_NT99141_YUV601)
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
    if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
    {
        CIU_4_CTL1 &= (~CIU_SCA_EN);
    #if((CHIP_OPTION == CHIP_A1016A))
        SdramArbit = (SdramArbit & (~0xff000000)) | 0x01000000;
    #endif
    }
    else
    {
        CIU_4_CTL1 |= (CIU_SCA_EN);
    #if((CHIP_OPTION == CHIP_A1016A))
        SdramArbit = (SdramArbit & (~0xff000000)) | 0x3f000000; //Lucian: walk aroudn A1016's bug.
    #endif
    }

  #if(CHIP_OPTION >= CHIP_A1018A)
    if( (InWidth== OutWidth) && (InHeight==OutHeight) )
    {
        CIU_4_CTL1 |=CIU_SCA_SHAREBUF_EN| CIU_SCA_EN;
    }
  #endif
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
        GenerateCIU4_OSD2Bits(CiuOverlayImg4_Top, CiuOverlayImg4_Bot,
                              ciuOSDStr4,  CIU_OSD_CHARMAX,
                              ASCII_XLARGE_FONT_WIDTH, ASCII_XLARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);
    else    // VGA
        GenerateCIU4_OSD2Bits(CiuOverlayImg4_Top, CiuOverlayImg4_Bot,
                              ciuOSDStr4,  CIU_OSD_CHARMAX,
                              ASCII_LARGE_FONT_WIDTH, ASCII_LARGE_FONT_HEIGHT,
                              8,3,8+4,3+8);

    if(OSD_en)
    {
        CIU_4_OVL_IDXCOLOR_Y   = 0x00ffffff;
        CIU_4_OVL_IDXCOLOR_CB  = 0x00808080;
        CIU_4_OVL_IDXCOLOR_CR  = 0x00808080;

        CIU_4_CTL2 = CIU_NORMAL |
                    CIU_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_DATA_OUT_ENA |
                    CIU_YUVDATALATCH_NEG |
                    CIU_TESTIMG_DISA |
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if HW_DEINTERLACE_CIU4_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_ENA;
    }
    else
    {
        CIU_4_CTL2 = CIU_NORMAL |
                    CIU_ENA |
                    CIU_VSYNC_ACT_LO |
                    CIU_HSYNC_ACT_LO |
                    CIU_INT_DISA_FIFO_OVERF |
                    CIU_INT_ENA_FRAME_END |
                    CIU_DATA_OUT_ENA |
                    CIU_YUVDATALATCH_NEG |
                    CIU_TESTIMG_DISA |
                    CIU_INT_DISA_FIELD_END |
                    CIU_EXT_FIELD_EN |
                #if HW_DEINTERLACE_CIU4_ENA
                    CIU_INT_ENA_TOPEND |
                #endif
                    CIU_OSD_DISA;
    }
    #if(TV_DECODER == TW2866)
        CIU_4_CTL2 |= CIU_SPLITER_ENA; //enable CCIR4ch
    #endif
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
#endif
}

void ciuPreviewInMenu_CH1(u32 InWidth, u32 InHeight, u32 OutWidth, u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride)
{
    ciuPreviewInit_CH1(SIUMODE_PREVIEW_MENU, InWidth, InHeight, OutWidth, OutHeight, OutX, OutY, CIU1_OSD_EN, line_stride);
}

//Lucian: RF TX 必須使用CIU1,放大倍率固定為x2.由於HW限制,HD輸出無法做Zoom-in
s32 rfiuciu1ZoomIn(int OnOff,int Xpos,int Ypos)
{
   u32 W,H;
   u32 temp;
   //---------//
// A1020DIFF1016
#if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
	if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
		CIU_1_CTL1 |=CIU_SCA_SHAREBUF_EN;
	else
	    CIU_1_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
#endif

#if( (Sensor_OPTION==Sensor_CCIR656) || (Sensor_OPTION==Sensor_CCIR601) )  //For TV-in
   if(OnOff)
   {


      if(Xpos+320>640)
          Xpos=320;
      if(Ypos+120>240)
          Ypos=120;

      CIU_1_InputSize     = ( (640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
      CIU_1_IMG_STR       = ((((720-640)/2+Xpos)*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);

   }
   else
   {
      CIU_1_InputSize     = ( (640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((240)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x1
      CIU_1_IMG_STR       = (((720-640)/2*2)<<CIU_IMG_H_STR_SHFT) | (0<<CIU_IMG_V_STR_SHFT);

   }

#else //For Sensor-in
   if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
   {
       //Cannot use zoom function //
   }
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480)
   {
       if(OnOff)
       {
           if(OnOff==2) //x2: 600x360
           {
               if(Xpos+600>1280)
                  Xpos=1280-600;
               if(Ypos+360>720)
                  Ypos=720-360;
               #if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
               CIU_1_SA_InputSize = ((600)<<CIU_INPUT_SIZE_X_SHFT) | ((360)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
               CIU_1_IMG_SCALAR_START       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
               #else
               CIU_1_InputSize     = ( (600*2)<<CIU_INPUT_SIZE_X_SHFT) | ((360)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
               CIU_1_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
               #endif
           }
           else //x 1.5: 800x480
           {
               if(Xpos+800>1280)
                  Xpos=1280-800;
               if(Ypos+480>720)
                  Ypos=720-480;
               #if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
               CIU_1_SA_InputSize = ((800)<<CIU_INPUT_SIZE_X_SHFT) | ((480)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x1.5
               CIU_1_IMG_SCALAR_START = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
               #else
               CIU_1_InputSize     = ( (800*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
               CIU_1_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
               #endif
           }
       }
       else
       {
           #if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
           CIU_1_SA_InputSize = ((1280)<<CIU_INPUT_SIZE_X_SHFT) | ((720)<<CIU_INPUT_SIZE_Y_SHFT);
           CIU_1_IMG_SCALAR_START=0;
           #else
           CIU_1_InputSize     = ( (1280*2)<<CIU_INPUT_SIZE_X_SHFT) | ((720)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_1_IMG_STR=0;
           #endif
       }
   
   }
#if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)   
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240)
   {
       #if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
       CIU_1_SA_InputSize = ((1280)<<CIU_INPUT_SIZE_X_SHFT) | ((720)<<CIU_INPUT_SIZE_Y_SHFT);
       CIU_1_IMG_SCALAR_START=0;
       #else
       CIU_1_InputSize     = ( (1280*2)<<CIU_INPUT_SIZE_X_SHFT) | ((720)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
       CIU_1_IMG_STR=0;
       #endif
   }
#endif   
   else
   {
       if(OnOff)
       {
           if(Xpos+320>640)
              Xpos=320;
           if(Ypos+240>480)
              Ypos=240;
           #if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
           CIU_1_SA_InputSize = ((640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480/2)<<CIU_INPUT_SIZE_Y_SHFT);
           CIU_1_IMG_SCALAR_START=0;
           #else
           CIU_1_InputSize     = ( (640/2*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480/2)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_1_IMG_STR       = ((Xpos*2)<<CIU_IMG_H_STR_SHFT) | (Ypos<<CIU_IMG_V_STR_SHFT);
           #endif
       }
       else
       {
           #if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
           CIU_1_SA_InputSize = ((640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480)<<CIU_INPUT_SIZE_Y_SHFT);
           CIU_1_IMG_SCALAR_START=0;
           #else
           CIU_1_InputSize     = ( (640*2)<<CIU_INPUT_SIZE_X_SHFT) | ((480)<<CIU_INPUT_SIZE_Y_SHFT); //Zoom x2
           CIU_1_IMG_STR=0;
           #endif
       }
   }
#endif


}



s32 ciu1ScUpZoom(s32 zoomFactor)
{
   u32 W,H;

   getPreviewZoomSize(zoomFactor,&W,&H);
   // A1020DIFF1016
#if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
   //CIU_1_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
#endif

   CIU_1_InputSize     = ( (W*2)<<CIU_INPUT_SIZE_X_SHFT) | (H<<CIU_INPUT_SIZE_Y_SHFT);

   if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
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
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1088)
   {
       if( (1920 >= W) && (1088 >= H) )
           CIU_1_IMG_STR       = (((1920-W)/2)<<CIU_IMG_H_STR_SHFT) | (((1088-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_1_IMG_STR = 0;
   }
   else
   {
       CIU_1_IMG_STR = 0;
   }

}

s32 ciu2ScUpZoom(s32 zoomFactor)
{
   u32 W,H;

   getPreviewZoomSize(zoomFactor,&W,&H);
   // A1020DIFF1016
#if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
   CIU_2_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
#endif

   CIU_2_InputSize     = ( (W*2)<<CIU_INPUT_SIZE_X_SHFT) | (H<<CIU_INPUT_SIZE_Y_SHFT);


   if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
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
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1088)
   {
       if( (1920 >= W) && (1088 >= H) )
           CIU_2_IMG_STR       = (((1920-W)/2)<<CIU_IMG_H_STR_SHFT) | (((1088-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_2_IMG_STR = 0;
   }
   else
   {
       CIU_2_IMG_STR = 0;
   }
}

s32 ciu3ScUpZoom(s32 zoomFactor)
{
   u32 W,H;

   getPreviewZoomSize(zoomFactor,&W,&H);
   // A1020DIFF1016
#if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
   CIU_3_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
#endif

   CIU_3_InputSize     = ( (W*2)<<CIU_INPUT_SIZE_X_SHFT) | (H<<CIU_INPUT_SIZE_Y_SHFT);


   if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
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
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1088)
   {
       if( (1920 >= W) && (1088 >= H) )
           CIU_3_IMG_STR       = (((1920-W)/2)<<CIU_IMG_H_STR_SHFT) | (((1088-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_3_IMG_STR = 0;
   }
   else
   {
       CIU_3_IMG_STR = 0;
   }
}

s32 ciu4ScUpZoom(s32 zoomFactor)
{
   u32 W,H;

   getPreviewZoomSize(zoomFactor,&W,&H);
   // A1020DIFF1016
#if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
   CIU_4_CTL1 &= (~CIU_SCA_SHAREBUF_EN);
#endif

   CIU_4_InputSize     = ( (W*2)<<CIU_INPUT_SIZE_X_SHFT) | (H<<CIU_INPUT_SIZE_Y_SHFT);

   if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
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
   else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1088)
   {
       if( (1920 >= W) && (1088 >= H) )
           CIU_4_IMG_STR       = (((1920-W)/2)<<CIU_IMG_H_STR_SHFT) | (((1088-H)/2)<<CIU_IMG_V_STR_SHFT);
       else
           CIU_4_IMG_STR = 0;
   }
   else
   {
       CIU_4_IMG_STR = 0;
   }
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

#if( (CIU1_OPTION==Sensor_CCIR656) || (CIU1_OPTION==Sensor_CCIR601) )
	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSD1 width is not multiples of 32 \n");
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

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;
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

                 #if 1
                     if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                        source  = (u8*)ASCII_XFont[index];
                     else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_320x240)
                        source  = (u8*)ASCII_SFont[index];
                     else   // VGA
                        source  = (u8*)ASCII_Font[index];
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
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

	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSD2 width is not multiples of 32 \n");
	}

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

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;

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

                 #if 1
                     if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                        source  = (u8*)ASCII_XFont[index];
                     else   // VGA
                        source  = (u8*)ASCII_Font[index];
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
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

	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSD3 width is not multiples of 32 \n");
	}
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

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;

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

                 #if 1
                     if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                        source  = (u8*)ASCII_XFont[index];
                     else   // VGA
                        source  = (u8*)ASCII_Font[index];
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
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

	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSD4 width is not multiples of 32 \n");
	}
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

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;

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

                 #if 1
                     if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
                        source  = (u8*)ASCII_XFont[index];
                     else   // VGA
                        source  = (u8*)ASCII_Font[index];
                 #else //Lucian: 無法做 QVGA
                     if(!ASCII_flag ) //Lsk : QVGA frames
                        source  = (u8*)QVGA_ASCII_Font[j];
                     else
                        source  = (u8*)ASCII_Font[j];
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
    strcpy(ciuszString4, szString);
#endif
    return  1;
}

s32 GenerateCIU1_SP_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,
                                  char *szString, int MaxStrLen,
                                  int FontW, int FontH,
                                  int Font_STR_X,int Font_STR_Y,
                                  int Font_END_X,int Font_END_Y)
{
#if( (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A) )
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

	if( (FontW/4*char_W) & 0x1f )
    {
       DEBUG_CIU("Error! CIU OSDsp width is not multiples of 32 \n");
	}
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

    FontW_Byte = FontW *2 / 8;
    osd1    = (u8*)OSDImg_top;
    osd2    = (u8*)OSDImg_bot;
    osd_stride=FontW_Byte*char_W;
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
             if(szString[k] != ciuszString1[k])
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

#endif

s32 ciuCaptureVideo_CH1(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride)
{
    //ciu_1_FrameTime = 0;
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
    if(sysPIPMain == PIP_MAIN_CH1)
    {
        ciuPreviewInit_CH1(SIUMODE_PREVIEW, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU1_OSD_EN, line_stride);
        ciuPreviewInit_CH3(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth / 2, OutHeight / 2, OutWidth / 2, OutHeight / 2, CIU2_OSD_EN, line_stride);
    }
    else
        ciuPreviewInit_CH3(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU2_OSD_EN, line_stride);
    return 1;
}

s32 ciuCaptureVideo_CH4(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride)
{
    ciu_4_FrameTime = 0;
    if(sysPIPMain == PIP_MAIN_CH1)
    {
        ciuPreviewInit_CH1(SIUMODE_PREVIEW, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU1_OSD_EN, line_stride);
        ciuPreviewInit_CH4(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth / 2, OutHeight / 2, OutWidth / 2, OutHeight / 2, CIU2_OSD_EN, line_stride);
    }
    else
        ciuPreviewInit_CH4(SIUMODE_MPEGAVI, InWidth, InHeight, OutWidth, OutHeight, 0, 0, CIU2_OSD_EN, line_stride);
    return 1;
}

  #if (HW_DEINTERLACE_CIU1_ENA || HW_DEINTERLACE_CIU2_ENA || HW_DEINTERLACE_CIU3_ENA || HW_DEINTERLACE_CIU4_ENA)
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
            
           DEBUG_CIU("Avg=%d,%d,\n",Avg_Diff,MotionCnt);
      }

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
    u32 fps;
    
    if(last_count == 0)
    {
        last_count=ciu_1_FPS_Count;
        return 0;
    }
    fps=ciu_1_FPS_Count - last_count;


    if(fps <3)
    {
        count_error++;
        if(count_error >5)
        {
            DEBUG_TIMER("Frame error %d , system reboot\n", fps);
			sysForceWDTtoReboot();
        }
        
    }
    else
    {
        count_error=0;
    }
    
    last_count =ciu_1_FPS_Count;    
    return fps;
    
}
#else
void ciuIntHandler_CH1(void){}
void ciuIntHandler_CH2(void){}
void ciuIntHandler_CH3(void){}
void ciuIntHandler_CH4(void){}
void ciu_1_Stop(void){}
void ciu_2_Stop(void){}
void ciu_3_Stop(void){}
void ciu_4_Stop(void){}
s32 ciu1ScUpZoom(s32 zoomFactor){return 1;}
s32 ciu2ScUpZoom(s32 zoomFactor){return 1;}
s32 ciu3ScUpZoom(s32 zoomFactor){return 1;}
s32 ciu4ScUpZoom(s32 zoomFactor){return 1;}
s32 ciuPreviewInit_CH1(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuPreviewInit_CH2(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuPreviewInit_CH3(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuPreviewInit_CH4(u8 mode,u32 InWidth, u32 InHeight,u32 OutWidth,u32 OutHeight, u32 OutX, u32 OutY, u8 OSD_en,u32 line_stride){return 1;}
s32 CiuInit(void){return 1;}
s32 ciuCaptureVideo_CH1(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuCaptureVideo_CH2(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuCaptureVideo_CH3(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride){return 1;}
s32 ciuCaptureVideo_CH4(u32 InWidth, u32 InHeight,u32 OutWidth, u32 OutHeight, u8 OSD_en,u32 line_stride){return 1;}
s32 GenerateCIU1_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,char *szString, int MaxStrLen,int FontW, int FontH,int Font_STR_X,int Font_STR_Y,int Font_END_X,int Font_END_Y){return 1;}
s32 GenerateCIU2_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,char *szString, int MaxStrLen,int FontW, int FontH,int Font_STR_X,int Font_STR_Y,int Font_END_X,int Font_END_Y){return 1;}
s32 GenerateCIU3_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,char *szString, int MaxStrLen,int FontW, int FontH,int Font_STR_X,int Font_STR_Y,int Font_END_X,int Font_END_Y){return 1;}
s32 GenerateCIU4_OSD2Bits(u32 *OSDImg_top, u32 *OSDImg_bot,char *szString, int MaxStrLen,int FontW, int FontH,int Font_STR_X,int Font_STR_Y,int Font_END_X,int Font_END_Y){return 1;}
#endif


#endif


