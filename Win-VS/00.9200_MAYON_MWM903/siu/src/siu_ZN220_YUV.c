
/*
Copyright (c) 2014 Mars Semiconductor Corp.

Module Name:

    siu_XC7021_GC2023

Abstract:

    The routines of Sensor Interface Unit.
    Control XC7021 ISP + GC2023 sensor
            
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2107/10/05  Amy Pan Create  
*/


#include "general.h"
#include "board.h"
#if (Sensor_OPTION == Sensor_ZN220_YUV601)
#include "task.h"
#include "siuapi.h"
#include "siu.h"
#include "siureg.h"
#include "../ciu/inc/ciureg.h"
#include "sensorapi.h"
#include "i2capi.h"
#include "siuapi.h"
#include "ipuapi.h"
#include "isuapi.h"
#include "fsapi.h"
#include "rtcapi.h"
#include "dcfapi.h"
#include "jpegapi.h"
#include "gpioapi.h"  //lisa 070514
#include "sysapi.h"
#include "uiapi.h"
#include "iisapi.h"
#include "timerapi.h"
#include "rfiuapi.h"

/*
 *********************************************************************************************************
 *  SYS Constant
 *********************************************************************************************************
 */

#define SIU_TIMEOUT			20	
 /*
 *********************************************************************************************************
 * SIU Constant
 *********************************************************************************************************
 */
 //----- Function Option -----//
#define REC_D1_MODE         0  // 1: D1, 0: VGA
#define ANTI_FICKER_60HZ    1  // 1: 60, 0: 50
#define MAX_AE_EXPTIME_1_30 1  // 1: 30 fps, 0: 15 fps (Max)

#if (SDV_SELECT ==  SSDV_1)//DG033
#define SENSOR_ROT_0_DEG    0  // 1: 0 degree, 0: 180 degree
#else //DG608
#define SENSOR_ROT_0_DEG    1  // 1: 0 degree, 0: 180 degree
#endif



/*--------- For new AE Algorithm---------*/



#define AE_AGC_SYNC_FRAMENUM              1

#define AE_SYNC_AGC_SW_POLEN              0    //AGC/SW set use polling or semephore

#define AECNT_PREVIEW                     5    //30/(5+1)=5 次/sec (motion detection)
#define AECNT_VideoCap                   24


#define SIU_CAPOEFINT_DISA	    0xffffd0ff //Register mask for disable siu interrrupt


// For new AE Table


#define Gain_Level 6
#define AE_E_Table_Num	    16
#define AE_Table_Row_Num	15


#define SW_VIDEOCLIP2PREVIEW_RATION    1000
#define SW_CAPTURE2PREVIEW_RATION      1000   //x1000, 

 /*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
   720   702   684   666   648   630   612   594   576   558   540   522   504   486   468   450   432   414   396
   378   360   342   324   306   288   270   252   234   216   198   180   162   144   126   108    90
 
 */
//-------- Digital Zoom ---------//
/*Hardware limit: In preview/VideoClip mode, Y,X direction cannot do up-scaling.*/
    ZOOM_FACTOR PreviewZoomParm_HD[MAX_PREVIEW_ZOOM_FACTOR] = 
    {
    	    {{1280,  720},  100}, 
            {{1248,  702},  101},
            {{1216,  684},  102}, 
            {{1184,  666},  103}, 
            {{1152,  648},  105},
            {{1120,  630},  106},
            {{1088,  612},  107}, 
            {{1056,  594},  118},    
            {{1024,  576},  110}, 
            {{992,   558},  111},   
            {{960,   540},  112},  
            {{928,   522},  113},  
            {{896,   504},  114},
            {{864,   486},  115},
            {{832,   468},  117}, 
            {{800,   450},  118}, 
            {{768,   432},  119}, 
            {{736,   414},  120}, 
            {{704,   396},  122}, 
            {{672,   378},  123}, 
            {{640,   360},  124}, 
            {{608,   342},  125}, 
            {{576,   324},  126}, 
            {{544,   306},  127}, 
            {{512,   288},  129},      
            {{480,   270},  130}, 
            {{448,   252},  132},
            {{416,   234},  133}, 
            {{384,   216},  134},
            {{352,   198},  135}, 
            {{320,   180},  136},
            {{288,   162},  137}, 
            {{256,   144},  139},
            {{224,   126},  140}, 
            {{192,   108},  141},
            {{160,   90},  142}, 
            {{152,   84},  143},
            {{144,   78},  144}, 
            {{136,   72},  146},
            {{128,   66},  147}, 
            {{120,   60},  148},
            {{112,   54},  150}, 
            {{104,   48},  151},
            {{96,    42},  152}, 
            {{88,    36},  153}
     };
     ZOOM_FACTOR PreviewZoomParm_VGA[MAX_PREVIEW_ZOOM_FACTOR] = 
     {
            {{640,  480},  100},  
            {{624,  468},  101}, 
            {{608,  456},  102},  
            {{592,  444},  103},  
            {{576,  432},  105}, 
            {{560,  420},  106}, 
            {{544,  408},  107},  
            {{528,  396},  118},     
            {{512,  384},  110},  
            {{496,  372},  111},    
            {{480,  360},  112},   
            {{464,  348},  113},   
            {{448,  336},  114}, 
            {{432,  324},  115}, 
            {{416,  312},  117},  
            {{400,  300},  118},  
            {{384,  288},  119},  
            {{368,  276},  120},  
            {{352,  264},  122},  
            {{336,  252},  123},  
            {{320,  240},  124},  
            {{304,  228},  125},  
            {{288,  216},  126},  
            {{272,  204},  127},  
            {{256,  192},  129},     
            {{240,  180},  130},  
            {{224,  168},  132}, 
            {{208,  156},  133},  
            {{192,  144},  134}, 
            {{176,  132},  135},  
            {{160,  120},  136}, 
            {{144,  108},  137},  
            {{128,   96},  139}, 
            {{112,   84},  140},  
            {{96,    72},  141}, 
            {{80,    60},  142},  
            {{64,    48},  143},
            {{64,    48},  144}, 
            {{64,    48},  146},
            {{64,    48},  147}, 
            {{64,    48},  148},
            {{64,    48},  150}, 
            {{64,    48},  151},
            {{64,    48},  152}, 
            {{64,    48},  153}
     };
//---- White Balance Compensation ----//
/*
   [0]: R gain compensation ratio
   [1]: B gain compensation ration
   [2]: Version 
   [4]: Confirm key=0x12345678
*/
s32 siuWBComp_RBRatio[4];

//-----Defect Compensation -----//


//-------Set SIU pre-gamma -------//
#define SIU_B_GAMMA_TBL_COUNT		17

const u16 siuPreGammaRefTab_X[SIU_B_GAMMA_TBL_COUNT]= { 0, 128, 256, 288, 320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480, 496, 512 };
const u8 siuPreGammaRefTab_B[SIU_B_GAMMA_TBL_COUNT] = { 0,  64, 128, 144, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255 };
const u8 siuPreGammaRefTab_R[SIU_B_GAMMA_TBL_COUNT] = { 0,  64, 128, 144, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255 };
const u8 siuPreGammaRefTab_Gr[SIU_B_GAMMA_TBL_COUNT]= { 0,  64, 128, 144, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255 };
const u8 siuPreGammaRefTab_Gb[SIU_B_GAMMA_TBL_COUNT]= { 0,  64, 128, 144, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255 };

const u16 siuPreGammaRefTab_X_Night[SIU_B_GAMMA_TBL_COUNT]= { 0, 8, 16, 24, 32, 48, 64, 80, 96, 128, 160, 192, 256, 320, 384, 448, 512};
const u16 siuPreGammaRefTab_Y_Night[SIU_B_GAMMA_TBL_COUNT]= { 0,14, 23, 30, 37, 49, 60, 70, 79,  97, 113, 129, 157, 184, 209, 233, 255};


u16 siuPreGammaTab_X[SIU_B_GAMMA_TBL_COUNT]= { 0, 128, 256, 288, 320, 336, 352, 368, 384, 400, 416, 432, 448, 464, 480, 496, 512 };
u8 siuPreGammaTab_B[SIU_B_GAMMA_TBL_COUNT] = { 0,  64, 128, 144, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255 };
u8 siuPreGammaTab_R[SIU_B_GAMMA_TBL_COUNT] = { 0,  64, 128, 144, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255 };
u8 siuPreGammaTab_Gr[SIU_B_GAMMA_TBL_COUNT]= { 0,  64, 128, 144, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255 };
u8 siuPreGammaTab_Gb[SIU_B_GAMMA_TBL_COUNT]= { 0,  64, 128, 144, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 255 };
//---Sys control----//
OS_STK siuTaskStack[SIU_TASK_STACK_SIZE]; /* Stack of task siuTask() */
OS_EVENT* siuSemEvt, *siuCapEvt;          /* semaphore to synchronize event processing */

u8 siuOpMode    = SIUMODE_START;
u8 siuCaptureFlag = 0;

u8 siuAeReadyToCapImage = 0;  


//-------Sensor Control ----------//
SENSOR_VALIDSIZE sensor_validsize;
u8 siuVideoZoomSect=0xf;

//-------3A  Control -------//
u8 siuFrameCount = 0;
u8 siuAFCount = 0;
u8 siuAfEnable = 0;
u8 siuAeEnable = 1;
u8 siuAwbEnable = 1;

//-------Lens Shading control------//
SIU_LENS_SHADING lensShading = 
{ 
	{SENSOR_VALID_SIZE_X / 2, SENSOR_VALID_SIZE_Y / 2 },	/* centerOffset */	
	(SENSOR_VALID_SIZE_X / 2) * (SENSOR_VALID_SIZE_X / 2),	/* cornerX2 */	
	(SENSOR_VALID_SIZE_Y / 2) * (SENSOR_VALID_SIZE_Y / 2),	/* cornerY2 */
	 256,							                    /* shading_G_Gain */
	 256,							                    /* shading_R_Gain */
	 256,							                    /* shading_B_Gain */
	(u32)SIU_LC_RTYPE_SK7,					                /* rType */
	(u32)SIU_LC_SCAL_1_1,					                /* scaleSize */
	(u32)SIU_LC_OUT_INVERT_DISA 				            /* outputInvert */
};

//-------Flash Light Control ------//
u32 ae_YavgValue = 0;
u16 siuInterTimeCount = 0x12C;
u8 siuFlashGoFrameCount = 0;
u8 siuFlashGetYFrameCount = 0;
u8 siuPreFlash1ToPreFlash2FrameOffset = 1;
u8 siuPreFlash2ToMainFlashFrameOffset = 1;
u16 siuFlashLightPreChargeTimeCount = 0;
u8 siuPreFlashCount = 0;
u8 siuFlashLightReadyToGo = 0;
u8 siuFlashLightReadyToGetYavg = 0;


u8 siuFlashLightMode = SIU_FLASH_LIGHT_ALWAYS_OFF;
u8 siuFlashLightTestFlag = 0;
u8 siuFlashLightShoot = 0;      //決定是否打閃光燈.
u8 siuShootMainFlash=0;         //決定是否打出主閃: 用於User強迫閃光, 但環境又不需打閃光燈,旨在防紅眼.

#if(VIDEO_RESOLUTION_SEL == VIDEO_HD_IN_HD_OUT)
u32 siuVideoWidth   = 1280;
u32 siuVideoHeight  = 720;
#else
u32 siuVideoWidth   = 640;
u32 siuVideoHeight  = 480;
#endif

u8  IsSensorInit;

u8  DayNightMode    = 0xff;

//------------------------ AE control -------------------------//
//------------------------ AE control -------------------------//
/*
            -2.0EV    -1.5EV    -1.0EV    -0.5EV    0EV     0.5EV     1.0EV     1.5EV    2.0EV
            -12dB     -9dB      -6dB      -3dB      0dB       3dB      6dB       9dB     12dB
          ----------------------------------------------------------------------------------------------
Software      30      43         60        85       120      170       239      240       255
 
Hardware       8       13         21        34       55       90       148       240     245
             (22)     (31)       (44)      (62)     (87)    (123)     (174)     (255)   (255)
*/

DEF_AE_Tab AE_EG_Tab_VideoPrev_5060Hz[2][2][AE_E_Table_Num*Gain_Level];  //AE Index Table

u8 AE_Flicker_50_60_sel = SENSOR_AE_FLICKER_60HZ;
u8 siuY_TargetIndex     = 5;
u8 siuContrastIndex     = 5;
u8 siuSaturationIndex   = 5;
u8 siuSharpnessIndex    = 5;


#define TAR_MID 0x60
#define CON_MID 0x40
#define SAT_MID 0x20
#define SHA_MID 0x10
#define TAR_MID_N 0x80 //no use

#if (HW_BOARD_OPTION  == MR9100_TX_RDI_CA811)
const u8 AETargetMeanTab[11]    =   {TAR_MID-0x20, TAR_MID-0x18, TAR_MID-0x10, TAR_MID-0x8, TAR_MID, TAR_MID+0x8, TAR_MID+0x10, TAR_MID+0x18, TAR_MID+0x20, TAR_MID+0x28, TAR_MID+0x30};
#else
const u8 AETargetMeanTab[11]    =   {TAR_MID-0x20, TAR_MID-0x18, TAR_MID-0x10, TAR_MID-0x8, TAR_MID, TAR_MID+0x8, TAR_MID+0x10, TAR_MID+0x18, TAR_MID+0x20, TAR_MID+0x28, TAR_MID+0x30};
#endif
const s8 AETargetMeanTab_N[11]    =   {TAR_MID_N-0x40, TAR_MID_N-0x30, TAR_MID_N-0x20, TAR_MID_N-0x10, TAR_MID_N, TAR_MID_N+0x10, TAR_MID_N+0x20, TAR_MID_N+0x30, TAR_MID_N+0x40, TAR_MID_N+0x50, TAR_MID_N+0x60};

const s8 AEContrastTab[11]      =   {CON_MID-0x40, CON_MID-0x38, CON_MID-0x30, CON_MID-0x20, CON_MID-0x10, CON_MID, CON_MID+0x10, CON_MID+20, CON_MID+0x30, CON_MID+0x38, CON_MID+0x40};

const s8 AESaturationTab[11]    =   {SAT_MID-20, SAT_MID-16, SAT_MID-12, SAT_MID-8, SAT_MID-4, SAT_MID, SAT_MID+4, SAT_MID+8, SAT_MID+12, SAT_MID+16, SAT_MID+20};

const s8 AESharpnessTab[11]  =   {0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A};

SIU_AEINFO siu_aeinfo;
AE_WIN aeWin;
u16 aeWinYsum[25];          //AE 之25個window之個別Y sum. (256 unit).

u8 siuAeSyncAgcSwControl = 0;
u8  AEConvergeCount = 0;       //AE 收斂次數


u16 AECurSet = 8*Gain_Level;
u16 AEPreSet = 8*Gain_Level;


u8 uiDigZoomSetFlag = 0; 
//-----------AWB control-------------//
u8 Awb_flag = 0;
u8 siuAWBModeSel = 0;


//---------For AV sync: Audio clock -------//
/* Peter 070104 */
u32 siuFrameTime;
/* mh@2006/11/22: for time stamp control */
u32 unMp4SkipFrameCount;
u32 unMp4FrameDuration[SIU_FRAME_DURATION_NUM];  /* Peter 070403 */
/* mh@2006/11/22: END */
u8 avi_FrameSkip; /*BJ 0530 S*/	
u32 siuFrameNumInMP4, siuSkipFrameRate; /* Peter 20061106 */


#define ElemsOfArray(x) (sizeof(x) / sizeof(x[0]))
#if (HW_BOARD_OPTION  == MR9100_TX_RDI_CA811)
const u8 ZN220_SETTING_1080P_10FPS_1[] = 
{
    0x00,0x00,
    0x0C,0x02,
    0x0D,0x00,
    0xD8,0x00,
    0xD9,0x99,
    0xDA,0xE8,
    0xDB,0x0B,
    0xDC,0xCF,
    0x0E,0x10,
    0x18,0x00,  //Mirror
    0x1C,0x78,
    0x1E,0x03,
    0x1F,0x04,
    0x38,0x00,
    0x5A,0x5F,
    0x5E,0xCE,
    0x6B,0x07,
    0x6C,0x9E,
    0x6D,0x07,
    0x6E,0xD0,
    0x78,0x19,
    0x7A,0x1C,
    0x7B,0x07,
    0x7C,0xD0,
    0x7E,0x02,
    0x92,0x02,
    0x9B,0xC0,
    0xD0,0x88,
    0xD2,0x0E,
    0xD3,0x44,
    0xD5,0x0C,
    0xD6,0x4C,
    0xD7,0xBC,
    0x21,0x2F,
    0x22,0xD0,
    0xB5,0x01,
    0x00,0x01,
    0x0F,0x80,
    0x03,0x80,
    0x00,0x03,
    0xFC,0x6B,
    0xFD,0x6B,
    
    //isp setting
    0x00,0x01,
    0x01,0xDB,
    0x02,0x53,
    0x05,0xF8,
    0x06,0x81,
    0x07,0xF4,
    0x09,0x8B,
    0x09,0xCB,
    0x0A,0xD6,
    0x0B,0x02,
    0x0C,0xC8,
    0x13,0x10,
    
    0x9E,0x9C,  //POSTCH_RGAIN
    0xA0,0xD1,  //POSTCH_BGAIN
    
    0x00,0x03,
    0x3E,0x10,  //EPC1
    0x3F,0x10,  //EPC2
    
    0x00,0x04,
    0x65,0x08,  //EPC          
    
    0x00,0x02,
    0x02,0x11,  //GM1
    0x03,0x1E,  //GM2
    0x04,0x26,  //GM3
    0x05,0x2E,  //GM4
    0x06,0x3C,  //GM5
    0x07,0x47,  //GM6
    0x08,0x5B,  //GM7
    0x09,0x6C,  //GM8
    0x0A,0x8A,  //GM9
    0x0B,0xA5,  //GM10
    0x0C,0xBD,  //GM11
    0x0D,0xD4,  //GM12
    0x0E,0xEB,  //GM13
    0x10,0x10,  //GM_ATT_TGAIN_TH1_H
    0x11,0x00,  //GM_ATT_TGAIN_TH1_L
    0x12,0x18,  //GM_ATT_TGAIN_TH2_H
    0x13,0x00,  //GM_ATT_TGAIN_TH2_L
    0x15,0x30,  //GM_ATT0xT2
    
    0x00,0x03,
    0x1C,0x40,
    0x2C,0x18,
    0x2D,0xFE,
    0x2E,0xFE,
    0x4A,0x00,  //Auto_DtoN_mode
    0x37,0x00,  //SAT_TH1
    0x38,0x01,  //SAT_TH2
    0x39,0x08,  //SAT_TH3
    0x3A,0x14,  //SAT_TH4
    0x40,0x10,  //AE_HOLD
    0x42,0x04,  //AE_RELEASE
    0x43,0x00,  //AE_STAY
    0x4D,0x64,  //EGP_REF0_L(firstExp)
    0x4F,0x64,  //EGP_REF1_L(firstAgain)
    0x51,0x64,  //EGP_REF2_L(Exp_Limit)
    0x53,0x46,  //EGP_REF3_H(Again_Limit)
    0x54,0x40,  //EGP_REF3_M(Again_Limit)
    0x55,0x00,  //MAX_EGP_T(Dgain_Limit)
    0x56,0xAF,  //MAX_EGP_H(Dgain_Limit)
    0x57,0xA0,  //MAX_EGP_L(Dgain_Limit)
    0x00,0x07,
    0xDE,0x03,
    0xDF,0x01,
    0xE3,0x01,
    0xE4,0x80,
    0xE7,0x06,
    0xE8,0xA4,
    0xEB,0x10,  //
    0xEC,0x00,  //
    0xED,0x00,  //
    0xEE,0x46,  //
    0xEF,0x40,  //
    0xF0,0xA0,  //
    0xF1,0x10,  //
    0xF2,0x00,  //
    0xF3,0x80,
    0xF4,0x01,


};

const u8 ZN220_SETTING_1080P_10FPS_2[] = 
{
    0x00,0x04,
    0x41,0xE6,
    0x42,0x15,
    0x43,0x18,
    0x44,0x00,
    0x01,0x00,  //A0xB_CEN_0xINX_BEGIN 
    0x02,0x00,  //A0xB_CEN_0xINX_BEGIN 
    0x03,0x07,  //A0xB_CEN_0xINX_END   
    0x04,0x7F,  //A0xB_CEN_0xINX_END   
    0x05,0x00,  //A0xB_CEN_0xINY_BEGIN 
    0x06,0x00,  //A0xB_CEN_0xINY_BEGIN 
    0x07,0x04,  //A0xB_CEN_0xINY_END   
    0x08,0x37,  //A0xB_CEN_0xINY_END   
    0x19,0x10,  //YLO0x_TH1
    0x20,0x78,  //YSAM_HIGH
    0x21,0x47,  //PX1
    0x22,0x35,  //PY1
    0x23,0x2D,  //PX2
    0x24,0x6A,  //PY2
    0x25,0x6F,  //PX3
    0x26,0x2E,  //PY3
    0x27,0x47,  //PX4
    0x28,0x35,  //PY4
    0x35,0x01,  //
    0x36,0xF6,  //
    0x37,0x00,  //
    0x38,0xB3,  //
    0x45,0x10,  //A0xB_C0xIN_0xT_TH1
    0x46,0x10,  //A0xB_C0xIN_0xT_TH2   
    0x5F,0x02,  //RGAIN_RELEASE 
    0x62,0x02,  //BGAIN_RELEASE
    0x78,0x80,
    0x7F,0x80,
    0x80,0x5A,  //INDOOR_MAX_RGAIN     
    0x82,0x68,  //INDOOR_MAX_BGAIN
    0x84,0x60,  //OUTDOOR_MAX_RGAIN 
    0x86,0x68,  //OUTDOOR_MAX_BGAIN
    0x88,0x65,  //AUTO_MAX_RGAIN     
    0x89,0x27,  //AUTO_MIN_RGAIN     
    0x8A,0x58,  //AUTO_MAX_BGAIN     
    0x8B,0x26,  //AUTO_MIN_BGAIN  
    0x94,0x58,
    0x95,0x27,
    0x96,0x59,
    0x97,0x26,
    0x00,0x05,
    0x00,0x06,
    0xDC,0x10,  //OSD_AGAIN_LMT_H
    0xDD,0x00,  //OSD_AGAIN_LMT_L
    0xD4,0x03,
    0xD6,0x10,  //OSD_AGC1
    0xD7,0x18,  //OSD_AGC2
    0xD8,0x20,  //OSD_AGC3
    0xD9,0x30,  //OSD_AGC4
    0xDA,0x40,  //OSD_AGC5
    0x00,0x07,
    0x01,0x1F,  //CC_HEXA_MGR_GAIN
    0x02,0xFF,  //CC_HEXA_MGR_GAIN
    0x03,0x01,  //CC_HEXA_MGR_GAIN
    0x04,0x20,  //CC_HEXA_MGR_GAIN
    0x05,0x20,  //CC_HEXA_RYE_GAIN
    0x06,0xFF,  //CC_HEXA_RYE_GAIN
    0x07,0x00,  //CC_HEXA_RYE_GAIN
    0x08,0x20,  //CC_HEXA_RYE_GAIN
    0x09,0x20,  //CC_HEXA_YEG_GAIN
    0x0A,0x00,  //CC_HEXA_YEG_GAIN
    0x0B,0x00,  //CC_HEXA_YEG_GAIN
    0x0C,0x20,  //CC_HEXA_YEG_GAIN
    0x0D,0x20,  //CC_HEXA_GCY_GAIN
    0x0E,0x00,  //CC_HEXA_GCY_GAIN
    0x0F,0x00,  //CC_HEXA_GCY_GAIN
    0x10,0x20,  //CC_HEXA_GCY_GAIN
    0x11,0x20,  //CC_HEXA_CYB_GAIN
    0x12,0xFF,  //CC_HEXA_CYB_GAIN
    0x13,0x01,  //CC_HEXA_CYB_GAIN
    0x14,0x20,  //CC_HEXA_CYB_GAIN
    0x15,0x20,  //CC_HEXA_BMG_GAIN
    0x16,0xFF,  //CC_HEXA_BMG_GAIN
    0x17,0x01,  //CC_HEXA_BMG_GAIN
    0x18,0x20,  //CC_HEXA_BMG_GAIN
    0x25,0x05,  //
    0x93,0x31,  //CTC_REF_0
    0x94,0x35,  //CTC_REF_1
    0x95,0x44,  //CTC_REF_2
    0x96,0x4D,  //CTC_REF_3
    0x97,0x00,  //CTC_CC11A_S
    0x98,0x84,  //CTC_CC11A_0
    0x99,0xA6,  //CTC_CC11A_1
    0x9A,0x7E,  //CTC_CC11A_2
    0x9B,0x70,  //CTC_CC11A_3
    0x9C,0x00,  //CTC_CC11B_S
    0x9D,0x84,  //CTC_CC11B_0
    0x9E,0xA6,  //CTC_CC11B_1
    0x9F,0x7E,  //CTC_CC11B_2
    0xA0,0x70,  //CTC_CC11B_3
    0xA1,0xCC,  //CTC_CC12_S
    0xA2,0xF8,  //CTC_CC12_0
    0xA3,0x06,  //CTC_CC12_1
    0xA4,0xFC,  //CTC_CC12_2
    0xA5,0x10,  //CTC_CC12_3
    0xA6,0xF0,  //CTC_CC21_S
    0xA7,0xD0,  //CTC_CC21_0
    0xA8,0xE8,  //CTC_CC21_1
    0xA9,0x02,  //CTC_CC21_2
    0xAA,0x0C,  //CTC_CC21_3
    0xAB,0x00,  //CTC_CC22A_S
    0xAC,0x74,  //CTC_CC22A_0
    0xAD,0x63,  //CTC_CC22A_1
    0xAE,0x7B,  //CTC_CC22A_2
    0xAF,0x78,  //CTC_CC22A_3
    0xB0,0x00,  //CTC_CC22B_S
    0xB1,0x68,  //CTC_CC22B_0
    0xB2,0x63,  //CTC_CC22B_1
    0xB3,0x7B,  //CTC_CC22B_2
    0xB4,0x6B,  //CTC_CC22B_3
    0x2B,0x7C,
    0x2C,0x79,
    0x2D,0x77,
    0x2E,0x75,
    0x2F,0x74,
    0x30,0x73,
    0x31,0x80,
    0x32,0x72,
    0x33,0x72,
    0x34,0x73,
    0x35,0x74,
    0x36,0x75,
    0x37,0x77,
    0x38,0x7A,
    0x39,0x7D,
    0x3B,0x7D,
    0x3C,0x7A,
    0x3D,0x77,
    0x3E,0x75,
    0x3F,0x74,
    0x40,0x73,
    0x41,0x73,
    0x45,0x74,
    0x48,0x7A,
    0x49,0x7D,
    0x4B,0x7D,
    0x4C,0x7A,
    0x4D,0x77,
    0x50,0x73,
    0x51,0x73,
    0x52,0x72,
    0x53,0x72,
    0x54,0x73,
    0x55,0x74,
    0x56,0x75,
    0x57,0x77,
    0x58,0x7A,
    0x59,0x7D,
    0x5A,0x80,
    0x5B,0x7D,
    0x5D,0x77,
    0x5E,0x75,
    0x60,0x73,
    0x61,0x73,
    0x62,0x72,
    0x63,0x72,
    0x65,0x74,
    0x67,0x77,
    0x68,0x7A,
    0x69,0x7D,
    0x6A,0x81,
    0x6B,0x7D,
    0x6C,0x7A,
    0x6D,0x78,
    0x6E,0x75,
    0x6F,0x74,
    0x70,0x73,
    0x71,0x73,
    0x72,0x72,
    0x73,0x72,
    0x74,0x73,
    0x75,0x74,
    0x76,0x75,
    0x77,0x78,
    0x78,0x7A,
    0x79,0x7E,
    0xB5,0x2B,  //CTOG_REFGAIN0
    0xB6,0x31,  //CTOG_REFGAIN1
    0xB7,0x4E,  //CTOG_REFGAIN2
    0xB8,0x5A,  //CTOG_REFGAIN3
    0xB9,0x83,  //0xB_RG_OFFSET0
    0xBA,0x83,  //0xB_RG_OFFSET1
    0xBB,0x7C,  //0xB_RG_OFFSET2
    0xBC,0x7E,  //0xB_RG_OFFSET3
    0xBD,0x80,  //0xB_BG_OFFSET0
    0xBE,0x80,  //0xB_BG_OFFSET1
    0xBF,0x80,  //0xB_BG_OFFSET2
    0xC0,0x80,  //0xB_BG_OFFSET3
    
    0x00,0x01,
    0xBF,0x40,
    0xC0,0x50,  //EDG_MAX        
    0xC1,0x02,  //EDG_ALPHA      
    0xC2,0x00,
    0xC3,0x04,  //EDG_TGAIN_TH1_H
    0xC4,0x00,  //EDG_TGAIN_TH1_L
    0xC5,0x18,  //EDG_TGAIN_TH2_H
    0xC6,0x00,  //EDG_TGAIN_TH2_L    
    0x0C,0xC8,
    0xC9,0x40,
    0xCA,0x02,
    0xD5,0x64,  //CSUP_Y1_L
    0xD7,0x98,  //CSUP_Y2_L
    0xD9,0x08,  //CSUP_TGAIN_TH1_H
    0xDB,0x10,  //CSUP_TGAIN_TH2_H
    0xDD,0x00,  //CSUP_GAIN1
    0xDE,0x25,  //CSUP_GAIN2
    0xE5,0x00,  //CONTRA_EGP_TH1_T
    0xE6,0x0B,  //CONTRA_EGP_TH1_H
    0xE7,0xB0,  //CONTRA_EGP_TH1_M
    0xE9,0x00,  //CONTRA_EGP_TH2_T
    0xEA,0x17,  //CONTRA_EGP_TH2_H
    0xEB,0x60,  //CONTRA_EGP_TH2_M
    0xF0,0x0B,  //CONTRA_EGP_TH1_H
    0xF1,0xB0,  //CONTRA_EGP_TH1_M
    0xF3,0x17,  //CONTRA_EGP_TH2_H
    0xF4,0x60,  //CONTRA_EGP_TH2_M
    0xAC,0x38,  //HS11
    0xAF,0x38,  //HS22
    0xB0,0x03,  //HS_UO
    0xB1,0x02,  //HS_VO
    0xFE,0x00,  //GRAYBOOST_TH2
    0x00,0x02,
    0x2B,0x40,  //YC_CG0
    0x2C,0x40,  //YC_CG1
    0x2D,0x40,  //YC_CG2    
    0x2E,0x40,  //YC_CG3
    0x2F,0x40,  //YC_CG4
    0x30,0x40,  //YC_CG5
    0x31,0x40,  //YC_CG6
    0x32,0x40,  //YC_CG7
    0x33,0x40,  //YC_CG8
    0x34,0x40,  //YC_CG9 
    0x35,0x01,  //CCR11
    0x36,0x14,  //CCR11
    0x37,0x03,  //CCR12
    0x38,0x70,  //CCR12
    0x39,0x03,  //CCR13
    0x3A,0xFC,  //CCR13
    0x3B,0x03,  //CCR21
    0x3C,0x9B,  //CCR21
    0x3D,0x01,  //CCR22
    0x3E,0x28,  //CCR22
    0x3F,0x03,  //CCR23
    0x40,0xC1,  //CCR23
    0x41,0x03,  //CCR31
    0x42,0xEC,  //CCR31
    0x43,0x03,  //CCR32
    0x44,0x10,  //CCR32
    0x45,0x01,  //CCR33
    0x46,0x87,  //CCR33
    0xA0,0x38,  //BF_ALPHA
    0xA8,0x0C,
    0xA9,0x00,
    0xB1,0x01,
    0xB2,0x00,
    0xB3,0x02,
    0xB4,0x00,
    0xBC,0x01,
    0xBD,0x02,
    0xBE,0x02,
    0xBF,0x05,
    0xC0,0x09,
    0xC1,0x05,
    0xC2,0x07,
    0xC3,0x0B,
    0xC4,0x0F,
    0xC5,0x16,
    0xC6,0x1F,
    0xC7,0x07,
    0xC8,0x0A,
    0xC9,0x0F,
    0xCA,0x15,
    0xCB,0x1F,
    0xCC,0x2C,
    0xCD,0x09,
    0xCE,0x0D,
    0xCF,0x12,
    0xD0,0x19,
    0xD1,0x25,
    0xD2,0x35,
    0xD3,0x0A,
    0xD4,0x0F,
    0xD5,0x15,
    0xD6,0x1D,
    0xD7,0x2B,
    0xD8,0x3D,
    0xDA,0x01,
    0xDB,0x02,
    0xDC,0x02,
    0xDD,0x05,
    0xDE,0x09,
    0xDF,0x05,
    0xE0,0x07,
    0xE1,0x0B,
    0xE2,0x0F,
    0xE3,0x16,
    0xE4,0x1F,
    0xE5,0x07,
    0xE6,0x0A,
    0xE7,0x0F,
    0xE8,0x15,
    0xE9,0x1F,
    0xEA,0x2C,
    0xEB,0x09,
    0xEC,0x0D,
    0xED,0x12,
    0xEE,0x19,
    0xEF,0x25,
    0xF0,0x35,
    0xF1,0x0A,
    0xF2,0x0F,
    0xF3,0x15,
    0xF4,0x1D,
    0xF5,0x2B,
    0xF6,0x3D,
    0xFC,0x02,
    0xFD,0x10,
    0x00,0x00,                       
    0x38,0x3F, //blc iir             
    0x00,0x00,
    0x0D,0x02,  //data output

};

#else
const u8 ZN220_SETTING_1080P_10FPS_1[] = 
{
0x00,0x00,
0x0C,0x02,
0x0D,0x00,
0xD8,0x00,
0xD9,0x99,
0xDA,0xE8,
0xDB,0x0B,
0xDC,0xCF,
0x0E,0x10,
0x18,0xC0,
0x1C,0x78,
0x1E,0x03,
0x1F,0x04,
0x38,0x00,
0x06,0x08,
0x07,0x97,
0x57,0x00,
0x58,0x55,
0x59,0x00,
0x5A,0x5F,
0x5D,0x00,
0x5E,0xCE,
0x6F,0x00,
0x70,0xCF,
0x88,0x00,
0x89,0xD2,
0x73,0x00,
0x74,0xD2,
0x6B,0x07,
0x6C,0x9E,
0x6D,0x07,
0x6E,0xD0,
0x77,0x08,
0x78,0x19,
0x79,0x08,
0x7A,0x1C,
0x7B,0x07,
0x7C,0xD0,
0x7D,0x08,
0x7E,0x02,
0x8C,0x00,
0x8D,0x22,
0x92,0x02,
0x9B,0xC0,
0xD0,0x88,
0xD2,0x0E,
0xD3,0x44,
0xD4,0x7B,
0xD5,0x0C,
0xD6,0x4C,
0xD7,0xBC,
0x20,0x80,
0x21,0x2F,
0x22,0xD0,
0xB5,0x01,
0x00,0x01,
0x0F,0x80,
0x03,0x80,
0x04,0x00,
0x06,0x80,
0x13,0x10,
0x00,0x03,
0xFC,0x6B,
0xFD,0x6B,
0x00,0x01,
0x01,0xDB,
0x02,0x53,
0x05,0xF8,
0x06,0x81,
0x07,0xF4,
0x09,0x8B,
0x09,0xCB,
0x0A,0xD6,
0x0B,0x02,
0x13,0x00,
0x9E,0x9C,
0x9F,0x80,
0xA0,0xD1,
0x00,0x03,
0x3E,0x10,
0x3F,0x10,
0x00,0x04,
0x65,0x0C,
0x00,0x02,
0x01,0x00,
0x02,0x10,
0x03,0x1D,
0x04,0x29,
0x05,0x33,
0x06,0x45,
0x07,0x56,
0x08,0x6C,
0x09,0x7D,
0x0A,0x9C,
0x0B,0xB3,
0x0C,0xC9,
0x0D,0xDD,
0x0E,0xEF,
0x10,0x08,
0x11,0x00,
0x12,0x0C,
0x13,0x00,
0x14,0x00,
0x15,0x30,
0x00,0x03,
0x1C,0x40,
0x2C,0x18,
0x2D,0xFE,
0x2E,0xFE,
0x4A,0x00,
0x01,0x00,
0x02,0x00,
0x03,0x07,
0x04,0x7E,
0x05,0x00,
0x06,0x00,
0x07,0x04,
0x08,0x37,
0x09,0x00,
0x0A,0x00,
0x0B,0x07,
0x0C,0x7E,
0x0D,0x00,
0x0E,0xA0,
0x0F,0x04,
0x10,0x37,
0x11,0x02,
0x12,0x00,
0x13,0x05,
0x14,0x80,
0x15,0x01,
0x16,0x40,
0x17,0x04,
0x18,0x37,
0x1C,0x40,
0x24,0x08,
0x25,0x10,
0x26,0x10,
0x27,0x00,
0x28,0x0C,
0x29,0x0C,
0x2A,0x10,
0x2B,0x00,
0x21,0xAB,
0x2F,0x03,
0x30,0x00,
0x31,0x10,
0x32,0x00,
0x33,0x80,
0x34,0x70,
0x35,0x40,
0x36,0x30,
0x37,0x00,
0x38,0x01,
0x39,0x06,
0x3A,0x10,
0x3E,0x10,
0x3F,0x10,
0x40,0x10,
0x42,0x00,
0x43,0x00,
0x4C,0x04,
0x4D,0x63,
0x4E,0x04,
0x4F,0x63,
0x50,0x04,
0x51,0x63,
0x52,0x00,
0x53,0x72,
0x54,0x28,
0x55,0x01,
0x56,0x8F,
0x57,0x8C,
0x91,0x00,
0x92,0xA9,
0x93,0x00,
0x94,0x00,
0x95,0xCA,
0x96,0x00,
0x97,0xA3,
0x00,0x07,
0xDE,0x03,
0xDF,0x01,
0xE3,0x01,
0xE4,0x80,
0xE7,0x06,
0xE8,0xA4,
0xEB,0x1A,
0xEC,0x00,
0xED,0x00,
0xEE,0x72,
0xEF,0x28,
0xF0,0xE0,
0xF1,0x1A,
0xF2,0x00,
0xF3,0x80,
0xF4,0x01,
0x00,0x04,
0x41,0xE6,
0x42,0x15,
0x43,0x18,
0x44,0x00,
0x01,0x00,
0x02,0x00,
0x03,0x07,
0x04,0x7F,
0x05,0x00,
0x06,0x00,
0x07,0x04,
0x08,0x37,
0x09,0x00,
0x0A,0x00,
0x0B,0x07,
0x0C,0x7F,
0x0D,0x00,
0x0E,0x00,
0x0F,0x04,
0x10,0x37,
0x11,0x30,
0x12,0xFF,
0x13,0x00,
0x14,0x00,
0x15,0x00,
0x16,0xFF,
0x17,0xFF,
0x18,0xFF,
0x19,0x10,
0x1A,0x00,
0x1B,0x08,
0x1C,0x00,
0x1D,0x10,
0x1E,0x00,
0x1F,0x10,
0x20,0x78,
0x21,0x47,
0x22,0x35,
0x23,0x2D,
0x24,0x6A,
0x25,0x6F,
0x26,0x2E,
0x27,0x47,
0x28,0x35,
0x29,0x02,
0x2A,0x05,
0x2B,0x07,
0x2C,0x0A,
0x2D,0x02,
0x2E,0x05,
0x2F,0x07,
0x30,0x0A,
0x31,0x02,
0x32,0x05,
0x33,0x07,
0x34,0x0A,
0x35,0x01,
0x36,0xF6,
0x37,0x00,
0x38,0xB3,
0x39,0x86,
0x3A,0x42,
0x3B,0x20,
0x3C,0xF0,
0x3D,0x0A,
0x3E,0x0A,
0x3F,0x0A,
0x40,0x0A,
0x45,0x10,
0x46,0x10,
0x47,0x0C,
0x48,0x00,
0x49,0x10,
0x4A,0x00,
0x4B,0x10,
0x5B,0x03,
0x5C,0x03,
0x5D,0x03,
0x5E,0x00,
0x5F,0x01,
0x60,0x01,
0x61,0x00,
0x62,0x01,
0x63,0x01,
0x65,0x0C,
0x70,0x80,
0x71,0x80,
0x80,0x58,
0x81,0x27,
0x82,0x58,
0x83,0x26,
0x84,0x58,
0x85,0x27,
0x86,0x58,
0x87,0x26,
0x88,0x58,
0x89,0x27,
0x8A,0x58,
0x8B,0x26,
0x8C,0x51,
0x8D,0x2E,
0x8E,0x51,
0x8F,0x2E,
0x90,0x01,
0x91,0x00,
0x92,0x10,
0x93,0x00,
0x94,0x58,
0x95,0x27,
0x96,0x59,
0x97,0x26,
0x98,0x80,
0x99,0x80,
0x00,0x00,
0xBD,0x01,	//BLC_TGAIN_TH1_H
0xBE,0x00,	//BLC_TGAIN_TH1_L
0xBF,0x10,	//BLC_TGAIN_TH2_H
0xC0,0x00,	//BLC_TGAIN_TH2_L
0xC1,0xFF,	//BLC_OFFSET_PARAM1
0xC2,0xFE,	//BLC_OFFSET_PARAM2
0x00,0x05,
0x00,0x06,
0xDC,0x1A,
0xDD,0x00,
0xD4,0x03,
0xD6,0x10,
0xD7,0x18,
0xD8,0x20,
0xD9,0x30,
0xDA,0x5B,
0x00,0x06,
0xBC,0x05,
};



const u8 ZN220_SETTING_1080P_10FPS_2[] = 
{
0x00,0x07,
0x19,0x00,
0x1A,0xA3,
0x1B,0x40,
0x1C,0x2C,
0x1D,0x43,
0x1E,0x28,
0x1F,0x80,
0x20,0xA3,
0x21,0xC0,
0x22,0x2C,
0x23,0xC3,
0x24,0x28,
0x01,0x1D,
0x02,0x02,
0x03,0xFE,
0x04,0x20,
0x05,0x24,
0x06,0x04,
0x07,0x02,
0x08,0x22,
0x09,0x23,
0x0A,0x00,
0x0B,0x02,
0x0C,0x1D,
0x0D,0x1E,
0x0E,0x03,
0x0F,0xFD,
0x10,0x20,
0x11,0x1F,
0x12,0x04,
0x13,0x08,
0x14,0x24,
0x15,0x1E,
0x16,0x01,
0x17,0x06,
0x18,0x1A,
0x25,0x05,
0x93,0x29,
0x94,0x2E,
0x95,0x44,
0x96,0x4D,
0x97,0x00,
0x98,0xCD,
0x99,0xA6,
0x9A,0x7E,
0x9B,0x70,
0x9C,0x00,
0x9D,0xCD,
0x9E,0xA6,
0x9F,0x7E,
0xA0,0x70,
0xA1,0xCF,
0xA2,0xF8,
0xA3,0x06,
0xA4,0xFC,
0xA5,0xFF,
0xA6,0xF0,
0xA7,0xCA,
0xA8,0xE8,
0xA9,0x02,
0xAA,0x0C,
0xAB,0x00,
0xAC,0x6C,
0xAD,0x63,
0xAE,0x7B,
0xAF,0x6B,
0xB0,0x00,
0xB1,0x6C,
0xB2,0x63,
0xB3,0x7B,
0xB4,0x6B,
0x2A,0x80,
0x2B,0x7C,
0x2C,0x79,
0x2D,0x77,
0x2E,0x75,
0x2F,0x74,
0x30,0x73,
0x31,0x80,
0x32,0x72,
0x33,0x72,
0x34,0x73,
0x35,0x74,
0x36,0x75,
0x37,0x77,
0x38,0x7A,
0x39,0x7D,
0x3A,0x80,
0x3B,0x7D,
0x3C,0x7A,
0x3D,0x77,
0x3E,0x75,
0x3F,0x74,
0x40,0x73,
0x41,0x73,
0x42,0x72,
0x43,0x72,
0x44,0x73,
0x45,0x74,
0x46,0x75,
0x47,0x77,
0x48,0x7A,
0x49,0x7D,
0x4A,0x80,
0x4B,0x7D,
0x4C,0x7A,
0x4D,0x77,
0x4E,0x75,
0x4F,0x74,
0x50,0x73,
0x51,0x73,
0x52,0x72,
0x53,0x72,
0x54,0x73,
0x55,0x74,
0x56,0x75,
0x57,0x77,
0x58,0x7A,
0x59,0x7D,
0x5A,0x80,
0x5B,0x7D,
0x5C,0x7A,
0x5D,0x77,
0x5E,0x75,
0x5F,0x74,
0x60,0x73,
0x61,0x73,
0x62,0x72,
0x63,0x72,
0x64,0x73,
0x65,0x74,
0x66,0x75,
0x67,0x77,
0x68,0x7A,
0x69,0x7D,
0x6A,0x81,
0x6B,0x7D,
0x6C,0x7A,
0x6D,0x78,
0x6E,0x75,
0x6F,0x74,
0x70,0x73,
0x71,0x73,
0x72,0x72,
0x73,0x72,
0x74,0x73,
0x75,0x74,
0x76,0x75,
0x77,0x78,
0x78,0x7A,
0x79,0x7E,
0xB5,0x2B,
0xB6,0x31,
0xB7,0x41,
0xB8,0x4A,
0xB9,0x80,
0xBA,0x80,
0xBB,0x81,
0xBC,0x80,
0xBD,0x7F,
0xBE,0x81,
0xBF,0x80,
0xC0,0x80,
0x00,0x01,
0x3E,0x80,
0x3F,0x81,
0x40,0x81,
0x41,0x82,
0x42,0x83,
0x43,0x85,
0x44,0x86,
0x45,0x87,
0x46,0x89,
0x47,0x8A,
0x48,0x8C,
0x49,0x8E,
0x4A,0x90,
0x4B,0x92,
0x4C,0x94,
0x4D,0x96,
0x4E,0x98,
0x4F,0x9B,
0x50,0x9D,
0x51,0x9F,
0x52,0xA1,
0x53,0xA4,
0x54,0xA7,
0x55,0xA9,
0x56,0xAC,
0x57,0xAF,
0x58,0xB2,
0x59,0xB7,
0x5A,0xBC,
0x5B,0xBF,
0x5C,0xC5,
0x5D,0xCC,
0x5E,0xD6,
0x5F,0x3A,
0x60,0x0D,
0x61,0xD1,
0x62,0x0B,
0x63,0x31,
0xB2,0x00,
0xB3,0x7F,
0xB4,0x00,
0xB5,0x7F,
0xB6,0x03,
0xB7,0xFD,
0xB8,0x00,
0xB9,0x02,
0xBA,0x00,
0xBB,0x7B,
0xBC,0x00,
0xBD,0x7B,
0xBF,0x40,
0xC0,0x30,
0xC1,0x04,
0xC2,0x00,
0xC3,0x03,
0xC4,0x00,
0xC5,0x08,
0xC6,0x00,
0x0C,0xC8,
0xC9,0x40,
0xCA,0x02,
0xD4,0x00,
0xD5,0x00,
0xD6,0x03,
0xD7,0x98,
0xD8,0x00,
0xD9,0x08,
0xDA,0x00,
0xDB,0x0C,
0xDC,0x00,
0xDD,0x00,
0xDE,0x10,	
0xDF,0x80,	//OSD_contrast_gain
0xE0,0x04,	//OSD_brightness_offset
0xE2,0x80,
0xE3,0x80,
0xE4,0x80,
0xE5,0x00,
0xE6,0x0B,
0xE7,0xB0,
0xE8,0x00,
0xE9,0x00,
0xEA,0x17,
0xEB,0x60,
0xEC,0x00,
0xEF,0x00,
0xF0,0x0B,
0xF1,0xB0,
0xF2,0x00,
0xF3,0x17,
0xF4,0x60,
0xAC,0x24,
0xAD,0x00,
0xAE,0x00,
0xAF,0x24,
0xFD,0x00,
0xFE,0x00,
0x00,0x02,
0x16,0x00,
0x17,0x0B,
0x18,0x13,
0x19,0x1D,
0x1A,0x25,
0x1B,0x34,
0x1C,0x40,
0x1D,0x56,
0x1E,0x69,
0x1F,0x89,
0x20,0xA5,
0x21,0xBE,
0x22,0xD6,
0x23,0xEB,
0x24,0x80,
0x25,0x08,
0x26,0x00,
0x27,0x0C,
0x28,0x00,
0x29,0x80,
0x2A,0x80,
0x2B,0x40,
0x2C,0x40,
0x2D,0x40,
0x2E,0x40,
0x2F,0x40,
0x30,0x40,
0x31,0x40,
0x32,0x40,
0x33,0x40,
0x34,0x40,
0x35,0x01,
0x36,0x20,
0x37,0x03,
0x38,0x68,
0x39,0x03,
0x3A,0xF8,
0x3B,0x03,
0x3C,0x9B,
0x3D,0x01,
0x3E,0x28,
0x3F,0x03,
0x40,0xC1,
0x41,0x03,
0x42,0xF4,
0x43,0x03,
0x44,0x20,
0x45,0x01,
0x46,0x6C,
0x00,0x02,
0x47,0x00,
0x48,0x08,
0x49,0x00,
0x4A,0x00,
0x4B,0x00,
0x4C,0x08,
0xA1,0x00,
0xA2,0x50,
0xA3,0x00,
0xA4,0x50,
0xA5,0x00,
0xA6,0x08,
0xA7,0x00,
0xA8,0x0C,
0xA9,0x00,
0xAA,0x20,
0xAB,0x20,
0xAC,0x00,
0xAD,0x50,
0xAE,0x00,
0xAF,0x80,
0xB0,0x00,
0xB1,0x01,
0xB2,0x00,
0xB3,0x02,
0xB4,0x00,
0xB5,0x00,
0xB6,0x08,
0xB7,0x00,
0xB8,0x10,
0xB9,0x00,
0xBA,0x00,
0xBB,0x00,
0xBC,0x01,
0xBD,0x02,
0xBE,0x02,
0xBF,0x05,
0xC0,0x09,
0xC1,0x05,
0xC2,0x07,
0xC3,0x0B,
0xC4,0x0F,
0xC5,0x16,
0xC6,0x1F,
0xC7,0x07,
0xC8,0x0A,
0xC9,0x0F,
0xCA,0x15,
0xCB,0x1F,
0xCC,0x2C,
0xCD,0x09,
0xCE,0x0D,
0xCF,0x12,
0xD0,0x19,
0xD1,0x25,
0xD2,0x35,
0xD3,0x0A,
0xD4,0x0F,
0xD5,0x15,
0xD6,0x1D,
0xD7,0x2B,
0xD8,0x3D,
0xD9,0x00,
0xDA,0x01,
0xDB,0x02,
0xDC,0x02,
0xDD,0x05,
0xDE,0x09,
0xDF,0x05,
0xE0,0x07,
0xE1,0x0B,
0xE2,0x0F,
0xE3,0x16,
0xE4,0x1F,
0xE5,0x07,
0xE6,0x0A,
0xE7,0x0F,
0xE8,0x15,
0xE9,0x1F,
0xEA,0x2C,
0xEB,0x09,
0xEC,0x0D,
0xED,0x12,
0xEE,0x19,
0xEF,0x25,
0xF0,0x35,
0xF1,0x0A,
0xF2,0x0F,
0xF3,0x15,
0xF4,0x1D,
0xF5,0x2B,
0xF6,0x3D,
0xF8,0x12,
0xF9,0x30,
0xFC,0x02,
0xFD,0x10,
0x66,0xFF,
0x67,0xFF,
0x68,0xFF,
0x69,0xFF,
0x6A,0xFF,
0x6B,0xFF,
0x6C,0xFF,
0x6D,0xFF,
0x6E,0xFF,
0x6F,0xFF,
0x00,0x03,
0x3E,0x0C,
0x3F,0x0C,
0x00,0x04,
0x65,0x08,
0x00,0x00,
0x38,0x3F,
0x00,0x00,
0x0D,0x02,
};

#endif


/*
 *********************************************************************************************************
 * Extern Variables
 *********************************************************************************************************
 */
#if RFIU_SUPPORT 
extern DEF_RFIU_UNIT_CNTL gRfiuUnitCntl[MAX_RFIU_UNIT];
#endif
extern const s32 d50_IPU_CCM[10];
// OSD_AFreport, DefectPixel
#if(FACTORY_TOOL == TOOL_ON)
extern u8 config_mode_enable[LEVEL1_NODE_NUM*2];
#endif

extern s32 mp4_avifrmcnt, isu_avifrmcnt; /*BJ 0530 S*/	
extern s16 AWBgain_Preview[3];
extern s16 AWBgain_Capture[3];
extern s16 ipuColorCorrTransform[10]; //for 9002x, CCM 與CTM 合併用.
/* Peter 070104 */
extern u32 IISMode;        // 0: record, 1: playback, 2: receive and playback audio in preview mode
extern s64 IISTime;        // Current IIS playback time(micro second)
extern u32 IISTimeUnit;    // IIS playback time per DMA(micro second)

#if GET_SIU_RAWDATA_PURE
   extern s16 siuAdjustSW;
   extern s16 siuAdjustAGC;
#endif

/* FACTORY_TOOL */
// OSD_AFreport

extern u8 DefectPixel_Msg;

// OB
extern u16 OB_B;
extern u16 OB_Gb;
extern u16 OB_R;
extern u16 OB_Gr;



extern u8 uiMenuVideoSizeSetting;


#if HW_MD_SUPPORT
  extern u8 MotionDetect_en;
#endif

extern u8 TvOutMode;
extern u8 SIUMODE;

//--------- Motion Detect----------//


/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */ 


/* SW */
s32 siuGetAeYavg(u8*);

s32 siuInit(void);
s32 siuSensorInit(u8,u8);
s32 siuAeInit(void);
s32 siuSetOpticalBlack(u8, u8);
s32 siuSetDigitalGain(u8, u16);
s32 siuSetLensShading(SIU_LENS_SHADING*);
s32 siuSetRGB_PreGamma(u16*, u8*,u8*,u8*,u8*);
s32 siuSetAeWin(AE_WIN*);
s32 siuSetAwbWin(void);
s32 siuSetAeBlkWeight(u8* , u16*);
s32 siuGetOutputSize(u16*, u16*);

s32 siuGetAeWinYsum(u8, u16*); 
s32 siuGetAeHist(u8, SIU_HIST*);
s32 siuCaptureVideo(s8); /*BJ 0530 S*/	
s32 siuVideoZoom(s8 zoomFactor);

void siuTask(void*); 
s32 siuAutoFocus(void);
s32 siuAutoWhiteBalance(void);
s32 setSensorWinSize(s8, s8);
u16 getPreviewZoomScale(s32);
u16 getVideoZoomScale(s32);
void siuStop(void);
void siuSetSensorDayNight(u8 Level);

s32 ZN220_1080P_10fps(void);
s32 ZN220_720P_15fps(void);
u16 Cal_LOG2(u16 X);
 
void ZN220_SoftReset(void);
void ZN220_AECAWBOn(void);
void ZN220_BLCSpeed(u8 Level);
void ZN220_IRLedSwitch(void);
s32 siuSetSensorMirrorFilp(u8 mirror, u8 filp);

#if HW_MD_SUPPORT
 extern  void  mduMotionDetect_init();  
#endif

/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */  

/*

Routine Description:

	Initialize the Sensor Interface Unit.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/

s32 siuInit(void)
{

    // Create the semaphore 
    siuSemEvt = OSSemCreate(0);
    siuCapEvt = OSSemCreate(0);
    sysSIURst();


#if HW_MD_SUPPORT
    mduMotionDetect_init();  
#endif

#if (MULTI_CHANNEL_SEL & 0x01)
    OSTaskCreate(SIU_TASK, SIU_TASK_PARAMETER, SIU_TASK_STACK, SIU_TASK_PRIORITY_PVW); 
    siuSuspendTask();
#endif

	return 1;
}


/* SW 0107 S */
#define AE_TAB_ENTRY_NUM    90

void siuSetAETbl(void)
{
    
}

/* SW 0107 E */

void SetSensorSW(u16 EL)
{
	
}

void SetSensor_DigitalGain(u8 ggain)//set globe Digital gain
{
	

}

void SetSensor_AnalogGain(Analog_Gain ggain)//set globe Analog gain
{
}

void siuAdjustAE(u16 cur_index)
{
	
}


s32 siuPreviewInit(u8 zoomFactor,u8 mode)
{

    while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
	// initialize sensor
	//rfiuVideoInFrameRate=siuSensorInit(mode,zoomFactor);
	
	// initialize the siu module
	// reset the SIU module
	//SiuSensCtrl =	SIU_RESET;
	
	// Set start address of raw buffer	
	SiuRawAddr =	(u32)siuRawBuf;

	SiuSensCtrl =	
	            SIU_NORMAL |
			    SIU_ENA |    							//enable SIU module
        		SIU_SLAVE |                             //For CMOS sensor
        		SIU_VSYNC_ACT_LO |                      //Vsync active low
        		SIU_HSYNC_ACT_LO |                      //Hsync active low
                SIU_DEF_PIX_DISA |                      //D.pixel compesation disable
        		SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
        		SIU_INT_ENA_FRAM |                      //Frame end interrupt enaable
        		SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
        		SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
        		SIU_DST_SRAM |                          //SIU to SRAM
            #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      // data[9:2]
                SIU_DATA_12b |
            #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  // data[7:0]
                SIU_DATA_10b | 
            #endif
        		SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
                SIU_TEST_PATRN_DISA | //
        		SIU_OB_COMP_DISA |                      //OB correct disable 
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
        		SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        		SIU_AE_DISA |                           //AE report disable
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.


    SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                SIU_YUVMAP_36;
        
	
    SiuSensCtrl |= SIU_CAPT_ENA;

	return 1;	
}

s32 siuVideoClipInit(u8 zoomFactor)
{
	// initialize the siu module
	// reset the SIU module
	//SiuSensCtrl =	SIU_RESET;
    
	// Set start address of raw buffer	
	SiuRawAddr =	(u32)siuRawBuf;

	// Set sensor control 
	SiuSensCtrl =	
	            SIU_NORMAL |
			    SIU_ENA |    							//enable SIU module
        		SIU_SLAVE |                             //For CMOS sensor
        		SIU_VSYNC_ACT_HI |                      //Vsync active low
        		SIU_HSYNC_ACT_LO |                      //Hsync active low
                SIU_DEF_PIX_DISA |
        		SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
        		SIU_INT_ENA_FRAM |                      //Frame end interrupt enaable
        		SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
        		SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
        		SIU_DST_SRAM |                          //SIU to SRAM
        
           #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      // data[9:2]
                SIU_DATA_12b |
           #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  // data[7:0]
                SIU_DATA_10b | 
           #endif
        		SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
        		SIU_OB_COMP_DISA |                      //OB correct enable 
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
        		SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        		SIU_AE_DISA |                            //AE report enable
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.



    SiuDebugSel=SIU_INSEL_CCIR601       |  //0x08364000;
                SIU_YUVMAP_36;
    
    SiuSensCtrl |= SIU_CAPT_ENA;
    	 	    		
	return 1;	
}

s32 siuSetFlicker50_60Hz(int flicker_sel)
{

    switch(uiMenuVideoSizeSetting)
    {
        case UI_MENU_VIDEO_SIZE_1920x1072:
        case UI_MENU_VIDEO_SIZE_1600x896:
            ZN220_1080P_10fps();
            break;
        case UI_MENU_VIDEO_SIZE_1280X720:
        case UI_MENU_VIDEO_SIZE_640x352:
        case UI_MENU_VIDEO_SIZE_704x480:
            ZN220_720P_15fps();
            break;
    }
    DEBUG_SIU("siuSetFlicker50_60Hz %d\n", flicker_sel);

}

s32 siuSetExposureValue(s8 expBiasValue)
{
    siuY_TargetIndex = expBiasValue;
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    #else
    DEBUG_SIU("siuSetExposureValue(%d) Mean %x\n", expBiasValue, AETargetMeanTab[expBiasValue]);
    #endif
    //RX 6 level only, index RX to TX {0,2,4,5,6,8}
    i2cWrite_ZN220(0x00, 0x03); 
    i2cWrite_ZN220(0x3C, AETargetMeanTab[expBiasValue]); 
}


s32 siuSetContrast(s8 expBiasValue)
{}

s32 siuSetSaturation(s8 expBiasValue)
{}

s32 siuSetSharpness(s8 expBiasValue)
{}
s32 siuSetImage(void)
{
    //siuSetFlicker50_60Hz(iconflag[UI_MENU_SETIDX_TX_FLICKER]); //can't isolate apply flicker setting, 
    siuSetExposureValue(iconflag[UI_MENU_SETIDX_TX_BRIGHT]);

}

s32 ZN220_Init_1080P(void) //init setting is original 1080p30 related
{
    int i;
    u16 data;

    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    #else
    DEBUG_SIU("ZN220_Init_1080P() \n");
    #endif

    //gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    //OSTimeDly(1); //pwron pullup, low 32ms+20ms, high 50ms
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );
    //for(i=0;i<0x150000;i++); //252000=100ms 50000 10ms 150000 48ms
    OSTimeDly(1); //pwron pullup, low 30ms, high at least 30ms
    //////////////////////////// Base Settings ////////////////////////////////
	//ZN220_SETTING_1080P_10FPS_1 = sensor basic setting and AEC setting
    for(i = 0; i < ElemsOfArray(ZN220_SETTING_1080P_10FPS_1) - 1; i+=2)
    {
        i2cWrite_ZN220(ZN220_SETTING_1080P_10FPS_1[i], (u8)ZN220_SETTING_1080P_10FPS_1[i+1]);
    }
}

s32 ZN220_Init_1080P_2(void) //init setting is original 1080p30 related
{
    int i;
    u16 data;

    //ZN220_SETTING_1080P_10FPS_2 = other setting
    for(i = 0; i < ElemsOfArray(ZN220_SETTING_1080P_10FPS_2) - 1; i+=2)
    {
        i2cWrite_ZN220(ZN220_SETTING_1080P_10FPS_2[i], (u8)ZN220_SETTING_1080P_10FPS_2[i+1]);
    }

}

s32 ZN220_Manual_AEC(void) //init setting is original 1080p30 related
{
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    u32 i;
  #if (ADAPTIVE_AE_BY_ADC == 2)
    int adcavg = 0;
    u8 adcgroup = 0;
    u8 ADC_GROUP_MAX = 42;
    int adcavglast = 0;
    u16 data;

    for(i=0;i<4;i++)
        adcavg += adcGetValue(2);
    adcavg=adcavg>>2;

    //DEBUG_CYAN("%x index%d %x %d\n", adcavg, (adcavg>>6), ((adcavg>>6) <<6) , adcavg - ((adcavg>>6) <<6));
    if (SIUMODE == SIU_DAY_MODE)
    {
    #if NEW_LIGHT_SENSOR
        adcgroup = (adcavg - 0x30) / 0x60; 
        adcgroup = adcgroup < ADC_GROUP_MAX ? adcgroup : ADC_GROUP_MAX - 1;
    #else

        if (adcavg > 0x930)
        {//a50 23, a80 24, ab0 25, ae0 26, b10 27, b40 28, b70 29
            adcgroup = (adcavg - 0x930) / 0x30; // 930 18, 960 19, 990 20, 9c0 21, a20 22
            adcgroup = adcgroup + 18 < ADC_GROUP_MAX ? adcgroup + 18 : ADC_GROUP_MAX - 1;
        }
        else if(adcavg > 0x270)
        {
            adcgroup = (adcavg - 0x270) / 0x120; // >270 12, >390 13, >4b0 14, >5d0 15, >6f0 16, >810> 17
            adcgroup = adcgroup + 12;
        }
        else if(adcavg > 0x60) //>60 1, >90 2, >c0 3, >f0 4, >120 5, >150 6, >180 7, >1b0 8, >1e0 9, >210 10, >240 11
        {
            adcgroup = (adcavg - 0x60) / 0x30; 
            adcgroup = adcgroup + 1;
        }
        else //if(adcavg < 0x60) //50~c0 0
        {
            adcgroup = 0;
        }
        //DEBUG_GREEN("adcavg %x group %d last_D %d\n", adcavg, adcgroup, iconflag[UI_MENU_SETIDX_TX_EXPOSURE_LASTADC_D]);
    #endif
    }
  #elif (ADAPTIVE_AE_BY_ADC == 1)
    u16 adcavg;
    u8 adcgroup;

    for(i=0;i<4;i++)
        adcavg += adcGetValue(2);
    adcavg=adcavg>>2;
    adcgroup = (adcavg>>6);
    //DEBUG_CYAN("%x index%d %x %d\n", adcavg, (adcavg>>6), ((adcavg>>6) <<6) , adcavg - ((adcavg>>6) <<6));
  #endif
    i2cWrite_ZN220(0x00,0x03);
	i2cWrite_ZN220(0x2C,0x58);

    if (SIUMODE == SIU_DAY_MODE)
    {
      #if (ADAPTIVE_AE_BY_ADC == 2)
      
        adcavglast = (iconflag[UI_MENU_SETIDX_TX_EXPOSURE_LASTADC_DMB] << 8) + iconflag[UI_MENU_SETIDX_TX_EXPOSURE_LASTADC_DLB];
        if (adcgroup == iconflag[UI_MENU_SETIDX_TX_EXPOSURE_LASTADC_D] ||
            abs(adcavglast - adcavg) < 0x10)
        {
            i2cWrite_ZN220(0x5D,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dt]);
            i2cWrite_ZN220(0x5E,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dh]);
            i2cWrite_ZN220(0x5F,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dm]);
            i2cWrite_ZN220(0x60,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dl]);
            //DEBUG_GREEN("adcavg %x group %d last_ADC %x %x %x\n", adcavg, adcgroup, adcavglast, iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dh], iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dm]);
        }
        else if (iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + adcgroup * 2 + 1 ]!=0 || iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + adcgroup * 2]!=0)
        {
            i2cWrite_ZN220(0x5D,0x0);
            i2cWrite_ZN220(0x5E,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + adcgroup * 2]);
            i2cWrite_ZN220(0x5F,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + adcgroup * 2 + 1]);
            i2cWrite_ZN220(0x60,0x0);
            //DEBUG_GREEN("adcavg %x group %d jump %x %x\n", adcavg, adcgroup, iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + adcgroup * 2], iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + adcgroup * 2 + 1]);
        }
        else
        {
            for (i = 1; i < 5 ; i++)
            {
                if (((adcgroup+i)<ADC_GROUP_MAX) && (iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup+i) * 2 + 1 ]!=0 || iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup+i) * 2]!=0))
                {
                    i2cWrite_ZN220(0x5D,0x0);
                    i2cWrite_ZN220(0x5E,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup+i) * 2]);
                    i2cWrite_ZN220(0x5F,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup+i) * 2 + 1]);
                    i2cWrite_ZN220(0x60,0x0);
                    //DEBUG_GREEN("adcavg %x group %d +%d %x %x\n", adcavg, adcgroup, i, iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup+i) * 2], iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup+i) * 2 + 1]);
                    break;
                }
                else if ((adcgroup>=i) && (iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup-i) * 2 + 1 ]!=0 || iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup-i) * 2]!=0))
                {
                    i2cWrite_ZN220(0x5D,0x0);
                    i2cWrite_ZN220(0x5E,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup-i) * 2]);
                    i2cWrite_ZN220(0x5F,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup-i) * 2 + 1]);
                    i2cWrite_ZN220(0x60,0x0);
                    //DEBUG_GREEN("adcavg %x group %d -%d %x %x\n", adcavg, adcgroup, i, iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup-i) * 2], iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + (adcgroup-i) * 2 + 1]);
                    break;
                }
                else if (i == 4)
                {
                    //i2cWrite_ZN220(0x00,0x03); //really??
                    //i2cWrite_ZN220(0x2C,0x18);
                    //DEBUG_GREEN("adcavg %x group %d auto\n",adcavg, adcgroup);
                }
            }
        }

      #elif (ADAPTIVE_AE_BY_ADC == 1)
        if (adcgroup != iconflag[UI_MENU_SETIDX_TX_EXPOSURE_LASTADC_D])
        {
            if (adcgroup > 15) //ADC 0~0x3C0, 15 level, interval 64
                adcgroup = 15;
            i2cWrite_ZN220(0x5E,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + adcgroup * 2]);
            i2cWrite_ZN220(0x5F,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + adcgroup * 2 + 1]);
            //DEBUG_GREEN("%x index%d %x %x\n", adcavg, adcgroup, iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + adcgroup * 2], iconflag[UI_MENU_SETIDX_TX_EXPOSURE_0h + adcgroup * 2 + 1]);
        }
        else
        {
            i2cWrite_ZN220(0x5D,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dt]);
            i2cWrite_ZN220(0x5E,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dh]);
            i2cWrite_ZN220(0x5F,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dm]);
            i2cWrite_ZN220(0x60,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dl]);
            //DEBUG_GREEN("%x %x %x %x\n", iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dt], iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dh], iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dm], iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Dl]);
        }
      #endif
    }
    else
    {
      #if (ADAPTIVE_AE_BY_ADC == 2) //MWL613 is too dark no need to seprate interval

      #elif (ADAPTIVE_AE_BY_ADC == 1)
        //if (adcgroup != iconflag[UI_MENU_SETIDX_TX_EXPOSURE_LASTADC_N]) // haven't implemented
      #endif
        {
            i2cWrite_ZN220(0x5D,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nt]);
            i2cWrite_ZN220(0x5E,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nh]);
            i2cWrite_ZN220(0x5F,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nm]);
            i2cWrite_ZN220(0x60,iconflag[UI_MENU_SETIDX_TX_EXPOSURE_Nl]);
        }
    }

    //need to 4frame delay 33ms *4 = 132ms
    OSTimeDly(4);

    //i2cWrite_ZN220(0x00,0x03);
    //i2cWrite_ZN220(0x2C,0x18);
    #endif
}
#if (HW_BOARD_OPTION  == MR9100_TX_RDI_CA811)

s32 ZN220_1080P_10fps(void)
{
    int i;
    u8 data;
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD)
    #else
    DEBUG_SIU("ZN220_1080P_10fps\n");
    #endif
    i2cWrite_ZN220(0x00,0x00);
    i2cWrite_ZN220(0xD9,0xA8);
    i2cWrite_ZN220(0xDA,0xDF); //26.6666667Mhz to 70.37
    
    i2cWrite_ZN220(0xD7,0x7C);  //ramp
    
    //10 fps setting
    i2cWrite_ZN220(0x00,0x00);
    i2cWrite_ZN220(0x06,0x0C);
    i2cWrite_ZN220(0x07,0x36);  
    i2cWrite_ZN220(0x08,0x04);
    i2cWrite_ZN220(0x09,0x64);  //10fps
    
    //flicker
    i2cWrite_ZN220(0x00,0x03);
    
    i2cWrite_ZN220(0x91,0x00);
    i2cWrite_ZN220(0x92,0x5D);
    i2cWrite_ZN220(0x93,0x00);
    i2cWrite_ZN220(0x94,0x00);
    i2cWrite_ZN220(0x95,0x70);
    i2cWrite_ZN220(0x96,0x00);
    
    if (iconflag[UI_MENU_SETIDX_TX_FLICKER])
        i2cWrite_ZN220(0x97,0xA3);  //flicker 50Hz
    else
        i2cWrite_ZN220(0x97,0xA1);  //flicker 60Hz
    
    i2cWrite_ZN220(0x00,0x03);
    i2cWrite_ZN220(0x3E,0x10);  //EPC1 //0c
    i2cWrite_ZN220(0x3F,0x10);  //EPC2
    
    i2cWrite_ZN220(0x00,0x04);
    i2cWrite_ZN220(0x65,0x08);  //EPC   
    
    //scaling
    i2cWrite_ZN220(0x00,0x01);
    i2cWrite_ZN220(0x06,0x81);  //scaling = off
    i2cWrite_ZN220(0x0F,0x00);  //Sram
    
    i2cWrite_ZN220(0x00,0x01);
    i2cWrite_ZN220(0x04,0x00);
    i2cWrite_ZN220(0x14,0x02);
    i2cWrite_ZN220(0x15,0x00);
    i2cWrite_ZN220(0x16,0x02);
    i2cWrite_ZN220(0x17,0x00);
    i2cWrite_ZN220(0x18,0x00);
    i2cWrite_ZN220(0x19,0x04);
    
    i2cWrite_ZN220(0x20,0x00);
    i2cWrite_ZN220(0x21,0x00);
    i2cWrite_ZN220(0x22,0x07);
    i2cWrite_ZN220(0x23,0x7F);
    i2cWrite_ZN220(0x24,0x00);
    i2cWrite_ZN220(0x25,0x00);
    i2cWrite_ZN220(0x26,0x04);
    i2cWrite_ZN220(0x27,0x37);
    
    i2cWrite_ZN220(0x1A,0x00);
    i2cWrite_ZN220(0x1B,0x19);
    i2cWrite_ZN220(0x1C,0x04);
    i2cWrite_ZN220(0x1D,0x50);
}

s32 ZN220_720P_15fps(void)
{
    int i;
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD)
    #else
    DEBUG_SIU("ZN220_720P_15fps\n");
    #endif

    i2cWrite_ZN220(0x00,0x00);
    i2cWrite_ZN220(0xD9,0xA8);  //26.6666667Mhz to 84.44Mhz
    i2cWrite_ZN220(0xDA,0xF2);
    
    i2cWrite_ZN220(0xD7,0x7C);  //ramp
    
    if (iconflag[UI_MENU_SETIDX_TX_FLICKER]) //flicker 50Hz
    {
        //16.6666667 fps setting
        i2cWrite_ZN220(0x00,0x00);
        i2cWrite_ZN220(0x06,0x08);
        i2cWrite_ZN220(0x07,0xCA); //2251   
        i2cWrite_ZN220(0x08,0x04);
        i2cWrite_ZN220(0x09,0x64);  //16.66666667fps frame height.
        
        //flicker
        i2cWrite_ZN220(0x00,0x03);
        
        i2cWrite_ZN220(0x91,0x00);
        i2cWrite_ZN220(0x92,0x9C);
        i2cWrite_ZN220(0x93,0x00);
        i2cWrite_ZN220(0x94,0x00);
        i2cWrite_ZN220(0x95,0xBB);
        i2cWrite_ZN220(0x96,0x00);
        
        i2cWrite_ZN220(0x97,0xA3);  //flicker 50Hz
    }
    else
    {
        //15 fps setting
        i2cWrite_ZN220(0x00,0x00);
        i2cWrite_ZN220(0x06,0x09);
        i2cWrite_ZN220(0x07,0xC4); //2501   
        i2cWrite_ZN220(0x08,0x04);
        i2cWrite_ZN220(0x09,0x64);  //15ps frame height.
        
        //flicker
        i2cWrite_ZN220(0x00,0x03);
        
        i2cWrite_ZN220(0x91,0x00);
        i2cWrite_ZN220(0x92,0x8C);
        i2cWrite_ZN220(0x93,0x00);
        i2cWrite_ZN220(0x94,0x00);
        i2cWrite_ZN220(0x95,0xA8);
        i2cWrite_ZN220(0x96,0x00);
        
        i2cWrite_ZN220(0x97,0xA1);  //flicker 60Hz
    }
    i2cWrite_ZN220(0x00,0x03);
    i2cWrite_ZN220(0x3E,0x0C);  //EPC1
    i2cWrite_ZN220(0x3F,0x0C);  //EPC2
    
    i2cWrite_ZN220(0x00,0x04);
    i2cWrite_ZN220(0x65,0x08);  //EPC         
    
    //scaling
    i2cWrite_ZN220(0x00,0x01);
    i2cWrite_ZN220(0x06,0x83);  //scaling = on
    i2cWrite_ZN220(0x0F,0x00);  //Sram
    
    i2cWrite_ZN220(0x00,0x01);
    i2cWrite_ZN220(0x04,0x00);
    i2cWrite_ZN220(0x14,0x03);
    i2cWrite_ZN220(0x15,0x00);
    i2cWrite_ZN220(0x16,0x03);
    i2cWrite_ZN220(0x17,0x00);
    i2cWrite_ZN220(0x18,0x03);
    i2cWrite_ZN220(0x19,0x5A);
    
    i2cWrite_ZN220(0x20,0x00);
    i2cWrite_ZN220(0x21,0x00);
    i2cWrite_ZN220(0x22,0x04);
    i2cWrite_ZN220(0x23,0xFF);
    i2cWrite_ZN220(0x24,0x00);
    i2cWrite_ZN220(0x25,0x00);
    i2cWrite_ZN220(0x26,0x02);
    i2cWrite_ZN220(0x27,0xCF);
    
    i2cWrite_ZN220(0x1A,0x00);
    i2cWrite_ZN220(0x1B,0x19);
    i2cWrite_ZN220(0x1C,0x04);
    i2cWrite_ZN220(0x1D,0x50);
}

#else

s32 ZN220_1080P_10fps(void)
{
    int i;
    u8 data;
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD)
    #else
    DEBUG_SIU("ZN220_1080P_10fps\n");
    #endif
    i2cWrite_ZN220(0x00, 0x00);
    i2cWrite_ZN220(0xD9, 0xA9);
    i2cWrite_ZN220(0xDA, 0xE8);

    i2cWrite_ZN220(0xD7, 0x7C);	//ramp

    //10 fps setting
    i2cWrite_ZN220(0x00, 0x00);
    i2cWrite_ZN220(0x06, 0x0C);
    i2cWrite_ZN220(0x07, 0x2F);	
    i2cWrite_ZN220(0x08, 0x04);
    i2cWrite_ZN220(0x09, 0x64);	//10fps

    //flicker
    i2cWrite_ZN220(0x00, 0x03);

    i2cWrite_ZN220(0x91, 0x00);
    i2cWrite_ZN220(0x92, 0x5D);
    i2cWrite_ZN220(0x93, 0x00);
    i2cWrite_ZN220(0x94, 0x00);
    i2cWrite_ZN220(0x95, 0x70);
    i2cWrite_ZN220(0x96, 0x00);

    if (iconflag[UI_MENU_SETIDX_TX_FLICKER])
        i2cWrite_ZN220(0x97, 0xA3);	//flicker 50Hz
    else
        i2cWrite_ZN220(0x97, 0xA1);	//flicker 60Hz

    i2cWrite_ZN220(0x00, 0x03);
    i2cWrite_ZN220(0x3E, 0x10);	//EPC1 //0c
    i2cWrite_ZN220(0x3F, 0x10);	//EPC2

    i2cWrite_ZN220(0x00, 0x04);
    i2cWrite_ZN220(0x65, 0x0D);	//EPC         


    //rurururu2_2018.07.25
    //scaling
    i2cWrite_ZN220(0x00, 0x01);
    i2cWrite_ZN220(0x06, 0x83);	//scaling = on
    i2cWrite_ZN220(0x0F, 0x00);	//Sram

    i2cWrite_ZN220(0x00, 0x01);
    i2cWrite_ZN220(0x04, 0x00);
    i2cWrite_ZN220(0x14, 0x02);
    i2cWrite_ZN220(0x15, 0x00);
    i2cWrite_ZN220(0x16, 0x02);
    i2cWrite_ZN220(0x17, 0x00);
    i2cWrite_ZN220(0x18, 0x00);
    i2cWrite_ZN220(0x19, 0x04);

    i2cWrite_ZN220(0x20, 0x00);
    i2cWrite_ZN220(0x21, 0x00);
    i2cWrite_ZN220(0x22, 0x07);
    i2cWrite_ZN220(0x23, 0x7F);
    i2cWrite_ZN220(0x24, 0x00);
    i2cWrite_ZN220(0x25, 0x00);
    i2cWrite_ZN220(0x26, 0x04);
    i2cWrite_ZN220(0x27, 0x37);

    i2cWrite_ZN220(0x1A, 0x00);
    i2cWrite_ZN220(0x1B, 0x19);
    i2cWrite_ZN220(0x1C, 0x04);
    i2cWrite_ZN220(0x1D, 0x50);
 
}

s32 ZN220_720P_15fps(void)
{
    int i;
    #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
    #else
    DEBUG_SIU("ZN220_720P_15fps\n");
    #endif

    i2cWrite_ZN220(0x00, 0x00);
    i2cWrite_ZN220(0xD9, 0xA9);    //sysclk=27M, pclk84.375
    i2cWrite_ZN220(0xDA, 0xFD);

    i2cWrite_ZN220(0xD7, 0x7C);    //ramp

    if (iconflag[UI_MENU_SETIDX_TX_FLICKER]) //flicker 50Hz
    {
        //15 fps setting
        i2cWrite_ZN220(0x00, 0x00);
        i2cWrite_ZN220(0x06, 0x08);
        i2cWrite_ZN220(0x07, 0xC9);
        i2cWrite_ZN220(0x08, 0x04);
        i2cWrite_ZN220(0x09, 0x64);     //15fps frame height.
        //flicker
        i2cWrite_ZN220(0x00, 0x03);
        i2cWrite_ZN220(0x91, 0x00);
        i2cWrite_ZN220(0x92, 0x9C);
        i2cWrite_ZN220(0x93, 0x00);
        i2cWrite_ZN220(0x94, 0x00);
        i2cWrite_ZN220(0x95, 0xBB);
        i2cWrite_ZN220(0x96, 0x00);

        i2cWrite_ZN220(0x97, 0xA3);	//flicker 50Hz
    }
    else
    {
        //15 fps setting
        i2cWrite_ZN220(0x00, 0x00);
        i2cWrite_ZN220(0x06, 0x09);
        i2cWrite_ZN220(0x07, 0xC3);     
        i2cWrite_ZN220(0x08, 0x04);
        i2cWrite_ZN220(0x09, 0x64);     //15fps frame height.
        //flicker
        i2cWrite_ZN220(0x00, 0x03);
        i2cWrite_ZN220(0x91, 0x00);
        i2cWrite_ZN220(0x92, 0x8C);
        i2cWrite_ZN220(0x93, 0x00);
        i2cWrite_ZN220(0x94, 0x00);
        i2cWrite_ZN220(0x95, 0xA8);
        i2cWrite_ZN220(0x96, 0x00);

        i2cWrite_ZN220(0x97, 0xA1);	//flicker 60Hz
    }

    i2cWrite_ZN220(0x00, 0x03);
    i2cWrite_ZN220(0x3E, 0x10);	//EPC1
    i2cWrite_ZN220(0x3F, 0x10);	//EPC2

    i2cWrite_ZN220(0x00, 0x04);
    i2cWrite_ZN220(0x65, 0x0C);	//EPC         

    //scaling
    i2cWrite_ZN220(0x00, 0x01);
    i2cWrite_ZN220(0x06, 0x83);	//scaling = on
    i2cWrite_ZN220(0x0F, 0x00);	//Sram

    i2cWrite_ZN220(0x00, 0x01);
    i2cWrite_ZN220(0x04, 0x00);
    i2cWrite_ZN220(0x14, 0x03);
    i2cWrite_ZN220(0x15, 0x00);
    i2cWrite_ZN220(0x16, 0x03);
    i2cWrite_ZN220(0x17, 0x00);
    i2cWrite_ZN220(0x18, 0x03);
    i2cWrite_ZN220(0x19, 0x5A);

    i2cWrite_ZN220(0x20, 0x00);
    i2cWrite_ZN220(0x21, 0x00);
    i2cWrite_ZN220(0x22, 0x04);
    i2cWrite_ZN220(0x23, 0xFF);
    i2cWrite_ZN220(0x24, 0x00);
    i2cWrite_ZN220(0x25, 0x00);
    i2cWrite_ZN220(0x26, 0x02);
    i2cWrite_ZN220(0x27, 0xCF);

    i2cWrite_ZN220(0x1A, 0x00);
    i2cWrite_ZN220(0x1B, 0x1A);
    i2cWrite_ZN220(0x1C, 0x04);
    i2cWrite_ZN220(0x1D, 0x51);

}
#endif
s32 siuSetSensorMirrorFilp(u8 mirror, u8 filp)
{
    u8 data = mirror << 7 + filp << 6;
    i2cWrite_ZN220(0x0, 0x0);
    i2cWrite_ZN220(0x18, data);
}
/*

Routine Description:

	Initialize the Sensor Register.

Arguments:

	None.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSensorInit(u8 siuSensorMode,u8 zoomFactor)
{
    int FrameRate=30;
    u8 err;
    u16 data =0;
    static u8 justboot = 0;
    //----------//
    
#if(FPGA_BOARD_A1018_SERIES)
    //---------Run on Mclk=32 MHz--------------//    
    #if(SYS_CPU_CLK_FREQ == 32000000)
	  SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x00; //MClk=32/1=32MHz
	#elif(SYS_CPU_CLK_FREQ == 48000000)
      SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x01; //MClk=48/2=24MHz
	#endif    
#else   
    #if 0
       //---------Run on Mclk=24 MHz--------------//
       #if(SYS_CPU_CLK_FREQ == 96000000)
        SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x0b; //MClk=288/12=24MHz
       #elif(SYS_CPU_CLK_FREQ == 108000000)
        SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x08; //MClk=216/9=24MHz
       #elif( (SYS_CPU_CLK_FREQ == 160000000) || (SYS_CPU_CLK_FREQ == 162000000) )
        SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x13; //MClk=480/20=24MHz 
        //SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x27; //MClk=480/40=12MHz 
        //SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x3f; //MClk=480/64=7.5MHz 
       #elif(SYS_CPU_CLK_FREQ == 180000000)
        SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x13; //MClk=540/20=24MHz  
       #endif
    #else
       //---------Run on Mclk=27 MHz--------------//
       #if(SYS_CPU_CLK_FREQ == 160000000)
        #if REDUCE_HCLK_TO_128M
        SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x0d; //MClk=384/14=27.428MHz 
        #elif REDUCE_HCLK_TO_144M
        SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x0f; //MClk=432/16=27MHz 
        #else
        SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x11; //MClk=480/18=26.666MHz 
        #endif


       #elif(SYS_CPU_CLK_FREQ == 180000000)
        SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x13; //MClk=540/20=27MHz  
       #endif
    #endif
      
#endif

#if RFIU_SUPPORT 
   gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_FHD;
#endif

    switch(siuSensorMode)   	
    {	
        case SIUMODE_PREVIEW: 
        case SIUMODE_MPEGAVI: 

            if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896) )
            {
                if (justboot == 0)
                {
                    ZN220_Init_1080P();	//refer CA811 setting
                    siuSetSensorDayNight(SIUMODE);	//refer CA811 setting
                    ZN220_BLCSpeed(0x00);	//blc iir fast
                    ZN220_SoftReset(); //avoid AE/AWB no work
                    ZN220_IRLedSwitch(); //do nothing for RDI
                    ZN220_Manual_AEC(); //do nothing for RDI
                    ZN220_1080P_10fps(); //refer CA811 setting
                    ZN220_AECAWBOn(); //avoid AE/AWB no work
                    ZN220_Init_1080P_2(); //refer CA811 setting
                    FrameRate   = 10;
                    justboot = 1;
                }
                else
                { // do here when change resolution
                    ZN220_BLCSpeed(0x00);	//blc iir fast
                    ZN220_1080P_10fps();
                    FrameRate   = 10;
                    OSTimeDly(7);	//320msec
                    ZN220_BLCSpeed(0x3F);	//blc iir slow
                }

            }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) )
            {
                if (justboot == 0)
                {
                    ZN220_Init_1080P();
                    siuSetSensorDayNight(SIUMODE);
                    ZN220_BLCSpeed(0x00);	//blc iir fast
                    ZN220_SoftReset();
                    ZN220_IRLedSwitch();
                    ZN220_Manual_AEC();
                    ZN220_720P_15fps();
                    ZN220_AECAWBOn();
                    ZN220_Init_1080P_2();
                    FrameRate   = 16;
                    justboot = 1;
                    
                }
                else
                {
                    ZN220_BLCSpeed(0x00);	//blc iir fast
                    ZN220_720P_15fps();
                    FrameRate   = 16;
                    OSTimeDly(7);	//320msec
                    ZN220_BLCSpeed(0x3F);	//blc iir slow
                }
            }
			else
		    {
                ZN220_Init_1080P();
                siuSetSensorDayNight(SIUMODE);
                //rurururu2_2018.07.25
                ZN220_SoftReset();
                ZN220_IRLedSwitch();
                ZN220_Manual_AEC();
                ZN220_720P_15fps();
                ZN220_AECAWBOn();
                ZN220_Init_1080P_2();
                FrameRate   = 15;
                justboot = 1;
			}
    
            #if(SW_APPLICATION_OPTION == MR9120_BATCAM_TX5_MUTISTREAM_FWUPD || (SW_APPLICATION_OPTION == MR9120_DOORCAM_TX5_MUTISTREAM_FWUPD))
            #else
            DEBUG_SIU("UI_MENU_SETIDX_TX_FLICKER %d\n", iconflag[UI_MENU_SETIDX_TX_FLICKER]);
            #endif
            siuSetImage();

            #if (SENSOR_ROW_COL_MIRROR == 1)
            siuSetSensorMirrorFilp(1, 1);
            #endif
            if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072))
            {
                IsuScUpFIFODepth = 0x1d1d1111; //把MP的scalling的頻寬分給SP //避畫面抖一下
            }
            else
            {
                IsuScUpFIFODepth = 0x1d1d1111;
            }

            break;
        //-------------------//
        case SIUMODE_CAPTURE: 
           
            break;
        //-------------------//
        case SIUMODE_PREVIEW_ZOOM:
        case SIUMODE_MPEGAVI_ZOOM:
            

            break;
    }
    //----------------//
    /*
        Lucian:	當為VideoClip Zoom 模式時, 因錄影還在進行中,不可reset Mpeg control variable.
    */
    if(siuSensorMode != SIUMODE_MPEGAVI_ZOOM)
    {
    	mp4_avifrmcnt = 0;
    	isu_avifrmcnt = 0;  //0;
    	avi_FrameSkip = 0;	
        unMp4SkipFrameCount = 0; /* mh@2006/11/22: for time stamp control */
    }

    IsSensorInit    = 1;

	return FrameRate;
}

void ZN220_SoftReset(void){
    i2cWrite_ZN220(0x00, 0x00);
    i2cWrite_ZN220(0x0C, 0x01); //soft_reset
    i2cWrite_ZN220(0xB5, 0x01); //trigger

}
void ZN220_AECAWBOn(void){
    i2cWrite_ZN220(0x00,0x03);
    i2cWrite_ZN220(0x2C,0x18);
    i2cWrite_ZN220(0x00,0x04);
    i2cWrite_ZN220(0x41,0xE6);
}
void ZN220_BLCSpeed(u8 Level){
    i2cWrite_ZN220(0x00, 0x00);
    i2cWrite_ZN220(0x38, Level); //0x00 : min(fast), 0x3F : max(slow)
}

void ZN220_IRLedSwitch(void)
{
#if (IRLED_CONTROLL_BY_SENSOR == 1)
    if (SIUMODE == SIU_NIGHT_MODE)
    {

      i2cWrite_ZN220(0x0, 0x0);
      i2cWrite_ZN220(0xE0, 0xA); //driving
      i2cWrite_ZN220(0x0, 0x6);
      i2cWrite_ZN220(0x0F, 0x21); //IR LED on
    }
    else
    {
      i2cWrite_ZN220(0x0, 0x0);
      i2cWrite_ZN220(0xE0, 0xA); //driving
      i2cWrite_ZN220(0x0, 0x6);
      i2cWrite_ZN220(0x0F, 0x1); //IR LED off
    }
#endif
}
#if (HW_BOARD_OPTION  == MR9100_TX_RDI_CA811)
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;
    u32 waitFlag;
    u8  err;

    DayNightMode    = Level;

    if(Level == SIU_NIGHT_MODE)
    {
        DEBUG_SIU("##enter night\n");
        i2cWrite_ZN220(0x00,0x01);
        i2cWrite_ZN220(0x02,0x57);  //lens shading = on
        i2cWrite_ZN220(0x05,0x79);  //BW mode
        
        i2cWrite_ZN220(0xB0,0x00);  //HS_UO
        i2cWrite_ZN220(0xB1,0x00);  //HS_VO
        
        //AE window
        i2cWrite_ZN220(0x00,0x03);
        i2cWrite_ZN220(0x01,0x00);  //AE_WINX_BEGIN
        i2cWrite_ZN220(0x02,0x00);  //AE_WINX_BEGIN
        i2cWrite_ZN220(0x03,0x07);  //AE_WINX_END
        i2cWrite_ZN220(0x04,0x7E);  //AE_WINX_END
        i2cWrite_ZN220(0x05,0x00);  //AE_WINY_BEGIN
        i2cWrite_ZN220(0x06,0x00);  //AE_WINY_BEGIN
        i2cWrite_ZN220(0x07,0x04);  //AE_WINY_END
        i2cWrite_ZN220(0x08,0x37);  //AE_WINY_END
        i2cWrite_ZN220(0x09,0x00);  //AE_PERI_WINX_BEGIN
        i2cWrite_ZN220(0x0A,0x00);  //AE_PERI_WINX_BEGIN
        i2cWrite_ZN220(0x0B,0x07);  //AE_PERI_WINX_END
        i2cWrite_ZN220(0x0C,0x7E);  //AE_PERI_WINX_END
        i2cWrite_ZN220(0x0D,0x00);  //AE_PERI_WINY_BEGIN
        i2cWrite_ZN220(0x0E,0xA0);  //AE_PERI_WINY_BEGIN
        i2cWrite_ZN220(0x0F,0x04);  //AE_PERI_WINY_END
        i2cWrite_ZN220(0x10,0x37);  //AE_PERI_WINY_END
        i2cWrite_ZN220(0x11,0x02);  //AE_CEN_WINX_BEGIN
        i2cWrite_ZN220(0x12,0x00);  //AE_CEN_WINX_BEGIN
        i2cWrite_ZN220(0x13,0x05);  //AE_CEN_WINX_END
        i2cWrite_ZN220(0x14,0x80);  //AE_CEN_WINX_END
        i2cWrite_ZN220(0x15,0x01);  //AE_CEN_WINY_BEGIN
        i2cWrite_ZN220(0x16,0x40);  //AE_CEN_WINY_BEGIN
        i2cWrite_ZN220(0x17,0x04);  //AE_CEN_WINY_END
        i2cWrite_ZN220(0x18,0x37);  //AE_CEN_WINY_END
        
        //ae weight
        i2cWrite_ZN220(0x00,0x03);
        i2cWrite_ZN220(0x24,0x10);  //AE_CEN_WT
        i2cWrite_ZN220(0x25,0x10);  //AE_IN_WT
        i2cWrite_ZN220(0x26,0x11);  //AE_BOT_WT
        i2cWrite_ZN220(0x27,0x11);  //AE_RIG_WT
        
        i2cWrite_ZN220(0x28,0x10);  //SAT_CEN_WT
        i2cWrite_ZN220(0x29,0x10);  //SAT_IN_WT
        i2cWrite_ZN220(0x2A,0x11);  //SAT_BOT_WT
        i2cWrite_ZN220(0x2B,0x11);  //SAT_RIG_WT
        
        //contrast
        i2cWrite_ZN220(0x00,0x01);
        i2cWrite_ZN220(0xED,0x88);  //CONTRA_GAIN1    
        i2cWrite_ZN220(0xEE,0x80);  //CONTRA_GAIN2    
        
        i2cWrite_ZN220(0xF5,0x80);  //CONTREF_BOUND1  
        i2cWrite_ZN220(0xF6,0x80);  //CONTREF_BOUND2  
        i2cWrite_ZN220(0xF7,0x80);  //BRIGHT_BOUND1   
        i2cWrite_ZN220(0xF8,0x80);  //BRIGHT_BOUND2   
        //brightness = bright_bound - contref_bound
        
        
        //sharpness
        i2cWrite_ZN220(0x00,0x01);
        i2cWrite_ZN220(0xC7,0x40);  //EDG_GAIN1      
        i2cWrite_ZN220(0xC8,0x20);  //EDG_GAIN2      
        
        //target
        i2cWrite_ZN220(0x00,0x03);
        i2cWrite_ZN220(0x33,0x55);  //H_Max_Yt
        i2cWrite_ZN220(0x34,0x70);  //L_MAX_Yt
        i2cWrite_ZN220(0x35,0x55);  //H_MIN_Yt
        i2cWrite_ZN220(0x36,0x60);  //L_MIN_Yt
                      
        i2cWrite_ZN220(0x2D,0xFE); // AEC speed for saturtaion                     
        i2cWrite_ZN220(0x00,0x03);            
        i2cWrite_ZN220(0x3E,0x10);  //EPC1
        i2cWrite_ZN220(0x3F,0x10);  //EPC2
        

        i2cWrite_ZN220(0x00,0x02);  //PageC
        i2cWrite_ZN220(0xA0,0x30);  //BF_ALPHA
        i2cWrite_ZN220(0x15,0x30);	//GM_ATTWT2
        
        

#if NIGHT_COLOR_ENA // 彩色不切換 Saturation
#else
#endif
    }
    else
    {
        DEBUG_SIU("##enter day\n");
        i2cWrite_ZN220(0x00,0x01);
        i2cWrite_ZN220(0x02,0x53);	//lens shading = off
        i2cWrite_ZN220(0x05,0xF8);	//color mode

        i2cWrite_ZN220(0xB0,0x03);	//HS_UO
        i2cWrite_ZN220(0xB1,0x02);	//HS_VO

        //color Setting

        //AE window
        i2cWrite_ZN220(0x00,0x03);
        i2cWrite_ZN220(0x01,0x00);	//AE_WINX_BEGIN
        i2cWrite_ZN220(0x02,0x00);	//AE_WINX_BEGIN
        i2cWrite_ZN220(0x03,0x07);	//AE_WINX_END
        i2cWrite_ZN220(0x04,0x7E);	//AE_WINX_END
        i2cWrite_ZN220(0x05,0x00);	//AE_WINY_BEGIN
        i2cWrite_ZN220(0x06,0x00);	//AE_WINY_BEGIN
        i2cWrite_ZN220(0x07,0x04);	//AE_WINY_END
        i2cWrite_ZN220(0x08,0x37);	//AE_WINY_END
        i2cWrite_ZN220(0x09,0x00);	//AE_PERI_WINX_BEGIN
        i2cWrite_ZN220(0x0A,0x00);	//AE_PERI_WINX_BEGIN
        i2cWrite_ZN220(0x0B,0x07);	//AE_PERI_WINX_END
        i2cWrite_ZN220(0x0C,0x7E);	//AE_PERI_WINX_END
        i2cWrite_ZN220(0x0D,0x00);	//AE_PERI_WINY_BEGIN
        i2cWrite_ZN220(0x0E,0xA0);	//AE_PERI_WINY_BEGIN
        i2cWrite_ZN220(0x0F,0x04);	//AE_PERI_WINY_END
        i2cWrite_ZN220(0x10,0x37);	//AE_PERI_WINY_END
        i2cWrite_ZN220(0x11,0x01);	//AE_CEN_WINX_BEGIN
        i2cWrite_ZN220(0x12,0xFB);	//AE_CEN_WINX_BEGIN
        i2cWrite_ZN220(0x13,0x05);	//AE_CEN_WINX_END
        i2cWrite_ZN220(0x14,0x85);	//AE_CEN_WINX_END
        i2cWrite_ZN220(0x15,0x01);	//AE_CEN_WINY_BEGIN
        i2cWrite_ZN220(0x16,0x40);	//AE_CEN_WINY_BEGIN
        i2cWrite_ZN220(0x17,0x04);	//AE_CEN_WINY_END
        i2cWrite_ZN220(0x18,0x38);	//AE_CEN_WINY_END

        //ae weight
        i2cWrite_ZN220(0x00,0x03);
        i2cWrite_ZN220(0x24,0x07);	//AE_CEN_WT
        i2cWrite_ZN220(0x25,0x10);	//AE_IN_WT
        i2cWrite_ZN220(0x26,0x11);	//AE_BOT_WT
        i2cWrite_ZN220(0x27,0x11);	//AE_RIG_WT

        i2cWrite_ZN220(0x28,0x07);	//SAT_CEN_WT
        i2cWrite_ZN220(0x29,0x0A);	//SAT_IN_WT
        i2cWrite_ZN220(0x2A,0x11);	//SAT_BOT_WT
        i2cWrite_ZN220(0x2B,0x11);	//SAT_RIG_WT

        //contrast
        i2cWrite_ZN220(0x00,0x01);
        i2cWrite_ZN220(0xED,0xB0);	//CONTRA_GAIN1    
        i2cWrite_ZN220(0xEE,0x80);	//CONTRA_GAIN2    

        i2cWrite_ZN220(0xF5,0x60);	//CONTREF_BOUND1  
        i2cWrite_ZN220(0xF6,0x70);	//CONTREF_BOUND2  
        i2cWrite_ZN220(0xF7,0x60);	//BRIGHT_BOUND1   
        i2cWrite_ZN220(0xF8,0x80);	//BRIGHT_BOUND2   
        //brightness = bright_bound - contref_bound

        //sharpness
        i2cWrite_ZN220(0x00,0x01);
        i2cWrite_ZN220(0xC7,0xD0);	//EDG_GAIN1      
        i2cWrite_ZN220(0xC8,0x40);	//EDG_GAIN2      

        //target
        i2cWrite_ZN220(0x00,0x03);
        i2cWrite_ZN220(0x33,0x90);	//H_Max_Yt
        i2cWrite_ZN220(0x34,0x70);	//L_MAX_Yt
        i2cWrite_ZN220(0x35,0x60);	//H_MIN_Yt
        i2cWrite_ZN220(0x36,0x40);	//L_MIN_Yt
                      
        i2cWrite_ZN220(0x2D,0xFE); // AEC speed for saturtaion                     
        i2cWrite_ZN220(0x00,0x03);            
        i2cWrite_ZN220(0x3E,0x10);	//EPC1
        i2cWrite_ZN220(0x3F,0x10);	//EPC2

        i2cWrite_ZN220(0x00,0x02);	//PageC
        i2cWrite_ZN220(0xA0,0x50);	//BF_ALPHA
        i2cWrite_ZN220(0x15,0x50);	//GM_ATTWT2

    }
}


#else
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;
    u32 waitFlag;
    u8  err;

    DayNightMode    = Level;

    i2cWrite_SENSOR(0x00, 0x00);
    i2cWrite_SENSOR(0x38, 0x00); //blc_iir fast

    if(Level == SIU_NIGHT_MODE)
    {
        DEBUG_SIU("##enter night\n");

        i2cWrite_ZN220(0x00, 0x01);
        i2cWrite_ZN220(0x05, 0x79);	//BW mode
        
        i2cWrite_ZN220(0xB0, 0x00);	//HS_UO
        i2cWrite_ZN220(0xB1, 0x00);	//HS_VO
        
        i2cWrite_ZN220(0xDF, 0x90);	//OSD_CONTRAST_GAIN
        
        i2cWrite_ZN220(0x00, 0x02);
        i2cWrite_ZN220(0xA0, 0x50);	//BF_ALPHA
        i2cWrite_ZN220(0x15, 0x50);	//GM_ATTWT2
        
        i2cWrite_ZN220(0x00, 0x03);
        i2cWrite_ZN220(0x24, 0x10);	//ae center Weight //max_0x10
        i2cWrite_ZN220(0x25, 0x10); //ae peri weight //max_0x10
        i2cWrite_ZN220(0x26, 0x11); //ae top_bottom_weight
        i2cWrite_ZN220(0x27, 0x11); //ae right_left weight

        i2cWrite_ZN220(0x28, 0x10);	//sat center Weight //max_0x10    
        i2cWrite_ZN220(0x29, 0x10); //sat peri weight //max_0x10      
        i2cWrite_ZN220(0x2A, 0x11); //sat top_bottom_weight
        i2cWrite_ZN220(0x2B, 0x11); //sat right_left weight
        
        i2cWrite_ZN220(0x21, 0xD2); //sat TH
        
        i2cWrite_ZN220(0x00, 0x03);
        i2cWrite_ZN220(0x33, 0x55); //H_Max_Yt
        i2cWrite_ZN220(0x34, 0x70); //L_MAX_Yt
        i2cWrite_ZN220(0x35, 0x55); //H_MIN_Yt
        i2cWrite_ZN220(0x36, 0x60); //L_MIN_Yt
        
        i2cWrite_ZN220(0x37, 0x01); //SAT_TH1
        i2cWrite_ZN220(0x38, 0x03); //SAT_TH2
        i2cWrite_ZN220(0x39, 0x0B); //SAT_TH3
        i2cWrite_ZN220(0x3A, 0x14); //SAT_TH4
        
        //contrast
        i2cWrite_ZN220(0x00, 0x01);
        i2cWrite_ZN220(0xED, 0x88);	//contast_gain1 low_gain_contrast x1(0x80) max 0xFF
        i2cWrite_ZN220(0xEE, 0x80);	//contast_gain2 high_gain_contrast x1(0x80) max 0xFF

        i2cWrite_ZN220(0xF5, 0x80);	//CONTREF_BOUND1  low_gain contref
        i2cWrite_ZN220(0xF6, 0x80);	//CONTREF_BOUND2  high_gain contref
        i2cWrite_ZN220(0xF7, 0x80);	//BRIGHT_BOUND1  low_gain brightness
        i2cWrite_ZN220(0xF8, 0x80);	//BRIGHT_BOUND2  high_gain brightness
        //brightness = bright_bound - contref_bound
        
        //sharpness
        i2cWrite_ZN220(0x00, 0x01);
        i2cWrite_ZN220(0xC7, 0x40);	//EDG_GAIN1  low_gain_sharpness //max_0xFF
        i2cWrite_ZN220(0xC8, 0x20);	//EDG_GAIN2  high_gain_sharpness //max_0xFF
        

#if NIGHT_COLOR_ENA // 彩色不切換 Saturation
#else
#endif
    }
    else
    {
        DEBUG_SIU("##enter day\n");
        
        i2cWrite_ZN220(0x00, 0x01);
        i2cWrite_ZN220(0x05, 0xF8);	//color mode
        
        i2cWrite_ZN220(0xB0, 0x04);	//HS_UO
        i2cWrite_ZN220(0xB1, 0x02);	//HS_VO
        
        i2cWrite_ZN220(0xDF, 0x80);	//OSD_CONTRAST_GAIN
        
        i2cWrite_ZN220(0x00, 0x02);
        i2cWrite_ZN220(0xA0, 0x40);	//BF_ALPHA
        i2cWrite_ZN220(0x15, 0x30);	//GM_ATTWT2
        
        i2cWrite_ZN220(0x00, 0x03);
        i2cWrite_ZN220(0x24, 0x08);	//ae center Weight //max_0x10
        i2cWrite_ZN220(0x25, 0x10); //ae peri weight //max_0x10
        i2cWrite_ZN220(0x26, 0x10); //ae top_bottom_weight
        i2cWrite_ZN220(0x27, 0x00); //ae right_left weight

        i2cWrite_ZN220(0x28, 0x0C);	//sat center Weight //max_0x10    
        i2cWrite_ZN220(0x29, 0x0C); //sat peri weight //max_0x10      
        i2cWrite_ZN220(0x2A, 0x10); //sat top_bottom_weight
        i2cWrite_ZN220(0x2B, 0x00); //sat right_left weight
        
        i2cWrite_ZN220(0x21, 0xAB); //sat TH
        
        i2cWrite_ZN220(0x00, 0x03);
        i2cWrite_ZN220(0x33, 0x80); //H_Max_Yt
        i2cWrite_ZN220(0x34, 0x70); //L_MAX_Yt
        i2cWrite_ZN220(0x35, 0x40); //H_MIN_Yt
        i2cWrite_ZN220(0x36, 0x30); //L_MIN_Yt
        
        i2cWrite_ZN220(0x37, 0x00); //SAT_TH1
        i2cWrite_ZN220(0x38, 0x01); //SAT_TH2
        i2cWrite_ZN220(0x39, 0x06); //SAT_TH3
        i2cWrite_ZN220(0x3A, 0x10); //SAT_TH4
        
        //contrast
        i2cWrite_ZN220(0x00, 0x01);
        i2cWrite_ZN220(0xED, 0x80);	//contast_gain1 low_gain_contrast x1(0x80) max 0xFF
        i2cWrite_ZN220(0xEE, 0x80);	//contast_gain2 high_gain_contrast x1(0x80) max 0xFF

        i2cWrite_ZN220(0xF5, 0x80);	//CONTREF_BOUND1  low_gain contref
        i2cWrite_ZN220(0xF6, 0x80);	//CONTREF_BOUND2  high_gain contref
        i2cWrite_ZN220(0xF7, 0x80);	//BRIGHT_BOUND1  low_gain brightness
        i2cWrite_ZN220(0xF8, 0x80);	//BRIGHT_BOUND2  high_gain brightness
        //brightness = bright_bound - contref_bound
        
        //sharpness
        i2cWrite_ZN220(0x00, 0x01);
        i2cWrite_ZN220(0xC7, 0x60);	//EDG_GAIN1  low_gain_sharpness //max_0xFF
        i2cWrite_ZN220(0xC8, 0x20);	//EDG_GAIN2  high_gain_sharpness //max_0xFF

    }
    if (Main_Init_Ready == 1)
    {
        OSTimeDly(7);	//320msec
        i2cWrite_SENSOR(0x00, 0x00);
        i2cWrite_SENSOR(0x38, 0x3F);	//blc_iir slow
    }
}
#endif

s32 siuAeInit(void)  
{	
	siuSetAETbl();
    siuAdjustAE(AECurSet);
    return 1;
}


/*

Routine Description:

	The FIQ handler of Sensor Interface Unit.

Arguments:

	None.

Return Value:

	None.

*/
void siuIntHandler(void)
{
    u32 Temp;
	u32 intStat;  
	u32 frameAct_Thd;
    	
	intStat = SiuIntStat;
	
	if (intStat & SIU_INT_STAT_FIFO_OVERF)
	{
		/* Raw data FIFO overflow */
		DEBUG_SIU("\nSIU Data Overflow\n");
	}
	
	if (intStat & SIU_INT_STAT_FRAM)
	{
	    //No AE/AWB/AF
    #if MOTIONDETEC_ENA
	    siuFrameCount++;		
        frameAct_Thd = (siuOpMode == SIUMODE_MPEGAVI)?  MD_period_Video : MD_period_Preview;
		if (siuFrameCount > frameAct_Thd)
		{
			/* signal event semaphore */
            OSSemPost(siuSemEvt);
            siuFrameCount=0;
		}
    #else
        siuFrameCount++;	
        frameAct_Thd = (siuOpMode == SIUMODE_MPEGAVI)?  AECNT_VideoCap : AECNT_PREVIEW;
		if (siuFrameCount > frameAct_Thd)
		{
		    if(siuOpMode == SIUMODE_CAPTURE) 
                OSSemPost(siuCapEvt);
            else
                OSSemPost(siuSemEvt);           
            siuFrameCount=0;
			
		}
    #endif		
        if (siuOpMode == SIUMODE_MPEGAVI)
		{
		#if (USE_PROGRESSIVE_SENSOR && ISU_OUT_BY_VSYNC)     

        #else
		   //----------- 做影音同步,產生Audio clock -------------//
		    Temp                = siuFrameNumInMP4;
			siuFrameNumInMP4   += siuSkipFrameRate;
    

            //----------做Frame rate control----------//
			if (
                   ((isu_avifrmcnt - ((s32) avi_FrameSkip) - mp4_avifrmcnt) >= (MAX_VIDEO_FRAME_BUFFER-1)) || 
                   ((Temp & 0xffffffe0) != (siuFrameNumInMP4 & 0xffffffe0))
			   )
			{
            	     		// frame skip
				SiuSensCtrl &= (~SIU_CAPT_ENA);
			    avi_FrameSkip = 1;
			    unMp4SkipFrameCount++; /* mh@2006/11/22: for time stamp control */
			    DEBUG_SIU(".");
			}	
			else 
			{
				SiuSensCtrl |= SIU_CAPT_ENA;
				avi_FrameSkip = 0;
		        unMp4SkipFrameCount = 0;   
			}
         #endif

		}	
	}
	
	if (intStat & SIU_INT_STAT_16LINE)
	{
		/* 16 lines */
	}
	
	if (intStat & SIU_INT_STAT_LINE)
	{
		/* Line */
	}
	
	if (intStat & SIU_INT_STAT_DEF_PIX_FAIL)
	{
		/* Defect pixel failed */
	}
}



/*

Routine Description:

	Set Optical Black.

Arguments:

	comp - The color component (B/Gb/Gr/R).
	val  - The Optical Black value of the color component.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetOpticalBlack(u8 comp, u8 val)
{
	u32 ob;
	
	switch (comp)
	{
		case SIU_COMP_B:
			ob = SiuOb;
			SiuOb = (ob & ~SIU_OB_B_MASK) | (((u32)val) << SIU_OB_B_SHFT); 		
			break;
			
		case SIU_COMP_Gb:
			ob = SiuOb;
			SiuOb = (ob & ~SIU_OB_Gb_MASK) | (((u32)val) << SIU_OB_Gb_SHFT);
			break;
			
		case SIU_COMP_Gr:
			ob = SiuOb;
			SiuOb = (ob & ~SIU_OB_Gr_MASK) | (((u32)val) << SIU_OB_Gr_SHFT);
			break;
			
		case SIU_COMP_R:
			ob = SiuOb;
			SiuOb = (ob & ~SIU_OB_R_MASK) | (((u32)val) << SIU_OB_R_SHFT);
			break;
			
		default:
			/* Not a valid component */
			return 0;
	}			
				
	return 1;	
}

/*

Routine Description:

	Set Digital Gain.

Arguments:

	comp - The color component (B/Gb/Gr/R).
	val  - The Digital Gain value of the color component.

Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetDigitalGain(u8 comp, u16 val)
{
	u32 gain;
	
	switch (comp)
	{
		case SIU_COMP_B:
			gain = SiuBGbGain; 
			SiuBGbGain = (gain & ~SIU_GAIN_B_MASK) | (((u32)val) << SIU_GAIN_B_SHFT);
			break;
			
		case SIU_COMP_Gb:
			gain = SiuBGbGain;
			SiuBGbGain = (gain & ~SIU_GAIN_Gb_MASK) | (((u32)val) << SIU_GAIN_Gb_SHFT);
			break;
			
		case SIU_COMP_Gr:
			gain = SiuRGrGain;
			SiuRGrGain = (gain & ~SIU_GAIN_Gr_MASK) | (((u32)val) << SIU_GAIN_Gr_SHFT);
			break;
			
		case SIU_COMP_R:
			gain = SiuRGrGain;
			SiuRGrGain = (gain & ~SIU_GAIN_R_MASK) | (((u32)val) << SIU_GAIN_R_SHFT);
			break;
			
		default:
			/* Not a valid component */
			return 0;
	}			
				
	return 1;	
}

/*

Routine Description:

	Set Lens Shading.

Arguments:

	pLensShading - The parameters of Lens Shading.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetLensShading(SIU_LENS_SHADING* pLensShading)
{
	SiuLensCornerX2 = (pLensShading->cornerX2) | (((u32)pLensShading->shading_B_Gain)<<23);
	SiuLensCornerY2 = (pLensShading->cornerY2) | (((u32)pLensShading->shading_R_Gain)<<23);
	SiuLensCentOffs = (((u32)pLensShading->centerOffset.x) << SIU_LENS_CENT_X_SHFT) | 
			  (((u32)pLensShading->centerOffset.y) << SIU_LENS_CENT_Y_SHFT);
	SiuLensCompGain	= ((u32)pLensShading->shading_G_Gain) | 
			  pLensShading->rType |
			  pLensShading->scaleSize |
			  pLensShading->outputInvert;
	
	return 1;
}

/*

Routine Description:

	Set RGB Gamma.

Arguments:

	tbl - RGB gamma table.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetRGB_PreGamma(u16* pGammaTable_X,u8* pGammaTable_B,u8 *pGammaTable_R,u8 *pGammaTable_Gr,u8 *pGammaTable_Gb)
{
	return 1;
}



/*

Routine Description:

	Set AE Windows.

Arguments:

	pAeWin - The parameters of AE Windows.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetAeWin(AE_WIN* pAeWin)
{
    u32 aeBlkSum;


	// calculate and set the AE windowing size
    pAeWin->winSizeX = sensor_validsize.imgSize.x / 10;
    pAeWin->winSizeY = sensor_validsize.imgSize.y / 10;
	
	SiuAeWinSize  = (((u32)pAeWin->winSizeX) << SIU_AE_WIN_SIZE_X_SHFT) |
			        (((u32)pAeWin->winSizeY) << SIU_AE_WIN_SIZE_Y_SHFT) ;

    
    // the residue pixel will set in start no.
 	pAeWin->winStart.x = 0;
 	pAeWin->winStart.y = 0;
   
	SiuAeWinStart = (((u32)pAeWin->winStart.x) << SIU_AE_WIN_START_X_SHFT) |
			        (((u32)pAeWin->winStart.y) << SIU_AE_WIN_START_Y_SHFT) ;
			
	// calculate and set the scaling factor for AE windows	
	pAeWin->winSize = ((u32) pAeWin->winSizeX) * ((u32) pAeWin->winSizeY);
	
	aeBlkSum = pAeWin->winSize>>2;
	pAeWin->scaleSize = 0;
	
	//To avoid exceeding 12-bit limit: 2^(8+4)
	while((aeBlkSum > 255) && (pAeWin->scaleSize < 2))
	{
	    pAeWin->scaleSize++;
	    aeBlkSum = aeBlkSum>>2;
	}

	aeWin.histLoBond	= 0x18; /* histLoBond, unit:4 */
	aeWin.histHiBond	= 0xd2; /* histLoBond, unit:4 */			
	// Reg084		
	SiuAeCtrl     = (((u32)pAeWin->histLoBond) << SIU_AE_LO_BOND_SHFT) | 
			        (((u32)pAeWin->histHiBond) << SIU_AE_HI_BOND_SHFT) | 
			        (pAeWin->scaleSize << 16);	
					
	return 1;
}

/*

Routine Description:

	Set AE Window Weight.

Arguments:

	blkWeight - The weight of the window.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetAeBlkWeight(u8 *siuBlkWeight, u16 *siuWeightSum)
{
	volatile unsigned *aeWin = &(SiuAeWin0_1);
	u8 idx;
	
	*siuWeightSum = 0;
	for(idx=0; idx<25; idx+=2)
	{
		*(aeWin + (idx>>1)) = (*(siuBlkWeight+idx) << SIU_AE_WGT_WIN_EVEN_SHFT)
		     		        | (*(siuBlkWeight+idx+1) << SIU_AE_WGT_WIN_ODD_SHFT);
		*siuWeightSum += (*(siuBlkWeight+idx) + *(siuBlkWeight+idx+1));
	}
					       
	return 1;
}	

/*

Routine Description:

	Get AE Window Y sum.

Arguments:

	winNum - The window number.
	pWinYsum - The Ysum of the window.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuGetAeWinYsum(u8 winNum, u16* pWinYsum)
{
	volatile unsigned *aeWin = &(SiuAeWin0_1);
	u32 winRegNum = winNum / 2;
	u32 winOdd = winNum % 2;
	
	if (winOdd)
		*pWinYsum = (*(aeWin + winRegNum) & SIU_AE_SUM_WIN_ODD_MASK) >> SIU_AE_SUM_WIN_ODD_SHFT;	       
	else /* winEven */
		*pWinYsum = (*(aeWin + winRegNum) & SIU_AE_SUM_WIN_EVEN_MASK) >> SIU_AE_SUM_WIN_EVEN_SHFT;
					       
	return 1;
}	
/*

Routine Description:

	Get AE Histogram.

Arguments:

	loc - The location of ring to count.
	hist - The histogram of the ring location.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuGetAeHist(u8 loc, SIU_HIST* pHist)
{
	u32 rBond, gBond, bBond;
	
	switch (loc)
	{
		case SIU_AE_HIST_OUT:
			rBond = SiuAeRBondOut;
			gBond = SiuAeGBondOut;
			bBond = SiuAeBBondOut;
			break;
			
		case SIU_AE_HIST_MID:
			rBond = SiuAeRBondMid;
			gBond = SiuAeGBondMid;
			bBond = SiuAeBBondMid;
			break;
			
		case SIU_AE_HIST_INN:
			rBond = SiuAeRBondInn;
			gBond = SiuAeGBondInn;
			bBond = SiuAeBBondInn;
			break;
			
		default:
			/* Error ring location */
			return 0;
	}					

	pHist->redLoBond   = (u16)((rBond & SIU_AE_LO_BOND_HIST_MASK) >> SIU_AE_LO_BOND_HIST_SHFT);
	pHist->redHiBond   = (u16)((rBond & SIU_AE_HI_BOND_HIST_MASK) >> SIU_AE_HI_BOND_HIST_SHFT);
	pHist->greenLoBond = (u16)((gBond & SIU_AE_LO_BOND_HIST_MASK) >> SIU_AE_LO_BOND_HIST_SHFT);
	pHist->greenHiBond = (u16)((gBond & SIU_AE_HI_BOND_HIST_MASK) >> SIU_AE_HI_BOND_HIST_SHFT);
	pHist->blueLoBond  = (u16)((bBond & SIU_AE_LO_BOND_HIST_MASK) >> SIU_AE_LO_BOND_HIST_SHFT);
	pHist->blueHiBond  = (u16)((bBond & SIU_AE_HI_BOND_HIST_MASK) >> SIU_AE_HI_BOND_HIST_SHFT);
								       
	return 1;
}

	



	
/*

Routine Description:

	Set color tune.

Arguments:

	tune - Color tune.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetColorTune(u8 tune)
{
	switch (tune)
	{	
		case SIU_COLOR_TUNE_SEPIA:
		
			break;
			
		case SIU_COLOR_TUNE_BLACK_AND_WHITE:
		
			break;
			
		case SIU_COLOR_TUNE_NEGATIVE:
		
			break;
			
		case SIU_COLOR_TUNE_SOLARIZE:
		
			break;
				
		case SIU_COLOR_TUNE_NORMAL:
		default:
		
			break;		
	}
	
	/* set color effect */
			
	return 1;
}	


/*

Routine Description:

	Set flash light.

Arguments:

	mode - Mode of flash light.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetFlashLight(u8 mode)
{
	switch (mode)
	{
		case SIU_FLASH_LIGHT_RED_EYE_REDUCTION:
			siuFlashLightMode = SIU_FLASH_LIGHT_ALWAYS_ON;	        //Lucian:防紅眼=Force ON
			break;
			
		case SIU_FLASH_LIGHT_ALWAYS_ON:
			siuFlashLightMode = SIU_FLASH_LIGHT_ALWAYS_ON;
			break;
			
		case SIU_FLASH_LIGHT_ALWAYS_OFF:
			siuFlashLightMode = SIU_FLASH_LIGHT_ALWAYS_OFF;
			break;

		case SIU_FLASH_LIGHT_AUTOMATIC:         //Auto
			siuFlashLightMode = SIU_FLASH_LIGHT_AUTOMATIC;

		default:
			siuFlashLightMode = SIU_FLASH_LIGHT_AUTOMATIC;
			break;
	}
	
	return 1;
}	

/*

Routine Description:

	Set video resolution.

Arguments:

	width - Image width.
	height - Image height.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetVideoResolution(u16 width, u16 height)
{	
    siuVideoWidth   = width;
    siuVideoHeight  = height;

	return 1;
}

/*

Routine Description:

	Preview.

Arguments:

	zoomFactor - Zoom factor.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuPreview(s8 zoomFactor)
{
    siuAeEnable = 0;
    siuOpMode = SIUMODE_PREVIEW;
    siuFrameCount = 0;
    /* SW 0222 S */
	siuPreviewInit(zoomFactor,SIUMODE_PREVIEW);
    /* SW 0222 E */
    siuAeEnable = 1;

	
	
	return 1;
}

s32 siuPreviewZoom(s8 zoomFactor)
{
    siuOpMode = SIUMODE_PREVIEW;
	
	// initialize the siu module
	// reset the SIU module
	//SiuSensCtrl =	SIU_RESET;
		
	// Set sensor control 
	SiuSensCtrl =	
	            SIU_NORMAL |
			    SIU_ENA |    							//enable SIU module
        		SIU_SLAVE |                             //For CMOS sensor
        		SIU_VSYNC_ACT_HI |                      //Vsync active low
        		SIU_HSYNC_ACT_LO |                      //Hsync active low
                SIU_DEF_PIX_DISA |
        		SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
        		SIU_INT_ENA_FRAM |                      //Frame end interrupt enaable
        		SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
        		SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
        		SIU_DST_SRAM |                          //SIU to SRAM
         #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)		/* Sensor data is not shifted 2 bits. */      		
        		SIU_DATA_12b |
         #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit) 	/* Sensor data is shifted 2 bits. */    	
        	  	SIU_DATA_10b | 
         #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)    	/* Sensor data is shifted 4 bits. */    	
        	  	SIU_DATA_8b |         	  	        	  	
         #endif        	  
        		SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
        		SIU_OB_COMP_ENA |                      //OB correct enable 
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
        		SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        		SIU_AE_ENA |                            //AE report enable
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.
        					
    SiuSensCtrl |= SIU_CAPT_ENA;
	
	return 1;
}

/*

Routine Description:

	Capture primary.

Arguments:

	None.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuCapturePrimary(s8 zoomFactor)
{
	u16 flash = EXIF_VAL_FLASH_FIRED_NO | EXIF_VAL_FLASH_STROBE_RETURN_NONE | EXIF_VAL_FLASH_MODE_UNKNOWN | 
			    EXIF_VAL_FLASH_FUNCTION_NONE | EXIF_VAL_FLASH_RED_EYE_NONE;
	u8 err;
	u16	data;

    //=============================//
	RATIONAL fNumber = { 28, 10 };			/* F/2.8 */ 
	RATIONAL apertureValue = { 30, 10 };		/* 3.0 Av */
	RATIONAL exposureTime = { 3, 120 };		/* 3/120 sec */
	RATIONAL shutterSpeedValue = { 53, 10 };	/* 5.3 Tv */
	RATIONAL brightnessValue = { 33, 10 };		/* 3.3 Bv */
	//RATIONAL isoSpeedRatings = { 100, 1 };	/* ISO 100 */
	//RATIONAL speedValue = { 50, 10 };		/* 5 Sv */

	siuOpMode = SIUMODE_CAPTURE;
    siuFrameCount = 0;
	siuAeEnable = 0;	
	
	
	while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
	siuSensorInit(SIUMODE_CAPTURE,zoomFactor);	
	setSensorWinSize(zoomFactor, SIUMODE_CAPTURE);
	
	/* af/ae/awb and capture raw data */
	SiuRawAddr =    (u32)siuRawBuf;	

    	// Reset
	SiuSensCtrl = 0x0001;  
	
	/* Set Digital Gain */		
	SiuSensCtrl =	
	    SIU_NORMAL | 
		SIU_ENA |    					//enable SIU module
		SIU_SLAVE |                     //For CMOS sensor
		SIU_VSYNC_ACT_HI |              //Vsync active low.
		SIU_HSYNC_ACT_LO |              //Hsync active low
        SIU_DEF_PIX_DISA |
		SIU_INT_DISA_FIFO_OVERF |       //FIFO overflow interrupt disable
		SIU_INT_ENA_FRAM |              //Frame end interrupt enable         
		SIU_INT_DISA_8LINE |            //8 lines complete interrupt disable
		SIU_INT_DISA_LINE |             //1 lines complete interrupt disable
		SIU_DST_SDRAM |                 //SIU to SDRAM
    #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)		/* Sensor data is not shifted 2 bits. */      		
        SIU_DATA_12b |
    #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)	/* Sensor data is shifted 2 bits. */    	
        SIU_DATA_10b | 
    #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)    	/* Sensor data is shifted 4 bits. */    	
        SIU_DATA_8b |         	  	        	
    #endif    
		SIU_PIX_CLK_ACT_RIS |           //positive edge latch
        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		SIU_OB_COMP_ENA |              //OB compensation enable
		SIU_LENS_SHAD_DISA |            //Lens shading compensation disable
		SIU_RGB_GAMMA_DISA |            //Pre-gamma disable
		SIU_AE_ENA |		            //AE report disable
	 	SIU_TEST_PATRN_DISA |           //Test pattern disable
		SIU_SINGLE_CAPTURE_DISA;  	
    
    #if SIU_OB_DPIX_CALIB_ENA 
    	siuSetOpticalBlack(SIU_COMP_B, (OB_B)<<2);
    	siuSetOpticalBlack(SIU_COMP_Gb, (OB_Gb)<<2);
    	siuSetOpticalBlack(SIU_COMP_Gr, (OB_Gr)<<2);
    	siuSetOpticalBlack(SIU_COMP_R, (OB_R)<<2);
    #else
        siuSetOpticalBlack(SIU_COMP_B,  SIU_OB_B);
    	siuSetOpticalBlack(SIU_COMP_Gb, SIU_OB_Gb);
    	siuSetOpticalBlack(SIU_COMP_Gr, SIU_OB_Gr);
    	siuSetOpticalBlack(SIU_COMP_R,  SIU_OB_R);
    #endif	

    
	siuSetDigitalGain(SIU_COMP_B,  0x0100); 
	siuSetDigitalGain(SIU_COMP_Gb, 0x0100);
	siuSetDigitalGain(SIU_COMP_Gr, 0x0100);
	siuSetDigitalGain(SIU_COMP_R,  0x0100);
	
	/* Set Lens Shading */
	siuSetLensShading(&lensShading);

	
	
	/* Set RGB Gamma Table */
	siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
	
	siuPreFlashCount = 3;
	siuFrameCount = AECNT_PREVIEW-1; //Lucian: OV7720 沒有Snap mode.要等第二張再取像.
	siuFlashGetYFrameCount = AECNT_PREVIEW + 1;
	siuCaptureFlag = 1;
	siuAeEnable = 1;	
	
	// start image capture
	SiuSensCtrl |= SIU_CAPT_ENA;	
	 			    
	OSSemPend(siuCapEvt,SIU_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_SIU("Error: siuCapEvt is %d.\n", err);
		return 0;
	}
	SiuSensCtrl &= (~SIU_CAPT_ENA);

	siuFlashGetYFrameCount = 0;
	siuFlashGoFrameCount = 0;

	siuFlashLightShoot = 0;
	siuPreFlashCount = 1;

	/* set related EXIF IFD ... */
 #if SD_CARD_DISABLE

 #else
	exifSetFlash(flash);
	exifSetApertureValue(&fNumber, &apertureValue);
	exifSetShutterSpeedValue(&exposureTime, &shutterSpeedValue);
	exifSetBrightnessValue(&brightnessValue);
 #endif		
	return 1;	
}

s32 siuCapture_WBComp(s8 zoomFactor)
{
	
	u8 err;
	u16	data;

    //=============================//
	siuOpMode = SIUMODE_CAPTURE;
    siuFrameCount = 0;
	siuAeEnable = 0;	
		
	while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
	siuSensorInit(SIUMODE_CAPTURE,zoomFactor);	
	setSensorWinSize(zoomFactor, SIUMODE_CAPTURE);
	
	/* af/ae/awb and capture raw data */
	SiuRawAddr =    (u32)siuRawBuf;	

    	// Reset
	SiuSensCtrl = 0x0001;  
	
	/* Set Digital Gain */		
	SiuSensCtrl =	
	    SIU_NORMAL | 
		SIU_ENA |    					//enable SIU module
		SIU_SLAVE |                     //For CMOS sensor
		SIU_VSYNC_ACT_HI |              //Vsync active low.
		SIU_HSYNC_ACT_LO |              //Hsync active low
		SIU_DEF_PIX_DISA |              //Defect corect function disable
		SIU_INT_DISA_FIFO_OVERF |       //FIFO overflow interrupt disable
		SIU_INT_ENA_FRAM |              //Frame end interrupt enable         
		SIU_INT_DISA_8LINE |            //8 lines complete interrupt disable
		SIU_INT_DISA_LINE |             //1 lines complete interrupt disable
		SIU_DST_SDRAM |                 //SIU to SDRAM
   #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)		/* Sensor data is not shifted 2 bits. */      		
        SIU_DATA_12b |
   #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)	/* Sensor data is shifted 2 bits. */    	
        SIU_DATA_10b | 
   #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)    	/* Sensor data is shifted 4 bits. */    	
        SIU_DATA_8b |         	  	        	
   #endif    
		SIU_PIX_CLK_ACT_RIS |           //positive edge latch
        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		SIU_OB_COMP_ENA |              //OB compensation enable
		SIU_LENS_SHAD_DISA |            //Lens shading compensation disable
		SIU_RGB_GAMMA_DISA |            //Pre-gamma disable
		SIU_AE_ENA |		            //AE report disable
	 	SIU_TEST_PATRN_DISA |           //Test pattern disable
		SIU_SINGLE_CAPTURE_DISA;  	
    
    
	
	siuSetOpticalBlack(SIU_COMP_B,  OB_B<<2);
	siuSetOpticalBlack(SIU_COMP_Gb, OB_Gb<<2);
	siuSetOpticalBlack(SIU_COMP_Gr, OB_Gr<<2);
	siuSetOpticalBlack(SIU_COMP_R,  OB_R<<2);

	siuSetDigitalGain(SIU_COMP_B,  0x0100); 
	siuSetDigitalGain(SIU_COMP_Gb, 0x0100);
	siuSetDigitalGain(SIU_COMP_Gr, 0x0100);
	siuSetDigitalGain(SIU_COMP_R,  0x0100);
	
	/* Set Lens Shading */
	siuSetLensShading(&lensShading);

	
	
	/* Set RGB Gamma Table */
	siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
	
	siuPreFlashCount = 3;
	siuFrameCount = AECNT_PREVIEW-1; //Lucian: OV7720 沒有Snap mode.要等第二張再取像.
	siuFlashGetYFrameCount = AECNT_PREVIEW + 1;
	siuCaptureFlag = 1;
	siuAeEnable = 1;	
	
	// start image capture
	SiuSensCtrl |= SIU_CAPT_ENA;	
	 			    
	OSSemPend(siuCapEvt,SIU_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_SIU("Error: siuCapEvt is %d.\n", err);
		return 0;
	}
	SiuSensCtrl &= (~SIU_CAPT_ENA);

	siuFlashGetYFrameCount = 0;
	siuFlashGoFrameCount = 0;

	siuFlashLightShoot = 0;
	siuPreFlashCount = 1;
	return 1;	
}

s32 siuCapture_DarkLightPixelDetect(s8 zoomFactor,u8 SensorMode,u8 OpMode)
{	
	u8 err;
	u16	data;
	Analog_Gain AG;

    //=============================//	
	siuOpMode = SIUMODE_CAPTURE;
    siuFrameCount = 0;
	siuAeEnable = 0;	
	
	
	while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
	siuSensorInit(SensorMode,zoomFactor);	
	setSensorWinSize(zoomFactor, SensorMode);
	
	
	if(OpMode == DETECT_DARK_PIXEL)
	{
	     SetSensorSW(765);  //曝光時間設定 1/20 sec
	     AG.gain=0;       //AGC gain = x1;
	     SetSensor_AnalogGain(AG);
	}
	else if(OpMode == DETECT_LIGHT_PIXEL)
	{
	     //SetSensorSW(1);  //曝光時間設定最小
	     SetSensorSW(510);  //曝光時間設定1/30 (最大)
	     AG.gain=0x70; //不設AG= x1, 是因為要Detect Warm pixel
	     SetSensor_AnalogGain(AG);
	}
	else if(OpMode == DETECT_OPTICAL_BLACK)
	{
	     SetSensorSW(1);  //曝光時間設定最小
	     AG.gain=0;       //AGC gain = x1;
	     SetSensor_AnalogGain(AG); 
	}
	
	
	/* af/ae/awb and capture raw data */
	SiuRawAddr =    (u32)siuRawBuf;	

    	// Reset
	SiuSensCtrl = 0x0001;  
	
	/* Set Digital Gain */		
	SiuSensCtrl =	
	    SIU_NORMAL | 
		SIU_ENA |    					//enable SIU module
		SIU_SLAVE |                     //For CMOS sensor
		SIU_VSYNC_ACT_HI |              //Vsync active low.
		SIU_HSYNC_ACT_LO |              //Hsync active low
		SIU_DEF_PIX_DISA |              //Defect corect function disable
		SIU_INT_DISA_FIFO_OVERF |       //FIFO overflow interrupt disable
		SIU_INT_ENA_FRAM |              //Frame end interrupt enable         
		SIU_INT_DISA_8LINE |            //8 lines complete interrupt disable
		SIU_INT_DISA_LINE |             //1  lines complete interrupt disable
		SIU_DST_SDRAM |                 //SIU to SDRAM
   #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)		/* Sensor data is not shifted 2 bits. */      		
        SIU_DATA_12b |
   #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)	/* Sensor data is shifted 2 bits. */    	
        SIU_DATA_10b | 
   #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)    	/* Sensor data is shifted 4 bits. */    	
        SIU_DATA_8b |         	  	        	
   #endif    
		SIU_PIX_CLK_ACT_RIS |           //positive edge latch
        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		SIU_OB_COMP_ENA |              //OB compensation enable
		SIU_LENS_SHAD_DISA |            //Lens shading compensation disable
		SIU_RGB_GAMMA_DISA |            //Pre-gamma disable
		SIU_AE_ENA |		            //AE report disable
	 	SIU_TEST_PATRN_DISA |           //Test pattern disable
		SIU_SINGLE_CAPTURE_DISA;  	
    
    siuSetOpticalBlack(SIU_COMP_B,  OB_B<<2);
	siuSetOpticalBlack(SIU_COMP_Gb, OB_Gb<<2);
	siuSetOpticalBlack(SIU_COMP_Gr, OB_Gr<<2);
	siuSetOpticalBlack(SIU_COMP_R,  OB_R<<2);

	siuSetDigitalGain(SIU_COMP_B,  0x0100); 
	siuSetDigitalGain(SIU_COMP_Gb, 0x0100);
	siuSetDigitalGain(SIU_COMP_Gr, 0x0100);
	siuSetDigitalGain(SIU_COMP_R,  0x0100);
	
	/* Set Lens Shading */
	siuSetLensShading(&lensShading);

	
	/* Set RGB Gamma Table */
	siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
	
	siuPreFlashCount = 3;
	siuFrameCount = AECNT_PREVIEW-2; //Lucian: OV7720 沒有Snap mode.要等第二張再取像.
	siuFlashGetYFrameCount = AECNT_PREVIEW + 1;
	siuCaptureFlag = 1;
	siuAeEnable = 1;	
	
	// start image capture
	SiuSensCtrl |= SIU_CAPT_ENA;	
	 			    
	OSSemPend(siuCapEvt,SIU_TIMEOUT, &err);
	if (err != OS_NO_ERR)
	{
		DEBUG_SIU("Error: siuCapEvt is %d.\n", err);
		return 0;
	}
	SiuSensCtrl &= (~SIU_CAPT_ENA);

	siuFlashGetYFrameCount = 0;
	siuFlashGoFrameCount = 0;
	siuFlashLightShoot = 0;
	siuPreFlashCount = 1;

	
	return 1;	
}

u16 Cal_LOG2(u16 X)
{
    u16 INT_Log2X;
    u16 Fract_Log2X;
    u16 Y;
    static u8 LogTab[11]={0,1,3,4,5,6,7,8,8,9,10};
    
    INT_Log2X=0;
    Y=10*X;
    while( (X>>1)!=0 )
    {
        INT_Log2X ++;
        Y >>=1;
        X >>=1;
    }
    if(Y<10) Y=10;
    if(Y>20) Y=20;
    Fract_Log2X= LogTab[Y-10];
    
    return (INT_Log2X*10+Fract_Log2X);
    
}


/*BJ 0530 S*/	
/*

Routine Description:

	Preview.

Arguments:

	zoomFactor - Zoom factor.
	
Return Value:

	0 - Failure.
	1 - Success.

*/


s32 siuCaptureVideo(s8 zoomFactor)
{
	siuOpMode = SIUMODE_MPEGAVI;
    siuFrameCount       = 0;
	siuAeEnable = 1;	
    siuFrameNumInMP4    = 0;
    siuSkipFrameRate    = 0;
    siuFrameTime        = 0;    /* Peter 070104 */
    
    

    OSTimeDly(1);    
	siuVideoClipInit(zoomFactor);	
	
	return 1;
}

s32 siuVideoZoom(s8 zoomFactor)
{
	siuOpMode = SIUMODE_MPEGAVI;    
		
	// initialize the siu module
	// reset the SIU module
	//SiuSensCtrl =	SIU_RESET;
    
    SiuSensCtrl =	
	            SIU_NORMAL |
			    SIU_ENA |    							//enable SIU module
        		SIU_SLAVE |                             //For CMOS sensor
        		SIU_VSYNC_ACT_HI |                      //Vsync active low
        		SIU_HSYNC_ACT_LO |                      //Hsync active low
                SIU_DEF_PIX_DISA |
        		SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
        		SIU_INT_ENA_FRAM |                      //Frame end interrupt enaable
        		SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
        		SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
        		SIU_DST_SRAM |                          //SIU to SRAM
         #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)		/* Sensor data is not shifted 2 bits. */      		
        		SIU_DATA_12b |
         #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit) 	/* Sensor data is shifted 2 bits. */    	
        	  	SIU_DATA_10b | 
         #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)    	/* Sensor data is shifted 4 bits. */    	
        	  	SIU_DATA_8b |         	  	        	  	
         #endif        	  
        		SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
        		SIU_OB_COMP_ENA |                      //OB correct enable 
        		SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
        		SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
        		SIU_AE_ENA |                            //AE report enable
        		SIU_TEST_PATRN_DISA |                   //Test pattern disable
        		SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.	
    SiuSensCtrl |= SIU_CAPT_ENA;
	
	return 1;
}
/*BJ 0530 E*/	

/*

Routine Description:

	Resume task for AF, AE, and AWB.

Arguments:

	None.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuResumeTask(void)
{
	/* Resume the task */
	
	return 1;
}

/*

Routine Description:

	Suspend task for AF, AE, and AWB.

Arguments:

	None.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSuspendTask(void)
{
	/* Suspend the task */

	//OSTaskSuspend(SIU_TASK_PRIORITY);
	
	return 1;
}

/*

Routine Description:

	The SIU task.

Arguments:

	pData - The task parameter.

Return Value:

	None.

*/
// Always keep the camera in Preview mode 
void siuTask(void* pData)
{
	u8 err;
    u8 level            = 0;
    u32 i;
    u8 pulse_time=250;   //每輸出250次Hight Level 大約 花0.5ms
    static int  Mode    = -1;
    u8 mdset;
    
	while (1)
	{
		OSSemPend(siuSemEvt, OS_IPC_WAIT_FOREVER, &err);

		if (err != OS_NO_ERR)
		{
			DEBUG_SIU("Error: siuSemEvt is %d.\n", err);
			//return ;
		}

	}
}


s32 siuGetOutputSize(u16 *height_size, u16 *width_size)
{
	*height_size = sensor_validsize.imgSize.y;
	*width_size = sensor_validsize.imgSize.x;	
	return 1; 	
}

/*

Routine Description:

	Auto-focus.

Arguments:

	None.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuAutoFocus(void)
{	
	
	return 1;
}




/* SW */
/*

Routine Description:

	Get AE Y average value.

Arguments:

	None.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuGetAeYavg(u8 *siuBlkWeight)
{
	s32 ae_WeightedYsum;
	u16 ae_WinSize;
	u8 i,idx;
	
	// Get Y sum value of 25 AE Windows  
	ae_WeightedYsum = ((s32) SiuAeWgtSum * 256); //weight sum 一個單位為256 

	// Get AE windows
	if (aeWin.scaleSize == 0)
		ae_WinSize = (aeWin.winSize)>>2;
	else if (aeWin.scaleSize == 1)
		ae_WinSize = (aeWin.winSize)>>4;
	else if (aeWin.scaleSize == 2)
		ae_WinSize = (aeWin.winSize)>>6;
	else
	{
	    ae_WinSize = 1; //avoid to div zero.
		DEBUG_SIU("scale factor illegal\n");
	}

	ae_WeightedYsum = ae_WeightedYsum / ae_WinSize;

	ae_YavgValue = ae_WeightedYsum / aeWin.weightacc;
					       
	return 1;
}


s32 siuSetAwbWin()
{

#if 1 // After Fine Tune @Lucian 20080729
    //AWB window-size config: 設定full size(640x480),4 pixel/unit.
	SiuAwbWinStart	= 0x00000000; //start point=(0,0)
	SiuAwbWinSize	= 0x007800A0; //Width=160x4, Height=120x4;
              
	SiuAwbCtrl	= 0x03300001;    // scale=1: v:2,h:2,四點取一點.
        		 
	SiuAwbThresh	= 0x0fe640f0; //Y_MAX_THR=230
	                              //Y_MIN_THR=15
	                              //Saturation High bond=80
	                              //RGB High boud=240
	                              
	
	SiuAwbGain1	= 0x00005080; //128 表示為1.
	SiuAwbGain2	= 0x0000a6ff; 
#else //Gray World
	SiuAwbWinStart	= 0x00000000;
	SiuAwbWinSize	= 0x007800A0;
              
	SiuAwbCtrl	= 0xFFFF0001;
        		 
	SiuAwbThresh	= 0x32C8FFFF;
	SiuAwbGain1	= 0x00008080;
	SiuAwbGain2	= 0x00008080;
#endif
				
	return 1;
}

/* This function will be removed when UI can send the resolution information*/
/*
   This function set the resoultion of SIU, IPU, ISU, and JPEG
   Because the size of these modules are depended on Sensor
   THis setting are based on 2M image sensor
*/

s32 setSensorWinSize(s8 zoomFactor, s8 siuSensorMode)
{
	u8 zoomScale; 
    
    u32 Img_Width;
    u32 Img_Height;    
        
    uiDigZoomSetFlag = 1;
    switch (siuSensorMode)
	{
	    case SIUMODE_CAPTURE: //Capture mode, 目前無法使用        
	    case SIUMODE_PREVIEW: //Preview mode	         		
        case SIUMODE_MPEGAVI: //Video clip mode: 不支援Zoom-in/Out
            if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) )
            {
	            Img_Width   = 1280;
	            Img_Height  = 720;
	                        
	            sensor_validsize.imgSize.x  = Img_Width * 2;
	            sensor_validsize.imgSize.y  = Img_Height;
	            sensor_validsize.imgStr.x   = 0;
	            sensor_validsize.imgStr.y   = 0;

	            SiuValidSize =  (sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
	                            (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);  
	        

	            SiuValidStart = (sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
	                            (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
	            
	            ipuSetIOSize(Img_Width, Img_Height);
	         	isuSetIOSize(Img_Width, Img_Height); 
            }
			else if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_320x240))
			{
	            Img_Width   = 640;
	            Img_Height  = 480;
	                        
	            sensor_validsize.imgSize.x  = Img_Width * 2;
	            sensor_validsize.imgSize.y  = Img_Height;
	            sensor_validsize.imgStr.x   = 0;
	            sensor_validsize.imgStr.y   = 0;

	            SiuValidSize =  (sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
	                            (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);  
	        

	            SiuValidStart = (sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
	                            (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
	            
	            ipuSetIOSize(Img_Width, Img_Height);
	         	isuSetIOSize(Img_Width, Img_Height); 
		    }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1920x1072) )
            {
	            Img_Width   = 1920;
	            Img_Height  = 1080;
	                        
	            sensor_validsize.imgSize.x  = Img_Width * 2;
	            sensor_validsize.imgSize.y  = Img_Height;
	            sensor_validsize.imgStr.x   = 0;
	            sensor_validsize.imgStr.y   = 0;

	            SiuValidSize =  (sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
	                            (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);  
	        

	            SiuValidStart = (sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
	                            (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
	            
	            ipuSetIOSize(Img_Width, Img_Height);
	         	isuSetIOSize(Img_Width, Img_Height); 
            }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1600x896) )
            {
	            Img_Width   = 1600;
	            Img_Height  = 896;
	                        
	            sensor_validsize.imgSize.x  = Img_Width * 2;
	            sensor_validsize.imgSize.y  = Img_Height;
	            sensor_validsize.imgStr.x   = (1920-1600)/2;
	            sensor_validsize.imgStr.y   = (1080-896)/2;

	            SiuValidSize =  (sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
	                            (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);  
	        

	            SiuValidStart = (sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
	                            (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
	            
	            ipuSetIOSize(Img_Width, Img_Height);
	         	isuSetIOSize(Img_Width, Img_Height); 
            }
            else
            {
	          #if REC_D1_MODE
	            Img_Width =704; 
	            Img_Height=480;
	          #else
	            Img_Width =640; 
	            Img_Height=480;
	          #endif
	    		sensor_validsize.imgSize.x = Img_Width*2;
	    		sensor_validsize.imgSize.y = Img_Height;
	            sensor_validsize.imgStr.x = 0;// 134
	    		sensor_validsize.imgStr.y = 0;	       
	    		
	    		SiuValidSize = 	(sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
				                (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);	
	        
	        	SiuValidStart =	(sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
	        			        (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
	                
	            ipuSetIOSize(Img_Width, Img_Height);
	         	isuSetIOSize(Img_Width, Img_Height); 
            }
            break;
	}
	        	                     		
	uiDigZoomSetFlag = 0;

	return 1;
}

void siuSetAWBGain(s32 Rgain,s32 Bgain)
{
    Rgain=(Rgain*64+500)/1000; //Lucian: 轉為 2.6 format
    if(Rgain > 0x0ff)
        Rgain=0x0ff;
    Bgain=(Bgain*64+500)/1000;
    if(Bgain > 0x0ff)
        Bgain=0x0ff;

    //SIU digital gain =1 @601/656 mode.
    SiuPreGammaGain =  (0x40 << SIU_R_GAIN_SHFT)      |
                       (0x40 << SIU_Gr_GAIN_SHFT)     |
                       (0x40 << SIU_Gb_GAIN_SHFT)     |
                       (0x40 << SIU_B_GAIN_SHFT);
}



void siuStop(void)
{
	// set SIU_OE = 0
	//SiuSensCtrl &= 0xffffdfff;
	//SiuSensCtrl &= ~(SIU_CAPT_ENA | SIU_INT_ENA_FRAM | SIU_DEF_PIX_ENA);
}

u16 getPreviewZoomScale(s32 zoomFactor)
{
     return (PreviewZoomParm_VGA[zoomFactor].scale);
}

void getPreviewZoomSize(s32 zoomFactor,u32* X,u32* Y)
{
     *X=PreviewZoomParm_VGA[zoomFactor].size.x;
     *Y=PreviewZoomParm_VGA[zoomFactor].size.y;
}


void siuGetPreviewZoomWidthHeight(s32 zoomFactor,u16 *W, u16 *H)
{
    *W=640;
    *H=480;
}


u16 getVideoZoomScale(s32 zoomFactor)
{
     return (PreviewZoomParm_VGA[zoomFactor].scale);
}

u16 getSensorRawWidth(void)
{
    return   sensor_validsize.imgSize.x; 
}

void siu_FCINTE_ena(void)
{
   SiuSensCtrl |= SIU_INT_ENA_FRAM; 
}

void siu_FCINTE_disa(void)
{
   SiuSensCtrl &= (~SIU_INT_ENA_FRAM); 
}


#endif	








