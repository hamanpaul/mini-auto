/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    siu.c

Abstract:

    The routines of Sensor Interface Unit.
    1. Sensor 參數設定:
        a. Preview/Capture/Video clip mode.
        b. Digital Zoom.(Smart Zoom)
    2. SIU 參數設定.(Ex. OB,Digital Gain, Lens shading, Pre-gamma)          
    3. Auto Exposure.
    4. Auto White Balance. (Preview/Video Clip mode)
    5. Flash Light control.

Environment:

        ARM RealView Developer Suite

Revision History:
    
    2005/08/26  David Tsai  Create  
*/

#include "general.h"
 #if (Sensor_OPTION == Sensor_MI_5M)    //Lisa 5M
        #include "board.h"
        
        #include "task.h"
        #include "siuapi.h"
        #include "siu.h"
        #include "siureg.h"
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
        #include "../../isu/inc/isureg.h"
        #include "rfiuapi.h"


/*
 *********************************************************************************************************
 *  SIU Function Enable
 *********************************************************************************************************
 */
 //Lucian: PA9001/PA002's defect-pixel-compensation has bug! 不建議使用.
 //
#if ( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A))
#define SIU_DEFECT_PIXEL_ENA    0
#else
#define SIU_DEFECT_PIXEL_ENA    0
#endif

/*
 *********************************************************************************************************
 *  SYS Constant
 *********************************************************************************************************
 */

#define SIU_TIMEOUT         30  
 /*
 *********************************************************************************************************
 * SIU Constant
 *********************************************************************************************************
 */
 
//-----AE_report------//by lisa_0425_S
#if (AE_report==AE_report_Soft)
  #define AE_WSIZE_SEGMENT              5
  #define AE_HSIZE_SEGMENT              5
  
  #define AE_WIN_WIDTH                  160     //PANNEL_X  160
  #define AE_WIN_HEIGHT                 240     //PANNEL_Y    240
  #define AE_DOWN_SCALAR_X              4
  #define AE_DOWN_SCALAR_Y              8

  #define AE_WIN_WIDTH_TV               640     //PANNEL_X  160
  #define AE_WIN_HEIGHT_TV              480     //PANNEL_Y    240
  #define AE_DOWN_SCALAR_X_TV           8
  #define AE_DOWN_SCALAR_Y_TV           8
#endif

#define AE_AGC_SYNC_FRAMENUM            1
#define AE_SYNC_AGC_SW_POLEN            0    //AGC/SW set use polling or semephore

#define AECNT_PREVIEW                   3  
#if (AE_report==AE_report_Soft)
   #define AECNT_VideoCap               24
#else
   #define AECNT_VideoCap               12 
#endif
#define SIU_CAPOEFINT_DISA              0xffffd0ff //Register mask for disable siu interrrupt


//-- For new AE Table --//
#define Gain_Level           6
#define AE_E_Table_Num      16
#define AE_Table_Row_Num    15

//----Exp. time 轉換常數 ----//

  #define SW_VIDEOCLIP2PREVIEW_RATION    906

#if SIU_MCLK_48M_CAPTURE
   #define SW_CAPTURE2PREVIEW_RATION      889   //x1000,  889是理論值,913是實驗值,444 @ 24MHz
#else
   #define SW_CAPTURE2PREVIEW_RATION      444 
#endif


 /*
 *********************************************************************************************************
 * Variables
 *********************************************************************************************************
 */
u32 siuSceneMode_sel=0;
u32 siuISO_sel=0;
extern u8 IsuOutAreaFlagOnTV;

//-------- Digital Zoom ---------//
/*Hardware limit: In preview/VideoClip mode, Y,X direction cannot do up-scaling.*/
ZOOM_FACTOR CaptureZoomParm[MAX_PREVIEW_ZOOM_FACTOR] = 
{
        {{2560, 1920},  100},
        {{2368, 1776},  108},
        {{2176, 1632},  118},
        {{2048, 1536},  125},
        {{1856, 1392},  138},
        {{1728, 1296},  148},
        {{1600, 1200},  160},
        {{1472, 1104},  174},
        {{1408, 1056},  182},        
        {{1280,  960},  200},        
        {{1216,  912},  211},
        {{1088,  816},  235},
        {{1024,  768},  250},
        {{ 960,  720},  267},
        {{ 896,  672},  286},
        {{ 832,  624},  308},
        {{ 768,  576},  333},
        {{ 704,  528},  364},
        {{ 672,  504},  390},  //Lucian: PA9001 bug, can't process 640x480 bayer data. Set others parameter
};

ZOOM_FACTOR PreviewZoomParm_VGA[MAX_PREVIEW_ZOOM_FACTOR] = 
{
        {{640, 480},  100}, // 0
        {{592, 444},  108}, // 1
        {{544, 408},  118}, // 2
        {{512, 384},  125}, // 3
        {{464, 348},  138}, // 4
        {{432, 324},  148}, // 5
        {{400, 300},  160}, // 6
        {{368, 276},  174}, // 7
        {{352, 264},  182}, // 8
        
        {{336, 252},  190}, // 9
        {{320, 240},  200}, // 10
        {{304, 228},  210}, // 11
        {{288, 216},  222}, // 12
        {{272, 204},  235}, // 13
        {{256, 192},  250}, // 14
        {{240, 180},  266}, // 15
        {{224, 168},  286}, // 16
        {{208, 156},  307}, // 17
        {{192, 144},  333}, // 18
};

ZOOM_FACTOR PreviewZoomParm_HD[MAX_PREVIEW_ZOOM_FACTOR] = 
{
        {{1280,  720},  100}, // 0
        {{1216,  684},  102}, // 1
        {{1152,  648},  105}, // 2
        {{1056,  594},  118}, // 3   
        {{992,   558},  111}, // 4  
        {{896,   504},  114}, // 5
        {{832,   468},  117}, // 6
        {{736,   414},  120}, // 7
        {{672,   378},  123}, // 8
        
        {{640,   360},  124}, // 9
        {{576,   324},  126}, // 10
        {{512,   288},  129}, // 11     
        {{480,   270},  130}, // 12
        {{448,   252},  132}, // 13
        {{416,   234},  133}, // 14
        {{384,   216},  134}, // 15
        {{352,   198},  135}, // 16
        {{320,   180},  136}, // 17
        {{288,   162},  137}, // 18
};

ZOOM_FACTOR VideoZoomParm_VGA[MAX_VIDEOCLIP_ZOOM_FACTOR] = 
{    
        {{800,  600},  100}, // 0
        {{784,  588},  102}, // 1
        {{768,  576},  104}, // 2
        {{736,  552},  108}, // 3
        {{712,  534},  112}, // 4
        {{688,  516},  118}, //5
        {{640,  480},  125}, //6                
        
        {{800,  600},  150}, //7
        {{768,  576},  156}, //8
        {{736,  552},  163}, //9
        {{712,  534},  168}, //10
        {{688,  516},  174}, //11
        {{640,  480},  187}, //12
                
        {{800,  600},  300}, //13
        {{768,  576},  312}, //14
        {{736,  552},  326}, //15    
        {{712,  534},  337}, //16    
        {{688,  516},  348}, //17
        {{640,  480},  375}, //18
};

ZOOM_FACTOR VideoZoomParm_D1[MAX_VIDEOCLIP_ZOOM_FACTOR] = 
{
        {{800,  544},  100}, // 0
        {{784,  524},  102}, // 1
        {{768,  512},  104}, // 2
        {{752,  504},  108}, // 3
        {{736,  492},  112}, // 4
        {{720,  480},  118}, //5
        {{720,  480},  125}, //6                
        
        {{800,  536},  150}, //7
        {{784,  524},  156}, //8
        {{768,  512},  163}, //9
        {{752,  504},  168}, //10
        {{736,  492},  174}, //11
        {{720,  480},  187}, //12
                
        {{800,  536},  300}, //13
        {{784,  524},  312}, //14
        {{768,  512},  326}, //15    
        {{752,  504},  337}, //16    
        {{736,  492},  348}, //17
        {{720,  480},  375}, //18
    
};
//---- White Balance Compensation ----//
s32 siuWBComp_RBRatio[4];

//-----Defect Compensation -----//
#define SIU_DEF_PIX_TBL_COUNT   3
__align(16) SIU_COORD siuDefPixTbl[SIU_DEF_PIX_TBL_COUNT] = 
{
    { 0x0055,   0x0055 },
    { 0x00aa,   0x00aa },
    { 0xffff,   0xffff },       
};

u8 testflag = 0;

__align(16)	DEFECT_PIXEL_COORDINATE_SDV1 Coordinate_SDV1[Capture_TargetNum_SDV1];




//-------Set SIU pre-gamma -------//
#define SIU_B_GAMMA_TBL_COUNT     17

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
u8 siuPreviewFlag = 1;
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
#if ( (HW_BOARD_OPTION == SALIX_SDV)||(HW_BOARD_OPTION==HX_DH500)||(HW_BOARD_OPTION==ULTMOST_SDV))//JJ added
    SIU_LENS_SHADING lensShading_Capture = 
{ 
        {1330, 850 },   /* centerOffset(X,Y) */ 
        1330 * 1330,    /* cornerX2 */  
        850 * 850,  /* cornerY2 */
        288,                                                    /* shading_G_Gain */
        320,                                                    /* shading_R_Gain */
        288,                                                    /* shading_B_Gain */
        (u32)SIU_LC_RTYPE_SK9,                                  /* rType */
        (u32)SIU_LC_SCAL_1_1,                                   /* scaleSize */
        (u32)SIU_LC_OUT_INVERT_DISA                             /* outputInvert */
    };
    
    SIU_LENS_SHADING lensShading_Preview = 
    { 
        {332, 212 },    /* centerOffset(X,Y) */ 
        332 * 332,  /* cornerX2 */  
        212 * 212,  /* cornerY2 */
        511,                                                    /* shading_G_Gain */
        511,                                                    /* shading_R_Gain */
        511,                                                    /* shading_B_Gain */
        (u32)SIU_LC_RTYPE_SK7,                                  /* rType */
        (u32)SIU_LC_SCAL_1_1,                                   /* scaleSize */
        (u32)SIU_LC_OUT_INVERT_DISA                             /* outputInvert */
    };
    
    SIU_LENS_SHADING lensShading_Video = 
    { 
        {416, 266 },    /* centerOffset(X,Y) */ 
        416 * 416,  /* cornerX2 */  
        266 * 266,  /* cornerY2 */
        511,                                                    /* shading_G_Gain */
        511,                                                    /* shading_R_Gain */
        511,                                                    /* shading_B_Gain */
        (u32)SIU_LC_RTYPE_SK7,                                  /* rType */
        (u32)SIU_LC_SCAL_1_1,                                   /* scaleSize */
        (u32)SIU_LC_OUT_INVERT_DISA                             /* outputInvert */
    };
#else
    SIU_LENS_SHADING lensShading_Capture = 
    { 
        {SENSOR_VALID_SIZE_X / 2, SENSOR_VALID_SIZE_Y / 2 },    /* centerOffset */  
        (SENSOR_VALID_SIZE_X / 2) * (SENSOR_VALID_SIZE_X / 2),  /* cornerX2 */  
        (SENSOR_VALID_SIZE_Y / 2) * (SENSOR_VALID_SIZE_Y / 2),  /* cornerY2 */
         256,                                               /* shading_G_Gain */
         256,                                               /* shading_R_Gain */
         256,                                               /* shading_B_Gain */
        (u32)SIU_LC_RTYPE_SK9,                                  /* rType */
        (u32)SIU_LC_SCAL_1_1,                                   /* scaleSize */
        (u32)SIU_LC_OUT_INVERT_DISA                             /* outputInvert */
    };
    
    SIU_LENS_SHADING lensShading_Preview = 
    { 
        {320, 240 },    /* centerOffset(X,Y) */ 
        320 * 320,  /* cornerX2 */  
        240 * 240,  /* cornerY2 */
        511,                                                    /* shading_G_Gain */
        511,                                                    /* shading_R_Gain */
        511,                                                    /* shading_B_Gain */
    (u32)SIU_LC_RTYPE_SK7,                                  /* rType */
    (u32)SIU_LC_SCAL_1_1,                                   /* scaleSize */
    (u32)SIU_LC_OUT_INVERT_DISA                             /* outputInvert */
};

    SIU_LENS_SHADING lensShading_Video = 
    { 
        {400, 300 },    /* centerOffset(X,Y) */ 
        400 * 400,  /* cornerX2 */  
        300 * 300,  /* cornerY2 */
        511,                                                    /* shading_G_Gain */
        511,                                                    /* shading_R_Gain */
        511,                                                    /* shading_B_Gain */
        (u32)SIU_LC_RTYPE_SK7,                                  /* rType */
        (u32)SIU_LC_SCAL_1_1,                                   /* scaleSize */
        (u32)SIU_LC_OUT_INVERT_DISA                             /* outputInvert */
    };
#endif
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
u8 siuFlashLightYtarget = 0x60;
u32 siuFlashLightCount = 0x1F4;

u32 siuYavg[3]={0};
u8 siuAeYreport[25] = {0};
u8 siuRatioRmsTbl[256] = {1,1,2,2,2,2,3,3,3,3,
             3,3,4,4,4,4,4,4,4,4,
             5,5,5,5,5,5,5,5,5,5,
             6,6,6,6,6,6,6,6,6,6,
             6,6,7,7,7,7,7,7,7,7,
             7,7,7,7,7,7,8,8,8,8,
             8,8,8,8,8,8,8,8,8,8,
             8,8,9,9,9,9,9,9,9,9,
             9,9,9,9,9,9,9,9,9,9,
            10,10,10,10,10,10,10,10,10,10,
            10,10,10,10,10,10,10,10,10,10,
            11,11,11,11,11,11,11,11,11,11,
            11,11,11,11,11,11,11,11,11,11,
            11,11,12,12,12,12,12,12,12,12,
            12,12,12,12,12,12,12,12,12,12,
            12,12,12,12,12,12,13,13,13,13,
            13,13,13,13,13,13,13,13,13,13,
            13,13,13,13,13,13,13,13,13,13,
            13,13,14,14,14,14,14,14,14,14,
            14,14,14,14,14,14,14,14,14,14,
            14,14,14,14,14,14,14,14,14,14,
            15,15,15,15,15,15,15,15,15,15,
            15,15,15,15,15,15,15,15,15,15,
            15,15,15,15,15,15,15,15,15,15,
            16,16,16,16,16,16,16,16,16,16,
            16,16,16,16,16,16};

u8 siuFlashLightInterPolationTbl[160] = {
            250,244,239,235,230,225,221,217,213,208,
            205,201,197,193,190,186,183,180,177,174,
            171,168,165,162,160,157,154,152,149,147,
            145,142,140,138,136,134,132,130,128,126,
            124,122,120,119,117,115,114,112,111,109,
            108,106,105,103,102,101,99,98,97,95,
            94,93,92,91,90,88,87,86,85,84,
            83,82,81,80,79,78,77,76,76,75,
            74,73,72,71,71,70,69,68,68,67,
            66,65,65,64,63,63,62,61,61,60,
            59,59,58,58,57,57,56,55,55,54,
            54,53,53,52,52,51,51,50,50,49,
            49,48,48,48,47,47,46,46,45,45,
            45,44,44,43,43,43,42,42,42,41,
            41,41,40,40,40,39,39,39,38,38,
            38,37,37,37,36,36,36,35,35,35};

//------------------------ AE control -------------------------//
/*
          -2.5EV -2.0EV    -1.5EV    -1.0EV    -0.5EV    0EV     0.5EV     1.0EV     1.5EV    2.0EV
           -15dB -12dB     -9dB      -6dB      -3dB      0dB       3dB      6dB       9dB     12dB
          ----------------------------------------------------------------------------------------------
Software    21     30      43         60        85       120      170       239      240       255
 
Hardware           10       15         23        37       60     102       172       240     245
                  (24.93     (35.22)  (49.74)   (70.26) (99.25)  (140.19)  (198.03)    (255)  (255)
*/
#if (AE_report==AE_report_Soft)
  #if(HW_BOARD_OPTION==ULTMOST_SDV)
     const u8 siuSetYtargetTab[9]={15, 21, 30, 43, 60, 85, 120, 170, 239};   //Lucian: modify to 85
  #else
     const u8 siuSetYtargetTab[9]={30, 43, 60, 85, 120, 170, 239, 240, 255};   //Lucian: modify to 120
  #endif
  u32 AE_blk_YSum[AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT];
#else
  const u8 siuSetYtargetTab[9]={10, 15, 23, 37, 60, 102, 172, 240, 245};  //Lucian: modify 65->50->42->55  
#endif

u8 siuY_TargetIndex=4;


DEF_AE_Tab AE_EG_Tab_VideoPrev_5060Hz[2][2][AE_E_Table_Num*Gain_Level];  //AE Index Table
u8 AE_Flicker_50_60_sel=SENSOR_AE_FLICKER_60HZ;

u16 Log_Table[256] ={6,11,17,23,29,34,40,45,51,57,62,68,73,79,84,90,95,100,106,111,116,122,127,132,
                    138,143,148,153,159,164,169,174,179,184,189,194,199,204,209,214,219,224,229,234,
                    239,244,249,254,259,264,268,273,278,283,288,292,297,302,297,302,306,311,316,320,
                    325,330,334,339,343,348,353,357,362,366,371,375,380,384,388,393,397,402,406,411,
                    415,419,424,428,432,436,441,445,449,454,458,462,466,470,475,479,483,487,491,495,
                    500,504,508,512,516,520,524,528,532,536,540,544,548,552,556,560,564,568,572,576,
                    580,584,587,591,595,599,603,607,610,614,618,622,626,633,637,641,644,648,656,659,
                    663,667,670,674,678,681,685,689,692,696,699,703,707,710,714,717,721,728,731,735,
                    738,742,745,749,752,756,759,763,766,770,773,776,780,783,787,790,793,797,800,803,
                    807,810,813,817,820,823,827,830,833,837,840,843,846,850,853,856,859,863,866,869,
                    872,875,879,882,885,888,891,894,898,901,904,907,910,913,916,919,922,926,929,932,
                    935,938,941,944,947,950,953,956,959,962,965,968,971,974,977,980,983,986,989,992,
                    995,998,1001,1004,1007,1010,1012,1015,1018,1021,1024};

/* SW */
u16 AE_Log_Mapping_Table[256] = {0,0,6165,9771,12330,14315,15937,17308,18495,19543,20480,21328,22102,22814,23473,24086,24660,
                25200,25708,26189,26645,27079,27493,27888,28267,28630,28979,29314,29638,29950,30251,30543,
                30825,31099,31365,31623,31873,32117,32354,32585,32810,33030,33244,33453,33658,33858,34053,
                34245,34432,34615,34795,34971,35144,35313,35479,35643,35803,35960,36115,36267,36417,36564,
                36708,36850,36991,37128,37264,37398,37530,37660,37788,37914,38038,38161,38282,38401,38519,
                38635,38750,38863,38975,39086,39195,39303,39409,39514,39619,39721,39823,39924,40023,40121,
                40218,40315,40410,40504,40597,40689,40780,40871,40960,41049,41136,41223,41309,41394,41478,
                41562,41645,41726,41808,41888,41968,42047,42125,42203,42280,42356,42432,42507,42582,42655,
                42729,42801,42873,42945,43016,43086,43156,43225,43294,43362,43429,43496,43563,43629,43695,
                43760,43825,43889,43953,44016,44079,44141,44203,44265,44326,44387,44447,44507,44566,44625,
                44684,44742,44800,44858,44915,44972,45028,45085,45140,45196,45251,45306,45360,45414,45468,
                45521,45574,45627,45680,45732,45784,45835,45886,45937,45988,46038,46089,46138,46188,46237,
                46286,46335,46383,46432,46480,46527,46575,46622,46669,46716,46762,46808,46854,46900,46945,
                46991,47036,47081,47125,47169,47214,47258,47301,47345,47388,47431,47474,47517,47559,
                47601,47643,47685,47727,47768,47810,47851,47892,47932,47973,48013,48053,48093,48133,48173,
                48212,48251,48291,48329,48368,48407,48445,48483,48522,48559,48597,48635,48672,48710,48747,
                48784,48821,48857,48894,48930,48966,49002,49038,49074,49110,49145,49181,49216,49251,49286,
                };

#if (AE_report==AE_report_Soft)

u8 siu_dftAeWeight1[AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT] = {
                                0, 1,  3,  1, 0,
                                1, 2,  5,  2, 1,
                                3, 4,  8,  4, 3,
                                1, 4,  6,  4, 1,
                                0, 3,  5,  3, 0
                          };
#else
    
    u8 siu_dftAeWeight1[25] = {
            0, 1,  3,  1, 0,
            1, 2,  5,  2, 1,
            3, 4,  8,  4, 3,
            1, 4,  6,  4, 1,
            0, 3,  5,  3, 0
            
    };
    
#endif

u8 siu_dftAeWeight2[25] = {  //閃光燈用之 AE weight table.
        8, 8, 8, 8, 8,
        8, 9, 9, 9, 8,
        8, 9,10, 9, 8,
        8, 9,10, 9, 8,
        7, 8, 8, 8, 7
};

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
extern u8 config_mode_enable[6];
#endif

extern s32 mp4_avifrmcnt, isu_avifrmcnt; /*BJ 0530 S*/  
extern s16 Preview_ipuCCM_DFT[];
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

extern u8 DefectPixel_Msg;

// OB
extern u16 OB_B;
extern u16 OB_Gb;
extern u16 OB_R;
extern u16 OB_Gr;

extern u8 sysCheckZoomRun_flag;

#if (AE_report==AE_report_Soft)
   extern volatile s32 isu_idufrmcnt;
#endif

extern u8 uiMenuVideoSizeSetting;


#if HW_MD_SUPPORT
  extern u8 MotionDetect_en;
#endif
/*
 *********************************************************************************************************
 * Function prototype
 *********************************************************************************************************
 */ 

s32 siuGetFlashLightYavg(u8);
s32 siuFlashLightPreCharge(void);

/* SW */
s32 siuGetAeYavg(u8*);

s32 siuInit(void);
s32 siuSensorInit(u8,u8);
s32 siuAeInit(void);
s32 siuSetOpticalBlack(u8, u8);
s32 siuSetDigitalGain(u8, u16);
s32 siuSetLensShading(SIU_LENS_SHADING*,u32 LS_gain_x100);
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
s32 siuAutoExposure(void);
s32 siuAutoWhiteBalance(void);
void siuAE_ExposureControl(void);
s32 setSensorWinSize(s8, s8);
u16 getPreviewZoomScale(s32);
u16 getVideoZoomScale(s32);
void siuStop(void);
u16 Cal_LOG2(u16 X);
void siuSetAwbGainOnPreGamma(u32 Rgain,u32 Grgain,u32 Gbgain,u32 Bgain,int mode);


#if HW_MD_SUPPORT
extern  void  mduMotionDetect_init();  
#endif


/*
 *********************************************************************************************************
 * Function
 *********************************************************************************************************
 */  

#define R_Gain_MAX           1800
#define R_Gain_MIN           800
#define B_Gain_MAX           2100
#define B_Gain_MIN           900

void siuAwbGrayWorld(int mode)
{
    u8 * img_ptr;
    u32 i , j , Rsum=0 , Gsum=0 , Bsum=0;
    u32 Rgain , Bgain , Ggain,Grgain,Gbgain;
    u32 temp1 , temp2;
    s16 ipuCCM_DFT[10];


    Bsum = SiuAwbBSum;
    Gsum = SiuAwbGSum;
    Rsum = SiuAwbRSum;

    if(Bsum == 0 || Gsum == 0 || Rsum == 0)
        return;
        
    if(ae_YavgValue<6) //Lucian: 在低亮度下, 色溫直接設定為5000K
    {
        Rgain = AWBgain_Preview[0];
        Ggain = 1000;
        Bgain = AWBgain_Preview[2];        
    }
    else
    {
        if(Rsum==0) Rsum=1;
        Rgain = (Gsum*1000)/Rsum;
        if(Rgain>R_Gain_MAX) Rgain=R_Gain_MAX;
        if(Rgain<R_Gain_MIN) Rgain=R_Gain_MIN;  
        
        Ggain = 1000;
        
        if(Bsum==0) Bsum=1;
        Bgain = (Gsum*1000)/Bsum;
        if(Bgain>B_Gain_MAX) Bgain=B_Gain_MAX;
        if(Bgain<B_Gain_MIN) Bgain=B_Gain_MIN;  
    }
    
    AWBgain_Preview[0] = (s16)Rgain;
    AWBgain_Preview[1] = (s16)Ggain;
    AWBgain_Preview[2] = (s16)Bgain;

    //DEBUG_SIU("-->Rgain=%d,Bgain=%d\n",Rgain,Bgain);

#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A) || (CHIP_OPTION == CHIP_A1026A))
  #if  PA9003_AWB_GAIN_IN_SIU
        siuSetAWBGain(  (s32)AWBgain_Preview[0] *(255)/(255-OB_R),
                        (s32)AWBgain_Preview[2] *(255)/(255-OB_B) 
                      );
  #else
        ipuSetCfaAWBgain(  (s32)AWBgain_Preview[0] *(255)/(255-OB_R),
                           (s32)AWBgain_Preview[2] *(255)/(255-OB_B) 
                         );    
  #endif
#elif(CHIP_OPTION == CHIP_PA9002D) //PA9002D: Color transform 已內建進 Hardware.
    #if( PA9002D_AWB_EN==1 )
        Rgain  =  Rgain * ( 255/(255-OB_R));
        Grgain =  Ggain * ( 255/(255-OB_Gr));
        Gbgain =  Ggain * ( 255/(255-OB_Gb));
        Bgain  =  Bgain * ( 255/(255-OB_B));
        
        Preview_ipuCCM_DFT[0] = (s16)d50_IPU_CCM[0];
        Preview_ipuCCM_DFT[1] = (s16)d50_IPU_CCM[1];
        Preview_ipuCCM_DFT[2] = (s16)d50_IPU_CCM[2];
        Preview_ipuCCM_DFT[3] = (s16)d50_IPU_CCM[3];
        Preview_ipuCCM_DFT[4] = (s16)d50_IPU_CCM[4];
        Preview_ipuCCM_DFT[5] = (s16)d50_IPU_CCM[5];
        Preview_ipuCCM_DFT[6] = (s16)d50_IPU_CCM[6];
        Preview_ipuCCM_DFT[7] = (s16)d50_IPU_CCM[7];
        Preview_ipuCCM_DFT[8] = (s16)d50_IPU_CCM[8];

        ipuSetColorCorrMatrix(Preview_ipuCCM_DFT);
        siuSetAwbGainOnPreGamma(Rgain,Grgain,Gbgain,Bgain,mode);
        siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
    #else
        Preview_ipuCCM_DFT[0] = (s16)( (d50_IPU_CCM[0]*AWBgain_Preview[0]/1000)*(255)/(255-OB_R) );
        Preview_ipuCCM_DFT[1] = (s16)(d50_IPU_CCM[1]*AWBgain_Preview[1]/1000);
        Preview_ipuCCM_DFT[2] = (s16)(d50_IPU_CCM[2]*AWBgain_Preview[2]/1000);
        Preview_ipuCCM_DFT[3] = (s16)(d50_IPU_CCM[3]*AWBgain_Preview[0]/1000);
        Preview_ipuCCM_DFT[4] = (s16)( (d50_IPU_CCM[4]*AWBgain_Preview[1]/1000)*(255)/(255-OB_Gr) );
        Preview_ipuCCM_DFT[5] = (s16)(d50_IPU_CCM[5]*AWBgain_Preview[2]/1000);
        Preview_ipuCCM_DFT[6] = (s16)(d50_IPU_CCM[6]*AWBgain_Preview[0]/1000);
        Preview_ipuCCM_DFT[7] = (s16)(d50_IPU_CCM[7]*AWBgain_Preview[1]/1000);
        Preview_ipuCCM_DFT[8] = (s16)( (d50_IPU_CCM[8]*AWBgain_Preview[2]/1000)*(255)/(255-OB_B) );
        
        ipuSetColorCorrMatrix(Preview_ipuCCM_DFT);
    #endif

#else   //PA9001D
        Preview_ipuCCM_DFT[0] = (s16)( (d50_IPU_CCM[0]*AWBgain_Preview[0]/1000)*(255)/(255-OB_R) );
        Preview_ipuCCM_DFT[1] = (s16)(d50_IPU_CCM[1]*AWBgain_Preview[1]/1000);
        Preview_ipuCCM_DFT[2] = (s16)(d50_IPU_CCM[2]*AWBgain_Preview[2]/1000);
        Preview_ipuCCM_DFT[3] = (s16)(d50_IPU_CCM[3]*AWBgain_Preview[0]/1000);
        Preview_ipuCCM_DFT[4] = (s16)( (d50_IPU_CCM[4]*AWBgain_Preview[1]/1000)*(255)/(255-OB_Gr) );
        Preview_ipuCCM_DFT[5] = (s16)(d50_IPU_CCM[5]*AWBgain_Preview[2]/1000);
        Preview_ipuCCM_DFT[6] = (s16)(d50_IPU_CCM[6]*AWBgain_Preview[0]/1000);
        Preview_ipuCCM_DFT[7] = (s16)(d50_IPU_CCM[7]*AWBgain_Preview[1]/1000);
        Preview_ipuCCM_DFT[8] = (s16)( (d50_IPU_CCM[8]*AWBgain_Preview[2]/1000)*(255)/(255-OB_B) );
        
        ipuSetColorCorrMatrix(Preview_ipuCCM_DFT);
#endif  
}

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

 #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
    siuSetAwbGainOnPreGamma(1280,1000,1000,1330,SIU_AWB_NORMAL);  //Lucian: 預設光源為D5000 K.
    siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
 #else
    siuSetAwbGainOnPreGamma(1000,1000,1000,1000,SIU_AWB_NORMAL);  //Lucian: 不做Pre-AWB.
    siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
 #endif
 

 #if HW_MD_SUPPORT
    mduMotionDetect_init();  
 #endif

    OSTaskCreate(SIU_TASK, SIU_TASK_PARAMETER, SIU_TASK_STACK, SIU_TASK_PRIORITY_PVW); 
    siuSuspendTask();

    return 1;
}


/* SW 0107 S */
#define AE_TAB_ENTRY_NUM    90

void siuSetAETbl(void)
{
    u8 i;    
    
    //--------------------------Capture/Preview Mode,60Hz----------------------------//
    /*
       Lucian: 將Capture/Preview mode 的最大曝光時間限定在1/30sec(60Hz),1/25sec(50Hz)
               防止手震造成影像模糊.
    */
   #if 0 //最大曝光時間= 1/7.5 sec.
    const static u16 SW_Prev_60Hz[AE_TAB_ENTRY_NUM]={
              1,     1,     1,     1,     1,    1, 
              2,     2,     3,     3,     3,    3,
              4,     4,     5,     5,     6,    7, 
              8,     9,    10,    11,    13,   14, 
             16,    18,    20,    23,    25,   29, 
             33,    37,    42,    47,    52,   59, 
             65,    73,    82,    92,   103,  116, 
            129,   129,   129,   129,   129,  129, 
            259,   259,   259,   389,   389,  389, 
            518,   518,   648,   648,   777,  907, 
           1035,  1035,  1294,  1423,  1551, 1810, 
           2070,  2070,  2070,  2070,  2070, 2070, 
           2070,  2070,  2070,  2070,  2070, 2070, 
           2070,  2070,  2070,  2070,  2070, 2070, 
           2070,  2070,  2070,  2070,  2070, 2070 
    };
    
    const static u8 AG_Prev_60Hz[AE_TAB_ENTRY_NUM]={  
          8,   9,  10,  11,  12,  14,
          8,  10,   8,   9,  10,  10,
          8,   9,   8,   9,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,  10,
          8,   9,   8,   9,   8,   8,
          8,   9,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
         16,  18,  20,  23,  25,  28,
         32,  18,  20,  23,  25,  28,
         32,  32,  32,  32,  32,  32   
    };
    
    const static u8 MUL_Prev_60Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1
    };
    
    const static u8 DG_Prev_60Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 2, 3, 4, 6 
    };
    
    const static u16 DBTab_Prev_60Hz[AE_TAB_ENTRY_NUM+1]={
        0,
        1048,         937,         848,         774,        1371,        1187,        
        1985,        1622,        1047,         937,           0,         574,        
        1048,         937,        1048,         574,        1371,        1188,        
        1047,         937,         848,        1486,         659,        1188,        
        1047,         937,        1243,         742,        1320,        1149,        
        1018,        1127,        1001,         899,        1123,         862,        
        1032,        1034,        1024,        1004,        1057,         945,        
        1048,         937,         848,        1485,         660,        1222,
        1047,         937,        1633,        1048,         937,         563,        
        1047,         944,        1048,         567,        1376,        1174,        
        1048,         939,         845,         766,        1374,        1193,        
        1048,         937,         848,        1486,         659,        1187,        
        1048,         937,        1243,         742,        1008,        1188,        
        1047,         937,        1243,         742,        1008,        1188,        
        1047,         937,         848,         774,        1371,           0

    };
    #endif

    // 最大曝光時間=1/30 sec
    const static u16 SW_Prev_60Hz[AE_TAB_ENTRY_NUM]={
              1,     1,     1,     1,     1,    1, 
              2,     2,     3,     3,     3,    3,
              4,     4,     5,     5,     6,    7, 
              8,     9,    10,    11,    13,   14,
             16,    18,    20,    23,    25,   29, 
             33,    37,    42,    47,    52,   59, 
             65,    73,    82,    92,   103,  116,
            129,   129,   129,   129,   129,  129,
            259,   259,   259,   389,   389,  389, 
            518,   518,   518,   518,   518,  518,
            518,   518,   518,   518,   518,  518,
            518,   518,   518,   518,   518,  518, 
            518,   518,   518,   518,   518,  518,
            518,   518,   518,   518,   518,  518,
            518,   518,   518,   518,   518,  518 
    };
    
    const static u8 AG_Prev_60Hz[AE_TAB_ENTRY_NUM]={  
          8,   9,  10,  11,  12,  14,
          8,  10,   8,   9,  10,  10,
          8,   9,   8,   9,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,  10,
          8,   9,  10,  11,  13,  14,
          16, 18,  20,  23,  25,  28,
          32, 18,  20,  23,  26,  29,
          32,  32,  32,  32,  32,  32,
          32,  32,  32,  32,  32,  32,
          32,  32,  32,  32,  32,  32   
    };
    
    const static u8 MUL_Prev_60Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1
    };
    
    const static u8 DG_Prev_60Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 2, 3, 5, 6,
          8,10,12,15,17,20,
         24,28,32,37,43,49
    };
    
    const static u16 DBTab_Prev_60Hz[AE_TAB_ENTRY_NUM+1]={
           0,
        1048,         937,         848,         774,        1371,        1187,        
        1985,        1622,        1047,         937,           0,         574,        
        1048,         937,        1048,         574,        1371,        1188,        
        1047,         937,         848,        1486,         659,        1188,        
        1047,         937,        1243,         742,        1320,        1149,        
        1018,        1127,        1001,         899,        1123,         862,        
        1032,        1034,        1024,        1004,        1057,         945,        
        1048,         937,         848,        1485,         660,        1222,
        1047,         937,        1633,        1048,         937,         563,        
        1047,         937,         848,        1486,         659,        1188,        
        1047,         938,        1243,         741,        1008,        1188,        
        1048,         937,        1243,        1090,         971,         876,        
        1048,         937,         847,        1486,         659,        1188,       
        1048,         937,        1243,         742,        1008,        1187,        
        1048,         937,        1048,        1113,         989,           0
      };
    
    //--------------------------Capture Mode,50Hz----------------------------//
  #if 0
    const static u16 SW_Prev_50Hz[AE_TAB_ENTRY_NUM]={
              1,     1,     1,     1,     1,    1, 
              2,     2,     2,     3,     3,    4,
              5,     5,     6,     6,     8,    9, 
             10,    11,    13,    14,    16,   18,
             19,    21,    24,    27,    30,   34,
             39,    44,    49,    55,    62,   69,
             78,    88,    98,   110,   124,  139,
            155,   155,   155,   155,   155,  155,
            310,   310,   310,   465,   465,  465,
            620,   620,   775,   775,   930, 1085,
           1240,  1240,  1550,  1705,  1860, 2171,
           2481,  2481,  2481,  2481,  2481, 2481,
           2481,  2481,  2481,  2481,  2481, 2481,
           2481,  2481,  2481,  2481,  2481, 2481,
           2481,  2481,  2481,  2481,  2481, 2481 
    };
    
    const static u8 AG_Prev_50Hz[AE_TAB_ENTRY_NUM]={  
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,   8,
          8,   9,   8,   9,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,  10,
          8,   9,   8,   9,   8,   8,
          8,   9,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
         16,  18,  20,  23,  25,  28,
         32,  18,  20,  23,  25,  28,
         32,  32,  32,  32,  32,  32     
    };
    
    const static u8 MUL_Prev_50Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1
    };
    
    const static u8 DG_Prev_50Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 2, 3, 4, 6
    };
    
    const static u16 DBTab_Prev_50Hz[AE_TAB_ENTRY_NUM+1]={
        0,
        1048,         937,         848,        1486,         659,        1187,
        1048,         937,        1622,        1047,        1511,        1985,        
        1048,         574,        1047,        1512,        1047,         937,         
         848,        1486,         659,        1188,        1047,         481,         
         890,        1188,        1048,         937,        1113,        1220,        
        1073,         957,        1028,        1065,         952,        1090,        
        1073,         958,        1027,        1066,        1015,         969,        
        1048,         937,         848,        1486,         659,        1187,
        1048,         937,        1622,        1047,         937,         574,        
        1048,         937,        1048,         574,        1371,        1188,        
        1047,         937,         848,         774,        1375,        1187,        
        1048,         937,         848,        1486,         659,        1187,        
        1048,         937,        1243,         742,        1008,        1187,        
        1048,         937,        1243,         742,        1008,        1188,        
        1047,         937,         848,         774,        1371,           0
    };
   #endif
    //最大曝光時間 = 1/25 sec
    const static u16 SW_Prev_50Hz[AE_TAB_ENTRY_NUM]={
              1,     1,     1,     1,     1,    1, 
              2,     2,     2,     3,     3,    4,
              5,     5,     6,     6,     8,    9, 
             10,    11,    13,    14,    16,   18, 
             19,    21,    24,    27,    30,   34, 
             39,    44,    49,    55,    62,   69, 
             78,    88,    98,   110,   124,  139, 
            155,   155,   155,   155,   155,  155, 
            310,   310,   310,   465,   465,  465, 
            620,   620,   620,   620,   620,  620, 
            620,   620,   620,   620,   620,  620, 
            620,   620,   620,   620,   620,  620, 
            620,   620,   620,   620,   620,  620, 
            620,   620,   620,   620,   620,  620, 
            620,   620,   620,   620,   620,  620 
    };
    
    const static u8 AG_Prev_50Hz[AE_TAB_ENTRY_NUM]={  
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,   8,
          8,   9,   8,   9,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,  10,
          8,   9,  10,  11,  13,  14,
         16,  18,  20,  23,  25,  28,
         32,  18,  20,  23,  25,  28,
         32,  32,  32,  32,  32,  32,
         32,  32,  32,  32,  32,  32,
         32,  32,  32,  32,  32,  32 
    };
    
    const static u8 MUL_Prev_50Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1
    };
    
    const static u8 DG_Prev_50Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 2, 3, 5, 6,
          8,10,12,15,17,20,
         24,28,32,37,43,49
    };
    
    const static u16 DBTab_Prev_50Hz[AE_TAB_ENTRY_NUM+1]={
           0,
        1048,         937,         848,        1486,         659,        1187,                                                                                                                                    
        1048,         937,        1622,        1047,        1511,        1985,                                                                                                                                       
        1048,         574,        1047,        1512,        1047,         937,                                             
         848,        1486,         659,        1188,        1047,         481,                                             
         890,        1188,        1048,         937,        1113,        1220,                                             
        1073,         957,        1028,        1065,         952,        1090,
        1073,         958,        1027,        1066,        1015,         969,                                                      
        1048,         937,         848,        1486,         659,        1187,                                                      
        1048,         937,        1622,        1047,         937,         574,                                                           
        1048,         937,         848,        1486,         659,        1188,                                                
        1047,         937,        1243,         742,        1008,        1188,                                                             
        1047,         937,        1243,         742,        1008,        1188,                                                                
        1047,         937,         848,        1486,         659,        1188,                                                   
        1047,         938,        1243,         741,        1008,        1188,                                             
        1048,         937,        1047,        1114,         989,           0
      };
    //-------------------------------Video Mode,60Hz----------------------------------------------------//
    /*
      Lucian: 目前將Video clip mode 的最大曝光時間設定在1/60sec(60Hz), 1/50sec(50Hz),以求影像清晰.
    */
  #if 0  //Min Frame rate=20Hz
      const static u16 SW_Video_60Hz[AE_TAB_ENTRY_NUM]={
              1,     1,     1,     1,     1,    1,
              2,     2,     3,     3,     3,    3,
              4,     4,     5,     5,     6,    7,
              8,     9,    10,    11,    13,   14, 
             16,    18,    20,    23,    25,   29, 
             33,    37,    42,    47,    52,   59,
             65,    73,    82,    92,   103,  116, 
            129,   129,   129,   129,   129,  129, 
            259,   259,   259,   389,   389,  389,
            518,   518,   648,   648,   777,  777,
            777,   777,   777,   777,   777,  777,
            777,  777,    777,   777,   777,  777,
            777,  777,    777,   777,   777,  777,
            777,  777,    777,   777,   777,  777,
            777,  777,    777,   777,   777,  777 
    };
    
    const static u8 AG_Video_60Hz[AE_TAB_ENTRY_NUM]={  
          8,   9,  10,  11,  12,  14,
          8,  10,   8,   9,  10,  10,
          8,   9,   8,   9,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,  10,
          8,   9,   8,   9,   8,   9,
          11, 12,  14,  16,  18,  20,
          22, 25,  28,  31,  18,  20,
          22,  25,  28,  31,  32,  32,
          32,  32,  32,  32,  32,  32,
          32,  32,  32,  32,  32,  32    
    };
    
    const static u8 MUL_Video_60Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1
    };
    
    const static u8 DG_Video_60Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 1, 2,
          3, 4, 6, 8,10,12,
         14,17,20,23,27,31
    };
    
    const static u16 DBTab_Video_60Hz[AE_TAB_ENTRY_NUM+1]={
        0,
        1048,         937,         848,         774,        1371,        1187,        
        1985,        1622,        1047,         937,           0,         574,        
        1048,         937,        1048,         574,        1371,        1188,
        1047,         937,         848,        1486,         659,        1188,        
        1047,         937,        1243,         742,        1320,        1149,        
        1018,        1127,        1001,         899,        1123,         862,        
        1032,        1034,        1024,        1004,        1057,         945,        
        1048,         937,         848,        1485,         660,        1222,
        1047,         937,        1633,        1048,         937,         563,        
        1047,         944,        1048,         567,        1048,        1785,         
         773,        1372,        1187,        1048,         937,         848,        
        1137,        1008,         905,        1330,         937,         848,        
        1137,        1008,         905,        1330,         937,         848,         
         774,        1371,        1187,        1048,         937,         848,        
        1137,        1008,         905,        1080,         962,           0
      };
   #endif

   #if 0 //min frame rate=30Hz
      const static u16 SW_Video_60Hz[AE_TAB_ENTRY_NUM]={
              1,     1,     1,     1,     1,    1, 
              2,     2,     3,     3,     3,    3,
              4,     4,     5,     5,     6,    7, 
              8,     9,    10,    11,    13,   14,
             16,    18,    20,    23,    25,   29, 
             33,    37,    42,    47,    52,   59, 
             65,    73,    82,    92,   103,  116,
            129,   129,   129,   129,   129,  129,
            259,   259,   259,   389,   389,  389, 
            518,   518,   518,   518,   518,  518,
            518,   518,   518,   518,   518,  518,
            518,   518,   518,   518,   518,  518, 
            518,   518,   518,   518,   518,  518,
            518,   518,   518,   518,   518,  518,
            518,   518,   518,   518,   518,  518 
    };
    
    const static u8 AG_Video_60Hz[AE_TAB_ENTRY_NUM]={  
          8,   9,  10,  11,  12,  14,
          8,  10,   8,   9,  10,  10,
          8,   9,   8,   9,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,  10,
          8,   9,  10,  11,  13,  14,
          16, 18,  20,  23,  25,  28,
          32, 18,  20,  23,  26,  29,
          32,  32,  32,  32,  32,  32,
          32,  32,  32,  32,  32,  32,
          32,  32,  32,  32,  32,  32   
    };
    
    const static u8 MUL_Video_60Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1
    };
    
    const static u8 DG_Video_60Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 2, 3, 5, 6,
          8,10,12,15,17,20,
         24,28,32,37,43,49
    };
    
    const static u16 DBTab_Video_60Hz[AE_TAB_ENTRY_NUM+1]={
           0,
        1048,         937,         848,         774,        1371,        1187,        
        1985,        1622,        1047,         937,           0,         574,        
        1048,         937,        1048,         574,        1371,        1188,        
        1047,         937,         848,        1486,         659,        1188,        
        1047,         937,        1243,         742,        1320,        1149,        
        1018,        1127,        1001,         899,        1123,         862,        
        1032,        1034,        1024,        1004,        1057,         945,        
        1048,         937,         848,        1485,         660,        1222,
        1047,         937,        1633,        1048,         937,         563,        
        1047,         937,         848,        1486,         659,        1188,        
        1047,         938,        1243,         741,        1008,        1188,        
        1048,         937,        1243,        1090,         971,         876,        
        1048,         937,         847,        1486,         659,        1188,       
        1048,         937,        1243,         742,        1008,        1187,        
        1048,         937,        1048,        1113,         989,           0
      };
   #endif 

   #if 1 //min frame rate=60Hz
      const static u16 SW_Video_60Hz[AE_TAB_ENTRY_NUM]={
              1,     1,     1,     1,     1,    1, 
              2,     2,     3,     3,     3,    3,
              4,     4,     5,     5,     6,    7, 
              8,     9,    10,    11,    13,   14, 
             16,    18,    20,    23,    25,   29, 
             33,    37,    42,    47,    52,   59, 
             65,    73,    82,    92,   103,  116, 
            129,   129,   129,   129,   129,  129, 
            259,   259,   259,   259,   259,  259, 
            259,   259,   259,   259,   259,  259, 
            259,   259,   259,   259,   259,  259,  
            259,   259,   259,   259,   259,  259, 
            259,   259,   259,   259,   259,  259, 
            259,   259,   259,   259,   259,  259, 
            259,   259,   259,   259,   259,  259
    };
    
    const static u8 AG_Video_60Hz[AE_TAB_ENTRY_NUM]={  
          8,   9,  10,  11,  12,   14,
          8,  10,   8,   9,  10,   10,
          8,   9,   8,   9,   8,    8,
          8,   8,   8,   8,   8,    8,
          8,   8,   8,   8,   8,    8,
          8,   8,   8,   8,   8,    8,
          8,   8,   8,   8,   8,    8,
          8,   9,  10,  11,  13,   14,
          8,   9,  10,  11,  13,   14,
          16, 18,  20,  23,  25,   28,
          32, 18,  20,  23,  25,   28,
          32, 32,  32,  32,  32,   32,
          32,  32,  32,  32,  32,  32,
          32,  32,  32,  32,  32,  32,
          32,  32,  32,  32,  32,  32     
    };
    
    const static u8 MUL_Video_60Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1
    };
    
    const static u8 DG_Video_60Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 2, 3, 5, 6,
          8,10,12,15,17,20,
         24,28,32,37,43,49, 
         56,64,72,80,90,104
    };
    
    const static u16 DBTab_Video_60Hz[AE_TAB_ENTRY_NUM+1]={
              0,
           1048,         937,         848,         774,        1371,        1187,
           1985,        1622,        1047,         937,           0,         574,
           1048,         937,        1048,         574,        1371,        1188,
           1047,         937,         848,        1486,         659,        1188,
           1047,         937,        1243,         742,        1320,        1149,
           1018,        1127,        1001,         899,        1123,         862,
           1032,        1034,        1024,        1004,        1057,         945,
           1048,         937,         848,        1485,         660,        1222,
           1047,         937,         848,        1486,         659,        1188,
           1047,         937,        1244,         741,        1008,        1188,
           1047,         938,        1243,         741,        1008,        1188,
           1048,         937,         847,        1486,         659,        1188,
           1048,         937,        1243,         741,        1008,        1188,
           1048,         937,        1047,        1114,         989,        1030,
           1048,         937,         848,         957,        1188,           0
      };
   #endif 
   
      //-------------------------------Video Mode,50Hz----------------------------------------------------//
   #if 0   //Min framerate=20Hz
      const static u16 SW_Video_50Hz[AE_TAB_ENTRY_NUM]={
              1,     1,     1,     1,     1,    1,
              2,     2,     2,     3,     3,    4,
              5,     5,     6,     6,     8,    9,
             10,    11,    13,    14,    16,   18,
             19,    21,    24,    27,    30,   34,
             39,    44,    49,    55,    62,   69,
             78,    88,    98,   110,   124,  139,
            155,   155,   155,   155,   155,  155,
            310,   310,   310,   465,   465,  465,
            620,   620,   775,   775,   775,  775,
            775,   775,   775,   775,   775,  775,
            775,   775,   775,   775,   775,  775,
            775,   775,   775,   775,   775,  775,
            775,   775,   775,   775,   775,  775,
            775,   775,   775,   775,   775,  775 
    };
    
    const static u8 AG_Video_50Hz[AE_TAB_ENTRY_NUM]={  
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,   8,
          8,   9,   8,   9,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,  10,
          8,   9,   8,   9,  10,  11,
         13,  15,  16,  18,  21,  23,
         26,  29,  32,  18,  20,  23,
         26,  29,  32,  32,  32,  32,
         32,  32,  32,  32,  32,  32,
         32,  32,  32,  32,  32,  32    
    };
    
    const static u8 MUL_Video_50Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1
    };
    
    const static u8 DG_Video_50Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 1, 2, 3,
          5, 7, 9,11,13,15,
         18,21,25,29,33,38
    };
    
    const static u16 DBTab_Video_50Hz[AE_TAB_ENTRY_NUM+1]={
        0,
        1048,         937,         848,        1486,         659,        1187,        
        1048,         937,        1622,        1047,        1511,        1985,        
        1048,         574,        1047,        1512,        1047,         937,         
         848,        1486,         659,        1188,        1047,         481,         
         890,        1188,        1048,         937,        1113,        1220,        
        1073,         957,        1028,        1065,         952,        1090,        
        1073,         958,        1027,        1066,        1015,         969,        
        1048,         937,         848,        1486,         659,        1187,
        1048,         937,        1622,        1047,         937,         574,        
        1048,         937,        1048,         937,         848,        1485,        
        1273,         574,        1048,        1371,         809,        1091,         
         971,         875,        1048,         937,        1243,        1091,         
         971,         875,        1048,         937,         848,        1486,        
        1273,        1113,         989,         890,         809,        1091,         
         971,        1149,        1018,         913,        1023,           0
      };
    #endif

    #if 0 ////Min framerate=25Hz
    const static u16 SW_Video_50Hz[AE_TAB_ENTRY_NUM]={
              1,     1,     1,     1,     1,    1, 
              2,     2,     2,     3,     3,    4,
              5,     5,     6,     6,     8,    9, 
             10,    11,    13,    14,    16,   18, 
             19,    21,    24,    27,    30,   34, 
             39,    44,    49,    55,    62,   69, 
             78,    88,    98,   110,   124,  139, 
            155,   155,   155,   155,   155,  155, 
            310,   310,   310,   465,   465,  465, 
            620,   620,   620,   620,   620,  620, 
            620,   620,   620,   620,   620,  620, 
            620,   620,   620,   620,   620,  620, 
            620,   620,   620,   620,   620,  620, 
            620,   620,   620,   620,   620,  620, 
            620,   620,   620,   620,   620,  620 
    };
    
    const static u8 AG_Video_50Hz[AE_TAB_ENTRY_NUM]={  
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,   8,
          8,   9,   8,   9,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,  10,
          8,   9,  10,  11,  13,  14,
         16,  18,  20,  23,  25,  28,
         32,  18,  20,  23,  25,  28,
         32,  32,  32,  32,  32,  32,
         32,  32,  32,  32,  32,  32,
         32,  32,  32,  32,  32,  32 
    };
    
    const static u8 MUL_Video_50Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1
    };
    
    const static u8 DG_Video_50Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 2, 3, 5, 6,
          8,10,12,15,17,20,
         24,28,32,37,43,49
    };
    
    const static u16 DBTab_Video_50Hz[AE_TAB_ENTRY_NUM+1]={
           0,
        1048,         937,         848,        1486,         659,        1187,                                                                                                                                    
        1048,         937,        1622,        1047,        1511,        1985,                                                                                                                                       
        1048,         574,        1047,        1512,        1047,         937,                                             
         848,        1486,         659,        1188,        1047,         481,                                             
         890,        1188,        1048,         937,        1113,        1220,                                             
        1073,         957,        1028,        1065,         952,        1090,
        1073,         958,        1027,        1066,        1015,         969,                                                      
        1048,         937,         848,        1486,         659,        1187,                                                      
        1048,         937,        1622,        1047,         937,         574,                                                           
        1048,         937,         848,        1486,         659,        1188,                                                
        1047,         937,        1243,         742,        1008,        1188,                                                             
        1047,         937,        1243,         742,        1008,        1188,                                                                
        1047,         937,         848,        1486,         659,        1188,                                                   
        1047,         938,        1243,         741,        1008,        1188,                                             
        1048,         937,        1047,        1114,         989,           0
      };
    #endif

    #if 1 ////Min framerate=50Hz
    const static u16 SW_Video_50Hz[AE_TAB_ENTRY_NUM]={
              1,     1,     1,     1,     1,    1,
              2,     2,     2,     3,     3,    4,
              5,     5,     6,     6,     8,    9,
             10,    11,    13,    14,    16,   18,
             19,    21,    24,    27,    30,   34,
             39,    44,    49,    55,    62,   69,
             78,    88,    98,   110,   124,  139,
            155,   155,   155,   155,   155,  155,
            310,   310,   310,   310,   310,  310,
            310,   310,   310,   310,   310,  310,
            310,   310,   310,   310,   310,  310,
            310,   310,   310,   310,   310,  310,
            310,   310,   310,   310,   310,  310,
            310,   310,   310,   310,   310,  310,
            310,   310,   310,   310,   310,  310
    };
    
    const static u8 AG_Video_50Hz[AE_TAB_ENTRY_NUM]={  
          8,   9,  10,  11,  13,  14,
          8,   9,  10,   8,   9,   8,
          8,   9,   8,   9,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   8,   8,   8,   8,   8,
          8,   9,  10,  11,  13,  14,
          8,   9,  10,  11,  13,  14,
         16,  18,  20,  23,  25,  28,
         32,  18,  20,  23,  25,  28,
         32,  32,  32,  32,  32,  32,
         32,  32,  32,  32,  32,  32,
         32,  32,  32,  32,  32,  32,
         32,  32,  32,  32,  32,  32 
    };
    
    const static u8 MUL_Video_50Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1,
          1, 1, 1, 1, 1, 1
    };
    
    const static u8 DG_Video_50Hz[AE_TAB_ENTRY_NUM]={  
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 0, 0, 0, 0, 0,
          0, 1, 2, 3, 5, 6,
          8,10,12,15,17,20,
         24,28,32,37,43,49,
         56,64,72,80,90,104
    };
    
    const static u16 DBTab_Video_50Hz[AE_TAB_ENTRY_NUM+1]={
           0,
        1048,         937,         848,        1486,         659,        1187,
        1048,         937,        1622,        1047,        1511,        1985,
        1048,         574,        1047,        1512,        1047,         937,
         848,        1486,         659,        1188,        1047,         481,
         890,        1188,        1048,         937,        1113,        1220,
        1073,         957,        1028,        1065,         952,        1090,
        1073,         958,        1027,        1066,        1015,         969,
        1048,         937,         848,        1486,         659,        1187,
        1048,         937,         848,        1486,         659,        1187,
        1048,         937,        1243,         742,        1008,        1188,
        1047,         937,        1243,         742,        1008,        1188,
        1047,         937,         848,        1486,         659,        1188,
        1047,         937,        1244,         741,        1008,        1188,
        1047,         938,        1047,        1113,         990,        1030,
        1048,         937,         847,         958,        1187,           0
      };
    #endif
      //-----------------------------------------------------------------------------------//
      for(i=0;i<AE_TAB_ENTRY_NUM;i++)
      {
        AE_EG_Tab_VideoPrev_5060Hz[0][0][i].DB_p        = DBTab_Prev_60Hz[i+1];
        AE_EG_Tab_VideoPrev_5060Hz[0][0][i].DB_n        = DBTab_Prev_60Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[0][0][i].EL          = SW_Prev_60Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[0][0][i].AG.gain     = AG_Prev_60Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[0][0][i].AG.mul      = MUL_Prev_60Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[0][0][i].F_num       = 0;
        AE_EG_Tab_VideoPrev_5060Hz[0][0][i].DG          = DG_Prev_60Hz[i];
      }
      
      for(i=0;i<AE_TAB_ENTRY_NUM;i++)
      {
        AE_EG_Tab_VideoPrev_5060Hz[1][0][i].DB_p        = DBTab_Video_60Hz[i+1];
        AE_EG_Tab_VideoPrev_5060Hz[1][0][i].DB_n        = DBTab_Video_60Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[1][0][i].EL          = SW_Video_60Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[1][0][i].AG.gain     = AG_Video_60Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[1][0][i].AG.mul      = MUL_Video_60Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[1][0][i].F_num       = 0;
        AE_EG_Tab_VideoPrev_5060Hz[1][0][i].DG          = DG_Video_60Hz[i];
      }
      
      for(i=0;i<AE_TAB_ENTRY_NUM;i++)
      {
        AE_EG_Tab_VideoPrev_5060Hz[0][1][i].DB_p        = DBTab_Prev_50Hz[i+1];
        AE_EG_Tab_VideoPrev_5060Hz[0][1][i].DB_n        = DBTab_Prev_50Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[0][1][i].EL          = SW_Prev_50Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[0][1][i].AG.gain     = AG_Prev_50Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[0][1][i].AG.mul      = MUL_Prev_50Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[0][1][i].F_num       = 0;
        AE_EG_Tab_VideoPrev_5060Hz[0][1][i].DG          = DG_Prev_50Hz[i];
      }
      
      for(i=0;i<AE_TAB_ENTRY_NUM;i++)
      {
        AE_EG_Tab_VideoPrev_5060Hz[1][1][i].DB_p        = DBTab_Video_50Hz[i+1];
        AE_EG_Tab_VideoPrev_5060Hz[1][1][i].DB_n        = DBTab_Video_50Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[1][1][i].EL          = SW_Video_50Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[1][1][i].AG.gain     = AG_Video_50Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[1][1][i].AG.mul      = MUL_Video_50Hz[i];
        AE_EG_Tab_VideoPrev_5060Hz[1][1][i].F_num       = 0;
        AE_EG_Tab_VideoPrev_5060Hz[1][1][i].DG          = DG_Video_50Hz[i];
      }


}

/* SW 0107 E */

void SetSensorSW(u16 EL)
{
    i2cWrite_SENSOR(0x9,EL);
}

void SetSensor_DigitalGain(u8 ggain)//set globe Digital gain
{
    u8 i;
    u16 data;
    if(ggain > 127)
    {
        ggain=127;
        DEBUG_SIU("Gain : Not a valid value\n");
    }
    i2cRead_SENSOR(0x35, &data);
    i2cWrite_SENSOR(0x35, (data&0x00FF)|(ggain<<8));

}

void SetSensor_AnalogGain(Analog_Gain ggain)//set globe Analog gain
{
    u8 i;
    u16 data;
    if(ggain.gain > 63)
    {
        ggain.gain=63;
        DEBUG_SIU("Gain : Not a valid value\n");
    }

    i2cRead_SENSOR(0x35, &data);
    i2cWrite_SENSOR( 0x35, (data & 0xff00)|(ggain.gain |(ggain.mul<<6)) );
}

void siuAdjustAE(u16 cur_index)
{
    u32 EL;
    u8  DG;
    u16 data;
    Analog_Gain AG;

    //Inital state in Preview mode
    
    if(siuOpMode == SIUMODE_MPEGAVI)
    {
        if(cur_index>AEVideoCurSet_Max)
            cur_index=AEVideoCurSet_Max;
        EL  = (AE_EG_Tab_VideoPrev_5060Hz[1][AE_Flicker_50_60_sel][cur_index].EL * SW_VIDEOCLIP2PREVIEW_RATION + 500)/ 1000;
        AG  = AE_EG_Tab_VideoPrev_5060Hz[1][AE_Flicker_50_60_sel][cur_index].AG;
        DG  = AE_EG_Tab_VideoPrev_5060Hz[1][AE_Flicker_50_60_sel][cur_index].DG;
    }
    else 
    {
        EL  = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][cur_index].EL;
        AG  = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][cur_index].AG;
        DG  = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][cur_index].DG;
    }


    SetSensorSW(EL);
    SetSensor_AnalogGain(AG);
    SetSensor_DigitalGain(DG);

}


s32 siuPreviewInit(u8 zoomFactor,u8 mode)
{
    while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
    
    //initialize sensor
    rfiuVideoInFrameRate=siuSensorInit(mode,zoomFactor);
    
    // initialize the siu module
    // reset the SIU module
    //SiuSensCtrl = SIU_RESET;
    
    // Set start address of raw buffer  
    SiuRawAddr =    (u32)siuRawBuf;

    // Set sensor control 
    SiuSensCtrl =   
                SIU_NORMAL |
                SIU_ENA |                               //enable SIU module
                SIU_SLAVE |                             //For CMOS sensor
                SIU_VSYNC_ACT_LO |                      //Vsync active low
                SIU_HSYNC_ACT_LO |                      //Hsync active low
                SIU_DEF_PIX_DISA |
                SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
                SIU_INT_ENA_FRAM |                      //Frame end interrupt enaable
                SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
                SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
                SIU_DST_SRAM |                          //SIU to SRAM
         #if(VIDEO_RESOLUTION_SEL== VIDEO_1920x1440DECI3x3TOVGA)
                SIU_PDSEL_3x3 |
         #endif
         #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)          /* Sensor data is not shifted 2 bits. */            
                SIU_DATA_12b |
         #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)      /* Sensor data is shifted 2 bits. */        
                SIU_DATA_10b | 
         #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)      /* Sensor data is shifted 4 bits. */        
                SIU_DATA_8b |                               
         #endif            
                SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
         #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
            (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
                SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		 #else
		        SIU_FRAM_VSYNC |
         #endif
                SIU_OB_COMP_ENA |                      //OB correct enable 
         #if ( (HW_BOARD_OPTION == SALIX_SDV)||(HW_BOARD_OPTION==HX_DH500)/*||(HW_BOARD_OPTION==ULTMOST_SDV)*/)
               SIU_LENS_SHAD_ENA |                    //Lens shading compensation disable
         #else
               SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
         #endif

         #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
               SIU_RGB_GAMMA_ENA |
         #else
               SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
         #endif
               SIU_AE_ENA |                            //AE report enable
               SIU_TEST_PATRN_DISA |                   //Test pattern disable
               SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.

	 #if ( (CHIP_OPTION == CHIP_PA9002D) || (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
         (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
	    SiuDebugSel =SIU_4GAMMATab_EN;
	 #endif

    
    
        OB_B  = SIU_OB_B>>2;
        OB_Gb = SIU_OB_Gb>>2;
        OB_R  = SIU_OB_R>>2;
        OB_Gr = SIU_OB_Gr>>2;

    
        siuSetOpticalBlack(SIU_COMP_B,  SIU_OB_B);
        siuSetOpticalBlack(SIU_COMP_Gb, SIU_OB_Gb);
        siuSetOpticalBlack(SIU_COMP_Gr, SIU_OB_Gr);
        siuSetOpticalBlack(SIU_COMP_R,  SIU_OB_R);
            
    // Set Digital Gain: 可當作AWB gain.        
    #if SIU_AWBCALIB_ENA
        siuSetDigitalGain(SIU_COMP_B,  siuWBComp_RBRatio[1]);       
        siuSetDigitalGain(SIU_COMP_Gb, 0x0100); 
        siuSetDigitalGain(SIU_COMP_Gr, 0x0100); 
        siuSetDigitalGain(SIU_COMP_R,  siuWBComp_RBRatio[0]);
    #else   
    siuSetDigitalGain(SIU_COMP_B,  0x0100);     
    siuSetDigitalGain(SIU_COMP_Gb, 0x0100); 
    siuSetDigitalGain(SIU_COMP_Gr, 0x0100); 
    siuSetDigitalGain(SIU_COMP_R,  0x0100); 
    #endif

    // Set Lens Shading 
    siuSetLensShading(&lensShading_Preview,100); 

    // Set Defect Pixel Table 
    SiuDefPixTbl = (u32)Coordinate_SDV1;
    
    // Set RGB pre-Gamma Table
	siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
                    
    // Set AE block weighting
    siuSetAeBlkWeight(siu_dftAeWeight1, &(aeWin.weightacc));

    // Initialize AE parameters
    siuAeInit();
    
    // Set AWB Window
    siuSetAwbWin();
       
    SiuSensCtrl |= SIU_CAPT_ENA;
                
    return 1;   
}

s32 siuVideoClipInit(u8 zoomFactor)
{       
    // initialize the siu module
    // reset the SIU module
    //SiuSensCtrl = SIU_RESET;
    
    // Set start address of raw buffer  
    SiuRawAddr =    (u32)siuRawBuf;

    // Set sensor control 
    SiuSensCtrl =   
                SIU_NORMAL |
                SIU_ENA |                               //enable SIU module
                SIU_SLAVE |                             //For CMOS sensor
                SIU_VSYNC_ACT_LO |                      //Vsync active low
                SIU_HSYNC_ACT_LO |                      //Hsync active low
                SIU_DEF_PIX_DISA |
                SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
                SIU_INT_ENA_FRAM |                      //Frame end interrupt enaable
                SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
                SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
                SIU_DST_SRAM |                          //SIU to SRAM
         #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      // Sensor data is not shifted 2 bits. 
                SIU_DATA_12b |
         #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  // Sensor data is shifted 2 bits.       
                SIU_DATA_10b | 
         #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)      // Sensor data is shifted 4 bits.       
                SIU_DATA_8b |                               
         #endif            
                SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
         #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
            (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
                SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		 #else
		        SIU_FRAM_VSYNC |
         #endif                
		        SIU_OB_COMP_ENA |                      //OB correct enable 
         #if ( (HW_BOARD_OPTION == SALIX_SDV)||(HW_BOARD_OPTION==HX_DH500)/*||(HW_BOARD_OPTION==ULTMOST_SDV)*/)
                SIU_LENS_SHAD_ENA |                    //Lens shading compensation disable
         #else
                SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
         #endif
         
         #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
               SIU_RGB_GAMMA_ENA |
         #else
                SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
         #endif
                SIU_AE_ENA |                            //AE report enable
                SIU_TEST_PATRN_DISA |                   //Test pattern disable
                SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.

	 #if ( (CHIP_OPTION == CHIP_PA9002D) || (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
         (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
	    SiuDebugSel =SIU_4GAMMATab_EN;
	 #endif

    
        OB_B  = SIU_OB_B>>2;
        OB_Gb = SIU_OB_Gb>>2;
        OB_R  = SIU_OB_R>>2;
        OB_Gr = SIU_OB_Gr>>2;

    
        siuSetOpticalBlack(SIU_COMP_B,  SIU_OB_B);
        siuSetOpticalBlack(SIU_COMP_Gb, SIU_OB_Gb);
        siuSetOpticalBlack(SIU_COMP_Gr, SIU_OB_Gr);
        siuSetOpticalBlack(SIU_COMP_R,  SIU_OB_R);
            
    // Set Digital Gain: 可當作AWB gain.        
    #if SIU_AWBCALIB_ENA
        siuSetDigitalGain(SIU_COMP_B,  siuWBComp_RBRatio[1]);       
        siuSetDigitalGain(SIU_COMP_Gb, 0x0100); 
        siuSetDigitalGain(SIU_COMP_Gr, 0x0100); 
        siuSetDigitalGain(SIU_COMP_R,  siuWBComp_RBRatio[0]);
    #else           
	    siuSetDigitalGain(SIU_COMP_B,  0x0100);     
	    siuSetDigitalGain(SIU_COMP_Gb, 0x0100); 
	    siuSetDigitalGain(SIU_COMP_Gr, 0x0100); 
	    siuSetDigitalGain(SIU_COMP_R,  0x0100); 
    #endif

    // Set Lens Shading 
    if(zoomFactor<7) //Zoom x1
       siuSetLensShading(&lensShading_Video,100); 
    else if( zoomFactor<13 ) //Zoom x2
       siuSetLensShading(&lensShading_Video,100);
    else //Zoom x3
       siuSetLensShading(&lensShading_Video,56);

    // Set Defect Pixel Table 
    SiuDefPixTbl = (u32)Coordinate_SDV1;
        
    // Set RGB pre-Gamma Table
	siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
                    
            
    // Set AE block weighting
    siuSetAeBlkWeight(siu_dftAeWeight1, &(aeWin.weightacc));

    // Initialize AE parameters
    siuAeInit();
    
    // Set AWB Window
    siuSetAwbWin();

    SiuSensCtrl |= SIU_CAPT_ENA;
                        
    return 1;   
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
    u16 i;      
    u16 AESnapshotCurSet = AECurSet;
    u16 EL;
    Analog_Gain AG;
    u32 DG;    
    u16 data;
    u16 temp,scale,base,AGgain;
    int FrameRate=30;
    //--------------------------//
    
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))   
   #if(SYS_CPU_CLK_FREQ == 32000000)
       SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x01; //MClk=32/2=16MHz
   #elif(SYS_CPU_CLK_FREQ == 48000000)
       SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x01; //MClk=48/2=24MHz
   #endif
#else   
   #if(VIDEO_RESOLUTION_SEL == VIDEO_VGA_IN_VGA_OUT)
	   #if(SYS_CPU_CLK_FREQ == 96000000)
	     SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x0b; //MClk=288/12=24MHz
	   #elif(SYS_CPU_CLK_FREQ == 108000000)
	     SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x08; //MClk=216/9=24MHz
	   #elif(SYS_CPU_CLK_FREQ == 160000000)
         SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x13; //MClk=480/20=24MHz
       #elif(SYS_CPU_CLK_FREQ == 180000000)
         SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x13; //MClk=540/20=27MHz  
	   #endif
   #else
       #if(SYS_CPU_CLK_FREQ == 96000000)
	     SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x05; //MClk=288/6=48MHz
	   #elif(SYS_CPU_CLK_FREQ == 108000000)
	     SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x03; //MClk=216/4=54MHz
	   #elif(SYS_CPU_CLK_FREQ == 160000000)
         SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x09; //MClk=480/10=48MHz
       #elif(SYS_CPU_CLK_FREQ == 180000000)
         SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x09; //MClk=540/10=54MHz    
	   #endif
   #endif

#endif

#if RFIU_SUPPORT 
   gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_HD;
#endif
    
        
    switch(siuSensorMode)       
    {   
        case SIUMODE_CAPTURE: //Capture mode                       
            i2cWrite_SENSOR(0x01, 54);                          //Row start(Y)
            i2cWrite_SENSOR(0x02, 16);                          //Column start(X)
            i2cWrite_SENSOR(0x03, IMG_MAX_HEIGHT  + 4*4 + 1*4); //Row size:
            i2cWrite_SENSOR(0x04, IMG_MAX_WIDTH  + 4*4 + 1*4); //Column size
            i2cWrite_SENSOR(0x5, 450);                           //Horizontal Blank
            i2cWrite_SENSOR(0x6, 25);                             //Vertical Blank
            i2cWrite_SENSOR(0x0a, 0x8000);                    //Lucian Fixed bug->Invert Pixel clock: match to siu rise-edge latch;
         #if SENSOR_ROW_COL_MIRROR
            i2cWrite_SENSOR(0x20, 0xc040);                      //Row BLC enable, Row/Column mirror
         #else
            i2cWrite_SENSOR(0x20, 0x0040);                      //Row BLC: Black level compensation
         #endif
            i2cWrite_SENSOR(0x22, 0x0000);                  //Row: No skip
            i2cWrite_SENSOR(0x23, 0x0000);                  //Column: No skip
                
            i2cWrite_SENSOR(0x60, 0);                         //G1 offset
            i2cWrite_SENSOR(0x61, 0);                         //G2 offset
            i2cWrite_SENSOR(0x63, 0);                         //R offset
            i2cWrite_SENSOR(0x64, 0);                         //B offset
    
            i2cWrite_SENSOR(0x62, 0);                         //Auto BLC enable     

            //Lucian: 由於PCB 線長特性不同,需要特別調整.
            #if( (HW_BOARD_OPTION==ISP_PA9002D_SYS_256P)|| (HW_BOARD_OPTION==A1016B_FPGA_BOARD) || \
                (HW_BOARD_OPTION==A1016_FPGA_BOARD) || (HW_BOARD_OPTION  == A1018_FPGA_BOARD) || \
                (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
                (HW_BOARD_OPTION  == A1026A_FPGA_BOARD)) 
                i2cWrite_SENSOR(0x07, 0x1f82);                //Set slew rate: 7 (0~7,Low/High)
            #else
                i2cWrite_SENSOR(0x07, 0x02);                //Set slew rate: 0 (0~7)
            #endif
        
            //------Lucian: 判斷閃光燈打開時機,EL/AG/DG之設定-----//
            if(siuFlashLightMode==SIU_FLASH_LIGHT_ALWAYS_ON)
            {
                siuFlashLightShoot = 1;
                siuShootMainFlash=1;
                if(AE_Flicker_50_60_sel)//50Hz
                    EL = 2066;  // CMOS limit: EL > (H+VB)=(1938+26)=1964, (15/100*1000000/72.583=2066)
                else //60Hz
                    EL = 2066;  // CMOS limit: EL > (H+VB)=(1938+26)=1964, (18/120*1000000/72.583=2066)
                
                AG.gain = 8;
                AG.mul = 0;
                DG = 0;         
            }
            else if ((AECurSet >= AECapCurSet_Max) && (siuFlashLightMode == SIU_FLASH_LIGHT_AUTOMATIC)) 
            {   //Auto 閃光燈打開時.
                siuFlashLightShoot = 1;
                siuShootMainFlash=1;
                if(AE_Flicker_50_60_sel)//50Hz
                    EL = 2066;  // CMOS limit: EL > (H+VB)=(1938+26)=1964, (15/100*1000000/72.583=2066)
                else //60Hz
                    EL = 2066;  // CMOS limit: EL > (H+VB)=(1938+26)=1964, (18/120*1000000/72.583=2066)
                
                AG.gain = 8;
                AG.mul = 0;
                DG = 0;         
            }
            else if ((AECurSet > 66) && (siuFlashLightMode == SIU_FLASH_LIGHT_AUTOMATIC)) 
            {   //Auto 閃光燈打開時.
                siuFlashLightShoot = 1;
                siuShootMainFlash=1;
                if(AE_Flicker_50_60_sel)//50Hz
                    EL = 2066;  // CMOS limit: EL > (H+VB)=(1938+26)=1964, (15/100*1000000/72.583=2066)
                else //60Hz
                    EL = 2066;  // CMOS limit: EL > (H+VB)=(1938+26)=1964, (18/120*1000000/72.583=2066)
                AG.gain = 8;
                AG.mul = 0;
                DG = 0;         
            }
            else //No flash light
            {
                siuFlashLightShoot = 0;
                siuShootMainFlash=0;
                
                if (AESnapshotCurSet > AECapCurSet_Max) //限制最大曝光時間.
                  AESnapshotCurSet = AECapCurSet_Max;
                
                EL = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][AESnapshotCurSet].EL;
                AG.gain = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][AESnapshotCurSet].AG.gain;
                AG.mul = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][AESnapshotCurSet].AG.mul;
                DG = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][AESnapshotCurSet].DG;      

                //------Scene Mode, ISO adjustment-------//
                if( (siuSceneMode_sel == SIU_SCENEMODE_AUTO) || (siuSceneMode_sel == SIU_SCENEMODE_LANDSCAPE) )
                {
                   if(siuISO_sel == SIU_ISO_100)
                   {
                      if(AESnapshotCurSet >= 60)
                      {
                         temp=AESnapshotCurSet-54;
                         scale= temp / Gain_Level;
                         base = temp % Gain_Level;

                         EL = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][54+base].EL << scale;
                         AG.gain = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][54+base].AG.gain;
                         AG.mul = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][54+base].AG.mul;
                      }
                      
                   }
                   else if(siuISO_sel == SIU_ISO_200)
                   {
                      if(AESnapshotCurSet >= 66)
                      {
                         temp=AESnapshotCurSet-60;
                         scale= temp / Gain_Level;
                         base = temp % Gain_Level;

                         EL = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][60+base].EL << scale;
                         AG.gain = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][60+base].AG.gain;
                         AG.mul = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][60+base].AG.mul;
                      }
                      else if( (AESnapshotCurSet >= 54) && (AESnapshotCurSet < 60) )
                      {
                         base = AESnapshotCurSet % Gain_Level;
                         
                         EL = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][48+base].EL;
                         AG.gain = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][48+base].AG.gain * 2;
                         AG.mul = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][48+base].AG.mul;
                      }
                      else if( (AESnapshotCurSet >= 48) && (AESnapshotCurSet < 54) )
                      {
                         base = AESnapshotCurSet % Gain_Level;
                         
                         EL = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][42+base].EL;
                         AG.gain = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][42+base].AG.gain*2;
                         AG.mul = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][42+base].AG.mul;
                      }
                      
                   }
                   else //Auto mode
                   {

                   }
                   
                }
                else //Sport mode: Max exp. time=1/120 or 1/100 sec.(暫不啟動Digital gain.)
                {
                   if(AESnapshotCurSet >= 48)
                   {
                       temp=AESnapshotCurSet-42;
                       scale= temp / Gain_Level;
                       base = temp % Gain_Level;

                       EL = AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][42+base].EL;
                       AGgain= AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][42+base].AG.gain *
                               (AE_EG_Tab_VideoPrev_5060Hz[0][AE_Flicker_50_60_sel][42+base].AG.mul+1);
                       AGgain =AGgain<<scale;
                       if(AGgain >= 64)
                       {
                          AG.gain=32;
                          AG.mul =1;
                       }
                       else
                       {
                          if(AGgain>32)
                          {
                            AG.gain=(AGgain+1)/2;
                            AG.mul =1;
                          }
                          else
                          {
                            AG.gain=AGgain;
                            AG.mul =0;
                          }
                       }
                       
                   }
                }
                //---------------------------------------//
                
                EL= (EL*SW_CAPTURE2PREVIEW_RATION+500)/1000; //SW for Capture mode和Preview 有比例關係.

            }  
            //---------------------//
             
            SetSensorSW(EL);
            SetSensor_AnalogGain(AG);
            SetSensor_DigitalGain(DG);
            break;
            
        case SIUMODE_PREVIEW:  //Preview mode
        case SIUMODE_PREVIEW_ZOOM:
        case SIUMODE_MPEGAVI:  //Video clip mode
        case SIUMODE_MPEGAVI_ZOOM:
            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
            {
                i2cWrite_SENSOR(0x01, 54+(IMG_MAX_HEIGHT-720*2)/2);       //Row start(Y)
                i2cWrite_SENSOR(0x02, 16+(IMG_MAX_WIDTH-1280*2)/2);        //Column start(X)
                i2cWrite_SENSOR(0x03, 720*2  +6*2 + 1*2);                        //Row size(Y)
                i2cWrite_SENSOR(0x04, 1280*2 +4*2 + 1*2);                    //Column size(X)
                i2cWrite_SENSOR(0x5, 450);                                //Horizontal Blank: increase exposure and decrease frame rate
                i2cWrite_SENSOR(0x6, 0);                                //Vertical Blank
                i2cWrite_SENSOR(0x0a, 0x8000);                          //Lucian Fixed bug->Invert Pixel clock: match to siu rise-edge latch;
                
                i2cWrite_SENSOR(0x20, 0x40);                            //Row BLC
                i2cWrite_SENSOR(0x22, 0x0001);                          //Row skip x1 (No skip)
                i2cWrite_SENSOR(0x23, 0x0001);                          //Column skip x1 (No skip)
            }
            else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
            {
            #if(VIDEO_RESOLUTION_SEL == VIDEO_HD_IN_VGA_OUT)
                i2cWrite_SENSOR(0x01, 54+(IMG_MAX_HEIGHT-720*2)/2);       //Row start(Y)
                i2cWrite_SENSOR(0x02, 16+(IMG_MAX_WIDTH-1280*2)/2);        //Column start(X)
                i2cWrite_SENSOR(0x03, 720*2  +6*2 + 1*2);                        //Row size(Y)
                i2cWrite_SENSOR(0x04, 1280*2 +4*2 + 1*2);                    //Column size(X)
                i2cWrite_SENSOR(0x5, 450);                                //Horizontal Blank: increase exposure and decrease frame rate
                i2cWrite_SENSOR(0x6, 0);                                //Vertical Blank
                i2cWrite_SENSOR(0x0a, 0x8000);                          //Lucian Fixed bug->Invert Pixel clock: match to siu rise-edge latch;
                
                i2cWrite_SENSOR(0x20, 0x40);                            //Row BLC
                i2cWrite_SENSOR(0x22, 0x0001);                          //Row skip x1 (No skip)
                i2cWrite_SENSOR(0x23, 0x0001);                          //Column skip x1 (No skip)
                
            #elif(VIDEO_RESOLUTION_SEL== VIDEO_1920x1440DECI3x3TOVGA)
                i2cWrite_SENSOR(0x01, 54+(IMG_MAX_HEIGHT-480*3)/2);       //Row start(Y)
                i2cWrite_SENSOR(0x02, 16+(IMG_MAX_WIDTH-640*3)/2);        //Column start(X)
                i2cWrite_SENSOR(0x03, 480*3 +6*3 + 1*3);                        //Row size(Y)
                i2cWrite_SENSOR(0x04, 640*3 +4*3 + 1*3);                    //Column size(X)
                i2cWrite_SENSOR(0x5, 450);                                //Horizontal Blank: increase exposure and decrease frame rate
                i2cWrite_SENSOR(0x6, 0);                                //Vertical Blank
                i2cWrite_SENSOR(0x0a, 0x8000);                          //Lucian Fixed bug->Invert Pixel clock: match to siu rise-edge latch;
                
                i2cWrite_SENSOR(0x20, 0x40);                            //Row BLC
                i2cWrite_SENSOR(0x22, 0x0000);                          //Row skip x1 (No skip)
                i2cWrite_SENSOR(0x23, 0x0000);                          //Column skip x1 (No skip)             
            #else  //VGA size
                i2cWrite_SENSOR(0x01, 54);                        //Row start(Y)
                i2cWrite_SENSOR(0x02, 16);                        //Column start(X)
                i2cWrite_SENSOR(0x03, IMG_MAX_HEIGHT  +6*4 + 1*4);  //Row size
                i2cWrite_SENSOR(0x04, IMG_MAX_WIDTH   +4*4 + 1*4);  //Column size            
                i2cWrite_SENSOR(0x5, 450);                        //Horizontal Blank: increase exposure and decrease frame rate
                i2cWrite_SENSOR(0x6, 25);                         //Vertical Blank
                i2cWrite_SENSOR(0x0a, 0x8000);                    //Lucian Fixed bug->Invert Pixel clock: match to siu rise-edge latch;
                i2cWrite_SENSOR(0x22, 0x0003);                    //Row skip x4
                i2cWrite_SENSOR(0x23, 0x0003);                    //Column skip x4  
            #endif
            }
            
        #if SENSOR_ROW_COL_MIRROR
            i2cWrite_SENSOR(0x20, 0xc040);                    //Row BLC enable, Row/Column mirror
        #else
            i2cWrite_SENSOR(0x20, 0x0040);                    //Row BLC: Black level compensation
        #endif                
            i2cWrite_SENSOR(0x60, 0);                         //G1 offset
            i2cWrite_SENSOR(0x61, 0);                         //G2 offset
            i2cWrite_SENSOR(0x63, 0);                         //R offset
            i2cWrite_SENSOR(0x64, 0);                         //B offset
            i2cWrite_SENSOR(0x62, 0);                         //Auto BLC enable 
            i2cWrite_SENSOR(0x07, 2);             
            i2cWrite_SENSOR(0x0b, 0x01);                      //Lucian: Sensor Restart.

        break;

    }
            
    /*
        Lucian: 當為VideoClip Zoom 模式時, 因錄影還在進行中,不可reset Mpeg control variable.
    */
    if(siuSensorMode != SIUMODE_MPEGAVI_ZOOM)
    {
        mp4_avifrmcnt = 0;
        isu_avifrmcnt = 0;  //0;
        avi_FrameSkip = 0;  
        unMp4SkipFrameCount = 0; /* mh@2006/11/22: for time stamp control */
    }
   
    return FrameRate;
}

s32 siuAeInit(void)  
{   
    siuSetAETbl();
    siuAdjustAE(AECurSet);  
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
    u8 flashratio = 0;
    u8 flashremain = 0;
#if FINE_TIME_STAMP
    s32 IISTimeOffsetSIU, TimeOffset;
    u32 IISTime1;
#endif

#if (DEBUG_SIU_FRAM_STR_INTR_USE_LED6 && ((HW_BOARD_OPTION == A1013_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016_FPGA_BOARD) || (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)|| \
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD)))
    static u32 Debug_count=0;
#endif

    intStat = SiuIntStat;
    
    if (intStat & SIU_INT_STAT_FIFO_OVERF)
    {
        /* Raw data FIFO overflow */
        DEBUG_SIU("\nSIU Data Overflow\n");
    }
    
    if (intStat & SIU_INT_STAT_FRAM)
    {
        siuAFCount++;     //lisa 070514     
        if (siuAeEnable == 1)
            siuFrameCount++;
        else
            siuFrameCount = 0;
            

        if ( 
             ((siuOpMode == SIUMODE_PREVIEW) || (siuOpMode == SIUMODE_MPEGAVI)) && 
             (siuFrameCount == AE_AGC_SYNC_FRAMENUM) && 
             (siuAeSyncAgcSwControl == 1)
           )
        {
            siuAeSyncAgcSwControl = 0;
        #if AE_SYNC_AGC_SW_POLEN
        
        #else
            OSSemPost(siuSemEvt);
        #endif
        }
        //---------閃光燈控制 Start-----------//            
        #if(FLASH_LIGHT_SEL == FLASH_LIGHT_SRC)
            if ( 
                 (siuOpMode == SIUMODE_CAPTURE) && 
                 (siuFlashLightShoot == 1) && 
                 (siuFrameCount == siuFlashGoFrameCount) &&
                 (siuPreFlashCount == 3)
                 
               )
            {  //打主閃             
                //siuFlashLightReadyToGo = 1;
                if(siuShootMainFlash==1)
                {
                    gpioSetLevel(0, 8, 0); //Flash-light-recharge is off.
                    Gpio0Level |= (0x80);               
                    Gpio0Level &= ~(0x80);              
                }
            }
        
        #elif(FLASH_LIGHT_SEL == FLASH_LIGHT_PWM)       
        if ((siuOpMode == SIUMODE_CAPTURE) && (siuFlashLightShoot == 1) && (siuPreFlashCount != 0))
        {
            if ((siuOpMode == SIUMODE_CAPTURE) && (siuFlashLightShoot == 1) && (siuPreFlashCount == 1))
            { //第一次預閃
                if (siuFrameCount == siuFlashGoFrameCount)
                    siuFlashLightReadyToGo = 1;
                else if (siuFrameCount == siuFlashGetYFrameCount)
                {
                    siuGetFlashLightYavg(0);
                    siuYavg[0] = ae_YavgValue;
                    siuFlashLightReadyToGetYavg = 1;
                }
            }
            else if ((siuOpMode == SIUMODE_CAPTURE) && (siuFlashLightShoot == 1) && (siuPreFlashCount == 2))
            {   //第二次預閃
                if (siuFrameCount == siuFlashGoFrameCount)
                    siuFlashLightReadyToGo = 1;
                else if (siuFrameCount == siuFlashGetYFrameCount)
                {
                    siuGetFlashLightYavg(1);
                    siuYavg[1] = ae_YavgValue;

                    if (ae_YavgValue == 0) //若所得Y值仍為零,則打光所有能量
                        siuFlashLightCount = (0xFA * 30);
                    else if (ae_YavgValue <= siuFlashLightYtarget)
                    {   //亮度仍不足,做外插.
                        flashratio = siuFlashLightYtarget / siuYavg[1];
                        flashratio = ((siuYavg[1] >= ((siuFlashLightYtarget * (2 * flashratio + 1))/2)) ? (flashratio+1) : flashratio);
                        siuFlashLightCount = (siuInterTimeCount * siuRatioRmsTbl[flashratio]);
                    }
                    else if (ae_YavgValue > siuFlashLightYtarget)
                    {   //亮度過多,做內插. 
                        siuFlashLightCount = siuFlashLightInterPolationTbl[(siuYavg[1] - siuFlashLightYtarget)];
                    }
                    siuFlashLightReadyToGetYavg = 1;
                }
            }
            else if ((siuOpMode == SIUMODE_CAPTURE) && (siuFlashLightShoot == 1) && (siuPreFlashCount == 3))
            {  //打主閃
                if (siuFrameCount == siuFlashGoFrameCount)
                    siuFlashLightReadyToGo = 1;
            }
        }       
        #endif  
        //---------閃光燈控制 End-----------//
        
        frameAct_Thd = (siuOpMode == SIUMODE_MPEGAVI)?  AECNT_VideoCap : AECNT_PREVIEW;
        if (siuFrameCount > frameAct_Thd)
        {
            /* signal event semaphore */
            switch (siuOpMode) 
            {
                                case SIUMODE_PREVIEW:
                                        if ((siuPreviewFlag == 1) && (uiDigZoomSetFlag == 0))
                                        { //uiDigZoomSetFlag: 避免Digital Zoom在調整sensor window size時, 啟動AE control.
                                                siuPreviewFlag = 0;
                                                OSSemPost(siuSemEvt);
                                        }        
                                        break;
                                case SIUMODE_CAPTURE:
                                        if ((siuCaptureFlag == 1) && (siuPreFlashCount == 3) && (siuFrameCount == siuFlashGetYFrameCount))                                        
                                        {
                                                siuGetFlashLightYavg(2);
                                                siuYavg[2] = ae_YavgValue;
                        
                                                SiuSensCtrl &= SIU_CAPOEFINT_DISA; // disable siu interrupt and data output.
                                                siuCaptureFlag = 0;
                                                OSSemPost(siuCapEvt);
                                        }        
                                        break;
                                case SIUMODE_MPEGAVI:
                                        if ((siuPreviewFlag == 1) && (uiDigZoomSetFlag == 0))
                                        { //uiDigZoomSetFlag: 避免Digital Zoom在調整sensor window size時, 啟動AE control.
                                            siuPreviewFlag = 0;
                                            OSSemPost(siuSemEvt);
                                        }
                                        break;
            }
        }
        
        //if (siuOpMode == SIUMODE_MPEGAVI)
        if (siuOpMode == SIUMODE_MPEGAVI && sysCheckZoomRun_flag == 0)
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
                //frame skip
                SiuSensCtrl &= (~SIU_CAPT_ENA);
                avi_FrameSkip = 1;
                unMp4SkipFrameCount++; /* mh@2006/11/22: for time stamp control */
                //DEBUG_SIU(".");
                DEBUG_SIU(".");
#if MPEG_DEBUG_ENA
                // for PA9002D debug
                gpioDebug1(isu_avifrmcnt - mp4_avifrmcnt);
#endif
            }   
            else 
            {
                SiuSensCtrl |= SIU_CAPT_ENA;
                avi_FrameSkip = 0;
        
            #if (MPEG4_CONTAINER_OPTION == MPEG4_CONTAINER_MP4)
                unMp4FrameDuration[isu_avifrmcnt % SIU_FRAME_DURATION_NUM]  = 33 * (1 + unMp4SkipFrameCount);  /* Peter 070403 */
            #endif
                unMp4SkipFrameCount = 0;   
#if MPEG_DEBUG_ENA
                // for PA9002D debug
                gpioDebug1(isu_avifrmcnt - mp4_avifrmcnt);
                //gpioSetLevel(1, 26, 0 );
#endif
            }
        #endif
        }   
        
        if(sysCheckZoomRun_flag) {
            avi_FrameSkip   = 0;
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
    
#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
    if (intStat & SIU_INT_STAT_FRAME_STR)
    {
    }
#endif

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
s32 siuSetLensShading(SIU_LENS_SHADING* pLensShading,u32 LS_gain_x100)
{
    SiuLensCornerX2 = (pLensShading->cornerX2) | (((u32)pLensShading->shading_B_Gain*LS_gain_x100/100)<<23);
    SiuLensCornerY2 = (pLensShading->cornerY2) | (((u32)pLensShading->shading_R_Gain*LS_gain_x100/100)<<23);
    SiuLensCentOffs = (((u32)pLensShading->centerOffset.x) << SIU_LENS_CENT_X_SHFT) | 
              (((u32)pLensShading->centerOffset.y) << SIU_LENS_CENT_Y_SHFT);
    SiuLensCompGain = ((u32)pLensShading->shading_G_Gain*LS_gain_x100/100) | 
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
	u32 i;
	volatile unsigned *B_Gamma  = &(Siu_B_Gamma1);
    volatile unsigned *Gr_Gamma = &(Siu_Gr_Gamma1);
    volatile unsigned *Gb_Gamma = &(Siu_Gb_Gamma1);
    volatile unsigned *R_Gamma  = &(Siu_R_Gamma1);

	
	for (i = 1; i < SIU_B_GAMMA_TBL_COUNT-1; i++)
	{
		*B_Gamma++  = (((u32)pGammaTable_X[i]) << SIU_B_GAMMA_X_SHFT) | 
			          (((u32)pGammaTable_B[i]) << SIU_B_GAMMA_Y_SHFT);
	}
    *B_Gamma= (((u32)pGammaTable_B[0]) << SIU_B_GAMMA_X_SHFT) | 
			  (((u32)pGammaTable_B[16]) << SIU_B_GAMMA_Y_SHFT);
    
#if (CHIP_OPTION == CHIP_PA9002D)
    Siu_GbRGb_Gamma5=pGammaTable_Gb[16] | (pGammaTable_R[16]<<8) | (pGammaTable_Gr[16]<<16);
    
    for(i=0 ; i<4 ; i++)
    {
        *Gr_Gamma++  = pGammaTable_Gr[0] | (pGammaTable_Gr[1]<<8) | (pGammaTable_Gr[2]<<16) | (pGammaTable_Gr[3]<<24);
        pGammaTable_Gr +=4;
    }

    for(i=0 ; i<4 ; i++)
    {
        *Gb_Gamma++  = pGammaTable_Gb[0] | (pGammaTable_Gb[1]<<8) | (pGammaTable_Gb[2]<<16) | (pGammaTable_Gb[3]<<24);
        pGammaTable_Gb +=4;
    }

    for(i=0 ; i<4 ; i++)
    {
        *R_Gamma++  = pGammaTable_R[0] | (pGammaTable_R[1]<<8) | (pGammaTable_R[2]<<16) | (pGammaTable_R[3]<<24);
        pGammaTable_R +=4;
    }

#endif

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

    aeWin.histLoBond    = 0x18; /* histLoBond, unit:4 */
    aeWin.histHiBond    = 0xd2; /* histLoBond, unit:4 */            
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
        *pWinYsum = ( *(aeWin + winRegNum) & SIU_AE_SUM_WIN_ODD_MASK) >> SIU_AE_SUM_WIN_ODD_SHFT;          
    else /* winEven */
        *pWinYsum = ( *(aeWin + winRegNum) & SIU_AE_SUM_WIN_EVEN_MASK) >> SIU_AE_SUM_WIN_EVEN_SHFT;
                           
    return 1;
}   
#if (AE_report==AE_report_Soft)
/*

Routine Description:

    Get AE Window Y  average by software.

Arguments:

    siuBlkWeight - The weight of  AE window.

    
Return Value:

    0 - Failure.
    1 - Success.

*/
//-----AE_report------//by lisa_0425_S
//減少時間1.H方向不用每次讀取2.在做加法時block切換移到外面
//目前量到時間為22ms
//切割水平和垂直數目最好基數如13:11或11:7,且切割後每個block的size最好為2的次幕,用shift比較有效率

s32 siuGetAeYavg_Soft_Panel(u8 *siuBlkWeight)
{
    unsigned char *img_ptr,*pp;
    unsigned int   in_data;
    unsigned int ae_WinSize;
    unsigned int blk_vidx, blk_hidx, nextblk_vidx, nextblk_hidx;
    unsigned int i, j,k;
    unsigned int temp_Y;
    unsigned int aeweight_sum=0;
    unsigned int Sum;
    s32 ae_WeightedYsum=0;
    u32 AE_blk_YSum[AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT]={0}; 
    
       
    blk_vidx = 0;
    nextblk_vidx= (AE_WIN_HEIGHT/AE_HSIZE_SEGMENT); 

    i=((isu_idufrmcnt-1) % 3);
    if(i==0)
        img_ptr=(u8*) PKBuf0;
    else if (i==1)
        img_ptr=(u8*) PKBuf1;
    else
        img_ptr=(u8*) PKBuf2;

    for (j= 0; j < AE_WIN_HEIGHT; j+=AE_DOWN_SCALAR_Y)     
    {
        if (j>= nextblk_vidx )
        {
            blk_vidx++;
            nextblk_vidx += (AE_WIN_HEIGHT/AE_HSIZE_SEGMENT);
        }
        blk_hidx = 0;
        nextblk_hidx =( AE_WIN_WIDTH*2/AE_WSIZE_SEGMENT); 
        Sum=0;
        pp=img_ptr + j*AE_WIN_WIDTH_TV*2;
        for (i=0; i< (AE_WIN_WIDTH*2); i+=(2*AE_DOWN_SCALAR_X) )
        {
                if (i>= nextblk_hidx )   
                {
                        AE_blk_YSum[(blk_vidx)*(AE_WSIZE_SEGMENT)+blk_hidx] += Sum;
                        Sum=0;
                        blk_hidx++;
                        nextblk_hidx += (AE_WIN_WIDTH*2/AE_WSIZE_SEGMENT);
                }                        
                Sum += *pp;
                pp+=(2*AE_DOWN_SCALAR_X) ;
        }
    }
    for(k=0;k<(AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT);k++)
    {
       ae_WeightedYsum += (AE_blk_YSum[k] ) * (*(siuBlkWeight+k));
       aeweight_sum +=*(siuBlkWeight+k);
    }
    ae_WinSize=(AE_WIN_HEIGHT/AE_DOWN_SCALAR_Y)*(AE_WIN_WIDTH/AE_DOWN_SCALAR_X);
    ae_WeightedYsum = ae_WeightedYsum / ae_WinSize;
    ae_YavgValue = ((AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT)*ae_WeightedYsum) / aeweight_sum;
    //DEBUG_SIU("ae_YavgValue=%d\n",ae_YavgValue);
     
}

s32 siuGetAeYavg_Soft_TV_Full(u8 *siuBlkWeight)
{
    unsigned char *img_ptr,*pp;
    unsigned int   in_data;
    unsigned int ae_WinSize;
    unsigned int blk_vidx, blk_hidx, nextblk_vidx, nextblk_hidx;
    unsigned int i, j,k;
    unsigned int temp_Y;
    unsigned int aeweight_sum=0;
    unsigned int Sum;
    s32 ae_WeightedYsum=0;
    u32 AE_blk_YSum[AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT]={0}; 
    
       
    blk_vidx = 0;
    nextblk_vidx= (AE_WIN_HEIGHT_TV/AE_HSIZE_SEGMENT); 

    i=((isu_idufrmcnt-1) % 3);
    if(i==0)
        img_ptr=(u8*) PKBuf0;
    else if (i==1)
        img_ptr=(u8*) PKBuf1;
    else
        img_ptr=(u8*) PKBuf2;

    for (j= 0; j < AE_WIN_HEIGHT_TV; j+=AE_DOWN_SCALAR_Y)     
    {
        if (j>= nextblk_vidx )
        {
            blk_vidx++;
            nextblk_vidx += (AE_WIN_HEIGHT_TV/AE_HSIZE_SEGMENT);
        }
        blk_hidx = 0;
        nextblk_hidx =( AE_WIN_WIDTH_TV*2/AE_WSIZE_SEGMENT); 
        Sum=0;
        pp=img_ptr + j*AE_WIN_WIDTH_TV*2;
        for (i=0; i< (AE_WIN_WIDTH_TV*2); i+=(2*AE_DOWN_SCALAR_X) )
        {
                if (i>= nextblk_hidx )   
                {
                        AE_blk_YSum[(blk_vidx)*(AE_WSIZE_SEGMENT)+blk_hidx] += Sum;
                        Sum=0;
                        blk_hidx++;
                        nextblk_hidx += (AE_WIN_WIDTH_TV*2/AE_WSIZE_SEGMENT);
                }                        
                Sum += *pp;
                pp+=(2*AE_DOWN_SCALAR_X) ;
        }
    }
    for(k=0;k<(AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT);k++)
    {
       ae_WeightedYsum += (AE_blk_YSum[k] ) * (*(siuBlkWeight+k));
       aeweight_sum +=*(siuBlkWeight+k);
    }
    ae_WinSize=(AE_WIN_HEIGHT_TV/AE_DOWN_SCALAR_Y)*(AE_WIN_WIDTH_TV/AE_DOWN_SCALAR_X);
    ae_WeightedYsum = ae_WeightedYsum / ae_WinSize;
    ae_YavgValue = ((AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT)*ae_WeightedYsum) / aeweight_sum;
    //DEBUG_SIU("ae_YavgValue=%d\n",ae_YavgValue);
     
}

s32 siuGetAeYavg_Soft_TV_Half(u8 *siuBlkWeight)
{
    unsigned char *img_ptr,*pp;
    unsigned int   in_data;
    unsigned int ae_WinSize;
    unsigned int blk_vidx, blk_hidx, nextblk_vidx, nextblk_hidx;
    unsigned int i, j,k;
    unsigned int temp_Y;
    unsigned int aeweight_sum=0;
    unsigned int Sum;
    s32 ae_WeightedYsum=0;
    u32 AE_blk_YSum[AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT]={0}; 
    
       
    blk_vidx = 0;
    nextblk_vidx= (AE_WIN_HEIGHT_TV/2/AE_HSIZE_SEGMENT); 

    i=((isu_idufrmcnt-1) % 3);
    if(i==0)
        img_ptr=(u8*) PKBuf0;
    else if (i==1)
        img_ptr=(u8*) PKBuf1;
    else
        img_ptr=(u8*) PKBuf2;

    for (j= 0; j < AE_WIN_HEIGHT_TV/2; j+=AE_DOWN_SCALAR_Y_TV)     
    {
        if (j>= nextblk_vidx )
        {
            blk_vidx++;
            nextblk_vidx += (AE_WIN_HEIGHT_TV/2/AE_HSIZE_SEGMENT);
        }
        blk_hidx = 0;
        nextblk_hidx =( AE_WIN_WIDTH_TV/2*2/AE_WSIZE_SEGMENT); 
        Sum=0;
        pp=img_ptr + j*AE_WIN_WIDTH_TV/2*2;
        for (i=0; i< (AE_WIN_WIDTH_TV/2*2); i+=(2*AE_DOWN_SCALAR_X_TV) )
        {
                if (i>= nextblk_hidx )   
                {
                        AE_blk_YSum[(blk_vidx)*(AE_WSIZE_SEGMENT)+blk_hidx] += Sum;
                        Sum=0;
                        blk_hidx++;
                        nextblk_hidx += (AE_WIN_WIDTH_TV/2*2/AE_WSIZE_SEGMENT);
                }                        
                Sum += *pp;
                pp+=(2*AE_DOWN_SCALAR_X_TV) ;
        }
    }
    for(k=0;k<(AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT);k++)
    {
       ae_WeightedYsum += (AE_blk_YSum[k] ) * (*(siuBlkWeight+k));
       aeweight_sum +=*(siuBlkWeight+k);
    }
    ae_WinSize=(AE_WIN_HEIGHT_TV/2/AE_DOWN_SCALAR_Y_TV)*(AE_WIN_WIDTH_TV/2/AE_DOWN_SCALAR_X_TV);
    ae_WeightedYsum = ae_WeightedYsum / ae_WinSize;
    ae_YavgValue = ((AE_WSIZE_SEGMENT*AE_HSIZE_SEGMENT)*ae_WeightedYsum) / aeweight_sum;
    //DEBUG_SIU("ae_YavgValue=%d\n",ae_YavgValue);
     
}

#endif
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

    Set subject distance for focus.

Arguments:

    subjectDistance - Subject distance.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 siuSetSubjectDistance(u32 subjectDistance)
{
    exifSetSubjectDistance(subjectDistance);    /* in unit of centimeter */

    /* set initial position of zoom lens for focus */
                
    return 1;
}

/*

Routine Description:

    Set exposure bias value.

Arguments:

    expBiasValue - Exposure bias value (expBiasValue = Exposure Bias Value * 10)
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 siuSetExposureValue(s8 expBiasValue)
{
    const s8 ExposureBiasValueTab[9]={-20,-15,-10,-5,0,5,10,15,20};
    
    exifSetExposureBiasValue(ExposureBiasValueTab[expBiasValue]);       /* expBiasValue = Exposure Bias Value * 10 */       
    siuY_TargetIndex=expBiasValue;
    /* set exposure bias */
    
    return 1;
}

s32 siuSetFlicker50_60Hz(int flicker_sel)
{


    return 0;
}
/*

Routine Description:

    Set light source.

Arguments:

    lightSource - Light source.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 siuSetLightSource(u16 lightSource)
{
    switch (lightSource)
    {
        case SIU_LIGHT_SOURCE_DAYLIGHT:
            AWBgain_Capture[0]=1481;
            AWBgain_Capture[1]=1000;
            AWBgain_Capture[2]=1374;
            
            AWBgain_Preview[0]=1481;
            AWBgain_Preview[1]=1000;
            AWBgain_Preview[2]=1374;
        
            break;
            
        case SIU_LIGHT_SOURCE_CLOUDY_WEATHER:
            AWBgain_Capture[0]=1548;
            AWBgain_Capture[1]=1000;
            AWBgain_Capture[2]=1305;
        
            AWBgain_Preview[0]=1548;
            AWBgain_Preview[1]=1000;
            AWBgain_Preview[2]=1305;        
            break;
            
        case SIU_LIGHT_SOURCE_TUNGSTEN:
            AWBgain_Capture[0]=938;
            AWBgain_Capture[1]=1000;
            AWBgain_Capture[2]=2011;
            
            AWBgain_Preview[0]=938;
            AWBgain_Preview[1]=1000;
            AWBgain_Preview[2]=2011;        
            break;  
            
        case SIU_LIGHT_SOURCE_FLUORESCENT:
            AWBgain_Capture[0]=1548;
            AWBgain_Capture[1]=1000;
            AWBgain_Capture[2]=1305;
        
            AWBgain_Preview[0]=1548;
            AWBgain_Preview[1]=1000;
            AWBgain_Preview[2]=1305;    
            break;
            
            
        case SIU_LIGHT_SOURCE_UNKNOWN: //Auto mode
        
            break;
            
        /*
        case SIU_LIGHT_SOURCE_FLASH:
        
            break;
            
        case SIU_LIGHT_SOURCE_FINE_WEATHER:
        
            break;
            

            
        case SIU_LIGHT_SOURCE_SHADE:
        
            break;
            
        case SIU_LIGHT_SOURCE_DAYLIGHT_FLUORESCENT:
        
            break;
            
        case SIU_LIGHT_SOURCE_DAY_WHITE_FLUORESCENT:
        
            break;
            
        case SIU_LIGHT_SOURCE_COOL_WHITE_FLUORESCENT:
        
            break;
            
        case SIU_LIGHT_SOURCE_WHITE_FLUORESCENT:
        
            break;
            
        case SIU_LIGHT_SOURCE_STANDARD_LIGHT_A:
        
            break;
            
        case SIU_LIGHT_SOURCE_STANDARD_LIGHT_B:
        
            break;
            
        case SIU_LIGHT_SOURCE_STANDARD_LIGHT_C:
        
            break;
            
        case SIU_LIGHT_SOURCE_D55:
        
            break;
            
        case SIU_LIGHT_SOURCE_D65:
        
            break;
            
        case SIU_LIGHT_SOURCE_D75:
        
            break;
            
        case SIU_LIGHT_SOURCE_D50:
        
            break;
            
        case SIU_LIGHT_SOURCE_ISO_STUDIO_TUNGSTEN:
        
            break;
            
        case SIU_LIGHT_SOURCE_OTHER_LIGHT_SOURCE:
        
            break;
        
        case SIU_LIGHT_SOURCE_UNKNOWN:
        */
        default:            
            break;
    }       
    
    exifSetLightSource(lightSource);        
    /* set light source - if it is unknown, use automatic white balance; otherwise, use manual white balance */
                
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

    Set image resolution.

Arguments:

    width - Image width.
    height - Image height.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 siuSetImageResolution(u16 width, u16 height)
{
    exifSetImageResolution(width, height);
    
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

#if 1  //Lucian: 先拿掉閃光燈功能
    siuFlashLightMode = SIU_FLASH_LIGHT_ALWAYS_OFF;   
#else
    switch (mode)
    {
        case SIU_FLASH_LIGHT_RED_EYE_REDUCTION:
            siuFlashLightMode = SIU_FLASH_LIGHT_ALWAYS_ON;          //Lucian:防紅眼=Force ON
            break;
            
        case SIU_FLASH_LIGHT_ALWAYS_ON:
            siuFlashLightMode = SIU_FLASH_LIGHT_ALWAYS_ON;
            break;
            
        case SIU_FLASH_LIGHT_ALWAYS_OFF:
            siuFlashLightMode = SIU_FLASH_LIGHT_ALWAYS_OFF;
            break;

        case SIU_FLASH_LIGHT_AUTOMATIC:        //Auto
            siuFlashLightMode = SIU_FLASH_LIGHT_AUTOMATIC;

        default:
            siuFlashLightMode = SIU_FLASH_LIGHT_AUTOMATIC;
            break;
    }
#endif    
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
    siuAdjustAE(AECurSet); //Lucian: Preview->Video, AE index transfer, Set Sensor Again.
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
    //SiuSensCtrl = SIU_RESET;
        
    // Set sensor control 
    SiuSensCtrl =   
                SIU_NORMAL |
                SIU_ENA |                               //enable SIU module
                SIU_SLAVE |                             //For CMOS sensor
                SIU_VSYNC_ACT_LO |                      //Vsync active low
                SIU_HSYNC_ACT_LO |                      //Hsync active low
                SIU_DEF_PIX_DISA |
                SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
                SIU_INT_ENA_FRAM |                      //Frame end interrupt enaable
                SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
                SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
                SIU_DST_SRAM |                          //SIU to SRAM
//              SIU_CAPT_ENA |                          //SIU out enable
#if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      /* Sensor data is not shifted 2 bits. */            
                SIU_DATA_12b |
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  /* Sensor data is shifted 2 bits. */        
                SIU_DATA_10b | 
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)      /* Sensor data is shifted 4 bits. */        
                SIU_DATA_8b |                               
#endif            
                SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
         #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
            (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
                SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		 #else
		        SIU_FRAM_VSYNC |
         #endif                
		        SIU_OB_COMP_ENA |                      //OB correct enable 
            #if ( (HW_BOARD_OPTION == SALIX_SDV)||(HW_BOARD_OPTION==HX_DH500)/*||(HW_BOARD_OPTION==ULTMOST_SDV)*/)
                SIU_LENS_SHAD_ENA |                    //Lens shading compensation disable
            #else
                SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
            #endif
            #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
                SIU_RGB_GAMMA_ENA |
            #else
                SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
            #endif                
                SIU_AE_ENA |                            //AE report enable
                SIU_TEST_PATRN_DISA |                   //Test pattern disable
                SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.
                            
   
       siuSetLensShading(&lensShading_Preview,100); 
            
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
    u16 data;
    u8 Sv,Av,Tv,Bv;
    u32 ExpTime;
    s32 PrevVideoAEtab_sel;
    int i,j;
    u32 tempReg;

    //=============================//
    RATIONAL fNumber = { 28, 10 };              /* F/2.8 */ 
    RATIONAL apertureValue = { 30, 10 };        /* Av=2*log2(F)=3.0 */

    RATIONAL exposureTime;          
    RATIONAL shutterSpeedValue; //Tv=-log2(exptime)

    RATIONAL brightnessValue;       /* 3.3 Bv */

    //RATIONAL isoSpeedRatings = { 100, 1 };        /* ISO 100 */
    //RATIONAL speedValue = { 50, 10 };         /* Sv= log2(ISO/3.125)=5 */
    Sv=50;
    Av=30;
    //================================================//

    // reset the SIU module
    //SiuSensCtrl = SIU_RESET;
    // stop preview process
    isuStop();
    ipuStop();
    siuStop();    

    
    siuOpMode = SIUMODE_CAPTURE;
    siuFrameCount = 0;  //siuFrameCount 歸零
    siuAeEnable = 0;    //AE 功能暫時關掉
    

    
    while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
#if SIU_MCLK_48M_CAPTURE
        #if( (CHIP_OPTION  == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) )
           SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x00; //MClk=48/1=48MHz
		#elif( (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1020A)|| (CHIP_OPTION == CHIP_A1026A))
		   #if(SYS_CPU_CLK_FREQ == 96000000)
             SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x05; //MClk=288/6=48MHz
		   #elif(SYS_CPU_CLK_FREQ == 108000000)
             SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x03; //MClk=216/4=54MHz
		   #endif
		#elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )
		   //not implement 
        #elif(CHIP_OPTION  == CHIP_PA9002D || CHIP_OPTION  == CHIP_PA9002C)
           SYS_CLK1 = (SYS_CLK1 & 0xffffff00) | 0x00000003;       //Set MCLK=PLLCLK(192M)/4=48MHz. 
        #else
           SYS_CLK1 &= 0xffffffc0;       //Set MCLK=PLLCLK/1=48MHz. 
        #endif         
#endif
    
    setSensorWinSize(zoomFactor, SIUMODE_CAPTURE);
    siuSensorInit(SIUMODE_CAPTURE,zoomFactor);  
  /* 暫時用continue mode 代替
    if (siuFlashLightShoot == 0)  
    {  //Sensor切換Mode:  Continue mode --> Snap mode
        i2cRead_SENSOR(0x1e, &data);
        data |= 0x300;
        i2cWrite_SENSOR(0x1e, data);//071109    
    }  
    */ 
    SiuRawAddr =    (u32)siuRawBuf; //Set Bayer data address.
    
    // Set sensor to enter snap mode and Invert_Trigger is set:
    
    #if(FLASH_LIGHT_SEL == FLASH_LIGHT_PWM) 
        if (siuFlashLightShoot == 1)    
    {   //維持 Continue mode
        siuSetAeBlkWeight(siu_dftAeWeight2, &(aeWin.weightacc)); //有閃光燈時,改變AE weight table
    }   
    #endif
    
    /* Set Digital Gain */      
    SiuSensCtrl =   
        SIU_NORMAL | 
        SIU_SLAVE |                                   //For CMOS sensor
        SIU_VSYNC_ACT_LO |                    //Vsync active low.
        SIU_HSYNC_ACT_LO |                   //Hsync active low
        SIU_DEF_PIX_DISA |
        SIU_INT_DISA_FIFO_OVERF |       //FIFO overflow interrupt disable
        SIU_INT_ENA_FRAM |                  //Frame end interrupt enable         
        SIU_INT_DISA_8LINE |               //8 lines complete interrupt disable
        SIU_INT_DISA_LINE |             //1        lines complete interrupt disable
        SIU_DST_SDRAM |                 //SIU to SDRAM
    #if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      /* Sensor data is not shifted 2 bits. */            
                SIU_DATA_12b |
    #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  /* Sensor data is shifted 2 bits. */        
                SIU_DATA_10b | 
    #elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)      /* Sensor data is shifted 4 bits. */        
                SIU_DATA_8b |                           
    #endif    
        SIU_PIX_CLK_ACT_RIS |           //positive edge latch
 #if( (CHIP_OPTION == CHIP_A1013A)|| (CHIP_OPTION == CHIP_A1013B) ||(CHIP_OPTION == CHIP_A1016A)|| (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
 #else
        SIU_FRAM_VSYNC |
 #endif                

#if GET_SIU_RAWDATA_PURE
        //Disable OB,Lens shading,Pre-gamma, AE report
#else            
        SIU_OB_COMP_ENA |              //OB compensation enable
     #if ( (HW_BOARD_OPTION == SALIX_SDV)||(HW_BOARD_OPTION==HX_DH500)/*||(HW_BOARD_OPTION==ULTMOST_SDV)*/)
        SIU_LENS_SHAD_ENA |            //Lens shading compensation disable
     #else
        SIU_LENS_SHAD_DISA |            //Lens shading compensation disable
     #endif

     #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
        SIU_RGB_GAMMA_ENA |
     #else
        SIU_RGB_GAMMA_DISA |            //Pre-gamma disable
        SIU_AE_ENA |                    //AE report disable
     #endif
#endif      
        SIU_TEST_PATRN_DISA | 
        //SIU_TEST_PATRN_1 |          //Test pattern disable
        SIU_DO_THD_8WORD |
        SIU_SINGLE_CAPTURE_DISA;    

	 #if ( (CHIP_OPTION == CHIP_PA9002D) || (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
         (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
	    SiuDebugSel =SIU_4GAMMATab_EN;
	 #endif
   
    
        OB_B  = SIU_OB_B>>2;
        OB_Gb = SIU_OB_Gb>>2;
        OB_R  = SIU_OB_R>>2;
        OB_Gr = SIU_OB_Gr>>2;

    
        siuSetOpticalBlack(SIU_COMP_B,  SIU_OB_B);
        siuSetOpticalBlack(SIU_COMP_Gb, SIU_OB_Gb);
        siuSetOpticalBlack(SIU_COMP_Gr, SIU_OB_Gr);
        siuSetOpticalBlack(SIU_COMP_R,  SIU_OB_R);
    

    #if SIU_AWBCALIB_ENA
    siuSetDigitalGain(SIU_COMP_B,  siuWBComp_RBRatio[1]);       
        siuSetDigitalGain(SIU_COMP_Gb, 0x0100); 
        siuSetDigitalGain(SIU_COMP_Gr, 0x0100); 
        siuSetDigitalGain(SIU_COMP_R,  siuWBComp_RBRatio[0]);
    #else   
        siuSetDigitalGain(SIU_COMP_B,  0x0100); 
        siuSetDigitalGain(SIU_COMP_Gb, 0x0100);
        siuSetDigitalGain(SIU_COMP_Gr, 0x0100);
        siuSetDigitalGain(SIU_COMP_R,  0x0100);
    #endif
    
    /* Set Lens Shading */
    siuSetLensShading(&lensShading_Capture,100);

    /* Set Defect Pixel Table */
        
    SiuDefPixTbl = (u32)Coordinate_SDV1;
    
    /* Set RGB Gamma Table */
	siuSetRGB_PreGamma(siuPreGammaTab_X,siuPreGammaTab_B,siuPreGammaTab_R,siuPreGammaTab_Gr,siuPreGammaTab_Gb);
    
    
                
    /* 目前Preview/Capture的AE 設定相同, 但未來可做成不同*/

    if (siuFlashLightShoot == 0) //閃光燈關閉
    {
    #if GET_SIU_RAWDATA_PURE  //手動調整:SW(shuttle width), Global gain 
        SetSensorSW(siuAdjustSW);
        i2cWrite_SENSOR( 0x35, siuAdjustAGC);
    #endif
        
        //Lucian: move here. 2008_0218
        siuPreFlashCount = 3;
        siuFrameCount = AECNT_PREVIEW-2;//AECNT_PREVIEW; //
        siuFlashGetYFrameCount = AECNT_PREVIEW + 1;
        siuCaptureFlag = 1;
        siuAeEnable = 1;    

        // start image capture
        SiuSensCtrl |= (SIU_CAPT_ENA | SIU_ENA);    
        
        //Trigger sensor: 暫時用continue mode 代替.
        /*
        i2cRead_SENSOR(0x0b, &data);
        data |= 0x04;
        i2cWrite_SENSOR(0x0b, data);//071109
        */
    
    }
    else //閃光燈啟動
    {
            
        #if(FLASH_LIGHT_SEL == FLASH_LIGHT_SRC)
            siuFrameCount = 1;
            siuCaptureFlag = 1;             
            SiuSensCtrl |= (SIU_CAPT_ENA | SIU_ENA);// start image capture

            //--------主閃-------//
            siuFlashGoFrameCount =3;
            siuFlashGetYFrameCount = siuFlashGoFrameCount + 1;
            
            siuFlashLightReadyToGo = 0;
            siuFlashLightReadyToGetYavg = 0;                
            siuPreFlashCount = 3;
            siuAeEnable = 1;
            
        #elif(FLASH_LIGHT_SEL == FLASH_LIGHT_PWM)
            siuFrameCount = 0;
            siuCaptureFlag = 1;
                SiuSensCtrl |= (SIU_CAPT_ENA | SIU_ENA);// start image capture
        
            //---第一次預閃---//
            SYS_CTL0 |= 0x200000; //Timer-2(PWM0) enable
            Timer2Count = 0xB0;   //設定打出時間
            Timer2IntEna = 0x02;
            siuFlashGoFrameCount = 2;
    
            siuFlashGetYFrameCount = siuFlashGoFrameCount + 1;
    
            siuPreFlashCount = 1;
            siuAeEnable = 1;
            while (siuFlashLightReadyToGo == 0);// 等到讓siuFrameCount 數到 siuFlashGoFrameCount(2),再往下做.
    
            
        
            Gpio0Level |= (0x80);    //打開閃光燈
            Timer2Ctrl = 0x10000000; //Timer-2 enable       
            
            while ((Timer0123IntStat & 0x04) == 0); //wait untile receive timer-2 interrupt.
            Gpio0Level &= ~(0x80);   //關閉閃光燈
            Timer2Ctrl = 0; //Timer-2 disable
    
            
    
            while (siuFlashLightReadyToGetYavg == 0);// 等到讓siuFrameCount 數到 siuFlashGetYFrameCount(3),再往下做.
            
            
            //---第二次預閃---//
            siuFlashGoFrameCount = siuFlashGetYFrameCount + siuPreFlash1ToPreFlash2FrameOffset;
            siuFlashGetYFrameCount = siuFlashGoFrameCount + 1;
    
            siuFlashLightReadyToGo = 0;
            siuFlashLightReadyToGetYavg = 0;
            SYS_CTL0 |= 0x200000;
            Timer2Count = 0xFA; //設定打出時間
            Timer2IntEna = 0x02;
    
            siuPreFlashCount += 1;
        
            while (siuFlashLightReadyToGo == 0); // 等到讓siuFrameCount 數到 siuFlashGoFrameCount(4),再往下做.
    
            
    
            
            Gpio0Level |= (0x80);//打開閃光燈
            Timer2Ctrl = 0x10000000;//Tiner-2 enable
     
            while ((Timer0123IntStat & 0x04) == 0);//wait untile receive timer-2 interrupt.
            Gpio0Level &= ~(0x80); //關閉閃光燈
            Timer2Ctrl = 0; //Timer-2 disable
           
    
            
    
            while (siuFlashLightReadyToGetYavg == 0);// 等到讓siuFrameCount 數到 siuFlashGetYFrameCount(5),再往下做.
                    
            //--------主閃-------//
            siuFlashGoFrameCount = siuFlashGetYFrameCount + siuPreFlash2ToMainFlashFrameOffset;
            siuFlashGetYFrameCount = siuFlashGoFrameCount + 1;
            
            siuFlashLightReadyToGo = 0;
            siuFlashLightReadyToGetYavg = 0;
            SYS_CTL0 |= 0x200000;
            Timer2Count = siuFlashLightCount; //設定主閃能量.
            Timer2IntEna = 0x02;
    
            siuPreFlashCount += 1;
    
            while (siuFlashLightReadyToGo == 0); // 等到讓siuFrameCount 數到 siuFlashGoFrameCount(6),再往下做.
    
            
            
            if(siuShootMainFlash==1)
               {
                Gpio0Level |= (0x80);
                Timer2Ctrl = 0x10000000;
                 
                while ((Timer0123IntStat & 0x04) == 0);
                
                Gpio0Level &= ~(0x80);
                Timer2Ctrl = 0;
            }
    
            
        #endif      
    }   
    


    OSSemPend(siuCapEvt,SIU_TIMEOUT, &err);
    if (err != OS_NO_ERR)
    {
        DEBUG_SIU("Error: siuCapEvt is %d.\n", err);
        return 0;
    }
    SiuSensCtrl &= (~SIU_CAPT_ENA);


        siuStop();
        
    while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.
    
#if SIU_MCLK_48M_CAPTURE    
    #if(Sensor_OPTION ==Sensor_MI_5M)           
        #if( (CHIP_OPTION  == CHIP_A1013A)|| (CHIP_OPTION == CHIP_A1013B) ) 
             SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x01; //MClk=48/2=24MHz
        #elif((CHIP_OPTION == CHIP_A1016A) ||(CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
		   #if(SYS_CPU_CLK_FREQ == 96000000)
             SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x0b; //MClk=288/12=24MHz
		   #elif(SYS_CPU_CLK_FREQ == 108000000)
             SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x08; //MClk=216/9=24MHz
		   #endif
		#elif((CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) )  
           //not implement
        #elif(CHIP_OPTION  == CHIP_PA9002D || CHIP_OPTION  == CHIP_PA9002C )
           SYS_CLK1 = (SYS_CLK1 & 0xffffff00) | 0x00000007;       //Set MCLK=PLLCLK(192M)/8=24MHz.      
        #else
           SYS_CLK1 |= 0x1;         //Set MCLK=PLLCLK/2=24MHz.
        #endif    
    #endif    
#endif

    if (siuFlashLightShoot == 0)
    {       //Sensor切換Mode:  Snap mode --> Continue mode
            i2cRead_SENSOR(0x1e, &data);
            data &= (~0x300);
            i2cWrite_SENSOR(0x1e, data);//071109
            //Triger sensor
            
            i2cRead_SENSOR(0x0b, &data);
            data &= (~0x04);
            i2cWrite_SENSOR(0x0b, data);//071109        
    }

    //Reset flash light control counter
    siuFlashLightReadyToGo = 0;
    siuFlashLightReadyToGetYavg = 0;

    siuFlashGetYFrameCount = 0;
    siuFlashGoFrameCount = 0;

    siuFlashLightShoot = 0;
    siuPreFlashCount = 0;

    /* set related EXIF IFD ... */
    exifSetFlash(flash);
    exifSetApertureValue(&fNumber, &apertureValue); 
    //==//
    
    if(siuOpMode == SIUMODE_MPEGAVI)
       PrevVideoAEtab_sel=1;
    else
       PrevVideoAEtab_sel=0;
    ExpTime=AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].EL *645; //tRow=64.5 us
    ExpTime=10000000/ExpTime;
    exposureTime.numerator=1;
    exposureTime.denominator=ExpTime;
    
    Tv=Cal_LOG2(ExpTime);
    shutterSpeedValue.numerator=Tv;
    shutterSpeedValue.denominator=10;
    
    brightnessValue.numerator=Av+Tv-Sv;
    brightnessValue.denominator=10;
    
    exifSetShutterSpeedValue(&exposureTime, &shutterSpeedValue);
    exifSetBrightnessValue(&brightnessValue);
        
    return 1;   
}



s32 siuCapture_WBComp(s8 zoomFactor)
{
    
    return 1;
}

s32 siuCapture_DarkLightPixelDetect(s8 zoomFactor,u8 SensorMode,u8 OpMode)
{   
    
    
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
    siuFrameCount = 0;
    siuAeEnable = 1;    
    siuFrameNumInMP4    = 0;
    siuSkipFrameRate    = 0;
    siuFrameTime        = 0;    /* Peter 070104 */
    
    /*
    // initialize sensor
    while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: 變頻時 Vsync 不可為High.    
    siuSensorInit(SIUMODE_MPEGAVI,zoomFactor); 
    siuAdjustAE(AECurSet); //Lucian: Preview->Video, AE index transfer, Set Sensor Again.
    */
    //DEBUG_SIU("VideoMode: EPL=%d, GC=%d\n", siu_aeinfo.Exposure_Line, siu_aeinfo.Gain_Code);
    OSTimeDly(1);
    siuVideoClipInit(zoomFactor);   
    siuPreviewFlag = 1;


    return 1;
}

s32 siuVideoZoom(s8 zoomFactor)
{
    siuOpMode = SIUMODE_MPEGAVI;    
        
    // initialize the siu module
    // reset the SIU module
    //SiuSensCtrl = SIU_RESET;
    
    SiuSensCtrl =   
                SIU_NORMAL |
                SIU_ENA |                               //enable SIU module
                SIU_SLAVE |                             //For CMOS sensor
                SIU_VSYNC_ACT_LO |                      //Vsync active low
                SIU_HSYNC_ACT_LO |                      //Hsync active low
                SIU_DEF_PIX_DISA |
                SIU_INT_DISA_FIFO_OVERF |               //FIFO overflow interrupt disable
                SIU_INT_ENA_FRAM |                      //Frame end interrupt enaable
                SIU_INT_DISA_8LINE |                    //8 row complete interrupt disable
                SIU_INT_DISA_LINE |                     //every row complete interrupt disalbe
                SIU_DST_SRAM |                          //SIU to SRAM
//              SIU_CAPT_ENA |                          //SIU out enable
#if (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_NO_SHIFT)      /* Sensor data is not shifted 2 bits. */            
                SIU_DATA_12b |
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_2bit)  /* Sensor data is shifted 2 bits. */        
                SIU_DATA_10b | 
#elif (SENSOR_DATA_SHIFT_OPTION == SENSOR_DATA_SHIFT_4bit)      /* Sensor data is shifted 4 bits. */        
                SIU_DATA_8b |                               
#endif            
                SIU_PIX_CLK_ACT_RIS |                   //positive edge latch 
		 #if( (CHIP_OPTION == CHIP_A1013A)|| (CHIP_OPTION == CHIP_A1013B) ||(CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
          (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		 #else
		        SIU_FRAM_VSYNC |
		 #endif                  
                SIU_OB_COMP_ENA |                      //OB correct enable 
            #if ( (HW_BOARD_OPTION == SALIX_SDV)||(HW_BOARD_OPTION==HX_DH500)/*||(HW_BOARD_OPTION==ULTMOST_SDV)*/)
                SIU_LENS_SHAD_ENA |                    //Lens shading compensation disable
            #else
                SIU_LENS_SHAD_DISA |                    //Lens shading compensation disable
            #endif
            #if( (PA9002D_AWB_EN==1) && (CHIP_OPTION == CHIP_PA9002D) )
               SIU_RGB_GAMMA_ENA |
            #else
               SIU_RGB_GAMMA_DISA |                    //Pre-gamma disable
            #endif
                SIU_AE_ENA |                            //AE report enable
                SIU_TEST_PATRN_DISA |                   //Test pattern disable
                SIU_SINGLE_CAPTURE_DISA;                //Single capture disable.   
    
    if(zoomFactor<7) //Zoom x1
       siuSetLensShading(&lensShading_Video,100); 
    else if( zoomFactor<13 ) //Zoom x2
       siuSetLensShading(&lensShading_Video,100);
    else //Zoom x3
       siuSetLensShading(&lensShading_Video,56);
       
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
    while (1)
    {
        OSSemPend(siuSemEvt, OS_IPC_WAIT_FOREVER, &err);
        if (err != OS_NO_ERR)
        {
            DEBUG_SIU("Error: siuSemEvt is %d.\n", err);
            return ;
        }
        siuPreviewFlag = 1; 
                        
        #if(HW_BOARD_OPTION==ALTERA_FPGA)
        
        #else
            //-------- Auto-Exposure------- //
          #if(FACTORY_TOOL == TOOL_ON)
            if (config_mode_enable[UI_MENU_SETIDX_FOCUS-UI_MENU_SETIDX_LAST-1])
            {
                SetSensorSW(310);
            }
            else
          #endif
            {
                if (siuAeEnable)            
                {
                   if( (sysCheckZoomRun_flag==0) && (siuOpMode !=SIUMODE_CAPTURE) )         
                      siuAutoExposure();
                }
            }
        #endif  
        //------ auto-white-balance---- //
        /*
        when
           Awb_flag=0 ==> AE 不收斂.
                    1 ==> AE 收斂.
                    2 ==> AWB收斂
        */
        if (siuAwbEnable)           
        {
            if(Awb_flag == 1) //AE 收斂後,才做AWB
            {
                // OSD_AFreport
        #if(FACTORY_TOOL == TOOL_ON)
                if (!config_mode_enable[UI_MENU_SETIDX_FOCUS-UI_MENU_SETIDX_LAST-1])
        #endif
                {
                   siuAwbGrayWorld(SIU_AWB_NORMAL);
                   Awb_flag = 2;
                }
             }
        }
        
        /* -------auto-focus----------- */
     #if 0
        if (siuAfEnable)
            siuAutoFocus();
     #endif                   
        //------Flash Light Precharge-------//
        siuFlashLightPreCharge();

        //-----Manual Focus: 測試用-------//
        // OSD_AFreport
     #if(FACTORY_TOOL == TOOL_ON)
        if (config_mode_enable[UI_MENU_SETIDX_FOCUS-UI_MENU_SETIDX_LAST-1])
            ipuGetAFReport();   
     #endif      
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

/*

Routine Description:

    Get Flash Light Y average value.

Arguments:

    idx - Y average value of siuYavg[idx]
    
Return Value:

    0 - Failure.
    1 - Success.

*/

s32 siuGetFlashLightYavg(u8 idx)
{
    volatile unsigned *aeWinSum = &(SiuAeWin0_1);
    s32 ae_WeightedYsum = 0;
    u16 ae_WinSize = 0;
    u32 ae_Ysum = 0; 
    u8 i,pos;

    for(pos=0; pos<25; pos+=2)
    {
        siuAeYreport[pos] = ((*(aeWinSum + pos) & SIU_AE_SUM_WIN_EVEN_MASK) >> SIU_AE_SUM_WIN_EVEN_SHFT);
        siuAeYreport[pos+1] = ((*(aeWinSum + (pos+1)) & SIU_AE_SUM_WIN_ODD_MASK) >> SIU_AE_SUM_WIN_ODD_SHFT);
        
        ae_Ysum = (ae_Ysum + siuAeYreport[pos] + siuAeYreport[pos+1]);
    }

    // Get Y sum value of 25 AE Windows  
    ae_WeightedYsum = ((s32) SiuAeWgtSum * 256);

    if (aeWin.scaleSize == 0)
        ae_WinSize = (aeWin.winSize / 4);
    else if (aeWin.scaleSize == 1)
        ae_WinSize = (aeWin.winSize / 16);
    else if (aeWin.scaleSize == 2)
        ae_WinSize = (aeWin.winSize / 64);

    ae_WeightedYsum = ae_WeightedYsum / ae_WinSize;
    ae_YavgValue = ae_WeightedYsum / aeWin.weightacc;

}

/*

Routine Description:

    Set siu Flash Light Pre-Charge function.

Arguments:

    idx - Y average value of siuYavg[idx]
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 siuFlashLightPreCharge(void)
{
    static u8 ledon = 0;
    u8 level;

    #if(FLASH_LIGHT_SEL == FLASH_LIGHT_SRC)
        if ((siuFlashLightMode == SIU_FLASH_LIGHT_AUTOMATIC || siuFlashLightMode == SIU_FLASH_LIGHT_ALWAYS_ON) && (siuFlashReady == 0))
        {           
            gpioSetLevel(0, 8, 1); //Flash Light recharege is turn-on    
            gpioGetLevel(0, 6, &level); //Flash light detect
            if(level==1)
                siuFlashReady = 1;
    
            if (siuFlashReady == 0)
            {
                ledon ^= 1;
                gpioSetLevel(3, 26, ledon);
            }
            else //Flash light is ready
            {
                ledon = 0;
                gpioSetLevel(3, 26, ledon);
                siuFlashLightPreChargeTimeCount = 0;
            }
        }
        else if (siuFlashLightMode == SIU_FLASH_LIGHT_ALWAYS_OFF)
        {
            gpioSetLevel(0, 8, 0); //Flash-light-recharge is off.
        }
    
    #elif(FLASH_LIGHT_SEL == FLASH_LIGHT_PWM) 
    if ((siuFlashLightMode == SIU_FLASH_LIGHT_AUTOMATIC || siuFlashLightMode == SIU_FLASH_LIGHT_ALWAYS_ON) && (siuFlashReady == 0))
    {
        // PWM Enable: Timer-3
        SYS_CTL0 |= 0x00400000;
        Timer3Count = 0x0000005f; 
        Timer3Ctrl = 0x06a00000;

        gpioGetLevel(0, 6, &level); //Flash light detect
        if(!level)
                siuFlashReady = 1;

            if (siuFlashReady == 0)
        {
            ledon ^= 1;
                gpioSetLevel(0, 0, ledon);
        }
        else //Flash light is ready
        {
            ledon = 0;
            gpioSetLevel(0, 0, ledon);
            
            Timer3Ctrl &= ~(0x00400000);
                SYS_CTL0 &= ~(0x00400000);
            siuFlashLightPreChargeTimeCount = 0;
        }
    }
    else if (siuFlashLightMode == SIU_FLASH_LIGHT_ALWAYS_OFF)
    {
        Timer3Ctrl &= ~(0x00400000);
            SYS_CTL0 &= ~(0x00400000);
    }
    #endif

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

    //DEBUG_SIU("-->ae_YavgValue = %d\n",ae_YavgValue);
                           
    return 1;
}

/*

Routine Description:

    Auto-exposure.

Arguments:

    None.
    
Return Value:

    0 - Failure.
    1 - Success.

*/
s32 siuAutoExposure(void)
{       

    u8 i;
    //SIU_HIST AEWinRingHist[3];  //AE 之3個 Ring Area之Histogram.

    
    /* SW */
    siuAeEnable = 0;
//-----AE_report------//by lisa_0425_S
#if (AE_report==AE_report_Soft)
    #if SW_AE_DEBUG_ENA
        gpioSetLevel(1, 10,1 );    
    #endif
    if(sysTVOutOnFlag) //TV-out
    {
       if(IsuOutAreaFlagOnTV==FULL_TV_OUT)
          siuGetAeYavg_Soft_TV_Full(siu_dftAeWeight1);
       else
          siuGetAeYavg_Soft_TV_Half(siu_dftAeWeight1);
    }
    else
       siuGetAeYavg_Soft_Panel(siu_dftAeWeight1);
    #if SW_AE_DEBUG_ENA
        gpioSetLevel(1, 10,0 );
    #endif
//-----AE_report------//by lisa_0425_E
#else
    for (i = 0; i < 25; i++)
        siuGetAeWinYsum(i, &aeWinYsum[i]);
        
    /*  目前algorithm 並沒有用到此參數,暫時拿掉.
    for (i = SIU_AE_HIST_OUT; i <= SIU_AE_HIST_INN; i++)
        siuGetAeHist(i, &AEWinRingHist[i]);
    */
    
    /* SW */        
    siuGetAeYavg(siu_dftAeWeight1);
#endif
    //  Hist_WgtAdj(); //20051226
    if (siuOpMode != SIUMODE_CAPTURE)
        siuAE_ExposureControl();

    return 1;
}

/*
   Lucian: AE algorithm 有必要做修正.
*/

void siuAE_ExposureControl(void)
{
    u8 err;    
	u8 i;
	u32 dBSum;
	u16 ae_logYtarget = AE_Log_Mapping_Table[ siuSetYtargetTab[4] ]; //x 1024
	u16 ae_logYavg = AE_Log_Mapping_Table[ae_YavgValue]; // x 1024
	s32 ae_logYdiff = ae_logYavg - ae_logYtarget; 
	s32 ae_abs_logYdiff = (ae_logYdiff < 0)? (-1 * ae_logYdiff) : ae_logYdiff;
	static u8 PrintFlag = 0;
	s32 PrevVideoAEtab_sel;
//#if(AE_report == AE_report_Soft)
	u32 Dif_Ylog;
//#endif
	//===//	
	if(siuOpMode == SIUMODE_MPEGAVI)
	   PrevVideoAEtab_sel=1;
	else
	   PrevVideoAEtab_sel=0;

	if (ae_abs_logYdiff > 1500) //Condition: 差距> 1.5 dB: 設定大一點防止震盪.
	{
		/* SW 0222 S */
		siuAeReadyToCapImage = 0;
		/* SW 0222 E */
		PrintFlag = 1;
		dBSum=0;
		
		/*
		    DB_p: 亮->暗: Index 遞增 1, dB 差距
		    DB_n: 暗->亮: Index 遞減 1, dB 差距		
		*/

        if (ae_logYdiff > 0)
		{
		        if( (ae_abs_logYdiff > 6000) && (ae_YavgValue>180) )
			    {
			        Dif_Ylog=ae_abs_logYdiff*3/2;
			        while(dBSum < Dif_Ylog)
					{
					   ae_abs_logYdiff -= AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_n;
					   dBSum +=AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_n;
					   AECurSet --;
    					   if(AECurSet <AECurSet_Min)
					   {
					       AECurSet =AECurSet_Min;
					       break;
					   }					      
					}		
			        
			    }
				else if (ae_abs_logYdiff > 6000) //6 dB
				{   
				    Dif_Ylog=ae_abs_logYdiff*7/8;
					while(dBSum < Dif_Ylog)
					{
					   ae_abs_logYdiff -= AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_n;
					   dBSum +=AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_n;
					   AECurSet --;
					   if(AECurSet <AECurSet_Min)
					   {
					       AECurSet =AECurSet_Min;
					       break;
					   }					      
					}														
				}
			    else if (ae_abs_logYdiff > 3000) //3 dB
				{   
				    Dif_Ylog=ae_abs_logYdiff*4/8;				    
					while(dBSum < Dif_Ylog)
					{
					   ae_abs_logYdiff -= AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_n;
					   dBSum +=AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_n;
					   AECurSet --;
					   if(AECurSet <AECurSet_Min)
					   {
					       AECurSet =AECurSet_Min;
					       break;
					   }					      
					}														
				}
				else 
				{					
					   ae_abs_logYdiff -= AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_n;
					   AECurSet --;
					   if(AECurSet <AECurSet_Min)
					   {
					       AECurSet =AECurSet_Min;
					   }					
    			}
		}
		else if (ae_logYdiff < 0)
		{		
				if (ae_abs_logYdiff > 6000)
				{	
				    Dif_Ylog=ae_abs_logYdiff*7/8;
					while(dBSum < Dif_Ylog) 
					{
					   ae_abs_logYdiff -= AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_p;
					   dBSum +=AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_p;
					   AECurSet ++;
					   if(siuOpMode == SIUMODE_MPEGAVI)
					   {
					       if(AECurSet >AEVideoCurSet_Max)
    					   {
    					       AECurSet =AEVideoCurSet_Max;
    					       break;
    					   } 
					   }
					   else
					   {
    					   if(AECurSet >AEPrevCurSet_Max)
    					   {
    					       AECurSet =AEPrevCurSet_Max;
    					       break;
    					   }
    				   }					      
					}		
				}
			    else if (ae_abs_logYdiff > 3000)
				{	
				    Dif_Ylog=ae_abs_logYdiff*4/8;				    
					while(dBSum < Dif_Ylog)				
					{
					   ae_abs_logYdiff -= AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_p;
					   dBSum +=AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_p;
					   AECurSet ++;
					   if(siuOpMode == SIUMODE_MPEGAVI)
					   {
					       if(AECurSet >AEVideoCurSet_Max)
    					   {
    					       AECurSet =AEVideoCurSet_Max;
    					       break;
    					   } 
					   }
					   else
					   {
    					   if(AECurSet >AEPrevCurSet_Max)
    					   {
    					       AECurSet =AEPrevCurSet_Max;
    					       break;
    					   }
    				   }					      
					}		
				}
				else 
				{				    
					   ae_abs_logYdiff -= AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DB_p;
					   AECurSet ++;
					   
					   if(siuOpMode == SIUMODE_MPEGAVI)
					   {
					       if(AECurSet >AEVideoCurSet_Max)
    					   {
    					       AECurSet =AEVideoCurSet_Max;
    					   } 
					   }
					   else
					   {
    					   if(AECurSet >AEPrevCurSet_Max)
    					   {
    					       AECurSet =AEPrevCurSet_Max;
    					   }		
    				   }			      					
    			}
		} 

		if (AEPreSet != AECurSet)
		{		     
			 if (AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AEPreSet].EL != AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].EL)
			 {
              #if(HW_BOARD_OPTION==ALTERA_FPGA)
                if(siuOpMode == SIUMODE_MPEGAVI)
			       SetSensorSW( (AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].EL * SW_VIDEOCLIP2PREVIEW_RATION*27/48 + 500 )/ 1000);
			    else
                   SetSensorSW(AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].EL*27/48);
              #else
			    if(siuOpMode == SIUMODE_MPEGAVI)
			       SetSensorSW((AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].EL * SW_VIDEOCLIP2PREVIEW_RATION + 500) / 1000);
			    else
				   SetSensorSW(AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].EL);
			  #endif	   
				if (((AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AEPreSet].AG.gain) != (AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].AG.gain)) ) 
				{
			        	siuAeSyncAgcSwControl = 1;
			        	siuFrameCount = 0;
			            siuAeEnable = 1;	
			             #if AE_SYNC_AGC_SW_POLEN
            		       while((siuAeSyncAgcSwControl == 1) && ((siuOpMode == SIUMODE_PREVIEW) || (siuOpMode == SIUMODE_MPEGAVI)));
            		     #else
            		       if( (siuAeSyncAgcSwControl == 1) && ((siuOpMode == SIUMODE_PREVIEW) || (siuOpMode == SIUMODE_MPEGAVI)) )
            		       {
                		        OSSemPend(siuSemEvt, SIU_TIMEOUT, &err);
                        		if (err != OS_NO_ERR)
                        		{
                        			DEBUG_SIU("Error: siuSemEvt is %d.\n", err);
                        			return ;
                        		}
                    	   }
            		     #endif	            		    
				}
			 }
			 
		     siuAeEnable = 0;
		     siuFrameCount = 0;

		     if (((AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AEPreSet].AG.gain) != (AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].AG.gain)) )
		     	  SetSensor_AnalogGain(AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].AG);	     
		     AEConvergeCount += 1;
		}
	
		if (siuAwbEnable == 1)
		{
			if(AECurSet == AEPreSet)
				Awb_flag = 1;
			else
				Awb_flag = 0;
		}

	}
	else
	{   //AE 收斂時
		if (PrintFlag == 1)
		{
#if ADDAPP2TOJPEG
            if(siuOpMode != SIUMODE_MPEGAVI)
            {
    		    exifApp2Data->ae_report.ConvergeCount=AEConvergeCount;
    		    exifApp2Data->ae_report.ae_YavgValue=ae_YavgValue;
    		    exifApp2Data->ae_report.ae_logYdiff=ae_logYdiff;
    		    exifApp2Data->ae_report.AECurSet=AECurSet;
    		    exifApp2Data->ae_report.EL=AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].EL;
    		    exifApp2Data->ae_report.AG=AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].AG.gain;
    		    exifApp2Data->ae_report.MUL=AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].AG.mul;
    		    exifApp2Data->ae_report.DG=AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DG;
    		    exifApp2Data->siuY_TargetIndex=siuY_TargetIndex;
                exifApp2Data->AE_Y_Target=siuSetYtargetTab[siuY_TargetIndex];
    		    for(i=0;i<25;i++)
    		    { 
    		       exifApp2Data->aeWinYsum[i]=aeWinYsum[i];
    		       if(siuFlashLightShoot)
    		         exifApp2Data->AeWeightTab[i]=siu_dftAeWeight2[i];
    		       else
    		         exifApp2Data->AeWeightTab[i]=siu_dftAeWeight1[i];
    		    }
    		}
#endif
            #if 0  //Lucian: 若打開,會影響到Video clip 流暢度,因 printf()會佔住interrrupt時間.   @080305
			DEBUG_SIU("---Debug Report\n");
			DEBUG_SIU("Converge Count is %d\n",AEConvergeCount);
			DEBUG_SIU("YavgValue : %d\n",ae_YavgValue);
			DEBUG_SIU("YLogValue : %d (diff)\n",ae_logYdiff);
			DEBUG_SIU("AECurSet : %d\n",AECurSet);
			DEBUG_SIU("EL : %d\n",AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].EL);
			DEBUG_SIU("AG : %d\n",AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].AG.gain);
			DEBUG_SIU("MUL : %d\n",AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].AG.mul);
			DEBUG_SIU("DG : %d\n",AE_EG_Tab_VideoPrev_5060Hz[PrevVideoAEtab_sel][AE_Flicker_50_60_sel][AECurSet].DG);
			DEBUG_SIU("---End Debug Report\n");
			#endif
			PrintFlag = 0;
	        AEConvergeCount = 0;
		}
		
		/* SW 0222 S */
		siuAeReadyToCapImage = 1; //確認AE收斂後,才能進行Image capture.
		/* SW 0222 E */		
		if(siuAwbEnable == 1 && (Awb_flag != 2))
		{
			Awb_flag = 1;
		}
	}

	AEPreSet = AECurSet;
	siuFrameCount = 0;	
	siuAeEnable = 1;
}



s32 siuSetAwbWin()
{
    //AWB window-size config: 設定full size(640x480),4 pixel/unit.
    SiuAwbWinStart  = 0x00000000; //start point=(0,0)
    SiuAwbWinSize   = 0x007800A0; //Width=160x4, Height=120x4;
              
    SiuAwbCtrl  = 0x03300001;    // scale=1: v:2,h:2,四點取一點.
                 
    SiuAwbThresh    = 0x0fe63cf0; //Y_MAX_THR=230
                                  //Y_MIN_THR=15
                                  //Saturation High bond=60
                                  //RGB High boud=240
                                    
    SiuAwbGain1 = 0x00006c80; //128 表示為1.
    SiuAwbGain2 = 0x0000e6ff; 
                
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
        case SIUMODE_CAPTURE: //Capture mode
            Img_Width =CaptureZoomParm[zoomFactor].size.x;
            Img_Height=CaptureZoomParm[zoomFactor].size.y;
            
            sensor_validsize.imgSize.x = Img_Width + 4;
            sensor_validsize.imgSize.y = Img_Height + 4;
        #if SENSOR_ROW_COL_MIRROR
            sensor_validsize.imgStr.x = ((IMG_MAX_WIDTH-Img_Width)/2) | 0x01;
            sensor_validsize.imgStr.y = ((IMG_MAX_HEIGHT-Img_Height)/2) & (~0x1);
        #else
            sensor_validsize.imgStr.x = ((IMG_MAX_WIDTH-Img_Width)/2) & (~0x1);
            sensor_validsize.imgStr.y = ((IMG_MAX_HEIGHT-Img_Height)/2) | 0x01;
        #endif

             SiuValidSize =  (sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
                             (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);  
        

             SiuValidStart = (sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
                             (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
            
         
             ipuSetIOSize(sensor_validsize.imgSize.x, sensor_validsize.imgSize.y);
             isuSetIOSize(sensor_validsize.imgSize.x-4, sensor_validsize.imgSize.y-4);
     
             break;
            
        case SIUMODE_PREVIEW: //Preview mode
        case SIUMODE_MPEGAVI: //Video clip mode

        if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
        {
            Img_Width   = 1280;
            Img_Height  = 720;
                        
            sensor_validsize.imgSize.x  = Img_Width + 4;
            sensor_validsize.imgSize.y  = Img_Height + 6;
            sensor_validsize.imgStr.x   = 0;
            sensor_validsize.imgStr.y   = 1;

            SiuValidSize =  (sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
                            (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);  
        

            SiuValidStart = (sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
                            (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
            
            ipuSetIOSize(sensor_validsize.imgSize.x, sensor_validsize.imgSize.y);
            isuSetIOSize(sensor_validsize.imgSize.x-4, sensor_validsize.imgSize.y-6);
        }   
        else if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480)
        {
        #if(VIDEO_RESOLUTION_SEL == VIDEO_HD_IN_VGA_OUT)    
            Img_Width   = 1280;
            Img_Height  = 720;
                        
            sensor_validsize.imgSize.x  = Img_Width + 4;
            sensor_validsize.imgSize.y  = Img_Height + 6;
            sensor_validsize.imgStr.x   = 0;
            sensor_validsize.imgStr.y   = 1;

            SiuValidSize =  (sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
                            (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);  
        

            SiuValidStart = (sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
                            (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
            
            ipuSetIOSize(sensor_validsize.imgSize.x, sensor_validsize.imgSize.y);
            isuSetIOSize(Img_Width, Img_Height);
        #elif(VIDEO_RESOLUTION_SEL== VIDEO_1920x1440DECI3x3TOVGA)
            Img_Width   = 640;
            Img_Height  = 480;
                        
            sensor_validsize.imgSize.x  = Img_Width*3 + 4*3;
            sensor_validsize.imgSize.y  = Img_Height*3 + 6*3;
            sensor_validsize.imgStr.x   = 0;
            sensor_validsize.imgStr.y   = 1;

            SiuValidSize =  (sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
                            (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);  
        

            SiuValidStart = (sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
                            (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
            
            ipuSetIOSize(sensor_validsize.imgSize.x/3, sensor_validsize.imgSize.y/3);
            isuSetIOSize(sensor_validsize.imgSize.x/3-4, sensor_validsize.imgSize.y/3-6);
        #else
            Img_Width =640;
            Img_Height=480;
            
            sensor_validsize.imgSize.x = Img_Width + 4;
            sensor_validsize.imgSize.y = Img_Height + 6;
             #if SENSOR_ROW_COL_MIRROR
                sensor_validsize.imgStr.x = ((640-Img_Width)/2) | 0x01;
                sensor_validsize.imgStr.y = ((480-Img_Height)/2) & (~0x1);
             #else
                sensor_validsize.imgStr.x = ((640-Img_Width)/2) & (~0x1);
                sensor_validsize.imgStr.y = ((480-Img_Height)/2) | 0x01;
             #endif   

            SiuValidSize =  (sensor_validsize.imgSize.x << SIU_VALID_SIZE_X_SHFT) |
                            (sensor_validsize.imgSize.y << SIU_VALID_SIZE_Y_SHFT);  
        

            SiuValidStart = (sensor_validsize.imgStr.x << SIU_VALID_START_X_SHFT) |
                            (sensor_validsize.imgStr.y << SIU_VALID_START_Y_SHFT);
            
            ipuSetIOSize(sensor_validsize.imgSize.x, sensor_validsize.imgSize.y);
            isuSetIOSize(sensor_validsize.imgSize.x-4, sensor_validsize.imgSize.y-6);
        #endif     
        }
            
        break;
            
    }
    

    
    
    
    // Set AE Window 
    siuSetAeWin(&aeWin);


    uiDigZoomSetFlag = 0;

    return 1;
}
void siuStop(void)
{
    // set SIU_OE = 0
    //SiuSensCtrl &= 0xffffdfff;
    SiuSensCtrl &= ~(SIU_CAPT_ENA | SIU_INT_ENA_FRAM | SIU_DEF_PIX_ENA);
}


u16 getPreviewZoomScale(s32 zoomFactor)
{
     return (PreviewZoomParm_VGA[zoomFactor].scale);
}

void getPreviewZoomSize(s32 zoomFactor,u32* X,u32* Y)
{
     if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
     {
        *X=PreviewZoomParm_HD[zoomFactor].size.x;
        *Y=PreviewZoomParm_HD[zoomFactor].size.y;
     }
     else
     {
        *X=PreviewZoomParm_VGA[zoomFactor].size.x;
        *Y=PreviewZoomParm_VGA[zoomFactor].size.y;
     }
}


u16 getVideoZoomScale(s32 zoomFactor)
{
     if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_720x480)
          return (VideoZoomParm_D1[zoomFactor].scale);
     else
         return (VideoZoomParm_VGA[zoomFactor].scale);
}

u16 getSensorRawWidth(void)
{
    return   sensor_validsize.imgSize.x; 
}

void siuGetPreviewZoomWidthHeight(s32 zoomFactor,u16 *W, u16 *H)
{
    *W=PreviewZoomParm_VGA[zoomFactor].size.x;
    *H=PreviewZoomParm_VGA[zoomFactor].size.y;
}


#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||(CHIP_OPTION == CHIP_A1026A))
void siuSetAWBGain(s32 Rgain,s32 Bgain)
{
    Rgain=(Rgain*64+500)/1000; //Lucian: 轉為 2.6 format
    if(Rgain > 0x0ff)
        Rgain=0x0ff;
    Bgain=(Bgain*64+500)/1000;
    if(Bgain > 0x0ff)
        Bgain=0x0ff;
    
    SiuPreGammaGain = ((u32)Rgain << SIU_R_GAIN_SHFT) |
                       (0x40 << SIU_Gr_GAIN_SHFT)     |
                       (0x40 << SIU_Gb_GAIN_SHFT)     |
                      ((u32)Bgain << SIU_B_GAIN_SHFT);

}
#endif

void siuSetAwbGainOnPreGamma(u32 Rgain,u32 Grgain,u32 Gbgain,u32 Bgain,int mode)
{
   u32 i;
   u32 temp;

   if(mode == SIU_AWB_NORMAL)
   {
       for(i=0;i<SIU_B_GAMMA_TBL_COUNT;i++)
       {
         temp=(u32)siuPreGammaRefTab_R[i]* Rgain/1000;
         if(temp>255)
            temp=255;
         siuPreGammaTab_R[i]=temp;

         temp=(u32)siuPreGammaRefTab_Gr[i]* Grgain/1000;
         if(temp>255)
            temp=255;
         siuPreGammaTab_Gr[i]=temp;

         temp=(u32)siuPreGammaRefTab_Gb[i]* Gbgain/1000;
         if(temp>255)
            temp=255;
         siuPreGammaTab_Gb[i]=temp;
         
         temp=(u32)siuPreGammaRefTab_B[i]* Bgain/1000;
         if(temp>255)
            temp=255;
         siuPreGammaTab_B[i]=temp;

         siuPreGammaTab_X[i]=siuPreGammaRefTab_X[i];
       }
   }
   else
   {
       for(i=0;i<SIU_B_GAMMA_TBL_COUNT;i++)
       {
         temp=(u32)siuPreGammaRefTab_Y_Night[i]* Rgain/1000;
         if(temp>255)
            temp=255;
         siuPreGammaTab_R[i]=temp;

         temp=(u32)siuPreGammaRefTab_Y_Night[i]* Grgain/1000;
         if(temp>255)
            temp=255;
         siuPreGammaTab_Gr[i]=temp;

         temp=(u32)siuPreGammaRefTab_Y_Night[i]* Gbgain/1000;
         if(temp>255)
            temp=255;
         siuPreGammaTab_Gb[i]=temp;
         
         temp=(u32)siuPreGammaRefTab_Y_Night[i]* Bgain/1000;
         if(temp>255)
            temp=255;
         siuPreGammaTab_B[i]=temp;

         siuPreGammaTab_X[i]=siuPreGammaRefTab_X_Night[i];
       }
   }
}
void siu_FCINTE_ena(void)
{
   SiuSensCtrl |= SIU_INT_ENA_FRAM; 
}

void siu_FCINTE_disa(void)
{
   SiuSensCtrl &= (~SIU_INT_ENA_FRAM); 
}

#endif  //End of Lisa 5M 

