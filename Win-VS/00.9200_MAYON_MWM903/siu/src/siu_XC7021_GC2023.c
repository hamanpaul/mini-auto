
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
#if (Sensor_OPTION == Sensor_XC7021_GC2023)
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

#define AECNT_PREVIEW                     5    //30/(5+1)=5 ¶∏/sec (motion detection)
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
u8 siuFlashLightShoot = 0;      //®M©w¨Oß_•¥∞{•˙øO.
u8 siuShootMainFlash=0;         //®M©w¨Oß_•¥•X•D∞{: •Œ©ÛUser±j≠¢∞{•˙, ¶˝¿Ùπ“§S§£ª›•¥∞{•˙øO,¶Æ¶b®æ¨ı≤¥.

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


#define TAR_MID 0xb0
#define CON_MID 0x40
#define SAT_MID 0x20
#define SHA_MID 0x10
#define TAR_MID_N 0x00

const u8 AETargetMeanTab[11]    =   {TAR_MID-0x70, TAR_MID-0x70, TAR_MID-0x30, TAR_MID-0x30, TAR_MID, TAR_MID+0x30, TAR_MID+0x70, TAR_MID+0x70, TAR_MID+0xa0, TAR_MID+0xa0, TAR_MID+0xa0};
const s8 AETargetMeanTab_N[11]    =   {TAR_MID_N-0x50, TAR_MID_N-0x40, TAR_MID_N-0x30, TAR_MID_N-0x20, TAR_MID_N-0x10, TAR_MID_N, TAR_MID_N+0x10, TAR_MID_N+0x20, TAR_MID_N+0x30, TAR_MID_N+0x40, TAR_MID_N+0x50};

const s8 AEContrastTab[11]      =   {CON_MID-0x40, CON_MID-0x38, CON_MID-0x30, CON_MID-0x20, CON_MID-0x10, CON_MID, CON_MID+0x10, CON_MID+20, CON_MID+0x30, CON_MID+0x38, CON_MID+0x40};

const s8 AESaturationTab[11]    =   {SAT_MID-20, SAT_MID-16, SAT_MID-12, SAT_MID-8, SAT_MID-4, SAT_MID, SAT_MID+4, SAT_MID+8, SAT_MID+12, SAT_MID+16, SAT_MID+20};

const s8 AESharpnessTab[11]  =   {0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A};

SIU_AEINFO siu_aeinfo;
AE_WIN aeWin;
u16 aeWinYsum[25];          //AE §ß25≠”window§ß≠”ßOY sum. (256 unit).

u8 siuAeSyncAgcSwControl = 0;
u8  AEConvergeCount = 0;       //AE ¶¨¿ƒ¶∏º∆


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

#if( (CHIP_OPTION == CHIP_A1019A) || (CHIP_OPTION == CHIP_A1021A) )
u32 XC721_720P_FrameRate  = 20;
u32 XC7021_1080P_FrameRate  = 10;
#else
u32 XC7021_720P_FrameRate  = 15;
u32 XC7021_1080P_FrameRate  = 10;
#endif


#define ElemsOfArray(x) (sizeof(x) / sizeof(x[0]))

const u16 XC7021_default_regs[] = {
// XC7021 setting
    0xfffd,0x80, 
    0xfffe,0x50,  //§¿≠∂
    0x001c,0xff,  //CLOCK  
    0x001d,0xff, 
    0x001e,0xff, 
    0x001f,0xef, 
    0x0018,0x00,  //RESET 
    0x0019,0x00, 
    0x001a,0x00, 
    0x001b,0x00, 
    0x00bc,0x11,  //PAD 
    0x00bd,0x00, 
    0x00be,0x00, 
    0x00bf,0x00,
         
    0x0030,0x09,
    0x0031,0x02,
    0x0032,0x06,//05
    0x0020,0x01,
    0x0021,0x0E,
    0x0023,0x02,
    0x0024,0x05,
    0x0025,0x00,//6
    0x0026,0x01,
    0x0027,0x06, 
    0x0028,0x08,  //I2C_CLK 
    0x0029,0x00, 
    0xfffe,0x25, 
    0x200b,0x33,  //de-glitch                            
    0x0002,0x80,  //I2C master speedê˙I2C ê˚     
    0xfffe,0x50,                                         
    0x0200,0x03,  //SPWD_GPIO_enable bit1:pwdn bit0:reset
    0x0204,0x03,  //output                   
    0x0208,0x02,  // pwdn high active, rst low active    
    0x0208,0x01,  //pwdn lowêﬁreset high                 

    0xfffe,0x25,
    0x6002,0x24,//max:50
    0x6003,0x10,
    0x6008,0x01,//bit[0] pclk ∑•© ;bit[1] Vsync;bit[2] Hsync

    0xfffe,0x50,
    0x00bc,0x11,
    0x001b,0x00,
    0x0090,0x29,
    0xfffe,0x20,
    0x0000,0x20,
    0x0004,0x07,
    0x0005,0x80,
    0x0006,0x04,
    0x0007,0x38,
    0xfffe, 0x26,
    0x4000, 0xF9,
    0x6001, 0x14,
    0x6005, 0xc4,
    0x6006, 0xA,
    0x6007, 0x8C,
    0x6008, 0x9,
    0x6009, 0xFC,

    0x8000,0x3f,
    0x8001,0x80,
    0x8002,0x07,
    0x8003,0x38,
    0x8004,0x04,
    0x8005,0x03,
    0x8006,0x05,
    0x8007,0x99,
    0x8008,0x14,

    0x8010,0x04,
    0x8012,0x80,
    0x8013,0x07,
    0x8014,0x38,
    0x8015,0x04,
    0x8016,0x00,
    0x8017,0x00,
    0x8018,0x00,
    0x8019,0x00,

    0xfffe, 0x30,
    0x0001, 0x11,
    0x0004, 0x18,
    0x0006, 0x7,
    0x0007, 0x80,
    0x0008, 0x4,
    0x0009, 0x38,
    0x000a, 0x5,
    0x000b, 0x0,
    0x000c, 0x2,
    0x000d, 0xD0,
    0x0027, 0xF1,
    0x005e, 0x7,
    0x005f, 0x7F,
    0x0060, 0x4,
    0x0061, 0x37,
    0x1908, 0x0,
    0x1900, 0x0,
    0x1901, 0x0,
    0x1902, 0x0,
    0x1903, 0x0,
    0x1904, 0x7,
    0x1905, 0x80,
    0x1906, 0x4,
    0x1907, 0x38,


    0xfffe,0x25,
    0x0002,0x80,

    ///////////////////////
    //////////////////////
    0xfffe,0x30,  //AVG
    0x1f00,0x00,
    0x1f01,0x00,
    0x1f02,0x00,
    0x1f03,0x00,
    0x1f04,0x07,
    0x1f05,0x60,//80
    0x1f06,0x04,
    0x1f07,0x30,  //38
    0x1f08,0x03,


    0xfffe, 0x50,
    0x000e, 0x54,
    0xfffe, 0x14,
    0x0006, 0x2,
    0x0007, 0x8,
    0x0014, 0x00,
    0x0015, 0x14,
    0x0016, 0x6,
    0x0017, 0x5c,
    0x3d4, 0x9c,
    0x3d5, 0x21,
    0x3d6, 0xff,
    0x3d7, 0xf8,
    0x3d8, 0xd4,
    0x3d9, 0x1,
    0x3da, 0x48,
    0x3dc, 0xd4,
    0x3dd, 0x1,
    0x3de, 0x50,
    0x3df, 0x4,
    0x3e0, 0x18,
    0x3e1, 0x60,
    0x3e3, 0x14,
    0x3e4, 0xa8,
    0x3e5, 0x63,
    0x3e6, 0x1,
    0x3e7, 0xd0,
    0x3e8, 0x19,
    0x3e9, 0x60,
    0x3eb, 0x14,
    0x3ec, 0xa9,
    0x3ed, 0x6b,
    0x3ee, 0x7,
    0x3ef, 0x64,
    0x3f0, 0x94,
    0x3f1, 0xa3,
    0x3f4, 0x19,
    0x3f5, 0xa0,
    0x3f7, 0x14,
    0x3f8, 0xa9,
    0x3f9, 0xad,
    0x3fa, 0x7,
    0x3fb, 0x6c,
    0x3fc, 0x85,
    0x3fd, 0xb,
    0x400, 0x18,
    0x401, 0x60,
    0x403, 0x14,
    0x404, 0xa8,
    0x405, 0x63,
    0x406, 0x2,
    0x407, 0xb0,
    0x408, 0xb8,
    0x409, 0xe5,
    0x40b, 0x44,
    0x40c, 0x85,
    0x40d, 0x43,
    0x410, 0x9c,
    0x411, 0x80,
    0x413, 0x3,
    0x414, 0x8c,
    0x415, 0xaa,
    0x417, 0x3a,
    0x418, 0x84,
    0x419, 0x6d,
    0x41c, 0xe0,
    0x41d, 0xe7,
    0x41e, 0x28,
    0x41f, 0x2,
    0x420, 0x8c,
    0x421, 0xca,
    0x423, 0x3c,
    0x424, 0x18,
    0x425, 0xa0,
    0x427, 0x14,
    0x428, 0xa8,
    0x429, 0xa5,
    0x42a, 0x7,
    0x42b, 0x68,
    0x42c, 0xd4,
    0x42d, 0xb,
    0x42e, 0x18,
    0x430, 0xe5,
    0x431, 0xa6,
    0x432, 0x38,
    0x434, 0x10,
    0x437, 0x3,
    0x438, 0xd4,
    0x439, 0x5,
    0x43a, 0x40,
    0x43c, 0xa8,
    0x43d, 0xc7,
    0x440, 0x8c,
    0x441, 0xaa,
    0x443, 0x3b,
    0x444, 0xe5,
    0x445, 0x66,
    0x446, 0x28,
    0x448, 0x10,
    0x44b, 0x3,
    0x44c, 0x15,
    0x450, 0xa8,
    0x451, 0xc5,
    0x454, 0xe0,
    0x455, 0x66,
    0x456, 0x18,
    0x458, 0xd4,
    0x459, 0xd,
    0x45a, 0x30,
    0x45c, 0x7,
    0x45d, 0xfb,
    0x45e, 0x1c,
    0x45f, 0xd4,
    0x460, 0xe0,
    0x461, 0x63,
    0x462, 0x40,
    0x464, 0x9c,
    0x465, 0x60,
    0x466, 0x1,
    0x468, 0xdc,
    0x469, 0xa,
    0x46a, 0x58,
    0x46b, 0xfa,
    0x46c, 0xe0,
    0x46d, 0x83,
    0x46e, 0x58,
    0x46f, 0x2,
    0x470, 0x18,
    0x471, 0xc0,
    0x473, 0x14,
    0x474, 0xa8,
    0x475, 0xc6,
    0x476, 0x1,
    0x477, 0xe0,
    0x478, 0x94,
    0x479, 0xa6,
    0x47c, 0x94,
    0x47d, 0x66,
    0x47f, 0x2,
    0x480, 0xe0,
    0x481, 0x84,
    0x482, 0x2b,
    0x483, 0x6,
    0x484, 0xe1,
    0x485, 0x63,
    0x486, 0x5b,
    0x487, 0x6,
    0x488, 0xe1,
    0x489, 0x6b,
    0x48a, 0x20,
    0x48c, 0xb9,
    0x48d, 0x6b,
    0x48f, 0x48,
    0x490, 0xdc,
    0x491, 0xa,
    0x492, 0x58,
    0x493, 0xc4,
    0x494, 0x85,
    0x495, 0x21,
    0x498, 0x85,
    0x499, 0x41,
    0x49b, 0x4,
    0x49c, 0x44,
    0x49e, 0x48,
    0x4a0, 0x9c,
    0x4a1, 0x21,
    0x4a3, 0x8,
    0x4a4, 0x18,
    0x4a5, 0x80,
    0x4a7, 0x14,
    0x4a8, 0xa8,
    0x4a9, 0x84,
    0x4aa, 0x2,
    0x4ab, 0xcc,
    0x4ac, 0x84,
    0x4ad, 0x64,
    0x4b0, 0xbc,
    0x4b1, 0x23,
    0x4b3, 0x41,
    0x4b4, 0xc,
    0x4b7, 0x6,
    0x4b8, 0xbc,
    0x4b9, 0x23,
    0x4bb, 0x3f,
    0x4bc, 0x10,
    0x4bf, 0x6,
    0x4c0, 0x9c,
    0x4c1, 0x60,
    0x4c3, 0x3e,
    0x4c7, 0x4,
    0x4c8, 0xd4,
    0x4c9, 0x4,
    0x4ca, 0x18,
    0x4cc, 0x9c,
    0x4cd, 0x60,
    0x4cf, 0x42,
    0x4d0, 0xd4,
    0x4d1, 0x4,
    0x4d2, 0x18,
    0x4d4, 0x44,
    0x4d6, 0x48,
    0x4d8, 0x15,
    0x4dc, 0x9c,
    0x4dd, 0x21,
    0x4de, 0xff,
    0x4df, 0xec,
    0x4e0, 0xd4,
    0x4e1, 0x1,
    0x4e2, 0x48,
    0x4e4, 0xd4,
    0x4e5, 0x1,
    0x4e6, 0x50,
    0x4e7, 0x4,
    0x4e8, 0xd4,
    0x4e9, 0x1,
    0x4ea, 0x60,
    0x4eb, 0x8,
    0x4ec, 0xd4,
    0x4ed, 0x1,
    0x4ee, 0x70,
    0x4ef, 0xc,
    0x4f0, 0x9c,
    0x4f1, 0x60,
    0x4f4, 0x19,
    0x4f5, 0x80,
    0x4f7, 0x14,
    0x4f8, 0xa9,
    0x4f9, 0x8c,
    0x4fa, 0x2,
    0x4fb, 0xb0,
    0x4fc, 0xdc,
    0x4fd, 0x1,
    0x4fe, 0x18,
    0x4ff, 0x10,
    0x500, 0x9c,
    0x501, 0x80,
    0x504, 0x84,
    0x505, 0x6c,
    0x508, 0xd8,
    0x509, 0x1,
    0x50a, 0x20,
    0x50b, 0x12,
    0x50c, 0x84,
    0x50d, 0x63,
    0x50f, 0xd0,
    0x510, 0xb8,
    0x511, 0x63,
    0x513, 0x49,
    0x514, 0xa4,
    0x515, 0x63,
    0x517, 0xff,
    0x518, 0xbc,
    0x519, 0x23,
    0x51c, 0x10,
    0x51f, 0x21,
    0x520, 0x9c,
    0x521, 0xa0,
    0x524, 0xd8,
    0x525, 0x1,
    0x526, 0x28,
    0x527, 0x10,
    0x528, 0x84,
    0x529, 0x8c,
    0x52c, 0x9d,
    0x52d, 0x40,
    0x530, 0x84,
    0x531, 0x64,
    0x533, 0xd0,
    0x534, 0x9d,
    0x535, 0xc1,
    0x537, 0x10,
    0x538, 0xb8,
    0x539, 0x63,
    0x53b, 0x45,
    0x53c, 0xe0,
    0x53d, 0x63,
    0x53e, 0x28,
    0x53f, 0x48,
    0x540, 0xd8,
    0x541, 0x1,
    0x542, 0x18,
    0x543, 0x11,
    0x544, 0x84,
    0x545, 0x64,
    0x547, 0xd0,
    0x548, 0xb8,
    0x549, 0x63,
    0x54b, 0x1,
    0x54c, 0xe0,
    0x54d, 0x63,
    0x54e, 0x28,
    0x54f, 0x48,
    0x550, 0xb8,
    0x551, 0x63,
    0x553, 0x2,
    0x557, 0x6,
    0x558, 0xd8,
    0x559, 0x1,
    0x55a, 0x18,
    0x55b, 0x12,
    0x55c, 0x10,
    0x55f, 0x2d,
    0x560, 0x15,
    0x567, 0x1b,
    0x568, 0x84,
    0x569, 0x6c,
    0x56c, 0xbc,
    0x56d, 0x2a,
    0x56f, 0x1,
    0x570, 0x13,
    0x571, 0xff,
    0x572, 0xff,
    0x573, 0xfb,
    0x574, 0xbc,
    0x575, 0x2a,
    0x577, 0x2,
    0x578, 0x84,
    0x579, 0x6c,
    0x57c, 0x8c,
    0x57d, 0x83,
    0x57f, 0x8b,
    0x580, 0x7,
    0x581, 0xfb,
    0x582, 0x1b,
    0x583, 0x90,
    0x584, 0x94,
    0x585, 0x63,
    0x587, 0xb2,
    0x588, 0x9c,
    0x589, 0x80,
    0x58a, 0xff,
    0x58b, 0xf0,
    0x58c, 0x8c,
    0x58d, 0x61,
    0x58f, 0x11,
    0x590, 0xe1,
    0x591, 0x6b,
    0x592, 0x20,
    0x593, 0x3,
    0x594, 0xe1,
    0x595, 0x63,
    0x596, 0x58,
    0x597, 0x4,
    0x59b, 0x1e,
    0x59c, 0xd8,
    0x59d, 0x1,
    0x59e, 0x58,
    0x59f, 0x11,
    0x5a0, 0xbc,
    0x5a1, 0x23,
    0x5a3, 0x1,
    0x5a4, 0xc,
    0x5a7, 0x17,
    0x5a8, 0x9c,
    0x5a9, 0x83,
    0x5aa, 0xff,
    0x5ab, 0xfe,
    0x5ac, 0xbc,
    0x5ad, 0x44,
    0x5af, 0x1,
    0x5b0, 0xc,
    0x5b3, 0x10,
    0x5b4, 0x9c,
    0x5b5, 0x63,
    0x5b6, 0xff,
    0x5b7, 0xfc,
    0x5b8, 0xbc,
    0x5b9, 0x43,
    0x5bb, 0x3,
    0x5bc, 0x13,
    0x5bd, 0xff,
    0x5be, 0xff,
    0x5bf, 0xdb,
    0x5c0, 0x9c,
    0x5c1, 0x80,
    0x5c3, 0x6,
    0x5c4, 0x9c,
    0x5c5, 0xa0,
    0x5c7, 0x3,
    0x5c8, 0x3,
    0x5c9, 0xff,
    0x5ca, 0xff,
    0x5cb, 0xd8,
    0x5cc, 0xd8,
    0x5cd, 0x1,
    0x5ce, 0x20,
    0x5cf, 0x10,
    0x5d0, 0x8c,
    0x5d1, 0x83,
    0x5d3, 0x8b,
    0x5d4, 0x7,
    0x5d5, 0xfb,
    0x5d6, 0x1b,
    0x5d7, 0x7b,
    0x5d8, 0x94,
    0x5d9, 0x63,
    0x5db, 0xb4,
    0x5dc, 0x8c,
    0x5dd, 0x61,
    0x5df, 0x12,
    0x5e0, 0xa5,
    0x5e1, 0x6b,
    0x5e3, 0x3,
    0x5e4, 0xe1,
    0x5e5, 0x63,
    0x5e6, 0x58,
    0x5e7, 0x4,
    0x5eb, 0xa,
    0x5ec, 0xd8,
    0x5ed, 0x1,
    0x5ee, 0x58,
    0x5ef, 0x12,
    0x5f0, 0x9c,
    0x5f1, 0x60,
    0x5f3, 0x4,
    0x5f4, 0x9c,
    0x5f5, 0xa0,
    0x5f7, 0x2,
    0x5f8, 0x3,
    0x5f9, 0xff,
    0x5fa, 0xff,
    0x5fb, 0xcc,
    0x5fc, 0xd8,
    0x5fd, 0x1,
    0x5fe, 0x18,
    0x5ff, 0x10,
    0x600, 0x9c,
    0x601, 0x80,
    0x603, 0x2,
    0x604, 0xa8,
    0x605, 0xa3,
    0x608, 0x3,
    0x609, 0xff,
    0x60a, 0xff,
    0x60b, 0xc8,
    0x60c, 0xd8,
    0x60d, 0x1,
    0x60e, 0x20,
    0x60f, 0x10,
    0x610, 0x84,
    0x611, 0xac,
    0x614, 0xe0,
    0x615, 0x6a,
    0x616, 0x50,
    0x618, 0xe0,
    0x619, 0x8e,
    0x61a, 0x50,
    0x61c, 0xe0,
    0x61d, 0x63,
    0x61e, 0x28,
    0x620, 0x8c,
    0x621, 0x84,
    0x624, 0x94,
    0x625, 0x63,
    0x627, 0xb0,
    0x628, 0x7,
    0x629, 0xfb,
    0x62a, 0x19,
    0x62b, 0x8b,
    0x62c, 0x8c,
    0x62d, 0xa5,
    0x62f, 0x8b,
    0x630, 0x9c,
    0x631, 0x6a,
    0x633, 0x1,
    0x634, 0xa5,
    0x635, 0x43,
    0x637, 0xff,
    0x638, 0xbc,
    0x639, 0x4a,
    0x63b, 0x2,
    0x63c, 0xf,
    0x63d, 0xff,
    0x63e, 0xff,
    0x63f, 0xcd,
    0x640, 0xbc,
    0x641, 0x2a,
    0x643, 0x1,
    0x644, 0x85,
    0x645, 0x21,
    0x648, 0x85,
    0x649, 0x41,
    0x64b, 0x4,
    0x64c, 0x85,
    0x64d, 0x81,
    0x64f, 0x8,
    0x650, 0x85,
    0x651, 0xc1,
    0x653, 0xc,
    0x654, 0x44,
    0x656, 0x48,
    0x658, 0x9c,
    0x659, 0x21,
    0x65b, 0x14,
    0x65c, 0x9c,
    0x65d, 0x21,
    0x65e, 0xff,
    0x65f, 0xf4,
    0x660, 0xd4,
    0x661, 0x1,
    0x662, 0x48,
    0x664, 0xd4,
    0x665, 0x1,
    0x666, 0x50,
    0x667, 0x4,
    0x668, 0xd4,
    0x669, 0x1,
    0x66a, 0x60,
    0x66b, 0x8,
    0x66c, 0xbc,
    0x66d, 0x3,
    0x66e, 0x1,
    0x66f, 0xc,
    0x670, 0x10,
    0x673, 0x15,
    0x674, 0xbc,
    0x675, 0x43,
    0x676, 0x1,
    0x677, 0xc,
    0x678, 0xc,
    0x67b, 0xb,
    0x67c, 0xbc,
    0x67d, 0x3,
    0x67e, 0x1,
    0x67f, 0x5,
    0x680, 0xbc,
    0x681, 0x3,
    0x682, 0x1,
    0x683, 0x10,
    0x684, 0x10,
    0x687, 0x16,
    0x688, 0xbc,
    0x689, 0x3,
    0x68a, 0x4,
    0x68b, 0xc,
    0x68c, 0x19,
    0x68d, 0x80,
    0x68f, 0x14,
    0x690, 0xa9,
    0x691, 0x8c,
    0x692, 0x3,
    0x693, 0x18,
    0x694, 0xc,
    0x697, 0x29,
    0x698, 0x15,
    0x69f, 0x25,
    0x6a0, 0x15,
    0x6a4, 0x19,
    0x6a5, 0x80,
    0x6a7, 0x14,
    0x6a8, 0xa9,
    0x6a9, 0x8c,
    0x6aa, 0x3,
    0x6ab, 0x18,
    0x6ac, 0xc,
    0x6af, 0x23,
    0x6b0, 0x15,
    0x6b4, 0x7,
    0x6b5, 0xff,
    0x6b6, 0xff,
    0x6b7, 0x48,
    0x6b8, 0x15,
    0x6bf, 0x20,
    0x6c0, 0x84,
    0x6c1, 0x6c,
    0x6c4, 0x19,
    0x6c5, 0x80,
    0x6c7, 0x14,
    0x6c8, 0xa9,
    0x6c9, 0x8c,
    0x6ca, 0x3,
    0x6cb, 0x18,
    0x6cc, 0x7,
    0x6cd, 0xff,
    0x6ce, 0xff,
    0x6cf, 0x76,
    0x6d0, 0x15,
    0x6d7, 0x1a,
    0x6d8, 0x84,
    0x6d9, 0x6c,
    0x6dc, 0x18,
    0x6dd, 0x60,
    0x6df, 0x14,
    0x6e0, 0xa8,
    0x6e1, 0x63,
    0x6e2, 0x2,
    0x6e3, 0xb0,
    0x6e4, 0x19,
    0x6e5, 0x40,
    0x6e7, 0x14,
    0x6e8, 0xa9,
    0x6e9, 0x4a,
    0x6ea, 0x2,
    0x6eb, 0xf8,
    0x6ec, 0x84,
    0x6ed, 0x83,
    0x6f0, 0x84,
    0x6f1, 0xaa,
    0x6f4, 0x94,
    0x6f5, 0x64,
    0x6f7, 0x7e,
    0x6f8, 0xe4,
    0x6f9, 0xa3,
    0x6fa, 0x28,
    0x6fc, 0x10,
    0x6ff, 0x4,
    0x700, 0x15,
    0x704, 0x9c,
    0x705, 0x60,
    0x707, 0x4,
    0x708, 0xd8,
    0x709, 0x4,
    0x70a, 0x18,
    0x70b, 0xd7,
    0x70c, 0x19,
    0x70d, 0x80,
    0x70f, 0x14,
    0x710, 0xa9,
    0x711, 0x8c,
    0x712, 0x3,
    0x713, 0x18,
    0x714, 0x84,
    0x715, 0x6c,
    0x718, 0x7,
    0x719, 0xfb,
    0x71a, 0x5,
    0x71b, 0xc4,
    0x71c, 0x84,
    0x71d, 0x63,
    0x71e, 0x1,
    0x71f, 0xc0,
    0x720, 0x9c,
    0x721, 0x80,
    0x722, 0xff,
    0x723, 0xf0,
    0x724, 0xe1,
    0x725, 0x6b,
    0x726, 0x20,
    0x727, 0x3,
    0x72b, 0x4,
    0x72c, 0xd4,
    0x72d, 0xa,
    0x72e, 0x58,
    0x730, 0x7,
    0x731, 0xff,
    0x732, 0xff,
    0x733, 0x6b,
    0x734, 0x15,
    0x738, 0x84,
    0x739, 0x6c,
    0x73c, 0x9d,
    0x73d, 0x60,
    0x740, 0xd4,
    0x741, 0x3,
    0x742, 0x59,
    0x743, 0xcc,
    0x744, 0xd4,
    0x745, 0x3,
    0x746, 0x59,
    0x747, 0xc0,
    0x748, 0xd4,
    0x749, 0x3,
    0x74a, 0x59,
    0x74b, 0xc4,
    0x74c, 0xd4,
    0x74d, 0x3,
    0x74e, 0x59,
    0x74f, 0xc8,
    0x750, 0x85,
    0x751, 0x21,
    0x754, 0x85,
    0x755, 0x41,
    0x757, 0x4,
    0x758, 0x85,
    0x759, 0x81,
    0x75b, 0x8,
    0x75c, 0x44,
    0x75e, 0x48,
    0x760, 0x9c,
    0x761, 0x21,
    0x763, 0xc,

    ///////////    patch configure   /////////////////
    //banding
    0xfffe,0x14,

    0x00b4,0x02,//0:off,1:60hz,2:50hz

    0x00b6,0x1b,
    0x00b7,0xd7,
    0x00b8,0x21,
    0x00b9,0x68,

    0xfffe,0x14,
    0x00be,0x6e,  	//camera i2c id
    0x00bf,0x00,  	//camera i2c bits     

    0x00c0,0x05,  	//sensor type gain
    0x00c1,0x02,  	//sensor type exposure

    //0x00c3,0x01,    //sensor mode select

    //exposure
    0x00c4,0x00,  //write camera exposure variable [15:12]
    0x00c5,0x03,
    0x00c6,0x00,  //write camera exposure variable [11:4]
    0x00c7,0x04,
    0x00c8,0x00,  //write camera exposure variable [3:0]
    0x00c9,0x00,

    0x00cc,0x00,  //camera exposure addr mask 1
    0x00cd,0xff,      
    0x00ce,0x00,  //camera exposure addr mask 2
    0x00cf,0xff,
    0x00d0,0x00,  //camera exposure addr mask 3
    0x00d1,0x00,
    0x00d2,0x00,  //camera exposure addr mask 4
    0x00d3,0x00,

    //gain
    0x00e4,0x00,  //camera gain addr  0
    0x00e5,0xb6,

    0x00e6,0x00,  //camera gain addr  1
    0x00e7,0xb1,

    0x00e8,0x00,  //camera gain addr  2
    0x00e9,0xb2,




    0x00ec,0x00,  //camera gain addr mask 1
    0x00ed,0xff,
    0x00ee,0x00,  //camera gain addr mask 2
    0x00ef,0xff,
    0x00f0,0x00,  //camera gain addr mask 3
    0x00f1,0xff,
    0x00f2,0x00,  //camera gain addr mask 4
    0x00f3,0x00,

    ////////////////////ENABLE////////////////////////////
    0xfffe,0x30,
    0x0051,0x03,

    0xfffe,0x50,
    0x0137,0x99,

    0xfffe,0x14,
    0x0026,0x01,
    0x00aa,0x04,
    0x00ab,0x20,
    0x0092,0x43,
    0x0078,0x20,
    0x007f,0x20,
    /////////////////////////////////////////////////////
    ///////////////////   patch   end     ///////////////
    /////////////////////////////////////////////////////

    0xfffe,0x14,
    0x006c,0x01,
    0x006d,0xb0,
    0x013e,0x00,//awb move
    0x0143,0x00,
    //AE window
    0x3f,0x01,
    0x40,0x01,
    0x41,0x01,
    0x42,0x01,
    0x43,0x01,
    0x44,0x01,
    0x45,0x01,
    0x46,0x01,
    0x47,0x01,
    0x48,0x01,
    0x49,0x01,
    0x4a,0x40,
    0x4b,0x40,
    0x4c,0x40,
    0x4d,0x01,
    0x4e,0x01,
    0x4f,0x40,
    0x50,0x40,
    0x51,0x40,
    0x52,0x01,
    0x53,0x01,
    0x54,0x01,
    0x55,0x01,
    0x56,0x01,
    0x57,0x01,
    0x3a,0x01,
    0x77,0x10,
    0xfffe,0x30, 
    0x0000,0xcf,
    0x0001,0x87,
    0x130e,0x08,
    0x130f,0x00,
    0xe00,0x34, 
    0xe0a,0x00, 
    0xe0b,0xff,
    0xf00,0x08,
    0xf05,0x00,
    0xf06,0x00,
    0xf0a,0x00, 
    0x2000,0x30,
    0x2001,0x16,
    //0x1452,0x78,
    //0x1453,0x70,
    //0x1454,0xff,
    0x1701,0x50,
    0x1702,0x60,
    0x1704,0x20,
    0x1707,0x00,

    0x1450,0x6e,
    0x1451,0x00,
    0x1452,0x6e,
    0x1453,0x4e,




    //lenc
    0xfffe,0x30,

    0x0200,0x3c,//lenc2_95
    0x0201,0x20,
    0x0202,0x19,
    0x0203,0x19,
    0x0204,0x1f,
    0x0205,0x3c,
    0x0206,0xa ,
    0x0207,0x5 ,
    0x0208,0x2 ,
    0x0209,0x3 ,
    0x020a,0x6 ,
    0x020b,0xa ,
    0x020c,0x4 ,
    0x020d,0x0 ,
    0x020e,0x0 ,
    0x020f,0x0 ,
    0x0210,0x0 ,
    0x0211,0x5 ,
    0x0212,0x5 ,
    0x0213,0x0 ,
    0x0214,0x0 ,
    0x0215,0x0 ,
    0x0216,0x1 ,
    0x0217,0x6 ,
    0x0218,0xa ,
    0x0219,0x6 ,
    0x021a,0x3 ,
    0x021b,0x3 ,
    0x021c,0x6 ,
    0x021d,0xb ,
    0x021e,0x3f,
    0x021f,0x2a,
    0x0220,0x1f,
    0x0221,0x20,
    0x0222,0x2a,
    0x0223,0x3f,
    0x0224,0x46,
    0x0225,0x46,
    0x0226,0x34,
    0x0227,0x56,
    0x0228,0x26,
    0x022a,0x32,
    0x022b,0x10,
    0x022c,0x11,
    0x022d,0x10,
    0x022e,0x53,
    0x0230,0x0 ,
    0x0231,0x12,
    0x0232,0x31,
    0x0233,0x22,
    0x0234,0x10,
    0x0236,0x32,
    0x0237,0x10,
    0x0238,0x11,
    0x0239,0x10,
    0x023a,0x53,
    0x023c,0x27,
    0x023d,0x57,
    0x023e,0x55,
    0x023f,0x47,
    0x0240,0x17,
    0x0248,0xef,

    0xfffe,0x30,	
    0x024d,0x0,	
    0x024e,0xCC,	
    0x024f,0x1,	
    0x250,0x6C,	
    0x251,0x1,	
    0x252,0x11,	
    0x253,0x0,	
    0x254,0xF2,	


    /////////////////////////////

    0xfffe,0x30,
    0x0013,0x10,


    //AWB
    0xfffe,0x14, //awb_0601
    0x0027,0x01,
    0x013c,0x02,
    0x0174,0x00,
    0x0175,0x00,
    0x0176,0x06,
    0x0177,0x00,
    0x0178,0x00,
    0x0179,0x00,
    0x017a,0x04,
    0x017b,0x00,
    0x017c,0x00,
    0x017d,0x00,
    0x017e,0x04,
    0x017f,0x00,
    0x0180,0x00,
    0x0181,0x00,
    0x0182,0x04,
    0x0183,0x04,
    0x01a8,0x00,
    0x01a9,0x00,
    0x01aa,0x06,
    0x01ab,0x00,
    0x01ac,0x00,
    0x01ad,0x00,
    0x01ae,0x04,
    0x01af,0x00,
    0x01b0,0x00,
    0x01b1,0x00,
    0x01b2,0x04,
    0x01b3,0x04,
                

    0xfffe,0x14,
    0x0027,0x01,
    0x013c,0x01,
    0x013d,0x01,
    0x013e,0x00,
    0x0170,0x0f,
    0x0171,0xff,






    0xfffd,0x80,  //AWB_4
    0xfffe,0x30,
    0x0000,0xcf,
    0x0001,0x83,
    0x0003,0xe5,

    0x0730,0x7c,  // win1 startx
    0x0731,0x9a,  // win1 endx
    0x0732,0x36,  // win1 starty
    0x0733,0x48,  // win1 endy
    0x0734,0x40,  // win2 startx
    0x0735,0x50,  // win2 endx
    0x0736,0x6d,  // win2 starty
    0x0737,0x85,  // win2 endy
    0x0738,0x6b,  // win3 startx
    0x0739,0x8c,  // win3 endx
    0x073a,0x45,  // win3 starty
    0x073b,0x58,  // win3 endy
    0x073c,0x5c,  // win4 startx
    0x073d,0x6f,  // win4 endx
    0x073e,0x62,  // win4 starty
    0x073f,0x70,  // win4 endy
    0x0740,0x50,  // win5 startx
    0x0741,0x5c,  // win5 endx
    0x0742,0x54,  // win5 starty
    0x0743,0x78,  // win5 endy
    0x0744,0x00,  // win6 startx
    0x0745,0x00,  // win6 endx
    0x0746,0x00,  // win6 starty
    0x0747,0x00,  // win6 endy
    0x0748,0x5a,  // win7 startx
    0x0749,0x75,  // win7 endx
    0x074a,0x6c,  // win7 starty
    0x074b,0x82,  // win7 endy
    0x074c,0x5c,  // win8 startx
    0x074d,0x6f,  // win8 endx
    0x074e,0x50,  // win8 starty
    0x074f,0x62,  // win8 endy
    0x0750,0x00,  // win9 startx
    0x0751,0x00,  // win9 endx
    0x0752,0x00,  // win9 starty
    0x0753,0x00,  // win9 endy
    0x0754,0x00,  // win10 startx
    0x0755,0x00,  // win10 endx
    0x0756,0x00,  // win10 starty
    0x0757,0x00,  // win10 endy
    0x0758,0x00,  // win11 startx
    0x0759,0x00,  // win11 endx
    0x075a,0x00,  // win11 starty
    0x075b,0x00,  // win11 endy
    0x075c,0x00,  // win12 startx
    0x075d,0x00,  // win12 endx
    0x075e,0x00,  // win12 starty
    0x075f,0x00,  // win12 endy
    0x0760,0x00,  // win13 startx
    0x0761,0x00,  // win13 endx
    0x0762,0x00,  // win13 starty
    0x0763,0x00,  // win13 endy
    0x0764,0x00,  // win14 startx
    0x0765,0x00,  // win14 endx
    0x0766,0x00,  // win14 starty
    0x0767,0x00,  // win14 endy
    0x0768,0x00,  // win15 startx
    0x0769,0x00,  // win15 endx
    0x076a,0x00,  // win15 starty
    0x076b,0x00,  // win15 endy
    0x076c,0x00,  // win16 startx
    0x076d,0x00,  // win16 endx
    0x076e,0x00,  // win16 starty
    0x076f,0x00,  // win16 endy
    0x0770,0x21,  // wt1 wt2
    0x0771,0x22,  // wt3 wt4
    0x0772,0x20,  // wt5 wt6    //10
    0x0773,0x11,  // wt7 wt8
    0x0774,0x00,  // wt9 wt10
    0x0775,0x00,  // wt11 wt12
    0x0776,0x00,  // wt13 wt14
    0x0777,0x00,  // wt15 wt16

    0xfffe,0x14,
    0x0027,0x01,
    0x0170,0x0f,
    0x0171,0xff,


    0xfffd, 0x80,//nathan5
    0xfffe, 0x30,
    0x1400, 0x00,
    0x1401, 0x05,
    0x1402, 0x0a,
    0x1403, 0x0f,
    0x1404, 0x15,
    0x1405, 0x1a,
    0x1406, 0x1f,
    0x1407, 0x24,
    0x1408, 0x29,
    0x1409, 0x2e,
    0x140a, 0x32,
    0x140b, 0x37,
    0x140c, 0x3c,
    0x140d, 0x40,
    0x140e, 0x45,
    0x140f, 0x49,
    0x1410, 0x4e,
    0x1411, 0x52,
    0x1412, 0x56,
    0x1413, 0x5a,
    0x1414, 0x5e,
    0x1415, 0x62,
    0x1416, 0x65,
    0x1417, 0x69,
    0x1418, 0x6c,
    0x1419, 0x6f,
    0x141a, 0x73,
    0x141b, 0x76,
    0x141c, 0x79,
    0x141d, 0x7c,
    0x141e, 0x7e,
    0x141f, 0x81,
    0x1420, 0x84,
    0x1421, 0x89,
    0x1422, 0x8d,
    0x1423, 0x92,
    0x1424, 0x96,
    0x1425, 0x9a,
    0x1426, 0x9d,
    0x1427, 0xa1,
    0x1428, 0xa4,
    0x1429, 0xa7,
    0x142a, 0xaa,
    0x142b, 0xad,
    0x142c, 0xaf,
    0x142d, 0xb2,
    0x142e, 0xb4,
    0x142f, 0xb6,
    0x1430, 0xb8,
    0x1431, 0xbc,
    0x1432, 0xc0,
    0x1433, 0xc4,
    0x1434, 0xc8,
    0x1435, 0xcc,
    0x1436, 0xd0,
    0x1437, 0xd4,
    0x1438, 0xd8,
    0x1439, 0xdd,
    0x143a, 0xe1,
    0x143b, 0xe6,
    0x143c, 0xeb,
    0x143d, 0xf0,
    0x143e, 0xf5,
    0x143f, 0xfa,
    0x1440, 0xff,




    //cmx
    0x1200,0x4,	
    0x1201,0x50,	
    0x1202,0x0,	
    0x1203,0x33,	
    0x1204,0x1,	
    0x1205,0x69,	
    0x1206,0x0,	
    0x1207,0xF4,	
    0x1208,0x1,	
    0x1209,0x6,	
    0x120A,0x1,	
    0x120B,0xB7,	
    0x120C,0x7,	
    0x120D,0x40,	
    0x120E,0x0,	
    0x120F,0x47,	
    0x1210,0x0,	
    0x1211,0xC0,	
    0x1212,0x0,	
    0x1213,0x88,	
    0x1214,0x0,	
    0x1215,0x6A,	
    0x1216,0x2,	
    0x1217,0x17,	
    0x1218,0x5,	
    0x1219,0xA8,	
    0x121A,0x0,	
    0x121B,0x2,	
    0x121C,0x0,	
    0x121D,0x41,	
    0x121E,0x1,	
    0x121F,0x77,	
    0x1220,0x0,	
    0x1221,0x1C,	
    0x1222,0x2,	
    0x1223,0xDD,	
    0x122e,0x2,	
    0x122F,0x1A,	
    0x1230,0x12,	
    0x1228,0x0,	
    0x1229,0x62,	
    0x122A,0x0,	
    0x122B,0xD9,	
    0x122C,0x1,	
    0x122D,0x5B,	


    0xfffe,0x30,
    0x1231,0x02,
    	

    //TOP
    0xfffe,0x30,
    0x0000,0xcF,  
    0x0001,0x93,  //a7
    0x0003,0xe5,
    0x0013,0x10,
    0x071c,0x1a,
    0x1700,0x09,
    0x1701,0x40,
    0x1702,0x40,
    0x1704,0x20,
    0xfffe,0x14,
    0x0027,0x01
};


const u8 GC2033_default_regs[] = {
	
    0xf2,0x00,
    0xf3,0x00,
    0xf6,0x00,
    0xfc,0x06,
    0xf7,0x03,
    0xf8,0x09, 
    0xf9,0x06,
    0xfa,0x05,
    0xfc,0x0e,
    /////////////////////////////////////////////////
    ////////////   ANALOG & CISCTL   ////////////////
    /////////////////////////////////////////////////
    0xfe,0x00,
    0x03,0x03,
    0x04,0xf6, 
    0x05,0x02, //HB
    0x06,0xd0,
    0x07,0x00, //VB
    0x08,0x92, 
    0x09,0x00,
    0x0a,0x00, //row start
    0x0b,0x00,
    0x0c,0x00, //col start
    0x0d,0x04, 
    0x0e,0x40, //height 1088
    0x0f,0x07, 
    0x10,0x88, //width 1928
    0x12,0xe2,
    0x17,0x54,
    0x18,0x02,
    0x19,0x0d,
    0x1a,0x18,
    0x1c,0x6c,
    0x1d,0x12,
    0x20,0x54,
    0x21,0x2c,
    0x23,0xf0,
    0x24,0xc1,
    0x25,0x18,
    0x26,0x64,
    0x28,0x20,
    0x29,0x08,
    0x2a,0x08,
    0x2b,0x48,
    0x2d,0x1c,
    0x2f,0x40,
    0x30,0x99,
    0x34,0x00,
    0x38,0x80,
    0x3b,0x12,
    0x3d,0xb0,
    0xcc,0x8a,
    0xcd,0x99,
    0xcf,0x70,
    0xd0,0xcb,
    0xd2,0xc1,
    0xd8,0x80,
    0xda,0x14,
    0xdc,0x24,
    0xe1,0x14,
    0xe3,0xf0,
    0xe4,0xfa,
    0xe6,0x1f,
    0xe8,0x02,
    0xe9,0x02,
    0xea,0x03,
    0xeb,0x03,
    /////////////////////////////////////////////////
    //////////////////   ISP   //////////////////////
    /////////////////////////////////////////////////
    0xfe,0x00,
    0x80,0x5c,
    0x88,0x23,
    0x89,0x03,
    0x90,0x01, 
    0x92,0x03, //2<= y <=6
    0x94,0x03, //2<= x <=6
    0x95,0x04, //crop win height
    0x96,0x38,
    0x97,0x07, //crop win width
    0x98,0x80,
    /////////////////////////////////////////////////
    //////////////////   BLK   //////////////////////
    /////////////////////////////////////////////////
    0xfe,0x00,
    0x40,0x22,
    0x43,0x07,
    0x4e,0x3c,
    0x4f,0x00,
    0x60,0x00,
    0x61,0x80,
    /////////////////////////////////////////////////
    //////////////////   GAIN   /////////////////////
    /////////////////////////////////////////////////
    0xfe,0x00,
    0xb0,0x58,
    0xb1,0x01, 
    0xb2,0x00, 
    0xb6,0x00, 
    0xfe,0x01,
    0x01,0x00,
    0x02,0x01,
    0x03,0x02,
    0x04,0x03,
    0x05,0x04,
    0x06,0x05,
    0x07,0x06,
    0x08,0x0e,
    0x09,0x16,
    0x0a,0x1e,
    0x0b,0x36,
    0x0c,0x3e,
    0x0d,0x56,
    0x0e,0x5e,
    /////////////////////////////////////////////////
    //////////////////   DNDD   /////////////////////
    /////////////////////////////////////////////////
    0xfe,0x02,
    0x81,0x05,
    /////////////////////////////////////////////////
    //////////////////   dark sun   /////////////////
    /////////////////////////////////////////////////
    0xfe,0x01,
    0x54,0x77,
    0x58,0x00,
    0x5a,0x05,
    /////////////////////////////////////////////////
    //////////////////	 MIPI	/////////////////////
    /////////////////////////////////////////////////
    0xfe,0x03, 
    0x01,0x00,
    0x02,0x00,
    0x03,0x00,
    0x06,0x00,
    0x10,0x00,
    0x15,0x00,
    0x36,0x83,
    0x8f,0x64,
    /////////////////////////////////////////////////
    //////////////////   pad enable   ///////////////
    /////////////////////////////////////////////////
    0xfe,0x00, 
    0xf3,0x01,
    0xfa,0x85,
    0xf2,0x0f		
};

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
extern s16 ipuColorCorrTransform[10]; //for 9002x, CCM ªPCTM ¶X®÷•Œ.
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

u16 Cal_LOG2(u16 X);
 

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

    while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: ≈‹¿WÆ… Vsync §£•i¨∞High.
	// initialize sensor
	//rfiuVideoInFrameRate=siuSensorInit(mode,zoomFactor);
	
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
        		SIU_VSYNC_ACT_HI|                      //Vsync active low
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
                SIU_YUVMAP_C9;
        
	
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

s32 siuSetFlicker50_60Hz(int flicker_sel) //0:60Hz:NTSC, 1:50Hz:PAL
{
    i2cWrite_XC7021(0xfffe,0x14);
    if (flicker_sel)
        i2cWrite_XC7021(0x00b4,0x02);  //banding mode 01:60HZ 02:50HZ
    else
        i2cWrite_XC7021(0x00b4,0x01);  //banding mode 01:60HZ 02:50HZ
    DEBUG_SIU("siuSetFlicker50_60Hz %d\n", flicker_sel);

    i2cWrite_XC7021(0x00b6,0x1b);
    i2cWrite_XC7021(0x00b7,0xd7);	//60HZ Unit
    i2cWrite_XC7021(0x00b8,0x21);
    i2cWrite_XC7021(0x00b9,0x68);	//50HZ Unit
}

s32 siuSetExposureValue(s8 expBiasValue)
{

    siuY_TargetIndex = expBiasValue;
    DEBUG_SIU("siuSetExposureValue(%d) \n", expBiasValue);
    //RX 6 level only, index RX to TX {0,0,2,4,5,6,8}
    i2cWrite_XC7021(0xfffe, 0x14); 
    if (expBiasValue < 6)
        i2cWrite_XC7021(0x006c, 0x01);
    else
        i2cWrite_XC7021(0x006c, 0x02);
    i2cWrite_XC7021(0x006d, AETargetMeanTab[expBiasValue]);

}


s32 siuSetContrast(s8 expBiasValue)
{}

s32 siuSetSaturation(s8 expBiasValue)
{}

s32 siuSetSharpness(s8 expBiasValue)
{}
s32 siuSetImage(void)
{
    siuSetFlicker50_60Hz(iconflag[UI_MENU_SETIDX_TX_FLICKER]);
    siuSetExposureValue(iconflag[UI_MENU_SETIDX_TX_BRIGHT]);

}
s32 XC7021BypassOn(void)
{
    i2cWrite_XC7021(0xfffd,0x80);
    i2cWrite_XC7021(0xfffe,0x50);
    i2cWrite_XC7021(0x004D,0x01);
}
s32 XC7021BypassOff(void)
{
    i2cWrite_XC7021(0xfffd,0x80);
    i2cWrite_XC7021(0xfffe,0x50);
    i2cWrite_XC7021(0x004D,0x00);
}
s32 GC2023_Init_1080P(void) //init setting is original 1080p related
{
    u16 data;
    int i;
    DEBUG_SIU("GC2023_Init_1080P() \n");
    XC7021BypassOn();

    for(i = 0; i < ElemsOfArray(GC2033_default_regs)-1 ; i+=2)
    {
        i2cWrite_GC2023((u16)GC2033_default_regs[i], GC2033_default_regs[i+1]);
        //DEBUG_SIU("i = %d %x, %x\n", i, GC2033_default_regs[i], GC2033_default_regs[i+1]);
    }
    #if 0
    for(i = 0; i < 115*2-1; i+=2)
    {
        i2cRead_GC2023((u16)GC2033_default_regs[i], &data);
        DEBUG_SIU("%x, %x => read %x\n", GC2033_default_regs[i], GC2033_default_regs[i+1], data);
    }
    #endif

    XC7021BypassOff();
}

s32 XC7021_Init_1080P(void) //init setting is original 1080p related
{
    int i;

    DEBUG_SIU("XC7021_Init_1080P() \n");

    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 ); //sensor reset
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 0 );
    for(i=0;i<0x0ff;i++);
    gpioSetLevel(GPIO_RST_SENSOR_GROUP, GPIO_RST_SENSOR, 1 );
    OSTimeDly(10);

    for(i = 0; i < ElemsOfArray(XC7021_default_regs) - 1; i+=2)
    {
        i2cWrite_XC7021(XC7021_default_regs[i], (u8)XC7021_default_regs[i+1]);
        //DEBUG_SIU("i = %d %x, %x\n", i, XC7021_default_regs[i], XC7021_default_regs[i+1]);
    }
}

s32 XC7021_1080P_10fps(void)
{
    DEBUG_SIU("XC7021_1080P_10fps\n");
    i2cWrite_XC7021(0xfffe, 0x26);
    i2cWrite_XC7021(0x8010, 0x08);

    //sensor init           
    i2cWrite_XC7021(0xfffd, 0x80);
    i2cWrite_XC7021(0xfffe, 0x50);
    i2cWrite_XC7021(0x0004, 0x00);
    i2cWrite_XC7021(0xfffd, 0x80);
    i2cWrite_XC7021(0xfffe, 0x14);
    i2cWrite_XC7021(0x1300, 0x02);

    i2cWrite_XC7021(0x1301, 0x05);
    i2cWrite_XC7021(0x1302, 0x05);
    i2cWrite_XC7021(0x1303, 0x06);
    i2cWrite_XC7021(0x1304, 0x90);

    i2cWrite_XC7021(0xfffd, 0x80);
    i2cWrite_XC7021(0xfffe, 0x50);
    i2cWrite_XC7021(0x0007, 0x6e);
    i2cWrite_XC7021(0x000d, 0x30);
    i2cWrite_XC7021(0x0009, 0x00);
    i2cWrite_XC7021(0x00c4, 0x10);
    i2cWrite_XC7021(0x00c0, 0x01);

    OSTimeDly(10);

    //ISP init
    i2cWrite_XC7021(0xfffe, 0x26);
    i2cWrite_XC7021(0x4000, 0xF9);
    i2cWrite_XC7021(0x6001, 0x14);
    i2cWrite_XC7021(0x6005, 0xc4);
    i2cWrite_XC7021(0x6006, 0x0F);
    i2cWrite_XC7021(0x6007, 0xA0);
    i2cWrite_XC7021(0x6008, 0x0E);

    i2cWrite_XC7021(0xfffe, 0x30);
    i2cWrite_XC7021(0x0001, 0x83);

    i2cWrite_XC7021(0x0004, 0x18);
    i2cWrite_XC7021(0x0006, 0x07);
    i2cWrite_XC7021(0x0007, 0x80);
    i2cWrite_XC7021(0x0008, 0x04);
    i2cWrite_XC7021(0x0009, 0x38);
    i2cWrite_XC7021(0x000a, 0x07);
    i2cWrite_XC7021(0x000b, 0x80);
    i2cWrite_XC7021(0x000c, 0x04);
    i2cWrite_XC7021(0x000d, 0x38);

    i2cWrite_XC7021(0x0027, 0xF1);
    i2cWrite_XC7021(0x005e, 0x07);
    i2cWrite_XC7021(0x005f, 0x7F);
    i2cWrite_XC7021(0x0060, 0x04);
    i2cWrite_XC7021(0x0061, 0x37);
    i2cWrite_XC7021(0x1908, 0x01);
    i2cWrite_XC7021(0x1900, 0x00);
    i2cWrite_XC7021(0x1901, 0x00);
    i2cWrite_XC7021(0x1902, 0x00);
    i2cWrite_XC7021(0x1903, 0x02);
    i2cWrite_XC7021(0x1904, 0x07);
    i2cWrite_XC7021(0x1905, 0x80);
    i2cWrite_XC7021(0x1906, 0x04);
    i2cWrite_XC7021(0x1907, 0x38);

    OSTimeDly(10);

    i2cWrite_XC7021(0xfffe, 0x26);
    i2cWrite_XC7021(0x8010, 0x0c);

}

s32 XC7021_720P_20fps()
{
    DEBUG_SIU("XC7021_720P_20fps\n");

    i2cWrite_XC7021(0xfffe, 0x26);
    i2cWrite_XC7021(0x8010, 0x08);

    //sensor init
    i2cWrite_XC7021(0xfffd, 0x80);
    i2cWrite_XC7021(0xfffe, 0x50);
    i2cWrite_XC7021(0x0004, 0x00);
    i2cWrite_XC7021(0xfffd, 0x80);
    i2cWrite_XC7021(0xfffe, 0x14);
    i2cWrite_XC7021(0x1300, 0x2);

    i2cWrite_XC7021(0x1301, 0x05);
    i2cWrite_XC7021(0x1302, 0x02);
    i2cWrite_XC7021(0x1303, 0x06);
    i2cWrite_XC7021(0x1304, 0xd0);

    i2cWrite_XC7021(0xfffd, 0x80);
    i2cWrite_XC7021(0xfffe, 0x50);
    i2cWrite_XC7021(0x0007, 0x6e);
    i2cWrite_XC7021(0x000d, 0x30);
    i2cWrite_XC7021(0x0009, 0x00);
    i2cWrite_XC7021(0x00c4, 0x10);
    i2cWrite_XC7021(0x00c0, 0x01);

    OSTimeDly(10);

    //ISP init
    i2cWrite_XC7021(0xfffe, 0x26);
    i2cWrite_XC7021(0x4000, 0xF9);
    i2cWrite_XC7021(0x6001, 0x14);
    i2cWrite_XC7021(0x6005, 0xc4);
    i2cWrite_XC7021(0x6006, 0x0A);
    i2cWrite_XC7021(0x6007, 0x8C);
    i2cWrite_XC7021(0x6008, 0x09);

    i2cWrite_XC7021(0xfffe, 0x30);
    i2cWrite_XC7021(0x0001, 0x93);
    i2cWrite_XC7021(0x0004, 0x18);
    i2cWrite_XC7021(0x0006, 0x07);
    i2cWrite_XC7021(0x0007, 0x80);
    i2cWrite_XC7021(0x0008, 0x04);
    i2cWrite_XC7021(0x0009, 0x38);
    i2cWrite_XC7021(0x000a, 0x05);
    i2cWrite_XC7021(0x000b, 0x00);
    i2cWrite_XC7021(0x000c, 0x02);
    i2cWrite_XC7021(0x000d, 0xD0);

    i2cWrite_XC7021(0x1908, 0x00);
    i2cWrite_XC7021(0x1903, 0x00);

    OSTimeDly(10);

    i2cWrite_XC7021(0xfffe, 0x26);
    i2cWrite_XC7021(0x8010, 0x0c);

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
    
#if FPGA_BOARD_A1018_SERIES
    //---------Run on Mclk=32 MHz--------------//    
    #if(SYS_CPU_CLK_FREQ == 32000000)
	  SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x00; //MClk=32/1=32MHz
	#elif(SYS_CPU_CLK_FREQ == 48000000)
      SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x01; //MClk=48/2=24MHz
	#endif    
#else   
    #if 1
       //---------Run on Mclk=24 MHz--------------//
       #if(SYS_CPU_CLK_FREQ == 96000000)
        SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x0b; //MClk=288/12=24MHz
       #elif(SYS_CPU_CLK_FREQ == 108000000)
        SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x08; //MClk=216/9=24MHz
       #elif( (SYS_CPU_CLK_FREQ == 160000000) || (SYS_CPU_CLK_FREQ == 162000000) )
        #if REDUCE_HCLK_TO_128M
        SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x0f; //MClk=384/16=24MHz 
        #elif REDUCE_HCLK_TO_144M
        SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x11; //MClk=432/18=24MHz 
        #else
        SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x13; //MClk=480/20=24MHz 
        #endif
       #elif(SYS_CPU_CLK_FREQ == 180000000)
        SYS_CLK1 = (SYS_CLK1 & (~0x0000003f)) | 0x13; //MClk=540/20=24MHz  
       #endif
    #else
       //---------Run on Mclk=27 MHz--------------//
       #if(SYS_CPU_CLK_FREQ == 160000000)
        #if REDUCE_HCLK_TO_128M
        SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x0d; //MClk=384/14=27.428MHz 
        #elif REDUCE_HCLK_TO_144M
        SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x0f; //MClk=384/16=27MHz 
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
                    XC7021_Init_1080P();
                    GC2023_Init_1080P();
                }
                XC7021_1080P_10fps();
                FrameRate   = 10;

            }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x352) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) )
            {
                if (justboot == 0)
                {
                    XC7021_Init_1080P();
                    GC2023_Init_1080P();
                }
                XC7021_720P_20fps();
                FrameRate   = 20;
            }
			else
		    {
    		    if (justboot == 0)
                {
                    XC7021_Init_1080P();
                    GC2023_Init_1080P();
    		    }
                XC7021_720P_20fps();
                FrameRate   = 20;
			}

            OSTimeDly(10);

            if(justboot == 0)
            {//Sensor already send data(not stable like half green half black), make a pulse by setting Vsync active to clear sensor buffer
                justboot = 1;

                SiuSensCtrl |= SIU_VSYNC_ACT_HI; //Vsync active HI
                SiuSensCtrl |= SIU_VSYNC_ACT_HI;

            }
            else
            {//Enable sensor and make CIU enable earlier, to throw two might-wrong frames
                GpioActFlashSelect |= CHIP_IO_SEN_EN;
                OSTimeDly(1);
                CIU_5_CTL2|= CIU_ENA;
            }
            
            if((uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720))
            {
                IsuScUpFIFODepth = 0x1d1d1111; //ß‚MP™∫scalling™∫¿Wºe§¿µπSP°A¥£§…scalling ÆƒØ‡ //¡◊™·´Ã
            }
            
            siuSetImage();
            #if (HW_BOARD_OPTION == MR9100_TX_MAYON_MWL612)
            siuSetSensorDayNight(SIUMODE);
            #endif

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
        Lucian:	∑Ì¨∞VideoClip Zoom º“¶°Æ…, ¶]ø˝ºv¡Ÿ¶b∂i¶Ê§§,§£•ireset Mpeg control variable.
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

#if (HW_BOARD_OPTION == MR9100_TX_MAYON_MWL612)
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
        i2cWrite_XC7021(0xfffe,0x14);
        i2cWrite_XC7021(0x00aa,0x01);
        i2cWrite_XC7021(0x00ab,0xd0);
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x1700,0x81);//±m¶‚∂¬•’§¡¥´°A09°G±m¶‚°B81°G∂¬•’
        i2cWrite_XC7021(0x1701,0x40);
        i2cWrite_XC7021(0x1702,0x40);
        i2cWrite_XC7021(0x1704,0x20);
        i2cWrite_XC7021(0x0f00,0xf0);
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x0e0a,0x04);
        i2cWrite_XC7021(0x130e,0x09);
        i2cWrite_XC7021(0x130f,0x0d); 
        i2cWrite_XC7021(0x0013,0x10);
#if NIGHT_COLOR_ENA // ±m¶‚§£§¡¥´ Saturation
#else
#endif

    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        i2cWrite_XC7021(0xfffe,0x14);
        i2cWrite_XC7021(0x00aa,0x04);
        i2cWrite_XC7021(0x00ab,0x20);
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x1700,0x09);//±m¶‚∂¬•’§¡¥´°A09°G±m¶‚°B81°G∂¬•’
        i2cWrite_XC7021(0x1701,0x40);//π°©M´◊ Cb
        i2cWrite_XC7021(0x1702,0x48);//π°©M´◊ Cr
        i2cWrite_XC7021(0x1704,0x21);//πÔ§Ò´◊
        i2cWrite_XC7021(0x0f00,0x20);//•hæ∏°Amax°Gf0
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x0e0a,0x04);
        i2cWrite_XC7021(0x130e,0x08);
        i2cWrite_XC7021(0x130f,0x00); 
        i2cWrite_XC7021(0x0013,0x06);
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
    if(Level == SIU_NIGHT_MODE)
    {
        DEBUG_SIU("##enter night\n");
        i2cWrite_XC7021(0xfffe,0x14);
        i2cWrite_XC7021(0x006c,0x01);
        i2cWrite_XC7021(0x006d,0xd0);//•ÿº–´G´◊°A6c-H°B6d-L
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x1700,0x81);//±m¶‚∂¬•’§¡?°A09°G±m¶‚°B81°G∂¬•’
        i2cWrite_XC7021(0x1701,0x60);//π°©M´◊ Cb
        i2cWrite_XC7021(0x1702,0x60);//π°©M´◊ Cr
        i2cWrite_XC7021(0x1704,0x22);//πÔ§Ò´◊
        i2cWrite_XC7021(0x0f00,0x20);//•hæ∏°Amax°Gf0       
#if NIGHT_COLOR_ENA // ±m¶‚§£§¡¥´ Saturation
#else
#endif
    }
    else if(Level == SIU_DAY_MODE)
    {
        DEBUG_SIU("##enter day\n");
        i2cWrite_XC7021(0xfffe,0x14);
        i2cWrite_XC7021(0x006c,0x01);
        i2cWrite_XC7021(0x006d,0xd0);//•ÿ?´G´◊°A6c-H°B6d-L
        i2cWrite_XC7021(0xfffe,0x30);
        i2cWrite_XC7021(0x1700,0x09);//±m¶‚∂¬•’§¡?°A09°G±m¶‚°B81°G∂¬•’
        i2cWrite_XC7021(0x1701,0x60);//π°©M´◊ Cb
        i2cWrite_XC7021(0x1702,0x60);//π°©M´◊ Cr
        i2cWrite_XC7021(0x1704,0x22);//πÔ§Ò´◊
        i2cWrite_XC7021(0x0f00,0x20);//•hæ∏°Amax°Gf0
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
		   //----------- ∞µºv≠µ¶P®B,≤£•ÕAudio clock -------------//
		    Temp                = siuFrameNumInMP4;
			siuFrameNumInMP4   += siuSkipFrameRate;
    

            //----------∞µFrame rate control----------//
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
			siuFlashLightMode = SIU_FLASH_LIGHT_ALWAYS_ON;	        //Lucian:®æ¨ı≤¥=Force ON
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
	
	
	while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: ≈‹¿WÆ… Vsync §£•i¨∞High.
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
	siuFrameCount = AECNT_PREVIEW-1; //Lucian: OV7720 ®S¶≥Snap mode.≠nµ•≤ƒ§G±i¶A®˙π≥.
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
		
	while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: ≈‹¿WÆ… Vsync §£•i¨∞High.
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
	siuFrameCount = AECNT_PREVIEW-1; //Lucian: OV7720 ®S¶≥Snap mode.≠nµ•≤ƒ§G±i¶A®˙π≥.
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
	
	
	while(SiuSyncStat & 0x01); //hold when Vsync=HIGH: ≈‹¿WÆ… Vsync §£•i¨∞High.
	siuSensorInit(SensorMode,zoomFactor);	
	setSensorWinSize(zoomFactor, SensorMode);
	
	
	if(OpMode == DETECT_DARK_PIXEL)
	{
	     SetSensorSW(765);  //√n•˙Æ…∂°≥]©w 1/20 sec
	     AG.gain=0;       //AGC gain = x1;
	     SetSensor_AnalogGain(AG);
	}
	else if(OpMode == DETECT_LIGHT_PIXEL)
	{
	     //SetSensorSW(1);  //√n•˙Æ…∂°≥]©w≥Ã§p
	     SetSensorSW(510);  //√n•˙Æ…∂°≥]©w1/30 (≥Ã§j)
	     AG.gain=0x70; //§£≥]AG= x1, ¨O¶]¨∞≠nDetect Warm pixel
	     SetSensor_AnalogGain(AG);
	}
	else if(OpMode == DETECT_OPTICAL_BLACK)
	{
	     SetSensorSW(1);  //√n•˙Æ…∂°≥]©w≥Ã§p
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
	siuFrameCount = AECNT_PREVIEW-2; //Lucian: OV7720 ®S¶≥Snap mode.≠nµ•≤ƒ§G±i¶A®˙π≥.
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
    u8 pulse_time=250;   //®CøÈ•X250¶∏Hight Level §j¨˘ ™·0.5ms
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
	ae_WeightedYsum = ((s32) SiuAeWgtSum * 256); //weight sum §@≠”≥Ê¶Ï¨∞256 

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
    //AWB window-size config: ≥]©wfull size(640x480),4 pixel/unit.
	SiuAwbWinStart	= 0x00000000; //start point=(0,0)
	SiuAwbWinSize	= 0x007800A0; //Width=160x4, Height=120x4;
              
	SiuAwbCtrl	= 0x03300001;    // scale=1: v:2,h:2,•|¬I®˙§@¬I.
        		 
	SiuAwbThresh	= 0x0fe640f0; //Y_MAX_THR=230
	                              //Y_MIN_THR=15
	                              //Saturation High bond=80
	                              //RGB High boud=240
	                              
	
	SiuAwbGain1	= 0x00005080; //128 ™Ì•‹¨∞1.
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
	    case SIUMODE_CAPTURE: //Capture mode, •ÿ´eµL™k®œ•Œ        
	    case SIUMODE_PREVIEW: //Preview mode	         		
        case SIUMODE_MPEGAVI: //Video clip mode: §£§‰¥©Zoom-in/Out
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
    Rgain=(Rgain*64+500)/1000; //Lucian: ¬‡¨∞ 2.6 format
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








