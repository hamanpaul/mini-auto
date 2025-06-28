
/*
Copyright (c) 2008 Mars Semiconductor Corp.

Module Name:

    siu_MI1320.c

Abstract:

    The routines of Sensor Interface Unit.
    Control MI1320 (1.3M) sensor
            
Environment:

        ARM RealView Developer Suite

Revision History:
    
    2009/04/17  Lucian Yuan  Create  
*/


#include "general.h"
#if (Sensor_OPTION == Sensor_HM1375_YUV601)
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

#if(VIDEO_RESOLUTION_SEL == VIDEO_HD_IN_HD_OUT)
#define HM1375_FRAME_RATE   15
#else   // (VIDEO_RESOLUTION_SEL == VIDEO_VGA_IN_VGA_OUT)
#define HM1375_FRAME_RATE   30
#endif



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
u8 siuY_TargetIndex     = 4;

//const s8 AETargetMeanTab[9] = {0x1c, 0x24, 0x2c, 0x34, 0x3c, 0x44, 0x4c, 0x54, 0x5c};
#if((HW_BOARD_OPTION == MR8120_TX_RDI)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA532)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||\
    ((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT == 7)||(UI_PROJ_OPT == 4)||(UI_PROJ_OPT == 2) || (UI_PROJ_OPT == 5) || (UI_PROJ_OPT == 8))))
const s8 AETargetMeanTab[9] = {0x00, 0x0f, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f};
#elif( (HW_BOARD_OPTION  == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
const s8 AETargetMeanTab[9] = {0x00, 0x0f, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f};
#elif (((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT != 7)&&(UI_PROJ_OPT != 4)&&(UI_PROJ_OPT != 2)&&(UI_PROJ_OPT != 5)&&(UI_PROJ_OPT != 8))) ||\
     (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652)  )
const s8 AETargetMeanTab_720P[9]    = {0x00, 0x08, 0x1f, 0x20, 0x26, 0x3f, 0x4f, 0x58, 0x5f};
const s8 AETargetMeanTab_VGA[9]     = {0x05, 0x10, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74, 0x84};
#elif ((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION  == MR8120S_TX_GCT_VM00)||\
    (HW_BOARD_OPTION == MR8120_TX_GCT_VM00))
//const s8 AETargetMeanTab_720P[9]    = {0x14, 0x20, 0x24, 0x30, 0x34, 0x3c, 0x48, 0x50, 0x54};
//const s8 AETargetMeanTab_720P[9]    = {0x28, 0x34, 0x38, 0x44, 0x48, 0x50, 0x5c, 0x64, 0x68};
const s8 AETargetMeanTab_720P[9]    = {0x1C, 0x28, 0x2C, 0x38, 0x3C, 0x44, 0x50, 0x58, 0x5C};
const s8 AETargetMeanTab_VGA[9]     = {0x05, 0x10, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74, 0x84};
#elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP) || (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
#define TAR_MID_D 0x3E
#define TAR_MID_N 0x2B

const s8 AETargetMeanTab_720P[9]    = {TAR_MID_D-32, TAR_MID_D-20, TAR_MID_D-16, TAR_MID_D-4, TAR_MID_D, TAR_MID_D+8, TAR_MID_D+20, TAR_MID_D+28, TAR_MID_D+32};
const s8 AETargetMeanTab_720P_N[9]  = {TAR_MID_N-32, TAR_MID_N-20, TAR_MID_N-16, TAR_MID_N-4, TAR_MID_N, TAR_MID_N+8, TAR_MID_N+20, TAR_MID_N+28, TAR_MID_N+32};
const s8 AETargetMeanTab_VGA[9]     = {0x05, 0x10, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74, 0x84};
#else
const s8 AETargetMeanTab[9] = {0x00, 0x0c, 0x1c, 0x2c, 0x3c, 0x4c, 0x5c, 0x6c, 0x7c};
#endif

DEF_AE_Tab AE_EG_Tab_VideoPrev_5060Hz[2][2][AE_E_Table_Num*Gain_Level];  //AE Index Table
u8 AE_Flicker_50_60_sel = SENSOR_AE_FLICKER_60HZ;

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

    OSTaskCreate(SIU_TASK, SIU_TASK_PARAMETER, SIU_TASK_STACK, SIU_TASK_PRIORITY_PVW); 
    siuSuspendTask();

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
	rfiuVideoInFrameRate=siuSensorInit(mode,zoomFactor);
	
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
		 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
          (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
          (CHIP_OPTION == CHIP_A1026A))
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		 #else
		        SIU_FRAM_VSYNC |
		 #endif   
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
        		SIU_VSYNC_ACT_LO |                      //Vsync active low
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
		 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
          (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
          (CHIP_OPTION == CHIP_A1026A))
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		 #else
		        SIU_FRAM_VSYNC |
		 #endif          		
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
#if ((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION  == MR8120S_TX_GCT_VM00)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)||\
    (HW_BOARD_OPTION == MR8120_TX_GCT_VM00))
void SetHM1375_720P_15FPS_GCT_DAY(void)
{

    i2cWrite_HM1375(0x0870, 0x00); 
    i2cWrite_HM1375(0x0871, 0x14); 
    i2cWrite_HM1375(0x0872, 0x01); 
    i2cWrite_HM1375(0x0873, 0x20); 
    i2cWrite_HM1375(0x0874, 0x00); 
    i2cWrite_HM1375(0x0875, 0x14); 
    i2cWrite_HM1375(0x0876, 0x00); 
    i2cWrite_HM1375(0x0877, 0xEC);

    //Hm1375 ,1280 x 720
    if(AE_Flicker_50_60_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
    i2cWrite_HM1375(0x04C0, 0x88);
    i2cWrite_HM1375(0x04C1, 0x10);
    i2cWrite_HM1375(0x04B0, 0x5D);
    i2cWrite_HM1375(0x0370, 0x02);
    i2cWrite_HM1375(0x0122, 0xCB);
    //i2cWrite_HM1375(0x0125, 0xD2);
    i2cWrite_HM1375(0x0125, 0xDE);  // 抑制假色
    i2cWrite_HM1375(0x0374, 0x60);

    i2cWrite_HM1375(0x05B0, 0x06);
    i2cWrite_HM1375(0x05B1, 0x02);
    i2cWrite_HM1375(0x0376, 0x02);
    i2cWrite_HM1375(0x02a0, 0x06);
    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // HD不變
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab_720P[siuY_TargetIndex]);  
        i2cWrite_HM1375(0x0381, AETargetMeanTab_720P[siuY_TargetIndex]+12);
        i2cWrite_HM1375(0x0382, AETargetMeanTab_720P[siuY_TargetIndex]-12);
    }
    else    // VGA調亮
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab_VGA[siuY_TargetIndex]);  
        i2cWrite_HM1375(0x0381, AETargetMeanTab_VGA[siuY_TargetIndex]+12);
        i2cWrite_HM1375(0x0382, AETargetMeanTab_VGA[siuY_TargetIndex]-12);
    }


    i2cWrite_HM1375(0x0870, 0x00);
    i2cWrite_HM1375(0x0871, 0x14);
    i2cWrite_HM1375(0x0872, 0x01);
    i2cWrite_HM1375(0x0873, 0x20);
    i2cWrite_HM1375(0x0874, 0x00);
    i2cWrite_HM1375(0x0875, 0x14);
    i2cWrite_HM1375(0x0876, 0x00);
    i2cWrite_HM1375(0x0877, 0xEC);
    ;//Gamma 
    i2cWrite_HM1375(0x0280, 0x00);
    i2cWrite_HM1375(0x0281, 0x4D);
    i2cWrite_HM1375(0x0282, 0x00);
    i2cWrite_HM1375(0x0283, 0x85);
    i2cWrite_HM1375(0x0284, 0x00);
    i2cWrite_HM1375(0x0285, 0xE8);
    i2cWrite_HM1375(0x0286, 0x01);
    i2cWrite_HM1375(0x0287, 0x91);
    i2cWrite_HM1375(0x0288, 0x01);
    i2cWrite_HM1375(0x0289, 0xD0);
    i2cWrite_HM1375(0x028A, 0x02);
    i2cWrite_HM1375(0x028B, 0x0A);
    i2cWrite_HM1375(0x028C, 0x02);
    i2cWrite_HM1375(0x028D, 0x32);
    i2cWrite_HM1375(0x028E, 0x02);
    i2cWrite_HM1375(0x028F, 0x58);
    i2cWrite_HM1375(0x0290, 0x02);
    i2cWrite_HM1375(0x0291, 0x78);
    i2cWrite_HM1375(0x0292, 0x02);
    i2cWrite_HM1375(0x0293, 0x95);
    i2cWrite_HM1375(0x0294, 0x02);
    i2cWrite_HM1375(0x0295, 0xCC);
    i2cWrite_HM1375(0x0296, 0x02);
    i2cWrite_HM1375(0x0297, 0xFA);
    i2cWrite_HM1375(0x0298, 0x03);
    i2cWrite_HM1375(0x0299, 0x49);
    i2cWrite_HM1375(0x029A, 0x03);
    i2cWrite_HM1375(0x029B, 0x87);
    i2cWrite_HM1375(0x029C, 0x03);
    i2cWrite_HM1375(0x029D, 0xBA);
    i2cWrite_HM1375(0x029E, 0x00);
    i2cWrite_HM1375(0x029F, 0x5C);

    i2cWrite_HM1375(0x0815, 0x00);
    i2cWrite_HM1375(0x0816, 0x4C);
    i2cWrite_HM1375(0x0817, 0x00);
    i2cWrite_HM1375(0x0818, 0x7B);
    i2cWrite_HM1375(0x0819, 0x00);
    i2cWrite_HM1375(0x081A, 0xCA);
    i2cWrite_HM1375(0x081B, 0x01);
    i2cWrite_HM1375(0x081C, 0x3E);
    i2cWrite_HM1375(0x081D, 0x01);
    i2cWrite_HM1375(0x081E, 0x77);
    i2cWrite_HM1375(0x081F, 0x01);
    i2cWrite_HM1375(0x0820, 0xAA);
    i2cWrite_HM1375(0x0821, 0x01);
    i2cWrite_HM1375(0x0822, 0xCE);
    i2cWrite_HM1375(0x0823, 0x01);
    i2cWrite_HM1375(0x0824, 0xEE);
    i2cWrite_HM1375(0x0825, 0x02);
    i2cWrite_HM1375(0x0826, 0x16);
    i2cWrite_HM1375(0x0827, 0x02);
    i2cWrite_HM1375(0x0828, 0x33);
    i2cWrite_HM1375(0x0829, 0x02);
    i2cWrite_HM1375(0x082A, 0x65);
    i2cWrite_HM1375(0x082B, 0x02);
    i2cWrite_HM1375(0x082C, 0x91);
    i2cWrite_HM1375(0x082D, 0x02);
    i2cWrite_HM1375(0x082E, 0xDC);
    i2cWrite_HM1375(0x082F, 0x03);
    i2cWrite_HM1375(0x0830, 0x28);
    i2cWrite_HM1375(0x0831, 0x03);
    i2cWrite_HM1375(0x0832, 0x74);
    i2cWrite_HM1375(0x0833, 0x03);
    i2cWrite_HM1375(0x0834, 0xFF);

    i2cWrite_HM1375(0x0100, 0x01);
    i2cWrite_HM1375(0x0101, 0x01);
    i2cWrite_HM1375(0x0000, 0x01);
}
#endif

#if ((HW_BOARD_OPTION == MR8120_TX_RDI)|| (HW_BOARD_OPTION == MR8120_TX_RDI_CA672) ||  (HW_BOARD_OPTION == MR8120_TX_RDI_CA532) ||\
    (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652) )
void SetHM1375_720P_15FPS_RDI_DAY(void)
{
    i2cWrite_HM1375(0x0870, 0x00); 
    i2cWrite_HM1375(0x0871, 0x14); 
    i2cWrite_HM1375(0x0872, 0x01); 
    i2cWrite_HM1375(0x0873, 0x20); 
    i2cWrite_HM1375(0x0874, 0x00); 
    i2cWrite_HM1375(0x0875, 0x14); 
    i2cWrite_HM1375(0x0876, 0x00); 
    i2cWrite_HM1375(0x0877, 0xEC);

    //Hm1375 ,1280 x 720
    if(AE_Flicker_50_60_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
    i2cWrite_HM1375(0x04C0, 0x88);
    i2cWrite_HM1375(0x04C1, 0x10);
    i2cWrite_HM1375(0x04B0, 0x5D);
    i2cWrite_HM1375(0x0370, 0x02);
    i2cWrite_HM1375(0x0122, 0xCB);
    //i2cWrite_HM1375(0x0125, 0xD2);
    i2cWrite_HM1375(0x0125, 0xDE);  // 抑制假色
    i2cWrite_HM1375(0x0374, 0x30);

    i2cWrite_HM1375(0x05B0, 0x06);
    i2cWrite_HM1375(0x05B1, 0x02);
    i2cWrite_HM1375(0x0376, 0x02);
    i2cWrite_HM1375(0x02a0, 0x06);

  // 20140115
  #if (((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT != 7) && (UI_PROJ_OPT != 4)&&(UI_PROJ_OPT != 2)&&(UI_PROJ_OPT != 5)&&(UI_PROJ_OPT != 8))) ||\
      (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652) )
    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // HD不變
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab_720P[siuY_TargetIndex]);  
        i2cWrite_HM1375(0x0381, AETargetMeanTab_720P[siuY_TargetIndex]+12);
        i2cWrite_HM1375(0x0382, AETargetMeanTab_720P[siuY_TargetIndex]-12);
    }
    else    // VGA調亮
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab_VGA[siuY_TargetIndex]);  
        i2cWrite_HM1375(0x0381, AETargetMeanTab_VGA[siuY_TargetIndex]+12);
        i2cWrite_HM1375(0x0382, AETargetMeanTab_VGA[siuY_TargetIndex]-12);
    }
  #else
    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // HD不變
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);  
        i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
        i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
    }
    else    // VGA調亮
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex] + 5);  
        i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+17);
        i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-7);
    }
  #endif

    i2cWrite_HM1375(0x0870, 0x00);
    i2cWrite_HM1375(0x0871, 0x14);
    i2cWrite_HM1375(0x0872, 0x01);
    i2cWrite_HM1375(0x0873, 0x20);
    i2cWrite_HM1375(0x0874, 0x00);
    i2cWrite_HM1375(0x0875, 0x14);
    i2cWrite_HM1375(0x0876, 0x00);
    i2cWrite_HM1375(0x0877, 0xEC);
    //Gamma
    i2cWrite_HM1375(0x0280, 0x00);
    i2cWrite_HM1375(0x0281, 0x4D);
    i2cWrite_HM1375(0x0282, 0x00);
    i2cWrite_HM1375(0x0283, 0x85);
    i2cWrite_HM1375(0x0284, 0x00);
    i2cWrite_HM1375(0x0285, 0xE8);
    i2cWrite_HM1375(0x0286, 0x01);
    i2cWrite_HM1375(0x0287, 0x91);
    i2cWrite_HM1375(0x0288, 0x01);
    i2cWrite_HM1375(0x0289, 0xD0);
    i2cWrite_HM1375(0x028A, 0x02);
    i2cWrite_HM1375(0x028B, 0x0A);
    i2cWrite_HM1375(0x028C, 0x02);
    i2cWrite_HM1375(0x028D, 0x32);
    i2cWrite_HM1375(0x028E, 0x02);
    i2cWrite_HM1375(0x028F, 0x58);
    i2cWrite_HM1375(0x0290, 0x02);
    i2cWrite_HM1375(0x0291, 0x78);
    i2cWrite_HM1375(0x0292, 0x02);
    i2cWrite_HM1375(0x0293, 0x95);
    i2cWrite_HM1375(0x0294, 0x02);
    i2cWrite_HM1375(0x0295, 0xCC);
    i2cWrite_HM1375(0x0296, 0x02);
    i2cWrite_HM1375(0x0297, 0xFA);
    i2cWrite_HM1375(0x0298, 0x03);
    i2cWrite_HM1375(0x0299, 0x49);
    i2cWrite_HM1375(0x029A, 0x03);
    i2cWrite_HM1375(0x029B, 0x87);
    i2cWrite_HM1375(0x029C, 0x03);
    i2cWrite_HM1375(0x029D, 0xBA);
    i2cWrite_HM1375(0x029E, 0x00);
    i2cWrite_HM1375(0x029F, 0x5C);

    i2cWrite_HM1375(0x0815, 0x00);
    i2cWrite_HM1375(0x0816, 0x4C);
    i2cWrite_HM1375(0x0817, 0x00);
    i2cWrite_HM1375(0x0818, 0x7B);
    i2cWrite_HM1375(0x0819, 0x00);
    i2cWrite_HM1375(0x081A, 0xCA);
    i2cWrite_HM1375(0x081B, 0x01);
    i2cWrite_HM1375(0x081C, 0x3E);
    i2cWrite_HM1375(0x081D, 0x01);
    i2cWrite_HM1375(0x081E, 0x77);
    i2cWrite_HM1375(0x081F, 0x01);
    i2cWrite_HM1375(0x0820, 0xAA);
    i2cWrite_HM1375(0x0821, 0x01);
    i2cWrite_HM1375(0x0822, 0xCE);
    i2cWrite_HM1375(0x0823, 0x01);
    i2cWrite_HM1375(0x0824, 0xEE);
    i2cWrite_HM1375(0x0825, 0x02);
    i2cWrite_HM1375(0x0826, 0x16);
    i2cWrite_HM1375(0x0827, 0x02);
    i2cWrite_HM1375(0x0828, 0x33);
    i2cWrite_HM1375(0x0829, 0x02);
    i2cWrite_HM1375(0x082A, 0x65);
    i2cWrite_HM1375(0x082B, 0x02);
    i2cWrite_HM1375(0x082C, 0x91);
    i2cWrite_HM1375(0x082D, 0x02);
    i2cWrite_HM1375(0x082E, 0xDC);
    i2cWrite_HM1375(0x082F, 0x03);
    i2cWrite_HM1375(0x0830, 0x28);
    i2cWrite_HM1375(0x0831, 0x03);
    i2cWrite_HM1375(0x0832, 0x74);
    i2cWrite_HM1375(0x0833, 0x03);
    i2cWrite_HM1375(0x0834, 0xFF);

    i2cWrite_HM1375(0x0100, 0x01);
    i2cWrite_HM1375(0x0101, 0x01);
    i2cWrite_HM1375(0x0000, 0x01);
}
#endif

#if( (HW_BOARD_OPTION  == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
void SetHM1375_720P_15FPS_Mayon_DAY(void)
{
    i2cWrite_HM1375(0x0870, 0x00); 
    i2cWrite_HM1375(0x0871, 0x14); 
    i2cWrite_HM1375(0x0872, 0x01); 
    i2cWrite_HM1375(0x0873, 0x20); 
    i2cWrite_HM1375(0x0874, 0x00); 
    i2cWrite_HM1375(0x0875, 0x14); 
    i2cWrite_HM1375(0x0876, 0x00); 
    i2cWrite_HM1375(0x0877, 0xEC);

    //Hm1375 ,1280 x 720
    //------Flicker adjustment-----//
    if(AE_Flicker_50_60_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }

    i2cWrite_HM1375(0x04C0, 0x88);
    i2cWrite_HM1375(0x04C1, 0x10);
    i2cWrite_HM1375(0x04B0, 0x5D);
    i2cWrite_HM1375(0x0370, 0x02);
    i2cWrite_HM1375(0x0122, 0xCB);
    //i2cWrite_HM1375(0x0125, 0xD2);
    i2cWrite_HM1375(0x0125, 0xDE);  // 抑制假色
    i2cWrite_HM1375(0x0374, 0x30);
    i2cWrite_HM1375(0x05B0, 0x06);
    i2cWrite_HM1375(0x05B1, 0x02);
    i2cWrite_HM1375(0x0376, 0x02);
    i2cWrite_HM1375(0x02a0, 0x06);
    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // HD不變
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);  
        i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
        i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
    }
    else    // VGA調亮
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex] + 5);  
        i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+17);
        i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-7);
    }
    i2cWrite_HM1375(0x0870, 0x00);
    i2cWrite_HM1375(0x0871, 0x14);
    i2cWrite_HM1375(0x0872, 0x01);
    i2cWrite_HM1375(0x0873, 0x20);
    i2cWrite_HM1375(0x0874, 0x00);
    i2cWrite_HM1375(0x0875, 0x14);
    i2cWrite_HM1375(0x0876, 0x00);
    i2cWrite_HM1375(0x0877, 0xEC);
    ;//Gamma 
    i2cWrite_HM1375(0x0280, 0x00);
    i2cWrite_HM1375(0x0281, 0x4D);
    i2cWrite_HM1375(0x0282, 0x00);
    i2cWrite_HM1375(0x0283, 0x85);
    i2cWrite_HM1375(0x0284, 0x00);
    i2cWrite_HM1375(0x0285, 0xE8);
    i2cWrite_HM1375(0x0286, 0x01);
    i2cWrite_HM1375(0x0287, 0x91);
    i2cWrite_HM1375(0x0288, 0x01);
    i2cWrite_HM1375(0x0289, 0xD0);
    i2cWrite_HM1375(0x028A, 0x02);
    i2cWrite_HM1375(0x028B, 0x0A);
    i2cWrite_HM1375(0x028C, 0x02);
    i2cWrite_HM1375(0x028D, 0x32);
    i2cWrite_HM1375(0x028E, 0x02);
    i2cWrite_HM1375(0x028F, 0x58);
    i2cWrite_HM1375(0x0290, 0x02);
    i2cWrite_HM1375(0x0291, 0x78);
    i2cWrite_HM1375(0x0292, 0x02);
    i2cWrite_HM1375(0x0293, 0x95);
    i2cWrite_HM1375(0x0294, 0x02);
    i2cWrite_HM1375(0x0295, 0xCC);
    i2cWrite_HM1375(0x0296, 0x02);
    i2cWrite_HM1375(0x0297, 0xFA);
    i2cWrite_HM1375(0x0298, 0x03);
    i2cWrite_HM1375(0x0299, 0x49);
    i2cWrite_HM1375(0x029A, 0x03);
    i2cWrite_HM1375(0x029B, 0x87);
    i2cWrite_HM1375(0x029C, 0x03);
    i2cWrite_HM1375(0x029D, 0xBA);
    i2cWrite_HM1375(0x029E, 0x00);
    i2cWrite_HM1375(0x029F, 0x5C);

    i2cWrite_HM1375(0x0815, 0x00);
    i2cWrite_HM1375(0x0816, 0x4C);
    i2cWrite_HM1375(0x0817, 0x00);
    i2cWrite_HM1375(0x0818, 0x7B);
    i2cWrite_HM1375(0x0819, 0x00);
    i2cWrite_HM1375(0x081A, 0xCA);
    i2cWrite_HM1375(0x081B, 0x01);
    i2cWrite_HM1375(0x081C, 0x3E);
    i2cWrite_HM1375(0x081D, 0x01);
    i2cWrite_HM1375(0x081E, 0x77);
    i2cWrite_HM1375(0x081F, 0x01);
    i2cWrite_HM1375(0x0820, 0xAA);
    i2cWrite_HM1375(0x0821, 0x01);
    i2cWrite_HM1375(0x0822, 0xCE);
    i2cWrite_HM1375(0x0823, 0x01);
    i2cWrite_HM1375(0x0824, 0xEE);
    i2cWrite_HM1375(0x0825, 0x02);
    i2cWrite_HM1375(0x0826, 0x16);
    i2cWrite_HM1375(0x0827, 0x02);
    i2cWrite_HM1375(0x0828, 0x33);
    i2cWrite_HM1375(0x0829, 0x02);
    i2cWrite_HM1375(0x082A, 0x65);
    i2cWrite_HM1375(0x082B, 0x02);
    i2cWrite_HM1375(0x082C, 0x91);
    i2cWrite_HM1375(0x082D, 0x02);
    i2cWrite_HM1375(0x082E, 0xDC);
    i2cWrite_HM1375(0x082F, 0x03);
    i2cWrite_HM1375(0x0830, 0x28);
    i2cWrite_HM1375(0x0831, 0x03);
    i2cWrite_HM1375(0x0832, 0x74);
    i2cWrite_HM1375(0x0833, 0x03);
    i2cWrite_HM1375(0x0834, 0xFF);

    i2cWrite_HM1375(0x0100, 0x01);
    i2cWrite_HM1375(0x0101, 0x01);
    i2cWrite_HM1375(0x0000, 0x01);
}
#endif

#if(HW_BOARD_OPTION == MR8120_TX_TRANWO)

void SetHM1375_720P_15FPS_TRANWO(void)
{
    int i;
	u8	data;
    int count;
    
    DEBUG_SIU("SetHM1375_720P_15FPS_TRANWO()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);

        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    i2cWrite_HM1375(0x0022, 0x01);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);
    i2cWrite_HM1375(0x0022, 0x00);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);

    i2cWrite_HM1375(0x0004, 0x18);  // hw reset mode
    
    i2cWrite_HM1375(0x000C, 0x04);  // MODE      _Reserved bit
    i2cWrite_HM1375(0x0006, 0x08);
    //i2cWrite_HM1375(0x0006, 0x0B);  // Flip
    i2cWrite_HM1375(0x000A, 0x00);  // CHIPCFG _Full frame, chip default 00
    i2cWrite_HM1375(0x000F, 0x10);  // IMGCFG
    i2cWrite_HM1375(0x0012, 0x01);  // RESERVED 
    i2cWrite_HM1375(0x0013, 0x02);  // RESERVED
    i2cWrite_HM1375(0x0015, 0x01);  // INTG_H    _Set up integration
    i2cWrite_HM1375(0x0016, 0x00);  // INTG_L    _Set up integration
    i2cWrite_HM1375(0x0018, 0x00);  // AGAIN     _Set up coarse gain
    i2cWrite_HM1375(0x001D, 0x40);  // DGGAIN    _Set up fine gain
    i2cWrite_HM1375(0x0020, 0x10);  // OPRTCFG   _AE Gain enabled, Pclk transition on falling edge
#if (HW_BOARD_OPTION == MR8211_ZINWELL)
    i2cWrite_HM1375(0x0023, 0x00);  // IOCNTR    _IO pad drive strength, IO & PCLK pad 電流都降到最低 for EMI testing
#else
    i2cWrite_HM1375(0x0023, 0x43);  // IOCNTR    _IO pad drive strength
#endif
    i2cWrite_HM1375(0x0024, 0x20);  // CKCNTR    _Reserved bit
//#if (HM1375_FRAME_RATE == 30)
#if 0
    i2cWrite_HM1375(0x0025, 0x00);  // CKCFG     _Select PLL clock -> 24MHz to 78 MHz
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0025, 0x01);  // CKCFG     _Select PLL clock -> 24MHz to 39 MHz
#endif
    i2cWrite_HM1375(0x0026, 0x6C);
    //i2cWrite_HM1375(0x0027, 0x30);  // OPORTCNTR _YUV output, ITU601 Disable, YCbYCr...
    i2cWrite_HM1375(0x0027, 0x10);  // OPORTCNTR _YUV output, ITU601 Disable, CbYCrY...
    i2cWrite_HM1375(0x0028, 0x01);  // CKMUL     _Reserved bit
    i2cWrite_HM1375(0x0030, 0x00);  // EDRCFG    _Disable EDR mode
    i2cWrite_HM1375(0x0034, 0x0E);  // EDRBLEND  _Set EDR blend to 0.875
    i2cWrite_HM1375(0x0035, 0x01);  // RESERVED
    i2cWrite_HM1375(0x0036, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0038, 0x02);  // EDR2AGAIN _Set up EDR2 coarse gain
    i2cWrite_HM1375(0x0039, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003A, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003B, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003C, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003D, 0x40);  // EDR2DGAIN _Set up EDR2 fine gain
    i2cWrite_HM1375(0x003F, 0x14);  // RESERVED      less gradient effect

    //----------------------------------------------
    //BLACK LEVEL CONTROL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0040, 0x10);  // BLCTGT    _Black level target
    i2cWrite_HM1375(0x0044, 0x07);  // BLCCFG    _BLC configuration enable, reserved bit

    //----------------------------------------------
    //RESERVED REGISTERS FOR SENSOR READOUT_TIMING
    //----------------------------------------------

    i2cWrite_HM1375(0x0045, 0x35);  // RESERVED       
    i2cWrite_HM1375(0x0048, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x004E, 0xFF);  // RESERVED 
    i2cWrite_HM1375(0x0070, 0x22);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0071, 0x3F);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0072, 0x22);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0073, 0x30);  // RESERVED    - 0920 updated by Willie
    i2cWrite_HM1375(0x0074, 0x13);  // RESERVED
    i2cWrite_HM1375(0x0075, 0x40);  // RESERVED
    i2cWrite_HM1375(0x0076, 0x24);  // RESERVED
    i2cWrite_HM1375(0x0078, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x007A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x007B, 0x14);  // RESERVED
    i2cWrite_HM1375(0x007C, 0x10);  // RESERVED
    i2cWrite_HM1375(0x0080, 0xC9);  // RESERVED
    i2cWrite_HM1375(0x0081, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0082, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0083, 0xB0);  // RESERVED
    i2cWrite_HM1375(0x0084, 0x60);  // RESERVED
    i2cWrite_HM1375(0x0086, 0x3E);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0087, 0x70);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0088, 0x11);  // RESERVED
    i2cWrite_HM1375(0x0089, 0x3C);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x008A, 0x87);  // RESERVED   // Rev.E updated by Willie
    i2cWrite_HM1375(0x008D, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0090, 0x07);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0091, 0x09);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0092, 0x0C);  
    i2cWrite_HM1375(0x0093, 0x0C);  
    i2cWrite_HM1375(0x0094, 0x0C);  
    i2cWrite_HM1375(0x0095, 0x0C);  
    i2cWrite_HM1375(0x0096, 0x01);  // AGAIN gain (nonEDR) table update for CFPN improvement 0824
    i2cWrite_HM1375(0x0097, 0x00);  
    i2cWrite_HM1375(0x0098, 0x04);  
    i2cWrite_HM1375(0x0099, 0x08);  
    i2cWrite_HM1375(0x009A, 0x0C);  


    //----------------------------------------------
    //IMAGE PIPELINE PROCESSING CONTROL
    //----------------------------------------------

    //------Flicker adjustment-----//
    if(AE_Flicker_50_60_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
    i2cWrite_HM1375(0x0121, 0x81);  // IPPCNTR2
    i2cWrite_HM1375(0x0122, 0xEB);  // IPPCNTR3
    i2cWrite_HM1375(0x0123, 0x29);  // IPPCNTR4
    i2cWrite_HM1375(0x0124, 0x50);  // CCMCNTR
    i2cWrite_HM1375(0x0125, 0xDE);  // IPPCNTR5
    i2cWrite_HM1375(0x0126, 0xB1);  // IPPCNTR6

    //----------------------------------------------
    //FLARE CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x013D, 0x0F);  
    i2cWrite_HM1375(0x013E, 0x0F);  
    i2cWrite_HM1375(0x013F, 0x0F);  

    //----------------------------------------------
    //BAD PIXEL CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0140, 0x14); 
    i2cWrite_HM1375(0x0141, 0x0A);
    i2cWrite_HM1375(0x0142, 0x14);
    i2cWrite_HM1375(0x0143, 0x0A);

    //----------------------------------------------
    //RESERVED
    //----------------------------------------------

    i2cWrite_HM1375(0x0144, 0x08);  // RESERVED
    i2cWrite_HM1375(0x0145, 0x04);  // RESERVED
    i2cWrite_HM1375(0x0146, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0147, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x0148, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0149, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x014A, 0x96);  // RESERVED
    i2cWrite_HM1375(0x014B, 0xC8);  // RESERVED

    //----------------------------------------------
    //SHARPENING CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0150, 0x14);  // SHPTHLR   
    i2cWrite_HM1375(0x0151, 0x30);  // SHPTHLR_A 
    i2cWrite_HM1375(0x0152, 0x54);  // SHPTHHR   
    i2cWrite_HM1375(0x0153, 0x70);  // SHPTHHR_A 
    i2cWrite_HM1375(0x0154, 0x14);  // SHPTHLG   
    i2cWrite_HM1375(0x0155, 0x30);  // SHPTHLG_A 
    i2cWrite_HM1375(0x0156, 0x54);  // SHPTHHG   
    i2cWrite_HM1375(0x0157, 0x70);  // SHPTHHG_A 
    i2cWrite_HM1375(0x0158, 0x14);  // SHPTHLB   
    i2cWrite_HM1375(0x0159, 0x30);  // SHPTHLB_A 
    i2cWrite_HM1375(0x015A, 0x54);  // SHPTHHB   
    i2cWrite_HM1375(0x015B, 0x70);  // SHPTHHB_A 
    i2cWrite_HM1375(0x015C, 0x30);  // SHPSTR    _Sharpness strength
    i2cWrite_HM1375(0x015D, 0x00);  // SHPSTR_A  _sharpness strength_Alpha0


    //----------------------------------------------
    // NOISE FILTER CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x01D8, 0x20);  // NFHTHG
    i2cWrite_HM1375(0x01D9, 0x08);  // NFLTHG
    i2cWrite_HM1375(0x01DA, 0x20);  // NFHTHB
    i2cWrite_HM1375(0x01DB, 0x08);  // NFLTHB
    i2cWrite_HM1375(0x01DC, 0x20);  // NFHTHR
    i2cWrite_HM1375(0x01DD, 0x08);  // NFLTHR
    i2cWrite_HM1375(0x01DE, 0x50);  // NFHTHG_A
    i2cWrite_HM1375(0x01E0, 0x50);  // NFHTHB_A
    i2cWrite_HM1375(0x01E2, 0x50);  // NFHTHR_A
    i2cWrite_HM1375(0x01E4, 0x10);  // NFSTR
    i2cWrite_HM1375(0x01E5, 0x10);  // NFSTR_A 
    i2cWrite_HM1375(0x01E6, 0x02);  // NFSTR_OUTA
    i2cWrite_HM1375(0x01E7, 0x10);  // NFMTHG   
    i2cWrite_HM1375(0x01E8, 0x10);  // NFMTHB  
    i2cWrite_HM1375(0x01E9, 0x10);  // NFMTHR
    i2cWrite_HM1375(0x01EC, 0x28);  // NFMTHR_A

    //----------------------------------------------
    //LENS SHADING CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0220, 0x00);
    i2cWrite_HM1375(0x0221, 0xA0);
    i2cWrite_HM1375(0x0222, 0x00);
    i2cWrite_HM1375(0x0223, 0x80);
    i2cWrite_HM1375(0x0224, 0x80);
    i2cWrite_HM1375(0x0225, 0x00);
    i2cWrite_HM1375(0x0226, 0x80);
    i2cWrite_HM1375(0x0227, 0x80);
    i2cWrite_HM1375(0x0228, 0x00);
    i2cWrite_HM1375(0x0229, 0x80);
    i2cWrite_HM1375(0x022A, 0x80);
    i2cWrite_HM1375(0x022B, 0x00);
    i2cWrite_HM1375(0x022C, 0x80);
    i2cWrite_HM1375(0x022D, 0x12);
    i2cWrite_HM1375(0x022E, 0x10);
    i2cWrite_HM1375(0x022F, 0x12);
    i2cWrite_HM1375(0x0230, 0x10);
    i2cWrite_HM1375(0x0231, 0x12);
    i2cWrite_HM1375(0x0232, 0x10);
    i2cWrite_HM1375(0x0233, 0x12);
    i2cWrite_HM1375(0x0234, 0x10);
    i2cWrite_HM1375(0x0235, 0x88);
    i2cWrite_HM1375(0x0236, 0x02);
    i2cWrite_HM1375(0x0237, 0x88);
    i2cWrite_HM1375(0x0238, 0x02);
    i2cWrite_HM1375(0x0239, 0x88);
    i2cWrite_HM1375(0x023A, 0x02);
    i2cWrite_HM1375(0x023B, 0x88);
    i2cWrite_HM1375(0x023C, 0x02);
    i2cWrite_HM1375(0x023D, 0x04);
    i2cWrite_HM1375(0x023E, 0x02);
    i2cWrite_HM1375(0x023F, 0x04);
    i2cWrite_HM1375(0x0240, 0x02);
    i2cWrite_HM1375(0x0241, 0x04);
    i2cWrite_HM1375(0x0242, 0x02);
    i2cWrite_HM1375(0x0243, 0x04);
    i2cWrite_HM1375(0x0244, 0x02);
    i2cWrite_HM1375(0x0251, 0x10);

    //----------------------------------------------
    //GAMMA CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0280, 0x00);  // normal Gamma
    i2cWrite_HM1375(0x0281, 0x46);  
    i2cWrite_HM1375(0x0282, 0x00);  
    i2cWrite_HM1375(0x0283, 0x77);  
    i2cWrite_HM1375(0x0284, 0x00);  
    i2cWrite_HM1375(0x0285, 0xCC);  
    i2cWrite_HM1375(0x0286, 0x01);  
    i2cWrite_HM1375(0x0287, 0x62);  
    i2cWrite_HM1375(0x0288, 0x01);  
    i2cWrite_HM1375(0x0289, 0x9B);  
    i2cWrite_HM1375(0x028A, 0x01);  
    i2cWrite_HM1375(0x028B, 0xCF);  
    i2cWrite_HM1375(0x028C, 0x01);  
    i2cWrite_HM1375(0x028D, 0xF6);  
    i2cWrite_HM1375(0x028E, 0x02);  
    i2cWrite_HM1375(0x028F, 0x1B);  
    i2cWrite_HM1375(0x0290, 0x02);  
    i2cWrite_HM1375(0x0291, 0x3B);  
    i2cWrite_HM1375(0x0292, 0x02);  
    i2cWrite_HM1375(0x0293, 0x59);  
    i2cWrite_HM1375(0x0294, 0x02);  
    i2cWrite_HM1375(0x0295, 0x91);  
    i2cWrite_HM1375(0x0296, 0x02);  
    i2cWrite_HM1375(0x0297, 0xC3);  
    i2cWrite_HM1375(0x0298, 0x03);  
    i2cWrite_HM1375(0x0299, 0x1B);  
    i2cWrite_HM1375(0x029A, 0x03);  
    i2cWrite_HM1375(0x029B, 0x65);  
    i2cWrite_HM1375(0x029C, 0x03);  
    i2cWrite_HM1375(0x029D, 0xA5);  // updated by MH 06/17
    i2cWrite_HM1375(0x029E, 0x00);  // slope high byte    
    i2cWrite_HM1375(0x029F, 0x78);  // slope low byte
    i2cWrite_HM1375(0x02A0, 0x04);  // GAM_A

    //----------------------------------------------
    //COLOR CORRECTION MATRIX
    //----------------------------------------------

    i2cWrite_HM1375(0x02C0, 0x80);  //CCM00_L    _Adjust for D65 color temperature
    i2cWrite_HM1375(0x02C1, 0x01);  //CCM00_H
    i2cWrite_HM1375(0x02C2, 0x71);  
    i2cWrite_HM1375(0x02C3, 0x04);  
    i2cWrite_HM1375(0x02C4, 0x0F);  
    i2cWrite_HM1375(0x02C5, 0x04);  
    i2cWrite_HM1375(0x02C6, 0x3D);  
    i2cWrite_HM1375(0x02C7, 0x04);  
    i2cWrite_HM1375(0x02C8, 0x94);  
    i2cWrite_HM1375(0x02C9, 0x01);  
    i2cWrite_HM1375(0x02CA, 0x57);  
    i2cWrite_HM1375(0x02CB, 0x04);  
    i2cWrite_HM1375(0x02CC, 0x0F);  
    i2cWrite_HM1375(0x02CD, 0x04);  
    i2cWrite_HM1375(0x02CE, 0x8F);  
    i2cWrite_HM1375(0x02CF, 0x04);  
    i2cWrite_HM1375(0x02D0, 0x9E);  
    i2cWrite_HM1375(0x02D1, 0x01);  
    i2cWrite_HM1375(0x02E0, 0x06);  // CCM_A     _reduce to 0.267x
    i2cWrite_HM1375(0x02E1, 0xC0);  // RESERVED
    i2cWrite_HM1375(0x02E2, 0xE0);  // RESERVED
    i2cWrite_HM1375(0x02F0, 0x48);  // ACCM00_L  _Adjust for IncA color temperature
    i2cWrite_HM1375(0x02F1, 0x01);  // ACCM00_H
    i2cWrite_HM1375(0x02F2, 0x32);  
    i2cWrite_HM1375(0x02F3, 0x04);  
    i2cWrite_HM1375(0x02F4, 0x16);  
    i2cWrite_HM1375(0x02F5, 0x04);  
    i2cWrite_HM1375(0x02F6, 0x52);  
    i2cWrite_HM1375(0x02F7, 0x04);  
    i2cWrite_HM1375(0x02F8, 0xAA);  
    i2cWrite_HM1375(0x02F9, 0x01);  
    i2cWrite_HM1375(0x02FA, 0x58);  
    i2cWrite_HM1375(0x02FB, 0x04);  
    i2cWrite_HM1375(0x02FC, 0x56);  
    i2cWrite_HM1375(0x02FD, 0x04);  
    i2cWrite_HM1375(0x02FE, 0xDD);  
    i2cWrite_HM1375(0x02FF, 0x04);  
    i2cWrite_HM1375(0x0300, 0x33);  
    i2cWrite_HM1375(0x0301, 0x02);  

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE WINDOW CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0324, 0x00);
    i2cWrite_HM1375(0x0325, 0x01);

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE DETECTION AND LIMITS
    //----------------------------------------------

    i2cWrite_HM1375(0x0333, 0x00);  
    i2cWrite_HM1375(0x0334, 0x00);  
    i2cWrite_HM1375(0x0335, 0x86);  
    i2cWrite_HM1375(0x0340, 0x40);      
    i2cWrite_HM1375(0x0341, 0x44);  
    i2cWrite_HM1375(0x0342, 0x4A);  
    i2cWrite_HM1375(0x0343, 0x2B);  
    i2cWrite_HM1375(0x0344, 0x94);  
    i2cWrite_HM1375(0x0345, 0x3F);  
    i2cWrite_HM1375(0x0346, 0x8E);  
    i2cWrite_HM1375(0x0347, 0x51);  
    i2cWrite_HM1375(0x0348, 0x75);  
    i2cWrite_HM1375(0x0349, 0x5C);  
    i2cWrite_HM1375(0x034A, 0x6A);  
    i2cWrite_HM1375(0x034B, 0x68);  
    i2cWrite_HM1375(0x034C, 0x5E);  
    i2cWrite_HM1375(0x0350, 0x7C);  
    i2cWrite_HM1375(0x0351, 0x78);  
    i2cWrite_HM1375(0x0352, 0x08);  
    i2cWrite_HM1375(0x0353, 0x04);  
    i2cWrite_HM1375(0x0354, 0x80);  
    i2cWrite_HM1375(0x0355, 0x9A);  
    i2cWrite_HM1375(0x0356, 0xCC);  
    i2cWrite_HM1375(0x0357, 0xFF);  
    i2cWrite_HM1375(0x0358, 0xFF);  
    i2cWrite_HM1375(0x035A, 0xFF);  
    i2cWrite_HM1375(0x035B, 0x00);  
    i2cWrite_HM1375(0x035C, 0x70);  
    i2cWrite_HM1375(0x035D, 0x80);  
    i2cWrite_HM1375(0x035F, 0xA0);  
    i2cWrite_HM1375(0x0488, 0x30);  
    i2cWrite_HM1375(0x0360, 0xDF);   
    i2cWrite_HM1375(0x0361, 0x00);  
    i2cWrite_HM1375(0x0362, 0xFF);  
    i2cWrite_HM1375(0x0363, 0x03);  
    i2cWrite_HM1375(0x0364, 0xFF);  
    i2cWrite_HM1375(0x037B, 0x11);  //whole nonEDR to EDR ratio 
    i2cWrite_HM1375(0x037C, 0x1E);  //EDR LONG litbin pop out to nonEDR 

    //----------------------------------------------
    //AUTOMATIC EXPOSURE CONFIGURATION
    //----------------------------------------------

    i2cWrite_HM1375(0x0380, 0xFF);       
    i2cWrite_HM1375(0x0383, 0x50);  // RESERVED       
    i2cWrite_HM1375(0x038A, 0x64);   
    i2cWrite_HM1375(0x038B, 0x64);   
    //i2cWrite_HM1375(0x038E, 0x3C);   
    i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);   
    i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
    i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
    i2cWrite_HM1375(0x0391, 0x2A);  // RESERVED        
    i2cWrite_HM1375(0x0393, 0x1E);  // RESERVED       
    i2cWrite_HM1375(0x0394, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0395, 0x23);         
    i2cWrite_HM1375(0x0398, 0x03);  // RESERVED
    i2cWrite_HM1375(0x0399, 0x45);  // RESERVED
    i2cWrite_HM1375(0x039A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x039B, 0x8B);  // RESERVED
    i2cWrite_HM1375(0x039C, 0x0D);  // RESERVED
    i2cWrite_HM1375(0x039D, 0x16);  // RESERVED
    i2cWrite_HM1375(0x039E, 0x0A);  // RESERVED
    i2cWrite_HM1375(0x039F, 0x10);       
    i2cWrite_HM1375(0x03A0, 0x10);          
    i2cWrite_HM1375(0x03A1, 0xE5);  // RESERVED   
    i2cWrite_HM1375(0x03A2, 0x06);  // RESERVED   
    i2cWrite_HM1375(0x03A4, 0x18);        
    i2cWrite_HM1375(0x03A5, 0x48);    
    i2cWrite_HM1375(0x03A6, 0x2D);  // RESERVED
    i2cWrite_HM1375(0x03A7, 0x78);  // RESERVED
    i2cWrite_HM1375(0x03AC, 0x5A);  // RESERVED
    i2cWrite_HM1375(0x03AD, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x03AE, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x03AF, 0x04);  // RESERVED
    i2cWrite_HM1375(0x03B0, 0x35);  // RESERVED
    i2cWrite_HM1375(0x03B1, 0x14);  // RESERVED
    i2cWrite_HM1375(0x036F, 0x04);  // Bayer denoise strength EDR
    i2cWrite_HM1375(0x0370, 0x0A);  // Bayer denoise strength nonEDR
    i2cWrite_HM1375(0x0371, 0x04);  // Bayer denoise strength EDR A0
    i2cWrite_HM1375(0x0372, 0x00);  // Bayer denoise strength nonEDR A0
    i2cWrite_HM1375(0x0373, 0x40);  // raw sharpness strength EDR
    i2cWrite_HM1375(0x0374, 0x20);  // raw sharpness strength nonEDR
    i2cWrite_HM1375(0x0375, 0x04);  // raw sharpness strength EDR A0
    i2cWrite_HM1375(0x0376, 0x00);  // raw sharpness strength nonEDR A0
    i2cWrite_HM1375(0x0377, 0x08);  // Y Denoise strength EDR
    i2cWrite_HM1375(0x0378, 0x08);  // Y Denoise strength nonEDR
    i2cWrite_HM1375(0x0379, 0x04);  // Y Denoise strength EDR A0
    i2cWrite_HM1375(0x037A, 0x08);  // Y Denoise strength nonEDR A0

    //----------------------------------------------
    //DIGITAL BLACK LEVEL OFFSET CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0420, 0x00);    
    i2cWrite_HM1375(0x0421, 0x00);    
    i2cWrite_HM1375(0x0422, 0x00);    
    i2cWrite_HM1375(0x0423, 0x84);    

    //----------------------------------------------
    //AUTO BLACK LEVEL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0430, 0x10);   
    i2cWrite_HM1375(0x0431, 0x60);   
    i2cWrite_HM1375(0x0432, 0x10);   
    i2cWrite_HM1375(0x0433, 0x20);   
    i2cWrite_HM1375(0x0434, 0x00);   
    i2cWrite_HM1375(0x0435, 0x30);   
    i2cWrite_HM1375(0x0436, 0x00);   

    //----------------------------------------------
    //LOWLIGHT_OUTDOOR IPP CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0450, 0xFD);   
    i2cWrite_HM1375(0x0451, 0xD8);   
    i2cWrite_HM1375(0x0452, 0xA0);   
    i2cWrite_HM1375(0x0453, 0x50);   
    i2cWrite_HM1375(0x0454, 0x00);       
    i2cWrite_HM1375(0x0459, 0x04);    
    i2cWrite_HM1375(0x045A, 0x00);   
    i2cWrite_HM1375(0x045B, 0x30);   
    i2cWrite_HM1375(0x045C, 0x01);   
    i2cWrite_HM1375(0x045D, 0x70);   
    i2cWrite_HM1375(0x0460, 0x00);    
    i2cWrite_HM1375(0x0461, 0x00);    
    i2cWrite_HM1375(0x0462, 0x00);    
    i2cWrite_HM1375(0x0465, 0x16);    
    i2cWrite_HM1375(0x0466, 0x14);    
    i2cWrite_HM1375(0x0478, 0x00);   

    //----------------------------------------------
    //COLOR SPACE CONVERSION_SATURATION ADJ
    //----------------------------------------------

    i2cWrite_HM1375(0x0480, 0x60);  //day:0x60 night:0x50
    i2cWrite_HM1375(0x0481, 0x06);  
    i2cWrite_HM1375(0x0482, 0x0C);  

    //----------------------------------------------
    //CONTRAST_BRIGHTNESS
    //----------------------------------------------


    i2cWrite_HM1375(0x04B0, 0x4C);  // Contrast 05032011 by mh
    i2cWrite_HM1375(0x04B1, 0x86);  // contrast 06/17 by mh
    i2cWrite_HM1375(0x04B2, 0x00);  //
    i2cWrite_HM1375(0x04B3, 0x18);  //
    i2cWrite_HM1375(0x04B4, 0x00);  //
    i2cWrite_HM1375(0x04B5, 0x00);  //
    i2cWrite_HM1375(0x04B6, 0x30);  //
    i2cWrite_HM1375(0x04B7, 0x00);  //
    i2cWrite_HM1375(0x04B8, 0x00);  //
    i2cWrite_HM1375(0x04B9, 0x10);  //
    i2cWrite_HM1375(0x04BA, 0x00);  //
    i2cWrite_HM1375(0x04BB, 0x00);  //
    i2cWrite_HM1375(0x04BD, 0x00);  //

    //----------------------------------------------
    //EDR CONTRAST
    //----------------------------------------------

    i2cWrite_HM1375(0x04D0, 0x56);     
    i2cWrite_HM1375(0x04D6, 0x30);  
    i2cWrite_HM1375(0x04DD, 0x10);  
    i2cWrite_HM1375(0x04D9, 0x16);  
    i2cWrite_HM1375(0x04D3, 0x18);  

    //----------------------------------------------
    //AE FLICKER STEP SIZE
    //----------------------------------------------

//#if (HM1375_FRAME_RATE == 30)
#if 0
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0xD0);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0xFA);  
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0x68);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0x7D);  
#endif
    i2cWrite_HM1375(0x0580, 0x50);  // RESERVED
    i2cWrite_HM1375(0x0581, 0x30);  // RESERVED

    //----------------------------------------------
    //Y_COLOR NOISE REDUCTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0582, 0x2D);  
    i2cWrite_HM1375(0x0583, 0x16);  
    i2cWrite_HM1375(0x0584, 0x1E);  
    i2cWrite_HM1375(0x0585, 0x0F);  
    i2cWrite_HM1375(0x0586, 0x08);  
    i2cWrite_HM1375(0x0587, 0x10);  
    i2cWrite_HM1375(0x0590, 0x10);  
    i2cWrite_HM1375(0x0591, 0x10);  
    i2cWrite_HM1375(0x0592, 0x05);  
    i2cWrite_HM1375(0x0593, 0x05);  
    i2cWrite_HM1375(0x0594, 0x04);  
    i2cWrite_HM1375(0x0595, 0x06);  

    //----------------------------------------------
    //Y_Sharpness strength
    //----------------------------------------------

    i2cWrite_HM1375(0x05B0, 0x04);    
    i2cWrite_HM1375(0x05B1, 0x00);    


    //----------------------------------------------
    //WINDOW_SCALER
    //----------------------------------------------

    i2cWrite_HM1375(0x05E4, 0x08);  
    i2cWrite_HM1375(0x05E5, 0x00);  
    i2cWrite_HM1375(0x05E6, 0x07);  
    i2cWrite_HM1375(0x05E7, 0x05);  
    i2cWrite_HM1375(0x05E8, 0x0A);  
    i2cWrite_HM1375(0x05E9, 0x00);  
    i2cWrite_HM1375(0x05EA, 0xD9);  
    i2cWrite_HM1375(0x05EB, 0x02);  

    //----------------------------------------------
    //FLEXI ENGINE_AE ADJUST CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0666, 0x02);        
    i2cWrite_HM1375(0x0667, 0xE0);      
    i2cWrite_HM1375(0x067F, 0x19);      
    i2cWrite_HM1375(0x067C, 0x00);  
    i2cWrite_HM1375(0x067D, 0x00);  
    i2cWrite_HM1375(0x0682, 0x00);  
    i2cWrite_HM1375(0x0683, 0x00);      
    i2cWrite_HM1375(0x0688, 0x00);  
    i2cWrite_HM1375(0x0689, 0x00);  
    i2cWrite_HM1375(0x068E, 0x00);  
    i2cWrite_HM1375(0x068F, 0x00);  
    i2cWrite_HM1375(0x0695, 0x00);   
    i2cWrite_HM1375(0x0694, 0x00);      
    i2cWrite_HM1375(0x0697, 0x19);      
    i2cWrite_HM1375(0x069B, 0x00);  
    i2cWrite_HM1375(0x069C, 0x30);  // max EDR ratio   
    i2cWrite_HM1375(0x0720, 0x00);  
    i2cWrite_HM1375(0x0725, 0x6A);      
    i2cWrite_HM1375(0x0726, 0x03);  
    i2cWrite_HM1375(0x072B, 0x64);  
    i2cWrite_HM1375(0x072C, 0x64);  
    i2cWrite_HM1375(0x072D, 0x20);  
    i2cWrite_HM1375(0x072E, 0x82);  //turn off night mode
    i2cWrite_HM1375(0x072F, 0x08);      
    i2cWrite_HM1375(0x0800, 0x16);  
    i2cWrite_HM1375(0x0801, 0x30);    
    i2cWrite_HM1375(0x0802, 0x00);  
    i2cWrite_HM1375(0x0803, 0x68);  
    i2cWrite_HM1375(0x0804, 0x01);  
    i2cWrite_HM1375(0x0805, 0x28);  
    i2cWrite_HM1375(0x0806, 0x10);   
    i2cWrite_HM1375(0x0808, 0x1D);  
    i2cWrite_HM1375(0x0809, 0x18);  
    i2cWrite_HM1375(0x080A, 0x10);       
    i2cWrite_HM1375(0x080B, 0x07);       
    i2cWrite_HM1375(0x080D, 0x0F);  
    i2cWrite_HM1375(0x080E, 0x0F);    
    i2cWrite_HM1375(0x0810, 0x00);  
    i2cWrite_HM1375(0x0811, 0x08);       
    i2cWrite_HM1375(0x0812, 0x20);  
    i2cWrite_HM1375(0x0857, 0x0A);  
    i2cWrite_HM1375(0x0858, 0x30);  
    i2cWrite_HM1375(0x0859, 0x01);  
#if 0   // 15fps
    i2cWrite_HM1375(0x085A, 0x06);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x80);  
#else   // 30fps
    i2cWrite_HM1375(0x085A, 0x03);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x40);  
#endif
    i2cWrite_HM1375(0x085C, 0x03);  
    i2cWrite_HM1375(0x085D, 0x7F);  
    i2cWrite_HM1375(0x085E, 0x02);  //(Long)Max INTG  
    i2cWrite_HM1375(0x085F, 0xD0);  
    i2cWrite_HM1375(0x0860, 0x03);      
    i2cWrite_HM1375(0x0861, 0x7F);  
    i2cWrite_HM1375(0x0862, 0x02);  //(short)Max INTG     
    i2cWrite_HM1375(0x0863, 0xD0);  
    i2cWrite_HM1375(0x0864, 0x00);  //(short)Max AG   
    i2cWrite_HM1375(0x0865, 0x7F);  
    i2cWrite_HM1375(0x0866, 0x01);  
    i2cWrite_HM1375(0x0867, 0x00);  
    i2cWrite_HM1375(0x0868, 0x40);  
    i2cWrite_HM1375(0x0869, 0x01);  
    i2cWrite_HM1375(0x086A, 0x00);  
    i2cWrite_HM1375(0x086B, 0x40);  
    i2cWrite_HM1375(0x086C, 0x01);  
    i2cWrite_HM1375(0x086D, 0x00);  
    i2cWrite_HM1375(0x086E, 0x40);  
    i2cWrite_HM1375(0x0870, 0x00);  
    i2cWrite_HM1375(0x0871, 0x14);  
    i2cWrite_HM1375(0x0872, 0x01);  
    i2cWrite_HM1375(0x0873, 0x20);  
    i2cWrite_HM1375(0x0874, 0x00);  
    i2cWrite_HM1375(0x0875, 0x14);  
    i2cWrite_HM1375(0x0876, 0x00);  
    i2cWrite_HM1375(0x0877, 0xEC);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MAXIMUM EDR 
    //----------------------------------------------

    i2cWrite_HM1375(0x0815, 0x00);  
    i2cWrite_HM1375(0x0816, 0x4C);  
    i2cWrite_HM1375(0x0817, 0x00);  
    i2cWrite_HM1375(0x0818, 0x7B);  
    i2cWrite_HM1375(0x0819, 0x00);  
    i2cWrite_HM1375(0x081A, 0xCA);  
    i2cWrite_HM1375(0x081B, 0x01);  
    i2cWrite_HM1375(0x081C, 0x3E);  
    i2cWrite_HM1375(0x081D, 0x01);  
    i2cWrite_HM1375(0x081E, 0x77);  
    i2cWrite_HM1375(0x081F, 0x01);  
    i2cWrite_HM1375(0x0820, 0xAA);  
    i2cWrite_HM1375(0x0821, 0x01);  
    i2cWrite_HM1375(0x0822, 0xCE);  
    i2cWrite_HM1375(0x0823, 0x01);  
    i2cWrite_HM1375(0x0824, 0xEE);  
    i2cWrite_HM1375(0x0825, 0x02);  
    i2cWrite_HM1375(0x0826, 0x16);  
    i2cWrite_HM1375(0x0827, 0x02);  
    i2cWrite_HM1375(0x0828, 0x33);  
    i2cWrite_HM1375(0x0829, 0x02);  
    i2cWrite_HM1375(0x082A, 0x65);  
    i2cWrite_HM1375(0x082B, 0x02);  
    i2cWrite_HM1375(0x082C, 0x91);  
    i2cWrite_HM1375(0x082D, 0x02);  
    i2cWrite_HM1375(0x082E, 0xDC);  
    i2cWrite_HM1375(0x082F, 0x03);  
    i2cWrite_HM1375(0x0830, 0x28);  
    i2cWrite_HM1375(0x0831, 0x03);  
    i2cWrite_HM1375(0x0832, 0x74);  
    i2cWrite_HM1375(0x0833, 0x03);  
    i2cWrite_HM1375(0x0834, 0xFF);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MINIMUM EDR
    //----------------------------------------------

    i2cWrite_HM1375(0x0882, 0x00);  
    i2cWrite_HM1375(0x0883, 0x3E);  
    i2cWrite_HM1375(0x0884, 0x00);  
    i2cWrite_HM1375(0x0885, 0x70);  
    i2cWrite_HM1375(0x0886, 0x00);  
    i2cWrite_HM1375(0x0887, 0xB8);  
    i2cWrite_HM1375(0x0888, 0x01);  
    i2cWrite_HM1375(0x0889, 0x28);  
    i2cWrite_HM1375(0x088A, 0x01);  
    i2cWrite_HM1375(0x088B, 0x5B);  
    i2cWrite_HM1375(0x088C, 0x01);  
    i2cWrite_HM1375(0x088D, 0x8A);  
    i2cWrite_HM1375(0x088E, 0x01);  
    i2cWrite_HM1375(0x088F, 0xB1);  
    i2cWrite_HM1375(0x0890, 0x01);  
    i2cWrite_HM1375(0x0891, 0xD9);  
    i2cWrite_HM1375(0x0892, 0x01);  
    i2cWrite_HM1375(0x0893, 0xEE);  
    i2cWrite_HM1375(0x0894, 0x02);  
    i2cWrite_HM1375(0x0895, 0x0F);  
    i2cWrite_HM1375(0x0896, 0x02);  
    i2cWrite_HM1375(0x0897, 0x4C);  
    i2cWrite_HM1375(0x0898, 0x02);  
    i2cWrite_HM1375(0x0899, 0x74);  
    i2cWrite_HM1375(0x089A, 0x02);  
    i2cWrite_HM1375(0x089B, 0xC3);  
    i2cWrite_HM1375(0x089C, 0x03);  
    i2cWrite_HM1375(0x089D, 0x0F);  
    i2cWrite_HM1375(0x089E, 0x03);  
    i2cWrite_HM1375(0x089F, 0x57);  
    i2cWrite_HM1375(0x08A0, 0x03);  
    i2cWrite_HM1375(0x08A1, 0xFF);  

    //----------------------------------------------
    //COMMAND UPDATE_TRIGGER
    //----------------------------------------------
    //OSTimeDly(4);
    i2cWrite_HM1375(0x0100, 0x01);  // CMU AE
    i2cWrite_HM1375(0x0101, 0x01);  // CMU AWB
    i2cWrite_HM1375(0x0000, 0x01);  // CMU
    i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger

}


void SetHM1375_VGA_30FPS_TRANWO(void)
{
    int i;
	u8	data;
    int count;

    
    DEBUG_SIU("SetHM1375_VGA_30FPS()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);

        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    i2cWrite_HM1375(0x0022, 0x01);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);
    i2cWrite_HM1375(0x0022, 0x00);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);

    i2cWrite_HM1375(0x0004, 0x18);  // hw reset mode
    
    i2cWrite_HM1375(0x000C, 0x04);  // MODE      _Reserved bit
    i2cWrite_HM1375(0x0006, 0x0C);
    i2cWrite_HM1375(0x000A, 0x20);  // CHIPCFG _Full frame, chip default 00
    i2cWrite_HM1375(0x000F, 0x10);  // IMGCFG
    i2cWrite_HM1375(0x0012, 0x01);  // RESERVED 
    i2cWrite_HM1375(0x0013, 0x02);  // RESERVED
    i2cWrite_HM1375(0x0015, 0x01);  // INTG_H    _Set up integration
    i2cWrite_HM1375(0x0016, 0x00);  // INTG_L    _Set up integration
    i2cWrite_HM1375(0x0018, 0x00);  // AGAIN     _Set up coarse gain
    i2cWrite_HM1375(0x001D, 0x40);  // DGGAIN    _Set up fine gain
    i2cWrite_HM1375(0x0020, 0x10);  // OPRTCFG   _AE Gain enabled, Pclk transition on falling edge
    i2cWrite_HM1375(0x0023, 0x43);  // IOCNTR    _IO pad drive strength
    i2cWrite_HM1375(0x0024, 0x20);  // CKCNTR    _Reserved bit
//#if (HM1375_FRAME_RATE == 30)
#if 1
    i2cWrite_HM1375(0x0025, 0x00);  // CKCFG     _Select PLL clock -> 24MHz to 96 MHz
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0025, 0x01);  // CKCFG     _Select PLL clock -> 24MHz to 48 MHz
#endif
    i2cWrite_HM1375(0x0026, 0x6F);
    //i2cWrite_HM1375(0x0027, 0x30);  // OPORTCNTR _YUV output, ITU601 Disable, YCbYCr...
    i2cWrite_HM1375(0x0027, 0x10);  // OPORTCNTR _YUV output, ITU601 Disable, CbYCrY...
    i2cWrite_HM1375(0x0028, 0x01);  // CKMUL     _Reserved bit
    i2cWrite_HM1375(0x002E, 0x0E);  // ExtraRawSel	_sharpness
    i2cWrite_HM1375(0x0030, 0x00);  // EDRCFG    _Disable EDR mode
    i2cWrite_HM1375(0x0034, 0x0E);  // EDRBLEND  _Set EDR blend to 0.875
    i2cWrite_HM1375(0x0035, 0x01);  // RESERVED
    i2cWrite_HM1375(0x0036, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0038, 0x02);  // EDR2AGAIN _Set up EDR2 coarse gain
    i2cWrite_HM1375(0x0039, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003A, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003B, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003C, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003D, 0x40);  // EDR2DGAIN _Set up EDR2 fine gain
    i2cWrite_HM1375(0x003F, 0x14);  // RESERVED      less gradient effect

    //----------------------------------------------
    //BLACK LEVEL CONTROL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0040, 0x10);  // BLCTGT    _Black level target
    i2cWrite_HM1375(0x0044, 0x07);  // BLCCFG    _BLC configuration enable, reserved bit

    //----------------------------------------------
    //RESERVED REGISTERS FOR SENSOR READOUT_TIMING
    //----------------------------------------------

    i2cWrite_HM1375(0x0045, 0x25);  // RESERVED       
    i2cWrite_HM1375(0x0048, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x004E, 0xFF);  // RESERVED 
    i2cWrite_HM1375(0x0070, 0x00);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0071, 0x4F);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0072, 0x00);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0073, 0x30);  // RESERVED    - 0920 updated by Willie
    i2cWrite_HM1375(0x0074, 0x13);  // RESERVED
    i2cWrite_HM1375(0x0075, 0x40);  // RESERVED
    i2cWrite_HM1375(0x0076, 0x24);  // RESERVED
    i2cWrite_HM1375(0x0078, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x007A, 0x05);  // RESERVED
    i2cWrite_HM1375(0x007B, 0xF2);  // RESERVED
    i2cWrite_HM1375(0x007C, 0x10);  // RESERVED
    i2cWrite_HM1375(0x0080, 0xC9);  // RESERVED
    i2cWrite_HM1375(0x0081, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0082, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0083, 0xB0);  // RESERVED
    i2cWrite_HM1375(0x0084, 0x70);  // RESERVED
    i2cWrite_HM1375(0x0086, 0x3E);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0087, 0x70);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0088, 0x11);  // RESERVED
    i2cWrite_HM1375(0x0089, 0x3C);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x008A, 0x87);  // RESERVED   // Rev.E updated by Willie
    i2cWrite_HM1375(0x008D, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0090, 0x07);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0091, 0x09);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0092, 0x0C);  
    i2cWrite_HM1375(0x0093, 0x0C);  
    i2cWrite_HM1375(0x0094, 0x0C);  
    i2cWrite_HM1375(0x0095, 0x0C);  
    i2cWrite_HM1375(0x0096, 0x01);  // AGAIN gain (nonEDR) table update for CFPN improvement 0824
    i2cWrite_HM1375(0x0097, 0x00);  
    i2cWrite_HM1375(0x0098, 0x04);  
    i2cWrite_HM1375(0x0099, 0x08);  
    i2cWrite_HM1375(0x009A, 0x0C);  


    //----------------------------------------------
    //IMAGE PIPELINE PROCESSING CONTROL
    //----------------------------------------------

    if(AE_Flicker_50_60_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
    i2cWrite_HM1375(0x0121, 0x81);  // IPPCNTR2
    i2cWrite_HM1375(0x0122, 0xEB);  // IPPCNTR3
    i2cWrite_HM1375(0x0123, 0x29);  // IPPCNTR4
    i2cWrite_HM1375(0x0124, 0x50);  // CCMCNTR
    i2cWrite_HM1375(0x0125, 0xDE);  // IPPCNTR5
    i2cWrite_HM1375(0x0126, 0xB1);  // IPPCNTR6

    //----------------------------------------------
    //FLARE CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x013D, 0x0F);  
    i2cWrite_HM1375(0x013E, 0x0F);  
    i2cWrite_HM1375(0x013F, 0x0F);  

    //----------------------------------------------
    //BAD PIXEL CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0140, 0x14); 
    i2cWrite_HM1375(0x0141, 0x0A);
    i2cWrite_HM1375(0x0142, 0x14);
    i2cWrite_HM1375(0x0143, 0x0A);

    //----------------------------------------------
    //RESERVED
    //----------------------------------------------

    i2cWrite_HM1375(0x0144, 0x08);  // RESERVED
    i2cWrite_HM1375(0x0145, 0x04);  // RESERVED
    i2cWrite_HM1375(0x0146, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0147, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x0148, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0149, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x014A, 0x96);  // RESERVED
    i2cWrite_HM1375(0x014B, 0xC8);  // RESERVED

    //----------------------------------------------
    //SHARPENING CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0150, 0x14);  // SHPTHLR   
    i2cWrite_HM1375(0x0151, 0x30);  // SHPTHLR_A 
    i2cWrite_HM1375(0x0152, 0x54);  // SHPTHHR   
    i2cWrite_HM1375(0x0153, 0x70);  // SHPTHHR_A 
    i2cWrite_HM1375(0x0154, 0x14);  // SHPTHLG   
    i2cWrite_HM1375(0x0155, 0x30);  // SHPTHLG_A 
    i2cWrite_HM1375(0x0156, 0x54);  // SHPTHHG   
    i2cWrite_HM1375(0x0157, 0x70);  // SHPTHHG_A 
    i2cWrite_HM1375(0x0158, 0x14);  // SHPTHLB   
    i2cWrite_HM1375(0x0159, 0x30);  // SHPTHLB_A 
    i2cWrite_HM1375(0x015A, 0x54);  // SHPTHHB   
    i2cWrite_HM1375(0x015B, 0x70);  // SHPTHHB_A 
    i2cWrite_HM1375(0x015C, 0x30);  // SHPSTR    _Sharpness strength
    i2cWrite_HM1375(0x015D, 0x00);  // SHPSTR_A  _sharpness strength_Alpha0


    //----------------------------------------------
    // NOISE FILTER CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x01D8, 0x20);  // NFHTHG
    i2cWrite_HM1375(0x01D9, 0x08);  // NFLTHG
    i2cWrite_HM1375(0x01DA, 0x20);  // NFHTHB
    i2cWrite_HM1375(0x01DB, 0x08);  // NFLTHB
    i2cWrite_HM1375(0x01DC, 0x20);  // NFHTHR
    i2cWrite_HM1375(0x01DD, 0x08);  // NFLTHR
    i2cWrite_HM1375(0x01DE, 0x20);  // NFHTHG_A
    i2cWrite_HM1375(0x01DF, 0x08);  // NFLTHG_A
    i2cWrite_HM1375(0x01E0, 0x20);  // NFHTHB_A
    i2cWrite_HM1375(0x01E1, 0x08);  // NFLTHB_A
    i2cWrite_HM1375(0x01E2, 0xFF);  // NFHTHR_A
    i2cWrite_HM1375(0x01E3, 0x00);  // NFLTHR_A
    i2cWrite_HM1375(0x01E4, 0x10);  // NFSTR
    i2cWrite_HM1375(0x01E5, 0x10);  // NFSTR_A 
    i2cWrite_HM1375(0x01E6, 0x02);  // NFSTR_OUTA
    i2cWrite_HM1375(0x01E7, 0x10);  // NFMTHG   
    i2cWrite_HM1375(0x01E8, 0x10);  // NFMTHB  
    i2cWrite_HM1375(0x01E9, 0x10);  // NFMTHR
    i2cWrite_HM1375(0x01EA, 0x10);  // NFMTHR_A
    i2cWrite_HM1375(0x01EC, 0xFA);  // NFMTHR_A
    i2cWrite_HM1375(0x01EB, 0x10);  // NFMTHR_A

    //----------------------------------------------
    //LENS SHADING CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0220, 0xF0);
    i2cWrite_HM1375(0x0221, 0xA0);
    i2cWrite_HM1375(0x0222, 0x00);
    i2cWrite_HM1375(0x0223, 0x80);
    i2cWrite_HM1375(0x0224, 0x80);
    i2cWrite_HM1375(0x0225, 0x00);
    i2cWrite_HM1375(0x0226, 0x80);
    i2cWrite_HM1375(0x0227, 0x80);
    i2cWrite_HM1375(0x0228, 0x00);
    i2cWrite_HM1375(0x0229, 0x80);
    i2cWrite_HM1375(0x022A, 0x80);
    i2cWrite_HM1375(0x022B, 0x00);
    i2cWrite_HM1375(0x022C, 0x80);
    i2cWrite_HM1375(0x022D, 0x12);
    i2cWrite_HM1375(0x022E, 0x10);
    i2cWrite_HM1375(0x022F, 0x12);
    i2cWrite_HM1375(0x0230, 0x10);
    i2cWrite_HM1375(0x0231, 0x12);
    i2cWrite_HM1375(0x0232, 0x10);
    i2cWrite_HM1375(0x0233, 0x12);
    i2cWrite_HM1375(0x0234, 0x10);
    i2cWrite_HM1375(0x0235, 0x80);
    i2cWrite_HM1375(0x0236, 0x02);
    i2cWrite_HM1375(0x0237, 0x80);
    i2cWrite_HM1375(0x0238, 0x02);
    i2cWrite_HM1375(0x0239, 0x80);
    i2cWrite_HM1375(0x023A, 0x02);
    i2cWrite_HM1375(0x023B, 0x80);
    i2cWrite_HM1375(0x023C, 0x02);
    i2cWrite_HM1375(0x023D, 0x00);
    i2cWrite_HM1375(0x023E, 0x02);
    i2cWrite_HM1375(0x023F, 0x00);
    i2cWrite_HM1375(0x0240, 0x02);
    i2cWrite_HM1375(0x0241, 0x00);
    i2cWrite_HM1375(0x0242, 0x02);
    i2cWrite_HM1375(0x0243, 0x00);
    i2cWrite_HM1375(0x0244, 0x02);
    i2cWrite_HM1375(0x0251, 0x10);

    //----------------------------------------------
    //GAMMA CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0280, 0x00);  // normal Gamma
    i2cWrite_HM1375(0x0281, 0x46);  
    i2cWrite_HM1375(0x0282, 0x00);  
    i2cWrite_HM1375(0x0283, 0x77);  
    i2cWrite_HM1375(0x0284, 0x00);  
    i2cWrite_HM1375(0x0285, 0xCC);  
    i2cWrite_HM1375(0x0286, 0x01);  
    i2cWrite_HM1375(0x0287, 0x62);  
    i2cWrite_HM1375(0x0288, 0x01);  
    i2cWrite_HM1375(0x0289, 0x9B);  
    i2cWrite_HM1375(0x028A, 0x01);  
    i2cWrite_HM1375(0x028B, 0xCF);  
    i2cWrite_HM1375(0x028C, 0x01);  
    i2cWrite_HM1375(0x028D, 0xF6);  
    i2cWrite_HM1375(0x028E, 0x02);  
    i2cWrite_HM1375(0x028F, 0x1B);  
    i2cWrite_HM1375(0x0290, 0x02);  
    i2cWrite_HM1375(0x0291, 0x3B);  
    i2cWrite_HM1375(0x0292, 0x02);  
    i2cWrite_HM1375(0x0293, 0x59);  
    i2cWrite_HM1375(0x0294, 0x02);  
    i2cWrite_HM1375(0x0295, 0x91);  
    i2cWrite_HM1375(0x0296, 0x02);  
    i2cWrite_HM1375(0x0297, 0xC3);  
    i2cWrite_HM1375(0x0298, 0x03);  
    i2cWrite_HM1375(0x0299, 0x1B);  
    i2cWrite_HM1375(0x029A, 0x03);  
    i2cWrite_HM1375(0x029B, 0x65);  
    i2cWrite_HM1375(0x029C, 0x03);  
    i2cWrite_HM1375(0x029D, 0xA5);  // updated by MH 06/17
    i2cWrite_HM1375(0x029E, 0x00);  // slope high byte    
    i2cWrite_HM1375(0x029F, 0x78);  // slope low byte
    i2cWrite_HM1375(0x02A0, 0x04);  // GAM_A

    //----------------------------------------------
    //COLOR CORRECTION MATRIX
    //----------------------------------------------

    i2cWrite_HM1375(0x02C0, 0x80);  //CCM00_L    _Adjust for D65 color temperature
    i2cWrite_HM1375(0x02C1, 0x01);  //CCM00_H
    i2cWrite_HM1375(0x02C2, 0x71);  
    i2cWrite_HM1375(0x02C3, 0x04);  
    i2cWrite_HM1375(0x02C4, 0x0F);  
    i2cWrite_HM1375(0x02C5, 0x04);  
    i2cWrite_HM1375(0x02C6, 0x3D);  
    i2cWrite_HM1375(0x02C7, 0x04);  
    i2cWrite_HM1375(0x02C8, 0x94);  
    i2cWrite_HM1375(0x02C9, 0x01);  
    i2cWrite_HM1375(0x02CA, 0x57);  
    i2cWrite_HM1375(0x02CB, 0x04);  
    i2cWrite_HM1375(0x02CC, 0x0F);  
    i2cWrite_HM1375(0x02CD, 0x04);  
    i2cWrite_HM1375(0x02CE, 0x8F);  
    i2cWrite_HM1375(0x02CF, 0x04);  
    i2cWrite_HM1375(0x02D0, 0x9E);  
    i2cWrite_HM1375(0x02D1, 0x01);  
    i2cWrite_HM1375(0x02E0, 0x06);  // CCM_A     _reduce to 0.267x
    i2cWrite_HM1375(0x02E1, 0xC0);  // RESERVED
    i2cWrite_HM1375(0x02E2, 0xE0);  // RESERVED
    i2cWrite_HM1375(0x02F0, 0x48);  // ACCM00_L  _Adjust for IncA color temperature
    i2cWrite_HM1375(0x02F1, 0x01);  // ACCM00_H
    i2cWrite_HM1375(0x02F2, 0x32);  
    i2cWrite_HM1375(0x02F3, 0x04);  
    i2cWrite_HM1375(0x02F4, 0x16);  
    i2cWrite_HM1375(0x02F5, 0x04);  
    i2cWrite_HM1375(0x02F6, 0x52);  
    i2cWrite_HM1375(0x02F7, 0x04);  
    i2cWrite_HM1375(0x02F8, 0xAA);  
    i2cWrite_HM1375(0x02F9, 0x01);  
    i2cWrite_HM1375(0x02FA, 0x58);  
    i2cWrite_HM1375(0x02FB, 0x04);  
    i2cWrite_HM1375(0x02FC, 0x56);  
    i2cWrite_HM1375(0x02FD, 0x04);  
    i2cWrite_HM1375(0x02FE, 0xDD);  
    i2cWrite_HM1375(0x02FF, 0x04);  
    i2cWrite_HM1375(0x0300, 0x33);  
    i2cWrite_HM1375(0x0301, 0x02);  

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE WINDOW CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0324, 0x00);
    i2cWrite_HM1375(0x0325, 0x01);

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE DETECTION AND LIMITS
    //----------------------------------------------

    i2cWrite_HM1375(0x0333, 0x00);   
    i2cWrite_HM1375(0x0334, 0x00);  
    i2cWrite_HM1375(0x0335, 0x86);  
    i2cWrite_HM1375(0x0340, 0x40);      
    i2cWrite_HM1375(0x0341, 0x44);  
    i2cWrite_HM1375(0x0342, 0x4A);  
    i2cWrite_HM1375(0x0343, 0x2B);  
    i2cWrite_HM1375(0x0344, 0x94);  
    i2cWrite_HM1375(0x0345, 0x3F);  
    i2cWrite_HM1375(0x0346, 0x8E);  
    i2cWrite_HM1375(0x0347, 0x51);  
    i2cWrite_HM1375(0x0348, 0x75);  
    i2cWrite_HM1375(0x0349, 0x5C);  
    i2cWrite_HM1375(0x034A, 0x6A);  
    i2cWrite_HM1375(0x034B, 0x68);  
    i2cWrite_HM1375(0x034C, 0x5E);  
    i2cWrite_HM1375(0x0350, 0x7C);  
    i2cWrite_HM1375(0x0351, 0x78);  
    i2cWrite_HM1375(0x0352, 0x08);  
    i2cWrite_HM1375(0x0353, 0x04);  
    i2cWrite_HM1375(0x0354, 0x80);  
    i2cWrite_HM1375(0x0355, 0x9A);  
    i2cWrite_HM1375(0x0356, 0xCC);  
    i2cWrite_HM1375(0x0357, 0xFF);  
    i2cWrite_HM1375(0x0358, 0xFF);  
    i2cWrite_HM1375(0x035A, 0xFF);  
    i2cWrite_HM1375(0x035B, 0x00);  
    i2cWrite_HM1375(0x035C, 0x70);  
    i2cWrite_HM1375(0x035D, 0x80);  
    i2cWrite_HM1375(0x035F, 0xA0);  
    i2cWrite_HM1375(0x0488, 0x30);  
    i2cWrite_HM1375(0x0360, 0xDF);   
    i2cWrite_HM1375(0x0361, 0x00);  
    i2cWrite_HM1375(0x0362, 0xFF);  
    i2cWrite_HM1375(0x0363, 0x03);  
    i2cWrite_HM1375(0x0364, 0xFF);  
    i2cWrite_HM1375(0x037B, 0x11);  //whole nonEDR to EDR ratio 
    i2cWrite_HM1375(0x037C, 0x1E);  //EDR LONG litbin pop out to nonEDR 

    //----------------------------------------------
    //AUTOMATIC EXPOSURE CONFIGURATION
    //----------------------------------------------

    i2cWrite_HM1375(0x0380, 0xFF);       
    i2cWrite_HM1375(0x0383, 0x50);  // RESERVED       
    i2cWrite_HM1375(0x038A, 0x64);   
    i2cWrite_HM1375(0x038B, 0x64);   
    i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);   
    i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
    i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
    i2cWrite_HM1375(0x0391, 0x2A);  // RESERVED        
    i2cWrite_HM1375(0x0393, 0x1E);  // RESERVED       
    i2cWrite_HM1375(0x0394, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0395, 0x23);         
    i2cWrite_HM1375(0x0398, 0x03);  // RESERVED
    i2cWrite_HM1375(0x0399, 0x45);  // RESERVED
    i2cWrite_HM1375(0x039A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x039B, 0x8B);  // RESERVED
    i2cWrite_HM1375(0x039C, 0x0D);  // RESERVED
    i2cWrite_HM1375(0x039D, 0x16);  // RESERVED
    i2cWrite_HM1375(0x039E, 0x0A);  // RESERVED
    i2cWrite_HM1375(0x039F, 0x10);       
    i2cWrite_HM1375(0x03A0, 0x10);          
    i2cWrite_HM1375(0x03A1, 0xE5);  // RESERVED   
    i2cWrite_HM1375(0x03A2, 0x06);  // RESERVED   
    i2cWrite_HM1375(0x03A4, 0x18);        
    i2cWrite_HM1375(0x03A5, 0x48);    
    i2cWrite_HM1375(0x03A6, 0x2D);  // RESERVED
    i2cWrite_HM1375(0x03A7, 0x78);  // RESERVED
    i2cWrite_HM1375(0x03AC, 0x5A);  // RESERVED
    i2cWrite_HM1375(0x03AD, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x03AE, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x03AF, 0x04);  // RESERVED
    i2cWrite_HM1375(0x03B0, 0x35);  // RESERVED
    i2cWrite_HM1375(0x03B1, 0x14);  // RESERVED
    i2cWrite_HM1375(0x036F, 0x04);  // Bayer denoise strength EDR
    i2cWrite_HM1375(0x0370, 0x0A);  // Bayer denoise strength nonEDR
    i2cWrite_HM1375(0x0371, 0x04);  // Bayer denoise strength EDR A0
    i2cWrite_HM1375(0x0372, 0x10);  // Bayer denoise strength nonEDR A0
    i2cWrite_HM1375(0x0373, 0x40);  // raw sharpness strength EDR
    i2cWrite_HM1375(0x0374, 0x20);  // raw sharpness strength nonEDR
    i2cWrite_HM1375(0x0375, 0x04);  // raw sharpness strength EDR A0
    i2cWrite_HM1375(0x0376, 0x00);  // raw sharpness strength nonEDR A0
    i2cWrite_HM1375(0x0377, 0x08);  // Y Denoise strength EDR
    i2cWrite_HM1375(0x0378, 0x08);  // Y Denoise strength nonEDR
    i2cWrite_HM1375(0x0379, 0x04);  // Y Denoise strength EDR A0
    i2cWrite_HM1375(0x037A, 0x08);  // Y Denoise strength nonEDR A0

    //----------------------------------------------
    //DIGITAL BLACK LEVEL OFFSET CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0420, 0x00);    
    i2cWrite_HM1375(0x0421, 0x00);    
    i2cWrite_HM1375(0x0422, 0x00);    
    i2cWrite_HM1375(0x0423, 0x84);    

    //----------------------------------------------
    //AUTO BLACK LEVEL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0430, 0x10);   
    i2cWrite_HM1375(0x0431, 0x60);   
    i2cWrite_HM1375(0x0432, 0x10);   
    i2cWrite_HM1375(0x0433, 0x20);   
    i2cWrite_HM1375(0x0434, 0x00);   
    i2cWrite_HM1375(0x0435, 0x30);   
    i2cWrite_HM1375(0x0436, 0x00);   

    //----------------------------------------------
    //LOWLIGHT_OUTDOOR IPP CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0450, 0xFD);   
    i2cWrite_HM1375(0x0451, 0xD8);   
    i2cWrite_HM1375(0x0452, 0xA0);   
    i2cWrite_HM1375(0x0453, 0x50);   
    i2cWrite_HM1375(0x0454, 0x00);       
    i2cWrite_HM1375(0x0459, 0x04);    
    i2cWrite_HM1375(0x045A, 0x00);   
    i2cWrite_HM1375(0x045B, 0x30);   
    i2cWrite_HM1375(0x045C, 0x01);   
    i2cWrite_HM1375(0x045D, 0x70);   
    i2cWrite_HM1375(0x0460, 0x00);    
    i2cWrite_HM1375(0x0461, 0x00);    
    i2cWrite_HM1375(0x0462, 0x00);    
    i2cWrite_HM1375(0x0465, 0x16);  // AWB IIR
    i2cWrite_HM1375(0x0466, 0x14);    
    i2cWrite_HM1375(0x0478, 0x00);   

    //----------------------------------------------
    //COLOR SPACE CONVERSION_SATURATION ADJ
    //----------------------------------------------

    i2cWrite_HM1375(0x0480, 0x60);  
    i2cWrite_HM1375(0x0481, 0x06);  
    i2cWrite_HM1375(0x0482, 0x0C);  

    //----------------------------------------------
    //CONTRAST_BRIGHTNESS
    //----------------------------------------------


    i2cWrite_HM1375(0x04B0, 0x4C);  // Contrast 05032011 by mh
    i2cWrite_HM1375(0x04B1, 0x86);  // contrast 06/17 by mh
    i2cWrite_HM1375(0x04B2, 0x00);  //
    i2cWrite_HM1375(0x04B3, 0x18);  //
    i2cWrite_HM1375(0x04B4, 0x00);  //
    i2cWrite_HM1375(0x04B5, 0x00);  //
    i2cWrite_HM1375(0x04B6, 0x30);  //
    i2cWrite_HM1375(0x04B7, 0x00);  //
    i2cWrite_HM1375(0x04B8, 0x00);  //
    i2cWrite_HM1375(0x04B9, 0x10);  //
    i2cWrite_HM1375(0x04BA, 0x00);  //
    i2cWrite_HM1375(0x04BB, 0x00);  //
    i2cWrite_HM1375(0x04BD, 0x00);  //

    //----------------------------------------------
    //EDR CONTRAST
    //----------------------------------------------

    i2cWrite_HM1375(0x04D0, 0x56);     
    i2cWrite_HM1375(0x04D6, 0x30);  
    i2cWrite_HM1375(0x04DD, 0x10);  
    i2cWrite_HM1375(0x04D9, 0x16);  
    i2cWrite_HM1375(0x04D3, 0x18);  

    //----------------------------------------------
    //AE FLICKER STEP SIZE
    //----------------------------------------------

//#if (HM1375_FRAME_RATE == 30)
#if 1
    i2cWrite_HM1375(0x0540, 0x01);   
    i2cWrite_HM1375(0x0541, 0x06);   
    i2cWrite_HM1375(0x0542, 0x01);   
    i2cWrite_HM1375(0x0543, 0x3B);  
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0x83);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0x9D);  
#endif
    i2cWrite_HM1375(0x0580, 0x50);  // RESERVED
    i2cWrite_HM1375(0x0581, 0x30);  // RESERVED

    //----------------------------------------------
    //Y_COLOR NOISE REDUCTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0582, 0x2D);  
    i2cWrite_HM1375(0x0583, 0x16);  
    i2cWrite_HM1375(0x0584, 0x1E);  
    i2cWrite_HM1375(0x0585, 0x0F);  
    i2cWrite_HM1375(0x0586, 0x08);  
    i2cWrite_HM1375(0x0587, 0x10);  
    i2cWrite_HM1375(0x0590, 0x10);  
    i2cWrite_HM1375(0x0591, 0x10);  
    i2cWrite_HM1375(0x0592, 0x05);  
    i2cWrite_HM1375(0x0593, 0x05);  
    i2cWrite_HM1375(0x0594, 0x04);  
    i2cWrite_HM1375(0x0595, 0x06);  

    //----------------------------------------------
    //Y_Sharpness strength
    //----------------------------------------------

    i2cWrite_HM1375(0x05B0, 0x04);    
    i2cWrite_HM1375(0x05B1, 0x00);    


    //----------------------------------------------
    //WINDOW_SCALER
    //----------------------------------------------

    i2cWrite_HM1375(0x05E4, 0x02);  
    i2cWrite_HM1375(0x05E5, 0x00);  
    i2cWrite_HM1375(0x05E6, 0x81);  
    i2cWrite_HM1375(0x05E7, 0x02);  
    i2cWrite_HM1375(0x05E8, 0x09);  
    i2cWrite_HM1375(0x05E9, 0x00);  
    i2cWrite_HM1375(0x05EA, 0xE8);  
    i2cWrite_HM1375(0x05EB, 0x01);  

    //----------------------------------------------
    //FLEXI ENGINE_AE ADJUST CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0666, 0x02);        
    i2cWrite_HM1375(0x0667, 0xE0);      
    i2cWrite_HM1375(0x067F, 0x19);      
    i2cWrite_HM1375(0x067C, 0x00);  
    i2cWrite_HM1375(0x067D, 0x00);  
    i2cWrite_HM1375(0x0682, 0x00);  
    i2cWrite_HM1375(0x0683, 0x00);      
    i2cWrite_HM1375(0x0688, 0x00);  
    i2cWrite_HM1375(0x0689, 0x00);  
    i2cWrite_HM1375(0x068E, 0x00);  
    i2cWrite_HM1375(0x068F, 0x00);  
    i2cWrite_HM1375(0x0695, 0x00);   
    i2cWrite_HM1375(0x0694, 0x00);      
    i2cWrite_HM1375(0x0697, 0x19);      
    i2cWrite_HM1375(0x069B, 0x00);  
    i2cWrite_HM1375(0x069C, 0x20);  // max EDR ratio   
    i2cWrite_HM1375(0x0720, 0x00);  
    i2cWrite_HM1375(0x0725, 0x6A);      
    i2cWrite_HM1375(0x0726, 0x03);  
    i2cWrite_HM1375(0x072B, 0x64);  
    i2cWrite_HM1375(0x072C, 0x64);  
    i2cWrite_HM1375(0x072D, 0x20);  
    i2cWrite_HM1375(0x072E, 0x82);  //turn off night mode
    i2cWrite_HM1375(0x072F, 0x08);      
    i2cWrite_HM1375(0x0800, 0x16);  
    i2cWrite_HM1375(0x0801, 0x4F);    
    i2cWrite_HM1375(0x0802, 0x00);  
    i2cWrite_HM1375(0x0803, 0x68);  
    i2cWrite_HM1375(0x0804, 0x01);  
    i2cWrite_HM1375(0x0805, 0x28);  
    i2cWrite_HM1375(0x0806, 0x10);   
    i2cWrite_HM1375(0x0808, 0x1D);  
    i2cWrite_HM1375(0x0809, 0x18);  
    i2cWrite_HM1375(0x080A, 0x10);       
    i2cWrite_HM1375(0x080B, 0x07);       
    i2cWrite_HM1375(0x080D, 0x0F);  
    i2cWrite_HM1375(0x080E, 0x0F);    
    i2cWrite_HM1375(0x0810, 0x00);  
    i2cWrite_HM1375(0x0811, 0x08);       
    i2cWrite_HM1375(0x0812, 0x20);  
    i2cWrite_HM1375(0x0857, 0x0A);  
    i2cWrite_HM1375(0x0858, 0x04);  
    i2cWrite_HM1375(0x0859, 0x01);  
#if 0   // 15fps
    i2cWrite_HM1375(0x085A, 0x08);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x30);  
#else   // 30fps
    i2cWrite_HM1375(0x085A, 0x04);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x18);  
#endif
    i2cWrite_HM1375(0x085C, 0x03);  
    i2cWrite_HM1375(0x085D, 0x7F);  
    i2cWrite_HM1375(0x085E, 0x02);  //(Long)Max INTG  
    i2cWrite_HM1375(0x085F, 0xD0);  
    i2cWrite_HM1375(0x0860, 0x03);      
    i2cWrite_HM1375(0x0861, 0x7F);  
    i2cWrite_HM1375(0x0862, 0x02);  //(short)Max INTG     
    i2cWrite_HM1375(0x0863, 0xD0);  
    i2cWrite_HM1375(0x0864, 0x02);  //(short)Max AG   
    i2cWrite_HM1375(0x0865, 0x7F);  
    i2cWrite_HM1375(0x0866, 0x01);  
    i2cWrite_HM1375(0x0867, 0x00);  
    i2cWrite_HM1375(0x0868, 0x40);  
    i2cWrite_HM1375(0x0869, 0x01);  
    i2cWrite_HM1375(0x086A, 0x00);  
    i2cWrite_HM1375(0x086B, 0x40);  
    i2cWrite_HM1375(0x086C, 0x01);  
    i2cWrite_HM1375(0x086D, 0x00);  
    i2cWrite_HM1375(0x086E, 0x40);  
    i2cWrite_HM1375(0x0870, 0x00);  
    i2cWrite_HM1375(0x0871, 0x14);  
    i2cWrite_HM1375(0x0872, 0x01);  
    i2cWrite_HM1375(0x0873, 0x20);  
    i2cWrite_HM1375(0x0874, 0x00);  
    i2cWrite_HM1375(0x0875, 0x14);  
    i2cWrite_HM1375(0x0876, 0x00);  
    i2cWrite_HM1375(0x0877, 0xEC);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MAXIMUM EDR 
    //----------------------------------------------

    i2cWrite_HM1375(0x0815, 0x00);  
    i2cWrite_HM1375(0x0816, 0x4C);  
    i2cWrite_HM1375(0x0817, 0x00);  
    i2cWrite_HM1375(0x0818, 0x7B);  
    i2cWrite_HM1375(0x0819, 0x00);  
    i2cWrite_HM1375(0x081A, 0xCA);  
    i2cWrite_HM1375(0x081B, 0x01);  
    i2cWrite_HM1375(0x081C, 0x3E);  
    i2cWrite_HM1375(0x081D, 0x01);  
    i2cWrite_HM1375(0x081E, 0x77);  
    i2cWrite_HM1375(0x081F, 0x01);  
    i2cWrite_HM1375(0x0820, 0xAA);  
    i2cWrite_HM1375(0x0821, 0x01);  
    i2cWrite_HM1375(0x0822, 0xCE);  
    i2cWrite_HM1375(0x0823, 0x01);  
    i2cWrite_HM1375(0x0824, 0xEE);  
    i2cWrite_HM1375(0x0825, 0x02);  
    i2cWrite_HM1375(0x0826, 0x16);  
    i2cWrite_HM1375(0x0827, 0x02);  
    i2cWrite_HM1375(0x0828, 0x33);  
    i2cWrite_HM1375(0x0829, 0x02);  
    i2cWrite_HM1375(0x082A, 0x65);  
    i2cWrite_HM1375(0x082B, 0x02);  
    i2cWrite_HM1375(0x082C, 0x91);  
    i2cWrite_HM1375(0x082D, 0x02);  
    i2cWrite_HM1375(0x082E, 0xDC);  
    i2cWrite_HM1375(0x082F, 0x03);  
    i2cWrite_HM1375(0x0830, 0x28);  
    i2cWrite_HM1375(0x0831, 0x03);  
    i2cWrite_HM1375(0x0832, 0x74);  
    i2cWrite_HM1375(0x0833, 0x03);  
    i2cWrite_HM1375(0x0834, 0xFF);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MINIMUM EDR
    //----------------------------------------------

    i2cWrite_HM1375(0x0882, 0x00);  
    i2cWrite_HM1375(0x0883, 0x3E);  
    i2cWrite_HM1375(0x0884, 0x00);  
    i2cWrite_HM1375(0x0885, 0x70);  
    i2cWrite_HM1375(0x0886, 0x00);  
    i2cWrite_HM1375(0x0887, 0xB8);  
    i2cWrite_HM1375(0x0888, 0x01);  
    i2cWrite_HM1375(0x0889, 0x28);  
    i2cWrite_HM1375(0x088A, 0x01);  
    i2cWrite_HM1375(0x088B, 0x5B);  
    i2cWrite_HM1375(0x088C, 0x01);  
    i2cWrite_HM1375(0x088D, 0x8A);  
    i2cWrite_HM1375(0x088E, 0x01);  
    i2cWrite_HM1375(0x088F, 0xB1);  
    i2cWrite_HM1375(0x0890, 0x01);  
    i2cWrite_HM1375(0x0891, 0xD9);  
    i2cWrite_HM1375(0x0892, 0x01);  
    i2cWrite_HM1375(0x0893, 0xEE);  
    i2cWrite_HM1375(0x0894, 0x02);  
    i2cWrite_HM1375(0x0895, 0x0F);  
    i2cWrite_HM1375(0x0896, 0x02);  
    i2cWrite_HM1375(0x0897, 0x4C);  
    i2cWrite_HM1375(0x0898, 0x02);  
    i2cWrite_HM1375(0x0899, 0x74);  
    i2cWrite_HM1375(0x089A, 0x02);  
    i2cWrite_HM1375(0x089B, 0xC3);  
    i2cWrite_HM1375(0x089C, 0x03);  
    i2cWrite_HM1375(0x089D, 0x0F);  
    i2cWrite_HM1375(0x089E, 0x03);  
    i2cWrite_HM1375(0x089F, 0x57);  
    i2cWrite_HM1375(0x08A0, 0x03);  
    i2cWrite_HM1375(0x08A1, 0xFF);  

    //----------------------------------------------
    //COMMAND UPDATE_TRIGGER
    //----------------------------------------------
    //OSTimeDly(4);
    i2cWrite_HM1375(0x0100, 0x01);  // CMU AE
    i2cWrite_HM1375(0x0101, 0x01);  // CMU AWB
    i2cWrite_HM1375(0x0000, 0x01);  // CMU
    i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger

}

#elif(HW_BOARD_OPTION == MR8120_TX_SUNIN_820HD)
void SetHM1375_720P_15FPS_SUNNIN(void)
#if 0
{
    int i;
	u8	data;
    int count;

    DEBUG_SIU("SetHM1375_720P_15FPS_SUNNIN()\n");

    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);

        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    i2cWrite_HM1375(0x0022, 0x01);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);
    i2cWrite_HM1375(0x0022, 0x00);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);

    i2cWrite_HM1375(0x0004, 0x18);  // hw reset mode
    
    i2cWrite_HM1375(0x000C ,0x04);	// MODE		_Reserved bit

    i2cWrite_HM1375(0x0006 ,0x08);
    i2cWrite_HM1375(0x000A ,0x00);  // CHIPCFG	_Full frame, chip default 00
    i2cWrite_HM1375(0x000F ,0x10);	// IMGCFG	_Select Variable frame rate paul-edit 18 ->10
    i2cWrite_HM1375(0x0012 ,0x01);	// RESERVED 
    i2cWrite_HM1375(0x0013 ,0x02);	// RESERVED

    i2cWrite_HM1375(0x0015 ,0x01);	// INTG_H	_Set up integration
    i2cWrite_HM1375(0x0016 ,0x00);	// INTG_L	_Set up integration
    i2cWrite_HM1375(0x0018 ,0x00);	// AGAIN		_Set up coarse gain
    i2cWrite_HM1375(0x001D ,0x40);  // DGGAIN	_Set up fine gain


    i2cWrite_HM1375(0x0020 ,0x10);	// OPRTCFG	_AE Gain enabled, Pclk transition on falling edge
    i2cWrite_HM1375(0x0023 ,0x43);	// IOCNTR	_IO pad drive strength
    i2cWrite_HM1375(0x0024 ,0x20);	// CKCNTR	_Reserved bit
    i2cWrite_HM1375(0x0025 ,0x00);	// CKCFG		_Select PLL clock -> 24MHz to 78 MHz
    i2cWrite_HM1375(0x0026 ,0x6C);
    i2cWrite_HM1375(0x0027 ,0x30);	// OPORTCNTR	_YUV output, ITU601 Disable
    i2cWrite_HM1375(0x0028 ,0x01);  // CKMUL		_Reserved bit

    i2cWrite_HM1375(0x0030 ,0x00);	// EDRCFG	_Disable EDR mode
    i2cWrite_HM1375(0x0034 ,0x0E);  // EDRBLEND	_Set EDR blend to 0.875
    i2cWrite_HM1375(0x0035 ,0x01);	// RESERVED
    i2cWrite_HM1375(0x0036 ,0x00);	// RESERVED
    i2cWrite_HM1375(0x0038 ,0x02);	// EDR2AGAIN	_Set up EDR2 coarse gain
    i2cWrite_HM1375(0x0039 ,0x01); 	// RESERVED
    i2cWrite_HM1375(0x003A ,0x01);	// RESERVED
    i2cWrite_HM1375(0x003B ,0xFF); 	// RESERVED
    i2cWrite_HM1375(0x003C ,0xFF); 	// RESERVED
    i2cWrite_HM1375(0x003D ,0x40);  // EDR2DGAIN	_Set up EDR2 fine gain
    i2cWrite_HM1375(0x003F ,0x14);  // RESERVED      less gradient effect

    //----------------------------------------------
    //BLACK LEVEL CONTROL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0040 ,0x10);	// BLCTGT	_Black level target
    i2cWrite_HM1375(0x0044 ,0x07);	// BLCCFG	_BLC configuration enable, reserved bit

    //----------------------------------------------
    //RESERVED REGISTERS FOR SENSOR READOUT_TIMING
    //----------------------------------------------

    i2cWrite_HM1375(0x0045 ,0x31);  // RESERVED       
    i2cWrite_HM1375(0x0048 ,0x7F); 	// RESERVED
    i2cWrite_HM1375(0x004E ,0xFF);	// RESERVED 
    i2cWrite_HM1375(0x0070 ,0x22);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0071 ,0x3F);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0072 ,0x22);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0073 ,0x30);  // RESERVED    - 0920 updated by Willie
    i2cWrite_HM1375(0x0074 ,0x13);  // RESERVED
    i2cWrite_HM1375(0x0075 ,0x40);  // RESERVED
    i2cWrite_HM1375(0x0076 ,0x24);  // RESERVED
    i2cWrite_HM1375(0x0078 ,0x0F);	// RESERVED
    i2cWrite_HM1375(0x007A ,0x06);  // RESERVED
    i2cWrite_HM1375(0x007B ,0x14);  // RESERVED
    i2cWrite_HM1375(0x007C ,0x10);  // RESERVED
    i2cWrite_HM1375(0x0080 ,0xC9);  // RESERVED
    i2cWrite_HM1375(0x0081 ,0x00);  // RESERVED
    i2cWrite_HM1375(0x0082 ,0x28); 	// RESERVED
    i2cWrite_HM1375(0x0083 ,0xB0);  // RESERVED
    i2cWrite_HM1375(0x0084 ,0x60);  // RESERVED
    i2cWrite_HM1375(0x0086 ,0x3E);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0087 ,0x70);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0088 ,0x11);  // RESERVED
    i2cWrite_HM1375(0x0089 ,0x3C);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x008A ,0x87);  // RESERVED   // Rev.E updated by Willie
    i2cWrite_HM1375(0x008D ,0x64);  // RESERVED

    i2cWrite_HM1375(0x0090 ,0x07);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0091 ,0x09);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0092 ,0x0C);
    i2cWrite_HM1375(0x0093 ,0x0C);
    i2cWrite_HM1375(0x0094 ,0x0C);
    i2cWrite_HM1375(0x0095 ,0x0C);
    i2cWrite_HM1375(0x0096 ,0x01);  // AGAIN gain (nonEDR) table update for CFPN improvement 0824
    i2cWrite_HM1375(0x0097 ,0x00);
    i2cWrite_HM1375(0x0098 ,0x04);
    i2cWrite_HM1375(0x0099 ,0x08);
    i2cWrite_HM1375(0x009A ,0x0C);


    //----------------------------------------------
    //IMAGE PIPELINE PROCESSING CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0120 ,0x37);  // IPPCNTR1	
    i2cWrite_HM1375(0x0121 ,0x81);  // IPPCNTR2
    i2cWrite_HM1375(0x0122 ,0xEB);  // IPPCNTR3
    i2cWrite_HM1375(0x0123 ,0x29);  // IPPCNTR4
    i2cWrite_HM1375(0x0124 ,0x50);  // CCMCNTR
    i2cWrite_HM1375(0x0125 ,0xDE);  // IPPCNTR5
    i2cWrite_HM1375(0x0126 ,0xB1);  // IPPCNTR6

    //----------------------------------------------
    //FLARE CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x013D ,0x0F);
    i2cWrite_HM1375(0x013E ,0x0F);
    i2cWrite_HM1375(0x013F ,0x0F);

    //----------------------------------------------
    //BAD PIXEL CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0140 ,0x14);		
    i2cWrite_HM1375(0x0141 ,0x0A);	
    i2cWrite_HM1375(0x0142 ,0x14);	
    i2cWrite_HM1375(0x0143 ,0x0A);	

    //----------------------------------------------
    //RESERVED
    //----------------------------------------------

    i2cWrite_HM1375(0x0144 ,0x08);	// RESERVED
    i2cWrite_HM1375(0x0145 ,0x04);	// RESERVED
    i2cWrite_HM1375(0x0146 ,0x28);	// RESERVED
    i2cWrite_HM1375(0x0147 ,0x3C);	// RESERVED
    i2cWrite_HM1375(0x0148 ,0x28);	// RESERVED
    i2cWrite_HM1375(0x0149 ,0x3C);	// RESERVED
    i2cWrite_HM1375(0x014A ,0x96);	// RESERVED
    i2cWrite_HM1375(0x014B ,0xC8);	// RESERVED

    //----------------------------------------------
    //SHARPENING CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0150 ,0x14);  // SHPTHLR	
    i2cWrite_HM1375(0x0151 ,0x30);	// SHPTHLR_A	
    i2cWrite_HM1375(0x0152 ,0x54);  // SHPTHHR	
    i2cWrite_HM1375(0x0153 ,0x70);	// SHPTHHR_A	
    i2cWrite_HM1375(0x0154 ,0x14);  // SHPTHLG	
    i2cWrite_HM1375(0x0155 ,0x30);	// SHPTHLG_A	
    i2cWrite_HM1375(0x0156 ,0x54);  // SHPTHHG	
    i2cWrite_HM1375(0x0157 ,0x70);	// SHPTHHG_A	
    i2cWrite_HM1375(0x0158 ,0x14);  // SHPTHLB	
    i2cWrite_HM1375(0x0159 ,0x30);	// SHPTHLB_A	
    i2cWrite_HM1375(0x015A ,0x54);  // SHPTHHB	
    i2cWrite_HM1375(0x015B ,0x70);	// SHPTHHB_A	
    i2cWrite_HM1375(0x015C ,0x30);  // SHPSTR	_Sharpness strength
    i2cWrite_HM1375(0x015D ,0x00);  // SHPSTR_A	_sharpness strength_Alpha0


    //----------------------------------------------
    // NOISE FILTER CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x01D8 ,0x20); 	// NFHTHG
    i2cWrite_HM1375(0x01D9 ,0x08); 	// NFLTHG
    i2cWrite_HM1375(0x01DA ,0x20);	// NFHTHB
    i2cWrite_HM1375(0x01DB ,0x08); 	// NFLTHB
    i2cWrite_HM1375(0x01DC ,0x20);	// NFHTHR
    i2cWrite_HM1375(0x01DD ,0x08); 	// NFLTHR
    i2cWrite_HM1375(0x01DE ,0x50); 	// NFHTHG_A
    i2cWrite_HM1375(0x01E0 ,0x50);	// NFHTHB_A
    i2cWrite_HM1375(0x01E2 ,0x50);	// NFHTHR_A
    i2cWrite_HM1375(0x01E4 ,0x10);	// NFSTR
    i2cWrite_HM1375(0x01E5 ,0x10);	// NFSTR_A 
    i2cWrite_HM1375(0x01E6 ,0x02);	// NFSTR_OUTA
    i2cWrite_HM1375(0x01E7 ,0x10);  // NFMTHG   
    i2cWrite_HM1375(0x01E8 ,0x10);  // NFMTHB  
    i2cWrite_HM1375(0x01E9 ,0x10);  // NFMTHR
    i2cWrite_HM1375(0x01EC ,0x28);	// NFMTHR_A

    //----------------------------------------------
    //LENS SHADING CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0220 ,0x00); 
    i2cWrite_HM1375(0x0221 ,0xA0);
    i2cWrite_HM1375(0x0222 ,0x00);
    i2cWrite_HM1375(0x0223 ,0x80);
    i2cWrite_HM1375(0x0224 ,0x80);
    i2cWrite_HM1375(0x0225 ,0x00);
    i2cWrite_HM1375(0x0226 ,0x80);
    i2cWrite_HM1375(0x0227 ,0x80);
    i2cWrite_HM1375(0x0228 ,0x00);
    i2cWrite_HM1375(0x0229 ,0x80);
    i2cWrite_HM1375(0x022A ,0x80);
    i2cWrite_HM1375(0x022B ,0x00);
    i2cWrite_HM1375(0x022C ,0x80);
    i2cWrite_HM1375(0x022D ,0x12);
    i2cWrite_HM1375(0x022E ,0x10);
    i2cWrite_HM1375(0x022F ,0x12);
    i2cWrite_HM1375(0x0230 ,0x10);
    i2cWrite_HM1375(0x0231 ,0x12);
    i2cWrite_HM1375(0x0232 ,0x10);
    i2cWrite_HM1375(0x0233 ,0x12);
    i2cWrite_HM1375(0x0234 ,0x10);
    i2cWrite_HM1375(0x0235 ,0x88);
    i2cWrite_HM1375(0x0236 ,0x02);
    i2cWrite_HM1375(0x0237 ,0x88);
    i2cWrite_HM1375(0x0238 ,0x02);
    i2cWrite_HM1375(0x0239 ,0x88);
    i2cWrite_HM1375(0x023A ,0x02);
    i2cWrite_HM1375(0x023B ,0x88);
    i2cWrite_HM1375(0x023C ,0x02);
    i2cWrite_HM1375(0x023D ,0x04);
    i2cWrite_HM1375(0x023E ,0x02);
    i2cWrite_HM1375(0x023F ,0x04);
    i2cWrite_HM1375(0x0240 ,0x02);
    i2cWrite_HM1375(0x0241 ,0x04);
    i2cWrite_HM1375(0x0242 ,0x02);
    i2cWrite_HM1375(0x0243 ,0x04);
    i2cWrite_HM1375(0x0244 ,0x02);
    i2cWrite_HM1375(0x0251 ,0x10);

    //----------------------------------------------
    //GAMMA CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0280 ,0x00);      // normal Gamma
    i2cWrite_HM1375(0x0281 ,0x3B);
    i2cWrite_HM1375(0x0282 ,0x00);
    i2cWrite_HM1375(0x0283 ,0x7C);
    i2cWrite_HM1375(0x0284 ,0x00);
    i2cWrite_HM1375(0x0285 ,0xD6);
    i2cWrite_HM1375(0x0286 ,0x01);
    i2cWrite_HM1375(0x0287 ,0x58);
    i2cWrite_HM1375(0x0288 ,0x01);
    i2cWrite_HM1375(0x0289 ,0x99);
    i2cWrite_HM1375(0x028A ,0x01);
    i2cWrite_HM1375(0x028B ,0xD1);
    i2cWrite_HM1375(0x028C ,0x01);
    i2cWrite_HM1375(0x028D ,0xFF);
    i2cWrite_HM1375(0x028E ,0x02);
    i2cWrite_HM1375(0x028F ,0x2D);
    i2cWrite_HM1375(0x0290 ,0x02);
    i2cWrite_HM1375(0x0291 ,0x59);
    i2cWrite_HM1375(0x0292 ,0x02);
    i2cWrite_HM1375(0x0293 ,0x81);
    i2cWrite_HM1375(0x0294 ,0x02);
    i2cWrite_HM1375(0x0295 ,0xBC);
    i2cWrite_HM1375(0x0296 ,0x02);
    i2cWrite_HM1375(0x0297 ,0xF0);
    i2cWrite_HM1375(0x0298 ,0x03);
    i2cWrite_HM1375(0x0299 ,0x50);
    i2cWrite_HM1375(0x029A ,0x03);
    i2cWrite_HM1375(0x029B ,0x91);
    i2cWrite_HM1375(0x029C ,0x03);
    i2cWrite_HM1375(0x029D ,0xC6);  // updated by MH 06/17
    i2cWrite_HM1375(0x029E ,0x00);  // slope high byte	  
    i2cWrite_HM1375(0x029F ,0x4C);  // slope low byte
     	
    i2cWrite_HM1375(0x02A0 ,0x06);	// GAM_A

    //----------------------------------------------
    //COLOR CORRECTION MATRIX
    //----------------------------------------------

    i2cWrite_HM1375(0x02C0 ,0x80);  //CCM00_L	_Adjust for D65 color temperature
    i2cWrite_HM1375(0x02C1 ,0x01);	//CCM00_H
    i2cWrite_HM1375(0x02C2 ,0x71);
    i2cWrite_HM1375(0x02C3 ,0x04);
    i2cWrite_HM1375(0x02C4 ,0x0F);
    i2cWrite_HM1375(0x02C5 ,0x04);
    i2cWrite_HM1375(0x02C6 ,0x3D);
    i2cWrite_HM1375(0x02C7 ,0x04);
    i2cWrite_HM1375(0x02C8 ,0x94);
    i2cWrite_HM1375(0x02C9 ,0x01);
    i2cWrite_HM1375(0x02CA ,0x57);
    i2cWrite_HM1375(0x02CB ,0x04);
    i2cWrite_HM1375(0x02CC ,0x0F);
    i2cWrite_HM1375(0x02CD ,0x04);
    i2cWrite_HM1375(0x02CE ,0x8F);
    i2cWrite_HM1375(0x02CF ,0x04);
    i2cWrite_HM1375(0x02D0 ,0x9E);
    i2cWrite_HM1375(0x02D1 ,0x01);
    i2cWrite_HM1375(0x02E0 ,0x06);	// CCM_A		_reduce to 0.267x
    i2cWrite_HM1375(0x02E1 ,0xC0);	// RESERVED
    i2cWrite_HM1375(0x02E2 ,0xE0);	// RESERVED
    i2cWrite_HM1375(0x02F0 ,0x48);	// ACCM00_L	_Adjust for IncA color temperature
    i2cWrite_HM1375(0x02F1 ,0x01);	// ACCM00_H
    i2cWrite_HM1375(0x02F2 ,0x32);
    i2cWrite_HM1375(0x02F3 ,0x04);
    i2cWrite_HM1375(0x02F4 ,0x16);
    i2cWrite_HM1375(0x02F5 ,0x04);
    i2cWrite_HM1375(0x02F6 ,0x52);
    i2cWrite_HM1375(0x02F7 ,0x04);
    i2cWrite_HM1375(0x02F8 ,0xAA);
    i2cWrite_HM1375(0x02F9 ,0x01);
    i2cWrite_HM1375(0x02FA ,0x58);
    i2cWrite_HM1375(0x02FB ,0x04);
    i2cWrite_HM1375(0x02FC ,0x56);
    i2cWrite_HM1375(0x02FD ,0x04);
    i2cWrite_HM1375(0x02FE ,0xDD);
    i2cWrite_HM1375(0x02FF ,0x04);
    i2cWrite_HM1375(0x0300 ,0x33);
    i2cWrite_HM1375(0x0301 ,0x02);

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE WINDOW CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0324 ,0x00);
    i2cWrite_HM1375(0x0325 ,0x01);   

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE DETECTION AND LIMITS
    //----------------------------------------------

    i2cWrite_HM1375(0x0333 ,0x86);	 
    i2cWrite_HM1375(0x0334 ,0x00);
    i2cWrite_HM1375(0x0335 ,0x86);
     
    i2cWrite_HM1375(0x0340 ,0x40);        
    i2cWrite_HM1375(0x0341 ,0x44);
    i2cWrite_HM1375(0x0342 ,0x4A);
    i2cWrite_HM1375(0x0343 ,0x2B);	
    i2cWrite_HM1375(0x0344 ,0x94);	
    i2cWrite_HM1375(0x0345 ,0x3F);	
    i2cWrite_HM1375(0x0346 ,0x8E);	
    i2cWrite_HM1375(0x0347 ,0x51);	
    i2cWrite_HM1375(0x0348 ,0x75);	
    i2cWrite_HM1375(0x0349 ,0x5C);	
    i2cWrite_HM1375(0x034A ,0x6A);
    i2cWrite_HM1375(0x034B ,0x68);	
    i2cWrite_HM1375(0x034C ,0x5E);
    i2cWrite_HM1375(0x0350 ,0x7C);
    i2cWrite_HM1375(0x0351 ,0x78);
    i2cWrite_HM1375(0x0352 ,0x08);
    i2cWrite_HM1375(0x0353 ,0x04);	
    i2cWrite_HM1375(0x0354 ,0x80);	
    i2cWrite_HM1375(0x0355 ,0x9A);
    i2cWrite_HM1375(0x0356 ,0xCC);
    i2cWrite_HM1375(0x0357 ,0xFF);	
    i2cWrite_HM1375(0x0358 ,0xFF);	
    i2cWrite_HM1375(0x035A ,0xFF);	
    i2cWrite_HM1375(0x035B ,0x00);	
    i2cWrite_HM1375(0x035C ,0x70);
    i2cWrite_HM1375(0x035D ,0x80);
    i2cWrite_HM1375(0x035F ,0xA0);
    i2cWrite_HM1375(0x0488 ,0x30);
    i2cWrite_HM1375(0x0360 ,0xDF);	 
    i2cWrite_HM1375(0x0361 ,0x00);
    i2cWrite_HM1375(0x0362 ,0xFF);
    i2cWrite_HM1375(0x0363 ,0x03);
    i2cWrite_HM1375(0x0364 ,0xFF);
    	

    i2cWrite_HM1375(0x037B ,0x11); //whole nonEDR to EDR ratio 
    i2cWrite_HM1375(0x037C ,0x1E); //EDR LONG litbin pop out to nonEDR 

    //----------------------------------------------
    //AUTOMATIC EXPOSURE CONFIGURATION
    //----------------------------------------------

    i2cWrite_HM1375(0x0380 ,0xFF);	 	 
    i2cWrite_HM1375(0x0383 ,0x50);  // RESERVED		 
    i2cWrite_HM1375(0x038A ,0x64);	 
    i2cWrite_HM1375(0x038B ,0x64);	 
    i2cWrite_HM1375(0x038E ,0x23);  // AE Target   20130702	 JKBACK
    i2cWrite_HM1375(0x0391 ,0x2A);  // RESERVED        
    i2cWrite_HM1375(0x0393 ,0x1E);  // RESERVED       
    i2cWrite_HM1375(0x0394 ,0x64);	// RESERVED
    i2cWrite_HM1375(0x0395 ,0x23);	       
    i2cWrite_HM1375(0x0398 ,0x03);	// RESERVED
    i2cWrite_HM1375(0x0399 ,0x45);	// RESERVED
    i2cWrite_HM1375(0x039A ,0x06);	// RESERVED
    i2cWrite_HM1375(0x039B ,0x8B);	// RESERVED
    i2cWrite_HM1375(0x039C ,0x0D);	// RESERVED
    i2cWrite_HM1375(0x039D ,0x16);	// RESERVED
    i2cWrite_HM1375(0x039E ,0x0A);	// RESERVED
    i2cWrite_HM1375(0x039F ,0x10);       	 
    i2cWrite_HM1375(0x03A0 ,0x10);       	 	
    i2cWrite_HM1375(0x03A1 ,0xE5);	// RESERVED   
    i2cWrite_HM1375(0x03A2 ,0x06);   	// RESERVED   
    i2cWrite_HM1375(0x03A4 ,0x18);   	      
    i2cWrite_HM1375(0x03A5 ,0x48);      
    i2cWrite_HM1375(0x03A6 ,0x2D);	// RESERVED
    i2cWrite_HM1375(0x03A7 ,0x78);	// RESERVED
    i2cWrite_HM1375(0x03AC ,0x5A);	// RESERVED
    i2cWrite_HM1375(0x03AD ,0x0F);	// RESERVED
    i2cWrite_HM1375(0x03AE ,0x7F);	// RESERVED
    i2cWrite_HM1375(0x03AF ,0x04);	// RESERVED
    i2cWrite_HM1375(0x03B0 ,0x35);	// RESERVED
    i2cWrite_HM1375(0x03B1 ,0x14);	// RESERVED


    i2cWrite_HM1375(0x036F ,0x04);  // Bayer denoise strength EDR
    i2cWrite_HM1375(0x0370 ,0x0A);  // Bayer denoise strength nonEDR       
    i2cWrite_HM1375(0x0371 ,0x04);  // Bayer denoise strength EDR A0
    i2cWrite_HM1375(0x0372 ,0x00);  // Bayer denoise strength nonEDR A0    

    i2cWrite_HM1375(0x0373 ,0x40);  // raw sharpness strength EDR
    i2cWrite_HM1375(0x0374 ,0x20);  // raw sharpness strength nonEDR
    i2cWrite_HM1375(0x0375 ,0x04);  // raw sharpness strength EDR A0
    i2cWrite_HM1375(0x0376 ,0x00);  // raw sharpness strength nonEDR A0

    i2cWrite_HM1375(0x0377 ,0x08);  // Y Denoise strength EDR
    i2cWrite_HM1375(0x0378 ,0x07);  // Y Denoise strength nonEDR     JKBACK 20130612 (08) -> (07)   
    i2cWrite_HM1375(0x0379 ,0x04);  // Y Denoise strength EDR A0
    i2cWrite_HM1375(0x037A ,0x08);  // Y Denoise strength nonEDR A0  

    //----------------------------------------------
    //DIGITAL BLACK LEVEL OFFSET CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0420 ,0x00);	
    i2cWrite_HM1375(0x0421 ,0x00);	
    i2cWrite_HM1375(0x0422 ,0x00);	
    i2cWrite_HM1375(0x0423 ,0x00);  //JKBACK 20130612 (84) -> (00)	

    //----------------------------------------------
    //AUTO BLACK LEVEL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0430 ,0x10);	
    i2cWrite_HM1375(0x0431 ,0x60);	
    i2cWrite_HM1375(0x0432 ,0x10);	
    i2cWrite_HM1375(0x0433 ,0x20);	
    i2cWrite_HM1375(0x0434 ,0x00);	
    i2cWrite_HM1375(0x0435 ,0x30);	
    i2cWrite_HM1375(0x0436 ,0x00);	

    //----------------------------------------------
    //LOWLIGHT_OUTDOOR IPP CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0450 ,0xFD);	
    i2cWrite_HM1375(0x0451 ,0xD8);	
    i2cWrite_HM1375(0x0452 ,0xA0);	
    i2cWrite_HM1375(0x0453 ,0x50);	
    i2cWrite_HM1375(0x0459 ,0x04);	 
    i2cWrite_HM1375(0x045A ,0x00);	
    i2cWrite_HM1375(0x045B ,0x30);	
    i2cWrite_HM1375(0x045C ,0x01);	
    i2cWrite_HM1375(0x045D ,0x70);	
    i2cWrite_HM1375(0x0460 ,0x00);	 
    i2cWrite_HM1375(0x0461 ,0x00);	 
    i2cWrite_HM1375(0x0462 ,0x00);	 
    i2cWrite_HM1375(0x0465 ,0x16);   //paul-edit 86->16	 
    i2cWrite_HM1375(0x0466 ,0x14);   //paul-edit 84->14	 
    i2cWrite_HM1375(0x0478 ,0x00);	

    //----------------------------------------------
    //COLOR SPACE CONVERSION_SATURATION ADJ
    //----------------------------------------------
     
    i2cWrite_HM1375(0x0480 ,0x60);	 
    i2cWrite_HM1375(0x0481 ,0x06);	
    i2cWrite_HM1375(0x0482 ,0x0C);	 

    //----------------------------------------------
    //CONTRAST_BRIGHTNESS
    //----------------------------------------------

            
    i2cWrite_HM1375(0x04B0 ,0x4C);	// Contrast 05032011 by mh      --- JKBACK 20130612 (4C) -> (40) -> (4C)
    i2cWrite_HM1375(0x04B1 ,0x05);	// contrast 06/17 by mh
    i2cWrite_HM1375(0x04B2 ,0x00);	//
    i2cWrite_HM1375(0x04B3 ,0x18);	//
    i2cWrite_HM1375(0x04B4 ,0x00);	//
    i2cWrite_HM1375(0x04B5 ,0x00);	//
    i2cWrite_HM1375(0x04B6 ,0x30);	//
    i2cWrite_HM1375(0x04B7 ,0x00);	//
    i2cWrite_HM1375(0x04B8 ,0x00);	//
    i2cWrite_HM1375(0x04B9 ,0x10);	//
    i2cWrite_HM1375(0x04BA ,0x00);	//
    i2cWrite_HM1375(0x04BB ,0x00);	//
    i2cWrite_HM1375(0x04BD ,0x00);	//

    i2cWrite_HM1375(0x04C0 ,0x01);	//jkback add 20130523   // JKBACK 20130612 (02) -> (10) -> (01)
    	
    //----------------------------------------------
    //EDR CONTRAST
    //----------------------------------------------

    i2cWrite_HM1375(0x04D0 ,0x56);        
    i2cWrite_HM1375(0x04D6 ,0x30);
    i2cWrite_HM1375(0x04DD ,0x10);
    i2cWrite_HM1375(0x04D9 ,0x16);
    i2cWrite_HM1375(0x04D3 ,0x18);

    //----------------------------------------------
    //AE FLICKER STEP SIZE
    //----------------------------------------------

    i2cWrite_HM1375(0x0540 ,0x00);	 
    i2cWrite_HM1375(0x0541 ,0x68);  //paul-edit D0->68	 
    i2cWrite_HM1375(0x0542 ,0x00);	 
    i2cWrite_HM1375(0x0543 ,0x7D);  //paul-edit FA->7D
    i2cWrite_HM1375(0x0580 ,0x50);	// RESERVED
    i2cWrite_HM1375(0x0581 ,0x30);	// RESERVED

    //----------------------------------------------
    //Y_COLOR NOISE REDUCTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0582 ,0x2D);	
    i2cWrite_HM1375(0x0583 ,0x16);	
    i2cWrite_HM1375(0x0584 ,0x1E);	
    i2cWrite_HM1375(0x0585 ,0x0F);	
    i2cWrite_HM1375(0x0586 ,0x08);	
    i2cWrite_HM1375(0x0587 ,0x10);	 
    i2cWrite_HM1375(0x0590 ,0x10);	
    i2cWrite_HM1375(0x0591 ,0x10);	 
    i2cWrite_HM1375(0x0592 ,0x05);	
    i2cWrite_HM1375(0x0593 ,0x05);	
    i2cWrite_HM1375(0x0594 ,0x04);	
    i2cWrite_HM1375(0x0595 ,0x06);

    //----------------------------------------------
    //Y_Sharpness strength
    //----------------------------------------------

    i2cWrite_HM1375(0x05B0 ,0x08);  //JKBACK 20130612 (04) -> (06) -> (08)	
    i2cWrite_HM1375(0x05B1 ,0x00);	


    //----------------------------------------------
    //WINDOW_SCALER
    //----------------------------------------------

    i2cWrite_HM1375(0x05E4 ,0x08);	 
    i2cWrite_HM1375(0x05E5 ,0x00);
    i2cWrite_HM1375(0x05E6 ,0x07);	
    i2cWrite_HM1375(0x05E7 ,0x05);	
    i2cWrite_HM1375(0x05E8 ,0x0A);	
    i2cWrite_HM1375(0x05E9 ,0x00);	
    i2cWrite_HM1375(0x05EA ,0xD9);	
    i2cWrite_HM1375(0x05EB ,0x02);	

    //----------------------------------------------
    //FLEXI ENGINE_AE ADJUST CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0666 ,0x02);          
    i2cWrite_HM1375(0x0667 ,0xE0);        
    i2cWrite_HM1375(0x067F ,0x19);        
    i2cWrite_HM1375(0x067C ,0x00);
    i2cWrite_HM1375(0x067D ,0x00);
    i2cWrite_HM1375(0x0682 ,0x00);
    i2cWrite_HM1375(0x0683 ,0x00);        
    i2cWrite_HM1375(0x0688 ,0x00);
    i2cWrite_HM1375(0x0689 ,0x00);
    i2cWrite_HM1375(0x068E ,0x00);
    i2cWrite_HM1375(0x068F ,0x00);
    i2cWrite_HM1375(0x0695 ,0x00);     
    i2cWrite_HM1375(0x0694 ,0x00);        
    i2cWrite_HM1375(0x0697 ,0x19);        
    i2cWrite_HM1375(0x069B ,0x00);   
    i2cWrite_HM1375(0x069C ,0x30);  // max EDR ratio  //paul-edit 20->30 

    i2cWrite_HM1375(0x0720 ,0x00);
    i2cWrite_HM1375(0x0725 ,0x6A);        
    i2cWrite_HM1375(0x0726 ,0x0A);  // JKBACK 20130529 Ori(03)
    i2cWrite_HM1375(0x072B ,0x64);	
    i2cWrite_HM1375(0x072C ,0x64);	
    i2cWrite_HM1375(0x072D ,0x20);      
    i2cWrite_HM1375(0x072E ,0x82);  //turn off night mode
    i2cWrite_HM1375(0x072F ,0x08);        

    i2cWrite_HM1375(0x0800 ,0x16);  
    i2cWrite_HM1375(0x0801 ,0x30);  //paul-edit 4F->30  
    i2cWrite_HM1375(0x0802 ,0x00);
    i2cWrite_HM1375(0x0803 ,0xCC);  //(68) JKBACK 20130529
    i2cWrite_HM1375(0x0804 ,0x01);
    i2cWrite_HM1375(0x0805 ,0x28);
    i2cWrite_HM1375(0x0806 ,0x10);	 
    i2cWrite_HM1375(0x0808 ,0x1D);  
    i2cWrite_HM1375(0x0809 ,0x18);
    i2cWrite_HM1375(0x080A ,0x10);         
    i2cWrite_HM1375(0x080B ,0x07);         
    i2cWrite_HM1375(0x080D ,0x0F);
    i2cWrite_HM1375(0x080E ,0x0F);      
    i2cWrite_HM1375(0x0810 ,0x00);
    i2cWrite_HM1375(0x0811 ,0x08);         
    i2cWrite_HM1375(0x0812 ,0x20);
    i2cWrite_HM1375(0x0857 ,0x0A);  
    i2cWrite_HM1375(0x0858 ,0x30);   //paul-edit 04->30
    i2cWrite_HM1375(0x0859 ,0x01);  
    i2cWrite_HM1375(0x085A ,0x03);
    i2cWrite_HM1375(0x085B ,0x40);
    i2cWrite_HM1375(0x085C ,0x04);   //paul-edit 04->03   
    i2cWrite_HM1375(0x085D ,0x7F);
    i2cWrite_HM1375(0x085E ,0x02);   //paul-edit 03->02
    i2cWrite_HM1375(0x085F ,0xD0);   //paul-edit B0->D0
    i2cWrite_HM1375(0x0860 ,0x03);        
    i2cWrite_HM1375(0x0861 ,0x7F);
    i2cWrite_HM1375(0x0862 ,0x02);   //paul-edit 03->02	 
    i2cWrite_HM1375(0x0863 ,0xD0);   //paul-edit B0->D0
    i2cWrite_HM1375(0x0864 ,0x02);   //paul-edit 02->00    
    i2cWrite_HM1375(0x0865 ,0x7F);
    i2cWrite_HM1375(0x0866 ,0x01);   
    i2cWrite_HM1375(0x0867 ,0x01);
    i2cWrite_HM1375(0x0868 ,0x40);
    i2cWrite_HM1375(0x0869 ,0x01);
    i2cWrite_HM1375(0x086A ,0x00);
    i2cWrite_HM1375(0x086B ,0x40);
    i2cWrite_HM1375(0x086C ,0x01);
    i2cWrite_HM1375(0x086D ,0x00);
    i2cWrite_HM1375(0x086E ,0x40);
    i2cWrite_HM1375(0x0870 ,0x00);    //JKBACK Edit 20130529
    i2cWrite_HM1375(0x0871 ,0x28);    //0x0014 (80/20)     ->  0x0040 (256/64)          HIMAX : 0x0069 (420/105)   0x0028(160/40)
    i2cWrite_HM1375(0x0872 ,0x01);    //
    i2cWrite_HM1375(0x0873 ,0x18);    //0x0120 (1152/288)  ->  0x0100 (1024/256)        HIMAX : 0x00D7 (860/215)   0x0118(1120/280)
    i2cWrite_HM1375(0x0874 ,0x00);    //
    i2cWrite_HM1375(0x0875 ,0x3C);    //0x0014 (80/20)     ->  0x0024 (144/36)          HIMAX : 0x003C (240/60)          -->        0x003C(240/60)
    i2cWrite_HM1375(0x0876 ,0x00);    //
    i2cWrite_HM1375(0x0877 ,0x90);    //0x00EC (944/236)   ->  0x0090 (576/144)         HIMAX : 0x0078 (480/120)         
     
    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MAXIMUM EDR 
    //----------------------------------------------

    i2cWrite_HM1375(0x0815 ,0x00);
    i2cWrite_HM1375(0x0816 ,0x4C);
    i2cWrite_HM1375(0x0817 ,0x00);
    i2cWrite_HM1375(0x0818 ,0x7B);
    i2cWrite_HM1375(0x0819 ,0x00);
    i2cWrite_HM1375(0x081A ,0xCA);
    i2cWrite_HM1375(0x081B ,0x01);
    i2cWrite_HM1375(0x081C ,0x3E);
    i2cWrite_HM1375(0x081D ,0x01);
    i2cWrite_HM1375(0x081E ,0x77);
    i2cWrite_HM1375(0x081F ,0x01);
    i2cWrite_HM1375(0x0820 ,0xAA);
    i2cWrite_HM1375(0x0821 ,0x01);
    i2cWrite_HM1375(0x0822 ,0xCE);
    i2cWrite_HM1375(0x0823 ,0x01);
    i2cWrite_HM1375(0x0824 ,0xEE);
    i2cWrite_HM1375(0x0825 ,0x02);
    i2cWrite_HM1375(0x0826 ,0x16);
    i2cWrite_HM1375(0x0827 ,0x02);
    i2cWrite_HM1375(0x0828 ,0x33);
    i2cWrite_HM1375(0x0829 ,0x02);
    i2cWrite_HM1375(0x082A ,0x65);
    i2cWrite_HM1375(0x082B ,0x02);
    i2cWrite_HM1375(0x082C ,0x91);
    i2cWrite_HM1375(0x082D ,0x02);
    i2cWrite_HM1375(0x082E ,0xDC);
    i2cWrite_HM1375(0x082F ,0x03);
    i2cWrite_HM1375(0x0830 ,0x28);
    i2cWrite_HM1375(0x0831 ,0x03);
    i2cWrite_HM1375(0x0832 ,0x74);
    i2cWrite_HM1375(0x0833 ,0x03);
    i2cWrite_HM1375(0x0834 ,0xFF);

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MINIMUM EDR
    //----------------------------------------------

    i2cWrite_HM1375(0x0882 ,0x00);
    i2cWrite_HM1375(0x0883 ,0x3E);
    i2cWrite_HM1375(0x0884 ,0x00);
    i2cWrite_HM1375(0x0885 ,0x70);
    i2cWrite_HM1375(0x0886 ,0x00);
    i2cWrite_HM1375(0x0887 ,0xB8);
    i2cWrite_HM1375(0x0888 ,0x01);
    i2cWrite_HM1375(0x0889 ,0x28);
    i2cWrite_HM1375(0x088A ,0x01);
    i2cWrite_HM1375(0x088B ,0x5B);
    i2cWrite_HM1375(0x088C ,0x01);
    i2cWrite_HM1375(0x088D ,0x8A);
    i2cWrite_HM1375(0x088E ,0x01);
    i2cWrite_HM1375(0x088F ,0xB1);
    i2cWrite_HM1375(0x0890 ,0x01);
    i2cWrite_HM1375(0x0891 ,0xD9);
    i2cWrite_HM1375(0x0892 ,0x01);
    i2cWrite_HM1375(0x0893 ,0xEE);
    i2cWrite_HM1375(0x0894 ,0x02);
    i2cWrite_HM1375(0x0895 ,0x0F);
    i2cWrite_HM1375(0x0896 ,0x02);
    i2cWrite_HM1375(0x0897 ,0x4C);
    i2cWrite_HM1375(0x0898 ,0x02);
    i2cWrite_HM1375(0x0899 ,0x74);
    i2cWrite_HM1375(0x089A ,0x02);
    i2cWrite_HM1375(0x089B ,0xC3);
    i2cWrite_HM1375(0x089C ,0x03);
    i2cWrite_HM1375(0x089D ,0x0F);
    i2cWrite_HM1375(0x089E ,0x03);
    i2cWrite_HM1375(0x089F ,0x57);
    i2cWrite_HM1375(0x08A0 ,0x03);
    i2cWrite_HM1375(0x08A1 ,0xFF);

    //----------------------------------------------
    //COMMAND UPDATE_TRIGGER
    //----------------------------------------------

    i2cWrite_HM1375(0x0100 ,0x01);  // CMU AE
    i2cWrite_HM1375(0x0101 ,0x01);  // CMU AWB
    i2cWrite_HM1375(0x0000 ,0x01);	// CMU
    i2cWrite_HM1375(0x002C ,0x00);  // Reset 8051
    i2cWrite_HM1375(0x0005 ,0x01);  // Trigger

}
#else
{
    int i;
	u8	data;
    int count;
    
    DEBUG_SIU("SetHM1375_720P_15FPS_TRANWO_TEST()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);

        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    i2cWrite_HM1375(0x0022, 0x01);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);
    i2cWrite_HM1375(0x0022, 0x00);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);

    i2cWrite_HM1375(0x0004, 0x18);  // hw reset mode
    
    i2cWrite_HM1375(0x000C, 0x04);  // MODE      _Reserved bit
    i2cWrite_HM1375(0x0006, 0x08);
    //i2cWrite_HM1375(0x0006, 0x0B);  // Flip
    i2cWrite_HM1375(0x000A, 0x00);  // CHIPCFG _Full frame, chip default 00
    i2cWrite_HM1375(0x000F, 0x10);  // IMGCFG
    i2cWrite_HM1375(0x0012, 0x01);  // RESERVED 
    i2cWrite_HM1375(0x0013, 0x02);  // RESERVED
    i2cWrite_HM1375(0x0015, 0x01);  // INTG_H    _Set up integration
    i2cWrite_HM1375(0x0016, 0x00);  // INTG_L    _Set up integration
    i2cWrite_HM1375(0x0018, 0x00);  // AGAIN     _Set up coarse gain
    i2cWrite_HM1375(0x001D, 0x40);  // DGGAIN    _Set up fine gain
    i2cWrite_HM1375(0x0020, 0x10);  // OPRTCFG   _AE Gain enabled, Pclk transition on falling edge
#if (HW_BOARD_OPTION == MR8211_ZINWELL)
    i2cWrite_HM1375(0x0023, 0x00);  // IOCNTR    _IO pad drive strength, IO & PCLK pad 電流都降到最低 for EMI testing
#else
    i2cWrite_HM1375(0x0023, 0x43);  // IOCNTR    _IO pad drive strength
#endif
    i2cWrite_HM1375(0x0024, 0x20);  // CKCNTR    _Reserved bit
//#if (HM1375_FRAME_RATE == 30)
#if 0
    i2cWrite_HM1375(0x0025, 0x00);  // CKCFG     _Select PLL clock -> 24MHz to 78 MHz
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0025, 0x01);  // CKCFG     _Select PLL clock -> 24MHz to 39 MHz
#endif
    i2cWrite_HM1375(0x0026, 0x6C);
    //i2cWrite_HM1375(0x0027, 0x30);  // OPORTCNTR _YUV output, ITU601 Disable, YCbYCr...
    i2cWrite_HM1375(0x0027, 0x10);  // OPORTCNTR _YUV output, ITU601 Disable, CbYCrY...
    i2cWrite_HM1375(0x0028, 0x01);  // CKMUL     _Reserved bit
    i2cWrite_HM1375(0x0030, 0x00);  // EDRCFG    _Disable EDR mode
    i2cWrite_HM1375(0x0034, 0x0E);  // EDRBLEND  _Set EDR blend to 0.875
    i2cWrite_HM1375(0x0035, 0x01);  // RESERVED
    i2cWrite_HM1375(0x0036, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0038, 0x02);  // EDR2AGAIN _Set up EDR2 coarse gain
    i2cWrite_HM1375(0x0039, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003A, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003B, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003C, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003D, 0x40);  // EDR2DGAIN _Set up EDR2 fine gain
    i2cWrite_HM1375(0x003F, 0x14);  // RESERVED      less gradient effect

    //----------------------------------------------
    //BLACK LEVEL CONTROL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0040, 0x10);  // BLCTGT    _Black level target
    i2cWrite_HM1375(0x0044, 0x07);  // BLCCFG    _BLC configuration enable, reserved bit

    //----------------------------------------------
    //RESERVED REGISTERS FOR SENSOR READOUT_TIMING
    //----------------------------------------------

    i2cWrite_HM1375(0x0045, 0x35);  // RESERVED       
    i2cWrite_HM1375(0x0048, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x004E, 0xFF);  // RESERVED 
    i2cWrite_HM1375(0x0070, 0x22);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0071, 0x3F);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0072, 0x22);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0073, 0x30);  // RESERVED    - 0920 updated by Willie
    i2cWrite_HM1375(0x0074, 0x13);  // RESERVED
    i2cWrite_HM1375(0x0075, 0x40);  // RESERVED
    i2cWrite_HM1375(0x0076, 0x24);  // RESERVED
    i2cWrite_HM1375(0x0078, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x007A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x007B, 0x14);  // RESERVED
    i2cWrite_HM1375(0x007C, 0x10);  // RESERVED
    i2cWrite_HM1375(0x0080, 0xC9);  // RESERVED
    i2cWrite_HM1375(0x0081, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0082, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0083, 0xB0);  // RESERVED
    i2cWrite_HM1375(0x0084, 0x60);  // RESERVED
    i2cWrite_HM1375(0x0086, 0x3E);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0087, 0x70);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0088, 0x11);  // RESERVED
    i2cWrite_HM1375(0x0089, 0x3C);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x008A, 0x87);  // RESERVED   // Rev.E updated by Willie
    i2cWrite_HM1375(0x008D, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0090, 0x07);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0091, 0x09);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0092, 0x0C);  
    i2cWrite_HM1375(0x0093, 0x0C);  
    i2cWrite_HM1375(0x0094, 0x0C);  
    i2cWrite_HM1375(0x0095, 0x0C);  
    i2cWrite_HM1375(0x0096, 0x01);  // AGAIN gain (nonEDR) table update for CFPN improvement 0824
    i2cWrite_HM1375(0x0097, 0x00);  
    i2cWrite_HM1375(0x0098, 0x04);  
    i2cWrite_HM1375(0x0099, 0x08);  
    i2cWrite_HM1375(0x009A, 0x0C);  


    //----------------------------------------------
    //IMAGE PIPELINE PROCESSING CONTROL
    //----------------------------------------------

    //------Flicker adjustment-----//
    if(AE_Flicker_50_60_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
    i2cWrite_HM1375(0x0121, 0x81);  // IPPCNTR2
    i2cWrite_HM1375(0x0122, 0xEB);  // IPPCNTR3
    i2cWrite_HM1375(0x0123, 0x29);  // IPPCNTR4
    i2cWrite_HM1375(0x0124, 0x50);  // CCMCNTR
    i2cWrite_HM1375(0x0125, 0xDE);  // IPPCNTR5
    i2cWrite_HM1375(0x0126, 0xB1);  // IPPCNTR6

    //----------------------------------------------
    //FLARE CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x013D, 0x0F);  
    i2cWrite_HM1375(0x013E, 0x0F);  
    i2cWrite_HM1375(0x013F, 0x0F);  

    //----------------------------------------------
    //BAD PIXEL CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0140, 0x14); 
    i2cWrite_HM1375(0x0141, 0x0A);
    i2cWrite_HM1375(0x0142, 0x14);
    i2cWrite_HM1375(0x0143, 0x0A);

    //----------------------------------------------
    //RESERVED
    //----------------------------------------------

    i2cWrite_HM1375(0x0144, 0x08);  // RESERVED
    i2cWrite_HM1375(0x0145, 0x04);  // RESERVED
    i2cWrite_HM1375(0x0146, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0147, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x0148, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0149, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x014A, 0x96);  // RESERVED
    i2cWrite_HM1375(0x014B, 0xC8);  // RESERVED

    //----------------------------------------------
    //SHARPENING CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0150, 0x14);  // SHPTHLR   
    i2cWrite_HM1375(0x0151, 0x30);  // SHPTHLR_A 
    i2cWrite_HM1375(0x0152, 0x54);  // SHPTHHR   
    i2cWrite_HM1375(0x0153, 0x70);  // SHPTHHR_A 
    i2cWrite_HM1375(0x0154, 0x14);  // SHPTHLG   
    i2cWrite_HM1375(0x0155, 0x30);  // SHPTHLG_A 
    i2cWrite_HM1375(0x0156, 0x54);  // SHPTHHG   
    i2cWrite_HM1375(0x0157, 0x70);  // SHPTHHG_A 
    i2cWrite_HM1375(0x0158, 0x14);  // SHPTHLB   
    i2cWrite_HM1375(0x0159, 0x30);  // SHPTHLB_A 
    i2cWrite_HM1375(0x015A, 0x54);  // SHPTHHB   
    i2cWrite_HM1375(0x015B, 0x70);  // SHPTHHB_A 
    i2cWrite_HM1375(0x015C, 0x30);  // SHPSTR    _Sharpness strength
    i2cWrite_HM1375(0x015D, 0x00);  // SHPSTR_A  _sharpness strength_Alpha0


    //----------------------------------------------
    // NOISE FILTER CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x01D8, 0x20);  // NFHTHG
    i2cWrite_HM1375(0x01D9, 0x08);  // NFLTHG
    i2cWrite_HM1375(0x01DA, 0x20);  // NFHTHB
    i2cWrite_HM1375(0x01DB, 0x08);  // NFLTHB
    i2cWrite_HM1375(0x01DC, 0x20);  // NFHTHR
    i2cWrite_HM1375(0x01DD, 0x08);  // NFLTHR
    i2cWrite_HM1375(0x01DE, 0x50);  // NFHTHG_A
    i2cWrite_HM1375(0x01E0, 0x50);  // NFHTHB_A
    i2cWrite_HM1375(0x01E2, 0x50);  // NFHTHR_A
    i2cWrite_HM1375(0x01E4, 0x10);  // NFSTR
    i2cWrite_HM1375(0x01E5, 0x10);  // NFSTR_A 
    i2cWrite_HM1375(0x01E6, 0x02);  // NFSTR_OUTA
    i2cWrite_HM1375(0x01E7, 0x10);  // NFMTHG   
    i2cWrite_HM1375(0x01E8, 0x10);  // NFMTHB  
    i2cWrite_HM1375(0x01E9, 0x10);  // NFMTHR
    i2cWrite_HM1375(0x01EC, 0x28);  // NFMTHR_A

    //----------------------------------------------
    //LENS SHADING CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0220, 0x00);
    i2cWrite_HM1375(0x0221, 0xA0);
    i2cWrite_HM1375(0x0222, 0x00);
    i2cWrite_HM1375(0x0223, 0x80);
    i2cWrite_HM1375(0x0224, 0x80);
    i2cWrite_HM1375(0x0225, 0x00);
    i2cWrite_HM1375(0x0226, 0x80);
    i2cWrite_HM1375(0x0227, 0x80);
    i2cWrite_HM1375(0x0228, 0x00);
    i2cWrite_HM1375(0x0229, 0x80);
    i2cWrite_HM1375(0x022A, 0x80);
    i2cWrite_HM1375(0x022B, 0x00);
    i2cWrite_HM1375(0x022C, 0x80);
    i2cWrite_HM1375(0x022D, 0x12);
    i2cWrite_HM1375(0x022E, 0x10);
    i2cWrite_HM1375(0x022F, 0x12);
    i2cWrite_HM1375(0x0230, 0x10);
    i2cWrite_HM1375(0x0231, 0x12);
    i2cWrite_HM1375(0x0232, 0x10);
    i2cWrite_HM1375(0x0233, 0x12);
    i2cWrite_HM1375(0x0234, 0x10);
    i2cWrite_HM1375(0x0235, 0x88);
    i2cWrite_HM1375(0x0236, 0x02);
    i2cWrite_HM1375(0x0237, 0x88);
    i2cWrite_HM1375(0x0238, 0x02);
    i2cWrite_HM1375(0x0239, 0x88);
    i2cWrite_HM1375(0x023A, 0x02);
    i2cWrite_HM1375(0x023B, 0x88);
    i2cWrite_HM1375(0x023C, 0x02);
    i2cWrite_HM1375(0x023D, 0x04);
    i2cWrite_HM1375(0x023E, 0x02);
    i2cWrite_HM1375(0x023F, 0x04);
    i2cWrite_HM1375(0x0240, 0x02);
    i2cWrite_HM1375(0x0241, 0x04);
    i2cWrite_HM1375(0x0242, 0x02);
    i2cWrite_HM1375(0x0243, 0x04);
    i2cWrite_HM1375(0x0244, 0x02);
    i2cWrite_HM1375(0x0251, 0x10);

    //----------------------------------------------
    //GAMMA CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0280, 0x00);  // normal Gamma
    i2cWrite_HM1375(0x0281, 0x46);  
    i2cWrite_HM1375(0x0282, 0x00);  
    i2cWrite_HM1375(0x0283, 0x77);  
    i2cWrite_HM1375(0x0284, 0x00);  
    i2cWrite_HM1375(0x0285, 0xCC);  
    i2cWrite_HM1375(0x0286, 0x01);  
    i2cWrite_HM1375(0x0287, 0x62);  
    i2cWrite_HM1375(0x0288, 0x01);  
    i2cWrite_HM1375(0x0289, 0x9B);  
    i2cWrite_HM1375(0x028A, 0x01);  
    i2cWrite_HM1375(0x028B, 0xCF);  
    i2cWrite_HM1375(0x028C, 0x01);  
    i2cWrite_HM1375(0x028D, 0xF6);  
    i2cWrite_HM1375(0x028E, 0x02);  
    i2cWrite_HM1375(0x028F, 0x1B);  
    i2cWrite_HM1375(0x0290, 0x02);  
    i2cWrite_HM1375(0x0291, 0x3B);  
    i2cWrite_HM1375(0x0292, 0x02);  
    i2cWrite_HM1375(0x0293, 0x59);  
    i2cWrite_HM1375(0x0294, 0x02);  
    i2cWrite_HM1375(0x0295, 0x91);  
    i2cWrite_HM1375(0x0296, 0x02);  
    i2cWrite_HM1375(0x0297, 0xC3);  
    i2cWrite_HM1375(0x0298, 0x03);  
    i2cWrite_HM1375(0x0299, 0x1B);  
    i2cWrite_HM1375(0x029A, 0x03);  
    i2cWrite_HM1375(0x029B, 0x65);  
    i2cWrite_HM1375(0x029C, 0x03);  
    i2cWrite_HM1375(0x029D, 0xA5);  // updated by MH 06/17
    i2cWrite_HM1375(0x029E, 0x00);  // slope high byte    
    i2cWrite_HM1375(0x029F, 0x78);  // slope low byte
    i2cWrite_HM1375(0x02A0, 0x04);  // GAM_A

    //----------------------------------------------
    //COLOR CORRECTION MATRIX
    //----------------------------------------------

    i2cWrite_HM1375(0x02C0, 0x80);  //CCM00_L    _Adjust for D65 color temperature
    i2cWrite_HM1375(0x02C1, 0x01);  //CCM00_H
    i2cWrite_HM1375(0x02C2, 0x71);  
    i2cWrite_HM1375(0x02C3, 0x04);  
    i2cWrite_HM1375(0x02C4, 0x0F);  
    i2cWrite_HM1375(0x02C5, 0x04);  
    i2cWrite_HM1375(0x02C6, 0x3D);  
    i2cWrite_HM1375(0x02C7, 0x04);  
    i2cWrite_HM1375(0x02C8, 0x94);  
    i2cWrite_HM1375(0x02C9, 0x01);  
    i2cWrite_HM1375(0x02CA, 0x57);  
    i2cWrite_HM1375(0x02CB, 0x04);  
    i2cWrite_HM1375(0x02CC, 0x0F);  
    i2cWrite_HM1375(0x02CD, 0x04);  
    i2cWrite_HM1375(0x02CE, 0x8F);  
    i2cWrite_HM1375(0x02CF, 0x04);  
    i2cWrite_HM1375(0x02D0, 0x9E);  
    i2cWrite_HM1375(0x02D1, 0x01);  
    i2cWrite_HM1375(0x02E0, 0x06);  // CCM_A     _reduce to 0.267x
    i2cWrite_HM1375(0x02E1, 0xC0);  // RESERVED
    i2cWrite_HM1375(0x02E2, 0xE0);  // RESERVED
    i2cWrite_HM1375(0x02F0, 0x48);  // ACCM00_L  _Adjust for IncA color temperature
    i2cWrite_HM1375(0x02F1, 0x01);  // ACCM00_H
    i2cWrite_HM1375(0x02F2, 0x32);  
    i2cWrite_HM1375(0x02F3, 0x04);  
    i2cWrite_HM1375(0x02F4, 0x16);  
    i2cWrite_HM1375(0x02F5, 0x04);  
    i2cWrite_HM1375(0x02F6, 0x52);  
    i2cWrite_HM1375(0x02F7, 0x04);  
    i2cWrite_HM1375(0x02F8, 0xAA);  
    i2cWrite_HM1375(0x02F9, 0x01);  
    i2cWrite_HM1375(0x02FA, 0x58);  
    i2cWrite_HM1375(0x02FB, 0x04);  
    i2cWrite_HM1375(0x02FC, 0x56);  
    i2cWrite_HM1375(0x02FD, 0x04);  
    i2cWrite_HM1375(0x02FE, 0xDD);  
    i2cWrite_HM1375(0x02FF, 0x04);  
    i2cWrite_HM1375(0x0300, 0x33);  
    i2cWrite_HM1375(0x0301, 0x02);  

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE WINDOW CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0324, 0x00);
    i2cWrite_HM1375(0x0325, 0x01);

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE DETECTION AND LIMITS
    //----------------------------------------------

    i2cWrite_HM1375(0x0333, 0x00);  
    i2cWrite_HM1375(0x0334, 0x00);  
    i2cWrite_HM1375(0x0335, 0x86);  
    i2cWrite_HM1375(0x0340, 0x40);      
    i2cWrite_HM1375(0x0341, 0x44);  
    i2cWrite_HM1375(0x0342, 0x4A);  
    i2cWrite_HM1375(0x0343, 0x2B);  
    i2cWrite_HM1375(0x0344, 0x94);  
    i2cWrite_HM1375(0x0345, 0x3F);  
    i2cWrite_HM1375(0x0346, 0x8E);  
    i2cWrite_HM1375(0x0347, 0x51);  
    i2cWrite_HM1375(0x0348, 0x75);  
    i2cWrite_HM1375(0x0349, 0x5C);  
    i2cWrite_HM1375(0x034A, 0x6A);  
    i2cWrite_HM1375(0x034B, 0x68);  
    i2cWrite_HM1375(0x034C, 0x5E);  
    i2cWrite_HM1375(0x0350, 0x7C);  
    i2cWrite_HM1375(0x0351, 0x78);  
    i2cWrite_HM1375(0x0352, 0x08);  
    i2cWrite_HM1375(0x0353, 0x04);  
    i2cWrite_HM1375(0x0354, 0x80);  
    i2cWrite_HM1375(0x0355, 0x9A);  
    i2cWrite_HM1375(0x0356, 0xCC);  
    i2cWrite_HM1375(0x0357, 0xFF);  
    i2cWrite_HM1375(0x0358, 0xFF);  
    i2cWrite_HM1375(0x035A, 0xFF);  
    i2cWrite_HM1375(0x035B, 0x00);  
    i2cWrite_HM1375(0x035C, 0x70);  
    i2cWrite_HM1375(0x035D, 0x80);  
    i2cWrite_HM1375(0x035F, 0xA0);  
    i2cWrite_HM1375(0x0488, 0x30);  
    i2cWrite_HM1375(0x0360, 0xDF);   
    i2cWrite_HM1375(0x0361, 0x00);  
    i2cWrite_HM1375(0x0362, 0xFF);  
    i2cWrite_HM1375(0x0363, 0x03);  
    i2cWrite_HM1375(0x0364, 0xFF);  
    i2cWrite_HM1375(0x037B, 0x11);  //whole nonEDR to EDR ratio 
    i2cWrite_HM1375(0x037C, 0x1E);  //EDR LONG litbin pop out to nonEDR 

    //----------------------------------------------
    //AUTOMATIC EXPOSURE CONFIGURATION
    //----------------------------------------------

    i2cWrite_HM1375(0x0380, 0xFF);       
    i2cWrite_HM1375(0x0383, 0x50);  // RESERVED       
    i2cWrite_HM1375(0x038A, 0x64);   
    i2cWrite_HM1375(0x038B, 0x64);   
    //i2cWrite_HM1375(0x038E, 0x3C);   
    i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);   
    i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
    i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
    i2cWrite_HM1375(0x0391, 0x2A);  // RESERVED        
    i2cWrite_HM1375(0x0393, 0x1E);  // RESERVED       
    i2cWrite_HM1375(0x0394, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0395, 0x23);         
    i2cWrite_HM1375(0x0398, 0x03);  // RESERVED
    i2cWrite_HM1375(0x0399, 0x45);  // RESERVED
    i2cWrite_HM1375(0x039A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x039B, 0x8B);  // RESERVED
    i2cWrite_HM1375(0x039C, 0x0D);  // RESERVED
    i2cWrite_HM1375(0x039D, 0x16);  // RESERVED
    i2cWrite_HM1375(0x039E, 0x0A);  // RESERVED
    i2cWrite_HM1375(0x039F, 0x10);       
    i2cWrite_HM1375(0x03A0, 0x10);          
    i2cWrite_HM1375(0x03A1, 0xE5);  // RESERVED   
    i2cWrite_HM1375(0x03A2, 0x06);  // RESERVED   
    i2cWrite_HM1375(0x03A4, 0x18);        
    i2cWrite_HM1375(0x03A5, 0x48);    
    i2cWrite_HM1375(0x03A6, 0x2D);  // RESERVED
    i2cWrite_HM1375(0x03A7, 0x78);  // RESERVED
    i2cWrite_HM1375(0x03AC, 0x5A);  // RESERVED
    i2cWrite_HM1375(0x03AD, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x03AE, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x03AF, 0x04);  // RESERVED
    i2cWrite_HM1375(0x03B0, 0x35);  // RESERVED
    i2cWrite_HM1375(0x03B1, 0x14);  // RESERVED
    i2cWrite_HM1375(0x036F, 0x04);  // Bayer denoise strength EDR
    i2cWrite_HM1375(0x0370, 0x0A);  // Bayer denoise strength nonEDR
    i2cWrite_HM1375(0x0371, 0x04);  // Bayer denoise strength EDR A0
    i2cWrite_HM1375(0x0372, 0x00);  // Bayer denoise strength nonEDR A0
    i2cWrite_HM1375(0x0373, 0x40);  // raw sharpness strength EDR
    i2cWrite_HM1375(0x0374, 0x20);  // raw sharpness strength nonEDR
    i2cWrite_HM1375(0x0375, 0x04);  // raw sharpness strength EDR A0
    i2cWrite_HM1375(0x0376, 0x00);  // raw sharpness strength nonEDR A0
    i2cWrite_HM1375(0x0377, 0x08);  // Y Denoise strength EDR
    i2cWrite_HM1375(0x0378, 0x08);  // Y Denoise strength nonEDR
    i2cWrite_HM1375(0x0379, 0x04);  // Y Denoise strength EDR A0
    i2cWrite_HM1375(0x037A, 0x08);  // Y Denoise strength nonEDR A0

    //----------------------------------------------
    //DIGITAL BLACK LEVEL OFFSET CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0420, 0x00);    
    i2cWrite_HM1375(0x0421, 0x00);    
    i2cWrite_HM1375(0x0422, 0x00);    
    i2cWrite_HM1375(0x0423, 0x84);    

    //----------------------------------------------
    //AUTO BLACK LEVEL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0430, 0x10);   
    i2cWrite_HM1375(0x0431, 0x60);   
    i2cWrite_HM1375(0x0432, 0x10);   
    i2cWrite_HM1375(0x0433, 0x20);   
    i2cWrite_HM1375(0x0434, 0x00);   
    i2cWrite_HM1375(0x0435, 0x30);   
    i2cWrite_HM1375(0x0436, 0x00);   

    //----------------------------------------------
    //LOWLIGHT_OUTDOOR IPP CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0450, 0xFD);   
    i2cWrite_HM1375(0x0451, 0xD8);   
    i2cWrite_HM1375(0x0452, 0xA0);   
    i2cWrite_HM1375(0x0453, 0x50);   
    i2cWrite_HM1375(0x0454, 0x00);       
    i2cWrite_HM1375(0x0459, 0x04);    
    i2cWrite_HM1375(0x045A, 0x00);   
    i2cWrite_HM1375(0x045B, 0x30);   
    i2cWrite_HM1375(0x045C, 0x01);   
    i2cWrite_HM1375(0x045D, 0x70);   
    i2cWrite_HM1375(0x0460, 0x00);    
    i2cWrite_HM1375(0x0461, 0x00);    
    i2cWrite_HM1375(0x0462, 0x00);    
    i2cWrite_HM1375(0x0465, 0x16);    
    i2cWrite_HM1375(0x0466, 0x14);    
    i2cWrite_HM1375(0x0478, 0x00);   

    //----------------------------------------------
    //COLOR SPACE CONVERSION_SATURATION ADJ
    //----------------------------------------------

    i2cWrite_HM1375(0x0480, 0x60);  //day:0x60 night:0x50
    i2cWrite_HM1375(0x0481, 0x06);  
    i2cWrite_HM1375(0x0482, 0x0C);  

    //----------------------------------------------
    //CONTRAST_BRIGHTNESS
    //----------------------------------------------


    i2cWrite_HM1375(0x04B0, 0x4C);  // Contrast 05032011 by mh
    i2cWrite_HM1375(0x04B1, 0x86);  // contrast 06/17 by mh
    i2cWrite_HM1375(0x04B2, 0x00);  //
    i2cWrite_HM1375(0x04B3, 0x18);  //
    i2cWrite_HM1375(0x04B4, 0x00);  //
    i2cWrite_HM1375(0x04B5, 0x00);  //
    i2cWrite_HM1375(0x04B6, 0x30);  //
    i2cWrite_HM1375(0x04B7, 0x00);  //
    i2cWrite_HM1375(0x04B8, 0x00);  //
    i2cWrite_HM1375(0x04B9, 0x10);  //
    i2cWrite_HM1375(0x04BA, 0x00);  //
    i2cWrite_HM1375(0x04BB, 0x00);  //
    i2cWrite_HM1375(0x04BD, 0x00);  //

    //----------------------------------------------
    //EDR CONTRAST
    //----------------------------------------------

    i2cWrite_HM1375(0x04D0, 0x56);     
    i2cWrite_HM1375(0x04D6, 0x30);  
    i2cWrite_HM1375(0x04DD, 0x10);  
    i2cWrite_HM1375(0x04D9, 0x16);  
    i2cWrite_HM1375(0x04D3, 0x18);  

    //----------------------------------------------
    //AE FLICKER STEP SIZE
    //----------------------------------------------

//#if (HM1375_FRAME_RATE == 30)
#if 0
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0xD0);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0xFA);  
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0x68);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0x7D);  
#endif
    i2cWrite_HM1375(0x0580, 0x50);  // RESERVED
    i2cWrite_HM1375(0x0581, 0x30);  // RESERVED

    //----------------------------------------------
    //Y_COLOR NOISE REDUCTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0582, 0x2D);  
    i2cWrite_HM1375(0x0583, 0x16);  
    i2cWrite_HM1375(0x0584, 0x1E);  
    i2cWrite_HM1375(0x0585, 0x0F);  
    i2cWrite_HM1375(0x0586, 0x08);  
    i2cWrite_HM1375(0x0587, 0x10);  
    i2cWrite_HM1375(0x0590, 0x10);  
    i2cWrite_HM1375(0x0591, 0x10);  
    i2cWrite_HM1375(0x0592, 0x05);  
    i2cWrite_HM1375(0x0593, 0x05);  
    i2cWrite_HM1375(0x0594, 0x04);  
    i2cWrite_HM1375(0x0595, 0x06);  

    //----------------------------------------------
    //Y_Sharpness strength
    //----------------------------------------------

    i2cWrite_HM1375(0x05B0, 0x04);    
    i2cWrite_HM1375(0x05B1, 0x00);    


    //----------------------------------------------
    //WINDOW_SCALER
    //----------------------------------------------

    i2cWrite_HM1375(0x05E4, 0x08);  
    i2cWrite_HM1375(0x05E5, 0x00);  
    i2cWrite_HM1375(0x05E6, 0x07);  
    i2cWrite_HM1375(0x05E7, 0x05);  
    i2cWrite_HM1375(0x05E8, 0x0A);  
    i2cWrite_HM1375(0x05E9, 0x00);  
    i2cWrite_HM1375(0x05EA, 0xD9);  
    i2cWrite_HM1375(0x05EB, 0x02);  

    //----------------------------------------------
    //FLEXI ENGINE_AE ADJUST CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0666, 0x02);        
    i2cWrite_HM1375(0x0667, 0xE0);      
    i2cWrite_HM1375(0x067F, 0x19);      
    i2cWrite_HM1375(0x067C, 0x00);  
    i2cWrite_HM1375(0x067D, 0x00);  
    i2cWrite_HM1375(0x0682, 0x00);  
    i2cWrite_HM1375(0x0683, 0x00);      
    i2cWrite_HM1375(0x0688, 0x00);  
    i2cWrite_HM1375(0x0689, 0x00);  
    i2cWrite_HM1375(0x068E, 0x00);  
    i2cWrite_HM1375(0x068F, 0x00);  
    i2cWrite_HM1375(0x0695, 0x00);   
    i2cWrite_HM1375(0x0694, 0x00);      
    i2cWrite_HM1375(0x0697, 0x19);      
    i2cWrite_HM1375(0x069B, 0x00);  
    i2cWrite_HM1375(0x069C, 0x30);  // max EDR ratio   
    i2cWrite_HM1375(0x0720, 0x00);  
    i2cWrite_HM1375(0x0725, 0x6A);      
    i2cWrite_HM1375(0x0726, 0x03);  
    i2cWrite_HM1375(0x072B, 0x64);  
    i2cWrite_HM1375(0x072C, 0x64);  
    i2cWrite_HM1375(0x072D, 0x20);  
    i2cWrite_HM1375(0x072E, 0x82);  //turn off night mode
    i2cWrite_HM1375(0x072F, 0x08);      
    i2cWrite_HM1375(0x0800, 0x16);  
    i2cWrite_HM1375(0x0801, 0x30);    
    i2cWrite_HM1375(0x0802, 0x00);  
    i2cWrite_HM1375(0x0803, 0x68);  
    i2cWrite_HM1375(0x0804, 0x01);  
    i2cWrite_HM1375(0x0805, 0x28);  
    i2cWrite_HM1375(0x0806, 0x10);   
    i2cWrite_HM1375(0x0808, 0x1D);  
    i2cWrite_HM1375(0x0809, 0x18);  
    i2cWrite_HM1375(0x080A, 0x10);       
    i2cWrite_HM1375(0x080B, 0x07);       
    i2cWrite_HM1375(0x080D, 0x0F);  
    i2cWrite_HM1375(0x080E, 0x0F);    
    i2cWrite_HM1375(0x0810, 0x00);  
    i2cWrite_HM1375(0x0811, 0x08);       
    i2cWrite_HM1375(0x0812, 0x20);  
    i2cWrite_HM1375(0x0857, 0x0A);  
    i2cWrite_HM1375(0x0858, 0x30);  
    i2cWrite_HM1375(0x0859, 0x01);  
#if 0   // 15fps
    i2cWrite_HM1375(0x085A, 0x06);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x80);  
#else   // 30fps
    i2cWrite_HM1375(0x085A, 0x03);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x40);  
#endif
    i2cWrite_HM1375(0x085C, 0x03);  
    i2cWrite_HM1375(0x085D, 0x7F);  
    i2cWrite_HM1375(0x085E, 0x02);  //(Long)Max INTG  
    i2cWrite_HM1375(0x085F, 0xD0);  
    i2cWrite_HM1375(0x0860, 0x03);      
    i2cWrite_HM1375(0x0861, 0x7F);  
    i2cWrite_HM1375(0x0862, 0x02);  //(short)Max INTG     
    i2cWrite_HM1375(0x0863, 0xD0);  
    i2cWrite_HM1375(0x0864, 0x00);  //(short)Max AG   
    i2cWrite_HM1375(0x0865, 0x7F);  
    i2cWrite_HM1375(0x0866, 0x01);  
    i2cWrite_HM1375(0x0867, 0x00);  
    i2cWrite_HM1375(0x0868, 0x40);  
    i2cWrite_HM1375(0x0869, 0x01);  
    i2cWrite_HM1375(0x086A, 0x00);  
    i2cWrite_HM1375(0x086B, 0x40);  
    i2cWrite_HM1375(0x086C, 0x01);  
    i2cWrite_HM1375(0x086D, 0x00);  
    i2cWrite_HM1375(0x086E, 0x40);  
    i2cWrite_HM1375(0x0870, 0x00);  
    i2cWrite_HM1375(0x0871, 0x14);  
    i2cWrite_HM1375(0x0872, 0x01);  
    i2cWrite_HM1375(0x0873, 0x20);  
    i2cWrite_HM1375(0x0874, 0x00);  
    i2cWrite_HM1375(0x0875, 0x14);  
    i2cWrite_HM1375(0x0876, 0x00);  
    i2cWrite_HM1375(0x0877, 0xEC);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MAXIMUM EDR 
    //----------------------------------------------

    i2cWrite_HM1375(0x0815, 0x00);  
    i2cWrite_HM1375(0x0816, 0x4C);  
    i2cWrite_HM1375(0x0817, 0x00);  
    i2cWrite_HM1375(0x0818, 0x7B);  
    i2cWrite_HM1375(0x0819, 0x00);  
    i2cWrite_HM1375(0x081A, 0xCA);  
    i2cWrite_HM1375(0x081B, 0x01);  
    i2cWrite_HM1375(0x081C, 0x3E);  
    i2cWrite_HM1375(0x081D, 0x01);  
    i2cWrite_HM1375(0x081E, 0x77);  
    i2cWrite_HM1375(0x081F, 0x01);  
    i2cWrite_HM1375(0x0820, 0xAA);  
    i2cWrite_HM1375(0x0821, 0x01);  
    i2cWrite_HM1375(0x0822, 0xCE);  
    i2cWrite_HM1375(0x0823, 0x01);  
    i2cWrite_HM1375(0x0824, 0xEE);  
    i2cWrite_HM1375(0x0825, 0x02);  
    i2cWrite_HM1375(0x0826, 0x16);  
    i2cWrite_HM1375(0x0827, 0x02);  
    i2cWrite_HM1375(0x0828, 0x33);  
    i2cWrite_HM1375(0x0829, 0x02);  
    i2cWrite_HM1375(0x082A, 0x65);  
    i2cWrite_HM1375(0x082B, 0x02);  
    i2cWrite_HM1375(0x082C, 0x91);  
    i2cWrite_HM1375(0x082D, 0x02);  
    i2cWrite_HM1375(0x082E, 0xDC);  
    i2cWrite_HM1375(0x082F, 0x03);  
    i2cWrite_HM1375(0x0830, 0x28);  
    i2cWrite_HM1375(0x0831, 0x03);  
    i2cWrite_HM1375(0x0832, 0x74);  
    i2cWrite_HM1375(0x0833, 0x03);  
    i2cWrite_HM1375(0x0834, 0xFF);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MINIMUM EDR
    //----------------------------------------------

    i2cWrite_HM1375(0x0882, 0x00);  
    i2cWrite_HM1375(0x0883, 0x3E);  
    i2cWrite_HM1375(0x0884, 0x00);  
    i2cWrite_HM1375(0x0885, 0x70);  
    i2cWrite_HM1375(0x0886, 0x00);  
    i2cWrite_HM1375(0x0887, 0xB8);  
    i2cWrite_HM1375(0x0888, 0x01);  
    i2cWrite_HM1375(0x0889, 0x28);  
    i2cWrite_HM1375(0x088A, 0x01);  
    i2cWrite_HM1375(0x088B, 0x5B);  
    i2cWrite_HM1375(0x088C, 0x01);  
    i2cWrite_HM1375(0x088D, 0x8A);  
    i2cWrite_HM1375(0x088E, 0x01);  
    i2cWrite_HM1375(0x088F, 0xB1);  
    i2cWrite_HM1375(0x0890, 0x01);  
    i2cWrite_HM1375(0x0891, 0xD9);  
    i2cWrite_HM1375(0x0892, 0x01);  
    i2cWrite_HM1375(0x0893, 0xEE);  
    i2cWrite_HM1375(0x0894, 0x02);  
    i2cWrite_HM1375(0x0895, 0x0F);  
    i2cWrite_HM1375(0x0896, 0x02);  
    i2cWrite_HM1375(0x0897, 0x4C);  
    i2cWrite_HM1375(0x0898, 0x02);  
    i2cWrite_HM1375(0x0899, 0x74);  
    i2cWrite_HM1375(0x089A, 0x02);  
    i2cWrite_HM1375(0x089B, 0xC3);  
    i2cWrite_HM1375(0x089C, 0x03);  
    i2cWrite_HM1375(0x089D, 0x0F);  
    i2cWrite_HM1375(0x089E, 0x03);  
    i2cWrite_HM1375(0x089F, 0x57);  
    i2cWrite_HM1375(0x08A0, 0x03);  
    i2cWrite_HM1375(0x08A1, 0xFF);  

    //----------------------------------------------
    //COMMAND UPDATE_TRIGGER
    //----------------------------------------------
    //OSTimeDly(4);
    i2cWrite_HM1375(0x0100, 0x01);  // CMU AE
    i2cWrite_HM1375(0x0101, 0x01);  // CMU AWB
    i2cWrite_HM1375(0x0000, 0x01);  // CMU
    i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger

}
#endif
#endif

void SetHM1375_720P_change_15FPS(void)
{
    DEBUG_SIU("SetHM1375_720P_change_15FPS()\n");
    i2cWrite_HM1375(0x0025, 0x01);  // CKCFG     _Select PLL clock -> 24MHz to 39 MHz
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0x68);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0x7D);  

}

void SetHM1375_720P_change_30FPS(void)
{
    DEBUG_SIU("SetHM1375_720P_change_30FPS()\n");
    i2cWrite_HM1375(0x0025, 0x00);  // CKCFG     _Select PLL clock -> 24MHz to 78 MHz
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0xD0);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0xFA);  

}

void SetHM1375_720P_15FPS(void)
{
    int i;
	u8	data;
    int count;
    
    DEBUG_SIU("SetHM1375_720P_15FPS()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);

        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    i2cWrite_HM1375(0x0022, 0x01);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);
    i2cWrite_HM1375(0x0022, 0x00);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);

    i2cWrite_HM1375(0x0004, 0x18);  // hw reset mode
    
    i2cWrite_HM1375(0x000C, 0x04);  // MODE      _Reserved bit
    i2cWrite_HM1375(0x0006, 0x08);
    //i2cWrite_HM1375(0x0006, 0x0B);  // Flip
    i2cWrite_HM1375(0x000A, 0x00);  // CHIPCFG _Full frame, chip default 00
    i2cWrite_HM1375(0x000F, 0x10);  // IMGCFG
    i2cWrite_HM1375(0x0012, 0x01);  // RESERVED 
    i2cWrite_HM1375(0x0013, 0x02);  // RESERVED
    i2cWrite_HM1375(0x0015, 0x01);  // INTG_H    _Set up integration
    i2cWrite_HM1375(0x0016, 0x00);  // INTG_L    _Set up integration
    i2cWrite_HM1375(0x0018, 0x00);  // AGAIN     _Set up coarse gain
    i2cWrite_HM1375(0x001D, 0x40);  // DGGAIN    _Set up fine gain
    i2cWrite_HM1375(0x0020, 0x10);  // OPRTCFG   _AE Gain enabled, Pclk transition on falling edge
#if (HW_BOARD_OPTION == MR8211_ZINWELL)
    i2cWrite_HM1375(0x0023, 0x00);  // IOCNTR    _IO pad drive strength, IO & PCLK pad 電流都降到最低 for EMI testing
#else
    i2cWrite_HM1375(0x0023, 0x43);  // IOCNTR    _IO pad drive strength
#endif
    i2cWrite_HM1375(0x0024, 0x20);  // CKCNTR    _Reserved bit
//#if (HM1375_FRAME_RATE == 30)
#if 0
    i2cWrite_HM1375(0x0025, 0x00);  // CKCFG     _Select PLL clock -> 24MHz to 78 MHz
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0025, 0x01);  // CKCFG     _Select PLL clock -> 24MHz to 39 MHz
#endif
    i2cWrite_HM1375(0x0026, 0x6C);

    //i2cWrite_HM1375(0x0027, 0x30);  // OPORTCNTR _YUV output, ITU601 Disable, YCbYCr...
    i2cWrite_HM1375(0x0027, 0x10);  // OPORTCNTR _YUV output, ITU601 Disable, CbYCrY...
    i2cWrite_HM1375(0x0028, 0x01);  // CKMUL     _Reserved bit
    i2cWrite_HM1375(0x0030, 0x00);  // EDRCFG    _Disable EDR mode
    i2cWrite_HM1375(0x0034, 0x0E);  // EDRBLEND  _Set EDR blend to 0.875
    i2cWrite_HM1375(0x0035, 0x01);  // RESERVED
    i2cWrite_HM1375(0x0036, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0038, 0x02);  // EDR2AGAIN _Set up EDR2 coarse gain
    i2cWrite_HM1375(0x0039, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003A, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003B, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003C, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003D, 0x40);  // EDR2DGAIN _Set up EDR2 fine gain
    i2cWrite_HM1375(0x003F, 0x14);  // RESERVED      less gradient effect

    //----------------------------------------------
    //BLACK LEVEL CONTROL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0040, 0x10);  // BLCTGT    _Black level target
    i2cWrite_HM1375(0x0044, 0x07);  // BLCCFG    _BLC configuration enable, reserved bit

    //----------------------------------------------
    //RESERVED REGISTERS FOR SENSOR READOUT_TIMING
    //----------------------------------------------

    i2cWrite_HM1375(0x0045, 0x35);  // RESERVED       
    i2cWrite_HM1375(0x0048, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x004E, 0xFF);  // RESERVED 
    i2cWrite_HM1375(0x0070, 0x22);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0071, 0x3F);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0072, 0x22);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0073, 0x30);  // RESERVED    - 0920 updated by Willie
    i2cWrite_HM1375(0x0074, 0x13);  // RESERVED
    i2cWrite_HM1375(0x0075, 0x40);  // RESERVED
    i2cWrite_HM1375(0x0076, 0x24);  // RESERVED
    i2cWrite_HM1375(0x0078, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x007A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x007B, 0x14);  // RESERVED
    i2cWrite_HM1375(0x007C, 0x10);  // RESERVED
    i2cWrite_HM1375(0x0080, 0xC9);  // RESERVED
    i2cWrite_HM1375(0x0081, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0082, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0083, 0xB0);  // RESERVED
    i2cWrite_HM1375(0x0084, 0x60);  // RESERVED
    i2cWrite_HM1375(0x0086, 0x3E);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0087, 0x70);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0088, 0x11);  // RESERVED
    i2cWrite_HM1375(0x0089, 0x3C);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x008A, 0x87);  // RESERVED   // Rev.E updated by Willie
    i2cWrite_HM1375(0x008D, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0090, 0x07);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0091, 0x09);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0092, 0x0C);  
    i2cWrite_HM1375(0x0093, 0x0C);  
    i2cWrite_HM1375(0x0094, 0x0C);  
    i2cWrite_HM1375(0x0095, 0x0C);  
    i2cWrite_HM1375(0x0096, 0x01);  // AGAIN gain (nonEDR) table update for CFPN improvement 0824
    i2cWrite_HM1375(0x0097, 0x00);  
    i2cWrite_HM1375(0x0098, 0x04);  
    i2cWrite_HM1375(0x0099, 0x08);  
    i2cWrite_HM1375(0x009A, 0x0C);  


    //----------------------------------------------
    //IMAGE PIPELINE PROCESSING CONTROL
    //----------------------------------------------

    //------Flicker adjustment-----//
    if(AE_Flicker_50_60_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
    i2cWrite_HM1375(0x0121, 0x81);  // IPPCNTR2
    i2cWrite_HM1375(0x0122, 0xEB);  // IPPCNTR3
    i2cWrite_HM1375(0x0123, 0x29);  // IPPCNTR4
    i2cWrite_HM1375(0x0124, 0x50);  // CCMCNTR
    i2cWrite_HM1375(0x0125, 0xDE);  // IPPCNTR5
    i2cWrite_HM1375(0x0126, 0xB1);  // IPPCNTR6

    //----------------------------------------------
    //FLARE CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x013D, 0x0F);  
    i2cWrite_HM1375(0x013E, 0x0F);  
    i2cWrite_HM1375(0x013F, 0x0F);  

    //----------------------------------------------
    //BAD PIXEL CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0140, 0x14); 
    i2cWrite_HM1375(0x0141, 0x0A);
    i2cWrite_HM1375(0x0142, 0x14);
    i2cWrite_HM1375(0x0143, 0x0A);

    //----------------------------------------------
    //RESERVED
    //----------------------------------------------

    i2cWrite_HM1375(0x0144, 0x08);  // RESERVED
    i2cWrite_HM1375(0x0145, 0x04);  // RESERVED
    i2cWrite_HM1375(0x0146, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0147, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x0148, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0149, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x014A, 0x96);  // RESERVED
    i2cWrite_HM1375(0x014B, 0xC8);  // RESERVED

    //----------------------------------------------
    //SHARPENING CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0150, 0x14);  // SHPTHLR   
    i2cWrite_HM1375(0x0151, 0x30);  // SHPTHLR_A 
    i2cWrite_HM1375(0x0152, 0x54);  // SHPTHHR   
    i2cWrite_HM1375(0x0153, 0x70);  // SHPTHHR_A 
    i2cWrite_HM1375(0x0154, 0x14);  // SHPTHLG   
    i2cWrite_HM1375(0x0155, 0x30);  // SHPTHLG_A 
    i2cWrite_HM1375(0x0156, 0x54);  // SHPTHHG   
    i2cWrite_HM1375(0x0157, 0x70);  // SHPTHHG_A 
    i2cWrite_HM1375(0x0158, 0x14);  // SHPTHLB   
    i2cWrite_HM1375(0x0159, 0x30);  // SHPTHLB_A 
    i2cWrite_HM1375(0x015A, 0x54);  // SHPTHHB   
    i2cWrite_HM1375(0x015B, 0x70);  // SHPTHHB_A 
    i2cWrite_HM1375(0x015C, 0x30);  // SHPSTR    _Sharpness strength
    i2cWrite_HM1375(0x015D, 0x00);  // SHPSTR_A  _sharpness strength_Alpha0


    //----------------------------------------------
    // NOISE FILTER CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x01D8, 0x20);  // NFHTHG
    i2cWrite_HM1375(0x01D9, 0x08);  // NFLTHG
    i2cWrite_HM1375(0x01DA, 0x20);  // NFHTHB
    i2cWrite_HM1375(0x01DB, 0x08);  // NFLTHB
    i2cWrite_HM1375(0x01DC, 0x20);  // NFHTHR
    i2cWrite_HM1375(0x01DD, 0x08);  // NFLTHR
    i2cWrite_HM1375(0x01DE, 0x50);  // NFHTHG_A
    i2cWrite_HM1375(0x01E0, 0x50);  // NFHTHB_A
    i2cWrite_HM1375(0x01E2, 0x50);  // NFHTHR_A
    i2cWrite_HM1375(0x01E4, 0x10);  // NFSTR
    i2cWrite_HM1375(0x01E5, 0x10);  // NFSTR_A 
    i2cWrite_HM1375(0x01E6, 0x02);  // NFSTR_OUTA
    i2cWrite_HM1375(0x01E7, 0x10);  // NFMTHG   
    i2cWrite_HM1375(0x01E8, 0x10);  // NFMTHB  
    i2cWrite_HM1375(0x01E9, 0x10);  // NFMTHR
    i2cWrite_HM1375(0x01EC, 0x28);  // NFMTHR_A

    //----------------------------------------------
    //LENS SHADING CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0220, 0x00);
    i2cWrite_HM1375(0x0221, 0xA0);
    i2cWrite_HM1375(0x0222, 0x00);
    i2cWrite_HM1375(0x0223, 0x80);
    i2cWrite_HM1375(0x0224, 0x80);
    i2cWrite_HM1375(0x0225, 0x00);
    i2cWrite_HM1375(0x0226, 0x80);
    i2cWrite_HM1375(0x0227, 0x80);
    i2cWrite_HM1375(0x0228, 0x00);
    i2cWrite_HM1375(0x0229, 0x80);
    i2cWrite_HM1375(0x022A, 0x80);
    i2cWrite_HM1375(0x022B, 0x00);
    i2cWrite_HM1375(0x022C, 0x80);
    i2cWrite_HM1375(0x022D, 0x12);
    i2cWrite_HM1375(0x022E, 0x10);
    i2cWrite_HM1375(0x022F, 0x12);
    i2cWrite_HM1375(0x0230, 0x10);
    i2cWrite_HM1375(0x0231, 0x12);
    i2cWrite_HM1375(0x0232, 0x10);
    i2cWrite_HM1375(0x0233, 0x12);
    i2cWrite_HM1375(0x0234, 0x10);
    i2cWrite_HM1375(0x0235, 0x88);
    i2cWrite_HM1375(0x0236, 0x02);
    i2cWrite_HM1375(0x0237, 0x88);
    i2cWrite_HM1375(0x0238, 0x02);
    i2cWrite_HM1375(0x0239, 0x88);
    i2cWrite_HM1375(0x023A, 0x02);
    i2cWrite_HM1375(0x023B, 0x88);
    i2cWrite_HM1375(0x023C, 0x02);
    i2cWrite_HM1375(0x023D, 0x04);
    i2cWrite_HM1375(0x023E, 0x02);
    i2cWrite_HM1375(0x023F, 0x04);
    i2cWrite_HM1375(0x0240, 0x02);
    i2cWrite_HM1375(0x0241, 0x04);
    i2cWrite_HM1375(0x0242, 0x02);
    i2cWrite_HM1375(0x0243, 0x04);
    i2cWrite_HM1375(0x0244, 0x02);
    i2cWrite_HM1375(0x0251, 0x10);

    //----------------------------------------------
    //GAMMA CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0280, 0x00);  // normal Gamma
    i2cWrite_HM1375(0x0281, 0x41);  
    i2cWrite_HM1375(0x0282, 0x00);  
    i2cWrite_HM1375(0x0283, 0x6D);  
    i2cWrite_HM1375(0x0284, 0x00);  
    i2cWrite_HM1375(0x0285, 0xBC);  
    i2cWrite_HM1375(0x0286, 0x01);  
    i2cWrite_HM1375(0x0287, 0x45);  
    i2cWrite_HM1375(0x0288, 0x01);  
    i2cWrite_HM1375(0x0289, 0x7B);  
    i2cWrite_HM1375(0x028A, 0x01);  
    i2cWrite_HM1375(0x028B, 0xAC);  
    i2cWrite_HM1375(0x028C, 0x01);  
    i2cWrite_HM1375(0x028D, 0xD2);  
    i2cWrite_HM1375(0x028E, 0x01);  
    i2cWrite_HM1375(0x028F, 0xF6);  
    i2cWrite_HM1375(0x0290, 0x02);  
    i2cWrite_HM1375(0x0291, 0x16);  
    i2cWrite_HM1375(0x0292, 0x02);  
    i2cWrite_HM1375(0x0293, 0x35);  
    i2cWrite_HM1375(0x0294, 0x02);  
    i2cWrite_HM1375(0x0295, 0x6E);  
    i2cWrite_HM1375(0x0296, 0x02);  
    i2cWrite_HM1375(0x0297, 0xA2);  
    i2cWrite_HM1375(0x0298, 0x02);  
    i2cWrite_HM1375(0x0299, 0xFF);  
    i2cWrite_HM1375(0x029A, 0x03);  
    i2cWrite_HM1375(0x029B, 0x51);  
    i2cWrite_HM1375(0x029C, 0x03);  
    i2cWrite_HM1375(0x029D, 0x9B);  // updated by MH 06/17
    i2cWrite_HM1375(0x029E, 0x00);  // slope high byte    
    i2cWrite_HM1375(0x029F, 0x85);  // slope low byte
    i2cWrite_HM1375(0x02A0, 0x04);  // GAM_A

    //----------------------------------------------
    //COLOR CORRECTION MATRIX
    //----------------------------------------------

    i2cWrite_HM1375(0x02C0, 0x80);  //CCM00_L    _Adjust for D65 color temperature
    i2cWrite_HM1375(0x02C1, 0x01);  //CCM00_H
    i2cWrite_HM1375(0x02C2, 0x71);  
    i2cWrite_HM1375(0x02C3, 0x04);  
    i2cWrite_HM1375(0x02C4, 0x0F);  
    i2cWrite_HM1375(0x02C5, 0x04);  
    i2cWrite_HM1375(0x02C6, 0x3D);  
    i2cWrite_HM1375(0x02C7, 0x04);  
    i2cWrite_HM1375(0x02C8, 0x94);  
    i2cWrite_HM1375(0x02C9, 0x01);  
    i2cWrite_HM1375(0x02CA, 0x57);  
    i2cWrite_HM1375(0x02CB, 0x04);  
    i2cWrite_HM1375(0x02CC, 0x0F);  
    i2cWrite_HM1375(0x02CD, 0x04);  
    i2cWrite_HM1375(0x02CE, 0x8F);  
    i2cWrite_HM1375(0x02CF, 0x04);  
    i2cWrite_HM1375(0x02D0, 0x9E);  
    i2cWrite_HM1375(0x02D1, 0x01);  
    i2cWrite_HM1375(0x02E0, 0x06);  // CCM_A     _reduce to 0.267x
    i2cWrite_HM1375(0x02E1, 0xC0);  // RESERVED
    i2cWrite_HM1375(0x02E2, 0xE0);  // RESERVED
    i2cWrite_HM1375(0x02F0, 0x48);  // ACCM00_L  _Adjust for IncA color temperature
    i2cWrite_HM1375(0x02F1, 0x01);  // ACCM00_H
    i2cWrite_HM1375(0x02F2, 0x32);  
    i2cWrite_HM1375(0x02F3, 0x04);  
    i2cWrite_HM1375(0x02F4, 0x16);  
    i2cWrite_HM1375(0x02F5, 0x04);  
    i2cWrite_HM1375(0x02F6, 0x52);  
    i2cWrite_HM1375(0x02F7, 0x04);  
    i2cWrite_HM1375(0x02F8, 0xAA);  
    i2cWrite_HM1375(0x02F9, 0x01);  
    i2cWrite_HM1375(0x02FA, 0x58);  
    i2cWrite_HM1375(0x02FB, 0x04);  
    i2cWrite_HM1375(0x02FC, 0x56);  
    i2cWrite_HM1375(0x02FD, 0x04);  
    i2cWrite_HM1375(0x02FE, 0xDD);  
    i2cWrite_HM1375(0x02FF, 0x04);  
    i2cWrite_HM1375(0x0300, 0x33);  
    i2cWrite_HM1375(0x0301, 0x02);  

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE WINDOW CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0324, 0x00);
    i2cWrite_HM1375(0x0325, 0x01);

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE DETECTION AND LIMITS
    //----------------------------------------------

    i2cWrite_HM1375(0x0333, 0x86);   
    i2cWrite_HM1375(0x0334, 0x00);  
    i2cWrite_HM1375(0x0335, 0x86);  
    i2cWrite_HM1375(0x0340, 0x40);      
    i2cWrite_HM1375(0x0341, 0x44);  
    i2cWrite_HM1375(0x0342, 0x4A);  
    i2cWrite_HM1375(0x0343, 0x2B);  
    i2cWrite_HM1375(0x0344, 0x94);  
    i2cWrite_HM1375(0x0345, 0x3F);  
    i2cWrite_HM1375(0x0346, 0x8E);  
    i2cWrite_HM1375(0x0347, 0x51);  
    i2cWrite_HM1375(0x0348, 0x75);  
    i2cWrite_HM1375(0x0349, 0x5C);  
    i2cWrite_HM1375(0x034A, 0x6A);  
    i2cWrite_HM1375(0x034B, 0x68);  
    i2cWrite_HM1375(0x034C, 0x5E);  
    i2cWrite_HM1375(0x0350, 0x7C);  
    i2cWrite_HM1375(0x0351, 0x78);  
    i2cWrite_HM1375(0x0352, 0x08);  
    i2cWrite_HM1375(0x0353, 0x04);  
    i2cWrite_HM1375(0x0354, 0x80);  
    i2cWrite_HM1375(0x0355, 0x9A);  
    i2cWrite_HM1375(0x0356, 0xCC);  
    i2cWrite_HM1375(0x0357, 0xFF);  
    i2cWrite_HM1375(0x0358, 0xFF);  
    i2cWrite_HM1375(0x035A, 0xFF);  
    i2cWrite_HM1375(0x035B, 0x00);  
    i2cWrite_HM1375(0x035C, 0x70);  
    i2cWrite_HM1375(0x035D, 0x80);  
    i2cWrite_HM1375(0x035F, 0xA0);  
    i2cWrite_HM1375(0x0488, 0x30);  
    i2cWrite_HM1375(0x0360, 0xDF);   
    i2cWrite_HM1375(0x0361, 0x00);  
    i2cWrite_HM1375(0x0362, 0xFF);  
    i2cWrite_HM1375(0x0363, 0x03);  
    i2cWrite_HM1375(0x0364, 0xFF);  
    i2cWrite_HM1375(0x037B, 0x11);  //whole nonEDR to EDR ratio 
    i2cWrite_HM1375(0x037C, 0x1E);  //EDR LONG litbin pop out to nonEDR 

    //----------------------------------------------
    //AUTOMATIC EXPOSURE CONFIGURATION
    //----------------------------------------------

    i2cWrite_HM1375(0x0380, 0xFF);       
    i2cWrite_HM1375(0x0383, 0x50);  // RESERVED       
    i2cWrite_HM1375(0x038A, 0x64);   
    i2cWrite_HM1375(0x038B, 0x64);   
  #if (((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT != 7)&&(UI_PROJ_OPT != 4)&&(UI_PROJ_OPT != 2)&&(UI_PROJ_OPT != 5)&&(UI_PROJ_OPT != 8))) ||\
    (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652) ||\
    (HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION  == MR8120S_TX_GCT_VM00)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)||\
    (HW_BOARD_OPTION == MR8120_TX_GCT_VM00))
    i2cWrite_HM1375(0x038E, AETargetMeanTab_720P[siuY_TargetIndex]);  
    i2cWrite_HM1375(0x0381, AETargetMeanTab_720P[siuY_TargetIndex]+12);
    i2cWrite_HM1375(0x0382, AETargetMeanTab_720P[siuY_TargetIndex]-12);
  #else
    i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);   
    i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
    i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
  #endif
    i2cWrite_HM1375(0x0391, 0x2A);  // RESERVED        
    i2cWrite_HM1375(0x0393, 0x1E);  // RESERVED       
    i2cWrite_HM1375(0x0394, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0395, 0x23);         
    i2cWrite_HM1375(0x0398, 0x03);  // RESERVED
    i2cWrite_HM1375(0x0399, 0x45);  // RESERVED
    i2cWrite_HM1375(0x039A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x039B, 0x8B);  // RESERVED
    i2cWrite_HM1375(0x039C, 0x0D);  // RESERVED
    i2cWrite_HM1375(0x039D, 0x16);  // RESERVED
    i2cWrite_HM1375(0x039E, 0x0A);  // RESERVED
    i2cWrite_HM1375(0x039F, 0x10);       
    i2cWrite_HM1375(0x03A0, 0x10);          
    i2cWrite_HM1375(0x03A1, 0xE5);  // RESERVED   
    i2cWrite_HM1375(0x03A2, 0x06);  // RESERVED   
    i2cWrite_HM1375(0x03A4, 0x18);        
    i2cWrite_HM1375(0x03A5, 0x48);    
    i2cWrite_HM1375(0x03A6, 0x2D);  // RESERVED
    i2cWrite_HM1375(0x03A7, 0x78);  // RESERVED
    i2cWrite_HM1375(0x03AC, 0x5A);  // RESERVED
    i2cWrite_HM1375(0x03AD, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x03AE, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x03AF, 0x04);  // RESERVED
    i2cWrite_HM1375(0x03B0, 0x35);  // RESERVED
    i2cWrite_HM1375(0x03B1, 0x14);  // RESERVED
    i2cWrite_HM1375(0x036F, 0x04);  // Bayer denoise strength EDR
    i2cWrite_HM1375(0x0370, 0x0A);  // Bayer denoise strength nonEDR
    i2cWrite_HM1375(0x0371, 0x04);  // Bayer denoise strength EDR A0
    i2cWrite_HM1375(0x0372, 0x00);  // Bayer denoise strength nonEDR A0
    i2cWrite_HM1375(0x0373, 0x40);  // raw sharpness strength EDR
    i2cWrite_HM1375(0x0374, 0x20);  // raw sharpness strength nonEDR
    i2cWrite_HM1375(0x0375, 0x04);  // raw sharpness strength EDR A0
    i2cWrite_HM1375(0x0376, 0x00);  // raw sharpness strength nonEDR A0
    i2cWrite_HM1375(0x0377, 0x08);  // Y Denoise strength EDR
    i2cWrite_HM1375(0x0378, 0x08);  // Y Denoise strength nonEDR
    i2cWrite_HM1375(0x0379, 0x04);  // Y Denoise strength EDR A0
    i2cWrite_HM1375(0x037A, 0x08);  // Y Denoise strength nonEDR A0

    //----------------------------------------------
    //DIGITAL BLACK LEVEL OFFSET CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0420, 0x00);    
    i2cWrite_HM1375(0x0421, 0x00);    
    i2cWrite_HM1375(0x0422, 0x00);    
    i2cWrite_HM1375(0x0423, 0x84);    

    //----------------------------------------------
    //AUTO BLACK LEVEL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0430, 0x10);   
    i2cWrite_HM1375(0x0431, 0x60);   
    i2cWrite_HM1375(0x0432, 0x10);   
    i2cWrite_HM1375(0x0433, 0x20);   
    i2cWrite_HM1375(0x0434, 0x00);   
    i2cWrite_HM1375(0x0435, 0x30);   
    i2cWrite_HM1375(0x0436, 0x00);   

    //----------------------------------------------
    //LOWLIGHT_OUTDOOR IPP CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0450, 0xFD);   
    i2cWrite_HM1375(0x0451, 0xD8);   
    i2cWrite_HM1375(0x0452, 0xA0);   
    i2cWrite_HM1375(0x0453, 0x50);   
    i2cWrite_HM1375(0x0454, 0x00);       
    i2cWrite_HM1375(0x0459, 0x04);    
    i2cWrite_HM1375(0x045A, 0x00);   
    i2cWrite_HM1375(0x045B, 0x30);   
    i2cWrite_HM1375(0x045C, 0x01);   
    i2cWrite_HM1375(0x045D, 0x70);   
    i2cWrite_HM1375(0x0460, 0x00);    
    i2cWrite_HM1375(0x0461, 0x00);    
    i2cWrite_HM1375(0x0462, 0x00);    
    i2cWrite_HM1375(0x0465, 0x16);    
    i2cWrite_HM1375(0x0466, 0x14);    
    i2cWrite_HM1375(0x0478, 0x00);   

    //----------------------------------------------
    //COLOR SPACE CONVERSION_SATURATION ADJ
    //----------------------------------------------

    i2cWrite_HM1375(0x0480, 0x60);
    i2cWrite_HM1375(0x0481, 0x06);  
    i2cWrite_HM1375(0x0482, 0x0C);  

    //----------------------------------------------
    //CONTRAST_BRIGHTNESS
    //----------------------------------------------


    i2cWrite_HM1375(0x04B0, 0x4C);  // Contrast 05032011 by mh
    i2cWrite_HM1375(0x04B1, 0x86);  // contrast 06/17 by mh
    i2cWrite_HM1375(0x04B2, 0x00);  //
    i2cWrite_HM1375(0x04B3, 0x18);  //
    i2cWrite_HM1375(0x04B4, 0x00);  //
    i2cWrite_HM1375(0x04B5, 0x00);  //
    i2cWrite_HM1375(0x04B6, 0x30);  //
    i2cWrite_HM1375(0x04B7, 0x00);  //
    i2cWrite_HM1375(0x04B8, 0x00);  //
    i2cWrite_HM1375(0x04B9, 0x10);  //
    i2cWrite_HM1375(0x04BA, 0x00);  //
    i2cWrite_HM1375(0x04BB, 0x00);  //
    i2cWrite_HM1375(0x04BD, 0x00);  //

    //----------------------------------------------
    //EDR CONTRAST
    //----------------------------------------------

    i2cWrite_HM1375(0x04D0, 0x56);     
    i2cWrite_HM1375(0x04D6, 0x30);  
    i2cWrite_HM1375(0x04DD, 0x10);  
    i2cWrite_HM1375(0x04D9, 0x16);  
    i2cWrite_HM1375(0x04D3, 0x18);  

    //----------------------------------------------
    //AE FLICKER STEP SIZE
    //----------------------------------------------

//#if (HM1375_FRAME_RATE == 30)
#if 0
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0xD0);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0xFA);  
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0x68);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0x7D);  
#endif
    i2cWrite_HM1375(0x0580, 0x50);  // RESERVED
    i2cWrite_HM1375(0x0581, 0x30);  // RESERVED

    //----------------------------------------------
    //Y_COLOR NOISE REDUCTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0582, 0x2D);  
    i2cWrite_HM1375(0x0583, 0x16);  
    i2cWrite_HM1375(0x0584, 0x1E);  
    i2cWrite_HM1375(0x0585, 0x0F);  
    i2cWrite_HM1375(0x0586, 0x08);  
    i2cWrite_HM1375(0x0587, 0x10);  
    i2cWrite_HM1375(0x0590, 0x10);  
    i2cWrite_HM1375(0x0591, 0x10);  
    i2cWrite_HM1375(0x0592, 0x05);  
    i2cWrite_HM1375(0x0593, 0x05);  
    i2cWrite_HM1375(0x0594, 0x04);  
    i2cWrite_HM1375(0x0595, 0x06);  

    //----------------------------------------------
    //Y_Sharpness strength
    //----------------------------------------------

    i2cWrite_HM1375(0x05B0, 0x04);    
    i2cWrite_HM1375(0x05B1, 0x00);    


    //----------------------------------------------
    //WINDOW_SCALER
    //----------------------------------------------

    i2cWrite_HM1375(0x05E4, 0x08);  
    i2cWrite_HM1375(0x05E5, 0x00);  
    i2cWrite_HM1375(0x05E6, 0x07);  
    i2cWrite_HM1375(0x05E7, 0x05);  
    i2cWrite_HM1375(0x05E8, 0x0A);  
    i2cWrite_HM1375(0x05E9, 0x00);  
    i2cWrite_HM1375(0x05EA, 0xD9);  
    i2cWrite_HM1375(0x05EB, 0x02);  

    //----------------------------------------------
    //FLEXI ENGINE_AE ADJUST CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0666, 0x02);        
    i2cWrite_HM1375(0x0667, 0xE0);      
    i2cWrite_HM1375(0x067F, 0x19);      
    i2cWrite_HM1375(0x067C, 0x00);  
    i2cWrite_HM1375(0x067D, 0x00);  
    i2cWrite_HM1375(0x0682, 0x00);  
    i2cWrite_HM1375(0x0683, 0x00);      
    i2cWrite_HM1375(0x0688, 0x00);  
    i2cWrite_HM1375(0x0689, 0x00);  
    i2cWrite_HM1375(0x068E, 0x00);  
    i2cWrite_HM1375(0x068F, 0x00);  
    i2cWrite_HM1375(0x0695, 0x00);   
    i2cWrite_HM1375(0x0694, 0x00);      
    i2cWrite_HM1375(0x0697, 0x19);      
    i2cWrite_HM1375(0x069B, 0x00);  
    i2cWrite_HM1375(0x069C, 0x30);  // max EDR ratio   
    i2cWrite_HM1375(0x0720, 0x00);  
    i2cWrite_HM1375(0x0725, 0x6A);      
    i2cWrite_HM1375(0x0726, 0x03);  
    i2cWrite_HM1375(0x072B, 0x64);  
    i2cWrite_HM1375(0x072C, 0x64);  
    i2cWrite_HM1375(0x072D, 0x20);  
    i2cWrite_HM1375(0x072E, 0x82);  //turn off night mode
    i2cWrite_HM1375(0x072F, 0x08);      
    i2cWrite_HM1375(0x0800, 0x16);  
    i2cWrite_HM1375(0x0801, 0x30);    
    i2cWrite_HM1375(0x0802, 0x00);  
    i2cWrite_HM1375(0x0803, 0x68);  
    i2cWrite_HM1375(0x0804, 0x01);  
    i2cWrite_HM1375(0x0805, 0x28);  
    i2cWrite_HM1375(0x0806, 0x10);   
    i2cWrite_HM1375(0x0808, 0x1D);  
    i2cWrite_HM1375(0x0809, 0x18);  
    i2cWrite_HM1375(0x080A, 0x10);       
    i2cWrite_HM1375(0x080B, 0x07);       
    i2cWrite_HM1375(0x080D, 0x0F);  
    i2cWrite_HM1375(0x080E, 0x0F);    
    i2cWrite_HM1375(0x0810, 0x00);  
    i2cWrite_HM1375(0x0811, 0x08);       
    i2cWrite_HM1375(0x0812, 0x20);  
    i2cWrite_HM1375(0x0857, 0x0A);  
    i2cWrite_HM1375(0x0858, 0x30);  
    i2cWrite_HM1375(0x0859, 0x01);  
#if 0   // 15fps
    i2cWrite_HM1375(0x085A, 0x06);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x80);  
#else   // 30fps
    i2cWrite_HM1375(0x085A, 0x03);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x40);  
#endif
    i2cWrite_HM1375(0x085C, 0x03);  
    i2cWrite_HM1375(0x085D, 0x7F);  
    i2cWrite_HM1375(0x085E, 0x02);  //(Long)Max INTG  
    i2cWrite_HM1375(0x085F, 0xD0);  
    i2cWrite_HM1375(0x0860, 0x03);      
    i2cWrite_HM1375(0x0861, 0x7F);  
    i2cWrite_HM1375(0x0862, 0x02);  //(short)Max INTG     
    i2cWrite_HM1375(0x0863, 0xD0);  
    i2cWrite_HM1375(0x0864, 0x00);  //(short)Max AG   
    i2cWrite_HM1375(0x0865, 0x7F);  
    i2cWrite_HM1375(0x0866, 0x01);  
    i2cWrite_HM1375(0x0867, 0x00);  
    i2cWrite_HM1375(0x0868, 0x40);  
    i2cWrite_HM1375(0x0869, 0x01);  
    i2cWrite_HM1375(0x086A, 0x00);  
    i2cWrite_HM1375(0x086B, 0x40);  
    i2cWrite_HM1375(0x086C, 0x01);  
    i2cWrite_HM1375(0x086D, 0x00);  
    i2cWrite_HM1375(0x086E, 0x40);  
    i2cWrite_HM1375(0x0870, 0x00);  
    i2cWrite_HM1375(0x0871, 0x14);  
    i2cWrite_HM1375(0x0872, 0x01);  
    i2cWrite_HM1375(0x0873, 0x20);  
    i2cWrite_HM1375(0x0874, 0x00);  
    i2cWrite_HM1375(0x0875, 0x14);  
    i2cWrite_HM1375(0x0876, 0x00);  
    i2cWrite_HM1375(0x0877, 0xEC);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MAXIMUM EDR 
    //----------------------------------------------

    i2cWrite_HM1375(0x0815, 0x00);  
    i2cWrite_HM1375(0x0816, 0x4C);  
    i2cWrite_HM1375(0x0817, 0x00);  
    i2cWrite_HM1375(0x0818, 0x7B);  
    i2cWrite_HM1375(0x0819, 0x00);  
    i2cWrite_HM1375(0x081A, 0xCA);  
    i2cWrite_HM1375(0x081B, 0x01);  
    i2cWrite_HM1375(0x081C, 0x3E);  
    i2cWrite_HM1375(0x081D, 0x01);  
    i2cWrite_HM1375(0x081E, 0x77);  
    i2cWrite_HM1375(0x081F, 0x01);  
    i2cWrite_HM1375(0x0820, 0xAA);  
    i2cWrite_HM1375(0x0821, 0x01);  
    i2cWrite_HM1375(0x0822, 0xCE);  
    i2cWrite_HM1375(0x0823, 0x01);  
    i2cWrite_HM1375(0x0824, 0xEE);  
    i2cWrite_HM1375(0x0825, 0x02);  
    i2cWrite_HM1375(0x0826, 0x16);  
    i2cWrite_HM1375(0x0827, 0x02);  
    i2cWrite_HM1375(0x0828, 0x33);  
    i2cWrite_HM1375(0x0829, 0x02);  
    i2cWrite_HM1375(0x082A, 0x65);  
    i2cWrite_HM1375(0x082B, 0x02);  
    i2cWrite_HM1375(0x082C, 0x91);  
    i2cWrite_HM1375(0x082D, 0x02);  
    i2cWrite_HM1375(0x082E, 0xDC);  
    i2cWrite_HM1375(0x082F, 0x03);  
    i2cWrite_HM1375(0x0830, 0x28);  
    i2cWrite_HM1375(0x0831, 0x03);  
    i2cWrite_HM1375(0x0832, 0x74);  
    i2cWrite_HM1375(0x0833, 0x03);  
    i2cWrite_HM1375(0x0834, 0xFF);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MINIMUM EDR
    //----------------------------------------------

    i2cWrite_HM1375(0x0882, 0x00);  
    i2cWrite_HM1375(0x0883, 0x3E);  
    i2cWrite_HM1375(0x0884, 0x00);  
    i2cWrite_HM1375(0x0885, 0x70);  
    i2cWrite_HM1375(0x0886, 0x00);  
    i2cWrite_HM1375(0x0887, 0xB8);  
    i2cWrite_HM1375(0x0888, 0x01);  
    i2cWrite_HM1375(0x0889, 0x28);  
    i2cWrite_HM1375(0x088A, 0x01);  
    i2cWrite_HM1375(0x088B, 0x5B);  
    i2cWrite_HM1375(0x088C, 0x01);  
    i2cWrite_HM1375(0x088D, 0x8A);  
    i2cWrite_HM1375(0x088E, 0x01);  
    i2cWrite_HM1375(0x088F, 0xB1);  
    i2cWrite_HM1375(0x0890, 0x01);  
    i2cWrite_HM1375(0x0891, 0xD9);  
    i2cWrite_HM1375(0x0892, 0x01);  
    i2cWrite_HM1375(0x0893, 0xEE);  
    i2cWrite_HM1375(0x0894, 0x02);  
    i2cWrite_HM1375(0x0895, 0x0F);  
    i2cWrite_HM1375(0x0896, 0x02);  
    i2cWrite_HM1375(0x0897, 0x4C);  
    i2cWrite_HM1375(0x0898, 0x02);  
    i2cWrite_HM1375(0x0899, 0x74);  
    i2cWrite_HM1375(0x089A, 0x02);  
    i2cWrite_HM1375(0x089B, 0xC3);  
    i2cWrite_HM1375(0x089C, 0x03);  
    i2cWrite_HM1375(0x089D, 0x0F);  
    i2cWrite_HM1375(0x089E, 0x03);  
    i2cWrite_HM1375(0x089F, 0x57);  
    i2cWrite_HM1375(0x08A0, 0x03);  
    i2cWrite_HM1375(0x08A1, 0xFF);  

    //----------------------------------------------
    //COMMAND UPDATE_TRIGGER
    //----------------------------------------------
    //OSTimeDly(4);
    i2cWrite_HM1375(0x0100, 0x01);  // CMU AE
    i2cWrite_HM1375(0x0101, 0x01);  // CMU AWB
    i2cWrite_HM1375(0x0000, 0x01);  // CMU
    i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger

}

void SetHM1375_720P_30FPS(void)
{
    int i;
	u8	data;
    int count;
    
    DEBUG_SIU("SetHM1375_720P_30FPS()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);

        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    i2cWrite_HM1375(0x0022, 0x01);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);
    i2cWrite_HM1375(0x0022, 0x00);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);

    i2cWrite_HM1375(0x0004, 0x18);  // hw reset mode
    
    i2cWrite_HM1375(0x000C, 0x04);  // MODE      _Reserved bit
    i2cWrite_HM1375(0x0006, 0x08);
    //i2cWrite_HM1375(0x0006, 0x0B);  // Flip
    i2cWrite_HM1375(0x000A, 0x00);  // CHIPCFG _Full frame, chip default 00
    i2cWrite_HM1375(0x000F, 0x10);  // IMGCFG
    i2cWrite_HM1375(0x0012, 0x01);  // RESERVED 
    i2cWrite_HM1375(0x0013, 0x02);  // RESERVED
    i2cWrite_HM1375(0x0015, 0x01);  // INTG_H    _Set up integration
    i2cWrite_HM1375(0x0016, 0x00);  // INTG_L    _Set up integration
    i2cWrite_HM1375(0x0018, 0x00);  // AGAIN     _Set up coarse gain
    i2cWrite_HM1375(0x001D, 0x40);  // DGGAIN    _Set up fine gain
    i2cWrite_HM1375(0x0020, 0x10);  // OPRTCFG   _AE Gain enabled, Pclk transition on falling edge
#if (HW_BOARD_OPTION == MR8211_ZINWELL)
    i2cWrite_HM1375(0x0023, 0x00);  // IOCNTR    _IO pad drive strength, IO & PCLK pad 電流都降到最低 for EMI testing
#else
    i2cWrite_HM1375(0x0023, 0x43);  // IOCNTR    _IO pad drive strength
#endif
    i2cWrite_HM1375(0x0024, 0x20);  // CKCNTR    _Reserved bit
//#if (HM1375_FRAME_RATE == 30)
#if 1
    i2cWrite_HM1375(0x0025, 0x00);  // CKCFG     _Select PLL clock -> 24MHz to 78 MHz
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0025, 0x01);  // CKCFG     _Select PLL clock -> 24MHz to 39 MHz
#endif
    i2cWrite_HM1375(0x0026, 0x6C);
    //i2cWrite_HM1375(0x0027, 0x30);  // OPORTCNTR _YUV output, ITU601 Disable, YCbYCr...
    i2cWrite_HM1375(0x0027, 0x10);  // OPORTCNTR _YUV output, ITU601 Disable, CbYCrY...
    i2cWrite_HM1375(0x0028, 0x01);  // CKMUL     _Reserved bit
    i2cWrite_HM1375(0x0030, 0x00);  // EDRCFG    _Disable EDR mode
    i2cWrite_HM1375(0x0034, 0x0E);  // EDRBLEND  _Set EDR blend to 0.875
    i2cWrite_HM1375(0x0035, 0x01);  // RESERVED
    i2cWrite_HM1375(0x0036, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0038, 0x02);  // EDR2AGAIN _Set up EDR2 coarse gain
    i2cWrite_HM1375(0x0039, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003A, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003B, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003C, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003D, 0x40);  // EDR2DGAIN _Set up EDR2 fine gain
    i2cWrite_HM1375(0x003F, 0x14);  // RESERVED      less gradient effect

    //----------------------------------------------
    //BLACK LEVEL CONTROL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0040, 0x10);  // BLCTGT    _Black level target
    i2cWrite_HM1375(0x0044, 0x07);  // BLCCFG    _BLC configuration enable, reserved bit

    //----------------------------------------------
    //RESERVED REGISTERS FOR SENSOR READOUT_TIMING
    //----------------------------------------------

    i2cWrite_HM1375(0x0045, 0x35);  // RESERVED       
    i2cWrite_HM1375(0x0048, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x004E, 0xFF);  // RESERVED 
    i2cWrite_HM1375(0x0070, 0x22);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0071, 0x3F);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0072, 0x22);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0073, 0x30);  // RESERVED    - 0920 updated by Willie
    i2cWrite_HM1375(0x0074, 0x13);  // RESERVED
    i2cWrite_HM1375(0x0075, 0x40);  // RESERVED
    i2cWrite_HM1375(0x0076, 0x24);  // RESERVED
    i2cWrite_HM1375(0x0078, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x007A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x007B, 0x14);  // RESERVED
    i2cWrite_HM1375(0x007C, 0x10);  // RESERVED
    i2cWrite_HM1375(0x0080, 0xC9);  // RESERVED
    i2cWrite_HM1375(0x0081, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0082, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0083, 0xB0);  // RESERVED
    i2cWrite_HM1375(0x0084, 0x60);  // RESERVED
    i2cWrite_HM1375(0x0086, 0x3E);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0087, 0x70);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0088, 0x11);  // RESERVED
    i2cWrite_HM1375(0x0089, 0x3C);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x008A, 0x87);  // RESERVED   // Rev.E updated by Willie
    i2cWrite_HM1375(0x008D, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0090, 0x07);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0091, 0x09);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0092, 0x0C);  
    i2cWrite_HM1375(0x0093, 0x0C);  
    i2cWrite_HM1375(0x0094, 0x0C);  
    i2cWrite_HM1375(0x0095, 0x0C);  
    i2cWrite_HM1375(0x0096, 0x01);  // AGAIN gain (nonEDR) table update for CFPN improvement 0824
    i2cWrite_HM1375(0x0097, 0x00);  
    i2cWrite_HM1375(0x0098, 0x04);  
    i2cWrite_HM1375(0x0099, 0x08);  
    i2cWrite_HM1375(0x009A, 0x0C);  


    //----------------------------------------------
    //IMAGE PIPELINE PROCESSING CONTROL
    //----------------------------------------------

    //------Flicker adjustment-----//
    if(AE_Flicker_50_60_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
    i2cWrite_HM1375(0x0121, 0x81);  // IPPCNTR2
    i2cWrite_HM1375(0x0122, 0xEB);  // IPPCNTR3
    i2cWrite_HM1375(0x0123, 0x29);  // IPPCNTR4
    i2cWrite_HM1375(0x0124, 0x50);  // CCMCNTR
    i2cWrite_HM1375(0x0125, 0xDE);  // IPPCNTR5
    i2cWrite_HM1375(0x0126, 0xB1);  // IPPCNTR6

    //----------------------------------------------
    //FLARE CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x013D, 0x0F);  
    i2cWrite_HM1375(0x013E, 0x0F);  
    i2cWrite_HM1375(0x013F, 0x0F);  

    //----------------------------------------------
    //BAD PIXEL CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0140, 0x14); 
    i2cWrite_HM1375(0x0141, 0x0A);
    i2cWrite_HM1375(0x0142, 0x14);
    i2cWrite_HM1375(0x0143, 0x0A);

    //----------------------------------------------
    //RESERVED
    //----------------------------------------------

    i2cWrite_HM1375(0x0144, 0x08);  // RESERVED
    i2cWrite_HM1375(0x0145, 0x04);  // RESERVED
    i2cWrite_HM1375(0x0146, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0147, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x0148, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0149, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x014A, 0x96);  // RESERVED
    i2cWrite_HM1375(0x014B, 0xC8);  // RESERVED

    //----------------------------------------------
    //SHARPENING CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0150, 0x14);  // SHPTHLR   
    i2cWrite_HM1375(0x0151, 0x30);  // SHPTHLR_A 
    i2cWrite_HM1375(0x0152, 0x54);  // SHPTHHR   
    i2cWrite_HM1375(0x0153, 0x70);  // SHPTHHR_A 
    i2cWrite_HM1375(0x0154, 0x14);  // SHPTHLG   
    i2cWrite_HM1375(0x0155, 0x30);  // SHPTHLG_A 
    i2cWrite_HM1375(0x0156, 0x54);  // SHPTHHG   
    i2cWrite_HM1375(0x0157, 0x70);  // SHPTHHG_A 
    i2cWrite_HM1375(0x0158, 0x14);  // SHPTHLB   
    i2cWrite_HM1375(0x0159, 0x30);  // SHPTHLB_A 
    i2cWrite_HM1375(0x015A, 0x54);  // SHPTHHB   
    i2cWrite_HM1375(0x015B, 0x70);  // SHPTHHB_A 
    i2cWrite_HM1375(0x015C, 0x30);  // SHPSTR    _Sharpness strength
    i2cWrite_HM1375(0x015D, 0x00);  // SHPSTR_A  _sharpness strength_Alpha0


    //----------------------------------------------
    // NOISE FILTER CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x01D8, 0x20);  // NFHTHG
    i2cWrite_HM1375(0x01D9, 0x08);  // NFLTHG
    i2cWrite_HM1375(0x01DA, 0x20);  // NFHTHB
    i2cWrite_HM1375(0x01DB, 0x08);  // NFLTHB
    i2cWrite_HM1375(0x01DC, 0x20);  // NFHTHR
    i2cWrite_HM1375(0x01DD, 0x08);  // NFLTHR
    i2cWrite_HM1375(0x01DE, 0x50);  // NFHTHG_A
    i2cWrite_HM1375(0x01E0, 0x50);  // NFHTHB_A
    i2cWrite_HM1375(0x01E2, 0x50);  // NFHTHR_A
    i2cWrite_HM1375(0x01E4, 0x10);  // NFSTR
    i2cWrite_HM1375(0x01E5, 0x10);  // NFSTR_A 
    i2cWrite_HM1375(0x01E6, 0x02);  // NFSTR_OUTA
    i2cWrite_HM1375(0x01E7, 0x10);  // NFMTHG   
    i2cWrite_HM1375(0x01E8, 0x10);  // NFMTHB  
    i2cWrite_HM1375(0x01E9, 0x10);  // NFMTHR
    i2cWrite_HM1375(0x01EC, 0x28);  // NFMTHR_A

    //----------------------------------------------
    //LENS SHADING CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0220, 0x00);
    i2cWrite_HM1375(0x0221, 0xA0);
    i2cWrite_HM1375(0x0222, 0x00);
    i2cWrite_HM1375(0x0223, 0x80);
    i2cWrite_HM1375(0x0224, 0x80);
    i2cWrite_HM1375(0x0225, 0x00);
    i2cWrite_HM1375(0x0226, 0x80);
    i2cWrite_HM1375(0x0227, 0x80);
    i2cWrite_HM1375(0x0228, 0x00);
    i2cWrite_HM1375(0x0229, 0x80);
    i2cWrite_HM1375(0x022A, 0x80);
    i2cWrite_HM1375(0x022B, 0x00);
    i2cWrite_HM1375(0x022C, 0x80);
    i2cWrite_HM1375(0x022D, 0x12);
    i2cWrite_HM1375(0x022E, 0x10);
    i2cWrite_HM1375(0x022F, 0x12);
    i2cWrite_HM1375(0x0230, 0x10);
    i2cWrite_HM1375(0x0231, 0x12);
    i2cWrite_HM1375(0x0232, 0x10);
    i2cWrite_HM1375(0x0233, 0x12);
    i2cWrite_HM1375(0x0234, 0x10);
    i2cWrite_HM1375(0x0235, 0x88);
    i2cWrite_HM1375(0x0236, 0x02);
    i2cWrite_HM1375(0x0237, 0x88);
    i2cWrite_HM1375(0x0238, 0x02);
    i2cWrite_HM1375(0x0239, 0x88);
    i2cWrite_HM1375(0x023A, 0x02);
    i2cWrite_HM1375(0x023B, 0x88);
    i2cWrite_HM1375(0x023C, 0x02);
    i2cWrite_HM1375(0x023D, 0x04);
    i2cWrite_HM1375(0x023E, 0x02);
    i2cWrite_HM1375(0x023F, 0x04);
    i2cWrite_HM1375(0x0240, 0x02);
    i2cWrite_HM1375(0x0241, 0x04);
    i2cWrite_HM1375(0x0242, 0x02);
    i2cWrite_HM1375(0x0243, 0x04);
    i2cWrite_HM1375(0x0244, 0x02);
    i2cWrite_HM1375(0x0251, 0x10);

    //----------------------------------------------
    //GAMMA CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0280, 0x00);  // normal Gamma
    i2cWrite_HM1375(0x0281, 0x41);  
    i2cWrite_HM1375(0x0282, 0x00);  
    i2cWrite_HM1375(0x0283, 0x6D);  
    i2cWrite_HM1375(0x0284, 0x00);  
    i2cWrite_HM1375(0x0285, 0xBC);  
    i2cWrite_HM1375(0x0286, 0x01);  
    i2cWrite_HM1375(0x0287, 0x45);  
    i2cWrite_HM1375(0x0288, 0x01);  
    i2cWrite_HM1375(0x0289, 0x7B);  
    i2cWrite_HM1375(0x028A, 0x01);  
    i2cWrite_HM1375(0x028B, 0xAC);  
    i2cWrite_HM1375(0x028C, 0x01);  
    i2cWrite_HM1375(0x028D, 0xD2);  
    i2cWrite_HM1375(0x028E, 0x01);  
    i2cWrite_HM1375(0x028F, 0xF6);  
    i2cWrite_HM1375(0x0290, 0x02);  
    i2cWrite_HM1375(0x0291, 0x16);  
    i2cWrite_HM1375(0x0292, 0x02);  
    i2cWrite_HM1375(0x0293, 0x35);  
    i2cWrite_HM1375(0x0294, 0x02);  
    i2cWrite_HM1375(0x0295, 0x6E);  
    i2cWrite_HM1375(0x0296, 0x02);  
    i2cWrite_HM1375(0x0297, 0xA2);  
    i2cWrite_HM1375(0x0298, 0x02);  
    i2cWrite_HM1375(0x0299, 0xFF);  
    i2cWrite_HM1375(0x029A, 0x03);  
    i2cWrite_HM1375(0x029B, 0x51);  
    i2cWrite_HM1375(0x029C, 0x03);  
    i2cWrite_HM1375(0x029D, 0x9B);  // updated by MH 06/17
    i2cWrite_HM1375(0x029E, 0x00);  // slope high byte    
    i2cWrite_HM1375(0x029F, 0x85);  // slope low byte
    i2cWrite_HM1375(0x02A0, 0x04);  // GAM_A

    //----------------------------------------------
    //COLOR CORRECTION MATRIX
    //----------------------------------------------

    i2cWrite_HM1375(0x02C0, 0x80);  //CCM00_L    _Adjust for D65 color temperature
    i2cWrite_HM1375(0x02C1, 0x01);  //CCM00_H
    i2cWrite_HM1375(0x02C2, 0x71);  
    i2cWrite_HM1375(0x02C3, 0x04);  
    i2cWrite_HM1375(0x02C4, 0x0F);  
    i2cWrite_HM1375(0x02C5, 0x04);  
    i2cWrite_HM1375(0x02C6, 0x3D);  
    i2cWrite_HM1375(0x02C7, 0x04);  
    i2cWrite_HM1375(0x02C8, 0x94);  
    i2cWrite_HM1375(0x02C9, 0x01);  
    i2cWrite_HM1375(0x02CA, 0x57);  
    i2cWrite_HM1375(0x02CB, 0x04);  
    i2cWrite_HM1375(0x02CC, 0x0F);  
    i2cWrite_HM1375(0x02CD, 0x04);  
    i2cWrite_HM1375(0x02CE, 0x8F);  
    i2cWrite_HM1375(0x02CF, 0x04);  
    i2cWrite_HM1375(0x02D0, 0x9E);  
    i2cWrite_HM1375(0x02D1, 0x01);  
    i2cWrite_HM1375(0x02E0, 0x06);  // CCM_A     _reduce to 0.267x
    i2cWrite_HM1375(0x02E1, 0xC0);  // RESERVED
    i2cWrite_HM1375(0x02E2, 0xE0);  // RESERVED
    i2cWrite_HM1375(0x02F0, 0x48);  // ACCM00_L  _Adjust for IncA color temperature
    i2cWrite_HM1375(0x02F1, 0x01);  // ACCM00_H
    i2cWrite_HM1375(0x02F2, 0x32);  
    i2cWrite_HM1375(0x02F3, 0x04);  
    i2cWrite_HM1375(0x02F4, 0x16);  
    i2cWrite_HM1375(0x02F5, 0x04);  
    i2cWrite_HM1375(0x02F6, 0x52);  
    i2cWrite_HM1375(0x02F7, 0x04);  
    i2cWrite_HM1375(0x02F8, 0xAA);  
    i2cWrite_HM1375(0x02F9, 0x01);  
    i2cWrite_HM1375(0x02FA, 0x58);  
    i2cWrite_HM1375(0x02FB, 0x04);  
    i2cWrite_HM1375(0x02FC, 0x56);  
    i2cWrite_HM1375(0x02FD, 0x04);  
    i2cWrite_HM1375(0x02FE, 0xDD);  
    i2cWrite_HM1375(0x02FF, 0x04);  
    i2cWrite_HM1375(0x0300, 0x33);  
    i2cWrite_HM1375(0x0301, 0x02);  

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE WINDOW CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0324, 0x00);
    i2cWrite_HM1375(0x0325, 0x01);

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE DETECTION AND LIMITS
    //----------------------------------------------

    i2cWrite_HM1375(0x0333, 0x86);   
    i2cWrite_HM1375(0x0334, 0x00);  
    i2cWrite_HM1375(0x0335, 0x86);  
    i2cWrite_HM1375(0x0340, 0x40);      
    i2cWrite_HM1375(0x0341, 0x44);  
    i2cWrite_HM1375(0x0342, 0x4A);  
    i2cWrite_HM1375(0x0343, 0x2B);  
    i2cWrite_HM1375(0x0344, 0x94);  
    i2cWrite_HM1375(0x0345, 0x3F);  
    i2cWrite_HM1375(0x0346, 0x8E);  
    i2cWrite_HM1375(0x0347, 0x51);  
    i2cWrite_HM1375(0x0348, 0x75);  
    i2cWrite_HM1375(0x0349, 0x5C);  
    i2cWrite_HM1375(0x034A, 0x6A);  
    i2cWrite_HM1375(0x034B, 0x68);  
    i2cWrite_HM1375(0x034C, 0x5E);  
    i2cWrite_HM1375(0x0350, 0x7C);  
    i2cWrite_HM1375(0x0351, 0x78);  
    i2cWrite_HM1375(0x0352, 0x08);  
    i2cWrite_HM1375(0x0353, 0x04);  
    i2cWrite_HM1375(0x0354, 0x80);  
    i2cWrite_HM1375(0x0355, 0x9A);  
    i2cWrite_HM1375(0x0356, 0xCC);  
    i2cWrite_HM1375(0x0357, 0xFF);  
    i2cWrite_HM1375(0x0358, 0xFF);  
    i2cWrite_HM1375(0x035A, 0xFF);  
    i2cWrite_HM1375(0x035B, 0x00);  
    i2cWrite_HM1375(0x035C, 0x70);  
    i2cWrite_HM1375(0x035D, 0x80);  
    i2cWrite_HM1375(0x035F, 0xA0);  
    i2cWrite_HM1375(0x0488, 0x30);  
    i2cWrite_HM1375(0x0360, 0xDF);   
    i2cWrite_HM1375(0x0361, 0x00);  
    i2cWrite_HM1375(0x0362, 0xFF);  
    i2cWrite_HM1375(0x0363, 0x03);  
    i2cWrite_HM1375(0x0364, 0xFF);  
    i2cWrite_HM1375(0x037B, 0x11);  //whole nonEDR to EDR ratio 
    i2cWrite_HM1375(0x037C, 0x1E);  //EDR LONG litbin pop out to nonEDR 

    //----------------------------------------------
    //AUTOMATIC EXPOSURE CONFIGURATION
    //----------------------------------------------

    i2cWrite_HM1375(0x0380, 0xFF);       
    i2cWrite_HM1375(0x0383, 0x50);  // RESERVED       
    i2cWrite_HM1375(0x038A, 0x64);   
    i2cWrite_HM1375(0x038B, 0x64);   
  #if (((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT != 7)&&(UI_PROJ_OPT != 4)&&(UI_PROJ_OPT != 2)&&(UI_PROJ_OPT != 5)&&(UI_PROJ_OPT != 8))) ||\
    (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652)||\
    (HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION  == MR8120S_TX_GCT_VM00)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)||\
    (HW_BOARD_OPTION == MR8120_TX_GCT_VM00))
    i2cWrite_HM1375(0x038E, AETargetMeanTab_720P[siuY_TargetIndex]);  
    i2cWrite_HM1375(0x0381, AETargetMeanTab_720P[siuY_TargetIndex]+12);
    i2cWrite_HM1375(0x0382, AETargetMeanTab_720P[siuY_TargetIndex]-12);
  #else
    i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);   
    i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
    i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
  #endif
    i2cWrite_HM1375(0x0391, 0x2A);  // RESERVED        
    i2cWrite_HM1375(0x0393, 0x1E);  // RESERVED       
    i2cWrite_HM1375(0x0394, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0395, 0x23);         
    i2cWrite_HM1375(0x0398, 0x03);  // RESERVED
    i2cWrite_HM1375(0x0399, 0x45);  // RESERVED
    i2cWrite_HM1375(0x039A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x039B, 0x8B);  // RESERVED
    i2cWrite_HM1375(0x039C, 0x0D);  // RESERVED
    i2cWrite_HM1375(0x039D, 0x16);  // RESERVED
    i2cWrite_HM1375(0x039E, 0x0A);  // RESERVED
    i2cWrite_HM1375(0x039F, 0x10);       
    i2cWrite_HM1375(0x03A0, 0x10);          
    i2cWrite_HM1375(0x03A1, 0xE5);  // RESERVED   
    i2cWrite_HM1375(0x03A2, 0x06);  // RESERVED   
    i2cWrite_HM1375(0x03A4, 0x18);        
    i2cWrite_HM1375(0x03A5, 0x48);    
    i2cWrite_HM1375(0x03A6, 0x2D);  // RESERVED
    i2cWrite_HM1375(0x03A7, 0x78);  // RESERVED
    i2cWrite_HM1375(0x03AC, 0x5A);  // RESERVED
    i2cWrite_HM1375(0x03AD, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x03AE, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x03AF, 0x04);  // RESERVED
    i2cWrite_HM1375(0x03B0, 0x35);  // RESERVED
    i2cWrite_HM1375(0x03B1, 0x14);  // RESERVED
    i2cWrite_HM1375(0x036F, 0x04);  // Bayer denoise strength EDR
    i2cWrite_HM1375(0x0370, 0x0A);  // Bayer denoise strength nonEDR
    i2cWrite_HM1375(0x0371, 0x04);  // Bayer denoise strength EDR A0
    i2cWrite_HM1375(0x0372, 0x00);  // Bayer denoise strength nonEDR A0
    i2cWrite_HM1375(0x0373, 0x40);  // raw sharpness strength EDR
    i2cWrite_HM1375(0x0374, 0x20);  // raw sharpness strength nonEDR

    i2cWrite_HM1375(0x0375, 0x04);  // raw sharpness strength EDR A0
    i2cWrite_HM1375(0x0376, 0x00);  // raw sharpness strength nonEDR A0
    i2cWrite_HM1375(0x0377, 0x08);  // Y Denoise strength EDR
    i2cWrite_HM1375(0x0378, 0x08);  // Y Denoise strength nonEDR
    i2cWrite_HM1375(0x0379, 0x04);  // Y Denoise strength EDR A0
    i2cWrite_HM1375(0x037A, 0x08);  // Y Denoise strength nonEDR A0

    //----------------------------------------------
    //DIGITAL BLACK LEVEL OFFSET CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0420, 0x00);    
    i2cWrite_HM1375(0x0421, 0x00);    
    i2cWrite_HM1375(0x0422, 0x00);    
    i2cWrite_HM1375(0x0423, 0x84);    

    //----------------------------------------------
    //AUTO BLACK LEVEL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0430, 0x10);   
    i2cWrite_HM1375(0x0431, 0x60);   
    i2cWrite_HM1375(0x0432, 0x10);   
    i2cWrite_HM1375(0x0433, 0x20);   
    i2cWrite_HM1375(0x0434, 0x00);   
    i2cWrite_HM1375(0x0435, 0x30);   
    i2cWrite_HM1375(0x0436, 0x00);   

    //----------------------------------------------
    //LOWLIGHT_OUTDOOR IPP CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0450, 0xFD);   
    i2cWrite_HM1375(0x0451, 0xD8);   
    i2cWrite_HM1375(0x0452, 0xA0);   
    i2cWrite_HM1375(0x0453, 0x50);   
    i2cWrite_HM1375(0x0454, 0x00);       
    i2cWrite_HM1375(0x0459, 0x04);    
    i2cWrite_HM1375(0x045A, 0x00);   
    i2cWrite_HM1375(0x045B, 0x30);   
    i2cWrite_HM1375(0x045C, 0x01);   
    i2cWrite_HM1375(0x045D, 0x70);   
    i2cWrite_HM1375(0x0460, 0x00);    
    i2cWrite_HM1375(0x0461, 0x00);    
    i2cWrite_HM1375(0x0462, 0x00);    
    i2cWrite_HM1375(0x0465, 0x16);    
    i2cWrite_HM1375(0x0466, 0x14);    
    i2cWrite_HM1375(0x0478, 0x00);   

    //----------------------------------------------
    //COLOR SPACE CONVERSION_SATURATION ADJ
    //----------------------------------------------

    i2cWrite_HM1375(0x0480, 0x60);
    i2cWrite_HM1375(0x0481, 0x06);  
    i2cWrite_HM1375(0x0482, 0x0C);  

    //----------------------------------------------
    //CONTRAST_BRIGHTNESS
    //----------------------------------------------


    i2cWrite_HM1375(0x04B0, 0x4C);  // Contrast 05032011 by mh
    i2cWrite_HM1375(0x04B1, 0x86);  // contrast 06/17 by mh
    i2cWrite_HM1375(0x04B2, 0x00);  //
    i2cWrite_HM1375(0x04B3, 0x18);  //
    i2cWrite_HM1375(0x04B4, 0x00);  //
    i2cWrite_HM1375(0x04B5, 0x00);  //
    i2cWrite_HM1375(0x04B6, 0x30);  //
    i2cWrite_HM1375(0x04B7, 0x00);  //
    i2cWrite_HM1375(0x04B8, 0x00);  //
    i2cWrite_HM1375(0x04B9, 0x10);  //
    i2cWrite_HM1375(0x04BA, 0x00);  //
    i2cWrite_HM1375(0x04BB, 0x00);  //
    i2cWrite_HM1375(0x04BD, 0x00);  //

    //----------------------------------------------
    //EDR CONTRAST
    //----------------------------------------------

    i2cWrite_HM1375(0x04D0, 0x56);     
    i2cWrite_HM1375(0x04D6, 0x30);  
    i2cWrite_HM1375(0x04DD, 0x10);  
    i2cWrite_HM1375(0x04D9, 0x16);  
    i2cWrite_HM1375(0x04D3, 0x18);  

    //----------------------------------------------
    //AE FLICKER STEP SIZE
    //----------------------------------------------

//#if (HM1375_FRAME_RATE == 30)
#if 1
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0xD0);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0xFA);  
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0x68);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0x7D);  
#endif
    i2cWrite_HM1375(0x0580, 0x50);  // RESERVED
    i2cWrite_HM1375(0x0581, 0x30);  // RESERVED

    //----------------------------------------------
    //Y_COLOR NOISE REDUCTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0582, 0x2D);  
    i2cWrite_HM1375(0x0583, 0x16);  
    i2cWrite_HM1375(0x0584, 0x1E);  
    i2cWrite_HM1375(0x0585, 0x0F);  
    i2cWrite_HM1375(0x0586, 0x08);  
    i2cWrite_HM1375(0x0587, 0x10);  
    i2cWrite_HM1375(0x0590, 0x10);  
    i2cWrite_HM1375(0x0591, 0x10);  
    i2cWrite_HM1375(0x0592, 0x05);  
    i2cWrite_HM1375(0x0593, 0x05);  
    i2cWrite_HM1375(0x0594, 0x04);  
    i2cWrite_HM1375(0x0595, 0x06);  

    //----------------------------------------------
    //Y_Sharpness strength
    //----------------------------------------------

    i2cWrite_HM1375(0x05B0, 0x04);    
    i2cWrite_HM1375(0x05B1, 0x00);    


    //----------------------------------------------
    //WINDOW_SCALER
    //----------------------------------------------

    i2cWrite_HM1375(0x05E4, 0x08);  
    i2cWrite_HM1375(0x05E5, 0x00);  
    i2cWrite_HM1375(0x05E6, 0x07);  
    i2cWrite_HM1375(0x05E7, 0x05);  
    i2cWrite_HM1375(0x05E8, 0x0A);  
    i2cWrite_HM1375(0x05E9, 0x00);  
    i2cWrite_HM1375(0x05EA, 0xD9);  
    i2cWrite_HM1375(0x05EB, 0x02);  

    //----------------------------------------------
    //FLEXI ENGINE_AE ADJUST CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0666, 0x02);        
    i2cWrite_HM1375(0x0667, 0xE0);      
    i2cWrite_HM1375(0x067F, 0x19);      
    i2cWrite_HM1375(0x067C, 0x00);  
    i2cWrite_HM1375(0x067D, 0x00);  
    i2cWrite_HM1375(0x0682, 0x00);  
    i2cWrite_HM1375(0x0683, 0x00);      
    i2cWrite_HM1375(0x0688, 0x00);  
    i2cWrite_HM1375(0x0689, 0x00);  
    i2cWrite_HM1375(0x068E, 0x00);  
    i2cWrite_HM1375(0x068F, 0x00);  
    i2cWrite_HM1375(0x0695, 0x00);   
    i2cWrite_HM1375(0x0694, 0x00);      
    i2cWrite_HM1375(0x0697, 0x19);      
    i2cWrite_HM1375(0x069B, 0x00);  
    i2cWrite_HM1375(0x069C, 0x30);  // max EDR ratio   
    i2cWrite_HM1375(0x0720, 0x00);  
    i2cWrite_HM1375(0x0725, 0x6A);      
    i2cWrite_HM1375(0x0726, 0x03);  
    i2cWrite_HM1375(0x072B, 0x64);  
    i2cWrite_HM1375(0x072C, 0x64);  
    i2cWrite_HM1375(0x072D, 0x20);  
    i2cWrite_HM1375(0x072E, 0x82);  //turn off night mode
    i2cWrite_HM1375(0x072F, 0x08);      
    i2cWrite_HM1375(0x0800, 0x16);  
    i2cWrite_HM1375(0x0801, 0x30);    
    i2cWrite_HM1375(0x0802, 0x00);  
    i2cWrite_HM1375(0x0803, 0x68);  
    i2cWrite_HM1375(0x0804, 0x01);  
    i2cWrite_HM1375(0x0805, 0x28);  
    i2cWrite_HM1375(0x0806, 0x10);   
    i2cWrite_HM1375(0x0808, 0x1D);  
    i2cWrite_HM1375(0x0809, 0x18);  
    i2cWrite_HM1375(0x080A, 0x10);       
    i2cWrite_HM1375(0x080B, 0x07);       
    i2cWrite_HM1375(0x080D, 0x0F);  
    i2cWrite_HM1375(0x080E, 0x0F);    
    i2cWrite_HM1375(0x0810, 0x00);  
    i2cWrite_HM1375(0x0811, 0x08);       
    i2cWrite_HM1375(0x0812, 0x20);  
    i2cWrite_HM1375(0x0857, 0x0A);  
    i2cWrite_HM1375(0x0858, 0x30);  
    i2cWrite_HM1375(0x0859, 0x01);  
#if 0   // 15fps
    i2cWrite_HM1375(0x085A, 0x06);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x80);  
#else   // 30fps
    i2cWrite_HM1375(0x085A, 0x03);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x40);  
#endif
    i2cWrite_HM1375(0x085C, 0x03);  
    i2cWrite_HM1375(0x085D, 0x7F);  
    i2cWrite_HM1375(0x085E, 0x02);  //(Long)Max INTG  
    i2cWrite_HM1375(0x085F, 0xD0);  
    i2cWrite_HM1375(0x0860, 0x03);      
    i2cWrite_HM1375(0x0861, 0x7F);  
    i2cWrite_HM1375(0x0862, 0x02);  //(short)Max INTG     
    i2cWrite_HM1375(0x0863, 0xD0);  
    i2cWrite_HM1375(0x0864, 0x00);  //(short)Max AG   
    i2cWrite_HM1375(0x0865, 0x7F);  
    i2cWrite_HM1375(0x0866, 0x01);  
    i2cWrite_HM1375(0x0867, 0x00);  
    i2cWrite_HM1375(0x0868, 0x40);  
    i2cWrite_HM1375(0x0869, 0x01);  
    i2cWrite_HM1375(0x086A, 0x00);  
    i2cWrite_HM1375(0x086B, 0x40);  
    i2cWrite_HM1375(0x086C, 0x01);  
    i2cWrite_HM1375(0x086D, 0x00);  
    i2cWrite_HM1375(0x086E, 0x40);  
    i2cWrite_HM1375(0x0870, 0x00);  
    i2cWrite_HM1375(0x0871, 0x14);  
    i2cWrite_HM1375(0x0872, 0x01);  
    i2cWrite_HM1375(0x0873, 0x20);  
    i2cWrite_HM1375(0x0874, 0x00);  
    i2cWrite_HM1375(0x0875, 0x14);  
    i2cWrite_HM1375(0x0876, 0x00);  
    i2cWrite_HM1375(0x0877, 0xEC);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MAXIMUM EDR 
    //----------------------------------------------

    i2cWrite_HM1375(0x0815, 0x00);  
    i2cWrite_HM1375(0x0816, 0x4C);  
    i2cWrite_HM1375(0x0817, 0x00);  
    i2cWrite_HM1375(0x0818, 0x7B);  
    i2cWrite_HM1375(0x0819, 0x00);  
    i2cWrite_HM1375(0x081A, 0xCA);  
    i2cWrite_HM1375(0x081B, 0x01);  
    i2cWrite_HM1375(0x081C, 0x3E);  
    i2cWrite_HM1375(0x081D, 0x01);  
    i2cWrite_HM1375(0x081E, 0x77);  
    i2cWrite_HM1375(0x081F, 0x01);  
    i2cWrite_HM1375(0x0820, 0xAA);  
    i2cWrite_HM1375(0x0821, 0x01);  
    i2cWrite_HM1375(0x0822, 0xCE);  
    i2cWrite_HM1375(0x0823, 0x01);  
    i2cWrite_HM1375(0x0824, 0xEE);  
    i2cWrite_HM1375(0x0825, 0x02);  
    i2cWrite_HM1375(0x0826, 0x16);  
    i2cWrite_HM1375(0x0827, 0x02);  
    i2cWrite_HM1375(0x0828, 0x33);  
    i2cWrite_HM1375(0x0829, 0x02);  
    i2cWrite_HM1375(0x082A, 0x65);  
    i2cWrite_HM1375(0x082B, 0x02);  
    i2cWrite_HM1375(0x082C, 0x91);  
    i2cWrite_HM1375(0x082D, 0x02);  
    i2cWrite_HM1375(0x082E, 0xDC);  
    i2cWrite_HM1375(0x082F, 0x03);  
    i2cWrite_HM1375(0x0830, 0x28);  
    i2cWrite_HM1375(0x0831, 0x03);  
    i2cWrite_HM1375(0x0832, 0x74);  
    i2cWrite_HM1375(0x0833, 0x03);  
    i2cWrite_HM1375(0x0834, 0xFF);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MINIMUM EDR
    //----------------------------------------------

    i2cWrite_HM1375(0x0882, 0x00);  
    i2cWrite_HM1375(0x0883, 0x3E);  
    i2cWrite_HM1375(0x0884, 0x00);  
    i2cWrite_HM1375(0x0885, 0x70);  
    i2cWrite_HM1375(0x0886, 0x00);  
    i2cWrite_HM1375(0x0887, 0xB8);  
    i2cWrite_HM1375(0x0888, 0x01);  
    i2cWrite_HM1375(0x0889, 0x28);  
    i2cWrite_HM1375(0x088A, 0x01);  
    i2cWrite_HM1375(0x088B, 0x5B);  
    i2cWrite_HM1375(0x088C, 0x01);  
    i2cWrite_HM1375(0x088D, 0x8A);  
    i2cWrite_HM1375(0x088E, 0x01);  
    i2cWrite_HM1375(0x088F, 0xB1);  
    i2cWrite_HM1375(0x0890, 0x01);  
    i2cWrite_HM1375(0x0891, 0xD9);  
    i2cWrite_HM1375(0x0892, 0x01);  
    i2cWrite_HM1375(0x0893, 0xEE);  
    i2cWrite_HM1375(0x0894, 0x02);  
    i2cWrite_HM1375(0x0895, 0x0F);  
    i2cWrite_HM1375(0x0896, 0x02);  
    i2cWrite_HM1375(0x0897, 0x4C);  
    i2cWrite_HM1375(0x0898, 0x02);  
    i2cWrite_HM1375(0x0899, 0x74);  
    i2cWrite_HM1375(0x089A, 0x02);  
    i2cWrite_HM1375(0x089B, 0xC3);  
    i2cWrite_HM1375(0x089C, 0x03);  
    i2cWrite_HM1375(0x089D, 0x0F);  
    i2cWrite_HM1375(0x089E, 0x03);  
    i2cWrite_HM1375(0x089F, 0x57);  
    i2cWrite_HM1375(0x08A0, 0x03);  
    i2cWrite_HM1375(0x08A1, 0xFF);  

    //----------------------------------------------
    //COMMAND UPDATE_TRIGGER
    //----------------------------------------------
    //OSTimeDly(4);
    i2cWrite_HM1375(0x0100, 0x01);  // CMU AE
    i2cWrite_HM1375(0x0101, 0x01);  // CMU AWB
    i2cWrite_HM1375(0x0000, 0x01);  // CMU
    i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger

}

void SetHM1375_MIRROR_FLIP(u8 Level)
{
    u8 data=0;
    DEBUG_SIU("SetHM1375_MIRROR_FLIP() %d\n", Level);
    
    i2cRead_HM1375(0x0006, &data);
    if (Level == GPIO_MIRROR_FLIP_OFF)
    {
        data &=~0x03; 
        i2cWrite_HM1375(0x0006, data);
    }
    else
    {        
        data |= 0x03;
        i2cWrite_HM1375(0x0006, data);
    }
}
void SetHM1375_MIRROR_FLIP_1(u8 flip, u8 mirror)
{
    u8 data=0;
    DEBUG_SIU("SetHM1375_MIRROR_FLIP_1() %d %d \n", flip,mirror);
    
    i2cRead_HM1375(0x0006, &data);
    if (flip == GPIO_MIRROR_FLIP_OFF)
    {
        data &=~0x01; 
        i2cWrite_HM1375(0x0006, data);
    }
    else
    {        
        data |= 0x01;
        i2cWrite_HM1375(0x0006, data);
    }
    
    if (mirror == GPIO_MIRROR_FLIP_OFF)
    {
        data &=~0x02; 
        i2cWrite_HM1375(0x0006, data);
    }
    else
    {        
        data |= 0x02;
        i2cWrite_HM1375(0x0006, data);
    }
}

void SetHM1375_VGA_30FPS(void)
{
    int i;
	u8	data;
    int count;

    
    DEBUG_SIU("SetHM1375_VGA_30FPS()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);

        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    i2cWrite_HM1375(0x0022, 0x01);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);
    i2cWrite_HM1375(0x0022, 0x00);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);

    i2cWrite_HM1375(0x0004, 0x18);  // hw reset mode
    
    i2cWrite_HM1375(0x000C, 0x04);  // MODE      _Reserved bit
    i2cWrite_HM1375(0x0006, 0x0C);
    i2cWrite_HM1375(0x000A, 0x20);  // CHIPCFG _Full frame, chip default 00
    i2cWrite_HM1375(0x000F, 0x10);  // IMGCFG
    i2cWrite_HM1375(0x0012, 0x01);  // RESERVED 
    i2cWrite_HM1375(0x0013, 0x02);  // RESERVED
    i2cWrite_HM1375(0x0015, 0x01);  // INTG_H    _Set up integration
    i2cWrite_HM1375(0x0016, 0x00);  // INTG_L    _Set up integration
    i2cWrite_HM1375(0x0018, 0x00);  // AGAIN     _Set up coarse gain
    i2cWrite_HM1375(0x001D, 0x40);  // DGGAIN    _Set up fine gain
    i2cWrite_HM1375(0x0020, 0x10);  // OPRTCFG   _AE Gain enabled, Pclk transition on falling edge
#if (HW_BOARD_OPTION == MR8211_ZINWELL)
    i2cWrite_HM1375(0x0023, 0x00);  // IOCNTR    _IO pad drive strength, IO & PCLK pad 電流都降到最低 for EMI testing
#else
    i2cWrite_HM1375(0x0023, 0x43);  // IOCNTR    _IO pad drive strength
#endif
    i2cWrite_HM1375(0x0024, 0x20);  // CKCNTR    _Reserved bit
//#if (HM1375_FRAME_RATE == 30)
#if 1
    i2cWrite_HM1375(0x0025, 0x00);  // CKCFG     _Select PLL clock -> 24MHz to 96 MHz
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0025, 0x01);  // CKCFG     _Select PLL clock -> 24MHz to 48 MHz
#endif
    i2cWrite_HM1375(0x0026, 0x6F);
    //i2cWrite_HM1375(0x0027, 0x30);  // OPORTCNTR _YUV output, ITU601 Disable, YCbYCr...
    i2cWrite_HM1375(0x0027, 0x10);  // OPORTCNTR _YUV output, ITU601 Disable, CbYCrY...
    i2cWrite_HM1375(0x0028, 0x01);  // CKMUL     _Reserved bit
    i2cWrite_HM1375(0x002E, 0x0E);  // ExtraRawSel	_sharpness
    i2cWrite_HM1375(0x0030, 0x00);  // EDRCFG    _Disable EDR mode
    i2cWrite_HM1375(0x0034, 0x0E);  // EDRBLEND  _Set EDR blend to 0.875
    i2cWrite_HM1375(0x0035, 0x01);  // RESERVED
    i2cWrite_HM1375(0x0036, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0038, 0x02);  // EDR2AGAIN _Set up EDR2 coarse gain
    i2cWrite_HM1375(0x0039, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003A, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003B, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003C, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003D, 0x40);  // EDR2DGAIN _Set up EDR2 fine gain
    i2cWrite_HM1375(0x003F, 0x14);  // RESERVED      less gradient effect

    //----------------------------------------------
    //BLACK LEVEL CONTROL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0040, 0x10);  // BLCTGT    _Black level target
    i2cWrite_HM1375(0x0044, 0x07);  // BLCCFG    _BLC configuration enable, reserved bit

    //----------------------------------------------
    //RESERVED REGISTERS FOR SENSOR READOUT_TIMING
    //----------------------------------------------

    i2cWrite_HM1375(0x0045, 0x25);  // RESERVED       
    i2cWrite_HM1375(0x0048, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x004E, 0xFF);  // RESERVED 
    i2cWrite_HM1375(0x0070, 0x00);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0071, 0x4F);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0072, 0x00);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0073, 0x30);  // RESERVED    - 0920 updated by Willie
    i2cWrite_HM1375(0x0074, 0x13);  // RESERVED
    i2cWrite_HM1375(0x0075, 0x40);  // RESERVED
    i2cWrite_HM1375(0x0076, 0x24);  // RESERVED
    i2cWrite_HM1375(0x0078, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x007A, 0x05);  // RESERVED
    i2cWrite_HM1375(0x007B, 0xF2);  // RESERVED
    i2cWrite_HM1375(0x007C, 0x10);  // RESERVED
    i2cWrite_HM1375(0x0080, 0xC9);  // RESERVED
    i2cWrite_HM1375(0x0081, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0082, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0083, 0xB0);  // RESERVED
    i2cWrite_HM1375(0x0084, 0x70);  // RESERVED
    i2cWrite_HM1375(0x0086, 0x3E);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0087, 0x70);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0088, 0x11);  // RESERVED
    i2cWrite_HM1375(0x0089, 0x3C);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x008A, 0x87);  // RESERVED   // Rev.E updated by Willie
    i2cWrite_HM1375(0x008D, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0090, 0x07);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0091, 0x09);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0092, 0x0C);  
    i2cWrite_HM1375(0x0093, 0x0C);  
    i2cWrite_HM1375(0x0094, 0x0C);  
    i2cWrite_HM1375(0x0095, 0x0C);  
    i2cWrite_HM1375(0x0096, 0x01);  // AGAIN gain (nonEDR) table update for CFPN improvement 0824
    i2cWrite_HM1375(0x0097, 0x00);  
    i2cWrite_HM1375(0x0098, 0x04);  
    i2cWrite_HM1375(0x0099, 0x08);  
    i2cWrite_HM1375(0x009A, 0x0C);  


    //----------------------------------------------
    //IMAGE PIPELINE PROCESSING CONTROL
    //----------------------------------------------

    if(AE_Flicker_50_60_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
    i2cWrite_HM1375(0x0121, 0x81);  // IPPCNTR2
    i2cWrite_HM1375(0x0122, 0xEB);  // IPPCNTR3
    i2cWrite_HM1375(0x0123, 0x29);  // IPPCNTR4
    i2cWrite_HM1375(0x0124, 0x50);  // CCMCNTR
    i2cWrite_HM1375(0x0125, 0xDE);  // IPPCNTR5
    i2cWrite_HM1375(0x0126, 0xB1);  // IPPCNTR6

    //----------------------------------------------
    //FLARE CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x013D, 0x0F);  
    i2cWrite_HM1375(0x013E, 0x0F);  
    i2cWrite_HM1375(0x013F, 0x0F);  

    //----------------------------------------------
    //BAD PIXEL CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0140, 0x14); 
    i2cWrite_HM1375(0x0141, 0x0A);
    i2cWrite_HM1375(0x0142, 0x14);
    i2cWrite_HM1375(0x0143, 0x0A);

    //----------------------------------------------
    //RESERVED
    //----------------------------------------------

    i2cWrite_HM1375(0x0144, 0x08);  // RESERVED
    i2cWrite_HM1375(0x0145, 0x04);  // RESERVED
    i2cWrite_HM1375(0x0146, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0147, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x0148, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0149, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x014A, 0x96);  // RESERVED
    i2cWrite_HM1375(0x014B, 0xC8);  // RESERVED

    //----------------------------------------------
    //SHARPENING CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0150, 0x14);  // SHPTHLR   
    i2cWrite_HM1375(0x0151, 0x30);  // SHPTHLR_A 
    i2cWrite_HM1375(0x0152, 0x54);  // SHPTHHR   
    i2cWrite_HM1375(0x0153, 0x70);  // SHPTHHR_A 
    i2cWrite_HM1375(0x0154, 0x14);  // SHPTHLG   
    i2cWrite_HM1375(0x0155, 0x30);  // SHPTHLG_A 
    i2cWrite_HM1375(0x0156, 0x54);  // SHPTHHG   
    i2cWrite_HM1375(0x0157, 0x70);  // SHPTHHG_A 
    i2cWrite_HM1375(0x0158, 0x14);  // SHPTHLB   
    i2cWrite_HM1375(0x0159, 0x30);  // SHPTHLB_A 
    i2cWrite_HM1375(0x015A, 0x54);  // SHPTHHB   
    i2cWrite_HM1375(0x015B, 0x70);  // SHPTHHB_A 
    i2cWrite_HM1375(0x015C, 0x30);  // SHPSTR    _Sharpness strength
    i2cWrite_HM1375(0x015D, 0x00);  // SHPSTR_A  _sharpness strength_Alpha0


    //----------------------------------------------
    // NOISE FILTER CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x01D8, 0x20);  // NFHTHG
    i2cWrite_HM1375(0x01D9, 0x08);  // NFLTHG
    i2cWrite_HM1375(0x01DA, 0x20);  // NFHTHB
    i2cWrite_HM1375(0x01DB, 0x08);  // NFLTHB
    i2cWrite_HM1375(0x01DC, 0x20);  // NFHTHR
    i2cWrite_HM1375(0x01DD, 0x08);  // NFLTHR
    i2cWrite_HM1375(0x01DE, 0x20);  // NFHTHG_A
    i2cWrite_HM1375(0x01DF, 0x08);  // NFLTHG_A
    i2cWrite_HM1375(0x01E0, 0x20);  // NFHTHB_A
    i2cWrite_HM1375(0x01E1, 0x08);  // NFLTHB_A
    i2cWrite_HM1375(0x01E2, 0xFF);  // NFHTHR_A
    i2cWrite_HM1375(0x01E3, 0x00);  // NFLTHR_A
    i2cWrite_HM1375(0x01E4, 0x10);  // NFSTR
    i2cWrite_HM1375(0x01E5, 0x10);  // NFSTR_A 
    i2cWrite_HM1375(0x01E6, 0x02);  // NFSTR_OUTA
    i2cWrite_HM1375(0x01E7, 0x10);  // NFMTHG   
    i2cWrite_HM1375(0x01E8, 0x10);  // NFMTHB  
    i2cWrite_HM1375(0x01E9, 0x10);  // NFMTHR
    i2cWrite_HM1375(0x01EA, 0x10);  // NFMTHR_A
    i2cWrite_HM1375(0x01EC, 0xFA);  // NFMTHR_A
    i2cWrite_HM1375(0x01EB, 0x10);  // NFMTHR_A

    //----------------------------------------------
    //LENS SHADING CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0220, 0xF0);
    i2cWrite_HM1375(0x0221, 0xA0);
    i2cWrite_HM1375(0x0222, 0x00);
    i2cWrite_HM1375(0x0223, 0x80);
    i2cWrite_HM1375(0x0224, 0x80);
    i2cWrite_HM1375(0x0225, 0x00);
    i2cWrite_HM1375(0x0226, 0x80);
    i2cWrite_HM1375(0x0227, 0x80);
    i2cWrite_HM1375(0x0228, 0x00);
    i2cWrite_HM1375(0x0229, 0x80);
    i2cWrite_HM1375(0x022A, 0x80);
    i2cWrite_HM1375(0x022B, 0x00);
    i2cWrite_HM1375(0x022C, 0x80);
    i2cWrite_HM1375(0x022D, 0x12);
    i2cWrite_HM1375(0x022E, 0x10);
    i2cWrite_HM1375(0x022F, 0x12);
    i2cWrite_HM1375(0x0230, 0x10);
    i2cWrite_HM1375(0x0231, 0x12);
    i2cWrite_HM1375(0x0232, 0x10);
    i2cWrite_HM1375(0x0233, 0x12);
    i2cWrite_HM1375(0x0234, 0x10);
    i2cWrite_HM1375(0x0235, 0x80);
    i2cWrite_HM1375(0x0236, 0x02);
    i2cWrite_HM1375(0x0237, 0x80);
    i2cWrite_HM1375(0x0238, 0x02);
    i2cWrite_HM1375(0x0239, 0x80);
    i2cWrite_HM1375(0x023A, 0x02);
    i2cWrite_HM1375(0x023B, 0x80);
    i2cWrite_HM1375(0x023C, 0x02);
    i2cWrite_HM1375(0x023D, 0x00);
    i2cWrite_HM1375(0x023E, 0x02);
    i2cWrite_HM1375(0x023F, 0x00);
    i2cWrite_HM1375(0x0240, 0x02);
    i2cWrite_HM1375(0x0241, 0x00);
    i2cWrite_HM1375(0x0242, 0x02);
    i2cWrite_HM1375(0x0243, 0x00);
    i2cWrite_HM1375(0x0244, 0x02);
    i2cWrite_HM1375(0x0251, 0x10);

    //----------------------------------------------
    //GAMMA CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0280, 0x00);  // normal Gamma
    i2cWrite_HM1375(0x0281, 0x41);  
    i2cWrite_HM1375(0x0282, 0x00);  
    i2cWrite_HM1375(0x0283, 0x6D);  
    i2cWrite_HM1375(0x0284, 0x00);  
    i2cWrite_HM1375(0x0285, 0xBC);  
    i2cWrite_HM1375(0x0286, 0x01);  
    i2cWrite_HM1375(0x0287, 0x45);  
    i2cWrite_HM1375(0x0288, 0x01);  
    i2cWrite_HM1375(0x0289, 0x7B);  
    i2cWrite_HM1375(0x028A, 0x01);  
    i2cWrite_HM1375(0x028B, 0xAC);  
    i2cWrite_HM1375(0x028C, 0x01);  
    i2cWrite_HM1375(0x028D, 0xD2);  
    i2cWrite_HM1375(0x028E, 0x01);  
    i2cWrite_HM1375(0x028F, 0xF6);  
    i2cWrite_HM1375(0x0290, 0x02);  
    i2cWrite_HM1375(0x0291, 0x16);  
    i2cWrite_HM1375(0x0292, 0x02);  
    i2cWrite_HM1375(0x0293, 0x35);  
    i2cWrite_HM1375(0x0294, 0x02);  
    i2cWrite_HM1375(0x0295, 0x6E);  
    i2cWrite_HM1375(0x0296, 0x02);  
    i2cWrite_HM1375(0x0297, 0xA2);  
    i2cWrite_HM1375(0x0298, 0x02);  
    i2cWrite_HM1375(0x0299, 0xFF);  
    i2cWrite_HM1375(0x029A, 0x03);  
    i2cWrite_HM1375(0x029B, 0x51);  
    i2cWrite_HM1375(0x029C, 0x03);  
    i2cWrite_HM1375(0x029D, 0x9B);  // updated by MH 06/17
    i2cWrite_HM1375(0x029E, 0x00);  // slope high byte    
    i2cWrite_HM1375(0x029F, 0x85);  // slope low byte
    i2cWrite_HM1375(0x02A0, 0x04);  // GAM_A

    //----------------------------------------------
    //COLOR CORRECTION MATRIX
    //----------------------------------------------

    i2cWrite_HM1375(0x02C0, 0x80);  //CCM00_L    _Adjust for D65 color temperature
    i2cWrite_HM1375(0x02C1, 0x01);  //CCM00_H
    i2cWrite_HM1375(0x02C2, 0x71);  
    i2cWrite_HM1375(0x02C3, 0x04);  
    i2cWrite_HM1375(0x02C4, 0x0F);  
    i2cWrite_HM1375(0x02C5, 0x04);  
    i2cWrite_HM1375(0x02C6, 0x3D);  
    i2cWrite_HM1375(0x02C7, 0x04);  
    i2cWrite_HM1375(0x02C8, 0x94);  
    i2cWrite_HM1375(0x02C9, 0x01);  
    i2cWrite_HM1375(0x02CA, 0x57);  
    i2cWrite_HM1375(0x02CB, 0x04);  
    i2cWrite_HM1375(0x02CC, 0x0F);  
    i2cWrite_HM1375(0x02CD, 0x04);  
    i2cWrite_HM1375(0x02CE, 0x8F);  
    i2cWrite_HM1375(0x02CF, 0x04);  
    i2cWrite_HM1375(0x02D0, 0x9E);  
    i2cWrite_HM1375(0x02D1, 0x01);  
    i2cWrite_HM1375(0x02E0, 0x06);  // CCM_A     _reduce to 0.267x
    i2cWrite_HM1375(0x02E1, 0xC0);  // RESERVED
    i2cWrite_HM1375(0x02E2, 0xE0);  // RESERVED
    i2cWrite_HM1375(0x02F0, 0x48);  // ACCM00_L  _Adjust for IncA color temperature
    i2cWrite_HM1375(0x02F1, 0x01);  // ACCM00_H
    i2cWrite_HM1375(0x02F2, 0x32);  
    i2cWrite_HM1375(0x02F3, 0x04);  
    i2cWrite_HM1375(0x02F4, 0x16);  
    i2cWrite_HM1375(0x02F5, 0x04);  
    i2cWrite_HM1375(0x02F6, 0x52);  
    i2cWrite_HM1375(0x02F7, 0x04);  
    i2cWrite_HM1375(0x02F8, 0xAA);  
    i2cWrite_HM1375(0x02F9, 0x01);  
    i2cWrite_HM1375(0x02FA, 0x58);  
    i2cWrite_HM1375(0x02FB, 0x04);  
    i2cWrite_HM1375(0x02FC, 0x56);  
    i2cWrite_HM1375(0x02FD, 0x04);  
    i2cWrite_HM1375(0x02FE, 0xDD);  
    i2cWrite_HM1375(0x02FF, 0x04);  
    i2cWrite_HM1375(0x0300, 0x33);  
    i2cWrite_HM1375(0x0301, 0x02);  

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE WINDOW CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0324, 0x00);
    i2cWrite_HM1375(0x0325, 0x01);

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE DETECTION AND LIMITS
    //----------------------------------------------

    i2cWrite_HM1375(0x0333, 0x86);   
    i2cWrite_HM1375(0x0334, 0x00);  
    i2cWrite_HM1375(0x0335, 0x86);  
    i2cWrite_HM1375(0x0340, 0x40);      
    i2cWrite_HM1375(0x0341, 0x44);  
    i2cWrite_HM1375(0x0342, 0x4A);  
    i2cWrite_HM1375(0x0343, 0x2B);  
    i2cWrite_HM1375(0x0344, 0x94);  
    i2cWrite_HM1375(0x0345, 0x3F);  
    i2cWrite_HM1375(0x0346, 0x8E);  
    i2cWrite_HM1375(0x0347, 0x51);  
    i2cWrite_HM1375(0x0348, 0x75);  
    i2cWrite_HM1375(0x0349, 0x5C);  
    i2cWrite_HM1375(0x034A, 0x6A);  
    i2cWrite_HM1375(0x034B, 0x68);  
    i2cWrite_HM1375(0x034C, 0x5E);  
    i2cWrite_HM1375(0x0350, 0x7C);  
    i2cWrite_HM1375(0x0351, 0x78);  
    i2cWrite_HM1375(0x0352, 0x08);  
    i2cWrite_HM1375(0x0353, 0x04);  
    i2cWrite_HM1375(0x0354, 0x80);  
    i2cWrite_HM1375(0x0355, 0x9A);  
    i2cWrite_HM1375(0x0356, 0xCC);  
    i2cWrite_HM1375(0x0357, 0xFF);  
    i2cWrite_HM1375(0x0358, 0xFF);  
    i2cWrite_HM1375(0x035A, 0xFF);  
    i2cWrite_HM1375(0x035B, 0x00);  
    i2cWrite_HM1375(0x035C, 0x70);  
    i2cWrite_HM1375(0x035D, 0x80);  
    i2cWrite_HM1375(0x035F, 0xA0);  
    i2cWrite_HM1375(0x0488, 0x30);  
    i2cWrite_HM1375(0x0360, 0xDF);   
    i2cWrite_HM1375(0x0361, 0x00);  
    i2cWrite_HM1375(0x0362, 0xFF);  
    i2cWrite_HM1375(0x0363, 0x03);  
    i2cWrite_HM1375(0x0364, 0xFF);  
    i2cWrite_HM1375(0x037B, 0x11);  //whole nonEDR to EDR ratio 
    i2cWrite_HM1375(0x037C, 0x1E);  //EDR LONG litbin pop out to nonEDR 

    //----------------------------------------------
    //AUTOMATIC EXPOSURE CONFIGURATION
    //----------------------------------------------

    i2cWrite_HM1375(0x0380, 0xFF);       
    i2cWrite_HM1375(0x0383, 0x50);  // RESERVED       
    i2cWrite_HM1375(0x038A, 0x64);   
    i2cWrite_HM1375(0x038B, 0x64);   
#if (((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT != 7)&&(UI_PROJ_OPT != 4)&&(UI_PROJ_OPT != 2)&&(UI_PROJ_OPT != 5)&&(UI_PROJ_OPT != 8))) ||\
    (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652) ||\
    (HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
    (HW_BOARD_OPTION  == MR8120S_TX_GCT_VM00)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)||\
    (HW_BOARD_OPTION == MR8120_TX_GCT_VM00))
    i2cWrite_HM1375(0x038E, AETargetMeanTab_VGA[siuY_TargetIndex]);  
    i2cWrite_HM1375(0x0381, AETargetMeanTab_VGA[siuY_TargetIndex]+12);
    i2cWrite_HM1375(0x0382, AETargetMeanTab_VGA[siuY_TargetIndex]-12);
#elif((HW_BOARD_OPTION == MR8120_TX_RDI) || ((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT == 7)||(UI_PROJ_OPT == 4)||(UI_PROJ_OPT == 2) ||(UI_PROJ_OPT == 5) || (UI_PROJ_OPT == 8)))||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA532)|| (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
    i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex] + 5);  
    i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+17);
    i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-7);
#else
    i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);  
    i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
    i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
#endif
    i2cWrite_HM1375(0x0391, 0x2A);  // RESERVED        
    i2cWrite_HM1375(0x0393, 0x1E);  // RESERVED       
    i2cWrite_HM1375(0x0394, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0395, 0x23);         
    i2cWrite_HM1375(0x0398, 0x03);  // RESERVED
    i2cWrite_HM1375(0x0399, 0x45);  // RESERVED
    i2cWrite_HM1375(0x039A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x039B, 0x8B);  // RESERVED
    i2cWrite_HM1375(0x039C, 0x0D);  // RESERVED
    i2cWrite_HM1375(0x039D, 0x16);  // RESERVED
    i2cWrite_HM1375(0x039E, 0x0A);  // RESERVED
    i2cWrite_HM1375(0x039F, 0x10);       
    i2cWrite_HM1375(0x03A0, 0x10);          
    i2cWrite_HM1375(0x03A1, 0xE5);  // RESERVED   
    i2cWrite_HM1375(0x03A2, 0x06);  // RESERVED   
    i2cWrite_HM1375(0x03A4, 0x18);        
    i2cWrite_HM1375(0x03A5, 0x48);    
    i2cWrite_HM1375(0x03A6, 0x2D);  // RESERVED
    i2cWrite_HM1375(0x03A7, 0x78);  // RESERVED
    i2cWrite_HM1375(0x03AC, 0x5A);  // RESERVED
    i2cWrite_HM1375(0x03AD, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x03AE, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x03AF, 0x04);  // RESERVED
    i2cWrite_HM1375(0x03B0, 0x35);  // RESERVED
    i2cWrite_HM1375(0x03B1, 0x14);  // RESERVED
    i2cWrite_HM1375(0x036F, 0x04);  // Bayer denoise strength EDR
    i2cWrite_HM1375(0x0370, 0x0A);  // Bayer denoise strength nonEDR
    i2cWrite_HM1375(0x0371, 0x04);  // Bayer denoise strength EDR A0
    i2cWrite_HM1375(0x0372, 0x10);  // Bayer denoise strength nonEDR A0
    i2cWrite_HM1375(0x0373, 0x40);  // raw sharpness strength EDR
    i2cWrite_HM1375(0x0374, 0x20);  // raw sharpness strength nonEDR

    i2cWrite_HM1375(0x0375, 0x04);  // raw sharpness strength EDR A0
    i2cWrite_HM1375(0x0376, 0x00);  // raw sharpness strength nonEDR A0
    i2cWrite_HM1375(0x0377, 0x08);  // Y Denoise strength EDR
    i2cWrite_HM1375(0x0378, 0x08);  // Y Denoise strength nonEDR
    i2cWrite_HM1375(0x0379, 0x04);  // Y Denoise strength EDR A0
    i2cWrite_HM1375(0x037A, 0x08);  // Y Denoise strength nonEDR A0

    //----------------------------------------------
    //DIGITAL BLACK LEVEL OFFSET CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0420, 0x00);    
    i2cWrite_HM1375(0x0421, 0x00);    
    i2cWrite_HM1375(0x0422, 0x00);    
    i2cWrite_HM1375(0x0423, 0x84);    

    //----------------------------------------------
    //AUTO BLACK LEVEL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0430, 0x10);   
    i2cWrite_HM1375(0x0431, 0x60);   
    i2cWrite_HM1375(0x0432, 0x10);   
    i2cWrite_HM1375(0x0433, 0x20);   
    i2cWrite_HM1375(0x0434, 0x00);   
    i2cWrite_HM1375(0x0435, 0x30);   
    i2cWrite_HM1375(0x0436, 0x00);   

    //----------------------------------------------
    //LOWLIGHT_OUTDOOR IPP CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0450, 0xFD);   
    i2cWrite_HM1375(0x0451, 0xD8);   
    i2cWrite_HM1375(0x0452, 0xA0);   
    i2cWrite_HM1375(0x0453, 0x50);   
    i2cWrite_HM1375(0x0454, 0x00);       
    i2cWrite_HM1375(0x0459, 0x04);    
    i2cWrite_HM1375(0x045A, 0x00);   
    i2cWrite_HM1375(0x045B, 0x30);   
    i2cWrite_HM1375(0x045C, 0x01);   
    i2cWrite_HM1375(0x045D, 0x70);   
    i2cWrite_HM1375(0x0460, 0x00);    
    i2cWrite_HM1375(0x0461, 0x00);    
    i2cWrite_HM1375(0x0462, 0x00);    
    i2cWrite_HM1375(0x0465, 0x16);  // AWB IIR
    i2cWrite_HM1375(0x0466, 0x14);    
    i2cWrite_HM1375(0x0478, 0x00);   

    //----------------------------------------------
    //COLOR SPACE CONVERSION_SATURATION ADJ
    //----------------------------------------------

    i2cWrite_HM1375(0x0480, 0x60);

    i2cWrite_HM1375(0x0481, 0x06);  
    i2cWrite_HM1375(0x0482, 0x0C);  

    //----------------------------------------------
    //CONTRAST_BRIGHTNESS
    //----------------------------------------------


    i2cWrite_HM1375(0x04B0, 0x4C);  // Contrast 05032011 by mh
    i2cWrite_HM1375(0x04B1, 0x86);  // contrast 06/17 by mh
    i2cWrite_HM1375(0x04B2, 0x00);  //
    i2cWrite_HM1375(0x04B3, 0x18);  //
    i2cWrite_HM1375(0x04B4, 0x00);  //
    i2cWrite_HM1375(0x04B5, 0x00);  //
    i2cWrite_HM1375(0x04B6, 0x30);  //
    i2cWrite_HM1375(0x04B7, 0x00);  //
    i2cWrite_HM1375(0x04B8, 0x00);  //
    i2cWrite_HM1375(0x04B9, 0x10);  //
    i2cWrite_HM1375(0x04BA, 0x00);  //
    i2cWrite_HM1375(0x04BB, 0x00);  //
    i2cWrite_HM1375(0x04BD, 0x00);  //

    //----------------------------------------------
    //EDR CONTRAST
    //----------------------------------------------

    i2cWrite_HM1375(0x04D0, 0x56);     
    i2cWrite_HM1375(0x04D6, 0x30);  
    i2cWrite_HM1375(0x04DD, 0x10);  
    i2cWrite_HM1375(0x04D9, 0x16);  
    i2cWrite_HM1375(0x04D3, 0x18);  

    //----------------------------------------------
    //AE FLICKER STEP SIZE
    //----------------------------------------------

//#if (HM1375_FRAME_RATE == 30)
#if 1
    i2cWrite_HM1375(0x0540, 0x01);   
    i2cWrite_HM1375(0x0541, 0x06);   
    i2cWrite_HM1375(0x0542, 0x01);   
    i2cWrite_HM1375(0x0543, 0x3B);  
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0x83);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0x9D);  
#endif
    i2cWrite_HM1375(0x0580, 0x50);  // RESERVED
    i2cWrite_HM1375(0x0581, 0x30);  // RESERVED

    //----------------------------------------------
    //Y_COLOR NOISE REDUCTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0582, 0x2D);  
    i2cWrite_HM1375(0x0583, 0x16);  
    i2cWrite_HM1375(0x0584, 0x1E);  
    i2cWrite_HM1375(0x0585, 0x0F);  
    i2cWrite_HM1375(0x0586, 0x08);  
    i2cWrite_HM1375(0x0587, 0x10);  
    i2cWrite_HM1375(0x0590, 0x10);  
    i2cWrite_HM1375(0x0591, 0x10);  
    i2cWrite_HM1375(0x0592, 0x05);  
    i2cWrite_HM1375(0x0593, 0x05);  
    i2cWrite_HM1375(0x0594, 0x04);  
    i2cWrite_HM1375(0x0595, 0x06);  

    //----------------------------------------------
    //Y_Sharpness strength
    //----------------------------------------------

    i2cWrite_HM1375(0x05B0, 0x04);    
    i2cWrite_HM1375(0x05B1, 0x00);    


    //----------------------------------------------
    //WINDOW_SCALER
    //----------------------------------------------

    i2cWrite_HM1375(0x05E4, 0x02);  
    i2cWrite_HM1375(0x05E5, 0x00);  
    i2cWrite_HM1375(0x05E6, 0x81);  
    i2cWrite_HM1375(0x05E7, 0x02);  
    i2cWrite_HM1375(0x05E8, 0x09);  
    i2cWrite_HM1375(0x05E9, 0x00);  
    i2cWrite_HM1375(0x05EA, 0xE8);  
    i2cWrite_HM1375(0x05EB, 0x01);  

    //----------------------------------------------
    //FLEXI ENGINE_AE ADJUST CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0666, 0x02);        
    i2cWrite_HM1375(0x0667, 0xE0);      
    i2cWrite_HM1375(0x067F, 0x19);      
    i2cWrite_HM1375(0x067C, 0x00);  
    i2cWrite_HM1375(0x067D, 0x00);  
    i2cWrite_HM1375(0x0682, 0x00);  
    i2cWrite_HM1375(0x0683, 0x00);      
    i2cWrite_HM1375(0x0688, 0x00);  
    i2cWrite_HM1375(0x0689, 0x00);  
    i2cWrite_HM1375(0x068E, 0x00);  
    i2cWrite_HM1375(0x068F, 0x00);  
    i2cWrite_HM1375(0x0695, 0x00);   
    i2cWrite_HM1375(0x0694, 0x00);      
    i2cWrite_HM1375(0x0697, 0x19);      
    i2cWrite_HM1375(0x069B, 0x00);  
    i2cWrite_HM1375(0x069C, 0x20);  // max EDR ratio   
    i2cWrite_HM1375(0x0720, 0x00);  
    i2cWrite_HM1375(0x0725, 0x6A);      
    i2cWrite_HM1375(0x0726, 0x03);  
    i2cWrite_HM1375(0x072B, 0x64);  
    i2cWrite_HM1375(0x072C, 0x64);  
    i2cWrite_HM1375(0x072D, 0x20);  
    i2cWrite_HM1375(0x072E, 0x82);  //turn off night mode
    i2cWrite_HM1375(0x072F, 0x08);      
    i2cWrite_HM1375(0x0800, 0x16);  
    i2cWrite_HM1375(0x0801, 0x4F);    
    i2cWrite_HM1375(0x0802, 0x00);  
    i2cWrite_HM1375(0x0803, 0x68);  
    i2cWrite_HM1375(0x0804, 0x01);  
    i2cWrite_HM1375(0x0805, 0x28);  
    i2cWrite_HM1375(0x0806, 0x10);   
    i2cWrite_HM1375(0x0808, 0x1D);  
    i2cWrite_HM1375(0x0809, 0x18);  
    i2cWrite_HM1375(0x080A, 0x10);       
    i2cWrite_HM1375(0x080B, 0x07);       
    i2cWrite_HM1375(0x080D, 0x0F);  
    i2cWrite_HM1375(0x080E, 0x0F);    
    i2cWrite_HM1375(0x0810, 0x00);  
    i2cWrite_HM1375(0x0811, 0x08);       
    i2cWrite_HM1375(0x0812, 0x20);  
    i2cWrite_HM1375(0x0857, 0x0A);  
    i2cWrite_HM1375(0x0858, 0x04);  
    i2cWrite_HM1375(0x0859, 0x01);  
#if 0   // 15fps
    i2cWrite_HM1375(0x085A, 0x08);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x30);  
#else   // 30fps
    i2cWrite_HM1375(0x085A, 0x04);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x18);  
#endif
    i2cWrite_HM1375(0x085C, 0x03);  
    i2cWrite_HM1375(0x085D, 0x7F);  
    i2cWrite_HM1375(0x085E, 0x02);  //(Long)Max INTG  
    i2cWrite_HM1375(0x085F, 0xD0);  
    i2cWrite_HM1375(0x0860, 0x03);      
    i2cWrite_HM1375(0x0861, 0x7F);  
    i2cWrite_HM1375(0x0862, 0x02);  //(short)Max INTG     
    i2cWrite_HM1375(0x0863, 0xD0);  
    i2cWrite_HM1375(0x0864, 0x02);  //(short)Max AG   
    i2cWrite_HM1375(0x0865, 0x7F);  
    i2cWrite_HM1375(0x0866, 0x01);  
    i2cWrite_HM1375(0x0867, 0x00);  
    i2cWrite_HM1375(0x0868, 0x40);  
    i2cWrite_HM1375(0x0869, 0x01);  
    i2cWrite_HM1375(0x086A, 0x00);  
    i2cWrite_HM1375(0x086B, 0x40);  
    i2cWrite_HM1375(0x086C, 0x01);  
    i2cWrite_HM1375(0x086D, 0x00);  
    i2cWrite_HM1375(0x086E, 0x40);  
    i2cWrite_HM1375(0x0870, 0x00);  
    i2cWrite_HM1375(0x0871, 0x14);  
    i2cWrite_HM1375(0x0872, 0x01);  
    i2cWrite_HM1375(0x0873, 0x20);  
    i2cWrite_HM1375(0x0874, 0x00);  
    i2cWrite_HM1375(0x0875, 0x14);  
    i2cWrite_HM1375(0x0876, 0x00);  
    i2cWrite_HM1375(0x0877, 0xEC);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MAXIMUM EDR 
    //----------------------------------------------

    i2cWrite_HM1375(0x0815, 0x00);  
    i2cWrite_HM1375(0x0816, 0x4C);  
    i2cWrite_HM1375(0x0817, 0x00);  
    i2cWrite_HM1375(0x0818, 0x7B);  
    i2cWrite_HM1375(0x0819, 0x00);  
    i2cWrite_HM1375(0x081A, 0xCA);  
    i2cWrite_HM1375(0x081B, 0x01);  
    i2cWrite_HM1375(0x081C, 0x3E);  
    i2cWrite_HM1375(0x081D, 0x01);  
    i2cWrite_HM1375(0x081E, 0x77);  
    i2cWrite_HM1375(0x081F, 0x01);  
    i2cWrite_HM1375(0x0820, 0xAA);  
    i2cWrite_HM1375(0x0821, 0x01);  
    i2cWrite_HM1375(0x0822, 0xCE);  
    i2cWrite_HM1375(0x0823, 0x01);  
    i2cWrite_HM1375(0x0824, 0xEE);  
    i2cWrite_HM1375(0x0825, 0x02);  
    i2cWrite_HM1375(0x0826, 0x16);  
    i2cWrite_HM1375(0x0827, 0x02);  
    i2cWrite_HM1375(0x0828, 0x33);  
    i2cWrite_HM1375(0x0829, 0x02);  
    i2cWrite_HM1375(0x082A, 0x65);  
    i2cWrite_HM1375(0x082B, 0x02);  
    i2cWrite_HM1375(0x082C, 0x91);  
    i2cWrite_HM1375(0x082D, 0x02);  
    i2cWrite_HM1375(0x082E, 0xDC);  
    i2cWrite_HM1375(0x082F, 0x03);  
    i2cWrite_HM1375(0x0830, 0x28);  
    i2cWrite_HM1375(0x0831, 0x03);  
    i2cWrite_HM1375(0x0832, 0x74);  
    i2cWrite_HM1375(0x0833, 0x03);  
    i2cWrite_HM1375(0x0834, 0xFF);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MINIMUM EDR
    //----------------------------------------------

    i2cWrite_HM1375(0x0882, 0x00);  
    i2cWrite_HM1375(0x0883, 0x3E);  
    i2cWrite_HM1375(0x0884, 0x00);  
    i2cWrite_HM1375(0x0885, 0x70);  
    i2cWrite_HM1375(0x0886, 0x00);  
    i2cWrite_HM1375(0x0887, 0xB8);  
    i2cWrite_HM1375(0x0888, 0x01);  
    i2cWrite_HM1375(0x0889, 0x28);  
    i2cWrite_HM1375(0x088A, 0x01);  
    i2cWrite_HM1375(0x088B, 0x5B);  
    i2cWrite_HM1375(0x088C, 0x01);  
    i2cWrite_HM1375(0x088D, 0x8A);  
    i2cWrite_HM1375(0x088E, 0x01);  
    i2cWrite_HM1375(0x088F, 0xB1);  
    i2cWrite_HM1375(0x0890, 0x01);  
    i2cWrite_HM1375(0x0891, 0xD9);  
    i2cWrite_HM1375(0x0892, 0x01);  
    i2cWrite_HM1375(0x0893, 0xEE);  
    i2cWrite_HM1375(0x0894, 0x02);  
    i2cWrite_HM1375(0x0895, 0x0F);  
    i2cWrite_HM1375(0x0896, 0x02);  
    i2cWrite_HM1375(0x0897, 0x4C);  
    i2cWrite_HM1375(0x0898, 0x02);  
    i2cWrite_HM1375(0x0899, 0x74);  
    i2cWrite_HM1375(0x089A, 0x02);  
    i2cWrite_HM1375(0x089B, 0xC3);  
    i2cWrite_HM1375(0x089C, 0x03);  
    i2cWrite_HM1375(0x089D, 0x0F);  
    i2cWrite_HM1375(0x089E, 0x03);  
    i2cWrite_HM1375(0x089F, 0x57);  
    i2cWrite_HM1375(0x08A0, 0x03);  
    i2cWrite_HM1375(0x08A1, 0xFF);  

    //----------------------------------------------
    //COMMAND UPDATE_TRIGGER
    //----------------------------------------------
    //OSTimeDly(4);
    i2cWrite_HM1375(0x0100, 0x01);  // CMU AE
    i2cWrite_HM1375(0x0101, 0x01);  // CMU AWB
    i2cWrite_HM1375(0x0000, 0x01);  // CMU
    i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger

}

#if (HW_BOARD_OPTION == MR8120_TX_COMMAX)
void SetHM1375_VGA_30FPS_Commax(void)
{
    int i;
	u8	data;
    int count;

    
    DEBUG_SIU("SetHM1375_VGA_30FPS_Commax()\n");

    //----------------------------------------------
    //SENSOR INITIALIZATION
    //----------------------------------------------
    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);

        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    i2cWrite_HM1375(0x0022, 0x01);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);
    i2cWrite_HM1375(0x0022, 0x00);  // SFTSRT    _Soft Reset
    for(i=0;i<100;i++);

    i2cWrite_HM1375(0x0004, 0x18);  // hw reset mode
    
    i2cWrite_HM1375(0x000C, 0x04);  // MODE      _Reserved bit
    i2cWrite_HM1375(0x0006, 0x0C);
    //i2cWrite_HM1375(0x0006, 0x0F);  // Flip
    i2cWrite_HM1375(0x000A, 0x20);  // CHIPCFG _Full frame, chip default 00
    i2cWrite_HM1375(0x000F, 0x10);  // IMGCFG
    i2cWrite_HM1375(0x0012, 0x01);  // RESERVED 
    i2cWrite_HM1375(0x0013, 0x02);  // RESERVED
    i2cWrite_HM1375(0x0015, 0x01);  // INTG_H    _Set up integration
    i2cWrite_HM1375(0x0016, 0x00);  // INTG_L    _Set up integration
    i2cWrite_HM1375(0x0018, 0x00);  // AGAIN     _Set up coarse gain
    i2cWrite_HM1375(0x001D, 0x40);  // DGGAIN    _Set up fine gain
    i2cWrite_HM1375(0x0020, 0x10);  // OPRTCFG   _AE Gain enabled, Pclk transition on falling edge
#if (HW_BOARD_OPTION == MR8211_ZINWELL)
    i2cWrite_HM1375(0x0023, 0x00);  // IOCNTR    _IO pad drive strength, IO & PCLK pad 電流都降到最低 for EMI testing
#else
    i2cWrite_HM1375(0x0023, 0x43);  // IOCNTR    _IO pad drive strength
#endif
    i2cWrite_HM1375(0x0024, 0x20);  // CKCNTR    _Reserved bit
//#if (HM1375_FRAME_RATE == 30)
#if 1
    i2cWrite_HM1375(0x0025, 0x00);  // CKCFG     _Select PLL clock -> 24MHz to 96 MHz
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0025, 0x01);  // CKCFG     _Select PLL clock -> 24MHz to 48 MHz
#endif
    i2cWrite_HM1375(0x0026, 0x6F);
    //i2cWrite_HM1375(0x0027, 0x30);  // OPORTCNTR _YUV output, ITU601 Disable, YCbYCr...
    i2cWrite_HM1375(0x0027, 0x10);  // OPORTCNTR _YUV output, ITU601 Disable, CbYCrY...
    i2cWrite_HM1375(0x0028, 0x01);  // CKMUL     _Reserved bit
    i2cWrite_HM1375(0x002E, 0x0E);  // ExtraRawSel	_sharpness
    i2cWrite_HM1375(0x0030, 0x00);  // EDRCFG    _Disable EDR mode
    i2cWrite_HM1375(0x0034, 0x0E);  // EDRBLEND  _Set EDR blend to 0.875
    i2cWrite_HM1375(0x0035, 0x01);  // RESERVED
    i2cWrite_HM1375(0x0036, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0038, 0x02);  // EDR2AGAIN _Set up EDR2 coarse gain
    i2cWrite_HM1375(0x0039, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003A, 0x01);  // RESERVED
    i2cWrite_HM1375(0x003B, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003C, 0xFF);  // RESERVED
    i2cWrite_HM1375(0x003D, 0x40);  // EDR2DGAIN _Set up EDR2 fine gain
    i2cWrite_HM1375(0x003F, 0x14);  // RESERVED      less gradient effect

    //----------------------------------------------
    //BLACK LEVEL CONTROL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0040, 0x10);  // BLCTGT    _Black level target
    i2cWrite_HM1375(0x0044, 0x07);  // BLCCFG    _BLC configuration enable, reserved bit

    //----------------------------------------------
    //RESERVED REGISTERS FOR SENSOR READOUT_TIMING
    //----------------------------------------------

    i2cWrite_HM1375(0x0045, 0x25);  // RESERVED       
    i2cWrite_HM1375(0x0048, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x004E, 0xFF);  // RESERVED 
    i2cWrite_HM1375(0x0070, 0x00);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0071, 0x4F);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0072, 0x00);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0073, 0x30);  // RESERVED    - 0920 updated by Willie
    i2cWrite_HM1375(0x0074, 0x13);  // RESERVED
    i2cWrite_HM1375(0x0075, 0x40);  // RESERVED
    i2cWrite_HM1375(0x0076, 0x24);  // RESERVED
    i2cWrite_HM1375(0x0078, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x007A, 0x05);  // RESERVED
    i2cWrite_HM1375(0x007B, 0xF2);  // RESERVED
    i2cWrite_HM1375(0x007C, 0x10);  // RESERVED
    i2cWrite_HM1375(0x0080, 0xC9);  // RESERVED
    i2cWrite_HM1375(0x0081, 0x00);  // RESERVED
    i2cWrite_HM1375(0x0082, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0083, 0xB0);  // RESERVED
    i2cWrite_HM1375(0x0084, 0x70);  // RESERVED
    i2cWrite_HM1375(0x0086, 0x3E);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0087, 0x70);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x0088, 0x11);  // RESERVED
    i2cWrite_HM1375(0x0089, 0x3C);  // RESERVED    - 1024 updated by Willie 4 S-2
    i2cWrite_HM1375(0x008A, 0x87);  // RESERVED   // Rev.E updated by Willie
    i2cWrite_HM1375(0x008D, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0090, 0x07);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0091, 0x09);  // - 12152011 updated by Willie
    i2cWrite_HM1375(0x0092, 0x0C);  
    i2cWrite_HM1375(0x0093, 0x0C);  
    i2cWrite_HM1375(0x0094, 0x0C);  
    i2cWrite_HM1375(0x0095, 0x0C);  
    i2cWrite_HM1375(0x0096, 0x01);  // AGAIN gain (nonEDR) table update for CFPN improvement 0824
    i2cWrite_HM1375(0x0097, 0x00);  
    i2cWrite_HM1375(0x0098, 0x04);  
    i2cWrite_HM1375(0x0099, 0x08);  
    i2cWrite_HM1375(0x009A, 0x0C);  


    //----------------------------------------------
    //IMAGE PIPELINE PROCESSING CONTROL
    //----------------------------------------------

    if(AE_Flicker_50_60_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
    i2cWrite_HM1375(0x0121, 0x81);  // IPPCNTR2
    i2cWrite_HM1375(0x0122, 0xEB);  // IPPCNTR3
    i2cWrite_HM1375(0x0123, 0x29);  // IPPCNTR4
    i2cWrite_HM1375(0x0124, 0x50);  // CCMCNTR
    i2cWrite_HM1375(0x0125, 0xDE);  // IPPCNTR5
    i2cWrite_HM1375(0x0126, 0xB1);  // IPPCNTR6

    //----------------------------------------------
    //FLARE CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x013D, 0x0F);  
    i2cWrite_HM1375(0x013E, 0x0F);  
    i2cWrite_HM1375(0x013F, 0x0F);  

    //----------------------------------------------
    //BAD PIXEL CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0140, 0x14); 
    i2cWrite_HM1375(0x0141, 0x0A);
    i2cWrite_HM1375(0x0142, 0x14);
    i2cWrite_HM1375(0x0143, 0x0A);

    //----------------------------------------------
    //RESERVED
    //----------------------------------------------

    i2cWrite_HM1375(0x0144, 0x08);  // RESERVED
    i2cWrite_HM1375(0x0145, 0x04);  // RESERVED
    i2cWrite_HM1375(0x0146, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0147, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x0148, 0x28);  // RESERVED
    i2cWrite_HM1375(0x0149, 0x3C);  // RESERVED
    i2cWrite_HM1375(0x014A, 0x96);  // RESERVED
    i2cWrite_HM1375(0x014B, 0xC8);  // RESERVED

    //----------------------------------------------
    //SHARPENING CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0150, 0x14);  // SHPTHLR   
    i2cWrite_HM1375(0x0151, 0x30);  // SHPTHLR_A 
    i2cWrite_HM1375(0x0152, 0x54);  // SHPTHHR   
    i2cWrite_HM1375(0x0153, 0x70);  // SHPTHHR_A 
    i2cWrite_HM1375(0x0154, 0x14);  // SHPTHLG   
    i2cWrite_HM1375(0x0155, 0x30);  // SHPTHLG_A 
    i2cWrite_HM1375(0x0156, 0x54);  // SHPTHHG   
    i2cWrite_HM1375(0x0157, 0x70);  // SHPTHHG_A 
    i2cWrite_HM1375(0x0158, 0x14);  // SHPTHLB   
    i2cWrite_HM1375(0x0159, 0x30);  // SHPTHLB_A 
    i2cWrite_HM1375(0x015A, 0x54);  // SHPTHHB   
    i2cWrite_HM1375(0x015B, 0x70);  // SHPTHHB_A 
    i2cWrite_HM1375(0x015C, 0x30);  // SHPSTR    _Sharpness strength
    i2cWrite_HM1375(0x015D, 0x00);  // SHPSTR_A  _sharpness strength_Alpha0


    //----------------------------------------------
    // NOISE FILTER CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x01D8, 0x20);  // NFHTHG
    i2cWrite_HM1375(0x01D9, 0x08);  // NFLTHG
    i2cWrite_HM1375(0x01DA, 0x20);  // NFHTHB
    i2cWrite_HM1375(0x01DB, 0x08);  // NFLTHB
    i2cWrite_HM1375(0x01DC, 0x20);  // NFHTHR
    i2cWrite_HM1375(0x01DD, 0x08);  // NFLTHR
    i2cWrite_HM1375(0x01DE, 0x20);  // NFHTHG_A
    i2cWrite_HM1375(0x01DF, 0x08);  // NFLTHG_A
    i2cWrite_HM1375(0x01E0, 0x20);  // NFHTHB_A
    i2cWrite_HM1375(0x01E1, 0x08);  // NFLTHB_A
    i2cWrite_HM1375(0x01E2, 0xFF);  // NFHTHR_A
    i2cWrite_HM1375(0x01E3, 0x00);  // NFLTHR_A
    i2cWrite_HM1375(0x01E4, 0x10);  // NFSTR
    i2cWrite_HM1375(0x01E5, 0x10);  // NFSTR_A 
    i2cWrite_HM1375(0x01E6, 0x02);  // NFSTR_OUTA
    i2cWrite_HM1375(0x01E7, 0x10);  // NFMTHG   
    i2cWrite_HM1375(0x01E8, 0x10);  // NFMTHB  
    i2cWrite_HM1375(0x01E9, 0x10);  // NFMTHR
    i2cWrite_HM1375(0x01EA, 0x10);  // NFMTHR_A
    i2cWrite_HM1375(0x01EC, 0xFA);  // NFMTHR_A
    i2cWrite_HM1375(0x01EB, 0x10);  // NFMTHR_A

    //----------------------------------------------
    //LENS SHADING CORRECTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0220, 0xF0);
    i2cWrite_HM1375(0x0221, 0xA0);
    i2cWrite_HM1375(0x0222, 0x00);
    i2cWrite_HM1375(0x0223, 0x80);
    i2cWrite_HM1375(0x0224, 0x80);
    i2cWrite_HM1375(0x0225, 0x00);
    i2cWrite_HM1375(0x0226, 0x80);
    i2cWrite_HM1375(0x0227, 0x80);
    i2cWrite_HM1375(0x0228, 0x00);
    i2cWrite_HM1375(0x0229, 0x80);
    i2cWrite_HM1375(0x022A, 0x80);
    i2cWrite_HM1375(0x022B, 0x00);
    i2cWrite_HM1375(0x022C, 0x80);
    i2cWrite_HM1375(0x022D, 0x12);
    i2cWrite_HM1375(0x022E, 0x10);
    i2cWrite_HM1375(0x022F, 0x12);
    i2cWrite_HM1375(0x0230, 0x10);
    i2cWrite_HM1375(0x0231, 0x12);
    i2cWrite_HM1375(0x0232, 0x10);
    i2cWrite_HM1375(0x0233, 0x12);
    i2cWrite_HM1375(0x0234, 0x10);
    i2cWrite_HM1375(0x0235, 0x80);
    i2cWrite_HM1375(0x0236, 0x02);
    i2cWrite_HM1375(0x0237, 0x80);
    i2cWrite_HM1375(0x0238, 0x02);
    i2cWrite_HM1375(0x0239, 0x80);
    i2cWrite_HM1375(0x023A, 0x02);
    i2cWrite_HM1375(0x023B, 0x80);
    i2cWrite_HM1375(0x023C, 0x02);
    i2cWrite_HM1375(0x023D, 0x00);
    i2cWrite_HM1375(0x023E, 0x02);
    i2cWrite_HM1375(0x023F, 0x00);
    i2cWrite_HM1375(0x0240, 0x02);
    i2cWrite_HM1375(0x0241, 0x00);
    i2cWrite_HM1375(0x0242, 0x02);
    i2cWrite_HM1375(0x0243, 0x00);
    i2cWrite_HM1375(0x0244, 0x02);
    i2cWrite_HM1375(0x0251, 0x10);

    //----------------------------------------------
    //GAMMA CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0280, 0x00);  // normal Gamma
    i2cWrite_HM1375(0x0281, 0x5D);  
    i2cWrite_HM1375(0x0282, 0x00);  
    i2cWrite_HM1375(0x0283, 0xA1);  
    i2cWrite_HM1375(0x0284, 0x00);  
    i2cWrite_HM1375(0x0285, 0xF5);  
    i2cWrite_HM1375(0x0286, 0x01);  
    i2cWrite_HM1375(0x0287, 0x5E);  
    i2cWrite_HM1375(0x0288, 0x01);  
    i2cWrite_HM1375(0x0289, 0x8C);  
    i2cWrite_HM1375(0x028A, 0x01);  
    i2cWrite_HM1375(0x028B, 0xB8);  
    i2cWrite_HM1375(0x028C, 0x01);  
    i2cWrite_HM1375(0x028D, 0xDD);  
    i2cWrite_HM1375(0x028E, 0x01);  
    i2cWrite_HM1375(0x028F, 0xFF);  
    i2cWrite_HM1375(0x0290, 0x02);  
    i2cWrite_HM1375(0x0291, 0x24);  
    i2cWrite_HM1375(0x0292, 0x02);  
    i2cWrite_HM1375(0x0293, 0x46);  
    i2cWrite_HM1375(0x0294, 0x02);  
    i2cWrite_HM1375(0x0295, 0x84);  
    i2cWrite_HM1375(0x0296, 0x02);  
    i2cWrite_HM1375(0x0297, 0xB9);  
    i2cWrite_HM1375(0x0298, 0x03);  
    i2cWrite_HM1375(0x0299, 0x1C);  
    i2cWrite_HM1375(0x029A, 0x03);  
    i2cWrite_HM1375(0x029B, 0x69);  
    i2cWrite_HM1375(0x029C, 0x03);  
    i2cWrite_HM1375(0x029D, 0xB0);  // updated by MH 06/17
    i2cWrite_HM1375(0x029E, 0x00);  // slope high byte    
    i2cWrite_HM1375(0x029F, 0x69);  // slope low byte
    i2cWrite_HM1375(0x02A0, 0x04);  // GAM_A

    //----------------------------------------------
    //COLOR CORRECTION MATRIX
    //----------------------------------------------

    i2cWrite_HM1375(0x02C0, 0x80);  //CCM00_L    _Adjust for D65 color temperature
    i2cWrite_HM1375(0x02C1, 0x01);  //CCM00_H
    i2cWrite_HM1375(0x02C2, 0x71);  
    i2cWrite_HM1375(0x02C3, 0x04);  
    i2cWrite_HM1375(0x02C4, 0x0F);  
    i2cWrite_HM1375(0x02C5, 0x04);  
    i2cWrite_HM1375(0x02C6, 0x3D);  
    i2cWrite_HM1375(0x02C7, 0x04);  
    i2cWrite_HM1375(0x02C8, 0x94);  
    i2cWrite_HM1375(0x02C9, 0x01);  
    i2cWrite_HM1375(0x02CA, 0x57);  
    i2cWrite_HM1375(0x02CB, 0x04);  
    i2cWrite_HM1375(0x02CC, 0x0F);  
    i2cWrite_HM1375(0x02CD, 0x04);  
    i2cWrite_HM1375(0x02CE, 0x8F);  
    i2cWrite_HM1375(0x02CF, 0x04);  
    i2cWrite_HM1375(0x02D0, 0x9E);  
    i2cWrite_HM1375(0x02D1, 0x01);  
    i2cWrite_HM1375(0x02E0, 0x06);  // CCM_A     _reduce to 0.267x
    i2cWrite_HM1375(0x02E1, 0xC0);  // RESERVED
    i2cWrite_HM1375(0x02E2, 0xE0);  // RESERVED
    i2cWrite_HM1375(0x02F0, 0x48);  // ACCM00_L  _Adjust for IncA color temperature
    i2cWrite_HM1375(0x02F1, 0x01);  // ACCM00_H
    i2cWrite_HM1375(0x02F2, 0x32);  
    i2cWrite_HM1375(0x02F3, 0x04);  
    i2cWrite_HM1375(0x02F4, 0x16);  
    i2cWrite_HM1375(0x02F5, 0x04);  
    i2cWrite_HM1375(0x02F6, 0x52);  
    i2cWrite_HM1375(0x02F7, 0x04);  
    i2cWrite_HM1375(0x02F8, 0xAA);  
    i2cWrite_HM1375(0x02F9, 0x01);  
    i2cWrite_HM1375(0x02FA, 0x58);  
    i2cWrite_HM1375(0x02FB, 0x04);  
    i2cWrite_HM1375(0x02FC, 0x56);  
    i2cWrite_HM1375(0x02FD, 0x04);  
    i2cWrite_HM1375(0x02FE, 0xDD);  
    i2cWrite_HM1375(0x02FF, 0x04);  
    i2cWrite_HM1375(0x0300, 0x33);  
    i2cWrite_HM1375(0x0301, 0x02);  

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE WINDOW CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0324, 0x00);
    i2cWrite_HM1375(0x0325, 0x01);

    //----------------------------------------------
    //AUTOMATIC WHITE BALANCE DETECTION AND LIMITS
    //----------------------------------------------

    i2cWrite_HM1375(0x0333, 0x86);   
    i2cWrite_HM1375(0x0334, 0x00);  
    i2cWrite_HM1375(0x0335, 0x86);  
    i2cWrite_HM1375(0x0340, 0x40);      
    i2cWrite_HM1375(0x0341, 0x44);  
    i2cWrite_HM1375(0x0342, 0x4A);  
    i2cWrite_HM1375(0x0343, 0x2B);  
    i2cWrite_HM1375(0x0344, 0x94);  
    i2cWrite_HM1375(0x0345, 0x3F);  
    i2cWrite_HM1375(0x0346, 0x8E);  
    i2cWrite_HM1375(0x0347, 0x51);  
    i2cWrite_HM1375(0x0348, 0x75);  
    i2cWrite_HM1375(0x0349, 0x5C);  
    i2cWrite_HM1375(0x034A, 0x6A);  
    i2cWrite_HM1375(0x034B, 0x68);  
    i2cWrite_HM1375(0x034C, 0x5E);  
    i2cWrite_HM1375(0x0350, 0x7C);  
    i2cWrite_HM1375(0x0351, 0x78);  
    i2cWrite_HM1375(0x0352, 0x08);  
    i2cWrite_HM1375(0x0353, 0x04);  
    i2cWrite_HM1375(0x0354, 0x80);  
    i2cWrite_HM1375(0x0355, 0x9A);  
    i2cWrite_HM1375(0x0356, 0xCC);  
    i2cWrite_HM1375(0x0357, 0xFF);  
    i2cWrite_HM1375(0x0358, 0xFF);  
    i2cWrite_HM1375(0x035A, 0xFF);  
    i2cWrite_HM1375(0x035B, 0x00);  
    i2cWrite_HM1375(0x035C, 0x70);  
    i2cWrite_HM1375(0x035D, 0x80);  
    i2cWrite_HM1375(0x035F, 0xA0);  
    i2cWrite_HM1375(0x0488, 0x30);  
    i2cWrite_HM1375(0x0360, 0xDF);   
    i2cWrite_HM1375(0x0361, 0x00);  
    i2cWrite_HM1375(0x0362, 0xFF);  
    i2cWrite_HM1375(0x0363, 0x03);  
    i2cWrite_HM1375(0x0364, 0xFF);  
    i2cWrite_HM1375(0x037B, 0x11);  //whole nonEDR to EDR ratio 
    i2cWrite_HM1375(0x037C, 0x1E);  //EDR LONG litbin pop out to nonEDR 

    //----------------------------------------------
    //AUTOMATIC EXPOSURE CONFIGURATION
    //----------------------------------------------

    i2cWrite_HM1375(0x0380, 0xFF);       
    i2cWrite_HM1375(0x0383, 0x50);  // RESERVED       
    i2cWrite_HM1375(0x038A, 0x64);   
    i2cWrite_HM1375(0x038B, 0x64);   
    //i2cWrite_HM1375(0x038E, 0x3C);   
    i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);   
    i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
    i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
    i2cWrite_HM1375(0x0391, 0x2A);  // RESERVED        
    i2cWrite_HM1375(0x0393, 0x1E);  // RESERVED       
    i2cWrite_HM1375(0x0394, 0x64);  // RESERVED
    i2cWrite_HM1375(0x0395, 0x23);         
    i2cWrite_HM1375(0x0398, 0x03);  // RESERVED
    i2cWrite_HM1375(0x0399, 0x45);  // RESERVED
    i2cWrite_HM1375(0x039A, 0x06);  // RESERVED
    i2cWrite_HM1375(0x039B, 0x8B);  // RESERVED
    i2cWrite_HM1375(0x039C, 0x0D);  // RESERVED
    i2cWrite_HM1375(0x039D, 0x16);  // RESERVED
    i2cWrite_HM1375(0x039E, 0x0A);  // RESERVED
    i2cWrite_HM1375(0x039F, 0x10);       
    i2cWrite_HM1375(0x03A0, 0x10);          
    i2cWrite_HM1375(0x03A1, 0xE5);  // RESERVED   
    i2cWrite_HM1375(0x03A2, 0x06);  // RESERVED   
    i2cWrite_HM1375(0x03A4, 0x18);        
    i2cWrite_HM1375(0x03A5, 0x48);    
    i2cWrite_HM1375(0x03A6, 0x2D);  // RESERVED
    i2cWrite_HM1375(0x03A7, 0x78);  // RESERVED
    i2cWrite_HM1375(0x03AC, 0x5A);  // RESERVED
    i2cWrite_HM1375(0x03AD, 0x0F);  // RESERVED
    i2cWrite_HM1375(0x03AE, 0x7F);  // RESERVED
    i2cWrite_HM1375(0x03AF, 0x04);  // RESERVED
    i2cWrite_HM1375(0x03B0, 0x35);  // RESERVED
    i2cWrite_HM1375(0x03B1, 0x14);  // RESERVED
    i2cWrite_HM1375(0x036F, 0x04);  // Bayer denoise strength EDR
    i2cWrite_HM1375(0x0370, 0x0A);  // Bayer denoise strength nonEDR
    i2cWrite_HM1375(0x0371, 0x04);  // Bayer denoise strength EDR A0
    i2cWrite_HM1375(0x0372, 0x10);  // Bayer denoise strength nonEDR A0
    i2cWrite_HM1375(0x0373, 0x40);  // raw sharpness strength EDR
    i2cWrite_HM1375(0x0374, 0x20);  // raw sharpness strength nonEDR
    i2cWrite_HM1375(0x0375, 0x04);  // raw sharpness strength EDR A0
    i2cWrite_HM1375(0x0376, 0x00);  // raw sharpness strength nonEDR A0
    i2cWrite_HM1375(0x0377, 0x08);  // Y Denoise strength EDR
    i2cWrite_HM1375(0x0378, 0x08);  // Y Denoise strength nonEDR
    i2cWrite_HM1375(0x0379, 0x04);  // Y Denoise strength EDR A0
    i2cWrite_HM1375(0x037A, 0x08);  // Y Denoise strength nonEDR A0

    //----------------------------------------------
    //DIGITAL BLACK LEVEL OFFSET CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0420, 0x00);    
    i2cWrite_HM1375(0x0421, 0x00);    
    i2cWrite_HM1375(0x0422, 0x00);    
    i2cWrite_HM1375(0x0423, 0x84);    

    //----------------------------------------------
    //AUTO BLACK LEVEL 
    //----------------------------------------------

    i2cWrite_HM1375(0x0430, 0x10);   
    i2cWrite_HM1375(0x0431, 0x60);   
    i2cWrite_HM1375(0x0432, 0x10);   
    i2cWrite_HM1375(0x0433, 0x20);   
    i2cWrite_HM1375(0x0434, 0x00);   
    i2cWrite_HM1375(0x0435, 0x30);   
    i2cWrite_HM1375(0x0436, 0x00);   

    //----------------------------------------------
    //LOWLIGHT_OUTDOOR IPP CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0450, 0xFD);   
    i2cWrite_HM1375(0x0451, 0xD8);   
    i2cWrite_HM1375(0x0452, 0xA0);   
    i2cWrite_HM1375(0x0453, 0x50);   
    i2cWrite_HM1375(0x0454, 0x00);       
    i2cWrite_HM1375(0x0459, 0x04);    
    i2cWrite_HM1375(0x045A, 0x00);   
    i2cWrite_HM1375(0x045B, 0x30);   
    i2cWrite_HM1375(0x045C, 0x01);   
    i2cWrite_HM1375(0x045D, 0x70);   
    i2cWrite_HM1375(0x0460, 0x00);    
    i2cWrite_HM1375(0x0461, 0x00);    
    i2cWrite_HM1375(0x0462, 0x00);    
    i2cWrite_HM1375(0x0465, 0x16);  // AWB IIR
    i2cWrite_HM1375(0x0466, 0x14);    
    i2cWrite_HM1375(0x0478, 0x00);   

    //----------------------------------------------
    //COLOR SPACE CONVERSION_SATURATION ADJ
    //----------------------------------------------

    i2cWrite_HM1375(0x0480, 0x60);  
    i2cWrite_HM1375(0x0481, 0x06);  
    i2cWrite_HM1375(0x0482, 0x0C);  

    //----------------------------------------------
    //CONTRAST_BRIGHTNESS
    //----------------------------------------------


    i2cWrite_HM1375(0x04B0, 0x4C);  // Contrast 05032011 by mh
    i2cWrite_HM1375(0x04B1, 0x86);  // contrast 06/17 by mh
    i2cWrite_HM1375(0x04B2, 0x00);  //
    i2cWrite_HM1375(0x04B3, 0x18);  //
    i2cWrite_HM1375(0x04B4, 0x00);  //
    i2cWrite_HM1375(0x04B5, 0x00);  //
    i2cWrite_HM1375(0x04B6, 0x30);  //
    i2cWrite_HM1375(0x04B7, 0x00);  //
    i2cWrite_HM1375(0x04B8, 0x00);  //
    i2cWrite_HM1375(0x04B9, 0x10);  //
    i2cWrite_HM1375(0x04BA, 0x00);  //
    i2cWrite_HM1375(0x04BB, 0x00);  //
    i2cWrite_HM1375(0x04BD, 0x00);  //

    //----------------------------------------------
    //EDR CONTRAST
    //----------------------------------------------

    i2cWrite_HM1375(0x04D0, 0x56);     
    i2cWrite_HM1375(0x04D6, 0x30);  
    i2cWrite_HM1375(0x04DD, 0x10);  
    i2cWrite_HM1375(0x04D9, 0x16);  
    i2cWrite_HM1375(0x04D3, 0x18);  

    //----------------------------------------------
    //AE FLICKER STEP SIZE
    //----------------------------------------------

//#if (HM1375_FRAME_RATE == 30)
#if 1
    i2cWrite_HM1375(0x0540, 0x01);   
    i2cWrite_HM1375(0x0541, 0x06);   
    i2cWrite_HM1375(0x0542, 0x01);   
    i2cWrite_HM1375(0x0543, 0x3B);  
#else   // HM1375_FRAME_RATE == 15
    i2cWrite_HM1375(0x0540, 0x00);   
    i2cWrite_HM1375(0x0541, 0x83);   
    i2cWrite_HM1375(0x0542, 0x00);   
    i2cWrite_HM1375(0x0543, 0x9D);  
#endif
    i2cWrite_HM1375(0x0580, 0x50);  // RESERVED
    i2cWrite_HM1375(0x0581, 0x30);  // RESERVED

    //----------------------------------------------
    //Y_COLOR NOISE REDUCTION
    //----------------------------------------------

    i2cWrite_HM1375(0x0582, 0x2D);  
    i2cWrite_HM1375(0x0583, 0x16);  
    i2cWrite_HM1375(0x0584, 0x1E);  
    i2cWrite_HM1375(0x0585, 0x0F);  
    i2cWrite_HM1375(0x0586, 0x08);  
    i2cWrite_HM1375(0x0587, 0x10);  
    i2cWrite_HM1375(0x0590, 0x10);  
    i2cWrite_HM1375(0x0591, 0x10);  
    i2cWrite_HM1375(0x0592, 0x05);  
    i2cWrite_HM1375(0x0593, 0x05);  
    i2cWrite_HM1375(0x0594, 0x04);  
    i2cWrite_HM1375(0x0595, 0x06);  

    //----------------------------------------------
    //Y_Sharpness strength
    //----------------------------------------------

    i2cWrite_HM1375(0x05B0, 0x04);    
    i2cWrite_HM1375(0x05B1, 0x00);    


    //----------------------------------------------
    //WINDOW_SCALER
    //----------------------------------------------

    i2cWrite_HM1375(0x05E4, 0x02);  
    i2cWrite_HM1375(0x05E5, 0x00);  
    i2cWrite_HM1375(0x05E6, 0x81);  
    i2cWrite_HM1375(0x05E7, 0x02);  
    i2cWrite_HM1375(0x05E8, 0x09);  
    i2cWrite_HM1375(0x05E9, 0x00);  
    i2cWrite_HM1375(0x05EA, 0xE8);  
    i2cWrite_HM1375(0x05EB, 0x01);  

    //----------------------------------------------
    //FLEXI ENGINE_AE ADJUST CONTROL
    //----------------------------------------------

    i2cWrite_HM1375(0x0666, 0x02);        
    i2cWrite_HM1375(0x0667, 0xE0);      
    i2cWrite_HM1375(0x067F, 0x19);      
    i2cWrite_HM1375(0x067C, 0x00);  
    i2cWrite_HM1375(0x067D, 0x00);  
    i2cWrite_HM1375(0x0682, 0x00);  
    i2cWrite_HM1375(0x0683, 0x00);      
    i2cWrite_HM1375(0x0688, 0x00);  
    i2cWrite_HM1375(0x0689, 0x00);  
    i2cWrite_HM1375(0x068E, 0x00);  
    i2cWrite_HM1375(0x068F, 0x00);  
    i2cWrite_HM1375(0x0695, 0x00);   
    i2cWrite_HM1375(0x0694, 0x00);      
    i2cWrite_HM1375(0x0697, 0x19);      
    i2cWrite_HM1375(0x069B, 0x00);  
    i2cWrite_HM1375(0x069C, 0x20);  // max EDR ratio   
    i2cWrite_HM1375(0x0720, 0x00);  
    i2cWrite_HM1375(0x0725, 0x6A);      
    i2cWrite_HM1375(0x0726, 0x03);  
    i2cWrite_HM1375(0x072B, 0x64);  
    i2cWrite_HM1375(0x072C, 0x64);  
    i2cWrite_HM1375(0x072D, 0x20);  
    i2cWrite_HM1375(0x072E, 0x82);  //turn off night mode
    i2cWrite_HM1375(0x072F, 0x08);      
    i2cWrite_HM1375(0x0800, 0x16);  
    i2cWrite_HM1375(0x0801, 0x4F);    
    i2cWrite_HM1375(0x0802, 0x00);  
    i2cWrite_HM1375(0x0803, 0x68);  
    i2cWrite_HM1375(0x0804, 0x01);  
    i2cWrite_HM1375(0x0805, 0x28);  
    i2cWrite_HM1375(0x0806, 0x10);   
    i2cWrite_HM1375(0x0808, 0x1D);  
    i2cWrite_HM1375(0x0809, 0x18);  
    i2cWrite_HM1375(0x080A, 0x10);       
    i2cWrite_HM1375(0x080B, 0x07);       
    i2cWrite_HM1375(0x080D, 0x0F);  
    i2cWrite_HM1375(0x080E, 0x0F);    
    i2cWrite_HM1375(0x0810, 0x00);  
    i2cWrite_HM1375(0x0811, 0x08);       
    i2cWrite_HM1375(0x0812, 0x20);  
    i2cWrite_HM1375(0x0857, 0x0A);  
    i2cWrite_HM1375(0x0858, 0x04);  
    i2cWrite_HM1375(0x0859, 0x01);  
#if 0   // 15fps
    i2cWrite_HM1375(0x085A, 0x08);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x30);  
#else   // 30fps
    i2cWrite_HM1375(0x085A, 0x04);  //(whole)Max INTG
    i2cWrite_HM1375(0x085B, 0x18);  
#endif
    i2cWrite_HM1375(0x085C, 0x03);  
    i2cWrite_HM1375(0x085D, 0x7F);  
    i2cWrite_HM1375(0x085E, 0x02);  //(Long)Max INTG  
    i2cWrite_HM1375(0x085F, 0xD0);  
    i2cWrite_HM1375(0x0860, 0x03);      
    i2cWrite_HM1375(0x0861, 0x7F);  
    i2cWrite_HM1375(0x0862, 0x02);  //(short)Max INTG     
    i2cWrite_HM1375(0x0863, 0xD0);  
    i2cWrite_HM1375(0x0864, 0x02);  //(short)Max AG   
    i2cWrite_HM1375(0x0865, 0x7F);  
    i2cWrite_HM1375(0x0866, 0x01);  
    i2cWrite_HM1375(0x0867, 0x00);  
    i2cWrite_HM1375(0x0868, 0x40);  
    i2cWrite_HM1375(0x0869, 0x01);  
    i2cWrite_HM1375(0x086A, 0x00);  
    i2cWrite_HM1375(0x086B, 0x40);  
    i2cWrite_HM1375(0x086C, 0x01);  
    i2cWrite_HM1375(0x086D, 0x00);  
    i2cWrite_HM1375(0x086E, 0x40);  
    i2cWrite_HM1375(0x0870, 0x00);  
    i2cWrite_HM1375(0x0871, 0x56);  
    i2cWrite_HM1375(0x0872, 0x00);  
    i2cWrite_HM1375(0x0873, 0xEC);  
    i2cWrite_HM1375(0x0874, 0x00);  
    i2cWrite_HM1375(0x0875, 0x56);  
    i2cWrite_HM1375(0x0876, 0x00);  
    i2cWrite_HM1375(0x0877, 0xEC);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MAXIMUM EDR 
    //----------------------------------------------

    i2cWrite_HM1375(0x0815, 0x00);  
    i2cWrite_HM1375(0x0816, 0x4C);  
    i2cWrite_HM1375(0x0817, 0x00);  
    i2cWrite_HM1375(0x0818, 0x7B);  
    i2cWrite_HM1375(0x0819, 0x00);  
    i2cWrite_HM1375(0x081A, 0xCA);  
    i2cWrite_HM1375(0x081B, 0x01);  
    i2cWrite_HM1375(0x081C, 0x3E);  
    i2cWrite_HM1375(0x081D, 0x01);  
    i2cWrite_HM1375(0x081E, 0x77);  
    i2cWrite_HM1375(0x081F, 0x01);  
    i2cWrite_HM1375(0x0820, 0xAA);  
    i2cWrite_HM1375(0x0821, 0x01);  
    i2cWrite_HM1375(0x0822, 0xCE);  
    i2cWrite_HM1375(0x0823, 0x01);  
    i2cWrite_HM1375(0x0824, 0xEE);  
    i2cWrite_HM1375(0x0825, 0x02);  
    i2cWrite_HM1375(0x0826, 0x16);  
    i2cWrite_HM1375(0x0827, 0x02);  
    i2cWrite_HM1375(0x0828, 0x33);  
    i2cWrite_HM1375(0x0829, 0x02);  
    i2cWrite_HM1375(0x082A, 0x65);  
    i2cWrite_HM1375(0x082B, 0x02);  
    i2cWrite_HM1375(0x082C, 0x91);  
    i2cWrite_HM1375(0x082D, 0x02);  
    i2cWrite_HM1375(0x082E, 0xDC);  
    i2cWrite_HM1375(0x082F, 0x03);  
    i2cWrite_HM1375(0x0830, 0x28);  
    i2cWrite_HM1375(0x0831, 0x03);  
    i2cWrite_HM1375(0x0832, 0x74);  
    i2cWrite_HM1375(0x0833, 0x03);  
    i2cWrite_HM1375(0x0834, 0xFF);  

    //----------------------------------------------
    //FLEXI ENGINE_GAMMA FOR MINIMUM EDR
    //----------------------------------------------

    i2cWrite_HM1375(0x0882, 0x00);  
    i2cWrite_HM1375(0x0883, 0x3E);  
    i2cWrite_HM1375(0x0884, 0x00);  
    i2cWrite_HM1375(0x0885, 0x70);  
    i2cWrite_HM1375(0x0886, 0x00);  
    i2cWrite_HM1375(0x0887, 0xB8);  
    i2cWrite_HM1375(0x0888, 0x01);  
    i2cWrite_HM1375(0x0889, 0x28);  
    i2cWrite_HM1375(0x088A, 0x01);  
    i2cWrite_HM1375(0x088B, 0x5B);  
    i2cWrite_HM1375(0x088C, 0x01);  
    i2cWrite_HM1375(0x088D, 0x8A);  
    i2cWrite_HM1375(0x088E, 0x01);  
    i2cWrite_HM1375(0x088F, 0xB1);  
    i2cWrite_HM1375(0x0890, 0x01);  
    i2cWrite_HM1375(0x0891, 0xD9);  
    i2cWrite_HM1375(0x0892, 0x01);  
    i2cWrite_HM1375(0x0893, 0xEE);  
    i2cWrite_HM1375(0x0894, 0x02);  
    i2cWrite_HM1375(0x0895, 0x0F);  
    i2cWrite_HM1375(0x0896, 0x02);  
    i2cWrite_HM1375(0x0897, 0x4C);  
    i2cWrite_HM1375(0x0898, 0x02);  
    i2cWrite_HM1375(0x0899, 0x74);  
    i2cWrite_HM1375(0x089A, 0x02);  
    i2cWrite_HM1375(0x089B, 0xC3);  
    i2cWrite_HM1375(0x089C, 0x03);  
    i2cWrite_HM1375(0x089D, 0x0F);  
    i2cWrite_HM1375(0x089E, 0x03);  
    i2cWrite_HM1375(0x089F, 0x57);  
    i2cWrite_HM1375(0x08A0, 0x03);  
    i2cWrite_HM1375(0x08A1, 0xFF);  

    //----------------------------------------------
    //COMMAND UPDATE_TRIGGER
    //----------------------------------------------
    //OSTimeDly(4);
    i2cWrite_HM1375(0x0100, 0x01);  // CMU AE
    i2cWrite_HM1375(0x0101, 0x01);  // CMU AWB
    i2cWrite_HM1375(0x0000, 0x01);  // CMU
    i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger

}

#endif


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
	u8  data, level;
    u32 i;
    u32 tempReg;
    int FrameRate=30;
	u8	data1=0;
    #if((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
        (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    static u8  GCT_run_onu=1;
    #endif
    //----------//
#if( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN) )
DEBUG_SIU("siuSensorInit(%d,%d)\n",siuSensorMode,zoomFactor);//debug only
#endif
#if( (HW_BOARD_OPTION == A1013_FPGA_BOARD) || (HW_BOARD_OPTION == A1016_FPGA_BOARD) || \
    (HW_BOARD_OPTION == A1016B_FPGA_BOARD) || (HW_BOARD_OPTION == A1018_FPGA_BOARD)|| \
    (HW_BOARD_OPTION == A1018B_FPGA_BOARD) || (HW_BOARD_OPTION  == A1020A_FPGA_BOARD)||\
    (HW_BOARD_OPTION  == A1026A_FPGA_BOARD))
    //---------Run on Mclk=32 MHz--------------//    
    #if(SYS_CPU_CLK_FREQ == 32000000)
	  SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x00; //MClk=32/1=32MHz
	#elif(SYS_CPU_CLK_FREQ == 48000000)
      SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x01; //MClk=48/2=24MHz
	#endif    
#else   
   //---------Run on Mclk=24 MHz--------------//
   #if(SYS_CPU_CLK_FREQ == 96000000)
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x0b; //MClk=288/12=24MHz
   #elif(SYS_CPU_CLK_FREQ == 108000000)
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x08; //MClk=216/9=24MHz
   #elif(SYS_CPU_CLK_FREQ == 160000000)
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x13; //MClk=480/20=24MHz 
   #elif(SYS_CPU_CLK_FREQ == 180000000)
    SYS_CLK1 = (SYS_CLK1 & (~0x000000ff)) | 0x13; //MClk=540/20=24MHz  
   #endif
      
#endif

#if RFIU_SUPPORT 
   gRfiuUnitCntl[0].RFpara.TX_SensorType=TX_SENSORTYPE_HD;
#endif

    switch(siuSensorMode)   	
    {	
        case SIUMODE_PREVIEW: 
        case SIUMODE_MPEGAVI: 
            if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )
            {
                #if(HW_BOARD_OPTION == MR8120_TX_TRANWO)
                FrameRate   = 15;
                SetHM1375_720P_15FPS_TRANWO();                
                #elif(HW_BOARD_OPTION == MR8120_TX_SUNIN_820HD)
                FrameRate   = 15;
                SetHM1375_720P_15FPS_SUNNIN();
                #elif( (HW_BOARD_OPTION ==  A1018_EVB_128M) || (HW_BOARD_OPTION ==  A1018_EVB_256M_TW2866) || (HW_BOARD_OPTION  == A1018_EVB_256M_HM1375) || (HW_BOARD_OPTION ==  MR9120_TX_DB) )
                FrameRate   = 30;
                SetHM1375_720P_30FPS();
                #elif((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
                    (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
                  if (GCT_run_onu)
                  {
                      #if USE_704x480_RESO
                         FrameRate   = 30;
                         SetHM1375_720P_30FPS();
                      #else
                         FrameRate   = 15;
                         SetHM1375_720P_15FPS();
                      #endif
                      GCT_run_onu = 0;
                  }
                  else
                  {
                  #if(SW_APPLICATION_OPTION == MR8211_RFCAM_TX1)
                      if(sys8211TXWifiStat==MR8211_ENTER_WIFI)
                      {
                         if(rfiu_TX_P2pVideoQuality == RFWIFI_P2P_QUALITY_HIGH)
                         {
                            FrameRate   = 15;
                            SetHM1375_720P_change_15FPS();
                         }
                         else
                         {
                            FrameRate   = 30;
                            SetHM1375_720P_change_30FPS();
                         }
                      }
                      else if(sys8211TXWifiStat==MR8211_LEAVING_WIFI)
                      {
                         FrameRate   = 30;
                         SetHM1375_720P_change_30FPS();
                      }
                      else
                      {
                      #if USE_704x480_RESO
                         FrameRate   = 30;
                         //SetHM1375_720P_30FPS();
                      #else
                         FrameRate   = 15;
                         //SetHM1375_720P_15FPS();
                      #endif
                      }
                  #endif
                  }
                #elif( (HW_BOARD_OPTION == MR8120_TX_DB2) &&  ( (SW_APPLICATION_OPTION == MR8100_RFCAM_TX1) || (SW_APPLICATION_OPTION == MR8211_RFCAM_TX1) ) )
                  #if USE_704x480_RESO
                     FrameRate   = 30;
                     SetHM1375_720P_30FPS();
                  #else
                     FrameRate   = 15;
                     SetHM1375_720P_15FPS();
                  #endif
                #else
                FrameRate   = 15;
                SetHM1375_720P_15FPS();
                #endif
                
            }
            else if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_640x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_320x240) )
            {
                FrameRate   = 30;
            #if (HW_BOARD_OPTION == MR8120_TX_COMMAX)
                SetHM1375_VGA_30FPS_Commax();
            #elif (HW_BOARD_OPTION == MR8120_TX_TRANWO)
                SetHM1375_VGA_30FPS_TRANWO();
            #else
                SetHM1375_VGA_30FPS();
            #endif
            }
			else
		    {
                FrameRate   = 30;
                SetHM1375_VGA_30FPS();
			}
        #if((HW_BOARD_OPTION == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA672) ||\
           (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) ||\
           (HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))

            #if((HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542))
                if (gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level) == 0)
                    level = SIU_DAY_MODE;
                if(level == SIU_DAY_MODE)
                {
                    DEBUG_SIU("@@enter day \n");
                    siuSetSensorDayNight(SIU_DAY_MODE);
                }
                else 
                {
                    DEBUG_SIU("##enter night \n");
                    siuSetSensorDayNight(SIU_NIGHT_MODE);
                }
                gpioGetLevel(GPIO_GROUP_MIRROR_FLIP,GPIO_PIN_MIRROR_FLIP,&level);
                if(level == GPIO_MIRROR_FLIP_ON)
                {
                    SetHM1375_MIRROR_FLIP(GPIO_MIRROR_FLIP_ON);
                }
                else
                {
                    SetHM1375_MIRROR_FLIP(GPIO_MIRROR_FLIP_OFF);

                }
            #else
                  #if SMART_LED
                    if(((AdcRecData_G1G2 >> 16) ^ 0x0800) < 0x0800)
                        level   = SIU_DAY_MODE;
                    else
                        level   = SIU_NIGHT_MODE;
                  #else
                    if (gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level) == 0)
                        level = SIU_DAY_MODE;
                  #endif

                    if(level == SIU_DAY_MODE)
                    {
                        DEBUG_SIU("@@enter day \n");
                    #if SMART_LED
                        Timer3Ctrl  = 0x06ff0004;   // LED全暗
                    #endif
                        siuSetSensorDayNight(SIU_DAY_MODE);
                    }
                    else 
                    {
                        DEBUG_SIU("##enter night \n");
                    #if SMART_LED
                        Timer3Ctrl  = 0x06400004;   // LED全亮
                    #endif
                        siuSetSensorDayNight(SIU_NIGHT_MODE);
                    }

            #endif
        #elif(HW_BOARD_OPTION == MR8120_TX_RDI_CA652)
            siuSetSensorDayNight(DayNightMode);
        #elif((HW_BOARD_OPTION  == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
            (HW_BOARD_OPTION  == MR8120S_TX_GCT_VM00)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)||\
            (HW_BOARD_OPTION == MR8120_TX_GCT_VM00))
            siuSetSensorDayNight(DayNightMode);
        #elif( (HW_BOARD_OPTION  == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
            gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level);

            if(level == SIU_DAY_MODE)
            {
                DEBUG_SIU("@@enter day \n");
                siuSetSensorDayNight(SIU_DAY_MODE);
            }
            else 
            {
                DEBUG_SIU("##enter night \n");
                siuSetSensorDayNight(SIU_NIGHT_MODE);
            }
        #elif(HW_BOARD_OPTION == MR8120_TX_JIT)
            gpioGetLevel(GPIO_GROUP_IR_DETECTION,GPIO_PIN_IR_DETECTION,&level);

            //if(level == SIU_DAY_MODE)
            if(level == 1)
            {
                DEBUG_SIU("@@enter day \n");
                siuSetSensorDayNight(SIU_NIGHT_MODE);
            }
            else 
            {
                DEBUG_SIU("##enter night \n");
                siuSetSensorDayNight(SIU_DAY_MODE);
            }
            SetHM1375_MIRROR_FLIP_1(0, GPIO_MIRROR_FLIP_ON);
		#elif( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN)&&(USE_SENSOR_DAY_NIGHT_MODE==2) )
            gpioGetLevel(GPIO_GROUP_DAY_NIGHT,GPIO_PIN_DAY_NIGHT,&level);
            if(level == SIU_DAY_MODE)
            {
                DEBUG_SIU("@@enter day \n");
                siuSetSensorDayNight(SIU_DAY_MODE);
            }
            else 
            {
                DEBUG_SIU("##enter night \n");
                siuSetSensorDayNight(SIU_NIGHT_MODE);
            }			
        #endif    

    	/*
    	SetHM1375_720P_15FPS();
		i2cWrite_HM1375(0x0005,0x00);
		OSTimeDly(2);
		while(1)
		{
			data1	= 0x01;
			i2cWrite_HM1375(0x0281,0x10);
			i2cRead_HM1375(0x0281,&data1);
			if(data1 != 0x10)
			{
				DEBUG_SIU("\nerror data1 = %x\n", data1);
			}
			
				
			data1	= 0x01;
			i2cWrite_HM1375(0x0281,0x25);
			i2cRead_HM1375(0x0281,&data1);
			if(data1 != 0x25)
			{
				DEBUG_SIU("\nerror data1 = %x\n", data1);
			}
			
    	}
    	*/

            break;
        //-------------------//
        case SIUMODE_CAPTURE: 
        case SIUMODE_CAP_RREVIEW: 
            if(uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720)
            {
                FrameRate   = 15;
                SetHM1375_720P_15FPS();            
            }
            else
            {
                FrameRate   = 30;
                SetHM1375_720P_30FPS();            
            }
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
    
#if FINE_TIME_STAMP // use IIS time + Timer3 to calculate frame time
	s32 IISTimeOffsetSIU, TimeOffset;
	u32 IISTime1;
#endif
	
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

	Set subject distance for focus.

Arguments:

	subjectDistance - Subject distance.
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetSubjectDistance(u32 subjectDistance)
{
	exifSetSubjectDistance(subjectDistance);	/* in unit of centimeter */

	/* set initial position of zoom lens for focus */
				
	return 1;
}

/*

Routine Description:

	Set exposure bias value.

Arguments:

	expBiasValue - Exposure bias value (expBiasValue = Exposure Bias Value * 10)
	               Valid value : 0~8
	
Return Value:

	0 - Failure.
	1 - Success.

*/
s32 siuSetExposureValue(s8 expBiasValue)
{
	const s8 ExposureBiasValueTab[9]    = {-20, -15, -10, -5, 0, 5, 10, 15, 20};
	u8	data;
    int count;

	
    if((expBiasValue > 8) || (expBiasValue < 0))
    {
        DEBUG_SIU("siuSetExposureValue(%d) fail!!!\n", expBiasValue);
        return 0;
    }

	exifSetExposureBiasValue(ExposureBiasValueTab[expBiasValue]);		/* expBiasValue = Exposure Bias Value * 10 */		
	siuY_TargetIndex    = expBiasValue;
	
	/* set exposure bias */
    if(IsSensorInit)    // MCLK 設為24MHz之後才存取IIC
    {
    #if( (HW_BOARD_OPTION == MR8100_GCT_VM9710) || (HW_BOARD_OPTION == MR8120_TX_DB2) || (HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||\
        (HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I)||(HW_BOARD_OPTION == MR8120_TX_GCT_VM00))

    #else
        count=0;
    	do
    	{
    		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
        	OSTimeDly(4);
    		data 	= 0xff;
    		i2cRead_HM1375(0x0005, &data);
    	    OSTimeDly(4);

            if(data)
                DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
            if(count >50)
            {
               DEBUG_SIU("Sensor fatal Error!\n");
			   sysForceWDTtoReboot();
            }
            count ++;
    	} while(data != 0);
    #endif
#if (((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT != 7)&&(UI_PROJ_OPT != 4)&&(UI_PROJ_OPT != 2)&&(UI_PROJ_OPT != 5)&&(UI_PROJ_OPT != 8))) ||\
    (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652) ||\
    (HW_BOARD_OPTION  == MR8100_GCT_VM9710)||(HW_BOARD_OPTION  == MR8120S_TX_GCT_VM00)||\
    (HW_BOARD_OPTION == MR8120_TX_GCT_VM00))
        if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // HD不變
        {
            i2cWrite_HM1375(0x038E, AETargetMeanTab_720P[siuY_TargetIndex]);  
            i2cWrite_HM1375(0x0381, AETargetMeanTab_720P[siuY_TargetIndex]+12);
            i2cWrite_HM1375(0x0382, AETargetMeanTab_720P[siuY_TargetIndex]-12);
        }
        else    // VGA調亮
        {
            i2cWrite_HM1375(0x038E, AETargetMeanTab_VGA[siuY_TargetIndex]);  
            i2cWrite_HM1375(0x0381, AETargetMeanTab_VGA[siuY_TargetIndex]+12);
            i2cWrite_HM1375(0x0382, AETargetMeanTab_VGA[siuY_TargetIndex]-12);
        }
#elif ((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
    if(DayNightMode == SIU_NIGHT_MODE)
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab_720P_N[siuY_TargetIndex]);  
    }
    else
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab_720P[siuY_TargetIndex]);  
        i2cWrite_HM1375(0x0381, AETargetMeanTab_720P[siuY_TargetIndex]+12);
        i2cWrite_HM1375(0x0382, AETargetMeanTab_720P[siuY_TargetIndex]-12);
    }


#elif((HW_BOARD_OPTION == MR8120_TX_RDI)||(HW_BOARD_OPTION == MR8120_TX_RDI_CA532)|| (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||\
      ((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT == 7)||(UI_PROJ_OPT == 4) ||(UI_PROJ_OPT == 5) || (UI_PROJ_OPT == 8))))
    #if 0
        i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);  
    #else   // 20140115
        if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // HD不變
        {
            i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);  
            i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
            i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
        }
        else    // VGA調亮
        {
            i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex] + 5);  
            i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+17);
            i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-7);
        }
    #endif
#elif( (HW_BOARD_OPTION  == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
        //if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // HD不變
        {
            i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);  
            i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
            i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
        }
        /*
        else    // VGA調亮
        {
            i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex] + 5);  
            i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+17);
            i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-7);
        }*/
#else
        i2cWrite_HM1375(0x038E, AETargetMeanTab[siuY_TargetIndex]);  
        i2cWrite_HM1375(0x0381, AETargetMeanTab[siuY_TargetIndex]+12);
        i2cWrite_HM1375(0x0382, AETargetMeanTab[siuY_TargetIndex]-12);
#endif
        i2cWrite_HM1375(0x0005, 0x01);  // Trigger
        //i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
    }


	return 1;
}

#if 0 //( (HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ( (UI_PROJ_OPT == 9) ) )//解決flicker 切換異常暫不使用 170210
s32 siuSetFlicker50_60Hz(int flicker_sel)
{
    DEBUG_GREEN("siuSetFlicker50_60Hz TEST1\n");
    if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
    OSTimeDly(5);
    if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )   // HD不變
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab_720P[siuY_TargetIndex]+0x30);  
        i2cWrite_HM1375(0x0100,0x01);
        i2cWrite_HM1375(0x0101,0x01);
        OSTimeDly(5);
        i2cWrite_HM1375(0x038E, AETargetMeanTab_720P[siuY_TargetIndex]);  
    }
    else
    {
        i2cWrite_HM1375(0x038E, AETargetMeanTab_VGA[siuY_TargetIndex]+0x30);
        i2cWrite_HM1375(0x0100,0x01);
        i2cWrite_HM1375(0x0101,0x01);
        OSTimeDly(5);
        i2cWrite_HM1375(0x038E, AETargetMeanTab_VGA[siuY_TargetIndex]);  
    }
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger
    //i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051


    return 0;
}
#else
s32 siuSetFlicker50_60Hz(int flicker_sel)
{
	u8	data;
    int count;

    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);

        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    if(flicker_sel == SENSOR_AE_FLICKER_60HZ)  //60Hz
    {
        i2cWrite_HM1375(0x0120, 0x37);  // IPPCNTR1  
    }
    else //50Hz
    {
        i2cWrite_HM1375(0x0120, 0x36);  // IPPCNTR1  
    }
	i2cWrite_HM1375(0x0005, 0x01);  // Trigger
    //i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051


    return 0;
}
#endif
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
        		SIU_VSYNC_ACT_LO |                      //Vsync active low
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
		 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
          (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
          (CHIP_OPTION == CHIP_A1026A))
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		 #else
		        SIU_FRAM_VSYNC |
		 #endif          		
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
		SIU_VSYNC_ACT_LO |              //Vsync active low.
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
 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B)|| (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
 #else
        SIU_FRAM_VSYNC |
 #endif  		
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
	exifSetFlash(flash);
	exifSetApertureValue(&fNumber, &apertureValue);
	exifSetShutterSpeedValue(&exposureTime, &shutterSpeedValue);
	exifSetBrightnessValue(&brightnessValue);
		
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
		SIU_VSYNC_ACT_LO |              //Vsync active low.
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
 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
 #else
        SIU_FRAM_VSYNC |
 #endif  		
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
		SIU_VSYNC_ACT_LO |              //Vsync active low.
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
 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || (CHIP_OPTION == CHIP_A1016B) || \
    (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
 #else
        SIU_FRAM_VSYNC |
 #endif  		
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
        		SIU_VSYNC_ACT_LO |                      //Vsync active low
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
		 #if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
          (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
          (CHIP_OPTION == CHIP_A1026A))
		        SIU_FRAM_DATA_END |                     //Interrupt mode slection for frame end: data end
		 #else
		        SIU_FRAM_VSYNC |
		 #endif          		
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


    #if 0 //HW_MD_SUPPORT
        if(MotionDetect_en)
           MotionDetect_API(MD_SIU_ID);
    #endif

#if 0   // move to ciuTask_CH1(), Peter
    #if((HW_BOARD_OPTION == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA672) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
        gpioGetLevel(GPIO_GROUP_SENSOR_DAYNIGHT, GPIO_BIT_SENSOR_DAYNIGHT, &level);

        if(Mode != level)
        {
            mdset=MotionDetect_en;
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
            }
            else 
            {
                DEBUG_SIU("##enter night \n");
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                Mode    = SIU_NIGHT_MODE;
            }

            if(mdset)
            {
            #if HW_MD_SUPPORT
               mduMotionDetect_ONOFF(1);
    	    #endif
            }
        }


     #elif (HW_BOARD_OPTION == MR8120_TX_GCT )
        gpioGetLevel(GPIO_GROUP_DAY_NIGHT,GPIO_PIN_DAY_NIGHT, &level);
      
        if(Mode != level)
        {
            if(level == SIU_DAY_MODE)
            {      
                gpioSetLevel(GPIO_GROUP_DAY_PWM, GPIO_PIN_DAY_PWM,0);           
                for(i=0;i<pulse_time*210;i++)
                    gpioSetLevel(GPIO_GROUP_DAY_PWM, GPIO_PIN_DAY_PWM, 1);
                gpioSetLevel(GPIO_GROUP_DAY_PWM, GPIO_PIN_DAY_PWM,0);           
                siuSetSensorDayNight(SIU_DAY_MODE);
                Mode =SIU_DAY_MODE;
            }
            else
            {
                gpioSetLevel(GPIO_GROUP_NIGHT_PWM, GPIO_PIN_NIGHT_PWM, 0);
                for(i=0;i<pulse_time*210;i++)
                    gpioSetLevel(GPIO_GROUP_NIGHT_PWM, GPIO_PIN_NIGHT_PWM, 1);
                gpioSetLevel(GPIO_GROUP_NIGHT_PWM, GPIO_PIN_NIGHT_PWM, 0);
                siuSetSensorDayNight(SIU_NIGHT_MODE);
                Mode =SIU_NIGHT_MODE;
            }

        }
    #endif      
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

#if 1 //(HW_BOARD_OPTION==SALIX_SDV) // After Fine Tune @Lucian 20080729
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
	    case SIUMODE_CAP_RREVIEW: 	         		
        case SIUMODE_MPEGAVI: //Video clip mode: 不支援Zoom-in/Out
            if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )
            {
	            Img_Width   = 1280;
	            Img_Height  = 720;
	                        
	            sensor_validsize.imgSize.x  = Img_Width * 2;
	            sensor_validsize.imgSize.y  = Img_Height;
	            sensor_validsize.imgStr.x   = 0;
	            sensor_validsize.imgStr.y   = 1;

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
	            sensor_validsize.imgStr.y   = 1;

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
	    		sensor_validsize.imgStr.y = 1;	       
	    		
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

#if( (CHIP_OPTION == CHIP_A1013A) || (CHIP_OPTION == CHIP_A1013B) || (CHIP_OPTION == CHIP_A1016A) || \
    (CHIP_OPTION == CHIP_A1016B) || (CHIP_OPTION == CHIP_A1018A) || (CHIP_OPTION == CHIP_A1018B) || (CHIP_OPTION == CHIP_A1020A)||\
    (CHIP_OPTION == CHIP_A1026A))
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
#endif

#if((HW_BOARD_OPTION == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA672) || (HW_BOARD_OPTION == MR8120_TX_GCT) || (HW_BOARD_OPTION == MR8211_ZINWELL)||\
    (HW_BOARD_OPTION == MR8120_TX_JIT)|| (HW_BOARD_OPTION == MR8120_TX_MA8806) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA532) ||(HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) ||\
    ( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN)&&(USE_SENSOR_DAY_NIGHT_MODE==2) ) ||\
    (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    DayNightMode =Level;
    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);
        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    if(Level == SIU_NIGHT_MODE) //AGC > 16x: 進入夜間模式
	{   //夜間模式
        DEBUG_SIU("##enter night\n");
        i2cWrite_HM1375(0x085C, 0x04);
        i2cWrite_HM1375(0x085D, 0xB0);
        i2cWrite_HM1375(0x02A0, 0x06);
        i2cWrite_HM1375(0x04C1, 0x08);
        i2cWrite_HM1375(0x0370, 0x08);
        i2cWrite_HM1375(0x0372, 0x02);
        #if (NIGHT_COLOR_ENA)
        i2cWrite_HM1375(0x0480, 0x60); //夜間彩色
        #else
        i2cWrite_HM1375(0x0480, 0x00);
        #endif
        i2cWrite_HM1375(0x0481, 0x04);
        
    #if((HW_BOARD_OPTION == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA672) ||\
        (HW_BOARD_OPTION == MR8120_TX_RDI_CA532)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CA542)||(HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) ||\
        (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))   // 20131112
        i2cWrite_HM1375(0x04B6, 0x30);
        i2cWrite_HM1375(0x04B9, 0x10);
        i2cWrite_HM1375(0x05B0, 0x04);
        i2cWrite_HM1375(0x0374, 0x40);
        i2cWrite_HM1375(0x0370, 0x02);

        // for test
        i2cWrite_HM1375(0x085D, 0xC0);
        i2cWrite_HM1375(0x085A, 0x06);  //(whole)Max INTG
        i2cWrite_HM1375(0x085B, 0x80);  

      #if (((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT != 7)&&(UI_PROJ_OPT != 4)&&(UI_PROJ_OPT != 2)&&(UI_PROJ_OPT != 5)&&(UI_PROJ_OPT != 8)))||\
          (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652) )
        // 20140306
        i2cWrite_HM1375(0x04B0, 0x60);
        i2cWrite_HM1375(0x02A0, 0x08); 
        i2cWrite_HM1375(0x04C0, 0x10); 
        i2cWrite_HM1375(0x04C1, 0x08);
        i2cWrite_HM1375(0x05B0, 0x04); 
      #elif((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && (UI_PROJ_OPT == 5)) //夜間圖像不清晰,只先用在CA632
        i2cWrite_HM1375(0x04c1, 0x18);
        i2cWrite_HM1375(0x04c0, 0x00);
        i2cWrite_HM1375(0x04b0, 0x39);
        i2cWrite_HM1375(0x0374, 0x4b);
        i2cWrite_HM1375(0x05b0, 0x1f);
        i2cWrite_HM1375(0x038e, 0x2b);
        i2cWrite_HM1375(0x0803, 0x68);
      #else // (HW_BOARD_OPTION == MR8120_TX_RDI)
        // 20140113
        i2cWrite_HM1375(0x04B0, 0x60);
        i2cWrite_HM1375(0x04C0, 0x88); 
        i2cWrite_HM1375(0x05B0, 0x04);
      #endif

    #elif (HW_BOARD_OPTION == MR8211_ZINWELL)
        i2cWrite_HM1375(0x0372, 0x10);
        if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )
        {
            // 消除夜間模式下的flicker
            i2cWrite_HM1375(0x085A, 0x18);  //(whole)Max INTG
            i2cWrite_HM1375(0x085B, 0x80);  
        }
        else    // VGA
        {
            i2cWrite_HM1375(0x085A, 0x06);
            i2cWrite_HM1375(0x085B, 0x80);
        }
    #endif
    #if 0//((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && (UI_PROJ_OPT == 6)) //20161128
        i2cWrite_HM1375(0x02A0, 0x08);
        i2cWrite_HM1375(0x05B0, 0x04);
        i2cWrite_HM1375(0x05B1, 0x02);
        i2cWrite_HM1375(0x0374, 0x40);
        i2cWrite_HM1375(0x0370, 0x02);
        i2cWrite_HM1375(0x0372, 0x02);
        i2cWrite_HM1375(0x04B0, 0x60);
        i2cWrite_HM1375(0x04B6, 0x30);
        i2cWrite_HM1375(0x04BD, 0x00);
        i2cWrite_HM1375(0x04B9, 0x10);
        i2cWrite_HM1375(0x04B3, 0x18);
        i2cWrite_HM1375(0x04B1, 0x86);
        i2cWrite_HM1375(0x04C0, 0x10);
        i2cWrite_HM1375(0x04C1, 0x08);
        i2cWrite_HM1375(0x0430, 0x00);
        i2cWrite_HM1375(0x038E, 0x2B);
        i2cWrite_HM1375(0x0870, 0x00);
        i2cWrite_HM1375(0x0871, 0xA4);
        i2cWrite_HM1375(0x0872, 0x00);
        i2cWrite_HM1375(0x0873, 0xDE);
        i2cWrite_HM1375(0x0874, 0x00);
        i2cWrite_HM1375(0x0875, 0x57);
        i2cWrite_HM1375(0x0876, 0x00);
        i2cWrite_HM1375(0x0877, 0xEC);
    #endif

        i2cWrite_HM1375(0x0100, 0x01);
        i2cWrite_HM1375(0x0101, 0x01);
        i2cWrite_HM1375(0x0000, 0x01);
	}
    else
    {   //日間模式
        DEBUG_SIU("##enter day\n");
        i2cWrite_HM1375(0x085C, 0x03);
        i2cWrite_HM1375(0x085D, 0x7F);
        i2cWrite_HM1375(0x02A0, 0x04);
        i2cWrite_HM1375(0x04C1, 0x00);
        i2cWrite_HM1375(0x0370, 0x0A);
        i2cWrite_HM1375(0x0372, 0x00);
        i2cWrite_HM1375(0x0480, 0x60);
        i2cWrite_HM1375(0x0481, 0x06);
        i2cWrite_HM1375(0x0100, 0x01);
        i2cWrite_HM1375(0x0101, 0x01);
        i2cWrite_HM1375(0x0000, 0x01);

    #if((HW_BOARD_OPTION == MR8120_TX_RDI) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA672) ||\
        (HW_BOARD_OPTION == MR8120_TX_RDI_CA532) || (HW_BOARD_OPTION  == MR8120_TX_RDI_CA542) ||(HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) ||\
        (HW_BOARD_OPTION == MR8120_TX_RDI_CA652) )
        SetHM1375_720P_15FPS_RDI_DAY();

        // 20131112
        i2cWrite_HM1375(0x04B0, 0x5D);
        i2cWrite_HM1375(0x04B6, 0x30);
        i2cWrite_HM1375(0x04B9, 0x10);
        i2cWrite_HM1375(0x05B0, 0x08);
        i2cWrite_HM1375(0x0370, 0x00);
        i2cWrite_HM1375(0x0374, 0x40);
        i2cWrite_HM1375(0x085D, 0xC0);

        // for test
        i2cWrite_HM1375(0x085D, 0xC0);
        i2cWrite_HM1375(0x085A, 0x06);  //(whole)Max INTG
        i2cWrite_HM1375(0x085B, 0x80);  

      #if (((HW_BOARD_OPTION == MR8120_TX_RDI_CA672) && ((UI_PROJ_OPT != 7)&&(UI_PROJ_OPT != 4)&&(UI_PROJ_OPT != 2)&&(UI_PROJ_OPT != 5)&&(UI_PROJ_OPT != 8))) ||\
           (HW_BOARD_OPTION  == MR8120_TX_RDI_CL692) || (HW_BOARD_OPTION == MR8120_TX_RDI_CA652))
        // 20140218
        if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )
        {
            i2cWrite_HM1375(0x04C0, 0x88); 
            i2cWrite_HM1375(0x05B0, 0x08); 
        }
        else    // VGA
        {
            i2cWrite_HM1375(0x04C0, 0x88); 
            i2cWrite_HM1375(0x05B0, 0x0C);
        }
      #else
        // 20140123
        i2cWrite_HM1375(0x04C0, 0x88); 
        i2cWrite_HM1375(0x05B0, 0x0C); 
      #endif


    #elif (HW_BOARD_OPTION == MR8211_ZINWELL)
        if( (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_1280X720) ||  (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_704x480) || (uiMenuVideoSizeSetting == UI_MENU_VIDEO_SIZE_352x240) )
        {
            i2cWrite_HM1375(0x085A, 0x03);  //(whole)Max INTG
            i2cWrite_HM1375(0x085B, 0x40);  
        }
        else    // VGA
        {
            i2cWrite_HM1375(0x085A, 0x04);  //(whole)Max INTG
            i2cWrite_HM1375(0x085B, 0x18);  
        }
    #endif
    }
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger
    //i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
}
#elif((HW_BOARD_OPTION == MR8211_TX_RDI_SEP)||(HW_BOARD_OPTION == MR8211B_TX_RDI_WD542I))
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    count=0;
    DayNightMode =Level;

    if(Level == SIU_NIGHT_MODE) //AGC > 16x: 進入夜間模式
	{   //夜間模式
        DEBUG_SIU("##enter night\n");

        i2cWrite_HM1375(0x0025,0x00);   
        i2cWrite_HM1375(0x0026,0x67); 
        i2cWrite_HM1375(0x0010,0x00);   
        i2cWrite_HM1375(0x0011,0x02);
        i2cWrite_HM1375(0x0045,0x35);
        i2cWrite_HM1375(0x007A,0x06);
        i2cWrite_HM1375(0x007B,0x40);
        i2cWrite_HM1375(0x0540,0x00);
        i2cWrite_HM1375(0x0541,0x7D);
        i2cWrite_HM1375(0x0542,0x00);
        i2cWrite_HM1375(0x0543,0x69);
        i2cWrite_HM1375(0x085A,0x08);             
        i2cWrite_HM1375(0x085B,0x70);
        i2cWrite_HM1375(0x085C,0x02);             
        i2cWrite_HM1375(0x085D,0xC0);
        i2cWrite_HM1375(0x0726,0x03);
        i2cWrite_HM1375(0x0398,0x03);
        i2cWrite_HM1375(0x0399,0x80);
        i2cWrite_HM1375(0x039A,0x04);
        i2cWrite_HM1375(0x039B,0xBB);
        i2cWrite_HM1375(0x039C,0x06);
        i2cWrite_HM1375(0x039D,0xC2);
        i2cWrite_HM1375(0x03A6,0x2D);
        i2cWrite_HM1375(0x03A7,0x78);
        i2cWrite_HM1375(0x03B1,0x01);
        i2cWrite_HM1375(0x03B0,0x1B);

        i2cWrite_HM1375(0x05B0,0x04);
        i2cWrite_HM1375(0x05B1,0x00);
        i2cWrite_HM1375(0x0370,0x02);
        i2cWrite_HM1375(0x0372,0x02);
        i2cWrite_HM1375(0x0374,0x40);
        i2cWrite_HM1375(0x0376,0x00);
        i2cWrite_HM1375(0x0480,0x00);
        i2cWrite_HM1375(0x0481,0x00);
        i2cWrite_HM1375(0x0482,0x00);
        i2cWrite_HM1375(0x02E3,0x03);
        i2cWrite_HM1375(0x02E0,0x03);

        i2cWrite_HM1375(0x04C0,0x00);
        i2cWrite_HM1375(0x04C1,0x05);
        i2cWrite_HM1375(0x02A0,0x04);

        i2cWrite_HM1375(0x038E,AETargetMeanTab_720P_N[siuY_TargetIndex]);
        i2cWrite_HM1375(0x0430,0x00);
        i2cWrite_HM1375(0x0870,0x00);
        i2cWrite_HM1375(0x0871,0x34);
        i2cWrite_HM1375(0x0872,0x00);
        i2cWrite_HM1375(0x0873,0xEC);
        i2cWrite_HM1375(0x0874,0x00);
        i2cWrite_HM1375(0x0875,0x20);
        i2cWrite_HM1375(0x0876,0x00);
        i2cWrite_HM1375(0x0877,0x9C);

        i2cWrite_HM1375(0x0123,0x2D);   
        i2cWrite_HM1375(0x01B0,0x14);   
        i2cWrite_HM1375(0x0251,0x10);   
        i2cWrite_HM1375(0x0252,0x08); 
        i2cWrite_HM1375(0x0220,0x00);    
        i2cWrite_HM1375(0x0221,0x90);   
        i2cWrite_HM1375(0x0222,0x00);   
        i2cWrite_HM1375(0x0223,0x80);   
        i2cWrite_HM1375(0x0224,0x80);   
        i2cWrite_HM1375(0x0225,0x00);   
        i2cWrite_HM1375(0x0226,0x80);   
        i2cWrite_HM1375(0x0227,0x80);   
        i2cWrite_HM1375(0x0228,0x00);   
        i2cWrite_HM1375(0x0229,0x80);   
        i2cWrite_HM1375(0x022A,0x90);   
        i2cWrite_HM1375(0x022B,0x00);   
        i2cWrite_HM1375(0x022C,0x98);   

        i2cWrite_HM1375(0x0420,0x00); 
        i2cWrite_HM1375(0x0423,0x84);
        i2cWrite_HM1375(0x0333,0x00);   
        i2cWrite_HM1375(0x0335,0x05);
        i2cWrite_HM1375(0x0349,0x5F);
        i2cWrite_HM1375(0x034B,0x65);     
        i2cWrite_HM1375(0x034C,0x3F);     
        i2cWrite_HM1375(0x0432,0x30);

        i2cWrite_HM1375(0x02C0,0x89);
        i2cWrite_HM1375(0x02C1,0x01);
        i2cWrite_HM1375(0x02C2,0x71);
        i2cWrite_HM1375(0x02C3,0x04);
        i2cWrite_HM1375(0x02C4,0x0F);
        i2cWrite_HM1375(0x02C5,0x04);
        i2cWrite_HM1375(0x02C6,0x3D);
        i2cWrite_HM1375(0x02C7,0x04);
        i2cWrite_HM1375(0x02C8,0x93);
        i2cWrite_HM1375(0x02C9,0x01);
        i2cWrite_HM1375(0x02CA,0x57);
        i2cWrite_HM1375(0x02CB,0x04);
        i2cWrite_HM1375(0x02CC,0x0F);
        i2cWrite_HM1375(0x02CD,0x04);
        i2cWrite_HM1375(0x02CE,0x8F);
        i2cWrite_HM1375(0x02CF,0x04);
        i2cWrite_HM1375(0x02D0,0x9E);
        i2cWrite_HM1375(0x02D1,0x01);

        i2cWrite_HM1375(0x0803,0xC8);  
        i2cWrite_HM1375(0x038A,0x64);   //Day to Night
        i2cWrite_HM1375(0x038B,0x64);   //Night to Day
        i2cWrite_HM1375(0x0395,0x12);
        i2cWrite_HM1375(0x0465,0x22);
        i2cWrite_HM1375(0x0466,0x22); 
        i2cWrite_HM1375(0x03C3,0x08); 
        i2cWrite_HM1375(0x0144,0x00);
        i2cWrite_HM1375(0x0145,0x00);  
        i2cWrite_HM1375(0x0450,0xFF);  
        i2cWrite_HM1375(0x0453,0x00);                                   
     
        i2cWrite_HM1375(0x04B0,0x59);   
        i2cWrite_HM1375(0x04B6,0x30);   
        i2cWrite_HM1375(0x04BD,0x15);   
        i2cWrite_HM1375(0x04B9,0x11);   
        i2cWrite_HM1375(0x04B3,0x13);   
        i2cWrite_HM1375(0x04B1,0x08);   
        i2cWrite_HM1375(0x04BA,0x00); 

        i2cWrite_HM1375(0x0100,0x01);
        i2cWrite_HM1375(0x0101,0x01);
        i2cWrite_HM1375(0x0000,0x01);
    }
    else
    {   //日間模式
        DEBUG_SIU("##enter day\n");

        i2cWrite_HM1375(0x0023,0x77); 
        i2cWrite_HM1375(0x0025,0x00);   
        i2cWrite_HM1375(0x0026,0x68); 
        i2cWrite_HM1375(0x0010,0x00);   
        i2cWrite_HM1375(0x0011,0x02);
        i2cWrite_HM1375(0x0045,0x35);
        i2cWrite_HM1375(0x007A,0x06);
        i2cWrite_HM1375(0x007B,0x47);
        i2cWrite_HM1375(0x0540,0x00);
        i2cWrite_HM1375(0x0541,0x8C);
        i2cWrite_HM1375(0x0542,0x00);
        i2cWrite_HM1375(0x0543,0xA8);
        i2cWrite_HM1375(0x085A,0x04);         
        i2cWrite_HM1375(0x085B,0xBB); //12.5fps 
        i2cWrite_HM1375(0x085C,0x02);             
        i2cWrite_HM1375(0x085D,0x7F);
        i2cWrite_HM1375(0x0726,0x03);             
        i2cWrite_HM1375(0x0398,0x03); //25fps           
        i2cWrite_HM1375(0x0399,0x80);     
        i2cWrite_HM1375(0x039A,0x04);             
        i2cWrite_HM1375(0x039B,0xBB); //15fps       
        i2cWrite_HM1375(0x039C,0x06);       
        i2cWrite_HM1375(0x039D,0xC2); //10FPS           
        i2cWrite_HM1375(0x03A6,0x2D);       
        i2cWrite_HM1375(0x03A7,0x78);             
        i2cWrite_HM1375(0x03B1,0x01);             
        i2cWrite_HM1375(0x03B0,0x1B); 
      
        i2cWrite_HM1375(0x05B0,0x0A);
        i2cWrite_HM1375(0x05B1,0x00);
        i2cWrite_HM1375(0x0370,0x00);
        i2cWrite_HM1375(0x0372,0x08);
        i2cWrite_HM1375(0x0374,0x60);
        i2cWrite_HM1375(0x0376,0x00);
        i2cWrite_HM1375(0x0480,0x58);
        i2cWrite_HM1375(0x0481,0x06);
        i2cWrite_HM1375(0x0482,0x06);
        i2cWrite_HM1375(0x02E3,0x06);
        i2cWrite_HM1375(0x02E0,0x06);

        i2cWrite_HM1375(0x04C0,0x88);
        i2cWrite_HM1375(0x04C1,0x00);
        i2cWrite_HM1375(0x02A0,0x06);

        i2cWrite_HM1375(0x038E,AETargetMeanTab_720P[siuY_TargetIndex]);
        i2cWrite_HM1375(0x0430,0x10);
        i2cWrite_HM1375(0x0870,0x00);
        i2cWrite_HM1375(0x0871,0x2C);
        i2cWrite_HM1375(0x0872,0x01);
        i2cWrite_HM1375(0x0873,0x16);
        i2cWrite_HM1375(0x0874,0x00);
        i2cWrite_HM1375(0x0875,0x1E);
        i2cWrite_HM1375(0x0876,0x00);
        i2cWrite_HM1375(0x0877,0xA0);

        i2cWrite_HM1375(0x0123,0x2D);   
        i2cWrite_HM1375(0x01B0,0x14);   
        i2cWrite_HM1375(0x0251,0x10);   
        i2cWrite_HM1375(0x0252,0x08); 
        i2cWrite_HM1375(0x0220,0x00);    
        i2cWrite_HM1375(0x0221,0x90);   
        i2cWrite_HM1375(0x0222,0x00);   
        i2cWrite_HM1375(0x0223,0x80);   
        i2cWrite_HM1375(0x0224,0x80);   
        i2cWrite_HM1375(0x0225,0x00);   
        i2cWrite_HM1375(0x0226,0x80);   
        i2cWrite_HM1375(0x0227,0x80);   
        i2cWrite_HM1375(0x0228,0x00);   
        i2cWrite_HM1375(0x0229,0x80);   
        i2cWrite_HM1375(0x022A,0x90);   
        i2cWrite_HM1375(0x022B,0x00);   
        i2cWrite_HM1375(0x022C,0x98);  
     
        i2cWrite_HM1375(0x0420,0x84); 
        i2cWrite_HM1375(0x0421,0x84);
        i2cWrite_HM1375(0x0422,0x84);
        i2cWrite_HM1375(0x0423,0x84);
        i2cWrite_HM1375(0x0333,0x86);   
        i2cWrite_HM1375(0x0335,0x05);
        i2cWrite_HM1375(0x0347,0x48);    
        i2cWrite_HM1375(0x0349,0x5F);
        i2cWrite_HM1375(0x034B,0x65);     
        i2cWrite_HM1375(0x034C,0x52);    
        i2cWrite_HM1375(0x0432,0x30);

        i2cWrite_HM1375(0x02C0,0x82);
        i2cWrite_HM1375(0x02C1,0x01);
        i2cWrite_HM1375(0x02C2,0x71);
        i2cWrite_HM1375(0x02C3,0x04);
        i2cWrite_HM1375(0x02C4,0x0F);
        i2cWrite_HM1375(0x02C5,0x04);
        i2cWrite_HM1375(0x02C6,0x3D);
        i2cWrite_HM1375(0x02C7,0x04);
        i2cWrite_HM1375(0x02C8,0x93);
        i2cWrite_HM1375(0x02C9,0x01);
        i2cWrite_HM1375(0x02CA,0x57);
        i2cWrite_HM1375(0x02CB,0x04);
        i2cWrite_HM1375(0x02CC,0x0F);
        i2cWrite_HM1375(0x02CD,0x04);
        i2cWrite_HM1375(0x02CE,0x8F);
        i2cWrite_HM1375(0x02CF,0x04);
        i2cWrite_HM1375(0x02D0,0x9E);
        i2cWrite_HM1375(0x02D1,0x01);

        i2cWrite_HM1375(0x0803,0xC8);  
        i2cWrite_HM1375(0x038A,0x64);   //Day to Night
        i2cWrite_HM1375(0x038B,0x64);   //Night to Day
        i2cWrite_HM1375(0x0395,0x12);
        i2cWrite_HM1375(0x0465,0x22);
        i2cWrite_HM1375(0x0466,0x22); 
        i2cWrite_HM1375(0x03C3,0x08); 
        i2cWrite_HM1375(0x0144,0x00);
        i2cWrite_HM1375(0x0145,0x00);  
        i2cWrite_HM1375(0x0450,0xFF);  
        i2cWrite_HM1375(0x0453,0x00);                                   

        i2cWrite_HM1375(0x04B0,0x4C);   
        i2cWrite_HM1375(0x04B6,0x30);   
        i2cWrite_HM1375(0x04BD,0x00);   
        i2cWrite_HM1375(0x04B9,0x10);   
        i2cWrite_HM1375(0x04B3,0x18);   
        i2cWrite_HM1375(0x04B1,0x88);   
        i2cWrite_HM1375(0x04BA,0x00); 

        i2cWrite_HM1375(0x0100,0x01);
        i2cWrite_HM1375(0x0101,0x01);
        i2cWrite_HM1375(0x0000,0x01); 
    
    }


}
#elif((HW_BOARD_OPTION  == MR8100_GCT_VM9710) ||(HW_BOARD_OPTION  == MR8120S_TX_GCT_VM00)||\
    (HW_BOARD_OPTION == MR8120_TX_GCT_VM00))
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    count=0;
    DayNightMode =Level;
    i2cWrite_HM1375(0x0380,0xfc);
    if(Level == SIU_NIGHT_MODE) //AGC > 16x: 進入夜間模式
	{   //夜間模式
        DEBUG_SIU("##enter night\n");
        #if 1
        //GCT_HM1375_20160104-V04
        // night 8 fps
        i2cWrite_HM1375(0x0025,0x00);                       
        i2cWrite_HM1375(0x0026,0x68); //54M
        i2cWrite_HM1375(0x0010,0x00);                       
        i2cWrite_HM1375(0x0011,0x02);                       
        i2cWrite_HM1375(0x0045,0x35);                       
        i2cWrite_HM1375(0x007A,0x06);                       
        i2cWrite_HM1375(0x007B,0x47); //0x47                 
        i2cWrite_HM1375(0x0540,0x00);                       
        i2cWrite_HM1375(0x0541,0x8c); //0x8c                      
        i2cWrite_HM1375(0x0542,0x00);                       
        i2cWrite_HM1375(0x0543,0xa8); //0xa8
        i2cWrite_HM1375(0x085C,0x02);                       
        i2cWrite_HM1375(0x085D,0x7F);                       
        i2cWrite_HM1375(0x085A,0x08); //870 -> 20-8 fps
        i2cWrite_HM1375(0x085B,0x70); //20-12.5fps          
        i2cWrite_HM1375(0x0726,0x03);                       
        i2cWrite_HM1375(0x0398,0x03);
        i2cWrite_HM1375(0x0399,0x80);                       
        i2cWrite_HM1375(0x039A,0x04);                       
        i2cWrite_HM1375(0x039B,0xBB);
        i2cWrite_HM1375(0x039C,0x06);                       
        i2cWrite_HM1375(0x039D,0xC2);
        i2cWrite_HM1375(0x03A6,0x2D);                       
        i2cWrite_HM1375(0x03A7,0x78);                       
        i2cWrite_HM1375(0x03B1,0x01);                       
        i2cWrite_HM1375(0x03B0,0x1B);                       
        i2cWrite_HM1375(0x05B0,0x04);                       
        i2cWrite_HM1375(0x05B1,0x00);                       
        i2cWrite_HM1375(0x0374,0x80);                       
        i2cWrite_HM1375(0x0376,0x00);                       
        i2cWrite_HM1375(0x0370,0x02);                       
        i2cWrite_HM1375(0x0372,0x08);                       
        i2cWrite_HM1375(0x0480,0x00);                       
        i2cWrite_HM1375(0x0481,0x04);  
        i2cWrite_HM1375(0x0482,0x08);                      
        i2cWrite_HM1375(0x02E3,0x03);                       
        i2cWrite_HM1375(0x04C1,0x00);                       
        i2cWrite_HM1375(0x04C0,0x08);                       
        i2cWrite_HM1375(0x038E,0x40); 
        i2cWrite_HM1375(0x04B0,0x78);
        i2cWrite_HM1375(0x04B1,0x86);
        i2cWrite_HM1375(0x04B6,0x1E);
        i2cWrite_HM1375(0x04BA,0x00);                         
        i2cWrite_HM1375(0x04BD,0x26);
        i2cWrite_HM1375(0x04B9,0x15);
        i2cWrite_HM1375(0x04B3,0x24);
        i2cWrite_HM1375(0x02A0,0x06);                       
        i2cWrite_HM1375(0x0870,0x00);                       
        i2cWrite_HM1375(0x0871,0x6E);                       
        i2cWrite_HM1375(0x0872,0x00);                       
        i2cWrite_HM1375(0x0873,0xE6);                       
        i2cWrite_HM1375(0x0874,0x00);                       
        i2cWrite_HM1375(0x0875,0x34);                       
        i2cWrite_HM1375(0x0876,0x00);                       
        i2cWrite_HM1375(0x0877,0x8C);                       
        i2cWrite_HM1375(0x0423,0x84);
        i2cWrite_HM1375(0x0347,0x41);
        i2cWrite_HM1375(0x0349,0x63);
        i2cWrite_HM1375(0x0432,0x30);
        i2cWrite_HM1375(0x0076,0x24);
        i2cWrite_HM1375(0x038A,0x64);   //Day to Night
        i2cWrite_HM1375(0x038B,0x64);   //Night to Day
        i2cWrite_HM1375(0x0395,0x12);
        i2cWrite_HM1375(0x0465,0x22);
        i2cWrite_HM1375(0x0466,0x22); 
        i2cWrite_HM1375(0x03C3,0x08); 
        i2cWrite_HM1375(0x0144,0x00);  // RESERVED
        i2cWrite_HM1375(0x0145,0x00);  // RESERVED
        i2cWrite_HM1375(0x0450,0xFF);   
        i2cWrite_HM1375(0x0453,0x00);                 
        i2cWrite_HM1375(0x0100,0x01);                       
        i2cWrite_HM1375(0x0101,0x01);                       
        i2cWrite_HM1375(0x0000,0x01);        
        #elif 0
        //GCT_HM1375_20160104-V03
        // night 8 fps
        i2cWrite_HM1375(0x0025,0x00);                       
        i2cWrite_HM1375(0x0026,0x68); //54M
        i2cWrite_HM1375(0x0010,0x00);                       
        i2cWrite_HM1375(0x0011,0x02);                       
        i2cWrite_HM1375(0x0045,0x35);                       
        i2cWrite_HM1375(0x007A,0x06);                       
        i2cWrite_HM1375(0x007B,0x47); //0x47                 
        i2cWrite_HM1375(0x0540,0x00);                       
        i2cWrite_HM1375(0x0541,0x8c); //0x8c                      
        i2cWrite_HM1375(0x0542,0x00);                       
        i2cWrite_HM1375(0x0543,0xa8); //0xa8
        i2cWrite_HM1375(0x085C,0x02);                       
        i2cWrite_HM1375(0x085D,0x7F);                       
        i2cWrite_HM1375(0x085A,0x08); //870 -> 20-8 fps
        i2cWrite_HM1375(0x085B,0x70); //20-12.5fps          
        i2cWrite_HM1375(0x0726,0x03);                       
        i2cWrite_HM1375(0x0398,0x03);
        i2cWrite_HM1375(0x0399,0x80);                       
        i2cWrite_HM1375(0x039A,0x04);                       
        i2cWrite_HM1375(0x039B,0xBB);
        i2cWrite_HM1375(0x039C,0x06);                       
        i2cWrite_HM1375(0x039D,0xC2);
        i2cWrite_HM1375(0x03A6,0x2D);                       
        i2cWrite_HM1375(0x03A7,0x78);                       
        i2cWrite_HM1375(0x03B1,0x01);                       
        i2cWrite_HM1375(0x03B0,0x1B);                       
        i2cWrite_HM1375(0x05B0,0x04);                       
        i2cWrite_HM1375(0x05B1,0x00);                       
        i2cWrite_HM1375(0x0374,0x80);                       
        i2cWrite_HM1375(0x0376,0x00);                       
        i2cWrite_HM1375(0x0370,0x02);                       
        i2cWrite_HM1375(0x0372,0x08);                       
        i2cWrite_HM1375(0x0480,0x00);                       
        i2cWrite_HM1375(0x0481,0x04);  
        i2cWrite_HM1375(0x0482,0x08);                      
        i2cWrite_HM1375(0x02E3,0x03);                       
        i2cWrite_HM1375(0x04C1,0x00);                       
        i2cWrite_HM1375(0x04C0,0x08);                       
        i2cWrite_HM1375(0x038E,0x40); 
        i2cWrite_HM1375(0x04B0,0x78);
        i2cWrite_HM1375(0x04B1,0x86);
        i2cWrite_HM1375(0x04B6,0x1E);
        i2cWrite_HM1375(0x04BA,0x00);                         
        i2cWrite_HM1375(0x04BD,0x26);
        i2cWrite_HM1375(0x04B9,0x15);
        i2cWrite_HM1375(0x04B3,0x24);
        i2cWrite_HM1375(0x02A0,0x06);                       
        i2cWrite_HM1375(0x0870,0x00);                       
        i2cWrite_HM1375(0x0871,0x6E);                       
        i2cWrite_HM1375(0x0872,0x00);                       
        i2cWrite_HM1375(0x0873,0xE6);                       
        i2cWrite_HM1375(0x0874,0x00);                       
        i2cWrite_HM1375(0x0875,0x34);                       
        i2cWrite_HM1375(0x0876,0x00);                       
        i2cWrite_HM1375(0x0877,0x8C);                       
        i2cWrite_HM1375(0x0423,0x84);
        i2cWrite_HM1375(0x0347,0x41);
        i2cWrite_HM1375(0x0349,0x63);
        i2cWrite_HM1375(0x0432,0x30);
        i2cWrite_HM1375(0x0076,0x24);
        i2cWrite_HM1375(0x038A,0x20);   //Day to Night
        i2cWrite_HM1375(0x038B,0x40);   //Night to Day
        i2cWrite_HM1375(0x0395,0x36);
        i2cWrite_HM1375(0x0465,0x46);
        i2cWrite_HM1375(0x0466,0x46); 
        i2cWrite_HM1375(0x03C3,0x08); 
        i2cWrite_HM1375(0x0144,0x00);  // RESERVED
        i2cWrite_HM1375(0x0145,0x00);  // RESERVED
        i2cWrite_HM1375(0x0450,0xFF);   
        i2cWrite_HM1375(0x0453,0x00);                 
        i2cWrite_HM1375(0x0100,0x01);                       
        i2cWrite_HM1375(0x0101,0x01);                       
        i2cWrite_HM1375(0x0000,0x01);
        #elif 0
        //GCT_HM1375_20151228-V01
        // night 8 fps
        i2cWrite_HM1375(0x0025,0x00);                       
        i2cWrite_HM1375(0x0026,0x68); //54M
        i2cWrite_HM1375(0x0010,0x00);                       
        i2cWrite_HM1375(0x0011,0x02);                       
        i2cWrite_HM1375(0x0045,0x35);                       
        i2cWrite_HM1375(0x007A,0x06);                       
        i2cWrite_HM1375(0x007B,0x47); //0x47                 
        i2cWrite_HM1375(0x0540,0x00);                       
        i2cWrite_HM1375(0x0541,0x8c); //0x8c                      
        i2cWrite_HM1375(0x0542,0x00);                       
        i2cWrite_HM1375(0x0543,0xa8); //0xa8
        i2cWrite_HM1375(0x085C,0x02);                       
        i2cWrite_HM1375(0x085D,0x7F);                       
        i2cWrite_HM1375(0x085A,0x08); //870 -> 20-8 fps
        i2cWrite_HM1375(0x085B,0x70); //20-12.5fps          
        i2cWrite_HM1375(0x0726,0x03);                       
        i2cWrite_HM1375(0x0398,0x03);
        i2cWrite_HM1375(0x0399,0x80);                       
        i2cWrite_HM1375(0x039A,0x04);                       
        i2cWrite_HM1375(0x039B,0xBB);
        i2cWrite_HM1375(0x039C,0x06);                       
        i2cWrite_HM1375(0x039D,0xC2);
        i2cWrite_HM1375(0x03A6,0x2D);                       
        i2cWrite_HM1375(0x03A7,0x78);                       
        i2cWrite_HM1375(0x03B1,0x01);                       
        i2cWrite_HM1375(0x03B0,0x1B);                       
        i2cWrite_HM1375(0x05B0,0x04);                       
        i2cWrite_HM1375(0x05B1,0x00);                       
        i2cWrite_HM1375(0x0374,0x80);                       
        i2cWrite_HM1375(0x0376,0x00);                       
        i2cWrite_HM1375(0x0370,0x02);                       
        i2cWrite_HM1375(0x0372,0x08);                       
        i2cWrite_HM1375(0x0480,0x00);                       
        i2cWrite_HM1375(0x0481,0x04);                       
        i2cWrite_HM1375(0x02E3,0x03);                       
        i2cWrite_HM1375(0x04C1,0x10);                       
        i2cWrite_HM1375(0x04C0,0x08);                       
        i2cWrite_HM1375(0x038E,0x40); 
        i2cWrite_HM1375(0x04B0,0x78);
        i2cWrite_HM1375(0x04B1,0x86);
        i2cWrite_HM1375(0x04B6,0x1E);
        i2cWrite_HM1375(0x04BA,0x00);                         
        i2cWrite_HM1375(0x04BD,0x26);
        i2cWrite_HM1375(0x04B9,0x15);
        i2cWrite_HM1375(0x04B3,0x24);
        i2cWrite_HM1375(0x02A0,0x06);                       
        i2cWrite_HM1375(0x0870,0x00);                       
        i2cWrite_HM1375(0x0871,0x6E);                       
        i2cWrite_HM1375(0x0872,0x00);                       
        i2cWrite_HM1375(0x0873,0xE6);                       
        i2cWrite_HM1375(0x0874,0x00);                       
        i2cWrite_HM1375(0x0875,0x34);                       
        i2cWrite_HM1375(0x0876,0x00);                       
        i2cWrite_HM1375(0x0877,0x8C);                       
        i2cWrite_HM1375(0x0423,0x84);
        i2cWrite_HM1375(0x0432,0x08);
        i2cWrite_HM1375(0x0347,0x41);
        i2cWrite_HM1375(0x0349,0x63);
        i2cWrite_HM1375(0x0432,0x08);
        i2cWrite_HM1375(0x0076,0x24);
        i2cWrite_HM1375(0x038A,0x50);   //Day to Night
        i2cWrite_HM1375(0x038B,0x10);   //Night to Day
        i2cWrite_HM1375(0x0395,0x12);
        i2cWrite_HM1375(0x0465,0x46);
        i2cWrite_HM1375(0x0466,0x46); 
        i2cWrite_HM1375(0x03C3,0x08); 
        i2cWrite_HM1375(0x0144,0x00);  // RESERVED
        i2cWrite_HM1375(0x0145,0x00);  // RESERVED                  
        i2cWrite_HM1375(0x0100,0x01);                       
        i2cWrite_HM1375(0x0101,0x01);                       
        #elif 0
        //GCT_HM1375_20150923
        //08-29,66Mhz@25fps //Night mode 
        // 8 fps
        i2cWrite_HM1375(0x0025,0x00);                       
        i2cWrite_HM1375(0x0026,0x68); //54M
        i2cWrite_HM1375(0x0010,0x00);                       
        i2cWrite_HM1375(0x0011,0x10);                       
        i2cWrite_HM1375(0x0045,0x35);                       
        i2cWrite_HM1375(0x007A,0x06);                       
        i2cWrite_HM1375(0x007B,0x47); //0x47                 
        i2cWrite_HM1375(0x0540,0x00);                       
        i2cWrite_HM1375(0x0541,0x8c); //0x8c                      
        i2cWrite_HM1375(0x0542,0x00);                       
        i2cWrite_HM1375(0x0543,0xa8); //0xa8
        i2cWrite_HM1375(0x085C,0x02);                       
        i2cWrite_HM1375(0x085D,0x7F);                       
        i2cWrite_HM1375(0x085A,0x08); //870 -> 20-8 fps
        i2cWrite_HM1375(0x085B,0x70); //20-12.5fps          
        i2cWrite_HM1375(0x0726,0x03);                       
        i2cWrite_HM1375(0x0398,0x03);
        i2cWrite_HM1375(0x0399,0x80);                       
        i2cWrite_HM1375(0x039A,0x04);                       
        i2cWrite_HM1375(0x039B,0xBB);
        i2cWrite_HM1375(0x039C,0x06);                       
        i2cWrite_HM1375(0x039D,0xC2);
        i2cWrite_HM1375(0x03A6,0x2D);                       
        i2cWrite_HM1375(0x03A7,0x78);                       
        i2cWrite_HM1375(0x03B1,0x01);                       
        i2cWrite_HM1375(0x03B0,0x1B);                       
        i2cWrite_HM1375(0x05B0,0x04);                       
        i2cWrite_HM1375(0x05B1,0x00);                       
        i2cWrite_HM1375(0x0374,0x80);                       
        i2cWrite_HM1375(0x0376,0x00);                       
        i2cWrite_HM1375(0x0370,0x02);                       
        i2cWrite_HM1375(0x0372,0x08);                       
        i2cWrite_HM1375(0x0480,0x00);                       
        i2cWrite_HM1375(0x0481,0x04);                       
        i2cWrite_HM1375(0x02E3,0x03);                       
        i2cWrite_HM1375(0x04C1,0x10);                       
        i2cWrite_HM1375(0x04C0,0x08);                       
        i2cWrite_HM1375(0x038E,0x40); 
        i2cWrite_HM1375(0x04B0,0x78);
        i2cWrite_HM1375(0x04B1,0x86);
        i2cWrite_HM1375(0x04B6,0x1E);
        i2cWrite_HM1375(0x04BA,0x00);                         
        i2cWrite_HM1375(0x04BD,0x26);
        i2cWrite_HM1375(0x04B9,0x15);
        i2cWrite_HM1375(0x04B3,0x24);
        i2cWrite_HM1375(0x02A0,0x06);                       
        i2cWrite_HM1375(0x0870,0x00);                       
        i2cWrite_HM1375(0x0871,0x6D);                       
        i2cWrite_HM1375(0x0872,0x00);                       
        i2cWrite_HM1375(0x0873,0xE5);                       
        i2cWrite_HM1375(0x0874,0x00);                       
        i2cWrite_HM1375(0x0875,0x34);                       
        i2cWrite_HM1375(0x0876,0x00);                       
        i2cWrite_HM1375(0x0877,0x8D);                       
        i2cWrite_HM1375(0x0423,0x84);
        i2cWrite_HM1375(0x0432,0x08);
        i2cWrite_HM1375(0x0347,0x41);
        i2cWrite_HM1375(0x0349,0x63);
        i2cWrite_HM1375(0x0432,0x08);
        i2cWrite_HM1375(0x0100,0x01);                       
        i2cWrite_HM1375(0x0101,0x01);                       
        i2cWrite_HM1375(0x0000,0x01);
        #elif 0
        //GCT_HM1375_20150917
        //08-29,66Mhz@25fps //Night mode 
        i2cWrite_HM1375(0x0025,0x00);                       
        i2cWrite_HM1375(0x0026,0x68);                       
        i2cWrite_HM1375(0x0010,0x00);                       
        i2cWrite_HM1375(0x0011,0x10);                       
        i2cWrite_HM1375(0x0045,0x35);                       
        i2cWrite_HM1375(0x007A,0x06);                       
        i2cWrite_HM1375(0x007B,0x15);                       
        i2cWrite_HM1375(0x0540,0x00);                       
        i2cWrite_HM1375(0x0541,0x90);                       
        i2cWrite_HM1375(0x0542,0x00);                       
        i2cWrite_HM1375(0x0543,0xAD);                       
        i2cWrite_HM1375(0x085C,0x01);                       
        i2cWrite_HM1375(0x085D,0x40);                       
        i2cWrite_HM1375(0x085A,0x06);                       
        i2cWrite_HM1375(0x085B,0xC2); //12.5fps          
        i2cWrite_HM1375(0x0726,0x83);                       
        i2cWrite_HM1375(0x0398,0x03); // 25fps              
        i2cWrite_HM1375(0x0399,0x80);                       
        i2cWrite_HM1375(0x039A,0x04);                       
        i2cWrite_HM1375(0x039B,0xBB); //15fps               
        i2cWrite_HM1375(0x039C,0x06);                       
        i2cWrite_HM1375(0x039D,0xC2); //10FPS                
        i2cWrite_HM1375(0x03A6,0x2D);                       
        i2cWrite_HM1375(0x03A7,0x78);                       
        i2cWrite_HM1375(0x03B1,0x01);                       
        i2cWrite_HM1375(0x03B0,0x1B);                       
        i2cWrite_HM1375(0x05B0,0x04);                       
        i2cWrite_HM1375(0x05B1,0x00);                       
        i2cWrite_HM1375(0x0374,0x80);                       
        i2cWrite_HM1375(0x0376,0x00);                       
        i2cWrite_HM1375(0x0370,0x02);                       
        i2cWrite_HM1375(0x0372,0x08);                       
        i2cWrite_HM1375(0x0480,0x00);                       
        i2cWrite_HM1375(0x0481,0x04);                       
        i2cWrite_HM1375(0x02E3,0x03);                       
        i2cWrite_HM1375(0x04C1,0x10);                       
        i2cWrite_HM1375(0x04C0,0x00);                       
        i2cWrite_HM1375(0x038E,0x40);                       
        i2cWrite_HM1375(0x04B1,0x86); 
        i2cWrite_HM1375(0x04BA,0x00);                         
        i2cWrite_HM1375(0x02A0,0x06);                       
        i2cWrite_HM1375(0x0870,0x00);                       
        i2cWrite_HM1375(0x0871,0x6D);                       
        i2cWrite_HM1375(0x0872,0x00);                       
        i2cWrite_HM1375(0x0873,0xE5);                       
        i2cWrite_HM1375(0x0874,0x00);                       
        i2cWrite_HM1375(0x0875,0x34);                       
        i2cWrite_HM1375(0x0876,0x00);                       
        i2cWrite_HM1375(0x0877,0x8D);                       
        i2cWrite_HM1375(0x0432,0x00);
        i2cWrite_HM1375(0x0100,0x01);                       
        i2cWrite_HM1375(0x0101,0x01);                       
        i2cWrite_HM1375(0x0000,0x01);
        #endif
	}
    else
    {   //日間模式
        DEBUG_SIU("##enter day\n");
        #if 1
        //GCT_HM1375_20160104-V04
        //54Mhz@20.1fps- //Day mode 
        i2cWrite_HM1375(0x0025,0x00); 
        i2cWrite_HM1375(0x0026,0x68);             
        i2cWrite_HM1375(0x0010,0x00);             
        i2cWrite_HM1375(0x0011,0x02);             
        i2cWrite_HM1375(0x0045,0x35);             
        i2cWrite_HM1375(0x007A,0x06);           
        i2cWrite_HM1375(0x007B,0x47);           
        i2cWrite_HM1375(0x0540,0x00);           
        i2cWrite_HM1375(0x0541,0x8C);           
        i2cWrite_HM1375(0x0542,0x00);             
        i2cWrite_HM1375(0x0543,0xA8);             
        i2cWrite_HM1375(0x085C,0x02);             
        i2cWrite_HM1375(0x085D,0x7F);             
        i2cWrite_HM1375(0x085A,0x04);         
        i2cWrite_HM1375(0x085B,0xBB); //; 12.5fps   
        i2cWrite_HM1375(0x0726,0x03);             
        i2cWrite_HM1375(0x0398,0x03); //25fps           
        i2cWrite_HM1375(0x0399,0x80);     
        i2cWrite_HM1375(0x039A,0x04);             
        i2cWrite_HM1375(0x039B,0xBB); //15fps       
        i2cWrite_HM1375(0x039C,0x06);       
        i2cWrite_HM1375(0x039D,0xC2); //10FPS           
        i2cWrite_HM1375(0x03A6,0x2D);       
        i2cWrite_HM1375(0x03A7,0x78);             
        i2cWrite_HM1375(0x03B1,0x01);             
        i2cWrite_HM1375(0x03B0,0x1B);             
        i2cWrite_HM1375(0x05B0,0x07);             
        i2cWrite_HM1375(0x05B1,0x00);             
        i2cWrite_HM1375(0x0374,0x80);             
        i2cWrite_HM1375(0x0376,0x00);           
        i2cWrite_HM1375(0x0370,0x02);             
        i2cWrite_HM1375(0x0372,0x08);             
        i2cWrite_HM1375(0x0480,0x70);             
        i2cWrite_HM1375(0x0481,0x04);   
        i2cWrite_HM1375(0x0482,0x08);                     
        i2cWrite_HM1375(0x02E3,0x06);           
        i2cWrite_HM1375(0x04C1,0x88);   
        i2cWrite_HM1375(0x04C0,0x00); 
        i2cWrite_HM1375(0x038E,AETargetMeanTab_720P[siuY_TargetIndex]); 
        i2cWrite_HM1375(0x04B0,0x78);
        i2cWrite_HM1375(0x04B1,0x86); 
        i2cWrite_HM1375(0x04B6,0x1E);
        i2cWrite_HM1375(0x04BA,0x00);
        i2cWrite_HM1375(0x04BD,0x60);
        i2cWrite_HM1375(0x04B9,0x15);
        i2cWrite_HM1375(0x04B3,0x24);
        i2cWrite_HM1375(0x02A0,0x06);   
        i2cWrite_HM1375(0x0870,0x00); 
        i2cWrite_HM1375(0x0871,0x14); 
        i2cWrite_HM1375(0x0872,0x01); 
        i2cWrite_HM1375(0x0873,0x20); 
        i2cWrite_HM1375(0x0874,0x00); 
        i2cWrite_HM1375(0x0875,0x14); 
        i2cWrite_HM1375(0x0876,0x00); 
        i2cWrite_HM1375(0x0877,0xEC);
        i2cWrite_HM1375(0x0423,0x84);
        i2cWrite_HM1375(0x0347,0x41);
        i2cWrite_HM1375(0x0349,0x63);
        i2cWrite_HM1375(0x0432,0x30);
        i2cWrite_HM1375(0x0076,0x24);
    	i2cWrite_HM1375(0x0803,0xC8);  
        i2cWrite_HM1375(0x038A,0x64);   //Day to Night
        i2cWrite_HM1375(0x038B,0x64);   //Night to Day
        i2cWrite_HM1375(0x0395,0x12);
        i2cWrite_HM1375(0x0465,0x22);
        i2cWrite_HM1375(0x0466,0x22); 
        i2cWrite_HM1375(0x03C3,0x08); 
        i2cWrite_HM1375(0x0144,0x00);  // RESERVED
        i2cWrite_HM1375(0x0145,0x00);  // RESERVED   
        i2cWrite_HM1375(0x0223,0x8F); 
        i2cWrite_HM1375(0x022C,0x8F); 
        i2cWrite_HM1375(0x0450,0xFF);  
        i2cWrite_HM1375(0x0453,0x00);                                   
        i2cWrite_HM1375(0x0100,0x01);   
        i2cWrite_HM1375(0x0101,0x01);             
        i2cWrite_HM1375(0x0000,0x01);
        #elif 0
        //GCT_HM1375_20160104-V03
        //54Mhz@20.1fps- //Day mode 
        i2cWrite_HM1375(0x0025,0x00); 
        i2cWrite_HM1375(0x0026,0x68);             
        i2cWrite_HM1375(0x0010,0x00);             
        i2cWrite_HM1375(0x0011,0x02);             
        i2cWrite_HM1375(0x0045,0x35);             
        i2cWrite_HM1375(0x007A,0x06);           
        i2cWrite_HM1375(0x007B,0x47);           
        i2cWrite_HM1375(0x0540,0x00);           
        i2cWrite_HM1375(0x0541,0x8C);           
        i2cWrite_HM1375(0x0542,0x00);             
        i2cWrite_HM1375(0x0543,0xA8);             
        i2cWrite_HM1375(0x085C,0x02);             
        i2cWrite_HM1375(0x085D,0x7F);             
        i2cWrite_HM1375(0x085A,0x04);         
        i2cWrite_HM1375(0x085B,0xBB); //; 12.5fps   
        i2cWrite_HM1375(0x0726,0x03);             
        i2cWrite_HM1375(0x0398,0x03); //25fps           
        i2cWrite_HM1375(0x0399,0x80);     
        i2cWrite_HM1375(0x039A,0x04);             
        i2cWrite_HM1375(0x039B,0xBB); //15fps       
        i2cWrite_HM1375(0x039C,0x06);       
        i2cWrite_HM1375(0x039D,0xC2); //10FPS           
        i2cWrite_HM1375(0x03A6,0x2D);       
        i2cWrite_HM1375(0x03A7,0x78);             
        i2cWrite_HM1375(0x03B1,0x01);             
        i2cWrite_HM1375(0x03B0,0x1B);             
        i2cWrite_HM1375(0x05B0,0x07);             
        i2cWrite_HM1375(0x05B1,0x00);             
        i2cWrite_HM1375(0x0374,0x80);             
        i2cWrite_HM1375(0x0376,0x00);           
        i2cWrite_HM1375(0x0370,0x02);             
        i2cWrite_HM1375(0x0372,0x08);             
        i2cWrite_HM1375(0x0480,0x70);             
        i2cWrite_HM1375(0x0481,0x04);   
        i2cWrite_HM1375(0x0482,0x08);                     
        i2cWrite_HM1375(0x02E3,0x06);           
        i2cWrite_HM1375(0x04C1,0x88);   
        i2cWrite_HM1375(0x04C0,0x00); 
        i2cWrite_HM1375(0x038E,AETargetMeanTab_720P[siuY_TargetIndex]); 
        i2cWrite_HM1375(0x04B0,0x78);
        i2cWrite_HM1375(0x04B1,0x86); 
        i2cWrite_HM1375(0x04B6,0x1E);
        i2cWrite_HM1375(0x04BA,0x00);
        i2cWrite_HM1375(0x04BD,0x60);
        i2cWrite_HM1375(0x04B9,0x15);
        i2cWrite_HM1375(0x04B3,0x24);
        i2cWrite_HM1375(0x02A0,0x06);   
        i2cWrite_HM1375(0x0870,0x00); 
        i2cWrite_HM1375(0x0871,0x14); 
        i2cWrite_HM1375(0x0872,0x01); 
        i2cWrite_HM1375(0x0873,0x20); 
        i2cWrite_HM1375(0x0874,0x00); 
        i2cWrite_HM1375(0x0875,0x14); 
        i2cWrite_HM1375(0x0876,0x00); 
        i2cWrite_HM1375(0x0877,0xEC);
        i2cWrite_HM1375(0x0423,0x84);
        i2cWrite_HM1375(0x0347,0x41);
        i2cWrite_HM1375(0x0349,0x63);
        i2cWrite_HM1375(0x0432,0x30);
        i2cWrite_HM1375(0x0076,0x24);
    	i2cWrite_HM1375(0x0803,0xC8);  
        i2cWrite_HM1375(0x038A,0x20);   //Day to Night
        i2cWrite_HM1375(0x038B,0x40);   //Night to Day
        i2cWrite_HM1375(0x0395,0x36);
        i2cWrite_HM1375(0x0465,0x46);
        i2cWrite_HM1375(0x0466,0x46); 
        i2cWrite_HM1375(0x03C3,0x08); 
        i2cWrite_HM1375(0x0144,0x00);  // RESERVED
        i2cWrite_HM1375(0x0145,0x00);  // RESERVED   
        i2cWrite_HM1375(0x0223,0x8F); 
        i2cWrite_HM1375(0x022C,0x8F); 
        i2cWrite_HM1375(0x0450,0xFF);  
        i2cWrite_HM1375(0x0453,0x00);                                   
        i2cWrite_HM1375(0x0100,0x01);   
        i2cWrite_HM1375(0x0101,0x01);             
        i2cWrite_HM1375(0x0000,0x01);
        #elif 0
        //GCT_HM1375_20151228-V01
        //66Mhz@25fps //Day mode 
        i2cWrite_HM1375(0x0025,0x00); 
        i2cWrite_HM1375(0x0026,0x68);             
        i2cWrite_HM1375(0x0010,0x00);             
        i2cWrite_HM1375(0x0011,0x02);             
        i2cWrite_HM1375(0x0045,0x35);             
        i2cWrite_HM1375(0x007A,0x06);           
        i2cWrite_HM1375(0x007B,0x47);           
        i2cWrite_HM1375(0x0540,0x00);           
        i2cWrite_HM1375(0x0541,0x8C);           
        i2cWrite_HM1375(0x0542,0x00);             
        i2cWrite_HM1375(0x0543,0xA8);             
        i2cWrite_HM1375(0x085C,0x02);             
        i2cWrite_HM1375(0x085D,0x7F);             
        i2cWrite_HM1375(0x085A,0x04);         
        i2cWrite_HM1375(0x085B,0xBB); //; 12.5fps   
        i2cWrite_HM1375(0x0726,0x03);             
        i2cWrite_HM1375(0x0398,0x03); //25fps           
        i2cWrite_HM1375(0x0399,0x80);     
        i2cWrite_HM1375(0x039A,0x04);             
        i2cWrite_HM1375(0x039B,0xBB); //15fps       
        i2cWrite_HM1375(0x039C,0x06);       
        i2cWrite_HM1375(0x039D,0xC2); //10FPS           
        i2cWrite_HM1375(0x03A6,0x2D);       
        i2cWrite_HM1375(0x03A7,0x78);             
        i2cWrite_HM1375(0x03B1,0x01);             
        i2cWrite_HM1375(0x03B0,0x1B);             
        i2cWrite_HM1375(0x05B0,0x04);             
        i2cWrite_HM1375(0x05B1,0x00);             
        i2cWrite_HM1375(0x0374,0x80);             
        i2cWrite_HM1375(0x0376,0x00);           
        i2cWrite_HM1375(0x0370,0x02);             
        i2cWrite_HM1375(0x0372,0x08);             
        i2cWrite_HM1375(0x0480,0x70);             
        i2cWrite_HM1375(0x0481,0x04);                       
        i2cWrite_HM1375(0x02E3,0x06);           
        i2cWrite_HM1375(0x04C1,0x88);   
        i2cWrite_HM1375(0x04C0,0x10); 
        i2cWrite_HM1375(0x038E,AETargetMeanTab_720P[siuY_TargetIndex]); 
        i2cWrite_HM1375(0x04B0,0x78);
        i2cWrite_HM1375(0x04B1,0x86); 
        i2cWrite_HM1375(0x04B6,0x1E);
        i2cWrite_HM1375(0x04BA,0x00);
        i2cWrite_HM1375(0x04BD,0x26);
        i2cWrite_HM1375(0x04B9,0x15);
        i2cWrite_HM1375(0x04B3,0x24);
        i2cWrite_HM1375(0x02A0,0x06);   
        i2cWrite_HM1375(0x0870,0x00); 
        i2cWrite_HM1375(0x0871,0x40); //12-17
        i2cWrite_HM1375(0x0872,0x01); 
        i2cWrite_HM1375(0x0873,0x00); 
        i2cWrite_HM1375(0x0874,0x00); 
        i2cWrite_HM1375(0x0875,0x16); 
        i2cWrite_HM1375(0x0876,0x00); 
        i2cWrite_HM1375(0x0877,0x98);
        i2cWrite_HM1375(0x0423,0x84);
        i2cWrite_HM1375(0x0432,0x08); 
        i2cWrite_HM1375(0x0347,0x41);
        i2cWrite_HM1375(0x0349,0x63);
        i2cWrite_HM1375(0x0432,0x08);
        i2cWrite_HM1375(0x0076,0x24);
    	i2cWrite_HM1375(0x0803,0xC8);  
        i2cWrite_HM1375(0x038A,0x50);   //Day to Night
        i2cWrite_HM1375(0x038B,0x10);   //Night to Day
        i2cWrite_HM1375(0x0395,0x12);
        i2cWrite_HM1375(0x0465,0x46);
        i2cWrite_HM1375(0x0466,0x46); 
        i2cWrite_HM1375(0x03C3,0x08); 
        i2cWrite_HM1375(0x0144,0x00);  // RESERVED
        i2cWrite_HM1375(0x0145,0x00);  // RESERVED                     
        i2cWrite_HM1375(0x0100,0x01);   
        i2cWrite_HM1375(0x0101,0x01);             
        
        #elif 0
        //GCT_HM1375_20150923
        //08-29,66Mhz@25fps //Day mode 
        i2cWrite_HM1375(0x0025,0x00); 
        i2cWrite_HM1375(0x0026,0x68);             
        i2cWrite_HM1375(0x0010,0x00);             
        i2cWrite_HM1375(0x0011,0x10);             
        i2cWrite_HM1375(0x0045,0x35);             
        i2cWrite_HM1375(0x007A,0x06);           
        i2cWrite_HM1375(0x007B,0x15);           
        i2cWrite_HM1375(0x0540,0x00);           
        i2cWrite_HM1375(0x0541,0x90);           
        i2cWrite_HM1375(0x0542,0x00);             
        i2cWrite_HM1375(0x0543,0xAD);             
        i2cWrite_HM1375(0x085C,0x02);             
        i2cWrite_HM1375(0x085D,0x7F);             
        i2cWrite_HM1375(0x085A,0x04);         
        i2cWrite_HM1375(0x085B,0xBB); //; 12.5fps   
        i2cWrite_HM1375(0x0726,0x03);             
        i2cWrite_HM1375(0x0398,0x03); //25fps           
        i2cWrite_HM1375(0x0399,0x80);     
        i2cWrite_HM1375(0x039A,0x04);             
        i2cWrite_HM1375(0x039B,0xBB); //15fps       
        i2cWrite_HM1375(0x039C,0x06);       
        i2cWrite_HM1375(0x039D,0xC2); //10FPS           
        i2cWrite_HM1375(0x03A6,0x2D);       
        i2cWrite_HM1375(0x03A7,0x78);             
        i2cWrite_HM1375(0x03B1,0x01);             
        i2cWrite_HM1375(0x03B0,0x1B);             
        i2cWrite_HM1375(0x05B0,0x04);             
        i2cWrite_HM1375(0x05B1,0x00);             
        i2cWrite_HM1375(0x0374,0x80);             
        i2cWrite_HM1375(0x0376,0x00);           
        i2cWrite_HM1375(0x0370,0x02);             
        i2cWrite_HM1375(0x0372,0x08);             
        i2cWrite_HM1375(0x0480,0x70);             
        i2cWrite_HM1375(0x0481,0x04);                       
        i2cWrite_HM1375(0x02E3,0x06);           
        i2cWrite_HM1375(0x04C1,0x88);   
        i2cWrite_HM1375(0x04C0,0x10); 
        i2cWrite_HM1375(0x038E,AETargetMeanTab_720P[siuY_TargetIndex]); 
        i2cWrite_HM1375(0x04B0,0x78);
        i2cWrite_HM1375(0x04B1,0x86); 
        i2cWrite_HM1375(0x04B6,0x1E);
        i2cWrite_HM1375(0x04BA,0x00);
        i2cWrite_HM1375(0x04BD,0x26);
        i2cWrite_HM1375(0x04B9,0x15);
        i2cWrite_HM1375(0x04B3,0x24);
        i2cWrite_HM1375(0x02A0,0x06);   
        i2cWrite_HM1375(0x0870,0x00); 
        i2cWrite_HM1375(0x0871,0x61); 
        i2cWrite_HM1375(0x0872,0x01); 
        i2cWrite_HM1375(0x0873,0x20); 
        i2cWrite_HM1375(0x0874,0x00); 
        i2cWrite_HM1375(0x0875,0x14); 
        i2cWrite_HM1375(0x0876,0x00); 
        i2cWrite_HM1375(0x0877,0x95);
        i2cWrite_HM1375(0x0423,0x84);
        i2cWrite_HM1375(0x0432,0x08); 
        i2cWrite_HM1375(0x0347,0x41);
        i2cWrite_HM1375(0x0349,0x63);
        i2cWrite_HM1375(0x0432,0x08);
        i2cWrite_HM1375(0x0100,0x01);   
        i2cWrite_HM1375(0x0101,0x01);             
        i2cWrite_HM1375(0x0000,0x01);
        #elif 0
        //GCT_HM1375_20150917
        //08-29,66Mhz@25fps //Day mode 
        i2cWrite_HM1375(0x0025,0x00); 
        i2cWrite_HM1375(0x0026,0x68);             
        i2cWrite_HM1375(0x0010,0x00);             
        i2cWrite_HM1375(0x0011,0x10);             
        i2cWrite_HM1375(0x0045,0x35);             
        i2cWrite_HM1375(0x007A,0x06);           
        i2cWrite_HM1375(0x007B,0x15);           
        i2cWrite_HM1375(0x0540,0x00);           
        i2cWrite_HM1375(0x0541,0x90);           
        i2cWrite_HM1375(0x0542,0x00);             
        i2cWrite_HM1375(0x0543,0xAD);             
        i2cWrite_HM1375(0x085C,0x02);             
        i2cWrite_HM1375(0x085D,0x7F);             
        i2cWrite_HM1375(0x085A,0x06);         
        i2cWrite_HM1375(0x085B,0xC2); //; 12.5fps   
        i2cWrite_HM1375(0x0726,0x83);             
        i2cWrite_HM1375(0x0398,0x03); //25fps           
        i2cWrite_HM1375(0x0399,0x80);     
        i2cWrite_HM1375(0x039A,0x04);             
        i2cWrite_HM1375(0x039B,0xBB); //15fps       
        i2cWrite_HM1375(0x039C,0x06);       
        i2cWrite_HM1375(0x039D,0xC2); //10FPS           
        i2cWrite_HM1375(0x03A6,0x2D);       
        i2cWrite_HM1375(0x03A7,0x78);             
        i2cWrite_HM1375(0x03B1,0x01);             
        i2cWrite_HM1375(0x03B0,0x1B);             
        i2cWrite_HM1375(0x05B0,0x04);             
        i2cWrite_HM1375(0x05B1,0x00);             
        i2cWrite_HM1375(0x0374,0x80);             
        i2cWrite_HM1375(0x0376,0x00);           
        i2cWrite_HM1375(0x0370,0x02);             
        i2cWrite_HM1375(0x0372,0x08);             
        i2cWrite_HM1375(0x0480,0x5C);             
        i2cWrite_HM1375(0x0481,0x04);                       
        i2cWrite_HM1375(0x02E3,0x06);           
        i2cWrite_HM1375(0x04C1,0x88);   
        i2cWrite_HM1375(0x04C0,0x10); 
        i2cWrite_HM1375(0x038E,0x40); 
        i2cWrite_HM1375(0x04B1,0x86); 
        i2cWrite_HM1375(0x04BA,0x00);
        i2cWrite_HM1375(0x02A0,0x06);   
        i2cWrite_HM1375(0x0870,0x00); 
        i2cWrite_HM1375(0x0871,0x61); 
        i2cWrite_HM1375(0x0872,0x01); 
        i2cWrite_HM1375(0x0873,0x20); 
        i2cWrite_HM1375(0x0874,0x00); 
        i2cWrite_HM1375(0x0875,0x14); 
        i2cWrite_HM1375(0x0876,0x00); 
        i2cWrite_HM1375(0x0877,0x95); 
        i2cWrite_HM1375(0x0432,0x10); 
        i2cWrite_HM1375(0x0100,0x01);   
        i2cWrite_HM1375(0x0101,0x01);             
        i2cWrite_HM1375(0x0000,0x01);
        #endif
        
    }
    //i2cWrite_HM1375(0x0005, 0x01);  // Trigger
    i2cWrite_HM1375(0x0380,0xff);
    //i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
}
#elif( (HW_BOARD_OPTION  == MR8120_TX_MAYON) || (HW_BOARD_OPTION == MR8120B_TX_MAYON) )
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);
        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

    if(Level == SIU_NIGHT_MODE)
	{   //夜間模式
        DEBUG_SIU("##enter night\n");
        i2cWrite_HM1375(0x085C, 0x04);
        i2cWrite_HM1375(0x085D, 0xB0);
        i2cWrite_HM1375(0x02A0, 0x06);
        i2cWrite_HM1375(0x04C1, 0x08);
        i2cWrite_HM1375(0x0370, 0x08);
        i2cWrite_HM1375(0x0372, 0x02);
        i2cWrite_HM1375(0x0480, 0x00);
        i2cWrite_HM1375(0x0481, 0x04);
        
        i2cWrite_HM1375(0x04B0, 0x60);
        i2cWrite_HM1375(0x04B6, 0x30);
        i2cWrite_HM1375(0x04B9, 0x10);
        i2cWrite_HM1375(0x05B0, 0x04);
        i2cWrite_HM1375(0x0374, 0x40);
        i2cWrite_HM1375(0x0370, 0x02);

        i2cWrite_HM1375(0x085D, 0xC0);
        i2cWrite_HM1375(0x085A, 0x06);  //(whole)Max INTG
        i2cWrite_HM1375(0x085B, 0x80);  

        i2cWrite_HM1375(0x04C0, 0x88); 
        i2cWrite_HM1375(0x05B0, 0x04); 

        i2cWrite_HM1375(0x0100, 0x01);
        i2cWrite_HM1375(0x0101, 0x01);
        i2cWrite_HM1375(0x0000, 0x01);
	}
    else
    {   //日間模式
        DEBUG_SIU("##enter day\n");
        i2cWrite_HM1375(0x085C, 0x03);
        i2cWrite_HM1375(0x085D, 0x7F);
        i2cWrite_HM1375(0x02A0, 0x04);
        i2cWrite_HM1375(0x04C1, 0x00);
        i2cWrite_HM1375(0x0370, 0x0A);
        i2cWrite_HM1375(0x0372, 0x00);
        i2cWrite_HM1375(0x0480, 0x60);
        i2cWrite_HM1375(0x0481, 0x06);
        i2cWrite_HM1375(0x0100, 0x01);
        i2cWrite_HM1375(0x0101, 0x01);
        i2cWrite_HM1375(0x0000, 0x01);

        SetHM1375_720P_15FPS_Mayon_DAY();

        i2cWrite_HM1375(0x04B0, 0x5D);
        i2cWrite_HM1375(0x04B6, 0x30);
        i2cWrite_HM1375(0x04B9, 0x10);
        i2cWrite_HM1375(0x05B0, 0x08);
        i2cWrite_HM1375(0x0370, 0x00);
        i2cWrite_HM1375(0x0374, 0x40);
        i2cWrite_HM1375(0x085D, 0xC0);

        i2cWrite_HM1375(0x085D, 0xC0);
        i2cWrite_HM1375(0x085A, 0x06);  //(whole)Max INTG
        i2cWrite_HM1375(0x085B, 0x80);  

        i2cWrite_HM1375(0x04C0, 0x88); 
        i2cWrite_HM1375(0x05B0, 0x0C); 
    }
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger
}


#elif (HW_BOARD_OPTION == MR8120_TX_TRANWO)
void siuSetSensorDayNight(u8 Level)
{
	u8	data;
    int count;

    count=0;
	do
	{
		i2cWrite_HM1375(0x0005, 0x00);  // No video, 存取register之前要先關掉shutter,否則可能會出錯
    	OSTimeDly(4);
		data 	= 0xff;
		i2cRead_HM1375(0x0005, &data);
    	OSTimeDly(4);

        if(data)
            DEBUG_SIU("i2cWrite_HM1375(0x0005, 0x00) = 0x%02x fail!!!\n", data);
        if(count >50)
        {
           DEBUG_SIU("Sensor fatal Error!\n");
		   sysForceWDTtoReboot();
        }
        count ++;
	} while(data != 0);

	if(Level == SIU_NIGHT_MODE) //AGC > 16x: 進入夜間模式
 	{
 		i2cWrite_HM1375(0x0480, 0x40);	
 	}
	else
	{
		i2cWrite_HM1375(0x0480, 0x60);
	}
    i2cWrite_HM1375(0x0005, 0x01);  // Trigger
    //i2cWrite_HM1375(0x002C, 0x00);  // Reset 8051
}
/*	
//This is just a skeleton for AFN720P 
#elif( (HW_BOARD_OPTION == MR6730_AFN)&&(HW_DERIV_MODEL==HW_DEVTYPE_CDVR_AFN720PSEN)&&(USE_SENSOR_DAY_NIGHT_MODE==2) )
void siuSetSensorDayNight(u8 Mode)
{
	if(Mode == SIU_NIGHT_MODE)
	{
		DEBUG_SIU("##enter night\n");
  		//...
	}
	else
	{
		DEBUG_SIU("##enter day\n");
		//...
	}
}
*/
#endif

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

void siu_MI1320PowerDown(void)
{
   i2cWrite_SENSOR(0x0d, 0x0006);
}

void siu_MI1320PowerUp(void)
{
   i2cWrite_SENSOR(0x0d, 0x0000);
}

u32 siuGetExposureGain(void)
{
    u8  AGAIN, DGGAIN, INTG_H, INTG_L;
    u32 Gain;

    i2cRead_HM1375(0x0015, &INTG_H);
    i2cRead_HM1375(0x0016, &INTG_L);
    i2cRead_HM1375(0x0018, &AGAIN);
    i2cRead_HM1375(0x001D, &DGGAIN);

    Gain    = ((((u32)INTG_H) << 8) | (u32)INTG_L) * ((u32)DGGAIN << AGAIN);
}
void siuSetSensorChrome(u8 level)
{



}


#endif	





